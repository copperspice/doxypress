/************************************************************************
*
* Copyright (c) 2014-2025 Barbara Geller & Ansel Sermersheim
*
* Copyright (c) 1997-2014 Dimitri van Heesch
*
* DoxyPress is free software. You can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
* https://www.gnu.org/licenses/
*
*************************************************************************/

#include <parse_make.h>

#include <doxy_globals.h>
#include <outputgen.h>

static CodeGenerator          *g_code;
static QString                 g_currentFontClass;
static QSharedPointer<FileDef> g_sourceFileDef;
static int                     g_yyLineNr;

void MakeFileParser::parseCode(CodeGenerator &codeOutIntf, const QString &scopeName, const QString &input,
         SrcLangExt xx, bool isExampleBlock, const QString &exampleName, QSharedPointer<FileDef> fileDef,
         int startLine, int endLine, bool inlineFragment, QSharedPointer<MemberDef> memberDef, bool showLineNumbers,
         QSharedPointer<Definition> searchCtx, bool collectXRefs )
{
   (void) scopeName;
   (void) xx;
   (void) isExampleBlock;
   (void) exampleName;
   (void) startLine;
   (void) endLine;
   (void) inlineFragment;
   (void) memberDef;
   (void) showLineNumbers;
   (void) searchCtx;
   (void) collectXRefs;

   if (input.isEmpty()) {
      return;
   }

   g_code = &codeOutIntf;
   g_sourceFileDef = fileDef;

   g_currentFontClass = QString();
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
   g_code->writeLineNumber(QString(), QString(), QString(), g_yyLineNr);

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
      g_currentFontClass = QString();
   }
}

bool MakeFileParser::needsPreprocessing(const QString &) const
{
   return false;
}

void MakeFileParser::parseInput(const QString &fileName, const QString &fileBuf, QSharedPointer<Entry> root,
                                enum ParserMode mode, QStringList &includedFiles, bool useClang)
{
   (void) fileName;
   (void) fileBuf;
   (void) root;
   (void) mode;
   (void) includedFiles;
   (void) useClang;
}

void MakeFileParser::parsePrototype(const QString &text)
{
   (void) text;
}

void MakeFileParser::resetCodeParserState()
{
}
