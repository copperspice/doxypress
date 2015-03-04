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

#include <config.h>
#include <doxy_setup.h>
#include <doxy_globals.h>
#include <language.h>
#include <message.h>
#include <pre.h>
#include <util.h>

bool Config::getBool(const QString &name)
{
   bool retval   = false;  
   auto hashIter = m_cfgBool.find(name);

    if (hashIter != m_cfgBool.end()) {
      retval = hashIter.value().value;

   } else { 
      fprintf(stderr, "Warning: %s was not retrieved from the project bool table\n", qPrintable(name) );
       
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
      fprintf(stderr, "Warning: %s was not retrieved from the project enum table\n", qPrintable(name) );
   
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
      fprintf(stderr, "Warning: %s was not retrieved from the project integer table\n", qPrintable(name) );
   
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
      fprintf(stderr, "Warning: %s was not retrieved from the project list table\n", qPrintable(name) );
   
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
      fprintf(stderr, "Warning: %s was not retrieved from the project string table\n", qPrintable(name) );
   
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
bool Config::preVerify()
{ 
   bool isError = false;

   // **
   if (! (Config::getBool("generate-html") || Config::getBool("generate-latex") || Config::getBool("generate-man") ||
          Config::getBool("generate-perl") || Config::getBool("generate-rtf")   || Config::getBool("generate-xml") ||
          Config::getBool("generate-autogen-def") || Config::getBool("generate-docbook")) && Config::getString("generate-tagfile").isEmpty() ) {

      err("No output format was selected, at least one output format must be set\n");
      isError = true;
   }
 
   // ** 
   auto iterString = m_cfgString.find("html-header");
   QString headerFile = iterString.value().value;

   if (! headerFile.isEmpty()) {
      QFileInfo fi(headerFile);

      if (! fi.exists()) {
         err("HTML HEADER file `%s' does not exist\n", qPrintable(headerFile)); 
         isError = true;
      }
   }

   // ** 
   iterString = m_cfgString.find("html-footer");
   QString footerFile = iterString.value().value;

   if (! footerFile.isEmpty()) {
      QFileInfo fi(footerFile);

      if (! fi.exists()) {
         err("HTML FOOTER file `%s' does not exist\n", qPrintable(footerFile));
         isError = true;
      }
   }

   // **   
   if (Config::getBool("use-mathjax")) {
      iterString = m_cfgString.find("mathjax-codefile");   
      QString mathJaxCodefile = iterString.value().value;

      if (! mathJaxCodefile.isEmpty()) {
         QFileInfo fi(mathJaxCodefile);

         if (! fi.exists()) {
            err("MATHJAX_CODEFILE file `%s' does not exist\n", qPrintable(mathJaxCodefile));
            isError = true;
         }
      }
   }
 

   // **
   iterString = m_cfgString.find("latex-header");
   QString latexHeaderFile = iterString.value().value;

   if (!latexHeaderFile.isEmpty()) {
      QFileInfo fi(latexHeaderFile);

      if (! fi.exists()) {
         err("LATEX HEADER file `%s' does not exist\n", qPrintable(latexHeaderFile));
         isError = true;
      }
   }

   return isError;
}

bool Config::verify()
{
   bool isError = false;

   // **
   auto iterString = m_cfgString.find("output-dir");
   QString outputDirectory = iterString.value().value;

   if (outputDirectory.isEmpty()) {
       outputDirectory = QDir::currentPath().toUtf8();
   
   } else {
      QDir dir(outputDirectory);

      if (! dir.exists()) {
         dir.setPath(QDir::currentPath());

         if (! dir.mkdir(outputDirectory)) {
            err("Output directory `%s' does not exist and can not be created\n", qPrintable(outputDirectory));            
            isError = true;

         } else {
            msg("Output directory `%s' was automatically created\n", qPrintable(outputDirectory));

         }

         dir.cd(outputDirectory);
      }

      outputDirectory = dir.absolutePath().toUtf8();
   }

   iterString.value().value = outputDirectory;


   // ** check include path
   auto iterList = m_cfgList.find("include-path");
   QStringList includePath = iterList.value().value;

   for (auto s : includePath) {   
      QFileInfo fi(s);

      if (fi.exists()) {
         addSearchDir(fi.absoluteFilePath());   

      } else {
         warnMsg("INCLUDE PATH `%s' does not exist\n", qPrintable(s));

      }          
   }

   iterList.value().value = includePath;


   // ** 
   iterString = m_cfgString.find("layout-file");
   QString layoutFileName = iterString.value().value;

   if (layoutFileName.isEmpty()) {
      layoutFileName    = "doxy_layout.xml";      
   }

   iterString.value().value = layoutFileName;

   // **
   iterString = m_cfgString.find("latex-bib-style");
   QString style = iterString.value().value;

   if (style.isEmpty()) {
      style = "plain";
   }

   iterString.value().value = style;

   // **
   auto iterEnum = m_cfgEnum.find("output-language");
   QString outputLanguage = iterEnum.value().value;

   if (outputLanguage.isEmpty()) {
      outputLanguage = "English";
   }

   if (! setTranslator(outputLanguage)) {
      warn_uncond("Output language %s not supported, using English\n", qPrintable(outputLanguage));
      outputLanguage = "English";
   }

   iterEnum.value().value = outputLanguage;

   
   // **
   iterString = m_cfgString.find("html-file-extension");
   QString htmlFileExtension = iterString.value().value.trimmed();

   if (htmlFileExtension.isEmpty()) {
      htmlFileExtension = ".html";
   }

   iterString.value().value   = htmlFileExtension;
   Doxygen::htmlFileExtension = htmlFileExtension;


   // **
   iterString = m_cfgString.find("warn-format");
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


   // **
   iterEnum = m_cfgEnum.find("mathjax-format");   
   QString mathJaxFormat  = iterEnum.value().value;
  
   if (! mathJaxFormat.isEmpty() && mathJaxFormat != "HTML-CSS" && mathJaxFormat != "NativeMML" && mathJaxFormat != "SVG") {
      err("Error: Unsupported value for MATHJAX FORMAT, value should be HTML-CSS, NativeMML, or SVG\n");

      iterEnum.value().value = "HTML-CSS";
   }


   // **
   iterString = m_cfgString.find("man-extension");
   QString manExtension = iterString.value().value;

   // set default man page extension if non is given by the user
   if (manExtension.isEmpty()) {
      manExtension = "3";

   } else {

      if (manExtension.at(0) == '.') {
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


   // **
   iterEnum = m_cfgEnum.find("latex-paper-type");
   QString paperType = iterEnum.value().value;

   paperType = paperType.toLower().trimmed();

   if (paperType.isEmpty()) {
      paperType = "a4";
   }

   if (paperType != "a4" && paperType != "letter" && paperType != "legal" && paperType != "executive") {
      warnMsg("Unknown paper type %s specified, using a4\n", qPrintable(paperType));
      paperType = "a4";
   }

   iterEnum.value().value = paperType;



   // ********** Save data to structers and variables   

   // **   
   Doxygen::parseSourcesNeeded = Config::getBool("dot-call") ||  Config::getBool("dot-called-by") ||
                                 Config::getBool("ref-relation") || Config::getBool("ref-by-relation");
   
   Doxygen::markdownSupport    = Config::getBool("markdown");


   // ** check input
   iterList = m_cfgList.find("input-source");
   QStringList inputSource = iterList.value().value;

   if (inputSource.count() == 0) {
      // use current dir as the default
      inputSource.append(QDir::currentPath());

   } else {    

      for (auto s : inputSource) {
         QFileInfo fi(s);

         if (! fi.exists()) {
            warnMsg("INPUT SOURCE `%s' does not exist\n", qPrintable(s));
         }         
      }
   }

   iterList.value().value = inputSource;


   // **
   if (Config::getBool("generate-treeview") && Config::getBool("generate-chm")) {
      warnMsg("When enabling 'GENERATE CHM', 'GENERATE TREEVIEW' tag must be disabled\n");

      auto iterBool = m_cfgBool.find("generate-treeeview");
      bool data = iterBool.value().value;

      iterBool.value().value = false;
   }

   // **
   if (Config::getBool("html-search") && Config::getBool("generate-chm")) {
      warnMsg("When enabling 'GENERATE CHM', HTML SEARCH' must be disabled\n");

      auto iterBool = m_cfgBool.find("html-search");
      bool data = iterBool.value().value;

      iterBool.value().value = false;
   }

   // **
   if (Config::getBool("inline-grouped-classes") && Config::getBool("separate-member-pages")) {
      err("Error: When enabling 'INLINE GROUPED CLASSES', 'SEPARATE MEMBER PAGES' taga must be disabled\n");

      auto iterBool = m_cfgBool.find("separate-member-pages");
      bool data = iterBool.value().value;

      iterBool.value().value = false;
   }


   // ** 
   if (! Config::getBool("generate-html") && Config::getBool("generate-chm")) {
      warnMsg("GENERATE CHM requires GENERATE HTML to be set\n");
   }


   // **
   if (Config::getBool("generate-qthelp")) {

      iterString = m_cfgString.find("qhp-namespace");
      QString temp = iterString.value().value;

      if (temp.isEmpty()) {
         err("When GENERATE Qt Help is set, QHP NAMESPACE can not be empty. Setting to the default value of 'org.doxypress.doc'\n");      

         iterString.value().value = "org.doxypress.doc";
      }

      iterString = m_cfgString.find("qhp-virtual-folder");
      temp = iterString.value().value;

      if (temp.isEmpty()) {
         err("When GENERATE Qt HELP is set, QHP VIRTUAL FOLDER can not be empty. Setting to the defualt of doc'\n");      
         
         iterString.value().value = "doc";
      }
   }

/*
   // ** expand the relative stripFromPath values
   auto iterList = m_cfgList.find("strip-from-path");
   QStringList stripFromPath = iterList.value().value;

   if ( stripFromPath.isEmpty() ) {       
      stripFromPath.append(QDir::currentPath() + "/");

   } else {
      cleanUpPaths(stripFromPath);

   }

   iterList.value().value = stripFromPath;
*/



/*

   // ** expand the relative stripFromPath values
   iterList = m_cfgList.find("strip-from-inc-path");
   QStringList stripFromIncPath = iterList.value().value;

   cleanUpPaths(stripFromIncPath);

   iterList.value().value = stripFromIncPath;
*/



   // **
   if (Config::getBool("optimize-java") && Config::getBool("inline-info")) {
      warnMsg("Java does have an inline concept, setting INLINE INFO to false\n");
     
      auto iterBool = m_cfgBool.find("inline-info");
      bool data = iterBool.value().value;

      iterBool.value().value = false;
   }


   // ** add predefined macro name to a dictionary
   const QStringList expandAsDefinedList = Config::getList("expand-as-defined");

   for (auto s : expandAsDefinedList) {
      if (! Doxygen::expandAsDefinedDict.contains(s)) {
         Doxygen::expandAsDefinedDict.insert(s);
      }      
   }

   // ** Add custom extension mappings 
   const QStringList extMaps = Config::getList("extension-mapping");
  
   for (auto mapStr : extMaps) { 
      int i = mapStr.indexOf('=');

      if (i != -1) {
         QString extension = mapStr.left(i).trimmed().toLower();
         QString language  = mapStr.mid(i + 1).trimmed().toLower();

         if (! updateLanguageMapping(extension, language)) {
            err("Unable to map file extension '%s' to '%s', verify the 'EXTENSION MAPPING' tag\n", 
                  qPrintable(extension), qPrintable(language));

         } else {
            msg("Adding custom extension mapping: .%s, will be treated as language %s\n", 
                  qPrintable(extension), qPrintable(language));
         }
      }
   } 


   // **
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
            err("PlantUml jar file 'plantuml.jar' was not found at the path specified by "
                   "PLANTUML JAR PATH '%s'\n", qPrintable(plantumlJarPath));

            iterString.value().value = QString();
         }

      } else if (file.exists() && file.isFile() && plantumlJarPath.endsWith(".jar")) { 
         // plantuml-jar-path is a file

         plantumlJarPath = file.absolutePath() + QDir::separator() ;
         iterString.value().value = plantumlJarPath;

      } else {
         err("'PLANTUML JAR PATH' does not exist and is not a directory: %s\n", qPrintable(plantumlJarPath));         
         iterString.value().value = QString();
      }
   }
   
   // read aliases and store them in a dictionary
   readAliases();

   return isError;
}




  
/*  
   // check aliases
   QStringList &aliasList = Config_getList("ALIASES");
 
   for (auto alias: aliasList) {
      QRegExp re1("[a-z_A-Z][a-z_A-Z0-9]*[ \t]*=");         // alias without argument
      QRegExp re2("[a-z_A-Z][a-z_A-Z0-9]*\\{[0-9]*\\}[ \t]*="); // alias with argument
      
      alias = alias.trimmed();

      if (! (re1.indexIn(alias) == 0 || re2.indexIn(alias) == 0)) {
         err("Error: Alias format: `%s' \n is invalid, use \"name=value\" or \"name{n}=value\", where n "
                  "is the number of arguments\n\n", qPrintable(alias.));
      }
      
   }
  
   // ** check dot image format
   iterEnum = m_cfgEnum.find("dot-image-format");
   QString dotImageFormat = 

   dotImageFormat = dotImageFormat.trimmed();

   if (dotImageFormat.isEmpty()) {
      dotImageFormat = "png";
   }

   if (dotImageFormat!="gif" && dotImageFormat!="png" && dotImageFormat!="jpg") {
      err("Invalid value for DOT_IMAGE_FORMAT: `%s'. Using the default.\n",dotImageFormat.data());

      dotImageFormat = "png";
   }

   // **
   QString dotFontName = Config_getString("DOT_FONTNAME");

   if (dotFontName == "FreeSans" || dotFontName == "FreeSans.ttf") {

      err("Warning: doxygen no longer ships with the FreeSans font.\n"
                 "You may want to clear or change 'DOT FONTNAME'\n"
                 "Otherwise you run the risk that the wrong font is being used for dot generated graphs.\n");
   }


   // check dot path
   iterString = m_cfgString.find("");
   QByteArray &dotPath = Config_getString("DOT_PATH");

   if (! dotPath.isEmpty()) {
      QFileInfo fi(dotPath);

      if (fi.exists() && fi.isFile()) { // user specified path + exec
         dotPath = fi.absolutePath().toUtf8() + "/";

      } else {
         QFileInfo dp(dotPath + "/dot" + portable_commandExtension());

         if (!dp.exists() || !dp.isFile()) {
            err("Warning: the dot tool could not be found at %s\n", dotPath.data());
            dotPath = "";

         } else {
            dotPath = dp.absolutePath() + "/";
         }
      }

   } else {      
      dotPath = "";
   }


   // check mscgen path
   iterString = m_cfgString.find("");
   QString mscgenPath = Config_getString("MSCGEN_PATH");

   if (!mscgenPath.isEmpty()) {
      QFileInfo dp(mscgenPath + "/mscgen" + portable_commandExtension());

      if (! dp.exists() || !dp.isFile()) {
         warnMsg("MSCGEN ool could not be found at %s\n", mscgenPath.data());
         mscgenPath = "";

      } else {
         mscgenPath = dp.absolutePath() + "/";

      }

   } else { 
      mscgenPath = "";
   }


   // check dia path
   iterString = m_cfgString.find("");
   QString diaPath = Config_getString("DIA_PATH");

   if (! diaPath.isEmpty()) {
      QFileInfo dp(diaPath + "/dia" + portable_commandExtension());

      if (! dp.exists() || ! dp.isFile()) {
         warnMsg("DIA could not be found at %s\n", diaPath.data());
         diaPath = "";

      } else {
         diaPath = dp.absolutePath() + "/";

      }

   } else { 
      diaPath = "";
   }



   // add default pattern if needed
   QStringList &filePatternList = Config_getList("FILE_PATTERNS");
   if (filePatternList.isEmpty()) {
      filePatternList.append("*.c");
      filePatternList.append("*.cc");
      filePatternList.append("*.cxx");
      filePatternList.append("*.cpp");
      filePatternList.append("*.c++");
      //filePatternList.append("*.d");
      filePatternList.append("*.java");
      filePatternList.append("*.ii");
      filePatternList.append("*.ixx");
      filePatternList.append("*.ipp");
      filePatternList.append("*.i++");
      filePatternList.append("*.inl");
      filePatternList.append("*.h");
      filePatternList.append("*.hh");
      filePatternList.append("*.hxx");
      filePatternList.append("*.hpp");
      filePatternList.append("*.h++");
      filePatternList.append("*.idl");
      filePatternList.append("*.odl");
      filePatternList.append("*.cs");
      filePatternList.append("*.php");
      filePatternList.append("*.php3");
      filePatternList.append("*.inc");
      filePatternList.append("*.m");
      filePatternList.append("*.mm");
      filePatternList.append("*.dox");
      filePatternList.append("*.py");
      filePatternList.append("*.f90");
      filePatternList.append("*.f");
      filePatternList.append("*.for");
      filePatternList.append("*.tcl");
      filePatternList.append("*.md");
      filePatternList.append("*.markdown");

      if (portable_fileSystemIsCaseSensitive() == Qt::CaseSensitive) {
         // unix => case sensitive match => also include useful uppercase versions

         filePatternList.append("*.C");
         filePatternList.append("*.CC");
         filePatternList.append("*.C++");
         filePatternList.append("*.II");
         filePatternList.append("*.I++");
         filePatternList.append("*.H");
         filePatternList.append("*.HH");
         filePatternList.append("*.H++");
         filePatternList.append("*.CS");
         filePatternList.append("*.PHP");
         filePatternList.append("*.PHP3");
         filePatternList.append("*.M");
         filePatternList.append("*.MM");
         filePatternList.append("*.PY");
         filePatternList.append("*.F90");
         filePatternList.append("*.F");
         filePatternList.append("*.TCL");
         filePatternList.append("*.MD");
         filePatternList.append("*.MARKDOWN");
      }
   }

   // add default pattern if needed
   QStringList &examplePatternList = Config_getList("EXAMPLE_PATTERNS");
   if (examplePatternList.isEmpty()) {
      examplePatternList.append("*");
   }


   int &depth = Config_getInt("MAX_DOT_GRAPH_DEPTH");
   if (depth == 0) {
      depth = 1000;
   }

   int hue = Config::getInt("html-colorstyle-hue");
   if (hue < 0) {
      hue = 0;
   } else if (hue >= 360) {
      hue = hue % 360;
   }

   int sat = Config::getInt("html-colorstyle-sat");
   if (sat < 0) {
      sat = 0;
   } else if (sat > 255) {
      sat = 255;
   }
   int gamma = Config::getInt("html-colorstyle-gamma");
   if (gamma < 40) {
      gamma = 40;
   } else if (gamma > 240) {
      gamma = 240;
   }
 
   // add default words if needed
   QStringList &annotationFromBrief = Config_getList("ABBREVIATE_BRIEF");
   if (annotationFromBrief.isEmpty()) {
      annotationFromBrief.append("The $name class");
      annotationFromBrief.append("The $name widget");
      annotationFromBrief.append("The $name file");
      annotationFromBrief.append("is");
      annotationFromBrief.append("provides");
      annotationFromBrief.append("specifies");
      annotationFromBrief.append("contains");
      annotationFromBrief.append("represents");
      annotationFromBrief.append("a");
      annotationFromBrief.append("an");
      annotationFromBrief.append("the");
   }
     
*/



/*
static void cleanUpPaths(QStringList &str)
{
   for (auto &sfp : str) {  

      if (sfp.isEmpty()) {
         continue;
      }

      sfp.replace("\\", "/");
    
      QByteArray path = sfp.toUtf8();

      if ((path.at(0) != '/' && (path.length() <= 2 || path.at(1) != ':')) || path.at(path.length() - 1) != '/' ) {

         QFileInfo fi(path);

         if (fi.exists() && fi.isDir()) {
            sfp = fi.absoluteFilePath() + "/";           
         }
      }      
   }
}

*/

