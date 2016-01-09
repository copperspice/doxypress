/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim 
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

#ifndef MEMBERNAME_H
#define MEMBERNAME_H

#include <QList>

#include <memberdef.h>

/** Class representing all MemberDef objects with the same name */
class MemberName : public QList<QSharedPointer<MemberDef>>
{
 public:
   MemberName(const QString &name);
   ~MemberName();

   QString memberName() const {
      return name;
   }

 private:
   int compareValues(QSharedPointer<const MemberDef> item1, QSharedPointer<const MemberDef> item2) const;
   QString name;
};

/** Data associated with a MemberDef in an inheritance relation. */
struct MemberInfo {
   MemberInfo(QSharedPointer<MemberDef> md, Protection p, Specifier v, bool inh)
       : memberDef(md), prot(p), virt(v), inherited(inh)
   {}

   ~MemberInfo() {}

   QSharedPointer<MemberDef> memberDef;
   Protection prot;
   Specifier  virt;

   bool     inherited;
   QString  scopePath;
   QString  ambiguityResolutionScope;

   QSharedPointer<ClassDef> ambigClass;
};

/** Class representing all MemberInfo objects with the same name */
class MemberNameInfo : public QList<MemberInfo>
{
 public:
   MemberNameInfo(const QString &name);
   ~MemberNameInfo() {}

   QString memberName() const {
      return name;
   }

 private:
   int compareValues(QSharedPointer<const MemberInfo> item1, QSharedPointer<const MemberInfo> item2) const;
   QString name;
};

#endif
