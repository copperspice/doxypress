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

#ifndef DOCBOOKVISITOR_H
#define DOCBOOKVISITOR_H

#include <QStack>
#include <QTextStream>

#include <docparser.h>
#include <docvisitor.h>

class CodeGenerator;

/*! @brief Concrete visitor implementation for Docbook output. */
class DocbookDocVisitor : public DocVisitor
{
 public:
   DocbookDocVisitor(QTextStream &t, CodeGenerator &ci);
   ~DocbookDocVisitor();

   void visit(DocWord *) override;
   void visit(DocLinkedWord *) override;
   void visit(DocWhiteSpace *) override;
   void visit(DocSymbol *) override;
   void visit(DocEmoji *) override;
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
   void visitPre(DocPara *)  override;
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
   void visitPre(DocSection *) override;
   void visitPost(DocSection *) override;
   void visitPre(DocHtmlList *) override;
   void visitPost(DocHtmlList *)  override;
   void visitPre(DocHtmlListItem *) override;
   void visitPost(DocHtmlListItem *) override;

   //void visitPre(DocHtmlPre *) ;
   //void visitPost(DocHtmlPre *);

   void visitPre(DocHtmlDescList *) override;
   void visitPost(DocHtmlDescList *) override;
   void visitPre(DocHtmlDescTitle *) override;
   void visitPost(DocHtmlDescTitle *) override;
   void visitPre(DocHtmlDescData *) override;
   void visitPost(DocHtmlDescData *) override;
   void visitPre(DocHtmlTable *) override;
   void visitPost(DocHtmlTable *) override;
   void visitPre(DocHtmlRow *) override;
   void visitPost(DocHtmlRow *)  override;
   void visitPre(DocHtmlCell *) override;
   void visitPost(DocHtmlCell *) override;
   void visitPre(DocHtmlCaption *) override;
   void visitPost(DocHtmlCaption *) override;
   void visitPre(DocInternal *) override;
   void visitPost(DocInternal *) override;
   void visitPre(DocHRef *) override;
   void visitPost(DocHRef *) override;
   void visitPre(DocHtmlHeader *) override;
   void visitPost(DocHtmlHeader *) override;
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
   void visitPre(DocRef *) override;
   void visitPost(DocRef *) override;
   void visitPre(DocSecRefItem *) override;
   void visitPost(DocSecRefItem *) override;
   void visitPre(DocSecRefList *) override;
   void visitPost(DocSecRefList *) override;

   //void visitPre(DocLanguage *);
   //void visitPost(DocLanguage *);

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
   void visitPost(DocHtmlBlockQuote *) override;
   void visitPre(DocParBlock *) override;
   void visitPost(DocParBlock *) override;

 private:
   void filter(const QString &str);
   void startLink(const QString &file, const QString &anchor);
   void endLink();
   void pushEnabled();
   void popEnabled();
   void startMscFile(const QString &fileName, const QString &width, const QString &height,
            bool hasCaption, const QList<DocNode *> &children);
   void endMscFile(bool hasCaption);

   void startDiaFile(const QString &fileName, const QString &width, const QString &height,
            bool hasCaption, const QList<DocNode *> &children);
   void endDiaFile(bool hasCaption);

   void startDotFile(const QString &fileName, const QString &width, const QString &height,
            bool hasCaption, const QList<DocNode *> &children);
   void endDotFile(bool hasCaption);

   void writeDotFile(const QString &fileName, DocVerbatim *s);
   void writeMscFile(const QString &fileName, DocVerbatim *s);
   void writeDiaFile(const QString &fileName, DocVerbatim *s);
   void writePlantUMLFile(const QString &fileName, DocVerbatim *s);

   void visitPreStart(QTextStream  &t, const QList<DocNode  *> &children, bool hasCaption,
                  const QString &name, const QString &width, const QString &height, bool inlineImage = false);

   void visitPostEnd(QTextStream  &t, bool hasCaption, bool inlineImage = false);
   void visitCaption(const QList<DocNode *> &children);

   QTextStream &m_t;
   CodeGenerator &m_ci;

   bool m_insidePre;
   bool m_hide;

   int m_colCnt;

   QStack<bool> m_enabled;
   QStack<bool> m_bodySet;
   QString m_langExt;
};

#endif
