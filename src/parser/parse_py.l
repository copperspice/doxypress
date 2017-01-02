/*************************************************************************
 *
 * Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim
 * Copyright (C) 1997-2014 by Dimitri van Heesch.

*************************************************************************/

%{

#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegExp>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>

#include <arguments.h>
#include <commentscan.h>
#include <config.h>
#include <doxy_globals.h>
#include <default_args.h>
#include <entry.h>
#include <language.h>
#include <message.h>
#include <parse_py.h>
#include <util.h>

#define DBG_CTX(...)     do { } while(0)

#define YY_NO_INPUT 1
#define YY_NEVER_INTERACTIVE 1

static ParserInterface *s_thisParser;
static QString          s_inputString;
static int		         s_inputPosition;
static QFile            s_inputFile;

static Protection	protection;

static QSharedPointer<Entry> current_root;
static QSharedPointer<Entry> current;
static QSharedPointer<Entry> previous;
static QSharedPointer<Entry> bodyEntry;

static int		         yyLineNr = 1;
static QString 		   yyFileName;
static MethodTypes    	mtype;
static bool       		gstat;
static Specifier       	virt;

static int              docBlockContext;
static QString          docBlock;
static bool             docBlockInBody;
static bool             isJavaDocStyle;
static bool             docBrief;
static bool             docBlockSpecial;

static bool             g_doubleQuote;
static bool             g_specialBlock;
static int              g_stringContext;
static QString         *g_copyString;
static int              g_indent    = 0;
static int              g_curIndent = 0;

static QHash<QString, QString *>  g_packageNameCache;

static char             g_atomStart;
static char             g_atomEnd;
static int              g_atomCount;

static QString          g_moduleScope;
static QString          g_packageName;
static QString          g_defVal;

static int              g_braceCount;

static bool             g_lexInit = false;
static bool             g_packageCommentAllowed;

static bool             g_start_init = false;
static int              g_search_count = 0;

static void initParser()
{
   protection = Public;
   mtype = Method;
   gstat = false;
   virt  = Normal;

   previous = QSharedPointer<Entry>();
   g_packageCommentAllowed = true;
}

static void initEntry()
{
   current->protection = protection ;
   current->mtype      = mtype;
   current->virt       = virt;
   current->stat       = gstat;
   current->lang       = SrcLangExt_Python;

   current->setParent(current_root);

   initGroupInfo(current);
   gstat = false;
}

static void newEntry()
{
   previous = current;
   current_root->addSubEntry(current, current_root);

   current = QMakeShared<Entry>();
   initEntry();
}

static void newVariable()
{
   if (!current->name.isEmpty() && current->name.at(0) == '_') {
      // mark as private
      current->protection = Private;
   }

   if (current_root->section & Entry::COMPOUND_MASK) {
      // mark as class variable
      current->stat = true;
   }

   newEntry();
}

static void newFunction()
{
   if (current->name.left(2) == "__" && current->name.right(2) == "__") {
      // special method name
      // refere to http://docs.python.org/ref/specialnames.html
      current->protection = Public;

   } else if (current->name.at(0) == '_') {
      current->protection = Private;

   }
}

static inline int computeIndent(const QString &str)
{
   static int tabSize = Config::getInt("tab-size");
   int col = 0;

   for (auto c : str)  {

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

static QString findPackageScopeFromPath(const QString &path)
{
   QString *pScope = g_packageNameCache.value(path);

   if (pScope) {
      return *pScope;
   }

   QFileInfo pf(path + "/__init__.py"); // found package initialization file

   if (pf.exists()) {
      int i = path.lastIndexOf('/');

      if (i != -1) {
         QString scope = findPackageScopeFromPath(path.left(i));

         if (! scope.isEmpty()) {
            scope += "::";
         }

         scope += path.mid(i + 1);
         g_packageNameCache.insert(path, new QString(scope));
         return scope;
      }
   }

   return "";
}

static QString findPackageScope(const QString &fileName)
{
   if (! fileName.isEmpty()) {
      return "";
   }

   QFileInfo fi(fileName);

   return findPackageScopeFromPath(fi.absolutePath());
}

static void lineCount()
{
   DBG_CTX((stderr, "yyLineNr = %d\n", yyLineNr));

   for (const char *p = yytext; *p; ++p) {
      yyLineNr += (*p == '\n') ;
   }
}

static void incLineNr()
{
   DBG_CTX((stderr, "yyLineNr = %d\n", yyLineNr));
   yyLineNr++;
}

static void startCommentBlock(bool brief)
{
   if (brief) {
      current->briefFile = yyFileName;
      current->briefLine = yyLineNr;
   } else {
      current->docFile = yyFileName;
      current->docLine = yyLineNr;
   }
}

static void handleCommentBlock(const QString &doc, bool isBrief)
{
   // TODO: Fix this
   docBlockInBody = false;

   if (docBlockInBody && previous && ! previous->doc.isEmpty()) {
      previous->doc = previous->doc.trimmed() + "\n\n";
   }

   bool needsEntry;
   int position = 0;
   int lineNr   = isBrief ? current->briefLine : current->docLine;

   while (parseCommentBlock(s_thisParser, (docBlockInBody && previous) ? previous : current,
             doc, yyFileName,
             lineNr,                                      // passed by reference
             docBlockInBody ? false : isBrief, isJavaDocStyle, docBlockInBody,
             protection, position, needsEntry)) {         // last 3 are passed by reference

      if (needsEntry) {
         newEntry();
      }
   }

   if (needsEntry) {
      newEntry();
   }

}

static void endOfDef(int correction = 0)
{
   if (bodyEntry) {
      bodyEntry->endBodyLine  = yyLineNr - correction;
      bodyEntry = QSharedPointer<Entry>();
   }

   newEntry();
}

static inline void addToString(const QString &s)
{
   if (g_copyString) {
      (*g_copyString) += s;
   }
}

static void initTriDoubleQuoteBlock()
{
   docBlockContext = YY_START;
   docBlockInBody  = false;
   isJavaDocStyle  = false;

   QString text = QString::fromUtf8(yytext);
   docBlockSpecial = text.endsWith('!');

   docBlock.resize(0);
   g_doubleQuote = true;
   startCommentBlock(false);
}

static void initTriSingleQuoteBlock()
{
   docBlockContext = YY_START;
   docBlockInBody  = false;
   isJavaDocStyle  = false;

   QString text = QString::fromUtf8(yytext);
   docBlockSpecial = text.endsWith('!');

   docBlock.resize(0);
   g_doubleQuote = false;
   startCommentBlock(false);
}

static void initSpecialBlock()
{
   docBlockContext  = YY_START;
   docBlockInBody   = false;
   isJavaDocStyle   = true;
   docBrief         = true;

   docBlock.resize(0);
   startCommentBlock(true);
}

static void searchFoundDef()
{
   current->fileName  = yyFileName;
   current->startLine = yyLineNr;
   current->bodyLine  = yyLineNr;
   current->section   = Entry::FUNCTION_SEC;

   current->lang = SrcLangExt_Python;
   current->virt = Normal;
   current->stat = gstat;
   current->mtype = mtype = Method;
   current->type.resize(0);
   current->name.resize(0);
   current->args.resize(0);

   current->argList.clear();

   g_packageCommentAllowed = false;
   gstat = false;
}

static void searchFoundClass()
{
   current->section = Entry::CLASS_SEC;
   current->argList.clear();
   current->type += "class" ;

   current->fileName  = yyFileName;
   current->startLine  = yyLineNr;
   current->bodyLine  = yyLineNr;
   g_packageCommentAllowed = false;
}

#undef	YY_INPUT
#define	YY_INPUT(buf,result,max_size) result = yyread(buf,max_size);

static int yyread(char *buf, int max_size)
{
   int c = 0;

   while (s_inputString[s_inputPosition] != 0) {

      QString tmp1    = s_inputString.at(s_inputPosition);
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

      s_inputPosition++;
   }

   return c;
}

%}

   /* start command character */

BB                [ \t]+
B                 [ \t]*
NEWLINE           \n
BN                [ \t\n]

DIGIT             [0-9]

HEXNUMBER         "0"[xX][0-9a-fA-F]+[lL]?
OCTNUMBER         "0"[0-7]+[lL]?
NUMBER            {DIGIT}+[lLjJ]?
INTNUMBER         {HEXNUMBER}|{OCTNUMBER}|{NUMBER}
FLOATNUMBER       {DIGIT}+"."{DIGIT}+([eE][+\-]?{DIGIT}+)?[jJ]?
BOOL              ("True"|"False")
LETTER            [A-Za-z\x80-\xFF]
NONEMPTY          [A-Za-z0-9_\x80-\xFF]
EXPCHAR           [#(){}\[\],:.%/\\=`*~|&<>!;+-]
NONEMPTYEXP       [^ \t\n:]
PARAMNONEMPTY     [^ \t\n():]
IDENTIFIER        ({LETTER}|"_")({LETTER}|{DIGIT}|"_")*
SCOPE             {IDENTIFIER}("."{IDENTIFIER})*
BORDER            ([^A-Za-z0-9])

TRISINGLEQUOTE    {STRINGPREFIX}?"'''"(!)?
TRIDOUBLEQUOTE    {STRINGPREFIX}?"\"\"\""(!)?
ENDTRISINGLEQUOTE "'''"
ENDTRIDOUBLEQUOTE "\"\"\""
LONGSTRINGCHAR    [^\\"']
ESCAPESEQ         ("\\")(.)
LONGSTRINGITEM    ({LONGSTRINGCHAR}|{ESCAPESEQ})
SMALLQUOTE        ("\"\""|"\""|"'"|"''")
LONGSTRINGBLOCK   ({LONGSTRINGITEM}+|{SMALLQUOTE})

SHORTSTRING       ("'"{SHORTSTRINGITEM}*"'"|'"'{SHORTSTRINGITEM}*'"')
SHORTSTRINGITEM   ({SHORTSTRINGCHAR}|{ESCAPESEQ})
SHORTSTRINGCHAR   [^\\\n"]
STRINGLITERAL     {STRINGPREFIX}?( {SHORTSTRING} | {LONGSTRING})
STRINGPREFIX      ("r"|"u"|"ur"|"R"|"U"|"UR"|"Ur"|"uR")
KEYWORD           ("lambda"|"import"|"class"|"assert"|"as"|"from"|"global"|"def"|"True"|"False")
FLOWKW            ("or"|"and"|"is"|"not"|"print"|"for"|"in"|"if"|"try"|"except"|"yield"|"raise"|"break"|"continue"|"pass"|"if"|"return"|"while"|"elif"|"else"|"finally")
POUNDCOMMENT      "#"[^#\n][^\n]*
SCRIPTCOMMENT     "#!".*

STARTDOCSYMS      "##"

%option noyywrap

  /* Main start state */

%x Search
%x SearchMemVars

  /* Mid-comment states */

  /* %x FuncDoubleComment */
  /* %x ClassDoubleComment */
%x TryClassDocString
%x TripleComment
%x SpecialComment

  /* Function states */

%x FunctionDec
%x FunctionParams
%x FunctionBody
%x FunctionParamDefVal

  /* Class states */

%x ClassDec
%x ClassInheritance
%x ClassCaptureIndent
%x ClassBody

  /* Variable states */
%x VariableDec
%x VariableEnd
%x VariableAtom

  /* String states */

%x SingleQuoteString
%x DoubleQuoteString
%x TripleString

  /* import */
%x FromMod
%x FromModItem
%x Import

%%

<Search>{

   ^{B}"def"{BB}       {
      // start of a function/method definition with indent
      DBG_CTX((stderr, "Found def at %d\n", yyLineNr));

      QString text = QString::fromUtf8(yytext);

      g_indent = computeIndent(text);
      searchFoundDef();
      BEGIN( FunctionDec );
   }

   "def"{BB}           {
      // start of a function/method definition
      searchFoundDef();
      BEGIN( FunctionDec );
   }

   ^{B}"class"{BB}    {
      // start of a class definition with indent
      DBG_CTX((stderr,"Found class at %d\n",yyLineNr));

      QString text = QString::fromUtf8(yytext);

      g_indent = computeIndent(text);
      searchFoundClass();
      BEGIN( ClassDec ) ;
   }

   "class"{BB}        {
      // start of a class definition
     searchFoundClass();
     BEGIN( ClassDec ) ;
   }

   ^{B}"from"{BB}    |
      "from"{BB}          {

      // start of an from import
      g_packageCommentAllowed = false;
      BEGIN( FromMod );
   }

   ^{B}"import"{BB}  |
       "import"{BB}       {

      // start of an import statement
      g_packageCommentAllowed = false;
      BEGIN( Import );
   }

   ^{B}{IDENTIFIER}/{B}"="{B}"property" {
      // property
      QString text = QString::fromUtf8(yytext);

      current->section   = Entry::VARIABLE_SEC;
      current->mtype     = Property;
      current->name      = text.trimmed();
      current->fileName  = yyFileName;
      current->startLine = yyLineNr;
      current->bodyLine  = yyLineNr;
      g_packageCommentAllowed = false;

      BEGIN(VariableDec);
   }

   ^{B}{IDENTIFIER}/{B}"="[^=] {
      // variable
      if (g_search_count) {
         REJECT;
      }

      QString text = QString::fromUtf8(yytext);

      g_indent = computeIndent(yytext);
      current->section   = Entry::VARIABLE_SEC;
      current->name      = text.trimmed();
      current->fileName  = yyFileName;
      current->startLine = yyLineNr;
      current->bodyLine  = yyLineNr;
      g_packageCommentAllowed = false;

      BEGIN(VariableDec);
   }

   {B}{IDENTIFIER}/({B},{B}{IDENTIFIER})*{B}")"*{B}"="[^=] {
      // list of variables, can not place the default value so we will skip it later on in a general rule
      // Also note ")" this is to catch also (a,b). the "("
      // is caught in the rule: [(], the ")" will be handled in [)]

      if (g_search_count > 1) {
         REJECT;
      }

      QString text = QString::fromUtf8(yytext);
      g_indent = computeIndent(text);

      current->section   = Entry::VARIABLE_SEC;
      current->name      = text.trimmed();
      current->fileName  = yyFileName;
      current->startLine = yyLineNr;
      current->bodyLine  = yyLineNr;

      g_packageCommentAllowed = false;
      newVariable();
   }

   "'"          {
      // start of a single quoted string
      g_stringContext = YY_START;
      g_copyString    = 0;
      g_packageCommentAllowed = false;

      BEGIN( SingleQuoteString );
   }

   "\""         {
      // start of a double quoted string
      g_stringContext = YY_START;
      g_copyString    = 0;
      g_packageCommentAllowed = false;
      BEGIN( DoubleQuoteString );
   }

   "@staticmethod"  {
      gstat = true;
   }

   {SCRIPTCOMMENT}   {
      // Unix type script comment
      if (yyLineNr != 1)   {
         REJECT;
      }
   }

   {POUNDCOMMENT}    {
      // normal comment
      g_packageCommentAllowed = false;
   }

   {IDENTIFIER}      {
      // some other identifier
      g_packageCommentAllowed = false;
   }

   ^{BB}         {
      QString text = QString::fromUtf8(yytext);
      g_curIndent  = computeIndent(text);
   }

   {NEWLINE}+        {
      // new line
      lineCount();
   }

   {TRIDOUBLEQUOTE}  {
      // start of a comment block
      initTriDoubleQuoteBlock();
      BEGIN(TripleComment);
   }

   {TRISINGLEQUOTE}  {
      // start of a comment block
      initTriSingleQuoteBlock();
      BEGIN(TripleComment);
   }

   {STARTDOCSYMS}/[^#]    {
      // start of a special comment
      QString text = QString::fromUtf8(yytext);
      g_curIndent  = computeIndent(text);

      g_packageCommentAllowed = false;
      initSpecialBlock();
      BEGIN(SpecialComment);
   }

   [(]     {
      // we have to do something with (
      g_search_count += 1;
   }

   [)]     {
      // we have to do something with )
      g_search_count -= 1;
   }

   [^\n]    {
      // any other character...this is the major default
      // that should catch everything else in Body
   }
}

<FromMod>{
   "."            {
      // python3 style imports
   }

   {IDENTIFIER}({B}"."{B}{IDENTIFIER})* {
      // from package import
      QString text = QString::fromUtf8(yytext);
      g_packageName = text;
   }

   "import"{B}    {
      BEGIN(FromModItem);
   }

   \n             {
      incLineNr();
      BEGIN(Search);
   }

   {B}             {
   }

   .               {
      unput(*yytext);
      BEGIN(Search);
   }
}

<FromModItem>{
   "*"       {
      // import all
      QString item      = g_packageName;
      current->name     = removeRedundantWhiteSpace(substitute(item,".","::"));
      current->fileName = yyFileName;
      current->section  = Entry::USINGDIR_SEC;
      current_root->addSubEntry(current, current_root);

      current = QMakeShared<Entry>();
      initEntry();
      BEGIN(Search);
   }

   {IDENTIFIER}/{B}","{B} {
      QString text = QString::fromUtf8(yytext);
      QString item = g_packageName+"." + text;

      current->name = removeRedundantWhiteSpace(substitute(item,".","::"));
      current->fileName = yyFileName;

      current->section  = Entry::USINGDECL_SEC;
      current_root->addSubEntry(current, current_root);

      current = QMakeShared<Entry>();
      initEntry();
   }

  {IDENTIFIER}  {
      QString text  = QString::fromUtf8(yytext);
      QString item  = g_packageName+"." + text;

      current->name = removeRedundantWhiteSpace(substitute(item,".","::"));
      current->fileName = yyFileName;

      current->section  = Entry::USINGDECL_SEC;
      current_root->addSubEntry(current, current_root);

      current = QMakeShared<Entry>();
      initEntry();
      BEGIN(Search);
   }

   \n     {
      incLineNr();
      BEGIN(Search);
   }

   {B}       {
   }

   ","       {
   }

   .         {
      unput(*yytext);
      BEGIN(Search);
   }
}

<Import>{
   {IDENTIFIER}({B}"."{B}{IDENTIFIER})* {
      QString text  = QString::fromUtf8(yytext);
      current->name = removeRedundantWhiteSpace(substitute(text,".","::"));

      current->fileName = yyFileName;
      current->section  = Entry::USINGDECL_SEC;
      current_root->addSubEntry(current, current_root);

      current = QMakeShared<Entry>();
      initEntry();
      BEGIN(Search);
   }

   \n        {
      incLineNr();
      BEGIN(Search);
   }

   {B}       {
   }

   .         {
      unput(*yytext);
      BEGIN(Search);
   }
}

<SearchMemVars>{
   "self."{IDENTIFIER}/{B}"=" {
         QString text = QString::fromUtf8(yytext);

         DBG_CTX((stderr,"Found instance method variable %s in %s at %d\n", csPrintable(text.mid(5)), csPrintable(current_root->name), yyLineNr));


         current->name      = text.mid(5);
         current->section   = Entry::VARIABLE_SEC;
         current->fileName  = yyFileName;
         current->startLine = yyLineNr;
         current->bodyLine  = yyLineNr;
         current->type.resize(0);

         if (current->name.at(0) == '_') {
            // mark as private
            current->protection = Private;
         }

         newEntry();
   }

   "cls."{IDENTIFIER}/{B}"=" {
      QString text = QString::fromUtf8(yytext);

      DBG_CTX(stderr, "Found class method variable %s in %s at%d\n",
                  csPrintable(text.mid(4)), csPrintable(current_root->name), yyLineNr);

      current->name      = text.mid(4);
      current->section   = Entry::VARIABLE_SEC;
      current->fileName  = yyFileName;
      current->startLine = yyLineNr;
      current->bodyLine  = yyLineNr;
      current->type.resize(0);

      if (current->name.at(0)=='_') {
         // mark as private
         current->protection = Private;
      }

   newEntry();
                      }
   {TRIDOUBLEQUOTE}  {
      // start of a comment block
      initTriDoubleQuoteBlock();
      BEGIN(TripleComment);
   }

   {TRISINGLEQUOTE}  {
      // start of a comment block
      initTriSingleQuoteBlock();
      BEGIN(TripleComment);
   }

   {STARTDOCSYMS}/[^#]    {
      // start of a special comment
      initSpecialBlock();
      BEGIN(SpecialComment);
   }

   {POUNDCOMMENT}    {
      // #
   }

   "'"                {
      // start of a single quoted string
      g_stringContext = YY_START;
      g_copyString    = 0;
      BEGIN( SingleQuoteString );
   }

   "\""              {
      // start of a double quoted string
      g_stringContext = YY_START;
      g_copyString    = 0;
      BEGIN( DoubleQuoteString );
   }

   \n                  {
      incLineNr();
   }

   {IDENTIFIER}        {
      // identifiers
   }

   [^'"\.#a-z_A-Z\n]+  {
      // other uninteresting stuff
   }

   .                   {
      // anything else
   }
}

<FunctionBody>{
   \n{B}/{IDENTIFIER}{BB}  {
      DBG_CTX((stderr, "indent %d <= %d\n", computeIndent(text.mid(1)), g_indent));

      QString text = QString::fromUtf8(yytext);

      if (computeIndent(text.mid(1)) <= g_indent)  {

         for (int i = yyleng - 1; i >= 0; i--) {
            unput(yytext[i]);
         }

         endOfDef();
         BEGIN(Search);

      } else {
         incLineNr();
         current->m_program += text;
      }
   }

   \n{B}/"##"          {
      QString text = QString::fromUtf8(yytext);

      if (computeIndent(text.mid(1)) <= g_indent)  {

         for (int i = yyleng - 1; i >= 0; i--) {
           unput(yytext[i]);
         }

         endOfDef();
         BEGIN(Search);

      } else {
         incLineNr();
         current->m_program += text;
      }
   }

   <<EOF>>          {
         endOfDef();
         yyterminate();
   }

   ^{BB}/\n         {
      // skip empty line
      QString text = QString::fromUtf8(yytext);
      current->m_program += text;
   }

   ^{BB}         {
      // something at indent >0
      QString text = QString::fromUtf8(yytext);
      current->m_program += text;

      g_curIndent = computeIndent(text);

      if (g_curIndent <= g_indent) {
         // jumped out of the function
         endOfDef(1);
         BEGIN(Search);
      }
   }

   "'"                {
      // start of a single quoted string
      QString text = QString::fromUtf8(yytext);
      current->m_program += text;

      g_stringContext    = YY_START;
      g_specialBlock     = false;

      g_copyString = &current->m_program;
      BEGIN( SingleQuoteString );
   }

   "\""              {
      // start of a double quoted string
      QString text = QString::fromUtf8(yytext);
      current->m_program += text;

      g_stringContext = YY_START;
      g_specialBlock  = false;

      g_copyString = &current->m_program;
      BEGIN( DoubleQuoteString );
   }

   [^ \t\n#'".]+     {
      // non-special stuff
      QString text = QString::fromUtf8(yytext);
      current->m_program += text;
      g_specialBlock = false;
   }

    ^{POUNDCOMMENT}   {
      // normal comment
      QString text = QString::fromUtf8(yytext);
      current->m_program += text;
   }

   "#".*             {
      // comment half way
      QString text = QString::fromUtf8(yytext);
      current->m_program += text;
   }

   {NEWLINE}        {
      QString text = QString::fromUtf8(yytext);
      incLineNr();
      current->m_program += text;
   }

   .                 {
      // any character
      QString text = QString::fromUtf8(yytext);
      current->m_program += text[0];
      g_specialBlock = false;
   }

   {TRIDOUBLEQUOTE}  {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);
      current->m_program += text;
      initTriDoubleQuoteBlock();
      BEGIN(TripleComment);
   }

   {TRISINGLEQUOTE}  {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);
      current->m_program += text;
      initTriSingleQuoteBlock();
      BEGIN(TripleComment);
   }

   {STARTDOCSYMS}/[^#]  {
      // start of a special comment
      initSpecialBlock();
      BEGIN(SpecialComment);
   }
}

<FunctionDec>{

   {IDENTIFIER}            {
      //found function name
      QString text = QString::fromUtf8(yytext);

      if (current->type.isEmpty())  {
         current->type = "def";
      }

      current->name = text;
      current->name = current->name.trimmed();
      newFunction();
   }

   {B}":"         {
      // function without arguments
      g_specialBlock = true; // expecting a docstring
      bodyEntry = current;
      current->bodyLine  = yyLineNr;
      BEGIN( FunctionBody );
   }

   {B}"("                  {
      BEGIN( FunctionParams );
   }
}

<FunctionParams>{
   ({BB}|",")          {
   }

   {IDENTIFIER}        {
      // Name of parameter
      QString text = QString::fromUtf8(yytext);
      lineCount();

      current->argList.append( Argument() );
      current->argList.last().name = text.trimmed();
      current->argList.last().type = "";
   }

   "="             {
      // default value

      // TODO: this rule is too simple, need to be able to
      // match things like =")" as well

      g_defVal.resize(0);
      g_braceCount = 0;
      BEGIN(FunctionParamDefVal);
   }

   ")"                {
      // end of parameter list
      current->args = argListToString(current->argList);
   }

   ":"{B}             {
      g_specialBlock    = true; // expecting a docstring
      bodyEntry         = current;
      current->bodyLine = yyLineNr;
      BEGIN( FunctionBody );
   }

   {POUNDCOMMENT}   {
      // a comment
   }

   {PARAMNONEMPTY}     {
      // default rule inside arguments
   }

}

<FunctionParamDefVal>{
   "("       {
      // internal opening brace
      QString text = QString::fromUtf8(yytext);
      g_braceCount++;
      g_defVal += text[0];
   }

   ","       |
   ")"             {
      QString text = QString::fromUtf8(yytext);

      if (g_braceCount == 0)   {
         // end of default argument

         if (current->argList.isEmpty()) {
            current->argList.last().defval = g_defVal.trimmed();
         }

          if (text[0] == ')') {
             current->args = argListToString(current->argList);
          }

          BEGIN(FunctionParams);

      }  else  {
         // continue

         if (text[0] == ')') {
            g_braceCount--;
         }

         g_defVal += text[0];
      }
   }

   .         {
      QString text = QString::fromUtf8(yytext);
      g_defVal += text[0];
   }

   \n                 {
      QString text = QString::fromUtf8(yytext);
      g_defVal += text[0];
      incLineNr();
   }
}


<ClassBody>{
   \n/{IDENTIFIER}{BB}  {
      // new def at indent 0
      incLineNr();
      endOfDef();
      BEGIN(Search);
   }

   \n/"##"[^#]       {
      // start of a special comment at indent 0
      incLineNr();
      endOfDef();
      BEGIN(Search);
   }

   ^{BB}/\n         {
      // skip empty line
      QString text = QString::fromUtf8(yytext);
      current->m_program += text;
   }

   <<EOF>>          {
         endOfDef();
         yyterminate();
   }

   ^{BB}         {
      // something at indent >0
      QString text = QString::fromUtf8(yytext);
      g_curIndent  = computeIndent(text);

      DBG_CTX((stderr,"g_curIndent = %d g_indent = %d\n", g_curIndent, g_indent));

      if (g_curIndent<=g_indent)  {
         // jumped out of the class/method

         endOfDef(1);
         g_indent = g_curIndent;
         BEGIN(Search);

      } else {
         current->m_program += text;
         }
                  }
   "'"                {
      // start of a single quoted string
      QString text = QString::fromUtf8(yytext);
      current->m_program += text[0];

      g_stringContext = YY_START;
      g_specialBlock  = false;
      g_copyString    = &current->m_program;

      BEGIN( SingleQuoteString );
   }

   "\""              {
      // start of a double quoted string
      QString text = QString::fromUtf8(yytext);
      current->m_program += text[0];

      g_stringContext = YY_START;
      g_specialBlock  = false;
      g_copyString    = &current->m_program;
      BEGIN( DoubleQuoteString );
   }

   [^ \t\n#'"]+      {
      // non-special stuff
      QString text = QString::fromUtf8(yytext);
      current->m_program += text;
      g_specialBlock = false;
   }

   {NEWLINE}        {
      QString text = QString::fromUtf8(yytext);
      current->m_program += text[0];
      incLineNr();
   }

   {POUNDCOMMENT}    {
      // normal comment
      QString text = QString::fromUtf8(yytext);
      current->m_program += text;
   }

   .                 {
      // any character
      QString text = QString::fromUtf8(yytext);
      g_specialBlock = false;
      current->m_program += text[0];
   }

   {TRIDOUBLEQUOTE}  {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);
      current->m_program += text;

      initTriDoubleQuoteBlock();
      BEGIN(TripleComment);
   }

    {TRISINGLEQUOTE}  {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);
      current->m_program += text;

      initTriSingleQuoteBlock();
      BEGIN(TripleComment);
   }
}

<ClassDec>{IDENTIFIER} {
      QString text = QString::fromUtf8(yytext);

      if (current->type.isEmpty())  {
         current->type = "class";
      }

      current->section = Entry::CLASS_SEC;
      current->name = text;

      // prepend scope in case of nested classes
      if (current_root->section&Entry::SCOPE_MASK) {
         current->name.prepend(current_root->name+"::");
      }

      current->name     = current->name.trimmed();
      current->fileName = yyFileName;
      docBlockContext   = YY_START;
      docBlockInBody    = false;
      isJavaDocStyle    = false;

      docBlock.resize(0);
      BEGIN(ClassInheritance);
   }

<ClassInheritance>{
   ({BB}|[\(,\)])      {
      // syntactic sugar for the list
   }

   ":"                {
      // begin of the class definition
      g_specialBlock = true; // expecting a docstring
      current->bodyLine  = yyLineNr;
      current->m_program.resize(0);
      BEGIN(ClassCaptureIndent);
   }

   {SCOPE}            {
      QString text = QString::fromUtf8(yytext);
      current->extends.append(BaseInfo(substitute(text,".","::"), Public, Normal));
      //Has base class-do stuff
   }
}

<ClassCaptureIndent>{
   "\n"|({BB}"\n")            {
      // Blankline - ignore, keep looking for indentation.
      QString text = QString::fromUtf8(yytext);
      lineCount();
      current->m_program += text;
   }

   {TRIDOUBLEQUOTE}           {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);

      initTriDoubleQuoteBlock();
      current->m_program += text;
      BEGIN(TripleComment);
   }

   {TRISINGLEQUOTE}           {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);

      initTriSingleQuoteBlock();
      current->m_program += text;
      BEGIN(TripleComment);
   }

   ^{BB}             {
      QString text = QString::fromUtf8(yytext);
      current->m_program += text;

      g_curIndent = computeIndent(text);
      bodyEntry   = current;

      DBG_CTX((stderr,"setting indent %d\n",g_curIndent));

      BEGIN(ClassBody);
   }

   ""/({NONEMPTY}|{EXPCHAR})  {
      // Just pushback an empty class, and resume parsing the body
      QString text = QString::fromUtf8(yytext);
      newEntry();
      current->m_program += text;

      BEGIN( Search );
   }
}

<VariableDec>{
   "="                {
      // the assignment operator
      QString text = QString::fromUtf8(yytext);
      g_start_init = true;
      current->initializer = text;
      current->initializer += " ";
   }

   {B}                {
      // spaces
      QString text = QString::fromUtf8(yytext);
      current->initializer += text;
   }

   {INTNUMBER}          {
      // integer value
      QString text  = QString::fromUtf8(yytext);

      if (current->type.isEmpty()) {
         current->type = "int";
      }

      current->initializer += text;      
   }

   {FLOATNUMBER}      {
      // floating point value
      QString text  = QString::fromUtf8(yytext);

      if (current->type.isEmpty()) {
         current->type = "float";
      }

      current->initializer += text;
   }

   {BOOL}             {
      // boolean value
      QString text  = QString::fromUtf8(yytext);

      if (current->type.isEmpty()) {
         current->type = "bool";
      }

      current->initializer += text;      
   }

   {STRINGPREFIX}?"'" {
      // string
      QString text  = QString::fromUtf8(yytext);

      if (current->type.isEmpty()) {
         current->type = "string";
      }

      current->initializer += text;
      g_copyString = &current->initializer;
      g_stringContext = VariableDec;
      BEGIN( SingleQuoteString );
   }

   {STRINGPREFIX}?"\"" {
      // string
      QString text  = QString::fromUtf8(yytext);

      if (current->type.isEmpty()) {
         current->type = "string";
      }

      current->initializer += text;
      g_copyString    = &current->initializer;
      g_stringContext = VariableDec;
      BEGIN( DoubleQuoteString );
   }

   {TRIDOUBLEQUOTE}   {
      // start of a comment block
      QString text  = QString::fromUtf8(yytext);

      if (current->type.isEmpty()) {
         current->type = "string";
      }

      current->initializer += text;
      g_doubleQuote   = true;
      g_copyString    = &current->initializer;
      g_stringContext = VariableDec;
      BEGIN(TripleString);
   }

   {TRISINGLEQUOTE}   {
      // start of a comment block
      QString text  = QString::fromUtf8(yytext);
      
      if (current->type.isEmpty()) {
         current->type = "string";
      }

      current->initializer += text;
      g_doubleQuote   = false;
      g_copyString    = &current->initializer;
      g_stringContext = VariableDec;
      BEGIN(TripleString);
   }

   "("            {
      // tuple, only when direct after =
      QString text = QString::fromUtf8(yytext);

      if (current->mtype != Property && g_start_init) {
         current->type = "tuple";
      }

      current->initializer += text[0];
      g_atomStart = '(';
      g_atomEnd   = ')';
      g_atomCount = 1;
      BEGIN( VariableAtom );
   }

   "["                {
      // list
      QString text = QString::fromUtf8(yytext);
      if (g_start_init) {
         current->type = "list";
      }

      current->initializer += text[0];
      g_atomStart = '[';
      g_atomEnd   = ']';
      g_atomCount = 1;
      BEGIN( VariableAtom );
   }

   "{"            {
      // dictionary
      QString text = QString::fromUtf8(yytext);
      if (g_start_init) current->type = "dictionary";

      current->initializer += text[0];
      g_atomStart = '{';
      g_atomEnd   = '}';
      g_atomCount = 1;
      BEGIN( VariableAtom );
   }

   "#".*              {
      // comment
      BEGIN( VariableEnd );
   }

   {IDENTIFIER}         {
      QString text = QString::fromUtf8(yytext);

      // do something based on the type of the IDENTIFIER
      if (current->type.isEmpty()) {
       
         for (auto child : current_root->children() )  {

            if (child->name == text) {
               current->type = child->type;
               break;
            }
         }
      }

      g_start_init = false;
      current->initializer += text;
   }

   .           {
      QString text = QString::fromUtf8(yytext);
      g_start_init = false;
      current->initializer += text[0];
   }

   \n             {
      unput('\n');
      BEGIN( VariableEnd );
   }
}

<VariableAtom>{
   [\(\[\{]         {
      QString text = QString::fromUtf8(yytext);
      current->initializer += text[0];

      if (g_atomStart == text[0]) {
         g_atomCount++;
      }
   }

   [\)\]\}]         {
      QString text = QString::fromUtf8(yytext);
      current->initializer += text[0];

      if (g_atomEnd == text[0]) {
         g_atomCount--;
      }

      if (g_atomCount == 0) {
      g_start_init = false;
      BEGIN(VariableDec);
      }
   }

   {TRIDOUBLEQUOTE}  {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);
      g_specialBlock = false;
      current->m_program += text;
      initTriDoubleQuoteBlock();
      BEGIN(TripleComment);
   }

   {TRISINGLEQUOTE}  {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);
                        g_specialBlock = false;
         current->m_program += text;
         initTriSingleQuoteBlock();
         BEGIN(TripleComment);
   }

   "'"                 {
      g_stringContext = YY_START;
      current->initializer += "'";
      g_copyString = &current->initializer;
      BEGIN( SingleQuoteString );
   }

   "\""                {
      g_stringContext=YY_START;
      current->initializer += "\"";
      g_copyString = &current->initializer;
      BEGIN( DoubleQuoteString );
   }

   {IDENTIFIER}       {
      QString text = QString::fromUtf8(yytext);
      current->initializer += text;
   }
   .           {
      QString text = QString::fromUtf8(yytext);
      current->initializer += text[0];
   }

   \n             {
      QString text = QString::fromUtf8(yytext);
      current->initializer += text[0];
      incLineNr();
   }
}

<VariableEnd>{
   \n                {
      incLineNr();
      newVariable();
      BEGIN(Search);
   }

   .             {
      unput(*yytext);
      newVariable();
      BEGIN(Search);
   }

   <<EOF>>           {
      yyterminate();
      newEntry();
   }
}

<TripleComment>{
   {ENDTRIDOUBLEQUOTE}   |
   {ENDTRISINGLEQUOTE}   {
      QString text = QString::fromUtf8(yytext);

      if (g_doubleQuote == (text[0] == '"'))  {

         if (g_specialBlock) {
            // expecting a docstring
            QString actualDoc = docBlock;

            if (! docBlockSpecial) {
               // legacy unformatted docstring

               actualDoc.prepend("\\verbatim ");
               actualDoc.append("\\endverbatim ");
            }

            handleCommentBlock(actualDoc, false);

         } else if (g_packageCommentAllowed) {
            // expecting module docs
            QString actualDoc = docBlock;

            if (! docBlockSpecial) {
               // legacy unformatted docstring

               actualDoc.prepend("\\verbatim ");
               actualDoc.append("\\endverbatim ");
            }

            actualDoc.prepend("\\namespace " + g_moduleScope + "\\_linebr ");
            handleCommentBlock(actualDoc, false);
         }

         if ((docBlockContext == ClassBody) || docBlockContext == FunctionBody) {

            current->m_program += docBlock;
            current->m_program += text;
         }

         BEGIN(docBlockContext);

      } else  {
         docBlock += text;

      }

      g_packageCommentAllowed = false;
   }

   ^{BB}      {
      // leading whitespace
      QString text = QString::fromUtf8(yytext);
      int indent   = computeIndent(text);

      if (indent >= g_curIndent) {
         // strip g_curIndent amount of whitespace

         for (int i= 0; i<indent - g_curIndent; i++) {
            docBlock+=' ';
         }

         DBG_CTX((stderr,"stripping indent %d\n",g_curIndent));

      } else {
         DBG_CTX((stderr,"not stripping: %d<%d\n",indent,g_curIndent));
         docBlock += text;

      }
   }

   [^"'\n \t\\]+       {
      QString text = QString::fromUtf8(yytext);
      docBlock += text;
   }

   \n         {
      QString text = QString::fromUtf8(yytext);
      incLineNr();
      docBlock += text;
   }

   \\.             {
      // escaped char
      QString text = QString::fromUtf8(yytext);
      docBlock += text;
   }

   .          {
      QString text = QString::fromUtf8(yytext);
      docBlock += text;
   }
}

<SpecialComment>{
   ^{B}"#"("#")*    {
      // skip leading hashes
   }

   \n/{B}"#"     {
      // continuation of the comment on the next line
      docBlock  += '\n';
      docBrief  =  false;
      startCommentBlock(false);
      incLineNr();
   }

   [^#\n]+             {
      // any other stuff
      QString text = QString::fromUtf8(yytext);
      docBlock += text;
   }

   \n         {
      // new line that ends the comment
      handleCommentBlock(docBlock, docBrief);
      incLineNr();
      BEGIN(docBlockContext);
   }

   .          {
      // anything we missed
      QString text = QString::fromUtf8(yytext);
      docBlock += text[0];
   }
}

<SingleQuoteString>{
   \\{B}\n                    {
      // line continuation
      QString text = QString::fromUtf8(yytext);
      addToString(text);
      incLineNr();
   }

   \\.               {
      // espaced char
      QString text = QString::fromUtf8(yytext);
      addToString(text);
   }

   "\"\"\""             {
      // tripple double quotes
      QString text = QString::fromUtf8(yytext);
      addToString(text);
   }

   "'"               {
      // end of the string
      QString text = QString::fromUtf8(yytext);
      addToString(yytext);
      BEGIN(g_stringContext);
   }

   [^"'\n\\]+                   {
      // normal chars
      QString text = QString::fromUtf8(yytext);
      addToString(text);
   }

   .                 {
      // normal char
      QString text = QString::fromUtf8(yytext);
      addToString(text);
   }
}

<DoubleQuoteString>{
   \\{B}\n                    {
      // line continuation
      QString text = QString::fromUtf8(yytext);
      addToString(text);
      incLineNr();
   }

   \\.               {
      // escaped char
      QString text = QString::fromUtf8(yytext);
      addToString(text);
                               }
    "'''"             {
      // tripple single quotes
      QString text = QString::fromUtf8(yytext);
      addToString(text);
   }

    "\""              {
      // end of the string
      QString text = QString::fromUtf8(yytext);
      addToString(text);
      BEGIN(g_stringContext);
   }

   [^"'\n\\]+                   {
      // normal chars
      QString text = QString::fromUtf8(yytext);
      addToString(text);
   }

   .                 {
      // normal char
      QString text = QString::fromUtf8(yytext);
      addToString(text);
   }
}

<TripleString>{
   {ENDTRIDOUBLEQUOTE}    |
   {ENDTRISINGLEQUOTE}    {
      QString text = QString::fromUtf8(yytext);
      *g_copyString += text;

      if (g_doubleQuote == (text[0]=='"'))  {
         BEGIN(g_stringContext);
      }
   }

   ({LONGSTRINGBLOCK}) {
      QString text = QString::fromUtf8(yytext);
      lineCount();
      *g_copyString += text;
   }

   \n         {
      QString text = QString::fromUtf8(yytext);
      incLineNr();
      *g_copyString += text[0];
   }

   .          {
      QString text = QString::fromUtf8(yytext);
      *g_copyString += text[0];
   }
}

  /* ------------ End rules -------------- */


<*>{NEWLINE}                   {
      lineCount();
   }

<*>.                           {
   }


%%

static void parseCompounds(QSharedPointer<Entry> rt)
{
   for (auto ce : rt->children() ) {
      if (! ce->m_program.isEmpty()) {
         // init scanner state
         s_inputString   = ce->m_program;
         s_inputPosition = 0;

         yyrestart( yyin );

         if (ce->section & Entry::COMPOUND_MASK) {
            current_root = ce;
            BEGIN( Search );

         } else if (ce->parent()) {
            current_root = ce->parent();
            BEGIN( SearchMemVars );
         }

         yyFileName = ce->fileName;
         yyLineNr   = ce->bodyLine ;

         current = QMakeShared<Entry>();
         initEntry();

         groupEnterCompound(yyFileName, yyLineNr, ce->name);

         yylex() ;
         g_lexInit = true;

         current = QSharedPointer<Entry>();
         ce->m_program.resize(0);

         groupLeaveCompound(yyFileName, yyLineNr, ce->name);

      }
      parseCompounds(ce);
   }
}

static void parseMain(const QString &fileName, const QString &fileBuf, QSharedPointer<Entry> rt)
{
   initParser();

   s_inputString   = fileBuf;
   s_inputPosition = 0;

   protection      = Public;
   mtype           = Method;
   gstat           = false;
   virt            = Normal;

   current_root    = rt;
   g_specialBlock  = false;

   s_inputFile.setFileName(fileName);

   if (s_inputFile.open(QIODevice::ReadOnly)) {
      yyLineNr   = 1;
      yyFileName = fileName;

      //setContext();
      msg("Parsing file %s\n", csPrintable(yyFileName));

      QFileInfo fi(fileName);
      g_moduleScope = findPackageScope(fileName);

      QString baseName = fi.baseName();

      if (baseName != "__init__") { // package initializer file is not a package itself
         if (!g_moduleScope.isEmpty()) {
            g_moduleScope += "::";
         }
         g_moduleScope += baseName;
      }

      current            = QMakeShared<Entry>();
      initEntry();

      current->name      = g_moduleScope;
      current->section   = Entry::NAMESPACE_SEC;
      current->type      = "namespace";
      current->fileName  = yyFileName;
      current->startLine = yyLineNr;
      current->bodyLine  = yyLineNr;

      rt->addSubEntry(current, rt);

      current_root  = current;
      initParser();

      current = QMakeShared<Entry>();

      groupEnterFile(yyFileName, yyLineNr);

      current->reset();
      initEntry();
      yyrestart( yyin );

      BEGIN( Search );
      yylex();
      g_lexInit = true;

      groupLeaveFile(yyFileName, yyLineNr);

      current_root->m_program.resize(0);

      current = QSharedPointer<Entry>();
      parseCompounds(current_root);

      s_inputFile.close();
   }
}

static void parsePrototype(const QString &text)
{
   if (text.isEmpty()) {
      warn(yyFileName, yyLineNr, "Empty prototype found");
      return;
   }

   g_specialBlock = false;
   g_packageCommentAllowed = false;

   QString orgInputString;
   int orgInputPosition;
   YY_BUFFER_STATE orgState;

   // save scanner state
   orgState = YY_CURRENT_BUFFER;
   yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
   orgInputString   = s_inputString;
   orgInputPosition = s_inputPosition;

   // set new string
   s_inputString   = text;
   s_inputPosition = 0;

   yyrestart( yyin );

   BEGIN( FunctionDec );

   yylex();
   g_lexInit = true;

   current->name = current->name.trimmed();
   if (current->section == Entry::MEMBERDOC_SEC && current->args.isEmpty()) {
      current->section = Entry::VARIABLEDOC_SEC;
   }

   // restore original scanner state

   YY_BUFFER_STATE tmpBuf = YY_CURRENT_BUFFER;
   yy_switch_to_buffer(orgState);
   yy_delete_buffer(tmpBuf);

   s_inputString   = orgInputString;
   s_inputPosition = orgInputPosition;
}

void pyFreeParser()
{
   if (g_lexInit) {
      yylex_destroy();
   }
}

void PythonLanguageParser::parseInput(const QString &fileName, const QString &fileBuf,
                  QSharedPointer<Entry> root, enum ParserMode mode, QStringList &includedFiles, bool useClang)
{
   s_thisParser = this;

   printlex(yy_flex_debug, true, __FILE__, fileName);
   ::parseMain(fileName, fileBuf, root);
   printlex(yy_flex_debug, false, __FILE__, fileName);
}

bool PythonLanguageParser::needsPreprocessing(const QString &)
{
  return false;
}

void PythonLanguageParser::parseCode(CodeOutputInterface &codeOutIntf, const QString &scopeName,
                  const QString &input, SrcLangExt, bool isExampleBlock,
                  const QString &exampleName, QSharedPointer<FileDef> fileDef, int startLine,
                  int endLine, bool inlineFragment, QSharedPointer<MemberDef> memberDef,
                  bool showLineNumbers, QSharedPointer<Definition> searchCtx, bool collectXRefs )
{
   ::parsePythonCode(codeOutIntf, scopeName, input, isExampleBlock, exampleName,
                  fileDef, startLine, endLine, inlineFragment, memberDef,
                  showLineNumbers, searchCtx, collectXRefs);
}

void PythonLanguageParser::parsePrototype(const QString &text)
{
   ::parsePrototype(text);
}

void PythonLanguageParser::resetCodeParserState()
{
   ::resetPythonCodeParserState();
}


