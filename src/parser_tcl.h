/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
 * Copyright (C) 2010-2011 by Rene Zaumseil
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

#ifndef PARSER_TCL_H
#define PARSER_TCL_H

#include <parserintf.h>

/** \brief Tcl language parser using state-based lexical scanning.
 *
 *  This is the Tcl language parser for doxyPress.
 */
class TclLanguageParser : public ParserInterface
{
 public:
   virtual ~TclLanguageParser() {}

   void startTranslationUnit(const char *) {}
   void finishTranslationUnit() {}
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

#endif
