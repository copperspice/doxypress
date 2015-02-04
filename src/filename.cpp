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

#include <config.h>
#include <filename.h>
#include <util.h>


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
         QSharedPointer<FileDef> fd;

         QList<QSharedPointer<FileDef>>::iterator it;

         for (it = this->begin(); it != this->end(); ++it) {   
         
            if (! (*it)->isReference()) {
               fd = *it;               
               break;
            }
         }
       
         if (fd) {     
            // init for safety
            char letter = '\0';

            if ( fd->m_path.length() > i) {
               letter = fd->m_path.at(i);
           
               if (letter == '/') {
                  // remember last position of dirname
                  j = i;   
               }
            }

            if (it != this->end()) {
               ++it;
            }

            while ( it != this->end() && (fd = *it) && ! found) {

               if (! fd->isReference()) {                 

                  if (i == fd->m_path.length()) {                     
                     found = true;

                  } else if (fd->m_path[i] != letter) {
                     found = true;

                  }
               }
              
               ++it;               
            }

            i++;
         }

      }     

      for (auto item : *this) {   
        
         if (! item->isReference()) {
            QByteArray prefix = item->m_path.right(item->m_path.length() - j - 1);
            item->setName(prefix + name);
           
            item->m_diskName = prefix + name;
         }
      }
   }
}

int FileName::compareValues(const FileDef *f1, const FileDef *f2) const
{
   return qstricmp(f1->fileName(), f2->fileName());
}

FileNameIterator::FileNameIterator(const FileName &fname) : QListIterator<QSharedPointer<FileDef>>(fname)
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

