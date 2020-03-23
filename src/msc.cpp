/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
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
#include <QTextStream>

#include <config.h>
#include <docparser.h>
#include <doxy_globals.h>
#include <index.h>
#include <message.h>
#include <msc.h>
#include <portable.h>
#include <util.h>

static const int maxCmdLine = 40960;

static bool convertMapFile(QTextStream &t, const QString &mapName, const QString &relPath, const QString &context)
{
   static int logCount = 0;
   QFile f(mapName);

   if (! f.open(QIODevice::ReadOnly)) {

      if (logCount < 5) {
         logCount++;

         errNoPrefixAll("\n");
         errAll("Unable to open dot map file %s\n"
                  "If dot was installed after a previous issue, delete the output directory and run DoxyPress again\n",
                   csPrintable(mapName));

      } else if (logCount == 5) {
         logCount++;
         errNoPrefixAll("\n** Suppressing all further messages regarding dot map file\n\n");

      }

      return false;
   }

   const int maxLineLen = 1024;
   char buf[maxLineLen];
   char url[maxLineLen];
   char ref[maxLineLen];

   int x1, y1, x2, y2;

   while (! f.atEnd()) {
      bool isRef = false;
      int numBytes = f.readLine(buf, maxLineLen);

      buf[numBytes - 1] = '\0';

      if (qstrncmp(buf, "rect", 4) == 0) {
         // obtain the url and the coordinates in the order used by graphviz-1.5
         sscanf(buf, "rect %s %d,%d %d,%d", url, &x1, &y1, &x2, &y2);

         QString tUrl = QString::fromUtf8(url);

         if (tUrl == "\\ref" || tUrl == "@ref") {
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
            // handle DoxyPress \ref tag URL reference
            DocRef *df = new DocRef( (DocNode *) 0, url, context );
            t << externalRef(relPath, df->ref(), true);

            if (!df->file().isEmpty()) {
               t << df->file() << Doxy_Globals::htmlFileExtension;
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
   QString absOutFile = outDir + "/" + outFile;

   // move to output dir so dot can find the font file
   QString oldDir = QDir::currentPath();
   QDir::setCurrent(outDir);

   QString mscExe = Config::getString("mscgen-path") + "mscgen" + portable_commandExtension();
   QString mscArgs;

   QString imageName = outFile;

   switch (format) {
      case MSC_BITMAP:
         mscArgs   += "-T png";
         imageName += ".png";
         break;

      case MSC_EPS:
         mscArgs   += "-T eps";
         imageName += ".eps";
         break;

      case MSC_SVG:
         mscArgs   += "-T svg";
         imageName += ".svg";
         break;

      default:
         QDir::setCurrent(oldDir);
         return;
   }

   mscArgs += " -i \"";
   mscArgs += inFile;

   mscArgs += "\" -o \"";
   mscArgs += imageName + "\"";

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
      epstopdfArgs = QString("\"%1.eps\" --outfile=\"%2.pdf\"").formatArgs(outFile, outFile);

      portable_sysTimerStart();
      if (portable_system("epstopdf", epstopdfArgs) != 0) {
         err("Unable to run epstopdf. Verify your TeX installation\n");
      }

      portable_sysTimerStop();
   }

   Doxy_Globals::indexList.addImageFile(imageName);

   QDir::setCurrent(oldDir);
}

QString getMscImageMapFromFile(const QString &inFile, const QString &outDir, const QString &relPath, const QString &context)
{
   QString result;

   QString outFile = inFile + ".map";

   // move to the output directory so dot can find the font file
   QString oldDir = QDir::currentPath();
   QDir::setCurrent(outDir);

   QString mscExe  = Config::getString("mscgen-path") + "mscgen" + portable_commandExtension();
   QString mscArgs = "-T ismap -i \"" + inFile + "\" -o \"" + outFile + "\"";

   portable_sysTimerStart();
   int exitCode = portable_system(mscExe, mscArgs, false);
   portable_sysTimerStop();

   if (exitCode == 0) {
      QTextStream tmpout(&result);

      convertMapFile(tmpout, outFile, relPath, context);
      QDir().remove(outFile);
   }

   QDir::setCurrent(oldDir);

   return result;
}

void writeMscImageMapFromFile(QTextStream &t, const QString &inFile, const QString &outDir, const QString &relPath,
                  const QString &baseName, const QString &context, MscOutputFormat format)
{
   QString mapName = baseName + ".map";

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

   QString imap = getMscImageMapFromFile(inFile, outDir, relPath, context);
   t << "<map name=\"" << mapName << "\" id=\"" << mapName << "\">" << imap << "</map>" << endl;
}

