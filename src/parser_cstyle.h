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
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#ifndef PARSER_CSTYLE_H
#define PARSER_CSTYLE_H

#include <parserintf.h>

/** \brief C-like language parser using state-based lexical scanning.
 *
 *  This is the language parser for doxyPress.  
 *  It supports C++ and various languages which are closely related to C++,
 *  such as C, C#, Objective-C, Java, PHP, and IDL.
 */
class CPPLanguageParser : public ParserInterface
{
 public:
   virtual ~CPPLanguageParser() {}

   void startTranslationUnit(const char *fileName);
   void finishTranslationUnit();

   void parseInput(const char *fileName, const char *fileBuf, QSharedPointer<Entry> root,
                   enum ParserMode mode, QStringList &includeFiles) override;

   bool needsPreprocessing(const QByteArray &extension);

   void parseCode(CodeOutputInterface &codeOutIntf, const char *scopeName, const QByteArray &input, SrcLangExt lang,
                  bool isExampleBlock, const char *exampleName = 0, 
                  QSharedPointer<FileDef> fileDef = QSharedPointer<FileDef>(),
                  int startLine = -1, int endLine = -1, bool inlineFragment = false,
                  QSharedPointer<MemberDef> memberDef = QSharedPointer<MemberDef>(), bool showLineNumbers = true,
                  QSharedPointer<Definition> searchCtx = QSharedPointer<Definition>(), bool collectXRefs = true) override ;

   void resetCodeParserState();
   void parsePrototype(const char *text);
};

void CPPScanFreeParser();

#endif
