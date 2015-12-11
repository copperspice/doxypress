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
const QString bibTmpFile("bibTmpFile_");
const QString bibTmpDir("bibTmpDir/");

CiteDict::CiteDict() : m_entries()
{  
}

void CiteDict::writeLatexBibliography(QTextStream &t)
{
   if (m_entries.isEmpty()) {
      return;
   }

   QString style = Config::getString("latex-bib-style");
 
   QString unit;
   if (Config::getBool("latex-compact")) {
      unit = "section";
   } else {
      unit = "chapter";
   }

   t << "% Bibliography\n"  "\\newpage\n" "\\phantomsection\n";
   bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");

   if (! pdfHyperlinks) {
      t << "\\clearemptydoublepage\n";
      t << "\\addcontentsline{toc}{" << unit << "}{" << theTranslator->trCiteReferences() << "}\n";
   }

   t << "\\bibliographystyle{" << style << "}\n" "\\bibliography{";
   const QStringList citeDataList = Config::getList("cite-bib-files");

   int i = 0;

   for (auto bibdata : citeDataList) {

      QString bibFile = bibdata;

      // file can now have multiple dots
      if (! bibFile.isEmpty() && bibFile.right(4) != ".bib") {
         bibFile += ".bib";
      }

      QFileInfo fi(bibFile);
      if (fi.exists()) {
         if (! bibFile.isEmpty()) {
            if (i) {
               t << ",";
            }
            i++;
            t << bibTmpFile << QString().setNum(i);
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
   m_entries.insert(label, CiteInfo(label));
}

CiteInfo *CiteDict::find(const QString &label) const
{
   if (! label.isEmpty())  {

      QHash<QString, CiteInfo>::const_iterator entry = m_entries.find(label);

      if (entry == m_entries.end()) {
        return 0;

      } else {
        return const_cast<CiteInfo *>(&entry.value());

      }

   } else {
      return 0;

   }
}

void CiteDict::clear()
{
   m_entries.clear();
}

bool CiteDict::isEmpty() const
{
   const QStringList citeBibFiles = Config::getList("cite-bib-files");
   return (citeBibFiles.count() == 0 || m_entries.isEmpty());
}

void CiteDict::generatePage() const
{ 
   // do not generate an empty citations page
   if (isEmpty()) {
      return;
   }

   // generate file with markers and citations to output-dir
   QFile f;

   QString outputDir = Config::getString("output-dir");
   QString citeListFile = outputDir + "/citelist.doc";

   f.setFileName(citeListFile);

   if ( ! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", qPrintable(citeListFile), f.error());
   }

   QTextStream t(&f);
   t << "<!-- BEGIN CITATIONS -->" << endl;
   t << "<!--" << endl;

   for (auto item : m_entries)  {
      t << "\\citation{" << item.label << "}" << endl;
   }

   t << "-->" << endl;
   t << "<!-- END CITATIONS -->" << endl;
   t << "<!-- BEGIN BIBLIOGRAPHY -->" << endl;
   t << "<!-- END BIBLIOGRAPHY -->" << endl;
   f.close();

   // generate bib2xhtml
   QString bib2xhtmlFile  = outputDir + "/bib2xhtml.pl";
   ResourceMgr::instance().copyResourceAs("html/bib2xhtml.pl", outputDir, "bib2xhtml.pl");

   // generate bst
   QString doxy_BstFile = outputDir + "/doxypress.bst";
   ResourceMgr::instance().copyResourceAs("html/doxypress.bst", outputDir, "doxypress.bst");

   // for all formats we just copy the bib files to as special output directory
   // so bibtex can find them without path (bibtex doesn't support paths or
   // filenames with spaces) Strictly not required when only latex is generated

   const QStringList citeDataList = Config::getList("cite-bib-files");

   QString bibOutputDir   = outputDir + "/" + bibTmpDir;
   QString bibOutputFiles = "";

   QDir thisDir;

   thisDir.mkdir(bibOutputDir);
   int i = 0;

   for (auto bibFile : citeDataList ) {   
     
      if (! bibFile.isEmpty() && bibFile.right(4) != ".bib") {
         bibFile += ".bib";
      }

      QFileInfo fi(bibFile);

      if (fi.exists()) {
         if (! bibFile.isEmpty()) {
            ++i;

            copyFile(bibFile, bibOutputDir + bibTmpFile + QString().setNum(i) + ".bib");
            bibOutputFiles = bibOutputFiles + " " + bibTmpDir + bibTmpFile + QString().setNum(i) + ".bib";
         }

      } else if (! fi.exists()) {
         err("Bib file %s was not found\n", qPrintable(bibFile));
      }
   }

   QString oldDir = QDir::currentPath();
   QDir::setCurrent(outputDir);

   // 5. run bib2xhtml perl script on the generated file which will insert the
   //    bibliography in citelist.doc
   
   portable_sysTimerStop();

   QString temp = "\"" + bib2xhtmlFile + "\" " + bibOutputFiles + " \"" + citeListFile + "\"";
   int exitCode = portable_system("perl", temp); 

   if (exitCode != 0) {
      err("Unable to run BibTeX. Verify the command 'perl --version' works from the command line. Exit code: %d\n", exitCode);
   }
   portable_sysTimerStop();

   QDir::setCurrent(oldDir);

   // 6. read back the file
   f.setFileName(citeListFile);

   if (! f.open(QIODevice::ReadOnly)) {
     err("Unable to open file for reading %s, error: %d\n", qPrintable(citeListFile), f.error()); 
   }

   bool insideBib = false;

   QString doc;
   QFileInfo fi(citeListFile);

   QString input;
   input = f.readAll();

   f.close();

   int p = 0;
   int s;


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
      if (insideBib && (i = line.indexOf("<a name=\"CITEREF_")) != -1) {
         int j = line.indexOf("\">[");
         int k = line.indexOf("]</a>");

         if (j != -1 && k != -1) {
            QString label  = line.mid(i + 17, j - i - 17);
            QString number = line.mid(j + 2, k - j - 1);

            CiteInfo *ci = this->find(label);
            
            if (ci) {
               ci->text = number;
            }
         }
      }
   }
   
   // 7. add it as a page
   addRelatedPage(CiteConsts::fileName, theTranslator->trCiteReferences(), doc, 0, CiteConsts::fileName, 1, 0, 
                  QSharedPointer<GroupDef>(), 0);

   // 8. for latex we just copy the bib files to the output and let
   //    latex do this work.

   if (Config::getBool("generate-latex")) {
      // copy bib files to the latex output dir

      const QStringList citeDataList = Config::getList("cite-bib-files");
      QString latexOutputDir = Config::getString("latex-output") + "/";

      int i = 0;

      for (auto bibFile : citeDataList ) {
         // Note: file can now have multiple dots

         if (! bibFile.isEmpty() && bibFile.right(4) != ".bib") {
            bibFile += ".bib";
         }

         QFileInfo fi(bibFile);
         if (fi.exists()) {
            if (!bibFile.isEmpty()) {
               // bug_700510, multile times the same name were overwriting; creating new names
               // also for names with spaces
               ++i;
               copyFile(bibFile, latexOutputDir + bibTmpFile + QString().setNum(i) + ".bib");
            }

         } else {
            err("Bib file %s was not found\n", qPrintable(bibFile));

         }

      }
   }

   // 9. Remove temporary files
   thisDir.remove(citeListFile);
   thisDir.remove(doxy_BstFile);
   thisDir.remove(bib2xhtmlFile);

   // we might try to remove too many files as empty files didn't get a coresponding new file
   // but the remove function does not emit an error for it and we don't catch the error return
   // so no problem.

   for (unsigned int j = 1; j <= citeDataList.count(); j++) {
      thisDir.remove(bibOutputDir + bibTmpFile + QString().setNum(j) + ".bib");
   }

   thisDir.rmdir(bibOutputDir);
}

