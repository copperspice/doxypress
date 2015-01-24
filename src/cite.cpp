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
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#include <QDir>
#include <QFile>

#include <cite.h>
#include <portable.h>
#include <config.h>
#include <message.h>
#include <util.h>
#include <language.h>
#include <resourcemgr.h>

const QByteArray CiteConsts::fileName("citelist");
const QByteArray CiteConsts::anchorPrefix("CITEREF_");
const QByteArray bibTmpFile("bibTmpFile_");
const QByteArray bibTmpDir("bibTmpDir/");

CiteDict::CiteDict() : m_entries()
{  
}

void CiteDict::writeLatexBibliography(QTextStream &t)
{
   if (m_entries.isEmpty()) {
      return;
   }

   QByteArray style = Config_getString("LATEX_BIB_STYLE");
   if (style.isEmpty()) {
      style = "plain";
   }

   QByteArray unit;
   if (Config_getBool("COMPACT_LATEX")) {
      unit = "section";
   } else {
      unit = "chapter";
   }

   t << "% Bibliography\n"  "\\newpage\n" "\\phantomsection\n";
   bool pdfHyperlinks = Config_getBool("PDF_HYPERLINKS");

   if (!pdfHyperlinks) {
      t << "\\clearemptydoublepage\n";
      t << "\\addcontentsline{toc}{" << unit << "}{" << theTranslator->trCiteReferences() << "}\n";
   }

   t << "\\bibliographystyle{" << style << "}\n" "\\bibliography{";
   QStringList &citeDataList = Config_getList("CITE_BIB_FILES");

// QByteArray latexOutputDir = Config_getString("LATEX_OUTPUT") + "/";

   int i = 0;

   for (auto bibdata : citeDataList) {

      QByteArray bibFile = bibdata.toLatin1();

      // Note: file can now have multiple dots
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

void CiteDict::insert(const char *label)
{
   m_entries.insert(label, CiteInfo(label));
}

CiteInfo *CiteDict::find(const char *label) const
{
   if (label)  {

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
   QStringList &citeBibFiles = Config_getList("CITE_BIB_FILES");
   return (citeBibFiles.count() == 0 || m_entries.isEmpty());
}

void CiteDict::generatePage() const
{
   //printf("** CiteDict::generatePage() count=%d\n",m_ordering.count());

   // do not generate an empty citations page
   if (isEmpty()) {
      return;   // nothing to cite
   }

   // 1. generate file with markers and citations to OUTPUT_DIRECTORY
   QFile f;

   QByteArray outputDir    = Config_getString("OUTPUT_DIRECTORY");
   QByteArray citeListFile = outputDir + "/citelist.doc";
   f.setFileName(citeListFile);

   if ( ! f.open(QIODevice::WriteOnly)) {
      err("could not open file %s for writing\n", citeListFile.data());
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

   // 2. generate bib2xhtml
   QByteArray bib2xhtmlFile  = outputDir + "/bib2xhtml.pl";
   ResourceMgr::instance().copyResourceAs("html/bib2xhtml.pl", outputDir, "bib2xhtml.pl");

   // 3. generate doxygen.bst
   QByteArray doxygenBstFile = outputDir + "/doxygen.bst";
   ResourceMgr::instance().copyResourceAs("html/doxygen.bst", outputDir, "doxygen.bst");

   // 4. for all formats we just copy the bib files to as special output directory
   //    so bibtex can find them without path (bibtex doesn't support paths or
   //    filenames with spaces!)
   //    Strictly not required when only latex is generated
   QStringList &citeDataList = Config_getList("CITE_BIB_FILES");
   QByteArray bibOutputDir = outputDir + "/" + bibTmpDir;
   QByteArray bibOutputFiles = "";
   QDir thisDir;

   thisDir.mkdir(bibOutputDir);

   for (auto bibdata : citeDataList ) {
      int i = 0;

      QByteArray bibFile = bibdata.toLatin1();

      if (! bibFile.isEmpty() && bibFile.right(4) != ".bib") {
         bibFile += ".bib";
      }

      QFileInfo fi(bibFile);

      if (fi.exists()) {
         if (! bibFile.isEmpty()) {
            ++i;
            copyFile(bibFile, bibOutputDir + bibTmpFile + QByteArray().setNum(i) + ".bib");
            bibOutputFiles = bibOutputFiles + " " + bibTmpDir + bibTmpFile + QByteArray().setNum(i) + ".bib";
         }

      } else if (!fi.exists()) {
         err("Bib file %s was not found\n", bibFile.data());
      }
   }

   QString oldDir = QDir::currentPath();
   QDir::setCurrent(outputDir);

   // 5. run bib2xhtml perl script on the generated file which will insert the
   //    bibliography in citelist.doc
   int exitCode;
   portable_sysTimerStop();
   if ((exitCode = portable_system("perl", "\"" + bib2xhtmlFile + "\" " + bibOutputFiles + " \"" +
                                   citeListFile + "\"")) != 0) {
      err("Problems running bibtex. Verify that the command 'perl --version' works from the command line. Exit code: %d\n",
          exitCode);
   }
   portable_sysTimerStop();

   QDir::setCurrent(oldDir);

   // 6. read back the file
   f.setFileName(citeListFile);

   if (! f.open(QIODevice::ReadOnly)) {
      err("could not open file %s for reading\n", citeListFile.data());
   }

   bool insideBib = false;

   QByteArray doc;
   QFileInfo fi(citeListFile);

   QByteArray input;
   input = f.readAll();

   f.close();

   int p = 0;
   int s;

   // printf("input=[%s]\n",input.data());

   while ((s = input.indexOf('\n', p)) != -1) {
      QByteArray line = input.mid(p, s - p);
      // printf("p=%d s=%d line=[%s]\n",p,s,line.data());

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
            QByteArray label  = line.mid(i + 17, j - i - 17);
            QByteArray number = line.mid(j + 2, k - j - 1);

            CiteInfo *ci = this->find(label);

            //printf("label='%s' number='%s' => %p\n",label.data(),number.data(),ci);
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
   if (Config_getBool("GENERATE_LATEX")) {
      // copy bib files to the latex output dir
      QStringList &citeDataList = Config_getList("CITE_BIB_FILES");
      QByteArray latexOutputDir = Config_getString("LATEX_OUTPUT") + "/";

      int i = 0;

      for (auto bibdata : citeDataList ) {
         QByteArray bibFile = bibdata.toLatin1();
         // Note: file can now have multiple dots

         if (!bibFile.isEmpty() && bibFile.right(4) != ".bib") {
            bibFile += ".bib";
         }

         QFileInfo fi(bibFile);
         if (fi.exists()) {
            if (!bibFile.isEmpty()) {
               // bug_700510, multile times the same name were overwriting; creating new names
               // also for names with spaces
               ++i;
               copyFile(bibFile, latexOutputDir + bibTmpFile + QByteArray().setNum(i) + ".bib");
            }

         } else {
            err("Bib file %s was not found\n", bibFile.data());
         }

      }
   }

   // 9. Remove temporary files
   thisDir.remove(citeListFile);
   thisDir.remove(doxygenBstFile);
   thisDir.remove(bib2xhtmlFile);
   // we might try to remove too many files as empty files didn't get a coresponding new file
   // but the remove function does not emit an error for it and we don't catch the error return
   // so no problem.
   for (unsigned int j = 1; j <= citeDataList.count(); j++) {
      thisDir.remove(bibOutputDir + bibTmpFile + QByteArray().setNum(j) + ".bib");
   }
   thisDir.rmdir(bibOutputDir);
}

