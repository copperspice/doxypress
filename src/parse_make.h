/************************************************************************
*
* Copyright (c) 2014-2023 Barbara Geller & Ansel Sermersheim
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

#ifndef PARSE_MAKE_H
#define PARSE_MAKE_H

#include <QStringList>

#include <parse_base.h>

/** \brief Make Language parser using state-based lexical scanning.
 *
 * This is the Make language parser for doxyPress.
 */
class MakeFileParser : public ParserInterface
{
   public:
      virtual ~MakeFileParser() {}

      void finishTranslationUnit()  override {}

      void parseInput(const QString &fileName, const QString &fileBuf, QSharedPointer<Entry> root,
                      enum ParserMode mode, QStringList &includeFiles, bool useClang = false) override;

      bool needsPreprocessing(const QString &extension) const override;

      void parseCode(CodeGenerator &codeOutIntf, const QString &scopeName, const QString &input, SrcLangExt lang,
                  bool isExampleBlock, const QString &exampleName = QString(),
                  QSharedPointer<FileDef> fileDef = QSharedPointer<FileDef>(),
                  int startLine = -1, int endLine = -1, bool inlineFragment = false,
                  QSharedPointer<MemberDef> memberDef = QSharedPointer<MemberDef>(), bool showLineNumbers = true,
                  QSharedPointer<Definition> searchCtx = QSharedPointer<Definition>(), bool collectXRefs = true) override ;

      void resetCodeParserState() override ;
      void parsePrototype(const QString &text) override;

   private:
        void endCodeLine();
        void startCodeLine();
        void endFontClass();
        void startFontClass(const QString &s);
};

#endif
