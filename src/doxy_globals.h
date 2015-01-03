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

#ifndef DOXY_GLOBALS_H
#define DOXY_GLOBALS_H

#include <QByteArray>
#include <QCache>
#include <QFileInfo>
#include <QHash>
#include <QList>
#include <QString>
#include <QTime>

#include <entry.h>
#include <filestorage.h>
#include <message.h>

class Statistics
{
 public:
   Statistics()
   {
   }

   void begin(const char *name) {
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
         msg("Spent %.3f seconds in %s", item->elapsed, item->name);
      }

      if (restore) {
         Debug::setFlag("time");
      }
   }

 private:
   struct stat {
      const char *name;
      double elapsed;
      stat() : name(NULL), elapsed(0) {}
      stat(const char *n, double el) : name(n), elapsed(el) {}
   };

   QList<stat *> stats;
   QTime time;

}; 

/*! \brief This class serves as a namespace for global variables used by doxygen.
 *
 *  All fields in this class are public and static, so they can be used directly.
 */
class Doxygen
{
 public:
   static PageDef                          *mainPage;
   static NamespaceDef                     *globalScope;
 
   static CiteDict                         *citeDict;

   static ClassSDict                       *classSDict;
   static ClassSDict                       *hiddenClasses;

   static DirSDict                          directories;
 
   static FormulaDict                      *formulaDict;
   static FormulaDict                      *formulaNameDict;
    
   static GenericsSDict                    *genericsDict;
     
   static GroupSDict                       *groupSDict;
  
   static FileNameDict                     *includeNameDict;
   static FileNameDict                     *exampleNameDict;   
   static FileNameDict                     *inputNameDict;
   static SortedList<FileName *>           *inputNameList;
   static FileNameDict                     *imageNameDict;
   static FileNameDict                     *dotFileNameDict;
   static FileNameDict                     *mscFileNameDict;
   static FileNameDict                     *diaFileNameDict;   

   static MemberNameSDict                  *memberNameSDict;
   static MemberNameSDict                  *functionNameSDict;

   static NamespaceSDict                   *namespaceSDict;

   static PageSDict                        *exampleSDict;
   static PageSDict                        *pageSDict;

   static SectionDict                      *sectionDict;

   static StringDict                        namespaceAliasDict;
   static StringDict                        tagDestinationDict;
   static StringDict                        aliasDict;    

   //
   static FormulaList                      *formulaList; 
   static ParserManager                     *parserManager;  
   static SearchIndexIntf                   *searchIndex; 
   static Store                             *symbolStorage;  
   static IndexList                         *indexList;

   //
   static QList<QByteArray>                  tagfileList;   

   static QHash<QString, void *>             inputPaths;   
   static QHash<QString, void *>             expandAsDefinedDict;   
   static QHash<QString, RefList>           *xrefLists;           // array of xref lists: todo, test, bug, deprecated ...
   static QHash<QString, int>               *htmlDirMap;
   static QHash<QString, DefinitionIntf *>  *symbolMap;
   static QHash<QString, Definition *>      *clangUsrMap;

   static QHash<long, QSharedPointer<MemberGroupInfo>>  memGrpInfoDict;
   static StringMap<QSharedPointer<DirRelation>>        dirRelations; 
     
   static QCache<QString, LookupInfo>       *lookupCache;
      
   static QTime       runningTime;   
   static int         subpageNestingLevel;

   static QByteArray  htmlFileExtension;

   static QString  objDBFileName;
   static QString  entryDBFileName;

   static QByteArray  spaces;
  
   static bool  suppressDocWarnings;
   static bool  outputToWizard;
   static bool  gatherDefines;
   static bool  userComments;   
   static bool  insideMainPage;
   static bool  parseSourcesNeeded;
   static bool  generatingXmlOutput;
   static bool  markdownSupport;
  
};

class Doxy_Globals 
{
   public:
      static QHash<QString, EntryNav>  g_classEntries;
      static QStringList               g_inputFiles;
      static QHash<QString, void *>    g_compoundKeywordDict;   // keywords recognised as compounds
      static QHash<QString, FileDef>   g_usingDeclarations;     // used classes
      static QHash<QString, void *>    g_pathsVisited;
   
      static OutputList               *g_outputList;            // list of output generating objects     
      static FileStorage              *g_storage;

      static Statistics g_stats; 
   
      static bool g_successfulRun;
      static bool g_dumpSymbolMap;
      static bool g_useOutputTemplate;
};

template<class T>
QSharedPointer<T> dummyShared(T *ptr)
{
   return QSharedPointer<T> (ptr, [](T *){} );
}


#endif
