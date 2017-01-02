/*************************************************************************
 *
 * Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim 
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

#include <namespacedef.h>

#include <config.h>
#include <classdef.h>
#include <docparser.h>
#include <doxy_globals.h>
#include <language.h>
#include <layout.h>
#include <message.h>
#include <membergroup.h>
#include <outputlist.h>
#include <util.h>

NamespaceDef::NamespaceDef(const QString &df, int dl, int dc, const QString &name, 
      const QString &lref, QString fName, const QString &type, bool isPublished) 
   : Definition(df, dl, dc, name), m_isPublished(isPublished), m_subGrouping(Config::getBool("allow-sub-grouping"))
{
   if (! fName.isEmpty()) {
      fileName = stripExtension(fName);

   } else {
      setFileName(name);
   }

   m_usingDeclMap   = new StringMap<QSharedPointer<Definition>>();
   m_innerCompounds = new StringMap<QSharedPointer<Definition>>();
      
   classSDict       = new ClassSDict();
   m_allMembersDict = 0;  
   memberGroupSDict = new MemberGroupSDict();  
   m_usingDirMap    = new NamespaceSDict();
    
   setReference(lref);
   
   visited = false; 

   if (type == "module") {
      m_type = MODULE;

   } else if (type == "constants") {
      m_type = CONSTANT_GROUP;

   } else if (type == "library") {
      m_type = LIBRARY;

   } else   {
      m_type = NAMESPACE;

   }  
}

NamespaceDef::~NamespaceDef()
{
   delete m_usingDeclMap;   
   delete m_innerCompounds;     

   delete classSDict;
   delete m_allMembersDict;
   delete memberGroupSDict;
   delete m_usingDirMap;    
}

void NamespaceDef::setFileName(const QString &fn)
{
  fileName = "namespace" + fn;
}

void NamespaceDef::distributeMemberGroupDocumentation()
{
   for (auto mg : *memberGroupSDict) {
      mg->distributeMemberGroupDocumentation();
   }
}

StringMap<QSharedPointer<Definition>> &NamespaceDef::getUsedClasses() 
{
   return *m_usingDeclMap;
}

void NamespaceDef::findSectionsInDocumentation()
{
   QSharedPointer<NamespaceDef> self = sharedFrom(this);
   docFindSections(documentation(), self, QSharedPointer<MemberGroup>(), docFile());
   
   for (auto mg : *memberGroupSDict) {
      mg->findSectionsInDocumentation();
   }  

   for (auto ml : m_memberLists) {
      if (ml->listType() & MemberListType_declarationLists) {
         ml->findSectionsInDocumentation();
      }
   }
}

void NamespaceDef::insertUsedFile(QSharedPointer<FileDef> fd)
{
   if (fd == 0) {
      return;
   }

   if (! files.contains(fd)) {
      if (Config::getBool("sort-member-docs")) {
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
   if (classSDict->find(cd->name()) == nullptr) {      

      // added 12/2015
      if (cd->isHidden()) {
         return;
      }

      classSDict->insert(cd->name(), cd);
   }
}

void NamespaceDef::insertNamespace(QSharedPointer<NamespaceDef> nd)
{
   if (m_namespaceSDict.find(nd->name()) == nullptr) {     
      m_namespaceSDict.insert(nd->name(), nd);
   }
}

void NamespaceDef::addMembersToMemberGroup()
{  
   QSharedPointer<NamespaceDef> self = sharedFrom(this);

   for (auto ml : m_memberLists) {
      if (ml->listType() & MemberListType_declarationLists) {
         ::addMembersToMemberGroup(ml, &memberGroupSDict, self);
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

   allMemberList->append(md);

   //
   if (! m_allMembersDict) {
      m_allMembersDict = new MemberSDict;
   }
  
   m_allMembersDict->insert(md->localName(), md);
 
   switch (md->memberType()) {

      case MemberType_Variable:
         addMemberToList(MemberListType_decVarMembers, md);
         addMemberToList(MemberListType_docVarMembers, md);
         break;

      case MemberType_Function:
         addMemberToList(MemberListType_decFuncMembers, md);
         addMemberToList(MemberListType_docFuncMembers, md);
         break;

      case MemberType_Typedef:
         addMemberToList(MemberListType_decTypedefMembers, md);
         addMemberToList(MemberListType_docTypedefMembers, md);
         break;

      case MemberType_Enumeration:
         addMemberToList(MemberListType_decEnumMembers, md);
         addMemberToList(MemberListType_docEnumMembers, md);
         break;

      case MemberType_EnumValue:
         break;

      case MemberType_Define:
         addMemberToList(MemberListType_decDefineMembers, md);
         addMemberToList(MemberListType_docDefineMembers, md);
         break;

      default:
         err("NamespaceDef::insertMembers(): member %s with class scope %s inserted in namespace scope `%s'\n",
             qPrintable(md->name()), md->getClassDef() ? qPrintable(md->getClassDef()->name()) : "", qPrintable(name()) );
   }
}

void NamespaceDef::computeAnchors()
{
   QSharedPointer<MemberList> allMemberList = getMemberList(MemberListType_allMembersList);

   if (allMemberList) {
      setAnchors(allMemberList);
   }
}

bool NamespaceDef::hasDetailedDescription() const
{
   static bool repeatBrief = Config::getBool("repeat-brief");

   return ((!briefDescription().isEmpty() && repeatBrief) || ! documentation().isEmpty());
}

void NamespaceDef::writeTagFile(QTextStream &tagFile)
{
   tagFile << "  <compound kind=\"namespace\">" << endl;
   tagFile << "    <name>" << convertToXML(name()) << "</name>" << endl;
   tagFile << "    <filename>" << convertToXML(getOutputFileBase()) << Doxy_Globals::htmlFileExtension << "</filename>" << endl;

   QString idStr = id();
   if (! idStr.isEmpty()) {
      tagFile << "    <clangid>" << convertToXML(idStr) << "</clangid>" << endl;
   }
   
   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Namespace)) {
      switch (lde->kind()) {

         case LayoutDocEntry::NamespaceNestedNamespaces:
         {
                        
            for (auto &nd : m_namespaceSDict) {
               if (nd->isLinkableInProject()) {
                  tagFile << "    <namespace>" << convertToXML(nd->name()) << "</namespace>" << endl;
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

void NamespaceDef::writeDetailedDescription(OutputList &ol, const QString &title)
{
  QSharedPointer<NamespaceDef> self = sharedFrom(this);

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
      if (! briefDescription().isEmpty() && Config::getBool("repeat-brief")) {
         ol.generateDoc(briefFile(), briefLine(), self, QSharedPointer<MemberDef>(), briefDescription(), false, false);
      }

      if (!briefDescription().isEmpty() && Config::getBool("repeat-brief") && !documentation().isEmpty()) {
         ol.pushGeneratorState();
         ol.disable(OutputGenerator::Man);
         ol.disable(OutputGenerator::RTF);
         //ol.newParagraph();                // FIXME:PARA
         ol.enableAll();
         ol.disableAllBut(OutputGenerator::Man);
         ol.enable(OutputGenerator::Latex);
         ol.writeString("\n\n");
         ol.popGeneratorState();
      }

      if (!documentation().isEmpty()) {
         ol.generateDoc(docFile(), docLine(), self, QSharedPointer<MemberDef>(), documentation() + "\n", true, false);
      }

      ol.endTextBlock();
   }
}

void NamespaceDef::writeBriefDescription(OutputList &ol)
{
   QSharedPointer<NamespaceDef> self = sharedFrom(this);

   if (hasBriefDescription()) {
      DocRoot *rootNode = validatingParseDoc(briefFile(), briefLine(), self, QSharedPointer<MemberDef>(),
                                             briefDescription(), true, false, "", true, false);

      if (rootNode && ! rootNode->isEmpty()) {
         ol.startParagraph();

         ol.pushGeneratorState();
         ol.disableAllBut(OutputGenerator::Man);
         ol.writeString(" - ");
         ol.popGeneratorState();

         ol.writeDoc(rootNode, self, QSharedPointer<MemberDef>());
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
   if (Config::getBool("separate-member-pages")) {
      ol.disable(OutputGenerator::Html);
      Doxy_Globals::suppressDocWarnings = true;
   }
}

void NamespaceDef::endMemberDocumentation(OutputList &ol)
{
   if (Config::getBool("separate-member-pages")) {
      ol.enable(OutputGenerator::Html);
      Doxy_Globals::suppressDocWarnings = false;
   }
}

void NamespaceDef::writeClassDeclarations(OutputList &ol, const QString &title)
{
   if (classSDict) {
      classSDict->writeDeclaration(ol, 0, title, true);
   }
}   

void NamespaceDef::writeInlineClasses(OutputList &ol)
{
   if (classSDict) {
      QSharedPointer<NamespaceDef> self = sharedFrom(this);
      classSDict->writeDocumentation(ol, self);
   }
}

void NamespaceDef::writeNamespaceDeclarations(OutputList &ol, const QString &title,
      bool const isConstantGroup)
{
   if (! m_namespaceSDict.isEmpty()) {
      m_namespaceSDict.writeDeclaration(ol, title, isConstantGroup, true);
   }
}

void NamespaceDef::writeMemberGroups(OutputList &ol)
{
   /* write user defined member groups */
  QSharedPointer<NamespaceDef> self = sharedFrom(this);

   if (memberGroupSDict) {
     
      for (auto mg : *memberGroupSDict) {
         if ((! mg->allMembersInSameSection() || ! m_subGrouping) && mg->header() != "[NOHEADER]") {
            mg->writeDeclarations(ol, QSharedPointer<ClassDef>(), self, QSharedPointer<FileDef>(), QSharedPointer<GroupDef>());
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
   ol.parseText(theTranslator->trGeneratedAutomatically(Config::getString("project-name")));
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
            (lde->kind() == LayoutDocEntry::NamespaceNestedNamespaces && ! m_namespaceSDict.isEmpty() &&
             m_namespaceSDict.declVisible())) {

         LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
         QString label = lde->kind() == LayoutDocEntry::NamespaceClasses ? "nested-classes" : "namespaces";

         ol.writeSummaryLink("", label, ls->title(lang), first);
         first = false;

      } else if (lde->kind() == LayoutDocEntry::MemberDecl) {
         LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
         QSharedPointer<MemberList> ml = getMemberList(lmd->type);

         if (ml && ml->declVisible()) {
            ol.writeSummaryLink("", MemberList::listTypeAsString(ml->listType()), lmd->title(lang), first);
            first = false;
         }
      }
   }

   if (! first) {
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
   QSharedPointer<NamespaceDef> self = sharedFrom(this);
   static bool generateTreeView = Config::getBool("generate-treeview");

   // static bool outputJava = Config::getBool("optimize-java");
   // static bool fortranOpt = Config::_getBool("optimize-fortran");

   QString pageTitle = title();
   startFile(ol, getOutputFileBase(), name(), pageTitle, HLI_NamespaceVisible, !generateTreeView);

   if (! generateTreeView) {
      if (getOuterScope() != Doxy_Globals::globalScope) {
         writeNavigationPath(ol);
      }
      ol.endQuickIndices();
   }

   startTitle(ol, getOutputFileBase(), self);
   ol.parseText(pageTitle);
   addGroupListToTitle(ol, self);
   addNamespaceAttributes(ol);
   endTitle(ol, getOutputFileBase(), displayName());

   ol.startContents();

   if (Doxy_Globals::searchIndexBase != nullptr) {
      Doxy_Globals::searchIndexBase->setCurrentDoc(self, anchor(), false);
      Doxy_Globals::searchIndexBase->addWord(localName(), true);
   }

   Doxy_Globals::indexList.addIndexItem(self, QSharedPointer<MemberDef>());

   // 
   SrcLangExt lang = getLanguage();
  
   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Namespace)) {

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

   ol.endContents();

   endFileWithNavPath(self, ol);

   if (Config::getBool("separate-member-pages")) {
      QSharedPointer<MemberList> allMemberList = getMemberList(MemberListType_allMembersList);

      if (allMemberList) {
         allMemberList->sort();
      }

      writeMemberPages(ol);
   }
}

void NamespaceDef::writeMemberPages(OutputList &ol)
{
   QSharedPointer<NamespaceDef> self = sharedFrom(this);

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   for (auto ml : m_memberLists) {
      if (ml->listType()&MemberListType_documentationLists) {
         ml->writeDocumentationPage(ol, displayName(), self);
      }
   }

   ol.popGeneratorState();
}

void NamespaceDef::writeQuickMemberLinks(OutputList &ol, QSharedPointer<MemberDef> currentMd) const
{
   static bool createSubDirs = Config::getBool("create-subdirs");

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
               ol.writeString(md->getOutputFileBase() + Doxy_Globals::htmlFileExtension + "#" + md->anchor());
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
   if (m_usingDirMap->find(nd->qualifiedName()) == 0) {
      m_usingDirMap->insert(nd->qualifiedName(), nd);
   }   
}

const NamespaceSDict &NamespaceDef::getUsedNamespaces() const
{
   return *m_usingDirMap;
}

void NamespaceDef::addUsingDeclaration(QSharedPointer<Definition> d)
{
   if (m_usingDeclMap->find(d->qualifiedName()) == 0) {
      m_usingDeclMap->insert(d->qualifiedName(), d);
   }
}

QString NamespaceDef::getOutputFileBase() const
{
   if (isReference()) {
      return fileName;

   } else {
      return convertNameToFile(fileName);
   }
}

QSharedPointer<Definition> NamespaceDef::findInnerCompound(const QString &n)
{
   if (n.isEmpty()) {
      return QSharedPointer<Definition>();
   }

   QSharedPointer<Definition> d = m_innerCompounds->find(n);

   if (d == nullptr) {      
      d = m_usingDirMap->find(n);
      
      if (d == nullptr) {
         d = m_usingDeclMap->find(n);
      }
   }

   return d;
}

void NamespaceDef::addListReferences()
{
   QSharedPointer<NamespaceDef> self = sharedFrom(this);
   // bool fortranOpt = Config::getBool("optimize-fortran");
   
   const QList<ListItemInfo> &xrefItems = getRefItems();

   QString prefix; 
   if (getLanguage() == SrcLangExt_Fortran) {
      prefix = theTranslator->trModule(true, true); 
   } else {
      prefix = theTranslator->trNamespace(true, true);
   }

   addRefItem(xrefItems, qualifiedName(), prefix, getOutputFileBase(), displayName(), "", self);
     
   for (auto mg : *memberGroupSDict) {
      mg->addListReferences(self);
   }

   for (auto ml : m_memberLists) {
      if (ml->listType()&MemberListType_documentationLists) {
         ml->addListReferences(self);
      }
   }
}

QString NamespaceDef::displayName(bool includeScope) const
{
   QString retval = includeScope ? name() : localName();

   SrcLangExt lang = getLanguage();
   QString sep = getLanguageSpecificSeparator(lang);

   if (sep != "::") {
      retval = substitute(retval, "::", sep);
   }

   // added 01/2016
   retval = renameNS_Aliases(retval);
  
   return retval;
}

QString NamespaceDef::localName() const
{
   QString retval = name();
   int i = retval.lastIndexOf("::");

   if (i != -1) {
      retval = retval.mid(i + 2);
   }

   // added 01/2016
   retval = renameNS_Aliases(retval);

   return retval;
}

void NamespaceDef::combineUsingRelations()
{
   if (visited) {
      return;   // already done
   }

   visited = true;
 
   for (auto nd : *m_usingDirMap) {
      nd->combineUsingRelations();
   }
  
   for (auto nd : *m_usingDirMap) {
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

int NamespaceSDict::compareMapValues(const QSharedPointer<NamespaceDef> &item1, const QSharedPointer<NamespaceDef> &item2) const
{
   return item1->name().compare(item2->name(), Qt::CaseInsensitive);
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

void NamespaceSDict::writeDeclaration(OutputList &ol, const QString &title, bool const isConstantGroup, bool localName)
{
   if (count() == 0) {
      // no namespaces in the list
      return;  
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

         } else if (! isConstantGroup) { 
            // ensure we only get extra section in IDL

            if (nd->isConstantGroup()) {
               err("Internal inconsistency: constant group but not IDL?\n");
            }

            found = true;
            break;
         }
      }
   }

   if (! found) {
      return;   // no linkable namespaces in the list
   }

   // write list of namespaces
   ol.startMemberHeader("namespaces");

   //bool javaOpt    = Config::getBool("optimize-java");
   //bool fortranOpt = Config::getBool("optimize-fortran");

   ol.parseText(title);
   ol.endMemberHeader();
   ol.startMemberList();
    
   for (auto nd : *this) {

      if (nd->isLinkable() && nd->hasDocumentation()) {

         SrcLangExt lang = nd->getLanguage();

         if (lang == SrcLangExt_IDL && (isConstantGroup != nd->isConstantGroup())) {
            continue;   // will be output in another pass, see layout_default.xml
         }

         ol.startMemberDeclaration();
         ol.startMemberItem(nd->getOutputFileBase(), 0);

         QString ct = nd->compoundTypeString();
         ol.docify(ct);
         ol.docify(" ");
         ol.insertMemberAlign();

         QString name;
         if (localName) {
            name = nd->localName();
         } else {
            name = nd->displayName();
         }

         ol.writeObjectLink(nd->getReference(), nd->getOutputFileBase(), 0, name);
         ol.endMemberItem();

         if (! nd->briefDescription().isEmpty() && Config::getBool("brief-member-desc")) {

            ol.startMemberDescription(nd->getOutputFileBase());
            ol.generateDoc(nd->briefFile(), nd->briefLine(), nd, QSharedPointer<MemberDef>(), 
                  nd->briefDescription(), false, false, "", true);

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
   QSharedPointer<MemberList> ml = QMakeShared<MemberList>(lt);
   m_memberLists.append(ml);
 
   return ml;
}

void NamespaceDef::addMemberToList(MemberListType lt, QSharedPointer<MemberDef> md)
{  
   QSharedPointer<NamespaceDef> self = sharedFrom(this);
   QSharedPointer<MemberList> ml     = createMemberList(lt);

   static bool sortBriefDocs  = Config::getBool("sort-brief-docs");
   static bool sortMemberDocs = Config::getBool("sort-member-docs");

   bool isSorted = false;

   if (sortBriefDocs && (ml->listType() & MemberListType_declarationLists)) {
      isSorted = true;

   } else if (sortMemberDocs && (ml->listType() & MemberListType_documentationLists)) {
      isSorted = true;

   }

   if (isSorted) {
      ml->inSort(md);

   } else {
      ml->append(md);
   }

   if (ml->listType() & MemberListType_declarationLists) {
      md->setSectionList(self, ml);
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

void NamespaceDef::writeMemberDeclarations(OutputList &ol, MemberListType lt, const QString &title)
{
   QSharedPointer<NamespaceDef> self = sharedFrom(this);
   QSharedPointer<MemberList> ml     = getMemberList(lt);

   if (ml) {
      ml->writeDeclarations(ol, QSharedPointer<ClassDef>(), self, QSharedPointer<FileDef>(), QSharedPointer<GroupDef>(), title, 0);
   }
}

void NamespaceDef::writeMemberDocumentation(OutputList &ol, MemberListType lt, const QString &title)
{
   QSharedPointer<NamespaceDef> self = sharedFrom(this);
   QSharedPointer<MemberList> ml = getMemberList(lt);

   if (ml) {
      ml->writeDocumentation(ol, csPrintable(displayName()), self, title);
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

   static bool extractAnonNs = Config::getBool("extract-anon-namespaces");

   if (extractAnonNs && name().mid(i, 20) == "anonymous_namespace{") {                                          
      return true;
   }

   return ! name().isEmpty() && name().at(i) != '@' && (hasDocumentation() || getLanguage() == SrcLangExt_CSharp) && 
          ! isReference() && ! isHidden() && ! isArtificial();      
}

bool NamespaceDef::isLinkable() const
{
   return isLinkableInProject() || isReference();
}

QSharedPointer<MemberDef> NamespaceDef::getMemberByName(const QString &n) const
{
   QSharedPointer<MemberDef> md;

   if (m_allMembersDict && ! n.isEmpty()) {
      md = QSharedPointer<MemberDef> (m_allMembersDict->find(n));
   }

   return md;
}

QString NamespaceDef::title() const
{
   SrcLangExt lang = getLanguage();

   QString pageTitle;
   QString tempDisplay = displayName();

   if (lang == SrcLangExt_Java) {
      pageTitle = theTranslator->trPackage(tempDisplay);

   } else if (lang == SrcLangExt_Fortran) {
      pageTitle = theTranslator->trModuleReference(tempDisplay);

   } else if (lang == SrcLangExt_IDL) {

      pageTitle = isConstantGroup() ? theTranslator->trConstantGroupReference(tempDisplay)
                  : theTranslator->trModuleReference(tempDisplay);

   } else {
      pageTitle = theTranslator->trNamespaceReference(tempDisplay);

   }

   return pageTitle;
}

QString NamespaceDef::compoundTypeString() const
{
   SrcLangExt lang = getLanguage();

   if (lang == SrcLangExt_Java) {
      return "package";

   } else if (lang == SrcLangExt_CSharp) {
      return "namespace";

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
