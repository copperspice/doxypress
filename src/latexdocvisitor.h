/************************************************************************
*
* Copyright (c) 2014-2023 Barbara Geller & Ansel Sermersheim
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

#ifndef LATEXDOCVISITOR_H
#define LATEXDOCVISITOR_H

#include <QList>
#include <QStack>
#include <QTextStream>

#include <docvisitor.h>

class LatexCodeGenerator;

class LatexDocVisitor : public DocVisitor
{
 public:
   LatexDocVisitor(QTextStream &t, LatexCodeGenerator &ci, const QString &langExt, bool insideTabbing);

   // visitor functions for leaf nodes

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
   void visitPre(DocLink *lnk) override;
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
   void visitPost(DocHtmlBlockQuote *) override;
   void visitPre(DocParBlock *) override;
   void visitPost(DocParBlock *) override;

 private:

   struct ActiveRowSpan {
      ActiveRowSpan(DocHtmlCell *c, int rs, int cs, int col)
         : cell(c), rowSpan(rs), colSpan(cs), column(col) {}

      DocHtmlCell *cell;
      int rowSpan;
      int colSpan;
      int column;
   };

   using RowSpanList = QList<ActiveRowSpan>;

   void filter(const QString &str);
   void startLink(const QString &ref, const QString &file, const QString &anchor, bool refToTable = false);
   void endLink(const QString &ref, const QString &file, const QString &anchor, bool refToTable = false);
   QString escapeMakeIndexChars(const QString &s);

   void startDotFile(const QString &fileName, const QString &width, const QString &height, bool hasCaption);
   void endDotFile(bool hasCaption);

   void startMscFile(const QString &fileName, const QString &width, const QString &height, bool hasCaption);
   void endMscFile(bool hasCaption);
   void writeMscFile(const QString &fileName, DocVerbatim *s);

   void startDiaFile(const QString &fileName, const QString &width, const QString &height, bool hasCaption);
   void endDiaFile(bool hasCaption);
   void writeDiaFile(const QString &fileName, DocVerbatim *s);

   void writePlantUMLFile(const QString &fileName, DocVerbatim *s);

   void pushEnabled();
   void popEnabled();

   QTextStream &m_t;
   LatexCodeGenerator &m_ci;

   bool m_insidePre;
   bool m_insideItem;
   bool m_hide;
   bool m_hideCaption;
   bool m_insideTabbing;

   QStack<bool> m_enabled;
   QString m_langExt;

   struct TableState {
      TableState() : numCols(0), currentColumn(0), inRowSpan(false), inColSpan(false), firstRow(false)
      { }

      RowSpanList rowSpans;

      int  numCols;
      int  currentColumn;
      bool inRowSpan;
      bool inColSpan;
      bool firstRow;
   };

   QStack<TableState> m_tableStateStack;    // needed for nested tables
   RowSpanList m_emptyRowSpanList;

   void pushTableState()
   {
      m_tableStateStack.push( TableState{} );
   }

   void popTableState()
   {
      m_tableStateStack.pop();
   }

   int currentColumn() const
   {
      return ! m_tableStateStack.isEmpty() ? m_tableStateStack.top().currentColumn : 0;
   }

   void setCurrentColumn(int col)
   {
      if (! m_tableStateStack.isEmpty()) m_tableStateStack.top().currentColumn = col;
   }

   int numCols() const
   {
      return ! m_tableStateStack.isEmpty() ? m_tableStateStack.top().numCols : 0;
   }

   void setNumCols(int num)
   {
      if (! m_tableStateStack.isEmpty()) {
         m_tableStateStack.top().numCols = num;
      }
   }

   bool inRowSpan() const
   {
      return ! m_tableStateStack.isEmpty() ? m_tableStateStack.top().inRowSpan : false;
   }

   void setInRowSpan(bool b)
   {
      if (! m_tableStateStack.isEmpty()) {
         m_tableStateStack.top().inRowSpan = b;
      }
   }

   bool inColSpan() const
   {
      return ! m_tableStateStack.isEmpty() ? m_tableStateStack.top().inColSpan : false;
   }

   void setInColSpan(bool b)
   {
      if (! m_tableStateStack.isEmpty()) {
         m_tableStateStack.top().inColSpan = b;
      }
   }

   bool firstRow() const
   {
      return ! m_tableStateStack.isEmpty() ? m_tableStateStack.top().firstRow : false;
   }

   void setFirstRow(bool b)
   {
      if (! m_tableStateStack.isEmpty()) {
         m_tableStateStack.top().firstRow = b;
      }
   }

   const RowSpanList &rowSpans()
   {
      return ! m_tableStateStack.isEmpty() ? m_tableStateStack.top().rowSpans : m_emptyRowSpanList;
   }

   void addRowSpan(const ActiveRowSpan &span)
   {
      if (! m_tableStateStack.isEmpty()) {
         m_tableStateStack.top().rowSpans.append(span);
      }
   }

   bool insideTable() const
   {
      return ! m_tableStateStack.isEmpty();
   }
};

#endif
