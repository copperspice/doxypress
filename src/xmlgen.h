/************************************************************************
*
* Copyright (c) 2014-2022 Barbara Geller & Ansel Sermersheim
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

#ifndef XMLGEN_H
#define XMLGEN_H

#include <outputgen.h>

class XMLCodeGenerator : public CodeGenerator
{
 public:

   XMLCodeGenerator(QTextStream &t) : m_t(t), m_lineNumber(-1), m_isMemberRef(false), m_col(0),
      m_insideCodeLine(false), m_normalHLNeedStartTag(true), m_insideSpecialHL(false)
   { }

   virtual ~XMLCodeGenerator()
   { }

   void codify(const QString &text) override;


   void writeCodeLink(const QString &ref, const QString &file, const QString &anchor,
                      const QString &name, const QString &tooltip) override;
   void writeTooltip(const QString &, const DocLinkInfo &, const QString &,
                     const QString &, const SourceLinkInfo &, const SourceLinkInfo &) override;

   void startCodeLine(bool) override;
   void endCodeLine() override;

   void startFontClass(const QString &colorClass) override ;
   void endFontClass() override;

   void writeCodeAnchor(const QString &) override;
   void writeLineNumber(const QString &extRef, const QString &compId, const QString &anchorId, int l) override;

   void setCurrentDoc(QSharedPointer<Definition> def, const QString &, bool)  override {
      (void) def;
   }

   void addWord(const QString &, bool) override {
   }

   void startCodeFragment(const QString &) override;
   void endCodeFragment(const QString &) override;

   void finish();

 private:
   QTextStream &m_t;
   QString m_refId;
   QString m_external;

   int m_lineNumber;
   bool m_isMemberRef;
   int m_col;

   bool m_insideCodeLine;
   bool m_normalHLNeedStartTag;
   bool m_insideSpecialHL;
};

void generateXML_output();

#endif
