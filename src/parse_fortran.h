/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim
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

#ifndef PARSE_FORTRAN_H
#define PARSE_FORTRAN_H

#include <parse_base.h>

/** \brief Fortran language parser using state-based lexical scanning.
 *
 *  This is the Fortran language parser for doxyPress.
 */
class Fortran_Parser : public ParserInterface
{
 public:
   Fortran_Parser(FortranFormat format = FortranFormat_Unknown) : m_format(format) { }
   virtual ~Fortran_Parser() {}

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

 private:
   FortranFormat m_format;
};

class Fortran_ParserFree : public Fortran_Parser
{
 public:
   Fortran_ParserFree() : Fortran_Parser(FortranFormat_Free) { }
};

class Fortran_ParserFixed : public Fortran_Parser
{
 public:
   Fortran_ParserFixed() : Fortran_Parser(FortranFormat_Fixed) { }
};

#endif
