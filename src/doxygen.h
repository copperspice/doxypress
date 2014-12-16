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

#ifndef DOXYGEN_H
#define DOXYGEN_H

#include <QCache>
#include <QDatetime>
#include <QFileInfo>
#include <QHash>
#include <QStringList>

#include <ftextstream.h>
#include <stringmap.h>
#include <membergroup.h>
#include <dirdef.h>
#include <memberlist.h>
#include <section.h>
#include <sortedlist.h>

class RefList;
class PageSList;
class PageSDict;
class PageDef;
class SearchIndexIntf;
class ParserManager;
class ObjCache;
class Store;
class BufStr;
class CiteDict;
class MemberDef;
class GroupDef;
class GroupSDict;
class FileDef;
class ClassDef;
class ClassSDict;
class GenericsSDict;
class MemberNameSDict;
class FileNameDict;
class NamespaceSDict;
class NamespaceDef;
class DefinitionIntf;
class DirSDict;
class DirRelation;
class IndexList;
class FormulaList;
class FormulaDict;
class FormulaNameDict;

struct MemberGroupInfo;

extern QByteArray   g_spaces;

struct LookupInfo {
   LookupInfo() : classDef(0), typeDef(0) {}

   LookupInfo(ClassDef *cd, MemberDef *td, QByteArray ts, QByteArray rt)
      : classDef(cd), typeDef(td), templSpec(ts), resolvedType(rt) {}

   ClassDef    *classDef;
   MemberDef   *typeDef;
   QByteArray   templSpec;
   QByteArray   resolvedType;
};

class StringDict : public QHash<QString, QByteArray>
{
 public:
   StringDict() : QHash<QString, QByteArray>() {}
   virtual ~StringDict() {}
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
    
   static GenericsSDict                     *genericsDict;
     
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
   static QHash<long, MemberGroupInfo>       memGrpInfoDict;
   static QHash<QString, void *>             expandAsDefinedDict;   
   static QHash<QString, RefList>           *xrefLists;           // array of xref lists: todo, test, bug, deprecated ...
   static QHash<QString, int>               *htmlDirMap;
   static QHash<QString, DefinitionIntf *>  *symbolMap;
   static QHash<QString, Definition>        *clangUsrMap;

   static StringMap<QSharedPointer<DirRelation>> dirRelations; 
     
   static QCache<QString, LookupInfo>       *lookupCache;
      
   static QTime       runningTime;   
   static int         subpageNestingLevel;

   static QByteArray  htmlFileExtension;
   static QByteArray  objDBFileName;
   static QByteArray  entryDBFileName;
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

void initDoxygen();
void readConfiguration(int argc, char **argv);
void checkConfiguration();
void adjustConfiguration();
void searchInputFiles(QList<QByteArray>  &inputFiles);
void parseInput();
void generateOutput();
void readAliases();
void readFormulaRepository();
void cleanUpDoxygen();

int readFileOrDirectory(const char *s, SortedList<FileName *> *fnList, FileNameDict *fnDict, StringDict *exclDict,
                        QList<QByteArray>  *patList, QList<QByteArray>  *exclPatList, QList<QByteArray>  *resultList,
                        StringDict *resultDict, bool recursive, bool errorIfNotExist = true, QHash<QString,
                        void *> *killDict = 0, QHash<QString, void *> *paths = 0 );

int readDir(QFileInfo *fi, SortedList<FileName *> *fnList, FileNameDict *fnDict, StringDict  *exclDict,
            QList<QByteArray>  *patList, QList<QByteArray>  *exclPatList, QList<QByteArray>  *resultList,
            StringDict *resultDict, bool errorIfNotExist, bool recursive, QHash<QString, void *> *killDict );

void copyAndFilterFile(const char *fileName, BufStr &dest);

#endif
