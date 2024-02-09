/************************************************************************
*
* Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
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

#ifndef MEMBERLIST_H
#define MEMBERLIST_H

#include <memberdef.h>
#include <sortedlist.h>
#include <types.h>

#include <QList>
#include <QTextStream>

#include <cassert>

class GroupDef;
class MemberGroup;

/** A list of MemberDef objects. */
class MemberList : public SortedList<QSharedPointer<MemberDef>>, public EnableSharedFromThis
{
 public:
   MemberList();
   MemberList(MemberListType lt);

   ~MemberList();

   MemberListType listType() const {
      return m_listType;
   }

   static QString listTypeAsString(MemberListType type);

   void append(QSharedPointer<MemberDef> md);
   void insert(uint index, QSharedPointer<MemberDef> md);


/*
   int varCount() const {
      assert(m_numDecMembers != -1);
      return m_varCnt;
   }

   int funcCount() const {
      assert(m_numDecMembers != -1);
      return m_funcCnt;
   }

   int enumCount() const  {
      assert(m_numDecMembers != -1);
      return m_enumCnt;
   }

   int enumValueCount() const {
      assert(m_numDecMembers != -1);
      return m_enumValCnt;
   }

   int typedefCount() const   {
      assert(m_numDecMembers != -1);
      return m_typeCnt;
   }

   int protoCount() const     {
      assert(m_numDecMembers != -1);
      return m_protoCnt;
   }

   int defineCount() const    {
      assert(m_numDecMembers != -1);
      return m_defCnt;
   }

   int friendCount() const    {
      assert(m_numDecMembers != -1);
      return m_friendCnt;
   }
*/

   int numDecMembers() const  {
      assert(m_numDecMembers != -1);
      return m_numDecMembers;
   }

   int numDecEnumValues() const {
      return m_numDecEnumValues;
   }

   int numDocMembers() const  {
      assert(m_numDocMembers != -1);
      return m_numDocMembers;
   }
   int numDocEnumValues() const {
      return m_numDocEnumValues;
   }

   void countDecMembers();
   void countDocMembers();

   int countInheritableMembers(QSharedPointer<ClassDef> inheritedFrom) const;

   void writePlainDeclarations(OutputList &ol, QSharedPointer<ClassDef> cd, QSharedPointer<NamespaceDef> nd,
                  QSharedPointer<FileDef> fd, QSharedPointer<GroupDef> gd,
                  QSharedPointer<ClassDef> inheritedFrom, const QString &inheritId);

   void writeDeclarations(OutputList &ol, QSharedPointer<ClassDef> cd, QSharedPointer<NamespaceDef> nd,
                  QSharedPointer<FileDef> fd, QSharedPointer<GroupDef> gd,
                  const QString &title, const QString &subtitle,
                  bool showEnumValues = false, bool showInline = false,
                  QSharedPointer<ClassDef> inheritedFrom = QSharedPointer<ClassDef>(),
                  MemberListType lt = MemberListType_pubMethods);

   void writeDocumentation(OutputList &ol, const QString &scopeName, QSharedPointer<Definition> container,
                  const QString &title, bool showEnumValues = false, bool showInline = false);

   void writeSimpleDocumentation(OutputList &ol, QSharedPointer<Definition> container);
   void writeDocumentationPage(OutputList &ol, const QString &scopeName, QSharedPointer<Definition> container);

   void writeTagFile(QTextStream &);
   bool declVisible() const;
   void addMemberGroup(QSharedPointer<MemberGroup> mg);

   void setInGroup(bool inGroup) {
      m_inGroup = inGroup;
   }

   void setInFile(bool inFile) {
      m_inFile = inFile;
   }

   void addListReferences(QSharedPointer<Definition> def);
   void findSectionsInDocumentation();

   QList<MemberGroup> *getMemberGroupList() const {
      return m_memberGroupList;
   }

   void setAnonymousEnumType();

 private:
   int countEnumValues(QSharedPointer<MemberDef> md, bool setAnonEnumType) const;

/*
   int m_varCnt;
   int m_funcCnt;
   int m_enumCnt;
   int m_enumValCnt;
   int m_typeCnt;
   int m_protoCnt;
   int m_defCnt;
   int m_friendCnt;
*/
   int m_numDecMembers;      // number of members in the brief part of the memberlist
   int m_numDecEnumValues;

   int m_numDocMembers;      // number of members in the detailed part of the memberlist
   int m_numDocEnumValues;

   QList<MemberGroup> *m_memberGroupList;

   bool m_inGroup;           // part of a group definition
   bool m_inFile;            // part of a file definition

   MemberListType m_listType;
};

/** An unsorted dictionary of MemberDef objects. */
class MemberDict : public QHash<QString, MemberDef>
{
 public:
   MemberDict()
      : QHash<QString, MemberDef>()
   {
   }

   virtual ~MemberDict()
   {
   }
};


#endif
