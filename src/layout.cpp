/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
*
* DoxyPress is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License version 2
* as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
*************************************************************************/

#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QString>
#include <QTextStream>
#include <QXmlAttributes>
#include <QXmlDefaultHandler>
#include <QXmlParseException>

#include <assert.h>

#include <layout.h>

#include <config.h>
#include <doxy_build_info.h>
#include <doxy_globals.h>
#include <doxy_setup.h>
#include <language.h>
#include <message.h>
#include <util.h>

static QString buildMultiTitle(const QString & titleA, enum SrcLangExt langId, const QString & titleB) {
   return titleA + "|" + QString::number(langId) + "=" + titleB;
}

template<class ...Ts>
static QString buildMultiTitle(const QString &titleA, enum SrcLangExt langId, const QString & titleB, Ts... Vs ) {

   QString retval = titleA + "|" + QString::number(langId) + "=" + titleB;
   retval = buildMultiTitle(retval, Vs...);

   return retval;
}

static bool elemIsVisible(const QXmlAttributes &attrib, bool defVal = true)
{
   QString visible = attrib.value("visible");

   if (visible.isEmpty()) {
      return defVal;
   }

   if (visible.at(0) == '$' && visible.length() > 1) {
      // values are in uppercase in the layout file
      QString id = visible.mid(1).toLower();

      bool value = Config::getBool(id);
      return value;
   }

   return visible != "no" && visible != "0";
}

LayoutNavEntry *LayoutNavEntry::find(LayoutNavEntry::Kind kind, const QString &file) const
{
   LayoutNavEntry *result = nullptr;

   for ( auto entry : m_children) {
      // depth first search, needed to find the entry furthest from the
      // root in case an entry is in the tree twice

      result = entry->find(kind, file);

      if (result) {
         return result;
      }

      if (entry->kind() == kind && (file.isEmpty() || entry->baseFile() == file)) {
         return entry;
      }
   }

   return result;
}

QString LayoutNavEntry::url() const
{
   QString url = baseFile().trimmed();

   if ((kind() != LayoutNavEntry::User && kind() != LayoutNavEntry::UserGroup) ||
         (kind() == LayoutNavEntry::UserGroup && url.startsWith("usergroup"))) {

      url += Doxy_Globals::htmlFileExtension;

   } else if (url.startsWith("@ref ") || url.startsWith("\\ref ")) {
      QSharedPointer<Definition> d;

      QString anchor;
      bool found = false;

      if (resolveLink(0, url.mid(5).trimmed(), true, &d, anchor)) {

         if (d && d->isLinkable()) {
            url = d->getOutputFileBase() + Doxy_Globals::htmlFileExtension;

            if (! anchor.isEmpty()) {
               url += "#" + anchor;
            }

            found = true;
         }
      }

      if (! found) {
         QString tmp = Config::getString("layout-file");
         msg("Explicit link request to '%s' in layout file '%s' could not be resolved\n",
                  csPrintable(url.mid(5)), csPrintable(tmp));
      }
   }

   return url;
}

class LayoutParser : public QXmlDefaultHandler
{
   private:
      class StartElement
      {

       typedef void (LayoutParser::*Handler)(const QXmlAttributes &attrib);

       public:
         StartElement(LayoutParser *parent, Handler h)
            : m_parent(parent), m_handler(h) {}

         virtual ~StartElement() {}

         virtual void operator()(const QXmlAttributes &attrib) {
            (m_parent->*m_handler)(attrib);
         }

       protected:
         StartElement() : m_parent(0), m_handler(0) {}

       private:
         LayoutParser *m_parent;
         Handler m_handler;
      };

      class StartElementKind : public StartElement
      {

       typedef void (LayoutParser::*Handler)(LayoutDocEntry::Kind kind, const QXmlAttributes &attrib);

       public:
         StartElementKind(LayoutParser *parent, LayoutDocEntry::Kind k, Handler h)
            : m_parent(parent), m_kind(k), m_handler(h) {}

         void operator()(const QXmlAttributes &attrib) override {
            (m_parent->*m_handler)(m_kind, attrib);
         }

       private:
         LayoutParser         *m_parent;
         LayoutDocEntry::Kind  m_kind;
         Handler               m_handler;
      };

      class StartElementSection : public StartElement
      {
        typedef void (LayoutParser::*Handler)(LayoutDocEntry::Kind kind, const QXmlAttributes &attrib, const QString &title);

        public:
         StartElementSection(LayoutParser *parent, LayoutDocEntry::Kind k, Handler h, const QString &title)
            : m_parent(parent), m_kind(k), m_handler(h), m_title(title) {}

         void operator()(const QXmlAttributes &attrib) override {
            (m_parent->*m_handler)(m_kind, attrib, m_title);
         }

       private:
         LayoutParser        *m_parent;
         LayoutDocEntry::Kind m_kind;
         Handler              m_handler;
         QString              m_title;
      };

      class StartElementMember : public StartElement
      {
        using Handler = void (LayoutParser::*)(const QXmlAttributes &attrib, MemberListType type,
                     const QString &title, const QString &subtitle);

        public:
          StartElementMember(LayoutParser *parent, Handler h, MemberListType type,
                     const QString &tl, const QString &ss = QString() )
            : m_parent(parent), m_handler(h), m_type(type), m_title(tl), m_subscript(ss) {}

          void operator()(const QXmlAttributes &attrib) override {
             (m_parent->*m_handler)(attrib, m_type, m_title, m_subscript);
         }

        private:
         LayoutParser  *m_parent;
         Handler        m_handler;
         MemberListType m_type;
         QString        m_title;
         QString        m_subscript;
      };

      class StartElementNavEntry : public StartElement
      {

        typedef void (LayoutParser::*Handler)(LayoutNavEntry::Kind kind, const QXmlAttributes &attrib, const QString &title);

        public:
           StartElementNavEntry(LayoutParser *parent, LayoutNavEntry::Kind kind, Handler h, const QString &tl )
              : m_parent(parent), m_kind(kind), m_handler(h), m_title(tl) {}

           void operator()(const QXmlAttributes &attrib) override {
              (m_parent->*m_handler)(m_kind, attrib, m_title);
           }

        private:
          LayoutParser          *m_parent;
          LayoutNavEntry::Kind   m_kind;
          Handler m_handler;
          QString m_title;
      };

      class EndElement
      {

        typedef void (LayoutParser::*Handler)();

        public:
         EndElement(LayoutParser *parent, Handler h) : m_parent(parent), m_handler(h) {}

         void operator()() {
            (m_parent->*m_handler)();
         }

        private:
         LayoutParser *m_parent;
         Handler       m_handler;
      };


 public:
   static LayoutParser &instance() {
      static LayoutParser theInstance;
      return theInstance;
   }

   void init() {
      m_part    = -1;
      m_rootNav = 0;

      // bool fortranOpt = Config::getBool("optimize-fortran");
      // bool javaOpt    = Config::getBool("optimize-java");

      // start & end
      m_sHandler.insert("doxypress-layout",          QMakeShared<StartElement>(this, &LayoutParser::startLayout));
      m_eHandler.insert("doxypress-layout",          QMakeShared<EndElement>(this,   &LayoutParser::endLayout));

      // class layout
      m_sHandler.insert("navindex",                  QMakeShared<StartElement>(this, &LayoutParser::startNavIndex));
      m_sHandler.insert("navindex/tab",              QMakeShared<StartElement>(this, &LayoutParser::startNavEntry));
      m_eHandler.insert("navindex/tab",              QMakeShared<EndElement>(this,   &LayoutParser::endNavEntry));
      m_eHandler.insert("navindex",                  QMakeShared<EndElement>(this,   &LayoutParser::endNavIndex));

      // class layout
      m_sHandler.insert("class",
               QMakeShared<StartElement>
               (this, &LayoutParser::startClass));

      m_sHandler.insert("class/briefdescription",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::BriefDesc, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/detaileddescription",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::DetailedDesc, &LayoutParser::startSectionEntry,
               theTranslator->trDetailedDescription()));

      m_sHandler.insert("class/authorsection",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::AuthorSection, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/includes",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::ClassIncludes, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/inheritancegraph",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::ClassInheritanceGraph, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/collaborationgraph",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::ClassCollaborationGraph, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/allmemberslink",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::ClassAllMembersLink, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/usedfiles",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::ClassUsedFiles, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/memberdecl",
               QMakeShared<StartElement>
               (this, &LayoutParser::startMemberDecl));

      m_sHandler.insert("class/memberdecl/nestedclasses",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::ClassNestedClasses, &LayoutParser::startSectionEntry,
               buildMultiTitle(theTranslator->trCompounds(), SrcLangExt_Fortran, theTranslator->trDataTypes())) );

      m_sHandler.insert("class/memberdecl/services",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_services, theTranslator->trServices()));

      m_sHandler.insert("class/memberdecl/interfaces",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_interfaces, theTranslator->trInterfaces()));

      //
      m_sHandler.insert("class/memberdecl/publictypedefs",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pubTypedefs, theTranslator->trPublicTypedefs()));

      m_sHandler.insert("class/memberdecl/protectedtypedefs",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proTypedefs, theTranslator->trProtectedTypedefs()));

      m_sHandler.insert("class/memberdecl/packagetypedefs",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pacTypedefs, theTranslator->trPackageTypedefs()));

      m_sHandler.insert("class/memberdecl/privatetypedefs",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priTypedefs, theTranslator->trPrivateTypedefs()));

      m_sHandler.insert("class/memberdecl/publictypes",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pubTypes, theTranslator->trPublicTypes()));

      m_sHandler.insert("class/memberdecl/publicslots",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pubSlots, theTranslator->trPublicSlots()));

      m_sHandler.insert("class/memberdecl/publicsignals",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pubSignals, theTranslator->trPublicSignals()));

      m_sHandler.insert("class/memberdecl/publicmethods",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pubMethods,
               buildMultiTitle(theTranslator->trPublicMembers(), SrcLangExt_ObjC, theTranslator->trInstanceMethods())));

      m_sHandler.insert("class/memberdecl/publicstaticmethods",
               QMakeShared<StartElementMember>
              (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pubStaticMethods,
               buildMultiTitle( theTranslator->trStaticPublicMembers(), SrcLangExt_ObjC, theTranslator->trClassMethods())));

      m_sHandler.insert("class/memberdecl/publicattributes",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pubAttribs, theTranslator->trPublicAttribs()));

      m_sHandler.insert("class/memberdecl/publicstaticattributes",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pubStaticAttribs, theTranslator->trStaticPublicAttribs()));

      m_sHandler.insert("class/memberdecl/protectedtypes",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proTypes, theTranslator->trProtectedTypes()));

      m_sHandler.insert("class/memberdecl/protectedslots",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proSlots, theTranslator->trProtectedSlots()));

     m_sHandler.insert("class/memberdecl/protectedsignals",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proSignals, theTranslator->trProtectedSignals()));

      m_sHandler.insert("class/memberdecl/protectedmethods",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proMethods, theTranslator->trProtectedMembers()));

      m_sHandler.insert("class/memberdecl/protectedstaticmethods",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proStaticMethods, theTranslator->trStaticProtectedMembers()));

      m_sHandler.insert("class/memberdecl/protectedattributes",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proAttribs, theTranslator->trProtectedAttribs()));

      m_sHandler.insert("class/memberdecl/protectedstaticattributes",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proStaticAttribs, theTranslator->trStaticProtectedAttribs()));

      m_sHandler.insert("class/memberdecl/packagetypes",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pacTypes, theTranslator->trPackageTypes()));

      m_sHandler.insert("class/memberdecl/packagemethods",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pacMethods, theTranslator->trPackageMembers()));

      m_sHandler.insert("class/memberdecl/packagestaticmethods",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pacStaticMethods, theTranslator->trStaticPackageMembers()));

      m_sHandler.insert("class/memberdecl/packageattributes",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pacAttribs, theTranslator->trPackageAttribs()));

      m_sHandler.insert("class/memberdecl/packagestaticattributes",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pacStaticAttribs, theTranslator->trStaticPackageAttribs()));

      m_sHandler.insert("class/memberdecl/properties",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_properties, theTranslator->trProperties()));

      m_sHandler.insert("class/memberdecl/events",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_events, theTranslator->trEvents()));

      m_sHandler.insert("class/memberdecl/privatetypes",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priTypes, theTranslator->trPrivateTypes()));

      m_sHandler.insert("class/memberdecl/privateslots",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priSlots, theTranslator->trPrivateSlots()));

      m_sHandler.insert("class/memberdecl/privatesignals",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priSignals, theTranslator->trPrivateSignals()));

      m_sHandler.insert("class/memberdecl/privatemethods",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priMethods, theTranslator->trPrivateMembers()));

      m_sHandler.insert("class/memberdecl/privatestaticmethods",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priStaticMethods, theTranslator->trStaticPrivateMembers()));

      m_sHandler.insert("class/memberdecl/privateattributes",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priAttribs, theTranslator->trPrivateAttribs()));

      m_sHandler.insert("class/memberdecl/privatestaticattributes",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priStaticAttribs, theTranslator->trStaticPrivateAttribs()));

      m_sHandler.insert("class/memberdecl/friends",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_friends, theTranslator->trFriends()));

      m_sHandler.insert("class/memberdecl/related",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_related, theTranslator->trRelatedFunctions(), theTranslator->trRelatedSubscript()));

      m_sHandler.insert("class/memberdecl/membergroups",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::MemberGroups, &LayoutParser::startSimpleEntry));

      m_eHandler.insert("class/memberdecl",
               QMakeShared<EndElement>
               (this, &LayoutParser::endMemberDecl));

      m_sHandler.insert("class/memberdef",
               QMakeShared<StartElement>
               (this, &LayoutParser::startMemberDef));

      m_sHandler.insert("class/memberdef/inlineclasses",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::ClassInlineClasses,
               &LayoutParser::startSectionEntry,
               buildMultiTitle(theTranslator->trClassDocumentation(), SrcLangExt_Fortran, theTranslator->trTypeDocumentation() )));

      m_sHandler.insert("class/memberdef/typedefs",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_typedefMembers, theTranslator->trMemberTypedefDocumentation()));

      m_sHandler.insert("class/memberdef/enums",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_enumMembers, theTranslator->trMemberEnumerationDocumentation()));

      m_sHandler.insert("class/memberdef/services",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_serviceMembers, theTranslator->trInterfaces()));

      m_sHandler.insert("class/memberdef/interfaces",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_interfaceMembers, theTranslator->trInterfaces()));

      m_sHandler.insert("class/memberdef/constructors",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_constructors, theTranslator->trConstructorDocumentation()));

      m_sHandler.insert("class/memberdef/functions",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_functionMembers, buildMultiTitle(theTranslator->trMemberFunctionDocumentation(),
               SrcLangExt_ObjC, theTranslator->trMethodDocumentation(),
               SrcLangExt_Fortran, theTranslator->trMemberFunctionDocumentationFortran() )));

      m_sHandler.insert("class/memberdef/related",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_relatedMembers, theTranslator->trRelatedFunctionDocumentation()));

      m_sHandler.insert("class/memberdef/variables",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_variableMembers, theTranslator->trMemberDataDocumentation()));

      m_sHandler.insert("class/memberdef/properties",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_propertyMembers, theTranslator->trPropertyDocumentation()));

      m_sHandler.insert("class/memberdef/events",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_eventMembers, theTranslator->trEventDocumentation()));

      m_eHandler.insert("class/memberdef",
               QMakeShared<EndElement>
               (this, &LayoutParser::endMemberDef));

      m_eHandler.insert("class",
               QMakeShared<EndElement>
               (this, &LayoutParser::endClass));

      // namespace layout
      m_sHandler.insert("namespace",
               QMakeShared<StartElement>
               (this, &LayoutParser::startNamespace));

      m_sHandler.insert("namespace/briefdescription",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::BriefDesc, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("namespace/detaileddescription",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::DetailedDesc,
               &LayoutParser::startSectionEntry, theTranslator->trDetailedDescription()));

      m_sHandler.insert("namespace/authorsection",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::AuthorSection, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("namespace/memberdecl",
               QMakeShared<StartElement>
               (this, &LayoutParser::startMemberDecl));

      m_sHandler.insert("namespace/memberdecl/nestednamespaces",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::NamespaceNestedNamespaces,
               &LayoutParser::startSectionEntry, buildMultiTitle( theTranslator->trNamespaces(),
               SrcLangExt_Java, theTranslator->trPackages(), SrcLangExt_IDL, theTranslator->trModules(),
               SrcLangExt_Fortran, theTranslator->trModules() )));

      m_sHandler.insert("namespace/memberdecl/constantgroups",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::NamespaceNestedConstantGroups,
               &LayoutParser::startSectionEntry, theTranslator->trConstantGroups()));

      m_sHandler.insert("namespace/memberdecl/classes",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::NamespaceClasses, &LayoutParser::startSectionEntry,
               buildMultiTitle(theTranslator->trCompounds(),
               SrcLangExt_Fortran, theTranslator->trDataTypes() )));

      m_sHandler.insert("namespace/memberdecl/membergroups",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::MemberGroups, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("namespace/memberdecl/typedefs",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decTypedefMembers, theTranslator->trTypedefs()));

      m_sHandler.insert("namespace/memberdecl/enums",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decEnumMembers, theTranslator->trEnumerations()));

      m_sHandler.insert("namespace/memberdecl/functions",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decFuncMembers, buildMultiTitle( theTranslator->trFunctions(),
               SrcLangExt_Fortran, theTranslator->trSubprograms() )));

      m_sHandler.insert("namespace/memberdecl/variables",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decVarMembers, theTranslator->trVariables()));

      m_eHandler.insert("namespace/memberdecl",
               QMakeShared<EndElement>
               (this, &LayoutParser::endMemberDecl));

      m_sHandler.insert("namespace/memberdef",
               QMakeShared<StartElement>
               (this, &LayoutParser::startMemberDef));

      m_sHandler.insert("namespace/memberdef/inlineclasses",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::NamespaceInlineClasses,
               &LayoutParser::startSectionEntry, buildMultiTitle(theTranslator->trClassDocumentation(),
               SrcLangExt_Fortran, theTranslator->trTypeDocumentation() )));

      m_sHandler.insert("namespace/memberdef/typedefs",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docTypedefMembers, theTranslator->trTypedefDocumentation()));

      m_sHandler.insert("namespace/memberdef/enums",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docEnumMembers, theTranslator->trEnumerationTypeDocumentation()));

      m_sHandler.insert("namespace/memberdef/functions",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docFuncMembers, buildMultiTitle( theTranslator->trFunctionDocumentation(),
               SrcLangExt_Fortran, theTranslator->trSubprogramDocumentation() )));

      m_sHandler.insert("namespace/memberdef/variables",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docVarMembers, theTranslator->trVariableDocumentation()));

      m_eHandler.insert("namespace/memberdef",
               QMakeShared<EndElement>
               (this, &LayoutParser::endMemberDef));

      m_eHandler.insert("namespace",
               QMakeShared<EndElement>
               (this, &LayoutParser::endNamespace));

      // file layout
      m_sHandler.insert("file",
               QMakeShared<StartElement>
               (this, &LayoutParser::startFile));

      m_sHandler.insert("file/briefdescription",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::BriefDesc, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("file/detaileddescription",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::DetailedDesc,
               &LayoutParser::startSectionEntry,theTranslator->trDetailedDescription()));

      m_sHandler.insert("file/authorsection",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::AuthorSection, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("file/includes",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::FileIncludes, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("file/includegraph",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::FileIncludeGraph, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("file/includedbygraph",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::FileIncludedByGraph,
               &LayoutParser::startSimpleEntry));

      m_sHandler.insert("file/sourcelink",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::FileSourceLink, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("file/memberdecl/membergroups",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::MemberGroups, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("file/memberdecl",
               QMakeShared<StartElement>
               (this, &LayoutParser::startMemberDecl));

      m_sHandler.insert("file/memberdecl/classes",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::FileClasses, &LayoutParser::startSectionEntry,
               buildMultiTitle(theTranslator->trCompounds(), SrcLangExt_Fortran, theTranslator->trDataTypes() )));

      m_sHandler.insert("file/memberdecl/namespaces",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::FileNamespaces, &LayoutParser::startSectionEntry,
                buildMultiTitle( theTranslator->trNamespaces(), SrcLangExt_Java,
               theTranslator->trPackages(), SrcLangExt_IDL, theTranslator->trModules(),
               SrcLangExt_Fortran, theTranslator->trModules() )));

      m_sHandler.insert("file/memberdecl/constantgroups",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::FileConstantGroups,
               &LayoutParser::startSectionEntry, theTranslator->trConstantGroups()));

      m_sHandler.insert("file/memberdecl/defines",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decDefineMembers, theTranslator->trDefines()));

      m_sHandler.insert("file/memberdecl/typedefs",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decTypedefMembers, theTranslator->trTypedefs()));

      m_sHandler.insert("file/memberdecl/enums",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decEnumMembers, theTranslator->trEnumerations()));

      m_sHandler.insert("file/memberdecl/functions",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decFuncMembers,
               buildMultiTitle( theTranslator->trFunctions(), SrcLangExt_Fortran, theTranslator->trSubprograms() )));

      m_sHandler.insert("file/memberdecl/variables",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decVarMembers, theTranslator->trVariables()));

      m_eHandler.insert("file/memberdecl",
               QMakeShared<EndElement>
               (this, &LayoutParser::endMemberDecl));

      m_sHandler.insert("file/memberdef",
               QMakeShared<StartElement>
               (this, &LayoutParser::startMemberDef));

      m_sHandler.insert("file/memberdef/inlineclasses",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::FileInlineClasses,
               &LayoutParser::startSectionEntry,
               buildMultiTitle( theTranslator->trClassDocumentation(), SrcLangExt_Fortran, theTranslator->trTypeDocumentation() )));

      m_sHandler.insert("file/memberdef/defines",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docDefineMembers, theTranslator->trDefineDocumentation()));

      m_sHandler.insert("file/memberdef/typedefs",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docTypedefMembers, theTranslator->trTypedefDocumentation()));

      m_sHandler.insert("file/memberdef/enums",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docEnumMembers, theTranslator->trEnumerationTypeDocumentation()));

      m_sHandler.insert("file/memberdef/functions",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docFuncMembers, buildMultiTitle(theTranslator->trFunctionDocumentation(),
               SrcLangExt_Fortran, theTranslator->trSubprogramDocumentation() )));

      m_sHandler.insert("file/memberdef/variables",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docVarMembers, theTranslator->trVariableDocumentation()));

      m_eHandler.insert("file/memberdef",
               QMakeShared<EndElement>
               (this, &LayoutParser::endMemberDef));

      m_eHandler.insert("file",
               QMakeShared<EndElement>
               (this, &LayoutParser::endFile));

      // group layout
      m_sHandler.insert("group",
               QMakeShared<StartElement>
               (this, &LayoutParser::startGroup));

      m_sHandler.insert("group/briefdescription",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::BriefDesc,
               &LayoutParser::startSimpleEntry));

      m_sHandler.insert("group/detaileddescription",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::DetailedDesc,
               &LayoutParser::startSectionEntry, theTranslator->trDetailedDescription()));

      m_sHandler.insert("group/authorsection",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::AuthorSection,
               &LayoutParser::startSimpleEntry));

      m_sHandler.insert("group/groupgraph",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::GroupGraph,
               &LayoutParser::startSimpleEntry));

      m_sHandler.insert("group/memberdecl/membergroups",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::MemberGroups,
               &LayoutParser::startSimpleEntry));

      m_sHandler.insert("group/memberdecl",
               QMakeShared<StartElement>
               (this, &LayoutParser::startMemberDecl));

      m_sHandler.insert("group/memberdecl/classes",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::GroupClasses,
               &LayoutParser::startSectionEntry,
               buildMultiTitle( theTranslator->trCompounds(), SrcLangExt_Fortran, theTranslator->trDataTypes() )));

      m_sHandler.insert("group/memberdecl/namespaces",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::GroupNamespaces,
               &LayoutParser::startSectionEntry, buildMultiTitle( theTranslator->trNamespaces(),
               SrcLangExt_Java, theTranslator->trPackages(), SrcLangExt_Fortran, theTranslator->trModules() )));

      m_sHandler.insert("group/memberdecl/dirs",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::GroupDirs,
               &LayoutParser::startSectionEntry, theTranslator->trDirectories() ));

      m_sHandler.insert("group/memberdecl/nestedgroups",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::GroupNestedGroups,
               &LayoutParser::startSectionEntry, theTranslator->trModules() ));

      m_sHandler.insert("group/memberdecl/files",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::GroupFiles,
               &LayoutParser::startSectionEntry, theTranslator->trFile(true, false) ));

      m_sHandler.insert("group/memberdecl/defines",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decDefineMembers, theTranslator->trDefines()));

      m_sHandler.insert("group/memberdecl/typedefs",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decTypedefMembers, theTranslator->trTypedefs()));

      m_sHandler.insert("group/memberdecl/enums",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decEnumMembers, theTranslator->trEnumerations()));

      m_sHandler.insert("group/memberdecl/enumvalues",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decEnumValMembers, theTranslator->trEnumerationValues()));

      m_sHandler.insert("group/memberdecl/functions",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decFuncMembers,
               buildMultiTitle(theTranslator->trFunctions(), SrcLangExt_Fortran, theTranslator->trSubprograms() )));

      m_sHandler.insert("group/memberdecl/variables",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decVarMembers, theTranslator->trVariables()));

      m_sHandler.insert("group/memberdecl/publicsignals",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decPubSignalMembers, theTranslator->trPublicSignals()));

      m_sHandler.insert("group/memberdecl/protectedsignals",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decProSignalMembers, theTranslator->trProtectedSignals()));

      m_sHandler.insert("group/memberdecl/privatesignals",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decPriSignalMembers, theTranslator->trPrivateSignals()));

      m_sHandler.insert("group/memberdecl/publicslots",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decPubSlotMembers, theTranslator->trPublicSlots()));

      m_sHandler.insert("group/memberdecl/protectedslots",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decProSlotMembers, theTranslator->trProtectedSlots()));

      m_sHandler.insert("group/memberdecl/privateslots",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decPriSlotMembers, theTranslator->trPrivateSlots()));

      m_sHandler.insert("group/memberdecl/events",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decEventMembers, theTranslator->trEvents()));

      m_sHandler.insert("group/memberdecl/properties",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decPropMembers, theTranslator->trProperties()));

      m_sHandler.insert("group/memberdecl/friends",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decFriendMembers, theTranslator->trFriends()));

      m_eHandler.insert("group/memberdecl",
               QMakeShared<EndElement>
               (this, &LayoutParser::endMemberDecl));

      m_sHandler.insert("group/memberdef",
               QMakeShared<StartElement>
               (this, &LayoutParser::startMemberDef));

      m_sHandler.insert("group/memberdef/pagedocs",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::GroupPageDocs,
               &LayoutParser::startSimpleEntry));

      m_sHandler.insert("group/memberdef/inlineclasses",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::GroupInlineClasses,
               &LayoutParser::startSectionEntry, buildMultiTitle(theTranslator->trClassDocumentation(),
               SrcLangExt_Fortran, theTranslator->trTypeDocumentation() )));

      m_sHandler.insert("group/memberdef/defines",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docDefineMembers, theTranslator->trDefineDocumentation()));

      m_sHandler.insert("group/memberdef/typedefs",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docTypedefMembers, theTranslator->trTypedefDocumentation()));

      m_sHandler.insert("group/memberdef/enums",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docEnumMembers, theTranslator->trEnumerationTypeDocumentation()));

      m_sHandler.insert("group/memberdef/enumvalues",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docEnumValMembers, theTranslator->trEnumerationValueDocumentation()));

      m_sHandler.insert("group/memberdef/functions",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docFuncMembers, buildMultiTitle( theTranslator->trFunctionDocumentation(),
               SrcLangExt_Fortran, theTranslator->trSubprogramDocumentation() )));

      m_sHandler.insert("group/memberdef/variables",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docVarMembers, theTranslator->trVariableDocumentation()));

      m_sHandler.insert("group/memberdef/publicsignals",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docPubSignalMembers, theTranslator->trPublicSignals()));

      m_sHandler.insert("group/memberdef/protectedsignals",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docProSignalMembers, theTranslator->trProtectedSignals()));

      m_sHandler.insert("group/memberdef/privatesignals",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docPriSignalMembers, theTranslator->trPrivateSignals()));

      m_sHandler.insert("group/memberdef/publicslots",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docPubSlotMembers, theTranslator->trPublicSlots()));

      m_sHandler.insert("group/memberdef/protectedslots",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docProSlotMembers, theTranslator->trProtectedSlots()));

      m_sHandler.insert("group/memberdef/privateslots",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docPriSlotMembers, theTranslator->trPrivateSlots()));

      m_sHandler.insert("group/memberdef/events",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docEventMembers, theTranslator->trEvents()));

      m_sHandler.insert("group/memberdef/properties",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docPropMembers, theTranslator->trProperties()));

      m_sHandler.insert("group/memberdef/friends",
               QMakeShared<StartElementMember>
               (this, &LayoutParser::startMemberDefEntry,
               MemberListType_docFriendMembers, theTranslator->trFriends()));

      m_eHandler.insert("group/memberdef",
               QMakeShared<EndElement>
               (this, &LayoutParser::endMemberDef));

      m_eHandler.insert("group",
               QMakeShared<EndElement>
               (this, &LayoutParser::endGroup));

      // directory layout
      m_sHandler.insert("directory",
               QMakeShared<StartElement>
               (this, &LayoutParser::startDirectory));

      m_sHandler.insert("directory/briefdescription",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::BriefDesc, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("directory/detaileddescription",
               QMakeShared<StartElementSection>
               (this, LayoutDocEntry::DetailedDesc, &LayoutParser::startSectionEntry,
               theTranslator->trDetailedDescription()));

      m_sHandler.insert("directory/directorygraph",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::DirGraph, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("directory/memberdecl",
               QMakeShared<StartElement>
               (this, &LayoutParser::startMemberDecl));

      m_sHandler.insert("directory/memberdecl/dirs",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::DirSubDirs, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("directory/memberdecl/files",
               QMakeShared<StartElementKind>
               (this, LayoutDocEntry::DirFiles, &LayoutParser::startSimpleEntry));

      m_eHandler.insert("directory/memberdecl",
               QMakeShared<EndElement>
               (this, &LayoutParser::endMemberDecl));

      m_eHandler.insert("directory",
               QMakeShared<EndElement>
               (this, &LayoutParser::endDirectory));
   }

   void startSimpleEntry(LayoutDocEntry::Kind k, const QXmlAttributes &attrib) {

      bool isVisible = elemIsVisible(attrib);

      if (m_part != -1 && isVisible) {
         LayoutDocManager::instance().addEntry((LayoutDocManager::LayoutPart)m_part, new LayoutDocEntrySimple(k));
      }
   }

   void startSectionEntry(LayoutDocEntry::Kind k, const QXmlAttributes &attrib, const QString &title) {
      bool isVisible    = elemIsVisible(attrib);
      QString userTitle = attrib.value("title");

      if (userTitle.isEmpty()) {
         userTitle = title;
      }

      if (m_part != -1 && isVisible) {
         LayoutDocManager::instance().addEntry((LayoutDocManager::LayoutPart)m_part, new LayoutDocEntrySection(k, userTitle));
      }
   }

   void startMemberDeclEntry(const QXmlAttributes &attrib, MemberListType type,
                  const QString &title, const QString &subscript) {

      QString userTitle     = attrib.value("title");
      QString userSubscript = attrib.value("subtitle");

      if (userTitle.isEmpty()) {
         userTitle = title;
      }

      if (userSubscript.isEmpty()) {
         userSubscript = subscript;
      }

      if (m_part != -1 /*&& isVisible*/) {
         LayoutDocManager::instance().addEntry((LayoutDocManager::LayoutPart)m_part,
                  new LayoutDocEntryMemberDecl(type, userTitle, userSubscript));
      }
   }

   void startMemberDefEntry(const QXmlAttributes &attrib, MemberListType type, const QString &title, const QString &unused) {
      QString userTitle = attrib.value("title");

      if (userTitle.isEmpty()) {
         userTitle = title;
      }

      if (m_part != -1 /*&& isVisible*/) {
         LayoutDocManager::instance().addEntry((LayoutDocManager::LayoutPart)m_part, new LayoutDocEntryMemberDef(type, userTitle));
      }
   }

   void startLayout(const QXmlAttributes &) {
   }

   void endLayout() {
   }

   void startNavIndex(const QXmlAttributes &) {
      m_scope   = "navindex/";
      m_rootNav = LayoutDocManager::instance().rootNavEntry();

      if (m_rootNav) {
         m_rootNav->clear();
      }
   }

   void endNavIndex() {
      m_scope = "";

      if (m_rootNav && ! m_rootNav->find(LayoutNavEntry::MainPage)) {

         // no MainPage node, add one as the first item of the root node
         LayoutNavEntry *tmpLayout = new LayoutNavEntry(m_rootNav, LayoutNavEntry::MainPage, true,
                  "index", theTranslator->trMainPage(), "");

         m_rootNav->prependChild(tmpLayout);
      }
   }

   struct NavEntryMap {

      NavEntryMap(const char *x1, LayoutNavEntry::Kind x2, const QString &x3, const QString &x4, const QString &x5, const char *x6) {

         key      = QString::fromLatin1(x1);
         kind     = x2;
         mainName = x3;
         subName  = x4;
         intro    = x5;
         baseFile = QString::fromLatin1(x6);
      }

      NavEntryMap(const char *x1, LayoutNavEntry::Kind x2, const QString &x3, const char *x4, const char *x5, const char *x6) {

         key      = QString::fromLatin1(x1);
         kind     = x2;
         mainName = x3;
         subName  = QString::fromLatin1(x4);
         intro    = QString::fromLatin1(x5);
         baseFile = QString::fromLatin1(x6);
      }

      QString key;                 // key in the layout xml file
      LayoutNavEntry::Kind kind;   // corresponding enum name
      QString mainName;            // default title for an item if it has children
      QString subName;             // optional name for an item if it is rendered as a child
      QString intro;               // introduction text to be put on the index page
      QString baseFile;            // base name of the file containing the index page
   };

   void startNavEntry(const QXmlAttributes &attrib) {
      static bool javaOpt    = Config::getBool("optimize-java");
      static bool fortranOpt = Config::getBool("optimize-fortran");
      static bool pythonOpt  = Config::getBool("optimize-python");

      static bool hasGraphicalHierarchy = Config::getBool("have-dot") && Config::getBool("dot-hierarchy");
      static bool extractAll = Config::getBool("extract-all");

      static NavEntryMap mapping[] = {
         {
            "mainpage",
            LayoutNavEntry::MainPage,
            theTranslator->trMainPage(),
            "",
            "",
            "index"
         },

         {
            "pages",
            LayoutNavEntry::Pages,
            theTranslator->trRelatedPages(),
            "",
            theTranslator->trRelatedPagesDescription(),
            "pages"
         },

         {
            "modules",
            LayoutNavEntry::Modules,
            theTranslator->trModules(),
            "",
            theTranslator->trModulesDescription(),
            "modules"
         },

         {
            "namespaces",
            LayoutNavEntry::Namespaces,

            [=]() {
               if (javaOpt || pythonOpt) {
                  return theTranslator->trPackages();

               } else if (fortranOpt) {
                  return theTranslator->trModules();

               } else  {
                  return theTranslator->trNamespaces();

               }
            }(),

            [=]() {
               if (javaOpt || pythonOpt) {
                  return theTranslator->trPackages();

               } else if (fortranOpt) {
                  return theTranslator->trModulesList();

               } else  {
                  return theTranslator->trNamespaceList();
               }
            }(),

            [=]() {
               if (javaOpt || pythonOpt) {
                  return theTranslator->trPackagesListDescription();

               } else if (fortranOpt) {
                  return theTranslator->trModulesListDescription(extractAll);

               } else  {
                  return theTranslator->trNamespacesListDescription(extractAll);

               }
            }(),

            "namespaces"
         },

         {
            "namespacelist",
            LayoutNavEntry::NamespaceList,

            [=]() {
               if (javaOpt || pythonOpt) {
                  return theTranslator->trPackages();

               } else if (fortranOpt) {
                  return theTranslator->trModulesList();

               } else  {
                  return theTranslator->trNamespaceList();

               }
            }(),

            "",

            [=]() {
               if (javaOpt || pythonOpt) {
                  return theTranslator->trPackagesListDescription();

               } else if (fortranOpt) {
                  return theTranslator->trModulesListDescription(extractAll);

               } else  {
                  return theTranslator->trNamespacesListDescription(extractAll);

               }
            }(),

            "namespaces"
         },

         {
            "namespacemembers",
            LayoutNavEntry::NamespaceMembers,

            [=]() {
               if (javaOpt || pythonOpt) {
                  return theTranslator->trPackageMembers();

               } else if (fortranOpt) {
                  return theTranslator->trModuleMembers();

               } else  {
                  return theTranslator->trNamespaceMembers();

               }
            }(),

            "",

            [=]() {
               if (fortranOpt) {
                  return theTranslator->trModulesMemberDescription(extractAll);

               } else  {
                  return theTranslator->trNamespaceMemberDescription(extractAll);

               }
            }(),

            "namespacemembers"
         },

         {
            "classindex",
            LayoutNavEntry::ClassIndex,

            [=]() {
               if (fortranOpt) {
                  return theTranslator->trDataTypes();

               } else  {
                  return theTranslator->trCompoundIndex();

               }
            }(),

            "",
            "",
            "classes"
         },

         {
            "classes",
            LayoutNavEntry::Classes,

            [=]() {
               if (fortranOpt) {
                  return theTranslator->trCompoundListFortran();

               } else  {
                  return theTranslator->trClasses();

               }
            }(),

            theTranslator->trCompoundList(),

            [=]() {
               if (fortranOpt) {
                  return theTranslator->trCompoundListDescriptionFortran();

               } else  {
                  return theTranslator->trCompoundListDescription();

               }
            }(),

            "annotated"
         },

         {
            "classlist",
            LayoutNavEntry::ClassList,

            [=]() {
               if (fortranOpt) {
                  return theTranslator->trCompoundListFortran();

               } else  {
                  return theTranslator->trCompoundList();

               }
            }(),

            "",

            [=]() {
               if (fortranOpt) {
                  return theTranslator->trCompoundListDescriptionFortran();

               } else  {
                  return theTranslator->trCompoundListDescription();

               }
            }(),

            "annotated"
         },

         {
            "hierarchy",
            LayoutNavEntry::ClassHierarchy,
            theTranslator->trClassHierarchy(),
            "",
            theTranslator->trClassHierarchyDescription(),
            hasGraphicalHierarchy ? "inherits" : "hierarchy"
         },

         {
            "classmembers",
            LayoutNavEntry::ClassMembers,

            [=]() {
               if (fortranOpt) {
                  return theTranslator->trCompoundMembersFortran();

               } else  {
                  return theTranslator->trCompoundMembers();

               }
            }(),

            "",

            [=]() {
               if (fortranOpt) {
                  return theTranslator->trCompoundMembersDescriptionFortran(extractAll);

               } else  {
                  return theTranslator->trCompoundMembersDescription(extractAll);

               }
            }(),

            "functions_all"
         },

         {
            "files",
            LayoutNavEntry::Files,
            theTranslator->trFile(true, false),
            theTranslator->trFileList(),
            theTranslator->trFileListDescription(extractAll),
            "files"
         },

         {
            "filelist",
            LayoutNavEntry::FileList,
            theTranslator->trFileList(),
            "",
            theTranslator->trFileListDescription(extractAll),
            "files"
         },

         {
            "globals",
            LayoutNavEntry::FileGlobals,
            theTranslator->trFileMembers(),
            "",
            theTranslator->trFileMembersDescription(extractAll),
            "globals"
         },

         {
            "filesource",
            LayoutNavEntry::FileSource,
            theTranslator->trFileSource(),
            "",
            theTranslator->trFileSourceDescription(),
            "filesource"
         },

         {
            "examples",
            LayoutNavEntry::Examples,
            theTranslator->trExamples(),
            "",
            theTranslator->trExamplesDescription(),
            "examples"
         },

         {
            "user",
            LayoutNavEntry::User,
            "",
            "",
            "",
            "user"
         },

         {
            "usergroup",
            LayoutNavEntry::UserGroup,
            "",
            "",
            "",
            "usergroup"
         },

         {
            "",                            // end of list
            (LayoutNavEntry::Kind)0,
            "",
            "",
            "",
            ""
         }
      };

      LayoutNavEntry::Kind tKind;

      // find type in the table
      int i = 0;
      QString type = attrib.value("type");

      while (! mapping[i].key.isEmpty()) {

         if (mapping[i].key == type) {
            tKind = mapping[i].kind;
            break;
         }

         i++;
      }

      if (mapping[i].key.isEmpty()) {

         if (type.isEmpty()) {
            err("Entry tag within a navindex has no type attribute, verify the project layout file\n");

         } else {
            err("Type '%s' is not supported for the entry tag within a navindex, verify the project layout file\n",
                              csPrintable(type));
         }

         m_invalidEntry = true;
         return;
      }

      QString baseFile = mapping[i].baseFile;
      QString title    = attrib.value("title");

      bool isVisible   = elemIsVisible(attrib);

      if (title.isEmpty()) {
         title = mapping[i].mainName;       // use title for main row

         if (m_rootNav != LayoutDocManager::instance().rootNavEntry() && ! mapping[i].subName.isEmpty()) {
            title = mapping[i].subName;

            // if this is a child of another row, use the subName if available
         }
      }

      QString intro = attrib.value("intro");

      if (intro.isEmpty()) {
         // use default intro text
         intro = mapping[i].intro;
      }

      QString url = attrib.value("url");
      if (mapping[i].kind == LayoutNavEntry::User && ! url.isEmpty()) {
         baseFile = url;

      } else if (tKind == LayoutNavEntry::UserGroup) {

         if (! url.isEmpty()) {
            baseFile = url;

         } else {
            baseFile += QString("%1").formatArg(m_userGroupCount++);

         }
      }

      // create new item and make it the new root
      LayoutNavEntry *tmpLayout = new LayoutNavEntry(m_rootNav, tKind, isVisible, baseFile, title, intro);
      m_rootNav->addChild(tmpLayout);
      m_rootNav = tmpLayout;
   }

   void endNavEntry() {
      // set the root back to the parent
      if (m_rootNav && ! m_invalidEntry) {
         m_rootNav = m_rootNav->parent();
      }

      m_invalidEntry = false;
   }

   void startClass(const QXmlAttributes &) {
      LayoutDocManager::instance().clear(LayoutDocManager::Class);
      m_scope = "class/";
      m_part = (int)LayoutDocManager::Class;
   }

   void endClass() {
      m_scope = "";
      m_part = -1;
   }

   void startNamespace(const QXmlAttributes &) {
      LayoutDocManager::instance().clear(LayoutDocManager::Namespace);
      m_scope = "namespace/";
      m_part = (int)LayoutDocManager::Namespace;
   }

   void endNamespace() {
      m_scope = "";
      m_part = -1;
   }

   void startFile(const QXmlAttributes &) {
      LayoutDocManager::instance().clear(LayoutDocManager::File);
      m_scope = "file/";
      m_part = (int)LayoutDocManager::File;
   }

   void endFile() {
      m_scope = "";
      m_part = -1;
   }

   void startGroup(const QXmlAttributes &) {
      LayoutDocManager::instance().clear(LayoutDocManager::Group);
      m_scope = "group/";
      m_part = (int)LayoutDocManager::Group;
   }

   void endGroup() {
      m_scope = "";
      m_part = -1;
   }

   void startDirectory(const QXmlAttributes &) {
      LayoutDocManager::instance().clear(LayoutDocManager::Directory);
      m_scope = "directory/";
      m_part = (int)LayoutDocManager::Directory;
   }

   void endDirectory() {
      m_scope = "";
      m_part = -1;
   }

   void startMemberDef(const QXmlAttributes &) {
      m_scope += "memberdef/";

      if (m_part != -1) {
         LayoutDocManager::instance().addEntry((LayoutDocManager::LayoutPart)m_part,
                  new LayoutDocEntrySimple(LayoutDocEntry::MemberDefStart));
      }
   }

   void endMemberDef() {
      int i = m_scope.lastIndexOf("memberdef/");

      if (i != -1) {
         m_scope = m_scope.left(i);

         if (m_part != -1) {
            LayoutDocManager::instance().addEntry((LayoutDocManager::LayoutPart)m_part,
                  new LayoutDocEntrySimple(LayoutDocEntry::MemberDefEnd));
         }
      }
   }

   void startMemberDecl(const QXmlAttributes &) {
      m_scope += "memberdecl/";
      if (m_part != -1) {
         LayoutDocManager::instance().addEntry((LayoutDocManager::LayoutPart)m_part,
                  new LayoutDocEntrySimple(LayoutDocEntry::MemberDeclStart));
      }
   }

   void endMemberDecl() {
      int i = m_scope.lastIndexOf("memberdecl/");

      if (i != -1) {
         m_scope = m_scope.left(i);

         if (m_part != -1) {
            LayoutDocManager::instance().addEntry((LayoutDocManager::LayoutPart)m_part,
                  new LayoutDocEntrySimple(LayoutDocEntry::MemberDeclEnd));
         }
      }
   }

   // reimplemented from QXmlDefaultHandler
   bool startElement(const QString &, const QString &, const QString &name, const QXmlAttributes &attrib ) override {

      QString temp = m_scope + name;
      QSharedPointer<StartElement> handler = m_sHandler[temp];

      if (handler) {
         (*handler)(attrib);

      } else {
         err("Unable to process layout file, XML tag '%s' was found in scope: '%s', \n",
                     csPrintable(name), csPrintable(m_scope));
      }

      return true;
   }

   bool endElement( const QString &, const QString &, const QString &name ) override {
      QSharedPointer<EndElement> handler;

      if (! m_scope.isEmpty() && m_scope.right(name.length() + 1 ) == name + "/") {
        // element ends current scope
        handler = m_eHandler[m_scope.left(m_scope.length()-1)];

      }  else  {
         // continue with current scope
        handler = m_eHandler[m_scope + name];
      }

      if (handler) {
        (*handler)();
      }

      return true;
   }

   bool startDocument() override {
      return true;
   }

 private:
   LayoutParser() :  m_invalidEntry(false) { }

   QHash<QString, QSharedPointer<StartElement>> m_sHandler;
   QHash<QString, QSharedPointer<EndElement>>   m_eHandler;
   QString m_scope;

   int             m_part;
   LayoutNavEntry *m_rootNav;
   bool            m_invalidEntry;
   static int      m_userGroupCount;
};

int LayoutParser::m_userGroupCount = 0;

class LayoutErrorHandler : public QXmlErrorHandler
{
 public:
   LayoutErrorHandler(const QString &fn)
      : fileName(fn)
   {}

   bool warning( const QXmlParseException &exception ) override {
      warn_uncond("at line %d column %d of %s: %s\n",
                  exception.lineNumber(), exception.columnNumber(), csPrintable(fileName), csPrintable(exception.message()));
      return false;
   }

   bool error( const QXmlParseException &exception ) override {
      err("at line %d column %d of %s: %s\n",
          exception.lineNumber(), exception.columnNumber(), csPrintable(fileName), csPrintable(exception.message()));
      return false;
   }

   bool fatalError( const QXmlParseException &exception ) override {
      err("at line %d column %d of %s: %s\n",
          exception.lineNumber(), exception.columnNumber(), csPrintable(fileName), csPrintable(exception.message()));
      return false;
   }

   QString errorString() const override {
      return QString("");
   }

 private:
   QString errorMsg;
   QString fileName;
};

static QString getLayout_Default()
{
   QString fileName = ":/resources/layout_default.xml";
   QFile f(fileName);

   if (! f.open(QFile::ReadOnly | QFile::Text)) {
      err("Unable to open file %s, error: %d\n", csPrintable(fileName), f.error());
      return QString("");
   }

   return f.readAll();
}

LayoutDocManager::LayoutDocManager()
{
   m_docRootNav = new LayoutNavEntry;
   LayoutParser::instance().init();
}

void LayoutDocManager::init()
{
   // parse the default layout
   LayoutErrorHandler errorHandler("layout_default.xml");

   QString layoutData = getLayout_Default();

   QXmlInputSource source;
   source.setData(layoutData);

   QXmlSimpleReader reader;

   reader.setContentHandler( &LayoutParser::instance() );
   reader.setErrorHandler( &errorHandler );
   reader.parse( source );
}

LayoutDocManager::~LayoutDocManager()
{
   delete m_docRootNav;
}

LayoutDocManager &LayoutDocManager::instance()
{
   static LayoutDocManager *theInstance = new LayoutDocManager;
   return *theInstance;
}

const QList<LayoutDocEntry *> &LayoutDocManager::docEntries(LayoutDocManager::LayoutPart part) const
{
   switch (part)  {
      case LayoutPart::Class:
         return m_docClass;

      case LayoutPart::Namespace:
         return m_docNamespace;

      case LayoutPart::File:
         return m_docFile;

      case LayoutPart::Group:
         return m_docGroup;

      case LayoutPart::Directory:
         return m_docDirectory;

      default:
         return m_docClass;
   }
}

LayoutNavEntry *LayoutDocManager::rootNavEntry() const
{
   return m_docRootNav;
}

void LayoutDocManager::addEntry(LayoutDocManager::LayoutPart part, LayoutDocEntry *e)
{
   switch (part)  {
      case LayoutPart::Class:
         m_docClass.append(e);

      case LayoutPart::Namespace:
         m_docNamespace.append(e);

      case LayoutPart::File:
         m_docFile.append(e);

      case LayoutPart::Group:
         m_docGroup.append(e);

      case LayoutPart::Directory:
         m_docDirectory.append(e);

      default:
        // ignore entry
        return;
   }
}

void LayoutDocManager::clear(LayoutDocManager::LayoutPart part)
{
   switch (part)  {
      case LayoutPart::Class:
         m_docClass.clear();

      case LayoutPart::Namespace:
         m_docNamespace.clear();

      case LayoutPart::File:
         m_docFile.clear();

      case LayoutPart::Group:
         m_docGroup.clear();

      case LayoutPart::Directory:
         m_docDirectory.clear();

      default:
         // ignore entry
        return;
   }
}

void LayoutDocManager::parse(QTextStream &t, const QString &fileName)
{
   LayoutErrorHandler errorHandler(fileName);

   QString data = t.readAll();

   QXmlInputSource source;
   source.setData(data);

   QXmlSimpleReader reader;
   reader.setContentHandler( &LayoutParser::instance() );
   reader.setErrorHandler( &errorHandler );
   reader.parse( source );
}

void writeDefaultLayoutFile(const QString &fileName)
{
   QFile f;
   bool ok = openOutputFile(fileName, f);

   if (! ok) {
      err("Unable to open file for writing %s, error: %d\n", csPrintable(fileName), f.error());
      return;
   }

   QString layoutData = getLayout_Default();
   layoutData.replace("$doxypressversion", versionString);
   layoutData.replace("$doxygenversion",   versionString);         // compatibility

   QTextStream t(&f);
   t << layoutData;

   printf("DoxyPress layout file generated: %s\n\n", csPrintable( f.fileName() ));
}

// Convert input to a title
// format of input can be a simple title "A title" or in case there are different
// titles for some programming languages they can take the following form:
//    "A title|16=Another title|8=Yet Another title"
//    where the number is a value of SrcLangExt in decimal notation (i.e. 16=Java, 8=IDL).

QString extractLanguageSpecificTitle(const QString &input, SrcLangExt lang)
{
   int i;
   int s = 0;
   int e = input.indexOf('|');

   if (e == -1) {
      return input;   // simple title case
   }

   int e1 = e;

   while (e != -1) { // look for 'number=title' pattern separated by '|'
      s = e + 1;

      e = input.indexOf('|', s);
      i = input.indexOf('=', s);

      assert(i > s);

      int key = input.mid(s, i - s).toInteger<int>();

      if (key == (int)lang) { // found matching key
         if (e == -1) {
            e = input.length();
         }
         return input.mid(i + 1, e - i - 1);
      }
   }

   return input.left(e1); // fallback, no explicit language key found
}


QString LayoutDocEntrySection::title(SrcLangExt lang) const
{
   return extractLanguageSpecificTitle(m_title, lang);
}

QString LayoutDocEntryMemberDecl::title(SrcLangExt lang) const
{
   return extractLanguageSpecificTitle(m_title, lang);
}

QString LayoutDocEntryMemberDecl::subtitle(SrcLangExt lang) const
{
   return extractLanguageSpecificTitle(m_subscript, lang);
}

QString LayoutDocEntryMemberDef::title(SrcLangExt lang) const
{
   return extractLanguageSpecificTitle(m_title, lang);
}

