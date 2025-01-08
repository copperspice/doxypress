/************************************************************************
*
* Copyright (c) 2014-2025 Barbara Geller & Ansel Sermersheim
*
* Copyright (c) 1997-2014 Dimitri van Heesch
*
* DoxyPress is free software. You can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
* https://www.gnu.org/licenses/
*
*************************************************************************/

#include <latexdocvisitor.h>

#include <cite.h>
#include <config.h>
#include <dia.h>
#include <docparser.h>
#include <dot.h>
#include <doxy_globals.h>
#include <emoji_entity.h>
#include <htmlattrib.h>
#include <htmlentity.h>
#include <language.h>
#include <latexgen.h>
#include <message.h>
#include <msc.h>
#include <outputgen.h>
#include <parse_base.h>
#include <plantuml.h>
#include <util.h>

#include <QFileInfo>

constexpr const int maxLevels = 5;

static const QString secLabels[maxLevels] = {
      "doxysection", "doxysubsection", "doxysubsubsection", "doxyparagraph", "doxysubparagraph" };

static QString getSectionName(int level)
{
   static bool compactLatex = Config::getBool("latex-compact");

   if (compactLatex) {
      ++level;
   }

   if (Doxy_Globals::insideMainPage) {
      --level;
   }

   return secLabels[qMin(maxLevels - 1, level)];
}

static void visitPreStart(QTextStream &t, const bool hasCaption, const QString &name,
         const QString &width, const QString &height, bool inlineImage = false)
{
   if (inlineImage) {
      t << "\n\\begin{DoxyInlineImage}\n";
    } else {
       if (hasCaption) {
         t << "\n\\begin{DoxyImage}\n";

       } else {
         t << "\n\\begin{DoxyImageNoCaption}\n"
                "  \\mbox{";
       }
    }

    t << "\\includegraphics";

    if (! width.isEmpty() || ! height.isEmpty()) {
      t << "[";
    }

    if (! width.isEmpty()) {
      t << "width=" << width;
    }

    if (! width.isEmpty() && !height.isEmpty()) {
      t << ",";
    }

    if (! height.isEmpty()) {
      t << "height=" << height;
    }

    if (width.isEmpty() && height.isEmpty()) {
      /* default setting */
      if (inlineImage) {
         t << "[height=\\baselineskip,keepaspectratio=true]";

      } else {
         t << "[width=\\textwidth,height=\\textheight/2,keepaspectratio=true]";

      }

    } else {
      t << "]";
    }

    t << "{" << name << "}";

    if (hasCaption) {
      if (! inlineImage) {
          t << "\n\\doxyfigcaption{";

       } else {
         // to catch the caption
         t << "%";
      }
    }
}

static void visitPostEnd(QTextStream &t, const bool hasCaption, bool inlineImage = false)
{
   if (inlineImage) {
      t << "\n\\end{DoxyInlineImage}\n";

   } else {
      // end mbox or caption
      t << "}\n";

      if (hasCaption) {
         t << "\\end{DoxyImage}\n";

      } else {
         t << "\\end{DoxyImageNoCaption}\n";
      }
   }
}

static void visitCaption(LatexDocVisitor *parent, QList<DocNode *> children)
{
   for (auto n : children) {
      n->accept(parent);
   }
}

QString LatexDocVisitor::escapeMakeIndexChars(const QString &str)
{
   QString result;

   for (auto c : str ) {

      switch (c.unicode()) {
         case '!':
            m_t << "\"!";
            break;

         case '"':
            m_t << "\"\"";
            break;

         case '@':
            m_t << "\"@";
            break;

         case '|':
            m_t << "\\texttt{\"|}";
            break;

         case '[':
            m_t << "[";
            break;

         case ']':
            m_t << "]";
            break;

         case '{':
            m_t << "\\lcurly{}";
            break;

         case '}':
            m_t << "\\rcurly{}";
            break;

         default:
            filter(c);
            break;
      }
   }

   return result;
}

LatexDocVisitor::LatexDocVisitor(QTextStream &t, LatexCodeGenerator &ci, const QString &langExt, bool insideTabbing)
   : DocVisitor(DocVisitor_Latex), m_t(t), m_ci(ci), m_insidePre(false), m_insideItem(false),
     m_hide(false), m_hideCaption(false), m_insideTabbing(insideTabbing), m_langExt(langExt)
{
}

void LatexDocVisitor::visit(DocWord *w)
{
   if (m_hide) {
      return;
   }

   filter(w->word());
}

void LatexDocVisitor::visit(DocLinkedWord *w)
{
   if (m_hide) {
      return;
   }

   startLink(w->ref(), w->file(), w->anchor());
   filter(w->word());
   endLink(w->ref(), w->file(), w->anchor());
}

void LatexDocVisitor::visit(DocWhiteSpace *w)
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

void LatexDocVisitor::visit(DocSymbol *s)
{
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   if (m_hide) {
      return;
   }

   QString res = HtmlEntityMapper::instance()->latex(s->symbol());

   if (! res.isEmpty()) {

      if (((s->symbol() == DocSymbol::Sym_lt) || (s->symbol() == DocSymbol::Sym_Less)) && (! m_insidePre)) {
         if (pdfHyperlinks) {
            m_t << "\\texorpdfstring{$<$}{<}";

         } else {
            m_t << "$<$";
         }

      } else if (((s->symbol() == DocSymbol::Sym_gt) || (s->symbol() == DocSymbol::Sym_Greater)) && (! m_insidePre)) {
         if (pdfHyperlinks) {
            m_t << "\\texorpdfstring{$>$}{>}";

         } else {
            m_t << "$>$";
         }

      } else {
         m_t << res;
      }

   } else {
      err("LaTeX unsupported HTML entity found: %s\n", csPrintable(HtmlEntityMapper::instance()->html(s->symbol(), true)) );
   }
}

void LatexDocVisitor::visit(DocEmoji *s)
{
   if (m_hide) {
      return;
   }

   QString result = EmojiEntityMapper::instance()->name(s->index());

   if (! result.isEmpty()) {
      // strip :
      QString imageName = result.mid(1, result.length() - 2);

      m_t << "\\doxyemoji{";
      filter(result);
      m_t << "}{" << imageName << "}";

   } else {
      m_t << s->name();
   }
}

void LatexDocVisitor::visit(DocURL *u)
{
   if (m_hide) {
      return;
   }

   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   if (pdfHyperlinks) {
      m_t << "\\href{";

      if (u->isEmail()) {
         m_t << "mailto:";
      }

      m_t << latexFilterURL(u->url()) << "}";
   }

   m_t << "{\\texttt{ ";
   filter(u->url());
   m_t << "}}";
}

void LatexDocVisitor::visit(DocLineBreak *)
{
   if (m_hide) {
      return;
   }
   m_t << "~\\newline\n";
}

void LatexDocVisitor::visit(DocHorRuler *)
{
   if (m_hide) {
      return;
   }

   if (insideTable()) {
      m_t << "\\DoxyHorRuler{1}\n";
   } else {
      m_t << "\\DoxyHorRuler{0}\n";
   }
}

void LatexDocVisitor::visit(DocStyleChange *s)
{
   if (m_hide) {
      return;
   }

   switch (s->style()) {
      case DocStyleChange::Bold:
         if (s->enable()) {
            m_t << "{\\bfseries{";
         } else {
            m_t << "}}";
         }
         break;

      case DocStyleChange::S:
      case DocStyleChange::Strike:
      case DocStyleChange::Del:
         if (s->enable()) {
            m_t << "\\sout{";
         } else {
            m_t << "}";
         }
         break;

      case DocStyleChange::Underline:
      case DocStyleChange::Ins:
         if (s->enable()) {
            m_t << "\\uline{";
         } else {
            m_t << "}";
         }
         break;

      case DocStyleChange::Italic:
         if (s->enable()) {
            m_t << "{\\itshape ";
         } else {
            m_t << "}";
         }
         break;

      case DocStyleChange::Code:
         if (s->enable()) {
            m_t << "{\\ttfamily ";
         }   else {
            m_t << "}";
         }
         break;
      case DocStyleChange::Subscript:
         if (s->enable()) {
            m_t << "\\textsubscript{";
         }    else {
            m_t << "}";
         }
         break;

      case DocStyleChange::Superscript:
         if (s->enable()) {
            m_t << "\\textsuperscript{";
         }    else {
            m_t << "}";
         }
         break;
      case DocStyleChange::Center:
         if (s->enable()) {
            m_t << "\\begin{center}";
         } else {
            m_t << "\\end{center} ";
         }
         break;

      case DocStyleChange::Small:
         if (s->enable()) {
            m_t << "\n\\footnotesize ";
         }  else {
            m_t << "\n\\normalsize ";
         }
         break;

      case DocStyleChange::Cite:
         if (s->enable()) {
            m_t << "{\\itshape ";
         } else {
            m_t << "}";
         }
         break;

      case DocStyleChange::Preformatted:
         if (s->enable()) {
            m_t << "\n\\begin{DoxyPre}";
            m_insidePre = true;
         } else {
            m_insidePre = false;
            m_t << "\\end{DoxyPre}\n";
         }
         break;

      case DocStyleChange::Div:
      case DocStyleChange::Span:
         // HTML only
         break;

      case DocStyleChange::Details:
         // emulation of the <details> tag

         if (! s->enable()) {
            m_t << "\n\n";
         }
         break;

      case DocStyleChange::Summary:
         // emulation of the <summary> tag inside a <details> tag

         if (s->enable()) {
            m_t << "{\\bfseries{";
         } else {
            m_t << "}}\\newline";
         }
         break;
   }
}

void LatexDocVisitor::visit(DocVerbatim *s)
{
   static const QString latexOutput = Config::getString("latex-output");

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
      case DocVerbatim::Code: {
         m_ci.startCodeFragment("DoxyCode");

         Doxy_Globals::parserManager.getParser(lang)->parseCode(m_ci, s->context(), s->text(),
                  langExt, s->isExample(), s->exampleFile());

         m_ci.endCodeFragment("DoxyCode");
      }
      break;

      case DocVerbatim::Verbatim:
         m_t << "\\begin{DoxyVerb}";
         m_t << s->text();
         m_t << "\\end{DoxyVerb}\n";
         break;

      case DocVerbatim::HtmlOnly:
      case DocVerbatim::XmlOnly:
      case DocVerbatim::ManOnly:
      case DocVerbatim::RtfOnly:
      case DocVerbatim::DocbookOnly:
         /* nothing */
         break;

      case DocVerbatim::LatexOnly:
         m_t << s->text();
         break;

      case DocVerbatim::Dot: {
         static int dotindex = 1;

         QString fileName = QString("%1%2.dot").formatArg(latexOutput + "/inline_dotgraph_").formatArg(dotindex);
         ++dotindex;

         QFile file(fileName);

         if (! file.open(QIODevice::WriteOnly)) {
            err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), file.error());

         } else {

            file.write( s->text().toUtf8() );
            file.close();

            startDotFile(fileName, s->width(), s->height(), s->hasCaption());
            visitCaption(this, s->children());
            endDotFile(s->hasCaption());

            if (Config::getBool("dot-cleanup")) {
               file.remove();
            }
         }
      }
      break;

      case DocVerbatim::Msc: {
         static int mscindex = 1;

         QString baseName = QString("%1%2").formatArg(latexOutput + "/inline_mscgraph_").formatArg(mscindex);
         ++mscindex;

         QFile file(baseName + ".msc");

         if (! file.open(QIODevice::WriteOnly)) {
            err("Unable to open file %s.msc for writing, OS Error #: %d\n", csPrintable(baseName), file.error());

         } else {

            QString text = "msc {";
            text += s->text();
            text += "}";

            file.write( text.toUtf8() );
            file.close();

            writeMscFile(baseName, s);

            if (Config::getBool("dot-cleanup")) {
               file.remove();
            }
         }
      }
      break;

      case DocVerbatim::PlantUML: {
         QString baseName = writePlantUMLSource(latexOutput, s->exampleFile(), s->text());

         writePlantUMLFile(baseName, s);
      }
      break;
   }
}

void LatexDocVisitor::visit(DocAnchor *anc)
{
   if (m_hide) {
      return;
   }

   static bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   m_t << "\\label{" << stripPath(anc->file()) << "_" << anc->anchor() << "}%" << endl;

   if (! anc->file().isEmpty() && pdfHyperlinks) {
      m_t << "\\Hypertarget{" << stripPath(anc->file()) << "_" << anc->anchor()
          << "}%" << endl;
   }
}

void LatexDocVisitor::visit(DocInclude *inc)
{
   if (m_hide) {
      return;
   }

   SrcLangExt langExt = getLanguageFromFileName(inc->extension());

   switch (inc->type()) {
      case DocInclude::IncWithLines: {
         m_ci.startCodeFragment("DoxyCodeInclude");

         QFileInfo cfi(inc->file());
         QSharedPointer<FileDef> fd = QMakeShared<FileDef>(cfi.path(), cfi.fileName());

         Doxy_Globals::parserManager.getParser(inc->extension())->parseCode(m_ci, inc->context(),
                     inc->text(), langExt, inc->isExample(), inc->exampleFile(), fd,
                     -1, -1, false, QSharedPointer<MemberDef>(), true);


         m_ci.endCodeFragment("DoxyCodeInclude");
      }
      break;

      case DocInclude::Include:
         m_ci.startCodeFragment("DoxyCodeInclude");

         Doxy_Globals::parserManager.getParser(inc->extension())->parseCode(m_ci, inc->context(),
                     inc->text(), langExt, inc->isExample(), inc->exampleFile(), QSharedPointer<FileDef>(),
                     -1, -1, true, QSharedPointer<MemberDef>(), false);

         m_ci.endCodeFragment("DoxyCodeInclude");
         break;

      case DocInclude::DontInclude:
      case DocInclude::DontIncWithLines:
      case DocInclude::HtmlInclude:
      case DocInclude::RtfInclude:
      case DocInclude::ManInclude:
      case DocInclude::XmlInclude:
      case DocInclude::DocbookInclude:
         break;

      case DocInclude::LatexInclude:
         m_t << inc->text();
         break;

      case DocInclude::VerbInclude:
         m_t << "\n\\begin{DoxyVerbInclude}\n";
         m_t << inc->text();
         m_t << "\\end{DoxyVerbInclude}\n";
         break;

      case DocInclude::Snippet:
         m_ci.startCodeFragment("DoxyCodeInclude");

         Doxy_Globals::parserManager.getParser(inc->extension())->parseCode(m_ci, inc->context(),
                  extractBlock(inc->text(), inc->blockId()), langExt,
                  inc->isExample(), inc->exampleFile());

         m_ci.endCodeFragment("DoxyCodeInclude");
         break;

      case DocInclude::SnipWithLines: {
        QFileInfo cfi( inc->file() );
        QSharedPointer<FileDef> fd = QMakeShared<FileDef>(cfi.path(), cfi.fileName());

        m_ci.startCodeFragment("DoxyCodeInclude");

        Doxy_Globals::parserManager.getParser(inc->extension())->parseCode(m_ci, inc->context(),
                  extractBlock(inc->text(),inc->blockId()), langExt,
                  inc->isExample(), inc->exampleFile(), fd,
                  lineBlock(inc->text(),inc->blockId()),
                  -1, false, QSharedPointer<MemberDef>(), true);

        m_ci.endCodeFragment("DoxyCodeInclude");
      }
      break;

      case DocInclude::IncludeDoc:
      case DocInclude::SnippetDoc:
         err("Unexpected command found for IncludeDoc or SnippetDoc in file: %s,"
               " contact the developers\n", csPrintable(inc->file()));
         break;
   }
}

void LatexDocVisitor::visit(DocIncOperator *op)
{
   if (op->isFirst()) {
      if (! m_hide) {
         m_ci.startCodeFragment("DoxyCodeInclude");
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
         Doxy_Globals::parserManager.getParser(fileExt)->parseCode(m_ci, op->context(), op->text(),
                  srcLangExt, op->isExample(), op->exampleFile());
      }

      pushEnabled();
      m_hide = true;
   }

   if (op->isLast()) {
      popEnabled();

      if (! m_hide) {
         m_ci.endCodeFragment("DoxyCodeInclude");
      }

   } else {
      if (! m_hide) {
         m_t << endl;
      }
   }
}

void LatexDocVisitor::visit(DocFormula *f)
{
   if (m_hide) {
      return;
   }

   for (QChar ch : f->text()) {

      if (ch == '\'') {
         m_t << "\\textnormal{\\textquotesingle}";

      } else {
         m_t << ch;

      }
   }
}

void LatexDocVisitor::visit(DocIndexEntry *i)
{
   if (m_hide) {
      return;
   }

   m_t << "\\index{";
   m_t << latexEscapeLabelName(i->entry());
   m_t << "@{";
   m_t << latexEscapeIndexChars(i->entry());
   m_t << "}}";
}

void LatexDocVisitor::visit(DocSimpleSectSep *)
{
}

void LatexDocVisitor::visit(DocCite *cite)
{
   if (m_hide) {
      return;
   }
   if (!cite->file().isEmpty()) {
      //startLink(cite->ref(),cite->file(),cite->anchor());
      QString anchor = cite->anchor();
      anchor = anchor.mid(CiteConsts::anchorPrefix.length()); // strip prefix
      m_t << "\\cite{" << anchor << "}";
   } else {
      m_t << "{\\bfseries [";
      filter(cite->text());
      m_t << "]}";
   }
}

//--------------------------------------
// visitor functions for compound nodes
//--------------------------------------

void LatexDocVisitor::visitPre(DocAutoList *l)
{
   if (m_hide) {
      return;
   }

   if (l->isEnumList()) {
      m_t << "\n\\begin{DoxyEnumerate}";
   } else {
      m_t << "\n\\begin{DoxyItemize}";
   }
}

void LatexDocVisitor::visitPost(DocAutoList *l)
{
   if (m_hide) {
      return;
   }

   if (l->isEnumList()) {
      m_t << "\n\\end{DoxyEnumerate}";
   } else {
      m_t << "\n\\end{DoxyItemize}";
   }
}

void LatexDocVisitor::visitPre(DocAutoListItem *)
{
   if (m_hide) {
      return;
   }
   m_t << "\n\\item ";
}

void LatexDocVisitor::visitPost(DocAutoListItem *)
{
}

void LatexDocVisitor::visitPre(DocPara *)
{
}

void LatexDocVisitor::visitPost(DocPara *p)
{
   if (m_hide) {
      return;
   }

   if (! p->isLast() &&            // omit <p> for last paragraph
         !(p->parent() &&           // and for parameter sections
           p->parent()->kind() == DocNode::Kind_ParamSect
          )
      ) {
      m_t << endl << endl;
   }
}

void LatexDocVisitor::visitPre(DocRoot *)
{
}

void LatexDocVisitor::visitPost(DocRoot *)
{
}

void LatexDocVisitor::visitPre(DocSimpleSect *s)
{
   if (m_hide) {
      return;
   }

   switch (s->type()) {
      case DocSimpleSect::See:
         m_t << "\\begin{DoxySeeAlso}{";
         filter(theTranslator->trSeeAlso());
         break;

      case DocSimpleSect::Return:
         m_t << "\\begin{DoxyReturn}{";
         filter(theTranslator->trReturns());
         break;

      case DocSimpleSect::Author:
         m_t << "\\begin{DoxyAuthor}{";
         filter(theTranslator->trAuthor(true, true));
         break;

      case DocSimpleSect::Authors:
         m_t << "\\begin{DoxyAuthor}{";
         filter(theTranslator->trAuthor(true, false));
         break;

      case DocSimpleSect::Version:
         m_t << "\\begin{DoxyVersion}{";
         filter(theTranslator->trVersion());
         break;

      case DocSimpleSect::Since:
         m_t << "\\begin{DoxySince}{";
         filter(theTranslator->trSince());
         break;

      case DocSimpleSect::Date:
         m_t << "\\begin{DoxyDate}{";
         filter(theTranslator->trDate());
         break;

      case DocSimpleSect::Note:
         m_t << "\\begin{DoxyNote}{";
         filter(theTranslator->trNote());
         break;

      case DocSimpleSect::Warning:
         m_t << "\\begin{DoxyWarning}{";
         filter(theTranslator->trWarning());
         break;

      case DocSimpleSect::Pre:
         m_t << "\\begin{DoxyPrecond}{";
         filter(theTranslator->trPrecondition());
         break;

      case DocSimpleSect::Post:
         m_t << "\\begin{DoxyPostcond}{";
         filter(theTranslator->trPostcondition());
         break;

      case DocSimpleSect::Copyright:
         m_t << "\\begin{DoxyCopyright}{";
         filter(theTranslator->trCopyright());
         break;

      case DocSimpleSect::Invar:
         m_t << "\\begin{DoxyInvariant}{";
         filter(theTranslator->trInvariant());
         break;

      case DocSimpleSect::Remark:
         m_t << "\\begin{DoxyRemark}{";
         filter(theTranslator->trRemarks());
         break;

      case DocSimpleSect::Attention:
         m_t << "\\begin{DoxyAttention}{";
         filter(theTranslator->trAttention());
         break;

      case DocSimpleSect::User:
         m_t << "\\begin{DoxyParagraph}{";
         break;

      case DocSimpleSect::Rcs:
         m_t << "\\begin{DoxyParagraph}{";
         break;

      case DocSimpleSect::Unknown:
         break;
   }

   // special case 1: user defined title
   if (s->type() != DocSimpleSect::User && s->type() != DocSimpleSect::Rcs) {
      m_t << "}\n";
   } else {
      m_insideItem = true;
   }
}

void LatexDocVisitor::visitPost(DocSimpleSect *s)
{
   if (m_hide) {
      return;
   }
   switch (s->type()) {
      case DocSimpleSect::See:
         m_t << "\n\\end{DoxySeeAlso}\n";
         break;
      case DocSimpleSect::Return:
         m_t << "\n\\end{DoxyReturn}\n";
         break;
      case DocSimpleSect::Author:
         m_t << "\n\\end{DoxyAuthor}\n";
         break;
      case DocSimpleSect::Authors:
         m_t << "\n\\end{DoxyAuthor}\n";
         break;
      case DocSimpleSect::Version:
         m_t << "\n\\end{DoxyVersion}\n";
         break;
      case DocSimpleSect::Since:
         m_t << "\n\\end{DoxySince}\n";
         break;
      case DocSimpleSect::Date:
         m_t << "\n\\end{DoxyDate}\n";
         break;
      case DocSimpleSect::Note:
         m_t << "\n\\end{DoxyNote}\n";
         break;
      case DocSimpleSect::Warning:
         m_t << "\n\\end{DoxyWarning}\n";
         break;
      case DocSimpleSect::Pre:
         m_t << "\n\\end{DoxyPrecond}\n";
         break;
      case DocSimpleSect::Post:
         m_t << "\n\\end{DoxyPostcond}\n";
         break;
      case DocSimpleSect::Copyright:
         m_t << "\n\\end{DoxyCopyright}\n";
         break;
      case DocSimpleSect::Invar:
         m_t << "\n\\end{DoxyInvariant}\n";
         break;
      case DocSimpleSect::Remark:
         m_t << "\n\\end{DoxyRemark}\n";
         break;
      case DocSimpleSect::Attention:
         m_t << "\n\\end{DoxyAttention}\n";
         break;
      case DocSimpleSect::User:
         m_t << "\n\\end{DoxyParagraph}\n";
         break;
      case DocSimpleSect::Rcs:
         m_t << "\n\\end{DoxyParagraph}\n";
         break;
      default:
         break;
   }
}

void LatexDocVisitor::visitPre(DocTitle *)
{
}

void LatexDocVisitor::visitPost(DocTitle *)
{
   if (m_hide) {
      return;
   }

   m_insideItem = false;
   m_t << "}\n";
}

void LatexDocVisitor::visitPre(DocSimpleList *)
{
   if (m_hide) {
      return;
   }

   m_t << "\\begin{DoxyItemize}" << endl;
}

void LatexDocVisitor::visitPost(DocSimpleList *)
{
   if (m_hide) {
      return;
   }
   m_t << "\\end{DoxyItemize}" << endl;
}

void LatexDocVisitor::visitPre(DocSimpleListItem *)
{
   if (m_hide) {
      return;
   }
   m_t << "\\item ";
}

void LatexDocVisitor::visitPost(DocSimpleListItem *)
{
}

void LatexDocVisitor::visitPre(DocSection *s)
{
   if (m_hide) {
      return;
   }

   static bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   if (pdfHyperlinks) {
      m_t << "\\hypertarget{" << stripPath(s->file()) << "_" << s->anchor() << "}{}";
   }

   m_t << "\\" << getSectionName(s->level()) << "{";
   filter(convertCharEntities(s->title()));
   m_t << "}\\label{" << stripPath(s->file()) << "_" << s->anchor() << "}" << endl;
}

void LatexDocVisitor::visitPost(DocSection *)
{
}

void LatexDocVisitor::visitPre(DocHtmlList *s)
{
   if (m_hide) {
      return;
   }

   if (s->type() == DocHtmlList::Ordered) {
      m_t << "\n\\begin{DoxyEnumerate}";
   } else {
      m_t << "\n\\begin{DoxyItemize}";
   }
}

void LatexDocVisitor::visitPost(DocHtmlList *s)
{
   if (m_hide) {
      return;
   }
   if (s->type() == DocHtmlList::Ordered) {
      m_t << "\n\\end{DoxyEnumerate}";
   } else {
      m_t << "\n\\end{DoxyItemize}";
   }
}

void LatexDocVisitor::visitPre(DocHtmlListItem *)
{
   if (m_hide) {
      return;
   }
   m_t << "\n\\item ";
}

void LatexDocVisitor::visitPost(DocHtmlListItem *)
{
}

//void LatexDocVisitor::visitPre(DocHtmlPre *)
//{
//  m_t << "\\small\\begin{alltt}";
//  m_insidePre=true;
//}

//void LatexDocVisitor::visitPost(DocHtmlPre *)
//{
//  m_insidePre=false;
//  m_t << "\\end{alltt}\\normalsize " << endl;
//}
static bool classMatchesReflist(const DocNode *n)
{
   const DocHtmlDescList *obj = dynamic_cast<const DocHtmlDescList *>(n);

   if (obj != nullptr) {
      HtmlAttribList list = obj->attribs();

      for (const auto &item : list) {

         if (item.name == "class") {

            if (item.value == "reflist") {
               return true;
            }

            break;
         }
      }
   }

   return false;
}

static bool listIsNested(const DocNode *n)
{
   bool isNested = false;

   if (n) {
      if (classMatchesReflist(n)) {
         return false;
      }

      n  = n->parent();
   }

   while (n && ! isNested) {
      if (n->kind() == DocNode::Kind_HtmlDescList) {
         isNested = ! classMatchesReflist(n);
      }

      n = n->parent();
   }

   return isNested;
}

void LatexDocVisitor::visitPre(DocHtmlDescList *dl)
{
   if (m_hide) {
      return;
   }

   if (classMatchesReflist(dl)) {
      m_t << "\n\\begin{DoxyRefList}";

   } else {
      if (listIsNested(dl)) {
         m_t << "\n\\hfill";
      }

      m_t << "\n\\begin{DoxyDescription}";
   }
}

void LatexDocVisitor::visitPost(DocHtmlDescList *dl)
{
   if (m_hide) {
      return;
   }

   if (classMatchesReflist(dl)) {
      m_t << "\n\\end{DoxyRefList}";
   } else {
      m_t << "\n\\end{DoxyDescription}";
   }
}

void LatexDocVisitor::visitPre(DocHtmlDescTitle *)
{
   if (m_hide) {
      return;
   }

   m_t << "\n\\item[";
   m_insideItem = true;
}

void LatexDocVisitor::visitPost(DocHtmlDescTitle *)
{
   if (m_hide) {
      return;
   }
   m_insideItem = false;
   m_t << "]";
}

void LatexDocVisitor::visitPre(DocHtmlDescData *)
{
}

void LatexDocVisitor::visitPost(DocHtmlDescData *)
{
}

static bool tableIsNested(const DocNode *n)
{
   bool isNested = false;

   while (n && ! isNested) {
      isNested = n->kind() == DocNode::Kind_HtmlTable || n->kind() == DocNode::Kind_ParamSect;
      n  = n->parent();
   }

   return isNested;
}

static void writeStartTableCommand(QTextStream &t, const DocNode *n, int cols)
{
   if (tableIsNested(n))  {
      t << "{\\begin{tabularx}{\\linewidth}{|*{" << cols << "}{>{\\raggedright\\arraybackslash}X|}}";

   } else {
      t << "\\tabulinesep=1mm\n\\begin{longtabu}spread 0pt [c]{*{" << cols << "}{|X[-1]}|}\n";
   }

   // return isNested ? "TabularNC" : "TabularC";
}

static void writeEndTableCommand(QTextStream &t, const DocNode *n)
{
   if (tableIsNested(n)) {
      t << "\\end{tabularx}}\n";

   } else {
      t << "\\end{longtabu}\n";
   }

   // return isNested ? "TabularNC" : "TabularC";
}

void LatexDocVisitor::visitPre(DocHtmlTable *t)
{
   if (m_hide) {
      return;
   }

   pushTableState();

   if (t->hasCaption()) {
      DocHtmlCaption *c = t->caption();

      static bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

      if (! c->file().isEmpty() && pdfHyperlinks) {
         m_t << "\\hypertarget{" << stripPath(c->file()) << "_" << c->anchor() << "}{}";
      }

      m_t << endl;
   }

   writeStartTableCommand(m_t, t->parent(), t->numColumns());

   if (t->hasCaption()) {
      DocHtmlCaption *c = t->caption();
      m_t << "\\caption{";

      visitCaption(this, c->children());

      m_t << "}";
      m_t << "\\label{" << stripPath(c->file()) << "_" << c->anchor() << "}";
      m_t << "\\\\\n";
   }

   setNumCols(t->numColumns());
   m_t << "\\hline\n";

   // check if first row is a heading and then render the row already here
   // and end it with \endfirsthead (triggered via m_firstRow == TRUE)
   // then repeat the row as normal and end it with \endhead (m_firstRow == FALSE)

   DocHtmlRow *firstRow = t->firstRow();

   if (firstRow && firstRow->isHeading()) {
      setFirstRow(true);
      DocNode *n = t->parent();

      if (! tableIsNested(n)) {
         firstRow->accept(this);
      }

      setFirstRow(false);
   }
}

void LatexDocVisitor::visitPost(DocHtmlTable *t)
{
   if (m_hide) {
      return;
   }

   writeEndTableCommand(m_t, t->parent());
   popTableState();
}

void LatexDocVisitor::visitPre(DocHtmlCaption *c)
{
   (void) c;

   m_hideCaption = m_hide;
   m_hide        = true;
}

void LatexDocVisitor::visitPost(DocHtmlCaption *)
{
   m_hide = m_hideCaption;
}

void LatexDocVisitor::visitPre(DocHtmlRow *)
{
   setCurrentColumn(0);
}

void LatexDocVisitor::visitPost(DocHtmlRow *row)
{
   if (m_hide) {
      return;
   }

   DocNode *n = row->parent() ->parent();

   int c = currentColumn();

   while (c <= numCols()) {
      // end of row while inside a row span?

      for (int i = 0; i < rowSpans().count(); ++i) {
         const ActiveRowSpan &span = rowSpans()[i];

         if (span.rowSpan > 0 && span.column == c && row->rowIndex() > span.cell->rowIndex() ) {
            // at a cell in a row span, but not the row that started the span

            m_t << "&";

            if (span.colSpan > 1) {
               // row span is also part of a column span

               m_t << "\\multicolumn{" << span.colSpan << "}{";
               m_t << "}|}{}";

            } else {
               // solitary row span
               m_t << "\\multicolumn{1}{c|}{}";
            }
         }
      }

      ++c;
   }

   m_t << "\\\\";

   int col = 1;

   for (int i = 0; i < rowSpans().count(); i++) {
      ActiveRowSpan &span = const_cast<ActiveRowSpan &>(rowSpans()[i]);

      if (span.rowSpan > 0) {
         --span.rowSpan;
      }

      if (span.rowSpan <= 0) {
         // inactive span

      } else if (span.column > col) {
         m_t << "\\cline{" << col << "-" << (span.column - 1) << "}";
         col = span.column + span.colSpan;

      } else {
         col = span.column + span.colSpan;
      }
   }

   if (col <= numCols()) {
      m_t << "\\cline{" << col << "-" << numCols() << "}";
   }

   m_t << "\n";

   if (row->isHeading() && row->rowIndex() == 1 && ! tableIsNested(n)) {

      if (firstRow()) {
         m_t << "\\endfirsthead" << endl;
         m_t << "\\hline" << endl;
         m_t << "\\endfoot" << endl;
         m_t << "\\hline" << endl;

      } else {
         m_t << "\\endhead" << endl;
      }

   }
}

void LatexDocVisitor::visitPre(DocHtmlCell *c)
{
   if (m_hide) {
      return;
   }

   DocHtmlRow *row = nullptr;
   if (c->parent() && c->parent()->kind() == DocNode::Kind_HtmlRow) {
      row = (DocHtmlRow *)c->parent();
   }

   setCurrentColumn(currentColumn() + 1);

   // Skip columns that span from above
   for (int i = 0; i < rowSpans().count(); ++i) {
      const ActiveRowSpan &span = rowSpans()[i];

      if (span.rowSpan > 0 && span.column == currentColumn()) {

         if (row && span.colSpan > 1) {
            m_t << "\\multicolumn{" << span.colSpan << "}{";

            if (currentColumn() == 1) {
               // add extra | for first column
               m_t << "|";
            }

            // alignment not relevant, empty column
            m_t << "l|}{" << (c->isHeading() ? "\\columncolor{\\tableheadbgcolor}" : "") << "}";

            setCurrentColumn(currentColumn() + span.colSpan);

         } else {
            setCurrentColumn(currentColumn() + 1);
         }

         m_t << "&";
      }
   }

   int cs    = c->colSpan();
   int align = c->alignment();

   if (cs > 1 && row) {
      setInColSpan(true);

      m_t << "\\multicolumn{" << cs << "}{";

      if (c->columnIndex() == 1) {
         // add extra | for first column
         m_t << "|";
      }
      switch (align) {
         case DocHtmlCell::Right:
          m_t << "r|}{";
            break;

         case DocHtmlCell::Center:
            m_t << "c|}{";
            break;

         default:
            m_t << "l|}{";
            break;
      }

   }

   int rs = c->rowSpan();

   if (rs > 0) {
      setInRowSpan(true);
      addRowSpan(ActiveRowSpan(c, rs, cs, currentColumn()));

      m_t << "\\multirow{" << rs << "}{*}{";
   }

   if (align == DocHtmlCell::Center) {
      m_t << "\\PBS\\centering ";

   } else if (align == DocHtmlCell::Right) {
      m_t << "\\PBS\\raggedleft ";

   }

   if (c->isHeading()) {
      m_t << "\\cellcolor{\\tableheadbgcolor}\\textbf{ ";
   }

   if (cs > 1) {
      setCurrentColumn(currentColumn() + cs - 1);
   }
}

void LatexDocVisitor::visitPost(DocHtmlCell *c)
{
   if (m_hide) {
      return;
   }

   if (c->isHeading()) {
      m_t << "}";
   }

   if (inRowSpan()) {
      setInRowSpan(false);
      m_t << "}";
   }

   if (inColSpan()) {
       setInColSpan(false);
      m_t << "}";
   }

   if (! c->isLast()) {
      m_t << "&";
   }
}

void LatexDocVisitor::visitPre(DocInternal *)
{
   if (m_hide) {
      return;
   }
}

void LatexDocVisitor::visitPost(DocInternal *)
{
   if (m_hide) {
      return;
   }
}

void LatexDocVisitor::visitPre(DocHRef *href)
{
   static bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   if (m_hide) {
      return;
   }

   if (pdfHyperlinks) {
      m_t << "\\href{";
      m_t << latexFilterURL(href->url());
      m_t << "}";
   }

   m_t << "{\\texttt{ ";
}

void LatexDocVisitor::visitPost(DocHRef *)
{
   if (m_hide) {
      return;
   }

   m_t << "}}";
}

void LatexDocVisitor::visitPre(DocHtmlHeader *header)
{
   if (m_hide) {
      return;
   }

   m_t << "\\" << getSectionName(header->level()) << "*{";
}

void LatexDocVisitor::visitPost(DocHtmlHeader *)
{
   if (m_hide) {
      return;
   }

   m_t << "}";
}

void LatexDocVisitor::visitPre(DocImage *img)
{
   if (img->type() == DocImage::Latex) {
      if (m_hide) {
         return;
      }

      QString gfxName = img->name();

      if (gfxName.endsWith(".eps") || gfxName.endsWith(".pdf")) {
         gfxName = gfxName.left(gfxName.length() - 4);
      }

      visitPreStart(m_t, img->hasCaption(), gfxName, img->width(), img->height(), img->isInlineImage());

   } else {
      // other format -> skip
      pushEnabled();
      m_hide = true;
   }
}

void LatexDocVisitor::visitPost(DocImage *img)
{
   if (img->type() == DocImage::Latex) {
      if (m_hide) {
         return;
      }

      visitPostEnd(m_t, img->hasCaption(), img->isInlineImage());

   } else {
      // other format
      popEnabled();
   }
}

void LatexDocVisitor::visitPre(DocDotFile *df)
{
   if (m_hide) {
      return;
   }

   startDotFile(df->file(), df->width(), df->height(), df->hasCaption());
}

void LatexDocVisitor::visitPost(DocDotFile *df)
{
   if (m_hide) {
      return;
   }

   endDotFile(df->hasCaption());
}
void LatexDocVisitor::visitPre(DocMscFile *df)
{
   if (m_hide) {
      return;
   }

   startMscFile(df->file(), df->width(), df->height(), df->hasCaption());
}

void LatexDocVisitor::visitPost(DocMscFile *df)
{
   if (m_hide) {
      return;
   }

   endMscFile(df->hasCaption());
}

void LatexDocVisitor::visitPre(DocDiaFile *df)
{
   if (m_hide) {
      return;
   }

   startDiaFile(df->file(), df->width(), df->height(), df->hasCaption());
}

void LatexDocVisitor::visitPost(DocDiaFile *df)
{
   if (m_hide) {
      return;
   }

   endDiaFile(df->hasCaption());
}
void LatexDocVisitor::visitPre(DocLink *lnk)
{
   if (m_hide) {
      return;
   }
   startLink(lnk->ref(), lnk->file(), lnk->anchor());
}

void LatexDocVisitor::visitPost(DocLink *lnk)
{
   if (m_hide) {
      return;
   }
   endLink(lnk->ref(), lnk->file(), lnk->anchor());
}

void LatexDocVisitor::visitPre(DocRef *ref)
{
   if (m_hide) {
      return;
   }

   // when ref->isSubPage()==true we use ref->file() for HTML and
   // ref->anchor() for LaTeX/RTF

   if (ref->isSubPage()) {
      startLink(ref->ref(), QString(), ref->anchor());

   } else {

      if (! ref->file().isEmpty()) {
         startLink(ref->ref(), ref->file(), ref->anchor(), ref->refToTable());
      }
   }

   if (! ref->hasLinkText()) {
      filter(ref->targetTitle());
   }
}

void LatexDocVisitor::visitPost(DocRef *ref)
{
   if (m_hide) {
      return;
   }

   if (ref->isSubPage()) {
      endLink(ref->ref(), QString(), ref->anchor());

   } else {
      if (! ref->file().isEmpty()) {
         endLink(ref->ref(), ref->file(), ref->anchor(), ref->refToTable());
      }
   }
}

void LatexDocVisitor::visitPre(DocSecRefItem *ref)
{
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   if (m_hide) {
      return;
   }

   m_t << "\\item \\contentsline{section}{";

   if (pdfHyperlinks) {
      m_t << "\\mbox{\\hyperlink{" << ref->file() << "_" << ref->anchor() << "}{" ;
   }
}

void LatexDocVisitor::visitPost(DocSecRefItem *ref)
{
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   if (m_hide) {
      return;
   }

   if (pdfHyperlinks) {
      m_t << "}}";
   }

   m_t << "}{\\ref{" << ref->file() << "_" << ref->anchor() << "}}{}" << endl;
}

void LatexDocVisitor::visitPre(DocSecRefList *)
{
   if (m_hide) {
      return;
   }
   m_t << "\\footnotesize" << endl;
   m_t << "\\begin{multicols}{2}" << endl;
   m_t << "\\begin{DoxyCompactList}" << endl;
}

void LatexDocVisitor::visitPost(DocSecRefList *)
{
   if (m_hide) {
      return;
   }
   m_t << "\\end{DoxyCompactList}" << endl;
   m_t << "\\end{multicols}" << endl;
   m_t << "\\normalsize" << endl;
}

void LatexDocVisitor::visitPre(DocParamSect *s)
{
   if (m_hide) {
      return;
   }

   bool hasInOutSpecs = s->hasInOutSpecifier();
   bool hasTypeSpecs  = s->hasTypeSpecifier();

   m_ci.incUsedTableLevel();

   switch (s->type()) {
      case DocParamSect::Param:
         m_t << "\n\\begin{DoxyParams}";
         if      (hasInOutSpecs && hasTypeSpecs) {
            m_t << "[2]";   // 2 extra cols

         } else if (hasInOutSpecs || hasTypeSpecs) {
            m_t << "[1]";   // 1 extra col

         }

         m_t << "{";
         filter(theTranslator->trParameters());
         break;

      case DocParamSect::RetVal:
         m_t << "\n\\begin{DoxyRetVals}{";
         filter(theTranslator->trReturnValues());
         break;

      case DocParamSect::Exception:
         m_t << "\n\\begin{DoxyExceptions}{";
         filter(theTranslator->trExceptions());
         break;

      case DocParamSect::TemplateParam:
         m_t << "\n\\begin{DoxyTemplParams}{";
         filter(theTranslator->trTemplateParameters());
         break;

      default:
         assert(0);
   }

   m_t << "}\n";
}

void LatexDocVisitor::visitPost(DocParamSect *s)
{
   if (m_hide) {
      return;
   }

   m_ci.decUsedTableLevel();

   switch (s->type()) {
      case DocParamSect::Param:
         m_t << "\\end{DoxyParams}\n";
         break;

      case DocParamSect::RetVal:
         m_t << "\\end{DoxyRetVals}\n";
         break;

      case DocParamSect::Exception:
         m_t << "\\end{DoxyExceptions}\n";
         break;

      case DocParamSect::TemplateParam:
         m_t << "\\end{DoxyTemplParams}\n";
         break;

      default:
         assert(0);
   }
}

void LatexDocVisitor::visitPre(DocParamList *pl)
{
   if (m_hide) {
      return;
   }

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

   if (!useTable) {
      m_t << "\\item[";
   }

   if (sect && sect->hasInOutSpecifier()) {
      if (pl->direction() != DocParamSect::Unspecified) {
         m_t << "\\mbox{\\texttt{ ";

         if (pl->direction() == DocParamSect::In) {
            m_t << "in";
         } else if (pl->direction() == DocParamSect::Out) {
            m_t << "out";
         } else if (pl->direction() == DocParamSect::InOut) {
            m_t << "in,out";
         }
         m_t << "}} ";
      }
      if (useTable) {
         m_t << " & ";
      }
   }

   if (sect && sect->hasTypeSpecifier()) {


      for (auto type : pl->paramTypes() ) {

         if (type->kind() == DocNode::Kind_Word) {
            visit((DocWord *)type);
         } else if (type->kind() == DocNode::Kind_LinkedWord) {
            visit((DocLinkedWord *)type);

         }  else if (type->kind() == DocNode::Kind_Sep) {
            m_t << " " << ((DocSeparator *)type)->chars() << " ";
         }
      }

      if (useTable) {
         m_t << " & ";
      }
   }

   m_t << "{\\em ";

   bool first = true;

   for (auto param : pl->parameters() ) {
      if (!first) {
         m_t << ",";
      } else {
         first = false;
      }

      m_insideItem = true;

      if (param->kind() == DocNode::Kind_Word) {
         visit((DocWord *)param);

      } else if (param->kind() == DocNode::Kind_LinkedWord) {
         visit((DocLinkedWord *)param);
      }

      m_insideItem = false;
   }

   m_t << "}";
   if (useTable) {
      m_t << " & ";
   } else {
      m_t << "]";
   }
}

void LatexDocVisitor::visitPost(DocParamList *pl)
{
   if (m_hide) {
      return;
   }

   DocParamSect::Type parentType = DocParamSect::Unknown;
   if (pl->parent() && pl->parent()->kind() == DocNode::Kind_ParamSect) {
      parentType = ((DocParamSect *)pl->parent())->type();
   }

   bool useTable = parentType == DocParamSect::Param ||
                   parentType == DocParamSect::RetVal ||
                   parentType == DocParamSect::Exception ||
                   parentType == DocParamSect::TemplateParam;

   if (useTable) {
      m_t << "\\\\" << endl
          << "\\hline" << endl;
   }
}

void LatexDocVisitor::visitPre(DocXRefItem *x)
{
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   if (m_hide) {
      return;
   }

   if (x->title().isEmpty()) {
      return;
   }

   m_t << "\\begin{DoxyRefDesc}{";
   filter(x->title());
   m_t << "}" << endl;

   bool anonymousEnum = x->file() == "@";
   m_t << "\\item[";

   if (pdfHyperlinks && ! anonymousEnum) {
      m_t << "\\mbox{\\hyperlink{" << stripPath(x->file()) << "_" << x->anchor() << "}{";
   } else {
      m_t << "\\textbf{ ";
   }

   m_insideItem = true;
   filter(x->title());

   m_insideItem = false;

   if (pdfHyperlinks && ! anonymousEnum) {
    m_t << "}";
   }

   m_t << "}]";
}

void LatexDocVisitor::visitPost(DocXRefItem *x)
{
   if (m_hide) {
      return;
   }

   if (x->title().isEmpty()) {
      return;
   }

   m_t << "\\end{DoxyRefDesc}" << endl;
}

void LatexDocVisitor::visitPre(DocInternalRef *ref)
{
   if (m_hide) {
      return;
   }

   startLink(QString(), ref->file(), ref->anchor());
}

void LatexDocVisitor::visitPost(DocInternalRef *ref)
{
   if (m_hide) {
      return;
   }
   endLink(QString(), ref->file(), ref->anchor());
}

void LatexDocVisitor::visitPre(DocCopy *)
{
}

void LatexDocVisitor::visitPost(DocCopy *)
{
}

void LatexDocVisitor::visitPre(DocText *)
{
}

void LatexDocVisitor::visitPost(DocText *)
{
}

void LatexDocVisitor::visitPre(DocHtmlBlockQuote *)
{
   if (m_hide) {
      return;
   }
   m_t << "\\begin{quote}" << endl;
}

void LatexDocVisitor::visitPost(DocHtmlBlockQuote *)
{
   if (m_hide) {
      return;
   }
   m_t << "\\end{quote}" << endl;
}

void LatexDocVisitor::visitPre(DocParBlock *)
{
   if (m_hide) {
      return;
   }
}

void LatexDocVisitor::visitPost(DocParBlock *)
{
   if (m_hide) {
      return;
   }
}

void LatexDocVisitor::filter(const QString &str)
{
   filterLatexString(m_t, str, m_insideTabbing, m_insidePre, m_insideItem, m_ci.usedTableLevel() > 0, false);
}

void LatexDocVisitor::startLink(const QString &ref, const QString &file, const QString &anchor, bool refToTable)
{
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   if (ref.isEmpty() && pdfHyperlinks) {
      // internal PDF link

      if (refToTable) {
         m_t << "\\doxytablelink{";

      } else {
         m_t << "\\mbox{\\hyperlink{";
      }

      if (! file.isEmpty()) {
         m_t << stripPath(file);
      }

      if (! file.isEmpty() && ! anchor.isEmpty()) {
         m_t << "_";
      }

      if (! anchor.isEmpty()) {
         m_t << anchor;
      }

      m_t << "}{";

   } else if (ref.isEmpty() && refToTable) {
      m_t << "\\doxytableref{";

   } else if (ref.isEmpty()) {
      // internal non-PDF link
      m_t << "\\doxyref{";

   } else {
      // external link
      m_t << "\\textbf{ ";
   }
}

void LatexDocVisitor::endLink(const QString &ref, const QString &file, const QString &anchor, bool refToTable)
{
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   m_t << "}";

   if (ref.isEmpty() && ! pdfHyperlinks) {
      m_t << "{";
      filter(theTranslator->trPageAbbreviation());

      m_t << "}{" << file;
      if (! file.isEmpty() && ! anchor.isEmpty()) {
         m_t << "_";
      }

      m_t << anchor << "}";
   }

   if (ref.isEmpty() && pdfHyperlinks) {
      // internal PDF link

      if (! refToTable) {
         m_t << "}";
      }
   }
}

void LatexDocVisitor::pushEnabled()
{
   m_enabled.push(m_hide);
}

void LatexDocVisitor::popEnabled()
{
   bool v = m_enabled.pop();
   m_hide = v;
}

void LatexDocVisitor::startDotFile(const QString &fileName, const QString &width, const QString &height, bool hasCaption)
{
   static const QString latexOutput = Config::getString("latex-output");

   QString baseName = fileName;
   int i;

   if ((i = baseName.lastIndexOf('/')) != -1) {
      baseName = baseName.right(baseName.length() - i - 1);
   }

   if ((i = baseName.indexOf('.')) != -1) {
      baseName = baseName.left(i);
   }

   baseName.prepend("dot_");

   QString name = fileName;

   writeDotGraphFromFile(name, latexOutput, baseName, GOF_EPS);
   visitPreStart(m_t, hasCaption, baseName, width, height);
}

void LatexDocVisitor::endDotFile(bool hasCaption)
{
   if (m_hide) {
      return;
   }

   visitPostEnd(m_t, hasCaption);
}

void LatexDocVisitor::startMscFile(const QString &fileName, const QString &width, const QString &height, bool hasCaption)
{
   static const QString latexOutput = Config::getString("latex-output");

   QString baseName = fileName;
   int i;

   if ((i = baseName.lastIndexOf('/')) != -1) {
      baseName = baseName.right(baseName.length() - i - 1);
   }

   if ((i = baseName.indexOf('.')) != -1) {
      baseName = baseName.left(i);
   }

   baseName.prepend("msc_");

   writeMscGraphFromFile(fileName, latexOutput, baseName, MSC_EPS);
   visitPreStart(m_t,hasCaption, baseName, width, height);
}

void LatexDocVisitor::endMscFile(bool hasCaption)
{
   if (m_hide) {
      return;
   }

   visitPostEnd(m_t, hasCaption);
}

void LatexDocVisitor::writeMscFile(const QString &baseName, DocVerbatim *s)
{
   static const QString latexOutput = Config::getString("latex-output");

   QString shortName = baseName;
   int i;

   if ((i = shortName.lastIndexOf('/')) != -1) {
      shortName = shortName.right(shortName.length() - i - 1);
   }

   writeMscGraphFromFile(baseName + ".msc", latexOutput, shortName, MSC_EPS);

   visitPreStart(m_t, s->hasCaption(), shortName, s->width(),s->height());
   visitCaption(this, s->children());
   visitPostEnd(m_t,  s->hasCaption());
}

void LatexDocVisitor::startDiaFile(const QString &fileName, const QString &width, const QString &height, bool hasCaption)
{
   static const QString latexOutput = Config::getString("latex-output");

   QString baseName = fileName;
   int i;

   if ((i = baseName.lastIndexOf('/')) != -1) {
      baseName = baseName.right(baseName.length() - i - 1);
   }

   if ((i = baseName.indexOf('.')) != -1) {
      baseName = baseName.left(i);
   }

   baseName.prepend("dia_");

   writeDiaGraphFromFile(fileName, latexOutput, baseName, DIA_EPS);
   visitPreStart(m_t,hasCaption, baseName, width, height);
}

void LatexDocVisitor::endDiaFile(bool hasCaption)
{
   if (m_hide) {
      return;
   }

   visitPostEnd(m_t, hasCaption);
}

void LatexDocVisitor::writeDiaFile(const QString &baseName, DocVerbatim *s)
{
   static const QString latexOutput = Config::getString("latex-output");

   QString shortName = baseName;
   int i;

   if ((i = shortName.lastIndexOf('/')) != -1) {
      shortName = shortName.right(shortName.length() - i - 1);
   }

   writeDiaGraphFromFile(baseName + ".dia", latexOutput, shortName, DIA_EPS);

   visitPreStart(m_t, s->hasCaption(), shortName, s->width(), s->height());
   visitCaption(this, s->children());
   visitPostEnd(m_t, s->hasCaption());
}

void LatexDocVisitor::writePlantUMLFile(const QString &baseName, DocVerbatim *s)
{
   static const QString latexOutput = Config::getString("latex-output");

   QString shortName = baseName;
   int i;

   if ((i = shortName.lastIndexOf('/')) != -1) {
      shortName = shortName.right(shortName.length() - i - 1);
   }

   generatePlantUMLOutput(baseName, latexOutput, PUML_EPS);

   visitPreStart(m_t, s->hasCaption(), shortName, s->width(), s->height());
   visitCaption(this, s->children());
   visitPostEnd(m_t, s->hasCaption());
}
