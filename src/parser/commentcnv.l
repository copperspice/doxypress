/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim
 * Copyright (C) 1997-2014 by Dimitri van Heesch.

*************************************************************************/

%{

#include <QChar>
#include <QStack>
#include <QRegExp>
#include <QTextStream>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <config.h>
#include <condparser.h>
#include <doxy_globals.h>
#include <message.h>
#include <util.h>

#define YY_NO_INPUT 1
#define YY_NEVER_INTERACTIVE 1

#define ADDCHAR(c)     g_outBuf += c

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

static QString  g_inBuf;
static QString  g_outBuf;

static int      g_inBufPos;
static int      g_col;
static int      g_blockHeadCol;
static bool     g_mlBrief;
static int      g_readLineCtx;
static bool     g_skip;
static QString  g_fileName;
static int      g_lineNr;
static int      g_condCtx;

static QStack<CondCtx *>     g_condStack;
static QStack<CommentCtx *>  g_commentStack;
static QString               g_blockName;

static int      g_lastCommentContext;
static bool     g_inSpecialComment;
static bool     g_inRoseComment;
static int      g_stringContext;
static int      g_charContext;
static int      g_javaBlock;
static bool     g_specialComment;

static QString  g_aliasString;
static int      g_blockCount;
static bool     g_lastEscaped;
static int      g_lastBlockContext;
static bool     g_pythonDocString;
static int      g_nestingCount;

static SrcLangExt g_lang;
static bool       isFixedForm;    // For Fortran

static void replaceCommentMarker(const QString &s, int len)
{
   const QChar *p   = s.constData();
   const QChar *ptr = p;
   QChar c;

   // copy leading blanks
   while ((c = *p) != 0 && (c == ' ' || c == '\t' || c == '\n')) {
      ADDCHAR(c);
      g_lineNr += c == '\n';
      p++;
   }

   // replace start of comment marker by blanks and the last character by a *
   int blanks = 0;

   while ((c = *p) != 0 && (c == '/' || c == '!' || c == '#')) {
      blanks++;
      p++;

      if (*p == '<') { // comment-after-item marker
         blanks++;
         p++;
      }
      if (c == '!') { // end after first !
         break;
      }
   }
   if (blanks > 0) {
      while (blanks > 2) {
         ADDCHAR(' ');
         blanks--;
      }
      if (blanks > 1) {
         ADDCHAR('*');
      }
      ADDCHAR(' ');
   }

   // copy comment line to output
   g_outBuf += QString(p, len - (p - ptr) );
}

static inline int computeIndent(const QString &str)
{
   int col = 0;
   static int tabSize = Config::getInt("tab-size");

   for (auto c : str) {

      if (c == ' ') {
         col++;

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
   if (g_skip) {
      // only add newlines

      for (int i = 0; i < len; i++) {
         if (s[i] == '\n') {
            ADDCHAR('\n');

            g_lineNr++;
         }
      }

   } else if (len > 0) {
      g_outBuf += s.mid(0, len);

      static int tabSize = Config::getInt("tab-size");

      for (int i = 0; i < len; i++) {
         switch (s[i].unicode()) {
            case '\n':
               g_col = 0;
               g_lineNr++;
               break;

            case '\t':
               g_col += tabSize - (g_col % tabSize);
               break;

            default:
               g_col++;
               break;
         }
      }
   }
}

static void startCondSection(const QString &sectId)
{
   CondParser prs;
   bool expResult = prs.parse(g_fileName, g_lineNr, sectId);

   g_condStack.push(new CondCtx(g_lineNr, sectId, g_skip));

   if (! expResult) {
      // not enabled
      g_skip = true;
   }
}

static void endCondSection()
{
   if (g_condStack.isEmpty()) {
      warn(g_fileName, g_lineNr, "Found \\endcond command without matching \\cond");
      g_skip = false;

   } else {
      CondCtx *ctx = g_condStack.pop();
      g_skip = ctx->skip;
   }
}

/** copies string \a s with length \a len to the output, while
 *  replacing any alias commands found in the string.
 */
static void replaceAliases(const QString &s)
{
   QString result = resolveAliasCmd(s);
   copyToOutput(result, result.length());
}

#undef  YY_INPUT
#define YY_INPUT(buf,result,max_size) result = yyread(buf,max_size);

static int yyread(char *buf, int max_size)
{
   int c = 0;

   while (g_inBuf[g_inBufPos] != 0) {

      QString tmp1    = g_inBuf.at(g_inBufPos);
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

      g_inBufPos++;
   }

   return c;
}
void replaceComment(int offset);

%}

%option noyywrap

%x Scan
%x SkipString
%x SkipChar
%x SComment
%x CComment
%x Verbatim
%x VerbatimCode
%x ReadLine
%x CondLine
%x ReadAliasArgs

%%

<Scan>[^"'!\/\n\\#-,]*   {
      /* consume anything that is not " / , or \n */

      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<Scan>[,]    {
      /* consume so we have a nice separator in long initialization lines */

      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<Scan>"\"\"\""!   {
      /* start of python long comment */

      if (g_lang != SrcLangExt_Python) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);

         g_pythonDocString = true;
         g_nestingCount    = 0;
         g_commentStack.clear();

         copyToOutput(text, text.length());

         BEGIN(CComment);
         g_commentStack.push(new CommentCtx(g_lineNr));
      }
   }

<Scan>![><!]/.*\n       {
      if (g_lang != SrcLangExt_Fortran) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);
         copyToOutput(text, text.length());

         g_nestingCount = 0;
         g_commentStack.clear();

         BEGIN(CComment);
         g_commentStack.push(new CommentCtx(g_lineNr));
      }
   }

<Scan>[Cc\*][><!]/.*\n     {
      if (g_lang != SrcLangExt_Fortran) {
         REJECT;

      } else {
         /* check for fixed format; we might have some conditional as part of multilene if like C<5 .and. & */

         if (isFixedForm && (g_col == 0)) {
            QString text = QString::fromUtf8(yytext);
            copyToOutput(text, text.length());

            g_nestingCount = 0;
            g_commentStack.clear();

            BEGIN(CComment);
            g_commentStack.push(new CommentCtx(g_lineNr));

         } else {
            REJECT;
         }
      }
   }

<Scan>!.*\n          {
      if (g_lang != SrcLangExt_Fortran) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);
         copyToOutput(text, text.length());;
      }
   }

<Scan>[Cc\*].*\n        {
      if (g_lang != SrcLangExt_Fortran) {
         REJECT;

      } else {

         if (g_col == 0) {
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

      g_stringContext = YY_START;
      BEGIN(SkipString);
   }

<Scan>'              {
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      g_charContext = YY_START;
      BEGIN(SkipChar);
   }

<Scan>\n    {
      /* new line */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<Scan>"//!"/.*\n[ \t]*"//"[\/!][^\/] |             /* start C++ style special comment block */
<Scan>("///"[/]*)/[^/].*\n[ \t]*"//"[\/!][^\/] {   /* start C++ style special comment block */
      // */ (editor syntax fix)

      if (g_mlBrief)      {
         REJECT;             // bail out if we do not need to convert

      } else {
      int i = 3;

      QString text = QString::fromUtf8(yytext);

      if (text[2] == '/') {
         while (i < text.length() && text[i] == '/') {
            i++;
         }
      }

      g_blockHeadCol = g_col;
      copyToOutput("/**",3);      // */ (editor syntax fix)

      replaceAliases(text.mid(i));
      g_inSpecialComment = true;

      g_readLineCtx = SComment;
      BEGIN(ReadLine);
      }
   }

<Scan>"//##Documentation".*/\n      {
      /* Start of Rational Rose ANSI C++ comment block */
      QString text = QString::fromUtf8(yytext);

      if (g_mlBrief) {
         REJECT;
      }

      int i = QString("//##Documentation").length();
      g_blockHeadCol = g_col;

      copyToOutput("/**",3);      // */ (editor syntax fix)

      replaceAliases(text.mid(i));
      g_inRoseComment = true;
      BEGIN(SComment);
   }

<Scan>"//"[!\/]/.*\n[ \t]*"//"[|\/][ \t]*[@\\]"}" {
      // next line contains an end marker, see bug 752712
      QString text = QString::fromUtf8(yytext);

      g_inSpecialComment = text[2] == '/' || text[2] == '!';
      copyToOutput(text, text.length());

      g_readLineCtx = YY_START;
      BEGIN(ReadLine);
   }

<Scan>"//"/.*\n    {
      /* one line C++ comment */
      QString text = QString::fromUtf8(yytext);

      g_inSpecialComment = text[2] == '/' || text[2] == '!';
      copyToOutput(text, text.length());

      g_readLineCtx = YY_START;
      BEGIN(ReadLine);
   }

<Scan>"/**/"      {
      /* avoid matching next rule for empty C comment, see bug 711723 */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<Scan>"/*"[*!]?   {
      /* start of a C comment */
      QString text = QString::fromUtf8(yytext);

      g_specialComment = (yyleng == 3);
      g_nestingCount   = 0;
      g_commentStack.clear();

      copyToOutput(text, text.length());

      BEGIN(CComment);
      g_commentStack.push(new CommentCtx(g_lineNr));
   }

<Scan>"#"("#")?   {
      if (g_lang != SrcLangExt_Python) {
         REJECT;

      }  else  {
         QString text = QString::fromUtf8(yytext);
         copyToOutput(text, text.length());

         g_nestingCount = 0;
         g_commentStack.clear();

         BEGIN(CComment);
         g_commentStack.push(new CommentCtx(g_lineNr));
      }
   }

<Scan>"--!"         {
      REJECT;
   }

<Scan>![><!]                 {
      if (g_lang != SrcLangExt_Fortran) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);
         copyToOutput(text, text.length());

         g_nestingCount = 0;
         g_commentStack.clear();

         BEGIN(CComment);
         g_commentStack.push(new CommentCtx(g_lineNr));
      }
   }

<CComment>"{@code"/[ \t\n]       {
      QString text = QString::fromUtf8(yytext);
      copyToOutput("@code", 5);

      g_lastCommentContext = YY_START;
      g_javaBlock = 1;
      g_blockName = text.mid(1);

      BEGIN(VerbatimCode);
   }

<CComment,ReadLine>[\\@]("dot"|"code"|"msc"|"startuml")/[^a-z_A-Z0-9] {
      /* start of a verbatim block */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      g_lastCommentContext = YY_START;
      g_javaBlock = 0;

      if (text.mid(1) == "startuml") {
         g_blockName = "uml";
      } else {
         g_blockName = text.mid(1);
      }

      BEGIN(VerbatimCode);
   }

<CComment,ReadLine>[\\@]("f$"|"f["|"f{") {
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      g_blockName = text.mid(1);

      if (g_blockName.at(1) == '[') {
         g_blockName[1] = ']';

      } else if (g_blockName.at(1) == '{')  {
         g_blockName[1] = '}';
      }

      g_lastCommentContext = YY_START;
      BEGIN(Verbatim);
   }

<CComment,ReadLine>[\\@]("verbatim"|"latexonly"|"htmlonly"|"xmlonly"|"docbookonly"|"rtfonly"|"manonly")/[^a-z_A-Z0-9] {
      /* start of a verbatim block */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      g_blockName = text.mid(1);
      g_lastCommentContext = YY_START;

      BEGIN(Verbatim);
   }

<Scan>.   {
      /* any ather character */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<Verbatim>[\\@]("endverbatim"|"endlatexonly"|"endhtmlonly"|"endxmlonly"|"enddocbookonly"|"endrtfonly"|"endmanonly"|"f$"|"f]"|"f}") {
      /* end of verbatim block */

      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      if (text.mid(1,1) == g_blockName) {          
         // end of formula
         BEGIN(g_lastCommentContext);

      } else if (text.mid(4) == g_blockName) {
         BEGIN(g_lastCommentContext);
      }
   }

<VerbatimCode>"{"     {
      if (g_javaBlock == 0) {
         REJECT;

      } else {
         g_javaBlock++;

         QString text = QString::fromUtf8(yytext);
         copyToOutput(text, text.length());
      }
   }

<VerbatimCode>"}"        {
      if (g_javaBlock == 0) {
         REJECT;

      } else {
         g_javaBlock--;

         if (g_javaBlock == 0) {
            copyToOutput(" @endcode ", 10);
            BEGIN(g_lastCommentContext);

         } else {
            QString text = QString::fromUtf8(yytext);
            copyToOutput(text, text.length());
         }
      }
   }

<VerbatimCode>[\\@]("enddot"|"endcode"|"endmsc"|"enduml") {
      /* end of verbatim block */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      if (text.mid(4) == g_blockName) {
         BEGIN(g_lastCommentContext);
      }
   }

<VerbatimCode>^[ \t]*"//"[\!\/]?   {
      /* skip leading comments */
      QString text = QString::fromUtf8(yytext);

      if (! g_inSpecialComment) {
         copyToOutput(text, text.length());

      } else {
         int len = 0;

         while (len < text.length() && (text[len] == ' ' || text[len] == '\t')) {
            len++;
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

<Verbatim,VerbatimCode>[^@\/\\\n{}]* {
      /* any character not a backslash or new line or } */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<Verbatim,VerbatimCode>\n     {
      /* new line in verbatim block */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<Verbatim>^[ \t]*"///"             {
      if (g_blockName == "dot" || g_blockName == "msc" || g_blockName == "uml" || g_blockName.startsWith('f') ) {
         // strip /// from dot images and formulas.

         QString text = QString::fromUtf8(yytext);
         int len = 0;

         while (len < text.length() && (text[len] == ' ' || text[len] == '\t')) {
            len++;
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
      copyToOutput(text, text.length());
   }

<SkipString>"\""              {
      /* end of string */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      BEGIN(g_stringContext);
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
      copyToOutput(text, text.length());
   }

<SkipChar>'                    {
      /* end of character literal */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      BEGIN(g_charContext);
   }

<SkipChar>.                    {
      /* any other string character */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<SkipChar>\n                   {
      /* new line character */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<CComment>[^\\!@*\n{\"\/]*     {
      /* anything that is not a '*' or command */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<CComment>"*"+[^*/\\@\n{\"]*   {
      /* stars without slashes */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<CComment>"\"\"\""             {
      /* end of Python docstring */

      if (g_lang != SrcLangExt_Python) {
         REJECT;

      } else {
         g_pythonDocString = false;

         QString text = QString::fromUtf8(yytext);
         copyToOutput(text, text.length());

         BEGIN(Scan);
      }
   }

<CComment>\n                    {
      /* new line in comment */
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      /* in case of Fortran always end of comment */
      if (g_lang == SrcLangExt_Fortran) {
         BEGIN(Scan);
      }
   }

<CComment>"/"+"*"                  {
      /* nested C comment */
      QString text = QString::fromUtf8(yytext);

      g_nestingCount++;
      g_commentStack.push(new CommentCtx(g_lineNr));
      copyToOutput(text, text.length());
   }

<CComment>"*"+"/"                  {
      /* end of C comment */
      if (g_lang == SrcLangExt_Python) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);
         copyToOutput(text, text.length());

         if (g_nestingCount <= 0) {
            BEGIN(Scan);

         } else {
            g_nestingCount--;
            delete g_commentStack.pop();
         }
      }
   }

<CComment>"\n"/[ \t]*[^#]     {
      /* end of Python comment */
      if (g_lang != SrcLangExt_Python || g_pythonDocString) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);
         copyToOutput(text, text.length());

         BEGIN(Scan);
      }
   }

<CComment>"\n"/[ \t]*[^\-]       {
      REJECT;
   }

   /* removed for bug 674842 (bug was introduced in rev 768)
<CComment>"'"           {
      g_charContext = YY_START;
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
      BEGIN(SkipChar);
   }

<CComment>"\""             {
      g_stringContext = YY_START;
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
      BEGIN(SkipString);
   }
   */

<CComment>.             {
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
      g_readLineCtx = YY_START;

      BEGIN(ReadLine);
   }

<SComment>\n[ \t]*"//"[\/!]("<")?[ \t]*[\\@]"}".*\n {
      /* See Bug 752712: end the multiline comment when finding a @} or \} command */

      QString text = QString::fromUtf8(yytext);

      copyToOutput(" */",3);
      copyToOutput(text, text.length());

      g_inSpecialComment = false;
      g_inRoseComment    = false;

      BEGIN(Scan);
   }

<SComment>\n[ \t]*"///"[^\/\n]/.*\n  {
      replaceComment(1);
      g_readLineCtx = YY_START;

      BEGIN(ReadLine);
   }

<SComment>^[ \t]*"//!"             |    // just //!
<SComment>^[ \t]*"//!<"/.*\n       |    // or   //!< something
<SComment>^[ \t]*"//!"[^<]/.*\n    {    // or   //!something
      replaceComment(0);
      g_readLineCtx = YY_START;

      BEGIN(ReadLine);
   }

<SComment>\n[ \t]*"//!"            |
<SComment>\n[ \t]*"//!<"/.*\n      |
<SComment>\n[ \t]*"//!"[^<\n]/.*\n {
      replaceComment(1);
      g_readLineCtx = YY_START;

      BEGIN(ReadLine);
   }

<SComment>^[ \t]*"//##"/.*\n       {
      if (! g_inRoseComment) {
         REJECT;

      } else {
         replaceComment(0);
         g_readLineCtx = YY_START;

         BEGIN(ReadLine);
      }
   }

<SComment>\n[ \t]*"//##"/.*\n      {
      if (! g_inRoseComment) {
         REJECT;

      } else {
         replaceComment(1);
         g_readLineCtx = YY_START;

         BEGIN(ReadLine);
      }
   }

<SComment>\n            {
      /* end of special comment */
      QString text = QString::fromUtf8(yytext);

      copyToOutput(" */",3);
      copyToOutput(text, text.length());

      g_inSpecialComment = false;
      g_inRoseComment    = false;

      BEGIN(Scan);
   }

<ReadLine>[^\\@\n]*/\n        {
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());

      BEGIN(g_readLineCtx);
   }

<CComment,ReadLine>[\\@][\\@][~a-z_A-Z][a-z_A-Z0-9]*[ \t]* {
      // escaped command
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

<CComment,ReadLine>[\\@]"cond"/[^a-z_A-Z0-9]     {
      // conditional section
      g_condCtx = YY_START;
      BEGIN(CondLine);
   }

<CComment,ReadLine>[\\@]"endcond"/[^a-z_A-Z0-9]  {
      // end of conditional section
      bool oldSkip = g_skip;
      endCondSection();

      if (YY_START == CComment && oldSkip && ! g_skip) {

         if (g_lang != SrcLangExt_Python && g_lang != SrcLangExt_Markdown && g_lang != SrcLangExt_Fortran) {
            ADDCHAR('/');
            ADDCHAR('*');

            if (g_specialComment) {
               ADDCHAR('*');
            }
         }
      }
   }

<CondLine>[!()&| \ta-z_A-Z0-9.\-]+ {
      QString text = QString::fromUtf8(yytext);

      bool oldSkip = g_skip;
      startCondSection(text);

      if ((g_condCtx == CComment || g_readLineCtx == SComment) && ! oldSkip && g_skip) {

         if (g_lang != SrcLangExt_Python && g_lang != SrcLangExt_Markdown && g_lang != SrcLangExt_Fortran)   {

            ADDCHAR('*');
            ADDCHAR('/');
         }
      }

      if (g_readLineCtx == SComment) {
         BEGIN(SComment);

      } else {
         BEGIN(g_condCtx);
      }
   }

<CondLine>[ \t]*
<CComment,ReadLine>[\\@]"cond"[ \t\r]*/\n |
<CondLine>.             {
      // forgot section id?
      QString text = QString::fromUtf8(yytext);

      if (YY_START != CondLine) {
         g_condCtx = YY_START;
      }

      bool oldSkip = g_skip;
      startCondSection(" ");    // fake section id causing the section to be hidden unconditionally

      if ((g_condCtx == CComment || g_readLineCtx == SComment) && ! oldSkip && g_skip) {
         if (g_lang != SrcLangExt_Python) {
            ADDCHAR('*');
            ADDCHAR('/');
         }
      }

      if (text[0] == '\n') {
         g_lineNr++;
      }

      if (g_readLineCtx == SComment) {
         BEGIN(SComment);

      } else {
         BEGIN(g_condCtx);
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

      g_lastBlockContext = YY_START;
      g_blockCount       = 1;
      g_aliasString      = text;
      g_lastEscaped      = 0;

      BEGIN( ReadAliasArgs );
   }

<ReadAliasArgs>^[ \t]*"//"[/!]/[^\n]+   {
      // skip leading special comments (see bug 618079)
   }

<ReadAliasArgs>"*/"        {
      // end of comment in the middle of an alias?
      if (g_lang == SrcLangExt_Python) {
         REJECT;

      } else  {
         // abort the alias, restart scanning
         QString text = QString::fromUtf8(yytext);

         copyToOutput(g_aliasString, g_aliasString.length());
         copyToOutput(text, text.length());

         BEGIN(Scan);
      }
   }

<ReadAliasArgs>[^{}\n\\\*]+      {
      g_aliasString += QString::fromUtf8(yytext);
      g_lastEscaped  = false;
   }

<ReadAliasArgs>"\\"        {

      if (g_lastEscaped) {
         g_lastEscaped = false;

      } else{
         g_lastEscaped = true;

      }

      g_aliasString += QString::fromUtf8(yytext);
   }

<ReadAliasArgs>\n          {
      g_aliasString += QString::fromUtf8(yytext);
      g_lineNr++;
      g_lastEscaped = false;
   }

<ReadAliasArgs>"{"         {
      g_aliasString += QString::fromUtf8(yytext);

      if (!g_lastEscaped) {
         g_blockCount++;
      }

      g_lastEscaped = false;
   }

<ReadAliasArgs>"}"         {
      g_aliasString += QString::fromUtf8(yytext);

      if (! g_lastEscaped) {
         g_blockCount--;
      }

      if (g_blockCount == 0) {
         replaceAliases(g_aliasString);
         BEGIN( g_lastBlockContext );
      }

      g_lastEscaped = false;

   }

<ReadAliasArgs>.        {
      g_aliasString += QString::fromUtf8(yytext);
      g_lastEscaped =  false;
   }

<ReadLine>.             {
      QString text = QString::fromUtf8(yytext);
      copyToOutput(text, text.length());
   }

%%

void replaceComment(int offset)
{
   QString text = QString::fromUtf8(yytext);

   if (g_mlBrief || g_skip) {
      copyToOutput(text, text.length());

   } else {
      int i = computeIndent(text.mid(offset));

      if (i == g_blockHeadCol) {
         replaceCommentMarker(text, text.length());

      } else {
         copyToOutput(" */", 3);

         for (int i = yyleng - 1; i >= 0; i--) {         
            char tmp = yytext[i];
            unput(tmp);
         }

         g_inSpecialComment = false;
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
      column++;

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
   g_mlBrief  = Config::getBool("multiline-cpp-brief");

   g_inBuf    = inBuf;
   g_outBuf   = "";

   g_inBufPos = 0;
   g_col      = 0;
   g_skip     = false;
   g_fileName = fileName;
   g_lang     = getLanguageFromFileName(fileName);
   g_lineNr   = 1;

   g_pythonDocString = false;

   g_condStack.clear();
   g_commentStack.clear();

   printlex(commentcnvYY_flex_debug, true, __FILE__, csPrintable(fileName));
   isFixedForm = false;

   if (g_lang == SrcLangExt_Fortran) {
      isFixedForm = recognizeFixedForm(inBuf);
   }

   if (g_lang == SrcLangExt_Markdown) {
      g_nestingCount = 0;
      BEGIN(CComment);
      g_commentStack.push(new CommentCtx(g_lineNr));

   } else {
      BEGIN(Scan);
   }

   yylex();

   while (! g_condStack.isEmpty()) {
      CondCtx *ctx = g_condStack.pop();
      QString sectionInfo = " ";

      if (ctx->sectionId != " ") {
         sectionInfo = QString(" with label %1 ").arg(ctx->sectionId);
      }

      warn(g_fileName, ctx->lineNr, "Conditional section %s does not have "
           "a corresponding \\endcond command", csPrintable(sectionInfo));
   }

   if (g_nestingCount > 0 && g_lang != SrcLangExt_Markdown) {
      QString tmp = "(probable line reference: ";

      bool first = true;

      while (! g_commentStack.isEmpty()) {
         CommentCtx *ctx = g_commentStack.pop();

         if (! first) {
            tmp += ", ";
         }

         tmp += QString::number(ctx->lineNr);
         first = false;
         delete ctx;
      }

      tmp += ")";
      warn(g_fileName, g_lineNr, "Reached end of file while still inside a (nested) comment. "
           "Nesting level %d %s", g_nestingCount + 1, csPrintable(tmp) );

      // add one for "normal" expected end of comment
   }

   g_commentStack.clear();
   g_nestingCount = 0;

   if (Debug::isFlagSet(Debug::CommentCnv)) {
      msg("-------------\n%s\n-------------\n", csPrintable(g_outBuf));
   }

   printlex(commentcnvYY_flex_debug, false, __FILE__, csPrintable(fileName) );

   return g_outBuf;
}