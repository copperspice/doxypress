/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim 
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

#ifndef CLANGPARSER_H
#define CLANGPARSER_H

#include <QStringList>

#include <clang-c/Index.h>
#include <clang-c/Documentation.h>

class CodeOutputInterface;
class FileDef;

class ClangParser
{
 public:  
   static ClangParser *instance();

   // Start parsing a file, returns a subset of the files only including the
   // ones which were actually found during parsing

   // fileName- name of the file to parse
   // filesInTranslationUnit- other files which are included by this file
   void start(const QString &fileName, QStringList &includeFiles);

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

  private:
   ClangParser();
   virtual ~ClangParser();

   class Private;
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
