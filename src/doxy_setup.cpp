/************************************************************************
*
* Copyright (c) 2014-2023 Barbara Geller & Ansel Sermersheim
* Copyright (c) 1997-2014 Dimitri van Heesch
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

#include <QDateTime>
#include <QTextCodec>
#include <QTextStream>

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <doxy_setup.h>

#include <cite.h>
#include <cmdmapper.h>
#include <code_cstyle.h>
#include <config.h>
#include <doxy_globals.h>
#include <doxy_build_info.h>
#include <entry.h>
#include <formula.h>
#include <htmlgen.h>
#include <index.h>
#include <language.h>
#include <latexgen.h>
#include <layout.h>
#include <outputlist.h>
#include <parse_cstyle.h>
#include <parse_file.h>
#include <parse_fortran.h>
#include <parse_make.h>
#include <parse_md.h>
#include <parse_py.h>
#include <parse_tcl.h>
#include <parse_xml.h>
#include <perlmodgen.h>
#include <portable.h>
#include <pre.h>
#include <rtfgen.h>
#include <util.h>

namespace Doxy_Setup {
   QString getValue(QStringList::iterator &iter, QStringList::iterator end);
   void usage();
}

using namespace Doxy_Setup;

enum Options {
     INVALID,
     BLANK_LAYOUT,
     BLANK_STYLE,
     DEBUG_DUMP,
     DEBUG_SYMBOLS,
     DATETIME,
     HELP,
     OUTPUT_APP,
     DVERSION,
};

static QHash<QString, int>     s_extLookup;
static QHash<QString, QString> s_langMapping;

void initDoxyPress()
{
   printf("\nInitialization\n");

   const QString lang = portable_getenv("LC_ALL");

   if (! lang.isEmpty()) {
      portable_setenv("LANG", lang);
   }

   setlocale(LC_ALL, "");
   setlocale(LC_CTYPE, "C");       // to get isspace(0xA0)==0, needed for UTF-8
   setlocale(LC_NUMERIC, "C");

   QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

   initPreprocessor();

   Doxy_Globals::parserManager.registerDefaultParser(new FileParser);

   Doxy_Globals::parserManager.registerParser("c",            new CPP_Parser);
   Doxy_Globals::parserManager.registerParser("python",       new Python_Parser);
   Doxy_Globals::parserManager.registerParser("fortran",      new Fortran_Parser);
   Doxy_Globals::parserManager.registerParser("fortranfree",  new Fortran_ParserFree);
   Doxy_Globals::parserManager.registerParser("fortranfixed", new Fortran_ParserFixed);
   Doxy_Globals::parserManager.registerParser("tcl",          new Tcl_Parser);
   Doxy_Globals::parserManager.registerParser("xml",          new XML_Parser);

   Doxy_Globals::parserManager.registerParser("make",         new MakeFileParser);
   Doxy_Globals::parserManager.registerParser("md",           new MarkdownFileParser);

   // register additional parsers here
   initDefaultLangMapping();

   // register user language mappings
   initUserLangMapping();

   initClassMemberIndices();
   initNamespaceMemberIndices();
   initFileMemberIndices();

   Doxy_Globals::globalScope       = QMakeShared<NamespaceDef>("<globalScope>", 1, 1, "<globalScope>");
   Doxy_Globals::classSDict        = ClassSDict(Config::getCase("sort-class-case-sensitive"));

   // initialize global constants
   Doxy_Globals::g_compoundKeywordDict.insert("template class");
   Doxy_Globals::g_compoundKeywordDict.insert("template struct");
   Doxy_Globals::g_compoundKeywordDict.insert("class");
   Doxy_Globals::g_compoundKeywordDict.insert("struct");
   Doxy_Globals::g_compoundKeywordDict.insert("union");
   Doxy_Globals::g_compoundKeywordDict.insert("interface");
   Doxy_Globals::g_compoundKeywordDict.insert("exception");
}

void shutDownDoxyPress()
{
   removePreProcessor();
   Doxy_Globals::indexList.clear();

   Mappers::freeMappers();
   codeFreeScanner();
}

// **
CommandLine parseCommandLine(QStringList argList)
{
   CommandLine cmdArgs;

   QFlatMap<QString, Options> argMap;
   argMap.insert( "-b",         OUTPUT_APP      );
   argMap.insert( "-d",         DEBUG_DUMP      );
   argMap.insert( "-h",         HELP            );
   argMap.insert( "-l",         BLANK_LAYOUT    );
   argMap.insert( "-m",         DEBUG_SYMBOLS   );
   argMap.insert( "-w",         BLANK_STYLE     );
   argMap.insert( "-v",         DVERSION        );

   argMap.insert( "--dt",       DATETIME        );
   argMap.insert( "--help",     HELP            );
   argMap.insert( "--version",  DVERSION        );

   QStringList dashList;
   for (auto item : argList) {

      if (item.startsWith("-")) {
         dashList.append(item);
      }
   }

   auto iter = argList.begin();

   for (auto item : dashList) {
      Options value = INVALID;

      if (argMap.contains(item)) {
         value = argMap.value(item);
         ++iter;
      }

      switch (value) {

         case INVALID:
            err("Option %s is invalid\n", csPrintable(item));
            Doxy_Work::stopDoxyPress();
            break;

         case BLANK_LAYOUT:
            cmdArgs.generateLayout = true;
            cmdArgs.layoutName = getValue(iter, argList.end());

            if (cmdArgs.layoutName.isEmpty()) {
               cmdArgs.layoutName = "doxy_layout.xml";
            }

            cmdArgs.generateDoxy = false;
            break;

         case BLANK_STYLE:
            cmdArgs.formatName = getValue(iter, argList.end());

            if (cmdArgs.formatName.isEmpty()) {
               err("Option \"-w\" is missing format specifier rtf, html or latex\n");
               Doxy_Work::stopDoxyPress();
            }

            cmdArgs.formatName = cmdArgs.formatName.toLower();

            if (cmdArgs.formatName == "rtf-ext") {
               cmdArgs.rtfExt = getValue(iter, argList.end());

               if (cmdArgs.rtfExt.isEmpty()) {
                  cmdArgs.rtfExt = "doxy_ext.rtf";
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


            } else if (cmdArgs.formatName == "html-head") {
               cmdArgs.htmlHead = getValue(iter, argList.end());

               if (cmdArgs.htmlHead.isEmpty()) {
                  cmdArgs.htmlHead = "doxy_header.html";
               }

               QFile f;
               if (openOutputFile(cmdArgs.htmlHead, f)) {
                  HtmlGenerator::writeHeaderFile(f);
               }
               f.close();


            } else if (cmdArgs.formatName == "html-foot") {
               cmdArgs.htmlFoot = getValue(iter, argList.end());

               if (cmdArgs.htmlFoot.isEmpty()) {
                  cmdArgs.htmlFoot = "doxy_footer.html";
               }

               QFile f;
               if (openOutputFile(cmdArgs.htmlFoot, f)) {
                  HtmlGenerator::writeFooterFile(f);
               }
               f.close();


            } else if (cmdArgs.formatName == "html-style") {
              cmdArgs.htmlStyle = getValue(iter, argList.end());

              if (cmdArgs.htmlStyle.isEmpty()) {
                 cmdArgs.htmlStyle = "doxypress.css";
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

               QFile f;
               if (openOutputFile(cmdArgs.latexHead, f)) {
                 LatexGenerator::writeHeaderFile(f);
               }
               f.close();


            } else if (cmdArgs.formatName == "latex-foot") {
              cmdArgs.latexFoot = getValue(iter, argList.end());

               if (cmdArgs.latexFoot.isEmpty()) {
                  cmdArgs.latexFoot = "doxy_footer.latex";
               }

               QFile f;
               if (openOutputFile(cmdArgs.latexFoot, f)) {
                  LatexGenerator::writeFooterFile(f);
               }
               f.close();


            } else if (cmdArgs.formatName == "latex-style") {
              cmdArgs.latexStyle = getValue(iter, argList.end());

               if (cmdArgs.latexStyle.isEmpty()) {
                  cmdArgs.latexStyle= "doxypress.sty";
               }

              QFile f;
              if (openOutputFile(cmdArgs.latexStyle, f)) {
                 LatexGenerator::writeStyleSheetFile(f);
               }
               f.close();


            } else  {
               err("Option \"-w %s\" is invalid\n", csPrintable(cmdArgs.formatName));
               Doxy_Work::stopDoxyPress();

            }

            cmdArgs.generateDoxy = false;
            break;

         case DEBUG_DUMP:
            cmdArgs.debugLabel = getValue(iter, argList.end());

            if (cmdArgs.debugLabel.isEmpty() ) {
               err("Option \"-d\" is missing a debug specifier\n");
               Doxy_Work::stopDoxyPress();
            }

            if (! Debug::setFlag(cmdArgs.debugLabel)) {
               err("Option \"-d\" has an unknown debug specifier: \"%s\".\n", csPrintable(cmdArgs.debugLabel));
               Doxy_Work::stopDoxyPress();
            }

            break;

         case DEBUG_SYMBOLS:
            Doxy_Globals::dumpGlossary = true;
            break;

         case OUTPUT_APP:
            setvbuf(stdout, NULL, _IONBF, 0);
            setvbuf(stderr, NULL, _IONBF, 0);

            Doxy_Globals::outputToApp = true;
            break;

         case DATETIME:
            cmdArgs.dateTimeStr = getValue(iter, argList.end());
            break;

         case HELP:
            usage();
            exit(0);

         case DVERSION:
            printf("\nDoxyPress Version: %s\n", csPrintable(versionString));
            printf("email: info@copperspice.com\n");
            exit(0);
      }
   }

   // is there anything left in the arguemnent list
   cmdArgs.configName = getValue(iter, argList.end());

   return cmdArgs;
}

void readConfiguration(struct CommandLine cmdArgs)
{
   // parse project file
   printf("\nDoxyPress Version: %s\n\n", csPrintable(versionString));

   if (cmdArgs.generateLayout) {
      writeDefaultLayoutFile(cmdArgs.layoutName);
   }

   if (! cmdArgs.generateDoxy) {
      exit(0);
   }

   if (cmdArgs.configName.isEmpty()) {

      for (auto item : QDir::current().entryList()) {

         if (item.compare("doxypress.json", Qt::CaseInsensitive) == 0) {
            cmdArgs.configName = item;
            break;
         }
      }

      if (cmdArgs.configName.isEmpty()) {

         printf("Usage: doxypress [OPTIONS] [project file name]\n");
         printf("doxypress --help for more information\n\n");

         printf("No project file was specified, default file 'doxypress.json' was not found\n\n");
         exit(1);
      }
   }

   QFileInfo fi(cmdArgs.configName);

   if (! fi.exists()) {
      printf("Project file %s was not found\n", csPrintable(cmdArgs.configName));
      exit(1);
   }

   // step 1
   if (! Config::parseConfig(cmdArgs.configName) ) {
      printf("\n\nUnable to parse the project file, please verify the syntax. \n"
             "For additional assistance submit a report to the developers.\nEmail: info@copperspice.com\n\n");
      exit(1);
   }

   // set for internal usage
   const QString dateTimeFormat = "yyyy/MM/dd HH:mm:ss";

   if (cmdArgs.dateTimeStr.isEmpty()) {
      QDateTime dt = QDateTime::currentDateTime();
      cmdArgs.dateTimeStr = dt.toString(dateTimeFormat);
   }

   Doxy_Globals::dateTime = QDateTime::fromString(cmdArgs.dateTimeStr, dateTimeFormat);

   // Perlmod path to the project file
   QFileInfo configFileInfo(cmdArgs.configName);
   setPerlModDoxyfile(configFileInfo.absoluteFilePath());
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

bool openOutputFile(const QString &outFile, QFile &f)
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
         QFileInfo backup(fi.filePath() + ".bak");

         if (backup.exists()) {
            // remove existing backup
            dir.remove(backup.filePath());
         }

         dir.rename(fi.filePath(), fi.filePath() + ".bak");
      }

      f.setFileName(outFile);
      fileOpened = f.open(QIODevice::WriteOnly | QIODevice::Text);
   }

   return fileOpened;
}

struct LangStruct {

   LangStruct(const char *key, SrcLangExt lang) {
      parserName = QString::fromLatin1(key);
      parserEnum = lang;
   }

   QString parserName;
   SrcLangExt    parserEnum;
};

//  language       parser Name        parser enum
static std::map<QString, LangStruct> s_languageTable {

   { "idl",          {"c",            SrcLangExt_IDL      }},
   { "java",         {"c",            SrcLangExt_Java     }},
   { "csharp",       {"c",            SrcLangExt_CSharp   }},
   { "d",            {"c",            SrcLangExt_D        }},
   { "docs",         {"c",            SrcLangExt_Docs     }},
   { "php",          {"c",            SrcLangExt_PHP      }},
   { "objective-c",  {"c",            SrcLangExt_ObjC     }},
   { "c",            {"c",            SrcLangExt_Cpp      }},
   { "c++",          {"c",            SrcLangExt_Cpp      }},
   { "fortran",      {"fortran",      SrcLangExt_Fortran  }},
   { "fortranfree",  {"fortranfree",  SrcLangExt_Fortran  }},
   { "fortranfixed", {"fortranfixed", SrcLangExt_Fortran  }},
   { "javascript",   {"c",            SrcLangExt_JS       }},
   { "python",       {"python",       SrcLangExt_Python   }},
   { "md",           {"md",           SrcLangExt_Markdown }},
   { "make",         {"make",         SrcLangExt_Make     }},
   { "tcl",          {"tcl",          SrcLangExt_Tcl      }},
   { "xml",          {"xml",          SrcLangExt_XML      }}
};

bool updateLanguageMapping(const QString &ext, const QString &language, bool userParser)
{
   if (ext.isEmpty()) {
      return false;
   }

   // part 1
   QString searchName = language.toLower();
   auto i = s_languageTable.find(searchName);

   if (i == s_languageTable.end()) {
      return false;
   }

   // found the language
   SrcLangExt parserId = i->second.parserEnum;

   // part 2
   QString extension = ext.toLower();

   if (! extension.startsWith('.')) {
      extension.prepend(".");
   }

   s_extLookup.insert(extension, parserId);

   if (userParser) {
      // user parsers will be registered later
      s_langMapping.insert(extension, i->second.parserName);

   } else if (! Doxy_Globals::parserManager.registerExtension(extension, i->second.parserName)) {
      msg("Unable to assign extension %-4s (%-7s), currently unsupported\n",
                  csPrintable(extension), csPrintable(i->second.parserName));
   }

   return true;
}

void initUserLangMapping()
{
   for (auto item = s_langMapping.begin(); item != s_langMapping.end(); item++)  {

      QString extension  = item.key();
      QString parserName = item.value();

      if (! Doxy_Globals::parserManager.registerExtension(extension, parserName)) {

         msg("Unable to assign extension %-4s (%-7s), currently unsupported\n",
             csPrintable(extension), csPrintable(parserName));
      }
   }
}

void initDefaultLangMapping()
{
   //                   extension      parser id
   updateLanguageMapping(".dox",      "docs");
   updateLanguageMapping(".txt",      "docs");
   updateLanguageMapping(".doc",      "docs");
   updateLanguageMapping(".c",        "c");
   updateLanguageMapping(".C",        "c");
   updateLanguageMapping(".cc",       "c");
   updateLanguageMapping(".CC",       "c");
   updateLanguageMapping(".cxx",      "c");
   updateLanguageMapping(".cpp",      "c");
   updateLanguageMapping(".c++",      "c");
   updateLanguageMapping(".ccm",      "c");
   updateLanguageMapping(".cppm",     "c");
   updateLanguageMapping(".cxxm",     "c");
   updateLanguageMapping(".c++m",     "c");
   updateLanguageMapping(".ii",       "c");
   updateLanguageMapping(".ixx",      "c");
   updateLanguageMapping(".ipp",      "c");
   updateLanguageMapping(".i++",      "c");
   updateLanguageMapping(".inl",      "c");
   updateLanguageMapping(".h",        "c");
   updateLanguageMapping(".H",        "c");
   updateLanguageMapping(".hh",       "c");
   updateLanguageMapping(".HH",       "c");
   updateLanguageMapping(".hxx",      "c");
   updateLanguageMapping(".hpp",      "c");
   updateLanguageMapping(".h++",      "c");
   updateLanguageMapping(".idl",      "idl");
   updateLanguageMapping(".ddl",      "idl");
   updateLanguageMapping(".odl",      "idl");
   updateLanguageMapping(".java",     "java");
   updateLanguageMapping(".as",       "javascript");
   updateLanguageMapping(".js",       "javascript");
   updateLanguageMapping(".cs",       "csharp");
   updateLanguageMapping(".d",        "d");
   updateLanguageMapping(".php",      "php");
   updateLanguageMapping(".php4",     "php");
   updateLanguageMapping(".php5",     "php");
   updateLanguageMapping(".inc",      "php");
   updateLanguageMapping(".phtml",    "php");
   updateLanguageMapping(".m",        "objective-c");
   updateLanguageMapping(".M",        "objective-c");
   updateLanguageMapping(".mm",       "objective-c");
   updateLanguageMapping(".mpp",      "c");
   updateLanguageMapping(".py",       "python");
   updateLanguageMapping(".pyw",      "python");
   updateLanguageMapping(".f",        "fortran");
   updateLanguageMapping(".for",      "fortran");
   updateLanguageMapping(".f90",      "fortran");
   updateLanguageMapping(".f95",      "fortran");
   updateLanguageMapping(".f03",      "fortran");
   updateLanguageMapping(".f08",      "fortran");
   updateLanguageMapping(".f18",      "fortran");
   updateLanguageMapping(".tcl",      "tcl");
   updateLanguageMapping(".md",       "md");
   updateLanguageMapping(".markdown", "md");
   updateLanguageMapping(".mk",       "make");
}

void addCodeOnlyMappings()
{
   updateLanguageMapping(".xml",      "xml");
}

SrcLangExt getLanguageFromFileName(const QString &fileName)
{
   QFileInfo fi(fileName);

   // get the filename extension, everything after the last dot
   QString ext = fi.suffix().toLower();

   if (ext.isEmpty()) {
      ext = ".no_extension";

   } else {
      ext.prepend(".");
   }

   if (! ext.isEmpty()) {
      // s_extLookUp data obtained from initDefaultLangMapping and user mapping
      auto iter = s_extLookup.find(ext);

      if (iter != s_extLookup.end() ) {
         // listed extension
         return (SrcLangExt) *iter;
      }
   }

   return SrcLangExt_Cpp;    // not listed, assume C language
}

QString getFileNameExtension(const QString &fileName)
{
   QString retval;

   if (fileName.isEmpty())  {
      return retval;
   }
   QFileInfo fi(fileName);

   // get the filename extension, everything after the last dot
   retval = fi.suffix();

   return retval;
}

void Doxy_Setup::usage()
{
   printf("\n");
   printf("DoxyPress: Version %s\n", csPrintable(versionString));
   printf("email: info@copperspice.com\n");

   printf("\n\n");
   printf("Usage: doxypress [OPTIONS] [project file name]\n");

   printf("\n");
   printf("Generate a style sheet file for RTF, HTML or Latex:\n");
   printf("   RTF:   -w  rtf-ext     [extensions file name]   Default is 'doxy_ext.rtf'\n");
   printf("   RTF:   -w  rtf-style   [stylesheet file name]   Default is 'doxy_style.rtf'\n");

   printf("\n");
   printf("   HTML:  -w  html-head   [header file name]       Default is 'doxy_header.html'\n");
   printf("   HTML:  -w  html-foot   [footer file name]       Default is 'doxy_footer.html'\n");
   printf("   HTML:  -w  html-style  [styleSheet file name]   Default is 'doxypress.css'\n");

   printf("\n");
   printf("   LaTeX: -w  latex-head  [header file name]       Default is 'doxy_header.latex'\n");
   printf("   LaTeX: -w  latex-foot  [footer file name]       Default is 'doxy_footer.latex'\n");
   printf("   LaTeX: -w  latex-style [styleSheet file name]   Default is 'doxypress.sty'\n");

   printf("\n\n");
   printf("Generate layout file which can be configured for documentation output:\n");
   printf("   -l   [layout file name]   Default is `doxy_layout.xml'\n");

   printf("\n");
   printf("Use passed date/time value in the output footer (yyyy/MM/dd HH:mm:ss):\n");
   printf("   --dt <date_time>          Default is the current system date and time\n");

   printf("\n");
   printf("Other Options:\n");
   printf("   -b      turns off output buffering of displayed messages\n");
   printf("   -h      display this help message\n");
   printf("   -m      dump symbol map\n");
   printf("   -v      display DoxyPress version\n");

   printf("\n");
   printf("Enable debug level by listing one or more flags:\n");
   printf("   -d   <level>\n");

   printf("\n");
   Debug::printFlags();
   printf("\n");
}
