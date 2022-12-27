/*************************************************************************
 *
 * Copyright (c) 2014-2022 Barbara Geller & Ansel Sermersheim
 * Copyright (c) 1997-2014 Dimitri van Heesch

*************************************************************************/

%{

#include <QFile>
#include <QString>
#include <QStack>
#include <QRegularExpression>

#include <ctype.h>

#include <doctokenizer.h>
#include <config.h>
#include <cmdmapper.h>
#include <definition.h>
#include <doxy_globals.h>
#include <message.h>
#include <membergroup.h>
#include <portable.h>
#include <util.h>

#define YY_NO_INPUT 1

#define TK_COMMAND_SEL() (yytext[0] == '@' ? TK_COMMAND_AT : TK_COMMAND_BS)

// global
TokenInfo          *g_token = nullptr;

// context for tokenizer phase
static int          s_commentState;

static int          s_inputPosition = 0;
static QString      s_inputString;

static QString      s_fileName;
static bool         s_insidePre;

static int          s_yyLineNum = 0;

static QString      s_secLabel;
static QString      s_secTitle;

static QString      s_endMarker;
static int          s_autoListLevel;
static int          s_sharpCount = 0;

static SectionInfo::SectionType    s_secType;

// context for section finding phase
static QSharedPointer<Definition>  s_definition;
static QSharedPointer<MemberGroup> s_memberGroup;

struct DocLexerContext {
   TokenInfo *token;
   int rule;
   int autoListLevel;
   int inputPos;
   QString inputString;
   YY_BUFFER_STATE state;
};
static QStack<DocLexerContext *> s_lexerStack;


// static functions
static void yyunput(QChar c, char *);

void doctokenizerYYpushContext()
{
  DocLexerContext *ctx = new DocLexerContext;

  ctx->rule          = YY_START;
  ctx->autoListLevel = s_autoListLevel;
  ctx->token         = g_token;
  ctx->inputPos      = s_inputPosition;
  ctx->inputString   = s_inputString;
  ctx->state         = YY_CURRENT_BUFFER;
  s_lexerStack.push(ctx);

  yy_switch_to_buffer(yy_create_buffer(doctokenizerYYin, YY_BUF_SIZE));
}

bool doctokenizerYYpopContext()
{
   if (s_lexerStack.isEmpty()) {
      return false;
   }

   DocLexerContext *ctx = s_lexerStack.pop();

   s_autoListLevel  = ctx->autoListLevel;
   s_inputPosition  = ctx->inputPos;
   s_inputString    = ctx->inputString;

   yy_delete_buffer(YY_CURRENT_BUFFER);
   yy_switch_to_buffer(ctx->state);
   BEGIN(ctx->rule);

   delete ctx;

   return TRUE;
}

QString extractTextAfterNewLine(const QString &text)
{
   auto iter_1 = text.indexOfFast('\n');
   auto iter_2 = text.indexOfFast("\\internal_linebr");

   if (iter_1 != text.end() && iter_1 < iter_2) {
      return QString(iter_1 + 1, text.end());
   }

   if (iter_2 != text.end()) {
      iter_2 = iter_2 + 16;

      if (iter_2 != text.end()) {

         if (*iter_2 == ' ') {
            // skip space after \\internal_linebr
            ++iter_2;
         }
      }

      return QString(iter_2, text.end());
   }

   return text;
}


QString tokToString(int token)
{
   QString retval = "ERROR";

   switch (token) {
      case 0:
         retval = "TK_EOF";
         break;

      case TK_WORD:
         retval = "TK_WORD";
         break;

      case TK_LNKWORD:
         retval = "TK_LNKWORD";
         break;

      case TK_WHITESPACE:
         retval = "TK_WHITESPACE";
         break;

      case TK_LISTITEM:
         retval = "TK_LISTITEM";
         break;

      case TK_ENDLIST:
         retval = "TK_ENDLIST";
         break;

      case TK_COMMAND_AT:
         retval = "TK_COMMAND_AT";
         break;

      case TK_COMMAND_BS:
         retval = "TK_COMMAND_BS";
         break;

      case TK_HTMLTAG:
         retval = "TK_HTMLTAG";
         break;

      case TK_SYMBOL:
         retval = "TK_SYMBOL";
         break;

      case TK_NEWPARA:
         retval = "TK_NEWPARA";
         break;

      case TK_RCSTAG:
         retval = "TK_RCSTAG";
         break;

      case TK_URL:
         retval = "TK_URL";
         break;
   }

   return retval;
}

static int computeIndent(const QString &str, int length)
{
   static const int tabSize = Config::getInt("tab-size");

   int i;
   int indent = 0;

   for (i = 0; i < length; i++) {

      if (str[i] == '\t') {
         indent += tabSize - (indent % tabSize);

      } else if (str[i] == '\n') {
         indent = 0;

      } else {
         ++indent;
      }
   }

   return indent;
}

static void processSection()
{
   QString file;

   if (s_memberGroup) {
      file = s_memberGroup->parent()->getOutputFileBase();

   } else if (s_definition) {
      file = s_definition->getOutputFileBase();

   } else {
      warn(s_fileName, s_yyLineNum, "Found section/anchor %s without context\n", csPrintable(s_secLabel));

   }

   QSharedPointer<SectionInfo> si;

   if ((si = Doxy_Globals::sectionDict.find(s_secLabel))) {

      si->fileName = file;
      si->type     = s_secType;
   }
}

static void handleHtmlTag()
{
   QString tagText = QString::fromUtf8(yytext);
   int len = tagText.length();

   g_token->attribs.clear();
   g_token->endTag   = false;
   g_token->emptyTag = false;

   // check for end tag
   int startNamePos = 1;

   if (len > 1 && tagText.at(1) == '/') {
      g_token->endTag = true;
      ++startNamePos;
   }

   // parse the name portion
   int i;

   for (i = startNamePos; i < len; ++i) {
      // check for valid HTML/XML name chars (including namespaces)
      QChar c = tagText.at(i);

      if (! ( c.isLetterOrNumber() || c == '-' || c == '_' || c == ':')) {
         break;
      }
   }

   g_token->name = tagText.mid(startNamePos, i - startNamePos);

   // parse the attributes, each attribute consists of a name and a value
   // result is stored in g_token->attribs

   int startName;
   int endName;
   int startAttrib;
   int endAttrib;

   while (i < len) {
      // skip spaces
      QChar c = tagText.at(i);

      while (i < len && c.isSpace()) {
         ++i;

         if (i >= len) {
            break;
         }

         c = tagText.at(i);
      }

      // check for end of the tag
      if (c == '>') {
         break;
      }

      // check for XML style "empty" tag
      if (c == '/') {
         g_token->emptyTag = true;
         break;
      }

      startName = i;

      // search for end of name
      while (i < len && ! c.isSpace() && c != '=' && c!= '>' ) {
         ++i;

         if (i >= len) {
            break;
         }

         c = tagText.at(i);
      }

      endName = i;

      HtmlAttrib opt;
      opt.name = tagText.mid(startName, endName - startName).toLower();

      // skip spaces
      while (i < len && c.isSpace()) {
         ++i;

         if (i >= len) {
            break;
         }

         c = tagText.at(i);
      }

      if (i < len && tagText.at(i) == '=') {
         ++i;

         if (i < len) {
            // option has value
            c = tagText.at(i);
         } else {
            c = '\0';
         }

         // skip spaces
         while (i < len && c.isSpace()) {
            ++i;

            if (i >= len) {
               break;
            }

            c = tagText.at(i);
         }

         if (i < len && tagText.at(i) == '\'') {
            ++i;

            if (i < len) {
               c = tagText.at(i);
            } else {
               c = '\0';
            }

            startAttrib = i;

            // search for matching quote
            while (i < len && c != '\'') {
               ++i;

               if (i >= len) {
                  break;
               }

               c = tagText.at(i);
            }

            endAttrib = i;
            ++i;

            if (i < len) {
               c = tagText.at(i);
            } else {
               c = '\0';
            }

         } else if (tagText.at(i) == '"') {

            ++i;

            if (i < len) {
               c = tagText.at(i);
            } else {
               c = '\0';
            }

            startAttrib = i;

            // search for matching quote
            while (i < len && c != '"') {
               ++i;

               if (i >= len) {
                  break;
               }

               c = tagText.at(i);
            }

            endAttrib = i;
            ++i;

            if (i < len) {
               c = tagText.at(i);
            } else {
               c = '\0';
            }

         } else {
            // value without any quotes
            startAttrib = i;

            // search for separator or end symbol
            while (i < len && ! c.isSpace() && c != '>') {
               ++i;

               if (i >= len) {
                  break;
               }

               c = tagText.at(i);
            }

            endAttrib = i;
            ++i;

            if (i < len) {
               // option has value
               c = tagText.at(i);

            } else {
               c = '\0';
            }
         }

         opt.value = tagText.mid(startAttrib, endAttrib - startAttrib);

         if (opt.name == "align") {
            opt.value = opt.value.toLower();

         } else if (opt.name == "valign") {
            opt.value = opt.value.toLower();

            if (opt.value == "center") {
               opt.value = "middle";
            }

         }

      } else {
         // start next option

      }

      g_token->attribs.append(opt);
   }
}

static QString stripEmptyLines(const QString &s)
{
   if (s.isEmpty()) {
      return QString();
   }

   int end   = s.length();
   int start = 0;
   int p     = 0;

   // skip leading empty lines
   while (true) {

      while (p < end) {
         QChar c = s[p];

         if (c != ' ' && c != '\t')  {
            break;
         }

         ++p;
      }

      if (p < end && s[p] == '\n') {
         start = ++p;

      } else {
         break;

      }
   }

   // skip trailing empty lines
   p = end - 1;
   if (p >= start && s.at(p) == '\n') {
      --p;
   }

   while (p >= start) {
      QChar c;

      while ((c = s[p]) != 0  && (c == ' ' || c == '\t')) {
         --p;
      }

      if (s[p] == '\n') {
         end = p;
      } else {
         break;
      }

      --p;
   }

   return s.mid(start, end - start);
}

#undef  YY_INPUT
#define YY_INPUT(buf, result, max_size) result = yyread(buf,max_size);

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

CMD         ("\\"|"@")
WS          [ \t\r\n]
NONWS       [^ \t\r\n]
BLANK       [ \t\r]
ID          [$a-z_A-Z\x80-\xFF][$a-z_A-Z0-9\x80-\xFF]*
LABELID     [a-z_A-Z\x80-\xFF][a-z_A-Z0-9\x80-\xFF\-]*
PHPTYPE     [\\:a-z_A-Z0-9\x80-\xFF\-]+
CITESCHAR   [a-z_A-Z0-9\x80-\xFF\-\?]
CITEECHAR   [a-z_A-Z0-9\x80-\xFF\-\+:\/\?]
CITEID      {CITESCHAR}{CITEECHAR}*("."{CITESCHAR}{CITEECHAR}*)*|"\""{CITESCHAR}{CITEECHAR}*("."{CITESCHAR}{CITEECHAR}*)*"\""
MAILADR     ("mailto:")?[a-z_A-Z0-9.+-]+"@"[a-z_A-Z0-9-]+("."[a-z_A-Z0-9\-]+)+[a-z_A-Z0-9\-]+
MAILWS      [\t a-z_A-Z0-9+-]
MAILADDR2   {MAILWS}+{BLANK}+("at"|"AT"|"_at_"|"_AT_"){BLANK}+{MAILWS}+("dot"|"DOT"|"_dot_"|"_DOT_"){BLANK}+{MAILWS}+

LISTITEM    {BLANK}*[-]("#")?{WS}
MLISTITEM   {BLANK}*[+*]{WS}
OLISTITEM   {BLANK}*[1-9][0-9]*"."{BLANK}
ENDLIST     {BLANK}*"."{BLANK}*\n
ATTRNAME    [a-z_A-Z\x80-\xFF][:a-z_A-Z0-9\x80-\xFF\-]*
ATTRIB      {ATTRNAME}{WS}*("="{WS}*(("\""[^\"]*"\"")|("'"[^\']*"'")|[^ \t\r\n'"><]+))?
URLCHAR     [a-z_A-Z0-9\!\~\,\:\;\'\$\?\@\&\%\#\.\-\+\/\=\x80-\xFF]
URLMASK     ({URLCHAR}+([({]{URLCHAR}*[)}])?)+
URLPROTOCOL ("http:"|"https:"|"ftp:"|"ftps:"|"sftp:"|"file:"|"news:"|"irc:"|"ircs:")
FILE_I_CHAR [a-z_A-Z0-9\\:\\\/\-\+=&#@]
FILE_E_CHAR [a-z_A-Z0-9\-\+=&#@]
FILE_CHARS  {FILE_I_CHAR}*{FILE_E_CHAR}+

H_FILEMASK  {FILE_I_CHAR}*("."{FILE_I_CHAR}+)+{FILE_CHARS}*
V_FILEMASK  {FILE_CHARS}("."{FILE_CHARS})*
FILEMASK    {V_FILEMASK}|{H_FILEMASK}

LINKMASK    [^ \t\n\r\\@<&${}]+("("[^\n)]*")")?({BLANK}*("const"|"volatile"){BLANK}+)?
VERBATIM    "verbatim"{BLANK}*
SPCMD1      {CMD}([a-z_A-Z][a-z_A-Z0-9]*|{VERBATIM}|"--"|"---")
SPCMD2      {CMD}[\\@<>&$#%~".+=|-]
SPCMD3      {CMD}_form#[0-9]+
SPCMD4      {CMD}"::"
SPCMD5      {CMD}":"
INOUT       "in"|"out"|("in"{BLANK}*","?{BLANK}*"out")|("out"{BLANK}*","?{BLANK}*"in")
PARAMIO     {CMD}param{BLANK}*"["{BLANK}*{INOUT}{BLANK}*"]"
VARARGS     "..."
TEMPCHAR    [a-z_A-Z0-9.,: \t\*\&\(\)\[\]]
FUNCCHAR    [a-z_A-Z0-9,:\<\> \t\n\^\*\&\[\]]|{VARARGS}|"\\internal_linebr"
FUNCPART    {FUNCCHAR}*("("{FUNCCHAR}*")"{FUNCCHAR}*)?
SCOPESEP    "::"|"#"|"."
TEMPLPART   "<"{TEMPCHAR}*("<"{TEMPCHAR}*("<"{TEMPCHAR}*">")?">")?">"
ANONNS      "anonymous_namespace{"[^}]*"}"
SCOPEPRE    (({ID}{TEMPLPART}?)|{ANONNS}){SCOPESEP}
SCOPEKEYS   ":"({ID}":")*

SCOPECPP    {SCOPEPRE}*(~)?{ID}{TEMPLPART}?
SCOPEOBJC   {SCOPEPRE}?{ID}{SCOPEKEYS}?
SCOPEMASK   {SCOPECPP}|{SCOPEOBJC}

FUNCARG     "("{FUNCPART}")"({BLANK}*("volatile"|"const"){BLANK})?
FUNCARG2    "("{FUNCPART}")"({BLANK}*("volatile"|"const"))?
OPNEW       {BLANK}+"new"({BLANK}*"[]")?
OPDEL       {BLANK}+"delete"({BLANK}*"[]")?

OPNORM      {OPNEW}|{OPDEL}|"+"|"-"|"*"|"/"|"%"|"^"|"&"|"|"|"~"|"!"|"="|"<"|">"|"+="|"-="|"*="|"/="|"%="|"^="|"&="|"|="|"<<"|">>"|"<<="|">>="|"=="|"!="|"<="|">="|"&&"|"||"|"++"|"--"|","|"->*"|"->"|"[]"|"()"|"<=>"

OPCAST      {BLANK}+[^<(\r\n.,][^(\r\n.,]*
OPMASK      ({BLANK}*{OPNORM}{FUNCARG})
OPMASKOPT   ({BLANK}*{OPNORM}{FUNCARG}?)|({OPCAST}{FUNCARG})
OPMASKOP2   ({BLANK}*{OPNORM}{FUNCARG2}?)|({OPCAST}{FUNCARG2})

CVSPEC      {BLANK}*("const"|"volatile")
LNKWORD1    ("::"|"#")?{SCOPEMASK}
LNKWORD2    (({SCOPEPRE}*"operator"{OPMASK})|({SCOPEPRE}"operator"{OPMASKOPT})|(("::"|"#"){SCOPEPRE}*"operator"{OPMASKOPT})){CVSPEC}?
LNKWORD3    ([0-9a-z_A-Z\-]+("/"|"\\"))*[0-9a-z_A-Z\-]+("."[0-9a-z_A-Z]+)+

ESCWORD     ("%"{ID}(("::"|"."){ID})*)|("%'")
CHARWORD1   [^ \t\n\r\\@<>()\[\]:;\?{}&%$#,."=']
CHARWORD2   [^ \-+0-9\t\n\r\\@<>()\[\]:;\?{}&%$#,."=']

WORD1       {ESCWORD}|{CHARWORD2}{CHARWORD1}*|"{"|"}"|"'\"'"|("\""([^"\n]*(\\\"|\n)?)*[^"\n]*"\"")
WORD2       "."|","|"("|")"|"["|"]"|"::"|":"|";"|"\?"|"="|"'"

WORD1NQ     {ESCWORD}|{CHARWORD1}+|"{"|"}"
WORD2NQ     "."|","|"("|")"|"["|"]"|"::"|":"|";"|"\?"|"="|"'"
CAPTION     [cC][aA][pP][tT][iI][oO][nN]
HTMLTAG     "<"(("/")?){ID}({WS}+{ATTRIB})*{WS}*(("/")?)">"

HTMLKEYL  "strong"|"center"|"table"|"caption"|"small"|"code"|"dfn"|"var"|"img"|"pre"|"sub"|"sup"|"tr"|"td"|"th"|"ol"|"ul"|"li"|"tt"|"kbd"|"em"|"hr"|"dl"|"dt"|"dd"|"br"|"i"|"a"|"b"|"p"|"strike"|"u"|"del"|"ins"|"s"
HTMLKEYU  "STRONG"|"CENTER"|"TABLE"|"CAPTION"|"SMALL"|"CODE"|"DFN"|"VAR"|"IMG"|"PRE"|"SUB"|"SUP"|"TR"|"TD"|"TH"|"OL"|"UL"|"LI"|"TT"|"KBD"|"EM"|"HR"|"DL"|"DT"|"DD"|"BR"|"I"|"A"|"B"|"P"|"STRIKE"|"U"|"DEL"|"INS"|"S"

HTMLKEYW  {HTMLKEYL}|{HTMLKEYU}

REFWORD2_PRE   ("#"|"::")?((({ID}{TEMPLPART}?)|{ANONNS})("."|"#"|"::"|"-"|"/"))*({ID}{TEMPLPART}?(":")?)
REFWORD2       {REFWORD2_PRE}{FUNCARG2}?
REFWORD2_NOCV  {REFWORD2_PRE}("("{FUNCPART}")")?
REFWORD3       ({ID}":")*{ID}":"?
REFWORD4_NOCV  (({SCOPEPRE}*"operator"{OPMASKOP2})|(("::"|"#"){SCOPEPRE}*"operator"{OPMASKOP2}))
REFWORD4       {REFWORD4_NOCV}{CVSPEC}?
REFWORD        {FILEMASK}|{LABELID}|{REFWORD2}|{REFWORD3}|{REFWORD4}
REFWORD_NOCV   {FILEMASK}|{LABELID}|{REFWORD2_NOCV}|{REFWORD3}|{REFWORD4_NOCV}

RCSID          "$"("Author"|"Date"|"Header"|"Id"|"Locker"|"Log"|"Name"|"RCSfile"|"Revision"|"Source"|"State")":"[^:\n$][^\n$]*"$"

%option never-interactive
%option nounistd
%option noyywrap
%option yylineno

%x St_Para
%x St_Comment
%x St_Title
%x St_TitleN
%x St_TitleQ
%x St_TitleA
%x St_TitleV
%x St_Code
%x St_CodeOpt
%x St_XmlCode
%x St_HtmlOnly
%x St_HtmlOnlyOption
%x St_ManOnly
%x St_LatexOnly
%x St_RtfOnly
%x St_XmlOnly
%x St_DbOnly
%x St_Verbatim
%x St_Dot
%x St_Msc
%x St_PlantUMLOpt
%x St_PlantUML
%x St_Param
%x St_XRefItem
%x St_XRefItem2
%x St_File
%x St_Pattern
%x St_Link
%x St_Cite
%x St_Ref
%x St_Ref2
%x St_IntRef
%x St_Text
%x St_SkipTitle
%x St_Anchor
%x St_Snippet
%x St_SetScope
%x St_SetScopeEnd
%x St_Options
%x St_Block
%x St_Emoji

%x St_Sections
%s St_SecLabel1
%s St_SecLabel2
%s St_SecTitle
%x St_SecSkip

%%
<St_Para>\r    {
      /* skip carriage return */
   }

<St_Para>^{LISTITEM}   {
      /* list item */
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      int dashPos = text.lastIndexOf('-');

      g_token->isEnumList = text.at(dashPos + 1) == '#';
      g_token->id         = -1;
      g_token->indent     = computeIndent(text, dashPos);

      return TK_LISTITEM;
   }

<St_Para>^{MLISTITEM}  {
      /* list item */
      if (! Doxy_Globals::markdownSupport || s_insidePre) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);
         s_yyLineNum += text.count('\n');

         static QRegularExpression regExp("^.*[*+]");
         QRegularExpressionMatch match = regExp.match(text);

         // assume match.hasMatch() is always true
         int listPos = match.capturedLength();

         g_token->isEnumList = false;
         g_token->id         = -1;
         g_token->indent     = computeIndent(text, listPos);

         return TK_LISTITEM;
      }
   }

<St_Para>^{OLISTITEM}  {
      /* numbered list item */
      if (! Doxy_Globals::markdownSupport || s_insidePre) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);

         static QRegularExpression re("[1-9]");
         int digitPos = text.indexOf(re);
         int dotPos   = text.indexOf('.',digitPos);

         g_token->isEnumList = true;
         g_token->id         = text.mid(digitPos, dotPos - digitPos).toInteger<int>();
         g_token->indent     = computeIndent(text, digitPos);
         return TK_LISTITEM;
      }
   }

<St_Para>{BLANK}*(\n|"\\internal_linebr"){LISTITEM}     {
      /* list item on next line */
      QString text = QString::fromUtf8(yytext);

      s_yyLineNum += text.count('\n');
      text        = extractTextAfterNewLine(text);
      int dashPos = text.lastIndexOf('-');

      g_token->isEnumList = text.at(dashPos + 1) == '#';
      g_token->id         = -1;
      g_token->indent     = computeIndent(text,dashPos);

      return TK_LISTITEM;
   }

<St_Para>{BLANK}*(\n|"\\internal_linebr"){MLISTITEM}     {
      /* list item on next line */
      if (! Doxy_Globals::markdownSupport || s_insidePre) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);

         s_yyLineNum += text.count('\n');
         text = extractTextAfterNewLine(text);

         static QRegularExpression regExp("^.*[*+]");
         QRegularExpressionMatch match = regExp.match(text);

         // assume match.hasMatch() is always true
         int markPos = match.capturedLength();

         g_token->isEnumList = false;
         g_token->id         = -1;
         g_token->indent     = computeIndent(text, markPos);

         return TK_LISTITEM;
      }
   }

<St_Para>{BLANK}*(\n|"\\internal_linebr"){OLISTITEM}     {
      /* list item on next line */

      if (! Doxy_Globals::markdownSupport || s_insidePre) {
         REJECT;

      } else {
         QString text = QString::fromUtf8(yytext);

         s_yyLineNum += text.count('\n');
         text        = extractTextAfterNewLine(text);

         static QRegularExpression regExp("([1-9]+)");
         QRegularExpressionMatch match = regExp.match(text);

         int digitPos = match.capturedStart() - text.begin();

         g_token->isEnumList = true;
         g_token->id         = match.captured().toInteger<int>();
         g_token->indent     = computeIndent(text, digitPos);

         return TK_LISTITEM;
      }
   }

<St_Para>^{ENDLIST}       {
      /* end list */
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      int dotPos = text.lastIndexOf('.');
      g_token->indent = computeIndent(text, dotPos);

      return TK_ENDLIST;
   }

<St_Para>{BLANK}*(\n|"\\internal_linebr"){ENDLIST}      {
      /* end list on next line */
      QString text = QString::fromUtf8(yytext);

      s_yyLineNum += text.count('\n');
      text        = extractTextAfterNewLine(text);
      int dotPos  = text.lastIndexOf('.');

      g_token->indent = computeIndent(text,dotPos);

      return TK_ENDLIST;
   }

<St_Para>"{"{BLANK}*"@linkplain"/{WS}+ {
      g_token->name = "javalinkplain";
      return TK_COMMAND_AT;
   }

<St_Para>"{"{BLANK}*"@link"/{WS}+ {
      g_token->name = "javalink";
      return TK_COMMAND_AT;
   }

<St_Para>"{"{BLANK}*"@inheritDoc"{BLANK}*"}" {
      g_token->name = "inheritdoc";
      return TK_COMMAND_AT;
   }

<St_Para>"@_fakenl"     {
      // artificial new line
   }

<St_Para>{SPCMD3}      {
      g_token->name = "form";

      bool ok;
      QString text = QString::fromUtf8(yytext);
      g_token->id  = text.right(text.length() - 6).toInteger<int>(&ok);
      assert(ok);

      return TK_COMMAND_SEL();
   }

<St_Para>{CMD}"n"\n    {
      /* \n followed by real newline */
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->name     = text.mid(1).trimmed();
      g_token->paramDir = TokenInfo::Unspecified;

      return TK_COMMAND_SEL();
   }

<St_Para>"\\internal_linebr"  {
   }

<St_Para>{SPCMD1}             |
<St_Para>{SPCMD2}             |
<St_Para>{SPCMD4}             |
<St_Para>{SPCMD5}     {
      /* special command */
      QString text  = QString::fromUtf8(yytext);

      g_token->name     = text.mid(1).trimmed();
      g_token->paramDir = TokenInfo::Unspecified;
      return TK_COMMAND_SEL();
   }

<St_Para>{PARAMIO}     {
      /* param [in,out] command */
      QString text = QString::fromUtf8(yytext);

      g_token->name = "param";

      bool isIn  = text.indexOf("in")  != -1;
      bool isOut = text.indexOf("out") != -1;

      if (isIn) {
         if (isOut) {
            g_token->paramDir = TokenInfo::InOut;
         } else {
            g_token->paramDir = TokenInfo::In;
         }

      } else if (isOut) {
         g_token->paramDir = TokenInfo::Out;

      } else {
         g_token->paramDir = TokenInfo::Unspecified;
      }

      return TK_COMMAND_SEL();
   }

<St_Para>{URLPROTOCOL}{URLMASK}/[,\.] {
      // URL, or URL.
      g_token->name = QString::fromUtf8(yytext);
      g_token->isEMailAddr = false;

      return TK_URL;
   }

<St_Para>{URLPROTOCOL}{URLMASK}  {
      // URL
      g_token->name = QString::fromUtf8(yytext);
      g_token->isEMailAddr = false;
      return TK_URL;
   }

<St_Para>"<"{URLPROTOCOL}{URLMASK}">"  {
      // URL
      g_token->name = QString::fromUtf8(yytext);
      g_token->name = g_token->name.mid(1, g_token->name.length() - 2);
      g_token->isEMailAddr = false;

      return TK_URL;
   }

<St_Para>{MAILADR}     {
      // mail address
      g_token->name = QString::fromUtf8(yytext);
      g_token->name = stripPrefix(g_token->name, "mailto:");
      g_token->isEMailAddr = true;

      return TK_URL;
   }

<St_Para>"<"{MAILADR}">" {
      // Mail address
      g_token->name = QString::fromUtf8(yytext);
      g_token->name = g_token->name.mid(1, g_token->name.length() - 2);
      g_token->name = stripPrefix(g_token->name, "mailto:");
      g_token->isEMailAddr = true;

      return TK_URL;
   }

<St_Para>"<"{MAILADDR2}">"  {
      // anti spam mail address
      g_token->name = QString::fromUtf8(yytext);
      return TK_WORD;
   }

<St_Para>{RCSID} {
      /* RCS tag */
      QString text = QString::fromUtf8(yytext);

      QString tagName = text.mid(1);
      int index = tagName.indexOf(':');

      g_token->name  = tagName.left(index);
      int text_begin = index + 2;
      int text_end   = tagName.length() - 1;

      if (tagName[text_begin - 1] == ':')  {
         /* check for Subversion fixed-length keyword */
         ++text_begin;
      }

      if (tagName[text_end - 1] == '#') {
         --text_end;
      }

      g_token->text = tagName.mid(text_begin, text_end - text_begin);
      return TK_RCSTAG;
   }

<St_Para,St_HtmlOnly,St_ManOnly,St_LatexOnly,St_RtfOnly,St_XmlOnly,St_DbOnly>"$("{ID}")"   |
<St_Para,St_HtmlOnly,St_ManOnly,St_LatexOnly,St_RtfOnly,St_XmlOnly,St_DbOnly>"$("{ID}"("{ID}"))"   {
      /* environment variable */
      QString text = QString::fromUtf8(yytext);

      text = text.mid(2);
      text.chop(1);

      QString value = portable_getenv(text);

      for (int i = value.length() - 1; i >= 0; --i) {
         unput(value.at(i));
      }
   }

<St_Para>{HTMLTAG}     {
      /* html tag */
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      handleHtmlTag();
      return TK_HTMLTAG;
   }

<St_Para,St_Text>"&"{ID}";" {
      /* special symbol */
      g_token->name = QString::fromUtf8(yytext);
      return TK_SYMBOL;
   }


  /********* patterns for linkable words ******************/

<St_Para>{ID}/"<"{HTMLKEYW}">"         {
      // prevent opening html tag to be recognized as a templated classes
      g_token->name = QString::fromUtf8(yytext);
      return TK_LNKWORD;
   }

<St_Para>{LNKWORD1}/"<br>"    | // prevent <br> html tag to be parsed as template arguments
<St_Para>{LNKWORD1}           |
<St_Para>{LNKWORD1}{FUNCARG}  |
<St_Para>{LNKWORD2}           |
<St_Para>{LNKWORD3}                    {
      g_token->name = QString::fromUtf8(yytext);
      return TK_LNKWORD;
   }

<St_Para>{LNKWORD1}{FUNCARG}{CVSPEC}[^a-z_A-Z0-9] {
      g_token->name = QString::fromUtf8(yytext);
      g_token->name.chop(1);

      unput(yytext[doctokenizerYYleng - 1]);

      return TK_LNKWORD;
   }

  /********* patterns for normal words ******************/

<St_Para,St_Text>[0-9]+    |
<St_Para,St_Text>[\-+]     |
<St_Para,St_Text>{WORD1}/("<"{BLANK}*{HTMLKEYW}{BLANK}*">")?  |
<St_Para,St_Text>{WORD1}("<"{BLANK}*{ID}+{BLANK}*">")?        |
<St_Para,St_Text>{WORD1}   |
<St_Para,St_Text>{WORD2}               {
      // function call, rule resolves multiple digit numbers
      // matches MyClass<int> but not MyClass<i>
      QString text = QString::fromUtf8(yytext);

      if (text.contains("\\internal_linebr")) {
         REJECT;
      }

      s_yyLineNum += text.count('\n');

      if (text.startsWith('%')) {
         // strip % if present
         g_token->name = text.mid(1);
      } else {
         g_token->name = text;
      }

      return TK_WORD;
   }

<St_Text>({ID}".")+{ID}                {
      g_token->name = QString::fromUtf8(yytext);
      return TK_WORD;
   }

<St_Para,St_Text>"operator"/{BLANK}*"<"[a-zA-Z_0-9]+">" {
      // */ (editor syntax fix)
      // Special case: word "operator" followed by a HTML command
      // avoid interpretation as "operator <"

      g_token->name = QString::fromUtf8(yytext);

      return TK_WORD;
   }


  /*******************************************************/

<St_Para,St_Text>{BLANK}+      |
<St_Para,St_Text>{BLANK}*\n{BLANK}* {
      /* white space */
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->chars = text;
      return TK_WHITESPACE;
   }

<St_Text>[\\@<>&$#%~]  {
      g_token->name = QString::fromUtf8(yytext);
      return TK_COMMAND_SEL();
   }

<St_Para>({BLANK}*\n)+{BLANK}*\n/{LISTITEM} {
      /* skip trailing paragraph followed by new list item */
      if (s_insidePre || s_autoListLevel == 0) {
         REJECT;
      }

      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');
   }

<St_Para>({BLANK}*\n)+{BLANK}*\n/{MLISTITEM} {
      /* skip trailing paragraph followed by new list item */
      if (! Doxy_Globals::markdownSupport || s_insidePre || s_autoListLevel == 0) {
         REJECT;
      }

      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');
   }

<St_Para>({BLANK}*\n)+{BLANK}*\n/{OLISTITEM} {
      /* skip trailing paragraph followed by new list item */
      if (! Doxy_Globals::markdownSupport || s_insidePre || s_autoListLevel == 0) {
         REJECT;
      }

      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');
   }

<St_Para,St_Param>({BLANK}*(\n|"\\internal_linebr"))+{BLANK}*(\n|"\\internal_linebr"){BLANK}* {
      QString text = QString::fromUtf8(yytext);

      s_yyLineNum += text.count('\n');

      if (s_insidePre) {
          g_token->chars = text;
          return TK_WHITESPACE;

      } else {
         g_token->indent = computeIndent(text, text.length());

         // put back the indentation (needed for list items)
         for (int i = 0; i < g_token->indent; ++i) {
            unput(' ');
         }

         // tell flex that after putting the last indent
         // back we are at the beginning of the line
         YY_CURRENT_BUFFER->yy_at_bol = 1;

         // start of a new paragraph
         return TK_NEWPARA;
      }
   }

<St_CodeOpt>{BLANK}*"{"(".")?{LABELID}"}" {
      g_token->name = QString::fromUtf8(yytext);

      int i = g_token->name.indexOf('{');
      g_token->name = g_token->name.mid(i + 1, g_token->name.length() - i - 2);

      BEGIN(St_Code);
   }

<St_CodeOpt>"\\internal_linebr" |
<St_CodeOpt>\n                  |
<St_CodeOpt>.          {
      for (int i = yyleng - 1; i >= 0; --i) {
         unput(yytext[i]);
      }

      BEGIN(St_Code);
   }

<St_Code>{WS}*{CMD}"endcode" {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      return RetVal_OK;
   }

<St_XmlCode>{WS}*"</code>" {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      return RetVal_OK;
   }

<St_Code,St_XmlCode>[^\\@\n<]+  |
<St_Code,St_XmlCode>\n          |
<St_Code,St_XmlCode>.              {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->verb += text;
   }

<St_HtmlOnlyOption>" [block]" {
      // the space is added in commentscan.l
      g_token->name = "block";
      BEGIN(St_HtmlOnly);
   }

<St_HtmlOnlyOption>.|\n {
      unput(yytext[0]);
      BEGIN(St_HtmlOnly);
   }

<St_HtmlOnlyOption>"\\internal_linebr" {
      for (int i = yyleng - 1; i >= 0; --i) {
         unput(yytext[i]);
      }

      BEGIN(St_HtmlOnly);
   }

<St_HtmlOnly>{CMD}"endhtmlonly" {
      return RetVal_OK;
   }

<St_HtmlOnly>[^\\@\n$]+    |
<St_HtmlOnly>\n            |
<St_HtmlOnly>.                {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->verb += text;
   }

<St_ManOnly>{CMD}"endmanonly" {
      return RetVal_OK;
   }

<St_ManOnly>[^\\@\n$]+    |
<St_ManOnly>\n            |
<St_ManOnly>.                {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->verb += text;
   }

<St_RtfOnly>{CMD}"endrtfonly" {
      return RetVal_OK;
   }

<St_RtfOnly>[^\\@\n$]+    |
<St_RtfOnly>\n            |
<St_RtfOnly>.                {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->verb += text;
   }

<St_LatexOnly>{CMD}"endlatexonly" {
      return RetVal_OK;
   }

<St_LatexOnly>[^\\@\n]+     |
<St_LatexOnly>\n            |
<St_LatexOnly>.                {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->verb += text;
   }

<St_XmlOnly>{CMD}"endxmlonly" {
      return RetVal_OK;
   }

<St_XmlOnly>[^\\@\n]+  |
<St_XmlOnly>\n         |
<St_XmlOnly>.             {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->verb += text;
   }

<St_DbOnly>{CMD}"enddocbookonly" {
      return RetVal_OK;
   }

<St_DbOnly>[^\\@\n]+  |
<St_DbOnly>\n         |
<St_DbOnly>.              {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->verb += text;
   }

<St_Verbatim>{CMD}"endverbatim" {
      g_token->verb = stripEmptyLines(g_token->verb);
      return RetVal_OK;
   }

<St_Verbatim>[^\\@\n]+ |
<St_Verbatim>\n        |
<St_Verbatim>.            {
      /* Verbatim text */
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->verb += text;
   }

<St_Dot>{CMD}"enddot"  {
      return RetVal_OK;
   }

<St_Dot>[^\\@\n]+      |
<St_Dot>\n             |
<St_Dot>.                 {
      /* dot text */
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->verb += text;
   }

<St_Msc>{CMD}("endmsc")  {
      return RetVal_OK;
   }

<St_Msc>[^\\@\n]+      |
<St_Msc>\n             |
<St_Msc>.                 {
      /* msc text */
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->verb += text;
   }

<St_PlantUMLOpt>{BLANK}*"{"[^}]*"}" {
      // case 1: file name is specified as {filename}
      g_token->sectionId = QString::fromUtf8(yytext).trimmed();

      // skip curly brackets around the optional image name
      g_token->sectionId = g_token->sectionId.mid(1, g_token->sectionId.length()-2).trimmed();
      return RetVal_OK;
   }

<St_PlantUMLOpt>{BLANK}*{FILEMASK}{BLANK}+/{ID}"=" {
      // case 2: plain file name specified followed by an attribute
      g_token->sectionId = QString::fromUtf8(yytext).trimmed();
      return RetVal_OK;
   }

<St_PlantUMLOpt>{BLANK}*{FILEMASK}{BLANK}+/"\"" {
      // case 3: plain file name specified followed by a quoted title
      g_token->sectionId = QString::fromUtf8(yytext).trimmed();
      return RetVal_OK;
   }

<St_PlantUMLOpt>{BLANK}*{FILEMASK}{BLANK}*/\n {
      // case 4: plain file name specified without title or attributes
      g_token->sectionId = QString::fromUtf8(yytext).trimmed();
      return RetVal_OK;
   }

<St_PlantUMLOpt>{BLANK}*{FILEMASK}{BLANK}*/"\\internal_linebr" {
      // case 5: plain file name specified without title or attributes
      g_token->sectionId = QString::fromUtf8(yytext).trimmed();

      return RetVal_OK;
   }

<St_PlantUMLOpt>"\\internal_linebr" |
<St_PlantUMLOpt>"\n"                |
<St_PlantUMLOpt>.              {
      for (int i = yyleng - 1; i >= 0; --i) {
         unput(yytext[i]);
      }

      g_token->sectionId = QString();

      return RetVal_OK;
   }

<St_PlantUML>{CMD}"enduml"     {
      return RetVal_OK;
   }

<St_PlantUML>[^\\@\n]+ |
<St_PlantUML>\n        |
<St_PlantUML>.                 {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->verb += text;
   }

<St_Title>"\""         {
      // quoted title
      BEGIN(St_TitleQ);
   }

<St_Title>[ \t]+       {
      g_token->chars = QString::fromUtf8(yytext);
      return TK_WHITESPACE;
   }

<St_Title>.           {
      // non-quoted title
      unput(yytext[0]);
      BEGIN(St_TitleN);
   }

<St_Title>\n          {
      unput(yytext[0]);
      return 0;
   }

<St_Title>"\\internal_linebr"    {
      for (int i = yyleng - 1; i >= 0; --i) {
         unput(yytext[i]);
      }

      return 0;
   }

<St_TitleN>"&"{ID}";"  {
      /* symbol */
      g_token->name = QString::fromUtf8(yytext);
      return TK_SYMBOL;
   }

<St_TitleN>{HTMLTAG}                   {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');
   }

<St_TitleN>\n         {
      /* new line => end of title */
      unput(yytext[0]);
      return 0;
   }

<St_TitleN>"\\internal_linebr"    {
      /* new line => end of title */
      for (int i = yyleng - 1; i >= 0; --i) {
         unput(yytext[i]);
      }

      return 0;
   }

<St_TitleN>{SPCMD1}    |
<St_TitleN>{SPCMD2}        {
      /* special command */
      QString text      = QString::fromUtf8(yytext);
      g_token->name     = text.mid(1);
      g_token->paramDir = TokenInfo::Unspecified;
      return TK_COMMAND_SEL();
   }

<St_TitleN>{ID}"="        {
      /* attribute */
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '%')  {
         // strip % if present
         g_token->name = text.mid(1);
      } else {
         g_token->name = text;
      }

      return TK_WORD;
   }

<St_TitleN>[\-+0-9]       |
<St_TitleN>{WORD1}        |
<St_TitleN>{WORD2}     {
      /* word */
      QString text = QString::fromUtf8(yytext);

      if (text.contains("\\internal_linebr")) {
         REJECT;
      }

      s_yyLineNum += text.count('\n');

      if (text.startsWith('%') ) {
         // strip % if present
         g_token->name = text.mid(1);

      } else {
         g_token->name = text;

      }

      return TK_WORD;
   }

<St_TitleN>[ \t]+      {
      g_token->chars = QString::fromUtf8(yytext);
         return TK_WHITESPACE;
   }

<St_TitleQ>"&"{ID}";"  {
      /* symbol */
      g_token->name = QString::fromUtf8(yytext);
         return TK_SYMBOL;
   }


<St_TitleQ>(\n|"\\internal_linebr")   {
      /* new line => end of title */
      for (int i = yyleng - 1; i >= 0; --i) {
         unput(yytext[i]);
      }

      return 0;
   }

<St_TitleQ>{SPCMD1}    |
<St_TitleQ>{SPCMD2}                    {
      /* special command */
      QString text = QString::fromUtf8(yytext);

      g_token->name     = text.mid(1);
      g_token->paramDir = TokenInfo::Unspecified;
      return TK_COMMAND_SEL();
   }

<St_TitleQ>{WORD1NQ}   |
<St_TitleQ>{WORD2NQ}                   {
      /* word */
      g_token->name = QString::fromUtf8(yytext);
      return TK_WORD;
   }

<St_TitleQ>[ \t]+      {
      g_token->chars = QString::fromUtf8(yytext);
      return TK_WHITESPACE;
   }

<St_TitleQ>"\""          {
      /* closing quote => end of title */
      BEGIN(St_TitleA);
      return 0;
   }

<St_TitleA>{BLANK}*{ID}{BLANK}*"="{BLANK}* {
      // title attribute
      g_token->name = QString::fromUtf8(yytext);

      int i = g_token->name.indexOf('=');
      g_token->name = g_token->name.left(i).trimmed();

      BEGIN(St_TitleV);
   }

<St_TitleV>[^ \t\r\n]+ {
      // attribute value
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->chars = text;
      BEGIN(St_TitleN);

      return TK_WORD;
   }

<St_TitleV,St_TitleA>. {
      unput(yytext[0]);
      return 0;
   }

<St_TitleV,St_TitleA>(\n|"\\internal_linebr")    {
      for (int i = yyleng - 1; i >= 0; --i) {
         unput(yytext[i]);
      }

      return 0;
   }

<St_Anchor>{LABELID}{WS}? {
      // anchor
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->name = text.trimmed();
      return TK_WORD;
   }

<St_Anchor>.          {
      unput(yytext[0]);
      return 0;
   }

<St_Cite>{CITEID}      {
      // label to cite
      QString text = QString::fromUtf8(yytext);


      if (text.startsWith('"')) {
         g_token->name = text.mid(1);
         g_token->name.chop(1);
      } else {
         g_token->name = text;
      }

      return TK_WORD;
   }

<St_Cite>{BLANK}       {
      // white space
      unput(' ');
      return 0;
   }

<St_Cite>(\n|"\\internal_linebr")   {
      // new line
      for (int i = yyleng - 1; i >= 0; --i) {
         unput(yytext[i]);
      }

      return 0;
   }

<St_Cite>.            {
      // any other character
      unput(yytext[0]);
      return 0;
   }

<St_Ref>{REFWORD_NOCV}/{BLANK}("const")[a-z_A-Z0-9] {
      g_token->name = QString::fromUtf8(yytext);
      return TK_WORD;
   }

<St_Ref>{REFWORD_NOCV}/{BLANK}("volatile")[a-z_A-Z0-9] {
      g_token->name = QString::fromUtf8(yytext);
      return TK_WORD;
   }

<St_Ref>{REFWORD}      {
      // label to refer to
      g_token->name = QString::fromUtf8(yytext);
      return TK_WORD;
   }

<St_Ref>{BLANK}        {
      // white space
      unput(' ');
      return 0;
   }

<St_Ref>{WS}+"\""{WS}* {
      // white space following by quoted string
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      BEGIN(St_Ref2);
   }

<St_Ref>(\n|"\\internal_linebr")         {
      // new line
      for (int i = yyleng - 1; i >= 0; --i) {
         unput(yytext[i]);
      }

      return 0;
   }

<St_Ref>.          {
      // any other character
      unput(yytext[0]);
      return 0;
   }

<St_IntRef>[A-Z_a-z0-9.:/#\-\+\(\)]+ {
      g_token->name = QString::fromUtf8(yytext);
      return TK_WORD;
   }

<St_IntRef>{BLANK}+"\"" {
      BEGIN(St_Ref2);
   }

<St_SetScope>({SCOPEMASK}|{ANONNS}){BLANK}|{FILEMASK} {
      g_token->name = QString::fromUtf8(yytext).trimmed();
      return TK_WORD;
   }

<St_SetScope>{SCOPEMASK}"<" {
      g_token->name = QString::fromUtf8(yytext).trimmed();
      s_sharpCount  = 1;
      BEGIN(St_SetScopeEnd);
   }

<St_SetScope>{BLANK}   {
   }

<St_SetScopeEnd>"<"    {
      g_token->name += QString::fromUtf8(yytext);
      ++s_sharpCount;
   }

<St_SetScopeEnd>">"    {
      g_token->name += QString::fromUtf8(yytext);
      --s_sharpCount;

      if (s_sharpCount <= 0) {
         return TK_WORD;
      }
   }

<St_SetScopeEnd>.      {
      g_token->name += QString::fromUtf8(yytext);
   }

<St_Ref2>"&"{ID}";"    {
      /* symbol */
      g_token->name = QString::fromUtf8(yytext);
      return TK_SYMBOL;
   }

<St_Ref2>"\""         {
      /* closing quote */
      return 0;
   }

<St_Ref2>"\n"|"\\internal_linebr"         {
      QString text = QString::fromUtf8(yytext);

      s_yyLineNum   += text.count('\n');
      g_token->name = " ";

      return TK_WORD;
   }

<St_Ref2>{SPCMD1}      |
<St_Ref2>{SPCMD2}                      {
      /* special command */
      QString text      = QString::fromUtf8(yytext);

      g_token->name     = text.mid(1);
      g_token->paramDir = TokenInfo::Unspecified;

      return TK_COMMAND_SEL();
   }

<St_Ref2>{WORD1NQ}     |
<St_Ref2>{WORD2NQ}     {
      /* word */
      g_token->name = QString::fromUtf8(yytext);
      return TK_WORD;
   }

<St_Ref2>[ \t]+        {
      g_token->chars = QString::fromUtf8(yytext);
      return TK_WHITESPACE;
   }

<St_XRefItem>{LABELID} {
      g_token->name = QString::fromUtf8(yytext);
   }

<St_XRefItem>" "       {
      BEGIN(St_XRefItem2);
   }

<St_XRefItem2>[0-9]+"." {
      QString text = QString::fromUtf8(yytext);

      text = text.left(yyleng - 1);
      g_token->id = text.toInteger<int>();
      return RetVal_OK;
   }

<St_Para,St_Title,St_Ref2>"<!--"     {
      /* html style comment block */
      s_commentState = YY_START;
      BEGIN(St_Comment);
   }

<St_Param>"\""[^\n\"]+"\"" {
      QString text  = QString::fromUtf8(yytext);
      g_token->name = QStringView(text.constBegin() + 1, text.constEnd() - 1);
      return TK_WORD;
   }

<St_Param>({PHPTYPE}{BLANK}*("["{BLANK}*"]")*{BLANK}*"|"{BLANK}*)*{PHPTYPE}{BLANK}*("["{BLANK}*"]")*{WS}+("&")?"$"{LABELID} {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      int j = text.indexOf('&');
      int i = text.indexOf('$');

      if (j < i && j != -1) {
         i = j;
      }

      QString types = text.left(i).trimmed();
      g_token->name = types + "#" + text.mid(i);

      return TK_WORD;
   }

<St_Param>[^ \t\n,@\\]+  {
      QString text  = QString::fromUtf8(yytext);
      g_token->name = text;

      if (g_token->name.at(yyleng-1) == ':') {
         g_token->name = g_token->name.left(yyleng-1);
      }

      return TK_WORD;
   }

<St_Param>{WS}*","{WS}*
<St_Param>{WS}           {
      // param separator
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->chars = text;

      return TK_WHITESPACE;
   }

<St_Options>{ID}       {
      g_token->name += QString::fromUtf8(yytext);
   }

<St_Options>{WS}*":"{WS}* {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->name += ":";
   }

<St_Options>{WS}*","{WS}*
<St_Options>{WS}       {
      // option separator
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->name += ",";
   }

<St_Options>"}"        {
      return TK_WORD;
   }

<St_Block>{ID}         {
      g_token->name += QString::fromUtf8(yytext);
   }

<St_Block>"]"          {
      return TK_WORD;
   }

<St_Emoji>[:0-9_a-z+-]+  {
      g_token->name = QString::fromUtf8(yytext);
      return TK_WORD;
   }

<St_Emoji>.    {
      return 0;
   }

<St_File>{FILEMASK}    {
      g_token->name = QString::fromUtf8(yytext);
      return TK_WORD;
   }

<St_File>"\""[^\n\"]+"\"" {
      QString text  = QString::fromUtf8(yytext);
      g_token->name = QStringView(text.constBegin() + 1, text.constEnd() - 1);
      return TK_WORD;
   }

<St_Pattern>[^\\\r\n]+    {
      g_token->name += QString::fromUtf8(yytext);
   }

<St_Pattern>"\\internal_linebr"   {
      g_token->name = g_token->name.trimmed();
      return TK_WORD;
   }

<St_Pattern>\n            {
      ++s_yyLineNum;
      g_token->name = g_token->name.trimmed();

      return TK_WORD;
   }

<St_Pattern>.             {
      g_token->name += QString::fromUtf8(yytext);
   }

<St_Link>{LINKMASK}|{REFWORD}    {
      g_token->name = QString::fromUtf8(yytext);
      return TK_WORD;
   }

<St_Comment>"-->"      {
      /* end of html comment */
      BEGIN(s_commentState);
   }

<St_Comment>[^-]+    {
      /* inside html comment */
   }

<St_Comment>.        {
      /* inside html comment */
   }

     /* State for skipping title (all chars until the end of the line) */

<St_SkipTitle>.      {
      // nothing
   }

<St_SkipTitle>(\n|"\\internal_linebr")     {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      return 0;
   }

     /* State for the pass used to find the anchors and sections */

<St_Sections>[^\n@\\<]+   {
      // nothing
   }

<St_Sections>{CMD}("<"|{CMD})  {
      // nothing
   }

<St_Sections>"<"{CAPTION}({WS}+{ATTRIB})*">" {
      QString tag = QString::fromUtf8(yytext);
      s_yyLineNum += tag.count('\n');

      int s = tag.indexOf("id=");

      if (s != -1)  {
         // command has id attribute
         QChar c = tag[s + 3];

         if (c == '\'' || c == '"')  {
            // valid start

            int e = tag.indexOf(c, s + 4);

            if (e != -1) {
               // found matching end

               s_secType  = SectionInfo::Table;
               s_secLabel = tag.mid(s + 4, e - s - 4);   // extract id

               processSection();
            }
         }
      }
    }

<St_Sections>{CMD}"anchor"{BLANK}+  {
      s_secType = SectionInfo::Anchor;
      BEGIN(St_SecLabel1);
   }

<St_Sections>{CMD}"section"{BLANK}+ {
      s_secType = SectionInfo::Section;
      BEGIN(St_SecLabel2);
   }

<St_Sections>{CMD}"subsection"{BLANK}+ {
      s_secType = SectionInfo::Subsection;
      BEGIN(St_SecLabel2);
   }

<St_Sections>{CMD}"subsubsection"{BLANK}+ {
      s_secType = SectionInfo::Subsubsection;
      BEGIN(St_SecLabel2);
   }

<St_Sections>{CMD}"paragraph"{BLANK}+ {
      s_secType = SectionInfo::Paragraph;
      BEGIN(St_SecLabel2);
   }

<St_Sections>{CMD}"verbatim"/[^a-z_A-Z0-9]  {
      s_endMarker="endverbatim";
      BEGIN(St_SecSkip);
   }

<St_Sections>{CMD}"dot"/[^a-z_A-Z0-9] {
      s_endMarker="enddot";
      BEGIN(St_SecSkip);
   }

<St_Sections>{CMD}"msc"/[^a-z_A-Z0-9] {
      s_endMarker="endmsc";
      BEGIN(St_SecSkip);
   }

<St_Sections>{CMD}"startuml"/[^a-z_A-Z0-9] {
      s_endMarker="enduml";
      BEGIN(St_SecSkip);
   }

<St_Sections>{CMD}"htmlonly"/[^a-z_A-Z0-9] {
      s_endMarker="endhtmlonly";
      BEGIN(St_SecSkip);
   }

<St_Sections>{CMD}"latexonly"/[^a-z_A-Z0-9] {
      s_endMarker="endlatexonly";
      BEGIN(St_SecSkip);
   }

<St_Sections>{CMD}"xmlonly"/[^a-z_A-Z0-9] {
      s_endMarker = "endxmlonly";
      BEGIN(St_SecSkip);
   }

<St_Sections>{CMD}"docbookonly"/[^a-z_A-Z0-9] {
      s_endMarker = "enddocbookonly";
      BEGIN(St_SecSkip);
   }

<St_Sections>{CMD}"code"/[^a-z_A-Z0-9] {
      s_endMarker = "endcode";
      BEGIN(St_SecSkip);
   }

<St_Sections>"<!--"                 {
      s_endMarker = "-->";
      BEGIN(St_SecSkip);
   }

<St_SecSkip>{CMD}{ID}          {
      QString text = QString::fromUtf8(yytext);

      if (text.mid(1) == s_endMarker) {
         BEGIN(St_Sections);
      }
   }

<St_SecSkip>"-->"           {
      QString text = QString::fromUtf8(yytext);

      if (text == s_endMarker) {
         BEGIN(St_Sections);
      }
   }

<St_SecSkip>[^a-z_A-Z0-9\-\\\@]+    {
      // nothing
   }

<St_SecSkip>.   {
      // nothing
   }

<St_SecSkip>(\n|"\\internal_linebr") {
      // nothing
   }

<St_Sections>.      {
      // nothing
   }

<St_Sections>(\n|"\\internal_linebr")   {
      // nothing
   }

<St_SecLabel1>{LABELID} {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      s_secLabel = text;
      processSection();

      BEGIN(St_Sections);
   }

<St_SecLabel2>{LABELID}{BLANK}+ |
<St_SecLabel2>{LABELID}         {
      s_secLabel = QString::fromUtf8(yytext).trimmed();
      BEGIN(St_SecTitle);
   }

<St_SecTitle>[^\n]+    |
<St_SecTitle>[^\n]*\n        {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      s_secTitle = text.trimmed();

      if (s_secTitle.endsWith("\\internal_linebr"))  {
         s_secTitle.chop(16);
      }

      processSection();
      BEGIN(St_Sections);
   }

<St_SecTitle,St_SecLabel1,St_SecLabel2>. {
      QString text = QString::fromUtf8(yytext);
      warn(s_fileName, s_yyLineNum, "Unexpected character `%s' while looking for section label or title", csPrintable(text));
   }

<St_Snippet>[^\\\n]+     |
<St_Snippet>"\\"   {
      g_token->name += QString::fromUtf8(yytext);

   }
<St_Snippet>(\n|"\\internal_linebr")   {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      g_token->name = g_token->name.trimmed();

      return TK_WORD;
   }

     /* Generic rules that work for all states */
<*>\n                  {
      QString text = QString::fromUtf8(yytext);
      s_yyLineNum += text.count('\n');

      warn(s_fileName, s_yyLineNum, "Unexpected new line character");
   }

<*>"\\internal_linebr"         {
   }

<*>[\\@<>&$#%~"=]      {
      // */ (editor syntax fix)
      /* unescaped special character */

      g_token->name = QString::fromUtf8(yytext);
      return TK_COMMAND_SEL();
   }

<*>[\xC0-\xFF][\x80-\xBF]+    {
      // utf-8 code point
      QString text = QString::fromUtf8(yytext);
      warn(s_fileName, yylineno,"Unexpected character '%s'", text.toUtf8().constData());
   }

<*>.                   {
      QString text = QString::fromUtf8(yytext);
      warn(s_fileName, s_yyLineNum,"Unexpected character '%s'", csPrintable(text));
   }

%%


static void yyunput(QChar c, char *)
{
   QString tmp1    = c;
   QByteArray tmp2 = tmp1.toUtf8();

   for (int i = tmp2.length() -1; i >= 0; i-- ) {
      unput(tmp2[i]);
   }
}

void doctokenizerYYFindSections(const QString &input, QSharedPointer<Definition> def,
               QSharedPointer<MemberGroup> mg, const QString &fileName)
{
   if (input.isEmpty()) {
      return;
   }

   printlex(yy_flex_debug, true, __FILE__, fileName);

   s_inputString   = input;
   s_inputPosition = 0;
   s_definition    = def;
   s_memberGroup   = mg;
   s_fileName      = fileName;

   BEGIN(St_Sections);

   s_yyLineNum = 1;
   doctokenizerYYlex();

   printlex(yy_flex_debug, false, __FILE__, fileName);
}

void doctokenizerYYinit(const QString &input, const QString &fileName)
{
   s_autoListLevel = 0;
   s_inputString   = input;
   s_inputPosition = 0;
   s_fileName      = fileName;
   s_insidePre     = false;

   BEGIN(St_Para);
}

void doctokenizerYYsetStatePara()
{
   BEGIN(St_Para);
}

void doctokenizerYYsetStateTitle()
{
   BEGIN(St_Title);
}

void doctokenizerYYsetStateTitleAttrValue()
{
   BEGIN(St_TitleV);
}

void doctokenizerYYsetStateCode()
{
   g_token->verb = "";
   g_token->name = "";
   BEGIN(St_CodeOpt);
}

void doctokenizerYYsetStateXmlCode()
{
   g_token->verb = "";
   g_token->name = "";
   BEGIN(St_XmlCode);
}

void doctokenizerYYsetStateHtmlOnly()
{
   g_token->verb = "";
   g_token->name = "";
   BEGIN(St_HtmlOnlyOption);
}

void doctokenizerYYsetStateManOnly()
{
   g_token->verb = "";
   BEGIN(St_ManOnly);
}

void doctokenizerYYsetStateRtfOnly()
{
   g_token->verb = "";
   BEGIN(St_RtfOnly);
}

void doctokenizerYYsetStateXmlOnly()
{
   g_token->verb = "";
   BEGIN(St_XmlOnly);
}

void doctokenizerYYsetStateDbOnly()
{
   g_token->verb = "";
   BEGIN(St_DbOnly);
}

void doctokenizerYYsetStateLatexOnly()
{
   g_token->verb = "";
   BEGIN(St_LatexOnly);
}

void doctokenizerYYsetStateVerbatim()
{
   g_token->verb = "";
   BEGIN(St_Verbatim);
}

void doctokenizerYYsetStateDot()
{
   g_token->verb = "";
   BEGIN(St_Dot);
}

void doctokenizerYYsetStateMsc()
{
   g_token->verb = "";
   BEGIN(St_Msc);
}

void doctokenizerYYsetStatePlantUMLOpt()
 {
   g_token->verb      = "";
   g_token->sectionId = "";
   BEGIN(St_PlantUMLOpt);
 }

void doctokenizerYYsetStatePlantUML()
{
   g_token->verb = "";
   BEGIN(St_PlantUML);
}

void doctokenizerYYsetStateParam()
{
   BEGIN(St_Param);
}

void doctokenizerYYsetStateXRefItem()
{
   BEGIN(St_XRefItem);
}

void doctokenizerYYsetStateFile()
{
   BEGIN(St_File);
}

void doctokenizerYYsetStatePattern()
{
   BEGIN(St_Pattern);
}

void doctokenizerYYsetStateLink()
{
   BEGIN(St_Link);
}

void doctokenizerYYsetStateCite()
{
   BEGIN(St_Cite);
}

void doctokenizerYYsetStateRef()
{
   BEGIN(St_Ref);
}

void doctokenizerYYsetStateInternalRef()
{
   BEGIN(St_IntRef);
}

void doctokenizerYYsetStateText()
{
   BEGIN(St_Text);
}

void doctokenizerYYsetStateSkipTitle()
{
   BEGIN(St_SkipTitle);
}

void doctokenizerYYsetStateAnchor()
{
   BEGIN(St_Anchor);
}

void doctokenizerYYsetStateSnippet()
{
   g_token->name = "";
   BEGIN(St_Snippet);
}

void doctokenizerYYsetStateSetScope()
{
   BEGIN(St_SetScope);
}

void doctokenizerYYsetStateOptions()
{
  g_token->name = "";
  BEGIN(St_Options);
}

void doctokenizerYYsetStateBlock()
{
  g_token->name = "";
  BEGIN(St_Block);
}
void doctokenizerYYsetStateEmoji()
{
  g_token->name = "";
  BEGIN(St_Emoji);
}
void doctokenizerYYcleanup()
{
   yy_delete_buffer( YY_CURRENT_BUFFER );
}

void doctokenizerYYsetInsidePre(bool b)
{
   s_insidePre = b;
}

void doctokenizerYYpushBackHtmlTag(const QString &tag)
{
   QString tagName = tag;

   int i;
   int l = tagName.length();

   unput('>');

   for (i = l - 1; i >= 0; i--) {
      unput(tag[i]);
   }

   unput('<');
}

void doctokenizerYYstartAutoList()
{
   ++s_autoListLevel;
}

void doctokenizerYYendAutoList()
{
   --s_autoListLevel;
}

void setDoctokenLineNum(int lineNum)
{
   s_yyLineNum = lineNum;
}

int getDoctokenLineNum()
{
   return s_yyLineNum;
}
