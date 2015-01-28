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

#ifndef GROUPDEF_H
#define GROUPDEF_H

#include <QByteArray>
#include <QString>
#include <QSharedPointer>
#include <QTextStream>

#include <definition.h>
#include <stringmap.h>

class ClassDef;
class ClassSDict;
class DirDef;
class Entry;
class FileList;
class FileDef;
class FTVHelp;
class MemberDef;
class MemberList;
class MemberGroupSDict;
class MemberNameInfoSDict;
class NamespaceDef;
class NamespaceSDict;
class OutputList;
class PageSDict;
class PageDef;

/** A model of a group of symbols. */
class GroupDef : public Definition
{
 public:
   GroupDef(const char *fileName, int line, const char *name, const char *title, QString refFileName = QString());
   ~GroupDef();

   DefType definitionType() const {
      return TypeGroup;
   }

   QByteArray getOutputFileBase() const;

   QByteArray anchor() const {
      return QByteArray();
   }

   QString displayName(bool = true) const override {
      return hasGroupTitle() ? title : Definition::name();
   }

   const char *groupTitle() const {
      return title;
   }

   void setGroupTitle( const char *newtitle );

   bool hasGroupTitle( ) const {
      return titleSet;
   }

   void addFile(QSharedPointer<FileDef> def);
   bool addClass(QSharedPointer<ClassDef> cd);
   bool addNamespace(QSharedPointer<NamespaceDef> def);
   void addGroup(QSharedPointer<GroupDef> def);  
   void addPage(QSharedPointer<PageDef> def);
   void addExample(QSharedPointer<PageDef> def);
   void addDir(QSharedPointer<DirDef> def);

   bool insertMember(QSharedPointer<MemberDef> def, bool docOnly = false);
   void removeMember(QSharedPointer<MemberDef> md);
   bool containsGroup(QSharedPointer<const GroupDef> def);    // true if def is already a subgroup
   void writeDocumentation(OutputList &ol);
   void writeMemberPages(OutputList &ol);
   void writeQuickMemberLinks(OutputList &ol, MemberDef *currentMd) const;
   void writeTagFile(QTextStream &);
   int  countMembers() const;
   bool isLinkableInProject() const;
   bool isLinkable() const;
   bool isASubGroup() const;
   void computeAnchors();

   void addMembersToMemberGroup();
   void distributeMemberGroupDocumentation();
   void findSectionsInDocumentation();

   void addListReferences();
  
   bool subGrouping() const {
      return m_subGrouping;
   }

   bool visited;    // number of times accessed for output - KPW

   //friend void writeGroupTreeNode(OutputList&, GroupDef*, int, FTVHelp*);
   // make accessible for writing tree view of group in index.cpp - KPW

   void setGroupScope(QSharedPointer<Definition> d) {
      groupScope = d;
   }
   QSharedPointer<Definition> getGroupScope() const {
      return groupScope;
   }

   QSharedPointer<MemberList> getMemberList(MemberListType lt) ;

   const QList<QSharedPointer<MemberList>> &getMemberLists() const {
      return m_memberLists;
   }

   /* user defined member groups */
   MemberGroupSDict *getMemberGroupSDict() const {
      return memberGroupSDict;
   }

   ClassSDict *getClasses() const {
      return classSDict;
   }

   NamespaceSDict *getNamespaces() const {
      return namespaceSDict;
   }

   PageSDict *getPages() const {
      return pageDict;
   }
  
   PageSDict *getExamples() const {
      return exampleDict;
   } 
 
   SortedList<QSharedPointer<DirDef>> *getDirs() const {
      return dirList;
   }

   FileList *getFiles() const {
      return fileList;
   }

   SortedList<QSharedPointer<GroupDef>> *getSubGroups() const {
      return groupList;
   }
    
   bool hasDetailedDescription() const;   
  
 protected:
   void addMemberListToGroup(QSharedPointer<MemberList>, bool (MemberDef::*)() const);

 private:
   QSharedPointer<MemberList> createMemberList(MemberListType lt);

   void addMemberToList(MemberListType lt, QSharedPointer<MemberDef> md);
   void writeMemberDeclarations(OutputList &ol, MemberListType lt, const QByteArray &title);
   void writeMemberDocumentation(OutputList &ol, MemberListType lt, const QByteArray &title);
   void removeMemberFromList(MemberListType lt, QSharedPointer<MemberDef> md);
   void writeGroupGraph(OutputList &ol);
   void writeFiles(OutputList &ol, const QByteArray &title);
   void writeNamespaces(OutputList &ol, const QByteArray &title);
   void writeNestedGroups(OutputList &ol, const QByteArray &title);
   void writeDirs(OutputList &ol, const QByteArray &title);
   void writeClasses(OutputList &ol, const QByteArray &title);
   void writeInlineClasses(OutputList &ol);
   void writePageDocumentation(OutputList &ol);
   void writeDetailedDescription(OutputList &ol, const QByteArray &title);
   void writeBriefDescription(OutputList &ol);
   void writeMemberGroups(OutputList &ol);
   void startMemberDeclarations(OutputList &ol);
   void endMemberDeclarations(OutputList &ol);
   void startMemberDocumentation(OutputList &ol);
   void endMemberDocumentation(OutputList &ol);
   void writeAuthorSection(OutputList &ol);
   void writeSummaryLinks(OutputList &ol);

   void updateLanguage(QSharedPointer<const Definition> d);

   QByteArray title;                    // title of the group
   bool titleSet;                       // true if title is not the same as the name
   QString fileName;                    // base name of the generated file
   
   ClassSDict *classSDict;              // list of classes in the group
   NamespaceSDict *namespaceSDict;      // list of namespaces in the group 
   PageSDict *pageDict;                 // list of pages in the group
   PageSDict *exampleDict;              // list of examples in the group

   SortedList<QSharedPointer<DirDef>> *dirList;       // list of directories in the group
   FileList *fileList;                  // list of files in the group

   SortedList<QSharedPointer<GroupDef>> *groupList;   // list of sub groups
  
   QSharedPointer<MemberList> allMemberList;
   MemberNameInfoSDict *allMemberNameInfoSDict;

   QSharedPointer<Definition> groupScope;

   QList<QSharedPointer<MemberList>>  m_memberLists;
   MemberGroupSDict  *memberGroupSDict;

   bool m_subGrouping;
};

/** A sorted dictionary of GroupDef objects. */
class GroupSDict : public StringMap<QSharedPointer<GroupDef>>
{
 public:
   GroupSDict() : StringMap<QSharedPointer<GroupDef>>() {}
   virtual ~GroupSDict() {}

 private:
   int compareValues(const GroupDef *item1, const GroupDef *item2) const {
      return qstrcmp(item1->groupTitle(), item2->groupTitle());
   }
};

void addClassToGroups(QSharedPointer<Entry> root, QSharedPointer<ClassDef> cd);
void addNamespaceToGroups(QSharedPointer<Entry> root, QSharedPointer<NamespaceDef> nd);
void addGroupToGroups(QSharedPointer<Entry> root, QSharedPointer<GroupDef> subGroup);
void addMemberToGroups(QSharedPointer<Entry> root, QSharedPointer<MemberDef> md);
void addExampleToGroups(Entry *root, QSharedPointer<PageDef> eg);
void addDirToGroups(QSharedPointer<Entry> root, QSharedPointer<DirDef> dd);

#endif

