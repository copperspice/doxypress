/*************************************************************************
 *
 * Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
 * Copyright (c) 1997-2014 Dimitri van Heesch

*************************************************************************/

%{

#include <condparser.h>
#include <config.h>
#include <doxy_globals.h>
#include <message.h>
#include <util.h>

#include <QChar>
#include <QStack>
#include <QTextStream>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define YY_NO_INPUT 1

#define ADDCHAR(c)     s_outputString += c

struct CondCtx
{
   CondCtx(int line, const QString &id, bool b)
      : lineNr(line), sectionId(id), skip(b)
   {}

   int lineNr;
   QString sectionId;
   bool skip;
};

struct CommentCtx
{
   CommentCtx(int line)
      : lineNr(line)
   {}

   int lineNr;
};

static QString  s_inputString;
static QString  s_outputString;
static int      s_inputPosition;

static int      s_col;
static int      s_blockHeadCol;
static bool     s_mlBrief;
static int      s_readLineCtx;
static bool     s_skip;
static QString  s_fileName;
static int      s_lineNr;
static int      s_condCtx;

static int      s_lastCommentContext;
static bool     s_inSpecialComment;
static bool     s_inRoseComment;
static int      s_stringContext;
static int      s_charContext;
static int      s_javaBlock;
static bool     s_specialComment;

static QString  s_aliasString;
static int      s_blockCount;
static bool     s_lastEscaped;
static int      s_lastBlockContext;
static bool     s_pythonDocString;
static int      s_nestingCount;

static SrcLangExt s_lang;
static bool       isFixedForm;    // for Fortran

static QStack<CondCtx>     s_condStack;
static QStack<CommentCtx>  s_commentStack;
static QString             s_blockName;

void replaceComment(int offset);

static void replaceCommentMarker(const QString &s, int len)
{
   QString::const_iterator iter     = s.constBegin();
   QString::const_iterator iter_end = s.constEnd();

   QChar c;

   // copy leading blanks
   while (iter != iter_end) {
      c = *iter;

      if (c == ' ' || c == '\t' || c == '\n') {
         ADDCHAR(c);

         if (c == '\n') {
            ++s_lineNr;
         }

         ++iter;

      } else {
         break;
      }
   }

   // replace start of comment marker by blanks and the last character by a *
   int blanks = 0;

   while (iter != iter_end) {
      c = *iter;

      if (c == '/' || c == '!' || c == '#') {

         ++blanks;
         ++iter;

         if (iter != iter_end && *iter == '<') {
            // comment-after-item marker
            ++blanks;
            ++iter;
         }

         if (c == '!') {
            // end after first !
            break;
         }

      } else {
         break;
      }
   }

   if (blanks > 0) {
      while (blanks > 2) {
         ADDCHAR(' ');
         --blanks;
      }

      if (blanks > 1) {
         ADDCHAR('*');
      }

      ADDCHAR(' ');
   }

   // copy comment line to output
   s_outputString += QStringView(iter,  s.constBegin() + len);
}

static inline int computeIndent(const QString &str)
{
   static const int tabSize = Config::getInt("tab-size");

   int col = 0;

   for (auto c : str) {

      if (c == ' ') {
         ++col;

      } else if (c == '\t') {
         col += tabSize - (col % tabSize);

      } else {
         break;
      }
   }

   return col;
}

static inline void copyToOutput(const QString &s, int len)
{
   static const int tabSize = Config::getInt("tab-size");

   if (s_skip) {
      // only add newlines

      for (int i = 0; i < len; i++) {
         switch (s[i].unicode()) {

            case '\n':
              ADDCHAR('\n');
               ++s_lineNr;
               s_col = 0;
               break;

            case '\t':
               s_col += tabSize -(s_col % tabSize);
               break;

            default:
               ++s_col;
               break;
         }
      }

   } else if (len > 0) {
      s_outputString += s.mid(0, len);

      for (int i = 0; i < len; i++) {
         switch (s[i].unicode()) {
            case '\n':
               s_col = 0;
               ++s_lineNr;
               break;

            case '\t':
               s_col += tabSize - (s_col % tabSize);
               break;

            default:
               ++s_col;
               break;
         }
      }
   }
}

static void startCondSection(const QString &sectId);
static void endCondSection();
static void handleCondSectionId(const QString &expression);

// copies string s with length len to the output
// while replacing any alias commands found in the string
static void replaceAliases(const QString &s)
{
   QString retval = resolveAliasCmd(s);
   copyToOutput(retval, retval.length());
}

#undef  YY_INPUT
#define YY_INPUT(buf,result,max_size) result = yyread(buf, max_size);

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

%option never-interactive
%option nounistd
%option noyywrap

%x Scan
%x SkipString
%x SkipChar
%x SComment
%x CComment
%x CNComment
%x Verbatim
%x VerbatimCode
%x ReadLine
%x CondLine
%x ReadAliasArgs

MAILADR   ("mailto:")?[a-z_A-Z0-9.+-]+"@"[a-z_A-Z0-9-]+("."[a-z_A-Z0-9\-]+)+[a-z_A-Z0-9\-]+

%%

<Scan>[^"'!\/\n\\#,\-=; \t]*   {
      /* consume anything that is not " / , or \n */

      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<Scan>[,= ;\t]    {
      /* consume so we have a nice separator in long initialization lines */

      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<Scan>"\"\"\""!   {
      /* start of python long comment */

      if (s_lang != SrcLangExt_Python) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);

         s_pythonDocString = true;
         s_nestingCount    = 1;
         s_commentStack.clear();

         copyToOutput(text, text.length());

         BEGIN(CComment);
         s_commentStack.push(CommentCtx(s_lineNr));
      }
   }

<Scan>![><!]/.*\n       {
      if (s_lang != SrcLangExt_Fortran) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);
         copyToOutput(text, text.length());

         s_nestingCount = 0;
         s_commentStack.clear();

         BEGIN(CComment);
         s_commentStack.push(CommentCtx(s_lineNr));
      }
   }

<Scan>[Cc\*][><!]/.*\n     {
      if (s_lang != SrcLangExt_Fortran) {
         REJECT;

      } else {
         /* check for fixed format; we might have some conditional as part of multi-line if like C<5 .and. & */

         if (isFixedForm && (s_col == 0)) {
            QString text = QString::fromUtf8(yytext);
            copyToOutput(text, text.length());

            s_nestingCount = 0;
            s_commentStack.clear();

            BEGIN(CComment);
            s_commentStack.push(CommentCtx(s_lineNr));

         } else {
            REJECT;
         }
      }
   }

<Scan>!.*\n          {
      if (s_lang != SrcLangExt_Fortran) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);
         copyToOutput(text, text.length());;
      }
   }

<Scan>[Cc\*].*\n        {
      if (s_lang != SrcLangExt_Fortran) {
         REJECT;

      } else {

         if (s_col == 0) {
            QString text = QString::fromUtf8(yytext);
            copyToOutput(text, text.length());

         } else {
            REJECT;
         }
      }
   }

<Scan>"\""   {
      /* start of a string */

      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      s_stringContext = YY_START;
      BEGIN(SkipString);
   }

<Scan>'              {
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      s_charContext = YY_START;
      BEGIN(SkipChar);
   }

<Scan>\n    {
      // new line
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<Scan>"//!"/.*\n[ \t]*"//"[\/!][^\/] |             /* start C++ style special comment block */
<Scan>("///"[/]*)/[^/].*\n[ \t]*"//"[\/!][^\/] {   /* start C++ style special comment block */
      // */ (editor syntax fix)

      if (s_mlBrief) {
         REJECT;             // bail out if we do not need to convert

      } else {
         int i = 3;

         QString text = QString::fromUtf8(yytext);

         if (text[2] == '/') {
            while (i < text.length() && text[i] == '/') {
               i++;
            }
         }

         s_blockHeadCol = s_col;
         copyToOutput("/**",3);      // */ (editor syntax fix)

         replaceAliases(text.mid(i));
         s_inSpecialComment = true;

         s_readLineCtx = SComment;
         BEGIN(ReadLine);
      }
   }

<Scan>"//##Documentation".*/\n      {
      /* Start of Rational Rose ANSI C++ comment block */
      QString text = QString::fromUtf8(yytext);

      if (s_mlBrief) {
         REJECT;
      }

      int i = QString("//##Documentation").length();
      s_blockHeadCol = s_col;

      copyToOutput("/**",3);      // */ (editor syntax fix)

      replaceAliases(text.mid(i));
      s_inRoseComment = true;
      BEGIN(SComment);
   }

<Scan>"//"[!\/]/.*\n[ \t]*"//"[|\/][ \t]*[@\\]"}" {
      // next line contains an end marker
      QString text = QString::fromUtf8(yytext);

      s_inSpecialComment = text[2] == '/' || text[2] == '!';
      copyToOutput(text, text.length());

      s_readLineCtx = YY_START;
      BEGIN(ReadLine);
   }

<Scan>"//"/.*\n    {
      /* one line C++ comment */
      QString text = QString::fromUtf8(yytext);

      s_inSpecialComment = text[2] == '/' || text[2] == '!';
      copyToOutput(text, text.length());

      s_readLineCtx = YY_START;
      BEGIN(ReadLine);
   }

<Scan>"/**/"      {
      /* avoid matching next rule for empty C comment */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<Scan>"/*"[*!]?   {
      /* start of a C comment */
      if (s_lang == SrcLangExt_Python || s_lang == SrcLangExt_Tcl) {
         REJECT;
      }

      QString text = QString::fromUtf8(yytext);

      s_specialComment = (yyleng == 3);

      s_nestingCount   = 1;
      s_commentStack.clear();

      copyToOutput(text, text.length());

      if (s_specialComment) {
         BEGIN(CComment);
      } else  {
         BEGIN(CNComment);
         s_commentStack.push(CommentCtx(s_lineNr));
      }
   }

<Scan>"#"("#")?   {
      if (s_lang != SrcLangExt_Python) {
         REJECT;

      }  else  {
         QString text = QString::fromUtf8(yytext);
         copyToOutput(text, text.length());

         s_nestingCount = 0;
         s_commentStack.clear();

         BEGIN(CComment);
         s_commentStack.push(CommentCtx(s_lineNr));
      }
   }

<Scan>"--!"         {
      REJECT;
   }

<Scan>![><!]                 {
      if (s_lang != SrcLangExt_Fortran) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);
         copyToOutput(text, text.length());

         s_nestingCount = 0;
         s_commentStack.clear();

         BEGIN(CComment);
         s_commentStack.push(CommentCtx(s_lineNr));
      }
   }

<CComment,CNComment,ReadLine>{MAILADR}  |
<CComment,CNComment,ReadLine>"<"{MAILADR}">" {
      // prevent parsing email address
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<CComment>"{@code"/[ \t\n]       {
      QString text = QString::fromUtf8(yytext);
      copyToOutput("@code", 5);

      s_lastCommentContext = YY_START;
      s_javaBlock = 1;
      s_blockName = text.mid(1);

      BEGIN(VerbatimCode);
   }

<CComment,ReadLine>^[ \t]*("```"[`]*|"~~~"[~]*) {
      /* start of markdown code block */

      if (! Doxy_Globals::markdownSupport) {
         REJECT;
      }

      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      s_lastCommentContext = YY_START;
      s_javaBlock = 0;
      s_blockName = text.trimmed().left(3);

      BEGIN(VerbatimCode);
   }

<CComment,ReadLine>[\\@]("dot"|"code"|"msc"|"startuml")/[^a-z_A-Z0-9] {
      /* start of a verbatim block */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      s_lastCommentContext = YY_START;
      s_javaBlock = 0;

      if (text.mid(1) == "startuml") {
         s_blockName = "uml";
      } else {
         s_blockName = text.mid(1);
      }

      BEGIN(VerbatimCode);
   }

<CComment,ReadLine>[\\@]("f$"|"f["|"f{"|"f(") {
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      s_blockName = text.mid(1);

      if (s_blockName.at(1) == '[') {
         s_blockName.replace(1, 1, ']');

      } else if (s_blockName.at(1) == '{')  {
         s_blockName.replace(1, 1, '}');

      } else if (s_blockName.at(1) == '(') {
         s_blockName.replace(1, 1, ')');

      }

      s_lastCommentContext = YY_START;

      BEGIN(Verbatim);
   }

<CComment,ReadLine>[\\@]("verbatim"|"latexonly"|"htmlonly"|"xmlonly"|"docbookonly"|"rtfonly"|"manonly")/[^a-z_A-Z0-9] {
      /* start of a verbatim block */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      s_blockName = text.mid(1);
      s_lastCommentContext = YY_START;

      BEGIN(Verbatim);
   }

<Scan>.   {
      /* any other character */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<Verbatim>[\\@]("endverbatim"|"endlatexonly"|"endhtmlonly"|"endxmlonly"|"enddocbookonly"|"endrtfonly"|"endmanonly"|"f$"|"f]"|"f}"|"f)") {
      /* end of verbatim block */

      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      if (text.mid(1) == s_blockName) {
         // end of formula
         BEGIN(s_lastCommentContext);

      } else if (text.mid(4) == s_blockName) {
         BEGIN(s_lastCommentContext);
      }
   }

<VerbatimCode>"{"     {
      if (s_javaBlock == 0) {
         REJECT;

      } else {
         ++s_javaBlock;

         QString text = QString::fromUtf8(yytext);
         copyToOutput(text, text.length());
      }
   }

<VerbatimCode>"}"        {
      if (s_javaBlock == 0) {
         REJECT;

      } else {
         --s_javaBlock;

         if (s_javaBlock == 0) {
            copyToOutput(" @endcode ", 10);
            BEGIN(s_lastCommentContext);

         } else {
            QString text = QString::fromUtf8(yytext);
            copyToOutput(text, text.length());
         }
      }
   }

<VerbatimCode>("```"[`]*|"~~~"[~]*) {
      /* end of markdown code block */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      if (text[0] == s_blockName[0]) {
         BEGIN(s_lastCommentContext);
      }
   }

<VerbatimCode>[\\@]("enddot"|"endcode"|"endmsc"|"enduml") {
      /* end of verbatim block */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      if (text.mid(4) == s_blockName) {
         BEGIN(s_lastCommentContext);
      }
   }

<VerbatimCode>^[ \t]*"//"[\!\/]?   {
      /* skip leading comments */
      QString text = QString::fromUtf8(yytext);

      if (! s_inSpecialComment) {
         copyToOutput(text, text.length());

      } else {
         int len = 0;

         while (len < text.length() && (text[len] == ' ' || text[len] == '\t')) {
            ++len;
         }

         copyToOutput(text, len);

         if (text.length() - len == 3) {
            // ends with //! or ///
            copyToOutput(" * ", 3);

         } else {
            // ends with //
            copyToOutput("//", 2);
         }
      }
   }

<Verbatim,VerbatimCode>[^`~@\/\\\n{}]*  {
      /* any character not a backslash or new line or } */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<Verbatim,VerbatimCode>\n     {
      /* new line in verbatim block */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<Verbatim>^[ \t]*"//"[/!]     {
      if (s_blockName == "dot" || s_blockName == "msc" || s_blockName == "uml" || s_blockName.startsWith('f') ) {
         // strip /// from dot images and formulas.

         QString text = QString::fromUtf8(yytext);
         int len = 0;

         while (len < text.length() && (text[len] == ' ' || text[len] == '\t')) {
            ++len;
         }

         copyToOutput(text, len);
         copyToOutput("   ", 3);

      } else  {
         // even slashes are verbatim (e.g. \verbatim, \code)
         REJECT;
      }
   }

<Verbatim,VerbatimCode>.      {
      /* any other character */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<SkipString>\\.               {
      /* escaped character in string */
      QString text = QString::fromUtf8(yytext);
      if (s_lang == SrcLangExt_Fortran) {
         unput(yytext[1]);
         copyToOutput(text, 1);
      } else {
         copyToOutput(text, text.length());
      }
   }

<SkipString>"\""              {
      /* end of string */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      BEGIN(s_stringContext);
   }

<SkipString>.                  {
      /* any other string character */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<SkipString>\n                 {
      /* new line inside string (illegal for some compilers) */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<SkipChar>\\.                  {
      /* escaped character */
      QString text = QString::fromUtf8(yytext);
      if (s_lang == SrcLangExt_Fortran) {
         unput(yytext[1]);
         copyToOutput(text, 1);
      } else {
         copyToOutput(text, text.length());
      }
   }

<SkipChar>'                    {
      /* end of character literal */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      BEGIN(s_charContext);
   }

<SkipChar>.                     {
      /* any other string character */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<SkipChar>\n                    {
      /* new line character */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<CComment,CNComment>[^ `~<\\!@*\n{\"\/]*  {
      /* anything that is not a '*' or command */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<CComment,CNComment>"*"+[^*\/\\@\n{\"]*    {
      /* stars without slashes */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<CComment>"\"\"\""  {
      /* end of Python docstring */

      if (s_lang != SrcLangExt_Python) {
         REJECT;

      } else {
         --s_nestingCount;
         s_pythonDocString = false;

         QString text = QString::fromUtf8(yytext);
         copyToOutput(text, text.length());

         BEGIN(Scan);
      }
   }

<CComment,CNComment>\n {
      /* new line in comment */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      /* in case of Fortran always end of comment */
      if (s_lang == SrcLangExt_Fortran) {
         BEGIN(Scan);
      }
   }

<CComment,CNComment>"/""/"+/"*/"  {
      /* already in C comment, not a start of a nested comment but the end */

      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<CComment,CNComment>"/"+"*"  {
      /* nested C comment */

      if (s_lang == SrcLangExt_Python || s_lang == SrcLangExt_Tcl || s_lang == SrcLangExt_Markdown) {
         REJECT;
      }

      QString text = QString::fromUtf8(yytext);

      ++s_nestingCount;
      s_commentStack.push(CommentCtx(s_lineNr));
      copyToOutput(text, text.length());
   }

<CComment,CNComment>"*"+"/"  {
      /* end of C comment */
      if (s_lang == SrcLangExt_Python || s_lang == SrcLangExt_Tcl || s_lang == SrcLangExt_Markdown) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);

         copyToOutput(text, text.length());
         --s_nestingCount;

         if (s_nestingCount <= 0) {
            BEGIN(Scan);

         } else {
            s_commentStack.pop();
         }
      }
   }

<CComment,CNComment>"\n"/[ \t]*"-"      {
      // end of Python comment

      if (s_lang != SrcLangExt_Python || s_pythonDocString) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);
         copyToOutput(text, text.length());

         BEGIN(Scan);
      }
   }

<CComment,CNComment>"\n"/[ \t]*[^ \t#\-]        {
      QString text = QString::fromUtf8(yytext);

      if (s_lang == SrcLangExt_Python) {

         if (s_pythonDocString) {
            REJECT;

         } else {
            copyToOutput(text, text.length());
            BEGIN(Scan);
         }

      } else {
         REJECT;
      }
   }

<CComment,CNComment>.             {
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<SComment>^[ \t]*"///"[\/]*/\n     {
      replaceComment(0);
   }

<SComment>\n[ \t]*"///"[\/]*/\n    {
      replaceComment(1);
   }

<SComment>^[ \t]*"///"[^\/\n]/.*\n {
      replaceComment(0);
      s_readLineCtx = YY_START;

      BEGIN(ReadLine);
   }

<SComment>\n[ \t]*"//"[\/!]("<")?[ \t]*[\\@]"}".*\n {
      /* end the multiline comment when finding a @} or \} command */

      QString text = QString::fromUtf8(yytext);

      copyToOutput(" */",3);
      copyToOutput(text, text.length());

      s_inSpecialComment = false;
      s_inRoseComment    = false;

      BEGIN(Scan);
   }

<SComment>\n[ \t]*"///"[^\/\n]/.*\n  {
      replaceComment(1);
      s_readLineCtx = YY_START;

      BEGIN(ReadLine);
   }

<SComment>^[ \t]*"//!"             |    // just //!
<SComment>^[ \t]*"//!<"/.*\n       |    // or   //!< something
<SComment>^[ \t]*"//!"[^<]/.*\n    {    // or   //!something
      replaceComment(0);
      s_readLineCtx = YY_START;

      BEGIN(ReadLine);
   }

<SComment>\n[ \t]*"//!"            |
<SComment>\n[ \t]*"//!<"/.*\n      |
<SComment>\n[ \t]*"//!"[^<\n]/.*\n {
      replaceComment(1);
      s_readLineCtx = YY_START;

      BEGIN(ReadLine);
   }

<SComment>^[ \t]*"//##"/.*\n       {
      if (! s_inRoseComment) {
         REJECT;

      } else {
         replaceComment(0);
         s_readLineCtx = YY_START;

         BEGIN(ReadLine);
      }
   }

<SComment>\n[ \t]*"//##"/.*\n      {
      if (! s_inRoseComment) {
         REJECT;

      } else {
         replaceComment(1);
         s_readLineCtx = YY_START;

         BEGIN(ReadLine);
      }
   }

<SComment>\n            {
      /* end of special comment */
      QString text = QString::fromUtf8(yytext);

      copyToOutput(" */",3);
      copyToOutput(text, text.length());

      s_inSpecialComment = false;
      s_inRoseComment    = false;
      s_readLineCtx      = Scan;
      BEGIN(Scan);
   }

<ReadLine>"/**"   {
      copyToOutput("/&zwj;**", 8);
   }

<ReadLine>"*/"   {
      copyToOutput("*&zwj;/", 7);
   }

<ReadLine>"*"    {
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<ReadLine>[^\\@\n\*/]*  {
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<ReadLine>[^\\@\n\*/]*/\n  {
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      BEGIN(s_readLineCtx);
   }

<CComment,CNComment,ReadLine>[\\@][\\@][~a-z_A-Z][a-z_A-Z0-9]*[ \t]* {
      // escaped command
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<CComment,ReadLine>[\\@]"cond"/[^a-z_A-Z0-9]     {
      // conditional section
      s_condCtx = YY_START;
      BEGIN(CondLine);
   }

<CComment,ReadLine>[\\@]"endcond"/[^a-z_A-Z0-9]  {
      // end of conditional section
      bool oldSkip = s_skip;
      endCondSection();

      if (YY_START == CComment && oldSkip && ! s_skip) {

         if (s_lang != SrcLangExt_Python && s_lang != SrcLangExt_Markdown && s_lang != SrcLangExt_Fortran) {
            ADDCHAR('/');
            ADDCHAR('*');

            if (s_specialComment) {
               ADDCHAR('*');
            }
         }
      }
   }

<CondLine>[!()&| \ta-z_A-Z0-9.\-]+ {
      QString text = QString::fromUtf8(yytext);
      handleCondSectionId(text);
   }

<CComment,ReadLine>[\\@]"cond"[ \t\r]*/\n {
      s_condCtx = YY_START;
      handleCondSectionId(" ");
   }

<CondLine>\n	|
<CondLine>. {
      // forgot section id
      QString text = QString::fromUtf8(yytext);
      handleCondSectionId(" ");

      if (text[0] == '\n') {
         ++s_lineNr;
         copyToOutput("\n", 1);
      }
   }

<CComment,ReadLine>[\\@][a-z_A-Z][a-z_A-Z0-9]*  {
      // expand alias without arguments
      QString text = QString::fromUtf8(yytext);
      replaceAliases(text);
   }

<CComment,ReadLine>[\\@][a-z_A-Z][a-z_A-Z0-9]*"{" {
      // expand alias with arguments
      QString text = QString::fromUtf8(yytext);

      s_lastBlockContext = YY_START;
      s_blockCount       = 1;
      s_aliasString      = text;
      s_lastEscaped      = 0;

      BEGIN( ReadAliasArgs );
   }

<ReadAliasArgs>^[ \t]*"//"[/!]/[^\n]+   {
      // skip leading special comments
   }

<ReadAliasArgs>"*/"        {
      // end of comment in the middle of an alias?
      if (s_lang == SrcLangExt_Python) {
         REJECT;

      } else  {
         // abort the alias, restart scanning
         QString text = QString::fromUtf8(yytext);

         copyToOutput(s_aliasString, s_aliasString.length());
         copyToOutput(text, text.length());

         BEGIN(Scan);
      }
   }

<ReadAliasArgs>[^{}\n\\\*]+      {
      s_aliasString += QString::fromUtf8(yytext);
      s_lastEscaped  = false;
   }

<ReadAliasArgs>"\\"        {

      if (s_lastEscaped) {
         s_lastEscaped = false;

      } else{
         s_lastEscaped = true;

      }

      s_aliasString += QString::fromUtf8(yytext);
   }

<ReadAliasArgs>\n          {
      s_aliasString += QString::fromUtf8(yytext);
      s_lineNr++;
      s_lastEscaped = false;
   }

<ReadAliasArgs>"{"         {
      s_aliasString += QString::fromUtf8(yytext);

      if (!s_lastEscaped) {
         s_blockCount++;
      }

      s_lastEscaped = false;
   }

<ReadAliasArgs>"}"         {
      s_aliasString += QString::fromUtf8(yytext);

      if (! s_lastEscaped) {
         --s_blockCount;
      }

      if (s_blockCount == 0) {
         replaceAliases(s_aliasString);
         BEGIN( s_lastBlockContext );
      }

      s_lastEscaped = false;

   }

<ReadAliasArgs>.        {
      s_aliasString += QString::fromUtf8(yytext);
      s_lastEscaped =  false;
   }

<ReadLine>.             {
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<*>.  {
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }


%%

void startCondSection(const QString &sectId)
{
   CondParser prs;
   bool expResult = prs.parse(s_fileName, s_lineNr, sectId);

   s_condStack.push(CondCtx(s_lineNr, sectId, s_skip));

   if (! expResult) {
      // not enabled
      s_skip = true;
   }
}

void endCondSection()
{
   if (s_condStack.isEmpty()) {
      warn(s_fileName, s_lineNr, "Found \\endcond command without matching \\cond");
      s_skip = false;

   } else {
      CondCtx ctx = s_condStack.pop();
      s_skip = ctx.skip;
   }
}

void handleCondSectionId(const QString &expression)
{
  bool oldSkip = s_skip;
  startCondSection(expression);

  if ((s_condCtx == CComment || s_readLineCtx == SComment) && ! oldSkip && s_skip) {
    if (s_lang != SrcLangExt_Python && s_lang != SrcLangExt_Markdown && s_lang != SrcLangExt_Fortran) {
      ADDCHAR('*');
      ADDCHAR('/');
    }
  }

  if (s_readLineCtx == SComment) {
    BEGIN(SComment);

  } else {
    BEGIN(s_condCtx);
  }
}

void replaceComment(int offset)
{
   QString text = QString::fromUtf8(yytext);

   if (s_mlBrief || s_skip) {
      copyToOutput(text, text.length());

   } else {
      int i = computeIndent(text.mid(offset));

      if (i == s_blockHeadCol) {
         replaceCommentMarker(text, text.length());

      } else {
         copyToOutput(" */", 3);

         for (int pos = yyleng - 1; pos >= 0; --pos) {
            char tmp = yytext[pos];
            unput(tmp);
         }

         s_inSpecialComment = false;
         BEGIN(Scan);
      }
   }
}

// simplified way to know if this is fixed form
// duplicate in fortrancode.l
static bool recognizeFixedForm(const QString &contents)
{
   int column    = 0;
   bool skipLine = false;

   for (int i = 0; true; i++) {
      ++column;

      switch (contents[i].unicode()) {
         case '\n':
            column = 0;
            skipLine = false;
            break;

         case ' ':
            break;

         case '\000':
            return false;

         case 'C':
         case 'c':
         case '*':
            if (column == 1) {
               return true;
            }

            if (skipLine) {
               break;
            }

            return false;

         case '!':
            if (column > 1 && column < 7) {
               return false;
            }

            skipLine = true;
            break;

         default:
            if (skipLine) {
               break;
            }

            if (column == 7) {
               return true;
            }

            return false;
      }
   }

   return false;
}


/*! This function does three things:
 *  1 Convert multi-line C++ style comment blocks (which are aligned) to C style
 *    comment blocks (if MULTILINE_CPP_IS_BRIEF is set to NO)
 *  2 Replaces aliases with their definition (see ALIASES)
 *  3 Handles conditional sections (cond...endcond blocks)
 */

// main entry point
QString convertCppComments(const QString &inBuf, const QString &fileName)
{
   s_mlBrief  = Config::getBool("multiline-cpp-brief");

   s_inputString  = inBuf;
   s_outputString = "";

   s_inputPosition = 0;
   s_col      = 0;
   s_skip     = false;
   s_fileName = fileName;
   s_lang     = getLanguageFromFileName(fileName);
   s_lineNr   = 1;

   s_pythonDocString = false;

   s_condStack.clear();
   s_commentStack.clear();

   printlex(commentcnvYY_flex_debug, true, __FILE__, fileName);
   isFixedForm = false;

   if (s_lang == SrcLangExt_Fortran) {
      isFixedForm = recognizeFixedForm(inBuf);
   }

   if (s_lang == SrcLangExt_Markdown) {
      s_nestingCount = 0;
      BEGIN(CComment);
      s_commentStack.push(CommentCtx(s_lineNr));

   } else {
      BEGIN(Scan);
   }

   yylex();

   while (! s_condStack.isEmpty()) {
      CondCtx ctx = s_condStack.pop();
      QString sectionInfo = " ";

      if (ctx.sectionId != " ") {
         sectionInfo = QString(" with label '%1' ").formatArg(ctx.sectionId);
      }

      warn(s_fileName, ctx.lineNr, "Conditional section %s does not have "
           "a corresponding \\endcond command", csPrintable(sectionInfo));
   }

   if (s_nestingCount > 0 && s_lang != SrcLangExt_Markdown && s_lang != SrcLangExt_Fortran) {
      QString tmp;

      if (! s_commentStack.isEmpty()) {
         tmp = ", review line # ";

         bool first = true;

         while (! s_commentStack.isEmpty()) {
            CommentCtx ctx = s_commentStack.pop();

            if (first) {
               first = false;
            } else {
               tmp += ", ";
            }

            tmp += QString::number(ctx.lineNr);
         }
      }

      warn(s_fileName, s_lineNr, "Reached end of file while inside a (nested) comment. "
           "\nNesting level %d %s", s_nestingCount, csPrintable(tmp) );

      // add one for "normal" expected end of comment
   }

   s_commentStack.clear();
   s_nestingCount = 0;

   return s_outputString;
}