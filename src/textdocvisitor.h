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

#ifndef TEXTDOCVISITOR_H
#define TEXTDOCVISITOR_H

#include <QByteArray>
#include <QStack>
#include <QTextStream>

#include <docvisitor.h>
#include <docparser.h>


/*! @brief Concrete visitor implementation for TEXT output. */
class TextDocVisitor : public DocVisitor
{
 public:
   TextDocVisitor(QTextStream &t) : DocVisitor(DocVisitor_Text), m_t(t) {}

   // visitor functions for leaf nodes
   void visit(DocWord *w) override {
      filter(w->word());
   }

   void visit(DocLinkedWord *w) override {
      filter(w->word());
   }

   void visit(DocWhiteSpace *) override {
      m_t << " ";
   }

   void visit(DocSymbol *) override;

   void visit(DocURL *u) override {
      filter(u->url());
   }

   void visit(DocLineBreak *) override {
      m_t << " ";
   }

   void visit(DocHorRuler *) override {}
   void visit(DocStyleChange *) override {}

   void visit(DocVerbatim *s) override{
      filter(s->text());
   }

   void visit(DocAnchor *)  override {}
   void visit(DocInclude *) override {}
   void visit(DocIncOperator *) override {}
   void visit(DocFormula *)  override {}
   void visit(DocIndexEntry *) override  {}
   void visit(DocSimpleSectSep *) override {}
   void visit(DocCite *) override {}

   // visitor functions for compound nodes
   void visitPre(DocAutoList *) override {}
   void visitPost(DocAutoList *) override {}
   void visitPre(DocAutoListItem *) override {}
   void visitPost(DocAutoListItem *) override {}
   void visitPre(DocPara *) override {}
   void visitPost(DocPara *) override {}
   void visitPre(DocRoot *) override {}
   void visitPost(DocRoot *) override {}
   void visitPre(DocSimpleSect *) override {}
   void visitPost(DocSimpleSect *) override {}
   void visitPre(DocTitle *) override {}
   void visitPost(DocTitle *) override {}
   void visitPre(DocSimpleList *) override {}
   void visitPost(DocSimpleList *) override {}
   void visitPre(DocSimpleListItem *) override {}
   void visitPost(DocSimpleListItem *) override {}
   void visitPre(DocSection *) override {}
   void visitPost(DocSection *) override {}
   void visitPre(DocHtmlList *) override {}
   void visitPost(DocHtmlList *) override {}
   void visitPre(DocHtmlListItem *) override {}
   void visitPost(DocHtmlListItem *) override {}
   void visitPre(DocHtmlDescList *) override {}
   void visitPost(DocHtmlDescList *) override {}
   void visitPre(DocHtmlDescTitle *) override {}
   void visitPost(DocHtmlDescTitle *) override {}
   void visitPre(DocHtmlDescData *) override {}
   void visitPost(DocHtmlDescData *) override {}
   void visitPre(DocHtmlTable *) override {}
   void visitPost(DocHtmlTable *) override {}
   void visitPre(DocHtmlRow *) override {}
   void visitPost(DocHtmlRow *) override {}
   void visitPre(DocHtmlCell *) override {}
   void visitPost(DocHtmlCell *) override {}
   void visitPre(DocHtmlCaption *) override {}
   void visitPost(DocHtmlCaption *) override {}
   void visitPre(DocInternal *) override {}
   void visitPost(DocInternal *) override {}
   void visitPre(DocHRef *) override {}
   void visitPost(DocHRef *) override {}
   void visitPre(DocHtmlHeader *) override {}
   void visitPost(DocHtmlHeader *) override {}
   void visitPre(DocImage *) override {}
   void visitPost(DocImage *) override {}
   void visitPre(DocDotFile *) override {}
   void visitPost(DocDotFile *) override {}

   void visitPre(DocMscFile *) override {}
   void visitPost(DocMscFile *) override {}
   void visitPre(DocDiaFile *) override {}
   void visitPost(DocDiaFile *) override {}
   void visitPre(DocLink *) override {}
   void visitPost(DocLink *) override {}
   void visitPre(DocRef *) override {}
   void visitPost(DocRef *) override {}
   void visitPre(DocSecRefItem *) override {}
   void visitPost(DocSecRefItem *) override {}
   void visitPre(DocSecRefList *) override {}
   void visitPost(DocSecRefList *) override {}
   void visitPre(DocParamSect *) override {}
   void visitPost(DocParamSect *) override {}
   void visitPre(DocParamList *) override {}
   void visitPost(DocParamList *) override {}
   void visitPre(DocXRefItem *) override {}
   void visitPost(DocXRefItem *) override {}
   void visitPre(DocInternalRef *) override {}
   void visitPost(DocInternalRef *) override {}
   void visitPre(DocCopy *) override {}
   void visitPost(DocCopy *) override {}
   void visitPre(DocText *) override {}
   void visitPost(DocText *) override {}
   void visitPre(DocHtmlBlockQuote *) override {}
   void visitPost(DocHtmlBlockQuote *) override {}
   void visitPre(DocParBlock *) override {}
   void visitPost(DocParBlock *) override {}

 private:
   void filter(const QString &str);
   QTextStream &m_t;
};

#endif
