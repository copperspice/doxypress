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

#ifndef FILEDEF_H
#define FILEDEF_H

#include <QList>
#include <QHash>

#include <index.h>
#include <definition.h>
#include <memberlist.h>
#include <stringmap.h>
#include <sortedlist.h>

class ClassSDict;
class ClassDef;
class DirDef;
class FileDef;
class FileList;
class FTextStream;
class MemberDef;
class MemberList;
class MemberGroupSDict;
class NamespaceDef;
class NamespaceSDict;
class OutputList;
class PackageDef; 

/** Class representing the data associated with a \#include statement. */
struct IncludeInfo {
   IncludeInfo() {
      fileDef  = 0;
      local    = false;
      indirect = false;
   }

   ~IncludeInfo() {}

   FileDef *fileDef;

   QByteArray includeName;
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
   //enum FileType { Source, Header, Unknown };

   FileDef(const char *p, const char *n, const char *ref = 0, const char *dn = 0);
   ~FileDef();

   // ----------------------------------------------------------------------

   DefType definitionType() const {
      return TypeFile;
   }

   /*! Returns the unique file name (this may include part of the path). */
   QByteArray name() const;
   QByteArray displayName(bool = true) const {
      return name();
   }
   QByteArray fileName() const {
      return m_fileName;
   }

   QByteArray getOutputFileBase() const {
      return convertNameToFile(m_diskName);
   }

   QByteArray anchor() const {
      return QByteArray();
   }

   QByteArray getFileBase() const {
      return m_diskName;
   }

   QByteArray getSourceFileBase() const;

   /*! Returns the name of the verbatim copy of this file (if any). */
   QByteArray includeName() const;

   /*! Returns the absolute path including the file name. */
   QByteArray getFilePath() const {
      return m_filePath;
   }

   /*! Returns the name as it is used in the documentation */
   const QByteArray &docName() const {
      return m_docname;
   }

   /*! Returns true if this file is a source file. */
   bool isSource() const {
      return m_isSource;
   }

   bool isDocumentationFile() const;

   Definition *getSourceDefinition(int lineNr) const;
   MemberDef *getSourceMember(int lineNr) const;

   /*! Returns the absolute path of this file. */
   QByteArray getPath() const {
      return m_path;
   }

   /*! Returns version of this file. */
   QByteArray getVersion() const {
      return m_fileVersion;
   }

   bool isLinkableInProject() const;

   bool isLinkable() const {
      return isLinkableInProject() || isReference();
   }

   bool isIncluded(const QByteArray &name) const;

   PackageDef *packageDef() const {
      return m_package;
   }

   DirDef *getDirDef() const      {
      return m_dir;
   }

   NamespaceSDict *getUsedNamespaces() const;
   StringMap<QSharedPointer<Definition>> *getUsedClasses() const      {
      return m_usingDeclList;
   }

   QList<IncludeInfo> *includeFileList() const    {
      return m_includeList;
   }

   QList<IncludeInfo> *includedByFileList() const {
      return m_includedByList;
   }

   void getAllIncludeFilesRecursively(QStringList &incFiles) const;

   MemberList *getMemberList(MemberListType lt) const;
   const QList<MemberList> &getMemberLists() const {
      return m_memberLists;
   }

   /* user defined member groups */
   MemberGroupSDict *getMemberGroupSDict() const {
      return m_memberGroupSDict;
   }

   NamespaceSDict *getNamespaceSDict() const     {
      return m_namespaceSDict;
   }

   ClassSDict *getClassSDict() const             {
      return m_classSDict;
   }

   QByteArray title() const;
   bool hasDetailedDescription() const;
   QByteArray fileVersion() const;

   bool subGrouping() const {
      return m_subGrouping;
   }

   //---------------------------------

   void addSourceRef(int line, Definition *d, MemberDef *md);

   void writeDocumentation(OutputList &ol);
   void writeMemberPages(OutputList &ol);
   void writeQuickMemberLinks(OutputList &ol, MemberDef *currentMd) const;
   void writeSummaryLinks(OutputList &ol);
   void writeTagFile(FTextStream &t);

   void startParsing();
   void writeSource(OutputList &ol, bool sameTu, QStringList &filesInSameTu);
   void parseSource(bool sameTu, QStringList &filesInSameTu);
   void finishParsing();

   friend void generatedFileNames();
   void insertMember(MemberDef *md);
   void insertClass(ClassDef *cd);
   void insertNamespace(NamespaceDef *nd);
   void computeAnchors();

   void setPackageDef(PackageDef *pd) {
      m_package = pd;
   }
   void setDirDef(DirDef *dd) {
      m_dir = dd;
   }

   void addUsingDirective(NamespaceDef *nd);
   void addUsingDeclaration(Definition *def);
   void combineUsingRelations();

   bool generateSourceFile() const;
   void sortMemberLists();

   void addIncludeDependency(FileDef *fd, const char *incName, bool local, bool imported, bool indirect);
   void addIncludedByDependency(FileDef *fd, const char *incName, bool local, bool imported);

   void addMembersToMemberGroup();
   void distributeMemberGroupDocumentation();
   void findSectionsInDocumentation();
   void addIncludedUsingDirectives();

   void addListReferences();
   //bool includes(FileDef *incFile,QHash<QString, FileDef> *includedFiles) const;
   //bool includesByName(const QByteArray &name) const;
   bool visited;

 protected:
   /**
    * Retrieves the file version from version control system.
    */
   void acquireFileVersion();

 private:
   MemberList *createMemberList(MemberListType lt);
   void addMemberToList(MemberListType lt, MemberDef *md);
   void writeMemberDeclarations(OutputList &ol, MemberListType lt, const QByteArray &title);
   void writeMemberDocumentation(OutputList &ol, MemberListType lt, const QByteArray &title);
   void writeIncludeFiles(OutputList &ol);
   void writeIncludeGraph(OutputList &ol);
   void writeIncludedByGraph(OutputList &ol);
   void writeMemberGroups(OutputList &ol);
   void writeAuthorSection(OutputList &ol);
   void writeSourceLink(OutputList &ol);
   void writeNamespaceDeclarations(OutputList &ol, const QByteArray &title, bool isConstantGroup);
   void writeClassDeclarations(OutputList &ol, const QByteArray &title);
   void writeInlineClasses(OutputList &ol);
   void startMemberDeclarations(OutputList &ol);
   void endMemberDeclarations(OutputList &ol);
   void startMemberDocumentation(OutputList &ol);
   void endMemberDocumentation(OutputList &ol);
   void writeDetailedDescription(OutputList &ol, const QByteArray &title);
   void writeBriefDescription(OutputList &ol);

   QHash<QString, IncludeInfo>   *m_includeDict;
   QList<IncludeInfo>            *m_includeList;
   QHash<QString, IncludeInfo>   *m_includedByDict;
   QList<IncludeInfo>            *m_includedByList;
   NamespaceSDict                *m_usingDirList;

   StringMap<QSharedPointer<Definition>>  *m_usingDeclList;

   QByteArray               m_path;
   QByteArray               m_filePath;
   QByteArray               m_diskName;
   QByteArray               m_fileName;
   QByteArray               m_docname;

   QHash<long, Definition> *m_srcDefDict;
   QHash<long, MemberDef>  *m_srcMemberDict;

   bool                     m_isSource;
   QByteArray               m_fileVersion;
   PackageDef              *m_package;
   DirDef                  *m_dir;
   QList<MemberList>        m_memberLists;
   MemberGroupSDict        *m_memberGroupSDict;
   NamespaceSDict          *m_namespaceSDict;
   ClassSDict              *m_classSDict;
   bool                     m_subGrouping;
};

/** Class representing a list of FileDef objects. */
class FileList : public SortedList<FileDef *>
{
 public:
   FileList() : m_pathName("tmp")
   {}

   FileList(const char *path) : m_pathName(path) 
   {}

   ~FileList() 
   {}

   QByteArray path() const {
      return m_pathName;
   }

 private:  
   QByteArray m_pathName;
};

class OutputNameDict : public QHash<QString, FileList>
{
 public:
   OutputNameDict() : QHash<QString, FileList>() {}
   ~OutputNameDict() {}
};

class Directory;

/** Class representing an entry (file or sub directory) in a directory */
class DirEntry
{
 public:
   enum EntryKind { Dir, File };
   DirEntry(DirEntry *parent, FileDef *fd)
      : m_parent(parent), m_name(fd->name()), m_kind(File), m_fd(fd),m_isLast(false) 
   { }

   DirEntry(DirEntry *parent, QByteArray name)
      : m_parent(parent), m_name(name), m_kind(Dir), m_fd(0), m_isLast(false) { }

   virtual ~DirEntry() { }

   EntryKind kind() const {
      return m_kind;
   }

   FileDef *file()  const {
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
   QByteArray name() const  {
      return m_name;
   }
   QByteArray path() const  {
      return parent() ? parent()->path() + "/" + name() : name();
   }

 protected:
   DirEntry *m_parent;
   QByteArray m_name;

 private:
   EntryKind m_kind;
   FileDef *m_fd;
   bool m_isLast;
};

/** Class representing a directory tree of DirEntry objects. */
class Directory : public DirEntry
{
 public:
   Directory(Directory *parent, const QByteArray &name)
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

   void rename(const QByteArray &name) {
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

