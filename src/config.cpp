/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim
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

#include <config.h>

#include <doxy_setup.h>
#include <doxy_globals.h>
#include <language.h>
#include <message.h>
#include <portable.h>
#include <pre.h>
#include <util.h>

bool Config::getBool(const QString &name)
{
   bool retval   = false;
   auto hashIter = m_cfgBool.find(name);

   if (hashIter != m_cfgBool.end()) {
      retval = hashIter.value().value;

   } else {
      fprintf(stderr, "Warning: %s was not retrieved from the project bool table\n", csPrintable(name) );

   }

   return retval;
}

QString Config::getEnum(const QString &name)
{
   QString retval;
   auto hashIter = m_cfgEnum.find(name);

    if (hashIter != m_cfgEnum.end()) {
      retval = hashIter.value().value;

   } else {
      fprintf(stderr, "Warning: %s was not retrieved from the project enum table\n", csPrintable(name) );

   }

   return retval;
}

int Config::getInt(const QString &name)
{
   int retval    = 0;
   auto hashIter = m_cfgInt.find(name);

    if (hashIter != m_cfgInt.end()) {
      retval = hashIter.value().value;

   } else {
      fprintf(stderr, "Warning: %s was not retrieved from the project integer table\n", csPrintable(name) );

   }

   return retval;
}

QStringList Config::getList(const QString &name)
{
   QStringList retval;
   auto hashIter = m_cfgList.find(name);

    if (hashIter != m_cfgList.end()) {
      retval = hashIter.value().value;

   } else {
      fprintf(stderr, "Warning: %s was not retrieved from the project list table\n", csPrintable(name) );

   }

   return retval;
}

QString Config::getString(const QString &name)
{
   QString retval;
   auto hashIter = m_cfgString.find(name);

    if (hashIter != m_cfgString.end()) {
      retval = hashIter.value().value;

   } else {
      fprintf(stderr, "Warning: %s was not retrieved from the project string table\n", csPrintable(name) );

   }

   return retval;
}

// update project data
void Config::setBool(const QString &name, bool data)
{
   auto hashBool = m_cfgBool.find(name);
   hashBool.value().value = data;
}

void Config::setList(const QString &name, const QStringList &data)
{
   auto hashIter = m_cfgList.find(name);
   hashIter.value().value = data;
}

// **
void Config::msgVerify()
{
   // ** messages
   auto iterString    = m_cfgString.find("warn-format");
   QString warnFormat = iterString.value().value.trimmed();

   if (warnFormat.isEmpty()) {
      warnFormat = "$file:$line $text";

   } else {
      if (warnFormat.indexOf("$file") == -1) {
         warnMsg("WARN FORMAT does not contain $file\n");
      }

      if (warnFormat.indexOf("$line") == -1) {
         warnMsg("WARN FORMAT does not contain $line\n");
      }

      if (warnFormat.indexOf("$text") == -1) {
         warnMsg("WARN FORMAT foes not contain $text\n");
      }
   }

   iterString.value().value = warnFormat;

   // set up messages
   initWarningFormat();
}

bool Config::preVerify()
{
   bool isOk = true;

   // **
   if (! (Config::getBool("generate-html") || Config::getBool("generate-latex") || Config::getBool("generate-man") ||
          Config::getBool("generate-perl") || Config::getBool("generate-rtf")   || Config::getBool("generate-xml") ||
          Config::getBool("generate-docbook")) && Config::getString("generate-tagfile").isEmpty() ) {

      err("No output format was indicated, at least one output format must be selected\n");
      isOk = false;
   }

   // **
   auto iterString = m_cfgString.find("html-header");
   QString headerFile = iterString.value().value;

   if (! headerFile.isEmpty()) {
      QFileInfo fi(headerFile);

      if (! fi.exists()) {
         err("HTML Header file `%s' does not exist\n", csPrintable(headerFile));
         isOk = false;
      }
   }

   // **
   iterString = m_cfgString.find("html-footer");
   QString footerFile = iterString.value().value;

   if (! footerFile.isEmpty()) {
      QFileInfo fi(footerFile);

      if (! fi.exists()) {
         err("HTML Footer file `%s' does not exist\n", csPrintable(footerFile));
         isOk = false;
      }
   }

   // **
   iterString = m_cfgString.find("latex-header");
   QString latexHeaderFile = iterString.value().value;

   if (! latexHeaderFile.isEmpty()) {
      QFileInfo fi(latexHeaderFile);

      if (! fi.exists()) {
         err("LaTeX Header file `%s' does not exist\n", csPrintable(latexHeaderFile));
         isOk = false;
      }
   }

   // **
   iterString = m_cfgString.find("latex-footer");
   QString latexFooterFile = iterString.value().value;

   if (! latexFooterFile.isEmpty()) {
      QFileInfo fi(latexFooterFile);

      if (! fi.exists()) {
         err("LaTeX Footer file `%s' does not exist\n", csPrintable(latexFooterFile));
         isOk = false;
      }
   }

   // **
   if (Config::getBool("use-mathjax")) {
      iterString = m_cfgString.find("mathjax-codefile");
      QString mathJaxCodefile = iterString.value().value;

      if (! mathJaxCodefile.isEmpty()) {
         QFileInfo fi(mathJaxCodefile);

         if (! fi.exists()) {
            err("MathJax Codefile file `%s' does not exist\n", csPrintable(mathJaxCodefile));
            isOk = false;
         }
      }
   }

   // ** currently not used
   if (Config::getBool("clang-parsing")) {
      // errNoPrefixAll("\n");
   }

   return isOk;
}

bool Config::verify()
{
   const static QStringList s_dotImageFormat = getDotImageFormat();
   const static QStringList s_mathJaxFormat  = getMathJaxFormat();
   const static QStringList s_latexPaperType = getLatexPaperType();
   const static QStringList s_rtfPaperType   = getRtfPaperType();

   bool isOk = true;

   // ** project
   auto iterString = m_cfgString.find("output-dir");
   QString outputDirectory = iterString.value().value;

   if (outputDirectory.isEmpty()) {
       outputDirectory = QDir::currentPath();

   } else {
      QDir dir(outputDirectory);

      if (! dir.exists()) {
         dir.setPath(QDir::currentPath());

         if (! dir.mkdir(outputDirectory)) {
            err("Output directory `%s' does not exist and can not be created\n", csPrintable(outputDirectory));
            isOk = false;

         } else {
            msg("Output directory `%s' was automatically created\n", csPrintable(outputDirectory));

         }

         dir.cd(outputDirectory);
      }

      outputDirectory = dir.absolutePath();
   }

   iterString.value().value = outputDirectory;


   // ** general
   auto iterEnum = m_cfgEnum.find("output-language");
   QString outputLanguage = iterEnum.value().value;

   if (outputLanguage.isEmpty()) {
      warn_uncond("Output language %s was not specified, using English\n", csPrintable(outputLanguage));
      outputLanguage = "English";
   }

   iterEnum.value().value = outputLanguage;


   // **
   auto iterList = m_cfgList.find("abbreviate-brief");
   QStringList abbreviatebrief = iterList.value().value;

   if (abbreviatebrief.isEmpty()) {
     abbreviatebrief = getAbbreviateBrief();

     iterList.value().value = abbreviatebrief;
   }


   // **
   iterList = m_cfgList.find("strip-from-path");
   QStringList stripFromPath = iterList.value().value;

   if (stripFromPath.isEmpty()) {
      stripFromPath.append(QDir::currentPath() + "/");

   } else {
      // expand the relative stripFromPath values
      cleanUpPaths(stripFromPath);
   }

   iterList.value().value = stripFromPath;


   // **
   iterList = m_cfgList.find("strip-from-inc-path");
   QStringList stripFromIncPath = iterList.value().value;

   // expand the relative stripFromPath values
   cleanUpPaths(stripFromIncPath);

   iterList.value().value = stripFromIncPath;


   // **
   iterList = m_cfgList.find("aliases");
   QStringList aliasList = iterList.value().value;

   for (auto alias: aliasList) {
      static QRegExp re1("[a-z_A-Z][a-z_A-Z0-9]*[ \t]*=");               // alias without argument
      static QRegExp re2("[a-z_A-Z][a-z_A-Z0-9]*\\{[0-9]*\\}[ \t]*=");   // alias with argument

      alias = alias.trimmed();

      if (! (re1.indexIn(alias) == 0 || re2.indexIn(alias) == 0)) {
         err("Alias format: `%s' \n is invalid, use \"name=value\" or \"name{n}=value\", where n "
                  "is the number of arguments\n\n", csPrintable(alias));
      }
   }

   // **
   const QStringList extMaps = Config::getList("language-mapping");

   for (auto mapStr : extMaps) {
      int i = mapStr.indexOf('=');

      if (i != -1) {
         QString extension = mapStr.left(i).trimmed().toLower();
         QString language  = mapStr.mid(i + 1).trimmed().toLower();

         if (! updateLanguageMapping(extension, language, true)) {
            err("Unable to map file extension '%s' to '%s', verify the Extension Mapping tag\n",
                  csPrintable(extension), csPrintable(language));

         } else {
            msg("Adding custom extension mapping: .%s, will be treated as language %s\n",
                  csPrintable(extension), csPrintable(language));
         }
      }
   }

   // **  build
   iterString = m_cfgString.find("layout-file");
   QString layoutFileName = iterString.value().value;

   if (layoutFileName.isEmpty()) {
      layoutFileName = "doxy_layout.xml";

   } else {
      QFileInfo fi(layoutFileName);

      if (! fi.exists()) {
         err("Layout file `%s' does not exist\n", csPrintable(layoutFileName));
         isOk = false;
      }
   }

   iterString.value().value = layoutFileName;


   // ** input
   iterList = m_cfgList.find("input-source");
   QStringList inputSource = iterList.value().value;

   if (inputSource.count() == 0) {
      // use current dir as the default
      inputSource.append(QDir::currentPath());

   } else {

      for (auto item : inputSource) {
         QFileInfo fi(item.trimmed());

         if (! fi.exists()) {
            warnMsg("INPUT SOURCE `%s' does not exist\n", csPrintable(item));
         }
      }
   }
   iterList.value().value = inputSource;

   // **
   iterList = m_cfgList.find("example-patterns");
   QStringList examplePatterns = iterList.value().value;

   if (examplePatterns.isEmpty()) {
      // add one
      examplePatterns.append("*");

      iterList.value().value = examplePatterns;
   }

   // **
   iterList = m_cfgList.find("input-patterns");
   QStringList inputPatterns = iterList.value().value;

   if (inputPatterns.isEmpty()) {
      inputPatterns = getFilePatterns();
      iterList.value().value = inputPatterns;
   }


   // ** source code
   iterList = m_cfgList.find("suffix-source-navtree");
   QStringList suffixSource = iterList.value().value;

   if (suffixSource.isEmpty()) {
      suffixSource = getSuffixSource();
      iterList.value().value = suffixSource;

   } else  {

      for (auto &item: suffixSource) {
          item = item.trimmed();

          if (item.startsWith('.')) {
            item = item.mid(1);
          }
      }

      iterList.value().value = suffixSource;
   }

   iterList = m_cfgList.find("suffix-header-navtree");
   QStringList suffixHeader = iterList.value().value;

   if (suffixHeader.isEmpty()) {
      suffixHeader = getSuffixHeader();
      iterList.value().value = suffixHeader;

   } else  {

      for (auto &item: suffixHeader) {
          item = item.trimmed();

          if (item.startsWith('.')) {
            item = item.mid(1);
          }
      }

      iterList.value().value = suffixHeader;
   }

   iterList = m_cfgList.find("suffix-exclude-navtree");
   QStringList suffixExclude = iterList.value().value;

   if (suffixExclude.isEmpty()) {
      suffixExclude = getSuffixExclude();
      iterList.value().value = suffixExclude;

   } else  {

      for (auto &item: suffixExclude) {
          item = item.trimmed();

          if (item.startsWith('.')) {
            item = item.mid(1);
          }
      }

      iterList.value().value = suffixExclude;
   }


   // ** preprocessing
   iterList = m_cfgList.find("include-path");
   QStringList includePath = iterList.value().value;

   for (auto item : includePath) {
      QFileInfo fi(item.trimmed());

      if (fi.exists()) {
         addSearchDir(fi.absoluteFilePath());

      } else {
         warnMsg("INCLUDE PATH `%s' does not exist\n", csPrintable(item));

      }
   }
   iterList.value().value = includePath;


   // ** dot  (full value, such as png:cairo:gd)
   iterEnum = m_cfgEnum.find("dot-image-format");
   QString dotImageFormat = iterEnum.value().value;

   dotImageFormat = dotImageFormat.trimmed();

   if (dotImageFormat.isEmpty()) {
      dotImageFormat = "png";

   } else if (! s_dotImageFormat.contains(dotImageFormat)) {
      err("Invalid value of %s for Dot Image Format, using the the default of png\n", csPrintable(dotImageFormat));

      dotImageFormat = "png";
   }

   iterEnum.value().value = dotImageFormat;

   // save the stripped extension (png, jpeg)
   QString dotImage = dotImageFormat;

   if (dotImage.contains(":"))  {
      static QRegExp regexp(":.*");
      dotImage = dotImage.replace(regexp , "");
   }

   m_cfgEnum.insert("dot-image-extension", struc_CfgEnum{ dotImage, DEFAULT } );


   //
   iterString = m_cfgString.find("mscgen-path");
   QString mscgenPath = iterString.value().value;

   if (! mscgenPath.isEmpty()) {
      QFileInfo dp(mscgenPath + "/mscgen" + portable_commandExtension());

      if (! dp.exists() || ! dp.isFile()) {
         warnMsg("Unable to find the MSCGEN program at %s\n", csPrintable(mscgenPath));
         mscgenPath = "";

      } else {
         mscgenPath = dp.absolutePath() + "/";

      }

      iterString.value().value = mscgenPath;
   }


   // **
   iterString = m_cfgString.find("dia-path");
   QString diaPath = iterString.value().value;

   if (! diaPath.isEmpty()) {
      QFileInfo dp(diaPath + "/dia" + portable_commandExtension());

      if (! dp.exists() || ! dp.isFile()) {
         warnMsg("Unable to find the DIA program at %s\n", csPrintable(diaPath));
         diaPath = "";

      } else {
         diaPath = dp.absolutePath() + "/";

      }

      iterString.value().value = diaPath;
   }


   // **
   iterString = m_cfgString.find("dot-path");
   QString dotPath = iterString.value().value;

   if (! dotPath.isEmpty()) {
      QFileInfo fi(dotPath);

      if (fi.exists() && fi.isFile()) {
         // user specified path + exec
         dotPath = fi.absoluteFilePath();

      } else {
         QFileInfo dp(dotPath + "/dot" + portable_commandExtension());

         if (! dp.exists() || ! dp.isFile()) {

            if (Config::getBool("have-dot")) {
               warnMsg("Unable to locate the dot program in %s\n", csPrintable(dotPath));
            }

            dotPath = "";

         } else {
            dotPath = dp.absoluteFilePath();
         }
      }

   } else {
      // changed to store the dot program
      dotPath = "dot" + portable_commandExtension();

   }

   iterString.value().value = dotPath;


   // ** dot
   iterString = m_cfgString.find("plantuml-jar-path");
   QString plantumlJarPath = iterString.value().value;

   if (! plantumlJarPath.isEmpty()) {
      QFileInfo file(plantumlJarPath);

      if (file.exists() && file.isDir()) {
         // plantuml-jar-path is directory

         QFileInfo jar(plantumlJarPath + QDir::separator() + "plantuml.jar");

         if (jar.exists() && jar.isFile()) {
            plantumlJarPath = jar.absolutePath() + QDir::separator();

         } else {
            err("PlantUml Jar File 'plantuml.jar' was not found at the path specified by "
                   "PlantUml Jar Path '%s'\n", csPrintable(plantumlJarPath));

            iterString.value().value = QString();
         }

      } else if (file.exists() && file.isFile() && plantumlJarPath.endsWith(".jar")) {
         // plantuml-jar-path is a file

         plantumlJarPath = file.absolutePath() + QDir::separator() ;
         iterString.value().value = plantumlJarPath;

      } else {
         err("PlantUml Jar Path does not exist and is not a directory: %s\n", csPrintable(plantumlJarPath));
         iterString.value().value = QString();
      }
   }

   // **
   auto iterInt = m_cfgInt.find("dot-graph-max-depth");
   int depth = iterInt.value().value;

   if (depth == 0 || depth > 1000) {
      iterInt.value().value = 1000;
   }


   // ** html
   iterString = m_cfgString.find("html-file-extension");
   QString htmlFileExtension = iterString.value().value.trimmed();

   if (htmlFileExtension.isEmpty()) {
      htmlFileExtension = ".html";
   }

   iterString.value().value = htmlFileExtension;
   Doxy_Globals::htmlFileExtension = htmlFileExtension;


   // **
   iterInt = m_cfgInt.find("html-colorstyle-hue");
   int hue = iterInt.value().value;

   if (hue < 0) {
      hue = 0;

   } else if (hue >= 360) {
      hue = hue % 360;

   }

   iterInt.value().value = hue;


    // **
   iterInt = m_cfgInt.find("html-colorstyle-sat");
   int sat = iterInt.value().value;

   if (sat < 0) {
      sat = 0;

   } else if (sat > 255) {
      sat = 255;

   }

   iterInt.value().value = sat;


   // **
   iterInt = m_cfgInt.find("html-colorstyle-gamma");
   int gamma = iterInt.value().value;

   if (gamma < 40) {
      gamma = 40;

   } else if (gamma > 240) {
      gamma = 240;

   }

   iterInt.value().value = gamma;


   // **
   iterEnum = m_cfgEnum.find("mathjax-format");
   QString mathJaxFormat = iterEnum.value().value;

   if (! s_mathJaxFormat.contains(mathJaxFormat)) {

      if (mathJaxFormat.isEmpty()) {
         err("MathJax Format can not be empty, setting to the default value of HTML-CSS\n");

      } else  {
         err("Invalid value of %s for MathJax Format, setting to the default value of HTML-CSS\n", csPrintable(mathJaxFormat));

      }

      iterEnum.value().value = "HTML-CSS";
   }

   // ** docset
   QString docset;

   iterString = m_cfgString.find("project-name");
   docset = iterString.value().value;

   if (docset.isEmpty()) {
      docset = "My Project";
   }
   iterString.value().value = docset;

   // *
   iterString = m_cfgString.find("docset-bundle-id");
   docset = iterString.value().value;

   if (docset.isEmpty()) {
      docset = "org.doxypress.Project";
   }
   iterString.value().value = docset;

   // *
   iterString = m_cfgString.find("docset-feedname");
   docset = iterString.value().value;

   if (docset.isEmpty()) {
      docset = "DoxyPress generated docs";
   }
   iterString.value().value = docset;

   // *
   iterString = m_cfgString.find("docset-publisher-id");
   docset = iterString.value().value;

   if (docset.isEmpty()) {
      docset = "org.doxypress.Publisher";
   }
   iterString.value().value = docset;

   // *
   iterString = m_cfgString.find("docset-publisher-name");
   docset = iterString.value().value;

   if (docset.isEmpty()) {
      docset = "Publisher";
   }
   iterString.value().value = docset;



   // ** latex
   iterString = m_cfgString.find("latex-bib-style");
   QString style = iterString.value().value;

   if (style.isEmpty()) {
      style = "plain";
   }

   iterString.value().value = style;


   // **
   iterEnum = m_cfgEnum.find("latex-paper-type");
   QString paperType = iterEnum.value().value;

   paperType = paperType.toLower().trimmed();

   if (paperType.isEmpty() || paperType == "a4wide") {
      paperType = "a4";

   } else if (! s_latexPaperType.contains(paperType)) {
      err("Invalid value of %s for LaTeX Paper Type, using the the default of a4\n", csPrintable(paperType));

      paperType = "a4";
   }

   iterEnum.value().value = paperType;


   // **  man
   iterString = m_cfgString.find("man-extension");
   QString manExtension = iterString.value().value;

   if (manExtension.isEmpty()) {
      manExtension = "3";

   } else {

      if (manExtension.startsWith('.')) {
         if (manExtension.length() == 1) {
            manExtension = "3";

         } else {
            // strip .
            manExtension = manExtension.mid(1);
         }
      }

      if (manExtension.at(0) < '0' || manExtension.at(0) > '9') {
         manExtension.prepend("3");
      }
   }

   iterString.value().value = manExtension;


   // ** qt help
   if (Config::getBool("generate-qthelp")) {

      iterString = m_cfgString.find("qhp-namespace");
      QString temp = iterString.value().value;

      if (temp.isEmpty()) {
         err("When Generate QtHelp is set, QHP Namespace can not be empty. Setting to the default value of 'org.doxypress.doc'\n");

         iterString.value().value = "org.doxypress.doc";
      }

      iterString = m_cfgString.find("qhp-virtual-folder");
      temp = iterString.value().value;

      if (temp.isEmpty()) {
         err("When Generate QtHelp is set, QHP Virtual Folder can not be empty. Setting to the defualt of 'doc'\n");

         iterString.value().value = "doc";
      }
   }

   // ** rtf
   iterEnum = m_cfgEnum.find("rtf-paper-type");

   paperType = paperType.toLower().trimmed();

   if (paperType.isEmpty()) {
      paperType = "a4";

   } else if (! s_rtfPaperType.contains(paperType)) {
      err("Invalid value of %s for RTF Paper Type, using the the default of a4\n", csPrintable(paperType));

      paperType = "a4";
   }

   iterEnum.value().value = paperType;


   // **********

   // **
   if (Config::getBool("generate-treeview") && Config::getBool("generate-chm")) {
      warnMsg("When enabling 'GENERATE CHM', 'GENERATE NAVIGATION TREEVIEW' tag must be disabled\n");

      auto iterBool = m_cfgBool.find("generate-treeview");
      bool data = iterBool.value().value;

      iterBool.value().value = false;
   }

   // **
   if (Config::getBool("html-search") && Config::getBool("generate-chm")) {
      warnMsg("When enabling 'Generate CHM', HTML Search' must be disabled\n");

      auto iterBool = m_cfgBool.find("html-search");
      bool data = iterBool.value().value;

      iterBool.value().value = false;
   }

   // **
   if (Config::getBool("inline-grouped-classes") && Config::getBool("separate-member-pages")) {
      err("Error when enabling 'INLINE GROUPED CLASSES', 'SEPARATE MEMBER PAGES' tags must be disabled\n");

      auto iterBool = m_cfgBool.find("separate-member-pages");
      bool data = iterBool.value().value;

      iterBool.value().value = false;
   }

   // **
   if (! Config::getBool("generate-html") && Config::getBool("generate-chm")) {
      warnMsg("GENERATE CHM requires Generate HTML to be set\n");
   }

   // **
   if (Config::getBool("optimize-java") && Config::getBool("inline-info")) {
      warnMsg("Java does not have an inline concept, setting 'INLINE INFO' off\n");

      auto iterBool = m_cfgBool.find("inline-info");
      bool data = iterBool.value().value;

      iterBool.value().value = false;
   }



   // ********** save data to structers and variables

   Doxy_Globals::parseSourcesNeeded = Config::getBool("dot-call") ||  Config::getBool("dot-called-by") ||
                  Config::getBool("ref-relation") || Config::getBool("ref-by-relation");

   Doxy_Globals::markdownSupport = Config::getBool("markdown");

   // **
   const QStringList expandAsDefinedList = Config::getList("expand-as-defined");

   for (auto item : expandAsDefinedList) {
      if (! Doxy_Globals::expandAsDefinedDict.contains(item)) {
         Doxy_Globals::expandAsDefinedDict.insert(item);
      }

   }

   // read command aliases, store in a dictionary
   loadCmd_Aliases();

   // read namespace aliases, store in a dictionary
   loadRenameNS_Aliases();

   return isOk;
}

void Config::cleanUpPaths(QStringList &str)
{
   for (auto &item : str) {
      item = item.trimmed();

      if (item.isEmpty()) {
         continue;
      }

      item.replace("\\", "/");

      if ((item.at(0) != '/' && (item.length() <= 2 || item.at(1) != ':')) || item.at(item.length() - 1) != '/' ) {
         QFileInfo fi(item);

         if (fi.exists() && fi.isDir()) {
            item = fi.absoluteFilePath() + "/";
         }
      }
   }
}

QStringList Config::getDotImageFormat()
{
   QStringList list;

   list.append("gif");
   list.append("jpg");
   list.append("png");
   list.append("svg");

   return list;
}

QStringList Config::getMathJaxFormat()
{
   QStringList list;

   list.append("HTML-CSS");
   list.append("NativeMML");
   list.append("SVG");

   return list;
}

QStringList Config::getLatexPaperType()
{
   QStringList list;

   list.append("a4");
   list.append("letter");
   list.append("legal");
   list.append("executive");

   return list;
}

QStringList Config::getRtfPaperType()
{
   QStringList list;

   list.append("a4");
   list.append("letter");
   list.append("legal");
   list.append("executive");

   return list;
}

QStringList Config::getAbbreviateBrief()
{
   QStringList list;

   list.append("The $name class");
   list.append("The $name widget");
   list.append("The $name file");
   list.append("is");
   list.append("provides");
   list.append("specifies");
   list.append("contains");
   list.append("represents");
   list.append("a");
   list.append("an");
   list.append("the");

   return list;
}

QStringList Config::getFilePatterns()
{
   QStringList list;

   list.append("*.as");
   list.append("*.c");
   list.append("*.cc");
   list.append("*.cpp");
   list.append("*.cxx");
   list.append("*.c++");
   list.append("*.cs");
   list.append("*.d");
   list.append("*.ddl");
   list.append("*.dox");
   list.append("*.for");
   list.append("*.f");
   list.append("*.f90");
   list.append("*.h");
   list.append("*.hh");
   list.append("*.hxx");
   list.append("*.hpp");
   list.append("*.h++");
   list.append("*.idl");
   list.append("*.ii");
   list.append("*.ixx");
   list.append("*.ipp");
   list.append("*.i++");
   list.append("*.inc");
   list.append("*.inl");
   list.append("*.java");
   list.append("*.js");
   list.append("*.m");
   list.append("*.md");
   list.append("*.mm");
   list.append("*.markdown");
   list.append("*.odl");
   list.append("*.php");
   list.append("*.php3");
   list.append("*.php4");
   list.append("*.php5");
   list.append("*.phtml");
   list.append("*.py");
   list.append("*.pyw");
   list.append("*.qsf");
   list.append("*.tcl");
   list.append("*.ucf");

   return list;
}

QStringList Config::getSuffixSource()
{
   QStringList list;

   list.append("c");
   list.append("cc");
   list.append("cxx");
   list.append("cpp");
   list.append("c++");
   list.append("ii");
   list.append("ixx");
   list.append("ipp");
   list.append("i++");
   list.append("inl");
   list.append("java");
   list.append("m");
   list.append("mm");
   list.append("xml");

   return list;
}

QStringList Config::getSuffixHeader()
{
   QStringList list;

   list.append("h");
   list.append("hh");
   list.append("hxx");
   list.append("hpp");
   list.append("h++");
   list.append("idl");
   list.append("ddl");
   list.append("pidl");

   return list;
}

QStringList Config::getSuffixExclude()
{
   QStringList list;

   list.append("doc");
   list.append("dox");
   list.append("md");
   list.append("markdown");
   list.append("txt");

   return list;
}

Qt::CaseSensitivity Config::getCase(const QString &name)
{
   Qt::CaseSensitivity isCase = Qt::CaseInsensitive;

   if (Config::getBool(name)) {
      isCase = Qt::CaseSensitive;
   }

   return isCase;
}

void Config::loadCmd_Aliases()
{
   // add aliases to a dictionary
   const QStringList list = Config::getList("aliases");

   for (auto item : list) {

      if (! Doxy_Globals::cmdAliasDict.contains(item)) {
         int i = item.indexOf('=');

         if (i > 0) {
            QString name  = item.left(i).trimmed();
            QString value = item.right(item.length() - i - 1);

            if (! name.isEmpty()) {
               // insert or update with the new alias
               Doxy_Globals::cmdAliasDict[name] = value;
            }
         }
      }
   }

   expandAliases();
   escapeAliases();
}

void Config::expandAliases()
{
   for (auto iter = Doxy_Globals::cmdAliasDict.begin(); iter != Doxy_Globals::cmdAliasDict.end(); ++iter) {
      *iter = expandAlias(iter.key(), *iter);
   }
}

void Config::escapeAliases()
{
   for (auto &s : Doxy_Globals::cmdAliasDict) {
      QString value = s;
      QString newValue;

      int in;
      int p = 0;

      // for each \n in the alias command value
      while ((in = value.indexOf("\\n", p)) != -1) {
         newValue += value.mid(p, in - p);

         // expand \n's except if \n is part of a built-in command


         if (value.mid(in, 5) != "\\note" && value.mid(in, 5) != "\\name" &&
               value.mid(in, 10) != "\\namespace" && value.mid(in, 14) != "\\nosubgrouping") {

            newValue += "\\_linebr ";

         } else {
            newValue += "\\n";

         }

         p = in + 2;
      }

      newValue += value.mid(p, value.length() - p);
      s = newValue;
   }
}

void Config::loadRenameNS_Aliases()
{
   // add aliases to a dictionary
   const QStringList list = Config::getList("ns-alias");

   for (auto item : list) {

      int i = item.indexOf('=');

      if (i > 0) {
         QString name  = item.left(i).trimmed();
         QString alias = item.right(item.length() - i - 1);

         if (! name.isEmpty() && ! alias.isEmpty()) {
            // insert or update with the alias
            Doxy_Globals::nsRenameOrig[name]   = alias;

            // insert or update with the name
            Doxy_Globals::nsRenameAlias[alias] = name;
         }
      }
   }
}