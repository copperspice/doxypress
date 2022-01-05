/*************************************************************************
 *
 * Copyright (c) 2014-2022 Barbara Geller & Ansel Sermersheim
 * Copyright (c) 1997-2014 Dimitri van Heesch
 * Copyright (c) 2010      Rene Zaumseil

*************************************************************************/

%{

#include <QFile>
#include <QHash>
#include <QRegularExpression>
#include <QStack>
#include <QStringList>
#include <QVector>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include <arguments.h>
#include <config.h>
#include <commentcnv.h>
#include <commentscan.h>
#include <default_args.h>
#include <doxy_globals.h>
#include <entry.h>
#include <language.h>
#include <message.h>
#include <outputlist.h>
#include <parse_tcl.h>
#include <pre.h>
#include <util.h>

#define YY_NO_INPUT 1

#define MAX_INCLUDE_DEPTH 10

// application error
#define tcl_err \
  printf("Error TCL file: %s, line %d ", csPrintable(tcl.file_name), yylineno); \
  yy_push_state(ERROR); \
  yyless(0); \
  printf

// Application warning
#define tcl_warn \
  printf("Warning TCL file: %s, line %d: ", csPrintable(tcl.file_name), yylineno); \
  printf

#define TCL_ERROR       1
#define TCL_OK          0
#define ckalloc         malloc
#define ckfree          free

int TclFindElement(const QString &list, int listLength, int &elementIndex, int &nextIndex, int *sizePtr, int *bracePtr)
{
   QString::const_iterator iter     = list.constBegin();
// QString::const_iterator iter_end = list.constEnd();

   QString::const_iterator elemStart;            /* Points to first byte of first element. */
   QString::const_iterator limit;                /* Points just after list's last byte. */

   int openBraces = 0;                           /* Brace nesting level during parse. */
   int inQuotes   = 0;
   int size       = 0;

   // Skim off leading white space and check for an opening brace or quote.
   // We treat embedded NULLs in the list as bytes belonging to a list element.

   limit = iter + listLength;

   while ((iter != limit) && (iter->isSpace())) {
      ++iter;
   }

   if (iter == limit) {
      // no element found
      elemStart = limit;
      goto done;
   }

   if (*iter == '{') {
      openBraces = 1;
      ++iter;

   } else if (*iter == '"') {
      inQuotes = 1;
      ++iter;

   }

   elemStart = iter;

   if (bracePtr != 0) {
      *bracePtr = openBraces;
   }

   // Find element's end (a space, close brace, or the end of the string
   while (iter != limit) {

      switch (iter->unicode()) {
         /*
          * Open brace: do not treat specially unless the element is in
          * braces. In this case, keep a nesting count.
          */

         case '{':
            if (openBraces != 0) {
               openBraces++;
            }
            break;

         /*
          * Close brace: if element is in braces, keep nesting count and
          * quit when the last close brace is seen.
          */

         case '}':
            if (openBraces > 1) {
               openBraces--;

            } else if (openBraces == 1) {
               size = iter - elemStart;
               ++iter;

               if ((iter >= limit) || iter->isSpace()) {
                  goto done;
               }

               // Garbage after the closing brace; return an error.
               return TCL_ERROR;
            }
            break;

          /*
          * Backslash: skip over everything up to the end of the backslash
          * sequence.
          */

         case '\\':
            ++iter;
            break;

          /*
          * Space: ignore if element is in braces or quotes; otherwise
          * terminate element.
          */

         case ' ':
         case '\f':
         case '\n':
         case '\r':
         case '\t':
         case '\v':
            if ((openBraces == 0) && ! inQuotes) {
               size = iter - elemStart;
               goto done;
            }
            break;

          /*
          * Double-quote: if element is in quotes then terminate it.
          */

         case '"':
            if (inQuotes) {
               size = (iter - elemStart);
               ++iter;

               if ((iter >= limit) || iter->isSpace()) {
                  goto done;
               }

                /*
                * Garbage after the closing quote; return an error.
                */
               return TCL_ERROR;
            }
            break;
      }

      ++iter;
   }

   // End of list: terminate element
   if (iter == limit) {
      if (openBraces != 0) {
         return TCL_ERROR;

      } else if (inQuotes) {
         return TCL_ERROR;

      }

      size = (iter - elemStart);
   }

done:
   while ((iter != limit) && iter->isSpace()) {
      ++iter;
   }

   elementIndex = elemStart - list.constBegin();
   nextIndex    = iter - list.constBegin();

   if (sizePtr != 0) {
      *sizePtr = size;
   }

   return TCL_OK;
}

/* Pointer to string with list structure. */
/* Pointer to location to fill in with the number of elements in the list. */
/* Pointer to place to store pointer to array of pointers to list elements. */

int tcl_split_internal(const QString &str, QStringList &list)
{
   int result;

   int elementIdx;
   int nextIdx;
   int brace;

   int length;
   int elSize;

   QString data = str;
   length = str.length();

   while (! data.isEmpty()) {

      result  = TclFindElement(data, length, elementIdx, nextIdx, &elSize, &brace);
      length -= nextIdx;

      if (result != TCL_OK) {
         return result;
      }

      list.append(data.mid(elementIdx, elSize));

      if (elementIdx == length) {
         break;
      }

      // remove used data
      data = data.mid(nextIdx);
   }

   return TCL_OK;
}

QStringList tcl_split_list(QString &str)
{
   QStringList retval;

   if (str.left(1) == "{" && str.right(1) == "}") {
      str = str.mid(1, str.length() - 2);

   } else if (str.left(1) == "\"" && str.right(1) == "\"") {
      str = str.mid(1, str.length() - 2);
   }

   if (tcl_split_internal(str, retval) != TCL_OK) {
      retval.append(str);
   }

   return retval;
}

// Structure containing information about current scan context
typedef struct {
   QChar tclType;                          // type of scan context: "\"" "{" "[" "?" " "
   int line0;                              // start line of scan context
   int line1;                              // end line of scan context
   YY_BUFFER_STATE buffer_state;           // value of scan context
   QString ns;                             // current namespace

   QSharedPointer<Entry> entry_fn;         // if set contains the current proc/method/constructor/destructor
   QSharedPointer<Entry> entry_cl;         // if set contain the current class
   QSharedPointer<Entry> entry_scan;       // current scan entry

   Protection protection;                  // current protections state
   QStringList after;                      // option/value list (options: NULL comment keyword script)
} tcl_scan;

// Structure containing all internal global variables
static struct
{
   CodeGenerator *code;                    // if set then we are codifying the file

   int code_line;                          // current line of code
   int code_linenumbers;                   // if true create line numbers in code
   QString code_font;                      // used font to codify
   bool config_autobrief;                  // value of configuration option

   QMap<QString, QString> config_subst;    // map of configuration option values

   QString s_inputString;                   // file contents
   int s_inputPosition;                     // position in file

   QString file_name;                      // name of used file
   ParserInterface *this_parser;           // myself

   int command;                            // true if command was found
   int comment;                            // set true if comment was scanned
   int brace_level;                        // bookkeeping of braces
   int bracket_level;                      // bookkeeping of brackets
   int bracket_quote;                      // bookkeeping of quotes (toggles)

   QChar word_is;                          // type of current word: "\"" "{" "[" "?" " "

   int line_comment;                       // line number of comment
   int line_commentline;                   // line number of comment after command
   int line_command;                       // line number of command
   int line_body0;                         // start line of body
   int line_body1;                         // end line of body

   QString string_command;                 // contain current command
   QString string_commentline;             // contain current comment after command
   QString string_commentcodify;           // current comment string used in codifying
   QString string_comment;                 // contain current comment
   QString string_last;                    // contain last read word or part of word
   QString string;

   QSharedPointer<Entry>  entry_main;      // top level entry
   QSharedPointer<Entry>  entry_file;      // entry of current file
   QSharedPointer<Entry>  entry_current;   // currently used entry
   QSharedPointer<Entry>  entry_inside;    // contain entry of current scan context

   QStringList listCommandwords;           // list of command words
   QList<tcl_scan *> listScan;             // scan contexts

   QMap<QString, QSharedPointer<Entry>> ns;   // all read namespace entries
   QMap<QString, QSharedPointer<Entry>> cl;   // all read class entries
   QMap<QString, QSharedPointer<Entry>> fn;   // all read function entries

   QList<QSharedPointer<Entry>> entry;        // list of all created entries, will be deleted after codifying

   Protection protection;                  // current protections state
   QSharedPointer<MemberDef> memberdef;    // contain current MemberDef when codifying
   bool collectXRefs;

} tcl;

// scanner functions
static int yyread(char *buf,int max_size);
static tcl_scan *tcl_scan_start(QChar type, const QString  &content, const QString &ns,
                  QSharedPointer<Entry> entry_cls, QSharedPointer<Entry> entry_fn);

static void tcl_scan_end();
static void tcl_comment(int what, const QString  &text);
static void tcl_word(int what, const QString  &text);
static void tcl_command(int what, const QString  &text);

// helper functions

// Create new entry, return new initialised entry
QSharedPointer<Entry> tcl_entry_new()
{
   QSharedPointer<Entry> myEntry = QMakeShared<Entry>();

   myEntry->section     = Entry::EMPTY_SEC;
   myEntry->m_entryName = "";
   myEntry->protection  = Public;
   myEntry->m_srcLang   = SrcLangExt_Tcl;

   myEntry->setData(EntryKey::File_Name,    tcl.file_name);
   myEntry->setData(EntryKey::Brief_Docs,   "");

   initGroupInfo(myEntry);

   // collect entries
   if (! tcl.code) {
      tcl.entry.insert(0, myEntry);
   }

   return myEntry;
}

// Set protection level
void tcl_protection(QSharedPointer<Entry> entry)
{
   if (entry->protection != Public && entry->protection != Protected && entry->protection != Private) {
      entry->protection = tcl.protection;
   }

   if (entry->protection != Protected && entry->protection != Private) {
      entry->protection = Public;
   }
}

//  Check name, return 'ns' and 'name' of given current 'ns0' and 'name0'
static void tcl_name(const QString &ns0, const QString &name0, QString &ns, QString &name)
{
   QString myNm;
   int myStart;

   if (name0.startsWith("::")) {
      myNm = name0.mid(2);

   } else if (ns0.length() && ns0 != " ") {
      myNm = ns0 + "::" + name0;

   } else {
      myNm = name0;
   }

   myStart = myNm.lastIndexOf("::");

   if (myStart == -1) {
      ns = "";
      name = myNm;

   } else if (myNm.length() - myStart == 2) {
      // ending with :: so get name equal to last component

      ns      = myNm.mid(0, myStart);
      myStart = ns.lastIndexOf("::");
      name    = myNm.mid(myStart + 2);

   } else {
      ns = myNm.mid(0, myStart);
      name = myNm.mid(myStart + 2);
   }
}

// Check name. Strip namespace qualifiers from name0 if inside inlined code segment
// return 'ns' and 'name' of given current 'ns0' and 'name0'
static void tcl_name_SnippetAware(const QString &ns0, const QString &name0, QString &ns, QString &name)
{
   // If we are inside an inlined code snippet then ns0 already contains the complete namespace path.
   // Any namespace qualifiers in name0 are redundant.

   int i = name0.lastIndexOf("::");

   if (i >= 0 && tcl.memberdef) {
      tcl_name(ns0, name0.mid(i + 2), ns, name);
   } else {
      tcl_name(ns0, name0, ns, name);
   }
}

// Check and return namespace entry.
// @return namespace entry
QSharedPointer<Entry> tcl_entry_namespace(const QString ns)
{
   QSharedPointer<Entry> myEntry;

   if (ns.length()) {
      myEntry = tcl.ns.value(ns);

   } else {
      myEntry = tcl.ns.value("::");
   }

   if (myEntry == nullptr) {
      myEntry = tcl_entry_new();

      myEntry->section      = Entry::NAMESPACE_SEC;
      myEntry->m_entryName  = ns;

      tcl.entry_main->addSubEntry(myEntry);
      tcl.ns.insert(ns, myEntry);
   }

   return myEntry;
}

// Check and return class entry.
// @return class entry
QSharedPointer<Entry> tcl_entry_class(const QString cl)
{
   QSharedPointer<Entry> myEntry;

   if (! cl.length()) {
      return QSharedPointer<Entry>();
   }

   myEntry = tcl.cl.value(cl);

   if (myEntry == nullptr) {
      myEntry = tcl_entry_new();

      myEntry->section     = Entry::CLASS_SEC;
      myEntry->m_entryName = cl;

      tcl.entry_main->addSubEntry(myEntry);
      tcl.cl.insert(cl, myEntry);
   }

   return myEntry;
}

// Check for keywords, return 1 if keyword and 0 otherwise
static int tcl_keyword(QString str)
{
   static QStringList myList;
   static int myInit = 1;

   if (myInit) {
      // tcl keywords

      myList << "append" << "apply" << "array" << "auto_execok" << "auto_import" << "auto_load" << "auto_mkindex"
             << "auto_qualify" << "auto_reset";

      myList << "binary";
      myList << "catch" << "cd" << "close" << "clock" << "concat";
      myList << "eof" << "eval" << "exec" << "exit" << "expr";
      myList << "fblocked" << "fconfigure" << "file" << "fileevent" << "flush" << "for" << "foreach" << "format";
      myList << "gets" << "global";
      myList << "http";
      myList << "if" << "incr" << "info" << "interp";
      myList << "join";
      myList << "lappend" << "lassign" << "lindex" << "linsert" << "llength" << "load" << "lrange" << "lrepeat"
             << "lreplace" << "lreverse" << "lset";

      myList << "namespace";
      myList << "package" << "parray" << "pid" << "pkg_mkIndex" << "proc" << "puts" << "pwd";
      myList << "registry" << "rename" << "return";
      myList << "scan" << "set" << "split" << "string" << "switch";

      myList << "tclLog" << "tcl_endOfWord" << "tcl_findLibrary" << "tcl_startOfNextWord" << "tcl_startOfPreviousWord"
             << "tcl_wordBreakAfter" << "tcl_wordBreakBefore" << "tell" << "time";

      myList << "unknown" << "upvar";
      myList << "variable" << "vwait";

      // tk keywords
      myList << "bell" << "bind" << "bindtags";
      myList << "clipboard" << "console" << "consoleinterp";
      myList << "destroy";
      myList << "event";
      myList << "focus";
      myList << "grid";
      myList << "lower";
      myList << "option";
      myList << "pack" << "place";
      myList << "raise";
      myList << "send";

      myList << "tkerror" << "tkwait" << "tk_bisque" << "tk_focusNext" << "tk_focusPrev" << "tk_focusFollowsMouse" << "tk_popup"
             << "tk_setPalette" << "tk_textCut" << "tk_TextCopy" << "tk_textPaste" << "chooseColor" << "tk_chooseColor"
             << "tk_chooseDirectory" << "tk_dialog" << "tk_getOpenFile" << "tkDialog" << "tk_getSaveFile" << "tk_messageBox";

      myList << "winfo" << "wm";
      myList << "button" << "canvas" << "checkbutton" << "entry" << "frame" << "image" << "label" << "labelframe"
             << "listbox" << "menu" << "menubutton" << "message" << "panedwindow" << "radiobutton" << "scale" << "scrollbar"
             << "spinbox" << "toplevel";

      myList.sort();
      myInit = 0;
   }

   str = str.trimmed();

   if (str.left(2) == "::") {
      str = str.mid(2);
   }

   if (myList.indexOf(str) != -1) {
      return (1);
   }

   return 0;
}

//  End codifying with special font class.
static void tcl_font_end()
{
   if (! tcl.code) {
      return;
   }

   if (! tcl.code_font.isEmpty()) {
      tcl.code->endFontClass();
      tcl.code_font = "";
   }
}

// Codify 'str' with special font class 's'.
static void tcl_codify(const QString &s, const QString &str)
{
   if (tcl.code == nullptr || str.isEmpty() ) {
      return;
   }

   if (! s.isEmpty() && s != "NULL") {
      tcl_font_end();
      tcl.code->startFontClass(s);
      tcl.code_font = s;
   }

   QString tmp;

   for (auto c : str) {

      if (c == '\n') {
         tcl.code_line++;
         tcl.code->codify(tmp);

         if (! tcl.code_font.isEmpty()) {
            tcl.code->endFontClass();
         }

         tcl.code->endCodeLine();
         tcl.code->startCodeLine(tcl.code_linenumbers);

         if (tcl.code_linenumbers) {
            tcl.code->writeLineNumber(QString(), QString(), QString(), tcl.code_line);
         }

         if (! tcl.code_font.isEmpty()) {
            tcl.code->startFontClass(tcl.code_font);
         }

         tmp = "";

      } else {
         tmp += c;

      }
   }

   if (tmp.endsWith('\x1A')) {
      // remove ^Z
      tmp.chop(1);
   }

   if (! tmp.isEmpty()) {
      tcl.code->codify(tmp);
   }

   tcl_font_end();
}

static void tcl_codify_cmd(const QString &s, int i)
{
   tcl_codify(s, tcl.listCommandwords.at(i));
}

// codify a string token , codifies string according to type.
// Starts a new scan context if needed (*myScan==0 and type == "script").
// returns NULL or the created scan context.

static tcl_scan *tcl_codify_token(tcl_scan *myScan, const QString type, const QString &string)
{
   if (myScan != nullptr) {

      if (! type.isEmpty()) {
         myScan->after << type << string;
      } else {
         myScan->after << "NULL" << string;
      }

   } else {
      if (type == "script") {

         if (tcl.listScan.length() > 0) {
            myScan = tcl.listScan.at(0);
            myScan = tcl_scan_start('?', string, myScan->ns, myScan->entry_cl, myScan->entry_fn);
         }

      } else {
         tcl_codify(type, string);
      }
   }

   return myScan;
}

#undef  YY_INPUT
#define YY_INPUT(buf,result,max_size) result = yyread(buf,max_size);

%}
ws              ([ \t]|\\\n)


%option never-interactive
%option nounistd
%option noyy_top_state
%option noyywrap
%option stack
%option yylineno

%x ERROR
%x TOP
%x COMMAND
%x WORD
%x COMMENT
%x COMMENT_NL
%x COMMENT_CODE
%x COMMENT_VERB
%x COMMENTLINE
%x COMMENTLINE_NL

%%

<ERROR>.    {
      yyterminate();
   }

<<EOF>>                 {
      if (tcl.listScan.length() < 1) {
         // error
         tcl_err("Tcl parser stack empty, parser error in file '%s'.\n", csPrintable(tcl.file_name) );
         yyterminate();

      } else if (tcl.listScan.length() == 1) {
         // exit, should be all done
         yyterminate();

      } else {
         // continue
         tcl_command(-1, "");
         tcl_scan_end();
      }
   }

<TOP>"#"                {
      yyless(0);
      tcl.line_comment = yylineno;
      tcl_comment(0, "");
   }

<TOP>({ws}|[\;\n])+     {
      QString text = QString::fromUtf8(yytext);
      tcl_codify("", text);
   }

<TOP>.                  {
      yyless(0);
      tcl.line_command = yylineno;
      tcl_command(0, "");
   }

<COMMENT>[ \t]*         {
      QString text = QString::fromUtf8(yytext);
      tcl_codify("comment", text);
   }

<COMMENT>"###".*\n      {
      QString text = QString::fromUtf8(yytext);
      tcl_codify("comment", text);
      tcl_comment(2, text.mid(1));
   }

<COMMENT>"##".*\\\n     {
      QString text = QString::fromUtf8(yytext);
      tcl_codify("comment", text);

      text = text.mid(2, text.length() - 3);
      text.append("\n");
      tcl_comment(1, text);

      yy_push_state(COMMENT_NL);
   }

<COMMENT>"##".*\n       {
      QString text = QString::fromUtf8(yytext);
      tcl_codify("comment", text);
      tcl_comment(1, text.mid(2));
   }

<COMMENT>"#"[@\\]"code"\n[ \t]*[^#] {
      QString text = QString::fromUtf8(yytext);

      tcl_codify("comment", text.left(7));
      tcl_comment(2, "\n@code\n");
      yyless(7);
      yy_push_state(COMMENT_CODE);
   }

<COMMENT>"#"[@\\]"verbatim"\n[ \t]*[^#] {
      QString text = QString::fromUtf8(yytext);

      tcl_codify("comment", text.left(11));
      tcl_comment(2, "\n@verbatim\n");
      yyless(11);
      yy_push_state(COMMENT_VERB);
   }

<COMMENT>"#".*\\\n {
      QString text = QString::fromUtf8(yytext);

      tcl_codify("comment", text);
      text = text.mid(1, text.length() - 3);
      text.append("\n");
      tcl_comment(2, text);
      yy_push_state(COMMENT_NL);
   }

<COMMENT>"#".*\n {
      QString text = QString::fromUtf8(yytext);

      tcl_codify("comment", text);
      tcl_comment(2, text.mid(1));
   }

<COMMENT>"#".*\x1A {
      QString text = QString::fromUtf8(yytext);

      text = text.mid(0, text.length() - 1);
      tcl_codify("comment", text);

      text = text.mid(1, text.length());
      tcl_comment(-2, text);
      unput(0x1A);
   }

<COMMENT>\x1A {
      tcl_comment(-2, "");
      unput(0x1A);
   }

<COMMENT>.|\n {
      QString text = QString::fromUtf8(yytext);
      tcl_comment(-2,  text);
      yyless(0);
   }

<COMMENT_CODE>"#"[@\\]"endcode"\n {
      QString text = QString::fromUtf8(yytext);

      text = text.left(text.length() - 10);
      tcl_comment(2, text);
      tcl_comment(2, "\n@endcode\n");
      yy_pop_state();
      yyless(0);
   }

<COMMENT_CODE>.*\n {
      yymore();
   }

<COMMENT_CODE>.*\x1A {
      yy_pop_state();
      yyless(0);
   }

<COMMENT_VERB>"#"[@\\]"endverbatim"\n {
      QString text = QString::fromUtf8(yytext);

      text = text.left(text.length() - 14);
      tcl_comment(2, text);
      tcl_comment(2, "\n@endverbatim\n");
      yy_pop_state();
      yyless(0);
   }

<COMMENT_VERB>.*\n {
      yymore();
   }

<COMMENT_VERB>.*\x1A {
      yy_pop_state();
      yyless(0);
   }

<COMMENT_NL>.*\\\n {
      QString text = QString::fromUtf8(yytext);

      tcl_codify("comment", text);
      tcl_comment(2, text);
   }

<COMMENT_NL>.*\n {
      QString text = QString::fromUtf8(yytext);

      tcl_codify("comment", text);
      tcl_comment(2, text);
      yy_pop_state();
   }

<COMMENT_NL>.*\x1A {
      yy_pop_state();
      yyless(0);
   }

<COMMENTLINE>.*\x1A {
      yy_pop_state();
      yyless(0);
   }

<COMMENTLINE>[ \t]* {
      QString text = QString::fromUtf8(yytext);
      tcl.string_commentcodify += text;
   }

<COMMENTLINE>"#<".*\\\n {
      QString text = QString::fromUtf8(yytext);
      tcl.string_commentcodify += text;

      text = text.mid(2, text.length() - 4);
      text.append("\n");
      tcl.string_commentline += text;

      yy_push_state(COMMENTLINE_NL);
   }

<COMMENTLINE>"#<".*\n {
      QString text = QString::fromUtf8(yytext);

      tcl.string_commentcodify += text;
      tcl.string_commentline   += text.mid(2);
   }

<COMMENTLINE>.|\n {
      QString text = QString::fromUtf8(yytext);

      yy_pop_state();

      if (tcl.string_commentline.length()) {
         tcl.entry_current->setData(EntryKey::Brief_Docs, tcl.string_commentline);
         tcl.entry_current->setData(EntryKey::Brief_File, tcl.file_name);
         tcl.entry_current->briefLine = tcl.line_commentline;
      }

      yyless(0);
      tcl_command(-1, tcl.string_commentcodify);
      tcl.string_commentline   = "";
      tcl.string_commentcodify = "";
   }

<COMMENTLINE_NL>.*\\\n {
      QString text = QString::fromUtf8(yytext);
      tcl.string_commentcodify += text;

      text = text.left(text.length() - 3);
      text.append("\n");
      tcl.string_commentline += text;
   }

<COMMENTLINE_NL>.*\n {
      QString text = QString::fromUtf8(yytext);

      tcl.string_commentcodify += text;
      tcl.string_commentline += text;
      yy_pop_state();
   }

<COMMENTLINE_NL>.*\x1A {
      QString text = QString::fromUtf8(yytext);

      text = text.left(text.length() - 1);
      tcl.string_commentcodify += text;
      tcl.string_commentline   += text;

      yy_pop_state();
      unput(0x1A);
   }

<COMMAND>{ws}*[\;]{ws}*"#<" {
      QString text = QString::fromUtf8(yytext);

      tcl.string_commentcodify = text;
      tcl.string_commentcodify = tcl.string_commentcodify.left(tcl.string_commentcodify.length()-2);
      tcl.string_commentline   = "";
      tcl.line_commentline     = yylineno;
      tcl.line_body1           = yylineno;

      unput('<');
      unput('#');
      yy_push_state(COMMENTLINE);
   }

<COMMAND>{ws}*\x1A {
      QString text = QString::fromUtf8(yytext);

      tcl.string_commentcodify = "";
      tcl.string_commentline   = "";
      tcl.line_body1           = yylineno;

      tcl_command(-1, text);
   }

<COMMAND>{ws}*; {
      QString text = QString::fromUtf8(yytext);

      tcl.string_commentcodify = "";
      tcl.string_commentline   = "";
      tcl.line_body1           = yylineno;

      tcl_command(-1, text);
   }

<COMMAND>{ws}*\n {
      QString text = QString::fromUtf8(yytext);

      tcl.string_commentcodify = "";
      tcl.string_commentline   = "";
      tcl.line_body1           = yylineno - 1;

      tcl_command(-1, text);
   }

<COMMAND>{ws}+ {
      QString text = QString::fromUtf8(yytext);
      tcl_command(1, text);
   }

<COMMAND>"{*}". {
      QString text = QString::fromUtf8(yytext);

      tcl.word_is     = ' ';
      tcl.string_last = "{*}";
      tcl_word(0, text.mid(3));
   }

<COMMAND>"\\"[\{\}\[\]\;\" \t] {
      QString text = QString::fromUtf8(yytext);

      tcl.word_is     = ' ';
      tcl.string_last = "";
      tcl_word(0, text);
   }

<COMMAND>. {
      QString text = QString::fromUtf8(yytext);

      tcl.word_is = ' ';
      if (text[0] == '{' || text[0] == '[' || text[0] == '"') {
         tcl.word_is = text[0];
      }

      tcl.string_last = "";
      tcl_word(0, text);
   }


<WORD>"\\\\" |
<WORD>"\\"[\{\}\[\]\;\" \t] {
      QString text = QString::fromUtf8(yytext);
      tcl_word(1, text);
   }

<WORD>"\\\n" {
      QString text = QString::fromUtf8(yytext);
      tcl_word(2, text);
   }

<WORD>"{" {
      QString text = QString::fromUtf8(yytext);
      tcl_word(3, text);
   }

<WORD>"}" {
      QString text = QString::fromUtf8(yytext);
      tcl_word(4, text);
   }

<WORD>"[" {
      QString text = QString::fromUtf8(yytext);
      tcl_word(5, text);
   }

<WORD>"]" {
      QString text = QString::fromUtf8(yytext);
      tcl_word(6, text);
   }

<WORD>"\"" {
      QString text = QString::fromUtf8(yytext);
      tcl_word(7, text);
   }

<WORD>" " {
      QString text = QString::fromUtf8(yytext);
      tcl_word(8, text);
   }

<WORD>"\t" {
      QString text = QString::fromUtf8(yytext);
      tcl_word(9, text);
   }

<WORD>";" {
      QString text = QString::fromUtf8(yytext);
      tcl_word(10, text);
   }

<WORD>"\n" {
      QString text = QString::fromUtf8(yytext);
      tcl_word(11, text);
   }

<WORD>\x1A {
      QString text = QString::fromUtf8(yytext);
      tcl_word(12, text);
   }

<WORD>. {
      QString text = QString::fromUtf8(yytext);
      tcl_word(1, text);
   }
%%

// Start new scan context for given 'content', return created new scan context.

YY_BUFFER_STATE tcl_scan_string (const QString &yystr)
{
   QByteArray tmp = yystr.toUtf8();
   return yy_scan_bytes(tmp.constData(), tmp.length());
}

static tcl_scan *tcl_scan_start(QChar type, const QString &content_t, const QString &ns,
                  QSharedPointer<Entry> entry_cl, QSharedPointer<Entry> entry_fn)
{
   QString content = content_t;

   tcl_scan *myScan = nullptr;

   if (tcl.listScan.length() > 0) {
      myScan = tcl.listScan.at(0);
   }

   myScan->line1 = yylineno;
   yy_push_state(TOP);

   myScan = new tcl_scan;
   myScan->tclType = ' ';

   switch (type.unicode()) {
      case '"':
      case '{':
      case '[':
         myScan->tclType = type;
         break;

      case '?':
         if (content[0] == '"' && content[content.length() - 1] == '"') {
            myScan->tclType = '"';
         }

         if (content[0] == '{' && content[content.length() - 1] == '}') {
            myScan->tclType = '{';
         }

         if (content[0] == '[' && content[content.length() - 1] == ']') {
            myScan->tclType = '[';
         }
   }

   if (myScan->tclType != ' ') {
      tcl_codify("", myScan->tclType);
      content = content.mid(1, content.length() - 2);
   }

   content += (char)0x1A;         // for detection end of scan context

   myScan->ns           = ns;
   myScan->entry_cl     = entry_cl;
   myScan->entry_fn     = entry_fn;
   myScan->entry_scan   = tcl.entry_current;
   myScan->buffer_state = tcl_scan_string(content);

   myScan->line0 = tcl.line_body0;
   myScan->line1 = tcl.line_body1;

   myScan->after.clear();
   yylineno = myScan->line0;
   myScan->protection = tcl.protection;

   tcl.entry_inside = myScan->entry_scan;
   tcl.entry_current = tcl_entry_new();

   tcl.listScan.insert(0, myScan);

   yy_switch_to_buffer(myScan->buffer_state);

   return (myScan);
}

// Close current scan context.
static void tcl_scan_end()
{
   int len = tcl.listScan.length();

   tcl_scan *myScan  = nullptr;
   tcl_scan *myScan1 = nullptr;

   if (len > 0) {
      myScan  = tcl.listScan.at(0);
   }

   if (len > 1) {
      myScan1 = tcl.listScan.at(1);
   }

   if (myScan->tclType == '{') {
      myScan->tclType = '}';
   }

   if (myScan->tclType == '[') {
      myScan->tclType = ']';
   }

   if (myScan->tclType != ' ') {
      tcl_codify("", myScan->tclType);
   }

   int myStart = -1;
   for (unsigned int i = 0; i < myScan->after.count(); i = i + 2) {
      if (myScan->after[i] == "script") {
         myStart = i;
         break;
      }
      tcl_codify(myScan->after[i], myScan->after[i + 1]);
   }

   yy_delete_buffer(myScan->buffer_state);
   yy_pop_state();
   tcl.entry_inside = myScan1->entry_scan;

   yy_switch_to_buffer(myScan1->buffer_state);
   yylineno       = myScan1->line1;
   tcl.protection = myScan1->protection;

   if (myStart >= 0) {
      myScan1 = tcl_scan_start('?', myScan->after[myStart + 1], myScan->ns, myScan->entry_cl, myScan->entry_fn);

      for (unsigned int i = myStart + 2; i < myScan->after.count(); i++) {
         myScan1->after.append(myScan->after[i]);
      }

      if (tcl.listScan.length() > 1 ) {
         tcl.listScan.removeAt(1);
      }

   } else if (tcl.listScan.length() > 0 ) {
      tcl.listScan.removeFirst();
   }
}

// Handling of word parsing.
static void tcl_word(int what, const QString &text)
{
   static QStack<QChar> myList;       // nesting level list
   static int myWhite = 0;            // set true when next char should be whitespace

   static QChar myWord;               // internal state

   switch (what) {
      case 0:// start
         yy_push_state(WORD);

         switch (text[0].unicode() ) {
            case '{':
            case '[':
            case '"':
               myWord = text[0];
               break;

            default:
               myWord = '.';
         }

         myList.clear();
         myList.push(myWord);

         myWhite = 0;

         break;

      case 1:// all other chars
         if (myWhite != 0) {
            // {x}y "x"y
            tcl_err("expected word separator: %s\n", csPrintable(text));
            return;
         }

         if (myList.isEmpty()) {
            myWord = '.';

            myList.push(myWord);
         }
         break;

      case 2:// \\\n
         if (myList.isEmpty()) {
            myWord = ' ';

            yy_pop_state();
            yyless(0);

            return;
         }

         switch (myList.top().unicode()) {
            case '{':
            case '[':
            case '"':
               break;

            case '.':
               if (myList.size() == 1) {
                  myWord = ' ';

                  yy_pop_state();
                  yyless(0);

                  return;
               }
               break;
         }

         myWhite = 0;
         break;

      case 3:// {
         if (myWhite != 0) {
            // {x}{ "x"{
            tcl_err("expected word separator: %s\n", csPrintable(text));
            return;
         }

         switch (myList.top().unicode()) {
            case '{':
            case '[':
               myList.push('{');
               break;

            case '"':
            case '.':
               break;
         }

         myWhite = 0;
         break;

      case 4:// }
         if (myWhite != 0) {
            // {x}{ "x"{
            tcl_err("expected word separator: %s\n", csPrintable(text));
            return;
         }

         switch (myList.top().unicode()) {
            case '{':// {{x}}

               myList.pop();

               if (myList.isEmpty() && ! tcl.code) {
                  myWhite = 1;
               }

               break;

            case '[':
            case '"':
            case '.':
               break;
         }

         break;
      case 5:// [
         if (myWhite != 0) {
            // {x}[
            tcl_err("expected word separator: %s\n", csPrintable(text));
            return;
         }

         switch (myList.top().unicode()) {
            case '{':
               break;

            case '[':
            case '"':
            case '.':
               myList.push('[');
               break;
         }

         myWhite = 0;
         break;

      case 6:// ]
         if (myWhite != 0) {
            // {x}]
            tcl_err("expected word separator: %s\n", csPrintable(text));
            return;
         }

         switch (myList.top().unicode()) {
            case '{':
               break;

            case '[':
               myList.pop();
               break;

            case '"':
            case '.':
               break;
         }
         myWhite = 0;
         break;

      case 7:// "
         if (myWhite != 0) {
            // {x}"
            tcl_err("expected word separator: %s\n", csPrintable(text));
            return;
         }
         switch (myList.top().unicode()) {
            case '{':
               break;

            case '[':
               myList.push('"');
               break;

            case '"':
               myList.pop();

            case '.':
               break;
         }
         break;

      case 8:// ' '
      case 9:// \t
      case 10:// ;
      case 11:// \n
         if (myList.isEmpty()) {
            myWord = ' ';

            yy_pop_state();
            yyless(0);
            return;
         }

         switch (myList.top().unicode()) {
            case '{':
            case '[':
            case '"':
               break;

            case '.':
               if (myList.size() == 1) {
                  myWord = ' ';

                  yy_pop_state();
                  yyless(0);

                  return;

               } else {
                  myList.pop();

               }
               break;
         }
         myWhite = 0;
         break;

      case 12:// \x1A
         if (myList.isEmpty()) {
            myWord = ' ';

            yy_pop_state();
            yyless(0);

            return;
         }

         if (myList.size() != 1 || myList[0] != '.') {
            tcl_warn("level = %ld expected = %c\n", static_cast<long>(myList.size()), myList.top().unicode());
         }

         myWord = ' ';
         yy_pop_state();
         yyless(0);

         return;

      default:
         tcl_err("wrong state: %d\n", what);
         return;
   }

   tcl.string_last += text;
}

// Handling of comment parsing.
static void tcl_comment(int what, const QString  &text)
{
   if (what == 0) {
      // begin of comment

      if (tcl.comment) {
         tcl_err("comment in comment\n");
         return;
      }

      yy_push_state(COMMENT);

      tcl.string_comment = "";
      tcl.comment = 0;

   } else if (what == 1) {
      // start new comment
      if (tcl.comment) {
         tcl_comment(99, ""); // inbody
      }

      tcl.string_comment = text;
      tcl.comment = 1;

   } else if (what == 2) {
      // add to comment
      if (tcl.comment) {
         tcl.string_comment += text;
      }

   } else if (what == -1 || what == -2) {
      // end of comment without/with command
      if (tcl.comment) {
         tcl.string_last = tcl.string_comment;
         tcl_comment(100 + what, "");

      } else {
         tcl.string_last = "";

      }

      yy_pop_state();
      tcl.string_comment = "";
      tcl.comment = 0;

   } else if (what == 98 || what == 99) {
      // 98=new 99=inbody

      if (tcl.this_parser && tcl.string_comment.length()) {
         int myPos  = 0;
         bool myNew = 0;
         int myLine = tcl.line_comment;

         QString myIn;
         QString myDoc;
         Protection myProt = tcl.protection;

         // resolve ALIASES
         myIn += "/*!";
         myIn += tcl.string_comment;
         myIn += "*/";

         myDoc = convertCppComments(myIn, tcl.file_name);
         myDoc = myDoc.mid(3);
         myDoc.chop(2);

         if (what == 99) {
            // inbody comment file or namespace or class or proc/method
            int myPos0;
            int myLine0;

            QSharedPointer<Entry> myEntry0 = QMakeShared<Entry>(); // used to test parsing

            QSharedPointer<Entry> myEntry;
            QSharedPointer<Entry> myEntry1;

            if (tcl.listScan.length() > 0 ) {
               if (tcl.listScan.at(0)->entry_fn) {
                  myEntry1 = tcl.listScan.at(0)->entry_fn;

               } else if (tcl.listScan.at(0)->entry_cl) {
                  myEntry1 = tcl.listScan.at(0)->entry_cl;

               }
            }

            myPos0  = myPos;
            myLine0 = myLine;

            while (parseCommentBlock(tcl.this_parser, myEntry0, myDoc, tcl.file_name,
                                     myLine, FALSE, tcl.config_autobrief, FALSE, myProt, myPos, myNew)) {

               if (myNew) {
                  // we need a new entry in this case
                  myNew = 0;
                  myEntry = tcl_entry_new();

                  parseCommentBlock(tcl.this_parser, myEntry, myDoc, tcl.file_name,
                                    myLine0, FALSE, tcl.config_autobrief, FALSE, myProt, myPos0, myNew);

                  tcl.entry_inside->addSubEntry(myEntry);

               } else {
                  // we can add to current entry in this case
                  if (! myEntry1 && tcl.listScan.length() > 0) {
                     myEntry1 = tcl_entry_namespace(tcl.listScan.at(0)->ns);
                  }

                  parseCommentBlock(tcl.this_parser, myEntry1, myDoc, tcl.file_name,
                                    myLine0, FALSE, tcl.config_autobrief, FALSE, myProt, myPos0, myNew);
               }

               myPos0  = myPos;
               myLine0 = myLine;
            }

            if (myNew) {
               // we need a new entry
               myNew   = 0;
               myEntry = tcl_entry_new();
               parseCommentBlock(tcl.this_parser, myEntry, myDoc, tcl.file_name,
                                 myLine0, FALSE, tcl.config_autobrief, FALSE, myProt, myPos0, myNew);

               tcl.entry_inside->addSubEntry(myEntry);

            } else {
               // we can add to current entry
               if (! myEntry1 && tcl.listScan.length() > 0) {
                  myEntry1 = tcl_entry_namespace(tcl.listScan.at(0)->ns);
               }

               parseCommentBlock(tcl.this_parser, myEntry1, myDoc, tcl.file_name,
                                 myLine0, FALSE, tcl.config_autobrief, FALSE, myProt, myPos0, myNew);
            }
         } else {
            // new entry
            tcl.entry_current = tcl_entry_new();

            while (parseCommentBlock(tcl.this_parser, tcl.entry_current, myDoc,
                                     tcl.file_name, myLine, false, tcl.config_autobrief, FALSE,
                                     myProt, myPos, myNew)) {
               if (myNew) {
                  tcl.entry_inside->addSubEntry(tcl.entry_current);
                  tcl.entry_current = tcl_entry_new();

               } else {
                  tcl.entry_current->section     = tcl.entry_inside->section;
                  tcl.entry_current->m_entryName = tcl.entry_inside->m_entryName;
               }
            }

            if (myNew) {
               tcl.entry_inside->addSubEntry(tcl.entry_current);
               tcl.entry_current = tcl_entry_new();
            } else {
               tcl.entry_current->section = tcl.entry_inside->section;
               tcl.entry_current->m_entryName = tcl.entry_inside->m_entryName;
            }
         }

         if (tcl.protection != myProt && tcl.listScan.length() > 0) {
            tcl.listScan.at(0)->protection = (tcl.protection = myProt);
         }
      }

   } else {
      tcl_err("what %d\n", what);
      return;
   }
}

// Parse given arglist .
static void tcl_command_ArgList(QString &arglist)
{
   QString retval = "";

   QStringList myArgs;
   myArgs = tcl_split_list(arglist);

   for (uint i = 0; i < myArgs.count(); i++) {
      Argument newArg;

      QStringList tmpList;
      tmpList = tcl_split_list(myArgs[i]);

      if (tmpList.count() == 2) {
         newArg.name   = tmpList.at(0);
         newArg.defval = tmpList.at(1);

         if (newArg.defval.isEmpty()) {
            newArg.defval = " ";
         }

         retval += "?" + newArg.name + "? ";

      } else {
         newArg.name = (myArgs.at(i));
         retval += QString(newArg.name) + " ";
      }

      tcl.entry_current->argList.append(newArg);
   }

   arglist = retval;
   tcl.entry_current->setData(EntryKey::Member_Args, arglist);
}

// Create link.
static void tcl_codify_link(const QString &name)
{
   if (tcl.code == nullptr || name.isEmpty()) {
      return;
   }

   static int init = 0;
   static QMap<QString, QSharedPointer<MemberDef>> fn;

   if (init == 0) {
      init = 1;

      for (auto mn : Doxy_Globals::memberNameSDict) {
         for (auto md : *mn) {
            fn.insert(md->qualifiedName(), md);
         }
      }

      for (auto mn : Doxy_Globals::functionNameSDict) {
         for (auto md : *mn) {
            fn.insert(md->qualifiedName(), md);
         }
      }
   }

   QSharedPointer<MemberDef> myDef;
   QString myName = name;

   if (name.mid(0, 2) == "::") {
      // fully qualified global command
      myName = myName.mid(2);
      myDef  = fn.value(myName);

   } else {
      // not qualified name
      QString myName1 = myName;
      myDef = QSharedPointer<MemberDef>();

      if (tcl.listScan.length() > 0 ) {
         myName1 = tcl.listScan.at(0)->ns;
      }

      if (myName1 == " " || myName1 == "") {
         myName1 = myName;

      } else {
         myName1 = myName1 + "::" + myName;

      }

      myDef = fn.value(myName1); // search namespace command

      if (! myDef) {
         myDef = fn.value(myName); // search global command
      }
   }

   if (myDef != nullptr) {
      // documented command

      tcl.code->writeCodeLink(myDef->getReference(), myDef->getOutputFileBase(), myDef->anchor(),
                              name, myDef->qualifiedName());

      if (tcl.memberdef) {
         myDef->addSourceReferencedBy(tcl.memberdef);
         tcl.memberdef->addSourceReferences(myDef);         // consider removing

      } else {
         QSharedPointer<Entry> callerEntry;
         unsigned int i;

         // walk the list of scan contexts and find the enclosing method or proc
         for (i = 0; i < tcl.listScan.length(); i++) {
            callerEntry = tcl.listScan.at(i)->entry_scan;

            if (callerEntry->mtype == MethodType::Method && ! callerEntry->m_entryName.isEmpty()) {
               break;
            }
         }

         if (i < tcl.listScan.length()) {
            // enclosing method found
            QString callerName = callerEntry->m_entryName;

            if (callerName.mid(0, 2) == "::") { // fully qualified global command
               callerName = callerName.mid(2);

            } else {
               if (! (tcl.listScan.at(0)->ns.trimmed().isEmpty())) {
                  callerName = tcl.listScan.at(0)->ns + "::" + callerEntry->m_entryName;
               }
            }

            QSharedPointer<MemberDef> callerDef;
            callerDef = fn.value(callerName);

            if (callerDef && myDef && tcl.collectXRefs) {
               addDocCrossReference(callerDef, myDef);
            }
         }
      }

   } else if (tcl_keyword(myName)) {
      // check keyword
      tcl_codify("keyword", name);

   } else {
      tcl_codify("", name);       // something else
   }
}

// scan general argument for brackets
//
// parses (*tcl.list_commandwords.at(i)).utf8() and checks for brackets.
// Starts a new scan context if needed (*myScan==0 and brackets found).
// Returns NULL or the created scan context.

static tcl_scan *tcl_command_ARG(tcl_scan *myScan, unsigned int i, bool ignoreOutermostBraces)
{
   QString myName;

   bool insideQuotes = false;
   unsigned int insideBrackets = 0;
   unsigned int insideBraces   = 0;

   myName = tcl.listCommandwords.at(i);

   if (i % 2 != 0) {
      // handle white space
      myScan = tcl_codify_token(myScan, "NULL", myName);

   } else {
      QString myStr = "";
      unsigned int j;

      for (j = 0; j < myName.length(); j++) {
         QChar c = myName[j];

         bool backslashed = false;

         if (j > 0) {
            backslashed = myName[j - 1] == '\\';
         }

         // this is a state machine
         // input is c
         // internal state is myScan and insideXXX
         // these are the transitions:

         if (c == '[' && ! backslashed && insideBraces == 0) {
            insideBrackets++;
         }

         if (c == ']' && ! backslashed && insideBraces == 0 && insideBrackets > 0) {
            insideBrackets--;
         }

         if (c == '{' && ! backslashed && !insideQuotes && !(ignoreOutermostBraces && j == 0)) {
            insideBraces++;
         }

         if (c == '}' && ! backslashed && !insideQuotes && insideBraces > 0) {
            insideBraces--;
         }

         if (c == '"' && ! backslashed && insideBraces == 0) {
            insideQuotes = ! insideQuotes;
         }

         // all output, depending on state and input
         if (c == '[' && !backslashed && insideBrackets == 1 && insideBraces == 0) {
            // the first opening bracket, output what we have so far
            myStr  += c;

            myScan = tcl_codify_token(myScan, "NULL", myStr);
            myStr  = "";

         } else if (c == ']' && ! backslashed && insideBrackets == 0 && insideBraces == 0) {
            // the last closing bracket, start recursion, switch to deferred
            myScan = tcl_codify_token(myScan, "script", myStr);
            myStr  = "";

            myStr += c;

         } else {
            myStr += c;
         }
      }

      if (i == 0 && myScan == nullptr) {
         tcl_codify_link(myStr);

      } else {
         myScan = tcl_codify_token(myScan, "NULL", myStr);

      }
   }

   return (myScan);
}

// Handle internal tcl commands.
// "eval arg ?arg ...?"
static void tcl_command_EVAL() {
   tcl_codify_cmd("keyword", 0);

   tcl_scan *myScan = nullptr;

   if (tcl.listScan.length() > 0 )  {
      myScan = tcl.listScan.at(0);
   }

   QString myString = "";

   // we simply rescan the line without the eval
   // we include leading whitespace because tcl_scan_start will examine
   // the first char. If it finds a bracket it will assume one expression in brackets.
   // Example: eval [list set] [list NotInvoked] [Invoked NotInvoked]

   for (int i = 1; i < tcl.listCommandwords.length(); i++) {
      myString += tcl.listCommandwords.at(i);
   }

   myScan = tcl_scan_start('?', myString, myScan->ns, myScan->entry_cl, myScan->entry_fn);
}

// Handle internal tcl commands.
// switch ?options? string pattern body ?pattern body ...?
// switch ?options? string {pattern body ?pattern body ...?}
static void tcl_command_SWITCH()
{
   tcl_codify_cmd("keyword", 0);
   tcl_codify_cmd("", 1);
   tcl_scan *myScan = nullptr;
   unsigned int i;

   QString token;

   // first: find the last option token
   unsigned int lastOptionIndex = 0;

   for (i = 2; i < tcl.listCommandwords.count(); i += 2) {
      token = tcl.listCommandwords.at(i);

      if (token == "--") {
         lastOptionIndex = i;
         break;
      }

      if (token[0] == '-' && i - lastOptionIndex == 2) {
         // options start with dash and should form a continuous chain
         lastOptionIndex = i;
      }
   }

   // second: eat up options
   for (i = 2; i <= lastOptionIndex; i++) {
      myScan = tcl_command_ARG(myScan, i, false);
   }

   // third: how many tokens are left?
   if (tcl.listCommandwords.count() - lastOptionIndex == 5) {

      myScan = tcl_command_ARG(myScan, lastOptionIndex + 1, false);
      myScan = tcl_command_ARG(myScan, lastOptionIndex + 2, false);
      myScan = tcl_command_ARG(myScan, lastOptionIndex + 3, false);

      // walk trough the list step by step
      // this way we can preserve whitespace

      bool inBraces = false;
      bool nextIsPattern = true;

      token = tcl.listCommandwords.at(lastOptionIndex + 4);

      if (token[0] == '{') {
         inBraces = true;
         token    = token.mid(1, token.length() - 2);
         myScan   = tcl_codify_token(myScan, "NULL", QString("{"));
      }

      // ToDo: check if multibyte chars are handled correctly
      while (token.length() > 0) {

         QString tempToken = token;

         int elem;
         int next;
         int size;

         TclFindElement(tempToken, token.length(), elem, next, &size, nullptr);

         // handle leading whitespace/opening brace/double quotes
         if (elem > 0) {
            myScan = tcl_codify_token(myScan, "NULL", token.left(elem));
         }

         // handle actual element without braces/double quotes
         if (nextIsPattern) {
            myScan = tcl_codify_token(myScan, "NULL", token.mid(elem, size));

         } else {
            myScan = tcl_codify_token(myScan, "script", token.mid(elem, size));

         }

         // handle trailing whitespace/closing brace/double quotes
         if (next - elem - size > 0) {
            myScan = tcl_codify_token(myScan, "NULL", token.mid(elem + size, next - elem - size));
         }

         nextIsPattern = ! nextIsPattern;
         token = token.mid(next);
      }

      if (inBraces) {
         myScan = tcl_codify_token(myScan, "NULL", QString("}"));
      }

      if (! nextIsPattern) {
         tcl_warn("Invalid switch syntax: last token is not a list of even elements\n");
      }

   } else if ((tcl.listCommandwords.count() - lastOptionIndex > 6) &&
              ((tcl.listCommandwords.count() - lastOptionIndex - 3) % 4 == 0)) {

      myScan = tcl_command_ARG(myScan, lastOptionIndex + 1, false);
      myScan = tcl_command_ARG(myScan, lastOptionIndex + 2, false);

      for (i = lastOptionIndex + 3; i < tcl.listCommandwords.count(); i += 4) {
         myScan = tcl_command_ARG(myScan, i + 0, false); // whitespace
         myScan = tcl_command_ARG(myScan, i + 1, false); // pattern
         myScan = tcl_command_ARG(myScan, i + 2, false); // whitespace

         myScan = tcl_codify_token(myScan, "script", tcl.listCommandwords.at(i + 3)); // script
      }

   } else {
      // not properly detected syntax
      tcl_warn("Invalid switch syntax: %d options followed by %ld tokens.\n",
             lastOptionIndex / 2, static_cast<long>((tcl.listCommandwords.count() - 1) / 2 - lastOptionIndex / 2));

      for (i = lastOptionIndex + 1; i <= tcl.listCommandwords.count(); i++) {
         myScan = tcl_command_ARG(myScan, i, false);
      }
   }
}

// Handle internal tcl commands.
// "catch script ?resultVarName? ?optionsVarName?"
static void tcl_command_CATCH()
{
   tcl_codify_cmd("keyword", 0);
   tcl_codify_cmd("", 1);

   tcl_scan *myScan = nullptr;

   if (tcl.listScan.length() > 0) {
      myScan = tcl.listScan.at(0);
   }

   myScan = tcl_scan_start('?', tcl.listCommandwords.at(2), myScan->ns, myScan->entry_cl, myScan->entry_fn);

   for (unsigned int i = 3; i < tcl.listCommandwords.count(); i++) {
      myScan = tcl_command_ARG(myScan, i, false);
   }
}

// Handle internal tcl commands.
// "if expr1 ?then? body1 elseif expr2 ?then? body2 elseif ... ?else?  ?bodyN?"
static void tcl_command_IF(QStringList type)
{
   tcl_codify_cmd("keyword", 0);
   tcl_codify_cmd("", 1);

   tcl_scan *myScan = nullptr;
   myScan = tcl_command_ARG(myScan, 2, true);

   for (int i = 3; i < tcl.listCommandwords.length(); i++) {
      if (type[i] == "expr") {
         myScan = tcl_command_ARG(myScan, i, true);

      } else {
         if (myScan != nullptr) {
            myScan->after << type[i] << tcl.listCommandwords[i];

         } else if (tcl.listScan.length() > 0) {
            myScan = tcl.listScan.at(0);
            myScan = tcl_scan_start('?', tcl.listCommandwords.at(i), myScan->ns, myScan->entry_cl, myScan->entry_fn);
         }
      }
   }
}
// Handle internal tcl commands.
// "for start test next body"
static void tcl_command_FOR()
{
   tcl_codify_cmd("keyword", 0);
   tcl_codify_cmd("", 1);

   tcl_scan *myScan = nullptr;

   if (tcl.listScan.length() > 0) {
      myScan = tcl.listScan.at(0);
   }

   myScan = tcl_scan_start('?', tcl.listCommandwords.at(2), myScan->ns, myScan->entry_cl, myScan->entry_fn);
   myScan->after << "NULL" << tcl.listCommandwords[3];

   myScan = tcl_command_ARG(myScan, 4, true);
   myScan->after << "NULL" << tcl.listCommandwords[5];
   myScan->after << "script" << tcl.listCommandwords[6];
   myScan->after << "NULL" << tcl.listCommandwords[7];
   myScan->after << "script" << tcl.listCommandwords[8];
}

///  Handle internal tcl commands
// "foreach varname list body" and
// "foreach varlist1 list1 ?varlist2 list2 ...? body"
static void tcl_command_FOREACH()
{
   tcl_scan *myScan = nullptr;
   tcl_codify_cmd("keyword", 0);

   for (int i = 1; i < tcl.listCommandwords.count() - 1; i++) {
      myScan = tcl_command_ARG(myScan, i, false);
   }

   if (myScan != nullptr) {
      myScan->after << "script" << tcl.listCommandwords[tcl.listCommandwords.count() - 1];

   } else if (tcl.listScan.length() > 0) {
      myScan = tcl.listScan.at(0);
      myScan = tcl_scan_start('?', tcl.listCommandwords.at(tcl.listCommandwords.count() - 1),
                  myScan->ns, myScan->entry_cl, myScan->entry_fn);
   }

}

/// Handle internal tcl commands.
// "while test body"
static void tcl_command_WHILE()
{
   tcl_codify_cmd("keyword", 0);
   tcl_codify_cmd("", 1);

   tcl_scan *myScan = nullptr;
   myScan = tcl_command_ARG(myScan, 2, true);
   myScan = tcl_command_ARG(myScan, 3, false);

   if (myScan != nullptr) {
      myScan->after << "script" << tcl.listCommandwords[4];

   } else if (tcl.listScan.length() > 0) {
      myScan = tcl.listScan.at(0);
      myScan = tcl_scan_start('?', tcl.listCommandwords.at(4), myScan->ns, myScan->entry_cl, myScan->entry_fn);
   }
}

// Handle all other commands
// Create links of first command word or first command word inside [].
static void tcl_command_OTHER()
{
   tcl_scan *myScan = nullptr;

   for (int i = 0; i < tcl.listCommandwords.length(); i++) {
      myScan = tcl_command_ARG(myScan, i, false);
   }
}

// Handle proc statements
static void tcl_command_PROC()
{
   QString myNs;
   QString myName;
   QSharedPointer<Entry> myEntryNs;
   QSharedPointer<Entry> myEntry;

   tcl_scan *myScan = nullptr;

   if (tcl.listScan.length() > 0) {
      myScan = tcl.listScan.at(0);
   }

   tcl_codify_cmd("keyword", 0);
   tcl_codify_cmd("", 1);
   tcl_codify_cmd("", 2);
   tcl_codify_cmd("", 3);
   tcl_codify_cmd("", 4);
   tcl_codify_cmd("", 5);
   tcl_name_SnippetAware(myScan->ns, tcl.listCommandwords.at(2), myNs, myName);

   if (myNs.length()) {
      myEntryNs = tcl_entry_namespace(myNs);
   } else {
      myEntryNs = tcl_entry_namespace(myScan->ns);
   }

   // why not needed here? tcl.fn.remove(myName);

   tcl.entry_current->section       = Entry::FUNCTION_SEC;
   tcl.entry_current->mtype         = MethodType::Method;
   tcl.entry_current->m_entryName   = myName;
   tcl.entry_current->startLine     = tcl.line_command;
   tcl.entry_current->startBodyLine = tcl.line_body0;
   tcl.entry_current->endBodyLine   = tcl.line_body1;
   tcl_protection(tcl.entry_current);

   tcl_command_ArgList(tcl.listCommandwords[4]);

   myEntryNs->addSubEntry(tcl.entry_current);

   myEntry = tcl.entry_current;
   tcl.fn.insert(myName, myEntry);
   myScan  = tcl_scan_start(tcl.word_is, tcl.listCommandwords.at(6), myEntryNs->m_entryName, QSharedPointer<Entry>(), myEntry);
}

//! Handle \c itcl::body statements and \c oo::define method and method inside \c itcl::class statements.
static void tcl_command_Method()
{
   QString myNs;
   QString myName;
   QSharedPointer<Entry> myEntryCl;
   QSharedPointer<Entry> myEntry;

   tcl_scan *myScan = nullptr;

   if (tcl.listScan.length() > 0) {
      myScan = tcl.listScan.at(0);
   }

   tcl_codify_cmd("keyword", 0);
   tcl_codify_cmd("", 1);
   tcl_codify_cmd("", 2);
   tcl_codify_cmd("", 3);
   tcl_codify_cmd("", 4);
   tcl_codify_cmd("", 5);
   tcl_name(myScan->ns, tcl.listCommandwords.at(2), myNs, myName);

   if (myNs.length()) {
      myEntryCl = tcl_entry_class(myNs);
   } else {
      myNs = myScan->ns;
      myEntryCl = myScan->entry_cl;
   }

   // needed in case of more then one definition p.e. itcl::method and itcl::body
   // see also bug #
   tcl.fn.remove(myName);
   tcl.entry_current->section       = Entry::FUNCTION_SEC;
   tcl.entry_current->mtype         = MethodType::Method;
   tcl.entry_current->m_entryName   = myName;
   tcl.entry_current->startLine     = tcl.line_command;
   tcl.entry_current->startBodyLine = tcl.line_body0;
   tcl.entry_current->endBodyLine   = tcl.line_body1;
   tcl_protection(tcl.entry_current);
   tcl_command_ArgList(tcl.listCommandwords[4]);

   myEntryCl->addSubEntry(tcl.entry_current);

   tcl.fn.insert(myName, tcl.entry_current);
   myEntry = tcl.entry_current;
   myScan  = tcl_scan_start(tcl.word_is, tcl.listCommandwords.at(6), myNs, myEntryCl, myEntry);
}

// Handle constructor statements inside class definitions.
static void tcl_command_Constructor()
{
   QString myNs;
   QString myName;
   QSharedPointer<Entry> myEntryCl;
   QSharedPointer<Entry> myEntry;

   tcl_scan *myScan = nullptr;

   if (tcl.listScan.length() > 0) {
      myScan = tcl.listScan.at(0);
   }

   tcl_codify_cmd("keyword", 0);
   tcl_codify_cmd("", 1);
   tcl_codify_cmd("", 2);
   tcl_codify_cmd("", 3);
   tcl_name(myScan->ns, tcl.listCommandwords.at(0), myNs, myName);

   if (myNs.length()) {
      myEntryCl = tcl_entry_class(myNs);
   } else {
      myNs = myScan->ns;
      myEntryCl = myScan->entry_cl;
   }

   tcl.entry_current->section       = Entry::FUNCTION_SEC;
   tcl.entry_current->mtype         = MethodType::Method;
   tcl.entry_current->m_entryName   = myName;
   tcl.entry_current->startLine     = tcl.line_command;
   tcl.entry_current->startBodyLine = tcl.line_body0;
   tcl.entry_current->endBodyLine   = tcl.line_body1;
   tcl_protection(tcl.entry_current);

   tcl_command_ArgList(tcl.listCommandwords[2]);

   if (myEntryCl) {
      myEntryCl->addSubEntry(tcl.entry_current);
   }

   myEntry = tcl.entry_current;
   tcl.fn.insert(myName, myEntry);
   myScan = tcl_scan_start(tcl.word_is, tcl.listCommandwords.at(4), myNs, myEntryCl, myEntry);
}

// Handle \c destructor statements inside class definitions.
static void tcl_command_DESTRUCTOR()
{
   QString myNs;
   QString myName;

   QSharedPointer<Entry> myEntryCl;
   QSharedPointer<Entry> myEntry;

   tcl_scan *myScan = nullptr;

   if (tcl.listScan.length() > 0) {
      myScan = tcl.listScan.at(0);
   }

   tcl_codify_cmd("keyword", 0);
   tcl_codify_cmd("", 1);
   tcl_name(myScan->ns, tcl.listCommandwords.at(0), myNs, myName);

  if (myNs.length()) {
      myEntryCl = tcl_entry_class(myNs);
   } else {
      myNs = myScan->ns;
      myEntryCl = myScan->entry_cl;
   }

   tcl.entry_current->section       = Entry::FUNCTION_SEC;
   tcl.entry_current->mtype         = MethodType::Method;
   tcl.entry_current->m_entryName   = myName;
   tcl.entry_current->startLine     = tcl.line_command;
   tcl.entry_current->startBodyLine = tcl.line_body0;
   tcl.entry_current->endBodyLine   = tcl.line_body1;
   tcl_protection(tcl.entry_current);

   myEntryCl->addSubEntry(tcl.entry_current);
   myEntry = tcl.entry_current;
   tcl.fn.insert(myName, myEntry);

   myScan = tcl_scan_start(tcl.word_is, tcl.listCommandwords.at(2), myNs, myEntryCl, myEntry);
}

// Handle  namespace statements
static void tcl_command_Namespace()
{
   QString myNs;
   QString myName;
   QString myStr;

   tcl_scan *myScan = nullptr;

   if (tcl.listScan.length() > 0) {
      myScan = tcl.listScan.at(0);
   }

   tcl_codify_cmd("keyword", 0);
   tcl_codify_cmd("", 1);
   tcl_codify_cmd("keyword", 2);
   tcl_codify_cmd("", 3);
   tcl_codify_cmd("", 4);
   tcl_codify_cmd("", 5);

   tcl_name(myScan->ns, tcl.listCommandwords.at(4), myNs, myName);

   if (myNs.length()) {
      myName = myNs + "::" + myName;
   }

   tcl.entry_current->section       = Entry::NAMESPACE_SEC;
   tcl.entry_current->m_entryName   = myName;
   tcl.entry_current->startLine     = tcl.line_command;
   tcl.entry_current->startBodyLine = tcl.line_body0;
   tcl.entry_current->endBodyLine   = tcl.line_body1;

   tcl.entry_main->addSubEntry(tcl.entry_current);
   tcl.ns.insert(myName, tcl.entry_current);

   myStr = tcl.listCommandwords.at(6);

   if (tcl.listCommandwords.count() > 7) {
      for (uint i = 7; i < tcl.listCommandwords.count(); i++) {
         myStr.append(tcl.listCommandwords.at(i));
      }

      tcl.word_is = ' ';
   }

   myScan = tcl_scan_start(tcl.word_is, myStr, myName, QSharedPointer<Entry>(), QSharedPointer<Entry>());
}

// Handle itcl::class statements.
static void tcl_command_ITCL_CLASS()
{
   QString myNs;
   QString myName;

   QSharedPointer<Entry> myEntryCl;

    tcl_scan *myScan = nullptr;

   if (tcl.listScan.length() > 0) {
      myScan = tcl.listScan.at(0);
   }

   tcl_codify_cmd("keyword", 0);
   tcl_codify_cmd("", 1);
   tcl_codify_cmd("NULL", 2);
   tcl_codify_cmd("NULL", 3);

   tcl_name(myScan->ns, tcl.listCommandwords.at(2), myNs, myName);

   if (myNs.length()) {
      myName = myNs + "::" + myName;
   }

   tcl.entry_current->section       = Entry::CLASS_SEC;
   tcl.entry_current->m_entryName   = myName;
   tcl.entry_current->startLine     = tcl.line_command;
   tcl.entry_current->startBodyLine = tcl.line_body0;
   tcl.entry_current->endBodyLine   = tcl.line_body1;

   tcl.entry_main->addSubEntry(tcl.entry_current);

   tcl.cl.insert(myName, tcl.entry_current);

   myEntryCl = tcl.entry_current;
   myScan = tcl_scan_start(tcl.word_is, tcl.listCommandwords.at(4), myName, myEntryCl, QSharedPointer<Entry>());
}

// Handle \c oo::class statements
static void tcl_command_OO_CLASS()
{
   QString myNs;
   QString myName;

   QSharedPointer<Entry> myEntryCl;

   tcl_scan *myScan = nullptr;

   if (tcl.listScan.length() > 0) {
      myScan = tcl.listScan.at(0);
   }

   tcl_codify_cmd("keyword", 0);
   tcl_codify_cmd("", 1);
   tcl_codify_cmd("NULL", 2);
   tcl_codify_cmd("NULL", 3);
   tcl_codify_cmd("NULL", 4);
   tcl_codify_cmd("NULL", 5);
   tcl_name(myScan->ns, tcl.listCommandwords.at(4), myNs, myName);

   if (myNs.length()) {
      myName = myNs + "::" + myName;
   }
   tcl.entry_current->section       = Entry::CLASS_SEC;
   tcl.entry_current->m_entryName   = myName;
   tcl.entry_current->startLine     = tcl.line_command;
   tcl.entry_current->startBodyLine = tcl.line_body0;
   tcl.entry_current->endBodyLine   = tcl.line_body1;

   tcl.entry_main->addSubEntry(tcl.entry_current);

   tcl.cl.insert(myName, tcl.entry_current);
   myEntryCl = tcl.entry_current;
   myScan = tcl_scan_start(tcl.word_is, tcl.listCommandwords.at(6), myName, myEntryCl, QSharedPointer<Entry>());
}

// Handle \c oo::define statements.
static void tcl_command_OO_DEFINE()
{
   QString myNs, myName, myStr;
   QSharedPointer<Entry> myEntryCl;

   tcl_scan *myScan = nullptr;

   if (tcl.listScan.length() > 0) {
      myScan = tcl.listScan.at(0);
   }

   tcl_codify_cmd("keyword", 0);
   tcl_codify_cmd("", 1);
   tcl_codify_cmd("NULL", 2);
   tcl_codify_cmd("NULL", 3);

   tcl_name(myScan->ns, tcl.listCommandwords.at(2), myNs, myName);
   if (myNs.length()) {
      myName = myNs + "::" + myName;
   }

   myEntryCl = tcl_entry_class(myName);
   myStr = tcl.listCommandwords.at(4);

   // special cases first
   // oo::define classname method methodname args script
   // oo::define classname constructor argList bodyScript
   // oo::define classname destructor bodyScript
   unsigned int n = tcl.listCommandwords.count();

   if ((myStr == "method"      && n == 11) || (myStr == "constructor" && n == 9) || (myStr == "destructor"  && n == 7)) {

      for (unsigned int i = 4; i < n - 1; i++) {
         tcl_codify_cmd("NULL", i);
      }

      QSharedPointer<Entry> myEntry;
      QString myMethod;

      tcl_name(myScan->ns, tcl.listCommandwords.at(n == 11 ? 6 : 4), myNs, myMethod);

      // code snippet taken from tcl_command_Method()/tcl_command_Constructor
      tcl.fn.remove(myMethod);

      tcl.entry_current->section       = Entry::FUNCTION_SEC;
      tcl.entry_current->mtype         = MethodType::Method;
      tcl.entry_current->m_entryName   = myMethod;
      tcl.entry_current->startLine     = tcl.line_command;
      tcl.entry_current->startBodyLine = tcl.line_body0;
      tcl.entry_current->endBodyLine   = tcl.line_body1;
      tcl_protection(tcl.entry_current);

      if (n == 11) {
         tcl_command_ArgList(tcl.listCommandwords[8]);

      } else if (n == 9) {
         tcl_command_ArgList(tcl.listCommandwords[6]);
      }

      if (myEntryCl) {
         myEntryCl->addSubEntry(tcl.entry_current);
      }

      tcl.fn.insert(myMethod, tcl.entry_current);
      myEntry = tcl.entry_current;
      myScan  = tcl_scan_start('?', tcl.listCommandwords.at(n - 1), myNs, myEntryCl, myEntry);

   } else {
      // the general case, simply concat all arguments into a script.
      // Note: all documentation collected just before the
      // oo::define command is lost

      if (tcl.listCommandwords.count() > 5) {

         for (uint i = 5; i < tcl.listCommandwords.count(); i++) {
            myStr.append(tcl.listCommandwords.at(i));
         }

         tcl.word_is = ' ';
      }

      myScan = tcl_scan_start(tcl.word_is, myStr, myName, myEntryCl, QSharedPointer<Entry>());
   }
}

// Handle \c variable statements.
static void tcl_command_Variable(int inclass)
{
   QString myNs;
   QString myName;
   QSharedPointer<Entry> myEntry;

   tcl_scan *myScan = nullptr;

   if (tcl.listScan.length() > 0) {
      myScan = tcl.listScan.at(0);
   }

   tcl_codify_cmd("keyword", 0);
   for (unsigned int i = 1; i < tcl.listCommandwords.length(); i++) {
      tcl_codify_cmd("", i);
   }

   tcl_name(myScan->ns, tcl.listCommandwords.at(2), myNs, myName);

   if (myNs.length()) {
      // qualified variables go into namespace
      myEntry = tcl_entry_namespace(myNs);
      tcl.entry_current->stat = true;

   } else {
      if (inclass) {
         myEntry = myScan->entry_cl;
         tcl.entry_current->stat = false;
      } else {
         myEntry = tcl_entry_namespace(myScan->ns);
         tcl.entry_current->stat = true;
      }
   }
   tcl.entry_current->section       = Entry::VARIABLE_SEC;
   tcl.entry_current->m_entryName   = myName;
   tcl.entry_current->startLine     = tcl.line_command;
   tcl.entry_current->startBodyLine = tcl.line_body0;
   tcl.entry_current->endBodyLine   = tcl.line_body1;
   tcl_protection(tcl.entry_current);

   myEntry->addSubEntry(tcl.entry_current);

   tcl.entry_current = tcl_entry_new();
}

// Handling of command parsing.
// what=0  -> ...
// what=1  -> ...
// what=-1 -> ...
static void tcl_command(int what, const QString &text)
{
   int myLine = 0;

   if (what == 0) {
      tcl.listScan.at(0)->line1 = yylineno;    // current line in scan context
      tcl.line_body0 = yylineno;               // start line of command

      yy_push_state(COMMAND);

      tcl.listCommandwords.clear();
      tcl.string_command = "";
      tcl.string_last = "";
      tcl.command = 1;
      return;

   } else if (what == 1) {
      if (tcl.string_last.length()) {
         tcl.listCommandwords.append(tcl.string_last);
         tcl.string_last = "";
      }

      if (! text.isEmpty() ) {
         tcl.listCommandwords.append(text);
      }
      return;

   } else if (what != -1) {
      // should not happen
      tcl_err("what %d\n", what);
      return;
   }

   QString myText = text;

   if (tcl.command == 0) {
      return; //TODO check on inside comment
   }

   if (tcl.string_last != "") {
      // get last word
      tcl.listCommandwords.append(tcl.string_last);
      tcl.string_last = "";
   }
   yy_pop_state();

   // check command
   QString myStr;
   tcl_scan *myScanBackup = nullptr;

   if (tcl.listCommandwords.length() > 0 ) {
      myStr = tcl.listCommandwords.at(0);
   }

   if (tcl.listScan.length() > 0 ) {
      myScanBackup = tcl.listScan.at(0);
   }

   int myLevel = 0;
   Protection myProt = tcl.protection;

   if (tcl.listCommandwords.length() < 3) {
      tcl_command_OTHER();
      goto command_end;
   }

   // remove leading "::" and apply TCL_SUBST
   if (myStr.left(2) == "::") {
      myStr = myStr.mid(2);
   }

   if (tcl.config_subst.contains(myStr)) {
      myStr = tcl.config_subst[myStr];
   }

   if (myStr == "private") {
      tcl.protection = Private;
      myLevel = 1;

   } else if (myStr == "protected") {
      tcl.protection = Protected;
      myLevel = 1;

   } else if (myStr == "public") {
      tcl.protection = Public;
      myLevel = 1;
   }

   if (myLevel) {
      tcl_codify_cmd("keyword", 0);
      tcl_codify_cmd("", 1);

      tcl.listCommandwords.removeOne(tcl.listCommandwords.at(1));
      tcl.listCommandwords.removeOne(tcl.listCommandwords.at(0));

      if (tcl.listScan.length() > 0 && tcl.listCommandwords.length() == 1) {
         tcl_scan *myScan = tcl.listScan.at(0);

         myScan = tcl_scan_start(tcl.word_is, tcl.listCommandwords.at(0), myScan->ns, myScan->entry_cl, myScan->entry_fn);

         myProt = tcl.protection;
         goto command_end;
      }

      myStr  = tcl.listCommandwords.at(0);

      // remove leading "::" and apply TCL_SUBST
      if (myStr.left(2) == "::") {
         myStr = myStr.mid(2);
      }

      if (tcl.config_subst.contains(myStr)) {
         myStr = tcl.config_subst[myStr];
      }
   }

   if (myStr == "proc") {
      if (tcl.listCommandwords.count() == 5) {
         // itcl::proc
         tcl.listCommandwords.append("");
         tcl.listCommandwords.append("");
      }

      if (tcl.listCommandwords.count() != 7) {
         myLine = __LINE__;
         goto command_warn;
      }

      tcl_command_PROC();
      goto command_end;
   }

   if (myStr == "method") {
      if (tcl.listCommandwords.count() == 5) {
         // itcl::method
         tcl.listCommandwords.append("");
         tcl.listCommandwords.append("");
      }
      if (tcl.listCommandwords.count() != 7) {
         myLine = __LINE__;
         goto command_warn;
      }
      tcl_command_Method();
      goto command_end;
   }

   if (myStr == "constructor") {
      if (tcl.listCommandwords.count() != 5) {
         myLine = __LINE__;
         goto command_warn;
      }
      tcl_command_Constructor();
      goto command_end;
   }

   if (myStr == "destructor") {
      if (tcl.listCommandwords.count() != 3) {
         myLine = __LINE__;
         goto command_warn;
      }
      tcl_command_DESTRUCTOR();
      goto command_end;
   }

   if (myStr == "namespace") {
      if (tcl.listCommandwords.at(2) == "eval") {
         if (tcl.listCommandwords.count() < 7) {
            myLine = __LINE__;
            goto command_warn;
         }
         tcl_command_Namespace();
         goto command_end;
      }
      tcl_command_OTHER();
      goto command_end;
   }

   if (myStr == "itcl::class") {
      if (tcl.listCommandwords.count() != 5) {
         myLine = __LINE__;
         goto command_warn;
      }
      tcl_command_ITCL_CLASS();
      goto command_end;
   }

   if (myStr == "itcl::body") {
      if (tcl.listCommandwords.count() != 7) {
         myLine = __LINE__;
         goto command_warn;
      }
      tcl_command_Method();
      goto command_end;
   }

   if (myStr == "oo::class") {
      if (tcl.listCommandwords.at(2) == "create") {
         if (tcl.listCommandwords.count() != 7) {
            myLine = __LINE__;
            goto command_warn;
         }
         tcl_command_OO_CLASS();
         goto command_end;
      }
      tcl_command_OTHER();
      goto command_end;
   }

   if (myStr == "oo::define") {
      if (tcl.listCommandwords.length() < 5) {
         myLine = __LINE__;
         goto command_warn;
      }
      tcl_command_OO_DEFINE();
      goto command_end;
   }

   if (myStr == "variable") {

      if (tcl.listCommandwords.length() < 3) {
         myLine = __LINE__;
         goto command_warn;
      }

      if (tcl.listScan.length() > 0 && tcl.listScan.at(0)->entry_fn == nullptr) {
         // only parsed outside functions
         tcl_command_Variable(tcl.listScan.at(0)->entry_cl && tcl.listScan.at(0)->entry_cl->m_entryName != "");
         goto command_end;
      }
   }

   if (myStr == "common") {
      if (tcl.listCommandwords.count() < 3) {
         myLine = __LINE__;
         goto command_warn;
      }
      if (tcl.listScan.at(0)->entry_fn == nullptr) {
         // only parsed outside functions
         tcl_command_Variable(0);
         goto command_end;
      }
   }

   if (myStr == "inherit" || myStr == "superclass") {
      if (tcl.listCommandwords.count() < 3) {
         myLine = __LINE__;
         goto command_warn;
      }

      if (tcl.listScan.at(0)->entry_cl && tcl.listScan.at(0)->entry_cl->m_entryName != "") {
         for (unsigned int i = 2; i < tcl.listCommandwords.count(); i = i + 2) {
            tcl.listScan.at(0)->entry_cl->extends.append(BaseInfo(tcl.listCommandwords.at(i), Public, Normal));
         }
      }
      goto command_end;
   }

   /*
    * Start of internal tcl keywords
    * Ready: switch, eval, catch, if, for, foreach, while
    */
   if (myStr == "switch") {
      if (tcl.listCommandwords.length() < 5) {
         myLine = __LINE__;
         goto command_warn;
      }
      tcl_command_SWITCH();
      goto command_end;
   }

   if (myStr == "eval") {
      if (tcl.listCommandwords.length() < 3) {
         myLine = __LINE__;
         goto command_warn;
      }
      tcl_command_EVAL();
      goto command_end;
   }
   if (myStr == "catch") {
      if (tcl.listCommandwords.length() < 3) {
         myLine = __LINE__;
         goto command_warn;
      }
      tcl_command_CATCH();
      goto command_end;
   }
   if (myStr == "for") {
      if (tcl.listCommandwords.length() != 9) {
         myLine = __LINE__;
         goto command_warn;
      }
      tcl_command_FOR();
      goto command_end;
   }
   if (myStr == "foreach") {
      if (tcl.listCommandwords.length() < 7 || tcl.listCommandwords.length() % 2 == 0) {
         myLine = __LINE__;
         goto command_warn;
      }
      tcl_command_FOREACH();
      goto command_end;
   }

   /*
   if expr1 ?then? body1 elseif expr2 ?then? body2 elseif ... ?else?  ?bodyN?
   */

   if (myStr == "if" && tcl.listCommandwords.length() > 4) {
      QStringList myType;

      myType << "keyword" << "NULL" << "expr" << "NULL";
      char myState = 'x'; // last word: e'x'pr 't'hen 'b'ody 'e'lse else'i'f..

      for (unsigned int i = 4; i < tcl.listCommandwords.count(); i = i + 2) {
         QString myStr = tcl.listCommandwords.at(i);

         if (myState == 'x') {
            if (myStr == "then") {
               myState = 't';
               myType << "keyword" << "NULL";
            } else {
               myState = 'b';
               myType << "script" << "NULL";
            }

         } else if (myState == 't') {
            myState = 'b';
            myType << "script" << "NULL";

         } else if (myState == 'b') {
            if (myStr == "elseif") {
               myState = 'i';
               myType << "keyword" << "NULL";
            } else if (myStr == "else" && i == tcl.listCommandwords.count() - 3) {
               myState = 'b';
               myType << "keyword" << "NULL" << "script";
               i = tcl.listCommandwords.count();
            } else if (i == tcl.listCommandwords.count() - 1) {
               myState = 'b';
               myType << "script";
               i = tcl.listCommandwords.count();
            } else {
               myLine = __LINE__;
               goto command_warn;
            }

         } else if (myState == 'i') {
            myState = 'x';
            myType << "expr" << "NULL";
         }
      }

      if (myState != 'b') {
         myLine = __LINE__;
         goto command_warn;
      }
      tcl_command_IF(myType);
      goto command_end;
   }

   if (myStr == "while") {
      if (tcl.listCommandwords.count() != 5) {
         myLine = __LINE__;
         goto command_warn;
      }
      tcl_command_WHILE();
      goto command_end;
   }

   tcl_command_OTHER();
   goto command_end;

command_warn:
   // print warning message because of wrong syntax
   tcl_warn("Line %d, Count = %ld: %s\n", myLine, static_cast<long>(tcl.listCommandwords.count()),
                  csPrintable(tcl.listCommandwords.join(" ")) );

   tcl_command_OTHER();

command_end:
   // add remaining text to current context
   if (! myText.isEmpty()) {

      if (tcl.listScan.length() > 0) {

         if (myScanBackup == tcl.listScan.at(0)) {
            tcl_codify("comment", myText);

         } else {
            tcl.listScan.at(0)->after << "comment" << myText;
         }

      }
   }

   tcl.listCommandwords.clear();
   tcl.command    = 0;
   tcl.protection = myProt;
}

//  Common initializations
static void tcl_init()
{
   // Get values from option TCL_SUBST
   static const QStringList myStrList = Config::getList("tcl-subst");

   tcl.config_subst.clear();

   for (auto s : myStrList) {

      QString myStr = s;
      int i = myStr.indexOf('=');

      if (i > 0) {
         QString myName  = myStr.left(i).trimmed();
         QString myValue = myStr.right(myStr.length() - i - 1).trimmed();

         tcl.config_subst[myName] = myValue;
      }
   }

   QChar tmp = tcl.s_inputString.at(tcl.s_inputString.length() - 1);

   if (tmp == 0x1A) {
      // no code

   } else if (tmp == '\n') {
      tcl.s_inputString.chop(1);
      tcl.s_inputString.append(0x1A);

   } else {
      tcl.s_inputString.append(0x1A);

   }

   tcl.code             = nullptr;
   tcl.code_font        = "";
   tcl.code_line        = 1;
   tcl.code_linenumbers = 1;

   tcl.config_autobrief = Config::getBool("javadoc-auto-brief");

   tcl.s_inputPosition   = 0;
   tcl.file_name        = "";
   tcl.this_parser      = nullptr;
   tcl.command          = 0;
   tcl.comment          = 0;

   tcl.brace_level      = 0;
   tcl.bracket_level    = 0;
   tcl.bracket_quote    = 0;
   tcl.word_is          = ' ';

   tcl.string_command       = "";
   tcl.string_commentline   = "";
   tcl.string_commentcodify = "";
   tcl.string_comment       = "";
   tcl.string_last          = "";

   tcl.entry_main        = QSharedPointer<Entry>();
   tcl.entry_file        = QSharedPointer<Entry>();
   tcl.entry_current     = QSharedPointer<Entry>();
   tcl.entry_inside      = QSharedPointer<Entry>();

   tcl.listCommandwords.clear();
   tcl.listScan.clear();
   tcl.ns.clear();
   tcl.cl.clear();
   tcl.fn.clear();

   yylineno        = 1;
   tcl.protection  = Public;
   tcl.memberdef   = QSharedPointer<MemberDef>();
}

// Start parsing
static void tcl_parse(const QString ns, const QString cls)
{
   tcl_scan *myScan = nullptr;

   tcl.entry_file              = tcl_entry_new();
   tcl.entry_file->m_entryName = tcl.file_name;
   tcl.entry_file->section     = Entry::SOURCE_SEC;
   tcl.entry_file->protection  = Public;

   tcl.entry_main->addSubEntry(tcl.entry_file);

   QSharedPointer<Entry> myEntry = tcl_entry_new();

   myEntry->m_entryName = "";

   tcl.entry_main->addSubEntry(myEntry);
   tcl.ns.insert("::", myEntry);
   tcl.entry_current = tcl_entry_new();

   yyrestart( yyin );
   BEGIN( TOP );

   yylineno = 1;

   myScan = new tcl_scan;
   myScan->tclType = ' ';

   myScan->after.clear();
   myScan->line0 = yylineno;
   myScan->line1 = yylineno;
   myScan->buffer_state = YY_CURRENT_BUFFER;
   myScan->ns   = ns;

   myScan->entry_cl   = tcl_entry_class(cls);
   myScan->entry_fn   = QSharedPointer<Entry>();

   tcl.entry_inside   = tcl.entry_file;
   myScan->entry_scan = tcl.entry_inside;
   tcl.listScan.insert(0, myScan);

   yylex();

   tcl.listScan.clear();
   tcl.ns.clear();
   tcl.cl.clear();
   tcl.fn.clear();
   tcl.entry.clear();
}

// Parse text file and build up entry tree
void Tcl_Parser::parseInput(const QString &fileName, const QString &input, QSharedPointer<Entry> root,
                  enum ParserMode mode, QStringList &includedFiles, bool useClang)
{
   (void) mode;
   (void) includedFiles;
   (void) useClang;

   QFile  myFile;

   myFile.setFileName(fileName);
   if (! myFile.open(QIODevice::ReadOnly)) {
      return;
   }

   if (input.isEmpty()) {
      return;
   }

   tcl.s_inputString = input;

   printlex(yy_flex_debug, true, __FILE__, fileName);

   msg("Parsing %s\n", csPrintable(fileName));
   groupEnterFile(fileName, yylineno);

   tcl_init();
   tcl.code        = nullptr;
   tcl.file_name   = fileName;
   tcl.this_parser = this;
   tcl.entry_main  = root;       /* toplevel entry */

   tcl_parse("", "");

   groupLeaveFile(tcl.file_name, yylineno);
   root->setData(EntryKey::Source_Text, "");
   myFile.close();

   printlex(yy_flex_debug, false, __FILE__, fileName);
}

// Parse file and codify
void Tcl_Parser::parseCode(CodeGenerator &codeOutIntf, const QString &scopeName, const QString &input,
                  SrcLangExt lang, bool isExampleBlock, const QString &exampleName, QSharedPointer<FileDef> fileDef, int startLine,
                  int endLine, bool inlineFragment, QSharedPointer<MemberDef> memberDef, bool showLineNumbers,
                  QSharedPointer<Definition> searchCtx, bool collectXRefs)
{
   (void) lang;
   (void) endLine;
   (void) inlineFragment;
   (void) searchCtx;

   if (input.length() < 1) {
      return;
   }

   printlex(yy_flex_debug, true, __FILE__, fileDef ? fileDef->fileName() : "" );
   tcl.s_inputString = input;

   QString myNs  = "";
   QString myCls = "";

   if (memberDef) {
      if (memberDef->getClassDef()) {
         myCls = memberDef->getClassDef()->displayName();
         myNs = myCls;

      } else if (memberDef->getNamespaceDef()) {
         myNs = memberDef->getNamespaceDef()->displayName();
      }
   }

   QString myStr = "Codifying..";

   if (! scopeName.isEmpty()) {
      myStr += " scope=";
      myStr += scopeName;
   }

   if (! exampleName.isEmpty()) {
      myStr += " example=";
      myStr += exampleName;
   }

   if (memberDef) {
      myStr += " member=";
      myStr += memberDef->memberTypeName();
      myStr += " ";
      myStr += memberDef->qualifiedName();
   }

   if (fileDef) {
      myStr += " file=";
      myStr += fileDef->fileName();
   }

   tcl_init();

   tcl.collectXRefs = collectXRefs;
   tcl.memberdef    = memberDef;
   tcl.code         = &codeOutIntf;

   if (startLine < 0) {
      startLine = 1;
   }

   yylineno             = startLine;
   tcl.code_linenumbers = showLineNumbers;
   tcl.code_line        = yylineno;
   tcl.code->startCodeLine(tcl.code_linenumbers);

   if (tcl.code_linenumbers) {
      tcl.code->writeLineNumber(QString(), QString(), QString(), tcl.code_line);
   }

   tcl.file_name   = "";
   tcl.this_parser = nullptr;

   if (isExampleBlock) {
      tcl_codify("", input);

   } else {
      tcl.entry_main = tcl_entry_new();
      tcl_parse(myNs, myCls);
   }

   tcl.code->endCodeLine();

   tcl.listScan.clear();
   tcl.ns.clear();
   tcl.cl.clear();
   tcl.fn.clear();
   tcl.entry.clear();

   printlex(yy_flex_debug, false, __FILE__, fileDef ? fileDef->fileName() : "");
}

bool Tcl_Parser::needsPreprocessing(const QString &) const
{
   return false;
}

void Tcl_Parser::resetCodeParserState()
{
}

void Tcl_Parser::parsePrototype(const QString &text)
{
   (void)text;
}

static int yyread(char *buf, int max_size)
{
   int len = max_size;

   const char *src = tcl.s_inputString.constData() + tcl.s_inputPosition;

   if (tcl.s_inputPosition + len >= tcl.s_inputString.size_storage()) {
      len = tcl.s_inputString.size_storage() - tcl.s_inputPosition;
   }

   memcpy(buf, src, len);
   tcl.s_inputPosition += len;

   return len;
}

