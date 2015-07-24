/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim
 * Copyright (C) 1997-2014 by Dimitri van Heesch. 
   
*************************************************************************/

%{

#include <QFile>
#include <QHash>
#include <QStack>
#include <QVector>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include <arguments.h>
#include <config.h>
#include <commentscan.h>
#include <code.h>
#include <doxy_globals.h>
#include <entry.h>
#include <message.h>
#include <defargs.h>
#include <language.h>
#include <parser_clang.h>
#include <parser_cstyle.h>
#include <util.h>

#define YY_NO_INPUT 1
#define YY_NEVER_INTERACTIVE 1

static ParserInterface *g_thisParser;
static QString          inputString;
static int              inputPosition;
static QFile            inputFile;
static int              lastContext;
static int              lastCContext;
static int              lastDocContext;
static int              lastCPPContext;
static int              lastSkipSharpContext;
static int              lastSkipRoundContext;
static int              lastStringContext;
static int              lastCurlyContext;
static int              lastRoundContext;
static int              lastSquareContext;
static int              lastInitializerContext;
static int              lastClassTemplSpecContext;
static int              lastPreLineCtrlContext;
static int              lastSkipVerbStringContext;
static int              lastCommentInArgContext;
static int              lastRawStringContext;
static int              lastCSConstraint;
static int              lastHereDocContext;
static int              lastDefineContext;
static int              lastAlignAsContext;
static Protection       protection;
static Protection       baseProt;

static int sharpCount   = 0;
static int roundCount   = 0;
static int curlyCount   = 0;
static int squareCount  = 0;
static int padCount     = 0;

static QSharedPointer<Entry> current_root;
static QSharedPointer<Entry> global_root;
static QSharedPointer<Entry> current;
static QSharedPointer<Entry> previous;
static QSharedPointer<Entry> tempEntry;
static QSharedPointer<Entry> firstTypedefEntry;
static QSharedPointer<Entry> memspecEntry;

static int yyLineNr     = 1;
static int yyBegLineNr  = yyLineNr;
static int yyColNr      = 1;
static int yyBegColNr   = yyColNr;
static int anonCount    = 0;
static int anonNSCount  = 0;

static QString          yyFileName;
static MethodTypes      mtype;
static bool             gstat;
static bool             removeSlashes;
static Specifier        virt;
static Specifier        baseVirt;

static QString          msType, msName, msArgs;
static bool             isTypedef;
static int              tmpDocType;
static QString          sectionLabel;
static QString          sectionTitle;
static QString          funcPtrType;
static QString          templateStr;
static QString          aliasName;
static QString          baseName;
static QString         *specName;
static QString          formulaText;
static bool             useOverrideCommands = false;

static SrcLangExt       language;
static bool             insideIDL    = false;       //!< processing IDL code?
static bool             insideJava   = false;       //!< processing Java code?
static bool             insideCSharp = false;       //!< processing C# code?
static bool             insideD      = false;       //!< processing D code?
static bool             insidePHP    = false;       //!< processing PHP code?
static bool             insideObjC   = false;       //!< processing Objective C code?
static bool             insideCli    = false;       //!< processing C++/CLI code?
static bool             insideJS     = false;       //!< processing JavaScript code?
static bool             insideCpp    = true;        //!< processing C/C++ code

static bool             insideCppQuote     = false;
static bool             insideProtocolList = false;

static bool             s_clangParser = false;

static int              argRoundCount;
static int              argSharpCount;
static int              currentArgumentContext;
static int              lastCopyArgStringContext;
static int              lastCopyArgContext;

static QString          *copyArgString;
static QString          fullArgString;
static ArgumentList     *currentArgumentList;
static char             lastCopyArgChar;

static QString          *pCopyQuotedString;
static QString          *pCopyRoundString;
static QString          *pCopyCurlyString;
static QString          *pCopyRawString;

static QString          *pCopyCurlyGString;
static QString          *pCopyRoundGString;
static QString          *pCopyQuotedGString;
static QString          *pCopyHereDocGString;
static QString          *pCopyRawGString;
static QString          *pSkipVerbString;
static QStack<Grouping *>  autoGroupStack;

static bool             insideFormula;
static bool             insideTryBlock = false;
static bool             insideCode;
static bool             needsSemi;
static int              initBracketCount;

static QString          oldStyleArgType;
static QString          docBackup;
static QString          briefBackup;

static int              docBlockContext;
static QString          docBlock;
static QString          docBlockName;
static bool             docBlockInBody;
static bool             docBlockAutoBrief;
static char             docBlockTerm;

static QString          idlAttr;
static QString          idlProp;
static bool             odlProp;

static bool             g_lexInit = false;
static bool             externC;

static QString          g_delimiter;

static int              g_column;
static int              g_fencedSize = 0;
static bool             g_nestedComment = 0;

static void initParser()
{
  sectionLabel.resize(0);
  sectionTitle.resize(0);
  baseName.resize(0);
  formulaText.resize(0);

  protection = Public;
  baseProt   = Public;
  sharpCount = 0;
  roundCount = 0;
  curlyCount = 0;
  mtype      = MethodTypes::Method;
  gstat      = false;
  virt       = Normal;
  baseVirt   = Normal;
  isTypedef  = false;

  autoGroupStack.clear();
  insideTryBlock = false;
  insideFormula  = false;
  insideCode     = false;

   insideCli = Config::getBool("cpp-cli-support");

   firstTypedefEntry = QSharedPointer<Entry>();
   previous          = QSharedPointer<Entry>();
   tempEntry         = QSharedPointer<Entry>();
   memspecEntry      = QSharedPointer<Entry>();
}

static void initEntry()
{
  if (insideJava)  {
    protection = (current_root->m_specFlags.spec & (Entry::Interface | Entry::Enum)) ? Public : Package;
  }
  current->protection = protection;
  current->mtype      = mtype;
  current->virt       = virt;
  current->stat       = gstat;
  current->lang       = language;

  initGroupInfo(current);
  isTypedef = false;
}

///// remove any automatic grouping and add new one (if given)
//static void setCurrentGroup( QCString *newgroup, Grouping::GroupPri_t pri )
//{
//   /* remove auto group name from current entry and discard it */
//   Grouping *g = current->groups->first();
//   int i = 0;
//   while (g)
//   {
//     if (g->pri <= Grouping::GROUPING_AUTO_DEF)
//     {
//       current->groups->remove(i);
//       i--;
//     }
//     g=current->groups->next();
//     i++;
//   }
//
//   /* use new group name instead? */
//   if ( newgroup )
//   {
//      current->groups->append(new Grouping(*newgroup, pri));
//   }
//}

//static int newMemberGroupId()
//{
//  static int curGroupId=0;
//  return curGroupId++;
//}


// forward declarations
// static void startGroupInDoc();
// static void endGroup();


static void lineCount()
{
  static int tabSize = Config::getInt("tab-size");
  const char *p;

  for (p = yytext ; *p ; ++p ) {

      if (*p == '\n') {
         yyLineNr++;
         g_column = 0;
         yyColNr  = 1;

      } else if (*p == '\t') {
         g_column += tabSize - (g_column % tabSize);

      } else {
         g_column++;
         yyColNr++;
      }
   }
}

static int computeIndent(const QString &str, int startIndent)
{
   static int tabSize = Config::getInt("tab-size");
   int col = startIndent;
  
   for (auto c : str) { 
 
      if (c == '\t') {
         col += tabSize-(col % tabSize);
      
      } else if (c == '\n') {
         col = 0; 
      
      } else {
         col++;

      }
  }

  return col;
}

static void addType(QSharedPointer<Entry> ce)
{
   uint tl = ce->type.length();

   if ( tl > 0 && ! ce->name.isEmpty() && ce->type.at(tl - 1) != '.') {
      ce->type += ' ' ;
   }

   ce->type += ce->name ;
   ce->name.resize(0) ;
   tl = ce->type.length();

   if ( tl > 0 && ! ce->args.isEmpty() && ce->type.at(tl - 1) != '.') {
      ce->type += ' ' ;
   }

   ce->type += ce->args ;
   ce->args.resize(0) ;
   ce->argList.clear();    
}

static QString stripQuotes(const QString &str)
{ 
   QString name = str;

   if (str.isEmpty()) {
      return name;
   }

   if (name.startsWith('\"') && name.endsWith('\"')) {
     name = name.mid(1, name.length()-2);
   }

   return name;
}

static void startCommentBlock(bool);
static void handleCommentBlock(const QString &doc, bool brief);
static void handleParametersCommentBlocks(ArgumentList *al);

static bool nameIsOperator(QString &name)
{
   int i = name.indexOf("operator");

   if (i == -1) {
      return false;
   }

   if (i == 0) {
     int len = name.length();

     if (len == 8 || ! isId(name.at(8))) {
         // case operator ::X
         return true;
      }
   }

   if (i > 0) {
      int len = name.length();

      if ( (len == (i + 8)) || (! isId(name.at(i - 1)) && ! isId(name.at(i + 8)))  ) {
         // case X::operator
         return true;
      }
   }

   // case TEXToperatorTEXT

  return false;
}

static void setContext()
{
  QString fileName = yyFileName;

  language     = getLanguageFromFileName(fileName);
  insideIDL    = language==SrcLangExt_IDL;
  insideJava   = language==SrcLangExt_Java;
  insideCSharp = language==SrcLangExt_CSharp;
  insideD      = language==SrcLangExt_D;
  insidePHP    = language==SrcLangExt_PHP;
  insideObjC   = language==SrcLangExt_ObjC;
  insideJS     = language==SrcLangExt_JS;
  insideCpp    = language==SrcLangExt_Cpp;

  if ( insidePHP ) {
    useOverrideCommands = true;
  }
}

static void prependScope()
{
   if (current_root->section & Entry::SCOPE_MASK) {
      current->name.prepend(current_root->name + "::");

      if (current_root->tArgLists) {

         if (current->tArgLists == 0) {
            current->tArgLists = new QList<ArgumentList>;
         }

         *current->tArgLists = *current_root->tArgLists;
      }
   }
}

/*! Returns true iff the current entry could be a K&R style C function */
static bool checkForKnRstyleC()
{
   if (yyFileName.right(2).toLower() != ".c") {
      // must be a C file
      return false;   
   }

   if (current->argList.isEmpty()) {
      // must have arguments
      return false;   
   }

   for (auto a : current->argList) {
      // K&R style arguments do not have a type, but DoxyPress expects a type
      // so it will think the argument has no name
      if (a.type.isEmpty() || ! a.name.isEmpty()) {
         return false;
      }
   }

  return true;
}

static void splitKnRArg(QString &oldStyleArgPtr, QString &oldStyleArgName)
{
   int si = current->args.length();

   if (oldStyleArgType.isEmpty()) {
      // new argument
      static QRegExp re("\\([^)]*\\)");

      int bi1 = re.lastIndexIn(current->args);
      int bi2 = bi1 != -1 ? re.lastIndexIn(current->args, bi1 - 1) : -1;
      QChar c;

      if (bi1 != -1 && bi2 != -1) { // found something like "int (*func)(int arg)"
         int s = bi2 + 1;
         oldStyleArgType = current->args.left(s);

         int i = s;

         while (i < si && ((c = current->args.at(i)) == '*' || c.isSpace() )) {
            i++;
         }

         oldStyleArgType += current->args.mid(s, i - s);
         s = i;

         while (i < si && isId(current->args.at(i))) {
            i++;
         }

         oldStyleArgName = current->args.mid(s, i - s);
         oldStyleArgType += current->args.mid(i);

      } else if (bi1 != -1) { 
         // redundant braces like in "int (*var)"
         int s = bi1;
         oldStyleArgType = current->args.left(s);
         s++;

         int i = s + 1;
         while (i < si && ((c = current->args.at(i)) == '*' || c.isSpace() )) {
            i++;
         }

         oldStyleArgType += current->args.mid(s, i - s);
         s = i;
         while (i < si && isId(current->args.at(i))) {
            i++;
         }
         oldStyleArgName = current->args.mid(s, i - s);

      } else {
         // normal "int *var"
         int l = si;
         int i = l - 1;
         int j;
         QChar c;

         // look for start of name in "type *name"
         while (i >= 0 && isId(current->args.at(i))) {
            i--;
         }

         j = i + 1;
         // look for start of *'s
         while (i >= 0 && ((c = current->args.at(i)) == '*' || c.isSpace() )) {
            i--;
         }

         i++;

         if (i != l) {
            oldStyleArgType = current->args.left(i);
            oldStyleArgPtr  = current->args.mid(i, j - i);
            oldStyleArgName = current->args.mid(j).trimmed();

         } else {
            oldStyleArgName = current->args.trimmed();
         }
      }

   } else { 
      // continuation like *arg2 in "int *args,*arg2"
      int l = si;
      int j = 0;
      QChar c;

      while (j < l && ((c = current->args.at(j)) == '*' || c.isSpace() )) {
         j++;
      }

      if (j > 0) {
         oldStyleArgPtr = current->args.left(j);
         oldStyleArgName = current->args.mid(j).trimmed();

      } else {
         oldStyleArgName = current->args.trimmed();
      }
   }
}


/*! Update the argument \a name with additional \a type info. For K&R style
 *  function the type is found \e after the argument list, so this routine
 *  in needed to fix up.
 */
static void addKnRArgInfo(const QString &type, const QString &name, const QString &brief, const QString &docs)
{
   for (auto a : current->argList) {
      if (a.type == name) {
         a.type = type.trimmed();

         if (a.type.left(9) == "register ") { 
            // strip keyword
            a.type = a.type.mid(9);
         }

         a.name = name.trimmed();

         if (! brief.isEmpty() && ! docs.isEmpty()) {
            a.docs = brief + "\n\n" + docs;

         } else if (! brief.isEmpty()) {
            a.docs = brief;

         } else {
            a.docs = docs;
         }
      }
   }
}

void fixArgumentListForJavaScript(ArgumentList *al)
{
   if (al == 0) {
      return;
   }

   for (auto a : *al) {
      if (! a.type.isEmpty() && a.name.isEmpty()) {
         // a->type is actually the (typeless) parameter name, so move it
         a.name = a.type;
         a.type.resize(0);
      }
   }
}

#undef   YY_INPUT
#define  YY_INPUT(buf,result,max_size) result = yyread(buf,max_size);

static int yyread(char *buf, int max_size)
{
   int c = 0;

   while (inputString[inputPosition] != 0) {

      QString tmp1    = inputString.at(inputPosition);
      QByteArray tmp2 = tmp1.toUtf8();

      if (c + tmp2.length() >= max_size) {
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

/* start command character */
CMD          ("\\"|"@")
SECTIONCMD   {CMD}("image"|"author"|"internal"|"version"|"date"|"deprecated"|"param"|"exception"|"return"[s]?|"retval"|"bug"|"warning"|"par"|"sa"|"see"|"pre"|"post"|"invariant"|"note"|"remark"[s]?|"todo"|"test"|"xrefitem"|"ingroup"|"callgraph"|"callergraph"|"latexonly"|"htmlonly"|"xmlonly"|"docbookonly"|"manonly"|"{"|"verbatim"|"dotfile"|"dot"|"defgroup"|"addtogroup"|"weakgroup"|"class"|"namespace"|"union"|"struct"|"fn"|"var"|"details"|"typedef"|"def"|"overload")|("<"{PRE}">")

BN           [ \t\n\r]
BL           [ \t\r]*"\n"
B            [ \t]
BS           ^(({B}*"//")?)(({B}*"*"+)?){B}*
ID           "$"?[a-z_A-Z\x80-\xFF][a-z_A-Z0-9\x80-\xFF]*
SCOPEID      {ID}({ID}*{BN}*"::"{BN}*)*({ID}?)
SCOPENAME    "$"?(({ID}?{BN}*"::"{BN}*)*)(((~|!){BN}*)?{ID})
PHPSCOPENAME ({ID}"\\")+{ID}
TSCOPE       {ID}("<"[a-z_A-Z0-9 \t\*\&,:]*">")?
CSSCOPENAME  (({ID}?{BN}*"."{BN}*)*)((~{BN}*)?{ID})
PRE          [pP][rR][eE]
CODE         [cC][oO][dD][eE]
CHARLIT      (("'"\\[0-7]{1,3}"'")|("'"\\."'")|("'"[^'\\\n]{1,4}"'"))
PHPKW        ("require"|"require_once"|"include"|"include_once"|"echo")[^a-zA-Z0-9_;]
IDLATTR      ("["[^\]]*"]"){BN}*
TYPEDEFPREFIX (("typedef"{BN}+)?)((("volatile"|"const"){BN}+)?)
RAWBEGIN     (u|U|L|u8)?R\"[^ \t\(\)\\]{0,16}"("
RAWEND       ")"[^ \t\(\)\\]{0,16}\"
ARITHOP      "+"|"-"|"/"|"*"|"%"|"--"|"++"
ASSIGNOP     "="|"*="|"/="|"%="|"+="|"-="|"<<="|">>="|"&="|"^="|"|="
LOGICOP      "=="|"!="|">"|"<"|">="|"<="|"&&"|"||"|"!"
BITOP        "&"|"|"|"^"|"<<"|">>"|"~"
OPERATOR     "operator"{B}*({ARITHOP}|{ASSIGNOP}|{LOGICOP}|{BITOP})

CS_MACRO     ("CS_OBJECT_MULTIPLE"|"CS_GADGET"|"CS_CLASSINFO"|"CS_INTERFACES"|"CS_ENUM"|"CS_FLAG"|"CS_INVOKABLE_CONSTRUCTOR_1"|"CS_INVOKABLE_CONSTRUCTOR_2"|("CS_DECLARE"[_A-Z0-9]+)|("Q_DECLARE"[_A-Z0-9]+)|"Q_DISABLE_COPY"|"Q_ENUM"|"Q_ENUMS"|"Q_INTERFACES"|"K_DECLARE_PRIVATE"|"K_D"|"PHONON_HEIR")

%option noyywrap

/* language parsing states */

%x AlignAs
%x AlignAsEnd
%x Define
%x DefineEnd
%x CompoundName
%x ClassVar
%x CSConstraintName
%x CSConstraintType
%x CSIndexer
%x ClassCategory
%x ClassTemplSpec
%x CliPropertyType
%x CliPropertyIndex
%x CliOverride
%x Bases
%x BasesProt
%x NextSemi
%x BitFields
%x EnumBaseType
%x FindMembers
%x FindMembersPHP
%x FindMemberName
%x FindFields
%x FindFieldArg
%x Function
%x FuncRound
%x ExcpRound
%x ExcpList
%x FuncQual
%x TrailingReturn
%x Operator
%x Array
%x ReadBody
%x ReadNSBody
%x ReadBodyIntf
%x Using
%x UsingAlias
%x UsingAliasEnd
%x UsingDirective
%x SkipCurly
%x SkipCurlyCpp
%x SkipCurlyEndDoc
%x SkipString
%x SkipPHPString
%x SkipInits
%x SkipC11Inits
%x SkipCPP
%x SkipCPPBlock
%x SkipComment
%x SkipCxxComment
%x SkipCurlyBlock
%x SkipRoundBlock
%x Sharp
%x SkipRound
%x SkipSquare
%x SkipRemainder
%x StaticAssert
%x DeclType
%x TypedefName
%x TryFunctionBlock
%x TryFunctionBlockEnd
%x Comment
%x PackageName
%x JavaImport
%x PHPUse
%x PHPUseAs
%x CSAccessorDecl
%x CSGeneric
%x PreLineCtrl
%x DefinePHP
%x DefinePHPEnd
%x OldStyleArgs
%x SkipVerbString
%x ObjCMethod
%x ObjCReturnType
%x ObjCParams
%x ObjCParamType
%x ObjCProtocolList
%x ObjCPropAttr
%x ObjCSkipStatement

%x QtPropType
%x QtPropName
%x QtPropAttr
%x QtPropRead
%x QtPropWrite

%x CsObject
%x CsObjectName
%x CsObjectClose
%x CsIgnore
%s CsRegEnum
%x CsPropReadVar
%x CsPropReadMethod
%x CsPropWriteVar
%x CsPropWriteMethod
%x CsPropResetVar
%x CsPropResetMethod
%x CsPropNotifyVar
%x CsPropNotifyMethod
%x CsPropVar
%x CsPropMethod
%x CsPropJustVar
%x CsPropClose
%x CsSignal
%x CsSignalProtection
%x CsSignalSlotMethod
%x CsSignalSlotClose
%x CsSlot
%x CsSlotProtection
%x CsSignalSlot2
%x CsSignalSlot2Name
%x CsSignalSlot2Extra
%x CsSignalSlot2Close

%x ReadInitializer
%x UNOIDLAttributeBlock
%x GetCallType
%x CppQuote
%x EndCppQuote
%x MemberSpec
%x MemberSpecSkip
%x EndTemplate
%x FuncPtr
%x FuncPtrOperator
%x EndFuncPtr
%x ReadFuncArgType
%x ReadTempArgs
%x IDLUnionCase
%x NSAliasName
%x NSAliasArg
%x CopyString
%x CopyPHPString
%x CopyGString
%x CopyPHPGString
%x CopyRound
%x CopyCurly
%x GCopyRound
%x GCopyCurly
%x SkipUnionSwitch
%x Specialization
%x FuncPtrInit
%x FuncFunc
%x FuncFuncEnd
%x FuncFuncType
%x FuncFuncArray
%x CopyArgString
%x CopyArgPHPString
%x CopyArgRound
%x CopyArgSharp
%x CopyArgComment
%x CopyArgCommentLine
%x CopyArgVerbatim
%x HereDoc
%x HereDocEnd
%x CopyHereDoc
%x CopyHereDocEnd
%x RawString
%x RawGString
%x IDLAttribute
%x IDLProp
%x IDLPropName

/* prototype scanner states */

%x Prototype
%x PrototypePtr
%x PrototypeQual
%x PrototypeExc
%x PrototypeSkipLine

/* comment parsing states */

%x DocLine
%x DocBlock
%x DocCopyBlock

%%

<NextSemi>"{"   {
      curlyCount=0;
      needsSemi = true;
      BEGIN(SkipCurlyBlock);
   }

<NextSemi>"("   {
      roundCount=0;
      BEGIN(SkipRoundBlock);
   }

<SkipRoundBlock>"("   {
      ++roundCount;
   }

<SkipRoundBlock>")"   {
      if (roundCount != 0) {
         --roundCount;
      } else {
         BEGIN( NextSemi );
      } 
   }

<SkipCurlyBlock>"{"   {
                    ++curlyCount ;
   }

<SkipCurlyBlock>"}"        {
                 if( curlyCount ) {
                   --curlyCount ;
                 } else if (needsSemi) {
                   BEGIN( NextSemi );
                 }
                 else
                 {
                   BEGIN( FindMembers );
                 }
   }

<NextSemi>\'     {
                   if (insidePHP) {
                    lastStringContext=NextSemi;
                    BEGIN(SkipPHPString);
                   }
   }

<NextSemi>{CHARLIT}        { 
      if (insidePHP) {
         REJECT;
      }
   }

<NextSemi>\"            {
                    lastStringContext=NextSemi;
                 BEGIN(SkipString);
   }

<NextSemi>[;,]             {
                    unput(*yytext);
                    BEGIN( FindMembers );
               }
<BitFields>[;,]            {
                    unput(*yytext);
                 BEGIN( FindMembers );
   }

<EnumBaseType>[{;,]                     {
                 current->args = current->args.simplified();
                 unput(*yytext);
                 BEGIN( ClassVar );
   }                                        

<FindMembers>"<?php"   { 
      // PHP code with unsupported extension?
      insidePHP = true;
   }

<FindMembersPHP>"<?"("php"?)            { 
      // PHP code start
      BEGIN( FindMembers );
   }

<FindMembersPHP>"<script"{BN}+"language"{BN}*"="{BN}*['"]?"php"['"]?{BN}*">" { 
      // PHP code start
      lineCount() ;
      BEGIN( FindMembers );
   }

<FindMembersPHP>[^\n<]+                 { 
      // Non-PHP code text, ignore
   }

<FindMembersPHP>\n                      { 
      // Non-PHP code text, ignore
      lineCount();
   }

<FindMembersPHP>.                       { 
      // Non-PHP code text, ignore
   }

<FindMembers>"?>"|"</script>"           { 
      // PHP code end
                 if (insidePHP)
                   BEGIN( FindMembersPHP );
                 else
                   REJECT;
   }

<FindMembers>{PHPKW}          { 
      if (insidePHP)
                   BEGIN( NextSemi );
                 else
                   REJECT;
   }

<FindMembers>"%{"[^\n]*          { 
      // Mozilla XPIDL lang-specific block
                 if (!insideIDL)
                   REJECT;
   }

<FindMembers>"%}"          { 
      // Mozilla XPIDL lang-specific block end
      if (! insideIDL) {
         REJECT;
      }
   }

<FindMembers>{B}*("properties"){BN}*":"{BN}*  { 
      // IDL or Borland C++ builder property
                 current->mtype = mtype = MethodTypes::Property;
                 current->protection = protection = Public ;
                 current->type.resize(0);
                 current->name.resize(0);
                 current->args.resize(0);
                 current->argList.clear();
                 lineCount() ;
   }

<FindMembers>{B}*"k_dcop"{BN}*":"{BN}*  {
                 current->mtype = mtype = MethodTypes::DCOP;
                 current->protection = protection = Public ;
                 current->type.resize(0);
                 current->name.resize(0);
                 current->args.resize(0);
                 current->argList.clear();
                 lineCount() ;
   }

<FindMembers>{B}*("signals"|"Q_SIGNALS"){BN}*":"{BN}* {
                 current->mtype = mtype = MethodTypes::Signal;
                 current->protection = protection = Public ;
                 current->type.resize(0);
                 current->name.resize(0);
                 current->args.resize(0);
                 current->argList.clear();
                 lineCount() ;
   }

<FindMembers>{B}*"public"{BN}*("slots"|"Q_SLOTS"){BN}*":"{BN}* {
                 current->mtype = mtype = MethodTypes::Slot;
                 current->protection = protection = Public ;
                 current->type.resize(0);
                 current->name.resize(0);
                 current->args.resize(0);
                 current->argList.clear();
                 lineCount();
   }

<FindMembers>{B}*"protected"{BN}*("slots"|"Q_SLOTS"){BN}*":"{BN}* {
                 current->protection = protection = Protected ;
                 current->mtype = mtype = MethodTypes::Slot;
                 current->type.resize(0);
                 current->name.resize(0);
                 current->args.resize(0);
                 current->argList.clear();
                 lineCount();
   }

<FindMembers>{B}*"private"{BN}*("slots"|"Q_SLOTS"){BN}*":"{BN}* {
                 current->protection = protection = Private ;
                 current->mtype = mtype = MethodTypes::Slot;
                 current->type.resize(0);
                 current->name.resize(0);
                 current->args.resize(0);
                 current->argList.clear();
                 lineCount();
   }

<FindMembers>{B}*("public"|"methods"|"__published"){BN}*":"{BN}* {
                 current->protection = protection = Public ;
                 current->mtype = mtype = MethodTypes::Method;
                 current->type.resize(0);
                 current->name.resize(0);
                 current->args.resize(0);
                 current->argList.clear();
                 lineCount() ;
   }

<FindMembers>{B}*"internal"{BN}*":"{BN}* {  
      // for now treat C++/CLI's internal as package...
      if (insideCli) {
                   current->protection = protection = Package ;
                   current->mtype = mtype = MethodTypes::Method;
                   current->type.resize(0);
                   current->name.resize(0);
                   current->args.resize(0);
                   current->argList.clear();
                   lineCount() ;
                 }
                 else
                 {
                   REJECT;
                 }
   }

<FindMembers>{B}*"protected"{BN}*":"{BN}* {
                    current->protection = protection = Protected ;
                 current->mtype = mtype = MethodTypes::Method;
                 current->type.resize(0);
                 current->name.resize(0);
                 current->args.resize(0);
                 current->argList.clear();
                 lineCount() ;
   }

<FindMembers>{B}*"private"{BN}*":"{BN}*   {
                    current->protection = protection = Private ;
                 current->mtype = mtype = MethodTypes::Method;
                 current->type.resize(0);
                 current->name.resize(0);
                 current->args.resize(0);
                 current->argList.clear();
                 lineCount() ;
   }

<FindMembers>{B}*"event"{BN}+           {
                    if (insideCli)
                 {
                   // C++/CLI event
                   lineCount() ;
                      current->mtype = mtype = MethodTypes::Event;
                   current->bodyLine = yyLineNr;
                   curlyCount=0;
                   BEGIN( CliPropertyType );
                 }
                 else if (insideCSharp)
                 {
                   lineCount() ;
                      current->mtype = MethodTypes::Event;
                   current->bodyLine = yyLineNr;
                 }
                 else
                 {
                   REJECT;
                 }
   }

<FindMembers>{B}*"property"{BN}+    {
                     if (insideCli)
                  {
                    // C++/CLI property
                    lineCount() ;
                       current->mtype = mtype = MethodTypes::Property;
                    current->bodyLine = yyLineNr;
                    curlyCount=0;
                    BEGIN( CliPropertyType );
                  }
                  else
                  {
                    REJECT;
                  }
   }

<CliPropertyType>{ID}         {
                    QString text = QString::fromUtf8(yytext);
                    addType( current );
                    current->name = text;
   }

<CliPropertyType>"["               { 
      // C++/CLI indexed property
                 QString text = QString::fromUtf8(yytext);
                 current->name += text;
                 BEGIN( CliPropertyIndex );
   }

<CliPropertyType>"{"          {
      curlyCount=0;
      BEGIN( CSAccessorDecl );
   }

<CliPropertyType>";"          {
                    unput(*yytext);
                 BEGIN( FindMembers );
   }

<CliPropertyType>\n        {
                                          lineCount();
   }

<CliPropertyType>{B}*         {
   }

<CliPropertyType>.         {
                    QString text = QString::fromUtf8(yytext);
                    addType( current );
                    current->type += text;
   }

<CliPropertyIndex>"]"         {
                    QString text = QString::fromUtf8(yytext);
                    BEGIN( CliPropertyType );
                    current->name+=text;
                  }
<CliPropertyIndex>.        {
                    QString text = QString::fromUtf8(yytext);
                    current->name+=text;
   }

  /*
<FindMembers>{B}*"property"{BN}+        {
      if (!current->type.isEmpty()) {
         REJECT;
     } else {
         current->mtype = mtype = MethodTypes::Property;
         lineCount();
     }
   }
  */

<FindMembers>{B}*"@private"{BN}+    {
                    current->protection = protection = Private ;
                 current->mtype = mtype = MethodTypes::Method;
                 current->type.resize(0);
                 current->name.resize(0);
                 current->args.resize(0);
                 current->argList.clear();
                 lineCount() ;
   }

<FindMembers>{B}*"@protected"{BN}+  {
                    current->protection = protection = Protected ;
                 current->mtype = mtype = MethodTypes::Method;
                 current->type.resize(0);
                 current->name.resize(0);
                 current->args.resize(0);
                 current->argList.clear();
                 lineCount() ;
   }

<FindMembers>{B}*"@public"{BN}+  {
                 current->protection = protection = Public ;
                 current->mtype = mtype = MethodTypes::Method;
                 current->type.resize(0);
                 current->name.resize(0);
                 current->args.resize(0);
                 current->argList.clear();
                 lineCount() ;
   }

<FindMembers>[\-+]{BN}*          {
                 QString text = QString::fromUtf8(yytext);

                 if (! insideObjC) {
                   REJECT; 

                 } else {
                   lineCount();
                   current->fileName    = yyFileName;
                   current->startLine   = yyLineNr;
                   current->startColumn = yyColNr;
                   current->bodyLine    = yyLineNr;
                   current->section     = Entry::FUNCTION_SEC;
                   current->protection  = protection = Public;

                   language   = current->lang = SrcLangExt_ObjC;
                   insideObjC = true;

                   current->virt  = Virtual;
                   current->stat  = (text[0]=='+');
                   current->mtype = mtype = MethodTypes::Method;
                   current->type.resize(0);
                   current->name.resize(0);
                   current->args.resize(0);
                   current->argList.clear();
                   BEGIN( ObjCMethod );
                 }
   }

<ObjCMethod>"("            {
      // start of method's return type
      BEGIN( ObjCReturnType );
   }

<ObjCMethod>{ID}        { 
      // found method name
      QString text = QString::fromUtf8(yytext);  

                 if (current->type.isEmpty())
                 {
                   current->type = "id";
                 }
                 current->name = text;
                                          if (insideCpp || insideObjC)
                                          {
                                            current->id = ClangParser::instance()->lookup(yyLineNr, text);
                                          }
   }

<ObjCMethod>":"{B}*        { 
      // start of parameter list
                    current->name += ':';

                 Argument temp;
                 current->argList.append(temp);

                    BEGIN( ObjCParams );
   }

<ObjCReturnType>[^)]*         { 
      // TODO: check if nested braches are possible.
      QString text = QString::fromUtf8(yytext); 
      current->type = text;
   }

<ObjCReturnType>")"        {
      BEGIN( ObjCMethod );
   }

<ObjCParams>({ID})?":"        { 
      // Keyword of parameter
                 QString text = QString::fromUtf8(yytext); 

                 text = text.left(text.length()-1); // strip :

                 if (text.isEmpty())
                 {
                   current->name += " :";
                 }
                 else
                 {
                   current->name += text + ":";
                 }
                 if (current->argList.last().type.isEmpty())
                 {
                   current->argList.last().type="id";
                 }

                 Argument a;
                 a.attrib=(QString)"["+text+"]";
                 current->argList.append(a);
   }

<ObjCParams>{ID}{BN}*         { 
      // name of parameter
                    QString text = QString::fromUtf8(yytext); 
                    lineCount();
                    current->argList.last().name = QString(text).trimmed();
   }

<ObjCParams>","{BN}*"..."     { 
      // name of parameter
                 lineCount();
                 // do we want the comma as part of the name?
                 //current->name += ",";
                 Argument a;
                 a.attrib="[,]";
                 a.type="...";
                 current->argList.append(a);
   }

   /*
<ObjCParams>":"            {
      current->name += ':';
   }
   */

<ObjCParams>"("            {
                 roundCount=0;
                 current->argList.last().type.resize(0);
                 BEGIN( ObjCParamType );
   }

<ObjCParamType>"("                      {
                    QString text = QString::fromUtf8(yytext); 
                    roundCount++;
                    current->argList.last().type+=text;
   }

<ObjCParamType>")"/{B}*          {
                    QString text = QString::fromUtf8(yytext); 
                    if (roundCount<=0)
                    {
                       BEGIN( ObjCParams );
                    }
                       else
                    {
                       current->argList.last().type += text;
                       roundCount--;
                    }
   }

<ObjCParamType>[^()]*         {
                       QString text = QString::fromUtf8(yytext); 
                       current->argList.last().type += QString(text).trimmed();
   }

<ObjCMethod,ObjCParams>";"       { 
      // end of method declaration
                 if (! current->argList.isEmpty() && current->argList.last().type.isEmpty())
                 {
                   current->argList.last().type="id";
                 }
                    current->args = argListToString(&current->argList);
                 //printf("argList=%s\n",current->args.data());
                    unput(';');
                    BEGIN( Function );
   }

<ObjCMethod,ObjCParams>(";"{BN}+)?"{"  { 
      // start of a method body
      lineCount();
             
                 if (! current->argList.isEmpty() && current->argList.last().type.isEmpty())
                 {
                   current->argList.last().type = "id";
                 }
                    current->args = argListToString(&current->argList);
                 unput('{');
                    BEGIN( Function );
   }

<FindMembers>{BN}{1,80}               {
                    lineCount();
   }

<FindMembers>"@"({ID}".")*{ID}{BN}*"("    {
                 QString text = QString::fromUtf8(yytext); 
                 if (insideJava) // Java annotation
                 {
                   lineCount();
                   lastSkipRoundContext = YY_START;
                   roundCount=0;
                   BEGIN( SkipRound );
                 }
                 else if (text.startsWith("@property")) // ObjC 2.0 property
                 {
                   current->mtype = mtype = MethodTypes::Property;
                   current->m_specFlags.m_isReadable = true;
                   current->m_specFlags.m_isWritable = true;
                   current->m_specFlags.m_isAssign = true;

                   current->protection = Public ;
                   unput('(');
                   BEGIN( ObjCPropAttr );
                 }
                 else
                 {
                   REJECT;
                 }
   }

<ObjCPropAttr>"getter="{ID}      {
                     QString text = QString::fromUtf8(yytext); 
                     current->m_read = text.mid(7);
   }

<ObjCPropAttr>"setter="{ID}      {
                     QString text = QString::fromUtf8(yytext);  
                     current->m_write = text.mid(7);
   }

<ObjCPropAttr>"readonly"      {
                 current->m_specFlags.m_isWritable = false;
   }

<ObjCPropAttr>"readwrite"     { 
      // default
   }

<ObjCPropAttr>"assign"        { 
      // default
   }

<ObjCPropAttr>"unsafe_unretained"       {
                 current->m_specFlags.m_isAssign  = false;
                 current->m_specFlags.spec |=Entry::Unretained;
   }

<ObjCPropAttr>"retain"        {
                 current->m_specFlags.m_isAssign  = false;
                 current->m_specFlags.spec |=Entry::Retain;
   }

<ObjCPropAttr>"copy"          {
                 current->m_specFlags.m_isAssign  = false;
                 current->m_specFlags.spec |=Entry::Copy;
   }

<ObjCPropAttr>"weak"                    {
                 current->m_specFlags.m_isAssign  = false;
                 current->m_specFlags.spec |=Entry::Weak;
                                        }
<ObjCPropAttr>"strong"                  {
                 current->m_specFlags.m_isAssign  = false;
                 current->m_specFlags.spec |=Entry::Strong;
   }

<ObjCPropAttr>"nonatomic"     {
                 current->m_specFlags.spec |= Entry::NonAtomic;
   }

<ObjCPropAttr>")"          {
      BEGIN(FindMembers);
   }

<FindMembers>"@"{ID}          {
                 QString text = QString::fromUtf8(yytext); 
                 if (insideJava) // Java annotation
                 {
                   // skip annotation
                 }  else if (text == "@property") // ObjC 2.0 property 
                 {
                   current->mtype = mtype = MethodTypes::Property;
                   current->m_specFlags.m_isReadable = true;
                   current->m_specFlags.m_isWritable = true;

                   current->protection = Public ;
                 }
                 else if ( text == "@synthesize") {
                   BEGIN( ObjCSkipStatement );
                 } else if (text == "@dynamic") {
                   BEGIN( ObjCSkipStatement ); 
                 } else {
                   REJECT;
                 }
   }

<ObjCSkipStatement>";"        {
                    BEGIN(FindMembers);
   }

<PackageName>{ID}(("."|"\\"){ID})*  {
                 QString text = QString::fromUtf8(yytext); 
                 isTypedef = false;
                 current->name = text;
                 current->name = substitute(current->name,".","::");
                 current->name = substitute(current->name,"\\","::");
                 current->section = Entry::NAMESPACE_SEC;
                 current->type = "namespace" ;
                 current->fileName  = yyFileName;
                 current->startLine = yyLineNr;
                 current->startColumn = yyColNr;
                 current->bodyLine  = yyLineNr;
                 lineCount();
   }

<PackageName>";"        {
                 current_root->addSubEntry(current, current_root);
                 current_root = current ;

                 current = QMakeShared<Entry>();
                 initEntry();
                    BEGIN(FindMembers);
   }

<PackageName>"{"        {
                 curlyCount=0;
                 BEGIN( ReadNSBody );
   }

<FindMembers>{B}*"initonly"{BN}+    {
                    current->type += " initonly ";
                 if (insideCli) current->m_specFlags.spec |= Entry::Initonly;
                 lineCount();
   }

<FindMembers>{B}*"static"{BN}+         { current->type += " static ";
                    current->stat = true;
                 lineCount();
   }

<FindMembers>{B}*"extern"{BN}+      {
                    current->stat = false;
                 current->explicitExternal = true;
                 lineCount();
   }

<FindMembers>{B}*"virtual"{BN}+        { current->type += " virtual ";
                 current->virt = Virtual;
                 lineCount();
   }

<FindMembers>{B}*"published"{BN}+   { 
      // UNO IDL published keyword
                 if (insideIDL)
                 {
                   lineCount();
                   current->m_specFlags.spec |= Entry::Published;
                 }
                 else
                 {
                   REJECT;
                 }
   }

<FindMembers>{B}*"abstract"{BN}+       {
      if (!insidePHP) {
                   current->type += " abstract ";
                   if (!insideJava)
                   {
                        current->virt = Pure;
                   }
                   else
                   {
                        current->m_specFlags.spec |= Entry::Abstract;
                   }
                 }
                 else
                 {
                   current->m_specFlags.spec |= Entry::Abstract;
                 }
                 lineCount();
   }

<FindMembers>{B}*"inline"{BN}+      { 
      current->m_specFlags.spec |=Entry::Inline;
      lineCount();
   }

<FindMembers>{B}*"mutable"{BN}+     { 
      current->m_specFlags.spec |=Entry::Mutable;
      lineCount();
   }

<FindMembers>{B}*"explicit"{BN}+    { 
      current->m_specFlags.spec |=Entry::Explicit;
      lineCount();
   }

<FindMembers>{B}*"@required"{BN}+   { 
      // Objective C 2.0 protocol required section
      current->m_specFlags.spec = (current->m_specFlags.spec & ~Entry::Optional) | Entry::Required;
      lineCount();
   }

<FindMembers>{B}*"@optional"{BN}+   {  
      // Objective C 2.0 protocol optional section
      current->m_specFlags.spec = (current->m_specFlags.spec & ~Entry::Required) | Entry::Optional;
      lineCount();
   }

  /*
<FindMembers>{B}*"import"{BN}+      { 
      // IDL import keyword
      BEGIN( NextSemi );
   }

  */

<FindMembers>{B}*"typename"{BN}+    { 
      lineCount(); 
   }

<FindMembers>{B}*"namespace"{BN}*/[^a-z_A-Z0-9]    {
                 isTypedef=false;
                 current->section = Entry::NAMESPACE_SEC;
                 current->type = "namespace" ;
                 current->fileName  = yyFileName;
                 current->startLine = yyLineNr;
                 current->startColumn = yyColNr;
                 current->bodyLine  = yyLineNr;
                 lineCount();
                 if (insidePHP)
                 {
                   BEGIN( PackageName );
                 }
                 else
                 {
                      BEGIN( CompoundName );
                 }
   }

<FindMembers>{B}*"module"{BN}+      {
                 lineCount();
                                          if (insideIDL)
                 {
                   isTypedef=false;
                   current->section = Entry::NAMESPACE_SEC;
                   current->type = "module" ;
                   current->fileName  = yyFileName;
                   current->startLine = yyLineNr;
                   current->startColumn = yyColNr;
                   current->bodyLine  = yyLineNr;
                   BEGIN( CompoundName );
                 }
                 else if (insideD)
                 {
                   lineCount();
                   BEGIN(PackageName);
                 }
                 else
                 {
                   QString text = QString::fromUtf8(yytext); 
                   addType( current ) ;
                   current->name = text.trimmed();
                 }
   }

<FindMembers>{B}*"library"{BN}+     {
                 lineCount();
                 if (insideIDL)
                 {
                      isTypedef=false;
                   current->section = Entry::NAMESPACE_SEC;
                   current->type = "library" ;
                   current->fileName  = yyFileName;
                   current->startLine = yyLineNr;
                   current->startColumn = yyColNr;
                   current->bodyLine  = yyLineNr;
                      BEGIN( CompoundName );
                 }
                 else
                 {

                   QString text = QString::fromUtf8(yytext);
                   addType( current ) ;
                   current->name = text.trimmed();
                 }
   }

<FindMembers>{B}*"constants"{BN}+   {  
      // UNO IDL constant group
      lineCount();
      if (insideIDL) {
                   isTypedef=false;
                   current->section = Entry::NAMESPACE_SEC;
                   current->type = "constants";
                   current->fileName  = yyFileName;
                   current->startLine = yyLineNr;
                   current->startColumn = yyColNr;
                   current->bodyLine  = yyLineNr;
                   BEGIN( CompoundName );
                 }
                 else
                 {
                   QString text = QString::fromUtf8(yytext);
                   addType( current ) ;
                   current->name = text.trimmed();
                 }
   }

<FindMembers>{BN}*("service"){BN}+     { 
      // UNO IDL service
                 lineCount();
                 if (insideIDL)
                 {
                   isTypedef=false;
                   current->section = Entry::CLASS_SEC;
                   current->m_specFlags.spec = Entry::Service |
                     // preserve UNO IDL [optional] or published
                     (current->m_specFlags.spec & (Entry::Optional|Entry::Published));
                   addType( current ) ;
                   current->type += " service " ;
                   current->fileName  = yyFileName;
                   current->startLine = yyLineNr;
                   current->bodyLine  = yyLineNr;
                   BEGIN( CompoundName );
                 }
                 else // TODO is addType right? just copy/pasted
                 {
                   QString text = QString::fromUtf8(yytext);
                   addType( current ) ;
                   current->name = text.trimmed();
                 }
   }

<FindMembers>{BN}*("singleton"){BN}+   { 
      // UNO IDL singleton
                 lineCount();
                 if (insideIDL)
                 {
                   isTypedef=false;
                   current->section = Entry::CLASS_SEC;
                   current->m_specFlags.spec = Entry::Singleton | (current->m_specFlags.spec & Entry::Published); // preserve
                   addType( current ) ;
                   current->type += " singleton " ;
                   current->fileName  = yyFileName;
                   current->startLine = yyLineNr;
                   current->bodyLine  = yyLineNr;
                   BEGIN( CompoundName );
                 }
                 else // TODO is addType right? just copy/pasted
                 {
                   QString text = QString::fromUtf8(yytext);
                   addType( current ) ;
                   current->name = text.trimmed();
                 }
   }

<FindMembers>{BN}*((("disp")?"interface")|"valuetype"){BN}+    { 
      // M$/Corba/UNO IDL/Java interface

      QString text = QString::fromUtf8(yytext);
      lineCount();

      if (insideIDL || insideJava || insideCSharp || insideD || insidePHP) {
                   isTypedef=false;
                   current->section = Entry::CLASS_SEC;

                   current->m_specFlags.spec = Entry::Interface |
                          // preserve UNO IDL [optional] or published
                          (current->m_specFlags.spec & (Entry::Optional|Entry::Published));

                   addType( current ) ;
                   current->type += " interface" ;
                   current->fileName    = yyFileName;
                   current->startLine   = yyLineNr;
                   current->startColumn = yyColNr;
                   current->bodyLine    = yyLineNr;
                   BEGIN( CompoundName );
                 }
                 else
                 {
                   addType( current ) ;
                   current->name = text.trimmed();
                 }
   }

<FindMembers>{B}*"@implementation"{BN}+   { 
      // Objective-C class implementation
                 lineCount();
                    isTypedef=false;
                 current->section = Entry::OBJCIMPL_SEC;
                 language = current->lang = SrcLangExt_ObjC;
                 insideObjC = true;
                 current->protection = protection = Public ;
                 addType( current ) ;
                 current->type += " implementation" ;
                 current->fileName  = yyFileName;
                 current->startLine = yyLineNr;
                 current->bodyLine  = yyLineNr;
                 BEGIN( CompoundName );
   }

<FindMembers>{B}*"@interface"{BN}+  { 
      // Objective-C class interface, or Java attribute
                 lineCount();
                    isTypedef=false;
                 current->section = Entry::CLASS_SEC;
                                          current->m_specFlags.spec = Entry::Interface;
                 if (!insideJava)
                 {
                   language = current->lang = SrcLangExt_ObjC;
                   insideObjC = true;
                 }
                 current->protection = protection = Public ;
                 addType( current ) ;
                 current->type += " interface" ;
                 current->fileName  = yyFileName;
                 current->startLine = yyLineNr;
                 current->startColumn = yyColNr;
                 current->bodyLine  = yyLineNr;
                 BEGIN( CompoundName );
   }

<FindMembers>{B}*"@protocol"{BN}+   { 
   // Objective-C protocol definition
                 lineCount();
                    isTypedef=false;
                 current->section = Entry::CLASS_SEC;
                 current->m_specFlags.spec = Entry::Protocol;
                 language = current->lang = SrcLangExt_ObjC;
                 insideObjC = true;
                 current->protection = protection = Public ;
                 addType( current ) ;
                 current->type += " protocol" ;
                 current->fileName  = yyFileName;
                 current->startLine = yyLineNr;
                 current->startColumn = yyColNr;
                 current->bodyLine  = yyLineNr;
                 BEGIN( CompoundName );
   }

<FindMembers>{B}*"exception"{BN}+   { 
      // Corba IDL exception
                 isTypedef=false;
                 current->section = Entry::CLASS_SEC;
                 current->m_specFlags.spec = Entry::Exception | (current->m_specFlags.spec & Entry::Published); // preserve UNO IDL
                 addType( current ) ;
                 current->type += " exception" ;
                 current->fileName  = yyFileName;
                 current->startLine = yyLineNr;
                 current->startColumn = yyColNr;
                 current->bodyLine  = yyLineNr;
                 lineCount();
                 BEGIN( CompoundName );
   }

<FindMembers>"@class" | // for Objective C class declarations
<FindMembers>{B}*{TYPEDEFPREFIX}"class{" |
<FindMembers>{B}*{TYPEDEFPREFIX}"class"{BN}+   {
      QString text = QString::fromUtf8(yytext);

                 isTypedef = text.indexOf("typedef")!=-1;

                 bool isConst     = text.indexOf("const")!=-1;
                 bool isVolatile  = text.indexOf("volatile")!=-1;
                 current->section = Entry::CLASS_SEC;

                 addType(current);
                 if (isConst) {
                    current->type += " const";
                 } else if (isVolatile) {
                    current->type += " volatile";
                 }

                 current->type += " class" ;
                 current->fileName    = yyFileName;
                 current->startLine   = yyLineNr;
                 current->startColumn = yyColNr;
                 current->bodyLine    = yyLineNr;

                 if (text[0] == '@') {
                   language = current->lang = SrcLangExt_ObjC;
                   insideObjC = true;
                 }

                 lineCount();
                 if (text[yyleng-1] == '{') {
                    unput('{');
                 } 

                 if (insidePHP && current->m_specFlags.spec & Entry::Abstract)
                 {
                   // convert Abstract to AbstractClass

                   current->m_specFlags.m_isAbstractClass = true;
                   current->m_specFlags.spec = current->m_specFlags.spec & ~Entry::Abstract;
                 }
                 BEGIN( CompoundName ) ;
   }

<FindMembers>{B}*"value class{" |         // C++/CLI extension
<FindMembers>{B}*"value class"{BN}+     {
                 QString text = QString::fromUtf8(yytext);
                 isTypedef=false;
                 current->section = Entry::CLASS_SEC;
                 current->m_specFlags.spec = Entry::Value;
                 addType( current ) ;
                 current->type += " value class" ;
                 current->fileName  = yyFileName;
                 current->startLine = yyLineNr;
                 current->startColumn = yyColNr;
                 current->bodyLine  = yyLineNr;
                 lineCount() ;
                 if (text[yyleng-1]=='{') {
                    unput('{');
                 } 
                 BEGIN( CompoundName ) ;
   }

<FindMembers>{B}*"ref class{" |          // C++/CLI extension
<FindMembers>{B}*"ref class"{BN}+       {
                 QString text = QString::fromUtf8(yytext); 
                 isTypedef=false;
                 current->section = Entry::CLASS_SEC;
                 current->m_specFlags.spec = Entry::Ref;
                 addType( current ) ;
                 current->type += " ref class" ;
                 current->fileName  = yyFileName;
                 current->startLine = yyLineNr;
                 current->startColumn = yyColNr;
                 current->bodyLine  = yyLineNr;
                 lineCount() ;
                 if (text[yyleng-1]=='{') {
                    unput('{');
                 } 
                 BEGIN( CompoundName ) ;
    }

<FindMembers>{B}*"interface class{" |     // C++/CLI extension
<FindMembers>{B}*"interface class"{BN}+ {
                 QString text = QString::fromUtf8(yytext);  
                 isTypedef = false;
                 current->section = Entry::CLASS_SEC;
                 current->m_specFlags.spec = Entry::Interface;
                 addType( current ) ;
                 current->type += " interface class" ;
                 current->fileName  = yyFileName;
                 current->startLine = yyLineNr;
                 current->startColumn = yyColNr;
                 current->bodyLine  = yyLineNr;
                 lineCount() ;
                 if (text[yyleng-1]=='{') {
                    unput('{');
                 } 
                 BEGIN( CompoundName ) ;
   }

<FindMembers>{B}*"coclass"{BN}+     {
                 QString text = QString::fromUtf8(yytext);
                 if (insideIDL) {
                   isTypedef=false;
                   current->section = Entry::CLASS_SEC;
                   addType( current ) ;
                   current->type += " coclass" ;
                   current->fileName    = yyFileName;
                   current->startLine   = yyLineNr;
                   current->startColumn = yyColNr;
                   current->bodyLine    = yyLineNr;
                   lineCount() ;
                   BEGIN( CompoundName ) ;
                 } else  { 
                   addType(current);
                   current->name = text;
                   current->name = current->name.trimmed();
                   lineCount();
                 }
   }

<FindMembers>{B}*{TYPEDEFPREFIX}"struct{" |
<FindMembers>{B}*{TYPEDEFPREFIX}"struct"/{BN}+ {
      QString text = QString::fromUtf8(yytext);
      
      isTypedef = text.indexOf("typedef") != -1;
      
      bool isConst    = text.indexOf("const") != -1;
      bool isVolatile = text.indexOf("volatile") != -1;
      
      current->section = Entry::CLASS_SEC;
      current->m_specFlags.spec = Entry::Struct | (current->m_specFlags.spec & Entry::Published);
      
      // preserve UNO IDL
      // bug 582676: can be a struct nested in an interface so keep insideObjC state
      //current->objc    = insideObjC = false;

      addType( current ) ;
                                          if (isConst)
                                          {
                                            current->type += " const";
                                          }
                                          else if (isVolatile)
                                          {
                                            current->type += " volatile";
                                          }
                 current->type += " struct" ;
                 current->fileName  = yyFileName;
                 current->startLine = yyLineNr;
                 current->startColumn = yyColNr;
                 current->bodyLine  = yyLineNr;
                 lineCount() ;
                 if (text[yyleng-1]=='{') {
                    unput('{');
                 }
                 BEGIN( CompoundName ) ;
    }

<FindMembers>{B}*"value struct{" |      // C++/CLI extension
<FindMembers>{B}*"value struct"{BN}+     {
                 QString text = QString::fromUtf8(yytext);
                 isTypedef=false;
                 current->section = Entry::CLASS_SEC;
                 current->m_specFlags.spec  = Entry::Struct | Entry::Value;
                 addType( current ) ;
                 current->type += " value struct" ;
                 current->fileName  = yyFileName;
                 current->startLine = yyLineNr;
                 current->startColumn = yyColNr;
                 current->bodyLine  = yyLineNr;
                 lineCount() ;
                 if (text[yyleng-1]=='{') unput('{');
                 BEGIN( CompoundName ) ;
   }

<FindMembers>{B}*"ref struct{" |       // C++/CLI extension
<FindMembers>{B}*"ref struct"{BN}+     {
                 QString text = QString::fromUtf8(yytext);
                 isTypedef = false;
                 current->section = Entry::CLASS_SEC;
                 current->m_specFlags.spec = Entry::Struct | Entry::Ref;
                 addType( current ) ;
                 current->type += " ref struct" ;
                 current->fileName  = yyFileName;
                 current->startLine = yyLineNr;
                 current->startColumn = yyColNr;
                 current->bodyLine  = yyLineNr;
                 lineCount();

                 if (text[yyleng-1]=='{') { 
                    unput('{');
                 } 
                 BEGIN( CompoundName ) ;
   }

<FindMembers>{B}*"interface struct{" |     // C++/CLI extension
<FindMembers>{B}*"interface struct"{BN}+ {
                 QString text = QString::fromUtf8(yytext);
                 isTypedef=false;
                 current->section = Entry::CLASS_SEC;
                 current->m_specFlags.spec = Entry::Struct | Entry::Interface;
                 addType( current ) ;
                 current->type += " interface struct";
                 current->fileName  = yyFileName;
                 current->startLine = yyLineNr;
                 current->startColumn = yyColNr;
                 current->bodyLine  = yyLineNr;
                 lineCount() ;
                 if (text[yyleng-1]=='{') {
                   unput('{');
                 } 
                 BEGIN( CompoundName ) ;
   }

<FindMembers>{B}*{TYPEDEFPREFIX}"union{" |
<FindMembers>{B}*{TYPEDEFPREFIX}"union"{BN}+ {
      QString text = QString::fromUtf8(yytext);        
      isTypedef = text.indexOf("typedef")!=-1;
      bool isConst     = text.indexOf("const")!=-1;
      bool isVolatile  = text.indexOf("volatile")!=-1;
      current->section = Entry::CLASS_SEC;
      current->m_specFlags.spec = Entry::Union;

                 // bug 582676: can be a struct nested in an interface so keep insideObjC state
                 // current->objc    = insideObjC = false;
                 addType( current ) ;
                 if (isConst) {
                    current->type += " const";
                 } else if (isVolatile) {
                    current->type += " volatile";
                 }
                 current->type += " union" ;
                 current->fileName  = yyFileName;
                 current->startLine = yyLineNr;
                 current->startColumn = yyColNr;
                 current->bodyLine  = yyLineNr;
                 lineCount() ;
                 if (text[yyleng-1]=='{') {
                    unput('{');
                 } 
                 BEGIN( CompoundName ) ;
   }

<FindMembers>{B}*{TYPEDEFPREFIX}{IDLATTR}?"enum"({BN}+("class"|"struct"))?"{" |
<FindMembers>{B}*{TYPEDEFPREFIX}{IDLATTR}?"enum"({BN}+("class"|"struct"))?{BN}+ { 
      // for IDL: typedef [something] enum
      QString text = QString::fromUtf8(yytext);
      isTypedef = text.indexOf("typedef")!=-1;

      bool isStrongEnum = text.indexOf("struct") != -1 || text.indexOf("class") != -1 || insideCSharp;

      if (insideJava) {
         current->section = Entry::CLASS_SEC;
         current->m_specFlags.spec = Entry::Enum;
      } else {
         current->section = Entry::ENUM_SEC ;
      }

                 addType( current ) ;
                 current->type += " enum";
                 if (isStrongEnum) {
                   current->m_specFlags.spec |= Entry::Strong;
                 }
                 current->fileName  = yyFileName;
                 current->startLine = yyLineNr;
                 current->startColumn = yyColNr;
                 current->bodyLine  = yyLineNr;
                 lineCount() ;
                 if (text[yyleng-1]=='{') {
                    unput('{');
                 } 
                 BEGIN( CompoundName ) ;
   }

<Operator>"("{BN}*")"({BN}*"<"[^>]*">"){BN}*/"("  {  
      // A::operator()<int>(int arg)
                 lineCount();
                 current->name += "()";
                 BEGIN( FindMembers );
   }

<Operator>"("{BN}*")"{BN}*/"("      {
                 QString text = QString::fromUtf8(yytext);
                 lineCount();
                 current->name += text ;
                 current->name = current->name.simplified();
                 BEGIN( FindMembers ) ;
   }

<Operator>";"           { 
      // can occur when importing members
      unput(';');
      BEGIN( FindMembers ) ;
   }

<Operator>[^(]             {
      QString text = QString::fromUtf8(yytext);
      lineCount();
      current->name += text[0];
   }

<Operator>"<>"             { 
      /* skip guided templ specifiers */ 
   }

<Operator>"("           {
                 current->name = current->name.simplified();
                 unput(*yytext);
                 BEGIN( FindMembers ) ;
   }

<FindMembers>("template"|"generic")({BN}*)"<"/[>]?    {  
      // generic is a C++/CLI extension
               lineCount();
               if (current->tArgLists==0) {
                   current->tArgLists = new QList<ArgumentList>;
               }

               ArgumentList temp;
               current->tArgLists->append(temp);

               ArgumentList *al = &current->tArgLists->last();

               currentArgumentList = al;
               templateStr = "<";

               fullArgString = templateStr;
               copyArgString = &templateStr;
               currentArgumentContext = FindMembers;

               BEGIN( ReadTempArgs );
   }

<FindMembers>"namespace"{BN}+/{ID}{BN}*"=" { 
      // namespace alias
                 lineCount();
                 BEGIN( NSAliasName );
   }

<NSAliasName>{ID}          {      
                 aliasName = QString::fromUtf8(yytext);
                 BEGIN( NSAliasArg );
   }

<NSAliasArg>({ID}"::")*{ID}      {
                     // TODO: namespace aliases are now treated as global entities
                     // while they should be aware of the scope they are in
                     QString text = QString::fromUtf8(yytext);
                     Doxy_Globals::namespaceAliasDict.insert(aliasName, text);                
   }

<NSAliasArg>";"            {
                    BEGIN( FindMembers );
   }

<PHPUse>({ID}{BN}*"\\"{BN}*)*{ID}/{BN}+"as"  {
                 QString text = QString::fromUtf8(yytext);
                 lineCount();
                 aliasName = text;
                 BEGIN(PHPUseAs);
   }

<PHPUse>({ID}{BN}*"\\"{BN}*)*{ID}   {

                 QString text = QString::fromUtf8(yytext);
                 lineCount();
                 current->name = removeRedundantWhiteSpace(substitute(text,"\\","::"));                   
                 current->fileName = yyFileName;

                 // add a using declaraton
                 current->section=Entry::USINGDECL_SEC;
                 current_root->addSubEntry(current, current_root);
                 current = QMakeShared<Entry>(*current);

                 // also add it as a using directive
                 current->section=Entry::USINGDIR_SEC;
                 current_root->addSubEntry(current, current_root);
                 current = QMakeShared<Entry>();
                 initEntry();
                 aliasName.resize(0);
   }

<PHPUseAs>{BN}+"as"{BN}+      {
                    lineCount();
   }

<PHPUseAs>{ID}             {
                 
                 QString text = QString::fromUtf8(yytext);   
   
                 Doxy_Globals::namespaceAliasDict.insert(text, 
                          removeRedundantWhiteSpace(substitute(aliasName,"\\","::")));
                 aliasName.resize(0);
   }

<PHPUse,PHPUseAs>[,;]         {
                 QString text = QString::fromUtf8(yytext);
                 if (text[0] ==',') {
                    BEGIN(PHPUse);
                 } else {
                    BEGIN(FindMembers);
                 }
   }

<JavaImport>({ID}{BN}*"."{BN}*)+"*"    { 
      // package import => add as a using directive
                 QString text = QString::fromUtf8(yytext);

                 lineCount();                 
                 current->name = removeRedundantWhiteSpace(substitute(text.left(text.length()-1),".","::"));
                 current->fileName = yyFileName;
                 current->section  = Entry::USINGDIR_SEC;
                 current_root->addSubEntry(current, current_root);
                 current = QMakeShared<Entry>();
                 initEntry();
                 BEGIN(Using);
   }

<JavaImport>({ID}{BN}*"."{BN}*)+{ID}   { 
      // class import => add as a using declaration
      lineCount();
                 QString text = QString::fromUtf8(yytext);
                 current->name = removeRedundantWhiteSpace(substitute(text,".","::"));
                 current->fileName = yyFileName;
                 if (insideD) {
                   current->section=Entry::USINGDIR_SEC;
                 } else {
                   current->section=Entry::USINGDECL_SEC;
                 }
                 current_root->addSubEntry(current, current_root);
                 current = QMakeShared<Entry>();
                 initEntry();
                 BEGIN(Using);
   }

<FindMembers>"using"{BN}+     {
                 current->startLine=yyLineNr;
                 current->startColumn = yyColNr;
                 lineCount();
                 BEGIN(Using);
   }

<Using>"namespace"{BN}+          { 
                    lineCount(); 
                    BEGIN(UsingDirective); 
   }

<Using>({ID}{BN}*("::"|"."){BN}*)*({ID}|{OPERATOR}) {

                 QString text = QString::fromUtf8(yytext);
                 lineCount();
                 current->name = text;
                 current->fileName = yyFileName;
                 current->section=Entry::USINGDECL_SEC;
                 current->startLine = yyLineNr;
                 current_root->addSubEntry(current, current_root);

                 previous = current;
                 current = QMakeShared<Entry>();

                 if (insideCSharp) /* Hack: in C# a using declaration and
                        directive have the same syntax, so we
                        also add it as a using directive here
                      */
                 {
                   current->name        = text;
                   current->fileName    = yyFileName;
                   current->startLine   = yyLineNr;
                   current->startColumn = yyColNr;
                   current->section=Entry::USINGDIR_SEC;
                   current_root->addSubEntry(current, current_root);
                   current = QMakeShared<Entry>();
                 }
                 initEntry();
                 BEGIN(Using);
   }

<Using>"="                              { 
      // C++11 style template alias?
      BEGIN(UsingAlias);
   }

<UsingAlias>";"                         {
                 previous->section=Entry::VARIABLE_SEC;
                 previous->type = "typedef "+previous->args;
                 previous->type=previous->type.simplified();
                 previous->args.resize(0);
                 previous->name=previous->name.trimmed();
                 previous->bodyLine = yyLineNr;
                 previous->m_specFlags.spec |= Entry::Alias;
                 BEGIN(FindMembers);
   }

<UsingAlias>";"{BN}*("/**"|"//!"|"/*!"|"///")"<" {
                 QString text = QString::fromUtf8(yytext);
                 docBlockContext   = UsingAliasEnd;
                 docBlockInBody    = false;

                 docBlockAutoBrief = ( text[yyleng-2]=='*' && Config::getBool("javadoc-auto-brief") ) ||
                                     ( text[yyleng-2]=='!' && Config::getBool("qt-auto-brief") );
                 QString indent;
                 indent.fill(' ',computeIndent(text, g_column));
                 docBlock=indent;
                 lineCount();

                 docBlockTerm = ';';
                 if (text[yyleng-3]=='/') {
                   startCommentBlock(true);
                   BEGIN( DocLine );
                 } else {
                   startCommentBlock(false);
                   BEGIN( DocBlock );
                 }
   }

<UsingAlias>.                           {
                                          QString text = QString::fromUtf8(yytext);
                                          previous->args += text;
   }

<UsingAlias>\n                          {
                                          QString text = QString::fromUtf8(yytext);
                                          previous->args += text;
                                          lineCount();
   }

<UsingAliasEnd>";"                      {
                                          previous->doc   = current->doc;
                                          previous->brief = current->brief;
                                          current->doc.resize(0);
                                          current->brief.resize(0);
                                          unput(';');
                                          BEGIN(UsingAlias);
   }

<UsingDirective>{SCOPENAME}      { 
                 QString text = QString::fromUtf8(yytext);
                 current->name = removeRedundantWhiteSpace(text);
                 current->fileName = yyFileName;
                 current->section=Entry::USINGDIR_SEC;
                 current_root->addSubEntry(current, current_root);
                 current = QMakeShared<Entry>();
                 initEntry();
                 BEGIN(Using);
   }

<Using>";"           { 
      BEGIN(FindMembers); 
   }
<FindMembers>{SCOPENAME}{BN}*"<>"   { 
      // guided template decl
      QString text = QString::fromUtf8(yytext);
      addType( current );
      current->name= text.left(text.length() - 2);
   }

<FindMembers>{SCOPENAME}{BN}*/"<"   { 
      // Note: this could be a return type
                 QString text = QString::fromUtf8(yytext);
                 roundCount = 0;
                 sharpCount = 0;
                 lineCount();
                 addType( current );
                 current->name = text;
                 current->name = current->name.trimmed();

                 if (nameIsOperator(current->name))
                   BEGIN( Operator );
                 else
                   BEGIN( EndTemplate );
   }

<FindMemberName>{SCOPENAME}{BN}*/"<"   {
                 QString text = QString::fromUtf8(yytext);
                 sharpCount=0;
                 roundCount=0;
                 lineCount();
                 current->name += text.trimmed();
                 if (nameIsOperator(current->name)) {
                   BEGIN( Operator );
                 } else {
                   BEGIN( EndTemplate );
                 }
   }

<EndTemplate>"<<<"                   {
                 if (! insidePHP) {
                   REJECT;
                 } else {
                   lastHereDocContext = YY_START;
                   BEGIN(HereDoc);
                 }
   }

<ClassTemplSpec,EndTemplate>"<<"    {
                    QString text = QString::fromUtf8(yytext);   
                    current->name += text;                    
   }

<EndTemplate>"<"        {
                   QString text = QString::fromUtf8(yytext);
                   if (roundCount == 0) {                    
                     sharpCount++;
                   }
                   current->name += text;
   }

<ClassTemplSpec,EndTemplate>">>"    {
                 QString text = QString::fromUtf8(yytext);
                 if (insideJava || insideCSharp || insideCli || roundCount == 0) {
                   unput('>');
                   unput(' ');
                   unput('>');
                 } else {
                      current->name += text;
                 }                    
   }

<EndTemplate>">"        {
                 current->name += '>';                 
                 if (roundCount==0 && --sharpCount <= 0) {
                   BEGIN(FindMembers);
                 }
   }

<EndTemplate>">"{BN}*"("      {
                 lineCount();
                 current->name += '>';
                 // *currentTemplateSpec+='>';
                 if (roundCount==0 && --sharpCount<=0)
                 {
                   current->bodyLine = yyLineNr;
                   current->args = "(";
                   currentArgumentContext = FuncQual;
                   fullArgString = current->args;
                   copyArgString = &current->args;
                   BEGIN( ReadFuncArgType ) ;
                 }
   }

<EndTemplate>">"{BN}*/"("({BN}*{ID}{BN}*"::")*({BN}*"*"{BN}*)+ { 
      // function pointer returning a template instance
                    lineCount();
                    current->name+='>';
                                          if (roundCount==0)
                                          {
                   BEGIN(FindMembers);
                                          }
   }

<EndTemplate>">"{BN}*/"::"       {
                    lineCount();
                    current->name+='>';
                    // *currentTemplateSpec+='>';
                 if (roundCount==0 && --sharpCount<=0)
                 {
                   BEGIN(FindMemberName);
                 }
   }

<ClassTemplSpec,EndTemplate>"("         { 
                                          QString text = QString::fromUtf8(yytext);
                                          current->name += text[0];
                                          roundCount++;
   }

<ClassTemplSpec,EndTemplate>")"         { 
                                          QString text = QString::fromUtf8(yytext);
                                          current->name += text[0];
                                          if (roundCount>0) {
                                             roundCount--;
                                          }      
   }

<EndTemplate>.             {
                    QString text = QString::fromUtf8(yytext);
                    current->name += text[0];
   }

<FindMembers>"define"{BN}*"("{BN}*["']    {
                 if (insidePHP) {
                   current->bodyLine = yyLineNr;
                   BEGIN( DefinePHP );
                 }
                 else
                   REJECT;
   }

<CopyHereDoc>{ID}                       { 
      // PHP heredoc
                 QString text = QString::fromUtf8(yytext);
                 g_delimiter = text;
                 *pCopyHereDocGString += text;
                 BEGIN(CopyHereDocEnd);
   }

<CopyHereDoc>"'"{ID}/"'"      { 
      // PHP nowdoc
                 QString text = QString::fromUtf8(yytext); 
                 g_delimiter = text.mid(1);
                 *pCopyHereDocGString += text;
                 BEGIN(CopyHereDocEnd);
   }

<HereDoc>{ID}                 { 
      // PHP heredoc
                 QString text = QString::fromUtf8(yytext);
                 g_delimiter = text;
                 BEGIN(HereDocEnd);
   }

<HereDoc>"'"{ID}/"'"          { 
      // PHP nowdoc
                 QString text = QString::fromUtf8(yytext);
                 g_delimiter = text.mid(1);
                 BEGIN(HereDocEnd);
   }

<HereDocEnd>^{ID}             { 
      // id at start of the line could mark the end of the block
                 QString text = QString::fromUtf8(yytext);
                 if (g_delimiter == text) {
                    // it is the end marker                 
                      BEGIN(lastHereDocContext);
                 }
   }

<HereDocEnd>.           { 
   }

<CopyHereDocEnd>^{ID}   { 
      // id at start of the line could mark the end of the block
                 QString text = QString::fromUtf8(yytext);
                 *pCopyHereDocGString += text;
                  if (g_delimiter == text) { 
                     // it is the end marker                 
                     BEGIN(lastHereDocContext);
                 }
   }

<CopyHereDocEnd>\n         {
      QString text = QString::fromUtf8(yytext);
      *pCopyHereDocGString += text;
   }

<CopyHereDocEnd>.             {
      QString text = QString::fromUtf8(yytext);
      *pCopyHereDocGString += text;
   }

<FindMembers>"Q_OBJECT"    { 
      // Q_object, ignore
   }

<FindMembers>"Q_PROPERTY"  { 
      // Q_property
     current->protection = Public;
     current->mtype = MethodTypes::Property;
     current->type.resize(0);
     BEGIN(QtPropType);
   }

<QtPropType>"("            { 
      // start of property arguments
   }

<QtPropAttr>")"            { 
      // end of property arguments
      unput(';');
      BEGIN(FindMembers);
   }

<QtPropType>"const"|"volatile"|"unsigned"|"signed"|"long"|"short" {
                 QString text = QString::fromUtf8(yytext);
                 current->type += text;
   }

<QtPropType>{B}+           {
                 QString text = QString::fromUtf8(yytext);
                 current->type += text;
   }

<QtPropType>({TSCOPE}"::")*{TSCOPE}    {
                 QString text = QString::fromUtf8(yytext);
                 current->type += text;
                 BEGIN(QtPropName);
   }

<QtPropName>{ID}           {
                  QString text = QString::fromUtf8(yytext);
                  current->name = text;
                  BEGIN(QtPropAttr);
   }

<QtPropAttr>"READ"         {
                 current->m_specFlags.m_isReadable = true;
                 BEGIN(QtPropRead);
   }

<QtPropAttr>"WRITE"        {
                 current->m_specFlags.m_isWritable = true;
                 BEGIN(QtPropWrite);
   }

<QtPropAttr>"RESET"{B}+{ID}      { 
      // reset method => not supported yet
   }

<QtPropAttr>"SCRIPTABLE"{B}+{ID}    { 
      // scriptable property => not supported yet
   }

<QtPropAttr>"DESIGNABLE"{B}+{ID}    { 
      // designable property => not supported yet
   }

<QtPropRead>{ID}           {
      QString text = QString::fromUtf8(yytext);
      current->m_read = text;
      BEGIN(QtPropAttr);
   }

<QtPropWrite>{ID}             {
      QString text = QString::fromUtf8(yytext);
      current->m_write = text;
      BEGIN(QtPropAttr);
   }


   /* begin CopperSpice specific parsing */
<FindMembers>"CS_OBJECT"           { 
      // cs_object
      BEGIN(CsObject);
   }

<CsObject>"("                      {
      BEGIN(CsObjectName);
   }

<CsObjectName>{ID}                 {
      // class name, do nothing
      BEGIN(CsObjectClose);
   }

<CsObjectClose>")"                 { 
      // end of cs_object
      BEGIN(FindMembers);
   }

<FindMembers>{CS_MACRO}            { 
      // ignore this macro
      roundCount = 0;
      BEGIN(CsIgnore);
   }

<CsIgnore>"("                      {
      ++roundCount;
   }

<CsIgnore>")"                      { 
      // end of macro ignore
      --roundCount;
      
      if (roundCount == 0)  {
        BEGIN(FindMembers);
      }
   }

<FindMembers>"CS_REGISTER_ENUM"     { 
      // cs_register_enum
      BEGIN(CsRegEnum);
   }

<CsRegEnum>"("                      {
      BEGIN(FindMembers);
   }

<CsPropClose>")"                    { 
      // end property
     lineCount();
     current->fileName    = yyFileName;
     current->startLine   = yyLineNr;
     current->startColumn = yyColNr;

     current->name        = current->name.trimmed();
     current->section     = Entry::VARIABLE_SEC;

     // unknown data type, decipher later on
     // current->type = "";

     current_root->addSubEntry(current, current_root);
     current = QMakeShared<Entry>();
     initEntry();
     BEGIN(FindMembers);
   }

<FindMembers>"CS_PROPERTY_READ"     {
     current->protection    = Public;
     current->mtype = MethodTypes::Property;
     current->m_specFlags.m_isReadable = true;
     current->type.resize(0);
     BEGIN(CsPropReadVar);
   }

<CsPropReadVar>"("                   {
   }

<CsPropReadVar>{ID}                  {   
      // cs property read name
      QString text = QString::fromUtf8(yytext);
      current->name = text;
   }

<CsPropReadVar>","                   {
      BEGIN(CsPropReadMethod);
   }

<CsPropReadMethod>{ID}               {
                 // cs property read method
                 QString text = QString::fromUtf8(yytext);
                 current->m_read = text;
                 BEGIN(CsPropClose);
   }

<FindMembers>"CS_PROPERTY_WRITE"        {
                 current->protection = Public;
                 current->mtype = MethodTypes::Property;
                 current->m_specFlags.m_isWritable = true;
                 current->type.resize(0);
                 BEGIN(CsPropWriteVar);
   }

<CsPropWriteVar>"("                  {
   }

<CsPropWriteVar>{ID}                    {
      // cs property write name
      QString text = QString::fromUtf8(yytext);
      current->name = text;
   }

<CsPropWriteVar>","                  {
      BEGIN(CsPropWriteMethod);
   }

<CsPropWriteMethod>{ID}                 {
                 QString text = QString::fromUtf8(yytext);
                 current->m_write = text;
                 BEGIN(CsPropClose);
   }

<FindMembers>"CS_PROPERTY_RESET"        {
                 QString text = QString::fromUtf8(yytext);
                 current->protection = Public;
                 current->mtype = MethodTypes::Property;
                 current->m_specFlags.m_isReset = true;
                 current->type.resize(0);
                 BEGIN(CsPropResetVar);
   }

<CsPropResetVar>"("                  {
   }

<CsPropResetVar>{ID}                    {
                 QString text = QString::fromUtf8(yytext);
                 current->name = text;
   }

<CsPropResetVar>","                  {
                 BEGIN(CsPropResetMethod);
   }

<CsPropResetMethod>{ID}                 {
                 QString text = QString::fromUtf8(yytext);
                 current->m_reset = text;
                 BEGIN(CsPropClose);
   }

<FindMembers>"CS_PROPERTY_NOTIFY"       {
                 current->protection    = Public;
                 current->mtype = MethodTypes::Property;
                 current->m_specFlags.m_isNotify = true;
                 current->type.resize(0);
                 BEGIN(CsPropNotifyVar);
   }

<CsPropNotifyVar>"("                  {
   }

<CsPropNotifyVar>{ID}                 {
                 QString text = QString::fromUtf8(yytext);
                 current->name = text;
   }

<CsPropNotifyVar>","                    {
                 BEGIN(CsPropNotifyMethod);
   }

<CsPropNotifyMethod>{ID}              {
                 QString text = QString::fromUtf8(yytext);
                 current->m_notify = text;
                 BEGIN(CsPropClose);
   }

<FindMembers>"CS_PROPERTY_REVISION"        {
                 current->protection    = Public;
                 current->mtype = MethodTypes::Property;
                 current->m_specFlags.m_isRevision = true;
                 current->type.resize(0);
                 BEGIN(CsPropVar);
   }

<FindMembers>"CS_PROPERTY_DESIGNABLE"      {
                 current->protection     = Public;
                 current->mtype = MethodTypes::Property;
                 current->m_specFlags.m_isDesignable = true;
                 current->type.resize(0);
                 BEGIN(CsPropVar);
   }

<FindMembers>"CS_PROPERTY_SCRIPTABLE"      {
                 current->protection    = Public;
                 current->mtype = MethodTypes::Property;
                 current->m_specFlags.m_isScriptable = true;
                 current->type.resize(0);
                 BEGIN(CsPropVar);
   }

<FindMembers>"CS_PROPERTY_STORED"       {
                 current->protection    = Public;
                 current->mtype = MethodTypes::Property;
                 current->m_specFlags.m_isStored = true;
                 current->type.resize(0);
                 BEGIN(CsPropVar);
   }

<FindMembers>"CS_PROPERTY_USER"         {
                 current->protection    = Public;
                 current->mtype = MethodTypes::Property;
                 current->m_specFlags.m_isUser  = true;
                 current->type.resize(0);
                 BEGIN(CsPropVar);
   }

<FindMembers>"CS_PROPERTY_CONSTANT"     {
                 current->protection    = Public;
                 current->mtype = MethodTypes::Property;
                 current->m_specFlags.m_isConstant  = true;
                 current->type.resize(0);
                 BEGIN(CsPropJustVar);
   }

<FindMembers>"CS_PROPERTY_FINAL"        {
      current->protection = Public;
      current->mtype = MethodTypes::Property;
      current->m_specFlags.m_isFinal = true;
      current->type.resize(0);
      BEGIN(CsPropJustVar);
   }

<CsPropVar>"("                    {
                 roundCount = 1;
                }
<CsPropVar>{ID}                   {
                 QString text = QString::fromUtf8(yytext);
                 current->name = text;
   }

<CsPropVar>","                    {
                 BEGIN(CsPropMethod);
   }

<CsPropMethod>{ID}                {
   }

<CsPropMethod>"("                 {
                 ++roundCount;
   }

<CsPropMethod>")"                 {
      --roundCount;
      
      if (roundCount == 0)  {
        lineCount();
        current->fileName    = yyFileName;
        current->startLine   = yyLineNr;
        current->startColumn = yyColNr;
      
        current->name        = current->name.trimmed();
        current->section     = Entry::VARIABLE_SEC;
      
        current_root->addSubEntry(current, current_root);
        current = QMakeShared<Entry>();
        initEntry();
        BEGIN(FindMembers);
      }
   }

<CsPropJustVar>"("   {                
   }

<CsPropJustVar>{ID}   {
      QString text = QString::fromUtf8(yytext);
      current->name = text;
      BEGIN(CsPropClose);
   }

<FindMembers>"CS_SIGNAL_1"              {
      current->mtype = MethodTypes::Signal;
      current->type.resize(0);
      BEGIN(CsSignal);
   }

<CsSignal>"("                  {
      BEGIN(CsSignalProtection);
   }

<CsSignalProtection>{ID}       {
      QString text = QString::fromUtf8(yytext);
      Protection visibility = getProtection(text);
      current->protection = visibility;
   }

<CsSignalProtection>","           {
      BEGIN(CsSignalSlotMethod);
   }

<CsSignalSlotMethod>{SCOPENAME}    {
                  QString text = QString::fromUtf8(yytext);
                  yyBegColNr  = yyColNr;
                  yyBegLineNr = yyLineNr;
                  lineCount();

                  if (text == "decltype") {
                     // C++11 decltype(x)
                     current->type += text;
                     BEGIN(DeclType);

                  } else {
                     addType(current);
                     current->name += text;

                     if (current->name.startsWith("static ")){
                        current->stat = true;
                        current->name = current->name.mid(7);

                     } else if (current->name.startsWith("inline ") ) {

                        if (current->type.isEmpty()) {
                           current->type ="inline";
                        } else {
                           current->type +="inline ";
                        }

                        current->name = current->name.mid(7);

                     } else if (current->name.startsWith("const ") ) {

                        if (current->type.isEmpty()) {
                           current->type = "const";

                        } else {
                           current->type += "const ";
                        }

                        current->name = current->name.mid(6);
                     }

                     if (nameIsOperator(text)) {
                        // BEGIN(Operator);    broom (on hold, may need to add)

                     } else {
                        BEGIN(CsSignalSlotMethod);
                     }
                  }
   }

<CsSignalSlotMethod>"("           {
                 QString text = QString::fromUtf8(yytext);
                 if (! current->name.isEmpty()) {
                   current->args     = text;
                   current->bodyLine = yyLineNr;
                   fullArgString     = current->args;
                   copyArgString     = &current->args;
                   currentArgumentContext = CsSignalSlotClose;
                   BEGIN(ReadFuncArgType);
                 }
   }

<CsSignalSlotClose>")"             {  
      // end of cs signal
      lineCount();
      current->fileName    = yyFileName;
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;
      
      current->name        = current->name.trimmed();
      current->section     = Entry::FUNCTION_SEC;
      
      currentArgumentContext = FindMembers;
      current_root->addSubEntry(current, current_root);
      current = QMakeShared<Entry>();
      initEntry();
      
      BEGIN(FindMembers);
   }

<FindMembers>"CS_SIGNAL_2"              {  
      // cs_signal_2, do nothing
      BEGIN(CsSignalSlot2);
   }

<FindMembers>"CS_SLOT_2"                {  
      // cs_slot_2, do nothing
      BEGIN(CsSignalSlot2);
   }

<FindMembers>"CS_SIGNAL_OVERLOAD"       {  
      // cs_signal_overload, do nothing
      BEGIN(CsSignalSlot2);
   }

<FindMembers>"CS_SLOT_OVERLOAD"         {  
      // cs_slot_overload, do nothing
      BEGIN(CsSignalSlot2);
   }

<CsSignalSlot2>"("                      {
      roundCount = 1;
      BEGIN(CsSignalSlot2Name);
   }

<CsSignalSlot2Name>{ID}                 {
      QString text = QString::fromUtf8(yytext);   
      current->name += text;
   }

<CsSignalSlot2Name>","                  {
      BEGIN(CsSignalSlot2Extra);
   }

<CsSignalSlot2Extra>"("                 {
      ++roundCount;
   }

<CsSignalSlot2Extra>{ID}                {
   }

<CsSignalSlot2Name,CsSignalSlot2Extra>")"        { 
      // end of signal2 or slot2 or overload
                 --roundCount;

                 if (roundCount == 0)  {
                    current = QMakeShared<Entry>();
                    initEntry();
                    BEGIN(FindMembers);
                 }
   }

<FindMembers>"CS_SLOT_1"       {
                  current->mtype = MethodTypes::Slot;
                  current->type.resize(0);
                  BEGIN(CsSlotProtection);
   }

<CsSlotProtection>{ID}       {
                  QString text = QString::fromUtf8(yytext);
                  Protection visibility = getProtection(text);
                  current->protection = visibility;
   }

<CsSlotProtection>","           {
                  BEGIN(CsSignalSlotMethod);
   }

   /* end CopperSpice specific parsing */

<FindMembers>"friend"{BN}+("class"|"union"|"struct"){BN}+ {
                 QString text = QString::fromUtf8(yytext);
                 current->name = text;
                 BEGIN(FindMembers);
   }

<FindMembers,FindMemberName>{SCOPENAME}   {
                 QString text = QString::fromUtf8(yytext);
                 if (insideCpp || insideObjC)  {
                     current->id = ClangParser::instance()->lookup(yyLineNr, text);
                 }

                 yyBegColNr  = yyColNr;
                 yyBegLineNr = yyLineNr;
                 lineCount();

                 if (insideIDL && text == "cpp_quote") {
                    BEGIN(CppQuote);

                 } else if ((insideIDL || insideJava || insideD) && text == "import") {
                   if (insideIDL) {
                     BEGIN(NextSemi);

                   } else  {
                     // insideJava or insideD
                     BEGIN(JavaImport);

                   }     
      
                 } else if (insidePHP && text == "use") {
                   BEGIN(PHPUse);

                 } else if (insideJava && text == "package") {
                   lineCount();
                   BEGIN(PackageName);

                 } else if (insideIDL && text == "case") {
                   BEGIN(IDLUnionCase);

                 } else if (insideTryBlock && text == "catch") {
                   insideTryBlock = false;
                   BEGIN(TryFunctionBlock);

                 } else if (insideCpp && text == "alignas") {
                   lastAlignAsContext = YY_START;
                   BEGIN(AlignAs);

                 } else if (insideJS && text == "var") { 
                   // javascript variable
                   current->type="var";

                 } else if (insideJS && text == "function") {
                   // javascript function
                   current->type = "function";

                 } else if (insideCSharp && text == "this") {
                   // C# indexer
                   addType( current ) ;
                   current->name="this";
                   BEGIN(CSIndexer);

                 } else if (insideCpp && text == "static_assert") {                  
                    // C++11 static_assert
                    BEGIN(StaticAssert);

                 } else if (insideCpp && text == "decltype") {
                    // C++11 decltype(x)
                    current->type += text;
                    BEGIN(DeclType);

                 } else {

                   if (YY_START == FindMembers) {
                     addType(current);
                   }

                   bool javaLike = insideJava || insideCSharp || insideD || insidePHP || insideJS;

                   if (javaLike && text == "public") {
                     current->protection = Public;

                   } else if (javaLike && text == "protected") {
                     current->protection = Protected;

                   } else if (javaLike && text == "internal") {
                     current->protection = Package;

                   } else if (javaLike && text == "private") {
                     current->protection = Private;

                   } else if (javaLike && text == "static") {

                     if (YY_START == FindMembers)  {
                       current->name  = text;
                     } else {
                       current->name += text;
                     }

                     current->stat = true; 

                   } else {
                     if (YY_START == FindMembers) {
                       current->name  = text;
                     }  else  {
                       current->name += text;
                     }

                     if (current->name.left(7)=="static ") {
                        current->stat = true;
                        current->name = current->name.mid(7);

                     } else if (current->name.left(7)=="inline ")  {

                        if (current->type.isEmpty()) {
                          current->type = "inline";
                        } else {
                          current->type += "inline ";
                        }

                        current->name = current->name.mid(7);

                     } else if (current->name.left(6)=="const ") {

                        if (current->type.isEmpty()) {
                          current->type = "const";

                        } else {
                          current->type += "const ";

                        }

                        current->name=current->name.mid(6);
                     }

                   }
                   
                   if (nameIsOperator(text)) {
                     BEGIN(Operator);
                   } else {
                     BEGIN(FindMembers);
                   }
                 }
   }

<StaticAssert>"("   {
      lastSkipRoundContext = FindMembers;
      roundCount=0;
      BEGIN(SkipRound);
   }

<StaticAssert>{BN}+   { 
      lineCount(); 
   }

<StaticAssert>.     { 
      // variable with static_assert as name?
      unput(*yytext);
      BEGIN(FindMembers);
   }

<DeclType>"("   {
      QString text = QString::fromUtf8(yytext);
      current->type += text;
      lastRoundContext = FindMembers;
      pCopyRoundString = &current->type;
      roundCount = 0;
      BEGIN(CopyRound);
   }

<DeclType>{BN}+   { 
      lineCount(); 
   }

<DeclType>.      {
         unput(*yytext);
         BEGIN(FindMembers);
   }

<CSIndexer>"["[^\n\]]*"]"     {
                    QString text = QString::fromUtf8(yytext);
                    current->name += removeRedundantWhiteSpace(text);
                    BEGIN(FindMembers);
   }

<FindMembers>[0-9]{ID}        { 
      // some number where we did not expect one
   }

<FindMembers>"."        {
                    if (insideJava || insideCSharp || insideD)
                 {
                   current->name+=".";
                 }
   }

<FindMembers>"::"          {
                 QString text = QString::fromUtf8(yytext);
                 current->name += text;
   }

<CppQuote>"("{B}*"\""         {
      insideCppQuote = true;
      BEGIN(FindMembers);
   }

<IDLUnionCase>"::"
<IDLUnionCase>":"       { 
      BEGIN(FindMembers); 
   }

<IDLUnionCase>\n        { 
      lineCount(); 
   }

<IDLUnionCase>.
<TryFunctionBlock>\n          { 
      lineCount(); 
   }

<TryFunctionBlock>"{"         {
                 curlyCount=0;
                 lastCurlyContext = TryFunctionBlockEnd ;
                 BEGIN( SkipCurly );
   }

<TryFunctionBlock>.
<TryFunctionBlockEnd>{BN}*"catch"   { 
      // {BN}* added to fix bug 611193
      lineCount(); 
      BEGIN(TryFunctionBlock);       
   }

<TryFunctionBlockEnd>\n          { 
      unput(*yytext); // rule added to fix bug id 601138
      BEGIN( FindMembers );
   }

<TryFunctionBlockEnd>.        { 
                    unput(*yytext);
                    BEGIN( FindMembers );
   }

<EndCppQuote>")"        {
                    insideCppQuote=false;
                 BEGIN(FindMembers);
   }

<FindMembers,FindFields>{B}*"#"     { 
      if (insidePHP) {
         REJECT;
      }
      lastCPPContext = YY_START;
      BEGIN( SkipCPP );
   }

<FindMembers,FindFields>{B}*"#"{B}*("cmake")?"define"    {
                    if (insidePHP) {
                       REJECT;
                    }
                    current->bodyLine = yyLineNr;
                    lastDefineContext = YY_START;
                    BEGIN( Define );
   }

<FindMembers,ReadBody,ReadNSBody,ReadBodyIntf,SkipCurly,SkipCurlyCpp>{B}*"#"{B}+[0-9]+{B}+/"\""    {
   /* line control directive */
                 QString text = QString::fromUtf8(yytext);
                 QRegExp intMatch("\\d+");
                 int startPos = intMatch.indexIn(text);
                 QString tmp = intMatch.cap(0); 
                 yyLineNr = tmp.toInt();                 
                 lastPreLineCtrlContext = YY_START;

                 if (YY_START == ReadBody || YY_START == ReadNSBody || YY_START == ReadBodyIntf) {
                    current->program += text;
                 }
                 BEGIN( PreLineCtrl );
   }

<PreLineCtrl>"\""[^\n\"]*"\""       {
                 QString text = QString::fromUtf8(yytext);
                 yyFileName = stripQuotes(text);

                 if (lastPreLineCtrlContext==ReadBody ||
                     lastPreLineCtrlContext==ReadNSBody ||
                     lastPreLineCtrlContext==ReadBodyIntf)
                 {
                   current->program += text;
                 }
   }

<PreLineCtrl>.             {
                 QString text = QString::fromUtf8(yytext);
                 if (lastPreLineCtrlContext==ReadBody ||
                     lastPreLineCtrlContext==ReadNSBody ||
                     lastPreLineCtrlContext==ReadBodyIntf)
                 {
                      current->program += text;
                 }
   }

<PreLineCtrl>\n            {
                 QString text = QString::fromUtf8(yytext);
                 if (lastPreLineCtrlContext==ReadBody ||
                     lastPreLineCtrlContext==ReadNSBody ||
                     lastPreLineCtrlContext==ReadBodyIntf)
                 {
                      current->program += text;
                 }
                    lineCount();
                    BEGIN( lastPreLineCtrlContext );
   }

<SkipCPP>.
<SkipCPP>\\[\r]*"\n"[\r]*     { 
                        lineCount();
   }

<SkipCPP>[\r]*\n[\r]*         { 
                 lineCount();
                 BEGIN( lastCPPContext) ;
   }

<Define>{ID}{B}*"("        {
                 QString text = QString::fromUtf8(yytext);
                 current->name = text;
                 current->name = current->name.left(current->name.length() - 1).trimmed();
                 current->args = "(";
                 current->bodyLine = yyLineNr;
                 currentArgumentContext = DefineEnd;
                 fullArgString=current->args;
                 copyArgString=&current->args;
                 BEGIN( ReadFuncArgType ) ;
   }

 /*
<DefineArg>")"             {      
      current->args += ')';
      BEGIN( DefineEnd );
   }

<DefineArg>.            {
      QString text = QString::fromUtf8(yytext);
      current->args += text[0];
   }
  */

<Define>{ID}            {     
      QString text = QString::fromUtf8(yytext);
      if (insideCpp || insideObjC) {
         current->id = ClangParser::instance()->lookup(yyLineNr, text);
      }
      current->bodyLine = yyLineNr;
      current->name = text;
      BEGIN(DefineEnd);
   }

<DefineEnd>\n           {                   
                 lineCount();
                 current->fileName   = yyFileName;
                 current->startLine  = yyLineNr;
                 current->startColumn = yyColNr;
                 current->type.resize(0);
                 current->args       = current->args.simplified();
                 current->name       = current->name.trimmed();
                 current->section    = Entry::DEFINE_SEC;
                 current_root->addSubEntry(current, current_root);
                 current = QMakeShared<Entry>();
                 initEntry();
                 BEGIN(lastDefineContext);
   }

<DefinePHPEnd>";"          {
                    //printf("End define\n");
                 current->fileName   = yyFileName;
                 current->startLine  = yyLineNr;
                 current->startColumn = yyColNr;
                 current->type.resize(0);
                 current->type       = "const";

                 QString init = current->initializer;
                 init = init.simplified();
                 init = init.left(init.length()-1);

                 current->initializer = init;
                 current->name       = current->name.trimmed();
                 current->section    = Entry::VARIABLE_SEC;
                 current_root->addSubEntry(current, current_root);
                 current = QMakeShared<Entry>();
                 initEntry();
                 BEGIN(FindMembers);
   }

<DefinePHPEnd>.
<DefineEnd>\\[\r]?\n          {
                   lineCount();
   }

<DefineEnd>\"           {
                 if (insideIDL && insideCppQuote)
                 {
                   BEGIN(EndCppQuote);
                 }
                 else
                 {
                   lastStringContext = DefineEnd;
                   BEGIN(SkipString);
                 }
   }

<DefineEnd>.
<DefinePHP>{ID}["']{BN}*","{BN}*    {
                 QString text = QString::fromUtf8(yytext);
                 current->name = text;
                 current->name = current->name.trimmed();
                 current->name = current->name.left(current->name.length() - 1).trimmed();
                 current->name = current->name.left(current->name.length() - 1);
                 current->bodyLine = yyLineNr;
                 lastRoundContext  = DefinePHPEnd;
                 pCopyRoundGString = &current->initializer;
                 roundCount = 0;
                 BEGIN( GCopyRound );
   }

<FindMembers>[\^%]         {  
   // ^ and % are C++/CLI extensions
                 QString text = QString::fromUtf8(yytext);
                 if (insideCli) {
                      addType( current );
                      current->name = text ;
                 }
                 else
                 {
                   REJECT;
                 }
   }

<FindMembers>[*&]+         {
                    QString text = QString::fromUtf8(yytext);
                    current->name += text;
                    addType( current );
   }

<FindMembers,MemberSpec,Function,NextSemi,EnumBaseType,BitFields,ReadInitializer,OldStyleArgs>";"{BN}*("/**"|"//!"|"/*!"|"///")"<" {    
                 // */ (editor syntax fix)

                 QString text = QString::fromUtf8(yytext);

                 if (current->bodyLine == -1) {
                   current->bodyLine = yyLineNr;
                 }

                 docBlockContext   = YY_START;
                 docBlockInBody    = false;
                 docBlockAutoBrief = ( text[text.length() - 2] == '*' && Config::getBool("javadoc-auto-brief") ) ||
                                     ( text[text.length() - 2] == '!' && Config::getBool("qt-auto-brief")  );

                 QString indent;
                 indent.fill(' ',computeIndent(text, g_column));
                 docBlock = indent;

                 lineCount();
                 docBlockTerm = ';';
                 if (YY_START == EnumBaseType && current->section == Entry::ENUM_SEC) {
                    current->bitfields = ":" + current->args;
                    current->args.resize(0);
                    current->section = Entry::VARIABLE_SEC;
                 }

                 if (text[text.length() - 3] == '/')  {
                   startCommentBlock(true);
                   BEGIN( DocLine );
                 }
                 else   {
                   startCommentBlock(false);
                   BEGIN( DocBlock );
                 }
   }

<MemberSpec,FindFields,FindMembers,NextSemi,EnumBaseType,BitFields,ReadInitializer,OldStyleArgs>","{BN}*("/**"|"//!"|"/*!"|"///")"<" {
                 // */ (editor syntax fix)

                 QString text = QString::fromUtf8(yytext);
                 docBlockContext   = YY_START;
                 docBlockInBody    = false;

                 docBlockAutoBrief = ( text[text.length() - 2] == '*' && Config::getBool("javadoc-auto-brief") ) ||
                                     ( text[text.length() - 2] == '!' && Config::getBool("qt-auto-brief")  );

                 QString indent;
                 indent.fill(' ',computeIndent(text, g_column));
                 docBlock=indent;
                 lineCount();

                 docBlockTerm = ',';
                 if (YY_START == EnumBaseType && current->section == Entry::ENUM_SEC)
                 {
                    current->bitfields = ":"+current->args;
                    current->args.resize(0);
                    current->section=Entry::VARIABLE_SEC;
                 }
                
                 if (text[text.length() - 3] == '/')
                 {
                   startCommentBlock(true);
                   BEGIN( DocLine );
                 }
                 else
                 {
                   startCommentBlock(false);
                   BEGIN( DocBlock );
                 }
   }

<DefineEnd,FindFields,FindFieldArg,ReadInitializer,OldStyleArgs>{BN}*("/**"|"//!"|"/*!"|"///")"<" {
                 // */ (editor syntax fix)

                 QString text = QString::fromUtf8(yytext);
                 if (current->bodyLine == -1) {
                   current->bodyLine = yyLineNr;
                 }

                 docBlockContext   = YY_START;
                 docBlockInBody    = false;
                 docBlockAutoBrief = ( text[text.length() - 2] == '*' && Config::getBool("javadoc-auto-brief") ) ||
                                     ( text[text.length() - 2] == '!' && Config::getBool("qt-auto-brief") );
                 QString indent;
                 indent.fill(' ', computeIndent(text, g_column));
                 docBlock = indent;
                 lineCount();

                 docBlockTerm = 0;
                 if (text[text.length() - 3] == '/')
                 {
                   startCommentBlock(true);
                   BEGIN( DocLine );
                 }
                 else
                 {
                   startCommentBlock(false);
                   BEGIN( DocBlock );
                 }
   }

<FindMembers,FindFields>("//"([!/]?){B}*{CMD}"{")|("/*"([!*]?){B}*{CMD}"{")   {
                 // */ (editor syntax fix)
              
                 QString text = QString::fromUtf8(yytext);
                 if (previous && previous->section == Entry::GROUPDOC_SEC) {
                   // link open command to the group defined in the previous entry
                   openGroup(previous, yyFileName, yyLineNr);
                 } else {
                   // link open command to the current entry
                   openGroup(current,yyFileName,yyLineNr);
                 }

                 //current = tmp;
                 initEntry();

                 if (text[1] == '/') {
                   if (text[2] == '!' || text[2] == '/') {
                     docBlockContext   = YY_START;
                     docBlockInBody    = false;
                     docBlockAutoBrief = false;
                     docBlock.resize(0);
                     docBlockTerm = 0;
                     startCommentBlock(true);
                     BEGIN(DocLine);
                   }
                   else
                   {
                     lastCContext = YY_START;
                     BEGIN(SkipCxxComment);
                   }

                 } else {
                   if (text[2] == '!' || text[2] == '*') {
                     docBlockContext   = YY_START;
                     docBlockInBody    = false;
                     docBlock.resize(0);

                     docBlockAutoBrief = ( text[text.length() - 2] == '*' && Config::getBool("javadoc-auto-brief") ) ||
                                         ( text[text.length() - 2] == '!' && Config::getBool("qt-auto-brief") );
                     docBlockTerm = 0;
                     startCommentBlock(false);
                     BEGIN(DocBlock);
                   } else {
                     lastCContext = YY_START;
                     BEGIN(SkipComment);
                   }
                 }
   }

<FindMembers,FindFields,ReadInitializer>"//"([!/]?){B}*{CMD}"}".*|"/*"([!*]?){B}*{CMD}"}"[^*]*"*/"    {
                    closeGroup(current, yyFileName, yyLineNr);
                    bool insideEnum = YY_START == FindFields || 
                        (YY_START == ReadInitializer && lastInitializerContext == FindFields); 

 					closeGroup(current, yyFileName, yyLineNr, insideEnum);
   }

<FindMembers>"="                                { 
      // in PHP code this could also be due to "<?="
                 QString text = QString::fromUtf8(yytext);
                 current->bodyLine      = yyLineNr;
                 current->initializer   = text;
                 lastInitializerContext = YY_START;
                 initBracketCount=0;
                 BEGIN(ReadInitializer);
   }

<UNOIDLAttributeBlock>{BN}*[gs]"et"{BN}+"raises"{BN}*"("{BN}*{SCOPENAME}{BN}*(","{BN}*{SCOPENAME}{BN}*)*")"{BN}*";"  {
                 QString text = QString::fromUtf8(yytext);
                 lineCount();
                 current->exception += " ";
                 current->exception += removeRedundantWhiteSpace(text);
   }

<UNOIDLAttributeBlock>"}"     {
                 current->exception += " }";
                 BEGIN(FindMembers);
   }

  /* Read initializer rules */
<ReadInitializer>"("          {
                    QString text = QString::fromUtf8(yytext);
                    lastRoundContext  = YY_START;
                    pCopyRoundGString = &current->initializer;
                    roundCount = 0;
                    current->initializer += text[0];
                    BEGIN(GCopyRound);
   }

<ReadInitializer>"{"          {
                    QString text = QString::fromUtf8(yytext);
                    lastCurlyContext  = YY_START;
                    pCopyCurlyGString = &current->initializer;
                    curlyCount = 0;
                    current->initializer += text[0];
                    BEGIN(GCopyCurly);
   }

<ReadInitializer>[;,]         {
                 // printf(">> initializer `%s' <<\n",current->initializer.data());
                 QString text = QString::fromUtf8(yytext);
                 if (text[0] == ';' && (current_root->m_specFlags.spec & Entry::Enum)) {
                   current->fileName    = yyFileName;
                   current->startLine   = yyLineNr;
                   current->startColumn = yyColNr;
                   current->args        = current->args.simplified();
                   current->name        = current->name.trimmed();
                   current->section     = Entry::VARIABLE_SEC;
                   current_root->addSubEntry(current, current_root);

                   current = QMakeShared<Entry>();
                   initEntry();
                   BEGIN(FindMembers);
                 }
                 else if (text[0] == ';' || (lastInitializerContext == FindFields && initBracketCount==0)) 
                    // initBracketCount == 0 was added for bug 665778
                 {
                      unput(*yytext);
                      BEGIN(lastInitializerContext);
                 }
                 else if (text[0] == ',' && initBracketCount == 0) // for "int a=0,b=0"
                 {
                      unput(*yytext);
                      BEGIN(lastInitializerContext);
                 }
                 else
                 {
                      current->initializer += text[0];
                 }
   }

<ReadInitializer>{RAWBEGIN}             { 
      // C++11 raw string
      QString text = QString::fromUtf8(yytext);

      if (! insideCpp) { 
         REJECT;
   
      } else {                                          
        current->initializer += text;

        int i = text.indexOf('"');
        g_delimiter = text.mid(i + 1);

        g_delimiter = g_delimiter.left(g_delimiter.length() - 1);
        lastRawStringContext = YY_START;
        pCopyRawGString = &current->initializer;
        BEGIN(RawGString);
      }
   }

<RawGString>{RAWEND}                    {
                                          QString text = QString::fromUtf8(yytext);
                                          *pCopyRawGString += text;

                                          QString delimiter = text.mid(1);
                                          delimiter = delimiter.left(delimiter.length() - 1);
                                          if (delimiter == g_delimiter)
                                          {
                                            BEGIN(lastRawStringContext);
                                          }
   }

<RawGString>[^)\n]+                     {
                                          QString text = QString::fromUtf8(yytext);
                                          *pCopyRawGString += text;
   }

<RawGString>.                           {
                                          QString text = QString::fromUtf8(yytext);
                                          *pCopyRawGString += text;
   }

<RawGString>\n                          {
                                          QString text = QString::fromUtf8(yytext);
                                          *pCopyRawGString += text;
                                          lineCount();
   }

<RawString>{RAWEND}                     {
                                          QString text = QString::fromUtf8(yytext);
                                          *pCopyRawString += text;
                                          fullArgString   += text;

                                          QString delimiter = text.mid(1);
                                          delimiter = delimiter.left(delimiter.length() - 1);
                                          if (delimiter == g_delimiter) {
                                            BEGIN(lastRawStringContext);
                                          }
   }

<RawString>[^)]+                        {
                                          QString text = QString::fromUtf8(yytext);
                                          *pCopyRawString += text;
                                          fullArgString   += text;
    }

<RawString>.                            {
                                          QString text = QString::fromUtf8(yytext);
                                          *pCopyRawString += text;
                                          fullArgString   += text;
   }

<RawString>\n                           {
                                          QString text = QString::fromUtf8(yytext);
                                          *pCopyRawString += text;
                                          fullArgString   += text;
                                          lineCount();
   }

<ReadInitializer>\"        {
                 QString text = QString::fromUtf8(yytext);
                 if (insideIDL && insideCppQuote) {
                   BEGIN(EndCppQuote);
                 } else {
                   lastStringContext    =  YY_START;
                   current->initializer += text;
                   pCopyQuotedGString   =  &current->initializer;
                   BEGIN(CopyGString);
                 }
   }

<ReadInitializer>"->"         {
                    QString text = QString::fromUtf8(yytext);
                    current->initializer += text;
   }

<ReadInitializer>"<<"         {
                    QString text = QString::fromUtf8(yytext);
                    current->initializer += text;
   }

<ReadInitializer>">>"         {
                    QString text = QString::fromUtf8(yytext);
                    current->initializer += text;
   }

<ReadInitializer>[<\[{(]      {
                    QString text = QString::fromUtf8(yytext);
                    initBracketCount++;
                    current->initializer += text[0];
   }

<ReadInitializer>[>\]})]      {
                    QString text = QString::fromUtf8(yytext);
                    initBracketCount--;
                    current->initializer += text[0];
   }

<ReadInitializer>\'        {
                 QString text = QString::fromUtf8(yytext);
                 if (insidePHP) {
                   current->initializer += text;
                   pCopyQuotedGString = &current->initializer;
                   lastStringContext  = YY_START;
                   BEGIN(CopyPHPGString);
                 } else {
                   current->initializer += text;
                 }
   }

<ReadInitializer>{CHARLIT}              {
                 QString text = QString::fromUtf8(yytext);
                 if (insidePHP)
                 {
                   REJECT;
                 }
                 else
                 {
                   current->initializer += text;
                 }
   }

<ReadInitializer>\n        {
                    QString text = QString::fromUtf8(yytext);
                    current->initializer += text[0];
                    lineCount();
                  }
<ReadInitializer>"@\""        {                   
                    QString text = QString::fromUtf8(yytext);
                    current->initializer += text;
                    if (! insideCSharp && ! insideObjC)  {
                      REJECT;
                    }  else {
                       // C#/ObjC verbatim string
                       lastSkipVerbStringContext = YY_START;
                       pSkipVerbString=&current->initializer;
                       BEGIN(SkipVerbString);
                    }
   }

<SkipVerbString>[^\n"]+          {
      QString text = QString::fromUtf8(yytext);
      *pSkipVerbString += text;
   }

<SkipVerbString>"\"\""        { 
      // quote escape
      QString text = QString::fromUtf8(yytext);
      *pSkipVerbString += text;
   }

<SkipVerbString>"\""          {
                 QString text = QString::fromUtf8(yytext);
                 *pSkipVerbString += text[0];
                 BEGIN(lastSkipVerbStringContext);
   }

<SkipVerbString>\n         {
                  QString text = QString::fromUtf8(yytext);
                  *pSkipVerbString += text[0];
                  lineCount();
   }

<SkipVerbString>.          {
                 QString text = QString::fromUtf8(yytext);
                 *pSkipVerbString += text[0];
   }

<ReadInitializer>"?>"         {
                 QString text = QString::fromUtf8(yytext);
                 if (insidePHP) {
                   BEGIN( FindMembersPHP );
                 }  else {
                   current->initializer += text;
                 }   
   }

<ReadInitializer>.         {
                    QString text = QString::fromUtf8(yytext);
                    current->initializer += text[0];
   }

  /* generic quoted string copy rules */
<CopyString,CopyPHPString>\\.       {
                    QString text = QString::fromUtf8(yytext);
                    *pCopyQuotedString += text;
   }

<CopyString>\"             {
                    QString text = QString::fromUtf8(yytext);
                    *pCopyQuotedString += text[0];
                    BEGIN( lastStringContext );
   }

<CopyPHPString>\'          {
                    QString text = QString::fromUtf8(yytext);
                    *pCopyQuotedString += text[0];
                    BEGIN( lastStringContext );
   }

<CopyString,CopyPHPString>"/*"|"*/"|"//" {
                    QString text = QString::fromUtf8(yytext);
                    *pCopyQuotedString += text;
   }

<CopyString,CopyPHPString>\n     {
                    QString text = QString::fromUtf8(yytext);
                    *pCopyQuotedString += text[0];
                    lineCount();
   }

<CopyString,CopyPHPString>.      {
                    QString text = QString::fromUtf8(yytext);
                    *pCopyQuotedString += text[0];
   }

  /* generic quoted growable string copy rules */
<CopyGString,CopyPHPGString>\\.     {
                    QString text = QString::fromUtf8(yytext);
                    *pCopyQuotedGString += text;
   }

<CopyGString>\"           {
                    QString text = QString::fromUtf8(yytext);
                    *pCopyQuotedGString += text[0];
                    BEGIN( lastStringContext );
   }

<CopyPHPGString>\'         {
                    QString text = QString::fromUtf8(yytext);
                    *pCopyQuotedGString += text[0];
                    BEGIN( lastStringContext );
   }

<CopyGString,CopyPHPGString>"/*"|"*/"|"//" {
                    QString text = QString::fromUtf8(yytext);
                    *pCopyQuotedGString += text;
   }

<CopyGString,CopyPHPGString>\n      {
                    QString text = QString::fromUtf8(yytext);
                    *pCopyQuotedGString += text[0];
                    lineCount();
   }

<CopyGString,CopyPHPGString>.       {
                    QString text = QString::fromUtf8(yytext);
                    *pCopyQuotedGString += text[0];
   }

  /* generic round bracket list copy rules */
<CopyRound>\"           {
                 QString text = QString::fromUtf8(yytext);
                 *pCopyRoundString += text[0];
                 pCopyQuotedString = pCopyRoundString;
                 lastStringContext = YY_START;
                 BEGIN(CopyString);
   }

<CopyRound>"("             {
                    QString text = QString::fromUtf8(yytext);
                    *pCopyRoundString += text[0];
                    roundCount++;
   }

<CopyRound>")"             {
                 QString text = QString::fromUtf8(yytext);
                 *pCopyRoundString += text[0];
                 if (--roundCount < 0) {
                   BEGIN(lastRoundContext);
                 }
   }

<CopyRound>\n           {
                    QString text = QString::fromUtf8(yytext);
                    lineCount();
                    *pCopyRoundString += text[0];
   }

<CopyRound>\'           {
                 QString text = QString::fromUtf8(yytext);
                 if (insidePHP) {
                   current->initializer += text;
                   pCopyQuotedString = pCopyRoundString;
                   lastStringContext = YY_START;
                   BEGIN(CopyPHPString);
                 } else {
                   *pCopyRoundString += text;
                 }
   }

<CopyRound>{CHARLIT}               {
                 QString text = QString::fromUtf8(yytext);
                 if (insidePHP)  {
                   REJECT;
                 } else  {
                  *pCopyRoundString += yytext;
                 }
   }

<CopyRound>[^"'()\n]+         {
                    QString text = QString::fromUtf8(yytext);
                    *pCopyRoundString += text;
   }

<CopyRound>.            {
                    QString text = QString::fromUtf8(yytext);
                    *pCopyRoundString += text[0];
   }

  /* generic round bracket list copy rules for growable strings */
<GCopyRound>\"             {

                 QString text = QString::fromUtf8(yytext);        //  BROOM start here
                 *pCopyRoundGString += text[0];
                 pCopyQuotedGString =  pCopyRoundGString;
                 lastStringContext  =  YY_START;
                 BEGIN(CopyGString);
   }

<GCopyRound>"("            {
      QString text = QString::fromUtf8(yytext);   
      *pCopyRoundGString += text[0];
      roundCount++;
   }

<GCopyRound>")"            {
                    *pCopyRoundGString+=*yytext;
                 if (--roundCount<0)
                   BEGIN(lastRoundContext);
   }

<GCopyRound>\n             {
                                          lineCount();
                    *pCopyRoundGString+=*yytext;
   }

<GCopyRound>\'             {
                    if (insidePHP)
                 {
                   current->initializer+=yytext;
                                            pCopyQuotedGString = pCopyRoundGString;
                                            lastStringContext=YY_START;
                   BEGIN(CopyPHPGString);
                 }
                 else
                 {
                   *pCopyRoundGString+=yytext;
                 }
   }

<GCopyRound>{CHARLIT}              {
                                          if (insidePHP)
                 {
                   REJECT;
                 }
                 else
                 {
                                            *pCopyRoundGString+=yytext;
                 }
   }

<GCopyRound>[^"'()\n/]+          {
                    *pCopyRoundGString+=yytext;
                  }
<GCopyRound>.           {
                    *pCopyRoundGString+=*yytext;
   }

  /* generic curly bracket list copy rules */
<CopyCurly>\"           {
                 *pCopyCurlyString+=*yytext;
                    pCopyQuotedString=pCopyCurlyString;
                 lastStringContext=YY_START;
                 BEGIN(CopyString);
   }

<CopyCurly>\'           {
                 *pCopyCurlyString+=*yytext;
                    if (insidePHP)
                 {
                   pCopyQuotedString=pCopyCurlyString;
                   lastStringContext=YY_START;
                   BEGIN(CopyPHPString);
                 }
   }

<CopyCurly>"{"             {
                    *pCopyCurlyString+=*yytext;
                 curlyCount++;
   }

<CopyCurly>"}"             {
                 *pCopyCurlyString+=*yytext;
                 if (--curlyCount<0)
                   BEGIN(lastCurlyContext);
   }

<CopyCurly>{CHARLIT}                    { 
      if (insidePHP) {
         REJECT;
      } else {
         *pCopyCurlyString+=yytext;
      }
   }

<CopyCurly>[^"'{}\/\n]+          {
                    *pCopyCurlyString+=yytext;
   }

<CopyCurly>"/"             { 
      *pCopyCurlyString+=yytext; 
   }
<CopyCurly>\n           {
      lineCount();
      *pCopyCurlyString+=*yytext;
   }

<CopyCurly>.            {
      *pCopyCurlyString+=*yytext;
   }

  /* generic curly bracket list copy rules for growable strings */
<GCopyCurly>^"#"{B}+[0-9]+{B}+"\""[^\"\n]+"\""{B}+"1"{B}*\n? {
       // start of included file marker
   }

<GCopyCurly>^"#"{B}+[0-9]+{B}+"\""[^\"\n]+"\""{B}+"2"{B}*\n? { 
      // end of included file marker
                 QString line = QString(yytext);
                 int s = line.indexOf(' ');
                 int e = line.indexOf('"',s);
                 yyLineNr = line.mid(s,e-s).toInt();
                 if (yytext[yyleng-1]=='\n')
                 {
                                            lineCount();
                                            g_column=0;
                 }
   }

<GCopyCurly>\"             {
                 *pCopyCurlyGString+=*yytext;
                    pCopyQuotedGString=pCopyCurlyGString;
                 lastStringContext=YY_START;
                 BEGIN(CopyGString);
   }

<GCopyCurly>\'             {
                 *pCopyCurlyGString+=*yytext;
                    if (insidePHP)
                 {
                   pCopyQuotedGString=pCopyCurlyGString;
                   lastStringContext=YY_START;
                   BEGIN(CopyPHPGString);
                 }
   }

<GCopyCurly>"{"            {
                    *pCopyCurlyGString+=*yytext;
                 curlyCount++;
   }

<GCopyCurly>"}"            {
                 *pCopyCurlyGString+=*yytext;
                 if (--curlyCount<0)
                   BEGIN(lastCurlyContext);
   }

<GCopyCurly>{CHARLIT}                    { 
      if (insidePHP) {
                   REJECT;
                 }
                 else
                 {
                   *pCopyCurlyGString+=yytext;
                 }
   }

<GCopyCurly>[^"'{}\/\n,]+     {
                    *pCopyCurlyGString+=yytext;
   }

<GCopyCurly>[,]+             {
                    *pCopyCurlyGString+=yytext;
   }

<GCopyCurly>"/"            { 
      *pCopyCurlyGString+=yytext; 
   }
<GCopyCurly>\n             {
      lineCount();
      *pCopyCurlyGString+=*yytext;
   }

<GCopyCurly>.           {
      *pCopyCurlyGString+=*yytext;
   }

<FindMembers>":"        {
                 if (current->type.isEmpty() && current->name=="enum")  {
                   // see bug 69041, C++11 style anon enum: 'enum : unsigned int {...}'

                   current->section=Entry::ENUM_SEC;
                   current->name.resize(0);
                   current->args.resize(0);
                   BEGIN(EnumBaseType);
                 }
                                          else
                                          {
                                            if (current->type.isEmpty()) // anonymous padding field, e.g. "int :7;"
                   {
                     addType(current);
                     current->name = QString("__pad%1__").arg(padCount++);
                   }

                   BEGIN(BitFields);
                   current->bitfields+=":";
                                          }
   }

<BitFields>.            {
                    current->bitfields+=*yytext;
   }

<EnumBaseType>.            {
                    current->args+=*yytext;
   }

<EnumBaseType>\n        {
                                          lineCount();
                    current->args+=' ';
   }

<FindMembers>[;,]       {
                 QString oldType = current->type;
                 if (current->bodyLine==-1)
                 {
                   current->bodyLine = yyLineNr;
                 }
                 if ( insidePHP && current->type.left(3) == "var" )
                 {
                    current->type = current->type.mid(3);
                 }
                 if (isTypedef && current->type.left(8)!="typedef ")
                 {
                   current->type.prepend("typedef ");
                 }
                 bool needNewCurrent=false;
                 if (! current->name.isEmpty() && current->section != Entry::ENUM_SEC)
                 {
                   current->type=current->type.simplified();
                   current->args=removeRedundantWhiteSpace(current->args);
                   current->name=current->name.trimmed();
                   if (current->section==Entry::CLASS_SEC) // remove spec for "struct Bla bla;"
                   {
                     current->m_specFlags = Entry::SpecifierFlags{};
                   }
                   current->section = Entry::VARIABLE_SEC;
                   current->fileName = yyFileName;
                   current->startLine = yyBegLineNr;
                   current->startColumn = yyBegColNr;
                   current_root->addSubEntry( current, current_root);
                   needNewCurrent=true;
                 }
                 if ( *yytext == ',')
                 {
                   bool stat = current->stat;
                   if (needNewCurrent) {
                     current = QMakeShared<Entry>(*current);
                     initEntry();
                   }
                   current->stat = stat; // the static attribute holds for all variables
                   current->name.resize(0);
                   current->args.resize(0);
                   current->brief.resize(0);
                   current->doc.resize(0);
                   current->initializer.resize(0);
                   current->bitfields.resize(0);
                   int i=oldType.length();

                   while (i>0 && (oldType[i-1]=='*' || oldType[i-1]=='&' || oldType[i-1]==' ')) i--;
                   current->type = oldType.left(i);

                 } else {
                   mtype = MethodTypes::Method;
                   virt  = Normal;

                   if (needNewCurrent) {
                     current = QMakeShared<Entry>();

                   } else if (current->groups) {
                     current->groups->clear();

                   }

                   initEntry();
                 }
   }

<FindMembers>"["        {
                 if (! insideCSharp && (current->name.isEmpty() ||  current->name == "typedef"))  {
                    // IDL function property
                   squareCount=1;
                   lastSquareContext = YY_START;
                   idlAttr.resize(0);
                   idlProp.resize(0);
                   current->mtype = mtype;

                   if (Config::getBool("idl-support") && current->mtype == Property) {
					       // inside the properties section of a dispinterface
					      odlProp = true;
					      current->m_specFlags.spec |= Entry::Gettable;
					      current->m_specFlags.spec |= Entry::Settable;
					    }

                   BEGIN( IDLAttribute );

                 } else if (insideCSharp && current->name.isEmpty()) {
                                          
                     squareCount=1;
                     lastSquareContext = YY_START;
                     // Skip the C# attribute for this member
                     current->args.resize(0);
                     BEGIN( SkipSquare );

                 } else {
                   current->args += yytext ;
                   squareCount=1;
                   BEGIN( Array ) ;
                 }
   }

<IDLAttribute>"]"          {
                    // end of IDL function attribute
                 if (--squareCount<=0)
                 {
                   lineCount();
                   if (current->mtype == MethodTypes::Property)
                     BEGIN( IDLPropName );
                   else
                     BEGIN( lastSquareContext );
                 }
   }

<IDLAttribute>"propput"                    {
                 if (Config::getBool("idl-support")) {
                   current->mtype = MethodTypes::Property;
                 }
                 current->m_specFlags.spec |= Entry::Settable;
   }

<IDLAttribute>"propget"       {
                 if (Config::getBool("idl-support")) {
                   current->mtype = MethodTypes::Property;
                 }
                 current->m_specFlags.spec |= Entry::Gettable;
   }

<IDLAttribute>"property" { 
      // UNO IDL property
      current->m_specFlags.spec |= Entry::Property;
   }

<IDLAttribute>"attribute" { 
      // UNO IDL attribute
      current->m_specFlags.spec |= Entry::Attribute;
   }

<IDLAttribute>"optional" { 
      // on UNO IDL interface/service/attribute/property
      current->m_specFlags.spec |= Entry::Optional;
   }

<IDLAttribute>"readonly" { 
      // on UNO IDL attribute or property
                 if (Config::getBool("idl-support") && odlProp) {
					    current->m_specFlags.spec ^= Entry::Settable;
					  } else {
					   current->m_specFlags.spec |= Entry::Readonly;
					  }                 
   }

<IDLAttribute>"bound" { 
      // on UNO IDL attribute or property
      current->m_specFlags.spec |= Entry::Bound;
   }

<IDLAttribute>"removable" { 
      // on UNO IDL property
      current->m_specFlags.spec |= Entry::Removable;
   }

<IDLAttribute>"constrained" { 
      // on UNO IDL property
      current->m_specFlags.spec |= Entry::Constrained;
   }

<IDLAttribute>"transient" { 
      // on UNO IDL property
      current->m_specFlags.spec |= Entry::Transient;
   }

<IDLAttribute>"maybevoid" { 
      // on UNO IDL property
      current->m_specFlags.spec |= Entry::MaybeVoid;
   }

<IDLAttribute>"maybedefault" { 
      // on UNO IDL property
      current->m_specFlags.spec |= Entry::MaybeDefault;
   }

<IDLAttribute>"maybeambiguous" { 
      // on UNO IDL property
      current->m_specFlags.spec |= Entry::MaybeAmbiguous;
   }

<IDLAttribute>.                 {
   }

<IDLPropName>{BN}*{ID}{BN}*      {
                 // return type (probably HRESULT) - skip it

                 if (odlProp) {
                   // property type
					    idlProp = yytext;
					  }
   }

<IDLPropName>{ID}{BN}*"("     {
                 current->name = yytext;
                 current->name = current->name.left(current->name.length()-1).trimmed();
                 current->startLine   = yyLineNr;
                 current->startColumn = yyColNr;
                 BEGIN( IDLProp );
   }

<IDLPropName>{BN}*"("{BN}*{ID}{BN}*")"{BN}*	{
					   if (odlProp) {
					     idlProp += yytext;
					   }
   }

<IDLPropName>{ID}{BN}*/";"		{
					   if (odlProp) {
					     current->name = yytext;
					     idlProp = idlProp.trimmed();
					     odlProp = false;

					     BEGIN( IDLProp );
					   }
   }

<IDLProp>{BN}*"["[^\]]*"]"{BN}*     {  
      // attribute of a parameter
                  idlAttr = yytext;
                  idlAttr=idlAttr.trimmed();
   }

<IDLProp>{ID}                       {  
      // property type
                  idlProp = yytext;
   }

<IDLProp>{BN}*{ID}{BN}*","          {  
      // Rare: Another parameter ([propput] HRESULT Item(int index, [in] Type theRealProperty);)
                 if (current->args.isEmpty())
                   current->args = "(";
                 else
                   current->args += ", ";
                 current->args += idlAttr;
                 current->args += " ";
                 current->args += idlProp;   // prop was actually type of extra parameter
                 current->args += " ";
                 current->args += yytext;
                 current->args = current->args.left(current->args.length() - 1);    // strip comma
                 idlProp.resize(0);
                 idlAttr.resize(0);
                 BEGIN( IDLProp );
   }

<IDLProp>{BN}*{ID}{BN}*")"{BN}*     {
                 // the parameter name for the property - just skip.
   }

<IDLProp>";"            {
                 current->fileName   = yyFileName;
                 current->type       = idlProp;
                    current->args       = current->args.simplified();

                    if (! current->args.isEmpty())
                      current->args += ")";

                    current->name       = current->name.trimmed();
                 current->section    = Entry::VARIABLE_SEC;
                 current_root->addSubEntry(current, current_root);

                 current = QMakeShared<Entry>();
                 initEntry();
                 BEGIN( FindMembers );
   }

<IDLProp>.           { 
      // spaces, *, or other stuff
      //idlProp+=yytext;
   }

<Array>"]"                     { current->args += *yytext ;
                 if (--squareCount<=0)
                                        BEGIN( FindMembers ) ;
   }

<FuncFuncArray>"]"              { current->args += *yytext ;
                 if (--squareCount<=0)
                                        BEGIN( Function ) ;
   }

<Array,FuncFuncArray>"["      { current->args += *yytext ;
                 squareCount++;
   }

<Array,FuncFuncArray>.        { current->args += *yytext ; }
<SkipSquare>"["            { squareCount++; }
<SkipSquare>"]"            {
                 if (--squareCount<=0)
                 BEGIN( lastSquareContext );
   }

<SkipSquare>\"             {
                 lastStringContext=YY_START;
                 BEGIN( SkipString );
   }

<SkipSquare>[^\n\[\]\"]+
<FindMembers>"<"        { addType( current ) ;
                 current->type += yytext ;
                 BEGIN( Sharp ) ;
   }

<Sharp>">"                           { 
                 current->type += *yytext ;
                 if (--sharpCount<=0)
                                        BEGIN( FindMembers ) ;
   }

<Sharp>"<"           { current->type += *yytext ;
                 sharpCount++;
   }

<Sharp>{BN}+                        {
                    current->type += ' ';
                    lineCount();
   }

<Sharp>.             { current->type += *yytext ; }
<FindFields>{ID}                    {
                   if (insideCpp || insideObjC)  {
                      current->id = ClangParser::instance()->lookup(yyLineNr,yytext);
                   }
                   current->bodyLine = yyLineNr;
                   current->name     = yytext;
   }

<FindFields>"("            {
                    // Java enum initializer
                    unput('(');
                    lastInitializerContext = YY_START;
                 initBracketCount=0;
                                          current->initializer = "=";
                    BEGIN(ReadInitializer);
   }

<FindFields>"="            {
                    lastInitializerContext = YY_START;
                 initBracketCount=0;
                                          current->initializer = yytext;
                    BEGIN(ReadInitializer);
   }

<FindFields>";"                         {
                    if (insideJava)  // last enum field in Java class
                 {
                   if (!current->name.isEmpty())
                   {
                     current->fileName   = yyFileName;
                     current->startLine  = yyLineNr;
                     current->startColumn = yyColNr;
                     current->type       = "@"; // enum marker
                     current->args       = current->args.simplified();
                     current->name       = current->name.trimmed();
                     current->section    = Entry::VARIABLE_SEC;
                     current_root->addSubEntry(current, current_root);
                     current = QMakeShared<Entry>();
                     initEntry();
                   }

                   BEGIN( FindMembers );
                 }
                 else
                 {
                   REJECT;
                 }
   }

<SkipRemainder>\n     {
      lineCount();
   }

<SkipRemainder>[^\n]*
<FindFields>","       {
                   if (! current->name.isEmpty()) {

                      current->fileName    = yyFileName;
                      current->startLine   = yyLineNr;
                      current->startColumn = yyColNr;

                      if (! (current_root->m_specFlags.spec & Entry::Enum)) {
                         // enum marker (this might be Java-style enums
                         current->type     = "@";
                      }

                      current->args    = current->args.simplified();
                      current->name    = current->name.trimmed();
                      current->section = Entry::VARIABLE_SEC;

                      // add enum value to the enum data type
                      current_root->addSubEntry(current, current_root);

                     if (! insideCSharp && ! insideJava && ! (current_root->m_specFlags.spec & Entry::Strong))  {
                                  // for C# and Java 1.5+ enum values always have to be explicitly qualified,
                                  // same for C++11 style enums (enum class Name {})

                        current = QMakeShared<Entry>(*current);

                       // add enum value to the enum data type parent (usually the class the enum belongs to)
                       current_root->parent()->addSubEntry(current, current_root->parent());
                     }

                     current = QMakeShared<Entry>();
                     initEntry();

                 }  else  {
                   // probably a redundant ","
                   current->reset();
                   initEntry();
                 }
   }

<FindFields>"["            { // attribute list in IDL
                 squareCount=1;
                 lastSquareContext = YY_START;
                 BEGIN(SkipSquare);
   }

  /*
<FindFieldArg>","          { 
      unput(*yytext); 
      BEGIN(FindFields); 
   }
  */

<ReadBody,ReadNSBody,ReadBodyIntf>[^\r\n\#{}"@'/<]*   { 
      current->program += yytext; 
   }

<ReadBody,ReadNSBody,ReadBodyIntf>"//".*     { 
      current->program += yytext; 
   }

<ReadBody,ReadNSBody,ReadBodyIntf>"#".*   { 
   if (! insidePHP)
                   REJECT;
                 // append PHP comment.
                 current->program += yytext ;
   }

<ReadBody,ReadNSBody,ReadBodyIntf>@\"  { 
      current->program += yytext ;
                                          pSkipVerbString = &current->program;
                                          lastSkipVerbStringContext=YY_START;
                                          BEGIN( SkipVerbString );
   }

<ReadBody,ReadNSBody,ReadBodyIntf>"<<<"   { 
   if (insidePHP) {
                                            current->program += yytext ;
                                            pCopyHereDocGString = &current->program;
                                            lastHereDocContext=YY_START;
                                            BEGIN( CopyHereDoc );
                                          }
                                          else
                                          {
                                            REJECT;
                                          }
   }

<ReadBody,ReadNSBody,ReadBodyIntf>\"   { 
   current->program += yytext ;
                                          pCopyQuotedGString = &current->program;
                                          lastStringContext=YY_START;
                                          BEGIN( CopyGString );
   }

<ReadBody,ReadNSBody,ReadBodyIntf>"/*"{B}*      { 
      // */ (editor syntax fix)
      current->program += yytext;
      lastContext = YY_START;
      BEGIN( Comment );
   }

<ReadBody,ReadNSBody,ReadBodyIntf>"/*"{BL}      { 
      // */ (editor syntax fix)
      current->program += yytext ;
      ++yyLineNr ;
      lastContext = YY_START ;
      BEGIN( Comment ) ;
   }

<ReadBody,ReadNSBody,ReadBodyIntf>"'"  {
                    if (!insidePHP)
                 {
                   current->program += yytext;
                 }
                 else
                 { // begin of single quoted string
                   current->program += yytext;
                                            pCopyQuotedGString = &current->program;
                                            lastStringContext=YY_START;
                   BEGIN(CopyPHPGString);
                 }
   }

<ReadBody,ReadNSBody,ReadBodyIntf>{CHARLIT} {
                                              if (insidePHP)
                     {
                  REJECT; // for PHP code single quotes
                               // are used for strings of arbitrary length
                     }
                     else
                     {
                                                current->program += yytext;
                     }
   }

<ReadBody,ReadNSBody,ReadBodyIntf>"{"   { 
      current->program += yytext ;
                 ++curlyCount ;
   }

<ReadBodyIntf>"}"          {
                 current->program += yytext ;
                 --curlyCount ;
   }

<ReadBody,ReadNSBody>"}"      { 
      // err("ReadBody count=%d\n",curlyCount);
                    if ( curlyCount>0 )
                 {
                   current->program += yytext ;
                   --curlyCount ;
                 }
                 else
                 {
                   current->endBodyLine = yyLineNr;
                   QString &cn = current->name;
                   QString rn = current_root->name;

                   if (!cn.isEmpty() && !rn.isEmpty())
                   {
                     prependScope();
                   }
                   if (isTypedef && cn.isEmpty())
                   {
                     //printf("Typedef Name\n");
                     BEGIN( TypedefName );
                   }
                   else
                   {
                     if ((current->section == Entry::ENUM_SEC) || (current->m_specFlags.spec & Entry::Enum))
                     {
                       current->program+=','; // add field terminator
                     }
                     // add compound definition to the tree
                     current->args=removeRedundantWhiteSpace(current->args);
                               // was: current->args.simplified();
                        current->type = current->type.simplified();
                        current->name = current->name.trimmed();
                     //printf("adding `%s' `%s' `%s' brief=%s insideObjC=%d %x\n",current->type.data(),current->name.data(),current->args.data(),current->brief.data(),insideObjC,current->section);
                     if (insideObjC &&
                    ((current->m_specFlags.spec & Entry::Interface) || (current->m_specFlags.spec == Entry::Category))
                   ) // method definition follows
                     {
                            BEGIN( ReadBodyIntf ) ;
                     }
                     else
                     {
                       current_root->addSubEntry( current , current_root);
                       memspecEntry = current;
                  current = QMakeShared<Entry>(*current);
                  if (current->section == Entry::NAMESPACE_SEC || (current->m_specFlags.spec == Entry::Interface) ||
                      insideJava || insidePHP || insideCSharp || insideD || insideJS
                     )
                  { // namespaces and interfaces and java classes ends with a closing bracket without semicolon
                    current->reset();
                    initEntry();

                    memspecEntry = QSharedPointer<Entry>();

                    BEGIN( FindMembers ) ;
                  }
                  else
                  {
                    static QRegExp re("@[0-9]+$");
                    if (! isTypedef && memspecEntry &&
                                re.indexIn(memspecEntry->name) == -1)
                    // not typedef or anonymous type (see bug691071)
                    {
                      // enabled the next two lines for bug 623424
                      current->doc.resize(0);
                      current->brief.resize(0);
                    }
                    BEGIN( MemberSpec ) ;
                  }
                     }
                   }
                 }
   }

<ReadBody>"}"{BN}+"typedef"{BN}+    { 
      // err("ReadBody count=%d\n",curlyCount);
                 lineCount();
                    if ( curlyCount>0 )
                 {
                   current->program += yytext ;
                   --curlyCount ;
                 }
                 else
                 {
                   isTypedef = true;
                   current->endBodyLine = yyLineNr;
                   QString &cn = current->name;
                   QString rn = current_root->name;
                   if (!cn.isEmpty() && !rn.isEmpty())
                   {
                     prependScope();
                   }
                   BEGIN( TypedefName );
                 }
   }

<TypedefName>("const"|"volatile"){BN}  { 
      // late "const" or "volatile" keyword
      lineCount();
      current->type.prepend(yytext);
   }

<TypedefName>{ID}          {
                 if ((current->section == Entry::ENUM_SEC) || (current->m_specFlags.spec & Entry::Enum))
                 {
                   current->program += ","; // add field terminator
                 }
                         current->name=yytext;
                 prependScope();
                    current->args = current->args.simplified();
                    current->type = current->type.simplified();
                 //printf("Adding compound %s %s %s\n",current->type.data(),current->name.data(),current->args.data());
                 current_root->addSubEntry( current , current_root);
                 if (!firstTypedefEntry)
                 {
                   firstTypedefEntry = current;
                 }
                 current = QMakeShared<Entry>();
                 initEntry();
                 isTypedef=true; // to undo reset by initEntry()
                    BEGIN(MemberSpecSkip);
   }

<TypedefName>";"        { 
      /* typedef of anonymous type */
                 current->name = QString("@%1").arg(anonCount++);

                 if ((current->section == Entry::ENUM_SEC) || (current->m_specFlags.spec & Entry::Enum))
                 {
                   current->program+=','; // add field terminator
                 }
                 // add compound definition to the tree
                    current->args = current->args.simplified();
                    current->type = current->type.simplified();
                 current_root->addSubEntry( current , current_root);
                 memspecEntry = current;
                 current = QMakeShared<Entry>(*current);
                 initEntry();
                 unput(';');
                 BEGIN( MemberSpec ) ;
   }

<MemberSpec>([*&]*{BN}*)*{ID}{BN}*("["[^\]\n]*"]")* { 
      // the [] part could be improved.
                    lineCount();
                    int i=0,l=(int)yyleng,j;
                 while (i<l && (!isId(yytext[i]))) i++;
                 msName = QString(yytext).right(l-i).trimmed();
                 j=msName.indexOf("[");
                 if (j!=-1)
                 {
                   msArgs=msName.right(msName.length()-j);
                   msName=msName.left(j);
                 }
                 msType=QString(yytext).left(i);

                 // handle *pName in: typedef { ... } name, *pName;
                 if (firstTypedefEntry)
                 {
                   if (firstTypedefEntry->m_specFlags.spec & Entry::Struct)
                   {
                     msType.prepend("struct "+firstTypedefEntry->name);
                   }
                   else if (firstTypedefEntry->m_specFlags.spec & Entry::Union)
                   {
                     msType.prepend("union "+firstTypedefEntry->name);
                   }
                   else if (firstTypedefEntry->section==Entry::ENUM_SEC)
                   {
                     msType.prepend("enum "+firstTypedefEntry->name);
                   }
                   else
                   {
                     msType.prepend(firstTypedefEntry->name);
                   }
                 }
   }

<MemberSpec>"("            { 
      // function with struct return type
                 addType(current);
                 current->name = msName;
                 current->m_specFlags = Entry::SpecifierFlags{};

                 unput('(');
                 BEGIN(FindMembers);
   }

<MemberSpec>[,;]        {
                    if (msName.isEmpty() && !current->name.isEmpty())
                 {
                   // see if the compound does not have a name or is inside another
                   // anonymous compound. If so we insert a
                   // special `anonymous' variable.
                   // Entry *p=current_root;
                   // Entry *p=current;

                   QSharedPointer<Entry> p = current;

                   while (p)
                   {
                     // only look for class scopes, not namespace scopes
                     if ((p->section & Entry::COMPOUND_MASK) && !p->name.isEmpty())
                     {
                  //printf("Trying scope `%s'\n",p->name.data());
                  int i=p->name.lastIndexOf("::");
                  int pi = (i==-1) ? 0 : i+2;
                  if (p->name.at(pi)=='@')
                  {
                    // anonymous compound inside -> insert dummy variable name
                    //printf("Adding anonymous variable for scope %s\n",p->name.data());

                    msName = QString("@%1").arg(anonCount++);
                    break;
                  }
                     }
                     //p=p->parent;
                     if (p==current) p=current_root; else p=p->parent();
                   }
                 }
                 //printf("msName=%s current->name=%s\n",msName.data(),current->name.data());
                 if (!msName.isEmpty()
                     /*&& msName!=current->name*/) // skip typedef T {} T;, removed due to bug608493
                 {
                   static bool typedefHidesStruct =  Config::getBool("use-typedef-name");

                   // case 1: typedef struct _S { ... } S_t;
                   // -> omit typedef and use S_t as the struct name

                   if (typedefHidesStruct &&  isTypedef &&
                     ((current->m_specFlags.spec & (Entry::Struct|Entry::Union)) ||
                      current->section == Entry::ENUM_SEC ) && msType.trimmed().isEmpty() && memspecEntry)
                   {
                     memspecEntry->name=msName;
                   }
                   else // case 2: create a typedef field
                   {
                     QSharedPointer<Entry> varEntry = QMakeShared<Entry>();
                     varEntry->lang = language;
                     varEntry->protection = current->protection ;
                     varEntry->mtype = current->mtype;
                     varEntry->virt = current->virt;
                     varEntry->stat = current->stat;
                     varEntry->section = Entry::VARIABLE_SEC;
                     varEntry->name = msName.trimmed();
                     varEntry->type = current->type.simplified()+" ";
                     varEntry->args = msArgs;
                     if (isTypedef)
                     {
                  varEntry->type.prepend("typedef ");
                  //  //printf("current->name = %s %s\n",current->name.data(),msName.data());
                     }
                     if (typedefHidesStruct && isTypedef && (current->m_specFlags.spec & (Entry::Struct|Entry::Union)) && memspecEntry)
                     // case 1: use S_t as type for pS_t in "typedef struct _S {} S_t, *pS_t;"
                     {
                        varEntry->type+=memspecEntry->name+msType;
                     }
                     else // case 2: use _S as type for for pS_t
                     {
                  varEntry->type+=current->name+msType;
                     }
                     varEntry->fileName = yyFileName;
                     varEntry->startLine = yyLineNr;
                     varEntry->startColumn = yyColNr;
                     varEntry->doc = current->doc;
                     varEntry->brief = current->brief;
                     varEntry->mGrpId = current->mGrpId;
                     varEntry->initializer = current->initializer;

                     // deep copy group list
                     for (auto g : *current->groups) {
                        varEntry->groups->append(g);
                     }

                     if (current->sli) { // copy special list items
                        for (auto lii : *current->sli) {
                           varEntry->addSpecialListItem(lii.type, lii.itemId);
                        }
                     }

                     current_root->addSubEntry(varEntry, current_root);
                   }
                 }
                 if (*yytext==';') // end of a struct/class ...
                 {
                   if (!isTypedef && msName.isEmpty() && memspecEntry && (current->section&Entry::COMPOUND_MASK))
                   { // case where a class/struct has a doc block after it
                     if (!current->doc.isEmpty())
                     {
                       memspecEntry->doc += current->doc;
                     }
                     if (!current->brief.isEmpty())
                     {
                       memspecEntry->brief += current->brief;
                     }
                      }
                   msType.resize(0);
                   msName.resize(0);
                   msArgs.resize(0);
                   isTypedef=false;

                   firstTypedefEntry = QSharedPointer<Entry>();
                   memspecEntry = QSharedPointer<Entry>();

                   current->reset();
                   initEntry();
                   BEGIN( FindMembers );
                 }
                 else
                 {
                   current->doc.resize(0);
                   current->brief.resize(0);
                 }

    }

<MemberSpec>"="            {
                    lastInitializerContext=YY_START;
                 initBracketCount=0;
                                          current->initializer = yytext;
                    BEGIN(ReadInitializer);
                    /* BEGIN(MemberSpecSkip); */
   }

  /*
<MemberSpecSkip>"{"        {
                 curlyCount=0;
                 lastCurlyContext = MemberSpecSkip;
                 previous = current;
                  BEGIN(SkipCurly);
   }
  */

<MemberSpecSkip>","        { 
      BEGIN(MemberSpec); 
   }

<MemberSpecSkip>";"             { 
      unput(';'); 
      BEGIN(MemberSpec); 
   }

<ReadBody,ReadNSBody,ReadBodyIntf>{BN}{1,80} { 
      current->program += yytext ;
      lineCount() ;
   }

<ReadBodyIntf>"@end"/[^a-z_A-Z0-9]  { 
      // end of Objective C block
      current_root->addSubEntry( current, current_root);
      current = QMakeShared<Entry>();
      initEntry();
      insideObjC=false;
      BEGIN( FindMembers );
   }

<ReadBody,ReadNSBody,ReadBodyIntf>.    { 
      current->program += yytext ; 
   }

<FindMembers>"("/{BN}*"::"*{BN}*({TSCOPE}{BN}*"::")*{TSCOPE}{BN}*")"{BN}*"(" | /* typedef void (A<int>::func_t)(args...) */
<FindMembers>("("({BN}*"::"*{BN}*{TSCOPE}{BN}*"::")*({BN}*[*&\^]{BN}*)+)+ {   
      /* typedef void (A::*ptr_t)(args...) or int (*func(int))[], the ^ is for Obj-C blocks */
      if (insidePHP) // reference parameter
                 {
                   REJECT
                 }
                 else
                 {
                   current->bodyLine = yyLineNr;
                   lineCount();
                   addType(current);
                   funcPtrType=yytext;
                   roundCount=0;
                   //current->type += yytext;
                   BEGIN( FuncPtr );
                 }
   }

<FuncPtr>{SCOPENAME}          {
                    current->name = yytext;
                 if (nameIsOperator(current->name))
                 {
                   BEGIN( FuncPtrOperator );
                 }
                 else
                 {
                   if (current->name=="const" || current->name=="volatile")
                   {
                     funcPtrType += current->name;
                   }
                   else
                   {
                     BEGIN( EndFuncPtr );
                   }
                 }
   }

<FuncPtr>.           {
      // printf("error: FuncPtr `%c' unexpected at line %d of %s\n",*yytext,yyLineNr,yyFileName);
   }

<FuncPtrOperator>"("{BN}*")"{BN}*/"("  {
                 current->name += yytext;
                 current->name = current->name.simplified();
                 lineCount();
   }

<FuncPtrOperator>\n        {
                    lineCount();
                    current->name += *yytext;
   }

<FuncPtrOperator>"("          {
                 unput(*yytext);
                 BEGIN( EndFuncPtr );
   }

<FuncPtrOperator>.         {
                    current->name += *yytext;
   }

<EndFuncPtr>")"{BN}*/";"      { 
      // a variable with extra braces
                 lineCount();
                 current->type += funcPtrType.mid(1);
                 BEGIN(FindMembers);
                  }
<EndFuncPtr>")"{BN}*/"("      { 
      // a function pointer
                 lineCount();
                 current->type += funcPtrType + ")";
                 BEGIN(FindMembers);
   }

<EndFuncPtr>")"{BN}*/"["      { 
      // an array of variables
                    lineCount();
                 current->type += funcPtrType;
                 current->args += ")";
                 BEGIN(FindMembers);
   }

<EndFuncPtr>"("            { 
      // a function returning a function or
      // a function returning a pointer to an array

      current->args += *yytext ;

      //roundCount=0;
      //BEGIN( FuncFunc );

      current->bodyLine = yyLineNr;
      currentArgumentContext = FuncFuncEnd;
      fullArgString=current->args;
      copyArgString=&current->args;
      BEGIN( ReadFuncArgType );
   }

<EndFuncPtr>"["[^\n\]]*"]"       {
                    funcPtrType+=yytext;
   }

<EndFuncPtr>")"            {
                    BEGIN(FindMembers);
   }

<FuncFunc>"("           {
                    current->args += *yytext ;
                    ++roundCount;
   }

<FuncFunc>")"           {
                    current->args += *yytext ;
                    if ( roundCount )
                   --roundCount;
                 else
                 {
                   BEGIN(FuncFuncEnd);
                 }
   }

<FuncFuncEnd>")"{BN}*"("      {
                    lineCount();
                 current->type += funcPtrType + ")(";
                 BEGIN(FuncFuncType);
   }

<FuncFuncEnd>")"{BN}*/[;{]       {
                    lineCount();
                 current->type += funcPtrType.mid(1);
                    BEGIN(Function);
   }

<FuncFuncEnd>")"{BN}*/"["     { // function returning a pointer to an array
                 lineCount();
                 current->type+=funcPtrType;
                 current->args+=")";
                 BEGIN(FuncFuncArray);
   }

<FuncFuncEnd>.             {
                    current->args += *yytext;
   }

<FuncFuncType>"("          {
                    current->type += *yytext;
                 roundCount++;
   }

<FuncFuncType>")"          {
                    current->type += *yytext;
                    if (roundCount)
                   --roundCount;
                 else
                   BEGIN(Function);
   }

<FuncFuncType>{BN}*","{BN}*      { 
      lineCount();
      current->type += ", " ; 
   }

<FuncFuncType>{BN}+        { 
      lineCount();
      current->type += ' ' ; 
   }

<FuncFuncType>.            {
      current->type += *yytext;
   }

<FindMembers>"("/{BN}*{ID}{BN}*"*"{BN}*{ID}*")(" { 
      // for catching typedef void (__stdcall *f)() like definitions

      if (current->type.left(7) == "typedef" && current->bodyLine == -1) {
         // the bodyLine check is to prevent this guard to be true more than once
         
         current->bodyLine = yyLineNr;
         BEGIN( GetCallType );

       } else if (!current->name.isEmpty())  {
         // normal function
         
         current->args = yytext;
         current->bodyLine = yyLineNr;
         currentArgumentContext = FuncQual;
         fullArgString=current->args;
         copyArgString=&current->args;
         BEGIN( ReadFuncArgType );         
      }
   }

<GetCallType>{BN}*{ID}{BN}*"*"      {
      lineCount();
      addType(current);
      funcPtrType="(";
      funcPtrType+=yytext;
      roundCount=0;
      BEGIN( FuncPtr );
   }

<FindMembers>"("        {
                 if (! current->name.isEmpty())
                 {
                   current->args = yytext;
                   current->bodyLine = yyLineNr;
                   currentArgumentContext = FuncQual;
                   fullArgString=current->args;
                   copyArgString=&current->args;
                   BEGIN( ReadFuncArgType ) ;
                   //printf(">>> Read function arguments current->argListcount()=%d\n",current->argListcount());
                 }
   }


  /*
<FindMembers>"("{BN}*("void"{BN}*)?")"    {     
      lineCount();
      current->args = "()";
      BEGIN( FuncQual );
   }
  */

  /* Function argument reading rules */

<ReadFuncArgType>[^ \/\r\t\n\)\(\"\'#]+ {       
      *copyArgString += yytext;
      fullArgString  += yytext;
   }

<CopyArgString,CopyArgPHPString>[^\n\\\"\']+       { 
      *copyArgString += yytext;
      fullArgString  += yytext;
   }

<CopyArgRound>[^\/\n\)\(\"\']+      {
      *copyArgString += yytext;
      fullArgString  += yytext;
   }

<ReadFuncArgType,ReadTempArgs>{BN}*    {
                    *copyArgString+=" ";
                    fullArgString+=" ";
                    lineCount();
   }

<ReadFuncArgType,CopyArgRound,CopyArgSharp,ReadTempArgs>{RAWBEGIN}   {
                 g_delimiter = yytext+2;
                 g_delimiter=g_delimiter.left(g_delimiter.length()-1);
                 lastRawStringContext = YY_START;
                 pCopyRawString = copyArgString;
                 *pCopyRawString+=yytext;
                 fullArgString+=yytext;
                 BEGIN(RawString);
   }

<ReadFuncArgType,CopyArgRound,CopyArgSharp,ReadTempArgs>\"  {
                    *copyArgString+=*yytext;
                    fullArgString+=*yytext;
                 lastCopyArgStringContext = YY_START;
                    BEGIN(CopyArgString);
   }

<ReadFuncArgType,ReadTempArgs>"("   {
                    *copyArgString+=*yytext;
                    fullArgString+=*yytext;
                    argRoundCount=0;
                 lastCopyArgContext = YY_START;
                 BEGIN(CopyArgRound);
   }

<ReadFuncArgType>")"          {
                    *copyArgString+=*yytext;
                    fullArgString+=*yytext;
                 stringToArgumentList(fullArgString, &current->argList);

                 if (insideJS) {
                   fixArgumentListForJavaScript(&current->argList);
                 }

                 handleParametersCommentBlocks(&current->argList);

                 // save the current documentation block
                 docBackup   = current->doc;
                 briefBackup = current->brief;

                 // go back to the saved state
                 BEGIN(currentArgumentContext);
    }

   /* a special comment */
<ReadFuncArgType,ReadTempArgs>("/*"[*!]|"//"[/!])("<"?)  {
      // */ (editor syntax fix)

      if (currentArgumentContext==DefineEnd) {
                 
                   // for defines we interpret a comment
                   // as documentation for the define
                   int i;for (i=(int)yyleng-1;i>=0;i--)
                   {
                     unput(yytext[i]);
                   }
                   stringToArgumentList(fullArgString, &current->argList);
                                            handleParametersCommentBlocks(&current->argList);
                   BEGIN(currentArgumentContext);
                 }
                 else // not a define
                 {
                   // for functions we interpret a comment
                   // as documentation for the argument
                   fullArgString+=yytext;
                   lastCopyArgChar=0;
                   lastCommentInArgContext=YY_START;
                   if (yytext[1]=='/')
                     BEGIN(CopyArgCommentLine);
                   else
                     BEGIN(CopyArgComment);
                 }
   }

   /* a non-special comment */
<ReadFuncArgType,ReadTempArgs>"/**/"   { 
      /* empty comment */ 
   }
<ReadFuncArgType,ReadTempArgs>"/*"  {
                    lastCContext = YY_START;
                 BEGIN( SkipComment );
   }

<ReadFuncArgType,ReadTempArgs>"//"  {
                    lastCContext = YY_START;
                 BEGIN( SkipCxxComment );
   }

  /*
<ReadFuncArgType,ReadTempArgs>"'#"  { 
      if (insidePHP)
         REJECT;
      *copyArgString  += yytext;
      fullArgString   += yytext;
   }

<ReadFuncArgType,ReadTempArgs>"#"   {
      if (!insidePHP)
         REJECT;
      lastCContext = YY_START;
      BEGIN( SkipCxxComment );
   }
  */

   /* `)' followed by a special comment */
<ReadFuncArgType>")"{BN}*("/*"[*!]|"//"[/!])"<"    {
      // */ (editor syntax fix)
      lineCount();
       if (currentArgumentContext==DefineEnd) {
                 
                   // for defines we interpret a comment
                   // as documentation for the define
                   int i;for (i=(int)yyleng-1;i>0;i--)
                   {
                     unput(yytext[i]);
                   }
                   *copyArgString+=*yytext;
                   fullArgString+=*yytext;
                   stringToArgumentList(fullArgString, &current->argList);
                                            handleParametersCommentBlocks(&current->argList);
                   BEGIN( currentArgumentContext );
                 }
                 else
                 {
                   // for functions we interpret a comment
                   // as documentation for the last argument
                   lastCopyArgChar=*yytext;
                   QString text=&yytext[1];
                   text=text.trimmed();
                   lastCommentInArgContext=YY_START;
                   fullArgString+=text;
                   if (text.indexOf("//")!=-1)
                     BEGIN( CopyArgCommentLine );
                   else
                     BEGIN( CopyArgComment );
                 }
   }

<CopyArgComment>^{B}*"*"+/{BN}+
<CopyArgComment>[^\n\\\@\*]+     { 
      fullArgString+=yytext; 
   }

<CopyArgComment>"*/"          { 
      fullArgString+=yytext;
      if (lastCopyArgChar != 0)
         unput(lastCopyArgChar);
      BEGIN( lastCommentInArgContext );
   }

<CopyArgCommentLine>\n        { 
      fullArgString+=yytext;
      lineCount();
                    if (lastCopyArgChar!=0)
                   unput(lastCopyArgChar);
                 BEGIN( lastCommentInArgContext );
   }

<CopyArgCommentLine>{CMD}("verbatim"|"latexonly"|"htmlonly"|"xmlonly"|"manonly"|"dot"|"code")/[^a-z_A-Z0-9]    { 
      // verbatim command (which could contain nested comments!)
      docBlockName=&yytext[1];
      fullArgString+=yytext;
      BEGIN(CopyArgVerbatim);
   }

<CopyArgCommentLine>{CMD}("f$"|"f["|"f{")            {
                      docBlockName=&yytext[1];
                 if (docBlockName.at(1)=='[')
                 {
                        docBlockName[1] ='}';
                 }
                 if (docBlockName.at(1)=='{')
                 {
                        docBlockName[1] ='}';
                 }
                    fullArgString+=yytext;
                    BEGIN(CopyArgVerbatim);
   }

<CopyArgVerbatim>[\\@]("endverbatim"|"endlatexonly"|"endhtmlonly"|"endxmlonly"|"enddocbookonly"|"endmanonly"|"enddot"|"endcode"|"f$"|"f]"|"f}")/[^a-z_A-Z0-9] { 
      // end of verbatim block
      fullArgString+=yytext;

      if (yytext[1] == 'f') // end of formula
                      {
                        BEGIN(CopyArgCommentLine);
                      }
                 if (&yytext[4]==docBlockName)
                 {
                      BEGIN(CopyArgCommentLine);
                 }
   }

<CopyArgCommentLine>[^\\\@\n]+      { 
      fullArgString+=yytext; 
   }

<CopyArgCommentLine>.         { 
      fullArgString += *yytext; 
   }
<CopyArgComment,CopyArgVerbatim>\n  { 
      fullArgString+=*yytext; 
      lineCount(); 
   }

<CopyArgComment,CopyArgVerbatim>.   { 
      fullArgString+=*yytext; 
   }


<CopyArgComment>{CMD}("brief"|"short"){B}+ {
      warn(yyFileName,yyLineNr, "Ignoring %c brief command inside argument documentation",*yytext);
      fullArgString+=' ';
   }

<ReadTempArgs>"<"          {
                 *copyArgString+=*yytext;
                 fullArgString+=*yytext;
                 argSharpCount=1;
                 BEGIN( CopyArgSharp );
   }

<ReadTempArgs>">"          {
                 *copyArgString+=*yytext;
                 fullArgString+=*yytext;
                 //printf("end template list %s\n",copyArgString->data());
                 stringToArgumentList(fullArgString,currentArgumentList);
                 BEGIN( currentArgumentContext );
   }

<CopyArgRound>"("          {
                    argRoundCount++;
                 *copyArgString+=*yytext;
                 fullArgString+=*yytext;
   }

<CopyArgRound>")"          {
                 *copyArgString+=*yytext;
                 fullArgString+=*yytext;
                 if (argRoundCount>0)
                   argRoundCount--;
                 else
                   BEGIN( lastCopyArgContext );
   }

<CopyArgSharp>"("                       {
                    *copyArgString+=*yytext;
                    fullArgString+=*yytext;
                    argRoundCount=0;
                 lastCopyArgContext = YY_START;
                 BEGIN( CopyArgRound );
   }

<CopyArgSharp>"<"          {
                    argSharpCount++;
                 //printf("argSharpCount++=%d  copy\n",argSharpCount);
                 *copyArgString+=*yytext;
                 fullArgString+=*yytext;
   }

<CopyArgSharp>">"          {
                    *copyArgString+=*yytext;
                    fullArgString+=*yytext;
                 argSharpCount--;
                 if (argSharpCount>0)
                 {
                   //printf("argSharpCount--=%d copy\n",argSharpCount);
                 }
                 else
                 {
                   BEGIN( ReadTempArgs );
                   //printf("end of argSharpCount\n");
                 }
   }

<CopyArgString,CopyArgPHPString>\\.    {
                    *copyArgString+=yytext;
                    fullArgString+=yytext;
   }

<CopyArgString>\"          {
                    *copyArgString+=*yytext;
                    fullArgString+=*yytext;
                 BEGIN( lastCopyArgStringContext );
   }

<CopyArgPHPString>\'          {
                    *copyArgString+=*yytext;
                    fullArgString+=*yytext;
                 BEGIN( lastCopyArgStringContext );
   }

<ReadFuncArgType,ReadTempArgs,CopyArgRound,CopyArgSharp>{CHARLIT}     {
                 if (insidePHP)
                 {
                   REJECT;
                 }
                 else
                 {
                      *copyArgString+=yytext;
                      fullArgString+=yytext;
                 }
   }

<ReadFuncArgType,ReadTempArgs,CopyArgRound,CopyArgSharp>\'     {
                    *copyArgString+=yytext;
                    fullArgString+=yytext;
                 if (insidePHP)
                 {
                   lastCopyArgStringContext=YY_START;
                   BEGIN(CopyArgPHPString);
                 }
   }

<ReadFuncArgType,ReadTempArgs,CopyArgString,CopyArgPHPString,CopyArgRound,CopyArgSharp>\n  {
                 lineCount();
                 *copyArgString+=*yytext;
                 fullArgString+=*yytext;
   }

<ReadFuncArgType,ReadTempArgs,CopyArgString,CopyArgPHPString,CopyArgRound,CopyArgSharp>.    {
                    *copyArgString+=*yytext;
                    fullArgString+=*yytext;
   }


<FuncRound>"("             { current->args += *yytext ;
                 ++roundCount ;
   }

<FuncRound>")"                          { current->args += *yytext ;
                 if ( roundCount )
                   --roundCount ;
                      else
                   BEGIN( FuncQual ) ;
   }

  /*
<FuncQual>"#"  { 
      if (insidePHP) {
         REJECT;
      }
      lastCPPContext = YY_START;
      BEGIN(SkipCPP);
   }

  */

<FuncQual>[{:;,]                        {
                      if ( qstrcmp(yytext,";") == 0 && insidePHP &&  ! containsWord(current->type,"function") )
                        {
                          current->reset();
                          initEntry();
                          BEGIN( FindMembers );
                        }
                        else
                        {
                          unput(*yytext);
                          BEGIN( Function );
                        }
   }

<FuncQual>{BN}*"abstract"{BN}*          { 
      // pure virtual member function
                                          lineCount() ;
                                          current->virt = Pure;
                                          current->args += " override ";
   }

<FuncQual,TrailingReturn>{BN}*"override"{BN}*          { 
      // C++11 overridden virtual member function
                                          lineCount() ;
                                          current->m_specFlags.spec |= Entry::Override;
                                          current->args += " override ";
                                          BEGIN(FuncQual);
   }

<FuncQual,TrailingReturn>{BN}*"final"{BN}*             { 
      // C++11 final method
                                          lineCount() ;
                                          current->m_specFlags.m_isFinal = true;
                                          current->args += " final ";
                                          BEGIN(FuncQual);
   }

<FuncQual>{BN}*"sealed"{BN}*            { 
      // sealed member function
                                          lineCount() ;
                                          current->m_specFlags.spec |= Entry::Sealed;
                                          current->args += " sealed ";
   }

<FuncQual>{BN}*"new"{BN}*               { 
      // new member function
                                          lineCount() ;
                                          current->m_specFlags.spec |= Entry::New;
                                          current->args += " new ";
   }

<FuncQual>{BN}*"const"{BN}*         { 
      // const member function
                 lineCount() ;
                 current->args += " const ";
                 current->argList.constSpecifier=true;
   }

<FuncQual>{BN}*"volatile"{BN}*      { 
      // volatile member function
                    lineCount() ;
                    current->args += " volatile ";
                 current->argList.volatileSpecifier=true;
   }

<FuncQual>{BN}*"noexcept"{BN}*      { 
      // noexcept qualifier
                    lineCount() ;
                 current->args += " noexcept ";
                 current->m_specFlags.spec |= Entry::NoExcept;
   }

<FuncQual>{BN}*"noexcept"{BN}*"("      { 
      // noexcept expression
                 lineCount() ;
                 current->args += " noexcept(";
                 current->m_specFlags.spec |= Entry::NoExcept;
                 lastRoundContext=FuncQual;
                 pCopyRoundString=&current->args;
                 roundCount=0;
                                          BEGIN(CopyRound);
   }

<FuncQual,TrailingReturn>{BN}*"="{BN}*"0"{BN}*     { 
      // pure virtual member function
                 lineCount() ;
                 current->args += " = 0";
                 current->virt = Pure;
                 current->argList.pureSpecifier = true;
                 BEGIN(FuncQual);
   }

<FuncQual,TrailingReturn>{BN}*"="{BN}*"delete"{BN}*     { 
      // C++11 explicitly delete member
                 lineCount();
                 current->args += " = delete";
                 current->m_specFlags.spec |= Entry::Delete;
                 current->argList.isDeleted = true;
                 BEGIN(FuncQual);
   }

<FuncQual,TrailingReturn>{BN}*"="{BN}*"default"{BN}*     { 
      // C++11 explicitly defaulted constructor/assignment operator
                 lineCount();
                 current->args += " = default";
                 current->m_specFlags.spec |= Entry::Default;
                 BEGIN(FuncQual);
   }

<FuncQual>{BN}*"->"{BN}*                {
                                          lineCount();
                                          current->argList.trailingReturnType = " -> ";
                                          current->args += " -> ";
                                          BEGIN(TrailingReturn);
   }

<TrailingReturn>[{;]                    {
                                          unput(*yytext);
                                          BEGIN(FuncQual);
   }

<TrailingReturn>.                       {
                                          current->argList.trailingReturnType+=yytext;
                                          current->args+=yytext;
   }

<TrailingReturn>\n                      {
                                          lineCount();
                                          current->argList.trailingReturnType+=yytext;
                                          current->args+=' ';
   }

<FuncRound,FuncFunc>{BN}*","{BN}*   {
                    lineCount() ;
                 current->args += ", " ;
   }

<FuncQual,FuncRound,FuncFunc>{BN}+     {
                    lineCount() ;
                 current->args += ' ' ;
   }

<Function,FuncQual,FuncRound,FuncFunc>"#" { if (insidePHP)
                      REJECT;
                    lastCPPContext = YY_START;
                    BEGIN(SkipCPP);
   }

<FuncQual>"="           {
                                          if (insideCli &&
                     (current_root->section&Entry::COMPOUND_MASK)
                    )
                 {
                   BEGIN(CliOverride);
                 }
                 else
                 {
                                            // typically an initialized function pointer
                      lastInitializerContext=YY_START;
                   initBracketCount=0;
                                            current->initializer = yytext;
                      BEGIN(ReadInitializer);
                 }
   }

<CliOverride>{ID}          {
   }

<CliOverride>"{"        {
                    unput(*yytext);
                 BEGIN(FuncQual);
   }

<CliOverride>\n            {
                                          lineCount();
   }

<CliOverride>.             {
   }

<FuncPtrInit>[{;]          {
                    unput(*yytext);
                 BEGIN(FuncQual);
   }

<FuncPtrInit>\"            {
                                          current->args += *yytext;
                    pCopyQuotedString=&current->args;
                 lastStringContext=FuncPtrInit;
                 BEGIN(CopyString);
   }

<FuncPtrInit>\'            {
                                          current->args += *yytext;
                 if (insidePHP)
                 {
                   pCopyQuotedString=&current->args;
                   lastStringContext=FuncPtrInit;
                   BEGIN(CopyPHPString);
                 }
   }

<FuncPtrInit>{CHARLIT}        {
                                          if (insidePHP)
                 {
                   REJECT;
                 }
                 else
                 {
                                            current->args += yytext;
                 }
   }

<FuncPtrInit>{ID}          {
                                          current->args += yytext;
   }

<FuncPtrInit>.             {
                                          current->args += *yytext;
   }

<FuncPtrInit>\n            {
                                          current->args += *yytext;
                                          lineCount();
   }

<FuncQual>{ID}             { 
      // typically a K&R style C function
                 if (insideCSharp && qstrcmp(yytext,"where")==0)
                 {
                   // type contraint for a method

                   current->typeConstr = ArgumentList();
                   current->typeConstr.append(Argument());
                   lastCSConstraint = YY_START;
                   BEGIN( CSConstraintName );
                 }
                 else if (checkForKnRstyleC())
                 {
                                            current->args = yytext;
                   oldStyleArgType.resize(0);
                   BEGIN(OldStyleArgs);
                 }
                 else
                 {
                   current->args += yytext;
                 }
   }

<OldStyleArgs>[,;]         {
                    QString oldStyleArgPtr;
                    QString oldStyleArgName;
                 splitKnRArg(oldStyleArgPtr,oldStyleArgName);
                 QString doc,brief;
                 if (current->doc!=docBackup)
                 {
                                            doc=current->doc;
                   current->doc=docBackup;
                 }
                 if (current->brief!=briefBackup)
                 {
                                            brief=current->brief;
                   current->brief=briefBackup;
                 }
                 addKnRArgInfo(oldStyleArgType+oldStyleArgPtr,
                               oldStyleArgName,brief,doc);
                 current->args.resize(0);
                 if (*yytext==';') oldStyleArgType.resize(0);
   }

<OldStyleArgs>{ID}         { 
      current->args += yytext; 
   }

<OldStyleArgs>"{"          {
                    current->args = argListToString(&current->argList);
                    unput('{');
                 BEGIN(FuncQual);
   }

<OldStyleArgs>.            { 
      current->args += *yytext; 
   }

<FuncQual,FuncRound,FuncFunc>.      { 
      current->args += *yytext; 
   }

<FuncQual>{BN}*"try:"         |
<FuncQual>{BN}*"try"{BN}+     { 
      /* try-function-block */
                 insideTryBlock=true;
                 lineCount();
                    if (yytext[yyleng-1]==':')
                 {
                   unput(':');
                   BEGIN( Function );
                 }
   }

<FuncQual>{BN}*"throw"{BN}*"("      { 
      // C++ style throw clause
                    current->exception = " throw (" ;
                 roundCount=0;
                 lineCount() ;
                 BEGIN( ExcpRound ) ;
   }

<FuncQual>{BN}*"raises"{BN}*"("         {
                    current->exception = " raises (" ;
                 lineCount() ;
                 roundCount=0;
                 BEGIN( ExcpRound ) ;
   }

<FuncQual>{BN}*"throws"{BN}+     { 
      // Java style throw clause
                    current->exception = " throws " ;
                 lineCount() ;
                 BEGIN( ExcpList );
   }

<ExcpRound>"("             { 
      current->exception += *yytext;
      ++roundCount;
   }

<ExcpRound>")"                          { 
      current->exception += *yytext ;
      if ( roundCount )
                   --roundCount ;
                      else
                   BEGIN( FuncQual ) ;
   }

<ExcpRound>.            {
                    current->exception += *yytext;
   }

<ExcpList>"{"           {
                    unput('{'); BEGIN( FuncQual );
   }

<ExcpList>";"           {
                    unput(';'); BEGIN( FuncQual );
                  }
<ExcpList>"\n"             {
                    current->exception += ' ';
                                          lineCount();
   }

<ExcpList>.             {
                    current->exception += *yytext;
   }

<Function>"("           { 
      current->type += current->name ;
                 current->name  = current->args ;
                 current->args  = yytext ;
                 roundCount=0;
                 BEGIN( FuncRound ) ;
   }

<Function>":"              {
                 if (! insidePHP) {
                    BEGIN(SkipInits);
                 } 
   }

<Function>[;{,]            {
   QString text = QString::fromUtf8(yytext);
   current->name = current->name.simplified();
   current->type = current->type.simplified();
   current->args = removeRedundantWhiteSpace(current->args);
   // was: current->args.simplified();

                 current->fileName    = yyFileName;
                 current->startLine   = yyBegLineNr;
                 current->startColumn = yyBegColNr;

                 static QRegExp re("\\([^)]*[*&][^)]*\\)");       // (...*...)     

                 if (text[0] !=';' || (current_root->section&Entry::COMPOUND_MASK) ) {
                   int tempArg = current->name.indexOf('<');

                   int ts = current->type.indexOf('<');
                   int te = current->type.lastIndexOf('>');
                   int ti = re.indexIn(current->type, 0);

                   // bug677315: A<int(void *, char *)> get(); is not a function pointer
                   // not a (...*...) pattern
                   // (...*...) is part of a template argument list

                   bool isFunction = (ti == -1) || (ts != -1 && ts < te && ts < ti && ti < te); 

                   QString tempName;
                   if (tempArg == -1) {
                      tempName = current->name; 

                   } else {
                      tempName = current->name.left(tempArg);
                   } 
      
                   if (! current->type.isEmpty() && (! isFunction || current->type.startsWith("typedef ")) )  {
                                      
                     if (isTypedef && ! current->type.startsWith("typedef ")) {
                        current->type.prepend("typedef ");
                     }

                     current->section = Entry::VARIABLE_SEC;

                   } else {
                     current->section = Entry::FUNCTION_SEC;
                     current->proto   = (text[0] == ';');
                   }

                 } else  {
                  // a global function prototype or function variable
                 
                   if (! current->type.isEmpty() && (re.indexIn(current->type, 0) !=-1 || current->type.left(8) == "typedef ")) {
                   
                     if (isTypedef && current->type.left(8)!="typedef ") {
                        current->type.prepend("typedef ");
                     }
                     
                     current->section = Entry::VARIABLE_SEC;

                   } else {
                     current->section = Entry::FUNCTION_SEC;
                     current->proto = true;
                   }
                 }

                 if ( insidePHP) {
                   if (findAndRemoveWord(current->type,"final"))
                   {
                      current->m_specFlags.m_isFinal = true;
                   }

                   if (findAndRemoveWord(current->type,"abstract"))
                   {
                     current->m_specFlags.spec |= Entry::Abstract;
                   }
                 }

                 if ( insidePHP && !containsWord(current->type,"function"))
                 {
                   initEntry();

                   if (text[0] == '{' ) {
                     lastCurlyContext = FindMembers;
                     curlyCount=0;
                     BEGIN( SkipCurly );
                   } else {
                     BEGIN( FindMembers );
                   }

                 } else {
                   if (insidePHP) {
                     findAndRemoveWord(current->type,"function");
                   }

                   previous = current;
                   current_root->addSubEntry(current, current_root);
                   current = QMakeShared<Entry>();
                   initEntry();

                   // Objective C 2.0: Required/Optional section
                   if (previous->m_specFlags.spec & (Entry::Optional | Entry::Required))
                   {
                     current->m_specFlags.spec |= previous->m_specFlags.spec & (Entry::Optional|Entry::Required);
                   }

                   lastCurlyContext = FindMembers;

                   if (text[0] == ',' ) {
                     current->type = previous->type;
                     // we need to strip any trailing * and & (see bugs 623023 and 649103 for test cases)
                     int i=current->type.length();
                     while (i > 0 && (current->type[i-1]=='*' || current->type[i-1]=='&' || current->type[i-1]==' ')) i--;
                     current->type = current->type.left(i);
                   }

                   if (text[0] == '{' ) {
                     if (! insidePHP && (current_root->section & Entry::COMPOUND_MASK) ) {
                        previous->m_specFlags.spec |= Entry::Inline;
                     }

                     curlyCount=0;
                     BEGIN( SkipCurly ) ;

                   } else {
                     if (previous->section != Entry::VARIABLE_SEC) {
                        previous->bodyLine=-1; // a function/member declaration
                     }   
                     BEGIN( FindMembers ) ;
                   }
                 }
   }

<SkipInits>{ID}{BN}*"{"                   { 
      // C++11 style initializer (see bug 688647)
                 lineCount();
                 curlyCount = 1;
                 BEGIN(SkipC11Inits);
   }

<SkipC11Inits>"{"                         {
                 ++curlyCount;
               }
<SkipC11Inits>"}"                         {
  					  if (--curlyCount <= 0) {
                    BEGIN(SkipInits);
                 }
               }
<SkipInits>"{"                            { 
      // C++11 style initializer
                 unput('{');
                 BEGIN( Function );
   }

<SkipCurly>"{"                            {
                 //addToBody(yytext);
                 ++curlyCount ;
   }

<SkipCurly>"}"/{BN}*("/*!"|"/**"|"//!"|"///")"<!--" | /* see bug710917 */
<SkipCurly>"}"             {
                         //addToBody(yytext);
                    if( curlyCount )
                 {
                   --curlyCount ;
                 }
                 else
                 {
                   if (current->sli && previous) // copy special list items
                  {
                     for (auto lii : *current->sli) {
                        previous->addSpecialListItem(lii.type, lii.itemId);
                     }
                     delete current->sli;
                     current->sli = 0;
                  }
                   if (previous) previous->endBodyLine=yyLineNr;
                   BEGIN( lastCurlyContext ) ;
                 }
   }

<SkipCurly>"}"{BN}*("/*!"|"/**"|"//!"|"///")"<" {
      // */ (editor syntax fix)

      lineCount();
      if ( curlyCount ) {
         //addToBody(yytext);
         --curlyCount ;
      }
                 else
                 {
                   current->endBodyLine=yyLineNr;

                   tempEntry = current; // temporarily switch to the previous entry
                   current = previous;
                   previous = QSharedPointer<Entry>();

                   docBlockContext   = SkipCurlyEndDoc;
                   docBlockInBody    = false;
                   docBlockAutoBrief = ( yytext[yyleng-2]=='*' && Config::getBool("javadoc-auto-brief") ) ||
                                       ( yytext[yyleng-2]=='!' && Config::getBool("qt-auto-brief") );
                   docBlock.resize(0);
                   docBlockTerm = '}';
                   if (yytext[yyleng-3]=='/')
                   {
                     startCommentBlock(true);
                     BEGIN( DocLine );
                   }
                   else
                   {
                     startCommentBlock(false);
                     BEGIN( DocBlock );
                   }
                 }
   }

<SkipCurlyEndDoc>"}"{BN}*("/*!"|"/**"|"//!"|"///")"<" { 

      // */ (editor syntax fix)
      // desc is followed by another one

                 docBlockContext   = SkipCurlyEndDoc;
                 docBlockInBody    = false;
                 docBlockAutoBrief = ( yytext[yyleng-2]=='*' && Config::getBool("javadoc-auto-brief") ) ||
                                     ( yytext[yyleng-2]=='!' && Config::getBool("qt-auto-brief") );
                 docBlock.resize(0);
                 docBlockTerm = '}';
                 if (yytext[yyleng-3]=='/')
                 {
                   startCommentBlock(true);
                   BEGIN( DocLine );
                 }
                 else
                 {
                   startCommentBlock(false);
                   BEGIN( DocBlock );
                 }
    }

<SkipCurlyEndDoc>"}"          {
                         //addToBody("}");
                                          if (tempEntry) // we can only switch back to current if no new item was created
                 {
                   current = tempEntry;
                    tempEntry = QSharedPointer<Entry>();
                 }
                    BEGIN( lastCurlyContext );
   }

<SkipCurly>\"                {
                         //addToBody(yytext);
                    lastStringContext=SkipCurly;
                         BEGIN( SkipString );
   }

<SkipCurly>^{B}*"#"                           {
                    if (insidePHP) {
                           REJECT;
                    } 
                    //addToBody(yytext);
                    BEGIN( SkipCurlyCpp );
   }

<SkipCurly,SkipC11Inits,SkipInits>\n          {
                    lineCount();
                    //addToBody(yytext);
   }

<SkipCurly,SkipCurlyCpp>"<<<"                 {
                    if (!insidePHP)
                 {
                   REJECT;
                 }
                 else
                 {
                                            lastHereDocContext = YY_START;
                   BEGIN(HereDoc);
                 }
   }

<SkipCurly,SkipCurlyCpp>[^\n#"'@\\/{}<]+ {
                                          lineCount();  // for g_column updates
                         //addToBody(yytext);
                  }
<SkipCurlyCpp>\n        {
                         //addToBody(yytext);
                                          lineCount();
                    lastCurlyContext = FindMembers;
                    BEGIN( SkipCurly );
   }

<SkipCurlyCpp>\\[\r]*"\n"[\r]*      {
                    //addToBody(yytext);
                                          lineCount();
   }

<SkipInits,SkipC11Inits,SkipCurly,SkipCurlyCpp>"/*"    {
                    //addToBody(yytext);
                    lastCContext = YY_START;
                    BEGIN(SkipComment);
   }

<SkipInits,SkipC11Inits,SkipCurly,SkipCurlyCpp>"//"  {
                    //addToBody(yytext);
                    lastCContext = YY_START;
                    BEGIN(SkipCxxComment);
   }

<SkipInits,SkipC11Inits>"("                          {
                   roundCount=0;
                   lastSkipRoundContext=YY_START;
                   BEGIN(SkipRound);
    }

<SkipInits,SkipC11Inits>\"           {
                    lastStringContext=YY_START;
                    BEGIN( SkipString );
               }
<SkipInits>;                          {
                 warn(yyFileName, yyLineNr, "Found a ';' while parsing an initializer list. "
                   "Some macros which are used without semicolons may not be parsed accurately.\n");

                 BEGIN( FindMembers );
   }

<SkipInits,SkipCurly,SkipCurlyCpp>"#"   {
                 if (!insidePHP) {
                    REJECT;
                 }    
                 //addToBody(yytext);
                 lastCContext = YY_START;
                 BEGIN(SkipCxxComment);
   }

<SkipInits,SkipCurly,SkipCurlyCpp>@\"   {
                 if (! insideCSharp) REJECT;
                 // C# verbatim string
                 lastSkipVerbStringContext=YY_START;
                 pSkipVerbString=&current->initializer;
                 BEGIN(SkipVerbString);
   }

<SkipInits,SkipCurly,SkipCurlyCpp>{CHARLIT}       {
                                          if (insidePHP) REJECT;
   }

<SkipInits,SkipCurly,SkipCurlyCpp>\'              {
                 if (insidePHP) {
                   lastStringContext=YY_START;
                   BEGIN(SkipPHPString);
                 }
   }

<SkipInits,SkipC11Inits,SkipCurly,SkipCurlyCpp>.  { 
   }

<SkipString,SkipPHPString>\\.                     { 
   }

<SkipString>\"                                    {
                  BEGIN( lastStringContext );
   }

<SkipPHPString>\'                                 {
                  BEGIN( lastStringContext );
   }

<SkipString,SkipPHPString>"/*"|"*/"|"//" { 
   }

<SkipString,SkipPHPString>\n     {
      lineCount();
   }

<SkipString,SkipPHPString>.      { }
<CompoundName>":"          { 
      // for "class : public base {} var;" construct, see bug 608359
      unput(':');
      BEGIN(ClassVar);
   }

<CompoundName>";"          {
                 current->section = Entry::EMPTY_SEC ;
                 current->type.resize(0) ;
                 current->name.resize(0) ;
                 current->args.resize(0) ;
                 current->argList.clear();
                 BEGIN( FindMembers ) ;
   }

<Bases>";"        {
                 if (insideIDL && (current->m_specFlags.spec & (Entry::Singleton | Entry::Service)))
                 {
                   // in UNO IDL a service or singleton may be defined
                   // completely like this: "service Foo : XFoo;"
                   if (!current->name.isEmpty() && !current_root->name.isEmpty())
                   {
                     prependScope();
                   }
                   current->name = current->name.trimmed();
                   // there can be only one base class here
                   if (! baseName.isEmpty())
                   {
                     current->extends.append(BaseInfo(baseName, Public, Normal));
                     baseName.resize(0);
                   }
                   current_root->addSubEntry( current, current_root);
                   current = QMakeShared<Entry>();
                 }
                 else
                 {
                   current->section = Entry::EMPTY_SEC ;
                   current->type.resize(0) ;
                   current->name.resize(0) ;
                   current->args.resize(0) ;
                   current->argList.clear();
                 }
                 BEGIN( FindMembers ) ;
    }

<CompoundName>{SCOPENAME}{BN}*/"<"  {
                    sharpCount = 0;
                    current->name = yytext ;
                 if (current->m_specFlags.spec & Entry::Protocol)
                 {
                   current->name+="-p";
                 }
                 lineCount();
                 lastClassTemplSpecContext = ClassVar;
                 if (insideObjC) // protocol list
                 {
                   BEGIN( ObjCProtocolList );
                 }
                 else if (insideCSharp) // C# generic class
                 {
                                            //current->name+="-g";
                   BEGIN( CSGeneric );
                 }
                 else // C++ template specialization
                 {
                   roundCount=0;
                   BEGIN( ClassTemplSpec );
                 }
    }

<CSGeneric>"<"             {
                 if (current->tArgLists==0)
                 {
                   current->tArgLists = new QList<ArgumentList>;
                   // current->tArgLists->setAutoDelete(true);
                 }

                 // check bug 612858 before enabling the next line
                 //current->spec |= Entry::Template;

                 current->tArgLists->append(ArgumentList());
                 ArgumentList *al = &current->tArgLists->last();

                 currentArgumentList = al;

                 templateStr="<";
                 current->name += "<";
                 fullArgString = templateStr;
                 copyArgString = &current->name;

                 //copyArgString = &templateStr;
                 currentArgumentContext = ClassVar;
                 BEGIN( ReadTempArgs );
   }

<ObjCProtocolList>"<"         {
                    insideProtocolList=true;
                    BEGIN( Bases );
   }

<ClassTemplSpec>">"({BN}*"::"{BN}*{SCOPENAME})?    {
                 current->name += yytext;
                 lineCount();
                    if (--sharpCount<=0)
                 {
                   current->name = removeRedundantWhiteSpace(current->name);
                   if (current->m_specFlags.spec & Entry::Protocol)
                   { // Objective-C protocol
                     unput('{'); // fake start of body
                     BEGIN( ClassVar );
                   }
                   else
                   {
                     BEGIN( lastClassTemplSpecContext );
                   }
                 }
   }

<ClassTemplSpec>"<"        {
                    current->name += yytext;
                    sharpCount++;
   }

<ClassTemplSpec>.          {
                    current->name += yytext;
   }

<CompoundName>{SCOPENAME}{BN}*";"   { 
      // forward declaration
                                          if (current->tArgLists && current->tArgLists->count()>0)
                                          {
                                            // found a forward template declaration, this has
                                            // a purpose of its own
                                            current->name = yytext;
                                            current->name=current->name.left(current->name.length()-1).trimmed();
                                            //printf("template class declaration for %s!\n",current->name.data());
                   QString rn = current_root->name;
                   //printf("cn=`%s' rn=`%s' isTypedef=%d\n",cn.data(),rn.data(),isTypedef);
                   if (!current->name.isEmpty() && !rn.isEmpty())
                   {
                     prependScope();
                   }
                                            current->m_specFlags.spec |= Entry::ForwardDecl;
                                            current_root->addSubEntry(current, current_root);
                                             current = QMakeShared<Entry>();
                                          }
                 else if (insideIDL &&
                          (((current_root->m_specFlags.spec & (Entry::Interface | Entry::Service)) &&
                            (current->m_specFlags.spec & Entry::Interface)) ||
                           ((current_root->m_specFlags.spec & (Entry::Service | Entry::Singleton)) &&
                            (current->m_specFlags.spec & Entry::Service))))
                 {
                   // interface inside of UNO IDL service or interface
                   // service inside of UNO IDL service or singleton
                   // there may be documentation on the member,
                   // so do not throw it away...
                   current->name = yytext;
                   current->name=current->name.left(current->name.length()-1).trimmed();
                   current->section = (current->m_specFlags.spec & Entry::Interface)
                       ? Entry::EXPORTED_INTERFACE_SEC
                       : Entry::INCLUDED_SERVICE_SEC;
//                 current->section = Entry::MEMBERDOC_SEC;
                   current->m_specFlags.spec &= ~(Entry::Interface|Entry::Service);

// FIXME: horrible: Interface == Gettable, so need to clear it - actually we are mixing values from different enums in
// this case... granted only Optional and Interface are actually valid in this context but urgh...
                   current_root->addSubEntry(current, current_root);
                   current = QMakeShared<Entry>();
                 }

                 unput(';');
                 current->reset();
                 initEntry();
                 if (isTypedef) // typedef of a class, put typedef keyword back
                 {
                   current->type.prepend("typedef");
                 }
                 BEGIN( FindMembers );
   }

<CompoundName>{SCOPENAME}/{BN}*"("  {
                 current->name = yytext ;
                 lineCount();
                                          if (insideCpp && current->name=="alignas") // C++11
                                          {
                                            lastAlignAsContext = YY_START;
                                            BEGIN( AlignAs );
                                          }
                                          else
                                          {
                                            if (current->m_specFlags.spec & Entry::Protocol)
                                            {
                                              current->name += "-p";
                                            }
                                            BEGIN( ClassVar );
                                          }
   }

<AlignAs>"("      { 
      roundCount=0;
      BEGIN( AlignAsEnd );
   }

<AlignAs>\n      { 
      lineCount(); 
   }

<AlignAs>.
<AlignAsEnd>"("  { 
      roundCount++; 
   }

<AlignAsEnd>")"  { 
      if (--roundCount <0 ) {
        BEGIN( lastAlignAsContext );
      }
   }

<AlignAsEnd>\n   { 
      lineCount(); 
   }

<AlignAsEnd>.
<CompoundName>{SCOPENAME}/{BN}*","  {  
      // multiple forward declarations on one line
      // e.g. @protocol A,B;
      current->reset();
      initEntry();
   }

<CompoundName>{SCOPENAME}     {
                 current->name = yytext ;
                                          if (insideCpp || insideObjC)
                                          {
                                            current->id = ClangParser::instance()->lookup(yyLineNr,yytext);
                                          }
                 lineCount();
                 if (current->m_specFlags.spec & Entry::Protocol)
                 {
                   current->name += "-p";
                 }
                 if ((current->m_specFlags.spec & Entry::Protocol) ||
                     current->section == Entry::OBJCIMPL_SEC)
                 {
                   unput('{'); // fake start of body
                 }
                 BEGIN( ClassVar );
   }

<CompoundName>{CSSCOPENAME}           { // C# style scope
                 current->name = substitute(yytext,".","::");
                 lineCount();
                 BEGIN( ClassVar );
   }

<ClassVar>{SCOPENAME}{BN}*/"("      {
                    if (insideIDL && qstrncmp(yytext,"switch",6)==0 && !isId(yytext[6]))
                 {
                   // Corba IDL style union
                   roundCount=0;
                   BEGIN(SkipUnionSwitch);
                 }
                 else
                 {
                      addType(current);
                   current->name = yytext;
                   current->name = current->name.trimmed();
                   lineCount();
                      BEGIN( FindMembers );
                 }
   }

<ClassVar>","           {
                 if (isTypedef)
                 {
                   // multiple types in one typedef
                   unput(',');
                   current->type.prepend("typedef ");
                   BEGIN(FindMembers);
                 }
                 else
                 {
                      // Multiple class forward declaration
                 }
   }

<ClassVar>("sealed"|"abstract")/{BN}*(":"|"{") {
                    if (insideCli)
                 {
                   if (yytext[0]=='s') // sealed
                     current->m_specFlags.spec|= Entry::SealedClass;
                   else // abstract
                     current->m_specFlags.m_isAbstractClass = true;
                   BEGIN( ClassVar );
                 }
                 else
                 {
                   REJECT;
                 }
   }

<ClassVar>{ID}             {
                                          if (insideCpp || insideObjC)
                                          {
                                            current->id = ClangParser::instance()->lookup(yyLineNr,yytext);
                                          }
                    if (insideIDL && qstrcmp(yytext,"switch")==0)
                 {
                   // Corba IDL style union
                   roundCount=0;
                   BEGIN(SkipUnionSwitch);
                 }
                 else if ((insideJava || insidePHP || insideJS) && (qstrcmp(yytext,"implements")==0 || qstrcmp(yytext,"extends")==0))
                 {
                      current->type.resize(0);
                   baseProt=Public;
                                            baseVirt=Normal;
                   baseName.resize(0);
                   BEGIN( BasesProt ) ;
                 }
                 else if (insideCSharp && qstrcmp(yytext,"where")==0) // C# type contraint
                 {

                   current->typeConstr = ArgumentList();
                   current->typeConstr.append(Argument());

                   lastCSConstraint = YY_START;
                   BEGIN( CSConstraintName );
                 }
                 else if (insideCli &&  qstrcmp(yytext,"abstract")==0)
                 {
                   current->m_specFlags.spec |= Entry::Abstract;
                 }
                 else if (insideCli && qstrcmp(yytext,"sealed")==0)
                 {
                   current->m_specFlags.spec |= Entry::Sealed;
                 }
                 else if (qstrcmp(yytext,"final")==0)
                 {
                     current->m_specFlags.m_isFinal = true;
                 }
                 else
                 {
                   if (current->section == Entry::ENUM_SEC)
                   { // found "enum a b" -> variable
                     current->section = Entry::VARIABLE_SEC ;
                   }
                   current->type += ' ' ;
                   current->type += current->name ;
                   current->name = yytext ;

                   if (nameIsOperator(current->name))
                   {
                          BEGIN( Operator );
                   }
                 }
   }

<ClassVar>[(\[]            {
                    if (insideObjC && *yytext=='(') // class category
                 {
                   current->name+='(';
                                            //if (current->section!=Entry::OBJCIMPL_SEC)
                                            //{
                     current->m_specFlags.spec |= Entry::Category;
                                            //}
                   BEGIN( ClassCategory );
                 }
                 else
                 {
                                            // probably a function anyway
                                            unput(*yytext);
                   BEGIN( FindMembers );
                 }
   }

<CSConstraintType,CSConstraintName>"/**/" {
      /* empty comment */ 
   }

<CSConstraintType,CSConstraintName>("/*"[*!]|"//"[/!])("<"?)   {
      // */ (editor syntax fix)
      // special comment
                                          fullArgString.resize(0);
                 lastCopyArgChar='#'; // end marker
                 lastCommentInArgContext=YY_START;
                 if (yytext[1]=='/')
                   BEGIN( CopyArgCommentLine );
                 else
                   BEGIN( CopyArgComment );
   }

<CSConstraintType,CSConstraintName>"#"    { 
      // artificially inserted token to signal end of comment block
                                          current->typeConstr.last().docs = fullArgString;
   }

<CSConstraintType>"{"         { 
      // end of type constraint reached
      // parse documentation of the constraints
      handleParametersCommentBlocks(&current->typeConstr);
      unput('{');
      BEGIN( lastCSConstraint );
   }

<CSConstraintType,CSConstraintName>";"  {
      handleParametersCommentBlocks(&current->typeConstr);
      unput(';');
      BEGIN( lastCSConstraint );
   }

<CSConstraintName>":"                   {
      BEGIN( CSConstraintType );
   }

<CSConstraintName>{ID}                  {
                                          // parameter name
                                          current->typeConstr.last().name = yytext;
   }

<CSConstraintType>"where"     { 
      // another constraint for a different param
                 current->typeConstr.append(Argument());
                                          BEGIN( CSConstraintName );
                  }
<CSConstraintType>({ID}".")*{ID}("<"{ID}">")?("()")?  {
                                          if (current->typeConstr.last().type.isEmpty())
                                              // first type constraint for this parameter
                                          {
                                            current->typeConstr.last().type=yytext;
                                          }
                                          else // new type constraint for same parameter
                                          {
                  QString name = current->typeConstr.last().name;
                  current->typeConstr.append(Argument());
                  current->typeConstr.last().name = name;
                  current->typeConstr.last().type = yytext;
                                          }
   }

<CSConstraintName,CSConstraintType>\n  {
                                          lineCount();
   }

<CSConstraintName,CSConstraintType>.   {
   }

<ClassCategory>{ID}        {
                 current->name+=yytext;
   }

<ClassCategory>")"/{BN}*"{"      {
                 current->name+=')';
                 BEGIN( ClassVar );
   }

<ClassCategory>")"/{BN}*"<"      {
                 current->name+=')';
                 BEGIN( ObjCProtocolList );
   }

<ClassCategory>")"         {
                 current->name+=')';
                 if ((current->section & Entry::Protocol) ||
                     current->section == Entry::OBJCIMPL_SEC)
                 {
                   unput('{'); // fake start of body
                 }
                 else // category has no variables so push back an empty body
                 {
                   unput('}');
                   unput('{');
                 }
                 BEGIN( ClassVar );
   }

<ClassVar>":"           {
                 if (current->section == Entry::VARIABLE_SEC) {
                    // enum A B:2, see bug 748208

                    current->bitfields += ":";
                    current->args.resize(0);
                    BEGIN(BitFields);

                 } else if (current->section == Entry::ENUM_SEC) {
                    // enum E:2, see bug 313527, 
                    // or C++11 style enum: 'E : unsigned int {...}'
                              
                   current->args.resize(0);
                   BEGIN(EnumBaseType);

                 } else {
                  current->type.resize(0);

                  if ((current->m_specFlags.spec & Entry::Interface) ||
                     (current->m_specFlags.spec & Entry::Struct)    ||
                     (current->m_specFlags.spec & Entry::Ref)       ||
                     (current->m_specFlags.spec & Entry::Value)     ||
                     insidePHP || insideCSharp || insideD || insideObjC || insideIDL) {

                      baseProt = Public;

                   } else {
                      baseProt = Private;

                   }  

                   baseVirt = Normal;
                   baseName.resize(0);
                   BEGIN( BasesProt ) ;
                 }
   }

<ClassVar>[;=*&]        {
                    unput(*yytext);
                 if (isTypedef) // typedef of a class, put typedef keyword back
                 {
                   current->type.prepend("typedef");
                 }
                 if ((yytext[0]=='*' || yytext[0]=='&') &&
                     current->section == Entry::ENUM_SEC)
                 { // found "enum a *b" -> variable
                   current->section = Entry::VARIABLE_SEC ;
                 }
                 BEGIN( FindMembers );
   }

<Bases,ClassVar>"///"/[^/]              {
                    if (!insideObjC)
                 {
                   REJECT;
                 }
                 else
                 {
                   lineCount();
                   current->program+=yytext;
                                            current->fileName = yyFileName ;
                   current->startLine = yyLineNr ;
                   current->startColumn = yyColNr;
                   curlyCount=0;
                   BEGIN( ReadBodyIntf );
                 }
   }

<Bases,ClassVar>("//"{B}*)?"/**"/[^/*]  |
<Bases,ClassVar>("//"{B}*)?"/*!"        |
<Bases,ClassVar>"//!"                   |
<Bases,ClassVar>[\-+]{BN}*       {
      // */ (editor syntax fix)

      if (! insideObjC) {
                   REJECT;
                 }
                 else
                 {
                   lineCount();
                   current->program+=yytext;
                                            current->fileName = yyFileName ;
                   current->startLine = yyLineNr ;
                   current->startColumn = yyColNr;
                   curlyCount=0;
                   BEGIN( ReadBodyIntf );
                 }
   }

<CompoundName,ClassVar>{B}*"{"{B}*  {
      current->fileName = yyFileName ;
      current->startLine = yyLineNr ;
      current->startColumn = yyColNr;
      current->name = removeRedundantWhiteSpace(current->name);
                 if (current->name.isEmpty() && !isTypedef) // anonymous compound
                 {
                   if (current->section==Entry::NAMESPACE_SEC) // allow reopening of anonymous namespaces
                   {
                     if (Config::getBool("extract-anon-namespaces")) // use visible name
                     {
                       current->name="anonymous_namespace{"+stripPath(current->fileName)+"}";
                     }
                     else // use invisible name
                     {
                        current->name = QString("@%1").arg(anonNSCount);
                     }
                   }
                   else
                   {
                      current->name = QString("@%1").arg(anonCount++);
                   }
                 }
                 curlyCount=0;
                 if (current_root && // not a nested struct inside an @interface section
                     !(current_root->m_specFlags.spec & Entry::Interface) &&
                     ((current->m_specFlags.spec & (Entry::Interface | Entry::Protocol | Entry::Category) ||
                       current->section==Entry::OBJCIMPL_SEC)
                     ) &&
                     insideObjC
                    )
                 { // ObjC body that ends with @end
                   BEGIN( ReadBodyIntf );
                 }
                 else if (current->section==Entry::NAMESPACE_SEC)
                 { // namespace body
                   BEGIN( ReadNSBody );
                 }
                 else
                 { // class body
                   BEGIN( ReadBody ) ;
                 }
   }

<BasesProt>"virtual"{BN}+               { 
      lineCount(); 
      baseVirt = Virtual; 
   }

<BasesProt>"public"{BN}+                { 
      lineCount(); 
      baseProt = Public; 
   }

<BasesProt>"protected"{BN}+             { 
      lineCount(); 
      baseProt = Protected; 
   }

<BasesProt>"internal"{BN}+              { 
      lineCount(); 
      baseProt = Package;    
   }

<BasesProt>"private"{BN}+               { 
      lineCount(); 
      baseProt = Private; 
   }

<BasesProt>{BN}         { 
      lineCount(); 
   }

<BasesProt>.            { 
      unput(*yytext); 
      BEGIN(Bases); 
   }

<Bases>("\\")?({ID}"\\")*{ID}       {      
      // PHP namespace token, not sure if interspacing is allowed but it gives problems (see bug 640847)
                 if (! insidePHP)
                 {
                   REJECT;
                 }
                 else // PHP base class of the form \Ns\Cl or Ns\Cl
                 {
                   lineCount();
                   QString bn=yytext;
                   bn = substitute(bn,"\\","::");
                   baseName += bn;
                   current->args += ' ';
                   current->args += yytext;
   }
                                        }
<Bases>("::")?{BN}*({ID}{BN}*"::"{BN}*)*{ID}    {     
      lineCount();
                  QString baseScope = yytext;
                  if (insideCSharp && baseScope.trimmed()=="where")
                 {
                   // type contraint for a class

                  current->typeConstr = ArgumentList();
                  current->typeConstr.append(Argument());

                  lastCSConstraint = YY_START;
                  BEGIN( CSConstraintName );
                 }
                 else
                 {
                      baseName+=yytext;
                   current->args += ' ';
                   current->args += yytext;
                 }
   }

<Bases>{BN}*{ID}("."{ID})*       { 
      // Java style class
                    QString name = substitute(yytext,".","::");
                 baseName += name;
                 current->args += ' ';
                 current->args += name;
   }

<ClassVar,Bases>\n/{BN}*[^{, \t\n]  {
                                          if (!insideObjC)
                 {
                   REJECT;
                 }
                 else
                 {
                                            lineCount();
                   unput('{');
                 }
   }

<ClassVar,Bases>"@end"        {
      // empty ObjC interface
      unput('d'); // insert fake body: {}@end
      unput('n');
      unput('e');
      unput('@');
      unput('}');
      unput('{');
   }

<ClassVar>"<"                     {
   current->name += *yytext;
                    sharpCount=1;
                 roundCount=0;
                 lastSkipSharpContext = YY_START;
                 specName = &current->name;
                 BEGIN ( Specialization );
   }

<Bases>{BN}*"<"                         {
                                          lineCount();
                    sharpCount=1;
                 roundCount=0;
                 lastSkipSharpContext = YY_START;
                 if (insideObjC) // start of protocol list
                 {
                   unput(',');
                 }
                 else // template specialization
                 {
                   //if (insideCSharp) // generic
                   //{
                   //  baseName+="-g";
                   //}
                                            templateStr = yytext;
                   specName = &templateStr;
                   BEGIN ( Specialization );
                 }
   }

<Specialization>"<"        { *specName += *yytext;
                    if (roundCount==0) sharpCount++;
   }

<Specialization>">"        {
                    *specName += *yytext;
                    if (roundCount==0 && --sharpCount<=0)
                 {
                   baseName+=removeRedundantWhiteSpace(*specName);
                   BEGIN(lastSkipSharpContext);
                 }
   }

<Specialization>{BN}+         { 
      lineCount(); 
      *specName +=' '; 
   }

<Specialization>"<<"          { 
      *specName += yytext; 
   }

<Specialization>">>"/{B}*"::"       { 
      // M$ C++ extension to allow >> to close a template...
                    unput('>');
                    unput(' ');
                    unput('>');
   }

<Specialization>">>"          {
                                          if (insideCSharp) // for C# >> ends a nested template
                 {
                   REJECT;
                 }
                 else // for C++ >> is a bitshift
                      // operator and > > would end
                      // a nested template.
                      // We require the bitshift to be enclosed in braces.
                      // See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2005/n1757.html
                 {
                   if (roundCount>0)
                   {
                                              *specName += yytext;
                   }
                   else
                   {
                        unput('>');
                        unput(' ');
                        unput('>');
                   }
                 }
   }

<Specialization>"typename"{BN}+     { 
      lineCount(); 
   }

<Specialization>"("        { 
      *specName += *yytext; 
      roundCount++; 
   }

<Specialization>")"        { 
      *specName += *yytext; 
      roundCount--; 
   }

<Specialization>.          {
                    *specName += *yytext;
   }

<SkipRound>"("             { ++roundCount; 
   }

<SkipRound>")"             { 
      if (--roundCount < 0)
                   BEGIN ( lastSkipRoundContext );
   }

<SkipRound>\"           {
                    lastStringContext=SkipRound;
                 BEGIN(SkipString);
   }

<Bases>","|(">"({BN}*"{")?)|({BN}+"implements"{BN}*)  { 
   lineCount();
                                          if (insideProtocolList)
                 {
                   baseName+="-p";
                 }
                 else
                 {
                                            current->args += ',' ;
                 }
                 current->name = removeRedundantWhiteSpace(current->name);
                    if (! baseName.isEmpty())
                 {
                     current->extends.append(BaseInfo(baseName, baseProt, baseVirt));
                 }

                 if ((current->m_specFlags.spec & (Entry::Interface|Entry::Struct)) ||
                     insideJava || insidePHP || insideCSharp ||
                     insideD || insideObjC || insideIDL)
                 {
                   baseProt=Public;
                 }
                 else
                 {
                   baseProt=Private;
                 }
                 baseVirt=Normal;
                 baseName.resize(0);
                                          if (*yytext=='>')
                 { // end of a ObjC protocol list
                      insideProtocolList=false;
                   if (yyleng==1)
                   {
                     unput('{'); // dummy start body
                   }
                   else
                   {
                     yyless(1);
                   }
                 }
                 else
                 {
                   if (*yytext==',' && insideObjC) // Begin of protocol list
                   {
                        insideProtocolList=true;
                   }
                   BEGIN(BasesProt);
                 }
   }

<Bases>{B}*"{"{B}*         { 
      current->fileName = yyFileName ;
                 current->startLine = yyLineNr ;
                 current->startColumn = yyColNr;
                 current->name = removeRedundantWhiteSpace(current->name);

                    if (!baseName.isEmpty())
                   current->extends.append(BaseInfo(baseName, baseProt, baseVirt));

                 curlyCount=0;
                 if (insideObjC)
                 {
                   BEGIN( ReadBodyIntf );
                 }
                 else
                 {
                   BEGIN( ReadBody ) ;
                 }
   }

<SkipUnionSwitch>{B}*"("      {
                    roundCount++;
   }

<SkipUnionSwitch>")"          {
                    if (--roundCount==0)
                 {
                   BEGIN(ClassVar);
                 }
   }

<SkipUnionSwitch>\n        { 
      lineCount(); 
   }

<SkipUnionSwitch>.
<Comment>{BN}+             { 
      current->program += yytext;
      lineCount() ;
   }

<Comment>"/*"           { 
      // */ (editor syntax fix)
      current->program += yytext; 
   }

<Comment>"//"           { 
      current->program += yytext; 
   }

<Comment>{CMD}("code"|"verbatim")   {
      insideCode=true;
      current->program += yytext ;
   }

<Comment>{CMD}("endcode"|"endverbatim")   {
      insideCode=false;
      current->program += yytext ;
   }

<Comment>[^ \.\t\r\n\/\*]+       { 
      current->program += yytext ; 
   }

<Comment>"*/"           { 
      current->program += yytext ;
      if (!insideCode) BEGIN( lastContext ) ;
   }

<Comment>.           { 
      current->program += *yytext ; 
   }


<FindMembers,FindFields,MemberSpec,FuncQual,SkipCurly,Operator,ClassVar,SkipInits,SkipC11Inits,Bases,OldStyleArgs>("//"{B}*)?"/*!" {
      // */ (editor syntax fix)
      //printf("Start doc block at %d\n",yyLineNr);
                 removeSlashes=(yytext[1]=='/');
                 tmpDocType=-1;
                 if (!current->doc.isEmpty())
                 {
                   current->doc+="\n\n";
                 }
                 else
                 {
                   current->docLine = yyLineNr;
                   current->docFile = yyFileName;
                 }

                 lastDocContext = YY_START;
                 if (current_root->section & Entry::SCOPE_MASK)
                                          {
                   current->inside = current_root->name+"::";
                                          }
                 docBlockContext   = YY_START;
                 docBlockInBody    = YY_START==SkipCurly;
                 docBlockAutoBrief = Config::getBool("qt-auto-brief");

                                          QString indent;
                                          indent.fill(' ',computeIndent(yytext,g_column));
                                          docBlock=indent;

                 if (docBlockAutoBrief)
                 {
                   current->briefLine = yyLineNr;
                   current->briefFile = yyFileName;
                 }
                 startCommentBlock(false);
                 BEGIN( DocBlock );
   }

<FindMembers,FindFields,MemberSpec,FuncQual,SkipCurly,Operator,ClassVar,SkipInits,Bases,OldStyleArgs>("//"{B}*)?"/**"/[^/*] {
                    removeSlashes=(yytext[1]=='/');
                 lastDocContext = YY_START;

                 //printf("Found comment block at %s:%d\n",yyFileName,yyLineNr);
                 if (current_root->section & Entry::SCOPE_MASK)
                                          {
                   current->inside = current_root->name+"::";
                                          }
                 current->docLine = yyLineNr;
                 current->docFile = yyFileName;
                 docBlockContext = YY_START;
                 docBlockInBody  = YY_START==SkipCurly;
                                          static bool javadocAutoBrief = Config::getBool("javadoc-auto-brief");
                 docBlockAutoBrief = javadocAutoBrief;

                                          QString indent;
                                          indent.fill(' ',computeIndent(yytext,g_column));
                                          docBlock=indent;

                 if (docBlockAutoBrief)
                 {
                   current->briefLine = yyLineNr;
                   current->briefFile = yyFileName;
                 }
                 startCommentBlock(false);
                 BEGIN( DocBlock );
   }

<FindMembers,FindFields,MemberSpec,SkipCurly,FuncQual,Operator,ClassVar,Bases,OldStyleArgs>"//!" {
                 tmpDocType=-1;
                 lastDocContext = YY_START;
                 if (current_root->section & Entry::SCOPE_MASK)
                                          {
                   current->inside = current_root->name+"::";
                                          }
                 docBlockContext   = YY_START;
                 docBlockInBody    = YY_START==SkipCurly;
                 docBlockAutoBrief = false;

                                          QString indent;
                                          indent.fill(' ',computeIndent(yytext,g_column));
                                          docBlock=indent;

                 startCommentBlock(current->brief.isEmpty());
                 BEGIN( DocLine );
   }

<FindMembers,FindFields,MemberSpec,SkipCurly,FuncQual,Operator,ClassVar,Bases,OldStyleArgs>"///"/[^/] {
                 tmpDocType=-1;
                 lastDocContext = YY_START;
                 if (current_root->section & Entry::SCOPE_MASK)
                                          {
                   current->inside = current_root->name+"::";
                                          }
                 docBlockContext   = YY_START;
                 docBlockInBody    = YY_START==SkipCurly;
                 docBlockAutoBrief = false;
                                          QString indent;
                                          indent.fill(' ',computeIndent(yytext,g_column));
                                          docBlock=indent;
                 startCommentBlock(current->brief.isEmpty());
                 BEGIN( DocLine );
   }

<FindMembers>"extern"{BN}*"\"C"("++")?"\""{BN}*("{")?  {
                 lineCount();
                 externC=true;
   }

<FindMembers>"{"        {
                                          if (externC)
                 {
                   externC=false;
                 }
                                          else if (insideCSharp &&
                     !current->name.isEmpty() &&
                     !current->type.isEmpty())
                 {
                   if (containsWord(current->type,"event")) // event
                   {
                        current->mtype = mtype = MethodTypes::Event;
                   }
                   else // property
                   {
                        current->mtype = mtype = MethodTypes::Property;
                   }
                   current->bodyLine = yyLineNr;
                   curlyCount=0;
                   BEGIN( CSAccessorDecl );
                 }
                 else if (insideIDL && (current->m_specFlags.spec & Entry::Attribute))
                 {
                   // UNO IDL: attributes may have setter and getter
                   // exception specifications
                   current->exception = " {";
                   BEGIN(UNOIDLAttributeBlock);
                 }
                 else
                 {
                   if ((insideJava || insideCSharp || insideD) &&
                  current->name.isEmpty()
                      )
                   {
                     // static Java initializer
                     needsSemi = false;
                     if (current->stat)
                     {
                  current->name="[static initializer]";
                  current->type.resize(0);
                     }
                     else
                     {
                  current->name="[instance initializer]";
                     }
                                              unput(*yytext);
                     BEGIN( Function );
                   }
                   else
                   {
                     // pre C++11 code -> ignore the initializer
                                              //needsSemi = true;
                     //current->type.resize(0);
                     //current->name.resize(0);
                     //current->args.resize(0);
                     //current->argList.clear();
                     //curlyCount=0;
                     //BEGIN( SkipCurlyBlock );

                                              // C++11 style initializer list
                        current->bodyLine = yyLineNr;
                                              current->initializer = yytext;
                     lastInitializerContext = YY_START;
                     initBracketCount=1;
                     BEGIN(ReadInitializer);
                   }
                 }
   }

<CSAccessorDecl>"{"        { 
      curlyCount++; 
   }

<CSAccessorDecl>"}"        {
                                          if (curlyCount)
                                          {
                                            curlyCount--;
                 }
                 else
                 {
                      mtype = MethodTypes::Method;
                                            virt = Normal;
                                            unput(';');
                   BEGIN(FindMembers);
                 }
   }

<CSAccessorDecl>"private "{BN}*"set"      { 
      if (curlyCount==0) current->m_specFlags.spec |= Entry::PrivateSettable;   
   }

<CSAccessorDecl>"protected "{BN}*"set"    { 
      if (curlyCount==0) current->m_specFlags.spec |= Entry::ProtectedSettable; 
   }

<CSAccessorDecl>"private "{BN}*"get"      { 
      if (curlyCount==0) current->m_specFlags.spec |= Entry::PrivateGettable;    
   }

<CSAccessorDecl>"protected "{BN}*"get"    { 
      if (curlyCount==0) current->m_specFlags.spec |= Entry::ProtectedGettable; 
   }

<CSAccessorDecl>"set"         { 
      if (curlyCount==0) current->m_specFlags.spec |= Entry::Settable;  
   }

<CSAccessorDecl>"get"         { 
      if (curlyCount==0) current->m_specFlags.spec |= Entry::Gettable;  
   }

<CSAccessorDecl>"add"         { 
      if (curlyCount==0) current->m_specFlags.spec |= Entry::Addable;   
   }

<CSAccessorDecl>"remove"      { 
      if (curlyCount==0) current->m_specFlags.spec |= Entry::Removable; 
   }

<CSAccessorDecl>"raise"       { 
      if (curlyCount==0) current->m_specFlags.spec |= Entry::Raisable;  
   }

<CSAccessorDecl>.          {
   }

<CSAccessorDecl>\n         { 
      lineCount(); 
   }

 
 /*  Documentation block related rules */

 /*  ---- Single line comments ------ */
<DocLine>[^\n]*"\n"[ \t]*"//"[/!]       { 
      // continuation of multiline C++-style comment          
      docBlock+=yytext;
      docBlock.resize(docBlock.length() - 3);
      lineCount();
}

<DocLine>{B}*"///"[/]+{B}*/"\n"     { 
      // ignore marker line (see bug700345)
      handleCommentBlock(docBlock, current->brief.isEmpty());
      BEGIN( docBlockContext );
   }

<DocLine>[^\n]*/"\n"          { 
      // whole line
      docBlock+=yytext;
      handleCommentBlock(docBlock, current->brief.isEmpty());
      BEGIN( docBlockContext );
   }

 /* ---- Comments blocks ------ */

<DocBlock>"*"*"*/"         { 
      // end of comment block
      handleCommentBlock(docBlock, false);
      BEGIN(docBlockContext);
   }

<DocBlock>^{B}*"*"+/[^/]      {

                                          QString indent;
                                          indent.fill(' ',computeIndent(yytext,g_column));
                                          docBlock+=indent;
   }

<DocBlock>^{B}*("//")?{B}*"*"+/[^//a-z_A-Z0-9*]    {
      // start of a comment line
                                          QString indent;
                                          indent.fill(' ',computeIndent(yytext,g_column));
                                          docBlock+=indent;
   }

<DocBlock>^{B}*("//"){B}*     { 
   // strip embedded C++ comments if at the start of a line
   }

<DocBlock>"//"             { 
      // slashes in the middle of a comment block
                    docBlock+=yytext;
   }

<DocBlock>"/*"             { 
      // */ (editor syntax fix)
      // start of a new comment in the middle of a comment block
      docBlock+=yytext;
   }

<DocBlock>("@@"|"\\\\"){ID}/[^a-z_A-Z0-9] { 
      // */ (editor syntax fix)
      // escaped command
      docBlock+=yytext;
   }

<DocBlock>{CMD}("f$"|"f["|"f{")          {
      // */ (editor syntax fix)

                  docBlock+=yytext;
                  docBlockName=&yytext[1];

                 if (docBlockName.at(1)=='{')
                 {
                    docBlockName[1]='}';
                 }

                 g_fencedSize=0;
                 g_nestedComment=false;
                 BEGIN(DocCopyBlock);
   }

<DocBlock>{B}*"<"{PRE}">"     {
                    docBlock+=yytext;
                    docBlockName="<pre>";
                    g_fencedSize=0;
                    g_nestedComment=false;
                    BEGIN(DocCopyBlock);
   }

<DocBlock>{CMD}("verbatim"|"latexonly"|"htmlonly"|"xmlonly"|"manonly"|"dot"|"code")/[^a-z_A-Z0-9]  { 
      // verbatim command (which could contain nested comments!)
                    docBlock+=yytext;
                    docBlockName=&yytext[1];
                    g_fencedSize=0;
                    g_nestedComment=false;
                    BEGIN(DocCopyBlock);
   }

<DocBlock>^({B}*"*"+)?{B}{0,3}"~~~"[~]*            {
                    docBlock+=yytext;
                    docBlockName="~~~";
                    g_fencedSize=yyleng;
                    g_nestedComment=false;
                    BEGIN(DocCopyBlock);
   }

<DocBlock>^({B}*"*"+)?{B}{0,3}"```"[`]*            {
                    docBlock += yytext;
                    docBlockName    = "```";
                    g_fencedSize    = yyleng;
                    g_nestedComment = false;
                    BEGIN(DocCopyBlock);
   }

<DocBlock>{B}*"<code>"                  {
                    if (insideCSharp)
                 {
                   docBlock+=yytext;
                   docBlockName="<code>";
                                            g_nestedComment=false;
                   BEGIN(DocCopyBlock);
                 }
                 else
                 {
                   REJECT;
                 }
   }

<DocBlock>[^@*~\/\\\n]+          { 
      // any character that isn't special
      docBlock+=yytext;
   }

<DocBlock>\n            { 
      // newline
      lineCount();
      docBlock+=*yytext;
   }

<DocBlock>.             { 
      // command block
      docBlock+=*yytext;
   }

 /* ---- Copy verbatim sections ------ */

<DocCopyBlock>"</"{PRE}">"       { 
      // end of a <pre> block
                    docBlock+=yytext;
                 if (docBlockName=="<pre>")
                 {
                      BEGIN(DocBlock);
                 }
   }

<DocCopyBlock>"</"{CODE}">"      { 
      // end of a <code> block
                    docBlock+=yytext;
                 if (docBlockName=="<code>")
                 {
                      BEGIN(DocBlock);
                 }
   }

<DocCopyBlock>[\\@]("f$"|"f]"|"f}")     {
      docBlock += yytext;
      BEGIN(DocBlock);
   }

<DocCopyBlock>[\\@]("endverbatim"|"endlatexonly"|"endhtmlonly"|"endxmlonly"|"enddocbookonly"|"endmanonly"|"enddot"|"endcode")/[^a-z_A-Z0-9] { 
      // end of verbatim block
                    docBlock+=yytext;
                 if (&yytext[4]==docBlockName)
                 {
                      BEGIN(DocBlock);
                 }
   }

<DocCopyBlock>^{B}*"*"+/{BN}+       { 
      // start of a comment line
                    if (docBlockName=="verbatim")
                 {
                   REJECT;
                 }
                    else if (docBlockName=="code")
                 {
                   REJECT;
                 }
                                          else
                                          {
                                            QString indent;
                                            indent.fill(' ',computeIndent(yytext,0));
                                            docBlock+=indent;
                                          }
   }

<DocCopyBlock>^{B}*"*"+/{BN}+"*"{BN}*  { 
      // start of a comment line with two *'s
                    if (docBlockName=="code")
                                          {
                                            QString indent;
                                            indent.fill(' ',computeIndent(yytext,0));
                                            docBlock+=indent;
                                          }
                                          else
                 {
                   REJECT;
                 }
   }

<DocCopyBlock>^{B}*"*"+/({ID}|"(")  { 
      // Assume *var or *(... is part of source code (see bug723516)
                 if (docBlockName=="code")
                                          {
                                            QString indent;
                                            indent.fill(' ',computeIndent(yytext,-1));
                                            docBlock+=indent+"*";
                                          }
                                          else
                                          {
                                            REJECT;
                                          }
   }

<DocCopyBlock>^{B}*"*"+/{BN}*       { 
      // start of a comment line with one *
                    if (docBlockName=="code")
                                          {
                                            QString indent;
                                            if (g_nestedComment) // keep * it is part of the code
                                            {
                                              indent.fill(' ',computeIndent(yytext,-1));
                                              docBlock+=indent+"*";
                                            }
                                            else // remove * it is part of the comment block
                                            {
                                              indent.fill(' ',computeIndent(yytext,0));
                                              docBlock+=indent;
                                            }
                                          }
                                          else
                 {
                   REJECT;
                 }
   }

<DocCopyBlock>^({B}*"*"+)?{B}{0,3}"~~~"[~]*        {
                         docBlock+=yytext;
                         if (g_fencedSize==yyleng)
                         {
                           BEGIN(DocBlock);
                         }
   }

<DocCopyBlock>^({B}*"*"+)?{B}{0,3}"```"[`]*        {
                          docBlock+=yytext;
                          if (g_fencedSize==yyleng) {
                              BEGIN(DocBlock);
                          }
   }

<DocCopyBlock>[^\<@/*\]~\$\\\n]+    { 
      // any character that is not special
      docBlock+=yytext;
   }

<DocCopyBlock>"/*"|"*/"|"//"     {
      // */ (editor syntax fix)
                                          if (yytext[1]=='*')
                                          {
                                            g_nestedComment=true;
                                          }
                                          else if (yytext[0]=='*')
                                          {
                                            g_nestedComment=false;
                                          }
                    docBlock+=yytext;
   }

<DocCopyBlock>\n        { 
      // newline
      docBlock+=*yytext;
      lineCount();
   }

<DocCopyBlock>.            { 
      // any other character
      docBlock+=*yytext;
   }

<DocCopyBlock><<EOF>>         {
      warn(yyFileName,yyLineNr, "reached end of file while inside a %s block\n"
         "The command that should end the block seems to be missing\n", qPrintable(docBlockName)); 
      yyterminate();
   }


    /* ------------- Prototype parser -------------- */

<Prototype>"operator"{B}*"("{B}*")"    {
                    current->name+=yytext;
   }

<Prototype>"("                 {
                    current->args+=*yytext;
                 currentArgumentContext = PrototypeQual;
                 fullArgString = current->args;
                 copyArgString = &current->args;
                 BEGIN( ReadFuncArgType ) ;
   }

<Prototype>"("({ID}"::")*({B}*[&*])+   {
                    current->type+=current->name + QString(yytext);
                 current->name.resize(0);
                    BEGIN( PrototypePtr );
   }

<PrototypePtr>{SCOPENAME}     {
                    current->name+=yytext;
   }

<PrototypePtr>"("          {
                    current->args+=*yytext;
                 currentArgumentContext = PrototypeQual;
                 fullArgString = current->args;
                 copyArgString = &current->args;
                 BEGIN( ReadFuncArgType ) ;
   }

<PrototypePtr>")"          {
                    current->type+=')';
                    BEGIN( Prototype );
   }

<PrototypePtr>.            {
                    current->name+=yytext;
   }

<PrototypeQual>"{"         {
                    BEGIN( PrototypeSkipLine);
   }

<PrototypeQual>{B}*"const"{B}*      {
                    current->args += " const ";
                 current->argList.constSpecifier=true;
   }

<PrototypeQual>{B}*"volatile"{B}*   {
                    current->args += " volatile ";
                 current->argList.volatileSpecifier=true;
   }

<PrototypeQual>{B}*"="{B}*"0"{B}*   {
                 current->args += " = 0";
                 current->virt = Pure;
                 current->argList.pureSpecifier=true;
   }

<PrototypeQual>"throw"{B}*"("            {
                    current->exception = "throw(";
                 BEGIN(PrototypeExc);
   }

<PrototypeExc>")"          {
                    current->exception += ')';
                 BEGIN(PrototypeQual);
   }

<PrototypeExc>.                 {
                    current->exception += *yytext;
   }

<PrototypeQual>.        {
                    current->args += *yytext;
   }

<Prototype>.                           {
                    current->name += *yytext;
   }

<PrototypeSkipLine>.          {
   }



  /* ------------ Generic rules -------------- */

<SkipCxxComment>.*"\\\n"      {  
      // line continuation
      if (insideCSharp)
                 {
                   REJECT;
                 }
                 else
                 {
                   lineCount();
                 }
   }

<SkipCxxComment>.*/\n         {
                 BEGIN( lastCContext ) ;
   }

<SkipComment>[^\*\n]+
<*>\n                { 
      lineCount(); 
   }

<*>\"    {
      if (insideIDL && insideCppQuote) {
         BEGIN(EndCppQuote);
      }
   }

<*>"#"                    {
                    if (!insidePHP)
                      REJECT;
                    lastCContext = YY_START ;
                 BEGIN( SkipCxxComment ) ;
   }

<*>\'                {
                    if (insidePHP)
                 {
                      lastStringContext=YY_START;
                   BEGIN(SkipPHPString);
                 }
   }

<*>\"                {
      if (insidePHP) {
         lastStringContext=YY_START;
         BEGIN(SkipString);
      }
   }

<*>.
<SkipComment>"//"|"/*"
<*>"/*"          { 
      // */ (editor syntax fix)
      lastCContext = YY_START ;
      BEGIN( SkipComment ) ;
   }

<SkipComment>{B}*"*/"         { 
      BEGIN( lastCContext ) ; 
   }

<*>"//"        {
      lastCContext = YY_START ;
      BEGIN( SkipCxxComment ) ;
   }

%%

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

static void newEntry()
{
   if (tempEntry == 0) {
      // if temp entry is not 0 it holds current and current is actually
      // replaced by previous which was already added to current_root
      // so we should not add it again  (see bug723314)

      current_root->addSubEntry(current, current_root);
   }

   tempEntry = QSharedPointer<Entry>();
   previous  = current;

   current = QMakeShared<Entry>();
   initEntry();
}

static void handleCommentBlock(const QString &doc, bool brief)
{
   static bool hideInBodyDocs = Config::getBool("hide-in-body-docs");
   
   int position = 0;
   bool needsEntry = false;
   
   if (docBlockInBody && hideInBodyDocs) {
      return;
   }

   if (doc.trimmed().isEmpty()) {
      return;
   }
   
   // line of block start
   int lineNr = brief ? current->briefLine : current->docLine;   
   
   // fill in inbodyFile && inbodyLine the first time, see bug 633891  
   QSharedPointer<Entry> docEntry = docBlockInBody && previous ? previous : current;
   
   if (docBlockInBody && docEntry && docEntry->inbodyLine == -1) {
      docEntry->inbodyFile = yyFileName;
      docEntry->inbodyLine = lineNr;
   }
   
   while (parseCommentBlock(g_thisParser, docBlockInBody && previous ? previous : current,
      stripIndentation(doc), yyFileName,                                  
      lineNr,                                       // line of block start
      docBlockInBody ? false : brief,               // isBrief
      docBlockInBody ? false : docBlockAutoBrief,   // isJavaDocStyle
      docBlockInBody,                               // isInBody
      protection, position, needsEntry) ) {

      if (needsEntry) {
         QString docFile = current->docFile;

         newEntry();
         current->docFile = docFile;
         current->docLine = lineNr;
      }
   }

   if (needsEntry) {
      newEntry();
   }
   
   if (docBlockTerm) {
      unput(docBlockTerm);
      docBlockTerm = 0;
   }
}

static void handleParametersCommentBlocks(ArgumentList *al)
{
   for (auto &a : *al) {

      if (! a.docs.isEmpty()) {
         int position = 0;
         bool needsEntry;

         // save context
         QString orgDoc    = current->doc;
         QString orgBrief  = current->brief;
         int orgDocLine    = current->docLine;
         int orgBriefLine  = current->briefLine;

         current->doc.resize(0);
         current->brief.resize(0);

         while (parseCommentBlock(g_thisParser, current,
                   a.docs,             // text
                   yyFileName,         // file
                   current->docLine,   // line of block start
                   false, false, false, protection, position, needsEntry ) ) {

            if (needsEntry) {
               newEntry();
            }
         }

         if (needsEntry) {
            newEntry();
         }

         a.docs = current->doc;

         // restore context
         current->doc       = orgDoc;
         current->brief     = orgBrief;
         current->docLine   = orgDocLine;
         current->briefLine = orgBriefLine;
      }
   }
}

static void  parseCompounds(QSharedPointer<Entry> rt)
{
   for (auto ce : rt->children() ) {

      if (! ce->program.isEmpty()) {

         padCount = 0;
         g_column = 0;

         inputString   = ce->program;
         inputPosition = 0;

         yyrestart( yyin );

         if (ce->section == Entry::ENUM_SEC || (ce->m_specFlags.spec & Entry::Enum)) {
            BEGIN(FindFields);
         } else {
            BEGIN(FindMembers);
         }

         current_root = ce;
         yyFileName   = ce->fileName;

         yyLineNr   = ce->startLine ;
         yyColNr    = ce->startColumn ;
         insideObjC = ce->lang == SrcLangExt_ObjC;

         current = QMakeShared<Entry>();
         gstat = false;
         initEntry();

         // deep copy group list from parent (see bug 727732)
         if (rt->groups) {
            for (auto g : *rt->groups) {
               ce->groups->append(g);
            }
         }

         int ni = ce->name.lastIndexOf("::");
         if (ni == -1) {
            ni = 0;

         } else {
            ni += 2;
         }

         // set default protection based on the compound type
         if ( ce->section == Entry::CLASS_SEC ) {
            // class

            if (insidePHP || insideD || insideJS || insideIDL) {
               current->protection = protection = Public ;

            } else if (insideJava) {
               current->protection = protection = (ce->m_specFlags.spec & (Entry::Interface | Entry::Enum)) ? Public : Package;

            } else if (ce->m_specFlags.spec & (Entry::Interface | Entry::Ref | Entry::Value | Entry::Struct | Entry::Union)) {
               if (ce->lang == SrcLangExt_ObjC) {
                  current->protection = protection = Protected;

               } else {
                  current->protection = protection = Public;
               }

            } else {
               current->protection = protection = Private;
            }

         } else if (ce->section == Entry::ENUM_SEC ) {
            // enum
            current->protection = protection = ce->protection;

         } else if (! ce->name.isEmpty() && ce->name.at(ni) == '@') {
            // unnamed union or namespace

            if (ce->section == Entry::NAMESPACE_SEC ) {
               // unnamed namespace
               current->stat = gstat = true;
            }
            current->protection = protection = ce->protection;

         } else {
            // named struct, union, protocol, category
            current->protection = protection = Public ;
         }

         mtype = Method;
         virt = Normal;

         groupEnterCompound(yyFileName, yyLineNr, ce->name);

         yylex() ;
         g_lexInit = true;

         groupLeaveCompound(yyFileName, yyLineNr, ce->name);
         current = QSharedPointer<Entry>();

         ce->program.resize(0);
      }

      parseCompounds(ce);
   }
}

static void parseMain(const QString &fileName, const QString &fileBuf, QSharedPointer<Entry> rt,
                      enum ParserMode mode, QStringList &includedFiles, bool useClang)
{
   initParser();

   inputString   = fileBuf;
   inputPosition = 0;
   g_column      = 0;

   protection    = Public;
   mtype         = MethodTypes::Method;
   gstat         = false;
   virt          = Normal;
   current_root  = rt;
   global_root   = rt;

   inputFile.setFileName(fileName);

   if (inputFile.open(QIODevice::ReadOnly)) {
      yyLineNr   = 1 ;
      yyFileName = fileName;

      setContext();

      if (useClang && (insideCpp || insideObjC) ) {
         // user wants clang for parsing
         s_clangParser = true;

         if (mode == ParserMode::SOURCE_FILE) {
            // new file
            ClangParser::instance()->start(fileName, includedFiles);

         } else {
            ClangParser::instance()->switchToFile(fileName);

         }
      } else  {
         s_clangParser = false;
      }
      
      rt->lang = language;
      current_root = rt ;
        
      initParser();
      groupEnterFile(yyFileName, yyLineNr);

      current = QMakeShared<Entry>();     
      int sec = guessSection(yyFileName);

      if (sec) {
         current->name    = yyFileName;
         current->section = sec;

         current_root->addSubEntry(current, current_root);
         current = QMakeShared<Entry>();
      }

      current->reset();

      initEntry();
      yyrestart( yyin );

      if ( insidePHP ) {
         BEGIN( FindMembersPHP );
      } else {
         BEGIN( FindMembers );
      }

      yylex();
      g_lexInit = true;


      if (YY_START == Comment) {
         warn(yyFileName, yyLineNr, "File ended in the middle of a comment block, Check for a missing \\endcode");
      }
    
      groupLeaveFile(yyFileName, yyLineNr);
      rt->program.resize(0);

      if (rt->children().contains(current) == 0)  {                     
         current = QSharedPointer<Entry>();
      }

      parseCompounds(rt);
      inputFile.close();

      anonNSCount++;
   } 
}

static void parsePrototype(const QString &text)
{
   if (text.isEmpty()) {
      warn(yyFileName,yyLineNr,"Empty prototype found");
      return;
   }

   if (! current) { 
      // nothing to store  
      return;
   }
   
   QString orgInputString;
   int orgInputPosition;
   
   YY_BUFFER_STATE orgState;
   
   // save scanner state
   orgState = YY_CURRENT_BUFFER;
   yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
   orgInputString   = inputString;
   orgInputPosition = inputPosition;
   
   // set new string
   inputString   = text;
   inputPosition = 0;
   g_column = 0;
   
   yyrestart( yyin );
   BEGIN(Prototype);
   
   yylex();
   g_lexInit = true;
   
   current->name = current->name.trimmed();
   if (current->section == Entry::MEMBERDOC_SEC && current->args.isEmpty())
    current->section = Entry::VARIABLEDOC_SEC;
   
   // restore original scanner state
   YY_BUFFER_STATE tmpState = YY_CURRENT_BUFFER;
   yy_switch_to_buffer(orgState);
   yy_delete_buffer(tmpState);
   
   inputString = orgInputString;
   inputPosition = orgInputPosition;
}

void CPPScanFreeParser()
{
   if (g_lexInit) {
      yylex_destroy();
   }
}

void CPPLanguageParser::startTranslationUnit(const QString &)
{
}

void CPPLanguageParser::finishTranslationUnit()
{
   bool processWithClang = insideCpp || insideObjC;
   
   if (processWithClang) {
      ClangParser::instance()->finish();
   }
}

void CPPLanguageParser::parseInput(const QString &fileName, const QString &fileBuf,
                  QSharedPointer<Entry> root, enum ParserMode mode,
                  QStringList & filesInSameTranslationUnit, bool useClang)
{
   g_thisParser = this;
   
   printlex(yy_flex_debug, true, __FILE__, fileName);
   
   ::parseMain(fileName,fileBuf,root, mode, filesInSameTranslationUnit, useClang);
   
   printlex(yy_flex_debug, false, __FILE__, fileName);
}

void CPPLanguageParser::parseCode(CodeOutputInterface &codeOutIntf, const QString &scopeName, const QString &input,
                  SrcLangExt lang, bool isExampleBlock, const QString &exampleName, QSharedPointer<FileDef> fileDef,
                  int startLine, int endLine, bool inlineFragment, QSharedPointer<MemberDef> memberDef,
                  bool showLineNumbers, QSharedPointer<Definition> searchCtx, bool collectXRefs )
{
   ::parseCCode(codeOutIntf,scopeName, input,lang,isExampleBlock,exampleName, fileDef,startLine, endLine,
                  inlineFragment,memberDef, showLineNumbers,searchCtx,collectXRefs);
}

bool CPPLanguageParser::needsPreprocessing(const QString &extension)
{
   QString fe=extension.toLower();
   SrcLangExt lang = getLanguageFromFileName(extension);

   return (SrcLangExt_Cpp == lang) ||
                  ! ( fe == ".java" || fe == ".as"  || fe == ".d"    || fe == ".php" ||
                      fe == ".php4" || fe == ".inc" || fe == ".phtml");
}

void CPPLanguageParser::resetCodeParserState()
{
   ::resetCCodeParserState();
}

void CPPLanguageParser::parsePrototype(const QString &text)
{
   ::parsePrototype(text);
}
