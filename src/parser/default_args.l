/*************************************************************************
 *
 * Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim
 * Copyright (C) 1997-2014 by Dimitri van Heesch.

*************************************************************************/

%{

#include <QRegExp>

#include <stdio.h>
#include <assert.h>
#include <ctype.h>

#include <arguments.h>
#include <default_args.h>
#include <entry.h>
#include <message.h>
#include <util.h>

#define YY_NO_INPUT 1
#define YY_NEVER_INTERACTIVE 1


static QString          g_inputString;
static int              g_inputPosition;
static ArgumentList     g_argList;
static QString         *g_copyArgValue;
static QString          g_curArgTypeName;
static QString          g_curArgDefValue;
static QString          g_curArgName;
static QString          g_curArgDocs;
static QString          g_curArgAttrib;
static QString          g_curArgArray;
static QString          g_curTypeConstraint;
static QString          g_extraTypeChars;
static int              g_argRoundCount;
static int              g_argSharpCount;
static int              g_argCurlyCount;
static int              g_readArgContext;
static int              g_lastDocContext;
static QChar            g_lastDocChar;
static int              g_lastExtendsContext;
static QString          g_delimiter;

// static functions
static void yyunput(QChar c, char *yy_bp);

#undef   YY_INPUT
#define  YY_INPUT(buf,result,max_size) result=yyread(buf,max_size);

static int yyread(char *buf, int max_size)
{
   int c = 0;

   while (g_inputString[g_inputPosition] != 0) {

      QString tmp1    = g_inputString.at(g_inputPosition);
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

      g_inputPosition++;
   }

   return c;
}

%}

B         [ \t]
ID        [a-z_A-Z\x80-\xFF][a-z_A-Z0-9\x80-\xFF]*
RAWBEGIN  (u|U|L|u8)?R\"[^ \t\(\)\\]{0,16}"("
RAWEND    ")"[^ \t\(\)\\]{0,16}\"

%option noyywrap

%x    Start
%x    CopyArgString
%x    CopyRawString
%x    CopyArgRound
%x    CopyArgRound2
%x    CopyArgSharp
%x    CopyArgCurly
%x    ReadFuncArgType
%x    ReadFuncArgDef
%x    ReadFuncArgPtr
%x    FuncQual
%x    ReadDocBlock
%x    ReadDocLine
%x    ReadTypeConstraint
%x    TrailingReturn

%%

<Start>[<(]             {
      BEGIN(ReadFuncArgType);
   }

<ReadFuncArgType>{B}*         {
      g_curArgTypeName += " ";
   }

<ReadFuncArgType>"["[^\]]*"]"       {
      QString text = QString::fromUtf8(yytext);

      if (g_curArgTypeName.trimmed().isEmpty()) {
         // for M$-ID
         g_curArgAttrib = text;

      } else {
         // array type

         g_curArgArray += text;
      }
   }

<ReadFuncArgDef>"'"\\[0-7]{1,3}"'"  {
      QString text = QString::fromUtf8(yytext);
      g_curArgDefValue += text;
   }

<ReadFuncArgDef>"'"\\."'"     {
      QString text = QString::fromUtf8(yytext);
      g_curArgDefValue += text;
   }

<ReadFuncArgDef>"'"."'"          {
      QString text = QString::fromUtf8(yytext);
      g_curArgDefValue += text;
   }

<ReadFuncArgDef>{RAWBEGIN}              {
      QString text = QString::fromUtf8(yytext);
      g_curArgDefValue += text;

      int i = text.indexOf('"');

      g_delimiter = text.mid(i + 1);
      g_delimiter = g_delimiter.left(g_delimiter.length() - 1);
      BEGIN( CopyRawString );
   }

<ReadFuncArgDef>\"         {
      QString text = QString::fromUtf8(yytext);
      g_curArgDefValue += text[0];
      BEGIN( CopyArgString );
   }

<ReadFuncArgType>"("([^:)]+{B}*"::")*{B}*[&*\^]+{B}*/{ID} {
      // function pointer as argument
      QString text = QString::fromUtf8(yytext);
      g_curArgTypeName += text;

      // g_curArgTypeName = g_curArgTypeName.simplifyWhiteSpace();
      BEGIN( ReadFuncArgPtr );
   }

<ReadFuncArgPtr>{ID}          {
      QString text = QString::fromUtf8(yytext);
      g_curArgName = text;
   }

<ReadFuncArgPtr>")"{B}*"("       {
      // function pointer
      QString text = QString::fromUtf8(yytext);
      g_curArgTypeName += text;

      g_readArgContext = ReadFuncArgType;
      g_copyArgValue   = &g_curArgTypeName;
      g_argRoundCount  = 0;
      BEGIN( CopyArgRound2 );
   }

<ReadFuncArgPtr>")"/{B}*"["      {
      // pointer to fixed size array
      QString text = QString::fromUtf8(yytext);

      g_curArgTypeName += text;
      g_curArgTypeName += g_curArgName;
      BEGIN( ReadFuncArgType );
   }

<ReadFuncArgPtr>")"        {
      // redundant braces detected, remove them
      int i   = g_curArgTypeName.lastIndexOf('(');
      int len = g_curArgTypeName.length();

      if (i != -1) {
         g_curArgTypeName = g_curArgTypeName.left(i) + g_curArgTypeName.right(len - i - 1);
      }

      g_curArgTypeName += g_curArgName;
      BEGIN( ReadFuncArgType );
   }

<ReadFuncArgType>"<="|">="|"->"|">>"|"<<" {
      // handle operators
      QString text = QString::fromUtf8(yytext);
      g_curArgTypeName += text;
   }

<ReadFuncArgType,ReadFuncArgDef>[({<]  {
      QString text = QString::fromUtf8(yytext);

      if (YY_START == ReadFuncArgType) {
         g_curArgTypeName += text[0];
         g_copyArgValue    = &g_curArgTypeName;

      } else {
         g_curArgDefValue += text[0];
         g_copyArgValue    = &g_curArgDefValue;
      }

      g_readArgContext = YY_START;

      if (text[0] == '(') {
         g_argRoundCount = 0;
         BEGIN( CopyArgRound );

      } else if (text[0] == '{') {
         g_argCurlyCount = 0;
         BEGIN( CopyArgCurly );

      } else {
         //  text == '<'

         g_argSharpCount = 0;
         g_argRoundCount = 0;
         BEGIN( CopyArgSharp );
      }
   }

<CopyArgRound,CopyArgRound2>"("     {
      QString text = QString::fromUtf8(yytext);
      g_argRoundCount++;
      *g_copyArgValue += text[0];
   }

<CopyArgRound,CopyArgRound2>")"({B}*{ID})* {
      QString text = QString::fromUtf8(yytext);
      *g_copyArgValue += text;

      if (g_argRoundCount > 0) {
         g_argRoundCount--;

      } else {

         if (YY_START == CopyArgRound2) {
            *g_copyArgValue += " " + g_curArgName;
         }

         BEGIN( g_readArgContext );
      }
   }

<CopyArgRound>")"/{B}*                  {
      QString text = QString::fromUtf8(yytext);
      *g_copyArgValue += text[0];

      if (g_argRoundCount > 0) {
         g_argRoundCount--;

      } else {
         BEGIN( g_readArgContext );

      }
   }

<CopyArgSharp>"<<"                      {
      QString text = QString::fromUtf8(yytext);

      if (g_argRoundCount > 0) {
         *g_copyArgValue += text;

      } else {
         REJECT;
      }
   }

<CopyArgSharp>">>"                      {
      QString text = QString::fromUtf8(yytext);

      if (g_argRoundCount > 0) {
         *g_copyArgValue += text;

      } else {
         REJECT;
      }
   }

<CopyArgSharp>"<"          {
      QString text = QString::fromUtf8(yytext);
      g_argSharpCount++;
      *g_copyArgValue += text[0];
   }

<CopyArgSharp>">"          {
      QString text = QString::fromUtf8(yytext);
      *g_copyArgValue += text[0];

      if (g_argSharpCount > 0) {
         g_argSharpCount--;

      } else  {
         BEGIN( g_readArgContext );
      }
   }

<CopyArgSharp>"("                       {
      QString text = QString::fromUtf8(yytext);
      g_argRoundCount++;
      *g_copyArgValue += text[0];
   }

<CopyArgSharp>")"                       {
      QString text = QString::fromUtf8(yytext);
      g_argRoundCount--;
      *g_copyArgValue += text[0];
   }

<CopyArgCurly>"{"          {
      QString text = QString::fromUtf8(yytext);
      g_argCurlyCount++;
      *g_copyArgValue += text[0];
   }

<CopyArgCurly>"}"          {
      QString text = QString::fromUtf8(yytext);
      *g_copyArgValue += text[0];

      if (g_argCurlyCount > 0) {
         g_argCurlyCount--;

      } else {
         BEGIN( g_readArgContext );
      }
   }

<CopyArgString>\\.         {
      QString text = QString::fromUtf8(yytext);
      g_curArgDefValue += text;
   }

<CopyRawString>{RAWEND}                 {
      QString text = QString::fromUtf8(yytext);
      g_curArgDefValue += text;

      QString delimiter = text.mid(1);
      delimiter = delimiter.left(delimiter.length() - 1);

      if (delimiter == g_delimiter) {
         BEGIN( ReadFuncArgDef );
      }
   }

<CopyArgString>\"          {
      QString text = QString::fromUtf8(yytext);
      g_curArgDefValue += text[0];
      BEGIN( ReadFuncArgDef );
   }

<ReadFuncArgType>"="          {
      BEGIN( ReadFuncArgDef );
   }

<ReadFuncArgType,ReadFuncArgDef>[,)>]{B}*("/*"[*!]|"//"[/!])"<" {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      g_lastDocContext = YY_START;
      g_lastDocChar    = text[0];

     if (text.indexOf("//") != -1) {
         BEGIN( ReadDocLine );

      } else {
         BEGIN( ReadDocBlock );
      }
   }

<ReadFuncArgType,ReadFuncArgDef>[,)>]  {
      QString text = QString::fromUtf8(yytext);

      if (text[0] == ')' && g_curArgTypeName.trimmed().isEmpty()) {
         g_curArgTypeName += text[0];
         BEGIN(FuncQual);

      } else {

         g_curArgTypeName = removeRedundantWhiteSpace(g_curArgTypeName);
         g_curArgDefValue = g_curArgDefValue.trimmed();

         int len = g_curArgTypeName.length();

         if (len > 0) {
            int i = len - 1;

            while (i >= 0 && (g_curArgTypeName.at(i).isSpace() || g_curArgTypeName.at(i) == '.')) {
               i--;
            }

            while (i >= 0 && (isId(g_curArgTypeName.at(i)) || g_curArgTypeName.at(i) == '$')) {
               i--;
            }

            Argument arg;

            arg.attrib         = g_curArgAttrib;
            arg.typeConstraint = g_curTypeConstraint.trimmed();
            arg.array          = "";

            if (i == len - 1 && g_curArgTypeName.at(i)==')') {
               // function argument

               int bi = g_curArgTypeName.indexOf('(');
               int fi = bi - 1;

               while (fi >= 0 && (isId(g_curArgTypeName.at(fi)) || g_curArgTypeName.at(fi) == ':')) {
                  fi--;
               }

               if (fi >= 0) {
                  arg.type  = g_curArgTypeName.left(fi + 1);
                  arg.name  = g_curArgTypeName.mid(fi + 1, bi - fi - 1).trimmed();
                  arg.array = g_curArgTypeName.right(len - bi);

               } else {
                  arg.type = g_curArgTypeName;

               }

            } else if (i >= 0 && g_curArgTypeName.at(i)!=':') {
               // type contains a name

               arg.type = removeRedundantWhiteSpace(g_curArgTypeName.left(i + 1)).trimmed();
               arg.name = g_curArgTypeName.right(len - i - 1).trimmed();

               // if the type becomes a type specifier only then we make a mistake
               // and need to correct it to avoid seeing a nameless parameter
               // "struct A" as a parameter with type "struct" and name "A".

               int sv = 0;

               if (arg.type.left(6) == "const ") {
                  sv = 6;

               } else if (arg.type.left(9) == "volatile ") {
                  sv = 9;

               }
             
               if (arg.type.mid(sv, 6) == "struct"   || arg.type.mid(sv, 5) == "union" ||
                     arg.type.mid(sv, 5) == "class"  || arg.type.mid(sv, 8) == "typename" ||
                     arg.type == "const" || arg.type == "volatile") {

                  arg.type = arg.type + " " + arg.name;
                  arg.name = "";
               }

            } else {
               // assume only the type was specified, try to determine name later         
               arg.type = removeRedundantWhiteSpace(g_curArgTypeName);

            }

            if (! arg.type.isEmpty() && arg.type.at(0) == '$')  {
               // typeless PHP name?
            
               arg.name = arg.type;
               arg.type = "";
            }
      
            arg.array  += removeRedundantWhiteSpace(g_curArgArray);
           
            int alen = arg.array.length();
      
            if (alen > 2 && arg.array.at(0) == '(' && arg.array.at(alen-1) == ')') {
               // fix-up for int *(a[10])
      
               int i     = arg.array.indexOf('[') - 1;
               arg.array = arg.array.mid(1, alen - 2);
      
               if (i > 0 && arg.name.isEmpty()) {
                  arg.name  = arg.array.left(i).trimmed();
                  arg.array = arg.array.mid(i);
               }
      
            }
   
            arg.defval = g_curArgDefValue;
            arg.docs   = g_curArgDocs.trimmed();

            g_argList.append(arg);
         }

         g_curArgAttrib.resize(0);
         g_curArgTypeName.resize(0);
         g_curArgDefValue.resize(0);
         g_curArgArray.resize(0);
         g_curArgDocs.resize(0);
         g_curTypeConstraint.resize(0);

         if (text[0] == ')') {
            BEGIN(FuncQual);

         } else {
            BEGIN( ReadFuncArgType );

         }
      }
   }

<ReadFuncArgType,ReadFuncArgPtr>"extends" {
      g_curTypeConstraint.resize(0);
      g_lastExtendsContext = YY_START;
      BEGIN(ReadTypeConstraint);
   }

<ReadFuncArgType,ReadFuncArgPtr>"$"?{ID} {
      QString text = QString::fromUtf8(yytext);

      if (YY_START == ReadFuncArgType && g_curArgArray == "[]") {
         // Java style array

         g_curArgTypeName += " []";
         g_curArgArray.resize(0);
      }

      g_curArgTypeName += text;
   }

<ReadFuncArgType,ReadFuncArgPtr>.   {
      QString text = QString::fromUtf8(yytext);
      g_curArgTypeName += text[0];
   }

<ReadFuncArgDef,CopyArgString>"<="|"->"|">="|">>"|"<<"   {
      QString text = QString::fromUtf8(yytext);
      g_curArgDefValue += text;
   }

<ReadFuncArgDef,CopyArgString,CopyRawString>.      {
      QString text = QString::fromUtf8(yytext);
      g_curArgDefValue += text[0];
   }

<CopyArgRound,CopyArgRound2,CopyArgSharp,CopyArgCurly>{ID}  {
      QString text = QString::fromUtf8(yytext);
      *g_copyArgValue += text;
   }

<CopyArgRound,CopyArgRound2,CopyArgSharp,CopyArgCurly>.  {
      QString text = QString::fromUtf8(yytext);
      *g_copyArgValue += text[0];
   }

<ReadTypeConstraint>[,)>]               {
      unput(*yytext);
      BEGIN(g_lastExtendsContext);
   }

<ReadTypeConstraint>.                   {
      QString text = QString::fromUtf8(yytext);
      g_curTypeConstraint += text;
   }

<ReadTypeConstraint>\n                  {
      g_curTypeConstraint += ' ';
   }

<FuncQual>"const"                {
      g_argList.constSpecifier = true;
   }

<FuncQual>"volatile"             {
      g_argList.volatileSpecifier = true;
   }

<FuncQual>"&"                   {
      g_argList.refSpecifier = RefType::LValueRef;
   }

<FuncQual>"&&"                     {
      g_argList.refSpecifier = RefType::RValueRef;
   }

<FuncQual,TrailingReturn>"="{B}*"0"    {
      g_argList.pureSpecifier = true;
      BEGIN(FuncQual);
   }

<FuncQual>"->"                          {
      // C++11 trailing return type
      g_argList.trailingReturnType = " -> ";
      BEGIN(TrailingReturn);
   }

<TrailingReturn>{B}/("final"|"override"){B}*  {
      unput(*yytext);
      BEGIN(FuncQual);
   }

<TrailingReturn>.                       {
      QString text = QString::fromUtf8(yytext);
      g_argList.trailingReturnType += text;
   }

<TrailingReturn>\n                      {
      QString text = QString::fromUtf8(yytext);
      g_argList.trailingReturnType += text;
   }

<FuncQual>")"{B}*"["[^]]*"]"     {
      // for functions returning a pointer to an array,
      // i.e. ")[]" in "int (*f(int))[4]" with argsString="(int))[4]"
      QString text = QString::fromUtf8(yytext);
      g_extraTypeChars = text;
   }

<ReadDocBlock>[^\*\n]+        {
      QString text = QString::fromUtf8(yytext);
      g_curArgDocs += text;
   }

<ReadDocLine>[^\n]+        {
      QString text = QString::fromUtf8(yytext);
      g_curArgDocs += text;
   }

<ReadDocBlock>"*/"         {
      if (g_lastDocChar != 0) {
         unput(g_lastDocChar);
      }
      BEGIN(g_lastDocContext);
   }

<ReadDocLine>\n            {
      if (g_lastDocChar != 0) {
         unput(g_lastDocChar);
      }
      BEGIN(g_lastDocContext);
   }

<ReadDocBlock>\n        {
      QString text = QString::fromUtf8(yytext);
      g_curArgDocs += text[0];
   }

<ReadDocBlock>.            {
      QString text = QString::fromUtf8(yytext);
      g_curArgDocs += text[0];
   }

<*>("/*"[*!]|"//"[/!])("<"?)     {
   // */ (editor syntax fix)
   g_lastDocContext=YY_START;
   g_lastDocChar=0;

   if (yytext[1]=='/')  {
      BEGIN( ReadDocLine );
   } else {
      BEGIN( ReadDocBlock );
   }
}

<*>\n

<*>.

%%

static void yyunput(QChar c, char *yy_bp)
{
   QString tmp1    = c;
   QByteArray tmp2 = tmp1.toUtf8();

   for (int i = tmp2.length() -1; i >= 0; i-- ) {
      unput(tmp2[i]);
   }
}

/*! Converts an argument string into an ArgumentList.
 *  \param[in] argsString the list of Arguments.
 *  \param[out] al a reference to resulting argument list pointer.
 *  \param[out] extraTypeChars point to string to which trailing characters
 *              for complex types are written to
 */

ArgumentList stringToArgumentList(const QString &argsString, const ArgumentList &al)
{
   QString dummy;
   return stringToArgumentList(argsString, al, dummy);
}


ArgumentList stringToArgumentList(const QString &argsString, const ArgumentList &al, QString &extraTypeChars)
{
   if (argsString.isEmpty()) {
      return ArgumentList();
   }

   printlex(yy_flex_debug, true, __FILE__, NULL);

   g_copyArgValue = 0;
   g_curArgDocs.resize(0);
   g_curArgAttrib.resize(0);
   g_curArgArray.resize(0);
   g_curTypeConstraint.resize(0);
   g_extraTypeChars.resize(0);

   g_argRoundCount = 0;
   g_argSharpCount = 0;
   g_argCurlyCount = 0;
   g_lastDocChar   = '\0';
   g_inputString   = argsString;
   g_inputPosition = 0;

   g_curArgTypeName.resize(0);
   g_curArgDefValue.resize(0);
   g_curArgName.resize(0);

   g_argList = al;

   yyrestart( yyin );
   BEGIN( Start );
   yylex();
   
   extraTypeChars = g_extraTypeChars;
   
   printlex(yy_flex_debug, false, __FILE__, NULL);

   return g_argList;
}
