/*************************************************************************
 *
 * Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
 * Copyright (c) 1997-2014 Dimitri van Heesch
 * Copyright (c) Anke Visser

*************************************************************************/

%{

#include <QByteArray>
#include <QFile>
#include <QMap>
#include <QStack>
#include <QRegularExpression>
#include <QVector>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include <arguments.h>
#include <config.h>
#include <entry.h>
#include <message.h>
#include <default_args.h>
#include <language.h>
#include <commentscan.h>
#include <parse_fortran.h>
#include <pre.h>
#include <util.h>

static const int fixedCommentAfter = 72;

// forward declarations, source in code_fortran.cpp
void parseFortranCode(CodeGenerator &, const QString &, const QString &, bool, const QString &,
                  QSharedPointer<FileDef> fd, int startLine, int endLine, bool inlineFragment,
                  QSharedPointer<MemberDef> memberDef, bool showLineNumbers, QSharedPointer<Definition> searchCtx,
                  bool collectRefs, FortranFormat format);

void resetFortranCodeParserState();
void codeFreeScanner();

// Toggle for some debugging info
// #define DBG_CTX(x) fprintf x
#define DBG_CTX(x) do { } while(0)

#define YY_NO_INPUT 1

enum ScanVar       { V_IGNORE, V_VARIABLE, V_PARAMETER, V_RESULT};
enum InterfaceType { IF_NONE, IF_SPECIFIC, IF_GENERIC, IF_ABSTRACT };

// Holds modifiers (attributes) for one symbol (variable, function, etc)
struct SymbolModifiers {
   enum Protection {NONE_P, PUBLIC, PRIVATE};
   enum Direction  {NONE_D, IN, OUT, INOUT};

   // used with function return value
   QString type;
   QString returnName;

   Protection protection;
   Direction direction;
   bool optional;
   bool protect;
   QString dimension;
   bool allocatable;
   bool external;
   bool intrinsic;
   bool parameter;
   bool pointer;
   bool target;
   bool save;
   bool deferred;
   bool nonoverridable;
   bool nopass;
   bool pass;
   bool contiguous;

   bool volat;       /* volatile is a reserved name */
   bool value;       /* volatile is a reserved name */
   QString passVar;
   QString bindVar;

   SymbolModifiers() : type(), returnName(), protection(NONE_P), direction(NONE_D),
      optional(false), protect(false), dimension(), allocatable(false),
      external(false), intrinsic(false), parameter(false),
      pointer(false), target(false), save(false), deferred(false), nonoverridable(false),
      nopass(false), pass(false), contiguous(false), volat(false), value(false), passVar() {}

   SymbolModifiers &operator |= (const SymbolModifiers &mdfs);
   SymbolModifiers &operator |= (QString mdfrString);
};

static const QString directionStrs[] =
{
   "", "intent(in)", "intent(out)", "intent(inout)"
};

static const QString directionParam[] =
{
   "", "[in]", "[out]", "[in,out]"
};

struct CommentInPrepass {
   CommentInPrepass(int commentCol, QString commentStr)
      : column(commentCol), str(commentStr)
   {
   }

   int column;
   QString str;
};

QStack<YY_BUFFER_STATE> include_stack;
QStack<QByteArray>      buffer_stack;

static QString          s_inputString;
static int		         s_inputPosition;

static bool             s_isFixedForm;
static QString          s_inputStringPrepass;    // Input string for prepass of line cont. '&'
static QString          s_inputStringSemi;       // Input string after command separator ';'
static unsigned int     s_inputPositionPrepass;
static int              s_lineCountPrepass = 0;

static QFile            inputFile;
static QString  		   yyFileName;

static int		         yyLineNr = 1;
static int		         yyColNr  = 0;

static QString          docBlock;
static bool             docBlockInBody = false;
static bool             docBlockJavaStyle;

static ScanVar          v_type  = V_IGNORE;              // type of parsed variable
static InterfaceType    ifType  = IF_NONE;

static QString          debugStr;
static QString          result;                          // function result

static QString          argType;                         // fortran type of an argument of a parameter list
static QString          argName;                         // last identifier name in variable list
static QString          s_initializer;                   // initial value of a variable
static int              initializerArrayScope;           // number if nested array scopes in initializer
static int              initializerScope;                // number if nested function calls in initializer
static QString          useModuleName;                   // name of module in the use statement
static Protection       defaultProtection;
static Protection       typeProtection;

static int              typeMode          = false;
static bool             functionLine      = false;
static bool             parsingPrototype  = false;       // see parsePrototype()
static QChar            stringStartSymbol;               // single or double quote

// accumulated modifiers of current statement, eg variable declaration.
static SymbolModifiers  currentModifiers;

// holds program scope->symbol name->symbol modifiers
static QMap<QSharedPointer<Entry>, QMap<QString, SymbolModifiers>> modifiers;
static int s_anonCount = 0;

static ParserInterface        *s_thisParser;

static QSharedPointer<Entry>  s_global_scope;
static bool s_fake_entry = false;

static QSharedPointer<Entry>	current_root;
static QSharedPointer<Entry>	global_root;
static QSharedPointer<Entry>	file_root;
static QSharedPointer<Entry>	current;
static QSharedPointer<Entry>	s_last_entry;
static QSharedPointer<Entry>	s_last_enum;

static QList<QSharedPointer<Entry>>  moduleProcedures;   // list of all interfaces which contain unresolved
static QList<QSharedPointer<Entry>>  subrCurrent;
static QList<CommentInPrepass *>     comments;

// methods
static int  yyread(char *buf, int max_size);
static void startCommentBlock(bool);
static void handleCommentBlock(const QString &doc, bool brief);
static void subrHandleCommentBlock(const QString &doc, bool brief);
static void subrHandleCommentBlockResult(const QString &doc, bool brief);
static void addCurrentEntry(bool caseInsensitive);
static void addModule(const QString &name = QString(), bool isModule = false);
static void addSubprogram(const QString &text);
static void addInterface(const QString &name, InterfaceType type);

static Argument *getParameter(const QString &name);
static void scanner_abort();

static void startScope(QSharedPointer<Entry> scope);
static bool endScope(QSharedPointer<Entry> scope, bool isGlobalRoot = false);
static void resolveModuleProcedures(QList<QSharedPointer<Entry>> &moduleProcedures, QSharedPointer<Entry> current_root);
static int getAmpersandAtTheStart(const  QString &buf, int length);
static int getAmpOrExclAtTheEnd(const QString &buf, int length, QChar ch);
static void truncatePrepass(int index);
static void pushBuffer(QString &buffer);
static void popBuffer();

static QString extractFromParens(const QString &name);
static QString extractBind(const QString &name);

static CommentInPrepass *locatePrepassComment(int from, int to);
static void updateVariablePrepassComment(int from, int to);
static void newLine();
static void initEntry();

#undef   YY_INPUT
#define  YY_INPUT(buf, result, max_size) result = yyread(buf, max_size);
#define  YY_USER_ACTION yyColNr += yyleng;


%}

IDSYM     [a-z_A-Z0-9]
NOTIDSYM  [^a-z_A-Z0-9]
SEPARATE  [:, \t]
ID        [a-z_A-Z%]+{IDSYM}*
ID_       [a-z_A-Z%]*{IDSYM}*
PP_ID     {ID}
LABELID   [a-z_A-Z]+[a-z_A-Z0-9\-]*
SUBPROG   (subroutine|function)
B         [ \t]
BS        [ \t]*
BS_       [ \t]+
BT_       ([ \t]+|[ \t]*"(")
COMMA     {BS},{BS}
ARGS_L0   ("("[^)]*")")
ARGS_L1a  [^()]*"("[^)]*")"[^)]*
ARGS_L1   ("("{ARGS_L1a}*")")
ARGS_L2   "("({ARGS_L0}|[^()]|{ARGS_L1a}|{ARGS_L1})*")"
ARGS      {BS}({ARGS_L0}|{ARGS_L1}|{ARGS_L2})
NOARGS    {BS}"\n"

NUM_TYPE  (complex|integer|logical|real)
LOG_OPER  (\.and\.|\.eq\.|\.eqv\.|\.ge\.|\.gt\.|\.le\.|\.lt\.|\.ne\.|\.neqv\.|\.or\.|\.not\.)
KIND      {ARGS}
CHAR      (CHARACTER{ARGS}?|CHARACTER{BS}"*"({BS}[0-9]+|{ARGS}))

TYPE_SPEC (({NUM_TYPE}({BS}"*"{BS}[0-9]+)?)|({NUM_TYPE}{KIND})|DOUBLE{BS}COMPLEX|DOUBLE{BS}PRECISION|ENUMERATOR|{CHAR}|TYPE{ARGS}|CLASS{ARGS}|PROCEDURE{ARGS}?)

INTENT_SPEC   intent{BS}"("{BS}(in|out|in{BS}out){BS}")"
ATTR_SPEC     (EXTERNAL|ALLOCATABLE|DIMENSION{ARGS}|{INTENT_SPEC}|INTRINSIC|OPTIONAL|PARAMETER|POINTER|PROTECTED|PRIVATE|PUBLIC|SAVE|TARGET|NOPASS|PASS{ARGS}?|DEFERRED|NON_OVERRIDABLE|CONTIGUOUS|VOLATILE|VALUE)

ACCESS_SPEC   (PRIVATE|PUBLIC)
LANGUAGE_BIND_SPEC BIND{BS}"("{BS}C{BS}((,{BS}NAME{BS}"="{BS}"\""(.*)"\""{BS})|(,{BS}NAME{BS}"="{BS}"'"(.*)"'"{BS}))?")"

/* Assume that attribute statements are almost the same as attributes. */
ATTR_STMT          {ATTR_SPEC}|DIMENSION|{ACCESS_SPEC}
EXTERNAL_STMT      (EXTERNAL)
CONTAINS           CONTAINS
PREFIX             ((NON_)?RECURSIVE{BS_}|IMPURE{BS_}|PURE{BS_}|ELEMENTAL{BS_}){0,4}((NON_)?RECURSIVE|IMPURE|PURE|ELEMENTAL)?
SCOPENAME          ({ID}{BS}"::"{BS})*

%option caseless
%option case-insensitive
%option never-interactive
%option nounistd
%option noyywrap
%option stack

%x    Subprog
%x    SubprogPrefix
%x    Parameterlist
%x    SubprogBody
%x    SubprogBodyContains
%x    Start
%x    Comment
%x    Module
%x    Program
%x    ModuleBody
%x    ModuleBodyContains
%x    AttributeList
%x    Variable
%x    Initialization
%x    ArrayInitializer
%x    Enum
%x    Typedef
%x    TypedefBody
%x    TypedefBodyContains
%x    InterfaceBody
%x    StrIgnore
%x    String
%x    Use
%x    UseOnly
%x    ModuleProcedure
%x    Prepass

 /** comment parsing states */
%x    DocBlock
%x    DocBackLine
%x    BlockData

/** prototype parsing */
%x    Prototype
%x    PrototypeSubprog
%x    PrototypeArgs

%%


<Prepass>^{BS}[&]*{BS}!.*\n             {
      /* skip lines with just comment. Note code was in free format or has been converted to it */
      s_lineCountPrepass ++;
   }

<Prepass>^{BS}\n                        {
      /* skip empty lines */
      s_lineCountPrepass ++;
   }

<*>^.*\n                                {
      // prepass: look for line continuations
      QString text = QString::fromUtf8(yytext);

      functionLine = false;
      DBG_CTX((stderr, "---%s", yytext));

      int indexStart = getAmpersandAtTheStart(text, yyleng);
      int indexEnd   = getAmpOrExclAtTheEnd(text, yyleng, '\0');

      if (indexEnd >= 0 && text[indexEnd] != '&') {
         // we are only interested in amp
         indexEnd = -1;
      }

      if (indexEnd < 0) {
         // no ampersand as line continuation
         if( YY_START == Prepass) {
            // last line in "continuation"

            // only take input after initial ampersand
            s_inputStringPrepass += text.mid(indexStart + 1);

            pushBuffer(s_inputStringPrepass);
            yyColNr = 0;
            yy_pop_state();

         } else {
            // simple line
            yyColNr = 0;
            REJECT;
         }

      } else {
         // line with continuation
         if(YY_START != Prepass) {
            comments.clear();
            yy_push_state(Prepass);
         }

         int length = s_inputStringPrepass.length();

         // Only take input after initial ampersand
         s_inputStringPrepass += text.mid(indexStart + 1);;
         s_lineCountPrepass ++;

         // cut off & and remove following comment if present
         truncatePrepass(length + indexEnd - (indexStart + 1));
      }
}


 /*------ ignore strings that are not initialization strings */
<String>\"|\'                           {
      // string ends with next quote without previous backspace
      QString text = QString::fromUtf8(yytext);

      if (text[0] != stringStartSymbol) {
         // single vs double quote

         yyColNr -= yyleng;
         REJECT;
      }

      if (yy_top_state() == Initialization || yy_top_state() == ArrayInitializer) {
         s_initializer += text;
      }

      yy_pop_state();
   }

<String>.                               {
      if (yy_top_state() == Initialization || yy_top_state() == ArrayInitializer) {
         QString text = QString::fromUtf8(yytext);
         s_initializer += text;
      }
   }

<*>\"|\'                                {
      /* string starts */
      QString text = QString::fromUtf8(yytext);

      if (YY_START == StrIgnore) {
         yyColNr -= yyleng;
         REJECT;
      }

      // ignore in simple comments
      yy_push_state(YY_START);

      if (yy_top_state() == Initialization || yy_top_state() == ArrayInitializer) {
         s_initializer += text;
      }

      stringStartSymbol = text[0]; // single or double quote
      BEGIN(String);
   }

 /*------ ignore simple comment (not documentation comments) */

<*>"!"/[^<>\n]                         {
      if (YY_START == String) {
         // "!" is ignored in strings
         yyColNr -= yyleng;
         REJECT;
      }

      // skip comment line (without docu comments "!>" "!<" )
      // ignore further "!" and ignore comments in Strings

      if ((YY_START != StrIgnore) && (YY_START != String)) {
         yy_push_state(YY_START);
         BEGIN(StrIgnore);

         debugStr = "*!";
         DBG_CTX((stderr,"start comment %d\n", yyLineNr));
      }
   }

<StrIgnore>.?/\n                         {
      // comment ends with endline character

      yy_pop_state();
      DBG_CTX((stderr,"end comment %d %s\n", yyLineNr, debugStr.data()));
   }

<StrIgnore>.                            {
      debugStr += QString::fromUtf8(yytext);
   }

 /*------ use handling --------------------------------- */

<Start,ModuleBody,SubprogBody>"use"{BS_} {
      if(YY_START == Start) {
         addModule();
         yy_push_state(ModuleBody);       // anon program
      }
      yy_push_state(Use);
   }

<Use>{ID}                               {
      QString text = QString::fromUtf8(yytext);

      current->section     = Entry::USINGDIR_SEC;
      current->m_entryName = text.toLower();
      current->setData(EntryKey::File_Name, yyFileName);

      current_root->addSubEntry(current);

      current = QMakeShared<Entry>();
      current->m_srcLang = SrcLangExt_Fortran;
      yy_pop_state();
   }

<Use>{ID}/,                             {
      useModuleName = QString::fromUtf8(yytext);;
   }

<Use>,{BS}"ONLY"                        {
      BEGIN(UseOnly);
    }

<UseOnly>{BS},{BS}                      {
   }


<UseOnly>{ID}                           {
      QString text = QString::fromUtf8(yytext);

      current->section     = Entry::USINGDECL_SEC;
      current->m_entryName = (useModuleName + "::" + text).toLower();
      current->setData(EntryKey::File_Name, yyFileName);

      current_root->addSubEntry(current);

      current = QMakeShared<Entry>();
      current->m_srcLang = SrcLangExt_Fortran;
   }

<Use,UseOnly>"\n"                       {
      yyColNr -= 1;
      unput(*yytext);
      yy_pop_state();
   }

 /* INTERFACE definitions */
<Start,ModuleBody,SubprogBody>{

^{BS}interface{IDSYM}+        {
      /* variable with interface prefix */
   }

^{BS}interface                          {
      ifType = IF_SPECIFIC;
      yy_push_state(InterfaceBody);
      // do not start a scope here, every
      // interface body is a scope of its own
   }


^{BS}abstract{BS_}interface             {
      ifType = IF_ABSTRACT;
      yy_push_state(InterfaceBody);
      // do not start a scope here, every
      // interface body is a scope of its own
   }

^{BS}interface{BS_}{ID}{ARGS}?          {
      QString text = QString::fromUtf8(yytext);

      ifType = IF_GENERIC;
      current->startBodyLine = yyLineNr + s_lineCountPrepass + 1;

      // need to be at the line after the definition and we have to take continuation lines into account.
      yy_push_state(InterfaceBody);

      // extract generic name
      text = text.trimmed();
      text = text.right(text.length() - 9).trimmed().toLower();
      addInterface(text, ifType);
      startScope(s_last_entry);
   }
}

<InterfaceBody>^{BS}end{BS}interface({BS_}{ID})? {
      // end scope only if GENERIC interface
      if (ifType == IF_GENERIC)  {
         s_last_entry->parent()->endBodyLine = yyLineNr - 1;
      }

      if (ifType == IF_GENERIC && ! endScope(current_root)) {
         yyterminate();
      }

      ifType = IF_NONE;
      yy_pop_state();
   }

<InterfaceBody>module{BS}procedure      {
      yy_push_state(YY_START);
      BEGIN(ModuleProcedure);
   }

<ModuleProcedure>{ID}                   {
      QString text = QString::fromUtf8(yytext);

      if (ifType == IF_ABSTRACT || ifType == IF_SPECIFIC) {
         addInterface(text, ifType);
         startScope(s_last_entry);
      }

      current->section     = Entry::FUNCTION_SEC ;
      current->m_entryName = text;
      moduleProcedures.append(current);
      addCurrentEntry(true);
   }

<ModuleProcedure>"\n"                   {
      yyColNr -= 1;
      unput(*yytext);
      yy_pop_state();
   }

<InterfaceBody>.                        {
   }

 /*-- Contains handling ---------------------------------*/
<Start>^{BS}{CONTAINS}/({BS}|\n|!|;)      {
      if (YY_START == Start) {
         addModule();
         yy_push_state(ModuleBodyContains);       // anon program
      }
   }

<ModuleBody>^{BS}{CONTAINS}/({BS}|\n|!|;)   {
      BEGIN(ModuleBodyContains);
   }

<SubprogBody>^{BS}{CONTAINS}/({BS}|\n|!|;)  {
      BEGIN(SubprogBodyContains);
   }

<TypedefBody>^{BS}{CONTAINS}/({BS}|\n|!|;)  {
      BEGIN(TypedefBodyContains);
   }

 /*------ module handling ------------------------------ */
<Start>block{BS}data{BS}{ID_}        {
      v_type = V_IGNORE;
      yy_push_state(BlockData);
      defaultProtection = Public;
   }

<Start>module|program{BS_}             {
      QString text = QString::fromUtf8(yytext);
      v_type = V_IGNORE;

      if (text[0] == 'm' || text[0] == 'M') {
         yy_push_state(Module);
      } else {
         yy_push_state(Program);
      }

      defaultProtection = Public;
   }

<BlockData>^{BS}"end"({BS}(block{BS}data)({BS_}{ID})?)?{BS}/(\n|!|;) {
      // end block data

      defaultProtection = Public;
      yy_pop_state();
   }

<Start,ModuleBody,ModuleBodyContains>"end"({BS}(module|program)({BS_}{ID})?)?{BS}/(\n|!|;) {
      // end module
      resolveModuleProcedures(moduleProcedures, current_root);
      if (! endScope(current_root)) {
         yyterminate();
      }

      defaultProtection = Public;

      if (s_global_scope != nullptr && ! s_fake_entry) {
         yy_push_state(Start);

      } else if (s_fake_entry) {
         yy_pop_state();                   // do not pop fake entry

      } else {
         yy_push_state(Start);
         s_fake_entry = true;              // indicates the global_scope has already been used
      }
   }

<Module>{ID}                           {
      QString text = QString::fromUtf8(yytext);
      addModule(text, true);
      BEGIN(ModuleBody);
   }

<Program>{ID}                           {
      QString text = QString::fromUtf8(yytext);
      addModule(text, false);
      BEGIN(ModuleBody);
   }

 /*------- access specification ------------------------ */

<ModuleBody>private/{BS}(\n|"!")         {
      defaultProtection = Private;
      current->protection = defaultProtection ;
   }

<ModuleBody>public/{BS}(\n|"!")          {
      defaultProtection = Public;
      current->protection = defaultProtection ;
   }

 /*------- type definition  ---------------------------- */

<ModuleBody>^{BS}type{BS}"="            {
      // no code
   }

<Start,ModuleBody>^{BS}type/[^a-z0-9_]   {
      if(YY_START == Start) {
         addModule();
         yy_push_state(ModuleBody);       // anon program
      }

      yy_push_state(Typedef);
      current->protection = defaultProtection;
      typeProtection = defaultProtection;
      typeMode = true;
   }

   /* start group  */
<Typedef>{
{COMMA}                                 {
      // no code
   }

{BS}"::"{BS}                            {
      // no code
   }

abstract                                {
      current->m_traits.setTrait(Entry::Virtue::AbstractClass);
   }

extends{ARGS}                           {
      QString text = QString::fromUtf8(yytext);

      QString basename = extractFromParens(text).toLower();
      current->extends.append(BaseInfo(basename, Public, Normal));
   }

public                                  {
      current->protection = Public;
      typeProtection = Public;
   }

private                                 {
      current->protection = Private;
      typeProtection      = Private;
   }

{LANGUAGE_BIND_SPEC}                    {
      // ignored for now
   }

{ID}                                    {
      // type name found
      QString text = QString::fromUtf8(yytext);

      current->section     = Entry::CLASS_SEC;
      current->m_entryName = text;

      current->m_traits.setTrait(Entry::Virtue::Struct);

      current->setData(EntryKey::File_Name, yyFileName);
      current->startBodyLine = yyLineNr;
      current->startLine     = yyLineNr;

      /* if type is part of a module, mod name is necessary for output */
      if ((current_root) && (current_root->section == Entry::CLASS_SEC || current_root->section == Entry::NAMESPACE_SEC)) {
         current->m_entryName = current_root->m_entryName + "::" + current->m_entryName;
      }

      addCurrentEntry(true);
      startScope(s_last_entry);
      BEGIN(TypedefBody);
   }
}

   /* start group  */
<TypedefBodyContains>{
      /* Type Bound Procedures */

^{BS}PROCEDURE{ARGS}?                   {
      current->setData(EntryKey::Member_Type, QString::fromUtf8(yytext).simplified());
   }

^{BS}final                              {
      current->m_traits.setTrait(Entry::Virtue::Final);
      current->setData(EntryKey::Member_Type, QString::fromUtf8(yytext).simplified());
   }

^{BS}generic                            {
      current->setData(EntryKey::Member_Type, QString::fromUtf8(yytext).simplified());
   }

{COMMA}                                 {
   }

{ATTR_SPEC}                             {
      currentModifiers |= QString::fromUtf8(yytext);
   }

{BS}"::"{BS}                            {
   }

{ID}                                    {
      QString text = QString::fromUtf8(yytext);

      modifiers[current_root][text.toLower()] |= currentModifiers;

      current->section     = Entry::FUNCTION_SEC;
      current->m_entryName = text;

      current->setData(EntryKey::File_Name, yyFileName);
      current->startBodyLine = yyLineNr;
      current->startLine     = yyLineNr;

      addCurrentEntry(true);
   }

{BS}"=>"[^(\n|\!)]*                     {
      /* Specific bindings come after the ID. */
      QString text = QString::fromUtf8(yytext);
      s_last_entry->setData(EntryKey::Member_Args, text.toLower());
   }

"\n"                                    {
      currentModifiers = SymbolModifiers();
      newLine();
      docBlock.resize(0);
   }
}

   /* start group  */
<TypedefBody,TypedefBodyContains>{

^{BS}"end"{BS}"type"({BS_}{ID})?{BS}/(\n|!|;) {
      // end type definition
      s_last_entry->parent()->endBodyLine = yyLineNr;

      if (! endScope(current_root)) {
         yyterminate();
      }

      typeMode = false;
      yy_pop_state();
   }
}

^{BS}"end"{BS}/(\n|!|;) {
      // incorrect end type definition
      warn(yyFileName, yyLineNr, "Found 'END' instead of 'END TYPE'");
      s_last_entry->parent()->endBodyLine = yyLineNr;

      if (! endScope(current_root)) {
         yyterminate();
      }

      typeMode = false;
      yy_pop_state();
   }


 /*------- module/global/typedef variable -------------- */

<SubprogBody,SubprogBodyContains>^{BS}[0-9]*{BS}"end"({BS}{SUBPROG}({BS_}{ID})?)?{BS}/(\n|!|;) {

      // ABSTRACT and specific interfaces are stored
      // in a scope of their own, even if multiple
      // are group in one INTERFACE/END INTERFACE block.

      if (ifType == IF_ABSTRACT || ifType == IF_SPECIFIC) {
         endScope(current_root);
         s_last_entry->endBodyLine = yyLineNr - 1;
      }

      current_root->endBodyLine = yyLineNr - 1;

      if (! endScope(current_root)) {
         yyterminate();
      }

      subrCurrent.removeAt(0);
      v_type = V_IGNORE;

      yy_pop_state();
   }

<BlockData>{

{ID}         {
      // no code
   }

}

<Start,ModuleBody,TypedefBody,SubprogBody,Enum>{
^{BS}{TYPE_SPEC}/{SEPARATE}             {
      QString text = QString::fromUtf8(yytext);

      s_last_enum.reset();

      if (YY_START == Enum) {
         argType = "@";       // enum marker
      } else {
         argType = text.simplified().toLower();
      }
      current->startBodyLine = yyLineNr + 1;
      current->endBodyLine   = yyLineNr + s_lineCountPrepass;

      // variable declaration starts
      if (YY_START == Start) {
         addModule();
         yy_push_state(ModuleBody);       // anon program
      }

      yy_push_state(AttributeList);
   }

{EXTERNAL_STMT}/({BS}"::"|{BS_}{ID})   {
      /* external can be a "type" or an attribute */
      QString text = QString::fromUtf8(yytext);

      if(YY_START == Start) {
         addModule();
         yy_push_state(ModuleBody);    // anon program
      }

      currentModifiers |= text.trimmed();
      argType = text.simplified().toLower();
      yy_push_state(AttributeList);
   }

{ATTR_STMT}/{BS_}{ID}        |
{ATTR_STMT}/{BS}"::"                   {
      /* attribute statement starts */
      QString text = QString::fromUtf8(yytext);

      currentModifiers |= text.trimmed();
      argType = QString();
      yy_push_state(YY_START);
      BEGIN( AttributeList ) ;
   }

{ID}                                   {
      // no code
   }

^{BS}"type"{BS_}"is"/{BT_}             {
      // no code
   }

^{BS}"type"{BS}"="                     {
      // no code
   }

^{BS}"class"{BS_}"is"/{BT_}            {
      // no code
   }

^{BS}"class"{BS_}"default"             {
      // no code
   }
}

<AttributeList>{
{COMMA}              {
      // no code
   }

{BS}                 {
      // no code
   }

{LANGUAGE_BIND_SPEC}  {
      currentModifiers |= QString::fromUtf8(yytext);
   }


{ATTR_SPEC}.         {
      // update current modifiers when it is an ATTR_SPEC and not a variable name
      QString text = QString::fromUtf8(yytext);
      QChar chr    = text[text.length() - 1];

      if (isId(chr)) {
         yyColNr -= yyleng;
         REJECT;

      } else {
         text = text.left(text.length() - 1);
         yyColNr -= 1;

         char tmp = yytext[yyleng - 1];
         unput(tmp);

         currentModifiers |= text;
      }
   }

"::"              {
      /* end attribute list */
      BEGIN( Variable );
   }

.                 {
      // unknown attribute, consider variable name
      yyColNr -= 1;
      unput(*yytext);
      BEGIN( Variable );
   }
}

<Variable>{BS}             {
      // no code
   }

<Variable>{ID}             {
      // parse variable declaration
      QString text = QString::fromUtf8(yytext).toLower();

      // remember attributes for the symbol
      modifiers[current_root][text] |= currentModifiers;

      argName = text;
      v_type  = V_IGNORE;

      if (! argType.isEmpty() && current_root->section != Entry::FUNCTION_SEC) {
         // new variable entry
         v_type = V_VARIABLE;
         current->section     = Entry::VARIABLE_SEC;
         current->m_entryName = argName;

         current->setData(EntryKey::Member_Type, argType);

         current->setData(EntryKey::File_Name, yyFileName);
         current->startBodyLine = yyLineNr;                  // used for source reference
         current->startLine     = yyLineNr;

         if (argType == "@") {
           QSharedPointer<Entry> tmpCopy = QMakeShared<Entry>(*current);
           current_root->addSubEntry(tmpCopy);

           // add to the scope surrounding the enum
           s_last_enum = current;

           current_root->parent()->addSubEntry(current);

           current = QMakeShared<Entry>();
           initEntry();

         } else {
            addCurrentEntry(true);
         }

      } else if (! argType.isEmpty()) {
         // declaration of parameter list: add type for corr. parameter
         Argument *argData = getParameter(argName);

         if (argData) {
            v_type= V_PARAMETER;

            if (! argType.isEmpty()) {
               argData->type = argType.trimmed();
            }

            if (! docBlock.isEmpty()) {
               subrHandleCommentBlock(docBlock, true);
            }
         }

         // save, it may be function return type
         if (argData) {
            modifiers[current_root][text].type = argType;

         } else {
            QString rootName = current_root->m_entryName.toLower();

            if (rootName == argName.toLower() ||
                  (modifiers[current_root->parent()][rootName].returnName.toLower() == argName.toLower())) {

               QString tmpType = current_root->getData(EntryKey::Member_Type);

               QString tmpLeft;
               QString tmpRight;

               int pos = tmpType.indexOf("function");

               if (pos != -1) {
                  v_type   = V_RESULT;
                  tmpLeft  = "";
                  tmpRight = "";

                  if (pos != 0) {
                     tmpLeft = tmpType.left(pos).trimmed();
                  }

                  if ((tmpType.length() - pos - strlen("function")) != 0) {
                     tmpRight = tmpType.right(tmpType.length() - pos - strlen("function")).trimmed();
                  }

                  tmpType = tmpLeft;

                  if (tmpRight.length() > 0)  {

                     if (tmpType.length() > 0) {
                        tmpType += " ";
                     }

                     tmpType += tmpRight;
                  }

                  if (argType.trimmed().length() > 0) {

                     if (tmpType.length() > 0)  {
                        tmpType += " ";
                     }

                     tmpType += argType.trimmed();
                  }

                  if (tmpType.length() > 0) {
                     tmpType += " ";
                  }

                  tmpType += "function";

                  if (! docBlock.isEmpty()) {
                     subrHandleCommentBlockResult(docBlock,true);
                  }

               } else  {
                  tmpType += " " + argType.trimmed();

               }

               tmpType = tmpType.trimmed();
               current_root->setData(EntryKey::Member_Type, tmpType);

               modifiers[current_root][text].type = tmpType;

            } else {
               modifiers[current_root][text].type = argType;

            }
         }

         // accumulated docs for argument should be cleared  because it is handled aother way
         // and these docsc can be unexpectedly passed to the next member
         current->setData(EntryKey::Brief_Docs, QString());
         current->setData(EntryKey::Main_Docs,  QString());
      }
   }

<Variable>{ARGS}        {
      /* dimension of the previous entry. */
      QString text = QString::fromUtf8(yytext);

      QString attr = "dimension";
      attr += text;

      modifiers[current_root][argName.toLower()] |= attr;
   }

<Variable>{COMMA}                       {
      // locate !< comment
      updateVariablePrepassComment(yyColNr - yyleng, yyColNr);
   }

<Variable>{BS}"="                       {
      yy_push_state(YY_START);
      s_initializer         = "=";
      initializerScope      = 0;
      initializerArrayScope = 0;
      BEGIN(Initialization);
   }

<Variable>"\n"                            {
      currentModifiers = SymbolModifiers();
      yy_pop_state();                         // end variable declaration list
      newLine();
      docBlock.resize(0);
   }

<Variable>";".*"\n"                     {
      QString text = QString::fromUtf8(yytext);

      currentModifiers = SymbolModifiers();
      yy_pop_state();                         // end variable declaration list
      docBlock.resize(0);

      s_inputStringSemi = " \n" + text.mid(1);
      --yyLineNr;

      pushBuffer(s_inputStringSemi);
   }

<*>";".*"\n"                            {
      QString text = QString::fromUtf8(yytext);

      if (YY_START == Variable) {
         REJECT;       // Just be on the safe side
      }

      if (YY_START == String) {
         REJECT;       // ";" ignored in strings
      }

      if (YY_START == StrIgnore) {
         REJECT;      // ";" ignored in regular comments
      }

      s_inputStringSemi = " \n" + text.mid(1);
      --yyLineNr;

      pushBuffer(s_inputStringSemi);
   }

<Initialization,ArrayInitializer>"["    |
<Initialization,ArrayInitializer>"(/"   {
      QString text = QString::fromUtf8(yytext);

      s_initializer += text;
      ++initializerArrayScope;

      BEGIN(ArrayInitializer);                // initializer may contain comma
   }

<ArrayInitializer>"]"                   |
<ArrayInitializer>"/)"                  {
      QString text = QString::fromUtf8(yytext);

      s_initializer += text;
      --initializerArrayScope;

      if (initializerArrayScope <= 0) {
         initializerArrayScope = 0;           // just in case
         BEGIN(Initialization);
      }
   }

<ArrayInitializer>.        {
      QString text = QString::fromUtf8(yytext);
      s_initializer += text;
   }

<Initialization>"("        {
      QString text = QString::fromUtf8(yytext);
      ++initializerScope;
      s_initializer += text;
   }

<Initialization>")"        {
      QString text = QString::fromUtf8(yytext);
      --initializerScope;
      s_initializer += text;
   }

<Initialization>{COMMA}    {
      if (initializerScope == 0) {
         updateVariablePrepassComment(yyColNr - yyleng, yyColNr);
         yy_pop_state();                      // end initialization
         if (s_last_enum) {
            s_last_enum->setData(EntryKey::Initial_Value, s_initializer);

         } else {
            if (v_type == V_VARIABLE) {
               s_last_entry->setData(EntryKey::Initial_Value, s_initializer);
            }
         }

      } else {
         s_initializer += ", ";
      }
   }

<Initialization>"\n"|"!"                {
      yy_pop_state();                         // end initialization

      if (s_last_enum) {
         s_last_enum->setData(EntryKey::Initial_Value, s_initializer);


      } else {

         if (v_type == V_VARIABLE)  {
            s_last_entry->setData(EntryKey::Initial_Value, s_initializer);
         }
      }

      yyColNr -= 1;
      unput(*yytext);
   }

<Initialization>.                       {
      QString text = QString::fromUtf8(yytext);
      s_initializer += text;
   }

<*>{BS}"enum"{BS}","{BS}"bind"{BS}"("{BS}"c"{BS}")"{BS} {
      QString text = QString::fromUtf8(yytext);

      if (YY_START == Start) {
        addModule();
        yy_push_state(ModuleBody);     // anon program
      }

      yy_push_state(Enum);
      current->protection = defaultProtection;
      typeProtection      = defaultProtection;
      typeMode            = true;

      current->m_traits.setTrait(Entry::Virtue::Struct);
      current->setData(EntryKey::Member_Args, QString());

      current->m_entryName = QString("@%1").formatArg(s_anonCount);
      ++s_anonCount;

      current->section = Entry::ENUM_SEC;

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine     = yyLineNr;
      current->startBodyLine = yyLineNr;

      if ((current_root) &&
          (current_root->section == Entry::CLASS_SEC ||
           current_root->section == Entry::NAMESPACE_SEC)) {
        current->m_entryName = current_root->m_entryName + "::" + current->m_entryName;
      }

      addCurrentEntry(true);
      startScope(s_last_entry);
      BEGIN(Enum);
   }

<Enum>"end"{BS}"enum"                   {
      s_last_entry->parent()->endBodyLine = yyLineNr;

      if (! endScope(current_root)) {
        yyterminate();
      }

      typeMode = false;
      yy_pop_state();
   }

 /*------ fortran subroutine/function handling --------- */
 /*       Start is initial condition                     */

<Start,ModuleBody,SubprogBody,InterfaceBody,ModuleBodyContains,SubprogBodyContains>^{BS}({PREFIX}{BS_})?{TYPE_SPEC}{BS}({PREFIX}{BS_})?/{SUBPROG}{BS_} {
      QString text = QString::fromUtf8(yytext);

      if (ifType == IF_ABSTRACT || ifType == IF_SPECIFIC) {
         addInterface("$interface$", ifType);
         startScope(s_last_entry);
      }

      // TYPE_SPEC is for old function style function result
      result = text.trimmed().toLower();
      current->setData(EntryKey::Member_Type, result);

      yy_push_state(SubprogPrefix);
   }

<SubprogPrefix>{BS}{SUBPROG}{BS_}     {
      // Fortran subroutine or function found
      QString text = QString::fromUtf8(yytext);

      v_type = V_IGNORE;
      result = text.trimmed();
      addSubprogram(result);
      BEGIN(Subprog);

      // need to be at the line after the definition and we have to take continuation lines into account
      current->startBodyLine = yyLineNr + s_lineCountPrepass + 1;
      current->startLine     = yyLineNr;
   }

<Start,ModuleBody,SubprogBody,InterfaceBody,ModuleBodyContains,SubprogBodyContains>^{BS}({PREFIX}{BS_})?{SUBPROG}{BS_} {
      // Fortran subroutine or function found
      QString text = QString::fromUtf8(yytext);

      v_type = V_IGNORE;
      if (ifType == IF_ABSTRACT || ifType == IF_SPECIFIC) {
         addInterface("$interface$", ifType);
         startScope(s_last_entry);
      }

      result = text.trimmed();
      addSubprogram(result);
      yy_push_state(Subprog);

      // need to be at the line after the definition and we have to take continuation lines into account
      current->startBodyLine = yyLineNr + s_lineCountPrepass + 1;
      current->startLine     = yyLineNr;
    }

<Subprog>{BS}                          {
      /* ignore white space */
   }

<Subprog>{ID}                          {
      QString text = QString::fromUtf8(yytext);

      current->m_entryName = text;
      modifiers[current_root][current->m_entryName.toLower()].returnName = current->m_entryName.toLower();

      if (ifType == IF_ABSTRACT || ifType == IF_SPECIFIC) {
         current_root->m_entryName.replace(QRegularExpression("\\$interface\\$"), text);
      }

      BEGIN(Parameterlist);
   }

<Parameterlist>"("                     {
      current->setData(EntryKey::Member_Args, "(");
   }

<Parameterlist>")"                     {
      QString tmpArgs = current->getData(EntryKey::Member_Args) + ")";
      tmpArgs = removeRedundantWhiteSpace(tmpArgs);
      current->setData(EntryKey::Member_Args, tmpArgs);

      addCurrentEntry(true);
      startScope(s_last_entry);
      BEGIN(SubprogBody);
   }

<Parameterlist>{COMMA}|{BS}            {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text);

      CommentInPrepass *c = locatePrepassComment(yyColNr - yyleng, yyColNr);

      if (c != nullptr) {
         if (current->argList.count() > 0) {
            current->argList[current->argList.count() - 1].docs = c->str;
         }
      }
   }

<Parameterlist>{ID}                    {
      // current->type not yet available
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text);

      Argument arg;
      arg.name  = text;
      arg.type  = "";

      current->argList.append(arg);
   }

<Parameterlist>{NOARGS}                {
      newLine();
      addCurrentEntry(true);
      startScope(s_last_entry);
      BEGIN(SubprogBody);
   }

<SubprogBody>result{BS}\({BS}{ID}      {
      QString text = QString::fromUtf8(yytext);

      if (functionLine) {
         result = text;
         result = result.right(result.length() - result.indexOf("(") - 1);
         result = result.trimmed();
         modifiers[current_root->parent()][current_root->m_entryName.toLower()].returnName = result;
      }

   }

 /*---- documentation comments ------------------------- */

<Variable,SubprogBody,ModuleBody,TypedefBody,TypedefBodyContains>"!<"  {
      /* backward docu comment */

      static const bool javaAutoBrief = Config::getBool("javadoc-auto-brief");

      if (v_type != V_IGNORE) {
         current->docLine  = yyLineNr;
         docBlockJavaStyle = false;
         docBlock.resize(0);

         docBlockJavaStyle = javaAutoBrief;
         startCommentBlock(true);
         yy_push_state(DocBackLine);

      } else {
         /* handle out of place !< comment as a normal comment */
         if (YY_START == String) {
            yyColNr -= yyleng;
            REJECT;
         }

         // "!" is ignored in strings
         // skip comment line (without docu comments "!>" "!<" )
         /* ignore further "!" and ignore comments in Strings */

         if ((YY_START != StrIgnore) && (YY_START != String)) {
            yy_push_state(YY_START);
            BEGIN(StrIgnore);
            debugStr = "*!";
         }
      }
   }

<DocBackLine>.*             {
      // contents of current comment line
      docBlock += QString::fromUtf8(yytext);
   }

<DocBackLine>"\n"{BS}"!"("<"|"!"+)     {
      // comment block (next line is also comment line)
      docBlock += "\n";       // \n is necessary for lists
      newLine();
   }

<DocBackLine>"\n"                {
      // comment block ends at the end of this line

      yyColNr -= 1;
      unput(*yytext);

      if (v_type == V_VARIABLE) {
         QSharedPointer<Entry> tmp_entry = current;

         // temporarily switch to the previous entry
         if (s_last_enum) {
            current = s_last_enum;
         } else {
            current = s_last_entry;
         }

         handleCommentBlock(docBlock, true);

         // switch back
         current = tmp_entry;

      } else if (v_type == V_PARAMETER) {
         subrHandleCommentBlock(docBlock,true);

      } else if (v_type == V_RESULT) {
         subrHandleCommentBlockResult(docBlock,true);

      }

      yy_pop_state();
      docBlock.resize(0);
}

<Start,SubprogBody,ModuleBody,TypedefBody,InterfaceBody,ModuleBodyContains,SubprogBodyContains,TypedefBodyContains,Enum>"!>"  {
      static const bool javaAutoBrief = Config::getBool("javadoc-auto-brief");

      yy_push_state(YY_START);
      current->docLine  = yyLineNr;
      docBlockJavaStyle = false;

      if (YY_START == SubprogBody) {
         docBlockInBody = true;
      }

      docBlock.resize(0);
      docBlockJavaStyle = javaAutoBrief;
      startCommentBlock(true);
      BEGIN(DocBlock);
   }

<DocBlock>.*            {
      // contents of current comment line
      docBlock += QString::fromUtf8(yytext);
   }

<DocBlock>"\n"{BS}"!"(">"|"!"+)     {
      // comment block (next line is also comment line)
      docBlock += "\n";       // \n is necessary for lists
      newLine();
   }

<DocBlock>"\n"                {
      // comment block ends at the end of this line
      yyColNr -= 1;
      unput(*yytext);
      handleCommentBlock(docBlock, true);
      yy_pop_state();
   }

 /*-----Prototype parsing ------------------------------ */
<Prototype>{BS}{SUBPROG}{BS_}       {
      BEGIN(PrototypeSubprog);
   }

<Prototype,PrototypeSubprog>{BS}{SCOPENAME}?{BS}{ID} {
      QString text = QString::fromUtf8(yytext);

      current->m_entryName = text.toLower().trimmed();
      BEGIN(PrototypeArgs);
   }

<PrototypeArgs>{

"("|")"|","|{BS_}          {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text);
   }

{ID}              {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text);

      Argument args;
      args.name = text.toLower();
      current->argList.append(args);
   }
}

<*>"\n"                                 {
      newLine();
      debugStr = "";
   }

 /*---- error: EOF in wrong state ---------------------- */

<*><<EOF>>                              {
      if (parsingPrototype) {
         yyterminate();

      } else if ( include_stack.isEmpty() ) {
         if (YY_START != INITIAL && YY_START != Start) {
            DBG_CTX((stderr,"==== Error: EOF reached in wrong state (end missing)"));
            scanner_abort();
         }

         yyterminate();

      } else {
         popBuffer();
      }
   }

<*>{LOG_OPER}                           {
      // fortran logical comparison keywords
   }

<*>.                                    {
      // ignore remaining text
   }

%%

#if 0
static void extractPrefix(QString &text)
{
  int prefixIndex = 0;
  int curIndex    = 0;
  bool cont       = true;

  const char* pre[] = {"RECURSIVE","IMPURE","PURE","ELEMENTAL"};

  while(cont) {
    cont = false;

    for(unsigned int i=0; i<4; i++) {
      if((prefixIndex=text.find(pre[i], curIndex, false))==0) {
        text.remove(0,strlen(pre[i]));
        text.trimmed();
        cont = true;
      }
    }
  }
}
#endif

static void newLine() {
   yyLineNr += s_lineCountPrepass + 1;
   s_lineCountPrepass = 0;

   comments.clear();
}

static CommentInPrepass *locatePrepassComment(int from, int to) {

   for(uint i = 0; i < comments.count(); i++) {
      // todo: optimize
      int c = comments.at(i)->column;

      if (c >= from && c <= to) {
         // comment for previous variable or parameter
         return comments.at(i);
      }
   }

   return nullptr;
}

static void updateVariablePrepassComment(int from, int to) {
   CommentInPrepass *c = locatePrepassComment(from, to);

   if (c != nullptr && v_type == V_VARIABLE) {
      s_last_entry->setData(EntryKey::Brief_Docs, c->str);

   } else if (c != nullptr && v_type == V_PARAMETER) {
      Argument *parameter = getParameter(argName);

      if (parameter) {
         parameter->docs = c->str;
      }
   }
}

static int getAmpersandAtTheStart(const QString &buf, int length)
{
   for (int i = 0; i < length; i++) {
      switch (buf[i].unicode()) {
         case ' ':
         case '\t':
            break;

         case '&':
            return i;

         default:
            return -1;
      }
   }

   return -1;
}

/* Returns ampersand index, comment start index or -1 if neither exist.*/
static int getAmpOrExclAtTheEnd(const QString &buf, int length, QChar ch)
{
   // Avoid ampersands in string and comments
   int parseState    = Start;
   int ampIndex      = -1;
   int commentIndex  = -1;

   QChar quoteSymbol = 0;
   quoteSymbol = ch;

   if (ch != '\0') {
      parseState = String;
   }

   for (int i = 0; i < length && parseState != Comment; i++) {
      // When in string, skip backslashes, legacy code

      if (parseState == String) {
         if (buf[i] == '\\') {
            i++;
         }
      }

      switch (buf[i].unicode()) {

         case '\'':
         case '"':
            // Close string, if quote symbol matches.
            // Quote symbol is set iff parseState==String
            if(buf[i] == quoteSymbol) {
               parseState = Start;
               quoteSymbol = 0;

            } else if(parseState == Start) {
               // Start new string, if not already in string or comment
               parseState  = String;
               quoteSymbol = buf[i];
            }

            ampIndex = -1; // invalidate prev ampersand
            break;

         case '!':
            // When in string or comment, ignore exclamation mark
            if (parseState == Start) {
               parseState   = Comment;
               commentIndex = i;
            }
            break;

         case ' ' :           // ignore whitespace
         case '\t':
         case '\n':           // this may be at the end of line
            break;

         case '&':
            ampIndex = i;
            break;

         default:
            ampIndex = -1;     // invalidate prev ampersand
      }
   }

   if (ampIndex >= 0) {
      return ampIndex;
   } else {
      return commentIndex;
   }
}

/* Although comments at the end of continuation line are grabbed by this function,
* we still do not know how to use them later in parsing.
*/
void truncatePrepass(int index)
{
   int length = s_inputStringPrepass.length();

   for (int i = index + 1; i < length; i++) {
      if (s_inputStringPrepass[i] == '!' && i < length - 1 && s_inputStringPrepass[i + 1] == '<') {
         // save comment
         struct CommentInPrepass *c = new CommentInPrepass(index, s_inputStringPrepass.right(length - i - 2));
         comments.append(c);
      }
   }

   s_inputStringPrepass.truncate(index);
}

// simplified way to know if this is fixed form
bool recognizeFixedForm(const QString &contents, FortranFormat format)
{
   int column    = 0;
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
            column   = 0;
            skipLine = false;
            break;

         case ' ':
            break;

         case '\000':
            return false;

         case '#':
            skipLine = true;
            break;

         case 'C':
         case 'c':
         case '*':
            if (column == 1) {
               return TRUE;
            }

            if (skipLine) {
               break;
            }
            return false;

         case '!':
            if (column > 1 && column < 7) {
               return FALSE;
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

/* change comments and bring line continuation character to previous line */
QString prepassFixedForm(const QString &contents, QVector<int> &hasContLine)
{
   int column                 = 0;
   int prevLineLength         = 0;
   int prevLineAmpOrExclIndex = -1;

   int curLine = 1;
   int skipped = 0;

   QChar prevQuote  = '\0';
   QChar thisQuote  = '\0';

   bool emptyLabel        = true;
   bool commented         = false;
   bool inSingle          = false;
   bool inDouble          = false;
   bool inBackslash       = false;
   bool fullCommentLine   = true;
   bool artificialComment = false;
   bool spaces            = true;

   QString newContents;

   for (int i = 0, pos = -1; true; ++i) {

      ++column;
      QChar c = contents[i];
      if (artificialComment && c != '\n') {

         if (c == '!' && spaces) {

            newContents += c;
            ++pos;

            artificialComment = false;
            spaces            = false;
            skipped           = 0;

            continue;

         } else if (c == ' ' || c == '\t')  {
            continue;

         } else {
           spaces = false;
           ++skipped;

           continue;
         }
      }

      ++pos;
      switch (c.unicode()) {
         case '\n':
            if (! fullCommentLine) {
               prevLineLength         = column;
               prevLineAmpOrExclIndex = getAmpOrExclAtTheEnd(contents.mid(i - prevLineLength + 1),
                  prevLineLength, prevQuote);

               if (prevLineAmpOrExclIndex == -1) {
                  prevLineAmpOrExclIndex = column - 1;
               }

               if (skipped) {
                  prevLineAmpOrExclIndex = -1;
                  skipped = 0;
                }

            } else {
               prevLineLength += column;

               // Even though a full comment line is not really a comment line it can be seen as one
               // An empty line is also seen as a comment line (small bonus)
               if (! hasContLine.empty()) {
                  hasContLine[curLine - 1] = 1;
               }
            }

            artificialComment = false;
            spaces            = true;
            fullCommentLine   = true;
            emptyLabel        = true;
            commented         = false;
            prevQuote         = thisQuote;

            newContents += c;

            column = 0;
            ++curLine;
            break;

         case ' ':
         case '\t':
            newContents += c;
            break;

         case '\000':
            if (! hasContLine.empty()) {
               return QString();
            }

            if (! newContents.endsWith('\n')) {
               newContents.append('\n');
            }

            return newContents;

         case '"':
         case '\'':
         case '\\':
            if ((column <= fixedCommentAfter) && (column != 6) && ! commented) {
               // some special cases in respect to strings and escaped string characters
               fullCommentLine = false;
               newContents    += c;

               if (c == '\\') {
                  inBackslash = ! inBackslash;
                  break;

               } else if (c == '\'') {

                  if (! inDouble) {
                     inSingle = !inSingle;

                     if (inSingle) {
                        thisQuote = c;

                     } else {
                        thisQuote = '\0';
                     }
                  }

                  break;

               } else if (c == '"') {

                  if (! inSingle) {
                     inDouble = ! inDouble;
                  }

                  if (inDouble) {
                     thisQuote = c;
                  } else {
                     thisQuote = '\0';
                  }

                  break;
               }
            }

            inBackslash = false;
            [[fallthrough]];

         case '#':
         case 'C':
         case 'c':
         case '*':
         case '!':
            if ((column <= fixedCommentAfter) && (column != 6)) {
               emptyLabel = false;

               if(column == 1) {
                  newContents += '!';
                  commented    = true;

               } else if ((c == '!') && ! inDouble && ! inSingle)  {
                  newContents += c;
                  commented    = true;

               } else {

                  if (! commented)   {
                     fullCommentLine = false;
                  }

                  newContents += c;
               }

               break;
            }
            [[fallthrough]];

         default:
            if (! commented && (column < 6) && c.isDigit()) {
               // remove any numbers in the first 5 positions

               newContents += ' ';

            } else if (column == 6 && emptyLabel) {
               // continuation

               if (! commented) {
                  fullCommentLine = false;
               }

               if (c != '0') {
                  // 0 not allowed as continuation character, see f95 standard paragraph 3.3.2.3
                  newContents += ' ';

                  if (prevLineAmpOrExclIndex == -1) {
                     // add & just before end of previous line
                     // first line is not a continuation line in code, just in snippets etc.

                     if (curLine != 1) {
                        newContents.insert(pos - 6, '&');
                     }
                     ++pos;

                  } else {
                     // add & just before end of previous line comment
                     // first line is not a continuation line in code, just in snippets etc
                     if (curLine != 1) {
                        int location = pos - 5 - prevLineLength + prevLineAmpOrExclIndex + skipped;

                        newContents.insert(location, '&');
                     }

                     skipped = 0;
                     ++pos;
                  }

                  if (! hasContLine.empty()) {
                     hasContLine[curLine - 1] = 1;
                  }

               } else {
                  // handle like space
                  newContents += c;

               }

               prevLineLength = 0;

            } else if ((column > fixedCommentAfter) && ! commented) {
               // first non commented non blank character after position fixedCommentAfter
               if (c == '&') {
                  newContents +=' ';

               } else if (c != '!') {
                  // not a possible start of a comment

                  newContents +=' ';

                  artificialComment = true;
                  spaces            = true;

                  skipped = 0;

               } else {

                  newContents += c;
                  commented    = true;
               }

            } else {
               if (! commented) {
                  fullCommentLine = false;
               }

               newContents += c;
               emptyLabel   = false;
            }

            break;
         }
   }

   if (! hasContLine.isEmpty()) {
      return QString();
   }

   if (! newContents.endsWith('\n')) {
      newContents.append('\n');
   }

   return newContents;
}

static void pushBuffer(QString &buffer)
{
   DBG_CTX((stderr, "--PUSH--%s", csPrintable(buffer)));

   buffer_stack.push(buffer.toUtf8());

   include_stack.push(YY_CURRENT_BUFFER);
   yy_switch_to_buffer(yy_scan_string( buffer_stack.top().constData() ));

   buffer = "";
}

static void popBuffer()
{
   DBG_CTX((stderr, "--POP--"));

   yy_switch_to_buffer(include_stack.pop());
   yy_delete_buffer(YY_CURRENT_BUFFER);

   buffer_stack.pop();
}

static void copyEntry(QSharedPointer<Entry> dest, QSharedPointer<Entry> src)
{
   dest->setData(EntryKey::File_Name,    src->getData(EntryKey::File_Name));
   dest->setData(EntryKey::Member_Type,  src->getData(EntryKey::Member_Type));
   dest->setData(EntryKey::Member_Args,  src->getData(EntryKey::Member_Args));
   dest->setData(EntryKey::Brief_Docs,   src->getData(EntryKey::Brief_Docs));
   dest->setData(EntryKey::Main_Docs,    src->getData(EntryKey::Main_Docs));

   dest->startLine     = src->startLine;
   dest->startBodyLine = src->startBodyLine;
   dest->endBodyLine   = src->endBodyLine;
   dest->argList       = src->argList;
}

/** fill empty interface module procedures with info from
    corresponding module subprogs
    @TODO: handle procedures in used modules
*/
void resolveModuleProcedures(QList<QSharedPointer<Entry>> &procedureList, QSharedPointer<Entry> root)
{
   if (procedureList.isEmpty()) {
      return;
   }

   // for all module procedures
   for (auto ce1 : procedureList)   {

      // for procedures in current module
      for (auto ce2 : root->children() ) {

         if (ce1->m_entryName == ce2->m_entryName) {
            copyEntry(ce1, ce2);
         }
      }
   }

   procedureList.clear();
}

/* Extracts string which resides within parentheses of provided string. */
static QString extractFromParens(const QString &name)
{
   QString extracted = name;
   int start = extracted.indexOf("(");

   if (start != -1) {
      extracted.remove(0, start + 1);
   }

   int end = extracted.lastIndexOf(")");
   if (end != -1) {
      int length = extracted.length();
      extracted.remove(end, length);
   }

   extracted = extracted.trimmed();

   return extracted;;
}

// remove useless spaces from bind statement
static QString extractBind(const QString &name)
{
  QString parensPart = extractFromParens(name);

  if (parensPart.length() == 1) {
    return "bind(C)";

  } else {
    // strip 'c'
    parensPart = parensPart.mid(1).trimmed();

    // strip ','
    parensPart = parensPart.mid(1).trimmed();

    // name part
    parensPart = parensPart.mid(4).trimmed();

    // = part
    parensPart = parensPart.mid(1).trimmed();

    return "bind(C, name=" + parensPart + ")";
  }
}

// Adds passed modifiers to these modifiers
SymbolModifiers& SymbolModifiers::operator|=(const SymbolModifiers &mdfs)
{
  if (mdfs.protection != NONE_P) {
      protection = mdfs.protection;
   }

   if (mdfs.direction != NONE_D) {
      direction = mdfs.direction;
   }

   optional |= mdfs.optional;

   if (! mdfs.dimension.isEmpty()) {
      dimension = mdfs.dimension;
   }

   allocatable |= mdfs.allocatable;
   external    |= mdfs.external;
   intrinsic   |= mdfs.intrinsic;
   protect     |= mdfs.protect;
   parameter   |= mdfs.parameter;
   pointer     |= mdfs.pointer;
   target      |= mdfs.target;
   save        |= mdfs.save;
   deferred    |= mdfs.deferred;

   nonoverridable |= mdfs.nonoverridable;

   nopass     |= mdfs.nopass;
   pass       |= mdfs.pass;
   passVar     = mdfs.passVar;
   bindVar     = mdfs.bindVar;

   contiguous |= mdfs.contiguous;
   volat      |= mdfs.volat;
   value      |= mdfs.value;

   return *this;
}

// Extracts  and adds passed modifier to these modifiers
SymbolModifiers& SymbolModifiers::operator|=(QString mdfString)
{
   QString mdfStringLower = mdfString.toLower();
   SymbolModifiers newMdf;

   if (mdfStringLower.indexOf("dimension") == 0) {
      newMdf.dimension = mdfStringLower;

   } else if (mdfStringLower.contains("intent")) {
      QString tmp = extractFromParens(mdfStringLower);

      bool isin  = tmp.contains("in");
      bool isout = tmp.contains("out");

      if (isin && isout) {
         newMdf.direction = SymbolModifiers::INOUT;

      } else if (isin) {
         newMdf.direction = SymbolModifiers::IN;

      } else if (isout) {
         newMdf.direction = SymbolModifiers::OUT;
      }

   } else if (mdfStringLower == "public")  {
      newMdf.protection = SymbolModifiers::PUBLIC;

   } else if (mdfStringLower == "private")  {
      newMdf.protection = SymbolModifiers::PRIVATE;

   } else if (mdfStringLower == "protected")  {
      newMdf.protect = true;

   } else if (mdfStringLower == "optional")  {
      newMdf.optional = true;

   } else if (mdfStringLower == "allocatable") {
      newMdf.allocatable = true;

   } else if (mdfStringLower == "external") {
      newMdf.external = true;

   } else if (mdfStringLower == "intrinsic") {
      newMdf.intrinsic = true;

   } else if (mdfStringLower == "parameter") {
      newMdf.parameter = true;

   } else if (mdfStringLower == "pointer")  {
      newMdf.pointer = true;

   } else if (mdfStringLower == "target") {
      newMdf.target = true;

   } else if (mdfStringLower == "save") {
      newMdf.save = true;

   } else if (mdfStringLower == "nopass") {
      newMdf.nopass = true;

   } else if (mdfStringLower == "deferred")    {
      newMdf.deferred = true;

   } else if (mdfStringLower == "non_overridable") {
      newMdf.nonoverridable = true;

   } else if (mdfStringLower == "contiguous") {
      newMdf.contiguous = true;

   } else if (mdfStringLower == "volatile") {
      newMdf.volat = true;

   } else if (mdfStringLower == "value") {
      newMdf.value = true;

   } else if (mdfStringLower.contains("pass")) {
      newMdf.pass = true;

      if (mdfStringLower.contains("(")) {
         newMdf.passVar = extractFromParens(mdfStringLower);

      } else {
         newMdf.passVar = "";
      }

   }  else if (mdfStringLower.startsWith("bind")) {
    // use the original string
    newMdf.bindVar = extractBind(mdfString);

   }

   (*this) |= newMdf;

   return *this;
}

/*! Find argument with given name in \a subprog entry. */
static Argument *findArgument(QSharedPointer<Entry> subprog, QString name, bool byTypeName = false)
{
   QString cname(name.toLower());

   for (unsigned int i = 0; i < subprog->argList.count(); i++) {
      Argument &arg = subprog->argList[i];

      if ((! byTypeName && arg.name.toLower() == cname) || (byTypeName && arg.type.toLower() == cname)) {
         return &arg;
      }
   }

   return 0;
}

/*! Find function with given name in entry. */
#if 0
static Entry *findFunction(Entry* entry, QString name)
{
  QString cname(name.toLower());

  EntryListIterator eli(*entry->children());
  Entry *ce;

  for (; (ce=eli.current()); ++eli) {
    if (ce->section != Entry::FUNCTION_SEC)
      continue;

    if (ce->m_entryName.toLower() == cname)
      return ce;
  }

  return 0;
}
#endif

// Apply modifiers stored in \a mdfs to the \a typeName string.
static QString applyModifiers(QString typeName, SymbolModifiers &mdfs)
{
   if (!mdfs.dimension.isEmpty()) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }

      typeName += mdfs.dimension;
   }

   if (mdfs.direction!=SymbolModifiers::NONE_D) {
      if (! typeName.isEmpty()) {
         typeName += ", ";
      }

      typeName += directionStrs[mdfs.direction];
   }

   if (mdfs.optional) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }

      typeName += "optional";
   }

   if (mdfs.allocatable) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }

      typeName += "allocatable";
   }

   if (mdfs.external) {
      if (! typeName.contains("external")) {
         if (! typeName.isEmpty()) {
             typeName += ", ";
         }

         typeName += "external";
      }
   }

   if (mdfs.intrinsic) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }

      typeName += "intrinsic";
   }

   if (mdfs.parameter) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }

      typeName += "parameter";
   }

   if (mdfs.pointer) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }

      typeName += "pointer";
   }

   if (mdfs.target) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }

      typeName += "target";
   }

   if (mdfs.save) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }
      typeName += "save";
   }

   if (mdfs.deferred) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }
      typeName += "deferred";
   }

   if (mdfs.nonoverridable) {
       if (! typeName.isEmpty()) {
          typeName += ", ";
      }

      typeName += "non_overridable";
   }

   if (mdfs.nopass) {
       if (! typeName.isEmpty()) {
          typeName += ", ";
      }
      typeName += "nopass";
   }

   if (mdfs.pass) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }
      typeName += "pass";

      if (! mdfs.passVar.isEmpty()) {
         typeName += "(" + mdfs.passVar + ")";
      }
   }

   if (! mdfs.bindVar.isEmpty()) {
    if (! typeName.isEmpty()) {
       typeName += ", ";
    }

    typeName += mdfs.bindVar;
   }

   if (mdfs.protection == SymbolModifiers::PUBLIC) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }

      typeName += "public";

   } else if (mdfs.protection == SymbolModifiers::PRIVATE) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }
      typeName += "private";
   }

   if (mdfs.protect) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }

      typeName += "protected";
   }

   if (mdfs.contiguous) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }

      typeName += "contiguous";
   }

   if (mdfs.volat) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }

      typeName += "volatile";
   }

   if (mdfs.value) {
      if (! typeName.isEmpty()) {
          typeName += ", ";
      }

      typeName += "value";
   }

   return typeName;
}

// Apply modifiers stored in \a mdfs to the  arg argument
static void applyModifiers(Argument *arg, SymbolModifiers &mdfs)
{
   QString tmpType = arg->type;
   arg->type   = applyModifiers(tmpType, mdfs);
}

// Apply modifiers stored in mdfs to the tmpEntry entry
static void applyModifiers(QSharedPointer<Entry> xEntry, SymbolModifiers &mdfs)
{
   QString tmpType = xEntry->getData(EntryKey::Member_Type);
   xEntry->setData(EntryKey::Member_Type, applyModifiers(tmpType, mdfs));

   if (mdfs.protection == SymbolModifiers::PUBLIC) {
      xEntry->protection = Public;

   } else if (mdfs.protection == SymbolModifiers::PRIVATE) {
      xEntry->protection = Private;

   }
}

/* Starts the new scope in fortran program. Consider using this function when
 * starting module, interface, function or other program block.
 */
static void startScope(QSharedPointer<Entry> scope)
{
  current_root = scope;          // start substructure

  QMap<QString, SymbolModifiers> mdfMap;
  modifiers.insert(scope, mdfMap);
}

// Ends scope in fortran program: may update subprogram arguments or module variable attributes
static bool endScope(QSharedPointer<Entry> scope, bool isGlobalRoot)
{
   if (s_global_scope == scope) {
      s_global_scope = QSharedPointer<Entry>();
      s_fake_entry   = false;
      return true;
   }

   if (s_fake_entry) {
      return true;
   }

   if (current_root->parent() || isGlobalRoot) {
      current_root = current_root->parent();          // end substructure

   } else {
      fprintf(stderr, "Parse error in end <scopename>\n");
      scanner_abort();
      return false;
   }

   // update variables or subprogram arguments with modifiers
   QMap<QString, SymbolModifiers> &mdfsMap = modifiers[scope];

   if (scope->section == Entry::FUNCTION_SEC) {
      // iterate all symbol modifiers of the scope

      for (auto iter = mdfsMap.begin(); iter != mdfsMap.end(); iter++) {
         Argument *arg = findArgument(scope, iter.key());

         if (arg) {
            applyModifiers(arg, iter.value());
         }
      }

      // find return type for function
      QString returnName = modifiers[current_root][scope->m_entryName.toLower()].returnName.toLower();

      if (modifiers[scope].contains(returnName)) {
         scope->setData(EntryKey::Member_Type,  modifiers[scope][returnName].type);     // returning type works
         applyModifiers(scope, modifiers[scope][returnName]);                           // returning array works
      }
   }

   if (scope->section == Entry::CLASS_SEC) {
      // was INTERFACE_SEC

      if (scope->parent()->section == Entry::FUNCTION_SEC) {
         // interface within function
         // iterate functions of interface and
         // try to find types for dummy(ie. argument) procedures.

         int count = 0;
         int found = false;

         for (auto ce : scope->children()){
            count++;

            if (ce->section != Entry::FUNCTION_SEC) {
               continue;
            }

            Argument *arg = findArgument(scope->parent(), ce->m_entryName, true);

            if (arg != 0) {
               // set type of dummy procedure argument to interface
               arg->name = arg->type;
               arg->type = scope->m_entryName;
            }

            if (ce->m_entryName.toLower() == scope->m_entryName.toLower()) {
               found = true;
            }
         }

         if ((count == 1) && found) {
            // clear all modifiers of the scope
            modifiers.remove(scope);

            scope->parent()->removeSubEntry(scope);
            scope = QSharedPointer<Entry>();

            return true;
         }
      }
   }

   if (scope->section != Entry::FUNCTION_SEC) {
      // not function section
      // iterate variables: get and apply modifiers

      for (auto ce : scope->children()) {

         if (ce->section != Entry::VARIABLE_SEC && ce->section != Entry::FUNCTION_SEC) {
            continue;
         }

         if (mdfsMap.contains(ce->m_entryName.toLower())) {
            applyModifiers(ce, mdfsMap[ce->m_entryName.toLower()]);
         }
      }
   }

   // clear all modifiers of the scope
   modifiers.remove(scope);

   return true;
}

#if 0
//! Return full name of the entry. Sometimes we must combine several names recursively.
static QString getFullName(Entry *e)
{
   QString name = e->m_entryName;
   if (e->section == Entry::CLASS_SEC || !e->parent() || e->parent()->m_entryName.isEmpty()) {
      return name;
   }

   return getFullName(e->parent()) + "::" + name;
}
#endif

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

static void initParser()
{
   s_last_entry = QSharedPointer<Entry>();
}

void initEntry()
{
   if (typeMode) {
      current->protection = typeProtection;
   } else {
      current->protection = defaultProtection;
   }

   current->mtype    = MethodType::Method;
   current->virt     = Specifier::Normal;
   current->m_static = false;
   current->m_srcLang = SrcLangExt_Fortran;

   initGroupInfo(current);
}

/**
  adds current entry to current_root and creates new current
*/
static void addCurrentEntry(bool caseInsensitive)
{
   if (caseInsensitive) {
      current->m_entryName = current->m_entryName.toLower();
   }

   current_root->addSubEntry(current);
   s_last_entry = current;

   current = QMakeShared<Entry>();
   initEntry();
}

static int max(int a, int b)
{
   return a > b ? a : b;
}

static void addModule(const QString &name, bool isModule)
{
   if (isModule) {
      current->section = Entry::NAMESPACE_SEC;
   } else {
     current->section = Entry::FUNCTION_SEC;
   }

   if (! name.isEmpty()) {
      current->m_entryName = name;

   } else {
      QString fname = yyFileName;

      int index = max(fname.lastIndexOf('/'), fname.lastIndexOf('\\'));
      fname = fname.right(fname.length() - index - 1);
      fname = fname.prepend("__").append("__");
      current->m_entryName = fname;
   }

   current->setData(EntryKey::Member_Type, "program");

   current->setData(EntryKey::File_Name, yyFileName);
   current->startBodyLine = yyLineNr;       // used for source reference
   current->startLine     = yyLineNr;
   current->protection    = Public;

   addCurrentEntry(true);
   startScope(s_last_entry);
}

static void addSubprogram(const QString &text)
{
   DBG_CTX((stderr,"1=========> got subprog, type: %s\n", text));

   subrCurrent.prepend(current);
   current->section = Entry::FUNCTION_SEC;

   QString subtype = text;
   subtype      = subtype.toLower().trimmed();
   functionLine = (subtype.indexOf("function") != -1);

   QString tmpType = current->getData(EntryKey::Member_Type);

   tmpType += " " + subtype;
   tmpType = tmpType.trimmed();

   current->setData(EntryKey::Member_Type, tmpType);
   current->setData(EntryKey::Member_Args, "");

   current->setData(EntryKey::File_Name, yyFileName);
   current->startBodyLine = yyLineNr;    // used for source reference start of body of routine
   current->startLine     = yyLineNr;    // used for source reference start of definition

   current->argList.clear();
   docBlock.resize(0);
}

// Adds interface to the root entry
static void addInterface(const QString &name, InterfaceType type)
{
   if (YY_START == Start) {
     addModule();
     yy_push_state(ModuleBody);             // anon program
   }

   current->section = Entry::CLASS_SEC;
   current->m_entryName = name;

   current->m_traits.clear();
   current->m_traits.setTrait(Entry::Virtue::Interface);

   switch (type) {
      case IF_ABSTRACT:
         current->setData(EntryKey::Member_Type, "abstract");
         break;

      case IF_GENERIC:
         current->setData(EntryKey::Member_Type, "generic");
         break;

      case IF_SPECIFIC:
      case IF_NONE:
      default:
         current->setData(EntryKey::Member_Type, "");

   }

   /* if type is part of a module, mod name is necessary for output */
   if ((current_root) && (current_root->section ==  Entry::CLASS_SEC || current_root->section ==  Entry::NAMESPACE_SEC))  {
      current->m_entryName = current_root->m_entryName + "::" + current->m_entryName;
   }

   current->setData(EntryKey::File_Name, yyFileName);
   current->startBodyLine = yyLineNr;
   current->startLine     = yyLineNr;

   addCurrentEntry(true);
}


/*! Get the argument name
 */
static Argument *getParameter(const QString &name)
{
  Argument *retval = nullptr;

   for (auto &a : current_root->argList) {
      if (a.name.toLower() == name.toLower()) {
         retval = &a;
         break;
      }
   }

   return retval;
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

static void handleCommentBlock(const QString &doc, bool brief)
{
   static const bool hideInBodyDocs = Config::getBool("hide-in-body-docs");

   bool needsEntry = false;
   int position    = 0;

   if (docBlockInBody && hideInBodyDocs) {
      docBlockInBody = false;
      return;
   }

   int lineNum = brief ? current->briefLine : current->docLine;

   while (parseCommentBlock(s_thisParser, docBlockInBody ? subrCurrent.first() : current,
         doc, yyFileName, lineNum,
         docBlockInBody ? false : brief,
         docBlockInBody ? false : docBlockJavaStyle,
         docBlockInBody, defaultProtection, position, needsEntry)) {

      if (needsEntry) {
         addCurrentEntry(false);
      }
   }


   if (needsEntry) {
      addCurrentEntry(false);
   }

   docBlockInBody = false;
}

/// Handle parameter description as defined after the declaration of the parameter
static void subrHandleCommentBlock(const QString &doc, bool brief)
{
   QString loc_doc;
   loc_doc = doc.trimmed();

   // temporarily switch to the entry of the subroutine / function
   QSharedPointer<Entry> tmp_entry = current;
   current = subrCurrent.first();

   // still in the specification section so no inbodyDocs yet, but parameter documentation
   current->setData(EntryKey::Inbody_Docs,   "");

   // strip \\param or @param, so we can do some extra checking, we will add it later on again.
   if (loc_doc.indexOf("\\param") == 0) {
      loc_doc = loc_doc.right(loc_doc.length() - strlen("\\param")).trimmed();

   } else if (loc_doc.indexOf("@param") == 0) {
      loc_doc = loc_doc.right(loc_doc.length() - strlen("@param")).trimmed();

   } else {
      // Do nothing work has been done by stripPrefix; (void)loc_doc: to overcome 'empty controlled statement' warning
      loc_doc = loc_doc.trimmed();

   }

   // direction as defined with the declaration of the parameter
   int dir1 = modifiers[current_root][argName.toLower()].direction;

   // in description [in] is specified

   if (loc_doc.toLower().indexOf(directionParam[SymbolModifiers::IN]) == 0) {
      // check if with the declaration intent(in) or nothing has been specified

      if ((directionParam[dir1] == directionParam[SymbolModifiers::NONE_D]) ||
                  (directionParam[dir1] == directionParam[SymbolModifiers::IN])) {

         // strip direction
         loc_doc = loc_doc.right(loc_doc.length() - directionParam[SymbolModifiers::IN].length());

         // in case of empty documentation or (now) just name, consider it as no documentation
         if (loc_doc.isEmpty() || (loc_doc.toLower() == argName.toLower())) {
           // reset current back to the part inside the routine
           current = tmp_entry;
           return;
         }

         handleCommentBlock(QString("\n\n@param ") + directionParam[SymbolModifiers::IN] + " " +
                  argName + " " + loc_doc, brief);

      } else {
         // something different specified, give warning and leave error

         warn(yyFileName, yyLineNr, "Routine: " + current->m_entryName + current->getData(EntryKey::Member_Args) +
                  " inconsistency between intent attribute and documentation for parameter: " + argName);

         handleCommentBlock(QString("\n\n@param ") + directionParam[dir1] + " " +
                  argName + " " + loc_doc, brief);
      }

   } else if (loc_doc.toLower().indexOf(directionParam[SymbolModifiers::OUT]) == 0) {
      // analogous to the [in] case, here [out] direction specified

      if ((directionParam[dir1] == directionParam[SymbolModifiers::NONE_D]) ||
                  (directionParam[dir1] == directionParam[SymbolModifiers::OUT])) {

         loc_doc = loc_doc.right(loc_doc.length() -directionParam[SymbolModifiers::OUT].length());

         if (loc_doc.isEmpty() || (loc_doc.toLower() == argName.toLower())) {
           current = tmp_entry;
           return;
         }

         handleCommentBlock(QString("\n\n@param ") + directionParam[SymbolModifiers::OUT] + " " +
                  argName + " " + loc_doc, brief);

      } else {
         warn(yyFileName, yyLineNr, "Routine: " + current->m_entryName + current->getData(EntryKey::Member_Args) +
                  " inconsistency between intent attribute and documentation for parameter: " + argName);

         handleCommentBlock(QString("\n\n@param ") + directionParam[dir1] + " " +
                  argName + " " + loc_doc, brief);;
      }

   } else if (loc_doc.toLower().indexOf(directionParam[SymbolModifiers::INOUT]) == 0)  {
      // analogous to the [in] case, here [in,out] direction specified

      if ((directionParam[dir1] == directionParam[SymbolModifiers::NONE_D]) ||
                  (directionParam[dir1] == directionParam[SymbolModifiers::INOUT])) {

         loc_doc = loc_doc.right(loc_doc.length() - directionParam[SymbolModifiers::INOUT].length());

         if (loc_doc.isEmpty() || (loc_doc.toLower() == argName.toLower())) {
            current=tmp_entry;
            return;
         }

         handleCommentBlock(QString("\n\n@param ") + directionParam[SymbolModifiers::INOUT] + " " +
                  argName + " " + loc_doc, brief);

      } else {
         warn(yyFileName,yyLineNr, "Routine: " + current->m_entryName + current->getData(EntryKey::Member_Args) +
                  " inconsistency between intent attribute and documentation for parameter: " + argName);

         handleCommentBlock(QString("\n\n@param ") + directionParam[dir1] + " " +
                  argName + " " + loc_doc, brief);
      }

   }  else {
      // analogous to the [in] case; here no direction specified

      if (loc_doc.isEmpty() || (loc_doc.toLower() == argName.toLower())) {
         current = tmp_entry;
         return;
      }

      handleCommentBlock(QString("\n\n@param ") + directionParam[dir1] + " " +
                  argName + " " + loc_doc, brief);
  }

  // reset current back to the part inside the routine
  current = tmp_entry;
}

// Handle result description as defined after the declaration of the parameter
static void subrHandleCommentBlockResult(const QString &doc, bool brief)
{
   QString loc_doc;
   loc_doc = doc.trimmed();

   // temporarily switch to the entry of the subroutine / function
   QSharedPointer<Entry> tmp_entry = current;
   current = subrCurrent.first();

   // Still in the specification section so no inbodyDocs yet, but parameter documentation
   current->setData(EntryKey::Inbody_Docs,   "");

   // strip \\returns or @returns, will be added later

   if (loc_doc.indexOf("\\returns") == 0) {
      loc_doc = loc_doc.right(loc_doc.length() - strlen("\\returns")).trimmed();

   } else if (loc_doc.indexOf("\\return") == 0) {
      loc_doc = loc_doc.right(loc_doc.length() - strlen("\\return")).trimmed();

   } else if (loc_doc.indexOf("@returns") == 0) {
      loc_doc = loc_doc.right(loc_doc.length() - strlen("@returns")).trimmed();

   } else if (loc_doc.indexOf("@return") == 0) {
      loc_doc = loc_doc.right(loc_doc.length() - strlen("@return")).trimmed();

   } else {
      // Do nothing work has been done by stripPrefix; (void)loc_doc: to overcome 'empty controlled statement' warning
      loc_doc = loc_doc.trimmed();

   }

   if (loc_doc.isEmpty() || (loc_doc.toLower() == argName.toLower())) {
      current = tmp_entry;
      return;
   }

   handleCommentBlock(QString("\n\n@returns ") + loc_doc, brief);

   // reset current back to the part inside the routine
   current = tmp_entry;
}

static void parseMain(const QString &fileName, const QString &fileBuf, QSharedPointer<Entry> &rt, FortranFormat format)
{
   initParser();

   defaultProtection      = Public;
   s_inputString          = fileBuf;
   s_inputPosition        = 0;
   s_inputStringPrepass   = QString();
   s_inputPositionPrepass = 0;

   current_root           = rt;
   global_root            = rt;

   inputFile.setFileName(fileName);

   if (inputFile.open(QIODevice::ReadOnly)) {
      s_isFixedForm = recognizeFixedForm(fileBuf, format);

      if (s_isFixedForm) {
         msg("Prepassing fixed form of %s\n", csPrintable(fileName));

         QVector<int> tmp;
         s_inputString = prepassFixedForm(fileBuf, tmp);

      } else if (! s_inputString.endsWith('\n') ) {
         s_inputString += '\n';

      }

      yyLineNr       = 1;
      yyFileName     = fileName;
      s_global_scope = rt;
      s_fake_entry   = false;

      msg("Parsing %s\n", csPrintable(yyFileName));

      startScope(rt);                                      // implies current_root = rt
      initParser();
      groupEnterFile(yyFileName, yyLineNr);

      current              = QMakeShared<Entry>();
      current->m_srcLang   = SrcLangExt_Fortran;
      current->m_entryName = yyFileName;
      current->section     = Entry::SOURCE_SEC;

      current_root->addSubEntry(current);

      file_root            = current;
      current              = QMakeShared<Entry>();
      current->m_srcLang   = SrcLangExt_Fortran;

      yyrestart( yyin );
      BEGIN( Start );
      yylex();

      groupLeaveFile(yyFileName, yyLineNr);

      if (s_global_scope != nullptr && ! s_fake_entry) {
         endScope(current_root, true);                     // global root
      }

      rt->setData(EntryKey::Source_Text, "");
      current = QSharedPointer<Entry>();

      moduleProcedures.clear();

      if (s_isFixedForm) {
         s_inputString = "";
      }

      inputFile.close();
  }
}

void Fortran_Parser::parseInput(const QString &fileName, const QString &fileBuf,
                  QSharedPointer<Entry> root, enum ParserMode mode, QStringList &includedFiles, bool useClang)
{
   (void) mode;
   (void) includedFiles;
   (void) useClang;

   s_thisParser = this;

   printlex(yy_flex_debug, true, __FILE__, fileName);
   ::parseMain(fileName, fileBuf, root, m_format);
   printlex(yy_flex_debug, false, __FILE__, fileName);
}

void Fortran_Parser::parseCode(CodeGenerator &codeOutIntf, const QString &scopeName,
                  const QString &input, SrcLangExt, bool isExampleBlock, const QString &exampleName,
                  QSharedPointer<FileDef> fileDef, int startLine, int endLine, bool inlineFragment,
                  QSharedPointer<MemberDef> memberDef, bool showLineNumbers, QSharedPointer<Definition>
                  searchCtx, bool collectXRefs)
{
   ::parseFortranCode(codeOutIntf, scopeName, input, isExampleBlock, exampleName, fileDef,
                  startLine, endLine, inlineFragment, memberDef, showLineNumbers, searchCtx,
                  collectXRefs, m_format);
}

bool Fortran_Parser::needsPreprocessing(const QString &extension) const
{
   // use preprocessor only for upper case extensions
   return extension != extension.toLower();
}

void Fortran_Parser::resetCodeParserState()
{
   ::resetFortranCodeParserState();
}

void Fortran_Parser::parsePrototype(const QString &text)
{
   QString buffer = text;

   pushBuffer(buffer);
   parsingPrototype = TRUE;

   BEGIN(Prototype);
   yylex();

   parsingPrototype = FALSE;
   popBuffer();
}


static void scanner_abort()
{
   fprintf(stderr, "********************************************************************\n");
   fprintf(stderr, "Error in file %s line: %d, state: %d\n", csPrintable(yyFileName), yyLineNr, YY_START);
   fprintf(stderr, "********************************************************************\n");

   bool start = FALSE;

   for (auto ce : global_root->children() ){
      if (ce == file_root) {
         start = TRUE;
      }

      if (start) {
         ce->reset();
      }
   }

   // dummy call to avoid compiler warning
   (void)yy_top_state();

   return;
}