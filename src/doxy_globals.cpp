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

#include <cite.h>
#include <classlist.h>
#include <dirdef.h>
#include <doxy_globals.h>
#include <filename.h>
#include <formula.h>
#include <groupdef.h>
#include <index.h>
#include <membergroup.h>
#include <membername.h>
#include <namespacedef.h>
#include <parserintf.h>
#include <pagedef.h>
#include <section.h>
#include <searchindex.h>
#include <reflist.h>

// part 1
ClassSDict      *Doxygen::classSDict = 0;
ClassSDict      *Doxygen::hiddenClasses = 0;
NamespaceSDict  *Doxygen::namespaceSDict = 0;
MemberNameSDict *Doxygen::memberNameSDict = 0;
MemberNameSDict *Doxygen::functionNameSDict = 0;
GroupSDict      *Doxygen::groupSDict = 0;
PageSDict       *Doxygen::pageSDict = 0;
PageSDict       *Doxygen::exampleSDict = 0;

FormulaList     *Doxygen::formulaList = 0;         // all formulas
FormulaDict     *Doxygen::formulaDict = 0;         // all formulas
FormulaDict     *Doxygen::formulaNameDict = 0;     // the label name of all formulas

SectionDict     *Doxygen::sectionDict = 0;         // all page sections
CiteDict        *Doxygen::citeDict = 0;            // database of bibliographic references
StringDict       Doxygen::aliasDict;               // aliases
DirSDict         Doxygen::directories;
GenericsSDict   *Doxygen::genericsDict;

FileNameDict    *Doxygen::inputNameDict = 0;
FileNameDict    *Doxygen::includeNameDict = 0;     // include names
FileNameDict    *Doxygen::exampleNameDict = 0;     // examples
FileNameDict    *Doxygen::imageNameDict = 0;       // images
FileNameDict    *Doxygen::dotFileNameDict = 0;     // dot files
FileNameDict    *Doxygen::mscFileNameDict = 0;     // msc files
FileNameDict    *Doxygen::diaFileNameDict = 0;     // dia files

StringDict       Doxygen::namespaceAliasDict;      // all namespace aliases
StringDict       Doxygen::tagDestinationDict;      // all tag locations

SortedList<QSharedPointer<FileName>> *Doxygen::inputNameList; // all input files

QCache<QString, LookupInfo>  *Doxygen::lookupCache;

QSet<QString>             Doxygen::inputPaths;
QSet<QString>             Doxygen::expandAsDefinedDict;                        // all macros that should be expanded
QHash<QString, RefList>  *Doxygen::xrefLists = new QHash<QString, RefList>;    // dictionary of cross-referenced item lists
QHash<QString, int>       *Doxygen::htmlDirMap = 0;

QHash<QString, QSharedPointer<Definition>>   Doxygen::clangUsrMap;
QHash<long, QSharedPointer<MemberGroupInfo>> Doxygen::memGrpInfoDict;                 // dictionary of the member groups heading

StringMap<QSharedPointer<DirRelation>>       Doxygen::dirRelations;

QSharedPointer<PageDef>       Doxygen::mainPage;
QSharedPointer<NamespaceDef>  Doxygen::globalScope;

SearchIndexIntf *Doxygen::searchIndex = 0;
ParserManager   *Doxygen::parserManager = 0;
Store           *Doxygen::symbolStorage;
IndexList       *Doxygen::indexList;

bool             Doxygen::insideMainPage = false;       // are we generating docs for the main page?
bool             Doxygen::parseSourcesNeeded = false;
bool             Doxygen::outputToApp = false;
bool             Doxygen::suppressDocWarnings = false;
bool             Doxygen::gatherDefines = true;
bool             Doxygen::userComments = false;
bool             Doxygen::generatingXmlOutput = false;
bool             Doxygen::markdownSupport = true;

int              Doxygen::subpageNestingLevel = 0;

QString          Doxygen::htmlFileExtension;
QString          Doxygen::tempA_FName;
QString          Doxygen::tempB_FName;
QTime            Doxygen::runningTime;


// part 2
QHash<QString, QSharedPointer<EntryNav>> Doxy_Globals::g_classEntries;

QStringList               Doxy_Globals::g_inputFiles;
QSet<QString>             Doxy_Globals::g_compoundKeywordDict;   
QHash<QString, FileDef>   Doxy_Globals::g_usingDeclarations;  
QSet<QString>             Doxy_Globals::g_pathsVisited;  

OutputList               *Doxy_Globals::g_outputList = 0;           
FileStorage              *Doxy_Globals::g_storage = 0;

Statistics Doxy_Globals::g_stats;

bool Doxy_Globals::g_dumpSymbolMap = false;
bool Doxy_Globals::g_programExit   = false;


// part 3
QMap<const EnableSharedFromThis *, QWeakPointer<EnableSharedFromThis>> &EnableSharedFromThis::m_selfMap()
{
   static QMap<const EnableSharedFromThis *, QWeakPointer<EnableSharedFromThis>> data;
   return data;
}

QHash<QString, Definition *> &Doxygen::symbolMap()
{
   static QHash<QString, Definition *> data;
   return data;  
} 
