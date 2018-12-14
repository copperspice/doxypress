/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim
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

#include <QDir>
#include <QStack>
#include <QHash>
#include <QFile>
#include <QTextStream>

#include <stdlib.h>

#include <perlmodgen.h>

#include <arguments.h>
#include <config.h>
#include <docparser.h>
#include <doxy_globals.h>
#include <htmlentity.h>
#include <message.h>
#include <membergroup.h>
#include <util.h>

#define PERLOUTPUT_MAX_INDENTATION 40

static QString pathDoxyfile;
static QString pathDoxyExec;

class PerlModOutputStream
{
 public:

   QString m_s;
   QTextStream *m_t;

   PerlModOutputStream(QTextStream *t = 0) : m_t(t) { }

   void add(char c);
   void add(QChar c);
   void add(const QString &s);
};

void PerlModOutputStream::add(char c)
{
   if (m_t != 0) {
      (*m_t) << c;

   } else {
      m_s += c;
   }
}

void PerlModOutputStream::add(QChar c)
{
   if (m_t != 0) {
      (*m_t) << c;

   } else {
      m_s += c;
   }
}

void PerlModOutputStream::add(const QString &s)
{
   if (m_t != 0) {
      (*m_t) << s;

   } else {
      m_s += s;
   }
}

class PerlModOutput
{
 public:

   bool m_pretty;

   inline PerlModOutput(bool pretty)
      : m_pretty(pretty), m_stream(0), m_indentation(false), m_blockstart(true) {
      m_spaces[0] = 0;
   }

   virtual ~PerlModOutput() { }

   inline void setPerlModOutputStream(PerlModOutputStream *os) {
      m_stream = os;
   }

   inline PerlModOutput &openSave() {
      internal_openSave();
      return *this;
   }

   inline PerlModOutput &closeSave(QString &str) {
      str = internal_closeSave();
      return *this;
   }

   inline PerlModOutput &continueBlock() {
      if (m_blockstart) {
         m_blockstart = false;
      } else {
         m_stream->add(',');
      }
      indent();
      return *this;
   }

   inline PerlModOutput &add(char c) {
      m_stream->add(c);
      return *this;
   }

   inline PerlModOutput &add(QChar c) {
      m_stream->add(c);
      return *this;
   }

   inline PerlModOutput &add(const QString &s) {
      m_stream->add(s);
      return *this;
   }

   inline PerlModOutput &add(int n) {
      m_stream->add(n);
      return *this;
   }

   inline PerlModOutput &add(unsigned int n) {
      m_stream->add(n);
      return *this;
   }

   PerlModOutput &addQuoted(const QString &s) {
      iaddQuoted(s);
      return *this;
   }

   inline PerlModOutput &indent() {
      if (m_pretty) {
         m_stream->add('\n');
         m_stream->add(m_spaces);
      }
      return *this;
   }

   inline PerlModOutput &open(QChar c, const QString &s = QString()) {
      iopen(c, s);
      return *this;
   }

   inline PerlModOutput &close(char c = 0) {
      iclose(c);
      return *this;
   }

   inline PerlModOutput &addField(const QString &s) {
      iaddField(s);
      return *this;
   }

   inline PerlModOutput &addFieldQuotedChar(const QString &field, QChar content) {
      iaddFieldQuotedChar(field, content);
      return *this;
   }

   inline PerlModOutput &addFieldQuotedString(const QString &field, const QString &content) {
      iaddFieldQuotedString(field, content);
      return *this;
   }

   inline PerlModOutput &addFieldBoolean(const QString &field, bool content) {
      return addFieldQuotedString(field, content ? QString("yes") : QString("no"));
   }

   inline PerlModOutput &openList(const QString &s = QString()) {
      open('[', s);
      return *this;
   }

   inline PerlModOutput &closeList() {
      close(']');
      return *this;
   }

   inline PerlModOutput &openHash(const QString &s = QString() ) {
      open('{', s);
      return *this;
   }
   inline PerlModOutput &closeHash() {
      close('}');
      return *this;
   }

 protected:
   void internal_openSave();
   QString internal_closeSave();

   void incIndent();
   void decIndent();

   void iaddQuoted(const QString &);
   void iaddFieldQuotedChar(const QString &, QChar);
   void iaddFieldQuotedString(const QString &, const QString &);
   void iaddField(const QString &);

   void iopen(QChar, const QString &);
   void iclose(QChar);

 private:
   PerlModOutputStream *m_stream;
   int m_indentation;
   bool m_blockstart;

   QStack<PerlModOutputStream *> m_saved;
   char m_spaces[PERLOUTPUT_MAX_INDENTATION * 2 + 2];
};

void PerlModOutput::internal_openSave()
{
   m_saved.push(m_stream);
   m_stream = new PerlModOutputStream();
}

QString PerlModOutput::internal_closeSave()
{
   QString retval = m_stream->m_s;

   delete m_stream;
   m_stream = m_saved.pop();

   return retval;
}

void PerlModOutput::incIndent()
{
   if (m_indentation < PERLOUTPUT_MAX_INDENTATION) {
      char *s = &m_spaces[m_indentation * 2];
      *s++ = ' ';
      *s++ = ' ';
      *s = 0;
   }

   m_indentation++;
}

void PerlModOutput::decIndent()
{
   m_indentation--;
   if (m_indentation < PERLOUTPUT_MAX_INDENTATION) {
      m_spaces[m_indentation * 2] = 0;
   }
}

void PerlModOutput::iaddQuoted(const QString &str)
{
   for (auto c : str ) {

      if ((c == '\'') || (c == '\\')) {
         m_stream->add('\\');
      }

      m_stream->add(c);
   }
}

void PerlModOutput::iaddField(const QString &str)
{
   continueBlock();
   m_stream->add(str);
   m_stream->add(m_pretty ? QString(" => ") : QString("=>"));
}

void PerlModOutput::iaddFieldQuotedChar(const QString &field, QChar content)
{
   iaddField(field);
   m_stream->add('\'');

   if ((content == '\'') || (content == '\\')) {
      m_stream->add('\\');
   }

   m_stream->add(content);
   m_stream->add('\'');
}

void PerlModOutput::iaddFieldQuotedString(const QString &field, const QString &content)
{
   if (content.isEmpty()) {
      return;
   }

   iaddField(field);
   m_stream->add('\'');
   iaddQuoted(content);
   m_stream->add('\'');
}

void PerlModOutput::iopen(QChar c, const QString &s)
{
   if (s != 0) {
      iaddField(s);
   } else {
      continueBlock();
   }

   m_stream->add(c);
   incIndent();
   m_blockstart = true;
}

void PerlModOutput::iclose(QChar c)
{
   decIndent();
   indent();

   if (c != 0) {
      m_stream->add(c);
   }

   m_blockstart = false;
}

/*! @brief Concrete visitor implementation for PerlMod output. */
class PerlModDocVisitor : public DocVisitor
{
 public:
   PerlModDocVisitor(PerlModOutput &);
   virtual ~PerlModDocVisitor() { }

   void finish();

   //--------------------------------------
   // visitor functions for leaf nodes
   //--------------------------------------

   void visit(DocWord *)override;
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

   //--------------------------------------
   // visitor functions for compound nodes
   //--------------------------------------

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
   //void visitPre(DocHtmlPre *);
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
   void visitPost(DocHtmlBlockQuote *)   override;
   void visitPre(DocParBlock *) override;
   void visitPost(DocParBlock *) override;

 private:
   void addLink(const QString &ref, const QString &file, const QString &anchor);

   void enterText();
   void leaveText();

   void openItem(const QString &);
   void closeItem();
   void singleItem(const QString &);
   void openSubBlock(const QString & = QString());
   void closeSubBlock();
   void openOther();
   void closeOther();

   PerlModOutput &m_output;
   bool m_textmode;
   bool m_textblockstart;
   QString m_other;
};

PerlModDocVisitor::PerlModDocVisitor(PerlModOutput &output)
   : DocVisitor(DocVisitor_Other), m_output(output), m_textmode(false), m_textblockstart(false)
{
   m_output.openList("doc");
}

void PerlModDocVisitor::finish()
{
   leaveText();
   m_output.closeList()
   .add(m_other);
}

void PerlModDocVisitor::addLink(const QString &, const QString &file, const QString &anchor)
{
   QString link = file;

   if (!anchor.isEmpty()) {
      (link += "_1") += anchor;
   }

   m_output.addFieldQuotedString("link", link);
}

void PerlModDocVisitor::openItem(const QString &name)
{
   leaveText();
   m_output.openHash().addFieldQuotedString("type", name);
}

void PerlModDocVisitor::closeItem()
{
   leaveText();
   m_output.closeHash();
}

void PerlModDocVisitor::enterText()
{
   if (m_textmode) {
      return;
   }
   openItem("text");
   m_output.addField("content").add('\'');
   m_textmode = true;
}

void PerlModDocVisitor::leaveText()
{
   if (!m_textmode) {
      return;
   }
   m_textmode = false;
   m_output
   .add('\'')
   .closeHash();
}

void PerlModDocVisitor::singleItem(const QString &name)
{
   openItem(name);
   closeItem();
}

void PerlModDocVisitor::openSubBlock(const QString &s)
{
   leaveText();
   m_output.openList(s);
   m_textblockstart = true;
}

void PerlModDocVisitor::closeSubBlock()
{
   leaveText();
   m_output.closeList();
}

void PerlModDocVisitor::openOther()
{
   // Using a secondary text stream will corrupt the perl file. Instead of
   // printing doc => [ data => [] ], it will print doc => [] data => [].
   /*
   leaveText();
   m_output.openSave();
   */
}

void PerlModDocVisitor::closeOther()
{
   // Using a secondary text stream will corrupt the perl file. Instead of
   // printing doc => [ data => [] ], it will print doc => [] data => [].
   /*
   QString other;
   leaveText();
   m_output.closeSave(other);
   m_other += other;
   */
}

void PerlModDocVisitor::visit(DocWord *w)
{
   enterText();
   m_output.addQuoted(w->word());
}

void PerlModDocVisitor::visit(DocLinkedWord *w)
{
   openItem("url");
   addLink(w->ref(), w->file(), w->anchor());
   m_output.addFieldQuotedString("content", w->word());
   closeItem();
}

void PerlModDocVisitor::visit(DocWhiteSpace *)
{
   enterText();
   m_output.add(' ');
}

void PerlModDocVisitor::visit(DocSymbol *sy)
{
   const DocSymbol::PerlSymb *res = HtmlEntityMapper::instance()->perl(sy->symbol());
   QString accent;

   if (! res->symb.isEmpty()) {
      switch (res->type) {
         case DocSymbol::Perl_string:
            enterText();
            m_output.add(res->symb);
            break;

         case DocSymbol::Perl_char:
            enterText();
            m_output.add(res->symb[0]);
            break;

         case DocSymbol::Perl_symbol:
            leaveText();
            openItem("symbol");
            m_output.addFieldQuotedString("symbol", res->symb);
            closeItem();
            break;

         default:
            switch (res->type) {
               case DocSymbol::Perl_umlaut:
                  accent = "umlaut";
                  break;

               case DocSymbol::Perl_acute:
                  accent = "acute";
                  break;

               case DocSymbol::Perl_grave:
                  accent = "grave";
                  break;

               case DocSymbol::Perl_circ:
                  accent = "circ";
                  break;

               case DocSymbol::Perl_slash:
                  accent = "slash";
                  break;

               case DocSymbol::Perl_tilde:
                  accent = "tilde";
                  break;

               case DocSymbol::Perl_cedilla:
                  accent = "cedilla";
                  break;

               case DocSymbol::Perl_ring:
                  accent = "ring";
                  break;

               default:
                  break;
            }

            leaveText();

            if (! accent.isEmpty()) {
               openItem("accent");
               m_output.addFieldQuotedString("accent", accent).addFieldQuotedChar("letter", res->symb[0]);
               closeItem();
            }
            break;
      }

   } else {
      err("Perl, Unsupported HTML entity found: %s\n", csPrintable(HtmlEntityMapper::instance()->html(sy->symbol(), true)) );
   }
}

void PerlModDocVisitor::visit(DocURL *u)
{
   openItem("url");
   m_output.addFieldQuotedString("content", u->url());
   closeItem();
}

void PerlModDocVisitor::visit(DocLineBreak *)
{
   singleItem("linebreak");
}
void PerlModDocVisitor::visit(DocHorRuler *)
{
   singleItem("hruler");
}

void PerlModDocVisitor::visit(DocStyleChange *s)
{
   QString style;

   switch (s->style()) {
      case DocStyleChange::Bold:
         style = "bold";
         break;
      case DocStyleChange::Italic:
         style = "italic";
         break;
      case DocStyleChange::Code:
         style = "code";
         break;
      case DocStyleChange::Subscript:
         style = "subscript";
         break;
      case DocStyleChange::Superscript:
         style = "superscript";
         break;
      case DocStyleChange::Center:
         style = "center";
         break;
      case DocStyleChange::Small:
         style = "small";
         break;
      case DocStyleChange::Preformatted:
         style = "preformatted";
         break;
      case DocStyleChange::Div:
         style = "div";
         break;
      case DocStyleChange::Span:
         style = "span";
         break;

   }

   openItem("style");
   m_output.addFieldQuotedString("style", style).addFieldBoolean("enable", s->enable());
   closeItem();
}

void PerlModDocVisitor::visit(DocVerbatim *s)
{
   QString type;

   switch (s->type()) {
      case DocVerbatim::Code:

#if 0
         m_output.add("<programlisting>");
         parseCode(m_ci, s->context(), s->text(), false, 0);
         m_output.add("</programlisting>");
         return;
#endif

      case DocVerbatim::Verbatim:
         type = "preformatted";
         break;

      case DocVerbatim::HtmlOnly:
         type = "htmlonly";
         break;

      case DocVerbatim::RtfOnly:
         type = "rtfonly";
         break;

      case DocVerbatim::ManOnly:
         type = "manonly";
         break;

      case DocVerbatim::LatexOnly:
         type = "latexonly";
         break;

      case DocVerbatim::XmlOnly:
         type = "xmlonly";
         break;

      case DocVerbatim::DocbookOnly:
         type = "docbookonly";
         break;

      case DocVerbatim::Dot:
         type = "dot";
         break;

      case DocVerbatim::Msc:
         type = "msc";
         break;

      case DocVerbatim::PlantUML:
         type = "plantuml";
         break;
   }

   openItem(type);

   if (s->hasCaption()) {
      openSubBlock("caption");

      for (auto item : s->children()) {
         item->accept(this);
      }

      closeSubBlock();
   }

   m_output.addFieldQuotedString("content", s->text());
   closeItem();
}

void PerlModDocVisitor::visit(DocAnchor *anc)
{
   QString anchor = anc->file() + "_1" + anc->anchor();

   openItem("anchor");
   m_output.addFieldQuotedString("id", anchor);
   closeItem();
}

void PerlModDocVisitor::visit(DocInclude *inc)
{
   QString type;

   switch (inc->type()) {
      case DocInclude::IncWithLines:
#if 0
      {
         m_t << "<div class=\"fragment\"><pre>";
         QFileInfo cfi( inc->file() );
         FileDef fd( cfi.path(), cfi.fileName() );
         parseCode(m_ci, inc->context(), inc->text().latin1(), inc->isExample(), inc->exampleFile(), &fd);
         m_t << "</pre></div>";
      }
      break;
#endif
      return;

      case DocInclude::Include:
#if 0
         m_output.add("<programlisting>");
         parseCode(m_ci, inc->context(), inc->text(), false, 0);
         m_output.add("</programlisting>");
#endif
         return;
      case DocInclude::DontInclude:
         return;
      case DocInclude::HtmlInclude:
         type = "htmlonly";
         break;
      case DocInclude::LatexInclude:
         type = "latexonly";
         break;
      case DocInclude::VerbInclude:
         type = "preformatted";
         break;
      case DocInclude::Snippet:
         return;
   }

   openItem(type);
   m_output.addFieldQuotedString("content", inc->text());
   closeItem();
}

void PerlModDocVisitor::visit(DocIncOperator *)
{
#if 0
   if (op->isFirst()) {
      m_output.add("<programlisting>");
   }

   if (op->type() != DocIncOperator::Skip) {
      parseCode(m_ci, op->context(), op->text(), false, 0);
   }

   if (op->isLast()) {
      m_output.add("</programlisting>");
   } else {
      m_output.add('\n');
   }
#endif
}

void PerlModDocVisitor::visit(DocFormula *f)
{
   openItem("formula");

   QString id;
   id += f->id();
   m_output.addFieldQuotedString("id", id).addFieldQuotedString("content", f->text());
   closeItem();
}

void PerlModDocVisitor::visit(DocIndexEntry *)
{
#if 0
   m_output.add("<indexentry>"
                "<primaryie>");
   m_output.addQuoted(ie->entry());
   m_output.add("</primaryie>"
                "<secondaryie></secondaryie>"
                "</indexentry>");
#endif
}

void PerlModDocVisitor::visit(DocSimpleSectSep *)
{
}

void PerlModDocVisitor::visit(DocCite *cite)
{
   openItem("cite");
   m_output.addFieldQuotedString("text", cite->text());
   closeItem();
}


//--------------------------------------
// visitor functions for compound nodes
//--------------------------------------

void PerlModDocVisitor::visitPre(DocAutoList *l)
{
   openItem("list");
   m_output.addFieldQuotedString("style", l->isEnumList() ? QString("ordered") : QString("itemized"));
   openSubBlock("content");
}

void PerlModDocVisitor::visitPost(DocAutoList *)
{
   closeSubBlock();
   closeItem();
}

void PerlModDocVisitor::visitPre(DocAutoListItem *)
{
   openSubBlock();
}

void PerlModDocVisitor::visitPost(DocAutoListItem *)
{
   closeSubBlock();
}

void PerlModDocVisitor::visitPre(DocPara *)
{
   if (m_textblockstart) {
      m_textblockstart = false;
   } else {
      singleItem("parbreak");
   }
}

void PerlModDocVisitor::visitPost(DocPara *)
{
   /*
   closeSubBlock();
   closeItem();
   */
}

void PerlModDocVisitor::visitPre(DocRoot *)
{
}

void PerlModDocVisitor::visitPost(DocRoot *)
{
}

void PerlModDocVisitor::visitPre(DocSimpleSect *s)
{
   QString type;

   switch (s->type()) {
      case DocSimpleSect::See:
         type = "see";
         break;
      case DocSimpleSect::Return:
         type = "return";
         break;
      case DocSimpleSect::Author:
         type = "author";
         break;
      case DocSimpleSect::Authors:
         type = "authors";
         break;
      case DocSimpleSect::Version:
         type = "version";
         break;
      case DocSimpleSect::Since:
         type = "since";
         break;
      case DocSimpleSect::Date:
         type = "date";
         break;
      case DocSimpleSect::Note:
         type = "note";
         break;
      case DocSimpleSect::Warning:
         type = "warning";
         break;
      case DocSimpleSect::Pre:
         type = "pre";
         break;
      case DocSimpleSect::Post:
         type = "post";
         break;
      case DocSimpleSect::Copyright:
         type = "copyright";
         break;
      case DocSimpleSect::Invar:
         type = "invariant";
         break;
      case DocSimpleSect::Remark:
         type = "remark";
         break;
      case DocSimpleSect::Attention:
         type = "attention";
         break;
      case DocSimpleSect::User:
         type = "par";
         break;
      case DocSimpleSect::Rcs:
         type = "rcs";
         break;
      case DocSimpleSect::Unknown:
         err("Unknown simple section found\n");
         break;
   }

   leaveText();
   m_output.openHash();
   openOther();
   openSubBlock(type);
}

void PerlModDocVisitor::visitPost(DocSimpleSect *)
{
   closeSubBlock();
   closeOther();
   m_output.closeHash();
}

void PerlModDocVisitor::visitPre(DocTitle *)
{
   openItem("title");
   openSubBlock("content");
}

void PerlModDocVisitor::visitPost(DocTitle *)
{
   closeSubBlock();
   closeItem();
}

void PerlModDocVisitor::visitPre(DocSimpleList *)
{
   openItem("list");
   m_output.addFieldQuotedString("style", "itemized");
   openSubBlock("content");
}

void PerlModDocVisitor::visitPost(DocSimpleList *)
{
   closeSubBlock();
   closeItem();
}

void PerlModDocVisitor::visitPre(DocSimpleListItem *)
{
   openSubBlock();
}
void PerlModDocVisitor::visitPost(DocSimpleListItem *)
{
   closeSubBlock();
}

void PerlModDocVisitor::visitPre(DocSection *s)
{
   QString sect = QString("sect%1").formatArg(s->level());

   openItem(sect);
   m_output.addFieldQuotedString("title", s->title());
   openSubBlock("content");
}

void PerlModDocVisitor::visitPost(DocSection *)
{
   closeSubBlock();
   closeItem();
}

void PerlModDocVisitor::visitPre(DocHtmlList *l)
{
   openItem("list");
   m_output.addFieldQuotedString("style", (l->type() == DocHtmlList::Ordered) ? QString("ordered") : QString("itemized"));
   openSubBlock("content");
}

void PerlModDocVisitor::visitPost(DocHtmlList *)
{
   closeSubBlock();
   closeItem();
}

void PerlModDocVisitor::visitPre(DocHtmlListItem *)
{
   openSubBlock();
}
void PerlModDocVisitor::visitPost(DocHtmlListItem *)
{
   closeSubBlock();
}

//void PerlModDocVisitor::visitPre(DocHtmlPre *)
//{
//  openItem("preformatted");
//  openSubBlock("content");
//  //m_insidePre=true;
//}

//void PerlModDocVisitor::visitPost(DocHtmlPre *)
//{
//  //m_insidePre=false;
//  closeSubBlock();
//  closeItem();
//}

void PerlModDocVisitor::visitPre(DocHtmlDescList *)
{
#if 0
   m_output.add("<variablelist>\n");
#endif
}

void PerlModDocVisitor::visitPost(DocHtmlDescList *)
{
#if 0
   m_output.add("</variablelist>\n");
#endif
}

void PerlModDocVisitor::visitPre(DocHtmlDescTitle *)
{
#if 0
   m_output.add("<varlistentry><term>");
#endif
}

void PerlModDocVisitor::visitPost(DocHtmlDescTitle *)
{
#if 0
   m_output.add("</term></varlistentry>\n");
#endif
}

void PerlModDocVisitor::visitPre(DocHtmlDescData *)
{
#if 0
   m_output.add("<listitem>");
#endif
}

void PerlModDocVisitor::visitPost(DocHtmlDescData *)
{
#if 0
   m_output.add("</listitem>\n");
#endif
}

void PerlModDocVisitor::visitPre(DocHtmlTable *)
{
#if 0
   m_output.add("<table rows=\"");
   m_output.add(t->numRows());
   m_output.add("\" cols=\"");
   m_output.add(t->numCols());
   m_output.add("\">");
#endif
}

void PerlModDocVisitor::visitPost(DocHtmlTable *)
{
#if 0
   m_output.add("</table>\n");
#endif
}

void PerlModDocVisitor::visitPre(DocHtmlRow *)
{
#if 0
   m_output.add("<row>\n");
#endif
}

void PerlModDocVisitor::visitPost(DocHtmlRow *)
{
#if 0
   m_output.add("</row>\n");
#endif
}

void PerlModDocVisitor::visitPre(DocHtmlCell *)
{
#if 0
   if (c->isHeading()) {
      m_output.add("<entry thead=\"yes\">");
   } else {
      m_output.add("<entry thead=\"no\">");
   }
#endif
}

void PerlModDocVisitor::visitPost(DocHtmlCell *)
{
#if 0
   m_output.add("</entry>");
#endif
}

void PerlModDocVisitor::visitPre(DocHtmlCaption *)
{
#if 0
   m_output.add("<caption>");
#endif
}

void PerlModDocVisitor::visitPost(DocHtmlCaption *)
{
#if 0
   m_output.add("</caption>\n");
#endif
}

void PerlModDocVisitor::visitPre(DocInternal *)
{
#if 0
   m_output.add("<internal>");
#endif
}

void PerlModDocVisitor::visitPost(DocInternal *)
{
#if 0
   m_output.add("</internal>");
#endif
}

void PerlModDocVisitor::visitPre(DocHRef *)
{
#if 0
   m_output.add("<ulink url=\"");
   m_output.add(href->url());
   m_output.add("\">");
#endif
}

void PerlModDocVisitor::visitPost(DocHRef *)
{
#if 0
   m_output.add("</ulink>");
#endif
}

void PerlModDocVisitor::visitPre(DocHtmlHeader *)
{
#if 0
   m_output.add("<sect");
   m_output.add(header->level());
   m_output.add(">");
#endif
}

void PerlModDocVisitor::visitPost(DocHtmlHeader *)
{
#if 0
   m_output.add("</sect");
   m_output.add(header->level());
   m_output.add(">\n");
#endif
}

void PerlModDocVisitor::visitPre(DocImage *)
{
#if 0
   m_output.add("<image type=\"");

   switch (img->type()) {
      case DocImage::Html:
         m_output.add("html");
         break;
      case DocImage::Latex:
         m_output.add("latex");
         break;
      case DocImage::Rtf:
         m_output.add("rtf");
         break;
   }
   m_output.add("\"");

   QString baseName = img->name();
   int i;

   if ((i = baseName.lastIndexOf('/')) != -1 || (i = baseName.lastIndexOf('\\')) != -1) {
      baseName = baseName.right(baseName.length() - i - 1);
   }

   m_output.add(" name=\"");
   m_output.add(baseName);
   m_output.add("\"");

   if (!img->width().isEmpty()) {
      m_output.add(" width=\"");
      m_output.addQuoted(img->width());
      m_output.add("\"");
   } else if (!img->height().isEmpty()) {
      m_output.add(" height=\"");
      m_output.addQuoted(img->height());
      m_output.add("\"");
   }
   m_output.add(">");
#endif
}

void PerlModDocVisitor::visitPost(DocImage *)
{
#if 0
   m_output.add("</image>");
#endif
}

void PerlModDocVisitor::visitPre(DocDotFile *)
{
#if 0
   m_output.add("<dotfile name=\"");
   m_output.add(df->file());
   m_output.add("\">");
#endif
}

void PerlModDocVisitor::visitPost(DocDotFile *)
{
#if 0
   m_output.add("</dotfile>");
#endif
}
void PerlModDocVisitor::visitPre(DocMscFile *)
{
#if 0
   m_output.add("<mscfile name=\"");
   m_output.add(df->file());
   m_output.add("\">");
#endif
}

void PerlModDocVisitor::visitPost(DocMscFile *)
{
#if 0
   m_output.add("<mscfile>");
#endif
}

void PerlModDocVisitor::visitPre(DocDiaFile *)
{
#if 0
   m_output.add("<diafile name=\"");
   m_output.add(df->file());
   m_output.add("\">");
#endif
}

void PerlModDocVisitor::visitPost(DocDiaFile *)
{
#if 0
   m_output.add("</diafile>");
#endif
}


void PerlModDocVisitor::visitPre(DocLink *lnk)
{
   openItem("link");
   addLink(lnk->ref(), lnk->file(), lnk->anchor());
}

void PerlModDocVisitor::visitPost(DocLink *)
{
   closeItem();
}

void PerlModDocVisitor::visitPre(DocRef *ref)
{
   openItem("ref");
   if (!ref->hasLinkText()) {
      m_output.addFieldQuotedString("text", ref->targetTitle());
   }
   openSubBlock("content");
}

void PerlModDocVisitor::visitPost(DocRef *)
{
   closeSubBlock();
   closeItem();
}

void PerlModDocVisitor::visitPre(DocSecRefItem *)
{
#if 0
   m_output.add("<tocitem id=\"");
   m_output.add(ref->file());
   m_output.add("_1");
   m_output.add(ref->anchor());
   m_output.add("\">");
#endif
}

void PerlModDocVisitor::visitPost(DocSecRefItem *)
{
#if 0
   m_output.add("</tocitem>");
#endif
}

void PerlModDocVisitor::visitPre(DocSecRefList *)
{
#if 0
   m_output.add("<toclist>");
#endif
}

void PerlModDocVisitor::visitPost(DocSecRefList *)
{
#if 0
   m_output.add("</toclist>");
#endif
}

//void PerlModDocVisitor::visitPre(DocLanguage *l)
//{
//  openItem("language");
//  m_output.addFieldQuotedString("id", l->id());
//}
//
//void PerlModDocVisitor::visitPost(DocLanguage *)
//{
//  closeItem();
//}

void PerlModDocVisitor::visitPre(DocParamSect *s)
{
   leaveText();
   QString type;

   switch (s->type()) {
      case DocParamSect::Param:
         type = "params";
         break;
      case DocParamSect::RetVal:
         type = "retvals";
         break;
      case DocParamSect::Exception:
         type = "exceptions";
         break;
      case DocParamSect::TemplateParam:
         type = "templateparam";
         break;
      case DocParamSect::Unknown:
         err("Unknown parameter section found\n");
         break;
   }

   m_output.openHash();

   openOther();
   openSubBlock(type);
}

void PerlModDocVisitor::visitPost(DocParamSect *)
{
   closeSubBlock();
   closeOther();

   m_output.closeHash();
}

void PerlModDocVisitor::visitPre(DocParamList *pl)
{
   leaveText();
   m_output.openHash().openList("parameters");

   for (auto param : pl->parameters()) {
      QString s;

      if (param->kind() == DocNode::Kind_Word) {
         s = ((DocWord *)param)->word();

      } else if (param->kind() == DocNode::Kind_LinkedWord) {
         s = ((DocLinkedWord *)param)->word();

      }

      QString dir        = "";
      DocParamSect *sect = nullptr;

      if (pl->parent()->kind() == DocNode::Kind_ParamSect) {
         sect = (DocParamSect *)pl->parent();
      }

      if (sect && sect->hasInOutSpecifier()) {
         if (pl->direction() != DocParamSect::Unspecified) {

            if (pl->direction() == DocParamSect::In) {
               dir = "in";

            } else if (pl->direction() == DocParamSect::Out) {
               dir = "out";

            } else if (pl->direction() == DocParamSect::InOut) {
               dir = "in,out";
            }
         }
      }

      m_output.openHash().addFieldQuotedString("name", s).addFieldQuotedString("dir", dir).closeHash();
   }

   m_output.closeList().openList("doc");
}

void PerlModDocVisitor::visitPost(DocParamList *)
{
   leaveText();
   m_output.closeList().closeHash();
}

void PerlModDocVisitor::visitPre(DocXRefItem *x)
{

#if 0
   m_output.add("<xrefsect id=\"");
   m_output.add(x->file());
   m_output.add("_1");
   m_output.add(x->anchor());
   m_output.add("\">");
   m_output.add("<xreftitle>");
   m_output.addQuoted(x->title());
   m_output.add("</xreftitle>");
   m_output.add("<xrefdescription>");
#endif

   if (x->title().isEmpty()) {
      return;
   }

   openItem("xrefitem");
   openSubBlock("content");
}

void PerlModDocVisitor::visitPost(DocXRefItem *x)
{
   if (x->title().isEmpty()) {
      return;
   }
   closeSubBlock();
   closeItem();

#if 0
   m_output.add("</xrefdescription>");
   m_output.add("</xrefsect>");
#endif

}

void PerlModDocVisitor::visitPre(DocInternalRef *ref)
{
   openItem("ref");
   addLink(0, ref->file(), ref->anchor());
   openSubBlock("content");
}

void PerlModDocVisitor::visitPost(DocInternalRef *)
{
   closeSubBlock();
   closeItem();
}

void PerlModDocVisitor::visitPre(DocCopy *)
{
}

void PerlModDocVisitor::visitPost(DocCopy *)
{
}

void PerlModDocVisitor::visitPre(DocText *)
{
}

void PerlModDocVisitor::visitPost(DocText *)
{
}

void PerlModDocVisitor::visitPre(DocHtmlBlockQuote *)
{
   openItem("blockquote");
   openSubBlock("content");
}

void PerlModDocVisitor::visitPost(DocHtmlBlockQuote *)
{
   closeSubBlock();
   closeItem();
}

void PerlModDocVisitor::visitPre(DocParBlock *)
{
}

void PerlModDocVisitor::visitPost(DocParBlock *)
{
}


static void addTemplateArgumentList(const ArgumentList &argList, PerlModOutput &output)
{
   if (argList.listEmpty()) {
      return;
   }

   output.openList("template_parameters");

   for (auto &a :argList) {
      output.openHash();

      if (! a.type.isEmpty()) {
         output.addFieldQuotedString("type", a.type);
      }

      if (! a.name.isEmpty()) {
         output.addFieldQuotedString("declaration_name", a.name).addFieldQuotedString("definition_name", a.name);
      }

      if (! a.defval.isEmpty()) {
         output.addFieldQuotedString("default", a.defval);
      }

      output.closeHash();
   }

   output.closeList();
}

static void addTemplateList(QSharedPointer<ClassDef> cd, PerlModOutput &output)
{
   addTemplateArgumentList(cd->getTemplateArgumentList(), output);
}

static void addPerlModDocBlock(PerlModOutput &output, const QString &name, const QString &fileName, int lineNr,
                               QSharedPointer<Definition> scope, QSharedPointer<MemberDef> md, const QString &text)
{
   QString stext = text.trimmed();

   if (stext.isEmpty()) {
      output.addField(name).add("{}");

   } else {

      DocNode *root = validatingParseDoc(fileName, lineNr, scope, md, stext, false, false);
      output.openHash(name);

      PerlModDocVisitor *visitor = new PerlModDocVisitor(output);
      root->accept(visitor);
      visitor->finish();
      output.closeHash();

      delete visitor;
      delete root;
   }
}

static const QString getProtectionName(Protection prot)
{
   QString retval;

   switch (prot) {
      case Public:
         retval = "public";
         break;

      case Protected:
         retval = "protected";
         break;

      case Private:
         retval = "private";
         break;

      case Package:
         retval = "package";
         break;

   }

   return retval;
}

static const QString getVirtualnessName(Specifier virt)
{
   QString retval;

   switch (virt) {
      case Normal:
         retval = "non_virtual";
         break;

      case Virtual:
         retval = "virtual";
         break;

      case Pure:
         retval = "pure_virtual";
         break;
   }

   return retval;
}

void setPerlModDoxyfile(const QString &qs)
{
   pathDoxyfile = qs;
   pathDoxyExec = QDir::currentPath();
}

class PerlModGenerator
{
 public:
   PerlModOutput m_output;

   QString pathDoxyStructurePM;
   QString pathDoxyDocsTex;
   QString pathDoxyFormatTex;
   QString pathDoxyLatexTex;
   QString pathDoxyLatexDVI;
   QString pathDoxyLatexPDF;
   QString pathDoxyStructureTex;
   QString pathDoxyDocsPM;
   QString pathDoxyLatexPL;
   QString pathDoxyLatexStructurePL;
   QString pathDoxyRules;
   QString pathMakefile;

   inline PerlModGenerator(bool pretty) : m_output(pretty) { }

   void generatePerlModForMember(QSharedPointer<MemberDef> md, QSharedPointer<Definition> def);

   void generatePerlModSection(QSharedPointer<Definition> def, QSharedPointer<MemberList> ml,
                  const QString &name, const QString &header = QString());

   void addListOfAllMembers(QSharedPointer<ClassDef> cd);
   void generatePerlModForClass(QSharedPointer<ClassDef> cd);
   void generatePerlModForNamespace(QSharedPointer<NamespaceDef> nd);
   void generatePerlModForFile(QSharedPointer<FileDef> fd);
   void generatePerlModForGroup(QSharedPointer<GroupDef> gd);
   void generatePerlModForPage(QSharedPointer<PageDef> pd);

   bool createOutputFile(QFile &f, const QString &s);
   bool createOutputDir(QDir &perlModDir);
   bool generateDoxyLatexTex();
   bool generateDoxyFormatTex();
   bool generateDoxyStructurePM();
   bool generateDoxyLatexPL();
   bool generateDoxyLatexStructurePL();
   bool generateDoxyRules();
   bool generateMakefile();
   bool generatePerlModOutput();

   void generate();
};

void PerlModGenerator::generatePerlModForMember(QSharedPointer<MemberDef> md, QSharedPointer<Definition> def)
{
   // + declaration/definition arg lists
   // + reimplements
   // + reimplementedBy
   // + exceptions
   // + const/volatile specifiers
   // - examples
   // - source definition
   // - source references
   // - source referenced by
   // - body code
   // - template arguments
   //     (templateArguments(), definitionTemplateParameterLists())

   QString memType;
   bool isFunc = false;

   switch (md->memberType()) {
      case MemberType_Define:
         memType = "define";
         break;

      case MemberType_EnumValue:
         memType = "enumvalue";
         break;

      case MemberType_Property:
         memType = "property";
         break;

      case MemberType_Variable:
         memType = "variable";
         break;

      case MemberType_Typedef:
         memType = "typedef";
         break;

      case MemberType_Enumeration:
         memType = "enum";
         break;

      case MemberType_Function:
         memType = "function";
         isFunc = true;
         break;

      case MemberType_Signal:
         memType = "signal";
         isFunc = true;
         break;

      // case MemberType_Prototype:
      //   memType="prototype";
      //   isFunc=true;
      //   break;

      case MemberType_DCOP:
         memType = "dcop";
         isFunc = true;
         break;

      case MemberType_Slot:
         memType = "slot";
         isFunc = true;
         break;

      case MemberType_Event:
         memType = "event";
         break;

      case MemberType_Interface:
         memType = "interface";
         break;

      case MemberType_Service:
         memType = "service";
         break;
   }

   if (md->isFriend()) {
      memType.prepend("friend ");
   }

   m_output.openHash()
      .addFieldQuotedString("kind", memType)
      .addFieldQuotedString("name", md->name())
      .addFieldQuotedString("virtualness", getVirtualnessName(md->virtualness()))
      .addFieldQuotedString("protection", getProtectionName(md->protection()))
      .addFieldBoolean("static", md->isStatic());

   addPerlModDocBlock(m_output, "brief",    md->getDefFileName(), md->getDefLine(), md->getOuterScope(), md, md->briefDescription());
   addPerlModDocBlock(m_output, "detailed", md->getDefFileName(), md->getDefLine(), md->getOuterScope(), md, md->documentation());

   if (md->memberType() != MemberType_Define && md->memberType() != MemberType_Enumeration) {
      m_output.addFieldQuotedString("type", md->typeString());
   }

   const ArgumentList &al = md->getArgumentList();

   if (isFunc) {
      // function
      m_output.addFieldBoolean("const", al.constSpecifier).addFieldBoolean("volatile", al.volatileSpecifier);

      m_output.openList("parameters");
      const ArgumentList &declAl = md->getDeclArgumentList();
      const ArgumentList &defAl  = md->getArgumentList();

      auto iter = defAl.begin();

      for (const auto &arg : declAl) {
         m_output.openHash();

         if (! arg.name.isEmpty()) {
            m_output.addFieldQuotedString("declaration_name", arg.name);
         }

         if (iter != defAl.end()) {
            const Argument &defArg = *iter;

            if (! defArg.name.isEmpty() && defArg.name != arg.name) {
               m_output.addFieldQuotedString("definition_name", defArg.name);
            }

            // increment must come at the end of this if
            ++iter;
         }

         if (! arg.type.isEmpty()) {
            m_output.addFieldQuotedString("type", arg.type);
         }

         if (! arg.array.isEmpty()) {
            m_output.addFieldQuotedString("array", arg.array);
         }

         if (! arg.defval.isEmpty()) {
            m_output.addFieldQuotedString("default_value", arg.defval);
         }

         if (! arg.attrib.isEmpty()) {
            m_output.addFieldQuotedString("attributes", arg.attrib);
         }

         m_output.closeHash();
      }

      m_output.closeList();

   } else if (md->memberType() == MemberType_Define && md->argsString() != 0) {
      // define
      m_output.openList("parameters");

      for (auto &arg : al) {
         m_output.openHash().addFieldQuotedString("name", arg.type).closeHash();
      }

      m_output.closeList();

   } else if (md->argsString() != 0) {
      m_output.addFieldQuotedString("arguments", md->argsString());
   }

   if (!md->initializer().isEmpty()) {
      m_output.addFieldQuotedString("initializer", md->initializer());
   }

   if (! md->excpString().isEmpty() ) {
      m_output.addFieldQuotedString("exceptions", md->excpString());
   }

   if (md->memberType() == MemberType_Enumeration) { // enum
      QSharedPointer<MemberList> enumFields = md->enumFieldList();

      if (enumFields) {
         m_output.openList("values");

         for (auto emd : *enumFields) {
            m_output.openHash().addFieldQuotedString("name", emd->name());

            if (! emd->initializer().isEmpty()) {
               m_output.addFieldQuotedString("initializer", emd->initializer());
            }

            addPerlModDocBlock(m_output, "brief",    emd->getDefFileName(), emd->getDefLine(), emd->getOuterScope(), emd, emd->briefDescription());
            addPerlModDocBlock(m_output, "detailed", emd->getDefFileName(), emd->getDefLine(), emd->getOuterScope(), emd, emd->documentation());

            m_output.closeHash();
         }

         m_output.closeList();
      }
   }

   QSharedPointer<MemberDef> rmd = md->reimplements();

   if (rmd) {
      m_output.openHash("reimplements").addFieldQuotedString("name", rmd->name()).closeHash();
   }

   QSharedPointer<MemberList> rbml = md->reimplementedBy();

   if (rbml) {
      m_output.openList("reimplemented_by");

      for (auto rmd : *rbml) {
         m_output.openHash().addFieldQuotedString("name", rmd->name()).closeHash();
      }

      m_output.closeList();
   }

   m_output.closeHash();
}

void PerlModGenerator::generatePerlModSection(QSharedPointer<Definition> d, QSharedPointer<MemberList> ml,
         const QString &name, const QString &header)
{
   if (ml == 0) {
      return;   // empty list
   }

   m_output.openHash(name);

   if (! header.isEmpty()) {
      m_output.addFieldQuotedString("header", header);
   }

   m_output.openList("members");

   for (auto md : *ml) {
      generatePerlModForMember(md, d);
   }

   m_output.closeList().closeHash();
}

void PerlModGenerator::addListOfAllMembers(QSharedPointer<ClassDef> cd)
{
   m_output.openList("all_members");

   for (auto mni : cd->memberNameInfoSDict()) {

      for (auto mi : *mni) {
         QSharedPointer<MemberDef> md = mi.memberDef;

         QSharedPointer<ClassDef>   cd  = md->getClassDef();
         QSharedPointer<Definition> def = md->getGroupDef();

         if (def == 0) {
            def = cd;
         }

         m_output.openHash().addFieldQuotedString("name", md->name())
            .addFieldQuotedString("virtualness", getVirtualnessName(md->virtualness()))
            .addFieldQuotedString("protection",  getProtectionName(mi.prot));

         if (! mi.ambiguityResolutionScope.isEmpty()) {
            m_output.addFieldQuotedString("ambiguity_scope", mi.ambiguityResolutionScope);
         }

         m_output.addFieldQuotedString("scope", cd->name()).closeHash();
      }
   }

   m_output.closeList();
}

void PerlModGenerator::generatePerlModForClass(QSharedPointer<ClassDef> cd)
{
   // + brief description
   // + detailed description
   // + template argument list(s)
   // - include file
   // + member groups
   // + inheritance diagram
   // + list of direct super classes
   // + list of direct sub classes
   // + list of inner classes
   // + collaboration diagram
   // + list of all members
   // + user defined member sections
   // + standard member sections
   // + detailed member documentation
   // - examples using the class

   if (cd->isReference()) {
      return;   // skip external references.
   }

   if (cd->name().indexOf('@') != -1) {
      return;   // skip anonymous compounds.
   }

   if (cd->templateMaster() != 0) {
      return;   // skip generated template instances.
   }

   m_output.openHash().addFieldQuotedString("name", cd->name());

   if (cd->baseClasses()) {
      m_output.openList("base");

      for (auto bcd : *cd->baseClasses()) {
         m_output.openHash()
         .addFieldQuotedString("name", bcd->classDef->displayName())
         .addFieldQuotedString("virtualness", getVirtualnessName(bcd->virt))
         .addFieldQuotedString("protection",  getProtectionName(bcd->prot)).closeHash();
      }

      m_output.closeList();
   }

   if (cd->subClasses()) {
      m_output.openList("derived");

      for (auto bcd : *cd->subClasses()) {
         m_output.openHash()
         .addFieldQuotedString("name", bcd->classDef->displayName())
         .addFieldQuotedString("virtualness", getVirtualnessName(bcd->virt))
         .addFieldQuotedString("protection",  getProtectionName(bcd->prot)).closeHash();
      }

      m_output.closeList();
   }

   const ClassSDict &cl = cd->getClassSDict();

   if (cl.count() > 0) {
      m_output.openList("inner");

      for (auto cd : cl) {
         m_output.openHash().addFieldQuotedString("name", cd->name()).closeHash();
      }

      m_output.closeList();
   }

   const IncludeInfo &item = cd->includeInfo();

   QString nm = item.includeName;

   if (nm.isEmpty() && item.fileDef) {
      nm = item.fileDef->docName();
   }

   if (! nm.isEmpty()) {
      m_output.openHash("includes");
      m_output.addFieldBoolean("local", item.local).addFieldQuotedString("name", nm).closeHash();
   }

   addTemplateList(cd, m_output);
   addListOfAllMembers(cd);

   for (auto mg : cd->getMemberGroupSDict()) {
      generatePerlModSection(cd, mg->members(), "user_defined", mg->header());
   }

   generatePerlModSection(cd, cd->getMemberList(MemberListType_pubTypes),         "public_typedefs");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_pubMethods),       "public_methods");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_pubAttribs),       "public_members");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_pubSignals),       "public-signals");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_pubSlots),         "public_slots");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_dcopMethods),      "dcop_methods");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_properties),       "properties");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_pubStaticMethods), "public_static_methods");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_pubStaticAttribs), "public_static_members");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_proTypes),         "protected_typedefs");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_proMethods),       "protected_methods");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_proAttribs),       "protected_members");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_proSignals),       "protected-signals");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_proSlots),         "protected_slots");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_proStaticMethods), "protected_static_methods");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_proStaticAttribs), "protected_static_members");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_priTypes),         "private_typedefs");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_priMethods),       "private_methods");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_priAttribs),       "private_members");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_priSignals),       "private-signals");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_priSlots),         "private_slots");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_priStaticMethods), "private_static_methods");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_priStaticAttribs), "private_static_members");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_friends),          "friend_methods");
   generatePerlModSection(cd, cd->getMemberList(MemberListType_related),          "related_methods");

   QSharedPointer<MemberDef> nullMem;

   addPerlModDocBlock(m_output, "brief",    cd->getDefFileName(), cd->getDefLine(), cd, nullMem, cd->briefDescription());
   addPerlModDocBlock(m_output, "detailed", cd->getDefFileName(), cd->getDefLine(), cd, nullMem, cd->documentation());

#if 0
   DotClassGraph inheritanceGraph(cd, DotClassGraph::Inheritance);
   if (!inheritanceGraph.isTrivial()) {
      t << "    <inheritancegraph>" << endl;
      inheritanceGraph.writePerlMod(t);
      t << "    </inheritancegraph>" << endl;
   }

   DotClassGraph collaborationGraph(cd, DotClassGraph::Implementation);
   if (!collaborationGraph.isTrivial()) {
      t << "    <collaborationgraph>" << endl;
      collaborationGraph.writePerlMod(t);
      t << "    </collaborationgraph>" << endl;
   }

   t << "    <location file=\""
     << cd->getDefFileName() << "\" line=\""
     << cd->getDefLine() << "\"";
   if (cd->getStartBodyLine() != -1) {
      t << " bodystart=\"" << cd->getStartBodyLine() << "\" bodyend=\""
        << cd->getEndBodyLine() << "\"";
   }

   t << "/>" << endl;
#endif

   m_output.closeHash();
}

void PerlModGenerator::generatePerlModForNamespace(QSharedPointer<NamespaceDef> nd)
{
   // + contained class definitions
   // + contained namespace definitions
   // + member groups
   // + normal members
   // + brief desc
   // + detailed desc
   // + location
   // - files containing (parts of) the namespace definition

   if (nd->isReference()) {
      return;   // skip external references
   }

   m_output.openHash().addFieldQuotedString("name", nd->name());

   const ClassSDict &cl = nd->getClassSDict();

   if (cl.count() > 0) {
      m_output.openList("classes");

      for (auto cd : cl) {
         m_output.openHash().addFieldQuotedString("name", cd->name()).closeHash();
      }

      m_output.closeList();
   }

   const NamespaceSDict &nl = nd->getNamespaceSDict();

   if (! nl.isEmpty()) {
      m_output.openList("namespaces");

      for (auto &nd : nl) {
         m_output.openHash().addFieldQuotedString("name", nd->name()).closeHash();
      }

      m_output.closeList();
   }

   for (auto mg : nd->getMemberGroupSDict()) {
      generatePerlModSection(nd, mg->members(), "user-defined", mg->header());
   }


   generatePerlModSection(nd, nd->getMemberList(MemberListType_decDefineMembers),  "defines");
   generatePerlModSection(nd, nd->getMemberList(MemberListType_decProtoMembers),   "prototypes");
   generatePerlModSection(nd, nd->getMemberList(MemberListType_decTypedefMembers), "typedefs");
   generatePerlModSection(nd, nd->getMemberList(MemberListType_decEnumMembers),    "enums");
   generatePerlModSection(nd, nd->getMemberList(MemberListType_decFuncMembers),    "functions");
   generatePerlModSection(nd, nd->getMemberList(MemberListType_decVarMembers),     "variables");

   QSharedPointer<Definition> nullDef;
   QSharedPointer<MemberDef> nullMem;

   addPerlModDocBlock(m_output, "brief",    nd->getDefFileName(), nd->getDefLine(), nullDef, nullMem, nd->briefDescription());
   addPerlModDocBlock(m_output, "detailed", nd->getDefFileName(), nd->getDefLine(), nullDef, nullMem, nd->documentation());

   m_output.closeHash();
}

void PerlModGenerator::generatePerlModForFile(QSharedPointer<FileDef> fd)
{
   // + includes files
   // + includedby files
   // - include graph
   // - included by graph
   // - contained class definitions
   // - contained namespace definitions
   // - member groups
   // + normal members
   // + brief desc
   // + detailed desc
   // - source code
   // - location
   // - number of lines

   if (fd->isReference()) {
      return;
   }

   m_output.openHash().addFieldQuotedString("name", fd->name());

   IncludeInfo *inc;
   m_output.openList("includes");

   if (fd->includeFileList()) {
      for (auto inc : *fd->includeFileList()) {
         m_output.openHash().addFieldQuotedString("name", inc.includeName);

         if (inc.fileDef && ! inc.fileDef->isReference()) {
            m_output.addFieldQuotedString("ref", inc.fileDef->getOutputFileBase());
         }

         m_output.closeHash();
      }
   }
   m_output.closeList();

   m_output.openList("included_by");

   if (fd->includedByFileList()) {
      for (auto inc : *fd->includedByFileList()) {
         m_output.openHash().addFieldQuotedString("name", inc.includeName);

         if (inc.fileDef && ! inc.fileDef->isReference()) {
            m_output.addFieldQuotedString("ref", inc.fileDef->getOutputFileBase());
         }

         m_output.closeHash();
      }
   }
   m_output.closeList();

   generatePerlModSection(fd, fd->getMemberList(MemberListType_decDefineMembers), "defines");
   generatePerlModSection(fd, fd->getMemberList(MemberListType_decProtoMembers), "prototypes");
   generatePerlModSection(fd, fd->getMemberList(MemberListType_decTypedefMembers), "typedefs");
   generatePerlModSection(fd, fd->getMemberList(MemberListType_decEnumMembers), "enums");
   generatePerlModSection(fd, fd->getMemberList(MemberListType_decFuncMembers), "functions");
   generatePerlModSection(fd, fd->getMemberList(MemberListType_decVarMembers), "variables");

   QSharedPointer<Definition> nullDef;
   QSharedPointer<MemberDef> nullMem;

   addPerlModDocBlock(m_output, "brief",    fd->getDefFileName(), fd->getDefLine(), nullDef, nullMem, fd->briefDescription());
   addPerlModDocBlock(m_output, "detailed", fd->getDefFileName(), fd->getDefLine(), nullDef, nullMem, fd->documentation());

   m_output.closeHash();
}

void PerlModGenerator::generatePerlModForGroup(QSharedPointer<GroupDef> gd)
{
   // + members
   // + member groups
   // + files
   // + classes
   // + namespaces
   // - packages
   // + pages
   // + child groups
   // - examples
   // + brief description
   // + detailed description

   if (gd->isReference()) {
      return;   // skip external references
   }

   m_output.openHash().addFieldQuotedString("name", gd->name()).addFieldQuotedString("title", gd->groupTitle());

   FileList fl = gd->getFiles();

   m_output.openList("files");

   for (auto fd : fl) {
      m_output.openHash().addFieldQuotedString("name", fd->name()).closeHash();
   }

   m_output.closeList();

   const ClassSDict &cl = gd->getClasses();
   if (cl.count() > 0) {
      m_output.openList("classes");

      for (auto cd : cl) {
         m_output.openHash().addFieldQuotedString("name", cd->name()).closeHash();
      }

      m_output.closeList();
   }

   const NamespaceSDict &nl = gd->getNamespaces();
   if (! nl.isEmpty()) {
      m_output.openList("namespaces");

      for (auto &nd : nl) {
         m_output.openHash().addFieldQuotedString("name", nd->name()).closeHash();
      }
      m_output.closeList();
   }

   PageSDict *pl = gd->getPages();
   if (pl) {
      m_output.openList("pages");

      for (auto pd : *pl) {
         m_output.openHash().addFieldQuotedString("title", pd->title()).closeHash();
      }
      m_output.closeList();
   }

   SortedList<QSharedPointer<GroupDef>> *gl = gd->getSubGroups();

   if (gl) {
      m_output.openList("groups");

      for (auto sgd : *gl) {
         m_output.openHash().addFieldQuotedString("title", sgd->groupTitle()).closeHash();
      }

      m_output.closeList();
   }

   for (auto mg : gd->getMemberGroupSDict()){
      generatePerlModSection(gd, mg->members(), "user-defined", mg->header());
   }

   generatePerlModSection(gd, gd->getMemberList(MemberListType_decDefineMembers),  "defines");
   generatePerlModSection(gd, gd->getMemberList(MemberListType_decProtoMembers),   "prototypes");
   generatePerlModSection(gd, gd->getMemberList(MemberListType_decTypedefMembers), "typedefs");
   generatePerlModSection(gd, gd->getMemberList(MemberListType_decEnumMembers),    "enums");
   generatePerlModSection(gd, gd->getMemberList(MemberListType_decFuncMembers),    "functions");
   generatePerlModSection(gd, gd->getMemberList(MemberListType_decVarMembers),     "variables");

   QSharedPointer<Definition> nullDef;
   QSharedPointer<MemberDef> nullMem;

   addPerlModDocBlock(m_output, "brief",    gd->getDefFileName(), gd->getDefLine(), nullDef, nullMem, gd->briefDescription());
   addPerlModDocBlock(m_output, "detailed", gd->getDefFileName(), gd->getDefLine(), nullDef, nullMem, gd->documentation());

   m_output.closeHash();
}

void PerlModGenerator::generatePerlModForPage(QSharedPointer<PageDef> pd)
{
   // + name
   // + title
   // + documentation

   if (pd->isReference()) {
      return;
   }

   m_output.openHash().addFieldQuotedString("name", pd->name());

   QSharedPointer<SectionInfo> si = Doxy_Globals::sectionDict.find(pd->name());
   if (si) {
      m_output.addFieldQuotedString("title4", filterTitle(si->title));
   }

   QSharedPointer<Definition> nullDef;
   QSharedPointer<MemberDef> nullMem;

   addPerlModDocBlock(m_output, "detailed", pd->docFile(), pd->docLine(), nullDef, nullMem, pd->documentation());

   m_output.closeHash();
}

bool PerlModGenerator::generatePerlModOutput()
{
   QFile outputFile;
   if (! createOutputFile(outputFile, pathDoxyDocsPM)) {
      return false;
   }

   QTextStream outputTextStream(&outputFile);
   PerlModOutputStream outputStream(&outputTextStream);
   m_output.setPerlModOutputStream(&outputStream);
   m_output.add("$doxydocs=").openHash();

   m_output.openList("classes");
   for (auto cd : Doxy_Globals::classSDict) {
      generatePerlModForClass(cd);
   }
   m_output.closeList();

   m_output.openList("namespaces");
   for (auto &nd : Doxy_Globals::namespaceSDict) {
      generatePerlModForNamespace(nd);
   }
   m_output.closeList();

   m_output.openList("files");
   for (auto &fn : Doxy_Globals::inputNameList) {
      for (auto fd : *fn)  {
         generatePerlModForFile(fd);
      }
   }
   m_output.closeList();

   m_output.openList("groups");
   for (auto gd : Doxy_Globals::groupSDict) {
      generatePerlModForGroup(gd);
   }
   m_output.closeList();


   m_output.openList("pages");
   for (auto &pd : Doxy_Globals::pageSDict) {
      generatePerlModForPage(pd);
   }


   if (Doxy_Globals::mainPage) {
      generatePerlModForPage(Doxy_Globals::mainPage);
   }
   m_output.closeList();


   m_output.closeHash().add(";\n1;\n");

   return true;
}

bool PerlModGenerator::createOutputFile(QFile &f, const QString &s)
{
   f.setFileName(s);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", csPrintable(s), f.error());
      return false;
   }

   return true;
}

bool PerlModGenerator::createOutputDir(QDir &perlModDir)
{
   static const QString outputDirectory = Config::getString("output-dir");

   QDir dir(outputDirectory);

   if (! dir.exists()) {
      dir.setPath(QDir::currentPath());

      if (! dir.mkdir(outputDirectory)) {
         err("Unable to create directory %s\n", csPrintable(outputDirectory));
         return false;
      }
   }

   perlModDir.setPath(outputDirectory + "/perlmod");

   if (! perlModDir.exists() && ! perlModDir.mkdir(outputDirectory + "/perlmod")) {
      err("Unable to create perlmod directory in %s\n", csPrintable(outputDirectory));
      return false;
   }
   return true;
}

bool PerlModGenerator::generateDoxyStructurePM()
{
   QFile doxyModelPM;
   if (!createOutputFile(doxyModelPM, pathDoxyStructurePM)) {
      return false;
   }

   QTextStream doxyModelPMStream(&doxyModelPM);
   doxyModelPMStream <<
                     "sub memberlist($) {\n"
                     "    my $prefix = $_[0];\n"
                     "    return\n"
                     "\t[ \"hash\", $prefix . \"s\",\n"
                     "\t  {\n"
                     "\t    members =>\n"
                     "\t      [ \"list\", $prefix . \"List\",\n"
                     "\t\t[ \"hash\", $prefix,\n"
                     "\t\t  {\n"
                     "\t\t    kind => [ \"string\", $prefix . \"Kind\" ],\n"
                     "\t\t    name => [ \"string\", $prefix . \"Name\" ],\n"
                     "\t\t    static => [ \"string\", $prefix . \"Static\" ],\n"
                     "\t\t    virtualness => [ \"string\", $prefix . \"Virtualness\" ],\n"
                     "\t\t    protection => [ \"string\", $prefix . \"Protection\" ],\n"
                     "\t\t    type => [ \"string\", $prefix . \"Type\" ],\n"
                     "\t\t    parameters =>\n"
                     "\t\t      [ \"list\", $prefix . \"Params\",\n"
                     "\t\t\t[ \"hash\", $prefix . \"Param\",\n"
                     "\t\t\t  {\n"
                     "\t\t\t    declaration_name => [ \"string\", $prefix . \"ParamName\" ],\n"
                     "\t\t\t    type => [ \"string\", $prefix . \"ParamType\" ],\n"
                     "\t\t\t  },\n"
                     "\t\t\t],\n"
                     "\t\t      ],\n"
                     "\t\t    detailed =>\n"
                     "\t\t      [ \"hash\", $prefix . \"Detailed\",\n"
                     "\t\t\t{\n"
                     "\t\t\t  doc => [ \"doc\", $prefix . \"DetailedDoc\" ],\n"
                     "\t\t\t  return => [ \"doc\", $prefix . \"Return\" ],\n"
                     "\t\t\t  see => [ \"doc\", $prefix . \"See\" ],\n"
                     "\t\t\t  params =>\n"
                     "\t\t\t    [ \"list\", $prefix . \"PDBlocks\",\n"
                     "\t\t\t      [ \"hash\", $prefix . \"PDBlock\",\n"
                     "\t\t\t\t{\n"
                     "\t\t\t\t  parameters =>\n"
                     "\t\t\t\t    [ \"list\", $prefix . \"PDParams\",\n"
                     "\t\t\t\t      [ \"hash\", $prefix . \"PDParam\",\n"
                     "\t\t\t\t\t{\n"
                     "\t\t\t\t\t  name => [ \"string\", $prefix . \"PDParamName\" ],\n"
                     "\t\t\t\t\t},\n"
                     "\t\t\t\t      ],\n"
                     "\t\t\t\t    ],\n"
                     "\t\t\t\t  doc => [ \"doc\", $prefix . \"PDDoc\" ],\n"
                     "\t\t\t\t},\n"
                     "\t\t\t      ],\n"
                     "\t\t\t    ],\n"
                     "\t\t\t},\n"
                     "\t\t      ],\n"
                     "\t\t  },\n"
                     "\t\t],\n"
                     "\t      ],\n"
                     "\t  },\n"
                     "\t];\n"
                     "}\n"
                     "\n"
                     "$doxystructure =\n"
                     "    [ \"hash\", \"Root\",\n"
                     "      {\n"
                     "\tfiles =>\n"
                     "\t  [ \"list\", \"Files\",\n"
                     "\t    [ \"hash\", \"File\",\n"
                     "\t      {\n"
                     "\t\tname => [ \"string\", \"FileName\" ],\n"
                     "\t\ttypedefs => memberlist(\"FileTypedef\"),\n"
                     "\t\tvariables => memberlist(\"FileVariable\"),\n"
                     "\t\tfunctions => memberlist(\"FileFunction\"),\n"
                     "\t\tdetailed =>\n"
                     "\t\t  [ \"hash\", \"FileDetailed\",\n"
                     "\t\t    {\n"
                     "\t\t      doc => [ \"doc\", \"FileDetailedDoc\" ],\n"
                     "\t\t    },\n"
                     "\t\t  ],\n"
                     "\t      },\n"
                     "\t    ],\n"
                     "\t  ],\n"
                     "\tpages =>\n"
                     "\t  [ \"list\", \"Pages\",\n"
                     "\t    [ \"hash\", \"Page\",\n"
                     "\t      {\n"
                     "\t\tname => [ \"string\", \"PageName\" ],\n"
                     "\t\tdetailed =>\n"
                     "\t\t  [ \"hash\", \"PageDetailed\",\n"
                     "\t\t    {\n"
                     "\t\t      doc => [ \"doc\", \"PageDetailedDoc\" ],\n"
                     "\t\t    },\n"
                     "\t\t  ],\n"
                     "\t      },\n"
                     "\t    ],\n"
                     "\t  ],\n"
                     "\tclasses =>\n"
                     "\t  [ \"list\", \"Classes\",\n"
                     "\t    [ \"hash\", \"Class\",\n"
                     "\t      {\n"
                     "\t\tname => [ \"string\", \"ClassName\" ],\n"
                     "\t\tpublic_typedefs => memberlist(\"ClassPublicTypedef\"),\n"
                     "\t\tpublic_methods => memberlist(\"ClassPublicMethod\"),\n"
                     "\t\tpublic_members => memberlist(\"ClassPublicMember\"),\n"
                     "\t\tprotected_typedefs => memberlist(\"ClassProtectedTypedef\"),\n"
                     "\t\tprotected_methods => memberlist(\"ClassProtectedMethod\"),\n"
                     "\t\tprotected_members => memberlist(\"ClassProtectedMember\"),\n"
                     "\t\tprivate_typedefs => memberlist(\"ClassPrivateTypedef\"),\n"
                     "\t\tprivate_methods => memberlist(\"ClassPrivateMethod\"),\n"
                     "\t\tprivate_members => memberlist(\"ClassPrivateMember\"),\n"
                     "\t\tdetailed =>\n"
                     "\t\t  [ \"hash\", \"ClassDetailed\",\n"
                     "\t\t    {\n"
                     "\t\t      doc => [ \"doc\", \"ClassDetailedDoc\" ],\n"
                     "\t\t    },\n"
                     "\t\t  ],\n"
                     "\t      },\n"
                     "\t    ],\n"
                     "\t  ],\n"
                     "\tgroups =>\n"
                     "\t  [ \"list\", \"Groups\",\n"
                     "\t    [ \"hash\", \"Group\",\n"
                     "\t      {\n"
                     "\t\tname => [ \"string\", \"GroupName\" ],\n"
                     "\t\ttitle => [ \"string\", \"GroupTitle\" ],\n"
                     "\t\tfiles =>\n"
                     "\t\t  [ \"list\", \"Files\",\n"
                     "\t\t    [ \"hash\", \"File\",\n"
                     "\t\t      {\n"
                     "\t\t        name => [ \"string\", \"Filename\" ]\n"
                     "\t\t      }\n"
                     "\t\t    ],\n"
                     "\t\t  ],\n"
                     "\t\tclasses  =>\n"
                     "\t\t  [ \"list\", \"Classes\",\n"
                     "\t\t    [ \"hash\", \"Class\",\n"
                     "\t\t      {\n"
                     "\t\t        name => [ \"string\", \"Classname\" ]\n"
                     "\t\t      }\n"
                     "\t\t    ],\n"
                     "\t\t  ],\n"
                     "\t\tnamespaces =>\n"
                     "\t\t  [ \"list\", \"Namespaces\",\n"
                     "\t\t    [ \"hash\", \"Namespace\",\n"
                     "\t\t      {\n"
                     "\t\t        name => [ \"string\", \"NamespaceName\" ]\n"
                     "\t\t      }\n"
                     "\t\t    ],\n"
                     "\t\t  ],\n"
                     "\t\tpages =>\n"
                     "\t\t  [ \"list\", \"Pages\",\n"
                     "\t\t    [ \"hash\", \"Page\","
                     "\t\t      {\n"
                     "\t\t        title => [ \"string\", \"PageName\" ]\n"
                     "\t\t      }\n"
                     "\t\t    ],\n"
                     "\t\t  ],\n"
                     "\t\tgroups =>\n"
                     "\t\t  [ \"list\", \"Groups\",\n"
                     "\t\t    [ \"hash\", \"Group\",\n"
                     "\t\t      {\n"
                     "\t\t        title => [ \"string\", \"GroupName\" ]\n"
                     "\t\t      }\n"
                     "\t\t    ],\n"
                     "\t\t  ],\n"
                     "\t\tfunctions => memberlist(\"GroupFunction\"),\n"
                     "\t\tdetailed =>\n"
                     "\t\t  [ \"hash\", \"GroupDetailed\",\n"
                     "\t\t    {\n"
                     "\t\t      doc => [ \"doc\", \"GroupDetailedDoc\" ],\n"
                     "\t\t    },\n"
                     "\t\t  ],\n"
                     "\t      }\n"
                     "\t    ],\n"
                     "\t  ],\n"
                     "      },\n"
                     "    ];\n"
                     "\n"
                     "1;\n";

   return true;
}

bool PerlModGenerator::generateDoxyRules()
{
   QFile doxyRules;
   if (! createOutputFile(doxyRules, pathDoxyRules)) {
      return false;
   }

   bool perlmodLatex = Config::getBool("perl-latex");
   QString prefix = Config::getString("perl-prefix");

   QTextStream doxyRulesStream(&doxyRules);
   doxyRulesStream <<
                   prefix << "DOXY_EXEC_PATH = " << pathDoxyExec << "\n" <<
                   prefix << "DOXYFILE = " << pathDoxyfile << "\n" <<
                   prefix << "DOXYDOCS_PM = " << pathDoxyDocsPM << "\n" <<
                   prefix << "DOXYSTRUCTURE_PM = " << pathDoxyStructurePM << "\n" <<
                   prefix << "DOXYRULES = " << pathDoxyRules << "\n";

   if (perlmodLatex)
      doxyRulesStream <<
                      prefix << "DOXYLATEX_PL = " << pathDoxyLatexPL << "\n" <<
                      prefix << "DOXYLATEXSTRUCTURE_PL = " << pathDoxyLatexStructurePL << "\n" <<
                      prefix << "DOXYSTRUCTURE_TEX = " << pathDoxyStructureTex << "\n" <<
                      prefix << "DOXYDOCS_TEX = " << pathDoxyDocsTex << "\n" <<
                      prefix << "DOXYFORMAT_TEX = " << pathDoxyFormatTex << "\n" <<
                      prefix << "DOXYLATEX_TEX = " << pathDoxyLatexTex << "\n" <<
                      prefix << "DOXYLATEX_DVI = " << pathDoxyLatexDVI << "\n" <<
                      prefix << "DOXYLATEX_PDF = " << pathDoxyLatexPDF << "\n";

   doxyRulesStream <<
                   "\n"
                   ".PHONY: clean-perlmod\n"
                   "clean-perlmod::\n"
                   "\trm -f $(" << prefix << "DOXYSTRUCTURE_PM) \\\n"
                   "\t$(" << prefix << "DOXYDOCS_PM)";

   if (perlmodLatex)
      doxyRulesStream <<
                      " \\\n"
                      "\t$(" << prefix << "DOXYLATEX_PL) \\\n"
                      "\t$(" << prefix << "DOXYLATEXSTRUCTURE_PL) \\\n"
                      "\t$(" << prefix << "DOXYDOCS_TEX) \\\n"
                      "\t$(" << prefix << "DOXYSTRUCTURE_TEX) \\\n"
                      "\t$(" << prefix << "DOXYFORMAT_TEX) \\\n"
                      "\t$(" << prefix << "DOXYLATEX_TEX) \\\n"
                      "\t$(" << prefix << "DOXYLATEX_PDF) \\\n"
                      "\t$(" << prefix << "DOXYLATEX_DVI) \\\n"
                      "\t$(addprefix $(" << prefix << "DOXYLATEX_TEX:tex=),out aux log)";
   doxyRulesStream << "\n\n";

   doxyRulesStream <<
                   "$(" << prefix << "DOXYRULES) \\\n"
                   "$(" << prefix << "DOXYMAKEFILE) \\\n"
                   "$(" << prefix << "DOXYSTRUCTURE_PM) \\\n"
                   "$(" << prefix << "DOXYDOCS_PM)";

   if (perlmodLatex) {
      doxyRulesStream <<
                      " \\\n"
                      "$(" << prefix << "DOXYLATEX_PL) \\\n"
                      "$(" << prefix << "DOXYLATEXSTRUCTURE_PL) \\\n"
                      "$(" << prefix << "DOXYFORMAT_TEX) \\\n"
                      "$(" << prefix << "DOXYLATEX_TEX)";
   }

   doxyRulesStream <<
                   ": \\\n"
                   "\t$(" << prefix << "DOXYFILE)\n"
                   "\tcd $(" << prefix << "DOXY_EXEC_PATH) ; doxypress \"$<\"\n";

   if (perlmodLatex) {
      doxyRulesStream <<
                      "\n"
                      "$(" << prefix << "DOXYDOCS_TEX): \\\n"
                      "$(" << prefix << "DOXYLATEX_PL) \\\n"
                      "$(" << prefix << "DOXYDOCS_PM)\n"
                      "\tperl -I\"$(<D)\" \"$<\" >\"$@\"\n"
                      "\n"
                      "$(" << prefix << "DOXYSTRUCTURE_TEX): \\\n"
                      "$(" << prefix << "DOXYLATEXSTRUCTURE_PL) \\\n"
                      "$(" << prefix << "DOXYSTRUCTURE_PM)\n"
                      "\tperl -I\"$(<D)\" \"$<\" >\"$@\"\n"
                      "\n"
                      "$(" << prefix << "DOXYLATEX_PDF) \\\n"
                      "$(" << prefix << "DOXYLATEX_DVI): \\\n"
                      "$(" << prefix << "DOXYLATEX_TEX) \\\n"
                      "$(" << prefix << "DOXYFORMAT_TEX) \\\n"
                      "$(" << prefix << "DOXYSTRUCTURE_TEX) \\\n"
                      "$(" << prefix << "DOXYDOCS_TEX)\n"
                      "\n"
                      "$(" << prefix << "DOXYLATEX_PDF): \\\n"
                      "$(" << prefix << "DOXYLATEX_TEX)\n"
                      "\tpdflatex -interaction=nonstopmode \"$<\"\n"
                      "\n"
                      "$(" << prefix << "DOXYLATEX_DVI): \\\n"
                      "$(" << prefix << "DOXYLATEX_TEX)\n"
                      "\tlatex -interaction=nonstopmode \"$<\"\n";
   }

   return true;
}

bool PerlModGenerator::generateMakefile()
{
   QFile makefile;
   if (!createOutputFile(makefile, pathMakefile)) {
      return false;
   }

   bool perlmodLatex = Config::getBool("perl-latex");
   QString prefix = Config::getString("perl-prefix");

   QTextStream makefileStream(&makefile);

   makefileStream <<
                  ".PHONY: default clean" << (perlmodLatex ? " pdf" : "") << "\n"
                  "default: " << (perlmodLatex ? "pdf" : "clean") << "\n"
                  "\n"
                  "include " << pathDoxyRules << "\n"
                  "\n"
                  "clean: clean-perlmod\n";

   if (perlmodLatex) {
      makefileStream <<
                     "pdf: $(" << prefix << "DOXYLATEX_PDF)\n"
                     "dvi: $(" << prefix << "DOXYLATEX_DVI)\n";
   }

   return true;
}

bool PerlModGenerator::generateDoxyLatexStructurePL()
{
   QFile doxyLatexStructurePL;
   if (!createOutputFile(doxyLatexStructurePL, pathDoxyLatexStructurePL)) {
      return false;
   }

   QTextStream doxyLatexStructurePLStream(&doxyLatexStructurePL);
   doxyLatexStructurePLStream <<
                              "use DoxyStructure;\n"
                              "\n"
                              "sub process($) {\n"
                              "\tmy $node = $_[0];\n"
                              "\tmy ($type, $name) = @$node[0, 1];\n"
                              "\tmy $command;\n"
                              "\tif ($type eq \"string\") { $command = \"String\" }\n"
                              "\telsif ($type eq \"doc\") { $command = \"Doc\" }\n"
                              "\telsif ($type eq \"hash\") {\n"
                              "\t\t$command = \"Hash\";\n"
                              "\t\tfor my $subnode (values %{$$node[2]}) {\n"
                              "\t\t\tprocess($subnode);\n"
                              "\t\t}\n"
                              "\t}\n"
                              "\telsif ($type eq \"list\") {\n"
                              "\t\t$command = \"List\";\n"
                              "\t\tprocess($$node[2]);\n"
                              "\t}\n"
                              "\tprint \"\\\\\" . $command . \"Node{\" . $name . \"}%\\n\";\n"
                              "}\n"
                              "\n"
                              "process($doxystructure);\n";

   return true;
}

bool PerlModGenerator::generateDoxyLatexPL()
{
   QFile doxyLatexPL;
   if (!createOutputFile(doxyLatexPL, pathDoxyLatexPL)) {
      return false;
   }

   QTextStream doxyLatexPLStream(&doxyLatexPL);
   doxyLatexPLStream <<
                     "use DoxyStructure;\n"
                     "use DoxyDocs;\n"
                     "\n"
                     "sub latex_quote($) {\n"
                     "\tmy $text = $_[0];\n"
                     "\t$text =~ s/\\\\/\\\\textbackslash /g;\n"
                     "\t$text =~ s/\\|/\\\\textbar /g;\n"
                     "\t$text =~ s/</\\\\textless /g;\n"
                     "\t$text =~ s/>/\\\\textgreater /g;\n"
                     "\t$text =~ s/~/\\\\textasciitilde /g;\n"
                     "\t$text =~ s/\\^/\\\\textasciicircum /g;\n"
                     "\t$text =~ s/[\\$&%#_{}]/\\\\$&/g;\n"
                     "\tprint $text;\n"
                     "}\n"
                     "\n"
                     "sub generate_doc($) {\n"
                     "\tmy $doc = $_[0];\n"
                     "\tfor my $item (@$doc) {\n"
                     "\t\tmy $type = $$item{type};\n"
                     "\t\tif ($type eq \"text\") {\n"
                     "\t\t\tlatex_quote($$item{content});\n"
                     "\t\t} elsif ($type eq \"parbreak\") {\n"
                     "\t\t\tprint \"\\n\\n\";\n"
                     "\t\t} elsif ($type eq \"style\") {\n"
                     "\t\t\tmy $style = $$item{style};\n"
                     "\t\t\tif ($$item{enable} eq \"yes\") {\n"
                     "\t\t\t\tif ($style eq \"bold\") { print '\\bfseries'; }\n"
                     "\t\t\t\tif ($style eq \"italic\") { print '\\itshape'; }\n"
                     "\t\t\t\tif ($style eq \"code\") { print '\\ttfamily'; }\n"
                     "\t\t\t} else {\n"
                     "\t\t\t\tif ($style eq \"bold\") { print '\\mdseries'; }\n"
                     "\t\t\t\tif ($style eq \"italic\") { print '\\upshape'; }\n"
                     "\t\t\t\tif ($style eq \"code\") { print '\\rmfamily'; }\n"
                     "\t\t\t}\n"
                     "\t\t\tprint '{}';\n"
                     "\t\t} elsif ($type eq \"symbol\") {\n"
                     "\t\t\tmy $symbol = $$item{symbol};\n"
                     "\t\t\tif ($symbol eq \"copyright\") { print '\\copyright'; }\n"
                     "\t\t\telsif ($symbol eq \"szlig\") { print '\\ss'; }\n"
                     "\t\t\tprint '{}';\n"
                     "\t\t} elsif ($type eq \"accent\") {\n"
                     "\t\t\tmy ($accent) = $$item{accent};\n"
                     "\t\t\tif ($accent eq \"umlaut\") { print '\\\"'; }\n"
                     "\t\t\telsif ($accent eq \"acute\") { print '\\\\\\''; }\n"
                     "\t\t\telsif ($accent eq \"grave\") { print '\\`'; }\n"
                     "\t\t\telsif ($accent eq \"circ\") { print '\\^'; }\n"
                     "\t\t\telsif ($accent eq \"tilde\") { print '\\~'; }\n"
                     "\t\t\telsif ($accent eq \"cedilla\") { print '\\c'; }\n"
                     "\t\t\telsif ($accent eq \"ring\") { print '\\r'; }\n"
                     "\t\t\tprint \"{\" . $$item{letter} . \"}\"; \n"
                     "\t\t} elsif ($type eq \"list\") {\n"
                     "\t\t\tmy $env = ($$item{style} eq \"ordered\") ? \"enumerate\" : \"itemize\";\n"
                     "\t\t\tprint \"\\n\\\\begin{\" . $env .\"}\";\n"
                     "\t\t  \tfor my $subitem (@{$$item{content}}) {\n"
                     "\t\t\t\tprint \"\\n\\\\item \";\n"
                     "\t\t\t\tgenerate_doc($subitem);\n"
                     "\t\t  \t}\n"
                     "\t\t\tprint \"\\n\\\\end{\" . $env .\"}\";\n"
                     "\t\t} elsif ($type eq \"url\") {\n"
                     "\t\t\tlatex_quote($$item{content});\n"
                     "\t\t}\n"
                     "\t}\n"
                     "}\n"
                     "\n"
                     "sub generate($$) {\n"
                     "\tmy ($item, $node) = @_;\n"
                     "\tmy ($type, $name) = @$node[0, 1];\n"
                     "\tif ($type eq \"string\") {\n"
                     "\t\tprint \"\\\\\" . $name . \"{\";\n"
                     "\t\tlatex_quote($item);\n"
                     "\t\tprint \"}\";\n"
                     "\t} elsif ($type eq \"doc\") {\n"
                     "\t\tif (@$item) {\n"
                     "\t\t\tprint \"\\\\\" . $name . \"{\";\n"
                     "\t\t\tgenerate_doc($item);\n"
                     "\t\t\tprint \"}%\\n\";\n"
                     "\t\t} else {\n"
                     "#\t\t\tprint \"\\\\\" . $name . \"Empty%\\n\";\n"
                     "\t\t}\n"
                     "\t} elsif ($type eq \"hash\") {\n"
                     "\t\tmy ($key, $value);\n"
                     "\t\twhile (($key, $subnode) = each %{$$node[2]}) {\n"
                     "\t\t\tmy $subname = $$subnode[1];\n"
                     "\t\t\tprint \"\\\\Defcs{field\" . $subname . \"}{\";\n"
                     "\t\t\tif ($$item{$key}) {\n"
                     "\t\t\t\tgenerate($$item{$key}, $subnode);\n"
                     "\t\t\t} else {\n"
                     "#\t\t\t\t\tprint \"\\\\\" . $subname . \"Empty%\\n\";\n"
                     "\t\t\t}\n"
                     "\t\t\tprint \"}%\\n\";\n"
                     "\t\t}\n"
                     "\t\tprint \"\\\\\" . $name . \"%\\n\";\n"
                     "\t} elsif ($type eq \"list\") {\n"
                     "\t\tmy $index = 0;\n"
                     "\t\tif (@$item) {\n"
                     "\t\t\tprint \"\\\\\" . $name . \"{%\\n\";\n"
                     "\t\t\tfor my $subitem (@$item) {\n"
                     "\t\t\t\tif ($index) {\n"
                     "\t\t\t\t\tprint \"\\\\\" . $name . \"Sep%\\n\";\n"
                     "\t\t\t\t}\n"
                     "\t\t\t\tgenerate($subitem, $$node[2]);\n"
                     "\t\t\t\t$index++;\n"
                     "\t\t\t}\n"
                     "\t\t\tprint \"}%\\n\";\n"
                     "\t\t} else {\n"
                     "#\t\t\tprint \"\\\\\" . $name . \"Empty%\\n\";\n"
                     "\t\t}\n"
                     "\t}\n"
                     "}\n"
                     "\n"
                     "generate($doxydocs, $doxystructure);\n";

   return true;
}

bool PerlModGenerator::generateDoxyFormatTex()
{
   QFile doxyFormatTex;
   if (!createOutputFile(doxyFormatTex, pathDoxyFormatTex)) {
      return false;
   }

   QTextStream doxyFormatTexStream(&doxyFormatTex);
   doxyFormatTexStream <<
                       "\\def\\Defcs#1{\\long\\expandafter\\def\\csname#1\\endcsname}\n"
                       "\\Defcs{Empty}{}\n"
                       "\\def\\IfEmpty#1{\\expandafter\\ifx\\csname#1\\endcsname\\Empty}\n"
                       "\n"
                       "\\def\\StringNode#1{\\Defcs{#1}##1{##1}}\n"
                       "\\def\\DocNode#1{\\Defcs{#1}##1{##1}}\n"
                       "\\def\\ListNode#1{\\Defcs{#1}##1{##1}\\Defcs{#1Sep}{}}\n"
                       "\\def\\HashNode#1{\\Defcs{#1}{}}\n"
                       "\n"
                       "\\input{" << pathDoxyStructureTex << "}\n"
                       "\n"
                       "\\newbox\\BoxA\n"
                       "\\dimendef\\DimenA=151\\relax\n"
                       "\\dimendef\\DimenB=152\\relax\n"
                       "\\countdef\\ZoneDepth=151\\relax\n"
                       "\n"
                       "\\def\\Cs#1{\\csname#1\\endcsname}\n"
                       "\\def\\Letcs#1{\\expandafter\\let\\csname#1\\endcsname}\n"
                       "\\def\\Heading#1{\\vskip 4mm\\relax\\textbf{#1}}\n"
                       "\\def\\See#1{\\begin{flushleft}\\Heading{See also: }#1\\end{flushleft}}\n"
                       "\n"
                       "\\def\\Frame#1{\\vskip 3mm\\relax\\fbox{ \\vbox{\\hsize0.95\\hsize\\vskip 1mm\\relax\n"
                       "\\raggedright#1\\vskip 0.5mm\\relax} }}\n"
                       "\n"
                       "\\def\\Zone#1#2#3{%\n"
                       "\\Defcs{Test#1}{#2}%\n"
                       "\\Defcs{Emit#1}{#3}%\n"
                       "\\Defcs{#1}{%\n"
                       "\\advance\\ZoneDepth1\\relax\n"
                       "\\Letcs{Mode\\number\\ZoneDepth}0\\relax\n"
                       "\\Letcs{Present\\number\\ZoneDepth}0\\relax\n"
                       "\\Cs{Test#1}\n"
                       "\\expandafter\\if\\Cs{Present\\number\\ZoneDepth}1%\n"
                       "\\advance\\ZoneDepth-1\\relax\n"
                       "\\Letcs{Present\\number\\ZoneDepth}1\\relax\n"
                       "\\expandafter\\if\\Cs{Mode\\number\\ZoneDepth}1%\n"
                       "\\advance\\ZoneDepth1\\relax\n"
                       "\\Letcs{Mode\\number\\ZoneDepth}1\\relax\n"
                       "\\Cs{Emit#1}\n"
                       "\\advance\\ZoneDepth-1\\relax\\fi\n"
                       "\\advance\\ZoneDepth1\\relax\\fi\n"
                       "\\advance\\ZoneDepth-1\\relax}}\n"
                       "\n"
                       "\\def\\Member#1#2{%\n"
                       "\\Defcs{Test#1}{\\Cs{field#1Detailed}\n"
                       "\\IfEmpty{field#1DetailedDoc}\\else\\Letcs{Present#1}1\\fi}\n"
                       "\\Defcs{#1}{\\Letcs{Present#1}0\\relax\n"
                       "\\Cs{Test#1}\\if1\\Cs{Present#1}\\Letcs{Present\\number\\ZoneDepth}1\\relax\n"
                       "\\if1\\Cs{Mode\\number\\ZoneDepth}#2\\fi\\fi}}\n"
                       "\n"
                       "\\def\\TypedefMemberList#1#2{%\n"
                       "\\Defcs{#1DetailedDoc}##1{\\vskip 5.5mm\\relax##1}%\n"
                       "\\Defcs{#1Name}##1{\\textbf{##1}}%\n"
                       "\\Defcs{#1See}##1{\\See{##1}}%\n"
                       "%\n"
                       "\\Zone{#1s}{\\Cs{field#1List}}{\\subsubsection{#2}\\Cs{field#1List}}%\n"
                       "\\Member{#1}{\\Frame{typedef \\Cs{field#1Type} \\Cs{field#1Name}}%\n"
                       "\\Cs{field#1DetailedDoc}\\Cs{field#1See}\\vskip 5mm\\relax}}%\n"
                       "\n"
                       "\\def\\VariableMemberList#1#2{%\n"
                       "\\Defcs{#1DetailedDoc}##1{\\vskip 5.5mm\\relax##1}%\n"
                       "\\Defcs{#1Name}##1{\\textbf{##1}}%\n"
                       "\\Defcs{#1See}##1{\\See{##1}}%\n"
                       "%\n"
                       "\\Zone{#1s}{\\Cs{field#1List}}{\\subsubsection{#2}\\Cs{field#1List}}%\n"
                       "\\Member{#1}{\\Frame{\\Cs{field#1Type}{} \\Cs{field#1Name}}%\n"
                       "\\Cs{field#1DetailedDoc}\\Cs{field#1See}\\vskip 5mm\\relax}}%\n"
                       "\n"
                       "\\def\\FunctionMemberList#1#2{%\n"
                       "\\Defcs{#1PDParamName}##1{\\textit{##1}}%\n"
                       "\\Defcs{#1PDParam}{\\Cs{field#1PDParamName}}%\n"
                       "\\Defcs{#1PDParamsSep}{, }%\n"
                       "\\Defcs{#1PDBlocksSep}{\\vskip 2mm\\relax}%\n"
                       "%\n"
                       "\\Defcs{#1PDBlocks}##1{%\n"
                       "\\Heading{Parameters:}\\vskip 1.5mm\\relax\n"
                       "\\DimenA0pt\\relax\n"
                       "\\Defcs{#1PDBlock}{\\setbox\\BoxA\\hbox{\\Cs{field#1PDParams}}%\n"
                       "\\ifdim\\DimenA<\\wd\\BoxA\\DimenA\\wd\\BoxA\\fi}%\n"
                       "##1%\n"
                       "\\advance\\DimenA3mm\\relax\n"
                       "\\DimenB\\hsize\\advance\\DimenB-\\DimenA\\relax\n"
                       "\\Defcs{#1PDBlock}{\\hbox to\\hsize{\\vtop{\\hsize\\DimenA\\relax\n"
                       "\\Cs{field#1PDParams}}\\hfill\n"
                       "\\vtop{\\hsize\\DimenB\\relax\\Cs{field#1PDDoc}}}}%\n"
                       "##1}\n"
                       "\n"
                       "\\Defcs{#1ParamName}##1{\\textit{##1}}\n"
                       "\\Defcs{#1Param}{\\Cs{field#1ParamType}{} \\Cs{field#1ParamName}}\n"
                       "\\Defcs{#1ParamsSep}{, }\n"
                       "\n"
                       "\\Defcs{#1Name}##1{\\textbf{##1}}\n"
                       "\\Defcs{#1See}##1{\\See{##1}}\n"
                       "\\Defcs{#1Return}##1{\\Heading{Returns: }##1}\n"
                       "\\Defcs{field#1Title}{\\Frame{\\Cs{field#1Type}{} \\Cs{field#1Name}(\\Cs{field#1Params})}}%\n"
                       "%\n"
                       "\\Zone{#1s}{\\Cs{field#1List}}{\\subsubsection{#2}\\Cs{field#1List}}%\n"
                       "\\Member{#1}{%\n"
                       "\\Cs{field#1Title}\\vskip 6mm\\relax\\Cs{field#1DetailedDoc}\n"
                       "\\Cs{field#1Return}\\Cs{field#1PDBlocks}\\Cs{field#1See}\\vskip 5mm\\relax}}\n"
                       "\n"
                       "\\def\\FileDetailed{\\fieldFileDetailedDoc\\par}\n"
                       "\\def\\ClassDetailed{\\fieldClassDetailedDoc\\par}\n"
                       "\n"
                       "\\def\\FileSubzones{\\fieldFileTypedefs\\fieldFileVariables\\fieldFileFunctions}\n"
                       "\n"
                       "\\def\\ClassSubzones{%\n"
                       "\\fieldClassPublicTypedefs\\fieldClassPublicMembers\\fieldClassPublicMethods\n"
                       "\\fieldClassProtectedTypedefs\\fieldClassProtectedMembers\\fieldClassProtectedMethods\n"
                       "\\fieldClassPrivateTypedefs\\fieldClassPrivateMembers\\fieldClassPrivateMethods}\n"
                       "\n"
                       "\\Member{Page}{\\subsection{\\fieldPageName}\\fieldPageDetailedDoc}\n"
                       "\n"
                       "\\TypedefMemberList{FileTypedef}{Typedefs}\n"
                       "\\VariableMemberList{FileVariable}{Variables}\n"
                       "\\FunctionMemberList{FileFunction}{Functions}\n"
                       "\\Zone{File}{\\FileSubzones}{\\subsection{\\fieldFileName}\\fieldFileDetailed\\FileSubzones}\n"
                       "\n"
                       "\\TypedefMemberList{ClassPublicTypedef}{Public Typedefs}\n"
                       "\\TypedefMemberList{ClassProtectedTypedef}{Protected Typedefs}\n"
                       "\\TypedefMemberList{ClassPrivateTypedef}{Private Typedefs}\n"
                       "\\VariableMemberList{ClassPublicMember}{Public Members}\n"
                       "\\VariableMemberList{ClassProtectedMember}{Protected Members}\n"
                       "\\VariableMemberList{ClassPrivateMember}{Private Members}\n"
                       "\\FunctionMemberList{ClassPublicMethod}{Public Methods}\n"
                       "\\FunctionMemberList{ClassProtectedMethod}{Protected Methods}\n"
                       "\\FunctionMemberList{ClassPrivateMethod}{Private Methods}\n"
                       "\\Zone{Class}{\\ClassSubzones}{\\subsection{\\fieldClassName}\\fieldClassDetailed\\ClassSubzones}\n"
                       "\n"
                       "\\Zone{AllPages}{\\fieldPages}{\\section{Pages}\\fieldPages}\n"
                       "\\Zone{AllFiles}{\\fieldFiles}{\\section{Files}\\fieldFiles}\n"
                       "\\Zone{AllClasses}{\\fieldClasses}{\\section{Classes}\\fieldClasses}\n"
                       "\n"
                       "\\newlength{\\oldparskip}\n"
                       "\\newlength{\\oldparindent}\n"
                       "\\newlength{\\oldfboxrule}\n"
                       "\n"
                       "\\ZoneDepth0\\relax\n"
                       "\\Letcs{Mode0}1\\relax\n"
                       "\n"
                       "\\def\\EmitDoxyDocs{%\n"
                       "\\setlength{\\oldparskip}{\\parskip}\n"
                       "\\setlength{\\oldparindent}{\\parindent}\n"
                       "\\setlength{\\oldfboxrule}{\\fboxrule}\n"
                       "\\setlength{\\parskip}{0cm}\n"
                       "\\setlength{\\parindent}{0cm}\n"
                       "\\setlength{\\fboxrule}{1pt}\n"
                       "\\AllPages\\AllFiles\\AllClasses\n"
                       "\\setlength{\\parskip}{\\oldparskip}\n"
                       "\\setlength{\\parindent}{\\oldparindent}\n"
                       "\\setlength{\\fboxrule}{\\oldfboxrule}}\n";

   return true;
}

bool PerlModGenerator::generateDoxyLatexTex()
{
   QFile doxyLatexTex;
   if (!createOutputFile(doxyLatexTex, pathDoxyLatexTex)) {
      return false;
   }

   QTextStream doxyLatexTexStream(&doxyLatexTex);
   doxyLatexTexStream <<
                      "\\documentclass[a4paper,12pt]{article}\n"
                      "\\usepackage[latin1]{inputenc}\n"
                      "\\usepackage[none]{hyphenat}\n"
                      "\\usepackage[T1]{fontenc}\n"
                      "\\usepackage{hyperref}\n"
                      "\\usepackage{times}\n"
                      "\n"
                      "\\input{doxyformat}\n"
                      "\n"
                      "\\begin{document}\n"
                      "\\input{" << pathDoxyDocsTex << "}\n"
                      "\\sloppy\n"
                      "\\EmitDoxyDocs\n"
                      "\\end{document}\n";

   return true;
}

void PerlModGenerator::generate()
{
   // + classes
   // + namespaces
   // + files
   // - packages
   // + groups
   // + related pages
   // - examples

   QDir perlModDir;
   if (! createOutputDir(perlModDir)) {
      return;
   }

   bool perlmodLatex = Config::getBool("perl-latex");

   QString perlModAbsPath = perlModDir.absolutePath();

   pathDoxyDocsPM         = perlModAbsPath + "/DoxyDocs.pm";
   pathDoxyStructurePM    = perlModAbsPath + "/DoxyStructure.pm";
   pathMakefile           = perlModAbsPath + "/Makefile";
   pathDoxyRules          = perlModAbsPath + "/doxyrules.make";

   if (perlmodLatex) {
      pathDoxyStructureTex     = perlModAbsPath + "/doxystructure.tex";
      pathDoxyFormatTex        = perlModAbsPath + "/doxyformat.tex";
      pathDoxyLatexTex         = perlModAbsPath + "/doxylatex.tex";
      pathDoxyLatexDVI         = perlModAbsPath + "/doxylatex.dvi";
      pathDoxyLatexPDF         = perlModAbsPath + "/doxylatex.pdf";
      pathDoxyDocsTex          = perlModAbsPath + "/doxydocs.tex";
      pathDoxyLatexPL          = perlModAbsPath + "/doxylatex.pl";
      pathDoxyLatexStructurePL = perlModAbsPath + "/doxylatex-structure.pl";
   }

   if (! (generatePerlModOutput() && generateDoxyStructurePM() && generateMakefile() && generateDoxyRules())) {
      return;
   }

   if (perlmodLatex) {
      if (! (generateDoxyLatexStructurePL() && generateDoxyLatexPL() && generateDoxyLatexTex() && generateDoxyFormatTex())) {
         return;
      }
   }
}

void generatePerl_output()
{
   PerlModGenerator pmg(Config::getBool("perl-pretty"));
   pmg.generate();
}
