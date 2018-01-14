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

#include <QByteArray>
#include <QHash>
#include <QSet>

#include <stdio.h>
#include <stdlib.h>

#include <parse_clang.h>

#include <commentscan.h>
#include <config.h>
#include <doxy_globals.h>
#include <entry.h>
#include <message.h>
#include <outputgen.h>
#include <parse_lib_tooling.h>
#include <qfileinfo.h>
#include <stringmap.h>
#include <tooltip.h>
#include <util.h>

static void handleCommentBlock(const QString &comment, bool brief, const QString &fileName, QSharedPointer<Entry> current);

static QSet<QString>                 s_includedFiles;
static QSharedPointer<Definition>    s_currentDefinition;
static QSharedPointer<MemberDef>     s_currentMemberDef;

static uint g_currentLine    = 0;
static uint g_bracketCount   = 0;
static bool g_searchForBody  = false;
static bool g_insideBody     = false;

QSharedPointer<Entry>                s_current_root;
QMap<QString, QSharedPointer<Entry>> s_entryMap;

static void writeLineNumber(CodeOutputInterface &ol, QSharedPointer<FileDef> fd, uint line);

class ClangParser::Private
{
 public:
   enum DetectedLang { Detected_Cpp, Detected_ObjC, Detected_ObjCpp };

   Private()
      : tu(0), tokens(0), numTokens(0), cursors(0), ufs(0), sources(0), numFiles(0),
        fileMapping(), detectedLang(Detected_Cpp)
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
   p = new Private;
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
         column = len + 1;

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
   QString retval = clang_getCString(text);

   clang_disposeString(text);

   return retval;
}

static QString getCursorUSR(CXCursor cursor)
{
   CXString text  = clang_getCursorUSR(cursor);
   QString retval = clang_getCString(text);

   clang_disposeString(text);

   return retval;
}

static QString getFileName(CXFile file)
{
   CXString text  = clang_getFileName(file);
   QString retval = clang_getCString(text);

   clang_disposeString(text);

   return retval;
}

static QString getTokenSpelling(CXTranslationUnit tu, CXToken token)
{
   CXString text   = clang_getTokenSpelling(tu, token);
   QString  retval = clang_getCString(text);

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
      keyWords.insert("@try",     "keywordflow");
      keyWords.insert("@catch",   "keywordflow");
      keyWords.insert("@finally", "keywordflow");

      keyWords.insert("bool",     "keywordtype");
      keyWords.insert("char",     "keywordtype");
      keyWords.insert("double",   "keywordtype");
      keyWords.insert("float",    "keywordtype");
      keyWords.insert("int",      "keywordtype");
      keyWords.insert("long",     "keywordtype");
      keyWords.insert("object",   "keywordtype");
      keyWords.insert("short",    "keywordtype");
      keyWords.insert("signed",   "keywordtype");
      keyWords.insert("unsigned", "keywordtype");
      keyWords.insert("void",     "keywordtype");
      keyWords.insert("wchar_t",  "keywordtype");
      keyWords.insert("size_t",   "keywordtype");
      keyWords.insert("boolean",  "keywordtype");
      keyWords.insert("id",       "keywordtype");
      keyWords.insert("SEL",      "keywordtype");
      keyWords.insert("string",   "keywordtype");
      keyWords.insert("nullptr",  "keywordtype");

      init = false;
   }

   auto iter = keyWords.find(key);

   if (iter != keyWords.end()) {
      return iter.value();
   }

   return "keyword";
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
      ol.writeLineNumber(0, 0, 0, line);
   }

   // set search page target
   if (Doxy_Globals::searchIndexBase != nullptr) {
      QString lineAnchor = QString("l%1").arg(line, 5, 10, QChar('0'));
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
   s_includedFiles.insert(getFileName(file));
}

static bool documentKind(CXCursor cursor)
{
   CXCursorKind kind = clang_getCursorKind(cursor);

   if (kind == CXCursor_FunctionDecl       ||
       kind == CXCursor_FunctionTemplate   ||
       kind == CXCursor_ClassDecl          ||
       kind == CXCursor_ClassTemplate      ||
       kind == CXCursor_StructDecl         ||
       kind == CXCursor_Constructor        ||
       kind == CXCursor_Destructor         ||
       kind == CXCursor_ConversionFunction ||
       kind == CXCursor_UnionDecl          ||
       kind == CXCursor_EnumDecl           ||
       kind == CXCursor_EnumConstantDecl   ||
       kind == CXCursor_FieldDecl          ||
       kind == CXCursor_ParmDecl           ||
       kind == CXCursor_VarDecl            ||
       kind == CXCursor_CXXMethod          ||
       kind == CXCursor_TypedefDecl        ||
       kind == CXCursor_TypeAliasDecl      ||
       kind == CXCursor_UnexposedDecl      ||
       kind == CXCursor_MacroDefinition    ||
       kind == CXCursor_Namespace    )  {

      return true;
   }

   return false;
}

// ** entry point
void ClangParser::start(const QString &fileName, const QString &fileBuffer, QStringList &includeFiles, QSharedPointer<Entry> root)
{
   static QStringList const includePath      = Config::getList("include-path");
   static QStringList const clangFlags       = Config::getList("clang-flags");
   static QStringList const preDefinedMacros = Config::getList("predefined-macros");

   static std::vector<std::string> clangCmdArgs;
   std::vector<std::string> argList;

   if (clangCmdArgs.empty()) {

      // add include paths for input files
      for (auto &item : Doxy_Globals::inputPaths) {
         std::string inc = std::string("-I") + item.toUtf8().constData();
         argList.push_back(std::move(inc));
      }

      // add external include paths
      for (auto &item : includePath) {
         std::string inc = std::string("-I") + item.toUtf8().constData();
         argList.push_back(std::move(inc));
      }

      // add predefinded macros
      for (const auto &item : preDefinedMacros) {
         std::string macro = std::string("-D") + item.toUtf8().constData();
         argList.push_back(std::move(macro));
      }

      // user specified flags
      for (auto &item : clangFlags) {
         argList.push_back(item.toUtf8().constData());
      }

      argList.push_back("-ferror-limit=0");
      argList.push_back("-x");

      // save for the next pass
      clangCmdArgs = argList;

   } else {
      // just copy the first x args since they do not change
      argList = clangCmdArgs;

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

   argList.push_back(fileName.toUtf8().constData());

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

   // what is in argList ( check this one more time )
   // data structure, source filename (in argv), cmd line args, num of cmd line args
   // pass include files, num of unsaved files, clang flag, trans unit (file name)

   std::vector<const char *> argv;
   for (auto &item : argList) {
      argv.push_back(item.c_str());
   }

   int argc = argList.size();

   // libClang - used to set up the tokens for comments
   CXErrorCode errorCode = clang_parseTranslationUnit2(p->index, 0, &argv[0], argc, p->ufs, numUnsavedFiles,
                  CXTranslationUnit_DetailedPreprocessingRecord, &(p->tu) );

   if (p->tu && errorCode == CXError_Success) {
      // filter out any includes not found by the clang parser
      determineInputFiles(includeFiles);

      // show any warnings the compiler produced
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

      if (root == nullptr)  {
         // called from writeSouce in fileDef

      } else {
         // libTooling - used to parse the source

         // start adding to entry
         s_current_root = root;
         s_entryMap.insert("TranslationUnit", root);

         // strip out the fileName
         argList.erase(argList.end() - 1);

         // pass all arguments except the file name, which was just removed
         clang::tooling::FixedCompilationDatabase options(".", argList);

         const std::string tmpFName = fileName.toUtf8().constData();

         // pass the main file to parse
         std::vector<std::string> sourceList;
         sourceList.push_back(tmpFName);

         // create a new clang tooling instance
         clang::tooling::ClangTool tool(options, sourceList);

         // use the file in memory
         tool.mapVirtualFile(tmpFName, p->sources[0].constData());

         // run the clang tooling to create a new FrontendAction
         int result = tool.run(clang::tooling::newFrontendActionFactory<DoxyFrontEnd>().get());
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
         // called from writeSouce in fileDef
         return;
      }

      static const bool javadoc_auto_brief = Config::getBool("javadoc-auto-brief");
      static const bool qt_auto_brief      = Config::getBool("qt-auto-brief");

      // walk the tokens
      for (int index = 0; index < p->numTokens; ++index)  {

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

               while (index < p->numTokens) {
                  // skip over the cursor comment
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

               while (index < p->numTokens && ! documentKind(cursor) ) {
                  cursor = p->cursors[++index];
               }

               // remove single *
               QRegExp reg("\n\\s*\\*");
               comment.replace(reg, "\n");

               if (javadoc_auto_brief) {
                  isBrief = true;
               }

            } else if (comment.startsWith("/*!")) {
               // */ (editor syntax fix),   qt comment

               int len = comment.length() - 5;
               comment = comment.mid(3, len);

               ++index;

               while (index < p->numTokens) {
                  // skip over the cursor comment
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

               while (index < p->numTokens && ! documentKind(cursor) ) {
                  cursor = p->cursors[++index];
               }

               if (qt_auto_brief) {
                  isBrief = true;
               }

            } else if (comment.startsWith("///") || comment.startsWith("//!")  ) {
               // triple slash or //!  which is always a brief

               comment = comment.mid(3);
               isBrief = true;

               int tmpIndex = index + 1;

               while (tmpIndex < p->numTokens)  {
                  // is this next cursor a comment?
                  CXTokenKind tokenKind = clang_getTokenKind(p->tokens[tmpIndex]);

                  if (tokenKind == CXToken_Comment) {
                     tmpIndex++;

                  }  else {
                     cursor = p->cursors[tmpIndex];
                     break;
                  }
               }

            } else  {
               continue;

            }

            // test if the cursor is related to documentation
            if (documentKind(cursor)) {

               // add the comment to cursor
               QString name = getCursorSpelling(cursor);
               QString key  = getCursorUSR(cursor);

               QSharedPointer<Entry> current = s_entryMap.value(key);

               if (current) {
                  handleCommentBlock(comment, false, fileName, current);

                  if (isBrief && current->getData(EntryKey::Brief_Docs).isEmpty()) {
                     QString brief;
                     QRegExp reg("([^.]*\\.)\\s(.*)");

                     if (reg.exactMatch(comment)) {
                        brief   = reg.cap(1);
                        comment = reg.cap(2);

                     } else {
                        brief   = comment;
                        comment = "";
                     }

                     current->setData(EntryKey::Brief_Docs, brief);
                  }
               }
            }
         }
      }

   } else {
      p->tokens    = 0;
      p->numTokens = 0;
      p->cursors   = 0;

      err("Clang failed to parse file %s\n", csPrintable(fileName));
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
   static bool hideInBodyDocs = Config::getBool("hide-in-body-docs");
   bool docBlockInBody = false;

/*
   if (docBlockInBody && hideInBodyDocs) {
      return;
   }
*/

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

/*
   if (! docBlockInBody) {
      isBrief = brief;

      bool docBlockAutoBrief = ( tmpChar == '*' && javadoc_auto_brief ) || ( tmpChar == '!' && qt_auto_brief );
      isJavaDocStyle = docBlockAutoBrief;
   }

   QSharedPointer<Entry> docEntry = docBlockInBody && previous ? previous : current;

   if (docBlockInBody && docEntry && docEntry->inbodyLine == -1) {
      docEntry->inbodyFile = fileName;
      docEntry->inbodyLine = lineNum;
   }
*/

   QSharedPointer<Entry> docEntry = current;

   while (parseCommentBlock(nullptr, docEntry, comment, fileName, lineNum, isBrief, isJavaDocStyle,
                  docBlockInBody, current->protection, position, needsEntry) ) {

      if (needsEntry) {
         QString docFile = current->getData(EntryKey::MainDocs_File);

         QSharedPointer<Entry> parent = current->parent();
         current = QMakeShared<Entry>();

         // resolve better by adding to orphan map
         if (parent) {
            parent->addSubEntry(current, parent);
         }

         current->setData(EntryKey::MainDocs_File, docFile);
         current->docLine = lineNum;

         docEntry = current;
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

         p->curToken--;                   // linear search to start of the line
         tokenLine = p->getCurrentTokenLine();

      } else {
         p->curToken /= 2;                // binary search backward
         tokenLine = p->getCurrentTokenLine();
      }
   }

   bool found = false;

   while (tokenLine <= line && p->curToken < p->numTokens && !found) {
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

            retval = clang_getCString(usr);
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

void ClangParser::linkInclude(CodeOutputInterface &ol, QSharedPointer<FileDef> fd, uint &line, uint &column, const QString &text)
{
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

   CXString usr = clang_getCursorUSR(c);
   const char *usrStr = clang_getCString(usr);

   QSharedPointer<Definition> d;

   if (usrStr) {
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
         line++;
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
               QString temp;

               if (cursorKind == CXCursor_PreprocessingDirective) {
                  temp = "preprocessor";

               } else {
                  temp = keywordToType(text);

               }

               codifyLines(ol, fd, text, line, column, temp);
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

                  for (QChar c : text) {
                     ++column;

                     if (c == '\n')  {
                        ++line;
                        column = 0;
                     }
                  }

               } else {
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

