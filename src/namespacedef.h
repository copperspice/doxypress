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

#ifndef NAMESPACEDEF_H
#define NAMESPACEDEF_H

#include <QHash>
#include <QStringList>
#include <QSharedPointer>
#include <QTextStream>

#include <definition.h>
#include <filenamelist.h>
#include <stringmap.h>

class ClassDef;
class ClassSDict;
class ConceptDef;
class MemberList;
class MemberDef;
class MemberGroupSDict;
class NamespaceList;
class NamespaceSDict;
class OutputList;

/** model of a namespace symbol */
class NamespaceDef : public Definition
{
 public:
   NamespaceDef(const QString &defFileName, int defLine, int defColumn, const QString &name, const QString &ref = QString(),
                QString refFile = QString(), const QString &type = QString(), bool isPublished = false);

   ~NamespaceDef();

   DefType definitionType() const override {
      return TypeNamespace;
   }

   QString getOutputFileBase() const override;

   QString anchor() const override {
      return QString();
   }

   void insertUsedFile(QSharedPointer<FileDef> fd);

   void writeDocumentation(OutputList &ol);
   void writeMemberPages(OutputList &ol);
   void writeQuickMemberLinks(OutputList &ol, QSharedPointer<MemberDef> currentMd) const override;

   void writeClassesToTagFile(QTextStream &tagFile, QSharedPointer<ClassSDict> list);
   void writeConceptsToTagFile(QTextStream &tagFile);
   void writeTagFile(QTextStream &);

   void insertClass(QSharedPointer<ClassDef> cd);
   void insertConcept(QSharedPointer<ConceptDef> cd);
   void insertNamespace(QSharedPointer<NamespaceDef> nd);
   void insertMember(QSharedPointer<MemberDef> md);

   void computeAnchors();
   void countMembers();
   int  numDocMembers() const;

   void addUsingDirective(QSharedPointer<NamespaceDef> nd);
   void addUsingDeclaration(QSharedPointer<Definition> def);

   const NamespaceSDict &getUsedNamespaces() const;
   const StringMap<QSharedPointer<Definition>> &getUsedClasses() const;

   void combineUsingRelations();
   QString displayName(bool = true) const override;
   QString localName() const;

   void setInlineNS(bool isInlineNS) {
      m_inlineNS = isInlineNS;
   }

   bool isInlineNS() const {
      return m_inlineNS;
   }

   bool isConstantGroup() const {
      return CONSTANT_GROUP == m_type;
   }

   bool isModule() const {
      return MODULE == m_type;
   }

   bool isLibrary() const {
      return LIBRARY == m_type;
   }

   bool isAnonymous() const;
   bool isAlias() const;

   bool isLinkableInProject() const override;
   bool isLinkable() const override;

   bool hasDetailedDescription() const;
   void addMembersToMemberGroup();
   void distributeMemberGroupDocumentation();
   void findSectionsInDocumentation();

   QSharedPointer<Definition> findInnerCompound(const QString &name) const override;
   void addInnerCompound(QSharedPointer<Definition> d) override;

   void addListReferences();

   void setFileName(const QString &fn);
   void setName(const QString &name) override;

   bool subGrouping() const {
      return m_subGrouping;
   }

   QSharedPointer<MemberList> getMemberList(MemberListType lt) const;

   const QList<QSharedPointer<MemberList>> &getMemberLists() const {
      return m_memberLists;
   }

   QSharedPointer<MemberDef> getMemberByName(const QString &) const;

   /*! Returns the user defined member groups */
   const MemberGroupSDict &getMemberGroupSDict() const {
      return m_memberGroupSDict;
   }

   /*! Returns the classes contained in this namespace */
   const ClassSDict &getClassSDict() const {
      return m_classSDict;
   }

   const ConceptSDict &getConceptSDict() const {
      return m_conceptSDict;
   }

   /*! Returns the namespaces contained in this namespace */
   const NamespaceSDict &getNamespaceSDict() const {
      return m_namespaceSDict;
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
   void writeConcepts(OutputList &ol, const QString &title);
   void writeInlineClasses(OutputList &ol);
   void writeNamespaceDeclarations(OutputList &ol, const QString &title, bool isConstantGroup = false);
   void writeMemberGroups(OutputList &ol);
   void writeAuthorSection(OutputList &ol);
   void startMemberDocumentation(OutputList &ol);
   void endMemberDocumentation(OutputList &ol);
   void writeSummaryLinks(OutputList &ol) override;
   void addNamespaceAttributes(OutputList &ol);

   bool m_subGrouping;
   bool m_isPublished;
   bool m_inlineNS;
   bool m_isAnonymous;

   QString  fileName;
   FileList files;

   StringMap<QSharedPointer<Definition>>  m_usingDeclMap;
   StringMap<QSharedPointer<Definition>>  m_innerCompounds;

   ClassSDict        m_classSDict;
   ConceptSDict      m_conceptSDict;
   NamespaceSDict    m_usingDirMap;
   NamespaceSDict    m_namespaceSDict;
   MemberSDict       m_allMembersDict;
   MemberGroupSDict  m_memberGroupSDict;

   QList<QSharedPointer<MemberList>> m_memberLists;

   enum { NAMESPACE, MODULE, CONSTANT_GROUP, LIBRARY } m_type;
};

#endif
