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
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#ifndef FILENAME_H
#define FILENAME_H

#include <QHash>
#include <QList>

#include <filedef.h>

/** Class representing all files with a certain base name */
class FileName : public FileList
{
 public:
   FileName(const char *fn, const char *name);
   ~FileName();

   const char *fileName() const {
      return name;
   }

   const char *fullName() const {
      return fName;
   }

   void generateDiskNames();

 private:
   int compareListValues(const FileDef *item1, const FileDef *item2) const;

   QByteArray name;
   QByteArray fName;
};

/** Iterator for FileDef objects in a FileName list. */
class FileNameIterator : public QListIterator<QSharedPointer<FileDef> >
{
 public:
   FileNameIterator(const FileName &list);
};

/** Unsorted dictionary of FileName objects. */
class FileNameDict : public StringMap<QSharedPointer<FileName>>
{
 public:
   FileNameDict();
   ~FileNameDict() {}
};

#endif
