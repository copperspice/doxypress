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

#include <sortedlist.h>

#include <classdef.h>
#include <config.h>
#include <dirdef.h>
#include <dot.h>
#include <filedef.h>
#include <filenamelist.h>
#include <ftvhelp.h>
#include <groupdef.h>

// BaseClassList --> SortedList<BaseClassDef *>
int compareListValues(const BaseClassDef *item1, const BaseClassDef *item2, bool flag)
{
   (void) flag;

   QSharedPointer<const ClassDef> c1 = item1->classDef;
   QSharedPointer<const ClassDef> c2 = item2->classDef;

   if (c1 == nullptr || c2 == nullptr) {
      return false;

   } else {
      return c1->name().compare(c2->name(), Qt::CaseInsensitive);
   }
}

// ClassList --> SortedList<ClassDef *>
int compareListValues(const QSharedPointer<ClassDef> item1, const QSharedPointer<ClassDef> item2, bool flag)
{
   (void) flag;
   static bool b = Config::getBool("sort-by-scope-name");

   if (b) {
      return item1->name().compare(item2->name(), Qt::CaseInsensitive);

   } else {
      return item1->className().compare(item2->className(), Qt::CaseInsensitive);

   }
}


// DirList ->  SortedList<DirDef *>
int compareListValues(const QSharedPointer<DirDef> &item1, const QSharedPointer<DirDef> &item2, bool flag)
{
   (void) flag;
   return item1->shortName().compare(item2->shortName(), Qt::CaseInsensitive);
}


// DotNodeList --> SortedList<DotNode *>
int compareListValues(const DotNode *item1, const DotNode *item2, bool flag)
{
   (void) flag;
   return item1->label().compare(item2->label(), Qt::CaseInsensitive);
}


// FileNameList --> SortedList<FileDef *>   true
// FileList     --> SortedList<FileDef *>   false
int compareListValues(const QSharedPointer<FileDef> &item1, const QSharedPointer<FileDef> &item2, bool flag)
{
   if (flag) {
      return item1->fileName().compare(item2->fileName(), Qt::CaseInsensitive);
   } else {
      return item1->name().compare(item2->name(), Qt::CaseInsensitive);
   }
}


// Doxy_Globals::inputNameList --> SortedList<FileNameList *>
int compareListValues(const QSharedPointer<FileNameList> &item1, const QSharedPointer<FileNameList> &item2, bool flag)
{
   (void) flag;
   int retval;

   if (Config::getBool("full-path-names")) {
      retval = item1->fullName().compare(item2->fullName(), Qt::CaseInsensitive);

   } else {
      retval = item1->fileName().compare(item2->fileName(), Qt::CaseInsensitive);

   }

   return retval;
}


// GroupList --> SortedList<GroupDef *>
int compareListValues(const QSharedPointer<GroupDef> &item1, const QSharedPointer<GroupDef> &item2, bool flag)
{
   (void) flag;
   return item1->groupTitle().compare(item2->groupTitle());
}


// MemberList --> SortedList<MemberDef *>
int compareListValues(const QSharedPointer<MemberDef> &c1, const QSharedPointer<MemberDef> &c2, bool flag)
{
   (void) flag;
   static bool sortConstructorsFirst = Config::getBool("sort-constructors-first");

   if (sortConstructorsFirst) {
      int ord1 = c1->isConstructor() ? 2 : (c1->isDestructor() ? 1 : 0);
      int ord2 = c2->isConstructor() ? 2 : (c2->isDestructor() ? 1 : 0);

      if (ord1 > ord2) {
         return -1;

      } else if (ord2 > ord1) {
         return 1;

      }
   }

   int cmp = c1->name().compare(c2->name(), Qt::CaseInsensitive );

   if (cmp == 0) {
      cmp = c1->argsString().compare(c2->argsString(), Qt::CaseInsensitive );
   }

   return cmp != 0 ? cmp : c1->getDefLine() - c2->getDefLine();
}


// NavIndexEntryList --> SortedList<NavIndexEntry *>
int compareListValues(const NavIndexEntry *item1, const NavIndexEntry *item2, bool flag)
{
   (void) flag;

   // sort list based on url
   return item1->m_url.compare(item2->m_url);
}