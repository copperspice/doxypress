
/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * written by Weston Thayer

*************************************************************************/

%{

#include <QString>

#include <stdio.h>

#include "code_xml.h"
// #include "parse_xml.h"

#include <config.h>
#include <doxy_globals.h>
#include <entry.h>
#include <outputlist.h>
#include <tooltip.h>
#include <util.h>

#define YY_NO_INPUT 1
#define YY_NEVER_INTERACTIVE 1

static CodeOutputInterface *s_code;

static QString       s_curClassName;

static QString       s_parmType;
static QString       s_parmName;

static QString       s_inputString;        // the code fragment as text
static int           s_inputPosition;      // read offset during parsing

static int           s_inputLines;         // number of line in the code fragment
static int	         s_yyColNr;            // current column number
static int           s_yyLineNr;           // current line number
static bool          s_needsTermination;

static bool          s_includeCodeFragment;
static QString       s_currentFontClass;

static bool          s_exampleBlock;
static QString       s_exampleName;
static QString       s_exampleFile;

static QString       s_type;
static QString       s_name;
static QString       s_args;

static QString       s_classScope;
static QString       s_CurrScope;

static QSharedPointer<Definition>    s_searchCtx;
static QSharedPointer<Definition>    s_currentDefinition;
static QSharedPointer<FileDef>       s_sourceFileDef;
static QSharedPointer<MemberDef>     s_currentMemberDef;

static void codify(const QString &text)
{
  s_code->codify(text);
}

static void setCurrentDoc(const QString &anchor)
{
   if (Doxy_Globals::searchIndexBase != nullptr) {
      if (s_searchCtx) {
         s_code->setCurrentDoc(s_searchCtx, s_searchCtx->anchor(), false);
      } else {
         s_code->setCurrentDoc(s_sourceFileDef, anchor, true);
      }
   }
}

/* start a new line of code, inserting a line number if s_sourceFileDef is TRUE.
 * If a definition starts at the current line, then the line number is linked to
 * the documentation of that definition.
 */
static void startCodeLine()
{
   if (s_sourceFileDef)  {
      QSharedPointer<Definition> d  = s_sourceFileDef->getSourceDefinition(s_yyLineNr);

      if (! s_includeCodeFragment && d && d->isLinkableInProject()) {
         s_currentDefinition = d;
         s_currentMemberDef  = s_sourceFileDef->getSourceMember(s_yyLineNr);

         s_classScope = d->name();


         QString lineAnchor = QString("l%1").formatArg(s_yyLineNr, 5, 10, QChar('0'));

         if (s_currentMemberDef) {
            s_code->writeLineNumber(s_currentMemberDef->getReference(),
                                    s_currentMemberDef->getOutputFileBase(),
                                    s_currentMemberDef->anchor(), s_yyLineNr);

            setCurrentDoc(lineAnchor);

         } else {
           s_code->writeLineNumber(d->getReference(), d->getOutputFileBase(), 0, s_yyLineNr);
           setCurrentDoc(lineAnchor);
         }

      } else {
         s_code->writeLineNumber(0, 0, 0, s_yyLineNr);
      }
   }

   s_code->startCodeLine(s_sourceFileDef);

   if (! s_currentFontClass.isEmpty()) {
      s_code->startFontClass(s_currentFontClass);
   }
}

static void endFontClass()
{
   if (! s_currentFontClass.isEmpty()) {
      s_code->endFontClass();
      s_currentFontClass = "";
   }
}

static void endCodeLine()
{
  endFontClass();
  s_code->endCodeLine();
}

static void nextCodeLine()
{
   QString fc = s_currentFontClass;
   endCodeLine();

   if (s_yyLineNr < s_inputLines) {
      s_currentFontClass = fc;
      startCodeLine();
   }
}

static void codifyLines(const QString &text)
{
   QString tmp;

   for (auto c : text) {

      if (c == '\n') {
         s_yyLineNr++;
         s_yyColNr = 1;

         s_code->codify(tmp);
         nextCodeLine();

         tmp = "";

      } else {
         tmp += c;
         s_yyColNr++;

      }
   }

   if (! tmp.isEmpty() )  {
      s_code->codify(tmp);
   }
}

static void startFontClass(const QString &s)
{
   endFontClass();
   s_code->startFontClass(s);
   s_currentFontClass = s;
}

// counts the number of lines in the input
static int countLines()
{
   int count = 1;

   if (s_inputString.isEmpty() ) {
      return count;
   }

   for (QChar c : s_inputString) {
      if (c == '\n') {
         ++count;
      }
   }

   if (s_inputString.last() != '\n') {
      // last line does not end with a \n, add extra line and explicitly terminate the line after parsing
      ++count;
      s_needsTermination = true;
   }

   return count;
}

#undef   YY_INPUT
#define  YY_INPUT(buf,result,max_size) result=yyread(buf,max_size);

static int yyread(char *buf, int max_size)
{
  int len = max_size;

   QString tmp1    = s_inputString.mid(s_inputPosition, max_size);
   QByteArray tmp2 = tmp1.toUtf8();

   while(len > 0 && tmp2.size() > len) {
     len = len / 2;

     tmp1.truncate(len);
     tmp2 = tmp1.toUtf8();
   };

   s_inputPosition += len;
   memcpy(buf, tmp2.constData(), tmp2.size());

   return tmp2.size();
}

%}

nl          (\r\n|\r|\n)
ws          [ \t]+
open        "<"
close       ">"
namestart   [A-Za-z\200-\377_]
namechar    [:A-Za-z\200-\377_0-9.-]
esc         "&#"[0-9]+";"|"&#x"[0-9a-fA-F]+";"
name        {namestart}{namechar}*
comment     {open}"!--"([^-]|"-"[^-])*"--"{close}
data        "random string"
string      \"([^"&]|{esc})*\"|\'([^'&]|{esc})*\'

%option noyywrap
%option nounput

%%

<INITIAL>{ws}       {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

<INITIAL>"/"        {
      QString text = QString::fromUtf8(yytext);
      endFontClass();
      codify(text);
   }

<INITIAL>"="        {
      QString text = QString::fromUtf8(yytext);
      endFontClass();
      codify(text);
   }

<INITIAL>{close}    {
      QString text = QString::fromUtf8(yytext);
      endFontClass();
      codify(text);
   }

<INITIAL>{name}     {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keyword");
      codify(text);
      endFontClass();
   }

<INITIAL>{string}   {
      QString text = QString::fromUtf8(yytext);
      startFontClass("stringliteral");
      codifyLines(text);
      endFontClass();
   }

{open}{ws}?{name}   {
      // Write the < in a different color
      QString text = QString::fromUtf8(yytext);

      QString openBracket = text.left(1);
      codify(openBracket);

      // write the rest
      text = text.mid(1);

      startFontClass("keywordtype");
      codify(text);
      endFontClass();

      BEGIN(INITIAL);
   }

{open}{ws}?"/"{name} {
      // write the "</" in a different color
      QString text = QString::fromUtf8(yytext);

      QString closeBracket = text.left(2);
      endFontClass();
      codify(closeBracket);

      // then write the rest
      text = text.mid(2);       // skip the '</'

      startFontClass("keywordtype");
      codify(text);
      endFontClass();

      BEGIN(INITIAL);
   }

{comment}           {
      QString text = QString::fromUtf8(yytext);
      startFontClass("comment");
      codifyLines(text);
      endFontClass();
   }

{nl}                {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

.                   {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

%%

void parseXmlCode(CodeOutputInterface &outputX, const QString &className, const QString &s,
                SrcLangExt lang, bool exBlock, const QString &exName, QSharedPointer<FileDef> fd,
                int startLine, int endLine, bool inlineFragment,
                QSharedPointer<MemberDef> memberDef, bool showLineNumbers,
                QSharedPointer<Definition> searchCtx, bool collectXRefs)
{
   if (s.isEmpty()) {
      return;
   }

   TooltipManager::instance()->clearTooltips();

   s_code             = &outputX;
   s_inputString      = s;
   s_inputPosition    = 0;
   s_currentFontClass = "";
   s_needsTermination = false;
   s_searchCtx        = searchCtx;

   if (startLine != -1) {
      s_yyLineNr    = startLine;
   } else {
      s_yyLineNr    = 1;
   }

   if (endLine != -1) {
    s_inputLines  = endLine + 1;
   } else {
    s_inputLines  = s_yyLineNr + countLines() - 1;
   }

   s_exampleBlock  = exBlock;
   s_exampleName   = exName;
   s_sourceFileDef = fd;

   bool cleanupSourceDef = FALSE;

   if (exBlock && fd == nullptr)  {
      // create a dummy filedef for the example

      s_sourceFileDef  = QMakeShared<FileDef>("", (! exName.isEmpty() ? exName : "generated"));
      cleanupSourceDef = true;
   }

   if (s_sourceFileDef) {
      setCurrentDoc("l00001");
   }

   s_includeCodeFragment = inlineFragment;

   startCodeLine();

   yyrestart( yyin );
   yylex();

   if (s_needsTermination)  {
      endCodeLine();
   }

   if (fd) {
      TooltipManager::instance()->writeTooltips(*s_code);
   }

  if (cleanupSourceDef) {
    // delete the temporary file definition used for this example
    s_sourceFileDef = QSharedPointer<FileDef>();
  }

  return;
}

void resetXmlCodeParserState()
{
  s_currentDefinition = QSharedPointer<Definition>();
  s_currentMemberDef  = QSharedPointer<MemberDef>();
}

