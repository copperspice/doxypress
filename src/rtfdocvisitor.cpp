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

#include <QFileInfo>

#include <rtfdocvisitor.h>

#include <config.h>
#include <dia.h>
#include <docparser.h>
#include <dot.h>
#include <doxy_globals.h>
#include <htmlentity.h>
#include <emoji_entity.h>
#include <language.h>
#include <msc.h>
#include <msc.h>
#include <message.h>
#include <outputgen.h>
#include <parse_base.h>
#include <plantuml.h>
#include <rtfstyle.h>
#include <util.h>

// #define DBG_RTF(x) m_t << x
#define DBG_RTF(x) do {} while(0)

static QString align(DocHtmlCell *cell)
{
   QString retval;

   HtmlAttribList attrs = cell->attribs();

   for (const auto &item : attrs) {

      if (item.name.toLower() == "align") {
         QString tmp = item.value.toLower();

         if (tmp == "center") {
            retval = "\\qc ";
            break;

         } else if (tmp == "right") {
            retval = "\\qr ";
            break;

         } else {
            break;

         }
      }
   }

   return retval;
}

RTFDocVisitor::RTFDocVisitor(QTextStream &t, CodeGenerator &ci, const QString &langExt)
   : DocVisitor(DocVisitor_RTF), m_t(t), m_ci(ci), m_insidePre(false),
     m_hide(false), m_indentLevel(0), m_lastIsPara(false), m_langExt(langExt)
{
}

QString RTFDocVisitor::getStyle(const QString &name)
{
   QString n = QString("%1%2").formatArg(name).formatArg(m_indentLevel);

   auto sd = rtf_Style.find(n);

   if (sd == rtf_Style.end()) {
      assert(sd != rtf_Style.end());
   }

   return sd->m_reference;
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

   if (! res.isEmpty()) {
      m_t << res;

   } else {
      err("RTF, Unsupported HTML entity found: %s\n", csPrintable(HtmlEntityMapper::instance()->html(s->symbol(), true)) );
   }

   m_lastIsPara = false;
}

void RTFDocVisitor::visit(DocEmoji *s)
{
   if (m_hide) {
      return;
   }

   const QString result = EmojiEntityMapper::instance()->unicode(s->index());

   if (! result.isEmpty()) {

      int valA = 0;
      int valB = 0;

      for (auto ch : result) {

         switch(ch.unicode()) {
            case '&':
            case '#':
            case 'x':
               break;

            case ';':
               valB = valA;
               valA = 0xd800 + ( ( valB - 0x10000 ) & 0xffc00 ) / 0x400 - 0x10000;
               m_t << "\\u" << valA << "?";

               valA = 0xdC00 + ( ( valB - 0x10000 ) & 0x3ff ) - 0x10000 ;
               m_t << "\\u" << valA << "?";

               valA = 0;

               break;

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
               valA = valA * 16 + ch.unicode() - '0';
               break;

            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
               valA = valA * 16 + ch.unicode() - 'a' + 10;
               break;
         }
      }

   } else {
      m_t << s->name();
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

      m_t << "}"
             "{\\fldrslt "
             "{\\cs37\\ul\\cf2 ";

      filter(u->url());

      m_t << "}"
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
         } else {
            m_t << "} ";
         }
         break;

      case DocStyleChange::S:
      case DocStyleChange::Strike:
      case DocStyleChange::Del:
         if (s->enable()) {
            m_t << "{\\strike ";
         } else {
            m_t << "} ";
         }

         break;

      case DocStyleChange::Underline:
      case DocStyleChange::Ins:
         if (s->enable()) {
            m_t << "{\\ul ";
         } else {
            m_t << "} ";
         }
         break;

      case DocStyleChange::Italic:
         if (s->enable()) {
            m_t << "{\\i ";
         } else {
            m_t << "} ";
         }
         break;

      case DocStyleChange::Code:
         if (s->enable()) {
            m_t << "{\\f2 ";
         } else {
            m_t << "} ";
         }
         break;

      case DocStyleChange::Subscript:
         if (s->enable()) {
            m_t << "{\\sub ";
         } else {
            m_t << "} ";
         }
         break;

      case DocStyleChange::Superscript:
         if (s->enable()) {
            m_t << "{\\super ";
         } else {
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

      case DocStyleChange::Cite:
         if (s->enable()) {
            m_t << "{\\i ";
         } else {
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


      // HTML only
      case DocStyleChange::Div:
      case DocStyleChange::Span:
         break;

      case DocStyleChange::Details:
         // emulation of the <details> tag
         if (s->enable()) {
            m_t << "{\n";
            m_t << "\\par\n";
         } else {
            m_t << "\\par";
            m_t << "}\n";
         }

         m_lastIsPara = true;
         break;

      case DocStyleChange::Summary:
         // emulation of the <summary> tag inside a <details> tag
         if (s->enable()) {
            m_t << "{\\b ";
         } else {
            m_t << "}\\par ";
         }
         break;
   }
}

static void visitCaption(RTFDocVisitor *parent, QList<DocNode *> children)
{
   for (auto n : children) {
      n->accept(parent);
   }
}

void RTFDocVisitor::visit(DocVerbatim *s)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visit(DocVerbatim)}\n");

   QString lang = m_langExt;

   if (! s->language().isEmpty()) {
      // explicit language setting
      lang = s->language();
   }

   SrcLangExt langExt = getLanguageFromFileName(lang);

   switch (s->type()) {
      case DocVerbatim::Code: // fall though
         m_t << "{" << endl;
         m_t << "\\par" << endl;
         m_t << rtf_Style_Reset << getStyle("CodeExample");

         Doxy_Globals::parserManager.getParser(lang)->parseCode(m_ci, s->context(),
               s->text(), langExt, s->isExample(), s->exampleFile());

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
         fileName = QString("%1%2.dot").formatArg(Config::getString("rtf-output") + "/inline_dotgraph_").formatArg(dotindex++);

         QFile file(fileName);
         if (! file.open(QIODevice::WriteOnly)) {
            err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), file.error());
         }

         file.write( s->text().toUtf8() );
         file.close();

         writeDotFile(fileName, s->hasCaption());
         visitCaption(this, s->children());
         includePicturePostRTF(true, s->hasCaption());

         if (Config::getBool("dot-cleanup")) {
            file.remove();
         }
      }
      break;

      case DocVerbatim::Msc: {
         static int mscindex = 1;

         QString baseName = QString("%1%2.msc").formatArg(Config::getString("rtf-output") + "/inline_mscgraph_").formatArg(mscindex++);

         QFile file(baseName);
         if (! file.open(QIODevice::WriteOnly)) {
            err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(baseName), file.error());
         }

         QString text = "msc {";
         text += s->text();
         text += "}";

         file.write( text.toUtf8() );
         file.close();

         writeMscFile(baseName, s->hasCaption());
         visitCaption(this, s->children());
         includePicturePostRTF(true, s->hasCaption());

         if (Config::getBool("dot-cleanup")) {
            file.remove();
         }
      }
      break;

      case DocVerbatim::PlantUML: {
         static QString rtfOutput = Config::getString("rtf-output");
         QString baseName = writePlantUMLSource(rtfOutput, s->exampleFile(), s->text());

         writePlantUMLFile(baseName, s->hasCaption());
         visitCaption(this, s->children());
         includePicturePostRTF(true, s->hasCaption());
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

   if (! anc->file().isEmpty() && ! anc->anchor().isEmpty()) {
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
         QSharedPointer<FileDef> fd = QMakeShared<FileDef>(cfi.path(), cfi.fileName());

         Doxy_Globals::parserManager.getParser(inc->extension())->parseCode(m_ci, inc->context(),
                     inc->text(), langExt, inc->isExample(), inc->exampleFile(), fd,
                     -1, -1, false, QSharedPointer<MemberDef>(), true);

         m_t << "\\par";
         m_t << "}" << endl;
      }
      break;

      case DocInclude::Include:
         m_t << "{" << endl;
         m_t << "\\par" << endl;
         m_t << rtf_Style_Reset << getStyle("CodeExample");

         Doxy_Globals::parserManager.getParser(inc->extension())->parseCode(m_ci, inc->context(),
                     inc->text(), langExt, inc->isExample(),inc->exampleFile(), QSharedPointer<FileDef>(),
                     -1, -1, true, QSharedPointer<MemberDef>(), false);

         m_t << "\\par";
         m_t << "}" << endl;
         break;

      case DocInclude::DontInclude:
      case DocInclude::DontIncWithLines:
      case DocInclude::HtmlInclude:
      case DocInclude::LatexInclude:
      case DocInclude::ManInclude:
      case DocInclude::XmlInclude:
      case DocInclude::DocbookInclude:
         break;

      case DocInclude::RtfInclude:
         m_t << inc->text();
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

         if (! m_lastIsPara) {
            m_t << "\\par" << endl;
         }

         m_t << rtf_Style_Reset << getStyle("CodeExample");

         Doxy_Globals::parserManager.getParser(inc->extension())->parseCode(m_ci, inc->context(),
                  extractBlock(inc->text(), inc->blockId()), langExt,
                  inc->isExample(), inc->exampleFile());

         m_t << "}";
         break;

      case DocInclude::SnipWithLines: {
         m_t << "{" << endl;

         if (! m_lastIsPara) {
            m_t << "\\par" << endl;
         }

         m_t << rtf_Style_Reset << getStyle("CodeExample");

         QFileInfo cfi(inc->file());
         QSharedPointer<FileDef> fd = QMakeShared<FileDef>(cfi.path(), cfi.fileName());

         Doxy_Globals::parserManager.getParser(inc->extension())->parseCode(m_ci, inc->context(),
                  extractBlock(inc->text(),inc->blockId()), langExt,
                  inc->isExample(), inc->exampleFile(), fd,
                  lineBlock(inc->text(), inc->blockId()), -1, false, QSharedPointer<MemberDef>(), true);

         m_t << "}";
      }
      break;

      case DocInclude::IncludeDoc:
      case DocInclude::SnippetDoc:
         err("Unexpected command found for IncludeDoc or SnippetDoc in file: %s,"
               " contact the developers\n", csPrintable(inc->file()));
         break;
   }

   m_lastIsPara = true;
}

void RTFDocVisitor::visit(DocIncOperator *op)
{
   QString fileExt = getFileNameExtension(op->includeFileName());

   if (fileExt.isEmpty()) {
      fileExt = m_langExt;
   }

   SrcLangExt srcLangExt = getLanguageFromFileName(fileExt);

   if (op->isFirst()) {
      if (! m_hide) {
         m_t << "{" << endl;
         m_t << "\\par" << endl;
         m_t << rtf_Style_Reset << getStyle("CodeExample");
      }

      pushEnabled();
      m_hide = true;
   }

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
         m_t << "\\par";
         m_t << "}" << endl;
      }
      m_lastIsPara = true;

   } else {
      if (! m_hide) {
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

   DBG_RTF("{\\comment RTFDocVisitor::visit(DocFormula)}\n");
   bool bDisplay = ! f->isInline();

   if (bDisplay)  {
      m_t << "\\par";
      m_t << "{";
      m_t << "\\pard\\plain";
      m_t << "\\pard";
      m_t << "\\qc";
   }

   m_t << "{ \\field\\flddirty {\\*\\fldinst  INCLUDEPICTURE \"" << f->relPath() << f->name() << ".png\" \\\\d \\\\*MERGEFORMAT}{\\fldrslt Image}}";
   if (bDisplay)  {
      m_t << "\\par}";
   }

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

   if (! cite->file().isEmpty()) {
      endLink(cite->ref());
   } else {
      m_t << "}";
   }
}

void RTFDocVisitor::visitPre(DocAutoList *l)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocAutoList)}\n");

   m_t << "{\n";
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

   if (! m_lastIsPara)  {
      m_t << "\\par";
   }

   m_t << "}" << endl;

   m_lastIsPara = true;
   if (m_indentLevel == 0) {
     m_t << "\\par" << endl;
   }
}

void RTFDocVisitor::visitPre(DocAutoListItem *)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocAutoListItem)}\n");
   if (!m_lastIsPara) {
      m_t << "\\par\n";
   }

   m_t << rtf_Style_Reset;

   if (rtf_listItemInfo[m_indentLevel].isEnum) {
      m_t << getStyle("ListEnum") << "\n";
      m_t << rtf_listItemInfo[m_indentLevel].number << ".\\tab ";

      ++rtf_listItemInfo[m_indentLevel].number;

   } else {
      m_t << getStyle("ListBullet") << "\n";
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

   // omit <p> for last paragraph and for parameters & sections

   if (! m_lastIsPara && ! p->isLast() && ! (p->parent() && p->parent()->kind() == DocNode::Kind_ParamSect)) {
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

   m_t << "{" << rtf_Style["BodyText"].m_reference << "\n";
}

void RTFDocVisitor::visitPost(DocRoot *r)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocRoot)}\n");

   if (! m_lastIsPara && ! r->singleLine()) {
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

   if (! m_lastIsPara) {
      m_t << "\\par\n";
   }

   m_t << "{";

   // start desc
   m_t << "{" << rtf_Style["Heading5"].m_reference << "\n";

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

      m_t << "\\par";
      m_t << "}"; // end bold
      incIndentLevel();
      m_t << rtf_Style_Reset << getStyle("DescContinue");
      m_t << "{\\s17 \\sa60 \\sb30\n";
   }

   m_lastIsPara = false;
}

void RTFDocVisitor::visitPost(DocSimpleSect *s)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocSimpleSect)}\n");

   if (! m_lastIsPara) {
      m_t << "\\par" << endl;
   }

   decIndentLevel();

   if (s->type() != DocSimpleSect::User && s->type() != DocSimpleSect::Rcs) {
      m_t << "}";
   }

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
   m_t << "}";                // end bold

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

   m_t << "{\n";
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

   m_t << "\\par" << rtf_Style_Reset << getStyle("ListBullet") << "\n";
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
      m_t << "\\par\n" ;
   }

   m_t << "{\\bkmkstart " << rtfFormatBmkStr(s->file() + "_" + s->anchor()) << "}\n";
   m_t << "{\\bkmkend " << rtfFormatBmkStr(s->file() + "_" + s->anchor()) << "}\n";

   m_t << "{{" // start section
       << rtf_Style_Reset;

   QString heading;
   int level = qMin(s->level() + 1, 4);

   heading = QString("Heading%1").formatArg(level);

   // set style
   m_t << rtf_Style[heading].m_reference << "\n";

   // make table of contents entry
   filter(s->title());

   m_t << "\n\\par}\n";
   m_t << "{\\tc\\tcl" << level << " \\v ";

   filter(s->title());
   m_t << "}\n";
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

   m_t << "{\n";
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
   m_t << "\\par\n";
   m_t << rtf_Style_Reset;

   if (rtf_listItemInfo[m_indentLevel].isEnum) {
      m_t << getStyle("ListEnum") << "\n";
      m_t << rtf_listItemInfo[m_indentLevel].number << ".\\tab ";

      ++rtf_listItemInfo[m_indentLevel].number;

   } else {
      m_t << getStyle("ListBullet") << "\n";
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

   m_t << "{" << rtf_Style["Heading5"].m_reference << "\n";
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

void RTFDocVisitor::visitPre(DocHtmlTable *t)
{
   if (m_hide) {
      return;
   }

   if (! m_lastIsPara) {
      m_t << "\\par\n";
   }

   m_lastIsPara = true;

   if (t->hasCaption()) {
      DocHtmlCaption *c = t->caption();
      m_t << "\\pard \\qc \\b";

      if (! c->file().isEmpty()) {
         m_t << "{\\bkmkstart " << rtfFormatBmkStr(stripPath(c->file()) + "_" + c->anchor()) << "}\n";
         m_t << "{\\bkmkend "   << rtfFormatBmkStr(stripPath(c->file()) + "_" + c->anchor()) << "}\n";
      }

      m_t << "{Table \\field\\flddirty{\\*\\fldinst { SEQ Table \\\\*Arabic }}{\\fldrslt {\\noproof 1}} ";
   }
}

void RTFDocVisitor::visitPost(DocHtmlTable *)
{
   if (m_hide) {
      return;
   }

   m_t << "\\pard\\plain" << endl;
   m_t << "\\par" << endl;
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPre(DocHtmlCaption *)
{
}

void RTFDocVisitor::visitPost(DocHtmlCaption *)
{
   m_t << "}\n\\par\n";
}

void RTFDocVisitor::visitPre(DocHtmlRow *r)
{
   if (m_hide) {
      return;
   }

   uint i, columnWidth = r->numCells() > 0 ? rtf_pageWidth / r->numCells() : 10;
   m_t << "\\trowd \\trgaph108\\trleft-108"
       "\\trbrdrt\\brdrs\\brdrw10 "
       "\\trbrdrl\\brdrs\\brdrw10 "
       "\\trbrdrb\\brdrs\\brdrw10 "
       "\\trbrdrr\\brdrs\\brdrw10 "
       "\\trbrdrh\\brdrs\\brdrw10 "
       "\\trbrdrv\\brdrs\\brdrw10 " << "\n";

   for (i = 0; i < r->numCells(); i++) {
      if (r->isHeading()) {
         m_t << "\\clcbpat16"; // set cell shading to light gray (color 16 in the clut)
      }

      m_t << "\\clvertalt\\clbrdrt\\brdrs\\brdrw10 "
             "\\clbrdrl\\brdrs\\brdrw10 "
             "\\clbrdrb\\brdrs\\brdrw10 "
             "\\clbrdrr \\brdrs\\brdrw10 "
             "\\cltxlrtb "
             "\\cellx" << ((i + 1)*columnWidth) << "\n";
   }

   m_t << "\\pard \\widctlpar\\intbl\\adjustright" << "\n";
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
      m_t <<  "}"
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
   int level = qMin(header->level(), 5);
   heading = QString("Heading%1").formatArg(level);

   // set style
   m_t << rtf_Style[heading].m_reference;

   // make open table of contents entry that will be closed in visitPost method
   m_t << "{\\tc\\tcl" << level << " ";

   m_lastIsPara = false;

}

void RTFDocVisitor::visitPost(DocHtmlHeader *)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocHtmlHeader)}\n");

   // close open table of contents entry
   m_t << "} \\par";

   // end section
   m_t << "}" << endl;

   m_lastIsPara = true;
}

void RTFDocVisitor::visitPre(DocImage *img)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocImage)}\n");

   includePicturePreRTF(img->name(), img->type() == DocImage::Rtf, img->hasCaption(), img->isInlineImage());
}

void RTFDocVisitor::includePicturePreRTF(const QString &name, const bool isTypeRTF, const bool hasCaption, bool inlineImage)
{
   if (isTypeRTF) {

      if (! inlineImage) {

         m_t << "\\par" << endl;
         m_t << "{" << endl;
         m_t << rtf_Style_Reset << endl;

         if (hasCaption || m_lastIsPara) {
            m_t << "\\par" << endl;

         }

         m_t << "\\pard \\qc ";
      }

      m_t << "{ \\field\\flddirty {\\*\\fldinst  INCLUDEPICTURE \"";
      m_t << name;
      m_t << "\" \\\\d \\\\*MERGEFORMAT}{\\fldrslt Image}}" << endl;
      if (! inlineImage) {
         m_t << "\\par" << endl;

         if (hasCaption) {
            m_t << "\\pard \\qc \\b";
            m_t << "{Image \\field\\flddirty{\\*\\fldinst { SEQ Image \\\\*Arabic }}{\\fldrslt {\\noproof 1}} ";
         }

         m_lastIsPara = true;

      } else {

         if (hasCaption) {
            // to prevent caption to be shown
            m_t << "{\\comment ";
         }
      }

   } else {
      // other format -> skip

      pushEnabled();
      m_hide = true;
   }
}

void RTFDocVisitor::visitPost(DocImage *img)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocImage)}\n");
   includePicturePostRTF(img->type() == DocImage::Rtf, img->hasCaption(), img->isInlineImage());
}

void RTFDocVisitor::includePicturePostRTF(const bool isTypeRTF, const bool hasCaption, bool inlineImage)
{
   if (isTypeRTF) {
      if (m_hide) {
         return;
      }

      if (inlineImage) {
         if (hasCaption) {
            m_t << " }";
         }

      } else {

         if (hasCaption) {

            m_t << "}" <<endl;
            m_t << "\\par}" <<endl;

         } else {
            m_t << "}" <<endl;
         }
      }

   } else {
      popEnabled();

   }
}

void RTFDocVisitor::visitPre(DocDotFile *df)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocDotFile)}\n");
   writeDotFile(df);
}

void RTFDocVisitor::visitPost(DocDotFile *df)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocDotFile)}\n");
   includePicturePostRTF(true, df->hasCaption());
}

void RTFDocVisitor::visitPre(DocMscFile *df)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocMscFile)}\n");
   writeMscFile(df);
}

void RTFDocVisitor::visitPost(DocMscFile *df)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocMscFile)}\n");
   includePicturePostRTF(true, df->hasCaption());
}

void RTFDocVisitor::visitPre(DocDiaFile *df)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocDiaFile)}\n");
   writeDiaFile(df);
}

void RTFDocVisitor::visitPost(DocDiaFile *df)
{
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocDiaFile)}\n");
   includePicturePostRTF(true, df->hasCaption());
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
      startLink(ref->ref(), QString(), ref->anchor());
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

   m_t << "{\n";
   incIndentLevel();

   m_t << rtf_Style_Reset << getStyle("LatexTOC") << "\n";
   m_t << "\\par\n";
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

   // start param list
   m_t << "{";

   if (! m_lastIsPara) {
      m_t << "\\par\n";
   }

   // start bold
   // m_t << "{\\b ";

   m_t << "{" << rtf_Style["Heading5"].m_reference << "\n";

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
         m_t << theTranslator->trTemplateParameters();
         break;

      default:
         assert(0);
   }

   m_t << "\\par";
   m_t << "}\n";

   bool useTable = s->type() == DocParamSect::Param ||
                   s->type() == DocParamSect::RetVal ||
                   s->type() == DocParamSect::Exception ||
                   s->type() == DocParamSect::TemplateParam;

   if (! useTable) {
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
   // m_t << "\\par" << endl;

   bool useTable = s->type() == DocParamSect::Param ||
                   s->type() == DocParamSect::RetVal ||
                   s->type() == DocParamSect::Exception ||
                   s->type() == DocParamSect::TemplateParam;

   if (! useTable) {
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
   DocParamSect *sect = nullptr;

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
             "\\trbrdrv\\brdrs\\brdrw10\\brdrcf15 " << "\n";

      for (i = 0; i < columnPos[config][0]; i++) {
         m_t << "\\clvertalt\\clbrdrt\\brdrs\\brdrw10\\brdrcf15 "
                "\\clbrdrl\\brdrs\\brdrw10\\brdrcf15 "
                "\\clbrdrb\\brdrs\\brdrw10\\brdrcf15 "
                "\\clbrdrr \\brdrs\\brdrw10\\brdrcf15 "
                "\\cltxlrtb "
                "\\cellx" << (rtf_pageWidth * columnPos[config][i + 1] / 100) << "\n";
      }

      m_t << "\\pard \\widctlpar\\intbl\\adjustright\n";
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

      for (auto type : pl->paramTypes() ) {
         if (type->kind() == DocNode::Kind_Word) {
            visit((DocWord *)type);
         } else if (type->kind() == DocNode::Kind_LinkedWord) {
            visit((DocLinkedWord *)type);

         } else if (type->kind() == DocNode::Kind_Sep) {
            m_t << " " << ((DocSeparator *)type)->chars() << " ";
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

   if (pl->parent() && pl->parent()->kind() == DocNode::Kind_ParamSect) {
      parentType = ((DocParamSect *)pl->parent())->type();
   }

   bool useTable = parentType == DocParamSect::Param ||
                   parentType == DocParamSect::RetVal ||
                   parentType == DocParamSect::Exception ||
                   parentType == DocParamSect::TemplateParam;

   if (useTable) {
      m_t << "\\cell }" << endl;
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

   if ( ! m_lastIsPara)   {
      m_t << "\\par\n";
      m_lastIsPara = true;
   }

   m_t << "{";

   // start param list
   // m_t << "{\\b "; // start bold

   m_t << "{" << rtf_Style["Heading5"].m_reference << "\n";

   if (Config::getBool("rtf-hyperlinks") && ! anonymousEnum) {
      QString refName;

      if (!x->file().isEmpty()) {
         refName += x->file();
      }

      if (! x->file().isEmpty() && ! x->anchor().isEmpty()) {
         refName += "_";
      }
      if (! x->anchor().isEmpty()) {
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
   m_t << "}" << endl;       // end xref item
   m_lastIsPara = true;
}

void RTFDocVisitor::visitPre(DocInternalRef *ref)
{
   if (m_hide) {
      return;
   }

   DBG_RTF("{\\comment RTFDocVisitor::visitPre(DocInternalRef)}\n");
   startLink(QString(), ref->file(), ref->anchor());
}

void RTFDocVisitor::visitPost(DocInternalRef *)
{
   if (m_hide) {
      return;
   }
   DBG_RTF("{\\comment RTFDocVisitor::visitPost(DocInternalRef)}\n");
   endLink(QString());
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
      m_t << "\\par\n" ;
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
   static const bool rtfHyperLinks = Config::getBool("rtf-hyperlinks");

   if (ref.isEmpty() && rtfHyperLinks) {
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

void RTFDocVisitor::writeDotFile(DocDotFile *df)
{
   writeDotFile(df->file(), df->hasCaption());
}

void RTFDocVisitor::writeDotFile(const QString &fileName, const bool hasCaption)
{
   QString baseName = fileName;

   int i;
   if ((i = baseName.lastIndexOf('/')) != -1) {
      baseName = baseName.right(baseName.length() - i - 1);
   }

   QString outDir = Config::getString("rtf-output");
   writeDotGraphFromFile(fileName, outDir, baseName, GOF_BITMAP);

   static const QString imageExt = Config::getEnum("dot-image-extension");
   includePicturePreRTF(baseName + "." + imageExt, true, hasCaption);
}

void RTFDocVisitor::writeMscFile(DocMscFile *df)
{
   writeMscFile(df->file(), df->hasCaption());
}

void RTFDocVisitor::writeMscFile(const QString &fileName, const bool hasCaption)
{
   QString baseName = fileName;

   int i;
   if ((i = baseName.lastIndexOf('/')) != -1) {
      baseName = baseName.right(baseName.length() - i - 1);
   }

   QString outDir = Config::getString("rtf-output");
   writeMscGraphFromFile(fileName, outDir, baseName, MSC_BITMAP);

   includePicturePreRTF(baseName + ".png", true, hasCaption);
}

void RTFDocVisitor::writeDiaFile(DocDiaFile *df)
{
   QString baseName = df->file();

   int i;
   if ((i = baseName.lastIndexOf('/')) != -1) {
      baseName = baseName.right(baseName.length() - i - 1);
   }

   QString outDir = Config::getString("rtf-output");
   writeDiaGraphFromFile(df->file(), outDir, baseName, DIA_BITMAP);
   includePicturePreRTF(baseName + ".png", true, df->hasCaption());
}

void RTFDocVisitor::writePlantUMLFile(const QString &fileName, const bool hasCaption)
{
   QString baseName = fileName;

   int i;
   if ((i = baseName.lastIndexOf('/')) != -1) {
      baseName = baseName.right(baseName.length() - i - 1);
   }

   QString outDir = Config::getString("rtf-output");
   generatePlantUMLOutput(fileName, outDir, PUML_BITMAP);
   includePicturePreRTF(baseName + ".png", true, hasCaption);
}

