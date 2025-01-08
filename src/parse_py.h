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

#ifndef PARSE_PY_H
#define PARSE_PY_H

#include <definition.h>
#include <outputgen.h>
#include <parse_base.h>
#include <types.h>

#include <QByteArray>
#include <QStringList>

/** Python Parser using state-based lexical scanning.
 *
 * This is the Python language parser for doxyPress.
 */
class Python_Parser : public ParserInterface
{
 public:
   virtual ~Python_Parser() {}

   void finishTranslationUnit() override {}

   void parseInput(const QString &fileName, const QString &fileBuf, QSharedPointer<Entry> root,
                   enum ParserMode mode, QStringList &includeFiles, bool useClang = false) override;

   bool needsPreprocessing(const QString &extension) const override;

   void parseCode(CodeGenerator &codeOutIntf, const QString &scopeName, const QString &input, SrcLangExt lang,
                  bool isExampleBlock, const QString &exampleName = QString(),
                  QSharedPointer<FileDef> fileDef = QSharedPointer<FileDef>(),
                  int startLine = -1, int endLine = -1, bool inlineFragment = false,
                  QSharedPointer<MemberDef> memberDef = QSharedPointer<MemberDef>(), bool showLineNumbers = true,
                  QSharedPointer<Definition> searchCtx = QSharedPointer<Definition>(), bool collectXRefs = true) override;

   void resetCodeParserState() override;
   void parsePrototype(const QString &text) override;
};

extern void parsePythonCode(CodeGenerator &od, const QString &, const QString &s, bool exBlock, const QString &exName,
                  QSharedPointer<FileDef> fd, int startLine, int endLine, bool inlineFragment,
                  QSharedPointer<MemberDef> memberDef, bool showLineNumbers, QSharedPointer<Definition> searchCtx,
                  bool collectXRefs);

extern void resetPythonCodeParserState();

void pyFreeParser();

#endif
