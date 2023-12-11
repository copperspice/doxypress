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

#ifndef FILEDEF_H
#define FILEDEF_H

#include <QList>
#include <QHash>
#include <QTextStream>

#include <definition.h>
#include <filenamelist.h>
#include <index.h>
#include <sortedlist.h>
#include <stringmap.h>

class ClassDef;
class ConceptDef;
class DirDef;
class DirEntryTree;
class FileDef;
class MemberDef;
class MemberList;
class OutputList;
class PackageDef;

/** Class representing the data associated with a \#include statement. */
struct IncludeInfo {
   IncludeInfo() {
      local    = false;
      imported = false;
      indirect = false;
   }

   ~IncludeInfo() {}

   QSharedPointer<FileDef> fileDef;

   QString includeName;
   bool local;
   bool imported;
   bool indirect;
};

/** A model of a file symbol.
 *
 *  An object of this class contains all file information that is gathered.
 *  This includes the members and compounds defined in the file.
 *
 *  The member writeDocumentation() can be used to generate the page of
 *  documentation to HTML and LaTeX.
 */
class FileDef : public Definition
{
   friend class FileNameList;

 public:
   // enum FileType { Source, Header, Unknown };

   FileDef(const QString &p, const QString &n, const QString &ref = QString(), const QString &dn = QString());
   ~FileDef();

   DefType definitionType() const override {
      return TypeFile;
   }

   /*! Returns the unique file name (this may include part of the path). */
   QString name() const;

   QString displayName(bool = true) const override {
      return name();
   }

   QString fileName() const {
      return m_fileName;
   }

   QString getOutputFileBase() const override {
      return convertNameToFile(m_diskName);
   }

   QString anchor() const override {
      return QString();
   }

   QString getFileBase() const {
      return m_diskName;
   }

   QString getSourceFileBase() const override;

   /*! Returns the name of the verbatim copy of this file (if any). */
   QString includeName() const;

   /*! Returns the absolute path including the file name. */
   QString getFilePath() const {
      return m_filePath;
   }

   /*! Returns the name as it is used in the documentation */
   const QString &docName() const {
      return m_docname;
   }

   /*! Returns true if this file is a source file. */
   bool isSource() const {
      return m_isSource;
   }

   bool isDocumentationFile() const;

   QSharedPointer<Definition> getSourceDefinition(int lineNr) const;
   QSharedPointer<MemberDef> getSourceMember(int lineNr) const;

   /*! Returns the absolute path of this file. */
   QString getPath() const {
      return m_path;
   }

   /*! Returns version of this file. */
   QString getVersion() const {
      return m_fileVersion;
   }

   bool isLinkableInProject() const override;

   bool isLinkable() const override {
      return isLinkableInProject() || isReference();
   }

   bool isIncluded(const QString &name) const;

   QSharedPointer<PackageDef> packageDef() const {
      return m_package;
   }

   QSharedPointer<DirDef> getDirDef() const {
      return m_dir;
   }

   const NamespaceSDict *getUsedNamespaces() const;

   const StringMap<QSharedPointer<Definition>> *getUsedClasses() const {
      return &m_usingDeclList;
   }

   const QList<IncludeInfo> *includeFileList() const  {
      if (m_includeList.isEmpty()) {
         return nullptr;
      }

      return &m_includeList;
   }

   QList<IncludeInfo> *includeFileList() {
      if (m_includeList.isEmpty()) {
         return 0;
      }

      return &m_includeList;
   }

   QList<IncludeInfo> *includedByFileList() {
      return &m_includedByList;
   }

   void getAllIncludeFilesRecursively(QStringList &incFiles) const;

   QSharedPointer<MemberList> getMemberList(MemberListType lt) const;

   const QList<QSharedPointer<MemberList>> &getMemberLists() const {
      return m_memberLists;
   }

   /* user defined member groups */
   const ClassSDict &getClassSDict()  {
      return m_classSDict;
   }

   const ConceptSDict &getConceptSDict() {
      return m_conceptSDict;
   }

   const MemberGroupSDict &getMemberGroupSDict() {
      return m_memberGroupSDict;
   }

   const NamespaceSDict &getNamespaceSDict() {
      return m_namespaceSDict;
   }

   QString title() const;
   bool hasDetailedDescription() const;
   QString fileVersion() const;

   bool subGrouping() const {
      return m_subGrouping;
   }

   void countMembers();
   int numDocMembers() const;
   int numDecMembers() const;

   void addSourceRef(int line, QSharedPointer<Definition> d, QSharedPointer<MemberDef> md);

   void writeDocumentation(OutputList &ol);
   void writeMemberPages(OutputList &ol);
   void writeQuickMemberLinks(OutputList &ol, QSharedPointer<MemberDef> currentMd) const override;
   void writeSummaryLinks(OutputList &ol) override;
   void writeTagFile(QTextStream &t);

   void startParsing();
   void writeSource(OutputList &ol, bool sameTu, QStringList &filesInSameTu);
   void parseSource(bool sameTu, QStringList &filesInSameTu);
   void finishParsing();

   friend void generatedFileNames();

   void insertMember(QSharedPointer<MemberDef> md);
   void insertClass(QSharedPointer<ClassDef> cd);
   void insertConcept(QSharedPointer<ConceptDef> conceptDef);
   void insertNamespace(QSharedPointer<NamespaceDef> nd);
   void computeAnchors();

   void setPackageDef(QSharedPointer<PackageDef> pd) {
      m_package = pd;
   }

   void setDirDef(QSharedPointer<DirDef> dd) {
      m_dir = dd;
   }

   void addUsingDirective(QSharedPointer<NamespaceDef> nd);
   void addUsingDeclaration(QSharedPointer<Definition> def);
   void combineUsingRelations();

   bool generateSourceFile() const;

   void addIncludeDependency(QSharedPointer<FileDef> fd, const QString &incName, bool local, bool imported, bool indirect);
   void addIncludedByDependency(QSharedPointer<FileDef> fd, const QString &incName, bool local, bool imported);

   void addMembersToMemberGroup();
   void distributeMemberGroupDocumentation();
   void findSectionsInDocumentation();
   void addIncludedUsingDirectives();

   void addListReferences();

   bool visited;

 protected:
   /**
    * Retrieves the file version from version control system.
    */
   void acquireFileVersion();

 private:
   QSharedPointer<MemberList> createMemberList(MemberListType lt);

   void addMemberToList(MemberListType lt, QSharedPointer<MemberDef> md);
   void writeMemberDeclarations(OutputList &ol, MemberListType lt, const QString &title);
   void writeMemberDocumentation(OutputList &ol, MemberListType lt, const QString &title);
   void writeIncludeFiles(OutputList &ol);
   void writeIncludeGraph(OutputList &ol);
   void writeIncludedByGraph(OutputList &ol);
   void writeMemberGroups(OutputList &ol);
   void writeAuthorSection(OutputList &ol);
   void writeSourceLink(OutputList &ol);
   void writeNamespaceDeclarations(OutputList &ol, const QString &title, bool isConstantGroup);
   void writeClassDeclarations(OutputList &ol, const QString &title);
   void writeInlineClasses(OutputList &ol);
   void startMemberDeclarations(OutputList &ol);
   void endMemberDeclarations(OutputList &ol);
   void startMemberDocumentation(OutputList &ol);
   void endMemberDocumentation(OutputList &ol);
   void writeDetailedDescription(OutputList &ol, const QString &title);
   void writeBriefDescription(OutputList &ol);

   QHash<QString, IncludeInfo>   m_includeDict;
   QList<IncludeInfo>            m_includeList;
   QHash<QString, IncludeInfo>   m_includedByDict;
   QList<IncludeInfo>            m_includedByList;
   NamespaceSDict                m_usingDirList;

   StringMap<QSharedPointer<Definition>>  m_usingDeclList;

   QString m_path;
   QString m_filePath;
   QString m_diskName;
   QString m_fileName;
   QString m_docname;

   QHash<long, QSharedPointer<Definition>> m_srcDefDict;
   QHash<long, QSharedPointer<MemberDef>>  m_srcMemberDict;

   bool               m_isSource;
   QString            m_fileVersion;

   QSharedPointer<PackageDef> m_package;

   QSharedPointer<DirDef> m_dir;
   QList<QSharedPointer<MemberList>> m_memberLists;

   ClassSDict         m_classSDict;
   ConceptSDict       m_conceptSDict;
   MemberGroupSDict   m_memberGroupSDict;
   NamespaceSDict     m_namespaceSDict;

   bool               m_subGrouping;
};

/** Class representing an entry (file or sub directory) in a directory */
class DirEntry
{
 public:
   enum EntryKind { Dir, File };

   DirEntry(QSharedPointer<DirEntry> parent, QSharedPointer<FileDef> fd)
      : m_parent(parent), m_name(fd->name()), m_kind(File), m_fd(fd), m_isLast(false)
   { }

   DirEntry(QSharedPointer<DirEntry> parent, QString name)
      : m_parent(parent), m_name(name), m_kind(Dir), m_fd(0), m_isLast(false) { }

   virtual ~DirEntry() { }

   EntryKind kind() const {
      return m_kind;
   }

   QSharedPointer<FileDef> file()  const {
      return m_fd;
   }

   bool isLast() const    {
      return m_isLast;
   }

   void setLast(bool b)   {
      m_isLast = b;
   }

   QSharedPointer<DirEntry> parent() const {
      return m_parent;
   }

   QString name() const  {
      return m_name;
   }

   QString path() const  {
      return parent() ? parent()->path() + "/" + name() : name();
   }

 protected:
   QSharedPointer<DirEntry> m_parent;
   QString m_name;

 private:
   EntryKind m_kind;
   QSharedPointer<FileDef> m_fd;
   bool m_isLast;
};

/** Class representing a directory tree of DirEntry objects. */
class DirEntryTree : public DirEntry
{
 public:
   DirEntryTree(QSharedPointer<DirEntryTree> parent, const QString &name)
      : DirEntry(parent, name)
   {
   }

   virtual ~DirEntryTree() {}

   void addChild(QSharedPointer<DirEntry> d) {
      m_children.append(d);
      d->setLast(true);
   }

   QList<QSharedPointer<DirEntry>> &children() {
      return m_children;
   }

   void rename(const QString &name) {
      m_name = name;
   }

   void reParent(QSharedPointer<DirEntryTree> parent)  {
      m_parent = parent;
   }

 private:
   QList<QSharedPointer<DirEntry>> m_children;
};

#endif

