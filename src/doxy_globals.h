/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
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

#ifndef DOXY_GLOBALS_H
#define DOXY_GLOBALS_H

#include <QByteArray>
#include <QCache>
#include <QHash>
#include <QMultiHash>
#include <QList>
#include <QString>
#include <QSharedPointer>
#include <QTime>

#include <classlist.h>
#include <cite.h>
#include <configure.h>
#include <dirdef.h>
#include <entry.h>
#include <filenamelist.h>
#include <formula.h>
#include <groupdef.h>
#include <membergroup.h>
#include <membername.h>
#include <message.h>
#include <namespacedef.h>
#include <pagedef.h>
#include <parse_base.h>
#include <reflist.h>
#include <searchindex.h>
#include <stringmap.h>

struct LookupInfo {
   LookupInfo() {}

   LookupInfo(QSharedPointer<ClassDef> cd, QSharedPointer<MemberDef> td, QString ts, QString rt)
      : classDef(cd), typeDef(td), templSpec(ts), resolvedType(rt)
   {}

   QSharedPointer<ClassDef>  classDef;
   QSharedPointer<MemberDef> typeDef;

   QString templSpec;
   QString resolvedType;
};

class StringDict : public QHash<QString, QString>
{
 public:
   StringDict() : QHash<QString, QString>()
   {}

   virtual ~StringDict()
   {}
};

class Statistics
{
 public:
   Statistics()
   {}

   void begin(const QString &name) {
      msg(name);
   }

   void end() {
   }

};

namespace Doxy_Work{
   void stopDoxyPress(int unused = 0);
}

/*! This class is for global variables used by DoxyPress
 *
 *  All fields in this class are public and static
 */
class Doxy_Globals
{
   public:
      static CiteDict          citeDict;

      static ClassSDict        classSDict;
      static ClassSDict        hiddenClasses;

      static DirSDict          directories;

      static FormulaDict       formulaDict;
      static FormulaDict       formulaNameDict;

      static GenericsSDict     genericsSDict;

      static GroupSDict        groupSDict;

      static FileNameDict      diaFileNameDict;
      static FileNameDict      dotFileNameDict;
      static FileNameDict      exampleNameDict;
      static FileNameDict      includeNameDict;
      static FileNameDict      inputNameDict;
      static FileNameDict      imageNameDict;
      static FileNameDict      mscFileNameDict;

      static MemberNameSDict   memberNameSDict;
      static MemberNameSDict   functionNameSDict;

      static NamespaceSDict    namespaceSDict;

      static PageSDict         exampleSDict;
      static PageSDict         pageSDict;

      static SectionDict       sectionDict;

      static StringDict        cmdAliasDict;
      static StringDict        namespaceAliasDict;
      static StringDict        tagDestinationDict;

      static FormulaList       formulaList;
      static IndexList         indexList;
      static ParserManager     parserManager;
      static OutputList        outputList;

      static Statistics        infoLog_Stat;

      static SortedList<QSharedPointer<FileNameList>> inputNameList;

      static QSharedPointer<PageDef>           mainPage;
      static QSharedPointer<NamespaceDef>      globalScope;
      static QSharedPointer<SearchIndex_Base>  searchIndexBase;

      static QSet<QString>             inputPaths;
      static QSet<QString>             expandAsDefinedDict;

      static QHash<QString, QString>   nsRenameOrig;
      static QHash<QString, QString>   nsRenameAlias;

      static QHash<QString, RefList>   xrefLists;

      static QHash<QString, QSharedPointer<Definition>>   clangUsrMap;
      static QHash<long, QSharedPointer<MemberGroupInfo>> memGrpInfoDict;

      static StringMap<QSharedPointer<DirRelation>>       dirRelations;
      static QCache<QString, LookupInfo>                  lookupCache;

      static QString htmlFileExtension;
      static QString latexStyleExtension;

      static bool gatherDefines;
      static bool outputToApp;
      static bool userComments;
      static bool insideMainPage;
      static bool parseSourcesNeeded;
      static bool generatingXmlOutput;
      static bool markdownSupport;
      static bool suppressDocWarnings;

      static bool dumpGlossary;
      static bool programExit;

      static int documentedFiles;
      static int documentedHtmlFiles;
      static int documentedSrcFiles;
      static int documentedPages;
      static int indexedPages;
      static int subpageNestingLevel;

      static QDateTime dateTime;

      static QHash<QString, QSharedPointer<Entry>>  g_classEntries;

      static QStringList               g_inputFiles;
      static QSet<QString>             g_compoundKeywordDict;
      static QSet<QString>             g_pathsVisited;
      static QMap<QString,  QString>   g_moduleHint;
      static QHash<QString, FileDef>   g_usingDeclarations;

      // must use a raw pointer since this method is called from a constructor
      static QMultiHash<QString, Definition *> &glossary();
};

#endif