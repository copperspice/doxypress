/************************************************************************
*
* Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
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

#ifndef PARSE_FILE_H
#define PARSE_FILE_H

#include <parse_base.h>

/** @brief General file parser */
class FileParser : public ParserInterface
{
 public:
   virtual ~FileParser()
   {
   }

   void finishTranslationUnit() override {}

   void parseInput(const QString &, const QString &, QSharedPointer<Entry>,
                  enum ParserMode mode, QStringList &includeFiles, bool useClang = false) override {
      (void) mode;
      (void) includeFiles;
      (void) useClang;
   };

   bool needsPreprocessing(const QString &) const override {
      return false;
   }

   void parseCode(CodeGenerator &codeOutIntf, const QString &scopeName, const QString &input, SrcLangExt lang,
                  bool isExampleBlock, const QString &exampleName = QString(),
                  QSharedPointer<FileDef> fileDef = QSharedPointer<FileDef>(),
                  int startLine = -1, int endLine = -1, bool inlineFragment = false,
                  QSharedPointer<MemberDef> memberDef = QSharedPointer<MemberDef>(), bool showLineNumbers = true,
                  QSharedPointer<Definition> searchCtx = QSharedPointer<Definition>(), bool collectXRefs = true) override;

   void resetCodeParserState() override {}
   void parsePrototype(const QString &) override {}
};


#endif
