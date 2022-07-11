/************************************************************************
*
* Copyright (c) 2014-2022 Barbara Geller & Ansel Sermersheim
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

#include <QRegularExpression>

#include <ctype.h>

#include <arguments.h>
#include <config.h>
#include <classdef.h>
#include <doxy_globals.h>
#include <dot.h>
#include <docparser.h>
#include <entry.h>
#include <groupdef.h>
#include <language.h>
#include <layout.h>
#include <membername.h>
#include <message.h>
#include <membergroup.h>
#include <outputlist.h>
#include <util.h>

GroupDef::GroupDef(const QString &df, int dl, const QString &g_name, const QString &t, QString refFileName)
                  : Definition(df, dl, 1, g_name)
{
   pageDict       = new PageSDict();
   exampleDict    = new PageSDict();

   dirList        = new SortedList<QSharedPointer<DirDef>>;
   groupList      = new SortedList<QSharedPointer<GroupDef>>;

   allMemberNameInfoSDict = new MemberNameInfoSDict();

   if (! refFileName.isEmpty() ) {
      fileName = stripExtension(refFileName);

   } else {
      fileName = "group_" + QString(g_name);
   }

   setGroupTitle(t);

   allMemberList = QMakeShared<MemberList>(MemberListType_allMembersList);
   visited       = 0;

   m_subGrouping = Config::getBool("allow-sub-grouping");
}

GroupDef::~GroupDef()
{
   delete dirList;
   delete groupList;

   delete pageDict;
   delete exampleDict;
   delete allMemberNameInfoSDict;
}

void GroupDef::setGroupTitle(const QString &t)
{
   if ( ! t.isEmpty() ) {
      m_title  = t;
      titleSet = true;

   } else {
      m_title = name();
      m_title = upperCaseFirstLetter(std::move(m_title));

      titleSet   = false;
   }
}

QString GroupDef::pathFragment_Internal() const
{
   return groupTitle();
}

void GroupDef::distributeMemberGroupDocumentation()
{
   for (auto mg : m_memberGroupSDict) {
      mg->distributeMemberGroupDocumentation();
   }
}

void GroupDef::findSectionsInDocumentation()
{
   QSharedPointer<GroupDef> self = sharedFrom(this);
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

void GroupDef::addFile(QSharedPointer<FileDef> def)
{
   static const bool sortBriefDocs = Config::getBool("sort-brief-docs");

   if (def->isHidden()) {
      return;
   }

   updateLanguage(def);

   if (sortBriefDocs) {
      fileList.inSort(def);

   } else {
      fileList.append(def);
   }
}

bool GroupDef::addClass(QSharedPointer<ClassDef> cd)
{
   if (cd->isHidden()) {
      return false;
   }

   updateLanguage(cd);
   QString key = cd->name();

   if (m_classSDict.find(key) == nullptr) {
      m_classSDict.insert(key, cd);
      return true;
   }

   return false;
}

bool GroupDef::addNamespace(QSharedPointer<NamespaceDef> def)
{
   if (def->isHidden()) {
      return false;
   }

   updateLanguage(def);

   if (m_namespaceSDict.find(def->name()) == 0) {
      m_namespaceSDict.insert(def->name(), def);
      return true;
   }

   return false;
}

void GroupDef::addDir(QSharedPointer<DirDef> def)
{
   static const bool sortBriefDocs = Config::getBool("sort-brief-docs");

   if (def->isHidden()) {
      return;
   }

   if (sortBriefDocs) {
      dirList->inSort(def);

   } else {
      dirList->append(def);
   }
}

void GroupDef::addPage(QSharedPointer<PageDef> def)
{
   QSharedPointer<GroupDef> self = sharedFrom(this);

   if (def->isHidden()) {
      return;
   }

   pageDict->insert(def->name(), def);

   def->makePartOfGroup(self);
}

void GroupDef::addExample(QSharedPointer<PageDef> def)
{
   if (def->isHidden()) {
      return;
   }

   exampleDict->insert(def->name(), def);
}

void GroupDef::addMembersToMemberGroup()
{
   QSharedPointer<GroupDef> self = sharedFrom(this);

   for (auto ml : m_memberLists) {
      if (ml->listType() & MemberListType_declarationLists) {
         ::addMembersToMemberGroup(ml, m_memberGroupSDict, self);
      }
   }

   for (auto mg : m_memberGroupSDict) {
      mg->setInGroup(true);
   }
}

bool GroupDef::insertMember(QSharedPointer<MemberDef> md, bool docOnly)
{
   if (md->isHidden()) {
      return false;
   }

   updateLanguage(md);

   QSharedPointer<MemberNameInfo> mni;

   if ((mni = (*allMemberNameInfoSDict)[md->name()])) {
      // member with this name already found

      for (auto &srcMi : *mni) {
         QSharedPointer<MemberDef> srcMd = srcMi.memberDef;

         if (srcMd == md) {
            return false;   // already added before
         }

         // same class or namespace
         // both inside a file => definition and declaration do not have to be in the same file

         bool sameScope = srcMd->getOuterScope() == md->getOuterScope() ||
                  (srcMd->getOuterScope()->definitionType() == Definition::TypeFile &&
                  md->getOuterScope()->definitionType() == Definition::TypeFile);

         const ArgumentList &srcMdAl  = srcMd->getArgumentList();
         const ArgumentList &mdAl     = md->getArgumentList();
         const ArgumentList &tSrcMdAl = srcMd->getTemplateArgumentList();
         const ArgumentList &tMdAl    = md->getTemplateArgumentList();

         if ( (srcMd->isFunction() && md->isFunction()) && (tSrcMdAl.count() == tMdAl.count()) ) {

            bool ok = matchArguments2(srcMd->getOuterScope(), srcMd->getFileDef(), srcMdAl, md->getOuterScope(),
                     md->getFileDef(), mdAl, true);

            if (ok && sameScope) {
               if (srcMd->getGroupAlias() == 0) {
                  md->setGroupAlias(srcMd);

               } else if (md != srcMd->getGroupAlias()) {
                  md->setGroupAlias(srcMd->getGroupAlias());

               }

               return false; // member is the same as the one already added
            }
         }
      }

      mni->append(MemberInfo(md, md->protection(), md->virtualness(), false));

   } else {
      QSharedPointer<MemberNameInfo> tmp = QMakeShared<MemberNameInfo>(md->name());
      tmp->append(MemberInfo(md, md->protection(), md->virtualness(), false));

      allMemberNameInfoSDict->insert(tmp->memberName(), mni);
   }

   allMemberList->append(md);

   switch (md->memberType()) {

      case MemberDefType::Variable:
         if (! docOnly) {
            addMemberToList(MemberListType_decVarMembers, md);
         }

         addMemberToList(MemberListType_docVarMembers, md);
         break;

      case MemberDefType::Function:
         if (! docOnly) {
            addMemberToList(MemberListType_decFuncMembers, md);
         }

         addMemberToList(MemberListType_docFuncMembers, md);
         break;

      case MemberDefType::Typedef:
         if (! docOnly) {
            addMemberToList(MemberListType_decTypedefMembers, md);
         }

         addMemberToList(MemberListType_docTypedefMembers, md);
         break;

      case MemberDefType::Enumeration:
         if (! docOnly) {
            addMemberToList(MemberListType_decEnumMembers, md);
         }

         addMemberToList(MemberListType_docEnumMembers, md);
         break;

      case MemberDefType::EnumValue:
         if (! docOnly) {
            addMemberToList(MemberListType_decEnumValMembers, md);
         }

         addMemberToList(MemberListType_docEnumValMembers, md);
         break;

      case MemberDefType::Define:
         if (! docOnly) {
            addMemberToList(MemberListType_decDefineMembers, md);
         }

         addMemberToList(MemberListType_docDefineMembers, md);
         break;

      case MemberDefType::Signal:
         if (md->protection() == Public) {
            if (! docOnly) {
               addMemberToList(MemberListType_decPubSignalMembers, md);
            }
            addMemberToList(MemberListType_docPubSignalMembers, md);

         } else if (md->protection() == Protected) {
            if (! docOnly) {
               addMemberToList(MemberListType_decProSignalMembers, md);
            }
            addMemberToList(MemberListType_docProSignalMembers, md);

         } else if (md->protection() == Private) {
            if (! docOnly) {
               addMemberToList(MemberListType_decPriSignalMembers, md);
            }
            addMemberToList(MemberListType_docPriSignalMembers, md);

         }
         break;

      case MemberDefType::Slot:
         if (md->protection() == Public) {
            if (! docOnly) {
               addMemberToList(MemberListType_decPubSlotMembers, md);
            }
            addMemberToList(MemberListType_docPubSlotMembers, md);

         } else if (md->protection() == Protected) {
            if (! docOnly) {
               addMemberToList(MemberListType_decProSlotMembers, md);
            }
            addMemberToList(MemberListType_docProSlotMembers, md);

         } else {
            if (! docOnly) {
               addMemberToList(MemberListType_decPriSlotMembers, md);
            }
            addMemberToList(MemberListType_docPriSlotMembers, md);
         }
         break;

      case MemberDefType::Event:
         if (! docOnly) {
            addMemberToList(MemberListType_decEventMembers, md);
         }

         addMemberToList(MemberListType_docEventMembers, md);
         break;

      case MemberDefType::Property:
         if (! docOnly) {
            addMemberToList(MemberListType_decPropMembers, md);
         }

         addMemberToList(MemberListType_docPropMembers, md);
         break;

      default:
         err("GroupDef::insertMember(): Member `%s' (typeid = %d) with scope `%s' inserted in group scope `%s'\n",
             csPrintable(md->name()), csPrintable(md->memberType()),
             md->getClassDef() ? csPrintable(md->getClassDef()->name()) : "", csPrintable(name()) );
   }

   // this really goes here
   if (md->isFriend()) {
      if (! docOnly) {
         addMemberToList(MemberListType_decFriendMembers, md);
      }

      addMemberToList(MemberListType_docFriendMembers, md);
   }

   return true;
}

void GroupDef::removeMember(QSharedPointer<MemberDef> md)
{
   QSharedPointer<MemberNameInfo> mni(allMemberNameInfoSDict->find(md->name()));

   if (mni) {

      for (auto iter = mni->begin(); iter != mni->end(); ++iter) {

         if (iter->memberDef == md) {
            mni->erase(iter);
            break;
          }
      }

      if (mni->isEmpty()) {
         allMemberNameInfoSDict->remove(md->name());
      }

      removeMemberFromList(MemberListType_allMembersList, md);

      switch (md->memberType()) {
         case MemberDefType::Variable:
            removeMemberFromList(MemberListType_decVarMembers, md);
            removeMemberFromList(MemberListType_docVarMembers, md);
            break;

         case MemberDefType::Function:
            removeMemberFromList(MemberListType_decFuncMembers, md);
            removeMemberFromList(MemberListType_docFuncMembers, md);
            break;

         case MemberDefType::Typedef:
            removeMemberFromList(MemberListType_decTypedefMembers, md);
            removeMemberFromList(MemberListType_docTypedefMembers, md);
            break;

         case MemberDefType::Enumeration:
            removeMemberFromList(MemberListType_decEnumMembers, md);
            removeMemberFromList(MemberListType_docEnumMembers, md);
            break;

         case MemberDefType::EnumValue:
            removeMemberFromList(MemberListType_decEnumValMembers, md);
            removeMemberFromList(MemberListType_docEnumValMembers, md);
            break;

         case MemberDefType::Define:
            removeMemberFromList(MemberListType_decDefineMembers, md);
            removeMemberFromList(MemberListType_docDefineMembers, md);
            break;

         case MemberDefType::Signal:
            if (md->protection() == Public) {
               removeMemberFromList(MemberListType_decPubSignalMembers, md);
               removeMemberFromList(MemberListType_docPubSignalMembers, md);

            } else if (md->protection() == Protected) {
               removeMemberFromList(MemberListType_decProSignalMembers, md);
               removeMemberFromList(MemberListType_docProSignalMembers, md);

            } else {
               removeMemberFromList(MemberListType_decPriSignalMembers, md);
               removeMemberFromList(MemberListType_docPriSignalMembers, md);
            }
            break;


         case MemberDefType::Slot:
            if (md->protection() == Public) {
               removeMemberFromList(MemberListType_decPubSlotMembers, md);
               removeMemberFromList(MemberListType_docPubSlotMembers, md);

            } else if (md->protection() == Protected) {
               removeMemberFromList(MemberListType_decProSlotMembers, md);
               removeMemberFromList(MemberListType_docProSlotMembers, md);

            } else {
               removeMemberFromList(MemberListType_decPriSlotMembers, md);
               removeMemberFromList(MemberListType_docPriSlotMembers, md);
            }
            break;

         case MemberDefType::Event:
            removeMemberFromList(MemberListType_decEventMembers, md);
            removeMemberFromList(MemberListType_docEventMembers, md);
            break;

         case MemberDefType::Property:
            removeMemberFromList(MemberListType_decPropMembers, md);
            removeMemberFromList(MemberListType_docPropMembers, md);
            break;

         default:
            err("GroupDef::removeMember(): unexpected member remove in file\n");
      }

      if (md->isFriend()) {
         removeMemberFromList(MemberListType_decFriendMembers, md);
         removeMemberFromList(MemberListType_docFriendMembers, md);
      }
   }
}

bool GroupDef::findGroup(QSharedPointer<const GroupDef> def) const
{
   if (this == def) {
      return true;

   } else if (groupList) {

      for (auto gd : *groupList) {
         if (gd->findGroup(def)) {
            return true;
         }
      }
   }

   return false;
}

void GroupDef::addGroup(QSharedPointer<GroupDef> def)
{
   groupList->append(def);
}

bool GroupDef::isASubGroup() const
{
   SortedList<QSharedPointer<GroupDef>> *groups = partOfGroups();
   return groups != 0 && groups->count() != 0;
}

void GroupDef::countMembers()
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

int GroupDef::numDocMembers() const
{
   return fileList.count() + m_classSDict.count() + m_namespaceSDict.count() + groupList->count() +
          allMemberList->count() + pageDict->count() + exampleDict->count();
}

/*! Compute the HTML anchor names for all members in the group */
void GroupDef::computeAnchors()
{
   setAnchors(allMemberList);
}

void GroupDef::writeTagFile(QTextStream &tagFile)
{
   tagFile << "  <compound kind=\"group\">" << endl;
   tagFile << "    <name>" << convertToXML(name()) << "</name>" << endl;
   tagFile << "    <title>" << convertToXML(m_title) << "</title>" << endl;
   tagFile << "    <filename>" << convertToXML(getOutputFileBase()) << Doxy_Globals::htmlFileExtension << "</filename>" << endl;

   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Group)) {
      switch (lde->kind()) {

         case LayoutDocEntry::GroupClasses:
         {
            for (const auto cd : m_classSDict) {
               if (cd->isLinkableInProject()) {
                  tagFile << "    <class kind=\"" << cd->compoundTypeString()
                          << "\">" << convertToXML(cd->name()) << "</class>" << endl;
               }
            }
         }
         break;

         case LayoutDocEntry::GroupNamespaces:
         {
            for (const auto &nd : m_namespaceSDict) {
               if (nd->isLinkableInProject()) {
                  tagFile << "    <namespace>" << convertToXML(nd->name())
                          << "</namespace>" << endl;
               }
            }

         }
         break;

         case LayoutDocEntry::GroupFiles:
         {
            for (const auto item : fileList)  {
               if (item->isLinkableInProject()) {
                  tagFile << "    <file>" << convertToXML(item->name()) << "</file>" << endl;
               }
            }
         }
         break;

         case LayoutDocEntry::GroupPageDocs:
         {
            if (pageDict != nullptr) {
               for (const auto item : *pageDict)  {
                  QString pageName = item->getOutputFileBase();

                  if (item->isLinkableInProject()) {
                     tagFile << "    <page>" << convertToXML(pageName) << "</page>" << endl;
                  }
               }
            }
         }
         break;

         case LayoutDocEntry::GroupDirs:
         {
            if (dirList) {
               for (const auto item : *dirList)  {
                  if (item->isLinkableInProject()) {
                     tagFile << "    <dir>" << convertToXML(item->displayName()) << "</dir>" << endl;
                  }
               }
            }
         }
         break;

         case LayoutDocEntry::GroupNestedGroups:
         {
            if (groupList != nullptr) {
               for (const auto item : *groupList)  {
                  if (item->isVisible()) {
                     tagFile << "    <subgroup>" << convertToXML(item->name()) << "</subgroup>" << endl;
                  }
               }
            }
         }
         break;

         case LayoutDocEntry::MemberDecl: {
            LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
            QSharedPointer<MemberList> ml = getMemberList(lmd->type);

            if (ml) {
               ml->writeTagFile(tagFile);
            }
         }
         break;

         case LayoutDocEntry::MemberGroups: {

            for (const auto mg : m_memberGroupSDict) {
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

void GroupDef::writeDetailedDescription(OutputList &ol, const QString &title)
{
   static const bool repeatBrief = Config::getBool("repeat-brief");

   QSharedPointer<GroupDef> self = sharedFrom(this);

   if ((! briefDescription().isEmpty() && repeatBrief)
         || ! documentation().isEmpty() || ! inbodyDocumentation().isEmpty()) {

      ol.pushGeneratorState();

      if (pageDict->count() != numDocMembers()) {
         // not only pages -> classical layout

         ol.pushGeneratorState();
         ol.disable(OutputGenerator::Html);
         ol.writeRuler();
         ol.popGeneratorState();

         ol.pushGeneratorState();
         ol.disableAllBut(OutputGenerator::Html);
         ol.writeAnchor("", "details");
         ol.popGeneratorState();

      } else {
         ol.disableAllBut(OutputGenerator::Man);    // always print title for man page
      }

      ol.startGroupHeader();
      ol.parseText(title);
      ol.endGroupHeader();
      ol.popGeneratorState();

      // repeat brief description
      if (! briefDescription().isEmpty() && repeatBrief) {
         ol.generateDoc(briefFile(), briefLine(), self, QSharedPointer<MemberDef>(), briefDescription(), false, false);
      }

      // write separator between brief and details
      if (! briefDescription().isEmpty() && repeatBrief && ! documentation().isEmpty()) {
         ol.pushGeneratorState();
         ol.disable(OutputGenerator::Man);
         ol.disable(OutputGenerator::RTF);

         // ol.newParagraph(); // FIXME:PARA

         ol.enableAll();
         ol.disableAllBut(OutputGenerator::Man);
         ol.enable(OutputGenerator::Latex);
         ol.writeString("\n\n");
         ol.popGeneratorState();
      }

      // write detailed documentation
      if (! documentation().isEmpty()) {
         ol.generateDoc(docFile(), docLine(), self, QSharedPointer<MemberDef>(), documentation() + "\n", true, false);
      }

      // write inbody documentation
      if (! inbodyDocumentation().isEmpty()) {
         ol.generateDoc(inbodyFile(), inbodyLine(), self, QSharedPointer<MemberDef>(),
                  inbodyDocumentation() + "\n", true, false);
      }
   }
}

void GroupDef::writeBriefDescription(OutputList &ol)
{
   QSharedPointer<GroupDef> self = sharedFrom(this);

   if (hasBriefDescription()) {
      DocRoot *rootNode = validatingParseDoc(briefFile(), briefLine(), self, QSharedPointer<MemberDef>(),
                                             briefDescription(), true, false, "", true, false);

      if (rootNode && !rootNode->isEmpty()) {
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
            ol.startTextLink(QString(), "details");
            ol.parseText(theTranslator->trMore());
            ol.endTextLink();
         }

         ol.popGeneratorState();
         ol.endParagraph();
      }

      delete rootNode;
   }

   ol.writeSynopsis();
}

void GroupDef::writeGroupGraph(OutputList &ol)
{
   static const bool haveDot = Config::getBool("have-dot");

   QSharedPointer<GroupDef> self = sharedFrom(this);

   if (haveDot) {
      DotGroupCollaboration graph(self);

      if (! graph.isTrivial()) {
         msg("Generating dependency graph for group %s\n", csPrintable(qualifiedName()));
         ol.pushGeneratorState();
         ol.disable(OutputGenerator::Man);
         //ol.startParagraph();
         ol.startGroupCollaboration();
         ol.parseText(theTranslator->trCollaborationDiagram(m_title));
         ol.endGroupCollaboration(graph);
         //ol.endParagraph();
         ol.popGeneratorState();
      }
   }
}

void GroupDef::writeFiles(OutputList &ol, const QString &title)
{
   static const bool briefMemberDesc = Config::getBool("brief-member-desc");

   if (fileList.count() > 0) {
      ol.startMemberHeader("files");
      ol.parseText(title);
      ol.endMemberHeader();
      ol.startMemberList();

      for (auto item : fileList) {

         if (! item->hasDocumentation()) {
            continue;
         }

         ol.startMemberDeclaration();
         ol.startMemberItem(item->getOutputFileBase(), 0);
         ol.docify(theTranslator->trFile(false, true) + " ");

         ol.insertMemberAlign();
         ol.writeObjectLink(item->getReference(), item->getOutputFileBase(), QString(), item->name());
         ol.endMemberItem();

         if (! item->briefDescription().isEmpty() && briefMemberDesc) {
            ol.startMemberDescription(item->getOutputFileBase());
            ol.generateDoc(briefFile(), briefLine(), item, QSharedPointer<MemberDef>(),
                           item->briefDescription(), false, false, "", true, false);

            ol.endMemberDescription();
         }
         ol.endMemberDeclaration(QString(), QString());
      }

      ol.endMemberList();
   }
}

void GroupDef::writeNamespaces(OutputList &ol, const QString &title)
{
   // write list of namespaces
   m_namespaceSDict.writeDeclaration(ol, title);
}

void GroupDef::writeNestedGroups(OutputList &ol, const QString &title)
{
   static const bool briefMemberDesc = Config::getBool("brief-member-desc");
   static const bool sortGroupNames  = Config::getBool("sort-group-names");

   // write list of groups
   int count = 0;

   if (groupList->count() > 0) {
      for (auto gd : *groupList) {
         if (gd->isVisible()) {
            count++;
         }
      }
   }

   if (count > 0) {
      ol.startMemberHeader("groups");
      ol.parseText(title);
      ol.endMemberHeader();
      ol.startMemberList();

      if (sortGroupNames) {
         groupList->sort();
      }

      for (auto gd : *groupList) {
         if (gd->isVisible()) {

            if (! gd->hasDocumentation()) {
               continue;
            }

            ol.startMemberDeclaration();
            ol.startMemberItem(gd->getOutputFileBase(), 0);

            ol.insertMemberAlign();
            ol.writeObjectLink(gd->getReference(), gd->getOutputFileBase(), QString(), gd->groupTitle());
            ol.endMemberItem();

            if (! gd->briefDescription().isEmpty() && briefMemberDesc) {
               ol.startMemberDescription(gd->getOutputFileBase());
               ol.generateDoc(briefFile(), briefLine(), gd, QSharedPointer<MemberDef>(), gd->briefDescription(),
                              false, false, "", true, false);

               ol.endMemberDescription();
            }
            ol.endMemberDeclaration(QString(), QString());
         }
      }
      ol.endMemberList();
   }
}

void GroupDef::writeDirs(OutputList &ol, const QString &title)
{
   static const bool briefMemberDesc = Config::getBool("brief-member-desc");

   // write list of directories
   if (dirList->count() > 0) {
      ol.startMemberHeader("dirs");
      ol.parseText(title);
      ol.endMemberHeader();
      ol.startMemberList();

      for (const auto dd : *dirList ) {

         if (! dd->hasDocumentation()) {
            continue;
         }

         ol.startMemberDeclaration();
         ol.startMemberItem(dd->getOutputFileBase(), 0);
         ol.parseText(theTranslator->trDir(false, true));

         ol.insertMemberAlign();
         ol.writeObjectLink(dd->getReference(), dd->getOutputFileBase(), QString(), dd->shortName());
         ol.endMemberItem();

         if (! dd->briefDescription().isEmpty() && briefMemberDesc) {
            ol.startMemberDescription(dd->getOutputFileBase());
            ol.generateDoc(briefFile(), briefLine(), dd, QSharedPointer<MemberDef>(), dd->briefDescription(),
                           false, false, QString(), true, false);

            ol.endMemberDescription();
         }
         ol.endMemberDeclaration(QString(), QString());
      }

      ol.endMemberList();
   }
}

void GroupDef::writeClasses(OutputList &ol, const QString &title)
{
   // write list of classes
   m_classSDict.writeDeclaration(ol, nullptr, title, false);
}

void GroupDef::writeInlineClasses(OutputList &ol)
{
   m_classSDict.writeDocumentation(ol);
}

void GroupDef::writePageDocumentation(OutputList &ol)
{
   for (auto pd : *pageDict) {
      if (! pd->isReference()) {
         QSharedPointer<SectionInfo> si;

         if (! pd->title().isEmpty() && ! pd->name().isEmpty() &&
                  (si = Doxy_Globals::sectionDict.find(pd->name())) != 0) {

            ol.startSection(si->label, si->title, SectionInfo::Subsection);
            ol.docify(si->title);
            ol.endSection(si->label, SectionInfo::Subsection);
         }

         ol.startTextBlock();
         ol.generateDoc(pd->docFile(), pd->docLine(), pd, QSharedPointer<MemberDef>(),
                  pd->documentation() + pd->inbodyDocumentation(), true, false, "", true, false);

         ol.endTextBlock();
      }
   }
}

void GroupDef::writeMemberGroups(OutputList &ol)
{
   QSharedPointer<GroupDef> self = sharedFrom(this);

   // write user defined member groups

   for (auto mg : m_memberGroupSDict) {
      mg->writeDeclarations(ol, QSharedPointer<ClassDef>(), QSharedPointer<NamespaceDef>(),
                 QSharedPointer<FileDef>(), self);
   }
}

void GroupDef::startMemberDeclarations(OutputList &ol)
{
   ol.startMemberSections();
}

void GroupDef::endMemberDeclarations(OutputList &ol)
{
   ol.endMemberSections();
}

void GroupDef::startMemberDocumentation(OutputList &ol)
{
   static const bool separateMemberPages  = Config::getBool("separate-member-pages");

   if (separateMemberPages) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
      Doxy_Globals::suppressDocWarnings = true;
   }
}

void GroupDef::endMemberDocumentation(OutputList &ol)
{
   static const bool separateMemberPages  = Config::getBool("separate-member-pages");

   if (separateMemberPages) {
      ol.popGeneratorState();
      Doxy_Globals::suppressDocWarnings = false;
   }
}

void GroupDef::writeAuthorSection(OutputList &ol)
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

void GroupDef::writeSummaryLinks(OutputList &ol)
{
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   bool first = true;
   SrcLangExt lang = getLanguage();

   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Group) ) {

      if ((lde->kind() == LayoutDocEntry::GroupClasses && m_classSDict.declVisible()) ||
            (lde->kind() == LayoutDocEntry::GroupNamespaces   && m_namespaceSDict.declVisible()) ||
            (lde->kind() == LayoutDocEntry::GroupFiles        && fileList.count()   > 0) ||
            (lde->kind() == LayoutDocEntry::GroupNestedGroups && groupList->count() > 0) ||
            (lde->kind() == LayoutDocEntry::GroupDirs         && dirList->count()   > 0) ) {

         LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;

         QString label;

         if (lde->kind() == LayoutDocEntry::GroupClasses) {
            label = "nested-classes";

         } else if (lde->kind() == LayoutDocEntry::GroupNamespaces)  {
            label = "namespaces";

         } else if (lde->kind() == LayoutDocEntry::GroupFiles)  {
            label = "files";

         } else if (lde->kind() == LayoutDocEntry::GroupNestedGroups) {
            label = "groups";

         } else {
            label = "dirs";
         }

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

void GroupDef::writeDocumentation(OutputList &ol)
{
   static const bool separateMemberPages  = Config::getBool("separate-member-pages");

   QSharedPointer<GroupDef> self = sharedFrom(this);

   ol.pushGeneratorState();
   startFile(ol, getOutputFileBase(), name(), m_title, HLI_Modules);

   ol.startHeaderSection();
   writeSummaryLinks(ol);

   ol.startTitleHead(getOutputFileBase());
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Man);
   ol.parseText(m_title);
   ol.popGeneratorState();
   addGroupListToTitle(ol, self);

   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Man);
   ol.endTitleHead(getOutputFileBase(), m_title);
   ol.popGeneratorState();
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Man);
   ol.endTitleHead(getOutputFileBase(), name());

   ol.popGeneratorState();
   ol.endHeaderSection();
   ol.startContents();

   if (Doxy_Globals::searchIndexBase != nullptr) {
      Doxy_Globals::searchIndexBase->setCurrentDoc(self, anchor(), false);

      static QRegularExpression regExp("[a-zA-Z_][-a-zA-Z_0-9]*");
      QRegularExpressionMatch match = regExp.match(m_title);

      while (match.hasMatch()) {
         // for each word in the title

         Doxy_Globals::searchIndexBase->addWord(match.captured(), true);
         match = regExp.match(m_title, match.capturedEnd());
      }
   }

   Doxy_Globals::indexList.addIndexItem(self, QSharedPointer<MemberDef>(), QString(), m_title);

   // start flexible part

   SrcLangExt lang = getLanguage();

   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Group)) {
      switch (lde->kind()) {
         case LayoutDocEntry::BriefDesc:
            writeBriefDescription(ol);
            break;

         case LayoutDocEntry::MemberDeclStart:
            startMemberDeclarations(ol);
            break;

         case LayoutDocEntry::GroupClasses: {
            LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
            writeClasses(ol, ls->title(lang));
         }
         break;

         case LayoutDocEntry::GroupInlineClasses: {
            writeInlineClasses(ol);
         }
         break;

         case LayoutDocEntry::GroupNamespaces: {
            LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
            writeNamespaces(ol, ls->title(lang));
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
         case LayoutDocEntry::MemberDef: {
            LayoutDocEntryMemberDef *lmd = (LayoutDocEntryMemberDef *)lde;
            writeMemberDocumentation(ol, lmd->type, lmd->title(lang));
         }
         break;

         case LayoutDocEntry::MemberDefEnd:
            endMemberDocumentation(ol);
            break;

         case LayoutDocEntry::GroupNestedGroups: {
            LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
            writeNestedGroups(ol, ls->title(lang));
         }
         break;

         case LayoutDocEntry::GroupPageDocs:
            writePageDocumentation(ol);
            break;

         case LayoutDocEntry::GroupDirs: {
            LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
            writeDirs(ol, ls->title(lang));
         }
         break;
         case LayoutDocEntry::GroupFiles: {
            LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
            writeFiles(ol, ls->title(lang));
         }
         break;

         case LayoutDocEntry::GroupGraph:
            writeGroupGraph(ol);
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
         case LayoutDocEntry::NamespaceNestedNamespaces:
         case LayoutDocEntry::NamespaceNestedConstantGroups:
         case LayoutDocEntry::NamespaceClasses:
         case LayoutDocEntry::NamespaceInlineClasses:
         case LayoutDocEntry::FileClasses:
         case LayoutDocEntry::FileNamespaces:
         case LayoutDocEntry::FileConstantGroups:
         case LayoutDocEntry::FileIncludes:
         case LayoutDocEntry::FileIncludeGraph:
         case LayoutDocEntry::FileIncludedByGraph:
         case LayoutDocEntry::FileSourceLink:
         case LayoutDocEntry::FileInlineClasses:
         case LayoutDocEntry::DirSubDirs:
         case LayoutDocEntry::DirFiles:
         case LayoutDocEntry::DirGraph:
            err("Internal inconsistency: member %d should not be part of "
                "LayoutDocManager::Group entry list\n", lde->kind());
            break;
      }
   }

   //---------------------------------------- end flexible part -------------------------------

   endFile(ol, false, false);

   ol.popGeneratorState();

   if (separateMemberPages) {
      allMemberList->sort();
      writeMemberPages(ol);
   }
}

void GroupDef::writeMemberPages(OutputList &ol)
{
   QSharedPointer<GroupDef> self = sharedFrom(this);

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   for (auto ml : m_memberLists) {
      if (ml->listType() & MemberListType_documentationLists) {
         ml->writeDocumentationPage(ol, name(), self);
      }
   }

   ol.popGeneratorState();
}

void GroupDef::writeQuickMemberLinks(OutputList &ol, QSharedPointer<MemberDef> currentMd) const
{
   static bool createSubDirs = Config::getBool("create-subdirs");

   ol.writeString("      <div class=\"navtab\">\n");
   ol.writeString("        <table>\n");

   for (auto md : *allMemberList) {

      if (md->getGroupDef() == this && md->isLinkable() && !md->isEnumValue()) {
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

   ol.writeString("        </table>\n");
   ol.writeString("      </div>\n");
}

void addClassToGroups(QSharedPointer<Entry> root, QSharedPointer<ClassDef> cd)
{
    for (auto &g : root->m_groups) {
      QSharedPointer<GroupDef> gd;

      if (! g.groupname.isEmpty() && (gd = Doxy_Globals::groupSDict.find(g.groupname))) {
         if (gd->addClass(cd)) {
            cd->makePartOfGroup(gd);
         }
      }
   }
}

void addNamespaceToGroups(QSharedPointer<Entry> root, QSharedPointer<NamespaceDef> nd)
{
   for (auto &g : root->m_groups) {
      QSharedPointer<GroupDef> gd;

      if (! g.groupname.isEmpty() && (gd = Doxy_Globals::groupSDict.find(g.groupname))) {
         if (gd->addNamespace(nd)) {
            nd->makePartOfGroup(gd);
         }
      }
   }
}

void addDirToGroups(QSharedPointer<Entry> root, QSharedPointer<DirDef> dd)
{
   for (auto &g : root->m_groups) {
       QSharedPointer<GroupDef> gd;

      if (! g.groupname.isEmpty() && (gd = Doxy_Globals::groupSDict.find(g.groupname))) {
         gd->addDir(dd);
         dd->makePartOfGroup(gd);
      }
   }
}

void addGroupToGroups(QSharedPointer<Entry> root, QSharedPointer<GroupDef> subGroup)
{
   for (auto &g : root->m_groups) {
      QSharedPointer<GroupDef> gd;

      if (! g.groupname.isEmpty() && (gd = Doxy_Globals::groupSDict.find(g.groupname))) {

         if (gd == subGroup) {
            warn(root->getData(EntryKey::File_Name), root->startLine, "Refusing to add group %s to itself", csPrintable(gd->name()));

         } else if (subGroup->findGroup(gd)) {
            warn(root->getData(EntryKey::File_Name), root->startLine, "Refusing to add group %s to group %s, since the latter is already a "
                 "subgroup of the former\n", csPrintable(subGroup->name()), csPrintable(gd->name()));

         } else if (! gd->findGroup(subGroup)) {
            gd->addGroup(subGroup);
            subGroup->makePartOfGroup(gd);
         }
      }
   }
}

/*! Add a member to the group with the highest priority */
void addMemberToGroups(QSharedPointer<Entry> root, QSharedPointer<MemberDef> md)
{
   // Search entry's group list for group with highest pri.

   Grouping::GroupPri_t pri = Grouping::GROUPING_LOWEST;
   QSharedPointer<GroupDef> fgd;

   for (auto &g : root->m_groups) {
      QSharedPointer<GroupDef> gd;

      if (! g.groupname.isEmpty() && (gd = Doxy_Globals::groupSDict.find(g.groupname)) && g.pri >= pri) {

         if (fgd && gd != fgd && g.pri == pri) {
            warn(root->getData(EntryKey::File_Name), root->startLine, "Member %s found in multiple %s groups. "
                 "The member will be put in group %s, and not in group %s",
                 csPrintable(md->name()), csPrintable(Grouping::getGroupPriName(pri)),
                 csPrintable(gd->name()), csPrintable(fgd->name()) );
         }

         fgd = gd;
         pri = g.pri;
      }
   }

   // put member into group defined by this entry?
   if (fgd) {
      QSharedPointer<GroupDef> mgd = md->getGroupDef();

      bool insertit = false;

      if (mgd == 0) {
         insertit = true;

      } else if (mgd != fgd) {
         bool moveit = false;

         // move member from one group to another if
         // - the new one has a higher priority
         // - the new entry has the same priority, but with docs where the old one had no docs

         if (md->getGroupPri() < pri) {
            moveit = true;

         } else {

            if (md->getGroupPri() == pri) {
               if (! root->getData(EntryKey::Main_Docs).isEmpty() && !md->getGroupHasDocs()) {
                  moveit = true;

               } else if (! root->getData(EntryKey::Main_Docs).isEmpty() && md->getGroupHasDocs()) {

                  warn(md->getGroupFileName(), md->getGroupStartLine(),
                       "Member documentation for %s found several times in %s groups\n"
                       "%s:%d: The member will remain in group %s, and will not be put into group %s",
                       csPrintable(md->name()), csPrintable(Grouping::getGroupPriName(pri)),
                       csPrintable(root->getData(EntryKey::File_Name)), root->startLine, csPrintable(mgd->name()), csPrintable(fgd->name()) );
               }
            }
         }

         if (moveit) {
            mgd->removeMember(md);
            insertit = true;
         }
      }

      if (insertit) {
         bool success = fgd->insertMember(md);

         if (success) {
            md->setGroupDef(fgd, pri, root->getData(EntryKey::File_Name), root->startLine, ! root->getData(EntryKey::Main_Docs).isEmpty());
            QSharedPointer<ClassDef> cd = md->getClassDefOfAnonymousType();

            if (cd) {
               cd->setGroupDefForAllMembers(fgd, pri, root->getData(EntryKey::File_Name), root->startLine,
                  root->getData(EntryKey::Main_Docs).length() != 0);
            }
         }
      }
   }
}

void addExampleToGroups(Entry *root, QSharedPointer<PageDef> eg)
{
   for (auto &g : root->m_groups) {
      QSharedPointer<GroupDef> gd;

      if (! g.groupname.isEmpty() && (gd = Doxy_Globals::groupSDict.find(g.groupname))) {
         gd->addExample(eg);
         eg->makePartOfGroup(gd);
      }
   }
}

QString GroupDef::getOutputFileBase() const
{
   if (isReference()) {
      return fileName;

   } else {
      return convertNameToFile(fileName);

   }
}

void GroupDef::addListReferences()
{
   QSharedPointer<GroupDef> self = sharedFrom(this);

   const QVector<ListItemInfo> &xrefItems = getRefItems();

   QString title;

   if (Config::getBool("bb-style"))  {
      // maybe allow a user defined value  12/2015
      title = "Component";

   } else {
      title = theTranslator->trGroup(true, true);
   }

   addRefItem(xrefItems, getOutputFileBase(), title, getOutputFileBase(),
            name(), QString(), QSharedPointer<Definition>() );

   for (auto mg : m_memberGroupSDict) {
      mg->addListReferences(self);
   }

   for (auto ml : m_memberLists) {
      if (ml->listType() & MemberListType_documentationLists) {
         ml->addListReferences(self);
      }
   }
}

QSharedPointer<MemberList> GroupDef::createMemberList(MemberListType lt)
{
   for (auto ml : m_memberLists) {
      if (ml->listType() == lt) {
         return ml;
      }
   }

   // not found, create a new member list
   QSharedPointer<MemberList> ml = QMakeShared<MemberList>(lt);

   ml->setInGroup(true);
   m_memberLists.append(ml);

   return m_memberLists.last();
}

void GroupDef::addMemberToList(MemberListType lt, QSharedPointer<MemberDef> md)
{
   static const bool sortBriefDocs  = Config::getBool("sort-brief-docs");
   static const bool sortMemberDocs = Config::getBool("sort-member-docs");

   QSharedPointer<MemberList> ml = createMemberList(lt);

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
}

QSharedPointer<MemberList> GroupDef::getMemberList(MemberListType lt)
{
   for (auto ml : m_memberLists) {
      if (ml->listType() == lt) {
         return ml;
      }
   }

   return QSharedPointer<MemberList>();
}

void GroupDef::writeMemberDeclarations(OutputList &ol, MemberListType lt, const QString &title)
{
   QSharedPointer<GroupDef> self = sharedFrom(this);
   QSharedPointer<MemberList> ml = getMemberList(lt);

   if (ml) {
      ml->writeDeclarations(ol, QSharedPointer<ClassDef>(), QSharedPointer<NamespaceDef>(),
            QSharedPointer<FileDef>(), self, title, QString());
   }
}

void GroupDef::writeMemberDocumentation(OutputList &ol, MemberListType lt, const QString &title)
{
   QSharedPointer<GroupDef> self = sharedFrom(this);
   QSharedPointer<MemberList> ml = getMemberList(lt);

   if (ml) {
      ml->writeDocumentation(ol, name(), self, title);
   }
}

void GroupDef::removeMemberFromList(MemberListType lt, QSharedPointer<MemberDef> md)
{
    QSharedPointer<MemberList> ml = getMemberList(lt);

   if (ml) {
      ml->removeOne(md);
   }
}

bool GroupDef::isLinkableInProject() const
{
   return ! isReference() && isLinkable();
}

bool GroupDef::isLinkable() const
{
   if ( hasUserDocumentation() ) {
      return true;
   }

   // no documentation, check if there are classes
   for (auto cd : m_classSDict) {

      if (! cd->name().isEmpty()) {
         return true;
      }
   }

   // may want to check for other group memmbers

   return false;
}

// let the "programming language" for a group depend on what is inserted into it.
// First item that has an associated languages determines the language for the whole group.

void GroupDef::updateLanguage(QSharedPointer<const Definition> d)
{
   if (getLanguage() == SrcLangExt_Unknown && d->getLanguage() != SrcLangExt_Unknown) {
      setLanguage(d->getLanguage());
   }
}

bool GroupDef::hasDetailedDescription() const
{
   static const bool repeatBrief = Config::getBool("repeat-brief");

   bool a = (! briefDescription().isEmpty() && repeatBrief);
   bool b = (! documentation().isEmpty() || ! inbodyDocumentation().isEmpty());

   bool retval  = (a || b) && (pageDict->count() != numDocMembers());

   return retval;
}