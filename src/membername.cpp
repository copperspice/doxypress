/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
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

#include <membername.h>

#include <classdef.h>
#include <filedef.h>
#include <util.h>

MemberName::MemberName(const QString &n) : QList<QSharedPointer<MemberDef>>()
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
      return c1->name().compare(c2->name());

   } else if (f1 && f2) {
      return f1->name().compare(f2->name());

   } else {
      return 0;
   }
}

MemberNameInfo::MemberNameInfo(const QString &n) : QList<MemberInfo>()
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
      return c1->name().compare(c2->name());

   } else if (f1 && f2) {
      return f1->name().compare(f2->name());

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
   QString name1 = n1->memberName();
   QString name2 = n2->memberName();

   QString tmp1 = name1.mid( getPrefixIndex(name1) );
   QString tmp2 = name2.mid( getPrefixIndex(name2) );
     
   int i = tmp1.compare(tmp2, Qt::CaseInsensitive);

   return i;
}

