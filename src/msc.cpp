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

#include <QDir>
#include <QTextStream>

#include <config.h>
#include <docparser.h>
#include <doxy_globals.h>
#include <message.h>
#include <msc.h>
#include <portable.h>
#include <util.h>

static const int maxCmdLine = 40960;

static bool convertMapFile(QTextStream &t, const char *mapName, const QByteArray relPath, const QByteArray &context)
{
   QFile f(mapName);

   if (! f.open(QIODevice::ReadOnly)) {
      err("failed to open map file %s for inclusion in the docs\n"
          "If you installed Graphviz/dot after a previous failing run, \n"
          "try deleting the output directory and rerun doxygen.\n", mapName);
      return false;
   }

   const int maxLineLen = 1024;
   char buf[maxLineLen];
   char url[maxLineLen];
   char ref[maxLineLen];
   int x1, y1, x2, y2;

   while (!f.atEnd()) {
      bool isRef = false;
      int numBytes = f.readLine(buf, maxLineLen);
      buf[numBytes - 1] = '\0';
      //printf("ReadLine `%s'\n",buf);
      if (qstrncmp(buf, "rect", 4) == 0) {
         // obtain the url and the coordinates in the order used by graphviz-1.5
         sscanf(buf, "rect %s %d,%d %d,%d", url, &x1, &y1, &x2, &y2);

         if (qstrcmp(url, "\\ref") == 0 || qstrcmp(url, "@ref") == 0) {
            isRef = true;
            sscanf(buf, "rect %s %s %d,%d %d,%d", ref, url, &x1, &y1, &x2, &y2);
         }

         // sanity checks
         if (y2 < y1) {
            int temp = y2;
            y2 = y1;
            y1 = temp;
         }
         if (x2 < x1) {
            int temp = x2;
            x2 = x1;
            x1 = temp;
         }

         t << "<area href=\"";

         if ( isRef ) {
            // handle doxygen \ref tag URL reference
            DocRef *df = new DocRef( (DocNode *) 0, url, context );
            t << externalRef(relPath, df->ref(), true);
            if (!df->file().isEmpty()) {
               t << df->file() << Doxygen::htmlFileExtension;
            }
            if (!df->anchor().isEmpty()) {
               t << "#" << df->anchor();
            }
            delete df;
         } else {
            t << url;
         }
         t << "\" shape=\"rect\" coords=\""
           << x1 << "," << y1 << "," << x2 << "," << y2 << "\""
           << " alt=\"\"/>" << endl;
      }
   }

   return true;
}

void writeMscGraphFromFile(const QString &inFile, const QString &outDir, const QString &outFile, MscOutputFormat format)
{
   QString absOutFile = outDir + QDir::separator() + outFile; 
 
   // chdir to the output dir, so dot can find the font file.
   QString oldDir = QDir::currentPath();

   // go to the html output directory (i.e. path)
   QDir::setCurrent(outDir);
   
   QString mscExe = Config::getString("mscgen-path") + "mscgen" + portable_commandExtension();

   QByteArray mscArgs;
   QByteArray extension;

   switch (format) {
      case MSC_BITMAP:
         mscArgs += "-T png";
         extension = ".png";
         break;

      case MSC_EPS:
         mscArgs += "-T eps";
         extension = ".eps";
         break;

      case MSC_SVG:
         mscArgs += "-T svg";
         extension = ".svg";
         break;

      default:
         QDir::setCurrent(oldDir);
         return;
   }

   mscArgs += " -i \"";
   mscArgs += inFile;

   mscArgs += "\" -o \"";
   mscArgs += outFile;
   mscArgs += extension + "\"";

   int exitCode;
   
   portable_sysTimerStart();
   if ((exitCode = portable_system(mscExe, mscArgs, false)) != 0) {
      portable_sysTimerStop();
     
      QDir::setCurrent(oldDir);
      return;
   }

   portable_sysTimerStop();

   if ( (format == MSC_EPS) && (Config::getBool("latex-pdf")) ) {

      QString epstopdfArgs;
      epstopdfArgs = QString("\"%1.eps\" --outfile=\"%2.pdf\"").arg(outFile).arg(outFile);

      portable_sysTimerStart();
      if (portable_system("epstopdf", epstopdfArgs) != 0) {
         err("Problem running epstopdf. Check your TeX installation\n");
      }

      portable_sysTimerStop();
   }

   QDir::setCurrent(oldDir);
}

QByteArray getMscImageMapFromFile(const QByteArray &inFile, const QByteArray &outDir,
                                  const QByteArray &relPath, const QByteArray &context)
{
   QByteArray outFile = inFile + ".map";

   //printf("*** running:getMscImageMapFromFile \n");
   // chdir to the output dir, so dot can find the font file.
   QByteArray oldDir = QDir::currentPath().toUtf8();

   // go to the html output directory (i.e. path)
   QDir::setCurrent(outDir);
   //printf("Going to dir %s\n",QDir::currentPath().data());

   QString mscExe = Config::getString("mscgen-path") + "mscgen" + portable_commandExtension();

   QByteArray mscArgs = "-T ismap -i \"";
   mscArgs += inFile;   

   mscArgs += "\" -o \"";
   mscArgs += outFile + "\"";

   int exitCode;
   portable_sysTimerStart();
   if ((exitCode = portable_system(mscExe, mscArgs, false)) != 0) {
      portable_sysTimerStop();
      QDir::setCurrent(oldDir);
      return "";
   }
   portable_sysTimerStop();

   QByteArray result;
   QTextStream tmpout(&result);

   convertMapFile(tmpout, outFile, relPath, context);
   QDir().remove(outFile);

   QDir::setCurrent(oldDir);
   return result.data();
}

void writeMscImageMapFromFile(QTextStream &t, const QByteArray &inFile, const QByteArray &outDir,
                              const QByteArray &relPath, const QByteArray &baseName, const QByteArray &context,
                              MscOutputFormat format)
{
   QByteArray mapName = baseName + ".map";
   QByteArray mapFile = inFile + ".map";

   t << "<img src=\"" << relPath << baseName << ".";
   switch (format) {
      case MSC_BITMAP:
         t << "png";
         break;
      case MSC_EPS:
         t << "eps";
         break;
      case MSC_SVG:
         t << "svg";
         break;
      default:
         t << "unknown";
   }
   t << "\" alt=\""
     << baseName << "\" border=\"0\" usemap=\"#" << mapName << "\"/>" << endl;
   QByteArray imap = getMscImageMapFromFile(inFile, outDir, relPath, context);
   t << "<map name=\"" << mapName << "\" id=\"" << mapName << "\">" << imap << "</map>" << endl;
}

