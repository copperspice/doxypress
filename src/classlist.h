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

class GenericsSDict
{
 public:
   GenericsSDict() : m_dict()
   {
   }

   ~GenericsSDict() {}

   void insert(const QString  &key, QSharedPointer<ClassDef> cd);
   QSharedPointer<ClassDef> find(const QString &key);

 private:
   StringMap<QSharedPointer<QHash<long, QSharedPointer<ClassDef>>>> m_dict;
};

#endif
