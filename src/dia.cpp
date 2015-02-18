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

#include <dia.h>
#include <portable.h>
#include <config.h>
#include <message.h>
#include <util.h>

static const int maxCmdLine = 40960;

void writeDiaGraphFromFile(const char *inFile, const char *outDir,
                           const char *outFile, DiaOutputFormat format)
{
   QByteArray absOutFile = outDir;
   absOutFile += portable_pathSeparator();
   absOutFile += outFile;

   // chdir to the output dir, so dot can find the font file.
   QByteArray oldDir = QDir::currentPath().toUtf8();

   // go to the html output directory (i.e. path)
   QDir::setCurrent(outDir);

   //printf("Going to dir %s\n",QDir::currentPath().data());
   QByteArray diaExe = Config_getString("DIA_PATH") + "dia" + portable_commandExtension();
   QByteArray diaArgs;
   QByteArray extension;

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

   //printf("*** running: %s %s outDir:%s %s\n",diaExe.data(),diaArgs.data(),outDir,outFile);
   portable_sysTimerStart();

   if ((exitCode = portable_system(diaExe, diaArgs, false)) != 0) {
      portable_sysTimerStop();
      ok = false;     
   }

   if (ok) {
      portable_sysTimerStop();
   
      if ( (format == DIA_EPS) && (Config_getBool("USE_PDFLATEX")) ) {      

         QString epstopdfArgs;
         epstopdfArgs = QString("\"%1.eps\" --outfile=\"%2.pdf\"").arg(outFile).arg(outFile);
   
         portable_sysTimerStart();

         if (portable_system("epstopdf", epstopdfArgs.toLatin1()) != 0) {
            err("Problems running epstopdf. Check your TeX installation!\n");
         }
         portable_sysTimerStop();
      }
   }

   QDir::setCurrent(oldDir);
}

