/*************************************************************************
 *
 * Copyright (c) 2014-2023 Barbara Geller & Ansel Sermersheim
 * Copyright (c) 1997-2014 Dimitri van Heesch
 *
*************************************************************************/

%{

#include <QDir>

#include <stdio.h>
#include <assert.h>
#include <ctype.h>

#include <code_cstyle_internal.h>

#include <arguments.h>
#include <config.h>
#include <doxy_globals.h>
#include <entry.h>
#include <message.h>
#include <outputlist.h>
#include <stringmap.h>
#include <tooltip.h>
#include <util.h>

// Toggle for some debugging info
// #define DBG_CTX(x) fprintf x

#define DBG_CTX(x) do { } while(0)

#define CLASSBLOCK 1
#define SCOPEBLOCK 2
#define INNERBLOCK 3

static bool          s_stripCodeComments;

static CodeGenerator *g_code;

static QString       g_curClassName;
static QStringList   g_curClassBases;

static QString       g_parmType;
static QString       g_parmName;

static QString       s_inputString;        // the code fragment as text
static int	         s_inputPosition;      // read offset during parsing
static int           g_inputLines;         // number of line in the code fragment
static int	         g_yyLineNr;           // current line number
static int	         g_yyColNr;            // current column number

static bool          g_skipCodify;
static bool          g_needsTermination;

static bool          g_exampleBlock;
static QString       g_exampleName;
static QString       g_exampleFile;

static bool          g_insideTemplate;
static QString       g_type;
static QString       g_name;
static QString       g_args;

static QString       g_classScope;
static QString       g_realScope;
static QStack<int>   g_scopeStack;
static QString       g_forceTagReference;

static int           g_anchorCount = 0;

static bool          g_lineNumbers;
static bool          g_includeCodeFragment;

static QString       g_currentFontClass;
static bool          g_searchingForBody;
static bool          g_insideBody;
static int           g_bodyCurlyCount;
static QString       g_saveName;
static QString       g_saveType;
static QString       g_delimiter;

static int           g_bracketCount = 0;
static int           g_curlyCount   = 0;
static int           g_sharpCount   = 0;
static bool          g_inFunctionTryBlock  = false;
static bool          g_inForEachExpression = false;

static int           g_lastTemplCastContext;
static int           g_lastSpecialCContext;
static int           g_lastStringContext;
static int           g_lastSkipCppContext;
static int           g_lastVerbStringContext;
static int           g_lastObjCCallContext;
static int           g_memCallContext;
static int           g_lastCContext;
static int           g_skipInlineInitContext;

static bool          g_insideCpp;
static bool          g_insideObjC;
static bool          g_insideJava;
static bool          g_insideCS;
static bool          g_insidePHP;
static bool          g_insideProtocolList;

static bool          g_lexInit = false;
static bool          g_collectXRefs;

static int           s_prefix_this = false;

static ClassSDict                  g_codeClassSDict;
static QSharedPointer<FileDef>     g_sourceFileDef;
static QSharedPointer<Definition>  g_currentDefinition;
static QSharedPointer<MemberDef>   g_currentMemberDef;

static QStack<int>                 g_classScopeLengthStack;
static QSharedPointer<Definition>  g_searchCtx;

static VariableContext             g_theVarContext;
static QStack<CallContext>         g_contextStack;

static QHash<long, QString *>      g_nameDict;
static QHash<long, QString *>      g_objectDict;
static QHash<long, QString *>      g_wordDict;
static QHash<long, QString *>      g_commentDict;

static int g_currentCtxId     = 0;
static int g_currentNameId    = 0;
static int g_currentObjId     = 0;
static int g_currentWordId    = 0;
static int g_currentCommentId = 0;
static int g_braceCount       = 0;

// context for an Objective-C method call
struct ObjCCallCtx
{
   int id;

   QString methodName;
   QString objectTypeOrName;
   QString comment;
   QString format;

   QSharedPointer<ClassDef> objectType;
   QSharedPointer<MemberDef> objectVar;
   QSharedPointer<MemberDef> method;

   int lexState;
   int braceCount;
};

// globals for objective-C method calls
static ObjCCallCtx *g_ObjC_currentCtx = nullptr;

static QStack<ObjCCallCtx *>      g_ObjC_contextStack;
static QHash<long, ObjCCallCtx *> g_ObjC_contextDict;

static void endFontClass();
static void startFontClass(const QString &s);
static void saveObjCContext();
static void restoreObjCContext();

static QSharedPointer<Definition> getScopeCallContext()
{
   if (! g_contextStack.isEmpty()) {
      CallContext &temp = g_contextStack.top();
      return temp.d;

   } else {
      return QSharedPointer<Definition>();

   }
}

static void setScopeCallContext(QSharedPointer<Definition> def)
{
   if (! g_contextStack.isEmpty()) {
      CallContext &temp = g_contextStack.top();
      temp.d = def;
   }
}

static void popCallContext()
{
   if (! g_contextStack.isEmpty()) {
      CallContext temp = g_contextStack.pop();

      g_name = temp.name;
      g_type = temp.type;
   }
}

static void pushCallContext()
{
   CallContext temp{};
   temp.name = g_name;
   temp.type = g_type;

   g_contextStack.push(std::move(temp));
}

QSharedPointer<ClassDef> VariableContext::dummyContext()
{
   static QSharedPointer<ClassDef> dummyContext = QMakeShared<ClassDef>(QString(), 0, 0, "dummyContext-code", CompoundType::Class);
   return dummyContext;
}

void VariableContext::addVariable(const QString &type, const QString &name)
{
   QString ltype = type.simplified();
   QString lname = name.simplified();

   if (ltype.startsWith("struct ")) {
      ltype = ltype.right(ltype.length() - 7);

   } else if (ltype.startsWith("union ")) {
      ltype = ltype.right(ltype.length() - 6);
   }

   if (ltype.isEmpty() || lname.isEmpty()) {
      return;
   }

   DBG_CTX((stderr, "** addVariable() trying: type = '%s' name = '%s' g_currentDefinition = %s\n",
            csPrintable(ltype), csPrintable(lname),
            g_currentDefinition ? csPrintable(g_currentDefinition->name()) : "<none>"));

   StringMap<QSharedPointer<ClassDef>> *scope = m_scopes.count() == 0 ? &m_globalScope : m_scopes.last();

   QSharedPointer<ClassDef> varType;

   int i = 0;

   if ((varType = g_codeClassSDict.find(ltype)) ||
       (varType = getResolvedClass(g_currentDefinition, g_sourceFileDef, ltype))) {

      // look for class definitions inside the code block
      // look for global class definitions

      DBG_CTX((stderr, "** addVariable() type = '%s' name = '%s'\n", csPrintable(ltype), csPrintable(lname)));

      scope->insert(lname, varType); // add it to a list

   } else if ((i = ltype.indexOf('<')) != -1) {
      // probably a template class
      QString typeName(ltype.left(i));

      QSharedPointer<ClassDef> newDef;
      QString templateArgs(ltype.right(ltype.length() - i));

      if ( ( (varType = g_codeClassSDict.find(typeName)) ||
             (varType = getResolvedClass(g_currentDefinition, g_sourceFileDef, typeName, 0, 0, true, true)) ) &&
              ! varType->getTemplateArgumentList().listEmpty() ) {

         // look for class definitions inside the code block, otherwise look for global class definitions
         newDef = varType->getVariableInstance(templateArgs);
      }

      if (newDef) {
         DBG_CTX((stderr, "** addVariable() type = '%s' templateArgs = '%s' name = '%s'\n", csPrintable(typeName),
                  csPrintable(templateArgs), csPrintable(lname)));

         scope->insert(lname, newDef);

      } else {
         // does not seem to be a template, try just the base name
         addVariable(typeName, name);

      }

   } else {
      if (m_scopes.count() > 0)  {
         // for local variables add a dummy entry so the name
         // is hidden to avoid false links to global variables with the same name
         // TODO: make this work for namespaces as well

         DBG_CTX((stderr, "** addVariable() dummy context for '%s'\n", csPrintable(lname)));
         scope->insert(lname, VariableContext::dummyContext());

      } else {
         DBG_CTX((stderr, "** addVariable() not adding variable\n"));

      }
   }
}

QSharedPointer<ClassDef> VariableContext::findVariable(const QString &name)
{
   if (name.isEmpty()) {
      return QSharedPointer<ClassDef>();
   }

   QSharedPointer<ClassDef> result;
   QString key = name;

   // search from inner to outer scope
   for (const auto &scope : m_scopes) {
      result = scope->find(key);

      if (result) {
         DBG_CTX((stderr, "** findVariable(%s) = %p\n", csPrintable(name), result));
         return result;
      }
   }

   // nothing found -> also try the global scope
   result = m_globalScope.find(name);
   DBG_CTX((stderr, "** findVariable(%s) = %p\n", csPrintable(name), result));

   return result;
}

/*! add class/namespace name s to the scope */
static void pushScope(const QString &s)
{
   g_classScopeLengthStack.push(g_classScope.length());

   if (g_classScope.isEmpty() || leftScopeMatch(s, g_classScope)) {
      g_classScope = s;

   } else {
      g_classScope += "::";
      g_classScope += s;
   }
}

/*! remove the top class/namespace name from the scope */
static void popScope()
{
   if (! g_classScopeLengthStack.isEmpty()) {

      int pLength = g_classScopeLengthStack.pop();
      g_classScope.truncate(pLength);
   }
}

static void setCurrentDoc(const QString &anchor)
{
   if (Doxy_Globals::searchIndexBase != nullptr) {
      if (g_searchCtx) {
         g_code->setCurrentDoc(g_searchCtx, g_searchCtx->anchor(), false);
      } else {
         g_code->setCurrentDoc(g_sourceFileDef, anchor, true);
      }
   }
}

static void addToSearchIndex(const QString &text)
{
   if (Doxy_Globals::searchIndexBase != nullptr) {
      g_code->addWord(text, false);
   }
}

static void setClassScope(const QString &name)
{
   QString n = name;

   n = n.simplified();
   int ts = n.indexOf('<');     // start of template
   int te = n.lastIndexOf('>'); // end of template

   if (ts != -1 && te != -1 && te > ts) {
      // remove template from scope
      n = n.left(ts) + n.right(n.length() - te - 1);
   }

   while (! g_classScopeLengthStack.isEmpty()) {
      popScope();
   }

   g_classScope.resize(0);

   int i;
   while ((i = n.indexOf("::")) != -1) {
      pushScope(n.left(i));
      n = n.mid(i + 2);
   }

   pushScope(n);
}

static bool skipLanguageKeywords(const QString &keyword)
{
   static QSet<QString> non_cpp_keywords = {
     "__assume", "__super", "abstract", "function", "gcnew", "gcroot", "generic", "get",
     "internal", "null", "pin_ptr", "raise", "remove", "self", "set", "transient", "sealed"
   };

   static QSet<QString> non_java_keywords = {
      "alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit", "atomic_noexcept", "auto",
      "bitand", "bitor", "bool",
      "char8_t", "char16_t", "char32_t", "compl", "concept", "consteval", "constexpr", "constinit",
      "const_cast", "co_await", "co_return", "co_yield",
      "decltype", "delete", "dynamic_cast", "explicit", "export", "extern",
      "friend", "inline", "mutable", "namespace", "noexcept", "not", "not_eq", "nullptr",
      "operator", "or", "or_eq", "reflexpr", "register", "reinterpret_cast", "requires",
      "signed", "sizeof", "static_assert", "_Static_assert", "static_cast", "struct",
      "template", "thread_local", "typedef", "typeid", "typename",
      "union", "unsigned", "using", "virtual", "wchar_t", "xor", "xor_eq",
      "override", "sealed"
   };

   bool retval = false;

   if (g_insideCpp) {
      retval = (non_cpp_keywords.contains(keyword));

   } else if (g_insideJava) {
      retval = (non_java_keywords.contains(keyword));

   }

   return retval;
}

/*! start a new line of code, inserting a line number if g_sourceFileDef
 * is true. If a definition starts at the current line, then the line
 * number is linked to the documentation of that definition.
 */
static void startCodeLine()
{
   if (g_sourceFileDef && g_lineNumbers) {
      QSharedPointer<Definition> d  = g_sourceFileDef->getSourceDefinition(g_yyLineNr);

      if (! g_includeCodeFragment && d) {
         g_currentDefinition = d;
         g_currentMemberDef  = g_sourceFileDef->getSourceMember(g_yyLineNr);

         g_insideBody       = false;
         g_searchingForBody = true;
         g_realScope        = d->name();

         g_type.resize(0);
         g_name.resize(0);
         g_args.resize(0);
         g_parmType.resize(0);
         g_parmName.resize(0);

         g_bodyCurlyCount = 0;

         QString lineAnchor = QString("l%1").formatArg(g_yyLineNr, 5, 10, QChar('0'));

         if (g_currentMemberDef) {
            g_code->writeLineNumber(g_currentMemberDef->getReference(), g_currentMemberDef->getOutputFileBase(),
                  g_currentMemberDef->anchor(), g_yyLineNr);

            setCurrentDoc(lineAnchor);

         } else if (d->isLinkableInProject()) {
            g_code->writeLineNumber(d->getReference(), d->getOutputFileBase(), QString(), g_yyLineNr);
            setCurrentDoc(lineAnchor);
         }

      } else {
         g_code->writeLineNumber(QString(), QString(), QString(), g_yyLineNr);
      }
  }

   DBG_CTX((stderr, "startCodeLine(%d)\n", g_yyLineNr));
   g_code->startCodeLine(g_sourceFileDef && g_lineNumbers);

   if (! g_currentFontClass.isEmpty()) {
      g_code->startFontClass(g_currentFontClass);
   }
}

static void endCodeLine()
{
   DBG_CTX((stderr, "endCodeLine(%d)\n", g_yyLineNr));
   endFontClass();
   g_code->endCodeLine();
}

static void nextCodeLine()
{
   QString fc = g_currentFontClass;
   endCodeLine();

   if (g_yyLineNr < g_inputLines) {
      g_currentFontClass = fc;
      startCodeLine();
   }
}

/*! write a code fragment `text' that may span multiple lines, inserting
 * line numbers for each line.
 */
static void codifyLines(const QString &text)
{
   QString tmp;

   for (auto c : text) {

      if (c == '\n') {
         ++g_yyLineNr;
         g_yyColNr = 1;

         g_code->codify(tmp);
         nextCodeLine();

         tmp = "";

      } else {
         tmp += c;
         ++g_yyColNr;

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
static void writeMultiLineCodeLink(CodeGenerator &ol, QSharedPointer<Definition> d,
                  const QString &text)

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
         ++g_yyLineNr;

         ol.writeCodeLink(ref, file, anchor, tmp, tooltip);
         nextCodeLine();

         tmp = "";

      } else {
         tmp += c;

      }
   }

   if (! tmp.isEmpty()) {
      ol.writeCodeLink(ref, file, anchor, tmp, tooltip);
   }
}

static void addType()
{
   if (g_name == "const") {
      g_name.resize(0);
      return;
   }

   if (! g_type.isEmpty()) {
      g_type += ' ';
   }

   g_type += g_name;
   g_name.resize(0);

   if (! g_type.isEmpty()) {
      g_type += ' ';
   }

   g_type += g_args;
   g_args.resize(0);
}

static void addParmType()
{
   if (g_parmName == "const") {
      g_parmName.resize(0);
      return;
   }

   if (!g_parmType.isEmpty()) {
      g_parmType += ' ' ;
   }

   g_parmType += g_parmName;
   g_parmName.resize(0);
}

static void addUsingDirective(const QString &name)
{
   if (g_sourceFileDef && ! name.isEmpty()) {
      QSharedPointer<NamespaceDef> nd = Doxy_Globals::namespaceSDict.find(name);

      if (nd) {
         g_sourceFileDef->addUsingDirective(nd);
      }
   }
}

static void setParameterList(QSharedPointer<MemberDef> md)
{
   g_classScope = md->getClassDef() ? md->getClassDef()->name() : "";
   const ArgumentList &argList = md->getArgumentList();

   for (const auto &arg : argList) {
      g_parmName = arg.name;
      g_parmType = arg.type;

      int i = g_parmType.indexOf('*');
      if (i != -1) {
         g_parmType = g_parmType.left(i);
      }

      i = g_parmType.indexOf('&');
      if (i != -1) {
         g_parmType = g_parmType.left(i);
      }

      g_parmType = stripPrefix(g_parmType, "const ");

      g_parmType = g_parmType.trimmed();
      g_theVarContext.addVariable(g_parmType, g_parmName);
   }
}

static QSharedPointer<ClassDef> stripClassName(const QString &s, QSharedPointer<Definition> d = g_currentDefinition)
{
   int pos = 0;
   QString type = s;
   QString className;
   QString templSpec;

   while (extractClassNameFromType(type, pos, className, templSpec) != -1) {
      QString clName = className + templSpec;
      QSharedPointer<ClassDef> cd;

      if (!g_classScope.isEmpty()) {
         cd = getResolvedClass(d, g_sourceFileDef, g_classScope + "::" + clName);
      }

      if (cd == 0) {
         cd = getResolvedClass(d, g_sourceFileDef, clName);
      }

      if (cd) {
         return cd;
      }
   }

   return QSharedPointer<ClassDef>();
}

static QSharedPointer<MemberDef> setCallContextForVar(const QString &name)
{
  if (name.isEmpty()) {
      return QSharedPointer<MemberDef>();
   }

   DBG_CTX((stderr, "setCallContextForVar(%s) g_classScope = %s\n", csPrintable(name), csPrintable(g_classScope)));

   int scopeEnd = name.lastIndexOf("::");

   if (scopeEnd != -1) {
      // name with explicit scope

      QString scope   = name.left(scopeEnd);
      QString locName = name.right(name.length() - scopeEnd - 2);

      QSharedPointer<ClassDef> mcd = getClass(scope);

      if (mcd && ! locName.isEmpty()) {
         QSharedPointer<MemberDef> md = mcd->getMemberByName(locName);

         if (md) {
            setScopeCallContext(stripClassName(md->typeString(), md->getOuterScope()));
            return md;
         }

      } else {
         // check namespace as well
         QSharedPointer<NamespaceDef> mnd = getResolvedNamespace(scope);

         if (mnd && !locName.isEmpty()) {
            QSharedPointer<MemberDef> md = mnd->getMemberByName(locName);

            if (md) {
               setScopeCallContext(stripClassName(md->typeString(), md->getOuterScope()));
               return md;
            }
         }
      }
   }

   QSharedPointer<MemberName> mn;
   QSharedPointer<ClassDef> mcd = g_theVarContext.findVariable(name);

   if (mcd) {
      // local variable

      if (mcd != VariableContext::dummyContext()) {
         DBG_CTX((stderr, "local var `%s' mcd = %s\n", csPrintable(name), csPrintable(mcd->name())));
         setScopeCallContext(mcd);
      }

   } else {
      DBG_CTX((stderr, "class member? scope = %s\n", csPrintable(g_classScope)));
      // look for a class member
      mcd = getClass(g_classScope);

      if (mcd) {
         DBG_CTX((stderr, "Inside class %s\n", csPrintable(mcd->name())));
         QSharedPointer<MemberDef> md = mcd->getMemberByName(name);

         if (md) {
            DBG_CTX((stderr, "Found member %s\n", csPrintable(md->name())));

            if (g_scopeStack.isEmpty() || g_scopeStack.top() != CLASSBLOCK) {
               DBG_CTX((stderr, "class member `%s' mcd=%s\n", csPrintable(name), csPrintable(mcd->name())));
               setScopeCallContext(stripClassName(md->typeString(), md->getOuterScope() ));
            }

            return md;
         }
      }
   }

   // look for a global member
   if ((mn = Doxy_Globals::functionNameSDict.find(name))) {

      if (mn->count() == 1) { // global defined only once
         QSharedPointer<MemberDef> md( mn->first());

         if (! md->isStatic() || md->getBodyDef() == g_sourceFileDef) {
            setScopeCallContext(stripClassName(md->typeString(), md->getOuterScope()) );
            return md;
         }

         return QSharedPointer<MemberDef>();

      } else if (mn->count() > 1) {
         // global defined more than once

         for (auto md : *mn) {

            // in case there are multiple members we could link to, we
            // only link to members if defined in the same file or
            // defined as external.

            if ((! md->isStatic() || md->getBodyDef() == g_sourceFileDef) &&
                  (g_forceTagReference.isEmpty() || g_forceTagReference == md->getReference())) {

               setScopeCallContext(stripClassName(md->typeString(), md->getOuterScope()));

               return md;
            }
         }

         return QSharedPointer<MemberDef>();
      }
   }

   return QSharedPointer<MemberDef>();
}

static void updateCallContextForSmartPointer()
{
   QSharedPointer<Definition> d = getScopeCallContext();
   QSharedPointer<MemberDef> md;

   if (d && d->definitionType() == Definition::TypeClass && (md = d.dynamicCast<ClassDef>()->isSmartPointer())) {
      QSharedPointer<ClassDef> ncd = stripClassName(md->typeString(), md->getOuterScope() );

      if (ncd) {
         setScopeCallContext(ncd);
      }
   }
}

static bool getLinkInScope(const QString &c, const QString &m, const QString  memberText,
                           CodeGenerator &ol, const QString &text, bool varOnly = false)
{
   QSharedPointer<MemberDef>    md;
   QSharedPointer<ClassDef>     cd;
   QSharedPointer<FileDef>      fd;
   QSharedPointer<NamespaceDef> nd;
   QSharedPointer<GroupDef>     gd;

   DBG_CTX((stderr, "getLinkInScope: trying `%s'::`%s' varOnly = %d\n", csPrintable(c), csPrintable(m), varOnly));

   if (getDefs(c, m, "()", md, cd, fd, nd, gd, false, g_sourceFileDef, false, g_forceTagReference) &&
         (! varOnly || md->isVariable())) {

      if (md->isLinkable()) {

         if (g_exampleBlock) {
            QString anchor = QString("a%1").formatArg(g_anchorCount);

            if (md->addExample(anchor, g_exampleName, g_exampleFile)) {
               ol.writeCodeAnchor(anchor);
               ++g_anchorCount;
            }
         }

         QSharedPointer<Definition> d;

         if (md->getOuterScope() == Doxy_Globals::globalScope) {
            d = md->getFileDef();

         } else {
            d = md->getOuterScope();

         }

         if (md->getGroupDef()) {
            d = md->getGroupDef();
         }

         if (d != nullptr && d->isLinkable()) {
            setScopeCallContext(stripClassName(md->typeString(), md->getOuterScope()));

            if (g_currentDefinition && g_currentMemberDef && md != g_currentMemberDef && g_insideBody && g_collectXRefs) {
               addDocCrossReference(g_currentMemberDef, md);
            }

            QString temp = text;

            if (text.isEmpty()) {
               temp = memberText;
            }

            writeMultiLineCodeLink(ol, md, temp);
            addToSearchIndex(temp);

            return true;
         }

      } else {
         // found member but is not linkable, make sure content inside is not
         //  assigned to the previous member

         g_currentMemberDef = QSharedPointer<MemberDef>();
      }
   }

   return false;
}

static bool getLink(const QString &className, const QString &memberName, CodeGenerator &ol,
                  const QString &text = QString(), bool varOnly = false)
{
   QString m = removeRedundantWhiteSpace(memberName);
   QString c = className;

   if (! getLinkInScope(c, m, memberName, ol, text, varOnly)) {

      if (! g_curClassName.isEmpty()) {
         if (! c.isEmpty()) {
            c.prepend("::");
         }

         c.prepend(g_curClassName);
         return getLinkInScope(c, m, memberName, ol, text, varOnly);
      }

      return false;
   }

   return true;
}

static void generateClassOrGlobalLink(CodeGenerator &ol, const QString &clName, bool typeOnly = false, bool varOnly = false)
{
   if (clName.isEmpty()) {
      return;
   }

   QString className = clName;

   if (className.startsWith('~')) {
      // correct for matching negated values for a destructor
      g_code->codify("~");
      className = className.mid(1);
   }

   // passed to writeMultiLineCodeLink() and used a few other places
   QString tmpClassName = className;

   if (g_insideProtocolList) {
      // for Obj-C
      className += "-p";
   }

   if (g_insidePHP) {
      className = substitute(className, "\\", "::");   // for PHP namespaces

   } else if (g_insideCS || g_insideJava) {
      className = substitute(className, ".", "::");    // for PHP namespaces
   }

   QSharedPointer<ClassDef> cd;
   QSharedPointer<ClassDef> lcd;
   QSharedPointer<MemberDef> md;

   bool isLocal = false;

   if (! s_prefix_this || (lcd = g_theVarContext.findVariable(className)) == 0) {
      // not a local variable
      QSharedPointer<Definition> d = g_currentDefinition;

      int i = className.indexOf('<');
      QString bareName = className;

      if (i != -1) {
         bareName = className.left(i);
      }
      cd = getResolvedClass(d, g_sourceFileDef, className, &md);

      if (cd == nullptr && md == nullptr && ! bareName.isEmpty()) {

         if (bareName != className) {
            // try unspecialized version
            cd = getResolvedClass(d, g_sourceFileDef, bareName, &md);
         }
      }

      QSharedPointer<NamespaceDef> nd = getResolvedNamespace(className);

      if (nd && nd->isLinkableInProject()) {
         setScopeCallContext(nd);
         addToSearchIndex(className);
         writeMultiLineCodeLink(*g_code, nd, tmpClassName);
         return;
      }

      QSharedPointer<ConceptDef> conceptDef = getResolvedConcept(d, bareName);

      if (conceptDef && conceptDef->isLinkable()) {
         setScopeCallContext(conceptDef);
         addToSearchIndex(className);
         writeMultiLineCodeLink(*g_code, conceptDef, tmpClassName);
         return;
       }

      if (cd == nullptr && md == nullptr) {
         // also see if it is variable or enum or enum value

         if (getLink(g_classScope, tmpClassName, ol, tmpClassName, varOnly)) {
            return;
         }
      }

   } else {
      if (lcd != VariableContext::dummyContext()) {
         setScopeCallContext(lcd);
      }

      isLocal = true;
   }

   s_prefix_this = false;       // discard the "this" prefix for the next calls

   if (cd && cd->isLinkable()) {
      // is it a linkable class

      if (g_exampleBlock) {
         QString anchor = QString("_a%1").formatArg(g_anchorCount);

         if (cd->addExample(anchor, g_exampleName, g_exampleFile)) {
            ol.writeCodeAnchor(anchor);
            ++g_anchorCount;
         }
      }

      writeMultiLineCodeLink(ol, cd, tmpClassName);
      addToSearchIndex(className);
      setScopeCallContext(cd);

      if (md) {
         QSharedPointer<Definition> d;

         if (md->getOuterScope() == Doxy_Globals::globalScope) {
            d = md->getFileDef();

         } else {
            d = md->getOuterScope();

         }

         if (md->getGroupDef()) {
            d = md->getGroupDef();
         }

         if (d && d->isLinkable() && md->isLinkable() && g_currentMemberDef && g_collectXRefs) {
            addDocCrossReference(g_currentMemberDef, md) ;
         }
      }

   } else {
      // not a class, maybe a global member

      if (! isLocal && (md != nullptr || (cd == nullptr && ! typeOnly))) {
         // not a class, see if it is a global enum/variable/typedef

         if (md == nullptr) {
            // not found as a typedef
            md = setCallContextForVar(tmpClassName);

            if (md && g_currentDefinition && isAccessibleFrom(g_currentDefinition, g_sourceFileDef, md) == -1) {
               md = QSharedPointer<MemberDef>(); // variable not accessible
            }
         }

         if (md && (! varOnly || md->isVariable())) {

            if (md->isLinkable()) {
               QString text;

               if (! g_forceTagReference.isEmpty()) {
                  // explicit reference to symbol in tag file
                  text = g_forceTagReference;

                  if (text.endsWith(".tag")) {
                     // strip .tag if present
                     text = text.left(text.length() - 4);
                  }

                  text += getLanguageSpecificSeparator(md->getLanguage());
                  text += tmpClassName;

                  md->setName(text);
                  md->setLocalName(text);

               } else {
                  // normal reference
                  text = tmpClassName;
               }

               writeMultiLineCodeLink(ol, md, text);
               addToSearchIndex(tmpClassName);

               if (g_currentMemberDef && g_collectXRefs) {
                  addDocCrossReference(g_currentMemberDef, md);
               }

               return;
            }
         }
      }

      // nothing found, just write out the word
      codifyLines(tmpClassName);
      addToSearchIndex(tmpClassName);
   }
}

static bool generateClassMemberLink(CodeGenerator &ol, QSharedPointer<MemberDef> xmd, const QString &memName)
{
   // extract class definition of the return type in order to resolve
   // a->b()->c() like call chains

   if (g_exampleBlock) {

      QString anchor = QString("a%1").formatArg(g_anchorCount);

      if (xmd->addExample(anchor, g_exampleName, g_exampleFile)) {
         ol.writeCodeAnchor(anchor);
         g_anchorCount++;
      }
   }

   QSharedPointer<ClassDef> typeClass = stripClassName(removeAnonymousScopes(xmd->typeString()), xmd->getOuterScope() );

   DBG_CTX((stderr, "%s -> typeName=%p\n", xmd->typeString(), typeClass));

   setScopeCallContext(typeClass);
   QSharedPointer<Definition> xd;

   if (xmd->getOuterScope() == Doxy_Globals::globalScope) {
      xd = xmd->getFileDef();

   } else {
      xd = xmd->getOuterScope();

   }

   if (xmd->getGroupDef()) {
      xd = xmd->getGroupDef();
   }

   if (xd && xd->isLinkable()) {

      if (xmd->templateMaster()) {
         xmd = xmd->templateMaster();
      }

      if (xmd->isLinkable()) {
         // add usage reference
         if (g_currentDefinition && g_currentMemberDef && g_insideBody && g_collectXRefs) {
            addDocCrossReference(g_currentMemberDef, xmd);
         }

         // write the actual link
         writeMultiLineCodeLink(ol, xmd, memName);
         addToSearchIndex(memName);
         return true;
      }
   }

   return false;
}

static bool generateClassMemberLink(CodeGenerator &ol, QSharedPointer<Definition> def, const QString &memName)
{
   if (def && def->definitionType() == Definition::TypeClass) {
      QSharedPointer<ClassDef>  cd  = def.dynamicCast<ClassDef>();
      QSharedPointer<MemberDef> xmd = cd->getMemberByName(memName);

      if (xmd) {
         return generateClassMemberLink(ol, xmd, memName);

      } else {
         QSharedPointer<Definition> innerDef = cd->findInnerCompound(memName);

         if (innerDef) {
            setScopeCallContext(innerDef);
            addToSearchIndex(memName);
            writeMultiLineCodeLink(*g_code, innerDef, memName);
            return true;
         }
      }

   } else if (def && def->definitionType() == Definition::TypeNamespace) {
      QSharedPointer<NamespaceDef> nd = def.dynamicCast<NamespaceDef>();
      QSharedPointer<Definition> innerDef = nd->findInnerCompound(memName);

      if (innerDef) {
         setScopeCallContext(innerDef);
         addToSearchIndex(memName);
         writeMultiLineCodeLink(*g_code, innerDef, memName);
         return true;
      }
   }

   return false;
}

static void generateMemberLink(CodeGenerator &ol, const QString &varName, const QString &memName)
{
   if (varName.isEmpty()) {
      return;
   }

   // look for the variable in the current context
   QSharedPointer<ClassDef> vcd = g_theVarContext.findVariable(varName);

   if (vcd) {
      if (vcd != VariableContext::dummyContext()) {

         if (getLink(vcd->name(), memName, ol)) {
            return;
         }

         if (vcd->baseClasses()) {
            for (const auto &item : *vcd->baseClasses()) {
               if (getLink(item->classDef->name(), memName, ol)) {
                  return;
               }
            }
         }
      }

   } else {
      // variable not in current context, maybe it is in a parent context
      vcd = getResolvedClass(g_currentDefinition, g_sourceFileDef, g_classScope);

      if (vcd != nullptr && vcd->isLinkable()) {

         QSharedPointer<MemberName> vmn = Doxy_Globals::memberNameSDict.find(varName);

         if (vmn == 0) {
            int vi;
            QString vn = varName;

            if ((vi = vn.lastIndexOf("::")) != -1 || (vi = vn.lastIndexOf('.')) != -1) {
               // explicit scope A::b(), probably static member
               QSharedPointer<ClassDef> jcd = getClass(vn.left(vi));

               vn = vn.right(vn.length() - vi - 2);
               vmn = Doxy_Globals::memberNameSDict.find(vn);

               if (vmn) {

                  for (const auto &vmd : *vmn)  {
                     if (vmd->getClassDef() == jcd) {

                        QSharedPointer<ClassDef> mcd = stripClassName(vmd->typeString(), vmd->getOuterScope());

                        if (mcd != nullptr && mcd->isLinkable()) {
                           if (generateClassMemberLink(ol, mcd.staticCast<Definition>(), memName)) {
                              return;
                           }
                        }
                     }
                  }
               }
            }
         }

         if (vmn) {
            for (const auto &vmd : *vmn) {
               if (vmd->getClassDef() == vcd) {
                  QSharedPointer<ClassDef> mcd = stripClassName(vmd->typeString(), vmd->getOuterScope() );

                  if (mcd != nullptr && mcd->isLinkable()) {
                     if (generateClassMemberLink(ol, mcd.staticCast<Definition>(), memName)) {
                        return;
                     }
                  }
               }
            }
         }
      }
   }

   // nothing found -> write result as is
   codifyLines(memName);
   addToSearchIndex(memName);
   return;
}

static void generatePHPVariableLink(CodeGenerator &ol, const QString &varName)
{
   // strip $this->
   QString name = varName.mid(7);
   name.prepend("$");

   if (! getLink(g_classScope, name, ol, varName)) {
      codifyLines(varName);
   }
}

static void generateFunctionLink(CodeGenerator &ol, const QString &funcName)
{
   QSharedPointer<ClassDef> ccd;

   QString locScope = g_classScope;
   QString locFunc = removeRedundantWhiteSpace(funcName);

   if (g_insidePHP && locFunc.startsWith("self::")) {
      locFunc = locFunc.mid(4);
   }

   QString funcScope;
   QString funcWithScope = locFunc;
   QString funcWithFullScope = locFunc;
   QString fullScope = locScope;

   DBG_CTX((stdout, "*** locScope=%s locFunc=%s\n", csPrintable(locScope), csPrintable(locFunc)));

   int len = 2;
   int i = locFunc.lastIndexOf("::");

   if (g_currentMemberDef && g_currentMemberDef->getClassDef() && funcName == g_currentMemberDef->localName() &&
         g_currentMemberDef->getDefLine() == g_yyLineNr && generateClassMemberLink(ol, g_currentMemberDef, funcName)) {

      // special case where funcName is the name of a method that is also
      // defined on this line. In this case we can directly link to
      // g_currentMemberDef, which is not only faster, but
      // in case of overloaded methods, this will make sure that we link to
      // the correct method, and thereby get the correct reimplemented relations.
      goto exit;
   }

   if (i == -1) {
      i = locFunc.lastIndexOf("."), len = 1;
   }

   if (i == -1) {
      i = locFunc.lastIndexOf("\\"), len = 1;   // for PHP
   }

   if (i > 0) {
      funcScope = locFunc.left(i);
      locFunc = locFunc.right(locFunc.length() - i - len).trimmed();
      int ts = locScope.indexOf('<');           // start of template
      int te = locScope.lastIndexOf('>');       // end of template

      if (ts != -1 && te != -1 && te > ts) {
         // remove template from scope
         locScope = locScope.left(ts) + locScope.right(locScope.length() - te - 1);
      }

      ts = funcScope.indexOf('<');              // start of template
      te = funcScope.lastIndexOf('>');          // end of template

      if (ts != -1 && te != -1 && te > ts) {
         // remove template from scope
         funcScope = funcScope.left(ts) + funcScope.right(funcScope.length() - te - 1);
      }

      if (! funcScope.isEmpty()) {
         funcWithScope = funcScope + "::" + locFunc;
         if (!locScope.isEmpty()) {
            fullScope = locScope + "::" + funcScope;
         }
      }
      if (! locScope.isEmpty()) {
         funcWithFullScope = locScope + "::" + funcWithScope;
      }
   }

   if (! fullScope.isEmpty() && (ccd = g_codeClassSDict.find(fullScope))) {

      if (ccd->baseClasses()) {
         for (const auto &item : *ccd->baseClasses()) {
            if (getLink(item->classDef->name(), locFunc, ol, funcName)) {
               goto exit;
            }
         }
      }
   }

   if (! locScope.isEmpty() && fullScope != locScope && (ccd = g_codeClassSDict.find(locScope))) {

      if (ccd->baseClasses()) {

         for (const auto &item : *ccd->baseClasses()) {
            if (getLink(item->classDef->name(), funcWithScope, ol, funcName)) {
               goto exit;
            }
         }
      }
   }

   if (! getLink(locScope, funcWithScope, ol, funcName)) {
      generateClassOrGlobalLink(ol, funcName);
   }

exit:
   g_forceTagReference.resize(0);
   return;
}

static bool isCastKeyword(const QString &text)
{
   auto iter = text.indexOfFast('<');

   if (iter == text.constEnd()) {
      return false;
   }

   QStringView word = QStringView(text.constBegin(), iter).trimmed();

   if (! word.endsWith("_cast")) {
      return false;
   }

   return word == "const_cast" || word == "static_cast" || word == "dynamic_cast" || word == "reinterpret_cast";
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
      g_needsTermination = true;
   }

   return count;
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
   endFontClass();
   g_code->startFontClass(s);
   g_currentFontClass = s;
}

// recursively writes a linkified Objective-C method call
static void writeObjCMethodCall(ObjCCallCtx *ctx)
{
   if (ctx == nullptr) {
      return;
   }

   QString::const_iterator iter = ctx->format.constBegin();

   if (! ctx->methodName.isEmpty()) {

      if (! ctx->objectTypeOrName.isEmpty() && ctx->objectTypeOrName.at(0) != '$') {

         QSharedPointer<ClassDef> cd = g_theVarContext.findVariable(ctx->objectTypeOrName);

         if (cd == 0) {
            // not a local variable
            if (ctx->objectTypeOrName == "self") {

               if (g_currentDefinition && g_currentDefinition->definitionType() == Definition::TypeClass) {
                  ctx->objectType = g_currentDefinition.dynamicCast<ClassDef>();
               }

            } else {
               ctx->objectType = getResolvedClass(g_currentDefinition, g_sourceFileDef, ctx->objectTypeOrName, &ctx->method);
            }

            if (ctx->objectType) {
               // found class
               ctx->method = ctx->objectType->getMemberByName(ctx->methodName);

            } else if (ctx->method == 0) {
               // search for class variable with the same name

               if (g_currentDefinition && g_currentDefinition->definitionType() == Definition::TypeClass) {
                  ctx->objectVar = g_currentDefinition.dynamicCast<ClassDef>()->getMemberByName(ctx->objectTypeOrName);

                  if (ctx->objectVar) {
                     ctx->objectType = stripClassName(ctx->objectVar->typeString());

                     if (ctx->objectType) {
                        ctx->method = ctx->objectType->getMemberByName(ctx->methodName);

                     }
                  }
               }
            }

         } else {
            // local variable

            if (cd != VariableContext::dummyContext()) {
               ctx->method = cd->getMemberByName(ctx->methodName);
            }
         }
      }
   }

   while (iter != ctx->format.constEnd())  {

      QChar c = *iter;
      ++iter;

      // for each character in ctx->format

      if (c == '$') {
         QChar nc = '\0';

         if (iter != ctx->format.constEnd()) {
            nc = *iter;
            ++iter;
         }

         if (nc == '$') {
            // escaped $
            g_code->codify("$");

         } else {
            // name fragment or reference to a nested call

            if (nc == 'n') {
               // name fragment
               nc = '\0';

               if (iter != ctx->format.constEnd()) {
                  nc = *iter;
                  ++iter;
               }

               QString refIdStr;

               while (nc != 0 && nc.isNumber() ) {
                  refIdStr += nc;

                  nc = '\0';

                  if (iter != ctx->format.constEnd()) {
                     nc = *iter;
                     ++iter;
                  }

               }
               --iter;

               int refId = refIdStr.toInteger<int>();
               QString *pName = g_nameDict.value(refId);

               if (pName) {
                  if (ctx->method && ctx->method->isLinkable()) {
                     writeMultiLineCodeLink(*g_code, ctx->method, *pName);

                     if (g_currentMemberDef && g_collectXRefs) {
                        addDocCrossReference(g_currentMemberDef, ctx->method);
                     }

                  } else {
                     codifyLines(*pName);
                  }
               }

            } else if (nc == 'o') {
               // reference to potential object name
               nc = '\0';

               if (iter != ctx->format.constEnd()) {
                  nc = *iter;
                  ++iter;
               }

               QString refIdStr;

               while (nc != 0 && nc.isNumber()) {
                  refIdStr += nc;

                  nc = '\0';

                  if (iter != ctx->format.constEnd()) {
                     nc = *iter;
                     ++iter;
                  }
               }
               --iter;

               int refId = refIdStr.toInteger<int>();
               QString *pObject = g_objectDict.value(refId);

               if (pObject) {
                  if (*pObject == "self") {
                     if (g_currentDefinition && g_currentDefinition->definitionType() == Definition::TypeClass) {

                        ctx->objectType = g_currentDefinition.dynamicCast<ClassDef>();

                        if (ctx->objectType->categoryOf()) {
                           ctx->objectType = ctx->objectType->categoryOf();
                        }

                        if (ctx->objectType) {
                           ctx->method = ctx->objectType->getMemberByName(ctx->methodName);
                        }
                     }

                     startFontClass("keyword");
                     codifyLines(*pObject);
                     endFontClass();

                  } else if (*pObject == "super") {

                     if (g_currentDefinition && g_currentDefinition->definitionType() == Definition::TypeClass) {

                        QSharedPointer<ClassDef> cd = g_currentDefinition.dynamicCast<ClassDef>();

                        if (cd->categoryOf()) {
                           cd = cd->categoryOf();
                        }

                        SortedList<BaseClassDef *> *bcd = cd->baseClasses();

                        if (bcd != nullptr) {
                           // get direct base class (there should be only one)

                           for (const auto &bclass : *bcd) {
                              if (bclass->classDef->compoundType() != CompoundType::Protocol) {
                                 ctx->objectType = bclass->classDef;

                                 if (ctx->objectType) {
                                    ctx->method = ctx->objectType->getMemberByName(ctx->methodName);
                                 }
                              }
                           }
                        }

                     }

                     startFontClass("keyword");
                     codifyLines(*pObject);
                     endFontClass();

                  } else if (ctx->objectVar && ctx->objectVar->isLinkable()) {
                     // object is class variable
                     writeMultiLineCodeLink(*g_code, ctx->objectVar, *pObject);

                     if (g_currentMemberDef && g_collectXRefs) {
                        addDocCrossReference(g_currentMemberDef, ctx->objectVar);
                     }

                  } else if (ctx->objectType && ctx->objectType != VariableContext::dummyContext() &&
                             ctx->objectType->isLinkable() ) {

                     // object is class name
                     QSharedPointer<ClassDef> cd = ctx->objectType;
                     writeMultiLineCodeLink(*g_code, cd, *pObject);

                  } else { // object still needs to be resolved

                     QSharedPointer<ClassDef> cd = getResolvedClass(g_currentDefinition, g_sourceFileDef, *pObject);

                     if (cd && cd->isLinkable()) {
                        if (ctx->objectType == 0) {
                           ctx->objectType = cd;
                        }
                        writeMultiLineCodeLink(*g_code, cd, *pObject);
                     } else {
                        codifyLines(*pObject);
                     }
                  }
               }

            } else if (nc == 'c') {
               // reference to nested call

               nc = '\0';

               if (iter != ctx->format.constEnd()) {
                  nc = *iter;
                  ++iter;
               }

               QString refIdStr;

               while (nc != 0 && nc.isNumber()) {
                  refIdStr += nc;

                  nc = '\0';

                  if (iter != ctx->format.constEnd()) {
                     nc = *iter;
                     ++iter;
                  }
               }
               --iter;

               int refId = refIdStr.toInteger<int>();
               ObjCCallCtx *ictx = g_ObjC_contextDict.value(refId);

               if (ictx) {
                  // recurse into nested call
                  writeObjCMethodCall(ictx);

                  if (ictx->method) { // link to nested call successfully
                     // get the ClassDef representing the method's return type

                     if (QString(ictx->method->typeString()) == "id") {
                        // see if the method name is unique, if so we link to it

                        QSharedPointer<MemberName> mn = Doxy_Globals::memberNameSDict.find(ctx->methodName);

                        if (mn && mn->count() == 1) {
                           // member name unique
                           ctx->method = mn->first();
                        }

                     } else {
                        ctx->objectType = stripClassName(ictx->method->typeString());
                        if (ctx->objectType) {
                           ctx->method = ctx->objectType->getMemberByName(ctx->methodName);
                        }
                     }

                  }
               }

            } else if (nc == 'w') {
               // some word
               nc = '\0';

               if (iter != ctx->format.constEnd()) {
                  nc = *iter;
                  ++iter;
               }

               QString refIdStr;

               while (nc != 0 && nc.isNumber()) {
                  refIdStr += nc;

                  nc = '\0';

                  if (iter != ctx->format.constEnd()) {
                     nc = *iter;
                     ++iter;
                  }
               }
               --iter;

               int refId = refIdStr.toInteger<int>();
               QString *pWord = g_wordDict.value(refId);

               if (pWord) {
                  codifyLines(*pWord);
               }

            } else if (nc == 'd')  {
               // comment block
               nc = '\0';

               if (iter != ctx->format.constEnd()) {
                  nc = *iter;
                  ++iter;
               }

               QString refIdStr;

               while (nc != 0 && nc.isNumber()) {
                  refIdStr += nc;

                  nc = '\0';

                  if (iter != ctx->format.constEnd()) {
                     nc = *iter;
                     ++iter;
                  }

               }
               --iter;

               int refId = refIdStr.toInteger<int>();

               QString *pComment = g_commentDict.value(refId);

               if (pComment) {
                  startFontClass("comment");
                  codifyLines(*pComment);
                  endFontClass();
               }

            } else {
               // illegal marker
               assert(! "Invalid escape sequence");

            }
         }

      } else {
         // normal non-marker character
         codifyLines(c);
      }
   }

}

// Replaces an Objective-C method name fragment s by a marker of the form
// $n12, the number (12) can later be used as a key for obtaining the name
// fragment, from g_nameDict
static QString escapeName(const QString &s)
{
   QString result = QString("$n%1").formatArg(g_currentNameId);

   g_nameDict.insert(g_currentNameId, new QString(s));
   g_currentNameId++;
   return result;
}

static QString escapeObject(const QString &s)
{
   QString result = QString("$o%1").formatArg(g_currentObjId);

   g_objectDict.insert(g_currentObjId, new QString(s));
   g_currentObjId++;
   return result;
}

static QString escapeWord(const QString &s)
{
   QString result = QString("$w%d").formatArg(g_currentWordId);

   g_wordDict.insert(g_currentWordId, new QString(s));
   g_currentWordId++;
   return result;
}

static QString escapeComment(const QString &s)
{
  QString result = QString("$d%d").formatArg(g_currentCommentId);

  g_commentDict.insert(g_currentCommentId, new QString(s));
  g_currentCommentId++;
  return result;
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

B           [ \t]
BN          [ \t\n\r]
ID          [$a-z_A-Z\x80-\xFF][$a-z_A-Z0-9\x80-\xFF]*
SEP         ("::"|"\\")
SCOPENAME   ("::"{BN}*)?({ID}{BN}*{SEP}{BN}*)*("~"{BN}*)?{ID}
TEMPLIST    "<"[^\"\}\{\(\)\/\n\>]*">"
SCOPETNAME  (((({ID}{TEMPLIST}?){BN}*)?{SEP}{BN}*)*)((~{BN}*)?{ID})
SCOPEPREFIX ({ID}{TEMPLIST}?{BN}*{SEP}{BN}*)+

KEYWORD_OBJC_A ("@public"|"@private"|"@protected"|"@class"|"@implementation"|"@interface"|"@end"|"@selector"|"@protocol")
KEYWORD_OBJC_B ("@optional"|"@required"|"@throw"|"@synthesize"|"@property")
KEYWORD_OBJC   {KEYWORD_OBJC_A}|{KEYWORD_OBJC_B}

  /* review skipLanguageKeywords()  */
KEYWORD_A   ("asm"|"__assume"|"auto"|"class"|"const"|"delete"|"enum"|"explicit"|"extern"|"false"|"friend"|"gcnew"|"gcroot"|"set"|"get")
KEYWORD_B   ("inline"|"internal"|"mutable"|"namespace"|"new"|"null"|"nullptr"|"override"|"operator"|"pin_ptr"|"private"|"protected"|"public")
KEYWORD_C   ("raise"|"register"|"remove"|"self"|"sizeof"|"static"|"struct"|"__super"|"function"|"template"|"generic"|"this"|"true"|"typedef")
KEYWORD_D   ("typeid"|"typename"|"union"|"using"|"virtual"|"volatile"|"abstract"|"final"|"import"|"synchronized")
KEYWORD_E   ("transient"|"alignas"|"alignof"|"concept"|"requires"|"decltype"|"constexpr"|"consteval"|"constinit")
KEYWORD_F   ("co_await"|"co_return"|"co_yield"|"static_assert"|"_Static_assert"|"noexcept"|"thread_local"|"enum"{B}+("class"|"struct"))
KEYWORD     {KEYWORD_A}|{KEYWORD_B}|{KEYWORD_C}|{KEYWORD_D}|{KEYWORD_E}|{KEYWORD_F}|{KEYWORD_OBJC}

FLOWKW         ("break"|"catch"|"continue"|"default"|"do"|"else"|"finally"|"return"|"switch"|"throw"|"throws"|"@catch"|"@finally")
FLOWCONDITION  ("case"|"for"|"foreach"|"for each"|"goto"|"if"|"try"|"while"|"@try")

TYPEKW_1  ("bool"|"byte"|"char"|"char8_t"|"char16_t"|"char32_t"|"double"|"float"|"int"|"long")
TYPEKW_2  ("object"|"short"|"signed"|"unsigned"|"void"|"wchar_t"|"size_t"|"boolean"|"id"|"SEL"|"string"|"nullptr")
TYPEKW    {TYPEKW_1}|{TYPEKW_2}

CASTKW    ("const_cast"|"dynamic_cast"|"reinterpret_cast"|"static_cast")
CHARLIT   (("'"\\[0-7]{1,3}"'")|("'"\\."'")|("'"[^' \\\n]{1,4}"'"))
ARITHOP   "+"|"-"|"/"|"*"|"%"|"--"|"++"
ASSIGNOP  "="|"*="|"/="|"%="|"+="|"-="|"<<="|">>="|"&="|"^="|"|="
LOGICOP   "=="|"!="|">"|"<"|">="|"<="|"&&"|"||"|"!"|"<=>"
BITOP     "&"|"|"|"^"|"<<"|">>"|"~"
OPERATOR  {ARITHOP}|{ASSIGNOP}|{LOGICOP}|{BITOP}
RAWBEGIN  (u|U|L|u8)?R\"[^ \t\(\)\\]{0,16}"("
RAWEND    ")"[^ \t\(\)\\]{0,16}\"

%option never-interactive
%option nounistd
%option noyywrap

%x    SkipString
%x    SkipStringS
%x    SkipVerbString
%x    SkipCPP
%x    SkipComment
%x    SkipCxxComment
%x    RemoveSpecialCComment
%x    StripSpecialCComment
%x    Body
%x    FuncCall
%x    MemberCall
%x    MemberCall2
%x    SkipInits
%x    ClassName
%x    AlignAs
%x    AlignAsEnd
%x    PackageName
%x    ClassVar
%x    CppCliTypeModifierFollowup
%x    Bases
%x    SkipSharp
%x    ReadInclude
%x    TemplDecl
%x    TemplCast
%x    CallEnd
%x    ObjCMethod
%x    ObjCParams
%x    ObjCParamType
%x    ObjCCall
%x    ObjCMName
%x    ObjCSkipStr
%x    ObjCCallComment
%x    OldStyleArgs
%x    ConceptName
%x    UsingName
%x    RawString
%x    InlineInit

%%

<*>\x0d
<Body>^([ \t]*"#"[ \t]*("include"|"import")[ \t]*)("<"|"\"") {
      QString text = QString::fromUtf8(yytext);
      startFontClass("preprocessor");
      g_code->codify(text);
      BEGIN( ReadInclude );
   }

<Body>("@interface"|"@implementation"|"@protocol")[ \t\n]+ {
      QString text = QString::fromUtf8(yytext);
      g_insideObjC = true;

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();

      if (! g_insideTemplate)  {
         BEGIN( ClassName );
      }
   }

<Body>(("public"|"private"){B}+)?("ref"|"value"|"interface"|"enum"){B}+("class"|"struct") {
      QString text = QString::fromUtf8(yytext);

      if (g_insideTemplate) {
         REJECT;
      }

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
      BEGIN( ClassName );
   }

<Body>"property"|"event"/{BN}*         {
      QString text = QString::fromUtf8(yytext);

      if (g_insideTemplate) {
         REJECT;
      }

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
   }

<Body>(KEYWORD_CPPCLI_DATATYPE|("partial"{B}+)?"class"|"struct"|"union"|"namespace"|"interface"){B}+ {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();

      if (! g_insideTemplate) {
         BEGIN( ClassName );
      }
   }

<Body>("package")[ \t\n]+        {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();

      BEGIN( PackageName );
   }

<ClassVar>\n            {
      QString text = QString::fromUtf8(yytext);
      if (! g_insideObjC) {
         REJECT;
      }
      codifyLines(text);
      BEGIN(Body);
   }

<Body,ClassVar,Bases>"-"|"+"     {
      QString text = QString::fromUtf8(yytext);

      if (! g_insideObjC || g_insideBody) {
         g_code->codify(text);

      } else {
         // Start of Objective-C method
         g_code->codify(text);
         BEGIN(ObjCMethod);
      }
   }

<ObjCMethod>":"            {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      BEGIN(ObjCParams);
   }

<ObjCParams>"("            {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      BEGIN(ObjCParamType);
   }

<ObjCParams,ObjCMethod>";"|"{"      {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);

      if (text[0] == '{') {

         if (g_searchingForBody) {
            g_searchingForBody = false;
            g_insideBody       = true;
         }

         if (g_insideBody) {
            g_bodyCurlyCount++;
         }

         if (! g_curClassName.isEmpty())  {
            // valid class name

            pushScope(g_curClassName);
            DBG_CTX((stderr, "** scope stack push SCOPEBLOCK\n"));
            g_scopeStack.push(SCOPEBLOCK);
         }
      }

      g_type.resize(0);
      g_name.resize(0);
      BEGIN(Body);
   }

<ObjCParams>{ID}{B}*":"       {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<ObjCParamType>{TYPEKW}       {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keywordtype");
      g_code->codify(text);
      endFontClass();
      g_parmType = text;
   }

<ObjCParamType>{ID}        {
      QString text = QString::fromUtf8(yytext);
      generateClassOrGlobalLink(*g_code, text);
      g_parmType = text;
   }

<ObjCParamType>")"         {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      BEGIN(ObjCParams);
   }

<ObjCParams>{ID}        {
      QString text = QString::fromUtf8(yytext);

      g_code->codify(text);
      g_parmName = text;
      g_theVarContext.addVariable(g_parmType, g_parmName);
      g_parmType.resize(0);
      g_parmName.resize(0);
   }

<ObjCMethod,ObjCParams,ObjCParamType>{ID} {
      QString text = QString::fromUtf8(yytext);
      generateClassOrGlobalLink(*g_code, text);
   }

<ObjCMethod,ObjCParams,ObjCParamType>.    {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<ObjCMethod,ObjCParams,ObjCParamType>\n   {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

<ReadInclude>[^\n\"\>]+/(">"|"\"")     {
      QString text = QString::fromUtf8(yytext);

      bool ambig;
      bool found = false;

      QSharedPointer<FileDef> fd = findFileDef(&Doxy_Globals::inputNameDict, text, ambig);

      if (fd && fd->isLinkable()) {

         if (ambig) {
            // multiple input files match the name
            QString name = QDir::cleanPath(text);

            if (! name.isEmpty() && g_sourceFileDef) {
               QSharedPointer<FileNameList> fn = Doxy_Globals::inputNameDict.find(name);

               if (fn != nullptr) {
                  // for each include name
                  for (const auto &item : *fn) {

                     if (found) {
                        break;
                     }

                     // see if this source file actually includes the given file
                     found = g_sourceFileDef->isIncluded(item->getFilePath());
                  }
               }
            }

         } else {
            // not ambiguous
            found = true;
         }
      }

      if (found) {
         writeMultiLineCodeLink(*g_code, fd, text);
      } else {
         g_code->codify(text);
      }

      char c = yyinput();
      QString t;

      t += c;
      g_code->codify(t);

      endFontClass();
      BEGIN( Body );
   }

<Body,Bases>^[ \t]*"#"        {
      QString text = QString::fromUtf8(yytext);

      startFontClass("preprocessor");
      g_lastSkipCppContext = YY_START;
      g_code->codify(text);

      BEGIN( SkipCPP ) ;
   }

<SkipCPP>\"          {
      QString text = QString::fromUtf8(yytext);

      g_code->codify(text);
      g_lastStringContext = YY_START;

      BEGIN( SkipString );
   }

<SkipCPP>.               {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<SkipCPP>[^\n\/\\\"]+    {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<SkipCPP>\\[\r]?\n       {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

<SkipCPP>"//"/[^/!]      {
      REJECT;     // reconsider

      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<Body,FuncCall>"{"         {
      QString text = QString::fromUtf8(yytext);
      g_theVarContext.pushScope();

      DBG_CTX((stderr,"** scope stack push INNERBLOCK\n"));
      g_scopeStack.push(INNERBLOCK);

      if (g_searchingForBody) {
         g_searchingForBody = false;
         g_insideBody       = true;
      }

      g_code->codify(text);

      if (g_insideBody)  {
         g_bodyCurlyCount++;
      }

      g_type.resize(0);
      g_name.resize(0);
      BEGIN( Body );
   }

<Body,FuncCall,MemberCall,MemberCall2>"}"    {
      QString text = QString::fromUtf8(yytext);

      g_theVarContext.popScope();
      g_type.clear();
      g_name.clear();

      if (! g_scopeStack.isEmpty()) {
         int scope = g_scopeStack.pop();

         DBG_CTX((stderr, "** scope stack pop SCOPEBLOCK = %d\n", scope == SCOPEBLOCK));

         if (scope == SCOPEBLOCK || scope == CLASSBLOCK)  {
            popScope();
         }
      }

      g_code->codify(text);

      DBG_CTX((stderr, "g_bodyCurlyCount = %d\n", g_bodyCurlyCount));

      if (--g_bodyCurlyCount <= 0) {
         g_insideBody       = false;
         g_currentMemberDef = QSharedPointer<MemberDef>();

         if (g_currentDefinition) {
            g_currentDefinition = g_currentDefinition->getOuterScope();
         }
      }
      BEGIN(Body);
   }

<Body,ClassVar>"@end"         {
      QString text = QString::fromUtf8(yytext);

      if (g_sourceFileDef) {
         QSharedPointer<FileDef> fd = g_sourceFileDef;
         g_insideObjC = fd->name().endsWith(".m", Qt::CaseInsensitive) || fd->name().endsWith(".mm", Qt::CaseInsensitive);

      } else {
         g_insideObjC = false;
      }

      if (g_insideBody) {
         g_theVarContext.popScope();

         if (! g_scopeStack.isEmpty()) {
            int scope = g_scopeStack.pop();

            DBG_CTX((stderr, "** scope stack pop SCOPEBLOCK = %d\n", scope == SCOPEBLOCK));

            if (scope == SCOPEBLOCK || scope == CLASSBLOCK) {
               popScope();
            }
         }

         g_insideBody = false;
      }

      startFontClass("keyword");
      g_code->codify(text);
      endFontClass();

      g_currentMemberDef = QSharedPointer<MemberDef>();

      if (g_currentDefinition)  {
         g_currentDefinition = g_currentDefinition->getOuterScope();
      }
      BEGIN(Body);
   }

<ClassName,ClassVar>";"          {
      QString text = QString::fromUtf8(yytext);

      if (g_insideCS) {
         g_code->codify(text);
         g_skipCodify = true;
         unput('{');

      } else {
         g_code->codify(text);
         g_searchingForBody = false;

         BEGIN( Body );
      }
   }

<ClassName,ClassVar>[*&^%]+         {
      QString text = QString::fromUtf8(yytext);

      g_type = g_curClassName;
      g_name.resize(0);
      g_code->codify(text);

      BEGIN(Body); // variable of type struct *
   }

<ClassName>"__declspec"{B}*"("{B}*{ID}{B}*")"   {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      g_code->codify(text);
      endFontClass();
   }

<ClassName>{ID}("."{ID})*     |
<ClassName>{ID}("::"{ID})*            {
      QString text = QString::fromUtf8(yytext);

      if (g_insideCS) {
         g_curClassName = substitute(text, ".", "::");
      } else {
         g_curClassName = text;
      }

      addType();

      if (g_curClassName == "alignas") {
         startFontClass("keyword");
         g_code->codify(text);
         endFontClass();

         BEGIN( AlignAs );

      } else {
         generateClassOrGlobalLink(*g_code, text);
         BEGIN( ClassVar );
      }
   }

<AlignAs>"("                            {
      QString text = QString::fromUtf8(yytext);
      g_bracketCount = 1;
      g_code->codify(text);
      BEGIN( AlignAsEnd );
   }

<AlignAs>\n                             {
      QString text = QString::fromUtf8(yytext);
      ++g_yyLineNr;
      codifyLines(text);
   }

<AlignAs>.                              {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<AlignAsEnd>"("                         {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      ++g_bracketCount;
   }

<AlignAsEnd>")"                  {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);

      --g_bracketCount;

      if (g_bracketCount <= 0) {
         BEGIN(ClassName);
      }
   }

<AlignAsEnd>\n                   {
      QString text = QString::fromUtf8(yytext);
      g_yyLineNr++;
      codifyLines(text);
   }

<AlignAsEnd>.                    {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<ClassName>{ID}("\\"{ID})*       {
      // PHP namespace
      QString text = QString::fromUtf8(yytext);
      g_curClassName = substitute(text, "\\", "::");
      g_scopeStack.push(CLASSBLOCK);
      pushScope(g_curClassName);
      addType();
      generateClassOrGlobalLink(*g_code, text);
      BEGIN( ClassVar );
   }

<ClassName>{ID}{B}*"("{ID}")"           {
      // Obj-C category
      QString text = QString::fromUtf8(yytext);
      g_curClassName = removeRedundantWhiteSpace(text);
      g_scopeStack.push(CLASSBLOCK);
      pushScope(g_curClassName);
      addType();
      generateClassOrGlobalLink(*g_code,  text);
      BEGIN( ClassVar );
   }

<PackageName>{ID}("."{ID})*      {
      QString text = QString::fromUtf8(yytext);
      g_curClassName = substitute(text, ".", "::");
      addType();
      codifyLines(text);
   }

<ClassVar>"="           {
      unput(*yytext);
      BEGIN( Body );
   }

<ClassVar>("extends"|"implements")  {
      QString text = QString::fromUtf8(yytext);

      // for Java
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();

      g_curClassBases.clear();
      BEGIN(Bases);
   }

<ClassVar>("sealed"|"abstract")/{BN}*(":"|"{") {
      QString text = QString::fromUtf8(yytext);

      DBG_CTX((stderr, "***** C++/CLI modifier %s on g_curClassName=%s\n", csPrintable(text),
                  csPrintable(g_curClassName)));

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();

      BEGIN(CppCliTypeModifierFollowup);
   }

<ClassVar>{ID}             {
      QString text = QString::fromUtf8(yytext);
      g_type = g_curClassName;
      g_name = text;

      if (g_insideBody) {
         g_theVarContext.addVariable(g_type, g_name);
      }

      generateClassOrGlobalLink(*g_code, text);
   }

<ClassName,ClassVar,CppCliTypeModifierFollowup>{B}*":"{B}*  {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
      g_curClassBases.clear();
      BEGIN(Bases);
   }

<PackageName>[ \t]*";"        |
<Bases>^{B}*/"@"{ID}          |    // Objective-C interface
<Bases,ClassName,ClassVar,CppCliTypeModifierFollowup>{B}*"{"{B}* {
      QString text = QString::fromUtf8(yytext);

      g_theVarContext.pushScope();

      if (! g_skipCodify) {
         g_code->codify(text);
         g_skipCodify = false;
      }

      if (YY_START == ClassVar && g_curClassName.isEmpty()) {
         g_curClassName = g_name;
      }

      if (g_searchingForBody) {
         g_searchingForBody = false;
         g_insideBody       = true;
      }

      if (g_insideBody) {
         ++g_bodyCurlyCount;
      }

      if (! g_curClassName.isEmpty()) {
         // valid class name

         DBG_CTX((stderr, "** scope stack push CLASSBLOCK\n"));
         g_scopeStack.push(CLASSBLOCK);

         pushScope(g_curClassName);
         DBG_CTX((stderr, "***** g_curClassName=%s\n", csPrintable(g_curClassName)));

         if (getResolvedClass(g_currentDefinition, g_sourceFileDef, g_curClassName) == 0) {

            DBG_CTX((stderr, "Adding new class %s\n", csPrintable(g_curClassName)));

            QSharedPointer<ClassDef> ncd = QMakeShared<ClassDef>("<code>", 1, 1, g_curClassName, CompoundType::Class,
                  QString(), QString(), false);

            g_codeClassSDict.insert(g_curClassName, ncd);

            // insert base classes
            for (const auto &str : g_curClassBases) {
               QSharedPointer<ClassDef> bcd;
               bcd = g_codeClassSDict.find(str);

               if (! bcd) {
                  bcd = getResolvedClass(g_currentDefinition, g_sourceFileDef, str);
               }

               if (bcd && bcd != ncd) {
                  ncd->insertBaseClass(bcd, str, Public, Normal);
               }

            }
         }

      } else {
         // not a class name -> assume inner block

         DBG_CTX((stderr,"** scope stack push INNERBLOCK\n"));
         g_scopeStack.push(INNERBLOCK);
      }

      g_curClassName.resize(0);
      g_curClassBases.clear();
      BEGIN(Body);
   }

<Bases>"virtual"|"public"|"protected"|"private"|"@public"|"@private"|"@protected" {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      g_code->codify(text);
      endFontClass();
   }

<Bases>{SEP}?({ID}{SEP})*{ID}   {
      QString text = QString::fromUtf8(yytext);

      DBG_CTX((stderr, "%s:addBase(%s)\n", csPrintable(g_curClassName), csPrintable(text) ));

      g_curClassBases.append(text);
      generateClassOrGlobalLink(*g_code, text);
   }

<Bases>"<"                      {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);

      if (! g_insideObjC) {
         g_sharpCount = 1;
         BEGIN (SkipSharp);

      } else {
         g_insideProtocolList = true;
      }
   }

<Bases>">"           {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      g_insideProtocolList = false;
   }

<SkipSharp>"<"       {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      ++g_sharpCount;
   }

<SkipSharp>">"       {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);

      if (--g_sharpCount <= 0) {
         BEGIN (Bases);
      }
   }

<SkipSharp>"\""      {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      g_lastStringContext = YY_START;

      BEGIN(SkipString);
   }

<SkipSharp>"\'"      {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      g_lastStringContext = YY_START;

      BEGIN(SkipStringS);
   }

<Bases>"("           {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      g_sharpCount = 1;
      BEGIN (SkipSharp);
   }

<SkipSharp>"("   {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      ++g_sharpCount;
   }

<SkipSharp>")"   {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);

      if (--g_sharpCount <= 0) {
         BEGIN (Bases);
      }
   }

<Bases>","       {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<Body>{SCOPEPREFIX}?"operator"{B}*"()"{B}*/"(" {
      QString text = QString::fromUtf8(yytext);

      addType();
      generateFunctionLink(*g_code, text);
      g_bracketCount = 0;
      g_args.resize(0);
      g_name += text;

      BEGIN( FuncCall );
   }

<Body>{SCOPEPREFIX}?"operator"/"("      {
      QString text = QString::fromUtf8(yytext);

      addType();
      generateFunctionLink(*g_code, text);
      g_bracketCount = 0;
      g_args.resize(0);
      g_name += text;

      BEGIN(FuncCall);
   }

<Body>{SCOPEPREFIX}?"operator"[^a-z_A-Z0-9\(\n]+/"(" {
      QString text = QString::fromUtf8(yytext);

      addType();
      generateFunctionLink(*g_code, text);
      g_bracketCount = 0;
      g_args.resize(0);
      g_name += text;
      BEGIN(FuncCall);
   }

<Body,TemplDecl>("template"|"generic")/([^a-zA-Z0-9])       {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();

      g_insideTemplate = true;
      g_sharpCount = 0;
   }

<Body>"concept"{BN}+                {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();

      BEGIN(ConceptName);
   }

<Body>"using"{BN}+"namespace"{BN}+  {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();

      BEGIN(UsingName);
   }

<Body>"using"{BN}+            {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();

      BEGIN(UsingName);
   }

<ConceptName>{ID}("::"{ID})*   {
      QString text = QString::fromUtf8(yytext);
      generateClassOrGlobalLink(*g_code, text);
   }

<ConceptName>"="        {
      QString text = QString::fromUtf8(yytext);

      codifyLines(text);
      BEGIN(Body);
   }

<UsingName>{ID}("::"{ID})*       {
      QString text = QString::fromUtf8(yytext);

      addUsingDirective(substitute(text, ".", "::"));
      generateClassOrGlobalLink(*g_code, text);

      g_scopeStack.push(CLASSBLOCK);
      pushScope(text);

      BEGIN(Body);
   }

<UsingName>\n           {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
      BEGIN(Body);
   }

<UsingName>.            {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
      BEGIN(Body);
   }

<Body,FuncCall>"$"?"this"("->"|".")    {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);                      // "this->" for C++ "this." for C#
      s_prefix_this = true;
   }

<Body>{KEYWORD}/([^a-z_A-Z0-9])     {
      QString text = QString::fromUtf8(yytext);

      if (g_insideJava && (text == "internal")) {
         // special case
         REJECT;
      }

      if (skipLanguageKeywords(text)) {
         REJECT;
      }

      startFontClass("keyword");
      codifyLines(text);

      if (text == "typedef") {
          addType();
          g_name += text;
      }

      endFontClass();
   }

<Body>{KEYWORD}/{B}*          {
      QString text = QString::fromUtf8(yytext);

      if (skipLanguageKeywords(text)) {
         REJECT;
      }

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
   }

<Body>{KEYWORD}/{BN}*"("      {
      QString text = QString::fromUtf8(yytext);

      if (skipLanguageKeywords(text)) {
         REJECT;
      }

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();

      g_name.resize(0);
      g_type.resize(0);
   }

<FuncCall>"in"/{BN}*          {
      QString text = QString::fromUtf8(yytext);
      if (! g_inForEachExpression) {
         REJECT;
      }

      startFontClass("keywordflow");
      codifyLines(text);
      endFontClass();

      // insert the variable in the parent scope
      g_theVarContext.popScope();
      g_theVarContext.addVariable(g_parmType, g_parmName);
      g_theVarContext.pushScope();
      g_name.resize(0);
      g_type.resize(0);
   }

<Body>{FLOWKW}/{BN}*"("          {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keywordflow");
      codifyLines(text);
      endFontClass();
      g_name.resize(0);
      g_type.resize(0);

      g_inForEachExpression = (text == "for each") || (text == "foreach");
      BEGIN(FuncCall);
   }

<Body>{FLOWCONDITION}/{BN}*"("          {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keywordflow");
      codifyLines(text);
      endFontClass();

      g_name.resize(0);
      g_type.resize(0);

      if (text == "for each" || text == "foreach") {
         g_inForEachExpression = true;
      } else {
         g_inForEachExpression = false;
      }

      BEGIN(FuncCall);
   }

<Body>{FLOWKW}/([^a-z_A-Z0-9])      {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keywordflow");
      codifyLines(text);
      endFontClass();

      if (g_inFunctionTryBlock && ((text == "catch") || (text == "finally")) ) {
         g_inFunctionTryBlock = false;
      }
   }

<Body>{FLOWCONDITION}/([^a-z_A-Z0-9]) 	{
      QString text = QString::fromUtf8(yytext);

      startFontClass("keywordflow");
      codifyLines(text);
      endFontClass();

      if (g_inFunctionTryBlock && (text == "catch" || text == "finally")) {
         g_inFunctionTryBlock = false;
      }
   }

<Body>{FLOWKW}/{B}*           {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keywordflow");
      codifyLines(text);
      endFontClass();
   }


<Body>{FLOWCONDITION}/{B}*              {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keywordflow");
      codifyLines(text);
      endFontClass();
   }

<Body>"*"{B}*")"              {
      // end of cast?
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      popCallContext();
      --g_bracketCount;
      g_parmType = g_name;

      BEGIN(FuncCall);
   }

<Body>"\\)"|"\\("    {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<Body>[\\|\)\+\-\/\%\~\!]     {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      g_name.resize(0);
      g_type.resize(0);

      if (text[0] == ')') {
         popCallContext();
         --g_bracketCount;
         BEGIN(FuncCall);
      }
   }

<Body,TemplDecl,ObjCMethod>{TYPEKW}/{B}* {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keywordtype");
      g_code->codify(text);
      endFontClass();
      addType();
      g_name += yytext;
   }

<Body>"generic"/{B}*"<"[^\n\/\-\.\{\"\>]*">"{B}* {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      g_code->codify(text);
      endFontClass();

      g_sharpCount = 0;
      BEGIN(TemplDecl);
   }

<Body>"template"/{B}*"<"[^\n\/\-\.\{\"\>]*">"{B}* {
      // template<...>
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      g_code->codify(text);
      endFontClass();

      g_sharpCount = 0;
      BEGIN(TemplDecl);
   }

<TemplDecl>"class"|"typename"       {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
   }

<TemplDecl>"<"             {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      g_sharpCount++;
   }

<TemplDecl>">"             {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      g_sharpCount--;

      if (g_sharpCount <= 0) {
         BEGIN(Body);
      }
   }

<TemplCast>">"             {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
      BEGIN( g_lastTemplCastContext );
   }

<TemplCast>{ID}("::"{ID})*       {
      QString text = QString::fromUtf8(yytext);
      generateClassOrGlobalLink(*g_code, text);
   }

<TemplCast>("const"|"volatile"){B}*    {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
   }

<TemplCast>[*^]*        {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

<Body,MemberCall2,FuncCall>{CASTKW}{B}*"<" {
      // static_cast<T>(
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      codifyLines(text);
      endFontClass();

      g_lastTemplCastContext = YY_START;
      BEGIN(TemplCast);
   }

<Body>"$this->"{SCOPENAME}/{BN}*[;,)\]] {
      // PHP member variable
      QString text = QString::fromUtf8(yytext);
      addType();
      generatePHPVariableLink(*g_code, text);
      g_name += text.mid(7);
   }

<Body,TemplCast>{SCOPENAME}{B}*"<"[^\n\/\-\.\{\"\>\(]*">"("::"{ID})*/{B}* {
      // A<T> *pt;
      QString text = QString::fromUtf8(yytext);



      if (isCastKeyword(text) && YY_START == Body) {
         REJECT;
      }

      addType();
      generateClassOrGlobalLink(*g_code, text);
      g_name += text;
   }

<Body>{SCOPENAME}/{BN}*[:;,)\]]      {
      // "int var;" or "var, var2" or "debug(f) macro" , or int var : 5;
      QString text = QString::fromUtf8(yytext);

      if (text.startsWith("typedef")) {
         REJECT;
      }

      addType();
      generateFunctionLink(*g_code, text);
      g_name += text;
   }

<Body>{SCOPENAME}/{B}*  {
      // p->func()
      QString text = QString::fromUtf8(yytext);

      if (text.startsWith("typedef")) {
         REJECT;
      }

      addType();
      generateClassOrGlobalLink(*g_code, text);
      g_name += yytext;
   }

<Body>"("{B}*("*"{B}*)+{SCOPENAME}+{B}*")"/{B}*    {
      // (*p)->func() but not "if (p) ..."
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);

      int s = 0;

      while (s < text.length() && ! isId(text[s])) {
         ++s;
      }

      int e = text.length() - 1;

      while (e > 0 && ! isId(text[e])) {
         --e;
      }

      QString varname = text.mid(s, e - s + 1);

      addType();
      g_name = varname;
   }

<Body>{SCOPETNAME}{B}*"<"[^\n\/\-\.\{\"\>]*">"/{BN}*"(" |
<Body>{SCOPETNAME}/{BN}*"("      {
      // a() or c::a() or t<A,B>::a() or A\B\foo()

      QString text = QString::fromUtf8(yytext);
      if (isCastKeyword(text)) {
         REJECT;
      }

      addType();
      generateFunctionLink(*g_code, text);

      g_bracketCount = 0;
      g_args.resize(0);
      g_name += text;
      BEGIN( FuncCall );
   }

<FuncCall,Body,MemberCall,MemberCall2,SkipInits,InlineInit>{RAWBEGIN}   {
      QString text = QString::fromUtf8(yytext);

      int i = text.indexOf('R');
      g_code->codify(text.left(i + 1));
      startFontClass("stringliteral");
      g_code->codify(text.mid(i + 1));

      g_lastStringContext = YY_START;
      g_inForEachExpression = false;
      g_delimiter = text.mid(i + 2);
      g_delimiter = g_delimiter.left(g_delimiter.length() - 1);

      BEGIN( RawString );
   }

<FuncCall,Body,MemberCall,MemberCall2,SkipInits,InlineInit>\"  {
      QString text = QString::fromUtf8(yytext);
      startFontClass("stringliteral");
      g_code->codify(text);
      g_lastStringContext   = YY_START;
      g_inForEachExpression = false;
      BEGIN( SkipString );
   }

<FuncCall,Body,MemberCall,MemberCall2,SkipInits,InlineInit>\'  {
      QString text = QString::fromUtf8(yytext);
      startFontClass("stringliteral");
      g_code->codify(text);
      g_lastStringContext   = YY_START;
      g_inForEachExpression = false;
      BEGIN( SkipStringS );
   }

<SkipString>[^\"\\\r\n]*      {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<SkipStringS>[^\'\\\r\n]*     {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<SkipString,SkipStringS>"//"|"/*"   {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<SkipString>@?\"        {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      g_code->codify(text);
      endFontClass();
      BEGIN( g_lastStringContext );
   }

<SkipStringS>\'            {
      QString text = QString::fromUtf8(yytext);

      g_code->codify(text);
      endFontClass();
      BEGIN( g_lastStringContext );
   }

<SkipString,SkipStringS>\\.      {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<RawString>{RAWEND}                     {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);

      QString delimiter = text.mid(1);
      delimiter = delimiter.left(delimiter.length() - 1);

      if (delimiter == g_delimiter)
      {
         BEGIN( g_lastStringContext );
      }
   }

<RawString>[^)\n]+               {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<RawString>.                     {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<RawString>\n                    {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

<SkipVerbString>[^"\n]+          {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<SkipVerbString>\"\"             {
      // escaped quote
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<SkipVerbString>\"         {
      // end of string
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      endFontClass();
      BEGIN( g_lastVerbStringContext );
   }

<SkipVerbString>.          {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<SkipVerbString>\n         {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

<Body>":"            {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      g_name.resize(0);
      g_type.resize(0);
   }

<Body>"<"            {
      QString text = QString::fromUtf8(yytext);

      if (g_insideTemplate) {
         g_sharpCount++;
      }
      g_code->codify(text);
   }

<Body>">"            {
      QString text = QString::fromUtf8(yytext);

      if (g_insideTemplate) {
         if (--g_sharpCount <= 0) {
            g_insideTemplate = false;
         }
      }
      g_code->codify(text);
   }

<Body,MemberCall,MemberCall2,FuncCall>"'"((\\0[Xx0-9]+)|(\\.)|(.))"'"   {
      QString text = QString::fromUtf8(yytext);
      startFontClass("charliteral");
      g_code->codify(text);
      endFontClass();
   }

<Body>"."|"->"             {
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '-') {
         // -> could be overloaded
         updateCallContextForSmartPointer();
      }

      g_code->codify(text);
      g_memCallContext = YY_START;
      BEGIN( MemberCall );
   }

<MemberCall>{SCOPETNAME}/{BN}*"("   {
      QString text = QString::fromUtf8(yytext);

      if (getScopeCallContext()) {
            if (! generateClassMemberLink(*g_code, getScopeCallContext(), text)) {
               g_code->codify(text);
               addToSearchIndex(text);
            }
            g_name.clear();

         } else {
            g_code->codify(text);
            addToSearchIndex(text);
            g_name.clear();
         }

         g_type.clear();
         g_bracketCount = 0;        // required for member links

         if (g_memCallContext == Body) {
            BEGIN(FuncCall);

         } else {
            BEGIN(g_memCallContext);
         }
   }

<MemberCall>{SCOPENAME}/{B}*     {
      QString text = QString::fromUtf8(yytext);

      if (getScopeCallContext()) {
         DBG_CTX((stderr, "g_theCallContext.getClass()=%p\n", getScopeCallContext()));

         if (! generateClassMemberLink(*g_code, getScopeCallContext(), text)) {
            g_code->codify(text);
            addToSearchIndex(text);
         }

         g_name.resize(0);

      } else {
         DBG_CTX((stderr, "No class context\n"));
         g_code->codify(text);
         addToSearchIndex(text);
         g_name.resize(0);
      }

      g_type.resize(0);
      BEGIN(g_memCallContext);
   }

<Body>[,=;\[]           {
      QString text = QString::fromUtf8(yytext);

      if (g_insideObjC && text[0] == '[')  {
         // printf("Found start of ObjC call\n");
         // start of a method call

         g_ObjC_contextDict.clear();
         g_nameDict.clear();
         g_objectDict.clear();
         g_wordDict.clear();
         g_commentDict.clear();

         g_currentCtxId    = 0;
         g_currentNameId   = 0;
         g_currentObjId    = 0;
         g_ObjC_currentCtx = 0;
         g_braceCount      = 0;

         unput('[');
         BEGIN(ObjCCall);

      } else {
         g_code->codify(text);
         g_saveName = g_name;
         g_saveType = g_type;

         if (text[0] != '[' && ! g_type.isEmpty())  {
            g_theVarContext.addVariable(g_type, g_name);
         }

         if (text[0] == ';' || text[0] == '=') {
            g_type.resize(0);
            g_name.resize(0);

         } else if (text[0] == '[') {
            pushCallContext();

         }

         g_args.resize(0);
         g_parmType.resize(0);
         g_parmName.resize(0);
      }
   }

  /*
<ObjCMemberCall>{ID}          {
      QString text = QString::fromUtf8(yytext);

      if (qstrcmp(yytext,"self") == 0 || qstrcmp(yytext,"super") == 0) {
         // TODO: get proper base class for "super"
         g_theCallContext.setClass(getClass(g_curClassName));
         startFontClass("keyword");
         g_code->codify(text);
         endFontClass();

      } else {
         generateClassOrGlobalLink(*g_code, text);
      }

      g_name.resize(0);
      BEGIN(ObjCMemberCall2);
   }

<ObjCMemberCall>"["        {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      pushCallContext();
   }

<ObjCMemberCall2>{ID}":"?     {
      QString text = QString::fromUtf8(yytext);

      g_name += text;
      if (g_theCallContext.getClass()) {
         //printf("Calling method %s\n", csPrintable(g_name));

         if (! generateClassMemberLink(*g_code, g_theCallContext. getClass(), g_name)) {
            g_code->codify(text);
            addToSearchIndex(g_name);
         }

      } else {
         g_code->codify(text);
         addToSearchIndex(g_name);
      }

      g_name.resize(0);
      BEGIN(ObjCMemberCall3);
   }

<ObjCMemberCall2,ObjCMemberCall3>"]"   {
      QString text = QString::fromUtf8(yytext);
      popCallContext();
      g_code->codify(text);
      BEGIN(Body);
   }

  */

<ObjCCall,ObjCMName>"["|"{"       {
      QString text = QString::fromUtf8(yytext);
      saveObjCContext();
      g_ObjC_currentCtx->format += text[0];

      BEGIN(ObjCCall);
   }

<ObjCCall,ObjCMName>"]"|"}"       {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->format += text[0];
      restoreObjCContext();
      BEGIN(ObjCMName);

      if (g_ObjC_currentCtx == nullptr) {
         // end of call
         writeObjCMethodCall(g_ObjC_contextDict.value(0));
         BEGIN(Body);
      }
   }

<ObjCCall,ObjCMName>"//".*      {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->format += escapeComment(text);
   }

<ObjCCall,ObjCMName>"/*"        {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);
      g_lastObjCCallContext = YY_START;
      g_ObjC_currentCtx->comment = text;
      BEGIN(ObjCCallComment);
   }

<ObjCCallComment>"*/"           {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->comment += text;
      g_ObjC_currentCtx->format  += escapeComment(g_ObjC_currentCtx->comment);
      BEGIN(g_lastObjCCallContext);
   }

<ObjCCallComment>[^*\n]+          {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->comment += text;
   }

<ObjCCallComment>"//"|"/*"        {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->comment += text;
   }

<ObjCCallComment>\n               {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->comment += text[0];
   }

<ObjCCallComment>.                {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->comment += text[0];
   }

<ObjCCall>{ID}                      {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->format += escapeObject(text);

      if (g_braceCount == 0) {
         g_ObjC_currentCtx->objectTypeOrName = text;
         BEGIN(ObjCMName);
      }
   }

<ObjCMName>{ID}/{BN}*"]"          {
      QString text = QString::fromUtf8(yytext);

      if (g_braceCount == 0 && g_ObjC_currentCtx->methodName.isEmpty()) {
         g_ObjC_currentCtx->methodName = text;
         g_ObjC_currentCtx->format += escapeName(text);

      } else {
         g_ObjC_currentCtx->format += escapeWord(text);
      }
   }

<ObjCMName>{ID}/{BN}*":"           {
      QString text = QString::fromUtf8(yytext);

      if (g_braceCount == 0) {
         g_ObjC_currentCtx->methodName += text;
         g_ObjC_currentCtx->methodName += ":";
      }

      g_ObjC_currentCtx->format += escapeName(text);
   }

<ObjCSkipStr>[^\n\"$\\]*           {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->format += text;
   }

<ObjCSkipStr>\\.             {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->format += text;
   }

<ObjCSkipStr>"\""               {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->format += text;
      BEGIN(g_lastStringContext);
   }

<ObjCCall,ObjCMName>{CHARLIT}      {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->format += text;
   }

<ObjCCall,ObjCMName>"@"?"\""       {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->format += text;
      g_lastStringContext        = YY_START;
      BEGIN(ObjCSkipStr);
   }

<ObjCCall,ObjCMName,ObjCSkipStr>"$" {
      g_ObjC_currentCtx->format += "$$";
   }

<ObjCCall,ObjCMName>"("            {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->format += text[0];
      g_braceCount++;
   }

<ObjCCall,ObjCMName>")"            {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->format += text[0];
      g_braceCount--;
   }

<ObjCSkipStr>"@"/"\""         {
      // needed to prevent matching the global rule (for C#)
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->format += text;
   }

<ObjCCall,ObjCMName,ObjCSkipStr>{ID} {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->format += escapeWord(text);
   }

<ObjCCall,ObjCMName,ObjCSkipStr>.  {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->format += text[0];
   }

<ObjCCall,ObjCMName,ObjCSkipStr>\n {
      QString text = QString::fromUtf8(yytext);
      g_ObjC_currentCtx->format += text[0];
   }

<Body>"]"            {
      QString text = QString::fromUtf8(yytext);

      popCallContext();
      g_code->codify(text);

      // TODO: nested arrays like: a[b[0]->func()]->func()

      g_name = g_saveName;
      g_type = g_saveType;
   }

<Body>[0-9]+            {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<Body>[0-9]+[xX][0-9A-Fa-f]+     {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<MemberCall2,FuncCall>{KEYWORD}/([^a-z_A-Z0-9]) {
      QString text = QString::fromUtf8(yytext);

      if (skipLanguageKeywords(text)) {
         REJECT;      }


      startFontClass("keyword");
      g_code->codify(text);
      endFontClass();
    }

<MemberCall2,FuncCall,OldStyleArgs,TemplCast>{TYPEKW}/([^a-z_A-Z0-9]) {
      QString text = QString::fromUtf8(yytext);
      addParmType();
      g_parmName = text;

      startFontClass("keywordtype");
      g_code->codify(text);
      endFontClass();
   }

<MemberCall2,FuncCall>{FLOWKW}/([^a-z_A-Z0-9]) {
      QString text = QString::fromUtf8(yytext);
      addParmType();
      g_parmName = text;
      startFontClass("keywordflow");
      g_code->codify(text);
      endFontClass();
   }

<MemberCall2,FuncCall>{FLOWCONDITION}/([^a-z_A-Z0-9]) {
      QString text = QString::fromUtf8(yytext);

      addParmType();
      g_parmName = text;
      startFontClass("keywordflow");
      g_code->codify(text);
      endFontClass();
   }

<MemberCall2,FuncCall>{ID}(({B}*"<"[^\n\[\](){}<>]*">")?({B}*"::"{B}*{ID})?)* {
      QString text = QString::fromUtf8(yytext);

      if (isCastKeyword(text)) {
         REJECT;
      }

      addParmType();
      g_parmName = text;
      generateClassOrGlobalLink(*g_code, text, ! g_insideBody);
   }

<FuncCall>";"           {
      // probably a cast, not a function call
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      g_inForEachExpression = false;
      BEGIN( Body );
   }

<MemberCall2,FuncCall>,          {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      g_theVarContext.addVariable(g_parmType, g_parmName);
      g_parmType.resize(0);
      g_parmName.resize(0);
   }

<MemberCall2,FuncCall>"{"     {
      QString text = QString::fromUtf8(yytext);

      if (g_bracketCount > 0) {
         g_code->codify(text);
         g_skipInlineInitContext = YY_START;
         g_curlyCount = 0;
         BEGIN(InlineInit);

      } else {
         REJECT;
      }
   }

<InlineInit>"{"               {
      QString text = QString::fromUtf8(yytext);
      g_curlyCount++;
      g_code->codify(text);
   }

<InlineInit>"}"               {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);

      if (--g_curlyCount <= 0) {
         BEGIN(g_skipInlineInitContext);
      }
   }

<InlineInit>\n                {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

<InlineInit>.                 {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<MemberCall2,FuncCall>"("     {
      QString text = QString::fromUtf8(yytext);
      g_parmType.resize(0);
      g_parmName.resize(0);
      g_code->codify(text);
      g_bracketCount++;
      pushCallContext();

      if (YY_START == FuncCall && ! g_insideBody) {
         g_theVarContext.pushScope();
      }
   }

<MemberCall2,FuncCall>{OPERATOR}        {
      // operator
      QString text = QString::fromUtf8(yytext);

      if ((text != "*") && (text != "&") && (text != "^") && (text != "%")) {
         // typically a pointer or reference

        // not a * or &, or C++/CLI's ^ or %
         g_parmType.resize(0);
         g_parmName.resize(0);
      }

      g_code->codify(text);
   }

<MemberCall,MemberCall2,FuncCall>("*"{B}*)?")"  {
      QString text = QString::fromUtf8(yytext);

      if (text[0] == ')') {
         // no a pointer cast

         if (g_parmType.isEmpty()) {
            g_parmType = g_parmName;
            g_parmName.resize(0);
         }

      } else {
         g_parmType = g_parmName;
         g_parmName.resize(0);
      }

      g_theVarContext.addVariable(g_parmType, g_parmName);

      popCallContext();
      g_inForEachExpression = false;

      // g_theCallContext.setClass(0);
      // commented out, otherwise a()->b() does not work for b().

      g_code->codify(text);
      --g_bracketCount;

      if (g_bracketCount <= 0)  {
         if (g_name.isEmpty()) {
            BEGIN( Body );
         } else {
            BEGIN( CallEnd );
         }
      }
   }

<CallEnd>[ \t\n]*          {
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

  /*
<MemberCall2,FuncCall>")"[ \t\n]*[;:]  {
  */

<CallEnd>[;:]           {
      QString text = QString::fromUtf8(yytext);

      codifyLines(text);
      g_bracketCount = 0;

      if (text[0] == ';') {
         g_searchingForBody = false;
      }

      if (! g_type.isEmpty()) {
         DBG_CTX((stderr, "Add variable g_type = %s g_name = %s)\n", csPrintable(g_type), csPrintable(g_name)));
         g_theVarContext.addVariable(g_type, g_name);

      }

      g_parmType.resize(0);
      g_parmName.resize(0);
      setScopeCallContext(QSharedPointer<Definition>());

      if (text[0] == ';' || g_insideBody) {
         if (! g_insideBody) {
            g_theVarContext.popScope();
         }

         g_name.resize(0);
         g_type.resize(0);
         BEGIN( Body );

      } else {
         g_bracketCount = 0;
         BEGIN( SkipInits );
      }
   }

<CallEnd>("const"|"volatile"|"sealed"|"override")({BN}+("const"|"volatile"|"sealed"|"override"))*/{BN}*(";"|"="|"throw"{BN}*"(") {
      QString text = QString::fromUtf8(yytext);
      startFontClass("keyword");
      codifyLines(text);
      endFontClass();
   }

<CallEnd,OldStyleArgs>("const"|"volatile"|"sealed"|"override")*({BN}+("const"|"volatile"|"sealed"|"override"))*{BN}*"{" {
      QString text = QString::fromUtf8(yytext);

      if (g_insideBody) {
         g_theVarContext.pushScope();
      }

      g_theVarContext.addVariable(g_parmType, g_parmName);
      // popCallContext();
      g_parmType.resize(0);
      g_parmName.resize(0);

      int index = g_name.lastIndexOf("::");

      DBG_CTX((stderr, "g_name = %s\n", csPrintable(g_name)));

      if (index != -1)  {
         QString scope = g_name.left(index);

         if (! g_classScope.isEmpty()) {
            scope.prepend(g_classScope + "::");
         }

         QSharedPointer<ClassDef> cd = getResolvedClass(Doxy_Globals::globalScope, g_sourceFileDef, scope);

         if (cd) {
            setClassScope(cd->name());
            g_scopeStack.push(SCOPEBLOCK);
            DBG_CTX((stderr,"** scope stack push SCOPEBLOCK\n"));

         } else  {
            // setClassScope(g_realScope);
            g_scopeStack.push(INNERBLOCK);
            DBG_CTX((stderr,"** scope stack push INNERBLOCK\n"));
         }

      } else {
         DBG_CTX((stderr,"** scope stack push INNERBLOCK\n"));
         g_scopeStack.push(INNERBLOCK);
      }

      text.truncate(text.length() - 1);

      if (! text.trimmed().isEmpty()) {
         startFontClass("keyword");
         codifyLines(text);
         endFontClass();

      } else {
         // just whitespace
         codifyLines(text);
      }

      g_code->codify("{");

      if (g_searchingForBody) {
         g_searchingForBody = false;
         g_insideBody       = true;
      }

      if (g_insideBody) {
         g_bodyCurlyCount++;
      }

      g_type.resize(0);
      g_name.resize(0);
      BEGIN( Body );
   }

<CallEnd>"try"             {
      // function-try-block
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      g_code->codify(text);
      endFontClass();
      g_inFunctionTryBlock = true;
   }

<CallEnd>"requires"        {
      QString text = QString::fromUtf8(yytext);

      startFontClass("keyword");
      g_code->codify(text);
      endFontClass();
   }

<CallEnd>{ID}           {
      QString text = QString::fromUtf8(yytext);

      if (g_insideBody || ! g_parmType.isEmpty())  {
         REJECT;
      }

      // could be K&R style definition
      addParmType();
      g_parmName = yytext;
      generateClassOrGlobalLink(*g_code, text, ! g_insideBody);
      BEGIN(OldStyleArgs);
   }

<OldStyleArgs>{ID}         {
      QString text = QString::fromUtf8(yytext);
      addParmType();
      g_parmName = text;
      generateClassOrGlobalLink(*g_code, text, ! g_insideBody);
   }

<OldStyleArgs>[,;]         {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      g_theVarContext.addVariable(g_parmType, g_parmName);

      if (text[0] == ';') {
         g_parmType.resize(0);
      }
      g_parmName.resize(0);
   }

<CallEnd,OldStyleArgs>"#"     {
      QString text = QString::fromUtf8(yytext);
      startFontClass("preprocessor");
      g_lastSkipCppContext = Body;
      g_code->codify(text);
      BEGIN( SkipCPP );
   }

<CallEnd>.           {
      unput(*yytext);

      if (! g_insideBody) {
         g_theVarContext.popScope();
      }

      g_name.resize(0);
      g_args.resize(0);
      g_parmType.resize(0);
      g_parmName.resize(0);
      BEGIN( Body );
   }

<SkipInits>";"             {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      g_type.resize(0);
      g_name.resize(0);
      BEGIN( Body );
   }

<SkipInits>"{"             {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);

      if (g_searchingForBody) {
         g_searchingForBody = false;
         g_insideBody       = true;
      }

      if (g_insideBody) {
         g_bodyCurlyCount++;
      }

      if (g_name.indexOf("::") != -1) {
         DBG_CTX((stderr,"** scope stack push SCOPEBLOCK\n"));
         g_scopeStack.push(SCOPEBLOCK);
         setClassScope(g_realScope);

      } else {
         DBG_CTX((stderr,"** scope stack push INNERBLOCK\n"));
         g_scopeStack.push(INNERBLOCK);
      }

      g_type.resize(0);
      g_name.resize(0);
      BEGIN( Body );
   }

<SkipInits>{ID}{B}*"{"  {
      QString text = QString::fromUtf8(yytext);

      int bracketPos = text.indexOf('{');
      int spacePos   = text.indexOf(' ');

      int len = spacePos == -1 ? bracketPos : spacePos;
      generateClassOrGlobalLink(*g_code, text.leftView(len));
      g_code->codify(text.midView(len));
   }

<SkipInits>{ID}            {
      QString text = QString::fromUtf8(yytext);
      generateClassOrGlobalLink(*g_code, text);
   }

<FuncCall>{ID}/"("         {
      QString text = QString::fromUtf8(yytext);
      generateFunctionLink(*g_code, text);
   }

<FuncCall>{ID}/("."|"->")  {
      QString text = QString::fromUtf8(yytext);
      g_name = text;
      generateClassOrGlobalLink(*g_code, text);
      BEGIN( MemberCall2 );
   }

<FuncCall,MemberCall2>("("{B}*("*"{B}*)+{ID}+{B}*")"{B}*)/("."|"->") {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);

      int s = 0;

      while (! isId(text[s])) {
         ++s;
      }

      int e = text.length() - 1;

     while (e > 0 && ! isId(text[e])) {
         --e;
      }

      g_name = text.mid(s, e - s + 1);
      BEGIN( MemberCall2 );
   }

<MemberCall2>{ID}/([ \t\n]*"(")         {
      QString text = QString::fromUtf8(yytext);

      if (! g_args.isEmpty())  {
         generateMemberLink(*g_code,g_args, text);
      } else {
         generateClassOrGlobalLink(*g_code, text);
      }

      g_args.resize(0);
      BEGIN( FuncCall );
   }

<MemberCall2>{ID}/([ \t\n]*("."|"->"))  {
      // g_code->codify(text);

      QString text = QString::fromUtf8(yytext);
      g_name = text;
      generateClassOrGlobalLink(*g_code, text);
      BEGIN( MemberCall2 );
   }

<MemberCall2>"->"|"."         {
      QString text = QString::fromUtf8(yytext);

      if (yytext[0]=='-') {
         // -> could be overloaded
         updateCallContextForSmartPointer();
      }

      g_code->codify(text);
      g_memCallContext = YY_START;
      BEGIN( MemberCall );
   }

<SkipComment>"/*"("!"?)"*/"      {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      endFontClass();
      BEGIN( g_lastCContext ) ;
   }

<SkipComment>"//"|"/*"        {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<SkipComment>[^*/\n]+         {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<SkipComment>[ \t]*"*/"          {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      endFontClass();

      if (g_lastCContext == SkipCPP) {
         startFontClass("preprocessor");
      }
      BEGIN( g_lastCContext );
   }

<SkipCxxComment>[^\r\n]*"\\"[\r]?\n    {
      // line continuation
      QString text = QString::fromUtf8(yytext);
      codifyLines(text);
   }

<SkipCxxComment>[^\r\n]+   {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<SkipCxxComment>\r
<SkipCxxComment>\n         {
      unput('\n');
      endFontClass();
      BEGIN( g_lastCContext );
   }

<SkipCxxComment>.          {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
   }

<RemoveSpecialCComment>"*/"{B}*\n({B}*\n)*({B}*(("//@"[{}])|("/*@"[{}]"*/")){B}*\n)?{B}*"/*"[*!]/[^/*] {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);
      g_yyLineNr += text.count('\n');
   }

<RemoveSpecialCComment>"*/"{B}*\n({B}*\n)*({B}*(("//@"[{}])|("/*@"[{}]"*/")){B}*\n)? {
      QString text = QString::fromUtf8(yytext);
      g_yyLineNr += text.count('\n');

      if (g_lastSpecialCContext == SkipCxxComment) {
         // force end of C++ comment here
         nextCodeLine();
         endFontClass();
         BEGIN( g_lastCContext );
      } else {

         if (text.endsWith('\n'))  {
            --g_yyLineNr;
            unput('\n');

         } else {
            nextCodeLine();
         }

         BEGIN(g_lastSpecialCContext);
      }
   }

<RemoveSpecialCComment>"*/"      {
      BEGIN(g_lastSpecialCContext);
   }

<RemoveSpecialCComment>[^*\n]+   {

   }

<RemoveSpecialCComment>"//"|"/*" {

   }

<RemoveSpecialCComment>\n  {
      // */ (editor syntax fix)
      ++g_yyLineNr;
   }

<RemoveSpecialCComment>.
<MemberCall>[^a-z_A-Z0-9(\n]     {
      QString text = QString::fromUtf8(yytext);

      g_code->codify(text);
      g_type.resize(0);
      g_name.resize(0);
      BEGIN(g_memCallContext);
   }

<*>\n({B}*"//"[!/][^\n]*\n)+     {
      // remove special one-line comment
      QString text = QString::fromUtf8(yytext);

      if (YY_START == SkipCPP) {
         REJECT;
      }

      if (s_stripCodeComments) {
         g_yyLineNr += text.count('\n');
         nextCodeLine();

      } else {
         startFontClass("comment");
         codifyLines(text);
         endFontClass();
      }

      if (YY_START == SkipCxxComment) {
         endFontClass();
         BEGIN( g_lastCContext );
      }
   }

<SkipCPP>\n/.*\n        {
      endFontClass();
      unput('\n');

      BEGIN( g_lastSkipCppContext );
   }

<*>\n{B}*"//@"[{}].*\n        {
      // remove one-line group marker
      QString text = QString::fromUtf8(yytext);

      if (s_stripCodeComments) {
         g_yyLineNr += 2;
         nextCodeLine();

      } else {
         startFontClass("comment");
         codifyLines(text);
         endFontClass();
      }

      if (YY_START == SkipCxxComment) {
         endFontClass();
         BEGIN( g_lastCContext );
      }
   }

<*>\n{B}*"/*@"[{}]         {
      // */ (editor syntax fix)
      // remove one-line group marker
      QString text = QString::fromUtf8(yytext);

      if (s_stripCodeComments) {
         if (YY_START != RemoveSpecialCComment) {
            g_lastSpecialCContext = YY_START;
         }

         ++g_yyLineNr;
         BEGIN(RemoveSpecialCComment);

      } else {
         // check is to prevent getting stuck in skipping C++ comments
         if (YY_START != SkipComment && YY_START != SkipCxxComment) {
            g_lastCContext = YY_START;
         }

         startFontClass("comment");
         codifyLines(text);
         BEGIN(SkipComment);
      }
   }

<*>^{B}*"//@"[{}].*\n         {
      // remove one-line group marker
      QString text = QString::fromUtf8(yytext);

      if (s_stripCodeComments) {
         ++g_yyLineNr;
         nextCodeLine();

      } else {
         startFontClass("comment");
         codifyLines(text);
         endFontClass();
      }
   }

<*>^{B}*"/*@"[{}]          {
      // */ (editor syntax fix)
      // remove multi-line group marker
      QString text = QString::fromUtf8(yytext);

      if (s_stripCodeComments) {

         if (YY_START != RemoveSpecialCComment) {
            g_lastSpecialCContext = YY_START;
         }

         BEGIN(RemoveSpecialCComment);

      } else {
         // check is to prevent getting stuck in skipping C++ comments
         if (YY_START != SkipComment && YY_START != SkipCxxComment) {
            g_lastCContext = YY_START;
         }

         startFontClass("comment");
         g_code->codify(text);
         BEGIN(SkipComment);
      }
   }

<*>^{B}*"//"[!/][^\n]*   {
      // remove special one-line comment
      QString text = QString::fromUtf8(yytext);

      if (! s_stripCodeComments) {
         startFontClass("comment");
         codifyLines(text);
         endFontClass();
      }
   }

<*>"//"[!/][^\n]*   {
      // strip special one-line comment
      QString text = QString::fromUtf8(yytext);

      if (YY_START == SkipComment || YY_START == SkipString) {
         REJECT;
      }

      if (! s_stripCodeComments) {
         startFontClass("comment");
         codifyLines(text);
         endFontClass();
      }
   }

<*>"/*[tag:"[^\]\n]*"]*/"{B}*       {
      // special pattern /*[tag:filename]*/ to force linking to a tag file
      QString text = QString::fromUtf8(yytext);

      g_forceTagReference = text;

      int s = g_forceTagReference.indexOf(':');
      int e = g_forceTagReference.lastIndexOf(']');

      g_forceTagReference = g_forceTagReference.mid(s + 1, e - s - 1);
   }

<*>\n{B}*"/*"[!*]/[^/*]       {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      if (s_stripCodeComments) {

         if (YY_START != RemoveSpecialCComment) {
            g_lastSpecialCContext = YY_START;
         }

         ++g_yyLineNr;
         BEGIN(RemoveSpecialCComment);

      } else {
         // prevent getting stuck in skipping C++ comments
         if (YY_START != SkipComment && YY_START != SkipCxxComment) {
            g_lastCContext = YY_START;
         }

         startFontClass("comment");
         codifyLines(text);
         BEGIN(SkipComment);
      }
   }

<*>^{B}*"/**"[*]+/[^/]                  {

      // special C "banner" comment block at a new line
      QString text = QString::fromUtf8(yytext);

      if (s_stripCodeComments) {
         if (YY_START != RemoveSpecialCComment) {
            g_lastSpecialCContext = YY_START;
         }

        BEGIN(RemoveSpecialCComment);

      } else {
         // check is to prevent getting stuck in skipping C++ comment
         if (YY_START != SkipComment && YY_START != SkipCxxComment) {
            g_lastCContext = YY_START;
         }

         startFontClass("comment");
         g_code->codify(text);

         BEGIN(SkipComment);
      }
   }
<*>^{B}*"/*"[!*]/[^/*]        {
      // */ (editor syntax fix)
      // special C comment block at a new line

      QString text = QString::fromUtf8(yytext);

      if (s_stripCodeComments) {
         if (YY_START != RemoveSpecialCComment) {
            g_lastSpecialCContext = YY_START;
         }

         BEGIN(RemoveSpecialCComment);

      } else {
         // check is to prevent getting stuck in skipping C++ comments

         if (YY_START != SkipComment && YY_START != SkipCxxComment) {
            g_lastCContext = YY_START;
         }

         startFontClass("comment");
         g_code->codify(text);
         BEGIN(SkipComment);
      }
   }

<*>"/*"[!*]/[^/*]          {
      // */ (editor syntax fix)
      // special C comment block half way a line

      QString text = QString::fromUtf8(yytext);

      if (YY_START == SkipString) {
         REJECT;
      }

      if (s_stripCodeComments) {
         if (YY_START != RemoveSpecialCComment) {
            g_lastSpecialCContext = YY_START;
         }

         BEGIN(RemoveSpecialCComment);

      } else {
         // check is to prevent getting stuck in skipping C++ comments

         if (YY_START != SkipComment && YY_START != SkipCxxComment) {
            g_lastCContext = YY_START;
         }

         startFontClass("comment");
         g_code->codify(text);
         BEGIN(SkipComment);
      }
    }

<*>"/*"("!"?)"*/"          {
      QString text = QString::fromUtf8(yytext);

      if (YY_START == SkipString) {
         REJECT;
      }

      if (! s_stripCodeComments) {
         startFontClass("comment");
         g_code->codify(text);
         endFontClass();
      }
   }

<SkipComment>[^\*\n]+      {
      g_code->codify(QString::fromUtf8(yytext));
   }

<*>"/*"              {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);

      startFontClass("comment");
      g_code->codify(text);

      // check is to prevent getting stuck in skipping C++ comments

      if (YY_START != SkipComment && YY_START != SkipCxxComment) {
         g_lastCContext = YY_START ;
      }
      BEGIN( SkipComment );
   }

<*>@\"               {
      // C# verbatim string
      QString text = QString::fromUtf8(yytext);
      startFontClass("stringliteral");
      g_code->codify(text);
      g_lastVerbStringContext = YY_START;
      BEGIN(SkipVerbString);
   }

<*>"//"              {
      QString text = QString::fromUtf8(yytext);
      startFontClass("comment");
      g_code->codify(text);
      g_lastCContext = YY_START;
      BEGIN( SkipCxxComment );
   }

<*>"("|"["              {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      pushCallContext();
   }

<*>")"|"]"              {
      QString text = QString::fromUtf8(yytext);
      g_code->codify(text);
      popCallContext();
   }

<*>\n                {
      QString text = QString::fromUtf8(yytext);
      g_yyColNr++;
      codifyLines(text);
   }

<*>[\xC0-\xFF][\x80-\xBF]+     {
      // utf-8 code point
      QString text = QString::fromUtf8(yytext);
      g_yyColNr += text.size();
      g_code->codify(text);
   }

<*>.                  {
      QString text = QString::fromUtf8(yytext);
      ++g_yyColNr;
      g_code->codify(text);
   }


  /*
<*>([ \t\n]*"\n"){2,}         {
      // combine multiple blank lines
      QString text = QString::fromUtf8(yytext);

      QString sepLine = text;
      g_code->codify("\n\n");
      g_yyLineNr += sepLine.contains('\n');

      char sepLine[3] = "\n\n";
      codifyLines(text);
   }
  */

%%

static void saveObjCContext()
{
   if (g_ObjC_currentCtx) {
      g_ObjC_currentCtx->format += QString("$c%1").formatArg(g_currentCtxId);

      if (g_braceCount == 0 && YY_START == ObjCCall) {
         g_ObjC_currentCtx->objectTypeOrName = g_ObjC_currentCtx->format.mid(1);
      }

      g_ObjC_contextStack.push(g_ObjC_currentCtx);
   }

   ObjCCallCtx *temp = new ObjCCallCtx;

   temp->id         = g_currentCtxId;
   temp->lexState   = YY_START;
   temp->braceCount = g_braceCount;
   temp->objectType = QSharedPointer<ClassDef>();
   temp->objectVar  = QSharedPointer<MemberDef>();
   temp->method     = QSharedPointer<MemberDef>();

   g_ObjC_contextDict.insert(g_currentCtxId, temp);
   g_ObjC_currentCtx = temp;

   g_braceCount = 0;
   g_currentCtxId++;
}

static void restoreObjCContext()
{
  BEGIN(g_ObjC_currentCtx->lexState);

  g_braceCount = g_ObjC_currentCtx->braceCount;

  if (! g_ObjC_contextStack.isEmpty()) {
    g_ObjC_currentCtx = g_ObjC_contextStack.pop();

  } else {
    g_ObjC_currentCtx = nullptr;
  }
}

void resetCCodeParserState()
{
  g_forceTagReference.clear();
  g_theVarContext.clear();
  g_classScopeLengthStack.clear();
  g_codeClassSDict.clear();
  g_curClassBases.clear();

  g_anchorCount = 0;
}

void parseCCode(CodeGenerator &outputX, const QString &className, const QString &s,
                SrcLangExt lang, bool exBlock, const QString &exName, QSharedPointer<FileDef> fd,
                int startLine, int endLine, bool inlineFragment,
                QSharedPointer<MemberDef> memberDef, bool showLineNumbers,
                QSharedPointer<Definition> searchCtx, bool collectXRefs)
{
   if (s.isEmpty()) {
      return;
   }

   s_stripCodeComments = Config::getBool("strip-code-comments");

   TooltipManager::instance()->clearTooltips();

   g_code               = &outputX;
   s_inputString        = s;
   s_inputPosition      = 0;
   g_currentFontClass   = QString();
   g_needsTermination   = false;
   g_searchCtx          = searchCtx;
   g_collectXRefs       = collectXRefs;
   g_inFunctionTryBlock = false;

   if (startLine != -1) {
      g_yyLineNr    = startLine;
   } else {
      g_yyLineNr    = 1;
   }

   if (endLine != -1) {
      g_inputLines  = endLine + 1;
   } else {
      g_inputLines  = g_yyLineNr + countLines() - 1;
   }

   g_curlyCount      = 0;
   g_bodyCurlyCount  = 0;
   g_bracketCount    = 0;
   g_sharpCount      = 0;
   g_insideTemplate  = false;

   //
   g_contextStack.clear();
   pushCallContext();

   g_scopeStack.clear();

   g_classScope    = className;
   g_exampleBlock  = exBlock;
   g_exampleName   = exName;
   g_sourceFileDef = fd;
   g_lineNumbers   = (fd != 0 && showLineNumbers);

   bool cleanupSourceDef = false;

   if (fd == nullptr) {
      // create a dummy filedef for the example
      g_sourceFileDef  = QMakeShared<FileDef>(QString(), (! exName.isEmpty() ? exName : "generated"));
      cleanupSourceDef = true;
   }

   g_insideObjC = lang == SrcLangExt_ObjC;
   g_insideJava = lang == SrcLangExt_Java;
   g_insideCS   = lang == SrcLangExt_CSharp;
   g_insidePHP  = lang == SrcLangExt_PHP;
   g_insideCpp  = lang == SrcLangExt_Cpp;

   if (g_sourceFileDef) {
      setCurrentDoc("l00001");
   }

   if (searchCtx == nullptr) {
       g_currentDefinition = getResolvedNamespace(className);
   } else {
       g_currentDefinition = searchCtx;
   }

   g_currentMemberDef  = QSharedPointer<MemberDef>();

   g_searchingForBody  = exBlock;
   g_insideBody        = false;
   g_bracketCount      = 0;

   if (! g_exampleName.isEmpty()) {
      g_exampleFile = convertNameToFile_internal(g_exampleName + "-example", false, true);
   }

   g_includeCodeFragment = inlineFragment;

   startCodeLine();
   g_type.resize(0);
   g_name.resize(0);
   g_args.resize(0);
   g_parmName.resize(0);
   g_parmType.resize(0);

   if (memberDef) {
      setParameterList(memberDef);
   }

   yyrestart( yyin );
   BEGIN( Body );
   yylex();

   g_lexInit = true;

   if (g_needsTermination) {
      endFontClass();
      DBG_CTX((stderr, "endCodeLine(%d)\n", g_yyLineNr));
      g_code->endCodeLine();
   }

   if (fd) {
      TooltipManager::instance()->writeTooltips(*g_code);
   }

   if (cleanupSourceDef) {
      // delete the temporary file definition used for this example
      g_sourceFileDef = QSharedPointer<FileDef>();
   }

   return;
}

void codeFreeScanner()
{
   if (g_lexInit) {
      yylex_destroy();
   }
}
