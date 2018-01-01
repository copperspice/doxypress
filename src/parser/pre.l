/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim
 * Copyright (C) 1997-2014 by Dimitri van Heesch.

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
#include <QRegExp>
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
   FileState(int size) : lineNr(1), fileBuf(size),
      oldFileBuf(""), oldFileBufPos(0), bufState(0) {}

   int       lineNr;
   QString   fileBuf;
   QString   oldFileBuf;
   int       oldFileBufPos;
   QString   fileName;
   YY_BUFFER_STATE bufState;
};

/** @brief Singleton which manages the defines available while proprocessing files
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

static QStringList              g_pathList;
static QString                  g_yyFileName;
static QSharedPointer<FileDef>  g_yyFileDef;
static QSharedPointer<FileDef>  g_inputFileDef;

static QSharedPointer<QHash<QString, int>> g_argDict;
static QStack<QSharedPointer<FileState>>   g_includeStack;
static QStack<QSharedPointer<CondCtx>>     g_condStack;
static QSharedPointer<DefineDict>          g_expandedDict;

static int          g_yyLineNr   = 1;
static int          g_yyMLines   = 1;
static int          g_yyColNr    = 1;
static int          g_ifcount    = 0;
static int          g_defArgs    = -1;

static QString      g_defName;
static QString      g_defText;
static QString      g_defLitText;
static QString      g_defArgsStr;
static QString      g_defExtraSpacing;
static bool         g_defVarArgs;
static int          g_lastCContext;
static int          g_lastCPPContext;
static QStack<int>  g_levelGuard;
static QString      g_inputBuf;
static int          g_inputBufPos;
static QString      g_outputBuf;
static int          g_roundCount;
static bool         g_quoteArg;

static int          g_findDefArgContext;
static bool         g_expectGuard;
static QString      g_guardName;
static QString      g_lastGuardName;
static QString      g_incName;
static QString      g_guardExpr;
static int          g_curlyCount;
static bool         g_nospaces;          // add extra spaces during macro expansion

static bool         g_macroExpansion;    // from the configuration
static bool         g_expandOnlyPredef;  // from the configuration
static int          g_commentCount;
static bool         g_insideComment;
static bool         g_isImported;
static QString      g_blockName;
static int          g_condCtx;
static bool         g_skip;

static bool         g_insideCS;          // C# has simpler preprocessor
static bool         g_isSource;

static bool         g_lexInit   = false;
static int          g_fenceSize = 0;
static bool         g_ccomment;

static void setFileName(const QString &name)
{
   bool ambig;
   QFileInfo fi(name);

   g_yyFileName = fi.absoluteFilePath();
   g_yyFileDef  = findFileDef(&Doxy_Globals::inputNameDict, g_yyFileName, ambig);

   if (g_yyFileDef == nullptr) {
      // if this is not an input file check if it is an include file
      g_yyFileDef = findFileDef(&Doxy_Globals::includeNameDict, g_yyFileName, ambig);
   }

   if (g_yyFileDef && g_yyFileDef->isReference()) {
      g_yyFileDef = QSharedPointer<FileDef>();
   }

   g_insideCS = getLanguageFromFileName(g_yyFileName) == SrcLangExt_CSharp;
   g_isSource = determineSection(g_yyFileName);
}

static void incrLevel()
{
   g_levelGuard.push(0);
}

static void decrLevel()
{
   if (g_levelGuard.size() > 0) {
      g_levelGuard.pop();

   } else {
      warn(g_yyFileName, g_yyLineNr, "More #endif's than #if's found\n");

   }
}

static bool otherCaseDone()
{
   if (g_levelGuard.size() == 0) {
      warn(g_yyFileName, g_yyLineNr, "Found an #else without a preceding #if\n");
      return true;

   } else {
      return g_levelGuard.top();

   }
}

static void setCaseDone(bool value)
{
   g_levelGuard.top() = value;
}

static QSet<QString> g_allIncludes;

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
      if (g_curlyCount == 0) {
         // not #include inside { ... }

         if (g_allIncludes.contains(absName)) {
            alreadyIncluded = true;
            return QSharedPointer<FileState>();
         }

         g_allIncludes.insert(absName);
      }

      // check include stack for absName
      QStack<QSharedPointer<FileState>> tmpStack;

      while (! g_includeStack.isEmpty()) {
         fs = g_includeStack.pop();

         if (fs->fileName == absName) {
            alreadyIncluded = true;
         }

         tmpStack.push(fs);
      }

      while (! tmpStack.isEmpty()) {
         fs = tmpStack.pop();

         g_includeStack.push(fs);
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
         fs->oldFileBuf    = g_inputBuf;
         fs->oldFileBufPos = g_inputBufPos;
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
         g_yyLineNr = 1;
         return fs;

      } else if (alreadyIncluded) {
         return QSharedPointer<FileState>();
      }
   }

   if (localInclude && ! g_yyFileName.isEmpty()) {
      QFileInfo fi(g_yyFileName);

      if (fi.exists()) {
         QString absName = fi.absolutePath() + "/" + fileName;
         QSharedPointer<FileState> fs = checkAndOpenFile(absName, alreadyIncluded);

         if (fs) {
            setFileName(absName);
            g_yyLineNr = 1;
            return fs;

         } else if (alreadyIncluded) {
            return QSharedPointer<FileState>();
         }
      }
   }

   if (g_pathList.isEmpty()) {
      return QSharedPointer<FileState>();
   }

   for (auto &s : g_pathList) {
      QString absName = s + "/" + fileName;

      QSharedPointer<FileState> fs = checkAndOpenFile(absName, alreadyIncluded);

      if (fs) {
         setFileName(absName);
         g_yyLineNr = 1;
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
      return "";
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
               return "";
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
            return "";
      }

      i--;
   }

   return "";
}

static int getNextChar(const QString &expr, QString *rest, uint &pos);
static int getCurrentChar(const QString &expr, QString *rest, uint pos);
static void unputChar(const QString &expr, QString *rest, uint &pos, char c);
static void expandExpression(QString &expr, QString *rest, int pos);

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

/*! Execute all ## operators in expr.
 * If the macro name before or after the operator contains a no-rescan
 * marker (@-) then this is removed (before the concatenated macro name
 * may be expanded again.
 */
static void processConcatOperators(QString &expr)
{
   QRegExp r("[ \\t\\n]*##[ \\t\\n]*");
   int l;
   int n;
   int i = 0;

   if (expr.isEmpty()) {
      return;
   }

   while ((n = r.indexIn(expr, i)) != -1) {
      l = r.matchedLength();

      if (n + l + 1 < (int)expr.length() && expr.at(n + l) == '@' && expr.at(n + l + 1) == '-') {
         // remove no-rescan marker after ID
         l += 2;
      }

      // remove the ## operator and the surrounding whitespace
      expr  = expr.left(n) + expr.right(expr.length() - n - l);
      int k = n - 1;

      while (k >= 0 && isId(expr.at(k))) {
         k--;
      }

      if (k > 0 && expr.at(k) == '-' && expr.at(k - 1) == '@') {
         // remove no-rescan marker before ID
         expr = expr.left(k - 1) + expr.right(expr.length() - k - 1);
         n -= 2;
      }

      i = n;
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

/*! replaces the function macro \a def whose argument list starts at
 * \a pos in expression \a expr.
 * Notice that this routine may scan beyond the \a expr string if needed.
 * In that case the characters will be read from the input file.
 * The replacement string will be returned in \a result and the
 * length of the (unexpanded) argument list is stored in \a len.
 */
static bool replaceFunctionMacro(const QString &expr, QString *rest, int pos, int &len, QSharedPointer<const A_Define> def, QString &result)
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
      unputChar(expr, rest, j, ' ');
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
               argKey = QString("@%1").arg(argCount++);    // key name
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

            while (!found && (cc = getNextChar(expr, rest, j)) != EOF && cc != 0) {
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
         } else { // append other characters
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
                     expandExpression(substArg, 0, 0);
                  }

                  if (inString) {
                     // if the marker is inside a string (because a # was put
                     // before the macro name), escape " and \ characters
                     resExpr += stringize(substArg);

                  } else {
                     if (hash && substArg.isEmpty()) {
                        // empty argument will be remove later on
                        resExpr += "@E";

                     } else if (g_nospaces) {
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

/*! preforms recursive macro expansion on the string \a expr
 *  starting at position \a pos.
 *  May read additional characters from the input while re-scanning!
 *  If \a expandAll is \c true then all macros in the expression are
 *  expanded, otherwise only the first is expanded.
 */
static void expandExpression(QString &expr, QString *rest, int pos)
{
   QString macroName;
   QString expMacro;

   bool definedTest = false;
   int i = pos;
   int l;
   int p;
   int len;

   while ((p = getNextId(expr, i, &l)) != -1) {
      // search for an macro name

      bool replaced = false;
      macroName = expr.mid(p, l);

      if (p < 2 || ! (expr.at(p - 2) == '@' && expr.at(p - 1) == '-')) {
         // no-rescan marker?

         if (! g_expandedDict->contains(macroName)) {

            // expand macro
            QSharedPointer<A_Define> def = DefineManager::instance().isDefined(macroName);

            if (definedTest) {
               // macro name was found after defined

               if (def) {
                  expMacro = " 1 ";
               } else {
                  expMacro = " 0 ";
               }

               replaced = true;
               len = l;
               definedTest = false;

            } else if (def && def->nargs == -1) {
               // simple macro
               // substitute the definition of the macro

               if (g_nospaces) {
                  expMacro = def->m_definition.trimmed();
               } else {
                  expMacro = " " + def->m_definition.trimmed() + " ";
               }

               replaced = true;
               len = l;

            } else if (def && def->nargs >= 0) {
               // function macro
               replaced = replaceFunctionMacro(expr, rest, p + l, len, def, expMacro);
               len += l;

            } else if (macroName == "defined") {
               definedTest = true;

            }

            if (replaced) {
               // expand the macro and rescan the expression

               //printf("replacing `%s'->`%s'\n",expr.mid(p,len).data(),expMacro.data());
               QString resultExpr = expMacro;
               QString restExpr   = expr.right(expr.length() - len - p);

               processConcatOperators(resultExpr);

               if (def && !def->nonRecursive) {
                  g_expandedDict->insert(macroName, def);
                  expandExpression(resultExpr, &restExpr, 0);
                  g_expandedDict->remove(macroName);
               }

               expr = expr.left(p) + resultExpr + restExpr;
               i    = p;

            } else {
               // move to the next macro name
               i = p + l;
            }

         } else { // move to the next macro name
            expr = expr.left(p) + "@-" + expr.right(expr.length() - p);
            i = p + l + 2;

         }

      } else { // no re-scan marker found, skip the macro name
         i = p + l;
      }
   }
}

/*! replaces all occurrences of @@@@ in \a s by @@
 *  and removes all occurrences of @@E.
 *  All identifiers found are replaced by 0L
 */
QString removeIdsAndMarkers(const QString &s)
{
   const QChar *p = s.constData();
   const QChar *ptr = p;
   QChar c;

   bool inNum = false;
   QString result;

   if (p) {

      while ((c = *p) != 0) {
         // replace @@ with @ and remove @E

         if (c == '@') {

            if (*(p + 1) == '@') {
               result += c;

            } else if (*(p + 1) == 'E') {
               // skip
            }

            p += 2;

         } else if (c.isNumber()) {
            // number

            result += c;
            p++;
            inNum = true;

         } else if (c == 'd' && ! inNum) {
            // identifier starting with a `d'

            if (s.mid(p - ptr, 8) == "defined " || s.mid(p - ptr, 8) == "defined(") {
               // defined keyword, skip defined
               p += 7;

            } else {
               result += "0L";
               p++;

               while ((c = *p) != 0 && isId(c)) {
                  p++;
               }
            }

         } else if ((c.isLetter() || c == '_') && ! inNum) {
            // replace identifier with 0L
            result += "0L";
            p++;

            while ((c = *p) != 0 && isId(c)) {
               p++;
            }

            if (*p == '(') { // undefined function macro
               p++;
               int count = 1;

               while ((c = *p++) != 0 ) {
                  if (c == '(') {
                     count++;

                  } else if (c == ')') {
                     count--;
                     if (count == 0) {
                        break;
                     }

                  } else if (c == '/') {
                     QChar pc = c;
                     c = *++p;

                     if (c == '*') { // start of C comment
                        while (*p != 0 && ! (pc == '*' && c == '/')) {
                           // search end of comment
                           pc = c;
                           c = *++p;
                        }
                        p++;
                     }
                  }
               }
            }

         } else if (c == '/') {
            // skip C comments

            QChar pc = c;
            c = *++p;

            if (c == '*') {
               // start of C comment

               while (*p != 0 && ! (pc == '*' && c == '/')) {
                  // search end of comment
                  pc = c;
                  c = *++p;
               }
               p++;

            } else { // not comment but division
               result += pc;
               goto nextChar;
            }

         } else {

         nextChar:
            result += c;
            QChar lc = c.toLower();

            if (! isId(lc) && lc != '.' /*&& lc!='-' && lc!='+'*/) {
               inNum = false;
            }
            p++;
         }
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
   const QChar *p = s.constData();
   QChar c;

   QString result;

   while ((c = *p) != 0) {
      switch (c.unicode()) {
         case '@': { // replace @@ with @
            if (*(p + 1) == '@') {
               result += c;
            }
            p += 2;
         }
         break;

         case '/': { // skip C comments
            result  += c;
            QChar pc = c;
            c = *++p;

            if (c == '*') { // start of C comment

               while (*p != 0  && !(pc == '*' && c == '/')) { // search end of comment
                  if (*p == '@' && *(p + 1) == '@') {
                     result += c;
                     p++;

                  } else {
                     result += c;
                  }

                  pc = c;
                  c  = *++p;
               }

               if (*p != 0) {
                  result += c;
                  p++;
               }
            }
         }
         break;

         case '"': { // skip string literals
            result += c;
            QChar pc = c;
            c = *++p;

            while (*p != 0 && (c != '"' || pc == '\\')) { // no end quote
               result += c;
               c = *++p;
            }
            if (*p != 0) {
               result += c;
               p++;
            }
         }
         break;

         case '\'': { // skip char literals
            result += c;
            QChar pc = c;
            c = *++p;

            while (*p != 0 && (c != '\'' || pc == '\\')) {
               // no end quote
               result += c;
               c = *++p;
            }

            if (*p != 0) {
               result += c;
               p++;
            }
         }
         break;

         default: {
            result += c;
            p++;
         }
         break;
      }
   }


   return result;
}

/*! compute the value of the expression in string \a expr.
 *  If needed the function may read additional characters from the input.
 */
bool computeExpression(const QString &expr)
{
   QString e = expr;

   expandExpression(e, 0, 0);
   e = removeIdsAndMarkers(e);

   if (e.isEmpty()) {
      return false;
   }

   bool retval = parseconstexp(g_yyFileName, g_yyLineNr, e);

   return retval;
}

/*! expands the macro definition in \a name
 *  If needed the function may read additional characters from the input
 */

QString expandMacro(const QString &name)
{
   QString n = name;

   expandExpression(n, 0, 0);
   n = removeMarkers(n);

   return n;
}

QSharedPointer<A_Define> newDefine()
{
   QSharedPointer<A_Define> def = QMakeShared<A_Define>();

   def->m_name       = g_defName;
   def->m_definition = g_defText.trimmed();
   def->nargs        = g_defArgs;
   def->m_fileName   = g_yyFileName;
   def->fileDef      = g_yyFileDef;
   def->lineNr       = g_yyLineNr - g_yyMLines;
   def->columnNr     = g_yyColNr;
   def->varArgs      = g_defVarArgs;

   if (! def->m_name.isEmpty() && Doxy_Globals::expandAsDefinedDict.contains(def->m_name)) {
      def->isPredefined = true;
   }

   return def;
}

void addDefine()
{
   if (g_skip) {
      return;   // do not add this define as it is inside a
   }

   // conditional section (cond command) that is disabled
   if (! Doxy_Globals::gatherDefines) {
      return;
   }

   QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(g_yyFileName, g_yyLineNr - g_yyMLines, g_yyColNr,
               "#define", g_defName, g_defArgsStr, "", Public, Normal, false, Member,
               MemberType_Define, ArgumentList(), ArgumentList());

   if (! g_defArgsStr.isEmpty()) {
      ArgumentList argList;

      argList = stringToArgumentList(g_defArgsStr);
      md->setArgumentList(argList);
   }

   int l = g_defLitText.indexOf('\n');

   if (l > 0 && g_defLitText.left(l).trimmed() == "\\") {
      // strip first line if it only contains a slash
      g_defLitText = g_defLitText.right(g_defLitText.length() - l - 1);

   } else if (l > 0) {
      // align the items on the first line with the items on the second line
      int k = l + 1;

      const QChar *p = g_defLitText.data() + k;
      QChar c;

      while ((c = *p++) != 0 && (c == ' ' || c == '\t')) {
         k++;
      }

      g_defLitText = g_defLitText.mid(l + 1, k - l - 1) + g_defLitText.trimmed();
   }
   md->setInitializer(g_defLitText.trimmed());

   md->setFileDef(g_inputFileDef);
   md->setDefinition("#define " + g_defName);

   QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict.find(g_defName);

   if (! mn) {
      mn = QMakeShared<MemberName>(g_defName);
      Doxy_Globals::functionNameSDict.insert(g_defName, mn);
   }

   mn->append(md);

   if (g_yyFileDef) {
      g_yyFileDef->insertMember(md);
   }
}

static inline void outputChar(QChar c)
{
   if (g_includeStack.isEmpty() || g_curlyCount > 0) {
      g_outputBuf += c;
   }
}

static inline void outputArray(const QString &a, int len)
{
   if (g_includeStack.isEmpty() || g_curlyCount > 0) {
      g_outputBuf += a.mid(0, len);
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

      QString dosExt = incFileName.right(4);
      if (dosExt == ".exe" || dosExt == ".dll" || dosExt == ".tlb") {
         // skip imported binary files (e.g. M$ type libraries)
         return;
      }

      QString oldFileName = g_yyFileName;

      QSharedPointer<FileDef> oldFileDef = g_yyFileDef;
      int oldLineNr = g_yyLineNr;

      // absIncFileName avoids difficulties for incFileName starting with "../" (bug 641336)
      QString absIncFileName = incFileName;
      {
         QFileInfo fi(g_yyFileName);

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

      DefineManager::instance().addInclude(g_yyFileName, absIncFileName);
      DefineManager::instance().addFileToContext(absIncFileName);

      // findFile will overwrite g_yyFileDef if found
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

            oldFileDef->addIncludeDependency(temp, incFileName, localInclude, g_isImported, false);

            // add included by dependency
            if (g_yyFileDef) {
               g_yyFileDef->addIncludedByDependency(oldFileDef, oldFileDef->docName(), localInclude, g_isImported);
            }

         } else if (g_inputFileDef) {
            g_inputFileDef->addIncludeDependency(QSharedPointer<FileDef>(), absIncFileName, localInclude, g_isImported, true);

         }

         fs->bufState = YY_CURRENT_BUFFER;
         fs->lineNr   = oldLineNr;
         fs->fileName = oldFileName;

         // push the state on the stack
         g_includeStack.push(fs);

         // set the scanner to the include file

         // Deal with file changes due to
         // #include's within { .. } blocks

         QString lineStr = QString("# 1 \"%1\" 1\n").arg(QString(g_yyFileName));
         outputArray(lineStr, lineStr.length());

         DBG_CTX((stderr, "Switching to include file %s\n", csPrintable(incFileName)));
         g_expectGuard = true;
         g_inputBuf    = fs->fileBuf;
         g_inputBufPos = 0;

         preYY_switch_to_buffer(preYY_create_buffer(0, YY_BUF_SIZE));

      } else {

         if (oldFileDef) {
            bool ambig;

            // change to absolute name for bug 641336
            QSharedPointer<FileDef> fd = findFileDef(&Doxy_Globals::inputNameDict, absIncFileName, ambig);

            // add include dependency to the file in which the #include was found
            oldFileDef->addIncludeDependency(ambig ? QSharedPointer<FileDef>() : fd, incFileName, localInclude, g_isImported, false);

            // add included by dependency
            if (fd) {
               fd->addIncludedByDependency(oldFileDef, oldFileDef->docName(), localInclude, g_isImported);
            }

         } else if (g_inputFileDef) {
            g_inputFileDef->addIncludeDependency(QSharedPointer<FileDef>(), absIncFileName, localInclude, g_isImported, true);
         }

         if (Debug::isFlagSet(Debug::Preprocessor)) {
            if (alreadyIncluded) {
               Debug::print(Debug::Preprocessor, 0, "#include %s: already included, skipping...\n", csPrintable(incFileName));
            } else {
               Debug::print(Debug::Preprocessor, 0, "#include %s: not found, skipping...\n", csPrintable(incFileName));
            }
         }

         if (g_curlyCount > 0 && !alreadyIncluded) { // failed to find #include inside { ... }
            warn(g_yyFileName, g_yyLineNr, "include file %s was not found, check the value of INCLUDE_PATH", csPrintable(incFileName));
         }
      }
   }
}

static void startCondSection(const QString &sectId)
{
   CondParser prs;

   bool expResult = prs.parse(g_yyFileName, g_yyLineNr, sectId);
   g_condStack.push( QMakeShared<CondCtx>(g_yyLineNr, sectId, g_skip) );

   if (! expResult) {
      g_skip = true;
   }
}

static void endCondSection()
{
   if (g_condStack.isEmpty()) {
      g_skip = false;

   } else {
      QSharedPointer<CondCtx> ctx = g_condStack.pop();
      g_skip = ctx->skip;
   }
}

static void forceEndCondSection()
{
   while (! g_condStack.isEmpty()) {
      g_condStack.pop();
   }

   g_skip = false;
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
#define YY_INPUT(buf,result,max_size)   result=yyread(buf,max_size);

static int yyread(char *buf, int max_size)
{
   int c = 0;

   while (g_inputBuf[g_inputBufPos] != 0) {

      QString tmp1    = g_inputBuf.at(g_inputBufPos);
      QByteArray tmp2 = tmp1.toUtf8();

      if (c + tmp2.length() >= max_size)  {
         // buffer is full
         break;
      }

      c += tmp2.length();

      for (auto letters : tmp2) {
         *buf = letters;
          buf++;
      }

      g_inputBufPos++;
   }

   return c;
}

%}

ID       [a-z_A-Z\x80-\xFF][a-z_A-Z0-9\x80-\xFF]*
B        [ \t]
BN       [ \t\r\n]
CHARLIT  (("'"\\[0-7]{1,3}"'")|("'"\\."'")|("'"[^'\\\n]{1,4}"'"))

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
      g_yyColNr  += yyleng;
      g_yyMLines = 0;
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

      if (skipFuncMacros && name != "Q_PROPERTY" && ! ( (g_includeStack.isEmpty() || g_curlyCount > 0) && g_macroExpansion &&
                  (def = DefineManager::instance().isDefined(name)) && (! g_expandOnlyPredef || def->isPredefined)) ) {

         outputChar('\n');
         g_yyLineNr++;

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

      g_yyLineNr += text.count('\n');
      outputArray(text, text.length());
   }

<CopyLine>"{"           {
      // count brackets inside the main file
      QString text = QString::fromUtf8(yytext);

      if (g_includeStack.isEmpty())  {
         g_curlyCount++;
      }

      outputChar(text[0]);
   }

<CopyLine>"}"           {
      // count brackets inside the main file
      QString text = QString::fromUtf8(yytext);

      if (g_includeStack.isEmpty() && g_curlyCount > 0)  {
         g_curlyCount--;
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
      if (getLanguageFromFileName(g_yyFileName) != SrcLangExt_Fortran) {
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

      g_expectGuard = false;
      QSharedPointer<A_Define> def;

      if ((g_includeStack.isEmpty() || g_curlyCount > 0) && g_macroExpansion &&
                  (def = DefineManager::instance().isDefined(text)) &&
                  (! g_expandOnlyPredef || def->isPredefined)) {

         // fount it
         g_roundCount = 0;
         g_defArgsStr = text;

         if (def->nargs == -1) {
            // no function macro

            QString result = def->isPredefined ? def->m_definition : expandMacro(g_defArgsStr);
            outputArray(result, result.length());

         } else {
            // zero or more arguments

            g_findDefArgContext = CopyLine;
            BEGIN(FindDefineArgs);

         }

      } else {
         outputArray(text, text.length());
      }
   }

<CopyLine>{ID}             {
      QString text = QString::fromUtf8(yytext);

      QSharedPointer<A_Define> def;

      if ((g_includeStack.isEmpty() || g_curlyCount > 0) && g_macroExpansion &&
                  (def = DefineManager::instance().isDefined(text)) &&
                   def->nargs == -1 && (! g_expandOnlyPredef || def->isPredefined)) {

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
      g_yyLineNr++;
      g_yyColNr = 1;
   }

<FindDefineArgs>"("        {
      g_defArgsStr += '(';
      g_roundCount++;
   }

<FindDefineArgs>")"        {
      g_defArgsStr += ')';
      g_roundCount--;

      if (g_roundCount == 0) {
         QString result = expandMacro(g_defArgsStr);

         if (g_findDefArgContext == CopyLine) {
            outputArray(result, result.length());
            BEGIN(g_findDefArgContext);

         } else {
            readIncludeFile(result);
            g_nospaces = false;
            BEGIN(Start);
         }
      }
   }

  /*
<FindDefineArgs>")"{B}*"("       {
      QString text = QString::fromUtf8(yytext);
      g_defArgsStr += text;
   }
  */

<FindDefineArgs>{CHARLIT}     {
      QString text = QString::fromUtf8(yytext);
      g_defArgsStr += text;
    }

<FindDefineArgs>"/*"[*]?                {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);
      g_defArgsStr += text;
      BEGIN(ArgCopyCComment);
   }

<FindDefineArgs>\"         {
      QString text = QString::fromUtf8(yytext);
      g_defArgsStr += text[0];
      BEGIN(ReadString);
   }

<FindDefineArgs>\n         {
      g_defArgsStr += ' ';
      g_yyLineNr++;
      outputChar('\n');
   }

<FindDefineArgs>"@"        {
      g_defArgsStr += "@@";
   }

<FindDefineArgs>.          {
      QString text = QString::fromUtf8(yytext);
      g_defArgsStr += text[0];
   }

<ArgCopyCComment>[^*\n]+      {
      g_defArgsStr += QString::fromUtf8(yytext);
   }

<ArgCopyCComment>"*/"         {
      g_defArgsStr += QString::fromUtf8(yytext);
      BEGIN(FindDefineArgs);
   }

<ArgCopyCComment>\n        {
      g_defArgsStr += ' ';
      g_yyLineNr++;
      outputChar('\n');
   }

<ArgCopyCComment>.         {
      g_defArgsStr += QString::fromUtf8(yytext);
   }

<ReadString>"\""        {
      QString text = QString::fromUtf8(yytext);
      g_defArgsStr += text[0];
      BEGIN(FindDefineArgs);
   }

<ReadString>"//"|"/*"         {
      // */ (editor syntax fix)
      g_defArgsStr += QString::fromUtf8(yytext);
   }

<ReadString>\\.            {
      g_defArgsStr += QString::fromUtf8(yytext);
   }

<ReadString>.           {
      QString text = QString::fromUtf8(yytext);
      g_defArgsStr += text[0];
   }

<Command>("include"|"import"){B}+/{ID}    {
      QString text = QString::fromUtf8(yytext);
      g_isImported = text[1] == 'm';

      if (g_macroExpansion) {
         BEGIN(IncludeID);
      }
   }

<Command>("include"|"import"){B}*[<"]  {
      QString text = QString::fromUtf8(yytext);
      g_isImported = text[1] == 'm';
      g_incName = text[text.length() - 1];
      BEGIN(Include);
   }

<Command>("cmake")?"define"{B}+     {
      g_yyColNr += yyleng;
      BEGIN(DefName);
   }

<Command>"ifdef"/{B}*"("      {
      incrLevel();
      g_guardExpr.resize(0);
      BEGIN(DefinedExpr2);
   }

<Command>"ifdef"/{B}+         {
      incrLevel();
      g_guardExpr.resize(0);
      BEGIN(DefinedExpr1);
   }

<Command>"ifndef"/{B}*"("     {
      incrLevel();
      g_guardExpr = "! ";
      BEGIN(DefinedExpr2);
   }

<Command>"ifndef"/{B}+        {
      incrLevel();
      g_guardExpr = "! ";
      BEGIN(DefinedExpr1);
   }

<Command>"if"/[ \t(!]         {
      incrLevel();
      g_guardExpr.resize(0);
      BEGIN(Guard);
   }

<Command>("elif"|"else"{B}*"if")/[ \t(!]  {
      if (! otherCaseDone()) {
         g_guardExpr.resize(0);
         BEGIN(Guard);

      } else {
         g_ifcount = 0;
         BEGIN(SkipCPPBlock);
      }
   }

<Command>"else"/[^a-z_A-Z0-9\x80-\xFF]       {

      if (otherCaseDone()) {
         g_ifcount = 0;
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
         g_guardExpr.resize(0);
         BEGIN(Guard);
      }
   }

<Command>"endif"/[^a-z_A-Z0-9\x80-\xFF]      {
      decrLevel();
   }

<Command,IgnoreLine>\n        {
      outputChar('\n');
      BEGIN(Start);
      g_yyLineNr++;
   }

<Command>"pragma"{B}+"once"   {
      g_expectGuard = false;
   }

<Command>{ID}                 {
      // unknown directive
      BEGIN(IgnoreLine);
   }

<IgnoreLine>\\[\r]?\n         {
      outputChar('\n');
      g_yyLineNr++;
   }

<IgnoreLine>.
<Command>. {
      g_yyColNr += yyleng;
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
      g_guardExpr += ' ';
      g_yyLineNr++;
   }

<Guard>"defined"/{B}*"("      {
      BEGIN(DefinedExpr2);
   }

<Guard>"defined"/{B}+         {
      BEGIN(DefinedExpr1);
   }

<Guard>{ID}      {
      QString text = QString::fromUtf8(yytext);
      g_guardExpr += text;
   }

<Guard>.         {
      QString text = QString::fromUtf8(yytext);
      g_guardExpr += text[0];
   }

<Guard>\n            {
      unput(*yytext);

      bool guard = computeExpression(g_guardExpr);
      setCaseDone(guard);

      if (guard) {
         BEGIN(Start);

      } else {
         g_ifcount = 0;
         BEGIN(SkipCPPBlock);
      }
   }

<DefinedExpr1,DefinedExpr2>\\\n     {
      g_yyLineNr++;
      outputChar('\n');
   }

<DefinedExpr1>{ID}         {
      QString text = QString::fromUtf8(yytext);

      if (DefineManager::instance().isDefined(text) || g_guardName == text) {
         g_guardExpr+=" 1L ";
      } else {
         g_guardExpr+=" 0L ";
      }

      g_lastGuardName = text;
      BEGIN(Guard);
   }

<DefinedExpr2>{ID}         {
      QString text = QString::fromUtf8(yytext);

      if (DefineManager::instance().isDefined(text) || g_guardName == text){
         g_guardExpr+=" 1L ";
      } else {
         g_guardExpr+=" 0L ";
      }

      g_lastGuardName = text;
   }

<DefinedExpr1,DefinedExpr2>\n       {
      // should not happen, handle anyway

      g_yyLineNr++;
      g_ifcount = 0;
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
      g_yyLineNr++;
      outputChar('\n');
   }

<SkipCPPBlock>.
<SkipCommand>"if"(("n")?("def"))?/[ \t(!]    {
      incrLevel();
      g_ifcount++;
   }

<SkipCommand>"else"        {
      // printf("Else! g_ifcount=%d otherCaseDone=%d\n",g_ifcount,otherCaseDone());

      if (g_ifcount == 0 && ! otherCaseDone()) {
         setCaseDone(true);
         BEGIN(Start);
      }
   }

<SkipCommand>("elif"|"else"{B}*"if")/[ \t(!]       {
      if (g_ifcount == 0)  {

         if (! otherCaseDone()) {
            g_guardExpr.resize(0);
            g_lastGuardName.resize(0);
            BEGIN(Guard);

         } else {
            BEGIN(SkipCPPBlock);
         }
      }
   }

<SkipCommand>"endif"          {
      g_expectGuard = false;
      decrLevel();

      if (--g_ifcount < 0) {
         BEGIN(Start);
      }
   }

<SkipCommand>\n            {
      outputChar('\n');
      g_yyLineNr++;
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
      g_lastCPPContext = YY_START;
      BEGIN(RemoveCPPComment);
   }

<SkipString>"/*"/[^\n]*                 {
      // */ (editor syntax fix)
   }

<SkipLine,SkipCommand,SkipCPPBlock>"/*"/[^\n]* {
      // */ (editor syntax fix)

      g_lastCContext=YY_START;
      BEGIN(RemoveCComment);
   }

<SkipLine>\n            {
      outputChar('\n');
      g_yyLineNr++;
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

      g_nospaces   = true;
      g_roundCount = 0;
      g_defArgsStr = text;
      g_findDefArgContext = IncludeID;
      BEGIN(FindDefineArgs);
   }

<IncludeID>{ID}            {
      QString text = QString::fromUtf8(yytext);

      g_nospaces = true;
      readIncludeFile(expandMacro(text));
      BEGIN(Start);
   }

<Include>[^\">\n]+[\">]          {
      QString text = QString::fromUtf8(yytext);

      g_incName += text;
      readIncludeFile(g_incName);

      if (g_isImported) {
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
      g_yyLineNr++;
   }

<EndImport>.            {
   }

<DefName>{ID}/("\\\n")*"("       {
      // define with argument

      g_argDict = QMakeShared<QHash<QString, int>>();

      g_defArgs = 0;
      g_defArgsStr.resize(0);
      g_defText.resize(0);
      g_defLitText.resize(0);

      g_defName    = QString::fromUtf8(yytext);
      g_defVarArgs = false;
      g_defExtraSpacing.resize(0);
      BEGIN(DefineArg);
   }

<DefName>{ID}{B}+"1"/[ \r\t\n]      {
      // special case: define with 1 -> can be "guard"

      g_argDict = QSharedPointer<QHash<QString, int>>();

      g_defArgs = -1;
      g_defArgsStr.resize(0);

      g_defName = QString::fromUtf8(yytext);
      g_defName = g_defName.left(g_defName.length() - 1).trimmed();
      g_defVarArgs = false;

      if (g_curlyCount > 0 || g_defName != g_lastGuardName || ! g_expectGuard) {
         // define may appear in the output
         QString tmp = "#define " + g_defName;
         outputArray(tmp, tmp.length());

         g_quoteArg      = false;
         g_insideComment = false;
         g_lastGuardName.resize(0);
         g_defText    = "1";
         g_defLitText = "1";
         BEGIN(DefineText);

      } else  {
         // define is a guard => hide

         g_defText.resize(0);
         g_defLitText.resize(0);
         BEGIN(Start);
      }

      g_expectGuard = false;
   }

<DefName>{ID}/{B}*"\n"        {
      // empty define
      QString text = QString::fromUtf8(yytext);

      g_argDict = QSharedPointer<QHash<QString, int>>();

      g_defArgs = -1;
      g_defName = text;

      g_defArgsStr.resize(0);
      g_defText.resize(0);
      g_defLitText.resize(0);
      g_defVarArgs = false;

      if (g_curlyCount > 0 || g_defName != g_lastGuardName || ! g_expectGuard) {
         // define may appear in the output
         QString tmp = "#define " + g_defName;
         outputArray(tmp, tmp.length());

         g_quoteArg      = false;
         g_insideComment = false;

         if (g_insideCS) {
            g_defText="1"; // for C#, use "1" as define text
         }

         BEGIN(DefineText);

      } else  {
         // define is a guard => hide

         g_guardName = text;
         g_lastGuardName.resize(0);
         BEGIN(Start);
      }

      g_expectGuard = false;
   }

<DefName>{ID}/{B}*         {
      // define with content
      QString text = QString::fromUtf8(yytext);

      g_argDict = QSharedPointer<QHash<QString, int>>();

      g_defArgs = -1;
      g_defArgsStr.resize(0);

      g_defText.resize(0);
      g_defLitText.resize(0);
      g_defName    = text;
      g_defVarArgs = false;

      QString tmp = "#define " + g_defName + g_defArgsStr;
      outputArray(tmp, tmp.length());

      g_quoteArg      = false;
      g_insideComment = false;
      BEGIN(DefineText);
   }

<DefineArg>"\\\n"              {
      g_defExtraSpacing += "\n";
      g_yyLineNr++;
   }

<DefineArg>","{B}*             {
      QString text = QString::fromUtf8(yytext);
      g_defArgsStr += text;
   }

<DefineArg>"("{B}*             {
      QString text = QString::fromUtf8(yytext);
      g_defArgsStr += text;
   }

<DefineArg>{B}*")"{B}*         {
      QString text = QString::fromUtf8(yytext);
      g_defArgsStr += text;

      QString tmp = "#define " + g_defName + g_defArgsStr + g_defExtraSpacing;
      outputArray(tmp, tmp.length());

      g_quoteArg      = false;
      g_insideComment = false;
      BEGIN(DefineText);
   }

<DefineArg>"..."        {
      // Variadic macro
      g_defVarArgs = true;
      g_defArgsStr +=  QString::fromUtf8(yytext);

      g_argDict->insert("__VA_ARGS__", g_defArgs);
      g_defArgs++;
   }

<DefineArg>{ID}{B}*("..."?)      {
      QString text = QString::fromUtf8(yytext);
      g_defArgsStr += text;

      g_defVarArgs = text.endsWith("...");

      if (g_defVarArgs)  {
         // strip ellipsis
         text = text.left(text.length() - 3);
      }

      text = text.trimmed();

      g_argDict->insert(text, g_defArgs);
      g_defArgs++;
   }

  /*
<DefineText>"/ **"|"/ *!"  {
      g_defText+=yytext;
      g_defLitText+=yytext;
      g_insideComment=true;
   }

<DefineText>"* /"          {
      g_defText+=yytext;
      g_defLitText+=yytext;
      g_insideComment=false;
   }
  */

<DefineText>"/*"[!*]?         {
      // */ (editor syntax fix)
       QString text = QString::fromUtf8(yytext);

      g_defText   += text;
      g_defLitText+= text;
      g_lastCContext = YY_START;
      g_commentCount = 1;
      BEGIN(CopyCComment);
   }

<DefineText>"//"[!/]?         {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());

      g_lastCPPContext  = YY_START;
      g_defLitText     += ' ';
      BEGIN(SkipCPPComment);
   }

<SkipCComment>[/]?"*/"        {
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '/') {
         outputChar('/');
      }

      outputChar('*');
      outputChar('/');

      if (--g_commentCount <= 0) {

         if (g_lastCContext == Start) {
            // small hack to make sure that ^... rule will
            // match when going to Start... Example: "/*...*/ some stuff..."

            YY_CURRENT_BUFFER->yy_at_bol = 1;
         }

         BEGIN(g_lastCContext);
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

      // g_commentCount++;
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

         g_fenceSize = yyleng;
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

         g_fenceSize = yyleng;
         BEGIN(SkipVerbatim);
      }
   }

<SkipCComment>[\\@][\\@]("verbatim"|"latexonly"|"htmlonly"|"xmlonly"|"docbookonly"|"rtfonly"|"manonly"|"dot"|"code"("{"[^}]*"}")?){BN}+ {
      QString text = QString::fromUtf8(yytext);

      outputArray(text, text.length());
      g_yyLineNr += text.count('\n');
   }

<SkipCComment>[\\@]("verbatim"|"latexonly"|"htmlonly"|"xmlonly"|"docbookonly"|"rtfonly"|"manonly"|"dot"|"code"("{"[^}]*"}")?){BN}+  {
      QString text = QString::fromUtf8(yytext);

      outputArray(text, text.length());

      g_yyLineNr += text.count('\n');
      g_fenceSize = 0;

      if (text[1] == 'f') {
         g_blockName = "f";

      } else {
         QString bn = text.mid(1);
         int i = bn.indexOf('{');

         // for \code{.c}
         if (i != -1) {
            bn = bn.left(i);
         }

         g_blockName = bn.trimmed();
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
      g_ccomment = true;
      g_condCtx  = YY_START;
      BEGIN(CondLineCpp);
   }

<SkipCComment>[\\@]"cond"[ \t]+  {
      // conditional section
      g_ccomment = false;
      g_condCtx  = YY_START;
      BEGIN(CondLineC);
   }

<CondLineC,CondLineCpp>[!()&| \ta-z_A-Z0-9\x80-\xFF.\-]+      {
      QString text = QString::fromUtf8(yytext);
      startCondSection(text);

      if (g_skip) {
         if (YY_START == CondLineC) {
            // end C comment
            outputArray("*/", 2);
            g_ccomment = true;

         } else {
            g_ccomment = false;
         }

         BEGIN(SkipCond);

      } else {
         BEGIN(g_condCtx);

      }
   }

<CondLineC,CondLineCpp>.      {
      // non-guard character
      unput(*yytext);
      startCondSection(" ");

      if (g_skip) {
         if (YY_START == CondLineC) {
            // end C comment
            outputArray("*/",2);
            g_ccomment = true;

         } else {
            g_ccomment = false;
         }

         BEGIN(SkipCond);

      } else {
         BEGIN(g_condCtx);
      }
   }

<SkipCComment,SkipCPPComment>[\\@]"cond"[ \t\r]*/\n {
      // no guard
      if (YY_START == SkipCComment) {
         g_ccomment=true;
         // end C comment
         outputArray("*/",2);

      } else {
         g_ccomment = false;
      }

      g_condCtx = YY_START;
      startCondSection(" ");
      BEGIN(SkipCond);
   }

<SkipCond>\n                            {
      g_yyLineNr++;
      outputChar('\n');
   }

<SkipCond>.                             {
   }

<SkipCond>[^\/\!*\\@\n]+                {
   }

<SkipCond>"//"[/!]                      {
      g_ccomment = false;
   }

<SkipCond>"/*"[*!]                      {
      // */ (editor syntax fix)
      g_ccomment = true;
   }

<SkipCond,SkipCComment,SkipCPPComment>[\\@][\\@]"endcond"/[^a-z_A-Z0-9\x80-\xFF] {
      if (! g_skip) {
         QString text = QString::fromUtf8(yytext);
         outputArray(text, text.length());
      }
   }

<SkipCond>[\\@]"endcond"/[^a-z_A-Z0-9\x80-\xFF]  {
      bool oldSkip = g_skip;
      endCondSection();

      if (oldSkip && ! g_skip) {
         if (g_ccomment) {
            outputArray("/** ",4);

            // */ (editor syntax fix)
         }

         BEGIN(g_condCtx);
      }
   }

<SkipCComment,SkipCPPComment>[\\@]"endcond"/[^a-z_A-Z0-9\x80-\xFF] {
      // */ (editor syntax fix)
      bool oldSkip = g_skip;
      endCondSection();

      if (oldSkip && ! g_skip)  {
         BEGIN(g_condCtx);
      }
   }

<SkipVerbatim>[\\@]("endverbatim"|"endlatexonly"|"endhtmlonly"|"endxmlonly"|"enddocbookonly"|"endrtfonly"|"endmanonly"|"enddot"|"endcode"|"f$"|"f]"|"f}") {
      /* end of verbatim block */

      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());

      if (text[1] == 'f' && g_blockName == "f") {
         BEGIN(SkipCComment);

      } else if (text.mid(4) == g_blockName) {
         BEGIN(SkipCComment);
      }
   }

<SkipVerbatim>^({B}*"*"+)?{B}{0,3}"~~~"[~]*                 {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());

      if (g_fenceSize == yyleng) {
         BEGIN(SkipCComment);
      }
   }

<SkipVerbatim>^({B}*"*"+)?{B}{0,3}"```"[`]*                 {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());

      if (g_fenceSize == yyleng) {
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
      g_yyLineNr++;
      outputChar('\n');
   }

<SkipCComment,SkipVerbatim>.     {
      outputChar(*yytext);
   }

<CopyCComment>[^*a-z_A-Z\x80-\xFF\n]*[^*a-z_A-Z\x80-\xFF\\\n] {
      QString text = QString::fromUtf8(yytext);

      g_defLitText += text;
      g_defText    += escapeAt(text);
   }

<CopyCComment>\\[\r]?\n    {
      QString text = QString::fromUtf8(yytext);
      g_defLitText += text;

      outputChar('\n');
      g_defText += " ";
      g_yyLineNr++;
      g_yyMLines++;
   }

<CopyCComment>"*/"         {
      QString text = QString::fromUtf8(yytext);

      g_defLitText += text;
      g_defText    += text;
      BEGIN(g_lastCContext);
   }

<CopyCComment>\n        {
      QString text = QString::fromUtf8(yytext);

      g_yyLineNr++;
      outputChar('\n');
      g_defLitText += text;
      g_defText+=' ';
   }

<RemoveCComment>"*/"{B}*"#"           {
      // see bug 594021 for a usecase for this rule
      if (g_lastCContext == SkipCPPBlock) {
         BEGIN(SkipCommand);

      } else {
         REJECT;
      }
   }

<RemoveCComment>"*/"               {
      BEGIN(g_lastCContext);
   }

<RemoveCComment>"//"
<RemoveCComment>"/*"
<RemoveCComment>[^*\x06\n]+
<RemoveCComment>\n         {
      // */ (editor syntax fix)

      g_yyLineNr++;
      outputChar('\n');
   }

<RemoveCComment>.
<SkipCPPComment>[^\n\/\\@]+      {
      QString text = QString::fromUtf8(yytext);
      outputArray(text, text.length());
   }

<SkipCPPComment,RemoveCPPComment>\n    {
      unput(*yytext);
      BEGIN(g_lastCPPContext);
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

      g_quoteArg   = true;
      g_defLitText += QString::fromUtf8(yytext);
   }

<DefineText,CopyCComment>{ID}       {
      QString text = QString::fromUtf8(yytext);
      g_defLitText += text;

      if (g_quoteArg) {
         g_defText+="\"";
      }

      if (g_defArgs > 0) {
         auto iter = g_argDict->find(text);

         if (iter != g_argDict->end()) {
            int n = iter.value();

            g_defText += '@';

            QString numStr = QString("%1").arg(n);
            g_defText += numStr;

         } else {
            g_defText += text;

         }

      } else {
         g_defText += text;
      }

      if (g_quoteArg) {
         g_defText += "\"";
      }

      g_quoteArg = false;
   }

<CopyCComment>.            {
      QString text = QString::fromUtf8(yytext);

      g_defLitText += text;
      g_defText    += text;
   }

<DefineText>\\[\r]?\n         {
      QString text = QString::fromUtf8(yytext);

      g_defLitText += text;
      outputChar('\n');
      g_defText += ' ';
      g_yyLineNr++;
      g_yyMLines++;
   }

<DefineText>\n             {
      QString text = QString::fromUtf8(yytext);

      QString comment = extractTrailingComment(g_defLitText);
      g_defLitText += text;

      if (! comment.isEmpty()) {
         outputArray(comment, comment.length());
         g_defLitText = g_defLitText.left(g_defLitText.length() - comment.length() - 1);
      }

      outputChar('\n');

      QSharedPointer<A_Define> def;

      if (g_includeStack.isEmpty() || g_curlyCount > 0) {
         addDefine();
      }

      def = DefineManager::instance().isDefined(g_defName);

      if (def == nullptr)  {
         // new define

         QSharedPointer<A_Define> nd = newDefine();
         DefineManager::instance().addDefine(g_yyFileName, nd);

      } else if (def) {
         // name already exists

         if (def->undef) {
            // undefined name
            def->undef        = false;
            def->m_name       = g_defName;
            def->m_definition = g_defText.trimmed();
            def->nargs        = g_defArgs;
            def->m_fileName   = g_yyFileName;
            def->lineNr       = g_yyLineNr - g_yyMLines;
            def->columnNr     = g_yyColNr;
         }
      }

      g_argDict = QSharedPointer<QHash<QString, int>>();

      g_yyLineNr++;
      g_yyColNr = 1;
      g_lastGuardName.resize(0);
      BEGIN(Start);
   }

<DefineText>{B}*        {
      QString text = QString::fromUtf8(yytext);
      g_defText    += ' ';
      g_defLitText += text;
   }

<DefineText>{B}*"##"{B}*      {
      QString text = QString::fromUtf8(yytext);
      g_defText    += "##";
      g_defLitText += text;
   }

<DefineText>"@"            {
      QString text = QString::fromUtf8(yytext);
      g_defText     += "@@";
      g_defLitText  += text;
   }

<DefineText>\"             {
      QString text = QString::fromUtf8(yytext);

      g_defText    += text[0];
      g_defLitText += text;

      if (! g_insideComment) {
         BEGIN(SkipDoubleQuote);
      }
   }

<DefineText>\'             {
      QString text = QString::fromUtf8(yytext);

      g_defText    += text[0];
      g_defLitText += text;

      if (! g_insideComment) {
         BEGIN(SkipSingleQuote);
      }
   }

<SkipDoubleQuote>"//"[/]?     {
      QString text = QString::fromUtf8(yytext);

      g_defText    += text;
      g_defLitText += text;
   }

<SkipDoubleQuote>"/*"         {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      g_defText    += text;
      g_defLitText += text;
   }

<SkipDoubleQuote>\"        {
      QString text = QString::fromUtf8(yytext);

      g_defText    += text[0];
      g_defLitText += text;
      BEGIN(DefineText);
   }

<SkipSingleQuote,SkipDoubleQuote>\\.   {
     QString text = QString::fromUtf8(yytext);

      g_defText    += text;
      g_defLitText += text;
   }

<SkipSingleQuote>\'        {
      QString text = QString::fromUtf8(yytext);

      g_defText    += text[0];
      g_defLitText += text;
      BEGIN(DefineText);
   }

<SkipDoubleQuote>.         {
      QString text = QString::fromUtf8(yytext);

      g_defText    += text[0];
      g_defLitText += text;
   }

<SkipSingleQuote>.         {
      QString text = QString::fromUtf8(yytext);

      g_defText    += text[0];
      g_defLitText += text;
   }

<DefineText>.           {
      QString text = QString::fromUtf8(yytext);

      g_defText    += text[0];
      g_defLitText += text;
   }

<<EOF>>              {
      DBG_CTX((stderr, "End of include file\n"));

      if (g_includeStack.isEmpty()) {
         DBG_CTX((stderr, "Terminating scanner\n"));
         yyterminate();

      } else {
         QSharedPointer<FileState> fs = g_includeStack.pop();

         YY_BUFFER_STATE oldBuf = YY_CURRENT_BUFFER;
         preYY_switch_to_buffer(fs->bufState );
         preYY_delete_buffer(oldBuf );

         g_yyLineNr    = fs->lineNr;
         g_inputBuf    = fs->oldFileBuf;
         g_inputBufPos = fs->oldFileBufPos;
         setFileName(fs->fileName);

         DBG_CTX((stderr, "######## FileName %s\n", csPrintable(g_yyFileName)));

         // deal with file changes due to
         // #include's within { .. } blocks

         QString lineStr = QString("# %1 \"%2\" 2").arg(g_yyLineNr).arg(QString(g_yyFileName));
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

         g_lastCContext = YY_START;
         g_commentCount = 1;

         if (yyleng == 3) {
            g_lastGuardName.resize(0); // reset guard in case the #define is documented
         }

         BEGIN(SkipCComment);
      }
   }

<*>"//"[/]?             {
      if (YY_START == SkipVerbatim || YY_START == SkipCond) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);
         outputArray(text, text.length());

         g_lastCPPContext = YY_START;

         if (yyleng == 3) {
            g_lastGuardName.resize(0); // reset guard in case the #define is documented
         }

         BEGIN(SkipCPPComment);
      }
   }

<*>\n                {
      outputChar('\n');
      g_yyLineNr++;
   }

<*>[\xC0-\xFF][\x80-\xBF]+    {
      // utf-8 code point
      QString text  = QString::fromUtf8(yytext);
      g_expectGuard = false;

      for (QChar c : text) {
         outputChar(c);
      }
   }

<*>.                {
      // catch all
      QString text  = QString::fromUtf8(yytext);
      g_expectGuard = false;

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
      g_pathList.append(fi.absoluteFilePath());
   }
}

void initPreprocessor()
{
   addSearchDir(".");
   g_expandedDict = QMakeShared<DefineDict>();
}

void removePreProcessor()
{
   g_expandedDict = QSharedPointer<DefineDict>();
   g_pathList.clear();

   DefineManager::deleteInstance();
}

QString preprocessFile(const QString &fileName, const QString &input)
{
   printlex(preYY_flex_debug, true, __FILE__, csPrintable(fileName) );

   g_macroExpansion   = Config::getBool("macro-expansion");
   g_expandOnlyPredef = Config::getBool("expand-only-predefined");

   g_skip        = false;
   g_curlyCount  = 0;
   g_nospaces    = false;
   g_inputBufPos = 0;

   g_inputBuf    = input;
   g_outputBuf   = "";

   g_includeStack.clear();
   g_expandedDict->clear();
   g_condStack.clear();

   uint orgOffset = 0;

   setFileName(fileName);

   g_inputFileDef = g_yyFileDef;
   DefineManager::instance().startContext(g_yyFileName);

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
            QRegExp reId("[a-z_A-Z\x80-\xFF][a-z_A-Z0-9\x80-\xFF]*");
            QHash<QString, int> argDict;

            int index = posOpen + 1;
            int pos;
            int len;
            int count = 0;

            // gather the formal arguments in a dictionary
            while (index < posClose && (pos = reId.indexIn(definedMacro, index))) {
               len = reId.matchedLength();

               if (len > 0) {
                  argDict.insert(definedMacro.mid(pos, len), count++);
                  index = pos + len;

               } else {
                  index++;
               }
            }

            // strip definition part
            QString tmp = definedMacro.right(definedMacro.length() - posEquals - 1);
            QString definition;

            index = 0;

            // substitute all occurrences of formal arguments with their corresponding markers
            while ((pos = reId.indexIn(tmp, index)) != -1) {
               len = reId.matchedLength();

               if (pos > index) {
                  definition += tmp.mid(index, pos - index);
               }

               auto iter = argDict.find(tmp.mid(pos, len));

               if (iter != argDict.end()) {
                  int argIndex = iter.value();

                  QString marker = QString(" @%1 ").arg(argIndex);
                  definition += marker;

               } else {
                  definition += tmp.mid(pos, len);
               }

               index = pos + len;
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
               def->fileDef      = g_yyFileDef;
               def->m_fileName   = fileName;

               DefineManager::instance().addDefine(g_yyFileName, def);
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
               def->fileDef      = g_yyFileDef;
               def->m_fileName   = fileName;
               DefineManager::instance().addDefine(g_yyFileName, def);
            }
         }
      }
   }

   g_yyLineNr = 1;
   g_yyColNr  = 1;
   g_ifcount  = 0;

   g_levelGuard.clear();

   BEGIN( Start );

   g_expectGuard = determineSection(fileName) == Entry::HEADER_SEC;
   g_guardName.resize(0);
   g_lastGuardName.resize(0);
   g_guardExpr = "";

   preYYlex();

   g_lexInit = true;

   while (! g_condStack.isEmpty()) {
      QSharedPointer<CondCtx> ctx = g_condStack.pop();
      QString sectionInfo = " ";

      if (ctx->sectionId != " ") {
         sectionInfo = QString(" with label %1 ").arg(QString(ctx->sectionId));
      }

      warn(fileName, ctx->lineNr, "Conditional section %s does not have "
           "a corresponding \\endcond command within this file.", csPrintable(sectionInfo));
   }

   // make sure we do not extend a \cond with missing \endcond over multiple files
   forceEndCondSection();

   DefineManager::instance().endContext();
   printlex(preYY_flex_debug, false, __FILE__, csPrintable(fileName));

   return g_outputBuf;
}

void preFreeScanner()
{
   if (g_lexInit) {
      preYYlex_destroy();
   }
}
