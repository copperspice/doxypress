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

#ifndef FILELIST_H
#define FILELIST_H

#include <QByteArray>

#include <sortedlist.h>

class FileDef;

/** Class representing a list of FileDef objects. */
class FileList : public SortedList<QSharedPointer<FileDef>>
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


#endif