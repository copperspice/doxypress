/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim
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

#ifndef PRINTDOCVISITOR_H
#define PRINTDOCVISITOR_H

#include <docvisitor.h>
#include <htmlentity.h>

/*! Concrete visitor implementation for pretty printing */
class PrintDocVisitor : public DocVisitor
{
 public:
   PrintDocVisitor() : DocVisitor(DocVisitor_Other), m_indent(0),
      m_needsEnter(false), m_insidePre(false) {}

   void visit(DocWord *w)  override {
      indent_leaf();
      printf("%s", csPrintable(w->word()));
   }

   void visit(DocLinkedWord *w) override {
      indent_leaf();
      printf("%s", csPrintable(w->word()));
   }

   void visit(DocWhiteSpace *w) override {
      indent_leaf();

      if (m_insidePre) {
         printf("%s", csPrintable(w->chars()));
      } else {
         printf(" ");
      }
   }

   void visit(DocSymbol *s) override {
      indent_leaf();
      QString res = HtmlEntityMapper::instance()->utf8(s->symbol(), true);

      if (! res.isEmpty()) {
         printf("%s", csPrintable(res));
      } else {
         printf("Print: Unsupported HTML entity found: %s\n", csPrintable(
                  HtmlEntityMapper::instance()->html(s->symbol(), true)) );
      }
   }

   void visit(DocURL *u) override {
      indent_leaf();
      printf("%s", csPrintable(u->url()));
   }

   void visit(DocLineBreak *) override {
      indent_leaf();
      printf("<br/>");
   }

   void visit(DocHorRuler *) override {
      indent_leaf();
      printf("<hr/>");
   }

   void visit(DocStyleChange *s) override {
      indent_leaf();

      switch (s->style()) {
         case DocStyleChange::Bold:
            if (s->enable()) {
               printf("<bold>");
            } else {
               printf("</bold>");
            }
            break;

         case DocStyleChange::Italic:
            if (s->enable()) {
               printf("<italic>");
            } else {
               printf("</italic>");
            }
            break;

         case DocStyleChange::Code:
            if (s->enable()) {
               printf("<code>");
            } else {
               printf("</code>");
            }
            break;

         case DocStyleChange::Subscript:
            if (s->enable()) {
               printf("<sub>");
            } else {
               printf("</sub>");
            }
            break;

         case DocStyleChange::Superscript:
            if (s->enable()) {
               printf("<sup>");
            } else {
               printf("</sup>");
            }
            break;

         case DocStyleChange::Center:
            if (s->enable()) {
               printf("<center>");
            } else {
               printf("</center>");
            }
            break;
         case DocStyleChange::Small:
            if (s->enable()) {
               printf("<small>");
            } else {
               printf("</small>");
            }
            break;

         case DocStyleChange::Preformatted:
            if (s->enable()) {
               printf("<pre>");
            } else {
               printf("</pre>");
            }
            break;

         case DocStyleChange::Div:
            if (s->enable()) {
               printf("<div>");
            } else {
               printf("</div>");
            }
            break;

         case DocStyleChange::Span:
            if (s->enable()) {
               printf("<span>");
            } else {
               printf("</span>");
            }
            break;
      }
   }

   void visit(DocVerbatim *s) override {
      indent_leaf();

      switch (s->type()) {
         case DocVerbatim::Code:
            printf("<code>");
            break;

         case DocVerbatim::Verbatim:
            printf("<verbatim>");
            break;

         case DocVerbatim::HtmlOnly:
            printf("<htmlonly>");
            break;

         case DocVerbatim::RtfOnly:
            printf("<rtfonly>");
            break;

         case DocVerbatim::ManOnly:
            printf("<manonly>");
            break;

         case DocVerbatim::LatexOnly:
            printf("<latexonly>");
            break;

         case DocVerbatim::XmlOnly:
            printf("<xmlonly>");
            break;

         case DocVerbatim::DocbookOnly:
            printf("<docbookonly>");
            break;

         case DocVerbatim::Dot:
            printf("<dot>");
            break;

         case DocVerbatim::Msc:
            printf("<msc>");
            break;

         case DocVerbatim::PlantUML:
            printf("<plantuml>");
            break;
      }

      printf("%s", csPrintable(s->text()));

      switch (s->type()) {
         case DocVerbatim::Code:
            printf("</code>");
            break;

         case DocVerbatim::Verbatim:
            printf("</verbatim>");
            break;

         case DocVerbatim::HtmlOnly:
            printf("</htmlonly>");
            break;

         case DocVerbatim::RtfOnly:
            printf("</rtfonly>");
            break;

         case DocVerbatim::ManOnly:
            printf("</manonly>");
            break;

         case DocVerbatim::LatexOnly:
            printf("</latexonly>");
            break;

         case DocVerbatim::XmlOnly:
            printf("</xmlonly>");
            break;

         case DocVerbatim::DocbookOnly:
            printf("</docbookonly>");
            break;

         case DocVerbatim::Dot:
            printf("</dot>");
            break;

         case DocVerbatim::Msc:
            printf("</msc>");
            break;

         case DocVerbatim::PlantUML:
            printf("</plantuml>");
            break;
      }
   }

   void visit(DocAnchor *a) override {
      indent_leaf();
      printf("<anchor name=\"%s\"/>", csPrintable(a->anchor()));
   }

   void visit(DocInclude *inc) override {
      indent_leaf();
      printf("<include file=\"%s\" type=\"", csPrintable(inc->file()));

      switch (inc->type()) {
         case DocInclude::Include:
            printf("include");
            break;
         case DocInclude::IncWithLines:
            printf("incwithlines");
            break;
         case DocInclude::DontInclude:
            printf("dontinclude");
            break;
         case DocInclude::HtmlInclude:
            printf("htmlinclude");
            break;
         case DocInclude::LatexInclude:
            printf("latexinclude");
            break;
         case DocInclude::VerbInclude:
            printf("verbinclude");
            break;
         case DocInclude::Snippet:
            printf("snippet");
            break;
      }
      printf("\"/>");
   }

   void visit(DocIncOperator *op) override {
      indent_leaf();
      printf("<incoperator pattern=\"%s\" type=\"", csPrintable(op->pattern()));

      switch (op->type()) {
         case DocIncOperator::Line:
            printf("line");
            break;
         case DocIncOperator::Skip:
            printf("skip");
            break;
         case DocIncOperator::SkipLine:
            printf("skipline");
            break;
         case DocIncOperator::Until:
            printf("until");
            break;
      }
      printf("\"/>");
   }

   void visit(DocFormula *f) override {
      indent_leaf();
      printf("<formula name = %s text = %s/>", csPrintable(f->name()), csPrintable(f->text()));
   }

   void visit(DocIndexEntry *i) override {
      indent_leaf();
      printf("<indexentry>%s</indexentry\n", csPrintable(i->entry()));
   }

   void visit(DocSimpleSectSep *) override {
      indent_leaf();
      printf("<simplesectsep/>");
   }

   void visit(DocCite *cite) override {
      indent_leaf();
      printf("<cite ref=\"%s\" file=\"%s\" anchor=\"%s\" text=\"%s\"/>\n",
             csPrintable(cite->ref()), csPrintable(cite->file()), csPrintable(cite->anchor()), csPrintable(cite->text()) );
   }

   void visitPre(DocAutoList *l) override {
      indent_pre();
      if (l->isEnumList()) {
         printf("<ol>\n");
      } else {
         printf("<ul>\n");
      }
   }

   void visitPost(DocAutoList *l) override {
      indent_post();
      if (l->isEnumList()) {
         printf("</ol>\n");
      } else {
         printf("</ul>\n");
      }
   }

   void visitPre(DocAutoListItem *) override {
      indent_pre();
      printf("<li>\n");
   }

   void visitPost(DocAutoListItem *) override {
      indent_post();
      printf("</li>\n");
   }

   void visitPre(DocPara *node) override {
      // print the address of the paragraph
      indent_pre();
      printf("<para> %08x \n", node);
   }

   void visitPost(DocPara *) override {
      indent_post();
      printf("</para>\n");
   }

   void visitPre(DocRoot *) override {
      indent_pre();
      printf("<root>\n");
   }

   void visitPost(DocRoot *) override {
      indent_post();
      printf("</root>\n");
   }

   void visitPre(DocSimpleSect *s) override {
      indent_pre();
      printf("<simplesect type=");

      switch (s->type()) {
         case DocSimpleSect::See:
            printf("see");
            break;

        case DocSimpleSect::Return:
            printf("return");
            break;

         case DocSimpleSect::Author:
            printf("author");
            break;

         case DocSimpleSect::Authors:
            printf("authors");
            break;

         case DocSimpleSect::Version:
            printf("version");
            break;

         case DocSimpleSect::Since:
            printf("since");
            break;

         case DocSimpleSect::Date:
            printf("date");
            break;

         case DocSimpleSect::Note:
            printf("note");
            break;

         case DocSimpleSect::Warning:
            printf("warning");
            break;

         case DocSimpleSect::Pre:
            printf("pre");
            break;

         case DocSimpleSect::Post:
            printf("post");
            break;

         case DocSimpleSect::Copyright:
            printf("copyright");
            break;

         case DocSimpleSect::Invar:
            printf("invar");
            break;

         case DocSimpleSect::Remark:
            printf("remark");
            break;

         case DocSimpleSect::Attention:
            printf("attention");
            break;

         case DocSimpleSect::User:
            printf("user");
            break;

         case DocSimpleSect::Rcs:
            printf("rcs");
            break;

         case DocSimpleSect::Unknown:
            printf("unknown");
            break;
      }
      printf(">\n");
   }

   void visitPost(DocSimpleSect *) override {
      indent_post();
      printf("</simplesect>\n");
   }

   void visitPre(DocTitle *) override {
      indent_pre();
      printf("<title>\n");
   }

   void visitPost(DocTitle *) override {
      indent_post();
      printf("</title>\n");
   }

   void visitPre(DocSimpleList *) override {
      indent_pre();
      printf("<ul>\n");
   }

   void visitPost(DocSimpleList *) override {
      indent_post();
      printf("</ul>\n");
   }

   void visitPre(DocSimpleListItem *) override {
      indent_pre();
      printf("<li>\n");
   }

   void visitPost(DocSimpleListItem *) override {
      indent_post();
      printf("</li>\n");
   }

   void visitPre(DocSection *s) override {
      indent_pre();
      printf("<sect%d>\n", s->level());
   }

   void visitPost(DocSection *s) override {
      indent_post();
      printf("</sect%d>\n", s->level());
   }

   void visitPre(DocHtmlList *s) override {
      indent_pre();
      if (s->type() == DocHtmlList::Ordered) {
         printf("<ol>\n");
      } else {
         printf("<ul>\n");
      }
   }

   void visitPost(DocHtmlList *s) override {
      indent_post();
      if (s->type() == DocHtmlList::Ordered) {
         printf("</ol>\n");
      } else {
         printf("</ul>\n");
      }
   }

   void visitPre(DocHtmlListItem *) override {
      indent_pre();
      printf("<li>\n");
   }

   void visitPost(DocHtmlListItem *) override {
      indent_post();
      printf("</li>\n");
   }

   //void visitPre(DocHtmlPre *)
   //{
   //  indent_pre();
   //  printf("<pre>\n");
   //  m_insidePre=true;
   //}
   //void visitPost(DocHtmlPre *)
   //{
   //  m_insidePre=false;
   //  indent_post();
   //  printf("</pre>\n");
   //}

   void visitPre(DocHtmlDescList *) override {
      indent_pre();
      printf("<dl>\n");
   }

   void visitPost(DocHtmlDescList *) override {
      indent_post();
      printf("</dl>\n");
   }

   void visitPre(DocHtmlDescTitle *) override {
      indent_pre();
      printf("<dt>\n");
   }

   void visitPost(DocHtmlDescTitle *) override {
      indent_post();
      printf("</dt>\n");
   }

   void visitPre(DocHtmlDescData *) override {
      indent_pre();
      printf("<dd>\n");
   }

   void visitPost(DocHtmlDescData *) override {
      indent_post();
      printf("</dd>\n");
   }

   void visitPre(DocHtmlTable *t) override {
      indent_pre();

      printf("<table rows=\"%d\" cols=\"%d\">\n", t->numRows(), t->numColumns());
   }

   void visitPost(DocHtmlTable *) override {
      indent_post();
      printf("</table>\n");
   }

   void visitPre(DocHtmlRow *) override {
      indent_pre();
      printf("<tr>\n");
   }

   void visitPost(DocHtmlRow *) override {
      indent_post();
      printf("</tr>\n");
   }

   void visitPre(DocHtmlCell *c) override {
      indent_pre();
      printf("<t%c>\n", c->isHeading() ? 'h' : 'd');
   }

   void visitPost(DocHtmlCell *c) override {
      indent_post();
      printf("</t%c>\n", c->isHeading() ? 'h' : 'd');
   }

   void visitPre(DocHtmlCaption *) override {
      indent_pre();
      printf("<caption>\n");
   }

   void visitPost(DocHtmlCaption *) override {
      indent_post();
      printf("</caption>\n");
   }

   void visitPre(DocInternal *) override {
      indent_pre();
      printf("<internal>\n");
   }

   void visitPost(DocInternal *) override {
      indent_post();
      printf("</internal>\n");
   }

   void visitPre(DocHRef *href) override {
      indent_pre();
      printf("<a url=\"%s\">\n", csPrintable(href->url()));
   }

   void visitPost(DocHRef *) override {
      indent_post();
      printf("</a>\n");
   }

   void visitPre(DocHtmlHeader *header) override {
      indent_pre();
      printf("<h%d>\n", header->level());
   }

   void visitPost(DocHtmlHeader *header) override {
      indent_post();
      printf("</h%d>\n", header->level());
   }

   void visitPre(DocImage *img) override {
      indent_pre();
      printf("<image src=\"%s\" type=\"", csPrintable(img->name()));

      switch (img->type()) {
         case DocImage::Html:
            printf("html");
            break;
         case DocImage::Latex:
            printf("latex");
            break;
         case DocImage::Rtf:
            printf("rtf");
            break;
         case DocImage::DocBook:
            printf("docbook");
            break;
      }

      printf("\" width=%s height=%s>\n", csPrintable(img->width()), csPrintable(img->height()));
   }

   void visitPost(DocImage *) override {
      indent_post();
      printf("</image>\n");
   }

   void visitPre(DocDotFile *df) override {
      indent_pre();
      printf("<dotfile src=\"%s\">\n", csPrintable(df->name()));
   }

   void visitPost(DocDotFile *) override {
      indent_post();
      printf("</dotfile>\n");
   }

   void visitPre(DocMscFile *df) override {
      indent_pre();
      printf("<mscfile src=\"%s\">\n", csPrintable(df->name()));
   }

   void visitPost(DocMscFile *) override {
      indent_post();
      printf("</mscfile>\n");
   }

   void visitPre(DocDiaFile *df) override {
      indent_pre();
      printf("<diafile src=\"%s\">\n", csPrintable(df->name()));
   }

   void visitPost(DocDiaFile *) override {
      indent_post();
      printf("</diafile>\n");
   }

   void visitPre(DocLink *lnk) override {
      indent_pre();
      printf("<link ref=\"%s\" file=\"%s\" anchor=\"%s\">\n",
             csPrintable(lnk->ref()), csPrintable(lnk->file()), csPrintable(lnk->anchor()));
   }

   void visitPost(DocLink *) override {
      indent_post();
      printf("</link>\n");
   }    
                                   
   void visitPre(DocRef *ref) override {
      indent_pre();
      printf("<ref ref=\"%s\" file=\"%s\" "
             "anchor=\"%s\" targetTitle=\"%s\""
             " hasLinkText=\"%s\" refToAnchor=\"%s\" refToSection=\"%s\" refToTable=\"%s\">\n",
             csPrintable(ref->ref()), csPrintable(ref->file()), csPrintable(ref->anchor()),
             csPrintable(ref->targetTitle()), ref->hasLinkText() ? "yes" : "no",
             ref->refToAnchor() ? "yes" : "no", ref->refToSection() ? "yes" : "no",
             ref->refToTable()?"yes":"no");
   }

   void visitPost(DocRef *) override {
      indent_post();
      printf("</ref>\n");
   }

   void visitPre(DocSecRefItem *ref) override {
      indent_pre();
      printf("<secrefitem target=\"%s\">\n", csPrintable(ref->target()));
   }

   void visitPost(DocSecRefItem *) override {
      indent_post();
      printf("</secrefitem>\n");
   }

   void visitPre(DocSecRefList *) override {
      indent_pre();
      printf("<secreflist>\n");
   }

   void visitPost(DocSecRefList *) override {
      indent_post();
      printf("</secreflist>\n");
   }

   void visitPre(DocParamList *pl) override {
      indent_pre();

      DocNode *param;

      printf("<parameters>");

      for (auto param : pl->parameters()) {
         printf("<param>");

         if (param->kind() == DocNode::Kind_Word) {
            visit((DocWord *)param);
         } else if (param->kind() == DocNode::Kind_LinkedWord) {
            visit((DocLinkedWord *)param);
         }

         printf("</param>");
      }

      printf("\n");
   }

   void visitPost(DocParamList *) override {
      indent_post();
      printf("</parameters>\n");
   }

   void visitPre(DocParamSect *ps) override {
      indent_pre();
      printf("<paramsect type=");

      switch (ps->type()) {
         case DocParamSect::Param:
            printf("param");
            break;
         case DocParamSect::RetVal:
            printf("retval");
            break;
         case DocParamSect::Exception:
            printf("exception");
            break;
         case DocParamSect::TemplateParam:
            printf("templateparam");
            break;
         case DocParamSect::Unknown:
            printf("unknown");
            break;
      }
      printf(">\n");
   }

   void visitPost(DocParamSect *) override {
      indent_post();
      printf("</paramsect>\n");
   }

   void visitPre(DocXRefItem *x) override {
      indent_pre();
      printf("<xrefitem file=\"%s\" anchor=\"%s\" title=\"%s\"/>\n",
             csPrintable(x->file()), csPrintable(x->anchor()), csPrintable(x->title()));
   }

   void visitPost(DocXRefItem *) override {
      indent_post();
      printf("<xrefitem/>\n");
   }

   void visitPre(DocInternalRef *r) override {
      indent_pre();
      printf("<internalref file=%s anchor=%s>\n", csPrintable(r->file()), csPrintable(r->anchor()));
   }

   void visitPost(DocInternalRef *) override {
      indent_post();
      printf("</internalref>\n");
   }
   void visitPre(DocCopy *c) override {
      indent_pre();
      printf("<copy link=\"%s\">\n", csPrintable(c->link()));
   }

   void visitPost(DocCopy *) override {
      indent_post();
      printf("</copy>\n");
   }

   void visitPre(DocText *) override {
      indent_pre();
      printf("<text>\n");
   }

   void visitPost(DocText *) override {
      indent_post();
      printf("</text>\n");
   }

   void visitPre(DocHtmlBlockQuote *) override {
      indent_pre();
      printf("<blockquote>\n");
   }

   void visitPost(DocHtmlBlockQuote *) override {
      indent_post();
      printf("</blockquote>\n");
   }

   void visitPre(DocParBlock *) override {
      indent_pre();
      printf("<parblock>\n");
   }

   void visitPost(DocParBlock *) override {
      indent_post();
      printf("</parblock>\n");
   }

 private:
   void indent() {
      if (m_needsEnter) {
         printf("\n");
      }

      for (int i = 0; i < m_indent; i++) {
         printf(" ");
      }

      m_needsEnter = false;
   }

   void indent_leaf() {
      if (!m_needsEnter) {
         indent();
      }

      m_needsEnter = true;
   }

   void indent_pre() {
      indent();
      m_indent++;
   }

   void indent_post() {
      m_indent--;
      indent();
   }


   int m_indent;
   bool m_needsEnter;
   bool m_insidePre;
};

#endif
