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

#include <plantuml.h>

#include <config.h>
#include <doxy_globals.h>
#include <portable.h>
#include <message.h>

static const int maxCmdLine = 40960;

QString writePlantUMLSource(const QString &outDir, const QString &fileName, const QString &content)
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
      err("Unable to open file for writing %s, error: %d\n", csPrintable(baseName), file.error());
   }

   QString text = "@startuml\n";
   text += content;
   text += "\n@enduml\n";

   file.write(text.toUtf8());
   file.close();

   return baseName;
}

void generatePlantUMLOutput(const QString &baseName, const QString &outDir, PlantUMLOutputFormat format)
{
   static const QString plantumlJarPath      = Config::getString("plantuml-jar-path");
   static const QString plantumlCfgFile      = Config::getString("plantuml-cfg-file");
   static const QStringList plantumlIncPath  = Config::getList("plantuml-inc-path");
   static const bool haveDot                 = Config::getBool("have-dot");
   static const QString dotPath              = Config::getString("dot-path");
   static const bool latexPdf                = Config::getBool("latex-pdf");

   QString pumlExe  = "java";
   QString pumlArgs = "";

   if (! plantumlIncPath.isEmpty()) {
      pumlArgs += "-Dplantuml.include.path=\"";
      pumlArgs += plantumlIncPath.join( QChar(portable_pathListSeparator()) );
      pumlArgs += "\" ";
   }

   pumlArgs += "-Djava.awt.headless=true -jar \"" + plantumlJarPath + "plantuml.jar\" ";

   if (! plantumlCfgFile.isEmpty()) {
      pumlArgs += "-config \"";
      pumlArgs += plantumlCfgFile;
      pumlArgs += "\" ";
   }

   if (haveDot && ! dotPath.isEmpty())   {
      pumlArgs += "-graphvizdot \"";
      pumlArgs += dotPath;
      pumlArgs += "dot\" ";
   }

   pumlArgs += "-o \"";
   pumlArgs += outDir;
   pumlArgs += "\" ";

   // strip path from basename contains path to create the image file
   // includ in the index.qhp (Qt help index file)

   QString imgName = baseName;
   int pos;

   if ((pos = imgName.lastIndexOf('/')) !=- 1 )  {
      // strip path
      pos     = imgName.length() - pos - 1;
      imgName = imgName.right(pos);
   }

   switch (format) {
      case PUML_BITMAP:
         pumlArgs += "-tpng";
         imgName  += ".png";
         break;

      case PUML_EPS:
         pumlArgs += "-teps";
         imgName  += ".eps";
         break;

      case PUML_SVG:
         pumlArgs += "-tsvg";
         imgName  += ".svg";
         break;
   }

   pumlArgs += " \"";
   pumlArgs += baseName;
   pumlArgs += ".pu\" ";
   pumlArgs += "-charset UTF-8 ";

   int exitCode;

   msg("Running PlantUML on generated file %s.pu\n", csPrintable(baseName));
   portable_sysTimerStart();

   if ((exitCode = portable_system(pumlExe, pumlArgs, true)) != 0) {
      err("Unable to run PlantUML, verify the command 'java -jar \"%splantuml.jar\" -h' works from "
         "the command line. Exit code: %d\n", csPrintable(plantumlJarPath), exitCode);

   } else if (Config::getBool("dot-cleanup")) {
      QFile(baseName + ".pu").remove();

   }

   portable_sysTimerStop();
   if ( (format == PUML_EPS) && latexPdf) {

      QString epstopdfArgs;
      epstopdfArgs = QString("\"%1.eps\" --outfile=\"%2.pdf\"").formatArg(baseName).formatArg(baseName);

      portable_sysTimerStart();

      if ((exitCode = portable_system("epstopdf", epstopdfArgs)) != 0) {
         err("Unable to run epstopdf, verify your TeX installation, exit code: %d\n", exitCode);
      }

      portable_sysTimerStop();
   }

   Doxy_Globals::indexList.addImageFile(imgName);
}

