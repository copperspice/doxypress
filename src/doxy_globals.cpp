/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
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

#include <doxy_globals.h>
#include <filedef.h>

class GenericsSDict;
class IndexList;

// part 1
ClassSDict      *Doxy_Globals::classSDict = 0;
ClassSDict      *Doxy_Globals::hiddenClasses = 0;
NamespaceSDict  *Doxy_Globals::namespaceSDict = 0;
MemberNameSDict *Doxy_Globals::memberNameSDict = 0;
MemberNameSDict *Doxy_Globals::functionNameSDict = 0;
GroupSDict      *Doxy_Globals::groupSDict = 0;
PageSDict       *Doxy_Globals::pageSDict = 0;
PageSDict       *Doxy_Globals::exampleSDict = 0;

FormulaList     *Doxy_Globals::formulaList = 0;         // all formulas
FormulaDict     *Doxy_Globals::formulaDict = 0;         // all formulas
FormulaDict     *Doxy_Globals::formulaNameDict = 0;     // the label name of all formulas

SectionDict     *Doxy_Globals::sectionDict = 0;         // all page sections
CiteDict        *Doxy_Globals::citeDict = 0;            // database of bibliographic references

StringDict       Doxy_Globals::namespaceAliasDict;      // all namespace aliases
StringDict       Doxy_Globals::tagDestinationDict;      // all tag locations
StringDict       Doxy_Globals::cmdAliasDict;            // cmd aliases
StringDict       Doxy_Globals::renameNSDict;            // rename namespaces

DirSDict         Doxy_Globals::directories;

QSharedPointer<GenericsSDict> Doxy_Globals::genericsDict;

FileNameDict    *Doxy_Globals::inputNameDict = 0;
FileNameDict    *Doxy_Globals::includeNameDict = 0;     // include names
FileNameDict    *Doxy_Globals::exampleNameDict = 0;     // examples
FileNameDict    *Doxy_Globals::imageNameDict = 0;       // images
FileNameDict    *Doxy_Globals::dotFileNameDict = 0;     // dot files
FileNameDict    *Doxy_Globals::mscFileNameDict = 0;     // msc files
FileNameDict    *Doxy_Globals::diaFileNameDict = 0;     // dia files

SortedList<QSharedPointer<FileNameList>> *Doxy_Globals::inputNameList; // all input files

QCache<QString, LookupInfo>  *Doxy_Globals::lookupCache;

QSet<QString>             Doxy_Globals::inputPaths;
QSet<QString>             Doxy_Globals::expandAsDefinedDict;                        // all macros that should be expanded
QHash<QString, RefList>  *Doxy_Globals::xrefLists = new QHash<QString, RefList>;    // dictionary of cross-referenced item lists
QHash<QString, int>      *Doxy_Globals::htmlDirMap = 0;

QHash<QString, QSharedPointer<Definition>>   Doxy_Globals::clangUsrMap;
QHash<long, QSharedPointer<MemberGroupInfo>> Doxy_Globals::memGrpInfoDict;          // dictionary of the member groups heading

StringMap<QSharedPointer<DirRelation>>       Doxy_Globals::dirRelations;

QSharedPointer<PageDef>       Doxy_Globals::mainPage;
QSharedPointer<NamespaceDef>  Doxy_Globals::globalScope;

SearchIndexIntf *Doxy_Globals::searchIndex = 0;
ParserManager   *Doxy_Globals::parserManager = 0;
Store           *Doxy_Globals::symbolStorage;
IndexList       *Doxy_Globals::indexList;

bool             Doxy_Globals::insideMainPage = false;       // are we generating docs for the main page?
bool             Doxy_Globals::parseSourcesNeeded = false;
bool             Doxy_Globals::outputToApp = false;
bool             Doxy_Globals::suppressDocWarnings = false;
bool             Doxy_Globals::gatherDefines = true;
bool             Doxy_Globals::userComments = false;
bool             Doxy_Globals::generatingXmlOutput = false;
bool             Doxy_Globals::markdownSupport = true;

int              Doxy_Globals::subpageNestingLevel = 0;

QString          Doxy_Globals::htmlFileExtension;
QString          Doxy_Globals::latexStyleExtension = ".sty";

QString          Doxy_Globals::tempA_FName;
QString          Doxy_Globals::tempB_FName;
QTime            Doxy_Globals::runningTime;


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
QHash<QString, Definition *> &Doxy_Globals::symbolMap()
{
   static QHash<QString, Definition *> data;
   return data;  
} 
