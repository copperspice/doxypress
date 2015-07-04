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

#ifndef CLASSLIST_H
#define CLASSLIST_H

#include <QList>
#include <QHash>

#include <classdef.h>
#include <definition.h>
#include <stringmap.h>

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
   ClassSDict(Qt::CaseSensitivity isCase = Qt::CaseSensitive) 
      : StringMap<QSharedPointer<ClassDef>>(isCase)
   {}

   ~ClassSDict() {}

   void writeDeclaration(OutputList &ol, const ClassDef::CompoundType *filter = 0,
                         const QString &header = 0, bool localNames = false);

   void writeDocumentation(OutputList &ol, Definition *container = 0);
   bool declVisible(const ClassDef::CompoundType *filter = 0) const;

 private:
   int compareMapValues(const QSharedPointer<ClassDef> &item1, const QSharedPointer<ClassDef> &item2) const override;
};

class GenericsSDict
{
 public:
   GenericsSDict() : m_dict()
   {
   }

   ~GenericsSDict() {}

   void insert(const QByteArray &key, QSharedPointer<ClassDef> cd);
   QSharedPointer<ClassDef> find(const QByteArray &key);

 private:
   StringMap<QSharedPointer<QHash<long, QSharedPointer<ClassDef>>>> m_dict;
};

#endif
