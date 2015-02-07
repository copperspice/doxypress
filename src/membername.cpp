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

#include <membername.h>
#include <classdef.h>
#include <util.h>
#include <filedef.h>

MemberName::MemberName(const char *n) : QList<QSharedPointer<MemberDef>>()
{
   name = n;
}

MemberName::~MemberName()
{
}

int MemberName::compareValues(QSharedPointer<const MemberDef> m1, QSharedPointer<const MemberDef> m2) const
{
   QSharedPointer<ClassDef> c1 = m1->getClassDef();
   QSharedPointer<ClassDef> c2 = m2->getClassDef();

   QSharedPointer<FileDef> f1 = m1->getFileDef();
   QSharedPointer<FileDef> f2 = m2->getFileDef();

   if (c1 && c2) {
      return qstrcmp(c1->name(), c2->name());

   } else if (f1 && f2) {
      return qstrcmp(f1->name(), f2->name());

   } else {
      return 0;
   }
}

MemberNameInfo::MemberNameInfo(const char *n) : QList<MemberInfo>()
{
   name = n;
}

int MemberNameInfo::compareValues(QSharedPointer<const MemberInfo> m1, QSharedPointer<const MemberInfo> m2) const
{
   QSharedPointer<ClassDef> c1 = m1->memberDef->getClassDef();
   QSharedPointer<ClassDef> c2 = m2->memberDef->getClassDef();

   QSharedPointer<FileDef> f1 = m1->memberDef->getFileDef();
   QSharedPointer<FileDef> f2 = m2->memberDef->getFileDef();

   if (c1 && c2) {
      return qstrcmp(c1->name(), c2->name());

   } else if (f1 && f2) {
      return qstrcmp(f1->name(), f2->name());

   } else {
      return 0;

   }
}

MemberNameIterator::MemberNameIterator(const MemberName &mnlist) 
   : QListIterator<QSharedPointer<MemberDef>>(mnlist)
{
}

int MemberNameSDict::compareMapValues(const QSharedPointer<MemberName> &n1, const QSharedPointer<MemberName> &n2) const
{
   return qstricmp(n1->memberName() + getPrefixIndex(n1->memberName()), n2->memberName() + getPrefixIndex(n2->memberName()) );
}

