/************************************************************************
*
* Copyright (c) 2014-2022 Barbara Geller & Ansel Sermersheim
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

#ifndef MEMBERNAME_H
#define MEMBERNAME_H

#include <QList>

#include <memberdef.h>

/** Class representing all MemberDef objects with the same name */
class MemberName : public QList<QSharedPointer<MemberDef>>
{
 public:
   MemberName(const QString &name);
  
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
