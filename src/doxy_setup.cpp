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
#include <fileparser.h>
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

// BROOM  (out for now)
//   Doxygen::parserManager->registerParser("python", new PythonLanguageScanner);

// BROOM  (out for now)
//   Doxygen::parserManager->registerParser("fortran",      new FortranLanguageScanner);
//   Doxygen::parserManager->registerParser("fortranfree",  new FortranLanguageScannerFree);
//   Doxygen::parserManager->registerParser("fortranfixed", new FortranLanguageScannerFixed);

// BROOM  (out for now)
//   Doxygen::parserManager->registerParser("dbusxml",      new DBusXMLScanner);

// BROOM  (out for now)
//   Doxygen::parserManager->registerParser("tcl",          new TclLanguageScanner);

   Doxygen::parserManager->registerParser("md",             new MarkdownFileParser);

   // register any additional parsers here
   initDefaultExtensionMapping();

   initClassMemberIndices();
   initNamespaceMemberIndices();
   initFileMemberIndices();
 

#ifdef USE_LIBCLANG
   Doxygen::clangUsrMap       = new QHash<QString, Definition *>();
#endif

   Doxygen::symbolMap         = new QHash<QString, DefinitionIntf *>();
   Doxygen::inputNameList     = new SortedList<FileName *>;

   Doxygen::memberNameSDict   = new MemberNameSDict();
   Doxygen::functionNameSDict = new MemberNameSDict();
   Doxygen::groupSDict        = new GroupSDict();

   Doxygen::globalScope       = new NamespaceDef("<globalScope>", 1, 1, "<globalScope>");

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

   // Initialize some global constants
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
   delete Doxygen::mainPage;
   delete Doxygen::pageSDict;
   delete Doxygen::exampleSDict;
   delete Doxygen::globalScope;
   delete Doxygen::xrefLists;
   delete Doxygen::parserManager;
   
   cleanUpPreprocessor();

   delete theTranslator;
   delete Doxy_Globals::g_outputList;

   printf("\n\n Clean Up - ONE");

   Mappers::freeMappers();
//BROOM    codeFreeScanner();

   if (Doxygen::symbolMap) {
      // iterate through Doxygen::symbolMap and delete all
      // DefinitionList objects, since they have no owner
     
      for (auto dli = Doxygen::symbolMap->begin(); dli != Doxygen::symbolMap->end(); ) {

         if ((*dli)->definitionType() == DefinitionIntf::TypeSymbolList) {
            DefinitionIntf *tmp = Doxygen::symbolMap->take(dli.key());
            delete (DefinitionList *)tmp;

         } else {
            ++dli;
         }
      }
   }

   printf("\n\n Clean Up - TWO");

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
   argMap.insert( "--e", RTF_EXTENSIONS  );
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
            err("Option %s is invalid\n", qPrintable(item)); 
            exit(0);
            
         case BLANK_CFG:
            cmdArgs.genConfig  = true;
            cmdArgs.configName = getValue(iter, argList.end());
           
            if (cmdArgs.configName.isEmpty()) {
               cmdArgs.configName = "Doxyfile.json";
            }

            break;

         case BLANK_LAYOUT:
            cmdArgs. genLayout  = true;
            cmdArgs.layoutName = getValue(iter, argList.end());
           
            if (cmdArgs.layoutName.isEmpty()) {
               cmdArgs.layoutName = "DoxygenLayout.xml";
            }

            break;

         case BLANK_STYLE:
            cmdArgs.formatName = getValue(iter, argList.end());

            if (cmdArgs.formatName.isEmpty()) {
               err("Option \"-w\" is missing format specifier rtf, html or latex\n");
               cleanUpDoxygen();
               exit(1);
            }

            cmdArgs.formatName = cmdArgs.formatName.toLower();

            if (cmdArgs.formatName == "rtf-stle") {
               cmdArgs.rtfStyle = getValue(iter, argList.end());

/*  broom

               QFile f;
               if (openOutputFile(cmdArgs.rtfStyle, f)) {
                  RTFGenerator::writeStyleSheetFile(f);
               }
               f.close();
*/

            } else  if (cmdArgs.formatName == "html-header") {
               cmdArgs.htmlHead = getValue(iter, argList.end());

               if (cmdArgs.htmlHead.isEmpty()) {
                  cmdArgs.htmlHead = "doxy_header.html";
               }

               QFile f;
               if (openOutputFile(cmdArgs.htmlHead, f)) {
                  HtmlGenerator::writeHeaderFile(f, qPrintable(cmdArgs.htmlHead));
               }
               f.close();


            } else  if (cmdArgs.formatName == "html-footer") {
               cmdArgs.htmlFoot = getValue(iter, argList.end());

               if (cmdArgs.htmlFoot.isEmpty()) {
                  cmdArgs.htmlFoot = "doxy_footerer.html";
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

/* BROOM
               QFile f;
               if (openOutputFile(cmdArgs.htmlStyle, f)) {
                  HtmlGenerator::writeStyleSheetFile(f);
               }
               f.close();
*/


            } else if (cmdArgs.formatName == "latex-head") {
              cmdArgs.latexHead = getValue(iter, argList.end());

/* BROOM 
               QFile f;
               if (openOutputFile(argv[optind + 1], f)) {
                 LatexGenerator::writeHeaderFile(f);
               }
               f.close();
*/


            } else if (cmdArgs.formatName == "latex-foot") {
              cmdArgs.latexFoot = getValue(iter, argList.end());

/* BROOM   
               QFile f;
               if (openOutputFile(argv[optind + 2], f)) {
                  LatexGenerator::writeFooterFile(f);
               }
               f.close();
*/ 

            } else if (cmdArgs.formatName == "latex-style") {
              cmdArgs.latexStyle = getValue(iter, argList.end());

/* BROOM
              QFile f;
              if (openOutputFile(argv[optind + 3], f)) {
                 LatexGenerator::writeStyleSheetFile(f);
               }
               f.close();
*/

            } else  {
               err("Option \"-w %s\" are invalid  \n", qPrintable(cmdArgs.formatName));
               cleanUpDoxygen();
               exit(1);

            }

            cleanUpDoxygen();
            exit(0);

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
       
         case RTF_EXTENSIONS:
            cmdArgs.formatName = getValue(iter, argList.end());

            if (cmdArgs.formatName.isEmpty()) {
               err("Option \"-e\" is missing an RTF format specifier\n");
               cleanUpDoxygen();
               exit(1);
            }

/*   BROOM
            if (qstricmp(formatName, "rtf") == 0) {
               if (optind + 1 >= argc) {
                  err("option \"-e rtf\" is missing an extensions file name\n");
                  cleanUpDoxygen();
                  exit(1);
               }

               QFile f;
               if (openOutputFile(argv[optind + 1], f)) {
                   RTFGenerator::writeExtensionsFile(f);
               }

               cleanUpDoxygen();
               exit(0);
            }
*/

            err("Option \"-e\" has invalid format specifier.\n");
            cleanUpDoxygen();
            exit(1);
          
         case DOXY_VERSION:
            msg("%s\n", versionString);

            cleanUpDoxygen();
            exit(0);
                  
         case OUTPUT_WIZARD:
            setvbuf(stdout, NULL, _IONBF, 0);

            Doxygen::outputToWizard = true;
            break;

         case HELP:       
            usage();
            exit(0);                    
      }      
   }

   return cmdArgs;
}

void readConfiguration(struct CommandLine cmdArgs)
{      
   // Parse or generate the config file  
   Config::instance()->init();

   if (cmdArgs.genConfig) {
      generateConfigFile(cmdArgs.configName);
      cleanUpDoxygen();
      exit(0);
   } 

   if (cmdArgs.genLayout) {
      writeDefaultLayoutFile(cmdArgs.layoutName);
      cleanUpDoxygen();
      exit(0);
   }


/*   BROOM - out for initial test

   QFileInfo configFileInfo1("Doxyfile"), configFileInfo2("doxyfile");

   if (optind >= argc) {
      if (configFileInfo1.exists()) {
         configName = "Doxyfile";

      } else if (configFileInfo2.exists()) {
         configName = "doxyfile";

      } else {
         err("CS Doxyfile not found and no input file specified!\n");
         usage();
         exit(1);
      }

   } else {
      QFileInfo fi(argv[optind]);

      if (fi.exists() || qstrcmp(argv[optind], "-") == 0) {
         configName = argv[optind];

      } else {
         err("Configuration file %s was not found\n", argv[optind]);
         usage();
         exit(1);
      }
   }


*/

   // printf("\n  Parse the Json file here ");
   // add test for failures
   // Config_Json::instance()->parseConfig();

   
   if (! Config::instance()->parse( qPrintable(cmdArgs.configName) )) {
      err("Unable to open or read configuration file %s\n", qPrintable(cmdArgs.configName));
      cleanUpDoxygen();
      exit(1);
   }

  
   // Perlmod wants to know the path to the config file
   QFileInfo configFileInfo(cmdArgs.configName);
//BROOM   setPerlModDoxyfile(qPrintable(configFileInfo.absoluteFilePath()));

}

// check and resolve config options
void checkConfiguration()
{
   printf("\n  Verify the cfg file is ok");   

   Config::instance()->substituteEnvironmentVars();
   Config::instance()->convertStrToVal();
   Config::instance()->check();
   initWarningFormat();

}


// adjust globals that depend on configuration settings 
void adjustConfiguration()
{

   printf("\n  Adjust the Cfg file");   


/*

   QByteArray outputLanguage = Config_getEnum("OUTPUT_LANGUAGE");

   if (!setTranslator(outputLanguage)) {
      warn_uncond("Output language %s not supported! Using English instead.\n",outputLanguage.data());
   }

   QStringList &includePath = Config_getList("INCLUDE_PATH");
   char *s = includePath.first();

   while (s) {
      QFileInfo fi(s);
      addSearchDir(fi.absoluteFilePath().toUtf8());
      s = includePath.next();
   }

   // Set the global html file extension. 
   Doxygen::htmlFileExtension = Config_getString("HTML_FILE_EXTENSION");

   Doxygen::parseSourcesNeeded = Config_getBool("CALL_GRAPH") ||  Config_getBool("CALLER_GRAPH") ||
                                 Config_getBool("REFERENCES_RELATION") || Config_getBool("REFERENCED_BY_RELATION");

   Doxygen::markdownSupport = Config_getBool("MARKDOWN_SUPPORT");


   // Add custom extension mappings 
   QStringList &extMaps = Config_getList("EXTENSION_MAPPING");
   char *mapping = extMaps.first();

   while (mapping) {
      QByteArray mapStr = mapping;
      int i;

      if ((i = mapStr.find('=')) != -1) {
         QByteArray ext = mapStr.left(i).trimmed().toLower();
         QByteArray language = mapStr.mid(i + 1).trimmed().toLower();

         if (!updateLanguageMapping(ext, language)) {
            err("Failed to map file extension '%s' to unsupported language '%s'.\n"
                "Check the EXTENSION_MAPPING setting in the config file.\n", ext.data(), language.data());

         } else {
            msg("Adding custom extension mapping: .%s will be treated as language %s\n", ext.data(), language.data());
         }
      }

      mapping = extMaps.next();
   }


   // add predefined macro name to a dictionary
   QStringList &expandAsDefinedList = Config_getList("EXPAND_AS_DEFINED");

   s = expandAsDefinedList.first();

   while (s) {
      if (Doxygen::expandAsDefinedDict[s] == 0) {
         Doxygen::expandAsDefinedDict.insert(s, (void *)666);
      }
      s = expandAsDefinedList.next();
   }

   // read aliases and store them in a dictionary
   readAliases();

   // store number of spaces in a tab into Doxygen::spaces
   int &tabSize = Config_getInt("TAB_SIZE");

   Doxygen::spaces.resize(tabSize + 1);
   int sp;

   for (sp = 0; sp < tabSize; sp++) {
      Doxygen::spaces.at(sp) = ' ';
   }

   Doxygen::spaces.at(tabSize) = '\0';

*/

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
      FTextStream t(&f);

      // BROOM   
      // Config_Json::instance()->writeNewCfg();      
    
      msg("\n\nConfiguration file `%s' created.\n\n", qPrintable(configFile));        
               

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
   msg("CS Doxygen: Version %s\n", versionString);

   msg("\n"); 
   msg("Generate a blank configuration file:\n");
   msg("   --g [config file name]\n\n");  
   msg("   Defalut file name is `Doxyfile.json'\n");

   msg("\n");
   msg("Generate a layout file to configure the documentation:\n");
   msg("   --l [layout file name]\n\n");
   msg("   Default file name is `Layout.xml'\n");

   msg("\n");
   msg("Generate documentation using an existing configuration file:\n");   
   msg("   [config file name]\n\n");
  
   msg("\n");
   msg("Generate a style sheet file for RTF, HTML or Latex:\n");
   msg("   RTF:     --w  rtf-style [stylesheet file name]\n");

   msg("\n");
   msg("   HTML:    --w html-head  [header file name] \n");
   msg("   HTML:    --w html-foot  [footer file name] \n");
   msg("   HTML:    --w html-style [styleSheet file name] \n");
  
   msg("\n");
   msg("   LaTeX:   --w  latex-head  [header file name] \n");
   msg("   LaTeX:   --w  latex-foot  [footer file name] \n");
   msg("   LaTeX:   --w  latex-style [styleSheet file name] \n");

   msg("\n");
   msg("Generate rtf extensions file\n");
   msg("   RTF:   --e  rtf extensionsFile\n\n");

   msg("\n");
   msg("   --h          display usage\n");
   msg("   --v          display version\n");
    
   msg("\n");
   msg("   --m          dump symbol map\n");
   msg("   --b          output to CS DoxyWizard\n"); 
   msg("   --d <level>  enable one or more debug levels\n");
  
   Debug::printFlags();   
}
