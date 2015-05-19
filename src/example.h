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

#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <QByteArray>

#include <stringmap.h>

class ClassDef;
class MemberName;

/** Data associated with an example. */
struct Example {
   QByteArray anchor;
   QByteArray name;
   QByteArray file;
};

/** A sorted dictionary of Example objects. */
class ExampleSDict : public StringMap<QSharedPointer<Example>>
{
 public:
   // CopperSpice - can add isCase
   ExampleSDict() : StringMap<QSharedPointer<Example>>() {}
   ~ExampleSDict() {}

 private:
   int compareMapValues(const QSharedPointer<Example> &item1, const QSharedPointer<Example> &item2) const override {
      return qstricmp(item1->name, item2->name);
   }
};

#endif
