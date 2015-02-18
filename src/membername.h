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
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#ifndef MEMBERNAME_H
#define MEMBERNAME_H

#include <QList>

#include <memberdef.h>
#include <stringmap.h>

/** Class representing all MemberDef objects with the same name */
class MemberName : public QList<QSharedPointer<MemberDef>>
{
 public:
   MemberName(const char *name);
   ~MemberName();

   const char *memberName() const {
      return name;
   }

 private:
   int compareValues(QSharedPointer<const MemberDef> item1, QSharedPointer<const MemberDef> item2) const;
   QByteArray name;
};

/** Iterator for MemberDef objects in a MemberName list. */
class MemberNameIterator : public QListIterator<QSharedPointer<MemberDef>>
{
 public:
   MemberNameIterator( const MemberName &list);
};

/** Sorted dictionary of MemberName objects. */
class MemberNameSDict : public StringMap<QSharedPointer<MemberName>>
{
 public:
   MemberNameSDict() : StringMap<QSharedPointer<MemberName>>() {}
   ~MemberNameSDict() {}

 private:
   int compareMapValues(const QSharedPointer<MemberName> &item1, const QSharedPointer<MemberName> &item2) const override;
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

   bool         inherited;
   QByteArray   scopePath;
   QByteArray   ambiguityResolutionScope;

   QSharedPointer<ClassDef> ambigClass;
};

/** Class representing all MemberInfo objects with the same name */
class MemberNameInfo : public QList<MemberInfo>
{
 public:
   MemberNameInfo(const char *name);
   ~MemberNameInfo() {}

   const char *memberName() const {
      return name;
   }

 private:
   int compareValues(QSharedPointer<const MemberInfo> item1, QSharedPointer<const MemberInfo> item2) const;
   QByteArray name;
};

/** Iterator for MemberInfo objects in a MemberNameInfo list. */
class MemberNameInfoIterator : public QListIterator<MemberInfo>
{
 public:
   MemberNameInfoIterator(const MemberNameInfo &mnii)
      : QListIterator<MemberInfo>(mnii) {}
};

/** Sorted dictionary of MemberNameInfo objects. */
class MemberNameInfoSDict : public StringMap<QSharedPointer<MemberNameInfo>>
{
 public:
   MemberNameInfoSDict() : StringMap<QSharedPointer<MemberNameInfo>>() {}
   ~MemberNameInfoSDict() {}

 private:
   int compareMapValues(const QSharedPointer<MemberNameInfo> &item1, const QSharedPointer<MemberNameInfo> &item2) const override {
      return qstricmp(item1->memberName(), item2->memberName());
   }
};

#endif
