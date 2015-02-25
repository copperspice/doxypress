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
//   #include <doxy_build_info.h>
#include <message.h>
//   #include <portable.h>
//   #include <util.h>

bool Config::getBool(const QString name)
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

QString Config::getEnum(const QString name)
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

int Config::getInt(const QString name)
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

QStringList Config::getList(const QString name)
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

QString Config::getString(const QString name)
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
void Config::setList(const QString name, QStringList data)
{
   auto hashIter = m_cfgList.find(name);     
   hashIter.value().value = data;
}


void Config::verify()
{
   printf("**  Adjust Project Configuration\n");   

// BROOM   Config::instance()->substituteEnvironmentVars();     
// BROOM   Config::instance()->convertStrToVal();
// BROOM   Config::instance()->check();

   initWarningFormat();

   // **
   auto hashStrIter = m_cfgString.find("output-dir");
   QString outputDirectory = hashStrIter.value().value;

   if (outputDirectory.isEmpty()) {
       outputDirectory = QDir::currentPath().toUtf8();
   
   } else {
      QDir dir(outputDirectory);

      if (! dir.exists()) {
         dir.setPath(QDir::currentPath());

         if (! dir.mkdir(outputDirectory)) {
            err("Output directory `%s' does not exist and can not be created\n", qPrintable(outputDirectory));            
            exit(1);

         } else {
            msg("Output directory `%s' was automatically created\n", qPrintable(outputDirectory));

         }

         dir.cd(outputDirectory);
      }

      outputDirectory = dir.absolutePath().toUtf8();
   }

   hashStrIter.value().value = outputDirectory;

   // ** 
   hashStrIter = m_cfgString.find("layout-file");
   QString layoutFileName = hashStrIter.value().value;

   if (layoutFileName.isEmpty()) {
      layoutFileName    = "doxy_layout.xml";      
   }

   hashStrIter.value().value = layoutFileName;

   // **
   hashStrIter = m_cfgString.find("latex-bib-style");
   QString style = hashStrIter.value().value;

   if (style.isEmpty()) {
      style = "plain";
   }

   hashStrIter.value().value = style;





/*


// BROOM CHECK



   QByteArray outputLanguage = Config_getEnum("OUTPUT_LANGUAGE");

   if (!setTranslator(outputLanguage)) {
      warn_uncond("Output language %s not supported, using English\n", outputLanguage.data());
   }

   QStringList &includePath = Config_getList("INCLUDE_PATH");
 
   for (auto s : includePath) { 
      QFileInfo fi(s);
      addSearchDir(fi.absoluteFilePath().toUtf8());      
   }

   // Set the global html file extension 
   Doxygen::htmlFileExtension = Config_getString("HTML_FILE_EXTENSION");

   Doxygen::parseSourcesNeeded = Config_getBool("CALL_GRAPH") ||  Config_getBool("CALLER_GRAPH") ||
                                 Config_getBool("REFERENCES_RELATION") || Config_getBool("REFERENCED_BY_RELATION");

   Doxygen::markdownSupport = Config_getBool("MARKDOWN_SUPPORT");

   // Add custom extension mappings 
   QStringList &extMaps = Config_getList("EXTENSION_MAPPING");
  
   for (auto mapping : extMaps) {
      QString mapStr = mapping;
      int i;

      if ((i = mapStr.indexOf('=')) != -1) {
         QString ext = mapStr.left(i).trimmed().toLower();
         QString language = mapStr.mid(i + 1).trimmed().toLower();

         if (! updateLanguageMapping(ext.toUtf8(), language.toUtf8())) {
            err("Unable to map file extension '%s' to '%s', verify the EXTENSION_MAPPING settings\n", qPrintable(ext), qPrintable(language));

         } else {
            msg("Adding custom extension mapping: .%s will be treated as language %s\n", qPrintable(ext), qPrintable(language));
         }
      }
   }     

   // add predefined macro name to a dictionary
   QStringList &expandAsDefinedList = Config_getList("EXPAND_AS_DEFINED");

   for (auto s : expandAsDefinedList) {
      if (Doxygen::expandAsDefinedDict[s] == 0) {
         Doxygen::expandAsDefinedDict.insert(s, (void *)666);
      }      
   }

   // read aliases and store them in a dictionary
   readAliases();

*/


}




/*

void config_err(const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   vfprintf(stderr, fmt, args);
   va_end(args);
}

void config_warn(const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   vfprintf(stderr, fmt, args);
   va_end(args);
}


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


/*


void Config::checkFileName(const char *optionName)
{
   QByteArray &s = Config_getString(optionName);
   QByteArray val = s.trimmed().toLower();

   if ((val == "yes" || val == "true"  || val == "1" || val == "all") ||
         (val == "no"  || val == "false" || val == "0" || val == "none")) {

      config_err("Error: file name expected for option %s, got %s instead. Ignoring...\n", optionName, s.data());
      s = ""; // note tihe use of &s above: this will change the option value!
   }
}

*/


/*

void Config::check()
{
   QByteArray &warnFormat = Config_getString("WARN_FORMAT");

   if (warnFormat.trimmed().isEmpty()) {
      warnFormat = "$file:$line $text";
   } else {
      if (warnFormat.indexOf("$file") == -1) {
         config_err("Warning: warning format does not contain a $file tag!\n");
      }
      if (warnFormat.indexOf("$line") == -1) {
         config_err("Warning: warning format does not contain a $line tag!\n");
      }
      if (warnFormat.indexOf("$text") == -1) {
         config_err("Warning: warning format foes not contain a $text tag!\n");
      }
   }

   QByteArray &manExtension = Config_getString("MAN_EXTENSION");

   // set default man page extension if non is given by the user
   if (manExtension.isEmpty()) {
      manExtension = ".3";
   }

   QByteArray &paperType = Config_getEnum("PAPER_TYPE");
   paperType = paperType.toLower().trimmed();
   if (paperType.isEmpty()) {
      paperType = "a4";
   }
   if (paperType != "a4" && paperType != "a4wide" && paperType != "letter" &&
         paperType != "legal" && paperType != "executive") {
      config_err("Error: Unknown page type specified\n");
   }

   QByteArray &outputLanguage = Config_getEnum("OUTPUT_LANGUAGE");
   outputLanguage = outputLanguage.trimmed();
   if (outputLanguage.isEmpty()) {
      outputLanguage = "English";
   }

   QByteArray &htmlFileExtension = Config_getString("HTML_FILE_EXTENSION");
   htmlFileExtension = htmlFileExtension.trimmed();
   if (htmlFileExtension.isEmpty()) {
      htmlFileExtension = ".html";
   }

   // expand the relative stripFromPath values
   QStringList &stripFromPath = Config_getList("STRIP_FROM_PATH");

   if ( stripFromPath.isEmpty() ) { 
      // by default use the current path
      stripFromPath.append(QDir::currentPath().toUtf8() + "/");

   } else {
      cleanUpPaths(stripFromPath);

   }

   // expand the relative stripFromPath values
   QStringList &stripFromIncPath = Config_getList("STRIP_FROM_INC_PATH");
   cleanUpPaths(stripFromIncPath);

   // Test to see if HTML header is valid
   QByteArray &headerFile = Config_getString("HTML_HEADER");
   if (!headerFile.isEmpty()) {
      QFileInfo fi(headerFile);
      if (!fi.exists()) {
         config_err("Error: tag HTML_HEADER: header file `%s' "
                    "does not exist\n", headerFile.data());
         exit(1);
      }
   }
   // Test to see if HTML footer is valid
   QByteArray &footerFile = Config_getString("HTML_FOOTER");
   if (!footerFile.isEmpty()) {
      QFileInfo fi(footerFile);
      if (!fi.exists()) {
         config_err("Error: tag HTML_FOOTER: footer file `%s' "
                    "does not exist\n", footerFile.data());
         exit(1);
      }
   }
   // Test to see if MathJax code file is valid
   if (Config_getBool("USE_MATHJAX")) {
      QByteArray &MathJaxCodefile = Config_getString("MATHJAX_CODEFILE");

      if (!MathJaxCodefile.isEmpty()) {
         QFileInfo fi(MathJaxCodefile);

         if (!fi.exists()) {
            config_err("Error: tag MATHJAX_CODEFILE file `%s' "
                       "does not exist\n", MathJaxCodefile.data());
            exit(1);
         }
      }
   }
   // Test to see if LaTeX header is valid
   QByteArray &latexHeaderFile = Config_getString("LATEX_HEADER");
   if (!latexHeaderFile.isEmpty()) {
      QFileInfo fi(latexHeaderFile);
      if (!fi.exists()) {
         config_err("Error: tag LATEX_HEADER: header file `%s' "
                    "does not exist\n", latexHeaderFile.data());
         exit(1);
      }
   }
   // check include path
   QStringList &includePath = Config_getList("INCLUDE_PATH");

   for (auto s : includePath) {   
      QFileInfo fi(s);

      if (! fi.exists()) {
         config_err("Warning: tag INCLUDE_PATH: include path `%s' does not exist\n", qPrintable(s));
      }  
   }

   // check aliases
   QStringList &aliasList = Config_getList("ALIASES");
 
   for (auto s: aliasList) {
      QRegExp re1("[a-z_A-Z][a-z_A-Z0-9]*[ \t]*=");         // alias without argument
      QRegExp re2("[a-z_A-Z][a-z_A-Z0-9]*\\{[0-9]*\\}[ \t]*="); // alias with argument

      QByteArray alias = s.toUtf8();
      alias = alias.trimmed();

      if (! (re1.indexIn(alias) == 0 || re2.indexIn(alias) == 0)) {
         config_err("Error: Alias format: `%s' \nForma is invalid, use \"name=value\" or \"name{n}=value\", where n is the number of arguments\n\n",
                    alias.data());
      }
      
   }

   // check if GENERATE_TREEVIEW and GENERATE_HTMLHELP are both enabled
   if (Config_getBool("GENERATE_TREEVIEW") && Config_getBool("GENERATE_HTMLHELP")) {
      config_err("Error: When enabling GENERATE_HTMLHELP the tree view (GENERATE_TREEVIEW) should be disabled. I'll do it for you.\n");
      Config_getBool("GENERATE_TREEVIEW") = FALSE;
   }
   if (Config_getBool("SEARCHENGINE") && Config_getBool("GENERATE_HTMLHELP")) {
      config_err("Error: When enabling GENERATE_HTMLHELP the search engine (SEARCHENGINE) should be disabled. I'll do it for you.\n");
      Config_getBool("SEARCHENGINE") = FALSE;
   }

   // check if SEPARATE_MEMBER_PAGES and INLINE_GROUPED_CLASSES are both enabled
   if (Config_getBool("SEPARATE_MEMBER_PAGES") && Config_getBool("INLINE_GROUPED_CLASSES")) {
      config_err("Error: When enabling INLINE_GROUPED_CLASSES the SEPARATE_MEMBER_PAGES option should be disabled. I'll do it for you.\n");
      Config_getBool("SEPARATE_MEMBER_PAGES") = FALSE;
   }

   // check dot image format
   QByteArray &dotImageFormat = Config_getEnum("DOT_IMAGE_FORMAT");
   dotImageFormat = dotImageFormat.trimmed();
   if (dotImageFormat.isEmpty()) {
      dotImageFormat = "png";
   }
   //else if (dotImageFormat!="gif" && dotImageFormat!="png" && dotImageFormat!="jpg")
   //{
   //  config_err("Invalid value for DOT_IMAGE_FORMAT: `%s'. Using the default.\n",dotImageFormat.data());
   //  dotImageFormat = "png";
   //}

   QByteArray &dotFontName = Config_getString("DOT_FONTNAME");
   if (dotFontName == "FreeSans" || dotFontName == "FreeSans.ttf") {
      config_err("Warning: doxygen no longer ships with the FreeSans font.\n"
                 "You may want to clear or change DOT_FONTNAME.\n"
                 "Otherwise you run the risk that the wrong font is being used for dot generated graphs.\n");
   }


   // check dot path
   QByteArray &dotPath = Config_getString("DOT_PATH");

   if (! dotPath.isEmpty()) {

      QFileInfo fi(dotPath);
      if (fi.exists() && fi.isFile()) { // user specified path + exec
         dotPath = fi.absolutePath().toUtf8() + "/";

      } else {
         QFileInfo dp(dotPath + "/dot" + portable_commandExtension());
         if (!dp.exists() || !dp.isFile()) {
            config_err("Warning: the dot tool could not be found at %s\n", dotPath.data());
            dotPath = "";
         } else {
            dotPath = dp.absolutePath().toUtf8() + "/";
         }
      }
#if defined(_WIN32) // convert slashes
      uint i = 0, l = dotPath.length();

      for (i = 0; i < l; i++) if (dotPath.at(i) == '/') {
            dotPath[i] = '\\';
         }
#endif

   } else { // make sure the string is empty but not null!
      dotPath = "";
   }

   // check mscgen path
   QByteArray &mscgenPath = Config_getString("MSCGEN_PATH");
   if (!mscgenPath.isEmpty()) {
      QFileInfo dp(mscgenPath + "/mscgen" + portable_commandExtension());

      if (!dp.exists() || !dp.isFile()) {
         config_err("Warning: the mscgen tool could not be found at %s\n", mscgenPath.data());
         mscgenPath = "";

      } else {
         mscgenPath = dp.absolutePath().toUtf8() + "/";

#if defined(_WIN32) // convert slashes
         uint i = 0, l = mscgenPath.length();

         for (i = 0; i < l; i++) if (mscgenPath.at(i) == '/') {
               mscgenPath[i] = '\\';
            }
#endif
      }
   } else { // make sure the string is empty but not null!
      mscgenPath = "";
   }

   // check plantuml path
   QByteArray &plantumlJarPath = Config_getString("PLANTUML_JAR_PATH");

   if (!plantumlJarPath.isEmpty()) {
      QFileInfo pu(plantumlJarPath);

      if (pu.exists() && pu.isDir()) { // PLANTUML_JAR_PATH is directory
         QFileInfo jar(plantumlJarPath + portable_pathSeparator() + "plantuml.jar");

         if (jar.exists() && jar.isFile()) {
            plantumlJarPath = jar.absolutePath().toUtf8() + portable_pathSeparator();
         } else {
            config_err("Jar file plantuml.jar not found at location "
                       "specified via PLANTUML_JAR_PATH: '%s'\n", plantumlJarPath.data());
            plantumlJarPath = "";
         }
      } else if (pu.exists() && pu.isFile() && plantumlJarPath.right(4) == ".jar") { // PLANTUML_JAR_PATH is file
         plantumlJarPath = pu.absolutePath().toUtf8() + portable_pathSeparator();
      } else {
         config_err("path specified via PLANTUML_JAR_PATH does not exist or not a directory: %s\n",
                    plantumlJarPath.data());
         plantumlJarPath = "";
      }
   }

   // check dia path
   QByteArray &diaPath = Config_getString("DIA_PATH");

   if (!diaPath.isEmpty()) {
      QFileInfo dp(diaPath + "/dia" + portable_commandExtension());

      if (!dp.exists() || !dp.isFile()) {
         config_err("Warning: dia could not be found at %s\n", diaPath.data());
         diaPath = "";

      } else {
         diaPath = dp.absolutePath().toUtf8() + "/";

#if defined(_WIN32) // convert slashes
         uint i = 0, l = diaPath.length();

         for (i = 0; i < l; i++) {
            if (diaPath.at(i) == '/') {
               diaPath[i] = '\\';
            }
         }
#endif
      }

   } else { // make sure the string is empty but not null!
      diaPath = "";
   }

   // check input
   QStringList &inputSources = Config_getList("INPUT");

   if (inputSources.count() == 0) {
      // use current dir as the default
      inputSources.append(QDir::currentPath().toUtf8());

   } else {    
      for (auto s : inputSources) {
         QFileInfo fi(s);

         if (!fi.exists()) {
            config_err("Warning: tag INPUT: input source `%s' does not exist\n", qPrintable(s));
         }         
      }
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

   // if no output format is enabled, warn the user
   if (!Config_getBool("GENERATE_HTML")    &&
         !Config_getBool("GENERATE_LATEX")   &&
         !Config_getBool("GENERATE_MAN")     &&
         !Config_getBool("GENERATE_RTF")     &&
         !Config_getBool("GENERATE_XML")     &&
         !Config_getBool("GENERATE_PERLMOD") &&
         !Config_getBool("GENERATE_RTF")     &&
         !Config_getBool("GENERATE_AUTOGEN_DEF") &&
         Config_getString("GENERATE_TAGFILE").isEmpty()
      ) {
      config_err("Warning: No output formats selected! Set at least one of the main GENERATE_* options to YES.\n");
   }

   // check HTMLHELP creation requirements
   if (!Config_getBool("GENERATE_HTML") &&
         Config_getBool("GENERATE_HTMLHELP")) {
      config_err("Warning: GENERATE_HTMLHELP=YES requires GENERATE_HTML=YES.\n");
   }

   // check QHP creation requirements
   if (Config_getBool("GENERATE_QHP")) {
      if (Config_getString("QHP_NAMESPACE").isEmpty()) {
         config_err("Error: GENERATE_QHP=YES requires QHP_NAMESPACE to be set. Using 'org.doxypress.doc' as default.\n");
         Config_getString("QHP_NAMESPACE") = "org.doxypress.doc";
      }

      if (Config_getString("QHP_VIRTUAL_FOLDER").isEmpty()) {
         config_err("Error: GENERATE_QHP=YES requires QHP_VIRTUAL_FOLDER to be set. Using 'doc' as default\n");
         Config_getString("QHP_VIRTUAL_FOLDER") = "doc";
      }
   }

   if (Config_getBool("OPTIMIZE_OUTPUT_JAVA") && Config_getBool("INLINE_INFO")) {
      // don't show inline info for Java output, since Java has no inline
      // concept.
      Config_getBool("INLINE_INFO") = FALSE;
   }

   int &depth = Config_getInt("MAX_DOT_GRAPH_DEPTH");
   if (depth == 0) {
      depth = 1000;
   }

   int &hue = Config_getInt("HTML_COLORSTYLE_HUE");
   if (hue < 0) {
      hue = 0;
   } else if (hue >= 360) {
      hue = hue % 360;
   }

   int &sat = Config_getInt("HTML_COLORSTYLE_SAT");
   if (sat < 0) {
      sat = 0;
   } else if (sat > 255) {
      sat = 255;
   }
   int &gamma = Config_getInt("HTML_COLORSTYLE_GAMMA");
   if (gamma < 40) {
      gamma = 40;
   } else if (gamma > 240) {
      gamma = 240;
   }

   QByteArray mathJaxFormat = Config_getEnum("MATHJAX_FORMAT");
   if (!mathJaxFormat.isEmpty() && mathJaxFormat != "HTML-CSS" &&
         mathJaxFormat != "NativeMML" && mathJaxFormat != "SVG") {
      config_err("Error: Unsupported value for MATHJAX_FORMAT: Should be one of HTML-CSS, NativeMML, or SVG\n");
      Config_getEnum("MATHJAX_FORMAT") = "HTML-CSS";
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
   

   checkFileName("GENERATE_TAGFILE");
}


*/