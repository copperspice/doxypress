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

#include <doxy_globals.h>
#include <outputgen.h>
#include <parser_make.h>

static CodeOutputInterface     *g_code;
static const char              *g_currentFontClass;
static QSharedPointer<FileDef> g_sourceFileDef;
static int                     g_yyLineNr;

void MakeFileParser::parseCode(CodeOutputInterface &codeOutIntf, const char *scopeName, const QByteArray &input, 
         SrcLangExt xx, bool isExampleBlock, const char *exampleName, QSharedPointer<FileDef> fileDef, int startLine, int endLine, 
         bool inlineFragment, QSharedPointer<MemberDef> memberDef, bool showLineNumbers, 
         QSharedPointer<Definition> searchCtx, bool collectXRefs )
{
 
   if (input.isEmpty()) {
      return;
   }

   g_code = &codeOutIntf;  
   g_sourceFileDef = fileDef;

   g_currentFontClass = 0;
   g_yyLineNr = 1;

   QStringList lines = QString(input).split("\n");
     
   for (auto s : lines ) { 
      this->startCodeLine();
      g_code->codify(s.toUtf8());
      this->endCodeLine();
   } 
}

//  **

void MakeFileParser::startCodeLine()
{    
   g_code->writeLineNumber(0, 0, 0, g_yyLineNr);

   if (g_currentFontClass) {
      g_code->startFontClass(g_currentFontClass);
   }

   ++g_yyLineNr;
}

void MakeFileParser::endCodeLine()
{
   endFontClass();
   g_code->endCodeLine();
}

void MakeFileParser::startFontClass(const char *s)
{
   endFontClass();
   g_code->startFontClass(s);
   g_currentFontClass = s;
}

void MakeFileParser::endFontClass()
{
   if (g_currentFontClass) {
      g_code->endFontClass();
      g_currentFontClass = 0;
   }
}

bool MakeFileParser::needsPreprocessing(const QString &)
{
   return false;
}

void MakeFileParser::parseInput(const char *fileName, const char *fileBuf, QSharedPointer<Entry> root,
                                enum ParserMode mode, QStringList &includedFiles, bool useClang) 
{
}

void MakeFileParser::parsePrototype(const char *text) 
{
}

void MakeFileParser::resetCodeParserState()
{
}

