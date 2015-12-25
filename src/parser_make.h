/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
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

#ifndef PARSER_MAKE_H
#define PARSER_MAKE_H

#include <QStringList>

#include <parser_base.h>

/** \brief Make Language parser using state-based lexical scanning.
 *
 * This is the Make language parser for doxyPress.
 */
class MakeFileParser : public ParserInterface
{
   public:
      virtual ~MakeFileParser() {}
   
      void startTranslationUnit(const QString &) override {}
      void finishTranslationUnit()  override {}
   
      void parseInput(const QString &fileName, const QString &fileBuf, QSharedPointer<Entry> root,
                      enum ParserMode mode, QStringList &includeFiles, bool useClang = false) override;
   
      bool needsPreprocessing(const QString &extension);

      void parseCode(CodeOutputInterface &codeOutIntf, const QString &scopeName, const QString &input, SrcLangExt lang,
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
