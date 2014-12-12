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

#ifndef CLASSLIST_H
#define CLASSLIST_H

#include <QList>
#include <QHash>

#include <classdef.h>
#include <stringmap.h>

class Definition;

/** An unsorted dictionary of ClassDef objects. */
class ClassDict : public QHash<QString, ClassDef>
{
 public:
   ClassDict() : QHash<QString, ClassDef>() {}
   ~ClassDict() {}
};

/** A sorted dictionary of ClassDef objects. */
class ClassSDict : public StringMap<QSharedPointer<ClassDef>>
{
 public:
   ClassSDict() : StringMap<QSharedPointer<ClassDef>>() {}
   ~ClassSDict() {}

   void writeDeclaration(OutputList &ol, const ClassDef::CompoundType *filter = 0,
                         const char *header = 0, bool localNames = false);

   void writeDocumentation(OutputList &ol, Definition *container = 0);
   bool declVisible(const ClassDef::CompoundType *filter = 0) const;

 private:
   int compareValues(const ClassDef *item1, const ClassDef *item2) const;
};

class GenericsSDict
{
 public:
   GenericsSDict() : m_dict()
   {
   }

   ~GenericsSDict() {}

   void insert(const QByteArray &key, ClassDef *cd);
   ClassDef *find(const QByteArray &key);

 private:
   StringMap<QSharedPointer<QHash<long, ClassDef *>>> m_dict;
};

#endif
