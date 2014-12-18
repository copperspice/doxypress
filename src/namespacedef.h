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

#ifndef NAMESPACEDEF_H
#define NAMESPACEDEF_H

#include <QStringList>
#include <QHash>

#include <stringmap.h>
#include <definition.h>
#include <filedef.h>

class ClassDef;
class ClassSDict;
class FTextStream;
class MemberList;
class MemberDef;
class MemberGroupSDict;
class NamespaceSDict;
class NamespaceList;
class OutputList;

/** A sorted dictionary of NamespaceDef objects. */
class NamespaceSDict : public StringMap<QSharedPointer<NamespaceDef>>
{
 public:
   NamespaceSDict() : StringMap<QSharedPointer<NamespaceDef>>() {}
   ~NamespaceSDict() {}

   void writeDeclaration(OutputList &ol, const char *title, bool isConstantGroup = false, bool localName = false);
   bool declVisible() const;

 private:
   int compareValues(const NamespaceDef *item1, const NamespaceDef *item2) const;       
};


/** A model of a namespace symbol. */
class NamespaceDef : public Definition
{
 public:
   NamespaceDef(const char *defFileName, int defLine, int defColumn, const char *name, const char *ref = 0,
                const char *refFile = 0, const char *type = 0, bool isPublished = false);

   ~NamespaceDef();

   DefType definitionType() const {
      return TypeNamespace;
   }

   QByteArray getOutputFileBase() const;

   QByteArray anchor() const {
      return QByteArray();
   }

   void insertUsedFile(FileDef *fd);

   void writeDocumentation(OutputList &ol);
   void writeMemberPages(OutputList &ol);
   void writeQuickMemberLinks(OutputList &ol, MemberDef *currentMd) const;
   void writeTagFile(FTextStream &);

   void insertClass(QSharedPointer<ClassDef> cd);
   void insertNamespace(QSharedPointer<NamespaceDef> nd);
   void insertMember(QSharedPointer<MemberDef> md);

   void computeAnchors();
   int countMembers();

   void addUsingDirective(QSharedPointer<NamespaceDef> nd);
   void addUsingDeclaration(QSharedPointer<Definition> def);

   const NamespaceSDict &getUsedNamespaces() const;

   const StringMap<QSharedPointer<Definition>> &getUsedClasses() const {
      return m_usingDeclMap;
   }

   void combineUsingRelations();
   QByteArray displayName(bool = true) const;
   QByteArray localName() const;

   bool isConstantGroup() const {
      return CONSTANT_GROUP == m_type;
   }

   bool isModule() const {
      return MODULE == m_type;
   }

   bool isLibrary() const {
      return LIBRARY == m_type;
   }

   bool isLinkableInProject() const;
   bool isLinkable() const;
   bool hasDetailedDescription() const;
   void addMembersToMemberGroup();
   void distributeMemberGroupDocumentation();
   void findSectionsInDocumentation();
 
   virtual Definition *findInnerCompound(const char *name);

   virtual void addInnerCompound(QSharedPointer<Definition> d) override;

   void addListReferences();

   bool subGrouping() const {
      return m_subGrouping;
   }

   MemberList *getMemberList(MemberListType lt) const;

   const QList<MemberList *> &getMemberLists() const {
      return m_memberLists;
   }

   MemberDef *getMemberByName(const QByteArray &) const;

   /*! Returns the user defined member groups */
   MemberGroupSDict *getMemberGroupSDict() const {
      return memberGroupSDict;
   }

   /*! Returns the classes contained in this namespace */
   ClassSDict *getClassSDict() const {
      return classSDict;
   }

   /*! Returns the namespaces contained in this namespace */
   NamespaceSDict *getNamespaceSDict() const {
      return namespaceSDict;
   }

   QByteArray title() const;
   QByteArray compoundTypeString() const;

   bool visited;

 private:
   MemberList *createMemberList(MemberListType lt);
   void addMemberToList(MemberListType lt, MemberDef *md);
   void writeMemberDeclarations(OutputList &ol, MemberListType lt, const QByteArray &title);
   void writeMemberDocumentation(OutputList &ol, MemberListType lt, const QByteArray &title);
   void writeDetailedDescription(OutputList &ol, const QByteArray &title);
   void writeBriefDescription(OutputList &ol);
   void startMemberDeclarations(OutputList &ol);
   void endMemberDeclarations(OutputList &ol);
   void writeClassDeclarations(OutputList &ol, const QByteArray &title);
   void writeInlineClasses(OutputList &ol);
   void writeNamespaceDeclarations(OutputList &ol, const QByteArray &title, bool isConstantGroup = false);
   void writeMemberGroups(OutputList &ol);
   void writeAuthorSection(OutputList &ol);
   void startMemberDocumentation(OutputList &ol);
   void endMemberDocumentation(OutputList &ol);
   void writeSummaryLinks(OutputList &ol);
   void addNamespaceAttributes(OutputList &ol);

   bool m_subGrouping;
   bool m_isPublished;

   QByteArray  fileName;
   FileList    files;
  
   StringMap<QSharedPointer<Definition>>   m_usingDeclMap;
   StringMap<QSharedPointer<Definition>>  *m_innerCompounds;

   NamespaceSDict        m_usingDirMap;
   NamespaceSDict       *namespaceSDict;
   MemberSDict          *m_allMembersDict;
   MemberGroupSDict     *memberGroupSDict;
   ClassSDict           *classSDict;
  
   QList<MemberList *>   m_memberLists;
 
   enum { NAMESPACE, MODULE, CONSTANT_GROUP, LIBRARY } m_type;
   
};

/** A list of NamespaceDef objects. */
class NamespaceList : public QList<NamespaceDef>
{
 public:
   ~NamespaceList() {}

   int compareValues(const NamespaceDef *nd1, const NamespaceDef *nd2) const {
      return qstricmp(nd1->name(), nd2->name());
   }
};

/** An iterator for NamespaceDef objects in a NamespaceList. */
class NamespaceListIterator : public QListIterator<NamespaceDef>
{
 public:
   NamespaceListIterator(const NamespaceList &l) :
      QListIterator<NamespaceDef>(l) {}
};

/** An unsorted dictionary of NamespaceDef objects. */
class NamespaceDict : public QHash<QString,NamespaceDef>
{
 public:
   NamespaceDict() : QHash<QString, NamespaceDef>() {}
   ~NamespaceDict() {}
};

#endif
