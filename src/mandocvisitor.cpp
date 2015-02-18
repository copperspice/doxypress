/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch. 
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
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

#include <QFileInfo>

#include <code.h>
#include <docparser.h>
#include <dot.h>
#include <doxy_globals.h>
#include <doxygen.h>
#include <filedef.h>
#include <htmlentity.h>
#include <language.h>
#include <mandocvisitor.h>
#include <message.h>
#include <outputgen.h>
#include <parserintf.h>
#include <util.h>

ManDocVisitor::ManDocVisitor(QTextStream &t, CodeOutputInterface &ci, const char *langExt)
   : DocVisitor(DocVisitor_Man), m_t(t), m_ci(ci), m_insidePre(false), m_hide(false), m_firstCol(false),
     m_indent(0), m_langExt(langExt)
{
}

//--------------------------------------
// visitor functions for leaf nodes
//--------------------------------------

void ManDocVisitor::visit(DocWord *w)
{
   if (m_hide) {
      return;
   }
   filter(w->word());
   m_firstCol = false;
}

void ManDocVisitor::visit(DocLinkedWord *w)
{
   if (m_hide) {
      return;
   }
   m_t << "\\fB";
   filter(w->word());
   m_t << "\\fP";
   m_firstCol = false;
}

void ManDocVisitor::visit(DocWhiteSpace *w)
{
   if (m_hide) {
      return;
   }
   if (m_insidePre) {
      m_t << w->chars();
      m_firstCol = w->chars().at(w->chars().length() - 1) == '\n';
   } else {
      m_t << " ";
      m_firstCol = false;
   }
}

void ManDocVisitor::visit(DocSymbol *s)
{
   if (m_hide) {
      return;
   }
   const char *res = HtmlEntityMapper::instance()->man(s->symbol());
   if (res) {
      m_t << res;
   } else {
      // no error or warning to be supplied
      // err("man: non supported HTML-entity found: &%s;\n",get_symbol_item(s->symbol()));
   }
   m_firstCol = false;
}

void ManDocVisitor::visit(DocURL *u)
{
   if (m_hide) {
      return;
   }
   m_t << u->url();
   m_firstCol = false;
}

void ManDocVisitor::visit(DocLineBreak *)
{
   if (m_hide) {
      return;
   }
   m_t << endl << ".br" << endl;
   m_firstCol = true;
}

void ManDocVisitor::visit(DocHorRuler *)
{
   if (m_hide) {
      return;
   }
   if (!m_firstCol) {
      m_t << endl;
   }
   m_t << ".PP" << endl;
   m_firstCol = true;
}

void ManDocVisitor::visit(DocStyleChange *s)
{
   if (m_hide) {
      return;
   }
   switch (s->style()) {
      case DocStyleChange::Bold:
         if (s->enable()) {
            m_t << "\\fB";
         }      else {
            m_t << "\\fP";
         }
         m_firstCol = false;
         break;
      case DocStyleChange::Italic:
         if (s->enable()) {
            m_t << "\\fI";
         }     else {
            m_t << "\\fP";
         }
         m_firstCol = false;
         break;
      case DocStyleChange::Code:
         if (s->enable()) {
            m_t << "\\fC";
         }   else {
            m_t << "\\fP";
         }
         m_firstCol = false;
         break;
      case DocStyleChange::Subscript:
         if (s->enable()) {
            m_t << "\\*<";
         }    else {
            m_t << "\\*> ";
         }
         m_firstCol = false;
         break;
      case DocStyleChange::Superscript:
         if (s->enable()) {
            m_t << "\\*{";
         }    else {
            m_t << "\\*} ";
         }
         m_firstCol = false;
         break;
      case DocStyleChange::Center:
         /* not supported */
         break;
      case DocStyleChange::Small:
         /* not supported */
         break;
      case DocStyleChange::Preformatted:
         if (s->enable()) {
            if (!m_firstCol) {
               m_t << endl;
            }
            m_t << ".PP" << endl;
            m_t << ".nf" << endl;
            m_insidePre = true;
         } else {
            m_insidePre = false;
            if (!m_firstCol) {
               m_t << endl;
            }
            m_t << ".fi" << endl;
            m_t << ".PP" << endl;
            m_firstCol = true;
         }
         break;
      case DocStyleChange::Div:  /* HTML only */
         break;
      case DocStyleChange::Span: /* HTML only */
         break;
   }
}

void ManDocVisitor::visit(DocVerbatim *s)
{
   if (m_hide) {
      return;
   }
   QByteArray lang = m_langExt;
   if (!s->language().isEmpty()) { // explicit language setting
      lang = s->language();
   }
   SrcLangExt langExt = getLanguageFromFileName(lang);
   switch (s->type()) {
      case DocVerbatim::Code: // fall though
         if (!m_firstCol) {
            m_t << endl;
         }
         m_t << ".PP" << endl;
         m_t << ".nf" << endl;
         Doxygen::parserManager->getParser(lang)
         ->parseCode(m_ci, s->context(), s->text(),
                     langExt,
                     s->isExample(), s->exampleFile());
         if (!m_firstCol) {
            m_t << endl;
         }
         m_t << ".fi" << endl;
         m_t << ".PP" << endl;
         m_firstCol = true;
         break;
      case DocVerbatim::Verbatim:
         if (!m_firstCol) {
            m_t << endl;
         }
         m_t << ".PP" << endl;
         m_t << ".nf" << endl;
         m_t << s->text();
         if (!m_firstCol) {
            m_t << endl;
         }
         m_t << ".fi" << endl;
         m_t << ".PP" << endl;
         m_firstCol = true;
         break;
      case DocVerbatim::ManOnly:
         m_t << s->text();
         break;
      case DocVerbatim::HtmlOnly:
      case DocVerbatim::XmlOnly:
      case DocVerbatim::LatexOnly:
      case DocVerbatim::RtfOnly:
      case DocVerbatim::DocbookOnly:
      case DocVerbatim::Dot:
      case DocVerbatim::Msc:
      case DocVerbatim::PlantUML:
         /* nothing */
         break;
   }
}

void ManDocVisitor::visit(DocAnchor *)
{
   /* no support for anchors in man pages */
}

void ManDocVisitor::visit(DocInclude *inc)
{
   if (m_hide) {
      return;
   }
   SrcLangExt langExt = getLanguageFromFileName(inc->extension());

   switch (inc->type()) {
      case DocInclude::IncWithLines: {
         if (! m_firstCol) {
            m_t << endl;
         }
         m_t << ".PP" << endl;
         m_t << ".nf" << endl;

         QFileInfo cfi( inc->file() );
         QSharedPointer<FileDef> fd = QMakeShared<FileDef>( cfi.path().toUtf8(), cfi.fileName().toUtf8() );

         Doxygen::parserManager->getParser(inc->extension())->parseCode(m_ci, inc->context(), inc->text(), 
                                           langExt, inc->isExample(), inc->exampleFile(), fd);

         if (! m_firstCol) {
            m_t << endl;
         }

         m_t << ".fi" << endl;
         m_t << ".PP" << endl;
         m_firstCol = true;
      }

      break;
      case DocInclude::Include:
         if (!m_firstCol) {
            m_t << endl;
         }
         m_t << ".PP" << endl;
         m_t << ".nf" << endl;
         Doxygen::parserManager->getParser(inc->extension())->parseCode(m_ci, inc->context(),inc->text(),langExt,
                     inc->isExample(), inc->exampleFile());

         if (!m_firstCol) {
            m_t << endl;
         }
         m_t << ".fi" << endl;
         m_t << ".PP" << endl;
         m_firstCol = true;
         break;

      case DocInclude::DontInclude:
         break;

      case DocInclude::HtmlInclude:
         break;

      case DocInclude::LatexInclude:
         break;

      case DocInclude::VerbInclude:
         if (!m_firstCol) {
            m_t << endl;
         }
         m_t << ".PP" << endl;
         m_t << ".nf" << endl;
         m_t << inc->text();

         if (!m_firstCol) {
            m_t << endl;
         }

         m_t << ".fi" << endl;
         m_t << ".PP" << endl;
         m_firstCol = true;
         break;

      case DocInclude::Snippet:
         if (!m_firstCol) {
            m_t << endl;
         }
         m_t << ".PP" << endl;
         m_t << ".nf" << endl;
         Doxygen::parserManager->getParser(inc->extension())->parseCode(m_ci, inc->context(), extractBlock(inc->text(), inc->blockId()),
                     langExt, inc->isExample(), inc->exampleFile() );

         if (!m_firstCol) {
            m_t << endl;
         }

         m_t << ".fi" << endl;
         m_t << ".PP" << endl;
         m_firstCol = true;
         break;
   }
}

void ManDocVisitor::visit(DocIncOperator *op)
{
   SrcLangExt langExt = getLanguageFromFileName(m_langExt);
   //printf("DocIncOperator: type=%d first=%d, last=%d text=`%s'\n",
   //    op->type(),op->isFirst(),op->isLast(),op->text().data());

   if (op->isFirst()) {
      if (!m_hide) {
         if (!m_firstCol) {
            m_t << endl;
         }

         m_t << ".PP" << endl;
         m_t << ".nf" << endl;
      }
      pushEnabled();
      m_hide = true;
   }

   if (op->type() != DocIncOperator::Skip) {
      popEnabled();
      if (!m_hide) {
         Doxygen::parserManager->getParser(m_langExt)
         ->parseCode(m_ci, op->context(), op->text(), langExt,
                     op->isExample(), op->exampleFile());
      }
      pushEnabled();
      m_hide = true;
   }
   if (op->isLast()) {
      popEnabled();
      if (!m_hide) {
         if (!m_firstCol) {
            m_t << endl;
         }
         m_t << ".fi" << endl;
         m_t << ".PP" << endl;
         m_firstCol = true;
      }
   } else {
      if (!m_hide) {
         m_t << endl;
      }
   }
}

void ManDocVisitor::visit(DocFormula *f)
{
   if (m_hide) {
      return;
   }
   m_t << f->text();
}

void ManDocVisitor::visit(DocIndexEntry *)
{
}

void ManDocVisitor::visit(DocSimpleSectSep *)
{
}

void ManDocVisitor::visit(DocCite *cite)
{
   if (m_hide) {
      return;
   }
   m_t << "\\fB";
   if (cite->file().isEmpty()) {
      m_t << "[";
   }
   filter(cite->text());
   if (cite->file().isEmpty()) {
      m_t << "]";
   }
   m_t << "\\fP";
}


//--------------------------------------
// visitor functions for compound nodes
//--------------------------------------

void ManDocVisitor::visitPre(DocAutoList *)
{
   if (m_hide) {
      return;
   }
   m_indent += 2;
}

void ManDocVisitor::visitPost(DocAutoList *)
{
   if (m_hide) {
      return;
   }
   m_indent -= 2;
   m_t << ".PP" << endl;
}

void ManDocVisitor::visitPre(DocAutoListItem *li)
{
   if (m_hide) {
      return;
   }
   QByteArray ws;
   ws.fill(' ', m_indent - 2);
   if (!m_firstCol) {
      m_t << endl;
   }
   m_t << ".IP \"" << ws;
   if (((DocAutoList *)li->parent())->isEnumList()) {
      m_t << li->itemNumber() << ".\" " << m_indent + 2;
   } else { // bullet list
      m_t << "\\(bu\" " << m_indent;
   }
   m_t << endl;
   m_firstCol = true;
}

void ManDocVisitor::visitPost(DocAutoListItem *)
{
   if (m_hide) {
      return;
   }
   m_t << endl;
   m_firstCol = true;
}

void ManDocVisitor::visitPre(DocPara *)
{
}

void ManDocVisitor::visitPost(DocPara *p)
{
   if (m_hide) {
      return;
   }
   if (!p->isLast() &&            // omit <p> for last paragraph
         !(p->parent() &&           // and for parameter sections
           p->parent()->kind() == DocNode::Kind_ParamSect
          )
      ) {
      if (!m_firstCol) {
         m_t << endl;
      }
      m_t << ".PP" << endl;
      m_firstCol = true;
   }
}

void ManDocVisitor::visitPre(DocRoot *)
{
}

void ManDocVisitor::visitPost(DocRoot *)
{
}

void ManDocVisitor::visitPre(DocSimpleSect *s)
{
   if (m_hide) {
      return;
   }
   if (!m_firstCol) {
      m_t << endl;
      m_t << ".PP" << endl;
   }
   m_t << "\\fB";
   switch (s->type()) {
      case DocSimpleSect::See:
         m_t << theTranslator->trSeeAlso();
         break;
      case DocSimpleSect::Return:
         m_t << theTranslator->trReturns();
         break;
      case DocSimpleSect::Author:
         m_t << theTranslator->trAuthor(true, true);
         break;
      case DocSimpleSect::Authors:
         m_t << theTranslator->trAuthor(true, false);
         break;
      case DocSimpleSect::Version:
         m_t << theTranslator->trVersion();
         break;
      case DocSimpleSect::Since:
         m_t << theTranslator->trSince();
         break;
      case DocSimpleSect::Date:
         m_t << theTranslator->trDate();
         break;
      case DocSimpleSect::Note:
         m_t << theTranslator->trNote();
         break;
      case DocSimpleSect::Warning:
         m_t << theTranslator->trWarning();
         break;
      case DocSimpleSect::Pre:
         m_t << theTranslator->trPrecondition();
         break;
      case DocSimpleSect::Post:
         m_t << theTranslator->trPostcondition();
         break;
      case DocSimpleSect::Copyright:
         m_t << theTranslator->trCopyright();
         break;
      case DocSimpleSect::Invar:
         m_t << theTranslator->trInvariant();
         break;
      case DocSimpleSect::Remark:
         m_t << theTranslator->trRemarks();
         break;
      case DocSimpleSect::Attention:
         m_t << theTranslator->trAttention();
         break;
      case DocSimpleSect::User:
         break;
      case DocSimpleSect::Rcs:
         break;
      case DocSimpleSect::Unknown:
         break;
   }

   // special case 1: user defined title
   if (s->type() != DocSimpleSect::User && s->type() != DocSimpleSect::Rcs) {
      m_t << ":\\fP" << endl;
      m_t << ".RS 4" << endl;
   }
}

void ManDocVisitor::visitPost(DocSimpleSect *)
{
   if (m_hide) {
      return;
   }
   if (!m_firstCol) {
      m_t << endl;
   }
   m_t << ".RE" << endl;
   m_t << ".PP" << endl;
   m_firstCol = true;
}

void ManDocVisitor::visitPre(DocTitle *)
{
}

void ManDocVisitor::visitPost(DocTitle *)
{
   if (m_hide) {
      return;
   }
   m_t << "\\fP" << endl;
   m_t << ".RS 4" << endl;
}

void ManDocVisitor::visitPre(DocSimpleList *)
{
   if (m_hide) {
      return;
   }
   m_indent += 2;
   if (!m_firstCol) {
      m_t << endl;
   }
   m_t << ".PD 0" << endl;
}

void ManDocVisitor::visitPost(DocSimpleList *)
{
   if (m_hide) {
      return;
   }
   m_indent -= 2;
   m_t << ".PP" << endl;
}

void ManDocVisitor::visitPre(DocSimpleListItem *)
{
   if (m_hide) {
      return;
   }
   QByteArray ws;
   ws.fill(' ', m_indent - 2);
   if (!m_firstCol) {
      m_t << endl;
   }
   m_t << ".IP \"" << ws << "\\(bu\" " << m_indent << endl;
   m_firstCol = true;
}

void ManDocVisitor::visitPost(DocSimpleListItem *)
{
   if (m_hide) {
      return;
   }
   m_t << endl;
   m_firstCol = true;
}

void ManDocVisitor::visitPre(DocSection *s)
{
   if (m_hide) {
      return;
   }
   if (!m_firstCol) {
      m_t << endl;
   }
   if (s->level() == 1) {
      m_t << ".SH";
   } else {
      m_t << ".SS";
   }
   m_t << " \"";
   filter(s->title());
   m_t << "\"" << endl;
   if (s->level() == 1) {
      m_t << ".PP" << endl;
   }
   m_firstCol = true;
}

void ManDocVisitor::visitPost(DocSection *)
{
}

void ManDocVisitor::visitPre(DocHtmlList *)
{
   if (m_hide) {
      return;
   }
   m_indent += 2;
   if (!m_firstCol) {
      m_t << endl;
   }
   m_t << ".PD 0" << endl;
}

void ManDocVisitor::visitPost(DocHtmlList *)
{
   if (m_hide) {
      return;
   }
   m_indent -= 2;
   if (!m_firstCol) {
      m_t << endl;
   }
   m_t << ".PP" << endl;
}

void ManDocVisitor::visitPre(DocHtmlListItem *li)
{
   if (m_hide) {
      return;
   }
   QByteArray ws;
   ws.fill(' ', m_indent - 2);
   if (!m_firstCol) {
      m_t << endl;
   }
   m_t << ".IP \"" << ws;
   if (((DocHtmlList *)li->parent())->type() == DocHtmlList::Ordered) {
      m_t << li->itemNumber() << ".\" " << m_indent + 2;
   } else { // bullet list
      m_t << "\\(bu\" " << m_indent;
   }
   m_t << endl;
   m_firstCol = true;
}

void ManDocVisitor::visitPost(DocHtmlListItem *)
{
   if (m_hide) {
      return;
   }
   m_t << endl;
   m_firstCol = true;
}

//void ManDocVisitor::visitPre(DocHtmlPre *)
//{
//  if (!m_firstCol) m_t << endl;
//  m_t << ".PP" << endl;
//  m_t << ".nf" << endl;
//  m_insidePre=true;
//}
//
//void ManDocVisitor::visitPost(DocHtmlPre *)
//{
//  m_insidePre=false;
//  if (!m_firstCol) m_t << endl;
//  m_t << ".fi" << endl;
//  m_t << ".PP" << endl;
//  m_firstCol=true;
//}

void ManDocVisitor::visitPre(DocHtmlDescList *)
{
}

void ManDocVisitor::visitPost(DocHtmlDescList *)
{
   if (m_hide) {
      return;
   }
   if (!m_firstCol) {
      m_t << endl;
   }
   m_t << ".PP" << endl;
   m_firstCol = true;
}

void ManDocVisitor::visitPre(DocHtmlDescTitle *)
{
   if (m_hide) {
      return;
   }
   if (!m_firstCol) {
      m_t << endl;
   }
   m_t << ".IP \"\\fB";
   m_firstCol = false;
}

void ManDocVisitor::visitPost(DocHtmlDescTitle *)
{
   if (m_hide) {
      return;
   }
   m_t << "\\fP\" 1c" << endl;
   m_firstCol = true;
}

void ManDocVisitor::visitPre(DocHtmlDescData *)
{
}

void ManDocVisitor::visitPost(DocHtmlDescData *)
{
}

void ManDocVisitor::visitPre(DocHtmlTable *)
{
}

void ManDocVisitor::visitPost(DocHtmlTable *)
{
}

void ManDocVisitor::visitPre(DocHtmlCaption *)
{
}

void ManDocVisitor::visitPost(DocHtmlCaption *)
{
}

void ManDocVisitor::visitPre(DocHtmlRow *)
{
}

void ManDocVisitor::visitPost(DocHtmlRow *)
{
}

void ManDocVisitor::visitPre(DocHtmlCell *)
{
}

void ManDocVisitor::visitPost(DocHtmlCell *)
{
}

void ManDocVisitor::visitPre(DocInternal *)
{
   if (m_hide) {
      return;
   }
   //if (!m_firstCol) m_t << endl;
   //m_t << ".PP" << endl;
   //m_t << "\\fB" << theTranslator->trForInternalUseOnly() << "\\fP" << endl;
   //m_t << ".RS 4" << endl;
}

void ManDocVisitor::visitPost(DocInternal *)
{
   if (m_hide) {
      return;
   }
   //if (!m_firstCol) m_t << endl;
   //m_t << ".RE" << endl;
   //m_t << ".PP" << endl;
   //m_firstCol=true;
}

void ManDocVisitor::visitPre(DocHRef *)
{
   if (m_hide) {
      return;
   }
   m_t << "\\fC";
}

void ManDocVisitor::visitPost(DocHRef *)
{
   if (m_hide) {
      return;
   }
   m_t << "\\fP";
}

void ManDocVisitor::visitPre(DocHtmlHeader *header)
{
   if (m_hide) {
      return;
   }
   if (!m_firstCol) {
      m_t << endl;
   }
   if (header->level() == 1) {
      m_t << ".SH";
   } else {
      m_t << ".SS";
   }
   m_t << " \"";
}

void ManDocVisitor::visitPost(DocHtmlHeader *header)
{
   if (m_hide) {
      return;
   }
   m_t << "\"" << endl;
   if (header->level() == 1) {
      m_t << ".PP" << endl;
   }
   m_firstCol = true;
}

void ManDocVisitor::visitPre(DocImage *)
{
}

void ManDocVisitor::visitPost(DocImage *)
{
}

void ManDocVisitor::visitPre(DocDotFile *)
{
}

void ManDocVisitor::visitPost(DocDotFile *)
{
}
void ManDocVisitor::visitPre(DocMscFile *)
{
}

void ManDocVisitor::visitPost(DocMscFile *)
{
}

void ManDocVisitor::visitPre(DocDiaFile *)
{
}

void ManDocVisitor::visitPost(DocDiaFile *)
{
}

void ManDocVisitor::visitPre(DocLink *)
{
   if (m_hide) {
      return;
   }
   m_t << "\\fB";
}

void ManDocVisitor::visitPost(DocLink *)
{
   if (m_hide) {
      return;
   }
   m_t << "\\fP";
}

void ManDocVisitor::visitPre(DocRef *ref)
{
   if (m_hide) {
      return;
   }
   m_t << "\\fB";
   if (!ref->hasLinkText()) {
      filter(ref->targetTitle());
   }
}

void ManDocVisitor::visitPost(DocRef *)
{
   if (m_hide) {
      return;
   }
   m_t << "\\fP";
}

void ManDocVisitor::visitPre(DocSecRefItem *)
{
   if (m_hide) {
      return;
   }
   QByteArray ws;
   ws.fill(' ', m_indent - 2);
   if (!m_firstCol) {
      m_t << endl;
   }
   m_t << ".IP \"" << ws << "\\(bu\" " << m_indent << endl;
   m_firstCol = true;
}

void ManDocVisitor::visitPost(DocSecRefItem *)
{
   if (m_hide) {
      return;
   }
   m_t << endl;
   m_firstCol = true;
}

void ManDocVisitor::visitPre(DocSecRefList *)
{
   if (m_hide) {
      return;
   }
   m_indent += 2;
}

void ManDocVisitor::visitPost(DocSecRefList *)
{
   if (m_hide) {
      return;
   }
   m_indent -= 2;
   if (!m_firstCol) {
      m_t << endl;
   }
   m_t << ".PP" << endl;
}

//void ManDocVisitor::visitPre(DocLanguage *l)
//{
//  QString langId = Config_getEnum("OUTPUT_LANGUAGE");
//  if (l->id().toLower()!=langId.lower())
//  {
//    pushEnabled();
//    m_hide = true;
//  }
//}
//
//void ManDocVisitor::visitPost(DocLanguage *l)
//{
//  QString langId = Config_getEnum("OUTPUT_LANGUAGE");
//  if (l->id().toLower()!=langId.lower())
//  {
//    popEnabled();
//  }
//}

void ManDocVisitor::visitPre(DocParamSect *s)
{
   if (m_hide) {
      return;
   }
   if (!m_firstCol) {
      m_t << endl;
      m_t << ".PP" << endl;
   }
   m_t << "\\fB";
   switch (s->type()) {
      case DocParamSect::Param:
         m_t << theTranslator->trParameters();
         break;
      case DocParamSect::RetVal:
         m_t << theTranslator->trReturnValues();
         break;
      case DocParamSect::Exception:
         m_t << theTranslator->trExceptions();
         break;
      case DocParamSect::TemplateParam:
         /* TODO: add this
         m_t << theTranslator->trTemplateParam(); break;
         */
         m_t << "Template Parameters";
         break;
      default:
         assert(0);
   }
   m_t << ":\\fP" << endl;
   m_t << ".RS 4" << endl;
}

void ManDocVisitor::visitPost(DocParamSect *)
{
   if (m_hide) {
      return;
   }
   if (!m_firstCol) {
      m_t << endl;
   }
   m_t << ".RE" << endl;
   m_t << ".PP" << endl;
   m_firstCol = true;
}

void ManDocVisitor::visitPre(DocParamList *pl)
{
   if (m_hide) {
      return;
   }

   m_t << "\\fI";
   
   bool first = true;

   for (auto param : pl->parameters()) {   
      if (!first) {
         m_t << ",";

      } else {
         first = false;
      }

      if (param->kind() == DocNode::Kind_Word) {
         visit((DocWord *)param);

      } else if (param->kind() == DocNode::Kind_LinkedWord) {
         visit((DocLinkedWord *)param);

      }
   }

   m_t << "\\fP ";
}

void ManDocVisitor::visitPost(DocParamList *pl)
{
   if (m_hide) {
      return;
   }

   if (!pl->isLast()) {
      if (!m_firstCol) {
         m_t << endl;
      }

      m_t << ".br" << endl;
   }
}

void ManDocVisitor::visitPre(DocXRefItem *x)
{
   if (m_hide) {
      return;
   }

   if (x->title().isEmpty()) {
      return;
   }

   if (!m_firstCol) {
      m_t << endl;
      m_t << ".PP" << endl;
   }

   m_t << "\\fB";
   filter(x->title());
   m_t << "\\fP" << endl;
   m_t << ".RS 4" << endl;
}

void ManDocVisitor::visitPost(DocXRefItem *x)
{
   if (m_hide) {
      return;
   }

   if (x->title().isEmpty()) {
      return;
   }

   if (!m_firstCol) {
      m_t << endl;
   }
   m_t << ".RE" << endl;
   m_t << ".PP" << endl;
   m_firstCol = true;
}

void ManDocVisitor::visitPre(DocInternalRef *)
{
   if (m_hide) {
      return;
   }
   m_t << "\\fB";
}

void ManDocVisitor::visitPost(DocInternalRef *)
{
   if (m_hide) {
      return;
   }

   m_t << "\\fP";
}

void ManDocVisitor::visitPre(DocCopy *)
{
}

void ManDocVisitor::visitPost(DocCopy *)
{
}

void ManDocVisitor::visitPre(DocText *)
{
}

void ManDocVisitor::visitPost(DocText *)
{
}

void ManDocVisitor::visitPre(DocHtmlBlockQuote *)
{
   if (m_hide) {
      return;
   }

   if (!m_firstCol) {
      m_t << endl;
      m_t << ".PP" << endl;
   }
   m_t << ".RS 4" << endl; // TODO: add support for nested block quotes
}

void ManDocVisitor::visitPost(DocHtmlBlockQuote *)
{
   if (m_hide) {
      return;
   }

   if (!m_firstCol) {
      m_t << endl;
   }

   m_t << ".RE" << endl;
   m_t << ".PP" << endl;
   m_firstCol = true;
}

void ManDocVisitor::visitPre(DocParBlock *)
{
}

void ManDocVisitor::visitPost(DocParBlock *)
{
}

void ManDocVisitor::filter(const char *str)
{
   if (str) {
      const char *p = str;
      char c = 0;
      while ((c = *p++)) {
         switch (c) {
            case '.':
               m_t << "\\&.";
               break; // see  bug652277
            case '\\':
               m_t << "\\\\";
               break;
            case '"':
               c = '\''; // fall through
            default:
               m_t << c;
               break;
         }
      }
   }
}

void ManDocVisitor::pushEnabled()
{
   m_enabled.push(m_hide);
}

void ManDocVisitor::popEnabled()
{
   bool v = m_enabled.pop();
   m_hide = v;  
}

