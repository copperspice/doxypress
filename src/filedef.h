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

#ifndef FILEDEF_H
#define FILEDEF_H

#include <QList>
#include <QHash>
#include <QTextStream>

#include <classlist.h>
#include <definition.h>
#include <filelist.h>
#include <index.h>
#include <memberlist.h>
#include <membergroup.h>
#include <namespacedef.h>
#include <stringmap.h>
#include <sortedlist.h>

class ClassDef;
class DirDef;
class FileDef;
class MemberDef;
class MemberList;
class OutputList;
class PackageDef; 

/** Class representing the data associated with a \#include statement. */
struct IncludeInfo {
   IncludeInfo() {      
      local    = false;
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
   friend class FileName;

 public:
   // enum FileType { Source, Header, Unknown };

   FileDef(const QString &p, const QString &n, const QString &ref = QString(), const QString &dn = QString());
   ~FileDef();

   DefType definitionType() const {
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
      return "";
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

   bool isLinkableInProject() const;

   bool isLinkable() const {
      return isLinkableInProject() || isReference();
   }

   bool isIncluded(const QString &name) const;

   PackageDef *packageDef() const {
      return m_package;
   }

   QSharedPointer<DirDef> getDirDef() const      {
      return m_dir;
   }

   NamespaceSDict *getUsedNamespaces();

   StringMap<QSharedPointer<Definition>> *getUsedClasses()  {
      return &m_usingDeclList;
   }

   const QList<IncludeInfo> *includeFileList() const    {
      if (m_includeList.isEmpty()) {
         return 0;
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
   MemberGroupSDict *getMemberGroupSDict() {
      return &m_memberGroupSDict;
   }

   NamespaceSDict *getNamespaceSDict() {
      return &m_namespaceSDict;
   }

   ClassSDict *getClassSDict()  {
      return &m_classSDict;
   }

   QString title() const;
   bool hasDetailedDescription() const;
   QString fileVersion() const;

   bool subGrouping() const {
      return m_subGrouping;
   }

   void addSourceRef(int line, QSharedPointer<Definition> d, QSharedPointer<MemberDef> md);

   void writeDocumentation(OutputList &ol);
   void writeMemberPages(OutputList &ol);
   void writeQuickMemberLinks(OutputList &ol, MemberDef *currentMd) const;
   void writeSummaryLinks(OutputList &ol);
   void writeTagFile(QTextStream &t);

   void startParsing();
   void writeSource(OutputList &ol, bool sameTu, QStringList &filesInSameTu);
   void parseSource(bool sameTu, QStringList &filesInSameTu);
   void finishParsing();

   friend void generatedFileNames();
   void insertMember(QSharedPointer<MemberDef> md);
   void insertClass(QSharedPointer<ClassDef> cd);
   void insertNamespace(QSharedPointer<NamespaceDef> nd);
   void computeAnchors();

   void setPackageDef(PackageDef *pd) {
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
   PackageDef        *m_package;

   QSharedPointer<DirDef> m_dir;
   QList<QSharedPointer<MemberList>> m_memberLists;

   MemberGroupSDict   m_memberGroupSDict;
   NamespaceSDict     m_namespaceSDict;
   ClassSDict         m_classSDict;

   bool               m_subGrouping;
};

class Directory;

/** Class representing an entry (file or sub directory) in a directory */
class DirEntry
{
 public:
   enum EntryKind { Dir, File };

   DirEntry(DirEntry *parent, QSharedPointer<FileDef> fd)
      : m_parent(parent), m_name(fd->name()), m_kind(File), m_fd(fd),m_isLast(false) 
   { }

   DirEntry(DirEntry *parent, QString name)
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

   DirEntry *parent() const {
      return m_parent;
   }

   QString name() const  {
      return m_name;
   }

   QString path() const  {
      return parent() ? parent()->path() + "/" + name() : name();
   }

 protected:
   DirEntry *m_parent;
   QString m_name;

 private:
   EntryKind m_kind;
   QSharedPointer<FileDef> m_fd;
   bool m_isLast;
};

/** Class representing a directory tree of DirEntry objects. */
class Directory : public DirEntry
{
 public:
   Directory(Directory *parent, const QString &name)
      : DirEntry(parent, name)
   {      
   }

   virtual ~Directory() {}

   void addChild(DirEntry *d) {
      m_children.append(d);
      d->setLast(true);
   }

   QList<DirEntry *> &children() {
      return m_children;
   }

   void rename(const QString &name) {
      m_name = name;
   }

   void reParent(Directory *parent)  {
      m_parent = parent;
   }

 private:
   QList<DirEntry *> m_children;
};

void generateFileTree();

#endif

