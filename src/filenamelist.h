/************************************************************************
*
* Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
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

#ifndef FILENAMELIST_H
#define FILENAMELIST_H

#include <sortedlist.h>

#include <QString>
#include <QSharedPointer>

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
