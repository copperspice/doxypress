/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
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

#ifndef PARSE_CLANG_H
#define PARSE_CLANG_H

#include <QStringList>

#include <clang-c/Index.h>
#include <clang-c/Documentation.h>

class CodeOutputInterface;
class Entry;
class FileDef;

class ClangParser
{
 public:
   static ClangParser *instance();

   // fileName  name of the file to parse
   // includeFiles- other files which are included by this file
   void start(const QString &fileName, const QString &fileBuffer, QStringList &includeFiles, QSharedPointer<Entry> root);

   // Switch to another file within the translation unit
   // fileName - name of the file to switch to
   void switchToFile(const QString &fileName);

   // clean up, free resources used in parsing
   void finish();

   // looks for a symbol which should be found at line, returns a clang unique ref to the symbol
   QString lookup(uint line, const QString &symbol);

   // writes the syntax highlighted source code for a file
   // ol- output generator list to write to, fd- file to write sources for
   void writeSources(CodeOutputInterface &ol, QSharedPointer<FileDef> fd);

   class Private;
   Private *getPrivate();

 private:
   ClangParser();
   virtual ~ClangParser();

   Private *p;

   void linkIdentifier(CodeOutputInterface &ol, QSharedPointer<FileDef> fd, uint &line,
                  uint &column, const QString &text, int tokenIndex);

   void linkMacro(CodeOutputInterface &ol, QSharedPointer<FileDef> fd, uint &line,
                  uint &column, const QString &text);

   void linkInclude(CodeOutputInterface &ol, QSharedPointer<FileDef> fd, uint &line,
                  uint &column, const QString &text);

   void determineInputFiles(QStringList &includeFiles);
};

#endif
