/*************************************************************************
 *
 * Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim 
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

#include <filenamelist.h>

#include <config.h>
#include <util.h>

FileNameList::FileNameList(const QString &fn, const QString &n) 
      : fName(fn), name(n)
{      
}

FileNameList::~FileNameList()
{
}

void FileNameList::generateDiskNames()
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
            QChar letter = '\0';

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
            QString prefix = item->m_path.right(item->m_path.length() - j - 1);
            item->setName(prefix + name);
           
            item->m_diskName = prefix + name;
         }
      }
   }
}

