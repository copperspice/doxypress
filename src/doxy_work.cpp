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

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <arguments.h>
#include <bufstr.h>
#include <cite.h>
#include <classlist.h>
#include <cmdmapper.h>
#include <code.h>
#include <commentcnv.h>
#include <config.h>
#include <dbusxmlscanner.h>
#include <declinfo.h>
#include <defargs.h>
#include <defgen.h>
#include <dirdef.h>
#include <docbookgen.h>
#include <docparser.h>
#include <docsets.h>
#include <dot.h>
#include <doxygen.h>
#include <eclipsehelp.h>
#include <entry.h>
#include <filename.h>
#include <fileparser.h>
#include <filestorage.h>
#include <formula.h>
#include <fortranscanner.h>
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
#include <markdown.h>
#include <membername.h>
#include <msc.h>
#include <namespacedef.h>
#include <objcache.h>
#include <outputlist.h>
#include <pagedef.h>
#include <parserintf.h>
#include <perlmodgen.h>
#include <portable.h>
#include <pre.h>
#include <pyscanner.h>
#include <qhp.h>
#include <reflist.h>
#include <rtfgen.h>
#include <scanner.h>
#include <searchindex.h>
#include <settings.h>
#include <sqlite3gen.h>
#include <store.h>
#include <tagreader.h>
#include <tclscanner.h>
#include <util.h>
#include <xmlgen.h>

#include <doxy_globals.h>

#define RECURSE_ENTRYTREE(func,var) \
  do {  \
       for (auto eli : var->children() ) { \
         func(&eli);   \
       }  \
  } while(0)


#if !defined(_WIN32) || defined(__CYGWIN__)
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
   void addClassToContext(EntryNav *rootNav);
   void addEnumValuesToEnums(EntryNav *rootNav);

   void addIncludeFile(ClassDef *cd, FileDef *ifd, Entry *root);
   void addInterfaceOrServiceToServiceOrSingleton(EntryNav *const rootNav, ClassDef *const cd, QByteArray const &rname);

   void addMethodToClass(EntryNav *rootNav, ClassDef *cd, const QByteArray &rname, bool isFriend);

   void addMembersToIndex();
   void addMembersToMemberGroup();

   void addMemberDocs(EntryNav *rootNav, MemberDef *md, const char *funcDecl,
                      ArgumentList *al, bool over_load, NamespaceSDict *nl = 0);

   void addMemberDocs(EntryNav *rootNav, MemberDef *md, const char *funcDecl, ArgumentList *al,
                             bool over_load, NamespaceSDict *);

   void addPageToContext(PageDef *pd, EntryNav *rootNav);
   void addListReferences();
   void addRelatedPage(EntryNav *rootNav);
   void addSourceReferences();
   void addSTLClasses(EntryNav *rootNav);
   void addSTLIterator(EntryNav *classEntryNav, const char *name);
   void addSTLMember(EntryNav *rootNav, const char *type, const char *name);

   void addVariable(EntryNav *rootNav, int isFuncPtr = -1);

   MemberDef *addVariableToClass(EntryNav *rootNav, ClassDef *cd, MemberType mtype, const QByteArray &name,
                                        bool fromAnnScope, MemberDef *fromAnnMemb, Protection prot, Relationship related);

   MemberDef *addVariableToFile(EntryNav *rootNav, MemberType mtype, const QByteArray &scope, const QByteArray &name,
                                       bool fromAnnScope, MemberDef *fromAnnMemb);

   void buildClassDocList(EntryNav *rootNav);
   void buildClassList(EntryNav *rootNav);
   void buildCompleteMemberLists();
   void buildFileList(EntryNav *rootNav);
   void buildExampleList(EntryNav *rootNav);
   void buildFunctionList(EntryNav *rootNav);
   void buildGroupList(EntryNav *rootNav);
   void buildGroupListFiltered(EntryNav *rootNav, bool additional, bool includeExternal);
   void buildInterfaceAndServiceList(EntryNav *const rootNav);
   void buildListOfUsingDecls(EntryNav *rootNav);
   void buildNamespaceList(EntryNav *rootNav);
   void buildPageList(EntryNav *rootNav);
   Definition *buildScopeFromQualifiedName(const QByteArray name, int level, SrcLangExt lang, TagInfo *tagInfo);
   void buildTypedefList(EntryNav *rootNav);
   void buildVarList(EntryNav *rootNav);

   void checkPageRelations();

   void combineUsingRelations();
   void computeClassRelations();
   int computeIdealCacheParam(uint v);
   void computeMemberReferences();
   void computeMemberRelations();
   void computePageRelations(EntryNav *rootNav);
   void computeTemplateClassRelations();

   ClassDef::CompoundType convertToCompoundType(int section, uint64_t specifier);

   void copyExtraFiles(const QByteArray &filesOption, const QByteArray &outputOption);
   void copyLogo();
   void copyStyleSheet();

   QByteArray createOutputDirectory(const QByteArray &baseDirName, const char *formatDirOption, const char *defaultDirName);
   void createTemplateInstanceMembers();
   ClassDef *createTagLessInstance(ClassDef *rootCd, ClassDef *templ, const QByteArray &fieldName);

   void distributeMemberGroupDocumentation();
   void dumpSymbol(FTextStream &t, Definition *d);
   void dumpSymbolMap();

   void escapeAliases();
   void expandAliases();
   QByteArray extractClassName(EntryNav *rootNav);
   void exitDoxygen();

   void filterMemberDocumentation(EntryNav *rootNav);

   void findBaseClassesForClass(EntryNav *rootNav, Definition *context, ClassDef *masterCd, ClassDef *instanceCd,
                                FindBaseClassRelation_Mode mode, bool isArtificial, ArgumentList *actualArgs = 0,
                                QHash<QString, int> *templateNames = 0);

   ClassDef *findClassDefinition(FileDef *fd, NamespaceDef *nd, const char *scopeName);

   void findClassEntries(EntryNav *rootNav);
   bool findClassRelation(EntryNav *rootNav, Definition *context, ClassDef *cd, BaseInfo *bi,
                          QHash<QString, int> *templateNames, FindBaseClassRelation_Mode mode, bool isArtificial);

   ClassDef *findClassWithinClassContext(Definition *context, ClassDef *cd, const QByteArray &name);

   void findDefineDocumentation(EntryNav *rootNav);
   void findDEV(const MemberNameSDict &mnsd);
   void findDirDocumentation(EntryNav *rootNav);
   void findDocumentedEnumValues();

   int findEndOfTemplate(const QByteArray &s, int startPos);
   void findEnums(EntryNav *rootNav);
   void findEnumDocumentation(EntryNav *rootNav);

   void findFriends();
   int findFunctionPtr(const QByteArray &type, int lang, int *pLength = 0);

   bool findGlobalMember(EntryNav *rootNav, const QByteArray &namespaceName, const char *type, const char *name,
                                const char *tempArg, const char *, const char *decl);

   void findGroupScope(EntryNav *rootNav);
   void findInheritedTemplateInstances();

   void findMainPage(EntryNav *rootNav);
   void findMainPageTagFiles(EntryNav *rootNav);
   void findMember(EntryNav *rootNav, QByteArray funcDecl, bool overloaded, bool isFunc);
   void findMemberDocumentation(EntryNav *rootNav);

   void findObjCMethodDefinitions(EntryNav *rootNav);

   Definition *findScopeFromQualifiedName(Definition *startScope, const QByteArray &n, FileDef *fileScope, TagInfo *tagInfo);
   Definition *findScopeFromQualifiedName(QSharedPointer<Definition> startScope, const QByteArray &n, FileDef *fileScope, TagInfo *tagInfo);

   void findSectionsInDocumentation();

   void findTagLessClasses();
   void findTagLessClasses(ClassDef *cd);

   bool findTemplateInstanceRelation(Entry *root, Definition *context, ClassDef *templateClass, const QByteArray &templSpec,
                                     QHash<QString, int> *templateNames, bool isArtificial);

   NamespaceDef *findUsedNamespace(NamespaceSDict *unl, const QByteArray &name);

   void findUsedClassesForClass(EntryNav *rootNav, Definition *context, ClassDef *masterCd, ClassDef *instanceCd, bool isArtificial,
                                ArgumentList *actualArgs = 0, QHash<QString, int> *templateNames = 0);

   void findUsedTemplateInstances();

   void findUsingDeclarations(EntryNav *rootNav);
   void findIncludedUsingDirectives();
   void findUsingDirectives(EntryNav *rootNav);
   void findUsingDeclImports(EntryNav *rootNav);

   void flushUnresolvedRelations();
   void flushCachedTemplateRelations();

   void generateClassList(ClassSDict &classSDict);
   void generateClassDocs();
   void generateExampleDocs();
   void generateFileDocs();
   void generateFileSources();
   void generateGroupDocs();
   void generateNamespaceDocs();
   void generatePageDocs();
   void generateXRefPages();

   QByteArray getQchFileName();
   QHash<QString, int> *getTemplateArgumentsInName(ArgumentList *templateArguments, const QByteArray &name);

   void inheritDocumentation();
   bool isClassSection(EntryNav *rootNav);
   bool isRecursiveBaseClass(const QByteArray &scope, const QByteArray &name);
   bool isSpecialization(const QList<ArgumentList> &srcTempArgLists, const QList<ArgumentList> &dstTempArgLists);
   bool isVarWithConstructor(EntryNav *rootNav);

   void mergeCategories();

   void organizeSubGroupsFiltered(EntryNav *rootNav, bool additional);
   void organizeSubGroups(EntryNav *rootNav);

   void parseFile(ParserInterface *parser, Entry *root, EntryNav *rootNav, FileDef *fd, const char *fn,
                bool sameTu, QStringList &filesInSameTu);

   void parseFiles(Entry *root, EntryNav *rootNav);
   void parseInput();
   void processTagLessClasses(ClassDef *rootCd, ClassDef *cd,ClassDef *tagParentCd, const QByteArray &prefix, int count);

   void resolveClassNestingRelations();
   void resolveUserReferences();
   void readTagFile(Entry *root, const char *tl);

   bool scopeIsTemplate(Definition *d);
   void sortMemberLists();

   void substituteTemplatesInArgList(const QList<ArgumentList> &srcTempArgLists, const QList<ArgumentList> &dstTempArgLists,
               ArgumentList *src, ArgumentList *dst, ArgumentList *funcTempArgs = 0);

   QByteArray substituteTemplatesInString( const QList<ArgumentList> &srcTempArgLists, const QList<ArgumentList> &dstTempArgLists,
      ArgumentList *funcTempArgList, const QByteArray &src);

   void transferFunctionDocumentation();
   void transferFunctionReferences();
   void transferRelatedFunctionDocumentation();

   void writeTagFile();
}

using namespace Doxy_Work;

// ** next two functions are the two core functions

void parseInput()
{
   atexit(exitDoxygen);

   // Make sure the output directory exists
   QByteArray &outputDirectory = Config_getString("OUTPUT_DIRECTORY");

   if (outputDirectory.isEmpty()) {
      outputDirectory = QDir::currentPath().toUtf8();

   } else {
      QDir dir(outputDirectory);

      if (!dir.exists()) {
         dir.setPath(QDir::currentPath());

         if (!dir.mkdir(outputDirectory)) {
            err("tag OUTPUT_DIRECTORY: Output directory `%s' does not "
                "exist and cannot be created\n", outputDirectory.data());

            cleanUpDoxygen();
            exit(1);

         } else {
            msg("Notice: Output directory `%s' does not exist. "
                "I have created it for you.\n", outputDirectory.data());
         }
         dir.cd(outputDirectory);
      }

      outputDirectory = dir.absolutePath().toUtf8();
   }

   // Initialize global lists and dictionaries
   Doxygen::symbolStorage = new Store;

   // also scale lookup cache with SYMBOL_CACHE_SIZE
   int cacheSize = Config_getInt("LOOKUP_CACHE_SIZE");

   if (cacheSize < 0) {
      cacheSize = 0;
   }

   if (cacheSize > 9) {
      cacheSize = 9;
   }

   uint lookupSize = 65536 << cacheSize;

   Doxygen::lookupCache = new QCache<QString, LookupInfo>(lookupSize);

#ifdef HAS_SIGNALS
   signal(SIGINT, stopDoxygen);
#endif

   uint pid = portable_pid();

   Doxygen::objDBFileName = QString("doxygen_objdb_%1.tmp").arg(pid);
   Doxygen::objDBFileName.prepend(outputDirectory + "/");

   Doxygen::entryDBFileName = QString("doxygen_entrydb_%1.tmp").arg(pid);
   Doxygen::entryDBFileName.prepend(outputDirectory + "/");

   if (Doxygen::symbolStorage->open(Doxygen::objDBFileName) == -1) {
      err("Failed to open temporary file %s\n", Doxygen::objDBFileName.data());
      exit(1);
   }

   // Check/create output directorties
   QByteArray htmlOutput;
   bool &generateHtml = Config_getBool("GENERATE_HTML");

   if (generateHtml || Doxy_Globals::g_useOutputTemplate /* TODO: temp hack */) {
      htmlOutput = createOutputDirectory(outputDirectory, "HTML_OUTPUT", "/html");
   }

   QByteArray docbookOutput;
   bool &generateDocbook = Config_getBool("GENERATE_DOCBOOK");

   if (generateDocbook) {
      docbookOutput = createOutputDirectory(outputDirectory, "DOCBOOK_OUTPUT", "/docbook");
   }

   QByteArray xmlOutput;
   bool &generateXml = Config_getBool("GENERATE_XML");
   if (generateXml) {
      xmlOutput = createOutputDirectory(outputDirectory, "XML_OUTPUT", "/xml");
   }

   QByteArray latexOutput;
   bool &generateLatex = Config_getBool("GENERATE_LATEX");
   if (generateLatex) {
      latexOutput = createOutputDirectory(outputDirectory, "LATEX_OUTPUT", "/latex");
   }

   QByteArray rtfOutput;
   bool &generateRtf = Config_getBool("GENERATE_RTF");
   if (generateRtf) {
      rtfOutput = createOutputDirectory(outputDirectory, "RTF_OUTPUT", "/rtf");
   }

   QByteArray manOutput;
   bool &generateMan = Config_getBool("GENERATE_MAN");
   if (generateMan) {
      manOutput = createOutputDirectory(outputDirectory, "MAN_OUTPUT", "/man");
   }

   //QByteArray sqlOutput;
   //bool &generateSql = Config_getBool("GENERATE_SQLITE3");
   //if (generateSql)
   //  sqlOutput = createOutputDirectory(outputDirectory,"SQLITE3_OUTPUT","/sqlite3");

   if (Config_getBool("HAVE_DOT")) {
      QByteArray curFontPath = Config_getString("DOT_FONTPATH");

      if (curFontPath.isEmpty()) {
         portable_getenv("DOTFONTPATH");
         QByteArray newFontPath = ".";
         if (!curFontPath.isEmpty()) {
            newFontPath += portable_pathListSeparator();
            newFontPath += curFontPath;
         }

         portable_setenv("DOTFONTPATH", newFontPath);

      } else {
         portable_setenv("DOTFONTPATH", curFontPath);
      }
   }

   // Handle layout file
   LayoutDocManager::instance().init();

   QByteArray &layoutFileName = Config_getString("LAYOUT_FILE");
   bool defaultLayoutUsed = false;

   if (layoutFileName.isEmpty()) {
      layoutFileName = "DoxygenLayout.xml";
      defaultLayoutUsed = true;
   }

   QFile layoutFile(layoutFileName);

   if (layoutFile.open(QIODevice::ReadOnly)) {
      msg("Parsing layout file %s...\n", layoutFileName.data());

      QTextStream t(&layoutFile);
      t.setCodec("UTF-8");

      LayoutDocManager::instance().parse(t, layoutFileName);

   } else if (!defaultLayoutUsed) {
      warn_uncond("failed to open layout file '%s' for reading!\n", layoutFileName.data());
   }

   /**************************************************************************
    *             Read and preprocess input                                  *
    **************************************************************************/

   // prevent search in the output directories
   QStringList &exclPatterns = Config_getList("EXCLUDE_PATTERNS");

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

   searchInputFiles();

   // Notice: the order of the function calls below is very important!

   if (Config_getBool("GENERATE_HTML")) {
      readFormulaRepository();
   }

   // Handle Tag Files
   Doxy_Globals::g_storage = new FileStorage;
   Doxy_Globals::g_storage->setName(Doxygen::entryDBFileName);

   if (! Doxy_Globals::g_storage->open(QIODevice::WriteOnly)) {
      err("Failed to create temporary storage file %s\n", Doxygen::entryDBFileName.data());
      exit(1);
   }

   Entry *root = new Entry;
   EntryNav *rootNav = new EntryNav(0, root);
   rootNav->setEntry(root);
   msg("Reading and parsing tag files\n");

   QStringList &tagFileList = Config_getList("TAGFILES");

   for (auto s : tagFileList) {
      readTagFile(root, qPrintable(s));
      root->createNavigationIndex(rootNav, Doxy_Globals::g_storage, 0);
   }

   // parse source files
   if (Config_getBool("BUILTIN_STL_SUPPORT")) {
      addSTLClasses(rootNav);
   }

   Doxy_Globals::g_stats.begin("Parsing files\n");
   parseFiles(root, rootNav);

   Doxy_Globals::g_storage->close();
   Doxy_Globals::g_stats.end();

   // we are done with input scanning now, so free up the buffers used by flex
   // (can be around 4MB)

   preFreeScanner();
   scanFreeScanner();
   pyscanFreeScanner();

   if (! Doxy_Globals::g_storage->open(QIODevice::ReadOnly)) {
      err("Failed to open temporary storage file %s for reading", Doxygen::entryDBFileName.data());
      exit(1);
   }

   // gather information
   Doxy_Globals::g_stats.begin("Building group list...\n");
   buildGroupList(rootNav);
   organizeSubGroups(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Building directory list...\n");
   buildDirectories();
   findDirDocumentation(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Building namespace list...\n");
   buildNamespaceList(rootNav);
   findUsingDirectives(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Building file list...\n");
   buildFileList(rootNav);
   Doxy_Globals::g_stats.end();
   //generateFileTree();

   Doxy_Globals::g_stats.begin("Building class list...\n");
   buildClassList(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Associating documentation with classes...\n");
   buildClassDocList(rootNav);

   // build list of using declarations here (global list)
   buildListOfUsingDecls(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Computing nesting relations for classes...\n");
   resolveClassNestingRelations();
   Doxy_Globals::g_stats.end();

   // 1.8.2-20121111: no longer add nested classes to the group as well
   //distributeClassGroupRelations();

   // calling buildClassList may result in cached relations that
   // become invalid after resolveClassNestingRelations(), that's why
   // we need to clear the cache here
   Doxygen::lookupCache->clear();

   // we don't need the list of using declaration anymore
   Doxy_Globals::g_usingDeclarations.clear();

   Doxy_Globals::g_stats.begin("Building example list...\n");
   buildExampleList(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Searching for enumerations...\n");
   findEnums(rootNav);
   Doxy_Globals::g_stats.end();

   // Since buildVarList calls isVarWithConstructor
   // and this calls getResolvedClass we need to process
   // typedefs first so the relations between classes via typedefs
   // are properly resolved. See bug 536385 for an example.
   Doxy_Globals::g_stats.begin("Searching for documented typedefs...\n");
   buildTypedefList(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Searching for members imported via using declarations...\n");
   findUsingDeclImports(rootNav);
   // this should be after buildTypedefList in order to properly import used typedefs
   findUsingDeclarations(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Searching for included using directives...\n");
   findIncludedUsingDirectives();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Searching for documented variables...\n");
   buildVarList(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Building interface member list...\n");
   buildInterfaceAndServiceList(rootNav); // UNO IDL

   Doxy_Globals::g_stats.begin("Building member list...\n"); // using class info only !
   buildFunctionList(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Searching for friends...\n");
   findFriends();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Searching for documented defines...\n");
   findDefineDocumentation(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Computing class inheritance relations...\n");
   findClassEntries(rootNav);
   findInheritedTemplateInstances();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Computing class usage relations...\n");
   findUsedTemplateInstances();
   Doxy_Globals::g_stats.end();

   if (Config_getBool("INLINE_SIMPLE_STRUCTS")) {
      Doxy_Globals::g_stats.begin("Searching for tag less structs...\n");
      findTagLessClasses();
      Doxy_Globals::g_stats.end();
   }

   Doxy_Globals::g_stats.begin("Flushing cached template relations that have become invalid...\n");
   flushCachedTemplateRelations();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Creating members for template instances...\n");
   createTemplateInstanceMembers();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Computing class relations...\n");
   computeTemplateClassRelations();
   flushUnresolvedRelations();

   computeClassRelations();
   Doxy_Globals::g_classEntries.clear();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Add enum values to enums...\n");
   addEnumValuesToEnums(rootNav);
   findEnumDocumentation(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Searching for member function documentation...\n");
   findObjCMethodDefinitions(rootNav);
   findMemberDocumentation(rootNav); // may introduce new members !

   transferRelatedFunctionDocumentation();
   transferFunctionDocumentation();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Building page list...\n");
   buildPageList(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Search for main page...\n");
   findMainPage(rootNav);
   findMainPageTagFiles(rootNav);
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Computing page relations...\n");
   computePageRelations(rootNav);
   checkPageRelations();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Determining the scope of groups...\n");
   findGroupScope(rootNav);
   Doxy_Globals::g_stats.end();

   msg("Freeing entry tree\n");
   delete rootNav;

   Doxy_Globals::g_storage->close();
   delete Doxy_Globals::g_storage;
   Doxy_Globals::g_storage = 0;

   QDir thisDir;
   thisDir.remove(Doxygen::entryDBFileName);

   Doxy_Globals::g_stats.begin("Determining which enums are documented\n");
   findDocumentedEnumValues();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Computing member relations...\n");
   mergeCategories();
   computeMemberRelations();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Building full member lists recursively...\n");
   buildCompleteMemberLists();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Adding members to member groups.\n");
   addMembersToMemberGroup();
   Doxy_Globals::g_stats.end();

   if (Config_getBool("DISTRIBUTE_GROUP_DOC")) {
      Doxy_Globals::g_stats.begin("Distributing member group documentation.\n");
      distributeMemberGroupDocumentation();
      Doxy_Globals::g_stats.end();
   }

   Doxy_Globals::g_stats.begin("Computing member references...\n");
   computeMemberReferences();
   Doxy_Globals::g_stats.end();

   if (Config_getBool("INHERIT_DOCS")) {
      Doxy_Globals::g_stats.begin("Inheriting documentation...\n");
      inheritDocumentation();
      Doxy_Globals::g_stats.end();
   }

   // compute the shortest possible names of all files
   // without losing the uniqueness of the file names.

   Doxy_Globals::g_stats.begin("Generating disk names...\n");

   for (auto item : *Doxygen::inputNameList) {
      item->generateDiskNames();
   }

   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Adding source references...\n");
   addSourceReferences();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Adding xrefitems...\n");
   addListReferences();
   generateXRefPages();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Sorting member lists...\n");
   sortMemberLists();
   Doxy_Globals::g_stats.end();

   if (Config_getBool("DIRECTORY_GRAPH")) {
      Doxy_Globals::g_stats.begin("Computing dependencies between directories...\n");
      computeDirDependencies();
      Doxy_Globals::g_stats.end();
   }

   //Doxy_Globals::g_stats.begin("Resolving citations...\n");
   //Doxygen::citeDict->resolve();

   Doxy_Globals::g_stats.begin("Generating citations page...\n");
   Doxygen::citeDict->generatePage();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Counting data structures...\n");
   countDataStructures();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Resolving user defined references...\n");
   resolveUserReferences();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Finding anchors and sections in the documentation...\n");
   findSectionsInDocumentation();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Transferring function references...\n");
   transferFunctionReferences();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Combining using relations...\n");
   combineUsingRelations();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Adding members to index pages...\n");
   addMembersToIndex();
   Doxy_Globals::g_stats.end();
}

void generateOutput()
{
   // Initialize output generators

   // dump all symbols
   if (Doxy_Globals::g_dumpSymbolMap) {
      dumpSymbolMap();
      exit(0);
   }

   initSearchIndexer();

   bool generateHtml  = Config_getBool("GENERATE_HTML");
   bool generateLatex = Config_getBool("GENERATE_LATEX");
   bool generateMan   = Config_getBool("GENERATE_MAN");
   bool generateRtf   = Config_getBool("GENERATE_RTF");

   Doxy_Globals::g_outputList = new OutputList(true);

   if (generateHtml) {
      Doxy_Globals::g_outputList->add(new HtmlGenerator);
      HtmlGenerator::init();

      // add HTML indexers that are enabled
      bool generateHtmlHelp    = Config_getBool("GENERATE_HTMLHELP");
      bool generateEclipseHelp = Config_getBool("GENERATE_ECLIPSEHELP");
      bool generateQhp         = Config_getBool("GENERATE_QHP");
      bool generateTreeView    = Config_getBool("GENERATE_TREEVIEW");
      bool generateDocSet      = Config_getBool("GENERATE_DOCSET");

      if (generateEclipseHelp) {
         Doxygen::indexList->addIndex(QSharedPointer<EclipseHelp>(new EclipseHelp));
      }

      if (generateHtmlHelp) {
         Doxygen::indexList->addIndex(QSharedPointer<HtmlHelp>(new HtmlHelp));
      }

      if (generateQhp) {
         Doxygen::indexList->addIndex(QSharedPointer<Qhp>(new Qhp));
      }

      if (generateTreeView) {
         Doxygen::indexList->addIndex(QSharedPointer<FTVHelp>(new FTVHelp(true)));
      }

      if (generateDocSet) {
         Doxygen::indexList->addIndex(QSharedPointer<DocSets>(new DocSets));
      }

      Doxygen::indexList->initialize();
      HtmlGenerator::writeTabData();

      // copy static stuff
      copyStyleSheet();
      copyLogo();
      copyExtraFiles("HTML_EXTRA_FILES", "HTML_OUTPUT");
      FTVHelp::generateTreeViewImages();
   }
   if (generateLatex) {
      Doxy_Globals::g_outputList->add(new LatexGenerator);
      LatexGenerator::init();

      // copy static stuff
      copyExtraFiles("LATEX_EXTRA_FILES", "LATEX_OUTPUT");
   }
   if (generateMan) {
      Doxy_Globals::g_outputList->add(new ManGenerator);
      ManGenerator::init();
   }
   if (generateRtf) {
      Doxy_Globals::g_outputList->add(new RTFGenerator);
      RTFGenerator::init();
   }
   if (Config_getBool("USE_HTAGS")) {
      Htags::useHtags = true;
      QByteArray htmldir = Config_getString("HTML_OUTPUT");

      if (!Htags::execute(htmldir)) {
         err("USE_HTAGS is YES but htags(1) failed. \n");
      }
      if (!Htags::loadFilemap(htmldir)) {
         err("htags(1) ended normally but failed to load the filemap. \n");
      }
   }

   /**************************************************************************
    *                        Generate documentation                          *
    **************************************************************************/

   if (generateHtml) {
      writeDoxFont(Config_getString("HTML_OUTPUT"));
   }
   if (generateLatex) {
      writeDoxFont(Config_getString("LATEX_OUTPUT"));
   }
   if (generateRtf) {
      writeDoxFont(Config_getString("RTF_OUTPUT"));
   }

   Doxy_Globals::g_stats.begin("Generating style sheet...\n");
   //printf("writing style info\n");
   Doxy_Globals::g_outputList->writeStyleInfo(0); // write first part
   Doxy_Globals::g_stats.end();

   static bool searchEngine      = Config_getBool("SEARCHENGINE");
   static bool serverBasedSearch = Config_getBool("SERVER_BASED_SEARCH");

   // generate search indices (need to do this before writing other HTML
   // pages as these contain a drop down menu with options depending on
   // what categories we find in this function.
   if (generateHtml && searchEngine) {
      Doxy_Globals::g_stats.begin("Generating search indices...\n");
      QByteArray searchDirName = Config_getString("HTML_OUTPUT") + "/search";
      QDir searchDir(searchDirName);

      if (!searchDir.exists() && !searchDir.mkdir(searchDirName)) {
         err("Could not create search results directory '%s' $PWD='%s'\n",
             searchDirName.data(), QDir::currentPath().data());
         exit(1);
      }
      HtmlGenerator::writeSearchData(searchDirName);
      if (!serverBasedSearch) { // client side search index
         writeJavascriptSearchIndex();
      }
      Doxy_Globals::g_stats.end();
   }

   Doxy_Globals::g_stats.begin("Generating example documentation...\n");
   generateExampleDocs();
   Doxy_Globals::g_stats.end();

   if (!Htags::useHtags) {
      Doxy_Globals::g_stats.begin("Generating file sources...\n");
      generateFileSources();
      Doxy_Globals::g_stats.end();
   }

   Doxy_Globals::g_stats.begin("Generating file documentation...\n");
   generateFileDocs();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Generating page documentation...\n");
   generatePageDocs();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Generating group documentation...\n");
   generateGroupDocs();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Generating class documentation...\n");
   generateClassDocs();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Generating namespace index...\n");
   generateNamespaceDocs();
   Doxy_Globals::g_stats.end();

   if (Config_getBool("GENERATE_LEGEND")) {
      Doxy_Globals::g_stats.begin("Generating graph info page...\n");
      writeGraphInfo(*Doxy_Globals::g_outputList);
      Doxy_Globals::g_stats.end();
   }

   Doxy_Globals::g_stats.begin("Generating directory documentation...\n");
   generateDirDocs(*Doxy_Globals::g_outputList);
   Doxy_Globals::g_stats.end();

   if (Doxygen::formulaList->count() > 0 && generateHtml && !Config_getBool("USE_MATHJAX")) {
      Doxy_Globals::g_stats.begin("Generating bitmaps for formulas in HTML...\n");
      Doxygen::formulaList->generateBitmaps(Config_getString("HTML_OUTPUT"));
      Doxy_Globals::g_stats.end();
   }

   if (Doxy_Globals::g_outputList->count() > 0) {
      writeIndexHierarchy(*Doxy_Globals::g_outputList);
   }

   Doxy_Globals::g_stats.begin("finalizing index lists...\n");
   Doxygen::indexList->finalize();
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("writing tag file...\n");
   writeTagFile();
   Doxy_Globals::g_stats.end();

   if (Config_getBool("DOT_CLEANUP")) {
      if (generateHtml) {
         removeDoxFont(Config_getString("HTML_OUTPUT"));
      }
      if (generateRtf) {
         removeDoxFont(Config_getString("RTF_OUTPUT"));
      }
      if (generateLatex) {
         removeDoxFont(Config_getString("LATEX_OUTPUT"));
      }
   }

   if (Config_getBool("GENERATE_XML")) {
      Doxy_Globals::g_stats.begin("Generating XML output...\n");
      Doxygen::generatingXmlOutput = true;
      generateXML();
      Doxygen::generatingXmlOutput = false;
      Doxy_Globals::g_stats.end();
   }
   if (USE_SQLITE3) {
      Doxy_Globals::g_stats.begin("Generating SQLITE3 output...\n");
      generateSqlite3();
      Doxy_Globals::g_stats.end();
   }

   if (Config_getBool("GENERATE_DOCBOOK")) {
      Doxy_Globals::g_stats.begin("Generating Docbook output...\n");
      generateDocbook();
      Doxy_Globals::g_stats.end();
   }

   if (Config_getBool("GENERATE_AUTOGEN_DEF")) {
      Doxy_Globals::g_stats.begin("Generating AutoGen DEF output...\n");
      generateDEF();
      Doxy_Globals::g_stats.end();
   }
   if (Config_getBool("GENERATE_PERLMOD")) {
      Doxy_Globals::g_stats.begin("Generating Perl module output...\n");
      generatePerlMod();
      Doxy_Globals::g_stats.end();
   }
   if (generateHtml && searchEngine && serverBasedSearch) {
      Doxy_Globals::g_stats.begin("Generating search index\n");
      if (Doxygen::searchIndex->kind() == SearchIndexIntf::Internal) { // write own search index
         HtmlGenerator::writeSearchPage();
         Doxygen::searchIndex->write(Config_getString("HTML_OUTPUT") + "/search/search.idx");
      } else { // write data for external search index
         HtmlGenerator::writeExternalSearchPage();
         QByteArray searchDataFile = Config_getString("SEARCHDATA_FILE");
         if (searchDataFile.isEmpty()) {
            searchDataFile = "searchdata.xml";
         }
         if (!portable_isAbsolutePath(searchDataFile)) {
            searchDataFile.prepend(Config_getString("OUTPUT_DIRECTORY") + "/");
         }
         Doxygen::searchIndex->write(searchDataFile);
      }
      Doxy_Globals::g_stats.end();
   }

   if (generateRtf) {
      Doxy_Globals::g_stats.begin("Combining RTF output...\n");
      if (!RTFGenerator::preProcessFileInplace(Config_getString("RTF_OUTPUT"), "refman.rtf")) {
         err("An error occurred during post-processing the RTF files!\n");
      }
      Doxy_Globals::g_stats.end();
   }

   if (Config_getBool("HAVE_DOT")) {
      Doxy_Globals::g_stats.begin("Running dot...\n");
      DotManager::instance()->run();
      Doxy_Globals::g_stats.end();
   }

   if (generateHtml &&
         Config_getBool("GENERATE_HTMLHELP") &&
         !Config_getString("HHC_LOCATION").isEmpty()) {
      Doxy_Globals::g_stats.begin("Running html help compiler...\n");
      QString oldDir = QDir::currentPath();
      QDir::setCurrent(Config_getString("HTML_OUTPUT"));
      portable_sysTimerStart();
      if (portable_system(Config_getString("HHC_LOCATION"), "index.hhp", false)) {
         err("failed to run html help compiler on index.hhp\n");
      }
      portable_sysTimerStop();
      QDir::setCurrent(oldDir);
      Doxy_Globals::g_stats.end();
   }
   if ( generateHtml && Config_getBool("GENERATE_QHP") && !Config_getString("QHG_LOCATION").isEmpty()) {
      Doxy_Globals::g_stats.begin("Running qhelpgenerator...\n");

      QString qhpFileName = Qhp::getQhpFileName();
      QString qchFileName = getQchFileName();

      QString args;
      QString("%1 -o \"%2\"").arg(qhpFileName).arg(qchFileName);

      QString const oldDir = QDir::currentPath();
      QDir::setCurrent(Config_getString("HTML_OUTPUT"));

      portable_sysTimerStart();

      if (portable_system(Config_getString("QHG_LOCATION"), qPrintable(args), false)) {
         err("Failed to run qhelpgenerator on index.qhp\n");
      }

      portable_sysTimerStop();
      QDir::setCurrent(oldDir);
      Doxy_Globals::g_stats.end();
   }

   msg("Lookup cache used %d/%d \n", Doxygen::lookupCache->count(), Doxygen::lookupCache->size());

   if (Debug::isFlagSet(Debug::Time)) {
      msg("Total elapsed time: %.3f seconds\n(of which %.3f seconds waiting for external tools to finish)\n",
          ((double)Doxygen::runningTime.elapsed()) / 1000.0, portable_getSysElapsedTime());

      Doxy_Globals::g_stats.print();

   } else {
      msg("Finished.\n");

   }

   // Start cleaning up
   cleanUpDoxygen();

   finializeSearchIndexer();
   Doxygen::symbolStorage->close();

   QDir thisDir;
   thisDir.remove(Doxygen::objDBFileName);
   Config::deleteInstance();

   delete Doxygen::symbolMap;
   delete Doxygen::clangUsrMap;
   delete Doxygen::symbolStorage;

   Doxy_Globals::g_successfulRun = true;
}

// ** other
ArgumentList *getTemplateArgumentsFromName( const QByteArray &name, const QList<ArgumentList> *tArgLists)
{
   if (tArgLists) {
      return 0;
   }

   auto item = tArgLists->begin();

   // for each scope fragment, check if it is a template and advance through the list if so.
   int i;
   int p = 0;

   while ((i = name.indexOf("::", p)) != -1) {
     QSharedPointer<NamespaceDef> nd (Doxygen::namespaceSDict->find(name.left(i)));

      if (nd) {
         ClassDef *cd = getClass(name.left(i));

         if (cd) {
            if (cd->templateArguments()) {
               ++item;
            }
         }
      }
      p = i + 2;
   }

   return const_cast<ArgumentList *> (&(*item));
}


// **  functions
void clearAll()
{
   Doxy_Globals::g_inputFiles.clear();

   Doxygen::classSDict->clear();
   Doxygen::namespaceSDict->clear();
   Doxygen::pageSDict->clear();
   Doxygen::exampleSDict->clear();
   Doxygen::inputNameList->clear();
   Doxygen::formulaList->clear();
   Doxygen::sectionDict->clear();
   Doxygen::inputNameDict->clear();
   Doxygen::includeNameDict->clear();
   Doxygen::exampleNameDict->clear();
   Doxygen::imageNameDict->clear();
   Doxygen::dotFileNameDict->clear();
   Doxygen::mscFileNameDict->clear();
   Doxygen::diaFileNameDict->clear();
   Doxygen::formulaDict->clear();
   Doxygen::formulaNameDict->clear();
   Doxygen::tagDestinationDict.clear();

   delete Doxygen::citeDict;
   delete Doxygen::mainPage;

   Doxygen::mainPage = 0;
}

void distributeClassGroupRelations()
{
   // static bool inlineGroupedClasses = Config_getBool("INLINE_GROUPED_CLASSES");

   for (auto cd : *Doxygen::classSDict) {
      cd->visited = false;
   }

   for (auto cd : *Doxygen::classSDict) {
      // distribute the group to nested classes as well

      GroupDef *gd = cd->partOfGroups()->at(0);

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
void Doxy_Work::addSTLMember(EntryNav *rootNav, const char *type, const char *name)
{
   Entry *memEntry      = new Entry;
   memEntry->name       = name;
   memEntry->type       = type;
   memEntry->protection = Public;
   memEntry->section    = Entry::VARIABLE_SEC;
   memEntry->brief      = "STL member";
   memEntry->hidden     = false;
   memEntry->artificial = true;

   //memEntry->parent     = root;
   //root->addSubEntry(memEntry);

   EntryNav *memEntryNav = new EntryNav(rootNav, memEntry);
   memEntryNav->setEntry(memEntry);
   rootNav->addChild(memEntryNav);
}

void Doxy_Work::addSTLIterator(EntryNav *classEntryNav, const char *name)
{
   Entry *iteratorClassEntry      = new Entry;
   iteratorClassEntry->fileName   = "[STL]";
   iteratorClassEntry->startLine  = 1;
   iteratorClassEntry->name       = name;
   iteratorClassEntry->section    = Entry::CLASS_SEC;
   iteratorClassEntry->brief      = "STL iterator class";
   iteratorClassEntry->hidden     = false;
   iteratorClassEntry->artificial = true;

   EntryNav *iteratorClassEntryNav = new EntryNav(classEntryNav, iteratorClassEntry);
   iteratorClassEntryNav->setEntry(iteratorClassEntry);
   classEntryNav->addChild(iteratorClassEntryNav);
}

void Doxy_Work::addSTLClasses(EntryNav *rootNav)
{
   Entry *namespaceEntry     = new Entry;
   namespaceEntry->fileName  = "[STL]";
   namespaceEntry->startLine = 1;
   //namespaceEntry->parent  = rootNav->entry();
   namespaceEntry->name      = "std";
   namespaceEntry->section   = Entry::NAMESPACE_SEC;
   namespaceEntry->brief     = "STL namespace";
   namespaceEntry->hidden    = false;
   namespaceEntry->artificial = true;

   //root->addSubEntry(namespaceEntry);
   EntryNav *namespaceEntryNav = new EntryNav(rootNav, namespaceEntry);
   namespaceEntryNav->setEntry(namespaceEntry);
   rootNav->addChild(namespaceEntryNav);

   STLInfo *info = g_stlinfo;

   while (info->className) {
      //printf("Adding STL class %s\n",info->className);
      QByteArray fullName = info->className;
      fullName.prepend("std::");

      // add fake Entry for the class
      Entry *classEntry     = new Entry;
      classEntry->fileName  = "[STL]";
      classEntry->startLine = 1;
      classEntry->name      = fullName;

      //classEntry->parent   = namespaceEntry;
      classEntry->section    = Entry::CLASS_SEC;
      classEntry->brief      = "STL class";
      classEntry->hidden     = false;
      classEntry->artificial = true;

      //namespaceEntry->addSubEntry(classEntry);
      EntryNav *classEntryNav = new EntryNav(namespaceEntryNav, classEntry);
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

         Entry *memEntry      = new Entry;
         memEntry->name       = "operator->";
         memEntry->args       = "()";
         memEntry->type       = "T*";
         memEntry->protection = Public;
         memEntry->section    = Entry::FUNCTION_SEC;
         memEntry->brief      = "STL member";
         memEntry->hidden     = false;
         memEntry->artificial = false;

         EntryNav *memEntryNav = new EntryNav(classEntryNav, memEntry);
         memEntryNav->setEntry(memEntry);
         classEntryNav->addChild(memEntryNav);
      }

      if (info->baseClass1) {
         classEntry->extends->append(BaseInfo(info->baseClass1, Public, info->virtualInheritance ? Virtual : Normal));
      }

      if (info->baseClass2) {
         classEntry->extends->append(BaseInfo(info->baseClass2, Public, info->virtualInheritance ? Virtual : Normal));
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

void Doxy_Work::addPageToContext(PageDef *pd, EntryNav *rootNav)
{
   if (rootNav->parent()) {
      // add the page to it's scope
      QByteArray scope = rootNav->parent()->name();

      if (rootNav->parent()->section() == Entry::PACKAGEDOC_SEC) {
         scope = substitute(scope, ".", "::");
      }

      scope = stripAnonymousNamespaceScope(scope);
      scope += "::" + pd->name();
      Definition *d = findScopeFromQualifiedName(Doxygen::globalScope, scope, 0, rootNav->tagInfo());

      if (d) {
         pd->setPageScope(d);
      }
   }
}

void Doxy_Work::addRelatedPage(EntryNav *rootNav)
{
   Entry *root = rootNav->entry();
   QSharedPointer<GroupDef> gd;

   for (auto g : *root->groups) {
      if (!g.groupname.isEmpty() && (gd = Doxygen::groupSDict->find(g.groupname))) {
         break;
      }
   }

   QByteArray doc;

   if (root->brief.isEmpty()) {
      doc = root->doc + root->inbodyDocs;
   } else {
      doc = root->brief + "\n\n" + root->doc + root->inbodyDocs;
   }

   PageDef *pd = addRelatedPage(root->name, root->args, doc, root->anchors, root->docFile, root->docLine,
                                root->sli, gd.data(), rootNav->tagInfo(), root->lang );

   if (pd) {
      pd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
      pd->addSectionsToDefinition(root->anchors);
      pd->setShowToc(root->stat);
      addPageToContext(pd, rootNav);
   }
}

void Doxy_Work::buildGroupListFiltered(EntryNav *rootNav, bool additional, bool includeExternal)
{
   if (rootNav->section() == Entry::GROUPDOC_SEC && !rootNav->name().isEmpty() &&
         ((! includeExternal && rootNav->tagInfo() == 0) || ( includeExternal && rootNav->tagInfo() != 0)) ) {

      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      if ((root->groupDocType == Entry::GROUPDOC_NORMAL && !additional) ||
            (root->groupDocType != Entry::GROUPDOC_NORMAL &&  additional)) {

         QSharedPointer<GroupDef> gd = Doxygen::groupSDict->find(root->name);

         //printf("Processing group '%s':'%s' add=%d ext=%d gd=%p\n",
         //    root->type.data(),root->name.data(),additional,includeExternal,gd);

         if (gd) {
            if ( !gd->hasGroupTitle() ) {
               gd->setGroupTitle( root->type );

            } else if ( root->type.length() > 0 && root->name != root->type && gd->groupTitle() != root->type ) {
               warn( root->fileName, root->startLine,
                     "group %s: ignoring title \"%s\" that does not match old title \"%s\"\n",
                     qPrint(root->name), qPrint(root->type), qPrint(gd->groupTitle()) );
            }

            gd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
            gd->setDocumentation( root->doc, root->docFile, root->docLine );
            gd->setInbodyDocumentation( root->inbodyDocs, root->inbodyFile, root->inbodyLine );
            gd->addSectionsToDefinition(root->anchors);
            gd->setRefItems(root->sli);
            gd->setLanguage(root->lang);

         } else {
            if (rootNav->tagInfo()) {
               gd = QSharedPointer<GroupDef> (new GroupDef(root->fileName, root->startLine, root->name, root->type, rootNav->tagInfo()->fileName));
               gd->setReference(rootNav->tagInfo()->tagName);

            } else {
               gd = QSharedPointer<GroupDef> (new GroupDef(root->fileName, root->startLine, root->name, root->type));
            }

            gd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
            // allow empty docs for group
            gd->setDocumentation(!root->doc.isEmpty() ? root->doc : QByteArray(" "), root->docFile, root->docLine, false);
            gd->setInbodyDocumentation( root->inbodyDocs, root->inbodyFile, root->inbodyLine );
            gd->addSectionsToDefinition(root->anchors);

            Doxygen::groupSDict->insert(root->name, gd);
            gd->setRefItems(root->sli);
            gd->setLanguage(root->lang);
         }
      }

      rootNav->releaseEntry();
   }

   for (auto e : rootNav->children() ) {
      buildGroupListFiltered(&e, additional, includeExternal);
   }

}

void Doxy_Work::buildGroupList(EntryNav *rootNav)
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

void Doxy_Work::findGroupScope(EntryNav *rootNav)
{
   if (rootNav->section() == Entry::GROUPDOC_SEC && !rootNav->name().isEmpty() &&
         rootNav->parent() && !rootNav->parent()->name().isEmpty()) {

      QSharedPointer<GroupDef> gd;

      if ((gd = Doxygen::groupSDict->find(rootNav->name()))) {
         QByteArray scope = rootNav->parent()->name();

         if (rootNav->parent()->section() == Entry::PACKAGEDOC_SEC) {
            scope = substitute(scope, ".", "::");
         }

         scope = stripAnonymousNamespaceScope(scope);
         scope += "::" + gd->name();
         Definition *d = findScopeFromQualifiedName(Doxygen::globalScope, scope, 0, rootNav->tagInfo());
         if (d) {
            gd->setGroupScope(d);
         }
      }
   }
   RECURSE_ENTRYTREE(findGroupScope, rootNav);
}

void Doxy_Work::organizeSubGroupsFiltered(EntryNav *rootNav, bool additional)
{
   if (rootNav->section() == Entry::GROUPDOC_SEC && !rootNav->name().isEmpty()) {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      if ((root->groupDocType == Entry::GROUPDOC_NORMAL && !additional) ||
            (root->groupDocType != Entry::GROUPDOC_NORMAL && additional)) {

         QSharedPointer<GroupDef> gd;

         if ((gd = Doxygen::groupSDict->find(root->name))) {
            //printf("adding %s to group %s\n",root->name.data(),gd->name().data());
            addGroupToGroups(root, gd.data());
         }
      }

      rootNav->releaseEntry();
   }

   for (auto e : rootNav->children() ) {
      organizeSubGroupsFiltered(&e, additional);
   }
}

void Doxy_Work::organizeSubGroups(EntryNav *rootNav)
{
   //printf("Defining groups\n");
   // first process the @defgroups blocks
   organizeSubGroupsFiltered(rootNav, false);

   //printf("Additional groups\n");
   // then process the @addtogroup, @weakgroup blocks
   organizeSubGroupsFiltered(rootNav, true);
}

void Doxy_Work::buildFileList(EntryNav *rootNav)
{
   if (((rootNav->section() == Entry::FILEDOC_SEC) ||
         ((rootNav->section() & Entry::FILE_MASK) && Config_getBool("EXTRACT_ALL"))) &&
         ! rootNav->name().isEmpty() && !rootNav->tagInfo() ) {

      // skip any file coming from tag files
      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      bool ambig;
      FileDef *fd = findFileDef(Doxygen::inputNameDict, root->name, ambig);

      //printf("**************** root->name=%s fd=%p\n",root->name.data(),fd);
      if (fd && !ambig) {
         {
            //printf("Adding documentation!\n");
            // using false in setDocumentation is small hack to make sure a file
            // is documented even if a \file command is used without further documentation

            fd->setDocumentation(root->doc, root->docFile, root->docLine, false);
            fd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
            fd->addSectionsToDefinition(root->anchors);
            fd->setRefItems(root->sli);

            for (auto g : *root->groups) {
               QSharedPointer<GroupDef> gd;

               if (! g.groupname.isEmpty() && (gd = Doxygen::groupSDict->find(g.groupname))) {
                  gd->addFile(fd);
                  fd->makePartOfGroup(gd.data());
               }
            }
         }

      } else {
         const char *fn = root->fileName.data();

         QString text;
         text = QString("the name `%1' supplied as the second argument in the \\file statement ").arg(QString(root->name));

         if (ambig) {
            // name is ambiguous

            text += "matches the following input files:\n";
            text += showFileDefMatches(Doxygen::inputNameDict, root->name);
            text += "Please use a more specific name by including a (larger) part of the path!";

         } else { // name is not an input file
            text += "is not an input file";
         }

         warn(fn, root->startLine, qPrintable(text));
      }

      rootNav->releaseEntry();
   }

   RECURSE_ENTRYTREE(buildFileList, rootNav);
}

void Doxy_Work::addIncludeFile(ClassDef *cd, FileDef *ifd, Entry *root)
{
   if ((!root->doc.trimmed().isEmpty() || !root->brief.trimmed().isEmpty() || Config_getBool("EXTRACT_ALL")) && root->protection != Private)  {
      //printf(">>>>>> includeFile=%s\n",root->includeFile.data());

      bool local = Config_getBool("FORCE_LOCAL_INCLUDES");
      QByteArray includeFile = root->includeFile;

      if (!includeFile.isEmpty() && includeFile.at(0) == '"') {
         local = true;
         includeFile = includeFile.mid(1, includeFile.length() - 2);

      } else if (!includeFile.isEmpty() && includeFile.at(0) == '<') {
         local = false;
         includeFile = includeFile.mid(1, includeFile.length() - 2);
      }

      bool ambig;
      FileDef *fd = 0;

      // see if we need to include a verbatim copy of the header file
      //printf("root->includeFile=%s\n",root->includeFile.data());

      if (!includeFile.isEmpty() && (fd = findFileDef(Doxygen::inputNameDict, includeFile, ambig)) == 0) {
         // explicit request

         QString text;
         text = QString("the name `%1' supplied as the argument of the \\class, \\struct, \\union, or \\include command ").
                        arg( QString(includeFile) );

         if (ambig) { // name is ambiguous
            text += "matches the following input files:\n";
            text += showFileDefMatches(Doxygen::inputNameDict, root->includeFile);
            text += "Please use a more specific name by including a (larger) part of the path!";

         } else { // name is not an input file
            text += "is not an input file";
         }

         warn(root->fileName, root->startLine, qPrintable(text) );

      } else if (includeFile.isEmpty() && ifd && guessSection(ifd->name()) == Entry::HEADER_SEC) {
         // see if the file extension makes sense
         // implicit assumption

         fd = ifd;
      }

      // if a file is found, we mark it as a source file.
      if (fd) {

         QString iName = !root->includeName.isEmpty() ? root->includeName : includeFile;

         if (!iName.isEmpty()) { // user specified include file
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

         } else if (! Config_getList("STRIP_FROM_INC_PATH").isEmpty()) {
            iName = stripFromIncludePath(fd->getFilePath());

         } else {
            // use name of the file containing the class definition
            iName = fd->name();
         }

         if (fd->generateSourceFile()) { // generate code for header
            cd->setIncludeFile(fd, iName, local, !root->includeName.isEmpty());

         } else { // put #include in the class documentation without link
            cd->setIncludeFile(0, iName, local, true);
         }
      }
   }
}

/*! returns the Definition object belonging to the first \a level levels of
 *  full qualified name \a name. Creates an artificial scope if the scope is
 *  not found and set the parent/child scope relation if the scope is found.
 */
Definition *Doxy_Work::buildScopeFromQualifiedName(const QByteArray name, int level, SrcLangExt lang, TagInfo *tagInfo)
{
   //printf("buildScopeFromQualifiedName(%s) level=%d\n",name.data(),level);
   int i = 0;
   int p = 0, l;

   Definition *prevScope = Doxygen::globalScope;
   QByteArray fullScope;

   while (i < level) {
      int idx = getScopeFragment(name, p, &l);
      QByteArray nsName = name.mid(idx, l);

      if (nsName.isEmpty()) {
         return prevScope;
      }

      if (!fullScope.isEmpty()) {
         fullScope += "::";
      }

      fullScope += nsName;
      QSharedPointer<NamespaceDef> nd (Doxygen::namespaceSDict->find(fullScope));

      QSharedPointer<Definition> innerScope = nd;
      ClassDef *cd = 0;

      if (nd) {
         cd = getClass(fullScope);
      }

      if (nd && cd) {
         // scope is a class
         innerScope = cd;

      } else if (nd == 0 && cd == 0 && fullScope.indexOf('<') == -1) {
         // scope is not known and could be a namespace
         // introduce bogus namespace

         nd = QSharedPointer<NamespaceDef> (new NamespaceDef( "[generated]", 1, 1, fullScope, tagInfo ? tagInfo->tagName : QByteArray(),
            tagInfo ? tagInfo->fileName : QByteArray()));

         nd->setLanguage(lang);

         // add namespace to the list
         Doxygen::namespaceSDict->insert(fullScope, nd);
         innerScope = nd.data();

      } else {
         // scope is a namespace

      }

      if (innerScope) {
         // make the parent/child scope relation
         prevScope->addInnerCompound(innerScope);
         innerScope->setOuterScope(prevScope);

      } else {
         // current scope is a class, so return only the namespace part...
         return prevScope;
      }

      // proceed to the next scope fragment
      p = idx + l + 2;
      prevScope = innerScope;
      i++;
   }
   return prevScope;
}

Definition *Doxy_Work::findScopeFromQualifiedName(QSharedPointer<Definition> startScope, const QByteArray &n, FileDef *fileScope, TagInfo *tagInfo)
{
   Definition *resultScope = startScope.data();

   if (resultScope == 0) {
      resultScope = Doxygen::globalScope;
   }

   QByteArray scope = stripTemplateSpecifiersFromScope(n, false);
   int l1 = 0, i1;
   i1 = getScopeFragment(scope, 0, &l1);

   if (i1 == -1) {
      //printf(">no fragments!\n");
      return resultScope;
   }

   int p = i1 + l1;
   int l2 = 0;
   int i2;

   while ((i2 = getScopeFragment(scope, p, &l2)) != -1) {
      QByteArray nestedNameSpecifier = scope.mid(i1, l1);
      Definition *orgScope = resultScope;

      resultScope = resultScope->findInnerCompound(nestedNameSpecifier);

      if (resultScope == 0) {
         NamespaceSDict *usedNamespaces;

         if (orgScope == Doxygen::globalScope && fileScope && (usedNamespaces = fileScope->getUsedNamespaces()))  {
            // also search for used namespaces

            for (auto nd : *usedNamespaces) {
               // restart search within the used namespace

               if (resultScope != 0) {
                  break;
               }

               resultScope = findScopeFromQualifiedName(nd.data(), n, fileScope, tagInfo);
            }

            if (resultScope) {
               // for a nested class A::I in used namespace N, we get
               // N::A::I while looking for A, so we should compare
               // resultScope->name() against scope.left(i2+l2)
               //printf("  -> result=%s scope=%s\n",resultScope->name().data(),scope.data());

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

            if (rightScopeMatch(item.key().toUtf8(), nestedNameSpecifier)) {
               // ui.currentKey() is the fully qualified name of nestedNameSpecifier
               // so use this instead.

               QByteArray fqn = QByteArray(item.key().toUtf8()) + scope.right(scope.length() - p);
               resultScope = buildScopeFromQualifiedName(fqn, fqn.count("::"), startScope->getLanguage(), 0);

               if (resultScope) {
                   return resultScope;
               }
            }

            ++item;
         }

         //printf("> name %s not found in scope %s\n",nestedNameSpecifier.data(),orgScope->name().data());
         return 0;
      }

   nextFragment:
      i1 = i2;
      l1 = l2;
      p = i2 + l2;
   }

   return resultScope;
}

ClassDef::CompoundType Doxy_Work::convertToCompoundType(int section, uint64_t specifier)
{
   ClassDef::CompoundType sec = ClassDef::Class;
   if (specifier & Entry::Struct) {
      sec = ClassDef::Struct;
   } else if (specifier & Entry::Union) {
      sec = ClassDef::Union;
   } else if (specifier & Entry::Category) {
      sec = ClassDef::Category;
   } else if (specifier & Entry::Interface) {
      sec = ClassDef::Interface;
   } else if (specifier & Entry::Protocol) {
      sec = ClassDef::Protocol;
   } else if (specifier & Entry::Exception) {
      sec = ClassDef::Exception;
   } else if (specifier & Entry::Service) {
      sec = ClassDef::Service;
   } else if (specifier & Entry::Singleton) {
      sec = ClassDef::Singleton;
   }

   switch (section) {
      //case Entry::UNION_SEC:
      case Entry::UNIONDOC_SEC:
         sec = ClassDef::Union;
         break;
      //case Entry::STRUCT_SEC:
      case Entry::STRUCTDOC_SEC:
         sec = ClassDef::Struct;
         break;
      //case Entry::INTERFACE_SEC:
      case Entry::INTERFACEDOC_SEC:
         sec = ClassDef::Interface;
         break;
      //case Entry::PROTOCOL_SEC:
      case Entry::PROTOCOLDOC_SEC:
         sec = ClassDef::Protocol;
         break;
      //case Entry::CATEGORY_SEC:
      case Entry::CATEGORYDOC_SEC:
         sec = ClassDef::Category;
         break;
      //case Entry::EXCEPTION_SEC:
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

void Doxy_Work::addClassToContext(EntryNav *rootNav)
{
   //printf("Loading entry for rootNav=%p name=%s\n",rootNav,rootNav->name().data());
   rootNav->loadEntry(Doxy_Globals::g_storage);
   Entry *root = rootNav->entry();

   //NamespaceDef *nd = 0;
   FileDef *fd = rootNav->fileDef();

   QByteArray scName;
   if (rootNav->parent()->section()&Entry::SCOPE_MASK) {
      scName = rootNav->parent()->name();
   }
   // name without parent's scope
   QByteArray fullName = root->name;

   // strip off any template parameters (but not those for specializations)
   fullName = stripTemplateSpecifiersFromScope(fullName);

   // name with scope (if not present already)
   QByteArray qualifiedName = fullName;
   if (!scName.isEmpty() && !leftScopeMatch(fullName, scName)) {
      qualifiedName.prepend(scName + "::");
   }

   // see if we already found the class before
   ClassDef *cd = getClass(qualifiedName);

   Debug::print(Debug::Classes, 0, "  Found class with name %s (qualifiedName=%s -> cd=%p)\n",
                cd ? cd->name().data() : root->name.data(), qualifiedName.data(), cd);

   if (cd) {
      fullName = cd->name();
      Debug::print(Debug::Classes, 0, "  Existing class %s!\n", cd->name().data());
      //if (cd->templateArguments()==0)
      //{
      //  //printf("existing ClassDef tempArgList=%p specScope=%s\n",root->tArgList,root->scopeSpec.data());
      //  cd->setTemplateArguments(tArgList);
      //}

      cd->setDocumentation(root->doc, root->docFile, root->docLine);
      cd->setBriefDescription(root->brief, root->briefFile, root->briefLine);

      if (root->bodyLine != -1 && cd->getStartBodyLine() == -1) {
         cd->setBodySegment(root->bodyLine, root->endBodyLine);
         cd->setBodyDef(fd);
      }
      //cd->setName(fullName); // change name to match docs

      if (cd->templateArguments() == 0 || (cd->isForwardDeclared() && (root->spec & Entry::ForwardDecl) == 0)) {
         // this happens if a template class declared with @class is found
         // before the actual definition or if a forward declaration has different template
         // parameter names.
         ArgumentList *tArgList =
            getTemplateArgumentsFromName(cd->name(), root->tArgLists);
         cd->setTemplateArguments(tArgList);
      }

      cd->setCompoundType(convertToCompoundType(root->section, root->spec));

   } else { // new class
      ClassDef::CompoundType sec = convertToCompoundType(root->section, root->spec);

      QByteArray className;
      QByteArray namespaceName;
      extractNamespaceName(fullName, className, namespaceName);

      //printf("New class: fullname %s namespace `%s' name=`%s' brief=`%s' docs=`%s'\n",
      //    fullName.data(),namespaceName.data(),className.data(),root->brief.data(),root->doc.data());

      QByteArray tagName;
      QByteArray refFileName;
      TagInfo *tagInfo = rootNav->tagInfo();
      int i;

      if (tagInfo) {
         tagName     = tagInfo->tagName;
         refFileName = tagInfo->fileName;

         if (fullName.indexOf("::") != -1)
            // symbols imported via tag files may come without the parent scope,
            // so we artificially create it here
         {
            buildScopeFromQualifiedName(fullName, fullName.count("::"), root->lang, tagInfo);
         }
      }

      ArgumentList *tArgList = 0;
      if ((root->lang == SrcLangExt_CSharp || root->lang == SrcLangExt_Java) && (i = fullName.indexOf('<')) != -1) {
         // a Java/C# generic class looks like a C++ specialization, so we need to split the
         // name and template arguments here
         tArgList = new ArgumentList;
         stringToArgumentList(fullName.mid(i), tArgList);
         fullName = fullName.left(i);

      } else {
         tArgList = getTemplateArgumentsFromName(fullName, root->tArgLists);
      }

      cd = new ClassDef(root->fileName, root->startLine, root->startColumn,
                        fullName, sec, tagName, refFileName, true, root->spec & Entry::Enum);

      Debug::print(Debug::Classes, 0, "  New class `%s' (sec=0x%08x)! #tArgLists=%d tagInfo=%p\n",
                   fullName.data(), sec, root->tArgLists ? (int)root->tArgLists->count() : -1, tagInfo);

      cd->setDocumentation(root->doc, root->docFile, root->docLine); // copy docs to definition
      cd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
      cd->setLanguage(root->lang);
      cd->setId(root->id);
      cd->setHidden(root->hidden);
      cd->setArtificial(root->artificial);
      cd->setClassSpecifier(root->spec);

      cd->setTypeConstraints(&root->typeConstr);

      cd->setTemplateArguments(tArgList);
      cd->setProtection(root->protection);
      cd->setIsStatic(root->stat);

      // file definition containing the class cd
      cd->setBodySegment(root->bodyLine, root->endBodyLine);
      cd->setBodyDef(fd);

      // see if the class is found inside a namespace
      cd->insertUsedFile(fd);

      // add class to the list
      Doxygen::classSDict->insert(fullName, QSharedPointer<ClassDef> (cd) );

      if (cd->isGeneric()) {
         // generics are also stored in a separate dictionary for fast lookup of instantions
         Doxygen::genericsDict->insert(fullName, cd);
      }
   }

   cd->addSectionsToDefinition(root->anchors);
   if (!root->subGrouping) {
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
// and all classes that have a documentation block before their definition.
void Doxy_Work::buildClassList(EntryNav *rootNav)
{
   if (((rootNav->section() & Entry::COMPOUND_MASK) || rootNav->section() == Entry::OBJCIMPL_SEC) && !rootNav->name().isEmpty()) {
      addClassToContext(rootNav);
   }

   RECURSE_ENTRYTREE(buildClassList, rootNav);
}

void Doxy_Work::buildClassDocList(EntryNav *rootNav)
{
   if (
      (rootNav->section() & Entry::COMPOUNDDOC_MASK) && !rootNav->name().isEmpty()
   ) {
      addClassToContext(rootNav);
   }
   RECURSE_ENTRYTREE(buildClassDocList, rootNav);
}

void Doxy_Work::resolveClassNestingRelations()
{
   ClassSDict::Iterator cli(*Doxygen::classSDict);

   for (cli.toFirst(); cli.current(); ++cli) {
      cli.current()->visited = false;
   }

   bool done = false;
   int iteration = 0;

   while (!done) {
      done = true;
      ++iteration;

      QSharedPointer<ClassDef> cd;

      for (cli.toFirst(); (cd = cli.current()); ++cli) {

         if (! cd->visited) {
            QByteArray name = stripAnonymousNamespaceScope(cd->name());

            // also add class to the correct structural context
            Definition *d = findScopeFromQualifiedName(Doxygen::globalScope, name, cd->getFileDef(), 0);

            if (d) {
               d->addInnerCompound(cd.data());

               cd->setOuterScope(d);
               cd->visited = true;

               done = false;
            }

         }
      }
   }

   //give warnings for unresolved compounds
   QSharedPointer<ClassDef> cd;

   for (cli.toFirst(); (cd = cli.current()); ++cli) {

      if (! cd->visited) {
         QByteArray name = stripAnonymousNamespaceScope(cd->name());

         /// create the scope artificially
         // anyway, so we can at least relate scopes properly.

         Definition *d = buildScopeFromQualifiedName(name, name.count("::"), cd->getLanguage(), 0);

         if (d != cd && !cd->getDefFileName().isEmpty())
            // avoid recursion in case of redundant scopes, i.e: namespace N { class N::C {}; }
            // for this case doxygen assumes the exitance of a namespace N::N in which C is to be found!
            // also avoid warning for stuff imported via a tagfile.
         {
            d->addInnerCompound(cd.data());
            cd->setOuterScope(d);
            warn(cd->getDefFileName(), cd->getDefLine(), "Internal inconsistency: scope for class %s not found", name.data());
         }
      }
   }
}

ClassDef *Doxy_Work::createTagLessInstance(ClassDef *rootCd, ClassDef *templ, const QByteArray &fieldName)
{
   QByteArray fullName = removeAnonymousScopes(templ->name());

   if (fullName.right(2) == "::") {
      fullName = fullName.left(fullName.length() - 2);
   }

   fullName += "." + fieldName;
   ClassDef *cd = new ClassDef(templ->getDefFileName(), templ->getDefLine(), templ->getDefColumn(), fullName,
                               templ->compoundType());

   cd->setDocumentation(templ->documentation(), templ->docFile(), templ->docLine()); // copy docs to definition
   cd->setBriefDescription(templ->briefDescription(), templ->briefFile(), templ->briefLine());
   cd->setLanguage(templ->getLanguage());
   cd->setBodySegment(templ->getStartBodyLine(), templ->getEndBodyLine());
   cd->setBodyDef(templ->getBodyDef());

   cd->setOuterScope(rootCd->getOuterScope());
   if (rootCd->getOuterScope() != Doxygen::globalScope) {
      rootCd->getOuterScope()->addInnerCompound(cd);
   }

   FileDef *fd = templ->getFileDef();

   if (fd) {
      cd->setFileDef(fd);
      fd->insertClass(cd);
   }

   SortedList<GroupDef *> *groups = rootCd->partOfGroups();

   if ( groups != 0 ) {
      for (auto item : *groups) {
         cd->makePartOfGroup(item);
         item->addClass(cd);
      }
   }
   //printf("** adding class %s based on %s\n",fullName.data(),templ->name().data());
   Doxygen::classSDict->insert(fullName, QSharedPointer<ClassDef> (cd) );

   MemberList *ml = templ->getMemberList(MemberListType_pubAttribs);

   if (ml) {

      for (auto md : *ml) {
         //printf("    Member %s type=%s\n",md->name().data(),md->typeString());
         MemberDef *imd = new MemberDef(md->getDefFileName(), md->getDefLine(), md->getDefColumn(),
                                        md->typeString(), md->name(), md->argsString(), md->excpString(),
                                        md->protection(), md->virtualness(), md->isStatic(), Member, md->memberType(), 0, 0);

         imd->setMemberClass(cd);
         imd->setDocumentation(md->documentation(), md->docFile(), md->docLine());
         imd->setBriefDescription(md->briefDescription(), md->briefFile(), md->briefLine());
         imd->setInbodyDocumentation(md->inbodyDocumentation(), md->inbodyFile(), md->inbodyLine());
         imd->setMemberSpecifiers(md->getMemberSpecifiers());
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
void Doxy_Work::processTagLessClasses(ClassDef *rootCd, ClassDef *cd,ClassDef *tagParentCd, const QByteArray &prefix, int count)
{
   //printf("%d: processTagLessClasses %s\n",count,cd->name().data());
   //printf("checking members for %s\n",cd->name().data());

   if (cd->getClassSDict()) {
      MemberList *ml = cd->getMemberList(MemberListType_pubAttribs);

      if (ml) {

         for (auto md : *ml) {
            QByteArray type = md->typeString();

            if (type.indexOf("::@") != -1) {
               // member of tag less struct/union

               for (auto icd : *cd->getClassSDict()) {

                  if (type.indexOf(icd->name()) != -1) {
                     // matching tag less struct/union
                     QByteArray name = md->name();

                     if (name.at(0) == '@') {
                        name = "__unnamed__";
                     }

                     if (!prefix.isEmpty()) {
                        name.prepend(prefix + ".");
                     }

                     QSharedPointer<ClassDef> ncd (createTagLessInstance(rootCd, icd.data(), name));

                     processTagLessClasses(rootCd, icd.data(), ncd.data(), name, count + 1);

                     tagParentCd->addTaggedInnerClass(ncd.data());
                     ncd->setTagLessReference(icd.data());

                     // replace tag-less type for generated/original member
                     // by newly created class name.
                     // note the difference between changing cd and tagParentCd.
                     // for the initial call this is the same pointer, but for
                     // recursive calls cd is the original tag-less struct (of which
                     // there is only one instance) and tagParentCd is the newly
                     // generated tagged struct of which there can be multiple instances!

                     MemberList *pml = tagParentCd->getMemberList(MemberListType_pubAttribs);

                     if (pml) {

                        for (auto pmd : *pml) {
                           if (pmd->name() == md->name()) {
                              pmd->setAccessorType(ncd.data(), substitute(pmd->typeString(), icd->name(), ncd->name()));
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

void Doxy_Work::findTagLessClasses(ClassDef *cd)
{
   if (cd->getClassSDict()) {
      for (auto icd : *cd->getClassSDict()) {
         if (icd->name().indexOf("@") == -1) { // process all non-anonymous inner classes
            findTagLessClasses(icd.data());
         }
      }
   }

   processTagLessClasses(cd, cd, cd, "", 0); // process tag less inner struct/classes (if any)
}

void Doxy_Work::findTagLessClasses()
{
   for (auto cd : *Doxygen::classSDict) {
      Definition *scope = cd->getOuterScope();
      if (scope && scope->definitionType() != Definition::TypeClass) { // that is not nested
         findTagLessClasses(cd.data());
      }
   }
}

// build a list of all namespaces mentioned in the documentation
// and all namespaces that have a documentation block before their definition.
void Doxy_Work::buildNamespaceList(EntryNav *rootNav)
{
   if ( (rootNav->section() == Entry::NAMESPACE_SEC || rootNav->section() == Entry::NAMESPACEDOC_SEC ||
       rootNav->section() == Entry::PACKAGEDOC_SEC) && !rootNav->name().isEmpty()) {

      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      //printf("** buildNamespaceList(%s)\n",root->name.data());

      QByteArray fName = root->name;
      if (root->section == Entry::PACKAGEDOC_SEC) {
         fName = substitute(fName, ".", "::");
      }

      QByteArray fullName = stripAnonymousNamespaceScope(fName);
      if (!fullName.isEmpty()) {

         QSharedPointer<NamespaceDef> nd;

         if ((nd = Doxygen::namespaceSDict->find(fullName))) {
            // existing namespace

            nd->setDocumentation(root->doc, root->docFile, root->docLine);
            nd->setName(fullName); // change name to match docs
            nd->addSectionsToDefinition(root->anchors);

            nd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
            if (nd->getLanguage() == SrcLangExt_Unknown) {
               nd->setLanguage(root->lang);
            }

            // file definition containing the namespace nd
            FileDef *fd = rootNav->fileDef();
            // insert the namespace in the file definition
            if (fd) {
               fd->insertNamespace(nd);
            }
            addNamespaceToGroups(root, nd);
            nd->setRefItems(root->sli);

         } else { // fresh namespace
            QByteArray tagName;
            QByteArray tagFileName;

            TagInfo *tagInfo = rootNav->tagInfo();
            if (tagInfo) {
               tagName     = tagInfo->tagName;
               tagFileName = tagInfo->fileName;
            }

            //printf("++ new namespace %s lang=%s tagName=%s\n",fullName.data(),langToString(root->lang).data(),tagName.data());
            NamespaceDef *nd = new NamespaceDef(root->fileName, root->startLine,
                                                root->startColumn, fullName, tagName, tagFileName,
                                                root->type, root->spec & Entry::Published);

            nd->setDocumentation(root->doc, root->docFile, root->docLine); // copy docs to definition
            nd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
            nd->addSectionsToDefinition(root->anchors);
            nd->setHidden(root->hidden);
            nd->setArtificial(root->artificial);
            nd->setLanguage(root->lang);
            nd->setId(root->id);

            //printf("Adding namespace to group\n");
            addNamespaceToGroups(root, nd);
            nd->setRefItems(root->sli);

            // file definition containing the namespace nd
            FileDef *fd = rootNav->fileDef();

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
            Doxygen::namespaceSDict->insert(fullName, nd);

            // also add namespace to the correct structural context
            Definition *d = findScopeFromQualifiedName(Doxygen::globalScope, fullName, 0, tagInfo);

            if (d == 0)  {
               // we didn't find anything, create the scope artificially
               // anyway, so we can at least relate scopes properly.

               Definition *d = buildScopeFromQualifiedName(fullName, fullName.count("::"), nd->getLanguage(), tagInfo);
               d->addInnerCompound(nd);
               nd->setOuterScope(d);

               // TODO: Due to the order in which the tag file is written
               // a nested class can be found before its parent!

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

NamespaceDef *Doxy_Work::findUsedNamespace(NamespaceSDict *unl, const QByteArray &name)
{
   NamespaceDef *usingNd = 0;

   if (unl) {
      for (auto und : *unl ) {
         QByteArray uScope = und->name() + "::";
         usingNd = getResolvedNamespace(uScope + name);
      }
   }
   return usingNd;
}

void Doxy_Work::findUsingDirectives(EntryNav *rootNav)
{
   if (rootNav->section() == Entry::USINGDIR_SEC) {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      //printf("Found using directive %s at line %d of %s\n",
      //    root->name.data(),root->startLine,root->fileName.data());
      QByteArray name = substitute(root->name, ".", "::");
      if (name.right(2) == "::") {
         name = name.left(name.length() - 2);
      }
      if (!name.isEmpty()) {
         NamespaceDef *usingNd = 0;
         NamespaceDef *nd = 0;
         FileDef      *fd = rootNav->fileDef();
         QByteArray nsName;

         // see if the using statement was found inside a namespace or inside
         // the global file scope.
         if (rootNav->parent() && rootNav->parent()->section() == Entry::NAMESPACE_SEC &&
               (fd == 0 || fd->getLanguage() != SrcLangExt_Java) // not a .java file
            ) {
            nsName = stripAnonymousNamespaceScope(rootNav->parent()->name());
            if (!nsName.isEmpty()) {
               nd = getResolvedNamespace(nsName);
            }
         }

         // find the scope in which the `using' namespace is defined by prepending
         // the possible scopes in which the using statement was found, starting
         // with the most inner scope and going to the most outer scope (i.e.
         // file scope).
         int scopeOffset = nsName.length();
         do {
            QByteArray scope = scopeOffset > 0 ?
                               nsName.left(scopeOffset) + "::" : QByteArray();
            usingNd = getResolvedNamespace(scope + name);
            //printf("Trying with scope=`%s' usingNd=%p\n",(scope+name).data(),usingNd);
            if (scopeOffset == 0) {
               scopeOffset = -1;
            } else if ((scopeOffset = nsName.lastIndexOf("::", scopeOffset - 1)) == -1) {
               scopeOffset = 0;
            }
         } while (scopeOffset >= 0 && usingNd == 0);

         if (usingNd == 0 && nd) // not found, try used namespaces in this scope
            // or in one of the parent namespace scopes
         {
            NamespaceDef *pnd = nd;
            while (pnd && usingNd == 0) {
               // also try with one of the used namespaces found earlier
               usingNd = findUsedNamespace(pnd->getUsedNamespaces(), name);

               // goto the parent
               Definition *s = pnd->getOuterScope();
               if (s && s->definitionType() == Definition::TypeNamespace) {
                  pnd = (NamespaceDef *)s;
               } else {
                  pnd = 0;
               }
            }
         }
         if (usingNd == 0 && fd) // still nothing, also try used namespace in the
            // global scope
         {
            usingNd = findUsedNamespace(fd->getUsedNamespaces(), name);
         }

         //printf("%s -> %s\n",name.data(),usingNd?usingNd->name().data():"<none>");

         // add the namespace the correct scope
         if (usingNd) {
            //printf("using fd=%p nd=%p\n",fd,nd);
            if (nd) {
               //printf("Inside namespace %s\n",nd->name().data());
               nd->addUsingDirective(usingNd);
            } else if (fd) {
               //printf("Inside file %s\n",fd->name().data());
               fd->addUsingDirective(usingNd);
            }
         } else { // unknown namespace, but add it anyway.
            //printf("++ new unknown namespace %s lang=%s\n",name.data(),langToString(root->lang).data());
            NamespaceDef *nd = new NamespaceDef(root->fileName, root->startLine, root->startColumn, name);
            nd->setDocumentation(root->doc, root->docFile, root->docLine); // copy docs to definition
            nd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
            nd->addSectionsToDefinition(root->anchors);

            //printf("** Adding namespace %s hidden=%d\n",name.data(),root->hidden);

            nd->setHidden(root->hidden);
            nd->setArtificial(true);
            nd->setLanguage(root->lang);
            nd->setId(root->id);

            for (auto g : *root->groups) {
               GroupDef *gd = 0;

               if (! g->groupname.isEmpty() && (gd = Doxygen::groupSDict->find(g->groupname))) {
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
            Doxygen::namespaceSDict->insert(name, nd);
            nd->setRefItems(root->sli);
         }
      }

      rootNav->releaseEntry();
   }

   RECURSE_ENTRYTREE(findUsingDirectives, rootNav);
}

void Doxy_Work::buildListOfUsingDecls(EntryNav *rootNav)
{
   if (rootNav->section() == Entry::USINGDECL_SEC && ! (rootNav->parent()->section()&Entry::COMPOUND_MASK)  ) {
      // not a class/struct member

      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      QByteArray name = substitute(root->name, ".", "::");

      if (! Doxy_Globals::g_usingDeclarations.contains(name)) {
         FileDef *fd = rootNav->fileDef();

         if (fd) {
            Doxy_Globals::g_usingDeclarations.insert(name, fd);
         }
      }

      rootNav->releaseEntry();
   }

   RECURSE_ENTRYTREE(buildListOfUsingDecls, rootNav);
}

void Doxy_Work::findUsingDeclarations(EntryNav *rootNav)
{
   if (rootNav->section() == Entry::USINGDECL_SEC &&
         !(rootNav->parent()->section()&Entry::COMPOUND_MASK) // not a class/struct member
      ) {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      //printf("Found using declaration %s at line %d of %s inside section %x\n",
      //   root->name.data(),root->startLine,root->fileName.data(),
      //   rootNav->parent()->section());
      if (!root->name.isEmpty()) {
         ClassDef *usingCd = 0;
         NamespaceDef *nd = 0;
         FileDef      *fd = rootNav->fileDef();
         QByteArray scName;

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
         // with the most inner scope and going to the most outer scope (i.e.
         // file scope).

         QByteArray name = substitute(root->name, ".", "::"); //Java/C# scope->internal
         usingCd = getClass(name);
         if (usingCd == 0) {
            usingCd = Doxygen::hiddenClasses->find(name);
         }

         //printf("%s -> %p\n",root->name.data(),usingCd);
         if (usingCd == 0) { // definition not in the input => add an artificial class
            Debug::print(Debug::Classes, 0, "  New using class `%s' (sec=0x%08x)! #tArgLists=%d\n",
                         name.data(), root->section, root->tArgLists ? (int)root->tArgLists->count() : -1);
            usingCd = new ClassDef(
               "<using>", 1, 1,
               name,
               ClassDef::Class);
            Doxygen::hiddenClasses->append(root->name, usingCd);
            usingCd->setArtificial(true);
            usingCd->setLanguage(root->lang);
         } else {
            Debug::print(Debug::Classes, 0, "  Found used class %s in scope=%s\n",
                         usingCd->name().data(), nd ? nd->name().data() : fd->name().data());
         }

         if (nd) {
            //printf("Inside namespace %s\n",nd->name().data());
            nd->addUsingDeclaration(usingCd);
         } else if (fd) {
            //printf("Inside file %s\n",fd->name().data());
            fd->addUsingDeclaration(usingCd);
         }
      }

      rootNav->releaseEntry();
   }
   RECURSE_ENTRYTREE(findUsingDeclarations, rootNav);
}

void Doxy_Work::findUsingDeclImports(EntryNav *rootNav)
{
   if (rootNav->section() == Entry::USINGDECL_SEC && (rootNav->parent()->section()&Entry::COMPOUND_MASK) )  {

      //printf("Found using declaration %s at line %d of %s inside section %x\n",
      //    root->name.data(),root->startLine,root->fileName.data(),
      //    root->parent->section);

      QByteArray fullName = removeRedundantWhiteSpace(rootNav->parent()->name());
      fullName = stripAnonymousNamespaceScope(fullName);
      fullName = stripTemplateSpecifiersFromScope(fullName);
      ClassDef *cd = getClass(fullName);

      if (cd) {
         //printf("found class %s\n",cd->name().data());
         int i = rootNav->name().find("::");

         if (i != -1) {
            QByteArray scope = rootNav->name().left(i);
            QByteArray memName = rootNav->name().right(rootNav->name().length() - i - 2);
            ClassDef *bcd = getResolvedClass(cd, 0, scope); // todo: file in fileScope parameter

            if (bcd) {
               //printf("found class %s\n",bcd->name().data());
               MemberNameInfoSDict *mndict = bcd->memberNameInfoSDict();

               if (mndict) {
                  MemberNameInfo *mni = mndict->find(memName);
                  if (mni) {
                     MemberNameInfoIterator mnii(*mni);
                     MemberInfo *mi;
                     for ( ; (mi = mnii.current()) ; ++mnii ) {
                        MemberDef *md = mi->memberDef;
                        if (md && md->protection() != Private) {

                           rootNav->loadEntry(Doxy_Globals::g_storage);
                           Entry *root = rootNav->entry();

                           //printf("found member %s\n",mni->memberName());
                           MemberDef *newMd = 0;
                           {
                              ArgumentList *templAl = md->templateArguments();
                              ArgumentList *al = md->templateArguments();
                              newMd = new MemberDef(
                                 root->fileName, root->startLine, root->startColumn,
                                 md->typeString(), memName, md->argsString(),
                                 md->excpString(), root->protection, root->virt,
                                 md->isStatic(), Member, md->memberType(),
                                 templAl, al
                              );
                           }
                           newMd->setMemberClass(cd);
                           cd->insertMember(newMd);
                           if (!root->doc.isEmpty() || !root->brief.isEmpty()) {
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
                           newMd->setMemberSpecifiers(md->getMemberSpecifiers());
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
   FileNameListIterator fnli(*Doxygen::inputNameList);
   FileName *fn;
   for (fnli.toFirst(); (fn = fnli.current()); ++fnli) {
      FileNameIterator fni(*fn);
      FileDef *fd;
      for (; (fd = fni.current()); ++fni) {
         fd->visited = false;
      }
   }
   // then recursively add using directives found in #include files
   // to files that have not been visited.
   for (fnli.toFirst(); (fn = fnli.current()); ++fnli) {
      FileNameIterator fni(*fn);
      FileDef *fd;
      for (fni.toFirst(); (fd = fni.current()); ++fni) {
         if (!fd->visited) {
            //printf("----- adding using directives for file %s\n",fd->name().data());
            fd->addIncludedUsingDirectives();
         }
      }
   }
}

MemberDef *Doxy_Work::addVariableToClass(EntryNav *rootNav, ClassDef *cd, MemberType mtype, const QByteArray &name,
                                     bool fromAnnScope, MemberDef *fromAnnMemb, Protection prot, Relationship related)
{
   Entry *root = rootNav->entry();

   QByteArray qualScope = cd->qualifiedNameWithTemplateParameters();
   QByteArray scopeSeparator = "::";
   SrcLangExt lang = cd->getLanguage();
   if (lang == SrcLangExt_Java || lang == SrcLangExt_CSharp) {
      qualScope = substitute(qualScope, "::", ".");
      scopeSeparator = ".";
   }
   Debug::print(Debug::Variables, 0,
                "  class variable:\n"
                "    `%s' `%s'::`%s' `%s' prot=`%d ann=%d init=`%s'\n",
                root->type.data(),
                qualScope.data(),
                name.data(),
                root->args.data(),
                root->protection,
                fromAnnScope,
                root->initializer.data()
               );

   QByteArray def;
   if (!root->type.isEmpty()) {
      if (related || mtype == MemberType_Friend || Config_getBool("HIDE_SCOPE_NAMES")) {
         if (root->spec & Entry::Alias) { // turn 'typedef B A' into 'using A = B'
            def = "using " + name + " = " + root->type.mid(7);
         } else {
            def = root->type + " " + name + root->args;
         }
      } else {
         if (root->spec & Entry::Alias) { // turn 'typedef B C::A' into 'using C::A = B'
            def = "using " + qualScope + scopeSeparator + name + " = " + root->type.mid(7);
         } else {
            def = root->type + " " + qualScope + scopeSeparator + name + root->args;
         }
      }
   } else {
      if (Config_getBool("HIDE_SCOPE_NAMES")) {
         def = name + root->args;
      } else {
         def = qualScope + scopeSeparator + name + root->args;
      }
   }

   def = stripPrefix(def, "static ");

   // see if the member is already found in the same scope
   // (this may be the case for a static member that is initialized
   //  outside the class)

   MemberName *mn = Doxygen::memberNameSDict->find(name);
   if (mn) {
      MemberNameIterator mni(*mn);
      MemberDef *md;

      for (mni.toFirst(); (md = mni.current()); ++mni) {
         //printf("md->getClassDef()=%p cd=%p type=[%s] md->typeString()=[%s]\n",
         //    md->getClassDef(),cd,root->type.data(),md->typeString());
         if (md->getClassDef() == cd &&
               removeRedundantWhiteSpace(root->type) == md->typeString())
            // member already in the scope
         {

            if (root->lang == SrcLangExt_ObjC &&
                  root->mtype == Property &&
                  md->memberType() == MemberType_Variable) {
               // Objective-C 2.0 property
               // turn variable into a property
               md->setProtection(root->protection);
               cd->reclassifyMember(md, MemberType_Property);
            }
            addMemberDocs(rootNav, md, def, 0, false);
            //printf("    Member already found!\n");
            return md;
         }
      }
   }

   // new member variable, typedef or enum value
   MemberDef *md = new MemberDef(
      root->fileName, root->startLine, root->startColumn,
      root->type, name, root->args, root->exception,
      prot, Normal, root->stat, related,
      mtype, root->tArgLists ? root->tArgLists->getLast() : 0, 0);
   md->setTagInfo(rootNav->tagInfo());
   md->setMemberClass(cd); // also sets outer scope (i.e. getOuterScope())
   //md->setDefFile(root->fileName);
   //md->setDefLine(root->startLine);
   md->setDocumentation(root->doc, root->docFile, root->docLine);
   md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
   md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
   md->setDefinition(def);
   md->setBitfields(root->bitfields);
   md->addSectionsToDefinition(root->anchors);
   md->setFromAnonymousScope(fromAnnScope);
   md->setFromAnonymousMember(fromAnnMemb);
   //md->setIndentDepth(indentDepth);
   md->setBodySegment(root->bodyLine, root->endBodyLine);
   md->setInitializer(root->initializer);
   md->setMaxInitLines(root->initLines);
   md->setMemberGroupId(root->mGrpId);
   md->setMemberSpecifiers(root->spec);
   md->setReadAccessor(root->read);
   md->setWriteAccessor(root->write);
   md->enableCallGraph(root->callGraph);
   md->enableCallerGraph(root->callerGraph);
   md->setHidden(root->hidden);
   md->setArtificial(root->artificial);
   md->setLanguage(root->lang);
   md->setId(root->id);
   addMemberToGroups(root, md);
   //if (root->mGrpId!=-1)
   //{
   //  printf("memberdef %s in memberGroup %d\n",name.data(),root->mGrpId);
   //  md->setMemberGroup(memberGroupDict[root->mGrpId]);
   //
   md->setBodyDef(rootNav->fileDef());

   //printf("    Adding member=%s\n",md->name().data());
   // add the member to the global list
   if (mn) {
      mn->append(md);
   } else { // new variable name
      mn = new MemberName(name);
      mn->append(md);
      //printf("Adding memberName=%s\n",mn->memberName());
      //Doxygen::memberNameDict.insert(name,mn);
      //Doxygen::memberNameList.append(mn);
      Doxygen::memberNameSDict->append(name, mn);
      // add the member to the class
   }
   //printf("    New member adding to %s (%p)!\n",cd->name().data(),cd);
   cd->insertMember(md);
   md->setRefItems(root->sli);

   //TODO: insert FileDef instead of filename strings.
   cd->insertUsedFile(rootNav->fileDef());
   rootNav->changeSection(Entry::EMPTY_SEC);
   return md;
}

MemberDef *Doxy_Work::addVariableToFile(EntryNav *rootNav, MemberType mtype, const QByteArray &scope, const QByteArray &name,
                                    bool fromAnnScope, MemberDef *fromAnnMemb)
{
   Entry *root = rootNav->entry();

   Debug::print(Debug::Variables, 0, "  global variable:\n"
                "    type=`%s' scope=`%s' name=`%s' args=`%s' prot=`%d mtype=%d lang=%d\n",
                root->type.data(), scope.data(), name.data(), root->args.data(), root->protection,
                mtype, root->lang );

   FileDef *fd = rootNav->fileDef();

   // see if we have a typedef that should hide a struct or union
   if (mtype == MemberType_Typedef && Config_getBool("TYPEDEF_HIDES_STRUCT")) {
      QByteArray type = root->type;
      type = stripPrefix(type, "typedef ");

      if (type.left(7) == "struct " || type.left(6) == "union ") {
         type = stripPrefix(type, "struct ");
         type = stripPrefix(type, "union ");

         static QRegExp re("[a-z_A-Z][a-z_A-Z0-9]*");
         int l, s;

         s = re.indexIn(type, 0);
         l = re.matchedLength();

         if (s >= 0) {
            QByteArray typeValue = type.mid(s, l);
            ClassDef *cd = getClass(typeValue);

            if (cd) {
               // this typedef should hide compound name cd, so we
               // change the name that is displayed from cd.
               cd->setClassName(name);
               cd->setDocumentation(root->doc, root->docFile, root->docLine);
               cd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
               return 0;
            }
         }
      }
   }

   // see if the function is inside a namespace
   NamespaceDef *nd = 0;

   if (! scope.isEmpty()) {
      if (scope.indexOf('@') != -1) {
         return 0;   // anonymous scope!
      }

      nd = getResolvedNamespace(scope);
   }

   QByteArray def;

   // determine the definition of the global variable
   if (nd && !nd->name().isEmpty() && nd->name().at(0) != '@' && !Config_getBool("HIDE_SCOPE_NAMES") )   {
      // variable is inside a namespace, so put the scope before the name

      SrcLangExt lang = nd->getLanguage();
      QByteArray sep = getLanguageSpecificSeparator(lang);

      if (!root->type.isEmpty()) {
         if (root->spec & Entry::Alias) { // turn 'typedef B NS::A' into 'using NS::A = B'
            def = "using " + nd->name() + sep + name + " = " + root->type;
         } else { // normal member
            def = root->type + " " + nd->name() + sep + name + root->args;
         }
      } else {
         def = nd->name() + sep + name + root->args;
      }
   } else {
      if (!root->type.isEmpty() && !root->name.isEmpty()) {
         if (name.at(0) == '@') { // dummy variable representing anonymous union
            def = root->type;
         } else {
            if (root->spec & Entry::Alias) { // turn 'typedef B A' into 'using A = B'
               def = "using " + root->name + " = " + root->type.mid(7);
            } else { // normal member
               def = root->type + " " + name + root->args;
            }
         }
      } else {
         def = name + root->args;
      }
   }
   def = stripPrefix(def, "static ");

   MemberName *mn = Doxygen::functionNameSDict->find(name);

   if (mn) {

      if (!scope.isEmpty()) {
         nd = getResolvedNamespace(scope);
      }

      MemberNameIterator mni(*mn);
      MemberDef *md;

      for (mni.toFirst(); (md = mni.current()); ++mni) {
         if (
            ((nd == 0 && md->getNamespaceDef() == 0 && md->getFileDef() &&
              root->fileName == md->getFileDef()->absoluteFilePath()
             ) // both variable names in the same file
             || (nd != 0 && md->getNamespaceDef() == nd) // both in same namespace
            )
            && !md->isDefine() // function style #define's can be "overloaded" by typedefs or variables
            && !md->isEnumerate() // in C# an enum value and enum can have the same name
         )
            // variable already in the scope
         {
            bool isPHPArray = md->getLanguage() == SrcLangExt_PHP &&
                              md->argsString() != root->args && root->args.find('[') != -1;

            bool staticsInDifferentFiles =
               root->stat && md->isStatic() &&
               root->fileName != md->getDefFileName();

            if (md->getFileDef() &&
                  !isPHPArray && // not a php array
                  !staticsInDifferentFiles
               )
               // not a php array variable
            {

               Debug::print(Debug::Variables, 0,
                            "    variable already found: scope=%s\n", md->getOuterScope()->name().data());
               addMemberDocs(rootNav, md, def, 0, false);
               md->setRefItems(root->sli);
               return md;
            }
         }
      }
   }
   Debug::print(Debug::Variables, 0,
                "    new variable, nd=%s!\n", nd ? nd->name().data() : "<global>");
   // new global variable, enum value or typedef
   MemberDef *md = new MemberDef(
      root->fileName, root->startLine, root->startColumn,
      root->type, name, root->args, 0,
      Public, Normal, root->stat, Member,
      mtype, root->tArgLists ? root->tArgLists->getLast() : 0, 0);
   md->setTagInfo(rootNav->tagInfo());
   md->setMemberSpecifiers(root->spec);
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
   //md->setOuterScope(fd);
   if (!root->explicitExternal) {
      md->setBodySegment(root->bodyLine, root->endBodyLine);
      md->setBodyDef(fd);
   }
   addMemberToGroups(root, md);

   md->setRefItems(root->sli);
   if (nd && !nd->name().isEmpty() && nd->name().at(0) != '@') {
      md->setNamespace(nd);
      nd->insertMember(md);
   }

   // add member to the file (we do this even if we have already inserted
   // it into the namespace.
   if (fd) {
      md->setFileDef(fd);
      fd->insertMember(md);
   }

   // add member definition to the list of globals
   if (mn) {
      mn->append(md);
   } else {
      mn = new MemberName(name);
      mn->append(md);
      Doxygen::functionNameSDict->append(name, mn);
   }
   rootNav->changeSection(Entry::EMPTY_SEC);
   return md;
}

/*! See if the return type string \a type is that of a function pointer
 *  \returns -1 if this is not a function pointer variable or
 *           the index at which the closing brace of (...*name) was found.
 */
int Doxy_Work::findFunctionPtr(const QByteArray &type, int lang, int *pLength = 0)
{
   if (lang == SrcLangExt_Fortran) {
      return -1;   // Fortran does not have function pointers
   }

   static const QRegExp re("([^)]*[\\*\\^][^)]*)");

   int i = -1, l;
   int bb = type.indexOf('<');
   int be = type.lastIndexOf('>');

   if (! type.isEmpty() && (i = re.indexIn(type, 0)) != -1 && // contains (...*...)
         type.indexOf("operator") == -1 && (type.indexOf(")(") == -1 || type.indexOf("typedef ") != -1) && ! (bb < i && i < be) ) {

      // bug665855: avoid treating "typedef A<void (T*)> type" as a function pointer
      l = re.matchedLength();

      if (pLength) {
         *pLength = l;
      }

      return i;

   } else {
      //printf("findFunctionPtr=%d\n",-1);
      return -1;
   }
}


/*! Returns true iff \a type is a class within scope \a context.
 *  Used to detect variable declarations that look like function prototypes.
 */
bool Doxy_Work::isVarWithConstructor(EntryNav *rootNav)
{
   static QRegExp initChars("[0-9\"'&*!^]+");
   static QRegExp idChars("[a-z_A-Z][a-z_A-Z0-9]*");
   bool result = false;
   bool typeIsClass;

   QByteArray type;
   Definition *ctx = 0;

   FileDef *fd = 0;
   int ti;

   rootNav->loadEntry(Doxy_Globals::g_storage);
   Entry *root = rootNav->entry();

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
         ctx = Doxygen::namespaceSDict->find(rootNav->parent()->name());
      }

      type = root->type;

      // remove qualifiers
      findAndRemoveWord(type, "const");
      findAndRemoveWord(type, "static");
      findAndRemoveWord(type, "volatile");

      //if (type.left(6)=="const ") type=type.right(type.length()-6);
      typeIsClass = getResolvedClass(ctx, fd, type) != 0;

      if (!typeIsClass && (ti = type.find('<')) != -1) {
         typeIsClass = getResolvedClass(ctx, fd, type.left(ti)) != 0;
      }

      if (typeIsClass) {
         // now we still have to check if the arguments are
         // types or values. Since we do not have complete type info
         // we need to rely on heuristics :-(

         ArgumentList *al = root->argList;
         if (al == 0 || al->isEmpty()) {
            result = false; // empty arg list -> function prototype.
            break;
         }

         for (auto a : *al) {
            if (!a->name.isEmpty() || !a->defval.isEmpty()) {
               if (a->name.indexOf(initChars) == 0) {
                  result = true;
               } else {
                  result = false; // arg has (type,name) pair -> function prototype
               }

               outerBreak = true;
               break;
            }

            if (a->type.isEmpty() || getResolvedClass(ctx, fd, a->type) != 0) {
               result = false; // arg type is a known type

               outerBreak = true;
               break;
            }

            if (checkIfTypedef(ctx, fd, a->type)) {
               //printf("%s:%d: false (arg is typedef)\n",__FILE__,__LINE__);
               result = false; // argument is a typedef

               outerBreak = true;
               break;
            }

            if (a->type.at(a->type.length() - 1) == '*' || a->type.at(a->type.length() - 1) == '&') {
               // type ends with * or & => pointer or reference
               result = false;

               outerBreak = true;
               break;
            }

            if (a->type.indexOf(initChars) == 0) {
               result = true; // argument type starts with typical initializer char

               outerBreak = true;
               break;
            }

            QByteArray resType = resolveTypeDef(ctx, a->type);
            if (resType.isEmpty()) {
               resType = a->type;
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

void Doxy_Work::addVariable(EntryNav *rootNav, int isFuncPtr = -1)
{
   rootNav->loadEntry(Doxy_Globals::g_storage);
   Entry *root = rootNav->entry();

   Debug::print(Debug::Variables, 0, "VARIABLE_SEC: \n"
                "  type=`%s' name=`%s' args=`%s' bodyLine=`%d' mGrpId=%d relates=%s\n",
                root->type.data(), root->name.data(), root->args.data(), root->bodyLine,
                root->mGrpId, root->relates.data() );

   if (root->type.isEmpty() && root->name.indexOf("operator") == -1 &&
         (root->name.indexOf('*') != -1 || root->name.indexOf('&') != -1)) {

      // recover from parse error caused by redundant braces
      // like in "int *(var[10]);", which is parsed as
      // type="" name="int *" args="(var[10])"

      root->type = root->name;
      static const QRegExp reName("[a-z_A-Z][a-z_A-Z0-9]*");
      int l = 0;
      int i = root->args.isEmpty() ? -1 : reName.match(root->args, 0, &l);

      if (i != -1) {
         root->name = root->args.mid(i, l);
         root->args = root->args.mid(i + l, root->args.find(')', i + l) - i - l);
      }

      //printf("new: type=`%s' name=`%s' args=`%s'\n",
      //    root->type.data(),root->name.data(),root->args.data());

   } else {
      int i = isFuncPtr;
      if (i == -1 && (root->spec & Entry::Alias) == 0) {
         i = findFunctionPtr(root->type, root->lang);   // for typedefs isFuncPtr is not yet set
      }

      Debug::print(Debug::Variables, 0, "  functionPtr? %s\n", i != -1 ? "yes" : "no");
      if (i != -1) {
         // function pointer
         int ai = root->type.indexOf('[', i);

         if (ai > i) { // function pointer array
            root->args.prepend(root->type.right(root->type.length() - ai));
            root->type = root->type.left(ai);

         } else if (root->type.indexOf(')', i) != -1) { // function ptr, not variable like "int (*bla)[10]"
            root->type = root->type.left(root->type.length() - 1);
            root->args.prepend(") ");
            //printf("root->type=%s root->args=%s\n",root->type.data(),root->args.data());
         }

      } else if (root->type.indexOf("typedef ") != -1 && root->type.right(2) == "()") { // typedef void (func)(int)
         root->type = root->type.left(root->type.length() - 1);
         root->args.prepend(") ");
      }
   }

   QByteArray scope, name = removeRedundantWhiteSpace(root->name);

   // find the scope of this variable
   EntryNav *p = rootNav->parent();

   while ((p->section() & Entry::SCOPE_MASK)) {
      QByteArray scopeName = p->name();
      if (!scopeName.isEmpty()) {
         scope.prepend(scopeName);
         break;
      }
      p = p->parent();
   }

   MemberType mtype;
   QByteArray type = root->type.trimmed();
   ClassDef *cd = 0;
   bool isRelated = false;
   bool isMemberOf = false;

   QByteArray classScope = stripAnonymousNamespaceScope(scope);
   classScope = stripTemplateSpecifiersFromScope(classScope, false);
   QByteArray annScopePrefix = scope.left(scope.length() - classScope.length());

   if (root->name.lastIndexOf("::") != -1) {
      if (root->type == "friend class" || root->type == "friend struct" ||
            root->type == "friend union") {
         cd = getClass(scope);
         if (cd) {
            addVariableToClass(rootNav,  // entry
                               cd,    // class to add member to
                               MemberType_Friend, // type of member
                               name, // name of the member
                               false,  // from Anonymous scope
                               0,      // anonymous member
                               Public, // protection
                               Member  // related to a class
                              );
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

   if (!root->relates.isEmpty()) { // related variable
      isRelated = true;
      isMemberOf = (root->relatesType == MemberOf);
      if (getClass(root->relates) == 0 && !scope.isEmpty()) {
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
      MemberDef *md = 0;

      // if cd is an anonymous (=tag less) scope we insert the member
      // into a non-anonymous parent scope as well. This is needed to
      // be able to refer to it using \var or \fn

      int si = scope.indexOf('@');

      static bool inlineSimpleStructs = Config_getBool("INLINE_SIMPLE_STRUCTS");
      if (si != -1 && !inlineSimpleStructs) { // anonymous scope or type

         QByteArray pScope;
         ClassDef *pcd = 0;
         pScope = scope.left(qMax(si - 2, 0)); // scope without tag less parts

         if (!pScope.isEmpty()) {
            pScope.prepend(annScopePrefix);
         } else if (annScopePrefix.length() > 2) {
            pScope = annScopePrefix.left(annScopePrefix.length() - 2);
         }

         if (name.at(0) != '@') {
            if (!pScope.isEmpty() && (pcd = getClass(pScope))) {
               md = addVariableToClass(rootNav, // entry
                                       pcd,   // class to add member to
                                       mtype, // member type
                                       name,  // member name
                                       true,  // from anonymous scope
                                       0,     // from anonymous member
                                       root->protection,
                                       isMemberOf ? Foreign : isRelated ? Related : Member
                                      );
               //added=true;
            } else { // anonymous scope inside namespace or file => put variable in the global scope
               if (mtype == MemberType_Variable) {
                  md = addVariableToFile(rootNav, mtype, pScope, name, true, 0);
               }
               //added=true;
            }
         }
      }

      //printf("name=`%s' scope=%s scope.right=%s\n",
      //                   name.data(),scope.data(),
      //                   scope.right(scope.length()-si).data());
      addVariableToClass(rootNav,   // entry
                         cd,     // class to add member to
                         mtype,  // member type
                         name,   // name of the member
                         false,  // from anonymous scope
                         md,     // from anonymous member
                         root->protection,
                         isMemberOf ? Foreign : isRelated ? Related : Member);
   } else if (!name.isEmpty()) { // global variable
      //printf("Inserting member in global scope %s!\n",scope.data());
      addVariableToFile(rootNav, mtype, scope, name, false,/*0,*/0);
   }

nextMember:
   rootNav->releaseEntry();
}

// Searches the Entry tree for typedef documentation sections.
// If found they are stored in their class or in the global list.
void Doxy_Work::buildTypedefList(EntryNav *rootNav)
{
   //printf("buildVarList(%s)\n",rootNav->name().data());

   if (!rootNav->name().isEmpty() &&
         rootNav->section() == Entry::VARIABLE_SEC &&
         rootNav->type().indexOf("typedef ") != -1 // its a typedef
      ) {
      addVariable(rootNav);
   }
   if (rootNav->children()) {

      for (auto e : *rootNav->children() ) {
         if (e->section() != Entry::ENUM_SEC) {
            buildTypedefList(e);
         }
      }
   }
}

// Searches the Entry tree for Variable documentation sections.
// If found they are stored in their class or in the global list.
void Doxy_Work::buildVarList(EntryNav *rootNav)
{
   //printf("buildVarList(%s) section=%08x\n",rootNav->name().data(),rootNav->section());
   int isFuncPtr = -1;

   if (!rootNav->name().isEmpty() &&
         (rootNav->type().isEmpty() || g_compoundKeywordDict.find(rootNav->type()) == 0) &&
         (
            (rootNav->section() == Entry::VARIABLE_SEC  // it's a variable
            ) ||
            (rootNav->section() == Entry::FUNCTION_SEC && // or maybe a function pointer variable
             (isFuncPtr = findFunctionPtr(rootNav->type(), rootNav->lang())) != -1
            ) ||
            (rootNav->section() == Entry::FUNCTION_SEC && // class variable initialized by constructor
             isVarWithConstructor(rootNav)
            )
         )
      ) { // documented variable
      addVariable(rootNav, isFuncPtr);
   }

   if (rootNav->children()) {
      for (auto e : *rootNav->children() ) {
         if (e->section() != Entry::ENUM_SEC) {
            buildVarList(e);
         }
      }
   }
}

// Searches the Entry tree for Interface sections (UNO IDL only).
// If found they are stored in their service or in the global list.
void Doxy_Work::addInterfaceOrServiceToServiceOrSingleton(EntryNav *const rootNav, ClassDef *const cd, QByteArray const &rname)
{
   Entry *const root = rootNav->entry();
   FileDef *const fd = rootNav->fileDef();

   enum MemberType const type = (rootNav->section() == Entry::EXPORTED_INTERFACE_SEC)
                                ? MemberType_Interface
                                : MemberType_Service;
   MemberDef *const md = new MemberDef(
      root->fileName, root->startLine, root->startColumn, root->type, rname,
      "", "", root->protection, root->virt, root->stat, Member,
      type, 0, root->argList);
   md->setTagInfo(rootNav->tagInfo());
   md->setMemberClass(cd);
   md->setDocumentation(root->doc, root->docFile, root->docLine);
   md->setDocsForDefinition(false);
   md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
   md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
   md->setBodySegment(root->bodyLine, root->endBodyLine);
   md->setMemberSpecifiers(root->spec);
   md->setMemberGroupId(root->mGrpId);
   md->setTypeConstraints(root->typeConstr);
   md->setLanguage(root->lang);
   md->setBodyDef(fd);
   md->setFileDef(fd);
   md->addSectionsToDefinition(root->anchors);
   QByteArray const def = root->type + " " + rname;
   md->setDefinition(def);
   md->enableCallGraph(root->callGraph);
   md->enableCallerGraph(root->callerGraph);

   Debug::print(Debug::Functions, 0,
                "  Interface Member:\n"
                "    `%s' `%s' proto=%d\n"
                "    def=`%s'\n",
                root->type.data(),
                rname.data(),
                root->proto,
                def.data()
               );

   // add member to the global list of all members
   MemberName *mn;

   if ((mn = Doxygen::memberNameSDict->find(rname))) {
      mn->append(md);

   } else {
      mn = new MemberName(rname);
      mn->append(md);
      Doxygen::memberNameSDict->append(rname, mn);
   }

   // add member to the class cd
   cd->insertMember(md);

   // also add the member as a "base" (to get nicer diagrams)
   // "optional" interface/service get Protected which turns into dashed line

   BaseInfo base(rname, (root->spec & (Entry::Optional)) ? Protected : Public, Normal);

   if (! findClassRelation(rootNav, cd, cd, &base, 0, DocumentedOnly, true)) {
      findClassRelation(rootNav, cd, cd, &base, 0, Undocumented, true);
   }

   // add file to list of used files
   cd->insertUsedFile(fd);

   addMemberToGroups(root, md);
   rootNav->changeSection(Entry::EMPTY_SEC);
   md->setRefItems(root->sli);
}

void Doxy_Work::buildInterfaceAndServiceList(EntryNav *const rootNav)
{
   if (rootNav->section() == Entry::EXPORTED_INTERFACE_SEC || rootNav->section() == Entry::INCLUDED_SERVICE_SEC) {

      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *const root = rootNav->entry();

      Debug::print(Debug::Functions, 0,"EXPORTED_INTERFACE_SEC:\n"
                   "  `%s' `%s'::`%s' `%s' relates=`%s' relatesType=`%d' file=`%s' "
                   "line=`%d' bodyLine=`%d' #tArgLists=%d mGrpId=%d spec=%lld proto=%d docFile=%s\n",

                   root->type.data(),
                   rootNav->parent()->name().data(),
                   root->name.data(),
                   root->args.data(),
                   root->relates.data(),
                   root->relatesType,
                   root->fileName.data(),
                   root->startLine,
                   root->bodyLine,
                   root->tArgLists ? (int)root->tArgLists->count() : -1,
                   root->mGrpId,
                   root->spec,
                   root->proto,
                   root->docFile.data() );

      QByteArray const rname = removeRedundantWhiteSpace(root->name);

      if (!rname.isEmpty()) {
         QByteArray const scope = rootNav->parent()->name();
         ClassDef *const cd = getClass(scope);
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
void Doxy_Work::addMethodToClass(EntryNav *rootNav, ClassDef *cd, const QByteArray &rname, bool isFriend)
{
   Entry *root = rootNav->entry();
   FileDef *fd = rootNav->fileDef();

   int l;
   static QRegExp re("([a-z_A-Z0-9: ]*[ &*]+[ ]*");
   int ts = root->type.find('<');
   int te = root->type.lastIndexOf('>');
   int i = re.match(root->type, 0, &l);

   if (i != -1 && ts != -1 && ts < te && ts < i && i < te) { // avoid changing A<int(int*)>, see bug 677315
      i = -1;
   }

   if (cd->getLanguage() == SrcLangExt_Cpp && // only C has pointers
         !root->type.isEmpty() && (root->spec & Entry::Alias) == 0 && i != -1) { // function variable

      root->args += root->type.right(root->type.length() - i - l);
      root->type = root->type.left(i + l);
   }

   QByteArray name = removeRedundantWhiteSpace(rname);
   if (name.left(2) == "::") {
      name = name.right(name.length() - 2);
   }

   MemberType mtype;
   if (isFriend) {
      mtype = MemberType_Friend;
   } else if (root->mtype == Signal) {
      mtype = MemberType_Signal;
   } else if (root->mtype == Slot) {
      mtype = MemberType_Slot;
   } else if (root->mtype == DCOP) {
      mtype = MemberType_DCOP;
   } else {
      mtype = MemberType_Function;
   }

   // strip redundant template specifier for constructors
   if ((fd == 0 || fd->getLanguage() == SrcLangExt_Cpp) && name.left(9) != "operator " &&
         (i = name.indexOf('<')) != -1 && name.indexOf('>') != -1) {

      name = name.left(i);
   }

   // adding class member
   MemberDef *md = new MemberDef(
      root->fileName, root->startLine, root->startColumn,
      root->type, name, root->args, root->exception,
      root->protection, root->virt,
      root->stat && root->relatesType != MemberOf,
      root->relates.isEmpty() ? Member :
      root->relatesType == MemberOf ? Foreign : Related,
      mtype, root->tArgLists ? root->tArgLists->getLast() : 0, root->argList);
   md->setTagInfo(rootNav->tagInfo());
   md->setMemberClass(cd);
   md->setDocumentation(root->doc, root->docFile, root->docLine);
   md->setDocsForDefinition(!root->proto);
   md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
   md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
   md->setBodySegment(root->bodyLine, root->endBodyLine);
   md->setMemberSpecifiers(root->spec);
   md->setMemberGroupId(root->mGrpId);
   md->setTypeConstraints(root->typeConstr);
   md->setLanguage(root->lang);
   md->setId(root->id);
   md->setBodyDef(fd);
   md->setFileDef(fd);
   //md->setScopeTemplateArguments(root->tArgList);
   md->addSectionsToDefinition(root->anchors);
   QByteArray def;
   QByteArray qualScope = cd->qualifiedNameWithTemplateParameters();
   SrcLangExt lang = cd->getLanguage();
   QByteArray scopeSeparator = getLanguageSpecificSeparator(lang);
   if (scopeSeparator != "::") {
      qualScope = substitute(qualScope, "::", scopeSeparator);
   }
   if (lang == SrcLangExt_PHP) {
      // for PHP we use Class::method and Namespace\method
      scopeSeparator = "::";
   }
   if (!root->relates.isEmpty() || isFriend || Config_getBool("HIDE_SCOPE_NAMES")) {
      if (!root->type.isEmpty()) {
         if (root->argList) {
            def = root->type + " " + name;
         } else {
            def = root->type + " " + name + root->args;
         }
      } else {
         if (root->argList) {
            def = name;
         } else {
            def = name + root->args;
         }
      }
   } else {
      if (!root->type.isEmpty()) {
         if (root->argList) {
            def = root->type + " " + qualScope + scopeSeparator + name;
         } else {
            def = root->type + " " + qualScope + scopeSeparator + name + root->args;
         }
      } else {
         if (root->argList) {
            def = qualScope + scopeSeparator + name;
         } else {
            def = qualScope + scopeSeparator + name + root->args;
         }
      }
   }
   if (def.left(7) == "friend ") {
      def = def.right(def.length() - 7);
   }
   md->setDefinition(def);
   md->enableCallGraph(root->callGraph);
   md->enableCallerGraph(root->callerGraph);

   Debug::print(Debug::Functions, 0,
                "  Func Member:\n"
                "    `%s' `%s'::`%s' `%s' proto=%d\n"
                "    def=`%s'\n",
                root->type.data(),
                qualScope.data(),
                rname.data(),
                root->args.data(),
                root->proto,
                def.data()
               );

   // add member to the global list of all members
   //printf("Adding member=%s class=%s\n",md->name().data(),cd->name().data());
   MemberName *mn;
   if ((mn = Doxygen::memberNameSDict->find(name))) {
      mn->append(md);
   } else {
      mn = new MemberName(name);
      mn->append(md);
      Doxygen::memberNameSDict->insert(name, mn);
   }

   // add member to the class cd
   cd->insertMember(md);
   // add file to list of used files
   cd->insertUsedFile(fd);

   addMemberToGroups(root, md);
   rootNav->changeSection(Entry::EMPTY_SEC);
   md->setRefItems(root->sli);
}

void Doxy_Work::buildFunctionList(EntryNav *rootNav)
{
   if (rootNav->section() == Entry::FUNCTION_SEC) {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      Debug::print(Debug::Functions, 0,
                   "FUNCTION_SEC:\n"
                   "  `%s' `%s'::`%s' `%s' relates=`%s' relatesType=`%d' file=`%s' line=`%d' bodyLine=`%d' #tArgLists=%d mGrpId=%d spec=%lld proto=%d docFile=%s\n",
                   root->type.data(),
                   rootNav->parent()->name().data(),
                   root->name.data(),
                   root->args.data(),
                   root->relates.data(),
                   root->relatesType,
                   root->fileName.data(),
                   root->startLine,
                   root->bodyLine,
                   root->tArgLists ? (int)root->tArgLists->count() : -1,
                   root->mGrpId,
                   root->spec,
                   root->proto,
                   root->docFile.data()
                  );

      bool isFriend = root->type.indexOf("friend ") != -1;
      QByteArray rname = removeRedundantWhiteSpace(root->name);

      QByteArray scope = rootNav->parent()->name(); //stripAnonymousNamespaceScope(root->parent->name);

      if (!rname.isEmpty() && scope.find('@') == -1) {
         ClassDef *cd = 0;
         // check if this function's parent is a class
         scope = stripTemplateSpecifiersFromScope(scope, false);

         FileDef *rfd = rootNav->fileDef();

         int memIndex = rname.lastIndexOf("::");

         cd = getClass(scope);
         if (cd && scope + "::" == rname.left(scope.length() + 2)) { // found A::f inside A
            // strip scope from name
            rname = rname.right(rname.length() - rootNav->parent()->name().length() - 2);
         }

         NamespaceDef *nd = 0;
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

         static QRegExp re("([a-z_A-Z0-9: ]*[ &*]+[ ]*");

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
   
               Debug::print(Debug::Functions, 0, "  --> member %s of class %s!\n", rname.data(), cd->name().data());
               addMethodToClass(rootNav, cd, rname, isFriend);

               done = true;
            }
         }

         if (done) { 
            // all done

         } else if (!((rootNav->parent()->section() & Entry::COMPOUND_MASK) || rootNav->parent()->section() == Entry::OBJCIMPL_SEC) &&
                    !isMember && (root->relates.isEmpty() || root->relatesType == Duplicate) &&
                    root->type.left(7) != "extern " && root->type.left(8) != "typedef ") {

            // no member => unrelated function

            /* check the uniqueness of the function name in the file.
             * A file could contain a function prototype and a function definition
             * or even multiple function prototypes.
             */
            bool found = false;
            MemberName *mn;
            MemberDef *md = 0;

            if ((mn = Doxygen::functionNameSDict->find(rname))) {
               Debug::print(Debug::Functions, 0, "  --> function %s already found!\n", rname.data());
               MemberNameIterator mni(*mn);

               for (mni.toFirst(); (!found && (md = mni.current())); ++mni) {
                  NamespaceDef *mnd = md->getNamespaceDef();
                  NamespaceDef *rnd = 0;
                  //printf("root namespace=%s\n",rootNav->parent()->name().data());

                  QByteArray fullScope = scope;
                  QByteArray parentScope = rootNav->parent()->name();

                  if (!parentScope.isEmpty() && !leftScopeMatch(parentScope, scope)) {
                     if (!scope.isEmpty()) {
                        fullScope.prepend("::");
                     }
                     fullScope.prepend(parentScope);
                  }

                  rnd = getResolvedNamespace(fullScope);
                  FileDef *mfd = md->getFileDef();
                  QByteArray nsName, rnsName;

                  if (mnd) {
                     nsName = mnd->name().copy();
                  }
                  if (rnd) {
                     rnsName = rnd->name().copy();
                  }

                  ArgumentList *mdAl = md->argumentList();
                  ArgumentList *mdTempl = md->templateArguments();

                  // in case of template functions, we need to check if the
                  // functions have the same number of template parameters
                  bool sameNumTemplateArgs = true;
                  bool matchingReturnTypes = true;

                  if (mdTempl != 0 && root->tArgLists) {
                     if (mdTempl->count() != root->tArgLists->getLast()->count()) {
                        sameNumTemplateArgs = false;
                     }
                     if (md->typeString() != removeRedundantWhiteSpace(root->type)) {
                        matchingReturnTypes = false;
                     }
                  }

                  bool staticsInDifferentFiles = root->stat && md->isStatic() && root->fileName != md->getDefFileName();

                  if (matchArguments2(md->getOuterScope(), mfd, mdAl,
                                     rnd ? rnd : Doxygen::globalScope, rfd, root->argList, false) &&
                     sameNumTemplateArgs && matchingReturnTypes && !staticsInDifferentFiles) {

                     GroupDef *gd = 0;

                     if (root->groups->getFirst() != 0) {
                        gd = Doxygen::groupSDict->find(root->groups->getFirst()->groupname.data());
                     }

                     // see if we need to create a new member
                     found = (mnd && rnd && nsName == rnsName) || // members are in the same namespace
                             ((mnd == 0 && rnd == 0 && mfd != 0 && // no external reference and
                               mfd->absoluteFilePath() == root->fileName));

                     // otherwise, allow a duplicate global member with the same argument list
                     if (!found && gd && gd == md->getGroupDef() && nsName == rnsName) {
                        // member is already in the group, so we don't want to add it again.
                        found = true;
                     }

                     if (found) {
                        // merge argument lists
                        mergeArguments(mdAl, root->argList, !root->doc.isEmpty());

                        // merge documentation
                        if (md->documentation().isEmpty() && !root->doc.isEmpty()) {
                           ArgumentList *argList = new ArgumentList;
                           stringToArgumentList(root->args, argList);

                           if (root->proto) {
                              //printf("setDeclArgumentList to %p\n",argList);
                              md->setDeclArgumentList(argList);
                           } else {
                              md->setArgumentList(argList);
                           }
                        }

                        md->setDocumentation(root->doc, root->docFile, root->docLine);
                        md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
                        md->setDocsForDefinition(!root->proto);

                        if (md->getStartBodyLine() == -1 && root->bodyLine != -1) {
                           md->setBodySegment(root->bodyLine, root->endBodyLine);
                           md->setBodyDef(rfd);
                        }

                        if (md->briefDescription().isEmpty() && !root->brief.isEmpty()) {
                           md->setArgsString(root->args);
                        }
                        md->setBriefDescription(root->brief, root->briefFile, root->briefLine);

                        md->addSectionsToDefinition(root->anchors);

                        md->enableCallGraph(md->hasCallGraph() || root->callGraph);
                        md->enableCallerGraph(md->hasCallerGraph() || root->callerGraph);

                        // merge ingroup specifiers
                        if (md->getGroupDef() == 0 && root->groups->getFirst() != 0) {
                           addMemberToGroups(root, md);
                        } else if (md->getGroupDef() != 0 && root->groups->count() == 0) {
                           //printf("existing member is grouped, new member not\n");
                           root->groups->append(new Grouping(md->getGroupDef()->name(), md->getGroupPri()));
                        } else if (md->getGroupDef() != 0 && root->groups->getFirst() != 0) {
                           //printf("both members are grouped\n");
                        }

                        // if md is a declaration and root is the corresponding
                        // definition, then turn md into a definition.
                        if (md->isPrototype() && !root->proto) {
                           md->setPrototype(false);
                        }
                     }
                  }
               }
            }
            if (!found) { /* global function is unique with respect to the file */
               Debug::print(Debug::Functions, 0, "  --> new function %s found!\n", rname.data());

               // new global function
               ArgumentList *tArgList = root->tArgLists ? root->tArgLists->getLast() : 0;
               QByteArray name = removeRedundantWhiteSpace(rname);

               md = new MemberDef(root->fileName, root->startLine, root->startColumn, root->type, name,
                  root->args, root->exception, root->protection, root->virt, root->stat, Member,
                  MemberType_Function, tArgList, root->argList);

               md->setTagInfo(rootNav->tagInfo());
               md->setLanguage(root->lang);
               md->setId(root->id);

               //md->setDefFile(root->fileName);
               //md->setDefLine(root->startLine);
               md->setDocumentation(root->doc, root->docFile, root->docLine);
               md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
               md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
               md->setPrototype(root->proto);
               md->setDocsForDefinition(!root->proto);
               md->setTypeConstraints(root->typeConstr);

               //md->setBody(root->body);
               md->setBodySegment(root->bodyLine, root->endBodyLine);

               FileDef *fd = rootNav->fileDef();
               md->setBodyDef(fd);
               md->addSectionsToDefinition(root->anchors);
               md->setMemberSpecifiers(root->spec);
               md->setMemberGroupId(root->mGrpId);

               // see if the function is inside a namespace that was not part of
               // the name already (in that case nd should be non-zero already)

               if (nd == 0 && rootNav->parent()->section() == Entry::NAMESPACE_SEC ) {
                  //QByteArray nscope=removeAnonymousScopes(rootNav->parent()->name());
                  QByteArray nscope = rootNav->parent()->name();
                  if (!nscope.isEmpty()) {
                     nd = getResolvedNamespace(nscope);
                  }
               }

               if (!scope.isEmpty()) {
                  QByteArray sep = getLanguageSpecificSeparator(root->lang);
                  if (sep != "::") {
                     scope = substitute(scope, "::", sep);
                  }
                  scope += sep;
               }

               QByteArray def;
               if (!root->type.isEmpty()) {
                  if (root->argList) {
                     def = root->type + " " + scope + name;
                  } else {
                     def = root->type + " " + scope + name + root->args;
                  }

               } else {
                  if (root->argList) {
                     def = scope + name.copy();
                  } else {
                     def = scope + name + root->args;
                  }
               }

               Debug::print(Debug::Functions, 0, "  Global Function:\n"
                            "    `%s' `%s'::`%s' `%s' proto=%d\n"
                            "    def=`%s'\n",
                            root->type.data(), rootNav->parent()->name().data(), rname.data(),
                            root->args.data(), root->proto, def.data() );

               md->setDefinition(def);
               md->enableCallGraph(root->callGraph);
               md->enableCallerGraph(root->callerGraph);

               md->setRefItems(root->sli);

               if (nd && !nd->name().isEmpty() && nd->name().at(0) != '@') {
                  // add member to namespace
                  md->setNamespace(nd);
                  nd->insertMember(md);
               }
               if (fd) {
                  // add member to the file (we do this even if we have already
                  // inserted it into the namespace)
                  md->setFileDef(fd);
                  fd->insertMember(md);
               }

               // add member to the list of file members
               //printf("Adding member=%s\n",md->name().data());
               MemberName *mn;
               if ((mn = Doxygen::functionNameSDict->find(name))) {
                  mn->append(md);
               } else {
                  mn = new MemberName(name);
                  mn->append(md);
                  Doxygen::functionNameSDict->insert(name, mn);
               }
               addMemberToGroups(root, md);
               if (root->relatesType == Simple) // if this is a relatesalso command,
                  // allow find Member to pick it up
               {
                  rootNav->changeSection(Entry::EMPTY_SEC); // Otherwise we have finished
                  // with this entry.

               }
            } else {
               FileDef *fd = rootNav->fileDef();
               if (fd) {
                  // add member to the file (we do this even if we have already
                  // inserted it into the namespace)
                  fd->insertMember(md);
               }
            }

            //printf("unrelated function %d `%s' `%s' `%s'\n",
            //    root->parent->section,root->type.data(),rname.data(),root->args.data());
         } else {
            Debug::print(Debug::Functions, 0, "  --> %s not processed!\n", rname.data());
         }
      } else if (rname.isEmpty()) {
         warn(root->fileName, root->startLine,
              "Illegal member name found."
             );
      }

      rootNav->releaseEntry();
   }

   RECURSE_ENTRYTREE(buildFunctionList, rootNav);
}

void Doxy_Work::findFriends()
{
   for (auto fn : *Doxygen::functionNameSDict) {
      // for each global function name

      MemberName *mn;

      if ((mn = Doxygen::memberNameSDict->find(fn->memberName()))) {
         // there are members with the same name

         for (auto fmd : *fn) {
            for (auto mmd : *mn) {

               ArgumentList *mmdAl = mmd->argumentList();
               ArgumentList *fmdAl = fmd->argumentList();

               if ((mmd->isFriend() || (mmd->isRelated() && mmd->isFunction())) &&
                     matchArguments2(mmd->getOuterScope(), mmd->getFileDef(), mmdAl,
                     fmd->getOuterScope(), fmd->getFileDef(), fmdAl,true)) {

                   // if the member is related and the arguments match then the
                   // function is actually a friend.

                  mergeArguments(mmdAl, fmdAl);

                  if (!fmd->documentation().isEmpty()) {
                     mmd->setDocumentation(fmd->documentation(), fmd->docFile(), fmd->docLine());
                  } else if (!mmd->documentation().isEmpty()) {
                     fmd->setDocumentation(mmd->documentation(), mmd->docFile(), mmd->docLine());
                  }
                  if (mmd->briefDescription().isEmpty() && !fmd->briefDescription().isEmpty()) {
                     mmd->setBriefDescription(fmd->briefDescription(), fmd->briefFile(), fmd->briefLine());
                  } else if (!mmd->briefDescription().isEmpty() && !fmd->briefDescription().isEmpty()) {
                     fmd->setBriefDescription(mmd->briefDescription(), mmd->briefFile(), mmd->briefLine());
                  }
                  if (!fmd->inbodyDocumentation().isEmpty()) {
                     mmd->setInbodyDocumentation(fmd->inbodyDocumentation(), fmd->inbodyFile(), fmd->inbodyLine());
                  } else if (!mmd->inbodyDocumentation().isEmpty()) {
                     fmd->setInbodyDocumentation(mmd->inbodyDocumentation(), mmd->inbodyFile(), mmd->inbodyLine());
                  }

                  //printf("body mmd %d fmd %d\n",mmd->getStartBodyLine(),fmd->getStartBodyLine());
                  if (mmd->getStartBodyLine() == -1 && fmd->getStartBodyLine() != -1) {
                     mmd->setBodySegment(fmd->getStartBodyLine(), fmd->getEndBodyLine());
                     mmd->setBodyDef(fmd->getBodyDef());
                     //mmd->setBodyMember(fmd);
                  } else if (mmd->getStartBodyLine() != -1 && fmd->getStartBodyLine() == -1) {
                     fmd->setBodySegment(mmd->getStartBodyLine(), mmd->getEndBodyLine());
                     fmd->setBodyDef(mmd->getBodyDef());
                     //fmd->setBodyMember(mmd);
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
   for (auto mn : *Doxygen::functionNameSDict) {

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
   for (auto mn : *Doxygen::functionNameSDict ) {
      MemberDef *mdef = 0;
      MemberDef *mdec = 0;

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

         if (
            matchArguments2(mdef->getOuterScope(), mdef->getFileDef(), mdefAl,
                            mdec->getOuterScope(), mdec->getFileDef(), mdecAl, true )

         ) { /* match found */
            MemberSDict *defDict = mdef->getReferencesMembers();
            MemberSDict *decDict = mdec->getReferencesMembers();

            if (defDict != 0) {
               MemberSDict::IteratorDict msdi(*defDict);
               MemberDef *rmd;

               for (msdi.toFirst(); (rmd = msdi.current()); ++msdi) {
                  if (decDict == 0 || decDict->find(rmd->name()) == 0) {
                     mdec->addSourceReferences(rmd);
                  }
               }
            }
            if (decDict != 0) {
               MemberSDict::IteratorDict msdi(*decDict);
               MemberDef *rmd;
               for (msdi.toFirst(); (rmd = msdi.current()); ++msdi) {
                  if (defDict == 0 || defDict->find(rmd->name()) == 0) {
                     mdef->addSourceReferences(rmd);
                  }
               }
            }

            defDict = mdef->getReferencedByMembers();
            decDict = mdec->getReferencedByMembers();
            if (defDict != 0) {
               MemberSDict::IteratorDict msdi(*defDict);
               MemberDef *rmd;
               for (msdi.toFirst(); (rmd = msdi.current()); ++msdi) {
                  if (decDict == 0 || decDict->find(rmd->name()) == 0) {
                     mdec->addSourceReferencedBy(rmd);
                  }
               }
            }
            if (decDict != 0) {
               MemberSDict::IteratorDict msdi(*decDict);
               MemberDef *rmd;
               for (msdi.toFirst(); (rmd = msdi.current()); ++msdi) {
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

   for (auto mn : *Doxygen::functionNameSDict) {

      /* find a matching function declaration and definition for this function */
      for (auto md : *mn) {

         MemberName *rmn;

         if ((rmn = Doxygen::memberNameSDict->find(md->name()))) {
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
QHash<QString, int> *Doxy_Work::getTemplateArgumentsInName(ArgumentList *templateArguments, const QByteArray &name)
{
   QHash<QString, int> *templateNames = new QHash<QString, int>();
   static QRegExp re("[a-z_A-Z][a-z_A-Z0-9:]*");

   if (templateArguments) {
      int count = 0;

      for (auto item : *templateArguments) {

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
ClassDef *Doxy_Work::findClassWithinClassContext(Definition *context, ClassDef *cd, const QByteArray &name)
{
   ClassDef *result = 0;
   if (cd == 0) {
      return result;
   }
   FileDef *fd = cd->getFileDef();
   if (context && cd != context) {
      result = getResolvedClass(context, 0, name, 0, 0, true, true);
   }
   if (result == 0) {
      result = getResolvedClass(cd, fd, name, 0, 0, true, true);
   }
   if (result == 0) { // try direct class, needed for namespaced classes imported via tag files (see bug624095)
      result = getClass(name);
   }
   if (result == 0 &&
         (cd->getLanguage() == SrcLangExt_CSharp || cd->getLanguage() == SrcLangExt_Java) &&
         name.find('<') != -1) {
      result = Doxygen::genericsDict->find(name);
   }

   return result;
}

void Doxy_Work::findUsedClassesForClass(EntryNav *rootNav, Definition *context, ClassDef *masterCd, ClassDef *instanceCd, bool isArtificial,
                                    ArgumentList *actualArgs = 0, QHash<QString, int> *templateNames = 0)
{
   masterCd->visited = true;
   ArgumentList *formalArgs = masterCd->templateArguments();

   if (masterCd->memberNameInfoSDict()) {

      for (auto mni : *masterCd->memberNameInfoSDict()) {

         for (auto mi : *mni) {
            MemberDef *md = mi->memberDef;

            if (md->isVariable() || md->isObjCProperty()) {
               // for each member variable in this class

               QByteArray type = normalizeNonTemplateArgumentsInString(md->typeString(), masterCd, formalArgs);
               QByteArray typedefValue = resolveTypeDef(masterCd, type);

               if (!typedefValue.isEmpty()) {
                  type = typedefValue;
               }

               int pos = 0;
               QByteArray usedClassName;
               QByteArray templSpec;
               bool found = false;

               // the type can contain template variables, replace them if present
               if (actualArgs) {
                  type = substituteTemplateArgumentsInString(type, formalArgs, actualArgs);
               }

               //printf("      template substitution gives=%s\n",type.data());
               while (!found && extractClassNameFromType(type, pos, usedClassName, templSpec, rootNav->lang()) != -1) {
                  // find the type (if any) that matches usedClassName
                  ClassDef *typeCd = getResolvedClass(masterCd,
                                                      masterCd->getFileDef(),
                                                      usedClassName,
                                                      0, 0,
                                                      false, true
                                                     );
                  //printf("====>  usedClassName=%s -> typeCd=%s\n",
                  //     usedClassName.data(),typeCd?typeCd->name().data():"<none>");
                  if (typeCd) {
                     usedClassName = typeCd->name();
                  }

                  int sp = usedClassName.find('<');
                  if (sp == -1) {
                     sp = 0;
                  }
                  int si = usedClassName.lastIndexOf("::", sp);
                  if (si != -1) {
                     // replace any namespace aliases
                     replaceNamespaceAliases(usedClassName, si);
                  }
                  // add any template arguments to the class
                  QByteArray usedName = removeRedundantWhiteSpace(usedClassName + templSpec);
                  //printf("    usedName=%s\n",usedName.data());

                  bool delTempNames = false;
                  if (templateNames == 0) {
                     templateNames = getTemplateArgumentsInName(formalArgs, usedName);
                     delTempNames = true;
                  }
                  BaseInfo bi(usedName, Public, Normal);
                  findClassRelation(rootNav, context, instanceCd, &bi, templateNames, TemplateInstances, isArtificial);

                  if (masterCd->templateArguments()) {
                     ArgumentListIterator ali(*masterCd->templateArguments());
                     Argument *arg;
                     int count = 0;
                     for (ali.toFirst(); (arg = ali.current()); ++ali, ++count) {
                        if (arg->name == usedName) { // type is a template argument
                           found = true;
                           Debug::print(Debug::Classes, 0, "    New used class `%s'\n", usedName.data());

                           ClassDef *usedCd = Doxygen::hiddenClasses->find(usedName);
                           if (usedCd == 0) {
                              usedCd = new ClassDef(
                                 masterCd->getDefFileName(), masterCd->getDefLine(),
                                 masterCd->getDefColumn(),
                                 usedName,
                                 ClassDef::Class);
                              //printf("making %s a template argument!!!\n",usedCd->name().data());
                              usedCd->makeTemplateArgument();
                              usedCd->setUsedOnly(true);
                              usedCd->setLanguage(masterCd->getLanguage());
                              Doxygen::hiddenClasses->append(usedName, usedCd);
                           }
                           if (isArtificial) {
                              usedCd->setArtificial(true);
                           }
                           Debug::print(Debug::Classes, 0, "      Adding used class `%s' (1)\n", usedCd->name().data());
                           instanceCd->addUsedClass(usedCd, md->name(), md->protection());
                           usedCd->addUsedByClass(instanceCd, md->name(), md->protection());
                        }
                     }
                  }

                  if (!found) {
                     ClassDef *usedCd = findClassWithinClassContext(context, masterCd, usedName);
                     //printf("Looking for used class %s: result=%s master=%s\n",
                     //    usedName.data(),usedCd?usedCd->name().data():"<none>",masterCd?masterCd->name().data():"<none>");

                     if (usedCd) {
                        found = true;
                        Debug::print(Debug::Classes, 0, "    Adding used class `%s' (2)\n", usedCd->name().data());
                        instanceCd->addUsedClass(usedCd, md->name(), md->protection()); // class exists
                        usedCd->addUsedByClass(instanceCd, md->name(), md->protection());
                     }
                  }
                  if (delTempNames) {
                     delete templateNames;
                     templateNames = 0;
                  }
               }
               if (!found && !type.isEmpty()) { // used class is not documented in any scope
                  ClassDef *usedCd = Doxygen::hiddenClasses->find(type);
                  if (usedCd == 0 && !Config_getBool("HIDE_UNDOC_RELATIONS")) {
                     if (type.right(2) == "(*" || type.right(2) == "(^") { // type is a function pointer
                        type += md->argsString();
                     }
                     Debug::print(Debug::Classes, 0, "  New undocumented used class `%s'\n", type.data());
                     usedCd = new ClassDef(
                        masterCd->getDefFileName(), masterCd->getDefLine(),
                        masterCd->getDefColumn(),
                        type, ClassDef::Class);
                     usedCd->setUsedOnly(true);
                     usedCd->setLanguage(masterCd->getLanguage());
                     Doxygen::hiddenClasses->append(type, usedCd);
                  }
                  if (usedCd) {
                     if (isArtificial) {
                        usedCd->setArtificial(true);
                     }
                     Debug::print(Debug::Classes, 0, "    Adding used class `%s' (3)\n", usedCd->name().data());
                     instanceCd->addUsedClass(usedCd, md->name(), md->protection());
                     usedCd->addUsedByClass(instanceCd, md->name(), md->protection());
                  }
               }
            }
         }
      }
   } else {
      //printf("no members for class %s (%p)\n",masterCd->name().data(),masterCd);
   }
}

void Doxy_Work::findBaseClassesForClass(EntryNav *rootNav, Definition *context, ClassDef *masterCd, ClassDef *instanceCd,
             FindBaseClassRelation_Mode mode, bool isArtificial, ArgumentList *actualArgs = 0, QHash<QString, int> *templateNames = 0)
{
   Entry *root = rootNav->entry();
   //if (masterCd->visited) return;

   masterCd->visited = true;
   // The base class could ofcouse also be a non-nested class

   ArgumentList *formalArgs = masterCd->templateArguments();
   QListIterator<BaseInfo> bii(*root->extends);
   BaseInfo *bi = 0;

   for (bii.toFirst(); (bi = bii.current()); ++bii) {
      //printf("masterCd=%s bi->name='%s' #actualArgs=%d\n",
      //    masterCd->localName().data(),bi->name.data(),actualArgs?(int)actualArgs->count():-1);
      bool delTempNames = false;
      if (templateNames == 0) {
         templateNames = getTemplateArgumentsInName(formalArgs, bi->name);
         delTempNames = true;
      }
      BaseInfo tbi(bi->name, bi->prot, bi->virt);
      if (actualArgs) { // substitute the formal template arguments of the base class
         tbi.name = substituteTemplateArgumentsInString(bi->name, formalArgs, actualArgs);
      }
      //printf("bi->name=%s tbi.name=%s\n",bi->name.data(),tbi.name.data());

      if (mode == DocumentedOnly) {
         // find a documented base class in the correct scope
         if (!findClassRelation(rootNav, context, instanceCd, &tbi, templateNames, DocumentedOnly, isArtificial)) {
            // 1.8.2: decided to show inheritance relations even if not documented,
            //        we do make them artificial, so they do not appear in the index
            //if (!Config_getBool("HIDE_UNDOC_RELATIONS"))
            bool b = Config_getBool("HIDE_UNDOC_RELATIONS") ? true : isArtificial;
            //{
            // no documented base class -> try to find an undocumented one
            findClassRelation(rootNav, context, instanceCd, &tbi, templateNames, Undocumented, b);
            //}
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

bool Doxy_Work::findTemplateInstanceRelation(Entry *root, Definition *context, ClassDef *templateClass, const QByteArray &templSpec,
                                         QHash<QString, int> *templateNames, bool isArtificial)
{
   Debug::print(Debug::Classes, 0, "    derived from template %s with parameters %s\n",
                templateClass->name().data(), templSpec.data());

   bool existingClass = (templSpec == tempArgListToString(templateClass->templateArguments(), root->lang) );

   if (existingClass) {
      return true;
   }

   bool freshInstance = false;
   ClassDef *instanceClass = templateClass->insertTemplateInstance(
                                root->fileName, root->startLine, root->startColumn, templSpec, freshInstance);
   if (isArtificial) {
      instanceClass->setArtificial(true);
   }
   instanceClass->setLanguage(root->lang);

   if (freshInstance) {
      Debug::print(Debug::Classes, 0, "      found fresh instance '%s'!\n", instanceClass->name().data());
      Doxygen::classSDict->append(instanceClass->name(), instanceClass);
      instanceClass->setTemplateBaseClassNames(templateNames);

      // search for new template instances caused by base classes of
      // instanceClass
      EntryNav *templateRootNav = g_classEntries.find(templateClass->name());

      if (templateRootNav) {
         bool unloadNeeded = false;
         Entry *templateRoot = templateRootNav->entry();

         if (templateRoot == 0) { // not yet loaded
            templateRootNav->loadEntry(Doxy_Globals::g_storage);
            templateRoot = templateRootNav->entry();

            assert(templateRoot != 0); // now it should really be loaded
            unloadNeeded = true;
         }

         Debug::print(Debug::Classes, 0, "        template root found %s templSpec=%s!\n",
                      templateRoot->name.data(), templSpec.data());
         ArgumentList *templArgs = new ArgumentList;
         stringToArgumentList(templSpec, templArgs);
         findBaseClassesForClass(templateRootNav, context, templateClass, instanceClass,
                                 TemplateInstances, isArtificial, templArgs, templateNames);

         findUsedClassesForClass(templateRootNav, context, templateClass, instanceClass,
                                 isArtificial, templArgs, templateNames);
         delete templArgs;

         if (unloadNeeded) { // still cleanup to do
            templateRootNav->releaseEntry();
         }
      } else {
         Debug::print(Debug::Classes, 0, "        no template root entry found!\n");
         // TODO: what happened if we get here?
      }

      //Debug::print(Debug::Classes,0,"    Template instance %s : \n",instanceClass->name().data());
      //ArgumentList *tl = templateClass->templateArguments();
   } else {
      Debug::print(Debug::Classes, 0, "      instance already exists!\n");
   }
   return true;
}

bool Doxy_Work::isRecursiveBaseClass(const QByteArray &scope, const QByteArray &name)
{
   QByteArray n = name;
   int index = n.find('<');
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
int Doxy_Work::findEndOfTemplate(const QByteArray &s, int startPos)
{
   // locate end of template
   int e = startPos;
   int brCount = 1;
   int roundCount = 0;
   int len = s.length();
   bool insideString = false;
   bool insideChar = false;
   char pc = 0;

   while (e < len && brCount != 0) {
      char c = s.at(e);
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

bool Doxy_Work::findClassRelation(EntryNav *rootNav, Definition *context, ClassDef *cd, BaseInfo *bi,
                              QHash<QString, int> *templateNames, FindBaseClassRelation_Mode mode, bool isArtificial)
{
   Entry *root = rootNav->entry();

   QByteArray biName = bi->name;
   bool explicitGlobalScope = false;
   //printf("findClassRelation: biName=`%s'\n",biName.data());
   if (biName.left(2) == "::") { // explicit global scope
      biName = biName.right(biName.length() - 2);
      explicitGlobalScope = true;
   }

   EntryNav *parentNode = rootNav->parent();
   bool lastParent = false;
   do // for each parent scope, starting with the largest scope
      // (in case of nested classes)
   {
      QByteArray scopeName = parentNode ? parentNode->name().data() : "";
      int scopeOffset = explicitGlobalScope ? 0 : scopeName.length();
      do { // try all parent scope prefixes, starting with the largest scope
         //printf("scopePrefix=`%s' biName=`%s'\n",
         //    scopeName.left(scopeOffset).data(),biName.data());

         QByteArray baseClassName = biName;
         if (scopeOffset > 0) {
            baseClassName.prepend(scopeName.left(scopeOffset) + "::");
         }

         MemberDef *baseClassTypeDef = 0;
         QByteArray templSpec;
         ClassDef *baseClass = getResolvedClass(explicitGlobalScope ? Doxygen::globalScope : context,
                                                cd->getFileDef(),
                                                baseClassName,
                                                &baseClassTypeDef,
                                                &templSpec,
                                                mode == Undocumented,
                                                true
                                               );

         if (!isRecursiveBaseClass(rootNav->name(), baseClassName)
               || explicitGlobalScope
               // sadly isRecursiveBaseClass always true for UNO IDL ifc/svc members
               // (i.e. this is needed for addInterfaceOrServiceToServiceOrSingleton)
               || (rootNav->lang() == SrcLangExt_IDL &&
                   (rootNav->section() == Entry::EXPORTED_INTERFACE_SEC ||
                    rootNav->section() == Entry::INCLUDED_SERVICE_SEC))) {

            Debug::print(
               Debug::Classes, 0, "    class relation %s inherited/used by %s found (%s and %s) templSpec='%s'\n",
               baseClassName.data(),
               rootNav->name().data(),
               (bi->prot == Private) ? "private" : ((bi->prot == Protected) ? "protected" : "public"),
               (bi->virt == Normal) ? "normal" : "virtual",
               templSpec.data()
            );

            int i = baseClassName.find('<');
            int si = baseClassName.lastIndexOf("::", i == -1 ? baseClassName.length() : i);

            if (si == -1) {
               si = 0;
            }

            if (baseClass == 0 && (root->lang == SrcLangExt_CSharp || root->lang == SrcLangExt_Java)) {

               // for Java/C# strip the template part before looking for matching
               baseClass = Doxygen::genericsDict->find(baseClassName.left(i));
               //printf("looking for '%s' result=%p\n",baseClassName.data(),baseClass);
            }
            if (baseClass == 0 && i != -1)
               // base class has template specifiers
            {
               // TODO: here we should try to find the correct template specialization
               // but for now, we only look for the unspecializated base class.
               int e = findEndOfTemplate(baseClassName, i + 1);

               //printf("baseClass==0 i=%d e=%d\n",i,e);
               if (e != -1) { // end of template was found at e
                  templSpec = removeRedundantWhiteSpace(baseClassName.mid(i, e - i));
                  baseClassName = baseClassName.left(i) + baseClassName.right(baseClassName.length() - e);
                  baseClass = getResolvedClass(explicitGlobalScope ? Doxygen::globalScope : context,
                                               cd->getFileDef(),
                                               baseClassName,
                                               &baseClassTypeDef,
                                               0, //&templSpec,
                                               mode == Undocumented,
                                               true
                                              );
                  //printf("baseClass=%p -> baseClass=%s templSpec=%s\n",
                  //      baseClass,baseClassName.data(),templSpec.data());
               }

            } else if (baseClass && !templSpec.isEmpty()) // we have a known class, but also
               // know it is a template, so see if
               // we can also link to the explicit
               // instance (for instance if a class
               // derived from a template argument)
            {
               //printf("baseClass=%p templSpec=%s\n",baseClass,templSpec.data());
               ClassDef *templClass = getClass(baseClass->name() + templSpec);
               if (templClass) {
                  // use the template instance instead of the template base.
                  baseClass = templClass;
                  templSpec.resize(0);
               }
            }

            //printf("cd=%p baseClass=%p\n",cd,baseClass);
            bool found = baseClass != 0 && (baseClass != cd || mode == TemplateInstances);
            //printf("1. found=%d\n",found);
            if (!found && si != -1) {
               QByteArray tmpTemplSpec;
               // replace any namespace aliases
               replaceNamespaceAliases(baseClassName, si);
               baseClass = getResolvedClass(explicitGlobalScope ? Doxygen::globalScope : context,
                                            cd->getFileDef(),
                                            baseClassName,
                                            &baseClassTypeDef,
                                            &tmpTemplSpec,
                                            mode == Undocumented,
                                            true
                                           );
               found = baseClass != 0 && baseClass != cd;
               if (found) {
                  templSpec = tmpTemplSpec;
               }
            }
            //printf("2. found=%d\n",found);

            if (!found) {
               baseClass = findClassWithinClassContext(context, cd, baseClassName);
               //printf("findClassWithinClassContext(%s,%s)=%p\n",
               //    cd->name().data(),baseClassName.data(),baseClass);
               found = baseClass != 0 && baseClass != cd;

            }

            if (!found) {
               // for PHP the "use A\B as C" construct map class C to A::B, so we lookup
               // the class name also in the alias mapping.
               QByteArray *aliasName = Doxygen::namespaceAliasDict[baseClassName];
               if (aliasName) { // see if it is indeed a class.
                  baseClass = getClass(*aliasName);
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

            if (found) { Debug::print(Debug::Classes, 0, "    Documented base class `%s' templSpec=%s\n", biName.data(),
                                      templSpec.isEmpty() ? "" : templSpec.data());

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
                  // see bug531637 for an example where this would otherwise hang doxygen

                  if (baseClassTypeDef == 0) {
                     //printf("       => findTemplateInstanceRelation: %p\n",baseClassTypeDef);
                     findTemplateInstanceRelation(root, context, baseClass, templSpec, templateNames, isArtificial);
                  }

               } else if (mode == DocumentedOnly || mode == Undocumented) {
                  //printf("       => insert base class\n");
                  QByteArray usedName;
                  if (baseClassTypeDef || cd->isCSharp()) {
                     usedName = biName;
                     //printf("***** usedName=%s templSpec=%s\n",usedName.data(),templSpec.data());
                  }
                  static bool sipSupport = Config_getBool("SIP_SUPPORT");
                  if (sipSupport) {
                     bi->prot = Public;
                  }
                  if (!cd->isSubClass(baseClass)) { // check for recursion, see bug690787
                     cd->insertBaseClass(baseClass, usedName, bi->prot, bi->virt, templSpec);
                     // add this class as super class to the base class
                     baseClass->insertSubClass(cd, bi->prot, bi->virt, templSpec);
                  } else {
                     warn(root->fileName, root->startLine,
                          "Detected potential recursive class relation "
                          "between class %s and base class %s!",
                          cd->name().data(), baseClass->name().data()
                         );
                  }
               }
               return true;

            } else if (mode == Undocumented && (scopeOffset == 0 || isATemplateArgument)) {
               Debug::print(Debug::Classes, 0,
                            "    New undocumented base class `%s' baseClassName=%s templSpec=%s isArtificial=%d\n",
                            biName.data(), baseClassName.data(), templSpec.data(), isArtificial
                           );
               baseClass = 0;
               if (isATemplateArgument) {
                  baseClass = Doxygen::hiddenClasses->find(baseClassName);
                  if (baseClass == 0) {
                     baseClass = new ClassDef(root->fileName, root->startLine, root->startColumn,
                                              baseClassName,
                                              ClassDef::Class);
                     Doxygen::hiddenClasses->append(baseClassName, baseClass);
                     if (isArtificial) {
                        baseClass->setArtificial(true);
                     }
                     baseClass->setLanguage(root->lang);
                  }
               } else {
                  baseClass = Doxygen::classSDict->find(baseClassName);
                  //printf("*** classDDict->find(%s)=%p biName=%s templSpec=%s\n",
                  //    baseClassName.data(),baseClass,biName.data(),templSpec.data());
                  if (baseClass == 0) {
                     baseClass = new ClassDef(root->fileName, root->startLine, root->startColumn,
                                              baseClassName,
                                              ClassDef::Class);
                     Doxygen::classSDict->append(baseClassName, baseClass);
                     if (isArtificial) {
                        baseClass->setArtificial(true);
                     }
                     baseClass->setLanguage(root->lang);
                     int si = baseClassName.lastIndexOf("::");
                     if (si != -1) { // class is nested
                        Definition *sd = findScopeFromQualifiedName(Doxygen::globalScope, baseClassName.left(si), 0, rootNav->tagInfo());
                        if (sd == 0 || sd == Doxygen::globalScope) { // outer scope not found
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
               baseClass->setOuterScope(Doxygen::globalScope);
               if (baseClassName.right(2) == "-p") {
                  baseClass->setCompoundType(ClassDef::Protocol);
               }
               return true;
            } else {
               Debug::print(Debug::Classes, 0, "    Base class `%s' not found\n", biName.data());
            }

         } else {
            if (mode != TemplateInstances) {
               warn(root->fileName, root->startLine,
                    "Detected potential recursive class relation "
                    "between class %s and base class %s!\n",
                    root->name.data(), baseClassName.data()
                   );
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
         //printf("new scopeOffset=`%d'",scopeOffset);
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
bool Doxy_Work::isClassSection(EntryNav *rootNav)
{
   if ( !rootNav->name().isEmpty() ) {
      if (rootNav->section() & Entry::COMPOUND_MASK)
         // is it a compound (class, struct, union, interface ...)
      {
         return true;
      } else if (rootNav->section() & Entry::COMPOUNDDOC_MASK)
         // is it a documentation block with inheritance info.
      {
         rootNav->loadEntry(Doxy_Globals::g_storage);
         Entry *root = rootNav->entry();
         bool extends = root->extends->count() > 0;
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
void Doxy_Work::findClassEntries(EntryNav *rootNav)
{
   if (isClassSection(rootNav)) {
      g_classEntries.insert(rootNav->name(), rootNav);
   }
   RECURSE_ENTRYTREE(findClassEntries, rootNav);
}

QByteArray Doxy_Work::extractClassName(EntryNav *rootNav)
{
   // strip any anonymous scopes first
   QByteArray bName = stripAnonymousNamespaceScope(rootNav->name());
   bName = stripTemplateSpecifiersFromScope(bName);
   int i;
   if ((rootNav->lang() == SrcLangExt_CSharp || rootNav->lang() == SrcLangExt_Java) &&
         (i = bName.find('<')) != -1) {
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
   ClassSDict::Iterator cli(*Doxygen::classSDict);
   for (cli.toFirst(); cli.current(); ++cli) {
      cli.current()->visited = false;
   }

   QHasIterator<QString, EntryNav> edi(g_classEntries);
   EntryNav *rootNav;

   for (; (rootNav = edi.current()); ++edi) {
      ClassDef *cd;
      QByteArray bName = extractClassName(rootNav);
      Debug::print(Debug::Classes, 0, "  Inheritance: Class %s : \n", bName.data());
      if ((cd = getClass(bName))) {
         rootNav->loadEntry(Doxy_Globals::g_storage);
         //printf("Class %s %d\n",cd->name().data(),root->extends->count());
         findBaseClassesForClass(rootNav, cd, cd, cd, TemplateInstances, false);
         rootNav->releaseEntry();
      }
   }
}

void Doxy_Work::findUsedTemplateInstances()
{
   ClassSDict::Iterator cli(*Doxygen::classSDict);
   for (cli.toFirst(); cli.current(); ++cli) {
      cli.current()->visited = false;
   }

   QHasIterator<QString, EntryNav> edi(g_classEntries);

   EntryNav *rootNav;
   for (; (rootNav = edi.current()); ++edi) {
      ClassDef *cd;
      QByteArray bName = extractClassName(rootNav);
      Debug::print(Debug::Classes, 0, "  Usage: Class %s : \n", bName.data());
      if ((cd = getClass(bName))) {
         rootNav->loadEntry(Doxy_Globals::g_storage);
         findUsedClassesForClass(rootNav, cd, cd, cd, true);
         rootNav->releaseEntry();
      }
   }
}

void Doxy_Work::computeClassRelations()
{
   ClassSDict::Iterator cli(*Doxygen::classSDict);
   for (cli.toFirst(); cli.current(); ++cli) {
      cli.current()->visited = false;
   }

   QHasIterator<QString, EntryNav> edi(g_classEntries);
   EntryNav *rootNav;

   for (; (rootNav = edi.current()); ++edi) {
      ClassDef *cd;

      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();
      QByteArray bName = extractClassName(rootNav);
      Debug::print(Debug::Classes, 0, "  Relations: Class %s : \n", bName.data());
      if ((cd = getClass(bName))) {
         findBaseClassesForClass(rootNav, cd, cd, cd, DocumentedOnly, false);
      }
      int numMembers = cd && cd->memberNameInfoSDict() ? cd->memberNameInfoSDict()->count() : 0;
      if ((cd == 0 || (!cd->hasDocumentation() && !cd->isReference())) && numMembers > 0 &&
            bName.right(2) != "::") {
         if (!root->name.isEmpty() && root->name.find('@') == -1 && // normal name
               (guessSection(root->fileName) == Entry::HEADER_SEC ||
                Config_getBool("EXTRACT_LOCAL_CLASSES")) && // not defined in source file
               protectionLevelVisible(root->protection) && // hidden by protection
               !Config_getBool("HIDE_UNDOC_CLASSES") // undocumented class are visible
            )
            warn_undoc(
               root->fileName, root->startLine,
               "Compound %s is not documented.",
               root->name.data()
            );
      }

      rootNav->releaseEntry();
   }
}

void Doxy_Work::computeTemplateClassRelations()
{
   QHasIterator<QString, EntryNav> edi(g_classEntries);
   EntryNav *rootNav;

   for (; (rootNav = edi.current()); ++edi) {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      QByteArray bName = stripAnonymousNamespaceScope(root->name);
      bName = stripTemplateSpecifiersFromScope(bName);
      ClassDef *cd = getClass(bName);

      // strip any anonymous scopes first
      QHash<QString, ClassDef> *templInstances = 0;

      if (cd && (templInstances = cd->getTemplateInstances())) {
         Debug::print(Debug::Classes, 0, "  Template class %s : \n", cd->name().data());
          QHasIterator<QString, ClassDef> tdi(*templInstances);

         ClassDef *tcd;

         for (tdi.toFirst(); (tcd = tdi.current()); ++tdi) { // for each template instance
            Debug::print(Debug::Classes, 0, "    Template instance %s : \n", tcd->name().data());
            QByteArray templSpec = tdi.currentKey();

            ArgumentList *templArgs = new ArgumentList;
            stringToArgumentList(templSpec, templArgs);

            QList<BaseInfo> *baseList = root->extends;
            QListIterator<BaseInfo> it(*baseList);

            BaseInfo *bi;

            for (; (bi = it.current()); ++it) { // for each base class of the template
               // check if the base class is a template argument
               BaseInfo tbi(bi->name, bi->prot, bi->virt);
               ArgumentList *tl = cd->templateArguments();

               if (tl) {
                  QHash<QString, int> *baseClassNames = tcd->getTemplateBaseClassNames();
                  QHash<QString, int> *templateNames = getTemplateArgumentsInName(tl, bi->name);
                  // for each template name that we inherit from we need to
                  // substitute the formal with the actual arguments

                  QHash<QString, int> *actualTemplateNames = new QHash<QString, int>();
                  QHashIterator<QString, int> qdi(*templateNames);

                  for (qdi.toFirst(); qdi.current(); ++qdi) {
                     int templIndex = *qdi.current();
                     Argument *actArg = 0;

                     if (templIndex < (int)templArgs->count()) {
                        actArg = templArgs->at(templIndex);
                     }

                     if (actArg != 0 && baseClassNames != 0 && baseClassNames->contains(actArg->type)
                         && ! actualTemplateNames->contains(actArg->type)) {
                        actualTemplateNames->insert(actArg->type, new int(templIndex));
                     }
                  }

                  delete templateNames;

                  tbi.name = substituteTemplateArgumentsInString(bi->name, tl, templArgs);

                  // find a documented base class in the correct scope
                  if (!findClassRelation(rootNav, cd, tcd, &tbi, actualTemplateNames, DocumentedOnly, false)) {
                     // no documented base class -> try to find an undocumented one
                     findClassRelation(rootNav, cd, tcd, &tbi, actualTemplateNames, Undocumented, true);
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
   ClassSDict::Iterator cli(*Doxygen::classSDict);
   ClassDef *cd = 0;
   for (cli.toFirst(); (cd = cli.current()); ++cli) {
      cd->computeAnchors();
   }
   FileNameListIterator fnli(*Doxygen::inputNameList);
   FileName *fn;
   for (fnli.toFirst(); (fn = fnli.current()); ++fnli) {
      FileNameIterator fni(*fn);
      FileDef *fd;
      for (; (fd = fni.current()); ++fni) {
         fd->computeAnchors();
      }
   }
   NamespaceSDict::Iterator nli(*Doxygen::namespaceSDict);
   NamespaceDef *nd = 0;
   for (nli.toFirst(); (nd = nli.current()); ++nli) {
      nd->computeAnchors();
   }
   GroupSDict::Iterator gli(*Doxygen::groupSDict);
   GroupDef *gd;
   for (gli.toFirst(); (gd = gli.current()); ++gli) {
      gd->computeAnchors();
   }
}

void Doxy_Work::addListReferences()
{
   MemberNameSDict::Iterator mnli(*Doxygen::memberNameSDict);
   MemberName *mn = 0;
   for (mnli.toFirst(); (mn = mnli.current()); ++mnli) {
      MemberNameIterator mni(*mn);
      MemberDef *md = 0;
      for (mni.toFirst(); (md = mni.current()); ++mni) {
         md->visited = false;
      }
   }
   MemberNameSDict::Iterator fmnli(*Doxygen::functionNameSDict);
   for (fmnli.toFirst(); (mn = fmnli.current()); ++fmnli) {
      MemberNameIterator mni(*mn);
      MemberDef *md = 0;
      for (mni.toFirst(); (md = mni.current()); ++mni) {
         md->visited = false;
      }
   }

   ClassSDict::Iterator cli(*Doxygen::classSDict);
   ClassDef *cd = 0;
   for (cli.toFirst(); (cd = cli.current()); ++cli) {
      cd->addListReferences();
   }

   FileNameListIterator fnli(*Doxygen::inputNameList);
   FileName *fn;
   for (fnli.toFirst(); (fn = fnli.current()); ++fnli) {
      FileNameIterator fni(*fn);
      FileDef *fd;
      for (; (fd = fni.current()); ++fni) {
         fd->addListReferences();
      }
   }

   NamespaceSDict::Iterator nli(*Doxygen::namespaceSDict);
   NamespaceDef *nd = 0;
   for (nli.toFirst(); (nd = nli.current()); ++nli) {
      nd->addListReferences();
   }

   GroupSDict::Iterator gli(*Doxygen::groupSDict);
   GroupDef *gd;
   for (gli.toFirst(); (gd = gli.current()); ++gli) {
      gd->addListReferences();
   }

   PageSDict::Iterator pdi(*Doxygen::pageSDict);
   PageDef *pd = 0;
   for (pdi.toFirst(); (pd = pdi.current()); ++pdi) {
      QByteArray name = pd->getOutputFileBase();
      if (pd->getGroupDef()) {
         name = pd->getGroupDef()->getOutputFileBase();
      }
      {
         QList<ListItemInfo> *xrefItems = pd->xrefListItems();
         addRefItem(xrefItems, name, theTranslator->trPage(true, true), name, pd->title(), 0, 0);
      }
   }

   DirSDict::Iterator ddi(*Doxygen::directories);
   DirDef *dd = 0;

   for (ddi.toFirst(); (dd = ddi.current()); ++ddi) {
      QByteArray name = dd->getOutputFileBase();
      QList<ListItemInfo> *xrefItems = dd->xrefListItems();

      addRefItem(xrefItems, name, theTranslator->trDir(true, true), name, dd->displayName(), 0, 0);
   }
}

void Doxy_Work::generateXRefPages()
{
   QHashIterator<QString, RefList> di(*Doxygen::xrefLists);
   RefList *rl;

   for (di.toFirst(); (rl = di.current()); ++di) {
      rl->generatePage();
   }
}

// Copy the documentation in entry `root' to member definition `md' and
// set the function declaration of the member to `funcDecl'. If the boolean
// over_load is set the standard overload text is added.

void Doxy_Work::addMemberDocs(EntryNav *rootNav, MemberDef *md, const char *funcDecl, ArgumentList *al,
                          bool over_load, NamespaceSDict *)
{
   Entry *root = rootNav->entry();

   QByteArray fDecl = funcDecl;

   // strip extern specifier
   fDecl = stripPrefix(fDecl, "extern ");

   md->setDefinition(fDecl);
   md->enableCallGraph(root->callGraph);
   md->enableCallerGraph(root->callerGraph);

   ClassDef     *cd = md->getClassDef();
   NamespaceDef *nd = md->getNamespaceDef();

   QByteArray fullName;

   if (cd) {
      fullName = cd->name();
   } else if (nd) {
      fullName = nd->name();
   }

   if (!fullName.isEmpty()) {
      fullName += "::";
   }
   fullName += md->name();
   FileDef *rfd = rootNav->fileDef();

   // TODO determine scope based on root not md
   Definition *rscope = md->getOuterScope();

   ArgumentList *mdAl = md->argumentList();
   if (al) {
      //printf("merging arguments (1) docs=%d\n",root->doc.isEmpty());
      mergeArguments(mdAl, al, !root->doc.isEmpty());
   } else {
      if (
         matchArguments2( md->getOuterScope(), md->getFileDef(), mdAl,
                          rscope, rfd, root->argList,
                          true
                        )
      ) {
         //printf("merging arguments (2)\n");
         mergeArguments(mdAl, root->argList, !root->doc.isEmpty());
      }
   }
   if (over_load) { // the \overload keyword was used
      QByteArray doc = getOverloadDocs();
      if (!root->doc.isEmpty()) {
         doc += "<p>";
         doc += root->doc;
      }
      md->setDocumentation(doc, root->docFile, root->docLine);
      md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
      md->setDocsForDefinition(!root->proto);
   } else {
      //printf("overwrite!\n");
      md->setDocumentation(root->doc, root->docFile, root->docLine);
      md->setDocsForDefinition(!root->proto);

      //printf("overwrite!\n");
      md->setBriefDescription(root->brief, root->briefFile, root->briefLine);

      if (
         (md->inbodyDocumentation().isEmpty() ||
          !rootNav->parent()->name().isEmpty()
         ) && !root->inbodyDocs.isEmpty()
      ) {
         md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
      }
   }

   //printf("initializer: '%s'(isEmpty=%d) '%s'(isEmpty=%d)\n",
   //    md->initializer().data(),md->initializer().isEmpty(),
   //    root->initializer.data(),root->initializer.isEmpty()
   //   );
   if (md->initializer().isEmpty() && !root->initializer.isEmpty()) {
      //printf("setInitializer\n");
      md->setInitializer(root->initializer);
   }

   md->setMaxInitLines(root->initLines);

   if (rfd) {
      if ((md->getStartBodyLine() == -1 && root->bodyLine != -1)
         ) {
         //printf("Setting new body segment [%d,%d]\n",root->bodyLine,root->endBodyLine);
         md->setBodySegment(root->bodyLine, root->endBodyLine);
         md->setBodyDef(rfd);
      }

      md->setRefItems(root->sli);
   }

   md->enableCallGraph(md->hasCallGraph() || root->callGraph);
   md->enableCallerGraph(md->hasCallerGraph() || root->callerGraph);

   md->mergeMemberSpecifiers(root->spec);
   md->addSectionsToDefinition(root->anchors);
   addMemberToGroups(root, md);
   if (cd) {
      cd->insertUsedFile(rfd);
   }
   //printf("root->mGrpId=%d\n",root->mGrpId);
   if (root->mGrpId != -1) {
      if (md->getMemberGroupId() != -1) {
         if (md->getMemberGroupId() != root->mGrpId) {
            warn(
               root->fileName, root->startLine,
               "member %s belongs to two different groups. The second "
               "one found here will be ignored.",
               md->name().data()
            );
         }
      } else { // set group id
         //printf("setMemberGroupId=%d md=%s\n",root->mGrpId,md->name().data());
         md->setMemberGroupId(root->mGrpId);
      }
   }
}


// find a class definition given the scope name and (optionally) a
// template list specifier
ClassDef *Doxy_Work::findClassDefinition(FileDef *fd, NamespaceDef *nd, const char *scopeName)
{
   ClassDef *tcd = getResolvedClass(nd, fd, scopeName, 0, 0, true, true);
   return tcd;
}

// Adds the documentation contained in `root' to a global function
// with name `name' and argument list `args' (for overloading) and
// function declaration `decl' to the corresponding member definition.
bool Doxy_Work::findGlobalMember(EntryNav *rootNav, const QByteArray &namespaceName, const char *type, const char *name,
                             const char *tempArg, const char *, const char *decl)
{
   Entry *root = rootNav->entry();
   Debug::print(Debug::FindMembers, 0,
                "2. findGlobalMember(namespace=%s,type=%s,name=%s,tempArg=%s,decl=%s)\n",
                namespaceName.data(), type, name, tempArg, decl);
   QByteArray n = name;

   if (n.isEmpty()) {
      return false;
   }

   if (n.find("::") != -1) {
      return false;   // skip undefined class members
   }

   MemberName *mn = Doxygen::functionNameSDict->find(n + tempArg); // look in function dictionary

   if (mn == 0) {
      mn = Doxygen::functionNameSDict->find(n); // try without template arguments
   }

   if (mn) {
      // function name defined

      Debug::print(Debug::FindMembers, 0, "3. Found symbol scope\n");

      MemberNameIterator mni(*mn);
      MemberDef *md;
      bool found = false;

      for (mni.toFirst(); (md = mni.current()) && !found; ++mni) {
         NamespaceDef *nd = md->getNamespaceDef();


         FileDef *fd = rootNav->fileDef();
         NamespaceSDict *nl = fd ? fd->getUsedNamespaces() : 0;


         // search in the list of namespaces that are imported via a
         // using declaration
         bool viaUsingDirective = nl && nd && nl->find(nd->qualifiedName()) != 0;

         if ((namespaceName.isEmpty() && nd == 0) ||   // not in a namespace
               (nd && nd->name() == namespaceName) ||    // or in the same namespace
               viaUsingDirective                         // member in `using' namespace
            ) {

            Debug::print(Debug::FindMembers, 0, "4. Try to add member `%s' to scope `%s'\n",
                         md->name().data(), namespaceName.data());

            QByteArray nsName = nd ? nd->name().data() : "";

            NamespaceDef *rnd = 0;

            if (! namespaceName.isEmpty()) {
               rnd = Doxygen::namespaceSDict->find(namespaceName);
            }

            ArgumentList *mdAl = md->argumentList();

            bool matching =
               (mdAl == 0 && root->argList->count() == 0) ||
               md->isVariable() || md->isTypedef() || /* in case of function pointers */
               matchArguments2(md->getOuterScope(), md->getFileDef(), mdAl,
                               rnd ? rnd : Doxygen::globalScope, fd, root->argList, false);

            // for template members we need to check if the number of
            // template arguments is the same, otherwise we are dealing with
            // different functions.
            if (matching && root->tArgLists) {
               ArgumentList *mdTempl = md->templateArguments();
               if (mdTempl) {
                  if (root->tArgLists->getLast()->count() != mdTempl->count()) {
                     matching = false;
                  }
               }
            }

            // for static members we also check if the comment block was found in
            // the same file. This is needed because static members with the same
            // name can be in different files. Thus it would be wrong to just
            // put the comment block at the first syntactically matching member.
            if (matching && md->isStatic() &&
                  md->getDefFileName() != root->fileName &&
                  mn->count() > 1) {
               matching = false;
            }

            // for template member we also need to check the return type
            if (md->templateArguments() != 0 && root->tArgLists != 0) {
               //printf("Comparing return types '%s'<->'%s'\n",
               //    md->typeString(),type);

               if (md->templateArguments()->count() != root->tArgLists->getLast()->count() ||
                     qstrcmp(md->typeString(), type) != 0) {
                  //printf(" ---> no matching\n");
                  matching = false;
               }
            }

            if (matching) { // add docs to the member
               Debug::print(Debug::FindMembers, 0, "5. Match found\n");
               addMemberDocs(rootNav, md, decl, root->argList, false);
               found = true;
            }
         }
      }
      if (!found && root->relatesType != Duplicate && root->section == Entry::FUNCTION_SEC) { // no match
         QByteArray fullFuncDecl = decl;
         if (root->argList) {
            fullFuncDecl += argListToString(root->argList, true);
         }
         QByteArray warnMsg =
            QByteArray("no matching file member found for \n") + substitute(fullFuncDecl, "%", "%%");
         if (mn->count() > 0) {
            warnMsg += "\nPossible candidates:\n";
            for (mni.toFirst(); (md = mni.current()); ++mni) {
               warnMsg += " '";
               warnMsg += substitute(md->declaration(), "%", "%%");
               warnMsg += "' at line " + QByteArray().setNum(md->getDefLine()) +
                          " of file" + md->getDefFileName() + "\n";
            }
         }
         warn(root->fileName, root->startLine, warnMsg);
      }
   } else { // got docs for an undefined member!
      if (root->type != "friend class" &&
            root->type != "friend struct" &&
            root->type != "friend union" &&
            (!Config_getBool("TYPEDEF_HIDES_STRUCT") ||
             root->type.find("typedef ") == -1)
         ) {
         warn(root->fileName, root->startLine,
              "documented symbol `%s' was not declared or defined.", decl
             );
      }
   }
   return true;
}

bool Doxy_Work::isSpecialization(const QList<ArgumentList> &srcTempArgLists, const QList<ArgumentList> &dstTempArgLists)
{
   QListIterator<ArgumentList> srclali(srcTempArgLists);
   QListIterator<ArgumentList> dstlali(dstTempArgLists);

   for (; srclali.current(); ++srclali, ++dstlali) {
      ArgumentList *sal = srclali.current();
      ArgumentList *dal = dstlali.current();
      if (!(sal && dal && sal->count() == dal->count())) {
         return true;
      }
   }

   return false;
}

bool Doxy_Work::scopeIsTemplate(Definition *d)
{
   bool result = false;
   if (d && d->definitionType() == Definition::TypeClass) {
      result = ((ClassDef *)d)->templateArguments() || scopeIsTemplate(d->getOuterScope());
   }
   return result;
}

QByteArray Doxy_Work::substituteTemplatesInString( const QList<ArgumentList> &srcTempArgLists, const QList<ArgumentList> &dstTempArgLists,
   ArgumentList *funcTempArgList, const QByteArray &src)
{
   // can be used to match template specializations

   QByteArray dst;
   QRegExp re( "[A-Za-z_][A-Za-z_0-9]*");
   //printf("type=%s\n",sa->type.data());

   int i, p = 0, l;

   while ((i = re.match(src, p, &l)) != -1) { // for each word in srcType
      bool found = false;
      dst += src.mid(p, i - p);
      QByteArray name = src.mid(i, l);

      QListIterator<ArgumentList> srclali(srcTempArgLists);
      QListIterator<ArgumentList> dstlali(dstTempArgLists);
      for (; srclali.current() && !found; ++srclali, ++dstlali) {
         ArgumentListIterator tsali(*srclali.current());
         ArgumentListIterator tdali(*dstlali.current());
         ArgumentListIterator *fali = 0;
         Argument *tsa = 0, *tda = 0, *fa = 0;
         if (funcTempArgList) {
            fali = new ArgumentListIterator(*funcTempArgList);
            fa = fali->current();
         }

         for (tsali.toFirst(); (tsa = tsali.current()) && !found; ++tsali) {
            tda = tdali.current();
            //if (tda) printf("tsa=%s|%s tda=%s|%s\n",
            //    tsa->type.data(),tsa->name.data(),
            //    tda->type.data(),tda->name.data());
            if (name == tsa->name) {
               if (tda && tda->name.isEmpty()) {
                  int vc = 0;
                  if (tda->type.left(6) == "class ") {
                     vc = 6;
                  } else if (tda->type.left(9) == "typename ") {
                     vc = 9;
                  }
                  if (vc > 0) { // convert type=="class T" to type=="class" name=="T"
                     tda->name = tda->type.mid(vc);
                     tda->type = tda->type.left(vc - 1);
                  }
               }
               if (tda && !tda->name.isEmpty()) {
                  name = tda->name; // substitute
                  found = true;
               } else if (fa) {
                  name = fa->type;
                  found = true;
               }
            }
            if (tda) {
               ++tdali;
            } else if (fali) {
               ++(*fali);
               fa = fali->current();
            }
         }

         delete fali;
         //printf("   srcList='%s' dstList='%s faList='%s'\n",
         //  argListToString(srclali.current()).data(),
         //  argListToString(dstlali.current()).data(),
         //  funcTempArgList ? argListToString(funcTempArgList).data() : "<none>");
      }
      dst += name;
      p = i + l;
   }
   dst += src.right(src.length() - p);
   //printf("  substituteTemplatesInString(%s)=%s\n",
   //    src.data(),dst.data());
   return dst;
}

void Doxy_Work::substituteTemplatesInArgList(const QList<ArgumentList> &srcTempArgLists, const QList<ArgumentList> &dstTempArgLists,
            ArgumentList *src, ArgumentList *dst, ArgumentList *funcTempArgs = 0)
{
   ArgumentListIterator sali(*src);
   ArgumentListIterator dali(*dst);
   Argument *sa = 0;
   Argument *da = dali.current();

   for (sali.toFirst(); (sa = sali.current()); ++sali) { // for each member argument
      QByteArray dstType = substituteTemplatesInString(
                              srcTempArgLists, dstTempArgLists, funcTempArgs,
                              sa->type);
      QByteArray dstArray = substituteTemplatesInString(
                               srcTempArgLists, dstTempArgLists, funcTempArgs,
                               sa->array);
      if (da == 0) {
         da = new Argument(*sa);
         dst->append(da);
         da->type = dstType;
         da->array = dstArray;
         da = 0;
      } else {
         da->type = dstType;
         da->type = dstArray;
         ++dali;
         da = dali.current();
      }
   }
   dst->constSpecifier     = src->constSpecifier;
   dst->volatileSpecifier  = src->volatileSpecifier;
   dst->pureSpecifier      = src->pureSpecifier;
   dst->trailingReturnType = substituteTemplatesInString(
                                srcTempArgLists, dstTempArgLists,
                                funcTempArgs, src->trailingReturnType);
   //printf("substituteTemplatesInArgList: replacing %s with %s\n",
   //    argListToString(src).data(),argListToString(dst).data()
   //    );
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
void Doxy_Work::findMember(EntryNav *rootNav, QByteArray funcDecl, bool overloaded, bool isFunc)
{
   Entry *root = rootNav->entry();

   Debug::print(Debug::FindMembers, 0,
                "findMember(root=%p,funcDecl=`%s',related=`%s',overload=%d,"
                "isFunc=%d mGrpId=%d tArgList=%p (#=%d) "
                "spec=%lld lang=%x\n",
                root, funcDecl.data(), root->relates.data(), overloaded, isFunc, root->mGrpId,
                root->tArgLists, root->tArgLists ? root->tArgLists->count() : 0,
                root->spec, root->lang
               );

   QByteArray scopeName;
   QByteArray className;
   QByteArray namespaceName;
   QByteArray funcType;
   QByteArray funcName;
   QByteArray funcArgs;
   QByteArray funcTempList;
   QByteArray exceptions;
   QByteArray funcSpec;

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
         root->spec |= Entry::Inline;
         done = false;
      }

      if (funcDecl.startsWith("explicit ")) {
         funcDecl = stripPrefix(funcDecl, "explicit ");
         root->spec |= Entry::Explicit;
         done = false;
      }

      if (funcDecl.startsWith("mutable ")) {
         funcDecl = stripPrefix(funcDecl, "mutable ");
         root->spec |= Entry::Mutable;
         done = false;
      }

      if (funcDecl.startsWith("virtual ")) {
         funcDecl = stripPrefix(funcDecl, "virtual ");
         done = false;
      }

   } while (!done);

   // delete any ; from the function declaration
   int sep;

   while ((sep = funcDecl.indexOf(';')) != -1) {
      funcDecl = (funcDecl.left(sep) + funcDecl.right(funcDecl.length() - sep - 1)).trimmed();
   }

   // make sure the first character is a space to simplify searching.
   if (!funcDecl.isEmpty() && funcDecl[0] != ' ') {
      funcDecl.prepend(" ");
   }

   // remove some superfluous spaces
   funcDecl = substitute(
                 substitute(
                    substitute(funcDecl, "~ ", "~"),
                    ":: ", "::"), 
                 " ::", "::").trimmed();

   //printf("funcDecl=`%s'\n",funcDecl.data());
   if (isFriend && funcDecl.left(6) == "class ") {
      //printf("friend class\n");
      funcDecl = funcDecl.right(funcDecl.length() - 6);
      funcName = funcDecl.copy();

   } else if (isFriend && funcDecl.left(7) == "struct ") {
      funcDecl = funcDecl.right(funcDecl.length() - 7);
      funcName = funcDecl.copy();

   } else {
      // extract information from the declarations
      parseFuncDecl(funcDecl, root->lang == SrcLangExt_ObjC, scopeName, funcType, funcName,
                    funcArgs, funcTempList, exceptions);
   }

   //printf("scopeName=`%s' funcType=`%s' funcName=`%s' funcArgs=`%s'\n",
   //    scopeName.data(),funcType.data(),funcName.data(),funcArgs.data());

   // the class name can also be a namespace name, we decide this later.
   // if a related class name is specified and the class name could
   // not be derived from the function declaration, then use the
   // related field.
   //printf("scopeName=`%s' className=`%s' namespaceName=`%s'\n",
   //    scopeName.data(),className.data(),namespaceName.data());

   if (!root->relates.isEmpty()) {
      // related member, prefix user specified scope
      isRelated = true;
      isMemberOf = (root->relatesType == MemberOf);

      if (getClass(root->relates) == 0 && !scopeName.isEmpty()) {
         scopeName = mergeScopes(scopeName, root->relates);
      } else {
         scopeName = root->relates;
      }
   }

   if (root->relates.isEmpty() && rootNav->parent() &&
         ((rootNav->parent()->section()&Entry::SCOPE_MASK) ||
          (rootNav->parent()->section() == Entry::OBJCIMPL_SEC) ) && 
           !rootNav->parent()->name().isEmpty()) {

      // see if we can combine scopeName
      // with the scope in which it was found

      QByteArray joinedName = rootNav->parent()->name() + "::" + scopeName;

      if (!scopeName.isEmpty() && (getClass(joinedName) || Doxygen::namespaceSDict->find(joinedName))) {
         scopeName = joinedName;

      } else {
         scopeName = mergeScopes(rootNav->parent()->name(), scopeName);
      }

   } else { // see if we can prefix a namespace or class that is used from the file
      FileDef *fd = rootNav->fileDef();

      if (fd) {
         NamespaceSDict *fnl = fd->getUsedNamespaces();
         if (fnl) {
            QByteArray joinedName;
            NamespaceDef *fnd;
            NamespaceSDict::Iterator nsdi(*fnl);
            for (nsdi.toFirst(); (fnd = nsdi.current()); ++nsdi) {
               joinedName = fnd->name() + "::" + scopeName;
               if (Doxygen::namespaceSDict->find(joinedName)) {
                  scopeName = joinedName;
                  break;
               }
            }
         }
      }
   }
   scopeName = stripTemplateSpecifiersFromScope(
                  removeRedundantWhiteSpace(scopeName), false, &funcSpec);

   // funcSpec contains the last template specifiers of the given scope.
   // If this method does not have any template arguments or they are
   // empty while funcSpec is not empty we assume this is a
   // specialization of a method. If not, we clear the funcSpec and treat
   // this as a normal method of a template class.
   if (!(root->tArgLists &&
         root->tArgLists->count() > 0 &&
         root->tArgLists->getFirst()->count() == 0
        )
      ) {
      funcSpec.resize(0);
   }

   // split scope into a namespace and a class part
   extractNamespaceName(scopeName, className, namespaceName, true);
   //printf("scopeName=`%s' className=`%s' namespaceName=`%s'\n",
   //       scopeName.data(),className.data(),namespaceName.data());

   //namespaceName=removeAnonymousScopes(namespaceName);
   if (namespaceName.find('@') != -1) {
      return;   // skip stuff in anonymous namespace...
   }

   //printf("namespaceName=`%s' className=`%s'\n",namespaceName.data(),className.data());
   // merge class and namespace scopes again
   scopeName.resize(0);
   if (!namespaceName.isEmpty()) {
      if (className.isEmpty()) {
         scopeName = namespaceName;
      } else if (!root->relates.isEmpty() || // relates command with explicit scope
                 !getClass(className)) { // class name only exists in a namespace
         scopeName = namespaceName + "::" + className;
      } else {
         scopeName = className;
      }
   } else if (!className.isEmpty()) {
      scopeName = className;
   }
   //printf("new scope=`%s'\n",scopeName.data());

   QByteArray tempScopeName = scopeName;
   ClassDef *cd = getClass(scopeName);
   if (cd) {
      if (funcSpec.isEmpty()) {
         int argListIndex = 0;
         tempScopeName = cd->qualifiedNameWithTemplateParameters(root->tArgLists, &argListIndex);
      } else {
         tempScopeName = scopeName + funcSpec;
      }
   }
   //printf("scopeName=%s cd=%p root->tArgLists=%p result=%s\n",
   //    scopeName.data(),cd,root->tArgLists,tempScopeName.data());

   //printf("scopeName=`%s' className=`%s'\n",scopeName.data(),className.data());
   // rebuild the function declaration (needed to get the scope right).
   if (!scopeName.isEmpty() && !isRelated && !isFriend && !Config_getBool("HIDE_SCOPE_NAMES")) {
      if (!funcType.isEmpty()) {
         if (isFunc) { // a function -> we use argList for the arguments
            funcDecl = funcType + " " + tempScopeName + "::" + funcName + funcTempList;
         } else {
            funcDecl = funcType + " " + tempScopeName + "::" + funcName + funcArgs;
         }
      } else {
         if (isFunc) { // a function => we use argList for the arguments
            funcDecl = tempScopeName + "::" + funcName + funcTempList;
         } else { // variable => add `argument' list
            funcDecl = tempScopeName + "::" + funcName + funcArgs;
         }
      }
   } else { // build declaration without scope
      if (!funcType.isEmpty()) { // but with a type
         if (isFunc) { // function => omit argument list
            funcDecl = funcType + " " + funcName + funcTempList;
         } else { // variable => add `argument' list
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

   if (funcType == "template class" && !funcTempList.isEmpty()) {
      return;   // ignore explicit template instantiations
   }

   Debug::print(Debug::FindMembers, 0,
                "findMember() Parse results:\n"
                "  namespaceName=`%s'\n"
                "  className=`%s`\n"
                "  funcType=`%s'\n"
                "  funcSpec=`%s'\n"
                "  funcName=`%s'\n"
                "  funcArgs=`%s'\n"
                "  funcTempList=`%s'\n"
                "  funcDecl=`%s'\n"
                "  related=`%s'\n"
                "  exceptions=`%s'\n"
                "  isRelated=%d\n"
                "  isMemberOf=%d\n"
                "  isFriend=%d\n"
                "  isFunc=%d\n\n",
                namespaceName.data(), className.data(),
                funcType.data(), funcSpec.data(), funcName.data(), funcArgs.data(), funcTempList.data(),
                funcDecl.data(), root->relates.data(), exceptions.data(), isRelated, isMemberOf, isFriend,
                isFunc
               );

   MemberName *mn = 0;
   if (!funcName.isEmpty()) { // function name is valid
      Debug::print(Debug::FindMembers, 0,
                   "1. funcName=`%s'\n", funcName.data());
      if (funcName.left(9) == "operator ") { // strip class scope from cast operator
         funcName = substitute(funcName, className + "::", "");
      }
      if (!funcTempList.isEmpty()) { // try with member specialization
         mn = Doxygen::memberNameSDict->find(funcName + funcTempList);
      }
      if (mn == 0) { // try without specialization
         mn = Doxygen::memberNameSDict->find(funcName);
      }
      if (!isRelated && mn) { // function name already found
         Debug::print(Debug::FindMembers, 0,
                      "2. member name exists (%d members with this name)\n", mn->count());
         if (!className.isEmpty()) { // class name is valid
            if (funcSpec.isEmpty()) { // not a member specialization
               int count = 0;
               int noMatchCount = 0;
               MemberNameIterator mni(*mn);
               MemberDef *md;
               bool memFound = false;
               for (mni.toFirst(); !memFound && (md = mni.current()); ++mni) {
                  ClassDef *cd = md->getClassDef();
                  Debug::print(Debug::FindMembers, 0,
                               "3. member definition found, "
                               "scope needed=`%s' scope=`%s' args=`%s' fileName=%s\n",
                               scopeName.data(), cd ? cd->name().data() : "<none>",
                               md->argsString(),
                               root->fileName.data());
                  //printf("Member %s (member scopeName=%s) (this scopeName=%s) classTempList=%s\n",md->name().data(),cd->name().data(),scopeName.data(),classTempList.data());
                  FileDef *fd = rootNav->fileDef();
                  NamespaceDef *nd = 0;
                  if (!namespaceName.isEmpty()) {
                     nd = getResolvedNamespace(namespaceName);
                  }

                  //printf("scopeName %s->%s\n",scopeName.data(),
                  //       stripTemplateSpecifiersFromScope(scopeName,false).data());

                  ClassDef *tcd = findClassDefinition(fd, nd, scopeName);
                  if (tcd == 0 && stripAnonymousNamespaceScope(cd->name()) == scopeName) {
                     // don't be fooled by anonymous scopes
                     tcd = cd;
                  }
                  //printf("Looking for %s inside nd=%s result=%p (%s) cd=%p\n",
                  //    scopeName.data(),nd?nd->name().data():"<none>",tcd,tcd?tcd->name().data():"",cd);

                  if (cd && tcd == cd) { // member's classes match
                     Debug::print(Debug::FindMembers, 0,
                                  "4. class definition %s found\n", cd->name().data());

                     // get the template parameter lists found at the member declaration
                     QList<ArgumentList> declTemplArgs;
                     cd->getTemplateParameterLists(declTemplArgs);
                     ArgumentList *templAl = md->templateArguments();
                     if (templAl) {
                        declTemplArgs.append(templAl);
                     }

                     // get the template parameter lists found at the member definition
                     QList<ArgumentList> *defTemplArgs = root->tArgLists;
                     //printf("defTemplArgs=%p\n",defTemplArgs);

                     // do we replace the decl argument lists with the def argument lists?
                     bool substDone = false;
                     ArgumentList *argList = 0;

                     /* substitute the occurrences of class template names in the
                      * argument list before matching
                      */
                     ArgumentList *mdAl = md->argumentList();
                     if (declTemplArgs.count() > 0 && defTemplArgs &&
                           declTemplArgs.count() == defTemplArgs->count() &&
                           mdAl
                        ) {
                        /* the function definition has template arguments
                         * and the class definition also has template arguments, so
                         * we must substitute the template names of the class by that
                         * of the function definition before matching.
                         */
                        argList = new ArgumentList;
                        substituteTemplatesInArgList(declTemplArgs, *defTemplArgs,
                                                     mdAl, argList);

                        substDone = true;
                     } else { /* no template arguments, compare argument lists directly */
                        argList = mdAl;
                     }

                     Debug::print(Debug::FindMembers, 0,
                                  "5. matching `%s'<=>`%s' className=%s namespaceName=%s\n",
                                  argListToString(argList, true).data(), argListToString(root->argList, true).data(),
                                  className.data(), namespaceName.data()
                                 );

                     bool matching =
                        md->isVariable() || md->isTypedef() || // needed for function pointers
                        (mdAl == 0 && root->argList->count() == 0) ||
                        matchArguments2(
                           md->getClassDef(), md->getFileDef(), argList,
                           cd, fd, root->argList,
                           true);

                     if (md->getLanguage() == SrcLangExt_ObjC && md->isVariable() && (root->section & Entry::FUNCTION_SEC)) {
                        matching = false; // don't match methods and attributes with the same name
                     }

                     // for template member we also need to check the return type
                     if (md->templateArguments() != 0 && root->tArgLists != 0) {
                        QByteArray memType = md->typeString();
                        memType.stripPrefix("static "); // see bug700696
                        funcType = substitute(stripTemplateSpecifiersFromScope(funcType, true),
                                              className + "::", ""); // see bug700693 & bug732594
                        Debug::print(Debug::FindMembers, 0,
                                     "5b. Comparing return types '%s'<->'%s' #args %d<->%d\n",
                                     md->typeString(), funcType.data(),
                                     md->templateArguments()->count(), root->tArgLists->getLast()->count());
                        if (md->templateArguments()->count() != root->tArgLists->getLast()->count() ||
                              qstrcmp(memType, funcType)) {
                           //printf(" ---> no matching\n");
                           matching = false;
                        }
                     }
                     bool rootIsUserDoc = (root->section & Entry::MEMBERDOC_SEC) != 0;
                     bool classIsTemplate = scopeIsTemplate(md->getClassDef());
                     bool mdIsTemplate    = md->templateArguments() != 0;
                     bool classOrMdIsTemplate = mdIsTemplate || classIsTemplate;
                     bool rootIsTemplate  = root->tArgLists != 0;
                     //printf("classIsTemplate=%d mdIsTemplate=%d rootIsTemplate=%d\n",classIsTemplate,mdIsTemplate,rootIsTemplate);
                     if (!rootIsUserDoc && // don't check out-of-line @fn references, see bug722457
                           (mdIsTemplate || rootIsTemplate) && // either md or root is a template
                           ((classOrMdIsTemplate && !rootIsTemplate) || (!classOrMdIsTemplate && rootIsTemplate))
                        ) {
                        // Method with template return type does not match method without return type
                        // even if the parameters are the same. See also bug709052
                        Debug::print(Debug::FindMembers, 0,
                                     "5b. Comparing return types: template v.s. non-template\n");
                        matching = false;
                     }


                     Debug::print(Debug::FindMembers, 0,
                                  "6. match results of matchArguments2 = %d\n", matching);

                     if (substDone) { // found a new argument list
                        if (matching) { // replace member's argument list
                           md->setDefinitionTemplateParameterLists(root->tArgLists);
                           md->setArgumentList(argList); // new owner of the list => no delete
                        } else { // no match
                           if (!funcTempList.isEmpty() &&
                                 isSpecialization(declTemplArgs, *defTemplArgs)) {
                              // check if we are dealing with a partial template
                              // specialization. In this case we add it to the class
                              // even though the member arguments do not match.

                              // TODO: copy other aspects?
                              root->protection = md->protection(); // copy protection level
                              addMethodToClass(rootNav, cd, md->name(), isFriend);
                              return;
                           }
                           delete argList;
                        }
                     }
                     if (matching) {
                        addMemberDocs(rootNav, md, funcDecl, 0, overloaded, 0/* TODO */);
                        count++;
                        memFound = true;
                     }
                  } else if (cd && cd != tcd) // we did find a class with the same name as cd
                     // but in a different namespace
                  {
                     noMatchCount++;
                  }
               }
               if (count == 0 && rootNav->parent() &&
                     rootNav->parent()->section() == Entry::OBJCIMPL_SEC) {
                  goto localObjCMethod;
               }
               if (count == 0 && !(isFriend && funcType == "class")) {
                  int candidates = 0;
                  ClassDef *ecd = 0, *ucd = 0;
                  MemberDef *emd = 0, *umd = 0;
                  if (mn->count() > 0) {
                     //printf("Assume template class\n");
                     for (mni.toFirst(); (md = mni.current()); ++mni) {
                        ClassDef *ccd = md->getClassDef();
                        MemberDef *cmd = md;
                        //printf("ccd->name()==%s className=%s\n",ccd->name().data(),className.data());
                        if (ccd != 0 && rightScopeMatch(ccd->name(), className)) {
                           ArgumentList *templAl = md->templateArguments();
                           if (root->tArgLists && templAl != 0 &&
                                 root->tArgLists->getLast()->count() <= templAl->count()) {
                              addMethodToClass(rootNav, ccd, md->name(), isFriend);
                              return;
                           }
                           if (md->argsString() == argListToString(root->argList, true, false)) {
                              // exact argument list match -> remember
                              ucd = ecd = ccd;
                              umd = emd = cmd;
                              Debug::print(Debug::FindMembers, 0,
                                           "7. new candidate className=%s scope=%s args=%s exact match\n",
                                           className.data(), ccd->name().data(), md->argsString());
                           } else { // arguments do not match, but member name and scope do -> remember
                              ucd = ccd;
                              umd = cmd;
                              Debug::print(Debug::FindMembers, 0,
                                           "7. new candidate className=%s scope=%s args=%s no match\n",
                                           className.data(), ccd->name().data(), md->argsString());
                           }
                           candidates++;
                        }
                     }
                  }
                  static bool strictProtoMatching = Config_getBool("STRICT_PROTO_MATCHING");
                  if (!strictProtoMatching) {
                     if (candidates == 1 && ucd && umd) {
                        // we didn't find an actual match on argument lists, but there is only 1 member with this
                        // name in the same scope, so that has to be the one.
                        addMemberDocs(rootNav, umd, funcDecl, 0, overloaded, 0);
                        return;
                     } else if (candidates > 1 && ecd && emd) {
                        // we didn't find a unique match using type resolution,
                        // but one of the matches has the exact same signature so
                        // we take that one.
                        addMemberDocs(rootNav, emd, funcDecl, 0, overloaded, 0);
                        return;
                     }
                  }

                  QByteArray warnMsg = "no ";
                  if (noMatchCount > 1) {
                     warnMsg += "uniquely ";
                  }
                  warnMsg += "matching class member found for \n";

                  if (root->tArgLists) {
                     QListIterator<ArgumentList> alli(*root->tArgLists);
                     ArgumentList *al;
                     for (; (al = alli.current()); ++alli) {
                        warnMsg += "  template ";
                        warnMsg += tempArgListToString(al, root->lang);
                        warnMsg += '\n';
                     }
                  }
                  QByteArray fullFuncDecl = funcDecl.copy();
                  if (isFunc) {
                     fullFuncDecl += argListToString(root->argList, true);
                  }

                  warnMsg += "  ";
                  warnMsg += fullFuncDecl;
                  warnMsg += '\n';

                  if (candidates > 0) {
                     warnMsg += "Possible candidates:\n";
                     for (mni.toFirst(); (md = mni.current()); ++mni) {
                        ClassDef *cd = md->getClassDef();
                        if (cd != 0 && rightScopeMatch(cd->name(), className)) {
                           ArgumentList *templAl = md->templateArguments();
                           if (templAl != 0) {
                              warnMsg += "  'template ";
                              warnMsg += tempArgListToString(templAl, root->lang);
                              warnMsg += '\n';
                           }
                           warnMsg += "  ";
                           if (md->typeString()) {
                              warnMsg += md->typeString();
                              warnMsg += ' ';
                           }
                           QByteArray qScope = cd->qualifiedNameWithTemplateParameters();
                           if (!qScope.isEmpty()) {
                              warnMsg += qScope + "::" + md->name();
                           }
                           if (md->argsString()) {
                              warnMsg += md->argsString();
                           }
                           if (noMatchCount > 1) {
                              warnMsg += "' at line " + QByteArray().setNum(md->getDefLine()) +
                                         " of file " + md->getDefFileName();
                           }

                           warnMsg += '\n';
                        }
                     }
                  }
                  warn_simple(root->fileName, root->startLine, warnMsg);
               }
            } else if (cd) { // member specialization
               MemberNameIterator mni(*mn);
               MemberDef *declMd = 0;
               MemberDef *md = 0;
               for (mni.toFirst(); (md = mni.current()); ++mni) {
                  if (md->getClassDef() == cd) {
                     // TODO: we should probably also check for matching arguments
                     declMd = md;
                     break;
                  }
               }
               MemberType mtype = MemberType_Function;
               ArgumentList *tArgList = new ArgumentList;
               //  getTemplateArgumentsFromName(cd->name()+"::"+funcName,root->tArgLists);
               md = new MemberDef(
                  root->fileName, root->startLine, root->startColumn,
                  funcType, funcName, funcArgs, exceptions,
                  declMd ? declMd->protection() : root->protection,
                  root->virt, root->stat, Member,
                  mtype, tArgList, root->argList);
               //printf("new specialized member %s args=`%s'\n",md->name().data(),funcArgs.data());
               md->setTagInfo(rootNav->tagInfo());
               md->setLanguage(root->lang);
               md->setId(root->id);
               md->setMemberClass(cd);
               md->setTemplateSpecialization(true);
               md->setTypeConstraints(root->typeConstr);
               md->setDefinition(funcDecl);
               md->enableCallGraph(root->callGraph);
               md->enableCallerGraph(root->callerGraph);
               md->setDocumentation(root->doc, root->docFile, root->docLine);
               md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
               md->setDocsForDefinition(!root->proto);
               md->setPrototype(root->proto);
               md->addSectionsToDefinition(root->anchors);
               md->setBodySegment(root->bodyLine, root->endBodyLine);
               FileDef *fd = rootNav->fileDef();
               md->setBodyDef(fd);
               md->setMemberSpecifiers(root->spec);
               md->setMemberGroupId(root->mGrpId);
               mn->append(md);
               cd->insertMember(md);
               md->setRefItems(root->sli);
               delete tArgList;
            } else {
               //printf("*** Specialized member %s of unknown scope %s%s found!\n",
               //        scopeName.data(),funcName.data(),funcArgs.data());
            }
         } else if (overloaded) { // check if the function belongs to only one class
            // for unique overloaded member we allow the class to be
            // omitted, this is to be Qt compatible. Using this should
            // however be avoided, because it is error prone
            MemberNameIterator mni(*mn);
            MemberDef *md = mni.toFirst();
            assert(md);
            ClassDef *cd = md->getClassDef();
            assert(cd);
            QByteArray className = cd->name().copy();
            ++mni;
            bool unique = true;
            for (; (md = mni.current()); ++mni) {
               ClassDef *cd = md->getClassDef();
               if (className != cd->name()) {
                  unique = false;
               }
            }
            if (unique) {
               MemberType mtype;
               if      (root->mtype == Signal) {
                  mtype = MemberType_Signal;
               } else if (root->mtype == Slot) {
                  mtype = MemberType_Slot;
               } else if (root->mtype == DCOP) {
                  mtype = MemberType_DCOP;
               } else {
                  mtype = MemberType_Function;
               }

               // new overloaded member function
               ArgumentList *tArgList =
                  getTemplateArgumentsFromName(cd->name() + "::" + funcName, root->tArgLists);
               //printf("new related member %s args=`%s'\n",md->name().data(),funcArgs.data());
               MemberDef *md = new MemberDef(
                  root->fileName, root->startLine, root->startColumn,
                  funcType, funcName, funcArgs, exceptions,
                  root->protection, root->virt, root->stat, Related,
                  mtype, tArgList, root->argList);
               md->setTagInfo(rootNav->tagInfo());
               md->setLanguage(root->lang);
               md->setId(root->id);
               md->setTypeConstraints(root->typeConstr);
               md->setMemberClass(cd);
               md->setDefinition(funcDecl);
               md->enableCallGraph(root->callGraph);
               md->enableCallerGraph(root->callerGraph);
               QByteArray doc = getOverloadDocs();
               doc += "<p>";
               doc += root->doc;
               md->setDocumentation(doc, root->docFile, root->docLine);
               md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
               md->setDocsForDefinition(!root->proto);
               md->setPrototype(root->proto);
               md->addSectionsToDefinition(root->anchors);
               md->setBodySegment(root->bodyLine, root->endBodyLine);
               FileDef *fd = rootNav->fileDef();
               md->setBodyDef(fd);
               md->setMemberSpecifiers(root->spec);
               md->setMemberGroupId(root->mGrpId);
               mn->append(md);
               cd->insertMember(md);
               cd->insertUsedFile(fd);
               md->setRefItems(root->sli);
            }
         } else { // unrelated function with the same name as a member
            if (!findGlobalMember(rootNav, namespaceName, funcType, funcName, funcTempList, funcArgs, funcDecl)) {
               QByteArray fullFuncDecl = funcDecl.copy();
               if (isFunc) {
                  fullFuncDecl += argListToString(root->argList, true);
               }
               warn(root->fileName, root->startLine,
                    "Cannot determine class for function\n%s",
                    fullFuncDecl.data()
                   );
            }
         }
      } else if (isRelated && !root->relates.isEmpty()) {
         Debug::print(Debug::FindMembers, 0, "2. related function\n"
                      "  scopeName=%s className=%s\n", scopeName.data(), className.data());
         if (className.isEmpty()) {
            className = root->relates;
         }
         ClassDef *cd;
         //printf("scopeName=`%s' className=`%s'\n",scopeName.data(),className.data());
         if ((cd = getClass(scopeName))) {
            bool newMember = true; // assume we have a new member
            bool newMemberName = false;
            MemberDef *mdDefine = 0;
            bool isDefine = false;
            {
               MemberName *mn = Doxygen::functionNameSDict->find(funcName);
               if (mn) {
                  MemberNameIterator mni(*mn);
                  mdDefine = mni.current();
                  while (mdDefine && !isDefine) {
                     isDefine = isDefine || mdDefine->isDefine();
                     if (!isDefine) {
                        ++mni;
                        mdDefine = mni.current();
                     }
                  }
               }
            }

            FileDef *fd = rootNav->fileDef();

            if ((mn = Doxygen::memberNameSDict->find(funcName)) == 0) {
               mn = new MemberName(funcName);
               newMemberName = true; // we create a new member name
            } else {
               MemberNameIterator mni(*mn);
               MemberDef *rmd;
               while ((rmd = mni.current()) && newMember) { // see if we got another member with matching arguments
                  ArgumentList *rmdAl = rmd->argumentList();

                  newMember =
                     className != rmd->getOuterScope()->name() ||
                     !matchArguments2(rmd->getOuterScope(), rmd->getFileDef(), rmdAl,
                                      cd, fd, root->argList,
                                      true);
                  if (newMember) {
                     ++mni;
                  }
               }
               if (!newMember && rmd) { // member already exists as rmd -> add docs
                  //printf("addMemberDocs for related member %s\n",root->name.data());
                  //rmd->setMemberDefTemplateArguments(root->mtArgList);
                  addMemberDocs(rootNav, rmd, funcDecl, 0, overloaded);
               }
            }

            if (newMember) { // need to create a new member
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

               //printf("New related name `%s' `%d'\n",funcName.data(),
               //    root->argList ? (int)root->argList->count() : -1);

               // first note that we pass:
               //   (root->tArgLists ? root->tArgLists->last() : 0)
               // for the template arguments fo the new "member."
               // this accurately reflects the template arguments of
               // the related function, which don't have to do with
               // those of the related class.
               MemberDef *md = new MemberDef(
                  root->fileName, root->startLine, root->startColumn,
                  funcType, funcName, funcArgs, exceptions,
                  root->protection, root->virt,
                  root->stat && !isMemberOf,
                  isMemberOf ? Foreign : Related,
                  mtype,
                  (root->tArgLists ? root->tArgLists->getLast() : 0),
                  funcArgs.isEmpty() ? 0 : root->argList);

               if (isDefine && mdDefine) {
                  md->setInitializer(mdDefine->initializer());
               }

               //
               // we still have the problem that
               // MemberDef::writeDocumentation() in memberdef.cpp
               // writes the template argument list for the class,
               // as if this member is a member of the class.
               // fortunately, MemberDef::writeDocumentation() has
               // a special mechanism that allows us to totally
               // override the set of template argument lists that
               // are printed.  We use that and set it to the
               // template argument lists of the related function.
               //
               md->setDefinitionTemplateParameterLists(root->tArgLists);

               md->setTagInfo(rootNav->tagInfo());



               //printf("Related member name=`%s' decl=`%s' bodyLine=`%d'\n",
               //       funcName.data(),funcDecl.data(),root->bodyLine);

               // try to find the matching line number of the body from the
               // global function list
               bool found = false;
               if (root->bodyLine == -1) {
                  MemberName *rmn = Doxygen::functionNameSDict->find(funcName);
                  if (rmn) {
                     MemberNameIterator rmni(*rmn);
                     MemberDef *rmd;
                     while ((rmd = rmni.current()) && !found) { // see if we got another member with matching arguments
                        ArgumentList *rmdAl = rmd->argumentList();
                        // check for matching argument lists
                        if (
                           matchArguments2(rmd->getOuterScope(), rmd->getFileDef(), rmdAl,
                                           cd, fd, root->argList,
                                           true)
                        ) {
                           found = true;
                        }
                        if (!found) {
                           ++rmni;
                        }
                     }
                     if (rmd) { // member found -> copy line number info
                        md->setBodySegment(rmd->getStartBodyLine(), rmd->getEndBodyLine());
                        md->setBodyDef(rmd->getBodyDef());
                        //md->setBodyMember(rmd);
                     }
                  }
               }
               if (!found) // line number could not be found or is available in this
                  // entry
               {
                  md->setBodySegment(root->bodyLine, root->endBodyLine);
                  md->setBodyDef(fd);
               }

               //if (root->mGrpId!=-1)
               //{
               //  md->setMemberGroup(memberGroupDict[root->mGrpId]);
               //}
               md->setMemberClass(cd);
               md->setMemberSpecifiers(root->spec);
               md->setDefinition(funcDecl);
               md->enableCallGraph(root->callGraph);
               md->enableCallerGraph(root->callerGraph);
               md->setDocumentation(root->doc, root->docFile, root->docLine);
               md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
               md->setDocsForDefinition(!root->proto);
               md->setPrototype(root->proto);
               md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
               md->addSectionsToDefinition(root->anchors);
               md->setMemberGroupId(root->mGrpId);
               md->setLanguage(root->lang);
               md->setId(root->id);
               //md->setMemberDefTemplateArguments(root->mtArgList);
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
               //printf("Adding member=%s\n",md->name().data());
               if (newMemberName) {
                  //Doxygen::memberNameList.append(mn);
                  //Doxygen::memberNameDict.insert(funcName,mn);
                  Doxygen::memberNameSDict->append(funcName, mn);
               }
            }
            if (root->relatesType == Duplicate) {
               if (!findGlobalMember(rootNav, namespaceName, funcType, funcName, funcTempList, funcArgs, funcDecl)) {
                  QByteArray fullFuncDecl = funcDecl.copy();
                  if (isFunc) {
                     fullFuncDecl += argListToString(root->argList, true);
                  }
                  warn(root->fileName, root->startLine,
                       "Cannot determine file/namespace for relatedalso function\n%s",
                       fullFuncDecl.data()
                      );
               }
            }
         } else {
            warn_undoc(root->fileName, root->startLine,
                       "class `%s' for related function `%s' is not "
                       "documented.",
                       className.data(), funcName.data()
                      );
         }
      } else if (rootNav->parent() && rootNav->parent()->section() == Entry::OBJCIMPL_SEC) {
      localObjCMethod:
         ClassDef *cd;
         //printf("scopeName=`%s' className=`%s'\n",scopeName.data(),className.data());
         if (Config_getBool("EXTRACT_LOCAL_METHODS") && (cd = getClass(scopeName))) {
            Debug::print(Debug::FindMembers, 0, "4. Local objective C method %s\n"
                         "  scopeName=%s className=%s\n", root->name.data(), scopeName.data(), className.data());
            //printf("Local objective C method `%s' of class `%s' found\n",root->name.data(),cd->name().data());
            MemberDef *md = new MemberDef(
               root->fileName, root->startLine, root->startColumn,
               funcType, funcName, funcArgs, exceptions,
               root->protection, root->virt, root->stat, Member,
               MemberType_Function, 0, root->argList);
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
            FileDef *fd = rootNav->fileDef();
            md->setBodyDef(fd);
            md->setMemberSpecifiers(root->spec);
            md->setMemberGroupId(root->mGrpId);
            cd->insertMember(md);
            cd->insertUsedFile(fd);
            md->setRefItems(root->sli);
            if ((mn = Doxygen::memberNameSDict->find(root->name))) {
               mn->append(md);
            } else {
               mn = new MemberName(root->name);
               mn->append(md);
               Doxygen::memberNameSDict->append(root->name, mn);
            }
         } else {
            // local objective C method found for class without interface
         }
      } else { // unrelated not overloaded member found
         bool globMem = findGlobalMember(rootNav, namespaceName, funcType, funcName, funcTempList, funcArgs, funcDecl);
         if (className.isEmpty() && !globMem) {
            warn(root->fileName, root->startLine,
                 "class for member `%s' cannot "
                 "be found.", funcName.data()
                );
         } else if (!className.isEmpty() && !globMem) {
            warn(root->fileName, root->startLine,
                 "member `%s' of class `%s' cannot be found",
                 funcName.data(), className.data());
         }
      }
   } else {
      // this should not be called
      warn(root->fileName, root->startLine,
           "member with no name found.");
   }
   return;
}

//----------------------------------------------------------------------
// find the members corresponding to the different documentation blocks
// that are extracted from the sources.
void Doxy_Work::filterMemberDocumentation(EntryNav *rootNav)
{
   Entry *root = rootNav->entry();
   int i = -1, l;
   Debug::print(Debug::FindMembers, 0,
                "findMemberDocumentation(): root->type=`%s' root->inside=`%s' root->name=`%s' root->args=`%s' section=%x root->spec=%lld root->mGrpId=%d\n",
                root->type.data(), root->inside.data(), root->name.data(), root->args.data(), root->section, root->spec, root->mGrpId
               );
   //printf("rootNav->parent()->name()=%s\n",rootNav->parent()->name().data());
   bool isFunc = true;

   if (root->relatesType == Duplicate && !root->relates.isEmpty()) {
      QByteArray tmp = root->relates;
      root->relates.resize(0);
      filterMemberDocumentation(rootNav);
      root->relates = tmp;
   }

   if ( // detect func variable/typedef to func ptr
      (i = findFunctionPtr(root->type, root->lang, &l)) != -1
   ) {
      //printf("Fixing function pointer!\n");
      // fix type and argument
      root->args.prepend(root->type.right(root->type.length() - i - l));
      root->type = root->type.left(i + l);
      //printf("Results type=%s,name=%s,args=%s\n",root->type.data(),root->name.data(),root->args.data());
      isFunc = false;
   } else if ((root->type.left(8) == "typedef " && root->args.find('(') != -1))
      // detect function types marked as functions
   {
      isFunc = false;
   }

   //printf("Member %s isFunc=%d\n",root->name.data(),isFunc);
   if (root->section == Entry::MEMBERDOC_SEC) {
      //printf("Documentation for inline member `%s' found args=`%s'\n",
      //    root->name.data(),root->args.data());
      //if (root->relates.length()) printf("  Relates %s\n",root->relates.data());
      if (root->type.isEmpty()) {
         findMember(rootNav, root->name + root->args + root->exception, false, isFunc);
      } else {
         findMember(rootNav, root->type + " " + root->name + root->args + root->exception, false, isFunc);
      }
   } else if (root->section == Entry::OVERLOADDOC_SEC) {
      //printf("Overloaded member %s found\n",root->name.data());
      findMember(rootNav, root->name, true, isFunc);
   } else if
   ((root->section == Entry::FUNCTION_SEC    // function
         ||
         (root->section == Entry::VARIABLE_SEC && // variable
          !root->type.isEmpty() &&                // with a type
          g_compoundKeywordDict.find(root->type) == 0 // that is not a keyword
          // (to skip forward declaration of class etc.)
         )
    )
   ) {
      //printf("Documentation for member `%s' found args=`%s' excp=`%s'\n",
      //    root->name.data(),root->args.data(),root->exception.data());
      //if (root->relates.length()) printf("  Relates %s\n",root->relates.data());
      //printf("Inside=%s\n Relates=%s\n",root->inside.data(),root->relates.data());
      if (root->type == "friend class" || root->type == "friend struct" ||
            root->type == "friend union") {
         findMember(rootNav,
                    root->type + " " +
                    root->name,
                    false, false);

      } else if (!root->type.isEmpty()) {
         findMember(rootNav,
                    root->type + " " +
                    root->inside +
                    root->name +
                    root->args +
                    root->exception,
                    false, isFunc);
      } else {
         findMember(rootNav,
                    root->inside +
                    root->name +
                    root->args +
                    root->exception,
                    false, isFunc);
      }
   } else if (root->section == Entry::DEFINE_SEC && !root->relates.isEmpty()) {
      findMember(rootNav, root->name + root->args, false, !root->args.isEmpty());
   } else if (root->section == Entry::VARIABLEDOC_SEC) {
      //printf("Documentation for variable %s found\n",root->name.data());
      //if (!root->relates.isEmpty()) printf("  Relates %s\n",root->relates.data());
      findMember(rootNav, root->name, false, false);
   } else if (root->section == Entry::EXPORTED_INTERFACE_SEC ||
              root->section == Entry::INCLUDED_SERVICE_SEC) {
      findMember(rootNav, root->type + " " + root->name, false, false);
   } else {
      // skip section
      //printf("skip section\n");
   }
}

void Doxy_Work::findMemberDocumentation(EntryNav *rootNav)
{
   if (rootNav->section() == Entry::MEMBERDOC_SEC || rootNav->section() == Entry::OVERLOADDOC_SEC ||
         rootNav->section() == Entry::FUNCTION_SEC || rootNav->section() == Entry::VARIABLE_SEC ||
         rootNav->section() == Entry::VARIABLEDOC_SEC || rootNav->section() == Entry::DEFINE_SEC ||
         rootNav->section() == Entry::INCLUDED_SERVICE_SEC || rootNav->section() == Entry::EXPORTED_INTERFACE_SEC ) {

      rootNav->loadEntry(Doxy_Globals::g_storage);

      filterMemberDocumentation(rootNav);

      rootNav->releaseEntry();
   }
   if (rootNav->children()) {
      for (auto e : *rootNav->children() ) {
         if (e->section() != Entry::ENUM_SEC) {
            findMemberDocumentation(e);
         }
      }
   }
}

void Doxy_Work::findObjCMethodDefinitions(EntryNav *rootNav)
{
   if (rootNav->children()) {

      for (auto objCIplNav : *rootNav->children() ) {

         if (objCImplNav->section() == Entry::OBJCIMPL_SEC && objCImplNav->children()) {

            for (auto objCMethodNav : *objCImplNav->children() ) {

               if (objCMethodNav->section() == Entry::FUNCTION_SEC) {
                  objCMethodNav->loadEntry(Doxy_Globals::g_storage);
                  Entry *objCMethod = objCMethodNav->entry();

                  //Printf("  Found ObjC method definition %s\n",objCMethod->name.data());
                  findMember(objCMethodNav, objCMethod->type + " " + objCImplNav->name() + "::" +
                             objCMethod->name + " " + objCMethod->args, false, true);

                  objCMethod->section = Entry::EMPTY_SEC;

                  objCMethodNav->releaseEntry();
               }
            }
         }
      }
   }
}

// find and add the enumeration to their classes, namespaces or files
void Doxy_Work::findEnums(EntryNav *rootNav)
{
   if (rootNav->section() == Entry::ENUM_SEC) {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      MemberDef      *md = 0;
      ClassDef       *cd = 0;
      FileDef        *fd = 0;
      NamespaceDef   *nd = 0;
      MemberNameSDict *mnsd = 0;
      bool isGlobal;
      bool isRelated = false;
      bool isMemberOf = false;
      //printf("Found enum with name `%s' relates=%s\n",root->name.data(),root->relates.data());
      int i;

      QByteArray name;
      QByteArray scope;

      if ((i = root->name.lastIndexOf("::")) != -1) { // scope is specified
         scope = root->name.left(i); // extract scope
         name = root->name.right(root->name.length() - i - 2); // extract name
         if ((cd = getClass(scope)) == 0) {
            nd = getResolvedNamespace(scope);
         }
      } else { // no scope, check the scope in which the docs where found
         if (( rootNav->parent()->section() & Entry::SCOPE_MASK )
               && !rootNav->parent()->name().isEmpty()
            ) { // found enum docs inside a compound
            scope = rootNav->parent()->name();
            if ((cd = getClass(scope)) == 0) {
               nd = getResolvedNamespace(scope);
            }
         }
         name = root->name;
      }

      if (!root->relates.isEmpty()) {
         // related member, prefix user specified scope
         isRelated = true;
         isMemberOf = (root->relatesType == MemberOf);
         if (getClass(root->relates) == 0 && !scope.isEmpty()) {
            scope = mergeScopes(scope, root->relates);
         } else {
            scope = root->relates.copy();
         }
         if ((cd = getClass(scope)) == 0) {
            nd = getResolvedNamespace(scope);
         }
      }

      if (cd && !name.isEmpty()) { // found a enum inside a compound
         //printf("Enum `%s'::`%s'\n",cd->name(),name.data());
         fd = 0;
         mnsd = Doxygen::memberNameSDict;
         isGlobal = false;
      } else if (nd && !nd->name().isEmpty() && nd->name().at(0) != '@') { // found enum inside namespace
         mnsd = Doxygen::functionNameSDict;
         isGlobal = true;
      } else { // found a global enum
         fd = rootNav->fileDef();
         mnsd = Doxygen::functionNameSDict;
         isGlobal = true;
      }

      if (!name.isEmpty()) {
         // new enum type
         md = new MemberDef(
            root->fileName, root->startLine, root->startColumn,
            0, name, 0, 0,
            root->protection, Normal, false,
            isMemberOf ? Foreign : isRelated ? Related : Member,
            MemberType_Enumeration,
            0, 0);
         md->setTagInfo(rootNav->tagInfo());
         md->setLanguage(root->lang);
         md->setId(root->id);
         if (!isGlobal) {
            md->setMemberClass(cd);
         } else {
            md->setFileDef(fd);
         }
         md->setBodySegment(root->bodyLine, root->endBodyLine);
         md->setBodyDef(rootNav->fileDef());
         md->setMemberSpecifiers(root->spec); // UNO IDL "published"
         md->setEnumBaseType(root->args);
         //printf("Enum %s definition at line %d of %s: protection=%d\n",
         //    root->name.data(),root->bodyLine,root->fileName.data(),root->protection);
         md->addSectionsToDefinition(root->anchors);
         md->setMemberGroupId(root->mGrpId);
         md->enableCallGraph(root->callGraph);
         md->enableCallerGraph(root->callerGraph);
         //printf("%s::setRefItems(%d)\n",md->name().data(),root->sli?root->sli->count():-1);
         md->setRefItems(root->sli);
         //printf("found enum %s nd=%p\n",name.data(),nd);
         bool defSet = false;

         QByteArray baseType = root->args;
         if (!baseType.isEmpty()) {
            baseType.prepend(" : ");
         }

         if (nd && !nd->name().isEmpty() && nd->name().at(0) != '@') {
            if (isRelated || Config_getBool("HIDE_SCOPE_NAMES")) {
               md->setDefinition(name + baseType);
            } else {
               md->setDefinition(nd->name() + "::" + name + baseType);
            }
            //printf("definition=%s\n",md->definition());
            defSet = true;
            md->setNamespace(nd);
            nd->insertMember(md);
         }

         // even if we have already added the enum to a namespace, we still
         // also want to add it to other appropriate places such as file
         // or class.
         if (isGlobal) {
            if (!defSet) {
               md->setDefinition(name + baseType);
            }
            if (fd == 0 && rootNav->parent()) {
               fd = rootNav->parent()->fileDef();
            }
            if (fd) {
               md->setFileDef(fd);
               fd->insertMember(md);
            }
         } else if (cd) {
            if (isRelated || Config_getBool("HIDE_SCOPE_NAMES")) {
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

         //printf("Adding member=%s\n",md->name().data());
         MemberName *mn;
         if ((mn = (*mnsd)[name])) {
            // this is used if the same enum is in multiple namespaces/classes
            mn->append(md);
         } else { // new enum name
            mn = new MemberName(name);
            mn->append(md);
            mnsd->append(name, mn);
            //printf("add %s to new memberName. Now %d members\n",
            //       name.data(),mn->count());
         }
         addMemberToGroups(root, md);
      }
      rootNav->releaseEntry();
   } else {
      RECURSE_ENTRYTREE(findEnums, rootNav);
   }
}

void Doxy_Work::addEnumValuesToEnums(EntryNav *rootNav)
{
   if (rootNav->section() == Entry::ENUM_SEC)
      // non anonymous enumeration
   {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      ClassDef       *cd = 0;
      FileDef        *fd = 0;
      NamespaceDef   *nd = 0;
      MemberNameSDict *mnsd = 0;
      bool isGlobal;
      bool isRelated = false;
      //printf("Found enum with name `%s' relates=%s\n",root->name.data(),root->relates.data());
      int i;

      QByteArray name;
      QByteArray scope;

      if ((i = root->name.lastIndexOf("::")) != -1) { // scope is specified
         scope = root->name.left(i); // extract scope
         name = root->name.right(root->name.length() - i - 2); // extract name
         if ((cd = getClass(scope)) == 0) {
            nd = getResolvedNamespace(scope);
         }
      } else { // no scope, check the scope in which the docs where found
         if (( rootNav->parent()->section() & Entry::SCOPE_MASK )
               && !rootNav->parent()->name().isEmpty()
            ) { // found enum docs inside a compound
            scope = rootNav->parent()->name();
            if ((cd = getClass(scope)) == 0) {
               nd = getResolvedNamespace(scope);
            }
         }
         name = root->name;
      }

      if (!root->relates.isEmpty()) {
         // related member, prefix user specified scope
         isRelated = true;
         if (getClass(root->relates) == 0 && !scope.isEmpty()) {
            scope = mergeScopes(scope, root->relates);
         } else {
            scope = root->relates.copy();
         }
         if ((cd = getClass(scope)) == 0) {
            nd = getResolvedNamespace(scope);
         }
      }

      if (cd && !name.isEmpty()) { // found a enum inside a compound
         //printf("Enum in class `%s'::`%s'\n",cd->name().data(),name.data());
         fd = 0;
         mnsd = Doxygen::memberNameSDict;
         isGlobal = false;
      } else if (nd && !nd->name().isEmpty() && nd->name().at(0) != '@') { // found enum inside namespace
         //printf("Enum in namespace `%s'::`%s'\n",nd->name().data(),name.data());
         mnsd = Doxygen::functionNameSDict;
         isGlobal = true;
      } else { // found a global enum
         fd = rootNav->fileDef();
         //printf("Enum in file `%s': `%s'\n",fd->name().data(),name.data());
         mnsd = Doxygen::functionNameSDict;
         isGlobal = true;
      }

      if (!name.isEmpty()) {
         //printf("** name=%s\n",name.data());
         MemberName *mn = mnsd->find(name); // for all members with this name
         if (mn) {
            MemberNameIterator mni(*mn);
            MemberDef *md;

            for (mni.toFirst(); (md = mni.current()) ; ++mni) { // for each enum in this list

               if (md->isEnumerate() && rootNav->children()) {
                  //printf("   enum with %d children\n",rootNav->children()->count());

                  for (auto e : *rootNav->children() ) {

                     SrcLangExt sle;
                     if (
                        (sle = rootNav->lang()) == SrcLangExt_CSharp ||
                        sle == SrcLangExt_Java ||
                        sle == SrcLangExt_XML ||
                        (root->spec & Entry::Strong)
                     ) {
                        // Unlike classic C/C++ enums, for C++11, C# & Java enum
                        // values are only visible inside the enum scope, so we must create
                        // them here and only add them to the enum
                        e->loadEntry(Doxy_Globals::g_storage);

                        Entry *root = e->entry();

                        //printf("md->qualifiedName()=%s rootNav->name()=%s tagInfo=%p name=%s\n",
                        //    md->qualifiedName().data(),rootNav->name().data(),rootNav->tagInfo(),root->name.data());
                        if (substitute(md->qualifiedName(), "::", ".") == // TODO: add function to get canonical representation
                              substitute(rootNav->name(), "::", ".") ||  // enum value scope matches that of the enum
                              rootNav->tagInfo()                         // be less strict for tag files as members can have incomplete scope
                           ) {
                           MemberDef *fmd = new MemberDef(
                              root->fileName, root->startLine, root->startColumn,
                              root->type, root->name, root->args, 0,
                              Public, Normal, root->stat, Member,
                              MemberType_EnumValue, 0, 0);
                           if      (md->getClassDef()) {
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
                           MemberName *mn = mnsd->find(root->name);
                           if (mn) {
                              mn->append(fmd);
                           } else {
                              mn = new MemberName(root->name);
                              mn->append(fmd);
                              mnsd->append(root->name, mn);
                           }
                        }
                        e->releaseEntry();
                     } else {
                        //printf("e->name=%s isRelated=%d\n",e->name().data(),isRelated);
                        MemberName *fmn = 0;
                        MemberNameSDict *emnsd = isRelated ? Doxygen::functionNameSDict : mnsd;
                        if (!e->name().isEmpty() && (fmn = (*emnsd)[e->name()]))
                           // get list of members with the same name as the field
                        {
                           MemberNameIterator fmni(*fmn);
                           MemberDef *fmd;
                           for (fmni.toFirst(); (fmd = fmni.current()) ; ++fmni) {
                              if (fmd->isEnumValue() && fmd->getOuterScope() == md->getOuterScope()) { // in same scope
                                 //printf("found enum value with same name %s in scope %s\n",
                                 //    fmd->name().data(),fmd->getOuterScope()->name().data());
                                 if (nd && !nd->name().isEmpty() && nd->name().at(0) != '@') {
                                    NamespaceDef *fnd = fmd->getNamespaceDef();
                                    if (fnd == nd) { // enum value is inside a namespace
                                       md->insertEnumField(fmd);
                                       fmd->setEnumScope(md);
                                    }
                                 } else if (isGlobal) {
                                    FileDef *ffd = fmd->getFileDef();
                                    if (ffd == fd) { // enum value has file scope
                                       md->insertEnumField(fmd);
                                       fmd->setEnumScope(md);
                                    }
                                 } else if (isRelated && cd) // reparent enum value to
                                    // match the enum's scope
                                 {
                                    md->insertEnumField(fmd);   // add field def to list
                                    fmd->setEnumScope(md);      // cross ref with enum name
                                    fmd->setEnumClassScope(cd); // cross ref with enum name
                                    fmd->setOuterScope(cd);
                                    fmd->makeRelated();
                                    cd->insertMember(fmd);
                                 } else {
                                    ClassDef *fcd = fmd->getClassDef();
                                    if (fcd == cd) { // enum value is inside a class
                                       //printf("Inserting enum field %s in enum scope %s\n",
                                       //    fmd->name().data(),md->name().data());
                                       md->insertEnumField(fmd); // add field def to list
                                       fmd->setEnumScope(md);    // cross ref with enum name
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
void Doxy_Work::findEnumDocumentation(EntryNav *rootNav)
{
   if (rootNav->section() == Entry::ENUMDOC_SEC && !rootNav->name().isEmpty() && rootNav->name().at(0) != '@') {
      // skip anonymous enums
      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      int i;
      QByteArray name;
      QByteArray scope;
      if ((i = root->name.lastIndexOf("::")) != -1) { // scope is specified as part of the name
         name = root->name.right(root->name.length() - i - 2); // extract name
         scope = root->name.left(i); // extract scope
         //printf("Scope=`%s' Name=`%s'\n",scope.data(),name.data());
      } else { // just the name
         name = root->name;
      }
      if (( rootNav->parent()->section() & Entry::SCOPE_MASK )
            && !rootNav->parent()->name().isEmpty()
         ) { // found enum docs inside a compound
         if (!scope.isEmpty()) {
            scope.prepend("::");
         }
         scope.prepend(rootNav->parent()->name());
      }
      ClassDef *cd = getClass(scope);

      if (!name.isEmpty()) {
         bool found = false;
         if (cd) {
            //printf("Enum: scope=`%s' name=`%s'\n",cd->name(),name.data());
            QByteArray className = cd->name().copy();
            MemberName *mn = Doxygen::memberNameSDict->find(name);
            if (mn) {
               MemberNameIterator mni(*mn);
               MemberDef *md;
               for (mni.toFirst(); (md = mni.current()) && !found; ++mni) {
                  ClassDef *cd = md->getClassDef();
                  if (cd && cd->name() == className && md->isEnumerate()) {
                     // documentation outside a compound overrides the documentation inside it
#if 0
                     if (!md->documentation() || rootNav->parent()->name().isEmpty())
#endif
                     {
                        md->setDocumentation(root->doc, root->docFile, root->docLine);
                        md->setDocsForDefinition(!root->proto);
                     }

                     // brief descriptions inside a compound override the documentation
                     // outside it
#if 0
                     if (!md->briefDescription() || !rootNav->parent()->name().isEmpty())
#endif
                     {
                        md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
                     }

                     if (!md->inbodyDocumentation() || !rootNav->parent()->name().isEmpty()) {
                        md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
                     }

                     if (root->mGrpId != -1 && md->getMemberGroupId() == -1) {
                        md->setMemberGroupId(root->mGrpId);
                     }

                     md->addSectionsToDefinition(root->anchors);
                     md->setRefItems(root->sli);

                     GroupDef *gd = md->getGroupDef();
                     if (gd == 0 && root->groups->getFirst() != 0) { // member not grouped but out-of-line documentation is
                        addMemberToGroups(root, md);
                     }

                     found = true;
                  }
               }
            } else {
               //printf("MemberName %s not found!\n",name.data());
            }
         } else { // enum outside class
            //printf("Enum outside class: %s grpId=%d\n",name.data(),root->mGrpId);
            MemberName *mn = Doxygen::functionNameSDict->find(name);
            if (mn) {
               MemberNameIterator mni(*mn);
               MemberDef *md;
               for (mni.toFirst(); (md = mni.current()) && !found; ++mni) {
                  if (md->isEnumerate()) {
                     md->setDocumentation(root->doc, root->docFile, root->docLine);
                     md->setDocsForDefinition(!root->proto);
                     md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
                     md->setInbodyDocumentation(root->inbodyDocs, root->inbodyFile, root->inbodyLine);
                     md->addSectionsToDefinition(root->anchors);
                     md->setMemberGroupId(root->mGrpId);

                     GroupDef *gd = md->getGroupDef();
                     if (gd == 0 && root->groups->getFirst() != 0) { // member not grouped but out-of-line documentation is
                        addMemberToGroups(root, md);
                     }

                     found = true;
                  }
               }
            }
         }
         if (!found) {
            warn(root->fileName, root->startLine,
                 "Documentation for undefined enum `%s' found.",
                 name.data()
                );
         }
      }

      rootNav->releaseEntry();
   }
   RECURSE_ENTRYTREE(findEnumDocumentation, rootNav);
}

// search for each enum (member or function) in mnl if it has documented
// enum values.
void Doxy_Work::findDEV(const MemberNameSDict &mnsd)
{
   MemberName *mn;
   MemberNameSDict::Iterator mnli(mnsd);
   // for each member name
   for (mnli.toFirst(); (mn = mnli.current()); ++mnli) {
      MemberDef *md;
      MemberNameIterator mni(*mn);
      // for each member definition
      for (mni.toFirst(); (md = mni.current()); ++mni) {
         if (md->isEnumerate()) { // member is an enum
            MemberList *fmdl = md->enumFieldList();
            int documentedEnumValues = 0;
            if (fmdl) { // enum has values
               QListIterator<MemberDef> fmni(*fmdl);
               MemberDef *fmd;
               // for each enum value
               for (fmni.toFirst(); (fmd = fmni.current()); ++fmni) {
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

// search for each enum (member or function) if it has documented enum
// values.
void Doxy_Work::findDocumentedEnumValues()
{
   findDEV(*Doxygen::memberNameSDict);
   findDEV(*Doxygen::functionNameSDict);
}

void Doxy_Work::addMembersToIndex()
{
   MemberName *mn;
   MemberNameSDict::Iterator mnli(*Doxygen::memberNameSDict);

   // for each member name
   for (mnli.toFirst(); (mn = mnli.current()); ++mnli) {
      MemberDef *md;
      MemberNameIterator mni(*mn);

      // for each member definition
      for (mni.toFirst(); (md = mni.current()); ++mni) {
         addClassMemberNameToIndex(md);
      }
   }
   MemberNameSDict::Iterator fnli(*Doxygen::functionNameSDict);
   // for each member name
   for (fnli.toFirst(); (mn = fnli.current()); ++fnli) {
      MemberDef *md;
      MemberNameIterator mni(*mn);
      // for each member definition
      for (mni.toFirst(); (md = mni.current()); ++mni) {
         if (md->getNamespaceDef()) {
            addNamespaceMemberNameToIndex(md);
         } else {
            addFileMemberNameToIndex(md);
         }
      }
   }
}

// computes the relation between all members. For each member `m'
// the members that override the implementation of `m' are searched and
// the member that `m' overrides is searched.

void Doxy_Work::computeMemberRelations()
{
   MemberNameSDict::Iterator mnli(*Doxygen::memberNameSDict);
   MemberName *mn;

   for ( ; (mn = mnli.current()) ; ++mnli ) { // for each member name
      MemberNameIterator mdi(*mn);
      MemberNameIterator bmdi(*mn);
      MemberDef *md;
      MemberDef *bmd;
      for ( ; (md = mdi.current()) ; ++mdi ) { // for each member with a specific name
         for ( bmdi.toFirst() ; (bmd = bmdi.current()); ++bmdi ) { // for each other member with the same name
            ClassDef *mcd  = md->getClassDef();
            if (mcd && mcd->baseClasses()) {
               ClassDef *bmcd = bmd->getClassDef();
               //printf("Check relation between `%s'::`%s' (%p) and `%s'::`%s' (%p)\n",
               //      mcd->name().data(),md->name().data(),md,
               //       bmcd->name().data(),bmd->name().data(),bmd
               //      );
               if (md != bmd && bmcd && mcd && bmcd != mcd &&
                     (bmd->virtualness() != Normal ||
                      bmcd->compoundType() == ClassDef::Interface ||
                      bmcd->compoundType() == ClassDef::Protocol
                     ) &&
                     md->isFunction() &&
                     mcd->isLinkable() &&
                     bmcd->isLinkable() &&
                     mcd->isBaseClass(bmcd, true)) {
                  //printf("  derived scope\n");
                  ArgumentList *bmdAl = bmd->argumentList();
                  ArgumentList *mdAl =  md->argumentList();
                  //printf(" Base argList=`%s'\n Super argList=`%s'\n",
                  //        argListToString(bmdAl.pointer()).data(),
                  //        argListToString(mdAl.pointer()).data()
                  //      );
                  if (
                     matchArguments2(bmd->getOuterScope(), bmd->getFileDef(), bmdAl,
                                     md->getOuterScope(), md->getFileDef(), mdAl,
                                     true
                                    )
                  ) {
                     MemberDef *rmd;
                     if ((rmd = md->reimplements()) == 0 ||
                           minClassDistance(mcd, bmcd) < minClassDistance(mcd, rmd->getClassDef())
                        ) {
                        //printf("setting (new) reimplements member\n");
                        md->setReimplements(bmd);
                     }
                     //printf("%s: add reimplementedBy member %s\n",bmcd->name().data(),mcd->name().data());
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
   ClassSDict::Iterator cli(*Doxygen::classSDict);
   ClassDef *cd;

   // for each class
   for (cli.toFirst(); (cd = cli.current()); ++cli) {
      // that is a template
      QHash<QString, ClassDef> *templInstances = cd->getTemplateInstances();

      if (templInstances) {
         QHashIterator<QString, ClassDef> qdi(*templInstances);
         ClassDef *tcd = 0;

         // for each instance of the template
         for (qdi.toFirst(); (tcd = qdi.current()); ++qdi) {
            tcd->addMembersToTemplateInstance(cd, qdi.currentKey());
         }
      }
   }
}

void Doxy_Work::mergeCategories()
{
   ClassDef *cd;
   ClassSDict::Iterator cli(*Doxygen::classSDict);
   // merge members of categories into the class they extend
   for (cli.toFirst(); (cd = cli.current()); ++cli) {
      int i = cd->name().find('(');
      if (i != -1) { // it is an Objective-C category
         QByteArray baseName = cd->name().left(i);
         ClassDef *baseClass = Doxygen::classSDict->find(baseName);
         if (baseClass) {
            //printf("*** merging members of category %s into %s\n",
            //    cd->name().data(),baseClass->name().data());
            baseClass->mergeCategory(cd);
         }
      }
   }
}

// builds the list of all members for each class
void Doxy_Work::buildCompleteMemberLists()
{
   ClassDef *cd;
   ClassSDict::Iterator cli(*Doxygen::classSDict);

   // merge the member list of base classes into the inherited classes.
   for (cli.toFirst(); (cd = cli.current()); ++cli) {
      if (// !cd->isReference() && // not an external class
         cd->subClasses() == 0 && // is a root of the hierarchy
         cd->baseClasses()) { // and has at least one base class
         //printf("*** merging members for %s\n",cd->name().data());
         cd->mergeMembers();
      }
   }
}


void Doxy_Work::generateFileSources()
{
   if (Doxygen::inputNameList->count() > 0) {

#if USE_LIBCLANG
      static bool clangAssistedParsing = Config_getBool("CLANG_ASSISTED_PARSING");

      if (clangAssistedParsing) {
         QHash<QString, void *> g_processedFiles;

         // create a dictionary with files to process
         QHash<QString, void *> g_filesToProcess;
         FileNameListIterator fnli(*Doxygen::inputNameList);
         FileName *fn;

         for (fnli.toFirst(); (fn = fnli.current()); ++fnli) {
            FileNameIterator fni(*fn);
            FileDef *fd;

            for (; (fd = fni.current()); ++fni) {
               g_filesToProcess.insert(fd->absoluteFilePath(), (void *)0x8);
            }
         }
         // process source files (and their include dependencies)
         for (fnli.toFirst(); (fn = fnli.current()); ++fnli) {
            FileNameIterator fni(*fn);
            FileDef *fd;
            for (; (fd = fni.current()); ++fni) {
               if (fd->isSource() && !fd->isReference()) {
                  QStringList filesInSameTu;
                  fd->getAllIncludeFilesRecursively(filesInSameTu);
                  fd->startParsing();
                  if (fd->generateSourceFile()) { // sources need to be shown in the output
                     msg("Generating code for file %s...\n", fd->docName().data());
                     fd->writeSource(*Doxy_Globals::g_outputList, false, filesInSameTu);

                  } else if (!fd->isReference() && Doxygen::parseSourcesNeeded)
                     // we needed to parse the sources even if we do not show them
                  {
                     msg("Parsing code for file %s...\n", fd->docName().data());
                     fd->parseSource(false, filesInSameTu);
                  }

                  char *incFile = filesInSameTu.first();
                  while (incFile && g_filesToProcess.find(incFile)) {
                     if (fd->absoluteFilePath() != incFile && !g_processedFiles.find(incFile)) {
                        QStringList moreFiles;
                        bool ambig;
                        FileDef *ifd = findFileDef(Doxygen::inputNameDict, incFile, ambig);
                        if (ifd && !ifd->isReference()) {
                           if (ifd->generateSourceFile()) { // sources need to be shown in the output
                              msg(" Generating code for file %s...\n", ifd->docName().data());
                              ifd->writeSource(*Doxy_Globals::g_outputList, true, moreFiles);

                           } else if (!ifd->isReference() && Doxygen::parseSourcesNeeded)
                              // we needed to parse the sources even if we do not show them
                           {
                              msg(" Parsing code for file %s...\n", ifd->docName().data());
                              ifd->parseSource(true, moreFiles);
                           }
                           g_processedFiles.insert(incFile, (void *)0x8);
                        }
                     }
                     incFile = filesInSameTu.next();
                  }
                  fd->finishParsing();
                  g_processedFiles.insert(fd->absoluteFilePath(), (void *)0x8);
               }
            }
         }
         // process remaining files
         for (fnli.toFirst(); (fn = fnli.current()); ++fnli) {
            FileNameIterator fni(*fn);
            FileDef *fd;
            for (; (fd = fni.current()); ++fni) {
               if (!g_processedFiles.find(fd->absoluteFilePath())) { // not yet processed
                  QStringList filesInSameTu;
                  fd->startParsing();
                  if (fd->generateSourceFile()) { // sources need to be shown in the output
                     msg("Generating code for file %s...\n", fd->docName().data());
                     fd->writeSource(*Doxy_Globals::g_outputList, false, filesInSameTu);

                  } else if (!fd->isReference() && Doxygen::parseSourcesNeeded)
                     // we needed to parse the sources even if we do not show them
                  {
                     msg("Parsing code for file %s...\n", fd->docName().data());
                     fd->parseSource(false, filesInSameTu);
                  }
                  fd->finishParsing();
               }
            }
         }
      } else
#endif
      {
         FileNameListIterator fnli(*Doxygen::inputNameList);
         FileName *fn;
         for (; (fn = fnli.current()); ++fnli) {
            FileNameIterator fni(*fn);
            FileDef *fd;
            for (; (fd = fni.current()); ++fni) {
               QStringList filesInSameTu;
               fd->startParsing();
               if (fd->generateSourceFile()) { // sources need to be shown in the output
                  msg("Generating code for file %s...\n", fd->docName().data());
                  fd->writeSource(*Doxy_Globals::g_outputList, false, filesInSameTu);

               } else if (!fd->isReference() && Doxygen::parseSourcesNeeded)
                  // we needed to parse the sources even if we do not show them
               {
                  msg("Parsing code for file %s...\n", fd->docName().data());
                  fd->parseSource(false, filesInSameTu);
               }
               fd->finishParsing();
            }
         }
      }
   }
}

void Doxy_Work::generateFileDocs()
{
   if (documentedHtmlFiles == 0) {
      return;
   }

   if (Doxygen::inputNameList->count() > 0) {
      FileNameListIterator fnli(*Doxygen::inputNameList);
      FileName *fn;
      for (fnli.toFirst(); (fn = fnli.current()); ++fnli) {
         FileNameIterator fni(*fn);
         FileDef *fd;
         for (fni.toFirst(); (fd = fni.current()); ++fni) {
            bool doc = fd->isLinkableInProject();
            if (doc) {
               msg("Generating docs for file %s...\n", fd->docName().data());
               fd->writeDocumentation(*Doxy_Globals::g_outputList);
            }
         }
      }
   }
}

void Doxy_Work::addSourceReferences()
{
   // add source references for class definitions
   for (auto cd : *Doxygen::classSDict) {
      FileDef *fd = cd->getBodyDef();

      if (fd && cd->isLinkableInProject() && cd->getStartBodyLine() != -1) {
         fd->addSourceRef(cd->getStartBodyLine(), cd, nullptr);
      }
   }

   // add source references for namespace definitions
   for (auto nd : *Doxygen::namespaceSDict) {
      FileDef *fd = nd->getBodyDef();

      if (fd && nd->isLinkableInProject() && nd->getStartBodyLine() != -1) {
         fd->addSourceRef(nd->getStartBodyLine(), nd, nullptr);
      }
   }

   // add source references for member names
   for (auto mn: *Doxygen::memberNameSDict) {

      for (auto md: *mn) {
         FileDef *fd = md->getBodyDef();

         if (fd && md->getStartBodyLine() != -1 && md->isLinkableInProject() &&
               (fd->generateSourceFile() || Doxygen::parseSourcesNeeded) ) {
            fd->addSourceRef(md->getStartBodyLine(), md->getOuterScope(), md);
         }
      }
   }

   MemberNameSDict::Iterator fnli(*Doxygen::functionNameSDict);
   for (fnli.toFirst(); (mn = fnli.current()); ++fnli) {
      MemberNameIterator mni(*mn);
      MemberDef *md = 0;
      for (mni.toFirst(); (md = mni.current()); ++mni) {
         FileDef *fd = md->getBodyDef();
         //printf("member %s body=[%d,%d] fd=%p link=%d parseSources=%d\n",
         //    md->name().data(),
         //    md->getStartBodyLine(),md->getEndBodyLine(),fd,
         //    md->isLinkableInProject(),
         //    Doxygen::parseSourcesNeeded);
         if (fd &&
               md->getStartBodyLine() != -1 &&
               md->isLinkableInProject() &&
               (fd->generateSourceFile() || Doxygen::parseSourcesNeeded)
            ) {
            //printf("Found member `%s' in file `%s' at line `%d' def=%s\n",
            //    md->name().data(),fd->name().data(),md->getStartBodyLine(),md->getOuterScope()->name().data());
            fd->addSourceRef(md->getStartBodyLine(), md->getOuterScope(), md);
         }
      }
   }
}

void Doxy_Work::sortMemberLists()
{
   // sort class member lists
   ClassSDict::Iterator cli(*Doxygen::classSDict);
   ClassDef *cd = 0;
   for (cli.toFirst(); (cd = cli.current()); ++cli) {
      cd->sortMemberLists();
   }

   // sort namespace member lists
   NamespaceSDict::Iterator nli(*Doxygen::namespaceSDict);
   NamespaceDef *nd = 0;
   for (nli.toFirst(); (nd = nli.current()); ++nli) {
      nd->sortMemberLists();
   }

   // sort file member lists
   FileNameListIterator fnli(*Doxygen::inputNameList);
   FileName *fn;
   for (; (fn = fnli.current()); ++fnli) {
      FileNameIterator fni(*fn);
      FileDef *fd;
      for (; (fd = fni.current()); ++fni) {
         fd->sortMemberLists();
      }
   }

   // sort group member lists
   GroupSDict::Iterator gli(*Doxygen::groupSDict);
   GroupDef *gd;
   for (gli.toFirst(); (gd = gli.current()); ++gli) {
      gd->sortMemberLists();
   }
}

// generate the documentation of all classes
void Doxy_Work::generateClassList(ClassSDict &classSDict)
{
   ClassSDict::Iterator cli(classSDict);
   for ( ; cli.current() ; ++cli ) {
      ClassDef *cd = cli.current();

      //printf("cd=%s getOuterScope=%p global=%p\n",cd->name().data(),cd->getOuterScope(),Doxygen::globalScope);
      if (cd &&
            (cd->getOuterScope() == 0 || // <-- should not happen, but can if we read an old tag file
             cd->getOuterScope() == Doxygen::globalScope // only look at global classes
            ) && !cd->isHidden() && !cd->isEmbeddedInOuterScope()
         ) {
         // skip external references, anonymous compounds and
         // template instances
         if ( cd->isLinkableInProject() && cd->templateMaster() == 0) {
            msg("Generating docs for compound %s...\n", cd->name().data());

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
   generateClassList(*Doxygen::classSDict);
   generateClassList(*Doxygen::hiddenClasses);
}

void Doxy_Work::inheritDocumentation()
{
   MemberNameSDict::Iterator mnli(*Doxygen::memberNameSDict);
   MemberName *mn;
   //int count=0;
   for (; (mn = mnli.current()); ++mnli) {
      MemberNameIterator mni(*mn);
      MemberDef *md;
      for (; (md = mni.current()); ++mni) {
         //printf("%04d Member `%s'\n",count++,md->name().data());
         if (md->documentation().isEmpty() && md->briefDescription().isEmpty()) {
            // no documentation yet
            MemberDef *bmd = md->reimplements();
            while (bmd && bmd->documentation().isEmpty() &&
                   bmd->briefDescription().isEmpty()
                  ) {
               // search up the inheritance tree for a documentation member
               //printf("bmd=%s class=%s\n",bmd->name().data(),bmd->getClassDef()->name().data());
               bmd = bmd->reimplements();
            }
            if (bmd) { // copy the documentation from the reimplemented member
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
   FileNameListIterator fnli(*Doxygen::inputNameList);
   FileName *fn;
   for (fnli.toFirst(); (fn = fnli.current()); ++fnli) {
      FileNameIterator fni(*fn);
      FileDef *fd;
      for (fni.toFirst(); (fd = fni.current()); ++fni) {
         fd->visited = false;
      }
   }
   for (fnli.toFirst(); (fn = fnli.current()); ++fnli) {
      FileNameIterator fni(*fn);
      FileDef *fd;
      for (fni.toFirst(); (fd = fni.current()); ++fni) {
         fd->combineUsingRelations();
      }
   }

   // for each namespace
   NamespaceSDict::Iterator nli(*Doxygen::namespaceSDict);
   NamespaceDef *nd;
   for (nli.toFirst() ; (nd = nli.current()) ; ++nli ) {
      nd->visited = false;
   }
   for (nli.toFirst() ; (nd = nli.current()) ; ++nli ) {
      nd->combineUsingRelations();
   }
}

void Doxy_Work::addMembersToMemberGroup()
{
   // for each class
   ClassSDict::Iterator cli(*Doxygen::classSDict);
   ClassDef *cd;
   for ( ; (cd = cli.current()) ; ++cli ) {
      cd->addMembersToMemberGroup();
   }
   // for each file
   FileNameListIterator fnli(*Doxygen::inputNameList);
   FileName *fn;
   for (fnli.toFirst(); (fn = fnli.current()); ++fnli) {
      FileNameIterator fni(*fn);
      FileDef *fd;
      for (fni.toFirst(); (fd = fni.current()); ++fni) {
         fd->addMembersToMemberGroup();
      }
   }
   // for each namespace
   NamespaceSDict::Iterator nli(*Doxygen::namespaceSDict);
   NamespaceDef *nd;
   for ( ; (nd = nli.current()) ; ++nli ) {
      nd->addMembersToMemberGroup();
   }
   // for each group
   GroupSDict::Iterator gli(*Doxygen::groupSDict);
   GroupDef *gd;
   for (gli.toFirst(); (gd = gli.current()); ++gli) {
      gd->addMembersToMemberGroup();
   }
}

void Doxy_Work::distributeMemberGroupDocumentation()
{
   // for each class
   ClassSDict::Iterator cli(*Doxygen::classSDict);
   ClassDef *cd;
   for ( ; (cd = cli.current()) ; ++cli ) {
      cd->distributeMemberGroupDocumentation();
   }
   // for each file
   FileNameListIterator fnli(*Doxygen::inputNameList);
   FileName *fn;
   for (fnli.toFirst(); (fn = fnli.current()); ++fnli) {
      FileNameIterator fni(*fn);
      FileDef *fd;
      for (fni.toFirst(); (fd = fni.current()); ++fni) {
         fd->distributeMemberGroupDocumentation();
      }
   }
   // for each namespace
   NamespaceSDict::Iterator nli(*Doxygen::namespaceSDict);
   NamespaceDef *nd;
   for ( ; (nd = nli.current()) ; ++nli ) {
      nd->distributeMemberGroupDocumentation();
   }
   // for each group
   GroupSDict::Iterator gli(*Doxygen::groupSDict);
   GroupDef *gd;
   for (gli.toFirst(); (gd = gli.current()); ++gli) {
      gd->distributeMemberGroupDocumentation();
   }
}

void Doxy_Work::findSectionsInDocumentation()
{
   // for each class
   ClassSDict::Iterator cli(*Doxygen::classSDict);
   ClassDef *cd;
   for ( ; (cd = cli.current()) ; ++cli ) {
      cd->findSectionsInDocumentation();
   }
   // for each file
   FileNameListIterator fnli(*Doxygen::inputNameList);
   FileName *fn;
   for (fnli.toFirst(); (fn = fnli.current()); ++fnli) {
      FileNameIterator fni(*fn);
      FileDef *fd;
      for (fni.toFirst(); (fd = fni.current()); ++fni) {
         fd->findSectionsInDocumentation();
      }
   }
   // for each namespace
   NamespaceSDict::Iterator nli(*Doxygen::namespaceSDict);
   NamespaceDef *nd;
   for ( ; (nd = nli.current()) ; ++nli ) {
      nd->findSectionsInDocumentation();
   }
   // for each group
   GroupSDict::Iterator gli(*Doxygen::groupSDict);
   GroupDef *gd;
   for (gli.toFirst(); (gd = gli.current()); ++gli) {
      gd->findSectionsInDocumentation();
   }
   // for each page
   PageSDict::Iterator pdi(*Doxygen::pageSDict);
   PageDef *pd = 0;
   for (pdi.toFirst(); (pd = pdi.current()); ++pdi) {
      pd->findSectionsInDocumentation();
   }
   if (Doxygen::mainPage) {
      Doxygen::mainPage->findSectionsInDocumentation();
   }
}

void Doxy_Work::flushCachedTemplateRelations()
{
   // remove all references to classes from the cache
   // as there can be new template instances in the inheritance path
   // to this class. Optimization: only remove those classes that
   // have inheritance instances as direct or indirect sub classes.

   QCacheIterator<QString, LookupInfo> ci(*Doxygen::lookupCache);

   LookupInfo *li = 0;

   for (ci.toFirst(); (li = ci.current()); ++ci) {
      if (li->classDef) {
         Doxygen::lookupCache->remove(ci.currentKey());
      }
   }

   // remove all cached typedef resolutions whose target is a
   // template class as this may now be a template instance

   MemberNameSDict::Iterator fnli(*Doxygen::functionNameSDict);
   MemberName *fn;

   for (; (fn = fnli.current()); ++fnli) { // for each global function name
      MemberNameIterator fni(*fn);
      MemberDef *fmd;
      for (; (fmd = fni.current()); ++fni) { // for each function with that name
         if (fmd->isTypedefValCached()) {
            ClassDef *cd = fmd->getCachedTypedefVal();
            if (cd->isTemplate()) {
               fmd->invalidateTypedefValCache();
            }
         }
      }
   }
   MemberNameSDict::Iterator mnli(*Doxygen::memberNameSDict);
   for (; (fn = mnli.current()); ++mnli) { // for each class method name
      MemberNameIterator mni(*fn);
      MemberDef *fmd;
      for (; (fmd = mni.current()); ++mni) { // for each function with that name
         if (fmd->isTypedefValCached()) {
            ClassDef *cd = fmd->getCachedTypedefVal();
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
   //
   QCacheIterator<QString, LookupInfo> ci(*Doxygen::lookupCache);

   LookupInfo *li = 0;
   for (ci.toFirst(); (li = ci.current()); ++ci) {
      if (li->classDef == 0 && li->typeDef == 0) {
         Doxygen::lookupCache->remove(ci.currentKey());
      }
   }

   MemberNameSDict::Iterator fnli(*Doxygen::functionNameSDict);
   MemberName *fn;
   for (; (fn = fnli.current()); ++fnli) { // for each global function name
      MemberNameIterator fni(*fn);
      MemberDef *fmd;
      for (; (fmd = fni.current()); ++fni) { // for each function with that name
         fmd->invalidateCachedArgumentTypes();
      }
   }
   MemberNameSDict::Iterator mnli(*Doxygen::memberNameSDict);
   for (; (fn = mnli.current()); ++mnli) { // for each class method name
      MemberNameIterator mni(*fn);
      MemberDef *fmd;
      for (; (fmd = mni.current()); ++mni) { // for each function with that name
         fmd->invalidateCachedArgumentTypes();
      }
   }

}

void Doxy_Work::findDefineDocumentation(EntryNav *rootNav)
{
   if ((rootNav->section() == Entry::DEFINEDOC_SEC ||
         rootNav->section() == Entry::DEFINE_SEC) && !rootNav->name().isEmpty()) {

      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      //printf("found define `%s' `%s' brief=`%s' doc=`%s'\n",
      //       root->name.data(),root->args.data(),root->brief.data(),root->doc.data());

      if (rootNav->tagInfo() && !root->name.isEmpty()) { // define read from a tag file
         MemberDef *md = new MemberDef("<tagfile>", 1, 1,
                                       "#define", root->name, root->args, 0,
                                       Public, Normal, false, Member, MemberType_Define, 0, 0);
         md->setTagInfo(rootNav->tagInfo());
         md->setLanguage(root->lang);
         //printf("Searching for `%s' fd=%p\n",filePathName.data(),fd);
         md->setFileDef(rootNav->parent()->fileDef());
         //printf("Adding member=%s\n",md->name().data());
         MemberName *mn;
         if ((mn = Doxygen::functionNameSDict->find(root->name))) {
            mn->append(md);
         } else {
            mn = new MemberName(root->name);
            mn->append(md);
            Doxygen::functionNameSDict->append(root->name, mn);
         }
      }
      MemberName *mn = Doxygen::functionNameSDict->find(root->name);
      if (mn) {
         MemberNameIterator mni(*mn);
         MemberDef *md;
         int count = 0;
         for (; (md = mni.current()); ++mni) {
            if (md->memberType() == MemberType_Define) {
               count++;
            }
         }
         if (count == 1) {
            for (mni.toFirst(); (md = mni.current()); ++mni) {
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
         } else if (count > 1 &&
                    (!root->doc.isEmpty() ||
                     !root->brief.isEmpty() ||
                     root->bodyLine != -1
                    )
                   )
            // multiple defines don't know where to add docs
            // but maybe they are in different files together with their documentation
         {
            for (mni.toFirst(); (md = mni.current()); ++mni) {
               if (md->memberType() == MemberType_Define) {
                  FileDef *fd = md->getFileDef();
                  if (fd && fd->absoluteFilePath() == root->fileName)
                     // doc and define in the same file assume they belong together.
                  {
#if 0
                     if (md->documentation().isEmpty())
#endif
                     {
                        md->setDocumentation(root->doc, root->docFile, root->docLine);
                        md->setDocsForDefinition(!root->proto);
                     }
#if 0
                     if (md->briefDescription().isEmpty())
#endif
                     {
                        md->setBriefDescription(root->brief, root->briefFile, root->briefLine);
                     }
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
            //warn("define %s found in the following files:\n",root->name.data());
            //warn("Cannot determine where to add the documentation found "
            //     "at line %d of file %s. \n",
            //     root->startLine,root->fileName.data());
         }
      } else if (!root->doc.isEmpty() || !root->brief.isEmpty()) { // define not found
         static bool preEnabled = Config_getBool("ENABLE_PREPROCESSING");
         if (preEnabled) {
            warn(root->fileName, root->startLine,
                 "documentation for unknown define %s found.\n",
                 root->name.data()
                );
         } else {
            warn(root->fileName, root->startLine,
                 "found documented #define but ignoring it because "
                 "ENABLE_PREPROCESSING is NO.\n",
                 root->name.data()
                );
         }
      }

      rootNav->releaseEntry();
   }
   RECURSE_ENTRYTREE(findDefineDocumentation, rootNav);
}

void Doxy_Work::findDirDocumentation(EntryNav *rootNav)
{
   if (rootNav->section() == Entry::DIRDOC_SEC) {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      QByteArray normalizedName = root->name;
      normalizedName = substitute(normalizedName, "\\", "/");

      //printf("root->docFile=%s normalizedName=%s\n",
      //    root->docFile.data(),normalizedName.data());

      if (root->docFile == normalizedName) { // current dir?
         int lastSlashPos = normalizedName.lastIndexOf('/');
         if (lastSlashPos != -1) { // strip file name
            normalizedName = normalizedName.left(lastSlashPos);
         }
      }

      if (normalizedName.at(normalizedName.length() - 1) != '/') {
         normalizedName += '/';
      }

      DirDef *dir, *matchingDir = 0;
      StringMap<QSharedPointer<DirDef>>::Iterator sdi(*Doxygen::directories);

      for (sdi.toFirst(); (dir = sdi.current()); ++sdi) {
         //printf("Dir: %s<->%s\n",dir->name().data(),normalizedName.data());

         if (dir->name().right(normalizedName.length()) == normalizedName) {
            if (matchingDir) {

               warn(root->fileName, root->startLine,
                    "\\dir command matches multiple directories.\n"
                    "  Applying the command for directory %s\n"
                    "  Ignoring the command for directory %s\n",
                    matchingDir->name().data(), dir->name().data() );

            } else {
               matchingDir = dir;
            }
         }
      }

      if (matchingDir) {
         //printf("Match for with dir %s\n",matchingDir->name().data());
         matchingDir->setBriefDescription(root->brief, root->briefFile, root->briefLine);
         matchingDir->setDocumentation(root->doc, root->docFile, root->docLine);
         matchingDir->setRefItems(root->sli);
         addDirToGroups(root, matchingDir);

      } else {
         warn(root->fileName, root->startLine, "No matching "
              "directory found for command \\dir %s\n", normalizedName.data());
      }

      rootNav->releaseEntry();
   }

   RECURSE_ENTRYTREE(findDirDocumentation, rootNav);
}

// create a (sorted) list of separate documentation pages
void Doxy_Work::buildPageList(EntryNav *rootNav)
{
   if (rootNav->section() == Entry::PAGEDOC_SEC) {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      if (!root->name.isEmpty()) {
         addRelatedPage(rootNav);
      }

      rootNav->releaseEntry();
   } else if (rootNav->section() == Entry::MAINPAGEDOC_SEC) {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      QByteArray title = root->args.trimmed();
      if (title.isEmpty()) {
         title = theTranslator->trMainPage();
      }
      //QByteArray name = Config_getBool("GENERATE_TREEVIEW")?"main":"index";
      QByteArray name = "index";
      addRefItem(root->sli,
                 name,
                 "page",
                 name,
                 title,
                 0, 0
                );

      rootNav->releaseEntry();
   }
   RECURSE_ENTRYTREE(buildPageList, rootNav);
}

// search for the main page defined in this project
void Doxy_Work::findMainPage(EntryNav *rootNav)
{
   if (rootNav->section() == Entry::MAINPAGEDOC_SEC) {
      rootNav->loadEntry(Doxy_Globals::g_storage);

      if (Doxygen::mainPage == 0 && rootNav->tagInfo() == 0) {
         Entry *root = rootNav->entry();
         //printf("Found main page! \n======\n%s\n=======\n",root->doc.data());
         QByteArray title = root->args.trimmed();
         //QByteArray indexName=Config_getBool("GENERATE_TREEVIEW")?"main":"index";
         QByteArray indexName = "index";
         Doxygen::mainPage = new PageDef(root->docFile, root->docLine,
                                         indexName, root->brief + root->doc + root->inbodyDocs, title);
         //setFileNameForSections(root->anchors,"index",Doxygen::mainPage);
         Doxygen::mainPage->setBriefDescription(root->brief, root->briefFile, root->briefLine);
         Doxygen::mainPage->setFileName(indexName, true);
         Doxygen::mainPage->setShowToc(root->stat);
         addPageToContext(Doxygen::mainPage, rootNav);

         SectionInfo *si = Doxygen::sectionDict->find(Doxygen::mainPage->name());
         if (si) {
            if (si->lineNr != -1) {
               warn(root->fileName, root->startLine, "multiple use of section label '%s' for main page, (first occurrence: %s, line %d)",
                    Doxygen::mainPage->name().data(), si->fileName.data(), si->lineNr);
            } else {
               warn(root->fileName, root->startLine, "multiple use of section label '%s' for main page, (first occurrence: %s)", Doxygen::mainPage->name().data(),
                    si->fileName.data());
            }
         } else {
            // a page name is a label as well! but should no be double either
            si = new SectionInfo(
               indexName, root->startLine,
               Doxygen::mainPage->name(),
               Doxygen::mainPage->title(),
               SectionInfo::Page,
               0); // level 0
            Doxygen::sectionDict->append(indexName, si);
            Doxygen::mainPage->addSectionsToDefinition(root->anchors);
         }
      } else if (rootNav->tagInfo() == 0) {
         Entry *root = rootNav->entry();
         warn(root->fileName, root->startLine,
              "found more than one \\mainpage comment block! Skipping this "
              "block."
             );
      }

      rootNav->releaseEntry();
   }
   RECURSE_ENTRYTREE(findMainPage, rootNav);
}

// search for the main page imported via tag files and add only the section labels
void Doxy_Work::findMainPageTagFiles(EntryNav *rootNav)
{
   if (rootNav->section() == Entry::MAINPAGEDOC_SEC) {
      rootNav->loadEntry(Doxy_Globals::g_storage);

      if (Doxygen::mainPage && rootNav->tagInfo()) {
         Entry *root = rootNav->entry();
         Doxygen::mainPage->addSectionsToDefinition(root->anchors);
      }
   }
   RECURSE_ENTRYTREE(findMainPageTagFiles, rootNav);
}

void Doxy_Work::computePageRelations(EntryNav *rootNav)
{
   if ((rootNav->section() == Entry::PAGEDOC_SEC ||
         rootNav->section() == Entry::MAINPAGEDOC_SEC
       )
         && !rootNav->name().isEmpty()
      ) {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      PageDef *pd = root->section == Entry::PAGEDOC_SEC ?
                    Doxygen::pageSDict->find(root->name) :
                    Doxygen::mainPage;
      if (pd) {
         QListIterator<BaseInfo> bii(*root->extends);
         BaseInfo *bi;
         for (bii.toFirst(); (bi = bii.current()); ++bii) {
            PageDef *subPd = Doxygen::pageSDict->find(bi->name);
            if (subPd) {
               pd->addInnerCompound(subPd);
               //printf("*** Added subpage relation: %s->%s\n",
               //    pd->name().data(),subPd->name().data());
            }
         }
      }

      rootNav->releaseEntry();
   }
   RECURSE_ENTRYTREE(computePageRelations, rootNav);
}

void Doxy_Work::checkPageRelations()
{
   PageSDict::Iterator pdi(*Doxygen::pageSDict);
   PageDef *pd = 0;

   for (pdi.toFirst(); (pd = pdi.current()); ++pdi) {
      Definition *ppd = pd->getOuterScope();

      while (ppd) {
         if (ppd == pd) {
            err("page defined at line %d of file %s with label %s is a subpage "
                "of itself! Please remove this cyclic dependency.\n",
                pd->docLine(), pd->docFile().data(), pd->name().data());
            exit(1);
         }
         ppd = ppd->getOuterScope();
      }
   }
}

void Doxy_Work::resolveUserReferences()
{
   StringMap<QSharedPointer<SectionInfo>>::Iterator sdi(*Doxygen::sectionDict);
   SectionInfo *si;

   for (; (si = sdi.current()); ++sdi) {
      //printf("si->label=`%s' si->definition=%s si->fileName=`%s'\n",
      //        si->label.data(),si->definition?si->definition->name().data():"<none>",
      //        si->fileName.data());
      PageDef *pd = 0;

      // hack: the items of a todo/test/bug/deprecated list are all fragments from
      // different files, so the resulting section's all have the wrong file
      // name (not from the todo/test/bug/deprecated list, but from the file in
      // which they are defined). We correct this here by looking at the
      // generated section labels!

      QHashIterator<QString, RefList> rli(*Doxygen::xrefLists);
      RefList *rl;

      for (rli.toFirst(); (rl = rli.current()); ++rli) {
         QByteArray label = "_" + rl->listName(); // "_todo", "_test", ...
         if (si->label.left(label.length()) == label) {
            si->fileName = rl->listName();
            si->generated = true;
            break;
         }
      }

      //printf("start: si->label=%s si->fileName=%s\n",si->label.data(),si->fileName.data());
      if (!si->generated) {
         // if this section is in a page and the page is in a group, then we
         // have to adjust the link file name to point to the group.
         if (!si->fileName.isEmpty() &&
               (pd = Doxygen::pageSDict->find(si->fileName)) &&
               pd->getGroupDef()) {
            si->fileName = pd->getGroupDef()->getOutputFileBase().copy();
         }

         if (si->definition) {
            // TODO: there should be one function in Definition that returns
            // the file to link to, so we can avoid the following tests.
            GroupDef *gd = 0;
            if (si->definition->definitionType() == Definition::TypeMember) {
               gd = ((MemberDef *)si->definition)->getGroupDef();
            }

            if (gd) {
               si->fileName = gd->getOutputFileBase().copy();
            } else {
               //si->fileName=si->definition->getOutputFileBase().copy();
               //printf("Setting si->fileName to %s\n",si->fileName.data());
            }
         }
      }
      //printf("end: si->label=%s si->fileName=%s\n",si->label.data(),si->fileName.data());
   }
}

// generate all separate documentation pages
void Doxy_Work::generatePageDocs()
{
   //printf("documentedPages=%d real=%d\n",documentedPages,Doxygen::pageSDict->count());
   if (documentedPages == 0) {
      return;
   }
   PageSDict::Iterator pdi(*Doxygen::pageSDict);
   PageDef *pd = 0;

   for (pdi.toFirst(); (pd = pdi.current()); ++pdi) {
      if (!pd->getGroupDef() && !pd->isReference()) {
         msg("Generating docs for page %s...\n", pd->name().data());
         Doxygen::insideMainPage = true;
         pd->writeDocumentation(*Doxy_Globals::g_outputList);
         Doxygen::insideMainPage = false;
      }
   }
}

// create a (sorted) list & dictionary of example pages
void Doxy_Work::buildExampleList(EntryNav *rootNav)
{
   if (rootNav->section() == Entry::EXAMPLE_SEC && !rootNav->name().isEmpty()) {
      rootNav->loadEntry(Doxy_Globals::g_storage);
      Entry *root = rootNav->entry();

      if (Doxygen::exampleSDict->find(root->name)) {
         warn(root->fileName, root->startLine, "Example %s was already documented. Ignoring "
              "documentation found here.", root->name.data());

      } else {
         PageDef *pd = new PageDef(root->fileName, root->startLine,
                                   root->name, root->brief + root->doc + root->inbodyDocs, root->args);
         pd->setBriefDescription(root->brief, root->briefFile, root->briefLine);
         pd->setFileName(convertNameToFile(pd->name() + "-example", false, true), false);
         pd->addSectionsToDefinition(root->anchors);
         pd->setLanguage(root->lang);
         //pi->addSections(root->anchors);

         Doxygen::exampleSDict->inSort(root->name, pd);
      }

      rootNav->releaseEntry();
   }
   RECURSE_ENTRYTREE(buildExampleList, rootNav);
}

// prints the Entry tree (for debugging)
void printNavTree(EntryNav *rootNav, int indent)
{
   QByteArray indentStr;
   indentStr.fill(' ', indent);

   msg("%s%s (sec=0x%x)\n", indentStr.isEmpty() ? "" : indentStr.data(),
       rootNav->name().isEmpty() ? "<empty>" : rootNav->name().data(),rootNav->section());

   if (rootNav->children()) {
      for (auto e : *rootNav->children() ) {
         printNavTree(eli.current(), indent + 2);
      }
   }
}

// generate the example documentation
void Doxy_Work::generateExampleDocs()
{
   Doxy_Globals::g_outputList->disable(OutputGenerator::Man);
   PageSDict::Iterator pdi(*Doxygen::exampleSDict);
   PageDef *pd = 0;
   for (pdi.toFirst(); (pd = pdi.current()); ++pdi) {
      msg("Generating docs for example %s...\n", pd->name().data());
      resetCCodeParserState();

      QByteArray n = pd->getOutputFileBase();
      startFile(*Doxy_Globals::g_outputList, n, n, pd->name());
      startTitle(*Doxy_Globals::g_outputList, n);

      Doxy_Globals::g_outputList->docify(pd->name());
      endTitle(*Doxy_Globals::g_outputList, n, 0);

      Doxy_Globals::g_outputList->startContents();
      Doxy_Globals::g_outputList->generateDoc(pd->docFile(),                            // file
                                pd->docLine(),                            // startLine
                                pd,                                       // context
                                0,                                        // memberDef
                                pd->documentation() + "\n\n\\include " + pd->name(),      // docs
                                true,                                     // index words
                                true,                                     // is example
                                pd->name()
                               );
      endFile(*Doxy_Globals::g_outputList); // contains g_outputList->endContents()
   }

   Doxy_Globals::g_outputList->enable(OutputGenerator::Man);
}

// generate module pages
void Doxy_Work::generateGroupDocs()
{
   GroupSDict::Iterator gli(*Doxygen::groupSDict);
   GroupDef *gd;
   for (gli.toFirst(); (gd = gli.current()); ++gli) {
      if (!gd->isReference()) {
         gd->writeDocumentation(*Doxy_Globals::g_outputList);
      }
   }
}

//static void generatePackageDocs()
//{
//  writePackageIndex(*g_outputList);
//
//  if (Doxygen::packageDict.count()>0)
//  {
//    PackageSDict::Iterator pdi(Doxygen::packageDict);
//    PackageDef *pd;
//    for (pdi.toFirst();(pd=pdi.current());++pdi)
//    {
//      pd->writeDocumentation(*g_outputList);
//    }
//  }
//}

// generate module pages
void Doxy_Work::generateNamespaceDocs()
{
   NamespaceSDict::Iterator nli(*Doxygen::namespaceSDict);
   NamespaceDef *nd;

   // for each namespace...
   for (; (nd = nli.current()); ++nli) {

      if (nd->isLinkableInProject()) {
         msg("Generating docs for namespace %s\n", nd->name().data());
         nd->writeDocumentation(*Doxy_Globals::g_outputList);
      }

      // for each class in the namespace...
      ClassSDict::Iterator cli(*nd->getClassSDict());
      ClassDef *cd;
      for ( ; (cd = cli.current()) ; ++cli ) {
         if ( ( cd->isLinkableInProject() &&
                cd->templateMaster() == 0
              ) // skip external references, anonymous compounds and
               // template instances and nested classes
               && !cd->isHidden() && !cd->isEmbeddedInOuterScope()
            ) {
            msg("Generating docs for compound %s...\n", cd->name().data());

            cd->writeDocumentation(*Doxy_Globals::g_outputList);
            cd->writeMemberList(*Doxy_Globals::g_outputList);
         }
         cd->writeDocumentationForInnerClasses(*Doxy_Globals::g_outputList);
      }
   }
}

#if defined(_WIN32)
static QByteArray fixSlashes(QByteArray &s)
{
   QByteArray result;
   uint i;
   for (i = 0; i < s.length(); i++) {
      switch (s.at(i)) {
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
   QByteArray &htmlStyleSheet = Config_getString("HTML_STYLESHEET");
   if (!htmlStyleSheet.isEmpty()) {
      QFileInfo fi(htmlStyleSheet);
      if (!fi.exists()) {
         err("Style sheet '%s' specified by HTML_STYLESHEET does not exist!\n", htmlStyleSheet.data());
         htmlStyleSheet.resize(0); // revert to the default
      } else {
         QByteArray destFileName = Config_getString("HTML_OUTPUT") + "/" + fi.fileName().data();
         copyFile(htmlStyleSheet, destFileName);
      }
   }
   QStringList htmlExtraStyleSheet = Config_getList("HTML_EXTRA_STYLESHEET");
   for (uint i = 0; i < htmlExtraStyleSheet.count(); ++i) {
      QByteArray fileName(htmlExtraStyleSheet.at(i));
      if (!fileName.isEmpty()) {
         QFileInfo fi(fileName);
         if (!fi.exists()) {
            err("Style sheet '%s' specified by HTML_EXTRA_STYLESHEET does not exist!\n", fileName.data());
         } else {
            QByteArray destFileName = Config_getString("HTML_OUTPUT") + "/" + fi.fileName().data();
            copyFile(fileName, destFileName);
         }
      }
   }
}

void Doxy_Work::copyLogo()
{
   QByteArray &projectLogo = Config_getString("PROJECT_LOGO");
   if (!projectLogo.isEmpty()) {
      QFileInfo fi(projectLogo);
      if (!fi.exists()) {
         err("Project logo '%s' specified by PROJECT_LOGO does not exist!\n", projectLogo.data());
         projectLogo.resize(0); // revert to the default
      } else {
         QByteArray destFileName = Config_getString("HTML_OUTPUT") + "/" + fi.fileName().data();
         copyFile(projectLogo, destFileName);
         Doxygen::indexList->addImageFile(fi.fileName().data());
      }
   }
}

void Doxy_Work::copyExtraFiles(const QByteArray &filesOption, const QByteArray &outputOption)
{
   QStringList files = Config_getList(filesOption);
   uint i;
   for (i = 0; i < files.count(); ++i) {
      QByteArray fileName(files.at(i));

      if (!fileName.isEmpty()) {
         QFileInfo fi(fileName);
         if (!fi.exists()) {
            err("Extra file '%s' specified in " + filesOption + " does not exist!\n", fileName.data());
         } else {
            QByteArray destFileName = Config_getString(outputOption) + "/" + fi.fileName().data();
            Doxygen::indexList->addImageFile(fi.fileName().toUtf8());
            copyFile(fileName, destFileName);
         }
      }
   }
}

static ParserInterface *getParserForFile(const char *fn)
{
   QByteArray fileName = fn;
   QByteArray extension;
   int sep = fileName.lastIndexOf('/');
   int ei = fileName.lastIndexOf('.');
   if (ei != -1 && (sep == -1 || ei > sep)) { // matches dir/file.ext but not dir.1/file
      extension = fileName.right(fileName.length() - ei);
   } else {
      extension = ".no_extension";
   }

   return Doxygen::parserManager->getParser(extension);
}

static void parseFile(ParserInterface *parser, Entry *root, EntryNav *rootNav, FileDef *fd, const char *fn,
                      bool sameTu, QStringList &filesInSameTu)
{
#if USE_LIBCLANG
   static bool clangAssistedParsing = Config_getBool("CLANG_ASSISTED_PARSING");
#else
   static bool clangAssistedParsing = false;
#endif

   QByteArray fileName = fn;
   QByteArray extension;
   int ei = fileName.lastIndexOf('.');
   if (ei != -1) {
      extension = fileName.right(fileName.length() - ei);
   } else {
      extension = ".no_extension";
   }

   QFileInfo fi(fileName);
   BufStr preBuf(fi.size() + 4096);

   if (Config_getBool("ENABLE_PREPROCESSING") &&
         parser->needsPreprocessing(extension)) {
      BufStr inBuf(fi.size() + 4096);
      msg("Preprocessing %s...\n", fn);
      readInputFile(fileName, inBuf);
      preprocessFile(fileName, inBuf, preBuf);
   } else { // no preprocessing
      msg("Reading %s...\n", fn);
      readInputFile(fileName, preBuf);
   }

   BufStr convBuf(preBuf.curPos() + 1024);

   // convert multi-line C++ comments to C style comments
   convertCppComments(&preBuf, &convBuf, fileName);

   convBuf.addChar('\0');

   if (clangAssistedParsing && !sameTu) {
      fd->getAllIncludeFilesRecursively(filesInSameTu);
   }

   // use language parse to parse the file
   parser->parseInput(fileName, convBuf.data(), root, sameTu, filesInSameTu);

   // store the Entry tree in a file and create an index to
   // navigate/load entries
   //printf("root->createNavigationIndex for %s\n",fd->name().data());

   root->createNavigationIndex(rootNav, Doxy_Globals::g_storage, fd);
}

//! parse the list of input files
static void parseFiles(Entry *root, EntryNav *rootNav)
{

#if USE_LIBCLANG
   static bool clangAssistedParsing = Config_getBool("CLANG_ASSISTED_PARSING");

   if (clangAssistedParsing) {
      QHash<QString, void *> g_processedFiles;

      // create a dictionary with files to process
      QHash<QString, void *> g_filesToProcess;
      QListIterator<QByteArray> it(g_inputFiles);

      QByteArray *s;

      for (; (s = it.current()); ++it) {
         g_filesToProcess.insert(*s, (void *)0x8);
      }

      // process source files (and their include dependencies)
      for (it.toFirst(); (s = it.current()); ++it) {
         bool ambig;
         FileDef *fd = findFileDef(Doxygen::inputNameDict, s->data(), ambig);
         assert(fd != 0);
         if (fd->isSource() && !fd->isReference()) { // this is a source file
            QStringList filesInSameTu;
            ParserInterface *parser = getParserForFile(s->data());
            parser->startTranslationUnit(s->data());
            parseFile(parser, root, rootNav, fd, s->data(), false, filesInSameTu);
            //printf("  got %d extra files in tu\n",filesInSameTu.count());

            // Now process any include files in the same translation unit
            // first. When libclang is used this is much more efficient.
            char *incFile = filesInSameTu.first();
            while (incFile && g_filesToProcess.find(incFile)) {
               if (qstrcmp(incFile, s->data()) && !g_processedFiles.find(incFile)) {
                  FileDef *ifd = findFileDef(Doxygen::inputNameDict, incFile, ambig);
                  if (ifd && !ifd->isReference()) {
                     QStringList moreFiles;
                     //printf("  Processing %s in same translation unit as %s\n",incFile,s->data());
                     parseFile(parser, root, rootNav, ifd, incFile, true, moreFiles);
                     g_processedFiles.insert(incFile, (void *)0x8);
                  }
               }
               incFile = filesInSameTu.next();
            }
            parser->finishTranslationUnit();
            g_processedFiles.insert(*s, (void *)0x8);
         }
      }
      // process remaining files
      for (it.toFirst(); (s = it.current()); ++it) {
         if (!g_processedFiles.find(*s)) { // not yet processed
            bool ambig;
            QStringList filesInSameTu;
            FileDef *fd = findFileDef(Doxygen::inputNameDict, s->data(), ambig);
            assert(fd != 0);
            ParserInterface *parser = getParserForFile(s->data());
            parser->startTranslationUnit(s->data());
            parseFile(parser, root, rootNav, fd, s->data(), false, filesInSameTu);
            parser->finishTranslationUnit();
            g_processedFiles.insert(*s, (void *)0x8);
         }
      }
   } else // normal pocessing
#endif
   {
      QListIterator<QByteArray> it(g_inputFiles);
      QByteArray *s;

      for (; (s = it.current()); ++it) {
         bool ambig;
         QStringList filesInSameTu;
         FileDef *fd = findFileDef(Doxygen::inputNameDict, s->data(), ambig);
         assert(fd != 0);
         ParserInterface *parser = getParserForFile(s->data());
         parser->startTranslationUnit(s->data());
         parseFile(parser, root, rootNav, fd, s->data(), false, filesInSameTu);
      }
   }
}

// resolves a path that may include symlinks, if a recursive symlink is
// found an empty string is returned.
static QByteArray resolveSymlink(QByteArray path)
{
   int sepPos = 0;
   int oldPos = 0;

   QFileInfo fi;

   QHash<QString, void *> nonSymlinks;
   QHash<QString, void *> known;

   QByteArray result = path;
   QByteArray oldPrefix = "/";

   do {
#ifdef WIN32
      // UNC path, skip server and share name

      if (sepPos == 0 && (result.left(2) == "//" || result.left(2) == "\\\\")) {
         sepPos = result.find('/', 2);
      }

      if (sepPos != -1) {
         sepPos = result.indexOf('/', sepPos + 1);
      }
#else
      sepPos = result.indexOf('/', sepPos + 1);
#endif

      QByteArray prefix = sepPos == -1 ? result : result.left(sepPos);

      if (! nonSymlinks.contains(prefix)) {
         fi.setFile(prefix);

         if (fi.isSymLink()) {
            QString target = fi.readLink();
            bool isRelative = QFileInfo(target).isRelative();

            if (isRelative) {
               target = QDir::cleanDirPath(oldPrefix + "/" + target.data());
            }

            if (sepPos != -1) {
               if (fi.isDir() && target.length() > 0 && target.at(target.length() - 1) != '/') {
                  target += '/';
               }
               target += result.mid(sepPos);
            }

            result = QDir::cleanDirPath(target).data();
            sepPos = 0;

            if (known.find(result)) {
               return QByteArray();   // recursive symlink!
            }

            known.insert(result, (void *)0x8);
            if (isRelative) {
               sepPos = oldPos;
            } else { // link to absolute path
               sepPos = 0;
               oldPrefix = "/";
            }
         } else {
            nonSymlinks.insert(prefix, (void *)0x8);
            oldPrefix = prefix;
         }
         oldPos = sepPos;
      }

   } while (sepPos != -1);

   return QDir::cleanDirPath(result).data();
}

static void exitDoxygen()
{
   if (!g_successfulRun) {
      // premature exit
      QDir thisDir;
      msg("Exiting...\n");

      if (!Doxygen::entryDBFileName.isEmpty()) {
         thisDir.remove(Doxygen::entryDBFileName);
      }

      if (!Doxygen::objDBFileName.isEmpty()) {
         thisDir.remove(Doxygen::objDBFileName);
      }
   }
}

static void readTagFile(Entry *root, const char *tl)
{
   QByteArray tagLine = tl;
   QByteArray fileName;
   QByteArray destName;

   int eqPos = tagLine.indexOf('=');

   if (eqPos != -1) { // tag command contains a destination
      fileName = tagLine.left(eqPos).trimmed();
      destName = tagLine.right(tagLine.length() - eqPos - 1).trimmed();
      QFileInfo fi(fileName);

      Doxygen::tagDestinationDict.insert(fi.absoluteFilePath().toUtf8(), new QByteArray(destName));

   } else {
      fileName = tagLine;

   }

   QFileInfo fi(fileName);
   if (! fi.exists() || !fi.isFile()) {
      err("Tag file `%s' does not exist or is not a file. Skipping it...\n",
          fileName.data());
      return;
   }

   if (!destName.isEmpty()) {
      msg("Reading tag file `%s', location `%s'...\n", fileName.data(), destName.data());
   } else {
      msg("Reading tag file `%s'...\n", fileName.data());
   }

   parseTagFile(root, fi.absoluteFilePath().toUtf8());
}

QByteArray Doxy_Work::createOutputDirectory(const QByteArray &baseDirName, const char *formatDirOption, const char *defaultDirName)
{
   // Note the & on the next line, we modify the formatDirOption!
   QByteArray &formatDirName = Config_getString(formatDirOption);

   if (formatDirName.isEmpty()) {
      formatDirName = baseDirName + defaultDirName;
   } else if (formatDirName[0] != '/' && (formatDirName.length() == 1 || formatDirName[1] != ':')) {
      formatDirName.prepend(baseDirName + '/');
   }

   QDir formatDir(formatDirName);
   if (!formatDir.exists() && !formatDir.mkdir(formatDirName)) {
      err("Could not create output directory %s\n", formatDirName.data());
      cleanUpDoxygen();
      exit(1);
   }
   return formatDirName;
}

// ** end of statics


// Read all files matching at least one pattern in `patList' in the
// directory represented by `fi'.
// The directory is read iff the recusiveFlag is set.
// The contents of all files is append to the input string

int readDir(QFileInfo *fi, SortedList<FileName *> *fnList, FileNameDict *fnDict, StringDict  *exclDict,
            QStringList *patList, QStringList *exclPatList, QList<QByteArray> *resultList, StringDict *resultDict,
            bool errorIfNotExist, bool recursive, QHash<QString, void *> *killDict, QHash<QString, void *> *paths)
{
   QByteArray dirName = fi->absoluteFilePath().toUtf8();

   if (paths && ! paths->contains(dirName)) {
      paths->insert(dirName, (void *)0x8);
   }

   if (fi->isSymLink()) {
      dirName = resolveSymlink(dirName.data());
      if (dirName.isEmpty()) {
         return 0;   // recusive symlink
      }
      if (g_pathsVisited.find(dirName)) {
         return 0;   // already visited path
      }
      g_pathsVisited.insert(dirName, (void *)0x8);
   }

   QDir dir(dirName);
   dir.setFilter( QDir::Files | QDir::Dirs | QDir::Hidden );

   int totalSize = 0;
   msg("Searching for files in directory %s\n", fi->absoluteFilePath().data());

   const QFileInfoList *list = dir.entryInfoList();

   if (list) {

      for (auto cfi : *list) {

         if (exclDict == 0 || exclDict->find(cfi->absoluteFilePath().toUtf8()) == 0) {
            // file should not be excluded

            if (! cfi.exists() || ! cfi.isReadable()) {
               if (errorIfNotExist) {
                  warn_uncond("source %s is not a readable file or directory... skipping.\n", cfi->absoluteFilePath().data());
               }

            } else if (cfi.isFile() && (! Config_getBool("EXCLUDE_SYMLINKS") || !cfi.isSymLink()) &&
                       (patList == 0 || patternMatch(cfi, patList)) && !patternMatch(cfi, exclPatList) &&
                       (killDict == 0 || ! killDict->contains(cfi.absoluteFilePath().toUtf8())) ) {

               totalSize += cfi->size() + cfi.absoluteFilePath().length() + 4;
               QByteArray name = cfi.fileName().toUtf8();

               if (fnDict) {
                  FileDef  *fd = new FileDef(cfi.dirPath().toUtf8() + "/", name);
                  FileName *fn = 0;

                  if (!name.isEmpty() && (fn = (*fnDict)[name])) {
                     fn->append(fd);

                  } else {
                     fn = new FileName(cfi->absoluteFilePath().toUtf8(), name);
                     fn->append(fd);

                     if (fnList) {
                        fnList->inSort(fn);
                     }
                     fnDict->insert(name, fn);
                  }
               }

               QByteArray *rs = 0;
               if (resultList || resultDict) {
                  rs = new QByteArray(cfi.absoluteFilePath().toUtf8());
               }
               if (resultList) {
                  resultList->append(rs);
               }
               if (resultDict) {
                  resultDict->insert(cfi.absoluteFilePath().toUtf8(), rs);
               }
               if (killDict) {
                  killDict->insert(cfi.absoluteFilePath().toUtf8(), (void *)0x8);
               }

            } else if (recursive &&
                       (!Config_getBool("EXCLUDE_SYMLINKS") || !cfi.isSymLink()) &&
                       cfi.isDir() && !patternMatch(*cfi, exclPatList) && cfi.fileName().at(0) != '.') {

               cfi.->setFile(cfi.absoluteFilePath());

               totalSize += readDir(cfi, fnList, fnDict, exclDict, patList, exclPatList, resultList,
                                    resultDict, errorIfNotExist, recursive, killDict, paths);
            }
         }

      }
   }
   return totalSize;
}

// read a file or all files in a directory and append their contents to the
// input string. The names of the files are appended to the `fiList' list.

int readFileOrDirectory(const QString &s, SortedList<FileName *> *fnList, FileNameDict *fnDict, StringDict *exclDict,
                        QStringList *patList, QStringList *exclPatList, QStringList *resultList, StringDict *resultDict,
                        bool recursive, bool errorIfNotExist, QHash<QString, void *> *killDict, QHash<QString, void *> *paths)
{
   // strip trailing slashes
   if (s == 0) {
      return 0;
   }

   QByteArray fs = s;
   char lc = fs.at(fs.length() - 1);

   if (lc == '/' || lc == '\\') {
      fs = fs.left(fs.length() - 1);
   }

   QFileInfo fi(fs);
   int totalSize = 0;

   {
      if (exclDict == 0 || ! exclDict->contains(fi.absoluteFilePath().toUtf8())) {

         if (! fi.exists() || ! fi.isReadable()) {
            if (errorIfNotExist) {
               warn_uncond("source %s is not a readable file or directory... skipping.\n", s);
            }

         } else if (!Config_getBool("EXCLUDE_SYMLINKS") || !fi.isSymLink()) {

            if (fi.isFile()) {
               QByteArray dirPath  = fi.absolutePath().toUtf8();
               QByteArray filePath = fi.absoluteFilePath().toUtf8();

               if (paths && paths->find(dirPath)) {
                  paths->insert(dirPath, (void *)0x8);
               }

               //printf("killDict->find(%s)\n",fi.absoluteFilePath().data());
               if (killDict == 0 || ! killDict->contains(filePath)) {
                  totalSize += fi.size() + fi.absoluteFilePath().length() + 4;

                  QByteArray name = fi.fileName().toUtf8();

                  if (fnDict) {
                     FileDef  *fd = new FileDef(dirPath + "/", name);
                     FileName *fn = 0;

                     if (!name.isEmpty() && (fn = (*fnDict)[name])) {
                        fn->append(fd);

                     } else {
                        fn = new FileName(filePath, name);
                        fn->append(fd);

                        if (fnList) {
                           fnList->inSort(fn);
                        }

                        fnDict->insert(name, fn);
                     }
                  }

                  QByteArray *rs = 0;
                  if (resultList || resultDict) {
                     rs = new QByteArray(filePath);
                     if (resultList) {
                        resultList->append(rs);
                     }
                     if (resultDict) {
                        resultDict->insert(filePath, rs);
                     }
                  }

                  if (killDict) {
                     killDict->insert(fi.absoluteFilePath().toUtf8(), (void *)0x8);
                  }
               }
            } else if (fi.isDir()) { // readable dir
               totalSize += readDir(&fi, fnList, fnDict, exclDict, patList,
                                    exclPatList, resultList, resultDict, errorIfNotExist,
                                    recursive, killDict, paths);
            }
         }
      }
   }
   return totalSize;
}

void readFormulaRepository()
{
   QFile f(Config_getString("HTML_OUTPUT") + "/formula.repository");
   if (f.open(QIODevice::ReadOnly)) { // open repository
      msg("Reading formula repository...\n");
      QTextStream t(&f);
      QByteArray line;

      while (!t.eof()) {
         line = t.readLine().toUtf8();
         int se = line.find(':'); // find name and text separator.
         if (se == -1) {
            warn_uncond("formula.repository is corrupted!\n");
            break;
         } else {
            QByteArray formName = line.left(se);
            QByteArray formText = line.right(line.length() - se - 1);

            Formula *f = new Formula(formText);

            Doxygen::formulaList->append(f);
            Doxygen::formulaDict->insert(formText, f);
            Doxygen::formulaNameDict->insert(formName, f);
         }
      }
   }
}

static void Doxy_Work::expandAliases()
{
   QHashIterator<QString, QByteArray> adi(Doxygen::aliasDict);
   QByteArray *s;

   for (adi.toFirst(); (s = adi.current()); ++adi) {
      *s = expandAlias(adi.currentKey(), *s);
   }
}

void Doxy_Work::escapeAliases()
{
   QHashIterator<QString,QByteArray> adi(Doxygen::aliasDict);
   QByteArray *s;

   for (adi.toFirst(); (s = adi.current()); ++adi) {
      QByteArray value = *s, newValue;
      int in, p = 0;

      // for each \n in the alias command value
      while ((in = value.find("\\n", p)) != -1) {
         newValue += value.mid(p, in - p);

         // expand \n's except if \n is part of a built-in command.
         if (value.mid(in, 5) != "\\note" &&
               value.mid(in, 5) != "\\name" &&
               value.mid(in, 10) != "\\namespace" &&
               value.mid(in, 14) != "\\nosubgrouping"
            ) {
            newValue += "\\_linebr ";
         } else {
            newValue += "\\n";
         }
         p = in + 2;
      }
      newValue += value.mid(p, value.length() - p);
      *s = newValue;
      //printf("Alias %s has value %s\n",adi.currentKey().data(),s->data());
   }
}

void readAliases()
{
   // add aliases to a dictionary
   QStringList &aliasList = Config_getList("ALIASES");
 
   for (auto alias : aliasList) {

      if (! Doxygen::aliasDict.contains(alias)) {         
         int i = alias.indexOf('=');

         if (i > 0) {
            QString name  = alias.left(i).trimmed();
            QString value = alias.right(alias.length() - i - 1);
            
            if (! name.isEmpty()) {
               // insert or update with the new alias
               Doxygen::aliasDict[name] = value;               
            }
         }
      }    
   }

   expandAliases();
   escapeAliases();
}

void Doxy_Work::dumpSymbol(FTextStream &t, Definition *d)
{
   QByteArray anchor;
   if (d->definitionType() == Definition::TypeMember) {
      MemberDef *md = (MemberDef *)d;
      anchor = ":" + md->anchor();
   }
   QByteArray scope;
   if (d->getOuterScope() && d->getOuterScope() != Doxygen::globalScope) {
      scope = d->getOuterScope()->getOutputFileBase() + Doxygen::htmlFileExtension;
   }
   t << "REPLACE INTO symbols (symbol_id,scope_id,name,file,line) VALUES('"
     << d->getOutputFileBase() + Doxygen::htmlFileExtension + anchor << "','"
     << scope << "','"
     << d->name() << "','"
     << d->getDefFileName() << "','"
     << d->getDefLine()
     << "');" << endl;
}

void Doxy_Work::dumpSymbolMap()
{
   QFile f("symbols.sql");

   if (f.open(QIODevice::WriteOnly)) {
      FTextStream t(&f);
      QHashIterator<QString,DefinitionIntf *> di(*Doxygen::symbolMap);
      DefinitionIntf *intf;

      for (; (intf = di.current()); ++di) {
         if (intf->definitionType() == DefinitionIntf::TypeSymbolList) { // list of symbols
            DefinitionListIterator dli(*(DefinitionList *)intf);
            Definition *d;
            // for each symbol
            for (dli.toFirst(); (d = dli.current()); ++dli) {
               dumpSymbol(t, d);
            }
         } else { // single symbol
            Definition *d = (Definition *)intf;
            if (d != Doxygen::globalScope) {
               dumpSymbol(t, d);
            }
         }
      }
   }
}

// read the argument of option `c' from the comment argument list and
// update the option index `optind'.
int Doxy_Work::computeIdealCacheParam(uint v)
{
   //printf("computeIdealCacheParam(v=%u)\n",v);

   int r = 0;
   while (v != 0) {
      v >>= 1, r++;
   }
   // r = log2(v)

   // convert to a valid cache size value
   return qMax(0, qMin(r - 16, 9));
}

#ifdef HAS_SIGNALS
static void stopDoxygen(int)
{
   QDir thisDir;
   msg("Cleaning up...\n");

   if (! Doxygen::entryDBFileName.isEmpty()) {
      thisDir.remove(Doxygen::entryDBFileName);
   }
   if (!Doxygen::objDBFileName.isEmpty()) {
      thisDir.remove(Doxygen::objDBFileName);
   }
   killpg(0, SIGINT);
   exit(1);
}
#endif

void Doxy_Work::writeTagFile()
{
   QByteArray &generateTagFile = Config_getString("GENERATE_TAGFILE");
   if (generateTagFile.isEmpty()) {
      return;
   }

   QFile tag(generateTagFile);
   if (!tag.open(QIODevice::WriteOnly)) {
      err("cannot open tag file %s for writing\n",
          generateTagFile.data()
         );
      return;
   }

   FTextStream tagFile(&tag);
   tagFile << "<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>" << endl;
   tagFile << "<tagfile>" << endl;

   // for each file
   for (auto fn : *Doxygen::inputNameList) {
      for (auto fd : *fn) {
         fd->writeTagFile(tagFile);
      }
   }

   // for each class
   for (auto cd : *Doxygen::classSDict) {
      cd->writeTagFile(tagFile);
   }

   // for each namespace
   for (auto nd : *Doxygen::namespaceSDict) {
      nd->writeTagFile(tagFile);
   }

   // for each group
   for (auto gd : *Doxygen::groupSDict) {
      gd->writeTagFile(tagFile);
   }

   // for each page
   for (auto pd : *Doxygen::pageSDict) {
      pd->writeTagFile(tagFile);
   }

   if (Doxygen::mainPage) {
      Doxygen::mainPage->writeTagFile(tagFile);
   }

   /*
   if (Doxygen::mainPage && !Config_getString("GENERATE_TAGFILE").isEmpty())
   {
     tagFile << "  <compound kind=\"page\">" << endl
                      << "    <name>"
                      << convertToXML(Doxygen::mainPage->name())
                      << "</name>" << endl
                      << "    <title>"
                      << convertToXML(Doxygen::mainPage->title())
                      << "</title>" << endl
                      << "    <filename>"
                      << convertToXML(Doxygen::mainPage->getOutputFileBase())
                      << "</filename>" << endl;

     mainPage->writeDocAnchorsToTagFile();
     tagFile << "  </compound>" << endl;
   }
   */

   tagFile << "</tagfile>" << endl;
}

QByteArray Doxy_Work:getQchFileName()
{
   QByteArray const &qchFile = Config_getString("QCH_FILE");

   if (!qchFile.isEmpty()) {
      return qchFile;
   }

   QByteArray const &projectName = Config_getString("PROJECT_NAME");
   QByteArray const &versionText = Config_getString("PROJECT_NUMBER");

   return QByteArray("../qch/")
          + (projectName.isEmpty() ? QByteArray("index") : projectName)
          + (versionText.isEmpty() ? QByteArray("") : QByteArray("-") + versionText)
          + QByteArray(".qch");
}

void searchInputFiles()
{
   QStringList &exclPatterns = Config_getList("EXCLUDE_PATTERNS");
   bool alwaysRecursive = Config_getBool("RECURSIVE");
   StringDict excludeNameDict;

   // gather names of all files in the include path
   Doxy_Globals::g_stats.begin("Searching for include files...\n");
   QStringList &includePathList = Config_getList("INCLUDE_PATH");

   for (auto s : includePathList) {
      QStringList &pl = Config_getList("INCLUDE_FILE_PATTERNS");

      if (pl.count() == 0) {
         pl = Config_getList("FILE_PATTERNS");
      }

      readFileOrDirectory(s, 0, Doxygen::includeNameDict, 0, &pl, &exclPatterns, 0, 0, alwaysRecursive);
   }
   Doxy_Globals::g_stats.end();

   //
   Doxy_Globals::g_stats.begin("Searching for example files...\n");
   QStringList &examplePathList = Config_getList("EXAMPLE_PATH");

   for (auto s : examplePathList) {
      readFileOrDirectory(s, 0, Doxygen::exampleNameDict, 0, &Config_getList("EXAMPLE_PATTERNS"),
                          0, 0, 0, (alwaysRecursive || Config_getBool("EXAMPLE_RECURSIVE")));
   }
   Doxy_Globals::g_stats.end();

   //
   Doxy_Globals::g_stats.begin("Searching for images...\n");

   QStringList &imagePathList = Config_getList("IMAGE_PATH");

   for (auto s : imagePathList) {
      readFileOrDirectory(s, 0, Doxygen::imageNameDict, 0, 0, 0, 0, 0, alwaysRecursive);
   }
   Doxy_Globals::g_stats.end();

   //
   Doxy_Globals::g_stats.begin("Searching for dot files...\n");
   QStringList &dotFileList = Config_getList("DOTFILE_DIRS");

   for (auto s : dotFileList) {
      readFileOrDirectory(s, 0, Doxygen::dotFileNameDict, 0, 0, 0, 0, 0, alwaysRecursive);
   }
   Doxy_Globals::g_stats.end();

   //
   Doxy_Globals::g_stats.begin("Searching for msc files...\n");
   QStringList &mscFileList = Config_getList("MSCFILE_DIRS");

   for (auto s : mscFileList) {
      readFileOrDirectory(s, 0, Doxygen::mscFileNameDict, 0, 0, 0, 0, 0, alwaysRecursive);
   }
   Doxy_Globals::g_stats.end();

   //
   Doxy_Globals::g_stats.begin("Searching for dia files...\n");
   QStringList &diaFileList = Config_getList("DIAFILE_DIRS");

   for (auto s : diaFileList) {
      readFileOrDirectory(s, 0, Doxygen::diaFileNameDict, 0, 0, 0, 0, 0, alwaysRecursive);
   }
   Doxy_Globals::g_stats.end();

   Doxy_Globals::g_stats.begin("Searching for files to exclude\n");
   QStringList &excludeList = Config_getList("EXCLUDE");

   for (auto s : excludeList) {
      readFileOrDirectory(s, 0, 0, 0, &Config_getList("FILE_PATTERNS"), 0, 0, &excludeNameDict, alwaysRecursive, false);
   }
   Doxy_Globals::g_stats.end();

   /**************************************************************************
    *             Determine Input Files                                      *
    **************************************************************************/

   Doxy_Globals::g_stats.begin("Searching for files to process...\n");

   QHash<QString, void *> killDict;
   QStringList &inputList = Config_getList("INPUT");

   for (auto s : inputList) {
      QByteArray path = s.toUtf8();
      uint l = path.length();

      if (l > 0) {
         // strip trailing slashes
         if (path.at(l - 1) == '\\' || path.at(l - 1) == '/') {
            path = path.left(l - 1);
         }

         readFileOrDirectory(path, Doxygen::inputNameList, Doxygen::inputNameDict, &excludeNameDict,
            &Config_getList("FILE_PATTERNS"), &exclPatterns, &Doxy_Globals::g_inputFiles, 0,
            alwaysRecursive, true, &killDict, &Doxygen::inputPaths);
      }
   }

   Doxy_Globals::g_stats.end();
}
