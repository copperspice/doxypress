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

#include <config.h>
#include <doxy_setup.h>
#include <doxy_globals.h>
#include <language.h>
#include <message.h>
#include <pre.h>
#include <util.h>

static void cleanUpPaths(QStringList &str);

static QString getAbbreviateBrief();
static QStringList getDotImageFormat();
static QStringList getMathJaxFormat();
static QStringList getLatexPaperType();

const static QStringList s_dotImageFormat = getDotImageFormat();
const static QStringList s_mathJaxFormat  = getMathJaxFormat();
const static QStringList s_latexPaperType = getLatexPaperType();

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
          Config::getBool("generate-autogen-def") || Config::getBool("generate-docbook")) && 
          Config::getString("generate-tagfile").isEmpty() ) {

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
   iterString = m_cfgString.find("latex-header");
   QString latexHeaderFile = iterString.value().value;

   if (! latexHeaderFile.isEmpty()) {
      QFileInfo fi(latexHeaderFile);

      if (! fi.exists()) {
         err("LATEX HEADER file `%s' does not exist\n", qPrintable(latexHeaderFile));
         isError = true;
      }
   }

   // **
   iterString = m_cfgString.find("latex-footer");
   QString latexFooterFile = iterString.value().value;

   if (! latexFooterFile.isEmpty()) {
      QFileInfo fi(latexFooterFile);

      if (! fi.exists()) {
         err("LATEX FOOTER file `%s' does not exist\n", qPrintable(latexFooterFile));
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
            err("MATHJAX CODEFILE file `%s' does not exist\n", qPrintable(mathJaxCodefile));
            isError = true;
         }
      }
   }

   return isError;
}

bool Config::verify()
{
   bool isError = false;
 
   // ** project
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


   // ** general
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
      QRegExp re1("[a-z_A-Z][a-z_A-Z0-9]*[ \t]*=");               // alias without argument
      QRegExp re2("[a-z_A-Z][a-z_A-Z0-9]*\\{[0-9]*\\}[ \t]*=");   // alias with argument
      
      alias = alias.trimmed();

      if (! (re1.indexIn(alias) == 0 || re2.indexIn(alias) == 0)) {
         err("Alias format: `%s' \n is invalid, use \"name=value\" or \"name{n}=value\", where n "
                  "is the number of arguments\n\n", qPrintable(alias));
      }      
   }


   // ** 
   const QStringList extMaps = Config::getList("language-mapping");
  
   for (auto mapStr : extMaps) { 
      int i = mapStr.indexOf('=');

      if (i != -1) {
         QString extension = mapStr.left(i).trimmed().toLower();
         QString language  = mapStr.mid(i + 1).trimmed().toLower();

         if (! updateLanguageMapping(extension, language)) {
            err("Unable to map file extension '%s' to '%s', verify the EXTENSION MAPPING tag\n", 
                  qPrintable(extension), qPrintable(language));

         } else {
            msg("Adding custom extension mapping: .%s, will be treated as language %s\n", 
                  qPrintable(extension), qPrintable(language));
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
         err("LAYOUT FILE file `%s' does not exist\n", qPrintable(layoutFileName));
         isError = true;
      }
   }

   iterString.value().value = layoutFileName;


   // ** messages
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
            warnMsg("INPUT SOURCE `%s' does not exist\n", qPrintable(item));
         }         
      }
   }

   iterList.value().value = inputSource;


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


   // ** preprocessing
   iterList = m_cfgList.find("include-path");
   QStringList includePath = iterList.value().value;

   for (auto item : includePath) {   
      QFileInfo fi(item.trimmed());

      if (fi.exists()) {
         addSearchDir(fi.absoluteFilePath());   

      } else {
         warnMsg("INCLUDE PATH `%s' does not exist\n", qPrintable(item));

      }          
   }

   iterList.value().value = includePath;


   // ** dot    
   iterEnum = m_cfgEnum.find("dot-image-format");             
   QString dotImageFormat = iterEnum.value().value;

   dotImageFormat = dotImageFormat.trimmed();

   if (dotImageFormat.isEmpty()) {
      dotImageFormat = "png";

   } else if (! s_dotImageFormat.contains(dotImageFormat)) {
      err("Invalid value of %s for DOT IMAGE FORMAT, using the the default of png\n", qPrintable(dotImageFormat));

      dotImageFormat = "png";
   }

   iterEnum.value().value = dotImageFormat;


   // 
   iterString = m_cfgString.find("mscgen-path");
   QString mscgenPath = iterString.value().value;

   if (! mscgenPath.isEmpty()) {
      QFileInfo dp(mscgenPath + "/mscgen" + portable_commandExtension());

      if (! dp.exists() || ! dp.isFile()) {
         warnMsg("MSCGEN program could not be found at %s\n", mscgenPath.data());
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
         warnMsg("DIA program could not be found at %s\n", qPrintable(diaPath));
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
         dotPath = fi.absolutePath().toUtf8() + "/";

      } else {
         QFileInfo dp(dotPath + "/dot" + portable_commandExtension());            

         if (! dp.exists() || !dp.isFile()) {
            err("Unable to locate DOT program at %s\n", qPrintable(dotPath));
            dotPath = "";

         } else {
            dotPath = dp.absolutePath() + "/";
         }
      }

   } else {      
      dotPath = "";

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
            err("PlantUml jar file 'plantuml.jar' was not found at the path specified by "
                   "PLANTUML JAR PATH '%s'\n", qPrintable(plantumlJarPath));

            iterString.value().value = QString();
         }

      } else if (file.exists() && file.isFile() && plantumlJarPath.endsWith(".jar")) { 
         // plantuml-jar-path is a file

         plantumlJarPath = file.absolutePath() + QDir::separator() ;
         iterString.value().value = plantumlJarPath;

      } else {
         err("PLANTUML JAR PATH does not exist and is not a directory: %s\n", qPrintable(plantumlJarPath));         
         iterString.value().value = QString();
      }
   }

   // **
   auto iterInt = m_cfgInt.find("dot-graph-max-nodes");
   int depth = iterInt.value().value;
   
   if (depth == 0) {
      err("DOT GRAPH MAX nodes was greater than maximum, setting to 1000");  

      iterInt.value().value = 1000;
   }
   
   
   // ** html
   iterString = m_cfgString.find("html-file-extension");
   QString htmlFileExtension = iterString.value().value.trimmed();

   if (htmlFileExtension.isEmpty()) {
      htmlFileExtension = ".html";
   }

   iterString.value().value   = htmlFileExtension;
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
      err("Invalid value of %s for MATHJAX FORMAT, using the the default of HTML-CSS", qPrintable(mathJaxFormat));
  
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

   if (paperType.isEmpty()) {
      paperType = "a4";

   } else if (! s_latexPaperType.contains(paperType)) {
      err("Invalid value of %s for LATEX PAPER TYPE, using the the default of a4\n", qPrintable(paperType));

      paperType = "a4";
   }

   iterEnum.value().value = paperType;


   // **  man
   iterString = m_cfgString.find("man-extension");
   QString manExtension = iterString.value().value;
 
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


   // ** qt help
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

   // **********  

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
   if (Config::getBool("optimize-java") && Config::getBool("inline-info")) {
      warnMsg("Java does have an inline concept, setting INLINE INFO to false\n");
     
      auto iterBool = m_cfgBool.find("inline-info");
      bool data = iterBool.value().value;

      iterBool.value().value = false;
   }

  

   // ********** Save data to structers and variables   
  
   Doxy_Globals::parseSourcesNeeded = Config::getBool("dot-call") ||  Config::getBool("dot-called-by") ||
                                 Config::getBool("ref-relation") || Config::getBool("ref-by-relation");
   
   Doxy_Globals::markdownSupport    = Config::getBool("markdown");


   // ** 
   const QStringList expandAsDefinedList = Config::getList("expand-as-defined");

   for (auto item : expandAsDefinedList) {
      // add predefined macro names to dictionary

      if (! Doxy_Globals::expandAsDefinedDict.contains(item)) {         
         Doxy_Globals::expandAsDefinedDict.insert(item);
      }      

   }  
   
   // read aliases and store them in a dictionary
   readAliases();

   return isError;
}

static void cleanUpPaths(QStringList &str)
{
   for (auto &item : str) {  

      if (item.isEmpty()) {
         continue;
      }

      item.replace("\\", "/");         
      item = item.trimmed();

      if ((item.at(0) != '/' && (item.length() <= 2 || item.at(1) != ':')) || item.at(item.length() - 1) != '/' ) {
         QFileInfo fi(item);

         if (fi.exists() && fi.isDir()) {
            item = fi.absoluteFilePath() + "/";           
         }
      }      
   }
}

static QStringList getDotImageFormat()
{
   QStringList list;

   list.append("gif");
   list.append("jpg");
   list.append("png");
   list.append("svg");
   
   return list;
}

static QStringList getMathJaxFormat()
{
   QStringList list;

   list.append("HTML-CSS");
   list.append("NativeMML");
   list.append("SVG");  

   return list;
}

static QStringList getLatexPaperType()
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
   list.append("*.qsf");
   list.append("*.tcl");
   list.append("*.ucf");

   return list;
}
