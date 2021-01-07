/************************************************************************
*
* Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
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

#ifndef A_DEFINE_H
#define A_DEFINE_H

#include <QHash>

class FileDef;

/** A class representing a macro definition. */
class A_Define
{
 public:
   A_Define();

   A_Define(const A_Define &d) = delete;
   A_Define(A_Define &&d)      = delete;

   A_Define &operator=(const A_Define &d) = delete;
   A_Define &operator=(A_Define &&d)      = delete;

   bool hasDocumentation();

   QString m_name;
   QString m_definition;
   QString m_fileName;

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

/** An unsorted dictionary of A_Define objects. */
using DefineDict = QHash<QString, QSharedPointer<A_Define>>;      // only used in pre.cpp (lex code)

#endif
