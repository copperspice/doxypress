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

   QByteArray name;
   QByteArray definition;
   QByteArray fileName;
   QByteArray doc;
   QByteArray brief;
   QByteArray args;
   QByteArray anchor;

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
      return qstricmp(d1->name, d2->name);
   }
};

/** A list of A_Define objects associated with a specific name. */
class DefineName : public QList<A_Define>
{
 public:
   DefineName(const char *n) : QList<A_Define>() {
      name = n;
   }

   ~DefineName() {}

   const char *nameString() const {
      return name;
   }

 private:
   int compareValues(const A_Define *d1, const A_Define *d2) const {
      return qstricmp(d1->name, d2->name);
   }

   QByteArray name;
};

/** A list of DefineName objects. */
class DefineNameList : public QList<DefineName>
{
 public:
   DefineNameList() : QList<DefineName>() {}
   ~DefineNameList() {}

 private:
   int compareValues(const DefineName *n1, const DefineName *n2) const {
      return qstricmp(n1->nameString(), n2->nameString());
   }
};

/** An unsorted dictionary of A_Define objects. */
using DefineDict     = QHash<QString, A_Define *>;      // only used in pre.cpp (lex code)

/** A sorted dictionary of DefineName object. */
using DefineNameDict = QHash<QString, DefineName>;

#endif
