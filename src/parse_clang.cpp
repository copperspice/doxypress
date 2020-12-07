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

#include <QByteArray>
#include <QHash>
#include <QSet>

#include <stdio.h>
#include <stdlib.h>

#include <parse_clang.h>
#include <parse_lib_tooling.h>

#include <commentscan.h>
#include <config.h>
#include <doxy_globals.h>
#include <entry.h>
#include <message.h>
#include <outputgen.h>
#include <qfileinfo.h>
#include <stringmap.h>
#include <tooltip.h>
#include <util.h>

static void handleCommentBlock(const QString &comment, bool brief, const QString &fileName, QSharedPointer<Entry> current);

static QSet<QString>               s_includedFiles;
static QSharedPointer<Definition>  s_currentDefinition;
static QSharedPointer<MemberDef>   s_currentMemberDef;

static uint g_currentLine   = 0;
static uint g_bracketCount  = 0;
static bool g_searchForBody = false;
static bool g_insideBody    = false;

QSharedPointer<Entry>                s_current_root;
QMap<QString, QSharedPointer<Entry>> s_entryMap;

static void writeLineNumber(CodeOutputInterface &ol, QSharedPointer<FileDef> fd, uint line);

class ClangParser::Private
{
 public:
   enum DetectedLang { Detected_Cpp, Detected_ObjC, Detected_ObjCpp };

   Private()
      : sources(nullptr), numFiles(0), numTokens(0), curLine(0), curToken(0),
        tu(0), tokens(nullptr), cursors(nullptr), ufs(nullptr), detectedLang(Detected_Cpp)
   {
   }

   int getCurrentTokenLine();

   QString fileName;
   QByteArray *sources;

   uint numFiles;
   uint numTokens;
   uint curLine;
   uint curToken;

   CXIndex            index;
   CXTranslationUnit  tu;
   CXToken           *tokens;
   CXCursor          *cursors;
   CXUnsavedFile     *ufs;

   QHash<QString, uint> fileMapping;
   DetectedLang detectedLang;
};

int ClangParser::Private::getCurrentTokenLine()
{
   if (numTokens == 0) {
      return 1;
   }

   // guard against filters that reduce the number of lines
   uint retval;
   uint c;

   if (curToken >= numTokens) {
      curToken = numTokens - 1;
   }

   CXSourceLocation start = clang_getTokenLocation(tu, tokens[curToken]);
   clang_getSpellingLocation(start, 0, &retval, &c, 0);

   return retval;
}

ClangParser *ClangParser::instance()
{
   static ClangParser m_instance;
   return &m_instance;
}

ClangParser::ClangParser()
{
   p = new ClangParser::Private;
}

ClangParser::~ClangParser()
{
   delete p;
}

ClangParser::Private *ClangParser::getPrivate()
{
   return p;
}

static void codifyLines(CodeOutputInterface &ol, QSharedPointer<FileDef> fd,
                  const QString &text, uint &line, uint &column, const QString &fontClass)
{
   if (! fontClass.isEmpty()) {
      ol.startFontClass(fontClass);
   }

   int pos = 0;
   int posStart;

   QChar c;
   bool done = false;

   while (! done) {
      posStart = pos;

      while (pos < text.length()) {
         c = text.at(pos);
         ++pos;

         if (c == '\n') {
            break;
         }

        column++;
      }

      if (c == '\n') {
         line++;

         int len = pos - posStart - 1;
         column  = len + 1;

         ol.codify(text.mid(posStart, len));

         if (! fontClass.isEmpty()) {
            ol.endFontClass();
         }

         ol.endCodeLine();

         ol.startCodeLine(true);
         writeLineNumber(ol, fd, line);

         if (! fontClass.isEmpty()) {
            ol.startFontClass(fontClass);
         }

      } else {
         ol.codify(text.mid(posStart) );
         done = true;
      }
   }

   if (! fontClass.isEmpty()) {
      ol.endFontClass();
   }
}

static QString detab(const QString &str)
{
   static int tabSize = Config::getInt("tab-size");

   QString out;
   int col = 0;

   const int maxIndent = 1000000;    // value representing infinity
   int minIndent = maxIndent;

   for (auto c : str) {

      switch (c.unicode()) {

         case '\t': {
            // expand tab
            int stop = tabSize - (col % tabSize);
            col += stop;

            while (stop--) {
               out += ' ';
            }
         }
         break;

         case '\n':
            // reset column counter
            out += c;
            col = 0;
            break;

         case ' ':
            // increment column counter
            out += c;
            col++;
            break;

         default:
            // non-whitespace, update minIndent
            out += c;

            if (col < minIndent) {
               minIndent = col;
            }
            col++;
      }
   }

   return out;
}

static void detectFunctionBody(const QString s)
{
  if (g_searchForBody && (s == ":" || s == "{")) {
      // start of 'body' (: is for constructor)
      g_searchForBody = false;
      g_insideBody    = true;

   } else if (g_searchForBody && s == ";") {
      // declaration only
      g_searchForBody = false;
      g_insideBody    = false;
   }

   if (g_insideBody && s == "{") {
      // increase scoping level
      g_bracketCount++;
   }

   if (g_insideBody && s == "}") {
      // decrease scoping level
      g_bracketCount--;

      if (g_bracketCount <= 0) {
         // got outside of function body
         g_insideBody   = false;
         g_bracketCount = 0;
      }
   }
}

static QString getCursorSpelling(CXCursor cursor)
{
   CXString text  = clang_getCursorSpelling(cursor);
   QString retval = QString::fromLatin1(clang_getCString(text));

   clang_disposeString(text);

   return retval;
}

static QString getCursorUSR(CXCursor cursor)
{
   CXString text  = clang_getCursorUSR(cursor);
   QString retval = QString::fromLatin1(clang_getCString(text));

   clang_disposeString(text);

   return retval;
}

static QString getFileName(CXFile file)
{
   CXString text  = clang_getFileName(file);
   QString retval = QString::fromLatin1(clang_getCString(text));

   clang_disposeString(text);

   return retval;
}

static QString getTokenSpelling(CXTranslationUnit tu, CXToken token)
{
   CXString text   = clang_getTokenSpelling(tu, token);
   QString  retval = QString::fromLatin1(clang_getCString(text));

   clang_disposeString(text);

   return retval;
}

static QString keywordToType(const QString &key)
{
   static bool init = true;
   static QMap<QString, QString> keyWords;

   if (init) {
      keyWords.insert("break",    "keywordflow");
      keyWords.insert("case",     "keywordflow");
      keyWords.insert("catch",    "keywordflow");
      keyWords.insert("continue", "keywordflow");
      keyWords.insert("default",  "keywordflow");
      keyWords.insert("do",       "keywordflow");
      keyWords.insert("else",     "keywordflow");
      keyWords.insert("finally",  "keywordflow");
      keyWords.insert("for",      "keywordflow");
      keyWords.insert("foreach",  "keywordflow");
      keyWords.insert("for each", "keywordflow");
      keyWords.insert("goto",     "keywordflow");
      keyWords.insert("if",       "keywordflow");
      keyWords.insert("return",   "keywordflow");
      keyWords.insert("switch",   "keywordflow");
      keyWords.insert("throw",    "keywordflow");
      keyWords.insert("throws",   "keywordflow");
      keyWords.insert("try",      "keywordflow");
      keyWords.insert("while",    "keywordflow");

      // objective-C
      keyWords.insert("@try",     "keywordflow");
      keyWords.insert("@catch",   "keywordflow");
      keyWords.insert("@finally", "keywordflow");

      keyWords.insert("bool",     "keywordtype");
      keyWords.insert("boolean",  "keywordtype");
      keyWords.insert("char",     "keywordtype");
      keyWords.insert("double",   "keywordtype");
      keyWords.insert("float",    "keywordtype");
      keyWords.insert("int",      "keywordtype");
      keyWords.insert("long",     "keywordtype");
      keyWords.insert("nullptr",  "keywordtype");
      keyWords.insert("object",   "keywordtype");
      keyWords.insert("short",    "keywordtype");
      keyWords.insert("signed",   "keywordtype");
      keyWords.insert("unsigned", "keywordtype");
      keyWords.insert("void",     "keywordtype");

      keyWords.insert("char16_t", "keywordtype");
      keyWords.insert("char32_t", "keywordtype");
      keyWords.insert("ssize_t",  "keywordtype");
      keyWords.insert("size_t",   "keywordtype");
      keyWords.insert("wchar_t",  "keywordtype");

      // objective-C
      keyWords.insert("id",       "keywordtype");
      keyWords.insert("SEL",      "keywordtype");
      keyWords.insert("string",   "keywordtype");

      init = false;
   }

   auto iter = keyWords.find(key);

   if (iter != keyWords.end()) {
      return iter.value();
   }

   return QString("keyword");
}

static void writeLineNumber(CodeOutputInterface &ol, QSharedPointer<FileDef> fd, uint line)
{
   QSharedPointer<Definition> d;

   if (fd) {
      d = fd->getSourceDefinition(line);
   }

   if (d && d->isLinkable()) {
      s_currentDefinition = d;
      g_currentLine = line;

      QSharedPointer<MemberDef> md = fd->getSourceMember(line);

      if (md && md->isLinkable()) {
         // link to member

         if (s_currentMemberDef != md) {
            // new member, start search for body

            g_searchForBody = true;
            g_insideBody    = false;
            g_bracketCount  = 0;
         }

         s_currentMemberDef = md;
         ol.writeLineNumber(md->getReference(), md->getOutputFileBase(), md->anchor(), line);

      } else {
         // link to compound
         s_currentMemberDef = QSharedPointer<MemberDef>();
         ol.writeLineNumber(d->getReference(), d->getOutputFileBase(), d->anchor(), line);
      }

   } else {
      // no link
      ol.writeLineNumber("", "", "", line);
   }

   // set search page target
   if (Doxy_Globals::searchIndexBase != nullptr) {
      QString lineAnchor = QString("l%1").formatArg(line, 5, 10, QChar('0'));
      ol.setCurrentDoc(fd, lineAnchor, true);
   }
}

static void writeMultiLineCodeLink(CodeOutputInterface &ol, QSharedPointer<FileDef> fd, uint &line, uint &column,
                  QSharedPointer<Definition> d, const QString &text)
{
   static bool sourceTooltips = Config::getBool("source-tooltips");
   TooltipManager::instance()->addTooltip(d);

   QString ref    = d->getReference();
   QString file   = d->getOutputFileBase();
   QString anchor = d->anchor();

   QString tooltip;

   if (! sourceTooltips) {
      // fall back to simple "title" tooltips
      tooltip = d->briefDescriptionAsTooltip();
   }

   QString tmp;

   for (auto c : text) {

      if (c == '\n') {
         line++;

         ol.writeCodeLink(ref, file, anchor, tmp, tooltip);
         ol.endCodeLine();

         ol.startCodeLine(true);
         writeLineNumber(ol, fd, line);

         tmp = "";

      } else {
         column++;
         tmp += c;

      }
   }

   if (! tmp.isEmpty() )  {
      ol.writeCodeLink(ref, file, anchor, tmp, tooltip);
   }
}

// call back, called for each include in a translation unit
static void inclusionVisitor(CXFile file, CXSourceLocation *, uint, CXClientData clientData)
{
   (void) clientData;
   s_includedFiles.insert(getFileName(file));
}

static bool documentKind(CXCursor cursor)
{
   CXCursorKind kind = clang_getCursorKind(cursor);

   if (kind == CXCursor_UnexposedDecl             ||            // 1
       kind == CXCursor_StructDecl                ||            // 2
       kind == CXCursor_UnionDecl                 ||            // 3
       kind == CXCursor_ClassDecl                 ||            // 4
       kind == CXCursor_EnumDecl                  ||            // 5
       kind == CXCursor_FieldDecl                 ||            // 6
       kind == CXCursor_EnumConstantDecl          ||            // 7
       kind == CXCursor_FunctionDecl              ||            // 8
       kind == CXCursor_VarDecl                   ||            // 9
       kind == CXCursor_ParmDecl                  ||            // 10

       kind == CXCursor_TypedefDecl               ||            // 20
       kind == CXCursor_CXXMethod                 ||            // 21
       kind == CXCursor_Namespace                 ||            // 22

       kind == CXCursor_Constructor               ||            // 24
       kind == CXCursor_Destructor                ||            // 25
       kind == CXCursor_ConversionFunction        ||            // 26
       kind == CXCursor_TemplateTypeParameter     ||            // 27
       kind == CXCursor_NonTypeTemplateParameter  ||            // 28
       kind == CXCursor_TemplateTemplateParameter ||            // 29
       kind == CXCursor_FunctionTemplate          ||            // 30
       kind == CXCursor_ClassTemplate             ||            // 31
       kind == CXCursor_ClassTemplatePartialSpecialization ||   // 32

       kind == CXCursor_UsingDirective            ||            // 34
       kind == CXCursor_UsingDeclaration          ||            // 35
       kind == CXCursor_TypeAliasDecl             ||            // 36

       kind == CXCursor_MacroDefinition   )                     // 501
   {
      return true;
   }

   return false;
}

// ** entry point
void ClangParser::start(const QString &fileName, const QString &fileBuffer, QStringList &includeFiles, QSharedPointer<Entry> root)
{
   static QStringList const includePath          = Config::getList("include-path");
   static QStringList const preDefinedMacros     = Config::getList("predefined-macros");

   static QString     const clangCompilationPath = Config::getString("clang-compilation-path");
   static QString     const clangDialect         = Config::getString("clang-dialect");
   static bool        const clangUseHeaders      = Config::getBool("clang-use-headers");
   static QStringList const clangFlags           = Config::getList("clang-flags");

   // static const Qt::CaseSensitivity allowUpperCaseNames_enum = Config::getCase("case-sensitive-fname");

   static std::vector<QString> clangCmdArgs;
   std::vector<QString> argList;

   static std::unique_ptr<clang::tooling::CompilationDatabase> db_json;
   static std::vector<clang::tooling::CompileCommand> db_commands;

   bool useFallBack = true;

   if (db_commands.empty() && ! clangCompilationPath.isEmpty()) {

      if (! QFile(clangCompilationPath).exists()) {
         errAll("Compilation database '%s' does not exist", csPrintable(clangCompilationPath));
      }

      std::string error;
      db_json = clang::tooling::CompilationDatabase::loadFromDirectory(clangCompilationPath.constData(), error);

      if (db_json == nullptr) {
            errAll("Using compilation path '%s' failed.\nClang error: %s\n", csPrintable(clangCompilationPath), error.c_str());
      }

      db_commands = db_json->getAllCompileCommands();
   }

   if (! db_commands.empty())  {
      // find the current file in the json compilation database
      QString tFname = fileName;
      QString tFakeHeader;

#ifdef Q_OS_WIN
      tFname = tFname.toLower();
#endif

      if (tFname.endsWith(".h")) {
         tFakeHeader = tFname;
         tFakeHeader.chop(2);
         tFakeHeader.append(".cpp");
      }

      std::vector<std::string> options;

      for (auto &cmd : db_commands)  {
         QString lookup = QString::fromStdString(cmd.Filename);

         if (lookup == tFname) {
            // exact match
            options = cmd.CommandLine;
            break;

         } else if (lookup == tFakeHeader)  {
            // .h would be fine
            options = cmd.CommandLine;
            break;

         }
      }

      if (! options.empty()) {
         for (auto iter = options.begin() + 1; iter != options.end(); ++iter) {
            QString item =  QString::fromStdString(*iter);

            if (item == "-isystem") {
               // take this one and the next one
               // argList.push_back(item);

               ++iter;
               argList.push_back("-I" + QString::fromStdString(*iter));

            } else if (item.startsWith("-D") || item.startsWith("-I") || item.startsWith("-c") || item.startsWith("-std")) {
               // take this one
               argList.push_back(item);
            }
         }

         argList.push_back("--no-warnings");
         argList.push_back("-ferror-limit=20");

         // user specified flags, last
         for (auto &item : clangFlags) {
            argList.push_back(item);
         }

         // keep last, next arg is the language id
         argList.push_back("-x");

         useFallBack = false;
      }
   }

   if (useFallBack)  {
      // not using the compilation database

      if (clangCmdArgs.empty()) {

         // user provided clang headers
         if (clangUseHeaders) {
            QString inc = "-I" + QCoreApplication::applicationDirPath() + "/include/10.0.1/include";
            argList.push_back(std::move(inc));
         }

         // add include paths for input files
         for (auto &item : Doxy_Globals::inputPaths) {
            argList.push_back("-I" + item);
         }

         // add external include paths
         for (auto &item : includePath) {
            argList.push_back("-I" + item);
         }

         // add predefinded macros
         for (const auto &item : preDefinedMacros) {
            argList.push_back("-D" + item);
         }

         // compile only
         argList.push_back("-c");

         // add dialect
         if (! clangDialect.isEmpty()) {
            argList.push_back(clangDialect);
         }

         argList.push_back("--no-warnings");
         argList.push_back("-ferror-limit=20");

         // user specified flags, last
         for (auto &item : clangFlags) {
            argList.push_back(item);
         }

         // keep last, next arg is the language id
         argList.push_back("-x");

         // save for the next pass
         clangCmdArgs = argList;

      } else {
         // just copy the first x args since they do not change
         argList = clangCmdArgs;

      }
   }

   // if the file is an .h file it can contain C, C++, or Objective C  (review)
   // detection will fail if we pass .h files containing ObjC code and no source

   SrcLangExt lang = getLanguageFromFileName(fileName);

   if (lang == SrcLangExt_ObjC || p->detectedLang != ClangParser::Private::Detected_Cpp) {
      QFileInfo fi(fileName);
      QString ext = fi.suffix().toLower();

      if (p->detectedLang == ClangParser::Private::Detected_Cpp &&
            (ext == "cpp" || ext == "cxx" || ext == "cc" || ext == "c")) {

         // fall back to C/C++ once we see an extension that indicates C++
         p->detectedLang = ClangParser::Private::Detected_Cpp;

      } else if (ext == "mm") {
         // switch to Objective C++
         p->detectedLang = ClangParser::Private::Detected_ObjCpp;

      } else if (ext == "m") {
         // switch to Objective C
         p->detectedLang = ClangParser::Private::Detected_ObjC;
      }
   }

   switch (p->detectedLang) {
      case ClangParser::Private::Detected_Cpp:
         argList.push_back("c++");
         break;

      case ClangParser::Private::Detected_ObjC:
         argList.push_back("objective-c");
         break;

      case ClangParser::Private::Detected_ObjCpp:
         argList.push_back("objective-c++");
         break;
   }

   // file name added
   argList.push_back(fileName);

   // exclude PCH files, disable diagnostics
   p->index    = clang_createIndex(false, false);

   p->fileName = fileName;
   p->curLine  = 1;
   p->curToken = 0;

   // provide the input and their dependencies as files in memory
   static bool filterSourceFiles = Config::getBool("filter-source-files");

   uint numUnsavedFiles = includeFiles.count() + 1;

   p->numFiles = numUnsavedFiles;
   p->sources  = new QByteArray[numUnsavedFiles];
   p->ufs      = new CXUnsavedFile[numUnsavedFiles];

   // load main file
   if (fileBuffer.isEmpty()) {
      p->sources[0]   = detab(fileToString(fileName, filterSourceFiles, true)).toUtf8();
   } else  {
      p->sources[0]   = fileBuffer.toUtf8();
   }

   p->ufs[0].Filename = strdup(fileName.toUtf8().constData());
   p->ufs[0].Contents = p->sources[0].constData();
   p->ufs[0].Length   = p->sources[0].length();

   //
   uint i = 1;
   for (const auto &item : includeFiles) {
      p->fileMapping.insert(item, i);

      // load include files
      p->sources[i]      = detab(fileToString(item, filterSourceFiles, true)).toUtf8();
      p->ufs[i].Filename = strdup(item.toUtf8().constData());
      p->ufs[i].Contents = p->sources[i].constData();
      p->ufs[i].Length   = p->sources[i].length();

      i++;
   }

   // copy data to a usable vector for clang
   std::vector<const char *> argv;
   for (auto &item : argList) {
      argv.push_back(item.constData());
   }

   int argc = argList.size();

   // passed data - index, 0, command line args, number of args, included files
   // num of unsaved files, clang flag indicating full preprocessing, translation unit structure

   // libClang - used to set up the tokens for comments
   CXErrorCode errorCode = clang_parseTranslationUnit2(p->index, 0, &argv[0], argc, p->ufs, numUnsavedFiles,
                  CXTranslationUnit_DetailedPreprocessingRecord, &(p->tu) );

   if (p->tu) {
      // filter out any includes not found by the clang parser
      determineInputFiles(includeFiles);

      // show warnings the compiler found
      uint diagCnt = clang_getNumDiagnostics(p->tu);

      for (uint i = 0; i != diagCnt; i++) {
         CXDiagnostic diag = clang_getDiagnostic(p->tu, i);
         CXString diagMsg  = clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());

         err("%s\n", clang_getCString(diagMsg));

         clang_disposeDiagnostic(diag);
         clang_disposeString(diagMsg);
      }

      if (diagCnt > 0) {
         msg("\n");
      }
   }

   if (errorCode == CXError_Success) {

      if (root == nullptr)  {
         // called from writeSouce() in fileDef

      } else {
         // libTooling - used to parse the source
         const std::string stdFName = fileName.constData();

         // start adding to our entry container
         s_current_root = root;
         s_entryMap.insert("TranslationUnit", root);

         if (false) {
/*
         if (! useFallBack)  {
            // hold for now

            QString tmp;
            for (auto item : argList) {
               tmp += item + "  ";
            }
            printf("\n Args: %s\n", csPrintable(tmp) );

            llvm::cl::OptionCategory category("doxypress");
            clang::tooling::CommonOptionsParser options(argc, &argv[0], category);

            // pass the main file name seperately
            std::vector<std::string> sourceList;
            sourceList.push_back(stdFName);

            // create a new clang tooling instance
            clang::tooling::ClangTool tool(options.getCompilations(), sourceList);

            // use the file in memory
            tool.mapVirtualFile(stdFName, p->sources[0].constData());

            // run the clang tooling to create a new FrontendAction
            int result = tool.run(clang::tooling::newFrontendActionFactory<DoxyFrontEnd>().get());
*/

         } else {
            // save argList in a different vector for libTooling
            std::vector<std::string> argTmp;

            for (auto &item : argList) {
               argTmp.push_back(item.constData());
            }

            // file name needed for libClang, but removed for libTooling
            argTmp.erase(argTmp.end() - 1);

            clang::tooling::FixedCompilationDatabase options(".", argTmp);

            // pass the main file name seperately
            std::vector<std::string> sourceList;
            sourceList.push_back(stdFName);

            // create a new clang tooling instance
            clang::tooling::ClangTool tool(options, sourceList);

            // use the file in memory
            tool.mapVirtualFile(stdFName, p->sources[0].constData());

            // run the clang tooling to create a new FrontendAction
            int result = tool.run(clang::tooling::newFrontendActionFactory<DoxyFrontEnd>().get());
            (void) result;
         }
      }

      // create a source range for the file
      QFileInfo fi(fileName);
      CXFile f = clang_getFile(p->tu, fileName.toUtf8().constData());

      CXSourceLocation fileBegin = clang_getLocationForOffset(p->tu, f, 0);
      CXSourceLocation fileEnd   = clang_getLocationForOffset(p->tu, f, p->ufs[0].Length);
      CXSourceRange    fileRange = clang_getRange(fileBegin, fileEnd);

      // generate tokens for the tu
      clang_tokenize(p->tu, fileRange, &p->tokens, &p->numTokens);

      // generate cursors for each token
      p->cursors = new CXCursor[p->numTokens];
      clang_annotateTokens(p->tu, p->tokens, p->numTokens, p->cursors);

      if (root == nullptr)  {
         // called from writeSouce() in fileDef
         return;
      }

      static const bool javadoc_auto_brief = Config::getBool("javadoc-auto-brief");
      static const bool qt_auto_brief      = Config::getBool("qt-auto-brief");

      // walk the tokens
      for (uint index = 0; index < p->numTokens; ++index)  {

         CXTokenKind tokenKind = clang_getTokenKind(p->tokens[index]);

         if (tokenKind == CXToken_Comment) {
            QString comment = getTokenSpelling(p->tu, p->tokens[index]).trimmed();

            CXCursor cursor;
            bool isBrief = false;

            // figure out which cursor this comment is associated with
            if (comment.isEmpty()) {
               // not a valid comment
               continue;

            } else if (comment.mid(3, 1) == "<")  {
               int tmpIndex = index - 1;

               while (tmpIndex >= 0)   {
                  cursor = p->cursors[tmpIndex];
                  bool found = true;

                  if (clang_Cursor_isNull(cursor) || ! documentKind(cursor)) {
                     found = false;

                  } else {
                     QString phrase = getTokenSpelling(p->tu, p->tokens[tmpIndex]);

                     if (phrase == "," || phrase == ";") {
                        found = false;
                     }
                  }

                  if (found) {
                     break;

                  } else {
                    tmpIndex--;
                    continue;

                  }
               }

               QChar char2 = comment.at(2);

               if (javadoc_auto_brief && char2 == '*') {
                  isBrief = true;

               } else if (qt_auto_brief && char2 == '!') {
                  isBrief = true;

               } else if (char2 == '/' || char2 == '!') {
                  isBrief = true;

               }

               if (comment.startsWith("/**<") || comment.startsWith("/*!<")) {
                  // */ (editor syntax fix)

                  int len = comment.length() - 6;
                  comment = comment.mid(4, len);

               } else {
                  comment = comment.mid(4);

               }

            } else if (comment.startsWith("/**")) {
               // */ (editor syntax fix),   javadoc comment

               int len = comment.length() - 5;
               comment = comment.mid(3, len);

               ++index;

               if (index < p->numTokens) {
                  // get the next cursor, skipping over the current comment
                  cursor = p->cursors[index];
               }

/*             while (index < p->numTokens) {
                  cursor = p->cursors[index];

                  tokenKind = clang_getTokenKind(p->tokens[index]);

                  if (tokenKind == CXToken_Comment) {
                     // next cursor is also a comment, merge
                     QString extra = getTokenSpelling(p->tu, p->tokens[index]).trimmed();

                     int len = extra.length() - 5;
                     extra = extra.mid(3, len);

                     comment += "\n\n" + extra;

                   } else {
                     break;

                   }

                   ++index;
               }
*/

               // skip punctuation and attribute
               uint bracket = 0;

               while (index < p->numTokens) {
                  CXTokenKind tokenKind = clang_getTokenKind(p->tokens[index]);

                  if (tokenKind != CXToken_Punctuation && bracket == 0) {
                     // might be another comment
                     break;
                  }

                  QString extra = getTokenSpelling(p->tu, p->tokens[index]);
                  if (extra == "[") {
                     ++bracket;
                  } else if (extra == "]") {
                     --bracket;
                  }

                  ++index;
                  cursor = p->cursors[index];
               }

               tokenKind = clang_getTokenKind(p->tokens[index]);

               if (tokenKind == CXToken_Comment) {
                  // back up since the loop counter will increment past this comment
                  --index;

               } else {
                  while (index < p->numTokens && ! documentKind(cursor) ) {
                     ++index;
                     cursor = p->cursors[index];
                  }
               }

               // remove single *
               QRegularExpression reg("\n\\s*\\*");
               comment.replace(reg, "\n");

               if (javadoc_auto_brief) {
                  isBrief = true;
               }

            } else if (comment.startsWith("/*!")) {
               // */ (editor syntax fix), qt comment

               int len = comment.length() - 5;
               comment = comment.mid(3, len);

               ++index;

               if (index < p->numTokens) {
                  // get the next cursor, skipping over the current comment
                  cursor = p->cursors[index];
               }

/*             while (index < p->numTokens) {
                  cursor = p->cursors[index];

                  tokenKind = clang_getTokenKind(p->tokens[index]);

                  if (tokenKind == CXToken_Comment) {
                     // next cursor is also a comment, merge
                     QString extra = getTokenSpelling(p->tu, p->tokens[index]).trimmed();

                     int len = extra.length() - 5;
                     extra   = extra.mid(3, len);

                     comment += "\n\n" + extra;

                  } else {
                    break;

                  }

                  ++index;
               }
*/

               // skip punctuation and attribute
               uint bracket = 0;

               while (index < p->numTokens) {
                  CXTokenKind tokenKind = clang_getTokenKind(p->tokens[index]);

                  if (tokenKind != CXToken_Punctuation && bracket == 0) {
                     // might be another comment
                     break;
                  }

                  QString extra = getTokenSpelling(p->tu, p->tokens[index]);

                  if (extra == "[") {
                     ++bracket;

                  } else if (extra == "]") {
                     --bracket;

                  } else if (extra == "::")  {
                     // bail out
                     break;
                  }

                  ++index;
                  cursor = p->cursors[index];
               }

               tokenKind = clang_getTokenKind(p->tokens[index]);

               if (tokenKind == CXToken_Comment) {
                  // back up since the loop counter will increment past this comment
                  --index;

               } else {
                  while (index < p->numTokens && ! documentKind(cursor) ) {
                     ++index;
                     cursor = p->cursors[index];
                  }
               }

               if (qt_auto_brief) {
                  isBrief = true;
               }

            } else if (comment.startsWith("///") || comment.startsWith("//!")  ) {
               // triple slash or //! starts a brief and may include details

               comment = comment.mid(3);
               isBrief = true;

               uint tmpIndex = index + 1;

               while (tmpIndex < p->numTokens)  {
                  // is the next cursor a comment?
                  CXTokenKind tokenKind = clang_getTokenKind(p->tokens[tmpIndex]);

                  if (tokenKind == CXToken_Comment) {
                     ++tmpIndex;

                  } else {
                     cursor = p->cursors[tmpIndex];

                     if (getCursorUSR(cursor).isEmpty()) {
                        QString extra = getTokenSpelling(p->tu, p->tokens[tmpIndex]);

                        if (extra == "#" || extra == "define") {
                           // define

                        } else {
                           CXCursorKind kind = clang_getCursorKind(cursor);

                           if (kind == CXCursor_MacroDefinition || kind == CXCursor_PreprocessingDirective ||
                                       kind == CXCursor_InclusionDirective) {
                              break;
                           }
                        }

                        ++tmpIndex;

                     } else {
                        break;
                     }
                  }
               }

               // skip punctuation and attribute
               uint bracket = 0;

               while (tmpIndex < p->numTokens) {
                  CXTokenKind tokenKind = clang_getTokenKind(p->tokens[tmpIndex]);

                  if (tokenKind != CXToken_Punctuation && bracket == 0) {
                     break;
                  }

                  QString extra = getTokenSpelling(p->tu, p->tokens[tmpIndex]);

                  if (extra == "[") {
                     ++bracket;

                  } else if (extra == "]") {
                     --bracket;

                  } else if (extra == "::")  {
                     // bail out
                     break;
                  }

                  ++tmpIndex;
                  cursor = p->cursors[tmpIndex];

               }

            } else  {
               continue;

            }

            if (! comment.isEmpty()) {
               // test if the cursor is related to something we have parsed

               QString name = getCursorSpelling(cursor);
               QString key  = getCursorUSR(cursor);

               if (key.contains("@macro@")) {
                  // remove any numbers in the key

                  static QRegularExpression regExp("@\\d+@macro@");
                  key.replace(regExp, "@macro@");
               }

               QSharedPointer<Entry> current = s_entryMap.value(key);

               if (current == nullptr) {
                  // documentation does not belong to any source code

                  current = QMakeShared<Entry>();
                  current->m_srcLang = SrcLangExt_Cpp;

                  s_current_root->addSubEntry(current, s_current_root);
               }

               if (current) {
                  handleCommentBlock(comment, false, fileName, current);

                  if (isBrief && current->getData(EntryKey::Brief_Docs).isEmpty()) {
                     QString brief;

                     static QRegularExpression regExp("([^.]*\\.)\\s(.*)",  QPatternOption::ExactMatchOption);
                     QRegularExpressionMatch match = regExp.match(comment);

                     if (match.hasMatch()) {
                        brief   = match.captured(1);
                        comment = match.captured(2);

                     } else {
                        brief   = comment;
                        comment = "";

                     }

                     current->setData(EntryKey::Brief_Docs, brief);
                     current->setData(EntryKey::Main_Docs,  comment);
                  }
               }
            }
         }
      }

   } else {
      p->tokens    = 0;
      p->numTokens = 0;
      p->cursors   = 0;

      if (errorCode == CXError_InvalidArguments ) {
         err("libClang failed to parse file %s, invalid arguments", csPrintable(fileName));

      } else if (errorCode == CXError_ASTReadError ) {
         err("libClang failed to parse file %s, problem in the AST\n", csPrintable(fileName));

      } else {
         err("libClang failed to parse file %s, terminated with error code %d \n", csPrintable(fileName), errorCode);

      }
   }
}

void ClangParser::finish()
{
   if (p->tu) {

      delete[] p->cursors;

      clang_disposeTokens(p->tu, p->tokens, p->numTokens);
      clang_disposeTranslationUnit(p->tu);
      clang_disposeIndex(p->index);

      p->fileMapping.clear();
      p->tokens    = 0;
      p->numTokens = 0;
      p->cursors   = 0;
   }

   for (uint i = 0; i < p->numFiles; i++) {
      free((void *)p->ufs[i].Filename);
   }

   delete[] p->ufs;
   delete[] p->sources;

   p->ufs       = 0;
   p->sources   = 0;
   p->numFiles  = 0;
   p->tu        = 0;
}

static void handleCommentBlock(const QString &comment, bool brief, const QString &fileName, QSharedPointer<Entry> current)
{
   bool docBlockInBody = false;

   int lineNum         = 0;
   int position        = 0;

   bool needsEntry     = false;
   bool isBrief        = false;
   bool isJavaDocStyle = false;

   if (brief) {
      lineNum = current->briefLine;
   } else {
      lineNum = current->docLine;
   }

   static QRegularExpression regExp("^\\s*[\\\\@](class|concept|enum|file|fn|page|var)\\b");
   QRegularExpressionMatch match = regExp.match(comment);

   if (match.hasMatch()) {
      // unique to clang, must pre add a new entry before calling parseCommentBlock

      QSharedPointer<Entry> parent = current->parent();

      current = QMakeShared<Entry>();
      current->m_srcLang = SrcLangExt_Cpp;

      if (parent) {
         parent->addSubEntry(current, parent);
      }
   }

   while (parseCommentBlock(nullptr, current, comment, fileName, lineNum, isBrief, isJavaDocStyle,
                  docBlockInBody, current->protection, position, needsEntry) ) {

      if (needsEntry) {
         QString docFile = current->getData(EntryKey::MainDocs_File);

         QSharedPointer<Entry> parent = current->parent();

         current = QMakeShared<Entry>();
         current->m_srcLang = SrcLangExt_Cpp;

         if (parent) {
            parent->addSubEntry(current, parent);
         }

         current->setData(EntryKey::MainDocs_File, docFile);
         current->docLine = lineNum;
      }
   }
}

// filter the files keeping those which where found as include files within the TU
// files - list of files to filter
void ClangParser::determineInputFiles(QStringList &files)
{
   // save included files used by the translation unit to a container
   clang_getInclusions(p->tu, inclusionVisitor, nullptr);

   // create a new filtered file list
   QStringList resultIncludes;

   for (auto item : files) {
      if (s_includedFiles.contains(item)) {
         resultIncludes.append(item);
      }
   }

   // replace the original list
   files = resultIncludes;
}

QString ClangParser::lookup(uint line, const QString &symbol)
{
   QString retval;

   if (symbol.isEmpty()) {
      return retval;
   }

   int symLen     = symbol.length();
   uint tokenLine = p->getCurrentTokenLine();

   while (tokenLine >= line && p->curToken > 0) {

      if (tokenLine == line) {
         // already at the right line

         p->curToken--;                         // linear search find the beginning of the line
         tokenLine = p->getCurrentTokenLine();

      } else {
         p->curToken /= 2;                      // binary search backwards
         tokenLine = p->getCurrentTokenLine();
      }
   }

   bool found = false;

   while (tokenLine <= line && p->curToken < p->numTokens && ! found) {
      CXString tokenString = clang_getTokenSpelling(p->tu, p->tokens[p->curToken]);

      const char *ts = clang_getCString(tokenString);
      int tl         = strlen(ts);
      int startIndex = p->curToken;

      if (tokenLine == line && strncmp(ts, symbol.toUtf8().constData(), tl) == 0) {
         // found partial match at the correct line
         int offset = tl;

         while (offset < symLen) {
            // symbol spans multiple tokens

            p->curToken++;

            if (p->curToken >= p->numTokens) {
               // end of token stream
               break;
            }

            tokenLine = p->getCurrentTokenLine();
            clang_disposeString(tokenString);

            tokenString = clang_getTokenSpelling(p->tu, p->tokens[p->curToken]);
            ts = clang_getCString(tokenString);
            tl = ts ? strlen(ts) : 0;

            // skip over any spaces in the symbol
            QChar c;

            while (offset < symLen && ((c = symbol[offset]) == ' ' || c == '\t' || c == '\r' || c == '\n')) {
               offset++;
            }

            if (strncmp(ts, symbol.mid(offset).toUtf8().constData(), tl) != 0) {
               // next token does not match
               break;
            }

            offset += tl;
         }

         if (offset == symLen) {
            // symbol matches the token(s)
            CXCursor c   = p->cursors[p->curToken];
            CXString usr = clang_getCursorUSR(c);

            retval = QString::fromLatin1(clang_getCString(usr));

            clang_disposeString(usr);
            found = true;

         } else {
            // reset token cursor to start of the search
            p->curToken = startIndex;
         }
      }

      clang_disposeString(tokenString);
      p->curToken++;

      if (p->curToken < p->numTokens) {
         tokenLine = p->getCurrentTokenLine();
      }
   }

   return retval;
}

void ClangParser::linkInclude(CodeOutputInterface &ol, QSharedPointer<FileDef> fileDef, uint &line,
            uint &column, const QString &text)
{
   (void) fileDef;

   QString incName = text;
   incName = incName.mid(1, incName.length() - 2); // strip ".." or  <..>

   QSharedPointer<FileDef> ifd;

   if (! incName.isEmpty()) {
      QSharedPointer<FileNameList> fn = Doxy_Globals::inputNameDict.find(incName);

      if (fn) {
         bool found = false;

         // for each include name, see if this source file actually includes the file
         for (auto fd : *fn) {

            if (found) {
               break;
            }

            ifd = fd;
            found = fd->isIncluded(ifd->getFilePath());
         }
      }

   }

   if (ifd) {
      ol.writeCodeLink(ifd->getReference(), ifd->getOutputFileBase(), 0, text, ifd->briefDescriptionAsTooltip());

   } else {
      codifyLines(ol, ifd, text, line, column, "preprocessor");
   }
}

void ClangParser::linkMacro(CodeOutputInterface &ol, QSharedPointer<FileDef> fd, uint &line, uint &column, const QString &text)
{
   QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict.find(text);

   if (mn) {
      for (auto md : *mn) {
         if (md->isDefine()) {
            writeMultiLineCodeLink(ol, fd, line, column, md, text);
            return;
         }

      }
   }

   codifyLines(ol, fd, text, line, column, "");
}

void ClangParser::linkIdentifier(CodeOutputInterface &ol, QSharedPointer<FileDef> fd,
                                 uint &line, uint &column, const QString &text, int tokenIndex)
{
   CXCursor c = p->cursors[tokenIndex];
   CXCursor r = clang_getCursorReferenced(c);

   if (! clang_equalCursors(r, c)) {
      // link to referenced location
      c = r;
   }

   CXCursor t = clang_getSpecializedCursorTemplate(c);

   if (! clang_Cursor_isNull(t) && !clang_equalCursors(t, c)) {
      // link to template
      c = t;
   }

   CXString usr   = clang_getCursorUSR(c);
   QString usrStr = QString::fromLatin1(clang_getCString(usr));

   QSharedPointer<Definition> d;

   if (! usrStr.isEmpty()) {
      d = Doxy_Globals::clangUsrMap.value(usrStr);
   }

   if (d && d->isLinkable()) {
      if (g_insideBody && s_currentMemberDef && d->definitionType() == Definition::TypeMember &&
            (s_currentMemberDef != d || g_currentLine < line)) {

         // avoid self-reference
         addDocCrossReference(s_currentMemberDef, d.dynamicCast<MemberDef>());
      }

      writeMultiLineCodeLink(ol, fd, line, column, d, text);

   } else {
      codifyLines(ol, fd, text, line, column, "");
   }

   clang_disposeString(usr);
}

void ClangParser::switchToFile(const QString &fileName)
{
   if (p->tu) {
      delete[] p->cursors;

      clang_disposeTokens(p->tu, p->tokens, p->numTokens);

      p->tokens    = 0;
      p->numTokens = 0;
      p->cursors   = 0;

      QFileInfo fi(fileName);
      CXFile f = clang_getFile(p->tu, fileName.toUtf8().constData());

      uint pIndex = p->fileMapping.value(fileName);

      if (pIndex < p->numFiles) {
         uint i = pIndex;

         CXSourceLocation fileBegin = clang_getLocationForOffset(p->tu, f, 0);
         CXSourceLocation fileEnd   = clang_getLocationForOffset(p->tu, f, p->ufs[i].Length);
         CXSourceRange    fileRange = clang_getRange(fileBegin, fileEnd);

         clang_tokenize(p->tu, fileRange, &p->tokens, &p->numTokens);

         p->cursors = new CXCursor[p->numTokens];
         clang_annotateTokens(p->tu, p->tokens, p->numTokens, p->cursors);

         p->curLine  = 1;
         p->curToken = 0;

      } else {
         err("Clang failed to find input file %s\n", csPrintable(fileName));

      }
   }
}

void ClangParser::writeSources(CodeOutputInterface &ol, QSharedPointer<FileDef> fd)
{
   static const bool stripCodeComments = Config::getBool("strip-code-comments");

   TooltipManager::instance()->clearTooltips();

   // set global parser state
   s_currentDefinition = QSharedPointer<Definition>();
   s_currentMemberDef  = QSharedPointer<MemberDef>();
   g_currentLine       = 0;
   g_searchForBody     = false;
   g_insideBody        = false;
   g_bracketCount      = 0;

   uint line   = 1;
   uint column = 1;

   QString lineNumber;
   QString lineAnchor;

   ol.startCodeLine(true);
   writeLineNumber(ol, fd, line);

   for (uint i = 0; i < p->numTokens; i++) {
      CXSourceLocation start = clang_getTokenLocation(p->tu, p->tokens[i]);

      uint t_line;
      uint t_col;

      clang_getSpellingLocation(start, 0, &t_line, &t_col, 0);

      if (t_line > line) {
         column = 1;
      }

      while (line < t_line) {
         ++line;

         ol.endCodeLine();

         ol.startCodeLine(true);
         writeLineNumber(ol, fd, line);
      }

      while (column < t_col) {
         ol.codify(" ");
         column++;
      }

      QString text = getTokenSpelling(p->tu, p->tokens[i]);

      CXCursorKind cursorKind  = clang_getCursorKind(p->cursors[i]);
      CXTokenKind tokenKind    = clang_getTokenKind(p->tokens[i]);

      switch (tokenKind) {
         case CXToken_Keyword:
            if (text == "operator") {
               linkIdentifier(ol, fd, line, column, text, i);

            } else {
               QString cssClass;

               if (cursorKind == CXCursor_PreprocessingDirective) {
                  cssClass = "preprocessor";

               } else {
                  cssClass = keywordToType(text);
               }

               codifyLines(ol, fd, text, line, column, cssClass);
            }
            break;

         case CXToken_Literal:
            if (cursorKind == CXCursor_InclusionDirective) {
               linkInclude(ol, fd, line, column, text);

            } else if (text[0] == '"' || text[0] == '\'') {
               codifyLines(ol, fd, text, line, column, "stringliteral");

            } else {
               codifyLines(ol, fd, text, line, column, "");

            }
            break;

         case CXToken_Comment:
            {
               bool omit = text.startsWith("/**") || text.startsWith("///") ||
                           text.startsWith("/*!") || text.startsWith("//!");

               // */ (editor syntax fix)

               if (stripCodeComments && omit && ! text.startsWith("/**********") ) {
                  // */ (editor syntax fix)

                  int cnt = 1;

                  // consume the comment
                  for (QChar c : text) {
                     ++column;

                     if (c == '\n')  {
                        ++line;
                        column = 0;

                        ++cnt;
                     }
                  }

                  ol.startFontClass("comment");
                  ol.codify("// " + QString::number(cnt) + " comment line(s) omitted");
                  ol.endFontClass();

               } else {
                  // output comments like the file license
                  codifyLines(ol, fd, text, line, column, "comment");

               }
            }

            break;

         default:
            // CXToken_Punctuation or CXToken_Identifier

            if (tokenKind == CXToken_Punctuation) {
               detectFunctionBody(text);
            }

            switch (cursorKind) {
               case CXCursor_PreprocessingDirective:
                  codifyLines(ol, fd, text, line, column, "preprocessor");
                  break;

               case CXCursor_MacroDefinition:
                  codifyLines(ol, fd, text, line, column, "preprocessor");
                  break;

               case CXCursor_InclusionDirective:
                  linkInclude(ol, fd, line, column, text);
                  break;

               case CXCursor_MacroExpansion:
                  linkMacro(ol, fd, line, column, text);
                  break;

               default:
                  if (tokenKind == CXToken_Identifier || (tokenKind == CXToken_Punctuation &&
                         (cursorKind == CXCursor_DeclRefExpr || cursorKind == CXCursor_MemberRefExpr ||
                          cursorKind == CXCursor_CallExpr || cursorKind == CXCursor_ObjCMessageExpr)) ) {

                     linkIdentifier(ol, fd, line, column, text, i);

                     if (Doxy_Globals::searchIndexBase != nullptr) {
                        ol.addWord(text, false);
                     }

                  } else {
                     codifyLines(ol, fd, text, line, column, "");
                  }

                  break;
            }
      }
   }

   ol.endCodeLine();
   TooltipManager::instance()->writeTooltips(ol);
}
