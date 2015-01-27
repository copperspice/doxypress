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

#include <QTextCodec>
#include <QTextStream>

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cite.h>
#include <classlist.h>
#include <cmdmapper.h>
#include <code.h>
#include <config.h>
#include <config_json.h>
#include <doxy_setup.h>
#include <doxy_build_info.h>
#include <entry.h>
#include <filename.h>
#include <filestorage.h>
#include <formula.h>
#include <groupdef.h>
#include <htmlgen.h>
#include <index.h>
#include <language.h>
#include <latexgen.h>
#include <layout.h>
#include <markdown.h>
#include <membername.h>
#include <namespacedef.h>
#include <outputlist.h>
#include <parser_file.h>
#include <parser_make.h>
#include <parser_py.h>
#include <pagedef.h>
#include <perlmodgen.h>
#include <portable.h>
#include <pre.h>
#include <reflist.h>
#include <rtfgen.h>
#include <scanner.h>
#include <util.h>

// globals
#include <doxy_globals.h>

namespace Doxy_Setup {
   QString getValue(QStringList::iterator &iter, QStringList::iterator end);
   void generateConfigFile(const QString &configFile);
   bool openOutputFile(const QString &outFile, QFile &f);   
   void usage();
}

using namespace Doxy_Setup;

enum Options {
     INVALID, 
     BLANK_CFG, 
     BLANK_LAYOUT,
     BLANK_STYLE,
     DEBUG_DUMP, 
     DEBUG_SYMBOLS,
     HELP, 
     OUTPUT_WIZARD,
     RTF_EXTENSIONS,            
     DOXY_VERSION,      
};

void initDoxygen()
{
   const char *lang = portable_getenv("LC_ALL");

   if (lang) {
      portable_setenv("LANG", lang);
   }

   setlocale(LC_ALL, "");
   setlocale(LC_CTYPE, "C");       // to get isspace(0xA0)==0, needed for UTF-8
   setlocale(LC_NUMERIC, "C");

   QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

   Doxygen::runningTime.start();
   initPreprocessor();

   Doxygen::parserManager = new ParserManager;
   Doxygen::parserManager->registerDefaultParser(new FileParser);
   Doxygen::parserManager->registerParser("c", new CLanguageScanner);

   Doxygen::parserManager->registerParser("python",         new PythonLanguageScanner);

// BROOM  (out for now)
//   Doxygen::parserManager->registerParser("fortran",      new FortranLanguageScanner);
//   Doxygen::parserManager->registerParser("fortranfree",  new FortranLanguageScannerFree);
//   Doxygen::parserManager->registerParser("fortranfixed", new FortranLanguageScannerFixed);

// BROOM  (out for now)
//   Doxygen::parserManager->registerParser("dbusxml",      new DBusXMLScanner);

// BROOM  (out for now)
//   Doxygen::parserManager->registerParser("tcl",          new TclLanguageScanner);

   Doxygen::parserManager->registerParser("md",             new MarkdownFileParser);
   Doxygen::parserManager->registerParser("make",           new MakeFileParser);

   // register any additional parsers here
   initDefaultExtensionMapping();

   initClassMemberIndices();
   initNamespaceMemberIndices();
   initFileMemberIndices();
 
   Doxygen::inputNameList     = new SortedList<FileName *>;

   Doxygen::memberNameSDict   = new MemberNameSDict();
   Doxygen::functionNameSDict = new MemberNameSDict();
   Doxygen::groupSDict        = new GroupSDict();

   Doxygen::globalScope       = QMakeShared<NamespaceDef>("<globalScope>", 1, 1, "<globalScope>");

   Doxygen::namespaceSDict    = new NamespaceSDict();
   Doxygen::classSDict        = new ClassSDict();
   Doxygen::hiddenClasses     = new ClassSDict();
  
   Doxygen::pageSDict         = new PageSDict();          // all doc pages
   Doxygen::exampleSDict      = new PageSDict();          // all examples

   Doxygen::inputNameDict     = new FileNameDict();
   Doxygen::includeNameDict   = new FileNameDict();
   Doxygen::exampleNameDict   = new FileNameDict();
   Doxygen::imageNameDict     = new FileNameDict();
   Doxygen::dotFileNameDict   = new FileNameDict();
   Doxygen::mscFileNameDict   = new FileNameDict();
   Doxygen::diaFileNameDict   = new FileNameDict();
   Doxygen::citeDict          = new CiteDict();
   Doxygen::genericsDict      = new GenericsSDict;
   Doxygen::indexList         = new IndexList;
   Doxygen::formulaList       = new FormulaList;
   Doxygen::formulaDict       = new FormulaDict();
   Doxygen::formulaNameDict   = new FormulaDict();
   Doxygen::sectionDict       = new SectionDict();

   // Initialize global constants
   Doxy_Globals::g_compoundKeywordDict.insert("template class", (void *)8);
   Doxy_Globals::g_compoundKeywordDict.insert("template struct", (void *)8);
   Doxy_Globals::g_compoundKeywordDict.insert("class",  (void *)8);
   Doxy_Globals::g_compoundKeywordDict.insert("struct", (void *)8);
   Doxy_Globals::g_compoundKeywordDict.insert("union",  (void *)8);
   Doxy_Globals::g_compoundKeywordDict.insert("interface", (void *)8);
   Doxy_Globals::g_compoundKeywordDict.insert("exception", (void *)8);
}

void cleanUpDoxygen()
{
   delete Doxygen::sectionDict;
   delete Doxygen::formulaNameDict;
   delete Doxygen::formulaDict;
   delete Doxygen::formulaList;
   delete Doxygen::indexList;
   delete Doxygen::genericsDict;
   delete Doxygen::inputNameDict;
   delete Doxygen::includeNameDict;
   delete Doxygen::exampleNameDict;
   delete Doxygen::imageNameDict;
   delete Doxygen::dotFileNameDict;
   delete Doxygen::mscFileNameDict;
   delete Doxygen::diaFileNameDict;  
   delete Doxygen::pageSDict;
   delete Doxygen::exampleSDict;  
   delete Doxygen::xrefLists;
   delete Doxygen::parserManager;
   
   cleanUpPreprocessor();

   delete theTranslator;
   delete Doxy_Globals::g_outputList;
  
   Mappers::freeMappers();
   codeFreeScanner();
   
   delete Doxygen::inputNameList;
   delete Doxygen::memberNameSDict;
   delete Doxygen::functionNameSDict;
   delete Doxygen::groupSDict;
   delete Doxygen::classSDict;
   delete Doxygen::hiddenClasses;
   delete Doxygen::namespaceSDict;   
}

// **
struct CommandLine parseCommandLine(QStringList argList)
{   
   CommandLine cmdArgs;

   QMap<QString, Options> argMap;
   argMap.insert( "--b", OUTPUT_WIZARD   );
   argMap.insert( "--d", DEBUG_DUMP      );   
   argMap.insert( "--g", BLANK_CFG       );
   argMap.insert( "--h", HELP            );
   argMap.insert( "--l", BLANK_LAYOUT    );
   argMap.insert( "--m", DEBUG_SYMBOLS   );
   argMap.insert( "--w", BLANK_STYLE     );
   argMap.insert( "--v", DOXY_VERSION    );

   QStringList dashList;

   for (auto item : argList)   {      
      if (item.startsWith("--")) {
         dashList.append(item);
      }
   }
   
   auto iter = argList.begin();
 
   for (auto item : dashList)   {    

      Options value = INVALID;
      
      if (argMap.contains(item)) {
         value = argMap.value(item);

         ++iter;
      }
                
      switch (value) {

         case INVALID:
            msg("\n");
            err("Option %s is invalid\n", qPrintable(item)); 

            cleanUpDoxygen();
            exit(0);      
            
         case BLANK_CFG:
            cmdArgs.genConfig  = true;
            cmdArgs.configName = getValue(iter, argList.end());
           
            if (cmdArgs.configName.isEmpty()) {
               cmdArgs.configName = "CS_Doxyfile.json";
            }

            cmdArgs.generateDoxy = false;   
            break;

         case BLANK_LAYOUT:
            cmdArgs.genLayout = true;
            cmdArgs.layoutName = getValue(iter, argList.end());
           
            if (cmdArgs.layoutName.isEmpty()) {
               cmdArgs.layoutName = "DoxygenLayout.xml";
            }

            cmdArgs.generateDoxy = false; 
            break;

         case BLANK_STYLE:
            cmdArgs.formatName = getValue(iter, argList.end());

            if (cmdArgs.formatName.isEmpty()) {
               err("Option \"-w\" is missing format specifier rtf, html or latex\n");
               cleanUpDoxygen();
               exit(1);
            }

            cmdArgs.formatName = cmdArgs.formatName.toLower();

            if (cmdArgs.formatName == "rtf-ext") {
               cmdArgs.rtfExt = getValue(iter, argList.end());

               if (cmdArgs.rtfExt.isEmpty()) {
                  cmdArgs.rtfExt = "doxy_style.rtf";
               }

               QFile f;
               if (openOutputFile(cmdArgs.rtfExt, f)) {
                   RTFGenerator::writeExtensionsFile(f);
               }
               f.close();                

            } else if (cmdArgs.formatName == "rtf-style") {
               cmdArgs.rtfStyle = getValue(iter, argList.end());

               if (cmdArgs.rtfStyle.isEmpty()) {
                  cmdArgs.rtfStyle = "doxy_style.rtf";
               }

               QFile f;
               if (openOutputFile(cmdArgs.rtfStyle, f)) {
                  RTFGenerator::writeStyleSheetFile(f);
               }
               f.close();


            } else  if (cmdArgs.formatName == "html-head") {
               cmdArgs.htmlHead = getValue(iter, argList.end());

               if (cmdArgs.htmlHead.isEmpty()) {
                  cmdArgs.htmlHead = "doxy_header.html";
               }

               QFile f;
               if (openOutputFile(cmdArgs.htmlHead, f)) {
                  HtmlGenerator::writeHeaderFile(f);
               }
               f.close();


            } else  if (cmdArgs.formatName == "html-foot") {
               cmdArgs.htmlFoot = getValue(iter, argList.end());

               if (cmdArgs.htmlFoot.isEmpty()) {
                  cmdArgs.htmlFoot = "doxy_footer.html";
               }

               QFile f;
               if (openOutputFile(cmdArgs.htmlFoot, f)) {
                  HtmlGenerator::writeFooterFile(f);
               }
               f.close();


            } else  if (cmdArgs.formatName == "html-style") {
              cmdArgs.htmlStyle = getValue(iter, argList.end());

              if (cmdArgs.htmlStyle.isEmpty()) {
                 cmdArgs.htmlStyle = "doxy_style.css";
              }

               QFile f;
               if (openOutputFile(cmdArgs.htmlStyle, f)) {
                  HtmlGenerator::writeStyleSheetFile(f);
               }
               f.close();


            } else if (cmdArgs.formatName == "latex-head") {
              cmdArgs.latexHead = getValue(iter, argList.end());

               if (cmdArgs.latexHead.isEmpty()) {
                  cmdArgs.latexHead = "doxy_header.latex";
               }
/* BROOM  latex
               QFile f;
               if (openOutputFile(cmdArgs.latexHead, f)) {
                 LatexGenerator::writeHeaderFile(f);
               }
               f.close();
*/

            } else if (cmdArgs.formatName == "latex-foot") {
              cmdArgs.latexFoot = getValue(iter, argList.end());
   
               if (cmdArgs.latexFoot.isEmpty()) {
                  cmdArgs.latexFoot = "doxy_footer.latex";
               }

/* BROOM  latex
               QFile f;
               if (openOutputFile(cmdArgs.latexFoot, f)) {
                  LatexGenerator::writeFooterFile(f);
               }
               f.close();
*/ 

            } else if (cmdArgs.formatName == "latex-style") {
              cmdArgs.latexStyle = getValue(iter, argList.end());

               if (cmdArgs.htmlStyle.isEmpty()) {
                  cmdArgs.htmlStyle= "doxy_style.latex";
               }

/* BROOM latex
              QFile f;
              if (openOutputFile(cmdArgs.latexStyle, f)) {
                 LatexGenerator::writeStyleSheetFile(f);
               }
               f.close();
*/

            } else  {
               err("Option \"-w %s\" is invalid\n", qPrintable(cmdArgs.formatName));
               cleanUpDoxygen();
               exit(1);

            }

            cmdArgs.generateDoxy = false; 
            break;

         case DEBUG_DUMP:
            cmdArgs.debugLabel = getValue(iter, argList.end());

            if (cmdArgs.debugLabel.isEmpty() ) {
               err("Option \"-d\" is missing a debug specifier.\n");              
               cleanUpDoxygen();
               exit(1);
            }
           
            if (! Debug::setFlag(cmdArgs.debugLabel)) {
               err("Option \"-d\" has an unknown debug specifier: \"%s\".\n", qPrintable(cmdArgs.debugLabel));

               cleanUpDoxygen();
               exit(1);
            }

            break;

         case DEBUG_SYMBOLS:
            Doxy_Globals::g_dumpSymbolMap = true;
            break;
          
         case DOXY_VERSION:
            msg("\nCS Doxygen Version: %s\n", versionString);

            cleanUpDoxygen();
            exit(0);
                  
         case OUTPUT_WIZARD:
            setvbuf(stdout, NULL, _IONBF, 0);

            Doxygen::outputToWizard = true;
            break;

         case HELP:       
            usage();

            cleanUpDoxygen();
            exit(0);                    
      }      
   }

   // is there anything left in the arguemnent list
   cmdArgs.configName = getValue(iter, argList.end());
 
   return cmdArgs;
}

void readConfiguration(struct CommandLine cmdArgs)
{      
   // Parse or generate the config file  
   Config::instance()->init();

   if (cmdArgs.genConfig) {
      generateConfigFile(cmdArgs.configName);     
   } 

   if (cmdArgs.genLayout) {
      writeDefaultLayoutFile(cmdArgs.layoutName);      
   }

   if (! cmdArgs.generateDoxy) {      
      cleanUpDoxygen();
      exit(0);      
   }

   if (cmdArgs.configName.isEmpty()) {
   
      for (auto item : QDir::current().entryList()) {          

         if (item.compare("CS_Doxyfile.json", Qt::CaseInsensitive) == 0) {
            cmdArgs.configName = item;
            break;
         } 
      }
     
      if (cmdArgs.configName.isEmpty()) {
         err("No configuration file name was specified and 'CS_Doxyfile.json' was not found"); 

         cleanUpDoxygen();
         exit(1);
      }        
   }  

   QFileInfo fi(cmdArgs.configName);

   if (! fi.exists()) {  
      err("Configuration file %s was not found\n", qPrintable(cmdArgs.configName));

      cleanUpDoxygen();
      exit(1);
   }
  

/*   broom - json set up

   if (! Config_Json::parseConfig(cmdArgs.configName) ) {
      err("Unable to open or read CS Doxygen configuration file %s\n", qPrintable(cmdArgs.configName));

      cleanUpDoxygen();
      exit(1);
   }
*/

   
   if (! Config::instance()->parse( qPrintable(cmdArgs.configName) )) {
      err("Unable to open or read configuration file %s\n", qPrintable(cmdArgs.configName));

      cleanUpDoxygen();
      exit(1);
   }
  
   // Perlmod wants to know the path to the config file
   QFileInfo configFileInfo(cmdArgs.configName);

//BROOM   setPerlModDoxyfile(qPrintable(cmdArgs.configName.absoluteFilePath()));

}

// check and resolve config options
void checkConfiguration()
{
   printf("\n**  Verifying the configuration file\n");   

   Config::instance()->substituteEnvironmentVars();
   Config::instance()->convertStrToVal();
   Config::instance()->check();
   initWarningFormat();

}

// adjust globals which depend on configuration settings 
void adjustConfiguration()
{
   printf("**  Adjust the Configuration file\n");   

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
}

QString Doxy_Setup::getValue(QStringList::iterator &iter, QStringList::iterator end)
{
   QString retval;

   if (iter == end) {
      // nothing
      
   } else {     

      if (! iter->startsWith("--")) {
         retval = *iter;
         ++iter;
      }
   }

   return retval;
}

/*! Generate a new version of the configuration file 
 */
void Doxy_Setup::generateConfigFile(const QString &configFile)
{
   QFile f;

   bool fileOpened = openOutputFile(configFile, f);
 
   if (fileOpened) {
      QTextStream t(&f);

      // BROOM  - json set up  
      // Config_Json::->writeNewConfig();      
    
      msg("\n\nConfiguration file `%s' created\n\n", qPrintable(configFile));        
               

   } else {
      err("Unable to open file %s for writing\n", qPrintable(configFile));  
      exit(1);
   }
}

bool Doxy_Setup::openOutputFile(const QString &outFile, QFile &f)
{
   bool fileOpened = false;
   bool writeToStdout = (outFile[0] == '-' && outFile[1] == '\0');

   if (writeToStdout) { 
      // write to stdout
      fileOpened = f.open(stdout, QIODevice::WriteOnly);

   } else { 
      // write to file
      QFileInfo fi(outFile);

      if (fi.exists()) { 
         // create a backup

         QDir dir = fi.dir();
         QFileInfo backup(fi.fileName() + ".bak");

         if (backup.exists()) { 
            // remove existing backup
            dir.remove(backup.fileName());
         }

         dir.rename(fi.fileName(), fi.fileName() + ".bak");
      }

      f.setFileName(outFile);

      fileOpened = f.open(QIODevice::WriteOnly | QIODevice::Text);
   }

   return fileOpened;
}

void Doxy_Setup::usage()
{
   msg("\n");
   msg("CS Doxygen: Version %s\n", versionString);

   msg("\n");
   msg("Generate documentation using an existing configuration file:\n");   
   msg("   [config file name]\n");

   msg("\n"); 
   msg("Generate a blank configuration file:\n");
   msg("   --g [config file name]   Default file name is `Doxyfile.json'\n");  
   
   msg("\n");
   msg("Generate a layout file to configure the documentation:\n");
   msg("   --l [layout file name]   Default file name is `Layout.xml'\n");
     
   msg("\n");
   msg("Generate a style sheet file for RTF, HTML or Latex:\n");
   msg("   RTF:    --w  rtf-ext   [extensions file name]\n");
   msg("   RTF:    --w  rtf-style [stylesheet file name]\n");

   msg("\n");
   msg("   HTML:   --w  html-head  [header file name] \n");
   msg("   HTML:   --w  html-foot  [footer file name] \n");
   msg("   HTML:   --w  html-style [styleSheet file name] \n");
  
   msg("\n");
   msg("   LaTeX:  --w  latex-head  [header file name] \n");
   msg("   LaTeX:  --w  latex-foot  [footer file name] \n");
   msg("   LaTeX:  --w  latex-style [styleSheet file name] \n");
   
   msg("\n");
   msg("Other Options:\n");
   msg("   --h  display usage\n");
   msg("   --v  display version\n");
    
   msg("\n");
   msg("   --b  output to CS DoxyWizard\n"); 
   msg("   --m  dump symbol map\n");  
   msg("   --d  <level> enable one or more debug levels\n");
  
   Debug::printFlags();   
}
