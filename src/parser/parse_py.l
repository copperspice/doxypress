/*************************************************************************
 *
 * Copyright (c) 2014-2023 Barbara Geller & Ansel Sermersheim
 * Copyright (c) 1997-2014 Dimitri van Heesch

*************************************************************************/

%{

#include <QFile>
#include <QFileInfo>
#include <QHash>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

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

static QSharedPointer<Entry> current_root;
static QSharedPointer<Entry> current;
static QSharedPointer<Entry> previous;
static QSharedPointer<Entry> bodyEntry;

static QSharedPointer<Entry>  s_docsEntry;    // which entry
static EntryKey               s_docsEnum;     // which enum in EntryKey (Source_Text, Initial_Value)

static bool s_defVal_active = false;

static ParserInterface  *s_thisParser;
static QString           s_inputString;
static int               s_inputPosition;
static QFile             s_inputFile;

static int               yyLineNr = 1;
static QString           yyFileName;

static Protection        protection;
static MethodType        mtype;
static Specifier         virt;

static bool              s_static;

static int               docBlockContext;
static QString           docBlock;
static bool              docBlockInBody;
static bool              isJavaDocStyle;
static bool              docBrief;
static bool              docBlockSpecial;

static bool              s_doubleQuote;
static bool              s_specialBlock;
static int               s_stringContext;
static int               s_indent    = 0;
static int               s_curIndent = 0;

static char              s_atomStart;
static char              s_atomEnd;
static int               s_atomCount;

static QString           s_moduleScope;
static QString           s_packageName;
static QString           s_defVal;

static int               s_braceCount;

static bool              s_lexInit = false;
static bool              s_packageCommentAllowed;

static bool              s_funcParamsEnd = false;
static bool              s_start_init    = false;
static int               s_search_count  = 0;
static QString           s_argType       = QString();

static void initParser()
{
   protection = Public;
   mtype    = MethodType::Method;
   s_static = false;
   virt     = Normal;

   previous = QSharedPointer<Entry>();
   s_packageCommentAllowed = true;
}

static void initEntry()
{
   current->protection = protection;
   current->mtype      = mtype;
   current->virt       = virt;
   current->m_static   = s_static;
   current->m_srcLang  = SrcLangExt_Python;

   current->setParent(current_root);

   initGroupInfo(current);
   s_static = false;
}

static void newEntry()
{
   previous = current;
   current_root->addSubEntry(current);

   current = QMakeShared<Entry>();
   initEntry();
}

static void newVariable()
{
   if (! current->m_entryName.isEmpty() && current->m_entryName.at(0) == '_') {

      if (current->m_entryName.size() < 2 || current->m_entryName.at(1) == '_')  {
         // mark as private
         current->protection = Private;

      } else {
        // mark as protected
        current->protection = Protected;

      }
   }

   if (current_root->section & Entry::COMPOUND_MASK) {
      // mark as class variable
      current->m_static = true;
   }

   newEntry();
}

static void newFunction()
{
   if (current->m_entryName.startsWith("__") && current->m_entryName.endsWith("__")) {

      // special method name, refer to https://docs.python.org/ref/specialnames.html
      current->protection = Public;

   } else if (current->m_entryName.at(0) == '_') {

      if (current->m_entryName.size() < 2 || current->m_entryName.at(1) == '_')  {
         // mark as private
         current->protection = Private;

      } else {
        // mark as protected
        current->protection = Protected;

      }
   }
}

static inline int computeIndent(const QString &str)
{
   static const int tabSize = Config::getInt("tab-size");

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
   static QHash<QString, QString> s_packageNameCache;

   QString pScope = s_packageNameCache.value(path);

   if (! pScope.isEmpty()) {
      return pScope;
   }

   // found package initialization file
   QFileInfo pf(path + "/__init__.py");

   if (pf.exists()) {
      int pos = path.lastIndexOf('/');

      if (pos != -1) {
         QString scope = findPackageScopeFromPath(path.left(pos));

         if (! scope.isEmpty()) {
            scope += "::";
         }

         scope += path.mid(pos + 1);
         s_packageNameCache.insert(path, scope);
         return scope;
      }
   }

   return QString();
}

static QString findPackageScope(const QString &fileName)
{
   if (! fileName.isEmpty()) {
      return QString();
   }

   QFileInfo fi(fileName);

   return findPackageScopeFromPath(fi.absolutePath());
}

static void lineCount()
{
   // commentscan, parse_py

   for (const char *p = yytext; *p; ++p) {
      yyLineNr += (*p == '\n');
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
      current->setData(EntryKey::Brief_File, yyFileName);
      current->briefLine = yyLineNr;

   } else {
      current->setData(EntryKey::MainDocs_File, yyFileName);
      current->docLine = yyLineNr;

   }
}

static void handleCommentBlock(const QString &doc, bool isBrief)
{
   docBlockInBody = false;

   QString currentDocs = current->getData(EntryKey::Main_Docs);

   if (! currentDocs.isEmpty()) {
      current->setData(EntryKey::Main_Docs, currentDocs.trimmed() + "\n\n");
   }

   if (docBlockInBody && previous) {
      QString prevDocs = previous->getData(EntryKey::Main_Docs);

      if (! prevDocs.isEmpty()) {
         previous->setData(EntryKey::Main_Docs, prevDocs.trimmed() + "\n\n");
      }
   }

   bool needsEntry = false;
   int position    = 0;
   int lineNr      = isBrief ? current->briefLine : current->docLine;

   while (parseCommentBlock(s_thisParser, (docBlockInBody && previous) ? previous : current,
             doc, yyFileName, lineNr, docBlockInBody ? false : isBrief, isJavaDocStyle, docBlockInBody,
             protection, position, needsEntry)) {

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

static void addToOutput(const QString &str)
{
   if (s_defVal_active) {
      s_defVal += str;

   } else if (s_docsEntry != nullptr) {
      s_docsEntry->appendData(s_docsEnum, str);
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
   s_doubleQuote = true;
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
   s_doubleQuote = false;
   startCommentBlock(false);
}

static void initSpecialBlock()
{
   docBlockContext  = YY_START;
   docBlockInBody   = false;
   isJavaDocStyle   = true;
   docBrief         = true;

   docBlock.resize(0);
   startCommentBlock(false);
}

static void searchFoundDef()
{
   current->setData(EntryKey::File_Name, yyFileName);
   current->startLine     = yyLineNr;
   current->startBodyLine = yyLineNr;

   current->section   = Entry::FUNCTION_SEC;
   current->m_srcLang = SrcLangExt_Python;
   current->virt      = Normal;
   current->m_static  = s_static;
   current->mtype     = (mtype = MethodType::Method);

   current->m_entryName.resize(0);
   current->setData(EntryKey::Member_Type, "");
   current->setData(EntryKey::Member_Args, "");
   current->argList.clear();

   s_packageCommentAllowed = false;
   s_static = false;
}

static void searchFoundClass()
{
   current->section = Entry::CLASS_SEC;
   current->argList.clear();

   current->appendData(EntryKey::Member_Type, "class");

   current->setData(EntryKey::File_Name, yyFileName);
   current->startLine     = yyLineNr;
   current->startBodyLine = yyLineNr;

   s_packageCommentAllowed = false;
}

#undef   YY_INPUT
#define  YY_INPUT(buf,result,max_size) result = yyread(buf,max_size);

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

FLOWKW            ("or"|"and"|"is"|"not"|"print"|"for"|"in"|"try"|"except"|"yield"|"raise"|"break"|"continue"|"pass"|"if"|"return"|"while"|"elif"|"else"|"finally")

POUNDCOMMENT      "#"[^#\n][^\n]*
SCRIPTCOMMENT     "#!".*

STARTDOCSYMS      "##"

%option never-interactive
%option nounistd
%option noyywrap

  /* Main start state */

%x Search
%x SearchMemVars

  /* Mid-comment states */

  /* %x FuncDoubleComment */
  /* %x ClassDoubleComment */
%x TripleComment
%x SpecialComment

  /* Function states */

%x FunctionDec
%x FunctionParams
%x FunctionBody
%x FunctionAnnotation
%x FunctionTypeAnnotation
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
%x SingleQuoteStringIgnore
%x DoubleQuoteStringIgnore

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

      s_indent = computeIndent(text);
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

      s_indent = computeIndent(text);
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
      s_packageCommentAllowed = false;
      BEGIN( FromMod );
   }

   ^{B}"import"{BB}  |
       "import"{BB}       {

      // start of an import statement
      s_packageCommentAllowed = false;
      BEGIN( Import );
   }

   ^{B}{IDENTIFIER}/{B}"="{B}"property" {
      // property
      QString text = QString::fromUtf8(yytext);

      current->section     = Entry::VARIABLE_SEC;
      current->mtype       = MethodType::Property;
      current->m_entryName = text.trimmed();

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine     = yyLineNr;
      current->startBodyLine = yyLineNr;

      s_packageCommentAllowed = false;

      BEGIN(VariableDec);
   }

   ^{B}{IDENTIFIER}/{B}"="[^=] {
      // variable
      if (s_search_count) {
         REJECT;
      }

      QString text = QString::fromUtf8(yytext);

      s_indent = computeIndent(text);
      current->section        = Entry::VARIABLE_SEC;
      current->m_entryName    = text.trimmed();

      current->setData(EntryKey::File_Name, yyFileName);;
      current->startLine      = yyLineNr;
      current->startBodyLine  = yyLineNr;
      s_packageCommentAllowed = false;

      BEGIN(VariableDec);
   }

   {B}{IDENTIFIER}/({B},{B}{IDENTIFIER})*{B}")"*{B}"="[^=] {
      // list of variables, can not place the default value so we will skip it later on in a general rule
      // Also note ")" this is to catch also (a,b). the "("
      // is caught in the rule: [(], the ")" will be handled in [)]

      if (s_search_count > 1) {
         REJECT;
      }

      QString text = QString::fromUtf8(yytext);
      s_indent = computeIndent(text);

      current->section        = Entry::VARIABLE_SEC;
      current->m_entryName    = text.trimmed();

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine      = yyLineNr;
      current->startBodyLine  = yyLineNr;

      s_packageCommentAllowed = false;
      newVariable();
   }

   "'"          {
      // start of a single quoted string
      s_stringContext = YY_START;

      s_defVal_active = false;
      s_docsEntry     = QSharedPointer<Entry>();

      s_packageCommentAllowed = false;

      BEGIN( SingleQuoteString );
   }

   "\""         {
      // start of a double quoted string
      s_stringContext = YY_START;

      s_defVal_active = false;
      s_docsEntry     = QSharedPointer<Entry>();

      s_packageCommentAllowed = false;

      BEGIN( DoubleQuoteString );
   }

   "@staticmethod"  {
      s_static = true;
   }

   {SCRIPTCOMMENT}   {
      // Unix type script comment
      if (yyLineNr != 1)   {
         REJECT;
      }
   }

   {POUNDCOMMENT}    {
      // normal comment
      s_packageCommentAllowed = false;
   }

   {IDENTIFIER}      {
      // some other identifier
      s_packageCommentAllowed = false;
   }

   ^{BB}  {
      QString text = QString::fromUtf8(yytext);
      s_curIndent  = computeIndent(text);
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
      s_curIndent  = computeIndent(text);

      s_packageCommentAllowed = false;
      initSpecialBlock();
      BEGIN(SpecialComment);
   }

   [(]     {
      // we have to do something with (
      s_search_count += 1;
   }

   [)]     {
      // we have to do something with )
      s_search_count -= 1;
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
      s_packageName = text;
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
      QString item      = s_packageName;

      current->m_entryName     = removeRedundantWhiteSpace(substitute(item,".","::"));
      current->section  = Entry::USINGDIR_SEC;
      current->setData(EntryKey::File_Name, yyFileName);

      current_root->addSubEntry(current);

      current = QMakeShared<Entry>();
      initEntry();
      BEGIN(Search);
   }

   {IDENTIFIER}/{B}","{B} {
      QString text = QString::fromUtf8(yytext);
      QString item = s_packageName+"." + text;

      current->m_entryName = removeRedundantWhiteSpace(substitute(item,".","::"));
      current->section  = Entry::USINGDECL_SEC;
      current->setData(EntryKey::File_Name, yyFileName);

      current_root->addSubEntry(current);
      current = QMakeShared<Entry>();
      initEntry();
   }

  {IDENTIFIER}  {
      QString text  = QString::fromUtf8(yytext);
      QString item  = s_packageName+"." + text;

      current->m_entryName = removeRedundantWhiteSpace(substitute(item,".","::"));
      current->section  = Entry::USINGDECL_SEC;
      current->setData(EntryKey::File_Name, yyFileName);

      current_root->addSubEntry(current);
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

      current->m_entryName = removeRedundantWhiteSpace(substitute(text,".","::"));
      current->section  = Entry::USINGDECL_SEC;
      current->setData(EntryKey::File_Name, yyFileName);

      current_root->addSubEntry(current);
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

         DBG_CTX((stderr,"Found instance method variable %s in %s at %d\n", csPrintable(text.mid(5)),
                  csPrintable(current_root->m_entryName), yyLineNr));

         current->m_entryName    = text.mid(5);
         current->section        = Entry::VARIABLE_SEC;

         current->setData(EntryKey::File_Name,   yyFileName);
         current->startLine      = yyLineNr;
         current->startBodyLine  = yyLineNr;

         current->setData(EntryKey::Member_Type, "");

         if (current->m_entryName.at(0) == '_') {

            if (current->m_entryName.size() < 2 || current->m_entryName.at(1) == '_')  {
               // mark as private
               current->protection = Private;

            } else {
              // mark as protected
              current->protection = Protected;

            }
         }

         newEntry();
   }

   "cls."{IDENTIFIER}/{B}"=" {
      QString text = QString::fromUtf8(yytext);

      DBG_CTX(stderr, "Found class method variable %s in %s at%d\n",
                  csPrintable(text.mid(4)), csPrintable(current_root->m_entryName), yyLineNr);

      current->m_entryName   = text.mid(4);
      current->section       = Entry::VARIABLE_SEC;

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine     = yyLineNr;
      current->startBodyLine = yyLineNr;

      current->setData(EntryKey::Member_Type, "");

      if (current->m_entryName.at(0) == '_') {

         if (current->m_entryName.size() < 2 || current->m_entryName.at(1) == '_')  {
            // mark as private
            current->protection = Private;

         } else {
           // mark as protected
           current->protection = Protected;

         }
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
      s_stringContext = YY_START;

      s_defVal_active = false;
      s_docsEntry     = QSharedPointer<Entry>();

      BEGIN( SingleQuoteString );
   }

   "\""              {
      // start of a double quoted string
      s_stringContext = YY_START;

      s_defVal_active = false;
      s_docsEntry     = QSharedPointer<Entry>();

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
   \n{B}/{IDENTIFIER}[^{LETTER}{DIGIT}_]  {
      DBG_CTX((stderr, "indent %d <= %d\n", computeIndent(text.mid(1)), s_indent));

      QString text = QString::fromUtf8(yytext);

      if (computeIndent(text.mid(1)) <= s_indent)  {

         for (int i = yyleng - 1; i >= 0; i--) {
            unput(yytext[i]);
         }

         endOfDef();
         BEGIN(Search);

      } else {
         incLineNr();
         current->appendData(EntryKey::Source_Text, text);
      }
   }

   \n{B}/"##"          {
      QString text = QString::fromUtf8(yytext);

      if (computeIndent(text.mid(1)) <= s_indent)  {

         for (int i = yyleng - 1; i >= 0; i--) {
           unput(yytext[i]);
         }

         endOfDef();
         BEGIN(Search);

      } else {
         incLineNr();
         current->appendData(EntryKey::Source_Text, text);
      }
   }

   <<EOF>>          {
         endOfDef();
         yyterminate();
   }

   ^{BB}/\n         {
      // skip empty line
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
   }

   ^{BB}         {
      // something at indent >0
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);

      s_curIndent = computeIndent(text);

      if (s_curIndent <= s_indent) {
         // jumped out of the function
         endOfDef(1);
         BEGIN(Search);
      }
   }

   "'"                {
      // start of a single quoted string
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);

      s_stringContext = YY_START;
      s_specialBlock  = false;

      s_defVal_active = false;
      s_docsEntry     = current;
      s_docsEnum      = EntryKey::Source_Text;

      BEGIN( SingleQuoteString );
   }

   "\""              {
      // start of a double quoted string
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);

      s_stringContext = YY_START;
      s_specialBlock  = false;

      s_defVal_active = false;
      s_docsEntry     = current;
      s_docsEnum      = EntryKey::Source_Text;

      BEGIN( DoubleQuoteString );
   }

   [^ \t\n#'".]+     {
      // non-special stuff
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);;
      s_specialBlock = false;
   }

    ^{POUNDCOMMENT}   {
      // normal comment
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
   }

   "#".*             {
      // comment half way
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
   }

   {NEWLINE}        {
      QString text = QString::fromUtf8(yytext);
      incLineNr();
      current->appendData(EntryKey::Source_Text, text);
   }

   .                 {
      // any character
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text[0]);

      s_specialBlock = false;
   }

   {TRIDOUBLEQUOTE}  {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
      initTriDoubleQuoteBlock();

      BEGIN(TripleComment);
   }

   {TRISINGLEQUOTE}  {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
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

      if (current->getData(EntryKey::Member_Type).isEmpty())  {
         current->setData(EntryKey::Member_Type, "def");
      }

      current->m_entryName = text;
      current->m_entryName = current->m_entryName.trimmed();
      newFunction();
   }

   {B}":"{B}       {
      // function without arguments
      s_specialBlock = true;          // expecting a docstring
      bodyEntry      = current;

      current->startBodyLine = yyLineNr;

      BEGIN(FunctionBody);
   }

   "->"           {
      s_defVal.clear();
      s_braceCount = 0;

      BEGIN(FunctionTypeAnnotation);
   }

   {B}"("        {
      s_funcParamsEnd = false;
      current->startBodyLine = yyLineNr;
      BEGIN( FunctionParams );
   }

   ")"           {
      // end of parameter list

      current->setData(EntryKey::Member_Args, argListToString(current->argList));
      s_funcParamsEnd = true;
   }
}

<FunctionParams>{
   ({BB}|",")          {
   }

   [\*]+               {
      s_argType = QString::fromUtf8(yytext);
   }

   {IDENTIFIER}        {
      // Name of parameter
      QString text = QString::fromUtf8(yytext);
      lineCount();

      current->argList.append( Argument() );
      current->argList.last().name = text.trimmed();
      current->argList.last().type = s_argType;
      s_argType = "";
   }

   "="                       {
      // default value

      // TODO: this rule is too simple, need to be able to
      // match things like =")" as well

      s_defVal.clear();
      s_braceCount = 0;

      BEGIN(FunctionParamDefVal);
   }

   ")"                       {
      unput(*yytext);
      BEGIN(FunctionDec);

   }

   ":"{B}                    {
      s_defVal.clear();
      s_braceCount = 0;
      BEGIN(FunctionAnnotation);
   }

   {POUNDCOMMENT}            {
      // a comment
   }

   {PARAMNONEMPTY}           {
      // default rule inside arguments
   }
}

<FunctionTypeAnnotation>{
   "{"      |
   "["      |
   "("                       {
      QString text = QString::fromUtf8(yytext);
      s_defVal += text[0];

      ++s_braceCount;
   }

   "}"      |
   "]"      |
   ")"                       {
      QString text = QString::fromUtf8(yytext);
      s_defVal += text[0];

      --s_braceCount;
   }

   ":"                       {
      QString text = QString::fromUtf8(yytext);

      if (s_braceCount == 0) {
         current->setData(EntryKey::Member_Type, s_defVal);

         unput(*yytext);
         BEGIN(FunctionDec);

      } else {
         s_defVal += text[0];
      }
   }

   "'"                       {
      QString text = QString::fromUtf8(yytext);
      s_defVal   += text[0];

      s_defVal_active = true;

      s_stringContext=FunctionTypeAnnotation;
      BEGIN(SingleQuoteString);
   }

   "\""                      {
      QString text = QString::fromUtf8(yytext);
      s_defVal   += text[0];

      s_defVal_active = true;

      s_stringContext = FunctionTypeAnnotation;
      BEGIN(DoubleQuoteString);
   }

   \n                        {
      QString text = QString::fromUtf8(yytext);
      s_defVal += text[0];
      incLineNr();
   }

   .                         {
      QString text = QString::fromUtf8(yytext);
      s_defVal += text[0];
   }
}

<FunctionAnnotation>{
   "{"       |
   "["       |
   "("                       {
      QString text = QString::fromUtf8(yytext);
      s_defVal += text[0];

      ++s_braceCount;
   }

   "}"       |
   "]"                       {
      QString text = QString::fromUtf8(yytext);
      s_defVal += text[0];

      --s_braceCount;
   }

   ")"        |
   "="        |
   ","                   {
      QString text = QString::fromUtf8(yytext);

      if (s_braceCount == 0){
         // end of default argument

         if (! current->argList.listEmpty()) {
            current->argList.last().type = s_defVal.trimmed();
         }

         if (text[0] != ',') {
            unput(*yytext);
         }

         BEGIN(FunctionParams);

      } else {
         if (text[0] == ')') {
            --s_braceCount;
         }

         s_defVal += text[0];
      }
   }

   "'"                       {
      QString text = QString::fromUtf8(yytext);
      s_defVal += text[0];

      s_defVal_active = true;

      s_stringContext = FunctionAnnotation;
      BEGIN(SingleQuoteString);
   }

   "\""                      {
      QString text = QString::fromUtf8(yytext);
      s_defVal += text[0];

      s_defVal_active = true;

      s_stringContext = FunctionAnnotation;
      BEGIN(DoubleQuoteString);
   }

   \n                        {
      QString text = QString::fromUtf8(yytext);
      s_defVal += text[0];

      incLineNr();
   }

   .             {
      QString text = QString::fromUtf8(yytext);
      s_defVal += text[0];
   }
}

<FunctionParamDefVal>{
   "{"       |
   "["       |
   "("                       {
      QString text = QString::fromUtf8(yytext);
      s_defVal += text[0];

      ++s_braceCount;
   }

   "}"       |
   "]"                       {
      QString text = QString::fromUtf8(yytext);
      s_defVal += text[0];

      --s_braceCount;
   }

   ")"       |
   ","                       {
      QString text = QString::fromUtf8(yytext);

      if (s_braceCount == 0) {

         if (! current->argList.listEmpty()) {
            current->argList.last().defval = s_defVal.trimmed();
         }

         if (text[0] != ',') {
            unput(*yytext);
         }

         BEGIN(FunctionParams);

      } else {
         if (text[0] == ')') {
            --s_braceCount;
         }

         s_defVal += text[0];
      }
   }

   "'"                       {
      QString text = QString::fromUtf8(yytext);
      s_defVal   += text[0];

      s_defVal_active = true;

      s_stringContext  = FunctionParamDefVal;
      BEGIN(SingleQuoteString);
   }

   "\""                      {
      QString text = QString::fromUtf8(yytext);
      s_defVal += text[0];

      s_defVal_active = true;

      s_stringContext = FunctionParamDefVal;
      BEGIN(DoubleQuoteString);
   }

   \n                        {
      QString text = QString::fromUtf8(yytext);
      s_defVal += text[0];
      incLineNr();
   }

   .                         {
      QString text = QString::fromUtf8(yytext);
      s_defVal += text[0];
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
      current->appendData(EntryKey::Source_Text, text);
   }

   <<EOF>>          {
         endOfDef();
         yyterminate();
   }

   ^{BB}         {
      // something at indent >0
      QString text = QString::fromUtf8(yytext);
      s_curIndent  = computeIndent(text);

      DBG_CTX((stderr,"s_curIndent = %d s_indent = %d\n", s_curIndent, s_indent));

      if (s_curIndent<=s_indent)  {
         // jumped out of the class/method

         endOfDef(1);
         s_indent = s_curIndent;
         BEGIN(Search);

      } else {
         current->appendData(EntryKey::Source_Text, text);
      }
   }

   "'"                {
      // start of a single quoted string
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text[0]);

      s_stringContext = YY_START;
      s_specialBlock  = false;

      s_defVal_active = false;
      s_docsEntry     = current;
      s_docsEnum      = EntryKey::Source_Text;

      BEGIN( SingleQuoteString );
   }

   "\""              {
      // start of a double quoted string
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text[0]);

      s_stringContext = YY_START;
      s_specialBlock  = false;

      s_defVal_active = false;
      s_docsEntry     = current;
      s_docsEnum      = EntryKey::Source_Text;

      BEGIN( DoubleQuoteString );
   }

   [^ \t\n#'"]+      {
      // non-special stuff
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
      s_specialBlock = false;
   }

   {NEWLINE}        {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text[0]);
      incLineNr();
   }

   {POUNDCOMMENT}    {
      // normal comment
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
   }

   .                 {
      // any character
      QString text = QString::fromUtf8(yytext);
      s_specialBlock = false;
      current->appendData(EntryKey::Source_Text, text);
   }

   {TRIDOUBLEQUOTE}  {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);

      initTriDoubleQuoteBlock();
      BEGIN(TripleComment);
   }

    {TRISINGLEQUOTE}  {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);

      initTriSingleQuoteBlock();
      BEGIN(TripleComment);
   }
}

<ClassDec>{IDENTIFIER} {
      QString text = QString::fromUtf8(yytext);

      if (current->getData(EntryKey::Member_Type).isEmpty())  {
         current->setData(EntryKey::Member_Type, "class");
      }

      current->section = Entry::CLASS_SEC;
      current->m_entryName = text;

      // prepend scope in case of nested classes
      if (current_root->section & Entry::SCOPE_MASK) {
         current->m_entryName.prepend(current_root->m_entryName + "::");
      }

      current->m_entryName     = current->m_entryName.trimmed();
      current->setData(EntryKey::File_Name,   yyFileName);

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
      s_specialBlock         = true;                // expecting a docstring
      current->startBodyLine = yyLineNr;
      current->setData(EntryKey::Source_Text, "");
      BEGIN(ClassCaptureIndent);
   }

   {SCOPE}            {
      QString text = QString::fromUtf8(yytext);
      current->extends.append(BaseInfo(substitute(text,".","::"), Public, Normal));
      // Has base class-do stuff
   }

   "'"                        {
      // start of a single quoted string
      s_stringContext = YY_START;
      BEGIN(SingleQuoteStringIgnore);
   }

   "\""                      {
      // start of a double quoted string
      s_stringContext = YY_START;
      BEGIN(DoubleQuoteStringIgnore);
   }
}

<SingleQuoteStringIgnore>{
   "'"                        {
      // end of a single quoted string
      BEGIN(s_stringContext);
   }

   .                       {
      // do nothing
   }
}

<DoubleQuoteStringIgnore>{
   "\""                       {
      // end of a double quoted string
      BEGIN(s_stringContext);
   }

   .                       {
      // do nothing
   }
}

<ClassCaptureIndent>{
   "\n"|({BB}"\n")            {
      // Blankline - ignore, keep looking for indentation.
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);

      lineCount();
   }

   {TRIDOUBLEQUOTE}           {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);

      initTriDoubleQuoteBlock();
      current->appendData(EntryKey::Source_Text, text);
      BEGIN(TripleComment);
   }

   {TRISINGLEQUOTE}           {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);

      initTriSingleQuoteBlock();
      current->appendData(EntryKey::Source_Text, text);
      BEGIN(TripleComment);
   }

   {STARTDOCSYMS}[#]*         {
         // start of a special comment
         initSpecialBlock();
         BEGIN(SpecialComment);
      }

   {POUNDCOMMENT}             {
         // ignore comment with just one #
      }

   ^{BB}             {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);

      s_curIndent = computeIndent(text);
      bodyEntry   = current;

      DBG_CTX((stderr,"setting indent %d\n",s_curIndent));

      BEGIN(ClassBody);
   }

   ""/({NONEMPTY}|{EXPCHAR})  {
      // Just pushback an empty class, and resume parsing the body
      QString text = QString::fromUtf8(yytext);

      newEntry();
      current->appendData(EntryKey::Source_Text, text);

      BEGIN( Search );
   }
}

<VariableDec>{
   "="                {
      // the assignment operator
      QString text = QString::fromUtf8(yytext);
      s_start_init = true;

      current->setData(EntryKey::Initial_Value, text + " ");
   }

   {B}                {
      // spaces
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Initial_Value, text);
   }

   {INTNUMBER}          {
      // integer value
      QString text  = QString::fromUtf8(yytext);

      if (current->getData(EntryKey::Member_Type).isEmpty())  {
         current->setData(EntryKey::Member_Type, "int");
      }

      current->appendData(EntryKey::Initial_Value, text);
   }

   {FLOATNUMBER}      {
      // floating point value
      QString text  = QString::fromUtf8(yytext);

      if (current->getData(EntryKey::Member_Type).isEmpty())  {
         current->setData(EntryKey::Member_Type, "float");
      }

      current->appendData(EntryKey::Initial_Value, text);
   }

   {BOOL}             {
      // boolean value
      QString text  = QString::fromUtf8(yytext);

      if (current->getData(EntryKey::Member_Type).isEmpty())  {
         current->setData(EntryKey::Member_Type, "bool");
      }

      current->appendData(EntryKey::Initial_Value, text);
   }

   {STRINGPREFIX}?"'" {
      // string
      QString text  = QString::fromUtf8(yytext);

      if (current->getData(EntryKey::Member_Type).isEmpty())  {
         current->setData(EntryKey::Member_Type, "string");
      }

      current->appendData(EntryKey::Initial_Value, text);

      s_defVal_active = false;
      s_docsEntry     = current;
      s_docsEnum      = EntryKey::Initial_Value;

      s_stringContext = VariableDec;

      BEGIN( SingleQuoteString );
   }

   {STRINGPREFIX}?"\"" {
      // string
      QString text  = QString::fromUtf8(yytext);

      if (current->getData(EntryKey::Member_Type).isEmpty())  {
         current->setData(EntryKey::Member_Type, "string");
      }

      current->appendData(EntryKey::Initial_Value, text);

      s_defVal_active = false;
      s_docsEntry     = current;
      s_docsEnum      = EntryKey::Initial_Value;

      s_stringContext = VariableDec;
      BEGIN( DoubleQuoteString );
   }

   {TRIDOUBLEQUOTE}   {
      // start of a comment block
      QString text  = QString::fromUtf8(yytext);

      if (current->getData(EntryKey::Member_Type).isEmpty())  {
         current->setData(EntryKey::Member_Type, "string");
      }

      current->appendData(EntryKey::Initial_Value, text);

      s_doubleQuote = true;

      s_defVal_active = false;
      s_docsEntry     = current;
      s_docsEnum      = EntryKey::Initial_Value;

      s_stringContext = VariableDec;

      BEGIN(TripleString);
   }

   {TRISINGLEQUOTE}   {
      // start of a comment block
      QString text  = QString::fromUtf8(yytext);

      if (current->getData(EntryKey::Member_Type).isEmpty())  {
         current->setData(EntryKey::Member_Type, "string");
      }

      current->appendData(EntryKey::Initial_Value, text);

      s_doubleQuote = false;

      s_defVal_active = false;
      s_docsEntry     = current;
      s_docsEnum      = EntryKey::Initial_Value;

      s_stringContext = VariableDec;

      BEGIN(TripleString);
   }

   "("            {
      // tuple, only when direct after =

      if (current->mtype != MethodType::Property && s_start_init) {
         current->setData(EntryKey::Member_Type, "tuple");
      }

      current->appendData(EntryKey::Initial_Value, "(");

      s_atomStart = '(';
      s_atomEnd   = ')';
      s_atomCount = 1;
      BEGIN( VariableAtom );
   }

   "["                {
      // list
      if (s_start_init) {
         current->setData(EntryKey::Member_Type, "list");
      }

      current->appendData(EntryKey::Initial_Value, "[");

      s_atomStart = '[';
      s_atomEnd   = ']';
      s_atomCount = 1;
      BEGIN( VariableAtom );
   }

   "{"            {
      // dictionary

      if (s_start_init) {
         current->setData(EntryKey::Member_Type, "dictionary");
      }

      current->appendData(EntryKey::Initial_Value, "{");

      s_atomStart = '{';
      s_atomEnd   = '}';
      s_atomCount = 1;
      BEGIN( VariableAtom );
   }

   "#".*              {
      // comment
      BEGIN( VariableEnd );
   }

   {IDENTIFIER}         {
      QString text = QString::fromUtf8(yytext);

      // do something based on the type of the IDENTIFIER
      if (current->getData(EntryKey::Member_Type).isEmpty())  {

         for (auto child : current_root->children() )  {

            if (child->m_entryName == text) {
               current->setData(EntryKey::Member_Type,  child->getData(EntryKey::Member_Type));
               break;
            }
         }
      }

      s_start_init = false;
      current->appendData(EntryKey::Initial_Value, text);
   }

   .           {
      QString text = QString::fromUtf8(yytext);
      s_start_init = false;
      current->appendData(EntryKey::Initial_Value, text);
   }

   \n             {
      unput('\n');
      BEGIN( VariableEnd );
   }
}

<VariableAtom>{
   [\(\[\{]         {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Initial_Value,  text[0]);

      if (s_atomStart == text[0]) {
         s_atomCount++;
      }
   }

   [\)\]\}]         {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Initial_Value,  text[0]);

      if (s_atomEnd == text[0]) {
         s_atomCount--;
      }

      if (s_atomCount == 0) {
         s_start_init = false;
         BEGIN(VariableDec);
      }
   }

   {TRIDOUBLEQUOTE}  {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);
      s_specialBlock = false;
      current->appendData(EntryKey::Source_Text, text);
      initTriDoubleQuoteBlock();
      BEGIN(TripleComment);
   }

   {TRISINGLEQUOTE}  {
      // start of a comment block
      QString text = QString::fromUtf8(yytext);

      s_specialBlock = false;
      current->appendData(EntryKey::Source_Text, text);
      initTriSingleQuoteBlock();

      BEGIN(TripleComment);
   }

   "'"                 {
      s_stringContext = YY_START;
      current->appendData(EntryKey::Initial_Value, "'");

      s_defVal_active = false;
      s_docsEntry     = current;
      s_docsEnum      = EntryKey::Initial_Value;

      BEGIN( SingleQuoteString );
   }

   "\""                {
      s_stringContext = YY_START;
      current->appendData(EntryKey::Initial_Value, "\"");

      s_defVal_active = false;
      s_docsEntry     = current;
      s_docsEnum      = EntryKey::Initial_Value;

      BEGIN( DoubleQuoteString );
   }

   {IDENTIFIER}       {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Initial_Value, text);
   }

   .           {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Initial_Value, text);
   }

   \n             {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Initial_Value, text);

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
   }
}

<TripleComment>{
   {ENDTRIDOUBLEQUOTE}   |
   {ENDTRISINGLEQUOTE}   {
      QString text = QString::fromUtf8(yytext);

      if (s_doubleQuote == (text[0] == '"'))  {

         if (s_specialBlock) {
            // expecting a docstring
            QString actualDoc = docBlock;

            if (! docBlockSpecial) {
               // legacy unformatted docstring
               if (! actualDoc.isEmpty()) {
                  actualDoc.prepend("\\verbatim ");
                  actualDoc.append("\\endverbatim ");
               }
            }

            handleCommentBlock(actualDoc, false);

         } else if (s_packageCommentAllowed) {
            // expecting module docs
            QString actualDoc = docBlock;

            if (! docBlockSpecial) {
               // legacy unformatted docstring
               if (! actualDoc.isEmpty()) {

                  actualDoc.prepend("\\verbatim ");
                  actualDoc.append("\\endverbatim ");
               }
            }

            actualDoc.prepend("\\namespace " + s_moduleScope + " ");
            handleCommentBlock(actualDoc, false);
         }

         if ((docBlockContext == ClassBody) || docBlockContext == FunctionBody) {
            current->appendData(EntryKey::Source_Text, docBlock);
            current->appendData(EntryKey::Source_Text, text);
         }

         BEGIN(docBlockContext);

      } else  {
         docBlock += text;

      }

      s_packageCommentAllowed = false;
   }

   ^{BB}      {
      // leading whitespace
      QString text = QString::fromUtf8(yytext);
      int indent   = computeIndent(text);

      if (indent >= s_curIndent) {
         // strip s_curIndent amount of whitespace

         for (int i= 0; i<indent - s_curIndent; i++) {
            docBlock += ' ';
         }

         DBG_CTX((stderr,"stripping indent %d\n",s_curIndent));

      } else {
         DBG_CTX((stderr,"not stripping: %d<%d\n",indent,s_curIndent));
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
      addToOutput(text);
      incLineNr();
   }

   \\.               {
      // escaped char
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

   "\"\"\""             {
      // triple double quotes
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

   "'"               {
      // end of the string
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
      BEGIN(s_stringContext);
   }

   [^"'\n\\]+                   {
      // normal chars
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

   .                 {
      // normal char
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }
}

<DoubleQuoteString>{
   \\{B}\n                    {
      // line continuation
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
      incLineNr();
   }

   \\.               {
      // escaped char
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
                               }
   "'''"             {
      // triple single quotes
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

   "\""              {
      // end of the string
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
      BEGIN(s_stringContext);
   }

   [^"'\n\\]+                   {
      // normal chars
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

   .                 {
      // normal char
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }
}

<TripleString>{
   {ENDTRIDOUBLEQUOTE}    |
   {ENDTRISINGLEQUOTE}    {
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);

      bool tmp = (text[0] == '"');

      if (s_doubleQuote == tmp)  {
         BEGIN(s_stringContext);
      }
   }

   ({LONGSTRINGBLOCK}) {
      QString text = QString::fromUtf8(yytext);
      lineCount();
      addToOutput(text);
   }

   \n         {
      QString text = QString::fromUtf8(yytext);
      incLineNr();
      addToOutput(text);
   }

   .          {
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
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
   // safter to make a temp copy
   const QVector<QSharedPointer<Entry>> tmpChildren = rt->children();

   for (auto ce : tmpChildren) {

      if (! ce->getData(EntryKey::Source_Text).isEmpty()) {
         // init scanner state

         s_inputString   = ce->getData(EntryKey::Source_Text);
         s_inputPosition = 0;

         yyrestart( yyin );

         if (ce->section & Entry::COMPOUND_MASK) {
            current_root = ce;
            BEGIN( Search );

         } else if (ce->parent()) {
            current_root = ce->parent();
            BEGIN( SearchMemVars );
         }

         yyFileName = ce->getData(EntryKey::File_Name);
         yyLineNr   = ce->startBodyLine;

         current = QMakeShared<Entry>();
         initEntry();

         groupEnterCompound(yyFileName, yyLineNr, ce->m_entryName);

         yylex() ;
         s_lexInit = true;

         current = QSharedPointer<Entry>();
         ce->setData(EntryKey::Source_Text, "");

         groupLeaveCompound(yyFileName, yyLineNr, ce->m_entryName);

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
   mtype           = MethodType::Method;
   s_static        = false;
   virt            = Normal;

   current_root    = rt;
   s_specialBlock  = false;

   s_inputFile.setFileName(fileName);

   if (s_inputFile.open(QIODevice::ReadOnly)) {
      yyLineNr   = 1;
      yyFileName = fileName;

      //setContext();
      msg("Parsing %s\n", csPrintable(yyFileName));

      QFileInfo fi(fileName);
      s_moduleScope = findPackageScope(fileName);

      QString baseName = fi.baseName();

      if (baseName != "__init__") { // package initializer file is not a package itself
         if (!s_moduleScope.isEmpty()) {
            s_moduleScope += "::";
         }
         s_moduleScope += baseName;
      }

      current            = QMakeShared<Entry>();
      initEntry();

      current->m_entryName    = s_moduleScope;
      current->section        = Entry::NAMESPACE_SEC;
      current->setData(EntryKey::Member_Type, "namespace");

      current->setData(EntryKey::File_Name,   yyFileName);
      current->startLine      = yyLineNr;
      current->startBodyLine  = yyLineNr;

      rt->addSubEntry(current);

      current_root  = current;
      initParser();

      current = QMakeShared<Entry>();
      groupEnterFile(yyFileName, yyLineNr);

      current->reset();
      initEntry();
      yyrestart( yyin );

      BEGIN( Search );
      yylex();
      s_lexInit = true;

      groupLeaveFile(yyFileName, yyLineNr);

      current_root->setData(EntryKey::Source_Text, "");

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

   s_specialBlock = false;
   s_packageCommentAllowed = false;

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
   s_lexInit = true;

   current->m_entryName = current->m_entryName.trimmed();
   if (current->section == Entry::MEMBERDOC_SEC && current->getData(EntryKey::Member_Args).isEmpty()) {
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
   if (s_lexInit) {
      yylex_destroy();
   }
}

void Python_Parser::parseInput(const QString &fileName, const QString &fileBuf,
                  QSharedPointer<Entry> root, enum ParserMode mode, QStringList &includedFiles, bool useClang)
{
   (void) mode;
   (void) includedFiles;
   (void) useClang;

   s_thisParser = this;

   printlex(yy_flex_debug, true, __FILE__, fileName);
   ::parseMain(fileName, fileBuf, root);
   printlex(yy_flex_debug, false, __FILE__, fileName);
}

bool Python_Parser::needsPreprocessing(const QString &) const
{
  return false;
}

void Python_Parser::parseCode(CodeGenerator &codeOutIntf, const QString &scopeName,
                  const QString &input, SrcLangExt, bool isExampleBlock,
                  const QString &exampleName, QSharedPointer<FileDef> fileDef, int startLine,
                  int endLine, bool inlineFragment, QSharedPointer<MemberDef> memberDef,
                  bool showLineNumbers, QSharedPointer<Definition> searchCtx, bool collectXRefs )
{
   ::parsePythonCode(codeOutIntf, scopeName, input, isExampleBlock, exampleName,
                  fileDef, startLine, endLine, inlineFragment, memberDef,
                  showLineNumbers, searchCtx, collectXRefs);
}

void Python_Parser::parsePrototype(const QString &text)
{
   ::parsePrototype(text);
}

void Python_Parser::resetCodeParserState()
{
   ::resetPythonCodeParserState();
}


