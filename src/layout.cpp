/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * All rights reserved.    
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License version 2
 * is hereby granted. No representations are made about the suitability of
 * this software for any purpose. It is provided "as is" without express or
 * implied warranty. See the GNU General Public License for more details.
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

#include <config.h>
#include <doxy_build_info.h>
#include <doxy_globals.h>
#include <layout.h>
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
   LayoutNavEntry *result = 0;
  
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
         (kind() == LayoutNavEntry::UserGroup && url.left(9) == "usergroup")) {

      url += Doxy_Globals::htmlFileExtension;

   } else if (url.left(5) == "@ref " || url.left(5) == "\\ref ") {
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
         QString temp = Config::getString("layout-file");
         msg("Explicit link request to '%s' in layout file '%s' could not be resolved\n", qPrintable(url.mid(5)), qPrintable(temp));
      }
   }
  
   return url;
}

class LayoutParser : public QXmlDefaultHandler
{
 private:
   class StartElementHandler 
   {
    typedef void (LayoutParser::*Handler)(const QXmlAttributes &attrib);

    public:
      StartElementHandler(LayoutParser *parent, Handler h)
         : m_parent(parent), m_handler(h) {}

      virtual ~StartElementHandler() {}

      virtual void operator()(const QXmlAttributes &attrib) {
         (m_parent->*m_handler)(attrib);
      }

    protected:
      StartElementHandler() : m_parent(0), m_handler(0) {}

    private:
      LayoutParser *m_parent;
      Handler m_handler;
   };

   class StartElementHandlerKind : public StartElementHandler
   {
    
    typedef void (LayoutParser::*Handler)(LayoutDocEntry::Kind kind, const QXmlAttributes &attrib);

    public:
      StartElementHandlerKind(LayoutParser *parent, LayoutDocEntry::Kind k, Handler h)
         : m_parent(parent), m_kind(k), m_handler(h) {}

      void operator()(const QXmlAttributes &attrib) override {
         (m_parent->*m_handler)(m_kind, attrib);
      }

    private:
      LayoutParser *m_parent;
      LayoutDocEntry::Kind m_kind;
      Handler m_handler;
   };

   class StartElementHandlerSection : public StartElementHandler
   {
     typedef void (LayoutParser::*Handler)(LayoutDocEntry::Kind kind, const QXmlAttributes &attrib, const QString &title);

     public:
      StartElementHandlerSection(LayoutParser *parent, LayoutDocEntry::Kind k, Handler h, const QString &title)
         : m_parent(parent), m_kind(k), m_handler(h), m_title(title) {}

      void operator()(const QXmlAttributes &attrib) {
         (m_parent->*m_handler)(m_kind, attrib, m_title);
      }

    private:
      LayoutParser *m_parent;
      LayoutDocEntry::Kind m_kind;
      Handler m_handler;
      QString m_title;
   };

   class StartElementHandlerMember : public StartElementHandler
   {  
     using Handler = void (LayoutParser::*)(const QXmlAttributes &attrib, MemberListType type, 
                  const QString &title, const QString &subtitle);     

     public:
       StartElementHandlerMember(LayoutParser *parent, Handler h, MemberListType type,
                  const QString &tl, const QString &ss = QString() )
         : m_parent(parent), m_handler(h), m_type(type), m_title(tl), m_subscript(ss) {}

       void operator()(const QXmlAttributes &attrib) {
          (m_parent->*m_handler)(attrib, m_type, m_title, m_subscript);
      }

     private:
      LayoutParser *m_parent;
      Handler m_handler;
      MemberListType m_type;
      QString m_title;
      QString m_subscript;
   };

   class StartElementHandlerNavEntry : public StartElementHandler
   {

     typedef void (LayoutParser::*Handler)(LayoutNavEntry::Kind kind, const QXmlAttributes &attrib, const QString &title);

     public:
        StartElementHandlerNavEntry(LayoutParser *parent, LayoutNavEntry::Kind kind, Handler h, const QString &tl )
           : m_parent(parent), m_kind(kind), m_handler(h), m_title(tl) {}

        void operator()(const QXmlAttributes &attrib) {
           (m_parent->*m_handler)(m_kind, attrib, m_title);
        }

     private:
       LayoutParser *m_parent;
       LayoutNavEntry::Kind m_kind;
       Handler m_handler;
       QString m_title;
   };

   class EndElementHandler
   {

     typedef void (LayoutParser::*Handler)();

     public:
      EndElementHandler(LayoutParser *parent, Handler h) : m_parent(parent), m_handler(h) {}
      void operator()() {
         (m_parent->*m_handler)();
      }

     private:
      LayoutParser *m_parent;
      Handler m_handler;
   };


 public:
   static LayoutParser &instance() {
      static LayoutParser *theInstance = new LayoutParser;
      return *theInstance;
   }

   void init() {     
      m_part    = -1;
      m_rootNav = 0;

      // bool fortranOpt = Config::getBool("optimize-fortran");     
      // bool javaOpt    = Config::getBool("optimize-java");

      // start & end handlers
      m_sHandler.insert("doxypress-layout",          new StartElementHandler(this, &LayoutParser::startLayout));
      m_eHandler.insert("doxypress-layout",          new EndElementHandler(this, &LayoutParser::endLayout));

      // class layout handlers
      m_sHandler.insert("navindex",                  new StartElementHandler(this, &LayoutParser::startNavIndex));
      m_sHandler.insert("navindex/tab",              new StartElementHandler(this, &LayoutParser::startNavEntry));
      m_eHandler.insert("navindex/tab",              new EndElementHandler(this, &LayoutParser::endNavEntry));
      m_eHandler.insert("navindex",                  new EndElementHandler(this, &LayoutParser::endNavIndex));

      // class layout handlers
      m_sHandler.insert("class",                     new StartElementHandler(this, &LayoutParser::startClass));
      m_sHandler.insert("class/briefdescription",    new StartElementHandlerKind(
               this, LayoutDocEntry::BriefDesc, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/detaileddescription", new StartElementHandlerSection(
               this, LayoutDocEntry::DetailedDesc, &LayoutParser::startSectionEntry,
               theTranslator->trDetailedDescription()));

      m_sHandler.insert("class/authorsection",       new StartElementHandlerKind(
               this, LayoutDocEntry::AuthorSection, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/includes",            new StartElementHandlerKind(
               this, LayoutDocEntry::ClassIncludes, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/inheritancegraph",    new StartElementHandlerKind(
               this, LayoutDocEntry::ClassInheritanceGraph, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/collaborationgraph",  new StartElementHandlerKind(
               this, LayoutDocEntry::ClassCollaborationGraph, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/allmemberslink",      new StartElementHandlerKind(
               this, LayoutDocEntry::ClassAllMembersLink, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/usedfiles",           new StartElementHandlerKind(
               this, LayoutDocEntry::ClassUsedFiles, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/memberdecl",                   new StartElementHandler(this, &LayoutParser::startMemberDecl));
         
      m_sHandler.insert("class/memberdecl/nestedclasses",     new StartElementHandlerSection(this, LayoutDocEntry::ClassNestedClasses,
               &LayoutParser::startSectionEntry, 
               buildMultiTitle(theTranslator->trCompounds(), SrcLangExt_Fortran, theTranslator->trDataTypes())) ); 
     
      m_sHandler.insert("class/memberdecl/services",          new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_services, theTranslator->trServices()));

      m_sHandler.insert("class/memberdecl/interfaces",        new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_interfaces, theTranslator->trInterfaces()));


      m_sHandler.insert("class/memberdecl/publictypedefs",    new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pubTypedefs, theTranslator->trPublicTypedefs()));
      
      m_sHandler.insert("class/memberdecl/protectedtypedefs", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proTypedefs, theTranslator->trProtectedTypedefs()));

      m_sHandler.insert("class/memberdecl/privatetypedefs",   new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priTypedefs, theTranslator->trPrivateTypedefs()));


      m_sHandler.insert("class/memberdecl/publictypes",       new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pubTypes, theTranslator->trPublicTypes()));

      m_sHandler.insert("class/memberdecl/publicslots",       new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pubSlots, theTranslator->trPublicSlots()));

      m_sHandler.insert("class/memberdecl/publicsignals",     new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pubSignals, theTranslator->trPublicSignals()));

      m_sHandler.insert("class/memberdecl/publicmethods",     new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry, 
               MemberListType_pubMethods, 
               buildMultiTitle(theTranslator->trPublicMembers(), SrcLangExt_ObjC, theTranslator->trInstanceMethods())));

      m_sHandler.insert("class/memberdecl/publicstaticmethods",    new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pubStaticMethods, 
               buildMultiTitle( theTranslator->trStaticPublicMembers(), SrcLangExt_ObjC, theTranslator->trClassMethods())));

      m_sHandler.insert("class/memberdecl/publicattributes",       new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pubAttribs, theTranslator->trPublicAttribs()));

      m_sHandler.insert("class/memberdecl/publicstaticattributes", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pubStaticAttribs, theTranslator->trStaticPublicAttribs()));

      m_sHandler.insert("class/memberdecl/protectedtypes",    new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proTypes, theTranslator->trProtectedTypes()));

      m_sHandler.insert("class/memberdecl/protectedslots",    new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proSlots, theTranslator->trProtectedSlots()));

     m_sHandler.insert("class/memberdecl/protectedsignals",   new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proSignals, theTranslator->trProtectedSignals()));

      m_sHandler.insert("class/memberdecl/protectedmethods",  new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proMethods, theTranslator->trProtectedMembers()));

      m_sHandler.insert("class/memberdecl/protectedstaticmethods", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proStaticMethods, theTranslator->trStaticProtectedMembers()));

      m_sHandler.insert("class/memberdecl/protectedattributes", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proAttribs, theTranslator->trProtectedAttribs()));

      m_sHandler.insert("class/memberdecl/protectedstaticattributes", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_proStaticAttribs, theTranslator->trStaticProtectedAttribs()));

      m_sHandler.insert("class/memberdecl/packagetypes", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pacTypes, theTranslator->trPackageTypes()));

      m_sHandler.insert("class/memberdecl/packagemethods", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pacMethods, theTranslator->trPackageMembers()));

      m_sHandler.insert("class/memberdecl/packagestaticmethods", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pacStaticMethods, theTranslator->trStaticPackageMembers()));

      m_sHandler.insert("class/memberdecl/packageattributes", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pacAttribs, theTranslator->trPackageAttribs()));

      m_sHandler.insert("class/memberdecl/packagestaticattributes", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_pacStaticAttribs, theTranslator->trStaticPackageAttribs()));

      m_sHandler.insert("class/memberdecl/properties", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_properties, theTranslator->trProperties()));

      m_sHandler.insert("class/memberdecl/events", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_events, theTranslator->trEvents()));

      m_sHandler.insert("class/memberdecl/privatetypes", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priTypes, theTranslator->trPrivateTypes()));

      m_sHandler.insert("class/memberdecl/privateslots", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priSlots, theTranslator->trPrivateSlots()));

      m_sHandler.insert("class/memberdecl/privatesignals",  new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priSignals, theTranslator->trPrivateSignals()));

      m_sHandler.insert("class/memberdecl/privatemethods", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priMethods, theTranslator->trPrivateMembers()));

      m_sHandler.insert("class/memberdecl/privatestaticmethods", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priStaticMethods, theTranslator->trStaticPrivateMembers()));

      m_sHandler.insert("class/memberdecl/privateattributes", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priAttribs, theTranslator->trPrivateAttribs()));

      m_sHandler.insert("class/memberdecl/privatestaticattributes", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_priStaticAttribs, theTranslator->trStaticPrivateAttribs()));

      m_sHandler.insert("class/memberdecl/friends", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_friends, theTranslator->trFriends()));

      m_sHandler.insert("class/memberdecl/related", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_related, theTranslator->trRelatedFunctions(), theTranslator->trRelatedSubscript()));

      m_sHandler.insert("class/memberdecl/membergroups",  new StartElementHandlerKind(
               this, LayoutDocEntry::MemberGroups, &LayoutParser::startSimpleEntry));

      m_eHandler.insert("class/memberdecl", new EndElementHandler(this, &LayoutParser::endMemberDecl));

      m_sHandler.insert("class/memberdef",  new StartElementHandler(this, &LayoutParser::startMemberDef));

      m_sHandler.insert("class/memberdef/inlineclasses", new StartElementHandlerSection(this, LayoutDocEntry::ClassInlineClasses, 
               &LayoutParser::startSectionEntry, 
               buildMultiTitle(theTranslator->trClassDocumentation(), SrcLangExt_Fortran, theTranslator->trTypeDocumentation() )));

      m_sHandler.insert("class/memberdef/typedefs", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_typedefMembers, theTranslator->trMemberTypedefDocumentation()));

      m_sHandler.insert("class/memberdef/enums", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_enumMembers, theTranslator->trMemberEnumerationDocumentation()));

      m_sHandler.insert("class/memberdef/services", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_serviceMembers, theTranslator->trInterfaces()));

      m_sHandler.insert("class/memberdef/interfaces", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_interfaceMembers, theTranslator->trInterfaces()));

      m_sHandler.insert("class/memberdef/constructors", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_constructors, theTranslator->trConstructorDocumentation()));

      m_sHandler.insert("class/memberdef/functions", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_functionMembers, buildMultiTitle(theTranslator->trMemberFunctionDocumentation(),
               SrcLangExt_ObjC, theTranslator->trMethodDocumentation(), 
               SrcLangExt_Fortran, theTranslator->trMemberFunctionDocumentationFortran() )));

      m_sHandler.insert("class/memberdef/related", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_relatedMembers, theTranslator->trRelatedFunctionDocumentation()));

      m_sHandler.insert("class/memberdef/variables", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_variableMembers, theTranslator->trMemberDataDocumentation()));

      m_sHandler.insert("class/memberdef/properties", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_propertyMembers, theTranslator->trPropertyDocumentation()));

      m_sHandler.insert("class/memberdef/events", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_eventMembers, theTranslator->trEventDocumentation()));

      m_eHandler.insert("class/memberdef", new EndElementHandler(this, &LayoutParser::endMemberDef));

      m_eHandler.insert("class", new EndElementHandler(this, &LayoutParser::endClass));


      // namespace layout handlers
      m_sHandler.insert("namespace", new StartElementHandler(this, &LayoutParser::startNamespace));
      m_sHandler.insert("namespace/briefdescription", new StartElementHandlerKind(this, 
               LayoutDocEntry::BriefDesc, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("namespace/detaileddescription", new StartElementHandlerSection(this, LayoutDocEntry::DetailedDesc,
               &LayoutParser::startSectionEntry, theTranslator->trDetailedDescription()));

      m_sHandler.insert("namespace/authorsection", new StartElementHandlerKind(this, LayoutDocEntry::AuthorSection,
               &LayoutParser::startSimpleEntry));

      m_sHandler.insert("namespace/memberdecl", new StartElementHandler(this, &LayoutParser::startMemberDecl));

      m_sHandler.insert("namespace/memberdecl/nestednamespaces", new StartElementHandlerSection(this, 
               LayoutDocEntry::NamespaceNestedNamespaces,
               &LayoutParser::startSectionEntry, buildMultiTitle( theTranslator->trNamespaces(),
               SrcLangExt_Java, theTranslator->trPackages(), SrcLangExt_IDL, theTranslator->trModules(), 
               SrcLangExt_Fortran, theTranslator->trModules() )));

      m_sHandler.insert("namespace/memberdecl/constantgroups", new StartElementHandlerSection(this, 
               LayoutDocEntry::NamespaceNestedConstantGroups, &LayoutParser::startSectionEntry, theTranslator->trConstantGroups()));

      m_sHandler.insert("namespace/memberdecl/classes", new StartElementHandlerSection(this, LayoutDocEntry::NamespaceClasses, 
               &LayoutParser::startSectionEntry, buildMultiTitle(theTranslator->trCompounds(),                              
               SrcLangExt_Fortran, theTranslator->trDataTypes() )));

      m_sHandler.insert("namespace/memberdecl/membergroups", new StartElementHandlerKind(this, LayoutDocEntry::MemberGroups, 
               &LayoutParser::startSimpleEntry));

      m_sHandler.insert("namespace/memberdecl/typedefs", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decTypedefMembers, theTranslator->trTypedefs()));

      m_sHandler.insert("namespace/memberdecl/enums", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decEnumMembers, theTranslator->trEnumerations()));

      m_sHandler.insert("namespace/memberdecl/functions", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decFuncMembers, buildMultiTitle( theTranslator->trFunctions(),
               SrcLangExt_Fortran, theTranslator->trSubprograms() )));

      m_sHandler.insert("namespace/memberdecl/variables", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decVarMembers, theTranslator->trVariables()));

      m_eHandler.insert("namespace/memberdecl", new EndElementHandler(this, &LayoutParser::endMemberDecl));
      m_sHandler.insert("namespace/memberdef",  new StartElementHandler(this, &LayoutParser::startMemberDef));

      m_sHandler.insert("namespace/memberdef/inlineclasses", new StartElementHandlerSection(this, LayoutDocEntry::NamespaceInlineClasses,
               &LayoutParser::startSectionEntry, buildMultiTitle(theTranslator->trClassDocumentation(),
               SrcLangExt_Fortran, theTranslator->trTypeDocumentation() )));

      m_sHandler.insert("namespace/memberdef/typedefs", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docTypedefMembers, theTranslator->trTypedefDocumentation()));

      m_sHandler.insert("namespace/memberdef/enums", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docEnumMembers, theTranslator->trEnumerationTypeDocumentation()));

      m_sHandler.insert("namespace/memberdef/functions", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docFuncMembers, buildMultiTitle( theTranslator->trFunctionDocumentation(),
               SrcLangExt_Fortran, theTranslator->trSubprogramDocumentation() )));

      m_sHandler.insert("namespace/memberdef/variables", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docVarMembers, theTranslator->trVariableDocumentation()));

      m_eHandler.insert("namespace/memberdef", new EndElementHandler(this, &LayoutParser::endMemberDef));
      m_eHandler.insert("namespace", new EndElementHandler(this, &LayoutParser::endNamespace));

      // file layout handlers
      m_sHandler.insert("file", new StartElementHandler(this, &LayoutParser::startFile));
      m_sHandler.insert("file/briefdescription", new StartElementHandlerKind(this, LayoutDocEntry::BriefDesc, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("file/detaileddescription", new StartElementHandlerSection(this, 
               LayoutDocEntry::DetailedDesc, &LayoutParser::startSectionEntry,theTranslator->trDetailedDescription()));

      m_sHandler.insert("file/authorsection", new StartElementHandlerKind(this, LayoutDocEntry::AuthorSection, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("file/includes", new StartElementHandlerKind(this, LayoutDocEntry::FileIncludes, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("file/includegraph", new StartElementHandlerKind(this, LayoutDocEntry::FileIncludeGraph, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("file/includedbygraph", new StartElementHandlerKind(this, LayoutDocEntry::FileIncludedByGraph,
               &LayoutParser::startSimpleEntry));

      m_sHandler.insert("file/sourcelink", new StartElementHandlerKind(this, LayoutDocEntry::FileSourceLink, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("file/memberdecl/membergroups", new StartElementHandlerKind(this, LayoutDocEntry::MemberGroups,
               &LayoutParser::startSimpleEntry));

      m_sHandler.insert("file/memberdecl", new StartElementHandler(this, &LayoutParser::startMemberDecl));

      m_sHandler.insert("file/memberdecl/classes", new StartElementHandlerSection(this, LayoutDocEntry::FileClasses,
               &LayoutParser::startSectionEntry,
               buildMultiTitle(theTranslator->trCompounds(), SrcLangExt_Fortran, theTranslator->trDataTypes() )));

      m_sHandler.insert("file/memberdecl/namespaces", new StartElementHandlerSection(this, LayoutDocEntry::FileNamespaces, 
               &LayoutParser::startSectionEntry, buildMultiTitle( theTranslator->trNamespaces(), SrcLangExt_Java,
               theTranslator->trPackages(), SrcLangExt_IDL, theTranslator->trModules(), 
               SrcLangExt_Fortran, theTranslator->trModules() )));

      m_sHandler.insert("file/memberdecl/constantgroups", new StartElementHandlerSection(this, LayoutDocEntry::FileConstantGroups,
               &LayoutParser::startSectionEntry, theTranslator->trConstantGroups()));

      m_sHandler.insert("file/memberdecl/defines", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decDefineMembers, theTranslator->trDefines()));

      m_sHandler.insert("file/memberdecl/typedefs", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decTypedefMembers, theTranslator->trTypedefs()));

      m_sHandler.insert("file/memberdecl/enums", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decEnumMembers, theTranslator->trEnumerations()));

      m_sHandler.insert("file/memberdecl/functions", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decFuncMembers, 
               buildMultiTitle( theTranslator->trFunctions(), SrcLangExt_Fortran, theTranslator->trSubprograms() )));

      m_sHandler.insert("file/memberdecl/variables", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decVarMembers, theTranslator->trVariables()));

      m_eHandler.insert("file/memberdecl", new EndElementHandler(this, &LayoutParser::endMemberDecl));
      m_sHandler.insert("file/memberdef",  new StartElementHandler(this, &LayoutParser::startMemberDef));

      m_sHandler.insert("file/memberdef/inlineclasses", new StartElementHandlerSection(this, LayoutDocEntry::FileInlineClasses, 
               &LayoutParser::startSectionEntry,
               buildMultiTitle( theTranslator->trClassDocumentation(), SrcLangExt_Fortran, theTranslator->trTypeDocumentation() )));

      m_sHandler.insert("file/memberdef/defines", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docDefineMembers, theTranslator->trDefineDocumentation()));

      m_sHandler.insert("file/memberdef/typedefs", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docTypedefMembers, theTranslator->trTypedefDocumentation()));

      m_sHandler.insert("file/memberdef/enums", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docEnumMembers, theTranslator->trEnumerationTypeDocumentation()));

      m_sHandler.insert("file/memberdef/functions", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docFuncMembers, buildMultiTitle(theTranslator->trFunctionDocumentation(),
               SrcLangExt_Fortran, theTranslator->trSubprogramDocumentation() )));

      m_sHandler.insert("file/memberdef/variables", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docVarMembers, theTranslator->trVariableDocumentation()));

      m_eHandler.insert("file/memberdef", new EndElementHandler(this, &LayoutParser::endMemberDef));
      m_eHandler.insert("file", new EndElementHandler(this, &LayoutParser::endFile));

      // group layout handlers
      m_sHandler.insert("group", new StartElementHandler(this, &LayoutParser::startGroup));
      m_sHandler.insert("group/briefdescription", new StartElementHandlerKind(this, LayoutDocEntry::BriefDesc, 
               &LayoutParser::startSimpleEntry));

      m_sHandler.insert("group/detaileddescription", new StartElementHandlerSection(this, LayoutDocEntry::DetailedDesc,
               &LayoutParser::startSectionEntry, theTranslator->trDetailedDescription()));

      m_sHandler.insert("group/authorsection", new StartElementHandlerKind(this, LayoutDocEntry::AuthorSection, 
               &LayoutParser::startSimpleEntry));

      m_sHandler.insert("group/groupgraph", new StartElementHandlerKind(this, LayoutDocEntry::GroupGraph, 
               &LayoutParser::startSimpleEntry));

      m_sHandler.insert("group/memberdecl/membergroups", new StartElementHandlerKind(this, LayoutDocEntry::MemberGroups,
               &LayoutParser::startSimpleEntry));

      m_sHandler.insert("group/memberdecl", new StartElementHandler(this, &LayoutParser::startMemberDecl));

      m_sHandler.insert("group/memberdecl/classes", new StartElementHandlerSection(this, LayoutDocEntry::GroupClasses,
               &LayoutParser::startSectionEntry,
               buildMultiTitle( theTranslator->trCompounds(), SrcLangExt_Fortran, theTranslator->trDataTypes() )));

      m_sHandler.insert("group/memberdecl/namespaces", new StartElementHandlerSection(this, LayoutDocEntry::GroupNamespaces,
               &LayoutParser::startSectionEntry, buildMultiTitle( theTranslator->trNamespaces(),
               SrcLangExt_Java, theTranslator->trPackages(), SrcLangExt_Fortran, theTranslator->trModules() )));

      m_sHandler.insert("group/memberdecl/dirs", new StartElementHandlerSection(this, LayoutDocEntry::GroupDirs, 
               &LayoutParser::startSectionEntry, theTranslator->trDirectories() ));

      m_sHandler.insert("group/memberdecl/nestedgroups", new StartElementHandlerSection(this, LayoutDocEntry::GroupNestedGroups, 
               &LayoutParser::startSectionEntry, theTranslator->trModules() ));

      m_sHandler.insert("group/memberdecl/files", new StartElementHandlerSection(this, LayoutDocEntry::GroupFiles, 
               &LayoutParser::startSectionEntry, theTranslator->trFile(true, false) ));

      m_sHandler.insert("group/memberdecl/defines", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decDefineMembers, theTranslator->trDefines()));

      m_sHandler.insert("group/memberdecl/typedefs", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decTypedefMembers, theTranslator->trTypedefs()));

      m_sHandler.insert("group/memberdecl/enums", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decEnumMembers, theTranslator->trEnumerations()));

      m_sHandler.insert("group/memberdecl/enumvalues", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decEnumValMembers, theTranslator->trEnumerationValues()));

      m_sHandler.insert("group/memberdecl/functions", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decFuncMembers, 
               buildMultiTitle(theTranslator->trFunctions(), SrcLangExt_Fortran, theTranslator->trSubprograms() )));

      m_sHandler.insert("group/memberdecl/variables", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decVarMembers, theTranslator->trVariables()));


      m_sHandler.insert("group/memberdecl/publicsignals", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decPubSignalMembers, theTranslator->trPublicSignals()));

     m_sHandler.insert("group/memberdecl/protectedsignals", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decProSignalMembers, theTranslator->trProtectedSignals()));

     m_sHandler.insert("group/memberdecl/privatesignals", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decPriSignalMembers, theTranslator->trPrivateSignals()));    

      m_sHandler.insert("group/memberdecl/publicslots", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decPubSlotMembers, theTranslator->trPublicSlots()));

      m_sHandler.insert("group/memberdecl/protectedslots", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decProSlotMembers, theTranslator->trProtectedSlots()));

      m_sHandler.insert("group/memberdecl/privateslots", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decPriSlotMembers, theTranslator->trPrivateSlots()));


      m_sHandler.insert("group/memberdecl/events", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decEventMembers, theTranslator->trEvents()));

      m_sHandler.insert("group/memberdecl/properties", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decPropMembers, theTranslator->trProperties()));

      m_sHandler.insert("group/memberdecl/friends", new StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
               MemberListType_decFriendMembers, theTranslator->trFriends()));

      m_eHandler.insert("group/memberdecl", new EndElementHandler(this, &LayoutParser::endMemberDecl));
      m_sHandler.insert("group/memberdef",  new StartElementHandler(this, &LayoutParser::startMemberDef));
      m_sHandler.insert("group/memberdef/pagedocs", new StartElementHandlerKind(this, LayoutDocEntry::GroupPageDocs,
               &LayoutParser::startSimpleEntry));

      m_sHandler.insert("group/memberdef/inlineclasses", new StartElementHandlerSection(this, LayoutDocEntry::GroupInlineClasses, 
               &LayoutParser::startSectionEntry, buildMultiTitle(theTranslator->trClassDocumentation(), SrcLangExt_Fortran,
               theTranslator->trTypeDocumentation() )));

      m_sHandler.insert("group/memberdef/defines", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docDefineMembers, theTranslator->trDefineDocumentation()));

      m_sHandler.insert("group/memberdef/typedefs", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docTypedefMembers, theTranslator->trTypedefDocumentation()));

      m_sHandler.insert("group/memberdef/enums", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docEnumMembers, theTranslator->trEnumerationTypeDocumentation()));

      m_sHandler.insert("group/memberdef/enumvalues", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docEnumValMembers, theTranslator->trEnumerationValueDocumentation()));

      m_sHandler.insert("group/memberdef/functions", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docFuncMembers, buildMultiTitle( theTranslator->trFunctionDocumentation(),
               SrcLangExt_Fortran, theTranslator->trSubprogramDocumentation() )));

      m_sHandler.insert("group/memberdef/variables", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docVarMembers, theTranslator->trVariableDocumentation()));


      m_sHandler.insert("group/memberdef/publicsignals",   new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docPubSignalMembers, theTranslator->trPublicSignals()));

      m_sHandler.insert("group/memberdef/protectedsignals", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docProSignalMembers, theTranslator->trProtectedSignals()));

      m_sHandler.insert("group/memberdef/privatesignals",   new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docPriSignalMembers, theTranslator->trPrivateSignals()));

      m_sHandler.insert("group/memberdef/publicslots",    new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docPubSlotMembers, theTranslator->trPublicSlots()));

      m_sHandler.insert("group/memberdef/protectedslots", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docProSlotMembers, theTranslator->trProtectedSlots()));

      m_sHandler.insert("group/memberdef/privateslots",   new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docPriSlotMembers, theTranslator->trPrivateSlots()));


      m_sHandler.insert("group/memberdef/events", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docEventMembers, theTranslator->trEvents()));

      m_sHandler.insert("group/memberdef/properties", new StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docPropMembers, theTranslator->trProperties()));

      m_sHandler.insert("group/memberdef/friends", new  StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
               MemberListType_docFriendMembers, theTranslator->trFriends()));

      m_eHandler.insert("group/memberdef", new EndElementHandler(this, &LayoutParser::endMemberDef));

      m_eHandler.insert("group", new EndElementHandler(this, &LayoutParser::endGroup));

      // directory layout handlers
      m_sHandler.insert("directory", new StartElementHandler(this, &LayoutParser::startDirectory));
      m_sHandler.insert("directory/briefdescription", 
               new StartElementHandlerKind(this, LayoutDocEntry::BriefDesc, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("directory/detaileddescription",
               new StartElementHandlerSection(this, LayoutDocEntry::DetailedDesc, &LayoutParser::startSectionEntry,
               theTranslator->trDetailedDescription()));

      m_sHandler.insert("directory/directorygraph",
               new StartElementHandlerKind(this, LayoutDocEntry::DirGraph, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("directory/memberdecl", new StartElementHandler(this, &LayoutParser::startMemberDecl));

      m_sHandler.insert("directory/memberdecl/dirs",
               new StartElementHandlerKind(this, LayoutDocEntry::DirSubDirs, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("directory/memberdecl/files",
               new StartElementHandlerKind(this, LayoutDocEntry::DirFiles, &LayoutParser::startSimpleEntry));

      m_eHandler.insert("directory/memberdecl",new EndElementHandler(this, &LayoutParser::endMemberDecl));
      m_eHandler.insert("directory", new EndElementHandler(this, &LayoutParser::endDirectory));
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
      m_scope = "navindex/";
      m_rootNav = LayoutDocManager::instance().rootNavEntry();
      if (m_rootNav) {
         m_rootNav->clear();
      }
   }

   void endNavIndex() {
      m_scope = "";

      if (m_rootNav && ! m_rootNav->find(LayoutNavEntry::MainPage)) {

         // no MainPage node... add one as the first item of the root node...
         new LayoutNavEntry(m_rootNav, LayoutNavEntry::MainPage, true, "index", theTranslator->trMainPage(), "");
      }
   }

   struct NavEntryMap {
         const char *typeStr;         // type attribute name in the XML file
         LayoutNavEntry::Kind kind;   // corresponding enum name
         QString mainName;            // default title for an item if it has children
         QString subName;             // optional name for an item if it is rendered as a child
         QString intro;               // introduction text to be put on the index page
         QString baseFile;            // base name of the file containing the index page
   };

   void startNavEntry(const QXmlAttributes &attrib) {
      static bool javaOpt    = Config::getBool("optimize-java");
      static bool fortranOpt = Config::getBool("optimize-fortran");
     
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
            javaOpt  ? theTranslator->trPackages() : fortranOpt ? theTranslator->trModules() : theTranslator->trNamespaces(),
            javaOpt  ? theTranslator->trPackages() : fortranOpt ? theTranslator->trModulesList() : theTranslator->trNamespaceList(),
            javaOpt  ? theTranslator->trPackageListDescription() : fortranOpt ? 
                                 theTranslator->trModulesListDescription(extractAll) : theTranslator->trNamespaceListDescription(extractAll),
            "namespaces"
         },
         {
            "namespacelist",
            LayoutNavEntry::NamespaceList,
            javaOpt ? theTranslator->trPackages() : fortranOpt ? theTranslator->trModulesList() : theTranslator->trNamespaceList(),
            "",
            javaOpt ? theTranslator->trPackageListDescription() : fortranOpt ? 
                                 theTranslator->trModulesListDescription(extractAll) : theTranslator->trNamespaceListDescription(extractAll),
            "namespaces"
         },
         {
            "namespacemembers",
            LayoutNavEntry::NamespaceMembers,
            javaOpt ? theTranslator->trPackageMembers() : fortranOpt ? theTranslator->trModulesMembers() : theTranslator->trNamespaceMembers(),
            "",
            fortranOpt ? theTranslator->trModulesMemberDescription(extractAll) : theTranslator->trNamespaceMemberDescription(extractAll),
            "namespacemembers"
         },
         {
            "classindex",
            LayoutNavEntry::ClassIndex,
            fortranOpt ? theTranslator->trDataTypes() : theTranslator->trCompoundIndex(),
            "",
            "",
            "classes"
         },
         {
            "classes",
            LayoutNavEntry::Classes,
            fortranOpt ? theTranslator->trCompoundListFortran() : theTranslator->trClasses(),
            theTranslator->trCompoundList(),
            fortranOpt ? theTranslator->trCompoundListDescriptionFortran() : theTranslator->trCompoundListDescription(),
            "annotated"
         },
         {
            "classlist",
            LayoutNavEntry::ClassList,
            fortranOpt ? theTranslator->trCompoundListFortran() : theTranslator->trCompoundList(),
            "",
            fortranOpt ? theTranslator->trCompoundListDescriptionFortran() : theTranslator->trCompoundListDescription(),
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
            fortranOpt ? theTranslator->trCompoundMembersFortran() : theTranslator->trCompoundMembers(),
            "",
            fortranOpt ? theTranslator->trCompoundMembersDescriptionFortran(extractAll) : 
                  theTranslator->trCompoundMembersDescription(extractAll), "functions_all"
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
         //{ "dirs",
         //  LayoutNavEntry::Dirs,
         //  theTranslator->trDirectories(),
         //  QByteArray(),
         //  theTranslator->trDirDescription(),
         //  "dirs"
         //},
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
            0, // end of list
            (LayoutNavEntry::Kind)0,
            "",
            "",
            "",
            ""
         }
      };

      LayoutNavEntry::Kind kind;

      // find type in the table
      int i = 0;
      QString type = attrib.value("type");

      while (mapping[i].typeStr) {
         if (mapping[i].typeStr == type) {
            kind = mapping[i].kind;
            break;
         }
         i++;
      }

      if (mapping[i].typeStr == 0) {
         if (type.isEmpty()) {
            err("Entry tag within a navindex has no type attribute, verify the layout file\n");

         } else {
            err("Type '%s' is not supported for the entry tag within a navindex, verify your layout file\n", 
                              qPrintable(type));
         }

         m_invalidEntry = true;
         return;
      }

      QString baseFile = mapping[i].baseFile;
      QString title = attrib.value("title");

      bool isVisible = elemIsVisible(attrib);

      if (title.isEmpty()) { 
         title = mapping[i].mainName; // use title for main row

         if (m_rootNav != LayoutDocManager::instance().rootNavEntry() && !mapping[i].subName.isEmpty()) {
            title = mapping[i].subName; // if this is a child of another row, use the subName if available
            // this is mainly done to get compatible naming with older versions
         }
      }


      QString intro = attrib.value("intro");

      if (intro.isEmpty()) { 
         // use default intro text
         intro = mapping[i].intro;
      }

      QString url = attrib.value("url");
      if (mapping[i].kind == LayoutNavEntry::User && !url.isEmpty()) {
         baseFile = url;

      } else if (kind == LayoutNavEntry::UserGroup) {

         if (! url.isEmpty()) {
            baseFile = url;

         } else {
            baseFile += QString("%1").arg(m_userGroupCount++).toLatin1();

         }
      }

      // create new item and make it the new root
      m_rootNav = new LayoutNavEntry(m_rootNav, kind, isVisible, baseFile, title, intro);
   }

   void endNavEntry() {
      // set the root back to the parent
      if (m_rootNav && !m_invalidEntry) {
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
   bool startElement(const QString &, const QString &, const QString &name, const QXmlAttributes &attrib ) {
      
      QString temp = m_scope + name;      
      StartElementHandler *handler = m_sHandler[temp];

      if (handler) {         
         (*handler)(attrib);             
    
      } else {
         err("Unable to process layout file, XML tag '%s' was found in scope: '%s', \n", 
                     qPrintable(name), qPrintable(m_scope));
      }

      return true;
   }

   bool endElement( const QString &, const QString &, const QString &name ) {           
      EndElementHandler *handler;

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

      return TRUE;
   }

   bool startDocument() {
      return true;
   }

 private:
   LayoutParser() :  m_invalidEntry(false) { }

   QHash<QString, StartElementHandler *> m_sHandler;
   QHash<QString, EndElementHandler *>   m_eHandler;
   QString m_scope;

   int m_part;
   LayoutNavEntry *m_rootNav;
   bool m_invalidEntry;
   static int m_userGroupCount;
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
                  exception.lineNumber(), exception.columnNumber(), qPrintable(fileName), qPrintable(exception.message()));
      return false;
   }

   bool error( const QXmlParseException &exception ) override {
      err("at line %d column %d of %s: %s\n",
          exception.lineNumber(), exception.columnNumber(), qPrintable(fileName), qPrintable(exception.message()));
      return false;
   }

   bool fatalError( const QXmlParseException &exception ) override {
      err("at line %d column %d of %s: %s\n",
          exception.lineNumber(), exception.columnNumber(), qPrintable(fileName), qPrintable(exception.message()));
      return false;
   }

   QString errorString() const override {
      return "";
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
      err("Unable to open file %s, error: %d\n", qPrintable(fileName), f.error());    
      return "";
   }

   return f.readAll();
}

class LayoutDocManager::Private
{
 public:
   QList<LayoutDocEntry *> m_docEntries[LayoutDocManager::NUM_PARTS];
   LayoutNavEntry *rootNav;
};

LayoutDocManager::LayoutDocManager()
{
   d = new Private;

   int i;
 
   d->rootNav = new LayoutNavEntry;
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
   delete d->rootNav;
   delete d;
}

LayoutDocManager &LayoutDocManager::instance()
{
   static LayoutDocManager *theInstance = new LayoutDocManager;
   return *theInstance;
}

const QList<LayoutDocEntry *> &LayoutDocManager::docEntries(LayoutDocManager::LayoutPart part) const
{
   return d->m_docEntries[(int)part];
}

LayoutNavEntry *LayoutDocManager::rootNavEntry() const
{
   return d->rootNav;
}

void LayoutDocManager::addEntry(LayoutDocManager::LayoutPart p, LayoutDocEntry *e)
{
   d->m_docEntries[(int)p].append(e);
}

void LayoutDocManager::clear(LayoutDocManager::LayoutPart p)
{
   d->m_docEntries[(int)p].clear();
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
   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", csPrintable(fileName), f.error());
      return;
   }

   QString layoutData = getLayout_Default();
   layoutData.replace("$doxypressversion", versionString);
   layoutData.replace("$doxygenversion",   versionString);         // compatibility

   QTextStream t(&f);
   t << layoutData;

   printf("DoxyPress layout file generated: %s", csPrintable( f.fileName() )); 
}

// Convert input to a title.
// The format of input can be a simple title "A title" or in case there are different
// titles for some programming languages they can take the following form:
// "A title|16=Another title|8=Yet Another title"
// where the number is a value of SrcLangExt in decimal notation (i.e. 16=Java, 8=IDL).

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

      int key = input.mid(s, i - s).toInt();

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

