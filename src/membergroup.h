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

#ifndef MEMBERGROUP_H
#define MEMBERGROUP_H

#include <doxy_shared.h>
#include <types.h>

#include <QTextStream>
#include <QVector>

class ClassDef;
class Definition;
class FileDef;
class GroupDef;
class MemberDef;
class MemberList;
class NamespaceDef;
class OutputList;

struct ListItemInfo;

#define DOX_NOGROUP -1

/** A class representing a group of members. */
class MemberGroup : public EnableSharedFromThis
{
  public:
   MemberGroup();
   MemberGroup(QSharedPointer<Definition> parent, int id, const QString &header, const QString &docText,
         const QString &docFile, int docLine);

   ~MemberGroup();

   QString header() const {
      return grpHeader;
   }

   int groupId() const {
      return grpId;
   }

   void insertMember(QSharedPointer<MemberDef> md);
   void setAnchors();
   void writePlainDeclarations(OutputList &ol, QSharedPointer<ClassDef> cd, QSharedPointer<NamespaceDef> nd,
                  QSharedPointer<FileDef> fd, QSharedPointer<GroupDef> gd,
                  QSharedPointer<ClassDef> inheritedFrom, const QString &inheritId);

   void writeDeclarations(OutputList &ol, QSharedPointer<ClassDef> cd, QSharedPointer<NamespaceDef> nd,
                  QSharedPointer<FileDef> fd, QSharedPointer<GroupDef> gd, bool showInline = false);

   void writeDocumentation(OutputList &ol, const QString &scopeName, QSharedPointer<Definition> container,
                  bool showEnumValues, bool showInline);

   void writeDocumentationPage(OutputList &ol, const QString &scopeName, QSharedPointer<Definition> container);
   void writeTagFile(QTextStream &);

   void addGroupedInheritedMembers(OutputList &ol, QSharedPointer<ClassDef> cd, MemberListType lt,
                  QSharedPointer<ClassDef> inheritedFrom, const QString &inheritId);

   void setAnonymousEnumType();

   QString documentation() const {
      return doc;
   }

   bool allMembersInSameSection() const {
      return inSameSection;
   }

   void addToDeclarationSection();

   void countDecMembers();
   void countDocMembers();

   int countGroupedInheritedMembers(MemberListType lt);
   void distributeMemberGroupDocumentation();
   void findSectionsInDocumentation();

/*
   int varCount() const;
   int funcCount() const;
   int enumCount() const;
   int enumValueCount() const;
   int typedefCount() const;
   int protoCount() const;
   int defineCount() const;
   int friendCount() const;
*/

   int numDecMembers() const;
   int numDecEnumValues() const;
   int numDocMembers() const;
   int numDocEnumValues() const;

   int countInheritableMembers(QSharedPointer<ClassDef> inheritedFrom) const;
   void setInGroup(bool b);
   void addListReferences(QSharedPointer<Definition> d);
   void setRefItems(const QVector<ListItemInfo> &list);

   QSharedPointer<MemberList> members() const {
      return memberList;
   }

   QSharedPointer<Definition> parent() const {
      return m_parent;
   }

   QString anchor() const;

   QString docFile() const {
      return m_docFile;
   }

   int docLine() const {
      return m_docLine;
   }

 private:
   QSharedPointer<MemberList> memberList;      // list of all members in the group
   QSharedPointer<MemberList> inDeclSection;
   QSharedPointer<Definition> m_parent;

   QString grpHeader;
   QString fileName;                           // base name of the generated file
   QString doc;
   QString m_docFile;

   int m_docLine;
   int grpId;

   bool inSameSection;

   QVector<ListItemInfo> m_xrefListItems;
};

/** Data collected for a member group */
struct MemberGroupInfo {
   MemberGroupInfo() : docLine(-1)
   {
   }

   ~MemberGroupInfo()
   {
   }

   void setRefItems(const QVector<ListItemInfo> &list);

   QString header;
   QString compoundName;
   QString doc;
   QString docFile;

   int docLine;

   QVector<ListItemInfo> m_listInfo;
};

#endif
