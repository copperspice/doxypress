/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
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

#ifndef NAMESPACEDEF_H
#define NAMESPACEDEF_H

#include <QHash>
#include <QStringList>
#include <QSharedPointer>
#include <QTextStream>

#include <definition.h>
#include <filenamelist.h>

class ClassDef;
class ClassSDict;
class MemberList;
class MemberDef;
class MemberGroupSDict;
class NamespaceList;
class NamespaceSDict;
class OutputList;

template<typename T>
class StringMap;

/** model of a namespace symbol */
class NamespaceDef : public Definition
{
 public:
   NamespaceDef(const QString &defFileName, int defLine, int defColumn, const QString &name, const QString &ref = QString(),
                QString refFile = QString(), const QString &type = QString(), bool isPublished = false);

   ~NamespaceDef();

   DefType definitionType() const {
      return TypeNamespace;
   }

   QString getOutputFileBase() const override;

   QString anchor() const override {
      return "";
   }

   void insertUsedFile(QSharedPointer<FileDef> fd);

   void writeDocumentation(OutputList &ol);
   void writeMemberPages(OutputList &ol);
   void writeQuickMemberLinks(OutputList &ol, QSharedPointer<MemberDef> currentMd) const;
   void writeTagFile(QTextStream &);

   void insertClass(QSharedPointer<ClassDef> cd);
   void insertNamespace(QSharedPointer<NamespaceDef> nd);
   void insertMember(QSharedPointer<MemberDef> md);

   void computeAnchors();
   int countMembers();

   void addUsingDirective(QSharedPointer<NamespaceDef> nd);
   void addUsingDeclaration(QSharedPointer<Definition> def);

   const NamespaceSDict &getUsedNamespaces() const;

   StringMap<QSharedPointer<Definition>> &getUsedClasses();

   void combineUsingRelations();
   QString displayName(bool = true) const override;
   QString localName() const;

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
 
   virtual QSharedPointer<Definition> findInnerCompound(const QString &name) override;
   virtual void addInnerCompound(QSharedPointer<Definition> d) override;

   void addListReferences();

   bool subGrouping() const {
      return m_subGrouping;
   }

   QSharedPointer<MemberList> getMemberList(MemberListType lt) const;

   const QList<QSharedPointer<MemberList>> &getMemberLists() const {
      return m_memberLists;
   }

   QSharedPointer<MemberDef> getMemberByName(const QString &) const;

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

   QString title() const;
   QString compoundTypeString() const;

   bool visited;

 private:
   QSharedPointer<MemberList> createMemberList(MemberListType lt);

   void addMemberToList(MemberListType lt, QSharedPointer<MemberDef> md);
   void writeMemberDeclarations(OutputList &ol, MemberListType lt, const QString &title);
   void writeMemberDocumentation(OutputList &ol, MemberListType lt, const QString &title);
   void writeDetailedDescription(OutputList &ol, const QString &title);
   void writeBriefDescription(OutputList &ol);
   void startMemberDeclarations(OutputList &ol);
   void endMemberDeclarations(OutputList &ol);
   void writeClassDeclarations(OutputList &ol, const QString &title);
   void writeInlineClasses(OutputList &ol);
   void writeNamespaceDeclarations(OutputList &ol, const QString &title, bool isConstantGroup = false);
   void writeMemberGroups(OutputList &ol);
   void writeAuthorSection(OutputList &ol);
   void startMemberDocumentation(OutputList &ol);
   void endMemberDocumentation(OutputList &ol);
   void writeSummaryLinks(OutputList &ol);
   void addNamespaceAttributes(OutputList &ol);

   bool m_subGrouping;
   bool m_isPublished;

   QString  fileName;
   FileList files;
  
   StringMap<QSharedPointer<Definition>>  *m_usingDeclMap;
   StringMap<QSharedPointer<Definition>>  *m_innerCompounds;

   NamespaceSDict    *m_usingDirMap;
   NamespaceSDict    *namespaceSDict;
   MemberSDict       *m_allMembersDict;
   MemberGroupSDict  *memberGroupSDict;
   ClassSDict        *classSDict;
  
   QList<QSharedPointer<MemberList>> m_memberLists;
 
   enum { NAMESPACE, MODULE, CONSTANT_GROUP, LIBRARY } m_type;   
};

#endif
