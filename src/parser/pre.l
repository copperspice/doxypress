/*************************************************************************
 *
 * Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 *
*************************************************************************/

%{

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QStringList>
#include <QStack>
#include <QVector>

#include <pre.h>

#include <a_define.h>
#include <arguments.h>
#include <constexp.h>
#include <config.h>
#include <condparser.h>
#include <doxy_globals.h>
#include <default_args.h>
#include <entry.h>
#include <message.h>
#include <membername.h>
#include <util.h>

#define YY_NEVER_INTERACTIVE 1

// Toggle for some debugging info
// #define DBG_CTX(x) fprintf x
#define DBG_CTX(x) do { } while(0)

struct CondCtx
{

   CondCtx(int line, const QString &id, bool b)
      : lineNr(line), sectionId(id), skip(b) {}

   int lineNr;
   QString sectionId;
   bool skip;
};

struct FileState {
   FileState(int size) : lineNr(1), curlyCount(0), fileBuf(size),
      oldFileBuf(""), oldFileBufPos(0), bufState(0) {}

   int       lineNr;
   int       curlyCount;

   QString   fileBuf;
   QString   oldFileBuf;
   int       oldFileBufPos;
   QString   fileName;
   YY_BUFFER_STATE bufState;
};

/** @brief Singleton which manages the defines available while preprocessing files
 */
class DefineManager
{
   // local class used to hold the defines for a single file
   class DefinesPerFile
   {
    public:
      DefinesPerFile() {
      }

      virtual ~DefinesPerFile() {
      }

      /** Adds a define in the context of a file. Will replace an existing define
       *  with the same name (redefinition)
       *  @param def The Define object to add.
       */
      void addDefine(QSharedPointer<A_Define> def) {
         QSharedPointer<A_Define> d = m_defines.value(def->m_name);

         if (d != nullptr) {
            // redefine
            m_defines.remove(d->m_name);
         }

         m_defines.insert(def->m_name, def);
      }

      /** Adds an include file for this file
       *  @param fileName The name of the include file
       */
      void addInclude(const QString &fileName) {
         m_includedFiles.insert(fileName);
      }

      void collectDefines(DefineDict &dict, QSet<QString> &includeStack);

    private:
      DefineDict m_defines;
      QSet<QString> m_includedFiles;
   };

 public:
   friend class DefinesPerFile;

   /** Returns a reference to the singleton */
   static DefineManager &instance() {
      if (theInstance == nullptr) {
         theInstance = new DefineManager;
      }
      return *theInstance;
   }

   static void deleteInstance() {
      delete theInstance;
      theInstance = nullptr;
   }

   /** Starts a context in which defines are collected.
    *  Called at the start of a new file that is preprocessed.
    *  @param fileName the name of the file to process.
    */
   void startContext(const QString &fileName) {
      m_contextDefines.clear();
      if (fileName.isEmpty()) {
         return;
      }

      QSharedPointer<DefinesPerFile> dpf = m_fileMap.value(fileName);
      if (dpf == nullptr) {
         // new file

         dpf = QMakeShared<DefinesPerFile>();
         m_fileMap.insert(fileName, dpf);
      }
   }

   /** Ends the context started with startContext() freeing any
    *  defines collected within in this context.
    */
   void endContext() {
      m_contextDefines.clear();
   }

   /** Add an included file to the current context.
    *  If the file has been pre-processed already, all defines are added to the context.
    *  @param fileName The name of the include file to add to the context.
    */
   void addFileToContext(const QString &fileName) {
      if (fileName.isEmpty()) {
         return;
      }

      QSharedPointer<DefinesPerFile> dpf = m_fileMap.value(fileName);

      if (dpf == nullptr) {
         // new file

         dpf = QMakeShared<DefinesPerFile>();
         m_fileMap.insert(fileName, dpf);

      } else {
         // existing file

         QSet<QString> includeStack;
         dpf->collectDefines(m_contextDefines, includeStack);
      }
   }

   /** Add a define to the manager object.
    *  @param fileName The file in which the define was found
    *  @param def The Define object to add.
    */
   void addDefine(const QString &fileName, QSharedPointer<A_Define> def) {
      if (fileName.isEmpty()) {
         return;
      }

      QSharedPointer<A_Define> d = m_contextDefines.value(def->m_name);

      if (d != nullptr) {
         // redefine
         m_contextDefines.remove(d->m_name);
      }

      m_contextDefines.insert(def->m_name, def);

      //
      QSharedPointer<DefinesPerFile> dpf = m_fileMap.value(fileName);

      if (dpf == nullptr) {
         dpf = QMakeShared<DefinesPerFile>();
         m_fileMap.insert(fileName, dpf);
      }

      dpf->addDefine(def);
   }

   /** Add an include relation to the manager object.
    *  @param fromFileName file name in which the include was found.
    *  @param toFileName file name that is included.
    */
   void addInclude(const QString &fromFileName, const QString &toFileName) {
      if (fromFileName.isEmpty() || toFileName.isEmpty()) {
         return;
      }

      QSharedPointer<DefinesPerFile> dpf = m_fileMap.value(fromFileName);

      if (dpf == nullptr) {
         dpf = QMakeShared<DefinesPerFile>();
         m_fileMap.insert(fromFileName, dpf);
      }
      dpf->addInclude(toFileName);
   }

   /** Returns a Define object given its name or 0 if the Define does not exist.
   */
   QSharedPointer<A_Define> isDefined(const QString &name) const {
      QSharedPointer<A_Define> d = m_contextDefines.value(name);

      if (d && d->undef) {
         d = QSharedPointer<A_Define>();
      }

      return d;
   }

   /** Returns a reference to the defines found in the current context. */
   const DefineDict &defineContext() const {
      return m_contextDefines;
   }

 private:
   static DefineManager *theInstance;

   /** Helper function to collect all define for a given file */
   void collectDefinesForFile(const QString &fileName, QSharedPointer<DefineDict> dict) {
      if (fileName.isEmpty()) {
         return;
      }

      QSharedPointer<DefinesPerFile> dpf = m_fileMap.value(fileName);

      if (dpf) {
         QSet<QString> includeStack;
         dpf->collectDefines(*dict, includeStack);
      }
   }

   /** returns the DefinesPerFile object for a given file name. */
   QSharedPointer<DefinesPerFile> find(const QString &fileName) const {
      if (fileName.isEmpty()) {
         return QSharedPointer<DefinesPerFile>();
      }

      return m_fileMap.value(fileName);
   }

   DefineManager() {
   }

   virtual ~DefineManager() {
   }

   QHash<QString, QSharedPointer<DefinesPerFile>> m_fileMap;
   DefineDict m_contextDefines;
};

/** Singleton instance */
DefineManager *DefineManager::theInstance = 0;

/** Collects all defines for a file and all files that the file includes.
 *  This function will recursively call itself for each file.
 *  @param dict The dictionary to fill with the defines. A redefine will
 *         replace a previous definition.
 *  @param includeStack The stack of includes, used to stop recursion in
 *         case there is a cyclic include dependency.
 */
void DefineManager::DefinesPerFile::collectDefines(DefineDict &dict, QSet<QString> &includeStack)
{
   {
      for (auto di : m_includedFiles) {
         QString incFile = di;

         QSharedPointer<DefinesPerFile> dpf = DefineManager::instance().find(incFile);

         if (dpf && ! includeStack.contains(incFile)) {
            includeStack.insert(incFile);
            dpf->collectDefines(dict, includeStack);
         }
      }
   }

   {
      for (const auto &def : m_defines)  {
         QSharedPointer<A_Define> d = dict.value(def->m_name);

         if (d != nullptr) {
            // redefine
            dict.remove(d->m_name);
         }

         dict.insert(def->m_name, def);
      }
   }
}

static QStringList              s_pathList;
static QString                  s_yyFileName;
static QSharedPointer<FileDef>  s_yyFileDef;
static QSharedPointer<FileDef>  s_inputFileDef;

static QSharedPointer<QHash<QString, int>> s_argDict;
static QStack<QSharedPointer<FileState>>   s_includeStack;
static QStack<QSharedPointer<CondCtx>>     s_condStack;
static QSharedPointer<DefineDict>          s_expandedDict;

static int          s_yyLineNr   = 1;
static int          s_yyMLines   = 1;
static int          s_yyColNr    = 1;
static int          s_ifcount    = 0;
static int          s_defArgs    = -1;

static QString      s_defName;
static QString      s_defText;
static QString      s_defLitText;
static QString      s_defArgsStr;
static QString      s_defExtraSpacing;
static bool         s_defVarArgs;

static int          s_lastCContext;
static int          s_lastCPPContext;
static QStack<int>  s_levelGuard;

static QString      s_outputString;
static QString      s_inputString;
static int          s_inputPosition;

static int          s_roundCount;
static bool         s_quoteArg;

static int          s_findDefArgContext;
static bool         s_expectGuard;
static QString      s_guardName;
static QString      s_lastGuardName;
static QString      s_incName;
static QString      s_guardExpr;
static int          s_curlyCount;
static bool         s_nospaces;          // add extra spaces during macro expansion

static bool         s_macroExpansion;    // from the configuration
static bool         s_expandOnlyPredef;  // from the configuration
static int          s_commentCount;
static bool         s_insideComment;
static bool         s_isImported;
static QString      s_blockName;
static int          s_condCtx;
static bool         s_skip;

static bool         s_insideCS;          // C# has simpler preprocessor
static bool         s_isSource;

static bool         s_lexInit   = false;
static int          s_fenceSize = 0;
static bool         s_ccomment;

static QSet<QString> s_allIncludes;
static QSet<QString> s_expansionDict;

#define MAX_EXPANSION_DEPTH 50

static void setFileName(const QString &name)
{
   bool ambig;
   QFileInfo fi(name);

   s_yyFileName = fi.absoluteFilePath();
   s_yyFileDef  = findFileDef(&Doxy_Globals::inputNameDict, s_yyFileName, ambig);

   if (s_yyFileDef == nullptr) {
      // if this is not an input file check if it is an include file
      s_yyFileDef = findFileDef(&Doxy_Globals::includeNameDict, s_yyFileName, ambig);
   }

   if (s_yyFileDef && s_yyFileDef->isReference()) {
      s_yyFileDef = QSharedPointer<FileDef>();
   }

   s_insideCS = getLanguageFromFileName(s_yyFileName) == SrcLangExt_CSharp;
   s_isSource = determineSection(s_yyFileName);
}

static void incrLevel()
{
   s_levelGuard.push(0);
}

static void decrLevel()
{
   if (s_levelGuard.size() > 0) {
      s_levelGuard.pop();

   } else {
      warn(s_yyFileName, s_yyLineNr, "More #endif's than #if's found\n");

   }
}

static bool otherCaseDone()
{
   if (s_levelGuard.size() == 0) {
      warn(s_yyFileName, s_yyLineNr, "Found an #else without a preceding #if\n");
      return true;

   } else {
      return s_levelGuard.top();

   }
}

static void setCaseDone(bool value)
{
   s_levelGuard.top() = value;
}

static QSharedPointer<FileState> checkAndOpenFile(const QString &fileName, bool &alreadyIncluded)
{
   alreadyIncluded = false;
   QSharedPointer<FileState> fs;

   QFileInfo fi(fileName);

   if (fi.exists() && fi.isFile()) {
      static const QStringList exclPatterns = Config::getList("exclude-patterns");

      if (patternMatch(fi, exclPatterns)) {
         return QSharedPointer<FileState>();
      }

      QString absName = fi.absoluteFilePath();

      // global guard
      if (s_curlyCount == 0) {
         // not #include inside { ... }

         if (s_allIncludes.contains(absName)) {
            alreadyIncluded = true;
            return QSharedPointer<FileState>();
         }

         s_allIncludes.insert(absName);
      }

      // check include stack for absName
      QStack<QSharedPointer<FileState>> tmpStack;

      while (! s_includeStack.isEmpty()) {
         fs = s_includeStack.pop();

         if (fs->fileName == absName) {
            alreadyIncluded = true;
         }

         tmpStack.push(fs);
      }

      while (! tmpStack.isEmpty()) {
         fs = tmpStack.pop();

         s_includeStack.push(fs);
      }

      if (alreadyIncluded) {
         return QSharedPointer<FileState>();
      }

      fs = QMakeShared<FileState>(fi.size() + 4096);
      alreadyIncluded = false;

      if (! readInputFile(absName, fs->fileBuf)) {
         // error
         fs = QSharedPointer<FileState>();

      } else {
         fs->oldFileBuf    = s_inputString;
         fs->oldFileBufPos = s_inputPosition;
      }
   }

   return fs;
}

static QSharedPointer<FileState> findFile(const QString &fileName, bool localInclude, bool &alreadyIncluded)
{
   if (QDir::isAbsolutePath(fileName)) {
      QSharedPointer<FileState> fs = checkAndOpenFile(fileName, alreadyIncluded);

      if (fs) {
         setFileName(fileName);
         s_yyLineNr = 1;
         return fs;

      } else if (alreadyIncluded) {
         return QSharedPointer<FileState>();
      }
   }

   if (localInclude && ! s_yyFileName.isEmpty()) {
      QFileInfo fi(s_yyFileName);

      if (fi.exists()) {
         QString absName = fi.absolutePath() + "/" + fileName;
         QSharedPointer<FileState> fs = checkAndOpenFile(absName, alreadyIncluded);

         if (fs) {
            setFileName(absName);
            s_yyLineNr = 1;
            return fs;

         } else if (alreadyIncluded) {
            return QSharedPointer<FileState>();
         }
      }
   }

   if (s_pathList.isEmpty()) {
      return QSharedPointer<FileState>();
   }

   for (auto &s : s_pathList) {
      QString absName = s + "/" + fileName;

      QSharedPointer<FileState> fs = checkAndOpenFile(absName, alreadyIncluded);

      if (fs) {
         setFileName(absName);
         s_yyLineNr = 1;
         return fs;

      } else if (alreadyIncluded) {
         return QSharedPointer<FileState>();
      }
   }

   return QSharedPointer<FileState>();
}

static QString extractTrailingComment(const QString &s)
{
   if (s.isEmpty()) {
      return QString("");
   }

   int i = s.length() - 1;

   while (i >= 0) {
      QChar c = s[i];

      switch (c.unicode()) {
         case '/': {
            i--;

            if (i >= 0 && s[i] == '*') {
               // end of a comment block
               i--;

               while (i > 0 && !(s[i - 1] == '/' && s[i] == '*')) {
                  i--;
               }

               if (i == 0) {
                  i++;
               }

               // only /*!< or /**< are treated as a comment for the macro name     */
               // otherwise the comment is treated as part of the macro definition

               return ((s[i + 1] == '*' || s[i + 1] == '!') && s[i + 2] == '<') ? s.mid(i - 1) : "";

            } else {
               return QString("");
            }
         }

         break;

         // whitespace or line-continuation
         case ' ':
         case '\t':
         case '\r':
         case '\n':
         case '\\':
            break;

         default:
            return QString("");
      }

      i--;
   }

   return QString("");
}

static int getNextChar(const QString &expr, QString *rest, uint &pos);
static int getCurrentChar(const QString &expr, QString *rest, uint pos);
static void unputChar(const QString &expr, QString *rest, uint &pos, char c);
static bool expandExpression(QString &expr, QString *rest, int pos, int level);

static QString stringize(const QString &s)
{
   QString result;
   QChar c;
   QChar pc;

   uint i = 0;
   bool inString = false;
   bool inChar   = false;

   while (i < s.length()) {

      if (! inString && ! inChar) {
         while (i < s.length() && ! inString && ! inChar) {
            c = s.at(i++);

            if (c == '"') {
               result += "\\\"";
               inString = true;
            } else if (c == '\'') {
               result += c;
               inChar = true;
            } else {
               result += c;
            }
         }

      } else if (inChar) {
         while (i < s.length() && inChar) {
            c = s.at(i++);

            if (c == '\'') {
               result += '\'';
               inChar = false;
            } else if (c == '\\') {
               result += "\\\\";
            } else {
               result += c;
            }
         }

      } else {
         pc = 0;

         while (i < s.length() && inString) {
            QChar c2 = s.at(i++);

            if (c2 == '"') {
               result += "\\\"";
               inString = pc == '\\';

            } else if (c2 == '\\') {
               result += "\\\\";

            } else {
               result += c2;
            }

            pc = c2;
         }
      }
   }

   return result;
}

static void processConcatOperators(QString &str)
{
   if (str.isEmpty()) {
      return;
   }

   static QRegularExpression regExp("[ \t\r\n]*##[ \t\r\n]*");
   QRegularExpressionMatch match = regExp.match(str);

   int matchLen;
   int pos;

   while (match.hasMatch()) {

      pos      = match.capturedStart() - str.constBegin();
      matchLen = match.capturedLength();

      if (pos + matchLen + 1 < str.length() && str.at(pos + matchLen) == '@' && str.at(pos + matchLen + 1) == '-') {
         // remove no-rescan marker after ID
         matchLen += 2;
      }

      // remove the ## operator and the surrounding whitespace
      str   = str.left(pos) + str.right(str.length() - pos - matchLen);
      int k = pos - 1;

      while (k >= 0 && isId(str.at(k))) {
         --k;
      }

      if (k > 0 && str.at(k) == '-' && str.at(k - 1) == '@') {
         // remove no-rescan marker before ID
         str  = str.left(k - 1) + str.right(str.length() - k - 1);
         pos -= 2;
      }

      match = regExp.match(str, str.constBegin() + pos);
   }
}

static void yyunput (int c,char *buf_ptr);
static void returnCharToStream(char c)
{
  unput(c);
}

static inline void addTillEndOfString(const QString &expr, QString *rest, uint &pos, char term, QString &arg)
{
   int cc;

   while ((cc = getNextChar(expr, rest, pos)) != EOF && cc != 0) {
      if (cc == '\\') {
         arg += (char)cc, cc = getNextChar(expr, rest, pos);
      } else if (cc == term) {
         return;
      }
      arg += (char)cc;
   }
}

/*! replaces the function macro def whose argument list starts at pos in expression \a expr.
 * Notice that this routine may scan beyond the expr string if needed.
 * In that case the characters will be read from the input file.
 * The replacement string will be returned in \a result and the
 * length of the (unexpanded) argument list is stored in \a len.
 */
static bool replaceFunctionMacro(const QString &expr, QString *rest, int pos, int &len,
                  QSharedPointer<const A_Define> def, QString &result, int level)
{
   uint j = pos;
   len    = 0;

   result.clear();
   int cc;

   while ((cc = getCurrentChar(expr, rest, j)) != EOF && isspace(cc)) {
      len++;
      getNextChar(expr, rest, j);
   }

   if (cc != '(') {
      unputChar(expr, rest, j, cc);
      return false;
   }

   // consume the `(' character
   getNextChar(expr, rest, j);

   // list of arguments
   QHash<QString, QString> argTable;

   QString arg;
   int argCount = 0;
   bool done    = false;

   // PHASE 1: read the macro arguments
   if (def->nargs == 0) {
      while ((cc = getNextChar(expr, rest, j)) != EOF && cc != 0) {
         char c = (char)cc;

         if (c == ')') {
            break;
         }
      }

   } else {
      while (! done && (argCount < def->nargs || def->varArgs) &&
             ((cc = getNextChar(expr, rest, j)) != EOF && cc != 0)) {

         char c = (char)cc;

         if (c == '(') {
            // argument is a function => search for matching )
            int level = 1;
            arg += c;

            //char term='\0';

            while ((cc = getNextChar(expr, rest, j)) != EOF && cc != 0) {
               char c = (char)cc;

               if (c == '\'' || c == '\"') {
                  // skip ('s and )'s inside strings
                  arg += c;
                  addTillEndOfString(expr, rest, j, c, arg);
               }

               if (c == ')') {
                  level--;
                  arg += c;
                  if (level == 0) {
                     break;
                  }

               } else if (c == '(') {
                  level++;
                  arg += c;

               } else {
                  arg += c;
               }
            }

         } else if (c == ')' || c == ',') {
            // last or next argument found

            if (c == ',' && argCount == def->nargs - 1 && def->varArgs) {
               arg = arg.trimmed();
               arg += ',';

            } else {

               QString argKey;
               argKey = QString("@%1").formatArg(argCount++);    // key name
               arg    = arg.trimmed();

               // add argument to the lookup table
               argTable.insert(argKey, arg);
               arg.clear();

               if (c == ')') {
                  // end of the argument list
                  done = true;
               }
            }

         } else if (c == '\"') {
            // append literal strings

            arg += c;
            bool found = false;

            while (!found && (cc = getNextChar(expr, rest, j)) != EOF && cc != 0) {
               found = cc == '"';

               if (cc == '\\') {
                  c = (char)cc;
                  arg += c;
                  if ((cc = getNextChar(expr, rest, j)) == EOF || cc == 0) {
                     break;
                  }
               }
               c = (char)cc;
               arg += c;
            }

         } else if (c == '\'') {
            // append literal characters
            arg += c;
            bool found = false;

            while (! found && (cc = getNextChar(expr, rest, j)) != EOF && cc != 0) {
               found = cc == '\'';

               if (cc == '\\') {
                  c = (char)cc;
                  arg += c;

                  if ((cc = getNextChar(expr, rest, j)) == EOF || cc == 0) {
                     break;
                  }
               }

               c = (char)cc;
               arg += c;
            }

         } else if (c == '/') {
            // possible start of a comment

            char prevChar = '\0';
            arg += c;

            cc = getCurrentChar(expr, rest, j);

            if (cc == '*') {
               // we have a comment

               while ((cc = getNextChar(expr,rest,j)) != EOF && cc != 0) {
                  c    = (char)cc;
                  arg += c;

                  if (c == '/' && prevChar == '*')  {
                     // found an end of comment
                     break;
                  }

                  prevChar = c;
               }
            }

         } else {
            // append other characters
            arg += c;
         }
      }
   }

   // PHASE 2: apply the macro function
   if (argCount == def->nargs || // same number of arguments
         (argCount >= def->nargs - 1 && def->varArgs)) {

         // variadic macro with at least as many
         // params as the non-variadic part (see bug731985)

      uint k = 0;

      // substitution of all formal arguments
      QString resExpr;
      const QString d = def->m_definition.trimmed();

      bool inString = false;

      while (k < d.length()) {
         if (d.at(k) == '@') {
            // maybe a marker, otherwise an escaped @

            if (d.at(k + 1) == '@') {
               // escaped @ => copy it (is unescaped later)

               k += 2;
               resExpr += "@@"; // we unescape these later

            } else if (d.at(k + 1) == '-') {
               // no-rescan marker
               k += 2;
               resExpr += "@-";

            } else {
               // argument marker, read the argument number
               QString key = "@";

               bool hash = false;
               int len2  = k - 1;

               // search for ## backward
               if (len2 >= 0 && d.at(len2) == '\"') {
                  len2--;
               }

               while (len2 >= 0 && d.at(len2) == ' ') {
                  len2--;
               }

               if (len2 > 0 && d.at(len2) == '#' && d.at(len2 - 1) == '#') {
                  hash = true;
               }

               k++;

               // scan the number
               while (k < d.length() && d.at(k) >= '0' && d.at(k) <= '9') {
                  key += d.at(k++);
               }

               if (! hash) {
                  // search for ## forward
                  len2 = k;

                  if (len2 < d.length() && d.at(len2) == '\"') {
                     len2++;
                  }

                  while (len2 < d.length() && d.at(len2) == ' ') {
                     len2++;
                  }

                  if (len2 < d.length() - 1 && d.at(len2) == '#' && d.at(len2 + 1) == '#') {
                     hash = true;
                  }
               }

               if (key.length() > 1 && argTable.contains(key)) {
                  QString substArg = argTable.value(key);

                  // only if no ## operator is before or after the argument marker, then do macro expansion
                  if (! hash) {
                     expandExpression(substArg, nullptr, 0, level + 1);
                  }

                  if (inString) {
                     // if the marker is inside a string (because a # was put
                     // before the macro name), escape " and \ characters
                     resExpr += stringize(substArg);

                  } else {
                     if (hash && substArg.isEmpty()) {
                        // empty argument will be remove later on
                        resExpr += "@E";

                     } else if (s_nospaces) {
                        resExpr += substArg;

                     } else {
                        resExpr += " " + substArg + " ";
                     }
                  }
               }
            }

         } else {
            // no marker, just copy

            if (! inString && d.at(k) == '\"') {
               inString = true; // entering a literal string

            } else if (inString && d.at(k) == '\"' && (d.at(k - 1) != '\\' || d.at(k - 2) == '\\')) {
               inString = false; // leaving a literal string
            }
            resExpr += d.at(k++);
         }
      }

      len    = j - pos;
      result = resExpr;

      return true;
   }

   return false;
}


/*! returns the next identifier in string \a expr by starting at position \a p.
 * The position of the identifier is returned (or -1 if nothing is found)
 * and \a l is its length. Any quoted strings are skipping during the search.
 */
static int getNextId(const QString &expr, int p, int *l)
{
   int n;

   while (p < expr.length()) {
      QChar c = expr.at(p++);

      if (c.isNumber()) {
         // skip number
         while (p < expr.length() && isId(expr.at(p))) {
            p++;
         }

      } else if (c.isLetter() || c == '_') {
         // read id
         n = p - 1;

         while (p < expr.length() && isId(expr.at(p))) {
            p++;
         }

         *l = p - n;
         return n;

      } else if (c == '"') {
         // skip string

         QChar ppc = 0;
         QChar pc = c;

         if (p < expr.length()) {
            c = expr.at(p);
         }

         while (p < expr.length() && (c != '"' || (pc == '\\' && ppc != '\\')))
            // continue as long as no " is found, but ignoring \", but not \\"
         {
            ppc = pc;
            pc = c;
            c = expr.at(p);
            p++;
         }

         if (p < expr.length()) {
            ++p;   // skip closing quote
         }

      } else if (c == '/') { // skip C Comment
         QChar pc = c;

         if (p < expr.length()) {
            c = expr.at(p);

            if (c == '*') { // Start of C comment
               p++;

               while (p < expr.length() && !(pc == '*' && c == '/')) {
                  pc = c;
                  c = expr.at(p++);
               }
            }
         }

      }
   }

   return -1;
}

/*! preforms recursive macro expansion on the string expr starting at position pos.
 *  May read additional characters from the input while re-scanning
 *  If expandAll is true then all macros in the expression are expanded, otherwise only the first is expanded
 */
static bool expandExpression(QString &expr, QString *rest, int pos, int level)
{
   if (expr.isEmpty()) {
     return true;
   }

   if (s_expansionDict.contains(expr) && level > MAX_EXPANSION_DEPTH) {
      // check for too deep recursive expansions
      return false;

   } else {
      s_expansionDict.insert(expr);
   }

   QString macroName;
   QString expMacro;

   bool definedTest = false;
   int i = pos;
   int tmpLen;
   int p;
   int len;

   int startPos     = pos;
   int samePosCount = 0;

   while ((p = getNextId(expr, i, &tmpLen)) != -1) {
      // search for an macro name

      bool replaced = false;
      macroName = expr.mid(p, tmpLen);

      if (p < 2 || ! (expr.at(p - 2) == '@' && expr.at(p - 1) == '-')) {
         // no-rescan marker?

         if (! s_expandedDict->contains(macroName)) {

            // expand macro
            QSharedPointer<A_Define> def = DefineManager::instance().isDefined(macroName);

            if (macroName == "defined") {
               definedTest = true;

            } else if (definedTest) {
               // macro name was found after defined

               if (def) {
                  expMacro = " 1 ";
               } else {
                  expMacro = " 0 ";
               }

               replaced    = true;
               len         = tmpLen;
               definedTest = false;

            } else if (def && def->nargs == -1) {
               // simple macro
               // substitute the definition of the macro

               if (s_nospaces) {
                  expMacro = def->m_definition.trimmed();
               } else {
                  expMacro = " " + def->m_definition.trimmed() + " ";
               }

               replaced = true;
               len = tmpLen;

            } else if (def && def->nargs >= 0) {
               // function macro
               replaced = replaceFunctionMacro(expr, rest, p + tmpLen, len, def, expMacro, level);
               len += tmpLen;
            }

            if (replaced) {
               // expand the macro and rescan the expression

               QString resultExpr = expMacro;
               QString restExpr   = expr.right(expr.length() - len - p);

               processConcatOperators(resultExpr);

               bool isExpanded = false;

               if (def && ! def->nonRecursive) {
                  s_expandedDict->insert(macroName, def);
                  isExpanded = expandExpression(resultExpr, &restExpr, 0, level + 1);
                  s_expandedDict->remove(macroName);
               }
               if (isExpanded) {
                  expr = expr.left(p) + resultExpr + restExpr;
                  i    = p;

                } else {
                   expr = expr.left(p) + "@-" + expr.right(expr.length()-p);
                   i    = p + tmpLen + 2;
                }

            } else {
               // move to the next macro name
               i = p + tmpLen;
            }

         } else {
            // move to the next macro name
            expr = expr.left(p) + "@-" + expr.right(expr.length() - p);
            i = p + tmpLen + 2;

         }

         // check for too many inplace expansions without making progress
         if (i == startPos) {
           samePosCount++;

         } else {
           startPos     = i;
           samePosCount = 0;
         }

         if (samePosCount > MAX_EXPANSION_DEPTH) {
           break;
         }


      } else {
         // no re-scan marker found, skip the macro name
         i = p + tmpLen;
      }
   }
  return true;
}

//  inputStr should point to the start of a string or character literal.
// the routine will return a pointer to just after the end of the literal
// the character making up the literal will be added to \a result.

QString::const_iterator processUntilMatch(QString::const_iterator iter_start, QString::const_iterator iter_end,
                  QString &result)
{
   if (iter_start == iter_end) {
      return iter_start;
   }

   // capture start character
   QChar term = *iter_start;

   if (term != '\'' && term != '"') {
      // not a valid literal
      return iter_start;
   }

   QChar ch = term;

   // output start character
   result += ch;
   ++iter_start;

   while (iter_start != iter_end) {
      ch = *iter_start;

      if (ch == term) {
         // found end marker of the literal, output end character and stop

         result += ch;
         ++iter_start;
         break;

      } else if (ch == '\\')  {
         // escaped character, process next character
         // as well without checking for end marker

         result += ch;
         ++iter_start;

         if (iter_start == iter_end) {
            // unexpected end of string after escape character
            break;
         }

         ch = *iter_start;
      }

      result += ch;
      ++iter_start;
   }

   return iter_start;
}


/*! replaces all occurrences of @@@@ in s by @@
 *  and removes all occurrences of @@E
 *  All identifiers found are replaced by 0L
 */
QString removeIdsAndMarkers(const QString &s)
{
   QString::const_iterator iter     = s.constBegin();
   QString::const_iterator iter_end = s.constEnd();

   QString result;

   bool inNum = false;

   while (iter != iter_end) {
      QChar c = *iter;

      // replace @@ with @ and remove @E
      if (c == '@') {

         if (*(iter + 1) == '@') {
            result += c;

         } else if (*(iter + 1) == 'E') {
            // skip
         }

         iter += 2;

      } else if (c.isNumber()) {
         // number

         result += c;
         inNum = true;

         ++iter;

      } else if (c == '\'') {
         iter = processUntilMatch(iter, iter_end, result);

      } else if (c == 'd' && ! inNum) {
         // identifier starting with a `d'

         QStringView tmp = QStringView(iter, iter_end);

         if (tmp.startsWith("defined ") || tmp.startsWith("defined(")) {
            // defined keyword, skip defined
            iter += 7;

         } else {
            result += "0L";
            ++iter;

            while (iter != iter_end) {
               c = *iter;

               if (isId(c)) {
                  ++iter;

               } else {
                  break;
               }
            }
         }

      } else if ((c.isLetter() || c == '_') && ! inNum) {
         // replace identifier with 0L
         result += "0L";
         ++iter;

         while (iter != iter_end) {
            c = *iter;

            if (isId(c)) {
               ++iter;

            } else {
               break;
            }
         }

         while (iter != iter_end) {
            c = *iter;

            if (c.isSpace()) {
               ++iter;

            } else {
               break;
            }
         }

         if (*iter == '(') {
            // undefined function macro
            ++iter;

            int count = 1;

            while (iter != iter_end) {
               c = *iter;
               ++iter;

               if (c == '(') {
                  count++;

               } else if (c == ')') {
                  count--;

                  if (count == 0) {
                     break;
                  }

               } else if (c == '/') {
                  QChar pc = c;

                  ++iter;
                  c = *iter;

                  if (c == '*') {
                     // start of C comment

                     while (iter != iter_end && ! (pc == '*' && c == '/')) {
                        // search end of comment
                        pc = c;

                        ++iter;
                        c = *iter;
                     }

                     ++iter;
                  }
               }
            }
         }

      } else if (c == '/') {
         // skip C comments
         QChar pc = c;

         ++iter;
         c = *iter;

         if (c == '*') {
            // start of C comment

            while (iter != iter_end && ! (pc == '*' && c == '/')) {
               // search end of comment
               pc = c;

               ++iter;
               c = *iter;
            }

            ++iter;

         } else {
            // not comment but division
            result += pc;
            result += c;

            QChar lc = c.toLower()[0];

            if (! isId(lc) && lc != '.') {
               inNum = false;
            }

            ++iter;
         }

      } else {
         result += c;
         QChar lc = c.toLower()[0];

         if (! isId(lc) && lc != '.') {
            inNum = false;
         }

         ++iter;
      }
   }

   return result;
}

/*! replaces all occurrences of @@ in \a s by @
 *  \par assumption:
 *   \a s only contains pairs of @@'s
 */
QString removeMarkers(const QString &s)
{
   QString::const_iterator iter     = s.constBegin();
   QString::const_iterator iter_end = s.constEnd();
   QString result;

   while (iter != iter_end) {
      QChar c = *iter;

      switch (c.unicode()) {

         case '@': {
            // replace @@ with @

            if (iter + 1 != iter_end && iter[1] ==  '@') {
               result += c;
               ++iter;
            }

            ++iter;
         }
         break;

         case '/': {
            // skip C comments
            result  += c;

            QChar pc = c;
            ++iter;

            if (iter != s.constEnd()) {
               c = *iter;
            }

            if (c == '*') {
               // start of C comment

               while (iter != iter_end  && ! (pc == '*' && c == '/')) {
                  // search end of comment

                  if (*iter == '@' &&  (iter + 1 != iter_end && iter[1] == '@')) {
                     result += c;
                     ++iter;

                  } else {
                     result += c;
                  }

                  pc = c;
                  ++iter;

                  if (iter != iter_end) {
                     c = *iter;
                  }
               }

               if (iter != iter_end) {
                  result += c;
                  ++iter;
               }
            }
         }
         break;

         case '"':
         case '\'':
            // skip string literals and char literals

            iter = processUntilMatch(iter, iter_end, result);
            break;

         default: {
            result += c;
            ++iter;
         }
         break;
      }
   }

   return result;
}

/*! compute the value of the expression in string expr.
 *  If needed the function may read additional characters from the input.
 */
bool computeExpression(const QString &expr)
{
   QString e = expr;

   s_expansionDict.clear();
   expandExpression(e, nullptr, 0, 0);

   e = removeIdsAndMarkers(e);

   if (e.isEmpty()) {
      return false;
   }

   bool retval = parseconstexp(s_yyFileName, s_yyLineNr, e);

   return retval;
}

/*! expands the macro definition in name
 *  If needed the function may read additional characters from the input
 */

QString expandMacro(const QString &name)
{
   QString n = name;

   s_expansionDict.clear();
   expandExpression(n, nullptr, 0, 0);

   n = removeMarkers(n);

   return n;
}

QSharedPointer<A_Define> newDefine()
{
   QSharedPointer<A_Define> def = QMakeShared<A_Define>();

   def->m_name       = s_defName;
   def->m_definition = s_defText.trimmed();
   def->nargs        = s_defArgs;
   def->m_fileName   = s_yyFileName;
   def->fileDef      = s_yyFileDef;
   def->lineNr       = s_yyLineNr - s_yyMLines;
   def->columnNr     = s_yyColNr;
   def->varArgs      = s_defVarArgs;

   if (! def->m_name.isEmpty() && Doxy_Globals::expandAsDefinedDict.contains(def->m_name)) {
      def->isPredefined = true;
   }

   return def;
}

static void addDefine()
{
   if (s_skip) {
      // do not add this define as it is inside a
      // conditional section (cond command) that is disabled

      return;
   }

   QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(s_yyFileName, s_yyLineNr - s_yyMLines, s_yyColNr,
               "#define", s_defName, s_defArgsStr, "", Protection::Public, Specifier::Normal, false,
               Relationship::Member, MemberDefType::Define, ArgumentList(), ArgumentList());

   if (! s_defArgsStr.isEmpty()) {
      ArgumentList argList;

      argList = stringToArgumentList(s_defArgsStr);
      md->setArgumentList(argList);
   }

   int len = s_defLitText.indexOf('\n');

   if (len > 0 && s_defLitText.left(len).trimmed() == "\\") {
      // strip first line if it only contains a slash
      s_defLitText = s_defLitText.right(s_defLitText.length() - len - 1);

   } else if (len > 0) {
      // align the items on the first line with the items on the second line
      int k = len + 1;

      QString::const_iterator iter = s_defLitText.constBegin() + k;

      while (iter !=  s_defLitText.constEnd())  {
         QChar c = *iter;
         ++iter;

         if (c == ' ' || c == '\t') {
            ++k;

         } else {
            break;

         }
      }

      s_defLitText = s_defLitText.mid(len + 1, k - len - 1) + s_defLitText.trimmed();
   }

   md->setInitializer(s_defLitText.trimmed());
   md->setFileDef(s_inputFileDef);
   md->setDefinition("#define " + s_defName);

   QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict.find(s_defName);

   if (! mn) {
      mn = QMakeShared<MemberName>(s_defName);
      Doxy_Globals::functionNameSDict.insert(s_defName, mn);
   }

   mn->append(md);

   if (s_yyFileDef) {
      s_yyFileDef->insertMember(md);
   }
}

static inline void outputChar(QChar c)
{
   if (s_includeStack.isEmpty() || s_curlyCount > 0) {
      s_outputString += c;
   }
}

static inline void outputArray(const QString &a, int len)
{
   if (s_includeStack.isEmpty() || s_curlyCount > 0) {
      s_outputString += a.mid(0, len);
   }
}

static void readIncludeFile(const QString &inc)
{
   static bool searchIncludes = Config::getBool("search-includes");
   uint i = 0;

   // find the start of the include file name
   while (i < inc.length() && (inc.at(i) == ' ' || inc.at(i) == '"' || inc.at(i) == '<')) {
      i++;
   }
   uint s = i;

   // was it a local include?
   bool localInclude = s > 0 && inc.at(s - 1) == '"';

   // find the end of the include file name
   while (i < inc.length() && inc.at(i) != '"' && inc.at(i) != '>') {
      i++;
   }

   if (s < inc.length() && i > s) {
      // valid include file name found

      // extract include path+name
      QString incFileName = inc.mid(s, i - s).trimmed();
      QString dosExt      = incFileName.right(4);

      if (dosExt == ".exe" || dosExt == ".dll" || dosExt == ".tlb") {
         // skip imported binary files (e.g. M$ type libraries)
         return;
      }

      QString oldFileName = s_yyFileName;

      QSharedPointer<FileDef> oldFileDef = s_yyFileDef;
      int oldLineNr = s_yyLineNr;

      // absIncFileName avoids difficulties for incFileName starting with "../" (bug 641336)
      QString absIncFileName = incFileName;
      {
         QFileInfo fi(s_yyFileName);

         if (fi.exists()) {
            QString absName = fi.absolutePath() + "/" + incFileName;

            QFileInfo fi2(absName);
            if (fi2.exists()) {
               absIncFileName = fi2.absoluteFilePath();

            } else if (searchIncludes) {
               const QStringList includePath = Config::getList("include-path");

               for (auto s : includePath) {

                  QFileInfo fi(s);

                  if (fi.exists() && fi.isDir()) {
                     QString absName = fi.absoluteFilePath() + "/" + incFileName;

                     QFileInfo fi2(absName);
                     if (fi2.exists()) {
                        absIncFileName = fi2.absoluteFilePath();
                        break;
                     }

                  }

               }
            }

         }
      }

      DefineManager::instance().addInclude(s_yyFileName, absIncFileName);
      DefineManager::instance().addFileToContext(absIncFileName);

      // findFile will overwrite s_yyFileDef if found
      QSharedPointer<FileState> fs;
      bool alreadyIncluded = false;

      if ((fs = findFile(incFileName, localInclude, alreadyIncluded))) {
         // see if the include file can be found

         if (oldFileDef) {
            // add include dependency to the file in which the #include was found
            bool ambig;

            // change to absolute name
            QSharedPointer<FileDef> incFd = findFileDef(&Doxy_Globals::inputNameDict, absIncFileName, ambig);

            QSharedPointer<FileDef> temp;
            if (ambig) {
               temp = QSharedPointer<FileDef>();
            } else {
               temp = incFd;
            }

            oldFileDef->addIncludeDependency(temp, incFileName, localInclude, s_isImported, false);

            // add included by dependency
            if (s_yyFileDef) {
               s_yyFileDef->addIncludedByDependency(oldFileDef, oldFileDef->docName(), localInclude, s_isImported);
            }

         } else if (s_inputFileDef) {
            s_inputFileDef->addIncludeDependency(QSharedPointer<FileDef>(), absIncFileName, localInclude, s_isImported, true);

         }

         fs->bufState   = YY_CURRENT_BUFFER;
         fs->lineNr     = oldLineNr;
         fs->fileName   = oldFileName;
         fs->curlyCount = s_curlyCount;

         s_curlyCount   = 0;

         // push the state on the stack
         s_includeStack.push(fs);

         // set the scanner to the include file

         // Deal with file changes due to
         // #include's within { .. } blocks

         QString lineStr = QString("# 1 \"%1\" 1\n").formatArg(QString(s_yyFileName));
         outputArray(lineStr, lineStr.length());

         DBG_CTX((stderr, "Switching to include file %s\n", csPrintable(incFileName)));
         s_expectGuard = true;
         s_inputString    = fs->fileBuf;
         s_inputPosition = 0;

         preYY_switch_to_buffer(preYY_create_buffer(0, YY_BUF_SIZE));

      } else {

         if (oldFileDef) {
            bool ambig;

            // change to absolute name for bug 641336
            QSharedPointer<FileDef> fd = findFileDef(&Doxy_Globals::inputNameDict, absIncFileName, ambig);

            // add include dependency to the file in which the #include was found
            oldFileDef->addIncludeDependency(ambig ? QSharedPointer<FileDef>() : fd, incFileName, localInclude, s_isImported, false);

            // add included by dependency
            if (fd) {
               fd->addIncludedByDependency(oldFileDef, oldFileDef->docName(), localInclude, s_isImported);
            }

         } else if (s_inputFileDef) {
            s_inputFileDef->addIncludeDependency(QSharedPointer<FileDef>(), absIncFileName, localInclude, s_isImported, true);
         }

         if (Debug::isFlagSet(Debug::Preprocessor)) {
            if (alreadyIncluded) {
               Debug::print(Debug::Preprocessor, 0, "#include %s: already included, skipping...\n", csPrintable(incFileName));
            } else {
               Debug::print(Debug::Preprocessor, 0, "#include %s: not found, skipping...\n", csPrintable(incFileName));
            }
         }

         if (s_curlyCount > 0 && !alreadyIncluded) { // failed to find #include inside { ... }
            warn(s_yyFileName, s_yyLineNr, "include file %s was not found, check the value of INCLUDE_PATH", csPrintable(incFileName));
         }
      }
   }
}

static void startCondSection(const QString &sectId)
{
   CondParser prs;

   bool expResult = prs.parse(s_yyFileName, s_yyLineNr, sectId);
   s_condStack.push( QMakeShared<CondCtx>(s_yyLineNr, sectId, s_skip) );

   if (! expResult) {
      s_skip = true;
   }
}

static void endCondSection()
{
   if (s_condStack.isEmpty()) {
      s_skip = false;

   } else {
      QSharedPointer<CondCtx> ctx = s_condStack.pop();
      s_skip = ctx->skip;
   }
}

static void forceEndCondSection()
{
   while (! s_condStack.isEmpty()) {
      s_condStack.pop();
   }

   s_skip = false;
}

static QString escapeAt(const QString &text)
{
   QString result;

   for (auto c : text) {
      if (c == '@') {
         result += "@@";

      } else {
         result += c;
      }
   }

   return result;
}

static char resolveTrigraph(char c)
{
   switch (c) {
      case '=':
         return '#';

      case '/':
         return '\\';

      case '\'':
         return '^';

      case '(':
         return '[';

      case ')':
         return ']';

      case '!':
         return '|';

      case '<':
         return '{';

      case '>':
         return '}';

      case '-':
         return '~';
   }

   return '?';
}

#undef  YY_INPUT
#define YY_INPUT(buf,result,max_size)   result = yyread(buf,max_size);

static int yyread(char *buf, int max_size)
{
   int len = max_size;

   const char *src = s_inputString.constData() + s_inputPosition;

   if (s_inputPosition + len >= s_inputString.size_storage()) {
      len = s_inputString.size_storage() - s_inputPosition;
   }

   memcpy(buf, src, len);
   s_inputPosition += len;

   return len;
}

%}

ID       [a-z_A-Z\x80-\xFF][a-z_A-Z0-9\x80-\xFF]*
B        [ \t]
BN       [ \t\r\n]
CHARLIT  (("'"\\[0-7]{1,3}"'")|("'"\\."'")|("'"[^'\\\n]{1,4}"'"))

%option never-interactive
%option caseless
%option nounistd
%option noyywrap

%x    Start
%x    Command
%x    SkipCommand
%x    SkipLine
%x    SkipString
%x    CopyLine
%x    CopyString
%x    CopyStringFtn
%x    Include
%x    IncludeID
%x    EndImport
%x    DefName
%x    DefineArg
%x    DefineText
%x    SkipCPPBlock
%x    Ifdef
%x    Ifndef
%x    SkipCComment
%x    ArgCopyCComment
%x    CopyCComment
%x    SkipVerbatim
%x    SkipCPPComment
%x    RemoveCComment
%x    RemoveCPPComment
%x    Guard
%x    DefinedExpr1
%x    DefinedExpr2
%x    SkipDoubleQuote
%x    SkipSingleQuote
%x    UndefName
%x    IgnoreLine
%x    FindDefineArgs
%x    ReadString
%x    CondLineC
%x    CondLineCpp
%x    SkipCond

%%

<*>\x06
<*>\x00
<*>\r
<*>"??"[=/'()!<>-]      {
      // Trigraph
      unput(resolveTrigraph(yytext[2]));
   }

<Start>^{B}*"#"            {
      BEGIN(Command);
      s_yyColNr  += yyleng;
      s_yyMLines = 0;
   }

<Start>^{B}*/[^#]          {

      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
      BEGIN(CopyLine);
   }


<Start>^{B}*[a-z_A-Z\x80-\xFF][a-z_A-Z0-9\x80-\xFF]+{B}*"("[^\)\n]*")"/{BN}{1,10}*[:{] {
      // constructors?
      int i;

      for (i = yyleng - 1; i >= 0; i--) {
         unput(yytext[i]);
      }

      BEGIN(CopyLine);
   }

<Start>^{B}*[_A-Z][_A-Z0-9]+{B}*"("[^\(\)\n]*"("[^\)\n]*")"[^\)\n]*")"{B}*\n |
<Start>^{B}*[_A-Z][_A-Z0-9]+{B}*"("[^\)\n]*")"{B}*\n       {

      // function list macro with one (...) argument, e.g. for K_GLOBAL_STATIC_WITH_ARGS
      // function like macro

      static bool skipFuncMacros = Config::getBool("skip-function-macros");

      QString name = QString::fromUtf8(yytext);
      name = name.left(name.indexOf('(')).trimmed();

      QSharedPointer<A_Define> def;

      if (skipFuncMacros && name != "Q_PROPERTY" && ! ( (s_includeStack.isEmpty() || s_curlyCount > 0) && s_macroExpansion &&
                  (def = DefineManager::instance().isDefined(name)) && (! s_expandOnlyPredef || def->isPredefined)) ) {

         outputChar('\n');
         s_yyLineNr++;

      } else {
         // do not skip

         for (int i = yyleng - 1; i >= 0; i--) {
            unput(yytext[i]);
         }

         BEGIN(CopyLine);
      }
   }

<CopyLine>"extern"{BN}{0,80}"\"C\""*{BN}{0,80}"{"  {
      QString text = QString::fromUtf8(yytext);

      s_yyLineNr += text.count('\n');
      outputArray(text, text.length());
   }

<CopyLine>"{"           {
      // count brackets inside the main file
      QString text = QString::fromUtf8(yytext);

      if (s_includeStack.isEmpty())  {
         s_curlyCount++;
      }

      outputChar(text[0]);
   }

<CopyLine>"}"           {
      // count brackets inside the main file
      QString text = QString::fromUtf8(yytext);

      if (s_includeStack.isEmpty() && s_curlyCount > 0)  {
         s_curlyCount--;
      }

      outputChar(text[0]);
   }

<CopyLine>"'"\\[0-7]{1,3}"'"     {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
   }

<CopyLine>"'"\\."'"        {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
   }

<CopyLine>"'"."'"          {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
   }

<CopyLine>\"            {
      QString text = QString::fromUtf8(yytext);
      outputChar(text[0]);
      BEGIN( CopyString );
   }

<CopyLine>\'            {
      if (getLanguageFromFileName(s_yyFileName) != SrcLangExt_Fortran) {
         REJECT;
      }

      QString text = QString::fromUtf8(yytext);
      outputChar(text[0]);

      BEGIN( CopyStringFtn );
   }

<CopyString>[^\"\\\r\n]+      {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
   }

<CopyString>\\.            {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
   }

<CopyString>\"             {
      QString text = QString::fromUtf8(yytext);
      outputChar(text[0]);
      BEGIN( CopyLine );
   }

<CopyStringFtn>[^\'\\\r\n]+      {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
   }

<CopyStringFtn>\\.         {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
   }

<CopyStringFtn>\'          {
      QString text = QString::fromUtf8(yytext);
      outputChar(text[0]);
      BEGIN( CopyLine );
   }

<CopyLine>{ID}/{BN}{0,80}"("     {
      QString text = QString::fromUtf8(yytext);

      s_expectGuard = false;
      QSharedPointer<A_Define> def;

      if ((s_includeStack.isEmpty() || s_curlyCount > 0) && s_macroExpansion &&
                  (def = DefineManager::instance().isDefined(text)) &&
                  (! s_expandOnlyPredef || def->isPredefined)) {

         // found it
         s_roundCount = 0;
         s_defArgsStr = text;

         if (def->nargs == -1) {
            // no function macro

            QString result = def->isPredefined ? def->m_definition : expandMacro(s_defArgsStr);
            outputArray(result, result.length());

         } else {
            // zero or more arguments

            s_findDefArgContext = CopyLine;
            BEGIN(FindDefineArgs);

         }

      } else {
         outputArray(text, text.length());
      }
   }

<CopyLine>{ID}             {
      QString text = QString::fromUtf8(yytext);

      QSharedPointer<A_Define> def;

      if ((s_includeStack.isEmpty() || s_curlyCount > 0) && s_macroExpansion &&
                  (def = DefineManager::instance().isDefined(text)) &&
                   def->nargs == -1 && (! s_expandOnlyPredef || def->isPredefined)) {

            QString result = def->isPredefined ? def->m_definition : expandMacro(text);
            outputArray(result, result.length());

      } else {
         outputArray(text, text.length());
      }
   }

<CopyLine>"\\"\r?/\n          {
      //  strip line continuation characters
   }

<CopyLine>.             {
      QString text = QString::fromUtf8(yytext);
      outputChar(text[0]);
   }

<CopyLine>\n            {
      outputChar('\n');
      BEGIN(Start);
      s_yyLineNr++;
      s_yyColNr = 1;
   }

<FindDefineArgs>"("        {
      s_defArgsStr += '(';
      s_roundCount++;
   }

<FindDefineArgs>")"        {
      s_defArgsStr += ')';
      s_roundCount--;

      if (s_roundCount == 0) {
         QString result = expandMacro(s_defArgsStr);

         if (s_findDefArgContext == CopyLine) {
            outputArray(result, result.length());
            BEGIN(s_findDefArgContext);

         } else {
            readIncludeFile(result);
            s_nospaces = false;
            BEGIN(Start);
         }
      }
   }

  /*
<FindDefineArgs>")"{B}*"("       {
      QString text = QString::fromUtf8(yytext);
      s_defArgsStr += text;
   }
  */

<FindDefineArgs>{CHARLIT}     {
      QString text = QString::fromUtf8(yytext);
      s_defArgsStr += text;
    }

<FindDefineArgs>"/*"[*]?                {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);
      s_defArgsStr += text;
      BEGIN(ArgCopyCComment);
   }

<FindDefineArgs>\"         {
      QString text = QString::fromUtf8(yytext);
      s_defArgsStr += text[0];
      BEGIN(ReadString);
   }


<FindDefineArgs>'           {
      QString text = QString::fromUtf8(yytext);

      if (getLanguageFromFileName(s_yyFileName) != SrcLangExt_Fortran) {
         REJECT;
      }

      s_defArgsStr += text[0];
      BEGIN(ReadString);
   }

<FindDefineArgs>\n         {
      s_defArgsStr += ' ';
      s_yyLineNr++;
      outputChar('\n');
   }

<FindDefineArgs>"@"        {
      s_defArgsStr += "@@";
   }

<FindDefineArgs>.          {
      QString text = QString::fromUtf8(yytext);
      s_defArgsStr += text[0];
   }

<ArgCopyCComment>[^*\n]+      {
      s_defArgsStr += QString::fromUtf8(yytext);
   }

<ArgCopyCComment>"*/"         {
      s_defArgsStr += QString::fromUtf8(yytext);
      BEGIN(FindDefineArgs);
   }

<ArgCopyCComment>\n        {
      s_defArgsStr += ' ';
      s_yyLineNr++;
      outputChar('\n');
   }

<ArgCopyCComment>.         {
      s_defArgsStr += QString::fromUtf8(yytext);
   }

<ReadString>"\""           {
      QString text = QString::fromUtf8(yytext);
      s_defArgsStr += text[0];
      BEGIN(FindDefineArgs);
   }

<ReadString>"'"            {
      QString text = QString::fromUtf8(yytext);

      if (getLanguageFromFileName(s_yyFileName) != SrcLangExt_Fortran) {
         REJECT;
      }

      s_defArgsStr += text[0];
      BEGIN(FindDefineArgs);
   }

<ReadString>"//"|"/*"         {
      // */ (editor syntax fix)
      s_defArgsStr += QString::fromUtf8(yytext);
   }

<ReadString>\\/\r?\n {
      // continue line
   }

<ReadString>\\.            {
      s_defArgsStr += QString::fromUtf8(yytext);
   }

<ReadString>.           {
      QString text = QString::fromUtf8(yytext);
      s_defArgsStr += text[0];
   }

<Command>("include"|"import"){B}+/{ID}    {
      QString text = QString::fromUtf8(yytext);
      s_isImported = text[1] == 'm';

      if (s_macroExpansion) {
         BEGIN(IncludeID);
      }
   }

<Command>("include"|"import"){B}*[<"]  {
      QString text = QString::fromUtf8(yytext);
      s_isImported = text[1] == 'm';
      s_incName = text[text.length() - 1];
      BEGIN(Include);
   }

<Command>("cmake")?"define"{B}+     {
      s_yyColNr += yyleng;
      BEGIN(DefName);
   }

<Command>"ifdef"/{B}*"("      {
      incrLevel();
      s_guardExpr.resize(0);
      BEGIN(DefinedExpr2);
   }

<Command>"ifdef"/{B}+         {
      incrLevel();
      s_guardExpr.resize(0);
      BEGIN(DefinedExpr1);
   }

<Command>"ifndef"/{B}*"("     {
      incrLevel();
      s_guardExpr = "! ";
      BEGIN(DefinedExpr2);
   }

<Command>"ifndef"/{B}+        {
      incrLevel();
      s_guardExpr = "! ";
      BEGIN(DefinedExpr1);
   }

<Command>"if"/[ \t(!]         {
      incrLevel();
      s_guardExpr.resize(0);
      BEGIN(Guard);
   }

<Command>("elif"|"else"{B}*"if")/[ \t(!]  {
      if (! otherCaseDone()) {
         s_guardExpr.resize(0);
         BEGIN(Guard);

      } else {
         s_ifcount = 0;
         BEGIN(SkipCPPBlock);
      }
   }

<Command>"else"/[^a-z_A-Z0-9\x80-\xFF]       {

      if (otherCaseDone()) {
         s_ifcount = 0;
         BEGIN(SkipCPPBlock);

      } else {
         setCaseDone(true);
      }
   }

<Command>"undef"{B}+          {
      BEGIN(UndefName);
   }

<Command>("elif"|"else"{B}*"if")/[ \t(!]  {
      if (! otherCaseDone()) {
         s_guardExpr.resize(0);
         BEGIN(Guard);
      }
   }

<Command>"endif"/[^a-z_A-Z0-9\x80-\xFF]      {
      decrLevel();
   }

<Command,IgnoreLine>\n        {
      outputChar('\n');
      BEGIN(Start);
      s_yyLineNr++;
   }

<Command>"pragma"{B}+"once"   {
      s_expectGuard = false;
   }

<Command>{ID}                 {
      // unknown directive
      BEGIN(IgnoreLine);
   }

<IgnoreLine>\\[\r]?\n         {
      outputChar('\n');
      s_yyLineNr++;
   }

<IgnoreLine>.
<Command>. {
      s_yyColNr += yyleng;
   }

<UndefName>{ID}            {
      QString text = QString::fromUtf8(yytext);

      QSharedPointer<A_Define> def;

      if ((def = DefineManager::instance().isDefined(text)) && ! def->nonRecursive) {
         def->undef = true;
      }
      BEGIN(Start);
   }

<Guard>\\[\r]?\n        {
      outputChar('\n');
      s_guardExpr += ' ';
      ++s_yyLineNr;
   }

<Guard>"defined"/{B}*"("      {
      BEGIN(DefinedExpr2);
   }

<Guard>"defined"/{B}+         {
      BEGIN(DefinedExpr1);
   }

<Guard>{ID}      {
      s_guardExpr += QString::fromUtf8(yytext);
   }

<Guard>"@" {
      s_guardExpr += "@@";
   }

<Guard>.         {
      QString text = QString::fromUtf8(yytext);
      s_guardExpr += text[0];
   }

<Guard>\n            {
      unput(*yytext);

      bool guard = computeExpression(s_guardExpr);
      setCaseDone(guard);

      if (guard) {
         BEGIN(Start);

      } else {
         s_ifcount = 0;
         BEGIN(SkipCPPBlock);
      }
   }

<DefinedExpr1,DefinedExpr2>\\\n     {
      s_yyLineNr++;
      outputChar('\n');
   }

<DefinedExpr1>{ID}         {
      QString text = QString::fromUtf8(yytext);

      if (DefineManager::instance().isDefined(text) || s_guardName == text) {
         s_guardExpr+=" 1L ";
      } else {
         s_guardExpr+=" 0L ";
      }

      s_lastGuardName = text;
      BEGIN(Guard);
   }

<DefinedExpr2>{ID}         {
      QString text = QString::fromUtf8(yytext);

      if (DefineManager::instance().isDefined(text) || s_guardName == text){
         s_guardExpr+=" 1L ";
      } else {
         s_guardExpr+=" 0L ";
      }

      s_lastGuardName = text;
   }

<DefinedExpr1,DefinedExpr2>\n       {
      // should not happen, handle anyway

      s_yyLineNr++;
      s_ifcount = 0;
      BEGIN(SkipCPPBlock);
   }

<DefinedExpr2>")"          {
      BEGIN(Guard);
   }

<DefinedExpr1,DefinedExpr2>.

<SkipCPPBlock>^{B}*"#"     {
      BEGIN(SkipCommand);
   }

<SkipCPPBlock>^{B}*/[^#]   {
      BEGIN(SkipLine);
   }

<SkipCPPBlock>\n           {
      s_yyLineNr++;
      outputChar('\n');
   }

<SkipCPPBlock>.
<SkipCommand>"if"(("n")?("def"))?/[ \t(!]    {
      incrLevel();
      s_ifcount++;
   }

<SkipCommand>"else"        {
      if (s_ifcount == 0 && ! otherCaseDone()) {
         setCaseDone(true);
         BEGIN(Start);
      }
   }

<SkipCommand>("elif"|"else"{B}*"if")/[ \t(!]       {
      if (s_ifcount == 0)  {

         if (! otherCaseDone()) {
            s_guardExpr.resize(0);
            s_lastGuardName.resize(0);
            BEGIN(Guard);

         } else {
            BEGIN(SkipCPPBlock);
         }
      }
   }

<SkipCommand>"endif"          {
      s_expectGuard = false;
      decrLevel();

      if (--s_ifcount < 0) {
         BEGIN(Start);
      }
   }

<SkipCommand>\n            {
      outputChar('\n');
      s_yyLineNr++;
      BEGIN(SkipCPPBlock);
   }

<SkipCommand>{ID}          {
      // unknown directive
      BEGIN(SkipLine);
   }

<SkipCommand>.
<SkipLine>[^'"/\n]+
<SkipLine>{CHARLIT}        {
   }

<SkipLine>\"               {
      BEGIN(SkipString);
   }

<SkipLine>.
<SkipString>"//"/[^\n]*    {
   }

<SkipLine,SkipCommand,SkipCPPBlock>"//"[^\n]* {
      s_lastCPPContext = YY_START;
      BEGIN(RemoveCPPComment);
   }

<SkipString>"/*"/[^\n]*                 {
      // */ (editor syntax fix)
   }

<SkipLine,SkipCommand,SkipCPPBlock>"/*"/[^\n]* {
      // */ (editor syntax fix)

      s_lastCContext=YY_START;
      BEGIN(RemoveCComment);
   }

<SkipLine>\n            {
      outputChar('\n');
      s_yyLineNr++;
      BEGIN(SkipCPPBlock);
   }

<SkipString>[^"\\\n]+      {
   }

<SkipString>\\.            {
   }

<SkipString>\"             {
      BEGIN(SkipLine);
   }

<SkipString>.              {
   }

<IncludeID>{ID}{B}*/"("    {
      QString text = QString::fromUtf8(yytext);

      s_nospaces   = true;
      s_roundCount = 0;
      s_defArgsStr = text;
      s_findDefArgContext = IncludeID;
      BEGIN(FindDefineArgs);
   }

<IncludeID>{ID}            {
      QString text = QString::fromUtf8(yytext);

      s_nospaces = true;
      readIncludeFile(expandMacro(text));
      BEGIN(Start);
   }

<Include>[^\">\n]+[\">]          {
      QString text = QString::fromUtf8(yytext);

      s_incName += text;
      readIncludeFile(s_incName);

      if (s_isImported) {
         BEGIN(EndImport);
      } else {
         BEGIN(Start);
      }
   }

<EndImport>[^\\\n]*/\n        {
      BEGIN(Start);
   }

<EndImport>\\[\r]?"\n"        {
      outputChar('\n');
      s_yyLineNr++;
   }

<EndImport>.            {
   }

<DefName>{ID}/("\\\n")*"("       {
      // define with argument

      s_argDict = QMakeShared<QHash<QString, int>>();

      s_defArgs = 0;
      s_defArgsStr.resize(0);
      s_defText.resize(0);
      s_defLitText.resize(0);

      s_defName    = QString::fromUtf8(yytext);
      s_defVarArgs = false;
      s_defExtraSpacing.resize(0);
      BEGIN(DefineArg);
   }

<DefName>{ID}{B}+"1"/[ \r\t\n]      {
      // special case: define with 1 -> can be "guard"

      s_argDict = QSharedPointer<QHash<QString, int>>();

      s_defArgs = -1;
      s_defArgsStr.resize(0);

      s_defName = QString::fromUtf8(yytext);
      s_defName = s_defName.left(s_defName.length() - 1).trimmed();
      s_defVarArgs = false;

      if (s_curlyCount > 0 || s_defName != s_lastGuardName || ! s_expectGuard) {
         // define may appear in the output
         QString tmp = "#define " + s_defName;
         outputArray(tmp, tmp.length());

         s_quoteArg      = false;
         s_insideComment = false;
         s_lastGuardName.resize(0);
         s_defText    = "1";
         s_defLitText = "1";
         BEGIN(DefineText);

      } else  {
         // define is a guard => hide

         s_defText.resize(0);
         s_defLitText.resize(0);
         BEGIN(Start);
      }

      s_expectGuard = false;
   }

<DefName>{ID}/{B}*"\n"        {
      // empty define
      QString text = QString::fromUtf8(yytext);

      s_argDict = QSharedPointer<QHash<QString, int>>();

      s_defArgs = -1;
      s_defName = text;

      s_defArgsStr.resize(0);
      s_defText.resize(0);
      s_defLitText.resize(0);
      s_defVarArgs = false;

      if (s_curlyCount > 0 || s_defName != s_lastGuardName || ! s_expectGuard) {
         // define may appear in the output
         QString tmp = "#define " + s_defName;
         outputArray(tmp, tmp.length());

         s_quoteArg      = false;
         s_insideComment = false;

         if (s_insideCS) {
            s_defText="1"; // for C#, use "1" as define text
         }

         BEGIN(DefineText);

      } else  {
         // define is a guard => hide

         s_guardName = text;
         s_lastGuardName.resize(0);
         BEGIN(Start);
      }

      s_expectGuard = false;
   }

<DefName>{ID}/{B}*         {
      // define with content
      QString text = QString::fromUtf8(yytext);

      s_argDict = QSharedPointer<QHash<QString, int>>();

      s_defArgs = -1;
      s_defArgsStr.resize(0);

      s_defText.resize(0);
      s_defLitText.resize(0);
      s_defName    = text;
      s_defVarArgs = false;

      QString tmp = "#define " + s_defName + s_defArgsStr;
      outputArray(tmp, tmp.length());

      s_quoteArg      = false;
      s_insideComment = false;
      BEGIN(DefineText);
   }

<DefineArg>"\\\n"              {
      s_defExtraSpacing += "\n";
      s_yyLineNr++;
   }

<DefineArg>","{B}*             {
      QString text = QString::fromUtf8(yytext);
      s_defArgsStr += text;
   }

<DefineArg>"("{B}*             {
      QString text = QString::fromUtf8(yytext);
      s_defArgsStr += text;
   }

<DefineArg>{B}*")"{B}*         {
      QString text = QString::fromUtf8(yytext);
      s_defArgsStr += text;

      QString tmp = "#define " + s_defName + s_defArgsStr + s_defExtraSpacing;
      outputArray(tmp, tmp.length());

      s_quoteArg      = false;
      s_insideComment = false;
      BEGIN(DefineText);
   }

<DefineArg>"..."        {
      // Variadic macro
      s_defVarArgs = true;
      s_defArgsStr +=  QString::fromUtf8(yytext);

      s_argDict->insert("__VA_ARGS__", s_defArgs);
      s_defArgs++;
   }

<DefineArg>{ID}{B}*("..."?)      {
      QString text = QString::fromUtf8(yytext);
      s_defArgsStr += text;

      s_defVarArgs = text.endsWith("...");

      if (s_defVarArgs)  {
         // strip ellipsis
         text = text.left(text.length() - 3);
      }

      text = text.trimmed();

      s_argDict->insert(text, s_defArgs);
      s_defArgs++;
   }

  /*
<DefineText>"/ **"|"/ *!"  {
      s_defText+=yytext;
      s_defLitText+=yytext;
      s_insideComment=true;
   }

<DefineText>"* /"          {
      s_defText+=yytext;
      s_defLitText+=yytext;
      s_insideComment=false;
   }
  */

<DefineText>"/*"[!*]?         {
      // */ (editor syntax fix)
       QString text = QString::fromUtf8(yytext);

      s_defText   += text;
      s_defLitText+= text;
      s_lastCContext = YY_START;
      s_commentCount = 1;
      BEGIN(CopyCComment);
   }

<DefineText>"//"[!/]?         {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());

      s_lastCPPContext  = YY_START;
      s_defLitText     += ' ';
      BEGIN(SkipCPPComment);
   }

<SkipCComment>[/]?"*/"        {
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '/') {
         outputChar('/');
      }

      outputChar('*');
      outputChar('/');

      if (--s_commentCount <= 0) {

         if (s_lastCContext == Start) {
            // small hack to make sure that ^... rule will
            // match when going to Start... Example: "/*...*/ some stuff..."

            YY_CURRENT_BUFFER->yy_at_bol = 1;
         }

         BEGIN(s_lastCContext);
      }
   }

<SkipCComment>"//"("/")*      {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
   }


<SkipCComment>"/*"         {
      // */ (editor syntax fix)
      outputChar('/');
      outputChar('*');

      // s_commentCount++;
   }

<SkipCComment>[\\@][\\@]("f{"|"f$"|"f[") {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
   }

<SkipCComment>^({B}*"*"+)?{B}{0,3}"~~~"[~]*   {
      static bool markdown = Config::getBool("markdown");

      if (! markdown) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);
         outputArray(text, text.length());

         s_fenceSize = yyleng;
         BEGIN(SkipVerbatim);
      }
   }

<SkipCComment>^({B}*"*"+)?{B}{0,3}"```"[`]*            {
      static bool markdown = Config::getBool("markdown");

      if (! markdown) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);
         outputArray(text, text.length());

         s_fenceSize = yyleng;
         BEGIN(SkipVerbatim);
      }
   }

<SkipCComment>[\\@][\\@]("verbatim"|"latexonly"|"htmlonly"|"xmlonly"|"docbookonly"|"rtfonly"|"manonly"|"dot"|"code"("{"[^}]*"}")?){BN}+ {
      QString text = QString::fromUtf8(yytext);

      outputArray(text, text.length());
      s_yyLineNr += text.count('\n');
   }

<SkipCComment>[\\@]("verbatim"|"latexonly"|"htmlonly"|"xmlonly"|"docbookonly"|"rtfonly"|"manonly"|"dot"|"code"("{"[^}]*"}")?){BN}+  {
      QString text = QString::fromUtf8(yytext);

      outputArray(text, text.length());

      s_yyLineNr += text.count('\n');
      s_fenceSize = 0;

      if (text[1] == 'f') {
         s_blockName = "f";

      } else {
         QString bn = text.mid(1);
         int i = bn.indexOf('{');

         // for \code{.c}
         if (i != -1) {
            bn = bn.left(i);
         }

         s_blockName = bn.trimmed();
      }
      BEGIN(SkipVerbatim);
   }

<SkipCComment,SkipCPPComment>[\\@][\\@]"cond"[ \t]+ {
      // escaped @cond
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
   }

<SkipCPPComment>[\\@]"cond"[ \t]+   {
      // conditional section
      s_ccomment = true;
      s_condCtx  = YY_START;
      BEGIN(CondLineCpp);
   }

<SkipCComment>[\\@]"cond"[ \t]+  {
      // conditional section
      s_ccomment = false;
      s_condCtx  = YY_START;
      BEGIN(CondLineC);
   }

<CondLineC,CondLineCpp>[!()&| \ta-z_A-Z0-9\x80-\xFF.\-]+      {
      QString text = QString::fromUtf8(yytext);
      startCondSection(text);

      if (s_skip) {
         if (YY_START == CondLineC) {
            // end C comment
            outputArray("*/", 2);
            s_ccomment = true;

         } else {
            s_ccomment = false;
         }

         BEGIN(SkipCond);

      } else {
         BEGIN(s_condCtx);

      }
   }

<CondLineC,CondLineCpp>.      {
      // non-guard character
      unput(*yytext);
      startCondSection(" ");

      if (s_skip) {
         if (YY_START == CondLineC) {
            // end C comment
            outputArray("*/",2);
            s_ccomment = true;

         } else {
            s_ccomment = false;
         }

         BEGIN(SkipCond);

      } else {
         BEGIN(s_condCtx);
      }
   }

<SkipCComment,SkipCPPComment>[\\@]"cond"[ \t\r]*/\n {
      // no guard
      if (YY_START == SkipCComment) {
         s_ccomment=true;
         // end C comment
         outputArray("*/",2);

      } else {
         s_ccomment = false;
      }

      s_condCtx = YY_START;
      startCondSection(" ");
      BEGIN(SkipCond);
   }

<SkipCond>\n                            {
      s_yyLineNr++;
      outputChar('\n');
   }

<SkipCond>.                             {
   }

<SkipCond>[^\/\!*\\@\n]+                {
   }

<SkipCond>"//"[/!]                      {
      s_ccomment = false;
   }

<SkipCond>"/*"[*!]                      {
      // */ (editor syntax fix)
      s_ccomment = true;
   }

<SkipCond,SkipCComment,SkipCPPComment>[\\@][\\@]"endcond"/[^a-z_A-Z0-9\x80-\xFF] {
      if (! s_skip) {
         QString text = QString::fromUtf8(yytext);
         outputArray(text, text.length());
      }
   }

<SkipCond>[\\@]"endcond"/[^a-z_A-Z0-9\x80-\xFF]  {
      bool oldSkip = s_skip;
      endCondSection();

      if (oldSkip && ! s_skip) {
         if (s_ccomment) {
            outputArray("/** ",4);

            // */ (editor syntax fix)
         }

         BEGIN(s_condCtx);
      }
   }

<SkipCComment,SkipCPPComment>[\\@]"endcond"/[^a-z_A-Z0-9\x80-\xFF] {
      // */ (editor syntax fix)
      bool oldSkip = s_skip;
      endCondSection();

      if (oldSkip && ! s_skip)  {
         BEGIN(s_condCtx);
      }
   }

<SkipVerbatim>[\\@]("endverbatim"|"endlatexonly"|"endhtmlonly"|"endxmlonly"|"enddocbookonly"|"endrtfonly"|"endmanonly"|"enddot"|"endcode"|"f$"|"f]"|"f}") {
      /* end of verbatim block */

      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());

      if (text[1] == 'f' && s_blockName == "f") {
         BEGIN(SkipCComment);

      } else if (text.mid(4) == s_blockName) {
         BEGIN(SkipCComment);
      }
   }

<SkipVerbatim>^({B}*"*"+)?{B}{0,3}"~~~"[~]*                 {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());

      if (s_fenceSize == yyleng) {
         BEGIN(SkipCComment);
      }
   }

<SkipVerbatim>^({B}*"*"+)?{B}{0,3}"```"[`]*                 {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());

      if (s_fenceSize == yyleng) {
         BEGIN(SkipCComment);
      }
   }

<SkipVerbatim>"*/"|"/*"          {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
   }

<SkipCComment,SkipVerbatim>[^*\\@\x06~`\n\/]+ {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
   }

<SkipCComment,SkipVerbatim>\n       {
      s_yyLineNr++;
      outputChar('\n');
   }

<SkipCComment,SkipVerbatim>.     {
      outputChar(*yytext);
   }

<CopyCComment>[^*a-z_A-Z\x80-\xFF\n]*[^*a-z_A-Z\x80-\xFF\\\n] {
      QString text = QString::fromUtf8(yytext);

      s_defLitText += text;
      s_defText    += escapeAt(text);
   }

<CopyCComment>\\[\r]?\n    {
      QString text = QString::fromUtf8(yytext);
      s_defLitText += text;

      s_defText += " ";
      s_yyLineNr++;
      s_yyMLines++;
   }

<CopyCComment>"*/"         {
      QString text = QString::fromUtf8(yytext);

      s_defLitText += text;
      s_defText    += text;
      BEGIN(s_lastCContext);
   }

<CopyCComment>\n        {
      QString text = QString::fromUtf8(yytext);

      s_yyLineNr++;

      s_defLitText += text;
      s_defText+=' ';
   }

<RemoveCComment>"*/"{B}*"#"           {
      // see bug 594021 for a usecase for this rule
      if (s_lastCContext == SkipCPPBlock) {
         BEGIN(SkipCommand);

      } else {
         REJECT;
      }
   }

<RemoveCComment>"*/"               {
      BEGIN(s_lastCContext);
   }

<RemoveCComment>"//"
<RemoveCComment>"/*"
<RemoveCComment>[^*\x06\n]+
<RemoveCComment>\n         {
      // */ (editor syntax fix)

      s_yyLineNr++;
      outputChar('\n');
   }

<RemoveCComment>.
<SkipCPPComment>[^\n\/\\@]+      {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
   }

<SkipCPPComment,RemoveCPPComment>\n    {
      unput(*yytext);
      BEGIN(s_lastCPPContext);
   }

<SkipCPPComment>"/*"          {
      // */ (editor syntax fix)
      outputChar('/');
      outputChar('*');
   }

<SkipCPPComment>"//"          {
      outputChar('/');
      outputChar('/');
   }

<SkipCPPComment>[^\x06\@\\\n]+      {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
   }

<SkipCPPComment>.          {
      outputChar(*yytext);
   }

<RemoveCPPComment>"/*"
<RemoveCPPComment>"//"
<RemoveCPPComment>[^\x06\n]+
<RemoveCPPComment>.
<DefineText>"#"            {
      // */ (editor syntax fix)

      s_quoteArg   = true;
      s_defLitText += QString::fromUtf8(yytext);
   }

<DefineText,CopyCComment>{ID}       {
      QString text = QString::fromUtf8(yytext);
      s_defLitText += text;

      if (s_quoteArg) {
         s_defText+="\"";
      }

      if (s_defArgs > 0) {
         auto iter = s_argDict->find(text);

         if (iter != s_argDict->end()) {
            int n = iter.value();

            s_defText += '@';

            QString numStr = QString("%1").formatArg(n);
            s_defText += numStr;

         } else {
            s_defText += text;

         }

      } else {
         s_defText += text;
      }

      if (s_quoteArg) {
         s_defText += "\"";
      }

      s_quoteArg = false;
   }

<CopyCComment>.            {
      QString text = QString::fromUtf8(yytext);

      s_defLitText += text;
      s_defText    += text;
   }

<DefineText>\\[\r]?\n         {
      QString text = QString::fromUtf8(yytext);

      s_defLitText += text;
      outputChar('\n');
      s_defText += ' ';
      s_yyLineNr++;
      s_yyMLines++;
   }

<DefineText>\n             {
      QString text = QString::fromUtf8(yytext);

      QString comment = extractTrailingComment(s_defLitText);
      s_defLitText += text;

      if (! comment.isEmpty()) {
         outputArray(comment, comment.length());
         s_defLitText = s_defLitText.left(s_defLitText.length() - comment.length() - 1);
      }

      outputChar('\n');

      QSharedPointer<A_Define> def;

      if (s_includeStack.isEmpty() || s_curlyCount > 0) {
         addDefine();
      }

      def = DefineManager::instance().isDefined(s_defName);

      if (def == nullptr)  {
         // new define

         QSharedPointer<A_Define> nd = newDefine();
         DefineManager::instance().addDefine(s_yyFileName, nd);

      } else if (def) {
         // name already exists

         if (def->undef) {
            // undefined name
            def->undef        = false;
            def->m_name       = s_defName;
            def->m_definition = s_defText.trimmed();
            def->nargs        = s_defArgs;
            def->m_fileName   = s_yyFileName;
            def->lineNr       = s_yyLineNr - s_yyMLines;
            def->columnNr     = s_yyColNr;
         }
      }

      s_argDict = QSharedPointer<QHash<QString, int>>();

      s_yyLineNr++;
      s_yyColNr = 1;
      s_lastGuardName.resize(0);
      BEGIN(Start);
   }

<DefineText>{B}*        {
      QString text = QString::fromUtf8(yytext);
      s_defText    += ' ';
      s_defLitText += text;
   }

<DefineText>{B}*"##"{B}*      {
      QString text = QString::fromUtf8(yytext);
      s_defText    += "##";
      s_defLitText += text;
   }

<DefineText>"@"            {
      QString text = QString::fromUtf8(yytext);
      s_defText     += "@@";
      s_defLitText  += text;
   }

<DefineText>\"             {
      QString text = QString::fromUtf8(yytext);

      s_defText    += text[0];
      s_defLitText += text;

      if (! s_insideComment) {
         BEGIN(SkipDoubleQuote);
      }
   }

<DefineText>\'             {
      QString text = QString::fromUtf8(yytext);

      s_defText    += text[0];
      s_defLitText += text;

      if (! s_insideComment) {
         BEGIN(SkipSingleQuote);
      }
   }

<SkipDoubleQuote>"//"[/]?     {
      QString text = QString::fromUtf8(yytext);

      s_defText    += text;
      s_defLitText += text;
   }

<SkipDoubleQuote>"/*"         {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      s_defText    += text;
      s_defLitText += text;
   }

<SkipDoubleQuote>\"        {
      QString text = QString::fromUtf8(yytext);

      s_defText    += text[0];
      s_defLitText += text;
      BEGIN(DefineText);
   }

<SkipSingleQuote,SkipDoubleQuote>\\.   {
     QString text = QString::fromUtf8(yytext);

      s_defText    += text;
      s_defLitText += text;
   }

<SkipSingleQuote>\'        {
      QString text = QString::fromUtf8(yytext);

      s_defText    += text[0];
      s_defLitText += text;
      BEGIN(DefineText);
   }

<SkipDoubleQuote>.         {
      QString text = QString::fromUtf8(yytext);

      s_defText    += text[0];
      s_defLitText += text;
   }

<SkipSingleQuote>.         {
      QString text = QString::fromUtf8(yytext);

      s_defText    += text[0];
      s_defLitText += text;
   }

<DefineText>.           {
      QString text = QString::fromUtf8(yytext);

      s_defText    += text[0];
      s_defLitText += text;
   }

<<EOF>>              {
      DBG_CTX((stderr, "End of include file\n"));

      if (s_includeStack.isEmpty()) {
         DBG_CTX((stderr, "Terminating scanner\n"));
         yyterminate();

      } else {
         QSharedPointer<FileState> fs = s_includeStack.pop();

         YY_BUFFER_STATE oldBuf = YY_CURRENT_BUFFER;
         preYY_switch_to_buffer(fs->bufState );
         preYY_delete_buffer(oldBuf );

         s_yyLineNr      = fs->lineNr;
         s_inputString   = fs->oldFileBuf;
         s_inputPosition = fs->oldFileBufPos;
         s_curlyCount    = fs->curlyCount;
         setFileName(fs->fileName);

         DBG_CTX((stderr, "######## FileName %s\n", csPrintable(s_yyFileName)));

         // deal with file changes due to
         // #include's within { .. } blocks

         QString lineStr = QString("# %1 \"%2\" 2").formatArg(s_yyLineNr).formatArg(QString(s_yyFileName));
         outputArray(lineStr, lineStr.length());
      }
   }

<*>"/*"/"*/"            |
<*>"/*"[*]?             {
      // */ (editor syntax fix)

      if (YY_START == SkipVerbatim || YY_START == SkipCond) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);
         outputArray(text, text.length());

         s_lastCContext = YY_START;
         s_commentCount = 1;

         if (yyleng == 3) {
            s_lastGuardName.resize(0); // reset guard in case the #define is documented
         }

         BEGIN(SkipCComment);
      }
   }

<*>"//"[/]?             {
      if (YY_START == SkipVerbatim || YY_START == SkipCond || getLanguageFromFileName(s_yyFileName) == SrcLangExt_Fortran) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);
         outputArray(text, text.length());

         s_lastCPPContext = YY_START;

         if (yyleng == 3) {
            s_lastGuardName.resize(0); // reset guard in case the #define is documented
         }

         BEGIN(SkipCPPComment);
      }
   }

<*>\n                {
      outputChar('\n');
      s_yyLineNr++;
   }

<*>[\xC0-\xFF][\x80-\xBF]+    {
      // utf-8 code point
      QString text  = QString::fromUtf8(yytext);
      s_expectGuard = false;

      for (QChar c : text) {
         outputChar(c);
      }
   }

<*>.                {
      // catch all
      QString text  = QString::fromUtf8(yytext);
      s_expectGuard = false;

      for (QChar c : text) {
         outputChar(c);
      }
   }


%%

static int getNextChar(const QString &expr, QString *rest, uint &pos)
{
   if (pos < expr.length()) {
      return expr.at(pos++).unicode();

   } else if (rest && ! rest->isEmpty()) {
      int cc  = rest->at(0).unicode();
      *rest   = rest->right(rest->length() - 1);
      return cc;

   } else {
      int cc = yyinput();

      return cc;
   }
}

static int getCurrentChar(const QString &expr, QString *rest, uint pos)
{
   if (pos < expr.length()) {
      return expr.at(pos).unicode();

   } else if (rest && ! rest->isEmpty()) {
      int cc = rest->at(0).unicode();
      return cc;

   } else {
      int cc = yyinput();

      returnCharToStream(cc);
      return cc;
   }
}

static void unputChar(const QString &expr, QString *rest, uint &pos, char c)
{
   if (pos < expr.length()) {
      pos++;

   } else if (rest) {
      char cs[2];
      cs[0] = c;
      cs[1] = '\0';
      rest->prepend(cs);

   } else {

      returnCharToStream(c);
   }
}

void addSearchDir(const QString &dir)
{
   QFileInfo fi(dir);

   if (fi.isDir()) {
      s_pathList.append(fi.absoluteFilePath());
   }
}

void initPreprocessor()
{
   addSearchDir(".");
   s_expandedDict = QMakeShared<DefineDict>();
}

void removePreProcessor()
{
   s_expandedDict = QSharedPointer<DefineDict>();
   s_pathList.clear();

   DefineManager::deleteInstance();
}

QString preprocessFile(const QString &fileName, const QString &input)
{
   printlex(preYY_flex_debug, true, __FILE__, fileName);

   s_macroExpansion   = Config::getBool("macro-expansion");
   s_expandOnlyPredef = Config::getBool("expand-only-predefined");

   s_skip        = false;
   s_curlyCount  = 0;
   s_nospaces    = false;

   s_inputPosition  = 0;
   s_inputString    = input;
   s_outputString   = "";

   s_includeStack.clear();
   s_expandedDict->clear();
   s_condStack.clear();

   setFileName(fileName);

   s_inputFileDef = s_yyFileDef;
   DefineManager::instance().startContext(s_yyFileName);

   static bool firstTime = true;

   if (firstTime)  {
      // add predefined macros
      static QStringList const preDefinedMacros = Config::getList("predefined-macros");

      for (const auto &definedMacro : preDefinedMacros) {

         int posEquals = definedMacro.indexOf('=');
         int posOpen   = definedMacro.indexOf('(');
         int posClose  = definedMacro.indexOf(')');

         bool nonRecursive = posEquals > 0 && definedMacro.at(posEquals - 1) == ':';

         if (posOpen == 0) {
            // no define name
            continue;
         }

         if (posOpen < posEquals && posClose < posEquals && posOpen != -1  &&
                  posClose != -1  &&  posOpen < posClose) {

            // predefined function macro definition

            // regexp matching an id
            static QRegularExpression regExp_id("[a-z_A-Z\x80-\xFF][a-z_A-Z0-9\x80-\xFF]*");
            QHash<QString, int> argDict;

            int index = posOpen + 1;
            int count = 0;
            int len;

            QRegularExpressionMatch match = regExp_id.match(definedMacro, definedMacro.constBegin() + index);

            // gather the formal arguments in a dictionary
            while (index < posClose) {

               if (match.hasMatch()) {
                  len = match.capturedLength();

                  if (len > 0) {
                     argDict.insert(match.captured(), count);
                     index = match.capturedEnd() - definedMacro.begin();

                     ++count;

                  } else {
                     ++index;
                  }

               } else {
                  break;
               }

               match = regExp_id.match(definedMacro, definedMacro.constBegin() + index);
            }

            // strip definition part
            QString tmp = definedMacro.right(definedMacro.length() - posEquals - 1);
            QString definition;

            index = 0;
            match = regExp_id.match(tmp);

            // substitute all occurrences of formal arguments with their corresponding markers
            while (match.hasMatch()) {

               int p = match.capturedStart() - tmp.constBegin();
               len   = match.capturedLength();

               definition += tmp.mid(index, p - index);

               auto iter = argDict.find(match.captured());

               if (iter != argDict.end()) {
                  int argIndex = iter.value();

                  QString marker = QString(" @%1 ").formatArg(argIndex);
                  definition += marker;

               } else {
                  definition += match.captured();
               }

               index = p + len;
               match = regExp_id.match(tmp, match.capturedEnd());
            }

            if (index < tmp.length()) {
               definition += tmp.mid(index, tmp.length() - index);
            }

            // add define definition to the dictionary of defines for this file
            QString dname = definedMacro.left(posOpen);

            if (! dname.isEmpty()) {
               QSharedPointer<A_Define> def = QMakeShared<A_Define>();

               def->m_name       = dname;
               def->m_definition = definition;
               def->nargs        = count;
               def->isPredefined = true;
               def->nonRecursive = nonRecursive;
               def->fileDef      = s_yyFileDef;
               def->m_fileName   = fileName;

               DefineManager::instance().addDefine(s_yyFileName, def);
            }

         } else if ((posOpen == -1 || posOpen > posEquals)   &&
                    (posClose == -1 || posClose > posEquals) &&
                    ! definedMacro.isEmpty() && definedMacro.length() > posEquals)  {

            // predefined non-function macro definition
            QSharedPointer<A_Define> def = QMakeShared<A_Define>();

            if (posEquals == -1) {
               // simple define without argument
               def->m_name = definedMacro;
               def->m_definition = "1"; // substitute occurrences by 1 (true)

            } else {
               // simple define with argument
               int ine = posEquals - (nonRecursive ? 1 : 0);
               def->m_name = definedMacro.left(ine);
               def->m_definition = definedMacro.right(definedMacro.length() - posEquals - 1);
            }

            if (! def->m_name.isEmpty()) {
               def->nargs = -1;
               def->isPredefined = true;
               def->nonRecursive = nonRecursive;
               def->fileDef      = s_yyFileDef;
               def->m_fileName   = fileName;
               DefineManager::instance().addDefine(s_yyFileName, def);
            }
         }
      }
   }

   s_yyLineNr = 1;
   s_yyColNr  = 1;
   s_ifcount  = 0;

   s_levelGuard.clear();

   BEGIN( Start );

   s_expectGuard = determineSection(fileName) == Entry::HEADER_SEC;
   s_guardName.resize(0);
   s_lastGuardName.resize(0);
   s_guardExpr = "";

   preYYlex();

   s_lexInit = true;

   while (! s_condStack.isEmpty()) {
      QSharedPointer<CondCtx> ctx = s_condStack.pop();
      QString sectionInfo = " ";

      if (ctx->sectionId != " ") {
         sectionInfo = QString(" with label '%1' ").formatArg(QString(ctx->sectionId));
      }

      warn(fileName, ctx->lineNr, "Conditional section %s does not have "
           "a corresponding \\endcond command within this file.", csPrintable(sectionInfo));
   }

   // make sure we do not extend a \cond with missing \endcond over multiple files
   forceEndCondSection();

   DefineManager::instance().endContext();
   printlex(preYY_flex_debug, false, __FILE__, fileName);

   return s_outputString;
}

void preFreeScanner()
{
   if (s_lexInit) {
      preYYlex_destroy();
   }
}
