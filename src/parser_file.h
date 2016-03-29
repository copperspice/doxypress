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

#ifndef PARSER_FILE_H
#define PARSER_FILE_H

#include <parser_base.h>

/** @brief General file parser */
class FileParser : public ParserInterface
{
 public:
   virtual ~FileParser() {} 
   void finishTranslationUnit() override {}

   void parseInput(const QString &, const QString &, QSharedPointer<Entry>, 
                  enum ParserMode mode, QStringList &includeFiles, bool useClang = false) override {};

   bool needsPreprocessing(const QString &) override {
      return false;
   }

   void parseCode(CodeOutputInterface &codeOutIntf, const QString &scopeName, const QString &input, SrcLangExt lang,
                  bool isExampleBlock, const QString &exampleName = QString(), 
                  QSharedPointer<FileDef> fileDef = QSharedPointer<FileDef>(),
                  int startLine = -1, int endLine = -1, bool inlineFragment = false,
                  QSharedPointer<MemberDef> memberDef = QSharedPointer<MemberDef>(), bool showLineNumbers = true,
                  QSharedPointer<Definition> searchCtx = QSharedPointer<Definition>(), bool collectXRefs = true) override;

   void resetCodeParserState() {}
   void parsePrototype(const QString &) {}
};


#endif
