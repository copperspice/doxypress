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
#include <doxygen.h>
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

#include <doxy_globals.h>

// part 1
ClassSDict      *Doxygen::classSDict = 0;
ClassSDict      *Doxygen::hiddenClasses = 0;
NamespaceSDict  *Doxygen::namespaceSDict = 0;
MemberNameSDict *Doxygen::memberNameSDict = 0;
MemberNameSDict *Doxygen::functionNameSDict = 0;

SortedList<FileName *>  *Doxygen::inputNameList = 0;       // all input files

FileNameDict    *Doxygen::inputNameDict = 0;
GroupSDict      *Doxygen::groupSDict = 0;
FormulaList     *Doxygen::formulaList = 0;         // all formulas
FormulaDict     *Doxygen::formulaDict = 0;         // all formulas
FormulaDict     *Doxygen::formulaNameDict = 0;     // the label name of all formulas
PageSDict       *Doxygen::pageSDict = 0;
PageSDict       *Doxygen::exampleSDict = 0;
SectionDict     *Doxygen::sectionDict = 0;         // all page sections
CiteDict        *Doxygen::citeDict = 0;            // database of bibliographic references
StringDict       Doxygen::aliasDict;               // aliases
DirSDict         Doxygen::directories;
GenericsSDict   *Doxygen::genericsDict;

FileNameDict    *Doxygen::includeNameDict = 0;     // include names
FileNameDict    *Doxygen::exampleNameDict = 0;     // examples
FileNameDict    *Doxygen::imageNameDict = 0;       // images
FileNameDict    *Doxygen::dotFileNameDict = 0;     // dot files
FileNameDict    *Doxygen::mscFileNameDict = 0;     // msc files
FileNameDict    *Doxygen::diaFileNameDict = 0;     // dia files

StringDict       Doxygen::namespaceAliasDict;      // all namespace aliases
StringDict       Doxygen::tagDestinationDict;      // all tag locations

QCache<QString, LookupInfo>     *Doxygen::lookupCache;

QHash<QString, void *>           Doxygen::inputPaths;
QHash<QString, void *>           Doxygen::expandAsDefinedDict;                        // all macros that should be expanded

QHash<QString, QSharedPointer<QList<Definition *>>> *Doxygen::symbolMap = 0;
 
QHash<QString, Definition *>    *Doxygen::clangUsrMap = 0;

QHash<QString, RefList>         *Doxygen::xrefLists = new QHash<QString, RefList>;    // dictionary of cross-referenced item lists
QHash<QString, int>             *Doxygen::htmlDirMap = 0;

QHash<long, QSharedPointer<MemberGroupInfo>> Doxygen::memGrpInfoDict;                 // dictionary of the member groups heading
StringMap<QSharedPointer<DirRelation>>       Doxygen::dirRelations;

PageDef         *Doxygen::mainPage = 0;
NamespaceDef    *Doxygen::globalScope = 0;
SearchIndexIntf *Doxygen::searchIndex = 0;
ParserManager   *Doxygen::parserManager = 0;
Store           *Doxygen::symbolStorage;
IndexList       *Doxygen::indexList;

bool             Doxygen::insideMainPage = false;       // are we generating docs for the main page?
bool             Doxygen::parseSourcesNeeded = false;
bool             Doxygen::outputToWizard = false;
bool             Doxygen::suppressDocWarnings = false;
bool             Doxygen::gatherDefines = true;
bool             Doxygen::userComments = false;
bool             Doxygen::generatingXmlOutput = false;
bool             Doxygen::markdownSupport = true;

QTime            Doxygen::runningTime;

QByteArray       Doxygen::htmlFileExtension;

QString          Doxygen::objDBFileName;
QString          Doxygen::entryDBFileName;

int              Doxygen::subpageNestingLevel = 0;


// part 2
QHash<QString, EntryNav>  Doxy_Globals::g_classEntries;
QStringList               Doxy_Globals::g_inputFiles;
QHash<QString, void *>    Doxy_Globals::g_compoundKeywordDict;   
QHash<QString, FileDef>   Doxy_Globals::g_usingDeclarations;  
QHash<QString, void *>    Doxy_Globals::g_pathsVisited;  

OutputList               *Doxy_Globals::g_outputList = 0;           
FileStorage              *Doxy_Globals::g_storage = 0;

Statistics Doxy_Globals::g_stats;

bool Doxy_Globals::g_successfulRun     = false;
bool Doxy_Globals::g_dumpSymbolMap     = false;
