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

#include <config.h>
#include <classdef.h>
#include <classlist.h>
#include <docparser.h>
#include <doxygen.h>
#include <language.h>
#include <layout.h>
#include <memberlist.h>
#include <message.h>
#include <searchindex.h>
#include <membergroup.h>
#include <namespacedef.h>
#include <outputlist.h>
#include <util.h>

// must appear after the previous include - resolve soon 
#include <doxy_globals.h>

NamespaceDef::NamespaceDef(const char *df, int dl, int dc, const char *name, const char *lref,
                           const char *fName, const char *type, bool isPublished) :
   Definition(df, dl, dc, name), m_isPublished(isPublished)
{
   if (fName) {
      fileName = stripExtension(fName);

   } else {
      fileName = "namespace";
      fileName += name;
   }

   classSDict       = new ClassSDict();
   namespaceSDict   = new NamespaceSDict();
   m_innerCompounds = new StringMap<QSharedPointer<Definition>>();
      
   m_allMembersDict = 0;

   setReference(lref);
   memberGroupSDict = new MemberGroupSDict;  
   visited = false;

   m_subGrouping = Config_getBool("SUBGROUPING");

   if (type && ! strcmp("module", type)) {
      m_type = MODULE;

   } else if (type && ! strcmp("constants", type)) {
      m_type = CONSTANT_GROUP;

   } else if (type && ! strcmp("library", type)) {
      m_type = LIBRARY;

   } else {
      m_type = NAMESPACE;

   }
}

NamespaceDef::~NamespaceDef()
{
   delete classSDict;
   delete namespaceSDict;
   delete m_innerCompounds;     
   delete memberGroupSDict;
   delete m_allMembersDict;
}

void NamespaceDef::distributeMemberGroupDocumentation()
{
   for (auto mg : *memberGroupSDict) {
      mg->distributeMemberGroupDocumentation();
   }
}

void NamespaceDef::findSectionsInDocumentation()
{
   docFindSections(documentation(), this, 0, docFile());
   
   for (auto mg : *memberGroupSDict) {
      mg->findSectionsInDocumentation();
   }  

   for (auto ml : m_memberLists) {
      if (ml->listType() & MemberListType_declarationLists) {
         ml->findSectionsInDocumentation();
      }
   }
}

void NamespaceDef::insertUsedFile(FileDef *fd)
{
   if (fd == 0) {
      return;
   }

   if (files.contains(fd)) {
      if (Config_getBool("SORT_MEMBER_DOCS")) {
         files.inSort(fd);

      } else {
         files.append(fd);
      }
   }
}

void NamespaceDef::addInnerCompound(QSharedPointer<Definition> d)
{
   m_innerCompounds->insert(d->localName(), d);
  
   if (d->definitionType() == Definition::TypeNamespace) {

      QSharedPointer<NamespaceDef> nd = d.dynamicCast<NamespaceDef>();
      assert(nd);    

      insertNamespace(nd);

   } else if (d->definitionType() == Definition::TypeClass) {

      QSharedPointer<ClassDef> cd = d.dynamicCast<ClassDef>();
      assert(cd);    

      insertClass(cd);
   }
}

void NamespaceDef::insertClass(QSharedPointer<ClassDef> cd)
{
   if (classSDict->find(cd->name()) == 0) {      
      classSDict->insert(cd->name(), cd);
   }
}

void NamespaceDef::insertNamespace(QSharedPointer<NamespaceDef> nd)
{
   if (namespaceSDict->find(nd->name()) == 0) {     
      namespaceSDict->insert(nd->name(), nd);
   }
}

void NamespaceDef::addMembersToMemberGroup()
{  
   for (auto ml : m_memberLists) {
      if (ml->listType() & MemberListType_declarationLists) {
         ::addMembersToMemberGroup(ml.data(), &memberGroupSDict, this);
      }
   }

   // add members inside sections to their groups
   if (memberGroupSDict) {     
      for (auto mg : *memberGroupSDict) {
         if (mg->allMembersInSameSection() && m_subGrouping) {            
            mg->addToDeclarationSection();
         }
      }
   }
}

void NamespaceDef::insertMember(QSharedPointer<MemberDef> md)
{
   if (md->isHidden()) {
      return;
   }

   QSharedPointer<MemberList> allMemberList = getMemberList(MemberListType_allMembersList);

   if (! allMemberList) {
      allMemberList = QSharedPointer<MemberList> (new MemberList(MemberListType_allMembersList));
      m_memberLists.append(allMemberList);
   }

   allMemberList->append(md.data());

   //
   if (! m_allMembersDict) {
      m_allMembersDict = new MemberSDict;
   }
  
   m_allMembersDict->insert(md->localName(), md);
  
   switch (md->memberType()) {

      case MemberType_Variable:
         addMemberToList(MemberListType_decVarMembers, md.data());
         addMemberToList(MemberListType_docVarMembers, md.data());
         break;

      case MemberType_Function:
         addMemberToList(MemberListType_decFuncMembers, md.data());
         addMemberToList(MemberListType_docFuncMembers, md.data());
         break;

      case MemberType_Typedef:
         addMemberToList(MemberListType_decTypedefMembers, md.data());
         addMemberToList(MemberListType_docTypedefMembers, md.data());
         break;

      case MemberType_Enumeration:
         addMemberToList(MemberListType_decEnumMembers, md.data());
         addMemberToList(MemberListType_docEnumMembers, md.data());
         break;

      case MemberType_EnumValue:
         break;

      case MemberType_Define:
         addMemberToList(MemberListType_decDefineMembers, md.data());
         addMemberToList(MemberListType_docDefineMembers, md.data());
         break;

      default:
         err("NamespaceDef::insertMembers(): "
             "member `%s' with class scope `%s' inserted in namespace scope `%s'!\n",
             md->name().data(),
             md->getClassDef() ? md->getClassDef()->name().data() : "",
             name().data());
   }
}

void NamespaceDef::computeAnchors()
{
   QSharedPointer<MemberList> allMemberList = getMemberList(MemberListType_allMembersList);

   if (allMemberList) {
      setAnchors(allMemberList.data());
   }
}

bool NamespaceDef::hasDetailedDescription() const
{
   static bool repeatBrief = Config_getBool("REPEAT_BRIEF");
   return ((!briefDescription().isEmpty() && repeatBrief) ||
           !documentation().isEmpty());
}

void NamespaceDef::writeTagFile(FTextStream &tagFile)
{
   tagFile << "  <compound kind=\"namespace\">" << endl;
   tagFile << "    <name>" << convertToXML(name()) << "</name>" << endl;
   tagFile << "    <filename>" << convertToXML(getOutputFileBase()) << Doxygen::htmlFileExtension << "</filename>" << endl;

   QByteArray idStr = id();
   if (!idStr.isEmpty()) {
      tagFile << "    <clangid>" << convertToXML(idStr) << "</clangid>" << endl;
   }
   
   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Namespace)) {
      switch (lde->kind()) {

         case LayoutDocEntry::NamespaceNestedNamespaces:
         {
            if (namespaceSDict) {              
               for (auto nd : *namespaceSDict) {
                  if (nd->isLinkableInProject()) {
                     tagFile << "    <namespace>" << convertToXML(nd->name()) << "</namespace>" << endl;
                  }
               }
            }
         }
         break;

         case LayoutDocEntry::NamespaceClasses:
         {
            if (classSDict) {              
               for (auto cd : *classSDict) {
                  if (cd->isLinkableInProject()) {
                     tagFile << "    <class kind=\"" << cd->compoundTypeString()
                             << "\">" << convertToXML(cd->name()) << "</class>" << endl;
                  }
               }
            }
         }

         case LayoutDocEntry::MemberDecl: 
         {
            LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
            QSharedPointer<MemberList> ml = getMemberList(lmd->type);

            if (ml) {
               ml->writeTagFile(tagFile);
            }
         }
         break;

         case LayoutDocEntry::MemberGroups: 
         {
            if (memberGroupSDict) {              
               for (auto mg : *memberGroupSDict) {
                  mg->writeTagFile(tagFile);
               }
            }
         }
         break;

         default:
            break;
      }
   }

   writeDocAnchorsToTagFile(tagFile);
   tagFile << "  </compound>" << endl;
}

void NamespaceDef::writeDetailedDescription(OutputList &ol, const QByteArray &title)
{
   if (hasDetailedDescription()) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
      ol.writeRuler();
      ol.popGeneratorState();
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);
      ol.writeAnchor(0, "details");
      ol.popGeneratorState();
      ol.startGroupHeader();
      ol.parseText(title);
      ol.endGroupHeader();

      ol.startTextBlock();
      if (!briefDescription().isEmpty() && Config_getBool("REPEAT_BRIEF")) {
         ol.generateDoc(briefFile(), briefLine(), this, 0, briefDescription(), false, false);
      }
      if (!briefDescription().isEmpty() && Config_getBool("REPEAT_BRIEF") &&
            !documentation().isEmpty()) {
         ol.pushGeneratorState();
         ol.disable(OutputGenerator::Man);
         ol.disable(OutputGenerator::RTF);
         //ol.newParagraph(); // FIXME:PARA
         ol.enableAll();
         ol.disableAllBut(OutputGenerator::Man);
         ol.enable(OutputGenerator::Latex);
         ol.writeString("\n\n");
         ol.popGeneratorState();
      }
      if (!documentation().isEmpty()) {
         ol.generateDoc(docFile(), docLine(), this, 0, documentation() + "\n", true, false);
      }
      ol.endTextBlock();
   }
}

void NamespaceDef::writeBriefDescription(OutputList &ol)
{
   if (hasBriefDescription()) {
      DocRoot *rootNode = validatingParseDoc(briefFile(), briefLine(), this, 0,
                                             briefDescription(), true, false, 0, true, false);
      if (rootNode && !rootNode->isEmpty()) {
         ol.startParagraph();
         ol.writeDoc(rootNode, this, 0);
         ol.pushGeneratorState();
         ol.disable(OutputGenerator::RTF);
         ol.writeString(" \n");
         ol.enable(OutputGenerator::RTF);

         if (hasDetailedDescription()) {
            ol.disableAllBut(OutputGenerator::Html);
            ol.startTextLink(0, "details");
            ol.parseText(theTranslator->trMore());
            ol.endTextLink();
         }
         ol.popGeneratorState();
         ol.endParagraph();
      }
      delete rootNode;

      // FIXME:PARA
      //ol.pushGeneratorState();
      //ol.disable(OutputGenerator::RTF);
      //ol.newParagraph();
      //ol.popGeneratorState();
   }
   ol.writeSynopsis();
}

void NamespaceDef::startMemberDeclarations(OutputList &ol)
{
   ol.startMemberSections();
}

void NamespaceDef::endMemberDeclarations(OutputList &ol)
{
   ol.endMemberSections();
}

void NamespaceDef::startMemberDocumentation(OutputList &ol)
{
   if (Config_getBool("SEPARATE_MEMBER_PAGES")) {
      ol.disable(OutputGenerator::Html);
      Doxygen::suppressDocWarnings = true;
   }
}

void NamespaceDef::endMemberDocumentation(OutputList &ol)
{
   if (Config_getBool("SEPARATE_MEMBER_PAGES")) {
      ol.enable(OutputGenerator::Html);
      Doxygen::suppressDocWarnings = false;
   }
}

void NamespaceDef::writeClassDeclarations(OutputList &ol, const QByteArray &title)
{
   if (classSDict) {
      classSDict->writeDeclaration(ol, 0, title, true);
   }
}

void NamespaceDef::writeInlineClasses(OutputList &ol)
{
   if (classSDict) {
      classSDict->writeDocumentation(ol, this);
   }
}

void NamespaceDef::writeNamespaceDeclarations(OutputList &ol, const QByteArray &title,
      bool const isConstantGroup)
{
   if (namespaceSDict) {
      namespaceSDict->writeDeclaration(ol, title, isConstantGroup, true);
   }
}

void NamespaceDef::writeMemberGroups(OutputList &ol)
{
   /* write user defined member groups */
   if (memberGroupSDict) {
     
      for (auto mg : *memberGroupSDict) {
         if ((! mg->allMembersInSameSection() || ! m_subGrouping) && mg->header() != "[NOHEADER]") {
            mg->writeDeclarations(ol, 0, this, 0, 0);
         }
      }

   }
}

void NamespaceDef::writeAuthorSection(OutputList &ol)
{
   // write Author section (Man only)
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Man);
   ol.startGroupHeader();
   ol.parseText(theTranslator->trAuthor(true, true));
   ol.endGroupHeader();
   ol.parseText(theTranslator->trGeneratedAutomatically(Config_getString("PROJECT_NAME")));
   ol.popGeneratorState();
}

void NamespaceDef::writeSummaryLinks(OutputList &ol)
{
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
   
   bool first = true;

   SrcLangExt lang = getLanguage();

   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Namespace)) {

      if ((lde->kind() == LayoutDocEntry::NamespaceClasses && classSDict && classSDict->declVisible()) ||
            (lde->kind() == LayoutDocEntry::NamespaceNestedNamespaces && namespaceSDict && namespaceSDict->declVisible())) {

         LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
         QByteArray label = lde->kind() == LayoutDocEntry::NamespaceClasses ? "nested-classes" : "namespaces";

         ol.writeSummaryLink(QString(""), label, ls->title(lang), first);
         first = false;

      } else if (lde->kind() == LayoutDocEntry::MemberDecl) {
         LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
         QSharedPointer<MemberList> ml = getMemberList(lmd->type);

         if (ml && ml->declVisible()) {
            ol.writeSummaryLink(QString(""), MemberList::listTypeAsString(ml->listType()), lmd->title(lang), first);
            first = false;
         }
      }
   }

   if (!first) {
      ol.writeString("  </div>\n");
   }
   ol.popGeneratorState();
}

void NamespaceDef::addNamespaceAttributes(OutputList &ol)
{
   // UNO IDL constant groups may be published
   if (getLanguage() == SrcLangExt_IDL && isConstantGroup() && m_isPublished) {
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);
      ol.startLabels();
      ol.writeLabel("published", false);
      ol.endLabels();
      ol.popGeneratorState();
   }
}

void NamespaceDef::writeDocumentation(OutputList &ol)
{
   static bool generateTreeView = Config_getBool("GENERATE_TREEVIEW");

   //static bool outputJava = Config_getBool("OPTIMIZE_OUTPUT_JAVA");
   //static bool fortranOpt = Config_getBool("OPTIMIZE_FOR_FORTRAN");

   QByteArray pageTitle = title();
   startFile(ol, getOutputFileBase(), name(), pageTitle, HLI_NamespaceVisible, !generateTreeView);

   if (!generateTreeView) {
      if (getOuterScope() != Doxygen::globalScope) {
         writeNavigationPath(ol);
      }
      ol.endQuickIndices();
   }

   startTitle(ol, getOutputFileBase(), this);
   ol.parseText(pageTitle);
   addGroupListToTitle(ol, this);
   addNamespaceAttributes(ol);
   endTitle(ol, getOutputFileBase(), displayName());
   ol.startContents();

   if (Doxygen::searchIndex) {
      Doxygen::searchIndex->setCurrentDoc(this, anchor(), false);
      Doxygen::searchIndex->addWord(localName(), true);
   }

   Doxygen::indexList->addIndexItem(this, 0);

   //---------------------------------------- start flexible part -------------------------------

   SrcLangExt lang = getLanguage();
  
   for (auto lde :LayoutDocManager::instance().docEntries(LayoutDocManager::Namespace)) {
      switch (lde->kind()) {
         case LayoutDocEntry::BriefDesc:
            writeBriefDescription(ol);
            break;
         case LayoutDocEntry::MemberDeclStart:
            startMemberDeclarations(ol);
            break;
         case LayoutDocEntry::NamespaceClasses: {
            LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
            writeClassDeclarations(ol, ls->title(lang));
         }
         break;
         case LayoutDocEntry::NamespaceNestedNamespaces: {
            LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
            writeNamespaceDeclarations(ol, ls->title(lang), false);
         }
         break;
         case LayoutDocEntry::NamespaceNestedConstantGroups: {
            LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
            writeNamespaceDeclarations(ol, ls->title(lang), true);
         }
         break;
         case LayoutDocEntry::MemberGroups:
            writeMemberGroups(ol);
            break;
         case LayoutDocEntry::MemberDecl: {
            LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
            writeMemberDeclarations(ol, lmd->type, lmd->title(lang));
         }
         break;
         case LayoutDocEntry::MemberDeclEnd:
            endMemberDeclarations(ol);
            break;
         case LayoutDocEntry::DetailedDesc: {
            LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
            writeDetailedDescription(ol, ls->title(lang));
         }
         break;
         case LayoutDocEntry::MemberDefStart:
            startMemberDocumentation(ol);
            break;
         case LayoutDocEntry::NamespaceInlineClasses:
            writeInlineClasses(ol);
            break;
         case LayoutDocEntry::MemberDef: {
            LayoutDocEntryMemberDef *lmd = (LayoutDocEntryMemberDef *)lde;
            writeMemberDocumentation(ol, lmd->type, lmd->title(lang));
         }
         break;
         case LayoutDocEntry::MemberDefEnd:
            endMemberDocumentation(ol);
            break;
         case LayoutDocEntry::AuthorSection:
            writeAuthorSection(ol);
            break;
         case LayoutDocEntry::ClassIncludes:
         case LayoutDocEntry::ClassInheritanceGraph:
         case LayoutDocEntry::ClassNestedClasses:
         case LayoutDocEntry::ClassCollaborationGraph:
         case LayoutDocEntry::ClassAllMembersLink:
         case LayoutDocEntry::ClassUsedFiles:
         case LayoutDocEntry::ClassInlineClasses:
         case LayoutDocEntry::FileClasses:
         case LayoutDocEntry::FileNamespaces:
         case LayoutDocEntry::FileConstantGroups:
         case LayoutDocEntry::FileIncludes:
         case LayoutDocEntry::FileIncludeGraph:
         case LayoutDocEntry::FileIncludedByGraph:
         case LayoutDocEntry::FileSourceLink:
         case LayoutDocEntry::FileInlineClasses:
         case LayoutDocEntry::GroupClasses:
         case LayoutDocEntry::GroupInlineClasses:
         case LayoutDocEntry::GroupNamespaces:
         case LayoutDocEntry::GroupDirs:
         case LayoutDocEntry::GroupNestedGroups:
         case LayoutDocEntry::GroupFiles:
         case LayoutDocEntry::GroupGraph:
         case LayoutDocEntry::GroupPageDocs:
         case LayoutDocEntry::DirSubDirs:
         case LayoutDocEntry::DirFiles:
         case LayoutDocEntry::DirGraph:
            err("Internal inconsistency: member %d should not be part of "
                "LayoutDocManager::Namespace entry list\n", lde->kind());
            break;
      }
   }

   //---------------------------------------- end flexible part -------------------------------

   ol.endContents();

   endFileWithNavPath(this, ol);

   if (Config_getBool("SEPARATE_MEMBER_PAGES")) {
      QSharedPointer<MemberList> allMemberList = getMemberList(MemberListType_allMembersList);

      if (allMemberList) {
         allMemberList->sort();
      }

      writeMemberPages(ol);
   }
}

void NamespaceDef::writeMemberPages(OutputList &ol)
{
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   for (auto ml : m_memberLists) {
      if (ml->listType()&MemberListType_documentationLists) {
         ml->writeDocumentationPage(ol, displayName(), this);
      }
   }
   ol.popGeneratorState();
}

void NamespaceDef::writeQuickMemberLinks(OutputList &ol, MemberDef *currentMd) const
{
   static bool createSubDirs = Config_getBool("CREATE_SUBDIRS");

   ol.writeString("      <div class=\"navtab\">\n");
   ol.writeString("        <table>\n");

   QSharedPointer<MemberList> allMemberList = getMemberList(MemberListType_allMembersList);

   if (allMemberList) {
      
      for (auto md : *allMemberList) {

         if (md->getNamespaceDef() == this && md->isLinkable() && !md->isEnumValue()) {
            ol.writeString("          <tr><td class=\"navtab\">");

            if (md->isLinkableInProject()) {
               if (md == currentMd) { // selected item => highlight
                  ol.writeString("<a class=\"qindexHL\" ");
               } else {
                  ol.writeString("<a class=\"qindex\" ");
               }

               ol.writeString("href=\"");
               if (createSubDirs) {
                  ol.writeString("../../");
               }
               ol.writeString(md->getOutputFileBase() + Doxygen::htmlFileExtension + "#" + md->anchor());
               ol.writeString("\">");
               ol.writeString(convertToHtml(md->localName()));
               ol.writeString("</a>");
            }
            ol.writeString("</td></tr>\n");
         }
      }
   }

   ol.writeString("        </table>\n");
   ol.writeString("      </div>\n");
}

int NamespaceDef::countMembers()
{
   QSharedPointer<MemberList> allMemberList = getMemberList(MemberListType_allMembersList);

   if (allMemberList) {
      allMemberList->countDocMembers();
   }

   return (allMemberList ? allMemberList->numDocMembers() : 0) + classSDict->count();
}

void NamespaceDef::addUsingDirective(QSharedPointer<NamespaceDef> nd)
{   
   if (m_usingDirMap.find(nd->qualifiedName()) == 0) {
      m_usingDirMap.insert(nd->qualifiedName(), nd);
   }   
}

const NamespaceSDict &NamespaceDef::getUsedNamespaces() const
{
   return m_usingDirMap;
}

void NamespaceDef::addUsingDeclaration(QSharedPointer<Definition> d)
{
   if (m_usingDeclMap.find(d->qualifiedName()) == 0) {
      m_usingDeclMap.insert(d->qualifiedName(), d);
   }
}

QByteArray NamespaceDef::getOutputFileBase() const
{
   if (isReference()) {
      return fileName;

   } else {
      return convertNameToFile(fileName).toUtf8();
   }
}

QSharedPointer<Definition> NamespaceDef::findInnerCompound(const char *n)
{
   if (n == 0) {
      return QSharedPointer<Definition>();
   }

   QSharedPointer<Definition> d = m_innerCompounds->find(n);

   if (! d) {      
      d = m_usingDirMap.find(n);
      
      if (! d) {
         d = m_usingDeclMap.find(n);
      }
   }

   return d;
}

void NamespaceDef::addListReferences()
{
   //bool fortranOpt = Config_getBool("OPTIMIZE_FOR_FORTRAN");
   {
      QList<ListItemInfo> *xrefItems = xrefListItems();
      addRefItem(xrefItems, qualifiedName(), 
                 getLanguage() == SrcLangExt_Fortran ? theTranslator->trModule(true, true) : theTranslator->trNamespace(true, true),
                 getOutputFileBase(), qPrintable(displayName()), 0, this );
   }  

   for (auto mg : *memberGroupSDict) {
      mg->addListReferences(this);
   }

   for (auto ml : m_memberLists) {
      if (ml->listType()&MemberListType_documentationLists) {
         ml->addListReferences(this);
      }
   }
}

QString NamespaceDef::displayName(bool includeScope) const
{
   QString result = includeScope ? name() : localName();

   SrcLangExt lang = getLanguage();
   QByteArray sep  = getLanguageSpecificSeparator(lang);

   if (sep != "::") {
      result = substitute(result.toUtf8(), "::", sep);
   }
  
   return result;
}

QByteArray NamespaceDef::localName() const
{
   QByteArray result = name();
   int i = result.lastIndexOf("::");

   if (i != -1) {
      result = result.mid(i + 2);
   }
   return result;
}

void NamespaceDef::combineUsingRelations()
{
   if (visited) {
      return;   // already done
   }

   visited = true;
 
   for (auto nd : m_usingDirMap) {
      nd->combineUsingRelations();
   }
  
   for (auto nd : m_usingDirMap) {
      // add used namespaces of namespace nd to this namespace
      
      for (auto und : nd->getUsedNamespaces() ) {
         addUsingDirective(und);
      }
  
      // add used classes of namespace nd to this namespace     
      for (auto ucd : nd->getUsedClasses() ) {
         addUsingDeclaration(ucd);
      }
   }
}

int NamespaceSDict::compareValues(const NamespaceDef *item1, const NamespaceDef *item2) const
{
   return qstricmp(item1->name(), item2->name());
}

bool NamespaceSDict::declVisible() const
{
   for (auto nd : *this) {
      if (nd->isLinkable()) {
         return true;
      }
   }

   return false;
}

void NamespaceSDict::writeDeclaration(OutputList &ol, const char *title, bool const isConstantGroup, bool localName)
{
   if (count() == 0) {
      return;   // no namespaces in the list
   }
  
   bool found = false;
  
   for (auto nd : *this) {

      if (found) {
         break;
      }

      if (nd->isLinkable()) {
         SrcLangExt lang = nd->getLanguage();

         if (SrcLangExt_IDL == lang) {
            if (isConstantGroup == nd->isConstantGroup()) {
               found = true;
               break;
            }

         } else if (!isConstantGroup) { // ensure we only get extra section in IDL
            if (nd->isConstantGroup()) {
               err("Internal inconsistency: constant group but not IDL?\n");
            }
            found = true;
            break;
         }
      }
   }

   if (!found) {
      return;   // no linkable namespaces in the list
   }

   // write list of namespaces
   ol.startMemberHeader("namespaces");

   //bool javaOpt    = Config_getBool("OPTIMIZE_OUTPUT_JAVA");
   //bool fortranOpt = Config_getBool("OPTIMIZE_FOR_FORTRAN");

   ol.parseText(title);
   ol.endMemberHeader();
   ol.startMemberList();
    
   for (auto nd : *this) {

      if (nd->isLinkable()) {

         SrcLangExt lang = nd->getLanguage();

         if (lang == SrcLangExt_IDL && (isConstantGroup != nd->isConstantGroup())) {
            continue;   // will be output in another pass, see layout_default.xml
         }

         ol.startMemberDeclaration();
         ol.startMemberItem(nd->getOutputFileBase(), 0);

         QByteArray ct = nd->compoundTypeString();
         ol.docify(ct);
         ol.docify(" ");
         ol.insertMemberAlign();

         QString name;
         if (localName) {
            name = nd->localName();
         } else {
            name = nd->displayName();
         }

         ol.writeObjectLink(nd->getReference(), nd->getOutputFileBase(), 0, name.toUtf8());
         ol.endMemberItem();

         if (! nd->briefDescription().isEmpty() && Config_getBool("BRIEF_MEMBER_DESC")) {

            ol.startMemberDescription(nd->getOutputFileBase());
            ol.generateDoc(nd->briefFile(), nd->briefLine(), nd.data(), 0, nd->briefDescription(), false, false, 0, true);
            ol.endMemberDescription();
         }

         ol.endMemberDeclaration(0, 0);
      }
   }
   ol.endMemberList();
}

QSharedPointer<MemberList> NamespaceDef::createMemberList(MemberListType lt)
{   
   for (auto item : m_memberLists) {
      if (item->listType() == lt) {
         return item;
      }
   }

   // not found, create a new member list
   QSharedPointer<MemberList> ml (new MemberList(lt));
   m_memberLists.append(ml);
 
   return ml;
}

void NamespaceDef::addMemberToList(MemberListType lt, MemberDef *md)
{  
   QSharedPointer<MemberList> ml = createMemberList(lt);
   ml->append(md);

   if (ml->listType() & MemberListType_declarationLists) {
      md->setSectionList(this, ml);
   }
}

QSharedPointer<MemberList> NamespaceDef::getMemberList(MemberListType lt) const
{
   for (auto ml : m_memberLists) {
      if (ml->listType() == lt) {
         return ml;
      }
   }

   return QSharedPointer<MemberList>();
}

void NamespaceDef::writeMemberDeclarations(OutputList &ol, MemberListType lt, const QByteArray &title)
{
   QSharedPointer<MemberList> ml = getMemberList(lt);

   if (ml) {
      ml->writeDeclarations(ol, 0, this, 0, 0, title, 0);
   }
}

void NamespaceDef::writeMemberDocumentation(OutputList &ol, MemberListType lt, const QByteArray &title)
{
   QSharedPointer<MemberList> ml = getMemberList(lt);

   if (ml) {
      ml->writeDocumentation(ol, qPrintable(displayName()), this, title);
   }
}

bool NamespaceDef::isLinkableInProject() const
{
   int i = name().lastIndexOf("::");

   if (i == -1) {
      i = 0;
   } else {
      i += 2;
   }

   static bool extractAnonNs = Config_getBool("EXTRACT_ANON_NSPACES");

   if (extractAnonNs && name().mid(i, 20) == "anonymous_namespace{") {                                          
      return true;
   }

   return !name().isEmpty() && name().at(i) != '@' &&  (hasDocumentation() || getLanguage() == SrcLangExt_CSharp) && 
          !isReference() && !isHidden() && ! isArtificial();      
}

bool NamespaceDef::isLinkable() const
{
   return isLinkableInProject() || isReference();
}

MemberDef *NamespaceDef::getMemberByName(const QByteArray &n) const
{
   QSharedPointer<MemberDef> md;

   if (m_allMembersDict && !n.isEmpty()) {
      md = QSharedPointer<MemberDef> (m_allMembersDict->find(n));
   }

   return md.data();
}

QByteArray NamespaceDef::title() const
{
   SrcLangExt lang = getLanguage();
   QByteArray pageTitle;

   QByteArray tempDisplay = displayName().toUtf8();

   if (lang == SrcLangExt_Java || lang == SrcLangExt_CSharp) {
      pageTitle = theTranslator->trPackage(tempDisplay.constData());

   } else if (lang == SrcLangExt_Fortran) {
      pageTitle = theTranslator->trModuleReference(tempDisplay.constData());

   } else if (lang == SrcLangExt_IDL) {

      pageTitle = isConstantGroup() ? theTranslator->trConstantGroupReference(tempDisplay.constData())
                  : theTranslator->trModuleReference(tempDisplay.constData());

   } else {
      pageTitle = theTranslator->trNamespaceReference(tempDisplay.constData());

   }

   return pageTitle;
}

QByteArray NamespaceDef::compoundTypeString() const
{
   SrcLangExt lang = getLanguage();

   if (lang == SrcLangExt_Java || lang == SrcLangExt_CSharp) {
      return "package";

   } else if (lang == SrcLangExt_Fortran) {
      return "module";

   } else if (lang == SrcLangExt_IDL) {

      if (isModule()) {
         return "module";

      } else if (isConstantGroup()) {

         return "constants";
      } else if (isLibrary()) {

         return "library";
      } else {
         err("Internal inconsistency: namespace in IDL not module, library or constant group\n");
      }
   }
   return "";
}
