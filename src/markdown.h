/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
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

#ifndef MARKDOWN_H
#define MARKDOWN_H

#include <QByteArray>

#include <parserintf.h>

class Entry;

/** processes string \a s and converts markdown into doxygen/html commands. */
QByteArray processMarkdown(const QByteArray &fileName, const int lineNr, Entry *e, const QByteArray &s);
QByteArray markdownFileNameToId(const QByteArray &fileName);

class MarkdownFileParser : public ParserInterface
{
 public:
   virtual ~MarkdownFileParser()
   {}

   void startTranslationUnit(const char *) {}
   void finishTranslationUnit() {}

   void parseInput(const char *fileName, const char *fileBuf, QSharedPointer<Entry>root, 
                  enum ParserMode mode, QStringList &includeFiles) override;

   bool needsPreprocessing(const QByteArray &) {
      return false;
   }
  
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
