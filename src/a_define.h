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

#ifndef A_DEFINE_H
#define A_DEFINE_H

#include <QHash>
#include <QList>

class FileDef;

/** A class representing a macro definition. */
class A_Define
{
 public:
   A_Define();
   A_Define(const A_Define &d);

   ~A_Define();

   bool hasDocumentation();

   QString name;
   QString definition;
   QString fileName;
   QString doc;
   QString brief;
   QString args;
   QString anchor;

   QSharedPointer<FileDef> fileDef;

   int lineNr;
   int columnNr;
   int nargs;

   bool undef;
   bool varArgs;
   bool isPredefined;
   bool nonRecursive;
};

/** A list of A_Define objects. */
class DefineList : public QList<A_Define>
{
 public:
   DefineList() : QList<A_Define>() {}
   ~DefineList() {}

 private:
   int compareValues(const A_Define *d1, const A_Define *d2) const {
      return d1->name.compare(d2->name, Qt::CaseInsensitive);
   }
};

/** A list of A_Define objects associated with a specific name. */
class DefineName : public QList<A_Define>
{
 public:
   DefineName(const QString &n) : QList<A_Define>() {
      name = n;
   }

   ~DefineName() {}

   const QString &nameString() const {
      return name;
   }

 private:
   int compareValues(const A_Define *d1, const A_Define *d2) const {
      return d1->name.compare(d2->name, Qt::CaseInsensitive);   
   }

   QString name;
};

/** A list of DefineName objects. */
class DefineNameList : public QList<DefineName>
{
 public:
   DefineNameList() : QList<DefineName>() {}
   ~DefineNameList() {}

 private:
   int compareValues(const DefineName *n1, const DefineName *n2) const {
      return n1->nameString().compare(n2->nameString(), Qt::CaseInsensitive);
   }
};

/** An unsorted dictionary of A_Define objects. */
using DefineDict     = QHash<QString, A_Define *>;      // only used in pre.cpp (lex code)

/** A sorted dictionary of DefineName object. */
using DefineNameDict = QHash<QString, DefineName>;

#endif
