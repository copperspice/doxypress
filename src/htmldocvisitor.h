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

#ifndef HTMLDOCVISITOR_H
#define HTMLDOCVISITOR_H

#include <QByteArray>
#include <QStack>
#include <QTextStream>

#include <docvisitor.h>

class CodeOutputInterface;
class Definition;
class DocNode;
class MemberDef;

/*! @brief Concrete visitor implementation for HTML output. */
class HtmlDocVisitor : public DocVisitor
{
 public:
   HtmlDocVisitor(QTextStream &t, CodeOutputInterface &ci, QSharedPointer<Definition> ctx);

   //--------------------------------------
   // visitor functions for leaf nodes
   //--------------------------------------

   void visit(DocWord *);
   void visit(DocLinkedWord *);
   void visit(DocWhiteSpace *);
   void visit(DocSymbol *);
   void visit(DocURL *);
   void visit(DocLineBreak *);
   void visit(DocHorRuler *);
   void visit(DocStyleChange *);
   void visit(DocVerbatim *);
   void visit(DocAnchor *);
   void visit(DocInclude *);
   void visit(DocIncOperator *);
   void visit(DocFormula *);
   void visit(DocIndexEntry *);
   void visit(DocSimpleSectSep *);
   void visit(DocCite *);

   //--------------------------------------
   // visitor functions for compound nodes
   //--------------------------------------

   void visitPre(DocAutoList *);
   void visitPost(DocAutoList *);
   void visitPre(DocAutoListItem *);
   void visitPost(DocAutoListItem *);
   void visitPre(DocPara *) ;
   void visitPost(DocPara *);
   void visitPre(DocRoot *);
   void visitPost(DocRoot *);
   void visitPre(DocSimpleSect *);
   void visitPost(DocSimpleSect *);
   void visitPre(DocTitle *);
   void visitPost(DocTitle *);
   void visitPre(DocSimpleList *);
   void visitPost(DocSimpleList *);
   void visitPre(DocSimpleListItem *);
   void visitPost(DocSimpleListItem *);
   void visitPre(DocSection *);
   void visitPost(DocSection *);
   void visitPre(DocHtmlList *);
   void visitPost(DocHtmlList *) ;
   void visitPre(DocHtmlListItem *);
   void visitPost(DocHtmlListItem *);
   void visitPre(DocHtmlDescList *);
   void visitPost(DocHtmlDescList *);
   void visitPre(DocHtmlDescTitle *);
   void visitPost(DocHtmlDescTitle *);
   void visitPre(DocHtmlDescData *);
   void visitPost(DocHtmlDescData *);
   void visitPre(DocHtmlTable *);
   void visitPost(DocHtmlTable *);
   void visitPre(DocHtmlRow *);
   void visitPost(DocHtmlRow *) ;
   void visitPre(DocHtmlCell *);
   void visitPost(DocHtmlCell *);
   void visitPre(DocHtmlCaption *);
   void visitPost(DocHtmlCaption *);
   void visitPre(DocInternal *);
   void visitPost(DocInternal *);
   void visitPre(DocHRef *);
   void visitPost(DocHRef *);
   void visitPre(DocHtmlHeader *);
   void visitPost(DocHtmlHeader *);
   void visitPre(DocImage *);
   void visitPost(DocImage *);
   void visitPre(DocDotFile *);
   void visitPost(DocDotFile *);
   void visitPre(DocMscFile *);
   void visitPost(DocMscFile *);
   void visitPre(DocDiaFile *);
   void visitPost(DocDiaFile *);
   void visitPre(DocLink *);
   void visitPost(DocLink *);
   void visitPre(DocRef *);
   void visitPost(DocRef *);
   void visitPre(DocSecRefItem *);
   void visitPost(DocSecRefItem *);
   void visitPre(DocSecRefList *);
   void visitPost(DocSecRefList *);
   void visitPre(DocParamSect *);
   void visitPost(DocParamSect *);
   void visitPre(DocParamList *);
   void visitPost(DocParamList *);
   void visitPre(DocXRefItem *);
   void visitPost(DocXRefItem *);
   void visitPre(DocInternalRef *);
   void visitPost(DocInternalRef *);
   void visitPre(DocCopy *);
   void visitPost(DocCopy *);
   void visitPre(DocText *);
   void visitPost(DocText *);
   void visitPre(DocHtmlBlockQuote *);
   void visitPost(DocHtmlBlockQuote *);
   void visitPre(DocParBlock *);
   void visitPost(DocParBlock *);

 private:

   //--------------------------------------
   // helper functions
   //--------------------------------------
  
   void filter(const char *str);
   void filterQuotedCdataAttr(const char *str);

   void startLink(const QByteArray &ref, const QByteArray &file, const QByteArray &relPath, const QByteArray &anchor,
                  const QByteArray &tooltip = "");

   void endLink();

   void writeDotFile(const QString &fileName, const QString &relPath, const QByteArray &context);
   void writeMscFile(const QString &fileName, const QString &relPath, const QByteArray &context);
   void writeDiaFile(const QString &fileName, const QString &relPath, const QByteArray &context);
   void writePlantUMLFile(const QString &fileName, const QString &relPath, const QByteArray &context);

   void pushEnabled();
   void popEnabled();

   void forceEndParagraph(DocNode *n);
   void forceStartParagraph(DocNode *n);

   //--------------------------------------
   // state variables
   //--------------------------------------

   QTextStream &m_t;
   CodeOutputInterface &m_ci;
   bool m_insidePre;
   bool m_hide;
   QStack<bool> m_enabled;

   QSharedPointer<Definition> m_ctx;
   QByteArray m_langExt;
};

#endif
