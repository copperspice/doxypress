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

#include <QFileInfo>

#include <docbookvisitor.h>

#include <config.h>
#include <dia.h>
#include <docparser.h>
#include <outputgen.h>
#include <docbookgen.h>
#include <doxy_globals.h>
#include <dot.h>
#include <htmlentity.h>
#include <language.h>
#include <message.h>
#include <msc.h>
#include <parse_base.h>
#include <plantuml.h>
#include <util.h>

static void visitCaption(DocbookDocVisitor *parent, QList<DocNode *> children)
{ 
   for (auto n : children) {
      n->accept(parent);
   }
}

static void visitPreStart(QTextStream &t, const bool hasCaption, const QString &name,  
                  const QString &width, const QString &height)
{
  QString tmpStr;
  t << "    <figure>" << endl;
  t << "        <title></title>" << endl;
  t << "        <mediaobject>" << endl;
  t << "            <imageobject>" << endl;
  t << "                <imagedata";

  if (! width.isEmpty()) {  
    t << " width=\"" << convertToXML(width) << "\"";

  } else {
    t << " width=\"50%\"";
  }

  if (! height.isEmpty()) {
      t << " depth=\"" << convertToXML(tmpStr) << "\"";   
  }

  t << " align=\"center\" valign=\"middle\" scalefit=\"1\" fileref=\"" << name << "\">";
  t << "</imagedata>" << endl;
  t << "            </imageobject>" << endl;

  if (hasCaption) {
    t << "        <caption>" << endl;
  }
}

static void visitPostEnd(QTextStream &t, const bool hasCaption)
{
   t << endl;

   if (hasCaption) {
      t << "        </caption>" << endl;
   }

   t << "        </mediaobject>" << endl;
   t << "    </figure>" << endl;
}

DocbookDocVisitor::DocbookDocVisitor(QTextStream &t, CodeOutputInterface &ci)
   : DocVisitor(DocVisitor_Docbook), m_t(t), m_ci(ci), m_insidePre(false), m_hide(false)
{
}

// visitor functions for leaf nodes
void DocbookDocVisitor::visit(DocWord *w)
{
   if (m_hide) {
      return;
   }
   filter(w->word());
}

void DocbookDocVisitor::visit(DocLinkedWord *w)
{
   if (m_hide) {
      return;
   }

   startLink(w->file(), w->anchor());
   filter(w->word());
   endLink();
}

void DocbookDocVisitor::visit(DocWhiteSpace *w)
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

void DocbookDocVisitor::visit(DocSymbol *s)
{
   if (m_hide) {
      return;
   }

   QString res = HtmlEntityMapper::instance()->docbook(s->symbol());

   if (! res.isEmpty()) {
      m_t << res;

   } else {
      err("DocBook unsupported HTML entity found: %s\n", qPrintable(HtmlEntityMapper::instance()->html(s->symbol(), true)) );
   }
}

void DocbookDocVisitor::visit(DocURL *u)
{
   if (m_hide) {
      return;
   }
   m_t << "<link xlink:href=\"";
   if (u->isEmail()) {
      m_t << "mailto:";
   }
   filter(u->url());
   m_t << "\">";
   filter(u->url());
   m_t << "</link>";
}

void DocbookDocVisitor::visit(DocLineBreak *)
{
   if (m_hide) {
      return;
   }
   m_t << endl << "<literallayout>\n</literallayout>" << endl;
}

void DocbookDocVisitor::visit(DocHorRuler *)
{
   if (m_hide) {
      return;
   }
   m_t << "<informaltable frame='bottom'><tgroup cols='1'><colspec align='center'/><tbody><row><entry align='center'>\n";
   m_t << "</entry></row></tbody></tgroup></informaltable>\n";
}

void DocbookDocVisitor::visit(DocStyleChange *s)
{
   if (m_hide) {
      return;
   }
   switch (s->style()) {
      case DocStyleChange::Bold:
         if (s->enable()) {
            m_t << "<emphasis role=\"bold\">";
         }      else {
            m_t << "</emphasis>";
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
            m_t << "<informaltable frame='none'><tgroup cols='1'><colspec align='center'/><tbody><row><entry align='center'>";
         } else {
            m_t << "</entry></row></tbody></tgroup></informaltable>";
         }
         break;
      case DocStyleChange::Preformatted:
         if (s->enable()) {
            m_t << "<literallayout>";
            m_insidePre = true;
         } else {
            m_t << "</literallayout>";
            m_insidePre = false;
         }
         break;
      /* There is no equivalent Docbook tag for rendering Small text */
      case DocStyleChange::Small: /* XSLT Stylesheets can be used */
         break;
      /* HTML only */
      case DocStyleChange::Div:  /* HTML only */
         break;
      case DocStyleChange::Span: /* HTML only */
         break;
   }
}

void DocbookDocVisitor::visit(DocVerbatim *s)
{
   if (m_hide) {
      return;
   }

   static const QString docbookOutDir = Config::getString("docbook-output");

   SrcLangExt langExt = getLanguageFromFileName(m_langExt);

   switch (s->type()) {
      case DocVerbatim::Code: 
         m_t << "<literallayout><computeroutput>";
         Doxy_Globals::parserManager->getParser(m_langExt)->parseCode(m_ci, s->context(), s->text(), 
                  langExt, s->isExample(), s->exampleFile());

         m_t << "</computeroutput></literallayout>";
         break;

      case DocVerbatim::Verbatim:
         m_t << "<literallayout>";
         filter(s->text());
         m_t << "</literallayout>";
         break;

      case DocVerbatim::HtmlOnly:
         break;

      case DocVerbatim::RtfOnly:
         break;

      case DocVerbatim::ManOnly:
         break;
      case DocVerbatim::LatexOnly:
         break;
      case DocVerbatim::XmlOnly:
         break;
      case DocVerbatim::DocbookOnly:
         break;
         m_t << s->text();
         break;

      case DocVerbatim::Dot: {
         static int dotindex = 1;
        
         QString name;
         QString baseName;       
         QString tempStr = docbookOutDir + "/inline_dotgraph_";
         QString stext   = s->text();

         m_t << "<para>" << endl;

         name = QString("dot_inline_dotgraph_%1").arg(dotindex);
         baseName = QString("tempStr%1").arg(dotindex++);

         QFile file(baseName + ".dot");

         if (! file.open(QIODevice::WriteOnly)) {
            err("Unable to open file for writing %s.dot, error: %d\n", csPrintable(baseName), file.error());          
         }  

         file.write( stext.toUtf8() );
         file.close();
        
         writeDotFile(baseName, s);
         m_t << "</para>" << endl;         
      }
      break;

      case DocVerbatim::Msc: {
         static int mscindex = 1;

         QString name;
         QString baseName;  
         QString tempStr = docbookOutDir + "/inline_mscgraph_";
         QString stext   = s->text();

         m_t << "<para>" << endl;
         
         name = QString("msc_inline_mscgraph_%1").arg(mscindex);
         baseName = QString("tempStr%1").arg(mscindex++);

         QFile file(baseName + ".msc");

         if (! file.open(QIODevice::WriteOnly)) {
            err("Unable to open file for writing %s.msc, error: %d\n", csPrintable(baseName), file.error());             
         }

         QString text = "msc {" + stext + "}";

         file.write( text.toUtf8() );
         file.close();

         writeMscFile(baseName,s);
         m_t << "</para>" << endl;
      }
      break;

      case DocVerbatim::PlantUML: {        
         QString baseName  = writePlantUMLSource(docbookOutDir, s->exampleFile(), s->text());
         QString shortName = baseName;

         int i;

         if ((i = shortName.lastIndexOf('/')) != -1) {
            shortName = shortName.right(shortName.length() - i - 1);
         }

         m_t << "<para>" << endl;
         writePlantUMLFile(baseName,s);
         m_t << "</para>" << endl;
      }
      break;
   }
}

void DocbookDocVisitor::visit(DocAnchor *anc)
{
   if (m_hide) {
      return;
   }
   m_t << "<anchor id=\"" << anc->file() << "_1" << anc->anchor() << "\"/>";
}

void DocbookDocVisitor::visit(DocInclude *inc)
{
   if (m_hide) {
      return;
   }

   SrcLangExt langExt = getLanguageFromFileName(inc->extension());

   switch (inc->type()) {
      case DocInclude::IncWithLines: {
         m_t << "<literallayout><computeroutput>";

         QFileInfo cfi( inc->file() );
         QSharedPointer<FileDef> fd = QMakeShared<FileDef>(cfi.path(), cfi.fileName());

         Doxy_Globals::parserManager->getParser(inc->extension())->parseCode(m_ci, inc->context(), 
                  inc->text(), langExt, inc->isExample(), inc->exampleFile(), fd);

         m_t << "</computeroutput></literallayout>";
      }
      break;

      case DocInclude::Include:
         m_t << "<literallayout><computeroutput>";
         Doxy_Globals::parserManager->getParser(inc->extension())->parseCode(m_ci, inc->context(), 
                  inc->text(),langExt, inc->isExample(), inc->exampleFile());

         m_t << "</computeroutput></literallayout>";
         break;

      case DocInclude::DontInclude:
         break;
      case DocInclude::HtmlInclude:
         break;
      case DocInclude::LatexInclude:
         break;
      case DocInclude::VerbInclude:
         m_t << "<verbatim>";
         filter(inc->text());
         m_t << "</verbatim>";
         break;

      case DocInclude::Snippet:
         m_t << "<literallayout><computeroutput>";
         Doxy_Globals::parserManager->getParser(inc->extension())->parseCode(m_ci, inc->context(), 
                  extractBlock(inc->text(), inc->blockId()), langExt, inc->isExample(), inc->exampleFile() );

         m_t << "</computeroutput></literallayout>";
         break;
   }
}

void DocbookDocVisitor::visit(DocIncOperator *op)
{
   if (op->isFirst()) {
      if ( !m_hide) {
         m_t << "<programlisting>";
      }
      pushEnabled();
      m_hide = true;
   }

   SrcLangExt langExt = getLanguageFromFileName(m_langExt);

   if (op->type() != DocIncOperator::Skip) {
      popEnabled();
      if (!m_hide) {
         Doxy_Globals::parserManager->getParser(m_langExt)
         ->parseCode(m_ci, op->context(),
                     op->text(), langExt, op->isExample(),
                     op->exampleFile());
      }
      pushEnabled();
      m_hide = true;
   }
   if (op->isLast()) {
      popEnabled();
      if (!m_hide) {
         m_t << "</programlisting>";
      }
   } else {
      if (!m_hide) {
         m_t << endl;
      }
   }
}

void DocbookDocVisitor::visit(DocFormula *f)
{
   if (m_hide) {
      return;
   }
   m_t << "<equation><title>" << f->name() << "</title>";
   filter(f->text());
   m_t << "</equation>";
}

void DocbookDocVisitor::visit(DocIndexEntry *ie)
{
   if (m_hide) {
      return;
   }
   m_t << "<indexentry><primaryie>" << endl;
   filter(ie->entry());
   m_t << "</primaryie><secondaryie></secondaryie></indexentry>" << endl;
}

void DocbookDocVisitor::visit(DocSimpleSectSep *)
{
   m_t << "<simplesect/>";
}

void DocbookDocVisitor::visit(DocCite *cite)
{
   if (m_hide) {
      return;
   }
   if (! cite->file().isEmpty()) {
      startLink(cite->file(), cite->anchor());
   }
   filter(cite->text());
   if (!cite->file().isEmpty()) {
      endLink();
   }
}

//--------------------------------------
// visitor functions for compound nodes
//--------------------------------------

void DocbookDocVisitor::visitPre(DocAutoList *l)
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

void DocbookDocVisitor::visitPost(DocAutoList *l)
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

void DocbookDocVisitor::visitPre(DocAutoListItem *)
{
   if (m_hide) {
      return;
   }
   m_t << "<listitem>";
}

void DocbookDocVisitor::visitPost(DocAutoListItem *)
{
   if (m_hide) {
      return;
   }
   m_t << "</listitem>";
}

void DocbookDocVisitor::visitPre(DocPara *)
{
   if (m_hide) {
      return;
   }
   m_t << endl;
   m_t << "<para>";
}

void DocbookDocVisitor::visitPost(DocPara *)
{
   if (m_hide) {
      return;
   }
   m_t << "</para>";
   m_t << endl;
}

void DocbookDocVisitor::visitPre(DocRoot *)
{
   //m_t << "<hr><h4><font color=\"red\">New parser:</font></h4>\n";
}

void DocbookDocVisitor::visitPost(DocRoot *)
{
   //m_t << "<hr><h4><font color=\"red\">Old parser:</font></h4>\n";
}

void DocbookDocVisitor::visitPre(DocSimpleSect *s)
{
   if (m_hide) {
      return;
   }
   switch (s->type()) {
      case DocSimpleSect::See:
         if (m_insidePre) {
            m_t << "<formalpara><title>" << theTranslator->trSeeAlso() << ": </title>" << endl;
         } else {
            m_t << "<formalpara><title>" << convertToXML(theTranslator->trSeeAlso()) << ": </title>" << endl;
         }
         break;
      case DocSimpleSect::Return:
         if (m_insidePre) {
            m_t << "<formalpara><title>" << theTranslator->trReturns() << ": </title>" << endl;
         } else {
            m_t << "<formalpara><title>" << convertToXML(theTranslator->trReturns()) << ": </title>" << endl;
         }
         break;
      case DocSimpleSect::Author:
         if (m_insidePre) {
            m_t << "<formalpara><title>" << theTranslator->trAuthor(true, true) << ": </title>" << endl;
         } else {
            m_t << "<formalpara><title>" << convertToXML(theTranslator->trAuthor(true, true)) << ": </title>" << endl;
         }
         break;
      case DocSimpleSect::Authors:
         if (m_insidePre) {
            m_t << "<formalpara><title>" << theTranslator->trAuthor(true, false) << ": </title>" << endl;
         } else {
            m_t << "<formalpara><title>" << convertToXML(theTranslator->trAuthor(true, false)) << ": </title>" << endl;
         }
         break;
      case DocSimpleSect::Version:
         if (m_insidePre) {
            m_t << "<formalpara><title>" << theTranslator->trVersion() << ": </title>" << endl;
         } else {
            m_t << "<formalpara><title>" << convertToXML(theTranslator->trVersion()) << ": </title>" << endl;
         }
         break;
      case DocSimpleSect::Since:
         if (m_insidePre) {
            m_t << "<formalpara><title>" << theTranslator->trSince() << ": </title>" << endl;
         } else {
            m_t << "<formalpara><title>" << convertToXML(theTranslator->trSince()) << ": </title>" << endl;
         }
         break;
      case DocSimpleSect::Date:
         if (m_insidePre) {
            m_t << "<formalpara><title>" << theTranslator->trDate() << ": </title>" << endl;
         } else {
            m_t << "<formalpara><title>" << convertToXML(theTranslator->trDate()) << ": </title>" << endl;
         }
         break;
      case DocSimpleSect::Note:
         if (m_insidePre) {
            m_t << "<formalpara><title>" << theTranslator->trNote() << ": </title>" << endl;
         } else {
            m_t << "<formalpara><title>" << convertToXML(theTranslator->trNote()) << ": </title>" << endl;
         }
         break;
      case DocSimpleSect::Warning:
         if (m_insidePre) {
            m_t << "<formalpara><title>" << theTranslator->trWarning() << ": </title>" << endl;
         } else {
            m_t << "<formalpara><title>" << convertToXML(theTranslator->trWarning()) << ": </title>" << endl;
         }
         break;
      case DocSimpleSect::Pre:
         if (m_insidePre) {
            m_t << "<formalpara><title>" << theTranslator->trPrecondition() << ": </title>" << endl;
         } else {
            m_t << "<formalpara><title>" << convertToXML(theTranslator->trPrecondition()) << ": </title>" << endl;
         }
         break;
      case DocSimpleSect::Post:
         if (m_insidePre) {
            m_t << "<formalpara><title>" << theTranslator->trPostcondition() << ": </title>" << endl;
         } else {
            m_t << "<formalpara><title>" << convertToXML(theTranslator->trPostcondition()) << ": </title>" << endl;
         }
         break;
      case DocSimpleSect::Copyright:
         if (m_insidePre) {
            m_t << "<formalpara><title>" << theTranslator->trCopyright() << ": </title>" << endl;
         } else {
            m_t << "<formalpara><title>" << convertToXML(theTranslator->trCopyright()) << ": </title>" << endl;
         }
         break;
      case DocSimpleSect::Invar:
         if (m_insidePre) {
            m_t << "<formalpara><title>" << theTranslator->trInvariant() << ": </title>" << endl;
         } else {
            m_t << "<formalpara><title>" << convertToXML(theTranslator->trInvariant()) << ": </title>" << endl;
         }
         break;
      case DocSimpleSect::Remark:
         if (m_insidePre) {
            m_t << "<formalpara><title>" << theTranslator->trRemarks() << ": </title>" << endl;
         } else {
            m_t << "<formalpara><title>" << convertToXML(theTranslator->trRemarks()) << ": </title>" << endl;
         }
         break;
      case DocSimpleSect::Attention:
         if (m_insidePre) {
            m_t << "<formalpara><title>" << theTranslator->trAttention() << ": </title>" << endl;
         } else {
            m_t << "<formalpara><title>" << convertToXML(theTranslator->trAttention()) << ": </title>" << endl;
         }
         break;
      case DocSimpleSect::User:
         m_t << "<formalpara><title></title>" << endl;
         break;
      case DocSimpleSect::Rcs:
         m_t << "<formalpara><title></title>" << endl;
         break;
      case DocSimpleSect::Unknown:
         m_t << "<formalpara><title></title>" << endl;
         break;
   }
}

void DocbookDocVisitor::visitPost(DocSimpleSect *)
{
   if (m_hide) {
      return;
   }
   m_t << "</formalpara>" << endl;
}

void DocbookDocVisitor::visitPre(DocTitle *)
{
   if (m_hide) {
      return;
   }
   m_t << "<title>";
}

void DocbookDocVisitor::visitPost(DocTitle *)
{
   if (m_hide) {
      return;
   }
   m_t << "</title>";
}

void DocbookDocVisitor::visitPre(DocSimpleList *)
{
   if (m_hide) {
      return;
   }
   m_t << "<itemizedlist>\n";
}

void DocbookDocVisitor::visitPost(DocSimpleList *)
{
   if (m_hide) {
      return;
   }
   m_t << "</itemizedlist>\n";
}

void DocbookDocVisitor::visitPre(DocSimpleListItem *)
{
   if (m_hide) {
      return;
   }
   m_t << "<listitem>";
}

void DocbookDocVisitor::visitPost(DocSimpleListItem *)
{
   if (m_hide) {
      return;
   }
   m_t << "</listitem>\n";
}

void DocbookDocVisitor::visitPre(DocSection *s)
{
   if (m_hide) {
      return;
   }
   m_t << "<section xml:id=\"" << s->file();
   if (!s->anchor().isEmpty()) {
      m_t << "_1" << s->anchor();
   }
   m_t << "\">" << endl;
   m_t << "<title>";
   filter(s->title());
   m_t << "</title>" << endl;
}

void DocbookDocVisitor::visitPost(DocSection *)
{
   m_t << "</section>\n";
}

void DocbookDocVisitor::visitPre(DocHtmlList *s)
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

void DocbookDocVisitor::visitPost(DocHtmlList *s)
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

void DocbookDocVisitor::visitPre(DocHtmlListItem *)
{
   if (m_hide) {
      return;
   }
   m_t << "<listitem>\n";
}

void DocbookDocVisitor::visitPost(DocHtmlListItem *)
{
   if (m_hide) {
      return;
   }
   m_t << "</listitem>\n";
}

void DocbookDocVisitor::visitPre(DocHtmlDescList *)
{
   if (m_hide) {
      return;
   }
   m_t << "<variablelist>\n";
}

void DocbookDocVisitor::visitPost(DocHtmlDescList *)
{
   if (m_hide) {
      return;
   }
   m_t << "</variablelist>\n";
}

void DocbookDocVisitor::visitPre(DocHtmlDescTitle *)
{
   if (m_hide) {
      return;
   }
   m_t << "<varlistentry><term>";
}

void DocbookDocVisitor::visitPost(DocHtmlDescTitle *)
{
   if (m_hide) {
      return;
   }
   m_t << "</term>\n";
}

void DocbookDocVisitor::visitPre(DocHtmlDescData *)
{
   if (m_hide) {
      return;
   }
   m_t << "<listitem>";
}

void DocbookDocVisitor::visitPost(DocHtmlDescData *)
{
   if (m_hide) {
      return;
   }
   m_t << "</listitem></varlistentry>\n";
}

void DocbookDocVisitor::visitPre(DocHtmlTable *t)
{
   if (m_hide) {
      return;
   }
   m_t << "<table frame=\"all\">" << endl;
   m_t << "    <title></title>" << endl;
   m_t << "    <tgroup cols=\"" << t->numColumns() << "\" align=\"left\" colsep=\"1\" rowsep=\"1\">" << endl;
   m_t << "    <tbody>" << endl;
}

void DocbookDocVisitor::visitPost(DocHtmlTable *)
{
   if (m_hide) {
      return;
   }
   m_t << "    </tbody>" << endl;
   m_t << "    </tgroup>" << endl;
   m_t << "</table>" << endl;
}

void DocbookDocVisitor::visitPre(DocHtmlRow *)
{
   if (m_hide) {
      return;
   }
   m_t << "<row>\n";
}

void DocbookDocVisitor::visitPost(DocHtmlRow *)
{
   if (m_hide) {
      return;
   }
   m_t << "</row>\n";
}

void DocbookDocVisitor::visitPre(DocHtmlCell *)
{
   if (m_hide) {
      return;
   }
   m_t << "<entry>";
}

void DocbookDocVisitor::visitPost(DocHtmlCell *)
{
   if (m_hide) {
      return;
   }
   m_t << "</entry>";
}

void DocbookDocVisitor::visitPre(DocHtmlCaption *)
{
   if (m_hide) {
      return;
   }
   m_t << "<caption>";
}

void DocbookDocVisitor::visitPost(DocHtmlCaption *)
{
   if (m_hide) {
      return;
   }
   m_t << "</caption>\n";
}

void DocbookDocVisitor::visitPre(DocInternal *)
{
   if (m_hide) {
      return;
   }
   // TODO: to be implemented
}

void DocbookDocVisitor::visitPost(DocInternal *)
{
   if (m_hide) {
      return;
   }
   // TODO: to be implemented
}

void DocbookDocVisitor::visitPre(DocHRef *href)
{
   if (m_hide) {
      return;
   }
   m_t << "<link xlink:href=\"" << href->url() << "\">";
}

void DocbookDocVisitor::visitPost(DocHRef *)
{
   if (m_hide) {
      return;
   }
   m_t << "</link>";
}

void DocbookDocVisitor::visitPre(DocHtmlHeader *)
{
   if (m_hide) {
      return;
   }
   m_t << "<formalpara><title>";
}

void DocbookDocVisitor::visitPost(DocHtmlHeader *)
{
   if (m_hide) {
      return;
   }
   m_t << "</title></formalpara>\n";
}

void DocbookDocVisitor::visitPre(DocImage *img)
{
   if (img->type() == DocImage::DocBook) {
      if (m_hide) {
         return;
      }

      m_t << endl;

      QString baseName = img->name();         
      int i;

      if ((i = baseName.lastIndexOf('/')) != -1 || (i = baseName.lastIndexOf('\\')) != -1) {
         baseName = baseName.right(baseName.length() - i - 1);
      }

      visitPreStart(m_t, img->hasCaption(), baseName, img->width(), img->height());

   } else {
      pushEnabled();
      m_hide = true;
   }
}

void DocbookDocVisitor::visitPost(DocImage *img)
{
   if (img->type() == DocImage::DocBook) {

      if (m_hide) {
         return;
      }

      static const QString docbookOutDir = Config::getString("docbook-output");

      visitPostEnd(m_t, img->hasCaption());

      QString baseName = img->name();
      int i;

      if ((i = baseName.lastIndexOf('/')) != -1 || (i = baseName.lastIndexOf('\\')) != -1) {
         baseName = baseName.right(baseName.length() - i - 1);
      }
   
      // copy the image to the output dir
      QString m_file;

      bool ambig;
      QSharedPointer<FileDef> fd = findFileDef(Doxy_Globals::imageNameDict, baseName, ambig);

      if (fd) {
         m_file = fd->getFilePath();
      }

      QFile inImage(m_file);
      QString outputFile = docbookOutDir + "/" + baseName;

      if (inImage.open(QIODevice::ReadOnly)) {

         QFile::remove(outputFile);
         bool copyOk = inImage.copy(outputFile);

         if (! copyOk) { 
            err("Unable to open image file for writing %s, error: %d\n", csPrintable(outputFile), inImage.error());
         }        

      } else {
         err("Unable to open image file for reading %s, error: %d\n", csPrintable(m_file), inImage.error());                
      }

   } else {
      popEnabled();

   }
}

void DocbookDocVisitor::visitPre(DocDotFile *df)
{
   if (m_hide) {
      return;
   }
   startDotFile(df->file(), df->width(), df->height(), df->hasCaption());
}

void DocbookDocVisitor::visitPost(DocDotFile *df)
{
   if (m_hide) {
      return;
   }
   endDotFile(df->hasCaption());
}

void DocbookDocVisitor::visitPre(DocMscFile *df)
{
   if (m_hide) {
      return;
   }
   startMscFile(df->file(), df->width(), df->height(), df->hasCaption());
}

void DocbookDocVisitor::visitPost(DocMscFile *df)
{
   if (m_hide) {
      return;
   }
   endMscFile(df->hasCaption());
}
void DocbookDocVisitor::visitPre(DocDiaFile *df)
{
   if (m_hide) {
      return;
   }
   startDiaFile(df->file(), df->width(), df->height(), df->hasCaption());
}

void DocbookDocVisitor::visitPost(DocDiaFile *df)
{
   if (m_hide) {
      return;
   }
   endDiaFile(df->hasCaption());
}

void DocbookDocVisitor::visitPre(DocLink *lnk)
{
   if (m_hide) {
      return;
   }
   startLink(lnk->file(), lnk->anchor());
}

void DocbookDocVisitor::visitPost(DocLink *)
{
   if (m_hide) {
      return;
   }
   endLink();
}

void DocbookDocVisitor::visitPre(DocRef *ref)
{
   if (m_hide) {
      return;
   }

   if (! ref->file().isEmpty()) {
      startLink(ref->file(), ref->anchor());
   }
   if (! ref->hasLinkText()) {
      filter(ref->targetTitle());
   }
}

void DocbookDocVisitor::visitPost(DocRef *ref)
{
   if (m_hide) {
      return;
   }
   if (!ref->file().isEmpty()) {
      endLink();
   }
}

void DocbookDocVisitor::visitPre(DocSecRefItem *ref)
{
   if (m_hide) {
      return;
   }
   m_t << "<tocitem id=\"" << ref->file() << "_1" << ref->anchor() << "\">";
}

void DocbookDocVisitor::visitPost(DocSecRefItem *)
{
   if (m_hide) {
      return;
   }
   m_t << "</tocitem>" << endl;
}

void DocbookDocVisitor::visitPre(DocSecRefList *)
{
   if (m_hide) {
      return;
   }
   m_t << "<toclist>" << endl;
}

void DocbookDocVisitor::visitPost(DocSecRefList *)
{
   if (m_hide) {
      return;
   }
   m_t << "</toclist>" << endl;
}

void DocbookDocVisitor::visitPre(DocParamSect *s)
{
   if (m_hide) {
      return;
   }
   m_t <<  endl;
   m_t << "                <formalpara>" << endl;
   m_t << "                    <title/>" << endl;
   m_t << "                    <table frame=\"all\">" << endl;
   m_t << "                        <title>";
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

   m_t << "                        </title>" << endl;
   m_t << "                        <tgroup cols=\"2\" align=\"left\" colsep=\"1\" rowsep=\"1\">" << endl;
   m_t << "                        <colspec colwidth=\"1*\"/>" << endl;
   m_t << "                        <colspec colwidth=\"4*\"/>" << endl;
   m_t << "                        <tbody>" << endl;
}

void DocbookDocVisitor::visitPost(DocParamSect *)
{
   if (m_hide) {
      return;
   }
   m_t << "                        </tbody>" << endl;
   m_t << "                        </tgroup>" << endl;
   m_t << "                    </table>" << endl;
   m_t << "                </formalpara>" << endl;
   m_t << "                ";
}

void DocbookDocVisitor::visitPre(DocParamList *pl)
{
   if (m_hide) {
      return;
   }
 
   m_t << "                            <row>" << endl;

   if (pl->parameters().isEmpty()) {
      m_t << "                                <entry></entry>" << endl;

   } else {
      m_t << "                                <entry>";
      int cnt = 0;

      for (auto param : pl->parameters()) {
         if (cnt) {
            m_t << ", ";
         }

         if (param->kind() == DocNode::Kind_Word) {
            visit((DocWord *)param);

         } else if (param->kind() == DocNode::Kind_LinkedWord) {
            visit((DocLinkedWord *)param);

         }

         cnt++;
      }
      m_t << "</entry>" << endl;
   }

   m_t << "                                <entry>";
}

void DocbookDocVisitor::visitPost(DocParamList *)
{
   if (m_hide) {
      return;
   }
   m_t << "</entry>" << endl;
   m_t << "                            </row>" << endl;
}

void DocbookDocVisitor::visitPre(DocXRefItem *x)
{
   if (m_hide) {
      return;
   }
   if (x->title().isEmpty()) {
      return;
   }
   m_t << "<para><link linkend=\"";
   m_t << x->file() << "_1" << x->anchor();
   m_t << "\">";
   filter(x->title());
   m_t << "</link>";
   m_t << " ";
}

void DocbookDocVisitor::visitPost(DocXRefItem *x)
{
   if (m_hide) {
      return;
   }
   if (x->title().isEmpty()) {
      return;
   }
   m_t << "</para>";
}

void DocbookDocVisitor::visitPre(DocInternalRef *ref)
{
   if (m_hide) {
      return;
   }
   startLink(ref->file(), ref->anchor());
}

void DocbookDocVisitor::visitPost(DocInternalRef *)
{
   if (m_hide) {
      return;
   }
   endLink();
   m_t << " ";
}

void DocbookDocVisitor::visitPre(DocCopy *)
{
   if (m_hide) {
      return;
   }
   // TODO: to be implemented
}


void DocbookDocVisitor::visitPost(DocCopy *)
{
   if (m_hide) {
      return;
   }
   // TODO: to be implemented
}


void DocbookDocVisitor::visitPre(DocText *)
{
   // TODO: to be implemented
}


void DocbookDocVisitor::visitPost(DocText *)
{
   // TODO: to be implemented
}


void DocbookDocVisitor::visitPre(DocHtmlBlockQuote *)
{
   if (m_hide) {
      return;
   }
   m_t << "<blockquote>";
}

void DocbookDocVisitor::visitPost(DocHtmlBlockQuote *)
{
   if (m_hide) {
      return;
   }
   m_t << "</blockquote>";
}

void DocbookDocVisitor::visitPre(DocParBlock *)
{
}

void DocbookDocVisitor::visitPost(DocParBlock *)
{
}

void DocbookDocVisitor::filter(const QString &str)
{
   m_t << convertToXML(str);
}

void DocbookDocVisitor::startLink(const QString &file, const QString &anchor)
{
   m_t << "<link linkend=\"" << file;
   if (!anchor.isEmpty()) {
      m_t << "_1" << anchor;
   }
   m_t << "\">";
}

void DocbookDocVisitor::endLink()
{
   m_t << "</link>";
}

void DocbookDocVisitor::pushEnabled()
{
   m_enabled.push(m_hide);
}

void DocbookDocVisitor::popEnabled()
{
   bool v = m_enabled.pop();   
   m_hide = v;   
}

void DocbookDocVisitor::writeMscFile(const QString &baseName, DocVerbatim *s)
{
   QString shortName = baseName;
   int i;

   if ((i = shortName.lastIndexOf('/')) != -1) {
      shortName = shortName.right(shortName.length() - i - 1);
   }

   static const QString docbookOutDir = Config::getString("docbook-output");

   writeMscGraphFromFile(baseName + ".msc", docbookOutDir, shortName, MSC_BITMAP);

   visitPreStart(m_t, s->hasCaption(), shortName, s->width(),s->height());
   visitCaption(this, s->children());
   visitPostEnd(m_t,  s->hasCaption()); 
}

void DocbookDocVisitor::writePlantUMLFile(const QString &baseName, DocVerbatim *s)
{
   QString shortName = baseName;
   int i;

   if ((i = shortName.lastIndexOf('/')) != -1) {
      shortName = shortName.right(shortName.length() - i - 1);
   }

   static const QString docbookOutDir = Config::getString("docbook-output");

   generatePlantUMLOutput(baseName, docbookOutDir, PUML_BITMAP);

   visitPreStart(m_t, s->hasCaption(), shortName, s->width(),s->height());
   visitCaption(this, s->children());
   visitPostEnd(m_t,  s->hasCaption());
}

void DocbookDocVisitor::startMscFile(const QString &fileName, const QString &width, const QString &height, 
                  bool hasCaption)
{
   static const QString docbookOutDir = Config::getString("docbook-output");

   QString baseName = fileName;
   int i;

   if ((i = baseName.lastIndexOf('/')) != -1) {
      baseName = baseName.right(baseName.length() - i - 1);
   }

   if ((i = baseName.indexOf('.')) != -1) {
      baseName = baseName.left(i);
   }

   baseName.prepend("msc_");  

   writeMscGraphFromFile(fileName, docbookOutDir, baseName, MSC_BITMAP);
   m_t << "<para>" << endl;
   visitPreStart(m_t, hasCaption, baseName + ".png", width, height);
}

void DocbookDocVisitor::endMscFile(bool hasCaption)
{
   if (m_hide) {
      return;
   }

   visitPostEnd(m_t, hasCaption);
   m_t << "</para>" << endl;
}

void DocbookDocVisitor::writeDiaFile(const QString &baseName, DocVerbatim *s)
{
   static const QString docbookOutDir = Config::getString("docbook-output");

   QString shortName = baseName;
   int i;

   if ((i = shortName.lastIndexOf('/')) != -1) {
      shortName = shortName.right(shortName.length() - i - 1);
   }

   writeDiaGraphFromFile(baseName + ".dia", docbookOutDir, shortName, DIA_BITMAP);
   visitPreStart(m_t, s->hasCaption(), shortName, s->width(), s->height());
   visitCaption(this, s->children());
   visitPostEnd(m_t,  s->hasCaption());
}

void DocbookDocVisitor::startDiaFile(const QString &fileName, const QString &width, const QString &height,
                  bool hasCaption )
{
   static const QString docbookOutDir = Config::getString("docbook-output");

   QString baseName = fileName;
   int i;

   if ((i = baseName.lastIndexOf('/')) != -1) {
      baseName = baseName.right(baseName.length() - i - 1);
   }

   if ((i = baseName.indexOf('.')) != -1) {
      baseName = baseName.left(i);
   }

   baseName.prepend("dia_");
 
   writeDiaGraphFromFile(fileName, docbookOutDir, baseName, DIA_BITMAP);
   m_t << "<para>" << endl;
   visitPreStart(m_t, hasCaption, baseName + ".png", width, height);
}

void DocbookDocVisitor::endDiaFile(bool hasCaption)
{
   if (m_hide) {
      return;
   }

   visitPostEnd(m_t, hasCaption);
   m_t << "endl";  
}

void DocbookDocVisitor::writeDotFile(const QString &baseName, DocVerbatim *s)
{
   static const QString docbookOutDir = Config::getString("docbook-output");

   QString shortName = baseName;
   int i;

   if ((i = shortName.lastIndexOf('/')) != -1) {
      shortName = shortName.right(shortName.length() - i - 1);
   }
  
   writeDotGraphFromFile(baseName + ".dot", docbookOutDir, shortName, GOF_BITMAP);

   visitPreStart(m_t, s->hasCaption(), baseName + ".dot", s->width(), s->height());
   visitCaption(this, s->children());
   visitPostEnd(m_t,  s->hasCaption());  
}

void DocbookDocVisitor::startDotFile(const QString &fileName, const QString &width, const QString &height, 
                  bool hasCaption)
{
   static const QString docbookOutDir = Config::getString("docbook-output");
   static const QString imageExt      = Config::getEnum("dot-image-extension");

   QString baseName = fileName;
   int i;

   if ((i = baseName.lastIndexOf('/')) != -1) {
      baseName = baseName.right(baseName.length() - i - 1);
   }

   if ((i = baseName.indexOf('.')) != -1) {
      baseName = baseName.left(i);
   }

   baseName.prepend("dot_");
 
   writeDotGraphFromFile(fileName, docbookOutDir, baseName, GOF_BITMAP);
   m_t << "<para>" << endl;
   visitPreStart(m_t, hasCaption, baseName + "." + imageExt, width, height);  
}

void DocbookDocVisitor::endDotFile(bool hasCaption)
{
   if (m_hide) {
      return;
   }

   m_t << endl;
   visitPostEnd(m_t, hasCaption);
   m_t << "</para>" << endl;
}
