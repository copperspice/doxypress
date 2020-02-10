/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
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

   bool needsPreprocessing(const QString &extension) const override;

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
