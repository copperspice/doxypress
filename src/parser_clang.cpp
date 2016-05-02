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

#include <QByteArray>
#include <QHash>
#include <QSet>

#include <stdio.h>
#include <stdlib.h>

#include <parser_clang.h>
// #include <parser_clang_tooling.h>

#include <config.h>
#include <doxy_globals.h>
#include <entry.h>
#include <message.h>
#include <outputgen.h>
#include <qfileinfo.h>
#include <stringmap.h>
#include <tooltip.h>
#include <util.h>

static QSet<QString>              s_includedFiles;
static QSharedPointer<Definition> s_currentDefinition;
static QSharedPointer<MemberDef>  s_currentMemberDef;

static QSharedPointer<Entry>      s_current_root;
static QSharedPointer<Entry>      s_lastClassEntry;

static QMap<QString, QSharedPointer<Entry>> s_entryMap;

static uint g_currentLine    = 0;
static uint g_bracketCount   = 0;
static bool g_searchForBody  = false;
static bool g_insideBody     = false;

static QString getCursorSpelling(CXCursor cursor);
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
            // reset colomn counter
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

static Protection getAccessSpecifier(CXCursor cursor)
{
   CX_CXXAccessSpecifier specifier = clang_getCXXAccessSpecifier(cursor);
   Protection retval;

   switch (specifier) {

      case CX_CXXPublic:
         retval = Public;
         break;

      case CX_CXXProtected:
         retval = Protected;
         break;

      case CX_CXXPrivate:
         retval = Private;
         break;
   }

   return retval;
}

static QString getCursorDisplayName(CXCursor cursor)
{
   CXString text  = clang_getCursorDisplayName(cursor);
   QString retval = clang_getCString(text);

   clang_disposeString(text);

   return retval;
}

static QString getCursorKindName(CXCursor cursor)
{
   CXCursorKind cursorKind = clang_getCursorKind(cursor);
   CXString text  = clang_getCursorKindSpelling(cursorKind);

   QString retval = clang_getCString(text);

   clang_disposeString(text);

   return retval;
}

static QString getCursorQualifiedName(CXCursor cursor)
{
   QString retval;

   CXCursor tmpCursor = cursor;

   while (! clang_Cursor_isNull(tmpCursor) && ! clang_isTranslationUnit(clang_getCursorKind(tmpCursor)) )  {
      retval.prepend(getCursorSpelling(tmpCursor) + "::");
      tmpCursor = clang_getCursorSemanticParent(tmpCursor);
   }

   return retval;
}

static QString getCursorTypeDef(CXCursor cursor)
{
   CXType type    = clang_getTypedefDeclUnderlyingType(cursor);

   CXString text  = clang_getTypeSpelling(type);
   QString retval = clang_getCString(text);

   clang_disposeString(text);

   return retval;
}

static QString getCursorResultType(CXCursor cursor)
{
   CXType type    = clang_getCursorResultType(cursor);

   CXString text  = clang_getTypeSpelling(type);
   QString retval = clang_getCString(text);

   clang_disposeString(text);

   return retval;
}

static QString getCursorSpelling(CXCursor cursor)
{
   CXString text  = clang_getCursorSpelling(cursor);
   QString retval = clang_getCString(text);

   clang_disposeString(text);

   return retval;
}

static QString getCursorType(CXCursor cursor)
{
   CXType type    = clang_getCursorType(cursor);

   CXString text  = clang_getTypeSpelling(type);
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
   if (Doxy_Globals::searchIndex) {
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

// call back, called for each cursor node
static CXChildVisitResult visitor(CXCursor cursor, CXCursor parentCursor, CXClientData clientData)
{
   ClangParser::Private *p = ClangParser::instance()->getPrivate();

   CXSourceLocation location = clang_getCursorLocation(cursor);
   if (clang_Location_isFromMainFile(location) == 0) {
      return CXChildVisit_Continue;
   }

   uint startLine   = 0;
   uint endLine     = 0;
   uint startColumn = 0;
   uint endColumn   = 0;

   CXSourceRange range            = clang_getCursorExtent(cursor);
   CXSourceLocation startLocation = clang_getRangeStart(range);
   CXSourceLocation endLocation   = clang_getRangeEnd(range);

   clang_getSpellingLocation(startLocation, nullptr, &startLine, &startColumn, nullptr);
   clang_getSpellingLocation(endLocation,   nullptr, &endLine,   &endColumn,   nullptr);

   CXCursorKind kind = clang_getCursorKind(cursor);
   QSharedPointer<Entry> parentEntry;

   switch (kind) {

      case CXCursor_TemplateTypeParameter:
         // template type

         {
            QString name = getCursorSpelling(cursor);

            CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
            QString parentName    = getCursorUSR(parentCursor);
            parentEntry           = s_entryMap.value(parentName);

            if (parentEntry != nullptr) {

               if (parentEntry->tArgLists == nullptr) {
                  parentEntry->tArgLists = new QList<ArgumentList>;

                  ArgumentList temp;
                  parentEntry->tArgLists->append(temp);
               }

               bool isType = true;
               QString type;
               QString tmpDefault;

               CXToken *tokens;
               uint numTokens;

               clang_tokenize(p->tu, range, &tokens, &numTokens);

               for (int j = 0; j < numTokens - 1; j++) {
                  // do not take the last token since this is a comma or a close parn
                  QString text = getTokenSpelling(p->tu, tokens[j]);

                  if (text == name) {
                     isType = false;

                  } else if (isType) {
                     type += text;

                  } else  if (text != "="){
                     tmpDefault += text;

                  }
               }

               clang_disposeTokens(p->tu, tokens, numTokens);

               ArgumentList *al = &parentEntry->tArgLists->last();

               Argument tmpArg;
               tmpArg.type   = type;
               tmpArg.name   = name;
               tmpArg.defval = tmpDefault;

               al->append(tmpArg);
            }
         }

         break;

      case CXCursor_CXXBaseSpecifier:
         // class inheritance

         {
            QString name = getCursorSpelling(cursor);

            if (s_lastClassEntry != nullptr && ! name.isEmpty()) {
               Protection protection = getAccessSpecifier(cursor);
               Specifier virtualType = Specifier::Normal;

               if (clang_isVirtualBase(cursor)) {
                  virtualType = Specifier::Virtual;
               }

               if (name.startsWith("class ") ) {
                  name = name.mid(6);

               } else if (name.startsWith("struct ") ) {
                  name = name.mid(7);

               }

               auto tmpBase = BaseInfo(name, protection, virtualType);
               s_lastClassEntry->extends.append(tmpBase);
            }
         }
         break;

      case CXCursor_FunctionDecl:
         // function

         {
            QString signature = getCursorDisplayName(cursor);
            QString name      = getCursorSpelling(cursor);
            QString args      = signature.mid(name.length());

            QSharedPointer<Entry> current = QMakeShared<Entry>();

            current->section     = Entry::FUNCTION_SEC;
            current->name        = name;
            current->type        = getCursorResultType(cursor);
            current->lang        = SrcLangExt_Cpp;

            current->fileName    = p->fileName;
            current->startLine   = startLine;
            current->startColumn = startColumn;
            current->bodyLine    = startLine;

            current->args        = args;

            QString key = getCursorUSR(cursor);
            s_entryMap.insert(key, current);

            s_current_root->addSubEntry(current, s_current_root);
         }
         break;

      case CXCursor_ClassDecl:
      case CXCursor_StructDecl:
      case CXCursor_ClassTemplate:
         // class, struct, class template

         {
            QString signature = getCursorDisplayName(cursor);
            QString name      = getCursorSpelling(cursor);

            QSharedPointer<Entry> current = QMakeShared<Entry>();

            current->section     = Entry::CLASS_SEC;

            if (kind == CXCursor_ClassTemplate) {
               current->name     = name;
            } else {
               current->name     = signature;
            }

            if (kind == CXCursor_StructDecl) {
               current->type     = "struct";
               current->m_traits.setTrait(Entry::Virtue::Struct);
            } else {
               current->type     = "class";
            }

            current->lang        = SrcLangExt_Cpp;

            current->fileName    = p->fileName;
            current->startLine   = startLine;
            current->startColumn = startColumn;
            current->bodyLine    = startLine;

            if (  FALSE  )   {   // not completed
               current->m_traits.setTrait(Entry::Virtue::Final);
            }

            //
            CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
            QString parentName    = getCursorUSR(parentCursor);

            QString key = getCursorUSR(cursor);
            s_entryMap.insert(key, current);

            // used for inheritance
            s_lastClassEntry = current;

            if (parentName.isEmpty())  {
               s_current_root->addSubEntry(current, s_current_root);

            } else {
               // nested class or struct
               parentEntry = s_entryMap.value(parentName);

               if (parentEntry != nullptr) {
                  current->name.prepend(parentEntry->name + "::");
                  current->protection = getAccessSpecifier(cursor);

                  parentEntry->addSubEntry(current, parentEntry);
               }
            }
         }
         break;

      case CXCursor_UnionDecl:
         // unions

         {
            QString name = getCursorSpelling(cursor);

            QSharedPointer<Entry> current = QMakeShared<Entry>();

            current->section     = Entry::CLASS_SEC;
            current->name        = name;
            current->type        = getCursorResultType(cursor) + " union";
            current->lang        = SrcLangExt_Cpp;

            current->fileName    = p->fileName;
            current->startLine   = startLine;
            current->startColumn = startColumn;
            current->bodyLine    = startLine;

            current->m_traits.setTrait(Entry::Virtue::Union);

            QString key = getCursorUSR(cursor);
            s_entryMap.insert(key, current);

            s_current_root->addSubEntry(current, s_current_root);
         }
         break;

      case CXCursor_EnumDecl:
         // enum

         {
            QString name = getCursorSpelling(cursor);

            CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
            QString parentName    = getCursorUSR(parentCursor);
            parentEntry           = s_entryMap.value(parentName);

            QString className     = getCursorQualifiedName(parentCursor);

            bool omit = false;

            for (auto entry : parentEntry->children() ) {
               if (entry->name == className + name) {
                   omit = true;
                   break;
               }
            }

            if (omit) {
               break;
            }

            QSharedPointer<Entry> current = QMakeShared<Entry>();

            current->section     = Entry::ENUM_SEC;
            current->name        = className + name;
            current->type        = "enum";
            current->protection  = getAccessSpecifier(cursor);
            current->lang        = SrcLangExt_Cpp;

            current->fileName    = p->fileName;
            current->startLine   = startLine;
            current->startColumn = startColumn;
            current->bodyLine    = startLine;

            bool isStrong = false;

            CXToken *tokens;
            uint numTokens;

            clang_tokenize(p->tu, range, &tokens, &numTokens);

            for (int j = 0; j < numTokens - 1; j++) {
               // do not take the last token since this is a comma or a close parn
               QString text = getTokenSpelling(p->tu, tokens[j]);

               if (text == "class" ||  text == "struct")  {
                  isStrong  = true;
                  break;

               } else if (j > 1) {
                  break;

               }
            }

            clang_disposeTokens(p->tu, tokens, numTokens);

            if (isStrong) {
               current->m_traits.setTrait(Entry::Virtue::Strong);
            }

            QString key = getCursorUSR(cursor);
            s_entryMap.insert(key, current);

            parentEntry->addSubEntry(current, parentEntry);
         }
         break;

      case CXCursorKind::CXCursor_EnumConstantDecl:
         // enum value

         {
            QString name = getCursorSpelling(cursor);

            CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
            QString parentName    = getCursorUSR(parentCursor);
            parentEntry           = s_entryMap.value(parentName);

            bool omit = false;

            for (auto entry : parentEntry->children() ) {

               if (entry->name == name) {
                   omit = true;
                   break;
               }
            }

            if (omit) {
               break;
            }

            QSharedPointer<Entry> current = QMakeShared<Entry>();

            current->section     = Entry::VARIABLE_SEC;
            current->name        = name;
            current->type        = "@";
            current->protection  = getAccessSpecifier(cursor);
            current->lang        = SrcLangExt_Cpp;

            current->fileName    = p->fileName;
            current->startLine   = startLine;
            current->startColumn = startColumn;
            current->bodyLine    = startLine;

            // parse default values
            bool isDefault = false;

            CXToken *tokens;
            uint numTokens;

            clang_tokenize(p->tu, range, &tokens, &numTokens);

            for (int j = 0; j < numTokens - 1; j++) {
               // do not take the last token since this is a comma or a close parn
               QString text = getTokenSpelling(p->tu, tokens[j]);

               if (isDefault) {
                  current->initializer = " = " + text;
                  break;

               } else if (text == "=")  {
                  isDefault = true;

               }
            }

            clang_disposeTokens(p->tu, tokens, numTokens);

            QString key = getCursorUSR(cursor);
            s_entryMap.insert(key, current);

            parentEntry->addSubEntry(current, parentEntry);

            //
            bool isStrong = parentEntry->m_traits.hasTrait(Entry::Virtue::Strong);

            if (! isStrong)  {
               auto tmpEntry = parentEntry->parent();
               if (tmpEntry) {
                  tmpEntry->addSubEntry(current, tmpEntry);
               }
            }
         }
         break;

      case CXCursor_CXXMethod:
      case CXCursor_Constructor:
      case CXCursor_Destructor:
      case CXCursor_ConversionFunction:
      case CXCursor_FunctionTemplate:
         // class method, template method

         {
            QString name  = getCursorSpelling(cursor);
            bool isMethod = true;

            if ((kind == CXCursor_Constructor) || (kind == CXCursor_Destructor)) {
               isMethod = false;
            }

            CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
            QString parentName    = getCursorUSR(parentCursor);
            parentEntry           = s_entryMap.value(parentName);

            if (parentEntry == nullptr) {
               break;
            }

            QSharedPointer<Entry> current = QMakeShared<Entry>();

            current->section     = Entry::FUNCTION_SEC;
            current->name        = name;

            if (isMethod) {
               current->type     = getCursorResultType(cursor);
            }

            current->protection  = getAccessSpecifier(cursor);
            current->lang        = SrcLangExt_Cpp;

            current->fileName    = p->fileName;
            current->startLine   = startLine;
            current->startColumn = startColumn;
            current->bodyLine    = startLine;

            // parse only the arguments
            ArgumentList tmpList;

            int maxCount;

            if (kind == CXCursor_FunctionTemplate) {
               maxCount = 0;
               current->m_traits.setTrait(Entry::Virtue::Template);

            } else {
               maxCount = clang_Cursor_getNumArguments(cursor);

            }

            QString tmpArgs = "(";

            for (int k = 0; k < maxCount; ++k)   {
               // part A
               CXCursor tmpCursor;
               tmpCursor = clang_Cursor_getArgument(cursor, k);

               QString tmpType;
               QString tmpName = getCursorDisplayName(tmpCursor);

               QString tmpDefault;
               bool isDefault = false;

               CXToken *tokens;
               uint numTokens;

               CXSourceRange cursorRange = clang_getCursorExtent(tmpCursor);
               clang_tokenize(p->tu, cursorRange, &tokens, &numTokens);

               for (int j = 0; j < numTokens - 1; j++) {
                  // do not take the last token since this is a comma or a close parn
                  QString text = getTokenSpelling(p->tu, tokens[j]);

                  if (isDefault) {
                     tmpDefault += text;

                  } else if (text == "=")  {
                     isDefault  = true;

                  } else {
                     tmpArgs += text + " ";

                     if (text != tmpName) {
                        tmpType += text + " ";
                     }
                  }
               }

               clang_disposeTokens(p->tu, tokens, numTokens);

               if (! tmpDefault.isEmpty()) {
                  tmpArgs += " = " + tmpDefault;
               }

               // part B
               Argument tmp;
               tmp.type   = tmpType;
               tmp.name   = tmpName;
               tmp.defval = tmpDefault;

               tmpList.append(tmp);

               if (k < maxCount -1) {
                  tmpArgs += ", ";
               }
            }

            tmpArgs += ")";

            //
            CXType type = clang_getCursorType(cursor);
            CXRefQualifierKind qualKind = clang_Type_getCXXRefQualifier(type);

            if (qualKind == CXRefQualifier_RValue) {
               tmpArgs   += " && ";
               tmpList.refSpecifier = RefType::RValueRef;

            } else if (qualKind == CXRefQualifier_LValue)  {
               tmpArgs   += " & ";
               tmpList.refSpecifier = RefType::LValueRef;
            }

            if (clang_CXXMethod_isConst(cursor))  {
               tmpArgs += " const ";
               tmpList.constSpecifier = true;
            }

            //
            CXCursor *overrides;
            unsigned int num_overrides;

            clang_getOverriddenCursors(cursor, &overrides, &num_overrides);

            if (num_overrides > 0)  {
               tmpArgs += " override ";
               current->m_traits.setTrait(Entry::Virtue::Override);

               clang_disposeOverriddenCursors(overrides);
            }

            if (clang_CXXMethod_isPureVirtual(cursor))  {
               current->type.prepend(" virtual ");
               current->virt = Specifier::Pure;

               tmpArgs += " = 0";
               tmpList.pureSpecifier = true;


            } else if (clang_CXXMethod_isVirtual(cursor))  {
               current->type.prepend(" virtual ");
               current->virt  = Specifier::Virtual;

            }

            if (clang_CXXMethod_isStatic(cursor))  {
               current->type.prepend("static ");
               current->stat = true;
            }

            CXToken *tokens;
            uint numTokens;

            clang_tokenize(p->tu, range, &tokens, &numTokens);

            for (int j = 0; j < numTokens - 1; j++) {
               // do not take the last token since this is a comma or a close parn
               QString text = getTokenSpelling(p->tu, tokens[j]);

               if (text == "(") {
                  break;

               } else if (text == "constexpr")  {
                  current->type.prepend("constexpr ");

               } else if (text == "explicit")  {
                  current->type.prepend("explicit ");
                  current->m_traits.setTrait(Entry::Virtue::Explicit);

               } else if (text == "inline")  {
                  current->m_traits.setTrait(Entry::Virtue::Inline);

               }
            }

            for (int j = numTokens - 1; j != 0; j--) {
               // do not take the last token since this is a comma or a close parn
               QString text = getTokenSpelling(p->tu, tokens[j]);

               if (text == ")") {
                  break;

               } else if (text == "delete")  {
                  tmpArgs += " = " + text;
                  tmpList.isDeleted = true;
                  current->m_traits.setTrait(Entry::Virtue::Delete);

               } else if (text == "default")  {
                  tmpArgs += " = " + text;
                  current->m_traits.setTrait(Entry::Virtue::Default);

               } else if (text == "final")  {
                  tmpArgs += " " + text;
                  current->m_traits.setTrait(Entry::Virtue::Final);

               } else if (text == "noexcept") {
                  tmpArgs += " " + text;
                  current->m_traits.setTrait(Entry::Virtue::NoExcept);

               } else if (text == "volatile")   {
                  tmpArgs += " " + text;
                  tmpList.volatileSpecifier = true;

               }
            }

            clang_disposeTokens(p->tu, tokens, numTokens);

/*

// does not working with libClang 3.7.1

            if ( FALSE ) {
               tmpArgs += " final ";
               current->m_traits.setTrait(Entry::Virtue::Final);
            }

            if ( FALSE ) {
               tmpArgs += " noexcept ";
               current->m_traits.setTrait(Entry::Virtue::NoExcept);
            }

            if ( FALSE ) {
               tmpArgs += " noexcept(FOO)";
               current->m_traits.setTrait(Entry::Virtue::NoExcept);
            }

            if ( FALSE )  {
               tmpArgs += " = default ";
               current->m_traits.setTrait(Entry::Virtue::Default);
            }

            if ( FALSE )  {
               current->m_traits.setTrait(Entry::Virtue::Explicit);
            }

            if (clang_getCursorAvailability(cursor) == CXAvailability_NotAvailable)  {
               tmpArgs += " = delete ";
               tmpList.isDeleted = true;
               current->m_traits.setTrait(Entry::Virtue::Delete);
            }

            if (clang_isVolatileQualifiedType(type))  {
               tmpArgs += " volatile ";
               tmpList.volatileSpecifier = true;
            }

            if (clang_Cursor_isFunctionInlined(cursor)) {
                current->m_traits.setTrait(Entry::Virtue::Inline);
            }
*/

            current->args        = tmpArgs;
            current->argList     = tmpList;

            QString key = getCursorUSR(cursor);
            s_entryMap.insert(key, current);

            parentEntry->addSubEntry(current, parentEntry);
         }

         break;

      case CXCursorKind::CXCursor_FieldDecl:
         // class member

         {
            QString name = getCursorSpelling(cursor);

            CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
            QString parentName    = getCursorUSR(parentCursor);
            parentEntry           = s_entryMap.value(parentName);

            if (parentEntry != nullptr) {
               QSharedPointer<Entry> current = QMakeShared<Entry>();

               current->section     = Entry::VARIABLE_SEC;
               current->name        = name;
               current->type        = getCursorType(cursor);
               current->protection  = getAccessSpecifier(cursor);
               current->lang        = SrcLangExt_Cpp;

               current->fileName    = p->fileName;
               current->startLine   = startLine;
               current->startColumn = startColumn;
               current->bodyLine    = startLine;

               // save for libClang 3.8
//             if (clang_CXXField_isMutable(cursor)) {
//                 current->m_traits.setTrait(Entry::Virtue::Mutable);
//             }

               CXToken *tokens;
               uint numTokens;

               clang_tokenize(p->tu, range, &tokens, &numTokens);

               for (int j = 0; j < numTokens - 1; j++) {
                  // do not take the last token since this is a comma or a close parn
                  QString text = getTokenSpelling(p->tu, tokens[j]);

                  if (text == "mutable") {
                     current->type.prepend("mutable ");
                     current->m_traits.setTrait(Entry::Virtue::Mutable);
                  }
               }

               clang_disposeTokens(p->tu, tokens, numTokens);

               QString key = getCursorUSR(cursor);
               s_entryMap.insert(key, current);

               parentEntry->addSubEntry(current, parentEntry);
            }
         }
         break;

      case CXCursorKind::CXCursor_ParmDecl:
         // parameters

         {
            QString name = getCursorSpelling(cursor);
            QString type = getCursorType(cursor);

            CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
            QString parentName    = getCursorUSR(parentCursor);
            parentEntry           = s_entryMap.value(parentName);

            if (parentEntry != nullptr) {

               if (parentEntry->m_traits.hasTrait(Entry::Virtue::Template)) {

                  QString tmpArgs = parentEntry->args;
                  tmpArgs = tmpArgs.mid(0, tmpArgs.length() - 1);

                  if (tmpArgs != "(") {
                     tmpArgs += ", ";
                  }

                  tmpArgs += type + " " + name;
                  tmpArgs += ")";

                  ArgumentList tmpList = parentEntry->argList;

                  Argument tmp;
                  tmp.type = type;
                  tmp.name = name;
                  tmpList.append(tmp);

                  parentEntry->args    = tmpArgs;
                  parentEntry->argList = tmpList;
               }
            }
         }
         break;

      case CXCursorKind::CXCursor_VarDecl:
         // variable

         {
            QString name = getCursorSpelling(cursor);

            CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
            QString parentName    = getCursorUSR(parentCursor);
            parentEntry           = s_entryMap.value(parentName);

            QSharedPointer<Entry> current = QMakeShared<Entry>();

            current->section     = Entry::VARIABLE_SEC;
            current->name        = name;
            current->type        = "static " + getCursorType(cursor);
            current->protection  = getAccessSpecifier(cursor);
            current->lang        = SrcLangExt_Cpp;

            current->fileName    = p->fileName;
            current->startLine   = startLine;
            current->startColumn = startColumn;
            current->bodyLine    = startLine;

            current->stat        = true;

            QString key = getCursorUSR(cursor);
            s_entryMap.insert(key, current);

            if (parentEntry) {
               parentEntry->addSubEntry(current, parentEntry);

            } else {
               // might need to test if part of a file

            }
         }
         break;

      case CXCursorKind::CXCursor_TypedefDecl:
      case CXCursorKind::CXCursor_TypeAliasDecl:
         // typedef

         {
            QString name = getCursorSpelling(cursor);

            CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
            QString parentName    = getCursorUSR(parentCursor);
            parentEntry           = s_entryMap.value(parentName);

            QSharedPointer<Entry> current = QMakeShared<Entry>();

            current->section     = Entry::VARIABLE_SEC;
            current->name        = name;
            current->type        = "typedef "+ getCursorTypeDef(cursor);
            current->protection  = getAccessSpecifier(cursor);
            current->lang        = SrcLangExt_Cpp;

            current->fileName    = p->fileName;
            current->startLine   = startLine;
            current->startColumn = startColumn;
            current->bodyLine    = startLine;

            if (kind == CXCursorKind::CXCursor_TypeAliasDecl) {
               current->m_traits.setTrait(Entry::Virtue::Alias);
            }

            QString key = getCursorUSR(cursor);
            s_entryMap.insert(key, current);

            parentEntry->addSubEntry(current, parentEntry);
         }
         break;

      case CXCursorKind::CXCursor_UnexposedDecl:
         // friend declaration

         {
            QString type;
            QString name;
            QString args;

            QStringList list;

            bool isFriend = false;
            bool isArgs   = false;

            CXToken *tokens;
            uint numTokens;

            clang_tokenize(p->tu, range, &tokens, &numTokens);

            for (int j = 0; j < numTokens - 1; j++) {
               // do not take the last token since this is a comma or a close parn
               QString text = getTokenSpelling(p->tu, tokens[j]);

               if (text == "friend")  {
                  isFriend = true;
                  type = "friend ";

               } else if (isFriend) {

                  if (isArgs) {
                     args += text + " ";

                  } else if (text == "(")  {
                     args  += text;
                     isArgs = true;

                  } else {
                     list.append(text);

                  }
               }
            }

            clang_disposeTokens(p->tu, tokens, numTokens);

            if (isFriend) {
               name = list[list.length() - 1];

               list.removeLast();
               type = "friend " + list.join("");

               // parse the argument list, not working with libClang
               ArgumentList tmpList;

               CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
               QString parentName    = getCursorUSR(parentCursor);
               parentEntry           = s_entryMap.value(parentName);

               QSharedPointer<Entry> current = QMakeShared<Entry>();

               current->section     = Entry::FUNCTION_SEC;
               current->name        = name;
               current->type        = type;
               current->protection  = getAccessSpecifier(cursor);
               current->lang        = SrcLangExt_Cpp;

               current->args        = args;
               current->argList     = tmpList;

               current->fileName    = p->fileName;
               current->startLine   = startLine;
               current->startColumn = startColumn;
               current->bodyLine    = startLine;

               QString key = getCursorUSR(cursor);
               s_entryMap.insert(key, current);

               parentEntry->addSubEntry(current, parentEntry);
            }
         }
         break;

      case CXCursorKind::CXCursor_PreprocessingDirective:
         break;

      case CXCursorKind::CXCursor_MacroDefinition:

         {
            QString name = getCursorSpelling(cursor);

            QSharedPointer<Entry> current = QMakeShared<Entry>();

            current->section     = Entry::DEFINE_SEC;
            current->name        = name;
            current->lang        = SrcLangExt_Cpp;

            QString key = getCursorUSR(cursor);
            s_entryMap.insert(key, current);

            s_current_root->addSubEntry(current, s_current_root);
         }
         break;

     case CXCursorKind::CXCursor_MacroExpansion:

         {
            QString name = getCursorSpelling(cursor);

            if (name == "CS_OBJECT" || name == "Q_OBJECT") {
               // do nothing
            }
         }
         break;

      case CXCursorKind::CXCursor_Namespace:

         {
            QString name = getCursorSpelling(cursor);

            CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
            QString parentName    = getCursorUSR(parentCursor);
            parentEntry           = s_entryMap.value(parentName);

            QSharedPointer<Entry> current = QMakeShared<Entry>();

            current->section     = Entry::NAMESPACE_SEC;
            current->name        = name;
            current->type        = "namespace";
            current->lang        = SrcLangExt_Cpp;

            current->fileName    = p->fileName;
            current->startLine   = startLine;
            current->startColumn = startColumn;
            current->bodyLine    = startLine;

            QString key = getCursorUSR(cursor);
            s_entryMap.insert(key, current);

            if (parentEntry == nullptr)  {
               s_current_root->addSubEntry(current, s_current_root);
            } else {
               parentEntry->addSubEntry(current, parentEntry);
            }
         }
         break;
   }

   return CXChildVisit_Recurse;
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
void ClangParser::start(const QString &fileName, QStringList &includeFiles, QSharedPointer<Entry> root)
{
   static QStringList includePath = Config::getList("include-path");
   static QStringList clangFlags  = Config::getList("clang-flags");

   // exclude PCH files, disable diagnostics
   p->index    = clang_createIndex(false, false);

   p->fileName = fileName;
   p->curLine  = 1;
   p->curToken = 0;

   // reserve space for the command line options
   const char **argv = (const char **)malloc(sizeof(char *) * (4 + Doxy_Globals::inputPaths.count()
                  + includePath.count() + clangFlags.count()));

   int argc = 0;

   // add include paths for input files
   for (auto item : Doxy_Globals::inputPaths) {
      QString inc = "-I" + item;
      argv[argc++] = strdup(inc.toUtf8());
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
         argv[argc++] = strdup("c++");
         break;

      case ClangParser::Private::Detected_ObjC:
         argv[argc++] = strdup("objective-c");
         break;

      case ClangParser::Private::Detected_ObjCpp:
         argv[argc++] = strdup("objective-c++");
         break;
   }

   // provide the input and their dependencies as unsaved files in memory
   static bool filterSourceFiles = Config::getBool("filter-source-files");

   argv[argc++] = strdup(fileName.toUtf8());

   uint numUnsavedFiles = includeFiles.count() + 1;

   p->numFiles = numUnsavedFiles;
   p->sources  = new QByteArray[numUnsavedFiles];
   p->ufs      = new CXUnsavedFile[numUnsavedFiles];

   p->sources[0]      = detab(fileToString(fileName, filterSourceFiles, true)).toUtf8();
   p->ufs[0].Filename = strdup(fileName.toUtf8());
   p->ufs[0].Contents = p->sources[0].constData();
   p->ufs[0].Length   = p->sources[0].length();

   //
   uint i = 1;
   for (auto item : includeFiles) {

      p->fileMapping.insert(item, i);

      p->sources[i]      = detab(fileToString(item, filterSourceFiles, true)).toUtf8();
      p->ufs[i].Filename = strdup(item.toUtf8());
      p->ufs[i].Contents = p->sources[i].constData();
      p->ufs[i].Length   = p->sources[i].length();

      i++;
   }

   // data structure, source filename (not needed, in argv), cmd line args, num of cmd line args
   // pass unsaved files(?), num of unsaved files, options flag, where to put the trans unit

   CXErrorCode errorCode = clang_parseTranslationUnit2(p->index, 0, argv, argc, p->ufs, numUnsavedFiles,
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
         // start adding to entry
         s_current_root = root;

         // walk the AST and visit every cursor, using libClang
         CXCursor rootCursor = clang_getTranslationUnitCursor(p->tu);
         clang_visitChildren(rootCursor, visitor, nullptr);
      }

      // free memory
      for (int i = 0; i < argc; ++i) {
         free(const_cast<char *>(argv[i]));
      }
      free(argv);

      // create a source range for the file
      QFileInfo fi(fileName);
      CXFile f = clang_getFile(p->tu, fileName.toUtf8());

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

      static bool javadoc_auto_brief = Config::getBool("javadoc-auto-brief");
      static bool qt_auto_brief      = Config::getBool("qt-auto-brief");

      // walk the tokens
      for (int index = 0; index < p->numTokens; index++)  {

         CXTokenKind tokenKind = clang_getTokenKind(p->tokens[index]);

         if (tokenKind == CXToken_Comment) {
            QString comment = getTokenSpelling(p->tu, p->tokens[index]);
            comment = comment.trimmed();

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

               QChar char2  = comment.at(2);

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

               cursor = p->cursors[index+1];

               int len = comment.length() - 5;
               comment = comment.mid(3, len);

               // remove single *
               QRegExp reg("\n\\s*\\*");
               comment.replace(reg, "\n");

               if (javadoc_auto_brief) {
                  isBrief = true;
               }

            } else if (comment.startsWith("/*!")) {
               // */ (editor syntax fix),   qt comment

               cursor = p->cursors[index+1];

               int len = comment.length() - 5;
               comment = comment.mid(3, len);

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

            // test the cursor
            if (documentKind(cursor)) {

               // add the comment to cursor
               QString name = getCursorSpelling(cursor);
               QString key  = getCursorUSR(cursor);

               QSharedPointer<Entry> current = s_entryMap.value(key);

               if (current) {
                  if (isBrief && current->brief.isEmpty()) {
                     QString brief;
                     QRegExp reg("([^.]*\\.)\\s(.*)");

                     if (reg.exactMatch(comment)) {
                        brief   = reg.cap(1);
                        comment = reg.cap(2);

                     } else {
                        brief   = comment;
                        comment = "";
                     }

                     current->brief = brief;
                  }

                  current->doc += comment;
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

      if (tokenLine == line && strncmp(ts, symbol.toUtf8(), tl) == 0) {
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

            if (strncmp(ts, symbol.mid(offset).toUtf8(), tl) != 0) {
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
      QSharedPointer<FileNameList> fn = Doxy_Globals::inputNameDict->find(incName);

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
   QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict->find(text);

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
      CXFile f = clang_getFile(p->tu, fileName.toUtf8());

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

                     if (Doxy_Globals::searchIndex) {
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

