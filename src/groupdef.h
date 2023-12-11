/************************************************************************
*
* Copyright (c) 2014-2023 Barbara Geller & Ansel Sermersheim
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

#ifndef GROUPDEF_H
#define GROUPDEF_H

#include <QString>
#include <QSharedPointer>
#include <QTextStream>

#include <definition.h>
#include <filenamelist.h>
#include <stringmap.h>

class ClassDef;
class ConceptDef;
class DirDef;
class Entry;
class FileDef;
class FTVHelp;
class MemberDef;
class MemberList;
class NamespaceDef;
class OutputList;
class PageSDict;
class PageDef;

/** A model of a group of symbols. */
class GroupDef : public Definition
{
 public:
   GroupDef(const QString &fileName, int line, const QString &name, const QString &title, QString refFileName = QString());
   ~GroupDef();

   DefType definitionType() const override {
      return TypeGroup;
   }

   QString getOutputFileBase() const override;

   QString anchor() const  override {
      return QString();
   }

   QString displayName(bool = true) const override {
      return hasGroupTitle() ? m_title : Definition::name();
   }

   QString groupTitle() const {
      return m_title;
   }

   void setGroupTitle(const QString &newtitle);

   bool hasGroupTitle( ) const {
      return titleSet;
   }

   void addFile(QSharedPointer<FileDef> def);
   bool addClass(QSharedPointer<ClassDef> cd);
   bool addConcept(QSharedPointer<ConceptDef> conceptDef);
   bool addNamespace(QSharedPointer<NamespaceDef> def);
   void addGroup(QSharedPointer<GroupDef> def);
   void addPage(QSharedPointer<PageDef> def);
   void addExample(QSharedPointer<PageDef> def);
   void addDir(QSharedPointer<DirDef> def);

   bool insertMember(QSharedPointer<MemberDef> def, bool docOnly = false);
   void removeMember(QSharedPointer<MemberDef> md);
   bool findGroup(QSharedPointer<const GroupDef> def) const;                 // true if def is a subgroup of this group

   void writeDocumentation(OutputList &ol);
   void writeMemberPages(OutputList &ol);
   void writeQuickMemberLinks(OutputList &ol, QSharedPointer<MemberDef> currentMd) const override;
   void writeTagFile(QTextStream &);
   bool isLinkableInProject() const override;
   bool isLinkable() const override;
   bool isASubGroup() const;
   void computeAnchors();

   void countMembers();
   int  numDocMembers() const;

   void addMembersToMemberGroup();
   void distributeMemberGroupDocumentation();
   void findSectionsInDocumentation();

   void addListReferences();

   bool subGrouping() const {
      return m_subGrouping;
   }

   bool visited;    // number of times accessed for output

   // friend void writeGroupTreeNode(OutputList&, GroupDef*, int, FTVHelp*);
   // make accessible for writing tree view of group in index.cpp

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
   const MemberGroupSDict &getMemberGroupSDict() const {
      return m_memberGroupSDict;
   }

   const ClassSDict &getClasses() const {
      return m_classSDict;
   }

   const ConceptSDict &getConceptSDict() const {
      return m_conceptSDict;
   }

   const NamespaceSDict &getNamespaces() const {
      return m_namespaceSDict;
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

   const FileList &getFiles() const {
      return fileList;
   }

   SortedList<QSharedPointer<GroupDef>> *getSubGroups() const {
      return groupList;
   }

   bool hasDetailedDescription() const;

 protected:
   void addMemberListToGroup(QSharedPointer<MemberList>, bool (MemberDef::*)() const);
   QString pathFragment_Internal() const override;

 private:
   QSharedPointer<MemberList> createMemberList(MemberListType lt);

   void addMemberToList(MemberListType lt, QSharedPointer<MemberDef> md);
   void writeMemberDeclarations(OutputList &ol, MemberListType lt, const QString &title);
   void writeMemberDocumentation(OutputList &ol, MemberListType lt, const QString &title);
   void removeMemberFromList(MemberListType lt, QSharedPointer<MemberDef> md);
   void writeGroupGraph(OutputList &ol);
   void writeFiles(OutputList &ol, const QString &title);
   void writeNamespaces(OutputList &ol, const QString &title);
   void writeNestedGroups(OutputList &ol, const QString &title);
   void writeDirs(OutputList &ol, const QString &title);
   void writeClasses(OutputList &ol, const QString &title);
   void writeConcepts(OutputList &ol, const QString &title);
   void writeInlineClasses(OutputList &ol);
   void writePageDocumentation(OutputList &ol);
   void writeDetailedDescription(OutputList &ol, const QString &title);
   void writeBriefDescription(OutputList &ol);
   void writeMemberGroups(OutputList &ol);
   void startMemberDeclarations(OutputList &ol);
   void endMemberDeclarations(OutputList &ol);
   void startMemberDocumentation(OutputList &ol);
   void endMemberDocumentation(OutputList &ol);
   void writeAuthorSection(OutputList &ol);
   void writeSummaryLinks(OutputList &ol) override;

   void updateLanguage(QSharedPointer<const Definition> d);

   QString m_title;                     // title of the group
   bool titleSet;                       // true if title is not the same as the name
   QString fileName;                    // base name of the generated file

   ClassSDict      m_classSDict;        // list of classes in the group
   ConceptSDict    m_conceptSDict;
   NamespaceSDict  m_namespaceSDict;    // list of namespaces in the group
   PageSDict      *pageDict;            // list of pages in the group
   PageSDict      *exampleDict;         // list of examples in the group

   SortedList<QSharedPointer<DirDef>> *dirList;       // list of directories in the group
   FileList fileList;                                 // list of files in the group

   SortedList<QSharedPointer<GroupDef>> *groupList;   // list of sub groups

   QSharedPointer<MemberList> allMemberList;
   MemberNameInfoSDict *allMemberNameInfoSDict;

   QSharedPointer<Definition> groupScope;

   QList<QSharedPointer<MemberList>>  m_memberLists;
   MemberGroupSDict m_memberGroupSDict;

   bool m_subGrouping;
};

void addClassToGroups(QSharedPointer<Entry> root, QSharedPointer<ClassDef> cd);
void addConceptToGroups(QSharedPointer<Entry> root, QSharedPointer<ConceptDef> conceptDef);
void addNamespaceToGroups(QSharedPointer<Entry> root, QSharedPointer<NamespaceDef> nd);
void addGroupToGroups(QSharedPointer<Entry> root, QSharedPointer<GroupDef> subGroup);
void addMemberToGroups(QSharedPointer<Entry> root, QSharedPointer<MemberDef> md);
void addExampleToGroups(Entry *root, QSharedPointer<PageDef> eg);
void addDirToGroups(QSharedPointer<Entry> root, QSharedPointer<DirDef> dd);

#endif

