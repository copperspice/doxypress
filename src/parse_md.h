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

#ifndef PARSE_MD_H
#define PARSE_MD_H

#include <parse_base.h>

class Entry;

/** processes and convert markdown into DoxyPress html commands. */
QString processMarkdown(const QString &fileName, const int lineNr, QSharedPointer<Entry> e,
      const QString &s, int indentLevel = 0);

QString markdownFileNameToId(const QString &fileName);

class MarkdownFileParser : public ParserInterface
{
 public:
   virtual ~MarkdownFileParser() {}

   void finishTranslationUnit() override {}

   void parseInput(const QString &fileName, const QString &fileBuf, QSharedPointer<Entry>root,
      enum ParserMode mode, QStringList &includeFiles, bool useClang = false) override;

   bool needsPreprocessing(const QString &) const override {
      return false;
   }

   void parseCode(CodeGenerator &codeOutIntf, const QString &scopeName, const QString &input, SrcLangExt lang,
                  bool isExampleBlock, const QString &exampleName = QString(),
                  QSharedPointer<FileDef> fileDef = QSharedPointer<FileDef>(),
                  int startLine = -1, int endLine = -1, bool inlineFragment = false,
                  QSharedPointer<MemberDef> memberDef = QSharedPointer<MemberDef>(), bool showLineNumbers = true,
                  QSharedPointer<Definition> searchCtx = QSharedPointer<Definition>(), bool collectXRefs = true) override ;

   void resetCodeParserState() override;
   void parsePrototype(const QString &text) override;
};

#endif
