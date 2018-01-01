/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim 
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

#include <parse_make.h>

#include <doxy_globals.h>
#include <outputgen.h>

static CodeOutputInterface     *g_code;
static QString                 g_currentFontClass;
static QSharedPointer<FileDef> g_sourceFileDef;
static int                     g_yyLineNr;

void MakeFileParser::parseCode(CodeOutputInterface &codeOutIntf, const QString &scopeName, const QString &input, 
         SrcLangExt xx, bool isExampleBlock, const QString &exampleName, QSharedPointer<FileDef> fileDef, int startLine, int endLine, 
         bool inlineFragment, QSharedPointer<MemberDef> memberDef, bool showLineNumbers, 
         QSharedPointer<Definition> searchCtx, bool collectXRefs )
{
 
   if (input.isEmpty()) {
      return;
   }

   g_code = &codeOutIntf;  
   g_sourceFileDef = fileDef;

   g_currentFontClass = "";
   g_yyLineNr = 1;

   QStringList lines = QString(input).split("\n");
     
   for (auto s : lines ) { 
      this->startCodeLine();
      g_code->codify(s);
      this->endCodeLine();
   } 
}

void MakeFileParser::startCodeLine()
{    
   g_code->writeLineNumber(0, 0, 0, g_yyLineNr);

   if (! g_currentFontClass.isEmpty()) {
      g_code->startFontClass(g_currentFontClass);
   }

   ++g_yyLineNr;
}

void MakeFileParser::endCodeLine()
{
   endFontClass();
   g_code->endCodeLine();
}

void MakeFileParser::startFontClass(const QString &s)
{
   endFontClass();
   g_code->startFontClass(s);
   g_currentFontClass = s;
}

void MakeFileParser::endFontClass()
{
   if (! g_currentFontClass.isEmpty()) {
      g_code->endFontClass();
      g_currentFontClass = "";
   }
}

bool MakeFileParser::needsPreprocessing(const QString &)
{
   return false;
}

void MakeFileParser::parseInput(const QString &fileName, const QString &fileBuf, QSharedPointer<Entry> root,
                                enum ParserMode mode, QStringList &includedFiles, bool useClang) 
{
}

void MakeFileParser::parsePrototype(const QString &text) 
{
}

void MakeFileParser::resetCodeParserState()
{
}
