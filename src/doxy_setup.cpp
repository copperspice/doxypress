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
#include <doxygen.h>
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

namespace {
   const char *getArg(int argc, char **argv, int &optind);
   void generateConfigFile(const char *configFile, bool shortList, bool updateOnly = false);
   bool openOutputFile(const char *outFile, QFile &f);
   void usageDev();
   void usage(const char *name);
}

void initDoxygen()
{
   const char *lang = portable_getenv("LC_ALL");

   if (lang) {
      portable_setenv("LANG", lang);
   }

   setlocale(LC_ALL, "");
   setlocale(LC_CTYPE, "C");       // to get isspace(0xA0)==0, needed for UTF-8
   setlocale(LC_NUMERIC, "C");

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

   Mappers::freeMappers();
   codeFreeScanner();

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

   delete Doxygen::inputNameList;
   delete Doxygen::memberNameSDict;
   delete Doxygen::functionNameSDict;
   delete Doxygen::groupSDict;
   delete Doxygen::classSDict;
   delete Doxygen::hiddenClasses;
   delete Doxygen::namespaceSDict;   
}


// **
void readConfiguration(int argc, char **argv)
{   
   int optind = 1;

   const char *configName = 0;
   const char *layoutName = 0;
   const char *debugLabel;
   const char *formatName;

   bool genConfig = false;
   bool shortList = false;  
   bool genLayout = false;

   int retVal;

   while (optind < argc && argv[optind][0] == '-' && (isalpha(argv[optind][1]) || argv[optind][1] == '?' || argv[optind][1] == '-')) {

      switch (argv[optind][1]) {
         case 'g':
            genConfig = true;
            configName = getArg(argc, argv, optind);

            if (optind + 1 < argc && qstrcmp(argv[optind + 1], "-") == 0) {
               configName = "-";
               optind++;
            }

            if (!configName) {
               configName = "Doxyfile";
            }
            break;

         case 'l':
            genLayout = true;
            layoutName = getArg(argc, argv, optind);

            if (!layoutName) {
               layoutName = "DoxygenLayout.xml";
            }
            break;

         case 'd':
            debugLabel = getArg(argc, argv, optind);

            if (! debugLabel) {
               err("option \"-d\" is missing debug specifier.\n");
               usageDev();
               cleanUpDoxygen();
               exit(1);
            }

            retVal = Debug::setFlag(debugLabel);
            if (! retVal) {
               err("option \"-d\" has unknown debug specifier: \"%s\".\n", debugLabel);
               cleanUpDoxygen();
               exit(1);
            }
            break;

         case 's':
            shortList = true;
            break;
       
         case 'e':
            formatName = getArg(argc, argv, optind);

            if (!formatName) {
               err("option \"-e\" is missing format specifier rtf.\n");
               cleanUpDoxygen();
               exit(1);
            }

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

            err("option \"-e\" has invalid format specifier.\n");
            cleanUpDoxygen();
            exit(1);
            break;

         case 'w':
            formatName = getArg(argc, argv, optind);
            if (!formatName) {
               err("option \"-w\" is missing format specifier rtf, html or latex\n");
               cleanUpDoxygen();
               exit(1);
            }
            if (qstricmp(formatName, "rtf") == 0) {
               if (optind + 1 >= argc) {
                  err("option \"-w rtf\" is missing a style sheet file name\n");
                  cleanUpDoxygen();
                  exit(1);
               }
               QFile f;
               if (openOutputFile(argv[optind + 1], f)) {
                  RTFGenerator::writeStyleSheetFile(f);
               }
               cleanUpDoxygen();
               exit(1);

            } else if (qstricmp(formatName, "html") == 0) {

               if (optind + 4 < argc || QFileInfo("Doxyfile").exists()) {

                  QByteArray df;

                  if (optind + 4 < argc) {
                     df = argv[optind + 4]; 

                  } else {
                     df = "Doxyfile";
                  }

                  if (!Config::instance()->parse(df)) {
                     err("error opening or reading configuration file %s!\n", argv[optind + 4]);
                     cleanUpDoxygen();
                     exit(1);
                  }

                  Config::instance()->substituteEnvironmentVars();
                  Config::instance()->convertStrToVal();

                  // avoid bootstrapping issues when the config file already
                  // refers to the files that we are supposed to parse.

                  Config_getString("HTML_HEADER") = "";
                  Config_getString("HTML_FOOTER") = "";
                  Config::instance()->check();

               } else {
                  Config::instance()->init();
               }

               if (optind + 3 >= argc) {
                  err("option \"-w html\" does not have enough arguments\n");
                  cleanUpDoxygen();
                  exit(1);
               }

               QByteArray outputLanguage = Config_getEnum("OUTPUT_LANGUAGE");
               if (!setTranslator(outputLanguage)) {
                  warn_uncond("Output language %s not supported! Using English instead.\n", outputLanguage.data());
               }

               QFile f;
               if (openOutputFile(argv[optind + 1], f)) {
                  HtmlGenerator::writeHeaderFile(f, argv[optind + 3]);
               }
               f.close();
               if (openOutputFile(argv[optind + 2], f)) {
                  HtmlGenerator::writeFooterFile(f);
               }
               f.close();
               if (openOutputFile(argv[optind + 3], f)) {
                  HtmlGenerator::writeStyleSheetFile(f);
               }
               cleanUpDoxygen();
               exit(0);
            } else if (qstricmp(formatName, "latex") == 0) {
               if (optind + 4 < argc) { // use config file to get settings
                  if (!Config::instance()->parse(argv[optind + 4])) {
                     err("error opening or reading configuration file %s!\n", argv[optind + 4]);
                     exit(1);
                  }
                  Config::instance()->substituteEnvironmentVars();
                  Config::instance()->convertStrToVal();
                  Config_getString("LATEX_HEADER") = "";
                  Config::instance()->check();
               } else { // use default config
                  Config::instance()->init();
               }
               if (optind + 3 >= argc) {
                  err("option \"-w latex\" does not have enough arguments\n");
                  cleanUpDoxygen();
                  exit(1);
               }

               QByteArray outputLanguage = Config_getEnum("OUTPUT_LANGUAGE");
               if (!setTranslator(outputLanguage)) {
                  warn_uncond("Output language %s not supported! Using English instead.\n", outputLanguage.data());
               }

               QFile f;
               if (openOutputFile(argv[optind + 1], f)) {
                  LatexGenerator::writeHeaderFile(f);
               }
               f.close();
               if (openOutputFile(argv[optind + 2], f)) {
                  LatexGenerator::writeFooterFile(f);
               }
               f.close();
               if (openOutputFile(argv[optind + 3], f)) {
                  LatexGenerator::writeStyleSheetFile(f);
               }
               cleanUpDoxygen();
               exit(0);
            } else {
               err("Illegal format specifier \"%s\": should be one of rtf, html or latex\n", formatName);
               cleanUpDoxygen();
               exit(1);
            }
            break;

         case 'm':
            Doxy_Globals::g_dumpSymbolMap = true;
            break;

         case 'v':
            msg("%s\n", versionString);
            cleanUpDoxygen();
            exit(0);
            break;

         case '-':
            if (qstrcmp(&argv[optind][2], "help") == 0) {
               usage(argv[0]);
               exit(0);
            } else if (qstrcmp(&argv[optind][2], "version") == 0) {
               msg("%s\n", versionString);
               cleanUpDoxygen();
               exit(0);
            } else {
               err("Unknown option \"-%s\"\n", &argv[optind][1]);
               usage(argv[0]);
               exit(1);
            }
            break;

         case 'b':
            setvbuf(stdout, NULL, _IONBF, 0);
            Doxygen::outputToWizard = true;
            break;

         case 'T':
            msg("Warning: this option activates output generation via Django like template files. "
                "This option is scheduled for doxygen 2.0, is currently incomplete and highly experimental! "
                "Only use if you are a doxygen developer\n");

            Doxy_Globals::g_useOutputTemplate = true;
            break;

         case 'h':
         case '?':
            usage(argv[0]);
            exit(0);
            break;

         default:
            err("Unknown option \"-%c\"\n", argv[optind][1]);
            usage(argv[0]);
            exit(1);
      }
      optind++;
   }
      
   // Parse or generate the config file  
   Config::instance()->init();

   if (genConfig) {
      generateConfigFile(configName, shortList);
      cleanUpDoxygen();
      exit(0);
   } 

   if (genLayout) {
      writeDefaultLayoutFile(layoutName);
      cleanUpDoxygen();
      exit(0);
   }

   QFileInfo configFileInfo1("Doxyfile"), configFileInfo2("doxyfile");

   if (optind >= argc) {
      if (configFileInfo1.exists()) {
         configName = "Doxyfile";

      } else if (configFileInfo2.exists()) {
         configName = "doxyfile";

      } else {
         err("Doxyfile not found and no input file specified!\n");
         usage(argv[0]);
         exit(1);
      }

   } else {
      QFileInfo fi(argv[optind]);

      if (fi.exists() || qstrcmp(argv[optind], "-") == 0) {
         configName = argv[optind];

      } else {
         err("configuration file %s not found!\n", argv[optind]);
         usage(argv[0]);
         exit(1);
      }
   }


   printf("\n  Parse the Json file here ");

   // add test for failures
   Config::instance()->parseConfig();

/*

   //BROOM - out for now
   if (! Config::instance()->parse(configName)) {
      err("could not open or read configuration file %s!\n", configName);
      cleanUpDoxygen();
      exit(1);
   }
*/
  
   // Perlmod wants to know the path to the config file
   QFileInfo configFileInfo(configName);
   setPerlModDoxyfile(qPrintable(configFileInfo.absoluteFilePath()));

}

// check and resolve config options
void checkConfiguration()
{

   printf("\n  Verify the cfg file is ok");   


//BROOM - out for now   Config::instance()->substituteEnvironmentVars();
//BROOM - out for now   Config::instance()->convertStrToVal();
//BROOM - out for now   Config::instance()->check();

//BROOM - out for now   initWarningFormat();
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

static const char *getArg(int argc, char **argv, int &optind)
{
   char *s = 0;
   if (qstrlen(&argv[optind][2]) > 0) {
      s = &argv[optind][2];
   } else if (optind + 1 < argc && argv[optind + 1][0] != '-') {
      s = argv[++optind];
   }
   return s;
}

/*! Generate a new version of the configuration file 
 */
static void generateConfigFile(const char *configFile)
{
   QFile f;

   bool fileOpened    = openOutputFile(configFile, f);
   bool writeToStdout = (configFile[0] == '-' && configFile[1] == '\0');

   if (fileOpened) {

      FTextStream t(&f);
      Config::instance()->writeNewCfg();      

      if (writeToStdout) {     
        // do nothing else
         
      } else {    
         msg("\n\nConfiguration file `%s' created.\n\n", configFile);
         msg("Now edit the configuration file and enter\n\n");

         if (qstrcmp(configFile, "Doxyfile") || qstrcmp(configFile, "doxyfile")) {
            msg("  doxygen %s\n\n", configFile);

         } else {
            msg("  doxygen\n\n");

         }

         msg("to generate the documentation for your project\n\n");        
      }

   } else {
      err("Can not open file %s for writing\n", configFile);
      exit(1);
   }
}

static bool openOutputFile(const char *outFile, QFile &f)
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

// print developer options
static void usageDev()
{
   msg("Developer parameters:\n");
   msg("  -m          dump symbol map\n");
   msg("  -b          output to wizard\n");
   msg("  -T          activates output generation via Django like template\n");
   msg("  -d <level>  enable a debug level, such as (multiple invocations of -d are possible):\n");

   Debug::printFlags();
}

// print the usage
static void usage(const char *name)
{
   msg("Doxygen version %s\n\n", versionString);
   msg("You can use doxygen in a number of ways:\n\n");
   msg("1) Use doxygen to generate a template configuration file:\n");
   msg("    %s [-s] -g [configName]\n\n", name);
   msg("    If - is used for configName doxygen will write to standard output.\n\n"); 
   msg("    If configName is omitted `Doxyfile' will be used as a default.\n\n");

   msg("2) Use doxygen to generate documentation using an existing ");
   msg("configuration file:\n");
   msg("    %s [configName]\n\n", name);
   msg("    If - is used for configName doxygen will read from standard input.\n\n");

   msg("3) Use doxygen to generate a template file controlling the layout of the\n");
   msg("   generated documentation:\n");
   msg("    %s -l layoutFileName.xml\n\n", name);

   msg("4) Use doxygen to generate a template style sheet file for RTF, HTML or Latex.\n");
   msg("    RTF:        %s -w rtf styleSheetFile\n", name);
   msg("    HTML:       %s -w html headerFile footerFile styleSheetFile [configFile]\n", name);
   msg("    LaTeX:      %s -w latex headerFile footerFile styleSheetFile [configFile]\n\n", name);

   msg("5) Use doxygen to generate a rtf extensions file\n");
   msg("    RTF:   %s -e rtf extensionsFile\n\n", name);

   msg("6) -v print version string\n");
}
