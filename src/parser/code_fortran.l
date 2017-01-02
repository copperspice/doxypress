/*************************************************************************
 *
 * Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim
 * Copyright (C) by Anke Visser
 * Copyright (C) 1997-2014 by Dimitri van Heesch.

*************************************************************************/

%{

#include <QDir>
#include <QRegExp>
#include <QStringList>

#include <set>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>

#include <config.h>
#include <default_args.h>
#include <doxy_globals.h>
#include <entry.h>
#include <filedef.h>
#include <groupdef.h>
#include <membername.h>
#include <message.h>
#include <namespacedef.h>
#include <outputlist.h>
#include <tooltip.h>
#include <util.h>

static const int fixedCommentAfter = 72;

// Toggle for some debugging info
// #define DBG_CTX(x) fprintf x
#define DBG_CTX(x) do { } while(0)

#define YY_NEVER_INTERACTIVE 1
#define YY_NO_INPUT 1
#define YY_NO_TOP_STATE 1

/*
 * For fixed formatted code position 6 is of importance (continuation character).
 * The following variables and macros keep track of the column number
 * YY_USER_ACTION is always called for each scan action
 * YY_FTN_REST    is used to handle end of lines and reset the column counter
 * YY_FTN_REJECT  resets the column counters when a pattern is rejected and thus rescanned.
 */
int yy_old_start = 0;
int yy_my_start  = 0;
int yy_end       = 1;

#define YY_USER_ACTION {yy_old_start = yy_my_start; yy_my_start = yy_end; yy_end += yyleng;}
#define YY_FTN_RESET   {yy_old_start = 0; yy_my_start = 0; yy_end = 1;}
#define YY_FTN_REJECT  {yy_end = yy_my_start; yy_my_start = yy_old_start; REJECT;}


class UseEntry
{
  public:
   QString module;          // just for debug
   QStringList onlyNames;   // entries of the ONLY-part
};

class Compare
{
   public:
      bool operator()(const QString &a, const QString &b) const {
         return a.compare(b, Qt::CaseInsensitive) < 0;
      }
};

/**
  Contains names of used modules and names of local variables.
*/
class Scope
{
   public:
      Scope() {}

      QStringList useNames;                      // names of used modules
      std::set<QString, Compare> localVars;      // names of local variables
};


static QString     docBlock;                     // contents of all lines of a documentation bloc
static QString     currentModule;                // name of the current enclosing module
static QString     currentClass;                 // name of the current enclosing class
static UseSDict    *useMembers = new UseSDict;   // info about used modules
static UseEntry    *useEntry = 0;                // current use statement info
static QList<Scope *> scopeStack;

static QString str = "";                         // contents of fortran string

static CodeOutputInterface *g_code;

// TODO: is this still needed? if so, make it work
static QString     g_parmType;
static QString     g_parmName;

static QString     g_inputString;                // the code fragment as text
static int         g_inputPosition;              // read offset during parsing
static int         g_inputLines;                 // number of line in the code fragment
static int         g_yyLineNr;                   // current line number
static bool        g_needsTermination;

static bool        g_collectXRefs;
static bool        g_isFixedForm;

static bool        g_insideBody;                 // inside subprog/program body? => create links
static QString     g_currentFontClass;

static bool        g_exampleBlock;
static QString     g_exampleName;
static QString     g_exampleFile;

static QSharedPointer<Definition> g_searchCtx;
static QSharedPointer<FileDef>    g_sourceFileDef;
static QSharedPointer<Definition> g_currentDefinition;
static QSharedPointer<MemberDef>  g_currentMemberDef;

static bool        g_includeCodeFragment;
static QChar       stringStartSymbol;            // single or double quote

// count in variable declaration to filter out declared from referenced names
static int         bracketCount = 0;

static bool        g_endComment;

// simplified way to know if this is fixed form
// duplicate in parse_fortran.l
static bool recognizeFixedForm(const QString &contents, FortranFormat format)
{
   int column = 0;
   bool skipLine = false;

   if (format == FortranFormat_Fixed) {
      return true;
   }

   if (format == FortranFormat_Free) {
      return false;
   }

   for (int i = 0; true; i++) {
      column++;

      switch (contents[i].unicode()) {
         case '\n':
            column = 0;
            skipLine = false;
            break;

         case ' ':
            break;

         case '#':
            skipLine = true;
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

static void endFontClass()
{
   if (! g_currentFontClass.isEmpty()) {
      g_code->endFontClass();
      g_currentFontClass = "";
   }
}

static void startFontClass(const QString &s)
{
   if (g_currentFontClass != s)  {
      endFontClass();
      g_code->startFontClass(s);
      g_currentFontClass = s;
   }
}

static void setCurrentDoc(const QString &anchor)
{
   if (Doxy_Globals::searchIndexBase != nullptr) {

      if (g_searchCtx) {
         Doxy_Globals::searchIndexBase->setCurrentDoc(g_searchCtx, g_searchCtx->anchor(), false);

      } else {
         Doxy_Globals::searchIndexBase->setCurrentDoc(g_sourceFileDef, anchor, true);
      }
   }
}

static void addToSearchIndex(const QString &text)
{
   if (Doxy_Globals::searchIndexBase != nullptr) {
      Doxy_Globals::searchIndexBase->addWord(text, false);
   }
}

/*! start a new line of code, inserting a line number if g_sourceFileDef
 * is true. If a definition starts at the current line, then the line
 * number is linked to the documentation of that definition.
 */
static void startCodeLine()
{
   if (g_sourceFileDef)   {

      QSharedPointer<Definition> d = g_sourceFileDef->getSourceDefinition(g_yyLineNr);

      if (! g_includeCodeFragment && d) {

         g_currentDefinition = d;
         g_currentMemberDef = g_sourceFileDef->getSourceMember(g_yyLineNr);
         g_insideBody = false;
         g_endComment = false;
         g_parmType.resize(0);
         g_parmName.resize(0);

         QString lineAnchor;
         lineAnchor = QString("l%1").arg(g_yyLineNr, 5, QChar('0'));

         if (g_currentMemberDef) {

            g_code->writeLineNumber(g_currentMemberDef->getReference(),
                  g_currentMemberDef->getOutputFileBase(),
                  g_currentMemberDef->anchor(), g_yyLineNr);

            setCurrentDoc(lineAnchor);

         } else if (d->isLinkableInProject()) {
            g_code->writeLineNumber(d->getReference(), d->getOutputFileBase(),
                  0, g_yyLineNr);

           setCurrentDoc(lineAnchor);
         }
      } else {
         g_code->writeLineNumber(0,0,0,g_yyLineNr);
      }
   }

   g_code->startCodeLine(g_sourceFileDef);

   if (! g_currentFontClass.isEmpty()) {
      g_code->startFontClass(g_currentFontClass);
   }
}


static void endFontClass();

static void endCodeLine()
{
   endFontClass();
   g_code->endCodeLine();
}

/*! write a code fragment `text' that may span multiple lines, inserting
 * line numbers for each line.
 */
static void codifyLines(const QString &text)
{
   QString tmp;
   QString tmp_currentFontClass = g_currentFontClass;

   for (auto c : text) {

      if (c == '\n') {
         g_yyLineNr++;

         g_code->codify(tmp);
         endCodeLine();

         if (g_yyLineNr < g_inputLines) {
            startCodeLine();
         }

         if (! tmp_currentFontClass.isEmpty()) {
            startFontClass(tmp_currentFontClass);
         }

         tmp = "";

      } else {
         tmp += c;

      }
   }

   if (! tmp.isEmpty() )  {
      g_code->codify(tmp);
   }
}

/*! writes a link to a fragment \a text that may span multiple lines, inserting
 * line numbers for each line. If \a text contains newlines, the link will be
 * split into multiple links with the same destination, one for each line.
 */
static void writeMultiLineCodeLink(CodeOutputInterface &ol,
                  QSharedPointer<Definition> d, const QString &text)
{
   static bool sourceTooltips = Config::getBool("source-tooltips");
   TooltipManager::instance()->addTooltip(d);

   QString ref    = d->getReference();
   QString file   = d->getOutputFileBase();
   QString anchor = d->anchor();
   QString tooltip;

   if (! sourceTooltips) {
      // fall back to simple "title" tooltips
      tooltip = d->briefDescriptionAsTooltip();
   }

   QString tmp;

   for (auto c : text) {

      if (c == '\n') {
         g_yyLineNr++;

         ol.writeCodeLink(ref, file, anchor, tmp, tooltip);
         endCodeLine();

         if (g_yyLineNr < g_inputLines) {
            startCodeLine();
         }

         tmp = "";

      } else {
         tmp += c;

      }
   }

   if ( ! tmp.isEmpty() ) {
      ol.writeCodeLink(ref, file, anchor, tmp, tooltip);
   }
}

/**
  searches for definition of a module (Namespace)
  @param mname the name of the module
  @param cd the entry, if found or null
  @returns true, if module is found
*/
static bool getFortranNamespaceDefs(const QString &moduleName, QSharedPointer<NamespaceDef> &cd)
{
   if (moduleName.isEmpty()) {
      return false;
   }

   // search for module
   if ((cd = Doxy_Globals::namespaceSDict.find(moduleName))) {
      return true;
   }

   return false;
}

/**
  searches for definition of a type
  @param tname the name of the type
  @param moduleName name of enclosing module or null, if global entry
  @param cd the entry, if found or null
  @param useDict dictionary of data of USE-statement
  @returns true, if type is found
*/
static bool getFortranTypeDefs(const QString &tname, const QString &moduleName,
                               QSharedPointer<ClassDef> &cd, UseSDict *usedict = nullptr)
{
   if (tname.isEmpty()) {
      return false;
   }

   // search for type
   if ((cd = Doxy_Globals::classSDict.find(tname))) {
      return true;

   } else if (! moduleName.isEmpty() && (cd = Doxy_Globals::classSDict.find(moduleName + "::" + tname))) {
      return true;

   } else  {

      for (auto use : *usedict)  {
         cd = Doxy_Globals::classSDict.find(use->module + "::" + tname);

         if (cd) {
            return true;
         }
      }
   }

   return false;
}

/**
  searches for definition of function memberName
  @param memberName the name of the function/variable
  @param moduleName name of enclosing module or null, if global entry
  @param md the entry, if found or null
  @param usedict array of data of USE-statement
  @returns true, if found
*/
static bool getFortranDefs(const QString &memberName, const QString &moduleName,
                  QSharedPointer<MemberDef> &md, UseSDict *usedict = nullptr)
{
   if (memberName.isEmpty()) {
      return FALSE;
   }

   // look in local variables
   for (auto scope : scopeStack) {
      if (scope->localVars.count(memberName) != 0) {
         return false;
      }
   }

   // search for function
   QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict.find(memberName);

   if (! mn) {
      mn = Doxy_Globals::memberNameSDict.find(memberName);
   }

   if (mn) {
      // name is known

      for (auto item : *mn) {
         md = item;

         // all found functions with given name
         QSharedPointer<FileDef>  fd = md->getFileDef();
         QSharedPointer<GroupDef> gd = md->getGroupDef();
         QSharedPointer<ClassDef> cd = md->getClassDef();

         if ((gd && gd->isLinkable()) || (fd && fd->isLinkable())) {
            QSharedPointer<NamespaceDef> nspace = md->getNamespaceDef();

            if (nspace == nullptr) {
               // found function in global scope

               if (cd == nullptr) {
                  // skip if bound to type
                  return true;
               }

            } else if (moduleName == nspace->name()) {
               // found in local scope
               return true;

            } else  {
               // else search in used modules

               QString moduleName = nspace->name();
               UseEntry *ue = usedict->find(moduleName);

               if (ue)  {
                  // check if only-list exists and if current entry exists is this list

                  QStringList &only = ue->onlyNames;

                  if (only.isEmpty()) {
                     return true; // whole module used

                  } else {

                     for ( QStringList::Iterator it = only.begin(); it != only.end(); ++it) {

                        if (memberName == (*it)) {
                           return true; // found in ONLY-part of use list
                        }
                     }
                  }
               }
            }
         } // if linkable
      } // for
   }

   return false;
}

/**
 gets the link to a generic procedure which depends not on the name, but on the parameter list
 @todo implementation
*/
static bool getGenericProcedureLink(const QSharedPointer<ClassDef> cd,
                  const QString &memberText, CodeOutputInterface &ol)
{
   return false;
}


static bool getLink(UseSDict *usedict, const QString &memberText,
                  CodeOutputInterface &ol, const QString &text)
{
   QSharedPointer<MemberDef> md;
   QString memberName = removeRedundantWhiteSpace(memberText);

   if (getFortranDefs(memberName, currentModule, md, usedict) && md->isLinkable())  {
      QSharedPointer<Definition> d;

      if (md->getOuterScope() == Doxy_Globals::globalScope) {
         d = md->getBodyDef();

      } else {
         d = md->getOuterScope();

      }

      if (md->getGroupDef()) {
         d = md->getGroupDef();
      }

      if (d && d->isLinkable()) {

         if (g_currentDefinition && g_currentMemberDef &&
                  md != g_currentMemberDef && g_insideBody && g_collectXRefs) {

            addDocCrossReference(g_currentMemberDef, md);
         }

         writeMultiLineCodeLink(ol, md, ! text.isEmpty() ? text : memberText);
         addToSearchIndex(! text.isEmpty() ? text : memberText);

         return true;
      }
   }

   return false;
}

static void generateLink(CodeOutputInterface &ol, const QString &lname)
{
   QSharedPointer<ClassDef> cd;
   QSharedPointer<NamespaceDef> nd;

   QString tmp = lname;
   tmp = removeRedundantWhiteSpace(tmp.toLower());

   // check if lowercase lname is a linkable type or interface
   if ( (getFortranTypeDefs(tmp, currentModule, cd, useMembers)) && cd->isLinkable() ) {

      if ( (cd->compoundType() == CompoundType::Class) && (getGenericProcedureLink(cd, tmp, ol)) ) {
         // no code

      } else  {
         // write type or interface link
         writeMultiLineCodeLink(ol,cd,tmp);
         addToSearchIndex(tmp);
      }

   // check for module
   } else if ( (getFortranNamespaceDefs(tmp, nd)) && nd->isLinkable() ) {
      // write module link

      writeMultiLineCodeLink(ol, nd, tmp);
      addToSearchIndex(tmp);

   // check for function/variable
   } else if (getLink(useMembers, tmp, ol, tmp))  {
      // no code

   } else {
      // nothing found, just write out the word
      codifyLines(tmp);
      addToSearchIndex(tmp);
   }
}

/*! counts the number of lines in the input */
static int countLines()
{
   int count = 1;

   if (g_inputString.isEmpty() ) {
      return count;
   }

   const QChar *p = g_inputString.constData();
   QChar c;

   while ((c = *p) != 0) {
      p++ ;

      if (c == '\n') {
         count++;
      }
   }

   if (*(p - 1) != '\n') {
      // last line does not end with a \n, so we add an extra
      // line and explicitly terminate the line after parsing
      count++;
      g_needsTermination = true;
   }

   return count;
}

static void startScope()
{
  DBG_CTX((stderr, "===> startScope %s", yytext));

  Scope *scope = new Scope;
  scopeStack.append(scope);
}


static void endScope()
{
  DBG_CTX((stderr,"===> endScope %s", yytext));

  if (scopeStack.isEmpty()) {
          DBG_CTX((stderr, "WARNING: (Fortran) Stack is empty\n"));
    return;
  }

  Scope *scope = scopeStack.last();
  scopeStack.removeLast();

  for (auto it : scope->useNames ) {
    useMembers->remove(it);
  }

  delete scope;
}

static void addUse(const QString &moduleName)
{
   if (! scopeStack.isEmpty()) {
      scopeStack.last()->useNames.append(moduleName);
   }
}

static void addLocalVar(const QString &varName)
{
   if (! scopeStack.isEmpty()) {
      scopeStack.last()->localVars.insert(varName);
   }
}

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

IDSYM     [a-z_A-Z0-9]
ID        [a-z_A-Z]+{IDSYM}*
SUBPROG   (subroutine|function)
B         [ \t]
BS        [ \t]*
BS_       [ \t]+
COMMA     {BS},{BS}
ARGS_L0   ("("[^)]*")")
ARGS_L1a  [^()]*"("[^)]*")"[^)]*
ARGS_L1   ("("{ARGS_L1a}*")")
ARGS_L2   "("({ARGS_L0}|[^()]|{ARGS_L1a}|{ARGS_L1})*")"
ARGS      {BS}({ARGS_L0}|{ARGS_L1}|{ARGS_L2})

NUM_TYPE  (complex|integer|logical|real)
LOG_OPER  (\.and\.|\.eq\.|\.eqv\.|\.ge\.|\.gt\.|\.le\.|\.lt\.|\.ne\.|\.neqv\.|\.or\.|\.not\.)
KIND      {ARGS}
CHAR      (CHARACTER{ARGS}?|CHARACTER{BS}"*"({BS}[0-9]+|{ARGS}))

TYPE_SPEC    (({NUM_TYPE}({BS}"*"{BS}[0-9]+)?)|({NUM_TYPE}{KIND})|DOUBLE{BS}COMPLEX|DOUBLE{BS}PRECISION|{CHAR}|TYPE|CLASS|PROCEDURE)

INTENT_SPEC  intent{BS}"("{BS}(in|out|in{BS}out){BS}")"

ATTR_SPEC    (IMPLICIT|ALLOCATABLE|DIMENSION{ARGS}|EXTERNAL|{INTENT_SPEC}|INTRINSIC|OPTIONAL|PARAMETER|POINTER|PROTECTED|PRIVATE|PUBLIC|SAVE|TARGET|RECURSIVE|PURE|IMPURE|ELEMENTAL|VALUE|NOPASS|DEFERRED|CONTIGUOUS|VOLATILE)

ACCESS_SPEC  (PROTECTED|PRIVATE|PUBLIC)

/* assume that attribute statements are almost the same as attributes. */
ATTR_STMT    {ATTR_SPEC}|DIMENSION
FLOW         (DO|SELECT|CASE|SELECT{BS}(CASE|TYPE)|WHERE|IF|THEN|ELSE|WHILE|FORALL|ELSEWHERE|ELSEIF|RETURN|CONTINUE|EXIT|GO{BS}TO)

COMMANDS     (FORMAT|CONTAINS|MODULE{BS_}PROCEDURE|WRITE|READ|ALLOCATE|ALLOCATED|ASSOCIATED|PRESENT|DEALLOCATE|NULLIFY|SIZE|INQUIRE|OPEN|CLOSE|FLUSH|DATA|COMMON)

IGNORE       (CALL)
PREFIX       (RECURSIVE{BS_}|IMPURE{BS_}|PURE{BS_}|ELEMENTAL{BS_}){0,3}(RECURSIVE|IMPURE|PURE|ELEMENTAL)?

%option case-insensitive
%option noyywrap
%option stack
%option caseless


%x Start
%x SubCall
%x FuncDef
%x ClassName
%x ClassVar
%x Subprog
%x DocBlock
%x Use
%x UseOnly
%x Import
%x Declaration
%x DeclarationBinding
%x DeclContLine
%x Parameterlist
%x String
%x Subprogend

%%

 /*-------- ignore ------------------------------------------------------------*/

<Start>{IGNORE}/{BS}"("                 {
      // do not search keywords, intrinsics... TODO: complete list
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

 /*-------- inner construct ---------------------------------------------------*/

<Start>{COMMANDS}/{BS}[,( \t\n]         {
      // highlight, font class is defined in css */

      QString text = QString::fromUtf8(yytext);
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
   }

<Start>{FLOW}/{BS}[,( \t\n]               {
      QString text = QString::fromUtf8(yytext);

      if (g_isFixedForm) {
         if ((yy_my_start == 1) && ((text[0] == 'c') || (text[0] == 'C'))) {
            YY_FTN_REJECT;
         }
      }

      /* font class is defined css */
      startFontClass("keywordflow");
      codifyLines(text);
      endFontClass();
   }

<Start>{BS}(CASE|CLASS|TYPE){BS_}(IS|DEFAULT) {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keywordflow");
      codifyLines(text);
      endFontClass();
   }

<Start>{BS}"end"({BS}{FLOW})/[ \t\n]       {
      // list is a bit long as not all have possible end
      QString text = QString::fromUtf8(yytext);

      startFontClass("keywordflow");
      codifyLines(text);
      endFontClass();
   }

<Start>"implicit"{BS}("none"|{TYPE_SPEC})  {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keywordtype");
      codifyLines(text);
      endFontClass();
   }

<Start>^{BS}"namelist"/[//]             {
      // Namelist specification
      QString text = QString::fromUtf8(yytext);

      startFontClass("keywordtype");
      codifyLines(text);
      endFontClass();
   }

 /*-------- use statement -------------------------------------------*/
<Start>"use"{BS_}                       {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keywordtype");
      codifyLines(text);
      endFontClass();

      yy_push_state(YY_START);
      BEGIN(Use);

   }

<Use>{ID}                               {
      QString text = QString::fromUtf8(yytext);
      QString tmp = text.toLower();

      g_insideBody = true;
      generateLink(*g_code, text);
      g_insideBody = false;

      /* append module name to use dict */
      useEntry = new UseEntry();

      // useEntry->module = text;
      // useMembers->append(yytext, useEntry);
      // addUse(text);

      useEntry->module = tmp;
      useMembers->insert(tmp, useEntry);
      addUse(tmp);
   }

<Use>,{BS}"ONLY"                        {
      // TODO: rename
      QString text = QString::fromUtf8(yytext);

      startFontClass("keywordtype");
      codifyLines(text);
      endFontClass();
      yy_push_state(YY_START);
      BEGIN(UseOnly);
   }

<UseOnly,Import>{BS},{BS}               {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

<UseOnly,Import>{BS}&{BS}"\n"           {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
      YY_FTN_RESET
   }

<UseOnly>{ID}                           {
      QString text = QString::fromUtf8(yytext);

      useEntry->onlyNames.append(text.toLower());
      g_insideBody = true;
      generateLink(*g_code, text);
      g_insideBody = false;
   }

<Use,UseOnly,Import>"\n"                {
      unput(*yytext);
      yy_pop_state();
      YY_FTN_RESET
   }

<Start>"import"{BS_}                    {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keywordtype");
      codifyLines(text);
      endFontClass();
      yy_push_state(YY_START);
      BEGIN(Import);
   }

<Import>{ID}                            {
      QString text = QString::fromUtf8(yytext);
      g_insideBody=  true;
      generateLink(*g_code, text);
      g_insideBody = false;
   }

 /*-------- fortran module  -----------------------------------------*/
<Start>("block"{BS}"data"|"program"|"module"|"interface")/{BS_}|({COMMA}{ACCESS_SPEC})|\n {
      QString text = QString::fromUtf8(yytext);

      startScope();
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
      yy_push_state(YY_START);
      BEGIN(ClassName);

      if (text == "module") {
         currentModule = "module";
      }
   }

<Start>("type")/{BS_}|({COMMA}({ACCESS_SPEC}|ABSTRACT|EXTENDS))|\n {
      QString text = QString::fromUtf8(yytext);

      startScope();
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
      yy_push_state(YY_START);
      BEGIN(ClassName);
      currentClass = "class";
   }

<ClassName>{ID}                          {
      QString text = QString::fromUtf8(yytext);

      if (currentModule == "module") {
         currentModule = text;
         currentModule = currentModule.toLower();
      }

      generateLink(*g_code, text);
      yy_pop_state();
   }
<ClassName>({ACCESS_SPEC}|ABSTRACT|EXTENDS)/[,:( ] {
      QString text = QString::fromUtf8(yytext);

      // variable declaration
      startFontClass("keyword");
      g_code->codify(text);
      endFontClass();
   }

<ClassName>\n           {
      // interface may be without name
      yy_pop_state();
      YY_FTN_REJECT;
   }

<Start>^{BS}"end"({BS_}"type").*        {
      // just reset currentClass, rest is done in following rule
      currentClass = "";
      YY_FTN_REJECT;
   }

<Start>^{BS}"end"({BS_}"module").*      {
      // just reset currentModule, rest is done in following rule
      currentModule = "";
      YY_FTN_REJECT;
   }

 /*-------- subprog definition -------------------------------------*/
<Start>({PREFIX}{BS_})?{TYPE_SPEC}{BS_}({PREFIX}{BS_})?{BS}/{SUBPROG}{BS_}  {
      // TYPE_SPEC is for old function style function result
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
   }

<Start>({PREFIX}{BS_})?{SUBPROG}{BS_}                  {
      // Fortran subroutine or function found
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
      yy_push_state(YY_START);
      BEGIN(Subprog);
   }

<Subprog>{ID}                           {
      // subroutine/function name
      QString text = QString::fromUtf8(yytext);

      DBG_CTX((stderr, "===> start subprogram %s\n", text));

      startScope();
      generateLink(*g_code, text);
   }

<Subprog>"result"/{BS}"("[^)]*")"       {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
   }

<Subprog>"("[^)]*")"                    {
      // ignore rest of line
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

<Subprog,Subprogend>"\n"                {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
      yy_pop_state();
      YY_FTN_RESET
   }

<Start>^{BS}"end"{BS}("block"{BS}"data"|{SUBPROG}|"module"|"program"|"type"|"interface")?{BS}     {
      // Fortran subroutine or function ends
      QString text = QString::fromUtf8(yytext);

      endScope();
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
      yy_push_state(YY_START);
      BEGIN(Subprogend);
   }

<Subprogend>{ID}/{BS}(\n|!)             {
      QString text = QString::fromUtf8(yytext);
      generateLink(*g_code, text);
      yy_pop_state();
   }

<Start>^{BS}"end"{BS}("block"{BS}"data"|{SUBPROG}|"module"|"program"|"type"|"interface"){BS}/(\n|!) {
      // Fortran subroutine or function ends
      QString text = QString::fromUtf8(yytext);
      endScope();
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
   }

 /*-------- variable declaration ----------------------------------*/
<Start>{TYPE_SPEC}/[,:( ]               {
      QString text = QString::fromUtf8(yytext);

      yy_push_state(YY_START);
      BEGIN(Declaration);
      startFontClass("keywordtype");
      g_code->codify(text);
      endFontClass();
   }

<Start>{ATTR_SPEC}             {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keywordtype");
      g_code->codify(text);
      endFontClass();
   }

<Declaration>({TYPE_SPEC}|{ATTR_SPEC})/[,:( ] {
      // variable declaration
      QString text = QString::fromUtf8(yytext);
      startFontClass("keywordtype");
      g_code->codify(text);
      endFontClass();
    }

<Declaration>{ID}                       {
      // local var
      QString text = QString::fromUtf8(yytext);

      if (g_currentMemberDef && g_currentMemberDef->isFunction() && bracketCount == 0) {
         g_code->codify(text);
         addLocalVar(text);

      } else {
         generateLink(*g_code, text);
      }
   }

<Declaration>{BS}("=>"|"="){BS}                        {
      // Procedure binding
      QString text = QString::fromUtf8(yytext);
      BEGIN(DeclarationBinding);
      g_code->codify(text);
   }

<DeclarationBinding>{ID}                       {
      // Type bound procedure link
      QString text = QString::fromUtf8(yytext);
      generateLink(*g_code, text);
      yy_pop_state();
   }

<Declaration>[(]        {
      // start of array specification
      QString text = QString::fromUtf8(yytext);
      bracketCount++;
      g_code->codify(text);
   }

<Declaration>[)]        {
      // end array specification
      QString text = QString::fromUtf8(yytext);
      bracketCount--;
      g_code->codify(text);
   }

<Declaration,DeclarationBinding>"&"                        {
      // continuation line
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      yy_push_state(YY_START);
      BEGIN(DeclContLine);
   }

<DeclContLine>"\n"                      {
      // declaration not yet finished
      QString text = QString::fromUtf8(yytext);

      codifyLines(text);
      bracketCount = 0;
      yy_pop_state();
      YY_FTN_RESET
   }

<Declaration,DeclarationBinding>"\n"                       {
      // end declaration line
      QString text = QString::fromUtf8(yytext);

      if (g_endComment) {
         g_endComment = false;

      } else {
         codifyLines(text);

      }

      bracketCount = 0;
      yy_pop_state();
      YY_FTN_RESET
   }

 /*-------- subprog calls  -----------------------------------------*/

<Start>"call"{BS_}                      {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
      yy_push_state(YY_START);
      BEGIN(SubCall);
   }

<SubCall>{ID}                           {
      // subroutine call
      QString text = QString::fromUtf8(yytext);

      g_insideBody = true;
      generateLink(*g_code, text);
      g_insideBody = false;
      yy_pop_state();
   }

<Start>{ID}{BS}/"("                     {
      // function call
      QString text = QString::fromUtf8(yytext);

      if (g_isFixedForm && yy_my_start == 6) {
         // fixed form continuation line
         YY_FTN_REJECT;
      } else {
         g_insideBody = true;
         generateLink(*g_code, text);
         g_insideBody = false;
      }
   }

 /*-------- comments ---------------------------------------------------*/
<Start,Declaration,DeclarationBinding>\n?{BS}"!>"|"!<"                 {
      // start comment line or comment block
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '\n') {
         yy_old_start = 0;
         yy_my_start  = 1;
         yy_end = yyleng;
      }

      // Actually we should see if ! on position 6, can be continuation
      // but the chance is very unlikely, so no effort to solve it here
      yy_push_state(YY_START);
      BEGIN(DocBlock);
      docBlock = text;
   }

<Declaration,DeclarationBinding>{BS}"!<"                   {
      // start comment line or comment block
      QString text = QString::fromUtf8(yytext);

      yy_push_state(YY_START);
      BEGIN(DocBlock);
      docBlock = text;
   }

<DocBlock>.*            {
      // contents of current comment line
      QString text = QString::fromUtf8(yytext);
      docBlock += text;
   }

<DocBlock>"\n"{BS}("!>"|"!<"|"!!")  {
      // comment block (next line is also comment line)
      QString text = QString::fromUtf8(yytext);

      yy_old_start = 0;
      yy_my_start  = 1;
      yy_end = yyleng;

      // we should see if ! on position 6, can be continuation
      // but the chance is very unlikely, so no effort to solve it here
      docBlock += text;
   }

<DocBlock>"\n"                {
      // comment block ends at the end of this line
      // remove special comment (default config)

      static const bool stripCodeComments = Config::getBool("strip-code-comments");

      if (stripCodeComments) {
         g_yyLineNr += docBlock.count('\n');
         g_yyLineNr += 1;
         endCodeLine();

         if (g_yyLineNr < g_inputLines) {
            startCodeLine();
         }

         g_endComment = true;

      } else {
         // do not remove comment

         startFontClass("comment");
         codifyLines(docBlock);
         endFontClass();
      }

      unput(*yytext);
      yy_pop_state();
      YY_FTN_RESET
   }

<*>"!"[^><\n].*|"!"$          {
      // normal comment
      QString text = QString::fromUtf8(yytext);

      if (YY_START == String) {
         YY_FTN_REJECT; // ignore in strings
      }

      if (g_isFixedForm && yy_my_start == 6) {
         YY_FTN_REJECT;
      }

      startFontClass("comment");
      codifyLines(text);
      endFontClass();
   }

<*>^[Cc*].*                   {
      // normal comment
      QString text = QString::fromUtf8(yytext);

      if (! g_isFixedForm) {
         YY_FTN_REJECT;
      }

      startFontClass("comment");
      codifyLines(text);
      endFontClass();
   }

<*>"assignment"/{BS}"("{BS}"="{BS}")"   {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
   }

<*>"operator"/{BS}"("[^)]*")"           {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
   }

 /*------ preprocessor  --------------------------------------------*/
<Start>"#".*\n                          {
      QString text = QString::fromUtf8(yytext);

      if (g_isFixedForm && yy_my_start == 6) {
         YY_FTN_REJECT;
      }

      startFontClass("preprocessor");
      codifyLines(text);
      endFontClass();
      YY_FTN_RESET
   }

 /*------ variable references?  -------------------------------------*/

<Start>"%"{BS}{ID}         {
      // ignore references to elements
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<Start>{ID}                             {
      QString text = QString::fromUtf8(yytext);

      g_insideBody = true;
      generateLink(*g_code, text);
      g_insideBody = false;
   }

 /*------ strings --------------------------------------------------*/
<*>"\\\\"                               {
      QString text = QString::fromUtf8(yytext);
      str += text; /* ignore \\  */
   }

<*>"\\\""|\\\'                          {
      QString text = QString::fromUtf8(yytext);
      str += text; /* ignore \"  */
   }

<String>\n                              {
      QString text = QString::fromUtf8(yytext);

      // string with \n inside
      str += text;

      startFontClass("stringliteral");
      codifyLines(str);
      endFontClass();

      str = "";
      YY_FTN_RESET
   }

<String>\"|\'                           {
      // string ends with next quote without previous backspace
      QString text = QString::fromUtf8(yytext);

      if(text[0] != stringStartSymbol) {
         YY_FTN_REJECT; // single vs double quote
      }

      str += text;
      startFontClass("stringliteral");
      codifyLines(str);
      endFontClass();
      yy_pop_state();
   }

<String>.                               {
      QString text = QString::fromUtf8(yytext);
      str += text;
   }

<*>\"|\'                                {
      /* string starts */
      /* if(YY_START == StrIgnore) YY_FTN_REJECT; // ignore in simple comments */
      QString text = QString::fromUtf8(yytext);

      if (g_isFixedForm && yy_my_start == 6) {
         YY_FTN_REJECT;
      }

      yy_push_state(YY_START);
      stringStartSymbol = text[0];    // single or double quote
      BEGIN(String);
      str = text;
   }

<*>\n                {
      QString text = QString::fromUtf8(yytext);

      if (g_endComment) {
         g_endComment = false;
      } else {
         codifyLines(text);
      }

      YY_FTN_RESET
   }

<*>^{BS}"type"{BS}"="                   {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<*>.                                    {
      QString text = QString::fromUtf8(yytext);

      if (g_isFixedForm && yy_my_start > fixedCommentAfter) {
         // yy_push_state(YY_START);
         // BEGIN(DocBlock);
         // docBlock = text;

         startFontClass("comment");
         codifyLines(text);

      } else {
         g_code->codify(text);
      }
   }

<*>{LOG_OPER}                           {
      // Fortran logical comparison keywords
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<*><<EOF>>                              {
      static const bool stripCodeComments = Config::getBool("strip-code-comments");

      if (YY_START == DocBlock) {

         if (! stripCodeComments) {
            startFontClass("comment");
            codifyLines(docBlock);
            endFontClass();
         }
      }
      yyterminate();
   }

%%

void resetFortranCodeParserState() {}

void parseFortranCode(CodeOutputInterface &od, const QString &className, const QString &s,
                  bool exBlock, const QString &exName, QSharedPointer<FileDef> fd, int startLine,
                  int endLine, bool inlineFragment, QSharedPointer<MemberDef> memberDef, bool,
                  QSharedPointer<Definition> searchCtx, bool collectXRefs, FortranFormat format)
{
   if (s.isEmpty()) {
      return;
   }

  printlex(yy_flex_debug, true, __FILE__, fd ? qPrintable(fd->fileName()) : "");

   TooltipManager::instance()->clearTooltips();
   g_code = &od;

   g_inputString      = s;
   g_inputPosition    = 0;
   g_isFixedForm      = recognizeFixedForm(s, format);
   g_currentFontClass = "";
   g_needsTermination = false;

  g_searchCtx         = searchCtx;
  g_collectXRefs      = collectXRefs;

  if (startLine != -1) {
      g_yyLineNr    = startLine;
   } else {
      g_yyLineNr    = 1;
   }

   if (endLine != -1) {
      g_inputLines  = endLine + 1;
   } else {
      g_inputLines  = countLines();
   }

   g_exampleBlock  = exBlock;
   g_exampleName   = exName;
   g_sourceFileDef = fd;

   if (exBlock && fd == 0) {
      // create a dummy filedef for the example
      g_sourceFileDef = QMakeShared<FileDef>("", exName);
   }

   if (g_sourceFileDef) {
      setCurrentDoc("l00001");
   }

   g_currentDefinition = QSharedPointer<Definition>();
   g_currentMemberDef  = QSharedPointer<MemberDef>();

   if (! g_exampleName.isEmpty()) {
      g_exampleFile = convertNameToFile_X(g_exampleName + "-example");
   }

   g_includeCodeFragment = inlineFragment;
   startCodeLine();
   g_parmName.resize(0);
   g_parmType.resize(0);

   yyrestart( yyin );
   BEGIN( Start );
   yylex();

   if (g_needsTermination) {
      endFontClass();
      g_code->endCodeLine();
   }

   if (fd) {
      TooltipManager::instance()->writeTooltips(*g_code);
   }

   if (exBlock && g_sourceFileDef) {
      // delete the temporary file definition used for this example
      g_sourceFileDef = QSharedPointer<FileDef>();
   }

   printlex(yy_flex_debug, false, __FILE__, fd ? fd->fileName(): NULL);

   return;
}
