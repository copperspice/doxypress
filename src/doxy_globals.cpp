/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim
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

CiteDict         Doxy_Globals::citeDict;                   // database of bibliographic references
ClassSDict       Doxy_Globals::classSDict;
ClassSDict       Doxy_Globals::hiddenClasses;

DirSDict         Doxy_Globals::directories;

FormulaDict      Doxy_Globals::formulaDict;                // all formulas
FormulaDict      Doxy_Globals::formulaNameDict;            // the label name of all formulas

GenericsSDict    Doxy_Globals::genericsSDict;

GroupSDict       Doxy_Globals::groupSDict;

FileNameDict     Doxy_Globals::diaFileNameDict;           // dia files
FileNameDict     Doxy_Globals::dotFileNameDict;           // dot files
FileNameDict     Doxy_Globals::exampleNameDict;           // examples
FileNameDict     Doxy_Globals::inputNameDict;
FileNameDict     Doxy_Globals::includeNameDict;           // include names
FileNameDict     Doxy_Globals::imageNameDict;             // images
FileNameDict     Doxy_Globals::mscFileNameDict;           // msc files

MemberNameSDict  Doxy_Globals::memberNameSDict;
MemberNameSDict  Doxy_Globals::functionNameSDict;

NamespaceSDict   Doxy_Globals::namespaceSDict;

PageSDict        Doxy_Globals::pageSDict;
PageSDict        Doxy_Globals::exampleSDict;

SectionDict      Doxy_Globals::sectionDict;                // all page sections

StringDict       Doxy_Globals::cmdAliasDict;               // cmd aliases
StringDict       Doxy_Globals::namespaceAliasDict;         // all namespace aliases
StringDict       Doxy_Globals::tagDestinationDict;         // all tag locations

FormulaList      Doxy_Globals::formulaList;                // all formulas
IndexList        Doxy_Globals::indexList;
ParserManager    Doxy_Globals::parserManager;
Store            Doxy_Globals::symbolStorage;
OutputList       Doxy_Globals::outputList;                 // list of output generating objects
FileStorage      Doxy_Globals::fileStorage;
Statistics       Doxy_Globals::infoLog_Stat;

SortedList<QSharedPointer<FileNameList>> Doxy_Globals::inputNameList;         // all input files

QSharedPointer<PageDef>          Doxy_Globals::mainPage;
QSharedPointer<NamespaceDef>     Doxy_Globals::globalScope;
QSharedPointer<SearchIndex_Base> Doxy_Globals::searchIndexBase;

QSet<QString>             Doxy_Globals::inputPaths;
QSet<QString>             Doxy_Globals::expandAsDefinedDict;                  // all macros which should be expanded

QHash<QString, QString>   Doxy_Globals::nsRenameOrig;                         // rename namespaces ( orignal, alias  )
QHash<QString, QString>   Doxy_Globals::nsRenameAlias;                        // rename namespaces ( alias, original )

QHash<QString, RefList>   Doxy_Globals::xrefLists;                            // cross-referenced item --todo, test, bug, deprecated

QHash<QString, QSharedPointer<Definition>>   Doxy_Globals::clangUsrMap;
QHash<long, QSharedPointer<MemberGroupInfo>> Doxy_Globals::memGrpInfoDict;    // dictionary of the member groups heading

StringMap<QSharedPointer<DirRelation>>       Doxy_Globals::dirRelations;
QCache<QString, LookupInfo>                  Doxy_Globals::lookupCache;

QString Doxy_Globals::htmlFileExtension;
QString Doxy_Globals::latexStyleExtension = ".sty";

QString Doxy_Globals::tempA_FName;
QString Doxy_Globals::tempB_FName;

bool Doxy_Globals::gatherDefines       = true;
bool Doxy_Globals::insideMainPage      = false;            // indicates if doc are generatedd for the main page
bool Doxy_Globals::parseSourcesNeeded  = false;
bool Doxy_Globals::outputToApp         = false;
bool Doxy_Globals::userComments        = false;
bool Doxy_Globals::generatingXmlOutput = false;
bool Doxy_Globals::markdownSupport     = true;
bool Doxy_Globals::suppressDocWarnings = false;

bool Doxy_Globals::dumpGlossary        = false;
bool Doxy_Globals::programExit         = false;

int Doxy_Globals::documentedFiles;
int Doxy_Globals::documentedHtmlFiles;
int Doxy_Globals::documentedSrcFiles;
int Doxy_Globals::documentedPages;
int Doxy_Globals::indexedPages;
int Doxy_Globals::subpageNestingLevel;

QDateTime Doxy_Globals::dateTime;

QHash<QString, QSharedPointer<MiniEntry>> Doxy_Globals::g_classEntries;

QStringList               Doxy_Globals::g_inputFiles;
QSet<QString>             Doxy_Globals::g_compoundKeywordDict;      // keywords recognised as compounds
QHash<QString, FileDef>   Doxy_Globals::g_usingDeclarations;        // used classes
QSet<QString>             Doxy_Globals::g_pathsVisited;

QMap<QString, QString>    Doxy_Globals::g_moduleHint;               // experimental

QMultiHash<QString, Definition *>  &Doxy_Globals::glossary()
{
   static QMultiHash<QString, Definition *> data;
   return data;
}
