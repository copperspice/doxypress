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

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <set>

#include <arguments.h>
#include <cmdmapper.h>
#include <code_cstyle.h>
#include <commentcnv.h>
#include <config.h>
#include <declinfo.h>
#include <default_args.h>
#include <docbookgen.h>
#include <docparser.h>
#include <docsets.h>
#include <dot.h>
#include <doxy_setup.h>
#include <doxy_globals.h>
#include <eclipsehelp.h>
#include <ftvhelp.h>
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
#include <parse_clang.h>
#include <parse_cstyle.h>
#include <parse_py.h>
#include <perlmodgen.h>
#include <portable.h>
#include <pre.h>
#include <qhp.h>
#include <rtfgen.h>
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
   { "auto_ptr",             0,                              0,                     "T",           "ptr",         0,     0,             false,   false },
   { "smart_ptr",            0,                              0,                     "T",           "ptr",         0,     0,             false,   false },
   { "unique_ptr",           0,                              0,                     "T",           "ptr",         0,     0,             false,   false },
   { "shared_ptr",           0,                              0,                     "T",           "ptr",         0,     0,             false,   false },
   { "weak_ptr",             0,                              0,                     "T",           "ptr",         0,     0,             false,   false },
   { "atomic",               0,                              0,                     "T",           "ptr",         0,     0,             FALSE,   FALSE },
   { "atomic_ref",           0,                              0,                     "T",           "ptr",         0,     0,             FALSE,   FALSE },
   { "lock_guard",           0,                              0,                     "T",           "ptr",         0,     0,             FALSE,   FALSE },
   { "unique_lock",          0,                              0,                     "T",           "ptr",         0,     0,             FALSE,   FALSE },
   { "shared_lock",          0,                              0,                     "T",           "ptr",         0,     0,             FALSE,   FALSE },
   { "ios_base",             0,                              0,                     0,             0,             0,     0,             false,   false },
   { "error_code",           0,                              0,                     0,             0,             0,     0,             false,   false },
   { "error_category",       0,                              0,                     0,             0,             0,     0,             false,   false },
   { "system_error",         0,                              0,                     0,             0,             0,     0,             false,   false },
   { "error_condition",      0,                              0,                     0,             0,             0,     0,             false,   false },
   { "thread",               0,                              0,                     0,             0,             0,     0,             false,   false },
   { "jthread",              0,                              0,                     0,             0,             0,            0,      FALSE,   FALSE },
   { "mutex",                0,                              0,                     0,             0,             0,            0,      FALSE,   FALSE },
   { "timed_mutex",          0,                              0,                     0,             0,             0,            0,      FALSE,   FALSE },
   { "recursive_mutex",      0,                              0,                     0,             0,             0,            0,      FALSE,   FALSE },
   { "recursive_timed_mutex",0,                              0,                     0,             0,             0,            0,      FALSE,   FALSE },
   { "shared_mutex",         0,                              0,                     0,             0,             0,            0,      FALSE,   FALSE },
   { "shared_timed_mutex",   0,                              0,                     0,             0,             0,            0,      FALSE,   FALSE },
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
   { "u8string",             "basic_string<char8_t>",        0,                     0,             0,             0,            0,      FALSE,   TRUE  },
   { "u16string",            "basic_string<char16_t>",       0,                     0,             0,             0,            0,      FALSE,   TRUE  },
   { "u32string",            "basic_string<char32_t>",       0,                     0,             0,             0,            0,      FALSE,   TRUE  },
   { "basic_string_view",    0,                              0,                     "Char",        0,             0,            0,      FALSE,   TRUE  },
   { "string_view",          "basic_string_view<char>",      0,                     0,             0,             0,            0,      FALSE,   TRUE  },
   { "wstring_view",         "basic_string_view<wchar_t>",   0,                     0,             0,             0,            0,      FALSE,   TRUE  },
   { "u8string_view",        "basic_string_view<char8_t>",   0,                     0,             0,             0,            0,      FALSE,   TRUE  },
   { "u16string_view",       "basic_string_view<char16_t>",  0,                     0,             0,             0,            0,      FALSE,   TRUE  },
   { "u32string_view",       "basic_string_view<char32_t>",  0,                     0,             0,             0,            0,      FALSE,   TRUE  },
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
   { "array",                0,                              0,                     "T",           "elements",    0,     0,             false,   false },
   { "vector",               0,                              0,                     "T",           "elements",    0,     0,             false,   true  },
   { "span",                 0,                              0,                     "T",           "elements",    0,     0,             FALSE,   TRUE  },
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

   void addClassToContext(QSharedPointer<Entry> ptrEntry);
   void addEnumValuesToEnums(QSharedPointer<Entry> ptrEntry);

   static void addIncludeFileClass(QSharedPointer<ClassDef> cd, QSharedPointer<FileDef> include_fd, QSharedPointer<Entry> root);
   void addInterfaceOrServiceToServiceOrSingleton(QSharedPointer<Entry> ptrEntry, QSharedPointer<ClassDef> cd,
                  QString const &rname);

   void addMethodToClass(QSharedPointer<Entry> ptrEntry, QSharedPointer<ClassDef> cd, const QString &rname, bool isFriend);

   void addMembersToIndex();
   void addMembersToMemberGroup();

   void addMemberDocs(QSharedPointer<Entry> ptrEntry, QSharedPointer<MemberDef> md, const QString &funcDecl,
                  ArgumentList &argList, bool overload, NamespaceSDict *nl = nullptr);

   void addPageToContext(QSharedPointer<PageDef> pd, QSharedPointer<Entry> ptrEntry);
   void addListReferences();
   void addRelatedPage_X(QSharedPointer<Entry> ptrEntry);
   void addSourceReferences();
   void addSTLClasses(QSharedPointer<Entry> ptrEntry);
   void addSTLIterator(QSharedPointer<Entry> classEntry, const QString &name);
   void addSTLMember(QSharedPointer<Entry> ptrEntry, const QString &type, const QString &name);

   void addVariable(QSharedPointer<Entry> ptrEntry, int isFuncPtr = -1);

   QSharedPointer<MemberDef> addVariableToClass(QSharedPointer<Entry> ptrEntry, QSharedPointer<ClassDef> cd,
                  MemberDefType memberType, const QString &name, bool fromAnnScope, QSharedPointer<MemberDef> fromAnnMemb,
                  Protection prot, Relationship related);

   QSharedPointer<MemberDef> addVariableToFile(QSharedPointer<Entry> ptrEntry, MemberDefType memberType, const QString &scope,
                  const QString &name, bool fromAnnScope, QSharedPointer<MemberDef> fromAnnMemb);

   void buildClassDocList(QSharedPointer<Entry> ptrEntry);
   void buildClassList(QSharedPointer<Entry> ptrEntry);
   void buildCompleteMemberLists();
   void buildFileList(QSharedPointer<Entry> ptrEntry);
   void buildExampleList(QSharedPointer<Entry> ptrEntry);
   void buildFunctionList(QSharedPointer<Entry> ptrEntry);
   void buildGroupList(QSharedPointer<Entry> ptrEntry);
   void buildGroupListFiltered(QSharedPointer<Entry> ptrEntry, bool additional, bool includeExternal);
   void buildInterfaceAndServiceList(QSharedPointer<Entry> ptrEntry);
   void buildListOfUsingDecls(QSharedPointer<Entry> ptrEntry);
   void buildNamespaceList(QSharedPointer<Entry> ptrEntry);
   void buildPageList(QSharedPointer<Entry> ptrEntry);

   QSharedPointer<Definition> buildScopeFromQualifiedName(const QString name, int level, SrcLangExt lang, const TagInfo &tagInfo);

   void buildTypedefList(QSharedPointer<Entry> ptrEntry);
   void buildVarList(QSharedPointer<Entry> ptrEntry);

   void checkPageRelations();

   void combineUsingRelations();
   void computeClassRelations();
   int computeIdealCacheParam(uint v);
   void computeMemberReferences();
   void computeMemberRelations();
   void computePageRelations(QSharedPointer<Entry> ptrEntry);
   void computeTemplateClassRelations();

   enum CompoundType convertToCompoundType(int section, Entry::Traits data);

   void copyExtraFiles(const QString &outputType);
   void copyLogo(const QString &outputType);
   void copyStyleSheet();
   void copyLatexStyleSheet();

   QString createOutputDirectory(const QString &baseDirName, const QString &formatDirOption, const QString &defaultDirName);
   void createTemplateInstanceMembersX();
   QSharedPointer<ClassDef> createTagLessInstance(QSharedPointer<ClassDef> rootCd, QSharedPointer<ClassDef> templ,
                   const QString &fieldName);

   void distributeMemberGroupDocumentation();
   void dumpPhrase(QTextStream &t, QSharedPointer<Definition> d);
   void dumpGlossary();

   QString extractClassName(QSharedPointer<Entry> ptrEntry);

   void filterMemberDocumentation(QSharedPointer<Entry> ptrEntry);

   void findBaseClassesForClass(QSharedPointer<Entry> ptrEntry, QSharedPointer<Definition> context, QSharedPointer<ClassDef> masterCd,
                  QSharedPointer<ClassDef> instanceCd, FindBaseClassRelation_Mode mode, bool isArtificial,
                  const ArgumentList &actualArgs = ArgumentList(), QHash<QString, int> templateNames = QHash<QString, int>());

   void findClassEntries(QSharedPointer<Entry> ptrEntry);
   bool findClassRelation(QSharedPointer<Entry> ptrEntry, QSharedPointer<Definition> context, QSharedPointer<ClassDef> cd, BaseInfo *bi,
                  QHash<QString, int> templateNames, FindBaseClassRelation_Mode mode, bool isArtificial);

   QSharedPointer<ClassDef> findClassWithinClassContext(QSharedPointer<Definition> context, QSharedPointer<ClassDef> cd,
                  const QString &name);

   void findDefineDocumentation(QSharedPointer<Entry> ptrEntry);
   void findDEV(const MemberNameSDict &mnsd);
   void findDirDocumentation(QSharedPointer<Entry> ptrEntry);
   void findDocumentedEnumValues();

   int findEndOfTemplate(const QString &s, int startPos);
   void findEnums(QSharedPointer<Entry> ptrEntry);
   void findEnumDocumentation(QSharedPointer<Entry> ptrEntry);

   void findFriends();
   int findFunctionPtr(const QString &type, int lang, int *pLength = 0);

   bool findGlobalMember(QSharedPointer<Entry> ptrEntry, const QString &namespaceName, const QString &type, const QString &name,
                  const QString &tempArg, const QString &decl);

   void findGroupScope(QSharedPointer<Entry> ptrEntry);
   void findInheritedTemplateInstances();

   void findMainPage(QSharedPointer<Entry> ptrEntry);
   void findMainPageTagFiles(QSharedPointer<Entry> ptrEntry);
   void findMember(QSharedPointer<Entry> ptrEntry, QString funcDecl, bool overloaded, bool isFunc);
   void findMemberDocumentation(QSharedPointer<Entry> ptrEntry);

   void findObjCMethodDefinitions(QSharedPointer<Entry> ptrEntry);

   QSharedPointer<Definition> findScopeFromQualifiedName(QSharedPointer<Definition> startScope, const QString &n,
                  QSharedPointer<FileDef> fileScope, const TagInfo &tagInfo);

   void findSectionsInDocumentation();

   void findTagLessClasses();
   void findTagLessClasses(QSharedPointer<ClassDef> cd);

   bool findTemplateInstanceRelation(QSharedPointer<Entry> ptrEntry, QSharedPointer<Definition> context,
                  QSharedPointer<ClassDef> templateClass, const QString &templSpec, QHash<QString, int> templateNames,
                  bool isArtificial);

   QSharedPointer<NamespaceDef> findUsedNamespace(const NamespaceSDict *unl, const QString &name);

   void findUsedClassesForClass(QSharedPointer<Entry> ptrEntry, QSharedPointer<Definition> context, QSharedPointer<ClassDef> masterCd,
                  QSharedPointer<ClassDef> instanceCd, bool isArtificial, const ArgumentList &actualArgs = ArgumentList(),
                  QHash<QString, int> templateNames = QHash<QString, int>());

   void findUsedTemplateInstances();

   void findUsingDeclarations(QSharedPointer<Entry> ptrEntry);
   void findIncludedUsingDirectives();
   void findUsingDirectives(QSharedPointer<Entry> ptrEntry);
   void findUsingDeclImports(QSharedPointer<Entry> ptrEntry);

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
   QHash<QString, int> getTemplateArgumentsInName(const ArgumentList &templateArgumentList, const QString &name);

   void inheritDocumentation();
   bool isClassSection(QSharedPointer<Entry> ptrEntry);
   bool isRecursiveBaseClass(const QString &scope, const QString &name);
   bool isSpecialization(const QVector<ArgumentList> &srcTempArgLists, const QVector<ArgumentList> &dstTempArgLists);
   bool isVarWithConstructor(QSharedPointer<Entry> ptrEntry);

   void mergeCategories();

   void organizeSubGroupsFiltered(QSharedPointer<Entry> ptrEntry, bool additional);
   void organizeSubGroups(QSharedPointer<Entry> ptrEntry);

   void parseFile(ParserInterface *parser, QSharedPointer<Entry> ptrEntry,
                  QSharedPointer<FileDef> fd, QString fileName, enum ParserMode mode, QStringList &filesInSameTu);

   void parseFiles(QSharedPointer<Entry> ptrEntry);

   void processFiles();
   void processTagLessClasses(QSharedPointer<ClassDef> rootCd, QSharedPointer<ClassDef> cd, QSharedPointer<ClassDef>tagParentCd,
                  const QString &prefix, int count);

struct ReadDirArgs {
   bool recursive       = false;
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

   void readTagFile(QSharedPointer<Entry> root, const QString &tag_file);

   bool scopeIsTemplate(QSharedPointer<Definition> d);
   ArgumentList substituteTemplatesInArgList(const QVector<ArgumentList> &srcTempArgLists, QVector<ArgumentList> &dstTempArgLists,
                  const ArgumentList &srcList);

   QString substituteTemplatesInString( const QVector<ArgumentList> &srcTempArgLists, QVector<ArgumentList> &dstTempArgLists,
                  const ArgumentList &funcTemplateArgList, const QString &src);

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
   int cacheSize           = Config::getInt("lookup-cache-size");

   if (cacheSize < 0) {
      cacheSize = 0;

   } else if (cacheSize > 9) {
      cacheSize = 9;
   }

   uint lookupSize = (65536 << cacheSize);
   Doxy_Globals::lookupCache.setMaxCost(lookupSize);

#ifdef HAS_SIGNALS
   signal(SIGINT, stopDoxyPress);
#endif

   // Check/create output directories
   QString htmlOutput;
   static const bool generateHtml = Config::getBool("generate-html");

   if (generateHtml) {
      htmlOutput = createOutputDirectory(outputDirectory, "html-output", "/html");
   }

   QString docbookOutput;
   static const bool generateDocbook = Config::getBool("generate-docbook");

   if (generateDocbook) {
      docbookOutput = createOutputDirectory(outputDirectory, "docbook-output", "/docbook");
   }

   QString xmlOutput;
   static const bool generateXml = Config::getBool("generate-xml");

   if (generateXml) {
      xmlOutput = createOutputDirectory(outputDirectory, "xml-output", "/xml");
   }

   QString latexOutput;
   static const bool generateLatex = Config::getBool("generate-latex");

   if (generateLatex) {
      latexOutput = createOutputDirectory(outputDirectory, "latex-output", "/latex");
   }

   QString rtfOutput;
   static const bool generateRtf = Config::getBool("generate-rtf");

   if (generateRtf) {
      rtfOutput = createOutputDirectory(outputDirectory, "rtf-output", "/rtf");
   }

   QString manOutput;
   static const bool generateMan = Config::getBool("generate-man");

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
      msg("Parse layout file %s\n", csPrintable(layoutFileName));

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

   msg("Parse tag files\n");

   const QStringList tagFileList = Config::getList("tag-files");
   QSharedPointer<Entry> root = QMakeShared<Entry>();

   for (const auto &s : tagFileList) {
      readTagFile(root, s);
      root->createNavigationIndex(QSharedPointer<FileDef>());      // broom - (saving a nullptr which gets changed later on)
   }

   // parse source files
   if (Config::getBool("built-in-stl-support")) {
      addSTLClasses(root);
   }

   msg("\n**  ");
   Doxy_Globals::infoLog_Stat.begin("Parsing\n");
   parseFiles(root);

   Doxy_Globals::infoLog_Stat.end();

   // done with input scanning, free up the buffers used by lex (can be around 4MB)
   preFreeScanner();
   cstyleFreeParser();
   pyFreeParser();

   // gather information
   Doxy_Globals::infoLog_Stat.begin("Building group list\n");
   buildGroupList(root);
   organizeSubGroups(root);
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Building directory list\n");
   buildDirectories();
   findDirDocumentation(root);
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Building namespace list\n");
   buildNamespaceList(root);

   findUsingDirectives(root);
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Building file list\n");
   buildFileList(root);
   Doxy_Globals::infoLog_Stat.end();

   // generateFileTree()

   Doxy_Globals::infoLog_Stat.begin("Building class list\n");
   buildClassList(root);
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Associating documentation with classes\n");
   buildClassDocList(root);

   // build list of using declarations here (global list)
   buildListOfUsingDecls(root);
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Computing nesting relations for classes\n");
   resolveClassNestingRelations();
   Doxy_Globals::infoLog_Stat.end();

   // no longer add nested classes to the group as well
   // distributeClassGroupRelations();

   // calling buildClassList may result in cached relations which become invalid
   // after resolveClassNestingRelations(), that is why we clear the cache here
   Doxy_Globals::lookupCache.clear();

   // we do not need the list of using declaration anymore
   Doxy_Globals::g_usingDeclarations.clear();

   Doxy_Globals::infoLog_Stat.begin("Building example list\n");
   buildExampleList(root);
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Searching for enumerations\n");
   findEnums(root);
   Doxy_Globals::infoLog_Stat.end();

   // Since buildVarList calls isVarWithConstructor and this calls getResolvedClass we need to process typedefs
   // first so the relations between classes via typedefs are properly resolved. See bug 536385 for an example.

   Doxy_Globals::infoLog_Stat.begin("Searching for documented typedefs\n");
   buildTypedefList(root);
   Doxy_Globals::infoLog_Stat.end();

   // this should be after buildTypedefList in order to properly import used typedefs
   Doxy_Globals::infoLog_Stat.begin("Searching for members imported via using declarations\n");
   findUsingDeclarations(root);
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Searching for included using directives\n");
   findIncludedUsingDirectives();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Searching for documented variables\n");
   buildVarList(root);
   Doxy_Globals::infoLog_Stat.end();

   // UNO IDL
   Doxy_Globals::infoLog_Stat.begin("Building interface member list\n");
   buildInterfaceAndServiceList(root);

   // using class info only
   Doxy_Globals::infoLog_Stat.begin("Building member list\n");
   buildFunctionList(root);
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Searching for friends\n");
   findFriends();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Searching for documented defines\n");
   findDefineDocumentation(root);
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Computing class inheritance relations\n");
   findClassEntries(root);
   findInheritedTemplateInstances();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Computing class usage relations\n");
   findUsedTemplateInstances();
   Doxy_Globals::infoLog_Stat.end();

   if (Config::getBool("inline-simple-struct")) {
      Doxy_Globals::infoLog_Stat.begin("Searching for tag less structs\n");
      findTagLessClasses();
      Doxy_Globals::infoLog_Stat.end();
   }

   Doxy_Globals::infoLog_Stat.begin("Flushing cached template relations\n");
   flushCachedTemplateRelations();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Computing class relations\n");
   computeTemplateClassRelations();
   flushUnresolvedRelations();

   computeClassRelations();
   Doxy_Globals::g_classEntries.clear();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Add enum values to enums\n");
   addEnumValuesToEnums(root);
   findEnumDocumentation(root);
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Searching for member function documentation\n");
   findObjCMethodDefinitions(root);
   findMemberDocumentation(root);       // may introduce new members
   findUsingDeclImports(root);          // may introduce new members

   transferRelatedFunctionDocumentation();
   transferFunctionDocumentation();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Creating members for template instances\n");
   createTemplateInstanceMembersX();
   Doxy_Globals::infoLog_Stat.end();

   // added 12/2015
   Doxy_Globals::infoLog_Stat.begin("Resolve empty Namespaces\n");
   resolveHiddenNamespace();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Building page list\n");
   buildPageList(root);
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Search for main page\n");
   findMainPage(root);
   findMainPageTagFiles(root);
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Computing page relations\n");
   computePageRelations(root);
   checkPageRelations();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Determining the scope of groups\n");
   findGroupScope(root);
   Doxy_Globals::infoLog_Stat.end();

   msg("Freeing entry tree\n");

   Doxy_Globals::infoLog_Stat.begin("Determining which enums are documented\n");
   findDocumentedEnumValues();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Computing member relations\n");
   mergeCategories();
   computeMemberRelations();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Building full member lists recursively\n");
   buildCompleteMemberLists();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Adding members to member groups\n");
   addMembersToMemberGroup();
   Doxy_Globals::infoLog_Stat.end();

   if (Config::getBool("duplicate-docs")) {
      Doxy_Globals::infoLog_Stat.begin("Duplicating member group documentation\n");
      distributeMemberGroupDocumentation();
      Doxy_Globals::infoLog_Stat.end();
   }

   Doxy_Globals::infoLog_Stat.begin("Computing member references\n");
   computeMemberReferences();
   Doxy_Globals::infoLog_Stat.end();

   if (Config::getBool("inherit-docs")) {
      Doxy_Globals::infoLog_Stat.begin("Inheriting documentation\n");
      inheritDocumentation();
      Doxy_Globals::infoLog_Stat.end();
   }

   // compute the shortest possible names of all files
   // without losing the uniqueness of the file names.

   Doxy_Globals::infoLog_Stat.begin("Generating disk names\n");

   for (auto &item : Doxy_Globals::inputNameList) {
      item->generateDiskNames();
   }

   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Adding source references\n");
   addSourceReferences();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Adding xrefitems\n");
   addListReferences();
   generateXRefPages();
   Doxy_Globals::infoLog_Stat.end();

   if (Config::getBool("directory-graph")) {
      Doxy_Globals::infoLog_Stat.begin("Computing dependencies between directories\n");
      computeDirDependencies();
      Doxy_Globals::infoLog_Stat.end();
   }

   // Doxy_Globals::infoLog_Stat.begin("Resolving citations\n");
   // Doxy_Globals::citeDict->resolve();

   Doxy_Globals::infoLog_Stat.begin("Generating citations page\n");
   Doxy_Globals::citeDict.generatePage();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Counting data structures\n");
   countDataStructures();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Resolving user defined references\n");
   resolveUserReferences();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Finding anchors and sections in the documentation\n");
   findSectionsInDocumentation();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Transferring function references\n");
   transferFunctionReferences();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Combining using relations\n");
   combineUsingRelations();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Adding members to index pages\n");
   addMembersToIndex();
   Doxy_Globals::infoLog_Stat.end();
}

void generateOutput()
{
   // initialize output generators
   printf("\n**  Generate Documentation Output\n");

   if (Doxy_Globals::dumpGlossary) {
      dumpGlossary();
      exit(0);
   }
   // move to the output directory
   QString outputDir = Config::getString("output-dir");
   QDir::setCurrent(outputDir);

   initSearchIndexer();

   // add extra languages for which we can only produce syntax highlighted code
   addCodeOnlyMappings();

   const bool generateHtml        = Config::getBool("generate-html");
   const bool generateDocbook     = Config::getBool("generate-docbook");
   const bool generateLatex       = Config::getBool("generate-latex");
   const bool generatePerl        = Config::getBool("generate-perl");
   const bool generateMan         = Config::getBool("generate-man");
   const bool generateRtf         = Config::getBool("generate-rtf");
   const bool generateXml         = Config::getBool("generate-xml");

   const QString htmlOutput       = Config::getString("html-output");
   const QString latexOutput      = Config::getString("latex-output");
   const QString rtfOutput        = Config::getString("rtf-output");
   const QString dookbookOutput   = Config::getString("docbook-output");

   // only used when HTML is enabled
   const bool generateHtmlHelp    = Config::getBool("generate-chm");
   const bool generateEclipseHelp = Config::getBool("generate-eclipse");
   const bool generateQhp         = Config::getBool("generate-qthelp");
   const bool generateTreeView    = Config::getBool("generate-treeview");
   const bool generateDocSet      = Config::getBool("generate-docset");

   if (generateHtml) {
      Doxy_Globals::infoLog_Stat.begin("Enable HTML output\n");

      Doxy_Globals::outputList.add(QMakeShared<HtmlGenerator>());
      HtmlGenerator::init();

      if (generateEclipseHelp) {
         Doxy_Globals::indexList.addIndex(QSharedPointer<EclipseHelp>(new EclipseHelp));
      }

      if (generateHtmlHelp) {
         Doxy_Globals::indexList.addIndex(QSharedPointer<HtmlHelp>(new HtmlHelp));
      }

      if (generateQhp) {
         Doxy_Globals::indexList.addIndex(QSharedPointer<Qhp>(new Qhp));
      }

      if (generateTreeView) {
         Doxy_Globals::indexList.addIndex(QSharedPointer<FTVHelp>(new FTVHelp(true)));
      }

      if (generateDocSet) {
         Doxy_Globals::indexList.addIndex(QSharedPointer<DocSets>(new DocSets));
      }

      Doxy_Globals::indexList.initialize();
      HtmlGenerator::writeTabData();
   }

   if (generateDocbook) {
      Doxy_Globals::infoLog_Stat.begin("Enable Docbook output\n");
   }

   if (generateLatex) {
      Doxy_Globals::infoLog_Stat.begin("Enable Latex output\n");

      Doxy_Globals::outputList.add(QMakeShared<LatexGenerator>());
      LatexGenerator::init();
   }

   if (generateMan) {
      Doxy_Globals::infoLog_Stat.begin("Enable Man output\n");

      Doxy_Globals::outputList.add(QMakeShared<ManGenerator>());
      ManGenerator::init();
   }

   if (generatePerl) {
      Doxy_Globals::infoLog_Stat.begin("Enable Perl output\n");
   }

   if (generateRtf) {
      Doxy_Globals::infoLog_Stat.begin("Enable RTF output\n");

      Doxy_Globals::outputList.add(QMakeShared<RTFGenerator>());
      RTFGenerator::init();

      copyLogo(rtfOutput);
   }

   if (generateXml) {
      Doxy_Globals::infoLog_Stat.begin("Enable XML output\n");
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
      writeDoxFont(latexOutput);
   }

   if (generateRtf) {
      writeDoxFont(rtfOutput);
   }

   Doxy_Globals::infoLog_Stat.begin("Generating style sheet\n");

   // write first part
   Doxy_Globals::outputList.writeStyleInfo(0);
   Doxy_Globals::infoLog_Stat.end();

   static bool searchEngine      = Config::getBool("html-search");
   static bool serverBasedSearch = Config::getBool("search-server-based");

   // generate search indices (need to do this before writing other HTML
   // pages as these contain a drop down menu with options depending on
   // what categories we find in this function.

   if (generateHtml && searchEngine) {
      Doxy_Globals::infoLog_Stat.begin("Generating search index\n");
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

      Doxy_Globals::infoLog_Stat.end();
   }

   Doxy_Globals::infoLog_Stat.begin("Generating example documentation\n");
   generateExampleDocs();
   Doxy_Globals::infoLog_Stat.end();

   if (! Htags::useHtags) {
      Doxy_Globals::infoLog_Stat.begin("Generating source code\n");
      generateSourceCode();
      Doxy_Globals::infoLog_Stat.end();
   }

   Doxy_Globals::infoLog_Stat.begin("Generating file documentation\n");
   generateFileDocs();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Generating page documentation\n");
   generatePageDocs();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Generating group documentation\n");
   generateGroupDocs();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Generating class documentation\n");
   generateClassDocs();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Generating namespace documentation\n");
   generateNamespaceDocs();
   Doxy_Globals::infoLog_Stat.end();

   if (Config::getBool("generate-legend")) {
      Doxy_Globals::infoLog_Stat.begin("Generating graph documentation\n");
      writeGraphInfo(Doxy_Globals::outputList);
      Doxy_Globals::infoLog_Stat.end();
   }

   Doxy_Globals::infoLog_Stat.begin("Generating directory documentation\n");
   generateDirDocs(Doxy_Globals::outputList);
   Doxy_Globals::infoLog_Stat.end();

   if (Doxy_Globals::formulaList.count() > 0 && generateHtml && ! Config::getBool("use-mathjax")) {
      Doxy_Globals::infoLog_Stat.begin("Generating bitmaps for formulas in HTML\n");
      Doxy_Globals::formulaList.generateBitmaps(htmlOutput);
      Doxy_Globals::infoLog_Stat.end();
   }

   if (Doxy_Globals::formulaList.count() > 0 && generateRtf) {
      Doxy_Globals::infoLog_Stat.begin("Generating bitmaps for formulas in RTF\n");
      Doxy_Globals::formulaList.generateBitmaps(rtfOutput);
      Doxy_Globals::infoLog_Stat.end();
  }

   if (Config::getBool("sort-group-names")) {
      // groupSDict -- always sorted
      // sort the sub groups
   }

   if (Doxy_Globals::outputList.count() > 0) {
      writeIndexHierarchy(Doxy_Globals::outputList);
   }

   Doxy_Globals::infoLog_Stat.begin("Finalizing index pages\n");
   Doxy_Globals::indexList.finalize();
   Doxy_Globals::infoLog_Stat.end();

   Doxy_Globals::infoLog_Stat.begin("Writing tag file\n");
   writeTagFile();
   Doxy_Globals::infoLog_Stat.end();

   if (Config::getBool("dot-cleanup")) {
      if (generateHtml) {
         removeDoxFont(htmlOutput);
      }

      if (generateRtf) {
         removeDoxFont(rtfOutput);
      }

      if (generateLatex) {
         removeDoxFont(latexOutput);
      }
   }

   if (generateXml) {
      Doxy_Globals::infoLog_Stat.begin("Generating XML output\n");

      Doxy_Globals::generatingXmlOutput = true;
      generateXML_output();
      Doxy_Globals::generatingXmlOutput = false;

      Doxy_Globals::infoLog_Stat.end();
   }

   if (generateDocbook) {
      Doxy_Globals::infoLog_Stat.begin("Generating Docbook output\n");
      generateDocbook_output();
      Doxy_Globals::infoLog_Stat.end();
   }

   if (generatePerl) {
      Doxy_Globals::infoLog_Stat.begin("Generating Perl output\n");
      generatePerl_output();
      Doxy_Globals::infoLog_Stat.end();
   }

   if (generateHtml && searchEngine && serverBasedSearch) {
      Doxy_Globals::infoLog_Stat.begin("Generating search index\n");

      if (Doxy_Globals::searchIndexBase->kind() == SearchIndex_Base::Internal) {
         // write own search index

         HtmlGenerator::writeSearchPage();
         Doxy_Globals::searchIndexBase->write(htmlOutput + "/search/search.idx");

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

         Doxy_Globals::searchIndexBase->write(searchDataFile);
      }

      Doxy_Globals::infoLog_Stat.end();
   }

   if (generateRtf) {
      Doxy_Globals::infoLog_Stat.begin("Post process RTF output\n");

      if (! RTFGenerator::preProcessFileInplace(rtfOutput, "refman.rtf")) {
         err("Error occurred during post processing of RTF files\n");
      }

      Doxy_Globals::infoLog_Stat.end();
   }

   if (Config::getBool("have-dot")) {
      Doxy_Globals::infoLog_Stat.begin("Running dot\n");

      DotManager::instance()->run();
      Doxy_Globals::infoLog_Stat.end();
   }

   // copy static files
   if (generateHtml)     {
      FTVHelp::generateTreeViewImages();

      copyStyleSheet();
      copyLogo(htmlOutput);
      copyExtraFiles("html");
   }

   if (generateLatex)  {
      Doxy_Globals::infoLog_Stat.begin("Post process Latex output\n");

      copyLatexStyleSheet();
      copyLogo(latexOutput);
      copyExtraFiles("latex");

      Doxy_Globals::infoLog_Stat.end();
   }

   if (generateHtml && generateHtmlHelp && ! Config::getString("hhc-location").isEmpty()) {

      Doxy_Globals::infoLog_Stat.begin("Running html help compiler\n");
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
      Doxy_Globals::infoLog_Stat.end();
   }

   if ( generateHtml && generateQhp && ! Config::getString("qthelp-gen-path").isEmpty()) {
      Doxy_Globals::infoLog_Stat.begin("Running QtHelp generator\n");

      QString qhpFileName = Qhp::getQhpFileName();
      QString qchFileName = getQchFileName();

      QString args = QString("%1 -o \"%2\"").formatArgs(qhpFileName, qchFileName);

      QString const oldDir = QDir::currentPath();
      QDir::setCurrent(htmlOutput);

      portable_sysTimerStart();

      if (portable_system(Config::getString("qthhelp-gen-path"), args, false)) {
         err("Unable to run qhelpgenerator on 'index.qhp'\n");
      }

      portable_sysTimerStop();
      QDir::setCurrent(oldDir);
      Doxy_Globals::infoLog_Stat.end();
   }

   msg("Lookup cache used %d/%d \n", Doxy_Globals::lookupCache.count(), Doxy_Globals::lookupCache.size());
   msg("Finished\n");

   // all done, cleaning up and exit
   shutDownDoxyPress();
   Doxy_Globals::programExit = true;
}

// ** other
ArgumentList getTemplateArgumentsFromName(const QString &name, const QVector<ArgumentList> &tArgLists)
{
   ArgumentList retval = ArgumentList();

   if (tArgLists.isEmpty()) {
      return retval;
   }

   // for each scope fragment, check if it is a template and advance through the list if so
   int index;
   int p = 0;

   auto item = tArgLists.begin();

   while (true) {
      index = name.indexOf("::", p);

      if (index == -1) {
         break;
      }

      QSharedPointer<NamespaceDef> nd (Doxy_Globals::namespaceSDict.find(name.left(index)));

      if (nd == nullptr) {
         QSharedPointer<ClassDef> cd = getClass(name.left(index));

         if (cd) {
            if (! cd->getTemplateArgumentList().listEmpty()) {
               ++item;

               if (item == tArgLists.end()) {
                  break;
               }
            }
         }
      }

      p = index + 2;
   }

   if (item != tArgLists.end()) {
      retval = *item;
   }

   return retval;
}

// **  functions
void clearAll()
{
   Doxy_Globals::g_inputFiles.clear();

   Doxy_Globals::classSDict.clear();
   Doxy_Globals::namespaceSDict.clear();
   Doxy_Globals::pageSDict.clear();
   Doxy_Globals::exampleSDict.clear();
   Doxy_Globals::inputNameList.clear();
   Doxy_Globals::formulaList.clear();
   Doxy_Globals::sectionDict.clear();
   Doxy_Globals::inputNameDict.clear();
   Doxy_Globals::includeNameDict.clear();
   Doxy_Globals::exampleNameDict.clear();
   Doxy_Globals::imageNameDict.clear();
   Doxy_Globals::dotFileNameDict.clear();
   Doxy_Globals::mscFileNameDict.clear();
   Doxy_Globals::diaFileNameDict.clear();

   Doxy_Globals::formulaDict.clear();
   Doxy_Globals::formulaNameDict.clear();
   Doxy_Globals::tagDestinationDict.clear();

   Doxy_Globals::mainPage = QSharedPointer<PageDef>();
}

void distributeClassGroupRelations()
{
   // static bool inlineGroupedClasses = Config::getBool("inline-grouped-classes");

   for (auto cd : Doxy_Globals::classSDict) {
      cd->visited = false;
   }

   for (auto cd : Doxy_Globals::classSDict) {
      // distribute the group to nested classes as well

      QSharedPointer<GroupDef> gd = cd->partOfGroups()->at(0);

      if (! cd->visited && cd->partOfGroups() != 0) {

         for (auto ncd : cd->getClassSDict() ) {
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
void Doxy_Work::addSTLMember(QSharedPointer<Entry> ptrEntry, const QString &type, const QString &name)
{
   QSharedPointer<Entry> memEntry = QMakeShared<Entry>();

   memEntry->m_entryName = name;
   memEntry->protection  = Public;
   memEntry->section     = Entry::Sections::VARIABLE_SEC;
   memEntry->hidden      = false;
   memEntry->artificial  = true;

   memEntry->setData(EntryKey::Member_Type,  type);
   memEntry->setData(EntryKey::Brief_Docs,   "STL member");

   ptrEntry->addSubEntry(memEntry);
}

void Doxy_Work::addSTLIterator(QSharedPointer<Entry> ptrEntry, const QString &name)
{
   QSharedPointer<Entry> iteratorClassEntry = QMakeShared<Entry>();

   iteratorClassEntry->m_entryName = name;
   iteratorClassEntry->startLine   = 1;
   iteratorClassEntry->section     = Entry::Sections::CLASS_SEC;
   iteratorClassEntry->hidden      = false;
   iteratorClassEntry->artificial  = true;

   iteratorClassEntry->setData(EntryKey::File_Name,    "[STL]");
   iteratorClassEntry->setData(EntryKey::Brief_Docs,   "STL iterator class");

   ptrEntry->addSubEntry(iteratorClassEntry);
}

void Doxy_Work::addSTLClasses(QSharedPointer<Entry> ptrEntry)
{
   QSharedPointer<Entry> namespaceEntry = QMakeShared<Entry>();

   namespaceEntry->m_entryName = "std";
   namespaceEntry->startLine   = 1;
   namespaceEntry->section     = Entry::Sections::NAMESPACE_SEC;
   namespaceEntry->hidden      = false;
   namespaceEntry->artificial  = true;

   namespaceEntry->setData(EntryKey::File_Name,   "[STL]");
   namespaceEntry->setData(EntryKey::Brief_Docs,  "STL namespace");

   ptrEntry->addSubEntry(namespaceEntry);

   STLInfo *info = g_stlinfo;

   while (info->className) {
      QString fullName = QString::fromLatin1(info->className);
      fullName.prepend("std::");

      // add fake Entry for the class
      QSharedPointer<Entry> classEntry = QMakeShared<Entry>();

      classEntry->m_entryName = fullName;
      classEntry->startLine   = 1;
      classEntry->section     = Entry::CLASS_SEC;
      classEntry->hidden      = false;
      classEntry->artificial  = true;

      classEntry->setData(EntryKey::File_Name,   "[STL]");
      classEntry->setData(EntryKey::Brief_Docs,  "STL class");

      namespaceEntry->addSubEntry(classEntry);

      // add template arguments to class
      if (info->templType1) {
         ArgumentList al;

         Argument arg;
         arg.type = "typename";
         arg.name = info->templType1;
         al.append(arg);

         if (info->templType2) {
            // another template argument
            arg = Argument();

            arg.type = "typename";
            arg.name = info->templType2;
            al.append(arg);
         }

         classEntry->m_templateArgLists.clear();
         classEntry->m_templateArgLists.append(al);
      }

      // add member variables
      if (info->templName1) {
         addSTLMember(classEntry, QString::fromLatin1(info->templType1), QString::fromLatin1(info->templName1));
      }

      if (info->templName2) {
         addSTLMember(classEntry, QString::fromLatin1(info->templType2), QString::fromLatin1(info->templName2));
      }

      if (fullName == "std::auto_ptr" || fullName == "std::smart_ptr" || fullName=="std::shared_ptr" ||
            fullName == "std::unique_ptr" || fullName == "std::weak_ptr") {

         QSharedPointer<Entry> memEntry = QMakeShared<Entry>();

         memEntry->m_entryName = "operator->";
         memEntry->protection  = Public;
         memEntry->section     = Entry::FUNCTION_SEC;
         memEntry->hidden      = false;
         memEntry->artificial  = false;

         memEntry->setData(EntryKey::Brief_Docs,    "STL member");
         memEntry->setData(EntryKey::Member_Args,   "()");
         memEntry->setData(EntryKey::Member_Type,   "T*");

         classEntry->addSubEntry(memEntry);
      }

      if (info->baseClass1) {
         classEntry->extends.append(BaseInfo(QString::fromLatin1(info->baseClass1), Public, info->virtualInheritance
                  ? Specifier::Virtual : Specifier::Normal));
      }

      if (info->baseClass2) {
         classEntry->extends.append(BaseInfo(QString::fromLatin1(info->baseClass2), Public, info->virtualInheritance
                  ? Specifier::Virtual : Specifier::Normal));
      }

      if (info->iterators) {
         // add iterator class
         addSTLIterator(classEntry, fullName + "::iterator");
         addSTLIterator(classEntry, fullName + "::const_iterator");
         addSTLIterator(classEntry, fullName + "::reverse_iterator");
         addSTLIterator(classEntry, fullName + "::const_reverse_iterator");
      }

      info++;
   }
}

void Doxy_Work::addPageToContext(QSharedPointer<PageDef> pd, QSharedPointer<Entry> ptrEntry)
{
   if (ptrEntry->parent()) {
      // add the page to it's scope
      QString scope = ptrEntry->parent()->m_entryName;

      if (ptrEntry->parent()->section == Entry::PACKAGEDOC_SEC) {
         scope = substitute(scope, ".", "::");
      }

      scope = stripAnonymousNamespaceScope(scope);
      scope += "::" + pd->name();

      QSharedPointer<Definition> def = findScopeFromQualifiedName(Doxy_Globals::globalScope, scope,
                  QSharedPointer<FileDef>(), ptrEntry->m_tagInfo);

      if (def) {
         pd->setPageScope(def);
      }
   }
}

void Doxy_Work::addRelatedPage_X(QSharedPointer<Entry> ptrEntry)
{
   QSharedPointer<Entry> root = ptrEntry->entry();
   QSharedPointer<GroupDef> gd;

   for (auto &g : root->m_groups) {
      if (! g.groupname.isEmpty() && (gd = Doxy_Globals::groupSDict.find(g.groupname))) {
         break;
      }
   }

   QString doc;
   QString tmpBriefDocs = root->getData(EntryKey::Brief_Docs);

   if (tmpBriefDocs.isEmpty()) {
      doc = root->getData(EntryKey::Main_Docs) + root->getData(EntryKey::Inbody_Docs);

   } else {
      doc = tmpBriefDocs + "\n\n" + root->getData(EntryKey::Main_Docs) + root->getData(EntryKey::Inbody_Docs);
   }

   QSharedPointer<PageDef> pd = addRelatedPage(root->m_entryName, root->getData(EntryKey::Member_Args),
                  doc, root->getData(EntryKey::MainDocs_File), root->docLine, root->m_specialLists, gd,
                  ptrEntry->m_tagInfo, root->m_srcLang);

   if (pd) {
      pd->setBriefDescription(tmpBriefDocs, root->getData(EntryKey::Brief_File), root->briefLine);
      pd->addSectionsToDefinition(root->m_anchors);
      pd->setLocalToc(root->localToc);

      addPageToContext(pd, ptrEntry);
   }
}

void Doxy_Work::buildGroupListFiltered(QSharedPointer<Entry> ptrEntry, bool additional, bool includeExternal)
{
   if (ptrEntry->section == Entry::GROUPDOC_SEC && ! ptrEntry->m_entryName.isEmpty() &&
         ((! includeExternal && ptrEntry->m_tagInfo.isEmpty()) || ( includeExternal && ! ptrEntry->m_tagInfo.isEmpty())) ) {

      QSharedPointer<Entry> root = ptrEntry->entry();

      if ((root->groupDocType == Entry::GROUPDOC_NORMAL && ! additional) ||
            (root->groupDocType != Entry::GROUPDOC_NORMAL && additional)) {

         QSharedPointer<GroupDef> gd = Doxy_Globals::groupSDict.find(root->m_entryName);

         if (gd) {
            QString entryMemberType = root->getData(EntryKey::Member_Type);

            if (! gd->hasGroupTitle() ) {
               gd->setGroupTitle(entryMemberType);

            } else if (entryMemberType.length() > 0 && root->m_entryName != entryMemberType &&
                     gd->groupTitle() != entryMemberType ) {

               warn( root->getData(EntryKey::File_Name), root->startLine,
                     "Group %s: ignoring title \"%s\" which does not match old title \"%s\"\n",
                     csPrintable(root->m_entryName), csPrintable(entryMemberType), csPrintable(gd->groupTitle()) );
            }

            gd->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
            gd->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
            gd->setInbodyDocumentation( root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File),
                  root->inbodyLine );
            gd->addSectionsToDefinition(root->m_anchors);
            gd->setRefItems(root->m_specialLists);
            gd->setLanguage(root->m_srcLang);

         } else {
            if (! ptrEntry->m_tagInfo.isEmpty()) {
               gd = QMakeShared<GroupDef>(root->getData(EntryKey::File_Name), root->startLine, root->m_entryName,
                  root->getData(EntryKey::Member_Type), ptrEntry->m_tagInfo.tag_FileName);

               gd->setReference(ptrEntry->m_tagInfo.tag_Name);

            } else {
               gd = QMakeShared<GroupDef>(root->getData(EntryKey::File_Name), root->startLine, root->m_entryName,
                  root->getData(EntryKey::Member_Type));
            }

            gd->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);

            // allow empty docs for group
            gd->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine, false);
            gd->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);
            gd->addSectionsToDefinition(root->m_anchors);

            Doxy_Globals::groupSDict.insert(root->m_entryName, gd);
            gd->setRefItems(root->m_specialLists);
            gd->setLanguage(root->m_srcLang);
         }
      }
   }

   for (auto e : ptrEntry->children() ) {
      buildGroupListFiltered(e, additional, includeExternal);
   }
}

void Doxy_Work::buildGroupList(QSharedPointer<Entry> ptrEntry)
{
   // --- first process only local groups
   // first process the @defgroups blocks
   buildGroupListFiltered(ptrEntry, false, false);

   // then process the @addtogroup, @weakgroup blocks
   buildGroupListFiltered(ptrEntry, true, false);

   // --- then also process external groups
   // first process the @defgroups blocks
   buildGroupListFiltered(ptrEntry, false, true);

   // then process the @addtogroup, @weakgroup blocks
   buildGroupListFiltered(ptrEntry, true, true);
}

void Doxy_Work::findGroupScope(QSharedPointer<Entry> ptrEntry)
{
   if (ptrEntry->section == Entry::GROUPDOC_SEC && ! ptrEntry->m_entryName.isEmpty() &&
         ptrEntry->parent() && ! ptrEntry->parent()->m_entryName.isEmpty()) {

      QSharedPointer<GroupDef> gd;

      if ((gd = Doxy_Globals::groupSDict.find(ptrEntry->m_entryName))) {
         QString scope = ptrEntry->parent()->m_entryName;

         if (ptrEntry->parent()->section == Entry::PACKAGEDOC_SEC) {
            scope = substitute(scope, ".", "::");
         }

         scope = stripAnonymousNamespaceScope(scope);
         scope += "::" + gd->name();

         QSharedPointer<Definition> def = findScopeFromQualifiedName(Doxy_Globals::globalScope, scope,
                  QSharedPointer<FileDef>(), ptrEntry->m_tagInfo);

         if (def) {
            gd->setGroupScope(def);
         }
      }
   }

   RECURSE_ENTRYTREE(findGroupScope, ptrEntry);
}

void Doxy_Work::organizeSubGroupsFiltered(QSharedPointer<Entry> ptrEntry, bool additional)
{
   if (ptrEntry->section == Entry::GROUPDOC_SEC && ! ptrEntry->m_entryName.isEmpty()) {

      QSharedPointer<Entry>root = ptrEntry->entry();

      if ((root->groupDocType == Entry::GROUPDOC_NORMAL && !additional) ||
            (root->groupDocType != Entry::GROUPDOC_NORMAL && additional)) {

         QSharedPointer<GroupDef> gd;

         if ((gd = Doxy_Globals::groupSDict.find(root->m_entryName))) {
            addGroupToGroups(root, gd);
         }
      }
   }

   for (auto e : ptrEntry->children() ) {
      organizeSubGroupsFiltered(e, additional);
   }
}

void Doxy_Work::organizeSubGroups(QSharedPointer<Entry> ptrEntry)
{
   // first process the @defgroups blocks
   organizeSubGroupsFiltered(ptrEntry, false);

   // then process the @addtogroup, @weakgroup blocks
   organizeSubGroupsFiltered(ptrEntry, true);
}

void Doxy_Work::buildFileList(QSharedPointer<Entry> ptrEntry)
{
   static const bool extractAll = Config::getBool("extract-all");

   if (((ptrEntry->section == Entry::FILEDOC_SEC) ||
         ((ptrEntry->section & Entry::FILE_MASK) && extractAll)) &&
         ! ptrEntry->m_entryName.isEmpty() && ptrEntry->m_tagInfo.isEmpty() ) {


      // skip any file coming from tag files
      QSharedPointer<Entry> root = ptrEntry->entry();

      bool ambig;
      QSharedPointer<FileDef> fd = findFileDef(&Doxy_Globals::inputNameDict, root->m_entryName, ambig);

      if (fd && ! ambig) {
         {
            // using false in setDocumentation is small hack to make sure a file
            // is documented even if a \file command is used without further documentation

            fd->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine, false);
            fd->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
            fd->addSectionsToDefinition(root->m_anchors);
            fd->setRefItems(root->m_specialLists);

            for (auto &g : root->m_groups) {
               QSharedPointer<GroupDef> gd;

               if (! g.groupname.isEmpty() && (gd = Doxy_Globals::groupSDict.find(g.groupname))) {
                  gd->addFile(fd);
                  fd->makePartOfGroup(gd);
               }
            }
         }

      } else {
         const QString fn = root->getData(EntryKey::File_Name);

         QString text;
         text = QString("Name `%1' supplied as the second argument in the \\file statement ").formatArg(root->m_entryName);

         if (ambig) {
            // name is ambiguous

            text += "matches the following input files:\n";
            text += showFileDefMatches(Doxy_Globals::inputNameDict, root->m_entryName);
            text += "Use a more specific name or include a larger part of the path";

         } else {
            // name is not an input file
            text += "is not an input file";
         }

         warn(fn, root->startLine, text);
      }
   }

   RECURSE_ENTRYTREE(buildFileList, ptrEntry);
}

void Doxy_Work::addIncludeFileClass(QSharedPointer<ClassDef> cd, QSharedPointer<FileDef> include_fd, QSharedPointer<Entry> root)
{
   static const bool forceLocalInc = Config::getBool("force-local-includes");
   static const bool extractAll    = Config::getBool("extract-all");

   static const QStringList stripFromIncPath = Config::getList("strip-from-inc-path");

   QString tmpMainDocs  = root->getData(EntryKey::Main_Docs).trimmed();
   QString tmpBriefDocs = root->getData(EntryKey::Brief_Docs).trimmed();

   if ( (! tmpMainDocs.isEmpty() || ! tmpBriefDocs.isEmpty() || extractAll) && root->protection != Private)  {
      bool local_inc      = forceLocalInc;
      QString includeFile = root->getData(EntryKey::Include_File);

      if (! includeFile.isEmpty() && includeFile.at(0) == '"') {
         local_inc   = true;
         includeFile = includeFile.mid(1, includeFile.length() - 2);

      } else if (! includeFile.isEmpty() && includeFile.at(0) == '<') {
         local_inc   = false;
         includeFile = includeFile.mid(1, includeFile.length() - 2);
      }

      bool ambig;
      QSharedPointer<FileDef> fd;

      // do need to include a verbatim copy of the header file

      if (! includeFile.isEmpty() && (fd = findFileDef(&Doxy_Globals::inputNameDict, includeFile, ambig)) == nullptr) {
         // explicit request

         QString text = QString("the name `%1' supplied as the argument of the \\class, \\struct, \\union, or \\include command ")
                        .formatArg(includeFile);

         if (ambig) {
            // name is ambiguous

            text += "matches the following input files:\n";
            text += showFileDefMatches(Doxy_Globals::inputNameDict, root->getData(EntryKey::Include_File));
            text += "Use a more specific name by including a larger part of the path\n";

         } else {
            // name is not an input file
            text += "is not an input file\n";
         }

         warn(root->getData(EntryKey::File_Name), root->startLine, text);

      } else if (includeFile.isEmpty() && include_fd && determineSection(include_fd->name()) == Entry::HEADER_SEC) {
         fd = include_fd;
      }

      // if a file is found, mark it as a source file
      if (fd != nullptr) {
         QString iName = root->getData(EntryKey::Include_Name);

         if (iName.isEmpty()) {
            iName = includeFile;
         }

         if (! iName.isEmpty()) {
            // user specified include file

            if (iName.at(0) == '<') {
               // explicit override
               local_inc = false;

            } else if (iName.at(0) == '"') {
               local_inc = true;

            }

            if (iName.at(0) == '"' || iName.at(0) == '<') {
               // strip quotes or brackets
               iName = iName.mid(1, iName.length() - 2);
            }

            if (iName.isEmpty()) {
               iName = fd->name();
            }

         } else if (! stripFromIncPath.isEmpty()) {
            iName = stripFromIncludePath(fd->getFilePath());

         } else {
            // use name of the file containing the class definition
            iName = fd->name();
         }

         if (fd->generateSourceFile()) {
            // generate code for header
            cd->setIncludeFile(fd, iName, local_inc, ! root->getData(EntryKey::Include_Name).isEmpty());

         } else {
            // put #include in the class documentation without link
            cd->setIncludeFile(QSharedPointer<FileDef>(), iName, local_inc, true);

         }
      }
   }
}

/*  returns the Definition object belonging to the first \a level levels of
 *  full qualified name \a name. Creates an artificial scope if the scope is
 *  not found and set the parent/child scope relation if the scope is found.
 */
QSharedPointer<Definition> Doxy_Work::buildScopeFromQualifiedName(const QString name, int level,
                  SrcLangExt lang, const TagInfo &tagInfo)
{
   int i = 0;
   int p = 0;
   int len;

   QSharedPointer<Definition> prevScope = Doxy_Globals::globalScope;
   QString fullScope;

   while (i < level) {
      int idx = getScopeFragment(name, p, &len);

      if (idx == -1) {
         return prevScope;
      }

      QString nsName = name.mid(idx, len);

      if (nsName.isEmpty()) {
         return prevScope;
      }

      if (! fullScope.isEmpty()) {
         fullScope += "::";
      }

      fullScope += nsName;
      QSharedPointer<NamespaceDef> nd (Doxy_Globals::namespaceSDict.find(fullScope));

      QSharedPointer<Definition> innerScope = nd;
      QSharedPointer<ClassDef> cd;

      if (! nd) {
         cd = getClass(fullScope);
      }

      if (nd == nullptr && cd) {
         // scope is a class
         innerScope = cd;

      } else if (nd == nullptr && cd == nullptr && ! fullScope.contains('<')) {
         // scope is not known and could be a namespace, introduce bogus namespace

         nd = QMakeShared<NamespaceDef>( "[generated]", 1, 1, fullScope, tagInfo.tag_Name, tagInfo.tag_FileName);
         nd->setLanguage(lang);

         // add namespace to the list
         Doxy_Globals::namespaceSDict.insert(fullScope, nd);
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
      p = idx + len + 2;
      prevScope = innerScope;
      i++;
   }

   return prevScope;
}

QSharedPointer<Definition> Doxy_Work::findScopeFromQualifiedName(QSharedPointer<Definition> startScope, const QString &n,
                  QSharedPointer<FileDef> fileScope, const TagInfo &tagInfo)
{
   QSharedPointer<Definition> resultScope = startScope;

   if (! resultScope) {
      resultScope = Doxy_Globals::globalScope;
   }

   QString scope = stripTemplateSpecifiersFromScope(n, false);
   int len1      = 0;
   int index1    = getScopeFragment(scope, 0, &len1);

   if (index1 == -1) {
      return resultScope;
   }

   int p    = index1 + len1;
   int len2 = 0;
   int index2;

   while ((index2 = getScopeFragment(scope, p, &len2)) != -1) {
      QString nestedNameSpecifier = scope.mid(index1, len1);

      QSharedPointer<Definition> orgScope = resultScope;
      resultScope = resultScope->findInnerCompound(nestedNameSpecifier);

      if (! resultScope) {
         const NamespaceSDict *usedNamespaces;

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
               // for a nested class A::I in used namespace N, we get N::A::I while looking for A,
               // so we should compare resultScope->name() against scope.left(index2 +len2)

               if (rightScopeMatch(resultScope->name(), scope.left(index2 + len2))) {
                  break;
               }

               index1 = index2;
               len1   = len2;
               p      = index2 + len2;

               continue;
            }
         }

         // search for used classes, issue: not been able to put them in the right scope yet,
         // because we are still resolving the scope relations
         // therefore loop through all used classes and see if there is a right scope match between the
         // used class and nestedNameSpecifier.

         auto item = Doxy_Globals::g_usingDeclarations.begin();

         for (auto usedFd : Doxy_Globals::g_usingDeclarations)  {

            if (rightScopeMatch(item.key(), nestedNameSpecifier)) {
               // item.key()is the fully qualified name of nestedNameSpecifier

               QString fqn = item.key() + scope.right(scope.length() - p);
               resultScope = buildScopeFromQualifiedName(fqn, fqn.count("::"), startScope->getLanguage(), TagInfo());

               if (resultScope) {
                  return resultScope;
               }
            }

            ++item;
         }

         return QSharedPointer<Definition>();
      }

      index1 = index2;
      len1   = len2;
      p      = index2 + len2;
   }

   return resultScope;
}

enum CompoundType Doxy_Work::convertToCompoundType(int section, Entry::Traits data)
{
   enum CompoundType sec = CompoundType::Class;

   if (data.hasTrait(Entry::Virtue::Struct)) {
      sec = CompoundType::Struct;

   } else if (data.hasTrait(Entry::Virtue::Union)) {
      sec = CompoundType::Union;

   } else if (data.hasTrait(Entry::Virtue::Category)) {
      sec = CompoundType::Category;

   } else if (data.hasTrait(Entry::Virtue::Interface)) {
      sec = CompoundType::Interface;

   } else if (data.hasTrait(Entry::Virtue::Protocol)) {
      sec = CompoundType::Protocol;

   } else if (data.hasTrait(Entry::Virtue::Exception)) {
      sec = CompoundType::Exception;

   } else if (data.hasTrait(Entry::Virtue::Service)) {
      sec = CompoundType::Service;

   } else if (data.hasTrait(Entry::Virtue::Singleton)) {
      sec = CompoundType::Singleton;
   }

   switch (section) {
      case Entry::UNIONDOC_SEC:
         sec =  CompoundType::Union;
         break;

      case Entry::STRUCTDOC_SEC:
         sec = CompoundType::Struct;
         break;

      case Entry::INTERFACEDOC_SEC:
         sec = CompoundType::Interface;
         break;

      case Entry::PROTOCOLDOC_SEC:
         sec = CompoundType::Protocol;
         break;

      case Entry::CATEGORYDOC_SEC:
         sec = CompoundType::Category;
         break;

      case Entry::EXCEPTIONDOC_SEC:
         sec = CompoundType::Exception;
         break;

      case Entry::SERVICEDOC_SEC:
         sec = CompoundType::Service;
         break;

      case Entry::SINGLETONDOC_SEC:
         sec = CompoundType::Singleton;
         break;
   }

   return sec;
}

void Doxy_Work::addClassToContext(QSharedPointer<Entry> ptrEntry)
{
   QSharedPointer<Entry> root = ptrEntry;
   QSharedPointer<FileDef> fd = ptrEntry->fileDef();

   QString scName;
   if (ptrEntry->parent()->section & Entry::SCOPE_MASK) {
      scName = ptrEntry->parent()->m_entryName;
   }

   // name without parent scope
   QString fullName = root->m_entryName;

   // strip off any template parameters (but not those for specializations)
   fullName = stripTemplateSpecifiersFromScope(fullName);

   // name with scope (if not present already)
   QString qualifiedName = fullName;

   if (! scName.isEmpty() && ! leftScopeMatch(fullName, scName)) {
      qualifiedName.prepend(scName + "::");
   }

   // check if this class has been seen before
   QSharedPointer<ClassDef> cd = getClass(qualifiedName);

   Debug::print(Debug::Classes, 0, "  Found class with name %s (qualifiedName=%s -> cd=%p)\n",
                cd ? csPrintable(cd->name()) : csPrintable(root->m_entryName), csPrintable(qualifiedName), cd.data() );

   if (cd) {
      // class exists
      fullName = cd->name();
      Debug::print(Debug::Classes, 0, "  Existing class %s\n", csPrintable(cd->name()) );

      cd->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
      cd->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);

      if (root->startBodyLine != -1 && cd->getStartBodyLine() == -1) {
         cd->setBodySegment(root->startBodyLine, root->endBodyLine);
         cd->setBodyDef(fd);
      }

      if (! root->m_templateArgLists.isEmpty() &&
                  (cd->getTemplateArgumentList().listEmpty() || (cd->isForwardDeclared() &&
                  (! root->m_traits.hasTrait(Entry::Virtue::ForwardDecl)) ))) {

         // happens if a template class is declared before the actual definition or
         // if a forward declaration has different template parameter names

         ArgumentList tArgList = getTemplateArgumentsFromName(cd->name(), root->m_templateArgLists);
         cd->setTemplateArgumentList(tArgList);
      }

      cd->setCompoundType(convertToCompoundType(root->section, root->m_traits));

   } else {
      // new class
      enum CompoundType sec = convertToCompoundType(root->section, root->m_traits);

      QString className;
      QString namespaceName;
      extractNamespaceName(fullName, className, namespaceName);

      QString tagName;
      QString refFileName;

      const TagInfo &tagInfo = ptrEntry->m_tagInfo;

      if (! tagInfo.isEmpty()) {
         tagName     = tagInfo.tag_Name;
         refFileName = tagInfo.tag_FileName;

         if (fullName.contains("::")) {
            // symbols imported via tag files may come without the parent scope
            // so we artificially create it here

            buildScopeFromQualifiedName(fullName, fullName.count("::"), root->m_srcLang, tagInfo);
         }
      }

      ArgumentList tArgList;
      int i;

      if ((root->m_srcLang == SrcLangExt_CSharp || root->m_srcLang == SrcLangExt_Java) && (i = fullName.indexOf('<')) != -1) {
         // a Java/C# generic class looks like a C++ specialization, split the name and template arguments here

         tArgList = stringToArgumentList(fullName.mid(i) );
         fullName = fullName.left(i);

      } else {
         tArgList = getTemplateArgumentsFromName(fullName, root->m_templateArgLists);

      }

      bool isEnum = root->m_traits.hasTrait(Entry::Virtue::Enum);

      cd = QMakeShared<ClassDef>(! tagInfo.isEmpty() ? tagName : root->getData(EntryKey::File_Name),
                  root->startLine, root->startColumn, fullName, sec, tagName, refFileName, true, isEnum);

      // copy docs to definition
      cd->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
      cd->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
      cd->setLanguage(root->m_srcLang);
      cd->setId(root->getData(EntryKey::Clang_Id));

      cd->setHidden(root->hidden);
      cd->setArtificial(root->artificial);
      cd->setClassTraits(root->m_traits);

      cd->setTypeConstraints(root->typeConstr);
      cd->setTemplateArgumentList(tArgList);

      cd->setProtection(root->protection);
      cd->setIsStatic(root->stat);

      // file definition containing the class cd
      cd->setBodySegment(root->startBodyLine, root->endBodyLine);
      cd->setBodyDef(fd);

      // see if the class is found inside a namespace
      cd->insertUsedFile(fd);

      // add class to the list
      Doxy_Globals::classSDict.insert(fullName, cd);

      if (cd->isGeneric()) {
         // generics are also stored in a separate dictionary for fast lookup of instantions
         Doxy_Globals::genericsSDict.insert(fullName, cd);
      }
   }

   cd->addSectionsToDefinition(root->m_anchors);
   if (! root->subGrouping) {
      cd->setSubGrouping(false);
   }

   if (cd->hasDocumentation()) {
      addIncludeFileClass(cd, fd, root);
   }

   if (fd && (root->section & Entry::COMPOUND_MASK)) {
      cd->setFileDef(fd);
      fd->insertClass(cd);
   }

   addClassToGroups(root, cd);
   cd->setRefItems(root->m_specialLists);
}

// build a list of all classes mentioned in the documentation
// and all classes that have a documentation block before their definition
void Doxy_Work::buildClassList(QSharedPointer<Entry> ptrEntry)
{
   if (((ptrEntry->section & Entry::COMPOUND_MASK) || ptrEntry->section == Entry::OBJCIMPL_SEC) &&
                  ! ptrEntry->m_entryName.isEmpty()) {
      addClassToContext(ptrEntry);
   }

   RECURSE_ENTRYTREE(buildClassList, ptrEntry);
}

void Doxy_Work::buildClassDocList(QSharedPointer<Entry> ptrEntry)
{
   if ((ptrEntry->section & Entry::COMPOUNDDOC_MASK) && ! ptrEntry->m_entryName.isEmpty()) {
      addClassToContext(ptrEntry);
   }

   RECURSE_ENTRYTREE(buildClassDocList, ptrEntry);
}

void Doxy_Work::resolveClassNestingRelations()
{
   for (auto item : Doxy_Globals::classSDict) {
      item->visited = false;
   }

   bool done = false;
   int iteration = 0;

   while (! done) {
      done = true;
      ++iteration;

      for (auto cd : Doxy_Globals::classSDict) {

         if (! cd->visited) {
            QString name = stripAnonymousNamespaceScope(cd->name());

            // add class to the correct structural context
            QSharedPointer<Definition> def = findScopeFromQualifiedName(Doxy_Globals::globalScope,
                  name, cd->getFileDef(), TagInfo());

            if (def) {
               def->addInnerCompound(cd);
               cd->setOuterScope(def);

               // for inline namespace add cd to the outer scope
               while (def->definitionType() == Definition::TypeNamespace) {
                  QSharedPointer<NamespaceDef> nd = def.dynamicCast<NamespaceDef>();

                  if (nd->isInlineNS()) {
                     def = def->getOuterScope();

                     if (def) {
                        // emerald, may need to set a flag to mark this inlineNS

                        QSharedPointer<ClassDef> aliasCd = cd;

                        def->addInnerCompound(aliasCd);
                        QString inLineNS_FullName = def->qualifiedName() + "::" + aliasCd->localName();

                        Doxy_Globals::classSDict.insert(inLineNS_FullName, aliasCd);
                        aliasCd->visited = true;
                     }

                  } else {
                     break;
                  }
               }

               cd->visited = true;
               done = false;
            }
         }
      }
   }

   // give warnings for unresolved compounds
   for (auto cd : Doxy_Globals::classSDict) {

      if (! cd->visited) {
         QString name = stripAnonymousNamespaceScope(cd->name());

         // create the scope artificially so we can at least relate scopes properly

         QSharedPointer<Definition> d = buildScopeFromQualifiedName(name, name.count("::"), cd->getLanguage(), TagInfo());

         if (d != cd && ! cd->getDefFileName().isEmpty())  {
            // avoid recursion in case of redundant scopes, i.e: namespace N { class N::C {}; }
            // for this case DoxyPress assumes the existence of a namespace N::N in which C is to be found
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

   if (fullName.endsWith("::")) {
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

   Doxy_Globals::classSDict.insert(fullName, QSharedPointer<ClassDef> (cd) );

   QSharedPointer<MemberList> ml = templ->getMemberList(MemberListType_pubAttribs);

   if (ml) {

      for (auto md : *ml) {

         QSharedPointer<MemberDef> imd = QMakeShared<MemberDef>(md->getDefFileName(), md->getDefLine(),
                   md->getDefColumn(), md->typeString(), md->name(), md->argsString(), md->excpString(),
                   md->protection(), md->virtualness(), md->isStatic(), Member, md->memberType(), ArgumentList(), ArgumentList());

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
   if (cd->getClassSDict().count() > 0) {
      QSharedPointer<MemberList> ml = cd->getMemberList(MemberListType_pubAttribs);

      if (ml) {

         for (auto md : *ml) {
            // return type
            QString type = md->typeString();

            if (type.contains("::@")) {
               // member of tag less struct/union

               for (auto icd : cd->getClassSDict()) {

                  if (type.contains(icd->name())) {
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

   for (auto icd : cd->getClassSDict()) {
      if (! icd->name().contains("@")) {
         // process all non-anonymous inner classes
         findTagLessClasses(icd);
      }
   }

   processTagLessClasses(cd, cd, cd, "", 0); // process tag less inner struct/classes (if any)
}

void Doxy_Work::findTagLessClasses()
{
   for (auto cd : Doxy_Globals::classSDict) {
      QSharedPointer<Definition> scope = cd->getOuterScope();

      if (scope && scope->definitionType() != Definition::TypeClass) {
         // that is not nested
         findTagLessClasses(cd);
      }
   }
}

// build a list of all namespaces mentioned in the documentation
// and all namespaces which have a documentation block before their definition

void Doxy_Work::buildNamespaceList(QSharedPointer<Entry> ptrEntry)
{
   if ( (ptrEntry->section == Entry::NAMESPACE_SEC || ptrEntry->section == Entry::NAMESPACEDOC_SEC ||
            ptrEntry->section == Entry::PACKAGEDOC_SEC) && ! ptrEntry->m_entryName.isEmpty()) {

      QSharedPointer<Entry> root = ptrEntry;

      QString fullName = root->m_entryName;

      if (root->section == Entry::PACKAGEDOC_SEC) {
         fullName = substitute(fullName, ".", "::");
      }

      fullName = stripAnonymousNamespaceScope(fullName);

      // change the namespace to match the ns alias
      auto iter = Doxy_Globals::nsRenameAlias.find(fullName);

      if (iter != Doxy_Globals::nsRenameAlias.end()) {
         fullName = iter.value();
      }

      if (! fullName.isEmpty()) {
         QSharedPointer<NamespaceDef> nd;

         if ((nd = Doxy_Globals::namespaceSDict.find(fullName))) {
            // existing namespace
            nd->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);

            // change name to match docs
            nd->setName(fullName);

            nd->addSectionsToDefinition(root->m_anchors);

            nd->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
            if (nd->getLanguage() == SrcLangExt_Unknown) {
               nd->setLanguage(root->m_srcLang);
            }

            if (ptrEntry->m_tagInfo.isEmpty())  {
              // if we found the namespace in a tag file and also in a project file, then remove the tag file reference

               nd->setReference("");
               nd->setFileName(fullName);
             }

            // file definition containing the namespace nd
            QSharedPointer<FileDef> fd = ptrEntry->fileDef();

            // insert the namespace in the file definition
            if (fd) {
               fd->insertNamespace(nd);
            }

            addNamespaceToGroups(root, nd);
            nd->setRefItems(root->m_specialLists);

         } else {
            // new namespace

            QString tagName;
            QString tagFileName;

            const TagInfo &tagInfo = ptrEntry->m_tagInfo;

            if (! tagInfo.isEmpty()) {
               tagName     = tagInfo.tag_Name;
               tagFileName = tagInfo.tag_FileName;
            }

            bool isPublished = root->m_traits.hasTrait(Entry::Virtue::Published);

            nd = QMakeShared<NamespaceDef>(! tagInfo.isEmpty() ? tagName : root->getData(EntryKey::File_Name),
                  root->startLine, root->startColumn, fullName, tagName, tagFileName,
                  root->getData(EntryKey::Member_Type), isPublished);

            nd->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
            nd->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
            nd->addSectionsToDefinition(root->m_anchors);
            nd->setHidden(root->hidden);
            nd->setArtificial(root->artificial);
            nd->setLanguage(root->m_srcLang);
            nd->setId(root->getData(EntryKey::Clang_Id));

            nd->setInlineNS(root->m_traits.hasTrait(Entry::Virtue::Inline));

            addNamespaceToGroups(root, nd);
            nd->setRefItems(root->m_specialLists);

            // file definition containing the namespace nd
            QSharedPointer<FileDef> fd = ptrEntry->fileDef();

            // insert the namespace in the file definition
            if (fd) {
               fd->insertNamespace(nd);
            }

            // the empty string test is needed for extract all case
            nd->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
            nd->insertUsedFile(fd);
            nd->setBodySegment(root->startBodyLine, root->endBodyLine);
            nd->setBodyDef(fd);

            // add class to the list
            Doxy_Globals::namespaceSDict.insert(fullName, nd);

            // also add namespace to the correct structural context
            QSharedPointer<Definition> def = findScopeFromQualifiedName(Doxy_Globals::globalScope, fullName,
                  QSharedPointer<FileDef>(), tagInfo);

            if (def == nullptr)  {
               // did not find anything, create the scope artificially
               // so we can at least relate scopes properly

               def = buildScopeFromQualifiedName(fullName, fullName.count("::"), nd->getLanguage(), tagInfo);

               def->addInnerCompound(nd);
               nd->setOuterScope(def);

               // TODO: Due to the order in which the tag file is written
               // a nested class can be found before its parent

            } else {
               def->addInnerCompound(nd);
               nd->setOuterScope(def);

               while (def->definitionType() == Definition::TypeNamespace) {
                  QSharedPointer<NamespaceDef> pnd = def.dynamicCast<NamespaceDef>();

                  // for inline namespace add nd to the outer scope
                  if (pnd->isInlineNS()) {
                     def = def->getOuterScope();

                     if (def) {
                        // emerald, may need to set a flag to mark this inlineNS

                        QSharedPointer<NamespaceDef> aliasNd = nd;
                        def->addInnerCompound(aliasNd);
                     }

                  } else {
                     break;
                  }
               }
            }
         }
      }
   }

   RECURSE_ENTRYTREE(buildNamespaceList, ptrEntry);
}

QSharedPointer<NamespaceDef> Doxy_Work::findUsedNamespace(const NamespaceSDict *unl, const QString &name)
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

void Doxy_Work::findUsingDirectives(QSharedPointer<Entry> ptrEntry)
{
   if (ptrEntry->section == Entry::USINGDIR_SEC) {
      QSharedPointer<Entry> root = ptrEntry->entry();

      QString name = substitute(root->m_entryName, ".", "::");

      if (name.endsWith("::")) {
         name.chop(2);
      }

      if (! name.isEmpty()) {
         QSharedPointer<NamespaceDef> usingNd;
         QSharedPointer<NamespaceDef> nd;
         QSharedPointer<FileDef> fd = ptrEntry->fileDef();

         QString nsName;

         // see if the using statement was found inside a namespace or inside the global file scope
         if (ptrEntry->parent() && ptrEntry->parent()->section == Entry::NAMESPACE_SEC &&
               (fd == 0 || fd->getLanguage() != SrcLangExt_Java) ) {

            // not a .java file
            nsName = stripAnonymousNamespaceScope(ptrEntry->parent()->m_entryName);

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

            QSharedPointer<NamespaceDef> nd = QMakeShared<NamespaceDef>(root->getData(EntryKey::File_Name),
                     root->startLine, root->startColumn, name);

            nd->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
            nd->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
            nd->addSectionsToDefinition(root->m_anchors);

            nd->setHidden(root->hidden);
            nd->setArtificial(true);
            nd->setLanguage(root->m_srcLang);
            nd->setId(root->getData(EntryKey::Clang_Id));

            nd->setInlineNS(root->m_traits.hasTrait(Entry::Virtue::Inline));

            for (auto &g : root->m_groups) {
               QSharedPointer<GroupDef> gd;

               if (! g.groupname.isEmpty() && (gd = Doxy_Globals::groupSDict.find(g.groupname))) {
                  gd->addNamespace(nd);
               }
            }

            // insert the namespace in the file definition
            if (fd) {
               fd->insertNamespace(nd);
               fd->addUsingDirective(nd);
            }

            // the empty string test is needed for extract all case
            nd->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
            nd->insertUsedFile(fd);

            // add class to the list
            Doxy_Globals::namespaceSDict.insert(name, nd);
            nd->setRefItems(root->m_specialLists);
         }
      }
   }

   RECURSE_ENTRYTREE(findUsingDirectives, ptrEntry);
}

void Doxy_Work::buildListOfUsingDecls(QSharedPointer<Entry> ptrEntry)
{
   if (ptrEntry->section == Entry::USINGDECL_SEC && ! (ptrEntry->parent()->section & Entry::COMPOUND_MASK)  ) {
      // not a class/struct member

      QSharedPointer<Entry> root = ptrEntry->entry();

      QString name = substitute(root->m_entryName, ".", "::");

      if (! Doxy_Globals::g_usingDeclarations.contains(name)) {
         QSharedPointer<FileDef> fd = ptrEntry->fileDef();

         if (fd) {
            Doxy_Globals::g_usingDeclarations.insert(name, *fd);
         }
      }
   }

   RECURSE_ENTRYTREE(buildListOfUsingDecls, ptrEntry);
}

void Doxy_Work::findUsingDeclarations(QSharedPointer<Entry> ptrEntry)
{
   if (ptrEntry->section == Entry::USINGDECL_SEC && !(ptrEntry->parent()->section & Entry::COMPOUND_MASK) ) {
      // not a class/struct member

      QSharedPointer<Entry> root = ptrEntry->entry();

      if (! root->m_entryName.isEmpty()) {
         QSharedPointer<ClassDef> usingCd;
         QSharedPointer<NamespaceDef> nd;

         QSharedPointer<FileDef> fd = ptrEntry->fileDef();
         QString scName;

         // see if the using statement was found inside a namespace or inside
         // the global file scope.

         if (ptrEntry->parent()->section == Entry::NAMESPACE_SEC) {
            scName = ptrEntry->parent()->m_entryName;

            if (!scName.isEmpty()) {
               nd = getResolvedNamespace(scName);
            }
         }

         // Assume the using statement was used to import a class.
         // Find the scope in which the `using' namespace is defined by prepending
         // the possible scopes in which the using statement was found, starting
         // with the most inner scope and going to the most outer scope (i.e. file scope).

         QString name = substitute(root->m_entryName, ".", "::"); //Java/C# scope->internal
         usingCd = getResolvedClass(nd, fd, name);

         if (usingCd == 0) {
            usingCd = Doxy_Globals::hiddenClasses.find(name);
         }

         if (usingCd == 0) {
            // definition not in the input => add an artificial class
            usingCd = QMakeShared<ClassDef>("<using>", 1, 1,name, CompoundType::Class);

            Doxy_Globals::hiddenClasses.insert(root->m_entryName, usingCd);
            usingCd->setArtificial(true);

            usingCd->setLanguage(root->m_srcLang);

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
   }

   RECURSE_ENTRYTREE(findUsingDeclarations, ptrEntry);
}

void Doxy_Work::findUsingDeclImports(QSharedPointer<Entry> ptrEntry)
{
   if (ptrEntry->section == Entry::USINGDECL_SEC && (ptrEntry->parent()->section & Entry::COMPOUND_MASK) )  {
      // in a class/struct

      QString fullName = removeRedundantWhiteSpace(ptrEntry->parent()->m_entryName);
      fullName = stripAnonymousNamespaceScope(fullName);
      fullName = stripTemplateSpecifiersFromScope(fullName);

      QSharedPointer<ClassDef> cd = getClass(fullName);

      if (cd) {
         int i = ptrEntry->m_entryName.indexOf("::");

         if (i != -1) {
            QString scope   = ptrEntry->m_entryName.left(i);
            QString memName = ptrEntry->m_entryName.right(ptrEntry->m_entryName.length() - i - 2);
            QSharedPointer<ClassDef> bcd = getResolvedClass(cd, QSharedPointer<FileDef>(), scope); // todo: file in fileScope parameter

            if (bcd && bcd != cd) {
               const MemberNameInfoSDict &mndict  = bcd->memberNameInfoSDict();
               QSharedPointer<MemberNameInfo> mni = mndict.find(memName);

               if (mni) {

                  for (auto &mi : *mni) {
                     QSharedPointer<MemberDef> md = mi.memberDef;

                     if (md && md->protection() != Private) {

                        QSharedPointer<Entry>root = ptrEntry->entry();
                        QSharedPointer<MemberDef> newMd;

                        {
                           QString fileName = root->getData(EntryKey::File_Name);

                           if (fileName.isEmpty() && ! ptrEntry->m_tagInfo.isEmpty()) {
                              fileName = ptrEntry->m_tagInfo.tag_Name;
                           }

                           const ArgumentList &templateArgList = md->getTemplateArgumentList();
                           const ArgumentList &arglist = md->getArgumentList();

                           newMd = QMakeShared<MemberDef>(fileName, root->startLine, root->startColumn, md->typeString(),
                              memName, md->argsString(), md->excpString(), root->protection, root->virt, md->isStatic(),
                              Member, md->memberType(), templateArgList, arglist);
                        }

                        newMd->setMemberClass(cd);
                        cd->insertMember(newMd);

                        if (! root->getData(EntryKey::Main_Docs).isEmpty() || ! root->getData(EntryKey::Brief_Docs).isEmpty()) {
                           newMd->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
                           newMd->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
                           newMd->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs),
                                    root->getData(EntryKey::Inbody_File), root->inbodyLine);

                        } else {
                           newMd->setDocumentation(md->documentation(), md->docFile(), md->docLine());
                           newMd->setBriefDescription(md->briefDescription(), md->briefFile(), md->briefLine());
                           newMd->setInbodyDocumentation(md->inbodyDocumentation(), md->inbodyFile(), md->inbodyLine());
                        }

                        newMd->setUsingDeclaration(true);
                        newMd->setDefinition(md->definition());
                        newMd->enableCallGraph(root->callGraph);
                        newMd->enableCallerGraph(root->callerGraph);
                        newMd->setBitfields(md->bitfieldString());
                        newMd->addSectionsToDefinition(root->m_anchors);
                        newMd->setBodySegment(md->getStartBodyLine(), md->getEndBodyLine());
                        newMd->setBodyDef(md->getBodyDef());
                        newMd->setInitializer(md->initializer());
                        newMd->setMaxInitLines(md->initializerLines());
                        newMd->setMemberGroupId(root->mGrpId);
                        newMd->setMemberTraits(md->getMemberTraits());
                        newMd->setLanguage(root->m_srcLang);
                        newMd->setId(root->getData(EntryKey::Clang_Id));
                     }
                  }
               }
            }
         }
      }

   }
   RECURSE_ENTRYTREE(findUsingDeclImports, ptrEntry);
}

void Doxy_Work::findIncludedUsingDirectives()
{
   // first mark all files as not visited
   for (auto &fn : Doxy_Globals::inputNameList) {

      for (auto fd : *fn) {
         fd->visited = false;
      }
   }

   // recursively add using directives found in #include files
   // to files which have not been visited
   for (auto &fn : Doxy_Globals::inputNameList) {

      for (auto fd : *fn) {
         if (! fd->visited) {
            fd->addIncludedUsingDirectives();
         }
      }
   }
}

QSharedPointer<MemberDef> Doxy_Work::addVariableToClass(QSharedPointer<Entry> ptrEntry, QSharedPointer<ClassDef> cd,
         MemberDefType memberType, const QString &name, bool fromAnnScope, QSharedPointer<MemberDef> fromAnnMemb,
         Protection prot, Relationship related)
{
   static const bool hideScopeNames = Config::getBool("hide-scope-names");

   QString qualScope = cd->qualifiedNameWithTemplateParameters();
   QString scopeSeparator = "::";

   SrcLangExt lang = cd->getLanguage();

   if (lang == SrcLangExt_Java || lang == SrcLangExt_CSharp) {
      qualScope = substitute(qualScope, "::", ".");
      scopeSeparator = ".";
   }

   Debug::print(Debug::Variables, 0, "  class variable:\n"
                "    `%s' `%s'::`%s' `%s' protection=`%d  ann=%d  initial value=`%s'\n",
                csPrintable(ptrEntry->getData(EntryKey::Member_Type)), csPrintable(qualScope), csPrintable(name),
                csPrintable(ptrEntry->getData(EntryKey::Member_Args)), ptrEntry->protection, fromAnnScope,
                csPrintable(ptrEntry->getData(EntryKey::Initial_Value)) );

   QString def;

   if (! ptrEntry->getData(EntryKey::Member_Type).isEmpty()) {

      bool isAlias = ptrEntry->m_traits.hasTrait(Entry::Virtue::Alias);

      if (related || hideScopeNames) {

         if (isAlias) {
            // turn 'typedef B A' into 'using A = B'
            def = "using " + name + " = " + ptrEntry->getData(EntryKey::Member_Type).mid(7);

         } else {
            def = ptrEntry->getData(EntryKey::Member_Type) + " " + name + ptrEntry->getData(EntryKey::Member_Args);

         }

      } else {
         if (isAlias) {
            // turn 'typedef B C::A' into 'using C::A = B'
            def = "using " + qualScope + scopeSeparator + name + " = " + ptrEntry->getData(EntryKey::Member_Type).mid(7);

         } else {
            def = ptrEntry->getData(EntryKey::Member_Type) + " " + qualScope +
                     scopeSeparator + name + ptrEntry->getData(EntryKey::Member_Args);

         }
      }

   } else if (hideScopeNames) {
      def = name + ptrEntry->getData(EntryKey::Member_Args);

   } else {
      def = qualScope + scopeSeparator + name + ptrEntry->getData(EntryKey::Member_Args);

   }

   def = stripPrefix(def, "static ");

   // see if the member is already found in the same scope
   // (this may be the case for a static member that is initialized outside the class)
   QSharedPointer<MemberName> mn = Doxy_Globals::memberNameSDict.find(name);

   if (mn) {

      for (auto md : *mn) {

         if (md->getClassDef() == cd && removeRedundantWhiteSpace(ptrEntry->getData(EntryKey::Member_Type)) == md->typeString()) {
            // member already in the scope

            if (ptrEntry->m_srcLang == SrcLangExt_ObjC && ptrEntry->mtype == MethodType::Property &&
                     md->memberType() == MemberDefType::Variable) {

               // Objective-C 2.0 property, turn variable into a property
               md->setProtection(ptrEntry->protection);
               cd->reclassifyMember(md, MemberDefType::Property);

            } else if (ptrEntry->mtype == MethodType::Property) {
               // set correct property

               if (ptrEntry->m_traits.hasTrait(Entry::Virtue::Readable))  {
                  md->setPropertyRead(ptrEntry->getData(EntryKey::Read_Property));
               }

               if (ptrEntry->m_traits.hasTrait(Entry::Virtue::Writable))  {
                  md->setPropertyWrite(ptrEntry->getData(EntryKey::Write_Property));
               }

               if (ptrEntry->m_traits.hasTrait(Entry::Virtue::Reset))  {
                  md->setPropertyReset(ptrEntry->getData(EntryKey::Reset_Property));
               }

               if (ptrEntry->m_traits.hasTrait(Entry::Virtue::Notify))  {
                  md->setPropertyNotify(ptrEntry->getData(EntryKey::Notify_Property));
               }
            }

            ArgumentList tmp;
            addMemberDocs(ptrEntry, md, def, tmp, false);

            return md;
         }
      }
   }

   QString fileName = ptrEntry->getData(EntryKey::File_Name);

   if (fileName.isEmpty() && ! ptrEntry->m_tagInfo.isEmpty() ) {
      fileName = ptrEntry->m_tagInfo.tag_Name;
   }

   ArgumentList tmpList;

   if (! ptrEntry->m_templateArgLists.isEmpty()) {
      tmpList = ptrEntry->m_templateArgLists.last();
   }

   // new member variable, typedef, enum value, or property
   QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(fileName, ptrEntry->startLine, ptrEntry->startColumn,
            ptrEntry->getData(EntryKey::Member_Type), name, ptrEntry->getData(EntryKey::Member_Args),
            ptrEntry->getData(EntryKey::Exception_Spec), prot, Normal, ptrEntry->stat, related, memberType,
            tmpList, ArgumentList() );

   md->setTagInfo(ptrEntry->m_tagInfo);
   md->setMemberClass(cd);                         // also sets outer scope (i.e. getOuterScope())

   // md->setDefFile(ptrEntry->fileName);
   // md->setDefLine(ptrEntry->startLine);

   md->setDocumentation(ptrEntry->getData(EntryKey::Main_Docs), ptrEntry->getData(EntryKey::MainDocs_File), ptrEntry->docLine);
   md->setBriefDescription(ptrEntry->getData(EntryKey::Brief_Docs), ptrEntry->getData(EntryKey::Brief_File), ptrEntry->briefLine);
   md->setInbodyDocumentation(ptrEntry->getData(EntryKey::Inbody_Docs), ptrEntry->getData(EntryKey::Inbody_File), ptrEntry->inbodyLine);
   md->setDefinition(def);
   md->setBitfields(ptrEntry->getData(EntryKey::Member_Bitfields));
   md->addSectionsToDefinition(ptrEntry->m_anchors);
   md->setFromAnonymousScope(fromAnnScope);
   md->setFromAnonymousMember(fromAnnMemb);

   // md->setIndentDepth(indentDepth);
   md->setBodySegment(ptrEntry->startBodyLine, ptrEntry->endBodyLine);
   md->setInitializer(ptrEntry->getData(EntryKey::Initial_Value));
   md->setMaxInitLines(ptrEntry->initLines);
   md->setMemberGroupId(ptrEntry->mGrpId);
   md->setMemberTraits(ptrEntry->m_traits);

   if (ptrEntry->mtype == MethodType::Property) {
      // set correct property

      if (ptrEntry->m_traits.hasTrait(Entry::Virtue::Readable))  {
         md->setPropertyRead(ptrEntry->getData(EntryKey::Read_Property));
      }

      if (ptrEntry->m_traits.hasTrait(Entry::Virtue::Writable))  {
         md->setPropertyWrite(ptrEntry->getData(EntryKey::Write_Property));
      }

      if (ptrEntry->m_traits.hasTrait(Entry::Virtue::Reset))  {
         md->setPropertyReset(ptrEntry->getData(EntryKey::Reset_Property));
      }

      if (ptrEntry->m_traits.hasTrait(Entry::Virtue::Notify))  {
         md->setPropertyNotify(ptrEntry->getData(EntryKey::Notify_Property));
      }
   }

   md->enableCallGraph(ptrEntry->callGraph);
   md->enableCallerGraph(ptrEntry->callerGraph);
   md->setHidden(ptrEntry->hidden);
   md->setArtificial(ptrEntry->artificial);
   md->setLanguage(ptrEntry->m_srcLang);
   md->setId(ptrEntry->getData(EntryKey::Clang_Id));
   addMemberToGroups(ptrEntry, md);

   md->setBodyDef(ptrEntry->fileDef());

   // add the member to the global list
   if (mn) {
      mn->append(md);

   } else {
      // new variable name, add the member to the class
      mn = QMakeShared<MemberName>(name);
      mn->append(md);

      Doxy_Globals::memberNameSDict.insert(name, mn);
   }

   cd->insertMember(md);
   md->setRefItems(ptrEntry->m_specialLists);

   // TODO: insert FileDef instead of filename strings
   cd->insertUsedFile(ptrEntry->fileDef());

   ptrEntry->setSection(Entry::EMPTY_SEC);

   return md;
}

QSharedPointer<MemberDef> Doxy_Work::addVariableToFile(QSharedPointer<Entry> ptrEntry, MemberDefType memberType,
         const QString &scope, const QString &name, bool fromAnnScope, QSharedPointer<MemberDef> fromAnnMemb)
{
   static const bool hideScopeNames = Config::getBool("hide-scope-names");
   static const bool useTypedefName = Config::getBool("use-typedef-name");

   QSharedPointer<Entry> root = ptrEntry->entry();

   Debug::print(Debug::Variables, 0, "  global variable:\n"
                "    type=`%s' scope=`%s' name=`%s' args=`%s' prot=`%d  memberType = %d  lang = %d\n",
                csPrintable(root->getData(EntryKey::Member_Type)), csPrintable(scope), csPrintable(name),
                csPrintable(root->getData(EntryKey::Member_Args)), root->protection, memberType, root->m_srcLang);

   QSharedPointer<FileDef> fd = ptrEntry->fileDef();

   // see if we have a typedef that should hide a struct or union
   if (memberType == MemberDefType::Typedef && useTypedefName) {
      QString type = root->getData(EntryKey::Member_Type);

      type = stripPrefix(type, "typedef ");

      if (type.startsWith("struct ") || type.startsWith("union ")) {
         type = stripPrefix(type, "struct ");
         type = stripPrefix(type, "union ");

         static QRegularExpression regExp("[a-z_A-Z][a-z_A-Z0-9]*");
         QRegularExpressionMatch match = regExp.match(type);

         if (match.hasMatch()) {
            QString typeValue = match.captured();

            QSharedPointer<ClassDef> cd = getClass(typeValue);

            if (cd) {
               // this typedef should hide compound name cd, so we
               // change the name that is displayed from cd
               cd->setClassName(name);
               cd->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
               cd->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);

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
   if (nd && ! nd->name().isEmpty() && nd->name().at(0) != '@' && ! hideScopeNames)   {
      // variable is inside a namespace, so put the scope before the name

      SrcLangExt lang = nd->getLanguage();
      QString sep  = getLanguageSpecificSeparator(lang);

      if (! root->getData(EntryKey::Member_Type).isEmpty()) {

         bool isAlias = root->m_traits.hasTrait(Entry::Virtue::Alias);

         if (isAlias) {
            // turn 'typedef B NS::A' into 'using NS::A = B'
            def = "using " + nd->name() + sep + name + " = " + root->getData(EntryKey::Member_Type);

         } else {
            // normal member
            def = root->getData(EntryKey::Member_Type) + " " + nd->name() + sep + name + root->getData(EntryKey::Member_Args);
         }

      } else {
         def = nd->name() + sep + name + root->getData(EntryKey::Member_Args);
      }

   } else {

      if (! root->getData(EntryKey::Member_Type).isEmpty() && ! root->m_entryName.isEmpty()) {

         if (name.startsWith('@')) {
            // dummy variable representing anonymous union
            def = root->getData(EntryKey::Member_Type);

         } else {
            bool isAlias = root->m_traits.hasTrait(Entry::Virtue::Alias);

            if (isAlias) {
               // turn 'typedef B A' into 'using A = B'
               def = "using " + root->m_entryName + " = " + root->getData(EntryKey::Member_Type).mid(7);

            } else {
               // normal member
               def = root->getData(EntryKey::Member_Type) + " " + name + root->getData(EntryKey::Member_Args);
            }
         }

      } else {
         def = name + root->getData(EntryKey::Member_Args);
      }
   }

   def = stripPrefix(def, "static ");

   QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict.find(name);

   if (mn) {
      if (! scope.isEmpty()) {
         nd = getResolvedNamespace(scope);
      }

      for (auto md : *mn) {
         if ( ((nd == nullptr && md->getNamespaceDef() == 0 && md->getFileDef() &&
                  root->getData(EntryKey::File_Name) == md->getFileDef()->getFilePath()) ||
                 (nd != nullptr && md->getNamespaceDef() == nd) ) && !md->isDefine() && ! md->isEnumerate()  ) {

            // in C# an enum value and enum can have the same name
            // function style #define's can be "overloaded" by typedefs or variables
            // both variable names in the same file variable already in the scope

            bool isPHPArray = false;
            QString tmpArgs = root->getData(EntryKey::Member_Args);

            if ((md->getLanguage() == SrcLangExt_PHP) && (md->argsString() != tmpArgs) &&  (tmpArgs.indexOf('[') != -1)) {
               isPHPArray = true;
            }

            bool staticsInDifferentFiles = root->stat && md->isStatic() && (root->getData(EntryKey::File_Name) != md->getDefFileName());

            if (md->getFileDef() && ! isPHPArray && ! staticsInDifferentFiles) {
               // not a php array
               // not a php array variable

               ArgumentList tmp;
               addMemberDocs(ptrEntry, md, def, tmp, false);
               md->setRefItems(root->m_specialLists);

               return md;
            }
         }
      }
   }

   QString fileName = root->getData(EntryKey::File_Name);

   if (fileName.isEmpty() && ! ptrEntry->m_tagInfo.isEmpty()) {
      fileName = ptrEntry->m_tagInfo.tag_Name;
   }

   ArgumentList tmpList;

   if (! root->m_templateArgLists.isEmpty()) {
      tmpList = root->m_templateArgLists.last();
   }

   // new global variable, enum value, or typedef
   QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(fileName, root->startLine, root->startColumn, root->getData(EntryKey::Member_Type),
                  name, root->getData(EntryKey::Member_Args), nullptr, root->protection, Normal, root->stat, Relationship::Member,
                  memberType, tmpList, ArgumentList());

   md->setTagInfo(ptrEntry->m_tagInfo);
   md->setMemberTraits(root->m_traits);
   md->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
   md->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
   md->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);
   md->addSectionsToDefinition(root->m_anchors);
   md->setFromAnonymousScope(fromAnnScope);
   md->setFromAnonymousMember(fromAnnMemb);
   md->setInitializer(root->getData(EntryKey::Initial_Value));
   md->setMaxInitLines(root->initLines);
   md->setMemberGroupId(root->mGrpId);
   md->setDefinition(def);
   md->setLanguage(root->m_srcLang);
   md->setId(root->getData(EntryKey::Clang_Id));
   md->enableCallGraph(root->callGraph);
   md->enableCallerGraph(root->callerGraph);
   md->setExplicitExternal(root->explicitExternal);

   if (! root->explicitExternal) {
      md->setBodySegment(root->startBodyLine, root->endBodyLine);
      md->setBodyDef(fd);
   }
   addMemberToGroups(root, md);

   md->setRefItems(root->m_specialLists);
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
      Doxy_Globals::functionNameSDict.insert(name, mn);
   }

   ptrEntry->setSection(Entry::EMPTY_SEC);

   return md;
}

/*! See if the return type string type is that of a function pointer
 *  \returns -1 if this is not a function pointer variable or
 *           the index at which the closing brace of (...*name) was found.
 */
int Doxy_Work::findFunctionPtr(const QString &type, int lang, int *pLength)
{
   if (lang == SrcLangExt_Fortran) {
      return -1;   // Fortran does not have function pointers
   }

   static const QRegularExpression regExp("\\([^)]*[\\*\\^][^)]*\\)");
   QRegularExpressionMatch match = regExp.match(type);


   if (match.hasMatch() && ! type.contains("operator")) {

      if (type.contains(")(") &&  !type.contains("typedef ")) {
         return -1;
      }

      auto iter = match.capturedStart();
      auto bb  = type.indexOfFast('<');
      auto be  = type.lastIndexOfFast('>');

      if (bb < iter && iter < be) {
         return -1;
      }

      if (pLength) {
         *pLength = match.capturedLength();
      }

      return match.capturedStart() - type.constBegin();

   } else {
      return -1;
   }
}

/*! Returns true if \a type is a class within scope \a context.
 *  Used to detect variable declarations that look like function prototypes.
 */
bool Doxy_Work::isVarWithConstructor(QSharedPointer<Entry> ptrEntry)
{
   static QRegularExpression regExp_init("[0-9\"'&*!^]+");
   static QRegularExpression regExp_id("[a-z_A-Z][a-z_A-Z0-9]*");

   bool result = false;
   bool typeIsClass;

   QString type;

   QSharedPointer<Definition> ctx;
   QSharedPointer<FileDef>    fd;
   QSharedPointer<Entry>      root = ptrEntry->entry();

   int ti;
   bool outerBreak = false;

   do {

      if (ptrEntry->parent()->section & Entry::COMPOUND_MASK) {
         // inside a class
         result = false;
         break;

      } else if ((fd = ptrEntry->fileDef()) && (fd->name().endsWith(".c") || fd->name().endsWith(".h")) ) {
         // inside a .c file
         result = false;
         break;
      }

      if (root->getData(EntryKey::Member_Type).isEmpty()) {
         result = false;
          break;
      }

      if (! ptrEntry->parent()->m_entryName.isEmpty()) {
         ctx = Doxy_Globals::namespaceSDict.find(ptrEntry->parent()->m_entryName);
      }

      type = root->getData(EntryKey::Member_Type);

      // remove qualifiers
      findAndRemoveWord(type, "const");
      findAndRemoveWord(type, "static");
      findAndRemoveWord(type, "volatile");

      typeIsClass = getResolvedClass(ctx, fd, type) != 0;

      if (! typeIsClass && (ti = type.indexOf('<')) != -1) {
         typeIsClass = getResolvedClass(ctx, fd, type.left(ti)) != 0;
      }

      if (typeIsClass) {
         // need to check if the arguments are types or values.
         // since we do not have complete type info, need on heuristics

         const ArgumentList &argList = root->argList;

         if (argList.listEmpty()) {
            result = false;
            break;
         }

         for (auto &a : argList) {

            if (! a.name.isEmpty() || ! a.defval.isEmpty()) {

               if (a.name.indexOf(regExp_init) == 0) {
                  result = true;

               } else {
                  // arg has (type,name) pair -> function prototype
                  result = false;

               }

               outerBreak = true;
               break;
            }

            if (a.type.isEmpty() || getResolvedClass(ctx, fd, a.type) != 0) {
               // arg type is a known type
               result = false;

               outerBreak = true;
               break;
            }

            if (checkIfTypedef(ctx, fd, a.type)) {
               // argument is a typedef
               result = false;

               outerBreak = true;
               break;
            }

            if (a.type.endsWith('*') || a.type.endsWith('&')) {
               // type ends with * or & => pointer or reference
               result = false;

               outerBreak = true;
               break;
            }

            if (a.type.indexOf(regExp_init) == 0) {
               // argument type starts with typical initializer char
               result = true;

               outerBreak = true;
               break;
            }

            QString resType = resolveTypeDef(ctx, a.type);

            if (resType.isEmpty()) {
               resType = a.type;
            }

            if (resType.indexOf(regExp_id) == 0) {
               // resType starts with identifier

               QRegularExpressionMatch match = regExp_id.match(resType);
               resType = resType.left(match.capturedLength());

               if (resType == "int"    || resType == "long"     || resType == "float" ||
                   resType == "double" || resType == "char"     || resType == "signed" ||
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

   return result;
}

void Doxy_Work::addVariable(QSharedPointer<Entry> ptrEntry, int isFuncPtr)
{
   QSharedPointer<Entry> root = ptrEntry->entry();

   Debug::print(Debug::Variables, 0, "VARIABLE_SEC: \n"
                "  type=`%s' name=`%s' args=`%s' bodyLine=`%d' mGrpId=%d relates=%s\n",
                csPrintable(root->getData(EntryKey::Member_Type)), csPrintable(root->m_entryName),
                csPrintable(root->getData(EntryKey::Member_Args)), root->startBodyLine, root->mGrpId,
                csPrintable(root->getData(EntryKey::Related_Class)) );

   if (root->getData(EntryKey::Member_Type).isEmpty() && ! root->m_entryName.contains("operator") &&
                  (root->m_entryName.contains('*') || root->m_entryName.contains('&'))) {

      // recover from parse error caused by redundant braces
      // like in "int *(var[10]);", which is parsed as -->  type="" name="int *" args="(var[10])"

      root->getData(EntryKey::Member_Type) = root->m_entryName;

      //
      QString tmpArgs = root->getData(EntryKey::Member_Args);

      static const QRegularExpression regExp_reName("[a-z_A-Z][a-z_A-Z0-9]*");
      QRegularExpressionMatch match = regExp_reName.match(tmpArgs);

      if (match.hasMatch()) {
         root->m_entryName = match.captured();

         auto iter = tmpArgs.indexOfFast(')', match.capturedEnd());
         root->setData(EntryKey::Member_Args, QStringView(match.capturedEnd(), iter));
      }

   } else {
      int i = isFuncPtr;

      bool isAlias = root->m_traits.hasTrait(Entry::Virtue::Alias);

      if (i == -1 && ! isAlias) {
         i = findFunctionPtr(root->getData(EntryKey::Member_Type), root->m_srcLang);   // for typedefs isFuncPtr is not yet set
      }

      Debug::print(Debug::Variables, 0, "  functionPtr? %s\n", i != -1 ? "yes" : "no");

      if (i != -1) {
         // function pointer
         QString tmpType = root->getData(EntryKey::Member_Type);
         int ai = tmpType.indexOf('[', i);

         if (ai > i) {
            // function pointer array
            root->prependData(EntryKey::Member_Args,   tmpType.right(tmpType.length() - ai));
            root->setData(EntryKey::Member_Type,       tmpType.left(ai));

         } else if (tmpType.indexOf(')', i) != -1) {
            // function ptr, not variable like "int (*bla)[10]"

            root->setData(EntryKey::Member_Type,       tmpType.left(tmpType.length() - 1));
            root->prependData(EntryKey::Member_Args,   ") ");

         }
      }
   }

   QString scope;
   QString name = removeRedundantWhiteSpace(root->m_entryName);

   // find the scope of this variable
   QSharedPointer<Entry> p = ptrEntry->parent();

   while ((p->section & Entry::SCOPE_MASK)) {
      QString scopeName = p->m_entryName;

      if (! scopeName.isEmpty()) {
         scope.prepend(scopeName);
         break;
      }

      p = p->parent();
   }

   QSharedPointer<ClassDef> cd = QSharedPointer<ClassDef>();

   bool isRelated  = false;
   bool isMemberOf = false;

   QString type = root->getData(EntryKey::Member_Type).trimmed();

   MemberDefType memberType;

   if (type == "@") {
      memberType = MemberDefType::EnumValue;

   } else if (type.startsWith("typedef ")) {
      memberType = MemberDefType::Typedef;

   } else if (root->mtype == MethodType::Property) {
      memberType = MemberDefType::Property;

   } else if (root->mtype == MethodType::Event) {
      memberType = MemberDefType::Event;

   } else {
      memberType = MemberDefType::Variable;
   }

   bool isFriend = false;

   if (type.startsWith("friend ")) {
      isFriend = true;
   }

   QString classScope = stripAnonymousNamespaceScope(scope);
   classScope = stripTemplateSpecifiersFromScope(classScope, false);

   QString annScopePrefix = scope.left(scope.length() - classScope.length());

   if (root->m_entryName.lastIndexOf("::") != -1) {

      if (root->getData(EntryKey::Member_Type) == "friend class" || root->getData(EntryKey::Member_Type) == "friend struct" ||
                  root->getData(EntryKey::Member_Type) == "friend union") {
         cd = getClass(scope);

         if (cd) {
            // cd  is the class to add member to
            addVariableToClass(ptrEntry, cd, memberType, name, false, QSharedPointer<MemberDef>(), Protection::Public,
                     Relationship::Friend);
         }
      }

      // skip this member, because it is a static variable definition (always?),  which will be found in a class scope as well,
      // but then we know the correct protection level, so only then it will be updated in the correct list

      return;
   }

   if (! root->getData(EntryKey::Related_Class).isEmpty()) {
      // related variable

      isRelated = true;
      isMemberOf = (root->relatesType == MemberOf);

      if (getClass(root->getData(EntryKey::Related_Class)) == 0 && ! scope.isEmpty()) {
         scope = mergeScopes(scope, root->getData(EntryKey::Related_Class));
      } else {
         scope = root->getData(EntryKey::Related_Class);
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

         if (name.at(0) != '@') {

            if (! pScope.isEmpty() && (pcd = getClass(pScope))) {
               // pcd is the class to add the member to

               Relationship tRelation;

               if (isFriend) {
                  tRelation = Relationship::Friend;

               } else if (isMemberOf) {
                  tRelation = Relationship::Foreign;

               } else if (isRelated) {
                  tRelation = Relationship::Related;

               } else {
                  tRelation = Relationship::Member;

               }

               md = addVariableToClass(ptrEntry, pcd, memberType, name, true, QSharedPointer<MemberDef>(), root->protection, tRelation);

            } else {
               // anonymous scope inside namespace or file => put variable in the global scope

               if (memberType == MemberDefType::Variable) {
                  md = addVariableToFile(ptrEntry, memberType, pScope, name, true, QSharedPointer<MemberDef>());
                  md->setFriend(isFriend);
               }

            }
         }
      }

      // cd is the class to add the member to
      Relationship tRelation;

      if (isFriend) {
         tRelation = Relationship::Friend;

      } else if (isMemberOf) {
         tRelation = Relationship::Foreign;

      } else if (isRelated) {
         tRelation = Relationship::Related;

      } else {
         tRelation = Relationship::Member;

      }

      addVariableToClass(ptrEntry, cd,  memberType, name, false, md, root->protection, tRelation);

   } else if (! name.isEmpty()) {
      // global variable, there is no classDef at this point

      addVariableToFile(ptrEntry, memberType, scope, name, false, QSharedPointer<MemberDef>() );
   }
}

// searches the Entry tree for typedef documentation sections, if found
// they are stored in their class or in the global list
void Doxy_Work::buildTypedefList(QSharedPointer<Entry> ptrEntry)
{
   if (! ptrEntry->m_entryName.isEmpty() && ptrEntry->section == Entry::VARIABLE_SEC &&
                  ptrEntry->getData(EntryKey::Member_Type).contains("typedef ")) {

      // it is a typedef
      addVariable(ptrEntry);
   }

   for (auto item : ptrEntry->children() ) {
      if (item->section != Entry::ENUM_SEC) {
         buildTypedefList(item);
      }
   }
}

// Searches the Entry tree for Variable documentation sections.
// If found they are stored in their class or in the global list.
void Doxy_Work::buildVarList(QSharedPointer<Entry> ptrEntry)
{
   int isFuncPtr = -1;
   bool isVar    = false;

   if (! ptrEntry->m_entryName.isEmpty() ) {
      QString tmpType = ptrEntry->getData(EntryKey::Member_Type);

      if (tmpType.isEmpty() || ! Doxy_Globals::g_compoundKeywordDict.contains(tmpType)) {

         if (ptrEntry->section == Entry::VARIABLE_SEC) {
            isVar = true;

         } else if (ptrEntry->section == Entry::FUNCTION_SEC) {
            isFuncPtr = findFunctionPtr(tmpType, ptrEntry->m_srcLang);

            if (isFuncPtr != -1) {
               // might be a function pointer variable
               isVar = true;

            } else if (isVarWithConstructor(ptrEntry)) {
               // class variable initialized by constructor
               isVar = true;

            }
         }
      }
   }

   if (isVar) {
      // documented variable
      addVariable(ptrEntry, isFuncPtr);
   }

   for (auto e : ptrEntry->children() ) {
      if (e->section != Entry::ENUM_SEC) {
         buildVarList(e);
      }
   }
}

void Doxy_Work::resolveHiddenNamespace()
{
   std::set<QString> visibleSet;
   std::map<QString, QSharedPointer<NamespaceDef>> hiddenMap;

   // are there any classes or members in this namespace
   for (auto &nd : Doxy_Globals::namespaceSDict) {
      const ClassSDict &cd = nd->getClassSDict();

      const QList<QSharedPointer<MemberList>> &list = nd->getMemberLists();
      bool hasMembers = false;

      for (auto item : list)  {
         if (item != nullptr && ! item->isEmpty())  {
            hasMembers = true;
            break;
         }
      }

      if (cd.isEmpty() && ! hasMembers) {
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
void Doxy_Work::addInterfaceOrServiceToServiceOrSingleton(QSharedPointer<Entry> ptrEntry, QSharedPointer<ClassDef> cd, QString const &rname)
{
   QSharedPointer<Entry> root = ptrEntry;
   QSharedPointer<FileDef> const fd = ptrEntry->fileDef();

   const MemberDefType type = (ptrEntry->section == Entry::EXPORTED_INTERFACE_SEC) ? MemberDefType::Interface : MemberDefType::Service;

   QString fileName = root->getData(EntryKey::File_Name);

   if (fileName.isEmpty() && ! ptrEntry->m_tagInfo.isEmpty()) {
      fileName = ptrEntry->m_tagInfo.tag_Name;
   }

   QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(fileName, root->startLine, root->startColumn,
         root->getData(EntryKey::Member_Type), rname, "", "", root->protection, root->virt, root->stat, Member,
         type, ArgumentList(), root->argList);

   md->setTagInfo(ptrEntry->m_tagInfo);
   md->setMemberClass(cd);
   md->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
   md->setDocsForDefinition(false);
   md->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
   md->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);
   md->setBodySegment(root->startBodyLine, root->endBodyLine);
   md->setMemberTraits(root->m_traits);
   md->setMemberGroupId(root->mGrpId);
   md->setTypeConstraints(root->typeConstr);
   md->setLanguage(root->m_srcLang);
   md->setBodyDef(fd);
   md->setFileDef(fd);
   md->addSectionsToDefinition(root->m_anchors);

   QString const def = root->getData(EntryKey::Member_Type) + " " + rname;
   md->setDefinition(def);
   md->enableCallGraph(root->callGraph);
   md->enableCallerGraph(root->callerGraph);

   Debug::print(Debug::Functions, 0, "  Interface Member:\n"
                "    `%s' `%s' proto=%d\n"
                "    def=`%s'\n", csPrintable(root->getData(EntryKey::Member_Type)), csPrintable(rname), root->proto, csPrintable(def));

   // add member to the global list of all members
   QSharedPointer<MemberName> mn;
   mn = Doxy_Globals::memberNameSDict.find(rname);

   if (mn) {
      mn->append(md);

   } else {
      mn = QMakeShared<MemberName>(rname);
      mn->append(md);
      Doxy_Globals::memberNameSDict.insert(rname, mn);
   }

   // add member to the class cd
   cd->insertMember(md);

   // also add the member as a "base" (to get nicer diagrams)
   // "optional" interface/service get Protected which turns into dashed line

   bool isOptional = root->m_traits.hasTrait(Entry::Virtue::Optional);

   BaseInfo base(rname, isOptional ? Protected : Public, Normal);

   if (! findClassRelation(ptrEntry, cd, cd, &base, QHash<QString, int>(), DocumentedOnly, true)) {
      findClassRelation(ptrEntry, cd, cd, &base, QHash<QString, int>(), Undocumented, true);
   }

   // add file to list of used files
   cd->insertUsedFile(fd);

   addMemberToGroups(root, md);
   ptrEntry->setSection(Entry::EMPTY_SEC);
   md->setRefItems(root->m_specialLists);
}

void Doxy_Work::buildInterfaceAndServiceList(QSharedPointer<Entry> ptrEntry)
{
   if (ptrEntry->section == Entry::EXPORTED_INTERFACE_SEC || ptrEntry->section == Entry::INCLUDED_SERVICE_SEC) {

      QSharedPointer<const Entry> root = ptrEntry->entry();

      int tmpValue = -1;

      if (! root->m_templateArgLists.isEmpty()) {
         tmpValue = root->m_templateArgLists.count();
      }

      Debug::print(Debug::Functions, 0,"EXPORTED_INTERFACE_SEC:\n"
                   "  `%s' `%s'::`%s' `%s' relates=`%s' relatesType=`%d' file=`%s' "
                   "line=`%d' bodyLine=`%d' #tArgLists=%d mGrpId=%d proto=%d docFile=%s\n",
                   csPrintable(root->getData(EntryKey::Member_Type)), csPrintable(ptrEntry->parent()->m_entryName),
                   csPrintable(root->m_entryName),
                   csPrintable(root->getData(EntryKey::Member_Args)), csPrintable(root->getData(EntryKey::Related_Class)), root->relatesType,
                   csPrintable(root->getData(EntryKey::File_Name)), root->startLine, root->startBodyLine, tmpValue, root->mGrpId, root->proto,
                   csPrintable(root->getData(EntryKey::MainDocs_File)) );

      QString const rname = removeRedundantWhiteSpace(root->m_entryName);

      if (! rname.isEmpty()) {
         QString const scope = ptrEntry->parent()->m_entryName;
         QSharedPointer<ClassDef> const cd = getClass(scope);
         assert(cd);

         if (cd && ((CompoundType::Interface == cd->compoundType()) || (CompoundType::Service   == cd->compoundType()) ||
                    (CompoundType::Singleton == cd->compoundType()))) {
            addInterfaceOrServiceToServiceOrSingleton(ptrEntry, cd, rname);

         } else {
            assert(false);
         }

      } else if (rname.isEmpty()) {
         warn(root->getData(EntryKey::File_Name), root->startLine, "Illegal member name found.");
      }
   }

   // can only have these in IDL anyway
   switch (ptrEntry->m_srcLang) {
      case SrcLangExt_Unknown: // (root node always is Unknown)
      case SrcLangExt_IDL:
         RECURSE_ENTRYTREE(buildInterfaceAndServiceList, ptrEntry);
         break;

      default:
         return; // nothing to do here
   }
}

// search the Entry tree for Function sections, if found store in their class or the global list

void Doxy_Work::addMethodToClass(QSharedPointer<Entry> ptrEntry, QSharedPointer<ClassDef> cd,
      const QString &rname, bool isFriend)
{
   static const bool hideScopeNames = Config::getBool("hide-scope-names");

   QSharedPointer<Entry> root = ptrEntry;
   QSharedPointer<FileDef> fd = ptrEntry->fileDef();

   QString entryMemberType = root->getData(EntryKey::Member_Type);

   static QRegularExpression regExp("\\([a-z_A-Z0-9: ]*[ &*]+[ ]*");
   QRegularExpressionMatch match = regExp.match(entryMemberType);

   int ts  = entryMemberType.indexOf('<');
   int te  = entryMemberType.lastIndexOf('>');

   int i   = -1;
   int len = 0;

   if (match.hasMatch()) {
      i   = match.capturedStart() - entryMemberType.constBegin();
      len = match.capturedLength();
   }

   if (i != -1 && ts != -1 && ts < te && ts < i && i < te) {
      // avoid changing A<int(int*)>, see bug 677315
      i = -1;
   }

   bool isAlias = root->m_traits.hasTrait(Entry::Virtue::Alias);

   if (cd->getLanguage() == SrcLangExt_Cpp && ! entryMemberType.isEmpty() && ! isAlias && i != -1) {
      // only C has pointers, function variable

      root->prependData(EntryKey::Member_Args, entryMemberType.right(entryMemberType.length() - i - len));
      root->setData(EntryKey::Member_Type,     entryMemberType.left(i + len));
   }

   QString name = removeRedundantWhiteSpace(rname);
   if (name.startsWith("::")) {
      name = name.right(name.length() - 2);
   }

   MemberDefType memberType;

   if (root->mtype == MethodType::Signal) {
      memberType = MemberDefType::Signal;

   } else if (root->mtype == MethodType::Slot) {
      memberType = MemberDefType::Slot;

   } else if (root->mtype == MethodType::DCOP) {
      memberType = MemberDefType::DCOP;

   } else {
      memberType = MemberDefType::Function;
   }

   // strip redundant template specifier for constructors
   if ((fd == nullptr || fd->getLanguage() == SrcLangExt_Cpp) && name.left(9) != "operator " &&
         (i = name.indexOf('<')) != -1 && name.indexOf('>') != -1) {

      name = name.left(i);
   }

   QString fileName = root->getData(EntryKey::File_Name);

   if (fileName.isEmpty() && ! ptrEntry->m_tagInfo.isEmpty()) {
      fileName = ptrEntry->m_tagInfo.tag_Name;
   }

   ArgumentList tmpList;

   if (! root->m_templateArgLists.isEmpty()) {
      tmpList = root->m_templateArgLists.last();
   }

   // adding class member
   Relationship tRelation;

   if (isFriend) {
      tRelation = Relationship::Friend;

   } else if (root->getData(EntryKey::Related_Class).isEmpty()) {
      tRelation = Relationship::Member;

   } else if (root->relatesType == RelatesType::MemberOf) {
      tRelation = Relationship::Foreign;

   } else {
      tRelation = Relationship::Related;

   }

   QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(fileName, root->startLine, root->startColumn,
               root->getData(EntryKey::Member_Type), name, root->getData(EntryKey::Member_Args),
               root->getData(EntryKey::Exception_Spec), root->protection, root->virt,
               (root->stat && root->relatesType != MemberOf), tRelation, memberType, tmpList, root->argList);

   md->setTagInfo(ptrEntry->m_tagInfo);
   md->setMemberClass(cd);
   md->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
   md->setDocsForDefinition(! root->proto);
   md->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
   md->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);
   md->setBodySegment(root->startBodyLine, root->endBodyLine);
   md->setMemberTraits(root->m_traits);
   md->setMemberGroupId(root->mGrpId);
   md->setTypeConstraints(root->typeConstr);
   md->setLanguage(root->m_srcLang);
   md->setId(root->getData(EntryKey::Clang_Id));
   md->setBodyDef(fd);
   md->setFileDef(fd);
   md->addSectionsToDefinition(root->m_anchors);
   md->setFriend(isFriend);

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

   if (! root->getData(EntryKey::Related_Class).isEmpty() || isFriend || hideScopeNames) {

      if (! root->getData(EntryKey::Member_Type).isEmpty()) {
         def = root->getData(EntryKey::Member_Type) + " " + name;

      } else {
         def = name;

      }

   } else {
      if (! root->getData(EntryKey::Member_Type).isEmpty()) {
        def = root->getData(EntryKey::Member_Type) + " " + qualScope + scopeSeparator + name;

      } else {
         def = qualScope + scopeSeparator + name;

      }
   }

   if (def.startsWith("friend ")) {
      def = def.right(def.length() - 7);
   }

   md->setDefinition(def);
   md->enableCallGraph(root->callGraph);
   md->enableCallerGraph(root->callerGraph);

   // add member to the global list of all members
   QSharedPointer<MemberName> mn;
   mn = Doxy_Globals::memberNameSDict.find(name);

   if (mn) {
      mn->append(md);

   } else {
      mn = QMakeShared<MemberName>(name);
      mn->append(md);
      Doxy_Globals::memberNameSDict.insert(name, mn);
   }

   // add member to the class cd
   cd->insertMember(md);

   // add file to list of used files
   cd->insertUsedFile(fd);

   // add member to groups
   addMemberToGroups(root, md);

   ptrEntry->setSection(Entry::EMPTY_SEC);

   md->setRefItems(root->m_specialLists);
}

void Doxy_Work::buildFunctionList(QSharedPointer<Entry> ptrEntry)
{
   if (ptrEntry->section == Entry::FUNCTION_SEC) {
      QSharedPointer<Entry> root = ptrEntry;

      int tmpValue = -1;

      if (! root->m_templateArgLists.isEmpty()) {
         tmpValue = root->m_templateArgLists.count();
      }

      Debug::print(Debug::Functions, 0,"FUNCTION_SEC:\n"
                   "  `%s' `%s'::`%s' `%s' relates=`%s' relatesType=`%d' file=`%s' "
                   "line=`%d' bodyLine=`%d' #tArgLists=%d mGrpId=%d proto=%d docFile=%s\n",
                   csPrintable(root->getData(EntryKey::Member_Type)), csPrintable(ptrEntry->parent()->m_entryName),
                   csPrintable(root->m_entryName),
                   csPrintable(root->getData(EntryKey::Member_Args)), csPrintable(root->getData(EntryKey::Related_Class)), root->relatesType,
                   csPrintable(root->getData(EntryKey::File_Name)), root->startLine, root->startBodyLine, tmpValue, root->mGrpId, root->proto,
                   csPrintable(root->getData(EntryKey::MainDocs_File)) );

      bool isFriend = root->getData(EntryKey::Member_Type).indexOf("friend ") != -1;
      QString rname = removeRedundantWhiteSpace(root->m_entryName);

      QString scope = ptrEntry->parent()->m_entryName;              // stripAnonymousNamespaceScope(root->parent->name);

      if (! rname.isEmpty() && scope.indexOf('@') == -1) {
         QSharedPointer<ClassDef> cd = QSharedPointer<ClassDef>();

         // check if this function's parent is a class
         scope = stripTemplateSpecifiersFromScope(scope, false);

         QSharedPointer<FileDef> rfd = ptrEntry->fileDef();
         int memIndex = rname.lastIndexOf("::");

         cd = getClass(scope);
         if (cd && scope + "::" == rname.left(scope.length() + 2)) {
            // found A::f inside A

            // strip scope from name
            rname = rname.right(rname.length() - ptrEntry->parent()->m_entryName.length() - 2);
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

         static QRegularExpression regExp("\\([a-z_A-Z0-9: ]*[ &*]+[ ]*");

         int ts = root->getData(EntryKey::Member_Type).indexOf('<');
         int te = root->getData(EntryKey::Member_Type).lastIndexOf('>');
         int ti;

         bool done = false;

         if (! ptrEntry->parent()->m_entryName.isEmpty() && (ptrEntry->parent()->section & Entry::COMPOUND_MASK) && cd) {

            QString tmpType = root->getData(EntryKey::Member_Type);
            QString tmpArgs = root->getData(EntryKey::Member_Args);

            ti = tmpType.indexOf(regExp);
            bool tempA = (ts != -1 && ts < te && ts < ti && ti < te);

            if (tmpType.isEmpty() || (ti == -1 || tempA || tmpArgs.indexOf(")[") != -1) ||
                  tmpType.indexOf(")(") != -1  || tmpType.indexOf("operator") != -1 || cd->getLanguage() != SrcLangExt_Cpp) {

               // odd code to exclude function pointers, type does not contain ..(..*
               // outside of < ... >
               // and args not )[.. -> function pointer
               // type contains ..)(.. and not "operator"
               // language other than C

               addMethodToClass(ptrEntry, cd, rname, isFriend);
               done = true;
            }
         }

         if (done) {
            // all done

         } else if (! ((ptrEntry->parent()->section & Entry::COMPOUND_MASK) || ptrEntry->parent()->section == Entry::OBJCIMPL_SEC) &&
                  ! isMember && (root->getData(EntryKey::Related_Class).isEmpty() || root->relatesType == Duplicate) &&
                  ! root->getData(EntryKey::Member_Type).startsWith("extern ") &&
                  ! root->getData(EntryKey::Member_Type).startsWith("typedef ") )   {

            // no member => unrelated function

            // check the uniqueness of the function name in the file. A file could contain a
            // function prototype and a function definition or even multiple function prototypes.

            QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict.find(rname);

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
                  QString parentScope = ptrEntry->parent()->m_entryName;

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

                  const ArgumentList &mdArgList = item->getArgumentList();
                  ArgumentList &mdTempArgList   = item->getTemplateArgumentList();

                  // for template functions, check if they have the same number of template parameters
                  bool sameNumTemplateArgs = true;
                  bool matchingReturnTypes = true;

                  if (! mdTempArgList.listEmpty() && ! root->m_templateArgLists.isEmpty()) {

                     if (mdTempArgList.count() != root->m_templateArgLists.last().count()) {
                        sameNumTemplateArgs = false;
                     }

                     if (item->typeString() != removeRedundantWhiteSpace(root->getData(EntryKey::Member_Type))) {
                        matchingReturnTypes = false;
                     }
                  }

                  bool staticsInDifferentFiles = root->stat && item->isStatic() &&
                           root->getData(EntryKey::File_Name) != item->getDefFileName();

                  bool tmp = matchArguments2(item->getOuterScope(), mfd, mdArgList,
                           rnd ? rnd : Doxy_Globals::globalScope, rfd, root->argList, false);

                  if (tmp && sameNumTemplateArgs && matchingReturnTypes && ! staticsInDifferentFiles) {

                     QSharedPointer<GroupDef> gd;

                     if (! root->m_groups.isEmpty()) {
                        gd = Doxy_Globals::groupSDict.find(root->m_groups.first().groupname);
                     }

                     // check if we need to create a new member
                     found = (mnd && rnd && nsName == rnsName) ||                       // members are in the same namespace
                             ((mnd == nullptr && rnd == nullptr && mfd != nullptr &&    // no external reference and
                               mfd->getFilePath() == root->getData(EntryKey::File_Name)));

                     // otherwise, allow a duplicate global member with the same argument list
                     if (! found && gd && gd == item->getGroupDef() && nsName == rnsName) {
                        // member is already in the group, do not add it again

                        found = true;
                     }

                     if (found) {
                        // md is used after this for() loop
                        md = item;

                        // merge argument lists
                        mergeArguments(mdTempArgList, root->argList, ! root->getData(EntryKey::Main_Docs).isEmpty());

                        // merge documentation
                        if (item->documentation().isEmpty() && ! root->getData(EntryKey::Main_Docs).isEmpty()) {

                           const ArgumentList &argList = stringToArgumentList(root->getData(EntryKey::Member_Args));

                           if (root->proto) {
                              item->setDeclArgumentList(argList);

                           } else {
                              item->setArgumentList(argList);

                           }
                        }

                        item->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
                        item->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);
                        item->setDocsForDefinition(! root->proto);

                        if (item->getStartBodyLine() == -1 && root->startBodyLine != -1) {
                           item->setBodySegment(root->startBodyLine, root->endBodyLine);
                           item->setBodyDef(rfd);
                        }

                        if (item->briefDescription().isEmpty() && ! root->getData(EntryKey::Brief_Docs).isEmpty()) {
                           item->setArgsString(root->getData(EntryKey::Member_Args));
                        }

                        item->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);

                        item->addSectionsToDefinition(root->m_anchors);

                        item->enableCallGraph(item->hasCallGraph() || root->callGraph);
                        item->enableCallerGraph(item->hasCallerGraph() || root->callerGraph);

                        // merge ingroup specifiers
                        if (item->getGroupDef() == 0 && root->m_groups.size() > 0) {
                           addMemberToGroups(root, item);

                        } else if (item->getGroupDef() != 0 && root->m_groups.count() == 0) {
                           root->m_groups.append(Grouping(item->getGroupDef()->name(), item->getGroupPri()));

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
               ArgumentList tArgList;

               if (! root->m_templateArgLists.isEmpty()) {
                  tArgList = root->m_templateArgLists.last();
               }

               QString name = removeRedundantWhiteSpace(rname);

               md = QMakeShared<MemberDef>(root->getData(EntryKey::File_Name), root->startLine, root->startColumn,
                  root->getData(EntryKey::Member_Type), name, root->getData(EntryKey::Member_Args),
                  root->getData(EntryKey::Exception_Spec), root->protection, root->virt, root->stat, Member,
                  MemberDefType::Function, tArgList, root->argList);

               md->setTagInfo(ptrEntry->m_tagInfo);
               md->setLanguage(root->m_srcLang);
               md->setId(root->getData(EntryKey::Clang_Id));

               md->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
               md->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
               md->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);
               md->setPrototype(root->proto);
               md->setDocsForDefinition(!root->proto);
               md->setTypeConstraints(root->typeConstr);

               md->setBodySegment(root->startBodyLine, root->endBodyLine);

               QSharedPointer<FileDef> fd = ptrEntry->fileDef();

               md->setBodyDef(fd);
               md->addSectionsToDefinition(root->m_anchors);
               md->setMemberTraits(root->m_traits);
               md->setMemberGroupId(root->mGrpId);

               // see if the function is inside a namespace that was not part of
               // the name already (in that case nd should be non-zero)

               if (nd == nullptr && ptrEntry->parent()->section == Entry::NAMESPACE_SEC ) {
                  QString nscope = ptrEntry->parent()->m_entryName;

                  if (! nscope.isEmpty()) {
                     nd = getResolvedNamespace(nscope);
                  }
               }

               if (! scope.isEmpty()) {
                  QString sep = getLanguageSpecificSeparator(root->m_srcLang);

                  if (sep != "::") {
                     scope = substitute(scope, "::", sep);
                  }

                  scope += sep;
               }

               QString def;

               if (! root->getData(EntryKey::Member_Type).isEmpty()) {
                  def = root->getData(EntryKey::Member_Type) + " " + scope + name;

               } else {
                  def = scope + name;

               }

               Debug::print(Debug::Functions, 0, "  Global Function:\n"
                            "    `%s' `%s'::`%s' `%s' proto=%d\n"
                            "    def=`%s'\n",
                            csPrintable(root->getData(EntryKey::Member_Type)), csPrintable(ptrEntry->parent()->m_entryName), csPrintable(rname),
                            csPrintable(root->getData(EntryKey::Member_Args)), root->proto, csPrintable(def) );

               md->setDefinition(def);
               md->enableCallGraph(root->callGraph);
               md->enableCallerGraph(root->callerGraph);

               md->setRefItems(root->m_specialLists);

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
               mn = Doxy_Globals::functionNameSDict.find(name);

               if (mn) {
                  mn->append(md);

               } else {
                  mn = QMakeShared<MemberName>(name);
                  mn->append(md);

                  Doxy_Globals::functionNameSDict.insert(name, mn);
               }

               addMemberToGroups(root, md);

               if (root->relatesType == Simple)  {
                  // if this is a relatesalso command allow find Member to pick it up
                   // Otherwise we have finished with this entry

                   ptrEntry->setSection(Entry::EMPTY_SEC);
               }

            } else {
               QSharedPointer<FileDef> fd = ptrEntry->fileDef();

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
         warn(root->getData(EntryKey::File_Name), root->startLine, "Illegal member name found." );
      }
   }

   RECURSE_ENTRYTREE(buildFunctionList, ptrEntry);
}

void Doxy_Work::findFriends()
{
   for (auto fn : Doxy_Globals::functionNameSDict) {
      // for each global function name

      QSharedPointer<MemberName> mn = Doxy_Globals::memberNameSDict.find(fn->memberName());

      if (mn) {
         // there are members with the same name

         for (auto fmd : *fn) {

            for (auto mmd : *mn) {

               ArgumentList &mmdAl = mmd->getArgumentList();
               ArgumentList &fmdAl = fmd->getArgumentList();

               bool ok = mmd->isFriend() || (mmd->isRelated() && mmd->isFunction());

               if (ok &&  matchArguments2(mmd->getOuterScope(), mmd->getFileDef(), mmdAl,
                                          fmd->getOuterScope(), fmd->getFileDef(), fmdAl, true)) {

                   // if the member is related and the arguments match then the
                   // function is actually a friend

                  mergeArguments(mmdAl, fmdAl);

                  if (! fmd->documentation().isEmpty()) {
                     mmd->setDocumentation(fmd->documentation(), fmd->docFile(), fmd->docLine());

                  } else if (! mmd->documentation().isEmpty()) {
                     fmd->setDocumentation(mmd->documentation(), mmd->docFile(), mmd->docLine());
                  }

                  if (mmd->briefDescription().isEmpty() && !fmd->briefDescription().isEmpty()) {
                     mmd->setBriefDescription(fmd->briefDescription(), fmd->briefFile(), fmd->briefLine());

                  } else if (! mmd->briefDescription().isEmpty() && !fmd->briefDescription().isEmpty()) {
                     fmd->setBriefDescription(mmd->briefDescription(), mmd->briefFile(), mmd->briefLine());
                  }

                  if (! fmd->inbodyDocumentation().isEmpty()) {
                     mmd->setInbodyDocumentation(fmd->inbodyDocumentation(), fmd->inbodyFile(), fmd->inbodyLine());

                  } else if (! mmd->inbodyDocumentation().isEmpty()) {
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
   // find matching function declaration and definitions
   for (auto mn : Doxy_Globals::functionNameSDict) {

      // find a matching function declaration and definition for this function
      for (auto mdec : *mn ) {

         if (mdec->isPrototype() || (mdec->isVariable() && mdec->isExternal()) ) {
            for (auto mdef : *mn ) {
               if (mdec != mdef && mdec->getNamespaceDef()== mdef->getNamespaceDef() && ! mdec->isAlias() && ! mdef->isAlias()) {
                  combineDeclarationAndDefinition(mdec, mdef);
               }
            }
         }
      }
   }
}

void Doxy_Work::transferFunctionReferences()
{
   for (auto mn : Doxy_Globals::functionNameSDict ) {
      QSharedPointer<MemberDef> mdef;
      QSharedPointer<MemberDef> mdec;

      /* find a matching function declaration and definition for this function */
      for (auto md : *mn) {

         if (md->isPrototype()) {
            mdec = md;
         } else if (md->isVariable() && md->isExternal()) {
            mdec = md;
         }

         if (md->isFunction() && ! md->isStatic() && !md->isPrototype()) {
            mdef = md;
         } else if (md->isVariable() && !md->isExternal() && !md->isStatic()) {
            mdef = md;
         }
      }

      if (mdef && mdec) {
         const ArgumentList &mdefAl = mdef->getArgumentList();
         const ArgumentList &mdecAl = mdec->getArgumentList();

         if ( matchArguments2(mdef->getOuterScope(), mdef->getFileDef(), mdefAl,
                              mdec->getOuterScope(), mdec->getFileDef(), mdecAl, true )) {

            /* match found */
            {
               const MemberSDict &defDict = mdef->getReferencesMembers();
               const MemberSDict &decDict = mdec->getReferencesMembers();

               for (auto &rmd : defDict) {
                  if (decDict.isEmpty() || decDict.find(rmd->name()) == nullptr) {
                     mdec->addSourceReferences(rmd);
                  }
               }

               for (auto &rmd : decDict) {
                  if (defDict.isEmpty() || defDict.find(rmd->name()) == nullptr) {
                     mdef->addSourceReferences(rmd);
                  }
               }
            }

            {
               const MemberSDict &defDict = mdef->getReferencedByMembers();
               const MemberSDict &decDict = mdec->getReferencedByMembers();

               for (auto &rmd : defDict) {
                  if (decDict.isEmpty() || decDict.find(rmd->name()) == nullptr) {
                     mdec->addSourceReferencedBy(rmd);
                  }
               }

              for (auto &rmd : decDict) {
                  if (defDict.isEmpty() || defDict.find(rmd->name()) == nullptr) {
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

   for (auto mn : Doxy_Globals::functionNameSDict) {

      /* find a matching function declaration and definition for this function */
      for (auto md : *mn) {

         QSharedPointer<MemberName> rmn = Doxy_Globals::memberNameSDict.find(md->name());

         if (rmn) {
            // check if there is a member with the same name

            for (auto rmd : *rmn) {

               const ArgumentList &mdAl  = md->getArgumentList();
               const ArgumentList &rmdAl = rmd->getArgumentList();

               if ((rmd->isRelated() || rmd->isForeign()) &&
                     matchArguments2(  md->getOuterScope(),  md->getFileDef(), mdAl,
                                      rmd->getOuterScope(), rmd->getFileDef(), rmdAl, true)) {

                  // related function
                  if (rmd->relatedAlso()) {
                     md->setRelatedAlso(rmd->relatedAlso());

                  } else if (rmd->isForeign()) {
                     md->makeForeign();

                  } else if (rmd->isFriend()) {
                     md->makeFriend();

                  } else {
                     md->makeRelated();

                  }
               }
            }
         }
      }
   }
}

/*! make a dictionary of all template arguments of class cd, which are part of the base class name
 * Example: A template class A with template arguments <R,S,T>
 * that inherits from B<T,T,S> will have T and S in the dictionary
 */
QHash<QString, int> Doxy_Work::getTemplateArgumentsInName(const ArgumentList &templateArgumentList, const QString &name)
{
   QHash<QString, int> templateNames;
   static QRegularExpression regExp("[a-z_A-Z][a-z_A-Z0-9:]*");

   int count = 0;

   for (auto &item : templateArgumentList) {
      QRegularExpressionMatch match = regExp.match(name);

      while (match.hasMatch()) {
         QString tmp = match.captured();

         if (tmp == item.name) {
            if (! templateNames.contains(tmp)) {
               templateNames.insert(tmp, count);
            }
         }

         match = regExp.match(name, match.capturedEnd());
      }

      ++count;
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

      result = Doxy_Globals::genericsSDict.find(name);
   }

   return result;
}

void Doxy_Work::findUsedClassesForClass(QSharedPointer<Entry> ptrEntry, QSharedPointer<Definition> context,
                  QSharedPointer<ClassDef> masterCd, QSharedPointer<ClassDef> instanceCd, bool isArtificial,
                  const ArgumentList &actualArgs, QHash<QString, int> templateNames)
{
   masterCd->visited = true;
   const ArgumentList &formalArgs = masterCd->getTemplateArgumentList();

   for (auto mni : masterCd->memberNameInfoSDict()) {

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
            if (! actualArgs.listEmpty()) {
               type = substituteTemplateArgumentsInString(type, formalArgs, actualArgs);
            }

            while (! found && extractClassNameFromType(type, pos, usedClassName, templSpec, ptrEntry->m_srcLang) != -1) {

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

               if (templateNames.isEmpty()) {
                  templateNames = getTemplateArgumentsInName(formalArgs, usedName);
               }

               BaseInfo bi(usedName, Public, Normal);
               findClassRelation(ptrEntry, context, instanceCd, &bi, templateNames, TemplateInstances, isArtificial);

               const ArgumentList &masterList = masterCd->getTemplateArgumentList();

               for (auto &arg : masterList) {
                  if (arg.name == usedName) {
                     // type is a template argument
                     found = true;
                     Debug::print(Debug::Classes, 0, "    New used class `%s'\n", csPrintable(usedName));

                     QSharedPointer<ClassDef> usedCd = Doxy_Globals::hiddenClasses.find(usedName);

                     if (! usedCd) {
                        usedCd = QMakeShared<ClassDef>(masterCd->getDefFileName(), masterCd->getDefLine(),
                                 masterCd->getDefColumn(), usedName, CompoundType::Class);

                        usedCd->makeTemplateArgument();
                        usedCd->setUsedOnly(true);
                        usedCd->setLanguage(masterCd->getLanguage());

                        Doxy_Globals::hiddenClasses.insert(usedName, usedCd);
                     }

                     if (isArtificial) {
                        usedCd->setArtificial(true);
                     }

                     Debug::print(Debug::Classes, 0, "      Adding used class `%s' (1)\n", csPrintable(usedCd->name()));
                     instanceCd->addUsedClass(usedCd, md->name(), md->protection());
                     usedCd->addUsedByClass(instanceCd, md->name(), md->protection());
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
            }

            if (! found && ! type.isEmpty()) {
               // used class is not documented in any scope
               QSharedPointer<ClassDef> usedCd = Doxy_Globals::hiddenClasses.find(type);

               if (usedCd == 0 && ! Config::getBool("hide-undoc-relations")) {
                  if (type.endsWith("(*") || type.endsWith("(^")) {
                     // type is a function pointer
                     type += md->argsString();
                  }

                  Debug::print(Debug::Classes, 0, "  New undocumented used class `%s'\n", csPrintable(type));

                  usedCd = QMakeShared<ClassDef>(masterCd->getDefFileName(), masterCd->getDefLine(),
                     masterCd->getDefColumn(), type, CompoundType::Class);

                  usedCd->setUsedOnly(true);
                  usedCd->setLanguage(masterCd->getLanguage());
                  Doxy_Globals::hiddenClasses.insert(type, usedCd);
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

void Doxy_Work::findBaseClassesForClass(QSharedPointer<Entry> ptrEntry, QSharedPointer<Definition> context,
         QSharedPointer<ClassDef> masterCd, QSharedPointer<ClassDef>instanceCd,
         FindBaseClassRelation_Mode mode, bool isArtificial, const ArgumentList &actualArgs,
         QHash<QString, int> templateNames)
{
   QSharedPointer<Entry> root = ptrEntry;

   masterCd->visited = true;

   // base class could also be a non-nested class
   ArgumentList &formalArgs = masterCd->getTemplateArgumentList();

   for (auto bi : root->extends) {

      if (templateNames.isEmpty()) {
         templateNames = getTemplateArgumentsInName(formalArgs, bi.name);
      }

      BaseInfo tbi(bi.name, bi.prot, bi.virt);

      if (! actualArgs.listEmpty()) {
         // substitute the formal template arguments of the base class
         tbi.name = substituteTemplateArgumentsInString(bi.name, formalArgs, actualArgs);
      }

      if (mode == DocumentedOnly) {
         // find a documented base class in the correct scope

         if (! findClassRelation(ptrEntry, context, instanceCd, &tbi, templateNames, DocumentedOnly, isArtificial)) {
            // decided to show inheritance relations even if not documented,
            // make them artificial, so they do not appear in the index

            bool hideUndocRelations = Config::getBool("hide-undoc-relations") ? true : isArtificial;

            // no documented base class -> try to find an undocumented one
            findClassRelation(ptrEntry, context, instanceCd, &tbi, templateNames, Undocumented, hideUndocRelations);
         }

      } else if (mode == TemplateInstances) {
         findClassRelation(ptrEntry, context, instanceCd, &tbi, templateNames, TemplateInstances, isArtificial);

      }

   }
}

bool Doxy_Work::findTemplateInstanceRelation(QSharedPointer<Entry> root, QSharedPointer<Definition> context, QSharedPointer<ClassDef> templateClass,
               const QString &templSpec, QHash<QString, int> templateNames, bool isArtificial)
{
   Debug::print(Debug::Classes, 0, "    derived from template %s with parameters %s\n",
                csPrintable(templateClass->name()), csPrintable(templSpec));

   bool existingClass = (templSpec == tempArgListToString(templateClass->getTemplateArgumentList(), root->m_srcLang) );

   if (existingClass) {
      return true;
   }

   bool freshInstance = false;
   QSharedPointer<ClassDef> instanceClass = templateClass->insertTemplateInstance(
                  root->getData(EntryKey::File_Name), root->startLine, root->startColumn, templSpec, freshInstance);

   if (isArtificial) {
      instanceClass->setArtificial(true);
   }

   instanceClass->setLanguage(root->m_srcLang);

   if (freshInstance) {
      Debug::print(Debug::Classes, 0, "      found fresh instance '%s'\n", csPrintable(instanceClass->name()));
      Doxy_Globals::classSDict.insert(instanceClass->name(), instanceClass);

      if (! templateNames.isEmpty()) {
         instanceClass->setTemplateBaseClassNames(templateNames);
      }

      // search for new template instances caused by base classes of instanceClass
      QSharedPointer<Entry> template_root = Doxy_Globals::g_classEntries.value(templateClass->name());

      if (template_root) {
         const ArgumentList &templArgs = stringToArgumentList(templSpec);

         findBaseClassesForClass(template_root, context, templateClass, instanceClass,
                                 TemplateInstances, isArtificial, templArgs, templateNames);

         findUsedClassesForClass(template_root, context, templateClass, instanceClass,
                                 isArtificial, templArgs, templateNames);
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

bool Doxy_Work::findClassRelation(QSharedPointer<Entry> ptrEntry, QSharedPointer<Definition> context, QSharedPointer<ClassDef> cd, BaseInfo *bi,
                                  QHash<QString, int> templateNames, FindBaseClassRelation_Mode mode, bool isArtificial)
{
   QSharedPointer<Entry> root = ptrEntry->entry();

   QString biName = bi->name;
   bool isExplicitGlobalScope = false;

   if (biName.startsWith("::")) {
      // explicit global scope
      biName = biName.right(biName.length() - 2);
      isExplicitGlobalScope = true;
   }

   QSharedPointer<Entry> parentNode = ptrEntry->parent();
   bool lastParent = false;

   // for each parent scope, starting with the largest scope
   // (in case of nested classes)

   do {
      QString scopeName;

      if (parentNode) {
         scopeName =  parentNode->m_entryName;
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

         if (! isRecursiveBaseClass(ptrEntry->m_entryName, baseClassName) || isExplicitGlobalScope
               || (ptrEntry->m_srcLang == SrcLangExt_IDL &&
                  (ptrEntry->section == Entry::EXPORTED_INTERFACE_SEC ||
                   ptrEntry->section == Entry::INCLUDED_SERVICE_SEC))) {

               // isRecursiveBaseClass is always true for UNO IDL ifc/svc members
               // (i.e. this is needed for addInterfaceOrServiceToServiceOrSingleton)

            Debug::print(Debug::Classes, 0, "    class relation %s inherited/used by %s found (%s and %s) templSpec='%s'\n",
               csPrintable(baseClassName), csPrintable(ptrEntry->m_entryName),
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

            if (baseClass == 0 && (root->m_srcLang == SrcLangExt_CSharp || root->m_srcLang == SrcLangExt_Java)) {
               // for Java/C# strip the template part before looking for matching

               baseClass = Doxy_Globals::genericsSDict.find(baseClassName.left(i));
            }

            if (baseClass == 0 && i != -1) {
               // base class has template specifiers

               // TODO: try to find the correct template specialization
               // for now, only look for the unspecialized base class

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

            if (templateNames.contains(biName)) {
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
               // since instantiating a template may introduce new inheritance relations.

               if (! templSpec.isEmpty() && mode == TemplateInstances) {
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
                     warn(root->getData(EntryKey::File_Name), root->startLine, "Detected potential recursive class relation "
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
                  baseClass = Doxy_Globals::hiddenClasses.find(baseClassName);

                  if (baseClass == 0) {
                     baseClass = QMakeShared<ClassDef>(root->getData(EntryKey::File_Name), root->startLine, root->startColumn,
                                             baseClassName, CompoundType::Class);

                     Doxy_Globals::hiddenClasses.insert(baseClassName, baseClass);

                     if (isArtificial) {
                        baseClass->setArtificial(true);
                     }

                     baseClass->setLanguage(root->m_srcLang);
                  }

               } else {
                  baseClass = Doxy_Globals::classSDict.find(baseClassName);

                  if (baseClass == 0) {
                     baseClass = QMakeShared<ClassDef>(root->getData(EntryKey::File_Name), root->startLine, root->startColumn,
                                             baseClassName, CompoundType::Class);

                     Doxy_Globals::classSDict.insert(baseClassName, baseClass);

                     if (isArtificial) {
                        baseClass->setArtificial(true);
                     }

                     baseClass->setLanguage(root->m_srcLang);
                     int si = baseClassName.lastIndexOf("::");

                     if (si != -1) {
                        // class is nested
                        QSharedPointer<Definition> sd = findScopeFromQualifiedName(Doxy_Globals::globalScope,
                                 baseClassName.left(si), QSharedPointer<FileDef>(), ptrEntry->m_tagInfo);

                        if (! sd || sd == Doxy_Globals::globalScope) {
                           // outer scope not found
                           baseClass->setArtificial(true); // see bug678139
                        }
                     }
                  }
               }

               if (biName.endsWith("-p")) {
                  biName = "<" + biName.left(biName.length() - 2) + ">";
               }

               // add base class to this class
               cd->insertBaseClass(baseClass, biName, bi->prot, bi->virt, templSpec);

               // add this class as super class to the base class
               baseClass->insertSubClass(cd, bi->prot, bi->virt, templSpec);

               // the undocumented base was found in this file
               baseClass->insertUsedFile(ptrEntry->fileDef());
               baseClass->setOuterScope(Doxy_Globals::globalScope);

               if (baseClassName.endsWith("-p")) {
                  baseClass->setCompoundType(CompoundType::Protocol);
               }

               return true;

            } else {
               Debug::print(Debug::Classes, 0, "    Base class `%s' not found\n", csPrintable(biName));
            }

         } else {
            if (mode != TemplateInstances) {
               warn(root->getData(EntryKey::File_Name), root->startLine, "Detected potential recursive class relation "
                    "between class %s and base class %s\n", csPrintable(root->m_entryName), csPrintable(baseClassName));
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
bool Doxy_Work::isClassSection(QSharedPointer<Entry> ptrEntry)
{
   if ( ! ptrEntry->m_entryName.isEmpty() ) {

      if (ptrEntry->section & Entry::COMPOUND_MASK) {
         // is it a compound (class, struct, union, interface ...)
         return true;

      } else if (ptrEntry->section & Entry::COMPOUNDDOC_MASK)  {
         // is it a documentation block with inheritance info.

         QSharedPointer<Entry> root = ptrEntry->entry();

         bool extends = root->extends.count() > 0;

         if (extends) {
            return true;
         }
      }
   }
   return false;
}

// Build a hash of all entrys in the tree starting with root
void Doxy_Work::findClassEntries(QSharedPointer<Entry> ptr_root)
{
   if (isClassSection(ptr_root)) {
      Doxy_Globals::g_classEntries.insert(ptr_root->m_entryName, ptr_root);
   }

   RECURSE_ENTRYTREE(findClassEntries, ptr_root);
}

QString Doxy_Work::extractClassName(QSharedPointer<Entry> ptrEntry)
{
   // strip any anonymous scopes first

   QString bName = stripAnonymousNamespaceScope(ptrEntry->m_entryName);
   bName = stripTemplateSpecifiersFromScope(bName);

   int i;

   if ((ptrEntry->m_srcLang == SrcLangExt_CSharp || ptrEntry->m_srcLang == SrcLangExt_Java) &&
         (i = bName.indexOf('<')) != -1) {

      // a Java/C# generic class looks like a C++ specialization, so we need to strip the
      // template part before looking for matches

      bName = bName.left(i);
   }

   return bName;
}

/*  Use the dictionary built by findClassEntries()
 *  look for additional template specializations which exist as inheritance relations only
 *  These instances will be added to the template they are derived from
 */
void Doxy_Work::findInheritedTemplateInstances()
{
   for (auto item : Doxy_Globals::classSDict) {
      item->visited = false;
   }

   for (auto ptrEntry : Doxy_Globals::g_classEntries) {
      QSharedPointer<ClassDef> cd;

      QString bName = extractClassName(ptrEntry);
      cd = getClass(bName);

      if (cd) {
         findBaseClassesForClass(ptrEntry, cd, cd, cd, TemplateInstances, false);
      }
   }
}

void Doxy_Work::findUsedTemplateInstances()
{
   for (auto item : Doxy_Globals::classSDict) {
      item->visited = false;
   }

   for (auto ptrEntry : Doxy_Globals::g_classEntries) {
      QSharedPointer<ClassDef> cd;

      QString bName = extractClassName(ptrEntry);
      cd = getClass(bName);

      Debug::print(Debug::Classes, 0, "  Usage: Class %s : \n", csPrintable(bName));

      if (cd) {
         findUsedClassesForClass(ptrEntry, cd, cd, cd, true);
         cd->addTypeConstraints();
      }
   }
}

void Doxy_Work::computeClassRelations()
{
   static bool extractLocalClass = Config::getBool("extract-local-classes");
   static bool hideUndocClasses  = Config::getBool("hide-undoc-classes");

   for (auto item : Doxy_Globals::classSDict) {
      item->visited = false;
   }

   for (auto ptrEntry : Doxy_Globals::g_classEntries) {
      QSharedPointer<ClassDef> cd;
      QString bName = extractClassName(ptrEntry);
      cd = getClass(bName);

      Debug::print(Debug::Classes, 0, "  Relations: Class %s : \n", csPrintable(bName));

      if (cd) {
         findBaseClassesForClass(ptrEntry, cd, cd, cd, DocumentedOnly, false);
      }

      int numMembers = 0;

      if (cd) {
         numMembers = cd->memberNameInfoSDict().count();
      }

      if ((cd == 0 || (! cd->hasDocumentation() && ! cd->isReference())) && numMembers > 0 && ! bName.endsWith("::")) {

         if (! ptrEntry->m_entryName.isEmpty() && ptrEntry->m_entryName.indexOf('@') == -1 &&
               (determineSection(ptrEntry->getData(EntryKey::File_Name)) == Entry::HEADER_SEC || extractLocalClass) &&
                protectionLevelVisible(ptrEntry->protection) && ! hideUndocClasses )

            warn_undoc(ptrEntry->getData(EntryKey::File_Name), ptrEntry->startLine,
                  "Compound %s was not documented", csPrintable(ptrEntry->m_entryName));
      }
   }
}

void Doxy_Work::computeTemplateClassRelations()
{
   for (auto ptrEntry : Doxy_Globals::g_classEntries) {

      QString bName = stripAnonymousNamespaceScope(ptrEntry->m_entryName);
      bName = stripTemplateSpecifiersFromScope(bName);

      QSharedPointer<ClassDef> cd = getClass(bName);

      if (cd) {
         // strip any anonymous scopes first
         const QHash<QString, QSharedPointer<ClassDef>> &templInstances = cd->getTemplateInstances();

         Debug::print(Debug::Classes, 0, "  Template class %s : \n", csPrintable(cd->name()));

         for (auto item = templInstances.begin(); item != templInstances.end(); ++item) {
            // for each template instance

            Debug::print(Debug::Classes, 0, "    Template instance %s : \n", csPrintable(item.value()->name()) );
            QString templSpec = item.key();

            const ArgumentList &templArgs = stringToArgumentList(templSpec);

            QVector<BaseInfo> &baseList = ptrEntry->extends;

            for (auto &bi : baseList) {
               // for each base class of the template, check if the base class is a template argument

               BaseInfo tbi(bi.name, bi.prot, bi.virt);
               const ArgumentList &tl = cd->getTemplateArgumentList();

               if (! tl.listEmpty()) {
                  const QHash<QString, int> &baseClassNames = item.value()->getTemplateBaseClassNames();
                  QHash<QString, int> templateNames         = getTemplateArgumentsInName(tl, bi.name);

                  // for each template name we inherit from, we need to
                  // substitute the formal with the actual arguments

                  QHash<QString, int> actualTemplateNames;

                  for (int index : templateNames) {

                     if (index < templArgs.count()) {
                        const Argument &tmpArg = templArgs.at(index);

                        if (baseClassNames.contains(tmpArg.type) && ! actualTemplateNames.contains(tmpArg.type)) {
                           actualTemplateNames.insert(tmpArg.type, index);
                        }
                     }
                  }

                  tbi.name = substituteTemplateArgumentsInString(bi.name, tl, templArgs);

                  // find a documented base class in the correct scope
                  if (! findClassRelation(ptrEntry, cd, item.value(), &tbi, actualTemplateNames, DocumentedOnly, false)) {

                     // no documented base class, try to find an undocumented one
                     findClassRelation(ptrEntry, cd, item.value(), &tbi, actualTemplateNames, Undocumented, true);
                  }

               }
            }
         }

         // class has no base classes
      }

   }
}

// compute the references (anchors in HTML) for each function in the file
void Doxy_Work::computeMemberReferences()
{
   for (auto &cd : Doxy_Globals::classSDict) {
      cd->computeAnchors();
   }

   for (auto &fn : Doxy_Globals::inputNameList) {
      for (auto &fd : *fn) {
         fd->computeAnchors();
      }
   }

   for (auto &nd : Doxy_Globals::namespaceSDict) {
      nd->computeAnchors();
   }

   for (auto &gd : Doxy_Globals::groupSDict) {
      gd->computeAnchors();
   }
}

void Doxy_Work::addListReferences()
{
   for (auto mn : Doxy_Globals::memberNameSDict) {
      for (auto md : *mn) {
         md->visited = false;
      }
   }

   for (auto &mn : Doxy_Globals::functionNameSDict) {
     for (auto md : *mn) {
         md->visited = false;
      }
   }

   for (auto &cd : Doxy_Globals::classSDict) {
      cd->addListReferences();
   }

   for (auto &fn : Doxy_Globals::inputNameList) {
      for (auto fd : *fn) {
         fd->addListReferences();
      }
   }

   for (auto &nd : Doxy_Globals::namespaceSDict) {
      nd->addListReferences();
   }

   for (auto &gd : Doxy_Globals::groupSDict) {
      gd->addListReferences();
   }

   for (auto &pd : Doxy_Globals::pageSDict) {
      QString name = pd->getOutputFileBase();

      if (pd->getGroupDef()) {
         name = pd->getGroupDef()->getOutputFileBase();
      }

      const QVector<ListItemInfo> &xrefItems = pd->getRefItems();
      addRefItem(xrefItems, name, theTranslator->trPage(true, true), name, pd->title(), "", QSharedPointer<Definition>());
   }

   for (auto &dd : Doxy_Globals::directories) {
      QString name = dd->getOutputFileBase();
      const QVector<ListItemInfo> &xrefItems = dd->getRefItems();

      addRefItem(xrefItems, name, theTranslator->trDir(true, true), name, dd->displayName(), "", QSharedPointer<Definition>());
   }
}

void Doxy_Work::generateXRefPages()
{
   for (auto &rl : Doxy_Globals::xrefLists) {
      rl.generatePage();
   }
}

// Copy the documentation in entry `root' to member definition `md' and
// set the function declaration of the member to `funcDecl'.
// If 'overload' is set the standard overload text is added.

void Doxy_Work::addMemberDocs(QSharedPointer<Entry> ptrEntry, QSharedPointer<MemberDef> md, const QString &funcDecl,
                  ArgumentList &argList, bool overload, NamespaceSDict *)
{
   QSharedPointer<Entry> root = ptrEntry->entry();

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
   QSharedPointer<FileDef> rfd = ptrEntry->fileDef();

   // TODO determine scope based on root not md
   QSharedPointer<Definition> rscope = md->getOuterScope();

   ArgumentList &mdArgList = md->getArgumentList();

   if (! argList.listEmpty()) {
      mergeArguments(mdArgList, argList, ! root->getData(EntryKey::Main_Docs).isEmpty());

   } else if (matchArguments2(md->getOuterScope(), md->getFileDef(), mdArgList, rscope, rfd, root->argList, true)) {
      mergeArguments(mdArgList, root->argList, ! root->getData(EntryKey::Main_Docs).isEmpty());

   }

   if (overload) {
      // the \overload keyword was used

      QString doc = theTranslator->trOverloadText();
      if (! root->getData(EntryKey::Main_Docs).isEmpty()) {
         doc += "<p>";
         doc += root->getData(EntryKey::Main_Docs);
      }

      md->setDocumentation(doc, root->getData(EntryKey::MainDocs_File), root->docLine);
      md->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);
      md->setDocsForDefinition(!root->proto);

   } else {
      md->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
      md->setDocsForDefinition(! root->proto);

      md->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);

      if ((md->inbodyDocumentation().isEmpty() || ! ptrEntry->parent()->m_entryName.isEmpty() ) && ! root->getData(EntryKey::Inbody_Docs).isEmpty()) {
         md->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);
      }
   }

   if (md->initializer().isEmpty() && ! root->getData(EntryKey::Initial_Value).isEmpty()) {
      md->setInitializer(root->getData(EntryKey::Initial_Value));
   }

   md->setMaxInitLines(root->initLines);

   if (rfd) {
      if ((md->getStartBodyLine() == -1 && root->startBodyLine != -1)) {
         md->setBodySegment(root->startBodyLine, root->endBodyLine);
         md->setBodyDef(rfd);
      }

      md->setRefItems(root->m_specialLists);
   }

   md->enableCallGraph(md->hasCallGraph() || root->callGraph);
   md->enableCallerGraph(md->hasCallerGraph() || root->callerGraph);

   md->mergeMemberTraits(root->m_traits);
   md->addSectionsToDefinition(root->m_anchors);
   addMemberToGroups(root, md);

   if (cd) {
      cd->insertUsedFile(rfd);
   }

   if (root->mGrpId != -1) {
      if (md->getMemberGroupId() != -1) {
         if (md->getMemberGroupId() != root->mGrpId) {
            warn(root->getData(EntryKey::File_Name), root->startLine,
               "member %s belongs to two different groups. The second "
               "one found here will be ignored.",  csPrintable(md->name()));
         }

      } else { // set group id
         md->setMemberGroupId(root->mGrpId);
      }
   }
}

// Adds the documentation contained in root to a global function with to the corresponding member definition
// name = `name' and argument list `args' (for overloading) and function declaration `decl'

bool Doxy_Work::findGlobalMember(QSharedPointer<Entry> ptrEntry, const QString &namespaceName, const QString &type,
                  const QString &name, const QString &tempArg, const QString &decl)
{
   static const bool useTypedefName = Config::getBool("use-typedef-name");

   QSharedPointer<Entry> root = ptrEntry->entry();

   Debug::print(Debug::FindMembers, 0, "\nDebug: findGlobalMember() namespace= %s   type= %s   name= %s   tempArg= %s   decl= %s\n",
                csPrintable(namespaceName), csPrintable(type), csPrintable(name),
                csPrintable(tempArg), csPrintable(decl) );

   if (name.isEmpty()) {
      return false;
   }

   if (name.indexOf("::") != -1) {
      // skip undefined class members
      return false;
   }

   // look in function dictionary
   QSharedPointer<MemberName> memberList = Doxy_Globals::functionNameSDict.find(name + tempArg);

   if (memberList == nullptr) {
      // try without template arguments
      memberList = Doxy_Globals::functionNameSDict.find(name);
   }

   if (memberList) {
      // function name defined

      Debug::print(Debug::FindMembers, 0, "\nDebug: findGlobalMember() found symbol scope\n");
      bool found = false;

      for (const auto &md : *memberList) {

         QSharedPointer<NamespaceDef> nd = md->getNamespaceDef();
         QSharedPointer<FileDef> fd      = ptrEntry->fileDef();

         const NamespaceSDict *nl = fd ? fd->getUsedNamespaces() : nullptr;

         // search in the list of namespaces that are imported via a using declaration
         bool viaUsingDirective = nl && nd && nl->find(nd->qualifiedName()) != nullptr;

         if ((namespaceName.isEmpty() && nd == nullptr) || (nd && nd->name() == namespaceName) || viaUsingDirective) {
            // not in a namespace, or in the same namespace, memeber in "using' namespace

            Debug::print(Debug::FindMembers, 0, "\nDebug: findGlobalMember() attempting to add member %s to scope %s\n",
                  csPrintable(md->name()), csPrintable(namespaceName));

            QSharedPointer<NamespaceDef> rnd;

            if (! namespaceName.isEmpty()) {
               rnd = Doxy_Globals::namespaceSDict.find(namespaceName);
            }

            const ArgumentList &mdAl = md->getArgumentList();

            bool matching = md->isVariable() || md->isTypedef();

            if (! matching) {
               if (rnd == nullptr) {
                  rnd = Doxy_Globals::globalScope;
               }

               matching = matchArguments2(md->getOuterScope(), md->getFileDef(), mdAl, rnd, fd, root->argList, false);
            }

            // for template members we need to check if the number of template arguments is the same
            // otherwise we are dealing with different functions

            if (matching && ! root->m_templateArgLists.isEmpty()) {

               const ArgumentList &mdTempl = md->getTemplateArgumentList();

               if (root->m_templateArgLists.last().count() != mdTempl.count()) {
                  matching = false;
               }
            }

            // for static members we also check if the comment block was found in the same file.
            // This is needed because static members with the same name can be in different files.
            // It would be incoorect to put the comment block at the first syntactically matching member.

            if (matching && md->isStatic() && md->getDefFileName() != root->getData(EntryKey::File_Name) && memberList->count() > 1) {
               matching = false;
            }

            // for template member, check the return type
            const ArgumentList &tmpList = md->getTemplateArgumentList();

            if (! root->m_templateArgLists.isEmpty()) {

               if (tmpList.count() != root->m_templateArgLists.last().count() || md->typeString() != type) {
                  matching = false;
               }
            }

            if (matching) {
               // add docs to the member
               Debug::print(Debug::FindMembers, 0, "\nDebug: findGlobalMember() match found\n");

               addMemberDocs(ptrEntry, md, decl, root->argList, false);
               found = true;
               break;
            }
         }
      }

      if (! found && root->relatesType != Duplicate && root->section == Entry::FUNCTION_SEC) {
         // no match
         QString signature = decl;
         signature += argListToString(root->argList, true);

         QString warnMsg = QString("No matching member or function found for \n") + substitute(signature, "%", "%%");

         if (memberList->count() > 0) {
            warnMsg += "\nPossible candidates:\n";

            for (const auto &md : *memberList) {
               warnMsg += "  '";
               warnMsg += substitute(md->declaration(), "%", "%%");
               warnMsg += "' at line " + QString::number(md->getDefLine()) + " in file " + md->getDefFileName() + "\n";
            }
         }

         warn(root->getData(EntryKey::File_Name), root->startLine, warnMsg);
      }

   } else {
      // have docs for an undefined member

      if (root->getData(EntryKey::Member_Type) != "friend class" && root->getData(EntryKey::Member_Type) != "friend struct" &&
            root->getData(EntryKey::Member_Type) != "friend union" && root->getData(EntryKey::Member_Type) != "friend" &&
            (! useTypedefName || root->getData(EntryKey::Member_Type).indexOf("typedef ") == -1)) {

         warn(root->getData(EntryKey::File_Name), root->startLine, "documented symbol `%s' was not declared or defined",
             csPrintable(decl) );
      }
   }

   return true;
}

bool Doxy_Work::isSpecialization(const QVector<ArgumentList> &srcTempArgLists, const QVector<ArgumentList> &dstTempArgLists)
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
      result = (! temp->getTemplateArgumentList().listEmpty() || scopeIsTemplate(d->getOuterScope()) );
   }

   return result;
}

QString Doxy_Work::substituteTemplatesInString(const QVector<ArgumentList> &srcTempArgLists, QVector<ArgumentList> &dstTempArgLists,
                  const ArgumentList &funcTemplateArgList, const QString &src)
{
   // can be used to match template specializations
   QString dst;

   static QRegularExpression regExp( "[A-Za-z_][A-Za-z_0-9]*");
   QRegularExpressionMatch match = regExp.match(src);

   QString::const_iterator current_iter = src.constBegin();
   QString::const_iterator start_iter   = src.constBegin();

   while (match.hasMatch()) {
      // for each word in srcType
      start_iter = match.capturedStart();

      dst += QStringView(current_iter, start_iter);
      QString name = match.captured();

      bool found = false;
      auto dstList_iter = dstTempArgLists.begin();

      for (auto &srcList_arg : srcTempArgLists) {

         if (found) {
            break;
         }

         ArgumentList::const_iterator fa_iter = funcTemplateArgList.begin();

         auto tmpDstList_iter = dstList_iter->begin();

         for (auto &arg : srcList_arg) {

            if (found) {
               break;
            }

            if (name == arg.name) {

               if (tmpDstList_iter != dstList_iter->end() && tmpDstList_iter->name.isEmpty()) {
                  int vc = 0;

                  if (tmpDstList_iter->type.startsWith("class ")) {
                     vc = 6;

                  } else if (tmpDstList_iter->type.startsWith("typename ")) {
                     vc = 9;

                  }

                  if (vc > 0) {
                     // convert type=="class T" to type=="class" name=="T"
                     tmpDstList_iter->name = tmpDstList_iter->type.mid(vc);
                     tmpDstList_iter->type = tmpDstList_iter->type.left(vc - 1);
                  }
               }

               if (tmpDstList_iter != dstList_iter->end() && ! tmpDstList_iter->name.isEmpty()) {
                  name  = tmpDstList_iter->name;       // substitute
                  found = true;

               } else if (fa_iter != funcTemplateArgList.end()) {
                  name  = fa_iter->type;
                  found = true;
               }
            }


            if (tmpDstList_iter != dstList_iter->end())  {
               ++tmpDstList_iter;

            } else if (fa_iter != funcTemplateArgList.end()) {
               ++fa_iter;

            }
         }

         if (dstList_iter != dstTempArgLists.end()) {
            ++dstList_iter;
         }
      }

      dst += name;

      current_iter = match.capturedEnd();
      match = regExp.match(src, current_iter);
   }

   dst += QStringView(current_iter, src.constEnd());

   return dst;
}

ArgumentList Doxy_Work::substituteTemplatesInArgList(const QVector<ArgumentList> &srcTempArgLists,
                  QVector<ArgumentList> &dstTempArgLists, const ArgumentList &srcList)
{
   ArgumentList retval;
   const ArgumentList funcTemplateArgList;

   for (auto &sa : srcList) {
      // for each member argument

      QString dstType  = substituteTemplatesInString(srcTempArgLists, dstTempArgLists, funcTemplateArgList, sa.type);
      QString dstArray = substituteTemplatesInString(srcTempArgLists, dstTempArgLists, funcTemplateArgList, sa.array);

      Argument arg = sa;
      arg.type  = dstType;
      arg.array = dstArray;

      retval.append(arg);
   }

   retval.constSpecifier     = srcList.constSpecifier;
   retval.volatileSpecifier  = srcList.volatileSpecifier;
   retval.pureSpecifier      = srcList.pureSpecifier;
   retval.refSpecifier       = srcList.refSpecifier;

   retval.trailingReturnType = substituteTemplatesInString(srcTempArgLists, dstTempArgLists,
                  funcTemplateArgList, srcList.trailingReturnType);

   return retval;
}

/* tries to find a member (in a documented class/file/namespace) which corresponds to the
 * function/variable declaration given in funcDecl.
 *
 * The boolean overloaded is used to specify whether or not a standard overload documentation
 * line should be generated.
 *
 * The boolean isFunc is a hint that indicates this is a function instead of a variable or typedef.
 */
void Doxy_Work::findMember(QSharedPointer<Entry> ptrEntry, QString funcDecl, bool overloaded, bool isFunc)
{
   static const bool hideScopeNames = Config::getBool("hide-scope-names");
   QSharedPointer<Entry> root = ptrEntry->entry();

   int tmpValue = 0;

   if (! root->m_templateArgLists.isEmpty()) {
      tmpValue = root->m_templateArgLists.count();
   }

   Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [start] root= %p   funcDecl= %s   related= %s\n"
                "  overload= %d   isFunc= %d   mGrpId= %d  (# of tArgs= %d)  lang= %x\n",
                root.data(), csPrintable(funcDecl), csPrintable(root->getData(EntryKey::Related_Class)), overloaded, isFunc, root->mGrpId,
                tmpValue, root->m_srcLang);

   QString scopeName;
   QString className;
   QString namespaceName;
   QString exceptions;

   QString funcType;
   QString funcName;
   QString funcArgs;
   QString funcTemplateArgs;
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

   // make sure the first character is a space to simplify searching
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
      // extract information from the declaration in the documentation

      parseFuncDecl(funcDecl, root->m_srcLang == SrcLangExt_ObjC, scopeName, funcType, funcName,
                    funcArgs, funcTemplateArgs, exceptions);
   }

   // class name can also be a namespace, decide later
   // if a related class name is specified and the class name could not
   // be derived from the function declaration then use the related field

   if (! root->getData(EntryKey::Related_Class).isEmpty()) {
      // related member, prefix user specified scope
      isRelated  = true;
      isMemberOf = (root->relatesType == MemberOf);

      if (getClass(root->getData(EntryKey::Related_Class)) == 0 && ! scopeName.isEmpty()) {
         scopeName = mergeScopes(scopeName, root->getData(EntryKey::Related_Class));
      } else {
         scopeName = root->getData(EntryKey::Related_Class);
      }
   }

   if (root->getData(EntryKey::Related_Class).isEmpty() && ptrEntry->parent() &&
         ((ptrEntry->parent()->section & Entry::SCOPE_MASK) ||
          (ptrEntry->parent()->section == Entry::OBJCIMPL_SEC) ) &&
         ! ptrEntry->parent()->m_entryName.isEmpty()) {

      // can we combine scopeName with the scope in which it was found
      QString joinedName = ptrEntry->parent()->m_entryName + "::" + scopeName;

      if (! scopeName.isEmpty() && (getClass(joinedName) || Doxy_Globals::namespaceSDict.find(joinedName))) {
         scopeName = joinedName;

      } else {
         scopeName = mergeScopes(ptrEntry->parent()->m_entryName, scopeName);
      }


   } else {
      // see if we can prefix a namespace or class that is used from the file
      QSharedPointer<FileDef> fd = ptrEntry->fileDef();

      if (fd) {
         const NamespaceSDict *fnl = fd->getUsedNamespaces();

         if (fnl) {
            QString joinedName;

            for (auto fnd : *fnl) {
               joinedName = fnd->name() + "::" + scopeName;

               if (Doxy_Globals::namespaceSDict.find(joinedName)) {
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

   if (! (root->m_templateArgLists.count() > 0 && root->m_templateArgLists.first().count() == 0) ) {
      funcSpec.resize(0);
   }

   // split scope into a namespace and a class part
   extractNamespaceName(scopeName, className, namespaceName, true);

   if (namespaceName.indexOf('@') != -1) {
      // skip stuff in anonymous namespace
      return;
   }

   // merge class and namespace scopes again
   scopeName = "";

   if (! namespaceName.isEmpty()) {
      if (className.isEmpty()) {
         scopeName = namespaceName;

      } else if (! root->getData(EntryKey::Related_Class).isEmpty() || ! getClass(className)) {
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
         tempScopeName = cd->qualifiedNameWithTemplateParameters(root->m_templateArgLists, &argListIndex);
      } else {
         tempScopeName = scopeName + funcSpec;
      }
   }

   // rebuild the function declaration (needed to get the scope right)
   if (! scopeName.isEmpty() && ! isRelated && ! isFriend && ! hideScopeNames) {
      if (! funcType.isEmpty()) {
         if (isFunc) {
            // a function -> we use argList for the arguments
            funcDecl = funcType + " " + tempScopeName + "::" + funcName + funcTemplateArgs;

         } else {
            funcDecl = funcType + " " + tempScopeName + "::" + funcName + funcArgs;

         }

      } else {
         if (isFunc) {
            // a function => we use argList for the arguments
            funcDecl = tempScopeName + "::" + funcName + funcTemplateArgs;

         } else {
            // variable => add 'argument' list
            funcDecl = tempScopeName + "::" + funcName + funcArgs;
         }
      }

   } else {
      // build declaration without scope

      if (! funcType.isEmpty()) {
         // with a type

         if (isFunc) {
            // function => omit argument list
            funcDecl = funcType + " " + funcName + funcTemplateArgs;

         } else {
            // variable => add `argument' list
            funcDecl = funcType + " " + funcName + funcArgs;
         }

      } else {
         // with no type

         if (isFunc) {
            funcDecl = funcName + funcTemplateArgs;
         } else {
            funcDecl = funcName + funcArgs;
         }
      }
   }

   if (funcType == "template class" && ! funcTemplateArgs.isEmpty()) {
      return;   // ignore explicit template instantiations
   }

   Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [parsed] namespaceName= %s   className= %s\n"
                "  funcType= %s   funcSpec= %s   funcName= %s   funcArgs= %s\n"
                "  funcTemplateArgs= %s   funcDecl= %s   related= %s   exceptions= %s\n"
                "  isRelated= %d   isMemberOf= %d   isFriend= %d   isFunc= %d\n",
                csPrintable(namespaceName), csPrintable(className),
                csPrintable(funcType), csPrintable(funcSpec), csPrintable(funcName), csPrintable(funcArgs),
                csPrintable(funcTemplateArgs), csPrintable(funcDecl), csPrintable(root->getData(EntryKey::Related_Class)),
                csPrintable(exceptions), isRelated, isMemberOf, isFriend, isFunc );

   QSharedPointer<MemberName> mn;

   if (! funcName.isEmpty()) {
      // function name is valid

      if (funcName.startsWith("operator ")) {
         // strip class scope from cast operator
         funcName = substitute(funcName, className + "::", "");
      }

      if (! funcTemplateArgs.isEmpty()) {
         // try with member specialization
         mn = Doxy_Globals::memberNameSDict.find(funcName + funcTemplateArgs);
      }

      if (mn == nullptr) {
         // try without specialization
         mn = Doxy_Globals::memberNameSDict.find(funcName);
      }

      if (! isRelated && mn != nullptr) {
         // function name already found

         Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [2] member name exists "
                  "(%d members with this name)\n", mn->count());

         if (! className.isEmpty()) {
            // class name is valid

            if (funcSpec.isEmpty()) {
               // not a member specialization
               int count = 0;

               int noMatchCount = 0;
               bool memFound    = false;

               QSharedPointer<FileDef>      fd = ptrEntry->fileDef();
               QSharedPointer<NamespaceDef> nd;

               if (! namespaceName.isEmpty()) {
                  nd = getResolvedNamespace(namespaceName);
               }

               // find a class def given the scope name and (optionally) a template list specifier
               QSharedPointer<ClassDef> tcd_hold = getResolvedClass(nd, fd, scopeName, nullptr, nullptr, true, true);

               for (auto md : *mn) {

                  if (memFound) {
                     break;
                  }

                  QSharedPointer<ClassDef> cd  = md->getClassDef();
                  QSharedPointer<ClassDef> tcd = tcd_hold;

                  if (tcd == nullptr && cd && stripAnonymousNamespaceScope(cd->name()) == scopeName) {
                     // could be an anonymous scope
                     tcd = cd;
                  }

                  if (cd && tcd == cd) {
                     // found the class this funcDecl belongs to

                     Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [4] class definition: %s\n",
                        csPrintable(cd->name()));

                     // get the template parameter lists found in the member declaration
                     QVector<ArgumentList> declTemplArgs;

                     cd->getTemplateParameterLists(declTemplArgs);
                     const ArgumentList &templAl = md->getTemplateArgumentList();

                     if (! templAl.listEmpty()) {
                        declTemplArgs.append(templAl);
                     }

                     // get the template parameter lists found at the member definition
                     QVector<ArgumentList> &defTemplArgs = root->m_templateArgLists;

                     // do we replace the decl argument lists with the def argument lists?
                     bool substDone = false;
                     ArgumentList argList;

                     // substitute the occurrences of class template names in the argument list before matching
                     const ArgumentList &mdAl = md->getArgumentList();

                     if (declTemplArgs.count() > 0 && declTemplArgs.count() == defTemplArgs.count() && ! mdAl.listEmpty()) {

                        // the function definition has template arguments and the class definition
                        // also has template arguments. must substitute the template names of
                        // the class with those of the function definition before matching.

                        argList   = substituteTemplatesInArgList(declTemplArgs, defTemplArgs, mdAl);
                        substDone = true;

                     } else {
                        // no template arguments, compare argument lists directly
                        argList = mdAl;
                     }

                     Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [5] matching %s with %s   className = %s "
                           "namespaceName= %s\n", csPrintable(argListToString(argList, true)),
                           csPrintable(argListToString(root->argList, true)),
                           csPrintable(className), csPrintable(namespaceName) );

                     bool matching = md->isVariable() || md->isTypedef();

                     if (! matching ) {

                        if (md->isProperty() && root->mtype == MethodType::Property)  {
                           // current source is for a property and docs are for a property

                           matching = true;

                        } else if (md->isProperty() || root->mtype == MethodType::Property)  {
                           // either current source or the docs are for a property

                           // matching remains false

                        } else if (matchArguments2(md->getClassDef(), md->getFileDef(), argList,
                              cd, fd, root->argList, true)) {
                           matching = true;
                        }
                     }

                     if (md->getLanguage() == SrcLangExt_ObjC && md->isVariable() && (root->section & Entry::FUNCTION_SEC)) {
                        // do not match methods and attributes with the same name
                        matching = false;
                     }

                     // for template member check the return type
                     if (! md->getTemplateArgumentList().listEmpty() && ! root->m_templateArgLists.isEmpty()) {
                        // return type from the source
                        QString memType = md->typeString();

                        memType  = stripPrefix(memType, "static ");
                        funcType = substitute(stripTemplateSpecifiersFromScope(funcType, true), className + "::", "");
                        memType  = substitute(stripTemplateSpecifiersFromScope(memType, true), className+"::","");

                        Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [6] comparing return types %s with %s"
                                     "#args %d with %d\n", csPrintable(md->typeString()), csPrintable(funcType),
                                     md->getTemplateArgumentList().count(), root->m_templateArgLists.last().count());

                        if (md->getTemplateArgumentList().count() != root->m_templateArgLists.last().count() ||
                                 memType != funcType) {
                           matching = false;
                        }

                     } else if (matching && ! (md->isVariable() || md->isTypedef() || md->isEnumValue()) ) {
                        // compare the return types

                        // return type from the source
                        QString memType = md->typeString();

                        memType  = stripPrefix(memType, "virtual ");
                        memType  = stripPrefix(memType, "explicit ");
                        memType  = stripPrefix(memType, "friend ");
                        memType  = stripPrefix(memType, "static ");

                        if (memType == "class") {
                           memType = "";
                        }

                        if (memType == "virtual") {
                           memType = "";
                        }

                        if (memType == "explicit") {
                           memType = "";
                        }

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

                        if (memType != funcType && memType != "auto" && funcType != "auto") {

                           warn(root->getData(EntryKey::File_Name), root->startLine,
                              "Return types do not agree for member: %s\n"
                              "   return type (source): %s   return type (docs): %s\n",
                              csPrintable(funcName + funcArgs), csPrintable(memType), csPrintable(funcType));
                        }
                     }

                     bool rootIsUserDoc       = (root->section & Entry::MEMBERDOC_SEC) != 0;
                     bool classIsTemplate     = scopeIsTemplate(md->getClassDef());
                     bool mdIsTemplate        = ! md->getTemplateArgumentList().listEmpty();
                     bool classOrMdIsTemplate = mdIsTemplate || classIsTemplate;
                     bool rootIsTemplate      = ! root->m_templateArgLists.isEmpty();

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
                           // replace members argument list
                           md->setTemplateParameterLists(root->m_templateArgLists);
                           md->setArgumentList(argList);

                        } else {
                           // no match

                           if (! funcTemplateArgs.isEmpty() && isSpecialization(declTemplArgs, defTemplArgs)) {
                              // check if we are dealing with a partial template specialization.
                              // In this case we add it to the class even though the member arguments do not match.

                              // TODO: copy other aspects?
                              root->protection = md->protection();

                              addMethodToClass(ptrEntry, cd, md->name(), isFriend);
                              return;
                           }
                        }
                     }

                     if (matching) {
                        ArgumentList tmp;
                        addMemberDocs(ptrEntry, md, funcDecl, tmp, overloaded, nullptr);

                        count++;
                        memFound = true;
                     }

                  } else if (cd && cd != tcd) {
                     // searching for funcDecl, found a different class which has a method of the same name
                     noMatchCount++;

                  }

                  // end of main for loop
               }

               if (count == 0 && ptrEntry->parent() && ptrEntry->parent()->section == Entry::OBJCIMPL_SEC) {
                  goto localObjCMethod;
               }

               if (count == 0 && ! (isFriend && funcType == "class")) {
                  int candidates = 0;

                  QSharedPointer<ClassDef> exact_cd;
                  QSharedPointer<ClassDef> maybe_cd;

                  QSharedPointer<MemberDef> exact_md;
                  QSharedPointer<MemberDef> maybe_md;

                  if (mn->count() > 0) {
                     // compare this method to every other one with the same name

                     for (auto md : *mn) {
                        QSharedPointer<ClassDef> ccd  = md->getClassDef();
                        QSharedPointer<MemberDef> cmd = md;

                        if (ccd != nullptr && rightScopeMatch(ccd->name(), className)) {

                           const ArgumentList &md_tArgList = md->getTemplateArgumentList();

                           if (! root->m_templateArgLists.isEmpty() && ! md_tArgList.listEmpty()) {

                              if (root->m_templateArgLists.last().count() <= md_tArgList.count()) {
                                 addMethodToClass(ptrEntry, ccd, md->name(), isFriend);
                                 return;
                              }
                           }

                           if (matchArguments2(md->getClassDef(), md->getFileDef(), md->getArgumentList(),
                                      ccd, ptrEntry->fileDef(), ptrEntry->argList, true)) {

                              // exact argument list match

                              exact_cd = ccd;
                              maybe_cd = ccd;

                              exact_md = cmd;
                              maybe_md = cmd;

                              candidates++;
                              break;

                           } else {
                              // arguments do not match exactly, but class and member name match
                              maybe_cd = ccd;
                              maybe_md = cmd;

                           }

                           candidates++;
                        }
                     }
                  }

                  static const bool strictSigMatching = Config::getBool("strict-sig-matching");

                  if (! strictSigMatching) {

                     if (candidates == 1 && maybe_cd != nullptr && maybe_md != nullptr) {
                        // did not find a full signature match on arguments, however there is only one member
                        // with this name in the scope, use the found match

                        ArgumentList tmp;
                        addMemberDocs(ptrEntry, maybe_md, funcDecl, tmp, overloaded, nullptr);
                        return;

                     } else if (candidates > 1 && exact_cd != nullptr && exact_md != nullptr) {
                        // did not find a unique match using type resolution however one of the matches
                        // has the exact same signature, use it

                        ArgumentList tmp;
                        addMemberDocs(ptrEntry, exact_md, funcDecl, tmp, overloaded, nullptr);
                        return;
                     }
                  }

                  QString warnMsg;

                  if (noMatchCount < 1) {
                     warnMsg = "documentation found, for a class member which was not found: \n";

                  } else {
                     if (root->m_templateArgLists.isEmpty()) {
                        warnMsg = "documentation found, for a class member which was not found: \n";
                     } else {
                        warnMsg = "no class member or template specialization found for: \n";
                     }

                     // might be docs with no matching method, function, or class
                     // might be a template specialization of a method with no specialization of the class
                  }

                  for (auto &al : root->m_templateArgLists) {
                     warnMsg += "  template ";
                     warnMsg += tempArgListToString(al, root->m_srcLang);
                     warnMsg += '\n';
                  }

                  QString fullFuncDecl = funcDecl;

                  if (isFunc) {
                     fullFuncDecl += argListToString(root->argList, false);
                  }

                  warnMsg += "  ";
                  warnMsg += fullFuncDecl;
                  warnMsg += '\n';

                  if (candidates > 0) {
                     warnMsg += "Possible candidates:\n";

                     for (auto md : *mn) {
                        QSharedPointer<ClassDef> cd = md->getClassDef();

                        if (cd != 0 && rightScopeMatch(cd->name(), className)) {
                           const ArgumentList &templAl = md->getTemplateArgumentList();

                           if (! templAl.listEmpty()) {
                              warnMsg += "  'template ";
                              warnMsg += tempArgListToString(templAl, root->m_srcLang);
                              warnMsg += '\n';
                           }

                           warnMsg += "  ";
                           if (! md->typeString().isEmpty()) {
                              warnMsg += md->typeString();
                              warnMsg += ' ';
                           }

                           QString qScope = cd->qualifiedNameWithTemplateParameters();
                           if (! qScope.isEmpty()) {
                              warnMsg += qScope + "::" + md->name();
                           }

                           if (! md->argsString().isEmpty()) {
                              warnMsg += md->argsString();
                           }

                           if (noMatchCount > 1) {
                              warnMsg += "' at line " + QString::number(md->getDefLine()) +  " of file " + md->getDefFileName();
                           }

                           warnMsg += '\n';
                        }
                     }
                  }

                  warn_simple(root->getData(EntryKey::File_Name), root->startLine, warnMsg);
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

               MemberDefType memberType = MemberDefType::Function;

               md = QMakeShared<MemberDef>(root->getData(EntryKey::File_Name), root->startLine, root->startColumn, funcType, funcName,
                                  funcArgs, exceptions, declMd ? declMd->protection() : root->protection,
                                  root->virt, root->stat, Member, memberType, ArgumentList(), root->argList);

               md->setTagInfo(ptrEntry->m_tagInfo);
               md->setLanguage(root->m_srcLang);
               md->setId(root->getData(EntryKey::Clang_Id));
               md->setMemberClass(cd);
               md->setTemplateSpecialization(true);
               md->setTypeConstraints(root->typeConstr);
               md->setDefinition(funcDecl);
               md->enableCallGraph(root->callGraph);
               md->enableCallerGraph(root->callerGraph);
               md->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
               md->setBriefDescription(root->getData(EntryKey::Brief_Docs),root->getData(EntryKey::Brief_File),root->briefLine);
               md->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);
               md->setDocsForDefinition(! root->proto);
               md->setPrototype(root->proto);
               md->addSectionsToDefinition(root->m_anchors);
               md->setBodySegment(root->startBodyLine, root->endBodyLine);

               QSharedPointer<FileDef> fd = ptrEntry->fileDef();
               md->setBodyDef(fd);
               md->setMemberTraits(root->m_traits);
               md->setMemberGroupId(root->mGrpId);
               mn->append(md);

               cd->insertMember(md);
               md->setRefItems(root->m_specialLists);
            }

         } else if (overloaded) {
            // check if the function belongs to only one class
            // for unique overloaded member we allow the class to be omitted
            // using this should however be avoided, because it is error prone

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
               MemberDefType memberType;

               if (root->mtype == MethodType::Signal) {
                  memberType = MemberDefType::Signal;

               } else if (root->mtype == MethodType::Slot) {
                  memberType = MemberDefType::Slot;

               } else if (root->mtype == MethodType::DCOP) {
                  memberType = MemberDefType::DCOP;

               } else {
                  memberType = MemberDefType::Function;

               }

               // new overloaded member function
               const ArgumentList &tArgList = getTemplateArgumentsFromName(cd->name() + "::" +
                     funcName, root->m_templateArgLists);

               QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(root->getData(EntryKey::File_Name), root->startLine, root->startColumn,
                     funcType, funcName, funcArgs, exceptions, root->protection, root->virt, root->stat,
                     Relationship::Related, memberType, tArgList, root->argList);

               md->setTagInfo(ptrEntry->m_tagInfo);
               md->setLanguage(root->m_srcLang);
               md->setId(root->getData(EntryKey::Clang_Id));
               md->setTypeConstraints(root->typeConstr);
               md->setMemberClass(cd);
               md->setDefinition(funcDecl);
               md->enableCallGraph(root->callGraph);
               md->enableCallerGraph(root->callerGraph);

               QString doc = theTranslator->trOverloadText();
               doc += "<p>";
               doc += root->getData(EntryKey::Main_Docs);

               md->setDocumentation(doc, root->getData(EntryKey::MainDocs_File), root->docLine);
               md->setBriefDescription(root->getData(EntryKey::Brief_Docs),root->getData(EntryKey::Brief_File),root->briefLine);
               md->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);

               md->setDocsForDefinition(! root->proto);
               md->setPrototype(root->proto);
               md->addSectionsToDefinition(root->m_anchors);
               md->setBodySegment(root->startBodyLine, root->endBodyLine);

               QSharedPointer<FileDef> fd = ptrEntry->fileDef();
               md->setBodyDef(fd);
               md->setMemberTraits(root->m_traits);
               md->setMemberGroupId(root->mGrpId);
               mn->append(md);
               cd->insertMember(md);
               cd->insertUsedFile(fd);
               md->setRefItems(root->m_specialLists);
            }

         } else {
            // unrelated function with the same name as a member

            if (! findGlobalMember(ptrEntry, namespaceName, funcType, funcName, funcTemplateArgs, funcDecl)) {
               QString fullFuncDecl = funcDecl;

               if (isFunc) {
                  fullFuncDecl += argListToString(root->argList, true);
               }

               warn(root->getData(EntryKey::File_Name), root->startLine, "Can not determine class for function\n%s", csPrintable(fullFuncDecl));
            }
         }

      } else if (isRelated && !  root->getData(EntryKey::Related_Class).isEmpty()) {
         Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [10] related function"
                      "   scopeName= %s   className= %s\n", csPrintable(scopeName), csPrintable(className));

         if (className.isEmpty()) {
            className = root->getData(EntryKey::Related_Class);
         }

         QSharedPointer<ClassDef> cd;

         if ((cd = getClass(scopeName))) {
            bool newMember = true;          // assume we have a new member
            bool newMemberName = false;

            QSharedPointer<MemberDef> mdDefine;
            bool isDefine = false;

            {
               QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict.find(funcName);

               if (mn) {
                  auto iter = mn->begin();
                  mdDefine = *iter;

                  while (mdDefine && !isDefine) {
                     isDefine = isDefine || mdDefine->isDefine();

                     if (! isDefine) {
                        ++iter;

                        if (iter == mn->end()) {
                           break;
                        }

                        mdDefine = *iter;
                     }
                  }
               }
            }

            QSharedPointer<FileDef> fd = ptrEntry->fileDef();

            if ((mn = Doxy_Globals::memberNameSDict.find(funcName)) == 0) {
               mn = QMakeShared<MemberName>(funcName);
               newMemberName = true;        // create a new member name

            } else {
               auto iter = mn->begin();
               QSharedPointer<MemberDef> rmd;

               while ((rmd = *iter) && newMember) {
                  // see if we have another member with matching arguments
                  const ArgumentList &rmdAl = rmd->getArgumentList();

                  newMember = className != rmd->getOuterScope()->name() ||
                     ! matchArguments2(rmd->getOuterScope(), rmd->getFileDef(), rmdAl, cd, fd, root->argList, true);

                  if (newMember) {
                     ++iter;

                     if (iter == mn->end()) {
                        break;
                     }
                  }
               }

               if (! newMember && rmd) {
                  // member already exists as rmd -> add docs

                  ArgumentList tmp;
                  addMemberDocs(ptrEntry, rmd, funcDecl, tmp, overloaded);
               }
            }

            if (newMember) {
               // need to create a new member
               MemberDefType memberType;

               if (isDefine) {
                  memberType = MemberDefType::Define;

               } else if (root->mtype == MethodType::Signal) {
                  memberType = MemberDefType::Signal;

               } else if (root->mtype == MethodType::Slot) {
                  memberType = MemberDefType::Slot;

               } else if (root->mtype == MethodType::DCOP) {
                  memberType = MemberDefType::DCOP;

               } else {
                  memberType = MemberDefType::Function;
               }

               if (isDefine && mdDefine) {
                  mdDefine->setHidden(true);
                  funcType = "#define";
                  funcArgs = mdDefine->argsString();
                  funcDecl = funcType + " " + funcName;
               }

               //   (root->m_templateArgLists ? root->m_templateArgLists->last() : ArgumentList<QList>() )
               // for the template arguments for the new "member."
               // this accurately reflects the template arguments of the related function,
               // which do not have to do with those of the related class.

               ArgumentList tmpArgList1;

               if (! root->m_templateArgLists.isEmpty()) {
                  tmpArgList1 = root->m_templateArgLists.last();
               }

               ArgumentList tmpArgList2;

               if (! funcArgs.isEmpty())   {
                  tmpArgList2 = root->argList;
               }

               Relationship tRelation;

               if (isFriend) {
                  tRelation = Relationship::Friend;

               } else if (isMemberOf) {
                  tRelation = Relationship::Foreign;

               } else {
                  tRelation = Relationship::Related;

               }

               QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(root->getData(EntryKey::File_Name), root->startLine,
                     root->startColumn, funcType, funcName, funcArgs, exceptions, root->protection, root->virt,
                     root->stat && ! isMemberOf, tRelation, memberType, tmpArgList1, tmpArgList2);

               if (isDefine && mdDefine) {
                  md->setInitializer(mdDefine->initializer());
               }

               // MemberDef::writeDocumentation() writes the template argument list for the class
               // as if this member is a member of the class. This method has a special mechanism that allows
               // us to totally override the set of template argument lists that are printed.  We use that and
               // set it to the template argument lists of the related function.

               md->setTemplateParameterLists(root->m_templateArgLists);
               md->setTagInfo(ptrEntry->m_tagInfo);

               // try to find the matching line number of the body from the global function list
               bool found = false;

               if (root->startBodyLine == -1) {
                  QSharedPointer<MemberName> rmn = Doxy_Globals::functionNameSDict.find(funcName);

                  if (rmn) {
                     auto iter = rmn->begin();
                     QSharedPointer<MemberDef> rmd;

                     while ((rmd = *iter) && ! found) {
                        // check if we have another member with matching arguments
                        const ArgumentList &rmdAl = rmd->getArgumentList();

                        // check for matching argument lists
                        if ( matchArguments2(rmd->getOuterScope(), rmd->getFileDef(), rmdAl, cd, fd, root->argList, true)) {
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

                  md->setBodySegment(root->startBodyLine, root->endBodyLine);
                  md->setBodyDef(fd);
               }

               md->setMemberClass(cd);
               md->setMemberTraits(root->m_traits);
               md->setDefinition(funcDecl);
               md->enableCallGraph(root->callGraph);
               md->enableCallerGraph(root->callerGraph);
               md->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
               md->setBriefDescription(root->getData(EntryKey::Brief_Docs),root->getData(EntryKey::Brief_File),root->briefLine);
               md->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);
               md->setDocsForDefinition(!root->proto);
               md->setPrototype(root->proto);
               md->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
               md->addSectionsToDefinition(root->m_anchors);
               md->setMemberGroupId(root->mGrpId);
               md->setLanguage(root->m_srcLang);
               md->setId(root->getData(EntryKey::Clang_Id));

               // md->setMemberDefTemplateArguments(root->mtArgList);
               mn->append(md);
               cd->insertMember(md);
               cd->insertUsedFile(fd);
               md->setRefItems(root->m_specialLists);

               if (root->relatesType == Duplicate) {
                  md->setRelatedAlso(cd);
               }

               if (! isDefine) {
                  addMemberToGroups(root, md);
               }

               if (newMemberName) {
                  Doxy_Globals::memberNameSDict.insert(funcName, mn);
               }
            }

            if (root->relatesType == Duplicate) {

               if (! findGlobalMember(ptrEntry, namespaceName, funcType, funcName, funcTemplateArgs, funcDecl)) {
                  QString fullFuncDecl = funcDecl;

                  if (isFunc) {
                     fullFuncDecl += argListToString(root->argList, true);
                  }

                  warn(root->getData(EntryKey::File_Name), root->startLine,
                       "Can not determine file/namespace for related also function\n%s", csPrintable(fullFuncDecl));
               }
            }

         } else {
            warn_undoc(root->getData(EntryKey::File_Name), root->startLine, "Class `%s' for related function `%s' is not "
                       "documented.", csPrintable(className), csPrintable(funcName));
         }

      } else if (ptrEntry->parent() && ptrEntry->parent()->section == Entry::OBJCIMPL_SEC) {

      localObjCMethod:
         QSharedPointer<ClassDef> cd;

         if (Config::getBool("extract-local-methods") && (cd = getClass(scopeName))) {
            Debug::print(Debug::FindMembers, 0, "\nDebug: findMember() [11]  method %s"
                         "   scopeName= %s   className= %s\n", csPrintable(root->m_entryName),
                         csPrintable(scopeName), csPrintable(className));

            QSharedPointer<MemberDef>md = QMakeShared<MemberDef>(root->getData(EntryKey::File_Name), root->startLine,
               root->startColumn, funcType, funcName, funcArgs, exceptions, root->protection, root->virt, root->stat, Member,
               MemberDefType::Function, ArgumentList(), root->argList);

            md->setTagInfo(ptrEntry->m_tagInfo);
            md->setLanguage(root->m_srcLang);
            md->setId(root->getData(EntryKey::Clang_Id));
            md->makeImplementationDetail();
            md->setMemberClass(cd);
            md->setDefinition(funcDecl);
            md->enableCallGraph(root->callGraph);
            md->enableCallerGraph(root->callerGraph);
            md->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
            md->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
            md->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);
            md->setDocsForDefinition(!root->proto);
            md->setPrototype(root->proto);
            md->addSectionsToDefinition(root->m_anchors);
            md->setBodySegment(root->startBodyLine, root->endBodyLine);

            QSharedPointer<FileDef> fd = ptrEntry->fileDef();
            md->setBodyDef(fd);
            md->setMemberTraits(root->m_traits);
            md->setMemberGroupId(root->mGrpId);
            cd->insertMember(md);
            cd->insertUsedFile(fd);
            md->setRefItems(root->m_specialLists);

            mn = Doxy_Globals::memberNameSDict.find(root->m_entryName);

            if (mn) {
               mn->append(md);

            } else {
               mn = QMakeShared<MemberName>(root->m_entryName);
               mn->append(md);
               Doxy_Globals::memberNameSDict.insert(root->m_entryName, mn);
            }

         } else {
            // local objective C method found for class without interface
         }

      } else {
         // unrelated not overloaded member found

         bool globMem = findGlobalMember(ptrEntry, namespaceName, funcType, funcName, funcTemplateArgs, funcDecl);

         if (className.isEmpty() && !globMem) {
            warn(root->getData(EntryKey::File_Name), root->startLine, "Class for member `%s' can not "
                 "be found.",  csPrintable(funcName) );

         } else if (!className.isEmpty() && ! globMem) {
            warn(root->getData(EntryKey::File_Name), root->startLine, "Member `%s' of class `%s' can not be found",
                  csPrintable(funcName),  csPrintable(className));
         }
      }

   } else {
      // this should not be called
      warn(root->getData(EntryKey::File_Name), root->startLine, "Member with no name found (unusual error)");
   }
}

// find the members corresponding to the different documentation blocks
// that are extracted from the sources.
void Doxy_Work::filterMemberDocumentation(QSharedPointer<Entry> ptrEntry)
{
   QSharedPointer<Entry> root = ptrEntry->entry();
   int i = -1;
   int len;

   Debug::print(Debug::FindMembers, 0, "\nDebug: filterMemberDocumentation(): member type)= %s  class name = %s  "
                "   name= %s\n   member args= %s   section= %x   mGrpId= %d\n",
                csPrintable(root->getData(EntryKey::Member_Type)), csPrintable(root->getData(EntryKey::Class_Name)),
                csPrintable(root->m_entryName), csPrintable(root->getData(EntryKey::Member_Args)), root->section, root->mGrpId);

   bool isFunc = true;

   if (root->relatesType == Duplicate && ! root->getData(EntryKey::Related_Class).isEmpty()) {
      QString tmp = root->getData(EntryKey::Related_Class);

      root->setData(EntryKey::Related_Class, "");
      filterMemberDocumentation(ptrEntry);
      root->setData(EntryKey::Related_Class, tmp);
   }

   // detect func variable/typedef to func ptr
   if ( (i = findFunctionPtr(root->getData(EntryKey::Member_Type), root->m_srcLang, &len)) != -1) {
      // fix type and argument
      QString tmpType = root->getData(EntryKey::Member_Type);

      root->prependData(EntryKey::Member_Args,  tmpType.right(tmpType.length() - i - len));
      root->setData(EntryKey::Member_Type,      tmpType.left(i + len));

      isFunc = false;

   } else if (root->getData(EntryKey::Member_Type).startsWith("typedef ") && ! root->getData(EntryKey::Member_Args).contains("(") ) {
      // detect function types marked as functions
      isFunc = false;

   }

   if (root->section == Entry::MEMBERDOC_SEC) {

      if (root->getData(EntryKey::Member_Type).isEmpty()) {
         findMember(ptrEntry, root->m_entryName + root->getData(EntryKey::Member_Args) +
               root->getData(EntryKey::Exception_Spec), false, isFunc);

      } else {
         findMember(ptrEntry, root->getData(EntryKey::Member_Type) + " " + root->m_entryName + root->getData(EntryKey::Member_Args) +
               root->getData(EntryKey::Exception_Spec), false, isFunc);
      }


   } else if (root->section == Entry::OVERLOADDOC_SEC) {
      findMember(ptrEntry, root->m_entryName, true, isFunc);

   } else if ((root->section == Entry::FUNCTION_SEC  || (root->section == Entry::VARIABLE_SEC && ! root->getData(EntryKey::Member_Type).isEmpty() &&
               ! Doxy_Globals::g_compoundKeywordDict.contains(root->getData(EntryKey::Member_Type)) ))) {

      // that is not a keyword
      // to skip forward declaration of class etc.

      if (root->getData(EntryKey::Member_Type) == "friend class" || root->getData(EntryKey::Member_Type) == "friend struct" ||
                  root->getData(EntryKey::Member_Type) == "friend union") {
         findMember(ptrEntry, root->getData(EntryKey::Member_Type) + " " + root->m_entryName, false, false);

      } else if (! root->getData(EntryKey::Member_Type).isEmpty()) {
         findMember(ptrEntry, root->getData(EntryKey::Member_Type) + " " + root->getData(EntryKey::Class_Name) + root->m_entryName +
                  root->getData(EntryKey::Member_Args) + root->getData(EntryKey::Exception_Spec), false, isFunc);

      } else {
         findMember(ptrEntry, root->getData(EntryKey::Class_Name) + root->m_entryName +
                  root->getData(EntryKey::Member_Args) + root->getData(EntryKey::Exception_Spec), false, isFunc);
      }

   } else if (root->section == Entry::DEFINE_SEC && ! root->getData(EntryKey::Related_Class).isEmpty()) {
      findMember(ptrEntry, root->m_entryName + root->getData(EntryKey::Member_Args), false, ! root->getData(EntryKey::Member_Args).isEmpty());

   } else if (root->section == Entry::VARIABLEDOC_SEC) {
      findMember(ptrEntry, root->m_entryName, false, false);

   } else if (root->section == Entry::EXPORTED_INTERFACE_SEC || root->section == Entry::INCLUDED_SERVICE_SEC) {
      findMember(ptrEntry, root->getData(EntryKey::Member_Type) + " " + root->m_entryName, false, false);

   } else {
      // skip section

   }
}

void Doxy_Work::findMemberDocumentation(QSharedPointer<Entry> ptrEntry)
{
   if (ptrEntry->section == Entry::MEMBERDOC_SEC   || ptrEntry->section == Entry::OVERLOADDOC_SEC ||
       ptrEntry->section == Entry::FUNCTION_SEC    || ptrEntry->section == Entry::VARIABLE_SEC ||
       ptrEntry->section == Entry::VARIABLEDOC_SEC || ptrEntry->section == Entry::DEFINE_SEC ||
       ptrEntry->section == Entry::INCLUDED_SERVICE_SEC || ptrEntry->section == Entry::EXPORTED_INTERFACE_SEC ) {

      filterMemberDocumentation(ptrEntry);
   }

   for (auto e : ptrEntry->children() ) {
      if (e->section != Entry::ENUM_SEC) {
         findMemberDocumentation(e);
      }
   }
}

void Doxy_Work::findObjCMethodDefinitions(QSharedPointer<Entry> ptrEntry)
{
   for (auto objCImplNav : ptrEntry->children() ) {
      if (objCImplNav->section == Entry::OBJCIMPL_SEC) {

         for (auto objCMethodNav : objCImplNav->children() ) {

            if (objCMethodNav->section == Entry::FUNCTION_SEC) {

               auto temp = objCMethodNav;

               QSharedPointer<Entry> objCMethod = temp->entry();

               findMember(temp, objCMethod->getData(EntryKey::Member_Type) + " " + objCImplNav->m_entryName + "::" +
                          objCMethod->m_entryName + " " + objCMethod->getData(EntryKey::Member_Args), false, true);

               objCMethod->section = Entry::EMPTY_SEC;
            }
         }
      }
   }
}

// find and add the enumeration to their classes, namespaces, or files
void Doxy_Work::findEnums(QSharedPointer<Entry> ptrEntry)
{
   static const bool hideScopeNames = Config::getBool("hide-scope-names");

   if (ptrEntry->section == Entry::ENUM_SEC) {
      QSharedPointer<Entry> root = ptrEntry->entry();

      QSharedPointer<MemberDef>    md;
      QSharedPointer<ClassDef>     cd;
      QSharedPointer<FileDef>      fd;
      QSharedPointer<NamespaceDef> nd;

      MemberNameSDict *mnsd = nullptr;

      bool isGlobal;
      bool isRelated  = false;
      bool isMemberOf = false;

      int i;

      QString name;
      QString scope;

      if ((i = root->m_entryName.lastIndexOf("::")) != -1) {
         // scope is specified

         scope = root->m_entryName.left(i);                                    // extract scope
         name  = root->m_entryName.right(root->m_entryName.length() - i - 2);  // extract name

         cd = getClass(scope);

         if (cd == nullptr) {
            nd = getResolvedNamespace(scope);
         }

      } else {
         // no scope, check the scope in which the docs where found

         if (( ptrEntry->parent()->section & Entry::SCOPE_MASK ) && !ptrEntry->parent()->m_entryName.isEmpty()) {
            // found enum docs inside a compound
            scope = ptrEntry->parent()->m_entryName;

            cd = getClass(scope);

            if (cd == nullptr) {
               nd = getResolvedNamespace(scope);
            }
         }

         name = root->m_entryName;
      }

      if (! root->getData(EntryKey::Related_Class).isEmpty()) {
         // related member, prefix user specified scope

         isRelated = true;
         isMemberOf = (root->relatesType == MemberOf);

         if (getClass(root->getData(EntryKey::Related_Class)) == nullptr && ! scope.isEmpty()) {
            scope = mergeScopes(scope, root->getData(EntryKey::Related_Class));

         } else {
            scope = root->getData(EntryKey::Related_Class);
         }

         if ((cd = getClass(scope)) == nullptr) {
            nd = getResolvedNamespace(scope);
         }
      }

      if (cd && ! name.isEmpty()) {
         // found an enum inside a compound

         fd   = QSharedPointer<FileDef>();
         mnsd = &Doxy_Globals::memberNameSDict;
         isGlobal = false;

      } else if (nd && ! nd->name().isEmpty() && nd->name().at(0) != '@') {
         // found enum inside namespace

         mnsd = &Doxy_Globals::functionNameSDict;
         isGlobal = true;

      } else {
         // found a global enum

         fd = ptrEntry->fileDef();
         mnsd = &Doxy_Globals::functionNameSDict;
         isGlobal = true;
      }

      if (! name.isEmpty()) {
         // new enum type

         md = QMakeShared<MemberDef>(root->getData(EntryKey::File_Name), root->startLine, root->startColumn,
                        "", name, "", "", root->protection, Normal, false, isMemberOf ? Foreign : isRelated ? Related : Member,
                        MemberDefType::Enumeration, ArgumentList(), ArgumentList());

         md->setTagInfo(ptrEntry->m_tagInfo);
         md->setLanguage(root->m_srcLang);
         md->setId(root->getData(EntryKey::Clang_Id));

         if (isGlobal) {
            md->setFileDef(fd);
         } else {
            md->setMemberClass(cd);
         }

         md->setBodySegment(root->startBodyLine, root->endBodyLine);
         md->setBodyDef(ptrEntry->fileDef());
         md->setMemberTraits(root->m_traits);
         md->setEnumBaseType(root->getData(EntryKey::Member_Args));

         md->addSectionsToDefinition(root->m_anchors);
         md->setMemberGroupId(root->mGrpId);
         md->enableCallGraph(root->callGraph);
         md->enableCallerGraph(root->callerGraph);

         md->setRefItems(root->m_specialLists);

         bool defSet = false;

         QString baseType = root->getData(EntryKey::Member_Args);
         if (! baseType.isEmpty()) {
            baseType.prepend(" : ");
         }

         if (nd && ! nd->name().isEmpty() && nd->name().at(0) != '@') {

            if (isRelated || hideScopeNames) {
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

            if (fd == nullptr && ptrEntry->parent()) {
               fd = ptrEntry->parent()->fileDef();
            }

            if (fd) {
               md->setFileDef(fd);
               fd->insertMember(md);
            }

         } else if (cd) {
            if (isRelated || hideScopeNames) {
               md->setDefinition(name + baseType);

            } else {
               md->setDefinition(cd->name() + "::" + name + baseType);
            }

            cd->insertMember(md);
            cd->insertUsedFile(fd);
         }

         md->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
         md->setDocsForDefinition(!root->proto);
         md->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
         md->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);

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

   } else {
      RECURSE_ENTRYTREE(findEnums, ptrEntry);

   }
}

void Doxy_Work::addEnumValuesToEnums(QSharedPointer<Entry> ptrEntry)
{
   if (ptrEntry->section == Entry::ENUM_SEC) {
      // non anonymous enumeration
      QSharedPointer<Entry> root  = ptrEntry;

      QSharedPointer<ClassDef> cd = QSharedPointer<ClassDef>();
      QSharedPointer<FileDef>  fd = QSharedPointer<FileDef>();

      QSharedPointer<NamespaceDef> nd;
      MemberNameSDict *mnsd = nullptr;

      bool isGlobal;
      bool isRelated = false;

      int i;

      QString name;
      QString scope;

      if ((i = root->m_entryName.lastIndexOf("::")) != -1) {
         // scope is specified

         scope = root->m_entryName.left(i);                                      // extract scope
         name  = root->m_entryName.right(root->m_entryName.length() - i - 2);    // extract name
         cd    = getClass(scope);

         if (cd == nullptr) {
            nd = getResolvedNamespace(scope);
         }

      } else {
         // no scope, check the scope in which the docs where found

         if (( ptrEntry->parent()->section & Entry::SCOPE_MASK ) && ! ptrEntry->parent()->m_entryName.isEmpty()) {
            // found enum docs inside a compound
            scope = ptrEntry->parent()->m_entryName;
            cd    = getClass(scope);

            if (cd == nullptr) {
               nd = getResolvedNamespace(scope);
            }
         }

         name = root->m_entryName;
      }

      if (! root->getData(EntryKey::Related_Class).isEmpty()) {
         // related member, prefix user specified scope
         isRelated = true;

         if (getClass(root->getData(EntryKey::Related_Class)) == nullptr && ! scope.isEmpty()) {
            scope = mergeScopes(scope, root->getData(EntryKey::Related_Class));

         } else {
            scope = root->getData(EntryKey::Related_Class);
         }

         if ((cd = getClass(scope)) == nullptr) {
            nd = getResolvedNamespace(scope);
         }
      }

      if (cd && ! name.isEmpty()) {
         // found an enum inside a compound

         fd   = QSharedPointer<FileDef>();
         mnsd = &Doxy_Globals::memberNameSDict;
         isGlobal = false;

      } else if (nd && ! nd->name().isEmpty() && nd->name().at(0) != '@') {
         // found enum inside namespace

         mnsd = &Doxy_Globals::functionNameSDict;
         isGlobal = true;

      } else {
         // found a global enum

         fd   = ptrEntry->fileDef();
         mnsd = &Doxy_Globals::functionNameSDict;
         isGlobal = true;
      }

      if (! name.isEmpty()) {
         QSharedPointer<MemberName> mn = mnsd->find(name);

         // for all members with this name
         if (mn) {

            for (auto md : *mn) {
               // for each enum in this list

               if (md->isEnumerate()) {

                  for (auto e : ptrEntry->children() ) {
                     SrcLangExt sle;

                     bool isStrong = root->m_traits.hasTrait(Entry::Virtue::Strong);

                     if ( (sle = ptrEntry->m_srcLang) == SrcLangExt_CSharp || sle == SrcLangExt_Java ||
                              sle == SrcLangExt_XML || isStrong) {

                        // Unlike C/C++ enums, C++11, C# & Java enum values are only
                        // visible inside the enum scope, must create them here and only add them to the enum

                        QSharedPointer<Entry> root = e->entry();

                        QString qualifiedName = substitute(ptrEntry->m_entryName,"::",".");

                        if (! scope.isEmpty() && ! ptrEntry->m_tagInfo.isEmpty()) {
                           qualifiedName = substitute(scope,"::",".") + "." + qualifiedName;
                        }

                        // TODO: add function to get canonical representation

                        if (substitute(md->qualifiedName(), "::", ".") == qualifiedName) {

                           // enum value scope matches that of the enum
                           // be less strict for tag files as members can have incomplete scope

                           QString fileName = root->getData(EntryKey::File_Name);

                           if (fileName.isEmpty() && ! ptrEntry->m_tagInfo.isEmpty() ) {
                              fileName = ptrEntry->m_tagInfo.tag_Name;
                           }

                           QSharedPointer<MemberDef> fmd = QMakeShared<MemberDef>(fileName, root->startLine,
                              root->startColumn, root->getData(EntryKey::Member_Type), root->m_entryName,
                              root->getData(EntryKey::Member_Args), "", root->protection, Normal, root->stat, Member,
                              MemberDefType::EnumValue, ArgumentList(), ArgumentList());

                           if (md->getClassDef()) {
                              fmd->setMemberClass(md->getClassDef());

                           } else if (md->getNamespaceDef()) {
                              fmd->setNamespace(md->getNamespaceDef());

                           } else if (md->getFileDef()) {
                              fmd->setFileDef(md->getFileDef());

                           }

                           fmd->setOuterScope(md->getOuterScope());
                           fmd->setTagInfo(e->m_tagInfo);
                           fmd->setLanguage(root->m_srcLang);
                           fmd->setId(root->getData(EntryKey::Clang_Id));
                           fmd->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
                           fmd->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
                           fmd->addSectionsToDefinition(root->m_anchors);
                           fmd->setInitializer(root->getData(EntryKey::Initial_Value));
                           fmd->setMaxInitLines(root->initLines);
                           fmd->setMemberGroupId(root->mGrpId);
                           fmd->setExplicitExternal(root->explicitExternal);
                           fmd->setRefItems(root->m_specialLists);
                           fmd->setAnchor();

                           md->insertEnumField(fmd);
                           fmd->setEnumScope(md, true);

                           QSharedPointer<MemberName> mn = mnsd->find(root->m_entryName);

                           if (mn) {
                              mn->append(fmd);

                           } else {
                              mn = QMakeShared<MemberName>(root->m_entryName);
                              mn->append(fmd);
                              mnsd->insert(root->m_entryName, mn);
                           }
                        }

                     } else {
                        // C++

                        QSharedPointer<MemberName> fmn;
                        MemberNameSDict *enumDict = nullptr;

                        if (isRelated) {
                           enumDict = &Doxy_Globals::functionNameSDict;
                        } else {
                           enumDict = mnsd;
                        }

                        QString valueName = e->m_entryName;

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

   } else {
      RECURSE_ENTRYTREE(addEnumValuesToEnums, ptrEntry);
   }
}

// find the documentation blocks for the enumerations
void Doxy_Work::findEnumDocumentation(QSharedPointer<Entry> ptrEntry)
{
   if (ptrEntry->section == Entry::ENUMDOC_SEC && ! ptrEntry->m_entryName.isEmpty() && ptrEntry->m_entryName.at(0) != '@') {
      // skip anonymous enums
      QSharedPointer<Entry> root = ptrEntry->entry();

      int i;
      QString name;
      QString scope;

      if ((i = root->m_entryName.lastIndexOf("::")) != -1) {
         // scope is specified as part of the name

         name  = root->m_entryName.right(root->m_entryName.length() - i - 2); // extract name
         scope = root->m_entryName.left(i); // extract scope

      } else {
         // just the name
         name = root->m_entryName;
      }

      if (( ptrEntry->parent()->section & Entry::SCOPE_MASK ) && ! ptrEntry->parent()->m_entryName.isEmpty()) {
         // found enum docs inside a compound
         if (! scope.isEmpty()) {
            scope.prepend("::");
         }

         scope.prepend(ptrEntry->parent()->m_entryName);
      }

      QSharedPointer<ClassDef> cd = getClass(scope);

      if (! name.isEmpty()) {
         bool found = false;

         if (cd) {
            QString className = cd->name();
            QSharedPointer<MemberName> mn = Doxy_Globals::memberNameSDict.find(name);

            if (mn) {
               for (auto md : *mn) {
                  if (found) {
                     break;
                  }

                  QSharedPointer<ClassDef> cd = md->getClassDef();

                  if (cd && cd->name() == className && md->isEnumerate()) {
                     // documentation outside a compound overrides the documentation inside it

                     md->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
                     md->setDocsForDefinition(! root->proto);

                     // brief descriptions inside a compound override the documentation outside it
                     md->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);

                     if (md->inbodyDocumentation().isEmpty() || ! ptrEntry->parent()->m_entryName.isEmpty()) {
                        md->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);
                     }

                     if (root->mGrpId != -1 && md->getMemberGroupId() == -1) {
                        md->setMemberGroupId(root->mGrpId);
                     }

                     md->addSectionsToDefinition(root->m_anchors);
                     md->setRefItems(root->m_specialLists);

                     QSharedPointer<GroupDef> gd = md->getGroupDef();

                     if (gd == 0 && ! root->m_groups.isEmpty()) {
                        // member not grouped but out-of-line documentation is
                        addMemberToGroups(root, md);
                     }

                     found = true;
                  }
               }
            }

         } else {
            // enum outside class, most likley in a namaspace

            QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict.find(name);

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
                     md->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
                     md->setDocsForDefinition(! root->proto);
                     md->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
                     md->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);
                     md->addSectionsToDefinition(root->m_anchors);
                     md->setMemberGroupId(root->mGrpId);

                     QSharedPointer<GroupDef> gd = md->getGroupDef();

                     if (gd == 0 && ! root->m_groups.isEmpty()) {
                         // member not grouped but out-of-line documentation is
                        addMemberToGroups(root, md);
                     }

                     found = true;
                  }
               }
            }
         }

         if (! found) {
            warn(root->getData(EntryKey::File_Name), root->startLine, "Documentation for undefined enum `%s' found.", csPrintable(name) );
         }
      }

   }

   RECURSE_ENTRYTREE(findEnumDocumentation, ptrEntry);
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
   findDEV(Doxy_Globals::memberNameSDict);
   findDEV(Doxy_Globals::functionNameSDict);
}

void Doxy_Work::addMembersToIndex()
{
   // for each member name
   for (auto mn : Doxy_Globals::memberNameSDict) {

      // for each member definition
      for (auto md : *mn) {
         addClassMemberNameToIndex(md);
      }
   }

   // for each member name
   for (auto mn : Doxy_Globals::functionNameSDict) {

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
   for (auto mn : Doxy_Globals::memberNameSDict) {
      // for each member name

     for (auto md : *mn) {
         // for each member with a specific name

         for (auto bmd : *mn) {
            // for each other member with the same name
            QSharedPointer<ClassDef> mcd  = md->getClassDef();

            if (mcd && mcd->baseClasses()) {
               QSharedPointer<ClassDef> bmcd = bmd->getClassDef();

               if (md != bmd && bmcd && mcd && bmcd != mcd) {
                  auto lang = bmd->getLanguage();

                  if (bmd->virtualness() != Normal || lang == SrcLangExt_Python || lang == SrcLangExt_Java ||
                      lang == SrcLangExt_PHP || bmcd->compoundType() == CompoundType::Interface ||
                      bmcd->compoundType() == CompoundType::Protocol) {

                      if (md->isFunction() && mcd->isLinkable() && bmcd->isLinkable() && mcd->isBaseClass(bmcd, true)) {

                        const ArgumentList &bmdAl = bmd->getArgumentList();
                        const ArgumentList &mdAl  = md->getArgumentList();

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
   }
}

void Doxy_Work::createTemplateInstanceMembersX()
{
   // for each class
   for (auto cd : Doxy_Globals::classSDict) {
      const QHash<QString, QSharedPointer<ClassDef>> &templInstances = cd->getTemplateInstances();

      // for each instance of the template
      for (auto item = templInstances.begin(); item != templInstances.end(); ++item) {
         item.value()->addMembersToTemplateInstance(cd, item.key());
      }
   }
}

void Doxy_Work::mergeCategories()
{
   // merge members of categories into the class they extends
   for (auto cd : Doxy_Globals::classSDict) {
      int i = cd->name().indexOf('(');

      if (i != -1) {
         // it is an Objective-C category
         QString baseName = cd->name().left(i);
         QSharedPointer<ClassDef> baseClass = Doxy_Globals::classSDict.find(baseName);

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
   for (auto cd : Doxy_Globals::classSDict) {

      if (cd->subClasses() == nullptr && cd->baseClasses()) {
         cd->mergeMembers();
      }
   }
}

void Doxy_Work::generateSourceCode()
{
   if (Doxy_Globals::inputNameList.count() > 0) {
      static bool clangParsing = Config::getBool("clang-parsing");

      if (clangParsing) {
         QSet<QString> processedFiles;
         QSet<QString> filesToProcess;

         for (auto &fn : Doxy_Globals::inputNameList) {
            for (auto fd : *fn) {
               filesToProcess.insert(fd->getFilePath());
            }
         }

         // process source files (and their include dependencies)
         for (auto &fn : Doxy_Globals::inputNameList) {

            for (auto fd : *fn) {
               if (fd->isSource() && ! fd->isReference()) {
                  QStringList includeFiles;

                  fd->getAllIncludeFilesRecursively(includeFiles);
                  fd->startParsing();

                  if (fd->generateSourceFile()) {
                     // source needs to be shown in the output

                     msg("Generating code for file %s\n", csPrintable(fd->docName()));
                     fd->writeSource(Doxy_Globals::outputList, false, includeFiles);

                  } else if (! fd->isReference() && Doxy_Globals::parseSourcesNeeded) {
                     // parse the source even if not shown

                     msg("Parsing code for file %s\n", csPrintable(fd->docName()));
                     fd->parseSource(false, includeFiles);
                  }

                  for (QString incFile : includeFiles) {

                     if (fd->getFilePath() != incFile && ! processedFiles.contains(incFile)) {
                        QStringList moreFiles;
                        bool ambig;

                        QSharedPointer<FileDef> ifd = findFileDef(&Doxy_Globals::inputNameDict, incFile, ambig);

                        if (ifd && ! ifd->isReference()) {

                           if (ifd->generateSourceFile()) {
                              // source needs to be shown in the output

                              msg("Generating code for file %s\n", csPrintable(ifd->docName()));
                              ifd->writeSource(Doxy_Globals::outputList, true, moreFiles);

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
         for (auto &fn : Doxy_Globals::inputNameList) {

            for (auto &fd : *fn) {
               if (! processedFiles.contains(fd->getFilePath())) {

                  QStringList includeFiles;
                  fd->startParsing();

                  if (fd->generateSourceFile()) {
                     // source needs to be shown in the output

                     msg("Generating code for file %s\n", csPrintable(fd->docName()));
                     fd->writeSource(Doxy_Globals::outputList, false, includeFiles);

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

         for (auto &fn : Doxy_Globals::inputNameList) {

           for (auto fd : *fn) {
               QStringList includeFiles;
               fd->startParsing();

               if (fd->generateSourceFile()) {
                  // source needs to be shown in the output
                  msg("Generating code for file %s\n", csPrintable(fd->docName()));

                  fd->writeSource(Doxy_Globals::outputList, false, includeFiles);

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

   if (Doxy_Globals::inputNameList.count() > 0) {

      for (auto &fn : Doxy_Globals::inputNameList) {

         for (auto fd : *fn) {
            bool doc = fd->isLinkableInProject();

            if (doc) {
               msg("Generating docs for file %s\n", csPrintable(fd->docName()));
               fd->writeDocumentation(Doxy_Globals::outputList);
            }
         }
      }
   }
}

void Doxy_Work::addSourceReferences()
{
   // add source references for class definitions
   for (auto cd : Doxy_Globals::classSDict) {
      QSharedPointer<FileDef> fd = cd->getBodyDef();

      if (fd && cd->isLinkableInProject() && cd->getStartBodyLine() != -1) {
         fd->addSourceRef(cd->getStartBodyLine(), cd, QSharedPointer<MemberDef>());
      }
   }

   // add source references for namespace definitions
   for (auto &nd : Doxy_Globals::namespaceSDict) {
      QSharedPointer<FileDef> fd = nd->getBodyDef();

      if (fd && nd->isLinkableInProject() && nd->getStartBodyLine() != -1) {
         fd->addSourceRef(nd->getStartBodyLine(), nd, QSharedPointer<MemberDef>());
      }
   }

   // add source references for member names
   for (auto mn: Doxy_Globals::memberNameSDict) {

      for (auto md : *mn) {
         QSharedPointer<FileDef> fd = md->getBodyDef();

         if (fd && md->getStartBodyLine() != -1 && md->isLinkableInProject() &&
               (fd->generateSourceFile() || Doxy_Globals::parseSourcesNeeded) ) {
            fd->addSourceRef(md->getStartBodyLine(), md->getOuterScope(), md);
         }
      }
   }

   for (auto mn : Doxy_Globals::functionNameSDict) {

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

            cd->writeDocumentation(Doxy_Globals::outputList);
            cd->writeMemberList(Doxy_Globals::outputList);
         }

         // even for undocumented classes, the inner classes can be documented.
         cd->writeDocumentationForInnerClasses(Doxy_Globals::outputList);
      }
   }
}

void Doxy_Work::generateClassDocs()
{
   generateClassList(Doxy_Globals::classSDict);
   generateClassList(Doxy_Globals::hiddenClasses);
}

void Doxy_Work::inheritDocumentation()
{
   for (auto mn : Doxy_Globals::memberNameSDict) {
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
   for (auto &fn : Doxy_Globals::inputNameList) {
      for (auto &fd : *fn) {
         fd->visited = false;
      }
   }

   for (auto &fn : Doxy_Globals::inputNameList) {
     for (auto &fd : *fn) {
         fd->combineUsingRelations();
      }
   }

   // for each namespace
   for (auto &nd : Doxy_Globals::namespaceSDict) {
      nd->visited = false;
   }

   for (auto &nd : Doxy_Globals::namespaceSDict) {
      nd->combineUsingRelations();
   }
}

void Doxy_Work::addMembersToMemberGroup()
{
   // for each class
   for (auto &cd : Doxy_Globals::classSDict) {
      cd->addMembersToMemberGroup();
   }

   // for each file
   for (auto &fn : Doxy_Globals::inputNameList) {
      for (auto &fd : *fn) {
         fd->addMembersToMemberGroup();
      }
   }

   // for each namespace
   for (auto &nd : Doxy_Globals::namespaceSDict) {
      nd->addMembersToMemberGroup();
   }

   // for each group
   for (auto &gd : Doxy_Globals::groupSDict) {
      gd->addMembersToMemberGroup();
   }
}

void Doxy_Work::distributeMemberGroupDocumentation()
{
   // for each class
   for (auto cd : Doxy_Globals::classSDict) {
      cd->distributeMemberGroupDocumentation();
   }

   // for each file
   for (auto &fn : Doxy_Globals::inputNameList) {
      for (auto fd : *fn) {
         fd->distributeMemberGroupDocumentation();
      }
   }

   // for each namespace
   for (auto &nd : Doxy_Globals::namespaceSDict) {
      nd->distributeMemberGroupDocumentation();
   }

   // for each group
   for (auto &gd : Doxy_Globals::groupSDict) {
      gd->distributeMemberGroupDocumentation();
   }
}

void Doxy_Work::findSectionsInDocumentation()
{

   // for each class
   for (auto &cd : Doxy_Globals::classSDict) {
      cd->findSectionsInDocumentation();
   }

   // for each file
   for (auto &fn : Doxy_Globals::inputNameList) {
     for (auto fd : *fn) {
         fd->findSectionsInDocumentation();
      }
   }

   // for each namespace
   for (auto &nd : Doxy_Globals::namespaceSDict) {
      nd->findSectionsInDocumentation();
   }

   // for each group
   for (auto gd : Doxy_Globals::groupSDict) {
      gd->findSectionsInDocumentation();
   }

   // for each page
   for (auto &pd : Doxy_Globals::pageSDict) {
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

   Doxy_Globals::lookupCache.clear();

   // remove all cached typedef resolutions whose target is a
   // template class as this may now be a template instance

   for (auto fn : Doxy_Globals::functionNameSDict) {
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

   for (auto fn : Doxy_Globals::memberNameSDict) {
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
   // (which is correct if you ignore the inheritance relation between A and B).
   //
   // class A { class I {} };
   // class B : public A {};
   // class C : public B::I {};

   Doxy_Globals::lookupCache.clear();

   for (auto fn : Doxy_Globals::functionNameSDict) {
      // for each global function name

      for (auto fmd : *fn) {
         // for each function with that name
         fmd->invalidateCachedArgumentTypes();
      }
   }

   for (auto fn : Doxy_Globals::memberNameSDict) {
      // for each class method name

      for (auto fmd : *fn) {
         // for each function with that name
         fmd->invalidateCachedArgumentTypes();
      }
   }
}

void Doxy_Work::findDefineDocumentation(QSharedPointer<Entry> ptrEntry)
{
   if ((ptrEntry->section == Entry::DEFINEDOC_SEC || ptrEntry->section == Entry::DEFINE_SEC) &&
                  ! ptrEntry->m_entryName.isEmpty()) {

      QSharedPointer<Entry> root = ptrEntry->entry();

      if (! ptrEntry->m_tagInfo.isEmpty() && ! root->m_entryName.isEmpty()) {

         // define read from a tag file
         QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(ptrEntry->m_tagInfo.tag_Name,
                  1, 1, "#define", root->m_entryName, root->getData(EntryKey::Member_Args), "",
                  Public, Normal, false, Member, MemberDefType::Define, ArgumentList(), ArgumentList());

         md->setTagInfo(ptrEntry->m_tagInfo);
         md->setLanguage(root->m_srcLang);
         md->setFileDef(ptrEntry->parent()->fileDef());

         QSharedPointer<MemberName> mn;

         if ((mn = Doxy_Globals::functionNameSDict.find(root->m_entryName))) {
            mn->append(md);

         } else {
            mn = QMakeShared<MemberName>(root->m_entryName);

            mn->append(md);
            Doxy_Globals::functionNameSDict.insert(root->m_entryName, mn);
         }
      }

      QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict.find(root->m_entryName);

      if (mn) {
         int count = 0;

         for (auto md : *mn) {
            if (md->memberType() == MemberDefType::Define) {
               count++;
            }
         }

         if (count == 1) {
            for (auto md : *mn) {
               if (md->memberType() == MemberDefType::Define) {
                  md->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
                  md->setDocsForDefinition(! root->proto);
                  md->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);

                  if (md->inbodyDocumentation().isEmpty()) {
                     md->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);
                  }

                  md->setBodySegment(root->startBodyLine, root->endBodyLine);
                  md->setBodyDef(ptrEntry->fileDef());
                  md->addSectionsToDefinition(root->m_anchors);
                  md->setMaxInitLines(root->initLines);
                  md->setRefItems(root->m_specialLists);

                  if (root->mGrpId != -1) {
                     md->setMemberGroupId(root->mGrpId);
                  }

                  addMemberToGroups(root, md);
               }
            }

         } else if (count > 1 && (! root->getData(EntryKey::Main_Docs).isEmpty() || ! root->getData(EntryKey::Brief_Docs).isEmpty() ||
                  root->startBodyLine != -1))

            // multiple defines,  do not know where to add docs
            // but maybe they are in different files together with their documentation

            for (auto md : *mn) {

               if (md->memberType() == MemberDefType::Define) {
                  QSharedPointer<FileDef> fd = md->getFileDef();

                  if (fd && fd->getFilePath() == root->getData(EntryKey::File_Name)) {
                     // doc and define in the same file assume they belong together.

                     md->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
                     md->setDocsForDefinition(!root->proto);
                     md->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);

                     if (md->inbodyDocumentation().isEmpty()) {
                        md->setInbodyDocumentation(root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Inbody_File), root->inbodyLine);
                     }

                     md->setBodySegment(root->startBodyLine, root->endBodyLine);
                     md->setBodyDef(ptrEntry->fileDef());
                     md->addSectionsToDefinition(root->m_anchors);
                     md->setRefItems(root->m_specialLists);
                     md->setLanguage(root->m_srcLang);

                     if (root->mGrpId != -1) {
                        md->setMemberGroupId(root->mGrpId);
                     }

                     addMemberToGroups(root, md);
                  }
               }
            }

      } else if (! root->getData(EntryKey::Main_Docs).isEmpty() || ! root->getData(EntryKey::Brief_Docs).isEmpty()) {
         // define not found
         static bool preEnabled = Config::getBool("enable-preprocessing");

         if (preEnabled) {
            warn(root->getData(EntryKey::File_Name), root->startLine, "documentation for unknown define %s found.\n",
                  csPrintable(root->m_entryName) );

         } else {
            warn(root->getData(EntryKey::File_Name), root->startLine, "found documented #define, ignoring since "
                 "ENABLE PREPROCESSING tag is not set\n",  csPrintable(root->m_entryName) );
         }
      }

   }

   RECURSE_ENTRYTREE(findDefineDocumentation, ptrEntry);
}

void Doxy_Work::findDirDocumentation(QSharedPointer<Entry> ptrEntry)
{
   if (ptrEntry->section == Entry::DIRDOC_SEC) {
      QSharedPointer<Entry> root = ptrEntry->entry();

      QString normalizedName = root->m_entryName;
      normalizedName = substitute(normalizedName, "\\", "/");

      if (root->getData(EntryKey::MainDocs_File) == normalizedName) {
         // current dir?
         int lastSlashPos = normalizedName.lastIndexOf('/');

         if (lastSlashPos != -1) {
            // strip file name
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

               warn(root->getData(EntryKey::File_Name), root->startLine, "\\dir command matches multiple directories.\n"
                    "  Applying the command for directory %s\n"
                    "  Ignoring the command for directory %s\n", csPrintable(matchingDir->name()),
                    csPrintable(dir->name()) );

            } else {
               matchingDir = dir;

            }
         }
      }

      if (matchingDir) {
         matchingDir->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
         matchingDir->setDocumentation(root->getData(EntryKey::Main_Docs), root->getData(EntryKey::MainDocs_File), root->docLine);
         matchingDir->setRefItems(root->m_specialLists);

         addDirToGroups(root, matchingDir);

      } else {
         warn(root->getData(EntryKey::File_Name), root->startLine, "No matching "
              "directory found for command \\dir %s\n",  csPrintable(normalizedName));
      }

   }

   RECURSE_ENTRYTREE(findDirDocumentation, ptrEntry);
}

// create a (sorted) list of separate documentation pages
void Doxy_Work::buildPageList(QSharedPointer<Entry> ptrEntry)
{
   if (ptrEntry->section == Entry::PAGEDOC_SEC) {
      QSharedPointer<Entry> root = ptrEntry->entry();

      if (! root->m_entryName.isEmpty()) {
         addRelatedPage_X(ptrEntry);
      }

   } else if (ptrEntry->section == Entry::MAINPAGEDOC_SEC) {

      QSharedPointer<Entry> root = ptrEntry->entry();

      QString title = root->getData(EntryKey::Member_Args).trimmed();

      if (title.isEmpty()) {
         title = theTranslator->trMainPage();
      }

      QString name = "index";
      addRefItem(root->m_specialLists, name, "page", name, title, "", QSharedPointer<Definition>());
   }

   // recursive call
   for (auto item : ptrEntry->children() ) {
      buildPageList(item);
   }
}

// search for \mainpage defined in this project
void Doxy_Work::findMainPage(QSharedPointer<Entry> ptrEntry)
{
   if (ptrEntry->section == Entry::MAINPAGEDOC_SEC) {

      if (Doxy_Globals::mainPage == nullptr && ptrEntry->m_tagInfo.isEmpty()) {
         QSharedPointer<Entry> root = ptrEntry->entry();

         QString title = root->getData(EntryKey::Member_Args).trimmed();

         QString indexName = "index";
         Doxy_Globals::mainPage = QMakeShared<PageDef>(root->getData(EntryKey::MainDocs_File), root->docLine, indexName,
                  root->getData(EntryKey::Brief_Docs) + root->getData(EntryKey::Main_Docs) +
                  root->getData(EntryKey::Inbody_Docs), title);

         Doxy_Globals::mainPage->setBriefDescription(root->getData(EntryKey::Brief_Docs),
                  root->getData(EntryKey::Brief_File), root->briefLine);
         Doxy_Globals::mainPage->setFileName(indexName);
         Doxy_Globals::mainPage->setLocalToc(root->localToc);

         addPageToContext(Doxy_Globals::mainPage, ptrEntry);

         QSharedPointer<SectionInfo> si = Doxy_Globals::sectionDict.find(Doxy_Globals::mainPage->name());

         if (si) {
            if (si->lineNr != -1) {
               warn(root->getData(EntryKey::File_Name), root->startLine, "multiple use of section label '%s' for main page, "
                  "(first occurrence: %s, line %d)", csPrintable(Doxy_Globals::mainPage->name()),
                   csPrintable(si->fileName), si->lineNr);

            } else {
               warn(root->getData(EntryKey::File_Name), root->startLine,
                    "Multiple use of section label '%s' for main page, (first occurrence: %s)",
                    csPrintable(Doxy_Globals::mainPage->name()), csPrintable(si->fileName));
            }

         } else {
            // page name is a label as well, but should not be double either
            si = QMakeShared<SectionInfo>(indexName, root->startLine, Doxy_Globals::mainPage->name(),
                              Doxy_Globals::mainPage->title(), SectionInfo::Page, 0);

            Doxy_Globals::sectionDict.insert(indexName, si);
            Doxy_Globals::mainPage->addSectionsToDefinition(root->m_anchors);
         }

      } else if (ptrEntry->m_tagInfo.isEmpty()) {
         QSharedPointer<Entry> root = ptrEntry->entry();

         warn(root->getData(EntryKey::File_Name), root->startLine, "Found more than one \\mainpage comment block "
            "(first occurrence: %s, line %d), Skip current block",
            csPrintable(Doxy_Globals::mainPage->docFile()), Doxy_Globals::mainPage->docLine());
      }
   }

   RECURSE_ENTRYTREE(findMainPage, ptrEntry);
}

// search for the main page imported via tag files and add only the section labels
void Doxy_Work::findMainPageTagFiles(QSharedPointer<Entry> ptrEntry)
{
   if (ptrEntry->section == Entry::MAINPAGEDOC_SEC) {

      if (Doxy_Globals::mainPage && ! ptrEntry->m_tagInfo.isEmpty() ) {
         QSharedPointer<Entry> root = ptrEntry->entry();
         Doxy_Globals::mainPage->addSectionsToDefinition(root->m_anchors);
      }
   }

   RECURSE_ENTRYTREE(findMainPageTagFiles, ptrEntry);
}

void Doxy_Work::computePageRelations(QSharedPointer<Entry> ptrEntry)
{
   if ((ptrEntry->section == Entry::PAGEDOC_SEC || ptrEntry->section == Entry::MAINPAGEDOC_SEC)
               && !ptrEntry->m_entryName.isEmpty() ) {


      QSharedPointer<Entry> root = ptrEntry->entry();
      QSharedPointer<PageDef> pd;

      if (root->section == Entry::PAGEDOC_SEC) {
         pd = Doxy_Globals::pageSDict.find(root->m_entryName);
      } else {
         pd = Doxy_Globals::mainPage;
      }

      if (pd) {

         for (auto bi : root->extends) {
            QSharedPointer<PageDef> subPd = Doxy_Globals::pageSDict.find(bi.name);

            if (pd == subPd) {
               err("Page defined at line %d of file %s with label %s is a direct "
                     "subpage of itself. You will need to remove this cyclic dependency.\n",
                     pd->docLine(), csPrintable(pd->docFile()), csPrintable(pd->name()));

               stopDoxyPress();

            } else if (subPd) {
               pd->addInnerCompound(subPd);
            }
         }
      }
   }

   RECURSE_ENTRYTREE(computePageRelations, ptrEntry);
}

void Doxy_Work::checkPageRelations()
{
   for (auto &pd : Doxy_Globals::pageSDict) {
      QSharedPointer<Definition> ppd = pd->getOuterScope();

      while (ppd) {
         if (ppd == pd) {
            err("Page defined at line %d of file %s with label %s is a subpage "
                "of itself. You will need to remove this cyclic dependency.\n",
                pd->docLine(), csPrintable(pd->docFile()), csPrintable(pd->name()));

            stopDoxyPress();
         }

         ppd = ppd->getOuterScope();
      }
   }
}

void Doxy_Work::resolveUserReferences()
{
    for (auto si : Doxy_Globals::sectionDict) {
      QSharedPointer<PageDef> pd;

      // hack: the items of a todo/test/bug/deprecated list are all fragments from different files
      // resulting section's all have the wrong file name (not from the todo/test/bug/deprecated list,
      // but from the file in which they are defined). Correct this here by looking at the generated section labels

      for (auto &rl : Doxy_Globals::xrefLists) {
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

         if (! si->fileName.isEmpty() && (pd = Doxy_Globals::pageSDict.find(si->fileName)) && pd->getGroupDef()) {
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

   for (auto &pd : Doxy_Globals::pageSDict) {

      if (! pd->getGroupDef() && ! pd->isReference()) {
         msg("Generating docs for page %s\n", csPrintable(pd->name()));

         Doxy_Globals::insideMainPage = true;
         pd->writeDocumentation(Doxy_Globals::outputList);
         Doxy_Globals::insideMainPage = false;
      }
   }
}

// create a (sorted) list & dictionary of example pages
void Doxy_Work::buildExampleList(QSharedPointer<Entry> ptrEntry)
{
   if ((ptrEntry->section == Entry::EXAMPLE_SEC || ptrEntry->section == Entry::EXAMPLE_LINENO_SEC) &&
               ! ptrEntry->m_entryName.isEmpty()) {

      QSharedPointer<Entry> root = ptrEntry->entry();

      if (Doxy_Globals::exampleSDict.find(root->m_entryName)) {
         warn(root->getData(EntryKey::File_Name), root->startLine, "Example %s was already documented. Ignoring "
              "documentation found here.", csPrintable(root->m_entryName));

      } else {
         QSharedPointer<PageDef> pd = QMakeShared<PageDef>(root->getData(EntryKey::File_Name), root->startLine,
                  root->m_entryName, root->getData(EntryKey::Brief_Docs) + root->getData(EntryKey::Main_Docs) +
                  root->getData(EntryKey::Inbody_Docs), root->getData(EntryKey::Member_Args));

         pd->setBriefDescription(root->getData(EntryKey::Brief_Docs), root->getData(EntryKey::Brief_File), root->briefLine);
         pd->setFileName(convertNameToFile_X(pd->name() + "-example", false, true));
         pd->addSectionsToDefinition(root->m_anchors);
         pd->setLanguage(root->m_srcLang);

         Doxy_Globals::exampleSDict.insert(root->m_entryName, pd);
      }
   }

   RECURSE_ENTRYTREE(buildExampleList, ptrEntry);
}

// print the entry tree (for debugging)
void printNavTree(QSharedPointer<Entry> ptrEntry, int indent)
{
   QString indentStr;
   indentStr.fill(' ', indent);

   msg("%s%s (section = 0x%x)\n", indentStr.isEmpty() ? "" : csPrintable(indentStr),
       ptrEntry->m_entryName.isEmpty() ? "<empty>" : csPrintable(ptrEntry->m_entryName),
       ptrEntry->section );

   for (auto e : ptrEntry->children()) {
      printNavTree(e, indent + 2);
   }
}

// generate the example documentation
void Doxy_Work::generateExampleDocs()
{
   Doxy_Globals::outputList.disable(OutputGenerator::Man);

   for (auto &pd : Doxy_Globals::exampleSDict) {
      msg("Generating docs for example %s\n", csPrintable(pd->name()) );
      resetCCodeParserState();

      QString n = pd->getOutputFileBase();

      startFile(Doxy_Globals::outputList, n, n, pd->name());
      startTitle(Doxy_Globals::outputList, n);

      Doxy_Globals::outputList.docify(pd->name());
      endTitle(Doxy_Globals::outputList, n, 0);

      Doxy_Globals::outputList.startContents();
      Doxy_Globals::outputList.generateDoc(pd->docFile(), pd->docLine(), pd, QSharedPointer<MemberDef>(),
                  pd->documentation() + "\n\n\\include " + pd->name(), true, true, pd->name() );

      endFile(Doxy_Globals::outputList); // contains g_outputList->endContents()
   }

   Doxy_Globals::outputList.enable(OutputGenerator::Man);
}

// generate module pages
void Doxy_Work::generateGroupDocs()
{
   for (auto gd : Doxy_Globals::groupSDict) {
      if (! gd->isReference()) {
         msg("Generating docs for group %s\n", csPrintable(gd->name()) );
         gd->writeDocumentation(Doxy_Globals::outputList);
      }
   }
}

//static void generatePackageDocs()
//{
//  writePackageIndex(*g_outputList);
//
//  if (Doxy_Globals::packageDict.count() > 0)  {
//    PackageSDict::iterator pdi(Doxy_Globals::packageDict);
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
   for (auto &nd : Doxy_Globals::namespaceSDict) {

      if (nd->isLinkableInProject()) {
         msg("Generating docs for namespace %s\n", csPrintable(nd->name()));
         nd->writeDocumentation(Doxy_Globals::outputList);
      }

      // for each class in the namespace
      for (auto cd : nd->getClassSDict()) {

         if ( (cd->isLinkableInProject() && cd->templateMaster() == nullptr) && ! cd->isHidden() && ! cd->isEmbeddedInOuterScope() ) {
              // skip external references, anonymous compounds and
              // template instances and nested classes

            msg("Generating docs for namespace compounds %s\n", csPrintable(cd->name()));

            cd->writeDocumentation(Doxy_Globals::outputList);
            cd->writeMemberList(Doxy_Globals::outputList);
         }

         cd->writeDocumentationForInnerClasses(Doxy_Globals::outputList);
      }
   }
}

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
   static const QDir configDir      = Config::getConfigDir();
   static const QString projectLogo = Config::getString("project-logo");

   if (! projectLogo.isEmpty()) {
      QFileInfo fi(configDir, projectLogo);

      if (fi.exists()) {
         QString destFileName = outputType + "/" + fi.fileName();
         copyFile(fi.absoluteFilePath(), destFileName);

         Doxy_Globals::indexList.addImageFile(fi.fileName());

      } else {
         err("Project logo file '%s' does not exist\n", csPrintable(projectLogo));

      }
   }
}

void Doxy_Work::copyExtraFiles(const QString &outputType)
{
   static const QDir configDir = Config::getConfigDir();

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

            Doxy_Globals::indexList.addImageFile(fi.fileName());
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

   return Doxy_Globals::parserManager.getParser(extension);
}

void Doxy_Work::parseFile(ParserInterface *parser, QSharedPointer<Entry> root,
      QSharedPointer<FileDef> fd, QString fileName, enum ParserMode mode, QStringList &includedFiles)
{
   static const bool clangParsing        = Config::getBool("clang-parsing");
   static const bool enablePreprocessing = Config::getBool("enable-preprocessing");

   QString extension;
   int ei = fileName.lastIndexOf('.');

   if (ei != -1) {
      extension = fileName.right(fileName.length() - ei);
   } else {
      extension = ".no_extension";
   }

   QFileInfo fi(fileName);
   QString fileContents;

   if (! clangParsing && enablePreprocessing && parser->needsPreprocessing(extension)) {
      msg("Processing %s\n", csPrintable(fileName));

      fileContents = readInputFile(fileName);
      fileContents = preprocessFile(fileName, fileContents);

   } else {
      // no preprocessing, if clang processing this branch is forced
      msg("Reading %s\n", csPrintable(fileName));
      fileContents = readInputFile(fileName);
   }

   if (! fileContents.endsWith("\n")) {
      // add extra newline to help parser
      fileContents += '\n';
   }

   // convert multi-line C++ comments to C style comments
   QString buffer = convertCppComments(fileContents, fileName);
   auto srcLang   = fd->getLanguage();

   if (clangParsing && (srcLang == SrcLangExt_Cpp || srcLang == SrcLangExt_ObjC)) {
      fd->getAllIncludeFilesRecursively(includedFiles);

      // use clang for parsing
      parser->parseInput(fileName, buffer, root, mode, includedFiles, true);

   } else {
      // use lex for parser
      parser->parseInput(fileName, buffer, root, mode, includedFiles, false);

   }

   // add fileDef to the child entries
   root->createNavigationIndex(fd);
}

// parse the list of input files
void Doxy_Work::parseFiles(QSharedPointer<Entry> root)
{
   // user specified
   static const bool clangParsing = Config::getBool("clang-parsing");

   if (clangParsing) {
      QSet<QString> processedFiles;
      QSet<QString> filesToProcess;

      for (auto fName : Doxy_Globals::g_inputFiles) {
         filesToProcess.insert(fName);
      }

      // process source files and their include dependencies
      for (auto fName : Doxy_Globals::g_inputFiles) {
         bool ambig;

         QSharedPointer<FileDef> fd = findFileDef(&Doxy_Globals::inputNameDict, fName, ambig);
         assert(fd != 0);

         if (fd->isSource() && ! fd->isReference()) {
            QStringList includedFiles;

            auto srcLang = fd->getLanguage();

            ParserInterface *parser = getParserForFile(fName);
            parseFile(parser, root, fd, fName, ParserMode::SOURCE_FILE, includedFiles);

            // process any include files in the the current source file
            for (auto file : includedFiles) {

               if (! filesToProcess.contains(file)) {
                  break;
               }

               if (file != fName && ! processedFiles.contains(file)) {
                  QSharedPointer<FileDef> ifd = findFileDef(&Doxy_Globals::inputNameDict, file, ambig);

                  if (ifd && ! ifd->isReference()) {
                     QStringList moreFiles;

                     parseFile(parser, root, ifd, file, ParserMode::INCLUDE_FILE, moreFiles);
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

            QSharedPointer<FileDef> fd = findFileDef(&Doxy_Globals::inputNameDict, fName, ambig);
            assert(fd != 0);

            auto srcLang = fd->getLanguage();

            ParserInterface *parser = getParserForFile(fName);
            parseFile(parser, root, fd, fName, ParserMode::SOURCE_FILE, includedFiles);

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

         QSharedPointer<FileDef> fd = findFileDef(&Doxy_Globals::inputNameDict, fName, ambig);
         assert(fd != nullptr);

         ParserInterface *parser = getParserForFile(fName);
         parseFile(parser, root, fd, fName, ParserMode::SOURCE_FILE, includedFiles);
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

      if (sepPos == 0 && (result.startsWith("//") || result.startsWith("\\\\"))) {
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

void Doxy_Work::readTagFile(QSharedPointer<Entry> root, const QString &tagFile)
{
   QString fname;
   QString url;

   int pos = tagFile.indexOf('=');

   if (pos != -1) {
      // tag command contains a destination
      fname = tagFile.left(pos).trimmed();
      url   = tagFile.right(tagFile.length() - pos - 1).trimmed();

      QFileInfo fi(fname);
      Doxy_Globals::tagDestinationDict.insert(fi.absoluteFilePath(), url);

   } else {
      fname = tagFile;

   }

   QFileInfo fi(fname);
   if (! fi.exists() || ! fi.isFile()) {
      err("Tag file `%s' does not exist or is not a file\n", csPrintable(fname));
      return;
   }

   if (! url.isEmpty()) {
      msg("Reading tag file `%s', location `%s'\n", csPrintable(fname), csPrintable(url));

   } else {
      msg("Reading tag file `%s'\n", csPrintable(fname));
   }

   parseTagFile(root, fi.absoluteFilePath());
}

QString Doxy_Work::createOutputDirectory(const QString &baseDirName, const QString &formatDirOption,
                  const QString &defaultDirName)
{
   QString formatDirName = Config::getString(formatDirOption);

   if (formatDirName.isEmpty()) {
      formatDirName = baseDirName + defaultDirName;

   } else if (formatDirName.at(0) != '/' && (formatDirName.length() == 1 || formatDirName[1] != ':')) {
      formatDirName.prepend(baseDirName + '/');
   }

   QDir formatDir(formatDirName);

   if (! formatDir.exists() && ! formatDir.mkpath(formatDirName)) {
      err("Unable to create output directory %s\n", csPrintable(formatDirName));
      stopDoxyPress();
   }

   return formatDirName;
}

// read all files matching at least one pattern in `patList' in the directory represented by `fi'
// directory is read if the recursiveFlag is set, contents of all files is append to the input string

void Doxy_Work::readDir(const QFileInfo &fi, ReadDirArgs &data)
{
   static const bool excludeSymlink = Config::getBool("exclude-symlinks");

   QString dirName = fi.absoluteFilePath();

   if (data.isPathSet && ! data.pathSet.contains(dirName)) {
      data.pathSet.insert(dirName);
   }

   if (fi.isSymLink()) {
      dirName = resolveSymlink(dirName);

      if (dirName.isEmpty()) {
         return;     // recursive symlink
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
                           data.fnList.append(fn);
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
                        data.fnList.append(fn);
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

            Doxy_Globals::formulaList.append(f);
            Doxy_Globals::formulaDict.insert(formText, f);
            Doxy_Globals::formulaNameDict.insert(formName, f);
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
   (void) unused;

   msg("Cleaning up\n");

#ifdef HAS_SIGNALS
   killpg(0, SIGINT);
#endif

   msg("DoxyPress aborted\n\n");

   Doxy_Globals::programExit = true;
   exit(1);
}

void Doxy_Work::writeTagFile()
{
   static const QString generateTagFile = Config::getString("generate-tagfile");

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
   for (auto &fn : Doxy_Globals::inputNameList) {
      for (auto fd : *fn) {
         if (fd->isLinkableInProject()) {
            fd->writeTagFile(tagFile);
         }
      }
   }

   // for each class
   for (auto cd : Doxy_Globals::classSDict) {
      if (cd->isLinkableInProject()) {
         cd->writeTagFile(tagFile);
      }
   }

   // for each namespace
   for (auto &nd : Doxy_Globals::namespaceSDict) {
      if (nd->isLinkableInProject()) {
         nd->writeTagFile(tagFile);
      }
   }

   // for each group
   for (auto gd : Doxy_Globals::groupSDict) {
      if (gd->isLinkableInProject()) {
         gd->writeTagFile(tagFile);
      }
   }

   // for each page
   for (auto &pd : Doxy_Globals::pageSDict) {
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
   Doxy_Globals::infoLog_Stat.begin("Searching for include files\n");

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
      data.fnDict             = Doxy_Globals::includeNameDict;
      data.includePatternList = includePatterns;
      data.excludePatternList = excludePatterns;

      readFileOrDirectory(fName, data);

      Doxy_Globals::includeNameDict = data.fnDict;
   }
   Doxy_Globals::infoLog_Stat.end();


   // examples
   Doxy_Globals::infoLog_Stat.begin("Searching for example files\n");

   const QStringList examplePath     = Config::getList("example-source");
   const QStringList examplePatterns = Config::getList("example-patterns");
   const bool exampleRecursive       = Config::getBool("example-recursive");

   for (auto s : examplePath) {
      ReadDirArgs data;

      data.recursive          = exampleRecursive;
      data.isFnDict           = true;
      data.fnDict             = Doxy_Globals::exampleNameDict;
      data.includePatternList = examplePatterns;

      readFileOrDirectory(s, data);

      Doxy_Globals::exampleNameDict = data.fnDict;
   }
   Doxy_Globals::infoLog_Stat.end();


   // images
   Doxy_Globals::infoLog_Stat.begin("Searching for images\n");
   const QStringList imagePath = Config::getList("image-path");

   for (auto s : imagePath) {
      ReadDirArgs data;

      data.recursive = inputRecursive;
      data.isFnDict  = true;
      data.fnDict    = Doxy_Globals::imageNameDict;

      readFileOrDirectory(s, data);

      Doxy_Globals::imageNameDict = data.fnDict;
   }
   Doxy_Globals::infoLog_Stat.end();


   // dot files
   Doxy_Globals::infoLog_Stat.begin("Searching for dot files\n");
   const QStringList dotFiles = Config::getList("dot-file-dirs");

   for (auto s : dotFiles) {
      ReadDirArgs data;

      data.recursive  = inputRecursive;
      data.isFnDict   = true;
      data.fnDict     = Doxy_Globals::dotFileNameDict;

      readFileOrDirectory(s, data);

      Doxy_Globals::dotFileNameDict = data.fnDict;
   }
   Doxy_Globals::infoLog_Stat.end();


   // msc
   Doxy_Globals::infoLog_Stat.begin("Searching for msc files\n");
   const QStringList mscFiles = Config::getList("msc-file-dirs");

   for (auto s : mscFiles) {
      ReadDirArgs data;

      data.recursive  = inputRecursive;
      data.isFnDict   = true;
      data.fnDict     = Doxy_Globals::mscFileNameDict;

      readFileOrDirectory(s, data);

      Doxy_Globals::mscFileNameDict = data.fnDict;
   }
   Doxy_Globals::infoLog_Stat.end();


   // dia
   Doxy_Globals::infoLog_Stat.begin("Searching for dia files\n");
   const QStringList diaFiles = Config::getList("dia-file-dirs");

   for (auto s : diaFiles) {
      ReadDirArgs data;

      data.recursive   = inputRecursive;
      data.isFnDict    = true;
      data.fnDict      = Doxy_Globals::diaFileNameDict;

      readFileOrDirectory(s, data);

      Doxy_Globals::diaFileNameDict = data.fnDict;
   }
   Doxy_Globals::infoLog_Stat.end();


   Doxy_Globals::infoLog_Stat.begin("Searching for files to exclude\n");
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
   Doxy_Globals::infoLog_Stat.end();


   // find input files
   Doxy_Globals::infoLog_Stat.begin("Searching for files to process\n");

   QSet<QString> killDict;
   QStringList inputSource = Config::getList("input-source");

   for (auto s : inputSource) {
      QString path = s;
      uint len     = path.length();

      if (len > 0) {
         // strip trailing slashes

         if (path.at(len - 1) == '\\' || path.at(len - 1) == '/') {
            path = path.left(len - 1);
         }

         ReadDirArgs data;

         data.recursive          = inputRecursive;
         data.isFnList           = true;
         data.fnList             = Doxy_Globals::inputNameList;
         data.isFnDict           = true;
         data.fnDict             = Doxy_Globals::inputNameDict;
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

         Doxy_Globals::inputNameList  = data.fnList;
         Doxy_Globals::inputNameDict  = data.fnDict;
         Doxy_Globals::g_inputFiles   = data.resultList;
         killDict                     = data.killDict;
         Doxy_Globals::inputPaths     = data.pathSet;
      }
   }

   // markdown as mainpage
   QString mdfileAsMainPage = Config::getString("mdfile-mainpage");

   if (mdfileAsMainPage.isEmpty()) {
      // used as a default mainpage
      mdfileAsMainPage = "README.md";
   }

   QFileInfo mdfile(mdfileAsMainPage);

   if (mdfile.exists() && ! Doxy_Globals::g_inputFiles.contains(mdfileAsMainPage) &&
               ! Doxy_Globals::g_inputFiles.contains(mdfile.absoluteFilePath()) ) {

      ReadDirArgs data;

      data.recursive          = false;
      data.isFnList           = true;
      data.fnList             = Doxy_Globals::inputNameList;
      data.isFnDict           = true;
      data.fnDict             = Doxy_Globals::inputNameDict;
      data.includePatternList = inputPatterns;
      data.excludePatternList = excludePatterns;
      data.excludeSet         = excludeSet;
      data.isResultList       = true;
      data.resultList         = Doxy_Globals::g_inputFiles;
      data.isKillDict         = false;
      data.killDict           = QSet<QString>();
      data.isPathSet          = true;
      data.pathSet            = Doxy_Globals::inputPaths;

      readFileOrDirectory(mdfile.absoluteFilePath(), data);

      Doxy_Globals::inputNameList  = data.fnList;
      Doxy_Globals::inputNameDict  = data.fnDict;
      Doxy_Globals::g_inputFiles   = data.resultList;
      Doxy_Globals::inputPaths     = data.pathSet;
   }

   // sort now
   Doxy_Globals::inputNameList.sort();

   Doxy_Globals::infoLog_Stat.end();
}

