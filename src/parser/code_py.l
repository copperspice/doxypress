/*************************************************************************
 *
 * Copyright (c) 2014-2023 Barbara Geller & Ansel Sermersheim
 * Copyright (c) 1997-2014 Dimitri van Heesch

*************************************************************************/

%{

#include <QStack>
#include <QSharedPointer>
#include <QStringList>

#include <stdio.h>

#include <config.h>
#include <doxy_globals.h>
#include <entry.h>
#include <message.h>
#include <outputlist.h>
#include <parse_cstyle.h>
#include <parse_py.h>
#include <tooltip.h>
#include <util.h>

#define DBG_CTX(...)     do { } while(0)

#define YY_NO_INPUT 1

static ClassSDict    s_codeClassSDict;
static QString       s_curClassName;
static QStringList   s_curClassBases;

static QString       s_inputString;        // the code fragment as text
static int	         s_inputPosition;      // read offset during parsing
static QString       s_currentFontClass;
static bool          s_needsTermination;

static bool          s_collectXRefs;
static int           s_inputLines;         // number of line in the code fragment
static int	         s_yyLineNr;           // current line number

static bool          s_includeCodeFragment;
static QString       s_realScope;

static int           s_bodyCurlyCount;
static bool          s_searchingForBody;
static QString       s_classScope;
static int           s_paramParens;

static bool          s_exampleBlock;
static QString       s_exampleName;
static QString       s_type;
static QString       s_name;

static bool          s_doubleStringIsDoc;
static bool          s_doubleQuote;
static bool          s_noSuiteFound;
static int           s_stringContext;

static QString       s_docBlock;
static bool          s_endComment;

static CodeGenerator               *s_code;

static QStack<uint>                 s_indents;           // tracks indentation levels for scoping in python
static QSharedPointer<Definition>   s_searchCtx;
static QSharedPointer<FileDef>      s_sourceFileDef;
static QSharedPointer<Definition>   s_currentDefinition;
static QSharedPointer<MemberDef>    s_currentMemberDef;

static void endFontClass();
static void adjustScopesAndSuites(unsigned indentLength);


/*! Represents a stack of variable to class mappings as found in the
 *  code. Each scope is enclosed in pushScope() and popScope() calls.
 *  Variables are added by calling addVariables() and one can search
 *  for variable using findVariable().
 */
class PyVariableContext
{
   public:
      static QSharedPointer<ClassDef> dummyContext();

      PyVariableContext() {
      }

      virtual ~PyVariableContext() {
      }

      void pushScope() {
         m_scopes.append(new StringMap<QSharedPointer<ClassDef>>);
      }

      void popScope() {
         if (m_scopes.count() > 0) {
            m_scopes.removeAt(m_scopes.count() - 1);
         }
      }

      void clear() {
         m_scopes.clear();
         m_globalScope.clear();
      }

      void clearExceptGlobal() {
         m_scopes.clear();
      }

      void addVariable(const QString &type, const QString &name);
      QSharedPointer<ClassDef> findVariable(const QString &name);

   private:
      StringMap<QSharedPointer<ClassDef>>          m_globalScope;
      QList<StringMap<QSharedPointer<ClassDef>> *> m_scopes;
};

QSharedPointer<ClassDef> PyVariableContext::dummyContext()
{
   static QSharedPointer<ClassDef> dummyContext = QMakeShared<ClassDef>("", 0, 0, "dummyContext-python", CompoundType::Class);
   return dummyContext;
}

void PyVariableContext::addVariable(const QString &type, const QString &name)
{
   QString ltype = type.simplified();
   QString lname = name.simplified();

   StringMap<QSharedPointer<ClassDef>> *scope;

   if (m_scopes.count() == 0 ) {
      scope = &m_globalScope;
   } else {
      scope = m_scopes.last();
   }

   QSharedPointer<ClassDef> varType;

   if ( (varType = s_codeClassSDict[ltype]) || (varType = getResolvedClass(s_currentDefinition, s_sourceFileDef, ltype)) ) {
      // look for class definitions inside the code block
      // look for global class definitions

      scope->insert(lname, varType);

   } else {
      if (m_scopes.count() > 0)  {
         // for local variables add a dummy entry so the name
         // is hidden to avoid false links to global variables with the same name
         // TODO: make this work for namespaces as well

         scope->insert(lname, PyVariableContext::dummyContext());
      }
   }
}

QSharedPointer<ClassDef> PyVariableContext::findVariable(const QString &name)
{
   if (name.isEmpty()) {
      return QSharedPointer<ClassDef>();
   }

   QSharedPointer<ClassDef> result;

   // search from inner to outer scope
   for (int k = m_scopes.size() - 1; k >= 0; --k) {

      result = m_scopes[k]->find(name);

      if (result) {
         return result;
      }
   }

   // nothing found -> also try the global scope
   result = m_globalScope.find(name);

   return result;
}

static PyVariableContext s_theVarContext;

class PyCallContext
{
   public:
      struct Ctx {
         Ctx() : name(s_name), type(s_type), cd(0)
         {}

         QString name;
         QString type;

         QSharedPointer<ClassDef> cd;
      };

      PyCallContext() {
         m_classList.append(new Ctx);
      }

      virtual ~PyCallContext() {}

      void setClass(QSharedPointer<ClassDef> cd) {
         Ctx *ctx = m_classList.last();

         if (ctx) {
            ctx->cd = cd;
         }
      }

      void pushScope() {
         m_classList.append(new Ctx);
      }

      void popScope() {
         if (m_classList.count() > 1) {
            Ctx *ctx = m_classList.last();

            if (ctx) {
               s_name = ctx->name;
               s_type = ctx->type;
            }
            m_classList.removeLast();
         }
      }

      void clear() {
         m_classList.clear();
         m_classList.append(new Ctx);
      }

      QSharedPointer<ClassDef> getClass() const {
         Ctx *ctx = m_classList.last();

         if (ctx) {
            return ctx->cd;
         } else {
            return QSharedPointer<ClassDef>();
         }
      }

   private:
      QList<Ctx *> m_classList;
};

static PyCallContext s_theCallContext;

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

static QSharedPointer<ClassDef> stripClassName(const QString &s, QSharedPointer<Definition> d = s_currentDefinition)
{
   int pos = 0;

   QString type = s;
   QString className;
   QString templSpec;

   while (extractClassNameFromType(type, pos, className, templSpec) != -1) {
      QString clName = className + templSpec;

      QSharedPointer<ClassDef> cd;

      if (! s_classScope.isEmpty()) {
         cd = getResolvedClass(d, s_sourceFileDef, s_classScope + "::" + clName);
      }

      if (cd == 0) {
         cd = getResolvedClass(d, s_sourceFileDef, clName);
      }

      if (cd) {
         return cd;
      }
   }

   return QSharedPointer<ClassDef>();
}

/* start a new line of code, inserting a line number if s_sourceFileDef
 * is true. If a definition starts at the current line, then the line
 * number is linked to the documentation of that definition.
 */
static void startCodeLine()
{
   if (s_sourceFileDef) {
      QSharedPointer<Definition> d = s_sourceFileDef->getSourceDefinition(s_yyLineNr);

      if (! s_includeCodeFragment && d && d->isLinkableInProject()) {
         s_currentDefinition = d;
         s_currentMemberDef  = s_sourceFileDef->getSourceMember(s_yyLineNr);
         s_endComment        = false;
         s_searchingForBody  = true;

         s_realScope  = d->name();
         s_classScope = d->name();

         s_bodyCurlyCount = 0;

         QString lineAnchor;
         lineAnchor = QString("l%1").formatArg(s_yyLineNr, 5, 10, QChar('0'));

         if (s_currentMemberDef) {
            s_code->writeLineNumber(s_currentMemberDef->getReference(), s_currentMemberDef->getOutputFileBase(),
                  s_currentMemberDef->anchor(), s_yyLineNr);

            setCurrentDoc(lineAnchor);

         } else {
            s_code->writeLineNumber(d->getReference(), d->getOutputFileBase(), QString(), s_yyLineNr);
            setCurrentDoc(lineAnchor);
         }

      } else {
         s_code->writeLineNumber(QString(), QString(), QString(), s_yyLineNr);
      }
   }

   s_code->startCodeLine(s_sourceFileDef);

   if (! s_currentFontClass.isEmpty()) {
      s_code->startFontClass(s_currentFontClass);
   }
}

static void codify(const QString &text)
{
   s_code->codify(text);
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

static void startFontClass(const QString &s)
{
   // if font class is already set do not stop and start it again
   if (s_currentFontClass != s) {
      endFontClass();
      s_code->startFontClass(s);
      s_currentFontClass = s;
   }
}

static void endFontClass()
{
   if (! s_currentFontClass.isEmpty() ) {
      s_code->endFontClass();
      s_currentFontClass = "";
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
         nextCodeLine();

         tmp = "";

      } else {
         tmp += c;

      }
   }

   if ( ! tmp.isEmpty() ) {
      ol.writeCodeLink(ref, file, anchor, tmp, tooltip);
   }
}

static void codifyLines(const QString &text)
{
   const QString tmp_currentFontClass = s_currentFontClass;

   QString tmp;

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

static bool getLinkInScope(const QString &c, const QString &m,  const QString &memberText,
                           CodeGenerator &ol, const QString &text )
{
   QSharedPointer<MemberDef>    md;
   QSharedPointer<ClassDef>     cd;
   QSharedPointer<FileDef>      fd;
   QSharedPointer<NamespaceDef> nd;
   QSharedPointer<GroupDef>     gd;

   if (getDefs(c, m, "()", md, cd, fd, nd, gd, false, s_sourceFileDef) && md->isLinkable()) {

      QSharedPointer<Definition> d;

      if (md->getOuterScope() == Doxy_Globals::globalScope) {
         d = md->getBodyDef() ;
      } else {
         d = md->getOuterScope();
      }

      if (md->getGroupDef()) {
         d = md->getGroupDef();
      }

      if (d && d->isLinkable()) {
         s_theCallContext.setClass(stripClassName(md->typeString(), md->getOuterScope()));

         if (s_currentDefinition && s_currentMemberDef && md != s_currentMemberDef && s_collectXRefs) {
            addDocCrossReference(s_currentMemberDef, md);
         }

         writeMultiLineCodeLink(ol, md, ! text.isEmpty() ? text : memberText);
         addToSearchIndex(! text.isEmpty() ? text : memberText);

         return true;
      }
   }

   return false;
}

static bool getLink(const QString &className, const QString &memberName, CodeGenerator &ol,
                  const QString &text = QString())
{
   QString m = removeRedundantWhiteSpace(memberName);
   QString c = className;

   if (! getLinkInScope(c, m, memberName, ol, text)) {
      if (! s_curClassName.isEmpty()) {

         if (! c.isEmpty()) {
            c.prepend("::");
         }

         c.prepend(s_curClassName);
         return getLinkInScope(c, m, memberName, ol, text);
      }
      return false;
   }
   return true;
}

/*
  For a given string in the source code,
  finds its class or global id and links to it.
*/
static void generateClassOrGlobalLink(CodeGenerator &ol, const QString &clName, bool typeOnly = false)
{
   QString className = clName;

   // do not do anything for empty text
   if (className.isEmpty()) {
      return;
   }

   DBG_CTX(stderr, "generateClassOrGlobalLink(className=%s)\n", csPrintable(className) );

   QSharedPointer<ClassDef> cd;
   QSharedPointer<ClassDef> lcd;

   // Member def that we may find
   QSharedPointer<MemberDef> md;

   if ((lcd = s_theVarContext.findVariable(className)) == 0) { // not a local variable
      QSharedPointer<Definition> d = s_currentDefinition;

      QString scope = substitute(className, ".", "::");

      cd = getResolvedClass(d, s_sourceFileDef, substitute(className, ".", "::"), &md);

      DBG_CTX(stderr, "d=%s s_sourceFileDef=%s\n", d ? csPrintable(d->displayName()) : "<null>",
               s_currentDefinition ? csPrintable(s_currentDefinition->displayName()) : "<null>");

      DBG_CTX(stderr, "is found as a type %s\n", cd ? csPrintable(cd->name()) : "<null>");

      if (cd == 0 && md == 0) {
         // also see if it is variable or enum or enum value
         QSharedPointer<NamespaceDef> nd = getResolvedNamespace(scope);

         if (nd) {
            writeMultiLineCodeLink(ol, nd, clName);
            addToSearchIndex(className);
            return;

         } else if (getLink(s_classScope, clName, ol, clName)) {
            return;
         }
      }

   } else {
      if (lcd != PyVariableContext::dummyContext()) {
         s_theCallContext.setClass(lcd);
      }

      DBG_CTX(stderr, "is a local variable cd=%p\n", cd.data());
   }

   if (cd && cd->isLinkable()) { // is it a linkable class
      writeMultiLineCodeLink(ol, cd, clName);
      addToSearchIndex(className);

      if (md) {
         QSharedPointer<Definition> d;

         if (md->getOuterScope() == Doxy_Globals::globalScope) {
            d = md->getBodyDef();
         } else {
            d = md->getOuterScope();
         }

         if (md->getGroupDef()) {
            d = md->getGroupDef();
         }

         if (d && d->isLinkable() && md->isLinkable() && s_currentMemberDef && s_collectXRefs) {
            addDocCrossReference(s_currentMemberDef, md);
         }
      }

   } else { // not a class, maybe a global member
      int scopeEnd = className.lastIndexOf(".");

      if (scopeEnd != -1 && !typeOnly) { // name with explicit scope
         QString scope = substitute(className.left(scopeEnd), ".", "::");
         QString locName = className.right(className.length() - scopeEnd - 1);

         QSharedPointer<ClassDef> mcd = getClass(scope);

         DBG_CTX(stderr, "scope=%s locName=%s mcd=%p\n", csPrintable(scope), csPrintable(locName), mcd.data());

         if (mcd) {
            QSharedPointer<MemberDef> itemMd = mcd->getMemberByName(locName);

            if (itemMd != nullptr) {
               s_theCallContext.setClass(stripClassName(itemMd->typeString(), itemMd->getOuterScope()) );
               writeMultiLineCodeLink(ol, itemMd, clName);
               addToSearchIndex(className);

               QSharedPointer<Definition> d;

               if (itemMd->getOuterScope() == Doxy_Globals::globalScope) {
                  d = itemMd->getBodyDef();
               } else {
                  d = itemMd->getOuterScope();
               }

               if (itemMd->getGroupDef()) {
                  d = itemMd->getGroupDef();
               }

               if (d && d->isLinkable() && itemMd->isLinkable() && s_currentMemberDef && s_collectXRefs) {
                  addDocCrossReference(s_currentMemberDef, itemMd);
               }

               return;
            }

         } else {
            // check namespace as well
            QSharedPointer<NamespaceDef> mnd = getResolvedNamespace(scope);

            if (mnd) {
               QSharedPointer<MemberDef> itemMd = mnd->getMemberByName(locName);

               if (itemMd != nullptr) {
                  s_theCallContext.setClass(stripClassName(itemMd->typeString(), itemMd->getOuterScope()) );
                  writeMultiLineCodeLink(ol, itemMd, clName);
                  addToSearchIndex(className);

                  QSharedPointer<Definition> d;

                  if (itemMd->getOuterScope() == Doxy_Globals::globalScope) {
                     d = itemMd->getBodyDef();

                  } else {
                     d = itemMd->getOuterScope();

                  }

                  if (itemMd->getGroupDef()) {
                     d = itemMd->getGroupDef();
                  }

                  if (d && d->isLinkable() && itemMd->isLinkable() && s_currentMemberDef && s_collectXRefs) {
                     addDocCrossReference(s_currentMemberDef, itemMd);
                  }

                  return;
               }
            }
         }
      }

      // nothing found, just write out the word
      codifyLines(clName);
      addToSearchIndex(clName);
   }
}

// seems to work for file members, but scopes are not being correctly tracked for classes
static void generateFunctionLink(CodeGenerator &ol, const QString &funcName)
{
   QSharedPointer<ClassDef> ccd;

   QString locScope = s_classScope;
   QString locFunc = removeRedundantWhiteSpace(funcName);

   DBG_CTX(stdout, "*** locScope=%s locFunc=%s\n", csPrintable(locScope), csPrintable(locFunc));
   int i = locFunc.lastIndexOf("::");

   if (i > 0) {
      locScope = locFunc.left(i);
      locFunc  = locFunc.right(locFunc.length() - i - 2).trimmed();
   }

   if (! locScope.isEmpty() && (ccd = s_codeClassSDict[locScope])) {

      if (ccd->baseClasses()) {
         for (auto item : *ccd->baseClasses() ) {
            if (getLink(item->classDef->name(), locFunc, ol, funcName)) {
               return;
            }
         }
      }
   }

   if (! getLink(locScope, locFunc, ol, funcName)) {
      generateClassOrGlobalLink(ol, funcName);
   }
   return;
}

static bool findMemberLink(CodeGenerator &ol, QSharedPointer<Definition> def, const QString &name)
{
   if (def->getOuterScope() && def->getOuterScope()->definitionType() == Definition::TypeClass &&
         s_currentDefinition->definitionType() == Definition::TypeClass) {

      QSharedPointer<ClassDef> cd     = def->getOuterScope().dynamicCast<ClassDef>();
      QSharedPointer<ClassDef> thisCd = s_currentDefinition.dynamicCast<ClassDef>();

      if (def->definitionType() == Definition::TypeMember) {
         if (s_currentMemberDef && s_collectXRefs) {
            addDocCrossReference(s_currentMemberDef, def.dynamicCast<MemberDef>());
         }
      }

      DBG_CTX(stderr, "cd=%s thisCd=%s\n", cd ? csPrintable(cd->name()) : "<none>",
                  thisCd ? csPrintable(thisCd->name()) : "<none>");

      // TODO: find the nearest base class in case cd is a base class of thisCd

      if (cd == thisCd || (thisCd && thisCd->isBaseClass(cd, true))) {
         writeMultiLineCodeLink(ol, def, name);
         return true;
      }
   }

   return false;
}

static void findMemberLink(CodeGenerator &ol, const QString &phrase)
{
   if (s_currentDefinition) {
      auto iter = Doxy_Globals::glossary().find(phrase);

      while (iter != Doxy_Globals::glossary().end() && iter.key() == phrase) {
         QSharedPointer<Definition> def = sharedFrom(iter.value());

         if (findMemberLink(ol, def, phrase)) {
            return;
         }

         ++iter;
      }
   }

   codify(phrase);
}

#undef  YY_INPUT
#define YY_INPUT(buf,result,max_size) result = yyread(buf, max_size);

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

BB                [ \t]+
B                 [ \t]*
NEWLINE           \n

DIGIT             [0-9]
LETTER            [A-Za-z\x80-\xFF]
NONEMPTY          [A-Za-z0-9_\x80-\xFF]
EXPCHAR           [#(){}\[\],:.%/\\=`*~|&<>!;+-]
NONEMPTYEXP       [^ \t\n:]
PARAMNONEMPTY     [^ \t\n():]
IDENTIFIER        ({LETTER}|"_")({LETTER}|{DIGIT}|"_")*
BORDER            ([^A-Za-z0-9])

POUNDCOMMENT      "##"

TRISINGLEQUOTE    "'''"
TRIDOUBLEQUOTE    "\"\"\""
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

KEYWORD           ("lambda"|"import"|"class"|"assert"|"with"|"as"|"from"|"global"|"def"|"True"|"False")
FLOWKW            ("or"|"and"|"is"|"not"|"print"|"for"|"in"|"if"|"try"|"except"|"yield"|"raise"|"break"|"continue"|"pass"|"if"|"return"|"while"|"elif"|"else"|"finally")

QUOTES            ("\""[^"]*"\"")
SINGLEQUOTES      ("'"[^']*"'")

LONGINTEGER       {INTEGER}("l"|"L")
INTEGER           ({DECIMALINTEGER}|{OCTINTEGER}|{HEXINTEGER})
DECIMALINTEGER    ({NONZERODIGIT}{DIGIT}*|"0")
OCTINTEGER        "0"{OCTDIGIT}+
HEXINTEGER        "0"("x"|"X"){HEXDIGIT}+
NONZERODIGIT      [1-9]
OCTDIGIT          [0-7]
HEXDIGIT          ({DIGIT}|[a-f]|[A-F])
FLOATNUMBER       ({POINTFLOAT}|{EXPONENTFLOAT})
POINTFLOAT        ({INTPART}?{FRACTION}|{INTPART}".")

EXPONENTFLOAT        ({INTPART}|{POINTFLOAT}){EXPONENT}
INTPART              {DIGIT}+
FRACTION             "."{DIGIT}+
EXPONENT             ("e"|"E")("+"|"-")?{DIGIT}+
IMAGNUMBER           ({FLOATNUMBER}|{INTPART})("j"|"J")
ATOM                 ({IDENTIFIER}|{LITERAL}|{ENCLOSURE})
ENCLOSURE            ({PARENTH_FORM}|{LIST_DISPLAY}|{DICT_DISPLAY}|{STRINs_CONVERSION})
LITERAL              ({STRINGLITERAL}|{INTEGER}|{LONGINTEGER}|{FLOATNUMBER}|{IMAGNUMBER})
PARENTH_FORM         "("{EXPRESSION_LIST}?")"
TEST                 ({AND_TEST}("or"{AND_TEST})*|{LAMBDA_FORM})
TESTLIST             {TEST}( ","{TEST})*","?
LIST_DISPLAY         "["{LISTMAKER}?"]"
LISTMAKER            {EXPRESSION}({LIST_FOR}|(","{EXPRESSION})*","?)
LIST_ITER            ({LIST_FOR}|{LIST_IF})
LIST_FOR             "for"{EXPRESSION_LIST}"in"{TESTLIST}{LIST_ITER}?
LIST_IF              "if"{TEST}{LIST_ITER}?
DICT_DISPLAY         "\{"{KEY_DATUM_LIST}?"\}"
KEY_DATUM_LIST       {KEY_DATUM}(","{KEY_DATUM})*","?
KEY_DATUM            {EXPRESSION}":"{EXPRESSION}
STRINs_CONVERSION    "`"{EXPRESSION_LIST}"`"
PRIMARY              ({ATOM}|{ATTRIBUTEREF}|{SUBSCRIPTION}|{SLICING}|{CALL})
ATTRIBUTEREF         {PRIMARY}"."{IDENTIFIER}
SUBSCRIPTION         {PRIMARY}"["{EXPRESSION_LIST}"]"
SLICING              ({SIMPLE_SLICING}|{EXTENDED_SLICING})
SIMPLE_SLICING       {PRIMARY}"["{SHORT_SLICE}"]"
EXTENDED_SLICING     {PRIMARY}"["{SLICE_LIST}"]"
SLICE_LIST           {SLICE_ITEM}(","{SLICE_ITEM})*","?
SLICE_ITEM           ({EXPRESSION}|{PROPER_SLICE}|{ELLIPSIS})
PROPER_SLICE         ({SHORT_SLICE}|{LONs_SLICE})
SHORT_SLICE          {LOWER_BOUND}?":"{UPPER_BOUND}?
LONs_SLICE           {SHORT_SLICE}":"{STRIDE}?
LOWER_BOUND          {EXPRESSION}
UPPER_BOUND          {EXPRESSION}
STRIDE               {EXPRESSION}
ELLIPSIS              "..."
CALL                 {PRIMARY}"("({ARGUMENT_LIST}","?)?")"

ARGUMENT_LIST        ({POSITIONAL_ARGUMENTS}(","{KEYWORD_ARGUMENTS})?(",""*"{EXPRESSION})?(",""**"{EXPRESSION})?|{KEYWORD_ARGUMENTS}(",""*"{EXPRESSION})?(",""**"{EXPRESSION})?|"*"{EXPRESSION}(",""**"{EXPRESSION})?|"**"{EXPRESSION})

POSITIONAL_ARGUMENTS {EXPRESSION}(","{EXPRESSION})*
KEYWORD_ARGUMENTS    {KEYWORD_ITEM}(","{KEYWORD_ITEM})*
KEYWORD_ITEM         {IDENTIFIER}"="{EXPRESSION}
POWER                {PRIMARY}("**"{U_EXPR})?
U_EXPR               ({POWER}|"-"{U_EXPR}|"+"{U_EXPR}|"\~"{U_EXPR})
M_EXPR               ({U_EXPR}|{M_EXPR}"*"{U_EXPR}|{M_EXPR}"//"{U_EXPR}|{M_EXPR}"/"{U_EXPR}|{M_EXPR}"\%"{U_EXPR})
A_EXPR               ({M_EXPR}|{A_EXPR}"+"{M_EXPR}|{A_EXPR}"-"{M_EXPR}
SHIFT_EXPR           ({A_EXPR}|{SHIFT_EXPR}("<<"|">>"){A_EXPR})
AND_EXPR             ({SHIFT_EXPR}|{AND_EXPR}"\;SPMamp;"{SHIFT_EXPR}
XOR_EXPR             ({AND_EXPR}|{XOR_EXPR}"\textasciicircum"{AND_EXPR})
OR_EXPR              ({XOR_EXPR}|{OR_EXPR}"|"{ XOR_EXPR})

COMPARISON           {OR_EXPR}({COMP_OPERATOR}{OR_EXPR})*
COMP_OPERATOR        ("<"|">"|"=="|">="|"<="|"<>"|"!="|"is""not"?|"not"?"in")
EXPRESSION           ({OR_TEST}|{LAMBDA_FORM})
OR_TEST              ({AND_TEST}|{OR_TEST}"or"{AND_TEST})
AND_TEST             ({NOT_TEST}|{AND_TEST}"and"{NOT_TEST})
NOT_TEST             ({COMPARISON}|"not"{NOT_TEST})
LAMBDA_FORM          "lambda"{PARAMETER_LIST}?":"{EXPRESSION}
EXPRESSION_LIST      {EXPRESSION}(","{EXPRESSION})*","?

SIMPLE_STMT          ({EXPRESSION_STMT}|{ASSERT_STMT}|{ASSIGNMENT_STMT}|{AUGMENTED_ASSIGNMENT_STMT}|{PASS_STMT}|{DEL_STMT}|{PRINT_STMT}|{RETURN_STMT}|{YIELD_STMT}|{RAISE_STMT}|{BREAK_STMT}|{CONTINUE_STMT}|{IMPORT_STMT}|{GLOBAL_STMT}|{EXEC_STMT})

EXPRESSION_STMT      {EXPRESSION_LIST}
ASSERT_STMT          "assert"{EXPRESSION}(","{EXPRESSION})?
ASSIGNMENT_STMT      ({TARGET_LIST}"=")+{EXPRESSION_LIST}
TARGET_LIST          {TARGET}(","{TARGET})*","?
TARGET               ({IDENTIFIER}|"("{TARGET_LIST}")"|"["{TARGET_LIST}"]"|{ATTRIBUTEREF}|{SUBSCRIPTION}|{SLICING})

%option never-interactive
%option nounistd
%option noyy_top_state
%option noyywrap
%option stack

%x Body

%x FunctionDec
%x FunctionParams

%x ClassDec
%x ClassInheritance

%x Suite
%x SuiteCaptureIndent
%x SuiteStart
%x SuiteMaintain
%x SuiteContinuing

%x LongString

%x SingleQuoteString
%x DoubleQuoteString
%x TripleString

%x DocBlock
%%

<Body,Suite>{
   "def"{BB}                     {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keyword");
      codify(text);
      endFontClass();
      BEGIN( FunctionDec );
   }

   "class"{BB}                   {
     QString text = QString::fromUtf8(yytext);
      startFontClass("keyword");
      codify(text);
      endFontClass();
      BEGIN( ClassDec );
      }

   "None"                        {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keywordtype");
      codify(text);
      endFontClass();
   }

   "self."{IDENTIFIER}/"."({IDENTIFIER}".")*{IDENTIFIER}"(" {
      QString text = QString::fromUtf8(code_py_YYtext);
      codify("self.");
		findMemberLink(*s_code, text.mid(5));
   }

   "self."{IDENTIFIER}/"("       {
      QString text = QString::fromUtf8(yytext);
      codify("self.");
      findMemberLink(*s_code, text.mid(5));
   }

   "self."{IDENTIFIER}/"."({IDENTIFIER}".")*{IDENTIFIER} {
      QString text = QString::fromUtf8(code_py_YYtext);
      codify("self.");
		findMemberLink(*s_code, text.mid(5));
   }

   "self."{IDENTIFIER}           {
      QString text = QString::fromUtf8(yytext);
      codify("self.");
      findMemberLink(*s_code, text.mid(5));
   }

   "cls."{IDENTIFIER}/"."({IDENTIFIER}".")*{IDENTIFIER}"(" {
      QString text = QString::fromUtf8(code_py_YYtext);
		codify("cls.");
	   findMemberLink(*s_code, text.mid(4));
   }

   "cls."{IDENTIFIER}/"("       {
      QString text = QString::fromUtf8(yytext);
      codify("cls.");
      findMemberLink(*s_code, text.mid(4));
   }

   "cls."{IDENTIFIER}/"."({IDENTIFIER}".")*{IDENTIFIER} {
      QString text = QString::fromUtf8(code_py_YYtext);
		codify("cls.");
	   findMemberLink(*s_code, text.mid(4));
   }

   "cls."{IDENTIFIER}           {
      QString text = QString::fromUtf8(yytext);
      codify("cls.");
      findMemberLink(*s_code, text.mid(4));
    }
}

<ClassDec>{IDENTIFIER}              {
   QString text = QString::fromUtf8(yytext);
   generateClassOrGlobalLink(*s_code, text);

   s_curClassName = text;
   s_curClassBases.clear();
   BEGIN( ClassInheritance );
}

<ClassInheritance>{
   ({BB}|[(,)])                     {
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }

   ({IDENTIFIER}".")*{IDENTIFIER}   {
      // The parser is assuming that ALL identifiers in this state are base classes;
      // it does not check to see that the first parenthesis has been seen.
      // it should probably be more strict about what to accept.

      QString text = QString::fromUtf8(yytext);
      s_curClassBases.append(text);

      generateClassOrGlobalLink(*s_code, text);
   }

   ":"                             {
      QString text = QString::fromUtf8(yytext);
      codify(text);

      // Assume this will be a one-line suite;
      // found counter-example in SuiteStart.

      // Push a class scope
      QSharedPointer<ClassDef> classDefToAdd = QMakeShared<ClassDef> ("<code>", 1, 1, s_curClassName,
                  CompoundType::Class, "", "", false);

      s_codeClassSDict.insert(s_curClassName, classDefToAdd);

      for (auto str : s_curClassBases) {

         QSharedPointer<ClassDef> baseDefToAdd;
         baseDefToAdd = s_codeClassSDict[str];

         // Try to find class in global scope

         if (baseDefToAdd == 0) {
            baseDefToAdd = getResolvedClass(s_currentDefinition, s_sourceFileDef, str);
         }

         if (baseDefToAdd && baseDefToAdd != classDefToAdd) {
            classDefToAdd->insertBaseClass(baseDefToAdd, str, Protection::Public, Specifier::Normal);
         }
      }

      // Reset class-parsing variables.
      s_curClassName.resize(0);
      s_curClassBases.clear();

      s_noSuiteFound = true;
      BEGIN( SuiteStart );
   }
}

<FunctionDec>{
   {IDENTIFIER}                    {
      QString text = QString::fromUtf8(yytext);
      generateFunctionLink(*s_code, text);
   }

   {B}"("                          {
      QString text = QString::fromUtf8(yytext);
      codify(text);
      BEGIN( FunctionParams );
   }
}

<FunctionParams>{
   ({BB}|",")  {
      // Parses delimiters
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }

   ({IDENTIFIER}|{PARAMNONEMPTY}+) {
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }

   ")"                             {
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }

   "\n"                            {
      QString text = QString::fromUtf8(yytext);
	   codifyLines(text);
   }

   ":"                             {
      QString text = QString::fromUtf8(yytext);
      codify(text);

      // assume this will be a one-line suite
      // found counter-example in SuiteStart.

      s_noSuiteFound = true;
      BEGIN( SuiteStart );
   }
}

<Body,Suite>{

   {KEYWORD}                  {
      // Position-sensitive rules, must come AFTER keyword-triggered rules
      // Must come BEFORE identifier NONEMPTY-like rules to syntax highlight.

      QString text = QString::fromUtf8(yytext);
      startFontClass("keyword");
      codify(text);
      endFontClass();
   }

   {FLOWKW}                   {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keywordflow");
      codify(text);
      endFontClass();
                               }
   ({IDENTIFIER}".")*{IDENTIFIER}/"("  {
      QString text = QString::fromUtf8(yytext);
      generateClassOrGlobalLink(*s_code, text);
   }

   ({IDENTIFIER}".")+{IDENTIFIER} {
      QString text = QString::fromUtf8(yytext);
      generateClassOrGlobalLink(*s_code, text, true);
   }

   {IDENTIFIER}               {
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }
}

<SuiteStart>{

   {BB}                               {
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }

   "pass"               {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
      BEGIN(Body);
   }

   {KEYWORD}                          {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();

      // no indentation necessary
      s_noSuiteFound = false;
   }

   {FLOWKW}                           {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keywordflow");
      codifyLines(text);
      endFontClass();

      // no indentation necessary
      s_noSuiteFound = false;
   }

   {IDENTIFIER}                       {
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }

   {POUNDCOMMENT}                     {
      QString text = QString::fromUtf8(yytext);
      if (YY_START == SingleQuoteString || YY_START == DoubleQuoteString || YY_START == TripleString) {
         REJECT;
      }

      yy_push_state(YY_START);
      BEGIN(DocBlock);
      s_docBlock = text;
   }

   {NEWLINE}                          {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);

      if ( s_noSuiteFound ) {
         BEGIN ( SuiteCaptureIndent );
      }
   }
}

<SuiteCaptureIndent>{
   "\n"|({BB}"\n")            {
      // Blankline - ignore, keep looking for indentation
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

   {BB}                       {
      // This state lasts momentarily, to check the indentation
      // level that is about to be used
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);

      s_indents.push(yyleng);
      BEGIN( Suite );
   }
}

<SuiteMaintain>{

   {BB}/({NONEMPTY}|{EXPCHAR}) {
      // This implements poor indentation-tracking should be improved.
      // (translate tabs to space, etc)
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
      adjustScopesAndSuites(yyleng);
   }

   "\n"|({BB}"\n")            {
      // If this ever succeeds, it means that this is a blank line, and
      // can be ignored.
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

   ""/({NONEMPTY}|{EXPCHAR})  {
      // Default rule; matches yhe empty string, assuming real text starts here.
      // Just go straight to Body
      adjustScopesAndSuites(0);
   }
}

<Suite>{NEWLINE}               {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
      BEGIN( SuiteMaintain );
   }

<Body>{IDENTIFIER}          {
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }

<Body>{NEWLINE}                {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

<SingleQuoteString>{
   /*
      // Single quoted string like 'That\'s a """nice""" string!'
   */

   \\{B}\n                    {
      // line continuation
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

   \\.               {
      // escaped char
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }

   {STRINGPREFIX}?{TRIDOUBLEQUOTE} {
      // triple double quotes
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }

   "'"               {
      // end of the string
      QString text = QString::fromUtf8(yytext);
      codify(text);
      endFontClass();

      BEGIN(s_stringContext);
   }

   [^"'\n\\]+                   {
      // normal chars
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }

   .                          {
      // normal char
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }
}

<DoubleQuoteString>{
   /*
      // Double quoted string like "That's \"a '''nice'''\" string!"
   */

   \\{B}\n                    {
      // line continuation
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

   \\.               {
      // escpaced char
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }

   {STRINGPREFIX}?{TRISINGLEQUOTE} {
      // triple single quotes
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }

   "\""              {
      // end of the string
      QString text = QString::fromUtf8(yytext);
      codify(text);
      endFontClass();

      BEGIN( s_stringContext );
   }

   [^"'\n\\]+                   {
      // normal chars
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }

   .                 {
      // normal char
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }
}

<TripleString>{
    {TRIDOUBLEQUOTE}   |
    {TRISINGLEQUOTE}   {

      QString text = QString::fromUtf8(yytext);
      codify(text);

      if (s_doubleQuote == (text[0] == '"')) {
         endFontClass();
         BEGIN(s_stringContext);
      }
    }

   {LONGSTRINGBLOCK}  {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

   \n             {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

   .              {
      QString text = QString::fromUtf8(yytext);
      codify(text);
   }
}

  /*
<*>({NONEMPTY}|{EXPCHAR}|{BB})           {
      // This should go one character at a time.
      QString text = QString::fromUtf8(yytext);
      codify(text);

      //endFontClass();
      BEGIN(Body);
   }
  */

<*>{STRINGPREFIX}?{TRISINGLEQUOTE} {
      QString text = QString::fromUtf8(yytext);

      if (YY_START == SingleQuoteString) {
         REJECT;
      }

		startFontClass("stringliteral");
		s_stringContext = YY_START;
		s_doubleQuote   = text[yyleng-1] == '"';
      codify(text);

		BEGIN(TripleString);
   }

<*>{STRINGPREFIX}?{TRIDOUBLEQUOTE} {
      QString text = QString::fromUtf8(yytext);

      if (YY_START == DoubleQuoteString) {
         REJECT;
      }

      startFontClass("stringliteral");
      s_stringContext = YY_START;
      s_doubleQuote   = text[yyleng-1] == '"';
      codify(text);

      BEGIN(TripleString);
   }

<*>{STRINGPREFIX}?"'"          {
      // single quoted string
      QString text = QString::fromUtf8(yytext);

      if (YY_START == SingleQuoteString || YY_START == DoubleQuoteString || YY_START == TripleString) {
         REJECT;
      }

      startFontClass("stringliteral");
      s_stringContext = YY_START;
      codify(text);

      BEGIN(SingleQuoteString);
   }

<*>{STRINGPREFIX}?"\""         {
      // double quoted string
      QString text = QString::fromUtf8(yytext);

      if (YY_START == SingleQuoteString || YY_START == DoubleQuoteString || YY_START == TripleString) {
         REJECT;
      }

      startFontClass("stringliteral");
      s_stringContext = YY_START;
      codify(text);

      BEGIN(DoubleQuoteString);
   }

<DocBlock>.*             {
      // contents of current comment line
      QString text = QString::fromUtf8(yytext);
      s_docBlock += text;
   }

<DocBlock>"\n"{B}("#")         {
      // comment block (next line is also comment line)
      QString text = QString::fromUtf8(yytext);
      s_docBlock += text;
   }

<DocBlock>{NEWLINE}            {
      // comment block ends at the end of this line
      // remove special comment (default config)

      static const bool stripCodeComments = Config::getBool("strip-code-comments");

      if (stripCodeComments)  {
         s_yyLineNr   += s_docBlock.count('\n');
         s_endComment = true;

      } else  {
         // do not remove comment

         startFontClass("comment");
         codifyLines(s_docBlock);
         endFontClass();
      }

      unput(*yytext);
      yy_pop_state();
   }

<*>{POUNDCOMMENT}.*            {
      if (YY_START == SingleQuoteString || YY_START == DoubleQuoteString || YY_START == TripleString) {
         REJECT;
      }

      QString text = QString::fromUtf8(yytext);
      yy_push_state(YY_START);

      BEGIN(DocBlock);
      s_docBlock = text;
   }

<*>"#".*                       {
      // normal comment
      if (YY_START == SingleQuoteString || YY_START == DoubleQuoteString || YY_START == TripleString) {
         REJECT;
      }

      QString text = QString::fromUtf8(yytext);
      startFontClass("comment");

      codifyLines(text);
      endFontClass();
   }

<*>{NEWLINE}                   {
      if (s_endComment) {
         s_endComment = false;
      } else {
         QString text = QString::fromUtf8(yytext);
         codifyLines(text);
      }

      BEGIN(Body);
   }

<*>[ \t]+                      {
      QString text = QString::fromUtf8(yytext);
      codify(text);
      BEGIN(Body);
   }

<*>[\xC0-\xFF][\x80-\xBF]+     {
      // utf-8 code point
      QString text = QString::fromUtf8(yytext);
      codify(text);
      BEGIN(Body);
   }

<*>.                           {
      // catch all
      QString text = QString::fromUtf8(yytext);
      codify(text);
      BEGIN(Body);
   }

<*><<EOF>>                     {
      static const bool stripCodeComments = Config::getBool("strip-code-comments");

      if (YY_START == DocBlock) {

         if (! stripCodeComments)  {
            startFontClass("comment");
            codifyLines(s_docBlock);
            endFontClass();
         }
       }

       yyterminate();
   }
%%

void resetPythonCodeParserState()
{
   s_currentDefinition = QSharedPointer<Definition>();
   s_currentMemberDef  = QSharedPointer<MemberDef>();

   s_doubleStringIsDoc = false;
   s_paramParens = 0;
   s_indents.clear();
   BEGIN( Body );
}

/*
  Examines current stack of white-space indentations;
  re-syncs the parser with the correct scope.
*/
static void adjustScopesAndSuites(unsigned indentLength)
{
   // States to pop
   if (!s_indents.isEmpty() && indentLength < s_indents.top()) {

      while (! s_indents.isEmpty() && indentLength < s_indents.top()) {
         // pop the old suite's indentation
         s_indents.pop();

         s_currentMemberDef = QSharedPointer<MemberDef>();

         if (s_currentDefinition) {
            s_currentDefinition = s_currentDefinition->getOuterScope();
         }
      }
   }

   // Are there any remaining indentation levels for suites?
   if (! s_indents.isEmpty()) {
    BEGIN( Suite );

   } else {
    BEGIN( Body );
   }
}

void parsePythonCode(CodeGenerator &od, const QString &, const QString &s, bool exBlock, const QString &exName,
                     QSharedPointer<FileDef> fd, int startLine, int endLine, bool inlineFragment,
                     QSharedPointer<MemberDef> , bool, QSharedPointer<Definition> searchCtx, bool collectXRefs)
{
   if (s.isEmpty()) {
      return;
   }

   printlex(yy_flex_debug, true, __FILE__, fd ? fd->fileName() : "" );

   TooltipManager::instance()->clearTooltips();
   s_code             = &od;
   s_inputString      = s;
   s_inputPosition    = 0;
   s_currentFontClass = "";
   s_needsTermination = false;
   s_searchCtx        = searchCtx;
   s_collectXRefs     = collectXRefs;

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

   s_includeCodeFragment = inlineFragment;

   bool cleanupSourceDef = false;

   if (exBlock && fd == nullptr) {
      // create a dummy filedef for the example
      s_sourceFileDef  = QMakeShared<FileDef>("", (! exName.isEmpty() ? exName : "generated"));
      cleanupSourceDef = true;
   }

   if (s_sourceFileDef) {
      setCurrentDoc("l00001");
   }

   // Starts line 1 on the output
   startCodeLine();

   yyrestart( yyin );
   yylex();

   if (s_needsTermination) {
      endCodeLine();
   }

   if (fd != nullptr) {
      TooltipManager::instance()->writeTooltips(*s_code);
   }

   if (cleanupSourceDef) {
      // clear the temporary file definition used for this example
      s_sourceFileDef = QSharedPointer<FileDef>();
   }

   printlex(yy_flex_debug, false, __FILE__, fd ? fd->fileName() : "" );
}
