/*************************************************************************
 *
 * Copyright (c) 2014-2022 Barbara Geller & Ansel Sermersheim
 * Copyright (c) 1997-2014 Dimitri van Heesch

*************************************************************************/

%{

#include <QRegularExpression>
#include <QStringList>

#include <stdio.h>
#include <assert.h>
#include <ctype.h>

#include <arguments.h>
#include <default_args.h>
#include <entry.h>
#include <message.h>
#include <util.h>

#define YY_NO_INPUT 1

static QString          s_inputString;
static int              s_inputPosition;
static ArgumentList     s_argList;
static SrcLangExt       s_language;
static QString         *s_copyArgValue;
static QString          s_curArgTypeName;
static QString          s_curArgDefValue;
static QString          s_curArgName;
static QString          s_curArgDocs;
static QString          s_curArgAttrib;
static QString          s_curArgArray;
static QString          s_curTypeConstraint;
static QString          s_extraTypeChars;
static int              s_argRoundCount;
static int              s_argSharpCount;
static int              s_argCurlyCount;
static int              s_readArgContext;
static int              s_lastDocContext;
static QChar            s_lastDocChar;
static int              s_lastExtendsContext;
static QString          s_delimiter;

// static functions
static void yyunput(QChar c, char *yy_bp);

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

static bool checkSpecialType(QString &name)
{
   static QSet<QString> keywords;

   if (keywords.isEmpty()) {

      keywords.insert("unsigned");
      keywords.insert("signed");
      keywords.insert("bool");
      keywords.insert("char");
      keywords.insert("wchar_t");
      keywords.insert("char8_t");
      keywords.insert("char16_t");
      keywords.insert("char32_t");
      keywords.insert("int");
      keywords.insert("short");
      keywords.insert("long");
      keywords.insert("float");
      keywords.insert("double");
      keywords.insert("int8_t");
      keywords.insert("uint8_t");
      keywords.insert("int16_t");
      keywords.insert("uint16_t");
      keywords.insert("int32_t");
      keywords.insert("uint32_t");
      keywords.insert("int64_t");
      keywords.insert("uint64_t");
      keywords.insert("intmax_t");
      keywords.insert("intptr_t");
      keywords.insert("uintmax_t");
      keywords.insert("uintptr_t");
      keywords.insert("const");
      keywords.insert("void");
      keywords.insert("volatile");
   }

   return ! name.isEmpty() && keywords.contains(name);
}


%}

B         [ \t]
ID        [$a-z_A-Z\x80-\xFF][$a-z_A-Z0-9\x80-\xFF]*
RAWBEGIN  (u|U|L|u8)?R\"[^ \t\(\)\\]{0,16}"("
RAWEND    ")"[^ \t\(\)\\]{0,16}\"

%option never-interactive
%option nounistd
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
      s_curArgTypeName += " ";
   }

<ReadFuncArgType>"["[^\]]*"]"       {
      QString text = QString::fromUtf8(yytext);

      if (s_curArgTypeName.trimmed().isEmpty()) {
         // for M$-ID
         s_curArgAttrib = text;

      } else {
         // array type

         s_curArgArray += text;
      }
   }

<ReadFuncArgDef>"'"\\[0-7]{1,3}"'"  {
      QString text = QString::fromUtf8(yytext);
      s_curArgDefValue += text;
   }

<ReadFuncArgDef>"'"\\."'"     {
      QString text = QString::fromUtf8(yytext);
      s_curArgDefValue += text;
   }

<ReadFuncArgDef>"'"."'"          {
      QString text = QString::fromUtf8(yytext);
      s_curArgDefValue += text;
   }

<ReadFuncArgDef>{RAWBEGIN}              {
      QString text = QString::fromUtf8(yytext);
      s_curArgDefValue += text;

      int i = text.indexOf('"');

      s_delimiter = text.mid(i + 1);
      s_delimiter = s_delimiter.left(s_delimiter.length() - 1);
      BEGIN( CopyRawString );
   }

<ReadFuncArgDef>\"         {
      QString text = QString::fromUtf8(yytext);
      s_curArgDefValue += text[0];
      BEGIN( CopyArgString );
   }

<ReadFuncArgType>"("([^:)]+{B}*"::")*{B}*[&*\^]+{B}*/{ID} {
      // function pointer as argument
      QString text = QString::fromUtf8(yytext);
      s_curArgTypeName += text;

      // s_curArgTypeName = s_curArgTypeName.simplifyWhiteSpace();
      BEGIN( ReadFuncArgPtr );
   }

<ReadFuncArgPtr>{ID}          {
      QString text = QString::fromUtf8(yytext);
      s_curArgName = text;
   }

<ReadFuncArgPtr>")"{B}*"("       {
      // function pointer
      QString text = QString::fromUtf8(yytext);
      s_curArgTypeName += text;

      s_readArgContext = ReadFuncArgType;
      s_copyArgValue   = &s_curArgTypeName;
      s_argRoundCount  = 0;
      BEGIN( CopyArgRound2 );
   }

<ReadFuncArgPtr>")"/{B}*"["      {
      // pointer to fixed size array
      QString text = QString::fromUtf8(yytext);

      s_curArgTypeName += text;
      s_curArgTypeName += s_curArgName;
      BEGIN( ReadFuncArgType );
   }

<ReadFuncArgPtr>")"        {
      // redundant braces detected, remove them
      int i   = s_curArgTypeName.lastIndexOf('(');
      int len = s_curArgTypeName.length();

      if (i != -1) {
         s_curArgTypeName = s_curArgTypeName.left(i) + s_curArgTypeName.right(len - i - 1);
      }

      s_curArgTypeName += s_curArgName;
      BEGIN( ReadFuncArgType );
   }

<ReadFuncArgType>"<="|">="|"->"|">>"|"<<" {
      // handle operators
      QString text = QString::fromUtf8(yytext);
      s_curArgTypeName += text;
   }

<ReadFuncArgType,ReadFuncArgDef>[({<]  {
      QString text = QString::fromUtf8(yytext);

      if (YY_START == ReadFuncArgType) {
         s_curArgTypeName += text[0];
         s_copyArgValue    = &s_curArgTypeName;

      } else {
         s_curArgDefValue += text[0];
         s_copyArgValue    = &s_curArgDefValue;
      }

      s_readArgContext = YY_START;

      if (text[0] == '(') {
         s_argRoundCount = 0;
         BEGIN(CopyArgRound);

      } else if (text[0] == '{') {
         s_argCurlyCount = 0;
         BEGIN( CopyArgCurly );

      } else {
         //  text == '<'

         s_argSharpCount = 0;
         s_argRoundCount = 0;
         BEGIN(CopyArgSharp);
      }
   }

<CopyArgRound,CopyArgRound2>"("     {
      QString text = QString::fromUtf8(yytext);
      s_argRoundCount++;
      *s_copyArgValue += text[0];
   }

<CopyArgRound,CopyArgRound2>")"({B}*{ID})* {
      QString text = QString::fromUtf8(yytext);
      *s_copyArgValue += text;

      if (s_argRoundCount > 0) {
         --s_argRoundCount;

      } else {

         if (YY_START == CopyArgRound2) {
            *s_copyArgValue += " " + s_curArgName;
         }

         BEGIN(s_readArgContext);
      }
   }

<CopyArgRound>")"/{B}*                  {
      QString text = QString::fromUtf8(yytext);
      *s_copyArgValue += text[0];

      if (s_argRoundCount > 0) {
         --s_argRoundCount;

      } else {
         BEGIN(s_readArgContext);

      }
   }

<CopyArgSharp>"<<"                      {
      QString text = QString::fromUtf8(yytext);

      if (s_argRoundCount > 0) {
         *s_copyArgValue += text;

      } else {
         REJECT;
      }
   }

<CopyArgSharp>">>"                      {
      QString text = QString::fromUtf8(yytext);

      if (s_argRoundCount > 0) {
         *s_copyArgValue += text;

      } else {
         REJECT;
      }
   }

<CopyArgSharp>"<"          {
      QString text = QString::fromUtf8(yytext);

      // do not count '<' inside '(' for code like: "< typename A = (i < 6) >"
      if (s_argRoundCount == 0) {
         ++s_argSharpCount;
      }

      *s_copyArgValue += text[0];
   }

<CopyArgSharp>">"          {
      QString text = QString::fromUtf8(yytext);
      *s_copyArgValue += text[0];

      if (s_argSharpCount == 0 && s_argRoundCount > 0) {
         // do not count '<' inside '(' for code like: "< typename A = (i < 6) >"

      } else {
         if (s_argSharpCount > 0) {
            --s_argSharpCount;
         } else  {
            BEGIN(s_readArgContext);
         }
      }
   }

<CopyArgSharp>"("                       {
      QString text = QString::fromUtf8(yytext);

      *s_copyArgValue += text[0];
      ++s_argRoundCount;
   }

<CopyArgSharp>")"                       {
      QString text = QString::fromUtf8(yytext);

      *s_copyArgValue += text[0];
      --s_argRoundCount;
   }

<CopyArgCurly>"{"          {
      QString text = QString::fromUtf8(yytext);

      *s_copyArgValue += text[0];
      ++s_argCurlyCount;
   }

<CopyArgCurly>"}"          {
      QString text = QString::fromUtf8(yytext);
      *s_copyArgValue += text[0];

      if (s_argCurlyCount > 0) {
         --s_argCurlyCount;

      } else {
         BEGIN(s_readArgContext);
      }
   }

<CopyArgString>\\.         {
      QString text = QString::fromUtf8(yytext);
      s_curArgDefValue += text;
   }

<CopyRawString>{RAWEND}                 {
      QString text = QString::fromUtf8(yytext);
      s_curArgDefValue += text;

      QString delimiter = text.mid(1);
      delimiter = delimiter.left(delimiter.length() - 1);

      if (delimiter == s_delimiter) {
         BEGIN( ReadFuncArgDef );
      }
   }

<CopyArgString>\"          {
      QString text = QString::fromUtf8(yytext);
      s_curArgDefValue += text[0];
      BEGIN( ReadFuncArgDef );
   }

<ReadFuncArgType>"="          {
      BEGIN( ReadFuncArgDef );
   }

<ReadFuncArgType,ReadFuncArgDef>[,)>]{B}*("/*"[*!]|"//"[/!])"<" {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      s_lastDocContext = YY_START;
      s_lastDocChar    = text[0];

     if (text.indexOf("//") != -1) {
         BEGIN( ReadDocLine );

      } else {
         BEGIN( ReadDocBlock );
      }
   }

<ReadFuncArgType,ReadFuncArgDef>[,)>]  {
      QString text = QString::fromUtf8(yytext);

      if (text[0] == ')' && s_curArgTypeName.trimmed().isEmpty()) {
         s_curArgTypeName += text[0];
         BEGIN(FuncQual);

      } else {

         s_curArgTypeName = removeRedundantWhiteSpace(s_curArgTypeName);
         s_curArgDefValue = s_curArgDefValue.trimmed();

         int len = s_curArgTypeName.length();

         if (len > 0) {
            int i = len - 1;

            while (i >= 0 && (s_curArgTypeName.at(i).isSpace() || s_curArgTypeName.at(i) == '.')) {
               i--;
            }

            while (i >= 0 && (isId(s_curArgTypeName.at(i)) || s_curArgTypeName.at(i) == '$')) {
               i--;
            }

            Argument arg;

            arg.attrib         = s_curArgAttrib;
            arg.typeConstraint = s_curTypeConstraint.trimmed();
            arg.array          = "";

            if (i == len - 1 && s_curArgTypeName.at(i) == ')') {
               // function argument

               int bi = s_curArgTypeName.indexOf('(');
               int fi = bi - 1;

               while (fi >= 0 && (isId(s_curArgTypeName.at(fi)) || s_curArgTypeName.at(fi) == ':')) {
                  fi--;
               }

               if (fi >= 0) {
                  arg.type  = s_curArgTypeName.left(fi + 1);
                  arg.name  = s_curArgTypeName.mid(fi + 1, bi - fi - 1).trimmed();
                  arg.array = s_curArgTypeName.right(len - bi);

               } else {
                  arg.type = s_curArgTypeName;

               }

            } else if (i >= 0 && s_curArgTypeName.at(i) != ':') {
               // type contains a name

               arg.type = removeRedundantWhiteSpace(s_curArgTypeName.left(i + 1)).trimmed();
               arg.name = s_curArgTypeName.right(len - i - 1).trimmed();

               // if the argument type is not a complete type, need to correct to avoid seeing a
               // nameless parameter "struct A" as a parameter with type "struct" and name "A".

               int sv = 0;

               if (arg.type.startsWith("const ")) {
                  sv = 6;

               } else if (arg.type.startsWith("volatile ")) {
                  sv = 9;

               }

               if (arg.type.mid(sv) == "struct"   || arg.type.mid(sv) == "union" ||
                     arg.type.mid(sv) == "class"  || arg.type.mid(sv) == "typename" ||
                     checkSpecialType(arg.name)) {

                  arg.type = arg.type + " " + arg.name;
                  arg.name = "";
               }

            } else {
               // assume only the type was specified, try to determine name later
               arg.type = removeRedundantWhiteSpace(s_curArgTypeName);

            }

            if (! arg.type.isEmpty() && arg.type.at(0) == '$')  {
               // typeless PHP name?

               arg.name = arg.type;
               arg.type = "";
            }

            arg.array += removeRedundantWhiteSpace(s_curArgArray);

            int alen = arg.array.length();

            if (alen > 2 && arg.array.at(0) == '(' && arg.array.at(alen-1) == ')') {
               // fix-up for int *(a[10])

               int pos     = arg.array.indexOf('[') - 1;
               arg.array = arg.array.mid(1, alen - 2);

               if (pos > 0 && arg.name.isEmpty()) {
                  arg.name  = arg.array.left(pos).trimmed();
                  arg.array = arg.array.mid(pos);
               }

            }

            arg.defval = s_curArgDefValue;
            arg.docs   = s_curArgDocs.trimmed();

            s_argList.append(arg);
         }

         s_curArgAttrib.resize(0);
         s_curArgTypeName.resize(0);
         s_curArgDefValue.resize(0);
         s_curArgArray.resize(0);
         s_curArgDocs.resize(0);
         s_curTypeConstraint.resize(0);

         if (text[0] == ')') {
            BEGIN(FuncQual);

         } else {
            BEGIN( ReadFuncArgType );

         }
      }
   }

<ReadFuncArgType,ReadFuncArgPtr>"extends" {
      s_curTypeConstraint.resize(0);
      s_lastExtendsContext = YY_START;
      BEGIN(ReadTypeConstraint);
   }

<ReadFuncArgType,ReadFuncArgPtr>"$"?{ID} {
      QString text = QString::fromUtf8(yytext);

      if (YY_START == ReadFuncArgType && s_curArgArray == "[]") {
         // Java style array

         s_curArgTypeName += " []";
         s_curArgArray.resize(0);
      }

      s_curArgTypeName += text;
   }

<ReadFuncArgType,ReadFuncArgPtr>.   {
      QString text = QString::fromUtf8(yytext);
      s_curArgTypeName += text[0];
   }

<ReadFuncArgDef,CopyArgString>"<="|"->"|">="|">>"|"<<"   {
      QString text = QString::fromUtf8(yytext);
      s_curArgDefValue += text;
   }

<ReadFuncArgDef,CopyArgString,CopyRawString>.      {
      QString text = QString::fromUtf8(yytext);
      s_curArgDefValue += text[0];
   }

<CopyArgRound,CopyArgRound2,CopyArgSharp,CopyArgCurly>{ID}  {
      QString text = QString::fromUtf8(yytext);
      *s_copyArgValue += text;
   }

<CopyArgRound,CopyArgRound2,CopyArgSharp,CopyArgCurly>.  {
      QString text = QString::fromUtf8(yytext);
      *s_copyArgValue += text[0];
   }

<ReadTypeConstraint>[,)>]               {
      unput(*yytext);
      BEGIN(s_lastExtendsContext);
   }

<ReadTypeConstraint>.                   {
      QString text = QString::fromUtf8(yytext);
      s_curTypeConstraint += text;
   }

<ReadTypeConstraint>\n                  {
      s_curTypeConstraint += ' ';
   }

<FuncQual>"const"               {
      s_argList.constSpecifier = true;
   }

<FuncQual>"volatile"            {
      s_argList.volatileSpecifier = true;
   }

<FuncQual>"&"                   {
      s_argList.refSpecifier = RefType::LValueRef;
   }

<FuncQual>"&&"                  {
      s_argList.refSpecifier = RefType::RValueRef;
   }

<FuncQual,TrailingReturn>"="{B}*"0"    {
      s_argList.pureSpecifier = true;
      BEGIN(FuncQual);
   }

<FuncQual>"->"                          {
      // C++11 trailing return type
      s_argList.trailingReturnType = " -> ";
      BEGIN(TrailingReturn);
   }

<TrailingReturn>{B}/("final"|"override"){B}*  {
      unput(*yytext);
      BEGIN(FuncQual);
   }

<TrailingReturn>.                       {
      QString text = QString::fromUtf8(yytext);
      s_argList.trailingReturnType += text;
   }

<TrailingReturn>\n                      {
      QString text = QString::fromUtf8(yytext);
      s_argList.trailingReturnType += text;
   }

<FuncQual>")"{B}*"["[^]]*"]"     {
      // for functions returning a pointer to an array,
      // i.e. ")[]" in "int (*f(int))[4]" with argsString="(int))[4]"
      QString text = QString::fromUtf8(yytext);
      s_extraTypeChars = text;
   }

<ReadDocBlock>[^\*\n]+        {
      QString text = QString::fromUtf8(yytext);
      s_curArgDocs += text;
   }

<ReadDocLine>[^\n]+        {
      QString text = QString::fromUtf8(yytext);
      s_curArgDocs += text;
   }

<ReadDocBlock>"*/"         {
      if (s_lastDocChar != 0) {
         unput(s_lastDocChar);
      }
      BEGIN(s_lastDocContext);
   }

<ReadDocLine>\n            {
      if (s_lastDocChar != 0) {
         unput(s_lastDocChar);
      }
      BEGIN(s_lastDocContext);
   }

<ReadDocBlock>\n        {
      QString text = QString::fromUtf8(yytext);
      s_curArgDocs += text[0];
   }

<ReadDocBlock>.            {
      QString text = QString::fromUtf8(yytext);
      s_curArgDocs += text[0];
   }

<*>("/*"[*!]|"//"[/!])("<"?)     {
   // */ (editor syntax fix)
   s_lastDocContext = YY_START;
   s_lastDocChar    = 0;

   if (yytext[1] == '/')  {
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
   (void) yy_bp;

   QString tmp1    = c;
   QByteArray tmp2 = tmp1.toUtf8();

   for (int i = tmp2.length() -1; i >= 0; i-- ) {
      unput(tmp2[i]);
   }
}

/*! Converts an argument string into an ArgumentList.
 *  \param[in]  list of Arguments.
 *  \param[out] a reference to resulting argument list pointer.
 *  \param[out] extraTypeChars point to string to which trailing characters for complex types are written to
 */
ArgumentList stringToArgumentList(SrcLangExt lang, QString &extraTypeChars,
      const QString &argsString, const ArgumentList &argList)
{
   ArgumentList retval;
   if (argsString.isEmpty()) {
      return retval;
   }

   s_inputString   = argsString;       // passed in value
   s_inputPosition = 0;
   s_argList       = argList;          // passed in value
   s_language      = lang;             // passed in value

   s_copyArgValue  = nullptr;

   s_curArgTypeName.clear();
   s_curArgDefValue.clear();
   s_curArgName.clear();
   s_curArgDocs.clear();
   s_curArgAttrib.clear();
   s_curArgArray.clear();
   s_curTypeConstraint.clear();
   s_extraTypeChars.clear();

   s_argRoundCount = 0;
   s_argSharpCount = 0;
   s_argCurlyCount = 0;

   s_lastDocChar   = '\0';

   yyrestart( yyin );
   BEGIN( Start );
   yylex();

   extraTypeChars = s_extraTypeChars;

   retval = s_argList;
   return retval;
}
