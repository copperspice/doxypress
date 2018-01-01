/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim 
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

#ifndef SORTEDLIST_H
#define SORTEDLIST_H

#include <QList>
#include <QSharedPointer>

#include <sortedlist_fwd.h>

class BaseClassDef;
class ClassDef;
class DirDef;
class DotNode;
class FileDef;
class FileNameList;
class GroupDef;
class MemberDef;
class NavIndexEntry;
 
// default value for flag is false

template <class T, bool flag>
class SortedList : public QList<T>
{
   public:
      template <class U>
      bool contains(const U &data);

      void inSort(const T &data);
      void sort();
};

template <class T, bool flag>
template <class U>
bool SortedList<T, flag>::contains(const U &data)
{
   for (auto &item : *this) {
      if (item == data) {
         return true;
      }      
   }

   return false;
}
   
template <class T, bool flag>
void SortedList<T, flag>::inSort(const T &data)
{
   typename QList<T>::iterator location;   
   location = std::lower_bound(this->begin(), this->end(), data, 
                  [](const T &temp1, const T &temp2) { return compareListValues(temp1, temp2, flag) < 0; } );

   this->insert(location, data);  
}

template <class T, bool flag>
void SortedList<T, flag>::sort()
{
   std::sort(this->begin(), this->end(), 
                  [](const T &temp1, const T &temp2) { return  compareListValues(temp1, temp2, flag) < 0; } );
}

int compareListValues(const BaseClassDef *item1,  const BaseClassDef *item2, bool flag);
int compareListValues(const DotNode *item1,       const DotNode *item2, bool flag);
int compareListValues(const NavIndexEntry *item1, const NavIndexEntry *item2, bool flag);

int compareListValues(const QSharedPointer<ClassDef>      &item1, const QSharedPointer<ClassDef>      &item2, bool flag);
int compareListValues(const QSharedPointer<DirDef>        &item1, const QSharedPointer<DirDef>        &item2, bool flag); 
int compareListValues(const QSharedPointer<FileDef>       &item1, const QSharedPointer<FileDef>       &item2, bool flag);
int compareListValues(const QSharedPointer<FileNameList>  &item1, const QSharedPointer<FileNameList>  &item2, bool flag);
int compareListValues(const QSharedPointer<GroupDef>      &item1, const QSharedPointer<GroupDef>      &item2, bool flag); 
int compareListValues(const QSharedPointer<MemberDef>     &item1, const QSharedPointer<MemberDef>     &item2, bool flag);

#endif