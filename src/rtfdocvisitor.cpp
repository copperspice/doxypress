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

#include <QFileInfo>

#include <config.h>
#include <dia.h>
#include <docparser.h>
#include <dot.h>
#include <doxy_globals.h>
#include <filedef.h>
#include <htmlentity.h>
#include <language.h>
#include <msc.h>
#include <msc.h>
#include <message.h>
#include <outputgen.h>
#include <parser_base.h>
#include <plantuml.h>
#include <rtfstyle.h>
#include <rtfdocvisitor.h>
#include <util.h>

// #define DBG_RTF(x) m_t << x
#define DBG_RTF(x) do {} while(0)

static QString align(DocHtmlCell *cell)
{
   HtmlAttribList attrs = cell->attribs();
   uint i;

   for (i = 0; i < attrs.count(); ++i) {
  
      if (attrs.at(i).name.toLower() == "align") {
         if (attrs.at(i).value.toLower() == "center") {
            return "\\qc ";

         } else if (attrs.at(i).value.toLower() == "right") {
            return "\\qr ";

         } else {
            return "";
         }
      }
   }

   return "";
}

RTFDocVisitor::RTFDocVisitor(QTextStream &t, CodeOutputInterface &ci, const QString &langExt)
   : DocVisitor(DocVisitor_RTF), m_t(t), m_ci(ci), m_insidePre(false),
     m_hide(false), m_indentLevel(0), m_lastIsPara(false), m_langExt(langExt)
{
}

QString RTFDocVisitor::getStyle(const QString &name)
{
   QString n = QString("%1%2").arg(name).arg(m_indentLevel);

   auto sd = rtf_Style.find(n);
   
   if (sd == rtf_Style.end()) {
      assert(sd != rtf_Style.end());
   }
  
   return sd->reference;
}

void RTFDocVisitor::incIndentLevel()
{
   if (m_indentLevel < rtf_maxIndentLevels - 1) {
      m_indentLevel++;
   }
}

void RTFDocVisitor::decIndentLevel()
{
   if (m_indentLevel > 0) {
      m_indentLevel--;
   }
}

// visitor functions for leaf nodes
void RTFDocVisitor::visit(DocWord *w)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visit(DocWord)}\n");
   filter(w->word());
   m_lastIsPara = false;
}

void RTFDocVisitor::visit(DocLinkedWord *w)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visit(DocLinkedWord)}\n");
   startLink(w->ref(), w->file(), w->anchor());
   filter(w->word());
   endLink(w->ref());

   m_lastIsPara = false;
}

void RTFDocVisitor::visit(DocWhiteSpace *w)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visit(DocWhiteSpace)}\n");

   if (m_insidePre) {
      m_t << w->chars();
   } else {
      m_t << " ";
   }

   m_lastIsPara = false;
}

void RTFDocVisitor::visit(DocSymbol *s)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visit(DocSymbol)}\n");
   const QString res = HtmlEntityMapper::instance()->rtf(s->symbol());


// brooom check


   if (! res.isEmpty()) {
      m_t << res;

   } else {
      err("RTF: non supported HTML-entity found: %s\n", HtmlEntityMapper::instance()->html(s->symbol(), true));
   }

   m_lastIsPara = false;
}

void RTFDocVisitor::visit(DocURL *u)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visit(DocURL)}\n");

   if (Config::getBool("rtf-hyperlinks")) {
      m_t << "{\\field "
          "{\\*\\fldinst "
          "{ HYPERLINK \"";
      if (u->isEmail()) {
         m_t << "mailto:";
      }
      m_t << u->url();
      m_t <<  "\" }"
          "{}";
      m_t <<   "}"
          "{\\fldrslt "
          "{\\cs37\\ul\\cf2 ";
      filter(u->url());
      m_t <<     "}"
          "}"
          "}" << endl;
   } else {
      m_t << "{\\f2 ";
      filter(u->url());
      m_t << "}";
   }

   m_lastIsPara = false;
}

void RTFDocVisitor::visit(DocLineBreak *)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visit(DocLineBreak)}\n");

   m_t << "\\par" << endl;
   m_lastIsPara = true;
}

void RTFDocVisitor::visit(DocHorRuler *)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visit(DocHorRuler)}\n");
   m_t << "{\\pard\\widctlpar\\brdrb\\brdrs\\brdrw5\\brsp20 \\adjustright \\par}" << endl;
   m_lastIsPara = true;
}

void RTFDocVisitor::visit(DocStyleChange *s)
{
   if (m_hide) {
      return;
   }

   m_lastIsPara = false;
   DBG_RTF("{\\comment RTFDocVisitor::visit(DocStyleChange)}\n");

   switch (s->style()) {
      case DocStyleChange::Bold:
         if (s->enable()) {
            m_t << "{\\b ";
         }      else {
            m_t << "} ";
         }
         break;
      case DocStyleChange::Italic:
         if (s->enable()) {
            m_t << "{\\i ";
         }     else {
            m_t << "} ";
         }
         break;
      case DocStyleChange::Code:
         if (s->enable()) {
            m_t << "{\\f2 ";
         }   else {
            m_t << "} ";
         }
         break;
      case DocStyleChange::Subscript:
         if (s->enable()) {
            m_t << "{\\sub ";
         }    else {
            m_t << "} ";
         }
         break;
      case DocStyleChange::Superscript:
         if (s->enable()) {
            m_t << "{\\super ";
         }    else {
            m_t << "} ";
         }
         break;
      case DocStyleChange::Center:
         if (s->enable()) {
            m_t << "{\\qc ";
         } else {
            m_t << "} ";
         }
         break;
      case DocStyleChange::Small:
         if (s->enable()) {
            m_t << "{\\sub ";
         }  else {
            m_t << "} ";
         }
         break;
      case DocStyleChange::Preformatted:
         if (s->enable()) {
            m_t << "{" << endl;
            m_t << "\\par" << endl;
            m_t << rtf_Style_Reset << getStyle("CodeExample");
            m_insidePre = true;
         } else {
            m_insidePre = false;
            m_t << "\\par";
            m_t << "}" << endl;
         }
         m_lastIsPara = true;
         break;
      case DocStyleChange::Div:  /* HTML only */
         break;
      case DocStyleChange::Span: /* HTML only */
         break;
   }
}

void RTFDocVisitor::visit(DocVerbatim *s)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visit(DocVerbatim)}\n");
   QString lang = m_langExt;

   if (! s->language().isEmpty()) { // explicit language setting
      lang = s->language();
   }

   SrcLangExt langExt = getLanguageFromFileName(lang);
   switch (s->type()) {
      case DocVerbatim::Code: // fall though
         m_t << "{" << endl;
         m_t << "\\par" << endl;
         m_t << rtf_Style_Reset << getStyle("CodeExample");
         Doxy_Globals::parserManager->getParser(lang)
         ->parseCode(m_ci, s->context(), s->text(), langExt,
                     s->isExample(), s->exampleFile());
         //m_t << "\\par" << endl;
         m_t << "}" << endl;
         break;
      case DocVerbatim::Verbatim:
         m_t << "{" << endl;
         m_t << "\\par" << endl;
         m_t << rtf_Style_Reset << getStyle("CodeExample");
         filter(s->text(), true);
         //m_t << "\\par" << endl;
         m_t << "}" << endl;
         break;
      case DocVerbatim::RtfOnly:
         m_t << s->text();
         break;
      case DocVerbatim::HtmlOnly:
      case DocVerbatim::LatexOnly:
      case DocVerbatim::XmlOnly:
      case DocVerbatim::ManOnly:
      case DocVerbatim::DocbookOnly:
         /* nothing */
         break;

      case DocVerbatim::Dot: {
         static int dotindex = 1;

         QString fileName;     
         fileName = QString("%1%2.dot").arg(Config::getString("rtf-output") + "/inline_dotgraph_").arg(dotindex++);

         QFile file(fileName);
         if (! file.open(QIODevice::WriteOnly)) {
            err("Could not open file %s for writing\n", fileName.data());
         }

         file.write( s->text().toUtf8() );
         file.close();

         m_t << "\\par{\\qc "; // center picture
         writeDotFile(fileName);

         m_t << "} ";

         if (Config::getBool("dot-cleanup")) {
            file.remove();
         }
      }
      break;

      case DocVerbatim::Msc: {
         static int mscindex = 1;

         QString baseName = QString("%1%2.msc").arg(Config::getString("rtf-output") + "/inline_mscgraph_").arg(mscindex++);
                         
         QFile file(baseName);
         if (! file.open(QIODevice::WriteOnly)) {
            err("Unable to open file for writing %s, error: %d\n", qPrintable(baseName), file.error());
         }

         QString text = "msc {";
         text += s->text();
         text += "}";

         file.write( text.toUtf8() );
         file.close();

         m_t << "\\par{\\qc "; // center picture
         writeMscFile(baseName);

         m_t << "} ";

         if (Config::getBool("dot-cleanup")) {
            file.remove();
         }
      }
      break;

      case DocVerbatim::PlantUML: {
         static QString rtfOutput = Config::getString("rtf-output");
         QString baseName = writePlantUMLSource(rtfOutput, s->exampleFile(), s->text());

         m_t << "\\par{\\qc "; // center picture
         writePlantUMLFile(baseName);
         m_t << "} ";
      }
      break;
   }
   m_lastIsPara = false;
}

void RTFDocVisitor::visit(DocAnchor *anc)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visit(DocAnchor)}\n");

   QString anchor;
   if (! anc->file().isEmpty()) {
      anchor += anc->file();
   }

   if (!anc->file().isEmpty() && !anc->anchor().isEmpty()) {
      anchor += "_";
   }

   if (!anc->anchor().isEmpty()) {
      anchor += anc->anchor();
   }

   m_t << "{\\bkmkstart " << rtfFormatBmkStr(anchor) << "}" << endl;
   m_t << "{\\bkmkend " << rtfFormatBmkStr(anchor) << "}" << endl;
   m_lastIsPara = false;
}

void RTFDocVisitor::visit(DocInclude *inc)
{
   if (m_hide) {
      return;
   }

   SrcLangExt langExt = getLanguageFromFileName(inc->extension());
   DBG_RTF("{\\comment RTFDocVisitor::visit(DocInclude)}\n");

   switch (inc->type()) {
      case DocInclude::IncWithLines: {
         m_t << "{" << endl;
         m_t << "\\par" << endl;
         m_t << rtf_Style_Reset << getStyle("CodeExample");

         QFileInfo cfi( inc->file() );
         QSharedPointer<FileDef> fd = QMakeShared<FileDef>(cfi.path().toUtf8(), cfi.fileName().toUtf8());

         Doxy_Globals::parserManager->getParser(inc->extension())->parseCode(m_ci, inc->context(),
                     inc->text(), langExt, inc->isExample(), inc->exampleFile(), fd);

         m_t << "\\par";
         m_t << "}" << endl;
      }
      break;
      case DocInclude::Include:
         m_t << "{" << endl;
         m_t << "\\par" << endl;
         m_t << rtf_Style_Reset << getStyle("CodeExample");
         Doxy_Globals::parserManager->getParser(inc->extension())
         ->parseCode(m_ci, inc->context(),
                     inc->text(), langExt, inc->isExample(),
                     inc->exampleFile());
         m_t << "\\par";
         m_t << "}" << endl;
         break;
      case DocInclude::DontInclude:
         break;
      case DocInclude::HtmlInclude:
         break;
      case DocInclude::LatexInclude:
         break;
      case DocInclude::VerbInclude:
         m_t << "{" << endl;
         m_t << "\\par" << endl;
         m_t << rtf_Style_Reset << getStyle("CodeExample");
         filter(inc->text());
         m_t << "\\par";
         m_t << "}" << endl;
         break;
      case DocInclude::Snippet:
         m_t << "{" << endl;
         if (!m_lastIsPara) {
            m_t << "\\par" << endl;
         }
         m_t << rtf_Style_Reset << getStyle("CodeExample");
         Doxy_Globals::parserManager->getParser(inc->extension())
         ->parseCode(m_ci,
                     inc->context(),
                     extractBlock(inc->text(), inc->blockId()),
                     langExt,
                     inc->isExample(),
                     inc->exampleFile()
                    );
         m_t << "}";
         break;
   }
   m_lastIsPara = true;
}

void RTFDocVisitor::visit(DocIncOperator *op)
{
   //printf("DocIncOperator: type=%d first=%d, last=%d text=`%s'\n",
   //    op->type(),op->isFirst(),op->isLast(),op->text().data());
   DBG_RTF("{\\comment RTFDocVisitor::visit(DocIncOperator)}\n");
   SrcLangExt langExt = getLanguageFromFileName(m_langExt);
   if (op->isFirst()) {
      if (!m_hide) {
         m_t << "{" << endl;
         m_t << "\\par" << endl;
         m_t << rtf_Style_Reset << getStyle("CodeExample");
      }
      pushEnabled();
      m_hide = true;
   }
   if (op->type() != DocIncOperator::Skip) {
      popEnabled();
      if (!m_hide) {
         Doxy_Globals::parserManager->getParser(m_langExt)
         ->parseCode(m_ci, op->context(), op->text(), langExt,
                     op->isExample(), op->exampleFile());
      }
      pushEnabled();
      m_hide = true;
   }
   if (op->isLast()) {
      popEnabled();
      if (!m_hide) {
         m_t << "\\par";
         m_t << "}" << endl;
      }
      m_lastIsPara = true;
   } else {
      if (!m_hide) {
         m_t << endl;
      }
      m_lastIsPara = false;
   }
}

void RTFDocVisitor::visit(DocFormula *f)
{
   if (m_hide) {
      return;
   }
   // TODO: do something sensible here, like including a bitmap
   DBG_RTF("{\\comment RTFDocVisitor::visit(DocFormula)}\n");
   m_t << f->text();
   m_lastIsPara = false;
}

void RTFDocVisitor::visit(DocIndexEntry *i)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visit(DocIndexEntry)}\n");
   m_t << "{\\xe \\v " << i->entry() << "}" << endl;
   m_lastIsPara = false;
}

void RTFDocVisitor::visit(DocSimpleSectSep *)
{
}

void RTFDocVisitor::visit(DocCite *cite)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocCite)}\n");

   if (!cite->file().isEmpty()) {
      startLink(cite->ref(), cite->file(), cite->anchor());
   } else {
      m_t << "{\\b ";
   }

   filter(cite->text());
   if (!cite->file().isEmpty()) {
      endLink(cite->ref());
   } else {
      m_t << "}";
   }
}


//--------------------------------------
// visitor functions for compound nodes
//--------------------------------------

void RTFDocVisitor::visitPre(DocAutoList *l)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocAutoList)}\n");
   m_t << "{" << endl;
   rtf_listItemInfo[m_indentLevel].isEnum = l->isEnumList();
   rtf_listItemInfo[m_indentLevel].number = 1;
   m_lastIsPara = false;
}

void RTFDocVisitor::visitPost(DocAutoList *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocAutoList)}\n");
   m_t << "\\par";
   m_t << "}" << endl;
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPre(DocAutoListItem *)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocAutoListItem)}\n");
   if (!m_lastIsPara) {
      m_t << "\\par" << endl;
   }

   m_t << rtf_Style_Reset;
   if (rtf_listItemInfo[m_indentLevel].isEnum) {
      m_t << getStyle("ListEnum") << endl;
      m_t << rtf_listItemInfo[m_indentLevel].number << ".\\tab ";
      rtf_listItemInfo[m_indentLevel].number++;
   } else {
      m_t << getStyle("ListBullet") << endl;
   }

   incIndentLevel();
   m_lastIsPara = false;
}

void RTFDocVisitor::visitPost(DocAutoListItem *)
{
   decIndentLevel();
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocAutoListItem)}\n");
}

void RTFDocVisitor::visitPre(DocPara *)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocPara)}\n");
}

void RTFDocVisitor::visitPost(DocPara *p)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocPara)}\n");

   if (!m_lastIsPara &&
         !p->isLast() &&            // omit <p> for last paragraph
         !(p->parent() &&           // and for parameters & sections
           p->parent()->kind() == DocNode::Kind_ParamSect
          )
      ) {
      m_t << "\\par" << endl;
      m_lastIsPara = true;
   }
}

void RTFDocVisitor::visitPre(DocRoot *r)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocRoot)}\n");

   if (r->indent()) {
      incIndentLevel();
   }
  
   m_t << "{" << rtf_Style["BodyText"].reference << endl;
}

void RTFDocVisitor::visitPost(DocRoot *r)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocRoot)}\n");
   if (!m_lastIsPara && !r->singleLine()) {
      m_t << "\\par" << endl;
   }
   m_t << "}";
   m_lastIsPara = true;
   if (r->indent()) {
      decIndentLevel();
   }
}

void RTFDocVisitor::visitPre(DocSimpleSect *s)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocSimpleSect)}\n");
   if (!m_lastIsPara) {
      m_t << "\\par" << endl;
   }

   m_t << "{"; 

   // start desc

   m_t << "{" << rtf_Style["Heading5"].reference << endl;

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
      m_t << ":";
      m_t << "\\par";
      m_t << "}"; // end bold
      incIndentLevel();
      m_t << rtf_Style_Reset << getStyle("DescContinue");
   }
   m_lastIsPara = false;
}

void RTFDocVisitor::visitPost(DocSimpleSect *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocSimpleSect)}\n");
   if (!m_lastIsPara) {
      m_t << "\\par" << endl;
   }
   decIndentLevel();
   m_t << "}"; // end desc
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPre(DocTitle *)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocTitle)}\n");
}

void RTFDocVisitor::visitPost(DocTitle *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocTitle)}\n");
   m_t << "\\par" << endl;
   m_t << "}"; // end bold
   incIndentLevel();
   m_t << rtf_Style_Reset << getStyle("DescContinue");
   m_lastIsPara = false;
}

void RTFDocVisitor::visitPre(DocSimpleList *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocSimpleSect)}\n");
   m_t << "{" << endl;
   rtf_listItemInfo[m_indentLevel].isEnum = false;
   m_lastIsPara = false;
}

void RTFDocVisitor::visitPost(DocSimpleList *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocSimpleSect)}\n");
   if (!m_lastIsPara) {
      m_t << "\\par" << endl;
   }
   m_t << "}" << endl;
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPre(DocSimpleListItem *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocSimpleListItem)}\n");
   m_t << "\\par" << rtf_Style_Reset << getStyle("ListBullet") << endl;
   m_lastIsPara = false;
   incIndentLevel();
}

void RTFDocVisitor::visitPost(DocSimpleListItem *)
{
   decIndentLevel();
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocSimpleListItem)}\n");
}

void RTFDocVisitor::visitPre(DocSection *s)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocSection)}\n");
   if (!m_lastIsPara) {
      m_t << "\\par" << endl;
   }
   m_t << "{\\bkmkstart " << rtfFormatBmkStr(s->file() + "_" + s->anchor()) << "}" << endl;
   m_t << "{\\bkmkend " << rtfFormatBmkStr(s->file() + "_" + s->anchor()) << "}" << endl;
   m_t << "{{" // start section
       << rtf_Style_Reset;

   QString heading;
   int level = qMin(s->level() + 1, 4);

   heading = QString("Heading%1").arg(level).toUtf8();
   
   // set style
   m_t << rtf_Style[heading].reference << endl;

   // make table of contents entry

   filter(s->title());
   m_t << endl << "\\par" << "}" << endl;
   m_t << "{\\tc\\tcl" << level << " \\v ";
   filter(s->title());
   m_t << "}" << endl;
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPost(DocSection *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocSection)}\n");
   m_t << "\\par}" << endl; // end section
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPre(DocHtmlList *l)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocHtmlList)}\n");
   m_t << "{" << endl;
   rtf_listItemInfo[m_indentLevel].isEnum = l->type() == DocHtmlList::Ordered;
   rtf_listItemInfo[m_indentLevel].number = 1;
   m_lastIsPara = false;
}

void RTFDocVisitor::visitPost(DocHtmlList *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocHtmlList)}\n");
   m_t << "\\par" << "}" << endl;
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPre(DocHtmlListItem *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocHtmlListItem)}\n");
   m_t << "\\par" << endl;
   m_t << rtf_Style_Reset;
   if (rtf_listItemInfo[m_indentLevel].isEnum) {
      m_t << getStyle("ListEnum") << endl;
      m_t << rtf_listItemInfo[m_indentLevel].number << ".\\tab ";
      rtf_listItemInfo[m_indentLevel].number++;
   } else {
      m_t << getStyle("ListBullet") << endl;
   }
   incIndentLevel();
   m_lastIsPara = false;
}

void RTFDocVisitor::visitPost(DocHtmlListItem *)
{
   decIndentLevel();
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocHtmlListItem)}\n");
}

void RTFDocVisitor::visitPre(DocHtmlDescList *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocHtmlDescList)}\n");
   //m_t << "{" << endl;
   //m_t << rtf_Style_Reset << getStyle("ListContinue");
   //m_lastIsPara=false;
}

void RTFDocVisitor::visitPost(DocHtmlDescList *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocHtmlDescList)}\n");
   //m_t << "}" << endl;
   //m_t << "\\par" << endl;
   //m_lastIsPara=true;
}

void RTFDocVisitor::visitPre(DocHtmlDescTitle *)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocHtmlDescTitle)}\n");
  
   m_t << "{" << rtf_Style["Heading5"].reference << endl;
   m_lastIsPara = false;
}

void RTFDocVisitor::visitPost(DocHtmlDescTitle *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocHtmlDescTitle)}\n");
   m_t << "\\par" << endl;
   m_t << "}" << endl;
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPre(DocHtmlDescData *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocHtmlDescData)}\n");
   incIndentLevel();
   m_t << "{" << rtf_Style_Reset << getStyle("DescContinue");
}

void RTFDocVisitor::visitPost(DocHtmlDescData *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocHtmlDescData)}\n");
   m_t << "\\par";
   m_t << "}" << endl;
   decIndentLevel();
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPre(DocHtmlTable *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocHtmlTable)}\n");
   if (!m_lastIsPara) {
      m_t << "\\par" << endl;
   }
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPost(DocHtmlTable *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocHtmlTable)}\n");
   m_t << "\\pard\\plain" << endl;
   m_t << "\\par" << endl;
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPre(DocHtmlCaption *)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocHtmlCaption)}\n");
}

void RTFDocVisitor::visitPost(DocHtmlCaption *)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocHtmlCaption)}\n");
}

void RTFDocVisitor::visitPre(DocHtmlRow *r)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocHtmlRow)}\n");
   uint i, columnWidth = r->numCells() > 0 ? rtf_pageWidth / r->numCells() : 10;
   m_t << "\\trowd \\trgaph108\\trleft-108"
       "\\trbrdrt\\brdrs\\brdrw10 "
       "\\trbrdrl\\brdrs\\brdrw10 "
       "\\trbrdrb\\brdrs\\brdrw10 "
       "\\trbrdrr\\brdrs\\brdrw10 "
       "\\trbrdrh\\brdrs\\brdrw10 "
       "\\trbrdrv\\brdrs\\brdrw10 " << endl;
   for (i = 0; i < r->numCells(); i++) {
      if (r->isHeading()) {
         m_t << "\\clcbpat16"; // set cell shading to light grey (color 16 in the clut)
      }
      m_t << "\\clvertalt\\clbrdrt\\brdrs\\brdrw10 "
          "\\clbrdrl\\brdrs\\brdrw10 "
          "\\clbrdrb\\brdrs\\brdrw10 "
          "\\clbrdrr \\brdrs\\brdrw10 "
          "\\cltxlrtb "
          "\\cellx" << ((i + 1)*columnWidth) << endl;
   }
   m_t << "\\pard \\widctlpar\\intbl\\adjustright" << endl;
   m_lastIsPara = false;
}

void RTFDocVisitor::visitPost(DocHtmlRow *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocHtmlRow)}\n");
   m_t << endl;
   m_t << "\\pard \\widctlpar\\intbl\\adjustright" << endl;
   m_t << "{\\row }" << endl;
   m_lastIsPara = false;
}

void RTFDocVisitor::visitPre(DocHtmlCell *c)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocHtmlCell)}\n");
   m_t << "{" << align(c);
   m_lastIsPara = false;
}

void RTFDocVisitor::visitPost(DocHtmlCell *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocHtmlCell)}\n");
   m_t << "\\cell }";
   m_lastIsPara = false;
}

void RTFDocVisitor::visitPre(DocInternal *)
{
   if (m_hide) {
      return;
   }
   //DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocInternal)}\n");
   //m_t << "{"; // start desc
   //m_t << "{\\b "; // start bold
   //m_t << theTranslator->trForInternalUseOnly();
   //m_t << "}"; // end bold
   //m_t << "\\par" << endl;
   //incIndentLevel();
   //m_t << rtf_Style_Reset << getStyle("DescContinue");
   //m_lastIsPara=false;
}

void RTFDocVisitor::visitPost(DocInternal *)
{
   if (m_hide) {
      return;
   }
   //DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocInternal)}\n");
   //m_t << "\\par";
   //decIndentLevel();
   //m_t << "}"; // end desc
   //m_lastIsPara=true;
}

void RTFDocVisitor::visitPre(DocHRef *href)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocHRef)}\n");

   if (Config::getBool("rtf-hyperlinks")) {
      m_t << "{\\field "
          "{\\*\\fldinst "
          "{ HYPERLINK \"" << href->url() << "\" "
          "}{}"
          "}"
          "{\\fldrslt "
          "{\\cs37\\ul\\cf2 ";

   } else {
      m_t << "{\\f2 ";
   }

   m_lastIsPara = false;
}

void RTFDocVisitor::visitPost(DocHRef *)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocHRef)}\n");

   if (Config::getBool("rtf-hyperlinks")) {
      m_t <<     "}"
          "}"
          "}";
   } else {
      m_t << "}";
   }
   m_lastIsPara = false;
}

void RTFDocVisitor::visitPre(DocHtmlHeader *header)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocHtmlHeader)}\n");
   m_t << "{" // start section
       << rtf_Style_Reset;

   QString heading;
   int level = qMin(header->level() + 2, 4);
   heading = QString("Heading%1").arg(level).toUtf8();

   // set style
   m_t << rtf_Style[heading].reference;

   // make table of contents entry
   m_t << "{\\tc\\tcl \\v " << level << "}";
   m_lastIsPara = false;

}

void RTFDocVisitor::visitPost(DocHtmlHeader *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocHtmlHeader)}\n");
   m_t << "\\par";
   m_t << "}" << endl; // end section
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPre(DocImage *img)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocImage)}\n");
   if (img->type() == DocImage::Rtf) {
      m_t << "\\par" << endl;
      m_t << "{" << endl;
      m_t << rtf_Style_Reset << endl;
      m_t << "\\par\\pard \\qc {\\field\\flddirty {\\*\\fldinst INCLUDEPICTURE \"";
      m_t << img->name();
      m_t << "\" \\\\d \\\\*MERGEFORMAT}{\\fldrslt IMAGE}}\\par" << endl;
      m_t << "}" << endl;
      m_lastIsPara = true;
   } else { // other format -> skip
   }
   // hide caption since it is not supported at the moment
   pushEnabled();
   m_hide = true;
}

void RTFDocVisitor::visitPost(DocImage *)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocImage)}\n");
   popEnabled();
}

void RTFDocVisitor::visitPre(DocDotFile *df)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocDotFile)}\n");
   writeDotFile(df->file());

   // hide caption since it is not supported at the moment
   pushEnabled();
   m_hide = true;
}

void RTFDocVisitor::visitPost(DocDotFile *)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocDotFile)}\n");
   popEnabled();
}
void RTFDocVisitor::visitPre(DocMscFile *df)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocMscFile)}\n");
   writeMscFile(df->file());

   // hide caption since it is not supported at the moment
   pushEnabled();
   m_hide = true;
}

void RTFDocVisitor::visitPost(DocMscFile *)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocMscFile)}\n");
   popEnabled();
}

void RTFDocVisitor::visitPre(DocDiaFile *df)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocDiaFile)}\n");
   writeDiaFile(df->file());

   // hide caption since it is not supported at the moment
   pushEnabled();
   m_hide = true;
}

void RTFDocVisitor::visitPost(DocDiaFile *)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocDiaFile)}\n");
   popEnabled();
}

void RTFDocVisitor::visitPre(DocLink *lnk)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocLink)}\n");
   startLink(lnk->ref(), lnk->file(), lnk->anchor());
}

void RTFDocVisitor::visitPost(DocLink *lnk)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocLink)}\n");
   endLink(lnk->ref());
}

void RTFDocVisitor::visitPre(DocRef *ref)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocRef)}\n");
   // when ref->isSubPage()==true we use ref->file() for HTML and
   // ref->anchor() for LaTeX/RTF
   if (ref->isSubPage()) {
      startLink(ref->ref(), 0, ref->anchor());
   } else {
      if (!ref->file().isEmpty()) {
         startLink(ref->ref(), ref->file(), ref->anchor());
      }
   }
   if (!ref->hasLinkText()) {
      filter(ref->targetTitle());
   }
}

void RTFDocVisitor::visitPost(DocRef *ref)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocRef)}\n");
   if (!ref->file().isEmpty()) {
      endLink(ref->ref());
   }
   //m_t << " ";
}


void RTFDocVisitor::visitPre(DocSecRefItem *)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocSecRefItem)}\n");
}

void RTFDocVisitor::visitPost(DocSecRefItem *)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocSecRefItem)}\n");
}

void RTFDocVisitor::visitPre(DocSecRefList *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocSecRefList)}\n");
   m_t << "{" << endl;
   incIndentLevel();
   m_t << rtf_Style_Reset << getStyle("LatexTOC") << endl;
   m_t << "\\par" << endl;
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPost(DocSecRefList *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocSecRefList)}\n");
   decIndentLevel();
   m_t << "\\par";
   m_t << "}" << endl;
   m_lastIsPara = true;
}

//void RTFDocVisitor::visitPre(DocLanguage *l)
//{
//  DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocLanguage)}\n");
//  QByteArray langId = Config::getEnum("output-language");
//  if (l->id().toLower()!=langId.lower())
//  {
//    pushEnabled();
//    m_hide = true;
//  }
//}
//
//void RTFDocVisitor::visitPost(DocLanguage *l)
//{
//  DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocLanguage)}\n");
//  QByteArray langId = Config::getEnum("output-language");
//  if (l->id().toLower()!=langId.lower())
//  {
//    popEnabled();
//  }
//}

void RTFDocVisitor::visitPre(DocParamSect *s)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocParamSect)}\n");
   m_t << "{"; // start param list
   if (!m_lastIsPara) {
      m_t << "\\par" << endl;
   }
   //m_t << "{\\b "; // start bold
   m_t << "{" << rtf_Style["Heading5"].reference << endl;

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
   m_t << ":";
   m_t << "\\par";
   m_t << "}" << endl;
   bool useTable = s->type() == DocParamSect::Param ||
                   s->type() == DocParamSect::RetVal ||
                   s->type() == DocParamSect::Exception ||
                   s->type() == DocParamSect::TemplateParam;
   if (!useTable) {
      incIndentLevel();
   }
   m_t << rtf_Style_Reset << getStyle("DescContinue");
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPost(DocParamSect *s)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocParamSect)}\n");
   //m_t << "\\par" << endl;
   bool useTable = s->type() == DocParamSect::Param ||
                   s->type() == DocParamSect::RetVal ||
                   s->type() == DocParamSect::Exception ||
                   s->type() == DocParamSect::TemplateParam;
   if (!useTable) {
      decIndentLevel();
   }
   m_t << "}" << endl;
}

void RTFDocVisitor::visitPre(DocParamList *pl)
{
   static int columnPos[4][5] = {
      { 2, 25, 100, 100, 100 }, // no inout, no type
      { 3, 14,  35, 100, 100 }, // inout, no type
      { 3, 25,  50, 100, 100 }, // no inout, type
      { 4, 14,  35, 55,  100 }, // inout, type
   };
   int config = 0;
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocParamList)}\n");

   DocParamSect::Type parentType = DocParamSect::Unknown;
   DocParamSect *sect = 0;
   if (pl->parent() && pl->parent()->kind() == DocNode::Kind_ParamSect) {
      parentType = ((DocParamSect *)pl->parent())->type();
      sect = (DocParamSect *)pl->parent();
   }
   bool useTable = parentType == DocParamSect::Param ||
                   parentType == DocParamSect::RetVal ||
                   parentType == DocParamSect::Exception ||
                   parentType == DocParamSect::TemplateParam;
   if (sect && sect->hasInOutSpecifier()) {
      config += 1;
   }
   if (sect && sect->hasTypeSpecifier()) {
      config += 2;
   }
   if (useTable) {
      int i;
      m_t << "\\trowd \\trgaph108\\trleft426\\tblind426"
          "\\trbrdrt\\brdrs\\brdrw10\\brdrcf15 "
          "\\trbrdrl\\brdrs\\brdrw10\\brdrcf15 "
          "\\trbrdrb\\brdrs\\brdrw10\\brdrcf15 "
          "\\trbrdrr\\brdrs\\brdrw10\\brdrcf15 "
          "\\trbrdrh\\brdrs\\brdrw10\\brdrcf15 "
          "\\trbrdrv\\brdrs\\brdrw10\\brdrcf15 " << endl;
      for (i = 0; i < columnPos[config][0]; i++) {
         m_t << "\\clvertalt\\clbrdrt\\brdrs\\brdrw10\\brdrcf15 "
             "\\clbrdrl\\brdrs\\brdrw10\\brdrcf15 "
             "\\clbrdrb\\brdrs\\brdrw10\\brdrcf15 "
             "\\clbrdrr \\brdrs\\brdrw10\\brdrcf15 "
             "\\cltxlrtb "
             "\\cellx" << (rtf_pageWidth * columnPos[config][i + 1] / 100) << endl;
      }
      m_t << "\\pard \\widctlpar\\intbl\\adjustright" << endl;
   }

   if (sect && sect->hasInOutSpecifier()) {
      if (useTable) {
         m_t << "{";
      }

      // Put in the direction: in/out/in,out if specified.
      if (pl->direction() != DocParamSect::Unspecified) {
         if (pl->direction() == DocParamSect::In) {
            m_t << "in";
         } else if (pl->direction() == DocParamSect::Out) {
            m_t << "out";
         } else if (pl->direction() == DocParamSect::InOut) {
            m_t << "in,out";
         }
      }

      if (useTable) {
         m_t << "\\cell }";
      }
   }

   if (sect && sect->hasTypeSpecifier()) {
      if (useTable) {
         m_t << "{";
      }
   

      bool first = true;

      for (auto type : pl->paramTypes() ) {
         if (!first) {
            m_t << " | ";
         } else {
            first = false;
         }
         if (type->kind() == DocNode::Kind_Word) {
            visit((DocWord *)type);
         } else if (type->kind() == DocNode::Kind_LinkedWord) {
            visit((DocLinkedWord *)type);
         }
      }
      if (useTable) {
         m_t << "\\cell }";
      }
   }


   if (useTable) {
      m_t << "{";
   }

   m_t << "{\\i ";
  
   
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
   m_t << "} ";

   if (useTable) {
      m_t << "\\cell }{";
   }
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPost(DocParamList *pl)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocParamList)}\n");

   DocParamSect::Type parentType = DocParamSect::Unknown;
   //DocParamSect *sect = 0;
   if (pl->parent() && pl->parent()->kind() == DocNode::Kind_ParamSect) {
      parentType = ((DocParamSect *)pl->parent())->type();
      //sect=(DocParamSect*)pl->parent();
   }
   bool useTable = parentType == DocParamSect::Param ||
                   parentType == DocParamSect::RetVal ||
                   parentType == DocParamSect::Exception ||
                   parentType == DocParamSect::TemplateParam;
   if (useTable) {
      m_t << "\\cell }" << endl;
      //m_t << "\\pard \\widctlpar\\intbl\\adjustright" << endl;
      m_t << "{\\row }" << endl;
   } else {
      m_t << "\\par" << endl;
   }

   m_lastIsPara = true;
}

void RTFDocVisitor::visitPre(DocXRefItem *x)
{
   if (m_hide) {
      return;
   }
   if (x->title().isEmpty()) {
      return;
   }

   bool anonymousEnum = x->file() == "@";
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocXRefItem)}\n");

   m_t << "{"; 

   // start param list
   //m_t << "{\\b "; // start bold

   m_t << "{" << rtf_Style["Heading5"].reference << endl;

   if (Config::getBool("rtf-hyperlinks") && ! anonymousEnum) {
      QString refName;

      if (!x->file().isEmpty()) {
         refName += x->file();
      }
      if (!x->file().isEmpty() && !x->anchor().isEmpty()) {
         refName += "_";
      }
      if (!x->anchor().isEmpty()) {
         refName += x->anchor();
      }

      m_t << "{\\field "
          "{\\*\\fldinst "
          "{ HYPERLINK  \\\\l \"" << refName << "\" "
          "}{}"
          "}"
          "{\\fldrslt "
          "{\\cs37\\ul\\cf2 ";
      filter(x->title());
      m_t <<     "}"
          "}"
          "}";
   } else {
      filter(x->title());
   }
   m_t << ":";
   m_t << "\\par";
   m_t << "}"; // end bold
   incIndentLevel();
   m_t << rtf_Style_Reset << getStyle("DescContinue");
   m_lastIsPara = false;
}

void RTFDocVisitor::visitPost(DocXRefItem *x)
{
   if (m_hide) {
      return;
   }
   if (x->title().isEmpty()) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocXRefItem)}\n");
   m_t << "\\par" << endl;
   decIndentLevel();
   m_t << "}" << endl; // end xref item
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPre(DocInternalRef *ref)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocInternalRef)}\n");
   startLink("", ref->file(), ref->anchor());
}

void RTFDocVisitor::visitPost(DocInternalRef *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocInternalRef)}\n");
   endLink("");
   m_t << " ";
}

void RTFDocVisitor::visitPre(DocCopy *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocCopy)}\n");
}

void RTFDocVisitor::visitPost(DocCopy *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocCopy)}\n");
}

void RTFDocVisitor::visitPre(DocText *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocText)}\n");
}

void RTFDocVisitor::visitPost(DocText *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocText)}\n");
}

void RTFDocVisitor::visitPre(DocHtmlBlockQuote *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocHtmlBlockQuote)}\n");
   if (!m_lastIsPara) {
      m_t << "\\par" << endl;
   }
   m_t << "{"; // start desc
   incIndentLevel();
   m_t << rtf_Style_Reset << getStyle("DescContinue");
}

void RTFDocVisitor::visitPost(DocHtmlBlockQuote *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocHtmlBlockQuote)}\n");
   if (!m_lastIsPara) {
      m_t << "\\par" << endl;
   }
   decIndentLevel();
   m_t << "}"; // end desc
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPre(DocParBlock *)
{
   if (m_hide) {
      return;
   }
}

void RTFDocVisitor::visitPost(DocParBlock *)
{
   if (m_hide) {
      return;
   }
}

void RTFDocVisitor::filter(const QString &str, bool verbatim)
{
   if (str.isEmpty()) {
      return;
   }

   for (auto c : str) {   
      switch (c.unicode()) {
         case '{':
            m_t << "\\{";
            break;

         case '}':
            m_t << "\\}";
            break;

         case '\\':
            m_t << "\\\\";
            break;

         case '\n':
            if (verbatim) {
               m_t << "\\par" << endl;
            } else {
               m_t << '\n';
            }
            break;

         default:
            m_t << c;        
         
      }
   }
}

void RTFDocVisitor::startLink(const QString &ref, const QString &file, const QString &anchor)
{
   if (ref.isEmpty() && Config::getBool("rtf-hyperlinks")) {

      QString refName;

      if (! file.isEmpty()) {
         refName += file;
      }

      if (! file.isEmpty() && ! anchor.isEmpty() ) {
         refName += '_';
      }

      if (! anchor.isEmpty()) {
         refName += anchor;
      }

      m_t << "{\\field {\\*\\fldinst { HYPERLINK  \\\\l \"";
      m_t << rtfFormatBmkStr(refName);
      m_t << "\" }{}";
      m_t << "}{\\fldrslt {\\cs37\\ul\\cf2 ";

   } else {
      m_t << "{\\b ";
   }
   m_lastIsPara = false;
}

void RTFDocVisitor::endLink(const QString &ref)
{
   if (ref.isEmpty() && Config::getBool("rtf-hyperlinks")) {
      m_t << "}}}";
   } else {
      m_t << "}";
   }
   m_lastIsPara = false;
}

void RTFDocVisitor::pushEnabled()
{
   m_enabled.push(m_hide);
}

void RTFDocVisitor::popEnabled()
{
   bool v = m_enabled.pop();
   m_hide = v;  
}

void RTFDocVisitor::writeDotFile(const QString &fileName)
{
   QString baseName = fileName;

   int i;

   if ((i = baseName.lastIndexOf('/')) != -1) {
      baseName = baseName.right(baseName.length() - i - 1);
   }

   QString outDir = Config::getString("rtf-output");
   writeDotGraphFromFile(fileName, outDir, baseName, GOF_BITMAP);

   if (!m_lastIsPara) {
      m_t << "\\par" << endl;
   }

   m_t << "{" << endl;
   m_t << rtf_Style_Reset;
   m_t << "\\pard \\qc {\\field\\flddirty {\\*\\fldinst INCLUDEPICTURE \"";
   m_t << baseName << "." << Config::getEnum("dot-image-format").toUtf8();
   m_t << "\" \\\\d \\\\*MERGEFORMAT}{\\fldrslt IMAGE}}\\par" << endl;
   m_t << "}" << endl;
   m_lastIsPara = true;
}

void RTFDocVisitor::writeMscFile(const QString &fileName)
{
   QString baseName = fileName;
   int i;

   if ((i = baseName.lastIndexOf('/')) != -1) {
      baseName = baseName.right(baseName.length() - i - 1);
   }

   QString outDir = Config::getString("rtf-output");
   writeMscGraphFromFile(fileName, outDir, baseName, MSC_BITMAP);

   if (! m_lastIsPara) {
      m_t << "\\par" << endl;
   }

   m_t << "{" << endl;
   m_t << rtf_Style_Reset;
   m_t << "\\pard \\qc {\\field\\flddirty {\\*\\fldinst INCLUDEPICTURE \"";
   m_t << baseName << ".png";
   m_t << "\" \\\\d \\\\*MERGEFORMAT}{\\fldrslt IMAGE}}\\par" << endl;
   m_t << "}" << endl;
   m_lastIsPara = true;
}

void RTFDocVisitor::writeDiaFile(const QString &fileName)
{
   QString baseName = fileName;
   int i;

   if ((i = baseName.lastIndexOf('/')) != -1) {
      baseName = baseName.right(baseName.length() - i - 1);
   }

   QString outDir = Config::getString("rtf-output");
   writeDiaGraphFromFile(fileName, outDir, baseName, DIA_BITMAP);

   if (!m_lastIsPara) {
      m_t << "\\par" << endl;
   }
   m_t << "{" << endl;
   m_t << rtf_Style_Reset;
   m_t << "\\pard \\qc {\\field\\flddirty {\\*\\fldinst INCLUDEPICTURE \"";
   m_t << baseName << ".png";
   m_t << "\" \\\\d \\\\*MERGEFORMAT}{\\fldrslt IMAGE}}\\par" << endl;
   m_t << "}" << endl;
   m_lastIsPara = true;
}

void RTFDocVisitor::writePlantUMLFile(const QString &fileName)
{
   QString baseName = fileName;
   int i;

   if ((i = baseName.lastIndexOf('/')) != -1) {
      baseName = baseName.right(baseName.length() - i - 1);   
   }

   QString outDir = Config::getString("rtf-output");
   generatePlantUMLOutput(fileName, outDir, PUML_BITMAP);

   if (!m_lastIsPara) {
      m_t << "\\par" << endl;
   }
   m_t << "{" << endl;
   m_t << rtf_Style_Reset;
   m_t << "\\pard \\qc {\\field\\flddirty {\\*\\fldinst INCLUDEPICTURE \"";
   m_t << baseName << ".png";
   m_t << "\" \\\\d \\\\*MERGEFORMAT}{\\fldrslt IMAGE}}\\par" << endl;
   m_t << "}" << endl;
   m_lastIsPara = true;
}

