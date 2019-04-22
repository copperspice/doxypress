/************************************************************************
*
* Copyright (C) 2014-2019 Barbara Geller & Ansel Sermersheim
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

#ifndef RTFDOCVISITOR_H
#define RTFDOCVISITOR_H

#include <QStack>
#include <QTextStream>

#include <docvisitor.h>

class CodeOutputInterface;

/*! @brief Concrete visitor implementation for RTF output. */
class RTFDocVisitor : public DocVisitor
{
 public:
   RTFDocVisitor(QTextStream &t, CodeOutputInterface &ci, const QString &langExt);

   // visitor functions for leaf nodes

   void visit(DocWord *) override;
   void visit(DocLinkedWord *) override;
   void visit(DocWhiteSpace *) override;
   void visit(DocSymbol *) override;
   void visit(DocURL *) override;
   void visit(DocLineBreak *) override;
   void visit(DocHorRuler *) override;
   void visit(DocStyleChange *) override;
   void visit(DocVerbatim *) override;
   void visit(DocAnchor *) override;
   void visit(DocInclude *) override;
   void visit(DocIncOperator *) override;
   void visit(DocFormula *) override;
   void visit(DocIndexEntry *) override;
   void visit(DocSimpleSectSep *) override;
   void visit(DocCite *) override;

   // visitor functions for compound nodes

   void visitPre(DocAutoList *) override;
   void visitPost(DocAutoList *) override;
   void visitPre(DocAutoListItem *) override;
   void visitPost(DocAutoListItem *) override;
   void visitPre(DocPara *) override;
   void visitPost(DocPara *) override;
   void visitPre(DocRoot *) override;
   void visitPost(DocRoot *) override;
   void visitPre(DocSimpleSect *) override;
   void visitPost(DocSimpleSect *) override;
   void visitPre(DocTitle *) override;
   void visitPost(DocTitle *) override;
   void visitPre(DocSimpleList *) override;
   void visitPost(DocSimpleList *) override;
   void visitPre(DocSimpleListItem *) override;
   void visitPost(DocSimpleListItem *) override;
   void visitPre(DocSection *s) override;
   void visitPost(DocSection *) override;
   void visitPre(DocHtmlList *s) override;
   void visitPost(DocHtmlList *s) override;
   void visitPre(DocHtmlListItem *) override;
   void visitPost(DocHtmlListItem *) override;

   //void visitPre(DocHtmlPre *);
   //void visitPost(DocHtmlPre *);

   void visitPre(DocHtmlDescList *) override;
   void visitPost(DocHtmlDescList *) override;
   void visitPre(DocHtmlDescTitle *) override;
   void visitPost(DocHtmlDescTitle *) override;
   void visitPre(DocHtmlDescData *) override;
   void visitPost(DocHtmlDescData *) override;
   void visitPre(DocHtmlTable *t) override;
   void visitPost(DocHtmlTable *t) override;
   void visitPre(DocHtmlCaption *) override;
   void visitPost(DocHtmlCaption *) override;
   void visitPre(DocHtmlRow *) override;
   void visitPost(DocHtmlRow *)  override;
   void visitPre(DocHtmlCell *) override;
   void visitPost(DocHtmlCell *) override;
   void visitPre(DocInternal *) override;
   void visitPost(DocInternal *) override;
   void visitPre(DocHRef *) override;
   void visitPost(DocHRef *) override;
   void visitPre(DocHtmlHeader *) override;
   void visitPost(DocHtmlHeader *)  override;
   void visitPre(DocImage *) override;
   void visitPost(DocImage *) override;
   void visitPre(DocDotFile *) override;
   void visitPost(DocDotFile *) override;
   void visitPre(DocMscFile *) override;
   void visitPost(DocMscFile *) override;
   void visitPre(DocDiaFile *) override;
   void visitPost(DocDiaFile *) override;
   void visitPre(DocLink *) override;
   void visitPost(DocLink *) override;
   void visitPre(DocRef *ref) override;
   void visitPost(DocRef *) override;
   void visitPre(DocSecRefItem *) override;
   void visitPost(DocSecRefItem *) override;
   void visitPre(DocSecRefList *) override;
   void visitPost(DocSecRefList *) override;
   void visitPre(DocParamSect *) override;
   void visitPost(DocParamSect *) override;
   void visitPre(DocParamList *) override;
   void visitPost(DocParamList *) override;
   void visitPre(DocXRefItem *) override;
   void visitPost(DocXRefItem *) override;
   void visitPre(DocInternalRef *) override;
   void visitPost(DocInternalRef *) override;
   void visitPre(DocCopy *) override;
   void visitPost(DocCopy *) override;
   void visitPre(DocText *) override;
   void visitPost(DocText *) override;
   void visitPre(DocHtmlBlockQuote *) override;
   void visitPost(DocHtmlBlockQuote *)  override; 
   void visitPre(DocParBlock *) override;
   void visitPost(DocParBlock *) override;

 private:
   void filter(const QString &str, bool verbatim = false);
   void startLink(const QString &ref, const QString &file, const QString &anchor);
   void endLink(const QString &ref);

   QString getStyle(const QString &name);
   void incIndentLevel();
   void decIndentLevel();

   void pushEnabled();
   void popEnabled();

   void includePicturePreRTF(const QString name, const bool isTypeRTF, const bool hasCaption);
   void includePicturePostRTF(const bool isTypeRTF, const bool hasCaption);

   void writeDotFile(const QString &fileName, const bool hasCaption);
   void writeDotFile(DocDotFile *);
   void writeMscFile(const QString &fileName, const bool hasCaption);
   void writeMscFile(DocMscFile *);
   void writeDiaFile(DocDiaFile *);
   void writePlantUMLFile(const QString &fileName, const bool hasCaption);

   QTextStream &m_t;
   CodeOutputInterface &m_ci;
   bool m_insidePre;
   bool m_hide;
   int m_indentLevel;
   QStack<bool> m_enabled;
   bool m_lastIsPara;
   QString m_langExt;
};

#endif
