/************************************************************************
*
* Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
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

#include <dia.h>

#include <config.h>
#include <message.h>
#include <portable.h>
#include <util.h>

static const int maxCmdLine = 40960;

void writeDiaGraphFromFile(const QString &inFile, const QString &outDir, const QString &outFile, DiaOutputFormat format)
{
   static const QString diaPath = Config::getString("dia-path");

   QString absOutFile = outDir + QDir::separator() +outFile;

   // move to the output dir so dot can find the font file.
   QString oldDir = QDir::currentPath();
   QDir::setCurrent(outDir);

   QString diaExe = diaPath + "dia" + portable_commandExtension();

   QString diaArgs;
   QString extension;

   diaArgs += "-n ";

   if (format == DIA_BITMAP) {
      diaArgs += "-t png-libart";
      extension = ".png";

   } else if (format == DIA_EPS) {
      diaArgs += "-t eps";
      extension = ".eps";
   }

   diaArgs += " -e \"";
   diaArgs += outFile;
   diaArgs += extension + "\"";

   diaArgs += " \"";
   diaArgs += inFile;
   diaArgs += "\"";

   bool ok = true;
   int exitCode;

   portable_sysTimerStart();

   if ((exitCode = portable_system(diaExe, diaArgs, false)) != 0) {
      portable_sysTimerStop();
      ok = false;
   }

   if (ok) {
      portable_sysTimerStop();

      if ( (format == DIA_EPS) && (Config::getBool("latex-pdf")) ) {

         QString epstopdfArgs;
         epstopdfArgs = QString("\"%1.eps\" --outfile=\"%2.pdf\"").formatArgs(outFile, outFile);

         portable_sysTimerStart();

         if (portable_system("epstopdf", epstopdfArgs) != 0) {
            err("Unable to run epstopdf. Check your TeX installation\n");
         }
         portable_sysTimerStop();
      }
   }

   QDir::setCurrent(oldDir);
}

