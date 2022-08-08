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

#include <QDir>
#include <QStandardPaths>

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

#include <stdlib.h>

LatexCodeGenerator::LatexCodeGenerator(QTextStream &t, const QString &relPath, const QString &sourceFileName)
   : m_relPath(relPath), m_sourceFileName(sourceFileName), m_col(0), m_usedTableLevel(0), m_t(t)
{
   m_prettyCode = Config::getBool("latex-source-code");
}

void LatexCodeGenerator::codify(const QString &str)
{
   static const int tabSize  = Config::getInt("tab-size");

   if (str.isEmpty()) {
      return;
   }

   QString result;

   QString::const_iterator iter     = str.constBegin();
   QString::const_iterator iter_end = str.constEnd();
   QChar c;

   while (iter != iter_end) {
      c = *iter;

      switch(c.unicode())  {
         case 0x0c:
            // remove ^L
            ++iter;
            break;

        case ' ':
            m_t << " ";
            m_col++;
            ++iter;

            break;

        case '^':
            m_t << "\\string^";
            m_col++;
            ++iter;

            break;

         case '\t':  {
            int spacesToNextTabStop = tabSize - (m_col % tabSize);
            m_t << QString(spacesToNextTabStop, QChar(' '));
            m_col += spacesToNextTabStop;

            ++iter;
            break;
         }

         case '\n':
            m_t << '\n';

            m_col = 0;
            ++iter;

            break;

         default:
            // gather characters until we find whitespace or reach the end of a line
            result = c;
            ++iter;

            // copy more characters
            while (iter != iter_end) {
               c = *iter;

               if (c != 0x0c && c != '\t' && c != '\n' && c != ' ' && c!='^')  {
                  result += c;
                  ++iter;
               } else {
                  break;
               }
            }

            filterLatexString(m_t, result, false, true, false, m_usedTableLevel > 0, false);
            break;
     }
  }

}

void LatexCodeGenerator::writeCodeLink(const QString &ref, const QString &f, const QString &anchor,
                  const QString &name, const QString &)
{
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static const bool usePDFLatex   = Config::getBool("latex-pdf");

   int len = name.length();

   if (ref.isEmpty() && usePDFLatex && pdfHyperlinks) {
      m_t << "\\mbox{\\hyperlink{";

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
      m_t << "}}";

   } else {
      codify(name);
   }

   m_col += len;
}

void LatexCodeGenerator::writeLineNumber(const QString &ref, const QString &fileName, const QString &anchor, int len)
{
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static const bool usePDFLatex   = Config::getBool("latex-pdf");

   if (! m_doxyCodeLineOpen) {
      m_t << "\\DoxyCodeLine{";
      m_doxyCodeLineOpen = true;
   }

   if (m_prettyCode) {
      QString lineNumber = QString("%1").formatArg(len, 5, 10, QChar('0'));

      if (! fileName.isEmpty() && ! m_sourceFileName.isEmpty()) {
         QString lineAnchor = "_l" + lineNumber;
         lineAnchor.prepend(stripExtensionGeneral(m_sourceFileName, ".tex"));

         if (usePDFLatex && pdfHyperlinks) {
            m_t << "\\Hypertarget{" << stripPath(lineAnchor) << "}";
         }

         writeCodeLink(ref, fileName, anchor, lineNumber, QString());

      } else {
         codify(lineNumber);
      }

      m_t << " ";

   } else {
      m_t << len << " ";
   }

   m_col = 0;
}

void LatexCodeGenerator::startCodeLine(bool)
{
   m_col = 0;

   if (! m_doxyCodeLineOpen) {
      m_t << "\\DoxyCodeLine{";
      m_doxyCodeLineOpen = true;
   }
}

void LatexCodeGenerator::endCodeLine()
{
  if (m_doxyCodeLineOpen) {
      m_t << "}";
      m_doxyCodeLineOpen = false;
   }

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

void LatexCodeGenerator::startCodeFragment(const QString &style)
{
   m_t << "\n\\begin{" << style << "}{" << m_usedTableLevel << "}\n";
}

void LatexCodeGenerator::endCodeFragment(const QString &style)
{
   endCodeLine();
   m_t << "\\end{" << style << "}\n";
}

// **
LatexGenerator::LatexGenerator() : OutputGenerator()
{
   m_outputDir  = Config::getString("latex-output");
   m_prettyCode = Config::getBool("latex-source-code");

   m_insideTabbing      = false;
   m_firstDescItem      = true;
   m_disableLinks       = false;
   m_templateMemberItem = false;

   m_indent = 0;
}

static void writeLatexMakefile()
{
   static const bool generateBib      = ! Doxy_Globals::citeDict.isEmpty();
   static const bool usePDFLatex      = Config::getBool("latex-pdf");
   static const QString latexOutput   = Config::getString("latex-output");
   static const QString latex_command = Config::getString("latex-cmd-name");
   static const QString mkidx_command = Config::getString("make-index-cmd-name");

   QString fileName = latexOutput + "/Makefile";

   QFile file(fileName);

   if (! file.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), file.error());
      Doxy_Work::stopDoxyPress();
   }

   QTextStream t(&file);

   if (! usePDFLatex) {
      // use plain old latex

      t << "LATEX_CMD=" << latex_command << endl
        << endl;

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
        << "\t$(LATEX_CMD) refman.tex" << endl
        << "\techo \"Running makeindex...\"" << endl
        << "\t" << mkidx_command << " refman.idx" << endl;

      if (generateBib) {
         t << "\techo \"Running bibtex...\"" << endl;
         t << "\tbibtex refman" << endl;
         t << "\techo \"Rerunning latex....\"" << endl;
         t << "\t$(LATEX_CMD) refman.tex" << endl;
      }

      t << "\techo \"Rerunning latex....\"" << endl
        << "\t$(LATEX_CMD) refman.tex" << endl
        << "\tlatex_count=8 ; \\" << endl
        << "\twhile egrep -s 'Rerun (LaTeX|to get cross-references right)' refman.log && [ $$latex_count -gt 0 ] ;\\" << endl
        << "\t    do \\" << endl
        << "\t      echo \"Rerunning latex....\" ;\\" << endl
        << "\t      $(LATEX_CMD) refman.tex ; \\" << endl
        << "\t      latex_count=`expr $$latex_count - 1` ;\\" << endl
        << "\t    done" << endl
        << "\t" << mkidx_command << " refman.idx" << endl
        << "\t$(LATEX_CMD) refman.tex" << endl << endl
        << "refman_2on1.ps: refman.ps" << endl
        << "\tpsnup -2 refman.ps >refman_2on1.ps" << endl
        << endl
        << "refman_2on1.pdf: refman_2on1.ps" << endl
        << "\tps2pdf refman_2on1.ps refman_2on1.pdf" << endl;

   } else {
      // use pdflatex for higher quality output

      t << "LATEX_CMD=" << latex_command << endl
        << endl;
      t << "all: refman.pdf" << endl << endl
        << "pdf: refman.pdf" << endl << endl;
      t << "refman.pdf: clean refman.tex" << endl;
      t << "\t$(LATEX_CMD) refman" << endl;
      t << "\t" << mkidx_command << " refman.idx" << endl;

      if (generateBib) {
         t << "\tbibtex refman" << endl;
         t << "\t$(LATEX_CMD) refman" << endl;
      }

      t << "\t$(LATEX_CMD) refman" << endl
        << "\tlatex_count=8 ; \\" << endl
        << "\twhile egrep -s 'Rerun (LaTeX|to get cross-references right)' refman.log && [ $$latex_count -gt 0 ] ;\\" << endl
        << "\t    do \\" << endl
        << "\t      echo \"Rerunning latex....\" ;\\" << endl
        << "\t      $(LATEX_CMD) refman ;\\" << endl
        << "\t      latex_count=`expr $$latex_count - 1` ;\\" << endl
        << "\t    done" << endl
        << "\t" << mkidx_command << " refman.idx" << endl
        << "\t$(LATEX_CMD) refman" << endl << endl;
   }

   t << endl
     << "clean:" << endl
     << "\trm -f "
     << "*.ps *.dvi *.aux *.toc *.idx *.ind *.ilg *.log *.out *.brf *.blg *.bbl refman.pdf" << endl;
}

static void writeMakeBat()
{

#if defined(Q_OS_WIN)
   static const bool usePDFLatex      = Config::getBool("latex-pdf");
   static const QString latexOutput   = Config::getString("latex-output");
   static const QString latex_command = Config::getString("latex-cmd-name");
   static const QString mkidx_command = Config::getString("make-index-cmd-name");

   QString fileName = latexOutput  + "/make.bat";

   QFile file(fileName);

   bool generateBib = ! Doxy_Globals::citeDict.isEmpty();

   if (! file.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), file.error());
      Doxy_Work::stopDoxyPress();
   }

   QTextStream t(&file);
   t << "set Dir_Old=%cd%\n";
   t << "cd /D %~dp0\n\n";
   t << "del /s /f *.ps *.dvi *.aux *.toc *.idx *.ind *.ilg *.log *.out *.brf *.blg *.bbl refman.pdf\n\n";

   if (! usePDFLatex) {
      // use plain old latex

      t << "set LATEX_CMD=" << latex_command << "\n";
      t << "%LATEX_CMD% refman.tex\n";

      t << "echo ----\n";
      t << mkidx_command << " refman.idx\n";

      if (generateBib) {
         t << "bibtex refman\n";
         t << "echo ----\n";
         t << "\t%LATEX_CMD% refman.tex\n";
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
      t << "%LATEX_CMD% refman.tex\n";
      t << "goto :repeat\n";
      t << ":skip\n";
      t << "endlocal\n";
      t << mkidx_command << " refman.idx\n";
      t << "%LATEX_CMD% refman.tex\n";
      t << "dvips -o refman.ps refman.dvi\n";

      QString gsPath = QStandardPaths::findExecutable("gswin64c");

      if (gsPath.isEmpty()) {
         gsPath = QStandardPaths::findExecutable("gswin32c");
      }

      if (gsPath.isEmpty()) {
         gsPath = "gswin32c";
      }

      t << gsPath + " -q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite "
        "-sOutputFile=refman.pdf -c save pop -f refman.ps\n";

   } else {
      // use pdflatex

      t << "set LATEX_CMD=" << latex_command << "\n";
      t << "%LATEX_CMD% refman\n";
      t << "echo ----\n";
      t << mkidx_command << " refman.idx\n";

      if (generateBib) {
         t << "bibtex refman" << endl;
         t << "%LATEX_CMD% refman" << endl;
      }

      t << "echo ----\n";
      t << "%LATEX_CMD% refman\n\n";
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
      t << "%LATEX_CMD% refman\n";
      t << "goto :repeat\n";
      t << ":skip\n";
      t << "endlocal\n";
      t << mkidx_command << " refman.idx\n";
      t << "%LATEX_CMD% refman\n";
      t << "cd /D %Dir_Old%\n";
      t << "set Dir_Old=\n";
   }
#endif

}

void LatexGenerator::init()
{
   QDir d(m_outputDir);

   if (! d.exists() && ! d.mkdir(m_outputDir)) {
      err("Unable to create output directory %s\n", csPrintable(m_outputDir));
      Doxy_Work::stopDoxyPress();
   }

   writeLatexMakefile();
   writeMakeBat();

   createSubDirs(d);
}

static void writeDefaultHeaderPart1(QTextStream &t_stream)
{
   static const QString projectName         = Config::getString("project-name");
   static const QString projectBrief        = Config::getString("project-brief");
   static const QString outputLanguage      = Config::getEnum("output-language").toLower();
   static const QString paperName           = Config::getEnum("latex-paper-type");
   static const QStringList latexStyleSheet = Config::getList("latex-stylesheets");

   static const bool latexBatchMode         = Config::getBool("latex-batch-mode");
   static const bool latexCompact           = Config::getBool("latex-compact");
   static const bool timeStamp              = Config::getBool("latex-timestamp");
   static const bool pdfHyperlinks          = Config::getBool("latex-hyper-pdf");
   static const bool usePDFLatex            = Config::getBool("latex-pdf");

   // Handle batch mode
   if (latexBatchMode) {
      t_stream << "\\batchmode\n";
   }

   // too many open files
   t_stream << "\\let\\mypdfximage\\pdfximage"
               "\\def\\pdfximage{\\immediate\\mypdfximage}";

   // Set document class depending on configuration
   QByteArray documentClass;

   if (latexCompact) {
      documentClass = "article";
   } else {
      documentClass = "book";
   }

   t_stream << "\\documentclass[twoside]{" << documentClass << "}\n\n";

   t_stream << "%% moved from doxypress.sty due to workaround for LaTex 2019 version and unmaintained tabu package\n"
               "\\usepackage{ifthen}\n"
               "\\ifx\\requestedLaTeXdate\\undefined\n"
               "\\usepackage{array}\n"
               "\\else\n"
               "\\usepackage{array}[=2016-10-06]\n"
               "\\fi\n"
               "%%\n";


   // Load required packages
   t_stream << "% Packages required by DoxyPress\n"
      "\\usepackage{fixltx2e}\n" // for \textsubscript
      "\\usepackage{calc}\n"
      "\\usepackage{doxypress}\n";

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
                "\\usepackage{ifpdf,ifxetex}\n"
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

   // QString emojiDir = Config_getString(latex-emoji-directory);
   // emerald - add later

   QString emojiDir = Config::getString("latex-output");

   if (emojiDir.isEmpty()) {
      emojiDir = ".";
   }

   emojiDir = substitute(emojiDir, "\\", "/");

   t_stream << "% Arguments of doxyemoji:\n"
      "% 1) ':<text>:' form of the emoji, already \"LaTeX\"-escaped\n"
      "% 2) file with the name of the emoji without the .png extension\n"
      "% in case image exist use this otherwise use the ':<text>:' form\n";

   t_stream << "\\newcommand{\\doxyemoji}[2]{%\n"
               "  \\IfFileExists{" << emojiDir << "/#2.png}{\\raisebox{-0.1em}{\\includegraphics[height=0.9em]{"
           << emojiDir << "/#2.png}}}{#1}%\n}\n";


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
     "\\newcommand{\\doxynormalparskip}{\\setlength{\\parskip}{3ex plus 2ex minus 2ex}}\n"
     "\\newcommand{\\doxytocparskip}{\\setlength{\\parskip}{1ex plus 0ex minus 0ex}}\n"
     "\\doxynormalparskip\n";

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

   t_stream << "\\makeatletter\n"
     "\\newcommand\\hrulefilll{\\leavevmode\\leaders\\hrule\\hskip 0pt plus 1filll\\kern\\z@}\n"
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

   filterLatexString(tg, generatedBy, false, false, false, false, false);

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
   writeLatexSpecialFormulaChars(t_stream);

   // Hyperlinks
   if (pdfHyperlinks) {
      t_stream << "% Hyperlinks (required, but should be loaded last)\n"
        "\\ifpdf\n"
        "  \\usepackage[pdftex,pagebackref=true]{hyperref}\n"
        "\\else\n"
        "  \\ifxetex\n"
        "    \\usepackage[pagebackref=true]{hyperref}\n"
        "  \\else\n"
        "    \\usepackage[ps2pdf,pagebackref=true]{hyperref}\n"
        "  \\fi\n"
        "\\fi\n"
        "\n"
        "\\hypersetup{%\n"
        "  colorlinks=true,%\n"
        "  linkcolor=blue,%\n"
        "  citecolor=blue,%\n"
        "  unicode,%\n"
        "  pdftitle={" << projectName << "},%\n"
        "  pdfsubject={" << projectBrief << "}%\n"
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

   // in page table of contents
   t_stream << "\\usepackage{etoc}\n"
       "\\etocsettocstyle{\\doxytocparskip}{\\doxynormalparskip}\n";
   // prevent numbers overlap the titles in toc
   t_stream << "\\renewcommand{\\numberline}[1]{#1~}\n";

   // End of preamble, now comes the document contents
   t_stream << "%===== C O N T E N T S =====\n"
     "\n"
     "\\begin{document}\n"
     "\\raggedbottom\n";

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
   static const bool timeStamp     = Config::getBool("latex-timestamp");
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static const bool usePDFLatex   = Config::getBool("latex-pdf");
   static const bool compactLatex  = Config::getBool("latex-compact");

   // Finalize project number
   t_stream << " DoxyPress " << versionString << "}\\\\\n";

   if (timeStamp) {
     t_stream << "\\vspace*{0.5cm}\n"
                 "{\\small " << dateToString(true) << "}\\\\\n";
   }

   t_stream << "\\end{center}\n"
               "\\end{titlepage}\n";

   if (! compactLatex) {
      t_stream << "\\clearemptydoublepage\n";
      t_stream << "\\pagenumbering{roman}\n";
   }

   // ToC
   t_stream << "\\tableofcontents\n";
   if (! compactLatex) {
      t_stream << "\\clearemptydoublepage\n";
   }

   t_stream << "\\pagenumbering{arabic}\n";

   if (pdfHyperlinks && usePDFLatex) {
      // re-enable anchors
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
        "\\addcontentsline{toc}{" << unit << "}{\\indexname}\n"
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
   m_codeGen = QMakeShared<LatexCodeGenerator>(m_textStream, m_relPath, stripPath(fileName));
}

void LatexGenerator::endFile()
{
   endPlainFile();
}

// void LatexGenerator::writeIndex()
// {
//  startFile("refman.tex");
// }

void LatexGenerator::startProjectNumber()
{
   m_textStream << "\\\\[1ex]\\large ";
}

void LatexGenerator::startIndexSection(IndexSections is)
{
   static const bool compactLatex   = Config::getBool("latex-compact");
   static const QString latexHeader = Config::getString("latex-header");

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
            m_textStream << "\\doxysection";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Introduction}\n"
         break;

      case isModuleIndex:
         if (compactLatex) {
            m_textStream << "\\doxysection";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Module Index}\n"
         break;

      case isDirIndex:
         if (compactLatex) {
            m_textStream << "\\doxysection";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Directory Index}\n"
         break;

      case isNamespaceIndex:
         if (compactLatex) {
            m_textStream << "\\doxysection";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Namespace Index}\"
         break;

      case isClassHierarchyIndex:
         if (compactLatex) {
            m_textStream << "\\doxysection";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Hierarchical Index}\n"
         break;

      case isCompoundIndex:
         if (compactLatex) {
            m_textStream << "\\doxysection";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Annotated Compound Index}\n"
         break;

      case isFileIndex:
         if (compactLatex) {
            m_textStream << "\\doxysection";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Annotated File Index}\n"
         break;

      case isPageIndex:
         if (compactLatex) {
            m_textStream << "\\doxysection";
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
                  m_textStream << "\\doxysection";
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
                  m_textStream << "\\doxysection";
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
                  m_textStream << "\\doxysection";
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
                  m_textStream << "\\doxysection";
               } else {
                  m_textStream << "\\chapter";
               }
               m_textStream << "{"; //Compound Documentation}\n";
               found = true;
            }
         }
      }
      break;

      case isFileDocumentation: {
         // show source code
         bool isFirst = true;

         for (auto &fn : Doxy_Globals::inputNameList) {

            for (auto fd : *fn) {

               if (fd->isLinkableInProject()) {
                  if (isFirst) {
                     if (compactLatex) {
                        m_textStream << "\\doxysection";
                     } else {
                        m_textStream << "\\chapter";
                     }

                     m_textStream << "{"; //File Source Documentation}\n";
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
            m_textStream << "\\doxysection";
         } else {
            m_textStream << "\\chapter";
         }

         m_textStream << "{"; //Example Documentation}\n";
         break;

      case isPageDocumentation:
         if (compactLatex) {
            m_textStream << "\\doxysection";
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
   static const bool sourceCode     = Config::getBool("source-code");
   static const QString latexHeader = Config::getString("latex-header");
   static const QString latexFooter = Config::getString("latex-footer");
   static const bool pdfHyperlinks  = Config::getBool("latex-hyper-pdf");

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
         m_textStream << "}\n\\label{index}";

         if (pdfHyperlinks) {
            m_textStream << "\\hypertarget{index}{}";
         }

         m_textStream << "\\input{index}\n";
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

      case isDirDocumentation: {
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

      case isNamespaceDocumentation: {
         bool found = false;

         for (auto &nd : Doxy_Globals::namespaceSDict) {

            if (nd->isLinkableInProject() && ! nd->isAlias()) {

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

      case isClassDocumentation: {
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
         // show source code
         bool isFirst = true;

         for (auto &fn : Doxy_Globals::inputNameList) {

            for (auto fd : *fn) {

               if (fd->isLinkableInProject()) {
                  if (isFirst) {
                     m_textStream << "}\n\\input{" << fd->getOutputFileBase() << "}\n";

                     if (sourceCode && m_prettyCode && fd->generateSourceFile()) {
                        m_textStream << "\\input{" << fd->getSourceFileBase() << "}\n";
                     }

                     isFirst = false;

                  } else {
                     m_textStream << "\\input" ;
                     m_textStream << "{" << fd->getOutputFileBase() << "}\n";

                     if (sourceCode && m_prettyCode && fd->generateSourceFile()) {
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

   result = result.replace("$datetimeHHMM",     dateTimeHHMM());
   result = result.replace("$datetime",         dateToString(true));
   result = result.replace("$date",             dateToString(false));
   result = result.replace("$year",             yearToString());

   result = result.replace("$doxypressversion", versionString);
   result = result.replace("$doxygenversion",   versionString);         // compatibility

   result = result.replace("$projectname",      projectName);
   result = result.replace("$projectversion",   projectVersion);
   result = result.replace("$projectbrief",     projectBrief);
   result = result.replace("$projectlogo",      stripPath(projectLogo));

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

  // workaround for issue in LaTeX version 2019, unmaintained tabu package
  startPlainFile("tabu_doxypress.sty");
  m_textStream << ResourceMgr::instance().getAsString("latex/tabu_doxypress.sty");
  endPlainFile();

  startPlainFile("longtable_doxypress.sty");
  m_textStream << ResourceMgr::instance().getAsString("latex/longtable_doxypress.sty");
  endPlainFile();
}

void LatexGenerator::newParagraph()
{
   m_textStream << endl << endl;
}

void LatexGenerator::startParagraph(const QString &className)
{
   (void) className;
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
      m_textStream << latexFilterURL(url);
      m_textStream << "}";
   }

   m_textStream << "{\\texttt{ ";
}

void LatexGenerator::endHtmlLink()
{
   m_textStream << "}}";
}

//void LatexGenerator::writeMailLink(const char *url)
//{
//  if (Config::getBool("latex-hyper-pdf")) {
//    m_textStream << "\\href{mailto:";
//    m_textStream << url;
//    m_textStream << "}";
//  }
//  m_textStream << "\\texttt{ ";
//  docify(url);
//  m_textStream << "}";
//}

void LatexGenerator::writeStartAnnoItem(const QString &, const QString &, const QString &path, const QString &name)
{
   m_textStream << "\\item\\contentsline{section}\\textbf{ ";

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

//void LatexGenerator::writeClassLink(const char *, const char *, const char *,const char *name)
//{
//  m_textStream << "{\\bf ";
//  docify(name);
//  m_textStream << "}";
//}

void LatexGenerator::startTextLink(const QString &file, const QString &anchor)
{
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   if (! m_disableLinks && pdfHyperlinks) {
      m_textStream << "\\mbox{\\hyperlink{";

      if (! file.isEmpty()) {
         m_textStream << stripPath(file);
      }

      if (! anchor.isEmpty()) {
         m_textStream << "_" << anchor;
      }

      m_textStream << "}{";

   } else {
      m_textStream << "\\textbf{ ";
   }
}

void LatexGenerator::endTextLink()
{
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   if (! m_disableLinks && pdfHyperlinks) {
      m_textStream << "}";
   }


   m_textStream << "}";
}

void LatexGenerator::writeObjectLink(const QString &ref, const QString &file, const QString &anchor, const QString &text)
{
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   if (! m_disableLinks && ref.isEmpty() && pdfHyperlinks) {
      m_textStream << "\\mbox{\\hyperlink{";

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
      m_textStream << "}}";

   } else {
      m_textStream << "\\textbf{ ";
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
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static const bool usePDFLatex   = Config::getBool("latex-pdf");
   static const bool latexCompact  = Config::getBool("latex-compact");

   if (usePDFLatex && pdfHyperlinks && ! fileName.isEmpty() ) {
      m_textStream << "\\hypertarget{" << stripPath(fileName) << "}{}";
   }

   if (latexCompact) {
      m_textStream << "\\doxysubsection{";
   } else {
      m_textStream << "\\doxysection{";
   }
}

void LatexGenerator::endTitleHead(const QString &fileName, const QString &name)
{
   m_textStream << "}" << endl;

   if (! name.isEmpty()) {
      m_textStream << "\\label{" << stripPath(fileName) << "}\\index{";
      m_textStream << latexEscapeLabelName(name);

      m_textStream << "@{";
      m_textStream << latexEscapeIndexChars(name);

      m_textStream << "}}" << endl;
   }
}

void LatexGenerator::startTitle()
{
   static const bool latexCompact = Config::getBool("latex-compact");

   if (latexCompact) {
      m_textStream << "\\doxysubsection{";

   } else {
      m_textStream << "\\doxysection{";

   }
}

void LatexGenerator::startGroupHeader(int extraIndentLevel)
{
   static const bool latexCompact = Config::getBool("latex-compact");

   if (latexCompact) {
      ++extraIndentLevel;
   }

   if (extraIndentLevel == 3) {
      m_textStream << "\\doxysubparagraph*{";
   } else if (extraIndentLevel == 2) {
      m_textStream << "\\doxyparagraph{";
   } else if (extraIndentLevel == 1) {
      m_textStream << "\\doxysubsubsection{";
   } else { // extraIndentLevel==0
      m_textStream << "\\doxysubsection{";
   }

   m_disableLinks = true;
}

void LatexGenerator::endGroupHeader(int)
{
   m_textStream << "}" << endl;
   m_disableLinks = false;
}

void LatexGenerator::startMemberHeader(const QString &, int)
{
   static const bool latexCompact = Config::getBool("latex-compact");

   if (latexCompact) {
      m_textStream << "\\doxysubsubsection*{";
   } else {
      m_textStream << "\\doxysubsection*{";
   }

   m_disableLinks = true;
}

void LatexGenerator::endMemberHeader()
{
   m_textStream << "}" << endl;
   m_disableLinks = false;
}

void LatexGenerator::startMemberDoc(const QString &clname, const QString &memname, const QString &,
                  const QString &title, int memCount, int memTotal, bool showInline)
{
   static const QString levelLab[] = {
      "doxysubsubsection",
      "doxyparagraph",
      "doxysubparagraph",
      "doxysubparagraph"
   };

   static const bool compactLatex  = Config::getBool("latex-compact");
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   if (! memname.isEmpty() && ! memname.startsWith('@')) {

      m_textStream << "\\index{";

      if (! clname.isEmpty()) {
         m_textStream << latexEscapeLabelName(clname);
         m_textStream << "@{";

         m_textStream << latexEscapeIndexChars(clname);
         m_textStream << "}!";
      }

      m_textStream << latexEscapeLabelName(memname);
      m_textStream << "@{";

      m_textStream << latexEscapeIndexChars(memname);
      m_textStream << "}}" << endl;
      m_textStream << "\\index{";

      m_textStream << latexEscapeLabelName(memname);
      m_textStream << "@{";

      m_textStream << latexEscapeIndexChars(memname);
      m_textStream << "}";

      if (! clname.isEmpty()) {
         m_textStream << "!";

         m_textStream << latexEscapeLabelName(clname);
         m_textStream << "@{";

         m_textStream << latexEscapeIndexChars(clname);
         m_textStream << "}";
      }

      m_textStream << "}" << endl;
   }


   int level = 0;

   if (showInline) {
      level += 2;
   }

   if (compactLatex) {
      ++level;
   }

   m_textStream << "\\" << levelLab[level];
   m_textStream << "{";

   if (pdfHyperlinks)   {
      m_textStream << "\\texorpdfstring{";
   }

   m_textStream << latexEscapeIndexChars(title);

   if (pdfHyperlinks) {
      m_textStream << "}{" << latexEscapePDFString(title) << "}";
   }
   if (memTotal > 1) {
    m_textStream << "\\hspace{0.1cm}{\\footnotesize\\ttfamily [" << memCount << "/" << memTotal << "]}";
   }
   m_textStream << "}";
   m_textStream << "\n{\\footnotesize\\ttfamily ";
}

void LatexGenerator::endMemberDoc(bool)
{
   m_textStream << "}\n\n";
   m_disableLinks = false;
}

void LatexGenerator::startDoxyAnchor(const QString &fName, const QString &, const QString &anchor,
                  const QString &, const QString &)
{
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static const bool usePDFLatex   = Config::getBool("latex-pdf");

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
   (void) fName;
   (void) anchor;
}

void LatexGenerator::writeAnchor(const QString &fName, const QString &name)
{
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static const bool usePDFLatex   = Config::getBool("latex-pdf");

   m_textStream << "\\label{" << stripPath(name) << "}" << endl;

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

void LatexGenerator::addIndexItemName(const QString &s1, const QString &s2)
{
   if (! s1.isEmpty()) {
      m_textStream << "\\index{";

      m_textStream << latexEscapeLabelName(s1);
      m_textStream << "@{";

      m_textStream << latexEscapeIndexChars(s1);
      m_textStream << "}";

      if (! s2.isEmpty()) {
         m_textStream << "!";

         m_textStream << latexEscapeLabelName(s2);
         m_textStream << "@{";

         m_textStream << latexEscapeIndexChars(s2);
         m_textStream << "}";
      }

      m_textStream << "}";
   }
}

void LatexGenerator::startSection(const QString &lab, const QString &, SectionInfo::SectionType type)
{
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static const bool usePDFLatex   = Config::getBool("latex-pdf");

   if (usePDFLatex && pdfHyperlinks) {
      m_textStream << "\\hypertarget{" << stripPath(lab) << "}{}";
   }

   m_textStream << "\\";

   if (Config::getBool("latex-compact")) {
      switch (type) {
         case SectionInfo::Page:
            m_textStream << "doxysubsection";
            break;

         case SectionInfo::Section:
            m_textStream << "doxysubsubsection";
            break;

         case SectionInfo::Subsection:
            m_textStream << "doxyparagraph";
            break;

         case SectionInfo::Subsubsection:
            m_textStream << "doxysubparagraph";
            break;

         case SectionInfo::Paragraph:
            m_textStream << "doxysubparagraph";
            break;

         default:
            assert(0);
            break;
      }

      m_textStream << "{";

   } else {
      switch (type) {
         case SectionInfo::Page:
            m_textStream << "doxysection";
            break;

         case SectionInfo::Section:
            m_textStream << "doxysubsection";
            break;

         case SectionInfo::Subsection:
            m_textStream << "doxysubsubsection";
            break;

         case SectionInfo::Subsubsection:
            m_textStream << "doxyparagraph";
            break;

         case SectionInfo::Paragraph:
            m_textStream << "doxysubparagraph";
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
   filterLatexString(m_textStream, text, m_insideTabbing, false, false, m_codeGen->usedTableLevel() > 0, false);
}

void LatexGenerator::writeChar(char c)
{
   QString tmp = QChar(c);
   docify(tmp);
}

void LatexGenerator::startClassDiagram()
{
   // if (Config::getBool("compact-latex")) {
   //    m_textStream << "\\doxysubsubsection"; else m_textStream << "\\doxysubsection";
   // }
   // m_textStream << "{";
}

void LatexGenerator::endClassDiagram(const ClassDiagram &d, const QString &fname, const QString &)
{
   d.writeFigure(m_textStream, m_outputDir, fname);
}

void LatexGenerator::startAnonTypeScope(int indent)
{
   if (indent == 0) {
      m_textStream << "\\begin{tabbing}" << endl;
      m_textStream << "xx\\=xx\\=xx\\=xx\\=xx\\=xx\\=xx\\=xx\\=xx\\=\\kill" << endl;
      m_insideTabbing = true;
   }

   m_indent = indent;
}

void LatexGenerator::endAnonTypeScope(int indent)
{
   if (indent == 0) {
      m_textStream << endl << "\\end{tabbing}";
      m_insideTabbing = false;
   }

   m_indent = indent;
}

void LatexGenerator::startMemberTemplateParams()
{
   if (m_templateMemberItem) {
      m_textStream << "{\\footnotesize ";
   }
}

void LatexGenerator::endMemberTemplateParams(const QString &, const QString &)
{
   if (m_templateMemberItem) {
      m_textStream << "}\\\\";
   }
}

void LatexGenerator::startMemberItem(const QString &, int annoType, const QString &, bool)
{

   if (! m_insideTabbing) {
      m_textStream << "\\item " << endl;
      m_templateMemberItem = (annoType == 3);
   }
}

void LatexGenerator::endMemberItem()
{
   if (m_insideTabbing) {
      m_textStream << "\\\\";
   }

   m_textStream << endl;
   m_templateMemberItem = false;
}

void LatexGenerator::startMemberDescription(const QString &, const QString &, bool)
{
   if (! m_insideTabbing) {
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
   if (! m_insideTabbing) {
      m_textStream << "\\end{DoxyCompactList}";
   } else {
      m_textStream << "}\\\\\n";
   }
}

void LatexGenerator::writeNonBreakableSpace(int)
{
   if (m_insideTabbing) {
      m_textStream << "\\>";
   } else {
      m_textStream << "~";
   }
}

void LatexGenerator::startDescTable(const QString &title)
{
   m_codeGen->incUsedTableLevel();
   m_textStream << "\\begin{DoxyEnumFields}{" << title << "}" << endl;
}

void LatexGenerator::endDescTable()
{
   m_codeGen->decUsedTableLevel();
   m_textStream << "\\end{DoxyEnumFields}" << endl;
}

void LatexGenerator::startDescTableRow()
{
   // this is needed to prevent the \hypertarget, \label, and \index commands from messing up
   // the row height

   m_textStream << "\\raisebox{\\heightof{T}}[0pt][0pt]{";
}

void LatexGenerator::endDescTableRow()
{
}

void LatexGenerator::startDescTableTitle()
{
   m_textStream << "}";
}

void LatexGenerator::endDescTableTitle()
{
}

void LatexGenerator::startDescTableData()
{
   m_textStream << "&";
}

void LatexGenerator::endDescTableData()
{
   m_textStream << "\\\\\n\\hline\n" << endl;
}

void LatexGenerator::lastIndexPage()
{
}

void LatexGenerator::startMemberList()
{
   if (! m_insideTabbing) {
      m_textStream << "\\begin{DoxyCompactItemize}" << endl;
   }
}

void LatexGenerator::endMemberList()
{
   if (! m_insideTabbing) {
      m_textStream << "\\end{DoxyCompactItemize}"   << endl;
   }
}

void LatexGenerator::startMemberGroupHeader(bool hasHeader)
{
   if (hasHeader) {
      m_textStream << "\\begin{Indent}";
   }

   m_textStream << "\\textbf{ ";
}

void LatexGenerator::endMemberGroupHeader()
{
   m_textStream << "}\\par" << endl;

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
   g.writeGraph(m_textStream, GOF_EPS, EOF_LaTeX, m_outputDir, m_fileName, m_relPath);
}

void LatexGenerator::startInclDepGraph()
{
}

void LatexGenerator::endInclDepGraph(const DotInclDepGraph &g)
{

   g.writeGraph(m_textStream, GOF_EPS, EOF_LaTeX, m_outputDir, m_fileName, m_relPath);
}

void LatexGenerator::startGroupCollaboration()
{
}

void LatexGenerator::endGroupCollaboration(const DotGroupCollaboration &g)
{
   g.writeGraph(m_textStream, GOF_EPS, EOF_LaTeX, m_outputDir, m_fileName, m_relPath);
}

void LatexGenerator::startCallGraph()
{
}

void LatexGenerator::endCallGraph(const DotCallGraph &g)
{
   g.writeGraph(m_textStream, GOF_EPS, EOF_LaTeX, m_outputDir, m_fileName, m_relPath);
}

void LatexGenerator::startDirDepGraph()
{
}

void LatexGenerator::endDirDepGraph(const DotDirDeps &g)
{
   g.writeGraph(m_textStream, GOF_EPS, EOF_LaTeX, m_outputDir, m_fileName, m_relPath);
}

void LatexGenerator::startDescription()
{
   m_textStream << "\\begin{description}" << endl;
}

void LatexGenerator::endDescription()
{
   m_textStream << "\\end{description}" << endl;
   m_firstDescItem = true;
}

void LatexGenerator::startDescItem()
{
   m_textStream << "\\item[";
   m_firstDescItem = true;
}

void LatexGenerator::endDescItem()
{
   if (m_firstDescItem) {
      m_textStream << "]" << endl;
      m_firstDescItem = false;
   } else {
      lineBreak();
   }
}

void LatexGenerator::startSimpleSect(SectionTypes, const QString &file, const QString &anchor, const QString &title)
{
   m_textStream << "\\begin{Desc}\n\\item[";

   if (! file.isEmpty()) {
      writeObjectLink(QString(), file, anchor, title);
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
   if (openBracket) {
      m_textStream << "(";
   }

   m_textStream << "\\begin{DoxyParamCaption}";
}

void LatexGenerator::endParameterList()
{
}

void LatexGenerator::startParameterType(bool first, const QString &key)
{
   m_textStream << "\\item[{";

   if (! first && ! key.isEmpty()) {
      docify(key);
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
   m_textStream << "}";

   if (last) {
      m_textStream << "\\end{DoxyParamCaption}";

      if (closeBracket) {
         m_textStream << ")";
      }
   }
}

void LatexGenerator::exceptionEntry(const QString &prefix, bool closeBracket)
{
   if (! prefix.isEmpty()) {
      m_textStream << " " << prefix << "(";

   } else if (closeBracket) {
      m_textStream << ")";

   }

   m_textStream << " ";
}

void LatexGenerator::writeDoc(DocNode *n, QSharedPointer<Definition> ctx, QSharedPointer<MemberDef> )
{
   LatexDocVisitor *visitor = new LatexDocVisitor(m_textStream, *m_codeGen, ctx ? ctx->getDefFileExtension()
      : QString(), m_insideTabbing);

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

void LatexGenerator::startInlineHeader()
{
   static const bool latexCompact = Config::getBool("latex-compact");

   if (latexCompact) {
      m_textStream << "\\doxyparagraph*{";
   } else {
      m_textStream << "\\doxysubsubsection*{";
   }
}

void LatexGenerator::endInlineHeader()
{
   m_textStream << "}" << endl;
}

void LatexGenerator::lineBreak(const QString &style)
{
   (void) style;

   if (m_insideTabbing) {
      m_textStream << "\\\\\n";

   } else {
      m_textStream << "\\newline\n";

   }
}

void LatexGenerator::startMemberDocSimple(bool isEnum)
{
   m_codeGen->incUsedTableLevel();

   if (isEnum) {
      m_textStream << "\\begin{DoxyEnumFields}{";
      docify(theTranslator->trEnumerationValues());

   } else {
      m_textStream << "\\begin{DoxyFields}{";
      docify(theTranslator->trCompoundMembers());
   }

   m_textStream << "}" << endl;
}

void LatexGenerator::endMemberDocSimple(bool isEnum)
{
   m_codeGen->decUsedTableLevel();

   if (isEnum) {
      m_textStream << "\\end{DoxyEnumFields}" << endl;
   } else {
      m_textStream << "\\end{DoxyFields}" << endl;
   }
}

void LatexGenerator::startInlineMemberType()
{
   // to prevent \+ from causing unwanted breaks
   m_insideTabbing = true;
}

void LatexGenerator::endInlineMemberType()
{
   m_textStream << "&" << endl;
   m_insideTabbing = false;
}

void LatexGenerator::startInlineMemberName()
{
   // to prevent \+ from causing unwanted breaks
   m_insideTabbing = true;
}

void LatexGenerator::endInlineMemberName()
{
   m_textStream << "&" << endl;
   m_insideTabbing = false;
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

void LatexGenerator::writeLabel(const QString &label, bool isLast)
{
   m_textStream << "{\\ttfamily [" << latexEscapeLabelName(label) << "]}";

   if (! isLast) {
      m_textStream << ", ";
   }
}

void LatexGenerator::endLabels()
{
}

