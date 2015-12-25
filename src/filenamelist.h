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

#ifndef FILENAMELIST_H
#define FILENAMELIST_H

#include <QString>
#include <QSharedPointer>

#include <sortedlist.h>

class FileDef;

/** Class representing all files with a certain base name */
class FileNameList : public SortedList<QSharedPointer<FileDef>, true>
{
 public:
   FileNameList(const QString &fn, const QString &name);
   ~FileNameList();

   QString fileName() const {
      return name;
   }

   QString fullName() const {
      return fName;
   }

   void generateDiskNames();

 private:  
   QString name;
   QString fName;
};

// stores a list of FileDef objects
using  FileList = SortedList<QSharedPointer<FileDef>>;

#endif
