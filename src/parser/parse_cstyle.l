/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim
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
#include <code_cstyle.h>
#include <default_args.h>
#include <doxy_globals.h>
#include <entry.h>
#include <language.h>
#include <message.h>
#include <parse_clang.h>
#include <parse_cstyle.h>
#include <util.h>

#define YY_NO_INPUT 1
#define YY_NEVER_INTERACTIVE 1

static ParserInterface *s_thisParser;
static QString          s_inputString;
static QFile            s_inputFile;
static int              s_inputPosition;

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
static int              lastC11AttributeContext;

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

static QString          msType;
static QString          msName;
static QString          msArgs;

static bool             isTypedef;
static int              tmpDocType;
static QString          sectionLabel;
static QString          sectionTitle;
static QString          funcPtrType;
static QString          aliasName;
static QString          baseName;
static QString         *specName;
static QString          formulaText;
static bool             useOverrideCommands = false;

static SrcLangExt       language;

static bool             insideIDL    = false;         // processing IDL code
static bool             insideJava   = false;         // processing Java code
static bool             insideCSharp = false;         // processing C# code
static bool             insideD      = false;         // processing D code
static bool             insidePHP    = false;         // processing PHP code
static bool             insideObjC   = false;         // processing Objective C code
static bool             insideCli    = false;         // processing C++/CLI code
static bool             insideJS     = false;         // processing JavaScript code
static bool             insideCpp    = true;          // processing C/C++ code

static bool             insideCppQuote     = false;
static bool             insideProtocolList = false;

static int              argRoundCount;
static int              argSharpCount;
static int              currentArgumentContext;
static int              lastCopyArgStringContext;
static int              lastCopyArgContext;

static QString          fullArgString;
static ArgumentList     *currentArgumentList;
static QChar            lastCopyArgChar;

enum class ArgKey {
   Entry_Name,
   Member_Args,
   Template_Args,
};

static QString                s_template_args;

static QSharedPointer<Entry>  s_argEntry;         // which entry
static ArgKey                 s_argEnum;          // which enum in EntryKey

static QSharedPointer<Entry>  s_quotedEntry;      // which entry
static EntryKey               s_quotedEnum;       // which enum in EntryKey

static QSharedPointer<Entry>  s_roundEntry;       // which entry
static EntryKey               s_roundEnum;        // which enum in EntryKey

static QSharedPointer<Entry>  s_curlyEntry;       // which entry
static EntryKey               s_curlyEnum;        // which enum in EntryKey

static QSharedPointer<Entry>  s_rawEntry;         // which entry
static EntryKey               s_rawEnum;          // which enum in EntryKey

static QSharedPointer<Entry>  s_quotedGEntry;     // which entry
static EntryKey               s_quotedGEnum;      // which enum in EntryKey

static QSharedPointer<Entry>  s_roundGEntry;      // which entry
static EntryKey               s_roundGEnum;       // which enum in EntryKey

static QSharedPointer<Entry>  s_curlyGEntry;      // which entry
static EntryKey               s_curlyGEnum;       // which enum in EntryKey

static QSharedPointer<Entry>  s_rawGEntry;        // which entry
static EntryKey               s_rawGEnum;         // which enum in EntryKey

static QSharedPointer<Entry>  s_hereDocEntry;     // which entry
static EntryKey               s_hereDocEnum;      // which enum in EntryKey

static QSharedPointer<Entry>  s_skipVerbEntry;    // which entry
static EntryKey               s_skipVerbEnum;     // which enum in EntryKey

static bool             insideFormula;
static bool             insideTryBlock = false;
static bool             insideCode;
static bool             needsSemi;
static int              initBracketCount;

static QString          oldStyleArgType;
static QString          s_mainBackup;
static QString          s_briefBackup;

static int              s_docBlockContext;
static QString          s_docBlock;
static QString          s_docBlockName;
static bool             s_docBlockInBody;
static bool             s_docBlockAutoBrief;
static char             s_docBlockTerm;

static QString          idlAttr;
static QString          idlProp;
static bool             odlProp;

static bool             s_lexInit = false;
static bool             externC;

static QString          s_delimiter;

static int              s_column;
static int              s_fencedSize = 0;
static bool             s_nestedComment = 0;

static QStack<Grouping *>     autoGroupStack;

// double declared
static void yyunput (int c, char *buf_ptr);

static void yyunput (QChar c, char *yy_bp )
{
   QString tmp1    = c;
   QByteArray tmp2 = tmp1.toUtf8();

   for (int i = tmp2.length() - 1; i >= 0;  i--) {
      unput(tmp2[i]);
   }
}

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

   insideCli      = Config::getBool("cpp-cli-support");

   previous          = QSharedPointer<Entry>();
   firstTypedefEntry = QSharedPointer<Entry>();
   tempEntry         = QSharedPointer<Entry>();
   memspecEntry      = QSharedPointer<Entry>();
}

static void initEntry()
{
   if (insideJava)  {
      bool isEnum      = current_root->m_traits.hasTrait(Entry::Virtue::Enum);
      bool isInterface = current_root->m_traits.hasTrait(Entry::Virtue::Interface);

      protection = (isEnum || isInterface) ? Public : Package;
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
//   Grouping *g = current->m_groups.first();
//   int i = 0;
//   while (g)
//   {
//     if (g->pri <= Grouping::GROUPING_AUTO_DEF)
//     {
//       current->m_groups.remove(i);
//       i--;
//     }
//     g=current->m_groups.next();
//     i++;
//   }
//
//   /* use new group name instead? */
//   if ( newgroup )
//   {
//      current->m_groups.append(new Grouping(*newgroup, pri));
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

  for (p = yytext; *p; ++p) {

      if (*p == '\n') {
         yyLineNr++;
         s_column = 0;
         yyColNr  = 1;

      } else if (*p == '\t') {
         s_column += tabSize - (s_column % tabSize);

      } else {
         s_column++;
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
   QString tmpType = ce->getData(EntryKey::Member_Type);

   if (! tmpType.isEmpty() && ! ce->name.isEmpty() && ! tmpType.endsWith('.')) {
      tmpType += ' ' ;
   }

   tmpType += ce->name;
   ce->name.clear();

   if (! tmpType.isEmpty() && ! ce->getData(EntryKey::Member_Args).isEmpty() && ! tmpType.endsWith('.'))  {
      tmpType += ' ';
   }

   ce->setData(EntryKey::Member_Type, tmpType + ce->getData(EntryKey::Member_Args));
   ce->setData(EntryKey::Member_Args, "");
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
static void handleParametersCommentBlocks(ArgumentList &al);

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

static void setContext(const QString &fileName)
{
  language     = getLanguageFromFileName(fileName);

  insideIDL    = language == SrcLangExt_IDL;
  insideJava   = language == SrcLangExt_Java;
  insideCSharp = language == SrcLangExt_CSharp;
  insideD      = language == SrcLangExt_D;
  insidePHP    = language == SrcLangExt_PHP;
  insideObjC   = language == SrcLangExt_ObjC;
  insideJS     = language == SrcLangExt_JS;
  insideCpp    = language == SrcLangExt_Cpp;

  if (insidePHP) {
    useOverrideCommands = true;
  }
}

static void prependScope()
{
   if (current_root->section & Entry::SCOPE_MASK) {
      current->name.prepend(current_root->name + "::");

      if (! current_root->m_templateArgLists.isEmpty()) {
         current->m_templateArgLists = current_root->m_templateArgLists;
      }
   }
}

static bool containsWord(const QString &str, const QString &word)
{
   QRegExp wordExp("\\b"+word+"\\b");
   return str.contains(wordExp);
}

/*! Returns true if the current entry could be a K&R style C function */
static bool checkForKnRstyleC()
{
   if (yyFileName.right(2).toLower() != ".c") {
      // must be a C file
      return false;
   }

   if (current->argList.listEmpty()) {
      // must have arguments
      return false;
   }

   for (const auto &a : current->argList) {
      // K&R style arguments do not have a type
      // DoxyPress expects a type so it will think the argument has no name

      if (a.type.isEmpty() || ! a.name.isEmpty()) {
         return false;
      }
   }

   return true;
}

static void splitKnRArg(QString &oldStyleArgPtr, QString &oldStyleArgName)
{
   QString tmpArgs = current->getData(EntryKey::Member_Args);
   int lenArgs     = tmpArgs.length();

   if (oldStyleArgType.isEmpty()) {
      // new argument
      static QRegExp re("\\([^)]*\\)");

      int bi1 = re.lastIndexIn(tmpArgs);
      int bi2 = bi1 != -1 ? re.lastIndexIn(tmpArgs, bi1 - 1) : -1;

      QChar c;

      if (bi1 != -1 && bi2 != -1) {
         // found something like "int (*func)(int arg)"

         int pos = bi2 + 1;
         oldStyleArgType = tmpArgs.left(pos);

         int i = pos;

         while (i < lenArgs && ((c = tmpArgs.at(i)) == '*' || c.isSpace() )) {
            i++;
         }

         oldStyleArgType += tmpArgs.mid(pos, i - pos);
         pos = i;

         while (i < lenArgs && isId(tmpArgs.at(i))) {
            i++;
         }

         oldStyleArgName = tmpArgs.mid(pos, i - pos);
         oldStyleArgType += tmpArgs.mid(i);

      } else if (bi1 != -1) {
         // redundant braces like in "int (*var)"
         int pos = bi1;

         oldStyleArgType = tmpArgs.left(pos);
         pos++;

         int i = pos + 1;
         while (i < lenArgs && ((c = tmpArgs.at(i)) == '*' || c.isSpace() )) {
            i++;
         }

         oldStyleArgType += tmpArgs.mid(pos, i - pos);
         pos = i;

         while (i < lenArgs && isId(tmpArgs.at(i))) {
            i++;
         }

         oldStyleArgName = tmpArgs.mid(pos, i - pos);

      } else {
         // normal "int *var"
         int len = lenArgs;
         int i   = len - 1;
         int j;

         QChar c;

         // look for start of name in "type *name"
         while (i >= 0 && isId(tmpArgs.at(i))) {
            i--;
         }

         j = i + 1;

         // look for start of *'s
         while (i >= 0 && ((c = tmpArgs.at(i)) == '*' || c.isSpace() )) {
            i--;
         }

         i++;

         if (i != len) {
            oldStyleArgType = tmpArgs.left(i);
            oldStyleArgPtr  = tmpArgs.mid(i, j - i);
            oldStyleArgName = tmpArgs.mid(j).trimmed();

         } else {
            oldStyleArgName = tmpArgs.trimmed();
         }
      }

   } else {
      // continuation like *arg2 in "int *args,*arg2"
      int len = lenArgs;
      int j   = 0;

      QChar c;

      while (j < len && ((c = tmpArgs.at(j)) == '*' || c.isSpace() )) {
         j++;
      }

      if (j > 0) {
         oldStyleArgPtr  = tmpArgs.left(j);
         oldStyleArgName = tmpArgs.mid(j).trimmed();

      } else {
         oldStyleArgName = tmpArgs.trimmed();
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

static void addToArgs(const QString &str)
{
   if (s_argEnum == ArgKey::Entry_Name) {
      s_argEntry->name += str;

   } else if (s_argEnum == ArgKey::Template_Args) {
      s_template_args += str;

   } else if (s_argEnum == ArgKey::Member_Args) {
      s_argEntry->appendData(EntryKey::Member_Args, str);

   }
}

static void addToOutput(QSharedPointer<Entry> entry, EntryKey key, const QString &str)
{
   if (entry == nullptr)  {
      addToArgs(str);

   } else {
      entry->appendData(key, str);
   }
}

static void addToOutput(QSharedPointer<Entry> entry, EntryKey key, QChar c)
{
   if (entry == nullptr)  {
      addToArgs(c);
   } else {
      entry->appendData(key, c);
   }
}

static void addToArgs(QChar str)
{
   if (s_argEnum == ArgKey::Entry_Name) {
      s_argEntry->name += str;

   } else if (s_argEnum == ArgKey::Template_Args) {
      s_template_args += str;

   } else if (s_argEnum == ArgKey::Member_Args) {
      s_argEntry->appendData(EntryKey::Member_Args, str);

   }
}

void fixArgumentListForJavaScript(ArgumentList &argList)
{
   for (auto &a : argList) {
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

   while (s_inputString[s_inputPosition] != 0) {

      QString tmp1    = s_inputString.at(s_inputPosition);
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

      s_inputPosition++;
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

CS_MACRO     ("CS_OBJECT_MULTIPLE"|"CS_OBJECT_INTERNAL"|"CS_GADGET"|"CS_GADGET_INTERNAL"|"CS_CLASSINFO"|"CS_INTERFACES"|"CS_ENUM"|"CS_FLAG"|"CS_INVOKABLE_CONSTRUCTOR_1"|"CS_INVOKABLE_CONSTRUCTOR_2"|("CS_DECLARE"[_A-Z0-9]+)|("Q_DECLARE"[_A-Z0-9]+)|"Q_DISABLE_COPY"|"Q_ENUM"|"Q_ENUMS"|"Q_INTERFACES"|"K_DECLARE_PRIVATE"|"K_D"|"PHONON_HEIR")

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
%x SkipC11Attribute
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
      curlyCount = 0;
      needsSemi  = true;
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

      } else {
       BEGIN( FindMembers );
      }
   }

<NextSemi>\'     {
      if (insidePHP) {
         lastStringContext = NextSemi;
         BEGIN(SkipPHPString);
      }
   }

<NextSemi>{CHARLIT}        {
      if (insidePHP) {
         REJECT;
      }
   }

<NextSemi>\"            {
      lastStringContext = NextSemi;
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

<EnumBaseType>[{;,]   {
      current->setData(EntryKey::Member_Args,  current->getData(EntryKey::Member_Args).simplified());

      unput(*yytext);
      BEGIN( ClassVar );
   }

<FindMembers>"<?php"   {
      // PHP code with unsupported extension?
      insidePHP = true;
   }

<FindMembersPHP>"<?"("php"?)  {
      // PHP code start
      BEGIN( FindMembers );
   }

<FindMembersPHP>"<script"{BN}+"language"{BN}*"="{BN}*['"]?"php"['"]?{BN}*">" {
      // PHP code start
      lineCount() ;
      BEGIN( FindMembers );
   }

<FindMembersPHP>[^\n<]+    {
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
      if (! insideIDL)
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

      current->mtype      = mtype = MethodTypes::Property;
      current->protection = protection = Public;

      current->name.resize(0);
      current->setData(EntryKey::Member_Type, "");
      current->setData(EntryKey::Member_Args, "");
      current->argList.clear();

      lineCount();
   }

<FindMembers>{B}*"k_dcop"{BN}*":"{BN}*  {
      current->mtype      = mtype = MethodTypes::DCOP;
      current->protection = protection = Public;

      current->name.resize(0);
      current->setData(EntryKey::Member_Type, "");
      current->setData(EntryKey::Member_Args, "");
      current->argList.clear();

      lineCount();
   }

<FindMembers>{B}*("signals"|"Q_SIGNALS"){BN}*":"{BN}* {
      current->mtype      = mtype = MethodTypes::Signal;
      current->protection = protection = Public ;

      current->name.resize(0);
      current->setData(EntryKey::Member_Type, "");
      current->setData(EntryKey::Member_Args, "");
      current->argList.clear();

      lineCount();
   }

<FindMembers>{B}*"public"{BN}*("slots"|"Q_SLOTS"){BN}*":"{BN}* {
      current->mtype      = mtype = MethodTypes::Slot;
      current->protection = protection = Public ;

      current->name.resize(0);
      current->setData(EntryKey::Member_Type, "");
      current->setData(EntryKey::Member_Args, "");
      current->argList.clear();

      lineCount();
   }

<FindMembers>{B}*"protected"{BN}*("slots"|"Q_SLOTS"){BN}*":"{BN}* {
      current->protection  = protection = Protected ;
      current->mtype       = mtype = MethodTypes::Slot;

      current->name.resize(0);
      current->setData(EntryKey::Member_Type, "");
      current->setData(EntryKey::Member_Args, "");
      current->argList.clear();

      lineCount();
   }

<FindMembers>{B}*"private"{BN}*("slots"|"Q_SLOTS"){BN}*":"{BN}* {
      current->protection = protection = Private ;
      current->mtype      = mtype = MethodTypes::Slot;

      current->name.resize(0);
      current->setData(EntryKey::Member_Type, "");
      current->setData(EntryKey::Member_Args, "");
      current->argList.clear();

      lineCount();
   }

<FindMembers>{B}*("public"|"methods"|"__published"){BN}*":"{BN}* {
      current->protection = protection = Public ;
      current->mtype      = mtype = MethodTypes::Method;

      current->name.resize(0);
      current->setData(EntryKey::Member_Type, "");
      current->setData(EntryKey::Member_Args, "");
      current->argList.clear();

      lineCount();
   }

<FindMembers>{B}*"internal"{BN}*":"{BN}* {
      // for now treat C++/CLI's internal as package...
      if (insideCli) {
         current->protection = protection = Package ;
         current->mtype      = mtype = MethodTypes::Method;

         current->name.resize(0);
         current->setData(EntryKey::Member_Type, "");
         current->setData(EntryKey::Member_Args, "");
         current->argList.clear();

         lineCount();

      } else {
         REJECT;
      }
   }

<FindMembers>{B}*"protected"{BN}*":"{BN}* {
      current->protection = protection = Protected ;
      current->mtype      = mtype = MethodTypes::Method;

      current->name.resize(0);
      current->setData(EntryKey::Member_Type, "");
      current->setData(EntryKey::Member_Args, "");
      current->argList.clear();

      lineCount();
   }

<FindMembers>{B}*"private"{BN}*":"{BN}*   {
      current->protection = protection = Private ;
      current->mtype      = mtype = MethodTypes::Method;

      current->name.resize(0);
      current->setData(EntryKey::Member_Type, "");
      current->setData(EntryKey::Member_Args, "");
      current->argList.clear();

      lineCount();
   }

<FindMembers>{B}*"event"{BN}+           {
      if (insideCli) {
         // C++/CLI event
         lineCount();

         current->mtype    = mtype = MethodTypes::Event;
         current->bodyLine = yyLineNr;
         curlyCount        = 0;

         BEGIN( CliPropertyType );

      } else if (insideCSharp) {
         lineCount();

         current->mtype    = MethodTypes::Event;
         current->bodyLine = yyLineNr;

      } else {
         REJECT;
      }
   }

<FindMembers>{B}*"property"{BN}+    {
      if (insideCli) {
         // C++/CLI property
         lineCount();
         current->mtype    = mtype = MethodTypes::Property;
         current->bodyLine = yyLineNr;
         curlyCount        = 0;

         BEGIN( CliPropertyType );

      } else {
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
      current->setData(EntryKey::Member_Args, "[");
      BEGIN( CliPropertyIndex );
   }

<CliPropertyType>"{"          {
      curlyCount = 0;
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
      current->appendData(EntryKey::Member_Type, text);
   }

<CliPropertyIndex>"]"         {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text);
      BEGIN( CliPropertyType );
   }

<CliPropertyIndex>.        {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text);
   }

  /*
<FindMembers>{B}*"property"{BN}+        {
      if (! current->getData(EntryKey::Member_Type).isEmpty()) {
         REJECT;
     } else {
         current->mtype = mtype = MethodTypes::Property;
         lineCount();
     }
   }
  */

<FindMembers>{B}*"@private"{BN}+    {
      current->protection = protection = Private ;
      current->mtype      = mtype = MethodTypes::Method;

      current->name.resize(0);
      current->setData(EntryKey::Member_Type, "");
      current->setData(EntryKey::Member_Args, "");
      current->argList.clear();

      lineCount();
   }

<FindMembers>{B}*"@protected"{BN}+  {
      current->protection = protection = Protected ;
      current->mtype      = mtype = MethodTypes::Method;

      current->name.resize(0);
      current->setData(EntryKey::Member_Type, "");
      current->setData(EntryKey::Member_Args, "");
      current->argList.clear();

      lineCount();
   }

<FindMembers>{B}*"@public"{BN}+  {
      current->protection = protection = Public ;
      current->mtype      = mtype = MethodTypes::Method;

      current->name.resize(0);
      current->setData(EntryKey::Member_Type, "");
      current->setData(EntryKey::Member_Args, "");
      current->argList.clear();

      lineCount();
   }

<FindMembers>[\-+]{BN}*          {
      QString text = QString::fromUtf8(yytext);

      if (! insideObjC) {
         REJECT;

      } else {
         lineCount();

         current->setData(EntryKey::File_Name, yyFileName);
         current->startLine   = yyLineNr;
         current->startColumn = yyColNr;

         current->bodyLine    = yyLineNr;
         current->section     = Entry::FUNCTION_SEC;
         current->protection  = protection = Public;

         language   = current->lang = SrcLangExt_ObjC;
         insideObjC = true;

         current->virt  = Specifier::Virtual;
         current->stat  = (text[0]=='+');
         current->mtype = mtype = MethodTypes::Method;

         current->name.resize(0);
         current->setData(EntryKey::Member_Type, "");
         current->setData(EntryKey::Member_Args, "");
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

      if (current->getData(EntryKey::Member_Type).isEmpty()) {
         current->setData(EntryKey::Member_Type, "id");
      }

      current->name = text;
      if (insideCpp || insideObjC) {
         current->setData(EntryKey::Clang_Id, ClangParser::instance()->lookup(yyLineNr, text));
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
      current->setData(EntryKey::Member_Type, text);
   }

<ObjCReturnType>")"        {
      BEGIN( ObjCMethod );
   }

<ObjCParams>({ID})?":"        {
      // Keyword of parameter
      QString text = QString::fromUtf8(yytext);
      text = text.left(text.length() - 1);

      if (text.isEmpty()) {
         current->name += " :";

      } else {
         current->name += text + ":";
      }

      if (current->argList.last().type.isEmpty()) {
         current->argList.last().type = "id";
      }

      Argument a;
      a.attrib = "[" + text + "]";
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
      a.attrib = "[,]";
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
      current->argList.last().type += text;
   }

<ObjCParamType>")"/{B}*          {
      QString text = QString::fromUtf8(yytext);

      if (roundCount <= 0) {
         BEGIN( ObjCParams );
      } else {
         current->argList.last().type += text;
         roundCount--;
      }
   }

<ObjCParamType>[^()]*         {
      QString text = QString::fromUtf8(yytext);
      current->argList.last().type += text.trimmed();
   }

<ObjCMethod,ObjCParams>";"       {
      // end of method declaration
      if (! current->argList.listEmpty() && current->argList.last().type.isEmpty()) {
         current->argList.last().type = "id";
      }

      current->setData(EntryKey::Member_Args, argListToString(current->argList));

      unput(';');
      BEGIN( Function );
   }

<ObjCMethod,ObjCParams>(";"{BN}+)?"{"  {
      // start of a method body
      lineCount();

      if (! current->argList.listEmpty() && current->argList.last().type.isEmpty()) {
         current->argList.last().type = "id";
      }

      current->setData(EntryKey::Member_Args, argListToString(current->argList));

      unput('{');
      BEGIN( Function );
   }

<FindMembers>{BN}{1,80}               {
      lineCount();
   }

<FindMembers>"@"({ID}".")*{ID}{BN}*"("    {
      QString text = QString::fromUtf8(yytext);
      if (insideJava) {
         // Java annotation
         lineCount();
         lastSkipRoundContext = YY_START;
         roundCount = 0;
         BEGIN( SkipRound );

      } else if (text.startsWith("@property")) {
         // ObjC 2.0 property

         current->mtype      = mtype = MethodTypes::Property;
         current->protection = Public;

         current->m_traits.setTrait(Entry::Virtue::Readable);
         current->m_traits.setTrait(Entry::Virtue::Writable);
         current->m_traits.setTrait(Entry::Virtue::Assign);

         unput('(');
         BEGIN( ObjCPropAttr );

      } else {
         REJECT;
      }
   }

<ObjCPropAttr>"getter="{ID}      {
      QString text = QString::fromUtf8(yytext);
      current->setData(EntryKey::Read_Property, text.mid(7));
   }

<ObjCPropAttr>"setter="{ID}      {
      QString text = QString::fromUtf8(yytext);
      current->setData(EntryKey::Write_Property, text.mid(7));
   }

<ObjCPropAttr>"readonly"      {
      current->m_traits.setTrait(Entry::Virtue::Writable, false);
   }

<ObjCPropAttr>"readwrite"     {
      // default
   }

<ObjCPropAttr>"assign"        {
      // default
   }

<ObjCPropAttr>"unsafe_unretained"       {
      current->m_traits.setTrait(Entry::Virtue::Assign, false);
      current->m_traits.setTrait(Entry::Virtue::Unretained);
   }

<ObjCPropAttr>"retain"        {
      current->m_traits.setTrait(Entry::Virtue::Assign, false);
      current->m_traits.setTrait(Entry::Virtue::Retain);
   }

<ObjCPropAttr>"copy"          {
      current->m_traits.setTrait(Entry::Virtue::Assign, false);
      current->m_traits.setTrait(Entry::Virtue::Copy);
   }

<ObjCPropAttr>"weak"                    {
      current->m_traits.setTrait(Entry::Virtue::Assign, false);
      current->m_traits.setTrait(Entry::Virtue::Weak);
   }

<ObjCPropAttr>"strong"                  {
      current->m_traits.setTrait(Entry::Virtue::Assign, false);
      current->m_traits.setTrait(Entry::Virtue::Strong);
   }

<ObjCPropAttr>"nonatomic"     {
      current->m_traits.setTrait(Entry::Virtue::NonAtomic);
   }

<ObjCPropAttr>")"          {
      BEGIN(FindMembers);
   }

<FindMembers>"@"{ID}          {
      QString text = QString::fromUtf8(yytext);

      if (insideJava)  {
         // Java annotaton
         // skip annotation

      }  else if (text == "@property") {
         // ObjC 2.0 property

         current->mtype = mtype = MethodTypes::Property;
         current->protection    = Public;
         current->m_traits.setTrait(Entry::Virtue::Readable);
         current->m_traits.setTrait(Entry::Virtue::Writable);

      } else if ( text == "@synthesize") {
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
      QString text         = QString::fromUtf8(yytext);
      isTypedef            = false;

      current->name        = text;
      current->name        = substitute(current->name,".","::");
      current->name        = substitute(current->name,"\\","::");

      current->section     = Entry::NAMESPACE_SEC;
      current->setData(EntryKey::Member_Type, "namespace");

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;
      current->bodyLine    = yyLineNr;

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
      current->appendData(EntryKey::Member_Type, "initonly ");

      if (insideCli)  {
         current->m_traits.setTrait(Entry::Virtue::Initonly);
      }

      lineCount();
   }

<FindMembers>{B}*"static"{BN}+      {
      current->appendData(EntryKey::Member_Type, " static ");
      current->stat = true;

      lineCount();
   }

<FindMembers>{B}*"extern"{BN}+      {
      current->stat = false;
      current->explicitExternal = true;
      lineCount();
   }

<FindMembers>{B}*"virtual"{BN}+     {

      current->appendData(EntryKey::Member_Type, " virtual ");
      current->virt = Virtual;
      lineCount();
   }

<FindMembers>{B}*"published"{BN}+   {
      // UNO IDL published keyword
      if (insideIDL) {
         lineCount();
         current->m_traits.setTrait(Entry::Virtue::Published);

      } else {
         REJECT;
      }
   }

<FindMembers>{B}*"abstract"{BN}+       {
      if (! insidePHP) {
         current->appendData(EntryKey::Member_Type, " abstract ");

         if (! insideJava) {
            current->virt = Pure;

         } else {
            current->m_traits.setTrait(Entry::Virtue::Abstract);
         }

      } else {
         current->m_traits.setTrait(Entry::Virtue::Abstract);
      }

      lineCount();
   }

<FindMembers>{B}*"inline"{BN}+      {
      current->m_traits.setTrait(Entry::Virtue::Inline);
      lineCount();
   }

<FindMembers>{B}*"mutable"{BN}+     {
      current->m_traits.setTrait(Entry::Virtue::Mutable);
      lineCount();
   }

<FindMembers>{B}*"explicit"{BN}+    {
      current->m_traits.setTrait(Entry::Virtue::Explicit);
      lineCount();
   }

<FindMembers>{B}*"@required"{BN}+   {
      // Objective C 2.0 protocol required section

      current->m_traits.setTrait(Entry::Virtue::Optional, false);
      current->m_traits.setTrait(Entry::Virtue::Required);

      lineCount();
   }

<FindMembers>{B}*"@optional"{BN}+   {
      // Objective C 2.0 protocol optional section

      current->m_traits.setTrait(Entry::Virtue::Optional);
      current->m_traits.setTrait(Entry::Virtue::Required, false);
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
      isTypedef            = false;
      current->section     = Entry::NAMESPACE_SEC;
      current->setData(EntryKey::Member_Type, "namespace");

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;
      current->bodyLine    = yyLineNr;

      lineCount();

      if (insidePHP) {
         BEGIN( PackageName );
      } else {
         BEGIN( CompoundName );
      }
   }

<FindMembers>{B}*"module"{BN}+      {
      lineCount();

      if (insideIDL) {
         isTypedef            = false;
         current->section     = Entry::NAMESPACE_SEC;
         current->setData(EntryKey::Member_Type, "module");

         current->setData(EntryKey::File_Name, yyFileName);
         current->startLine   = yyLineNr;
         current->startColumn = yyColNr;
         current->bodyLine    = yyLineNr;

         BEGIN( CompoundName );

      } else if (insideD) {
         lineCount();
         BEGIN(PackageName);

      } else {
         QString text = QString::fromUtf8(yytext);
         addType(current);
         current->name = text.trimmed();
      }
   }

<FindMembers>{B}*"library"{BN}+     {
      lineCount();

      if (insideIDL) {
         isTypedef = false;

         current->section     = Entry::NAMESPACE_SEC;
         current->setData(EntryKey::Member_Type, "library");

         current->setData(EntryKey::File_Name, yyFileName);
         current->startLine   = yyLineNr;
         current->startColumn = yyColNr;
         current->bodyLine    = yyLineNr;

         BEGIN( CompoundName );

      } else {
         QString text = QString::fromUtf8(yytext);
         addType(current);
         current->name = text.trimmed();
      }
   }

<FindMembers>{B}*"constants"{BN}+   {
      // UNO IDL constant group
      lineCount();

      if (insideIDL) {
         isTypedef = false;

         current->section     = Entry::NAMESPACE_SEC;
         current->setData(EntryKey::Member_Type, "constants");

         current->setData(EntryKey::File_Name, yyFileName);
         current->startLine   = yyLineNr;
         current->startColumn = yyColNr;
         current->bodyLine    = yyLineNr;

         BEGIN( CompoundName );

      } else {
         QString text = QString::fromUtf8(yytext);
         addType(current);
         current->name = text.trimmed();
      }
   }

<FindMembers>{BN}*("service"){BN}+     {
      // UNO IDL service
      lineCount();

      if (insideIDL) {
         isTypedef = false;
         current->section = Entry::CLASS_SEC;

         // preserve UNO IDL -optional and published
         bool isOptional  = current->m_traits.hasTrait(Entry::Virtue::Optional);
         bool isPublished = current->m_traits.hasTrait(Entry::Virtue::Published);

         current->m_traits.clear();
         current->m_traits.setTrait(Entry::Virtue::Service);

         current->m_traits.setTrait(Entry::Virtue::Optional,  isOptional);
         current->m_traits.setTrait(Entry::Virtue::Published, isPublished);

         addType(current);

         current->appendData(EntryKey::Member_Type, " service");
         current->setData(EntryKey::File_Name, yyFileName);

         current->startLine = yyLineNr;
         current->bodyLine  = yyLineNr;

         BEGIN( CompoundName );

      } else  {
         // TODO is addType right? just copy/pasted

         QString text = QString::fromUtf8(yytext);
         addType( current ) ;
         current->name = text.trimmed();
      }
   }

<FindMembers>{BN}*("singleton"){BN}+   {
      // UNO IDL singleton
      lineCount();

      if (insideIDL) {
         isTypedef = false;
         current->section = Entry::CLASS_SEC;

         // preserve
         bool isPublished = current->m_traits.hasTrait(Entry::Virtue::Published);

         current->m_traits.clear();
         current->m_traits.setTrait(Entry::Virtue::Singleton);

         current->m_traits.setTrait(Entry::Virtue::Published, isPublished);

         addType( current );
         current->appendData(EntryKey::Member_Type, " singleton ");

         current->setData(EntryKey::File_Name, yyFileName);
         current->startLine = yyLineNr;
         current->bodyLine  = yyLineNr;

         BEGIN( CompoundName );

      } else  {
         // TODO is addType right? just copy/pasted

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
         isTypedef = false;
         current->section = Entry::CLASS_SEC;

         // preserve UNO IDL optional and published
         bool isOptional  = current->m_traits.hasTrait(Entry::Virtue::Optional);
         bool isPublished = current->m_traits.hasTrait(Entry::Virtue::Published);

         current->m_traits.clear();
         current->m_traits.setTrait(Entry::Virtue::Interface);

         current->m_traits.setTrait(Entry::Virtue::Optional,  isOptional);
         current->m_traits.setTrait(Entry::Virtue::Published, isPublished);

         addType( current );
         current->appendData(EntryKey::Member_Type, " interface");

         current->setData(EntryKey::File_Name, yyFileName);
         current->startLine   = yyLineNr;
         current->startColumn = yyColNr;
         current->bodyLine    = yyLineNr;

         BEGIN( CompoundName );

      } else {
         addType( current ) ;
         current->name = text.trimmed();
      }
   }

<FindMembers>{B}*"@implementation"{BN}+   {
      // Objective-C class implementation
      lineCount();

      isTypedef = false;
      current->section    = Entry::OBJCIMPL_SEC;
      language            = current->lang = SrcLangExt_ObjC;
      insideObjC          = true;
      current->protection = protection = Public;

      addType(current);
      current->appendData(EntryKey::Member_Type, " implementation");

      current->setData(EntryKey::File_Name, yyFileName);;
      current->startLine = yyLineNr;
      current->bodyLine  = yyLineNr;

      BEGIN( CompoundName );
   }

<FindMembers>{B}*"@interface"{BN}+  {
      // Objective-C class interface, or Java attribute
      lineCount();
      isTypedef = false;
      current->section = Entry::CLASS_SEC;

      current->m_traits.clear();
      current->m_traits.setTrait(Entry::Virtue::Interface);

      if (! insideJava) {
         language   = current->lang = SrcLangExt_ObjC;
         insideObjC = true;
      }

      current->protection = protection = Public;

      addType(current);
      current->appendData(EntryKey::Member_Type, " interface");

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;
      current->bodyLine    = yyLineNr;

      BEGIN( CompoundName );
   }

<FindMembers>{B}*"@protocol"{BN}+   {
      // Objective-C protocol definition
      lineCount();

      isTypedef = false;
      current->section = Entry::CLASS_SEC;

      current->m_traits.clear();
      current->m_traits.setTrait(Entry::Virtue::Protocol);

      language = current->lang = SrcLangExt_ObjC;

      insideObjC = true;
      current->protection = protection = Public;

      addType(current);
      current->appendData(EntryKey::Member_Type, " protocol");

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;
      current->bodyLine    = yyLineNr;

      BEGIN( CompoundName );
   }

<FindMembers>{B}*"exception"{BN}+   {
      // Corba IDL exception
      isTypedef = false;
      current->section = Entry::CLASS_SEC;

      bool isPublished = current->m_traits.hasTrait(Entry::Virtue::Published);

      current->m_traits.clear();
      current->m_traits.setTrait(Entry::Virtue::Exception);
      current->m_traits.setTrait(Entry::Virtue::Published, isPublished);

      addType(current);
      current->appendData(EntryKey::Member_Type, " exception");

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;
      current->bodyLine    = yyLineNr;
      lineCount();

      BEGIN( CompoundName );
   }

<FindMembers>"@class" | // for Objective C class declarations
<FindMembers>{B}*{TYPEDEFPREFIX}"class{" |
<FindMembers>{B}*{TYPEDEFPREFIX}"class"{BN}+   {
      // add a new class
      QString text = QString::fromUtf8(yytext);

      isTypedef        = text.contains("typedef");
      bool isConst     = text.contains("const");
      bool isVolatile  = text.contains("volatile");

      current->section = Entry::CLASS_SEC;
      addType(current);

      if (isConst) {
        current->appendData(EntryKey::Member_Type, " const");

      } else if (isVolatile) {
        current->appendData(EntryKey::Member_Type, " volatile");

      }

      current->appendData(EntryKey::Member_Type, " class");

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;
      current->bodyLine    = yyLineNr;

      if (text[0] == '@') {
         language   = current->lang = SrcLangExt_ObjC;
         insideObjC = true;
      }

      lineCount();
      if (text.endsWith('{')) {
        unput('{');
      }

      bool isAbstract = current->m_traits.hasTrait(Entry::Virtue::Abstract);

      if (insidePHP && isAbstract) {
         // convert Abstract to AbstractClass
         current->m_traits.setTrait(Entry::Virtue::AbstractClass);
         current->m_traits.setTrait(Entry::Virtue::Abstract, false);
      }

      BEGIN( CompoundName );
   }

<FindMembers>{B}*"value class{" |         // C++/CLI extension
<FindMembers>{B}*"value class"{BN}+     {
      QString text = QString::fromUtf8(yytext);

      isTypedef = false;
      current->section = Entry::CLASS_SEC;

      current->m_traits.clear();
      current->m_traits.setTrait(Entry::Virtue::Value);

      addType(current);
      current->appendData(EntryKey::Member_Type, " value class");

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;
      current->bodyLine    = yyLineNr;

      lineCount() ;

      if (text.endsWith('{')) {
         unput('{');
      }

      BEGIN( CompoundName ) ;
   }

<FindMembers>{B}*"ref class{" |          // C++/CLI extension
<FindMembers>{B}*"ref class"{BN}+      {
      QString text = QString::fromUtf8(yytext);
      isTypedef = false;

      current->section = Entry::CLASS_SEC;
      current->m_traits.clear();
      current->m_traits.setTrait(Entry::Virtue::Ref);

      addType(current);
      current->appendData(EntryKey::Member_Type, " ref class");

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;
      current->bodyLine    = yyLineNr;

      lineCount();

      if (text.endsWith('{')) {
        unput('{');
      }
      BEGIN( CompoundName ) ;
    }

<FindMembers>{B}*"interface class{" |     // C++/CLI extension
<FindMembers>{B}*"interface class"{BN}+ {
      QString text = QString::fromUtf8(yytext);
      isTypedef = false;

      current->section = Entry::CLASS_SEC;
      current->m_traits.clear();
      current->m_traits.setTrait(Entry::Virtue::Interface);

      addType( current );
      current->appendData(EntryKey::Member_Type, " interface class");

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine = yyLineNr;
      current->startColumn = yyColNr;
      current->bodyLine  = yyLineNr;

      lineCount();

      if (text.endsWith('{')) {
         unput('{');
      }

      BEGIN( CompoundName ) ;
   }

<FindMembers>{B}*"coclass"{BN}+     {
      QString text = QString::fromUtf8(yytext);

      if (insideIDL) {
         isTypedef = false;

         current->section = Entry::CLASS_SEC;

         addType(current);
         current->appendData(EntryKey::Member_Type, " coclass");

         current->setData(EntryKey::File_Name, yyFileName);
         current->startLine   = yyLineNr;
         current->startColumn = yyColNr;
         current->bodyLine    = yyLineNr;

         lineCount();

         BEGIN( CompoundName );

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

      // preserve UNO IDL & Inline attributes
      // can be a struct nested in an interface so keep insideObjC state

      bool isPublished = current->m_traits.hasTrait(Entry::Virtue::Published);
      bool isInline    = current->m_traits.hasTrait(Entry::Virtue::Inline);

      current->m_traits.clear();
      current->m_traits.setTrait(Entry::Virtue::Struct);
      current->m_traits.setTrait(Entry::Virtue::Published, isPublished);
      current->m_traits.setTrait(Entry::Virtue::Inline, isInline);

      addType( current );
      if (isConst) {
         current->appendData(EntryKey::Member_Type, " const");

      } else if (isVolatile) {
         current->appendData(EntryKey::Member_Type, " volatile");
      }

      current->appendData(EntryKey::Member_Type, " struct");

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;
      current->bodyLine    = yyLineNr;

      lineCount();

      if (text.endsWith('{')) {
         unput('{');
      }

      BEGIN( CompoundName ) ;
   }

<FindMembers>{B}*"value struct{" |      // C++/CLI extension
<FindMembers>{B}*"value struct"{BN}+     {
      QString text = QString::fromUtf8(yytext);

      isTypedef        = false;
      current->section = Entry::CLASS_SEC;

      current->m_traits.clear();
      current->m_traits.setTrait(Entry::Virtue::Struct);
      current->m_traits.setTrait(Entry::Virtue::Value);

      addType(current);
      current->appendData(EntryKey::Member_Type, " value struct");

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;
      current->bodyLine    = yyLineNr;

      lineCount();

      if (text.endsWith('{')) {
         unput('{');
      }

      BEGIN( CompoundName ) ;
   }

<FindMembers>{B}*"ref struct{" |       // C++/CLI extension
<FindMembers>{B}*"ref struct"{BN}+     {
      QString text = QString::fromUtf8(yytext);

      isTypedef = false;
      current->section = Entry::CLASS_SEC;

      current->m_traits.clear();
      current->m_traits.setTrait(Entry::Virtue::Struct);
      current->m_traits.setTrait(Entry::Virtue::Ref);

      addType(current);
      current->appendData(EntryKey::Member_Type, " ref struct");

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;
      current->bodyLine    = yyLineNr;

      lineCount();

      if (text.endsWith('{')) {
         unput('{');
      }

      BEGIN( CompoundName ) ;
   }

<FindMembers>{B}*"interface struct{" |     // C++/CLI extension
<FindMembers>{B}*"interface struct"{BN}+ {
      QString text = QString::fromUtf8(yytext);

      isTypedef = false;
      current->section = Entry::CLASS_SEC;

      current->m_traits.clear();
      current->m_traits.setTrait(Entry::Virtue::Struct);
      current->m_traits.setTrait(Entry::Virtue::Interface);

      addType(current);
      current->appendData(EntryKey::Member_Type, " interface struct");

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;
      current->bodyLine    = yyLineNr;

      lineCount();

      if (text.endsWith('{')) {
         unput('{');
      }

      BEGIN( CompoundName ) ;
   }

<FindMembers>{B}*{TYPEDEFPREFIX}"union{" |
<FindMembers>{B}*{TYPEDEFPREFIX}"union"{BN}+ {
      QString text = QString::fromUtf8(yytext);

      isTypedef        = text.indexOf("typedef")  !=-1;
      bool isConst     = text.indexOf("const")    !=-1;
      bool isVolatile  = text.indexOf("volatile") !=-1;

      current->section = Entry::CLASS_SEC;
      current->m_traits.clear();
      current->m_traits.setTrait(Entry::Virtue::Union);

      // can be a struct nested in an interface so keep insideObjC state
      // current->objc = insideObjC = false;

      addType(current);
      if (isConst) {
         current->appendData(EntryKey::Member_Type, " const");

      } else if (isVolatile) {
         current->appendData(EntryKey::Member_Type, " volatile");

      }

      current->appendData(EntryKey::Member_Type, " union");

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine    = yyLineNr;
      current->startColumn  = yyColNr;
      current->bodyLine     = yyLineNr;

      lineCount();

      if (text.endsWith('{')) {
         unput('{');
      }

      BEGIN( CompoundName ) ;
   }

<FindMembers>{B}*{TYPEDEFPREFIX}{IDLATTR}?"enum"({BN}+("class"|"struct"))?"{" |
<FindMembers>{B}*{TYPEDEFPREFIX}{IDLATTR}?"enum"({BN}+("class"|"struct"))?{BN}+ {
      // for IDL: typedef [something] enum
      QString text = QString::fromUtf8(yytext);

      isTypedef = text.indexOf("typedef") != -1;
      bool isStrongEnum = text.indexOf("struct") != -1 || text.indexOf("class") != -1 || insideCSharp;

      if (insideJava) {
         current->section = Entry::CLASS_SEC;

         current->m_traits.clear();
         current->m_traits.setTrait(Entry::Virtue::Enum);

      } else {
         current->section = Entry::ENUM_SEC;

      }

      addType( current );
      current->appendData(EntryKey::Member_Type, " enum");

      if (isStrongEnum) {
         current->m_traits.setTrait(Entry::Virtue::Strong);
      }

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;
      current->bodyLine    = yyLineNr;

      lineCount();

      if (text.endsWith('{')) {
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
      BEGIN( FindMembers );
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
      // generic is a C++ / CLI extension
      lineCount();

      ArgumentList temp;
      current->m_templateArgLists.append(temp);

      currentArgumentList = &current->m_templateArgLists.last();

      s_template_args = "<";

      fullArgString = s_template_args;

      s_argEntry    = QSharedPointer<Entry>();
      s_argEnum     = ArgKey::Template_Args;

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
      current->setData(EntryKey::File_Name, yyFileName);

      // add a using declaraton
      current->section = Entry::USINGDECL_SEC;
      current_root->addSubEntry(current, current_root);
      current = QMakeShared<Entry>(*current);

      // also add it as a using directive
      current->section = Entry::USINGDIR_SEC;
      current_root->addSubEntry(current, current_root);
      current = QMakeShared<Entry>();

      initEntry();
      aliasName.resize(0);
   }

<PHPUseAs>{BN}+"as"{BN}+      {
      lineCount();
   }

<PHPUseAs>{ID}                {
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
      current->setData(EntryKey::File_Name, yyFileName);
      current->section  = Entry::USINGDIR_SEC;

      current_root->addSubEntry(current, current_root);
      current = QMakeShared<Entry>();
      initEntry();
      BEGIN(Using);
   }

<JavaImport>({ID}{BN}*"."{BN}*)+{ID}   {
      // class import => add as a using declaration
      QString text = QString::fromUtf8(yytext);

      lineCount();
      current->name = removeRedundantWhiteSpace(substitute(text,".","::"));
      current->setData(EntryKey::File_Name, yyFileName);

      if (insideD) {
         current->section=Entry::USINGDIR_SEC;
      } else {
         current->section=Entry::USINGDECL_SEC;
      }

      current_root->addSubEntry(current, current_root);
      previous = current;
      current  = QMakeShared<Entry>();
      initEntry();
      BEGIN(Using);
   }

<FindMembers>"using"{BN}+     {
      current->startLine   = yyLineNr;
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

      current->name      = text;
      current->section   = Entry::USINGDECL_SEC;

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine = yyLineNr;

      current_root->addSubEntry(current, current_root);
      previous = current;
      current = QMakeShared<Entry>();

      if (insideCSharp) {
         // in C# a using declaration and directive have the same syntax, so we
         // also add it as a using directive here

         current->name        = text;

         current->setData(EntryKey::File_Name, yyFileName);
         current->startLine   = yyLineNr;
         current->startColumn = yyColNr;
         current->section     = Entry::USINGDIR_SEC;

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
      QString tmpType = "";

      tmpType = "typedef " + previous->getData(EntryKey::Member_Args);
      tmpType =  tmpType.simplified();

      previous->section  = Entry::VARIABLE_SEC;
      previous->name     = previous->name.trimmed();
      previous->bodyLine = yyLineNr;

      previous->setData(EntryKey::Member_Type, tmpType);
      previous->setData(EntryKey::Member_Args, "");

      previous->m_traits.setTrait(Entry::Virtue::Alias);

      BEGIN(FindMembers);
   }

<UsingAlias>";"{BN}*("/**"|"//!"|"/*!"|"///")"<" {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      s_docBlockContext = UsingAliasEnd;
      s_docBlockInBody  = false;

      static bool javadoc_auto_brief = Config::getBool("javadoc-auto-brief");
      static bool qt_auto_brief      = Config::getBool("qt-auto-brief");

      QChar tmpChar = text[text.length() - 2];
      s_docBlockAutoBrief = ( tmpChar == '*' && javadoc_auto_brief ) || ( tmpChar == '!' && qt_auto_brief );

      s_docBlock = QString(computeIndent(text, s_column), QChar(' '));

      lineCount();
      s_docBlockTerm = ';';

      if ( (text.length() - 3) == '/') {
         startCommentBlock(true);
         BEGIN( DocLine );

      } else {
         startCommentBlock(false);
         BEGIN( DocBlock );
      }
   }

<UsingAlias>">>"  {
      previous->appendData(EntryKey::Member_Args, "> >");
   }

<UsingAlias>.                           {
      QString text = QString::fromUtf8(yytext);
      previous->appendData(EntryKey::Member_Args, text);
   }

<UsingAlias>\n                          {
      QString text = QString::fromUtf8(yytext);
      previous->appendData(EntryKey::Member_Args, text);
      lineCount();
   }

<UsingAliasEnd>";"                      {
      previous->setData(EntryKey::Brief_Docs, current->getData(EntryKey::Brief_Docs));
      previous->setData(EntryKey::Main_Docs,  current->getData(EntryKey::Main_Docs));

      current->setData(EntryKey::Brief_Docs,   "");
      current->setData(EntryKey::Main_Docs,    "");

      unput(';');
      BEGIN(UsingAlias);
   }

<UsingDirective>{SCOPENAME}      {
      QString text = QString::fromUtf8(yytext);

      current->name    = removeRedundantWhiteSpace(text);
      current->section = Entry::USINGDIR_SEC;

      current->setData(EntryKey::File_Name, yyFileName);

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

      if (nameIsOperator(current->name)) {
         BEGIN( Operator );
      } else {
         BEGIN( EndTemplate );
      }
   }

<FindMemberName>{SCOPENAME}{BN}*/"<"   {
      QString text = QString::fromUtf8(yytext);
      sharpCount = 0;
      roundCount = 0;
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
      if (roundCount == 0 && --sharpCount <= 0) {
         BEGIN(FindMembers);
      }
   }

<EndTemplate>">"{BN}*"("      {
      lineCount();
      current->name += '>';

      if (roundCount == 0 && --sharpCount <= 0) {

         current->bodyLine = yyLineNr;
         current->setData(EntryKey::Member_Args, "(");

         fullArgString = "(";

         s_argEntry  = current;
         s_argEnum   = ArgKey::Member_Args;

         currentArgumentContext = FuncQual;

         BEGIN( ReadFuncArgType );
      }
   }

<EndTemplate>">"{BN}*/"("({BN}*{ID}{BN}*"::")*({BN}*"*"{BN}*)+ {
      // function pointer returning a template instance
      lineCount();
      current->name += '>';

      if (roundCount == 0) {
         BEGIN(FindMembers);
      }
   }

<EndTemplate>">"{BN}*/"::"       {
      lineCount();
      current->name += '>';

      if (roundCount==0 && --sharpCount<=0) {
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
      }  else {
         REJECT;
      }
   }

<CopyHereDoc>{ID}                       {
      // PHP heredoc
      QString text = QString::fromUtf8(yytext);
      s_delimiter = text;

      addToOutput(s_hereDocEntry, s_hereDocEnum, text);

      BEGIN(CopyHereDocEnd);
   }

<CopyHereDoc>"'"{ID}/"'"      {
      // PHP nowdoc
      QString text = QString::fromUtf8(yytext);
      s_delimiter = text.mid(1);

      addToOutput(s_hereDocEntry, s_hereDocEnum, text);

      BEGIN(CopyHereDocEnd);
   }

<HereDoc>{ID}                 {
      // PHP heredoc
      QString text = QString::fromUtf8(yytext);
      s_delimiter = text;
      BEGIN(HereDocEnd);
   }

<HereDoc>"'"{ID}/"'"          {
      // PHP nowdoc
      QString text = QString::fromUtf8(yytext);
      s_delimiter = text.mid(1);
      BEGIN(HereDocEnd);
   }

<HereDocEnd>^{ID}             {
      // id at start of the line could mark the end of the block
      QString text = QString::fromUtf8(yytext);

      if (s_delimiter == text) {
         // it is the end marker
         BEGIN(lastHereDocContext);
      }
   }

<HereDocEnd>.           {
   }

<CopyHereDocEnd>^{ID}   {
      // id at start of the line could mark the end of the block
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_hereDocEntry, s_hereDocEnum, text);

      if (s_delimiter == text) {
         // it is the end marker
         BEGIN(lastHereDocContext);
      }
   }

<CopyHereDocEnd>\n         {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_hereDocEntry, s_hereDocEnum, text);
   }

<CopyHereDocEnd>.             {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_hereDocEntry, s_hereDocEnum, text);
   }

<FindMembers>"Q_OBJECT"    {
      // Q_object, ignore
   }

<FindMembers>"Q_PROPERTY"  {
      // Q_property
     current->protection = Public;
     current->mtype      = MethodTypes::Property;
     current->setData(EntryKey::Member_Type, "");

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
      current->appendData(EntryKey::Member_Type, text);
   }

<QtPropType>{B}+           {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Type, text);
   }

<QtPropType>({TSCOPE}"::")*{TSCOPE}    {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Type, text);
      BEGIN(QtPropName);
   }

<QtPropName>{ID}           {
      QString text = QString::fromUtf8(yytext);
      current->name = text;
      BEGIN(QtPropAttr);
   }

<QtPropAttr>"READ"         {
      current->m_traits.setTrait(Entry::Virtue::Readable);
      BEGIN(QtPropRead);
   }

<QtPropAttr>"WRITE"        {
      current->m_traits.setTrait(Entry::Virtue::Writable);
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
      current->setData(EntryKey::Read_Property, text);
      BEGIN(QtPropAttr);
   }

<QtPropWrite>{ID}             {
      QString text = QString::fromUtf8(yytext);
      current->setData(EntryKey::Write_Property, text);
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

<CsObjectClose>")"                  {
      // end of cs_object
      BEGIN(FindMembers);
   }

<FindMembers>{CS_MACRO}             {
      // ignore this macro
      roundCount = 0;
      BEGIN(CsIgnore);
   }

<CsIgnore>"("                       {
      ++roundCount;
   }

<CsIgnore>")"                       {
      // end of macro ignore
      --roundCount;

      if (roundCount == 0)          {
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

     current->setData(EntryKey::File_Name, yyFileName);
     current->startLine   = yyLineNr;
     current->startColumn = yyColNr;

     current->name        = current->name.trimmed();
     current->section     = Entry::VARIABLE_SEC;

     // unknown data type, decipher later on
     // current->setData(EntryKey::Member_Type, = "");

     current_root->addSubEntry(current, current_root);
     current = QMakeShared<Entry>();
     initEntry();
     BEGIN(FindMembers);
   }

<FindMembers>"CS_PROPERTY_READ"     {
     current->protection = Public;
     current->mtype      = MethodTypes::Property;
     current->m_traits.setTrait(Entry::Virtue::Readable);
     current->setData(EntryKey::Member_Type, "");

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
      current->setData(EntryKey::Read_Property, text);
      BEGIN(CsPropClose);
   }

<FindMembers>"CS_PROPERTY_WRITE"        {
      current->protection = Public;
      current->mtype      = MethodTypes::Property;
      current->m_traits.setTrait(Entry::Virtue::Writable);
      current->setData(EntryKey::Member_Type, "");

      BEGIN(CsPropWriteVar);
   }

<CsPropWriteVar>"("                  {
   }

<CsPropWriteVar>{ID}                 {
      // cs property write name
      QString text = QString::fromUtf8(yytext);
      current->name = text;
   }

<CsPropWriteVar>","                  {
      BEGIN(CsPropWriteMethod);
   }

<CsPropWriteMethod>{ID}              {
      QString text = QString::fromUtf8(yytext);
      current->setData(EntryKey::Write_Property, text);
      BEGIN(CsPropClose);
   }

<FindMembers>"CS_PROPERTY_RESET"     {
      QString text = QString::fromUtf8(yytext);

      current->protection = Public;
      current->mtype = MethodTypes::Property;
      current->m_traits.setTrait(Entry::Virtue::Reset);
      current->setData(EntryKey::Member_Type, "");

      BEGIN(CsPropResetVar);
   }

<CsPropResetVar>"("                  {
   }

<CsPropResetVar>{ID}                 {
      QString text = QString::fromUtf8(yytext);
      current->name = text;
   }

<CsPropResetVar>","                  {
      BEGIN(CsPropResetMethod);
   }

<CsPropResetMethod>{ID}                 {
      QString text = QString::fromUtf8(yytext);
      current->setData(EntryKey::Reset_Property, text);
      BEGIN(CsPropClose);
   }

<FindMembers>"CS_PROPERTY_NOTIFY"       {
      current->protection  = Public;
      current->mtype       = MethodTypes::Property;
      current->m_traits.setTrait(Entry::Virtue::Notify);
      current->setData(EntryKey::Member_Type, "");

      BEGIN(CsPropNotifyVar);
   }

<CsPropNotifyVar>"("                  {
   }

<CsPropNotifyVar>{ID}                 {
      QString text = QString::fromUtf8(yytext);
      current->name = text;
   }

<CsPropNotifyVar>","                  {
      BEGIN(CsPropNotifyMethod);
   }

<CsPropNotifyMethod>{ID}              {
      QString text = QString::fromUtf8(yytext);
      current->setData(EntryKey::Notify_Property, text);
      BEGIN(CsPropClose);
   }

<FindMembers>"CS_PROPERTY_REVISION"        {
      current->protection = Public;
      current->mtype      = MethodTypes::Property;
      current->m_traits.setTrait(Entry::Virtue::Revision);
      current->setData(EntryKey::Member_Type, "");

      BEGIN(CsPropVar);
   }

<FindMembers>"CS_PROPERTY_DESIGNABLE"      {
      current->protection = Public;
      current->mtype      = MethodTypes::Property;
      current->m_traits.setTrait(Entry::Virtue::Designable);
      current->setData(EntryKey::Member_Type, "");

      BEGIN(CsPropVar);
   }

<FindMembers>"CS_PROPERTY_SCRIPTABLE"      {
      current->protection = Public;
      current->mtype      = MethodTypes::Property;
      current->m_traits.setTrait(Entry::Virtue::Scriptable);
      current->setData(EntryKey::Member_Type, "");

      BEGIN(CsPropVar);
   }

<FindMembers>"CS_PROPERTY_STORED"       {
      current->protection = Public;
      current->mtype      = MethodTypes::Property;
      current->m_traits.setTrait(Entry::Virtue::Stored);
      current->setData(EntryKey::Member_Type, "");

      BEGIN(CsPropVar);
   }

<FindMembers>"CS_PROPERTY_USER"         {
      current->protection    = Public;
      current->mtype = MethodTypes::Property;
      current->m_traits.setTrait(Entry::Virtue::User);
      current->setData(EntryKey::Member_Type, "");

      BEGIN(CsPropVar);
   }

<FindMembers>"CS_PROPERTY_CONSTANT"     {
      current->protection  = Public;
      current->mtype       = MethodTypes::Property;
      current->m_traits.setTrait(Entry::Virtue::Constant);
      current->setData(EntryKey::Member_Type, "");

      BEGIN(CsPropJustVar);
   }

<FindMembers>"CS_PROPERTY_FINAL"        {
      current->protection = Public;
      current->mtype      = MethodTypes::Property;
      current->m_traits.setTrait(Entry::Virtue::Final_Property);
      current->setData(EntryKey::Member_Type, "");

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
        current->setData(EntryKey::File_Name, yyFileName);
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

<FindMembers>"CS_SIGNAL_1"     {
      current->mtype = MethodTypes::Signal;
      current->setData(EntryKey::Member_Type, "");
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

<FindMembers>"CS_SLOT_1"       {
      current->mtype = MethodTypes::Slot;
      current->setData(EntryKey::Member_Type, "");
      BEGIN(CsSlot);
   }

<CsSlot>"("                  {
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

<CsSignalSlotMethod>[<,>]    {
      QString text = QString::fromUtf8(yytext);
      current->name += text;
   }

<CsSignalSlotMethod>{B}*"inline"{BN}+      {
      current->m_traits.setTrait(Entry::Virtue::Inline);
      lineCount();
   }

<CsSignalSlotMethod>[&*]+ {
      QString text   = QString::fromUtf8(yytext);
      current->name += text;

      addType(current);
   }

<CsSignalSlotMethod>{SCOPENAME}    {
      QString text = QString::fromUtf8(yytext);
      yyBegColNr   = yyColNr;
      yyBegLineNr  = yyLineNr;
      lineCount();

      addType(current);
      current->name += text;

      BEGIN(CsSignalSlotMethod);
   }

<CsSignalSlotMethod>"("           {
      QString text = QString::fromUtf8(yytext);

      if (! current->name.isEmpty()) {

         current->setData(EntryKey::Member_Args, text);
         current->bodyLine = yyLineNr;

         fullArgString = text;

         s_argEntry    = current;
         s_argEnum     = ArgKey::Member_Args;

         currentArgumentContext = CsSignalSlotClose;

         BEGIN(ReadFuncArgType);
      }
   }

<CsSignalSlotClose>")"             {
      // end of cs signal
      lineCount();

      current->setData(EntryKey::File_Name, yyFileName);
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

   /* end CopperSpice specific parsing */

<FindMembers>"friend"{BN}+("class"|"union"|"struct"){BN}+ {
      QString text = QString::fromUtf8(yytext);
      current->name = text;
      BEGIN(FindMembers);
   }

<FindMembers,FindMemberName>{SCOPENAME}   {
      QString text = QString::fromUtf8(yytext);

      if (insideCpp || insideObjC)  {
         current->setData(EntryKey::Clang_Id, ClangParser::instance()->lookup(yyLineNr, text));
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
         current->setData(EntryKey::Member_Type, "var");

      } else if (insideJS && text == "function") {
         // javascript function
         current->setData(EntryKey::Member_Type, "function");

      } else if (insideCSharp && text == "this") {
         // C# indexer
         addType( current ) ;
         current->name = "this";
         BEGIN(CSIndexer);

      } else if (insideCpp && text == "static_assert") {
         // C++11 static_assert
         BEGIN(StaticAssert);

      } else if (insideCpp && text == "decltype") {
         // C++11 decltype(x)
         current->appendData(EntryKey::Member_Type, text);

         BEGIN(DeclType);

      } else {

         if (YY_START == FindMembers) {
            addType(current);
         }

         bool javaLike = ( insideJava || insideCSharp || insideD || insidePHP || insideJS );

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

            if (current->name.left(7) == "static ") {
               current->stat = true;
               current->name = current->name.mid(7);

            } else if (current->name.left(7) == "inline ")  {

               if (current->getData(EntryKey::Member_Type).isEmpty()) {
                  current->setData(EntryKey::Member_Type, "inline");

               } else {
                  current->appendData(EntryKey::Member_Type, "inline ");

               }

               current->name = current->name.mid(7);

            } else if (current->name.left(6) == "const ") {

               if (current->getData(EntryKey::Member_Type).isEmpty()) {
                  current->setData(EntryKey::Member_Type, "const");

               } else {
                  current->appendData(EntryKey::Member_Type, "const ");
               }

               current->name = current->name.mid(6);
            }
         }

         if (nameIsOperator(text)) {
            BEGIN(Operator);

         } else {
            externC = false;
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
      current->appendData(EntryKey::Member_Type, text);

      lastRoundContext = FindMembers;

      s_roundEntry = current;
      s_roundEnum  = EntryKey::Member_Type;
      roundCount   = 0;

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
      if (insideJava || insideCSharp || insideD)  {
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
      curlyCount = 0;
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
      insideCppQuote = false;
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
         current->appendData(EntryKey::Source_Text, text);
      }

      BEGIN( PreLineCtrl );
   }

<PreLineCtrl>"\""[^\n\"]*"\""       {
      QString text = QString::fromUtf8(yytext);
      yyFileName = stripQuotes(text);

      if (lastPreLineCtrlContext==ReadBody || lastPreLineCtrlContext==ReadNSBody || lastPreLineCtrlContext==ReadBodyIntf) {
         current->appendData(EntryKey::Source_Text, text);
      }
   }

<PreLineCtrl>.             {
      QString text = QString::fromUtf8(yytext);
      if (lastPreLineCtrlContext==ReadBody || lastPreLineCtrlContext==ReadNSBody || lastPreLineCtrlContext==ReadBodyIntf) {
         current->appendData(EntryKey::Source_Text, text);
      }
   }

<PreLineCtrl>\n            {
      QString text = QString::fromUtf8(yytext);
      if (lastPreLineCtrlContext == ReadBody || lastPreLineCtrlContext==ReadNSBody || lastPreLineCtrlContext==ReadBodyIntf) {
         current->appendData(EntryKey::Source_Text, text);
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

      current->setData(EntryKey::Member_Args, "(");

      current->bodyLine      = yyLineNr;
      currentArgumentContext = DefineEnd;

      fullArgString = "(";

      s_argEntry  = current;
      s_argEnum   = ArgKey::Member_Args;

      BEGIN( ReadFuncArgType ) ;
   }

 /*
<DefineArg>")"             {
      current->appendData(EntryKey::Member_Args, "(");
      BEGIN( DefineEnd );
   }

<DefineArg>.            {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args,  text[0]);
   }
 */

<Define>{ID}            {
      QString text = QString::fromUtf8(yytext);

      current->bodyLine = yyLineNr;
      current->name     = text;
      BEGIN(DefineEnd);
   }

<DefineEnd>\n           {
      lineCount();

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;

      current->setData(EntryKey::Member_Type,  "");
      current->setData(EntryKey::Member_Args,  current->getData(EntryKey::Member_Args).simplified());

      current->name        = current->name.trimmed();
      current->section     = Entry::DEFINE_SEC;

      current_root->addSubEntry(current, current_root);
      current = QMakeShared<Entry>();
      initEntry();
      BEGIN(lastDefineContext);
   }

<DefinePHPEnd>";"          {
      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;

      current->setData(EntryKey::Member_Type, "const");

      QString init = current->getData(EntryKey::Initial_Value);
      init = init.simplified();
      init = init.left(init.length()-1);
      current->setData(EntryKey::Initial_Value, init);

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
      if (insideIDL && insideCppQuote) {
         BEGIN(EndCppQuote);
      } else {
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

      s_roundGEntry = current;
      s_roundGEnum  = EntryKey::Initial_Value;

      roundCount = 0;
      BEGIN( GCopyRound );
   }

<FindMembers>[\^%]         {
      // ^ and % are C++/CLI extensions
      QString text = QString::fromUtf8(yytext);

      if (insideCli) {
          addType( current );
          current->name = text ;
      } else {
         REJECT;
      }
   }

<FindMembers>[*&]+         {
      QString text = QString::fromUtf8(yytext);
      current->name += text;
      addType( current );
   }

<FindMembers,MemberSpec,Function,NextSemi,EnumBaseType,BitFields,ReadInitializer,OldStyleArgs>";"{BN}*("/**"|"//!"|"/*!"|"///")"<" {
      // */ (editor syntax fix)  comment found after a member

      QString text = QString::fromUtf8(yytext);

      static bool javadoc_auto_brief = Config::getBool("javadoc-auto-brief");
      static bool qt_auto_brief      = Config::getBool("qt-auto-brief");

      if (current->bodyLine == -1) {
         current->bodyLine = yyLineNr;
      }

      s_docBlockContext = YY_START;
      s_docBlockInBody  = false;

      QChar tmpChar = text[text.length() - 2];
      s_docBlockAutoBrief = ( tmpChar == '*' && javadoc_auto_brief ) || ( tmpChar == '!' && qt_auto_brief );

      s_docBlock = QString(computeIndent(text, s_column), QChar(' '));

      lineCount();
      s_docBlockTerm = ';';

      if (YY_START == EnumBaseType && current->section == Entry::ENUM_SEC) {
         current->setData(EntryKey::Member_Bitfields,  ":" + current->getData(EntryKey::Member_Args));
         current->setData(EntryKey::Member_Args,       "");
         current->section = Entry::VARIABLE_SEC;
      }

      if (text[text.length() - 3] == '/')  {
         startCommentBlock(true);
         BEGIN(DocLine);

      } else   {
         startCommentBlock(false);
         BEGIN(DocBlock);
      }
   }

<MemberSpec,FindFields,FindMembers,NextSemi,EnumBaseType,BitFields,ReadInitializer,OldStyleArgs>","{BN}*("/**"|"//!"|"/*!"|"///")"<" {
      // */ (editor syntax fix)

      QString text = QString::fromUtf8(yytext);
      s_docBlockContext   = YY_START;
      s_docBlockInBody    = false;

      static bool javadoc_auto_brief = Config::getBool("javadoc-auto-brief");
      static bool qt_auto_brief      = Config::getBool("qt-auto-brief");

      QChar tmpChar = text[text.length() - 2];
      s_docBlockAutoBrief = ( tmpChar == '*' && javadoc_auto_brief ) || ( tmpChar == '!' && qt_auto_brief );

      s_docBlock = QString(computeIndent(text, s_column), QChar(' '));

      lineCount();
      s_docBlockTerm = ',';

      if (YY_START == EnumBaseType && current->section == Entry::ENUM_SEC) {
        current->setData(EntryKey::Member_Bitfields,  ":" + current->getData(EntryKey::Member_Args));
        current->setData(EntryKey::Member_Args,       "");

        current->section = Entry::VARIABLE_SEC;
      }

      if (text[text.length() - 3] == '/') {
       startCommentBlock(true);
       BEGIN( DocLine );

      } else {
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

      s_docBlockContext   = YY_START;
      s_docBlockInBody    = false;
      s_docBlockAutoBrief = ( text[text.length() - 2] == '*' && Config::getBool("javadoc-auto-brief") ) ||
                            ( text[text.length() - 2] == '!' && Config::getBool("qt-auto-brief") );

      s_docBlock = QString(computeIndent(text, s_column), QChar(' '));

      lineCount();
      s_docBlockTerm = 0;

      if (text[text.length() - 3] == '/') {
        startCommentBlock(true);
        BEGIN( DocLine );
      } else {
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

      // current = tmp;
      initEntry();

      if (text[1] == '/') {
         if (text[2] == '!' || text[2] == '/') {
            s_docBlockContext   = YY_START;
            s_docBlockInBody    = false;
            s_docBlockAutoBrief = false;
            s_docBlock.resize(0);
            s_docBlockTerm = 0;

            startCommentBlock(true);
            BEGIN(DocLine);

         } else {
            lastCContext = YY_START;
            BEGIN(SkipCxxComment);
         }

      } else {

         if (text[2] == '!' || text[2] == '*') {
            s_docBlockContext   = YY_START;
            s_docBlockInBody    = false;

            static bool javadoc_auto_brief = Config::getBool("javadoc-auto-brief");
            static bool qt_auto_brief      = Config::getBool("qt-auto-brief");

            QChar tmpChar = text[text.length() - 2];
            s_docBlockAutoBrief = ( tmpChar == '*' && javadoc_auto_brief ) || ( tmpChar == '!' && qt_auto_brief );

            s_docBlock.resize(0);
            s_docBlockTerm = 0;

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
      current->setData(EntryKey::Initial_Value, text);

      lastInitializerContext = YY_START;
      initBracketCount       = 0;

      BEGIN(ReadInitializer);
   }

<UNOIDLAttributeBlock>{BN}*[gs]"et"{BN}+"raises"{BN}*"("{BN}*{SCOPENAME}{BN}*(","{BN}*{SCOPENAME}{BN}*)*")"{BN}*";"  {
      QString text = QString::fromUtf8(yytext);
      lineCount();

      current->appendData(EntryKey::Exception_Spec, " " + removeRedundantWhiteSpace(text));
   }

<UNOIDLAttributeBlock>"}"     {
      current->appendData(EntryKey::Exception_Spec, " }");
      BEGIN(FindMembers);
   }

  /* Read initializer rules */
<ReadInitializer>"("          {
      QString text = QString::fromUtf8(yytext);

      lastRoundContext  = YY_START;

      s_roundGEntry = current;
      s_roundGEnum  = EntryKey::Initial_Value;
      roundCount    = 0;

      current->appendData(EntryKey::Initial_Value,  text[0]);

      BEGIN(GCopyRound);
   }

<ReadInitializer>"{"          {
      QString text = QString::fromUtf8(yytext);

      lastCurlyContext  = YY_START;

      s_curlyGEntry = current;
      s_curlyGEnum  = EntryKey::Initial_Value;
      curlyCount    = 0;

      current->appendData(EntryKey::Initial_Value,  text[0]);

      BEGIN(GCopyCurly);
   }

<ReadInitializer>[;,]         {
      QString text = QString::fromUtf8(yytext);

      bool isEnum = current_root->m_traits.hasTrait(Entry::Virtue::Enum);

      if (text[0] == ';' && isEnum) {
         current->setData(EntryKey::File_Name,  yyFileName);
         current->startLine   = yyLineNr;
         current->startColumn = yyColNr;

         current->setData(EntryKey::Member_Args, current->getData(EntryKey::Member_Args).simplified());

         current->name        = current->name.trimmed();
         current->section     = Entry::VARIABLE_SEC;

         current_root->addSubEntry(current, current_root);
         current = QMakeShared<Entry>();
         initEntry();

         BEGIN(FindMembers);

         } else if (text[0] == ';' || (lastInitializerContext == FindFields && initBracketCount==0))  {
            // initBracketCount == 0 was added for bug 665778

            unput(*yytext);
            BEGIN(lastInitializerContext);

         } else if (text[0] == ',' && initBracketCount == 0) {
            // for "int a=0,b=0"

            unput(*yytext);
            BEGIN(lastInitializerContext);

      } else  {
         current->appendData(EntryKey::Initial_Value,  text[0]);

      }
   }

<ReadInitializer>{RAWBEGIN}             {
      // C++11 raw string
      QString text = QString::fromUtf8(yytext);

      if (! insideCpp) {
         REJECT;

      } else {
        current->appendData(EntryKey::Initial_Value, text);

        int i       = text.indexOf('"');
        s_delimiter = text.mid(i + 1);
        s_delimiter = s_delimiter.left(s_delimiter.length() - 1);

        lastRawStringContext = YY_START;

        s_rawGEntry = current;
        s_rawGEnum  = EntryKey::Initial_Value;

        BEGIN(RawGString);
      }
   }

<RawGString>{RAWEND}                    {
      QString text = QString::fromUtf8(yytext);

      addToOutput(s_rawGEntry, s_rawGEnum, text);

      QString delimiter = text.mid(1);
      delimiter = delimiter.left(delimiter.length() - 1);

      if (delimiter == s_delimiter) {
         BEGIN(lastRawStringContext);
      }
   }

<RawGString>[^)\n]+                     {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_rawGEntry, s_rawGEnum, text);
   }

<RawGString>.                           {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_rawGEntry, s_rawGEnum, text);
   }

<RawGString>\n                          {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_rawGEntry, s_rawGEnum, text);
      lineCount();
   }

<RawString>{RAWEND}                     {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_rawEntry, s_rawEnum, text);

      fullArgString   += text;

      QString delimiter = text.mid(1);
      delimiter = delimiter.left(delimiter.length() - 1);

      if (delimiter == s_delimiter) {
         BEGIN(lastRawStringContext);
      }
   }

<RawString>[^)]+                        {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_rawEntry, s_rawEnum, text);
      fullArgString   += text;
    }

<RawString>.                            {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_rawEntry, s_rawEnum, text);
      fullArgString   += text;
   }

<RawString>\n                           {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_rawEntry, s_rawEnum, text);
      fullArgString   += text;
      lineCount();
   }

<ReadInitializer>\"        {
      QString text = QString::fromUtf8(yytext);

     if (insideIDL && insideCppQuote) {
         BEGIN(EndCppQuote);

      } else {
         lastStringContext    =  YY_START;
         current->appendData(EntryKey::Initial_Value, text);

         s_quotedGEntry = current;
         s_quotedGEnum  = EntryKey::Initial_Value;

         BEGIN(CopyGString);
      }
   }

<ReadInitializer>"->"         {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Initial_Value, text);
   }

<ReadInitializer>"<<"         {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Initial_Value, text);;
   }

<ReadInitializer>">>"         {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Initial_Value, text);
   }

<ReadInitializer>[<\[{(]      {
      QString text = QString::fromUtf8(yytext);
      initBracketCount++;
      current->appendData(EntryKey::Initial_Value,  text[0]);
   }

<ReadInitializer>[>\]})]      {
      QString text = QString::fromUtf8(yytext);
      initBracketCount--;
      current->appendData(EntryKey::Initial_Value,  text[0]);
   }

<ReadInitializer>\'        {
      QString text = QString::fromUtf8(yytext);

      if (insidePHP) {
         current->appendData(EntryKey::Initial_Value, text);

         s_quotedGEntry = current;
         s_quotedGEnum  = EntryKey::Initial_Value;

         lastStringContext  = YY_START;

         BEGIN(CopyPHPGString);

      } else {
         current->appendData(EntryKey::Initial_Value, text);
      }
   }

<ReadInitializer>{CHARLIT}              {
      QString text = QString::fromUtf8(yytext);

      if (insidePHP) {
         REJECT;
      } else {
         current->appendData(EntryKey::Initial_Value, text);
      }
   }

<ReadInitializer>\n        {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Initial_Value,  text[0]);
      lineCount();
   }

<ReadInitializer>"@\""        {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Initial_Value, text);

      if (! insideCSharp && ! insideObjC)  {
         REJECT;

      }  else {
         // C#/ObjC verbatim string
         lastSkipVerbStringContext = YY_START;

         s_skipVerbEntry = current;
         s_skipVerbEnum  = EntryKey::Initial_Value;

         BEGIN(SkipVerbString);
      }
   }

<SkipVerbString>[^\n"]+          {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_skipVerbEntry, s_skipVerbEnum, text);
   }

<SkipVerbString>"\"\""        {
      // quote escape
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_skipVerbEntry, s_skipVerbEnum, text);;
   }

<SkipVerbString>"\""          {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_skipVerbEntry, s_skipVerbEnum, text[0]);
      BEGIN(lastSkipVerbStringContext);
   }

<SkipVerbString>\n         {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_skipVerbEntry, s_skipVerbEnum, text[0]);
      lineCount();
   }

<SkipVerbString>.          {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_skipVerbEntry, s_skipVerbEnum, text[0]);
   }

<ReadInitializer>"?>"         {
      QString text = QString::fromUtf8(yytext);

      if (insidePHP) {
         BEGIN( FindMembersPHP );
      }  else {
         current->appendData(EntryKey::Initial_Value, text);
      }
   }

<ReadInitializer>.         {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Initial_Value,  text[0]);
   }

  /* generic quoted string copy rules */
<CopyString,CopyPHPString>\\.       {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_quotedEntry, s_quotedEnum, text);
   }

<CopyString>\"             {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_quotedEntry, s_quotedEnum, text[0]);
      BEGIN( lastStringContext );
   }

<CopyPHPString>\'          {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_quotedEntry, s_quotedEnum, text[0]);
      BEGIN( lastStringContext );
   }

<CopyString,CopyPHPString>"/*"|"*/"|"//" {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_quotedEntry, s_quotedEnum, text);
   }

<CopyString,CopyPHPString>\n     {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_quotedEntry, s_quotedEnum, text);
      lineCount();
   }

<CopyString,CopyPHPString>.      {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_quotedEntry, s_quotedEnum, text[0]);
   }

  /* generic quoted growable string copy rules */
<CopyGString,CopyPHPGString>\\.     {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_quotedGEntry, s_quotedGEnum, text);
   }

<CopyGString>\"           {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_quotedGEntry, s_quotedGEnum, text[0]);
      BEGIN( lastStringContext );
   }

<CopyPHPGString>\'         {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_quotedGEntry, s_quotedGEnum, text[0]);
      BEGIN( lastStringContext );
   }

<CopyGString,CopyPHPGString>"/*"|"*/"|"//" {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_quotedGEntry, s_quotedGEnum, text);
   }

<CopyGString,CopyPHPGString>\n      {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_quotedGEntry, s_quotedGEnum, text[0]);
      lineCount();
   }

<CopyGString,CopyPHPGString>.       {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_quotedGEntry, s_quotedGEnum, text[0]);
   }

  /* generic round bracket list copy rules */
<CopyRound>\"           {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_roundEntry, s_roundEnum, text[0]);

      s_quotedEntry = s_roundEntry;
      s_quotedEnum  = s_roundEnum;

      lastStringContext = YY_START;

      BEGIN(CopyString);
   }

<CopyRound>"("             {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_roundEntry, s_roundEnum, text[0]);
      roundCount++;
   }

<CopyRound>")"             {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_roundEntry, s_roundEnum, text[0]);

      if (--roundCount < 0) {
         BEGIN(lastRoundContext);
      }
   }

<CopyRound>\n           {
      QString text = QString::fromUtf8(yytext);
      lineCount();
      addToOutput(s_roundEntry, s_roundEnum, text[0]);
   }

<CopyRound>\'           {
      QString text = QString::fromUtf8(yytext);

      if (insidePHP) {
         current->appendData(EntryKey::Initial_Value, text);

         s_quotedEntry = s_roundEntry;
         s_quotedEnum  = s_roundEnum;

         lastStringContext = YY_START;

         BEGIN(CopyPHPString);

      } else {
         addToOutput(s_roundEntry, s_roundEnum, text);
      }
   }

<CopyRound>{CHARLIT}               {
      QString text = QString::fromUtf8(yytext);

      if (insidePHP)  {
         REJECT;
      } else  {
         addToOutput(s_roundEntry, s_roundEnum, text);
      }
   }

<CopyRound>[^"'()\n]+         {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_roundEntry, s_roundEnum, text);
   }

<CopyRound>.            {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_roundEntry, s_roundEnum, text[0]);
   }

  /* generic round bracket list copy rules for growable strings */
<GCopyRound>\"             {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_roundGEntry, s_roundGEnum, text[0]);

      s_quotedGEntry = s_roundGEntry;
      s_quotedGEnum  = s_roundGEnum;

      lastStringContext  =  YY_START;
      BEGIN(CopyGString);
   }

<GCopyRound>"("            {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_roundGEntry, s_roundGEnum, text[0]);
      roundCount++;
   }

<GCopyRound>")"            {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_roundGEntry, s_roundGEnum, text[0]);

      if (--roundCount<0)
         BEGIN(lastRoundContext);
   }

<GCopyRound>\n             {
      QString text = QString::fromUtf8(yytext);
      lineCount();
      addToOutput(s_roundGEntry, s_roundGEnum, text[0]);
   }

<GCopyRound>\'             {
      QString text = QString::fromUtf8(yytext);

     if (insidePHP) {
         current->appendData(EntryKey::Initial_Value, text);

         s_quotedGEntry = s_roundGEntry;
         s_quotedGEnum  = s_roundGEnum;

         lastStringContext  = YY_START;

         BEGIN(CopyPHPGString);

      } else {
         addToOutput(s_roundGEntry, s_roundGEnum, text);
      }
   }

<GCopyRound>{CHARLIT}              {
      QString text = QString::fromUtf8(yytext);

      if (insidePHP) {
         REJECT;
      } else {
         addToOutput(s_roundGEntry, s_roundGEnum, text);
      }
   }

<GCopyRound>[^"'()\n/]+          {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_roundGEntry, s_roundGEnum, text);
   }

<GCopyRound>.           {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_roundGEntry, s_roundGEnum, text[0]);
   }

  /* generic curly bracket list copy rules */
<CopyCurly>\"           {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_curlyEntry, s_curlyEnum, text[0]);

      s_quotedEntry = s_curlyEntry;
      s_quotedEnum  = s_curlyEnum;

      lastStringContext = YY_START;

      BEGIN(CopyString);
   }

<CopyCurly>\'           {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_curlyEntry, s_curlyEnum, text[0]);

      if (insidePHP) {
         s_quotedEntry = s_curlyEntry;
         s_quotedEnum  = s_curlyEnum;

         lastStringContext = YY_START;
         BEGIN(CopyPHPString);
      }
   }

<CopyCurly>"{"             {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_curlyEntry, s_curlyEnum, text[0]);
      curlyCount++;
   }

<CopyCurly>"}"             {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_curlyEntry, s_curlyEnum, text[0]);

      if (--curlyCount < 0) {
         BEGIN(lastCurlyContext);
      }
   }

<CopyCurly>{CHARLIT}                    {
      QString text = QString::fromUtf8(yytext);

      if (insidePHP) {
         REJECT;
      } else {
         addToOutput(s_curlyEntry, s_curlyEnum, text);
      }
   }

<CopyCurly>[^"'{}\/\n]+          {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_curlyEntry, s_curlyEnum, text);
   }

<CopyCurly>"/"             {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_curlyEntry, s_curlyEnum, text);
   }

<CopyCurly>\n           {
      QString text = QString::fromUtf8(yytext);
      lineCount();
      addToOutput(s_curlyEntry, s_curlyEnum, text[0]);
   }

<CopyCurly>.            {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_curlyEntry, s_curlyEnum, text[0]);
   }

  /* generic curly bracket list copy rules for growable strings */
<GCopyCurly>^"#"{B}+[0-9]+{B}+"\""[^\"\n]+"\""{B}+"1"{B}*\n? {
       // start of included file marker
   }

<GCopyCurly>^"#"{B}+[0-9]+{B}+"\""[^\"\n]+"\""{B}+"2"{B}*\n? {
      // end of included file marker
      QString text = QString::fromUtf8(yytext);

      int s = text.indexOf(' ');
      int e = text.indexOf('"',s);
      yyLineNr = text.mid(s, e-s).toInt();

      if (text[text.length() - 1] == '\n') {
         lineCount();
         s_column = 0;
      }
   }

<GCopyCurly>\"             {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_curlyGEntry, s_curlyGEnum, text[0]);

      s_quotedGEntry = s_curlyGEntry;
      s_quotedGEnum  = s_curlyGEnum;

      lastStringContext  = YY_START;

      BEGIN(CopyGString);
   }

<GCopyCurly>\'             {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_curlyGEntry, s_curlyGEnum, text[0]);

      if (insidePHP) {
         s_quotedGEntry = s_curlyGEntry;
         s_quotedGEnum  = s_curlyGEnum;

         lastStringContext  = YY_START;
         BEGIN(CopyPHPGString);
      }
   }

<GCopyCurly>"{"            {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_curlyGEntry, s_curlyGEnum, text[0]);
      curlyCount++;
   }

<GCopyCurly>"}"            {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_curlyGEntry, s_curlyGEnum, text[0]);

      if (--curlyCount < 0) {
         BEGIN(lastCurlyContext);
      }
   }

<GCopyCurly>{CHARLIT}                    {
      QString text = QString::fromUtf8(yytext);

      if (insidePHP) {
         REJECT;

      } else {
         addToOutput(s_curlyGEntry, s_curlyGEnum, text);
      }
   }

<GCopyCurly>[^"'{}\/\n,]+     {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_curlyGEntry, s_curlyGEnum, text);
   }

<GCopyCurly>[,]+             {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_curlyGEntry, s_curlyGEnum, text);
   }

<GCopyCurly>"/"            {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_curlyGEntry, s_curlyGEnum, text);
   }

<GCopyCurly>\n             {
      QString text = QString::fromUtf8(yytext);
      lineCount();
      addToOutput(s_curlyGEntry, s_curlyGEnum, text[0]);
   }

<GCopyCurly>.           {
      QString text = QString::fromUtf8(yytext);
      addToOutput(s_curlyGEntry, s_curlyGEnum, text[0]);
   }

<FindMembers>":"        {
      if (current->getData(EntryKey::Member_Type).isEmpty() && current->name == "enum")  {
         // see bug 69041, C++11 style anon enum: 'enum : unsigned int {...}'

         current->section = Entry::ENUM_SEC;
         current->name.resize(0);
         current->setData(EntryKey::Member_Args, "");

         BEGIN(EnumBaseType);

      } else {

         if (current->getData(EntryKey::Member_Type).isEmpty()) {
            // anonymous padding field, "int :7;"

            addType(current);
            current->name = QString("__pad%1__").arg(padCount++);
         }

         BEGIN(BitFields);
         current->appendData(EntryKey::Member_Bitfields, ":");
      }
   }

<BitFields>.            {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Bitfields,  text[0]);
   }

<EnumBaseType>.            {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);
   }

<EnumBaseType>\n        {
      lineCount();
      current->appendData(EntryKey::Member_Args, " ");
   }

<FindMembers>[;,]       {
      QString text    = QString::fromUtf8(yytext);
      QString tmpType = current->getData(EntryKey::Member_Type);
      QString oldType = tmpType;

      if (current->bodyLine == -1) {
         current->bodyLine = yyLineNr;
      }

      if (insidePHP && tmpType.startsWith("var") ) {
         tmpType = tmpType.mid(3);
      }

      if (isTypedef && ! tmpType.startsWith("typedef ") ) {
        tmpType.prepend("typedef ");
      }

      bool needNewCurrent = false;

      if (! current->name.isEmpty() && current->section != Entry::ENUM_SEC) {
         tmpType = tmpType.simplified();

         current->setData(EntryKey::Member_Args, removeRedundantWhiteSpace(current->getData(EntryKey::Member_Args)));
         current->name = current->name.trimmed();

         if (current->section == Entry::CLASS_SEC)  {
            // remove spec for "struct Bla bla;"
            current->m_traits.clear();
         }

         current->section     = Entry::VARIABLE_SEC;

         current->setData(EntryKey::File_Name, yyFileName);
         current->startLine   = yyBegLineNr;
         current->startColumn = yyBegColNr;

         current_root->addSubEntry(current, current_root);
         needNewCurrent = true;
      }

      current->setData(EntryKey::Member_Type, tmpType);

      if (text[0] == ',') {
         bool stat = current->stat;

         if (needNewCurrent) {
            current = QMakeShared<Entry>(*current);
            initEntry();
          }

         current->stat = stat;         // the static attribute holds for all variables

         current->name.resize(0);
         current->setData(EntryKey::Member_Args,      "");
         current->setData(EntryKey::Brief_Docs,       "");
         current->setData(EntryKey::Main_Docs,        "");
         current->setData(EntryKey::Initial_Value,    "");
         current->setData(EntryKey::Member_Bitfields, "");

         int i = oldType.length();

         while (i > 0 && (oldType[i-1]=='*' || oldType[i-1]=='&' || oldType[i-1]==' ')) {
            i--;
         }

         current->setData(EntryKey::Member_Type, oldType.left(i));

      } else {
         mtype = MethodTypes::Method;
         virt  = Normal;

         if (needNewCurrent) {
            current = QMakeShared<Entry>();

         } else {
            current->m_groups.clear();

         }

         initEntry();
      }
   }

<FindMembers>"["        {
      QString text = QString::fromUtf8(yytext);

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

            current->m_traits.setTrait(Entry::Virtue::Gettable);
            current->m_traits.setTrait(Entry::Virtue::Settable);
         }

         BEGIN( IDLAttribute );

      } else if (insideCSharp && current->name.isEmpty()) {

         squareCount = 1;
         lastSquareContext = YY_START;

         // Skip the C# attribute for this member
         current->setData(EntryKey::Member_Args, "");
         BEGIN( SkipSquare );

      } else {
         current->appendData(EntryKey::Member_Args, text);
         squareCount = 1;
         externC     = false;
         BEGIN( Array );
      }
   }

<IDLAttribute>"]"          {
      // end of IDL function attribute
      if (--squareCount <= 0) {
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

      current->m_traits.setTrait(Entry::Virtue::Settable);
   }

<IDLAttribute>"propget"       {
      if (Config::getBool("idl-support")) {
         current->mtype = MethodTypes::Property;
      }

      current->m_traits.setTrait(Entry::Virtue::Gettable);
   }

<IDLAttribute>"property" {
      // UNO IDL property
      current->m_traits.setTrait(Entry::Virtue::Property);
   }

<IDLAttribute>"attribute" {
      // UNO IDL attribute
      current->m_traits.setTrait(Entry::Virtue::Attribute);
   }

<IDLAttribute>"optional" {
      // on UNO IDL interface/service/attribute/property
      current->m_traits.setTrait(Entry::Virtue::Optional);
   }

<IDLAttribute>"readonly" {
      // on UNO IDL attribute or property
      if (Config::getBool("idl-support") && odlProp) {
         bool isSettable = current->m_traits.hasTrait(Entry::Virtue::Settable);
         current->m_traits.setTrait(Entry::Virtue::Settable, ! isSettable);

      } else {
         current->m_traits.setTrait(Entry::Virtue::Readonly);
      }
   }

<IDLAttribute>"bound" {
      // on UNO IDL attribute or property
      current->m_traits.setTrait(Entry::Virtue::Bound);
   }

<IDLAttribute>"removable" {
      // on UNO IDL property
      current->m_traits.setTrait(Entry::Virtue::Removable);
   }

<IDLAttribute>"constrained" {
      // on UNO IDL property
      current->m_traits.setTrait(Entry::Virtue::Constrained);
   }

<IDLAttribute>"transient" {
      // on UNO IDL property
      current->m_traits.setTrait(Entry::Virtue::Transient);
   }

<IDLAttribute>"maybevoid" {
      // on UNO IDL property
      current->m_traits.setTrait(Entry::Virtue::MaybeVoid);
   }

<IDLAttribute>"maybedefault" {
      // on UNO IDL property
      current->m_traits.setTrait(Entry::Virtue::MaybeDefault);
   }

<IDLAttribute>"maybeambiguous" {
      // on UNO IDL property
      current->m_traits.setTrait(Entry::Virtue::MaybeAmbiguous);
   }

<IDLAttribute>.    {
   }

<IDLPropName>{BN}*{ID}{BN}*      {
      // return type (probably HRESULT) - skip it
      QString text = QString::fromUtf8(yytext);

      if (odlProp) {
         // property type
			idlProp = text;
	   }
   }

<IDLPropName>{ID}{BN}*"("     {
      QString text = QString::fromUtf8(yytext);
      current->name = text;
      current->name = current->name.left(current->name.length()-1).trimmed();
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;
      BEGIN( IDLProp );
   }

<IDLPropName>{BN}*"("{BN}*{ID}{BN}*")"{BN}*	{
      QString text = QString::fromUtf8(yytext);

		if (odlProp) {
		   idlProp += text;
		}
   }

<IDLPropName>{ID}{BN}*/";"		{
      QString text = QString::fromUtf8(yytext);
      if (odlProp) {
         current->name = text;
         idlProp = idlProp.trimmed();
         odlProp = false;

         BEGIN( IDLProp );
      }
   }

<IDLProp>{BN}*"["[^\]]*"]"{BN}*     {
      // attribute of a parameter
      QString text = QString::fromUtf8(yytext);
      idlAttr = text;
      idlAttr = idlAttr.trimmed();
   }

<IDLProp>{ID}                       {
      // property type
      QString text = QString::fromUtf8(yytext);
      idlProp = text;
   }

<IDLProp>{BN}*{ID}{BN}*","          {
      // Rare: Another parameter ([propput] HRESULT Item(int index, [in] Type theRealProperty);)
      QString text    = QString::fromUtf8(yytext);
      QString tmpArgs = current->getData(EntryKey::Member_Args);

      if (tmpArgs.isEmpty()) {
         current->setData(EntryKey::Member_Args, "(");

      } else {
         // idlProp was actually type of extra parameter
         tmpArgs += ", " + idlAttr + " " + idlProp + " " + text;

         tmpArgs = tmpArgs.left(tmpArgs.length() - 1);             // strip comma
         current->setData(EntryKey::Member_Args, tmpArgs);

         idlProp.resize(0);
         idlAttr.resize(0);

         BEGIN( IDLProp );
      }
   }

<IDLProp>{BN}*{ID}{BN}*")"{BN}*     {
      // the parameter name for the property -just skip
   }

<IDLProp>";"            {
      QString tmpArgs = current->getData(EntryKey::Member_Args).simplified();

      current->setData(EntryKey::Member_Type,  idlProp);

      if (! tmpArgs.isEmpty())   {
         current->setData(EntryKey::Member_Args, tmpArgs + ")");
      }

      current->name       = current->name.trimmed();
      current->section    = Entry::VARIABLE_SEC;

      current->setData(EntryKey::File_Name, yyFileName);

      current_root->addSubEntry(current, current_root);
      current = QMakeShared<Entry>();
      initEntry();

      BEGIN( FindMembers );
   }

<IDLProp>.           {
      // spaces, *, or other stuff
      // QString text = QString::fromUtf8(yytext);
      // idlProp += text;
   }

<Array>"]"                     {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);

      if (--squareCount <= 0) {
         BEGIN( FindMembers );
      }
   }

<FuncFuncArray>"]"              {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);

      if (--squareCount <= 0) {
         BEGIN( Function );
      }
   }

<Array,FuncFuncArray>"["      {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);
      squareCount++;
   }

<Array,FuncFuncArray>.        {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);
   }

<SkipSquare>"["            {
      squareCount++;
   }

<SkipSquare>"]"            {
      if (--squareCount <= 0)  {
         BEGIN( lastSquareContext );
      }
   }

<SkipSquare>\"             {
      lastStringContext = YY_START;
      BEGIN( SkipString );
   }

<SkipSquare>[^\n\[\]\"]+
<FindMembers>"<"        {
      QString text = QString::fromUtf8(yytext);

      addType( current );
      current->appendData(EntryKey::Member_Type, text);

      BEGIN( Sharp );
   }

<Sharp>">"                           {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Type, text[0]);

      if (--sharpCount <= 0) {
         BEGIN( FindMembers );
      }
   }

<Sharp>"<"           {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Type, text[0]);
      sharpCount++;
   }

<Sharp>{BN}+         {
      current->appendData(EntryKey::Member_Type, " ");
      lineCount();
   }

<Sharp>.             {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Type, text[0]);
   }

<FindFields>{ID}     {
      QString text = QString::fromUtf8(yytext);
      current->bodyLine = yyLineNr;
      current->name     = text;
   }

<FindFields>"("      {
      // Java enum initializer
      unput('(');

      lastInitializerContext = YY_START;
      initBracketCount       = 0;

      current->setData(EntryKey::Initial_Value, "=");

      BEGIN(ReadInitializer);
   }

<FindFields>"="      {
      QString text = QString::fromUtf8(yytext);

      lastInitializerContext = YY_START;
      initBracketCount       = 0;
      current->setData(EntryKey::Initial_Value, text);

      BEGIN(ReadInitializer);
   }

<FindFields>";"      {
      if (insideJava)   {
         // last enum field in Java class

         if (! current->name.isEmpty()) {
            current->setData(EntryKey::File_Name, yyFileName);
            current->startLine   = yyLineNr;
            current->startColumn = yyColNr;

            current->name        = current->name.trimmed();
            current->section     = Entry::VARIABLE_SEC;

            current->setData(EntryKey::Member_Type,  "@");          // enum marker
            current->setData(EntryKey::Member_Args,  current->getData(EntryKey::Member_Args).simplified());

            current_root->addSubEntry(current, current_root);
            current = QMakeShared<Entry>();
            initEntry();
         }

         BEGIN( FindMembers );

      } else {
         REJECT;
      }
   }

<SkipRemainder>\n     {
      lineCount();
   }

<SkipRemainder>[^\n]*
<FindFields>","       {
      if (! current->name.isEmpty()) {

         current->setData(EntryKey::File_Name, yyFileName);
         current->startLine   = yyLineNr;
         current->startColumn = yyColNr;

         bool isEnum = current_root->m_traits.hasTrait(Entry::Virtue::Enum);

         if (! isEnum) {
            // enum marker (this might be Java-style enums
            current->setData(EntryKey::Member_Type,  "@");
         }

         current->section = Entry::VARIABLE_SEC;
         current->name    = current->name.trimmed();

         current->setData(EntryKey::Member_Args,  current->getData(EntryKey::Member_Args).simplified());

         // add enum value to the enum data type
         current_root->addSubEntry(current, current_root);

         bool isStrong = current_root->m_traits.hasTrait(Entry::Virtue::Strong);

         if (! insideCSharp && ! insideJava && ! isStrong)  {
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

<FindFields>"["            {
      // attribute list in IDL
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
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
   }

<ReadBody,ReadNSBody,ReadBodyIntf>"//".*     {
      // read a comment
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
   }

<ReadBody,ReadNSBody,ReadBodyIntf>"#".*   {
      QString text = QString::fromUtf8(yytext);

      if (! insidePHP) {
         REJECT;
      }

      // append PHP comment
      current->appendData(EntryKey::Source_Text, text);
   }

<ReadBody,ReadNSBody,ReadBodyIntf>@\"  {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);

      s_skipVerbEntry = current;
      s_skipVerbEnum  = EntryKey::Source_Text;

      lastSkipVerbStringContext = YY_START;
      BEGIN( SkipVerbString );
   }

<ReadBody,ReadNSBody,ReadBodyIntf>"<<<"   {
      QString text = QString::fromUtf8(yytext);

      if (insidePHP) {
         current->appendData(EntryKey::Source_Text, text);

         s_hereDocEntry = current;
         s_hereDocEnum  = EntryKey::Source_Text;

         lastHereDocContext  = YY_START;

         BEGIN( CopyHereDoc );

      } else {
         REJECT;
      }
   }

<ReadBody,ReadNSBody,ReadBodyIntf>\"   {
      QString text = QString::fromUtf8(yytext);

      current->appendData(EntryKey::Source_Text, text);

      s_quotedGEntry = current;
      s_quotedGEnum  = EntryKey::Source_Text;

      lastStringContext = YY_START;
      BEGIN( CopyGString );
   }

<ReadBody,ReadNSBody,ReadBodyIntf>"/*"{B}*      {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      current->appendData(EntryKey::Source_Text, text);
      lastContext = YY_START;
      BEGIN( Comment );
   }

<ReadBody,ReadNSBody,ReadBodyIntf>"/*"{BL}      {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      current->appendData(EntryKey::Source_Text, text);
      ++yyLineNr;
      lastContext = YY_START;
      BEGIN( Comment );
   }

<ReadBody,ReadNSBody,ReadBodyIntf>"'"  {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);

      if (insidePHP) {
         // begin of single quoted string
         s_quotedGEntry = current;
         s_quotedGEnum  = EntryKey::Source_Text;

         lastStringContext  = YY_START;

         BEGIN(CopyPHPGString);
      }
   }

<ReadBody,ReadNSBody,ReadBodyIntf>{CHARLIT} {
      QString text = QString::fromUtf8(yytext);

      if (insidePHP) {
         // for PHP code single quotes are used for strings of arbitrary length
         REJECT;

      } else {
         current->appendData(EntryKey::Source_Text, text);
      }
   }

<ReadBody,ReadNSBody,ReadBodyIntf>"{"   {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
      ++curlyCount;
   }

<ReadBodyIntf>"}"          {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
      --curlyCount ;
   }

<ReadBody,ReadNSBody>"}"      {
      QString text = QString::fromUtf8(yytext);

      if ( curlyCount > 0 )  {
         current->appendData(EntryKey::Source_Text, text);
         --curlyCount;

      } else {
         current->endBodyLine = yyLineNr;
         QString &cn = current->name;
         QString rn  = current_root->name;

         if (! cn.isEmpty() && ! rn.isEmpty()) {
            prependScope();
         }

         if (isTypedef && cn.isEmpty()) {
            BEGIN( TypedefName );

         } else {
            bool isEnum = current->m_traits.hasTrait(Entry::Virtue::Enum);

            if ((current->section == Entry::ENUM_SEC) || isEnum) {
               // add field terminator
               current->appendData(EntryKey::Source_Text, ",");
            }

            // add compound definition to the tree
            current->setData(EntryKey::Member_Args,  removeRedundantWhiteSpace(current->getData(EntryKey::Member_Args)));

            current->setData(EntryKey::Member_Type, current->getData(EntryKey::Member_Type).simplified());
            current->name = current->name.trimmed();

            bool isInterface     = current->m_traits.hasTrait(Entry::Virtue::Interface);
            bool isOnlyCategory  = current->m_traits.onlyHasTrait(Entry::Virtue::Category);

            if (insideObjC && (isInterface || isOnlyCategory) )  {
               // method definition follows
               BEGIN( ReadBodyIntf );

            } else {
               current_root->addSubEntry(current , current_root);
               memspecEntry = current;
               current = QMakeShared<Entry>(*current);

               bool isOnlyInterface = current->m_traits.onlyHasTrait(Entry::Virtue::Interface);

               if (current->section == Entry::NAMESPACE_SEC || isOnlyInterface ||
                     insideJava || insidePHP || insideCSharp || insideD || insideJS) {

                  // namespaces and interfaces and java classes ends with a closing bracket without semicolon
                  current->reset();
                  initEntry();

                  memspecEntry = QSharedPointer<Entry>();

                  BEGIN( FindMembers ) ;

               } else {
                  static QRegExp re("@[0-9]+$");

                  if (! isTypedef && memspecEntry && re.indexIn(memspecEntry->name) == -1) {
                     // not typedef or anonymous type
                     // enabled the next two lines for

                     current->setData(EntryKey::Brief_Docs,  "");
                     current->setData(EntryKey::Main_Docs,   "");
                  }

                  BEGIN( MemberSpec );
               }
            }
         }
      }
   }

<ReadBody>"}"{BN}+"typedef"{BN}+    {
      QString text = QString::fromUtf8(yytext);

      lineCount();
      if ( curlyCount >0 ) {
         current->appendData(EntryKey::Source_Text, text);
         --curlyCount;

      } else {
         isTypedef = true;
         current->endBodyLine = yyLineNr;
         QString &cn = current->name;
         QString rn = current_root->name;

         if (! cn.isEmpty() && !rn.isEmpty()) {
            prependScope();
         }

         BEGIN( TypedefName );
      }
   }

<TypedefName>("const"|"volatile"){BN}  {
      // late "const" or "volatile" keyword
      QString text = QString::fromUtf8(yytext);
      lineCount();

      current->prependData(EntryKey::Member_Type, text);
   }

<TypedefName>{ID}          {
      QString text = QString::fromUtf8(yytext);

      bool isEnum = current->m_traits.hasTrait(Entry::Virtue::Enum);

      if ((current->section == Entry::ENUM_SEC) || isEnum) {
         // add field terminator
         current->appendData(EntryKey::Source_Text, ",");
      }

      current->name = text;

      prependScope();
      current->setData(EntryKey::Member_Args,  current->getData(EntryKey::Member_Args).simplified());
      current->setData(EntryKey::Member_Type,  current->getData(EntryKey::Member_Type).simplified());

      current_root->addSubEntry(current , current_root);

      if (! firstTypedefEntry) {
         firstTypedefEntry = current;
      }

      current = QMakeShared<Entry>();
      initEntry();
      isTypedef = true;          // to undo reset by initEntry()
      BEGIN(MemberSpecSkip);
   }

<TypedefName>";"        {
      /* typedef of anonymous type */
      current->name = QString("@%1").arg(anonCount++);

      bool isEnum = current->m_traits.hasTrait(Entry::Virtue::Enum);

      if ((current->section == Entry::ENUM_SEC) || isEnum) {
         // add field terminator
         current->appendData(EntryKey::Source_Text, ",");
      }

      // add compound definition to the tree
      current->setData(EntryKey::Member_Args,  current->getData(EntryKey::Member_Args).simplified());
      current->setData(EntryKey::Member_Type,  current->getData(EntryKey::Member_Type).simplified());

      current_root->addSubEntry( current , current_root);
      memspecEntry = current;
      current = QMakeShared<Entry>(*current);
      initEntry();

      unput(';');
      BEGIN( MemberSpec );
   }

<MemberSpec>([*&]*{BN}*)*{ID}{BN}*("["[^\]\n]*"]")* {
      // the [] part could be improved.
      QString text = QString::fromUtf8(yytext);

      lineCount();
      int i = 0;
      int l = text.length();
      int j;

      while (i < l && (! isId(text[i])) ) {
        i++;
      }

      msName = text.right(l-i).trimmed();
      j = msName.indexOf("[");

      if (j != -1) {
         msArgs=msName.right(msName.length()-j);
         msName=msName.left(j);
      }

      msType = text.left(i);

      // handle *pName in: typedef { ... } name, *pName;
      if (firstTypedefEntry) {

         bool isStruct = firstTypedefEntry->m_traits.hasTrait(Entry::Virtue::Struct);
         bool isUnion  = firstTypedefEntry->m_traits.hasTrait(Entry::Virtue::Union);

         if (isStruct) {
            msType.prepend("struct " + firstTypedefEntry->name);

         } else if (isUnion) {
            msType.prepend("union " + firstTypedefEntry->name);

         }  else if (firstTypedefEntry->section == Entry::ENUM_SEC) {
            msType.prepend("enum " + firstTypedefEntry->name);

         } else {
            msType.prepend(firstTypedefEntry->name);
         }
      }
   }

<MemberSpec>"("            {
      // function with struct return type
      addType(current);
      current->name = msName;
      current->m_traits.clear();

      unput('(');
      BEGIN(FindMembers);
   }

<MemberSpec>[,;]        {
      QString text = QString::fromUtf8(yytext);

      if (msName.isEmpty() && ! current->name.isEmpty()) {
         // see if the compound does not have a name or is inside another
         // anonymous compound. If so we insert a special `anonymous' variable.
         // Entry *p=current_root;
         // Entry *p=current;

         QSharedPointer<Entry> p = current;

         while (p) {
            // only look for class scopes, not namespace scopes
            if ((p->section & Entry::COMPOUND_MASK) && !p->name.isEmpty()) {
               //printf("Trying scope `%s'\n",p->name.data());
               int i = p->name.lastIndexOf("::");
               int pi = (i==-1) ? 0 : i+2;

               if (p->name.at(pi)=='@') {
                  // anonymous compound inside -> insert dummy variable name

                  msName = QString("@%1").arg(anonCount++);
                  break;
               }
            }

            if (p == current) p=current_root; else p=p->parent();
         }
      }

      if (! msName.isEmpty() ) {
         //  && msName != current->name )
         // skip typedef T {} T;, removed due to bug608493

         static bool typedefHidesStruct =  Config::getBool("use-typedef-name");

         // case 1: typedef struct _S { ... } S_t;
         // -> omit typedef and use S_t as the struct name

         bool isUnion  = current->m_traits.hasTrait(Entry::Virtue::Union);
         bool isStruct = current->m_traits.hasTrait(Entry::Virtue::Struct);

         if (typedefHidesStruct &&  isTypedef && (isUnion || isStruct || current->section == Entry::ENUM_SEC) &&
                  msType.trimmed().isEmpty() && memspecEntry) {

            memspecEntry->name = msName;

         }  else  {
            // case 2: create a typedef field

            QSharedPointer<Entry> varEntry = QMakeShared<Entry>();
            varEntry->lang       = language;
            varEntry->protection = current->protection;
            varEntry->mtype      = current->mtype;
            varEntry->virt       = current->virt;
            varEntry->stat       = current->stat;
            varEntry->section    = Entry::VARIABLE_SEC;
            varEntry->name       = msName.trimmed();

            varEntry->setData(EntryKey::Member_Type,  current->getData(EntryKey::Member_Type).simplified() + " ");
            varEntry->setData(EntryKey::Member_Args,  msArgs);

            if (isTypedef) {
               varEntry->prependData(EntryKey::Member_Type, "typedef ");
            }

            bool isUnion  = current->m_traits.hasTrait(Entry::Virtue::Union);
            bool isStruct = current->m_traits.hasTrait(Entry::Virtue::Struct);

            if (typedefHidesStruct && isTypedef && (isUnion || isStruct) && memspecEntry) {
               // case 1: use S_t as type for pS_t in "typedef struct _S {} S_t, *pS_t;"

               varEntry->appendData(EntryKey::Member_Type, memspecEntry->name + msType);

            } else {
               // case 2: use _S as type for for pS_t

               varEntry->appendData(EntryKey::Member_Type,  current->name + msType);
            }

            varEntry->setData(EntryKey::File_Name, yyFileName);
            varEntry->startLine   = yyLineNr;
            varEntry->startColumn = yyColNr;

            varEntry->mGrpId      = current->mGrpId;

            varEntry->setData(EntryKey::Brief_Docs,     current->getData(EntryKey::Brief_Docs));
            varEntry->setData(EntryKey::Main_Docs,      current->getData(EntryKey::Main_Docs));
            varEntry->setData(EntryKey::Initial_Value,  current->getData(EntryKey::Initial_Value));

            // copy group list
            for (auto &g : current->m_groups) {
               varEntry->m_groups.append(g);
            }

            // copy special list items
            for (auto &lii : current->m_specialLists) {
               varEntry->addSpecialListItem(lii.type, lii.itemId);
            }

            current_root->addSubEntry(varEntry, current_root);
         }
      }

      if (text[0] == ';')  {
         // end of a struct/class ...

         if (! isTypedef && msName.isEmpty() && memspecEntry && (current->section&Entry::COMPOUND_MASK)) {
            // case where a class/struct has a doc block after it

            if (! current->getData(EntryKey::Main_Docs).isEmpty()) {
               memspecEntry->appendData(EntryKey::Main_Docs,  current->getData(EntryKey::Main_Docs));
            }

            if (! current->getData(EntryKey::Brief_Docs).isEmpty()) {
               memspecEntry->appendData(EntryKey::Brief_Docs, current->getData(EntryKey::Brief_Docs));
            }
         }

         msType.resize(0);
         msName.resize(0);
         msArgs.resize(0);
         isTypedef = false;

         firstTypedefEntry = QSharedPointer<Entry>();
         memspecEntry      = QSharedPointer<Entry>();

         current->reset();
         initEntry();
         BEGIN( FindMembers );

      } else {
         current->getData(EntryKey::Brief_Docs);
         current->getData(EntryKey::Main_Docs);
      }
    }

<MemberSpec>"="            {
      QString text = QString::fromUtf8(yytext);

      lastInitializerContext = YY_START;
      initBracketCount = 0;

      current->setData(EntryKey::Initial_Value, text);

      BEGIN(ReadInitializer);
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
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
      lineCount();
   }

<ReadBodyIntf>"@end"/[^a-z_A-Z0-9]  {
      // end of Objective C block
      current_root->addSubEntry( current, current_root);
      current = QMakeShared<Entry>();
      initEntry();

      language = current->lang = SrcLangExt_Cpp;
      insideObjC = false;

      BEGIN( FindMembers );
   }

<ReadBody,ReadNSBody,ReadBodyIntf>.    {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);;
   }

<FindMembers>"("/{BN}*"::"*{BN}*({TSCOPE}{BN}*"::")*{TSCOPE}{BN}*")"{BN}*"(" | /* typedef void (A<int>::func_t)(args...) */
<FindMembers>("("({BN}*"::"*{BN}*{TSCOPE}{BN}*"::")*({BN}*[*&\^]{BN}*)+)+ {
      /* typedef void (A::*ptr_t)(args...) or int (*func(int))[], the ^ is for Obj-C blocks */
      QString text = QString::fromUtf8(yytext);
      if (insidePHP)  {
         // reference parameter
         REJECT

      } else {
         current->bodyLine = yyLineNr;
         lineCount();
         addType(current);
         funcPtrType = text;
         roundCount  = 0;
         BEGIN( FuncPtr );
      }
   }

<FuncPtr>{SCOPENAME}          {
      QString text = QString::fromUtf8(yytext);
      current->name = text;
      if (nameIsOperator(current->name)) {
         BEGIN( FuncPtrOperator );

         } else {

         if (current->name == "const" || current->name == "volatile") {
            funcPtrType += current->name;
         } else {
            BEGIN( EndFuncPtr );
         }
      }
   }

<FuncPtr>.           {
      // printf("error: FuncPtr `%c' unexpected at line %d of %s\n",*yytext,yyLineNr,yyFileName);
   }

<FuncPtrOperator>"("{BN}*")"{BN}*/"("  {
      QString text = QString::fromUtf8(yytext);
      current->name += text;
      current->name = current->name.simplified();
      lineCount();
   }

<FuncPtrOperator>\n        {
      QString text = QString::fromUtf8(yytext);
      lineCount();
      current->name += text[0];
   }

<FuncPtrOperator>"("          {
      unput(*yytext);
      BEGIN( EndFuncPtr );
   }

<FuncPtrOperator>.         {
      QString text = QString::fromUtf8(yytext);
      current->name += text[0];
   }

<EndFuncPtr>")"{BN}*/";"      {
      // a variable with extra braces
      lineCount();
      current->appendData(EntryKey::Member_Type, funcPtrType.mid(1));
      BEGIN(FindMembers);
   }

<EndFuncPtr>")"{BN}*/"("      {
      // a function pointer
      lineCount();
      current->appendData(EntryKey::Member_Type, funcPtrType + ")");
      BEGIN(FindMembers);
   }

<EndFuncPtr>")"{BN}*/"["      {
      // an array of variables
      lineCount();
      current->appendData(EntryKey::Member_Type, funcPtrType);
      current->appendData(EntryKey::Member_Args, ")");

      BEGIN(FindMembers);
   }

<EndFuncPtr>"("            {
      // a function returning a function or
      // a function returning a pointer to an array
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);

      current->bodyLine      = yyLineNr;
      currentArgumentContext = FuncFuncEnd;
      fullArgString          = current->getData(EntryKey::Member_Args);

      s_argEntry  = current;
      s_argEnum   = ArgKey::Member_Args;

      BEGIN( ReadFuncArgType );
   }

<EndFuncPtr>"["[^\n\]]*"]"       {
      QString text = QString::fromUtf8(yytext);
      funcPtrType += text;
   }

<EndFuncPtr>")"            {
      BEGIN(FindMembers);
   }

<FuncFunc>"("           {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);
      ++roundCount;
   }

<FuncFunc>")"           {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);

      if ( roundCount ) {
         --roundCount;
      } else {
         BEGIN(FuncFuncEnd);
      }
   }

<FuncFuncEnd>")"{BN}*"("      {
      lineCount();
      current->appendData(EntryKey::Member_Type,  funcPtrType + ")(");
      BEGIN(FuncFuncType);
   }

<FuncFuncEnd>")"{BN}*/[;{]       {
      lineCount();
      current->appendData(EntryKey::Member_Type,  funcPtrType.mid(1));
      BEGIN(Function);
   }

<FuncFuncEnd>")"{BN}*/"["     {
      // function returning a pointer to an array
      lineCount();

      current->appendData(EntryKey::Member_Type, funcPtrType);
      current->appendData(EntryKey::Member_Args, ")");

      BEGIN(FuncFuncArray);
   }

<FuncFuncEnd>.             {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);
   }

<FuncFuncType>"("          {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Type, text[0]);
      roundCount++;
   }

<FuncFuncType>")"          {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Type, text[0]);

      if (roundCount) {
         --roundCount;
      } else {
         BEGIN(Function);
      }
   }

<FuncFuncType>{BN}*","{BN}*      {
      lineCount();
      current->appendData(EntryKey::Member_Type, ", ");
   }

<FuncFuncType>{BN}+        {
      lineCount();
      current->appendData(EntryKey::Member_Type, " ");
   }

<FuncFuncType>.            {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Type, text[0]);
   }

<FindMembers>"("/{BN}*{ID}{BN}*"*"{BN}*{ID}*")"{BN}*"("  {
      // for catching typedef void (__stdcall *f)() like definitions
      QString text = QString::fromUtf8(yytext);

      if (current->getData(EntryKey::Member_Type).startsWith("typedef") && current->bodyLine == -1) {
         // the bodyLine check is to prevent this guard to be true more than once

         current->bodyLine = yyLineNr;
         BEGIN( GetCallType );

       } else if (! current->name.isEmpty())  {
         // normal function
         current->setData(EntryKey::Member_Args, text);

         current->bodyLine      = yyLineNr;
         currentArgumentContext = FuncQual;
         fullArgString          = text;

         s_argEntry  = current;
         s_argEnum   = ArgKey::Member_Args;

         BEGIN( ReadFuncArgType );
      }
   }

<GetCallType>{BN}*{ID}{BN}*"*"      {
      QString text = QString::fromUtf8(yytext);
      lineCount();
      addType(current);
      funcPtrType = "(";
      funcPtrType += text;
      roundCount  = 0;
      BEGIN( FuncPtr );
   }

<FindMembers>"("        {
      QString text = QString::fromUtf8(yytext);

      if (! current->name.isEmpty()) {
         current->setData(EntryKey::Member_Args, text);

         current->bodyLine      = yyLineNr;
         currentArgumentContext = FuncQual;
         fullArgString          = text;

         s_argEntry  = current;
         s_argEnum   = ArgKey::Member_Args;

         BEGIN( ReadFuncArgType );
      }
   }

  /*
<FindMembers>"("{BN}*("void"{BN}*)?")"    {
      lineCount();
      current->setData(EntryKey::Member_Args, "()");
      BEGIN( FuncQual );
   }
  */

  /* Function argument reading rules */

<ReadFuncArgType>[^ \/\r\t\n\)\(\"\'#]+ {
      QString text = QString::fromUtf8(yytext);
      addToArgs(text);

      fullArgString  += text;
   }

<CopyArgString,CopyArgPHPString>[^\n\\\"\']+       {
      QString text = QString::fromUtf8(yytext);
      addToArgs(text);
      fullArgString  += text;
   }

<CopyArgRound>[^\/\n\)\(\"\']+      {
      QString text = QString::fromUtf8(yytext);
      addToArgs(text);
      fullArgString  += text;
   }

<ReadFuncArgType,ReadTempArgs>{BN}*    {
      addToArgs(" ");
      fullArgString  += " ";
      lineCount();
   }

<ReadFuncArgType,CopyArgRound,CopyArgSharp,ReadTempArgs>{RAWBEGIN}   {
      QString text = QString::fromUtf8(yytext);

      s_delimiter = text.mid(2);
      s_delimiter = s_delimiter.left(s_delimiter.length() - 1);

      lastRawStringContext = YY_START;

      s_rawEntry = QSharedPointer<Entry>();
      // s_rawEnum  = do not set

      addToOutput(s_rawEntry, s_rawEnum, text);

      fullArgString   += text;

      BEGIN(RawString);
   }

<ReadFuncArgType,CopyArgRound,CopyArgSharp,ReadTempArgs>\"  {
      QString text = QString::fromUtf8(yytext);

      addToArgs(text[0]);
      fullArgString += text[0];

      lastCopyArgStringContext = YY_START;
      BEGIN(CopyArgString);
   }

<ReadFuncArgType,ReadTempArgs>"("   {
      QString text = QString::fromUtf8(yytext);

      addToArgs(text[0]);
      fullArgString  += text[0];
      argRoundCount  = 0;

      lastCopyArgContext = YY_START;

      BEGIN(CopyArgRound);
   }

<ReadFuncArgType>")"          {
      QString text = QString::fromUtf8(yytext);

      addToArgs(text[0]);
      fullArgString  += text[0];

      current->argList = stringToArgumentList(fullArgString, current->argList);

      if (insideJS) {
         fixArgumentListForJavaScript(current->argList);
      }

      handleParametersCommentBlocks(current->argList);

      // save the current documentation block
      s_briefBackup = current->getData(EntryKey::Brief_Docs);
      s_mainBackup  = current->getData(EntryKey::Main_Docs);

      // go back to the saved state
      BEGIN(currentArgumentContext);
    }

   /* a special comment */
<ReadFuncArgType,ReadTempArgs>("/*"[*!]|"//"[/!])("<"?)  {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      if (currentArgumentContext == DefineEnd) {

         // for defines we interpret a comment as documentation for the define
         for (int i = yyleng - 1; i >= 0; i--) {
            unput(yytext[i]);
         }

         current->argList = stringToArgumentList(fullArgString, current->argList);
         handleParametersCommentBlocks(current->argList);
         BEGIN(currentArgumentContext);

      } else {
          // not a define
         // for functions we interpret a comment as documentation for the argument
         fullArgString  += text;
         lastCopyArgChar = QChar(0);
         lastCommentInArgContext = YY_START;

         if (text[1] == '/')
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
      // */ (editor syntax fix)
      lastCContext = YY_START;
      BEGIN( SkipComment );
   }

<ReadFuncArgType,ReadTempArgs>"//"  {
      lastCContext = YY_START;
      BEGIN( SkipCxxComment );
   }

  /*
<ReadFuncArgType,ReadTempArgs>"'#"  {
      QString text = QString::fromUtf8(yytext);
      if (insidePHP) {
         REJECT;
      }
      addToArgs(text[0]);
      fullArgString   += text;
   }

<ReadFuncArgType,ReadTempArgs>"#"   {
      if (!insidePHP) {
         REJECT;
      }
      lastCContext = YY_START;
      BEGIN( SkipCxxComment );
   }
  */

   /* `)' followed by a special comment */
<ReadFuncArgType>")"{BN}*("/*"[*!]|"//"[/!])"<"    {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);
      lineCount();

      if (currentArgumentContext == DefineEnd) {
         // for defines we interpret a comment as documentation for the define
         for (int i = yyleng -1; i > 0; i--) {
            unput(yytext[i]);
         }

         addToArgs(text[0]);
         fullArgString  += text[0];

         current->argList = stringToArgumentList(fullArgString, current->argList);
         handleParametersCommentBlocks(current->argList);
         BEGIN( currentArgumentContext );

         } else {
         // for functions we interpret a comment as documentation for the last argument
         lastCopyArgChar = text[0];

         QString tmp = text.mid(1).trimmed();

         lastCommentInArgContext = YY_START;
         fullArgString += tmp;

         if (tmp.indexOf("//") != -1) {
            BEGIN( CopyArgCommentLine );
         }   else {
            BEGIN( CopyArgComment );
         }
      }
   }

<CopyArgComment>^{B}*"*"+/{BN}+
<CopyArgComment>[^\n\\\@\*]+     {
      QString text = QString::fromUtf8(yytext);
      fullArgString += text;
   }

<CopyArgComment>"*/"          {
      QString text = QString::fromUtf8(yytext);
      fullArgString += text;

      if (lastCopyArgChar.unicode() != 0) {
         unput(lastCopyArgChar);
      }
      BEGIN( lastCommentInArgContext );
   }

<CopyArgCommentLine>\n        {
      QString text = QString::fromUtf8(yytext);
      fullArgString += text;
      lineCount();

      if (lastCopyArgChar.unicode() != 0) {
         unput(lastCopyArgChar);
      }

      BEGIN( lastCommentInArgContext );
   }

<CopyArgCommentLine>{CMD}("verbatim"|"latexonly"|"htmlonly"|"xmlonly"|"manonly"|"dot"|"code")/[^a-z_A-Z0-9]    {
      // verbatim command (which could contain nested comments!)
      QString text = QString::fromUtf8(yytext);

      s_docBlockName = text.mid(1);
      fullArgString += text;

      BEGIN(CopyArgVerbatim);
   }

<CopyArgCommentLine>{CMD}("f$"|"f["|"f{")       {
      QString text = QString::fromUtf8(yytext);

      s_docBlockName = text.mid(1);

      if (s_docBlockName.at(1) == '[') {
        s_docBlockName[1] ='}';
      }

      if (s_docBlockName.at(1) == '{') {
        s_docBlockName[1] ='}';
      }

      fullArgString += text;
      BEGIN(CopyArgVerbatim);
   }

<CopyArgVerbatim>[\\@]("endverbatim"|"endlatexonly"|"endhtmlonly"|"endxmlonly"|"enddocbookonly"|"endmanonly"|"enddot"|"endcode"|"f$"|"f]"|"f}")/[^a-z_A-Z0-9] {
      // end of verbatim block
      QString text = QString::fromUtf8(yytext);
      fullArgString += text;

      if (text[1] == 'f') {
         // end of formula
         BEGIN(CopyArgCommentLine);
      }

      if (text.mid(4) == s_docBlockName) {
         BEGIN(CopyArgCommentLine);
      }
   }

<CopyArgCommentLine>[^\\\@\n]+      {
      QString text = QString::fromUtf8(yytext);
      fullArgString += text;
   }

<CopyArgCommentLine>.         {
      QString text = QString::fromUtf8(yytext);
      fullArgString += text[0];
   }
<CopyArgComment,CopyArgVerbatim>\n  {
      QString text = QString::fromUtf8(yytext);
      fullArgString += text[0];
      lineCount();
   }

<CopyArgComment,CopyArgVerbatim>.   {
      QString text = QString::fromUtf8(yytext);
      fullArgString += text[0];
   }


<CopyArgComment>{CMD}("brief"|"short"){B}+ {
      QString text = QString::fromUtf8(yytext);
      warn(yyFileName, yyLineNr, "Ignoring %c brief command inside argument documentation", csPrintable(text[0]) );
      fullArgString += ' ';
   }

<ReadTempArgs>"<"          {
      QString text = QString::fromUtf8(yytext);
      addToArgs(text[0]);
      fullArgString  += text[0];
      argSharpCount  =  1;
      BEGIN( CopyArgSharp );
   }

<ReadTempArgs>">"          {
      QString text = QString::fromUtf8(yytext);
      addToArgs(text[0]);
      fullArgString  += text[0];

      *currentArgumentList = stringToArgumentList(fullArgString, *currentArgumentList);

      BEGIN( currentArgumentContext );
   }

<CopyArgRound>"("          {
      QString text = QString::fromUtf8(yytext);
      argRoundCount++;
      addToArgs(text[0]);
      fullArgString  += text[0];
   }

<CopyArgRound>")"          {
      QString text = QString::fromUtf8(yytext);
      addToArgs(text[0]);
      fullArgString  += text[0];

      if (argRoundCount > 0) {
         argRoundCount--;
      } else {
         BEGIN( lastCopyArgContext );
      }
   }

<CopyArgSharp>"("                       {
      QString text = QString::fromUtf8(yytext);
      addToArgs(text[0]);

      fullArgString  += text[0];
      argRoundCount  = 0;
      lastCopyArgContext = YY_START;

      BEGIN( CopyArgRound );
   }

<CopyArgSharp>"<"          {
      QString text = QString::fromUtf8(yytext);
      argSharpCount++;
      addToArgs(text[0]);
      fullArgString  += text[0];
   }

<CopyArgSharp>">"          {
      QString text = QString::fromUtf8(yytext);
      addToArgs(text[0]);
      fullArgString  += text[0];
      argSharpCount--;

      if (argSharpCount>0) {
         //

      } else {
         BEGIN( ReadTempArgs );

      }
   }

<CopyArgString,CopyArgPHPString>\\.    {
      QString text = QString::fromUtf8(yytext);
      addToArgs(text[0]);
      fullArgString  += text;
   }

<CopyArgString>\"          {
      QString text = QString::fromUtf8(yytext);
      addToArgs(text[0]);
      fullArgString  += text[0];
      BEGIN( lastCopyArgStringContext );
   }

<CopyArgPHPString>\'          {
      QString text = QString::fromUtf8(yytext);
      addToArgs(text[0]);
      fullArgString  += text[0];
      BEGIN( lastCopyArgStringContext );
   }

<ReadFuncArgType,ReadTempArgs,CopyArgRound,CopyArgSharp>{CHARLIT}     {
      QString text = QString::fromUtf8(yytext);

      if (insidePHP) {
         REJECT;
      } else {
         addToArgs(text);
         fullArgString  += text;
      }
   }

<ReadFuncArgType,ReadTempArgs,CopyArgRound,CopyArgSharp>\'     {
      QString text = QString::fromUtf8(yytext);
      addToArgs(text);
      fullArgString  += text;

      if (insidePHP) {
         lastCopyArgStringContext = YY_START;
         BEGIN(CopyArgPHPString);
      }
   }

<ReadFuncArgType,ReadTempArgs,CopyArgString,CopyArgPHPString,CopyArgRound,CopyArgSharp>\n  {
      QString text = QString::fromUtf8(yytext);
      lineCount();
      addToArgs(text[0]);
      fullArgString  += text[0];
   }

<ReadFuncArgType,ReadTempArgs,CopyArgString,CopyArgPHPString,CopyArgRound,CopyArgSharp>.    {
      QString text = QString::fromUtf8(yytext);
      addToArgs(text[0]);
      fullArgString  += text[0];
   }

<FuncRound>"("             {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args,  text[0]);
      ++roundCount;
   }

<FuncRound>")"                          {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args,  text[0]);

      if (roundCount) {
         --roundCount;
      } else {
         BEGIN( FuncQual );
      }
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

<FuncQual>[{:;,]    {
      QString text = QString::fromUtf8(yytext);

      if (text == ";" && insidePHP &&  ! containsWord(current->getData(EntryKey::Member_Type), "function") ) {
         current->reset();
         initEntry();
         BEGIN( FindMembers );

      } else {
         unput(*yytext);
         BEGIN( Function );
      }
   }

<FuncQual>{BN}*"abstract"{BN}*          {
      // pure virtual member function
      lineCount() ;
      current->virt = Pure;
      current->appendData(EntryKey::Member_Args, " override ");
   }

<FuncQual,TrailingReturn>{BN}*"override"{BN}*          {
      // C++11 overridden virtual member function
      lineCount();

      current->m_traits.setTrait(Entry::Virtue::Override);
      current->appendData(EntryKey::Member_Args, " override ");

      BEGIN(FuncQual);
   }

<FuncQual,TrailingReturn>{BN}*"final"{BN}*             {
      // C++11 final method
      lineCount();

      current->m_traits.setTrait(Entry::Virtue::Final);
      current->appendData(EntryKey::Member_Args, " final ");

      BEGIN(FuncQual);
   }

<FuncQual>{BN}*"sealed"{BN}*            {
      // sealed member function
      lineCount() ;
      current->m_traits.setTrait(Entry::Virtue::Sealed);
      current->appendData(EntryKey::Member_Args, " sealed ");
   }

<FuncQual>{BN}*"new"{BN}*               {
      // new member function
      lineCount() ;
      current->m_traits.setTrait(Entry::Virtue::New);
      current->appendData(EntryKey::Member_Args, " new ");
   }

<FuncQual>{BN}*"const"{BN}*         {
      // const member function
      lineCount();
      current->appendData(EntryKey::Member_Args, " const ");
      current->argList.constSpecifier = true;
   }

<FuncQual>{BN}*"volatile"{BN}*      {
      // volatile member function
      lineCount();
      current->appendData(EntryKey::Member_Args, " volatile ");
      current->argList.volatileSpecifier = true;
   }

<FuncQual>{BN}*"noexcept"{BN}*      {
      // noexcept qualifier
      lineCount();
      current->appendData(EntryKey::Member_Args, " noexcept ");
      current->m_traits.setTrait(Entry::Virtue::NoExcept);
   }

<FuncQual>{BN}*"noexcept"{BN}*"("      {
      // noexcept expression
      lineCount();

      current->appendData(EntryKey::Member_Args, " noexcept(");
      current->m_traits.setTrait(Entry::Virtue::NoExcept);
      lastRoundContext = FuncQual;

      s_roundEntry = current;
      s_roundEnum  = EntryKey::Member_Args;
      roundCount   = 0;

      BEGIN(CopyRound);
   }

<FuncQual>{BN}*"&"   {
      current->appendData(EntryKey::Member_Args, " &");
      current->argList.refSpecifier = RefType::LValueRef;
   }

<FuncQual>{BN}*"&&"  {
      current->appendData(EntryKey::Member_Args, " &&");
      current->argList.refSpecifier = RefType::RValueRef;
   }

<FuncQual,TrailingReturn>{BN}*"="{BN}*"0"{BN}*     {
      // pure virtual member function
      lineCount() ;
      current->appendData(EntryKey::Member_Args, " = 0");
      current->virt = Pure;
      current->argList.pureSpecifier = true;
      BEGIN(FuncQual);
   }

<FuncQual,TrailingReturn>{BN}*"="{BN}*"delete"{BN}*     {
      // C++11 explicitly delete member
      lineCount();
      current->appendData(EntryKey::Member_Args, " = delete");
      current->m_traits.setTrait(Entry::Virtue::Delete);
      current->argList.isDeleted = true;
      BEGIN(FuncQual);
   }

<FuncQual,TrailingReturn>{BN}*"="{BN}*"default"{BN}*     {
      // C++11 explicitly defaulted constructor/assignment operator
      lineCount();
      current->appendData(EntryKey::Member_Args, " = default");
      current->m_traits.setTrait(Entry::Virtue::Default);
      BEGIN(FuncQual);
   }

<FuncQual>{BN}*"->"{BN}*                {
      lineCount();
      current->argList.trailingReturnType = " -> ";
      current->appendData(EntryKey::Member_Args, " -> ");
      BEGIN(TrailingReturn);
   }

<TrailingReturn>[{;]                    {
      unput(*yytext);
      BEGIN(FuncQual);
   }

<TrailingReturn>.                       {
      QString text = QString::fromUtf8(yytext);
      current->argList.trailingReturnType += text;
      current->appendData(EntryKey::Member_Args, text);
   }

<TrailingReturn>\n                      {
      QString text = QString::fromUtf8(yytext);
      lineCount();
      current->argList.trailingReturnType += text;
      current->appendData(EntryKey::Member_Args, " ");
   }

<FuncRound,FuncFunc>{BN}*","{BN}*   {
      lineCount();
      current->appendData(EntryKey::Member_Args, ", ");
   }

<FuncQual,FuncRound,FuncFunc>{BN}+     {
      lineCount() ;
      current->appendData(EntryKey::Member_Args, " ");
   }

<Function,FuncQual,FuncRound,FuncFunc>"#" {
      if (insidePHP)  {
         REJECT;
      }

      lastCPPContext = YY_START;
      BEGIN(SkipCPP);
   }

<FuncQual>"="           {
      QString text = QString::fromUtf8(yytext);

      if (insideCli && (current_root->section&Entry::COMPOUND_MASK)) {
         BEGIN(CliOverride);

      } else {
         // typically an initialized function pointer
         lastInitializerContext = YY_START;
         initBracketCount       = 0;

         current->setData(EntryKey::Initial_Value, text);

         BEGIN(ReadInitializer);
      }
   }

<CliOverride>{ID}          {
   }

<CliOverride>"{"           {
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
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);

      s_quotedEntry = current;
      s_quotedEnum  = EntryKey::Member_Args;

      lastStringContext = FuncPtrInit;

      BEGIN(CopyString);
   }

<FuncPtrInit>\'            {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);

      if (insidePHP)  {
         s_quotedEntry = current;
         s_quotedEnum  = EntryKey::Member_Args;

         lastStringContext = FuncPtrInit;
         BEGIN(CopyPHPString);
      }
   }

<FuncPtrInit>{CHARLIT}        {
      QString text = QString::fromUtf8(yytext);

      if (insidePHP) {
         REJECT;
      } else {
         current->appendData(EntryKey::Member_Args, text);
      }
   }

<FuncPtrInit>{ID}          {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text);
   }

<FuncPtrInit>.             {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);
   }

<FuncPtrInit>\n            {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);
      lineCount();
   }

<FuncQual>{ID}             {
      // typically a K&R style C function
      QString text = QString::fromUtf8(yytext);

      if (insideCSharp && text == "where") {
         // type contraint for a method

         current->typeConstr = ArgumentList();
         current->typeConstr.append(Argument());

         lastCSConstraint = YY_START;
         BEGIN( CSConstraintName );

      } else if (checkForKnRstyleC()) {
         current->setData(EntryKey::Member_Args, text);
         oldStyleArgType.resize(0);
         BEGIN(OldStyleArgs);

      } else {
         current->appendData(EntryKey::Member_Args, text);
      }
   }

<OldStyleArgs>[,;]         {
      QString text = QString::fromUtf8(yytext);

      QString oldStyleArgPtr;
      QString oldStyleArgName;
      splitKnRArg(oldStyleArgPtr, oldStyleArgName);

      QString tmpBrief  = current->getData(EntryKey::Brief_Docs);
      QString tmpMain   = current->getData(EntryKey::Main_Docs);

      if (tmpBrief != s_briefBackup) {
         current->setData(EntryKey::Brief_Docs, s_briefBackup);
      } else{
         tmpBrief = "";
      }

      if (tmpMain != s_mainBackup) {
         current->setData(EntryKey::Main_Docs, s_mainBackup);
      } else {
         tmpMain = "";
      }

      addKnRArgInfo(oldStyleArgType + oldStyleArgPtr, oldStyleArgName, tmpBrief, tmpMain);
      current->setData(EntryKey::Member_Args, "");

      if (text[0] == ';') {
        oldStyleArgType = "";
      }
   }

<OldStyleArgs>{ID}         {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text);
   }

<OldStyleArgs>"{"          {
      current->setData(EntryKey::Member_Args, argListToString(current->argList));
      unput('{');
      BEGIN(FuncQual);
   }

<OldStyleArgs>.            {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);
   }

<FuncQual,FuncRound,FuncFunc>.      {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);
   }

<FuncQual>{BN}*"try:"         |
<FuncQual>{BN}*"try"{BN}+     {
      /* try-function-block */
      QString text = QString::fromUtf8(yytext);

      insideTryBlock = true;
      lineCount();

      if (text[text.length() - 1] == ':') {
         unput(':');
         BEGIN( Function );
      }
   }

<FuncQual>{BN}*"throw"{BN}*"("      {
      // C++ style throw clause
      current->setData(EntryKey::Exception_Spec, "  throw (");
      roundCount = 0;

      lineCount();
      BEGIN( ExcpRound );
   }

<FuncQual>{BN}*"raises"{BN}*"("         {
      current->setData(EntryKey::Exception_Spec, " raises (");

      lineCount() ;
      roundCount = 0;
      BEGIN( ExcpRound );
   }

<FuncQual>{BN}*"throws"{BN}+     {
      // Java style throw clause
      current->setData(EntryKey::Exception_Spec, " throws ");

      lineCount();
      BEGIN( ExcpList );
   }

<ExcpRound>"("             {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Exception_Spec, text[0]);
      ++roundCount;
   }

<ExcpRound>")"                          {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Exception_Spec, text[0]);

      if ( roundCount )
         --roundCount ;
      else
         BEGIN( FuncQual ) ;
   }

<ExcpRound>.            {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Exception_Spec, text[0]);
   }

<ExcpList>"{"           {
      unput('{');
      BEGIN( FuncQual );
   }

<ExcpList>";"           {
      unput(';');
      BEGIN( FuncQual );
   }

<ExcpList>"\n"             {
      current->appendData(EntryKey::Exception_Spec, " ");
      lineCount();
   }

<ExcpList>.             {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Exception_Spec, text[0]);
   }

<Function>"("           {
      QString text = QString::fromUtf8(yytext);

      current->appendData(EntryKey::Member_Type, current->name);
      current->name = current->getData(EntryKey::Member_Args);
      current->appendData(EntryKey::Member_Args, text);

      roundCount    = 0;

      BEGIN( FuncRound ) ;
   }

<Function>":"              {
      if (! insidePHP) {
         BEGIN(SkipInits);
      }
   }

<Function>[;{,]            {
      QString text = QString::fromUtf8(yytext);

      current->name        = current->name.simplified();
      current->setData(EntryKey::Member_Type,  current->getData(EntryKey::Member_Type).simplified());
      current->setData(EntryKey::Member_Args,  removeRedundantWhiteSpace(current->getData(EntryKey::Member_Args)));

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyBegLineNr;
      current->startColumn = yyBegColNr;

      static QRegExp re("\\([^)]*[*&][^)]*\\)");       // (...*...)

      if (text[0] !=';' || (current_root->section&Entry::COMPOUND_MASK) ) {
         int tempArg = current->name.indexOf('<');

         QString tmpType = current->getData(EntryKey::Member_Type);

         int ts = tmpType.indexOf('<');
         int te = tmpType.lastIndexOf('>');
         int ti = re.indexIn(tmpType, 0);

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

         if (! tmpType.isEmpty() && (! isFunction || tmpType.startsWith("typedef ")) )  {

            if (isTypedef && ! tmpType.startsWith("typedef ")) {
               current->prependData(EntryKey::Member_Type, "typedef ");
            }

            current->section = Entry::VARIABLE_SEC;

         } else {
            current->section = Entry::FUNCTION_SEC;
            current->proto   = (text[0] == ';');
         }

      } else  {
         // a global function or function variable
         QString tmpType = current->getData(EntryKey::Member_Type);

         if (! tmpType.isEmpty() && (re.indexIn(tmpType, 0) !=-1 || tmpType.startsWith("typedef ")) ) {

            if (isTypedef && ! tmpType.startsWith("typedef ")) {
               current->prependData(EntryKey::Member_Type, "typedef ");
            }

            current->section = Entry::VARIABLE_SEC;

         } else {
            current->section = Entry::FUNCTION_SEC;
            current->proto   = true;
         }

      }

      //
      QString tmpType = current->getData(EntryKey::Member_Type);

      if (insidePHP) {
         if (findAndRemoveWord(tmpType, "final")) {
            current->m_traits.setTrait(Entry::Virtue::Final);
         }

         if (findAndRemoveWord(tmpType, "abstract")) {
            current->m_traits.setTrait(Entry::Virtue::Abstract);
         }
      }

      current->setData(EntryKey::Member_Type, tmpType);

      if (insidePHP && ! containsWord(tmpType, "function")) {
         initEntry();

         if (text[0] == '{' ) {
            lastCurlyContext = FindMembers;
            curlyCount       = 0;
            BEGIN( SkipCurly );

         } else {
            BEGIN( FindMembers );

         }

      } else {
         if (insidePHP) {
            findAndRemoveWord(tmpType, "function");
            current->setData(EntryKey::Member_Type, tmpType);
         }

         previous = current;

         current_root->addSubEntry(current, current_root);
         current  = QMakeShared<Entry>();
         initEntry();

         bool isOptional_Prev = previous->m_traits.hasTrait(Entry::Virtue::Optional);
         bool isRequired_Prev = previous->m_traits.hasTrait(Entry::Virtue::Required);

         // Objective C 2.0: Required/Optional section

         if (isOptional_Prev) {
            current->m_traits.setTrait(Entry::Virtue::Optional);
         }

         if (isRequired_Prev) {
            current->m_traits.setTrait(Entry::Virtue::Required);
         }

         lastCurlyContext = FindMembers;

         if (text[0] == ',' ) {
            QString tmpType = previous->getData(EntryKey::Member_Type);

            // strip any trailing * and & (see bugs 623023 and 649103 for test cases)
            int i = tmpType.length();

            while (i > 0 && (tmpType[i-1] == '*' || tmpType[i-1] == '&' || tmpType[i-1] == ' ')) {
               i--;
            }

            current->setData(EntryKey::Member_Type, tmpType.left(i));
         }

         if (text[0] == '{' ) {
            if (! insidePHP && (current_root->section & Entry::COMPOUND_MASK) ) {
               previous->m_traits.setTrait(Entry::Virtue::Inline);
            }

            curlyCount = 0;
            BEGIN( SkipCurly ) ;

         } else {

            if (previous->section != Entry::VARIABLE_SEC) {
               // a function/member declaration
               previous->bodyLine = -1;
            }

            BEGIN( FindMembers );
         }
      }
   }

<SkipInits>">"{BN}*"{"                {
      // C++11 style initializer
      lineCount();
      curlyCount = 1;
      BEGIN(SkipC11Inits);
   }

<SkipInits>{ID}{BN}*"{"               {
      // C++11 style initializer
      lineCount();
      curlyCount = 1;
      BEGIN(SkipC11Inits);
   }

<SkipC11Inits>"{"                     {
      ++curlyCount;
   }

<SkipC11Inits>"}"                     {
      if (--curlyCount <= 0) {
         BEGIN(SkipInits);
      }
   }

SkipC11Attribute>"]]"                 {
      BEGIN(lastC11AttributeContext);
   }

<SkipInits>"{"                        {
      // C++11 style initializer
      unput('{');
      BEGIN( Function );
   }

<SkipCurly>"{"                        {
      ++curlyCount ;
   }

<SkipCurly>"}"/{BN}*("/*!"|"/**"|"//!"|"///")"<!--" |
<SkipCurly>"}"             {
      // */ (editor syntax fix)

      if (curlyCount)  {
         --curlyCount;

      } else {

         if (! current->m_specialLists.isEmpty() && previous) {
            // copy special list items

            for (auto &lii : current->m_specialLists) {
               previous->addSpecialListItem(lii.type, lii.itemId);
            }

            current->m_specialLists.clear();
         }

         if (previous) {
            previous->endBodyLine = yyLineNr;
         }

         BEGIN( lastCurlyContext );
         }
   }

<SkipCurly>"}"{BN}*("/*!"|"/**"|"//!"|"///")"<" {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);
      lineCount();

      if ( curlyCount ) {
         --curlyCount ;

      } else {
         current->endBodyLine = yyLineNr;

         tempEntry = current;          // temporarily switch to the previous entry
         current   = previous;
         previous  = QSharedPointer<Entry>();

         s_docBlockContext   = SkipCurlyEndDoc;
         s_docBlockInBody    = false;

         static bool javadoc_auto_brief = Config::getBool("javadoc-auto-brief");
         static bool qt_auto_brief      = Config::getBool("qt-auto-brief");

         QChar tmpChar = text[text.length() - 2];
         s_docBlockAutoBrief = ( tmpChar == '*' && javadoc_auto_brief ) || ( tmpChar == '!' && qt_auto_brief );

         s_docBlock.resize(0);
         s_docBlockTerm = '}';

         if (text[text.length() - 3] == '/') {
            startCommentBlock(true);
            BEGIN( DocLine );
         } else {
            startCommentBlock(false);
            BEGIN( DocBlock );
         }
      }
   }

<SkipCurlyEndDoc>"}"{BN}*("/*!"|"/**"|"//!"|"///")"<" {

      // */ (editor syntax fix)
      // desc is followed by another one
      QString text = QString::fromUtf8(yytext);

      static bool javadoc_auto_brief = Config::getBool("javadoc-auto-brief");
      static bool qt_auto_brief      = Config::getBool("qt-auto-brief");

      s_docBlockContext = SkipCurlyEndDoc;
      s_docBlockInBody  = false;

      QChar tmpChar = text[text.length() - 2];
      s_docBlockAutoBrief = ( tmpChar == '*' && javadoc_auto_brief ) || ( tmpChar == '!' && qt_auto_brief );

      s_docBlock.resize(0);
      s_docBlockTerm = '}';

      if (text[text.length() - 3] == '/') {
         startCommentBlock(true);
         BEGIN( DocLine );
      } else {
         startCommentBlock(false);
         BEGIN( DocBlock );
      }
    }

<SkipCurlyEndDoc>"}"          {
      // addToBody ("}");
      if (tempEntry) {
         // we can only switch back to current if no new item was created
         current   = tempEntry;
         tempEntry = QSharedPointer<Entry>();
      }
      BEGIN( lastCurlyContext );
   }

<SkipCurly>\"                {
      lastStringContext = SkipCurly;
      BEGIN( SkipString );
   }

<SkipCurly>^{B}*"#"                           {
      if (insidePHP) {
         REJECT;
      }

      BEGIN( SkipCurlyCpp );
   }

<SkipCurly,SkipC11Inits,SkipInits,SkipC11Attribute>\n          {
      lineCount();
   }

<SkipCurly,SkipCurlyCpp>"<<<"                 {
      if (! insidePHP) {
         REJECT;
      } else {
         lastHereDocContext = YY_START;
         BEGIN(HereDoc);
      }
   }

<SkipCurly,SkipCurlyCpp>[^\n#"'@\\/{}<]+ {
      // for s_column updates
      lineCount();
   }

<SkipCurlyCpp>\n        {
      lineCount();
      lastCurlyContext = FindMembers;
      BEGIN( SkipCurly );
   }

<SkipCurlyCpp>\\[\r]*"\n"[\r]*      {
      lineCount();
   }

<SkipInits,SkipC11Inits,SkipCurly,SkipCurlyCpp,SkipC11Attribute>"/*"    {
      // */ (editor syntax fix)
      lastCContext = YY_START;
      BEGIN(SkipComment);
   }

<SkipInits,SkipC11Inits,SkipCurly,SkipCurlyCpp,SkipC11Attribute>"//"   {
      lastCContext = YY_START;
      BEGIN(SkipCxxComment);
   }

<SkipInits,SkipC11Inits,SkipC11Attribute>"("      {
      roundCount = 0;
      lastSkipRoundContext = YY_START;
      BEGIN(SkipRound);
    }

<SkipInits,SkipC11Inits,SkipC11Attribute>\"      {
      lastStringContext = YY_START;
      BEGIN( SkipString );
   }

<SkipInits>;                           {
      warn(yyFileName, yyLineNr, "Found a ';' while parsing an initializer list. "
            "Some macros which are used without semicolons may not be parsed accurately.\n");

      BEGIN( FindMembers );
   }

<SkipInits,SkipCurly,SkipCurlyCpp>"#"   {
      if (!insidePHP) {
         REJECT;
      }

      lastCContext = YY_START;
      BEGIN(SkipCxxComment);
   }

<SkipInits,SkipCurly,SkipCurlyCpp>@\"   {
      if (! insideCSharp) {
         REJECT;
      }

      // C# verbatim string
      lastSkipVerbStringContext = YY_START;

      s_skipVerbEntry = current;
      s_skipVerbEnum  = EntryKey::Initial_Value;

      BEGIN(SkipVerbString);
   }

<SkipInits,SkipCurly,SkipCurlyCpp>{CHARLIT}       {
      if (insidePHP) {
         REJECT;
      }
   }

<SkipInits,SkipCurly,SkipCurlyCpp>\'              {
      if (insidePHP) {
         lastStringContext = YY_START;
         BEGIN(SkipPHPString);
      }
   }

<SkipInits,SkipC11Inits,SkipCurly,SkipCurlyCpp,SkipC11Attribute>.  {
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
      current->name    = "";
      current->section = Entry::EMPTY_SEC;

      current->setData(EntryKey::Member_Type, "");
      current->setData(EntryKey::Member_Args, "");

      current->argList.clear();

      BEGIN( FindMembers ) ;
   }

<Bases>";"        {
      bool isSingleton = current->m_traits.hasTrait(Entry::Virtue::Singleton);
      bool isService   = current->m_traits.hasTrait(Entry::Virtue::Service);

      if (insideIDL && (isSingleton || isService)) {
         // in UNO IDL a service or singleton may be defined
         // completely like this: "service Foo : XFoo;"

         if (! current->name.isEmpty() && ! current_root->name.isEmpty()) {
            prependScope();
         }

         current->name = current->name.trimmed();

         // there can be only one base class here
         if (! baseName.isEmpty()) {
            current->extends.append(BaseInfo(baseName, Public, Normal));
            baseName.resize(0);
         }

         current_root->addSubEntry( current, current_root);
         current = QMakeShared<Entry>();

      } else {
         current->section = Entry::EMPTY_SEC;
         current->name    = "";

         current->setData(EntryKey::Member_Type, "");
         current->setData(EntryKey::Member_Args, "");

         current->argList.clear();
      }

      BEGIN( FindMembers ) ;
   }

<CompoundName>{SCOPENAME}{BN}*/"<"  {
      QString text = QString::fromUtf8(yytext);

      sharpCount    = 0;
      current->name = text;

      bool isProtocol = current->m_traits.hasTrait(Entry::Virtue::Protocol);

      if (isProtocol) {
         current->name += "-p";
      }

      lineCount();
      lastClassTemplSpecContext = ClassVar;

      if (insideObjC)  {
         // protocol list
         BEGIN( ObjCProtocolList );

      } else if (insideCSharp) {
         // C# generic class

         // current->name += "-g";
         BEGIN( CSGeneric );

      } else  {
         // C++ template specialization

         roundCount = 0;
         BEGIN( ClassTemplSpec );
      }
   }

<CSGeneric>"<"             {
      current->m_templateArgLists.append(ArgumentList());
      currentArgumentList = &current->m_templateArgLists.last();

      s_template_args = "<";
      fullArgString   = s_template_args;

      current->name   += "<";

      s_argEntry  = current;
      s_argEnum   = ArgKey::Entry_Name;

      currentArgumentContext = ClassVar;
      BEGIN( ReadTempArgs );
   }

<ObjCProtocolList>"<"         {
      insideProtocolList=true;
      BEGIN( Bases );
   }

<ClassTemplSpec>">"({BN}*"::"{BN}*{SCOPENAME})?    {
      QString text   = QString::fromUtf8(yytext);
      current->name += text;

      lineCount();

      if (roundCount == 0 && --sharpCount <= 0) {
         current->name = removeRedundantWhiteSpace(current->name);

         bool isProtocol  = current->m_traits.hasTrait(Entry::Virtue::Protocol);

         if (isProtocol) {
            // Objective-C protocol
            unput('{');    // fake start of body
            BEGIN( ClassVar );

         } else {
            BEGIN( lastClassTemplSpecContext );
         }
       }
   }

<ClassTemplSpec>"<"        {
      QString text = QString::fromUtf8(yytext);
      current->name += text;

      if (roundCount == 0) {
         sharpCount++;
      }
   }

<ClassTemplSpec>.          {
      QString text = QString::fromUtf8(yytext);
      current->name += text;
   }

<CompoundName>{SCOPENAME}{BN}*";"   {
      // forward declaration
      QString text = QString::fromUtf8(yytext);

      bool isInterface      = current->m_traits.hasTrait(Entry::Virtue::Interface);
      bool isService        = current->m_traits.hasTrait(Entry::Virtue::Service);

      bool isInterface_Root = current_root->m_traits.hasTrait(Entry::Virtue::Interface);
      bool isService_Root   = current_root->m_traits.hasTrait(Entry::Virtue::Service);
      bool isSingleton_Root = current_root->m_traits.hasTrait(Entry::Virtue::Singleton);

      if (! current->m_templateArgLists.isEmpty()) {
          // found a forward template declaration, this has a purpose of its own
          current->name = text;
          current->name = current->name.left(current->name.length()-1).trimmed();

         QString rn = current_root->name;

         if (! current->name.isEmpty() && ! rn.isEmpty()) {
            prependScope();
         }

         current->m_traits.setTrait(Entry::Virtue::ForwardDecl);

         current_root->addSubEntry(current, current_root);
         current = QMakeShared<Entry>();

      } else if (insideIDL && ( ( (isInterface_Root || isService_Root) && (isInterface)) ||
                ( (isService_Root || isSingleton_Root) && (isService))))  {

         // interface inside of UNO IDL service or interface
         // service inside of UNO IDL service or singleton
         // there may be documentation on the member so do not throw it away

         current->name = text;
         current->name = current->name.left(current->name.length() - 1).trimmed();

         if (isInterface)  {
            current->section = Entry::EXPORTED_INTERFACE_SEC;
         } else {
            current->section = Entry::INCLUDED_SERVICE_SEC;
         }

         current->m_traits.setTrait(Entry::Virtue::Interface, false);
         current->m_traits.setTrait(Entry::Virtue::Service, false);

         current_root->addSubEntry(current, current_root);
         current = QMakeShared<Entry>();
       }

      unput(';');
      current->reset();
      initEntry();

      if (insideObjC) {
         language   = (current->lang = SrcLangExt_Cpp);
         insideObjC = false;
      }

      if (isTypedef) {
         // typedef of a class, put typedef keyword back
         current->prependData(EntryKey::Member_Type, "typedef");
      }

      BEGIN( FindMembers );
   }

<CompoundName>{SCOPENAME}/{BN}*"("  {
      QString text = QString::fromUtf8(yytext);
      current->name = text;
      lineCount();

      if (insideCpp && current->name == "alignas") {
         // C++11

         lastAlignAsContext = YY_START;
         BEGIN( AlignAs );

      } else {
         bool isProtocol = current->m_traits.hasTrait(Entry::Virtue::Protocol);

         if (isProtocol) {
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
      // have a new scope such as a new class
      QString text  = QString::fromUtf8(yytext);
      current->name = text;

      lineCount();
      bool isProtocol = current->m_traits.hasTrait(Entry::Virtue::Protocol);

      if (isProtocol) {
         current->name += "-p";
      }

      if (isProtocol || current->section == Entry::OBJCIMPL_SEC) {
         unput('{');       // fake start of body
      }

      BEGIN( ClassVar );
   }

<CompoundName>{CSSCOPENAME}           {
      // C# style scope
      QString text = QString::fromUtf8(yytext);
      current->name = substitute(text, ".", "::");
      lineCount();
      BEGIN( ClassVar );
   }

<ClassVar>{SCOPENAME}{BN}*/"("      {
      QString text = QString::fromUtf8(yytext);

      if (insideIDL && text.startsWith("switch") == 0 && ! isId(text[6])) {
         // Corba IDL style union
         roundCount = 0;
         BEGIN(SkipUnionSwitch);

      }  else {
         addType(current);
         current->name = text;
         current->name = current->name.trimmed();

         lineCount();
         BEGIN( FindMembers );
      }
   }

<ClassVar>","           {
      if (isTypedef) {
         // multiple types in one typedef
         unput(',');
         current->prependData(EntryKey::Member_Type, "typedef ");

         BEGIN(FindMembers);

      } else {
         // Multiple class forward declaration
      }
   }

<ClassVar>("sealed"|"abstract")/{BN}*(":"|"{") {
      QString text = QString::fromUtf8(yytext);

      if (insideCli) {
         if ( text[0] == 's')   {
            // sealed, C#
            current->m_traits.setTrait(Entry::Virtue::SealedClass);

         } else  {
            // abstract
            current->m_traits.setTrait(Entry::Virtue::AbstractClass);

         }

         BEGIN( ClassVar );

      } else {
         REJECT;

      }
   }

<ClassVar>{ID}             {
      QString text = QString::fromUtf8(yytext);

      if (insideIDL && text == "switch") {
          // Corba IDL style union
          roundCount=0;
          BEGIN(SkipUnionSwitch);

      } else if ((insideJava || insidePHP || insideJS) && (text == "implements" || text =="extends") ) {
          current->setData(EntryKey::Member_Type, "");
          baseProt = Public;
          baseVirt = Normal;
          baseName.resize(0);
          BEGIN( BasesProt );

      } else if (insideCSharp && text == "where")  {
         // C# type contraint

         current->typeConstr = ArgumentList();
         current->typeConstr.append(Argument());

         lastCSConstraint = YY_START;
         BEGIN( CSConstraintName );

      } else if (insideCli &&  text == "abstract") {
         current->m_traits.setTrait(Entry::Virtue::Abstract);

      } else if (insideCli && text == "sealed") {
         current->m_traits.setTrait(Entry::Virtue::Sealed);

      } else if (text == "final") {
         current->m_traits.setTrait(Entry::Virtue::Final);

      } else {

         if (current->section == Entry::ENUM_SEC) {
            // found "enum a b" -> variable
            current->section = Entry::VARIABLE_SEC ;
         }

         current->appendData(EntryKey::Member_Type, " " + current->name);
         current->name = text;

         if (nameIsOperator(current->name)) {
            BEGIN( Operator );
         }
      }
   }

<ClassVar>[(\[]            {
      QString text = QString::fromUtf8(yytext);

      if (insideObjC && text[0] == '(') {
         // class category

         current->name += '(';
         current->m_traits.setTrait(Entry::Virtue::Category);

         BEGIN( ClassCategory );

      } else {
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
      QString text = QString::fromUtf8(yytext);
      fullArgString.resize(0);
      lastCopyArgChar = '#';       // end marker
      lastCommentInArgContext = YY_START;

      if (text[1] == '/') {
         BEGIN( CopyArgCommentLine );
      } else {
         BEGIN( CopyArgComment );
      }
   }

<CSConstraintType,CSConstraintName>"#"    {
      // artificially inserted token to signal end of comment block
      current->typeConstr.last().docs = fullArgString;
   }

<CSConstraintType>"{"         {
      // end of type constraint reached
      // parse documentation of the constraints
      handleParametersCommentBlocks(current->typeConstr);
      unput('{');
      BEGIN( lastCSConstraint );
   }

<CSConstraintType,CSConstraintName>";"  {
      handleParametersCommentBlocks(current->typeConstr);
      unput(';');
      BEGIN( lastCSConstraint );
   }

<CSConstraintName>":"                   {
      BEGIN( CSConstraintType );
   }

<CSConstraintName>{ID}                  {
      // parameter name
      QString text = QString::fromUtf8(yytext);
      current->typeConstr.last().name = text;
   }

<CSConstraintType>"where"     {
      // another constraint for a different param
      current->typeConstr.append(Argument());
      BEGIN( CSConstraintName );
   }

<CSConstraintType>({ID}".")*{ID}("<"{ID}">")?("()")?  {
      QString text = QString::fromUtf8(yytext);
      if (current->typeConstr.last().type.isEmpty()) {
         // first type constraint for this parameter

         current->typeConstr.last().type = text;

      } else  {
         // new type constraint for same parameter

         QString name = current->typeConstr.last().name;
         current->typeConstr.append(Argument());
         current->typeConstr.last().name = name;
         current->typeConstr.last().type = text;
      }
   }

<CSConstraintName,CSConstraintType>\n  {
      lineCount();
   }

<CSConstraintName,CSConstraintType>.   {
   }

<ClassCategory>{ID}        {
      QString text = QString::fromUtf8(yytext);
      current->name += text;
   }

<ClassCategory>")"/{BN}*"{"      {
      current->name += ')';
      BEGIN( ClassVar );
   }

<ClassCategory>")"/{BN}*"<"      {
      current->name+=')';
      BEGIN( ObjCProtocolList );
   }

<ClassCategory>")"         {
      current->name += ')';

      if ((current->section & Entry::Protocol) || current->section == Entry::OBJCIMPL_SEC) {
         unput('{'); // fake start of body

      } else {
         // category has no variables so push back an empty body
         unput('}');
         unput('{');
      }

      BEGIN( ClassVar );
   }

<ClassVar>":"           {
      if (current->section == Entry::VARIABLE_SEC) {
         // enum A B:2, see bug 748208

         current->appendData(EntryKey::Member_Bitfields, ":");
         current->setData(EntryKey::Member_Args,         "" );
         BEGIN(BitFields);

      } else if (current->section == Entry::ENUM_SEC) {
         // enum E:2, see bug 313527,
         // or C++11 style enum: 'E : unsigned int {...}'

         current->setData(EntryKey::Member_Args, "");
         BEGIN(EnumBaseType);

      } else {
         current->setData(EntryKey::Member_Type, "");

         bool isInterface = current->m_traits.hasTrait(Entry::Virtue::Interface);
         bool isStruct    = current->m_traits.hasTrait(Entry::Virtue::Struct);
         bool isRef       = current->m_traits.hasTrait(Entry::Virtue::Ref);
         bool isValue     = current->m_traits.hasTrait(Entry::Virtue::Value);

         if (isInterface || isStruct || isRef || isValue ||
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
      QString text = QString::fromUtf8(yytext);
      unput(*yytext);

      if (isTypedef) {
         // typedef of a class, put typedef keyword back
         current->prependData(EntryKey::Member_Type, "typedef");
      }

      if ((text[0] == '*' || text[0] == '&') && current->section == Entry::ENUM_SEC) {
         // found "enum a *b" -> variable
         current->section = Entry::VARIABLE_SEC;
      }
      BEGIN( FindMembers );
   }

<Bases,ClassVar>"///"/[^/]              {
      QString text = QString::fromUtf8(yytext);

      if (! insideObjC) {
         REJECT;

      } else {
        lineCount();

        current->appendData(EntryKey::Source_Text, text);

        current->setData(EntryKey::File_Name, yyFileName);
        current->startLine   = yyLineNr;
        current->startColumn = yyColNr;

        curlyCount           = 0;

        BEGIN( ReadBodyIntf );
      }
   }

<Bases,ClassVar>("//"{B}*)?"/**"/[^/*]  |
<Bases,ClassVar>("//"{B}*)?"/*!"        |
<Bases,ClassVar>"//!"                   |
<Bases,ClassVar>[\-+]{BN}*       {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      if (! insideObjC) {
         REJECT;

      } else {
         lineCount();

         current->appendData(EntryKey::Source_Text, text);

         current->setData(EntryKey::File_Name, yyFileName);
         current->startLine   = yyLineNr;
         current->startColumn = yyColNr;
         curlyCount           = 0;

         BEGIN( ReadBodyIntf );
      }
   }

<CompoundName,ClassVar>{B}*"{"{B}*  {
      static const bool extractAnonNS = Config::getBool("extract-anon-namespaces");

      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;

      current->name = removeRedundantWhiteSpace(current->name);

      if (current->name.isEmpty() && ! isTypedef)  {
         // anonymous compound

         if (current->section == Entry::NAMESPACE_SEC) {
            // allow reopening of anonymous namespaces

            if (extractAnonNS) {
               // use visible name
               current->name = "anonymous_namespace{" + stripPath(current->getData(EntryKey::File_Name)) + "}";

            } else {
               // use invisible name
               current->name = QString("@%1").arg(anonNSCount);
            }

         } else {
            current->name = QString("@%1").arg(anonCount++);
         }
      }

      curlyCount = 0;
      bool ok = true;

      // not a nested struct inside an @interface section
      if (current_root) {

         bool isInterface      = current->m_traits.hasTrait(Entry::Virtue::Interface);
         bool isProtocol       = current->m_traits.hasTrait(Entry::Virtue::Protocol);
         bool isCategory       = current->m_traits.hasTrait(Entry::Virtue::Category);

         bool isInterface_Root = current_root->m_traits.hasTrait(Entry::Virtue::Interface);

         if (! isInterface_Root && ( (isInterface || isProtocol || isCategory ||
               current->section == Entry::OBJCIMPL_SEC) ) && insideObjC) {

            // ObjC body that ends with @end
            ok = false;

            BEGIN( ReadBodyIntf );
         }
      }

      if (ok) {

         if (current->section == Entry::NAMESPACE_SEC) {
            // namespace body
            BEGIN( ReadNSBody );

         } else {
            BEGIN( ReadBody );

         }
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
      if (! insideCli) {
         REJECT;
      }

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
      QString text = QString::fromUtf8(yytext);

      if (! insidePHP) {
         REJECT;
      } else {
         // PHP base class of the form \Ns\Cl or Ns\Cl
         lineCount();

         baseName += substitute(text,"\\","::");
         current->appendData(EntryKey::Member_Args, " " + text);
      }
   }

<Bases>("::")?{BN}*({ID}{BN}*"::"{BN}*)*{ID}    {
      QString text = QString::fromUtf8(yytext);
      lineCount();

      if (insideCSharp && text.trimmed() == "where") {
         // type contraint for a class

         current->typeConstr = ArgumentList();
         current->typeConstr.append(Argument());

         lastCSConstraint = YY_START;
         BEGIN( CSConstraintName );

     } else {
       baseName += text;
       current->appendData(EntryKey::Member_Args, " " + text);
     }
   }

<Bases>{BN}*{ID}("."{ID})*       {
      // Java style class
      QString text = QString::fromUtf8(yytext);
      QString name = substitute(text, ".","::");

      baseName += name;

      current->appendData(EntryKey::Member_Args, " " + name);
   }

<ClassVar,Bases>\n/{BN}*[^{, \t\n]  {
      if (!insideObjC) {
         REJECT;
      } else {
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
      QString text = QString::fromUtf8(yytext);
      current->name += text[0];

      sharpCount = 1;
      roundCount = 0;

      lastSkipSharpContext = YY_START;
      specName             = &current->name;

      BEGIN ( Specialization );
   }

<Bases>{BN}*"<"                         {
      QString text = QString::fromUtf8(yytext);
      lineCount();
      sharpCount=1;
      roundCount=0;
      lastSkipSharpContext = YY_START;

      if (insideObjC)  {
         // start of protocol list
         unput(',');

      } else  {
         // template specialization

         //if (insideCSharp) // generic
         //{
         //  baseName+="-g";
         //}

         s_template_args = text;
         specName = &s_template_args;

         BEGIN ( Specialization );
      }
   }

<Specialization>"<"        {
      QString text = QString::fromUtf8(yytext);
      *specName += text[0];

      if (roundCount == 0) {
         sharpCount++;
      }
   }

<Specialization>">"        {
      QString text = QString::fromUtf8(yytext);
      *specName += text[0];

      if (roundCount == 0 && --sharpCount <= 0) {
         baseName += removeRedundantWhiteSpace(*specName);
         BEGIN(lastSkipSharpContext);
      }
   }

<Specialization>{BN}+         {
      lineCount();
      *specName +=' ';
   }

<Specialization>"<<"          {
      QString text = QString::fromUtf8(yytext);
      *specName += text;
   }

<Specialization>">>"/{B}*"::"       {
      // M$ C++ extension to allow >> to close a template...
      unput('>');
      unput(' ');
      unput('>');
   }

<Specialization>">>"          {
      QString text = QString::fromUtf8(yytext);
      if (insideCSharp) {
         // for C# >> ends a nested template
         REJECT;

      } else  {
         // for C++ >> is a bitshift operator and > > would end a nested template
         // we require the bitshift to be enclosed in braces
         // See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2005/n1757.html

         if (roundCount > 0) {
            *specName += text;

         } else {
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
      QString text = QString::fromUtf8(yytext);
      *specName += text[0];
      roundCount++;
   }

<Specialization>")"        {
      QString text = QString::fromUtf8(yytext);
      *specName += text[0];
      roundCount--;
   }

<Specialization>.          {
      QString text = QString::fromUtf8(yytext);
      *specName += text[0];
   }

<SkipRound>"("             {
      ++roundCount;
   }

<SkipRound>")"             {
      if (--roundCount < 0) {
         BEGIN ( lastSkipRoundContext );
      }
   }

<SkipRound>\"           {
      lastStringContext=SkipRound;
      BEGIN(SkipString);
   }

<Bases>","|(">"({BN}*"{")?)|({BN}+"implements"{BN}*)  {
      QString text = QString::fromUtf8(yytext);
      lineCount();

      if (insideProtocolList) {
         baseName += "-p";
      } else {
         current->appendData(EntryKey::Member_Args, ",");
      }

      current->name = removeRedundantWhiteSpace(current->name);

      if (! baseName.isEmpty()) {
         current->extends.append(BaseInfo(baseName, baseProt, baseVirt));
      }

      bool isInterface = current->m_traits.hasTrait(Entry::Virtue::Interface);
      bool isStruct    = current->m_traits.hasTrait(Entry::Virtue::Struct);

      if ( isInterface || isStruct || insideJava || insidePHP || insideCSharp ||
               insideD || insideObjC || insideIDL) {

         baseProt = Public;

      } else {
         baseProt = Private;
      }

      baseVirt=Normal;
      baseName.resize(0);

      if (text[0] == '>') {
         // end of a ObjC protocol list
         insideProtocolList = false;

         if (text.length() == 1) {
            unput('{'); // dummy start body
         } else {
            yyless(1);
         }

      } else {

         if (text[0] == ',' && insideObjC) {
            // Begin of protocol list
            insideProtocolList=true;
         }

         BEGIN(BasesProt);
      }
   }

<Bases>{B}*"{"{B}*         {
      current->setData(EntryKey::File_Name, yyFileName);
      current->startLine   = yyLineNr;
      current->startColumn = yyColNr;

      current->name = removeRedundantWhiteSpace(current->name);

      if (! baseName.isEmpty()) {
         current->extends.append(BaseInfo(baseName, baseProt, baseVirt));
      }

      curlyCount = 0;

      if (insideObjC)  {
         BEGIN( ReadBodyIntf );
      }  else {
         BEGIN( ReadBody ) ;
      }
   }

<SkipUnionSwitch>{B}*"("      {
      roundCount++;
   }

<SkipUnionSwitch>")"          {
      if (--roundCount == 0 ) {
         BEGIN(ClassVar);
      }
   }

<SkipUnionSwitch>\n        {
      lineCount();
   }

<SkipUnionSwitch>.
<Comment>{BN}+             {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
      lineCount() ;
   }

<Comment>"/*"           {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
   }

<Comment>"//"           {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
   }

<Comment>{CMD}("code"|"verbatim")   {
      QString text = QString::fromUtf8(yytext);
      insideCode   = true;
      current->appendData(EntryKey::Source_Text, text);
   }

<Comment>{CMD}("endcode"|"endverbatim")   {
      QString text = QString::fromUtf8(yytext);
      insideCode   = false;
      current->appendData(EntryKey::Source_Text, text);
   }

<Comment>[^ \.\t\r\n\/\*]+       {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);
   }

<Comment>"*/"           {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text);

      if (! insideCode) {
         BEGIN(lastContext);
      }
   }

<Comment>.           {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Source_Text, text[0]);
   }

<FindMembers,FindFields,MemberSpec,FuncQual,SkipCurly,Operator,ClassVar,SkipInits,SkipC11Inits,SkipC11Attribute,Bases,OldStyleArgs>("//"{B}*)?"/*!" {

      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      removeSlashes = (text[1] == '/');
      tmpDocType    =- 1;

      if (! current->getData(EntryKey::Main_Docs).isEmpty()) {
         current->appendData(EntryKey::Main_Docs, "\n\n");
      }

      lastDocContext = YY_START;
      if (current_root->section & Entry::SCOPE_MASK) {
         current->setData(EntryKey::Class_Name, current_root->name + "::");
      }

      s_docBlockContext   = YY_START;
      s_docBlockInBody    = (YY_START == SkipCurly);

      s_docBlockAutoBrief = Config::getBool("qt-auto-brief");

      s_docBlock = QString(computeIndent(text, s_column), QChar(' '));

      if (s_docBlockAutoBrief) {
         current->setData(EntryKey::Brief_File, yyFileName);
         current->briefLine = yyLineNr;
      }

      startCommentBlock(false);
      BEGIN( DocBlock );
   }

<FindMembers,FindFields,MemberSpec,FuncQual,SkipCurly,Operator,ClassVar,SkipInits,Bases,OldStyleArgs>("//"{B}*)?"/**"/[^/*] {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      removeSlashes  = (text[1] == '/');
      lastDocContext = YY_START;

      if (current_root->section & Entry::SCOPE_MASK) {
         current->setData(EntryKey::Class_Name, current_root->name+"::");
      }

      s_docBlockContext  = YY_START;
      s_docBlockInBody   = YY_START == SkipCurly;

      static bool javadocAutoBrief = Config::getBool("javadoc-auto-brief");
      s_docBlockAutoBrief = javadocAutoBrief;

      s_docBlock = QString(computeIndent(text, s_column), QChar(' '));

      if (s_docBlockAutoBrief) {
         current->setData(EntryKey::Brief_File, yyFileName);
         current->briefLine = yyLineNr;
      }

      startCommentBlock(false);
      BEGIN( DocBlock );
   }

<FindMembers,FindFields,MemberSpec,SkipCurly,FuncQual,Operator,ClassVar,Bases,OldStyleArgs>"//!" {
      QString text = QString::fromUtf8(yytext);

      tmpDocType     = -1;
      lastDocContext = YY_START;

      if (current_root->section & Entry::SCOPE_MASK) {
         current->setData(EntryKey::Class_Name,  current_root->name+"::");
      }

      s_docBlockContext   = YY_START;
      s_docBlockInBody    = YY_START == SkipCurly;
      s_docBlockAutoBrief = false;

      s_docBlock = QString(computeIndent(text, s_column), QChar(' '));

      startCommentBlock(current->getData(EntryKey::Brief_Docs).isEmpty());
      BEGIN( DocLine );
   }

<FindMembers,FindFields,MemberSpec,SkipCurly,FuncQual,Operator,ClassVar,Bases,OldStyleArgs>"///"/[^/] {
      QString text = QString::fromUtf8(yytext);

      tmpDocType     = -1;
      lastDocContext = YY_START;

      if (current_root->section & Entry::SCOPE_MASK) {
         current->setData(EntryKey::Class_Name, current_root->name+"::");
      }

      s_docBlockContext   = YY_START;
      s_docBlockInBody    = YY_START == SkipCurly;
      s_docBlockAutoBrief = false;

      s_docBlock = QString(computeIndent(text, s_column), QChar(' '));

      startCommentBlock(current->getData(EntryKey::Brief_Docs).isEmpty());
      BEGIN( DocLine );
   }

<FindMembers>"extern"{BN}*"\"C"("++")?"\""{BN}*("{")?  {
      lineCount();
      externC = true;
   }

<FindMembers>"{"        {
      QString text = QString::fromUtf8(yytext);

      bool isAttribute = current->m_traits.hasTrait(Entry::Virtue::Attribute);
      QString tmpType  = current->getData(EntryKey::Member_Type);

      if (externC) {
         externC = false;

      } else if (insideCSharp && ! current->name.isEmpty() && ! tmpType.isEmpty())  {

         if (containsWord(tmpType, "event")) {
            // event
            current->mtype = mtype = MethodTypes::Event;

         } else {
            // property
            current->mtype = mtype = MethodTypes::Property;
         }

         current->bodyLine = yyLineNr;
         curlyCount = 0;

         BEGIN( CSAccessorDecl );

      }  else if (insideIDL && isAttribute) {
         // UNO IDL: attributes may have setter and getter
         current->setData(EntryKey::Exception_Spec, " {");

         BEGIN(UNOIDLAttributeBlock);

      } else {
         if ((insideJava || insideCSharp || insideD) && current->name.isEmpty()) {
            // static Java initializer
            needsSemi = false;

            if (current->stat) {
               current->name = "[static initializer]";
               current->setData(EntryKey::Member_Type, "");
            } else {
               current->name = "[instance initializer]";
            }

            unput(*yytext);
            BEGIN( Function );

         } else {
            // C++11 style initializer list
            current->setData(EntryKey::Initial_Value, text);

            current->bodyLine      = yyLineNr;
            lastInitializerContext = YY_START;
            initBracketCount       = 1;

            BEGIN(ReadInitializer);
         }
      }
   }

<CSAccessorDecl>"{"          {
      curlyCount++;
   }

<CSAccessorDecl>"}"{BN}*"="   {

      if (curlyCount != 0) {
        REJECT;

      } else {

         mtype = MethodTypes::Method;
         virt  = Normal;

         // default value
         unput('=');

         BEGIN(FindMembers);
      }
   }

<CSAccessorDecl>"}"          {
      if (curlyCount != 0) {
         curlyCount--;

      } else {
         mtype = MethodTypes::Method;
         virt  = Normal;
         unput(';');
         BEGIN(FindMembers);
      }
   }

<CSAccessorDecl>"private "{BN}*"set"      {
      if (curlyCount == 0) {
         current->m_traits.setTrait(Entry::Virtue::PrivateSettable);
      }
   }

<CSAccessorDecl>"protected "{BN}*"set"    {
      if (curlyCount == 0) {
         current->m_traits.setTrait(Entry::Virtue::ProtectedSettable);
      }
   }

<CSAccessorDecl>"private "{BN}*"get"      {
      if (curlyCount == 0) {
         current->m_traits.setTrait(Entry::Virtue::PrivateGettable);
      }
   }

<CSAccessorDecl>"protected "{BN}*"get"    {
      if (curlyCount == 0)  {
         current->m_traits.setTrait(Entry::Virtue::ProtectedGettable);
      }
   }

<CSAccessorDecl>"set"         {
      if (curlyCount == 0)
         current->m_traits.setTrait(Entry::Virtue::Settable);
   }

<CSAccessorDecl>"get"         {
      if (curlyCount == 0)
         current->m_traits.setTrait(Entry::Virtue::Gettable);
   }

<CSAccessorDecl>"add"         {
      if (curlyCount == 0)
         current->m_traits.setTrait(Entry::Virtue::Addable);
   }

<CSAccessorDecl>"remove"      {
      if (curlyCount == 0)
         current->m_traits.setTrait(Entry::Virtue::Removable);
   }

<CSAccessorDecl>"raise"       {
      if (curlyCount == 0)
         current->m_traits.setTrait(Entry::Virtue::Raisable);
   }

<CSAccessorDecl>.          {
   }

<CSAccessorDecl>\n         {
      lineCount();
   }


 /*  Documentation block related rules */

 /*  ---- Single line comments ------ */
<DocLine>[^\n]*"\n"[ \t]*"//"[/!][<]?       {
      // continuation of multiline C++-style comment
      QString text = QString::fromUtf8(yytext);

      s_docBlock += text;

      int markerLen = 3;
      if (text.endsWith('<') ) {
         markerLen = 4;
      }

      s_docBlock.resize(s_docBlock.length() - markerLen);
      lineCount();
}

<DocLine>{B}*"///"[/]+{B}*/"\n"     {
      // ignore marker line (see bug700345)
      handleCommentBlock(s_docBlock, current->getData(EntryKey::Brief_Docs).isEmpty());
      BEGIN( s_docBlockContext );
   }

<DocLine>[^\n]*/"\n"{B}*"//"[!/]{B}*{CMD}"}"	{
      // next line is an end group marker
      QString text = QString::fromUtf8(yytext);

  		s_docBlock += text;
		handleCommentBlock(s_docBlock, current->getData(EntryKey::Brief_Docs).isEmpty());

	   BEGIN( s_docBlockContext );
   }

<DocLine>[^\n]*/"\n"          {
      // whole line
      QString text = QString::fromUtf8(yytext);

      s_docBlock += text;
      handleCommentBlock(s_docBlock, current->getData(EntryKey::Brief_Docs).isEmpty());
      BEGIN( s_docBlockContext );
   }


 /* ---- Comments blocks ------ */
<DocBlock>"*"*"*/"         {
      // end of comment block
      handleCommentBlock(s_docBlock, false);
      BEGIN( s_docBlockContext);
   }

<DocBlock>^{B}*"*"+/[^/]      {
      QString text = QString::fromUtf8(yytext);
      s_docBlock += QString(computeIndent(text, s_column), QChar(' '));
   }

<DocBlock>^{B}*("//")?{B}*"*"+/[^//a-z_A-Z0-9*]    {
      // start of a comment line
      QString text = QString::fromUtf8(yytext);
      s_docBlock += QString(computeIndent(text, s_column), QChar(' '));
   }

<DocBlock>^{B}*("//"){B}*     {
      // strip embedded C++ comments when at the start of a line
   }

<DocBlock>"//"             {
      // slashes in the middle of a comment block
      QString text = QString::fromUtf8(yytext);
      s_docBlock += text;
   }

<DocBlock>"/*"             {
      // */ (editor syntax fix)
      // start of a new comment in the middle of a comment block
      QString text = QString::fromUtf8(yytext);
      s_docBlock += text;
   }

<DocBlock>("@@"|"\\\\"){ID}/[^a-z_A-Z0-9] {
      // */ (editor syntax fix)
      // escaped command
      QString text = QString::fromUtf8(yytext);
      s_docBlock += text;
   }

<DocBlock>{CMD}("f$"|"f["|"f{")          {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      s_docBlock += text;
      s_docBlockName = text.mid(1);

      if (s_docBlockName.at(1) == '{') {
         s_docBlockName[1] = '}';
      }

      s_fencedSize=0;
      s_nestedComment = false;

      BEGIN(DocCopyBlock);
   }

<DocBlock>{B}*"<"{PRE}">"     {
      QString text = QString::fromUtf8(yytext);

      s_docBlock      += text;
      s_docBlockName  = "<pre>";

      s_fencedSize    = 0;
      s_nestedComment = false;

      BEGIN(DocCopyBlock);
   }

<DocBlock>{CMD}("verbatim"|"latexonly"|"htmlonly"|"xmlonly"|"manonly"|"dot"|"code")/[^a-z_A-Z0-9]  {
      // verbatim command (which could contain nested comments)
      QString text = QString::fromUtf8(yytext);

      s_docBlock     += text;
      s_docBlockName = text.mid(1);

      s_fencedSize    = 0;
      s_nestedComment = false;

      BEGIN(DocCopyBlock);
   }

<DocBlock>^({B}*"*"+)?{B}{0,3}"~~~"[~]*    {
      QString text = QString::fromUtf8(yytext);

      s_docBlock     += substitute(text, "*", " ");
      s_docBlockName = "~~~";

      s_fencedSize    = text.length();
      s_nestedComment = false;

      BEGIN(DocCopyBlock);
   }

<DocBlock>^({B}*"*"+)?{B}{0,3}"```"[`]*    {
      QString text = QString::fromUtf8(yytext);

      s_docBlock        += substitute(text, "*", " ");
      s_docBlockName    = "```";

      s_fencedSize    = text.length();
      s_nestedComment = false;

      BEGIN(DocCopyBlock);
   }

<DocBlock>{B}*"<code>"                  {
      QString text = QString::fromUtf8(yytext);

      if (insideCSharp) {
         s_docBlock     += text;
         s_docBlockName = "<code>";

         s_nestedComment = false;
         BEGIN(DocCopyBlock);

      } else {
         REJECT;
      }
   }

<DocBlock>[^@*~\/\\\n]+          {
      // any character that is not special
      QString text = QString::fromUtf8(yytext);
      s_docBlock += text;
   }

<DocBlock>\n            {
      // newline
      QString text = QString::fromUtf8(yytext);

      lineCount();
      s_docBlock += text[0];
   }

<DocBlock>.             {
      // command block
      QString text = QString::fromUtf8(yytext);
      s_docBlock += text[0];
   }

 /* ---- Copy verbatim sections ------ */

<DocCopyBlock>"</"{PRE}">"       {
      // end of a <pre> block
      QString text = QString::fromUtf8(yytext);

      s_docBlock += text;

      if (s_docBlockName == "<pre>") {
         BEGIN(DocBlock);
      }
   }

<DocCopyBlock>"</"{CODE}">"      {
      // end of a <code> block
      QString text = QString::fromUtf8(yytext);

      s_docBlock += text;

      if (s_docBlockName == "<code>") {
         BEGIN(DocBlock);
      }
   }

<DocCopyBlock>[\\@]("f$"|"f]"|"f}")     {
      QString text = QString::fromUtf8(yytext);

      s_docBlock += text;
      BEGIN(DocBlock);
   }

<DocCopyBlock>[\\@]("endverbatim"|"endlatexonly"|"endhtmlonly"|"endxmlonly"|"enddocbookonly"|"endmanonly"|"enddot"|"endcode")/[^a-z_A-Z0-9] {
      // end of verbatim block
      QString text = QString::fromUtf8(yytext);

      s_docBlock += text;

      if (text.mid(4) == s_docBlockName) {
         BEGIN(DocBlock);
      }
   }

<DocCopyBlock>^{B}*"*"+/{BN}+       {
      // start of a comment line
      QString text = QString::fromUtf8(yytext);

      if (s_docBlockName == "verbatim") {
         REJECT;

      } else if (s_docBlockName == "code") {
         REJECT;

      } else {
         s_docBlock += QString(computeIndent(text, 0), QChar(' '));
      }
   }

<DocCopyBlock>^{B}*"*"+/{BN}+"*"{BN}*  {
      // start of a comment line with two *'s
      QString text = QString::fromUtf8(yytext);

      if (s_docBlockName == "code") {
         s_docBlock += QString(computeIndent(text, 0), QChar(' '));
      } else {
         REJECT;
      }
   }

<DocCopyBlock>^{B}*"*"+/({ID}|"(")  {
      // Assume *var or *(... is part of source code (see bug723516)
      QString text = QString::fromUtf8(yytext);

      if (s_docBlockName == "code") {
         s_docBlock += QString(computeIndent(text, -1), QChar(' ')) + "*";

       } else {
         REJECT;
       }
   }

<DocCopyBlock>^{B}*"*"+/{BN}*       {
      // start of a comment line with one *
      QString text = QString::fromUtf8(yytext);

      if (s_docBlockName == "code") {

         if (s_nestedComment) {
            // keep * it is part of the code

            s_docBlock += QString(computeIndent(text, -1), QChar(' ')) + "*";

         } else {
            // remove * it is part of the comment block

            s_docBlock += QString(computeIndent(text, 0), QChar(' '));
         }

      } else {
         REJECT;
      }
   }

<DocCopyBlock>^({B}*"*"+)?{B}{0,3}"~~~"[~]*        {
      QString text = QString::fromUtf8(yytext);

      s_docBlock += substitute(text, "*", " ");

      if (s_fencedSize == text.length()) {
         BEGIN(DocBlock);
      }
   }

<DocCopyBlock>^({B}*"*"+)?{B}{0,3}"```"[`]*        {
      QString text = QString::fromUtf8(yytext);

      s_docBlock += substitute(text, "*", " ");

      if (s_fencedSize == text.length()) {
         BEGIN(DocBlock);
      }
   }

<DocCopyBlock>[^\<@/*\]~\$\\\n]+    {
      // */ (editor syntax fix)
      // any character that is not special
      QString text = QString::fromUtf8(yytext);
      s_docBlock += yytext;
   }

<DocCopyBlock>"/*"|"*/"|"//"     {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      if (text[1] == '*') {
         s_nestedComment = true;

      } else if (text[0] == '*') {
         s_nestedComment = false;
      }

      s_docBlock += text;
   }

<DocCopyBlock>\n        {
      // newline
      QString text = QString::fromUtf8(yytext);

      s_docBlock += text[0];
      lineCount();
   }

<DocCopyBlock>.            {
      // any other character
      QString text = QString::fromUtf8(yytext);
      s_docBlock += text[0];
   }

<DocCopyBlock><<EOF>>         {
      warn(yyFileName,yyLineNr, "Reached end of file while inside a %s block\n"
         "The command which ends the block appears to be missing\n", csPrintable(s_docBlockName));
      yyterminate();
   }


    /* ------------- Prototype parser -------------- */

<Prototype>"operator"{B}*"("{B}*")"    {
      QString text = QString::fromUtf8(yytext);
      current->name += text;
   }

<Prototype>"("                 {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);

      fullArgString = current->getData(EntryKey::Member_Args);

      s_argEntry = current;
      s_argEnum  = ArgKey::Member_Args;

      currentArgumentContext = PrototypeQual;

      BEGIN( ReadFuncArgType );
   }

<Prototype>"("({ID}"::")*({B}*[&*])+   {
      QString text = QString::fromUtf8(yytext);

      current->appendData(EntryKey::Member_Type, current->name + text);
      current->name = "";
      BEGIN( PrototypePtr );
   }

<PrototypePtr>{SCOPENAME}     {
      QString text = QString::fromUtf8(yytext);
      current->name += text;
   }

<PrototypePtr>"("          {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);

      fullArgString = current->getData(EntryKey::Member_Args);

      s_argEntry = current;
      s_argEnum  = ArgKey::Member_Args;

      currentArgumentContext = PrototypeQual;

      BEGIN( ReadFuncArgType );
   }

<PrototypePtr>")"          {
      current->appendData(EntryKey::Member_Type, ")");
      BEGIN( Prototype );
   }

<PrototypePtr>.            {
      QString text = QString::fromUtf8(yytext);
      current->name += text;
   }

<PrototypeQual>"{"         {
      BEGIN( PrototypeSkipLine);
   }

<PrototypeQual>{B}*"const"{B}*      {
      current->appendData(EntryKey::Member_Args, " const ");
      current->argList.constSpecifier = true;
   }

<PrototypeQual>{B}*"volatile"{B}*   {
      current->appendData(EntryKey::Member_Args, " volatile ");
      current->argList.volatileSpecifier = true;
   }

<PrototypeQual>{B}*"&"{B}*          {
      current->appendData(EntryKey::Member_Args, " &");
      current->argList.refSpecifier = RefType::LValueRef;
   }

<PrototypeQual>{B}*"&&"{B}*         {
      current->appendData(EntryKey::Member_Args, " &&");
      current->argList.refSpecifier = RefType::RValueRef;
   }

<PrototypeQual>{B}*"="{B}*"0"{B}*   {
      current->appendData(EntryKey::Member_Args, " = 0");
      current->virt = Pure;
      current->argList.pureSpecifier = true;
   }

<PrototypeQual>"throw"{B}*"("            {
      current->setData(EntryKey::Exception_Spec, "throw(");
      BEGIN(PrototypeExc);
   }

<PrototypeExc>")"       {
      current->appendData(EntryKey::Exception_Spec, ")");
      BEGIN(PrototypeQual);
   }

<PrototypeExc>.         {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Exception_Spec, text[0]);
   }

<PrototypeQual>.        {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text[0]);
   }

<Prototype>.           {
      QString text = QString::fromUtf8(yytext);
      current->name += text[0];
   }

<PrototypeSkipLine>.   {
   }


  /* ------------ Generic rules -------------- */

<SkipCxxComment>.*"\\\n"      {
      // line continuation
      if (insideCSharp) {
         REJECT;
      } else {
         lineCount();
      }
   }

<SkipCxxComment>.*/\n         {
      BEGIN( lastCContext ) ;
   }

<SkipComment>[^\*\n]+    {
   }

"[["      {
      // C++11 attribute
      lastC11AttributeContext = YY_START;
      BEGIN( SkipC11Attribute );
   }

<*>\n     {
      lineCount();
   }

<*>\"     {
      if (insideIDL && insideCppQuote) {
         BEGIN(EndCppQuote);
      }
   }

<*>"#"                    {
      if (!insidePHP) {
         REJECT;
      }

      lastCContext = YY_START ;
      BEGIN( SkipCxxComment ) ;
   }

<*>\'                {
      if (insidePHP) {
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

<*>. {
   }

<SkipComment>"//"|"/*" {
   }

<*>"/*"                          {
      // */ (editor syntax fix)
      lastCContext = YY_START ;
      BEGIN( SkipComment );
   }

<SkipComment>{B}*"*/"            {
      BEGIN( lastCContext );
   }

<*>"//"                          {
      lastCContext = YY_START;
      BEGIN( SkipCxxComment );
   }
%%

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

static void newEntry()
{
   if (tempEntry == nullptr) {
      // if temp entry is not 0 it holds current
      // current is replaced by previous which was already added to current_root
      // do not add it again

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

   int position    = 0;
   bool needsEntry = false;

   if (s_docBlockInBody && hideInBodyDocs) {
      return;
   }

   if (doc.trimmed().isEmpty()) {
      return;
   }

   // line of block start
   int lineNr = brief ? current->briefLine : current->docLine;

   // fill in inbodyFile && inbodyLine the first time
   QSharedPointer<Entry> docEntry = s_docBlockInBody && previous ? previous : current;

   if (s_docBlockInBody && docEntry && docEntry->inbodyLine == -1) {
      docEntry->setData(EntryKey::Inbody_File, yyFileName);
      docEntry->inbodyLine = lineNr;
   }

   bool isBrief = false;
   bool isJavaDocStyle = false;

   if (! s_docBlockInBody) {
      isBrief = brief;
      isJavaDocStyle = s_docBlockAutoBrief;
   }

   while (parseCommentBlock(s_thisParser, docEntry, stripIndentation(doc), yyFileName,
                  lineNr, isBrief, isJavaDocStyle, s_docBlockInBody,
                  protection, position, needsEntry) ) {         // last 3 are passed by reference

      if (needsEntry) {
         QString tmpDocFile = current->getData(EntryKey::MainDocs_File),

         newEntry();

         current->setData(EntryKey::MainDocs_File, tmpDocFile);
         current->docLine = lineNr;

         //
         docEntry = s_docBlockInBody && previous ? previous : current;
      }
   }

   if (needsEntry) {
      newEntry();
   }

   if (s_docBlockTerm) {
      unput(s_docBlockTerm);
      s_docBlockTerm = 0;
   }
}

static void handleParametersCommentBlocks(ArgumentList &argList)
{
   for (auto &a : argList) {

      if (! a.docs.isEmpty()) {
         int position = 0;
         bool needsEntry;

         // save context
         QString orgBrief  = current->getData(EntryKey::Brief_Docs);
         QString orgMain   = current->getData(EntryKey::Main_Docs);

         int orgBriefLine  = current->briefLine;
         int orgDocLine    = current->docLine;

         current->setData(EntryKey::Brief_Docs, "");
         current->setData(EntryKey::Main_Docs,  "");

         while (parseCommentBlock(s_thisParser, current, a.docs, yyFileName, current->docLine,
                   false, false, false, protection, position, needsEntry ) ) {

            if (needsEntry) {
               newEntry();
            }
         }

         if (needsEntry) {
            newEntry();
         }

         a.docs = current->getData(EntryKey::Main_Docs);

         // restore context
         current->setData(EntryKey::Brief_Docs, orgBrief);
         current->setData(EntryKey::Main_Docs,  orgMain );
         current->briefLine = orgBriefLine;
         current->docLine   = orgDocLine;
      }
   }
}

static void parseCompounds(QSharedPointer<Entry> rt)
{
   static const bool groupNested = Config::getBool("group-nested-compounds");

   // safter to make a temp copy
   const QVector<QSharedPointer<Entry>> tmpChildren = rt->children();

   for (auto ce : tmpChildren) {

      if (! ce->getData(EntryKey::Source_Text).isEmpty()) {

         padCount = 0;
         s_column = 0;

         s_inputString   = ce->getData(EntryKey::Source_Text);
         s_inputPosition = 0;

         yyrestart( yyin );

         bool isEnum = ce->m_traits.hasTrait(Entry::Virtue::Enum);

         if (ce->section == Entry::ENUM_SEC || isEnum) {
            BEGIN(FindFields);
         } else {
            BEGIN(FindMembers);
         }

         current_root = ce;
         yyFileName   = ce->getData(EntryKey::File_Name);

         yyLineNr     = ce->startLine;
         yyColNr      = ce->startColumn;
         insideObjC   = ce->lang == SrcLangExt_ObjC;

         current = QMakeShared<Entry>();
         gstat = false;

         initEntry();

         // copy group list from parent
         if (groupNested && ce->section != Entry::ENUM_SEC && ! isEnum) {
            for (auto &g : rt->m_groups) {
               ce->m_groups.append(g);
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

            bool isEnum      = ce->m_traits.hasTrait(Entry::Virtue::Enum);
            bool isInterface = ce->m_traits.hasTrait(Entry::Virtue::Interface);
            bool isRef       = ce->m_traits.hasTrait(Entry::Virtue::Ref);
            bool isValue     = ce->m_traits.hasTrait(Entry::Virtue::Value);
            bool isStruct    = ce->m_traits.hasTrait(Entry::Virtue::Struct);
            bool isUnion     = ce->m_traits.hasTrait(Entry::Virtue::Union);

            if (insidePHP || insideD || insideJS || insideIDL) {
               current->protection = protection = Public ;

            } else if (insideJava) {
               protection = (isEnum || isInterface) ? Public : Package;
               current->protection = protection;

            } else if (isInterface || isRef || isValue || isStruct || isUnion) {

               if (ce->lang == SrcLangExt_ObjC) {
                  current->protection = protection = Protected;

               } else {
                  current->protection = protection = Public;
               }

            } else {
               current->protection = protection = Private;
            }

         } else if (ce->section == Entry::ENUM_SEC) {
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
         s_lexInit = true;

         groupLeaveCompound(yyFileName, yyLineNr, ce->name);
         current = QSharedPointer<Entry>();

         ce->setData(EntryKey::Source_Text, "");
      }

      parseCompounds(ce);
   }
}



// ****

void cstyleFreeParser()
{
   if (s_lexInit) {
      yylex_destroy();
   }
}

static void parseMain(const QString &fileName, const QString &fileBuf, QStringList &includedFiles,
                  QSharedPointer<Entry> rt)
{
   s_inputFile.setFileName(fileName);

   if (s_inputFile.open(QIODevice::ReadOnly)) {
      setContext(fileName);
      initParser();

      s_inputString   = fileBuf;
      s_inputPosition = 0;
      s_column        = 0;

      current_root    = rt;
      global_root     = rt;

      yyLineNr        = 1;
      yyFileName      = fileName;
      rt->lang        = language;

      groupEnterFile(yyFileName, yyLineNr);

      current = QMakeShared<Entry>();
      int sec = determineSection(yyFileName);

      if (sec != 0) {
         current->name    = yyFileName;
         current->section = sec;
         current_root->addSubEntry(current, current_root);
      }

      current->reset();

      initEntry();
      yyrestart(yyin);

      if (insidePHP) {
         BEGIN( FindMembersPHP );

      } else {
         BEGIN( FindMembers );

      }

      yylex();
      s_lexInit = true;

      if (YY_START == Comment) {
         warn(yyFileName, yyLineNr, "File ended in the middle of a comment block, Check for a missing \\endcode");
      }

      groupLeaveFile(yyFileName, yyLineNr);
      rt->setData(EntryKey::Source_Text, "");

      if (rt->children().contains(current) == 0)  {
         current = QSharedPointer<Entry>();
      }

      parseCompounds(rt);
      s_inputFile.close();

      anonNSCount++;
   }
}

static void parsePrototype(const QString &text)
{
   if (text.isEmpty()) {
      warn(yyFileName, yyLineNr,"Empty prototype found");
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
   orgInputString   = s_inputString;
   orgInputPosition = s_inputPosition;

   // set new string
   s_inputString   = text;
   s_inputPosition = 0;
   s_column        = 0;

   yyrestart( yyin );
   BEGIN(Prototype);

   yylex();
   s_lexInit = true;

   current->name = current->name.trimmed();
   if (current->section == Entry::MEMBERDOC_SEC && current->getData(EntryKey::Member_Args).isEmpty()) {
      current->section = Entry::VARIABLEDOC_SEC;
   }

   // restore original scanner state
   YY_BUFFER_STATE tmpState = YY_CURRENT_BUFFER;
   yy_switch_to_buffer(orgState);
   yy_delete_buffer(tmpState);

   s_inputString   = orgInputString;
   s_inputPosition = orgInputPosition;
}


// **

bool CPPLanguageParser::needsPreprocessing(const QString &extension)
{
   QString fe = extension.toLower();
   SrcLangExt lang = getLanguageFromFileName(extension);

   return (SrcLangExt_Cpp == lang) ||
                  ! ( fe == ".java" || fe == ".as"  || fe == ".d"    || fe == ".php" ||
                      fe == ".php4" || fe == ".inc" || fe == ".phtml");
}

void CPPLanguageParser::parseInput(const QString &fileName, const QString &fileBuffer,
                  QSharedPointer<Entry> root, enum ParserMode mode, QStringList &includedFiles, bool useClang)
{
   if (useClang) {

      if (mode == ParserMode::SOURCE_FILE) {
         ClangParser::instance()->start(fileName, fileBuffer, includedFiles, root);

      } else {
         // an include file
         ClangParser::instance()->switchToFile(fileName);

      }

   } else {
      s_thisParser = this;

      printlex(yy_flex_debug, true, __FILE__, fileName);
      ::parseMain(fileName, fileBuffer, includedFiles, root);
      printlex(yy_flex_debug, false, __FILE__, fileName);
   }
}

void CPPLanguageParser::parseCode(CodeOutputInterface &codeOutIntf, const QString &scopeName, const QString &input,
                  SrcLangExt lang, bool isExampleBlock, const QString &exampleName, QSharedPointer<FileDef> fileDef,
                  int startLine, int endLine, bool inlineFragment, QSharedPointer<MemberDef> memberDef,
                  bool showLineNumbers, QSharedPointer<Definition> searchCtx, bool collectXRefs )
{
   ::parseCCode(codeOutIntf,scopeName, input,lang,isExampleBlock,exampleName, fileDef,startLine, endLine,
                  inlineFragment,memberDef, showLineNumbers,searchCtx,collectXRefs);
}


void CPPLanguageParser::parsePrototype(const QString &text)
{
   ::parsePrototype(text);
}


void CPPLanguageParser::resetCodeParserState()
{
   ::resetCCodeParserState();
}

