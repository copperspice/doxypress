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

#include <filename.h>
#include <util.h>
#include <config.h>

FileName::FileName(const char *fn, const char *n) : FileList()
{   
   fName = fn;
   name  = n;
}

FileName::~FileName()
{
}

void FileName::generateDiskNames()
{  
   int count = 0; 

   for ( auto item : *this) {   
      if (! item->isReference()) {
         count++;
      }
   }
   
   if (count == 1) {
          
      for ( auto item : *this) {   
         if (item->isReference()) {
            // name if unique, so diskname is simply the name    
            item->m_diskName = name;
            break;
         }
      }
  
   } else if (count > 1) { 
      // multiple occurrences of the same file name
     
      int i = 0;
      int j = 0;

      bool found = false;     

      while (! found) { 
         // search for the common prefix of all paths
         FileDef *fd = 0;

         QList<FileDef *>::iterator it;

         for (it = this->begin(); it != this->end(); ++it) {   
            if ((*it)->isReference()) {
               fd = *it;               
               break;
            }
         }
       
         if (fd) {
            char c = fd->m_path.at(i);

            if (c == '/') {
               j = i;   // remember last position of dirname
            }

            ++it;
            while ((fd = *it) && ! found) {

               if (! fd->isReference()) {
                  //printf("i=%d j=%d fd->path=`%s' fd->name=`%s'\n",i,j,fd->path.left(i).data(),fd->name().data());

                  if (i == (int)fd->m_path.length()) {                     
                     found = true;

                  } else if (fd->m_path[i] != c) {
                     found = true;

                  }
               }
               ++it;
            }
            i++;
         }
      }     

      for (auto item : *this) {   
         //printf("fd->setName(%s)\n",(fd->path.right(fd->path.length()-j-1)+name).data());

         if (! item->isReference()) {
            QByteArray prefix = item->m_path.right(item->m_path.length() - j - 1);
            item->setName(prefix + name);

            //printf("!!!!!!!! non unique disk name=%s for fd=%s\n",(prefix+name).data(),fd->diskname.data());
            item->m_diskName = prefix + name;
         }
      }
   }
}

int FileName::compareValues(const FileDef *f1, const FileDef *f2) const
{
   return qstricmp(f1->fileName(), f2->fileName());
}

// ** 
FileNameIterator::FileNameIterator(const FileName &fname) : QListIterator<FileDef *>(fname)
{
}

// **
FileNameList::FileNameList() : QList<FileName>()
{
}

FileNameList::~FileNameList()
{
}

void FileNameList::generateDiskNames()
{ 
   for (auto item : *this) {
      item.generateDiskNames();
   }
}

int FileNameList::compareValues(const FileName *f1, const FileName *f2) const
{
   int retval;

   if (Config_getBool("FULL_PATH_NAMES")) {
      retval = qstricmp(f1->fullName(), f2->fullName());

   } else {
      retval = qstricmp(f1->fileName(), f2->fileName());

   }

   return retval;
}

// **
FileNameListIterator::FileNameListIterator(const FileNameList &fnlist) : QListIterator<FileName>(fnlist)
{
}


// ** 
static Qt::CaseSensitivity getCaseSenseNames()
{
   if (Config_getBool("CASE_SENSE_NAMES")) {
      return Qt::CaseSensitive;
   } else {
      return Qt::CaseInsensitive;
   }   
}

// **
FileNameDict::FileNameDict() : StringMap<QSharedPointer<FileName>>(getCaseSenseNames())
{
}
