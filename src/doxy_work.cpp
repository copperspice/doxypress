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

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QHash>
#include <QRegExp>
#include <QStringList>
#include <QTextCodec>
#include <QTextStream>

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
#include <context.h>
#include <dbusxmlscanner.h>
#include <debug.h>
#include <declinfo.h>
#include <defargs.h>
#include <defgen.h>
#include <dirdef.h>
#include <docbookgen.h>
#include <docparser.h>
#include <docsets.h>
#include <dot.h>
#include <dox_build_info.h>
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
#include <message.h>
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


// ** 
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
   Doxygen::objDBFileName.sprintf("doxygen_objdb_%d.tmp", pid);
   Doxygen::objDBFileName.prepend(outputDirectory + "/");
   Doxygen::entryDBFileName.sprintf("doxygen_entrydb_%d.tmp", pid);
   Doxygen::entryDBFileName.prepend(outputDirectory + "/");

   if (Doxygen::symbolStorage->open(Doxygen::objDBFileName) == -1) {
      err("Failed to open temporary file %s\n", Doxygen::objDBFileName.data());
      exit(1);
   }

  
   // Check/create output directorties
 
   QByteArray htmlOutput;
   bool &generateHtml = Config_getBool("GENERATE_HTML");
   if (generateHtml || g_useOutputTemplate /* TODO: temp hack */) {
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



   /**************************************************************************
    *             Handle layout file                                         *
    **************************************************************************/

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
      t.setEncoding(QTextStream::Latin1);
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

   /**************************************************************************
    *             Handle Tag Files                                           *
    **************************************************************************/

   g_storage = new FileStorage;
   g_storage->setName(Doxygen::entryDBFileName);
   if (!g_storage->open(QIODevice::WriteOnly)) {
      err("Failed to create temporary storage file %s\n",
          Doxygen::entryDBFileName.data());
      exit(1);
   }
   Entry *root = new Entry;
   EntryNav *rootNav = new EntryNav(0, root);
   rootNav->setEntry(root);
   msg("Reading and parsing tag files\n");

   QStringList &tagFileList = Config_getList("TAGFILES");
   char *s = tagFileList.first();
   while (s) {
      readTagFile(root, s);
      root->createNavigationIndex(rootNav, g_storage, 0);
      s = tagFileList.next();
   }

   /**************************************************************************
    *             Parse source files                                         *
    **************************************************************************/

   if (Config_getBool("BUILTIN_STL_SUPPORT")) {
      addSTLClasses(rootNav);
   }

   g_s.begin("Parsing files\n");
   parseFiles(root, rootNav);
   g_storage->close();
   g_s.end();

   // we are done with input scanning now, so free up the buffers used by flex
   // (can be around 4MB)
   preFreeScanner();
   scanFreeScanner();
   pyscanFreeScanner();

   if (!g_storage->open(QIODevice::ReadOnly)) {
      err("Failed to open temporary storage file %s for reading",
          Doxygen::entryDBFileName.data());
      exit(1);
   }

   /**************************************************************************
    *             Gather information                                         *
    **************************************************************************/

   g_s.begin("Building group list...\n");
   buildGroupList(rootNav);
   organizeSubGroups(rootNav);
   g_s.end();

   g_s.begin("Building directory list...\n");
   buildDirectories();
   findDirDocumentation(rootNav);
   g_s.end();

   g_s.begin("Building namespace list...\n");
   buildNamespaceList(rootNav);
   findUsingDirectives(rootNav);
   g_s.end();

   g_s.begin("Building file list...\n");
   buildFileList(rootNav);
   g_s.end();
   //generateFileTree();

   g_s.begin("Building class list...\n");
   buildClassList(rootNav);
   g_s.end();

   g_s.begin("Associating documentation with classes...\n");
   buildClassDocList(rootNav);

   // build list of using declarations here (global list)
   buildListOfUsingDecls(rootNav);
   g_s.end();

   g_s.begin("Computing nesting relations for classes...\n");
   resolveClassNestingRelations();
   g_s.end();

   // 1.8.2-20121111: no longer add nested classes to the group as well
   //distributeClassGroupRelations();

   // calling buildClassList may result in cached relations that
   // become invalid after resolveClassNestingRelations(), that's why
   // we need to clear the cache here
   Doxygen::lookupCache->clear();

   // we don't need the list of using declaration anymore
   g_usingDeclarations.clear();

   g_s.begin("Building example list...\n");
   buildExampleList(rootNav);
   g_s.end();

   g_s.begin("Searching for enumerations...\n");
   findEnums(rootNav);
   g_s.end();

   // Since buildVarList calls isVarWithConstructor
   // and this calls getResolvedClass we need to process
   // typedefs first so the relations between classes via typedefs
   // are properly resolved. See bug 536385 for an example.
   g_s.begin("Searching for documented typedefs...\n");
   buildTypedefList(rootNav);
   g_s.end();

   g_s.begin("Searching for members imported via using declarations...\n");
   findUsingDeclImports(rootNav);
   // this should be after buildTypedefList in order to properly import
   // used typedefs
   findUsingDeclarations(rootNav);
   g_s.end();

   g_s.begin("Searching for included using directives...\n");
   findIncludedUsingDirectives();
   g_s.end();

   g_s.begin("Searching for documented variables...\n");
   buildVarList(rootNav);
   g_s.end();

   g_s.begin("Building interface member list...\n");
   buildInterfaceAndServiceList(rootNav); // UNO IDL

   g_s.begin("Building member list...\n"); // using class info only !
   buildFunctionList(rootNav);
   g_s.end();

   g_s.begin("Searching for friends...\n");
   findFriends();
   g_s.end();

   g_s.begin("Searching for documented defines...\n");
   findDefineDocumentation(rootNav);
   g_s.end();

   g_s.begin("Computing class inheritance relations...\n");
   findClassEntries(rootNav);
   findInheritedTemplateInstances();
   g_s.end();

   g_s.begin("Computing class usage relations...\n");
   findUsedTemplateInstances();
   g_s.end();

   if (Config_getBool("INLINE_SIMPLE_STRUCTS")) {
      g_s.begin("Searching for tag less structs...\n");
      findTagLessClasses();
      g_s.end();
   }

   g_s.begin("Flushing cached template relations that have become invalid...\n");
   flushCachedTemplateRelations();
   g_s.end();

   g_s.begin("Creating members for template instances...\n");
   createTemplateInstanceMembers();
   g_s.end();

   g_s.begin("Computing class relations...\n");
   computeTemplateClassRelations();
   flushUnresolvedRelations();

   computeClassRelations();
   g_classEntries.clear();
   g_s.end();

   g_s.begin("Add enum values to enums...\n");
   addEnumValuesToEnums(rootNav);
   findEnumDocumentation(rootNav);
   g_s.end();

   g_s.begin("Searching for member function documentation...\n");
   findObjCMethodDefinitions(rootNav);
   findMemberDocumentation(rootNav); // may introduce new members !

   transferRelatedFunctionDocumentation();
   transferFunctionDocumentation();
   g_s.end();

   g_s.begin("Building page list...\n");
   buildPageList(rootNav);
   g_s.end();

   g_s.begin("Search for main page...\n");
   findMainPage(rootNav);
   findMainPageTagFiles(rootNav);
   g_s.end();

   g_s.begin("Computing page relations...\n");
   computePageRelations(rootNav);
   checkPageRelations();
   g_s.end();

   g_s.begin("Determining the scope of groups...\n");
   findGroupScope(rootNav);
   g_s.end();

   g_s.begin("Sorting lists...\n");
   Doxygen::memberNameSDict->sort();
   Doxygen::functionNameSDict->sort();
   Doxygen::hiddenClasses->sort();
   Doxygen::classSDict->sort();
   g_s.end();

   msg("Freeing entry tree\n");
   delete rootNav;
   g_storage->close();
   delete g_storage;
   g_storage = 0;

   QDir thisDir;
   thisDir.remove(Doxygen::entryDBFileName);

   g_s.begin("Determining which enums are documented\n");
   findDocumentedEnumValues();
   g_s.end();

   g_s.begin("Computing member relations...\n");
   mergeCategories();
   computeMemberRelations();
   g_s.end();

   g_s.begin("Building full member lists recursively...\n");
   buildCompleteMemberLists();
   g_s.end();

   g_s.begin("Adding members to member groups.\n");
   addMembersToMemberGroup();
   g_s.end();

   if (Config_getBool("DISTRIBUTE_GROUP_DOC")) {
      g_s.begin("Distributing member group documentation.\n");
      distributeMemberGroupDocumentation();
      g_s.end();
   }

   g_s.begin("Computing member references...\n");
   computeMemberReferences();
   g_s.end();

   if (Config_getBool("INHERIT_DOCS")) {
      g_s.begin("Inheriting documentation...\n");
      inheritDocumentation();
      g_s.end();
   }

   // compute the shortest possible names of all files
   // without losing the uniqueness of the file names.

   g_s.begin("Generating disk names...\n");

   for (auto item : Doxygen::inputNameList) {
      item.generateDiskNames();
   }

   g_s.end();

   g_s.begin("Adding source references...\n");
   addSourceReferences();
   g_s.end();

   g_s.begin("Adding xrefitems...\n");
   addListReferences();
   generateXRefPages();
   g_s.end();

   g_s.begin("Sorting member lists...\n");
   sortMemberLists();
   g_s.end();

   if (Config_getBool("DIRECTORY_GRAPH")) {
      g_s.begin("Computing dependencies between directories...\n");
      computeDirDependencies();
      g_s.end();
   }

   //g_s.begin("Resolving citations...\n");
   //Doxygen::citeDict->resolve();

   g_s.begin("Generating citations page...\n");
   Doxygen::citeDict->generatePage();
   g_s.end();

   g_s.begin("Counting data structures...\n");
   countDataStructures();
   g_s.end();

   g_s.begin("Resolving user defined references...\n");
   resolveUserReferences();
   g_s.end();

   g_s.begin("Finding anchors and sections in the documentation...\n");
   findSectionsInDocumentation();
   g_s.end();

   g_s.begin("Transferring function references...\n");
   transferFunctionReferences();
   g_s.end();

   g_s.begin("Combining using relations...\n");
   combineUsingRelations();
   g_s.end();

   g_s.begin("Adding members to index pages...\n");
   addMembersToIndex();
   g_s.end();
}

void generateOutput()
{
   /**************************************************************************
    *            Initialize output generators                                *
    **************************************************************************/

   //// dump all symbols
   if (g_dumpSymbolMap) {
      dumpSymbolMap();
      exit(0);
   }

   initSearchIndexer();

   bool generateHtml  = Config_getBool("GENERATE_HTML");
   bool generateLatex = Config_getBool("GENERATE_LATEX");
   bool generateMan   = Config_getBool("GENERATE_MAN");
   bool generateRtf   = Config_getBool("GENERATE_RTF");


   g_outputList = new OutputList(true);
   if (generateHtml) {
      g_outputList->add(new HtmlGenerator);
      HtmlGenerator::init();

      // add HTML indexers that are enabled
      bool generateHtmlHelp    = Config_getBool("GENERATE_HTMLHELP");
      bool generateEclipseHelp = Config_getBool("GENERATE_ECLIPSEHELP");
      bool generateQhp         = Config_getBool("GENERATE_QHP");
      bool generateTreeView    = Config_getBool("GENERATE_TREEVIEW");
      bool generateDocSet      = Config_getBool("GENERATE_DOCSET");
      if (generateEclipseHelp) {
         Doxygen::indexList->addIndex(new EclipseHelp);
      }
      if (generateHtmlHelp) {
         Doxygen::indexList->addIndex(new HtmlHelp);
      }
      if (generateQhp) {
         Doxygen::indexList->addIndex(new Qhp);
      }
      if (generateTreeView) {
         Doxygen::indexList->addIndex(new FTVHelp(true));
      }
      if (generateDocSet) {
         Doxygen::indexList->addIndex(new DocSets);
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
      g_outputList->add(new LatexGenerator);
      LatexGenerator::init();

      // copy static stuff
      copyExtraFiles("LATEX_EXTRA_FILES", "LATEX_OUTPUT");
   }
   if (generateMan) {
      g_outputList->add(new ManGenerator);
      ManGenerator::init();
   }
   if (generateRtf) {
      g_outputList->add(new RTFGenerator);
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

   g_s.begin("Generating style sheet...\n");
   //printf("writing style info\n");
   g_outputList->writeStyleInfo(0); // write first part
   g_s.end();

   static bool searchEngine      = Config_getBool("SEARCHENGINE");
   static bool serverBasedSearch = Config_getBool("SERVER_BASED_SEARCH");

   // generate search indices (need to do this before writing other HTML
   // pages as these contain a drop down menu with options depending on
   // what categories we find in this function.
   if (generateHtml && searchEngine) {
      g_s.begin("Generating search indices...\n");
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
      g_s.end();
   }

   g_s.begin("Generating example documentation...\n");
   generateExampleDocs();
   g_s.end();

   if (!Htags::useHtags) {
      g_s.begin("Generating file sources...\n");
      generateFileSources();
      g_s.end();
   }

   g_s.begin("Generating file documentation...\n");
   generateFileDocs();
   g_s.end();

   g_s.begin("Generating page documentation...\n");
   generatePageDocs();
   g_s.end();

   g_s.begin("Generating group documentation...\n");
   generateGroupDocs();
   g_s.end();

   g_s.begin("Generating class documentation...\n");
   generateClassDocs();
   g_s.end();

   g_s.begin("Generating namespace index...\n");
   generateNamespaceDocs();
   g_s.end();

   if (Config_getBool("GENERATE_LEGEND")) {
      g_s.begin("Generating graph info page...\n");
      writeGraphInfo(*g_outputList);
      g_s.end();
   }

   g_s.begin("Generating directory documentation...\n");
   generateDirDocs(*g_outputList);
   g_s.end();

   if (Doxygen::formulaList->count() > 0 && generateHtml
         && !Config_getBool("USE_MATHJAX")) {
      g_s.begin("Generating bitmaps for formulas in HTML...\n");
      Doxygen::formulaList->generateBitmaps(Config_getString("HTML_OUTPUT"));
      g_s.end();
   }

   if (Config_getBool("SORT_GROUP_NAMES")) {
      Doxygen::groupSDict->sort();
      GroupSDict::Iterator gli(*Doxygen::groupSDict);
      GroupDef *gd;
      for (gli.toFirst(); (gd = gli.current()); ++gli) {
         gd->sortSubGroups();
      }
   }

   if (g_outputList->count() > 0) {
      writeIndexHierarchy(*g_outputList);
   }

   g_s.begin("finalizing index lists...\n");
   Doxygen::indexList->finalize();
   g_s.end();

   g_s.begin("writing tag file...\n");
   writeTagFile();
   g_s.end();

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
      g_s.begin("Generating XML output...\n");
      Doxygen::generatingXmlOutput = true;
      generateXML();
      Doxygen::generatingXmlOutput = false;
      g_s.end();
   }
   if (USE_SQLITE3) {
      g_s.begin("Generating SQLITE3 output...\n");
      generateSqlite3();
      g_s.end();
   }

   if (Config_getBool("GENERATE_DOCBOOK")) {
      g_s.begin("Generating Docbook output...\n");
      generateDocbook();
      g_s.end();
   }

   if (Config_getBool("GENERATE_AUTOGEN_DEF")) {
      g_s.begin("Generating AutoGen DEF output...\n");
      generateDEF();
      g_s.end();
   }
   if (Config_getBool("GENERATE_PERLMOD")) {
      g_s.begin("Generating Perl module output...\n");
      generatePerlMod();
      g_s.end();
   }
   if (generateHtml && searchEngine && serverBasedSearch) {
      g_s.begin("Generating search index\n");
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
      g_s.end();
   }

   if (g_useOutputTemplate) {
      generateOutputViaTemplate();
   }

   if (generateRtf) {
      g_s.begin("Combining RTF output...\n");
      if (!RTFGenerator::preProcessFileInplace(Config_getString("RTF_OUTPUT"), "refman.rtf")) {
         err("An error occurred during post-processing the RTF files!\n");
      }
      g_s.end();
   }

   if (Config_getBool("HAVE_DOT")) {
      g_s.begin("Running dot...\n");
      DotManager::instance()->run();
      g_s.end();
   }

   if (generateHtml &&
         Config_getBool("GENERATE_HTMLHELP") &&
         !Config_getString("HHC_LOCATION").isEmpty()) {
      g_s.begin("Running html help compiler...\n");
      QString oldDir = QDir::currentPath();
      QDir::setCurrent(Config_getString("HTML_OUTPUT"));
      portable_sysTimerStart();
      if (portable_system(Config_getString("HHC_LOCATION"), "index.hhp", false)) {
         err("failed to run html help compiler on index.hhp\n");
      }
      portable_sysTimerStop();
      QDir::setCurrent(oldDir);
      g_s.end();
   }
   if ( generateHtml &&
         Config_getBool("GENERATE_QHP") &&
         !Config_getString("QHG_LOCATION").isEmpty()) {
      g_s.begin("Running qhelpgenerator...\n");
      QByteArray const qhpFileName = Qhp::getQhpFileName();
      QByteArray const qchFileName = getQchFileName();

      QByteArray const args = QByteArray().sprintf("%s -o \"%s\"", qhpFileName.data(), qchFileName.data());
      QString const oldDir = QDir::currentPath();
      QDir::setCurrent(Config_getString("HTML_OUTPUT"));
      portable_sysTimerStart();
      if (portable_system(Config_getString("QHG_LOCATION"), args.data(), false)) {
         err("failed to run qhelpgenerator on index.qhp\n");
      }
      portable_sysTimerStop();
      QDir::setCurrent(oldDir);
      g_s.end();
   }

   int cacheParam;
   msg("lookup cache used %d/%d hits=%d misses=%d\n",
       Doxygen::lookupCache->count(),
       Doxygen::lookupCache->size(),
       Doxygen::lookupCache->hits(),
       Doxygen::lookupCache->misses());

   cacheParam = computeIdealCacheParam(Doxygen::lookupCache->misses() * 2 / 3); // part of the cache is flushed, hence the 2/3 correction factor

   if (cacheParam > Config_getInt("LOOKUP_CACHE_SIZE")) {
      msg("Note: based on cache misses the ideal setting for LOOKUP_CACHE_SIZE is %d at the cost of higher memory usage.\n", cacheParam);
   }

   if (Debug::isFlagSet(Debug::Time)) {
      msg("Total elapsed time: %.3f seconds\n(of which %.3f seconds waiting for external tools to finish)\n",
          ((double)Doxygen::runningTime.elapsed()) / 1000.0,
          portable_getSysElapsedTime()
         );
      g_s.print();
   } else {
      msg("finished...\n");
   }
   
   // Start cleaning up                                  
   cleanUpDoxygen();

   finializeSearchIndexer();
   Doxygen::symbolStorage->close();

   QDir thisDir;
   thisDir.remove(Doxygen::objDBFileName);
   Config::deleteInstance();

   QTextCodec::deleteAllCodecs();
   delete Doxygen::symbolMap;
   delete Doxygen::clangUsrMap;
   delete Doxygen::symbolStorage;

   g_successfulRun = true;
}
