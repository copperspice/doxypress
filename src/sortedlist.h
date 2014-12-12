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

#ifndef SORTEDLIST_H
#define SORTEDLIST_H

#include <QList>

class BaseClassDef;
class ClassDef;
class DotNode;
class FileList;
class FileName;
class MemberDef;

 
template <class T>
class SortedList : public QList<T>
{
   public:
      void inSort(const T &data);
};

template <class T>
void SortedList<T>::inSort(const T &data)
{
   typename QList<T>::iterator location;   
   location = std::lower_bound(this->begin(), this->end(), data, [](const T &temp1, const T &temp2){ return compareValues(temp1, temp2) < 0; } );

   this->insert(location, data);  
}

int compareValues(const BaseClassDef *item1, const BaseClassDef *item2);
int compareValues(const ClassDef *item1, const ClassDef *item2);
int compareValues(const DotNode *item1, const DotNode *item2); 
int compareValues(const FileList *item1, const FileList *item2); 
int compareValues(const FileName *item1, const FileName *item2);
int compareValues(const MemberDef *c1, const MemberDef *c2);


#endif