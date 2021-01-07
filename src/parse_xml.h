/************************************************************************
*
* Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
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

#ifndef PARSE_XML_H
#define PARSE_XML_H

#include <parse_base.h>
#include <code_xml.h>

// XML scanner, only supports syntax highlighting
class XML_Parser : public ParserInterface
{
 public:
    virtual ~XML_Parser() {}

    void finishTranslationUnit() override {};

    void parseInput(const QString &fileName, const QString &fileBuffer, QSharedPointer<Entry> root,
                   enum ParserMode mode, QStringList &includeFiles, bool useClang = false) override {
      (void) fileName;
      (void) fileBuffer;
      (void) root;
      (void) mode;
      (void) includeFiles;
      (void) useClang;
    }

    bool needsPreprocessing(const QString &extension) const override {
      (void) extension;
      return false;
    }

    void parseCode(CodeGenerator &codeOutIntf, const QString &scopeName, const QString &input, SrcLangExt lang,
                  bool isExampleBlock, const QString &exampleName = QString(),
                  QSharedPointer<FileDef> fileDef = QSharedPointer<FileDef>(),
                  int startLine = -1, int endLine = -1, bool inlineFragment = false,
                  QSharedPointer<MemberDef> memberDef = QSharedPointer<MemberDef>(), bool showLineNumbers = true,
                  QSharedPointer<Definition> searchCtx = QSharedPointer<Definition>(), bool collectXRefs = true) override
   {
      parseXmlCode(codeOutIntf, scopeName, input, lang,
                  isExampleBlock, exampleName, fileDef, startLine, endLine, inlineFragment,
                  memberDef, showLineNumbers, searchCtx, collectXRefs);
    }

   void resetCodeParserState()
   {
      resetXmlCodeParserState();
   }

   void parsePrototype(const QString &text) override {
      (void) text;
   }
};

#endif
