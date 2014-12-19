/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch. 
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
 * All rights reserved.    
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License version 2
 * is hereby granted. No representations are made about the suitability of
 * this software for any purpose. It is provided "as is" without express or
 * implied warranty. See the GNU General Public License for more details.
 *
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#include <QByteArray>
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
#include <doxygen.h>
#include <dox_build_info.h>
#include <layout.h>
#include <language.h>
#include <message.h>
#include <util.h>

#define ADD_OPTION(langId,text) "|"+QByteArray().setNum(langId)+"="+text

#define COMPILE_FOR_1_OPTION(def,langId1,text1) \
  def+ADD_OPTION(langId1,text1)

#define COMPILE_FOR_2_OPTIONS(def,langId1,text1,langId2,text2) \
  COMPILE_FOR_1_OPTION(def,langId1,text1)+ADD_OPTION(langId2,text2)

#define COMPILE_FOR_3_OPTIONS(def,langId1,text1,langId2,text2,langId3,text3) \
  COMPILE_FOR_2_OPTIONS(def,langId1,text1,langId2,text2)+ADD_OPTION(langId3,text3)

#define COMPILE_FOR_4_OPTIONS(def,langId1,text1,langId2,text2,langId3,text3,langId4,text4) \
  COMPILE_FOR_3_OPTIONS(def,langId1,text1,langId2,text2,langId3,text3)+ADD_OPTION(langId4,text4)

static bool elemIsVisible(const QXmlAttributes &attrib, bool defVal = true)
{
   QByteArray visible = attrib.value("visible").toUtf8();

   if (visible.isEmpty()) {
      return defVal;
   }

   if (visible.at(0) == '$' && visible.length() > 1) {
      QByteArray id = visible.mid(1);
      ConfigOption *opt = Config::instance()->get(id);

      if (opt && opt->kind() == ConfigOption::O_Bool) {
         return *((ConfigBool *)opt)->valueRef();

      } else if (!opt) {
         err("found unsupported value %s for visible attribute in layout file\n", visible.data());
      }
   }

   return visible != "no" && visible != "0";
}

//---------------------------------------------------------------------------------

LayoutNavEntry *LayoutNavEntry::find(LayoutNavEntry::Kind kind, const char *file) const
{
   LayoutNavEntry *result = 0;
  
   for ( auto entry : m_children) {
      // depth first search, needed to find the entry furthest from the
      // root in case an entry is in the tree twice

      result = entry->find(kind, file);

      if (result) {
         return result;
      }

      if (entry->kind() == kind && (file == 0 || entry->baseFile() == file)) {
         return entry;
      }
   }

   return result;
}

QByteArray LayoutNavEntry::url() const
{
   QByteArray url = baseFile().trimmed();

   if ((kind() != LayoutNavEntry::User && kind() != LayoutNavEntry::UserGroup) || 
         (kind() == LayoutNavEntry::UserGroup && url.left(9) == "usergroup")) {
      url += Doxygen::htmlFileExtension;

   } else if (url.left(5) == "@ref " || url.left(5) == "\\ref ") {
      Definition *d;
      QByteArray anchor;
      bool found = false;

      if (resolveLink(0, url.mid(5).trimmed(), true, &d, anchor)) {

         if (d && d->isLinkable()) {
            url = d->getOutputFileBase() + Doxygen::htmlFileExtension;
            if (!anchor.isEmpty()) {
               url += "#" + anchor;
            }
            found = true;
         }
      }

      if (!found) {
         msg("explicit link request to '%s' in layout file '%s' could not be resolved\n", qPrint(url.mid(5)), qPrint(Config_getString("LAYOUT_FILE")));
      }
   }

   //printf("LayoutNavEntry::url()=%s\n",url.data());
   return url;
}

//---------------------------------------------------------------------------------

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

      void operator()(const QXmlAttributes &attrib) {
         (m_parent->*m_handler)(m_kind, attrib);
      }

    private:
      LayoutParser *m_parent;
      LayoutDocEntry::Kind m_kind;
      Handler m_handler;
   };

   class StartElementHandlerSection : public StartElementHandler
   {
     typedef void (LayoutParser::*Handler)(LayoutDocEntry::Kind kind, const QXmlAttributes &attrib, const QByteArray &title);

     public:
      StartElementHandlerSection(LayoutParser *parent, LayoutDocEntry::Kind k, Handler h, const QByteArray &title)
         : m_parent(parent), m_kind(k), m_handler(h), m_title(title) {}

      void operator()(const QXmlAttributes &attrib) {
         (m_parent->*m_handler)(m_kind, attrib, m_title);
      }

    private:
      LayoutParser *m_parent;
      LayoutDocEntry::Kind m_kind;
      Handler m_handler;
      QByteArray m_title;
   };

   class StartElementHandlerMember : public StartElementHandler
   {
      typedef void (LayoutParser::*Handler)(const QXmlAttributes &attrib, MemberListType type, const QByteArray &title,
                                            const QByteArray &subtitle);
    public:
      StartElementHandlerMember(LayoutParser *parent, Handler h, MemberListType type,
                                const QByteArray &tl, const QByteArray &ss = QByteArray() )

         : m_parent(parent), m_handler(h), m_type(type), m_title(tl), m_subscript(ss) {}

      void operator()(const QXmlAttributes &attrib) {
         (m_parent->*m_handler)(attrib, m_type, m_title, m_subscript);
      }

    private:
      LayoutParser *m_parent;
      Handler m_handler;
      MemberListType m_type;
      QByteArray m_title;
      QByteArray m_subscript;
   };

   class StartElementHandlerNavEntry : public StartElementHandler
   {

     typedef void (LayoutParser::*Handler)(LayoutNavEntry::Kind kind, const QXmlAttributes &attrib, const QByteArray &title);

      public:
        StartElementHandlerNavEntry(LayoutParser *parent, LayoutNavEntry::Kind kind, Handler h, const QByteArray &tl )
           : m_parent(parent), m_kind(kind), m_handler(h), m_title(tl) {}

        void operator()(const QXmlAttributes &attrib) {
           (m_parent->*m_handler)(m_kind, attrib, m_title);
        }

     private:
       LayoutParser *m_parent;
       LayoutNavEntry::Kind m_kind;
       Handler m_handler;
       QByteArray m_title;
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
      m_part = -1; // invalid
      m_rootNav = 0;

      //bool fortranOpt = Config_getBool("OPTIMIZE_FOR_FORTRAN");     
      //bool javaOpt    = Config_getBool("OPTIMIZE_OUTPUT_JAVA");

      // start & end handlers
      m_sHandler.insert("doxygenlayout", StartElementHandler(this, &LayoutParser::startLayout));
      m_eHandler.insert("doxygenlayout", EndElementHandler(this, &LayoutParser::endLayout));

      // class layout handlers
      m_sHandler.insert("navindex",      StartElementHandler(this, &LayoutParser::startNavIndex));
      m_sHandler.insert("navindex/tab",  StartElementHandler(this, &LayoutParser::startNavEntry));
      m_eHandler.insert("navindex/tab",  EndElementHandler(this, &LayoutParser::endNavEntry));
      m_eHandler.insert("navindex",      EndElementHandler(this, &LayoutParser::endNavIndex));

      // class layout handlers
      m_sHandler.insert("class",                     StartElementHandler(this, &LayoutParser::startClass));
      m_sHandler.insert("class/briefdescription",    StartElementHandlerKind(this, LayoutDocEntry::BriefDesc, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/detaileddescription", StartElementHandlerSection(this, LayoutDocEntry::DetailedDesc, &LayoutParser::startSectionEntry,
                              theTranslator->trDetailedDescription()));

      m_sHandler.insert("class/authorsection",       StartElementHandlerKind(this, LayoutDocEntry::AuthorSection, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("class/includes",            StartElementHandlerKind(this, LayoutDocEntry::ClassIncludes, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("class/inheritancegraph",    StartElementHandlerKind(this, LayoutDocEntry::ClassInheritanceGraph, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("class/collaborationgraph",  StartElementHandlerKind(this, LayoutDocEntry::ClassCollaborationGraph, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("class/allmemberslink",      StartElementHandlerKind(this, LayoutDocEntry::ClassAllMembersLink, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("class/usedfiles",           StartElementHandlerKind(this, LayoutDocEntry::ClassUsedFiles, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("class/memberdecl",          StartElementHandler(this, &LayoutParser::startMemberDecl));

      m_sHandler.insert("class/memberdecl/membergroups",  StartElementHandlerKind(this, LayoutDocEntry::MemberGroups, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("class/memberdecl/nestedclasses", StartElementHandlerSection(this, LayoutDocEntry::ClassNestedClasses, &LayoutParser::startSectionEntry,
                              COMPILE_FOR_1_OPTION(theTranslator->trCompounds(), SrcLangExt_Fortran, theTranslator->trDataTypes())));

      m_sHandler.insert("class/memberdecl/services",      StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_services, theTranslator->trServices()));

      m_sHandler.insert("class/memberdecl/interfaces",    StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_interfaces, theTranslator->trInterfaces()));

      m_sHandler.insert("class/memberdecl/publictypes",   StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_pubTypes, theTranslator->trPublicTypes()));

      m_sHandler.insert("class/memberdecl/publicslots",   StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_pubSlots, theTranslator->trPublicSlots()));

      m_sHandler.insert("class/memberdecl/signals",       StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_signals, theTranslator->trSignals()));

      m_sHandler.insert("class/memberdecl/publicmethods", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry, MemberListType_pubMethods, 
                              COMPILE_FOR_1_OPTION(theTranslator->trPublicMembers(), SrcLangExt_ObjC, theTranslator->trInstanceMethods())));

      m_sHandler.insert("class/memberdecl/publicstaticmethods", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_pubStaticMethods, COMPILE_FOR_1_OPTION( theTranslator->trStaticPublicMembers(),
                                 SrcLangExt_ObjC, theTranslator->trClassMethods() )));

      m_sHandler.insert("class/memberdecl/publicattributes", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_pubAttribs, theTranslator->trPublicAttribs()));

      m_sHandler.insert("class/memberdecl/publicstaticattributes", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_pubStaticAttribs, theTranslator->trStaticPublicAttribs()));

      m_sHandler.insert("class/memberdecl/protectedtypes", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_proTypes, theTranslator->trProtectedTypes()));

      m_sHandler.insert("class/memberdecl/protectedslots", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_proSlots, theTranslator->trProtectedSlots()));

      m_sHandler.insert("class/memberdecl/protectedmethods", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_proMethods, theTranslator->trProtectedMembers()));

      m_sHandler.insert("class/memberdecl/protectedstaticmethods", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_proStaticMethods, theTranslator->trStaticProtectedMembers()));

      m_sHandler.insert("class/memberdecl/protectedattributes", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_proAttribs, theTranslator->trProtectedAttribs()));

      m_sHandler.insert("class/memberdecl/protectedstaticattributes", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_proStaticAttribs, theTranslator->trStaticProtectedAttribs()));

      m_sHandler.insert("class/memberdecl/packagetypes", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_pacTypes, theTranslator->trPackageTypes()));

      m_sHandler.insert("class/memberdecl/packagemethods", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_pacMethods, theTranslator->trPackageMembers()));

      m_sHandler.insert("class/memberdecl/packagestaticmethods", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_pacStaticMethods, theTranslator->trStaticPackageMembers()));

      m_sHandler.insert("class/memberdecl/packageattributes", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_pacAttribs, theTranslator->trPackageAttribs()));

      m_sHandler.insert("class/memberdecl/packagestaticattributes", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_pacStaticAttribs, theTranslator->trStaticPackageAttribs()));

      m_sHandler.insert("class/memberdecl/properties", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_properties, theTranslator->trProperties()));

      m_sHandler.insert("class/memberdecl/events", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_events, theTranslator->trEvents()));

      m_sHandler.insert("class/memberdecl/privatetypes", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_priTypes, theTranslator->trPrivateTypes()));

      m_sHandler.insert("class/memberdecl/privateslots", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_priSlots, theTranslator->trPrivateSlots()));

      m_sHandler.insert("class/memberdecl/privatemethods", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_priMethods, theTranslator->trPrivateMembers()));

      m_sHandler.insert("class/memberdecl/privatestaticmethods", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_priStaticMethods, theTranslator->trStaticPrivateMembers()));

      m_sHandler.insert("class/memberdecl/privateattributes", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_priAttribs, theTranslator->trPrivateAttribs()));

      m_sHandler.insert("class/memberdecl/privatestaticattributes", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_priStaticAttribs, theTranslator->trStaticPrivateAttribs()));

      m_sHandler.insert("class/memberdecl/friends", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_friends, theTranslator->trFriends()));

      m_sHandler.insert("class/memberdecl/related", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_related, theTranslator->trRelatedFunctions(), theTranslator->trRelatedSubscript()));

      m_eHandler.insert("class/memberdecl", EndElementHandler(this, &LayoutParser::endMemberDecl));
      m_sHandler.insert("class/memberdef",  StartElementHandler(this, &LayoutParser::startMemberDef));

      m_sHandler.insert("class/memberdef/inlineclasses", StartElementHandlerSection(this, LayoutDocEntry::ClassInlineClasses, 
                              &LayoutParser::startSectionEntry, COMPILE_FOR_1_OPTION( theTranslator->trClassDocumentation(), 
                              SrcLangExt_Fortran, theTranslator->trTypeDocumentation() )));

      m_sHandler.insert("class/memberdef/typedefs", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_typedefMembers, theTranslator->trMemberTypedefDocumentation()));

      m_sHandler.insert("class/memberdef/enums", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_enumMembers, theTranslator->trMemberEnumerationDocumentation()));

      m_sHandler.insert("class/memberdef/services", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_serviceMembers, theTranslator->trInterfaces()));

      m_sHandler.insert("class/memberdef/interfaces", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_interfaceMembers, theTranslator->trInterfaces()));

      m_sHandler.insert("class/memberdef/constructors", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_constructors, theTranslator->trConstructorDocumentation()));

      m_sHandler.insert("class/memberdef/functions", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_functionMembers, COMPILE_FOR_2_OPTIONS(theTranslator->trMemberFunctionDocumentation(),
                              SrcLangExt_ObjC, theTranslator->trMethodDocumentation(), 
                              SrcLangExt_Fortran, theTranslator->trMemberFunctionDocumentationFortran() )));

      m_sHandler.insert("class/memberdef/related", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_relatedMembers, theTranslator->trRelatedFunctionDocumentation()));

      m_sHandler.insert("class/memberdef/variables", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_variableMembers, theTranslator->trMemberDataDocumentation()));

      m_sHandler.insert("class/memberdef/properties", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_propertyMembers, theTranslator->trPropertyDocumentation()));

      m_sHandler.insert("class/memberdef/events", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_eventMembers, theTranslator->trEventDocumentation()));

      m_eHandler.insert("class/memberdef", EndElementHandler(this, &LayoutParser::endMemberDef));

      m_eHandler.insert("class", EndElementHandler(this, &LayoutParser::endClass));


      // namespace layout handlers
      m_sHandler.insert("namespace", StartElementHandler(this, &LayoutParser::startNamespace));
      m_sHandler.insert("namespace/briefdescription", StartElementHandlerKind(this, LayoutDocEntry::BriefDesc, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("namespace/detaileddescription", StartElementHandlerSection(this, LayoutDocEntry::DetailedDesc, &LayoutParser::startSectionEntry,
                              theTranslator->trDetailedDescription()));

      m_sHandler.insert("namespace/authorsection", StartElementHandlerKind(this, LayoutDocEntry::AuthorSection, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("namespace/memberdecl", StartElementHandler(this, &LayoutParser::startMemberDecl));

      m_sHandler.insert("namespace/memberdecl/nestednamespaces", StartElementHandlerSection(this, LayoutDocEntry::NamespaceNestedNamespaces,
                              &LayoutParser::startSectionEntry, COMPILE_FOR_3_OPTIONS( theTranslator->trNamespaces(),
                              SrcLangExt_Java, theTranslator->trPackages(), SrcLangExt_IDL, theTranslator->trModules(), 
                              SrcLangExt_Fortran, theTranslator->trModules() )));

      m_sHandler.insert("namespace/memberdecl/constantgroups", StartElementHandlerSection(this, LayoutDocEntry::NamespaceNestedConstantGroups,
                              &LayoutParser::startSectionEntry, theTranslator->trConstantGroups()));

      m_sHandler.insert("namespace/memberdecl/classes", StartElementHandlerSection(this, LayoutDocEntry::NamespaceClasses, 
                              &LayoutParser::startSectionEntry, COMPILE_FOR_1_OPTION(theTranslator->trCompounds(),                              
                              SrcLangExt_Fortran, theTranslator->trDataTypes() )));

      m_sHandler.insert("namespace/memberdecl/membergroups", StartElementHandlerKind(this, LayoutDocEntry::MemberGroups, 
                              &LayoutParser::startSimpleEntry));

      m_sHandler.insert("namespace/memberdecl/typedefs", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decTypedefMembers, theTranslator->trTypedefs()));

      m_sHandler.insert("namespace/memberdecl/enums", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decEnumMembers, theTranslator->trEnumerations()));

      m_sHandler.insert("namespace/memberdecl/functions", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decFuncMembers, COMPILE_FOR_1_OPTION( theTranslator->trFunctions(),
                              SrcLangExt_Fortran, theTranslator->trSubprograms() )));

      m_sHandler.insert("namespace/memberdecl/variables", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decVarMembers, theTranslator->trVariables()));

      m_eHandler.insert("namespace/memberdecl", EndElementHandler(this, &LayoutParser::endMemberDecl));
      m_sHandler.insert("namespace/memberdef",  StartElementHandler(this, &LayoutParser::startMemberDef));

      m_sHandler.insert("namespace/memberdef/inlineclasses", StartElementHandlerSection(this, LayoutDocEntry::NamespaceInlineClasses,
                              &LayoutParser::startSectionEntry, COMPILE_FOR_1_OPTION(theTranslator->trClassDocumentation(),
                              SrcLangExt_Fortran, theTranslator->trTypeDocumentation() )));

      m_sHandler.insert("namespace/memberdef/typedefs", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docTypedefMembers, theTranslator->trTypedefDocumentation()));

      m_sHandler.insert("namespace/memberdef/enums", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docEnumMembers, theTranslator->trEnumerationTypeDocumentation()));

      m_sHandler.insert("namespace/memberdef/functions", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docFuncMembers, COMPILE_FOR_1_OPTION( theTranslator->trFunctionDocumentation(),
                              SrcLangExt_Fortran, theTranslator->trSubprogramDocumentation() )));

      m_sHandler.insert("namespace/memberdef/variables", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docVarMembers, theTranslator->trVariableDocumentation()));

      m_eHandler.insert("namespace/memberdef", EndElementHandler(this, &LayoutParser::endMemberDef));
      m_eHandler.insert("namespace", EndElementHandler(this, &LayoutParser::endNamespace));

      // file layout handlers
      m_sHandler.insert("file", StartElementHandler(this, &LayoutParser::startFile));
      m_sHandler.insert("file/briefdescription", StartElementHandlerKind(this, LayoutDocEntry::BriefDesc, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("file/detaileddescription", StartElementHandlerSection(this, LayoutDocEntry::DetailedDesc, &LayoutParser::startSectionEntry,
                              theTranslator->trDetailedDescription()));

      m_sHandler.insert("file/authorsection", StartElementHandlerKind(this, LayoutDocEntry::AuthorSection, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("file/includes", StartElementHandlerKind(this, LayoutDocEntry::FileIncludes, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("file/includegraph", StartElementHandlerKind(this, LayoutDocEntry::FileIncludeGraph, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("file/includedbygraph", StartElementHandlerKind(this, LayoutDocEntry::FileIncludedByGraph, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("file/sourcelink", StartElementHandlerKind(this, LayoutDocEntry::FileSourceLink, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("file/memberdecl/membergroups", StartElementHandlerKind(this, LayoutDocEntry::MemberGroups, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("file/memberdecl", StartElementHandler(this, &LayoutParser::startMemberDecl));

      m_sHandler.insert("file/memberdecl/classes", StartElementHandlerSection(this, LayoutDocEntry::FileClasses, &LayoutParser::startSectionEntry,
                              COMPILE_FOR_1_OPTION(theTranslator->trCompounds(), SrcLangExt_Fortran, theTranslator->trDataTypes() )));

      m_sHandler.insert("file/memberdecl/namespaces", StartElementHandlerSection(this, LayoutDocEntry::FileNamespaces, 
                              &LayoutParser::startSectionEntry, COMPILE_FOR_3_OPTIONS( theTranslator->trNamespaces(), SrcLangExt_Java,
                              theTranslator->trPackages(), SrcLangExt_IDL, theTranslator->trModules(), 
                              SrcLangExt_Fortran, theTranslator->trModules() )));

      m_sHandler.insert("file/memberdecl/constantgroups", StartElementHandlerSection(this, LayoutDocEntry::FileConstantGroups,
                              &LayoutParser::startSectionEntry, theTranslator->trConstantGroups()));

      m_sHandler.insert("file/memberdecl/defines", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decDefineMembers, theTranslator->trDefines()));

      m_sHandler.insert("file/memberdecl/typedefs", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decTypedefMembers, theTranslator->trTypedefs()));

      m_sHandler.insert("file/memberdecl/enums", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decEnumMembers, theTranslator->trEnumerations()));

      m_sHandler.insert("file/memberdecl/functions", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decFuncMembers, COMPILE_FOR_1_OPTION(
                              theTranslator->trFunctions(), SrcLangExt_Fortran, theTranslator->trSubprograms() )));

      m_sHandler.insert("file/memberdecl/variables", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decVarMembers, theTranslator->trVariables()));

      m_eHandler.insert("file/memberdecl", EndElementHandler(this, &LayoutParser::endMemberDecl));
      m_sHandler.insert("file/memberdef",  StartElementHandler(this, &LayoutParser::startMemberDef));

      m_sHandler.insert("file/memberdef/inlineclasses", StartElementHandlerSection(this, LayoutDocEntry::FileInlineClasses, 
                              &LayoutParser::startSectionEntry,
                              COMPILE_FOR_1_OPTION( theTranslator->trClassDocumentation(), SrcLangExt_Fortran, 
                              theTranslator->trTypeDocumentation() )));

      m_sHandler.insert("file/memberdef/defines", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docDefineMembers, theTranslator->trDefineDocumentation()));

      m_sHandler.insert("file/memberdef/typedefs", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docTypedefMembers, theTranslator->trTypedefDocumentation()));

      m_sHandler.insert("file/memberdef/enums", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docEnumMembers, theTranslator->trEnumerationTypeDocumentation()));

      m_sHandler.insert("file/memberdef/functions", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docFuncMembers, COMPILE_FOR_1_OPTION(theTranslator->trFunctionDocumentation(),
                              SrcLangExt_Fortran, theTranslator->trSubprogramDocumentation() )));

      m_sHandler.insert("file/memberdef/variables", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docVarMembers, theTranslator->trVariableDocumentation()));

      m_eHandler.insert("file/memberdef", EndElementHandler(this, &LayoutParser::endMemberDef));
      m_eHandler.insert("file", EndElementHandler(this, &LayoutParser::endFile));

      // group layout handlers
      m_sHandler.insert("group", StartElementHandler(this, &LayoutParser::startGroup));
      m_sHandler.insert("group/briefdescription", StartElementHandlerKind(this, LayoutDocEntry::BriefDesc, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("group/detaileddescription", StartElementHandlerSection(this, LayoutDocEntry::DetailedDesc, &LayoutParser::startSectionEntry,
                              theTranslator->trDetailedDescription()));

      m_sHandler.insert("group/authorsection", StartElementHandlerKind(this, LayoutDocEntry::AuthorSection, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("group/groupgraph", StartElementHandlerKind(this, LayoutDocEntry::GroupGraph, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("group/memberdecl/membergroups", StartElementHandlerKind(this, LayoutDocEntry::MemberGroups, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("group/memberdecl", StartElementHandler(this, &LayoutParser::startMemberDecl));

      m_sHandler.insert("group/memberdecl/classes", StartElementHandlerSection(this, LayoutDocEntry::GroupClasses, &LayoutParser::startSectionEntry,
                              COMPILE_FOR_1_OPTION( theTranslator->trCompounds(), SrcLangExt_Fortran, theTranslator->trDataTypes() )));

      m_sHandler.insert("group/memberdecl/namespaces", StartElementHandlerSection(this, LayoutDocEntry::GroupNamespaces,
                              &LayoutParser::startSectionEntry, COMPILE_FOR_2_OPTIONS( theTranslator->trNamespaces(),
                              SrcLangExt_Java, theTranslator->trPackages(), SrcLangExt_Fortran, theTranslator->trModules() )));

      m_sHandler.insert("group/memberdecl/dirs", StartElementHandlerSection(this, LayoutDocEntry::GroupDirs, &LayoutParser::startSectionEntry,
                              theTranslator->trDirectories() ));

      m_sHandler.insert("group/memberdecl/nestedgroups", StartElementHandlerSection(this, LayoutDocEntry::GroupNestedGroups, 
                              &LayoutParser::startSectionEntry, theTranslator->trModules() ));

      m_sHandler.insert("group/memberdecl/files", StartElementHandlerSection(this, LayoutDocEntry::GroupFiles, &LayoutParser::startSectionEntry,
                              theTranslator->trFile(true, false) ));

      m_sHandler.insert("group/memberdecl/defines", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decDefineMembers, theTranslator->trDefines()));

      m_sHandler.insert("group/memberdecl/typedefs", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decTypedefMembers, theTranslator->trTypedefs()));

      m_sHandler.insert("group/memberdecl/enums", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decEnumMembers, theTranslator->trEnumerations()));

      m_sHandler.insert("group/memberdecl/enumvalues", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decEnumValMembers, theTranslator->trEnumerationValues()));

      m_sHandler.insert("group/memberdecl/functions", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decFuncMembers, COMPILE_FOR_1_OPTION(
                                 theTranslator->trFunctions(), SrcLangExt_Fortran, theTranslator->trSubprograms() )));

      m_sHandler.insert("group/memberdecl/variables", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decVarMembers, theTranslator->trVariables()));

      m_sHandler.insert("group/memberdecl/signals", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decSignalMembers, theTranslator->trSignals()));

      m_sHandler.insert("group/memberdecl/publicslots", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decPubSlotMembers, theTranslator->trPublicSlots()));

      m_sHandler.insert("group/memberdecl/protectedslots", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decProSlotMembers, theTranslator->trProtectedSlots()));

      m_sHandler.insert("group/memberdecl/privateslots", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decPriSlotMembers, theTranslator->trPrivateSlots()));

      m_sHandler.insert("group/memberdecl/events", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decEventMembers, theTranslator->trEvents()));

      m_sHandler.insert("group/memberdecl/properties", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decPropMembers, theTranslator->trProperties()));

      m_sHandler.insert("group/memberdecl/friends", StartElementHandlerMember(this, &LayoutParser::startMemberDeclEntry,
                              MemberListType_decFriendMembers, theTranslator->trFriends()));

      m_eHandler.insert("group/memberdecl", EndElementHandler(this, &LayoutParser::endMemberDecl));
      m_sHandler.insert("group/memberdef", StartElementHandler(this, &LayoutParser::startMemberDef));
      m_sHandler.insert("group/memberdef/pagedocs", StartElementHandlerKind(this, LayoutDocEntry::GroupPageDocs, &LayoutParser::startSimpleEntry));

      m_sHandler.insert("group/memberdef/inlineclasses", StartElementHandlerSection(this, LayoutDocEntry::GroupInlineClasses, 
                              &LayoutParser::startSectionEntry, COMPILE_FOR_1_OPTION(theTranslator->trClassDocumentation(), SrcLangExt_Fortran,
                              theTranslator->trTypeDocumentation() )));

      m_sHandler.insert("group/memberdef/defines", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docDefineMembers, theTranslator->trDefineDocumentation()));

      m_sHandler.insert("group/memberdef/typedefs", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docTypedefMembers, theTranslator->trTypedefDocumentation()));

      m_sHandler.insert("group/memberdef/enums", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docEnumMembers, theTranslator->trEnumerationTypeDocumentation()));

      m_sHandler.insert("group/memberdef/enumvalues", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docEnumValMembers, theTranslator->trEnumerationValueDocumentation()));

      m_sHandler.insert("group/memberdef/functions", StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docFuncMembers, COMPILE_FOR_1_OPTION( theTranslator->trFunctionDocumentation(),
                                 SrcLangExt_Fortran, theTranslator->trSubprogramDocumentation() )));

      m_sHandler.insert("group/memberdef/variables",
                        StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docVarMembers, theTranslator->trVariableDocumentation()));
      m_sHandler.insert("group/memberdef/signals",
                        StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docSignalMembers, theTranslator->trSignals()));
      m_sHandler.insert("group/memberdef/publicslots",
                        StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docPubSlotMembers, theTranslator->trPublicSlots()));
      m_sHandler.insert("group/memberdef/protectedslots",
                        StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docProSlotMembers, theTranslator->trProtectedSlots()));
      m_sHandler.insert("group/memberdef/privateslots",
                        StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docPriSlotMembers, theTranslator->trPrivateSlots()));
      m_sHandler.insert("group/memberdef/events",
                        StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docEventMembers, theTranslator->trEvents()));
      m_sHandler.insert("group/memberdef/properties",
                        StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docPropMembers, theTranslator->trProperties()));
      m_sHandler.insert("group/memberdef/friends",
                        StartElementHandlerMember(this, &LayoutParser::startMemberDefEntry,
                              MemberListType_docFriendMembers, theTranslator->trFriends()));
      m_eHandler.insert("group/memberdef",
                        EndElementHandler(this, &LayoutParser::endMemberDef));

      m_eHandler.insert("group", EndElementHandler(this, &LayoutParser::endGroup));

      // directory layout handlers
      m_sHandler.insert("directory",
                        StartElementHandler(this, &LayoutParser::startDirectory));
      m_sHandler.insert("directory/briefdescription",
                        StartElementHandlerKind(this, LayoutDocEntry::BriefDesc, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("directory/detaileddescription",
                        StartElementHandlerSection(this, LayoutDocEntry::DetailedDesc, &LayoutParser::startSectionEntry,
                              theTranslator->trDetailedDescription()));
      m_sHandler.insert("directory/directorygraph",
                        StartElementHandlerKind(this, LayoutDocEntry::DirGraph, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("directory/memberdecl",
                        StartElementHandler(this, &LayoutParser::startMemberDecl));
      m_sHandler.insert("directory/memberdecl/dirs",
                        StartElementHandlerKind(this, LayoutDocEntry::DirSubDirs, &LayoutParser::startSimpleEntry));
      m_sHandler.insert("directory/memberdecl/files",
                        StartElementHandlerKind(this, LayoutDocEntry::DirFiles, &LayoutParser::startSimpleEntry));
      m_eHandler.insert("directory/memberdecl",
                        EndElementHandler(this, &LayoutParser::endMemberDecl));
      m_eHandler.insert("directory",
                        EndElementHandler(this, &LayoutParser::endDirectory));
   }

   void startSimpleEntry(LayoutDocEntry::Kind k, const QXmlAttributes &attrib) {
      bool isVisible = elemIsVisible(attrib);
      if (m_part != -1 && isVisible) { LayoutDocManager::instance().addEntry((LayoutDocManager::LayoutPart)m_part, new LayoutDocEntrySimple(k));
      }
   }

   void startSectionEntry(LayoutDocEntry::Kind k, const QXmlAttributes &attrib,
                          const QByteArray &title) {
      bool isVisible = elemIsVisible(attrib);
      QByteArray userTitle = attrib.value("title").toUtf8();

      //printf("startSectionEntry: title='%s' userTitle='%s'\n",
      //    title.data(),userTitle.data());

      if (userTitle.isEmpty()) {
         userTitle = title;
      }

      if (m_part != -1 && isVisible) {
         LayoutDocManager::instance().addEntry((LayoutDocManager::LayoutPart)m_part, new LayoutDocEntrySection(k, userTitle));
      }
   }


   void startMemberDeclEntry(const QXmlAttributes &attrib, MemberListType type,
                             const QByteArray &title, const QByteArray &subscript) {
      //QByteArray visible = convertToQByteArray(attrib.value("visible"));
      //bool isVisible = visible.isEmpty() || (visible!="no" && visible!="0");
      QByteArray userTitle     = attrib.value("title").toUtf8();
      QByteArray userSubscript = attrib.value("subtitle").toUtf8();
      if (userTitle.isEmpty()) {
         userTitle     = title;
      }
      if (userSubscript.isEmpty()) {
         userSubscript = subscript;
      }
      //printf("memberdecl: %s\n",userTitle.data());
      if (m_part != -1 /*&& isVisible*/) {
         LayoutDocManager::instance().addEntry((LayoutDocManager::LayoutPart)m_part,
                                               new LayoutDocEntryMemberDecl(type, userTitle, userSubscript));
      }
   }

   void startMemberDefEntry(const QXmlAttributes &attrib, MemberListType type, const QByteArray &title, const QByteArray &) {
      QByteArray userTitle = attrib.value("title").toUtf8();

      if (userTitle.isEmpty()) {
         userTitle = title;
      }

      //printf("memberdef: %s\n",userTitle.data());
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

   void startNavEntry(const QXmlAttributes &attrib) {
      static bool javaOpt    = Config_getBool("OPTIMIZE_OUTPUT_JAVA");
      static bool fortranOpt = Config_getBool("OPTIMIZE_FOR_FORTRAN");
     
      static bool hasGraphicalHierarchy = Config_getBool("HAVE_DOT") && Config_getBool("GRAPHICAL_HIERARCHY");
      static bool extractAll = Config_getBool("EXTRACT_ALL");

      static struct NavEntryMap {
         const char *typeStr;       // type attribute name in the XML file
         LayoutNavEntry::Kind kind; // corresponding enum name
         QByteArray mainName;         // default title for an item if it has children
         QByteArray subName;          // optional name for an item if it is rendered as a child
         QByteArray intro;            // introduction text to be put on the index page
         QByteArray baseFile;         // base name of the file containing the index page
      } mapping[] = {
         {
            "mainpage",
            LayoutNavEntry::MainPage,
            theTranslator->trMainPage(),
            QByteArray(),
            QByteArray(),
            "index"
         },
         {
            "pages",
            LayoutNavEntry::Pages,
            theTranslator->trRelatedPages(),
            QByteArray(),
            theTranslator->trRelatedPagesDescription(),
            "pages"
         },
         {
            "modules",
            LayoutNavEntry::Modules,
            theTranslator->trModules(),
            QByteArray(),
            theTranslator->trModulesDescription(),
            "modules"
         },
         {
            "namespaces",
            LayoutNavEntry::Namespaces,
            javaOpt  ? theTranslator->trPackages() : fortranOpt ? theTranslator->trModules() : theTranslator->trNamespaces(),
            javaOpt  ? theTranslator->trPackages() : fortranOpt ? theTranslator->trModulesList() : theTranslator->trNamespaceList(),
            javaOpt  ? theTranslator->trPackageListDescription() : fortranOpt ? theTranslator->trModulesListDescription(extractAll) : theTranslator->trNamespaceListDescription(extractAll),
            "namespaces"
         },
         {
            "namespacelist",
            LayoutNavEntry::NamespaceList,
            javaOpt ? theTranslator->trPackages() : fortranOpt ? theTranslator->trModulesList() : theTranslator->trNamespaceList(),
            QByteArray(),
            javaOpt ? theTranslator->trPackageListDescription() : fortranOpt ? theTranslator->trModulesListDescription(extractAll) : theTranslator->trNamespaceListDescription(extractAll),
            "namespaces"
         },
         {
            "namespacemembers",
            LayoutNavEntry::NamespaceMembers,
            javaOpt ? theTranslator->trPackageMembers() : fortranOpt ? theTranslator->trModulesMembers() : theTranslator->trNamespaceMembers(),
            QByteArray(),
            fortranOpt ? theTranslator->trModulesMemberDescription(extractAll) : theTranslator->trNamespaceMemberDescription(extractAll),
            "namespacemembers"
         },
         {
            "classindex",
            LayoutNavEntry::ClassIndex,
            fortranOpt ? theTranslator->trDataTypes() : theTranslator->trCompoundIndex(),
            QByteArray(),
            QByteArray(),
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
            QByteArray(),
            fortranOpt ? theTranslator->trCompoundListDescriptionFortran() : theTranslator->trCompoundListDescription(),
            "annotated"
         },
         {
            "hierarchy",
            LayoutNavEntry::ClassHierarchy,
            theTranslator->trClassHierarchy(),
            QByteArray(),
            theTranslator->trClassHierarchyDescription(),
            hasGraphicalHierarchy ? "inherits" : "hierarchy"
         },
         {
            "classmembers",
            LayoutNavEntry::ClassMembers,
            fortranOpt ? theTranslator->trCompoundMembersFortran() : theTranslator->trCompoundMembers(),
            QByteArray(),
            fortranOpt ? theTranslator->trCompoundMembersDescriptionFortran(extractAll) : theTranslator->trCompoundMembersDescription(extractAll),
            "functions"
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
            QByteArray(),
            theTranslator->trFileListDescription(extractAll),
            "files"
         },
         {
            "globals",
            LayoutNavEntry::FileGlobals,
            theTranslator->trFileMembers(),
            QByteArray(),
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
            QByteArray(),
            theTranslator->trExamplesDescription(),
            "examples"
         },
         {
            "user",
            LayoutNavEntry::User,
            QByteArray(),
            QByteArray(),
            QByteArray(),
            "user"
         },
         {
            "usergroup",
            LayoutNavEntry::UserGroup,
            QByteArray(),
            QByteArray(),
            QByteArray(),
            "usergroup"
         },
         {
            0, // end of list
            (LayoutNavEntry::Kind)0,
            QByteArray(),
            QByteArray(),
            QByteArray(),
            QByteArray()
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
            err("an entry tag within a navindex has no type attribute! Check your layout file!\n");
         } else {
            err("the type '%s' is not supported for the entry tag within a navindex! Check your layout file!\n", type.data());
         }
         m_invalidEntry = true;
         return;
      }

      QByteArray baseFile = mapping[i].baseFile;
      QByteArray title = attrib.value("title").toUtf8();

      bool isVisible = elemIsVisible(attrib);

      if (title.isEmpty()) { // use default title
         title = mapping[i].mainName; // use title for main row
         if (m_rootNav != LayoutDocManager::instance().rootNavEntry() && !mapping[i].subName.isEmpty()) {
            title = mapping[i].subName; // if this is a child of another row, use the subName if available
            // this is mainly done to get compatible naming with older versions.
         }
      }

      QByteArray intro = attrib.value("intro").toUtf8();
      if (intro.isEmpty()) { // use default intro text
         intro = mapping[i].intro;
      }

      QByteArray url = attrib.value("url").toUtf8();
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
   bool startElement( const QString &, const QString &, const QString &name, const QXmlAttributes &attrib ) {
      
      QByteArray temp = m_scope + name.toUtf8();
      
      if (m_sHandler.contains(temp)) {
         (*m_sHandler.find(temp))(attrib);         

      } else {
         err("Unexpected start tag `%s' found in scope='%s'!\n", name.data(), m_scope.data());
      }

      return true;
   }

   bool endElement( const QString &, const QString &, const QString &name ) {           
      auto item = m_eHandler.end();

      if (! m_scope.isEmpty() && m_scope.right(name.length() + 1) == name.toUtf8() + "/") {
         // element ends current scope
         item = m_eHandler.find(m_scope.left(m_scope.length() - 1));

      } else { 
         // continue with current scope
         item = m_eHandler.find(m_scope + name.toUtf8());

      }

      if (item != m_eHandler.end()) {
         (*item)();
      }

      return true;
   }

   bool startDocument() {
      return true;
   }

 private:
   LayoutParser() :  m_invalidEntry(false) { }

   QHash<QString, StartElementHandler> m_sHandler;
   QHash<QString, EndElementHandler>   m_eHandler;
   QByteArray m_scope;

   int m_part;
   LayoutNavEntry *m_rootNav;
   bool m_invalidEntry;
   static int m_userGroupCount;
};

int LayoutParser::m_userGroupCount = 0;

//---------------------------------------------------------------------------------

class LayoutErrorHandler : public QXmlErrorHandler
{
 public:
   LayoutErrorHandler(const char *fn) : fileName(fn) 
   {}

   bool warning( const QXmlParseException &exception ) override {
      warn_uncond("at line %d column %d of %s: %s\n",
                  exception.lineNumber(), exception.columnNumber(), fileName.data(),
                  exception.message().data());
      return false;
   }

   bool error( const QXmlParseException &exception ) override {
      err("at line %d column %d of %s: %s\n",
          exception.lineNumber(), exception.columnNumber(), fileName.data(),
          exception.message().data());
      return false;
   }

   bool fatalError( const QXmlParseException &exception ) override {
      err("fatal: at line %d column %d of %s: %s\n",
          exception.lineNumber(), exception.columnNumber(), fileName.data(),
          exception.message().data());
      return false;
   }

   QString errorString() const override {
      return "";
   }

 private:
   QString errorMsg;
   QString fileName;
};

//---------------------------------------------------------------------------------

static QByteArray getLayout_Default()
{  
   QFile file(":/resources/layout_default.xml");

   if (! file.open(QFile::ReadOnly | QFile::Text)) {
      // CS BROOM - add error handling

      //const QString msg = "Unable to open file " + m_fileName + " : " + file.errorString();
      // QMessageBox::warning( 0, "XML", msg );

      return "";
   }

   return file.readAll();
}

class LayoutDocManager::Private
{
 public:
   QList<LayoutDocEntry *> docEntries[LayoutDocManager::NrParts];
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

   QByteArray layoutData = getLayout_Default();
  
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
   return d->docEntries[(int)part];
}

LayoutNavEntry *LayoutDocManager::rootNavEntry() const
{
   return d->rootNav;
}

void LayoutDocManager::addEntry(LayoutDocManager::LayoutPart p, LayoutDocEntry *e)
{
   d->docEntries[(int)p].append(e);
}

void LayoutDocManager::clear(LayoutDocManager::LayoutPart p)
{
   d->docEntries[(int)p].clear();
}

void LayoutDocManager::parse(QTextStream &t, const char *fileName)
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

//---------------------------------------------------------------------------------

void writeDefaultLayoutFile(const char *fileName)
{
   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Failed to open file %s for writing!\n", fileName);
      return;
   }

   QByteArray layoutData = getLayout_Default();

   QTextStream t(&f);
   t << substitute(layoutData, "$doxygenversion", versionString);
}

//----------------------------------------------------------------------------------

// Convert input to a title.
// The format of input can be a simple title "A title" or in case there are different
// titles for some programming languages they can take the following form:
// "A title|16=Another title|8=Yet Another title"
// where the number is a value of SrcLangExt in decimal notation (i.e. 16=Java, 8=IDL).
QByteArray extractLanguageSpecificTitle(const QByteArray &input, SrcLangExt lang)
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

//----------------------------------------------------------------------------------

QByteArray LayoutDocEntrySection::title(SrcLangExt lang) const
{
   return extractLanguageSpecificTitle(m_title, lang);
}

//----------------------------------------------------------------------------------

QByteArray LayoutDocEntryMemberDecl::title(SrcLangExt lang) const
{
   return extractLanguageSpecificTitle(m_title, lang);
}

QByteArray LayoutDocEntryMemberDecl::subtitle(SrcLangExt lang) const
{
   return extractLanguageSpecificTitle(m_subscript, lang);
}

//----------------------------------------------------------------------------------

QByteArray LayoutDocEntryMemberDef::title(SrcLangExt lang) const
{
   return extractLanguageSpecificTitle(m_title, lang);
}

