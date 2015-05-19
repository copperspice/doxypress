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

#ifndef DIRDEF_H
#define DIRDEF_H

#include <QList>
#include <QStringList>
#include <QTextStream>

#include <definition.h>
#include <stringmap.h>
#include <sortedlist.h>

class ClassSDict;
class FileDef;
class OutputList;
class UsedDir;

/** A model of a directory symbol. */
class DirDef : public Definition
{
 public:
   DirDef(const char *path);
   virtual ~DirDef();

   // accessors
   DefType definitionType() const {
      return TypeDir;
   }

   QByteArray getOutputFileBase() const;
   QByteArray anchor() const {
      return QByteArray();
   }

   bool isLinkableInProject() const;
   bool isLinkable() const;

   QString displayName(bool = true) const override {
      return m_dispName;
   }

   const QString &shortName() const {
      return m_shortName;
   }

   void addSubDir(QSharedPointer<DirDef> subdir);

   FileList *getFiles() const        {
      return m_fileList;
   }

   void addFile(QSharedPointer<FileDef> fd);

   const SortedList<QSharedPointer<DirDef>> subDirs() const {
      return m_subdirs;
   }

   bool isCluster() const {
      return m_subdirs.count() > 0;
   }

   int level() const {
      return m_level;
   }

   QSharedPointer<DirDef> parent() const {
      return m_parent;
   }

   int dirCount() const {
      return m_dirCount;
   }

   const QHash<QString, UsedDir *> &usedDirs() const {
      return m_usedDirs;
   }

   bool isParentOf(QSharedPointer<DirDef> dir) const;
   bool depGraphIsTrivial() const;
   QByteArray shortTitle() const;
   bool hasDetailedDescription() const;

   // generate output
   void writeDocumentation(OutputList &ol);
   void writeDepGraph(QTextStream &t);
   void writeTagFile(QTextStream &t);

   static QSharedPointer<DirDef> mergeDirectoryInTree(const QByteArray &path);
   bool visited;

   void setDiskName(const QByteArray &name) {
      m_diskName = name;
   }

 private:
   friend void computeDirDependencies();

   void writeDetailedDescription(OutputList &ol, const QByteArray &title);
   void writeBriefDescription(OutputList &ol);
   void writeDirectoryGraph(OutputList &ol);
   void writeSubDirList(OutputList &ol);
   void writeFileList(OutputList &ol);
   void startMemberDeclarations(OutputList &ol);
   void endMemberDeclarations(OutputList &ol);

   void setLevel();

   static QSharedPointer<DirDef> createNewDir(const char *path);
   static bool matchPath(const QString &path, const QStringList &list);
   void addUsesDependency(QSharedPointer<DirDef> dir, QSharedPointer<FileDef> srcFd, QSharedPointer<FileDef> dstFd, bool inherited);
   void computeDependencies();

   SortedList<QSharedPointer<DirDef>> m_subdirs;

   QString m_dispName;
   QString m_shortName;
   QString m_diskName;

   // list of files in the group
   FileList *m_fileList;                 

   int m_dirCount;
   int m_level;

   QSharedPointer<DirDef> m_parent;

   QHash<QString, UsedDir *> m_usedDirs;
};

/** Class representing a pair of FileDef objects */
class FilePair
{
 public:
   FilePair(QSharedPointer<FileDef> src, QSharedPointer<FileDef> dst) 
      : m_src(src), m_dst(dst) {}

   QSharedPointer<const FileDef> source() const {
      return m_src;
   }

   QSharedPointer<const FileDef> destination() const {
      return m_dst;
   }

 private:
   QSharedPointer<FileDef> m_src;
   QSharedPointer<FileDef> m_dst;
};

/** A sorted dictionary of FilePair objects. */
class FilePairDict : public StringMap<QSharedPointer<FilePair>>
{
 public:
   // CopperSpice - can add isCase 
   FilePairDict() : StringMap<QSharedPointer<FilePair>>() {}

 private:
   int compareMapValues(const QSharedPointer<FilePair> &item1, const QSharedPointer<FilePair> &item2) const override;
};

/** Usage information of a directory. */
class UsedDir
{
 public:
   UsedDir(QSharedPointer<DirDef> dir, bool inherited);
   virtual ~UsedDir();

   void addFileDep(QSharedPointer<FileDef> srcFd, QSharedPointer<FileDef> dstFd);
   QSharedPointer<FilePair> findFilePair(const char *name);

   const FilePairDict &filePairs() const {
      return m_filePairs;
   }

   QSharedPointer<const DirDef> dir() const {
      return m_dir;
   }

   bool inherited() const {
      return m_inherited;
   }

 private:
   QSharedPointer<DirDef> m_dir;
   FilePairDict m_filePairs;
   bool m_inherited;
};

/** A usage relation between two directories. */
class DirRelation
{
 public:
   DirRelation(const QByteArray &name, QSharedPointer<DirDef> src, UsedDir *dst)
      : m_name(name), m_src(src), m_dst(dst)
   {}

   QSharedPointer<DirDef> source() const      {
      return m_src;
   }

   UsedDir *destination() const {
      return m_dst;
   }

   void writeDocumentation(OutputList &ol);
   QByteArray getOutputFileBase() const {
      return m_name;
   }

 private:
   QByteArray m_name;
   QSharedPointer<DirDef> m_src;
   UsedDir *m_dst;
};


/** A sorted dictionary of DirDef objects. */
class DirSDict : public StringMap<QSharedPointer<DirDef>>
{
 public:
   // CopperSpice - can add isCase
   DirSDict() : StringMap<QSharedPointer<DirDef>>() {}

   int compareMapValues(const QSharedPointer<DirDef> &item1, const QSharedPointer<DirDef> &item2) const override {
      return item1->shortName().compare(item2->shortName(), Qt::CaseInsensitive);
   }
};

void buildDirectories();
void generateDirDocs(OutputList &ol);
void computeDirDependencies();
void writeDirDependencyGraph(const char *file);

#endif
