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

#ifndef PARSER_PY_H
#define PARSER_PY_H

#include <QByteArray>
#include <QStringList>

#include <types.h>

#include <definition.h>
#include <filedef.h>
#include <memberdef.h>
#include <outputgen.h>
#include <parserintf.h>

/** \brief Python Language parser using state-based lexical scanning.
 *
 * This is the Python language parser for doxygen.
 */
class PythonLanguageScanner : public ParserInterface
{
 public:
   virtual ~PythonLanguageScanner() {}

   void startTranslationUnit(const char *) {}
   void finishTranslationUnit() {}

   void parseInput(const char *fileName, const char *fileBuf, QSharedPointer<Entry> root,
                   bool sameTranslationUnit, QStringList &filesInSameTranslationUnit) override;

   bool needsPreprocessing(const QByteArray &extension);
   void parseCode(CodeOutputInterface &codeOutIntf, const char *scopeName, const QByteArray &input,
                  SrcLangExt lang, bool isExampleBlock, const char *exampleName = 0,
                  FileDef *fileDef = 0, int startLine = -1, int endLine = -1, bool inlineFragment = false,
                  MemberDef *memberDef = 0, bool showLineNumbers = true, Definition *searchCtx = 0, 
                  bool collectXrefs = true );

   void resetCodeParserState();
   void parsePrototype(const char *text);
};

extern void parsePythonCode(CodeOutputInterface &, const char *, const QByteArray &,
                            bool , const char *, FileDef *fd, int startLine, int endLine, bool inlineFragment,
                            MemberDef *memberDef, bool showLineNumbers, Definition *searchCtx, bool collectXRefs);

extern void resetPythonCodeParserState();

void pyscanFreeScanner();

#endif
