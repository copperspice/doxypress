/*************************************************************************
 *
 * Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim
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

#include <stdlib.h>

#include <latexgen.h>

#include <cite.h>
#include <config.h>
#include <diagram.h>
#include <docparser.h>
#include <dot.h>
#include <doxy_build_info.h>
#include <doxy_globals.h>
#include <language.h>
#include <latexdocvisitor.h>
#include <message.h>
#include <resourcemgr.h>
#include <util.h>

LatexCodeGenerator::LatexCodeGenerator(QTextStream &t, const QString &relPath, const QString &sourceFileName)
  : m_relPath(relPath), m_sourceFileName(sourceFileName), m_col(0)
{
   m_prettyCode = Config::getBool("latex-source-code");
   setTextStream(t);
}

LatexCodeGenerator::LatexCodeGenerator() : m_col(0)
{
   m_prettyCode = Config::getBool("latex-source-code");
}

void LatexCodeGenerator::setTextStream(QTextStream &t)
{
   m_streamSet = (t.device() != nullptr);
   m_t.setDevice(t.device());
}

void LatexCodeGenerator::setRelativePath(const QString &path)
{
   m_relPath = path;
}

void LatexCodeGenerator::setSourceFileName(const QString &name)
{
   m_sourceFileName = name;
}

void LatexCodeGenerator::codify(const QString &str)
{
   if (str.isEmpty()) {
      return;
   }

   QString result;

   const QChar *p = str.constData();
   QChar c;

   static int tabSize   = Config::getInt("tab-size");
   const int maxLineLen = 108;

   while ((c = *p) != 0) {

      switch(c.unicode())  {
         case 0x0c:
            // remove ^L
            p++;
            break;

         case '\t':  {

            int spacesToNextTabStop = tabSize - (m_col % tabSize);
            m_t << QString(spacesToNextTabStop, QChar(' '));
            m_col += spacesToNextTabStop;

            p++;

            break;
         }

         case '\n':
            m_t << '\n';
            m_col = 0;
            p++;

            break;

         default:
            // gather characters until we find whitespace or are at the end of a line
            result += c;
            p++;

            if (m_col >= maxLineLen) {
               // force line break

               m_t << "\n      ";
               m_col = 0;

            } else  {
               // copy more characters

               while (m_col < maxLineLen && ((c = *p) != 0) && c != 0x0c && c != '\t' && c != '\n' && c != ' ')  {
                  result += c;
                  p++;
               }

               if (m_col >= maxLineLen) {
                  // force line break

                  m_t << "\n      ";
                  m_col=0;
               }
            }

            filterLatexString(m_t, result, false, true);
            break;
    }
  }

}

void LatexCodeGenerator::writeCodeLink(const QString &ref, const QString &f, const QString &anchor,
                  const QString &name, const QString &)
{
   static bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static bool usePDFLatex   = Config::getBool("latex-pdf");

   int len = name.length();

   if (m_col + len > 80) {
      m_t << "\n      ";
      m_col = 0;
   }

   if (ref.isEmpty() && usePDFLatex && pdfHyperlinks) {
      m_t << "\\hyperlink{";

      if (! f.isEmpty()) {
         m_t << stripPath(f);
      }

      if (! f.isEmpty() && ! anchor.isEmpty()) {
         m_t << "_";
      }

      if (! anchor.isEmpty()) {
         m_t << anchor;
      }

      m_t << "}{";
      codify(name);
      m_t << "}";

   } else {
      m_t << name;
   }

   m_col += len;
}

void LatexCodeGenerator::writeLineNumber(const QString &ref, const QString &fileName, const QString &anchor, int len)
{
   static bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static bool usePDFLatex   = Config::getBool("latex-pdf");

   if (m_prettyCode) {
      QString lineNumber = QString("%1").arg(len, 5, 10, QChar('0'));

      if (! fileName.isEmpty() && ! m_sourceFileName.isEmpty()) {
         QString lineAnchor = "_l" + lineNumber;
         lineAnchor.prepend(m_sourceFileName);

         if (usePDFLatex && pdfHyperlinks) {
            m_t << "\\Hypertarget{" << stripPath(lineAnchor) << "}";
         }

         writeCodeLink(ref, fileName, anchor, lineNumber, 0);

      } else {
         codify(lineNumber);
      }

      m_t << " ";

   } else {
      m_t << len << " ";
   }
}

void LatexCodeGenerator::startCodeLine(bool)
{
   m_col = 0;
}

void LatexCodeGenerator::endCodeLine()
{
   codify("\n");
}

void LatexCodeGenerator::startFontClass(const QString &name)
{
   m_t << "\\textcolor{" << name << "}{";
}

void LatexCodeGenerator::endFontClass()
{
   m_t << "}";
}

// **
LatexGenerator::LatexGenerator() : OutputGenerator()
{
   m_dir        = Config::getString("latex-output");
   m_prettyCode = Config::getBool("latex-source-code");

   m_indent = 0;

   insideTabbing      = false;
   firstDescItem      = true;
   disableLinks       = false;
   templateMemberItem = false;
}

LatexGenerator::~LatexGenerator()
{
}

static void writeLatexMakefile()
{
   bool generateBib = ! Doxy_Globals::citeDict.isEmpty();

   QString dir = Config::getString("latex-output");
   QString fileName = dir + "/Makefile";

   QFile file(fileName);

   if (! file.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing\n", qPrintable(fileName));
      Doxy_Work::stopDoxyPress();
   }

   // inserted by KONNO Akihisa <konno@researchers.jp> 2002-03-05
   QString latex_command = Config::getString("latex-cmd-name");
   QString mkidx_command = Config::getString("make-index-cmd-name");

   // end insertion by KONNO Akihisa <konno@researchers.jp> 2002-03-05
   QTextStream t(&file);

   if (! Config::getBool("latex-pdf")) {
      // use plain old latex

      t << "all: refman.dvi" << endl
        << endl
        << "ps: refman.ps" << endl
        << endl
        << "pdf: refman.pdf" << endl
        << endl
        << "ps_2on1: refman_2on1.ps" << endl
        << endl
        << "pdf_2on1: refman_2on1.pdf" << endl
        << endl
        << "refman.ps: refman.dvi" << endl
        << "\tdvips -o refman.ps refman.dvi" << endl
        << endl;
      t << "refman.pdf: refman.ps" << endl;
      t << "\tps2pdf refman.ps refman.pdf" << endl << endl;
      t << "refman.dvi: clean refman.tex doxypress.sty" << endl
        << "\techo \"Running latex...\"" << endl
        << "\t" << latex_command << " refman.tex" << endl
        << "\techo \"Running makeindex...\"" << endl
        << "\t" << mkidx_command << " refman.idx" << endl;

      if (generateBib) {
         t << "\techo \"Running bibtex...\"" << endl;
         t << "\tbibtex refman" << endl;
         t << "\techo \"Rerunning latex....\"" << endl;
         t << "\t" << latex_command << " refman.tex" << endl;
      }

      t << "\techo \"Rerunning latex....\"" << endl
        << "\t" << latex_command << " refman.tex" << endl
        << "\tlatex_count=8 ; \\" << endl
        << "\twhile egrep -s 'Rerun (LaTeX|to get cross-references right)' refman.log && [ $$latex_count -gt 0 ] ;\\" << endl
        << "\t    do \\" << endl
        << "\t      echo \"Rerunning latex....\" ;\\" << endl
        << "\t      " << latex_command << " refman.tex ;\\" << endl
        << "\t      latex_count=`expr $$latex_count - 1` ;\\" << endl
        << "\t    done" << endl
        << "\t" << mkidx_command << " refman.idx" << endl
        << "\t" << latex_command << " refman.tex" << endl << endl
        << "refman_2on1.ps: refman.ps" << endl
        << "\tpsnup -2 refman.ps >refman_2on1.ps" << endl
        << endl
        << "refman_2on1.pdf: refman_2on1.ps" << endl
        << "\tps2pdf refman_2on1.ps refman_2on1.pdf" << endl;

   } else { // use pdflatex for higher quality output
      t << "all: refman.pdf" << endl << endl
        << "pdf: refman.pdf" << endl << endl;
      t << "refman.pdf: clean refman.tex" << endl;
      t << "\tpdflatex refman" << endl;
      t << "\t" << mkidx_command << " refman.idx" << endl;

      if (generateBib) {
         t << "\tbibtex refman" << endl;
         t << "\tpdflatex refman" << endl;
      }

      t << "\tpdflatex refman" << endl
        << "\tlatex_count=8 ; \\" << endl
        << "\twhile egrep -s 'Rerun (LaTeX|to get cross-references right)' refman.log && [ $$latex_count -gt 0 ] ;\\" << endl
        << "\t    do \\" << endl
        << "\t      echo \"Rerunning latex....\" ;\\" << endl
        << "\t      pdflatex refman ;\\" << endl
        << "\t      latex_count=`expr $$latex_count - 1` ;\\" << endl
        << "\t    done" << endl
        << "\t" << mkidx_command << " refman.idx" << endl
        << "\tpdflatex refman" << endl << endl;
   }

   t << endl
     << "clean:" << endl
     << "\trm -f "
     << "*.ps *.dvi *.aux *.toc *.idx *.ind *.ilg *.log *.out *.brf *.blg *.bbl refman.pdf" << endl;
}

static void writeMakeBat()
{

#if defined(Q_OS_WIN)

   QString dir = Config::getString("latex-output");
   QString fileName = dir + "/make.bat";

   QString latex_command = Config::getString("latex-cmd-name");
   QString mkidx_command = Config::getString("make-index-cmd-name");

   QFile file(fileName);

   bool generateBib = ! Doxy_Globals::citeDict.isEmpty();

   if (! file.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing\n", csPrintable(fileName));
      Doxy_Work::stopDoxyPress();
   }

   QTextStream t(&file);
   t << "set Dir_Old=%cd%\n";
   t << "cd /D %~dp0\n\n";
   t << "del /s /f *.ps *.dvi *.aux *.toc *.idx *.ind *.ilg *.log *.out *.brf *.blg *.bbl refman.pdf\n\n";

   if (! Config::getBool("latex-pdf")) {
      // use plain old latex

      t << latex_command << " refman.tex\n";
      t << "echo ----\n";
      t << mkidx_command << " refman.idx\n";

      if (generateBib) {
         t << "bibtex refman\n";
         t << "echo ----\n";
         t << latex_command << " refman.tex\n";
      }

      t << "setlocal enabledelayedexpansion\n";
      t << "set count=8\n";
      t << ":repeat\n";
      t << "set content=X\n";
      t << "for /F \"tokens=*\" %%T in ( 'findstr /C:\"Rerun LaTeX\" refman.log' ) do set content=\"%%~T\"\n";

      t << "if !content! == X for /F \"tokens=*\" %%T in ( 'findstr /C:\"Rerun to get cross-references right\" refman.log' )"
                  " do set content=\"%%~T\"\n";

      t << "if !content! == X goto :skip\n";
      t << "set /a count-=1\n";
      t << "if !count! EQU 0 goto :skip\n\n";
      t << "echo ----\n";
      t << latex_command << " refman.tex\n";
      t << "goto :repeat\n";
      t << ":skip\n";
      t << "endlocal\n";
      t << mkidx_command << " refman.idx\n";
      t << latex_command << " refman.tex\n";
      t << "dvips -o refman.ps refman.dvi\n";
      t << "gswin32c -q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite "
        "-sOutputFile=refman.pdf -c save pop -f refman.ps\n";

   } else {
      // use pdflatex

      t << "pdflatex refman\n";
      t << "echo ----\n";
      t << mkidx_command << " refman.idx\n";

      if (generateBib) {
         t << "bibtex refman" << endl;
         t << "pdflatex refman" << endl;
      }

      t << "echo ----\n";
      t << "pdflatex refman\n\n";
      t << "setlocal enabledelayedexpansion\n";
      t << "set count=8\n";
      t << ":repeat\n";
      t << "set content=X\n";
      t << "for /F \"tokens=*\" %%T in ( 'findstr /C:\"Rerun LaTeX\" refman.log' ) do set content=\"%%~T\"\n";

      t << "if !content! == X for /F \"tokens=*\" %%T in ( 'findstr /C:\"Rerun to get cross-references right\" refman.log' )"
               " do set content=\"%%~T\"\n";

      t << "if !content! == X goto :skip\n";
      t << "set /a count-=1\n";
      t << "if !count! EQU 0 goto :skip\n\n";
      t << "echo ----\n";
      t << "pdflatex refman\n";
      t << "goto :repeat\n";
      t << ":skip\n";
      t << "endlocal\n";
      t << mkidx_command << " refman.idx\n";
      t << "pdflatex refman\n";
      t << "cd /D %Dir_Old%\n";
      t << "set Dir_Old=\n";
   }
#endif

}

void LatexGenerator::init()
{
   QString dir = Config::getString("latex-output");
   QDir d(dir);

   if (! d.exists() && ! d.mkdir(dir)) {
      err("Unable to create output directory %s\n", csPrintable(dir));
      Doxy_Work::stopDoxyPress();
   }

   writeLatexMakefile();
   writeMakeBat();

   createSubDirs(d);
}

static void writeDefaultHeaderPart1(QTextStream &t_stream)
{
   static const QString projectName         = Config::getString("project-name");
   static const QString outputLanguage      = Config::getEnum("output-language").toLower();
   static const QStringList latexStyleSheet = Config::getList("latex-stylesheets");
   static const bool latexBatchMode         = Config::getBool("latex-batch-mode");
   static const bool latexCompact           = Config::getBool("latex-compact");
   static const bool timeStamp              = Config::getBool("latex-timestamp");
   static const bool pdfHyperlinks          = Config::getBool("latex-hyper-pdf");
   static const bool usePDFLatex            = Config::getBool("latex-pdf");
   static const QString paperName           = Config::getEnum("latex-paper-type");

   // Handle batch mode
   if (latexBatchMode) {
      t_stream << "\\batchmode\n";
   }

   // Set document class depending on configuration
   QByteArray documentClass;

   if (latexCompact) {
      documentClass = "article";
   } else {
      documentClass = "book";
   }

   t_stream << "\\documentclass[twoside]{" << documentClass << "}\n"
     "\n";

   // Load required packages
   t_stream << "% Packages required by DoxyPress\n"
      "\\usepackage{fixltx2e}\n" // for \textsubscript
      "\\usepackage{calc}\n"
      "\\usepackage{doxypress}\n"
      "\\usepackage[export]{adjustbox} % also loads graphicx\n";

   for (auto fileName : latexStyleSheet) {

      if (! fileName.isEmpty()) {
         QFileInfo fi(fileName);

         if (fi.exists()) {
            if (checkExtension(fi.fileName(), Doxy_Globals::latexStyleExtension)) {
               // strip the extension, it will be added by the usepackage in the tex conversion process
               t_stream << "\\usepackage{" << stripExtensionGeneral(fi.fileName(),
                     Doxy_Globals::latexStyleExtension) << "}\n";

            } else {
               t_stream << "\\usepackage{" << fi.fileName() << "}\n";
            }
         }
      }
   }

   t_stream << "\\usepackage{graphicx}\n"
                "\\usepackage[utf8]{inputenc}\n"
                "\\usepackage{makeidx}\n"
                "\\usepackage{multicol}\n"
                "\\usepackage{multirow}\n"
                "\\PassOptionsToPackage{warn}{textcomp}\n"
                "\\usepackage{textcomp}\n"
                "\\usepackage[nointegrals]{wasysym}\n"
                "\\usepackage[table]{xcolor}\n"
                "\n";

   // Language support
   QString languageSupport = theTranslator->latexLanguageSupportCommand();

   if (! languageSupport.isEmpty()) {
      t_stream << "% NLS support packages\n"
               << languageSupport
               << "\n";
   }

   // Define default fonts
   t_stream << "% Font selection\n"
     "\\usepackage[T1]{fontenc}\n"
     "\\usepackage[scaled=.90]{helvet}\n"
     "\\usepackage{courier}\n"
     "\\usepackage{amssymb}\n"
     "\\usepackage{sectsty}\n"
     "\\renewcommand{\\familydefault}{\\sfdefault}\n"
     "\\allsectionsfont{%\n"
     "  \\fontseries{bc}\\selectfont%\n"
     "  \\color{darkgray}%\n"
     "}\n"
     "\\renewcommand{\\DoxyLabelFont}{%\n"
     "  \\fontseries{bc}\\selectfont%\n"
     "  \\color{darkgray}%\n"
     "}\n"
     "\\newcommand{\\+}{\\discretionary{\\mbox{\\scriptsize$\\hookleftarrow$}}{}{}}\n"
     "\n";

   // Define page & text layout

   t_stream << "% Page & text layout\n"
     "\\usepackage{geometry}\n"
     "\\geometry{%\n"
     "  " << paperName << "paper,%\n"
     "  top=2.5cm,%\n"
     "  bottom=2.5cm,%\n"
     "  left=2.5cm,%\n"
     "  right=2.5cm%\n"
     "}\n";

   // Allow a bit of overflow to go unnoticed by other means
   t_stream << "\\tolerance=750\n"
     "\\hfuzz=15pt\n"
     "\\hbadness=750\n"
     "\\setlength{\\emergencystretch}{15pt}\n"
     "\\setlength{\\parindent}{0cm}\n"
     "\\setlength{\\parskip}{3ex plus 2ex minus 2ex}\n";

   // Redefine paragraph/subparagraph environments, using sectsty fonts
   t_stream << "\\makeatletter\n"
     "\\renewcommand{\\paragraph}{%\n"
     "  \\@startsection{paragraph}{4}{0ex}{-1.0ex}{1.0ex}{%\n"
     "    \\normalfont\\normalsize\\bfseries\\SS@parafont%\n"
     "  }%\n"
     "}\n"
     "\\renewcommand{\\subparagraph}{%\n"
     "  \\@startsection{subparagraph}{5}{0ex}{-1.0ex}{1.0ex}{%\n"
     "    \\normalfont\\normalsize\\bfseries\\SS@subparafont%\n"
     "  }%\n"
     "}\n"
     "\\makeatother\n"
     "\n";

   // Headers & footers
   QString genString;
   QString generatedBy;
   QTextStream tg(&genString);

   if (timeStamp) {
      generatedBy = theTranslator->trGeneratedAt(dateToString(true), projectName);

   } else {
      generatedBy = theTranslator->trGeneratedBy();
   }

   filterLatexString(tg, generatedBy, false, false, false);

   t_stream << "% Headers & footers\n"
     "\\usepackage{fancyhdr}\n"
     "\\pagestyle{fancyplain}\n"
     "\\fancyhead[LE]{\\fancyplain{}{\\bfseries\\thepage}}\n"
     "\\fancyhead[CE]{\\fancyplain{}{}}\n"
     "\\fancyhead[RE]{\\fancyplain{}{\\bfseries\\leftmark}}\n"
     "\\fancyhead[LO]{\\fancyplain{}{\\bfseries\\rightmark}}\n"
     "\\fancyhead[CO]{\\fancyplain{}{}}\n"
     "\\fancyhead[RO]{\\fancyplain{}{\\bfseries\\thepage}}\n"
     "\\fancyfoot[LE]{\\fancyplain{}{}}\n"
     "\\fancyfoot[CE]{\\fancyplain{}{}}\n"
     "\\fancyfoot[RE]{\\fancyplain{}{\\bfseries\\scriptsize " << genString << " DoxyPress }}\n"
     "\\fancyfoot[LO]{\\fancyplain{}{\\bfseries\\scriptsize " << genString << " DoxyPress }}\n"
     "\\fancyfoot[CO]{\\fancyplain{}{}}\n"
     "\\fancyfoot[RO]{\\fancyplain{}{}}\n"
     "\\renewcommand{\\footrulewidth}{0.4pt}\n";

   if (! latexCompact) {
      t_stream << "\\renewcommand{\\chaptermark}[1]{%\n"
        "  \\markboth{#1}{}%\n"
        "}\n";
   }

   t_stream << "\\renewcommand{\\sectionmark}[1]{%\n"
     "  \\markright{\\thesection\\ #1}%\n"
     "}\n"
     "\n";

   // ToC, LoF, LoT, bibliography, and index
   t_stream << "% Indices & bibliography\n"
     "\\usepackage{natbib}\n"
     "\\usepackage[titles]{tocloft}\n"
     "\\setcounter{tocdepth}{3}\n"
     "\\setcounter{secnumdepth}{5}\n"
     "\\makeindex\n"
     "\n";

   writeExtraLatexPackages(t_stream);

   // Hyperlinks
   if (pdfHyperlinks) {
      t_stream << "% Hyperlinks (required, but should be loaded last)\n"
        "\\usepackage{ifpdf}\n"
        "\\ifpdf\n"
        "  \\usepackage[pdftex,pagebackref=true]{hyperref}\n"
        "\\else\n"
        "  \\usepackage[ps2pdf,pagebackref=true]{hyperref}\n"
        "\\fi\n"
        "\\hypersetup{%\n"
        "  colorlinks=true,%\n"
        "  linkcolor=blue,%\n"
        "  citecolor=blue,%\n"
        "  unicode%\n"
        "}\n"
        "\n";
   }

   // Custom commands used by the header
   t_stream << "% Custom commands\n"
     "\\newcommand{\\clearemptydoublepage}{%\n"
     "  \\newpage{\\pagestyle{empty}\\cleardoublepage}%\n"
     "}\n"
     "\n";

   // caption style definition
   t_stream << "\\usepackage{caption}\n"
   << "\\captionsetup{labelsep=space,justification=centering,font={bf},"
                  "singlelinecheck=off,skip=4pt,position=top}\n\n";

   // End of preamble, now comes the document contents
   t_stream << "%===== C O N T E N T S =====\n"
     "\n"
     "\\begin{document}\n";

  
   // set to the indicated language
   if (! outputLanguage.isEmpty() && outputLanguage != "english") {
      t_stream << "\\selectlanguage{" << outputLanguage << "}\n";
   }

   t_stream << "\n";

   // Front matter
   t_stream << "% Titlepage & ToC\n";
   
   if (pdfHyperlinks && usePDFLatex) {
      // To avoid duplicate page anchors due to reuse of same numbers for
      // the index (be it as roman numbers)
      t_stream << "\\hypersetup{pageanchor=false,\n"
//             << "             bookmarks=true,\n"            // not used, unclear why
               << "             bookmarksnumbered=true,\n"
               << "             pdfencoding=unicode\n"
               << "            }\n";
   }

   t_stream << "\\pagenumbering{alph}\n"
               "\\begin{titlepage}\n"
               "\\vspace*{7cm}\n"
               "\\begin{center}%\n"
               "{\\Large ";
}

static void writeDefaultHeaderPart2(QTextStream &t_stream)
{
   // part 2
   // Finalize project name

   t_stream << "}\\\\\n"
               "\\vspace*{1cm}\n"
               "{\\large ";
}

static void writeDefaultHeaderPart3(QTextStream &t_stream)
{
   // part 3
   static bool timeStamp = Config::getBool("latex-timestamp");

   // Finalize project number
   t_stream << " DoxyPress " << versionString << "}\\\\\n";

   if (timeStamp) {
     t_stream << "\\vspace*{0.5cm}\n"
                 "{\\small " << dateToString(true) << "}\\\\\n";
   }

   t_stream << "\\end{center}\n"
               "\\end{titlepage}\n";

   bool compactLatex = Config::getBool("latex-compact");
   if (!compactLatex) {
      t_stream << "\\clearemptydoublepage\n";
      t_stream << "\\pagenumbering{roman}\n";
   }

   // ToC
   t_stream << "\\tableofcontents\n";
   if (! compactLatex) {
      t_stream << "\\clearemptydoublepage\n";
   }

   t_stream << "\\pagenumbering{arabic}\n";

   static bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static bool usePDFLatex   = Config::getBool("latex-pdf");

   if (pdfHyperlinks && usePDFLatex) {
      // re-enable anchors again
      t_stream << "\\hypersetup{pageanchor=true}\n";
   }

   t_stream << "\n"
     "%--- Begin generated contents ---\n";
}

static void writeDefaultStyleSheet(QTextStream &t)
{
   t << ResourceMgr::instance().getAsString("latex/doxypress.sty");
}

static void writeDefaultFooter(QTextStream &t)
{
   t << "%--- End generated contents ---\n"
     "\n";

   // Bibliography
   Doxy_Globals::citeDict.writeLatexBibliography(t);

   // Index
   t << "% Index\n";

   QString unit;

   if (Config::getBool("latex-compact")) {
      unit = "section";

   } else {
      unit = "chapter";
      t << "\\backmatter\n";
   }

   t << "\\newpage\n"
        "\\phantomsection\n"
        "\\clearemptydoublepage\n"
        "\\addcontentsline{toc}{" << unit << "}{" << theTranslator->trRTFGeneralIndex() << "}\n"
        "\\printindex\n"
        "\n"
        "\\end{document}\n";
}

void LatexGenerator::writeHeaderFile(QFile &f)
{
   QTextStream t(&f);
   t << "% Latex header for DoxyPress " << versionString << endl;
   writeDefaultHeaderPart1(t);
   t << "Your title here";
   writeDefaultHeaderPart2(t);
   t << "Generated by";
   writeDefaultHeaderPart3(t);
}

void LatexGenerator::writeFooterFile(QFile &f)
{
   QTextStream t(&f);
   t << "% Latex footer for DoxyPress " << versionString << endl;
   writeDefaultFooter(t);
}

void LatexGenerator::writeStyleSheetFile(QFile &f)
{
   QTextStream t(&f);
   t << "% stylesheet for DoxyPress " << versionString << endl;
   writeDefaultStyleSheet(t);
}

void LatexGenerator::startFile(const QString &name, const QString &, const QString &)
{
   QString fileName = name;

   m_relPath = relativePathToRoot(fileName);

   if (! fileName.endsWith(".tex") && ! fileName.endsWith(".sty")) {
      fileName += ".tex";
   }

   startPlainFile(fileName);

   m_codeGen.setTextStream(m_textStream);
   m_codeGen.setRelativePath(m_relPath);
   m_codeGen.setSourceFileName(stripPath(fileName));
}

void LatexGenerator::endFile()
{
   endPlainFile();
   m_codeGen.setSourceFileName("");
}

//void LatexGenerator::writeIndex()
//{
//  startFile("refman.tex");
//}

void LatexGenerator::startProjectNumber()
{
   m_textStream << "\\\\[1ex]\\large ";
}

void LatexGenerator::startIndexSection(IndexSections is)
{
   const bool compactLatex   = Config::getBool("latex-compact");
   const QString latexHeader = Config::getString("latex-header");

   switch (is) {
      case isTitlePageStart:
         if (latexHeader.isEmpty()) {
            writeDefaultHeaderPart1(m_textStream);

         } else {
            QString header = fileToString(latexHeader);
            m_textStream << modifyKeywords(header);
         }

         break;

      case isTitlePageAuthor:
         if (latexHeader.isEmpty()) {
            writeDefaultHeaderPart2(m_textStream);
         }
         break;

      case isMainPage:
         if (compactLatex) {
            m_textStream << "\\section";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Introduction}\n"
         break;

      case isModuleIndex:
         if (compactLatex) {
            m_textStream << "\\section";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Module Index}\n"
         break;

      case isDirIndex:
         if (compactLatex) {
            m_textStream << "\\section";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Directory Index}\n"
         break;

      case isNamespaceIndex:
         if (compactLatex) {
            m_textStream << "\\section";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Namespace Index}\"
         break;

      case isClassHierarchyIndex:
         if (compactLatex) {
            m_textStream << "\\section";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Hierarchical Index}\n"
         break;

      case isCompoundIndex:
         if (compactLatex) {
            m_textStream << "\\section";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Annotated Compound Index}\n"
         break;

      case isFileIndex:
         if (compactLatex) {
            m_textStream << "\\section";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Annotated File Index}\n"
         break;

      case isPageIndex:
         if (compactLatex) {
            m_textStream << "\\section";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Annotated Page Index}\n"
         break;

      case isModuleDocumentation:
      {
         bool found = false;

         for (auto gd : Doxy_Globals::groupSDict) {
            if (found) {
               break;
            }

            if (! gd->isReference()) {
               if (compactLatex) {
                  m_textStream << "\\section";
               } else {
                  m_textStream << "\\chapter";
               }

               m_textStream << "{"; //Module Documentation}\n";
               found = true;
            }
         }
      }
      break;

      case isDirDocumentation:
      {
         bool found = false;

         for (auto dd : Doxy_Globals::directories) {
            if (found) {
               break;
            }

            if (dd->isLinkableInProject()) {
               if (compactLatex) {
                  m_textStream << "\\section";
               } else {
                  m_textStream << "\\chapter";
               }
               m_textStream << "{"; //Module Documentation}\n";
               found = true;
            }
         }
      }
      break;

      case isNamespaceDocumentation:
      {
         bool found = false;

         for (auto &nd : Doxy_Globals::namespaceSDict) {
            if (found) {
               break;
            }

            if (nd->isLinkableInProject()) {
               if (compactLatex) {
                  m_textStream << "\\section";
               } else {
                  m_textStream << "\\chapter";
               }
               m_textStream << "{"; // Namespace Documentation}\n":
               found = true;
            }
         }
      }
      break;

      case isClassDocumentation:
      {
         bool found = false;

         for (auto cd : Doxy_Globals::classSDict) {
            if (found) {
               break;
            }

            if (cd->isLinkableInProject() && cd->templateMaster() == 0 && !cd->isEmbeddedInOuterScope()) {
               if (compactLatex) {
                  m_textStream << "\\section";
               } else {
                  m_textStream << "\\chapter";
               }
               m_textStream << "{"; //Compound Documentation}\n";
               found = true;
            }
         }
      }
      break;

      case isFileDocumentation:
      {
         bool isFirst = true;

         for (auto &fn : Doxy_Globals::inputNameList) {

            for (auto fd : *fn) {

               if (fd->isLinkableInProject()) {
                  if (isFirst) {
                     if (compactLatex) {
                        m_textStream << "\\section";
                     } else {
                        m_textStream << "\\chapter";
                     }

                     m_textStream << "{"; //File Documentation}\n";
                     isFirst = false;
                     break;
                  }
               }
            }
         }
      }
      break;

      case isExampleDocumentation:

         if (compactLatex) {
            m_textStream << "\\section";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Example Documentation}\n";
         break;

      case isPageDocumentation:
         if (compactLatex) {
            m_textStream << "\\section";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Page Documentation}\n";
         break;

      case isPageDocumentation2:
         break;

      case isEndIndex:
         break;
   }
}

void LatexGenerator::endIndexSection(IndexSections is)
{
   // static bool compactLatex = Config::getBool("latex-compact");
   static bool sourceBrowser  = Config::getBool("source-code");
   static QString latexHeader = Config::getString("latex-header");
   static QString latexFooter = Config::getString("latex-footer");

   switch (is) {
      case isTitlePageStart:
         break;

      case isTitlePageAuthor:
         if (latexHeader.isEmpty()) {
            writeDefaultHeaderPart3(m_textStream);
         }
         break;

      case isMainPage:
      {
         // QString indexName = Config::getBool("generate-treeview")?"main":"index";

         QByteArray indexName = "index";
         m_textStream << "}\n\\label{index}";

         if (Config::getBool("latex-hyper-pdf")) {
            m_textStream << "\\hypertarget{index}{}";
         }
         m_textStream << "\\input{" << indexName << "}\n";
      }
      break;

      case isModuleIndex:
         m_textStream << "}\n\\input{modules}\n";
         break;
      case isDirIndex:
         m_textStream << "}\n\\input{dirs}\n";
         break;
      case isNamespaceIndex:
         m_textStream << "}\n\\input{namespaces}\n";
         break;
      case isClassHierarchyIndex:
         m_textStream << "}\n\\input{hierarchy}\n";
         break;
      case isCompoundIndex:
         m_textStream << "}\n\\input{annotated}\n";
         break;
      case isFileIndex:
         m_textStream << "}\n\\input{files}\n";
         break;
      case isPageIndex:
         m_textStream << "}\n\\input{pages}\n";
         break;

      case isModuleDocumentation:
      {
         bool found = false;

         for (auto gd : Doxy_Globals::groupSDict) {

            if (! gd->isReference()) {
               if (! found) {
                  m_textStream << "}\n\\input{" << gd->getOutputFileBase() << "}\n";
                  found = true;

               }  else {
                  m_textStream << "\\include";
                  m_textStream << "{" << gd->getOutputFileBase() << "}\n";
               }
            }
         }
      }
      break;

      case isDirDocumentation:
      {
         bool found = false;

         for (auto dd : Doxy_Globals::directories) {

            if (dd->isLinkableInProject())  {
               if (! found) {
                  m_textStream << "}\n\\input{" << dd->getOutputFileBase() << "}\n";
                  found = true;

               }  else  {
                  m_textStream << "\\input";
                  m_textStream << "{" << dd->getOutputFileBase() << "}\n";
               }
            }
         }
      }
      break;

      case isNamespaceDocumentation:
      {
         bool found = false;

         for (auto &nd : Doxy_Globals::namespaceSDict) {

            if (nd->isLinkableInProject()) {

               if (! found) {
                  m_textStream << "}\n\\input{" << nd->getOutputFileBase() << "}\n";
                  found = true;

               } else {
                  m_textStream << "\\input";
                  m_textStream << "{" << nd->getOutputFileBase() << "}\n";
               }
            }
         }
      }
      break;

      case isClassDocumentation:
      {
         bool found = false;

         for (auto cd : Doxy_Globals::classSDict) {

            if (cd->isLinkableInProject() && cd->templateMaster() == 0 && ! cd->isEmbeddedInOuterScope()) {

               if (! found) {
                  m_textStream << "}\n\\input{" << cd->getOutputFileBase() << "}\n";
                  found = true;

               } else {
                  m_textStream << "\\input";
                  m_textStream << "{" << cd->getOutputFileBase() << "}\n";
               }
            }
         }
      }
      break;

      case isFileDocumentation: {
         bool isFirst = true;

         for (auto &fn : Doxy_Globals::inputNameList) {

            for (auto fd : *fn) {

               if (fd->isLinkableInProject()) {
                  if (isFirst) {
                     m_textStream << "}\n\\input{" << fd->getOutputFileBase() << "}\n";

                     if (sourceBrowser && m_prettyCode && fd->generateSourceFile()) {
                        m_textStream << "\\input{" << fd->getSourceFileBase() << "}\n";
                     }

                     isFirst = false;

                  } else {
                     m_textStream << "\\input" ;
                     m_textStream << "{" << fd->getOutputFileBase() << "}\n";

                     if (sourceBrowser && m_prettyCode && fd->generateSourceFile()) {
                        m_textStream << "\\input{" << fd->getSourceFileBase() << "}\n";
                     }
                  }
               }
            }
         }
      }
      break;

      case isExampleDocumentation: {
         m_textStream << "}\n";

         bool isFirst = true;

         for (auto &pd : Doxy_Globals::exampleSDict) {

            if (isFirst) {
               m_textStream << "\\input{" << pd->getOutputFileBase() << "}\n";
               isFirst = false;

            } else {
               m_textStream << "\\input";
               m_textStream << "{" << pd->getOutputFileBase() << "}\n";
            }
         }
      }
      break;

      case isPageDocumentation: {
         m_textStream << "}\n";
      }
      break;

      case isPageDocumentation2:
         break;

      case isEndIndex:
         if (latexFooter.isEmpty()) {
            writeDefaultFooter(m_textStream);

         } else {
            QString footer = fileToString(latexFooter);
            m_textStream << modifyKeywords(footer);

         }
         break;
   }
}

QString LatexGenerator::modifyKeywords(const QString &output)
{
   static QString projectName    = convertToLatex(Config::getString("project-name"));
   static QString projectVersion = convertToLatex(Config::getString("project-version"));
   static QString projectBrief   = convertToLatex(Config::getString("project-brief"));
   static QString projectLogo    = Config::getString("project-logo");

   QString result = output;

   result = result.replace("$datetimeHHMM",   dateTimeHHMM());
   result = result.replace("$datetime",       dateToString(true));
   result = result.replace("$date",           dateToString(false));
   result = result.replace("$year",           yearToString());

   result = result.replace("$doxypressversion", versionString);
   result = result.replace("$doxygenversion",   versionString);         // compatibility

   result = result.replace("$projectname",    projectName);
   result = result.replace("$projectversion", projectVersion);
   result = result.replace("$projectbrief",   projectBrief);
   result = result.replace("$projectlogo",    stripPath(projectLogo));

   return result;
}

void LatexGenerator::writePageLink(const QString &name, bool)
{
   m_textStream << "\\input" ;
   m_textStream << "{" << name << "}\n";
}

void LatexGenerator::writeStyleInfo(int part)
{
   if (part > 0) {
      return;
   }

   startPlainFile("doxypress.sty");
   writeDefaultStyleSheet(m_textStream);
   endPlainFile();
}

void LatexGenerator::newParagraph()
{
   m_textStream << endl << endl;
}

void LatexGenerator::startParagraph(const QString &className)
{
   m_textStream << endl << endl;
}

void LatexGenerator::endParagraph()
{
   m_textStream << endl << endl;
}

void LatexGenerator::writeString(const QString &text)
{
   m_textStream << text;
}

void LatexGenerator::startIndexItem(const QString &ref, const QString &fn)
{
   m_textStream << "\\item ";

   if (ref.isEmpty() && ! fn.isEmpty()) {
      m_textStream << "\\contentsline{section}{";
   }
}

void LatexGenerator::endIndexItem(const QString &ref, const QString &fn)
{
   if (ref.isEmpty() && ! fn.isEmpty()) {
      m_textStream << "}{\\pageref{" << stripPath(fn) << "}}{}" << endl;
   }
}

void LatexGenerator::startHtmlLink(const QString &url)
{
   if (Config::getBool("latex-hyper-pdf")) {
      m_textStream << "\\href{";
      m_textStream << url;
      m_textStream << "}";
   }

   m_textStream << "{\\tt ";
}

void LatexGenerator::endHtmlLink()
{
   m_textStream << "}";
}

//void LatexGenerator::writeMailLink(const char *url)
//{
//  if (Config::getBool("latex-hyper-pdf")) {
//    m_textStream << "\\href{mailto:";
//    m_textStream << url;
//    m_textStream << "}";
//  }
//  m_textStream << "{\\tt ";
//  docify(url);
//  m_textStream << "}";
//}

void LatexGenerator::writeStartAnnoItem(const QString &, const QString &, const QString &path, const QString &name)
{
   m_textStream << "\\item\\contentsline{section}{\\bf ";

   if (! path.isEmpty()) {
      docify(path);
   }

   docify(name);
   m_textStream << "} ";
}

void LatexGenerator::writeEndAnnoItem(const QString &name)
{
   m_textStream << "}{\\pageref{" << stripPath(name) << "}}{}" << endl;
}

void LatexGenerator::startIndexKey()
{
   m_textStream << "\\item\\contentsline{section}{";
}

void LatexGenerator::endIndexKey()
{
}

void LatexGenerator::startIndexValue(bool hasBrief)
{
   m_textStream << " ";

   if (hasBrief) {
      m_textStream << "\\\\*";
   }
}

void LatexGenerator::endIndexValue(const QString &name, bool)
{
   m_textStream << "}{\\pageref{" << stripPath(name) << "}}{}" << endl;
}

//void LatexGenerator::writeClassLink(const char *,const char *, const char *,const char *name)
//{
//  m_textStream << "{\\bf ";
//  docify(name);
//  m_textStream << "}";
//}

void LatexGenerator::startTextLink(const QString &file, const QString &anchor)
{
   if (! disableLinks && Config::getBool("latex-hyper-pdf")) {
      m_textStream << "\\hyperlink{";

      if (! file.isEmpty()) {
         m_textStream << stripPath(file);
      }

      if (! anchor.isEmpty()) {
         m_textStream << "_" << anchor;
      }

      m_textStream << "}{";

   } else {
      m_textStream << "{\\textbf ";
   }
}

void LatexGenerator::endTextLink()
{
   m_textStream << "}";
}

void LatexGenerator::writeObjectLink(const QString &ref, const QString &file, const QString &anchor, const QString &text)
{
   static bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   if (! disableLinks && ref.isEmpty() && pdfHyperlinks) {
      m_textStream << "\\hyperlink{";

      if (! file.isEmpty()) {
         m_textStream << stripPath(file);
      }

      if (! file.isEmpty() && ! anchor.isEmpty()) {
         m_textStream << "_";
      }

      if (! anchor.isEmpty()) {
         m_textStream << anchor;
      }

      m_textStream << "}{";
      docify(text);
      m_textStream << "}";

   } else {
      m_textStream << "{\\bf ";
      docify(text);
      m_textStream << "}";
   }
}

void LatexGenerator::startPageRef()
{
   m_textStream << " \\doxyref{}{";
}

void LatexGenerator::endPageRef(const QString &clname, const QString &anchor)
{
   m_textStream << "}{";

   if (! clname.isEmpty()) {
      m_textStream << clname;
   }
   if (! anchor.isEmpty()) {
      m_textStream << "_" << anchor;
   }

   m_textStream << "}";
}

void LatexGenerator::startTitleHead(const QString &fileName)
{
   static bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static bool usePDFLatex   = Config::getBool("latex-pdf");

   if (usePDFLatex && pdfHyperlinks && ! fileName.isEmpty() ) {
      m_textStream << "\\hypertarget{" << stripPath(fileName) << "}{}";
   }

   if (Config::getBool("latex-compact")) {
      m_textStream << "\\subsection{";
   } else {
      m_textStream << "\\section{";
   }
}

void LatexGenerator::endTitleHead(const QString &fileName, const QString &name)
{
   m_textStream << "}" << endl;

   if (! name.isEmpty()) {
      m_textStream << "\\label{" << stripPath(fileName) << "}\\index{";
      m_textStream << latexEscapeLabelName(name,insideTabbing);

      m_textStream << "@{";
      m_textStream << latexEscapeIndexChars(name,insideTabbing);

      m_textStream << "}}" << endl;
   }
}

void LatexGenerator::startTitle()
{
   if (Config::getBool("latex-compact")) {
      m_textStream << "\\subsection{";

   } else {
      m_textStream << "\\section{";

   }
}

void LatexGenerator::startGroupHeader(int extraIndentLevel)
{
   if (Config::getBool("latex-compact")) {
      extraIndentLevel++;
   }

   if (extraIndentLevel == 3) {
      m_textStream << "\\subparagraph*{";
   } else if (extraIndentLevel == 2) {
      m_textStream << "\\paragraph{";
   } else if (extraIndentLevel == 1) {
      m_textStream << "\\subsubsection{";
   } else { // extraIndentLevel==0
      m_textStream << "\\subsection{";
   }

   disableLinks = true;
}

void LatexGenerator::endGroupHeader(int)
{
   disableLinks = false;
   m_textStream << "}" << endl;
}

void LatexGenerator::startMemberHeader(const QString &)
{
   if (Config::getBool("latex-compact")) {
      m_textStream << "\\subsubsection*{";
   } else {
      m_textStream << "\\subsection*{";
   }
   disableLinks = true;
}

void LatexGenerator::endMemberHeader()
{
   disableLinks = false;
   m_textStream << "}" << endl;
}

void LatexGenerator::startMemberDoc(const QString &clname, const QString &memname, const QString &,
                  const QString &title, bool showInline)
{
   if (! memname.isEmpty() && ! memname.startsWith('@')) {

      m_textStream << "\\index{";

      if (! clname.isEmpty()) {
         m_textStream << latexEscapeLabelName(clname, insideTabbing);
         m_textStream << "@{";

         m_textStream << latexEscapeIndexChars(clname, insideTabbing);
         m_textStream << "}!";
      }

      m_textStream << latexEscapeLabelName(memname, insideTabbing);
      m_textStream << "@{";

      m_textStream << latexEscapeIndexChars(memname, insideTabbing);;
      m_textStream << "}}" << endl;
      m_textStream << "\\index{";

      m_textStream << latexEscapeLabelName(memname, insideTabbing);
      m_textStream << "@{";

      m_textStream << latexEscapeIndexChars(memname, insideTabbing);
      m_textStream << "}";

      if (! clname.isEmpty()) {
         m_textStream << "!";

         m_textStream << latexEscapeLabelName(clname, insideTabbing);
         m_textStream << "@{";

         m_textStream << latexEscapeIndexChars(clname, insideTabbing);
         m_textStream << "}";
      }

      m_textStream << "}" << endl;
   }

   static const char *levelLab[] = { "subsubsection", "paragraph", "subparagraph", "subparagraph" };
   static bool compactLatex  = Config::getBool("latex-compact");
   static bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   int level = 0;

   if (showInline) {
      level += 2;
   }

   if (compactLatex) {
      level++;
   }

   m_textStream << "\\" << levelLab[level];
   m_textStream << "{";

   if (pdfHyperlinks)   {
      m_textStream << "\\texorpdfstring{";
   }

   m_textStream << latexEscapeIndexChars(title, insideTabbing);

   if (pdfHyperlinks) {
      m_textStream << "}{" << latexEscapePDFString(title) << "}";
   }

   m_textStream << "}";
   m_textStream << "\n{\\ttfamily ";
   // disableLinks = true;
}

void LatexGenerator::endMemberDoc(bool)
{
   disableLinks = false;
   m_textStream << "}";
}

void LatexGenerator::startDoxyAnchor(const QString &fName, const QString &, const QString &anchor,
                  const QString &, const QString &)
{
   static bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static bool usePDFLatex   = Config::getBool("latex-pdf");

   m_textStream << "\\mbox{";

   if (usePDFLatex && pdfHyperlinks) {
      m_textStream << "\\Hypertarget{";

      if (! fName.isEmpty()) {
         m_textStream << stripPath(fName);
      }

      if (! anchor.isEmpty()) {
         m_textStream << "_" << anchor;
      }

      m_textStream << "}";
   }

   m_textStream << "\\label{";

   if (! fName.isEmpty()) {
      m_textStream << stripPath(fName);
   }

   if (! anchor.isEmpty()) {
      m_textStream << "_" << anchor;
   }

   m_textStream << "}} " << endl;
}

void LatexGenerator::endDoxyAnchor(const QString &fName, const QString &anchor)
{
}

void LatexGenerator::writeAnchor(const QString &fName, const QString &name)
{
   m_textStream << "\\label{" << stripPath(name) << "}" << endl;

   static bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static bool usePDFLatex   = Config::getBool("latex-pdf");

   if (usePDFLatex && pdfHyperlinks) {

      if (! fName.isEmpty()) {
         m_textStream  << "\\Hypertarget{" << stripPath(fName) << "_" << stripPath(name) << "}" << endl;

      } else {
         m_textStream  << "\\Hypertarget{" << stripPath(name) << "}" << endl;
      }
   }
}


//void LatexGenerator::writeLatexLabel(const char *clName,const char *anchor)
//{
//  writeDoxyAnchor(0,clName,anchor,0);
//}

void LatexGenerator::addIndexItem(const QString &s1, const QString &s2)
{
   if (! s1.isEmpty()) {
      m_textStream << "\\index{";

      m_textStream << latexEscapeLabelName(s1, insideTabbing);
      m_textStream << "@{";

      m_textStream << latexEscapeIndexChars(s1, insideTabbing);
      m_textStream << "}";

      if (! s2.isEmpty()) {
         m_textStream << "!";

         m_textStream << latexEscapeLabelName(s2, insideTabbing);
         m_textStream << "@{";

         m_textStream << latexEscapeIndexChars(s2, insideTabbing);
         m_textStream << "}";
      }

      m_textStream << "}";
   }
}

void LatexGenerator::startSection(const QString &lab, const QString &, SectionInfo::SectionType type)
{
   static bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static bool usePDFLatex   = Config::getBool("latex-pdf");

   if (usePDFLatex && pdfHyperlinks) {
      m_textStream << "\\hypertarget{" << stripPath(lab) << "}{}";
   }

   m_textStream << "\\";

   if (Config::getBool("latex-compact")) {
      switch (type) {
         case SectionInfo::Page:
            m_textStream << "subsection";
            break;
         case SectionInfo::Section:
            m_textStream << "subsubsection";
            break;
         case SectionInfo::Subsection:
            m_textStream << "paragraph";
            break;
         case SectionInfo::Subsubsection:
            m_textStream << "subparagraph";
            break;
         case SectionInfo::Paragraph:
            m_textStream << "subparagraph";
            break;
         default:
            assert(0);
            break;
      }

      m_textStream << "{";

   } else {
      switch (type) {
         case SectionInfo::Page:
            m_textStream << "section";
            break;
         case SectionInfo::Section:
            m_textStream << "subsection";
            break;
         case SectionInfo::Subsection:
            m_textStream << "subsubsection";
            break;
         case SectionInfo::Subsubsection:
            m_textStream << "paragraph";
            break;
         case SectionInfo::Paragraph:
            m_textStream << "subparagraph";
            break;
         default:
            assert(0);
            break;
      }
      m_textStream << "{";
   }
}

void LatexGenerator::endSection(const QString &lab, SectionInfo::SectionType)
{
   m_textStream << "}\\label{" << lab << "}" << endl;
}

void LatexGenerator::docify(const QString &text)
{
   filterLatexString(m_textStream, text, insideTabbing, false, false);
}

void LatexGenerator::writeChar(char c)
{
   char cs[2];
   cs[0] = c;
   cs[1] = 0;
   docify(cs);
}

void LatexGenerator::startClassDiagram()
{
   // if (Config::getBool("compact-latex")) m_textStream << "\\subsubsection"; else m_textStream << "\\subsection";
   // m_textStream << "{";
}

void LatexGenerator::endClassDiagram(const ClassDiagram &d, const QString &fname, const QString &)
{
   d.writeFigure(m_textStream, m_dir, fname);
}

void LatexGenerator::startAnonTypeScope(int indent)
{
   if (indent == 0) {
      m_textStream << "\\begin{tabbing}" << endl;
      m_textStream << "xx\\=xx\\=xx\\=xx\\=xx\\=xx\\=xx\\=xx\\=xx\\=\\kill" << endl;
      insideTabbing = true;
   }

   m_indent = indent;
}

void LatexGenerator::endAnonTypeScope(int indent)
{
   if (indent == 0) {
      m_textStream << endl << "\\end{tabbing}";
      insideTabbing = false;
   }
   m_indent = indent;
}

void LatexGenerator::startMemberTemplateParams()
{
   if (templateMemberItem) {
      m_textStream << "{\\footnotesize ";
   }
}

void LatexGenerator::endMemberTemplateParams(const QString &, const QString &)
{
   if (templateMemberItem) {
      m_textStream << "}\\\\";
   }
}

void LatexGenerator::startMemberItem(const QString &, int annoType, const QString &)
{
   if (! insideTabbing) {
      m_textStream << "\\item " << endl;
      templateMemberItem = (annoType == 3);
   }
}

void LatexGenerator::endMemberItem()
{
   if (insideTabbing) {
      m_textStream << "\\\\";
   }
   templateMemberItem = false;
   m_textStream << endl;
}

void LatexGenerator::startMemberDescription(const QString &, const QString &)
{
   if (!insideTabbing) {
      m_textStream << "\\begin{DoxyCompactList}\\small\\item\\em ";

   } else {
      for (int i = 0; i < m_indent + 2; i++) {
         m_textStream << "\\>";
      }
      m_textStream << "{\\em ";
   }
}

void LatexGenerator::endMemberDescription()
{
   if (! insideTabbing) {
      m_textStream << "\\end{DoxyCompactList}";
   } else {
      m_textStream << "}\\\\\n";
   }
}

void LatexGenerator::writeNonBreakableSpace(int)
{
   if (insideTabbing) {
      m_textStream << "\\>";
   } else {
      m_textStream << "~";
   }
}

void LatexGenerator::startMemberList()
{
   if (!insideTabbing) {
      m_textStream << "\\begin{DoxyCompactItemize}" << endl;
   }
}

void LatexGenerator::endMemberList()
{
   if (! insideTabbing) {
      m_textStream << "\\end{DoxyCompactItemize}"   << endl;
   }
}

void LatexGenerator::startMemberGroupHeader(bool hasHeader)
{
   if (hasHeader) {
      m_textStream << "\\begin{Indent}";
   }

   m_textStream << "{\\bf ";
}

void LatexGenerator::endMemberGroupHeader()
{
   // changed back to rev 756 due to bug 660501
   m_textStream << "}\\par" << endl;
   //t << "}" << endl;
}

void LatexGenerator::startMemberGroupDocs()
{
   m_textStream << "{\\em ";
}

void LatexGenerator::endMemberGroupDocs()
{
   m_textStream << "}";
}

void LatexGenerator::startMemberGroup()
{
}

void LatexGenerator::endMemberGroup(bool hasHeader)
{
   if (hasHeader) {
      m_textStream << "\\end{Indent}";
   }
   m_textStream << endl;
}

void LatexGenerator::startDotGraph()
{
   newParagraph();
}

void LatexGenerator::endDotGraph(const DotClassGraph &g)
{
   g.writeGraph(m_textStream, GOF_EPS, EOF_LaTeX, Config::getString("latex-output"), m_fileName, m_relPath);
}

void LatexGenerator::startInclDepGraph()
{
}

void LatexGenerator::endInclDepGraph(const DotInclDepGraph &g)
{
   g.writeGraph(m_textStream, GOF_EPS, EOF_LaTeX, Config::getString("latex-output"), m_fileName, m_relPath);
}

void LatexGenerator::startGroupCollaboration()
{
}

void LatexGenerator::endGroupCollaboration(const DotGroupCollaboration &g)
{
   g.writeGraph(m_textStream, GOF_EPS, EOF_LaTeX, Config::getString("latex-output"), m_fileName, m_relPath);
}

void LatexGenerator::startCallGraph()
{
}

void LatexGenerator::endCallGraph(const DotCallGraph &g)
{
   g.writeGraph(m_textStream, GOF_EPS, EOF_LaTeX, Config::getString("latex-output"), m_fileName, m_relPath);
}

void LatexGenerator::startDirDepGraph()
{
}

void LatexGenerator::endDirDepGraph(const DotDirDeps &g)
{
   g.writeGraph(m_textStream, GOF_EPS, EOF_LaTeX, Config::getString("latex-output"), m_fileName, m_relPath);
}

void LatexGenerator::startDescription()
{
   m_textStream << "\\begin{description}" << endl;
}

void LatexGenerator::endDescription()
{
   m_textStream << "\\end{description}" << endl;
   firstDescItem = true;
}

void LatexGenerator::startDescItem()
{
   firstDescItem = true;
   m_textStream << "\\item[";
}

void LatexGenerator::endDescItem()
{
   if (firstDescItem) {
      m_textStream << "]" << endl;
      firstDescItem = false;
   } else {
      lineBreak();
   }
}

void LatexGenerator::startSimpleSect(SectionTypes, const QString &file, const QString &anchor, const QString &title)
{
   m_textStream << "\\begin{Desc}\n\\item[";

   if (! file.isEmpty()) {
      writeObjectLink(0, file, anchor, title);
   } else {
      docify(title);
   }
   m_textStream << "]";
}

void LatexGenerator::endSimpleSect()
{
   m_textStream << "\\end{Desc}" << endl;
}

void LatexGenerator::startParamList(ParamListTypes, const QString &title)
{
   m_textStream << "\\begin{Desc}\n\\item[";
   docify(title);
   m_textStream << "]";
}

void LatexGenerator::endParamList()
{
   m_textStream << "\\end{Desc}" << endl;
}

void LatexGenerator::startParameterList(bool openBracket)
{
   /* start of ParameterType ParameterName list */
   if (openBracket) {
      m_textStream << "(";
   }

   m_textStream << endl << "\\begin{DoxyParamCaption}" << endl;
}

void LatexGenerator::endParameterList()
{
}

void LatexGenerator::startParameterType(bool first, const QString &key)
{
   m_textStream << "\\item[{";

   if (! first && ! key.isEmpty()) {
      m_textStream << key;
   }
}

void LatexGenerator::endParameterType()
{
   m_textStream << "}]";
}

void LatexGenerator::startParameterName(bool)
{
   m_textStream << "{";
}

void LatexGenerator::endParameterName(bool last, bool, bool closeBracket)
{
   m_textStream << "}" << endl;

   if (last) {
      m_textStream << "\\end{DoxyParamCaption}" << endl;
      if (closeBracket) {
         m_textStream << ")";
      }
   }
}

void LatexGenerator::exceptionEntry(const QString &prefix, bool closeBracket)
{
   if (! prefix.isEmpty()) {
      m_textStream << " " << prefix;

   } else if (closeBracket) {
      m_textStream << ")";

   }

   m_textStream << " ";
}

void LatexGenerator::writeDoc(DocNode *n, QSharedPointer<Definition> ctx, QSharedPointer<MemberDef> )
{
   LatexDocVisitor *visitor = new LatexDocVisitor(m_textStream, *this, ctx ? ctx->getDefFileExtension() : QString(""), insideTabbing);
   n->accept(visitor);
   delete visitor;
}

void LatexGenerator::startConstraintList(const QString &header)
{
   m_textStream << "\\begin{Desc}\n\\item[";
   docify(header);
   m_textStream << "]";
   m_textStream << "\\begin{description}" << endl;
}

void LatexGenerator::startConstraintParam()
{
   m_textStream << "\\item[{\\em ";
}

void LatexGenerator::endConstraintParam()
{
}

void LatexGenerator::startConstraintType()
{
   m_textStream << "} : {\\em ";
}

void LatexGenerator::endConstraintType()
{
   m_textStream << "}]";
}

void LatexGenerator::startConstraintDocs()
{
}

void LatexGenerator::endConstraintDocs()
{
}

void LatexGenerator::endConstraintList()
{
   m_textStream << "\\end{description}" << endl;
   m_textStream << "\\end{Desc}" << endl;
}

void LatexGenerator::startCodeFragment()
{
   m_textStream << "\n\\begin{DoxyCode}\n";
}

void LatexGenerator::endCodeFragment()
{
   m_textStream << "\\end{DoxyCode}\n";
}

void LatexGenerator::startInlineHeader()
{
   if (Config::getBool("latex-compact")) {
      m_textStream << "\\paragraph*{";
   } else {
      m_textStream << "\\subsubsection*{";
   }
}

void LatexGenerator::endInlineHeader()
{
   m_textStream << "}" << endl;
}

void LatexGenerator::lineBreak(const QString &style)
{
   if (insideTabbing) {
      m_textStream << "\\\\\n";

   } else {
      m_textStream << "\\newline\n";

   }
}

void LatexGenerator::startMemberDocSimple()
{
   m_textStream << "\\begin{DoxyFields}{";
   docify(theTranslator->trCompoundMembers());
   m_textStream << "}" << endl;
}

void LatexGenerator::endMemberDocSimple()
{
   m_textStream << "\\end{DoxyFields}" << endl;
}

void LatexGenerator::startInlineMemberType()
{
   // to prevent \+ from causing unwanted breaks
   insideTabbing = true;
}

void LatexGenerator::endInlineMemberType()
{
   m_textStream << "&" << endl;
   insideTabbing = false;
}

void LatexGenerator::startInlineMemberName()
{
   // to prevent \+ from causing unwanted breaks
   insideTabbing = true;
}

void LatexGenerator::endInlineMemberName()
{
   m_textStream << "&" << endl;
   insideTabbing = false;
}

void LatexGenerator::startInlineMemberDoc()
{
}

void LatexGenerator::endInlineMemberDoc()
{
   m_textStream << "\\\\\n\\hline\n" << endl;
}

void LatexGenerator::startLabels()
{
   m_textStream << "\\hspace{0.3cm}";
}

void LatexGenerator::writeLabel(const QString &l, bool isLast)
{
   m_textStream << "{\\ttfamily [" << l << "]}";

   if (!isLast) {
      m_textStream << ", ";
   }
}

void LatexGenerator::endLabels()
{
}

