/*************************************************************************
 *
 * Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
 * Copyright (c) 1997-2014 Dimitri van Heesch
 * Copyright (c) Anke Visser

*************************************************************************/

%{

#include <QDir>
#include <QRegularExpression>
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

// Toggle for some debugging info
// #define DBG_CTX(x) fprintf x
#define DBG_CTX(x) do { } while(0)

#define YY_NO_INPUT 1

/*
 * For fixed formatted code position 6 is of importance (continuation character).
 * The following variables and macros keep track of the column number
 * YY_USER_ACTION is always called for each scan action
 * YY_FTN_RESET   is used to handle end of lines and reset the column counter
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
   QString module;             // just for debug
   QStringList onlyNames;      // entries of the ONLY-part
};

class Compare
{
 public:
   bool operator()(const QString &a, const QString &b) const {
      return a.compare(b, Qt::CaseInsensitive) < 0;
   }
};

// contains names of used modules and names of local variables
class Scope
{
 public:
   QStringList useNames;                      // names of used modules
   std::set<QString, Compare> localVars;      // names of local variables
   std::set<QString, Compare> externalVars;   // names of external variables
};

static const int fixedCommentAfter = 72;

static QString      docBlock;                     // contents of all lines of a documentation block
static QString      currentModule;                // name of the current enclosing module
static QString      currentClass;                 // name of the current enclosing class

static UseSDict     *useMembers = nullptr;        // info about used modules
static UseEntry     *useEntry   = nullptr;        // current use statement info

static QString      s_outputStr = QString();      // contents of Fortran string

static CodeGenerator *s_code;

// TODO: is this still needed? if so, make it work
static QString      s_parmType;
static QString      s_parmName;

static QString      s_inputString;                // the code fragment as text
static int          s_inputPosition;              // read offset during parsing
static int          s_inputLines;                 // number of line in the code fragment
static int          s_yyLineNr;                   // current line number

static int          s_contLineNr;                // current, local, line number for continuation determination
static QVector<int> s_hasContLine;               // does a line have a continuation line (fixed source)

static bool         s_needsTermination;
static bool         s_collectXRefs;
static bool         s_isFixedForm;
static bool         s_isExternal;

static bool         s_insideBody;                 // inside subprog/program body? => create links
static QString      s_currentFontClass;

static bool         s_exampleBlock;
static QString      s_exampleName;
static QString      s_exampleFile;

static bool        s_includeCodeFragment;
static QChar       s_stringStartSymbol;          // single or double quote

static int         s_bracketCount = 0;           // count in var declaration to filter out declared from referenced names
static int         s_inTypeDecl   = 0;           // signal when in type / class /procedure declaration
static bool        s_endComment;

static QSharedPointer<Definition> s_searchCtx;
static QSharedPointer<FileDef>    s_sourceFileDef;
static QSharedPointer<Definition> s_currentDefinition;
static QSharedPointer<MemberDef>  s_currentMemberDef;

static QList<Scope *>             s_scopeStack;

// simplified way to know if this is fixed form
bool recognizeFixedForm(const QString &contents, FortranFormat format);
QString prepassFixedForm(const QString &contents, QVector<int> &hasContLine);

static void endFontClass()
{
   if (! s_currentFontClass.isEmpty()) {
      s_code->endFontClass();
      s_currentFontClass = "";
   }
}

static void startFontClass(const QString &s)
{
   if (s_currentFontClass != s)  {
      endFontClass();
      s_code->startFontClass(s);
      s_currentFontClass = s;
   }
}

static void setCurrentDoc(const QString &anchor)
{
   if (Doxy_Globals::searchIndexBase != nullptr) {

      if (s_searchCtx) {
         Doxy_Globals::searchIndexBase->setCurrentDoc(s_searchCtx, s_searchCtx->anchor(), false);

      } else {
         Doxy_Globals::searchIndexBase->setCurrentDoc(s_sourceFileDef, anchor, true);
      }
   }
}

static void addToSearchIndex(const QString &text)
{
   if (Doxy_Globals::searchIndexBase != nullptr) {
      Doxy_Globals::searchIndexBase->addWord(text, false);
   }
}

/* start a new line of code, inserting a line number if s_sourceFileDef
 * is true. If a definition starts at the current line, then the line
 * number is linked to the documentation of that definition.
 */
static void startCodeLine()
{
   if (s_sourceFileDef)   {

      QSharedPointer<Definition> d = s_sourceFileDef->getSourceDefinition(s_yyLineNr);

      if (! s_includeCodeFragment && d) {

         s_currentDefinition = d;
         s_currentMemberDef  = s_sourceFileDef->getSourceMember(s_yyLineNr);
         s_insideBody = false;
         s_endComment = false;
         s_parmType.resize(0);
         s_parmName.resize(0);

         QString lineAnchor;
         lineAnchor = QString("l%1").formatArg(s_yyLineNr, 5, 10, QChar('0'));

         if (s_currentMemberDef) {
            s_code->writeLineNumber(s_currentMemberDef->getReference(), s_currentMemberDef->getOutputFileBase(),
                  s_currentMemberDef->anchor(), s_yyLineNr);

            setCurrentDoc(lineAnchor);

         } else if (d->isLinkableInProject()) {
            s_code->writeLineNumber(d->getReference(), d->getOutputFileBase(), QString(), s_yyLineNr);

           setCurrentDoc(lineAnchor);
         }

      } else {
         s_code->writeLineNumber(QString(), QString(), QString(), s_yyLineNr);
      }
   }

   s_code->startCodeLine(s_sourceFileDef != nullptr);

   if (! s_currentFontClass.isEmpty()) {
      s_code->startFontClass(s_currentFontClass);
   }
}


static void endFontClass();

static void endCodeLine()
{
   endFontClass();
   s_code->endCodeLine();
}

// write a code fragment `text' that may span multiple lines, inserting line numbers for each line
static void codifyLines(const QString &text)
{
   QString tmp;
   QString tmp_currentFontClass = s_currentFontClass;

   for (auto c : text) {

      if (c == '\n') {
         s_yyLineNr++;

         s_code->codify(tmp);
         endCodeLine();

         if (s_yyLineNr < s_inputLines) {
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
      s_code->codify(tmp);
   }
}

/* writes a link to a fragment \a text that may span multiple lines, inserting
 * line numbers for each line. If \a text contains newlines, the link will be
 * split into multiple links with the same destination, one for each line.
 */
static void writeMultiLineCodeLink(CodeGenerator &ol, QSharedPointer<Definition> d, const QString &text)
{
   static const bool sourceTooltips = Config::getBool("source-tooltips");
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
         s_yyLineNr++;

         ol.writeCodeLink(ref, file, anchor, tmp, tooltip);
         endCodeLine();

         if (s_yyLineNr < s_inputLines) {
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

//   searches for definition of a module (Namespace)
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

// searches for definition of a type
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
static QSharedPointer<MemberDef> getFortranDefs(const QString &memberName, const QString &moduleName, UseSDict *usedict = nullptr)
{
   if (memberName.isEmpty()) {
      return QSharedPointer<MemberDef>();
   }

   // look in local variables
   for (auto scope : s_scopeStack) {
      QString lowerMemberName = memberName.toLower();

      if ((scope->localVars.count(memberName) != 0) && (scope->externalVars.count(lowerMemberName) == 0)) {
         return QSharedPointer<MemberDef>();
      }
   }

   // search for function
   QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict.find(memberName);

   if (mn == nullptr) {
      mn = Doxy_Globals::memberNameSDict.find(memberName);
   }

   if (mn != nullptr) {
      // name is known

      for (const auto &md : *mn) {
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
                  return md;
               }

            } else if (moduleName == nspace->name()) {
               // found in local scope
               return md;

            } else  {
               // else search in used modules
               QString usedModuleName = nspace->name();
               UseEntry *ue = usedict->find(usedModuleName);

               if (ue != nullptr)  {
                  // check if only-list exists and if current entry exists is this list
                  QStringList &only = ue->onlyNames;

                  if (only.isEmpty()) {
                     // whole module used
                     return md;

                  } else {
                     for ( const auto &item : only) {
                        if (memberName == item) {
                           // found in ONLY-part of use list
                           return md;
                        }
                     }
                  }
               }
            }
         }
      }
   }

   return QSharedPointer<MemberDef>();
}

/**
 gets the link to a generic procedure which depends not on the name, but on the parameter list
 todo: implementation
*/
static bool getGenericProcedureLink(const QSharedPointer<ClassDef> cd,
            const QString &memberText, CodeGenerator &ol)
{
   (void) cd;
   (void) memberText;
   (void) ol;

   return false;
}

static bool getLink(UseSDict *usedict, const QString &memberText, CodeGenerator &ol, const QString &text)
{

   QString memberName = removeRedundantWhiteSpace(memberText);
   QSharedPointer<MemberDef> md = getFortranDefs(memberName, currentModule, usedict);

   if (md != nullptr && md->isLinkable())  {

      if (md->isVariable() && (md->getLanguage() != SrcLangExt_Fortran)) {
         // non Fortran variables are not handled at this time
         return false;
      }

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

         if (s_currentDefinition && s_currentMemberDef &&
                  md != s_currentMemberDef && s_insideBody && s_collectXRefs) {

            addDocCrossReference(s_currentMemberDef, md);
         }

         writeMultiLineCodeLink(ol, md, ! text.isEmpty() ? text : memberText);
         addToSearchIndex(! text.isEmpty() ? text : memberText);

         return true;
      }
   }

   return false;
}

static void generateLink(CodeGenerator &ol, const QString &lname)
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

static void startScope()
{
  DBG_CTX((stderr, "===> startScope %s", yytext));

  Scope *scope = new Scope;
  s_scopeStack.append(scope);
}

static void endScope()
{
  DBG_CTX((stderr,"===> endScope %s", yytext));

  if (s_scopeStack.isEmpty()) {
    DBG_CTX((stderr, "WARNING: (Fortran) Stack is empty\n"));
    return;
  }

  Scope *scope = s_scopeStack.last();
  s_scopeStack.removeLast();

  for (auto it : scope->useNames ) {
    useMembers->remove(it);
  }

  delete scope;
}

static void addUse(const QString &moduleName)
{
   if (! s_scopeStack.isEmpty()) {
      s_scopeStack.last()->useNames.append(moduleName);
   }
}

static void addLocalVar(const QString &varName)
{
   if (! s_scopeStack.isEmpty()) {
      QString lowerMemberName = varName.toLower();
      s_scopeStack.last()->localVars.insert(lowerMemberName);

      if (s_isExternal) {
         s_scopeStack.last()->externalVars.insert(lowerMemberName);
      }
   }
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

TYPE_SPEC    (({NUM_TYPE}({BS}"*"{BS}[0-9]+)?)|({NUM_TYPE}{KIND})|DOUBLE{BS}COMPLEX|DOUBLE{BS}PRECISION|{CHAR}|TYPE|CLASS|PROCEDURE|ENUMERATOR)

INTENT_SPEC  intent{BS}"("{BS}(in|out|in{BS}out){BS}")"

ATTR_SPEC    (IMPLICIT|ALLOCATABLE|DIMENSION{ARGS}|EXTERNAL|{INTENT_SPEC}|INTRINSIC|OPTIONAL|PARAMETER|POINTER|PROTECTED|PRIVATE|PUBLIC|SAVE|TARGET|(NON_)?RECURSIVE|PURE|IMPURE|ELEMENTAL|VALUE|NOPASS|DEFERRED|CONTIGUOUS|VOLATILE)

ACCESS_SPEC  (PROTECTED|PRIVATE|PUBLIC)

/* assume that attribute statements are almost the same as attributes. */
ATTR_STMT    {ATTR_SPEC}|DIMENSION
FLOW         (DO|SELECT|CASE|SELECT{BS}(CASE|TYPE)|WHERE|IF|THEN|ELSE|WHILE|FORALL|ELSEWHERE|ELSEIF|RETURN|CONTINUE|EXIT|GO{BS}TO)

COMMANDS     (FORMAT|CONTAINS|MODULE{BS_}PROCEDURE|WRITE|READ|ALLOCATE|ALLOCATED|ASSOCIATED|PRESENT|DEALLOCATE|NULLIFY|SIZE|INQUIRE|OPEN|CLOSE|FLUSH|DATA|COMMON)
IGNORE       (CALL)
PREFIX       ((NON_)?RECURSIVE{BS_}|IMPURE{BS_}|PURE{BS_}|ELEMENTAL{BS_}){0,4}((NON_)?RECURSIVE|IMPURE|PURE|ELEMENTAL)?

LANGUAGE_BIND_SPEC   BIND{BS}"("{BS}C{BS}(,{BS}NAME{BS}"="{BS}"\""(.*)"\""{BS})?")"

%option caseless
%option case-insensitive
%option never-interactive
%option nounistd
%option noyy_top_state
%option noyywrap
%option stack

%x Start
%x SubCall
%x ClassName
%x Subprog
%x DocBlock
%x Use
%x UseOnly
%x Import
%x Declaration
%x DeclarationBinding
%x DeclContLine
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

      if (s_isFixedForm) {
         if ((yy_my_start == 1) && ((text[0] == 'c') || (text[0] == 'C'))) {
            YY_FTN_REJECT;
         }
      }

      // font class is defined css
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

<Start>^{BS}"namelist"/[/]             {
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

<Use>"ONLY"                             {
      // TODO: rename
      QString text = QString::fromUtf8(yytext);

      startFontClass("keywordtype");
      codifyLines(text);
      endFontClass();

      yy_push_state(YY_START);
      BEGIN(UseOnly);
   }

<Use>{ID}                               {
      QString text = QString::fromUtf8(yytext);
      QString tmp = text.toLower();

      s_insideBody = true;
      generateLink(*s_code, text);
      s_insideBody = false;

      // append module name to use dict
      useEntry = new UseEntry();

      useEntry->module = tmp;
      useMembers->insert(tmp, useEntry);
      addUse(tmp);
   }

<Use,UseOnly,Import>{BS},{BS}               {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

<UseOnly,Import>{BS}&{BS}"\n"           {
      QString text = QString::fromUtf8(yytext);

      codifyLines(text);
      ++s_contLineNr;
      YY_FTN_RESET
   }

<UseOnly>{ID}                           {
      QString text = QString::fromUtf8(yytext);

      useEntry->onlyNames.append(text.toLower());
      s_insideBody = true;
      generateLink(*s_code, text);
      s_insideBody = false;
   }

<Use,UseOnly,Import>"\n"                {
      unput(*yytext);
      yy_pop_state();
      YY_FTN_RESET
   }

<*>"import"{BS}/"\n"   |
<*>"import"{BS_}                        {
      if(YY_START == String) {
         // ignore in strings
         YY_FTN_REJECT;
      }

      QString text = QString::fromUtf8(yytext);

      startFontClass("keywordtype");
      codifyLines(text);
      endFontClass();

      yy_push_state(YY_START);
      BEGIN(Import);
   }

<Import>{ID}                            {
      QString text = QString::fromUtf8(yytext);

      s_insideBody=  true;
      generateLink(*s_code, text);
      s_insideBody = false;
   }

<Import>("ONLY"|"NONE"|"ALL")          {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keywordtype");
      codifyLines(text);
      endFontClass();
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


<Start>("enum")/{BS_}|{BS}{COMMA}{BS}{LANGUAGE_BIND_SPEC}|\n {
      QString text = QString::fromUtf8(yytext);

      startScope();
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();

      yy_push_state(YY_START);
      BEGIN(ClassName);
   }

<*>{LANGUAGE_BIND_SPEC}                 {
      if (YY_START == String) {
         YY_FTN_REJECT;
      }

      QString text = QString::fromUtf8(yytext);

      // ignore in strings
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
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

      generateLink(*s_code, text);
      yy_pop_state();
   }

<ClassName>({ACCESS_SPEC}|ABSTRACT|EXTENDS)/[,:( ] {
      QString text = QString::fromUtf8(yytext);

      // variable declaration
      startFontClass("keyword");
      s_code->codify(text);
      endFontClass();
   }

<ClassName>\n           {
      // interface may be without name
      yy_pop_state();
      YY_FTN_REJECT;
   }

<Start>^{BS}"end"({BS_}"enum").*        {
      YY_FTN_REJECT;
   }

<Start>^{BS}"end"({BS_}"type").*        {
      YY_FTN_REJECT;
   }

<Start>^{BS}"end"({BS_}"module").*      {
      // just reset currentModule, rest is done in following rule
      currentModule = QString();
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

      startScope();
      generateLink(*s_code, text);
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
      ++s_contLineNr;

      yy_pop_state();
      YY_FTN_RESET
   }

<Start>"end"{BS}("block"{BS}"data"|{SUBPROG}|"module"|"program"|"enum"|"type"|"interface")?{BS}     {
      // Fortran subroutine or function ends
      QString text = QString::fromUtf8(yytext);

      endScope();
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();

      yy_push_state(YY_START);
      BEGIN(Subprogend);
   }

<Subprogend>{ID}/{BS}(\n|!|;)           {
      QString text = QString::fromUtf8(yytext);
      generateLink(*s_code, text);
      yy_pop_state();
   }

<Start>"end"{BS}("block"{BS}"data"|{SUBPROG}|"module"|"program"|"enum"|"type"|"interface"){BS}/(\n|!|;) {  // Fortran subroutine or function ends
      // Fortran subroutine or function ends
      QString text = QString::fromUtf8(yytext);

      endScope();
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
   }

 /*-------- variable declaration ----------------------------------*/

<Start>^{BS}"real"/[,:( ]              {
      // real is a bit tricky as it is a data type but also a function
      QString text = QString::fromUtf8(yytext);

      yy_push_state(YY_START);
      BEGIN(Declaration);

      startFontClass("keywordtype");
      s_code->codify(text);
      endFontClass();
   }

<Start>{TYPE_SPEC}/[,:( ]              {
      QString text = QString::fromUtf8(yytext);
      text = removeRedundantWhiteSpace(text.toLower());

      if (text.startsWith("real")) {
         YY_FTN_REJECT;
      }

      if (text == "type" || text == "class" || text == "procedure") {
         s_inTypeDecl = 1;
      }

      yy_push_state(YY_START);
      BEGIN(Declaration);

      startFontClass("keywordtype");
      s_code->codify(text);
      endFontClass();
   }

<Start>{ATTR_SPEC}                     {
      QString text = QString::fromUtf8(yytext);
      if (text == "external") {
        yy_push_state(YY_START);
        BEGIN(Declaration);

        s_isExternal = true;
      }
      startFontClass("keywordtype");
      s_code->codify(text);
      endFontClass();
   }

<Declaration>({TYPE_SPEC}|{ATTR_SPEC})/[,:( ] {
      // variable declaration
      QString text = QString::fromUtf8(yytext);

      if (text == "external") {
         s_isExternal = true;
      }

      startFontClass("keywordtype");
      s_code->codify(text);
      endFontClass();
    }

<Declaration>{ID}                      {
      // local var
      QString text = QString::fromUtf8(yytext);

      if (s_isFixedForm && yy_my_start == 1) {
         startFontClass("comment");
         s_code->codify(text);
         endFontClass();

      }  else if (s_currentMemberDef != nullptr && ( (s_currentMemberDef->isFunction() &&
                 (s_currentMemberDef->typeString() != "subroutine" || s_inTypeDecl) ) ||
                  s_currentMemberDef->isVariable() || s_currentMemberDef->isEnumValue() )) {
         generateLink(*s_code, text);

      } else {
         s_code->codify(text);
         addLocalVar(text);
      }
   }

<Declaration>{BS}("=>"|"="){BS}        {
      // Procedure binding
      QString text = QString::fromUtf8(yytext);

      BEGIN(DeclarationBinding);
      s_code->codify(text);
   }

<DeclarationBinding>{ID}               {
      // Type bound procedure link
      QString text = QString::fromUtf8(yytext);
      generateLink(*s_code, text);
      yy_pop_state();
   }

<Declaration>[(]                       {
      // start of array specification or class specification
      QString text = QString::fromUtf8(yytext);

      ++s_bracketCount;
      s_code->codify(text);
   }

<Declaration>[)]                       {
      // end array specification
      QString text = QString::fromUtf8(yytext);
      --s_bracketCount;

      if (! s_bracketCount) {
         s_inTypeDecl = 0;
      }

      s_code->codify(text);
   }

<Declaration,DeclarationBinding>"&"    {
      // continuation line
      QString text = QString::fromUtf8(yytext);
      s_code->codify(text);

      if (! s_isFixedForm) {
         yy_push_state(YY_START);
         BEGIN(DeclContLine);
      }
   }

<DeclContLine>"\n"                      {
      // declaration not yet finished
      QString text = QString::fromUtf8(yytext);

      ++s_contLineNr;
      codifyLines(text);
      s_bracketCount = 0;

      yy_pop_state();
      YY_FTN_RESET
   }

<Declaration,DeclarationBinding>"\n"                       {
      // end declaration line
      QString text = QString::fromUtf8(yytext);

      if (s_endComment) {
         s_endComment = false;

      } else {
         codifyLines(text);

      }

      s_bracketCount = 0;
      s_contLineNr++;

      if (! (! s_hasContLine.empty() && s_hasContLine[s_contLineNr - 1])) {
         s_isExternal = false;
         yy_pop_state();
      }
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

      s_insideBody = true;
      generateLink(*s_code, text);
      s_insideBody = false;
      yy_pop_state();
   }

<Start>{ID}{BS}/"("                     {
      // function call
      QString text = QString::fromUtf8(yytext);

      if (s_isFixedForm && yy_my_start == 6) {
         // fixed form continuation line
         YY_FTN_REJECT;
      } else if (text.trimmed().toLower() == "type") {
        yy_push_state(YY_START);
        BEGIN(Declaration);

        startFontClass("keywordtype");
        s_code->codify(text.trimmed());
        endFontClass();

        s_code->codify(text.mid(4));

      } else {
         s_insideBody = true;
         generateLink(*s_code, text);
         s_insideBody = false;
      }
   }

 /*-------- comments ---------------------------------------------------*/
<Start,Declaration,DeclarationBinding>\n?{BS}"!>"|"!<"                 {
      // start comment line or comment block
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '\n') {
         ++s_contLineNr;

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

      ++s_contLineNr;

      yy_old_start = 0;
      yy_my_start  = 1;
      yy_end = yyleng;

      // see if ! on position 6, can be continuation
      // but the chance is very unlikely
      docBlock += text;
   }

<DocBlock>"\n"                {
      // comment block ends at the end of this line
      // remove special comment (default config)

      static const bool stripCodeComments = Config::getBool("strip-code-comments");

      ++s_contLineNr;

      if (stripCodeComments) {
         s_yyLineNr += docBlock.count('\n') + 1;
         endCodeLine();

         if (s_yyLineNr < s_inputLines) {
            startCodeLine();
         }

         s_endComment = true;

      } else {
         // do not remove comment

         startFontClass("comment");
         codifyLines(docBlock);
         endFontClass();
      }

      unput(*yytext);
      --s_contLineNr;

      yy_pop_state();
      YY_FTN_RESET
   }

<*>"!"[^><\n].*|"!"$          {
      // normal comment
      QString text = QString::fromUtf8(yytext);

      if (YY_START == String) {
         YY_FTN_REJECT; // ignore in strings
      }

      if (s_isFixedForm && yy_my_start == 6) {
         YY_FTN_REJECT;
      }

      startFontClass("comment");
      codifyLines(text);
      endFontClass();
   }

<*>^[Cc*].*                   {
      // normal comment
      QString text = QString::fromUtf8(yytext);

      if (! s_isFixedForm) {
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

      if (s_isFixedForm && yy_my_start == 6) {
         YY_FTN_REJECT;
      }

      ++s_contLineNr;

      startFontClass("preprocessor");
      codifyLines(text);
      endFontClass();
      YY_FTN_RESET
   }

 /*------ variable references?  -------------------------------------*/

<Start>"%"{BS}{ID}         {
      // ignore references to elements
      QString text = QString::fromUtf8(yytext);
      s_code->codify(text);
   }

<Start>{ID}                             {
      QString text = QString::fromUtf8(yytext);

      s_insideBody = true;
      generateLink(*s_code, text);
      s_insideBody = false;
   }

 /*------ strings --------------------------------------------------*/
<String>\n                              {
      QString text = QString::fromUtf8(yytext);

      // string with \n inside
      ++s_contLineNr;
      s_outputStr += text;

      startFontClass("stringliteral");
      codifyLines(s_outputStr);
      endFontClass();

      s_outputStr = QString();
      YY_FTN_RESET
   }

<String>\"|\'                           {
      // string ends with next quote without previous backspace
      QString text = QString::fromUtf8(yytext);

      if(text[0] != s_stringStartSymbol) {
         YY_FTN_REJECT;                      // single vs double quote
      }

      s_outputStr += text;
      startFontClass("stringliteral");
      codifyLines(s_outputStr);
      endFontClass();
      yy_pop_state();
   }

<String>.                               {
      QString text = QString::fromUtf8(yytext);
      s_outputStr += text;
   }

<*>\"|\'                                {
      /* string starts */
      /* if(YY_START == StrIgnore) YY_FTN_REJECT; // ignore in simple comments */
      QString text = QString::fromUtf8(yytext);

      if (s_isFixedForm && yy_my_start == 6) {
         YY_FTN_REJECT;
      }

      yy_push_state(YY_START);
      s_stringStartSymbol = text[0];    // single or double quote
      BEGIN(String);
      s_outputStr = text;
   }

<*>\n                        {
      QString text = QString::fromUtf8(yytext);

      if (s_endComment) {
         s_endComment = false;
      } else {
         codifyLines(text);

         // comment can not extend over the end of a line, should always be terminated at the end of the line
         if (s_currentFontClass == "comment") {
            endFontClass();
         }
      }

      ++s_contLineNr;
      YY_FTN_RESET
   }

<*>^{BS}"type"{BS}"="        {
      QString text = QString::fromUtf8(yytext);
      s_code->codify(text);
   }

<*>[\xC0-\xFF][\x80-\xBF]+   {
      // utf-8 code point
      QString text = QString::fromUtf8(yytext);

      if (s_isFixedForm && yy_my_start > fixedCommentAfter) {
         startFontClass("comment");
         codifyLines(text);

      } else {
         s_code->codify(text);
      }
   }

<*>.                           {
      // catch all
      QString text = QString::fromUtf8(yytext);

      if (s_isFixedForm && yy_my_start > fixedCommentAfter) {
         startFontClass("comment");
         codifyLines(text);

      } else {
         s_code->codify(text);
      }
   }

<*>{LOG_OPER}                           {
      // Fortran logical comparison keywords
      QString text = QString::fromUtf8(yytext);
      s_code->codify(text);
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

static void checkContLines(const QString &str)
{
   int numLines = 2;          // one for element 0, one in case no \n at end

   for (auto c : str) {
      if (c == '\n') {
        ++numLines;
      }
   }

   s_hasContLine.resize(numLines);

   for (int i = 0; i < numLines; i++) {
      s_hasContLine[i] = 0;
   }

   prepassFixedForm(str, s_hasContLine);
   s_hasContLine[0] = 0;
}

void resetFortranCodeParserState() { }

void parseFortranCode(CodeGenerator &od, const QString &className, const QString &input,
                  bool isExampleBlock, const QString &exampleName, QSharedPointer<FileDef> fd, int startLine,
                  int endLine, bool inlineFragment, QSharedPointer<MemberDef> memberDef, bool,
                  QSharedPointer<Definition> searchCtx, bool collectXRefs, FortranFormat format)
{
   (void) className;
   (void) memberDef;

   if (input.isEmpty()) {
      return;
   }

   printlex(yy_flex_debug, true, __FILE__, fd == nullptr ? QString() : fd->fileName() );

   TooltipManager::instance()->clearTooltips();
   s_code = &od;

   useMembers = new UseSDict;

   s_inputString      = input;
   s_inputPosition    = 0;
   s_isFixedForm      = recognizeFixedForm(input, format);
   s_currentFontClass = QString();
   s_contLineNr       = 1;

   s_hasContLine.clear();
   s_needsTermination = false;

   if (s_isFixedForm) {
      checkContLines(s_inputString);
   }

   s_searchCtx        = searchCtx;
   s_collectXRefs     = collectXRefs;

   if (startLine != -1) {
      s_yyLineNr = startLine;
   } else {
      s_yyLineNr = 1;
   }

   if (endLine != -1) {
      s_inputLines  = endLine + 1;
   } else {
      s_inputLines  = s_yyLineNr + countLines() - 1;
   }

   s_exampleBlock  = isExampleBlock;
   s_exampleName   = exampleName;
   s_sourceFileDef = fd;

   if (isExampleBlock && fd == nullptr) {
      // create a dummy filedef for the example
      s_sourceFileDef = QMakeShared<FileDef>(QString(), exampleName);
   }

   if (s_sourceFileDef) {
      setCurrentDoc("l00001");
   }

   s_currentDefinition = QSharedPointer<Definition>();
   s_currentMemberDef  = QSharedPointer<MemberDef>();

   if (! s_exampleName.isEmpty()) {
      s_exampleFile = convertNameToFile_internal(s_exampleName + "-example");
   }

   s_includeCodeFragment = inlineFragment;
   startCodeLine();

   s_parmName.clear();
   s_parmType.clear();

   yyrestart( yyin );
   BEGIN( Start );
   yylex();

   if (s_needsTermination) {
      endFontClass();
      s_code->endCodeLine();
   }

   if (fd != nullptr) {
      TooltipManager::instance()->writeTooltips(*s_code);
   }

   if (isExampleBlock && s_sourceFileDef) {
      // delete the temporary file definition used for this example
      s_sourceFileDef = QSharedPointer<FileDef>();
   }

   delete useMembers;
   s_hasContLine.clear();

   return;
}
