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

#include <QHash>
#include <QSet>

#include <stdio.h>
#include <stdlib.h>

#include <clang-c/Index.h>

#include <config.h>
#include <doxy_globals.h>
#include <filedef.h>
#include <filename.h>
#include <growbuf.h>
#include <message.h>
#include <memberdef.h>
#include <membername.h>
#include <outputgen.h>
#include <parser_clang.h>
#include <qfileinfo.h>
#include <stringmap.h>
#include <tooltip.h>
#include <util.h>

static QSharedPointer<Definition> g_currentDefinition;
static QSharedPointer<MemberDef>  g_currentMemberDef;

static uint g_currentLine       = 0;
static bool g_searchForBody     = false;
static bool g_insideBody        = false;
static uint g_bracketCount      = 0;

ClangParser *ClangParser::instance()
{
   if (! s_instance) {
      s_instance = new ClangParser;
   }

   return s_instance;
}

ClangParser *ClangParser::s_instance = 0;

class ClangParser::Private
{
 public:
   enum DetectedLang { Detected_Cpp, Detected_ObjC, Detected_ObjCpp };

   Private() 
      : tu(0), tokens(0), numTokens(0), cursors(0), ufs(0), sources(0), numFiles(0), fileMapping(), detectedLang(Detected_Cpp) 
   {      
   }

   int getCurrentTokenLine();

   QByteArray fileName;
   QByteArray *sources;

   uint numFiles;
   uint numTokens;
   uint curLine;
   uint curToken;    

   CXIndex index;
   CXTranslationUnit tu; 
   CXToken *tokens;   
   CXCursor *cursors;  
   CXUnsavedFile *ufs;      

   QHash<QString, uint> fileMapping;
   DetectedLang detectedLang;
};

static QByteArray detab(const QByteArray &s)
{
   static int tabSize = Config::getInt("tab-size");

   GrowBuf out;
   int size = s.length();
   const char *data = s.data();

   int i   = 0;
   int col = 0;

   const int maxIndent = 1000000; // value representing infinity
   int minIndent = maxIndent;

   while (i < size) {
      char c = data[i++];

      switch (c) {

         case '\t': {
            // expand tab
            int stop = tabSize - (col % tabSize);           
            col += stop;

            while (stop--) {
               out.addChar(' ');
            }
         }
         break;

         case '\n': 
            // reset colomn counter
            out.addChar(c);
            col = 0;
            break;

         case ' ': 
            // increment column counter
            out.addChar(c);
            col++;
            break;

         default: 
            // non-whitespace => update minIndent
            out.addChar(c);

            if (c < 0 && i < size) { 
               // multibyte sequence

               out.addChar(data[i++]); // >= 2 bytes

               if (((uchar)c & 0xE0) == 0xE0 && i < size) {
                  out.addChar(data[i++]); // 3 bytes
               }

               if (((uchar)c & 0xF0) == 0xF0 && i < size) {
                  out.addChar(data[i++]); // 4 byres
               }
            }

            if (col < minIndent) {
               minIndent = col;
            }
            col++;
      }
   }

   out.addChar(0);
  
   return out.get();
}

/** Callback function called for each include in a translation unit */
static void inclusionVisitor(CXFile includedFile, CXSourceLocation * , unsigned, CXClientData clientData)
{
   QSet<QString> *fileDict = reinterpret_cast<QSet<QString> *>(clientData);

   CXString incFileName = clang_getFileName(includedFile);
 
   fileDict->insert(clang_getCString(incFileName));
   clang_disposeString(incFileName);
}

/** filter the \a files and only keep those that are found as include files
 *  within the current translation unit.
 *  @param[in,out] files The list of files to filter.
 */
void ClangParser::determineInputFiles(QStringList &files)
{
   // put the files in this translation unit in a dictionary
   QSet<QString> incFound;

   clang_getInclusions(p->tu, inclusionVisitor, (CXClientData) &incFound);

   // create a new filtered file list
   QStringList resultIncludes;
 
   for (auto item : files) {
      if (incFound.contains(item)) {
         resultIncludes.append(item);
      }
   }

   // replace the original list
   files = resultIncludes;
}

void ClangParser::start(const char *fileName, QStringList &includeFiles)
{
   static QStringList includePath = Config::getList("include-path");    
   static QStringList clangFlags  = Config::getList("clang-flags");

   p->fileName = fileName;
   p->index    = clang_createIndex(0, 0);
   p->curLine  = 1;
   p->curToken = 0;

   char **argv = (char **)malloc(sizeof(char *) * (4 + Doxy_Globals::inputPaths.count() + includePath.count() + clangFlags.count()));
   int argc = 0;

   // add include paths for input files  
   for (auto item : Doxy_Globals::inputPaths) { 
      QString inc = "-I" + item;
      argv[argc] = strdup(inc.toUtf8());  

      ++argc;
   }

   // add external include paths
   for (uint i = 0; i < includePath.count(); i++) {  
      QString inc = "-I" + includePath.at(i);
      argv[argc++] = strdup(inc.toUtf8());  
   }

   // user specified options
   for (uint i = 0; i < clangFlags.count(); i++) {
      argv[argc++] = strdup(clangFlags.at(i).toUtf8());
   }

   // extra options
   argv[argc++] = strdup("-ferror-limit=0");
   argv[argc++] = strdup("-x");

   // Since we can be presented with an .h file that can contain C, C++, or Objective C,
   // we need to configure the parser before knowing this.
   // Use the source file to detected the language. Detection will fail if you
   // pass a bunch of .h files containing ObjC code and no source 

   SrcLangExt lang = getLanguageFromFileName(fileName);

   if (lang == SrcLangExt_ObjC || p->detectedLang != ClangParser::Private::Detected_Cpp) {
      QByteArray fn = fileName;

      if (p->detectedLang == ClangParser::Private::Detected_Cpp &&
            (fn.right(4).toLower() == ".cpp" || fn.right(4).toLower() == ".cxx" ||
             fn.right(3).toLower() == ".cc" || fn.right(2).toLower() == ".c")) {

         // fall back to C/C++ once we see an extension that indicates this
         p->detectedLang = ClangParser::Private::Detected_Cpp;

      } else if (fn.right(3).toLower() == ".mm") { 
         // switch to Objective C++
         p->detectedLang = ClangParser::Private::Detected_ObjCpp;

      } else if (fn.right(2).toLower() == ".m") { 
         // switch to Objective C
         p->detectedLang = ClangParser::Private::Detected_ObjC;
      }
   }

   switch (p->detectedLang) {
      case ClangParser::Private::Detected_Cpp:
         argv[argc++] = strdup("c++");
         break;

      case ClangParser::Private::Detected_ObjC:
         argv[argc++] = strdup("objective-c");
         break;

      case ClangParser::Private::Detected_ObjCpp:
         argv[argc++] = strdup("objective-c++");
         break;
   }

   // provide the input and and its dependencies as unsaved files so we can pass the filtered versions
   static bool filterSourceFiles = Config::getBool("filter-source-files");

   argv[argc++] = strdup(fileName);
 
   uint numUnsavedFiles = includeFiles.count() + 1;

   p->numFiles = numUnsavedFiles;
   p->sources  = new QByteArray[numUnsavedFiles];
   p->ufs      = new CXUnsavedFile[numUnsavedFiles];

   p->sources[0]      = detab(fileToString(fileName, filterSourceFiles, true));
   p->ufs[0].Filename = strdup(fileName);
   p->ufs[0].Contents = p->sources[0].constData();
   p->ufs[0].Length   = p->sources[0].length();

   //  
   uint i = 1;
   for (auto item : includeFiles) {       

      p->fileMapping.insert(item, i);

      p->sources[i]      = detab(fileToString(item, filterSourceFiles, true));
      p->ufs[i].Filename = strdup(item.toUtf8());
      p->ufs[i].Contents = p->sources[i].constData();
      p->ufs[i].Length   = p->sources[i].length();

      i++;
   }

   // let libclang do the actual parsing
   CXErrorCode errorCode = clang_parseTranslationUnit2(p->index, 0, argv, argc, 0, 0, 
                  CXTranslationUnit_DetailedPreprocessingRecord, &(p->tu) );

   // free arguments
   for (int i = 0; i < argc; ++i) {
      free(argv[i]);
   }
   free(argv);

   if (p->tu) {
      // filter out any includes not found by the clang parser
      determineInputFiles(includeFiles);

      // show any warnings the compiler produced
      uint n = clang_getNumDiagnostics(p->tu);

      for (uint i = 0; i != n; ++i) {
         CXDiagnostic diag = clang_getDiagnostic(p->tu, i);
         CXString string   = clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());

         err("Clang parser warning -- %s\n", clang_getCString(string));
         clang_disposeString(string);

         clang_disposeDiagnostic(diag);
      }

      // create a source range for the given file
      QFileInfo fi(fileName);
      CXFile f = clang_getFile(p->tu, fileName);

      CXSourceLocation fileBegin = clang_getLocationForOffset(p->tu, f, 0);
      CXSourceLocation fileEnd   = clang_getLocationForOffset(p->tu, f, p->ufs[0].Length);
      CXSourceRange    fileRange = clang_getRange(fileBegin, fileEnd);

      // produce a token stream for the file
      clang_tokenize(p->tu, fileRange, &p->tokens, &p->numTokens);

      // produce cursors for each token in the stream
      p->cursors = new CXCursor[p->numTokens];
      clang_annotateTokens(p->tu, p->tokens, p->numTokens, p->cursors);

   } else {
      p->tokens    = 0;
      p->numTokens = 0;
      p->cursors   = 0;

      err("Clang failed to parse translation unit -- %s\n", fileName);
   }
}

void ClangParser::switchToFile(const char *fileName)
{
   if (p->tu) {
      delete[] p->cursors;

      clang_disposeTokens(p->tu, p->tokens, p->numTokens);

      p->tokens    = 0;
      p->numTokens = 0;
      p->cursors   = 0;

      QFileInfo fi(fileName);
      CXFile f = clang_getFile(p->tu, fileName);

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
         err("Clang: Failed to find input file %s in mapping\n", fileName);
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

int ClangParser::Private::getCurrentTokenLine()
{
   uint l, c;
   if (numTokens == 0) {
      return 1;
   }

   // guard against filters that reduce the number of lines
   if (curToken >= numTokens) {
      curToken = numTokens - 1;
   }

   CXSourceLocation start = clang_getTokenLocation(tu, tokens[curToken]);
   clang_getSpellingLocation(start, 0, &l, &c, 0);
   return l;
}

QByteArray ClangParser::lookup(uint line, const char *symbol)
{
   QByteArray result;

   if (symbol == 0) {
      return result;
   }

   int sl = strlen(symbol);
   uint l = p->getCurrentTokenLine();

   while (l >= line && p->curToken > 0) {

      if (l == line) { 
         // already at the right line

         p->curToken--; // linear search to start of the line
         l = p->getCurrentTokenLine();

      } else {
         p->curToken /= 2; // binary search backward
         l = p->getCurrentTokenLine();
      }
   }

   bool found = false;

   while (l <= line && p->curToken < p->numTokens && !found) {
      CXString tokenString = clang_getTokenSpelling(p->tu, p->tokens[p->curToken]);
     
      const char *ts = clang_getCString(tokenString);
      int tl = strlen(ts);
      int startIndex = p->curToken;

      if (l == line && strncmp(ts, symbol, tl) == 0) { 
         // found partial match at the correct line
         int offset = tl;

         while (offset < sl) { 
            // symbol spans multiple tokens

            p->curToken++;

            if (p->curToken >= p->numTokens) {
               break; // end of token stream
            }

            l = p->getCurrentTokenLine();
            clang_disposeString(tokenString);
            tokenString = clang_getTokenSpelling(p->tu, p->tokens[p->curToken]);
            ts = clang_getCString(tokenString);
            tl = ts ? strlen(ts) : 0;

            // skip over any spaces in the symbol
            char c;
            while (offset < sl && ((c = symbol[offset]) == ' ' || c == '\t' || c == '\r' || c == '\n')) {
               offset++;
            }

            if (strncmp(ts, symbol + offset, tl) != 0) { 
               // next token matches?

               break; // no match
            }

            offset += tl;
         }

         if (offset == sl) { 
            // symbol matches the token(s)
            CXCursor c = p->cursors[p->curToken];
            CXString usr = clang_getCursorUSR(c);
           
            result = clang_getCString(usr);
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
         l = p->getCurrentTokenLine();
      }
   }
  
   return result;
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
      g_currentDefinition = d;
      g_currentLine = line;

      QSharedPointer<MemberDef> md = fd->getSourceMember(line);

      if (md && md->isLinkable()) { 
         // link to member

         if (g_currentMemberDef != md) { 
            // new member, start search for body

            g_searchForBody = true;
            g_insideBody    = false;
            g_bracketCount  = 0;
         }

         g_currentMemberDef = md;
         ol.writeLineNumber(md->getReference(), md->getOutputFileBase(), md->anchor(), line);

      } else { 
         // link to compound
         g_currentMemberDef = QSharedPointer<MemberDef>();
         ol.writeLineNumber(d->getReference(), d->getOutputFileBase(), d->anchor(), line);
      }

   } else { 
      // no link
      ol.writeLineNumber(0, 0, 0, line);
   }

   // set search page target
   if (Doxy_Globals::searchIndex) {          
      QString lineAnchor = QString("l%1").arg(line, 5, 10, QChar('0'));

      ol.setCurrentDoc(fd, lineAnchor.toUtf8(), true);
   }
}

static void codifyLines(CodeOutputInterface &ol, QSharedPointer<FileDef> fd, const char *text,
                        uint &line, uint &column, const QString &fontClass)
{
   if (! fontClass.isEmpty()) {
      ol.startFontClass(fontClass.toUtf8());
   }

   const char *p  = text;
   const char *sp = p;
   char c;
   bool done = false;

   while (! done) {
      sp = p;

      while ((c = *p++) && c != '\n') {
         column++;
      }

      if (c == '\n') {
         line++;
         int l = (int)(p - sp - 1);
         column = l + 1;

         char *tmp = (char *)malloc(l + 1);
         memcpy(tmp, sp, l);

         tmp[l] = '\0';
         ol.codify(tmp);
         free(tmp);

         if (! fontClass.isEmpty()) {
            ol.endFontClass();
         }

         ol.endCodeLine();
         ol.startCodeLine(true);
         writeLineNumber(ol, fd, line);

         if (! fontClass.isEmpty()) {
            ol.startFontClass(fontClass.toUtf8());
         }

      } else {
         ol.codify(sp);
         done = true;
      }
   }

  if (! fontClass.isEmpty()) {
      ol.endFontClass();
   }
}

static void writeMultiLineCodeLink(CodeOutputInterface &ol, QSharedPointer<FileDef> fd, uint &line, uint &column, 
                  QSharedPointer<Definition> d, const char *text)
{
   static bool sourceTooltips = Config::getBool("source-tooltips");
   TooltipManager::instance()->addTooltip(d);

   QByteArray ref    = d->getReference();
   QByteArray file   = d->getOutputFileBase();
   QByteArray anchor = d->anchor();

   QByteArray tooltip;

   if (! sourceTooltips) { 
      // fall back to simple "title" tooltips
      tooltip = d->briefDescriptionAsTooltip();
   }

   bool done = false;
   char *p = (char *)text;

   while (!done) {
      char *sp = p;
      char c;

      while ((c = *p++) && c != '\n') {
         column++;
      }

      if (c == '\n') {
         line++;
         *(p - 1) = '\0';

         ol.writeCodeLink(ref, file, anchor, sp, tooltip);
         ol.endCodeLine();
         ol.startCodeLine(true);
         writeLineNumber(ol, fd, line);

      } else {

         ol.writeCodeLink(ref, file, anchor, sp, tooltip);
         done = true;
      }
   }
}

void ClangParser::linkInclude(CodeOutputInterface &ol, QSharedPointer<FileDef> fd, uint &line, uint &column, const char *text)
{
   QByteArray incName = text;
   incName = incName.mid(1, incName.length() - 2); // strip ".." or  <..>

   QSharedPointer<FileDef> ifd;

   if (! incName.isEmpty()) {     
      QSharedPointer<FileName> fn = Doxy_Globals::inputNameDict->find(incName);

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

void ClangParser::linkMacro(CodeOutputInterface &ol, QSharedPointer<FileDef> fd, uint &line, uint &column, const char *text)
{
   QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict->find(text);
  
   if (mn) {

//       for (auto iter = mn->begin(); iter != mn->end(); ++iter) { 

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
                                 uint &line, uint &column, const char *text, int tokenIndex)
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
      if (g_insideBody && g_currentMemberDef && d->definitionType() == Definition::TypeMember &&
            (g_currentMemberDef != d || g_currentLine < line)) { 

         // avoid self-reference
         addDocCrossReference(g_currentMemberDef, d.dynamicCast<MemberDef>());
      }

      writeMultiLineCodeLink(ol, fd, line, column, d, text);

   } else {
      codifyLines(ol, fd, text, line, column, "");
   }

   clang_disposeString(usr);
}

static void detectFunctionBody(const char *s)
{
 
   if (g_searchForBody && (qstrcmp(s, ":") == 0 || qstrcmp(s, "{") == 0)) { 
      // start of 'body' (: is for constructor)
      g_searchForBody = false;
      g_insideBody    = true;

   } else if (g_searchForBody && qstrcmp(s, ";") == 0) { 
      // declaration only
      g_searchForBody = false;
      g_insideBody    = false;
   }

   if (g_insideBody && qstrcmp(s, "{") == 0) { 
      // increase scoping level
      g_bracketCount++;
   }

   if (g_insideBody && qstrcmp(s, "}") == 0) { 
      // decrease scoping level
      g_bracketCount--;

      if (g_bracketCount <= 0) { 
         // got outside of function body
         g_insideBody   = false;
         g_bracketCount = 0;
      }
   }
}

void ClangParser::writeSources(CodeOutputInterface &ol, QSharedPointer<FileDef> fd)
{
   TooltipManager::instance()->clearTooltips();

   // set global parser state
   g_currentDefinition = QSharedPointer<Definition>();
   g_currentMemberDef  = QSharedPointer<MemberDef>();
   g_currentLine       = 0;
   g_searchForBody     = false;
   g_insideBody        = false;
   g_bracketCount      = 0;

   unsigned int line   = 1;
   unsigned int column = 1;

   QByteArray lineNumber;
   QByteArray lineAnchor;
   
   ol.startCodeLine(true);
   writeLineNumber(ol, fd, line);

   for (unsigned int i = 0; i < p->numTokens; i++) {
      CXSourceLocation start = clang_getTokenLocation(p->tu, p->tokens[i]);

      unsigned int t_line;
      unsigned int t_col;

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

      CXString tokenString = clang_getTokenSpelling(p->tu, p->tokens[i]);
      char const *s = clang_getCString(tokenString);

      CXCursorKind cursorKind  = clang_getCursorKind(p->cursors[i]);
      CXTokenKind tokenKind    = clang_getTokenKind(p->tokens[i]);

      switch (tokenKind) {
         case CXToken_Keyword:
            if (strcmp(s, "operator") == 0) {
               linkIdentifier(ol, fd, line, column, s, i);

            } else {
               QString temp;

               if (cursorKind == CXCursor_PreprocessingDirective) {
                  temp = "preprocessor";
 
               } else {
                  temp = keywordToType(s);

               }

               codifyLines(ol, fd, s, line, column, temp);
            }
            break;

         case CXToken_Literal:
            if (cursorKind == CXCursor_InclusionDirective) {
               linkInclude(ol, fd, line, column, s);

            } else if (s[0] == '"' || s[0] == '\'') {
               codifyLines(ol, fd, s, line, column, "stringliteral");

            } else {
               codifyLines(ol, fd, s, line, column, "");

            }
            break;

         case CXToken_Comment:
            codifyLines(ol, fd, s, line, column, "comment");
            break;

         default:  
            // CXToken_Punctuation or CXToken_Identifier

            if (tokenKind == CXToken_Punctuation) {
               detectFunctionBody(s);
            }

            switch (cursorKind) {
               case CXCursor_PreprocessingDirective:
                  codifyLines(ol, fd, s, line, column, "preprocessor");
                  break;

               case CXCursor_MacroDefinition:
                  codifyLines(ol, fd, s, line, column, "preprocessor");
                  break;

               case CXCursor_InclusionDirective:
                  linkInclude(ol, fd, line, column, s);
                  break;

               case CXCursor_MacroExpansion:
                  linkMacro(ol, fd, line, column, s);
                  break;

               default:
                  if (tokenKind == CXToken_Identifier || (tokenKind == CXToken_Punctuation && 
                         (cursorKind == CXCursor_DeclRefExpr || cursorKind == CXCursor_MemberRefExpr ||
                          cursorKind == CXCursor_CallExpr || cursorKind == CXCursor_ObjCMessageExpr)) ) {

                     linkIdentifier(ol, fd, line, column, s, i);

                     if (Doxy_Globals::searchIndex) {
                        ol.addWord(s, false);
                     }

                  } else {
                     codifyLines(ol, fd, s, line, column, "");
                  }

                  break;
            }
      }

      clang_disposeString(tokenString);
   }

   ol.endCodeLine();
   TooltipManager::instance()->writeTooltips(ol);
}

ClangParser::ClangParser()
{
   p = new Private;
}

ClangParser::~ClangParser()
{
   delete p;
}

