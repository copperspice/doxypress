/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim
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

#ifndef DOXY_GLOBALS_H
#define DOXY_GLOBALS_H

#include <QByteArray>
#include <QCache>
#include <QHash>
#include <QList>
#include <QString>
#include <QSharedPointer>
#include <QTime>

#include <classlist.h>
#include <cite.h>
#include <dirdef.h>
#include <entry.h>
#include <filenamelist.h>
#include <filestorage.h>
#include <formula.h>
#include <groupdef.h>
#include <membergroup.h>
#include <membername.h>
#include <message.h>
#include <namespacedef.h>
#include <pagedef.h>
#include <reflist.h>
#include <searchindex.h>
#include <stringmap.h>

class ParserManager;

enum ParserMode {
   SOURCE_FILE,
   INCLUDE_FILE
};

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
   {
   }

   void begin(const QString &name) {
      msg(name);

      stat *entry = new stat(name, 0);
      stats.append(entry);
      time.restart();
   }

   void end() {
      stats.last()->elapsed = ((double)time.elapsed()) / 1000.0;
   }

   void print() {
      bool restore = false;

      if (Debug::isFlagSet(Debug::Time)) {
         Debug::clearFlag("time");
         restore = true;
      }

      msg("----------------------\n");

      for (auto item : stats) {
         msg("Spent %.3f seconds in %s", item->elapsed, csPrintable(item->name));
      }

      if (restore) {
         Debug::setFlag("time");
      }
   }

 private:
   struct stat {
      QString name;
      double elapsed;

      stat() : elapsed(0)
      {}

      stat(const QString &n, double el) : name(n), elapsed(el)
      {}
   };

   QList<stat *> stats;
   QTime time;

}; 

/*! \brief This class serves as a namespace for global variables used by DoxyPress
 *
 *  All fields in this class are public and static so they can be used directly.
 */
class Doxy_Globals 
{
   public:
      static QSharedPointer<PageDef>       mainPage;
      static QSharedPointer<NamespaceDef>  globalScope;
    
      static CiteDict                 *citeDict;
   
      static ClassSDict               *classSDict;
      static ClassSDict               *hiddenClasses;
   
      static DirSDict                 directories;
    
      static FormulaDict              *formulaDict;
      static FormulaDict              *formulaNameDict;
       
      static QSharedPointer<GenericsSDict> genericsDict;     

      static GroupSDict               *groupSDict;
     
      static FileNameDict             *includeNameDict;
      static FileNameDict             *exampleNameDict;   
      static FileNameDict             *inputNameDict;
   
      static SortedList<QSharedPointer<FileNameList>> *inputNameList;
   
      static FileNameDict             *imageNameDict;
      static FileNameDict             *dotFileNameDict;
      static FileNameDict             *mscFileNameDict;
      static FileNameDict             *diaFileNameDict;   
   
      static MemberNameSDict          *memberNameSDict;
      static MemberNameSDict          *functionNameSDict;
      static NamespaceSDict           *namespaceSDict;
      static PageSDict                *exampleSDict;
      static PageSDict                *pageSDict;
      static SectionDict              *sectionDict;
   
      static StringDict                namespaceAliasDict;
      static StringDict                tagDestinationDict;
      static StringDict                cmdAliasDict;    
      static StringDict                renameNSDict;  
   
      //
      static FormulaList              *formulaList; 
      static ParserManager            *parserManager;  
      static SearchIndexIntf          *searchIndex; 
      static Store                    *symbolStorage;  
      static IndexList                *indexList;
   
      //
      static QList<QByteArray>        tagfileList;   
   
      static QSet<QString>            inputPaths;   
      static QSet<QString>            expandAsDefinedDict;   
      static QHash<QString, RefList>  *xrefLists;           // array of xref lists: todo, test, bug, deprecated ...
      static QHash<QString, int>      *htmlDirMap;
     
      static QHash<QString, QSharedPointer<Definition>> clangUsrMap;
   
      static QHash<long, QSharedPointer<MemberGroupInfo>> memGrpInfoDict;
      static StringMap<QSharedPointer<DirRelation>> dirRelations; 
        
      static QCache<QString, LookupInfo> *lookupCache;
         
      static QTime runningTime;   
      static int subpageNestingLevel;
   
      static QString htmlFileExtension;
      static QString latexStyleExtension;
   
      static QString tempA_FName;
      static QString tempB_FName;
      
      static bool  suppressDocWarnings;
      static bool  outputToApp;
      static bool  gatherDefines;
      static bool  userComments;   
      static bool  insideMainPage;
      static bool  parseSourcesNeeded;
      static bool  generatingXmlOutput;
      static bool  markdownSupport;     
      
      static QHash<QString, Definition *> &symbolMap();
      static QHash<QString, QSharedPointer<EntryNav>>    g_classEntries;

      static QStringList               g_inputFiles;
      static QSet<QString>             g_compoundKeywordDict;   // keywords recognised as compounds
      static QHash<QString, FileDef>   g_usingDeclarations;     // used classes
      static QSet<QString>             g_pathsVisited;
   
      static OutputList               *g_outputList;            // list of output generating objects     
      static FileStorage              *g_storage;
      
      static Statistics g_stats; 
         
      static bool g_dumpSymbolMap;
      static bool g_useOutputTemplate;

      static bool g_programExit;
};

#endif