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

#ifndef MEMBERGROUP_H
#define MEMBERGROUP_H

#include <QList>

#include <stringmap.h>
#include <types.h>

#define DOX_NOGROUP -1

class MemberList;
class MemberDef;
class ClassDef;
class NamespaceDef;
class FileDef;
class GroupDef;
class OutputList;
class Definition;
class StorageIntf;
class FTextStream;
struct ListItemInfo;

/** A class representing a group of members. */
class MemberGroup
{
 public:
   MemberGroup();
   MemberGroup(Definition *parent, int id, const char *header,
               const char *docs, const char *docFile);
   ~MemberGroup();
   QByteArray header() const {
      return grpHeader;
   }
   int groupId() const {
      return grpId;
   }
   void insertMember(MemberDef *md);
   void setAnchors();
   void writePlainDeclarations(OutputList &ol,
                               ClassDef *cd, NamespaceDef *nd, FileDef *fd, GroupDef *gd,
                               ClassDef *inheritedFrom, const char *inheritId);
   void writeDeclarations(OutputList &ol,
                          ClassDef *cd, NamespaceDef *nd, FileDef *fd, GroupDef *gd,
                          bool showInline = false);
   void writeDocumentation(OutputList &ol, const char *scopeName,
                           Definition *container, bool showEnumValues, bool showInline);
   void writeDocumentationPage(OutputList &ol, const char *scopeName,
                               Definition *container);
   void writeTagFile(FTextStream &);
   void addGroupedInheritedMembers(OutputList &ol, ClassDef *cd,
                                   MemberListType lt,
                                   ClassDef *inheritedFrom, const QByteArray &inheritId);

   const QByteArray &documentation() const {
      return doc;
   }
   bool allMembersInSameSection() const {
      return inSameSection;
   }
   void addToDeclarationSection();
   int countDecMembers(GroupDef *gd = 0);
   int countDocMembers();
   int countGroupedInheritedMembers(MemberListType lt);
   void distributeMemberGroupDocumentation();
   void findSectionsInDocumentation();
   int varCount() const;
   int funcCount() const;
   int enumCount() const;
   int enumValueCount() const;
   int typedefCount() const;
   int protoCount() const;
   int defineCount() const;
   int friendCount() const;
   int numDecMembers() const;
   int numDocMembers() const;
   int countInheritableMembers(ClassDef *inheritedFrom) const;
   void setInGroup(bool b);
   void addListReferences(Definition *d);
   void setRefItems(const QList<ListItemInfo> *sli);
   MemberList *members() const {
      return memberList;
   }
   Definition *parent() const {
      return m_parent;
   }
   QByteArray anchor() const;

   void marshal(StorageIntf *s);
   void unmarshal(StorageIntf *s);

 private:
   MemberList *memberList;      // list of all members in the group
   MemberList *inDeclSection;
   int grpId;
   QByteArray grpHeader;
   QByteArray fileName;           // base name of the generated file
   Definition *scope;
   QByteArray doc;
   bool inSameSection;
   int  m_numDecMembers;
   int  m_numDocMembers;
   Definition *m_parent;
   QByteArray m_docFile;
   QList<ListItemInfo> *m_xrefListItems;
};

/** A list of MemberGroup objects. */
class MemberGroupList : public QList<MemberGroup>
{
};

/** An iterator for MemberGroup objects in a MemberGroupList. */
class MemberGroupListIterator : public QListIterator<MemberGroup>
{
 public:
   MemberGroupListIterator(const MemberGroupList &l) :
      QListIterator<MemberGroup>(l) {}
};

/** A sorted dictionary of MemberGroup objects. */
class MemberGroupSDict : public LongMap<QSharedPointer<MemberGroup>>
{
 public:
   MemberGroupSDict() : LongMap<QSharedPointer<MemberGroup>>() {}
   ~MemberGroupSDict() {}

 private:
   int compareValues(const MemberGroup *item1, const MemberGroup *item2) const {
      return item1->groupId() - item2->groupId();
   }
};

/** Data collected for a member group */
struct MemberGroupInfo {
   MemberGroupInfo() : m_sli(0) {}
   ~MemberGroupInfo() {
      delete m_sli;
      m_sli = 0;
   }

   void setRefItems(const QList<ListItemInfo> *sli);

   QByteArray header;
   QByteArray doc;
   QByteArray docFile;
   QByteArray compoundName;
   QList<ListItemInfo> *m_sli;
};

#endif
