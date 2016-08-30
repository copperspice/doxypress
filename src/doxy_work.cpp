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

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <set>

#include <arguments.h>
#include <cite.h>
#include <cmdmapper.h>
#include <code_cstyle.h>
#include <config.h>
#include <commentcnv.h>
#include <config.h>
#include <declinfo.h>
#include <defargs.h>
#include <dirdef.h>
#include <docbookgen.h>
#include <docparser.h>
#include <docsets.h>
#include <dot.h>
#include <doxy_setup.h>
#include <doxy_globals.h>
#include <eclipsehelp.h>
#include <entry.h>
#include <filestorage.h>
#include <formula.h>
#include <ftvhelp.h>
#include <groupdef.h>
#include <htags.h>
#include <htmlgen.h>
#include <htmlhelp.h>
#include <index.h>
#include <language.h>
#include <latexgen.h>
#include <layout.h>
#include <logos.h>
#include <mangen.h>
#include <msc.h>
#include <objcache.h>
#include <outputlist.h>
#include <parse_base.h>
#include <parse_clang.h>
#include <parse_cstyle.h>
#include <parse_file.h>
#include <parse_fortran.h>
#include <parse_make.h>
#include <parse_md.h>
#include <parse_py.h>
#include <parse_tcl.h>
#include <perlmodgen.h>
#include <portable.h>
#include <pre.h>
#include <qhp.h>
#include <rtfgen.h>
#include <store.h>
#include <tagreader.h>
#include <util.h>
#include <xmlgen.h>

#define RECURSE_ENTRYTREE(func, var) \
   do {  \
      for (auto item : var->children() ) { \
         func(item);   \
      }  \
   } while(0)


#if ! defined(_WIN32) || defined(__CYGWIN__)
#include <signal.h>
#define HAS_SIGNALS
#endif

enum FindBaseClassRelation_Mode {
   TemplateInstances,
   DocumentedOnly,
   Undocumented
};

/** A struct contained the data for an STL class */
struct STLInfo {
   const char *className;
   const char *baseClass1;
   const char *baseClass2;
   const char *templType1;
   const char *templName1;
   const char *templType2;
   const char *templName2;

   bool virtualInheritance;
   bool iterators;
};

static STLInfo g_stlinfo[] = {
   // className              baseClass1                      baseClass2             templType1     templName1     templType2, templName2, virtInheritance, iterators

   { "allocator",            0,                              0,                     "T",           "elements",    0,     0,             false,   false },
   { "array",                0,                              0,                     "T",           "elements",    0,     0,             false,   false },
   { "auto_ptr",             0,                              0,                     "T",           "ptr",         0,     0,             false,   false },
   { "smart_ptr",            0,                              0,                     "T",           "ptr",         0,     0,             false,   false },
   { "unique_ptr",           0,                              0,                     "T",           "ptr",         0,     0,             false,   false },
   { "weak_ptr",             0,                              0,                     "T",           "ptr",         0,     0,             false,   false },
   { "ios_base",             0,                              0,                     0,             0,             0,     0,             false,   false },
   { "error_code",           0,                              0,                     0,             0,             0,     0,             false,   false },
   { "error_category",       0,                              0,                     0,             0,             0,     0,             false,   false },
   { "system_error",         0,                              0,                     0,             0,             0,     0,             false,   false },
   { "error_condition",      0,                              0,                     0,             0,             0,     0,             false,   false },
   { "thread",               0,                              0,                     0,             0,             0,     0,             false,   false },
   { "basic_ios",            "ios_base",                     0,                     "Char",        0,             0,     0,             false,   false },
   { "basic_istream",        "basic_ios<Char>",              0,                     "Char",        0,             0,     0,             true,    false },
   { "basic_ostream",        "basic_ios<Char>",              0,                     "Char",        0,             0,     0,             true,    false },
   { "basic_iostream",       "basic_istream<Char>",          "basic_ostream<Char>", "Char",        0,             0,     0,             false,   false },
   { "basic_ifstream",       "basic_istream<Char>",          0,                     "Char",        0,             0,     0,             false,   false },
   { "basic_ofstream",       "basic_ostream<Char>",          0,                     "Char",        0,             0,     0,             false,   false },
   { "basic_fstream",        "basic_iostream<Char>",         0,                     "Char",        0,             0,     0,             false,   false },
   { "basic_istringstream",  "basic_istream<Char>",          0,                     "Char",        0,             0,     0,             false,   false },
   { "basic_ostringstream",  "basic_ostream<Char>",          0,                     "Char",        0,             0,     0,             false,   false },
   { "basic_stringstream",   "basic_iostream<Char>",         0,                     "Char",        0,             0,     0,             false,   false },
   { "ios",                  "basic_ios<char>",              0,                     0,             0,             0,     0,             false,   false },
   { "wios",                 "basic_ios<wchar_t>",           0,                     0,             0,             0,     0,             false,   false },
   { "istream",              "basic_istream<char>",          0,                     0,             0,             0,     0,             false,   false },
   { "wistream",             "basic_istream<wchar_t>",       0,                     0,             0,             0,     0,             false,   false },
   { "ostream",              "basic_ostream<char>",          0,                     0,             0,             0,     0,             false,   false },
   { "wostream",             "basic_ostream<wchar_t>",       0,                     0,             0,             0,     0,             false,   false },
   { "ifstream",             "basic_ifstream<char>",         0,                     0,             0,             0,     0,             false,   false },
   { "wifstream",            "basic_ifstream<wchar_t>",      0,                     0,             0,             0,     0,             false,   false },
   { "ofstream",             "basic_ofstream<char>",         0,                     0,             0,             0,     0,             false,   false },
   { "wofstream",            "basic_ofstream<wchar_t>",      0,                     0,             0,             0,     0,             false,   false },
   { "fstream",              "basic_fstream<char>",          0,                     0,             0,             0,     0,             false,   false },
   { "wfstream",             "basic_fstream<wchar_t>",       0,                     0,             0,             0,     0,             false,   false },
   { "istringstream",        "basic_istringstream<char>",    0,                     0,             0,             0,     0,             false,   false },
   { "wistringstream",       "basic_istringstream<wchar_t>", 0,                     0,             0,             0,     0,             false,   false },
   { "ostringstream",        "basic_ostringstream<char>",    0,                     0,             0,             0,     0,             false,   false },
   { "wostringstream",       "basic_ostringstream<wchar_t>", 0,                     0,             0,             0,     0,             false,   false },
   { "stringstream",         "basic_stringstream<char>",     0,                     0,             0,             0,     0,             false,   false },
   { "wstringstream",        "basic_stringstream<wchar_t>",  0,                     0,             0,             0,     0,             false,   false },
   { "basic_string",         0,                              0,                     "Char",        0,             0,     0,             false,   true  },
   { "string",               "basic_string<char>",           0,                     0,             0,             0,     0,             false,   true  },
   { "wstring",              "basic_string<wchar_t>",        0,                     0,             0,             0,     0,             false,   true  },
   { "complex",              0,                              0,                     0,             0,             0,     0,             false,   false },
   { "bitset",               0,                              0,                     "Bits",        0,             0,     0,             false,   false },
   { "deque",                0,                              0,                     "T",           "elements",    0,     0,             false,   true  },
   { "list",                 0,                              0,                     "T",           "elements",    0,     0,             false,   true  },
   { "forward_list",         0,                              0,                     "T",           "elements",    0,     0,             false,   true  },
   { "map",                  0,                              0,                     "K",           "keys",        "T",   "elements",    false,   true  },
   { "unordered_map",        0,                              0,                     "K",           "keys",        "T",   "elements",    false,   true  },
   { "multimap",             0,                              0,                     "K",           "keys",        "T",   "elements",    false,   true  },
   { "unordered_multimap",   0,                              0,                     "K",           "keys",        "T",   "elements",    false,   true  },
   { "set",                  0,                              0,                     "K",           "keys",        0,     0,             false,   true  },
   { "unordered_set",        0,                              0,                     "K",           "keys",        0,     0,             false,   true  },
   { "multiset",             0,                              0,                     "K",           "keys",        0,     0,             false,   true  },
   { "unordered_multiset",   0,                              0,                     "K",           "keys",        0,     0,             false,   true  },
   { "vector",               0,                              0,                     "T",           "elements",    0,     0,             false,   true  },
   { "queue",                0,                              0,                     "T",           "elements",    0,     0,             false,   false },
   { "priority_queue",       0,                              0,                     "T",           "elements",    0,     0,             false,   false },
   { "stack",                0,                              0,                     "T",           "elements",    0,     0,             false,   false },
   { "valarray",             0,                              0,                     "T",           "elements",    0,     0,             false,   false },
   { "exception",            0,                              0,                     0,             0,             0,     0,             false,   false },
   { "bad_alloc",            "exception",                    0,                     0,             0,             0,     0,             false,   false },
   { "bad_cast",             "exception",                    0,                     0,             0,             0,     0,             false,   false },
   { "bad_typeid",           "exception",                    0,                     0,             0,             0,     0,             false,   false },
   { "logic_error",          "exception",                    0,                     0,             0,             0,     0,             false,   false },
   { "ios_base::failure",    "exception",                    0,                     0,             0,             0,     0,             false,   false },
   { "runtime_error",        "exception",                    0,                     0,             0,             0,     0,             false,   false },
   { "bad_exception",        "exception",                    0,                     0,             0,             0,     0,             false,   false },
   { "domain_error",         "logic_error",                  0,                     0,             0,             0,     0,             false,   false },
   { "invalid_argument",     "logic_error",                  0,                     0,             0,             0,     0,             false,   false },
   { "length_error",         "logic_error",                  0,                     0,             0,             0,     0,             false,   false },
   { "out_of_range",         "logic_error",                  0,                     0,             0,             0,     0,             false,   false },
   { "range_error",          "runtime_error",                0,                     0,             0,             0,     0,             false,   false },
   { "overflow_error",       "runtime_error",                0,                     0,             0,             0,     0,             false,   false },
   { "underflow_error",      "runtime_error",                0,                     0,             0,             0,     0,             false,   false },
   { 0,                      0,                              0,                     0,             0,             0,     0,             false,   false }
};

namespace Doxy_Work{

   void addClassToContext(QSharedPointer<EntryNav> rootNav);
   void addEnumValuesToEnums(QSharedPointer<EntryNav> rootNav);

   void addIncludeFile(QSharedPointer<ClassDef> cd, QSharedPointer<FileDef> ifd, QSharedPointer<Entry> root);
   void addInterfaceOrServiceToServiceOrSingleton(QSharedPointer<EntryNav> rootNav, QSharedPointer<ClassDef> cd, 
                  QString const &rname);

   void addMethodToClass(QSharedPointer<EntryNav> rootNav, QSharedPointer<ClassDef> cd, const QString &rname, bool isFriend);

   void addMembersToIndex();
   void addMembersToMemberGroup();

   void addMemberDocs(QSharedPointer<EntryNav> rootNav, QSharedPointer<MemberDef> md, const QString &funcDecl,
                  ArgumentList *al, bool overload, NamespaceSDict *nl = 0);
  
   void addPageToContext(QSharedPointer<PageDef> pd, QSharedPointer<EntryNav> rootNav);
   void addListReferences();
   void addRelatedPage(QSharedPointer<EntryNav> rootNav);
   void addSourceReferences();
   void addSTLClasses(QSharedPointer<EntryNav> rootNav);
   void addSTLIterator(QSharedPointer<EntryNav> classEntryNav, const QString &name);
   void addSTLMember(QSharedPointer<EntryNav> rootNav, const QString &type, const QString &name);

   void addVariable(QSharedPointer<EntryNav> rootNav, int isFuncPtr = -1);

   QSharedPointer<MemberDef> addVariableToClass(QSharedPointer<EntryNav> rootNav, QSharedPointer<ClassDef> cd, MemberType mtype, const QString &name,
                  bool fromAnnScope, QSharedPointer<MemberDef> fromAnnMemb, Protection prot, Relationship related);

   QSharedPointer<MemberDef> addVariableToFile(QSharedPointer<EntryNav> rootNav, MemberType mtype, const QString &scope, const QString &name,
                  bool fromAnnScope, QSharedPointer<MemberDef> fromAnnMemb);

   void buildClassDocList(QSharedPointer<EntryNav> rootNav);
   void buildClassList(QSharedPointer<EntryNav> rootNav);
   void buildCompleteMemberLists();
   void buildFileList(QSharedPointer<EntryNav> rootNav);
   void buildExampleList(QSharedPointer<EntryNav> rootNav);
   void buildFunctionList(QSharedPointer<EntryNav> rootNav);
   void buildGroupList(QSharedPointer<EntryNav> rootNav);
   void buildGroupListFiltered(QSharedPointer<EntryNav> rootNav, bool additional, bool includeExternal);
   void buildInterfaceAndServiceList(QSharedPointer<EntryNav> rootNav);
   void buildListOfUsingDecls(QSharedPointer<EntryNav> rootNav);
   void buildNamespaceList(QSharedPointer<EntryNav> rootNav);
   void buildPageList(QSharedPointer<EntryNav> rootNav);

   QSharedPointer<Definition> buildScopeFromQualifiedName(const QString name, int level, SrcLangExt lang, TagInfo *tagInfo);

   void buildTypedefList(QSharedPointer<EntryNav> rootNav);
   void buildVarList(QSharedPointer<EntryNav> rootNav);

   void checkPageRelations();

   void combineUsingRelations();
   void computeClassRelations();
   int computeIdealCacheParam(uint v);
   void computeMemberReferences();
   void computeMemberRelations();
   void computePageRelations(QSharedPointer<EntryNav> rootNav);
   void computeTemplateClassRelations();

   ClassDef::CompoundType convertToCompoundType(int section, Entry::Traits data);

   void copyExtraFiles(const QString &outputType);
   void copyLogo(const QString &outputType);
   void copyStyleSheet();
   void copyLatexStyleSheet();

   QString createOutputDirectory(const QString &baseDirName, const QString &formatDirOption, const QString &defaultDirName);
   void createTemplateInstanceMembers();
   QSharedPointer<ClassDef> createTagLessInstance(QSharedPointer<ClassDef> rootCd, QSharedPointer<ClassDef> templ, const QString &fieldName);

   void distributeMemberGroupDocumentation();
   void dumpPhrase(QTextStream &t, QSharedPointer<Definition> d);
   void dumpGlossary();

   QString extractClassName(QSharedPointer<EntryNav> rootNav);
   
   void filterMemberDocumentation(QSharedPointer<EntryNav> rootNav);

   void findBaseClassesForClass(QSharedPointer<EntryNav> rootNav, QSharedPointer<Definition> context, QSharedPointer<ClassDef> masterCd, 
                                QSharedPointer<ClassDef> instanceCd, FindBaseClassRelation_Mode mode, bool isArtificial, 
                                ArgumentList *actualArgs = 0, QHash<QString, int> *templateNames = 0);

   QSharedPointer<ClassDef> findClassDefinition(QSharedPointer<FileDef> fd, QSharedPointer<NamespaceDef> nd, const QString &scopeName);

   void findClassEntries(QSharedPointer<EntryNav> rootNav);
   bool findClassRelation(QSharedPointer<EntryNav> rootNav, QSharedPointer<Definition> context, QSharedPointer<ClassDef> cd, BaseInfo *bi,
                          QHash<QString, int> *templateNames, FindBaseClassRelation_Mode mode, bool isArtificial);

   QSharedPointer<ClassDef> findClassWithinClassContext(QSharedPointer<Definition> context, QSharedPointer<ClassDef> cd, const QString &name);

   void findDefineDocumentation(QSharedPointer<EntryNav> rootNav);
   void findDEV(const MemberNameSDict &mnsd);
   void findDirDocumentation(QSharedPointer<EntryNav> rootNav);
   void findDocumentedEnumValues();

   int findEndOfTemplate(const QString &s, int startPos);
   void findEnums(QSharedPointer<EntryNav> rootNav);
   void findEnumDocumentation(QSharedPointer<EntryNav> rootNav);

   void findFriends();
   int findFunctionPtr(const QString &type, int lang, int *pLength = 0);

   bool findGlobalMember(QSharedPointer<EntryNav> rootNav, const QString &namespaceName, const QString &type, const QString &name,
                                const QString &tempArg, const QString &, const QString &decl);

   void findGroupScope(QSharedPointer<EntryNav> rootNav);
   void findInheritedTemplateInstances();

   void findMainPage(QSharedPointer<EntryNav> rootNav);
   void findMainPageTagFiles(QSharedPointer<EntryNav> rootNav);
   void findMember(QSharedPointer<EntryNav> rootNav, QString funcDecl, bool overloaded, bool isFunc);
   void findMemberDocumentation(QSharedPointer<EntryNav> rootNav);

   void findObjCMethodDefinitions(QSharedPointer<EntryNav> rootNav);

   QSharedPointer<Definition> findScopeFromQualifiedName(QSharedPointer<Definition> startScope, const QString &n,
                                                         QSharedPointer<FileDef> fileScope, TagInfo *tagInfo);

   void findSectionsInDocumentation();

   void findTagLessClasses();
   void findTagLessClasses(QSharedPointer<ClassDef> cd);

   bool findTemplateInstanceRelation(QSharedPointer<Entry> root, QSharedPointer<Definition> context, QSharedPointer<ClassDef> templateClass,
                                     const QString &templSpec, QHash<QString, int> *templateNames, bool isArtificial);

   QSharedPointer<NamespaceDef> findUsedNamespace(NamespaceSDict *unl, const QString &name);

   void findUsedClassesForClass(QSharedPointer<EntryNav> rootNav, QSharedPointer<Definition> context, QSharedPointer<ClassDef> masterCd, 
            QSharedPointer<ClassDef> instanceCd, bool isArtificial, ArgumentList *actualArgs = 0, 
            QHash<QString, int> *templateNames = 0);

   void findUsedTemplateInstances();

   void findUsingDeclarations(QSharedPointer<EntryNav> rootNav);
   void findIncludedUsingDirectives();
   void findUsingDirectives(QSharedPointer<EntryNav> rootNav);
   void findUsingDeclImports(QSharedPointer<EntryNav> rootNav);

   void flushUnresolvedRelations();
   void flushCachedTemplateRelations();

   void generateClassList(ClassSDict &classSDict);
   void generateClassDocs();
   void generateExampleDocs();
   void generateFileDocs();
   void generateSourceCode();
   void generateGroupDocs();
   void generateNamespaceDocs();
   void generatePageDocs();
   void generateXRefPages();
  
   QString getQchFileName();
   QHash<QString, int> *getTemplateArgumentsInName(ArgumentList *templateArguments, const QString &name);

   void inheritDocumentation();
   bool isClassSection(QSharedPointer<EntryNav> rootNav);
   bool isRecursiveBaseClass(const QString &scope, const QString &name);
   bool isSpecialization(const QList<ArgumentList> &srcTempArgLists, const QList<ArgumentList> &dstTempArgLists);
   bool isVarWithConstructor(QSharedPointer<EntryNav> rootNav);

   void mergeCategories();

   void organizeSubGroupsFiltered(QSharedPointer<EntryNav> rootNav, bool additional);
   void organizeSubGroups(QSharedPointer<EntryNav> rootNav);

   void parseFile(ParserInterface *parser, QSharedPointer<Entry> root, QSharedPointer<EntryNav> rootNav, 
                  QSharedPointer<FileDef> fd, QString fileName, enum ParserMode mode, QStringList &filesInSameTu);

   void parseFiles(QSharedPointer<Entry> root, QSharedPointer<EntryNav> rootNav);

   void processFiles();
   void processTagLessClasses(QSharedPointer<ClassDef> rootCd, QSharedPointer<ClassDef> cd, QSharedPointer<ClassDef>tagParentCd, 
                  const QString &prefix, int count);

struct ReadDirArgs {
   bool recursive = false;
   bool errorIfNotExist = true; 

   QStringList includePatternList; 
   QStringList excludePatternList; 

   QSet<QString> excludeSet;
   
   bool isFnList = false;
   SortedList<QSharedPointer<FileNameList>> fnList;

   bool isFnDict = false;
   FileNameDict fnDict;     

   bool isResultList = false;
   QStringList resultList;

   bool isPrepExclude = false;
   QSet<QString> prepExcludeSet; 

   bool isKillDict = false;
   QSet<QString> killDict;

   bool isPathSet = false;
   QSet<QString> pathSet;
};

   void readFileOrDirectory(const QString &fileName, ReadDirArgs &data);
   void readDir(const QFileInfo &fileInfo, ReadDirArgs &data);  
           
   void resolveClassNestingRelations();
   void resolveHiddenNamespace();
   QString resolveSymlink(QString path);
   void resolveUserReferences();

   void readTagFile(QSharedPointer<Entry> root, const QString &tl);
   
   bool scopeIsTemplate(QSharedPointer<Definition> d);
   void substituteTemplatesInArgList(const QList<ArgumentList> &srcTempArgLists, const QList<ArgumentList> &dstTempArgLists,
               ArgumentList *src, ArgumentList *dst, ArgumentList *funcTempArgs = 0);

   QString substituteTemplatesInString( const QList<ArgumentList> &srcTempArgLists, const QList<ArgumentList> &dstTempArgLists,
      ArgumentList *funcTempArgList, const QString &src);

   void transferFunctionDocumentation();
   void transferFunctionReferences();
   void transferRelatedFunctionDocumentation();

   void writeTagFile();
}

using namespace Doxy_Work;

// ** next two functions are the two core functions

void processFiles()
{
   printf("Parse input files\n");   

   // make sure the output directory exists
   QString outputDirectory = Config::getString("output-dir");
  
   // initialize global lists and dictionaries
   Doxy_Globals::symbolStorage = new Store;
   
   int cacheSize = Config::getInt("lookup-cache-size");

   if (cacheSize < 0) {
      cacheSize = 0;
   }

   if (cacheSize > 9) {
      cacheSize = 9;
   }

   uint lookupSize = 65536 << cacheSize;

   Doxy_Globals::lookupCache = new QCache<QString, LookupInfo>(lookupSize);

#ifdef HAS_SIGNALS
   signal(SIGINT, stopDoxyPress);
#endif

   uint pid = portable_pid();

   Doxy_Globals::tempA_FName = QString("doxy_tempA_%1.tmp").arg(pid);
   Doxy_Globals::tempA_FName.prepend(outputDirectory + "/");

   Doxy_Globals::tempB_FName = QString("doxy_tempB_%1.tmp").arg(pid);
   Doxy_Globals::tempB_FName.prepend(outputDirectory + "/");

   if (Doxy_Globals::symbolStorage->open(Doxy_Globals::tempA_FName) == -1) {
      err("Unable to open temporary file %s\n", csPrintable(Doxy_Globals::tempA_FName));
      stopDoxyPress();
   }

   // Check/create output directorties
   QString htmlOutput;
   const bool generateHtml = Config::getBool("generate-html");

   if (generateHtml) {
      htmlOutput = createOutputDirectory(outputDirectory, "html-output", "/html");
   }

   QString docbookOutput;
   const bool generateDocbook = Config::getBool("generate-docbook");

   if (generateDocbook) {
      docbookOutput = createOutputDirectory(outputDirectory, "docbook-output", "/docbook");
   }

   QString xmlOutput;
   const bool generateXml = Config::getBool("generate-xml");

   if (generateXml) {
      xmlOutput = createOutputDirectory(outputDirectory, "xml-output", "/xml");
   }

   QString latexOutput;
   const bool generateLatex = Config::getBool("generate-latex");

   if (generateLatex) {
      latexOutput = createOutputDirectory(outputDirectory, "latex-output", "/latex");
   }

   QString rtfOutput;
   const bool generateRtf = Config::getBool("generate-rtf");

   if (generateRtf) {
      rtfOutput = createOutputDirectory(outputDirectory, "rtf-output", "/rtf");
   }

   QString manOutput;
   const bool generateMan = Config::getBool("generate-man");

   if (generateMan) {
      manOutput = createOutputDirectory(outputDirectory, "man-output", "/man");
   }

   if (Config::getBool("have-dot")) {
      QString curFontPath = Config::getString("dot-font-path");

      if (curFontPath.isEmpty()) {
         portable_getenv("dot-font-path");
         QString  newFontPath = ".";

         if (! curFontPath.isEmpty()) {
            newFontPath += portable_pathListSeparator();
            newFontPath += curFontPath;
         }

         portable_setenv("dot-font-path", newFontPath);

      } else {
         portable_setenv("dot-font-path", curFontPath);

      }
   }
  
   // handle layout file
   LayoutDocManager::instance().init();
 
   QString layoutFileName = Config::getString("layout-file");
 
   QFile layoutFile(layoutFileName);

   if (layoutFile.open(QIODevice::ReadOnly)) {
      msg("Parsing layout file %s\n", csPrintable(layoutFileName));

      QTextStream t(&layoutFile);
      t.setCodec("UTF-8");

      LayoutDocManager::instance().parse(t, layoutFileName);

   } else if (layoutFileName != "doxy_layout.xml")  {
      warn_uncond("Unable to open layout file '%s' for reading\n", csPrintable(layoutFileName));

   }
   
   // prevent search in the output directories
   QStringList exclPatterns = Config::getList("exclude-patterns");

   if (generateHtml) {
      exclPatterns.append(htmlOutput);
   }
   if (generateDocbook) {
      exclPatterns.append(docbookOutput);
   }
   if (generateXml) {
      exclPatterns.append(xmlOutput);
   }
   if (generateLatex) {
      exclPatterns.append(latexOutput);
   }
   if (generateRtf) {
      exclPatterns.append(rtfOutput);
   }
   if (generateMan) {
      exclPatterns.append(manOutput);
   }

   // update project data
   Config::setList("exclude-patterns", exclPatterns);

   searchInputFiles();

   // **  Note: the order of the function calls below are important

   if (Config::getBool("generate-html")) {
      readFormulaRepository();
   }

   // Handle Tag Files
   Doxy_Globals::g_storage = new FileStorage;
   Doxy_Globals::g_storage->setName(Doxy_Globals::tempB_FName);

   if (! Doxy_Globals::g_storage->open(QIODevice::WriteOnly)) {
      err("Unable to create temporary file %s\n", csPrintable(Doxy_Globals::tempB_FName));
      stopDoxyPress();
   }

   QSharedPointer<Entry> root = QMakeShared<Entry>();
   QSharedPointer<EntryNav> rootNav = QMakeShared<EntryNav>(QSharedPointer<EntryNav>(), root);
   rootNav->setEntry(root);
  
   msg("Parsing tag files\n");

   const QStringList tagFileList = Config::getList("tag-files");

   for (auto s : tagFileList) {
      readTagFile(root, csPrintable(s));
      root->createNavigationIndex(rootNav, Doxy_Globals::g_storage, QSharedPointer<FileDef>(), root);
   }

   // parse source files
   if (Config::getBool("built-in-stl-support")) {
      addSTLClasses(rootNav);
   }

   msg("\n**  ");
   Doxy_Globals::g_stats.begin("Parsing files\n");
   parseFiles(root, rootNav);

   Doxy_Globals::g_storage->close();
   Doxy_Globals::g_stats.end();

   // done with input scanning, free up the buffers used by lex (can be around 4MB)
   preFreeScanner();
   cstyleFreeParser();
   pyFreeParser();

   if (! Doxy_Globals::g_storage->open(QIODevice::ReadOnly)) {
      err("Unable to open temporary file %s\n", csPrintable(Doxy_Globals::tempB_FName));
      stopDoxyPress();
   }

   // gather information
   Doxy_Globals::g_stats.begin("Building group list\n");
   buildGroupList(rootNav);
   organizeSubGroups(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Building directory list\n");   
   buildDirectories();
   findDirDocumentation(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Building namespace list\n");
   buildNamespaceList(rootNav);
   findUsingDirectives(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Building file list\n");
   buildFileList(rootNav);
   Doxy_Globals::g_stats.end();

   // generateFileTree()   
  
   Doxy_Globals::g_stats.begin("Building class list\n");
   buildClassList(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Associating documentation with classes\n");
   buildClassDocList(rootNav);

   // build list of using declarations here (global list)
   buildListOfUsingDecls(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Computing nesting relations for classes\n");
   resolveClassNestingRelations();
   Doxy_Globals::g_stats.end();

   // no longer add nested classes to the group as well
   // distributeClassGroupRelations();

   // calling buildClassList may result in cached relations which become invalid
   // after resolveClassNestingRelations(), that is why we clear the cache here
   Doxy_Globals::lookupCache->clear();

   // we do not need the list of using declaration anymore
   Doxy_Globals::g_usingDeclarations.clear();

   Doxy_Globals::g_stats.begin("Building example list\n");
   buildExampleList(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Searching for enumerations\n");
   findEnums(rootNav);
   Doxy_Globals::g_stats.end();

   // Since buildVarList calls isVarWithConstructor and this calls getResolvedClass we need to process typedefs
   // first so the relations between classes via typedefs are properly resolved. See bug 536385 for an example.

   Doxy_Globals::g_stats.begin("Searching for documented typedefs\n");
   buildTypedefList(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Searching for members imported via using declarations\n");
   findUsingDeclImports(rootNav);

   // this should be after buildTypedefList in order to properly import used typedefs
   findUsingDeclarations(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Searching for included using directives\n");
   findIncludedUsingDirectives();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Searching for documented variables\n");
   buildVarList(rootNav);
   Doxy_Globals::g_stats.end();
  
   // UNO IDL
   Doxy_Globals::g_stats.begin("Building interface member list\n");
   buildInterfaceAndServiceList(rootNav);                  

   // using class info only
   Doxy_Globals::g_stats.begin("Building member list\n");   
   buildFunctionList(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Searching for friends\n");
   findFriends();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Searching for documented defines\n");
   findDefineDocumentation(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Computing class inheritance relations\n");
   findClassEntries(rootNav);
   findInheritedTemplateInstances();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Computing class usage relations\n");
   findUsedTemplateInstances();
   Doxy_Globals::g_stats.end();

   if (Config::getBool("inline-simple-struct")) {
      Doxy_Globals::g_stats.begin("Searching for tag less structs\n");
      findTagLessClasses();
      Doxy_Globals::g_stats.end();
   }

   Doxy_Globals::g_stats.begin("Flushing cached template relations\n");
   flushCachedTemplateRelations();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Creating members for template instances\n");
   createTemplateInstanceMembers();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Computing class relations\n");
   computeTemplateClassRelations();
   flushUnresolvedRelations();

   computeClassRelations();
   Doxy_Globals::g_classEntries.clear();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Add enum values to enums\n");
   addEnumValuesToEnums(rootNav);
   findEnumDocumentation(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Searching for member function documentation\n");
   findObjCMethodDefinitions(rootNav);
   findMemberDocumentation(rootNav);       // may introduce new members

   transferRelatedFunctionDocumentation();
   transferFunctionDocumentation();
   Doxy_Globals::g_stats.end();

   // added 12/2015
   Doxy_Globals::g_stats.begin("Resolve empty Namespaces\n");
   resolveHiddenNamespace();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Building page list\n");
   buildPageList(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Search for main page\n");
   findMainPage(rootNav);
   findMainPageTagFiles(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Computing page relations\n");
   computePageRelations(rootNav);
   checkPageRelations();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Determining the scope of groups\n");
   findGroupScope(rootNav);
   Doxy_Globals::g_stats.end();

   msg("Freeing entry tree\n");
   rootNav = QSharedPointer<EntryNav>();

   Doxy_Globals::g_storage->close();
   delete Doxy_Globals::g_storage;
   Doxy_Globals::g_storage = 0;

   //
   QDir thisDir;
   if (! Doxy_Globals::tempB_FName.isEmpty()) {
      thisDir.remove(Doxy_Globals::tempB_FName);
   }

   Doxy_Globals::g_stats.begin("Determining which enums are documented\n");
   findDocumentedEnumValues();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Computing member relations\n");
   mergeCategories();
   computeMemberRelations();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Building full member lists recursively\n");
   buildCompleteMemberLists();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Adding members to member groups\n");
   addMembersToMemberGroup();
   Doxy_Globals::g_stats.end();

   if (Config::getBool("duplicate-docs")) {
      Doxy_Globals::g_stats.begin("Duplicating member group documentation\n");
      distributeMemberGroupDocumentation();
      Doxy_Globals::g_stats.end();
   }

   Doxy_Globals::g_stats.begin("Computing member references\n");
   computeMemberReferences();
   Doxy_Globals::g_stats.end();

   if (Config::getBool("inherit-docs")) {
      Doxy_Globals::g_stats.begin("Inheriting documentation\n");
      inheritDocumentation();
      Doxy_Globals::g_stats.end();
   }

   // compute the shortest possible names of all files
   // without losing the uniqueness of the file names.

   Doxy_Globals::g_stats.begin("Generating disk names\n");

   for (auto item : *Doxy_Globals::inputNameList) {
      item->generateDiskNames();
   }

   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Adding source references\n");
   addSourceReferences();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Adding xrefitems\n");
   addListReferences();
   generateXRefPages();
   Doxy_Globals::g_stats.end();
 
   if (Config::getBool("directory-graph")) {
      Doxy_Globals::g_stats.begin("Computing dependencies between directories\n");
      computeDirDependencies();
      Doxy_Globals::g_stats.end();
   }

   // Doxy_Globals::g_stats.begin("Resolving citations\n");
   // Doxy_Globals::citeDict->resolve();

   Doxy_Globals::g_stats.begin("Generating citations page\n");
   Doxy_Globals::citeDict->generatePage();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Counting data structures\n");
   countDataStructures();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Resolving user defined references\n");
   resolveUserReferences();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Finding anchors and sections in the documentation\n");
   findSectionsInDocumentation();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Transferring function references\n");
   transferFunctionReferences();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Combining using relations\n");
   combineUsingRelations();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Adding members to index pages\n");
   addMembersToIndex();
   Doxy_Globals::g_stats.end();
}

void generateOutput()
{
   // Initialize output generators  
   printf("\n**  Generate Documentation Output\n");   
  
   if (Doxy_Globals::dumpGlossary) {
      dumpGlossary();
      exit(0);
   }

   // move to the output directory 
   QString outputDir = Config::getString("output-dir");   
   QDir::setCurrent(outputDir);

   initSearchIndexer();

   bool generateHtml  = Config::getBool("generate-html");
   bool generateLatex = Config::getBool("generate-latex");
   bool generateMan   = Config::getBool("generate-man");
   bool generateRtf   = Config::getBool("generate-rtf");

   static const QString htmlOutput  = Config::getString("html-output");
   static const QString latexOutput = Config::getString("latex-output");
  
   Doxy_Globals::g_outputList = new OutputList(true);

   if (generateHtml) {
      Doxy_Globals::g_outputList->add(new HtmlGenerator);
      HtmlGenerator::init();

      // add HTML indexers that are enabled
      bool generateHtmlHelp     = Config::getBool("generate-chm");
      bool generateEclipseHelp  = Config::getBool("generate-eclipse");
      bool generateQhp          = Config::getBool("generate-qthelp");
      bool generateTreeView     = Config::getBool("generate-treeview");
      bool generateDocSet       = Config::getBool("generate-docset");    

      if (generateEclipseHelp) {
         Doxy_Globals::indexList->addIndex(QSharedPointer<EclipseHelp>(new EclipseHelp));
      }

      if (generateHtmlHelp) {
         Doxy_Globals::indexList->addIndex(QSharedPointer<HtmlHelp>(new HtmlHelp));
      }

      if (generateQhp) {
         Doxy_Globals::indexList->addIndex(QSharedPointer<Qhp>(new Qhp));
      }

      if (generateTreeView) {
         Doxy_Globals::indexList->addIndex(QSharedPointer<FTVHelp>(new FTVHelp(true)));
      }

      if (generateDocSet) {
         Doxy_Globals::indexList->addIndex(QSharedPointer<DocSets>(new DocSets));
      }

      Doxy_Globals::indexList->initialize();
      HtmlGenerator::writeTabData();      
   }

   if (generateLatex) {    
      Doxy_Globals::g_outputList->add(new LatexGenerator);
      LatexGenerator::init(); 
   }

   if (generateMan) {
      Doxy_Globals::g_outputList->add(new ManGenerator);
      ManGenerator::init();
   }

   if (generateRtf) {
      static QString rtfOutput = Config::getString("rtf-output");

      Doxy_Globals::g_outputList->add(new RTFGenerator);
      RTFGenerator::init();

      copyLogo(rtfOutput);
   }

   if (Config::getBool("use-htags")) {
      Htags::useHtags = true;      

      if (! Htags::execute(htmlOutput)) {
         err("'USE HTAGS' is set, however htags(1) failed\n");
      }
      if (! Htags::loadFilemap(htmlOutput)) {
         err("htags(1) ended normally but failed to load the file map\n");
      }
   }
 
   // ** generate documentation
   if (generateHtml) {
      writeDoxFont(htmlOutput);
   }

   if (generateLatex) {
      writeDoxFont(Config::getString("latex-output"));
   }

   if (generateRtf) {
      writeDoxFont(Config::getString("rtf-output"));
   }

   Doxy_Globals::g_stats.begin("Generating style sheet\n");
  
   // write first part
   Doxy_Globals::g_outputList->writeStyleInfo(0); 
   Doxy_Globals::g_stats.end();

   static bool searchEngine      = Config::getBool("html-search");
   static bool serverBasedSearch = Config::getBool("search-server-based");

   // generate search indices (need to do this before writing other HTML
   // pages as these contain a drop down menu with options depending on
   // what categories we find in this function.

   if (generateHtml && searchEngine) {
      Doxy_Globals::g_stats.begin("Generating search index\n");
      QString searchDirName = htmlOutput + "/search";

      QDir searchDir(searchDirName);

      if (! searchDir.exists() && ! QDir::current().mkpath(searchDirName)) {
         err("Unable to create search directory '%s'\n", csPrintable(searchDirName));   
         stopDoxyPress();
      }

      HtmlGenerator::writeSearchData(searchDirName);

      if (! serverBasedSearch) { 
         // client side search index
         writeJavascriptSearchIndex();
      }

      Doxy_Globals::g_stats.end();
   }

   Doxy_Globals::g_stats.begin("Generating example documentation\n");
   generateExampleDocs();
   Doxy_Globals::g_stats.end();

   if (! Htags::useHtags) {
      Doxy_Globals::g_stats.begin("Generating source code\n");
      generateSourceCode();
      Doxy_Globals::g_stats.end();
   }

   Doxy_Globals::g_stats.begin("Generating file documentation\n");
   generateFileDocs();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Generating page documentation\n");
   generatePageDocs();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Generating group documentation\n");
   generateGroupDocs();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Generating class documentation\n");
   generateClassDocs();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Generating namespace documentation\n");
   generateNamespaceDocs();
   Doxy_Globals::g_stats.end();

   if (Config::getBool("generate-legend")) {
      Doxy_Globals::g_stats.begin("Generating graph documentation\n");
      writeGraphInfo(*Doxy_Globals::g_outputList);
      Doxy_Globals::g_stats.end();
   }

   Doxy_Globals::g_stats.begin("Generating directory documentation\n");
   generateDirDocs(*Doxy_Globals::g_outputList);
   Doxy_Globals::g_stats.end();

   if (Doxy_Globals::formulaList->count() > 0 && generateHtml && ! Config::getBool("use-mathjax")) {
      Doxy_Globals::g_stats.begin("Generating bitmaps for formulas in HTML\n");
      Doxy_Globals::formulaList->generateBitmaps(htmlOutput);
      Doxy_Globals::g_stats.end();
   }

   if (Config::getBool("sort-group-names")) {      
      // groupSDict -- always sorted  
      // sort the sub groups 
   }

   if (Doxy_Globals::g_outputList->count() > 0) {
      writeIndexHierarchy(*Doxy_Globals::g_outputList);
   }

   Doxy_Globals::g_stats.begin("Finalizing index pages\n");
   Doxy_Globals::indexList->finalize();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Writing tag file\n");
   writeTagFile();
   Doxy_Globals::g_stats.end();

   if (Config::getBool("dot-cleanup")) {
      if (generateHtml) {
         removeDoxFont(htmlOutput);
      }
      if (generateRtf) {
         removeDoxFont(Config::getString("rtf-output"));
      }
      if (generateLatex) {
         removeDoxFont(Config::getString("latex-output"));
      }
   }

   if (Config::getBool("generate-xml")) {
      Doxy_Globals::g_stats.begin("Generating XML output\n");
      Doxy_Globals::generatingXmlOutput = true;
      generateXML();
      Doxy_Globals::generatingXmlOutput = false;
      Doxy_Globals::g_stats.end();
   }

   if (Config::getBool("generate-docbook")) {
      Doxy_Globals::g_stats.begin("Generating Docbook output\n");
      generateDocbook();
      Doxy_Globals::g_stats.end();
   }
 
   if (Config::getBool("generate-perl")) {
      Doxy_Globals::g_stats.begin("Generating Perl module output\n");
      generatePerlMod();
      Doxy_Globals::g_stats.end();
   }

   if (generateHtml && searchEngine && serverBasedSearch) {
      Doxy_Globals::g_stats.begin("Generating search index\n");

      if (Doxy_Globals::searchIndex->kind() == SearchIndexIntf::Internal) { 
         // write own search index

         HtmlGenerator::writeSearchPage();
         Doxy_Globals::searchIndex->write(htmlOutput + "/search/search.idx");

      } else { 
         // write data for external search index
         HtmlGenerator::writeExternalSearchPage();

         QString searchDataFile = Config::getString("search-data-file");

         if (searchDataFile.isEmpty()) {
            searchDataFile = "searchdata.xml";
         }

         if (! QDir::isAbsolutePath(searchDataFile)) {                       
            QString outputDirectory = Config::getString("output-dir");
            searchDataFile.prepend(outputDirectory + "/");
         }

         Doxy_Globals::searchIndex->write(searchDataFile);
      }

      Doxy_Globals::g_stats.end();
   }

   if (generateRtf) {
      Doxy_Globals::g_stats.begin("Combining RTF output\n");
      if (! RTFGenerator::preProcessFileInplace(Config::getString("rtf-output"), "refman.rtf")) {
         err("Error occurred during post processing of RTF files\n");
      }

      Doxy_Globals::g_stats.end();
   }

   if (Config::getBool("have-dot")) {
      Doxy_Globals::g_stats.begin("Running dot\n");
      DotManager::instance()->run();
      Doxy_Globals::g_stats.end();
   }

   // copy static files
   if (generateHtml)     {
      FTVHelp::generateTreeViewImages();

      copyStyleSheet();
      copyLogo(htmlOutput);
      copyExtraFiles("html");     
   }

   if (generateLatex)  {  
      copyLatexStyleSheet();
      copyLogo(latexOutput);
      copyExtraFiles("latex");
   }

   if (generateHtml && Config::getBool("generate-chm") && ! Config::getString("hhc-location").isEmpty()) {

      Doxy_Globals::g_stats.begin("Running html help compiler\n");
      QString oldDir = QDir::currentPath();
      QDir::setCurrent(htmlOutput);

      portable_sysTimerStart();
    
      // this was false, now it is testing the extcmd flag
      bool isDebug = Debug::isFlagSet(Debug::ExtCmd);

      if (portable_system(Config::getString("hhc-location"), "index.hhp", isDebug)) {
         err("Unable to run HTML Help compiler on 'index.hhp'\n");
      }

      portable_sysTimerStop();
      QDir::setCurrent(oldDir);
      Doxy_Globals::g_stats.end();
   }

   if ( generateHtml && Config::getBool("generate-qthelp") && ! Config::getString("qthelp-gen-path").isEmpty()) {
      Doxy_Globals::g_stats.begin("Running QtHelp generator\n");

      QString qhpFileName = Qhp::getQhpFileName();
      QString qchFileName = getQchFileName();

      QString args = QString("%1 -o \"%2\"").arg(qhpFileName).arg(qchFileName);

      QString const oldDir = QDir::currentPath();
      QDir::setCurrent(htmlOutput);

      portable_sysTimerStart();

      if (portable_system(Config::getString("qthhelp-gen-path"), args, false)) {
         err("Unable to run qhelpgenerator on 'index.qhp'\n");
      }

      portable_sysTimerStop();
      QDir::setCurrent(oldDir);
      Doxy_Globals::g_stats.end();
   }

   msg("Lookup cache used %d/%d \n", Doxy_Globals::lookupCache->count(), Doxy_Globals::lookupCache->size());

   if (Debug::isFlagSet(Debug::Time)) {
      Doxy_Globals::g_stats.print();

   } else {
      msg("Finished\n");

   }

   // all done, cleaning up and exit  
   shutDownDoxyPress();
   Doxy_Globals::programExit = true;
}

// ** other
ArgumentList getTemplateArgumentsFromName(const QString &name, const QList<ArgumentList> *tArgLists)
{
   ArgumentList retval = ArgumentList();

   if (! tArgLists) {
      return retval;
   }

   // for each scope fragment, check if it is a template and advance through the list if so.
   int index;
   int p = 0;

   auto item = tArgLists->begin();

   while (true) {
      index = name.indexOf("::", p);

      if (index == -1) { 
         break;
      }
       
      QSharedPointer<NamespaceDef> nd (Doxy_Globals::namespaceSDict->find(name.left(index)));

      if (nd == nullptr) {
         QSharedPointer<ClassDef> cd = getClass(name.left(index));

         if (cd) {
            if (cd->templateArguments()) {
               ++item;

               if (item == tArgLists->end()) {
                  break;
               }
            }
         }
      }

      p = index + 2;
   }

   if (item != tArgLists->end()) {
      retval = *item;                     
   }
 
   return retval;
}


// **  functions
void clearAll()
{
   Doxy_Globals::g_inputFiles.clear();

   Doxy_Globals::classSDict->clear();
   Doxy_Globals::namespaceSDict->clear();
   Doxy_Globals::pageSDict->clear();
   Doxy_Globals::exampleSDict->clear();
   Doxy_Globals::inputNameList->clear();
   Doxy_Globals::formulaList->clear();
   Doxy_Globals::sectionDict->clear();
   Doxy_Globals::inputNameDict->clear();
   Doxy_Globals::includeNameDict->clear();
   Doxy_Globals::exampleNameDict->clear();
   Doxy_Globals::imageNameDict->clear();
   Doxy_Globals::dotFileNameDict->clear();
   Doxy_Globals::mscFileNameDict->clear();
   Doxy_Globals::diaFileNameDict->clear();
   Doxy_Globals::formulaDict->clear();
   Doxy_Globals::formulaNameDict->clear();
   Doxy_Globals::tagDestinationDict.clear();

   delete Doxy_Globals::citeDict;

   Doxy_Globals::mainPage = QSharedPointer<PageDef>();
}

void distributeClassGroupRelations()
{
   // static bool inlineGroupedClasses = Config::getBool("inline-grouped-classes");

   for (auto cd : *Doxy_Globals::classSDict) {
      cd->visited = false;
   }

   for (auto cd : *Doxy_Globals::classSDict) {
      // distribute the group to nested classes as well

      QSharedPointer<GroupDef> gd = cd->partOfGroups()->at(0);

      if (! cd->visited && cd->partOfGroups() != 0 && cd->getClassSDict()) {

         for (auto ncd : *cd->getClassSDict() ) {
            if (ncd->partOfGroups() == 0) {
               ncd->makePartOfGroup(gd);
               gd->addClass(ncd);
            }
         }

         // only visit every class once
         cd->visited = true;
      }
   }
}

// ** start of static methods
void Doxy_Work::addSTLMember(QSharedPointer<EntryNav> rootNav, const QString &type, const QString &name)
{
   QSharedPointer<Entry> memEntry = QMakeShared<Entry>();

   memEntry->name       = name;
   memEntry->type       = type;
   memEntry->protection = Public;
   memEntry->section    = Entry::Sections::VARIABLE_SEC;
   memEntry->brief      = "STL member";
   memEntry->hidden     = false;
   memEntry->artificial = true;
  
   QSharedPointer<EntryNav> memEntryNav = QMakeShared<EntryNav>(rootNav, memEntry);

   memEntryNav->setEntry(memEntry);
   rootNav->addChild(memEntryNav);
}

void Doxy_Work::addSTLIterator(QSharedPointer<EntryNav> classEntryNav, const QString &name)
{
   QSharedPointer<Entry> iteratorClassEntry = QMakeShared<Entry>();

   iteratorClassEntry->fileName   = "[STL]";
   iteratorClassEntry->startLine  = 1;
   iteratorClassEntry->name       = name;
   iteratorClassEntry->section    = Entry::Sections::CLASS_SEC;
   iteratorClassEntry->brief      = "STL iterator class";
   iteratorClassEntry->hidden     = false;
   iteratorClassEntry->artificial = true;

   QSharedPointer<EntryNav> iteratorClassEntryNav = QMakeShared<EntryNav>(classEntryNav, iteratorClassEntry);

   iteratorClassEntryNav->setEntry(iteratorClassEntry);
   classEntryNav->addChild(iteratorClassEntryNav);
}

void Doxy_Work::addSTLClasses(QSharedPointer<EntryNav> rootNav)
{
   QSharedPointer<Entry> namespaceEntry = QMakeShared<Entry>();

   namespaceEntry->fileName   = "[STL]";
   namespaceEntry->startLine  = 1;
   //namespaceEntry->parent   = rootNav->entry();
   namespaceEntry->name       = "std";
   namespaceEntry->section    = Entry::Sections::NAMESPACE_SEC;
   namespaceEntry->brief      = "STL namespace";
   namespaceEntry->hidden     = false;
   namespaceEntry->artificial = true;
  
   QSharedPointer<EntryNav> namespaceEntryNav = QMakeShared<EntryNav>(rootNav, namespaceEntry);
   namespaceEntryNav->setEntry(namespaceEntry);
   rootNav->addChild(namespaceEntryNav);

   STLInfo *info = g_stlinfo;

   while (info->className) {      
      QString fullName = info->className;
      fullName.prepend("std::");

      // add fake Entry for the class
      QSharedPointer<Entry> classEntry = QMakeShared<Entry>();
      classEntry->fileName  = "[STL]";
      classEntry->startLine = 1;
      classEntry->name      = fullName;

      //classEntry->parent   = namespaceEntry;
      classEntry->section    = Entry::CLASS_SEC;
      classEntry->brief      = "STL class";
      classEntry->hidden     = false;
      classEntry->artificial = true;

      // namespaceEntry->addSubEntry(classEntry);
      QSharedPointer<EntryNav> classEntryNav = QMakeShared<EntryNav>(namespaceEntryNav, classEntry);
      classEntryNav->setEntry(classEntry);
      namespaceEntryNav->addChild(classEntryNav);

      // add template arguments to class
      if (info->templType1) {
         ArgumentList al;
         Argument a;

         a.type = "typename";
         a.name = info->templType1;
         al.append(a);

         if (info->templType2) {
            // another template argument
            a = Argument();

            a.type = "typename";
            a.name = info->templType2;
            al.append(a);
         }

         classEntry->tArgLists = new QList<ArgumentList>;
         classEntry->tArgLists->append(al);
      }

      // add member variables
      if (info->templName1) {
         addSTLMember(classEntryNav, info->templType1, info->templName1);
      }

      if (info->templName2) {
         addSTLMember(classEntryNav, info->templType2, info->templName2);
      }

      if (fullName == "std::auto_ptr" || fullName == "std::smart_ptr" ||
            fullName == "std::unique_ptr" || fullName == "std::weak_ptr") {

         QSharedPointer<Entry> memEntry = QMakeShared<Entry>();
         memEntry->name       = "operator->";
         memEntry->args       = "()";
         memEntry->type       = "T*";
         memEntry->protection = Public;
         memEntry->section    = Entry::FUNCTION_SEC;
         memEntry->brief      = "STL member";
         memEntry->hidden     = false;
         memEntry->artificial = false;

         QSharedPointer<EntryNav> memEntryNav = QMakeShared<EntryNav>(classEntryNav, memEntry);
         memEntryNav->setEntry(memEntry);
         classEntryNav->addChild(memEntryNav);
      }

      if (info->baseClass1) {
         classEntry->extends.append(BaseInfo(info->baseClass1, Public, info->virtualInheritance ? Virtual : Normal));
      }

      if (info->baseClass2) {
         classEntry->extends.append(BaseInfo(info->baseClass2, Public, info->virtualInheritance ? Virtual : Normal));
      }

      if (info->iterators) {
         // add iterator class
         addSTLIterator(classEntryNav, fullName + "::iterator");
         addSTLIterator(classEntryNav, fullName + "::const_iterator");
         addSTLIterator(classEntryNav, fullName + "::reverse_iterator");
         addSTLIterator(classEntryNav, fullName + "::const_reverse_iterator");
      }
      info++;
   }
}

void Doxy_Work::addPageToContext(QSharedPointer<PageDef> pd, QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->parent()) {
      // add the page to it's scope
      QString scope = rootNav->parent()->name();

      if (rootNav->parent()->section() == Entry::PACKAGEDOC_SEC) {
         scope = substitute(scope, ".", "::");
      }

      scope = stripAnonymousNamespaceScope(scope);
      scope += "::" + pd->name();

      QSharedPointer<Definition> def = findScopeFromQualifiedName(Doxy_Globals::globalScope, scope, 
                  QSharedPointer<FileDef>(), rootNav->tagInfo());

      if (def) {
         pd->setPageScope(def);
      }
   }
}

void Doxy_Work::addRelatedPage(QSharedPointer<EntryNav> rootNav)
{
   QSharedPointer<Entry> root = rootNav->entry();
   QSharedPointer<GroupDef> gd;

   for (auto &g : *root->groups) {
      if (! g.groupname.isEmpty() && (gd = Doxy_Globals::groupSDict->find(g.groupname))) {
         break;
      }
   }

   QString doc;

   if (root->brief.isEmpty()) {
      doc = root->doc + root->inbodyDocs;
   } else {
      doc = root->brief + "\n\n" + root->doc + root->inbodyDocs;
   }

   QSharedPointer<PageDef> pd = addRelatedPage(root->name, root->args, doc, root->anchors, root->docFile, root->docLine,
                                root->sli, gd, rootNav->tagInfo(), root->lang);

   if (pd) {
      pd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
      pd->addSectionsToDefinition(root->anchors);
      pd->setShowToc(root->stat);

      addPageToContext(pd, rootNav);
   }
}

void Doxy_Work::buildGroupListFiltered(QSharedPointer<EntryNav> rootNav, bool additional, bool includeExternal)
{
   if (rootNav->section() == Entry::GROUPDOC_SEC && ! rootNav->name().isEmpty() &&
         ((! includeExternal && rootNav->tagInfo() == 0) || ( includeExternal && rootNav->tagInfo() != 0)) ) {

      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();

      if ((root->groupDocType == Entry::GROUPDOC_NORMAL && ! additional) ||
            (root->groupDocType != Entry::GROUPDOC_NORMAL && additional)) {

         QSharedPointer<GroupDef> gd = Doxy_Globals::groupSDict->find(root->name);
     
         if (gd) {
            if (! gd->hasGroupTitle() ) {
               gd->setGroupTitle(root->type);

            } else if ( root->type.length() > 0 && root->name != root->type && gd->groupTitle() != root->type ) {
               warn( root->fileName, root->startLine,
                     "group %s: ignoring title \"%s\" that does not match old title \"%s\"\n",
                     csPrintable(root->name), csPrintable(root->type), csPrintable(gd->groupTitle()) );
            }

            gd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
            gd->setDocumentation(root->doc, root->docFile, root->docLine);
            gd->setInbodyDocumentation( root->inbodyDocs, root->inbodyFile, root->inbodyLine );
            gd->addSectionsToDefinition(root->anchors);
            gd->setRefItems(root->sli);
            gd->setLanguage(root->lang);

         } else {
            if (rootNav->tagInfo()) {
               gd = QMakeShared<GroupDef>(root->fileName, root->startLine, root->name, root->type, rootNav->tagInfo()->fileName);
               gd->setReference(rootNav->tagInfo()->tagName);

            } else {
               gd = QMakeShared<GroupDef>(root->fileName, root->startLine, root->name, root->type);
            }

            gd->setBriefDescription(root->brief, root->briefFile, root->briefLine);

            // allow empty docs for group
            gd->setDocumentation(! root->doc.isEmpty() ? root->doc : QString(" "), root->docFile, root->docLine, false);
            gd->setInbodyDocumentation( root->inbodyDocs, root->inbodyFile, root->inbodyLine );
            gd->addSectionsToDefinition(root->anchors);

            Doxy_Globals::groupSDict->insert(root->name, gd);
            gd->setRefItems(root->sli);
            gd->setLanguage(root->lang);
         }
      }

      rootNav->releaseEntry();
   }

   for (auto e : rootNav->children() ) {
      buildGroupListFiltered(e, additional, includeExternal);
   }
}

void Doxy_Work::buildGroupList(QSharedPointer<EntryNav> rootNav)
{
   // --- first process only local groups
   // first process the @defgroups blocks
   buildGroupListFiltered(rootNav, false, false);

   // then process the @addtogroup, @weakgroup blocks
   buildGroupListFiltered(rootNav, true, false);

   // --- then also process external groups
   // first process the @defgroups blocks
   buildGroupListFiltered(rootNav, false, true);

   // then process the @addtogroup, @weakgroup blocks
   buildGroupListFiltered(rootNav, true, true);
}

void Doxy_Work::findGroupScope(QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->section() == Entry::GROUPDOC_SEC && !rootNav->name().isEmpty() &&
         rootNav->parent() && !rootNav->parent()->name().isEmpty()) {

      QSharedPointer<GroupDef> gd;

      if ((gd = Doxy_Globals::groupSDict->find(rootNav->name()))) {
         QString scope = rootNav->parent()->name();

         if (rootNav->parent()->section() == Entry::PACKAGEDOC_SEC) {
            scope = substitute(scope, ".", "::");
         }

         scope = stripAnonymousNamespaceScope(scope);
         scope += "::" + gd->name();

         QSharedPointer<Definition> def = findScopeFromQualifiedName(Doxy_Globals::globalScope, scope, 
                  QSharedPointer<FileDef>(), rootNav->tagInfo());

         if (def) {
            gd->setGroupScope(def);
         }
      }
   }

   RECURSE_ENTRYTREE(findGroupScope, rootNav);
}

void Doxy_Work::organizeSubGroupsFiltered(QSharedPointer<EntryNav> rootNav, bool additional)
{
   if (rootNav->section() == Entry::GROUPDOC_SEC && !rootNav->name().isEmpty()) {
      rootNav->loadEntry(Doxy_Globals::g_storage);

      QSharedPointer<Entry>root = rootNav->entry();

      if ((root->groupDocType == Entry::GROUPDOC_NORMAL && !additional) ||
            (root->groupDocType != Entry::GROUPDOC_NORMAL && additional)) {

         QSharedPointer<GroupDef> gd;

         if ((gd = Doxy_Globals::groupSDict->find(root->name))) {           
            addGroupToGroups(root, gd);
         }
      }

      rootNav->releaseEntry();
   }

   for (auto e : rootNav->children() ) {
      organizeSubGroupsFiltered(e, additional);
   }
}

void Doxy_Work::organizeSubGroups(QSharedPointer<EntryNav> rootNav)
{
   // first process the @defgroups blocks
   organizeSubGroupsFiltered(rootNav, false);

   // then process the @addtogroup, @weakgroup blocks
   organizeSubGroupsFiltered(rootNav, true);
}

void Doxy_Work::buildFileList(QSharedPointer<EntryNav> rootNav)
{
   if (((rootNav->section() == Entry::FILEDOC_SEC) ||
         ((rootNav->section() & Entry::FILE_MASK) && Config::getBool("extract-all"))) &&
         ! rootNav->name().isEmpty() && ! rootNav->tagInfo() ) {

      // skip any file coming from tag files
      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();

      bool ambig;
      QSharedPointer<FileDef> fd = findFileDef(Doxy_Globals::inputNameDict, root->name, ambig);
      
      if (fd && ! ambig) {
         {           
            // using false in setDocumentation is small hack to make sure a file
            // is documented even if a \file command is used without further documentation

            fd->setDocumentation(root->doc, root->docFile, root->docLine, false);
            fd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
            fd->addSectionsToDefinition(root->anchors);
            fd->setRefItems(root->sli);

            for (auto &g : *root->groups) {
               QSharedPointer<GroupDef> gd;

               if (! g.groupname.isEmpty() && (gd = Doxy_Globals::groupSDict->find(g.groupname))) {
                  gd->addFile(fd);
                  fd->makePartOfGroup(gd);
               }
            }
         }

      } else {
         const QString fn = root->fileName;

         QString text;
         text = QString("Name `%1' supplied as the second argument in the \\file statement ").arg(QString(root->name));

         if (ambig) {
            // name is ambiguous

            text += "matches the following input files:\n";
            text += showFileDefMatches(Doxy_Globals::inputNameDict, root->name);
            text += "Use a more specific name or include a larger part of the path";

         } else { 
            // name is not an input file
            text += "is not an input file";
         }

         warn(fn, root->startLine, csPrintable(text));
      }

      rootNav->releaseEntry();
   }

   RECURSE_ENTRYTREE(buildFileList, rootNav);
}

void Doxy_Work::addIncludeFile(QSharedPointer<ClassDef> cd, QSharedPointer<FileDef> ifd, QSharedPointer<Entry> root)
{
   QString temp1 = root->doc.trimmed();
   QString temp2 = root->brief.trimmed();

   if ( (! temp1.isEmpty() || ! temp2.isEmpty() || Config::getBool("extract-all")) && root->protection != Private)  {    
      bool local = Config::getBool("force-local-includes");

      QString includeFile = root->includeFile;

      if (! includeFile.isEmpty() && includeFile.at(0) == '"') {
         local = true;
         includeFile = includeFile.mid(1, includeFile.length() - 2);

      } else if (! includeFile.isEmpty() && includeFile.at(0) == '<') {
         local = false;
         includeFile = includeFile.mid(1, includeFile.length() - 2);
      }

      bool ambig;
      QSharedPointer<FileDef> fd;

      // see if we need to include a verbatim copy of the header file
  
      if (! includeFile.isEmpty() && (fd = findFileDef(Doxy_Globals::inputNameDict, includeFile, ambig)) == 0) {
         // explicit request

         QString text;
         text = QString("the name `%1' supplied as the argument of the \\class, \\struct, \\union, or \\include command ").
                        arg( QString(includeFile) );

         if (ambig) { 
            // name is ambiguous

            text += "matches the following input files:\n";
            text += showFileDefMatches(Doxy_Globals::inputNameDict, root->includeFile);
            text += "Use a more specific name by including a larger part of the path\n";

         } else { 
            // name is not an input file
            text += "is not an input file\n";
         }

         warn(root->fileName, root->startLine, csPrintable(text) );

      } else if (includeFile.isEmpty() && ifd && determineSection(ifd->name()) == Entry::HEADER_SEC) {        
         fd = ifd;
      }

      // if a file is found, mark it as a source file
      if (fd) {

         QString iName = ! root->includeName.isEmpty() ? root->includeName : includeFile;

         if (! iName.isEmpty()) { 
            // user specified include file

            if (iName.at(0) == '<') {
               local = false;   // explicit override

            } else if (iName.at(0) == '"') {
               local = true;
            }

            if (iName.at(0) == '"' || iName.at(0) == '<') {
               iName = iName.mid(1, iName.length() - 2); // strip quotes or brackets
            }

            if (iName.isEmpty()) {
               iName = fd->name();
            }

         } else if (! Config::getList("strip-from-inc-path").isEmpty()) {
            iName = stripFromIncludePath(fd->getFilePath());

         } else {
            // use name of the file containing the class definition
            iName = fd->name();
         }

         if (fd->generateSourceFile()) { 
            // generate code for header
            cd->setIncludeFile(fd, csPrintable(iName), local, ! root->includeName.isEmpty());

         } else { 
            // put #include in the class documentation without link
            cd->setIncludeFile(QSharedPointer<FileDef>(), csPrintable(iName), local, true);

         }
      }
   }
}

/*! returns the Definition object belonging to the first \a level levels of
 *  full qualified name \a name. Creates an artificial scope if the scope is
 *  not found and set the parent/child scope relation if the scope is found.
 */
QSharedPointer<Definition> Doxy_Work::buildScopeFromQualifiedName(const QString name, int level, 
                  SrcLangExt lang, TagInfo *tagInfo)
{  
   int i = 0;
   int p = 0;
   int l;

   QSharedPointer<Definition> prevScope = Doxy_Globals::globalScope;
   QString fullScope;

   while (i < level) {
      int idx = getScopeFragment(name, p, &l);

      if (idx == -1) {
         return prevScope;
      }   

      QString nsName = name.mid(idx, l);

      if (nsName.isEmpty()) {
         return prevScope;
      }

      if (! fullScope.isEmpty()) {
         fullScope += "::";
      }

      fullScope += nsName;
      QSharedPointer<NamespaceDef> nd (Doxy_Globals::namespaceSDict->find(fullScope));

      QSharedPointer<Definition> innerScope = nd;
      QSharedPointer<ClassDef> cd;

      if (! nd) {
         cd = getClass(fullScope);
      }

      if (nd == 0 && cd) {
         // scope is a class
         innerScope = cd;

      } else if (nd == 0 && cd == 0 && fullScope.indexOf('<') == -1) {
         // scope is not known and could be a namespace
         // introduce bogus namespace

         nd = QMakeShared<NamespaceDef>( "[generated]", 1, 1, fullScope, tagInfo ? tagInfo->tagName : QString(),
                        tagInfo ? tagInfo->fileName : QString());

         nd->setLanguage(lang);

         // add namespace to the list
         Doxy_Globals::namespaceSDict->insert(fullScope, nd);
         innerScope = nd;

      } else {
         // scope is a namespace

      }

      if (innerScope) {
         // make the parent/child scope relation
         prevScope->addInnerCompound(innerScope);
         innerScope->setOuterScope(prevScope);

      } else {
         // current scope is a class, so return only the namespace part
         return prevScope;
      }

      // proceed to the next scope fragment
      p = idx + l + 2;
      prevScope = innerScope;
      i++;
   }

   return prevScope;
}

QSharedPointer<Definition> Doxy_Work::findScopeFromQualifiedName(QSharedPointer<Definition> startScope, const QString &n,
                                                                 QSharedPointer<FileDef> fileScope, TagInfo *tagInfo)
{
   QSharedPointer<Definition> resultScope = startScope;

   if (! resultScope) {
      resultScope = Doxy_Globals::globalScope;
   }

   QString scope = stripTemplateSpecifiersFromScope(n, false);
   int l1 = 0;
   int i1;
   i1 = getScopeFragment(scope, 0, &l1);

   if (i1 == -1) {
      return resultScope;
   }

   int p  = i1 + l1;
   int l2 = 0;
   int i2;

   while ((i2 = getScopeFragment(scope, p, &l2)) != -1) {
      QString nestedNameSpecifier = scope.mid(i1, l1);

      QSharedPointer<Definition> orgScope = resultScope;
      resultScope = resultScope->findInnerCompound(nestedNameSpecifier);

      if (! resultScope) {
         NamespaceSDict *usedNamespaces;

         if (orgScope == Doxy_Globals::globalScope && fileScope && (usedNamespaces = fileScope->getUsedNamespaces()))  {
            // also search for used namespaces

            for (auto nd : *usedNamespaces) {
               // restart search within the used namespace

               if (! resultScope) {
                  break;
               }

               resultScope = findScopeFromQualifiedName(nd, n, fileScope, tagInfo);
            }

            if (resultScope) {
               // for a nested class A::I in used namespace N, we get
               // N::A::I while looking for A, so we should compare
               // resultScope->name() against scope.left(i2+l2)
              
               if (rightScopeMatch(resultScope->name(), scope.left(i2 + l2))) {
                  break;
               }

               goto nextFragment;
            }
         }

         // also search for used classes. Complication: we haven't been able
         // to put them in the right scope yet, because we are still resolving
         // the scope relations!
         // Therefore loop through all used classes and see if there is a right
         // scope match between the used class and nestedNameSpecifier.

         auto item = Doxy_Globals::g_usingDeclarations.begin();

         for (auto usedFd : Doxy_Globals::g_usingDeclarations)  {

            if (rightScopeMatch(item.key(), nestedNameSpecifier)) {
               // item.key()is the fully qualified name of nestedNameSpecifier

               QString fqn = item.key() + scope.right(scope.length() - p);
               resultScope = buildScopeFromQualifiedName(fqn, fqn.count("::"), startScope->getLanguage(), 0);

               if (resultScope) {
                   return resultScope;
               }
            }

            ++item;
         }

         return QSharedPointer<Definition>();
      }

   nextFragment:
      i1 = i2;
      l1 = l2;
      p  = i2 + l2;
   }

   return resultScope;
}

ClassDef::CompoundType Doxy_Work::convertToCompoundType(int section, Entry::Traits data)
{
   ClassDef::CompoundType sec = ClassDef::Class;

   if (data.hasTrait(Entry::Virtue::Struct)) {
      sec = ClassDef::Struct;

   } else if (data.hasTrait(Entry::Virtue::Union)) {
      sec = ClassDef::Union;

   } else if (data.hasTrait(Entry::Virtue::Category)) {
      sec = ClassDef::Category;

   } else if (data.hasTrait(Entry::Virtue::Interface)) {
      sec = ClassDef::Interface;

   } else if (data.hasTrait(Entry::Virtue::Protocol)) {
      sec = ClassDef::Protocol;

   } else if (data.hasTrait(Entry::Virtue::Exception)) {
      sec = ClassDef::Exception;

   } else if (data.hasTrait(Entry::Virtue::Service)) {
      sec = ClassDef::Service;

   } else if (data.hasTrait(Entry::Virtue::Singleton)) {
      sec = ClassDef::Singleton;
   }

   switch (section) {    
      case Entry::UNIONDOC_SEC:
         sec = ClassDef::Union;
         break;

      case Entry::STRUCTDOC_SEC:
         sec = ClassDef::Struct;
         break;

      case Entry::INTERFACEDOC_SEC:
         sec = ClassDef::Interface;
         break;

      case Entry::PROTOCOLDOC_SEC:
         sec = ClassDef::Protocol;
         break;

      case Entry::CATEGORYDOC_SEC:
         sec = ClassDef::Category;
         break;

      case Entry::EXCEPTIONDOC_SEC:
         sec = ClassDef::Exception;
         break;

      case Entry::SERVICEDOC_SEC:
         sec = ClassDef::Service;
         break;

      case Entry::SINGLETONDOC_SEC:
         sec = ClassDef::Singleton;
         break;
   }

   return sec;
}

void Doxy_Work::addClassToContext(QSharedPointer<EntryNav> rootNav)
{
   rootNav->loadEntry(Doxy_Globals::g_storage);

   QSharedPointer<Entry> root = rootNav->entry();
   QSharedPointer<FileDef> fd = rootNav->fileDef();

   QString scName;
   if (rootNav->parent()->section() & Entry::SCOPE_MASK) {
      scName = rootNav->parent()->name();
   }

   // name without parent's scope
   QString fullName = root->name;

   // strip off any template parameters (but not those for specializations)
   fullName = stripTemplateSpecifiersFromScope(fullName);

   // name with scope (if not present already)
   QString qualifiedName = fullName;

   if (! scName.isEmpty() && ! leftScopeMatch(fullName, scName)) {
      qualifiedName.prepend(scName + "::");
   }

   // see if we already found the class before
   QSharedPointer<ClassDef> cd = getClass(qualifiedName);

   Debug::print(Debug::Classes, 0, "  Found class with name %s (qualifiedName=%s -> cd=%p)\n",
                cd ? csPrintable(cd->name()) : csPrintable(root->name), csPrintable(qualifiedName), cd.data() );

   if (cd) {
      // class exists
      fullName = cd->name();
      Debug::print(Debug::Classes, 0, "  Existing class %s\n", csPrintable(cd->name()) );

      cd->setDocumentation(root->doc, root->docFile, root->docLine);
      cd->setBriefDescription(root->brief, root->briefFile, root->briefLine);

      if (root->bodyLine != -1 && cd->getStartBodyLine() == -1) {
         cd->setBodySegment(root->bodyLine, root->endBodyLine);
         cd->setBodyDef(fd);
      }

      if (root->tArgLists != nullptr && 
                  (cd->templateArguments() == 0 || (cd->isForwardDeclared() && 
                  (! root->m_traits.hasTrait(Entry::Virtue::ForwardDecl)) ))) {

         // happens if a template class is declared before the actual definition or 
         // if a forward declaration has different template parameter names

         ArgumentList tArgList = getTemplateArgumentsFromName(cd->name(), root->tArgLists);
         cd->setTemplateArguments(&tArgList);
      }

      cd->setCompoundType(convertToCompoundType(root->section, root->m_traits));

   } else { 
      // new class
      ClassDef::CompoundType sec = convertToCompoundType(root->section, root->m_traits);

      QString className;
      QString namespaceName;
      extractNamespaceName(fullName, className, namespaceName);

      QString tagName;
      QString refFileName;
      TagInfo *tagInfo = rootNav->tagInfo();

      if (tagInfo) {
         tagName     = tagInfo->tagName;
         refFileName = tagInfo->fileName;

         if (fullName.indexOf("::") != -1) {
            // symbols imported via tag files may come without the parent scope
            // so we artificially create it here

            buildScopeFromQualifiedName(fullName, fullName.count("::"), root->lang, tagInfo);
         }
      }

      ArgumentList tArgList;
      int i;

      if ((root->lang == SrcLangExt_CSharp || root->lang == SrcLangExt_Java) && (i = fullName.indexOf('<')) != -1) {
         // a Java/C# generic class looks like a C++ specialization, split the name and template arguments here
        
         stringToArgumentList(fullName.mid(i), &tArgList);
         fullName = fullName.left(i);

      } else {
         tArgList = getTemplateArgumentsFromName(fullName, root->tArgLists);

      }

      bool isEnum = root->m_traits.hasTrait(Entry::Virtue::Enum);

      cd = QMakeShared<ClassDef>(tagInfo ? tagName : root->fileName, root->startLine, root->startColumn,
                        fullName, sec, tagName, refFileName, true, isEnum);

      Debug::print(Debug::Classes, 0, "  New class `%s' (sec=0x%08x)! #tArgLists=%d tagInfo=%p\n",
                   csPrintable(fullName), sec, root->tArgLists ? (int)root->tArgLists->count() : -1, tagInfo);

      // copy docs to definition
      cd->setDocumentation(root->doc, root->docFile, root->docLine);
      cd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
      cd->setLanguage(root->lang);
      cd->setId(root->id);
      cd->setHidden(root->hidden);
      cd->setArtificial(root->artificial);
      cd->setClassTraits(root->m_traits);

      cd->setTypeConstraints(&root->typeConstr);
      cd->setTemplateArguments(&tArgList);   

      cd->setProtection(root->protection); 
      cd->setIsStatic(root->stat);

      // file definition containing the class cd
      cd->setBodySegment(root->bodyLine, root->endBodyLine);
      cd->setBodyDef(fd);

      // see if the class is found inside a namespace
      cd->insertUsedFile(fd);

      // add class to the list
      Doxy_Globals::classSDict->insert(fullName, cd);

      if (cd->isGeneric()) {
         // generics are also stored in a separate dictionary for fast lookup of instantions
         Doxy_Globals::genericsDict->insert(fullName, cd);
      }
   }

   cd->addSectionsToDefinition(root->anchors);
   if (! root->subGrouping) {
      cd->setSubGrouping(false);
   }

   if (cd->hasDocumentation()) {
      addIncludeFile(cd, fd, root);
   }

   if (fd && (root->section & Entry::COMPOUND_MASK)) {
      cd->setFileDef(fd);
      fd->insertClass(cd);
   }

   addClassToGroups(root, cd);
   cd->setRefItems(root->sli);

   rootNav->releaseEntry();
}

// build a list of all classes mentioned in the documentation
// and all classes that have a documentation block before their definition
void Doxy_Work::buildClassList(QSharedPointer<EntryNav> rootNav)
{
   if (((rootNav->section() & Entry::COMPOUND_MASK) || rootNav->section() == Entry::OBJCIMPL_SEC) && ! rootNav->name().isEmpty()) {
      addClassToContext(rootNav);
   }
 
   RECURSE_ENTRYTREE(buildClassList, rootNav);
}

void Doxy_Work::buildClassDocList(QSharedPointer<EntryNav> rootNav)
{
   if ((rootNav->section() & Entry::COMPOUNDDOC_MASK) && ! rootNav->name().isEmpty()) {
      addClassToContext(rootNav);
   }

   RECURSE_ENTRYTREE(buildClassDocList, rootNav);
}

void Doxy_Work::resolveClassNestingRelations()
{
   for (auto item : *Doxy_Globals::classSDict) {
      item->visited = false;
   }

   bool done = false;
   int iteration = 0;

   while (! done) {
      done = true;
      ++iteration;

      for (auto cd : *Doxy_Globals::classSDict) {

         if (! cd->visited) {
            QString name = stripAnonymousNamespaceScope(cd->name());

            // add class to the correct structural context
            QSharedPointer<Definition> def = findScopeFromQualifiedName(Doxy_Globals::globalScope, 
                  name, cd->getFileDef(), nullptr);

            if (def) {
               def->addInnerCompound(cd);

               cd->setOuterScope(def);
               cd->visited = true;

               done = false;
            }
         }
      }
   }

   // give warnings for unresolved compounds
   for (auto cd : *Doxy_Globals::classSDict) {

      if (! cd->visited) {
         QString name = stripAnonymousNamespaceScope(cd->name());

         /// create the scope artificially so we can at least relate scopes properly

         QSharedPointer<Definition> d = buildScopeFromQualifiedName(name, name.count("::"), cd->getLanguage(), 0);

         if (d != cd && ! cd->getDefFileName().isEmpty())  {
            // avoid recursion in case of redundant scopes, i.e: namespace N { class N::C {}; }
            // for this case DoxyPress assumes the exitance of a namespace N::N in which C is to be found!
            // also avoid warning for stuff imported via a tagfile.

            d->addInnerCompound(cd);
            cd->setOuterScope(d);

            warn(cd->getDefFileName(), cd->getDefLine(), "Internal inconsistency: scope for class %s not found", csPrintable(name));
         }
      }
   }
}

QSharedPointer<ClassDef> Doxy_Work::createTagLessInstance(QSharedPointer<ClassDef> rootCd, QSharedPointer<ClassDef> templ, 
                  const QString &fieldName)
{
   QString fullName = removeAnonymousScopes(templ->name());

   if (fullName.right(2) == "::") {
      fullName = fullName.left(fullName.length() - 2);
   }

   fullName += "." + fieldName;
   QSharedPointer<ClassDef> cd = QMakeShared<ClassDef>(templ->getDefFileName(), templ->getDefLine(), templ->getDefColumn(), 
                  fullName, templ->compoundType());

   cd->setDocumentation(templ->documentation(), templ->docFile(), templ->docLine()); // copy docs to definition
   cd->setBriefDescription(templ->briefDescription(), templ->briefFile(), templ->briefLine());
   cd->setLanguage(templ->getLanguage());
   cd->setBodySegment(templ->getStartBodyLine(), templ->getEndBodyLine());
   cd->setBodyDef(templ->getBodyDef());

   cd->setOuterScope(rootCd->getOuterScope());

   if (rootCd->getOuterScope() != Doxy_Globals::globalScope) {
      rootCd->getOuterScope()->addInnerCompound(cd);
   }

   QSharedPointer<FileDef> fd = templ->getFileDef();

   if (fd) {
      cd->setFileDef(fd);
      fd->insertClass(cd);
   }

   SortedList<QSharedPointer<GroupDef>> *groups = rootCd->partOfGroups();

   if (groups != 0) {
      for (auto item : *groups) {
         cd->makePartOfGroup(item);
         item->addClass(cd);
      }
   }
   
   Doxy_Globals::classSDict->insert(fullName, QSharedPointer<ClassDef> (cd) );

   QSharedPointer<MemberList> ml = templ->getMemberList(MemberListType_pubAttribs);

   if (ml) {

      for (auto md : *ml) {
        
         QSharedPointer<MemberDef> imd = QMakeShared<MemberDef>(md->getDefFileName(), md->getDefLine(),
                   md->getDefColumn(), md->typeString(), md->name(), md->argsString(), md->excpString(),
                   md->protection(), md->virtualness(), md->isStatic(), Member, md->memberType(), nullptr, nullptr);

         imd->setMemberClass(cd);

         imd->setDocumentation(md->documentation(), md->docFile(), md->docLine());
         imd->setBriefDescription(md->briefDescription(), md->briefFile(), md->briefLine());
         imd->setInbodyDocumentation(md->inbodyDocumentation(), md->inbodyFile(), md->inbodyLine());
         imd->setMemberTraits(md->getMemberTraits());
         imd->setMemberGroupId(md->getMemberGroupId());
         imd->setInitializer(md->initializer());
         imd->setMaxInitLines(md->initializerLines());
         imd->setBitfields(md->bitfieldString());
         imd->setLanguage(md->getLanguage());

         cd->insertMember(imd);
      }
   }

   return cd;
}

/** Look through the members of class \a cd and its public members.
 *  If there is a member m of a tag less struct/union,
 *  then we create a duplicate of the struct/union with the name of the
 *  member to identify it.
 *  So if cd has name S, then the tag less struct/union will get name S.m
 *  Since tag less structs can be nested we need to call this function
 *  recursively. Later on we need to patch the member types so we keep
 *  track of the hierarchy of classes we create.
 */
void Doxy_Work::processTagLessClasses(QSharedPointer<ClassDef> rootCd, QSharedPointer<ClassDef> cd, 
                  QSharedPointer<ClassDef>  tagParentCd, const QString &prefix, int count)
{
   if (cd->getClassSDict()) {
      QSharedPointer<MemberList> ml = cd->getMemberList(MemberListType_pubAttribs);

      if (ml) {

         for (auto md : *ml) {
            // return type
            QString type = md->typeString();

            if (type.indexOf("::@") != -1) {
               // member of tag less struct/union

               for (auto icd : *cd->getClassSDict()) {

                  if (type.indexOf(icd->name()) != -1) {
                     // matching tag less struct/union
                     QString name = md->name();

                     if (name.at(0) == '@') {
                        name = "__unnamed__";
                     }

                     if (! prefix.isEmpty()) {
                        name.prepend(prefix + ".");
                     }

                     QSharedPointer<ClassDef> ncd (createTagLessInstance(rootCd, icd, name));

                     processTagLessClasses(rootCd, icd, ncd, name, count + 1);

                     tagParentCd->addTaggedInnerClass(ncd);
                     ncd->setTagLessReference(icd);

                     // replace tag-less type for generated/original member
                     // by newly created class name.
                     // note the difference between changing cd and tagParentCd.
                     // for the initial call this is the same pointer, but for
                     // recursive calls cd is the original tag-less struct (of which
                     // there is only one instance) and tagParentCd is the newly
                     // generated tagged struct of which there can be multiple instances!

                     QSharedPointer<MemberList> pml = tagParentCd->getMemberList(MemberListType_pubAttribs);

                     if (pml) {

                        for (auto pmd : *pml) {
                           if (pmd->name() == md->name()) {
                              pmd->setAccessorType(ncd, substitute(pmd->typeString(), icd->name(), ncd->name()));
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
}

void Doxy_Work::findTagLessClasses(QSharedPointer<ClassDef> cd)
{
   if (cd->getClassSDict()) {
      for (auto icd : *cd->getClassSDict()) {
         if (icd->name().indexOf("@") == -1) { // process all non-anonymous inner classes
            findTagLessClasses(icd);
         }
      }
   }

   processTagLessClasses(cd, cd, cd, "", 0); // process tag less inner struct/classes (if any)
}

void Doxy_Work::findTagLessClasses()
{
   for (auto cd : *Doxy_Globals::classSDict) {
      QSharedPointer<Definition> scope = cd->getOuterScope();

      if (scope && scope->definitionType() != Definition::TypeClass) { 
         // that is not nested
         findTagLessClasses(cd);
      }
   }
}

// build a list of all namespaces mentioned in the documentation
// and all namespaces which have a documentation block before their definition
void Doxy_Work::buildNamespaceList(QSharedPointer<EntryNav> rootNav)
{
   if ( (rootNav->section() == Entry::NAMESPACE_SEC || rootNav->section() == Entry::NAMESPACEDOC_SEC ||
            rootNav->section() == Entry::PACKAGEDOC_SEC) && ! rootNav->name().isEmpty()) {

      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();
  
      QString fullName = root->name;
      if (root->section == Entry::PACKAGEDOC_SEC) {
         fullName = substitute(fullName, ".", "::");
      }

      fullName = stripAnonymousNamespaceScope(fullName);
         
      // change the namespace to be match the ns alias
      auto iter = Doxy_Globals::nsRenameAlias.find(fullName);
            
      if (iter != Doxy_Globals::nsRenameAlias.end()) {
         fullName = iter.value();         
      }  
      
      if (! fullName.isEmpty()) {
         QSharedPointer<NamespaceDef> nd;

         if ((nd = Doxy_Globals::namespaceSDict->find(fullName))) {
            // existing namespace

            nd->setDocumentation(root->doc, root->docFile, root->docLine);

            // change name to match docs
            nd->setName(fullName); 

            nd->addSectionsToDefinition(root->anchors);

            nd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
            if (nd->getLanguage() == SrcLangExt_Unknown) {
               nd->setLanguage(root->lang);
            }

            // file definition containing the namespace nd
            QSharedPointer<FileDef> fd = rootNav->fileDef();

            // insert the namespace in the file definition
            if (fd) {
               fd->insertNamespace(nd);
            }

            addNamespaceToGroups(root, nd);
            nd->setRefItems(root->sli);

         } else { 
            // new namespace

            QString tagName;
            QString tagFileName;

            TagInfo *tagInfo = rootNav->tagInfo();
            if (tagInfo) {
               tagName     = tagInfo->tagName;
               tagFileName = tagInfo->fileName;
            }

            bool isPublished = root->m_traits.hasTrait(Entry::Virtue::Published);

            nd = QMakeShared<NamespaceDef>(tagInfo ? tagName : root->fileName, root->startLine, root->startColumn,
                  fullName, tagName, tagFileName, root->type, isPublished);

            nd->setDocumentation(root->doc, root->docFile, root->docLine); // copy docs to definition
            nd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
            nd->addSectionsToDefinition(root->anchors);
            nd->setHidden(root->hidden);
            nd->setArtificial(root->artificial);
            nd->setLanguage(root->lang);
            nd->setId(root->id);

            addNamespaceToGroups(root, nd);
            nd->setRefItems(root->sli);

            // file definition containing the namespace nd
            QSharedPointer<FileDef> fd = rootNav->fileDef();

            // insert the namespace in the file definition
            if (fd) {
               fd->insertNamespace(nd);
            }

            // the empty string test is needed for extract all case
            nd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
            nd->insertUsedFile(fd);
            nd->setBodySegment(root->bodyLine, root->endBodyLine);
            nd->setBodyDef(fd);

            // add class to the list
            Doxy_Globals::namespaceSDict->insert(fullName, nd);

            // also add namespace to the correct structural context
            QSharedPointer<Definition> d = findScopeFromQualifiedName(Doxy_Globals::globalScope, fullName, QSharedPointer<FileDef>(), tagInfo);

            if (d == 0)  {
               // we did not find anything, create the scope artificially
               // anyway, so we can at least relate scopes properly.

               d = buildScopeFromQualifiedName(fullName, fullName.count("::"), nd->getLanguage(), tagInfo);

               d->addInnerCompound(nd);
               nd->setOuterScope(d);

               // TODO: Due to the order in which the tag file is written
               // a nested class can be found before its parent

            } else {
               d->addInnerCompound(nd);
               nd->setOuterScope(d);
            }
         }
      }

      rootNav->releaseEntry();
   }

   RECURSE_ENTRYTREE(buildNamespaceList, rootNav);
}

QSharedPointer<NamespaceDef> Doxy_Work::findUsedNamespace(NamespaceSDict *unl, const QString &name)
{
   QSharedPointer<NamespaceDef> usingNd;

   if (unl) {
      for (auto und : *unl) {
         QString uScope = und->name() + "::";
         usingNd = getResolvedNamespace(uScope + name);
      }
   }

   return usingNd;
}

void Doxy_Work::findUsingDirectives(QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->section() == Entry::USINGDIR_SEC) {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();

      QString name = substitute(root->name, ".", "::");

      if (name.right(2) == "::") {
         name = name.left(name.length() - 2);
      }

      if (! name.isEmpty()) {
         QSharedPointer<NamespaceDef> usingNd;
         QSharedPointer<NamespaceDef> nd;
         QSharedPointer<FileDef> fd = rootNav->fileDef();

         QString nsName;

         // see if the using statement was found inside a namespace or inside the global file scope
         if (rootNav->parent() && rootNav->parent()->section() == Entry::NAMESPACE_SEC &&
               (fd == 0 || fd->getLanguage() != SrcLangExt_Java) ) {

            // not a .java file
            nsName = stripAnonymousNamespaceScope(rootNav->parent()->name());

            if (! nsName.isEmpty()) {
               nd = getResolvedNamespace(nsName);
            }
         }

         // find the scope in which the `using' namespace is defined by prepending
         // the possible scopes in which the using statement was found
         // start with the most inner scope and going to the most outer scope (i.e. file scope)
         int scopeOffset = nsName.length();

         do {
            QString scope = scopeOffset > 0 ? nsName.left(scopeOffset) + "::" : QString();
            usingNd = getResolvedNamespace(scope + name);

           if (scopeOffset == 0) {
               scopeOffset = -1;

            } else if ((scopeOffset = nsName.lastIndexOf("::", scopeOffset - 1)) == -1) {
               scopeOffset = 0;

            }

         } while (scopeOffset >= 0 && usingNd == 0);

         if (usingNd == 0 && nd)  {
            // not found, try used namespaces in this scope
            // or in one of the parent namespace scopes

            QSharedPointer<NamespaceDef> pnd = nd;

            while (pnd && usingNd == 0) {
               // also try with one of the used namespaces found earlier
               usingNd = findUsedNamespace(const_cast<NamespaceSDict *>(&pnd->getUsedNamespaces()), name);

               // goto the parent
               QSharedPointer<Definition> s = pnd->getOuterScope();

               if (s && s->definitionType() == Definition::TypeNamespace) {
                  pnd = s.dynamicCast<NamespaceDef>();  

               } else {
                  pnd = QSharedPointer<NamespaceDef>();

               }
            }
         }

         if (usingNd == 0 && fd) {
            // still nothing, also try used namespace in the global scope         
            usingNd = findUsedNamespace(fd->getUsedNamespaces(), name);
         }

         // add the namespace the correct scope
         if (usingNd) {

            if (nd) {
               nd->addUsingDirective(usingNd);

            } else if (fd) {
               fd->addUsingDirective(usingNd);
            }

         } else {
            // unknown namespace, but add it anyway

            QSharedPointer<NamespaceDef> nd = QMakeShared<NamespaceDef>(root->fileName, root->startLine, root->startColumn, name);

            nd->setDocumentation(root->doc, root->docFile, root->docLine);             // copy docs to definition
            nd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
            nd->addSectionsToDefinition(root->anchors);

            nd->setHidden(root->hidden);
            nd->setArtificial(true);
            nd->setLanguage(root->lang);
            nd->setId(root->id);

            for (auto &g : *root->groups) {
               QSharedPointer<GroupDef> gd;

               if (! g.groupname.isEmpty() && (gd = Doxy_Globals::groupSDict->find(g.groupname))) {
                  gd->addNamespace(nd);
               }
            }

            // insert the namespace in the file definition
            if (fd) {
               fd->insertNamespace(nd);
               fd->addUsingDirective(nd);
            }

            // the empty string test is needed for extract all case
            nd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
            nd->insertUsedFile(fd);

            // add class to the list
            Doxy_Globals::namespaceSDict->insert(name, nd);
            nd->setRefItems(root->sli);
         }
      }

      rootNav->releaseEntry();
   }

   RECURSE_ENTRYTREE(findUsingDirectives, rootNav);
}

void Doxy_Work::buildListOfUsingDecls(QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->section() == Entry::USINGDECL_SEC && ! (rootNav->parent()->section()&Entry::COMPOUND_MASK)  ) {
      // not a class/struct member

      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();

      QString name = substitute(root->name, ".", "::");

      if (! Doxy_Globals::g_usingDeclarations.contains(name)) {
          QSharedPointer<FileDef> fd = rootNav->fileDef();

         if (fd) {
            Doxy_Globals::g_usingDeclarations.insert(name, *fd);
         }
      }

      rootNav->releaseEntry();
   }

   RECURSE_ENTRYTREE(buildListOfUsingDecls, rootNav);
}

void Doxy_Work::findUsingDeclarations(QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->section() == Entry::USINGDECL_SEC && !(rootNav->parent()->section()&Entry::COMPOUND_MASK) ) {
      // not a class/struct member
      
      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();     

      if (! root->name.isEmpty()) {
         QSharedPointer<ClassDef> usingCd;
         QSharedPointer<NamespaceDef> nd;

         QSharedPointer<FileDef> fd = rootNav->fileDef();
         QString scName;

         // see if the using statement was found inside a namespace or inside
         // the global file scope.

         if (rootNav->parent()->section() == Entry::NAMESPACE_SEC) {
            scName = rootNav->parent()->name();

            if (!scName.isEmpty()) {
               nd = getResolvedNamespace(scName);
            }
         }

         // Assume the using statement was used to import a class.
         // Find the scope in which the `using' namespace is defined by prepending
         // the possible scopes in which the using statement was found, starting
         // with the most inner scope and going to the most outer scope (i.e. file scope).

         QString name = substitute(root->name, ".", "::"); //Java/C# scope->internal
         usingCd = getResolvedClass(nd, fd, name);

         if (usingCd == 0) {
            usingCd = Doxy_Globals::hiddenClasses->find(name);
         }

         if (usingCd == 0) { 
            // definition not in the input => add an artificial class
            Debug::print(Debug::Classes, 0, "  New using class `%s' (sec=0x%08x)! #tArgLists=%d\n",
                         csPrintable(name), root->section, root->tArgLists ? root->tArgLists->count() : -1);

            usingCd = QMakeShared<ClassDef>("<using>", 1, 1,name, ClassDef::Class);

            Doxy_Globals::hiddenClasses->insert(root->name, usingCd);
            usingCd->setArtificial(true);

            usingCd->setLanguage(root->lang);

         } else {
            Debug::print(Debug::Classes, 0, "  Found used class %s in scope=%s\n",
                         csPrintable(usingCd->name()), nd ? csPrintable(nd->name()) : 
                         fd ? csPrintable(fd->name()) : "<unknown>");
         }

         if (nd) {           
            nd->addUsingDeclaration(usingCd);

         } else if (fd) {
            fd->addUsingDeclaration(usingCd);
         }
      }

      rootNav->releaseEntry();
   }
   RECURSE_ENTRYTREE(findUsingDeclarations, rootNav);
}

void Doxy_Work::findUsingDeclImports(QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->section() == Entry::USINGDECL_SEC && (rootNav->parent()->section()&Entry::COMPOUND_MASK) )  {
     
      QString fullName = removeRedundantWhiteSpace(rootNav->parent()->name());
      fullName = stripAnonymousNamespaceScope(fullName);
      fullName = stripTemplateSpecifiersFromScope(fullName);

      QSharedPointer<ClassDef> cd = getClass(fullName);

      if (cd) {         
         int i = rootNav->name().indexOf("::");

         if (i != -1) {
            QString scope   = rootNav->name().left(i);
            QString memName = rootNav->name().right(rootNav->name().length() - i - 2);
            QSharedPointer<ClassDef> bcd = getResolvedClass(cd, QSharedPointer<FileDef>(), scope); // todo: file in fileScope parameter

            if (bcd) {              
               MemberNameInfoSDict *mndict = bcd->memberNameInfoSDict();

               if (mndict) {
                  QSharedPointer<MemberNameInfo> mni = mndict->find(memName);

                  if (mni) {

                     for (auto &mi : *mni) {
                        QSharedPointer<MemberDef> md = mi.memberDef;

                        if (md && md->protection() != Private) {

                           rootNav->loadEntry(Doxy_Globals::g_storage);

                           QSharedPointer<Entry>root = rootNav->entry();
                           QSharedPointer<MemberDef> newMd;

                           {
                              QString fileName = root->fileName;

                              if (fileName.isEmpty() && rootNav->tagInfo()) {
                                 fileName = rootNav->tagInfo()->tagName;
                              }

                              ArgumentList *templAl = md->templateArguments();
                              ArgumentList *al = md->templateArguments();

                              newMd = QMakeShared<MemberDef>(fileName, root->startLine, root->startColumn, md->typeString(),
                                 memName, md->argsString(), md->excpString(), root->protection, root->virt, md->isStatic(),
                                 Member, md->memberType(),  templAl, al);
                           }

                           newMd->setMemberClass(cd);
                           cd->insertMember(newMd);

                           if (! root->doc.isEmpty() || ! root->brief.isEmpty()) {
                              newMd->setDocumentation(root->doc, root->docFile, root->docLine);
                              newMd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
                              newMd->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);

                           } else {
                              newMd->setDocumentation(md->documentation(), md->docFile(), md->docLine());
                              newMd->setBriefDescription(md->briefDescription(), md->briefFile(), md->briefLine());
                              newMd->setInbodyDocumentation(md->inbodyDocumentation(), md->inbodyFile(), md->inbodyLine());
                           }

                           newMd->setDefinition(md->definition());
                           newMd->enableCallGraph(root->callGraph);
                           newMd->enableCallerGraph(root->callerGraph);
                           newMd->setBitfields(md->bitfieldString());
                           newMd->addSectionsToDefinition(root->anchors);
                           newMd->setBodySegment(md->getStartBodyLine(), md->getEndBodyLine());
                           newMd->setBodyDef(md->getBodyDef());
                           newMd->setInitializer(md->initializer());
                           newMd->setMaxInitLines(md->initializerLines());
                           newMd->setMemberGroupId(root->mGrpId);
                           newMd->setMemberTraits(md->getMemberTraits());
                           newMd->setLanguage(root->lang);
                           newMd->setId(root->id);

                           rootNav->releaseEntry();
                        }
                     }
                  }
               }
            }
         }
      }

   }
   RECURSE_ENTRYTREE(findUsingDeclImports, rootNav);
}

void Doxy_Work::findIncludedUsingDirectives()
{
   // first mark all files as not visited
   for (auto fn : *Doxy_Globals::inputNameList) {

      for (auto fd : *fn) {
         fd->visited = false;
      }
   }

   // recursively add using directives found in #include files
   // to files which have not been visited
   for (auto fn : *Doxy_Globals::inputNameList) {

      for (auto fd : *fn) {
         if (! fd->visited) {            
            fd->addIncludedUsingDirectives();
         }
      }
   }
}

QSharedPointer<MemberDef> Doxy_Work::addVariableToClass(QSharedPointer<EntryNav> rootNav, QSharedPointer<ClassDef> cd, MemberType mtype, 
                           const QString &name, bool fromAnnScope, QSharedPointer<MemberDef> fromAnnMemb, Protection prot, Relationship related)
{
   QSharedPointer<Entry> root = rootNav->entry();

   QString qualScope = cd->qualifiedNameWithTemplateParameters();
   QString scopeSeparator = "::";

   SrcLangExt lang = cd->getLanguage();

   if (lang == SrcLangExt_Java || lang == SrcLangExt_CSharp) {
      qualScope = substitute(qualScope, "::", ".");
      scopeSeparator = ".";
   }

   Debug::print(Debug::Variables, 0, "  class variable:\n" 
                "    `%s' `%s'::`%s' `%s' prot=`%d ann=%d init=`%s'\n",
                csPrintable(root->type), csPrintable(qualScope), csPrintable(name), csPrintable(root->args),
                root->protection, fromAnnScope, csPrintable(root->initializer));

   QString def;

   if (! root->type.isEmpty()) {

      bool isAlias = root->m_traits.hasTrait(Entry::Virtue::Alias);

      if (related || mtype == MemberType_Friend || Config::getBool("hide-scope-names")) {
       
         if (isAlias) { 
            // turn 'typedef B A' into 'using A = B'
            def = "using " + name + " = " + root->type.mid(7);

         } else {
            def = root->type + " " + name + root->args;

         }

      } else {
         if (isAlias) { 
            // turn 'typedef B C::A' into 'using C::A = B'
            def = "using " + qualScope + scopeSeparator + name + " = " + root->type.mid(7);

         } else {
            def = root->type + " " + qualScope + scopeSeparator + name + root->args;

         }
      }

   } else {
      if (Config::getBool("hide-scope-names")) {
         def = name + root->args;

      } else {
         def = qualScope + scopeSeparator + name + root->args;

      }
   }

   def = stripPrefix(def, "static ");

   // see if the member is already found in the same scope
   // (this may be the case for a static member that is initialized outside the class)
   QSharedPointer<MemberName> mn = Doxy_Globals::memberNameSDict->find(name);

   if (mn) {

      for (auto md : *mn) {

         if (md->getClassDef() == cd && removeRedundantWhiteSpace(root->type) == md->typeString()) {
            // member already in the scope

            if (root->lang == SrcLangExt_ObjC && root->mtype == Property && md->memberType() == MemberType_Variable) {
               // Objective-C 2.0 property, turn variable into a property

               md->setProtection(root->protection);
               cd->reclassifyMember(md, MemberType_Property);

            } else if (root->mtype == Property) { 
               // copperspice has multiple property elements, update memberDef

               if (! root->m_read.isEmpty()) { 
                  md->setPropertyRead(root->m_read);
               }   
              
               if (! root->m_write.isEmpty()) {
                  md->setPropertyWrite(root->m_write);
               } 

               if (! root->m_reset.isEmpty()) {
                  md->setPropertyReset(root->m_reset);
               }

               if (! root->m_notify.isEmpty()) {
                  md->setPropertyNotify(root->m_notify);
               }
            }

            addMemberDocs(rootNav, md, def, 0, false);
            return md;
         }
      }
   }

   QString fileName = root->fileName;

   if (fileName.isEmpty() && rootNav->tagInfo()) {
      fileName = rootNav->tagInfo()->tagName;
   }

   // new member variable, typedef, enum value, or property
   QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(fileName, root->startLine, root->startColumn, 
            root->type, name, root->args, root->exception, prot, Normal, root->stat, 
            related, mtype, root->tArgLists ? &(root->tArgLists->last()) : 0, nullptr);

   md->setTagInfo(rootNav->tagInfo());
   md->setMemberClass(cd);                // also sets outer scope (i.e. getOuterScope())

   // md->setDefFile(root->fileName);
   // md->setDefLine(root->startLine);

   md->setDocumentation(root->doc, root->docFile, root->docLine);
   md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
   md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
   md->setDefinition(def);
   md->setBitfields(root->bitfields);
   md->addSectionsToDefinition(root->anchors);
   md->setFromAnonymousScope(fromAnnScope);
   md->setFromAnonymousMember(fromAnnMemb);

   // md->setIndentDepth(indentDepth);
   md->setBodySegment(root->bodyLine, root->endBodyLine);
   md->setInitializer(root->initializer);
   md->setMaxInitLines(root->initLines);
   md->setMemberGroupId(root->mGrpId);
   md->setMemberTraits(root->m_traits);

   // copperspice properties
   md->setPropertyRead(root->m_read);
   md->setPropertyWrite(root->m_write);
   md->setPropertyReset(root->m_reset);
   md->setPropertyNotify(root->m_notify);

   md->enableCallGraph(root->callGraph);
   md->enableCallerGraph(root->callerGraph);
   md->setHidden(root->hidden);
   md->setArtificial(root->artificial);
   md->setLanguage(root->lang);
   md->setId(root->id);
   addMemberToGroups(root, md);

   md->setBodyDef(rootNav->fileDef());

   // add the member to the global list
   if (mn) {
      mn->append(md);

   } else { 
      // new variable name, add the member to the class
      mn = QMakeShared<MemberName>(name);
      mn->append(md);

      Doxy_Globals::memberNameSDict->insert(name, mn); 
   }

   cd->insertMember(md);
   md->setRefItems(root->sli);

   //TODO: insert FileDef instead of filename strings
   cd->insertUsedFile(rootNav->fileDef());

   rootNav->changeSection(Entry::EMPTY_SEC);

   return md;
}

QSharedPointer<MemberDef> Doxy_Work::addVariableToFile(QSharedPointer<EntryNav> rootNav, MemberType mtype, const QString &scope, 
                     const QString &name, bool fromAnnScope, QSharedPointer<MemberDef> fromAnnMemb)
{
   QSharedPointer<Entry> root = rootNav->entry();

   Debug::print(Debug::Variables, 0, "  global variable:\n"
                "    type=`%s' scope=`%s' name=`%s' args=`%s' prot=`%d  mtype=%d  lang=%d\n",
                csPrintable(root->type), csPrintable(scope), csPrintable(name), csPrintable(root->args), 
                root->protection, mtype, root->lang );

   QSharedPointer<FileDef> fd = rootNav->fileDef();

   // see if we have a typedef that should hide a struct or union
   if (mtype == MemberType_Typedef && Config::getBool("use-typedef-name")) {
      QString type = root->type;

      type = stripPrefix(type, "typedef ");

      if (type.left(7) == "struct " || type.left(6) == "union ") {
         type = stripPrefix(type, "struct ");
         type = stripPrefix(type, "union ");

         static QRegExp re("[a-z_A-Z][a-z_A-Z0-9]*");
         int l;
         int s;

         s = re.indexIn(type, 0);
         l = re.matchedLength();

         if (s >= 0) {
            QString typeValue = type.mid(s, l);
            QSharedPointer<ClassDef> cd = getClass(typeValue);

            if (cd) {
               // this typedef should hide compound name cd, so we
               // change the name that is displayed from cd
               cd->setClassName(name);
               cd->setDocumentation(root->doc, root->docFile, root->docLine);
               cd->setBriefDescription(root->brief, root->briefFile, root->briefLine);

               return QSharedPointer<MemberDef>();
            }
         }
      }
   }

   // see if the function is inside a namespace
   QSharedPointer<NamespaceDef> nd;

   if (! scope.isEmpty()) {
      if (scope.indexOf('@') != -1) {
         return QSharedPointer<MemberDef>();   // anonymous scope
      }

      nd = getResolvedNamespace(scope);
   }

   QString def;

   // determine the definition of the global variable
   if (nd && ! nd->name().isEmpty() && nd->name().at(0) != '@' && ! Config::getBool("hide-scope-names") )   {
      // variable is inside a namespace, so put the scope before the name

      SrcLangExt lang = nd->getLanguage();
      QString sep  = getLanguageSpecificSeparator(lang);

      if (! root->type.isEmpty()) {

         bool isAlias = root->m_traits.hasTrait(Entry::Virtue::Alias);

         if (isAlias) { 
            // turn 'typedef B NS::A' into 'using NS::A = B'
            def = "using " + nd->name() + sep + name + " = " + root->type;

         } else { 
            // normal member
            def = root->type + " " + nd->name() + sep + name + root->args;
         }

      } else {
         def = nd->name() + sep + name + root->args;
      }

   } else {

      if (! root->type.isEmpty() && ! root->name.isEmpty()) { 
         if (name.at(0) == '@') { 
            // dummy variable representing anonymous union
            def = root->type;

         } else {
            bool isAlias = root->m_traits.hasTrait(Entry::Virtue::Alias);

            if (isAlias) { 
               // turn 'typedef B A' into 'using A = B'
               def = "using " + root->name + " = " + root->type.mid(7);

            } else { 
               // normal member
               def = root->type + " " + name + root->args;
            }
         }

      } else {
         def = name + root->args;
      }
   }

   def = stripPrefix(def, "static ");

   QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict->find(name);

   if (mn) {

      if (! scope.isEmpty()) {
         nd = getResolvedNamespace(scope);
      }

      for (auto md : *mn) {
         if ( ((nd == 0 && md->getNamespaceDef() == 0 && md->getFileDef() && root->fileName == md->getFileDef()->getFilePath())
             || (nd != 0 && md->getNamespaceDef() == nd) ) && !md->isDefine()  && !md->isEnumerate()  ) {

            // in C# an enum value and enum can have the same name
            // function style #define's can be "overloaded" by typedefs or variables
            // both variable names in the same file variable already in the scope
         
            bool isPHPArray = md->getLanguage() == SrcLangExt_PHP &&
                              md->argsString() != root->args && root->args.indexOf('[') != -1;

            bool staticsInDifferentFiles = root->stat && md->isStatic() && root->fileName != md->getDefFileName();

            if (md->getFileDef() && ! isPHPArray && ! staticsInDifferentFiles) {
               // not a php array
               // not a php array variable
            
               Debug::print(Debug::Variables, 0, "    variable already found: scope=%s\n", csPrintable(md->getOuterScope()->name()));

               addMemberDocs(rootNav, md, def, 0, false);
               md->setRefItems(root->sli);

               return md;
            }
         }
      }
   }

   QString fileName = root->fileName;

   if (fileName.isEmpty() && rootNav->tagInfo()) {
      fileName = rootNav->tagInfo()->tagName;
   }

   // new global variable, enum value, or typedef
   QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(fileName, root->startLine, root->startColumn, root->type, name,
                                              root->args, nullptr, Public, Normal, root->stat, Member, mtype,
                                              root->tArgLists ? &(root->tArgLists->last()) : nullptr, nullptr);
  
   md->setTagInfo(rootNav->tagInfo());
   md->setMemberTraits(root->m_traits);
   md->setDocumentation(root->doc, root->docFile, root->docLine);
   md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
   md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
   md->addSectionsToDefinition(root->anchors);
   md->setFromAnonymousScope(fromAnnScope);
   md->setFromAnonymousMember(fromAnnMemb);
   md->setInitializer(root->initializer);
   md->setMaxInitLines(root->initLines);
   md->setMemberGroupId(root->mGrpId);
   md->setDefinition(def);
   md->setLanguage(root->lang);
   md->setId(root->id);
   md->enableCallGraph(root->callGraph);
   md->enableCallerGraph(root->callerGraph);
   md->setExplicitExternal(root->explicitExternal);
   
   if (! root->explicitExternal) {
      md->setBodySegment(root->bodyLine, root->endBodyLine);
      md->setBodyDef(fd);
   }
   addMemberToGroups(root, md);

   md->setRefItems(root->sli);
   if (nd && ! nd->name().isEmpty() && nd->name().at(0) != '@') {
      md->setNamespace(nd);
      nd->insertMember(md);
   }

   // add member to the file, this is done even if already inserted it into the namespace
   if (fd) {
      md->setFileDef(fd);
      fd->insertMember(md);
   }

   // add member definition to the list of globals
   if (mn) {
      mn->append(md);

   } else {
      mn = QMakeShared<MemberName>(name);

      mn->append(md);
      Doxy_Globals::functionNameSDict->insert(name, mn);
   }

   rootNav->changeSection(Entry::EMPTY_SEC);

   return md;
}

/*! See if the return type string \a type is that of a function pointer
 *  \returns -1 if this is not a function pointer variable or
 *           the index at which the closing brace of (...*name) was found.
 */
int Doxy_Work::findFunctionPtr(const QString &type, int lang, int *pLength)
{
   if (lang == SrcLangExt_Fortran) {
      return -1;   // Fortran does not have function pointers
   }

   static const QRegExp re("\\([^)]*[\\*\\^][^)]*\\)");

   int i  = -1;
   int len;
   int bb = type.indexOf('<');
   int be = type.lastIndexOf('>');

   if (! type.isEmpty() && (i = re.indexIn(type, 0)) != -1 && type.indexOf("operator") == -1 && 
                  (type.indexOf(")(") == -1 || type.indexOf("typedef ") != -1) && ! (bb < i && i < be) ) {
      
      len = re.matchedLength();

      if (pLength) {
         *pLength = len;
      }

      return i;

   } else {
      return -1;
   }
}

/*! Returns true iff \a type is a class within scope \a context.
 *  Used to detect variable declarations that look like function prototypes.
 */
bool Doxy_Work::isVarWithConstructor(QSharedPointer<EntryNav> rootNav)
{
   static QRegExp initChars("[0-9\"'&*!^]+");
   static QRegExp idChars("[a-z_A-Z][a-z_A-Z0-9]*");

   bool result = false;
   bool typeIsClass;

   QString type;

   QSharedPointer<Definition> ctx;
   QSharedPointer<FileDef> fd;

   int ti;

   rootNav->loadEntry(Doxy_Globals::g_storage);
   QSharedPointer<Entry> root = rootNav->entry();

   bool outerBreak = false;

   do {

      if (rootNav->parent()->section() & Entry::COMPOUND_MASK) {
         // inside a class
         result = false;
         break;

      } else if ((fd = rootNav->fileDef()) && (fd->name().right(2) == ".c" || fd->name().right(2) == ".h") ) {
         // inside a .c file
         result = false;
         break;
      }

      if (root->type.isEmpty()) {
         result = false;
          break;
      }

      if (!rootNav->parent()->name().isEmpty()) {
         ctx = Doxy_Globals::namespaceSDict->find(rootNav->parent()->name());
      }

      type = root->type;

      // remove qualifiers
      findAndRemoveWord(type, "const");
      findAndRemoveWord(type, "static");
      findAndRemoveWord(type, "volatile");

      //if (type.left(6)=="const ") type=type.right(type.length()-6);
      typeIsClass = getResolvedClass(ctx, fd, type) != 0;

      if (!typeIsClass && (ti = type.indexOf('<')) != -1) {
         typeIsClass = getResolvedClass(ctx, fd, type.left(ti)) != 0;
      }

      if (typeIsClass) {
         // now we still have to check if the arguments are
         // types or values. Since we do not have complete type info
         // we need to rely on heuristics

         ArgumentList *al = &root->argList;
         if (al == 0 || al->isEmpty()) {
            result = false; // empty arg list -> function prototype.
            break;
         }

         for (auto &a : *al) {
            if (! a.name.isEmpty() || ! a.defval.isEmpty()) {

               if (initChars.indexIn(a.name) == 0) {
                  result = true;

               } else {
                  result = false; // arg has (type,name) pair -> function prototype

               }

               outerBreak = true;
               break;
            }

            if (a.type.isEmpty() || getResolvedClass(ctx, fd, a.type) != 0) {
               result = false; // arg type is a known type

               outerBreak = true;
               break;
            }

            if (checkIfTypedef(ctx, fd, a.type)) {
               result = false; // argument is a typedef

               outerBreak = true;
               break;
            }

            if (a.type.at(a.type.length() - 1) == '*' || a.type.at(a.type.length() - 1) == '&') {
               // type ends with * or & => pointer or reference
               result = false;

               outerBreak = true;
               break;
            }


            if (initChars.indexIn(a.type) == 0) {
               result = true; // argument type starts with typical initializer char

               outerBreak = true;
               break;
            }

            QString resType = resolveTypeDef(ctx, a.type);
            if (resType.isEmpty()) {
               resType = a.type;
            }

            int len;
            if (idChars.indexIn(resType, 0) == 0) {
               // resType starts with identifier
               len = idChars.matchedLength();

               resType = resType.left(len);

               if (resType == "int"    || resType == "long" || resType == "float" ||
                   resType == "double" || resType == "char" || resType == "signed" ||
                   resType == "const"  || resType == "unsigned" || resType == "void") {

                  result = false;

                  // type keyword -> function prototype
                  outerBreak = true;

                  break;
               }
            }
         }

         if (outerBreak) {
            break;
         }

         result = true;
      }

   } while(false);

   rootNav->releaseEntry();

   return result;
}

void Doxy_Work::addVariable(QSharedPointer<EntryNav> rootNav, int isFuncPtr)
{
   rootNav->loadEntry(Doxy_Globals::g_storage);
   QSharedPointer<Entry> root = rootNav->entry();

   Debug::print(Debug::Variables, 0, "VARIABLE_SEC: \n"
                "  type=`%s' name=`%s' args=`%s' bodyLine=`%d' mGrpId=%d relates=%s\n",
                csPrintable(root->type), csPrintable(root->name), csPrintable(root->args), root->bodyLine,
                root->mGrpId, csPrintable(root->relates) );

   if (root->type.isEmpty() && root->name.indexOf("operator") == -1 &&
         (root->name.indexOf('*') != -1 || root->name.indexOf('&') != -1)) {

      // recover from parse error caused by redundant braces
      // like in "int *(var[10]);", which is parsed as -->  type="" name="int *" args="(var[10])"

      root->type = root->name;
      static const QRegExp reName("[a-z_A-Z][a-z_A-Z0-9]*");

      int l = 0;
      int i = root->args.isEmpty() ? -1 : reName.indexIn(root->args, 0);
      l = reName.matchedLength();

      if (i != -1) {
         root->name = root->args.mid(i, l);
         root->args = root->args.mid(i + l, root->args.indexOf(')', i + l) - i - l);
      }

   } else {
      int i = isFuncPtr;

      bool isAlias = root->m_traits.hasTrait(Entry::Virtue::Alias);

      if (i == -1 && ! isAlias) {
         i = findFunctionPtr(root->type, root->lang);   // for typedefs isFuncPtr is not yet set
      }

      Debug::print(Debug::Variables, 0, "  functionPtr? %s\n", i != -1 ? "yes" : "no");
      if (i != -1) {
         // function pointer
         int ai = root->type.indexOf('[', i);

         if (ai > i) { 
            // function pointer array
            root->args.prepend(root->type.right(root->type.length() - ai));
            root->type = root->type.left(ai);

         } else if (root->type.indexOf(')', i) != -1) { // function ptr, not variable like "int (*bla)[10]"
            root->type = root->type.left(root->type.length() - 1);
            root->args.prepend(") ");            
         }

      } else if (root->type.indexOf("typedef ") != -1 && root->type.right(2) == "()") { // typedef void (func)(int)
         root->type = root->type.left(root->type.length() - 1);
         root->args.prepend(") ");
      }
   }

   QString scope;
   QString name = removeRedundantWhiteSpace(root->name);

   // find the scope of this variable
   QSharedPointer<EntryNav> p = rootNav->parent();

   while ((p->section() & Entry::SCOPE_MASK)) {
      QString scopeName = p->name();

      if (! scopeName.isEmpty()) {
         scope.prepend(scopeName);
         break;
      }

      p = p->parent();
   }

   MemberType mtype;
   QString type = root->type.trimmed();

   QSharedPointer<ClassDef> cd = QSharedPointer<ClassDef>();

   bool isRelated  = false;
   bool isMemberOf = false;

   QString classScope = stripAnonymousNamespaceScope(scope);
   classScope = stripTemplateSpecifiersFromScope(classScope, false);

   QString annScopePrefix = scope.left(scope.length() - classScope.length());

   if (root->name.lastIndexOf("::") != -1) {

      if (root->type == "friend class" || root->type == "friend struct" || root->type == "friend union") {
         cd = getClass(scope);

         if (cd) {
            // cd  is the class to add member to
            addVariableToClass(rootNav, cd, MemberType_Friend, name, false, QSharedPointer<MemberDef>(), Public, Member);
         }
      }

      goto nextMember;
      /* skip this member, because it is a
       * static variable definition (always?), which will be
       * found in a class scope as well, but then we know the
       * correct protection level, so only then it will be
       * inserted in the correct list!
       */
   }

   if (type == "@") {
      mtype = MemberType_EnumValue;

   } else if (type.left(8) == "typedef ") {
      mtype = MemberType_Typedef;

   } else if (type.left(7) == "friend ") {
      mtype = MemberType_Friend;

   } else if (root->mtype == Property) {
      mtype = MemberType_Property;

   } else if (root->mtype == Event) {
      mtype = MemberType_Event;

   } else {
      mtype = MemberType_Variable;
   }

   if (! root->relates.isEmpty()) { 
      // related variable

      isRelated = true;
      isMemberOf = (root->relatesType == MemberOf);

      if (getClass(root->relates) == 0 && ! scope.isEmpty()) {
         scope = mergeScopes(scope, root->relates);
      } else {
         scope = root->relates;
      }
   }

   cd = getClass(scope);

   if (cd == 0 && classScope != scope) {
      cd = getClass(classScope);
   }

   if (cd) {
      QSharedPointer<MemberDef> md;

      // if cd is an anonymous (=tag less) scope we insert the member
      // into a non-anonymous parent scope as well. This is needed to
      // be able to refer to it using \var or \fn

      int si = scope.indexOf('@');

      static bool inlineSimpleStructs = Config::getBool("inline-simple-struct");

      if (si != -1 && ! inlineSimpleStructs) { 
         // anonymous scope or type

         QString pScope;
         QSharedPointer<ClassDef> pcd = QSharedPointer<ClassDef>();
         pScope = scope.left(qMax(si - 2, 0)); // scope without tag less parts

         if (! pScope.isEmpty()) {
            pScope.prepend(annScopePrefix);

         } else if (annScopePrefix.length() > 2) {
            pScope = annScopePrefix.left(annScopePrefix.length() - 2);

         }

         if (name.at(0)!= '@') {

            if (! pScope.isEmpty() && (pcd = getClass(pScope))) {
               // pcd is the class to add the member to

               md = addVariableToClass(rootNav, pcd, mtype, name, true, QSharedPointer<MemberDef>(), 
                  root->protection, isMemberOf ? Foreign : isRelated ? Related : Member);
              
            } else { 
               // anonymous scope inside namespace or file => put variable in the global scope

               if (mtype == MemberType_Variable) {
                  md = addVariableToFile(rootNav, mtype, pScope, name, true, QSharedPointer<MemberDef>());
               }

            }
         }
      }

      // cd is the class to add the member to
      addVariableToClass(rootNav, cd,  mtype, name, false, md,   
                         root->protection, isMemberOf ? Foreign : isRelated ? Related : Member);

   } else if (!name.isEmpty()) {
      // global variable
      addVariableToFile(rootNav, mtype, scope, name, false, QSharedPointer<MemberDef>() );
   }

nextMember:
   rootNav->releaseEntry();
}

// Searches the Entry tree for typedef documentation sections.
// If found they are stored in their class or in the global list.
void Doxy_Work::buildTypedefList(QSharedPointer<EntryNav> rootNav)
{
   if (! rootNav->name().isEmpty() && rootNav->section() == Entry::VARIABLE_SEC &&
                  rootNav->type().indexOf("typedef ") != -1) {

      // it is a typedef
      addVariable(rootNav);
   }

   for (auto e : rootNav->children() ) {
      if (e->section() != Entry::ENUM_SEC) {
         buildTypedefList(e);
      }
   }
}

// Searches the Entry tree for Variable documentation sections.
// If found they are stored in their class or in the global list.
void Doxy_Work::buildVarList(QSharedPointer<EntryNav> rootNav)
{
   int isFuncPtr = -1;
   bool isVar    = false;

   if (! rootNav->name().isEmpty() ) {
     
      if (rootNav->type().isEmpty() || ! Doxy_Globals::g_compoundKeywordDict.contains(rootNav->type())) {
                    
         if (rootNav->section() == Entry::VARIABLE_SEC) { 
            isVar = true;  
    
         } else if (rootNav->section() == Entry::FUNCTION_SEC) {        
            isFuncPtr = findFunctionPtr(rootNav->type(), rootNav->lang()); 
   
            if (isFuncPtr != -1) {
               // might be a function pointer variable 
               isVar = true;

            } else if (isVarWithConstructor(rootNav)) {
               // class variable initialized by constructor 
               isVar = true;  

            }
         }      
      }
   }

   if (isVar) {
      // documented variable
      addVariable(rootNav, isFuncPtr);
   }
   
   for (auto e : rootNav->children() ) {
      if (e->section() != Entry::ENUM_SEC) {
         buildVarList(e);
      }
   }
}

void Doxy_Work::resolveHiddenNamespace()
{ 
   std::set<QString> visibleSet;
   std::map<QString, QSharedPointer<NamespaceDef>> hiddenMap;

   // are there any classes or members in this namespace
   for (auto nd : *Doxy_Globals::namespaceSDict) { 
      ClassSDict *cd  = nd->getClassSDict();          

      const QList<QSharedPointer<MemberList>> &list = nd->getMemberLists();
      bool hasMembers = false;      

      for (auto item : list)  { 
         if (item != nullptr && ! item->isEmpty())  {            
            hasMembers = true;
            break;
         }
      }

      if ( (cd == nullptr || cd->isEmpty()) && ! hasMembers ) {              
         hiddenMap.insert( std::make_pair(nd->name(), nd) );         
       
      } else  {         
         visibleSet.insert(nd->name());

      }    
   }

   // verify if the hidden ns should be visible
   for (auto item : hiddenMap) {

      auto begin = visibleSet.lower_bound(item.first + "::");
      auto end   = visibleSet.lower_bound(item.first + ":;");      // colon -- semicolon is correct

      if (begin == end) {         
         item.second->setHidden(true);            
      }      
   }
}

// Searches the Entry tree for Interface sections (UNO IDL only).
// If found they are stored in their service or in the global list.
void Doxy_Work::addInterfaceOrServiceToServiceOrSingleton(QSharedPointer<EntryNav> rootNav, QSharedPointer<ClassDef> cd, QString const &rname)
{
   QSharedPointer<Entry> root = rootNav->entry();
   QSharedPointer<FileDef> const fd = rootNav->fileDef();

   enum MemberType const type = (rootNav->section() == Entry::EXPORTED_INTERFACE_SEC)
                                ? MemberType_Interface : MemberType_Service;

   QString fileName = root->fileName;

   if (fileName.isEmpty() && rootNav->tagInfo()) {
      fileName = rootNav->tagInfo()->tagName;
   }

   QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(fileName, root->startLine, root->startColumn, root->type, rname,
         "", "", root->protection, root->virt, root->stat, Member, type, nullptr, &root->argList);

   md->setTagInfo(rootNav->tagInfo());
   md->setMemberClass(cd);
   md->setDocumentation(root->doc, root->docFile, root->docLine);
   md->setDocsForDefinition(false);
   md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
   md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
   md->setBodySegment(root->bodyLine, root->endBodyLine);
   md->setMemberTraits(root->m_traits);
   md->setMemberGroupId(root->mGrpId);
   md->setTypeConstraints(&root->typeConstr);
   md->setLanguage(root->lang);
   md->setBodyDef(fd);
   md->setFileDef(fd);
   md->addSectionsToDefinition(root->anchors);

   QString const def = root->type + " " + rname;
   md->setDefinition(def);
   md->enableCallGraph(root->callGraph);
   md->enableCallerGraph(root->callerGraph);

   Debug::print(Debug::Functions, 0, "  Interface Member:\n"
                "    `%s' `%s' proto=%d\n"
                "    def=`%s'\n", csPrintable(root->type), csPrintable(rname), root->proto, csPrintable(def));

   // add member to the global list of all members
   QSharedPointer<MemberName> mn;
   mn = Doxy_Globals::memberNameSDict->find(rname);

   if (mn) {
      mn->append(md);

   } else {
      mn = QMakeShared<MemberName>(rname);
      mn->append(md);
      Doxy_Globals::memberNameSDict->insert(rname, mn);
   }

   // add member to the class cd
   cd->insertMember(md);

   // also add the member as a "base" (to get nicer diagrams)
   // "optional" interface/service get Protected which turns into dashed line

   bool isOptional = root->m_traits.hasTrait(Entry::Virtue::Optional);

   BaseInfo base(rname, isOptional ? Protected : Public, Normal);

   if (! findClassRelation(rootNav, cd, cd, &base, 0, DocumentedOnly, true)) {
      findClassRelation(rootNav, cd, cd, &base, 0, Undocumented, true);
   }

   // add file to list of used files
   cd->insertUsedFile(fd);

   addMemberToGroups(root, md);
   rootNav->changeSection(Entry::EMPTY_SEC);
   md->setRefItems(root->sli);
}

void Doxy_Work::buildInterfaceAndServiceList(QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->section() == Entry::EXPORTED_INTERFACE_SEC || rootNav->section() == Entry::INCLUDED_SERVICE_SEC) {

      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<const Entry> root = rootNav->entry();

      Debug::print(Debug::Functions, 0,"EXPORTED_INTERFACE_SEC:\n"
                   "  `%s' `%s'::`%s' `%s' relates=`%s' relatesType=`%d' file=`%s' "
                   "line=`%d' bodyLine=`%d' #tArgLists=%d mGrpId=%d proto=%d docFile=%s\n",
                   csPrintable(root->type), csPrintable(rootNav->parent()->name()), csPrintable(root->name),
                   csPrintable(root->args), csPrintable(root->relates), root->relatesType,
                   csPrintable(root->fileName), root->startLine, root->bodyLine,
                   root->tArgLists ? (int)root->tArgLists->count() : -1,
                   root->mGrpId, root->proto, csPrintable(root->docFile) );

      QString const rname = removeRedundantWhiteSpace(root->name);

      if (! rname.isEmpty()) {
         QString const scope = rootNav->parent()->name();
         QSharedPointer<ClassDef> const cd = getClass(scope);
         assert(cd);

         if (cd && ((ClassDef::Interface == cd->compoundType()) || (ClassDef::Service   == cd->compoundType()) ||
                    (ClassDef::Singleton == cd->compoundType()))) {
            addInterfaceOrServiceToServiceOrSingleton(rootNav, cd, rname);

         } else {
            assert(false); // was checked by scanner.l
         }

      } else if (rname.isEmpty()) {
         warn(root->fileName, root->startLine, "Illegal member name found.");
      }

      rootNav->releaseEntry();
   }

   // can only have these in IDL anyway
   switch (rootNav->lang()) {
      case SrcLangExt_Unknown: // fall through (root node always is Unknown)
      case SrcLangExt_IDL:
         RECURSE_ENTRYTREE(buildInterfaceAndServiceList, rootNav);
         break;
      default:
         return; // nothing to do here
   }
}

// Searches the Entry tree for Function sections.
// If found they are stored in their class or in the global list.
void Doxy_Work::addMethodToClass(QSharedPointer<EntryNav> rootNav, QSharedPointer<ClassDef> cd, const QString &rname, bool isFriend)
{
   QSharedPointer<Entry> root = rootNav->entry();
   QSharedPointer<FileDef> fd = rootNav->fileDef();

   int l;
   static QRegExp re("\\([a-z_A-Z0-9: ]*[ &*]+[ ]*");
   int ts = root->type.indexOf('<');
   int te = root->type.lastIndexOf('>');

   int i = re.indexIn(root->type, 0);
   l = re.matchedLength();

   if (i != -1 && ts != -1 && ts < te && ts < i && i < te) {
      // avoid changing A<int(int*)>, see bug 677315
      i = -1;
   }

   bool isAlias = root->m_traits.hasTrait(Entry::Virtue::Alias);

   if (cd->getLanguage() == SrcLangExt_Cpp && ! root->type.isEmpty() && ! isAlias && i != -1) { 
      // only C has pointers, function variable

      root->args += root->type.right(root->type.length() - i - l);
      root->type = root->type.left(i + l);
   }

   QString name = removeRedundantWhiteSpace(rname);
   if (name.left(2) == "::") {
      name = name.right(name.length() - 2);
   }

   MemberType mtype;

   if (isFriend) {
      mtype = MemberType::MemberType_Friend;

   } else if (root->mtype == MethodTypes::Signal) {
      mtype = MemberType::MemberType_Signal;

   } else if (root->mtype == MethodTypes::Slot) {
      mtype = MemberType::MemberType_Slot;

   } else if (root->mtype == MethodTypes::DCOP) {
      mtype = MemberType::MemberType_DCOP;

   } else {
      mtype = MemberType::MemberType_Function;
   }

   // strip redundant template specifier for constructors
   if ((fd == 0 || fd->getLanguage() == SrcLangExt_Cpp) && name.left(9) != "operator " &&
         (i = name.indexOf('<')) != -1 && name.indexOf('>') != -1) {

      name = name.left(i);
   }

   QString fileName = root->fileName;

   if (fileName.isEmpty() && rootNav->tagInfo()) {
      fileName = rootNav->tagInfo()->tagName;
   }

   // adding class member
   QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(fileName, root->startLine, root->startColumn,
               root->type, name, root->args, root->exception, root->protection, root->virt,
               (root->stat && root->relatesType != MemberOf), 
               (root->relates.isEmpty() ? Member : root->relatesType == MemberOf ? Foreign : Related),
               mtype, root->tArgLists ? &root->tArgLists->last() : 0, &root->argList);

   md->setTagInfo(rootNav->tagInfo());
   md->setMemberClass(cd);
   md->setDocumentation(root->doc, root->docFile, root->docLine);
   md->setDocsForDefinition(! root->proto);
   md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
   md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
   md->setBodySegment(root->bodyLine, root->endBodyLine);
   md->setMemberTraits(root->m_traits);
   md->setMemberGroupId(root->mGrpId);
   md->setTypeConstraints(&root->typeConstr);
   md->setLanguage(root->lang);
   md->setId(root->id);
   md->setBodyDef(fd);
   md->setFileDef(fd);
   md->addSectionsToDefinition(root->anchors);

   QString def;
   QString qualScope = cd->qualifiedNameWithTemplateParameters();

   SrcLangExt lang = cd->getLanguage();
   QString scopeSeparator = getLanguageSpecificSeparator(lang);

   if (scopeSeparator != "::") {
      qualScope = substitute(qualScope, "::", scopeSeparator);
   }

   if (lang == SrcLangExt_PHP) {
      // for PHP we use Class::method and Namespace\method
      scopeSeparator = "::";
   }

   if (! root->relates.isEmpty() || isFriend || Config::getBool("hide-scope-names")) {

      if (! root->type.isEmpty()) {
         def = root->type + " " + name;

      } else {         
         def = name;         

      }

   } else {
      if (! root->type.isEmpty()) {
        def = root->type + " " + qualScope + scopeSeparator + name;
        
      } else {         
         def = qualScope + scopeSeparator + name;       

      }
   }

   if (def.left(7) == "friend ") {
      def = def.right(def.length() - 7);
   }

   md->setDefinition(def);
   md->enableCallGraph(root->callGraph);
   md->enableCallerGraph(root->callerGraph);
 
   // add member to the global list of all members
   QSharedPointer<MemberName> mn;
   mn = Doxy_Globals::memberNameSDict->find(name);

   if (mn) {
      mn->append(md);

   } else {
      mn = QMakeShared<MemberName>(name);
      mn->append(md);
      Doxy_Globals::memberNameSDict->insert(name, mn);
   }

   // add member to the class cd
   cd->insertMember(md);

   // add file to list of used files
   cd->insertUsedFile(fd);

   // add member to groups
   addMemberToGroups(root, md);
   rootNav->changeSection(Entry::EMPTY_SEC);
   md->setRefItems(root->sli);
}

void Doxy_Work::buildFunctionList(QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->section() == Entry::FUNCTION_SEC) {

      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();

      Debug::print(Debug::Functions, 0,"FUNCTION_SEC:\n"
                   "  `%s' `%s'::`%s' `%s' relates=`%s' relatesType=`%d' file=`%s' "
                   "line=`%d' bodyLine=`%d' #tArgLists=%d mGrpId=%d proto=%d docFile=%s\n",
                   csPrintable(root->type), csPrintable(rootNav->parent()->name()), csPrintable(root->name),
                   csPrintable(root->args), csPrintable(root->relates), root->relatesType, csPrintable(root->fileName),
                   root->startLine, root->bodyLine, root->tArgLists ? (int)root->tArgLists->count() : -1,
                   root->mGrpId, root->proto, csPrintable(root->docFile) );

      bool isFriend = root->type.indexOf("friend ") != -1;

      QString rname = removeRedundantWhiteSpace(root->name);
      QString scope = rootNav->parent()->name();                   // stripAnonymousNamespaceScope(root->parent->name);

      if (! rname.isEmpty() && scope.indexOf('@') == -1) {
         QSharedPointer<ClassDef> cd = QSharedPointer<ClassDef>();

         // check if this function's parent is a class
         scope = stripTemplateSpecifiersFromScope(scope, false);

         QSharedPointer<FileDef> rfd = rootNav->fileDef();
         int memIndex = rname.lastIndexOf("::");

         cd = getClass(scope);
         if (cd && scope + "::" == rname.left(scope.length() + 2)) { // found A::f inside A
            // strip scope from name
            rname = rname.right(rname.length() - rootNav->parent()->name().length() - 2);
         }

         QSharedPointer<NamespaceDef> nd;
         bool isMember = false;

         if (memIndex != -1) {
            int ts = rname.indexOf('<');
            int te = rname.indexOf('>');

            if (memIndex > 0 && (ts == -1 || te == -1)) {
               isMember = true;
            } else {
               isMember = memIndex < ts || memIndex > te;
            }
         }

         static QRegExp re("\\([a-z_A-Z0-9: ]*[ &*]+[ ]*");

         int ts = root->type.indexOf('<');
         int te = root->type.lastIndexOf('>');
         int ti;

         bool done = false;

         if (! rootNav->parent()->name().isEmpty() && (rootNav->parent()->section() & Entry::COMPOUND_MASK) && cd) {

            ti = re.indexIn(root->type, 0);
            bool tempA = (ts != -1 && ts < te && ts < ti && ti < te);

            if (root->type.isEmpty() || (ti == -1 || tempA ||
                          root->args.indexOf(")[") != -1) ||
                          root->type.indexOf(")(") != -1  ||
                          root->type.indexOf("operator") != -1 ||  cd->getLanguage() != SrcLangExt_Cpp) {

               // odd code to exclude function pointers, type does not contain ..(..*
               // outside of < ... >
               // and args not )[.. -> function pointer
               // type contains ..)(.. and not "operator"
               // language other than C
             
               addMethodToClass(rootNav, cd, rname, isFriend);
               done = true;
            }
         }

         if (done) {
            // all done

         } else if (! ((rootNav->parent()->section() & Entry::COMPOUND_MASK) || 
                    rootNav->parent()->section() == Entry::OBJCIMPL_SEC) &&
                    ! isMember && (root->relates.isEmpty() || root->relatesType == Duplicate) &&
                    root->type.left(7) != "extern " && root->type.left(8) != "typedef ") {

            // no member => unrelated function

            /* check the uniqueness of the function name in the file.
             * A file could contain a function prototype and a function definition
             * or even multiple function prototypes.
             */
            QSharedPointer<MemberName> mn;
            mn = Doxy_Globals::functionNameSDict->find(rname);

            bool found = false;            
            QSharedPointer<MemberDef> md;

            if (mn) {
               Debug::print(Debug::Functions, 0, "  --> function %s already found\n", csPrintable(rname));

               for (auto item : *mn) {

                  if (found) {
                     break;
                  }

                  QSharedPointer<NamespaceDef> mnd = item->getNamespaceDef();
                  QSharedPointer<NamespaceDef> rnd;

                  QString fullScope   = scope;
                  QString parentScope = rootNav->parent()->name();

                  if (! parentScope.isEmpty() && ! leftScopeMatch(parentScope, scope)) {
                     if (! scope.isEmpty()) {
                        fullScope.prepend("::");
                     }

                     fullScope.prepend(parentScope);
                  }

                  rnd = getResolvedNamespace(fullScope);
                  QSharedPointer<FileDef> mfd = item->getFileDef();
                  QString nsName, rnsName;

                  if (mnd) {
                     nsName = mnd->name();
                  }

                  if (rnd) {
                     rnsName = rnd->name();
                  }

                  ArgumentList *mdAl    = item->argumentList();
                  ArgumentList *mdTempl = item->templateArguments();

                  // in case of template functions, we need to check if the
                  // functions have the same number of template parameters
                  bool sameNumTemplateArgs = true;
                  bool matchingReturnTypes = true;

                  if (mdTempl != 0 && root->tArgLists) {
                     if (mdTempl->count() != root->tArgLists->last().count()) {
                        sameNumTemplateArgs = false;
                     }

                     if (item->typeString() != removeRedundantWhiteSpace(root->type)) {
                        matchingReturnTypes = false;
                     }
                  }

                  bool staticsInDifferentFiles = root->stat && item->isStatic() && root->fileName != item->getDefFileName();

                  bool tmp = matchArguments2(item->getOuterScope(), mfd, mdAl, 
                           rnd ? rnd : Doxy_Globals::globalScope, rfd, &root->argList, false);

                  if (tmp && sameNumTemplateArgs && matchingReturnTypes && ! staticsInDifferentFiles) {

                     QSharedPointer<GroupDef> gd;

                     if (! root->groups->isEmpty()) {
                        gd = Doxy_Globals::groupSDict->find(root->groups->first().groupname);
                     }

                     // see if we need to create a new member
                     found = (mnd && rnd && nsName == rnsName) ||        // members are in the same namespace
                             ((mnd == 0 && rnd == 0 && mfd != 0 &&       // no external reference and
                               mfd->getFilePath() == root->fileName));

                     // otherwise, allow a duplicate global member with the same argument list
                     if (! found && gd && gd == item->getGroupDef() && nsName == rnsName) {
                        // member is already in the group, so we don't want to add it again.
                        found = true;
                     }

                     if (found) {
                        // md is used after this for loop
                        md = item;

                        // merge argument lists
                        mergeArguments(mdAl, &root->argList, ! root->doc.isEmpty());

                        // merge documentation
                        if (item->documentation().isEmpty() && ! root->doc.isEmpty()) {
                           ArgumentList *argList = new ArgumentList;
                           stringToArgumentList(root->args, argList);

                           if (root->proto) {
                              item->setDeclArgumentList(argList);

                           } else {
                              item->setArgumentList(argList);

                           }
                        }

                        item->setDocumentation(root->doc, root->docFile, root->docLine);
                        item->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
                        item->setDocsForDefinition(! root->proto);

                        if (item->getStartBodyLine() == -1 && root->bodyLine != -1) {
                           item->setBodySegment(root->bodyLine, root->endBodyLine);
                           item->setBodyDef(rfd);
                        }

                        if (item->briefDescription().isEmpty() && ! root->brief.isEmpty()) {
                           item->setArgsString(root->args);
                        }

                        item->setBriefDescription(root->brief, root->briefFile, root->briefLine);

                        item->addSectionsToDefinition(root->anchors);

                        item->enableCallGraph(item->hasCallGraph() || root->callGraph);
                        item->enableCallerGraph(item->hasCallerGraph() || root->callerGraph);

                        // merge ingroup specifiers
                        if (item->getGroupDef() == 0 && root->groups->size() > 0) {
                           addMemberToGroups(root, item);

                        } else if (item->getGroupDef() != 0 && root->groups->count() == 0) {
                           root->groups->append(Grouping(item->getGroupDef()->name(), item->getGroupPri()));

                        }

                        // if item is a declaration and root is the corresponding
                        // definition, then turn item into a definition
                        if (item->isPrototype() && ! root->proto) {
                           item->setPrototype(false);
                        }
                     }
                  }
               }
            }

            if (! found) {
               /* global function is unique with respect to the file */
               Debug::print(Debug::Functions, 0, "  --> new function %s found\n", csPrintable(rname));

               // new global function
               ArgumentList *tArgList = root->tArgLists ? &root->tArgLists->last() : nullptr;
               QString name = removeRedundantWhiteSpace(rname);

               md = QMakeShared<MemberDef>(root->fileName, root->startLine, root->startColumn, root->type, name,
                  root->args, root->exception, root->protection, root->virt, root->stat, Member,
                  MemberType_Function, tArgList, &root->argList);

               md->setTagInfo(rootNav->tagInfo());
               md->setLanguage(root->lang);
               md->setId(root->id);

               md->setDocumentation(root->doc, root->docFile, root->docLine);
               md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
               md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
               md->setPrototype(root->proto);
               md->setDocsForDefinition(!root->proto);
               md->setTypeConstraints(&root->typeConstr);

               md->setBodySegment(root->bodyLine, root->endBodyLine);

               QSharedPointer<FileDef> fd = rootNav->fileDef();
               md->setBodyDef(fd);
               md->addSectionsToDefinition(root->anchors);
               md->setMemberTraits(root->m_traits);
               md->setMemberGroupId(root->mGrpId);

               // see if the function is inside a namespace that was not part of
               // the name already (in that case nd should be non-zero)

               if (nd == nullptr && rootNav->parent()->section() == Entry::NAMESPACE_SEC ) {
                  QString nscope = rootNav->parent()->name();

                  if (! nscope.isEmpty()) {
                     nd = getResolvedNamespace(nscope);
                  }
               }

               if (! scope.isEmpty()) {
                  QString sep = getLanguageSpecificSeparator(root->lang);

                  if (sep != "::") {
                     scope = substitute(scope, "::", sep);
                  }

                  scope += sep;
               }

               QString def;

               if (! root->type.isEmpty()) {
                  def = root->type + " " + scope + name;
                 
               } else {
                  def = scope + name;
                 
               }

               Debug::print(Debug::Functions, 0, "  Global Function:\n"
                            "    `%s' `%s'::`%s' `%s' proto=%d\n"
                            "    def=`%s'\n",
                            csPrintable(root->type), csPrintable(rootNav->parent()->name()), csPrintable(rname),
                            csPrintable(root->args), root->proto, csPrintable(def) );

               md->setDefinition(def);
               md->enableCallGraph(root->callGraph);
               md->enableCallerGraph(root->callerGraph);

               md->setRefItems(root->sli);

               if (nd && ! nd->name().isEmpty() && nd->name().at(0) != '@') {
                  // add member to namespace
                  md->setNamespace(nd);
                  nd->insertMember(md);            
               }

               if (fd) {
                  // add member to the file, done even if we have already inserted it into the namespace
                  md->setFileDef(fd);
                  fd->insertMember(md);
               }

               // add member to the list of file members
               QSharedPointer<MemberName> mn;
               mn = Doxy_Globals::functionNameSDict->find(name);

               if (mn) {
                  mn->append(md);

               } else {
                  mn = QMakeShared<MemberName>(name);   
                  mn->append(md);

                  Doxy_Globals::functionNameSDict->insert(name, mn);
               }

               addMemberToGroups(root, md);

               if (root->relatesType == Simple)  {
                  // if this is a relatesalso command, allow find Member to pick it up
               
                  rootNav->changeSection(Entry::EMPTY_SEC); 
                  // Otherwise we have finished with this entry
               }

            } else {
               QSharedPointer<FileDef> fd = rootNav->fileDef();

               if (fd) {
                  // add member to the file (we do this even if we have already
                  // inserted it into the namespace)
                  fd->insertMember(md);
               }
            }

         } else {
            Debug::print(Debug::Functions, 0, "  --> %s not processed\n", csPrintable(rname));

         }

      } else if (rname.isEmpty()) {
         warn(root->fileName, root->startLine, "Illegal member name found." );
      }

      rootNav->releaseEntry();
   }

   RECURSE_ENTRYTREE(buildFunctionList, rootNav);
}

void Doxy_Work::findFriends()
{
   for (auto fn : *Doxy_Globals::functionNameSDict) {
      // for each global function name

      QSharedPointer<MemberName> mn;
      mn = Doxy_Globals::memberNameSDict->find(fn->memberName());

      if (mn) {
         // there are members with the same name

         for (auto fmd : *fn) {

            for (auto mmd : *mn) {

               ArgumentList *mmdAl = mmd->argumentList();
               ArgumentList *fmdAl = fmd->argumentList();

               bool ok = mmd->isFriend() || (mmd->isRelated() && mmd->isFunction());

               if (ok &&  matchArguments2(mmd->getOuterScope(), mmd->getFileDef(), mmdAl,
                                          fmd->getOuterScope(), fmd->getFileDef(), fmdAl, true)) {

                   // if the member is related and the arguments match then the
                   // function is actually a friend.

                  mergeArguments(mmdAl, fmdAl);

                  if (! fmd->documentation().isEmpty()) {
                     mmd->setDocumentation(fmd->documentation(), fmd->docFile(), fmd->docLine());

                  } else if (!mmd->documentation().isEmpty()) {
                     fmd->setDocumentation(mmd->documentation(), mmd->docFile(), mmd->docLine());
                  }

                  if (mmd->briefDescription().isEmpty() && !fmd->briefDescription().isEmpty()) {
                     mmd->setBriefDescription(fmd->briefDescription(), fmd->briefFile(), fmd->briefLine());

                  } else if (!mmd->briefDescription().isEmpty() && !fmd->briefDescription().isEmpty()) {
                     fmd->setBriefDescription(mmd->briefDescription(), mmd->briefFile(), mmd->briefLine());
                  }

                  if (! fmd->inbodyDocumentation().isEmpty()) {
                     mmd->setInbodyDocumentation(fmd->inbodyDocumentation(), fmd->inbodyFile(), fmd->inbodyLine());

                  } else if (!mmd->inbodyDocumentation().isEmpty()) {
                     fmd->setInbodyDocumentation(mmd->inbodyDocumentation(), mmd->inbodyFile(), mmd->inbodyLine());

                  }
                  
                  if (mmd->getStartBodyLine() == -1 && fmd->getStartBodyLine() != -1) {
                     mmd->setBodySegment(fmd->getStartBodyLine(), fmd->getEndBodyLine());
                     mmd->setBodyDef(fmd->getBodyDef());
                   
                  } else if (mmd->getStartBodyLine() != -1 && fmd->getStartBodyLine() == -1) {
                     fmd->setBodySegment(mmd->getStartBodyLine(), mmd->getEndBodyLine());
                     fmd->setBodyDef(mmd->getBodyDef());
                     
                  }

                  mmd->setDocsForDefinition(fmd->isDocsForDefinition());

                  mmd->enableCallGraph(mmd->hasCallGraph() || fmd->hasCallGraph());
                  mmd->enableCallerGraph(mmd->hasCallerGraph() || fmd->hasCallerGraph());
                  fmd->enableCallGraph(mmd->hasCallGraph() || fmd->hasCallGraph());
                  fmd->enableCallerGraph(mmd->hasCallerGraph() || fmd->hasCallerGraph());
               }
            }
         }
      }
   }
}

void Doxy_Work::transferFunctionDocumentation()
{
   // find matching function declaration and definitions.
   for (auto mn : *Doxy_Globals::functionNameSDict) {

      /* find a matching function declaration and definition for this function */
      for (auto mdec : *mn ) {

         if (mdec->isPrototype() || (mdec->isVariable() && mdec->isExternal()) ) {
            for (auto mdef : *mn ) {
               combineDeclarationAndDefinition(mdec, mdef);
            }
         }
      }
   }
}

void Doxy_Work::transferFunctionReferences()
{
   for (auto mn : *Doxy_Globals::functionNameSDict ) {
      QSharedPointer<MemberDef> mdef;
      QSharedPointer<MemberDef> mdec;

      /* find a matching function declaration and definition for this function */
      for (auto md : *mn) {

         if (md->isPrototype()) {
            mdec = md;
         } else if (md->isVariable() && md->isExternal()) {
            mdec = md;
         }

         if (md->isFunction() && !md->isStatic() && !md->isPrototype()) {
            mdef = md;
         } else if (md->isVariable() && !md->isExternal() && !md->isStatic()) {
            mdef = md;
         }
      }

      if (mdef && mdec) {
         ArgumentList *mdefAl = mdef->argumentList();
         ArgumentList *mdecAl = mdec->argumentList();

         if ( matchArguments2(mdef->getOuterScope(), mdef->getFileDef(), mdefAl,
                              mdec->getOuterScope(), mdec->getFileDef(), mdecAl, true )) {

            /* match found */
            MemberSDict *defDict = mdef->getReferencesMembers();
            MemberSDict *decDict = mdec->getReferencesMembers();

            if (defDict != 0) {
               for (auto rmd : *defDict) {
                  if (decDict == 0 || decDict->find(rmd->name()) == 0) {
                     mdec->addSourceReferences(rmd);
                  }
               }
            }

            if (decDict != 0) {
               for (auto rmd : *decDict) {
                  if (defDict == 0 || defDict->find(rmd->name()) == 0) {
                     mdef->addSourceReferences(rmd);
                  }
               }
            }

            defDict = mdef->getReferencedByMembers();
            decDict = mdec->getReferencedByMembers();

            if (defDict != 0) {
               for (auto rmd : *defDict) {
                  if (decDict == 0 || decDict->find(rmd->name()) == 0) {
                     mdec->addSourceReferencedBy(rmd);
                  }
               }
            }

            if (decDict != 0) {
              for (auto rmd : *decDict) {
                  if (defDict == 0 || defDict->find(rmd->name()) == 0) {
                     mdef->addSourceReferencedBy(rmd);
                  }
               }
            }
         }
      }
   }
}

void Doxy_Work::transferRelatedFunctionDocumentation()
{
   // find match between function declaration and definition for related functions

   for (auto mn : *Doxy_Globals::functionNameSDict) {

      /* find a matching function declaration and definition for this function */
      for (auto md : *mn) {

         QSharedPointer<MemberName> rmn;
         rmn = Doxy_Globals::memberNameSDict->find(md->name());

         if (rmn) {
            // check if there is a member with the same name

            for (auto rmd : *rmn) {

               ArgumentList *mdAl  = md->argumentList();
               ArgumentList *rmdAl = rmd->argumentList();

               if ((rmd->isRelated() || rmd->isForeign()) &&
                     matchArguments2( md->getOuterScope(), md->getFileDef(), mdAl,
                                      rmd->getOuterScope(), rmd->getFileDef(), rmdAl, true)) {

                  // related function
                  if (rmd->relatedAlso()) {
                     md->setRelatedAlso(rmd->relatedAlso()); 

                  } else if (rmd->isForeign()) {
                     md->makeForeign();

                  } else {
                     md->makeRelated();

                  }
               }
            }
         }
      }
   }
}

/*! make a dictionary of all template arguments of class cd
 * that are part of the base class name.
 * Example: A template class A with template arguments <R,S,T>
 * that inherits from B<T,T,S> will have T and S in the dictionary.
 */
QHash<QString, int> *Doxy_Work::getTemplateArgumentsInName(ArgumentList *templateArguments, const QString &name)
{
   QHash<QString, int> *templateNames = new QHash<QString, int>();
   static QRegExp re("[a-z_A-Z][a-z_A-Z0-9:]*");

   if (templateArguments) {
      int count = 0;

      for (auto &item : *templateArguments) {

         int i;
         int len;
         int p = 0;

         while ((i = re.indexIn(name, p)) != -1) {
            len = re.matchedLength();

            QString n = name.mid(i, len);

            if (n == item.name) {
               if (! templateNames->contains(n)) {
                  templateNames->insert(n, count);
               }
            }

            p = i + len;
         }

         count++;
      }
   }

   return templateNames;
}

/*! Searches a class from within \a context and \a cd and returns its
 *  definition if found (otherwise 0 is returned).
 */
QSharedPointer<ClassDef> Doxy_Work::findClassWithinClassContext(QSharedPointer<Definition> context, QSharedPointer<ClassDef>cd, const QString &name)
{
   QSharedPointer<ClassDef> result = QSharedPointer<ClassDef>();

   if (cd == 0) {
      return result;
   }

   QSharedPointer<FileDef> fd = cd->getFileDef();

   if (context && cd != context) {
      result = getResolvedClass(context, QSharedPointer<FileDef>(), name, nullptr, 0, true, true);
   }

   if (result == 0) {
      result = getResolvedClass(cd, fd, name, 0, 0, true, true);
   }

   if (result == 0) { // try direct class, needed for namespaced classes imported via tag files (see bug624095)
      result = getClass(name);
   }

   if (result == 0 && (cd->getLanguage() == SrcLangExt_CSharp || cd->getLanguage() == SrcLangExt_Java) &&
         name.indexOf('<') != -1) {

      result = Doxy_Globals::genericsDict->find(name);
   }

   return result;
}

void Doxy_Work::findUsedClassesForClass(QSharedPointer<EntryNav> rootNav, QSharedPointer<Definition> context, QSharedPointer<ClassDef> masterCd,
               QSharedPointer<ClassDef> instanceCd, bool isArtificial, ArgumentList *actualArgs, QHash<QString, int> *templateNames)
{
   masterCd->visited = true;
   ArgumentList *formalArgs = masterCd->templateArguments();

   if (masterCd->memberNameInfoSDict()) {

      for (auto mni : *masterCd->memberNameInfoSDict()) {

         for (auto &mi : *mni) {
            QSharedPointer<MemberDef> md = mi.memberDef;

            if (md->isVariable() || md->isObjCProperty()) {
               // for each member variable in this class

               QString type = normalizeNonTemplateArgumentsInString(md->typeString(), masterCd, formalArgs);
               QString typedefValue = resolveTypeDef(masterCd, type);

               if (! typedefValue.isEmpty()) {
                  type = typedefValue;
               }

               int pos = 0;
               QString usedClassName;
               QString templSpec;

               bool found = false;

               // the type can contain template variables, replace them if present
               if (actualArgs) {
                  type = substituteTemplateArgumentsInString(type, formalArgs, actualArgs);
               }

               while (! found && extractClassNameFromType(type, pos, usedClassName, templSpec, rootNav->lang()) != -1) {

                  // find the type (if any) that matches usedClassName
                  QSharedPointer<ClassDef> typeCd = getResolvedClass(masterCd, 
                        masterCd->getFileDef(), usedClassName, 0, 0, false, true);

                  if (typeCd) {
                     usedClassName = typeCd->name();
                  }

                  int sp = usedClassName.indexOf('<');
                  if (sp == -1) {
                     sp = 0; 
                  }

                  int si = usedClassName.lastIndexOf("::", sp);
                  if (si != -1) {
                     // replace any namespace aliases
                     replaceNamespaceAliases(usedClassName, si);
                  }

                  // add any template arguments to the class
                  QString usedName = removeRedundantWhiteSpace(usedClassName + templSpec);

                  bool delTempNames = false;

                  if (templateNames == 0) {
                     templateNames = getTemplateArgumentsInName(formalArgs, usedName);
                     delTempNames = true;
                  }

                  BaseInfo bi(usedName, Public, Normal);
                  findClassRelation(rootNav, context, instanceCd, &bi, templateNames, TemplateInstances, isArtificial);

                  if (masterCd->templateArguments()) {
                     int count = 0;

                     for (auto &arg : *masterCd->templateArguments()) {
                        if (arg.name == usedName) {
                           // type is a template argument
                           found = true;
                           Debug::print(Debug::Classes, 0, "    New used class `%s'\n", csPrintable(usedName));

                           QSharedPointer<ClassDef> usedCd = Doxy_Globals::hiddenClasses->find(usedName);

                           if (! usedCd) {
                              usedCd = QMakeShared<ClassDef>(masterCd->getDefFileName(), masterCd->getDefLine(),
                                       masterCd->getDefColumn(), usedName, ClassDef::Class);

                              usedCd->makeTemplateArgument();
                              usedCd->setUsedOnly(true);
                              usedCd->setLanguage(masterCd->getLanguage());

                              Doxy_Globals::hiddenClasses->insert(usedName, usedCd);
                           }

                           if (isArtificial) {
                              usedCd->setArtificial(true);
                           }

                           Debug::print(Debug::Classes, 0, "      Adding used class `%s' (1)\n", csPrintable(usedCd->name()));
                           instanceCd->addUsedClass(usedCd, md->name(), md->protection());
                           usedCd->addUsedByClass(instanceCd, md->name(), md->protection());
                        }
                     }
                  }

                  if (! found) {
                     QSharedPointer<ClassDef> usedCd = findClassWithinClassContext(context, masterCd, usedName);

                     if (usedCd) {
                        found = true;

                        Debug::print(Debug::Classes, 0, "    Adding used class `%s' (2)\n", csPrintable(usedCd->name()));

                        instanceCd->addUsedClass(usedCd, md->name(), md->protection()); // class exists
                        usedCd->addUsedByClass(instanceCd, md->name(), md->protection());
                     }
                  }

                  if (delTempNames) {
                     delete templateNames;
                     templateNames = 0;
                  }
               }

               if (! found && ! type.isEmpty()) { 
                  // used class is not documented in any scope
                  QSharedPointer<ClassDef> usedCd = Doxy_Globals::hiddenClasses->find(type);

                  if (usedCd == 0 && ! Config::getBool("hide-undoc-relations")) {
                     if (type.right(2) == "(*" || type.right(2) == "(^") { 
                        // type is a function pointer
                        type += md->argsString();
                     }

                     Debug::print(Debug::Classes, 0, "  New undocumented used class `%s'\n", csPrintable(type));

                     usedCd = QMakeShared<ClassDef>(masterCd->getDefFileName(), masterCd->getDefLine(),
                        masterCd->getDefColumn(), type, ClassDef::Class);

                     usedCd->setUsedOnly(true);
                     usedCd->setLanguage(masterCd->getLanguage());
                     Doxy_Globals::hiddenClasses->insert(type, usedCd);
                  }

                  if (usedCd) {
                     if (isArtificial) {
                        usedCd->setArtificial(true);
                     }

                     Debug::print(Debug::Classes, 0, "    Adding used class `%s' (3)\n", csPrintable(usedCd->name()));
                     instanceCd->addUsedClass(usedCd, md->name(), md->protection());
                     usedCd->addUsedByClass(instanceCd, md->name(), md->protection());
                  }
               }
            }
         }
      }
   }
}

void Doxy_Work::findBaseClassesForClass(QSharedPointer<EntryNav> rootNav, QSharedPointer<Definition> context, QSharedPointer<ClassDef> masterCd, 
             QSharedPointer<ClassDef>instanceCd, FindBaseClassRelation_Mode mode, bool isArtificial, ArgumentList *actualArgs, 
             QHash<QString, int> *templateNames)
{
   QSharedPointer<Entry> root = rootNav->entry();

   masterCd->visited = true;

   // base class could also be a non-nested class
   ArgumentList *formalArgs = masterCd->templateArguments();

   for (auto bi : root->extends) {
      bool delTempNames = false;

      if (templateNames == 0) {
         templateNames = getTemplateArgumentsInName(formalArgs, bi.name);
         delTempNames  = true;
      }

      BaseInfo tbi(bi.name, bi.prot, bi.virt);

      if (actualArgs) { 
         // substitute the formal template arguments of the base class
         tbi.name = substituteTemplateArgumentsInString(bi.name, formalArgs, actualArgs);
      }

      if (mode == DocumentedOnly) {
         // find a documented base class in the correct scope

         if (! findClassRelation(rootNav, context, instanceCd, &tbi, templateNames, DocumentedOnly, isArtificial)) {
            // decided to show inheritance relations even if not documented,
            // make them artificial, so they do not appear in the index
           
            bool b = Config::getBool("hide-undoc-relations") ? true : isArtificial;

            // no documented base class -> try to find an undocumented one
            findClassRelation(rootNav, context, instanceCd, &tbi, templateNames, Undocumented, b);

         }

      } else if (mode == TemplateInstances) {
         findClassRelation(rootNav, context, instanceCd, &tbi, templateNames, TemplateInstances, isArtificial);

      }

      if (delTempNames) {
         delete templateNames;
         templateNames = 0;
      }
   }
}

bool Doxy_Work::findTemplateInstanceRelation(QSharedPointer<Entry> root, QSharedPointer<Definition> context, QSharedPointer<ClassDef> templateClass, 
               const QString &templSpec, QHash<QString, int> *templateNames, bool isArtificial)
{
   Debug::print(Debug::Classes, 0, "    derived from template %s with parameters %s\n",
                csPrintable(templateClass->name()), csPrintable(templSpec));

   bool existingClass = (templSpec == tempArgListToString(templateClass->templateArguments(), root->lang) );

   if (existingClass) {
      return true;
   }

   bool freshInstance = false;
   QSharedPointer<ClassDef> instanceClass = templateClass->insertTemplateInstance(
                                            root->fileName, root->startLine, root->startColumn, templSpec, freshInstance);
   if (isArtificial) {
      instanceClass->setArtificial(true);
   }

   instanceClass->setLanguage(root->lang);

   if (freshInstance) {
      Debug::print(Debug::Classes, 0, "      found fresh instance '%s'\n", csPrintable(instanceClass->name()));
      Doxy_Globals::classSDict->insert(instanceClass->name(), instanceClass);

      instanceClass->setTemplateBaseClassNames(templateNames);

      // search for new template instances caused by base classes of instanceClass
      QSharedPointer<EntryNav> templateRootNav = Doxy_Globals::g_classEntries.value(templateClass->name());

      if (templateRootNav) {
         bool unloadNeeded = false;
         QSharedPointer<Entry> templateRoot = templateRootNav->entry();

         if (templateRoot == 0) { 
            // not yet loaded
            templateRootNav->loadEntry(Doxy_Globals::g_storage);
            templateRoot = templateRootNav->entry();

            assert(templateRoot != 0); // now it should really be loaded
            unloadNeeded = true;
         }

         Debug::print(Debug::Classes, 0, "        template root found %s templSpec=%s\n",
                      csPrintable(templateRoot->name), csPrintable(templSpec));

         ArgumentList *templArgs = new ArgumentList;
         stringToArgumentList(templSpec, templArgs);

         findBaseClassesForClass(templateRootNav, context, templateClass, instanceClass,
                                 TemplateInstances, isArtificial, templArgs, templateNames);

         findUsedClassesForClass(templateRootNav, context, templateClass, instanceClass,
                                 isArtificial, templArgs, templateNames);
         delete templArgs;

         if (unloadNeeded) { 
            // still cleanup to do
            templateRootNav->releaseEntry();
         }

      } else {
         Debug::print(Debug::Classes, 0, "        no template root entry found\n");
         // TODO: what happened if we get here?
      }
     
   } else {
      Debug::print(Debug::Classes, 0, "      instance already exists\n");
   }
   return true;
}

bool Doxy_Work::isRecursiveBaseClass(const QString &scope, const QString &name)
{
   QString n = name;
   int index = n.indexOf('<');

   if (index != -1) {
      n = n.left(index);
   }

   bool result = rightScopeMatch(scope, n);
   return result;
}

/*! Searches for the end of a template in prototype \a s starting from
 *  character position \a startPos. If the end was found the position
 *  of the closing \> is returned, otherwise -1 is returned.
 *
 *  Handles exotic cases such as
 *  \code
 *    Class<(id<0)>
 *    Class<bits<<2>
 *    Class<"<">
 *    Class<'<'>
 *    Class<(")<")>
 *  \endcode
 */
int Doxy_Work::findEndOfTemplate(const QString &s, int startPos)
{
   // locate end of template
   int e   = startPos;
   int len = s.length();

   int brCount    = 1;
   int roundCount = 0;
   
   bool insideString = false;
   bool insideChar   = false;

   int pc = 0;

   while (e < len && brCount != 0) {

      int c = s.at(e).unicode();

      switch (c) {
         case '<':
            if (!insideString && !insideChar) {
               if (e < len - 1 && s.at(e + 1) == '<') {
                  e++;
               } else if (roundCount == 0) {
                  brCount++;
               }
            }
            break;

         case '>':
            if (!insideString && !insideChar) {
               if (e < len - 1 && s.at(e + 1) == '>') {
                  e++;
               } else if (roundCount == 0) {
                  brCount--;
               }
            }
            break;

         case '(':
            if (!insideString && !insideChar) {
               roundCount++;
            }
            break;

         case ')':
            if (!insideString && !insideChar) {
               roundCount--;
            }
            break;

         case '"':
            if (!insideChar) {
               if (insideString && pc != '\\') {
                  insideString = false;
               } else {
                  insideString = true;
               }
            }
            break;

         case '\'':
            if (!insideString) {
               if (insideChar && pc != '\\') {
                  insideChar = false;
               } else {
                  insideChar = true;
               }
            }
            break;
      }

      pc = c;
      e++;
   }

   return brCount == 0 ? e : -1;
}

bool Doxy_Work::findClassRelation(QSharedPointer<EntryNav> rootNav, QSharedPointer<Definition> context, QSharedPointer<ClassDef> cd, BaseInfo *bi,
                                  QHash<QString, int> *templateNames, FindBaseClassRelation_Mode mode, bool isArtificial)
{
   QSharedPointer<Entry> root = rootNav->entry();

   QString biName = bi->name;
   bool isExplicitGlobalScope = false;

   if (biName.left(2) == "::") { 
      // explicit global scope
      biName = biName.right(biName.length() - 2);
      isExplicitGlobalScope = true;
   }

   QSharedPointer<EntryNav> parentNode = rootNav->parent();
   bool lastParent = false;

   // for each parent scope, starting with the largest scope
   // (in case of nested classes)

   do {
      QString scopeName;

      if (parentNode) {
         scopeName =  parentNode->name();
      }

      int scopeOffset = isExplicitGlobalScope ? 0 : scopeName.length();

      do {
         // try all parent scope prefixes, starting with the largest scope         
         QString baseClassName = biName;

         if (scopeOffset > 0) {
            baseClassName.prepend(scopeName.left(scopeOffset) + "::");
         }

         QSharedPointer<MemberDef> baseClassTypeDef;     
         QString templSpec;

         QSharedPointer<Definition> scope; 

         if (isExplicitGlobalScope) { 
            scope = Doxy_Globals::globalScope;
         } else {
            scope = context;
         }   
        
         QSharedPointer<ClassDef> baseClass = getResolvedClass(scope, cd->getFileDef(), baseClassName, &baseClassTypeDef,
                                                               &templSpec, mode == Undocumented, true);

         if (! isRecursiveBaseClass(rootNav->name(), baseClassName) || isExplicitGlobalScope
               || (rootNav->lang() == SrcLangExt_IDL &&
                   (rootNav->section() == Entry::EXPORTED_INTERFACE_SEC ||
                    rootNav->section() == Entry::INCLUDED_SERVICE_SEC))) {

               // isRecursiveBaseClass is always true for UNO IDL ifc/svc members
               // (i.e. this is needed for addInterfaceOrServiceToServiceOrSingleton)

            Debug::print(Debug::Classes, 0, "    class relation %s inherited/used by %s found (%s and %s) templSpec='%s'\n",
               csPrintable(baseClassName), csPrintable(rootNav->name()),
               (bi->prot == Private) ? "private" : ((bi->prot == Protected) ? "protected" : "public"),
               (bi->virt == Normal) ? "normal" : "virtual", csPrintable(templSpec) );

            int i  = baseClassName.indexOf('<');   
         
            if (i == -1) {
               i = baseClassName.length() - 1;
            }

            int si = baseClassName.lastIndexOf("::", i);

            if (si == -1) {
               si = 0;
            }

            if (baseClass == 0 && (root->lang == SrcLangExt_CSharp || root->lang == SrcLangExt_Java)) {
               // for Java/C# strip the template part before looking for matching

               baseClass = Doxy_Globals::genericsDict->find(baseClassName.left(i));
            }

            if (baseClass == 0 && i != -1) {
               // base class has template specifiers

               // TODO: here we should try to find the correct template specialization
               // but for now, we only look for the unspecializated base class.

               int e = findEndOfTemplate(baseClassName, i + 1);

               if (e != -1) {
                  // end of template was found at e

                  templSpec = removeRedundantWhiteSpace(baseClassName.mid(i, e - i));
                  baseClassName = baseClassName.left(i) + baseClassName.right(baseClassName.length() - e);

                  QSharedPointer<Definition> scope; 

                  if (isExplicitGlobalScope) { 
                     scope = Doxy_Globals::globalScope;
                  } else {
                     scope = context;
                  }   

                  baseClass = getResolvedClass(scope, cd->getFileDef(), baseClassName, &baseClassTypeDef, 
                                               0, mode == Undocumented, true);
               }

            } else if (baseClass && !templSpec.isEmpty())  {
               // we have a known class, but also
               // know it is a template, so see if
               // we can also link to the explicit
               // instance (for instance if a class
               // derived from a template argument)

               QSharedPointer<ClassDef> templClass = getClass(baseClass->name() + templSpec);

               if (templClass) {
                  // use the template instance instead of the template base.
                  baseClass = templClass;
                  templSpec.resize(0);
               }
            }

            bool found = baseClass != 0 && (baseClass != cd || mode == TemplateInstances);

            if (! found && si != -1) {
               QString tmpTemplSpec;

               // replace any namespace aliases
               replaceNamespaceAliases(baseClassName, si);

               QSharedPointer<Definition> scope; 

               if (isExplicitGlobalScope) { 
                  scope = Doxy_Globals::globalScope;
               } else {
                  scope = context;
               }   

               baseClass = getResolvedClass(scope, cd->getFileDef(), baseClassName, &baseClassTypeDef, 
                                            &tmpTemplSpec, mode == Undocumented, true);

               found = (baseClass != 0 && baseClass != cd);

               if (found) {
                  templSpec = tmpTemplSpec;
               }
            }

            if (! found) {
               baseClass = findClassWithinClassContext(context, cd, baseClassName);
               found = (baseClass != 0 && baseClass != cd);

            }

            if (! found) {
               // for PHP the "use A\B as C" construct map class C to A::B, so we lookup
               // the class name also in the alias mapping.

               if (Doxy_Globals::namespaceAliasDict.contains(baseClassName)) {
                  // see if it is indeed a class
                  QString aliasName = Doxy_Globals::namespaceAliasDict[baseClassName];

                  baseClass = getClass(aliasName);
                  found = baseClass != 0 && baseClass != cd;
               }
            }

            bool isATemplateArgument;

            if (templateNames != 0 && templateNames->contains(biName)) {
               isATemplateArgument = true;

            } else {
               isATemplateArgument = false;

            }

            // make templSpec canonical
            // warning: the following line doesn't work for Mixin classes (see bug 560623)
            // templSpec = getCanonicalTemplateSpec(cd, cd->getFileDef(), templSpec);

            if (found) { 

               Debug::print(Debug::Classes, 0, "    Documented base class `%s' templSpec=%s\n", 
                           csPrintable(biName), templSpec.isEmpty() ? "" : csPrintable(templSpec) );

               // add base class to this class

               // if templSpec is not empty then we should "instantiate"
               // the template baseClass. A new ClassDef should be created
               // to represent the instance. To be able to add the (instantiated)
               // members and documentation of a template class
               // (inserted in that template class at a later stage),
               // the template should know about its instances.
               // the instantiation process, should be done in a recursive way,
               // since instantiating a template may introduce new inheritance
               // relations.

               if (!templSpec.isEmpty() && mode == TemplateInstances) {
                  // if baseClass is actually a typedef then we should not
                  // instantiate it, since typedefs are in a different namespace
                  // see bug531637 for an example where this would otherwise hang the program

                  if (baseClassTypeDef == 0) {
                     findTemplateInstanceRelation(root, context, baseClass, templSpec, templateNames, isArtificial);
                  }

               } else if (mode == DocumentedOnly || mode == Undocumented) {
                  //printf("       => insert base class\n");
                  QString usedName;

                  if (baseClassTypeDef || cd->isCSharp()) {
                     usedName = biName;
                  }

                  static bool sipSupport = Config::getBool("sip-support");
                  if (sipSupport) {
                     bi->prot = Public;
                  }

                  if (! cd->isSubClass(baseClass)) {
                     // check for recursion, see bug690787
                     cd->insertBaseClass(baseClass, usedName, bi->prot, bi->virt, templSpec);

                     // add this class as super class to the base class
                     baseClass->insertSubClass(cd, bi->prot, bi->virt, templSpec);

                  } else {
                     warn(root->fileName, root->startLine, "Detected potential recursive class relation "
                          "between class %s and base class %s!", csPrintable(cd->name()), csPrintable(baseClass->name()) );
                  }
               }

               return true;

            } else if (mode == Undocumented && (scopeOffset == 0 || isATemplateArgument)) {
               Debug::print(Debug::Classes, 0,
                            "    New undocumented base class `%s' baseClassName=%s templSpec=%s isArtificial=%d\n",
                            csPrintable(biName), csPrintable(baseClassName), csPrintable(templSpec), isArtificial);

               baseClass = QSharedPointer<ClassDef>();

               if (isATemplateArgument) {
                  baseClass = Doxy_Globals::hiddenClasses->find(baseClassName);

                  if (baseClass == 0) {
                     baseClass = QMakeShared<ClassDef>(root->fileName, root->startLine, root->startColumn,
                                             baseClassName, ClassDef::Class);

                     Doxy_Globals::hiddenClasses->insert(baseClassName, baseClass);

                     if (isArtificial) {
                        baseClass->setArtificial(true);
                     }

                     baseClass->setLanguage(root->lang);
                  }

               } else {
                  baseClass = Doxy_Globals::classSDict->find(baseClassName);

                  if (baseClass == 0) {
                     baseClass = QMakeShared<ClassDef>(root->fileName, root->startLine, root->startColumn,
                                             baseClassName, ClassDef::Class);

                     Doxy_Globals::classSDict->insert(baseClassName, baseClass);

                     if (isArtificial) {
                        baseClass->setArtificial(true);
                     }

                     baseClass->setLanguage(root->lang);
                     int si = baseClassName.lastIndexOf("::");

                     if (si != -1) { 
                        // class is nested
                        QSharedPointer<Definition> sd = findScopeFromQualifiedName(Doxy_Globals::globalScope,
                                 baseClassName.left(si), QSharedPointer<FileDef>(), rootNav->tagInfo());

                        if (! sd || sd == Doxy_Globals::globalScope) {
                           // outer scope not found
                           baseClass->setArtificial(true); // see bug678139
                        }
                     }
                  }
               }

               if (biName.right(2) == "-p") {
                  biName = "<" + biName.left(biName.length() - 2) + ">";
               }

               // add base class to this class
               cd->insertBaseClass(baseClass, biName, bi->prot, bi->virt, templSpec);

               // add this class as super class to the base class
               baseClass->insertSubClass(cd, bi->prot, bi->virt, templSpec);

               // the undocumented base was found in this file
               baseClass->insertUsedFile(rootNav->fileDef());
               baseClass->setOuterScope(Doxy_Globals::globalScope);

               if (baseClassName.right(2) == "-p") {
                  baseClass->setCompoundType(ClassDef::Protocol);
               }

               return true;

            } else {
               Debug::print(Debug::Classes, 0, "    Base class `%s' not found\n", csPrintable(biName));
            }

         } else {
            if (mode != TemplateInstances) {
               warn(root->fileName, root->startLine, "Detected potential recursive class relation "
                    "between class %s and base class %s\n", csPrintable(root->name), csPrintable(baseClassName));
            }

            // for mode==TemplateInstance this case is quite common and
            // indicates a relation between a template class and a template
            // instance with the same name.
         }
         if (scopeOffset == 0) {
            scopeOffset = -1;

         } else if ((scopeOffset = scopeName.lastIndexOf("::", scopeOffset - 1)) == -1) {
            scopeOffset = 0;
         }

      } while (scopeOffset >= 0);

      if (parentNode == 0) {
         lastParent = true;

      } else {
         parentNode = parentNode->parent();
      }

   } while (lastParent);

   return false;
}

// Computes the base and super classes for each class in the tree
bool Doxy_Work::isClassSection(QSharedPointer<EntryNav> rootNav)
{
   if ( ! rootNav->name().isEmpty() ) {
      if (rootNav->section() & Entry::COMPOUND_MASK) {
         // is it a compound (class, struct, union, interface ...)
         return true;

      } else if (rootNav->section() & Entry::COMPOUNDDOC_MASK)  {
         // is it a documentation block with inheritance info.

         rootNav->loadEntry(Doxy_Globals::g_storage);
         QSharedPointer<Entry> root = rootNav->entry();

         bool extends = root->extends.count() > 0;
         rootNav->releaseEntry();

         if (extends) {
            return true;
         }
      }
   }
   return false;
}

/*! Builds a dictionary of all entry nodes in the tree starting with \a root
 */
void Doxy_Work::findClassEntries(QSharedPointer<EntryNav> rootNav)
{
   if (isClassSection(rootNav)) {
      Doxy_Globals::g_classEntries.insert(rootNav->name(), rootNav);
   }

   RECURSE_ENTRYTREE(findClassEntries, rootNav);
}

QString Doxy_Work::extractClassName(QSharedPointer<EntryNav> rootNav)
{
   // strip any anonymous scopes first

   QString bName = stripAnonymousNamespaceScope(rootNav->name());
   bName = stripTemplateSpecifiersFromScope(bName);

   int i;

   if ((rootNav->lang() == SrcLangExt_CSharp || rootNav->lang() == SrcLangExt_Java) &&
         (i = bName.indexOf('<')) != -1) {
      // a Java/C# generic class looks like a C++ specialization, so we need to strip the
      // template part before looking for matches
      bName = bName.left(i);
   }

   return bName;
}

/*! Using the dictionary build by findClassEntries(), this
 *  function will look for additional template specialization that
 *  exists as inheritance relations only. These instances will be
 *  added to the template they are derived from.
 */
void Doxy_Work::findInheritedTemplateInstances()
{
   for (auto item : *Doxy_Globals::classSDict) {
      item->visited = false;
   }

   for (auto rootNav : Doxy_Globals::g_classEntries) {
      QSharedPointer<ClassDef> cd;
      QString bName = extractClassName(rootNav);

      Debug::print(Debug::Classes, 0, "  Inheritance: Class %s : \n", csPrintable(bName));

      cd = getClass(bName);

      if (cd) {
         rootNav->loadEntry(Doxy_Globals::g_storage);
         findBaseClassesForClass(rootNav, cd, cd, cd, TemplateInstances, false);
         rootNav->releaseEntry();
      }
   }
}

void Doxy_Work::findUsedTemplateInstances()
{
   for (auto item : *Doxy_Globals::classSDict) {
      item->visited = false;
   }

   for (auto rootNav : Doxy_Globals::g_classEntries) {
      QSharedPointer<ClassDef> cd;

      QString bName = extractClassName(rootNav);

      Debug::print(Debug::Classes, 0, "  Usage: Class %s : \n", csPrintable(bName));

      cd = getClass(bName);

      if (cd) {
         rootNav->loadEntry(Doxy_Globals::g_storage);

         findUsedClassesForClass(rootNav, cd, cd, cd, true);
         cd->addTypeConstraints();

         rootNav->releaseEntry();
      }
   }
}

void Doxy_Work::computeClassRelations()
{
   static bool extractLocalClass = Config::getBool("extract-local-classes");
   static bool hideUndocClasses  = Config::getBool("hide-undoc-classes");

   for (auto item : *Doxy_Globals::classSDict) {
      item->visited = false;
   }

   for (auto rootNav : Doxy_Globals::g_classEntries) {
      QSharedPointer<ClassDef> cd;

      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();

      QString bName = extractClassName(rootNav);
      Debug::print(Debug::Classes, 0, "  Relations: Class %s : \n", csPrintable(bName));

      cd = getClass(bName);

      if (cd) {
         findBaseClassesForClass(rootNav, cd, cd, cd, DocumentedOnly, false);
      }

      int numMembers = cd && cd->memberNameInfoSDict() ? cd->memberNameInfoSDict()->count() : 0;

      if ((cd == 0 || (!cd->hasDocumentation() && !cd->isReference())) && numMembers > 0 && bName.right(2) != "::") {

         if (! root->name.isEmpty() && root->name.indexOf('@') == -1 && 
               (determineSection(root->fileName) == Entry::HEADER_SEC || extractLocalClass) && 
                protectionLevelVisible(root->protection) && ! hideUndocClasses )

            warn_undoc(root->fileName, root->startLine, "Compound %s was not documented", csPrintable(root->name));
      }

      rootNav->releaseEntry();
   }
}

void Doxy_Work::computeTemplateClassRelations()
{
   for (auto rootNav : Doxy_Globals::g_classEntries) {
      rootNav->loadEntry(Doxy_Globals::g_storage);

      QSharedPointer<Entry> root = rootNav->entry();

      QString bName = stripAnonymousNamespaceScope(root->name);
      bName = stripTemplateSpecifiersFromScope(bName);

      QSharedPointer<ClassDef> cd = getClass(bName);

      // strip any anonymous scopes first
      QHash<QString, QSharedPointer<ClassDef>> *templInstances;

      if (cd && (templInstances = cd->getTemplateInstances())) {
         Debug::print(Debug::Classes, 0, "  Template class %s : \n", csPrintable(cd->name()));

         for (auto tcd = templInstances->begin(); tcd != templInstances->end(); ++tcd) {
            // for each template instance

            Debug::print(Debug::Classes, 0, "    Template instance %s : \n", csPrintable((*tcd)->name()) );
            QString templSpec = tcd.key();

            ArgumentList *templArgs = new ArgumentList;
            stringToArgumentList(csPrintable(templSpec), templArgs);

            QList<BaseInfo> &baseList = root->extends;

            for (auto &bi : baseList) {
               // for each base class of the template, check if the base class is a template argument

               BaseInfo tbi(bi.name, bi.prot, bi.virt);
               ArgumentList *tl = cd->templateArguments();

               if (tl) {
                  QHash<QString, int> *baseClassNames = (*tcd)->getTemplateBaseClassNames();
                  QHash<QString, int> *templateNames  = getTemplateArgumentsInName(tl, bi.name);

                  // for each template name that we inherit from we need to
                  // substitute the formal with the actual arguments

                  QHash<QString, int> *actualTemplateNames = new QHash<QString, int>();

                  for (int index : *templateNames) {

                     Argument *actArg = 0;

                     if (index < templArgs->count()) {
                        actArg = &(*templArgs)[index];
                     }

                     if (actArg != 0 && baseClassNames != 0 && baseClassNames->contains(actArg->type)
                            && ! actualTemplateNames->contains(actArg->type)) {

                        actualTemplateNames->insert(actArg->type, index);
                     }
                  }

                  delete templateNames;

                  tbi.name = substituteTemplateArgumentsInString(bi.name, tl, templArgs);

                  // find a documented base class in the correct scope
                  if (! findClassRelation(rootNav, cd, *tcd, &tbi, actualTemplateNames, DocumentedOnly, false)) {

                     // no documented base class -> try to find an undocumented one
                     findClassRelation(rootNav, cd, *tcd, &tbi, actualTemplateNames, Undocumented, true);
                  }

                  delete actualTemplateNames;
               }
            }

            delete templArgs;
         }

         // class has no base classes
      }

      rootNav->releaseEntry();
   }
}

// compute the references (anchors in HTML) for each function in the file
void Doxy_Work::computeMemberReferences()
{
   for (auto cd : *Doxy_Globals::classSDict) {
      cd->computeAnchors();
   }

   for (auto fn : *Doxy_Globals::inputNameList) {
      for (auto fd : *fn) {
         fd->computeAnchors();
      }
   }

   for (auto nd : *Doxy_Globals::namespaceSDict) {
      nd->computeAnchors();
   }

   for (auto gd : *Doxy_Globals::groupSDict) {
      gd->computeAnchors();
   }
}

void Doxy_Work::addListReferences()
{
   for (auto mn : *Doxy_Globals::memberNameSDict) {
      for (auto md : *mn) {
         md->visited = false;
      }
   }

   for (auto mn : *Doxy_Globals::functionNameSDict) {
     for (auto md : *mn) {
         md->visited = false;
      }
   }

   for (auto cd : *Doxy_Globals::classSDict) {
      cd->addListReferences();
   }

   for (auto fn : *Doxy_Globals::inputNameList) {
      for (auto fd : *fn) {
         fd->addListReferences();
      }
   }

   for (auto nd : *Doxy_Globals::namespaceSDict) {
      nd->addListReferences();
   }

   for (auto gd : *Doxy_Globals::groupSDict) {
      gd->addListReferences();
   }

   for (auto pd : *Doxy_Globals::pageSDict) {
      QString name = pd->getOutputFileBase();

      if (pd->getGroupDef()) {
         name = pd->getGroupDef()->getOutputFileBase();
      }
      
      QList<ListItemInfo> *xrefItems = pd->xrefListItems();
      addRefItem(xrefItems, name, theTranslator->trPage(true, true), name, pd->title(), "", QSharedPointer<Definition>());      
   }

   for (auto dd : Doxy_Globals::directories) {
      QString name = dd->getOutputFileBase();
      QList<ListItemInfo> *xrefItems = dd->xrefListItems();

      addRefItem(xrefItems, name, theTranslator->trDir(true, true), name, dd->displayName(), "", QSharedPointer<Definition>());
   }
}

void Doxy_Work::generateXRefPages()
{
   for (auto &rl : *Doxy_Globals::xrefLists) {
      rl.generatePage();
   }
}

// Copy the documentation in entry `root' to member definition `md' and 
// set the function declaration of the member to `funcDecl'.  
// If 'overload' is set the standard overload text is added.

void Doxy_Work::addMemberDocs(QSharedPointer<EntryNav> rootNav, QSharedPointer<MemberDef> md, const QString &funcDecl, ArgumentList *al,
                          bool overload, NamespaceSDict *)
{
   QSharedPointer<Entry> root = rootNav->entry();

   QString fDecl = funcDecl;

   // strip extern specifier
   fDecl = stripPrefix(fDecl, "extern ");

   md->setDefinition(fDecl);
   md->enableCallGraph(root->callGraph);
   md->enableCallerGraph(root->callerGraph);

   QSharedPointer<ClassDef> cd     = md->getClassDef();
   QSharedPointer<NamespaceDef> nd = md->getNamespaceDef();

   QString fullName;

   if (cd) {
      fullName = cd->name();

   } else if (nd) {
      fullName = nd->name();
   }

   if (! fullName.isEmpty()) {
      fullName += "::";
   }

   fullName += md->name();
   QSharedPointer<FileDef> rfd = rootNav->fileDef();

   // TODO determine scope based on root not md
   QSharedPointer<Definition> rscope = md->getOuterScope();

   ArgumentList *mdAl = md->argumentList();

   if (al) {
      mergeArguments(mdAl, al, !root->doc.isEmpty());

   } else {
      if (
         matchArguments2(md->getOuterScope(), md->getFileDef(), mdAl, rscope, rfd, &root->argList, true)) {
         mergeArguments(mdAl, &root->argList, ! root->doc.isEmpty());
      }
   }

   if (overload) { 
      // the \overload keyword was used

      QString doc = theTranslator->trOverloadText();
      if (! root->doc.isEmpty()) {
         doc += "<p>";
         doc += root->doc;
      }

      md->setDocumentation(doc, root->docFile, root->docLine);
      md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
      md->setDocsForDefinition(!root->proto);

   } else {     
      md->setDocumentation(root->doc, root->docFile, root->docLine);
      md->setDocsForDefinition(! root->proto);

      md->setBriefDescription(root->brief, root->briefFile, root->briefLine);

      if ((md->inbodyDocumentation().isEmpty() || ! rootNav->parent()->name().isEmpty() ) && !root->inbodyDocs.isEmpty()) {
         md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
      }
   }

   if (md->initializer().isEmpty() && !root->initializer.isEmpty()) {      
      md->setInitializer(root->initializer);
   }

   md->setMaxInitLines(root->initLines);

   if (rfd) {
      if ((md->getStartBodyLine() == -1 && root->bodyLine != -1)) {         
         md->setBodySegment(root->bodyLine, root->endBodyLine);
         md->setBodyDef(rfd);
      }

      md->setRefItems(root->sli);
   }

   md->enableCallGraph(md->hasCallGraph() || root->callGraph);
   md->enableCallerGraph(md->hasCallerGraph() || root->callerGraph);

   md->mergeMemberTraits(root->m_traits);
   md->addSectionsToDefinition(root->anchors);
   addMemberToGroups(root, md);

   if (cd) {
      cd->insertUsedFile(rfd);
   }

   if (root->mGrpId != -1) {
      if (md->getMemberGroupId() != -1) {
         if (md->getMemberGroupId() != root->mGrpId) {
            warn(root->fileName, root->startLine,
               "member %s belongs to two different groups. The second "
               "one found here will be ignored.",  csPrintable(md->name()));
         }

      } else { // set group id
         md->setMemberGroupId(root->mGrpId);
      }
   }
}

// find a class definition given the scope name and (optionally) a
// template list specifier
QSharedPointer<ClassDef> Doxy_Work::findClassDefinition(QSharedPointer<FileDef> fd, QSharedPointer<NamespaceDef> nd, const QString &scopeName)
{
   QSharedPointer<ClassDef> tcd = getResolvedClass(nd, fd, scopeName, 0, 0, true, true);
   return tcd;
}

// Adds the documentation contained in `root' to a global function
// with name `name' and argument list `args' (for overloading) and
// function declaration `decl' to the corresponding member definition.
bool Doxy_Work::findGlobalMember(QSharedPointer<EntryNav> rootNav, const QString &namespaceName, const QString &type, 
                  const QString &name, const QString &tempArg, const QString &, const QString &decl)
{
   QSharedPointer<Entry> root = rootNav->entry();

   Debug::print(Debug::FindMembers, 0, "\nDebug: findGlobalMember() namespace= %s   type= %s   name= %s   tempArg= %s   decl= %s\n",
                csPrintable(namespaceName), csPrintable(type), csPrintable(name), 
                csPrintable(tempArg), csPrintable(decl) );

   QString n = name;

   if (n.isEmpty()) {
      return false;
   }

   if (n.indexOf("::") != -1) {
      // skip undefined class members
      return false;   
   }

   // look in function dictionary
   QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict->find(n + tempArg); 

   if (mn == nullptr) {
      // try without template arguments
      mn = Doxy_Globals::functionNameSDict->find(n); 
   }

   if (mn) {
      // function name defined

      Debug::print(Debug::FindMembers, 0, "\nDebug: findGlobalMember() found symbol scope\n");
      bool found = false;

      for (auto md : *mn) {
         if (found) {
            break;
         }

         QSharedPointer<NamespaceDef> nd = md->getNamespaceDef();
         QSharedPointer<FileDef> fd = rootNav->fileDef();

         NamespaceSDict *nl = fd ? fd->getUsedNamespaces() : 0;

         // search in the list of namespaces that are imported via a
         // using declaration
         bool viaUsingDirective = nl && nd && nl->find(nd->qualifiedName()) != 0;

         if ((namespaceName.isEmpty() && nd == 0) || (nd && nd->name() == namespaceName) || viaUsingDirective) { 
            // not in a namespace, or in the same namespace, memeber in "using' namespace

            Debug::print(Debug::FindMembers, 0, "\nDebug: findGlobalMember() attempting to add member %s to scope %s\n",
                         csPrintable(md->name()), csPrintable(namespaceName));

            QSharedPointer<NamespaceDef> rnd;

            if (! namespaceName.isEmpty()) {
               rnd = Doxy_Globals::namespaceSDict->find(namespaceName);
            }

            ArgumentList *mdAl = md->argumentList();

            bool matching = (mdAl == 0 && root->argList.count() == 0) || md->isVariable() || md->isTypedef() ||
                     matchArguments2(md->getOuterScope(), md->getFileDef(), mdAl,
                     rnd ? rnd : Doxy_Globals::globalScope, fd, &root->argList, false);

            // for template members we need to check if the number of
            // template arguments is the same, otherwise we are dealing with different functions
            if (matching && root->tArgLists) {
               ArgumentList *mdTempl = md->templateArguments();

               if (mdTempl) {
                  if (root->tArgLists->last().count() != mdTempl->count()) {
                     matching = false;
                  }
               }
            }

            // for static members we also check if the comment block was found in
            // the same file. This is needed because static members with the same
            // name can be in different files. Thus it would be wrong to just
            // put the comment block at the first syntactically matching member.
            if (matching && md->isStatic() && md->getDefFileName() != root->fileName && mn->count() > 1) {
               matching = false;
            }

            // for template member we also need to check the return type
            if (md->templateArguments() != 0 && root->tArgLists != 0) {

               if (md->templateArguments()->count() != root->tArgLists->last().count() || md->typeString() != type) {
                  matching = false;
               }
            }

            if (matching) { 
               // add docs to the member
               Debug::print(Debug::FindMembers, 0, "\nDebug: findGlobalMember() match found\n");
               addMemberDocs(rootNav, md, decl, &root->argList, false);
               found = true;
            }
         }
      }

      if (! found && root->relatesType != Duplicate && root->section == Entry::FUNCTION_SEC) { 
         // no match
         QString fullFuncDecl = decl;         
         fullFuncDecl += argListToString(&root->argList, true);
         
         QString warnMsg = QString("No matching file member found for \n") + substitute(fullFuncDecl, "%", "%%");

         if (mn->count() > 0) {
            warnMsg += "\nPossible candidates:\n";

            for (auto md : *mn) {
               warnMsg += " '";
               warnMsg += substitute(md->declaration(), "%", "%%");
               warnMsg += "' at line " + QString().setNum(md->getDefLine()) + " of file" + md->getDefFileName() + "\n";
            }
         }

         warn(root->fileName, root->startLine, warnMsg);
      }
   } else { 
      // have docs for an undefined member

      if (root->type != "friend class" && root->type != "friend struct" &&
            root->type != "friend union" && (! Config::getBool("use-typedef-name") || root->type.indexOf("typedef ") == -1)) {

         warn(root->fileName, root->startLine, "documented symbol `%s' was not declared or defined", csPrintable(decl) );
      }
   }

   return true;
}

bool Doxy_Work::isSpecialization(const QList<ArgumentList> &srcTempArgLists, const QList<ArgumentList> &dstTempArgLists)
{
   auto dest = dstTempArgLists.begin();

   for (auto &source : srcTempArgLists) {

      if (dest == dstTempArgLists.end()) {
         break;
      }

      const ArgumentList &sal = source;
      const ArgumentList &dal = *dest;

      if (sal.count() != dal.count()) {
         return true;
      }

      ++dest;
   }

   return false;
}

bool Doxy_Work::scopeIsTemplate(QSharedPointer<Definition> d)
{
   bool result = false;

   if (d && d->definitionType() == Definition::TypeClass) {
      QSharedPointer<ClassDef> temp = d.dynamicCast<ClassDef>();   
      result = ( temp->templateArguments() || scopeIsTemplate(d->getOuterScope()) );
   }

   return result;
}

QString Doxy_Work::substituteTemplatesInString(const QList<ArgumentList> &srcTempArgLists, const QList<ArgumentList> &dstTempArgLists,
                                                  ArgumentList *funcTempArgList, const QString &src)
{
   // can be used to match template specializations

   QString dst;
   QRegExp re( "[A-Za-z_][A-Za-z_0-9]*");

   int i;
   int p = 0;
   int l;

   while ((i = re.indexIn(src, p)) != -1) {
      // for each word in srcType
      l = re.matchedLength();

      dst += src.mid(p, i - p);
      QString name = src.mid(i, l);

      bool found = false;
      auto dest  = dstTempArgLists.begin();

      for (auto &source : srcTempArgLists) {

         if (found) {
            break;
         }

         ArgumentList::iterator fali;
         Argument *fa = 0;

         if (funcTempArgList) {
            fali = funcTempArgList->begin();
            fa = &*fali;
         }
         
         auto tempDest = dest->begin();   
         
         for (auto &tsa : source) {

            if (found) {
               break;
            }

            Argument *tda = nullptr;

            if (tempDest != dest->end())  {
               tda = const_cast<Argument *>(& (*tempDest) );
            }

            if (name == tsa.name) {

               if (tda && tda->name.isEmpty()) {
                  int vc = 0;

                  if (tda->type.startsWith("class ")) {
                     vc = 6;

                  } else if (tda->type.startsWith("typename ")) {
                     vc = 9;

                  }

                  if (vc > 0) {
                     // convert type=="class T" to type=="class" name=="T"
                     tda->name = tda->type.mid(vc);
                     tda->type = tda->type.left(vc - 1);
                  }
               }

               if (tda && ! tda->name.isEmpty()) {
                  name = tda->name; // substitute
                  found = true;

               } else if (fa) {
                  name = fa->type;
                  found = true;
               }
            }

            if (tda) {

               if (tempDest != dest->end())  {
                  ++tempDest;
               }

            } else if (funcTempArgList) {

               if (fali != funcTempArgList->end()) {
                  ++fali;

                  if (fali != funcTempArgList->end()) {
                     fa = &*fali;
                  } else {
                     fa = 0;
                  }
               }
            }
         }

         if (dest != dstTempArgLists.end()) {
            ++dest;
         }
      }

      dst += name;
      p = i + l;
   }

   dst += src.right(src.length() - p);

   return dst;
}

void Doxy_Work::substituteTemplatesInArgList(const QList<ArgumentList> &srcTempArgLists, const QList<ArgumentList> &dstTempArgLists,
            ArgumentList *src, ArgumentList *dst, ArgumentList *funcTempArgs)
{  
   Argument da;

   if (! dst->isEmpty())  {
      da = dst->first();
   }
   
   auto destIter = dst->begin();

   for (auto &sa : *src) {
      // for each member argument

      QString dstType  = substituteTemplatesInString(srcTempArgLists, dstTempArgLists, funcTempArgs, sa.type);
      QString dstArray = substituteTemplatesInString(srcTempArgLists, dstTempArgLists, funcTempArgs, sa.array);

      if (destIter == dst->end()) {
         da = sa;

         da.type  = dstType;
         da.array = dstArray;

         dst->append(da);
         destIter = dst->end();

      } else {
         destIter->type  = dstType;
         destIter->array = dstArray;

         if (destIter != dst->end()) {
            ++destIter;
         }

         if (destIter != dst->end()) {
            da = *destIter;
         }
      }
   }

   dst->constSpecifier     = src->constSpecifier;
   dst->volatileSpecifier  = src->volatileSpecifier;
   dst->pureSpecifier      = src->pureSpecifier;
   dst->refSpecifier       = src->refSpecifier;
   dst->trailingReturnType = substituteTemplatesInString(srcTempArgLists, dstTempArgLists, funcTempArgs, src->trailingReturnType);
}

/*! This function tries to find a member (in a documented class/file/namespace)
 * that corresponds to the function/variable declaration given in \a funcDecl.
 *
 * The boolean \a overloaded is used to specify whether or not a standard
 * overload documentation line should be generated.
 *
 * The boolean \a isFunc is a hint that indicates that this is a function
 * instead of a variable or typedef.
 */
void Doxy_Work::findMember(QSharedPointer<EntryNav> rootNav, QString funcDecl, bool overloaded, bool isFunc)
{
   QSharedPointer<Entry> root = rootNav->entry();

   Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [start] root= %p   funcDecl= %s   related= %s\n"
                "  overload= %d   isFunc= %d   mGrpId= %d   tArgList= %p (# of tArgs= %d)  lang= %x\n",
                root.data(), csPrintable(funcDecl), csPrintable(root->relates), overloaded, isFunc, root->mGrpId,
                root->tArgLists, root->tArgLists ? root->tArgLists->count() : 0, root->lang);

   QString scopeName;
   QString className;
   QString namespaceName;
   QString funcType;
   QString funcName;
   QString funcArgs;
   QString funcTempList;
   QString exceptions;
   QString funcSpec;

   bool isRelated  = false;
   bool isMemberOf = false;
   bool isFriend   = false;
   bool done;

   do {
      done = true;

      // treat friends as related members
      if (funcDecl.startsWith("friend ")) {
         funcDecl = stripPrefix(funcDecl, "friend ");
         isFriend = true;
         done = false;
      }

      if (funcDecl.startsWith("inline ")) {
         funcDecl = stripPrefix(funcDecl, "inline ");
         root->m_traits.setTrait(Entry::Virtue::Inline);
         done = false;
      }

      if (funcDecl.startsWith("explicit ")) {
         funcDecl = stripPrefix(funcDecl, "explicit ");
         root->m_traits.setTrait(Entry::Virtue::Explicit);
         done = false;
      }

      if (funcDecl.startsWith("mutable ")) {
         funcDecl = stripPrefix(funcDecl, "mutable ");
         root->m_traits.setTrait(Entry::Virtue::Mutable);
         done = false;
      }

      if (funcDecl.startsWith("virtual ")) {
         funcDecl = stripPrefix(funcDecl, "virtual ");
         done = false;
      }

   } while (! done);

   // delete any ; from the function declaration
   int sep;

   while ((sep = funcDecl.indexOf(';')) != -1) {
      funcDecl = (funcDecl.left(sep) + funcDecl.right(funcDecl.length() - sep - 1)).trimmed();
   }

   // make sure the first character is a space to simplify searching.
   if (! funcDecl.isEmpty() && funcDecl[0] != ' ') {
      funcDecl.prepend(" ");
   }

   // remove some superfluous spaces
   funcDecl = substitute(
                 substitute(
                    substitute(funcDecl, "~ ", "~"),
                    ":: ", "::"),
                 " ::", "::").trimmed();
   
   if (isFriend && funcDecl.startsWith("class ")) {     
      funcDecl = funcDecl.right(funcDecl.length() - 6);
      funcName = funcDecl;

   } else if (isFriend && funcDecl.startsWith("struct ")) {
      funcDecl = funcDecl.right(funcDecl.length() - 7);
      funcName = funcDecl;

   } else {
      // extract information from the declarations
      parseFuncDecl(funcDecl, root->lang == SrcLangExt_ObjC, scopeName, funcType, funcName,
                    funcArgs, funcTempList, exceptions);
   }
   
   // the class name can also be a namespace name, we decide this later.
   // if a related class name is specified and the class name could
   // not be derived from the function declaration, then use the related field

   if (! root->relates.isEmpty()) {
      // related member, prefix user specified scope
      isRelated = true;
      isMemberOf = (root->relatesType == MemberOf);

      if (getClass(root->relates) == 0 && ! scopeName.isEmpty()) {
         scopeName = mergeScopes(scopeName, root->relates);
      } else {
         scopeName = root->relates;
      }
   }

   if (root->relates.isEmpty() && rootNav->parent() &&
         ((rootNav->parent()->section()&Entry::SCOPE_MASK) ||
          (rootNav->parent()->section() == Entry::OBJCIMPL_SEC) ) &&
           ! rootNav->parent()->name().isEmpty()) {

      // see if we can combine scopeName
      // with the scope in which it was found

      QString joinedName = rootNav->parent()->name() + "::" + scopeName;

      if (! scopeName.isEmpty() && (getClass(joinedName) || Doxy_Globals::namespaceSDict->find(joinedName))) {
         scopeName = joinedName;

      } else {
         scopeName = mergeScopes(rootNav->parent()->name(), scopeName);
      }

   } else {
      // see if we can prefix a namespace or class that is used from the file
      QSharedPointer<FileDef> fd = rootNav->fileDef();

      if (fd) {
         NamespaceSDict *fnl = fd->getUsedNamespaces();

         if (fnl) {
            QString joinedName;

            for (auto fnd : *fnl) {
               joinedName = fnd->name() + "::" + scopeName;

               if (Doxy_Globals::namespaceSDict->find(joinedName)) {
                  scopeName = joinedName;
                  break;
               }
            }
         }
      }
   }

   scopeName = stripTemplateSpecifiersFromScope(removeRedundantWhiteSpace(scopeName), false, &funcSpec);

   // funcSpec contains the last template specifiers of the given scope.
   // If this method does not have any template arguments or they are empty 
   // while funcSpec is not empty we assume this is a specialization of a method. 
   // If not, we clear the funcSpec and treat this as a normal method of a template class.

   if (! (root->tArgLists && root->tArgLists->count() > 0 && root->tArgLists->first().count() == 0) ) {
      funcSpec.resize(0);
   }

   // split scope into a namespace and a class part
   extractNamespaceName(scopeName, className, namespaceName, true);

   if (namespaceName.indexOf('@') != -1) {
      // skip stuff in anonymous namespace
      return;   
   }
   
   // merge class and namespace scopes again
   scopeName.resize(0);

   if (! namespaceName.isEmpty()) {
      if (className.isEmpty()) {
         scopeName = namespaceName;

      } else if (! root->relates.isEmpty() || ! getClass(className)) { 
         // relates command with explicit scopem, class name only exists in a namespace
         scopeName = namespaceName + "::" + className;

      } else {
         scopeName = className;
      }

   } else if (! className.isEmpty()) {
      scopeName = className;
   }

   QString tempScopeName = scopeName;
   QSharedPointer<ClassDef> cd = getClass(scopeName);

   if (cd) {
      if (funcSpec.isEmpty()) {
         int argListIndex = 0;
         tempScopeName = cd->qualifiedNameWithTemplateParameters(root->tArgLists, &argListIndex);
      } else {
         tempScopeName = scopeName + funcSpec;
      }
   }

   // rebuild the function declaration (needed to get the scope right)
   if (! scopeName.isEmpty() && !isRelated && ! isFriend && ! Config::getBool("hide-scope-names")) {
      if (! funcType.isEmpty()) {
         if (isFunc) { 
            // a function -> we use argList for the arguments
            funcDecl = funcType + " " + tempScopeName + "::" + funcName + funcTempList;
         } else {
            funcDecl = funcType + " " + tempScopeName + "::" + funcName + funcArgs;
         }

      } else {
         if (isFunc) { 
            // a function => we use argList for the arguments
            funcDecl = tempScopeName + "::" + funcName + funcTempList;
         } else { 
            // variable => add `argument' list
            funcDecl = tempScopeName + "::" + funcName + funcArgs;
         }
      }

   } else {      
      // build declaration without scope

      if (!funcType.isEmpty()) { // but with a type
         if (isFunc) { 
            // function => omit argument list
            funcDecl = funcType + " " + funcName + funcTempList;

         } else { 
            // variable => add `argument' list
            funcDecl = funcType + " " + funcName + funcArgs;
         }

      } else { // no type
         if (isFunc) {
            funcDecl = funcName + funcTempList;
         } else {
            funcDecl = funcName + funcArgs;
         }
      }
   }


   if (funcType == "template class" && ! funcTempList.isEmpty()) {
      return;   // ignore explicit template instantiations
   }

   Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [parsed] namespaceName= %s   className= %s\n"
                "  funcType= %s   funcSpec= %s   funcName= %s   funcArgs= %s\n"
                "  funcTempList= %s   funcDecl= %s   related= %s   exceptions= %s\n"
                "  isRelated= %d   isMemberOf= %d   isFriend= %d   isFunc= %d\n",
                csPrintable(namespaceName), csPrintable(className),
                csPrintable(funcType), csPrintable(funcSpec), csPrintable(funcName), csPrintable(funcArgs), 
                csPrintable(funcTempList), csPrintable(funcDecl), csPrintable(root->relates), 
                csPrintable(exceptions), isRelated, isMemberOf, isFriend, isFunc );

   QSharedPointer<MemberName> mn;

   if (! funcName.isEmpty()) {
      // function name is valid
      Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [1] funcName= %s\n", csPrintable(funcName));

      if (funcName.startsWith("operator ")) { 
         // strip class scope from cast operator
         funcName = substitute(funcName, className + "::", "");
      }

      if (! funcTempList.isEmpty()) { 
         // try with member specialization
         mn = Doxy_Globals::memberNameSDict->find(funcName + funcTempList);
      }

      if (mn == nullptr) { 
         // try without specialization
         mn = Doxy_Globals::memberNameSDict->find(funcName);
      }

      if (! isRelated && mn) { 
         // function name already found
         Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [2] member name exists "
                  "(%d members with this name)\n", mn->count());

         if (! className.isEmpty()) { 
            // class name is valid

            if (funcSpec.isEmpty()) { 
               // not a member specialization
               int count = 0;

               int noMatchCount = 0;
               bool memFound = false;

               for (auto md : *mn) {

                  if (memFound) {
                     break;
                  }

                  QSharedPointer<ClassDef> cd = md->getClassDef();
                  Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() member definition: \n"
                               "   scope needed= %s   scope= %s   args= %s   fileName= %s\n",
                               csPrintable(scopeName), cd ? csPrintable(cd->name()) : "none", 
                               csPrintable(md->argsString()), csPrintable(root->fileName)  );

                  QSharedPointer<FileDef> fd = rootNav->fileDef();
                  QSharedPointer<NamespaceDef> nd;

                  if (! namespaceName.isEmpty()) {
                     nd = getResolvedNamespace(namespaceName);
                  }

                  QSharedPointer<ClassDef> tcd = findClassDefinition(fd, nd, scopeName);

                  if (tcd == nullptr && cd && stripAnonymousNamespaceScope(cd->name()) == scopeName) {
                     // do not be fooled by anonymous scopes
                     tcd = cd;
                  }

                  if (cd && tcd == cd) { 
                     // member's classes match

                     Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [4] class definition: %s\n", 
                        csPrintable(cd->name()));

                     // get the template parameter lists found at the member declaration
                     QList<ArgumentList> declTemplArgs;

                     cd->getTemplateParameterLists(declTemplArgs);
                     ArgumentList *templAl = md->templateArguments();

                     if (templAl) {
                        declTemplArgs.append(*templAl);
                     }

                     // get the template parameter lists found at the member definition
                     QList<ArgumentList> *defTemplArgs = root->tArgLists;

                     // do we replace the decl argument lists with the def argument lists?
                     bool substDone = false;
                     ArgumentList *argList = 0;

                     // substitute the occurrences of class template names in the
                     // argument list before matching                     

                     ArgumentList *mdAl = md->argumentList();

                     if (declTemplArgs.count() > 0 && defTemplArgs &&
                           declTemplArgs.count() == defTemplArgs->count() && mdAl) {

                        /* the function definition has template arguments
                         * and the class definition also has template arguments, so
                         * we must substitute the template names of the class by that
                         * of the function definition before matching.
                         */

                        argList = new ArgumentList;
                        substituteTemplatesInArgList(declTemplArgs, *defTemplArgs, mdAl, argList);

                        substDone = true;

                     } else { 
                        // no template arguments, compare argument lists directly
                        argList = mdAl;

                     }

                     Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [5] matching %s with %s   className= %s "
                           "namespaceName= %s\n", csPrintable(argListToString(argList, true)), 
                           csPrintable(argListToString(&root->argList, true)), 
                           csPrintable(className), csPrintable(namespaceName) );
                    
                     bool matching = md->isVariable() || md->isTypedef();

                     if (! matching && (mdAl == 0 && root->argList.count() == 0)) {
                        matching = true;
                     }

                     if (! matching && matchArguments2(md->getClassDef(), md->getFileDef(), 
                           argList, cd, fd, &root->argList, true)) {
                        matching = true;
                     }

                     if (md->getLanguage() == SrcLangExt_ObjC && md->isVariable() && (root->section & Entry::FUNCTION_SEC)) {
                        // do not match methods and attributes with the same name
                        matching = false; 
                     }

                     // for template member check the return type
                     if (md->templateArguments() != 0 && root->tArgLists != 0) {
                        // return type from the source
                        QString memType = md->typeString();

                        // see bug700696
                        memType  = stripPrefix(memType, "static "); 

                        // see bug700693 & bug732594
                        funcType = substitute(stripTemplateSpecifiersFromScope(funcType, true), className + "::", ""); 
                        memType  = substitute(stripTemplateSpecifiersFromScope(memType, true), className+"::",""); 

                        Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [6] comparing return types %s with %s"
                                     "#args %d with %d\n", csPrintable(md->typeString()), csPrintable(funcType),
                                     md->templateArguments()->count(), root->tArgLists->last().count());

                        if (md->templateArguments()->count() != root->tArgLists->last().count() || memType != funcType) {
                           matching = false;
                        }

                     } else if (matching && ! (md->isVariable() || md->isTypedef() || md->isEnumValue()) ) {
                        // compare the return types

                        // value from source code
                        QString memType = md->typeString();

                        if (memType == "virtual") { 
                           memType = "";
                        }

                        memType  = stripPrefix(memType, "static "); 
                        memType  = stripPrefix(memType, "virtual "); 
                       
                        // remove myClass<T>, or myClass, etc
                        if (memType.startsWith(className)) {   
                           memType = stripTemplateSpecifiersFromScope(memType, true);
                           memType = substitute(memType, className + "::", ""); 
                        } 

                        // remove myClass<T>, or myClass, etc
                        if (funcType.startsWith(className)) {   
                           funcType = stripTemplateSpecifiersFromScope(funcType, true);
                           funcType = substitute(funcType, className + "::", ""); 
                        } 

                        if (memType != funcType) {  
                           warn(root->fileName, root->startLine, "Return types do not agree for member: %s\n"
                                "   return type: %s   return type: %s\n",     
                                csPrintable(funcName), csPrintable(memType), csPrintable(funcType) );
                        }
                     }

                     bool rootIsUserDoc       = (root->section & Entry::MEMBERDOC_SEC) != 0;
                     bool classIsTemplate     = scopeIsTemplate(md->getClassDef());
                     bool mdIsTemplate        = md->templateArguments() != 0;
                     bool classOrMdIsTemplate = mdIsTemplate || classIsTemplate;
                     bool rootIsTemplate      = root->tArgLists != 0;

                     if (! rootIsUserDoc &&                       // do not check out-of-line @fn references, see bug722457
                           (mdIsTemplate || rootIsTemplate) &&    // either md or root is a template
                           ((classOrMdIsTemplate && !rootIsTemplate) || (! classOrMdIsTemplate && rootIsTemplate))) {

                        // Method with template return type does not match method without return type
                        // even if the parameters are the same. See also bug709052

                        Debug::print(Debug::FindMembers, 0, 
                              "\nDebug: findMember() [7] comparing return types: template with non-template\n");

                        matching = false;
                     }                     

                     if (substDone) { 
                        // found a new argument list

                        if (matching) { 
                           // replace member's argument list
                           md->setDefinitionTemplateParameterLists(root->tArgLists);
                           md->setArgumentList(argList); // new owner of the list => no delete

                        } else { 
                           // no match

                           if (! funcTempList.isEmpty() && isSpecialization(declTemplArgs, *defTemplArgs)) {
                              // check if we are dealing with a partial template specialization. 
                              // In this case we add it to the class even though the member arguments do not match.

                              // TODO: copy other aspects?
                              root->protection = md->protection(); 
                              addMethodToClass(rootNav, cd, md->name(), isFriend);

                              return;
                           }

                           delete argList;
                        }
                     }

                     if (matching) {
                        addMemberDocs(rootNav, md, funcDecl, 0, overloaded, 0);
                        count++;
                        memFound = true;
                     }

                  } else if (cd && cd != tcd) {
                     // found a class with the same name as cd, but in a different namespace
                     noMatchCount++;
                  }

               }

               if (count == 0 && rootNav->parent() && rootNav->parent()->section() == Entry::OBJCIMPL_SEC) {
                  goto localObjCMethod;
               }

               if (count == 0 && ! (isFriend && funcType == "class")) {
                  int candidates = 0;

                  QSharedPointer<ClassDef> ecd = QSharedPointer<ClassDef>();
                  QSharedPointer<ClassDef> ucd = QSharedPointer<ClassDef>();

                  QSharedPointer<MemberDef> emd;
                  QSharedPointer<MemberDef> umd;

                  if (mn->count() > 0) {

                     for (auto md : *mn) {
                        QSharedPointer<ClassDef> ccd  = md->getClassDef();
                        QSharedPointer<MemberDef> cmd = md;
                        
                        if (ccd != 0 && rightScopeMatch(ccd->name(), className)) {
                           ArgumentList *templAl = md->templateArguments();

                           if (root->tArgLists && templAl != 0 && root->tArgLists->last().count() <= templAl->count()) {
                              addMethodToClass(rootNav, ccd, md->name(), isFriend);
                              return;
                           }

                           if (md->argsString() == argListToString(&root->argList, true, false)) {
                              // exact argument list match -> remember
                              ucd = ecd = ccd;
                              umd = emd = cmd;

                              Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [8] className= %s   scope= %s\n"
                                 "  args= %s [Exact Match]\n", csPrintable(className), csPrintable(ccd->name()), 
                                 csPrintable(md->argsString()) );

                           } else {
                              // arguments do not match, but member name and scope do -> remember
                              ucd = ccd;
                              umd = cmd;

                              Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [9] className= %s scope= %s args= %s"
                                 " [No Match]\n", csPrintable(className), csPrintable(ccd->name()), csPrintable(md->argsString()) );
                           }

                           candidates++;
                        }
                     }
                  }

                  static bool strictSigMatching = Config::getBool("strict-sig-matching");

                  if (! strictSigMatching) {

                     if (candidates == 1 && ucd && umd) {
                        // did not find a full signature match on arguments, there is only one  member 
                        // with this name in this same scope, use the mtype for the .h file

                        addMemberDocs(rootNav, umd, funcDecl, 0, overloaded, 0);
                        return;

                     } else if (candidates > 1 && ecd && emd) {
                        // did not find a unique match using type resolution, but one of the matches
                        // has the exact same signature so we take it

                        addMemberDocs(rootNav, emd, funcDecl, 0, overloaded, 0);
                        return;
                     }
                  }

                  QString warnMsg = "no ";

                  if (noMatchCount > 1) {
                     warnMsg += "uniquely ";
                  }

                  warnMsg += "matching class member found for \n";

                  if (root->tArgLists) {
                     for (auto &al : *root->tArgLists) {
                        warnMsg += "  template ";
                        warnMsg += tempArgListToString(&al, root->lang);
                        warnMsg += '\n';
                     }
                  }

                  QString fullFuncDecl = funcDecl;
                  if (isFunc) {
                     fullFuncDecl += argListToString(&root->argList, true);
                  }

                  warnMsg += "  ";
                  warnMsg += fullFuncDecl;
                  warnMsg += '\n';

                  if (candidates > 0) {
                     warnMsg += "Possible candidates:\n";

                     for (auto md : *mn) {
                        QSharedPointer<ClassDef> cd = md->getClassDef();

                        if (cd != 0 && rightScopeMatch(cd->name(), className)) {
                           ArgumentList *templAl = md->templateArguments();

                           if (templAl != 0) {
                              warnMsg += "  'template ";
                              warnMsg += tempArgListToString(templAl, root->lang);
                              warnMsg += '\n';
                           }

                           warnMsg += "  ";
                           if (! md->typeString().isEmpty()) {
                              warnMsg += md->typeString();
                              warnMsg += ' ';
                           }

                           QString qScope = cd->qualifiedNameWithTemplateParameters();
                           if (!qScope.isEmpty()) {
                              warnMsg += qScope + "::" + md->name();
                           }

                           if (! md->argsString().isEmpty()) {
                              warnMsg += md->argsString();
                           }

                           if (noMatchCount > 1) {
                              warnMsg += "' at line " + QString().setNum(md->getDefLine()) +
                                         " of file " + md->getDefFileName();
                           }

                           warnMsg += '\n';
                        }
                     }
                  }

                  warn_simple(root->fileName, root->startLine, warnMsg);
               }

            } else if (cd) { 
               // member specialization
               QSharedPointer<MemberDef> declMd;
               QSharedPointer<MemberDef> md;

               for (auto md : *mn) {
                  if (md->getClassDef() == cd) {
                     // TODO: should probably also check for matching arguments
                     declMd = md;
                     break;
                  }
               }

               MemberType mtype = MemberType_Function;
               ArgumentList *tArgList = new ArgumentList;

               //  getTemplateArgumentsFromName(cd->name()+"::"+funcName,root->tArgLists);
               md = QMakeShared<MemberDef>(root->fileName, root->startLine, root->startColumn, funcType, funcName,
                                  funcArgs, exceptions, declMd ? declMd->protection() : root->protection,
                                  root->virt, root->stat, Member, mtype, tArgList, &root->argList);

               md->setTagInfo(rootNav->tagInfo());
               md->setLanguage(root->lang);
               md->setId(root->id);
               md->setMemberClass(cd);
               md->setTemplateSpecialization(true);
               md->setTypeConstraints(&root->typeConstr);
               md->setDefinition(funcDecl);
               md->enableCallGraph(root->callGraph);
               md->enableCallerGraph(root->callerGraph);
               md->setDocumentation(root->doc, root->docFile, root->docLine);
               md->setBriefDescription(root->brief,root->briefFile,root->briefLine);
               md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
               md->setDocsForDefinition(! root->proto);
               md->setPrototype(root->proto);
               md->addSectionsToDefinition(root->anchors);
               md->setBodySegment(root->bodyLine, root->endBodyLine);

               QSharedPointer<FileDef> fd = rootNav->fileDef();
               md->setBodyDef(fd);
               md->setMemberTraits(root->m_traits);
               md->setMemberGroupId(root->mGrpId);
               mn->append(md);

               cd->insertMember(md);
               md->setRefItems(root->sli);
               delete tArgList;
            }

         } else if (overloaded) {
            // check if the function belongs to only one class
            // for unique overloaded member we allow the class to be
            // omitted, this is to be Qt compatible. Using this should
            // however be avoided, because it is error prone

            bool unique = true;
            auto iter = mn->begin();

            QSharedPointer<MemberDef> md = *iter;
            assert(md);

            QSharedPointer<ClassDef> cd = md->getClassDef();
            assert(cd);

            QString className = cd->name();
            ++iter;

            while (iter != mn->end()) {
               md = *iter;
               QSharedPointer<ClassDef> cd = md->getClassDef();

               if (className != cd->name()) {
                  unique = false;
               }

               ++iter;
            }

            if (unique) {
               MemberType mtype;

               if (root->mtype == Signal) {
                  mtype = MemberType_Signal;

               } else if (root->mtype == Slot) {
                  mtype = MemberType_Slot;

               } else if (root->mtype == DCOP) {
                  mtype = MemberType_DCOP;

               } else {
                  mtype = MemberType_Function;

               }

               // new overloaded member function
               ArgumentList tArgList = getTemplateArgumentsFromName(cd->name() + "::" + funcName, root->tArgLists);

               QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(root->fileName, root->startLine, root->startColumn, 
                     funcType, funcName, funcArgs, exceptions, root->protection, root->virt, root->stat, Related,
                     mtype, &tArgList, &root->argList);

               md->setTagInfo(rootNav->tagInfo());
               md->setLanguage(root->lang);
               md->setId(root->id);
               md->setTypeConstraints(&root->typeConstr);
               md->setMemberClass(cd);
               md->setDefinition(funcDecl);
               md->enableCallGraph(root->callGraph);
               md->enableCallerGraph(root->callerGraph);

               QString doc = theTranslator->trOverloadText();
               doc += "<p>";
               doc += root->doc;

               md->setDocumentation(doc, root->docFile, root->docLine);
               md->setBriefDescription(root->brief,root->briefFile,root->briefLine);
               md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);

               md->setDocsForDefinition(!root->proto);
               md->setPrototype(root->proto);
               md->addSectionsToDefinition(root->anchors);
               md->setBodySegment(root->bodyLine, root->endBodyLine);

               QSharedPointer<FileDef> fd = rootNav->fileDef();
               md->setBodyDef(fd);
               md->setMemberTraits(root->m_traits);
               md->setMemberGroupId(root->mGrpId);
               mn->append(md);
               cd->insertMember(md);
               cd->insertUsedFile(fd);
               md->setRefItems(root->sli);
            }

         } else { 
            // unrelated function with the same name as a member

            if (! findGlobalMember(rootNav, namespaceName, funcType, funcName, funcTempList, funcArgs, funcDecl)) {
               QString fullFuncDecl = funcDecl;

               if (isFunc) {
                  fullFuncDecl += argListToString(&root->argList, true);
               }

               warn(root->fileName, root->startLine, "Can not determine class for function\n%s", csPrintable(fullFuncDecl));
            }
         }

      } else if (isRelated && !  root->relates.isEmpty()) {
         Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [10] related function"
                      "   scopeName= %s   className= %s\n", csPrintable(scopeName), csPrintable(className));

         if (className.isEmpty()) {
            className = root->relates;
         }

         QSharedPointer<ClassDef> cd;

         if ((cd = getClass(scopeName))) {
            bool newMember = true; // assume we have a new member
            bool newMemberName = false;

            QSharedPointer<MemberDef> mdDefine;
            bool isDefine = false;

            {
               QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict->find(funcName);

               if (mn) {
                  auto iter = mn->begin();
                  mdDefine = *iter;

                  while (mdDefine && !isDefine) {
                     isDefine = isDefine || mdDefine->isDefine();

                     if (!isDefine) {
                        ++iter;

                        if (iter == mn->end()) {
                           break;
                        }

                        mdDefine = *iter;
                     }
                  }
               }
            }

            QSharedPointer<FileDef> fd = rootNav->fileDef();

            if ((mn = Doxy_Globals::memberNameSDict->find(funcName)) == 0) {
               mn = QMakeShared<MemberName>(funcName);
               newMemberName = true; // create a new member name

            } else {
               auto iter = mn->begin();
               QSharedPointer<MemberDef> rmd;

               while ((rmd = *iter) && newMember) {
                  // see if we got another member with matching arguments
                  ArgumentList *rmdAl = rmd->argumentList();

                  newMember = className != rmd->getOuterScope()->name() ||
                     ! matchArguments2(rmd->getOuterScope(), rmd->getFileDef(), rmdAl, cd, fd, &root->argList, true);

                  if (newMember) {
                     ++iter;

                     if (iter == mn->end()) {
                        break;
                     }
                  }
               }

               if (! newMember && rmd) {
                  // member already exists as rmd -> add docs
                  addMemberDocs(rootNav, rmd, funcDecl, 0, overloaded);
               }
            }

            if (newMember) { 
               // need to create a new member
               MemberType mtype;

               if (isDefine) {
                  mtype = MemberType_Define;

               } else if (root->mtype == Signal) {
                  mtype = MemberType_Signal;

               } else if (root->mtype == Slot) {
                  mtype = MemberType_Slot;

               } else if (root->mtype == DCOP) {
                  mtype = MemberType_DCOP;

               } else {
                  mtype = MemberType_Function;
               }

               if (isDefine && mdDefine) {
                  mdDefine->setHidden(true);
                  funcType = "#define";
                  funcArgs = mdDefine->argsString();
                  funcDecl = funcType + " " + funcName;
               }

               // first note that we pass:
               //   (root->tArgLists ? root->tArgLists->last() : 0)
               // for the template arguments fo the new "member."
               // this accurately reflects the template arguments of
               // the related function, which don't have to do with
               // those of the related class.

               QSharedPointer<MemberDef>md = QMakeShared<MemberDef>(root->fileName, root->startLine, root->startColumn, funcType,
                                funcName, funcArgs, exceptions, root->protection, root->virt,
                                root->stat && !isMemberOf, isMemberOf ? Foreign : Related, mtype, 
                                (root->tArgLists ? &root->tArgLists->last() : nullptr), 
                                funcArgs.isEmpty() ? nullptr : &root->argList);

               if (isDefine && mdDefine) {
                  md->setInitializer(mdDefine->initializer());
               }

               // Problem :
               // MemberDef::writeDocumentation() in memberdef.cpp writes the template argument list for the class,
               // as if this member is a member of the class. 
               // Fortunately, MemberDef::writeDocumentation() has a special mechanism that allows us to totally
               // override the set of template argument lists that are printed.  We use that and set it to the
               // template argument lists of the related function.

               md->setDefinitionTemplateParameterLists(root->tArgLists);
               md->setTagInfo(rootNav->tagInfo());

               // try to find the matching line number of the body from the
               // global function list
               bool found = false;

               if (root->bodyLine == -1) {
                  QSharedPointer<MemberName> rmn = Doxy_Globals::functionNameSDict->find(funcName);

                  if (rmn) {
                     auto iter = rmn->begin();
                     QSharedPointer<MemberDef> rmd;

                     while ((rmd = *iter) && ! found) { 
                        // see if we got another member with matching arguments
                        ArgumentList *rmdAl = rmd->argumentList();

                        // check for matching argument lists

                        if ( matchArguments2(rmd->getOuterScope(), rmd->getFileDef(), rmdAl, cd, fd, &root->argList, true)) {
                           found = true;
                        }

                        if (! found) {
                           ++iter;
                           
                           if (iter == rmn->end()) {
                              break;
                           }            
                        }
                     }
   
                     if (rmd) { 
                        // member found -> copy line number info
                        md->setBodySegment(rmd->getStartBodyLine(), rmd->getEndBodyLine());
                        md->setBodyDef(rmd->getBodyDef());                        
                     }
                  }
               }

               if (! found) {
                  // line number could not be found or is available in this entry
              
                  md->setBodySegment(root->bodyLine, root->endBodyLine);
                  md->setBodyDef(fd);
               }           

               md->setMemberClass(cd);
               md->setMemberTraits(root->m_traits);
               md->setDefinition(funcDecl);
               md->enableCallGraph(root->callGraph);
               md->enableCallerGraph(root->callerGraph);
               md->setDocumentation(root->doc, root->docFile, root->docLine);
               md->setBriefDescription(root->brief,root->briefFile,root->briefLine);
               md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
               md->setDocsForDefinition(!root->proto);
               md->setPrototype(root->proto);
               md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
               md->addSectionsToDefinition(root->anchors);
               md->setMemberGroupId(root->mGrpId);
               md->setLanguage(root->lang);
               md->setId(root->id);

               // md->setMemberDefTemplateArguments(root->mtArgList);
               mn->append(md);
               cd->insertMember(md);
               cd->insertUsedFile(fd);
               md->setRefItems(root->sli);

               if (root->relatesType == Duplicate) {
                  md->setRelatedAlso(cd);
               }

               if (!isDefine) {
                  addMemberToGroups(root, md);
               }
              
               if (newMemberName) {                 
                  Doxy_Globals::memberNameSDict->insert(funcName, mn);
               }
            }

            if (root->relatesType == Duplicate) {
               if (! findGlobalMember(rootNav, namespaceName, funcType, funcName, funcTempList, funcArgs, funcDecl)) {
                  QString fullFuncDecl = funcDecl;

                  if (isFunc) {
                     fullFuncDecl += argListToString(&root->argList, true);
                  }

                  warn(root->fileName, root->startLine, 
                       "Can not determine file/namespace for related also function\n%s", csPrintable(fullFuncDecl));
               }
            }

         } else {
            warn_undoc(root->fileName, root->startLine, "Class `%s' for related function `%s' is not "
                       "documented.", csPrintable(className), csPrintable(funcName));
         }

      } else if (rootNav->parent() && rootNav->parent()->section() == Entry::OBJCIMPL_SEC) {

      localObjCMethod:
         QSharedPointer<ClassDef> cd;

         if (Config::getBool("extract-local-methods") && (cd = getClass(scopeName))) {
            Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [11]  method %s"
                         "   scopeName= %s   className= %s\n", csPrintable(root->name), 
                         csPrintable(scopeName), csPrintable(className));
            
            QSharedPointer<MemberDef>md = QMakeShared<MemberDef>(root->fileName, root->startLine, root->startColumn,
               funcType, funcName, funcArgs, exceptions, root->protection, root->virt, root->stat, Member,
               MemberType_Function, nullptr, &root->argList);

            md->setTagInfo(rootNav->tagInfo());
            md->setLanguage(root->lang);
            md->setId(root->id);
            md->makeImplementationDetail();
            md->setMemberClass(cd);
            md->setDefinition(funcDecl);
            md->enableCallGraph(root->callGraph);
            md->enableCallerGraph(root->callerGraph);
            md->setDocumentation(root->doc, root->docFile, root->docLine);
            md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
            md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
            md->setDocsForDefinition(!root->proto);
            md->setPrototype(root->proto);
            md->addSectionsToDefinition(root->anchors);
            md->setBodySegment(root->bodyLine, root->endBodyLine);

            QSharedPointer<FileDef> fd = rootNav->fileDef();
            md->setBodyDef(fd);
            md->setMemberTraits(root->m_traits);
            md->setMemberGroupId(root->mGrpId);
            cd->insertMember(md);
            cd->insertUsedFile(fd);
            md->setRefItems(root->sli);

            mn = Doxy_Globals::memberNameSDict->find(root->name);

            if (mn) {
               mn->append(md);

            } else {
               mn = QMakeShared<MemberName>(root->name);
               mn->append(md);
               Doxy_Globals::memberNameSDict->insert(root->name, mn);
            }

         } else {
            // local objective C method found for class without interface
         }

      } else { 
         // unrelated not overloaded member found

         bool globMem = findGlobalMember(rootNav, namespaceName, funcType, funcName, funcTempList, funcArgs, funcDecl);

         if (className.isEmpty() && !globMem) {
            warn(root->fileName, root->startLine, "Class for member `%s' can not "
                 "be found.",  csPrintable(funcName) );

         } else if (!className.isEmpty() && ! globMem) {
            warn(root->fileName, root->startLine, "Member `%s' of class `%s' can not be found",
                  csPrintable(funcName),  csPrintable(className));
         }
      }

   } else {
      // this should not be called
      warn(root->fileName, root->startLine, "Member with no name found (unusual error)");
   }
}

// find the members corresponding to the different documentation blocks
// that are extracted from the sources.
void Doxy_Work::filterMemberDocumentation(QSharedPointer<EntryNav> rootNav)
{
   QSharedPointer<Entry> root = rootNav->entry();
   int i = -1;
   int l;  

   Debug::print(Debug::FindMembers, 0, "\nDebug: filterMemberDocumentation(): root->type= %s  root->inside= %s   root->name= %s\n"
                "   root->args= %s   section= %x   root->mGrpId= %d\n", 
                csPrintable(root->type), csPrintable(root->inside), csPrintable(root->name), csPrintable(root->args), 
                root->section, root->mGrpId);
   
   bool isFunc = true;

   if (root->relatesType == Duplicate && ! root->relates.isEmpty()) {
      QString tmp = root->relates;

      root->relates.resize(0);
      filterMemberDocumentation(rootNav);
      root->relates = tmp;
   }

   // detect func variable/typedef to func ptr
   if ( (i = findFunctionPtr(root->type, root->lang, &l)) != -1) {
      // fix type and argument

      root->args.prepend(root->type.right(root->type.length() - i - l));
      root->type = root->type.left(i + l);

      isFunc = false;

   } else if ((root->type.left(8) == "typedef " && root->args.indexOf('(') != -1)) {
      // detect function types marked as functions
      isFunc = false;
   }

   if (root->section == Entry::MEMBERDOC_SEC) {

      if (root->type.isEmpty()) {
         findMember(rootNav, root->name + root->args + root->exception, false, isFunc);

      } else {
         findMember(rootNav, root->type + " " + root->name + root->args + root->exception, false, isFunc);
      }

   } else if (root->section == Entry::OVERLOADDOC_SEC) {     
      findMember(rootNav, root->name, true, isFunc);

   } else if ((root->section == Entry::FUNCTION_SEC  || (root->section == Entry::VARIABLE_SEC && ! root->type.isEmpty() &&
               ! Doxy_Globals::g_compoundKeywordDict.contains(root->type) ))) {

      // that is not a keyword
      // to skip forward declaration of class etc.

      if (root->type == "friend class" || root->type == "friend struct" || root->type == "friend union") {
         findMember(rootNav, root->type + " " + root->name, false, false);

      } else if (! root->type.isEmpty()) {
         findMember(rootNav, root->type + " " + root->inside + root->name + root->args + root->exception, false, isFunc);

      } else {
         findMember(rootNav, root->inside + root->name + root->args + root->exception, false, isFunc);
      }

   } else if (root->section == Entry::DEFINE_SEC && ! root->relates.isEmpty()) {
      findMember(rootNav, root->name + root->args, false, ! root->args.isEmpty());

   } else if (root->section == Entry::VARIABLEDOC_SEC) {  
      findMember(rootNav, root->name, false, false);


   } else if (root->section == Entry::EXPORTED_INTERFACE_SEC || root->section == Entry::INCLUDED_SERVICE_SEC) {
      findMember(rootNav, root->type + " " + root->name, false, false);

   } else {
      // skip section      

   }
}

void Doxy_Work::findMemberDocumentation(QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->section() == Entry::MEMBERDOC_SEC   || rootNav->section() == Entry::OVERLOADDOC_SEC ||
       rootNav->section() == Entry::FUNCTION_SEC    || rootNav->section() == Entry::VARIABLE_SEC ||
       rootNav->section() == Entry::VARIABLEDOC_SEC || rootNav->section() == Entry::DEFINE_SEC ||
       rootNav->section() == Entry::INCLUDED_SERVICE_SEC || rootNav->section() == Entry::EXPORTED_INTERFACE_SEC ) {

      rootNav->loadEntry(Doxy_Globals::g_storage);
      filterMemberDocumentation(rootNav);
      rootNav->releaseEntry();
   }
 
   for (auto e : rootNav->children() ) {
      if (e->section() != Entry::ENUM_SEC) {
         findMemberDocumentation(e);
      }      
   }
}

void Doxy_Work::findObjCMethodDefinitions(QSharedPointer<EntryNav> rootNav)
{  
   for (auto objCImplNav : rootNav->children() ) {
      if (objCImplNav->section() == Entry::OBJCIMPL_SEC) {

         for (auto objCMethodNav : objCImplNav->children() ) {

            if (objCMethodNav->section() == Entry::FUNCTION_SEC) {

               auto temp = objCMethodNav; 
               temp->loadEntry(Doxy_Globals::g_storage);

               QSharedPointer<Entry> objCMethod = temp->entry();
              
               findMember(temp, objCMethod->type + " " + objCImplNav->name() + "::" +
                          objCMethod->name + " " + objCMethod->args, false, true);

               objCMethod->section = Entry::EMPTY_SEC;

               temp->releaseEntry();
            }
         }
      }
   }
}

// find and add the enumeration to their classes, namespaces, or files
void Doxy_Work::findEnums(QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->section() == Entry::ENUM_SEC) {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();

      QSharedPointer<MemberDef>    md;
      QSharedPointer<ClassDef>     cd;
      QSharedPointer<FileDef>      fd;
      QSharedPointer<NamespaceDef> nd;

      MemberNameSDict *mnsd = 0;

      bool isGlobal;
      bool isRelated  = false;
      bool isMemberOf = false;

      int i;

      QString name;
      QString scope;

      if ((i = root->name.lastIndexOf("::")) != -1) { 
         // scope is specified

         scope = root->name.left(i);                             // extract scope
         name  = root->name.right(root->name.length() - i - 2);  // extract name

         cd = getClass(scope);

         if (cd == 0) {
            nd = getResolvedNamespace(scope);
         }

      } else { 
         // no scope, check the scope in which the docs where found

         if (( rootNav->parent()->section() & Entry::SCOPE_MASK ) && !rootNav->parent()->name().isEmpty()) { 
            // found enum docs inside a compound
            scope = rootNav->parent()->name();

            cd = getClass(scope);

            if (cd == 0) {
               nd = getResolvedNamespace(scope);
            }
         }

         name = root->name;
      }

      if (! root->relates.isEmpty()) {
         // related member, prefix user specified scope

         isRelated = true;
         isMemberOf = (root->relatesType == MemberOf);

         if (getClass(root->relates) == 0 && ! scope.isEmpty()) {
            scope = mergeScopes(scope, root->relates);

         } else {
            scope = root->relates;
         }

         if ((cd = getClass(scope)) == 0) {
            nd = getResolvedNamespace(scope);
         }
      }

      if (cd && ! name.isEmpty()) { 
         // found a enum inside a compound        

         fd   = QSharedPointer<FileDef>();
         mnsd = Doxy_Globals::memberNameSDict;
         isGlobal = false;

      } else if (nd && ! nd->name().isEmpty() && nd->name().at(0) != '@') { 
         // found enum inside namespace

         mnsd = Doxy_Globals::functionNameSDict;
         isGlobal = true;

      } else { 
         // found a global enum

         fd = rootNav->fileDef();
         mnsd = Doxy_Globals::functionNameSDict;
         isGlobal = true;
      }

      if (! name.isEmpty()) {
         // new enum type

         md = QMakeShared<MemberDef>(root->fileName, root->startLine, root->startColumn,
                        nullptr, name, nullptr, nullptr, root->protection, Normal, false, 
                        isMemberOf ? Foreign : isRelated ? Related : Member,
                        MemberType_Enumeration, nullptr, nullptr); 

         md->setTagInfo(rootNav->tagInfo());
         md->setLanguage(root->lang);
         md->setId(root->id);

         if (isGlobal) {      
            md->setFileDef(fd);
         } else {
            md->setMemberClass(cd);            
         }

         md->setBodySegment(root->bodyLine, root->endBodyLine);
         md->setBodyDef(rootNav->fileDef());
         md->setMemberTraits(root->m_traits);
         md->setEnumBaseType(root->args);
         
         md->addSectionsToDefinition(root->anchors);
         md->setMemberGroupId(root->mGrpId);
         md->enableCallGraph(root->callGraph);
         md->enableCallerGraph(root->callerGraph);
                 
         md->setRefItems(root->sli);
         
         bool defSet = false;

         QString baseType = root->args;
         if (! baseType.isEmpty()) {
            baseType.prepend(" : ");
         }

         if (nd && ! nd->name().isEmpty() && nd->name().at(0) != '@') {

            if (isRelated || Config::getBool("hide-scope-names")) {
               md->setDefinition(name + baseType);
            } else {
               md->setDefinition(nd->name() + "::" + name + baseType);
            }          

            defSet = true;
            md->setNamespace(nd);
            nd->insertMember(md);
         }

         // even if we have already added the enum to a namespace, we still
         // also want to add it to other appropriate places such as file or class
         
         if (isGlobal) {

            if (! defSet) {
               md->setDefinition(name + baseType);
            }

            if (fd == nullptr && rootNav->parent()) {
               fd = rootNav->parent()->fileDef();
            }

            if (fd) {
               md->setFileDef(fd);
               fd->insertMember(md);
            }

         } else if (cd) {
            if (isRelated || Config::getBool("hide-scope-names")) {
               md->setDefinition(name + baseType);

            } else {
               md->setDefinition(cd->name() + "::" + name + baseType);
            }

            cd->insertMember(md);
            cd->insertUsedFile(fd);
         }

         md->setDocumentation(root->doc, root->docFile, root->docLine);
         md->setDocsForDefinition(!root->proto);
         md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
         md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
       
         QSharedPointer<MemberName> mn;

         if ((mn = (*mnsd)[name])) {
            // same enum was found in multiple namespaces/classes
            mn->append(md);

         } else {
            // new enum name
            mn = QMakeShared<MemberName>(name);

            mn->append(md);
            mnsd->insert(name, mn);           
         }

         addMemberToGroups(root, md);
      }

      rootNav->releaseEntry();

   } else {
      RECURSE_ENTRYTREE(findEnums, rootNav);

   }
}

void Doxy_Work::addEnumValuesToEnums(QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->section() == Entry::ENUM_SEC) {
      // non anonymous enumeration
   
      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();

      QSharedPointer<ClassDef> cd = QSharedPointer<ClassDef>();
      QSharedPointer<FileDef>  fd = QSharedPointer<FileDef>();

      QSharedPointer<NamespaceDef> nd;
      MemberNameSDict *mnsd = nullptr;

      bool isGlobal;
      bool isRelated = false;
      
      int i;

      QString name;
      QString scope;

      if ((i = root->name.lastIndexOf("::")) != -1) { 
         // scope is specified

         scope = root->name.left(i); // extract scope
         name  = root->name.right(root->name.length() - i - 2); // extract name

         if ((cd = getClass(scope)) == 0) {
            nd = getResolvedNamespace(scope);
         }

      } else { 
         // no scope, check the scope in which the docs where found

         if (( rootNav->parent()->section() & Entry::SCOPE_MASK ) && ! rootNav->parent()->name().isEmpty()) {
            // found enum docs inside a compound
            scope = rootNav->parent()->name();

            if ((cd = getClass(scope)) == 0) {
               nd = getResolvedNamespace(scope);
            }
         }

         name = root->name;
      }

      if (! root->relates.isEmpty()) {
         // related member, prefix user specified scope
         isRelated = true;

         if (getClass(root->relates) == 0 && !scope.isEmpty()) {
            scope = mergeScopes(scope, root->relates);

         } else {
            scope = root->relates;
         }

         if ((cd = getClass(scope)) == 0) {
            nd = getResolvedNamespace(scope);
         }
      }

      if (cd && ! name.isEmpty()) { 
         // found an enum inside a compound        

         fd   = QSharedPointer<FileDef>();
         mnsd = Doxy_Globals::memberNameSDict;
         isGlobal = false;

      } else if (nd && !nd->name().isEmpty() && nd->name().at(0) != '@') { 
         // found enum inside namespace       
  
         mnsd = Doxy_Globals::functionNameSDict;
         isGlobal = true;

      } else { 
         // found a global enum

         fd = rootNav->fileDef();         
         mnsd = Doxy_Globals::functionNameSDict;
         isGlobal = true;
      }

      if (! name.isEmpty()) {         
         QSharedPointer<MemberName> mn = mnsd->find(name); 

         // for all members with this name
         if (mn) {          

            for (auto md : *mn) {
               // for each enum in this list

               if (md->isEnumerate()) {
                  
                  for (auto e : rootNav->children() ) {                     
                     SrcLangExt sle;

                     bool isStrong = root->m_traits.hasTrait(Entry::Virtue::Strong);

                     if ( (sle = rootNav->lang()) == SrcLangExt_CSharp || sle == SrcLangExt_Java ||
                              sle == SrcLangExt_XML || isStrong) {

                        // Unlike classic C/C++ enums, C++11, C# & Java enum values are only
                        // visible inside the enum scope, must create them here and only add them to the enum

                        e->loadEntry(Doxy_Globals::g_storage);
                        QSharedPointer<Entry> root = e->entry();

                        QString qualifiedName = substitute(rootNav->name(),"::",".");

                        if (! scope.isEmpty() && rootNav->tagInfo()) {
                           qualifiedName = substitute(scope,"::",".") + "." + qualifiedName;
                        }
                        
                        // TODO: add function to get canonical representation

                        if (substitute(md->qualifiedName(), "::", ".") == qualifiedName) {

                           // enum value scope matches that of the enum
                           // be less strict for tag files as members can have incomplete scope

                           QString fileName = root->fileName;

                           if (fileName.isEmpty() && rootNav->tagInfo()) {
                              fileName = rootNav->tagInfo()->tagName;
                           }

                           QSharedPointer<MemberDef> fmd = QMakeShared<MemberDef>(fileName, root->startLine, 
                              root->startColumn, root->type, root->name, root->args, nullptr, Public, Normal, 
                              root->stat, Member, MemberType_EnumValue, nullptr, nullptr);

                           if (md->getClassDef()) {
                              fmd->setMemberClass(md->getClassDef());

                           } else if (md->getNamespaceDef()) {
                              fmd->setNamespace(md->getNamespaceDef());

                           } else if (md->getFileDef()) {
                              fmd->setFileDef(md->getFileDef());

                           }

                           fmd->setOuterScope(md->getOuterScope());
                           fmd->setTagInfo(e->tagInfo());
                           fmd->setLanguage(root->lang);
                           fmd->setId(root->id);
                           fmd->setDocumentation(root->doc, root->docFile, root->docLine);
                           fmd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
                           fmd->addSectionsToDefinition(root->anchors);
                           fmd->setInitializer(root->initializer);
                           fmd->setMaxInitLines(root->initLines);
                           fmd->setMemberGroupId(root->mGrpId);
                           fmd->setExplicitExternal(root->explicitExternal);
                           fmd->setRefItems(root->sli);
                           fmd->setAnchor();

                           md->insertEnumField(fmd);
                           fmd->setEnumScope(md, true);

                           QSharedPointer<MemberName> mn = mnsd->find(root->name);

                           if (mn) {
                              mn->append(fmd);

                           } else {
                              mn = QMakeShared<MemberName>(root->name);
                              mn->append(fmd);
                              mnsd->insert(root->name, mn);
                           }
                        }

                        e->releaseEntry();

                     } else {                       
                        // c++ 

                        QSharedPointer<MemberName> fmn;
                        MemberNameSDict *enumDict;

                        if (isRelated) {
                           enumDict = Doxy_Globals::functionNameSDict;
                        } else {
                           enumDict = mnsd;
                        }

                        QString valueName = e->name();                          

                        if (! valueName.isEmpty()) { 
                           fmn = enumDict->find(valueName);
                        }

                        if (fmn) {
                           // get a list of members with the same name as the enum field
                                                        
                           for (auto fmd : *fmn) {   
                              if (fmd->isEnumValue() && fmd->getOuterScope() == md->getOuterScope()) { 
                                 // in same scope
                                
                                 if (nd && ! nd->name().isEmpty() && nd->name().at(0) != '@') {
                                    QSharedPointer<NamespaceDef> fnd = fmd->getNamespaceDef();

                                    if (fnd == nd) { 
                                       // enum value is inside a namespace
                                       md->insertEnumField(fmd);
                                       fmd->setEnumScope(md);
                                    }

                                 } else if (isGlobal) {
                                    QSharedPointer<FileDef> ffd = fmd->getFileDef();

                                    if (ffd == fd) { 
                                       // enum value has file scope
                                       md->insertEnumField(fmd);
                                       fmd->setEnumScope(md);
                                    }

                                 } else if (isRelated && cd)  {
                                    // reparent enum value to match the enum's scope
                              
                                    md->insertEnumField(fmd);          // add field def to list
                                    fmd->setEnumScope(md);             // cross ref with enum name
                                    fmd->setEnumClassScope(cd);        // cross ref with enum name
                                    fmd->setOuterScope(cd);
                                    fmd->makeRelated();
                                    cd->insertMember(fmd);

                                 } else {
                                    QSharedPointer<ClassDef> fcd = fmd->getClassDef();

                                    if (fcd == cd) { 
                                       // enum value is inside a class
                               
                                       md->insertEnumField(fmd);  // add field def to list
                                       fmd->setEnumScope(md);     // cross ref with enum name
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }

      rootNav->releaseEntry();

   } else {
      RECURSE_ENTRYTREE(addEnumValuesToEnums, rootNav);
   }
}

// find the documentation blocks for the enumerations
void Doxy_Work::findEnumDocumentation(QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->section() == Entry::ENUMDOC_SEC && ! rootNav->name().isEmpty() && rootNav->name().at(0) != '@') {
      // skip anonymous enums
      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();

      int i;
      QString name;
      QString scope;

      if ((i = root->name.lastIndexOf("::")) != -1) { // scope is specified as part of the name
         name  = root->name.right(root->name.length() - i - 2); // extract name
         scope = root->name.left(i); // extract scope
        
      } else { 
         // just the name
         name = root->name;
      }

      if (( rootNav->parent()->section() & Entry::SCOPE_MASK ) && !rootNav->parent()->name().isEmpty()) { 
         // found enum docs inside a compound
         if (! scope.isEmpty()) {
            scope.prepend("::");
         }

         scope.prepend(rootNav->parent()->name());
      }

      QSharedPointer<ClassDef> cd = getClass(scope);

      if (! name.isEmpty()) {
         bool found = false;

         if (cd) {            
            QString className = cd->name();
            QSharedPointer<MemberName> mn = Doxy_Globals::memberNameSDict->find(name);

            if (mn) {             
               for (auto md : *mn) {   
                  if (found) {
                     break;
                  }

                  QSharedPointer<ClassDef> cd = md->getClassDef();

                  if (cd && cd->name() == className && md->isEnumerate()) {
                     // documentation outside a compound overrides the documentation inside it
                     
                     md->setDocumentation(root->doc, root->docFile, root->docLine);
                     md->setDocsForDefinition(! root->proto);
                     
                     // brief descriptions inside a compound override the documentation outside it                     
                     md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
                     
                     if (md->inbodyDocumentation().isEmpty() || ! rootNav->parent()->name().isEmpty()) {
                        md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
                     }

                     if (root->mGrpId != -1 && md->getMemberGroupId() == -1) {
                        md->setMemberGroupId(root->mGrpId);
                     }

                     md->addSectionsToDefinition(root->anchors);
                     md->setRefItems(root->sli);

                     QSharedPointer<GroupDef> gd = md->getGroupDef();

                     if (gd == 0 && ! root->groups->isEmpty()) { 
                        // member not grouped but out-of-line documentation is
                        addMemberToGroups(root, md);
                     }

                     found = true;
                  }
               }
            } else {
               
            }

         } else { 
            // enum outside class, most likley in a namaspace            

            QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict->find(name);

            if (mn) {              
               for (auto md : *mn) {

                  if (found) {    
                     break; 
                  }    

                  // ensure our MemberDef is the correct namespce
                  QSharedPointer<NamespaceDef> nd = md->getNamespaceDef();

                  if (nd && nd->name() != scope)  {
                     continue;
                  }

                  if (md->isEnumerate()) {
                     md->setDocumentation(root->doc, root->docFile, root->docLine);
                     md->setDocsForDefinition(! root->proto);
                     md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
                     md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
                     md->addSectionsToDefinition(root->anchors);
                     md->setMemberGroupId(root->mGrpId);

                     QSharedPointer<GroupDef> gd = md->getGroupDef();

                     if (gd == 0 && ! root->groups->isEmpty()) {
                         // member not grouped but out-of-line documentation is
                        addMemberToGroups(root, md);
                     }

                     found = true;
                  }
               }
            }
         }

         if (! found) {
            warn(root->fileName, root->startLine, "Documentation for undefined enum `%s' found.", csPrintable(name) );
         }
      }

      rootNav->releaseEntry();
   }

   RECURSE_ENTRYTREE(findEnumDocumentation, rootNav);
}

// search for each enum (member or function) in mnl if it has documented enum values
void Doxy_Work::findDEV(const MemberNameSDict &mnsd)
{
   // for each member name
   for (auto mn : mnsd)  {

      // for each member definition
      for (auto md : *mn)  {

         if (md->isEnumerate()) {
            // member is an enum
            QSharedPointer<MemberList> fmdl = md->enumFieldList();
            int documentedEnumValues = 0;

            if (fmdl) {
               // enum has values, for each enum value

               for (auto fmd : *fmdl)  {
                  if (fmd->isLinkableInProject()) {
                     documentedEnumValues++;
                  }
               }
            }

            // at least one enum value is documented
            if (documentedEnumValues > 0) {
               md->setDocumentedEnumValues(true);
            }
         }
      }
   }
}

// search for each enum (member or function) if it has documented enum vales
void Doxy_Work::findDocumentedEnumValues()
{
   findDEV(*Doxy_Globals::memberNameSDict);
   findDEV(*Doxy_Globals::functionNameSDict);
}

void Doxy_Work::addMembersToIndex()
{
   // for each member name
   for (auto mn : *Doxy_Globals::memberNameSDict) { 
      
      // for each member definition
      for (auto md : *mn) {
         addClassMemberNameToIndex(md);
      }
   }

   // for each member name 
   for (auto mn : *Doxy_Globals::functionNameSDict) {
     
      // for each member definition
      for (auto md : *mn) {
         if (md->getNamespaceDef()) {
            addNamespaceMemberNameToIndex(md);
         } else {
            addFileMemberNameToIndex(md);
         }
      }
   }
}

// computes the relation between all members. For each member `m'
// the members which override the implementation of `m' are searched and
// the member that `m' overrides is searched.

void Doxy_Work::computeMemberRelations()
{
   for (auto mn : *Doxy_Globals::memberNameSDict) { 
      // for each member name
       
     for (auto md : *mn) { 
         // for each member with a specific name

         for (auto bmd : *mn) { 
            // for each other member with the same name
            QSharedPointer<ClassDef> mcd  = md->getClassDef();

            if (mcd && mcd->baseClasses()) {
               QSharedPointer<ClassDef> bmcd = bmd->getClassDef();
             
               if (md != bmd && bmcd && mcd && bmcd != mcd &&
                     (bmd->virtualness() != Normal || bmcd->compoundType() == ClassDef::Interface ||
                      bmcd->compoundType() == ClassDef::Protocol) &&
                      md->isFunction() && mcd->isLinkable() && bmcd->isLinkable() && mcd->isBaseClass(bmcd, true)) {
                 
                  ArgumentList *bmdAl = bmd->argumentList();
                  ArgumentList *mdAl =  md->argumentList();
                 
                  if (matchArguments2(bmd->getOuterScope(), bmd->getFileDef(), bmdAl,
                                     md->getOuterScope(), md->getFileDef(), mdAl,true)) {

                     QSharedPointer<MemberDef> rmd;
                     if ((rmd = md->reimplements()) == 0 || 
                          minClassDistance(mcd, bmcd) < minClassDistance(mcd, rmd->getClassDef())) {

                        md->setReimplements(bmd);
                     }
                    
                     bmd->insertReimplementedBy(md);
                  }
               }
            }
         }
      }
   }
}

void Doxy_Work::createTemplateInstanceMembers()
{   
   // for each class
   for (auto cd : *Doxy_Globals::classSDict) {       
      QHash<QString, QSharedPointer<ClassDef>> *templInstances = cd->getTemplateInstances();

      if (templInstances) {             
         // for each instance of the template

         for (auto iter = templInstances->begin(); iter != templInstances->end(); ++iter) {
            (*iter)->addMembersToTemplateInstance(cd, csPrintable(iter.key()));
         }
      }
   }
}

void Doxy_Work::mergeCategories()
{   
   // merge members of categories into the class they extends   
   for (auto cd : *Doxy_Globals::classSDict) { 
      int i = cd->name().indexOf('(');

      if (i != -1) { 
         // it is an Objective-C category
         QString baseName = cd->name().left(i);
         QSharedPointer<ClassDef> baseClass = Doxy_Globals::classSDict->find(baseName);

         if (baseClass) {          
            baseClass->mergeCategory(cd);
         }
      }
   }
}

// builds the list of all members for each class
void Doxy_Work::buildCompleteMemberLists()
{
   // merge the member list of base classes into the inherited classes   
   for (auto cd : *Doxy_Globals::classSDict) { 

      if (cd->subClasses() == 0 && cd->baseClasses()) {         
         cd->mergeMembers();
      }
   }
}

void Doxy_Work::generateSourceCode()
{
   if (Doxy_Globals::inputNameList->count() > 0) {
      static bool clangParsing = Config::getBool("clang-parsing");

      if (clangParsing) {     
         QSet<QString> processedFiles;         
         QSet<QString> filesToProcess;
                    
         for (auto fn : *Doxy_Globals::inputNameList) {
            for (auto fd : *fn) {
               filesToProcess.insert(fd->getFilePath());
            }
         }

         // process source files (and their include dependencies)
         for (auto fn : *Doxy_Globals::inputNameList) {   
           
            for (auto fd : *fn) {
               if (fd->isSource() && ! fd->isReference()) {
                  QStringList includeFiles;

                  fd->getAllIncludeFilesRecursively(includeFiles);
                  fd->startParsing();

                  if (fd->generateSourceFile()) { 
                     // source needs to be shown in the output

                     msg("Generating code for file %s\n", csPrintable(fd->docName()));
                     fd->writeSource(*Doxy_Globals::g_outputList, false, includeFiles);

                  } else if (! fd->isReference() && Doxy_Globals::parseSourcesNeeded) {
                     // parse the source even if not shown                 

                     msg("Parsing code for file %s\n", csPrintable(fd->docName()));
                     fd->parseSource(false, includeFiles);
                  }

                  for (QString incFile : includeFiles) {

                     if (fd->getFilePath() != incFile && ! processedFiles.contains(incFile)) {
                        QStringList moreFiles;
                        bool ambig;

                        QSharedPointer<FileDef> ifd = findFileDef(Doxy_Globals::inputNameDict, incFile, ambig);

                        if (ifd && ! ifd->isReference()) {

                           if (ifd->generateSourceFile()) { 
                              // source needs to be shown in the output

                              msg("Generating code for file %s\n", csPrintable(ifd->docName()));
                              ifd->writeSource(*Doxy_Globals::g_outputList, true, moreFiles);

                           } else if (Doxy_Globals::parseSourcesNeeded)  {
                              // need to parse the sources even if we do not show it                           

                              msg("Parsing code for file %s\n", csPrintable(ifd->docName()));
                              ifd->parseSource(true, moreFiles);
                           }

                           processedFiles.insert(incFile);
                        }
                     }                     
                  }

                  fd->finishParsing();
                  processedFiles.insert(fd->getFilePath());
               }
            }
         }

         // process remaining files
         for (auto fn : *Doxy_Globals::inputNameList) {         

            for (auto fd : *fn) {
               if (! processedFiles.contains(fd->getFilePath())) { 
                 
                  QStringList includeFiles;
                  fd->startParsing();

                  if (fd->generateSourceFile()) { 
                     // source needs to be shown in the output

                     msg("Generating code for file %s\n", csPrintable(fd->docName()));
                     fd->writeSource(*Doxy_Globals::g_outputList, false, includeFiles);

                  } else if (! fd->isReference() && Doxy_Globals::parseSourcesNeeded)  {
                     // needed to parse the source even if we do not show it
                  
                     msg("Parsing code for file %s\n", csPrintable(fd->docName()));
                     fd->parseSource(false, includeFiles);
                  }

                  fd->finishParsing();
               }
            }
         }

      } else {
         // use lex and not clang      

         for (auto fn : *Doxy_Globals::inputNameList) {

           for (auto fd : *fn) {
               QStringList includeFiles;
               fd->startParsing();

               if (fd->generateSourceFile()) {
                  // source needs to be shown in the output
                  msg("Generating code for file %s\n", csPrintable(fd->docName()));

                  fd->writeSource(*Doxy_Globals::g_outputList, false, includeFiles);

               } else if (! fd->isReference() && Doxy_Globals::parseSourcesNeeded) {
                  // parse the sources even if we do not show it
            
                  msg("Parsing code for file %s\n",  csPrintable(fd->docName()));
                  fd->parseSource(false, includeFiles);
               }

               fd->finishParsing();
            }
         }
      }
   }
}

void Doxy_Work::generateFileDocs()
{
   if (Doxy_Globals::documentedHtmlFiles == 0) {
      return;
   }

   if (Doxy_Globals::inputNameList->count() > 0) {
     
      for (auto fn : *Doxy_Globals::inputNameList) {

         for (auto fd : *fn) {
            bool doc = fd->isLinkableInProject();

            if (doc) {
               msg("Generating docs for file %s\n",  csPrintable(fd->docName()));
               fd->writeDocumentation(*Doxy_Globals::g_outputList);
            }
         }
      }
   }
}

void Doxy_Work::addSourceReferences()
{
   // add source references for class definitions
   for (auto cd : *Doxy_Globals::classSDict) {
      QSharedPointer<FileDef> fd = cd->getBodyDef();

      if (fd && cd->isLinkableInProject() && cd->getStartBodyLine() != -1) {
         fd->addSourceRef(cd->getStartBodyLine(), cd, QSharedPointer<MemberDef>());
      }
   }

   // add source references for namespace definitions
   for (auto nd : *Doxy_Globals::namespaceSDict) {
      QSharedPointer<FileDef> fd = nd->getBodyDef();

      if (fd && nd->isLinkableInProject() && nd->getStartBodyLine() != -1) {
         fd->addSourceRef(nd->getStartBodyLine(), nd, QSharedPointer<MemberDef>());
      }
   }

   // add source references for member names
   for (auto mn: *Doxy_Globals::memberNameSDict) {

      for (auto md : *mn) {
         QSharedPointer<FileDef> fd = md->getBodyDef();

         if (fd && md->getStartBodyLine() != -1 && md->isLinkableInProject() &&
               (fd->generateSourceFile() || Doxy_Globals::parseSourcesNeeded) ) {
            fd->addSourceRef(md->getStartBodyLine(), md->getOuterScope(), md);
         }
      }
   }

   for (auto mn : *Doxy_Globals::functionNameSDict) {   
    
      for (auto md : *mn) {
         QSharedPointer<FileDef> fd = md->getBodyDef();

         if (fd && md->getStartBodyLine() != -1 && md->isLinkableInProject() &&
               (fd->generateSourceFile() || Doxy_Globals::parseSourcesNeeded)) {

            fd->addSourceRef(md->getStartBodyLine(), md->getOuterScope(), md);
         }
      }
   }
}

// generate the documentation of all classes
void Doxy_Work::generateClassList(ClassSDict &classSDict)
{
   for (auto cd : classSDict) {
            
      if (cd && (cd->getOuterScope() == 0 || // <-- should not happen, but can if we read an old tag file
                 cd->getOuterScope() == Doxy_Globals::globalScope) && ! cd->isHidden() && ! cd->isEmbeddedInOuterScope()) {

         // skip external references, anonymous compounds and template instances

         if ( cd->isLinkableInProject() && cd->templateMaster() == 0) {
            msg("Generating docs for compound %s\n", csPrintable(cd->name()));    

            cd->writeDocumentation(*Doxy_Globals::g_outputList);
            cd->writeMemberList(*Doxy_Globals::g_outputList);
         }

         // even for undocumented classes, the inner classes can be documented.
         cd->writeDocumentationForInnerClasses(*Doxy_Globals::g_outputList);
      }
   }
}

void Doxy_Work::generateClassDocs()
{
   generateClassList(*Doxy_Globals::classSDict);
   generateClassList(*Doxy_Globals::hiddenClasses);
}

void Doxy_Work::inheritDocumentation()
{
   for (auto mn : *Doxy_Globals::memberNameSDict) {
      for (auto md : *mn) {

         if (md->documentation().isEmpty() && md->briefDescription().isEmpty()) {
            // no documentation yet

            QSharedPointer<MemberDef> bmd = md->reimplements();

            while (bmd && bmd->documentation().isEmpty() && bmd->briefDescription().isEmpty() ) {
               // search up the inheritance tree for a documentation member             
               bmd = bmd->reimplements();
            }

            if (bmd) { 
               // copy the documentation from the reimplemented member

               md->setInheritsDocsFrom(bmd);
               md->setDocumentation(bmd->documentation(), bmd->docFile(), bmd->docLine());
               md->setDocsForDefinition(bmd->isDocsForDefinition());
               md->setBriefDescription(bmd->briefDescription(), bmd->briefFile(), bmd->briefLine());
               md->copyArgumentNames(bmd);
               md->setInbodyDocumentation(bmd->inbodyDocumentation(), bmd->inbodyFile(), bmd->inbodyLine());
            }
         }
      }
   }
}

void Doxy_Work::combineUsingRelations()
{
   // for each file 
   for (auto &fn : *Doxy_Globals::inputNameList) {  
      for (auto &fd : *fn) {
         fd->visited = false;
      }
   }

   for (auto &fn : *Doxy_Globals::inputNameList) {
     for (auto &fd : *fn) {
         fd->combineUsingRelations();
      }
   }

   // for each namespace  
   for (auto nd : *Doxy_Globals::namespaceSDict) {
      nd->visited = false;
   }

   for (auto nd : *Doxy_Globals::namespaceSDict) {
      nd->combineUsingRelations();
   }
}

void Doxy_Work::addMembersToMemberGroup()
{
   // for each class
   for (auto cd : *Doxy_Globals::classSDict) {
      cd->addMembersToMemberGroup();
   }

   // for each file
   for (auto &fn : *Doxy_Globals::inputNameList) {
      for (auto &fd : *fn) {
         fd->addMembersToMemberGroup();
      }
   }

   // for each namespace
   for (auto nd : *Doxy_Globals::namespaceSDict) {
      nd->addMembersToMemberGroup();
   }

   // for each group
   for (auto gd : *Doxy_Globals::groupSDict) {
      gd->addMembersToMemberGroup();
   }
}

void Doxy_Work::distributeMemberGroupDocumentation()
{
   // for each class
   for (auto cd : *Doxy_Globals::classSDict) {
      cd->distributeMemberGroupDocumentation();
   }

   // for each file
   for (auto fn : *Doxy_Globals::inputNameList) {
      for (auto fd : *fn) {
         fd->distributeMemberGroupDocumentation();
      }
   }

   // for each namespace
   for (auto nd : *Doxy_Globals::namespaceSDict) {
      nd->distributeMemberGroupDocumentation();
   }

   // for each group
   for (auto gd : *Doxy_Globals::groupSDict) {
      gd->distributeMemberGroupDocumentation();
   }
}

void Doxy_Work::findSectionsInDocumentation()
{

   // for each class
   for (auto cd : *Doxy_Globals::classSDict) {
      cd->findSectionsInDocumentation();
   }

   // for each file
   for (auto fn : *Doxy_Globals::inputNameList) {
     for (auto fd : *fn) {
         fd->findSectionsInDocumentation();
      }
   }

   // for each namespace
   for (auto nd : *Doxy_Globals::namespaceSDict) {
      nd->findSectionsInDocumentation();
   }

   // for each group
   for (auto gd : *Doxy_Globals::groupSDict) {
      gd->findSectionsInDocumentation();
   }

   // for each page
   for (auto pd : *Doxy_Globals::pageSDict) {
      pd->findSectionsInDocumentation();
   }

   if (Doxy_Globals::mainPage) {
      Doxy_Globals::mainPage->findSectionsInDocumentation();
   }
}

void Doxy_Work::flushCachedTemplateRelations()
{
   // remove all references to classes from the cache
   // as there can be new template instances in the inheritance path
   // to this class. Optimization: only remove those classes that
   // have inheritance instances as direct or indirect sub classes.

   Doxy_Globals::lookupCache->clear();

   // remove all cached typedef resolutions whose target is a
   // template class as this may now be a template instance

   for (auto fn : *Doxy_Globals::functionNameSDict) {
      // for each global function name

      for (auto fmd : *fn) {
         // for each function with that name

         if (fmd->isTypedefValCached()) {
            QSharedPointer<ClassDef> cd = fmd->getCachedTypedefVal();

            if (cd->isTemplate()) {
               fmd->invalidateTypedefValCache();
            }
         }
      }
   }

   for (auto fn : *Doxy_Globals::memberNameSDict) {
      // for each class method name

      for (auto fmd : *fn) {
         // for each function with that name

         if (fmd->isTypedefValCached()) {
            QSharedPointer<ClassDef> cd = fmd->getCachedTypedefVal();

            if (cd->isTemplate()) {
               fmd->invalidateTypedefValCache();
            }
         }
      }
   }
}

void Doxy_Work::flushUnresolvedRelations()
{
   // Remove all unresolved references to classes from the cache.
   // This is needed before resolving the inheritance relations, since
   // it would otherwise not find the inheritance relation
   // for C in the example below, as B::I was already found to be unresolvable
   // (which is correct if you igore the inheritance relation between A and B).
   //
   // class A { class I {} };
   // class B : public A {};
   // class C : public B::I {};
   
   Doxy_Globals::lookupCache->clear();
 
   for (auto fn : *Doxy_Globals::functionNameSDict) {
      // for each global function name
    
      for (auto fmd : *fn) {
         // for each function with that name
         fmd->invalidateCachedArgumentTypes();
      }
   }
  
   for (auto fn : *Doxy_Globals::memberNameSDict) { 
      // for each class method name
     
      for (auto fmd : *fn) { 
         // for each function with that name
         fmd->invalidateCachedArgumentTypes();
      }
   }
}

void Doxy_Work::findDefineDocumentation(QSharedPointer<EntryNav> rootNav)
{
   if ((rootNav->section() == Entry::DEFINEDOC_SEC ||
         rootNav->section() == Entry::DEFINE_SEC) && !rootNav->name().isEmpty()) {

      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();

      if (rootNav->tagInfo() && ! root->name.isEmpty()) { 

         // define read from a tag file
         QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(rootNav->tagInfo()->tagName, 
                  1, 1, "#define", root->name, root->args, nullptr,
                  Public, Normal, false, Member, MemberType_Define, nullptr, nullptr);

         md->setTagInfo(rootNav->tagInfo());
         md->setLanguage(root->lang);         
         md->setFileDef(rootNav->parent()->fileDef());
       
         QSharedPointer<MemberName> mn;

         if ((mn = Doxy_Globals::functionNameSDict->find(root->name))) {
            mn->append(md);

         } else {
            mn = QMakeShared<MemberName>(root->name); 

            mn->append(md);
            Doxy_Globals::functionNameSDict->insert(root->name, mn);
         }
      }

      QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict->find(root->name);

      if (mn) {    
         int count = 0;
   
         for (auto md : *mn) {
            if (md->memberType() == MemberType_Define) {
               count++;
            }
         }

         if (count == 1) {          
            for (auto md : *mn) {
               if (md->memberType() == MemberType_Define) {
                  md->setDocumentation(root->doc, root->docFile, root->docLine);
                  md->setDocsForDefinition(!root->proto);
                  md->setBriefDescription(root->brief, root->briefFile, root->briefLine);

                  if (md->inbodyDocumentation().isEmpty()) {
                     md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
                  }

                  md->setBodySegment(root->bodyLine, root->endBodyLine);
                  md->setBodyDef(rootNav->fileDef());
                  md->addSectionsToDefinition(root->anchors);
                  md->setMaxInitLines(root->initLines);
                  md->setRefItems(root->sli);

                  if (root->mGrpId != -1) {
                     md->setMemberGroupId(root->mGrpId);
                  }

                  addMemberToGroups(root, md);
               }
            }

         } else if (count > 1 && (! root->doc.isEmpty() || !root->brief.isEmpty() || root->bodyLine != -1))
            // multiple defines don't know where to add docs
            // but maybe they are in different files together with their documentation
                            
            for (auto md : *mn) {

               if (md->memberType() == MemberType_Define) {
                  QSharedPointer<FileDef> fd = md->getFileDef();

                  if (fd && fd->getFilePath() == root->fileName) {
                     // doc and define in the same file assume they belong together.
                                       
                     md->setDocumentation(root->doc, root->docFile, root->docLine);
                     md->setDocsForDefinition(!root->proto);                                          
                     md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
                     
                     if (md->inbodyDocumentation().isEmpty()) {
                        md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
                     }

                     md->setBodySegment(root->bodyLine, root->endBodyLine);
                     md->setBodyDef(rootNav->fileDef());
                     md->addSectionsToDefinition(root->anchors);
                     md->setRefItems(root->sli);
                     md->setLanguage(root->lang);

                     if (root->mGrpId != -1) {
                        md->setMemberGroupId(root->mGrpId);
                     }

                     addMemberToGroups(root, md);
                  }
               }
            }
           
      } else if (!root->doc.isEmpty() || !root->brief.isEmpty()) { // define not found
         static bool preEnabled = Config::getBool("enable-preprocessing");

         if (preEnabled) {
            warn(root->fileName, root->startLine, "documentation for unknown define %s found.\n",  
                  csPrintable(root->name) );

         } else {
            warn(root->fileName, root->startLine, "found documented #define, ignoring since "
                 "ENABLE PREPROCESSING tag is not set\n",  csPrintable(root->name) );
         }
      }

      rootNav->releaseEntry();
   }

   RECURSE_ENTRYTREE(findDefineDocumentation, rootNav);
}

void Doxy_Work::findDirDocumentation(QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->section() == Entry::DIRDOC_SEC) {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();

      QString normalizedName = root->name;
      normalizedName = substitute(normalizedName, "\\", "/");
     
      if (root->docFile == normalizedName) { // current dir?
         int lastSlashPos = normalizedName.lastIndexOf('/');

         if (lastSlashPos != -1) { // strip file name
            normalizedName = normalizedName.left(lastSlashPos);
         }
      }

      if (normalizedName.at(normalizedName.length() - 1) != '/') {
         normalizedName += '/';
      }

      QSharedPointer<DirDef> matchingDir;

      for (auto dir : Doxy_Globals::directories) {
  
         if (dir->name().right(normalizedName.length()) == normalizedName) {
            if (matchingDir) {

               warn(root->fileName, root->startLine, "\\dir command matches multiple directories.\n"
                    "  Applying the command for directory %s\n"
                    "  Ignoring the command for directory %s\n", csPrintable(matchingDir->name()), 
                    csPrintable(dir->name()) );

            } else {
               matchingDir = dir;

            }
         }
      }

      if (matchingDir) {         
         matchingDir->setBriefDescription(root->brief, root->briefFile, root->briefLine);
         matchingDir->setDocumentation(root->doc, root->docFile, root->docLine);
         matchingDir->setRefItems(root->sli);

         addDirToGroups(root, matchingDir);

      } else {
         warn(root->fileName, root->startLine, "No matching "
              "directory found for command \\dir %s\n",  csPrintable(normalizedName));
      }

      rootNav->releaseEntry();
   }

   RECURSE_ENTRYTREE(findDirDocumentation, rootNav);
}

// create a (sorted) list of separate documentation pages
void Doxy_Work::buildPageList(QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->section() == Entry::PAGEDOC_SEC) {

      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();

      if (! root->name.isEmpty()) {
         addRelatedPage(rootNav);
      }

      rootNav->releaseEntry();

   } else if (rootNav->section() == Entry::MAINPAGEDOC_SEC) {

      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();

      QString title = root->args.trimmed();

      if (title.isEmpty()) {
         title = theTranslator->trMainPage();
      }
      
      QString name = "index";
      addRefItem(root->sli, name, "page", name, title, 0, QSharedPointer<Definition>());

      rootNav->releaseEntry();  
   } 
 
   // recursive call
   for (auto item : rootNav->children() ) { 
      buildPageList(item);   
   }  
}

// search for \mainpage defined in this project
void Doxy_Work::findMainPage(QSharedPointer<EntryNav> rootNav)
{

   if (rootNav->section() == Entry::MAINPAGEDOC_SEC) {
      rootNav->loadEntry(Doxy_Globals::g_storage);

      if (Doxy_Globals::mainPage == nullptr && rootNav->tagInfo() == 0) {
         QSharedPointer<Entry> root = rootNav->entry();

         QString title = root->args.trimmed();

         QString indexName = "index";
         Doxy_Globals::mainPage = QMakeShared<PageDef>(root->docFile, root->docLine,
                                         indexName, root->brief + root->doc + root->inbodyDocs, title);
         
         Doxy_Globals::mainPage->setBriefDescription(root->brief, root->briefFile, root->briefLine);
         Doxy_Globals::mainPage->setFileName(indexName, true);
         Doxy_Globals::mainPage->setShowToc(root->stat);

         addPageToContext(Doxy_Globals::mainPage, rootNav);

         QSharedPointer<SectionInfo> si = Doxy_Globals::sectionDict->find(Doxy_Globals::mainPage->name());

         if (si) {
            if (si->lineNr != -1) {
               warn(root->fileName, root->startLine, "multiple use of section label '%s' for main page, "
                  "(first occurrence: %s, line %d)", csPrintable(Doxy_Globals::mainPage->name()), 
                   csPrintable(si->fileName), si->lineNr);

            } else {
               warn(root->fileName, root->startLine, "multiple use of section label '%s' for main page, (first occurrence: %s)", 
                    csPrintable(Doxy_Globals::mainPage->name()), csPrintable(si->fileName));
            }

         } else {
            // a page name is a label as well, but should not be double either
            si = QMakeShared<SectionInfo>(indexName, root->startLine, Doxy_Globals::mainPage->name(), 
                              Doxy_Globals::mainPage->title(), SectionInfo::Page, 0);

            Doxy_Globals::sectionDict->insert(indexName, si);
            Doxy_Globals::mainPage->addSectionsToDefinition(root->anchors);
         }

      } else if (rootNav->tagInfo() == 0) {
         QSharedPointer<Entry> root = rootNav->entry();

         warn(root->fileName, root->startLine, "found more than one \\mainpage comment block "
            "(first occurrence: %s, line %d), Skip current block", 
            csPrintable(Doxy_Globals::mainPage->docFile()), Doxy_Globals::mainPage->docLine());
      }

      rootNav->releaseEntry();
   }

   RECURSE_ENTRYTREE(findMainPage, rootNav);
}

// search for the main page imported via tag files and add only the section labels
void Doxy_Work::findMainPageTagFiles(QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->section() == Entry::MAINPAGEDOC_SEC) {
      rootNav->loadEntry(Doxy_Globals::g_storage);

      if (Doxy_Globals::mainPage && rootNav->tagInfo()) {
         QSharedPointer<Entry> root = rootNav->entry();
         Doxy_Globals::mainPage->addSectionsToDefinition(root->anchors);
      }
   }
   RECURSE_ENTRYTREE(findMainPageTagFiles, rootNav);
}

void Doxy_Work::computePageRelations(QSharedPointer<EntryNav> rootNav)
{
   if ((rootNav->section() == Entry::PAGEDOC_SEC || rootNav->section() == Entry::MAINPAGEDOC_SEC) 
               && !rootNav->name().isEmpty() ) {

      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();

      QSharedPointer<PageDef> pd = root->section == Entry::PAGEDOC_SEC ? Doxy_Globals::pageSDict->find(root->name) : Doxy_Globals::mainPage;

      if (pd) {       

         for (auto bi : root->extends) {
            QSharedPointer<PageDef> subPd = Doxy_Globals::pageSDict->find(bi.name);

            if (subPd) {
               pd->addInnerCompound(subPd);              
            }
         }
      }

      rootNav->releaseEntry();
   }

   RECURSE_ENTRYTREE(computePageRelations, rootNav);
}

void Doxy_Work::checkPageRelations()
{  
   for (auto pd : *Doxy_Globals::pageSDict) {
      QSharedPointer<Definition> ppd = pd->getOuterScope();

      while (ppd) {
         if (ppd == pd) {
            err("Page defined at line %d of file %s with label %s is a subpage "
                "of itself Remove this cyclic dependency.\n", pd->docLine(), 
                csPrintable(pd->docFile()), csPrintable(pd->name())); 

            stopDoxyPress();
         }

         ppd = ppd->getOuterScope();
      }
   }
}

void Doxy_Work::resolveUserReferences()
{
    for (auto si : *Doxy_Globals::sectionDict) {
      QSharedPointer<PageDef> pd;

      // hack: the items of a todo/test/bug/deprecated list are all fragments from
      // different files, so the resulting section's all have the wrong file
      // name (not from the todo/test/bug/deprecated list, but from the file in
      // which they are defined). We correct this here by looking at the generated section labels
    
      for (auto &rl : *Doxy_Globals::xrefLists) {
         QString label = "_" + rl.listName(); 

         // "_todo", "_test", ...

         if (si->label.left(label.length()) == label) {
            si->fileName = rl.listName();
            si->generated = true;

            break;
         }
      }
      
      if (! si->generated) {
         // if this section is in a page and the page is in a group, then we
         // have to adjust the link file name to point to the group.

         if (! si->fileName.isEmpty() && (pd = Doxy_Globals::pageSDict->find(si->fileName)) && pd->getGroupDef()) {
            si->fileName = pd->getGroupDef()->getOutputFileBase();
         }

         if (si->definition) {
            // TODO: there should be one function in Definition that returns
            // the file to link to, so we can avoid the following tests
            QSharedPointer<GroupDef> gd;

            if (si->definition->definitionType() == Definition::TypeMember) {
               QSharedPointer<MemberDef> temp = si->definition.dynamicCast<MemberDef>();
               gd = temp->getGroupDef();
            }

            if (gd) {
               si->fileName = gd->getOutputFileBase();

            } else {
               //  si->fileName=si->definition->getOutputFileBase();
               
            }
         }
      }     
   }
}

// generate all separate documentation pages
void Doxy_Work::generatePageDocs()
{   
   if (Doxy_Globals::documentedPages == 0) {
      return;
   }  

   for (auto pd : *Doxy_Globals::pageSDict) {

      if (! pd->getGroupDef() && ! pd->isReference()) {
         msg("Generating docs for page %s\n", csPrintable(pd->name()));

         Doxy_Globals::insideMainPage = true;
         pd->writeDocumentation(*Doxy_Globals::g_outputList);
         Doxy_Globals::insideMainPage = false;
      }
   }
}

// create a (sorted) list & dictionary of example pages
void Doxy_Work::buildExampleList(QSharedPointer<EntryNav> rootNav)
{
   if (rootNav->section() == Entry::EXAMPLE_SEC && !rootNav->name().isEmpty()) {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      QSharedPointer<Entry> root = rootNav->entry();

      if (Doxy_Globals::exampleSDict->find(root->name)) {
         warn(root->fileName, root->startLine, "Example %s was already documented. Ignoring "
              "documentation found here.", csPrintable(root->name));

      } else {
         QSharedPointer<PageDef> pd = QMakeShared<PageDef>(root->fileName, root->startLine,
                                   root->name, root->brief + root->doc + root->inbodyDocs, root->args);

         pd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
         pd->setFileName( csPrintable(convertNameToFile(pd->name() + "-example", false, true)), false);
         pd->addSectionsToDefinition(root->anchors);
         pd->setLanguage(root->lang);

         Doxy_Globals::exampleSDict->insert(root->name, pd);
      }

      rootNav->releaseEntry();
   }
   RECURSE_ENTRYTREE(buildExampleList, rootNav);
}

// print the entry tree (for debugging)
void printNavTree(QSharedPointer<EntryNav> rootNav, int indent)
{
   QString indentStr;
   indentStr.fill(' ', indent);

   msg("%s%s (section = 0x%x)\n", indentStr.isEmpty() ? "" : csPrintable(indentStr),
       rootNav->name().isEmpty() ? "<empty>" : csPrintable(rootNav->name()), 
       rootNav->section() );
  
   for (auto e : rootNav->children()) {
      printNavTree(e, indent + 2);
   }   
}

// generate the example documentation
void Doxy_Work::generateExampleDocs()
{
   Doxy_Globals::g_outputList->disable(OutputGenerator::Man);
  
   for (auto pd : *Doxy_Globals::exampleSDict) {
      msg("Generating docs for example %s\n", csPrintable(pd->name()) );
      resetCCodeParserState();

      QString n = pd->getOutputFileBase();

      startFile(*Doxy_Globals::g_outputList, n, n, pd->name());
      startTitle(*Doxy_Globals::g_outputList, n);

      Doxy_Globals::g_outputList->docify(pd->name());
      endTitle(*Doxy_Globals::g_outputList, n, 0);

      Doxy_Globals::g_outputList->startContents();
      Doxy_Globals::g_outputList->generateDoc(pd->docFile(), pd->docLine(), pd, QSharedPointer<MemberDef>(),
                  pd->documentation() + "\n\n\\include " + pd->name(), true, true, pd->name() );

      endFile(*Doxy_Globals::g_outputList); // contains g_outputList->endContents()
   }

   Doxy_Globals::g_outputList->enable(OutputGenerator::Man);
}

// generate module pages
void Doxy_Work::generateGroupDocs()
{ 
   for (auto gd : *Doxy_Globals::groupSDict) {
      if (! gd->isReference()) {
         msg("Generating docs for group %s\n", csPrintable(gd->name()) );
         gd->writeDocumentation(*Doxy_Globals::g_outputList);
      }
   }
}

//static void generatePackageDocs()
//{
//  writePackageIndex(*g_outputList);
//
//  if (Doxy_Globals::packageDict.count() > 0)  {
//    PackageSDict::Iterator pdi(Doxy_Globals::packageDict);
//    QSharedPointer<PackageDef pd;
//
//    for (pdi.toFirst();(pd=pdi.current());++pdi) //    
//       pd->writeDocumentation(*g_outputList);
//    }
//  }
//
//}

// generate module pages
void Doxy_Work::generateNamespaceDocs()
{
   // for each namespace
   for (auto nd : *Doxy_Globals::namespaceSDict) {

      if (nd->isLinkableInProject()) {
         msg("Generating docs for namespace %s\n", csPrintable(nd->name()));
         nd->writeDocumentation(*Doxy_Globals::g_outputList);
      }

      // for each class in the namespace   
      for (auto cd : *nd->getClassSDict()) {

         if ( ( cd->isLinkableInProject() && cd->templateMaster() == 0) && ! cd->isHidden() && !cd->isEmbeddedInOuterScope() ) {
              // skip external references, anonymous compounds and
              // template instances and nested classes             
         
            msg("Generating docs for namespace compounds %s\n", csPrintable(cd->name()));  

            cd->writeDocumentation(*Doxy_Globals::g_outputList);
            cd->writeMemberList(*Doxy_Globals::g_outputList);
         }

         cd->writeDocumentationForInnerClasses(*Doxy_Globals::g_outputList);
      }
   }
}

#if defined(_WIN32)
static QString fixSlashes(QString &s)
{
   QString result;
   uint i;

   for (i = 0; i < s.length(); i++) {

      switch (s.at(i).unicode()) {

         case '/':
         case '\\':
            result += "\\\\";
            break;

         default:
            result += s.at(i);
      }
   }
   return result;
}
#endif

void Doxy_Work::copyStyleSheet()
{
   static const QDir configDir    = Config::getConfigDir();
   static const QString outputDir = Config::getString("html-output") + "/";

    static const QStringList htmlStyleSheet = Config::getList("html-stylesheets");

   for (auto fileName : htmlStyleSheet) {             
      
      if (! fileName.isEmpty()) {
         QFileInfo fi(configDir, fileName);

         if (! fi.exists()) {
            err("Stylesheet '%s' specified in 'HTML STYLESHEETS' does not exist\n", csPrintable(fileName));

         } else if (fi.fileName() == "doxypress.css" || fi.fileName() == "tabs.css" || fi.fileName() == "navtree.css") {
            err("Style sheet %s specified by 'HTML STYLESHEETS' is using the built in stylesheet name. Please use a "
               "different file name\n", csPrintable(fileName));      
            
         } else {
            QString destFileName = outputDir + fi.fileName();
            copyFile(fi.absoluteFilePath(), destFileName);
         }
      }
   }
}

void Doxy_Work::copyLatexStyleSheet()
{  
   static const QDir configDir              = Config::getConfigDir();
   static const QString outputDir           = Config::getString("latex-output") + "/";
   static const QStringList latexStyleSheet = Config::getList("latex-stylesheets");
  
   for (auto fileName : latexStyleSheet) {

      if (! fileName.isEmpty()) {
         QFileInfo fi(configDir, fileName);

         if (! fi.exists()) {
            err("Stylesheet '%s' specified in 'LATEX STYLESHEETS' does not exist\n", csPrintable(fileName));

         } else {           
            QString destFileName = outputDir + fi.fileName();

            if (! checkExtension(fi.fileName(), Doxy_Globals::latexStyleExtension)) {
               destFileName += Doxy_Globals::latexStyleExtension;
            }
           
            copyFile(fi.absoluteFilePath(), destFileName);
         }
      }
   }
}

void Doxy_Work::copyLogo(const QString &outputType)
{
   static const QDir configDir = Config::getConfigDir();
   static const QString projectLogo = Config::getString("project-logo");

   if (! projectLogo.isEmpty()) {
      QFileInfo fi(configDir, projectLogo);

      if (! fi.exists()) {
         err("Project logo file '%s' does not exist\n", csPrintable(projectLogo));        

      } else {       
         QString destFileName = outputType + "/" + fi.fileName();
         copyFile(fi.absoluteFilePath(), destFileName);

         Doxy_Globals::indexList->addImageFile(fi.fileName());
      }
   }
}

void Doxy_Work::copyExtraFiles(const QString &outputType)
{  
   const QDir configDir = Config::getConfigDir();

   QString outputDir;
   QStringList extraFiles;

   if (outputType == "html") { 
      outputDir  = Config::getString("html-output") + "/";     
      extraFiles = Config::getList("html-extra-files");

   } else {
      outputDir  = Config::getString("latex-output") + "/";     
      extraFiles = Config::getList("latex-extra-files");   
   }
   
   for (auto fileName : extraFiles) {    

      if (! fileName.isEmpty()) {
         QFileInfo fi(configDir, fileName);

         if (! fi.exists()) {

            if (outputType == "html") {
               err("Extra file '%s' specified in 'HTML EXTRA FILES' does not exist\n", csPrintable(fileName));
            } else {
                err("Extra file '%s' specified in 'LATEX EXTRA FILES' does not exist\n", csPrintable(fileName));
            }

         } else {
            QString destFileName = outputDir + fi.fileName();          
            copyFile(fi.absoluteFilePath(), destFileName);

            Doxy_Globals::indexList->addImageFile(fi.fileName());
         }
      }
   }
}

static ParserInterface *getParserForFile(const QString &fn)
{
   QString fileName = fn;
   QString extension;

   int sep = fileName.lastIndexOf('/');
   int ei  = fileName.lastIndexOf('.');

   if (ei != -1 && (sep == -1 || ei > sep)) { 
      // matches dir/file.ext but not dir.1/file
      extension = fileName.right(fileName.length() - ei);

   } else {
      extension = ".no_extension";
   }

   return Doxy_Globals::parserManager->getParser(extension);
}

void Doxy_Work::parseFile(ParserInterface *parser, QSharedPointer<Entry> root, QSharedPointer<EntryNav> rootNav, 
                  QSharedPointer<FileDef> fd, QString fileName, enum ParserMode mode, QStringList &includedFiles)
{   
   static bool clangParsing = Config::getBool("clang-parsing");
          
   QString extension;
   int ei = fileName.lastIndexOf('.');

   if (ei != -1) {
      extension = fileName.right(fileName.length() - ei);
   } else {
      extension = ".no_extension";
   }
 
   QFileInfo fi(fileName);
   QString fileContents;

   // broom - not completed if clangParsing, do not preprocess

   if (Config::getBool("enable-preprocessing") && parser->needsPreprocessing(extension)) {      
      msg("Processing %s\n", csPrintable(fileName));

      fileContents = readInputFile(fileName); 
      fileContents = preprocessFile(fileName, fileContents);

   } else { 
      // no preprocessing
      msg("Reading %s\n", csPrintable(fileName));
      fileContents = readInputFile(fileName);
 
   }

   if (! fileContents.endsWith("\n")) {
      // add extra newline to help parser
      fileContents += '\n'; 
   }
   
   // convert multi-line C++ comments to C style comments
   QString convBuf = convertCppComments(fileContents, fileName);
   auto srcLang    = fd->getLanguage(); 

   if (clangParsing && (srcLang == SrcLangExt_Cpp || srcLang == SrcLangExt_ObjC)) {   
      fd->getAllIncludeFilesRecursively(includedFiles);
      
      // use clang for parsing  
      parser->parseInput(fileName, convBuf, root, mode, includedFiles, true);

   } else { 
      // use lex parser
      parser->parseInput(fileName, convBuf, root, mode, includedFiles, false);

   }

   // store the Entry tree in a file and create an index to navigate/load entries
   root->createNavigationIndex(rootNav, Doxy_Globals::g_storage, fd, root);
}

// parse the list of input files
void Doxy_Work::parseFiles(QSharedPointer<Entry> root, QSharedPointer<EntryNav> rootNav)
{
   // user specified 
   static bool clangParsing = Config::getBool("clang-parsing"); 
 
   if (clangParsing) {   
      QSet<QString> processedFiles;      
      QSet<QString> filesToProcess;
      
      for (auto fName : Doxy_Globals::g_inputFiles) { 
         filesToProcess.insert(fName);
      }

      // process source files and their include dependencies     
      for (auto fName : Doxy_Globals::g_inputFiles) { 
         bool ambig;

         QSharedPointer<FileDef> fd = findFileDef(Doxy_Globals::inputNameDict, fName, ambig);
         assert(fd != 0);

         if (fd->isSource() && ! fd->isReference()) {             
            QStringList includedFiles;

            auto srcLang = fd->getLanguage();

            ParserInterface *parser = getParserForFile(fName);           
            parseFile(parser, root, rootNav, fd, fName, ParserMode::SOURCE_FILE, includedFiles);
           
            // process any include files in the the current source file
            for (auto file : includedFiles) {

               if (! filesToProcess.contains(file)) {
                  break;
               }

               if (file != fName && ! processedFiles.contains(file)) {
                  QSharedPointer<FileDef> ifd = findFileDef(Doxy_Globals::inputNameDict, csPrintable(file), ambig);

                  if (ifd && ! ifd->isReference()) {
                     QStringList moreFiles;
                     
                     parseFile(parser, root, rootNav, ifd, file, ParserMode::INCLUDE_FILE, moreFiles);
                     processedFiles.insert(file);
                  }
               }               
            }           

            if (srcLang == SrcLangExt_Cpp || srcLang == SrcLangExt_ObjC) {   
               ClangParser::instance()->finish();
            }

            processedFiles.insert(fName);
         }
      }

      // process remaining files, treat as source files even if they are header files
      for (auto fName : Doxy_Globals::g_inputFiles) { 

         if (! processedFiles.contains(fName)) { 
            QStringList includedFiles;
            
            bool ambig;
            
            QSharedPointer<FileDef> fd = findFileDef(Doxy_Globals::inputNameDict, fName, ambig);
            assert(fd != 0);

            auto srcLang = fd->getLanguage();

            ParserInterface *parser = getParserForFile(fName);         
            parseFile(parser, root, rootNav, fd, fName, ParserMode::SOURCE_FILE, includedFiles);

            if (srcLang == SrcLangExt_Cpp || srcLang == SrcLangExt_ObjC) {   
               ClangParser::instance()->finish();
            }

            processedFiles.insert(fName);
         }
      }

   } else  {
      // use lex and not clang
        
      for (auto fName : Doxy_Globals::g_inputFiles) { 
         QStringList includedFiles;

         bool ambig;         

         QSharedPointer<FileDef> fd = findFileDef(Doxy_Globals::inputNameDict, fName, ambig);
         assert(fd != nullptr);

         ParserInterface *parser = getParserForFile(fName);        
         parseFile(parser, root, rootNav, fd, fName, ParserMode::SOURCE_FILE, includedFiles);
      }
   }
}

// resolves a path that may include symlinks, if a recursive symlink is
// found an empty string is returned.
QString  Doxy_Work::resolveSymlink(QString  path)
{
   int sepPos = 0;
   int oldPos = 0;

   QFileInfo fi;

   QSet<QString> nonSymlinks;
   QSet<QString> known;

   QString result = path;
   QString oldPrefix = "/";

   do {

#ifdef Q_OS_WIN
      // UNC path, skip server and share name

      if (sepPos == 0 && (result.left(2) == "//" || result.left(2) == "\\\\")) {
         sepPos = result.indexOf('/', 2);
      }

      if (sepPos != -1) {
         sepPos = result.indexOf('/', sepPos + 1);
      }
#else
      sepPos = result.indexOf('/', sepPos + 1);

#endif

      QString prefix = sepPos == -1 ? result : result.left(sepPos);

      if (! nonSymlinks.contains(prefix)) {
         fi.setFile(prefix);

         if (fi.isSymLink()) {
            QString target = fi.readLink();
            bool isRelative = QFileInfo(target).isRelative();

            if (isRelative) {
               target = QDir::cleanPath(oldPrefix + "/" + target);
            }

            if (sepPos != -1) {
               if (fi.isDir() && target.length() > 0 && target.at(target.length() - 1) != '/') {
                  target += '/';
               }
               target += result.mid(sepPos);
            }

            result = QDir::cleanPath(target);
            sepPos = 0;

            if (known.contains(result)) {
               // recursive symlink
               return QString();  
            }

            known.insert(result);

            if (isRelative) {
               sepPos = oldPos;

            } else { 
               // link to absolute path
               sepPos    = 0;
               oldPrefix = "/";
            }

         } else {
            nonSymlinks.insert(prefix);
            oldPrefix = prefix;
         }

         oldPos = sepPos;
      }

   } while (sepPos != -1);

   return QDir::cleanPath(result);
}

void Doxy_Work::readTagFile(QSharedPointer<Entry> root, const QString &tagLine)
{
   QString fileName;
   QString destName;

   int eqPos = tagLine.indexOf('=');

   if (eqPos != -1) { 
      // tag command contains a destination
      fileName = tagLine.left(eqPos).trimmed();
      destName = tagLine.right(tagLine.length() - eqPos - 1).trimmed();

      QFileInfo fi(fileName);
      Doxy_Globals::tagDestinationDict.insert(fi.absoluteFilePath(), destName);

   } else {
      fileName = tagLine;

   }

   QFileInfo fi(fileName);
   if (! fi.exists() || ! fi.isFile()) {
      err("Tag file `%s' does not exist or is not a file\n", csPrintable(fileName));
      return;
   }

   if (! destName.isEmpty()) {
      msg("Reading tag file `%s', location `%s'\n", csPrintable(fileName), csPrintable(destName));

   } else {
      msg("Reading tag file `%s'\n", csPrintable(fileName));
   }

   parseTagFile(root, fi.absoluteFilePath());
}

QString Doxy_Work::createOutputDirectory(const QString &baseDirName, const QString &formatDirOption, const QString &defaultDirName)
{   
   QString formatDirName = Config::getString(formatDirOption);

   if (formatDirName.isEmpty()) {
      formatDirName = baseDirName + defaultDirName;

   } else if (formatDirName.at(0) != '/' && (formatDirName.length() == 1 || formatDirName[1] != ':')) {
      formatDirName.prepend(baseDirName + '/');
   }

   QDir formatDir(formatDirName);

   if (! formatDir.exists() && ! formatDir.mkdir(formatDirName)) {  
      err("Unable to create output directory %s", csPrintable(formatDirName));  
      stopDoxyPress();
   }

   return formatDirName;
}

// read all files matching at least one pattern in `patList' in the directory represented by `fi'
// directory is read if the recusiveFlag is set, contents of all files is append to the input string

void Doxy_Work::readDir(const QFileInfo &fi, ReadDirArgs &data)
{
   QString dirName = fi.absoluteFilePath();

   if (data.isPathSet && ! data.pathSet.contains(dirName)) {
      data.pathSet.insert(dirName);
   }

   if (fi.isSymLink()) {
      dirName = resolveSymlink(dirName);

      if (dirName.isEmpty()) {
         return;     // recusive symlink
      }

      if (Doxy_Globals::g_pathsVisited.contains(dirName)) {
         return;     // already visited path
      }

      Doxy_Globals::g_pathsVisited.insert(dirName);
   }

   QDir dir(dirName);
   dir.setFilter(QDir::Files | QDir::Dirs | QDir::Hidden);

   int totalSize = 0;
   msg("Searching for files in directory %s\n", csPrintable(fi.absoluteFilePath()) );

   const QFileInfoList list = dir.entryInfoList();

   for (auto &cfi : list) {
      QString filePath = cfi.absoluteFilePath();

      if (! data.excludeSet.contains(filePath)) {
         // file should not be excluded

         if (! cfi.exists() || ! cfi.isReadable()) {

            if (data.errorIfNotExist) {
               warn_uncond("Source %s is not a readable file or directory\n", csPrintable(filePath));
            }

         } else {
            bool excludeSymlink = Config::getBool("exclude-symlinks");

            if (excludeSymlink && cfi.isSymLink())  {
               continue;
            }  

            if (cfi.isFile()) {

               bool testA = (data.includePatternList.isEmpty() || patternMatch(cfi, data.includePatternList));                
               bool testB = (! patternMatch(cfi, data.excludePatternList));
               
               if (testA && testB && ! data.killDict.contains(filePath) ) {
                           
                  totalSize += cfi.size() + filePath.length() + 4;
                  QString name = cfi.fileName();
      
                  if (data.isFnDict) {
                     QSharedPointer<FileDef> fd = QMakeShared<FileDef>(cfi.path() + "/", name);
                     QSharedPointer<FileNameList> fn;
      
                     if (! name.isEmpty() && (fn = data.fnDict[name])) {
                        fn->append(fd);
      
                     } else {
                        fn = QMakeShared<FileNameList>(filePath, name);
                        fn->append(fd);
      
                        if (data.isFnList) {
                           data.fnList.inSort(fn);
                        }
      
                        data.fnDict.insert(name, fn);
                     }
                  }                  
      
                  if (data.isResultList) {                
                     data.resultList.append(filePath);                                
                  }
      
                  if (data.isPrepExclude) {      
                     data.prepExcludeSet.insert(filePath);
                  }
      
                  if (data.isKillDict) {         
                     data.killDict.insert(filePath);                  
                  }     
               }            

            } else if (cfi.isDir() && data.recursive) {

               if (cfi.fileName().at(0) == '.') {
                  continue;
               }

               if (patternMatch(cfi, data.excludePatternList) ) {
                  continue;
               }   

               QFile tmp(filePath);
               readDir(tmp, data);               
            }
         }
      }
   }   
}

// read a file or all files in a directory and append their contents to the
// input string. The names of the files are appended to the `fiList' list.

void Doxy_Work::readFileOrDirectory(const QString &fn, ReadDirArgs &data)
{   
   if (fn.isEmpty()) {
      return;
   } 

   // strip trailing slashes
   QString fileName = fn;
  
   if (fileName.endsWith('/') || fileName.endsWith('\\')) {
      fileName = fileName.left(fileName.length() - 1);
   }

   QFileInfo fi(fileName);                  
 
   QString name     = fi.fileName();
   QString dirPath  = fi.absolutePath();
   QString filePath = fi.absoluteFilePath(); 
   
   if (! data.excludeSet.contains(filePath)) {

      if (! fi.exists() || ! fi.isReadable()) {
         if (data.errorIfNotExist) {
            warn_uncond("Source file %s is not a readable file or directory\n", csPrintable(fileName));
         }

      } else if (! Config::getBool("exclude-symlinks") || ! fi.isSymLink()) {

         if (fi.isFile()) {          
            
            if (! data.killDict.contains(filePath)) {
                            
               if (data.isFnDict) {
                  QSharedPointer<FileDef> fd = QMakeShared<FileDef>(dirPath + "/", name);
                  QSharedPointer<FileNameList> fn;

                  if (! name.isEmpty() && (fn = data.fnDict[name])) {
                     fn->append(fd);

                  } else {
                     fn = QMakeShared<FileNameList>(filePath, name);
                     fn->append(fd);

                     if (data.isFnList) {
                        data.fnList.inSort(fn);
                     }

                     // can be Doxy_Globals::inputNameDict->insert(), Doxy_Globals::exampleNameDict, etc
                     data.fnDict.insert(name, fn);
                  }
               }
                        
               if (data.isResultList) {                
                  data.resultList.append(filePath);                                
               }

               if (data.isPrepExclude) {      
                  data.prepExcludeSet.insert(filePath);
               }

               if (data.isKillDict) {         
                  data.killDict.insert(filePath);                  
               }                        
            }

         } else if (fi.isDir()) { 
            // readable dir
            readDir(fi, data);
         }
      }
   }
  
   return;
}

void readFormulaRepository()
{
   QFile f(Config::getString("html-output") + "/formula.repository");

   if (f.open(QIODevice::ReadOnly)) { 
      // open repository

      msg("Reading formula repository\n");
      QTextStream t(&f);
      QString line;

      while (! t.atEnd()) {
         line = t.readLine();
         int se = line.indexOf(':'); // find name and text separator

         if (se == -1) {
            warn_uncond("formula.repository is corrupted\n");
            break;

         } else {
            QString formName = line.left(se);
            QString formText = line.right(line.length() - se - 1);

            Formula f = Formula(formText);

            Doxy_Globals::formulaList->append(f);
            Doxy_Globals::formulaDict->insert(formText, f);
            Doxy_Globals::formulaNameDict->insert(formName, f);
         }
      }
   }
}

void Doxy_Work::dumpPhrase(QTextStream &t, QSharedPointer<Definition> def)
{
   QString anchor;

   if (def->definitionType() == Definition::TypeMember) {
      QSharedPointer<MemberDef> md = def.dynamicCast<MemberDef>();
      anchor = ":" + md->anchor();
   }

   QString scope;

   if (def->getOuterScope() && def->getOuterScope() != Doxy_Globals::globalScope) {
      scope = def->getOuterScope()->getOutputFileBase() + Doxy_Globals::htmlFileExtension;
   }

   t << "REPLACE INTO symbols (symbol_id,scope_id,name,file,line) VALUES('"
     << def->getOutputFileBase() + Doxy_Globals::htmlFileExtension + anchor << "','"
     << scope << "','"
     << def->name() << "','"
     << def->getDefFileName() << "','"
     << def->getDefLine()
     << "');" << endl;
}

void Doxy_Work::dumpGlossary()
{
   QFile f("symbols.sql");

   if (f.open(QIODevice::WriteOnly)) {
      QTextStream t(&f);

      for (auto item : Doxy_Globals::glossary()) {
         // list of phrases

         QSharedPointer<Definition> def = sharedFrom(item);
         dumpPhrase(t, def);         
      }
   }
}

// read the argument of option `c' from the comment argument list and
// update the option index `optind'.
int Doxy_Work::computeIdealCacheParam(uint v)
{   
   int r = 0;

   while (v != 0) {
      v >>= 1, r++;
   }
  
   // convert to a valid cache size value
   return qMax(0, qMin(r - 16, 9));
}

void Doxy_Work::stopDoxyPress(int unused)
{
   msg("Cleaning up\n");

   QDir thisDir;
   if (! Doxy_Globals::tempA_FName.isEmpty()) {
      Doxy_Globals::symbolStorage->close();
      thisDir.remove(Doxy_Globals::tempA_FName);
   }

   if (! Doxy_Globals::tempB_FName.isEmpty()) {
      Doxy_Globals::g_storage->close();
      thisDir.remove(Doxy_Globals::tempB_FName);
   }

#ifdef HAS_SIGNALS
   killpg(0, SIGINT);
#endif

   msg("DoxyPress aborted\n");
   exit(1);
}

void Doxy_Work::writeTagFile()
{
   const QString generateTagFile = Config::getString("generate-tagfile");

   if (generateTagFile.isEmpty()) {
      return;
   }

   QFile tag(generateTagFile);

   if (! tag.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", csPrintable(generateTagFile), tag.error());
      return;
   }

   QTextStream tagFile(&tag);
   tagFile << "<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>" << endl;
   tagFile << "<tagfile>" << endl;

   // for each file
   for (auto fn : *Doxy_Globals::inputNameList) {
      for (auto fd : *fn) {       
         if (fd->isLinkableInProject()) {
            fd->writeTagFile(tagFile);
         }  
      }
   }

   // for each class
   for (auto cd : *Doxy_Globals::classSDict) { 
      if (cd->isLinkableInProject()) {
         cd->writeTagFile(tagFile);
      }
   }

   // for each namespace
   for (auto nd : *Doxy_Globals::namespaceSDict) {
      if (nd->isLinkableInProject()) {
         nd->writeTagFile(tagFile);
      }
   }

   // for each group
   for (auto gd : *Doxy_Globals::groupSDict) {
      if (gd->isLinkableInProject()) {
         gd->writeTagFile(tagFile);
      }
   }

   // for each page
   for (auto pd : *Doxy_Globals::pageSDict) {
      if (pd->isLinkableInProject()) {
         pd->writeTagFile(tagFile);
      }
   }

   if (Doxy_Globals::mainPage) {
      Doxy_Globals::mainPage->writeTagFile(tagFile);
   }

   /*
   if (Doxy_Globals::mainPage && ! Config::getString("generate-tagfile").isEmpty())
   {
     tagFile << "  <compound kind=\"page\">" << endl
                      << "    <name>"
                      << convertToXML(Doxy_Globals::mainPage->name())
                      << "</name>" << endl
                      << "    <title>"
                      << convertToXML(Doxy_Globals::mainPage->title())
                      << "</title>" << endl
                      << "    <filename>"
                      << convertToXML(Doxy_Globals::mainPage->getOutputFileBase())
                      << "</filename>" << endl;

     mainPage->writeDocAnchorsToTagFile();
     tagFile << "  </compound>" << endl;
   }
   */

   tagFile << "</tagfile>" << endl;
}

QString Doxy_Work::getQchFileName()
{
   const QString qchFile = Config::getString("qch-file");

   if (! qchFile.isEmpty()) {
      return qchFile;
   }

   QString projectName    = Config::getString("project-name");
   QString projectVersion = Config::getString("project-version");

   if (projectName.isEmpty()) {
      projectName = "index";
   }

   if (! projectVersion.isEmpty()) {
      projectVersion = "-" + projectVersion; 
   }

   QString retval = "../qch/" + projectName + projectVersion + ".qch";

   return retval;
}

void searchInputFiles()
{     
   // gather names for all files in the include and example path
   QSet<QString> excludeSet;

   // source   
   Doxy_Globals::g_stats.begin("Searching for include files\n");

   const QStringList inputPatterns   = Config::getList("input-patterns");
   const bool inputRecursive         = Config::getBool("input-recursive");

   const QStringList excludePatterns = Config::getList("exclude-patterns");

   const QStringList includePath     = Config::getList("include-path");
   QStringList includePatterns       = Config::getList("include-patterns");  
       
   if (includePatterns.isEmpty()) {
      includePatterns = inputPatterns;
   }

   for (auto fName : includePath) {    
      ReadDirArgs data;

      data.recursive          = inputRecursive;         
      data.isFnDict           = true;
      data.fnDict             = *Doxy_Globals::includeNameDict;
      data.includePatternList = includePatterns;
      data.excludePatternList = excludePatterns;

      readFileOrDirectory(fName, data);

      *Doxy_Globals::includeNameDict = data.fnDict;
   }
   Doxy_Globals::g_stats.end();

   // examples
   Doxy_Globals::g_stats.begin("Searching for example files\n");

   const QStringList examplePath     = Config::getList("example-source");
   const QStringList examplePatterns = Config::getList("example-patterns");
   const bool exampleRecursive       = Config::getBool("example-recursive");

   for (auto s : examplePath) {     
      ReadDirArgs data;

      data.recursive          = exampleRecursive;
      data.isFnDict           = true;
      data.fnDict             = *Doxy_Globals::exampleNameDict;
      data.includePatternList = examplePatterns;
 
      readFileOrDirectory(s, data);

      *Doxy_Globals::exampleNameDict = data.fnDict;
   }
   Doxy_Globals::g_stats.end();


   // images
   Doxy_Globals::g_stats.begin("Searching for images\n");
   const QStringList imagePath = Config::getList("image-path");

   for (auto s : imagePath) {
      ReadDirArgs data;

      data.recursive = inputRecursive;
      data.isFnDict  = true;
      data.fnDict    = *Doxy_Globals::imageNameDict;

      readFileOrDirectory(s, data);

      *Doxy_Globals::imageNameDict = data.fnDict;
   }
   Doxy_Globals::g_stats.end();


   // dot files
   Doxy_Globals::g_stats.begin("Searching for dot files\n");
   const QStringList dotFiles = Config::getList("dot-file-dirs");

   for (auto s : dotFiles) {
      ReadDirArgs data;

      data.recursive  = inputRecursive;
      data.isFnDict   = true;
      data.fnDict     = *Doxy_Globals::dotFileNameDict;

      readFileOrDirectory(s, data);

      *Doxy_Globals::dotFileNameDict = data.fnDict;
   }
   Doxy_Globals::g_stats.end();


   // msc
   Doxy_Globals::g_stats.begin("Searching for msc files\n");
   const QStringList mscFiles = Config::getList("msc-file-dirs");

   for (auto s : mscFiles) {
      ReadDirArgs data;

      data.recursive  = inputRecursive;
      data.isFnDict   = true;
      data.fnDict     = *Doxy_Globals::mscFileNameDict;

      readFileOrDirectory(s, data);

      *Doxy_Globals::mscFileNameDict = data.fnDict;
   }
   Doxy_Globals::g_stats.end();


   // dia
   Doxy_Globals::g_stats.begin("Searching for dia files\n");
   const QStringList diaFiles = Config::getList("dia-file-dirs");

   for (auto s : diaFiles) {
      ReadDirArgs data;

      data.recursive   = inputRecursive;
      data.isFnDict    = true;
      data.fnDict      = *Doxy_Globals::diaFileNameDict;
    
      readFileOrDirectory(s, data);
      *Doxy_Globals::diaFileNameDict = data.fnDict;
   }
   Doxy_Globals::g_stats.end();


   Doxy_Globals::g_stats.begin("Searching for files to exclude\n");
   const QStringList excludeFiles  = Config::getList("exclude-files");   

   for (auto s : excludeFiles) {   
      ReadDirArgs data;

      data.recursive          = inputRecursive;
      data.errorIfNotExist    = false;
      data.includePatternList = inputPatterns;
      data.isPrepExclude      = true;
      data.prepExcludeSet     = excludeSet;
            
      readFileOrDirectory(s, data);  

      excludeSet = data.prepExcludeSet;  
   }
   Doxy_Globals::g_stats.end();


   // find input files
   Doxy_Globals::g_stats.begin("Searching for files to process\n");

   QSet<QString> killDict;
   QStringList inputSource = Config::getList("input-source");

   for (auto s : inputSource) {
      QString path = s;
      uint len = path.length();

      if (len > 0) {
         // strip trailing slashes
         if (path.at(len - 1) == '\\' || path.at(len - 1) == '/') {
            path = path.left(len - 1);
         }

         ReadDirArgs data;       

         data.recursive          = inputRecursive;         
         data.isFnList           = true;
         data.fnList             = *Doxy_Globals::inputNameList;
         data.isFnDict           = true;
         data.fnDict             = *Doxy_Globals::inputNameDict;   
         data.includePatternList = inputPatterns;
         data.excludePatternList = excludePatterns;
         data.excludeSet         = excludeSet;
         data.isResultList       = true;
         data.resultList         = Doxy_Globals::g_inputFiles;       
         data.isKillDict         = true;
         data.killDict           = killDict;
         data.isPathSet          = true;
         data.pathSet            = Doxy_Globals::inputPaths;

         readFileOrDirectory(path, data);              

         *Doxy_Globals::inputNameList  = data.fnList;
         *Doxy_Globals::inputNameDict  = data.fnDict; 
         Doxy_Globals::g_inputFiles    = data.resultList;          
         killDict                      = data.killDict;
         Doxy_Globals::inputPaths      = data.pathSet;   
      }
   }

   Doxy_Globals::g_stats.end();
}

