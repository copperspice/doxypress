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
 
#include <sortedlist.h>

#include <classdef.h>
#include <config.h>
#include <dirdef.h>
#include <dot.h>
#include <filedef.h>
#include <filename.h>
#include <groupdef.h>


// BaseClassList --> SortedList<BaseClassDef *>
// BaseClassListIterator --> QListIterator<BaseClassDef *>

int compareValues(const BaseClassDef *item1, const BaseClassDef *item2) 
{
   const ClassDef *c1 = item1->classDef;
   const ClassDef *c2 = item2->classDef;

   if (c1 == 0 || c2 == 0) {
      return false;

   } else {
      return qstricmp(c1->name(), c2->name());
   }
}


// ClassList --> SortedList<ClassDef *>
// ClassListIterator --> QListIterator<ClassDef *>

int compareValues(const ClassDef *item1, const ClassDef *item2)
{
   static bool b = Config_getBool("SORT_BY_SCOPE_NAME");

   if (b) {
      return qstricmp(item1->name(), item2->name());

   } else {
      return qstricmp(item1->className(), item2->className());

   }
}


// DirList ->  SortedList<DirDef *>

int compareValues(const DirDef *item1, const DirDef *item2)
{
    return qstricmp(item1->shortName(), item2->shortName());
}


// DotNodeList --> SortedList<DotNode *>

int compareValues(const DotNode *item1, const DotNode *item2) 
{
   return qstricmp(item1->label(), item2->label());
}


// FileList 

int compareValues(const FileDef *md1, const FileDef *md2) 
{
      return qstricmp(md1->name(), md2->name());
}


// FileNameList --> SortedList<FileName *>
// FileNameListIterator -->  QListIterator<FileName *>

int compareValues(const FileName *item1, const FileName *item2)
{
   int retval;

   if (Config_getBool("FULL_PATH_NAMES")) {
      retval = qstricmp(item1->fullName(), item2->fullName());

   } else {
      retval = qstricmp(item1->fileName(), item2->fileName());

   }

   return retval;
}


//   GroupList --> SortedList<GroupDef *>
//   GroupListIterator --> QListIterator<GroupDef *>

int compareValues(const GroupDef *item1, const GroupDef *item2) 
{
   return qstrcmp(item1->groupTitle(), item2->groupTitle());
}


// MemberList

int compareValues(const MemberDef *c1, const MemberDef *c2)
{
   static bool sortConstructorsFirst = Config_getBool("SORT_MEMBERS_CTORS_1ST");

   if (sortConstructorsFirst) {
      int ord1 = c1->isConstructor() ? 2 : (c1->isDestructor() ? 1 : 0);
      int ord2 = c2->isConstructor() ? 2 : (c2->isDestructor() ? 1 : 0);

      if (ord1 > ord2) {
         return -1;

      } else if (ord2 > ord1) {
         return 1;

      }
   }

   int cmp = qstricmp(c1->name(), c2->name());

   return cmp != 0 ? cmp : c1->getDefLine() - c2->getDefLine();
}


// OutputNameList -->  SortedList<FileList *>

int compareValues(const FileList *item1, const FileList *item2) 
{
   return qstricmp(item1->path(), item2->path());
}

