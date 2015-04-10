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

#include <plantuml.h>
#include <portable.h>
#include <config.h>
#include <message.h>

static const int maxCmdLine = 40960;

QByteArray writePlantUMLSource(const QString &outDir, const QString &fileName, const QByteArray &content)
{
   QString baseName;
   static int umlindex = 1;

   if (fileName.isEmpty()) { 
      // generate name
      baseName = outDir + "/inline_umlgraph_" + QString::number(umlindex++);

   } else { 
      // user specified name
      baseName = fileName;

      int i = baseName.lastIndexOf('.');
      if (i != -1) {
         baseName = baseName.left(i);
      }

      baseName.prepend(outDir + "/");
   }

   QFile file(baseName + ".pu");

   if (! file.open(QIODevice::WriteOnly)) {
      err("Could not open file %s for writing\n", qPrintable(baseName));
   }

   QString text = "@startuml\n";
   text += content;
   text += "@enduml\n";

   file.write(text.toUtf8());
   file.close();

   return baseName.toUtf8();
}

void generatePlantUMLOutput(const QString &baseName, const QString &outDir, PlantUMLOutputFormat format)
{
   static QString plantumlJarPath         = Config::getString("plantuml-jar-path"); 
   static QStringList pumlIncludePathList = Config::getList("plantuml-inc-path");

   QString pumlExe  = "java";
   QString pumlArgs = "";
   
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
   pumlArgs += "-charset " + Config::getString("input-encoding") + " ";

   int exitCode;
   
   msg("Running PlantUML on generated file %s.pu\n", qPrintable(baseName));
   portable_sysTimerStart();

   if ((exitCode = portable_system(pumlExe, pumlArgs, false)) != 0) {
      err("Problem running PlantUML, verify the command 'java -jar \"%splantuml.jar\" -h' works from the command line. Exit code: %d\n",
          plantumlJarPath.constData(), exitCode);

   } else if (Config::getBool("dot-cleanup")) {
      QFile(baseName + ".pu").remove();

   }

   portable_sysTimerStop();
   if ( (format == PUML_EPS) && (Config::getBool("latex-pdf")) ) {

      QString epstopdfArgs;
      epstopdfArgs = QString("\"%1.eps\" --outfile=\"%2.pdf\"").arg(baseName).arg(baseName);

      portable_sysTimerStart();

      if ((exitCode = portable_system("epstopdf", epstopdfArgs)) != 0) {
         err("Problem running epstopdf, verify your TeX installation, exit code: %d\n", exitCode);
      }

      portable_sysTimerStop();
   }
}

