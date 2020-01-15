/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
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

#ifndef DIRDEF_H
#define DIRDEF_H

#include <QList>
#include <QStringList>
#include <QTextStream>

#include <definition.h>
#include <filenamelist.h>
#include <sortedlist.h>
#include <stringmap.h>

class ClassSDict;
class OutputList;
class UsedDir;

// model of a directory symbol
class DirDef : public Definition
{
 public:
   DirDef(const QString &path);
   virtual ~DirDef();

   // accessors
   DefType definitionType() const override {
      return TypeDir;
   }

   QString getOutputFileBase() const override;

   QString anchor() const override {
      return QString("");
   }

   bool isLinkableInProject() const override;
   bool isLinkable() const override;

   QString displayName(bool = true) const override {
      return m_dispName;
   }

   const QString &shortName() const {
      return m_shortName;
   }

   void addSubDir(QSharedPointer<DirDef> subdir);

   const FileList &getFiles() const        {
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

   const QHash<QString, QSharedPointer<UsedDir>> &usedDirs() const {
      return m_usedDirs;
   }

   bool isParentOf(QSharedPointer<DirDef> dir) const;
   bool depGraphIsTrivial() const;
   QString shortTitle() const;
   bool hasDetailedDescription() const;

   // generate output
   void writeDocumentation(OutputList &ol);
   void writeDepGraph(QTextStream &t);
   void writeTagFile(QTextStream &t);

   static QSharedPointer<DirDef> mergeDirectoryInTree(const QString &path);
   bool visited;

   void setDiskName(const QString &name) {
      m_diskName = name;
   }

   void sort();

 private:
   friend void computeDirDependencies();

   void writeDetailedDescription(OutputList &ol, const QString &title);
   void writeBriefDescription(OutputList &ol);
   void writeDirectoryGraph(OutputList &ol);
   void writeSubDirList(OutputList &ol);
   void writeFileList(OutputList &ol);
   void startMemberDeclarations(OutputList &ol);
   void endMemberDeclarations(OutputList &ol);

   void setLevel();

   static QSharedPointer<DirDef> createNewDir(const QString &path);
   static bool matchPath(const QString &path, const QStringList &list);
   void addUsesDependency(QSharedPointer<DirDef> dir, QSharedPointer<FileDef> srcFd, QSharedPointer<FileDef> dstFd, bool inherited);
   void computeDependencies();

   SortedList<QSharedPointer<DirDef>> m_subdirs;

   QString m_dispName;
   QString m_shortName;
   QString m_diskName;

   // list of files in the group
   FileList m_fileList;

   int m_dirCount;
   int m_level;

   QSharedPointer<DirDef> m_parent;
   QHash<QString, QSharedPointer<UsedDir>> m_usedDirs;
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

/** Usage information of a directory. */
class UsedDir
{
 public:
   UsedDir(QSharedPointer<DirDef> dir, bool inherited);
   virtual ~UsedDir();

   void addFileDep(QSharedPointer<FileDef> srcFd, QSharedPointer<FileDef> dstFd);
   QSharedPointer<FilePair> findFilePair(const QString &name);

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
   DirRelation(const QString &name, QSharedPointer<DirDef> src, QSharedPointer<UsedDir> dst)
      : m_name(name), m_src(src), m_dst(dst)
   {}

   QSharedPointer<DirDef> source() const      {
      return m_src;
   }

   QSharedPointer<UsedDir> destination() const {
      return m_dst;
   }

   void writeDocumentation(OutputList &ol);

   QString getOutputFileBase() const {
      return m_name;
   }

 private:
   QString                 m_name;
   QSharedPointer<DirDef>  m_src;
   QSharedPointer<UsedDir> m_dst;
};

void buildDirectories();
void generateDirDocs(OutputList &ol);
void computeDirDependencies();

#endif
