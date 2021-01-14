/************************************************************************
*
* Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
* Copyright (c) 1997-2014 Dimitri van Heesch
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
   : Definition(df, dl, dc, name), m_isPublished(isPublished)
{
   static const bool allowSubGrouping = Config::getBool("allow-sub-grouping");

   m_subGrouping = allowSubGrouping;

   if (! fName.isEmpty()) {
      fileName = stripExtension(fName);

   } else {
      setFileName(name);
   }

   setReference(lref);

   visited    = false;
   m_inlineNS = false;

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
}

void NamespaceDef::setFileName(const QString &fn)
{
  fileName = "namespace" + fn;
}

void NamespaceDef::distributeMemberGroupDocumentation()
{
   for (auto mg : m_memberGroupSDict) {
      mg->distributeMemberGroupDocumentation();
   }
}

const StringMap<QSharedPointer<Definition>> &NamespaceDef::getUsedClasses() const
{
   return m_usingDeclMap;
}

void NamespaceDef::findSectionsInDocumentation()
{
   QSharedPointer<NamespaceDef> self = sharedFrom(this);
   docFindSections(documentation(), self, QSharedPointer<MemberGroup>(), docFile());

   for (auto mg : m_memberGroupSDict) {
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
   static const bool sortMemberDocs = Config::getBool("sort-member-docs");

   if (fd == nullptr) {
      return;
   }

   if (! files.contains(fd)) {
      if (sortMemberDocs) {
         files.inSort(fd);

      } else {
         files.append(fd);
      }
   }
}

void NamespaceDef::addInnerCompound(QSharedPointer<Definition> d)
{
   m_innerCompounds.insert(d->localName(), d);

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
   // added 12/2015
   if (cd->isHidden()) {
      return;
   }

   if (m_classSDict.find(cd->name()) == nullptr) {
      m_classSDict.insert(cd->name(), cd);
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
         ::addMembersToMemberGroup(ml, m_memberGroupSDict, self);
      }
   }

   // add members inside sections to their groups
   for (auto mg : m_memberGroupSDict) {
      if (mg->allMembersInSameSection() && m_subGrouping) {
         mg->addToDeclarationSection();
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
      allMemberList = QMakeShared<MemberList>(MemberListType_allMembersList);
      m_memberLists.append(allMemberList);
   }

   allMemberList->append(md);
   m_allMembersDict.insert(md->localName(), md);

   switch (md->memberType()) {

      case MemberDefType::Variable:
         addMemberToList(MemberListType_decVarMembers, md);
         addMemberToList(MemberListType_docVarMembers, md);
         break;

      case MemberDefType::Function:
         addMemberToList(MemberListType_decFuncMembers, md);
         addMemberToList(MemberListType_docFuncMembers, md);
         break;

      case MemberDefType::Typedef:
         addMemberToList(MemberListType_decTypedefMembers, md);
         addMemberToList(MemberListType_docTypedefMembers, md);
         break;

      case MemberDefType::Enumeration:
         addMemberToList(MemberListType_decEnumMembers, md);
         addMemberToList(MemberListType_docEnumMembers, md);
         break;

      case MemberDefType::EnumValue:
         break;

      case MemberDefType::Define:
         addMemberToList(MemberListType_decDefineMembers, md);
         addMemberToList(MemberListType_docDefineMembers, md);
         break;

      default:
         err("NamespaceDef::insertMembers(): Member %s with class scope %s inserted in namespace scope `%s'\n",
             csPrintable(md->name()), md->getClassDef() ? csPrintable(md->getClassDef()->name()) : "", csPrintable(name()) );
   }

   // inline namespace - insert current member in the outer scope as a duplicate
   if (isInlineNS()) {
      QSharedPointer<Definition> outerScope = getOuterScope();

      if (outerScope != nullptr)  {

         if (outerScope->definitionType() == Definition::TypeNamespace) {
            outerScope.dynamicCast<NamespaceDef>()->insertMember(md);

            // emerald, may need to set a flag to mark this inlineNS

         } else if (outerScope->definitionType() == Definition::TypeFile) {
            outerScope.dynamicCast<FileDef>()->insertMember(md);

            // emerald, may need to set a flag to mark this inlineNS
         }
      }
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
   static const bool repeatBrief = Config::getBool("repeat-brief");

   return ((! briefDescription().isEmpty() && repeatBrief) || ! documentation().isEmpty());
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
            for (auto cd : m_classSDict) {
               if (cd->isLinkableInProject()) {
                  tagFile << "    <class kind=\"" << cd->compoundTypeString()
                          << "\">" << convertToXML(cd->name()) << "</class>" << endl;
               }
            }
         }
         break;

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
            for (auto mg : m_memberGroupSDict) {
               mg->writeTagFile(tagFile);
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
   static const bool repeatBrief = Config::getBool("repeat-brief");

   QSharedPointer<NamespaceDef> self = sharedFrom(this);

   if (hasDetailedDescription()) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
      ol.writeRuler();
      ol.popGeneratorState();
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);
      ol.writeAnchor("", "details");
      ol.popGeneratorState();
      ol.startGroupHeader();
      ol.parseText(title);
      ol.endGroupHeader();

      ol.startTextBlock();
      if (! briefDescription().isEmpty() && repeatBrief) {
         ol.generateDoc(briefFile(), briefLine(), self, QSharedPointer<MemberDef>(), briefDescription(), false, false);
      }

      if (! briefDescription().isEmpty() && repeatBrief && ! documentation().isEmpty()) {
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

      if (! documentation().isEmpty()) {
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
            ol.startTextLink(getOutputFileBase(), "details");
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
   static const bool separateMemberPages = Config::getBool("separate-member-pages");

   if (separateMemberPages) {
      ol.disable(OutputGenerator::Html);
      Doxy_Globals::suppressDocWarnings = true;
   }
}

void NamespaceDef::endMemberDocumentation(OutputList &ol)
{
   static const bool separateMemberPages = Config::getBool("separate-member-pages");

   if (separateMemberPages) {
      ol.enable(OutputGenerator::Html);
      Doxy_Globals::suppressDocWarnings = false;
   }
}

void NamespaceDef::writeClassDeclarations(OutputList &ol, const QString &title)
{
   if (m_classSDict.count() > 0) {
      m_classSDict.writeDeclaration(ol, 0, title, true);
   }
}

void NamespaceDef::writeInlineClasses(OutputList &ol)
{
   if (m_classSDict.count() > 0) {
      QSharedPointer<NamespaceDef> self = sharedFrom(this);
      m_classSDict.writeDocumentation(ol, self);
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

  for (auto mg : m_memberGroupSDict) {
      if ((! mg->allMembersInSameSection() || ! m_subGrouping) && mg->header() != "[NOHEADER]") {
         mg->writeDeclarations(ol, QSharedPointer<ClassDef>(), self, QSharedPointer<FileDef>(), QSharedPointer<GroupDef>());
      }
   }
}

void NamespaceDef::writeAuthorSection(OutputList &ol)
{
   static const QString projectName = Config::getString("project-name");

   // write Author section (Man only)
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Man);
   ol.startGroupHeader();
   ol.parseText(theTranslator->trAuthor(true, true));
   ol.endGroupHeader();
   ol.parseText(theTranslator->trGeneratedAutomatically(projectName));
   ol.popGeneratorState();
}

void NamespaceDef::writeSummaryLinks(OutputList &ol)
{
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   bool first = true;

   SrcLangExt lang = getLanguage();

   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Namespace)) {

      if ((lde->kind() == LayoutDocEntry::NamespaceClasses && m_classSDict.count() > 0 && m_classSDict.declVisible()) ||
            (lde->kind() == LayoutDocEntry::NamespaceNestedNamespaces && ! m_namespaceSDict.isEmpty() &&
             m_namespaceSDict.declVisible())) {

         LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
         QString label = lde->kind() == LayoutDocEntry::NamespaceClasses ? QString("nested-classes") : QString("namespaces");

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
   static const bool generateTreeView    = Config::getBool("generate-treeview");
   static const bool separateMemberPages = Config::getBool("separate-member-pages");

   // static bool outputJava = Config::getBool("optimize-java");
   // static bool fortranOpt = Config::getBool("optimize-fortran");

   QSharedPointer<NamespaceDef> self = sharedFrom(this);

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
            // functions & subroutines in fortran

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

   if (separateMemberPages) {
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

         if (md->getNamespaceDef() == this && md->isLinkable() && ! md->isEnumValue()) {
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

void NamespaceDef::countMembers()
{
   for (auto &ml : m_memberLists) {
      ml->countDecMembers();
      ml->countDocMembers();
   }

   for (auto &mg : m_memberGroupSDict) {
      mg->countDecMembers();
      mg->countDocMembers();
   }
}

int NamespaceDef::numDocMembers() const
{
   QSharedPointer<MemberList> allMemberList = getMemberList(MemberListType_allMembersList);

   return (allMemberList ? allMemberList->numDocMembers() : 0) + m_innerCompounds.count();
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

QString NamespaceDef::getOutputFileBase() const
{
   if (isReference()) {
      return fileName;

   } else {
      return convertNameToFile(fileName);
   }
}

QSharedPointer<Definition> NamespaceDef::findInnerCompound(const QString &n) const
{
   if (n.isEmpty()) {
      return QSharedPointer<Definition>();
   }

   QSharedPointer<Definition> d = m_innerCompounds.find(n);

   if (d == nullptr) {
      d = m_usingDirMap.find(n);

      if (d == nullptr) {
         d = m_usingDeclMap.find(n);
      }
   }

   return d;
}

void NamespaceDef::addListReferences()
{
   // bool fortranOpt = Config::getBool("optimize-fortran");

   QSharedPointer<NamespaceDef> self = sharedFrom(this);

   const QVector<ListItemInfo> &xrefItems = getRefItems();

   QString prefix;
   if (getLanguage() == SrcLangExt_Fortran) {
      prefix = theTranslator->trModule(true, true);
   } else {
      prefix = theTranslator->trNamespace(true, true);
   }

   addRefItem(xrefItems, qualifiedName(), prefix, getOutputFileBase(), displayName(), "", self);

   for (auto mg : m_memberGroupSDict) {
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
   static const bool briefMemberDesc = Config::getBool("brief-member-desc");

   // bool javaOpt    = Config::getBool("optimize-java");
   // bool fortranOpt = Config::getBool("optimize-fortran");

   if (count() == 0) {
      // no namespaces in the list
      return;
   }

   bool found = false;

   for (auto nd : *this) {

      if (found) {
         break;
      }

      if (nd->isLinkable() && nd->hasDocumentation()) {
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

         ol.writeObjectLink(nd->getReference(), nd->getOutputFileBase(), QString(), name);
         ol.endMemberItem();

         if (! nd->briefDescription().isEmpty() && briefMemberDesc) {

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
   static const bool sortBriefDocs  = Config::getBool("sort-brief-docs");
   static const bool sortMemberDocs = Config::getBool("sort-member-docs");

   QSharedPointer<NamespaceDef> self = sharedFrom(this);
   QSharedPointer<MemberList> ml     = createMemberList(lt);

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
      ml->writeDocumentation(ol, displayName(), self, title);
   }
}

bool NamespaceDef::isLinkableInProject() const
{
  static const bool extractAnonNameSpace = Config::getBool("extract-anon-namespaces");

   int i = name().lastIndexOf("::");

   if (i == -1) {
      i = 0;
   } else {
      i += 2;
   }

   if (extractAnonNameSpace && name().mid(i, 20) == "anonymous_namespace{") {
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

   if (! n.isEmpty()) {
      md = m_allMembersDict.find(n);
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
   QString retval;

   if (lang == SrcLangExt_Java) {
      retval = "package";

   } else if (lang == SrcLangExt_CSharp) {
      retval = "namespace";

   } else if (lang == SrcLangExt_Fortran) {
      retval = "module";

   } else if (lang == SrcLangExt_IDL) {

      if (isModule()) {
         retval = "module";

      } else if (isConstantGroup()) {
         retval = "constants";

      } else if (isLibrary()) {
         retval = "library";

      } else {
         err("Internal inconsistency: namespace in IDL not module, library or constant group\n");
      }
   }

   return retval;
}
