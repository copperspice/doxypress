/************************************************************************
*
* Copyright (C) 2014-2019 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
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
#include <QFile>

#include <cite.h>

#include <config.h>
#include <language.h>
#include <message.h>
#include <portable.h>
#include <resourcemgr.h>
#include <util.h>

const QString CiteConsts::fileName("citelist");
const QString CiteConsts::anchorPrefix("CITEREF_");
const QString bibTmpFile("bibFile_");
const QString bibTmpDir(QString("bibTmp") + QDir::separator());

CiteDict::CiteDict()
{
}

void CiteDict::writeLatexBibliography(QTextStream &t)
{
   if (m_entries.isEmpty()) {
      return;
   }

   static const QString style            = Config::getString("latex-bib-style");
   static const bool latexCompact        = Config::getBool("latex-compact");
   static const bool pdfHyperlinks       = Config::getBool("latex-hyper-pdf");
   static const QStringList citeDataList = Config::getList("cite-bib-files");

   QString unit;

   if (latexCompact) {
      unit = "section";
   } else {
      unit = "chapter";
   }

   t << "% Bibliography\n"  "\\newpage\n" "\\phantomsection\n";

   if (! pdfHyperlinks) {
      t << "\\clearemptydoublepage\n";
      t << "\\addcontentsline{toc}{" << unit << "}{" << theTranslator->trCiteReferences() << "}\n";
   }

   t << "\\bibliographystyle{" << style << "}\n" "\\bibliography{";

   //
   int i = 0;

   for (auto bibdata : citeDataList) {
      QString bibFile = bibdata;

      // file can have multiple dots
      if (! bibFile.isEmpty() && ! bibFile.endsWith(".bib")) {
         bibFile += ".bib";
      }

      QFileInfo fi(bibFile);
      if (fi.exists()) {
         if (! bibFile.isEmpty()) {
            if (i) {
               t << ",";
            }

            t << bibTmpFile << QString::number(i);
            i++;

         }
      }

   }

   t << "}\n";

   if (pdfHyperlinks) {
      t << "\\addcontentsline{toc}{" << unit << "}{" << theTranslator->trCiteReferences() << "}\n";
   }

   t << "\n";
}

void CiteDict::insert(const QString &label)
{
   m_entries.insert(label, "");
}

QString CiteDict::find(const QString &label) const
{
   if (label.isEmpty())  {
      return QString("");

   } else {
      auto entry = m_entries.find(label);

      if (entry == m_entries.end()) {
        return QString("");

      } else {
        return entry.value();

      }
   }
}

void CiteDict::clear()
{
   m_entries.clear();
}

bool CiteDict::isEmpty() const
{
   static const QStringList citeBibFiles = Config::getList("cite-bib-files");

   return (citeBibFiles.count() == 0 || m_entries.isEmpty());
}

void CiteDict::generatePage() const
{
   // do not generate empty citations page
   if (isEmpty()) {
      return;
   }

   static const QString outputDir        = Config::getString("output-dir");
   static const QStringList citeDataList = Config::getList("cite-bib-files");
   static const bool generateLatex       = Config::getBool("generate-latex");
   static const QString latexOutput      = Config::getString("latex-output");

   // generate file with markers and citations to outputDir
   QFile f;

   QString citeListFile = outputDir + "/citelist.doc";
   f.setFileName(citeListFile);

   if ( ! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", csPrintable(citeListFile), f.error());
   }

   QTextStream t(&f);
   t << "<!-- BEGIN CITATIONS -->" << endl;
   t << "<!--" << endl;

   for (auto iter = m_entries.begin(); iter != m_entries.end(); ++iter)  {
      t << "\\citation{" << iter.key() << "}" << endl;
   }

   t << "-->" << endl;
   t << "<!-- END CITATIONS -->" << endl;
   t << "<!-- BEGIN BIBLIOGRAPHY -->" << endl;
   t << "<!-- END BIBLIOGRAPHY -->" << endl;
   f.close();

   // generate bib2xhtml
   QString bib2xhtmlFile = outputDir + "/bib2xhtml.pl";
   ResourceMgr::instance().copyResourceAs("html/bib2xhtml.pl", outputDir, "bib2xhtml.pl");

   // generate bst
   QString doxy_BstFile = outputDir + "/doxypress.bst";
   ResourceMgr::instance().copyResourceAs("html/doxypress.bst", outputDir, "doxypress.bst");

   // for all formats the bib files are copied to a temp output directory so bibtex can find the files
   // (bibtex does not support paths or filenames with spaces)

   QString bibOutputDir   = outputDir + QDir::separator() + bibTmpDir;
   QString bibOutputFiles = "";

   QDir thisDir;

   thisDir.mkdir(bibOutputDir);
   int i = 0;

   for (auto bibFile : citeDataList) {
      if (! bibFile.isEmpty() && ! bibFile.endsWith(".bib")) {
         bibFile += ".bib";
      }

      QFileInfo fi(bibFile);

      if (fi.exists()) {
         if (! bibFile.isEmpty()) {

            copyFile(bibFile, bibOutputDir + bibTmpFile + QString::number(i) + ".bib");
            bibOutputFiles = bibOutputFiles + " " + bibTmpDir + bibTmpFile + QString::number(i) + ".bib";

            ++i;
         }

      } else if (! fi.exists()) {
         err("Bib file %s was not found\n", csPrintable(bibFile));
      }
   }

   QString oldDir = QDir::currentPath();
   QDir::setCurrent(outputDir);

   // 5. run bib2xhtml perl script on the generated file which will insert the bibliography into citelist.doc
   QString curDir = QDir::currentPath() + QDir::separator();

   // perl "bib2xhtml.pl" curDir  bibOutputFiles  "citeList.doc"
   QString args  = "\"" + bib2xhtmlFile + "\" " +  curDir + " " + bibOutputFiles + " \"" + citeListFile + "\"";
   int exitCode  = portable_system("perl", args);

   if (exitCode == 1) {
      errAll("Issue with Perl or BibTeX. Verify 'perl --version' works from a Windows or Shell command line.\n\n");

   } else if (exitCode != 0) {
      errAll("Issue with Perl or BibTeX. \n       Verify messages appearing afer 'Generating citations page' in your "
                  "DoxyPress UI output. Exit Code: %d\n\n", exitCode);
   }

   QDir::setCurrent(oldDir);

   // 6. read the contents of the generated citeList file
   f.setFileName(citeListFile);

   if (! f.open(QIODevice::ReadOnly)) {
     err("Unable to open file for reading %s, error: %d\n", csPrintable(citeListFile), f.error());
   }

   QByteArray data = f.readAll();
   f.close();

   QString doc;
   QString input = QString::fromUtf8(data);

   int p = 0;
   int s;

   bool insideBib = false;

   while ((s = input.indexOf('\n', p)) != -1) {
      QString line = input.mid(p, s - p);

      p = s + 1;

      if (line.indexOf("<!-- BEGIN BIBLIOGRAPHY") != -1) {
         insideBib = true;

      } else if (line.indexOf("<!-- END BIBLIOGRAPH") != -1) {
         insideBib = false;

      } else if (insideBib) {
         doc += line + "\n";
      }

      int i;

      // determine text to use at the location of the @cite command
      if (insideBib && (i = line.indexOf("name=\"CITEREF_")) != -1) {
         int j = line.indexOf("\">[");
         int k = line.indexOf("]</a>");

         if (j != -1 && k != -1) {
            QString label  = line.mid(i + 14, j - i - 14);
            QString number = line.mid(j + 2, k - j - 1);

            auto iter = m_entries.find(label);

            if (iter != m_entries.end()) {
               iter.value() = number;
            }
         }
      }
   }

   // 7. add bibliography a new page
   addRelatedPage(CiteConsts::fileName, theTranslator->trCiteReferences(), doc, CiteConsts::fileName, 1,
                  QVector<ListItemInfo>(), QSharedPointer<GroupDef>());

   // 8. for latex copy the bib files to the output and let latex do this work
   if (generateLatex) {
      QString latexOutputDir = outputDir + QDir::separator() + latexOutput + QDir::separator();
      int i = 0;

      for (auto bibFile : citeDataList)  {
         // file can have multiple dots

         if (! bibFile.isEmpty() && ! bibFile.endsWith(".bib")) {
            bibFile += ".bib";
         }

         QFileInfo fi(bibFile);

         if (fi.exists()) {
            if (! bibFile.isEmpty()) {
               // suffix added to accomodate files with the same name
               copyFile(bibFile, latexOutputDir + bibTmpFile + QString::number(i) + ".bib");
               ++i;
            }

         } else {
            err("Bib file %s was not found\n", csPrintable(bibFile));

         }
      }
   }

   // remove temporary files
   thisDir.remove(citeListFile);
   thisDir.remove(doxy_BstFile);
   thisDir.remove(bib2xhtmlFile);

   for (int j = 0; j < citeDataList.count(); j++) {
      thisDir.remove(bibOutputDir + bibTmpFile + QString::number(j) + ".bib");
   }

   thisDir.rmdir(bibOutputDir);
}
