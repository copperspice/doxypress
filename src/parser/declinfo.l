/*************************************************************************
 *
 * Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim
 * Copyright (C) 1997-2014 by Dimitri van Heesch.

*************************************************************************/

%{

#include <stdio.h>
#include <assert.h>
#include <ctype.h>

#include <declinfo.h>
#include <message.h>
#include <util.h>

#define YY_NEVER_INTERACTIVE 1
#define YY_NO_INPUT 1
   
static QString  inputString;
static int      inputPosition;
static QString  className;
static QString  classTempList;
static QString  funcTempList;
static QString  s_scope;
static QString  s_type;
static QString  s_name;
static QString  s_args;

static int      sharpCount;
static bool     classTempListFound;
static bool     funcTempListFound;
static QString  exceptionString;
static bool     insideObjC;

static void addType()
{
   if (s_name.isEmpty() && s_scope.isEmpty()) {
      return;
   }

   if (! s_type.isEmpty()) {
      s_type += " ";
   }

   if (! s_scope.isEmpty()) {
      s_type += s_scope + "::";
   }

   s_type += s_name;

   s_scope.resize(0);
   s_name.resize(0);
}
  
static void addTypeName()
{
   if (s_name.isEmpty() || s_name.at(s_name.length() - 1) == ':') { 
      // end of Objective-C keyword => append to name not type
      return;
   }

   if (! s_type.isEmpty()) {
      s_type += ' ';
   }

   s_type += s_name;
   s_name.resize(0);
}
 
#undef   YY_INPUT
#define  YY_INPUT(buf,result,max_size) result=yyread(buf,max_size);

static int yyread(char *buf, int max_size)
{  
   int c = 0;

   while (inputString[inputPosition] != 0) {

      QString tmp1    = inputString.at(inputPosition);
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

      inputPosition++;     
   }

   return c;
}

%}

B     [ \t]
ID    "$"?([a-z_A-Z\x80-\xFF][a-z_A-Z0-9\x80-\xFF]*)|(@[0-9]+)

%option nounput
%option noyywrap

%x    Start
%x    Template
%x    ReadArgs
%x    Operator
%x    FuncPtr
%x    EndTemplate
%x    StripTempArgs
%x    SkipSharp
%x    ReadExceptions

%%

<Start>"operator"/({B}*"["{B}*"]")*    {
      // operator rule must be before {ID} rule

      s_name += QString::fromUtf8(yytext);
      BEGIN(Operator);
   }

<Start>{ID}{B}*"("{B}*{ID}{B}*")" { 
      // Objective-C class categories

      if (! insideObjC) {
         REJECT; 
      } else {                  
         s_name += QString::fromUtf8(yytext);
      }
   }

<Start>([~!]{B}*)?{ID}/({B}*"["{B}*"]")* { 
      // the []'s are for Java, 
      // the / was add to deal with multi-
      // dimensional C++ arrays like A[][15]
      // the leading ~ is for a destructor
      // the leading ! is for a C++/CLI finalizer (see bug 456475 and 635198)

      QString text = QString::fromUtf8(yytext);

      addTypeName();
      s_name += text;
   }

<Start>{B}*"::"{B}*     { 
      // found a scope specifier

      if (! s_scope.isEmpty()) {
         s_scope += "::" + s_name;     // add name to scope

      } else {
         s_scope = s_name;             // scope becomes name
      }

      s_name.resize(0);
   }

<Start>{B}*":"          { 
      // Objective-C argument separator
      s_name += QString::fromUtf8(yytext);
   }

<Start>[*&]+         {
      addType();
      s_type += QString::fromUtf8(yytext);
   }

<Start>{B}+          {
      addType();
   }

<Start>{B}*"("({ID}"::")*{B}*[&*]({B}*("const"|"volatile"){B}+)?  {
      addType();
      s_type += QString::fromUtf8(yytext).trimmed();
   }

<Start>{B}*")"          {
      s_type += ")";
   }

<Start>{B}*"("          { 
      // TODO: function pointers
      s_args += "(";
      BEGIN(ReadArgs);
   }

<Start>{B}*"["          {
      s_args += "[";
      BEGIN(ReadArgs);
   }

<Start>{B}*"<"          {
      s_name     += "<";
      sharpCount  = 0;
      BEGIN(Template);
   }

<Template>"<<"          { 
      s_name += "<<"; 
   }

<Template>">>"          { 
      s_name += ">>"; 
   }

<Template>"<"        {
      s_name += "<";
      sharpCount++;
   }

<Template>">"        {
      s_name += ">";

      if (sharpCount) {
         --sharpCount;

      } else {      
         BEGIN(Start);
      }
   }

<Template>.          {
      QString text = QString::fromUtf8(yytext);
      s_name += text[0];
   }

<Operator>{B}*"("{B}*")"{B}*"<>"{B}*/"("  {
      s_name += "() <>";
      BEGIN(ReadArgs);
   }

<Operator>{B}*"("{B}*")"{B}*/"("    {
      s_name += "()";
      BEGIN(ReadArgs);
   }

<Operator>[^(]*{B}*("<>"{B}*)?/"(" {
      s_name += QString::fromUtf8(yytext);
      BEGIN(ReadArgs);
   }

<ReadArgs>"throw"{B}*"("   {
      exceptionString = "throw(";
      BEGIN(ReadExceptions);
   }

<ReadArgs>.          {
      QString text = QString::fromUtf8(yytext);
      s_args += text[0];
   }

<ReadExceptions>.       {
      QString text = QString::fromUtf8(yytext);
      exceptionString += text[0];
   }

<*>.
<*>\n

%%


void parseFuncDecl(const QString &decl, bool objC, QString &cl, QString &t,
                   QString &n, QString &a, QString &ftl, QString &exc)
{
   printlex(declinfoYY_flex_debug, TRUE, __FILE__, NULL);

   inputString   = decl;

   if (inputString == 0) {
      return;
   }

   inputPosition      = 0;
   classTempListFound = FALSE;
   funcTempListFound  = FALSE;
   insideObjC = objC;  
   className.resize(0);
   classTempList.resize(0);
   funcTempList.resize(0);

   s_scope.resize(0);
   s_name.resize(0);
   s_type.resize(0);
   s_args.resize(0);

   exceptionString.resize(0);
 
   // try to find the type, scope, name and arguments
   declinfoYYrestart( declinfoYYin );
   BEGIN( Start );
   declinfoYYlex();

   int nb = s_name.lastIndexOf('[');

   if (nb != -1 && s_args.isEmpty()) { 
      // correct for [] in name ambigity (due to Java return type allowing [])
      s_args.prepend(s_name.right(s_name.length() - nb));
      s_name = s_name.left(nb);
   }


   cl = s_scope;
   n = removeRedundantWhiteSpace(s_name);
   int il;
   int ir;

   if ((il = n.indexOf('<')) != -1 && (ir = n.lastIndexOf('>')) != -1) {
      // TODO: handle cases like where n="operator<< <T>"
   
      ftl = removeRedundantWhiteSpace(n.right(n.length() - il));
      n = n.left(il);
   }
  
   t   = removeRedundantWhiteSpace(s_type);
   a   = removeRedundantWhiteSpace(s_args);
   exc = removeRedundantWhiteSpace(exceptionString);

   if (!t.isEmpty() && t.at(t.length() - 1) == ')') { 
      // for function pointers
      a.prepend(")");
      t = t.left(t.length() - 1);
   }

   printlex(declinfoYY_flex_debug, FALSE, __FILE__, NULL);

   return;  
}
