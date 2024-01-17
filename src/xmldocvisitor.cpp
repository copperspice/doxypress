/************************************************************************
*
* Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
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

#include <QFileInfo>

#include <xmldocvisitor.h>

#include <config.h>
#include <docparser.h>
#include <dot.h>
#include <doxy_globals.h>
#include <emoji_entity.h>
#include <htmlentity.h>
#include <language.h>
#include <message.h>
#include <outputgen.h>
#include <parse_base.h>
#include <util.h>
#include <xmlgen.h>

static void visitCaption(XmlDocVisitor *parent, QList<DocNode *> children)
{
   for (const auto &n : children) {
      n->accept(parent);
   }
}

static void visitPreStart(QTextStream &t, const QString &cmd, const bool doCaption, XmlDocVisitor *parent,
                  QList<DocNode *> children, const QString &name, bool writeType, DocImage::Type type,
                  const QString &width, const QString &height, bool inlineImage = false)
{
   t << "<" << cmd;

   if (writeType) {
      t << " type=\"";

      switch(type)     {

         case DocImage::DocBook:
            t << "docbook";
            break;

         case DocImage::Html:
            t << "html";
            break;

         case DocImage::Latex:
            t << "latex";
            break;

         case DocImage::Rtf:
            t << "rtf";
            break;

         case DocImage::Xml:
            t << "xml";
            break;

         case DocImage::None:
            break;
      }

      t << "\"";
   }

   if (! name.isEmpty()) {
      t << " name=\"" << convertToXML(name, true) << "\"";
   }

   if (! width.isEmpty()) {
      t << " width=\"" << convertToXML(width, false) << "\"";
   }

   if (! height.isEmpty()) {
      t << " height=\"" << convertToXML(height, false) << "\"";
   }
   if (inlineImage) {
      t << " inline=\"yes\"";
   }
   if (doCaption) {
      t << " caption=\"";
      visitCaption(parent, children);
      t << "\"";
   }

   t << ">";
}

static void visitPostEnd(QTextStream &t, const QString &cmd)
{
   t << "</" << cmd << ">" << endl;
}

XmlDocVisitor::XmlDocVisitor(QTextStream &t, CodeGenerator &ci)
   : DocVisitor(DocVisitor_XML), m_t(t), m_ci(ci), m_insidePre(false), m_hide(false)
{
}


void XmlDocVisitor::visit(DocWord *w)
{
   if (m_hide) {
      return;
   }

   filter(w->word());
}

void XmlDocVisitor::visit(DocLinkedWord *w)
{
   if (m_hide) {
      return;
   }

   startLink(w->ref(), w->file(), w->anchor());
   filter(w->word());
   endLink();
}

void XmlDocVisitor::visit(DocWhiteSpace *w)
{
   if (m_hide) {
      return;
   }

   if (m_insidePre) {
      m_t << w->chars();
   } else {
      m_t << " ";
   }
}

void XmlDocVisitor::visit(DocSymbol *s)
{
   if (m_hide) {
      return;
   }

   QString res = HtmlEntityMapper::instance()->xml(s->symbol());

   if (! res.isEmpty()) {
      m_t << res;
   } else {
      err("XML, Unsupported HTML entity found: %s\n", csPrintable(HtmlEntityMapper::instance()->html(s->symbol(), true)) );
   }
}

void XmlDocVisitor::visit(DocEmoji *s)
{
   if (m_hide) {
      return;
   }

   QString result = EmojiEntityMapper::instance()->name(s->index());

   if (! result.isEmpty()) {

      result = result.mid(1, result.length() - 2);
      m_t << "<emoji name=\"" << result << "\" unicode=\"";

      filter(EmojiEntityMapper::instance()->unicode(s->index()));
      m_t << "\"/>";

   } else {
      m_t << s->name();
   }

}

void XmlDocVisitor::visit(DocURL *u)
{
   if (m_hide) {
      return;
   }

   m_t << "<ulink url=\"";
   if (u->isEmail()) {
      m_t << "mailto:";
   }

   filter(u->url());
   m_t << "\">";

   filter(u->url());
   m_t << "</ulink>";
}

void XmlDocVisitor::visit(DocLineBreak *)
{
   if (m_hide) {
      return;
   }
   m_t << "<linebreak/>\n";
}

void XmlDocVisitor::visit(DocHorRuler *)
{
   if (m_hide) {
      return;
   }
   m_t << "<hruler/>\n";
}

void XmlDocVisitor::visit(DocStyleChange *s)
{
   if (m_hide) {
      return;
   }
   switch (s->style()) {
      case DocStyleChange::Bold:
         if (s->enable()) {
            m_t << "<bold>";
         } else {
            m_t << "</bold>";
         }
         break;

      case DocStyleChange::S:
         if (s->enable()) {
            m_t << "<s>";
         } else {
            m_t << "</s>";
         }
         break;

      case DocStyleChange::Strike:
         if (s->enable()) {
            m_t << "<strike>";
         } else {
            m_t << "</strike>";
         }
         break;

      case DocStyleChange::Del:
         if (s->enable()) {
            m_t << "<del>";
         } else {
            m_t << "</del>";
         }
         break;

      case DocStyleChange::Underline:
         if (s->enable())  {
            m_t << "<underline>";
         } else {
            m_t << "</underline>";
         }
         break;

      case DocStyleChange::Ins:
         if (s->enable()) {
            m_t << "<ins>";
         } else  {
            m_t << "</ins>";
         }
         break;

      case DocStyleChange::Italic:
         if (s->enable()) {
            m_t << "<emphasis>";
         }     else {
            m_t << "</emphasis>";
         }
         break;

      case DocStyleChange::Code:
         if (s->enable()) {
            m_t << "<computeroutput>";
         }   else {
            m_t << "</computeroutput>";
         }
         break;

      case DocStyleChange::Subscript:
         if (s->enable()) {
            m_t << "<subscript>";
         }    else {
            m_t << "</subscript>";
         }
         break;

      case DocStyleChange::Superscript:
         if (s->enable()) {
            m_t << "<superscript>";
         }    else {
            m_t << "</superscript>";
         }
         break;

      case DocStyleChange::Center:
         if (s->enable()) {
            m_t << "<center>";
         } else {
            m_t << "</center>";
         }
         break;

      case DocStyleChange::Small:
         if (s->enable()) {
            m_t << "<small>";
         }  else {
            m_t << "</small>";
         }
         break;

      case DocStyleChange::Cite:
         if (s->enable()) {
            m_t << "<cite>";
         } else {
            m_t << "</cite>";
         }
         break;

      case DocStyleChange::Preformatted:
         if (s->enable()) {
            m_t << "<preformatted>";
            m_insidePre = true;
         } else {
            m_t << "</preformatted>";
            m_insidePre = false;
         }
         break;

      case DocStyleChange::Div:
      case DocStyleChange::Span:
         /// HTML only
         break;

      case DocStyleChange::Details:
         if (s->enable()) {
            m_t << "<details>";
         } else {
            m_t << "</details>";
         }
         break;

      case DocStyleChange::Summary:
         if (s->enable()) {
            m_t << "<summary>";
         } else {
            m_t << "</summary>";
         }
         break;
   }
}

void XmlDocVisitor::visit(DocVerbatim *s)
{
   if (m_hide) {
      return;
   }

   QString lang = m_langExt;
   if (! s->language().isEmpty()) {
      // explicit language setting
      lang = s->language();
   }

   SrcLangExt langExt = getLanguageFromFileName(lang);

   switch (s->type()) {
      case DocVerbatim::Code:
         m_t << "<programlisting";

         if (! s->language().isEmpty()) {
            m_t << " filename=\"" << lang << "\">";
         } else {
          m_t << ">";
         }

         Doxy_Globals::parserManager.getParser(lang)->parseCode(m_ci, s->context(), s->text(),
                   langExt, s->isExample(), s->exampleFile());

         m_t << "</programlisting>";
         break;

      case DocVerbatim::Verbatim:
         m_t << "<verbatim>";
         filter(s->text());
         m_t << "</verbatim>";
         break;

      case DocVerbatim::HtmlOnly:
      case DocVerbatim::RtfOnly:
      case DocVerbatim::ManOnly:
      case DocVerbatim::LatexOnly:
      case DocVerbatim::DocbookOnly:
         // do nothing
         break;

      case DocVerbatim::XmlOnly:
         m_t << s->text();
         break;

      case DocVerbatim::Dot:
         visitPreStart(m_t, "dot", s->hasCaption(), this, s->children(), QString(), false,
                     DocImage::Html, s->width(), s->height());
         filter(s->text());
         visitPostEnd(m_t, "dot");
         break;

      case DocVerbatim::Msc:
         visitPreStart(m_t, "msc", s->hasCaption(), this, s->children(), QString(), false,
                     DocImage::Html, s->width(), s->height());
         filter(s->text());
         visitPostEnd(m_t, "msc");
         break;

      case DocVerbatim::PlantUML:
         visitPreStart(m_t, "plantuml", s->hasCaption(), this, s->children(), QString(), false,
                     DocImage::Html, s->width(), s->height());
         filter(s->text());
         visitPostEnd(m_t, "plantuml");
         break;
   }
}

void XmlDocVisitor::visit(DocAnchor *anc)
{
   if (m_hide) {
      return;
   }
   m_t << "<anchor id=\"" << anc->file() << "_1" << anc->anchor() << "\"/>";
}

void XmlDocVisitor::visit(DocInclude *inc)
{
   if (m_hide) {
      return;
   }

   SrcLangExt langExt = getLanguageFromFileName(inc->extension());

   switch (inc->type()) {
      case DocInclude::IncWithLines: {
         m_t << "<programlisting filename=\"" << inc->file() << "\">";

         QFileInfo cfi( inc->file() );
         QSharedPointer<FileDef> fd = QMakeShared<FileDef>(cfi.path(), cfi.fileName());

         Doxy_Globals::parserManager.getParser(inc->extension())->parseCode(m_ci, inc->context(),
                     inc->text(), langExt, inc->isExample(), inc->exampleFile(), fd,
                     -1, -1, false, QSharedPointer<MemberDef>(), true);

         m_t << "</programlisting>";
      }
      break;

      case DocInclude::Include:
         m_t << "<programlisting filename=\"" << inc->file() << "\">";

         Doxy_Globals::parserManager.getParser(inc->extension())->parseCode(m_ci, inc->context(),
                     inc->text(), langExt, inc->isExample(), inc->exampleFile(), QSharedPointer<FileDef>(),
                     -1, -1, true, QSharedPointer<MemberDef>(), false);

         m_t << "</programlisting>";
         break;

      case DocInclude::DontInclude:
      case DocInclude::DontIncWithLines:
         break;

      case DocInclude::HtmlInclude:
         if (inc->isBlock()) {
           m_t << "<htmlonly block=\"yes\">";
         } else {
            m_t << "<htmlonly>";
         }

         filter(inc->text());
         m_t << "</htmlonly>";
         break;

      case DocInclude::LatexInclude:
         m_t << "<latexonly>";
         filter(inc->text());
         m_t << "</latexonly>";
         break;

      case DocInclude::RtfInclude:
         m_t << "<rtfonly>";
         filter(inc->text());
         m_t << "</rtfonly>";
         break;

      case DocInclude::ManInclude:
         m_t << "<manonly>";
         filter(inc->text());
         m_t << "</manonly>";
         break;

      case DocInclude::XmlInclude:
         filter(inc->text());
         break;

      case DocInclude::DocbookInclude:
         m_t << "<docbookonly>";
         filter(inc->text());
         m_t << "</docbookonly>";
         break;

      case DocInclude::VerbInclude:
         m_t << "<verbatim>";
         filter(inc->text());
         m_t << "</verbatim>";
         break;

      case DocInclude::Snippet:
         m_t << "<programlisting filename=\"" << inc->file() << "\">";

         Doxy_Globals::parserManager.getParser(inc->extension())->parseCode(m_ci, inc->context(),
                  extractBlock(inc->text(), inc->blockId()), langExt,
                  inc->isExample(), inc->exampleFile());

         m_t << "</programlisting>";
         break;

      case DocInclude::SnipWithLines: {
         m_t << "<programlisting filename=\"" << inc->file() << "\">";

         QFileInfo cfi( inc->file() );
         QSharedPointer<FileDef> fd = QMakeShared<FileDef>(cfi.path(), cfi.fileName());

         Doxy_Globals::parserManager.getParser(inc->extension())->parseCode(m_ci, inc->context(),
                  extractBlock(inc->text(), inc->blockId()), langExt,
                  inc->isExample(), inc->exampleFile(), fd,
                  lineBlock(inc->text(), inc->blockId()), -1, false, QSharedPointer<MemberDef>(), true);

         m_t << "</programlisting>";
      }
      break;

      case DocInclude::IncludeDoc:
      case DocInclude::SnippetDoc:
         err("Unexpected command found for IncludeDoc or SnippetDoc in file: %s,"
               " contact the developers\n", csPrintable(inc->file()));
         break;
   }
}

void XmlDocVisitor::visit(DocIncOperator *op)
{

   if (op->isFirst()) {
      if (! m_hide) {
          m_t << "<programlisting filename=\"" << op->includeFileName() << "\">";
      }

      pushEnabled();
      m_hide = true;
   }

   QString fileExt = getFileNameExtension(op->includeFileName());

   if (fileExt.isEmpty()) {
      fileExt = m_langExt;
   }

   SrcLangExt srcLangExt = getLanguageFromFileName(fileExt);

   if (op->type() != DocIncOperator::Skip) {
      popEnabled();
      if (! m_hide) {
         Doxy_Globals::parserManager.getParser(fileExt)->parseCode(m_ci, op->context(),
                     op->text(), srcLangExt, op->isExample(), op->exampleFile());
      }
      pushEnabled();
      m_hide = true;
   }

   if (op->isLast()) {
      popEnabled();
      if (! m_hide) {
         m_t << "</programlisting>";
      }

   } else {
      if (! m_hide) {
         m_t << endl;
      }
   }
}

void XmlDocVisitor::visit(DocFormula *f)
{
   if (m_hide) {
      return;
   }

   m_t << "<formula id=\"" << f->id() << "\">";
   filter(f->text());
   m_t << "</formula>";
}

void XmlDocVisitor::visit(DocIndexEntry *ie)
{
   if (m_hide) {
      return;
   }

   m_t << "<indexentry>"
          "<primaryie>";

   filter(ie->entry());

   m_t << "</primaryie>"
          "<secondaryie></secondaryie>"
          "</indexentry>";
}

void XmlDocVisitor::visit(DocSimpleSectSep *sep)
{
   if (sep->parent() && sep->parent()->kind() == DocNode::Kind_SimpleSect) {
      visitPost((DocSimpleSect *)sep->parent()); // end current section
      visitPre((DocSimpleSect *)sep->parent());  // start new section
   }
}

void XmlDocVisitor::visit(DocCite *cite)
{
   if (m_hide) {
      return;
   }

   if (!cite->file().isEmpty()) {
      startLink(cite->ref(), cite->file(), cite->anchor());
   }

   filter(cite->text());

   if (!cite->file().isEmpty()) {
      endLink();
   }
}

//--------------------------------------
// visitor functions for compound nodes
//--------------------------------------

void XmlDocVisitor::visitPre(DocAutoList *l)
{
   if (m_hide) {
      return;
   }
   if (l->isEnumList()) {
      m_t << "<orderedlist>\n";
   } else {
      m_t << "<itemizedlist>\n";
   }
}

void XmlDocVisitor::visitPost(DocAutoList *l)
{
   if (m_hide) {
      return;
   }
   if (l->isEnumList()) {
      m_t << "</orderedlist>\n";
   } else {
      m_t << "</itemizedlist>\n";
   }
}

void XmlDocVisitor::visitPre(DocAutoListItem *)
{
   if (m_hide) {
      return;
   }

   m_t << "<listitem>";
}

void XmlDocVisitor::visitPost(DocAutoListItem *)
{
   if (m_hide) {
      return;
   }

   m_t << "</listitem>";
}

void XmlDocVisitor::visitPre(DocPara *)
{
   if (m_hide) {
      return;
   }

   m_t << "<para>";
}

void XmlDocVisitor::visitPost(DocPara *)
{
   if (m_hide) {
      return;
   }

   m_t << "</para>\n";
}

void XmlDocVisitor::visitPre(DocRoot *)
{
   //m_t << "<hr><h4><font color=\"red\">New parser:</font></h4>\n";
}

void XmlDocVisitor::visitPost(DocRoot *)
{
   //m_t << "<hr><h4><font color=\"red\">Old parser:</font></h4>\n";
}

void XmlDocVisitor::visitPre(DocSimpleSect *s)
{
   if (m_hide) {
      return;
   }

   m_t << "<simplesect kind=\"";

   switch (s->type()) {
      case DocSimpleSect::See:
         m_t << "see";
         break;
      case DocSimpleSect::Return:
         m_t << "return";
         break;
      case DocSimpleSect::Author:
         m_t << "author";
         break;
      case DocSimpleSect::Authors:
         m_t << "authors";
         break;
      case DocSimpleSect::Version:
         m_t << "version";
         break;
      case DocSimpleSect::Since:
         m_t << "since";
         break;
      case DocSimpleSect::Date:
         m_t << "date";
         break;
      case DocSimpleSect::Note:
         m_t << "note";
         break;
      case DocSimpleSect::Warning:
         m_t << "warning";
         break;
      case DocSimpleSect::Pre:
         m_t << "pre";
         break;
      case DocSimpleSect::Post:
         m_t << "post";
         break;
      case DocSimpleSect::Copyright:
         m_t << "copyright";
         break;
      case DocSimpleSect::Invar:
         m_t << "invariant";
         break;
      case DocSimpleSect::Remark:
         m_t << "remark";
         break;
      case DocSimpleSect::Attention:
         m_t << "attention";
         break;
      case DocSimpleSect::User:
         m_t << "par";
         break;
      case DocSimpleSect::Rcs:
         m_t << "rcs";
         break;
      case DocSimpleSect::Unknown:
         break;
   }
   m_t << "\">";
}

void XmlDocVisitor::visitPost(DocSimpleSect *)
{
   if (m_hide) {
      return;
   }
   m_t << "</simplesect>\n";
}

void XmlDocVisitor::visitPre(DocTitle *)
{
   if (m_hide) {
      return;
   }
   m_t << "<title>";
}

void XmlDocVisitor::visitPost(DocTitle *)
{
   if (m_hide) {
      return;
   }
   m_t << "</title>";
}

void XmlDocVisitor::visitPre(DocSimpleList *)
{
   if (m_hide) {
      return;
   }
   m_t << "<itemizedlist>\n";
}

void XmlDocVisitor::visitPost(DocSimpleList *)
{
   if (m_hide) {
      return;
   }
   m_t << "</itemizedlist>\n";
}

void XmlDocVisitor::visitPre(DocSimpleListItem *)
{
   if (m_hide) {
      return;
   }
   m_t << "<listitem>";
}

void XmlDocVisitor::visitPost(DocSimpleListItem *)
{
   if (m_hide) {
      return;
   }
   m_t << "</listitem>\n";
}

void XmlDocVisitor::visitPre(DocSection *s)
{
   if (m_hide) {
      return;
   }

   m_t << "<sect" << s->level() << " id=\"" << s->file();

   if (! s->anchor().isEmpty()) {
      m_t << "_1" << s->anchor();
   }

   m_t << "\">\n";
   m_t << "<title>";

   filter(convertCharEntities(s->title()));
   m_t << "</title>\n";
}

void XmlDocVisitor::visitPost(DocSection *s)
{
   m_t << "</sect" << s->level() << ">" << endl;
}

void XmlDocVisitor::visitPre(DocHtmlList *s)
{
   if (m_hide) {
      return;
   }

   if (s->type() == DocHtmlList::Ordered) {
      m_t << "<orderedlist>\n";
   } else {
      m_t << "<itemizedlist>\n";
   }
}

void XmlDocVisitor::visitPost(DocHtmlList *s)
{
   if (m_hide) {
      return;
   }
   if (s->type() == DocHtmlList::Ordered) {
      m_t << "</orderedlist>\n";
   } else {
      m_t << "</itemizedlist>\n";
   }
}

void XmlDocVisitor::visitPre(DocHtmlListItem *)
{
   if (m_hide) {
      return;
   }

   m_t << "<listitem>\n";
}

void XmlDocVisitor::visitPost(DocHtmlListItem *)
{
   if (m_hide) {
      return;
   }
   m_t << "</listitem>\n";
}

void XmlDocVisitor::visitPre(DocHtmlDescList *)
{
   if (m_hide) {
      return;
   }
   m_t << "<variablelist>\n";
}

void XmlDocVisitor::visitPost(DocHtmlDescList *)
{
   if (m_hide) {
      return;
   }
   m_t << "</variablelist>\n";
}

void XmlDocVisitor::visitPre(DocHtmlDescTitle *)
{
   if (m_hide) {
      return;
   }

   m_t << "<varlistentry><term>";
}

void XmlDocVisitor::visitPost(DocHtmlDescTitle *)
{
   if (m_hide) {
      return;
   }

   m_t << "</term></varlistentry>\n";
}

void XmlDocVisitor::visitPre(DocHtmlDescData *)
{
   if (m_hide) {
      return;
   }

   m_t << "<listitem>";
}

void XmlDocVisitor::visitPost(DocHtmlDescData *)
{
   if (m_hide) {
      return;
   }

   m_t << "</listitem>\n";
}

void XmlDocVisitor::visitPre(DocHtmlTable *t)
{
   if (m_hide) {
      return;
   }

   m_t << "<table rows=\"" << t->numRows()
       << "\" cols=\"" << t->numColumns() << "\"";

   for (const auto &opt : t->attribs()) {
    if (opt.name == "width") {
      m_t << " " << opt.name << "=\"" << opt.value << "\"";
    }
  }
  m_t << ">";

  if (t->hasCaption()) {
    DocHtmlCaption *c = t->caption();
    m_t << "<caption";

    if (! c->file().isEmpty()) {
      m_t << " id=\""  << stripPath(c->file()) << "_1" << c->anchor() << "\"";
    }

    m_t << ">";
  }
}

void XmlDocVisitor::visitPost(DocHtmlTable *)
{
   if (m_hide) {
      return;
   }

   m_t << "</table>\n";
}

void XmlDocVisitor::visitPre(DocHtmlRow *)
{
   if (m_hide) {
      return;
   }
   m_t << "<row>\n";
}

void XmlDocVisitor::visitPost(DocHtmlRow *)
{
   if (m_hide) {
      return;
   }
   m_t << "</row>\n";
}

void XmlDocVisitor::visitPre(DocHtmlCell *c)
{
   if (m_hide) {
      return;
   }

   if (c->isHeading()) {
      m_t << "<entry thead=\"yes\"";
   } else {
      m_t << "<entry thead=\"no\"";
   }

   for (const auto &opt : c->attribs()) {

      if (opt.name == "colspan" || opt.name == "rowspan") {
         m_t << " " << opt.name << "=\"" << opt.value.toInteger<int>() << "\"";

      } else if (opt.name == "align" && (opt.value == "right" || opt.value == "left" || opt.value == "center")) {
         m_t << " align=\"" << opt.value << "\"";

      } else if (opt.name ==" valign" && (opt.value == "bottom" || opt.value == "top" || opt.value == "middle")) {
         m_t << " valign=\"" << opt.value << "\"";

      } else if (opt.name == "width") {
         m_t << " width=\"" << opt.value << "\"";

      } else if (opt.name == "class") {
         // handle markdown generated attributes

         if (opt.value.startsWith("markdownTable")) {
            // handle markdown generated attributes

           if (opt.value.endsWith("Right")) {
             m_t << " align='right'";

           } else if (opt.value.endsWith("Left")) {
             m_t << " align='left'";

           } else if (opt.value.endsWith("Center")) {
             m_t << " align='center'";
           }

           // skip 'markdownTable*' value ending with "None"

         } else if (! opt.value.isEmpty())      {
           m_t << " class=\"" << convertToXML(opt.value) << "\"";

         }
      }
   }
   m_t << ">";
}

void XmlDocVisitor::visitPost(DocHtmlCell *)
{
   if (m_hide) {
      return;
   }

   m_t << "</entry>";
}

void XmlDocVisitor::visitPre(DocHtmlCaption *)
{
   if (m_hide) {
      return;
   }
}

void XmlDocVisitor::visitPost(DocHtmlCaption *)
{
   if (m_hide) {
      return;
   }

   m_t << "</caption>\n";
}

void XmlDocVisitor::visitPre(DocInternal *)
{
   if (m_hide) {
      return;
   }
   m_t << "<internal>";
}

void XmlDocVisitor::visitPost(DocInternal *)
{
   if (m_hide) {
      return;
   }
   m_t << "</internal>" << endl;
}

void XmlDocVisitor::visitPre(DocHRef *href)
{
   if (m_hide) {
      return;
   }

   m_t << "<ulink url=\"" << convertToXML(href->url(), true) << "\">";
}

void XmlDocVisitor::visitPost(DocHRef *)
{
   if (m_hide) {
      return;
   }

   m_t << "</ulink>";
}

void XmlDocVisitor::visitPre(DocHtmlHeader *header)
{
   if (m_hide) {
      return;
   }

   m_t << "<heading level=\"" << header->level() << "\">";
}

void XmlDocVisitor::visitPost(DocHtmlHeader *)
{
   if (m_hide) {
      return;
   }
   m_t << "</heading>\n";
}

void XmlDocVisitor::visitPre(DocImage *img)
{
   if (m_hide) {
      return;
   }

   static const QString xmlOutDir = Config::getString("xml-output");

   QString url      = img->url();
   QString baseName;

   if (url.isEmpty())   {
      baseName = img->relPath() + img->name();

   } else {
      baseName = correctURL(url, img->relPath());
   }

   visitPreStart(m_t, "image", false, this, img->children(), baseName, true, img->type(), img->width(), img->height(), img->isInlineImage());

   // copy the image to the output dir
   QSharedPointer<FileDef> fd;
   bool ambig;

   fd = findFileDef(&Doxy_Globals::imageNameDict, img->name(), ambig);

   if (fd != nullptr && url.isEmpty())  {
      QFile inImage(fd->getFilePath());
      QString outputFile = xmlOutDir + "/" + baseName;

      if (inImage.open(QIODevice::ReadOnly)) {

         QFile::remove(outputFile);
         bool copyOk = inImage.copy(outputFile);

         if (! copyOk) {
            err("Unable to open image file %s for writing, OS Error #: %d\n", csPrintable(outputFile), inImage.error());
         }

      } else {
         err("Unable to open image file %s for reading, OS Error #: %d\n", csPrintable( img->name() ), inImage.error());
      }
   }
}

void XmlDocVisitor::visitPost(DocImage *)
{
   if (m_hide) {
      return;
   }

   visitPostEnd(m_t, "image");
}

void XmlDocVisitor::visitPre(DocDotFile *df)
{
   if (m_hide) {
      return;
   }

   visitPreStart(m_t, "dotfile", false, this, df->children(), df->file(), false, DocImage::Html, df->width(), df->height());
}

void XmlDocVisitor::visitPost(DocDotFile *)
{
   if (m_hide) {
      return;
   }

   visitPostEnd(m_t, "dotfile");
}

void XmlDocVisitor::visitPre(DocMscFile *df)
{
   if (m_hide) {
      return;
   }

   visitPreStart(m_t, "mscfile", false, this, df->children(), df->file(), false, DocImage::Html, df->width(), df->height());
}

void XmlDocVisitor::visitPost(DocMscFile *)
{
   if (m_hide) {
      return;
   }

   visitPostEnd(m_t, "mscfile");
}

void XmlDocVisitor::visitPre(DocDiaFile *df)
{
   if (m_hide) {
      return;
   }

   visitPreStart(m_t, "diafile", false, this, df->children(), df->file(), false, DocImage::Html, df->width(), df->height());
}

void XmlDocVisitor::visitPost(DocDiaFile *)
{
   if (m_hide) {
      return;
   }

   visitPostEnd(m_t, "diafile");
}

void XmlDocVisitor::visitPre(DocLink *lnk)
{
   if (m_hide) {
      return;
   }

   startLink(lnk->ref(), lnk->file(), lnk->anchor());
}

void XmlDocVisitor::visitPost(DocLink *)
{
   if (m_hide) {
      return;
   }

   endLink();
}

void XmlDocVisitor::visitPre(DocRef *ref)
{
   if (m_hide) {
      return;
   }

   if (! ref->file().isEmpty()) {
      startLink(ref->ref(), ref->file(), ref->isSubPage() ? QString() : ref->anchor());
   }

   if (! ref->hasLinkText()) {
      filter(ref->targetTitle());
   }
}

void XmlDocVisitor::visitPost(DocRef *ref)
{
   if (m_hide) {
      return;
   }

   if (! ref->file().isEmpty()) {
      endLink();
   }
}

void XmlDocVisitor::visitPre(DocSecRefItem *ref)
{
   if (m_hide) {
      return;
   }

   m_t << "<tocitem id=\"" << ref->file() << "_1" << ref->anchor() << "\">";
}

void XmlDocVisitor::visitPost(DocSecRefItem *)
{
   if (m_hide) {
      return;
   }

   m_t << "</tocitem>" << endl;
}

void XmlDocVisitor::visitPre(DocSecRefList *)
{
   if (m_hide) {
      return;
   }

   m_t << "<toclist>\n";
}

void XmlDocVisitor::visitPost(DocSecRefList *)
{
   if (m_hide) {
      return;
   }

   m_t << "</toclist>" << endl;
}

//void XmlDocVisitor::visitPre(DocLanguage *l)
//{
//  if (m_hide) return;
//  m_t << "<language langid=\"" << l->id() << "\">";
//}
//
//void XmlDocVisitor::visitPost(DocLanguage *)
//{
//  if (m_hide) return;
//  m_t << "</language>" << endl;
//}

void XmlDocVisitor::visitPre(DocParamSect *s)
{
   if (m_hide) {
      return;
   }

   m_t << "<parameterlist kind=\"";
   switch (s->type()) {
      case DocParamSect::Param:
         m_t << "param";
         break;

      case DocParamSect::RetVal:
         m_t << "retval";
         break;

      case DocParamSect::Exception:
         m_t << "exception";
         break;

      case DocParamSect::TemplateParam:
         m_t << "templateparam";
         break;

      default:
         assert(0);
   }

   m_t << "\">";
}

void XmlDocVisitor::visitPost(DocParamSect *)
{
   if (m_hide) {
      return;
   }

   m_t << "</parameterlist>" << endl;
}

void XmlDocVisitor::visitPre(DocParamList *pl)
{
   if (m_hide) {
      return;
   }

   m_t << "<parameteritem>\n";
   m_t << "<parameternamelist>\n";

   for (const auto &param : pl->parameters()) {

      if (pl->paramTypes().count() > 0) {
         m_t << "<parametertype>";

         for (const auto &type : pl->paramTypes()) {
            if (type->kind() == DocNode::Kind_Word) {
               visit((DocWord *)type);

            } else if (type->kind() == DocNode::Kind_LinkedWord) {
               visit((DocLinkedWord *)type);

            } else if (type->kind() == DocNode::Kind_Sep) {
               m_t << "</parametertype>\n";
               m_t << "<parametertype>";
            }
         }

         m_t << "</parametertype>\n";
      }

      m_t << "<parametername";

      if (pl->direction() != DocParamSect::Unspecified) {
         m_t << " direction=\"";

         if (pl->direction() == DocParamSect::In) {
            m_t << "in";
         } else if (pl->direction() == DocParamSect::Out) {
            m_t << "out";
         } else if (pl->direction() == DocParamSect::InOut) {
            m_t << "inout";
         }
         m_t << "\"";
      }

      m_t << ">";
      if (param->kind() == DocNode::Kind_Word) {
         visit((DocWord *)param);
      } else if (param->kind() == DocNode::Kind_LinkedWord) {
         visit((DocLinkedWord *)param);
      }

      m_t << "</parametername>\n";
   }

   m_t << "</parameternamelist>\n";
   m_t << "<parameterdescription>\n";
}

void XmlDocVisitor::visitPost(DocParamList *)
{
   if (m_hide) {
      return;
   }

   m_t << "</parameterdescription>" << endl;
   m_t << "</parameteritem>" << endl;
}

void XmlDocVisitor::visitPre(DocXRefItem *x)
{
   if (m_hide) {
      return;
   }

   if (x->title().isEmpty()) {
      return;
   }

   m_t << "<xrefsect id=\"";
   m_t << x->file() << "_1" << x->anchor();
   m_t << "\">";
   m_t << "<xreftitle>";

   filter(x->title());

   m_t << "</xreftitle>";
   m_t << "<xrefdescription>";
}

void XmlDocVisitor::visitPost(DocXRefItem *x)
{
   if (m_hide) {
      return;
   }
   if (x->title().isEmpty()) {
      return;
   }
   m_t << "</xrefdescription>";
   m_t << "</xrefsect>";
}

void XmlDocVisitor::visitPre(DocInternalRef *ref)
{
   if (m_hide) {
      return;
   }

   startLink(QString(), ref->file(), ref->anchor());
}

void XmlDocVisitor::visitPost(DocInternalRef *)
{
   if (m_hide) {
      return;
   }

   endLink();
   m_t << " ";
}

void XmlDocVisitor::visitPre(DocCopy *c)
{
   if (m_hide) {
      return;
   }

   m_t << "<copydoc link=\"" << convertToXML(c->link()) << "\">";
}

void XmlDocVisitor::visitPost(DocCopy *)
{
   if (m_hide) {
      return;
   }

   m_t << "</copydoc>" << endl;
}

void XmlDocVisitor::visitPre(DocText *)
{
}

void XmlDocVisitor::visitPost(DocText *)
{
}

void XmlDocVisitor::visitPre(DocHtmlBlockQuote *)
{
   if (m_hide) {
      return;
   }
   m_t << "<blockquote>";
}

void XmlDocVisitor::visitPost(DocHtmlBlockQuote *)
{
   if (m_hide) {
      return;
   }

   m_t << "</blockquote>";
}

void XmlDocVisitor::visitPre(DocParBlock *)
{
   if (m_hide) {
      return;
   }

   m_t << "<parblock>";
}

void XmlDocVisitor::visitPost(DocParBlock *)
{
   if (m_hide) {
      return;
   }
   m_t << "</parblock>";
}


void XmlDocVisitor::filter(const QString &str)
{
   m_t << convertToXML(str);
}

void XmlDocVisitor::startLink(const QString &ref, const QString &file, const QString &anchor)
{
   m_t << "<ref refid=\"" << file;

   if (!anchor.isEmpty()) {
      m_t << "_1" << anchor;
   }

   m_t << "\" kindref=\"";

   if (!anchor.isEmpty()) {
      m_t << "member";
   } else {
      m_t << "compound";
   }

   m_t << "\"";
   if (!ref.isEmpty()) {
      m_t << " external=\"" << ref << "\"";
   }
   m_t << ">";
}

void XmlDocVisitor::endLink()
{
   m_t << "</ref>";
}

void XmlDocVisitor::pushEnabled()
{
   m_enabled.push(m_hide);
}

void XmlDocVisitor::popEnabled()
{
   bool v = m_enabled.pop();
   m_hide = v;
}

