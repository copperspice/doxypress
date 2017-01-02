/*************************************************************************
 *
 * Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim 
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

#ifndef PARSE_PY_H
#define PARSE_PY_H

#include <QByteArray>
#include <QStringList>

#include <types.h>

#include <definition.h>
#include <outputgen.h>
#include <parse_base.h>

/** \brief Python Parser using state-based lexical scanning.
 *
 * This is the Python language parser for doxyPress.
 */
class PythonLanguageParser : public ParserInterface
{
 public:
   virtual ~PythonLanguageParser() {}
   
   void finishTranslationUnit() override {}

   void parseInput(const QString &fileName, const QString &fileBuf, QSharedPointer<Entry> root,
                   enum ParserMode mode, QStringList &includeFiles, bool useClang = false) override;

   bool needsPreprocessing(const QString &extension) override;

   void parseCode(CodeOutputInterface &codeOutIntf, const QString &scopeName, const QString &input, SrcLangExt lang,
                  bool isExampleBlock, const QString &exampleName = QString(), 
                  QSharedPointer<FileDef> fileDef = QSharedPointer<FileDef>(),
                  int startLine = -1, int endLine = -1, bool inlineFragment = false,
                  QSharedPointer<MemberDef> memberDef = QSharedPointer<MemberDef>(), bool showLineNumbers = true,
                  QSharedPointer<Definition> searchCtx = QSharedPointer<Definition>(), bool collectXRefs = true) override ;

   void resetCodeParserState() override;
   void parsePrototype(const QString &text) override;
};

extern void parsePythonCode(CodeOutputInterface &, const QString &, const QString &,
                            bool , const QString &, QSharedPointer<FileDef> fd, int startLine, int endLine, bool inlineFragment,
                            QSharedPointer<MemberDef> memberDef, bool showLineNumbers, QSharedPointer<Definition> searchCtx,
                            bool collectXRefs);

extern void resetPythonCodeParserState();

void pyFreeParser();

#endif
