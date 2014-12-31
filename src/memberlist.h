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

#ifndef MEMBERLIST_H
#define MEMBERLIST_H

#include <QList>

#include <cassert>

#include <memberdef.h>
#include <stringmap.h>
#include <sortedlist.h>
#include <types.h>

class GroupDef;
class MemberGroup;
class StorageIntf;

/** A list of MemberDef objects. */
class MemberList : public SortedList<MemberDef *>
{
 public:
   MemberList();
   MemberList(MemberListType lt);

   ~MemberList();

   MemberListType listType() const {
      return m_listType;
   }

   static QByteArray listTypeAsString(MemberListType type);

   void append(MemberDef *md);
   void insert(uint index, MemberDef *md); 
 
   int varCount() const       {
      assert(m_numDecMembers != -1);
      return m_varCnt;
   }

   int funcCount() const      {
      assert(m_numDecMembers != -1);
      return m_funcCnt;
   }

   int enumCount() const      {
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
   int numDecMembers() const  {
      assert(m_numDecMembers != -1);
      return m_numDecMembers;
   }
   int numDocMembers() const  {
      assert(m_numDocMembers != -1);
      return m_numDocMembers;
   }
  
   void countDecMembers(bool countEnumValues = false, GroupDef *gd = 0);
   void countDocMembers(bool countEnumValues = false);
   int countInheritableMembers(ClassDef *inheritedFrom) const;

   void writePlainDeclarations(OutputList &ol,
                               ClassDef *cd, NamespaceDef *nd, FileDef *fd, GroupDef *gd,
                               ClassDef *inheritedFrom, const char *inheritId);

   void writeDeclarations(OutputList &ol,
                          ClassDef *cd, NamespaceDef *nd, FileDef *fd, GroupDef *gd,
                          const char *title, const char *subtitle,
                          bool showEnumValues = false, bool showInline = false,
                          ClassDef *inheritedFrom = 0, MemberListType lt = MemberListType_pubMethods);

   void writeDocumentation(OutputList &ol, const char *scopeName,
                           Definition *container, const char *title, bool showEnumValues = false, bool showInline = false);

   void writeSimpleDocumentation(OutputList &ol, Definition *container);   

   void writeDocumentationPage(OutputList &ol, const char *scopeName, Definition *container);

   void writeDocumentationPage(OutputList &ol, const QString &scopeName, Definition *container)
   {
      writeDocumentationPage(ol, qPrintable(scopeName), container);
   }

   void writeDocumentationPage(OutputList &ol, const QByteArray &scopeName, Definition *container)
   {
      writeDocumentationPage(ol, scopeName.constData(), container);
   }

   void writeTagFile(FTextStream &);
   bool declVisible() const;
   void addMemberGroup(MemberGroup *mg);

   void setInGroup(bool inGroup) {
      m_inGroup = inGroup;
   }

   void setInFile(bool inFile) {
      m_inFile = inFile;
   }

   void addListReferences(Definition *def);
   void findSectionsInDocumentation();
 
   QList<MemberGroup> *getMemberGroupList() const {
      return memberGroupList;
   }

   void marshal(StorageIntf *s);
   void unmarshal(StorageIntf *s);

 private:
   int m_varCnt;
   int m_funcCnt;
   int m_enumCnt;
   int m_enumValCnt;
   int m_typeCnt;
   int m_protoCnt;
   int m_defCnt;
   int m_friendCnt;
   int m_numDecMembers;  // number of members in the brief part of the memberlist
   int m_numDocMembers;  // number of members in the detailed part of the memberlist

   QList<MemberGroup> *memberGroupList;

   bool m_inGroup;       // is this list part of a group definition
   bool m_inFile;        // is this list part of a file definition

   MemberListType m_listType; 
};

/** An unsorted dictionary of MemberDef objects. */
class MemberDict : public QHash<QString, MemberDef>
{
 public:
   MemberDict() : QHash<QString, MemberDef>() 
   {}

   virtual ~MemberDict() {}
};

/** A sorted dictionary of MemberDef objects. */
class MemberSDict : public StringMap<QSharedPointer<MemberDef>>
{
 public:
   MemberSDict() : StringMap<QSharedPointer<MemberDef>>() {}
   virtual ~MemberSDict() {}

 private:
   int compareValues(const MemberDef *item1, const MemberDef *item2) const;
};


#endif
