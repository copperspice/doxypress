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

#include <dia.h>
#include <portable.h>
#include <config.h>
#include <message.h>
#include <util.h>

static const int maxCmdLine = 40960;

void writeDiaGraphFromFile(const QString &inFile, const QString &outDir, const QString &outFile, DiaOutputFormat format)
{
   QString absOutFile = outDir + QDir::separator() +outFile;

   // move to the output dir so dot can find the font file.
   QString oldDir = QDir::currentPath();   
   QDir::setCurrent(outDir);

   QString diaExe = Config::getString("dia-path") + "dia" + portable_commandExtension();

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
         epstopdfArgs = QString("\"%1.eps\" --outfile=\"%2.pdf\"").arg(outFile).arg(outFile);
   
         portable_sysTimerStart();

         if (portable_system("epstopdf", epstopdfArgs) != 0) {
            err("Unable to run epstopdf. Check your TeX installation\n");
         }
         portable_sysTimerStop();
      }
   }

   QDir::setCurrent(oldDir);
}

