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

#include <plantuml.h>
#include <portable.h>
#include <config.h>
#include <message.h>

static const int maxCmdLine = 40960;

QByteArray writePlantUMLSource(const QByteArray &outDir, const QByteArray &fileName, const QByteArray &content)
{
   QByteArray baseName;
   static int umlindex = 1;

   if (fileName.isEmpty()) { // generate name
      baseName = outDir + "/inline_umlgraph_" + QByteArray().setNum(umlindex++);

   } else { // user specified name
      baseName = fileName;

      int i = baseName.lastIndexOf('.');
      if (i != -1) {
         baseName = baseName.left(i);
      }

      baseName.prepend(outDir + "/");
   }

   QFile file(baseName + ".pu");
   if (!file.open(QIODevice::WriteOnly)) {
      err("Could not open file %s for writing\n", baseName.data());
   }

   QByteArray text = "@startuml\n";
   text += content;
   text += "@enduml\n";
   file.write( text, text.length() );
   file.close();

   return baseName;
}

void generatePlantUMLOutput(const char *baseName, const char *outDir, PlantUMLOutputFormat format)
{
   static QByteArray plantumlJarPath = Config_getString("PLANTUML_JAR_PATH");

   QByteArray pumlExe  = "java";
   QByteArray pumlArgs = "";

   QStringList pumlIncludePathList = Config_getList("PLANTUML_INCLUDE_PATH");
   QString s = pumlIncludePathList.first();

   if (! pumlIncludePathList.isEmpty()) {     
      pumlArgs += "-Dplantuml.include.path=\"";
      pumlArgs += pumlIncludePathList.join( QChar(portable_pathListSeparator()) );
      pumlArgs += "\" ";     
   }

   pumlArgs += "-Djava.awt.headless=true -jar \"" + plantumlJarPath + "plantuml.jar\" ";
   pumlArgs += "-o \"";
   pumlArgs += outDir;
   pumlArgs += "\" ";

   QByteArray extension;

   switch (format) {
      case PUML_BITMAP:
         pumlArgs += "-tpng";
         extension = ".png";
         break;

      case PUML_EPS:
         pumlArgs += "-teps";
         extension = ".eps";
         break;

      case PUML_SVG:
         pumlArgs += "-tsvg";
         extension = ".svg";
         break;
   }

   pumlArgs += " \"";
   pumlArgs += baseName;
   pumlArgs += ".pu\" ";
   pumlArgs += "-charset " + Config_getString("INPUT_ENCODING") + " ";

   int exitCode;

   // printf("*** running: %s %s outDir:%s %s\n",pumlExe.data(),pumlArgs.data(),outDir,outFile);

   msg("Running PlantUML on generated file %s.pu\n", baseName);
   portable_sysTimerStart();

   if ((exitCode = portable_system(pumlExe, pumlArgs, false)) != 0) {
      err("Problems running PlantUML. Verify the command 'java -jar \"%splantuml.jar\" -h' works from the command line. Exit code: %d\n",
          plantumlJarPath.data(), exitCode);

   } else if (Config_getBool("DOT_CLEANUP")) {
      QFile(QByteArray(baseName) + ".pu").remove();

   }

   portable_sysTimerStop();
   if ( (format == PUML_EPS) && (Config_getBool("USE_PDFLATEX")) ) {

      QString epstopdfArgs;
      epstopdfArgs = QString("\"%1.eps\" --outfile=\"%2.pdf\"").arg(baseName).arg(baseName);

      portable_sysTimerStart();

      if ((exitCode = portable_system("epstopdf", epstopdfArgs.toLatin1())) != 0) {
         err("Problems running epstopdf. Check your TeX installation! Exit code: %d\n", exitCode);
      }

      portable_sysTimerStop();
   }
}

