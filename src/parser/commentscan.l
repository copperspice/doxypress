/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim
 * Copyright (C) 1997-2014 by Dimitri van Heesch.

*************************************************************************/

%{

#include <QFile>
#include <QStack>
#include <QVector>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include <cite.h>
#include <condparser.h>
#include <config.h>
#include <default_args.h>
#include <doxy_globals.h>
#include <entry.h>
#include <formula.h>
#include <index.h>
#include <language.h>
#include <membergroup.h>
#include <message.h>
#include <outputlist.h>
#include <parse_base.h>
#include <parse_cstyle.h>
#include <parse_md.h>
#include <util.h>

#define YY_NO_INPUT 1
#define YY_NEVER_INTERACTIVE 1

// forward declarations
static bool handleBrief(const QString &);
static bool handleFn(const QString &);
static bool handleDef(const QString &);
static bool handleOverload(const QString &);
static bool handleEnum(const QString &);
static bool handleDefGroup(const QString &);
static bool handleAddToGroup(const QString &);
static bool handleWeakGroup(const QString &);
static bool handleNamespace(const QString &);
static bool handlePackage(const QString &);
static bool handleClass(const QString &);
static bool handleHeaderFile(const QString &);
static bool handleProtocol(const QString &);
static bool handleCategory(const QString &);
static bool handleUnion(const QString &);
static bool handleStruct(const QString &);
static bool handleInterface(const QString &);
static bool handleIdlException(const QString &);
static bool handlePage(const QString &);
static bool handleMainpage(const QString &);
static bool handleFile(const QString &);
static bool handleDir(const QString &);
static bool handleExample(const QString &);
static bool handleDetails(const QString &);
static bool handleName(const QString &);
static bool handleTodo(const QString &);
static bool handleTest(const QString &);
static bool handleBug(const QString &);
static bool handleSubpage(const QString &s);
static bool handleDeprecated(const QString &);
static bool handleXRefItem(const QString &);
static bool handleRelated(const QString &);
static bool handleRelatedAlso(const QString &);
static bool handleMemberOf(const QString &);
static bool handleRefItem(const QString &);
static bool handleSection(const QString &);
static bool handleAnchor(const QString &);
static bool handleCite(const QString &);
static bool handleFormatBlock(const QString &);
static bool handleAddIndex(const QString &);
static bool handleIf(const QString &);
static bool handleIfNot(const QString &);
static bool handleElseIf(const QString &);
static bool handleElse(const QString &);
static bool handleEndIf(const QString &);
static bool handleIngroup(const QString &);
static bool handleNoSubGrouping(const QString &);
static bool handleShowInitializer(const QString &);
static bool handleHideInitializer(const QString &);
static bool handleCallgraph(const QString &);
static bool handleHideCallgraph(const QString &);
static bool handleCallergraph(const QString &);
static bool handleHideCallergraph(const QString &);
static bool handleInternal(const QString &);
static bool handleLineBr(const QString &);
static bool handleStatic(const QString &);
static bool handlePure(const QString &);
static bool handlePrivate(const QString &);
static bool handlePrivateSection(const QString &);
static bool handleProtected(const QString &);
static bool handleProtectedSection(const QString &);
static bool handlePublic(const QString &s);
static bool handlePublicSection(const QString &s);
static bool handleToc(const QString &s);
static bool handleInherit(const QString &);
static bool handleExtends(const QString &);
static bool handleCopyDoc(const QString &);
static bool handleCopyBrief(const QString &);
static bool handleCopyDetails(const QString &);
static bool handleParBlock(const QString &);
static bool handleEndParBlock(const QString &);
static void handleGuard(const QString &expr);
static bool handleParam(const QString &);
static bool handleRetval(const QString &);

static void checkFormula();

using DocCmdFunc = bool (*)(const QString &name);

struct DocCmdMap
{
  const char *cmdName;
  DocCmdFunc handler;
  bool endsBrief;
};

// map of command to handler function
static DocCmdMap docCmdMap[] =
{
  // command name      handler function         ends brief description
  { "brief",           &handleBrief,            false },
  { "short",           &handleBrief,            false },
  { "fn",              &handleFn,               false },
  { "var",             &handleFn,               false },
  { "typedef",         &handleFn,               false },
  { "property",        &handleFn,               false },
  { "def",             &handleDef,              false },
  { "overload",        &handleOverload,         false },
  { "enum",            &handleEnum,             false },
  { "defgroup",        &handleDefGroup,         false },
  { "group",           &handleDefGroup,         false },
  { "addtogroup",      &handleAddToGroup,       false },
  { "weakgroup",       &handleWeakGroup,        false },
  { "namespace",       &handleNamespace,        false },
  { "package",         &handlePackage,          false },
  { "class",           &handleClass,            false },
  { "headerfile",      &handleHeaderFile,       false },
  { "protocol",        &handleProtocol,         false },
  { "category",        &handleCategory,         false },
  { "union",           &handleUnion,            false },
  { "struct",          &handleStruct,           false },
  { "interface",       &handleInterface,        false },
  { "idlexcept",       &handleIdlException,     false },
  { "page",            &handlePage,             false },
  { "mainpage",        &handleMainpage,         false },
  { "file",            &handleFile,             false },
  { "dir",             &handleDir,              false },
  { "example",         &handleExample,          false },
  { "details",         &handleDetails,          true  },
  { "name",            &handleName,             false },
  { "todo",            &handleTodo,             false }, // end brief will be done differently
  { "test",            &handleTest,             false }, // end brief will be done differently
  { "bug",             &handleBug,              false }, // end brief will be done differently
  { "deprecated",      &handleDeprecated,       false }, // end brief will be done differently
  { "xrefitem",        &handleXRefItem,         false }, // end brief will be done differently
  { "related",         &handleRelated,          true  },
  { "relates",         &handleRelated,          true  },
  { "relatedalso",     &handleRelatedAlso,      true  },
  { "relatesalso",     &handleRelatedAlso,      true  },
  { "parblock",        &handleParBlock,         true  },
  { "endparblock",     &handleEndParBlock,      true  },
  { "refitem",         &handleRefItem,          true  },
  { "cite",            &handleCite,             false },
  { "subpage",         &handleSubpage,          true  },
  { "section",         &handleSection,          true  },
  { "subsection",      &handleSection,          true  },
  { "subsubsection",   &handleSection,          true  },
  { "paragraph",       &handleSection,          true  },
  { "anchor",          &handleAnchor,           true  },
  { "verbatim",        &handleFormatBlock,      true  },
  { "latexonly",       &handleFormatBlock,      false },
  { "htmlonly",        &handleFormatBlock,      false },
  { "xmlonly",         &handleFormatBlock,      false },
  { "docbookonly",     &handleFormatBlock,      false },
  { "rtfonly",         &handleFormatBlock,      false },
  { "manonly",         &handleFormatBlock,      false },
  { "dot",             &handleFormatBlock,      true  },
  { "msc",             &handleFormatBlock,      true  },
  { "startuml",        &handleFormatBlock,      true  },
  { "code",            &handleFormatBlock,      true  },
  { "addindex",        &handleAddIndex,         false },
  { "if",              &handleIf,               false },
  { "ifnot",           &handleIfNot,            false },
  { "elseif",          &handleElseIf,           false },
  { "else",            &handleElse,             false },
  { "endif",           &handleEndIf,            false },
  { "ingroup",         &handleIngroup,          false },
  { "nosubgrouping",   &handleNoSubGrouping,    false },
  { "showinitializer", &handleShowInitializer,  false },
  { "hideinitializer", &handleHideInitializer,  false },
  { "callgraph",       &handleCallgraph,        false },
  { "hidecallgraph",   &handleHideCallgraph,    false },
  { "callergraph",     &handleCallergraph,      false },
  { "hidecallergraph", &handleHideCallergraph,  false },
  { "internal",        &handleInternal,         true  },
  { "_linebr",         &handleLineBr,           false },
  { "static",          &handleStatic,           false },
  { "pure",            &handlePure,             false },
  { "private",         &handlePrivate,          false },
  { "privatesection",  &handlePrivateSection,   false },
  { "protected",       &handleProtected,        false },
  { "protectedsection",&handleProtectedSection, false },
  { "public",          &handlePublic,           false },
  { "publicsection",   &handlePublicSection,    false },
  { "tableofcontents", &handleToc,              false },
  { "inherit",         &handleInherit,          true  },
  { "extends",         &handleExtends,          true  },
  { "implements",      &handleExtends,          true  },
  { "memberof",        &handleMemberOf,         true  },
  { "arg",             0,                       true  },
  { "attention",       0,                       true  },
  { "author",          0,                       true  },
  { "authors",         0,                       true  },
  { "copydoc",         &handleCopyDoc,          true  },
  { "copybrief",       &handleCopyBrief,        false },
  { "copydetails",     &handleCopyDetails,      true  },
  { "copyright",       0,                       true  },
  { "date",            0,                       true  },
  { "dotfile",         0,                       true  },
  { "htmlinclude",     0,                       false },
  { "image",           0,                       true  },
  { "include",         0,                       true  },
  { "includelineno",   0,                       true  },
  { "invariant",       0,                       true  },
  { "latexinclude",    0,                       false },
  { "li",              0,                       true  },
  { "line",            0,                       true  },
  { "note",            0,                       true  },
  { "par",             0,                       true  },
  { "param",           &handleParam,            true  },
  { "tparam",          0,                       true  },
  { "post",            0,                       true  },
  { "pre",             0,                       true  },
  { "remark",          0,                       true  },
  { "remarks",         0,                       true  },
  { "result",          0,                       true  },
  { "return",          0,                       true  },
  { "returns",         0,                       true  },
  { "exception",       0,                       true  },
  { "retval",          &handleRetval,           true  },
  { "sa",              0,                       true  },
  { "see",             0,                       true  },
  { "since",           0,                       true  },
  { "throw",           0,                       true  },
  { "throws",          0,                       true  },
  { "until",           0,                       true  },
  { "verbinclude",     0,                       false },
  { "version",         0,                       true  },
  { "warning",         0,                       true  },
  { 0, 0, false }
};

/** @brief Command mapper.
 *
 *  Maps a command name (as found in a comment block) onto a specific handler function
 */
class DocCmdMapper
{
 public:
   struct Cmd {
      DocCmdFunc func;
      bool endsBrief;
   };

   /** maps a command name to a handler function */
   static Cmd *map(const QString &name) {
      return instance()->find(name);
   }

   /** release the singleton */
   static void freeInstance() {
      delete s_instance;
      s_instance = 0;
   }

  private:
    static DocCmdMapper *instance() {
      if (s_instance == nullptr) {
         s_instance = new DocCmdMapper;
      }
      return s_instance;
   }

   DocCmdMapper() {
      DocCmdMap *p = docCmdMap;

      while (p->cmdName) {
         if (m_map.contains(p->cmdName)) {
            err("DocCmdMapper: command %s already added\n", p->cmdName);
            Doxy_Work::stopDoxyPress();
         }

         Cmd *cmd       = new Cmd;
         cmd->func      = p->handler;
         cmd->endsBrief = p->endsBrief;

         m_map.insert(p->cmdName, cmd);
         p++;
      }
   }

   Cmd *find(const QString &name) {
      return m_map.value(name);
   }

   QHash<QString, Cmd *> m_map;
   static DocCmdMapper *s_instance;
};

DocCmdMapper *DocCmdMapper::s_instance = 0;

#define YY_NEVER_INTERACTIVE 1

enum XRefKind {
  XRef_Item,
  XRef_Todo,
  XRef_Test,
  XRef_Bug,
  XRef_Deprecated,
  XRef_None
};

enum OutputContext {
  OutputMainDoc,
  OutputBrief,
  OutputXRef,
  OutputInbody
};

enum GuardType {
  Guard_If,
  Guard_IfNot,
  Guard_Skip
};

class GuardedSection
{
 public:
   GuardedSection(bool enabled, bool parentVisible)
      : m_enabled(enabled), m_parentVisible(parentVisible) {}
   bool isEnabled() const {
      return m_enabled;
   }
   bool parentVisible() const {
      return m_parentVisible;
   }

 private:
   bool m_enabled;
   bool m_parentVisible;
};

void openGroup(QSharedPointer<Entry> e, const QString &file, int line);
void closeGroup(QSharedPointer<Entry> e, const QString &file, int line, bool foundInline = false);
void initGroupInfo(QSharedPointer<Entry> e);
static void groupAddDocs(QSharedPointer<Entry> e);

static QString          s_inputString;         // input string
static int              s_inputPosition;       // read pointer
static int              s_prevPosition;
static char            *s_bufferPosition;

static QString          yyFileName;            // file name that is read from
static int              yyLineNr;              // line number in the input
static bool             inBody;                // was the comment found inside the body of a function?
static OutputContext    inContext;             // are we inside the brief, details or xref part
static bool             briefEndsAtDot;        // does the brief description stop at a dot?
static QString          formulaText;           // Running text of a formula
static QString          formulaEnv;            // environment name
static int              formulaNewLines;       // amount of new lines in the formula

static QString          s_outputXRef;          // tmp argument of todo/test/../xrefitem commands
static QString          blockName;             // preformatted block name (e.g. verbatim, latexonly,...)
static XRefKind         xrefKind;              // kind of cross-reference command
static XRefKind         newXRefKind;
static GuardType        guardType;             // kind of guard for conditional section
static bool             enabledSectionFound;
static QString          functionProto;         // function prototype

static bool             s_needNewEntry;
static int              s_docBlockContext;

static QString          s_sectionLabel;
static QString          s_sectionTitle;
static int              s_sectionLevel;
static QString          xrefItemKey;
static QString          newXRefItemKey;
static QString          xrefItemTitle;
static QString          xrefListTitle;
static Protection       protection;

static bool             xrefAppendFlag;
static bool             inGroupParamFound;
static int              braceCount;
static bool             insidePre;
static bool             s_parseMore;

static int              s_condCount;
static int              s_commentCount;
static QString          s_spaceBeforeCmd;
static QString          s_spaceBeforeIf;
static QString          s_copyDocArg;
static QString          s_guardExpr;
static int              s_roundCount;
static bool             s_insideParBlock;

static int              s_memberGroupId = DOX_NOGROUP;
static QString          s_memberGroupHeader;
static QString          s_memberGroupDocs;
static QString          s_memberGroupRelates;
static QString          s_compoundName;

static bool             s_internalDocs;
static bool             s_processInternalDocs = false;


static ParserInterface        *langParser;        // the language parser that is calling us

static QSharedPointer<Entry>  s_docsEntry;        // which entry
static EntryKey               s_docsEnum;         // which enum in EntryKey (brief, main, inbody)

static QStack<GuardedSection> s_guards;           // tracks nested conditional sections (if, ifnot, ..)
static QSharedPointer<Entry>  current;            // working entry

static QStack<QSharedPointer<Grouping>> s_autoGroupStack;

static void initParser()
{
   s_sectionLabel.resize(0);
   s_sectionTitle.resize(0);
   s_memberGroupHeader.resize(0);

   s_insideParBlock = false;
   s_internalDocs   = Config::getBool("internal-docs");
}

static bool getDocSectionName(int s)
{
  switch(s) {
    case Entry::CLASSDOC_SEC:
    case Entry::STRUCTDOC_SEC:
    case Entry::UNIONDOC_SEC:
    case Entry::EXCEPTIONDOC_SEC:
    case Entry::NAMESPACEDOC_SEC:
    case Entry::PROTOCOLDOC_SEC:
    case Entry::CATEGORYDOC_SEC:
    case Entry::ENUMDOC_SEC:
    case Entry::PAGEDOC_SEC:
    case Entry::VARIABLEDOC_SEC:
    case Entry::MEMBERDOC_SEC:
    case Entry::OVERLOADDOC_SEC:
    case Entry::FILEDOC_SEC:
    case Entry::DEFINEDOC_SEC:
    case Entry::GROUPDOC_SEC:
    case Entry::MAINPAGEDOC_SEC:
    case Entry::PACKAGEDOC_SEC:
    case Entry::DIRDOC_SEC:
    case Entry::EXAMPLE_SEC:
    case Entry::MEMBERGRP_SEC:
      return true;
    default:
      return false;
  }
}

static bool makeStructuralIndicator(Entry::Sections s)
{
  if (getDocSectionName(current->section)) {
    return true;

  } else {
    s_needNewEntry     = true;
    current->section   = s;
    current->startLine = yyLineNr;

    current->setData(EntryKey::File_Name, yyFileName);
    return false;
  }
}
static void lineCount()
{
  for (const char *c = yytext ; *c ; ++c) {
    yyLineNr += (*c == '\n') ;
  }
}

static QString stripQuotes(const QString &s)
{
   QString name = s;

   if (name.isEmpty()) {
      return name;
   }

   if (name.at(0) == '"' && name.at(name.length() - 1) == '"') {
      name = name.mid(1, name.length() - 2);
   }

   return name;
}

static void addXRefItem(const QString &listName, const QString &itemTitle, const QString &listTitle, bool append)
{
   QSharedPointer<Entry> docEntry = current; // inBody && previous ? previous : current;

   if (listName.isEmpty()) {
      return;
   }

   auto refList = Doxy_Globals::xrefLists.find(listName);

   if (refList == Doxy_Globals::xrefLists.end()) {
      // new list
      Doxy_Globals::xrefLists.insert(listName, RefList(listName, listTitle, itemTitle));
      refList = Doxy_Globals::xrefLists.find(listName);
   }

   ListItemInfo *listItem;

   for (auto &item : docEntry->m_specialLists) {
      listItem = &item;

      if (listItem->type == listName) {
         break;
      }
   }

   if (listItem && append) {
      // already found item of same type just before this one

      RefItem *item = refList->getRefItem(listItem->itemId);
      assert(item != 0);

      item->text += " <p>";

      if (Doxy_Globals::markdownSupport) {
         item->text += processMarkdown(yyFileName, yyLineNr, current, s_outputXRef);
      } else {
         item->text += s_outputXRef;
      }

   } else {
      // new item

      int itemId  = refList->addRefItem();

      // if we have already an item from the same list type (e.g. a second @todo)
      // in the same Entry (i.e. lii!=0) then we reuse its link anchor.

      QString anchorLabel = QString("_%1%2").arg(listName).arg(itemId, 6, 10, QChar('0'));

      RefItem *item = refList->getRefItem(itemId);
      assert(item != 0);

      if (Doxy_Globals::markdownSupport) {
         item->text = processMarkdown(yyFileName, yyLineNr, current, s_outputXRef);
      } else {
         item->text = s_outputXRef;
      }

      item->listAnchor = anchorLabel;
      docEntry->addSpecialListItem(listName, itemId);

      QString cmdString = QString("\\xrefitem %1 %2.").arg(listName).arg(itemId);

      if (inBody) {
         docEntry->appendData(EntryKey::Inbody_Docs, cmdString);
      } else {
         docEntry->appendData(EntryKey::Main_Docs,   cmdString);
      }

      QSharedPointer<SectionInfo> si = Doxy_Globals::sectionDict.find(anchorLabel);

      if (si) {
         if (si->lineNr != -1) {
            warn(listName, yyLineNr, "Multiple use of section label '%s', (first occurrence: %s, line %d)",
                  csPrintable(anchorLabel), csPrintable(si->fileName), si->lineNr);

         } else {
            warn(listName, yyLineNr, "Multiple use of section label '%s', (first occurrence: %s)",
                  csPrintable(anchorLabel), csPrintable(si->fileName));
         }

      } else {
         si = QMakeShared<SectionInfo>(listName, yyLineNr, anchorLabel, s_sectionTitle, SectionInfo::Anchor, s_sectionLevel);
         Doxy_Globals::sectionDict.insert(anchorLabel, si);
         docEntry->m_anchors.append(*si);
      }
   }

   s_outputXRef = "";
}

// Adds a formula text to the list/dictionary of formulas if it was
// not already added. Returns the label of the formula.
static QString addFormula()
{
   QString formLabel;
   QString fText = formulaText.simplified();

   auto f = Doxy_Globals::formulaDict.find(fText);

   if (f == Doxy_Globals::formulaDict.end()) {
      Formula temp = Formula(fText);

      Doxy_Globals::formulaList.append(temp);
      Doxy_Globals::formulaDict.insert(fText, temp);

      formLabel = QString("\\form#%1").arg(temp.getId());

      Doxy_Globals::formulaNameDict.insert(formLabel, temp);
      f = Doxy_Globals::formulaDict.find(fText);

   } else {
      formLabel = QString("\\form#%1").arg(f->getId());

   }

   for (int i = 0; i < formulaNewLines; i++) {
      formLabel += "@_fakenl";   // add fake newlines to
   }

   // keep the warnings correctly aligned
   return formLabel;
}

static void checkFormula();

static SectionInfo::SectionType sectionLevelToType(int level)
{
  if (level >= 0 && level < 5) {
      return (SectionInfo::SectionType)level;
   }

   return SectionInfo::Anchor;
}

static void addSection()
{
   QSharedPointer<SectionInfo> si = Doxy_Globals::sectionDict.find(s_sectionLabel);

   if (si) {
      if (si->lineNr != -1) {
         warn(yyFileName, yyLineNr, "Multiple use of section label '%s' while adding section, (first occurrence: %s, line %d)",
                  csPrintable(s_sectionLabel), csPrintable(si->fileName), si->lineNr);

      } else {
         warn(yyFileName, yyLineNr, "Multiple use of section label '%s' while adding section, (first occurrence: %s)",
                  csPrintable(s_sectionLabel), csPrintable(si->fileName));
      }

   } else {
      // create a new section element
      s_sectionTitle += QString::fromUtf8(yytext).trimmed();

      si = QMakeShared<SectionInfo>(yyFileName, yyLineNr, s_sectionLabel, s_sectionTitle,
                  sectionLevelToType(s_sectionLevel), s_sectionLevel);

      // add section to this entry
      current->m_anchors.append(*si);

      // add section to the global dictionary
      Doxy_Globals::sectionDict.insert(s_sectionLabel, si);

      // for a section the si->fileName is the physical file name. this will be replaced with the section name
      // in doctokenizer processSection()
   }
}

static void addCite()
{
   QString text = QString::fromUtf8(yytext);
   Doxy_Globals::citeDict.insert(text);
}

// strip trailing whitespace (excluding newlines) from string s
static void stripTrailingWhiteSpace(QString &s)
{
   uint len = s.length();

   int i = len - 1;
   QChar c;

   while (i >= 0 && ((c = s.at(i)) == ' ' || c == '\t' || c == '\r')) {
      i--;
   }

   if (i != len - 1) {
      // string up to and including char at pos i and \0 terminator
      s.resize(i + 2);
   }
}

// selects the output to write to
static inline void setOutput(OutputContext ctx)
{
   bool xrefAppendToPrev = xrefAppendFlag;

   // determine append flag for the next item (i.e. the end of this item)
   xrefAppendFlag = ! inBody &&
                    inContext == OutputXRef && ctx == OutputXRef &&    // two consecutive xref items
                    newXRefKind == xrefKind &&                         // of the same kind
                    (xrefKind != XRef_Item ||
                    newXRefItemKey == xrefItemKey);                    // with the same key if \xrefitem

   if (inContext == OutputXRef) {

      // end of XRef section => add the item
      // See if we can append this new xref item to the previous one.
      // We know this at the start of the next item of the same
      // type and need to remember this until the end of that item.

    switch(xrefKind) {
         case XRef_Todo:
            addXRefItem("todo", theTranslator->trTodo(), theTranslator->trTodoList(), xrefAppendToPrev);
            break;

         case XRef_Test:
            addXRefItem("test", theTranslator->trTest(), theTranslator->trTestList(), xrefAppendToPrev);
            break;

         case XRef_Bug:
            addXRefItem("bug", theTranslator->trBug(), theTranslator->trBugList(), xrefAppendToPrev);
            break;

         case XRef_Deprecated:
            addXRefItem("deprecated", theTranslator->trDeprecated(), theTranslator->trDeprecatedList(), xrefAppendToPrev);
            break;

         case XRef_Item:  // user defined list
            addXRefItem(xrefItemKey, xrefItemTitle, xrefListTitle, xrefAppendToPrev);
            break;

         case XRef_None:
            assert(0);
            break;
      }
   }

  xrefItemKey = newXRefItemKey;

  int oldContext = inContext;
  inContext = ctx;

   if (inContext != OutputXRef && inBody) {
      inContext = OutputInbody;
   }

  switch(inContext) {
      case OutputMainDoc:
         if (oldContext != inContext) {

            QString tmpDocs = current->getData(EntryKey::Main_Docs);
            stripTrailingWhiteSpace(tmpDocs);
            current->setData(EntryKey::Main_Docs, tmpDocs);

            if (current->getData(EntryKey::MainDocs_File).isEmpty()) {
               current->setData(EntryKey::MainDocs_File, yyFileName);
               current->docLine = yyLineNr;
            }
         }

         s_docsEntry = current;
         s_docsEnum  = EntryKey::Main_Docs;

         break;

      case OutputBrief:
         if (oldContext != inContext) {

            if (current->getData(EntryKey::Brief_File).isEmpty()) {
               current->setData(EntryKey::Brief_File,   yyFileName);
               current->briefLine = yyLineNr;
            }
         }

         if (current->getData(EntryKey::Brief_Docs).trimmed().isEmpty())  {
            // we only want one brief description even if multiple are given
            s_docsEntry = current;
            s_docsEnum  = EntryKey::Brief_Docs;

         } else {
            s_docsEntry  = current;
            s_docsEnum   = EntryKey::Main_Docs;

            inContext    = OutputMainDoc;             // need to switch to detailed docs
         }
         break;

      case OutputInbody:
         s_docsEntry = current;
         s_docsEnum  = EntryKey::Inbody_Docs;
         break;

      case OutputXRef:
         // indicates s_outputXRef should be used for the output string
         s_docsEntry = QSharedPointer<Entry>();

         break;
   }
}

static void addAnchor(const QString &anchorName)
{
   QSharedPointer<SectionInfo> si = Doxy_Globals::sectionDict.find(anchorName);

   if (si) {
      // anchor name already exists
      si->dupAnchor_cnt++;

   } else {
      // title is empty, level is zero
      si = QMakeShared<SectionInfo>(yyFileName, yyLineNr, anchorName, QString(""), SectionInfo::Anchor, 0);
      si->dupAnchor_fName = yyFileName;

      Doxy_Globals::sectionDict.insert(anchorName, si);
      current->m_anchors.append(*si);
   }
}

// add a string to one of the three doc outputs
static void addToOutput(const QString &str)
{
   if (s_docsEntry == nullptr) {
      // indicates s_outputXRef should be used for the output string
      s_outputXRef += str;

   } else {
      s_docsEntry->appendData(s_docsEnum, str);
   }
}

static void addToOutput(QChar c)
{
   if (s_docsEntry == nullptr) {
      // indicates s_outputXRef should be used for the output string
      s_outputXRef += c;

   } else {
      s_docsEntry->appendData(s_docsEnum, c);
   }
}

static void endBrief(bool isOutput = true)
{
   if (! current->getData(EntryKey::Brief_Docs).trimmed().isEmpty()) {
      // only go to the detailed description if we found some brief description and not just whitespace

      briefEndsAtDot = false;
      setOutput(OutputMainDoc);

      if (isOutput) {
         QString text = QString::fromUtf8(yytext);
         addToOutput(text);
      }
   }
}

static void handleGuard(const QString &expr);

#undef   YY_INPUT
#define  YY_INPUT(buf,result,max_size) result = yyread(buf,max_size);

static int yyread(char *buf, int max_size)
{
   s_bufferPosition = buf;
   s_prevPosition   = s_inputPosition;

   //
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

/* start command character */
CMD             ("\\"|"@")
DCMD1           ("arg"|"attention"|"author"|"cite"|"code")
DCMD2           ("date"|"dot"|"msc"|"dotfile"|"example"|"startuml")
DCMD3           ("htmlinclude"|"htmlonly"|"image"|"include")
DCMD4           ("includelineno"|"internal"|"invariant")
DCMD5           ("latexinclude"|"latexonly"|"li"|"line"|"manonly"|"name")
DCMD6           ("note"|"par"|"paragraph"|"param"|"post")
DCMD7           ("pre"|"remarks"|(("relate"[sd])("also")?))
DCMD8           ("remarks"|("return"[s]?)|"retval"|"sa"|"section")
DCMD9           ("see"|"since"|"subsection"|"subsubsection")
DCMD10          ("throw"|"until"|"verbatim")
DCMD11          ("verbinclude"|"version"|"warning")
DETAILEDCMD     {CMD}({DCMD1}|{DCMD2}|{DCMD3}|{DCMD4}|{DCMD5}|{DCMD6}|{DCMD7}|{DCMD8}|{DCMD9}|{DCMD10}|{DCMD11})
XREFCMD         {CMD}("bug"|"deprecated"|"test"|"todo"|"xrefitem")
PRE             [pP][rR][eE]
TABLE           [tT][aA][bB][lL][eE]
P               [pP]
UL              [uU][lL]
OL              [oO][lL]
DL              [dD][lL]
IMG             [iI][mM][gG]
HR              [hH][rR]
PARA            [pP][aA][rR][aA]
CODE            [cC][oO][dD][eE]
CAPTION         [cC][aA][pP][tT][iI][oO][nN]
DETAILEDHTML    {PRE}|{UL}|{TABLE}|{OL}|{DL}|{P}|[Hh][1-6]|{IMG}|{HR}|{PARA}
DETAILEDHTMLOPT {CODE}
BN              [ \t\n\r]
BL              [ \t\r]*"\n"
B               [ \t]
BS              ^(({B}*"//")?)(({B}*"*"+)?){B}*
ATTR            ({B}+[^>\n]*)?
DOCNL           "\n"|"\\_linebr"
LC              "\\"{B}*"\n"
NW              [^a-z_A-Z0-9]
FILESCHAR       [a-z_A-Z0-9\x80-\xFF\\:\\\/\-\+@&#]
FILEECHAR       [a-z_A-Z0-9\x80-\xFF\-\+@&#]
FILE            ({FILESCHAR}*{FILEECHAR}+("."{FILESCHAR}*{FILEECHAR}+)*)|("\""[^\n\"]*"\"")
ID              "$"?[a-z_A-Z\x80-\xFF][a-z_A-Z0-9\x80-\xFF]*
LABELID         [a-z_A-Z\x80-\xFF][a-z_A-Z0-9\x80-\xFF\-]*
CITESCHAR       [a-z_A-Z0-9\x80-\xFF]
CITEECHAR       [a-z_A-Z0-9\x80-\xFF\-\+:\/]*
CITEID          {CITESCHAR}{CITEECHAR}*("."{CITESCHAR}{CITEECHAR}*)*
SCOPEID         {ID}({ID}*{BN}*"::"{BN}*)*({ID}?)
SCOPENAME       "$"?(({ID}?{BN}*("::"|"."){BN}*)*)((~{BN}*)?{ID})
TMPLSPEC        "<"{BN}*[^>]+{BN}*">"
MAILADDR         [a-z_A-Z0-9.+\-]+"@"[a-z_A-Z0-9\-]+("."[a-z_A-Z0-9\-]+)+[a-z_A-Z0-9\-]+
RCSTAG          "$"{ID}":"[^\n$]+"$"

%option noyywrap

/* comment parsing states */
%x Comment
%x PageDocArg1
%x PageDocArg2
%x RelatesParam1
%x ClassDocArg1
%x ClassDocArg2
%x ClassDocArg3
%x CategoryDocArg1
%x XRefItemParam1
%x XRefItemParam2
%x XRefItemParam3
%x FileDocArg1
%x ParamArg1
%x EnumDocArg1
%x NameSpaceDocArg1
%x PackageDocArg1
%x GroupDocArg1
%x GroupDocArg2
%x SectionLabel
%x SectionTitle
%x SubpageLabel
%x SubpageTitle
%x FormatBlock
%x LineParam
%x GuardParam
%x GuardParamEnd
%x SkipGuardedSection
%x SkipInternal
%x NameParam
%x InGroupParam
%x FnParam
%x OverloadParam
%x InheritParam
%x ExtendsParam
%x ReadFormulaShort
%x ReadFormulaLong
%x AnchorLabel
%x HtmlComment
%x SkipLang
%x CiteLabel
%x CopyDoc
%x GuardExpr

%%

  /* What can happen while parsing a comment block:
   *   commands (e.g. @page, or \page)
   *   escaped commands (e.g. @@page or \\page).
   *   formulas (e.g. \f$ \f[ \f{..)
   *   directories (e.g. \doxy\src\)
   *   autolist end. (e.g. a dot on an otherwise empty line)
   *   newlines.
   *   end of brief description due to blank line.
   *   end of brief description due to some command (@command, or <command>).
   *   words and whitespace and other characters (#,?!, etc).
   *   grouping commands (e.g. @{ and @})
   *   language switch (e.g. \~english or \~).
   *   mail address (e.g. dimitri@stack.nl).
   *   quoted text, such as "foo@bar"
   *   XML commands, <summary></summary><remarks></remarks>
   */

<Comment>{CMD}{CMD}[a-z_A-Z]+{B}*   {
      // escaped command
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>{CMD}{CMD}"~"[a-z_A-Z]*    {
      // escaped command
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>{MAILADDR}    {
      // mail address
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>"\""[^"\n]*"\""      {
      // quoted text
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>("\\"[a-z_A-Z]+)+"\\"      {
      // directory (or chain of commands)
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>"<"{DETAILEDHTML}{ATTR}">"    {
      // HTML command ends a brief description
      setOutput(OutputMainDoc);

      // continue with the same input
      REJECT;
   }

<Comment>"<"{DETAILEDHTMLOPT}{ATTR}">"    {
      // HTML command that ends a brief description
      if (current->m_srcLang == SrcLangExt_CSharp) {
         setOutput(OutputMainDoc);
      }

      // continue with the same input
      REJECT;
   }

<Comment>"<summary>"          {
      // start of a .NET XML style brief description
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
      setOutput(OutputBrief);
   }

<Comment>"<remarks>"          {
      // start of a .NET XML style detailed description
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
      setOutput(OutputBrief);
   }

<Comment>"</summary>"          {
      // start of a .NET XML style detailed description
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
      setOutput(OutputMainDoc);
   }

<Comment>"</remarks>"         {
      // end of a brief or detailed description
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>"<"{CAPTION}{ATTR}">"          {
      QString tag = QString::fromUtf8(yytext);

      int s = tag.indexOf("id=");

      if (s != -1) {
         // command has id attribute
         QChar c = tag[s + 3];

         if (c == '\'' || c == '"') {
            // valid start
            int e = tag.indexOf(c,s + 4);

            if (e != -1) {
               // found matching end

               QString id = tag.mid(s + 4, e-s-4);   // extract id
               addAnchor(id);
            }
         }
      }

      addToOutput(tag);
   }

<Comment>"<"{PRE}{ATTR}">"    {
      insidePre = true;
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>"</"{PRE}">"         {
      insidePre = false;
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>{RCSTAG}             {
      // RCS tag which end a brief description
      setOutput(OutputMainDoc);
      REJECT;
   }

<Comment>"<!--"               {
      BEGIN(HtmlComment);
   }

<Comment>{B}*{CMD}"endinternal"{B}*    {
      addToOutput("\\endinternal ");

      if (! s_processInternalDocs) {
         warn(yyFileName, yyLineNr, "Found \\endinternal without matching \\internal");
      }

      s_processInternalDocs = false;
   }

<Comment>{B}*{CMD}[a-z_A-Z]+{B}*    {
      // might be a valid command
      // the {B}* in the front was added for bug620924

      QString text = QString::fromUtf8(yytext);

      QString cmdName = text.trimmed().mid(1);
      DocCmdMapper::Cmd *cmdPtr = DocCmdMapper::map(cmdName);

      if (cmdPtr) {
         // special action is required

         int i = 0;
         while (text[i] == ' ' || text[i] == '\t') {
            i++;
         }

         s_spaceBeforeCmd = QString(text).left(i);

         if (cmdPtr->endsBrief && ! (inContext == OutputXRef && cmdName == "parblock")) {
            briefEndsAtDot = false;

            // this command forces the end of brief description
            setOutput(OutputMainDoc);
         }

         if (cmdPtr->func && cmdPtr->func(cmdName)) {
            // implicit split of the comment block into two entries
            // restart the next block at the start of this command

            s_parseMore = true;

            s_inputPosition = s_prevPosition + (yy_bp - s_bufferPosition);
            yyterminate();

         } else if (cmdPtr->func == nullptr) {
            // command without handler is processed later by parserdoc.cpp
            addToOutput(text);
         }

      } else {
         // command not relevant
         addToOutput(text);
      }
   }

<Comment>{B}*("\\\\"|"@@")"f"[$\[{]    {
      // escaped formula command
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>{B}*{CMD}"~"[a-z_A-Z-]*       {
      // language switch command
      static const QString outputLanguage = Config::getEnum("output-language");

      QString text   = QString::fromUtf8(yytext);
      QString langId = text.trimmed().mid(2);

      if (! langId.isEmpty() && outputLanguage.compare(langId, Qt::CaseInsensitive) != 0) {
         // enable language specific section
         BEGIN(SkipLang);
      }
   }

<Comment>{B}*{CMD}"f{"[^}\n]+"}"("{"?)  {
      // start of a formula with custom environment
      QString text = QString::fromUtf8(yytext);

      formulaText = "\\begin";
      formulaEnv = text.trimmed().mid(2);

      if (formulaEnv.at(formulaEnv.length() - 1) == '{') {
         // remove trailing open brace
         formulaEnv = formulaEnv.left(formulaEnv.length() - 1);
      }

      formulaText += formulaEnv;
      formulaNewLines = 0;
      BEGIN(ReadFormulaLong);
   }

<Comment>{B}*{CMD}"f$"        {
      // start of a inline formula
      formulaText     = "$";
      formulaNewLines = 0;
      BEGIN(ReadFormulaShort);
   }

<Comment>{B}*{CMD}"f["        {
      // start of a block formula
      formulaText      = "\\[";
      formulaNewLines  = 0;
      BEGIN(ReadFormulaLong);
   }

<Comment>{B}*{CMD}"{"        {
      // beginning of a group
      openGroup(current, yyFileName, yyLineNr);
   }

<Comment>{B}*{CMD}"}"        {
      // end of a group
      QString text = QString::fromUtf8(yytext);

      closeGroup(current, yyFileName, yyLineNr, true);

      s_memberGroupHeader.clear();
      s_parseMore    = true;
      s_needNewEntry = true;

      s_inputPosition = s_prevPosition + (yy_bp - s_bufferPosition) + text.length();
      yyterminate();
   }

<Comment>{B}*{CMD}[$@\\&~<>#%]      {
      // escaped character
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>[a-z_A-Z]*[a-zA-Z]       {
      // normal word
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>^{B}*"."{B}*/\n                {
      // explicit end autolist: e.g "  ."
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>^{B}*[1-9][0-9]*"."{B}+        |
<Comment>^{B}*[*+]{B}+        {
      // start of autolist
      if (! Doxy_Globals::markdownSupport) {
         REJECT;

      } else {

         if (inContext != OutputXRef) {
            briefEndsAtDot = false;
            setOutput(OutputMainDoc);
         }

         QString text = QString::fromUtf8(yytext);
         addToOutput(text);
      }
   }

<Comment>^{B}*"-"{B}+         {
      // start of autolist
      if (inContext != OutputXRef) {
         briefEndsAtDot = false;
         setOutput(OutputMainDoc);
      }

      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>^{B}*([\-:|]{B}*)*("--"|"---")({B}*[\-:|])*{B}*/\n {
      // horizontal line (dashed)
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>{CMD}"---"       {
      // escaped mdash
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>{CMD}"--"        {
      // escaped mdash
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>"---"            {
      // mdash
      QString text = QString::fromUtf8(yytext);
      addToOutput(insidePre || Doxy_Globals::markdownSupport ? text : "&mdash;");
   }

<Comment>"--"                           {
      // ndash
      QString text = QString::fromUtf8(yytext);
      addToOutput(insidePre || Doxy_Globals::markdownSupport ? text : "&ndash;");
   }

<Comment>"-#"{B}+                       {
      // numbered item
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>("."+)[a-z_A-Z0-9\)]       {
      // . at start or in the middle of a word, or ellipsis
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>".\\"[ \t]        {
      // . with escaped space
      QString text = QString::fromUtf8(yytext);

      addToOutput(text[0]);
      addToOutput(text[2]);
   }

<Comment>".,"           {
      // . with comma such as "e.g.,"
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>"...\\"[ \t]         {
      // ellipsis with escaped space
      addToOutput("... ");
   }

<Comment>".."[\.]?/[^ \t\n]      {
      // internal ellipsis
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>(\n|\\_linebr)({B}*(\n|\\_linebr))+    {
      // at least one blank line (or blank line command)

      QString text = QString::fromUtf8(yytext);

      if (inContext == OutputXRef) {
         // see bug 613024, need to put the newlines after ending the XRef section.

         if (! s_insideParBlock) {
            setOutput(OutputMainDoc);
         }

         for (int i = 0; i < text.length();  ) {
            if (text[i] == '\n') {
               addToOutput('\n');
               i++;

            } else if (text.mid(i) == "\\_linebr")  {
               addToOutput('\n');
               i += 8;

            } else  {
               i++;
            }
         }

      } else if (inContext != OutputBrief) {

         for (int i = 0; i< text.length(); ) {
            if (text[i] == '\n') {
               addToOutput('\n');
               i++;

            } else if (text.mid(i) == "\\_linebr")  {
                addToOutput('\n');
                i += 8;

            } else  {
               i++;

            }
         }

         setOutput(OutputMainDoc);

      } else {
          // inContext == OutputBrief
          // only go to the detailed description if we have
          // found some brief description and not just whitespace

         endBrief(false);
      }

      lineCount();
   }

<Comment>"."            {
      // potential end of a JavaDoc style comment
      QString text = QString::fromUtf8(yytext);
      addToOutput(text[0]);

      if (briefEndsAtDot) {
         setOutput(OutputMainDoc);
         briefEndsAtDot = false;
      }
   }


<Comment>\n             {
      // newline
      addToOutput('\n');
      yyLineNr++;
   }


<Comment>[\xC0-\xFF][\x80-\xBF]+    {
      // utf-8 code point
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }


<Comment>.    {
      // catch all for anything else
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

 /* --------------   Rules for handling HTML comments ----------- */

<HtmlComment>"--"[!]?">"{B}*     {
      BEGIN( Comment );
   }

<HtmlComment>{DOCNL}          {
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '\n') {
         yyLineNr++;
      }
   }

<HtmlComment>[^\\\n\-]+          {
      // ignore unimportant characters
   }

<HtmlComment>.             {
      // ignore every else
   }


 /* --------------   Rules for handling formulas ---------------- */

<ReadFormulaShort>{CMD}"f$"      {
      // end of inline formula
      formulaText += "$";
      addToOutput(" " + addFormula());
      BEGIN(Comment);
   }

<ReadFormulaLong>{CMD}"f]"       {
      // end of block formula
      formulaText += "\\]";
      addToOutput(" " + addFormula());
      BEGIN(Comment);
   }

<ReadFormulaLong>{CMD}"f}"       {
      // end of custom env formula
      formulaText += "\\end";
      formulaText += formulaEnv;
      addToOutput(" " + addFormula());
      BEGIN(Comment);
   }

<ReadFormulaLong,ReadFormulaShort>[^\\@\n]+ {
      // any non-special character
      QString text = QString::fromUtf8(yytext);
      formulaText += text;
   }

<ReadFormulaLong,ReadFormulaShort>\n   {
      // new line
      QString text = QString::fromUtf8(yytext);

      formulaNewLines++;
      formulaText += text[0];
      yyLineNr++;
   }

<ReadFormulaLong,ReadFormulaShort>.     {
      // any othe character
      QString text = QString::fromUtf8(yytext);
      formulaText += text[0];
   }


  /* ------------ handle argument of enum command --------------- */

<EnumDocArg1>{SCOPEID}        {
      // handle argument
      QString text = QString::fromUtf8(yytext);
      current->m_entryName = text;
      BEGIN( Comment );
   }

<EnumDocArg1>{LC}          {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<EnumDocArg1>{DOCNL}          {
      // missing argument
      QString text = QString::fromUtf8(yytext);

      warn(yyFileName, yyLineNr, "Missing argument after \\enum");

      addToOutput('\n');
      if (text[0] == '\n') {
         yyLineNr++;
      }

      BEGIN( Comment );
   }

<EnumDocArg1>.             {
      // ignore other stuff
   }


  /* ------------ handle argument of namespace command --------------- */

<NameSpaceDocArg1>{SCOPENAME}       {
      // handle argument
      QString text  = QString::fromUtf8(yytext);
      current->m_entryName = substitute(text,".", "::");
      BEGIN( Comment );
   }

<NameSpaceDocArg1>{LC}        {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<NameSpaceDocArg1>{DOCNL}     {
      // missing argument
      QString text = QString::fromUtf8(yytext);
      warn(yyFileName, yyLineNr, "Missing argument after \\namespace");

      addToOutput('\n');
      if (text[0] == '\n') {
         yyLineNr++;
      }
      BEGIN( Comment );
   }

<NameSpaceDocArg1>.        {
      // ignore other stuff
   }


  /* ------------ handle argument of package command --------------- */

<PackageDocArg1>{ID}("."{ID})*      {
      // handle argument
      QString text = QString::fromUtf8(yytext);
      current->m_entryName = text;
      BEGIN( Comment );
   }

<PackageDocArg1>{LC}          {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<PackageDocArg1>{DOCNL}          {
      // missing argument
      QString text = QString::fromUtf8(yytext);

      warn(yyFileName, yyLineNr, "Missing argument after \\package");
      addToOutput('\n');

      if (text[0] == '\n') {
         yyLineNr++;
      }
      BEGIN( Comment );
   }
<PackageDocArg1>.          {
      // ignore other stuff
   }


  /* ------ handle argument of class/struct/union command --------------- */

<ClassDocArg1>{SCOPENAME}{TMPLSPEC}     {
      QString text = QString::fromUtf8(yytext);
      current->m_entryName = substitute(removeRedundantWhiteSpace(text),".","::");
      BEGIN( ClassDocArg2 );
   }

<ClassDocArg1>{SCOPENAME}     {
      // first argument
      QString text = QString::fromUtf8(yytext);

      current->m_entryName = substitute(text,".","::");
      if (current->section == Entry::PROTOCOLDOC_SEC) {
         current->m_entryName += "-p";
      }

      // prepend outer scope name
      BEGIN( ClassDocArg2 );
   }

<CategoryDocArg1>{SCOPENAME}{B}*"("[^\)]+")" {
      QString text  = QString::fromUtf8(yytext);
      current->m_entryName = substitute(text,".","::");
      BEGIN( ClassDocArg2 );
   }

<ClassDocArg1,CategoryDocArg1>{LC}      {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<ClassDocArg1,CategoryDocArg1>{DOCNL}  {
      // missing argument
      QString text = QString::fromUtf8(yytext);

      warn(yyFileName, yyLineNr, "Missing argument after \\%s",
                  YY_START == ClassDocArg1 ? "class" : "category" );

      addToOutput('\n');

      if (text[0] == '\n') {
         yyLineNr++;
      }
      BEGIN( Comment );
   }

<ClassDocArg1,CategoryDocArg1>.     {
      // ignore other stuff
   }

<ClassDocArg2>{FILE}|"<>"     {
      // second argument, include file
      QString text = QString::fromUtf8(yytext);
      current->setData(EntryKey::Include_File, text);
      BEGIN( ClassDocArg3 );
   }

<ClassDocArg2>{LC}         {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<ClassDocArg2>{DOCNL}         {
      QString text = QString::fromUtf8(yytext);

      addToOutput('\n');

      if (text[0] == '\n')  {
         yyLineNr++;
      }
      BEGIN( Comment );
   }

<ClassDocArg2>.            {
      // ignore other stuff
   }


<ClassDocArg3>[<"]?{FILE}?[">]?     {
      // third argument, include file name
      QString text = QString::fromUtf8(yytext);
      current->setData(EntryKey::Include_Name, text);
      BEGIN( Comment );
   }

<ClassDocArg3>{LC}         {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<ClassDocArg3>{DOCNL}         {
      QString text = QString::fromUtf8(yytext);

      if (text[0] =='\n') {
         yyLineNr++;
      }

      BEGIN( Comment );
   }

<ClassDocArg3>.            {
      // ignore other stuff
   }


  /* --------- handle arguments of {def,add,weak} group commands --------- */

<GroupDocArg1>{LABELID}(".html"?)   {
      // group name
      QString text  = QString::fromUtf8(yytext);
      current->m_entryName = text;

      // lastDefGroup.groupname = text;
      // lastDefGroup.pri = current->groupingPri();
      // the .html stuff is for Qt compatibility

      if (current->m_entryName.endsWith(".html")) {
         current->m_entryName = current->m_entryName.left(current->m_entryName.length() - 5);
      }

      current->setData(EntryKey::Member_Type, "");
      BEGIN(GroupDocArg2);
   }

<GroupDocArg1>"\\"{B}*"\n"       {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<GroupDocArg1>{DOCNL}         {
      // missing argument
      QString text = QString::fromUtf8(yytext);

      warn(yyFileName, yyLineNr, "Missing group name after %s", csPrintable(current->groupDocCmd()) );

      addToOutput('\n');
      if (text[0] == '\n') {
         yyLineNr++;
      }
      BEGIN( Comment );
   }

<GroupDocArg2>"\\"{B}*"\n"       {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<GroupDocArg2>[^\n\\\*]+      {
      // title (stored in type)
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Type, text.trimmed());
   }

<GroupDocArg2>{DOCNL}         {
      QString text = QString::fromUtf8(yytext);

      if ( current->groupDocType == Entry::GROUPDOC_NORMAL && current->getData(EntryKey::Member_Type).isEmpty()) {
            // defgroup requires second argument
            warn(yyFileName, yyLineNr, "Missing title after \\defgroup %s", csPrintable(current->m_entryName) );
      }

      if (text[0] == '\n') {
         yyLineNr++;
      }

      addToOutput('\n');
      BEGIN( Comment );
   }


  /* --------- handle arguments of page/mainpage command ------------------- */

<PageDocArg1>{FILE}        {
      // first argument; page name
      QString text  = QString::fromUtf8(yytext);
      current->m_entryName = stripQuotes(text);
      BEGIN( PageDocArg2 );
   }

<PageDocArg1>{LC}          {
      yyLineNr++;
      addToOutput('\n');
   }

<PageDocArg1>{DOCNL}          {
      // missing arguement
      QString text = QString::fromUtf8(yytext);
      warn(yyFileName, yyLineNr, "Missing argument after \\page");

      if (text[0] == '\n') {
         yyLineNr++;
      }

      addToOutput('\n');
      BEGIN( Comment );
   }

<PageDocArg1>.             {
      // ignore other stuff
   }

<PageDocArg2>.*"\n"        {
      // second argument; page title
      QString text = QString::fromUtf8(yytext);

      yyLineNr++;
      current->setData(EntryKey::Member_Args, text);

      addToOutput('\n');
      BEGIN( Comment );
   }

  /* --------- handle arguments of the param command ------------ */
<ParamArg1>{ID}/{B}*","   {
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '_' && Doxy_Globals::markdownSupport) {
         addToOutput('\\');
      }

      addToOutput(text);
   }

<ParamArg1>","   {
      addToOutput(" , ");
   }

<ParamArg1>{ID}  {
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '_' && Doxy_Globals::markdownSupport) {
         addToOutput('\\');
      }

      addToOutput(text);
      BEGIN( Comment );
   }

<ParamArg1>.    {
      unput(yytext[0]);
      BEGIN( Comment );
   }

  /* --------- handle arguments of the file/dir/example command ------------ */

<FileDocArg1>{DOCNL}          {
      // no file name specified
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '\n') {
         yyLineNr++;
      }

      addToOutput('\n');
      BEGIN( Comment );
   }

<FileDocArg1>{FILE}        {
      // first argument; name
      QString text = QString::fromUtf8(yytext);
      current->m_entryName = stripQuotes(text);
      BEGIN( Comment );
   }

<FileDocArg1>{LC}          {
      yyLineNr++;
      addToOutput('\n');
   }

<FileDocArg1>.             {
      // ignore other stuff
   }


  /* --------- handle arguments of the xrefitem command ------------ */

<XRefItemParam1>{LABELID}     {
      // first argument
      QString text = QString::fromUtf8(yytext);

      newXRefItemKey = text;
      setOutput(OutputXRef);

      BEGIN(XRefItemParam2);
   }

<XRefItemParam1>{LC}          {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<XRefItemParam1>{DOCNL}       {
      // missing arguments
      QString text = QString::fromUtf8(yytext);
      warn(yyFileName, yyLineNr, "Missing first argument of \\xrefitem");

      if (text[0] == '\n') {
         yyLineNr++;
      }

      addToOutput('\n');
      inContext = OutputMainDoc;
      BEGIN( Comment );
   }

<XRefItemParam1>.          {
      // ignore other stuff
   }

<XRefItemParam2>"\""[^\n\"]*"\""    {
      // second argument
      QString text = QString::fromUtf8(yytext);
      xrefItemTitle = stripQuotes(text);
      BEGIN(XRefItemParam3);
   }

<XRefItemParam2>{LC}          {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<XRefItemParam2>{DOCNL}          {
      // missing argument
      QString text = QString::fromUtf8(yytext);
      warn(yyFileName, yyLineNr, "Missing second argument of \\xrefitem");

      if (text[0] == '\n') {
         yyLineNr++;
      }

      addToOutput('\n');
      inContext = OutputMainDoc;
      BEGIN( Comment );
   }

<XRefItemParam2>.          {
      // ignore other stuff
   }

<XRefItemParam3>"\""[^\n\"]*"\""    {
      // third argument
      QString text = QString::fromUtf8(yytext);

      xrefListTitle = stripQuotes(text);
      xrefKind = XRef_Item;

      BEGIN( Comment );
   }

<XRefItemParam2,XRefItemParam3>{LC}    {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<XRefItemParam3>{DOCNL}          {
      // missing argument
      QString text = QString::fromUtf8(yytext);
      warn(yyFileName, yyLineNr,"Missing third argument of \\xrefitem");

      if (text[0] == '\n') {
         yyLineNr++;
      }

      addToOutput('\n');
      inContext = OutputMainDoc;
      BEGIN( Comment );
   }

<XRefItemParam3>.          {
      // ignore other stuff
   }

  /* ----- handle arguments of the relates(also)/memberof command ------- */

<RelatesParam1>({ID}("::"|"."))*{ID}   {
      // argument
      QString text = QString::fromUtf8(yytext);

      current->setData(EntryKey::Related_Class, text);
      BEGIN( Comment );
   }

<RelatesParam1>{LC}        {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<RelatesParam1>{DOCNL}        {
      // missing argument
      QString text = QString::fromUtf8(yytext);
      warn(yyFileName, yyLineNr, "Missing argument of \\relates or \\memberof command");

      if (text[0] == '\n') {
         yyLineNr++;
      }

      addToOutput('\n');
      BEGIN( Comment );
   }

<RelatesParam1>.        {
      // ignore other stuff
   }


  /* ----- handle arguments of the relates(also)/addindex commands ----- */

<LineParam>{DOCNL}         {
      // end of argument
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '\n')  {
         yyLineNr++;
      }

      addToOutput('\n');
      BEGIN( Comment );
   }

<LineParam>{LC}            {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<LineParam>.            {
      // ignore other stuff
      QString text = QString::fromUtf8(yytext);
      addToOutput(text[0]);
   }


  /* ----- handle arguments of the section/subsection/.. commands ------- */

<SectionLabel>{LABELID}          {
      // first argyment
      QString text = QString::fromUtf8(yytext);

      s_sectionLabel = text;
      addToOutput(text);
      s_sectionTitle.resize(0);

      BEGIN(SectionTitle);
   }

<SectionLabel>{DOCNL}         {
      // missing argument
      QString text = QString::fromUtf8(yytext);
      warn(yyFileName, yyLineNr, "\\section command has no label");

      if (text[0] == '\n') {
         yyLineNr++;
      }

      addToOutput('\n');
      BEGIN( Comment );
   }

<SectionLabel>.            {
      // invalid character for section label
      warn(yyFileName, yyLineNr, "Invalid or missing section label");
      BEGIN(Comment);
   }

<SectionTitle>[^\n@\\*]*/"\n"           {
      // end of section title
      QString text = QString::fromUtf8(yytext);
      addSection();
      addToOutput(text);
      BEGIN( Comment );
   }

<SectionTitle>[^\n@\\]*/"\\_linebr"     {
      // end of section title
      QString text = QString::fromUtf8(yytext);
      addSection();
      addToOutput(text);
      BEGIN( Comment );
   }

<SectionTitle>{LC}         {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<SectionTitle>[^\n@\\]*          {
      // any character without special meaning
      QString text = QString::fromUtf8(yytext);
      s_sectionTitle += text;
      addToOutput(text);
   }

<SectionTitle>("\\\\"|"@@"){ID}     {
      // unescape escaped command
      QString text = QString::fromUtf8(yytext);

      s_sectionTitle += text.mid(1);
      addToOutput(text);
   }

<SectionTitle>{CMD}[$@\\&~<>#%]     {
      // unescape escaped character
      QString text = QString::fromUtf8(yytext);

      s_sectionTitle += text[1];
      addToOutput(text);
   }

<SectionTitle>.            {
      // anything else
      QString text = QString::fromUtf8(yytext);

      s_sectionTitle += text;
      addToOutput(text[0]);
   }

  /* ----- handle arguments of the subpage command ------- */

<SubpageLabel>{LABELID}          {
      // first argument
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);

      // we add subpage labels as a kind of "inheritance" relation to prevent
      // needing to add another list to the Entry class.

      current->extends.append(BaseInfo(text, Public, Normal));
      BEGIN(SubpageTitle);
   }

<SubpageLabel>{DOCNL}         {
      // missing argument
      QString text = QString::fromUtf8(yytext);
      warn(yyFileName, yyLineNr, "\\subpage command has no label");

      if (text[0] == '\n') {
         yyLineNr++;
      }

      addToOutput('\n');
      BEGIN( Comment );
   }

<SubpageTitle>{DOCNL}         {
      // no title, end command
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
      BEGIN( Comment );
   }

<SubpageTitle>[ \t]*"\""[^\"\n]*"\""   {
      // add title, end of command
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
      BEGIN( Comment );
   }

<SubpageTitle>.            {
      // no title, end of command
      unput(yytext[0]);
      BEGIN( Comment );
   }


  /* ----- handle arguments of the anchor command ------- */

<AnchorLabel>{LABELID}        {
      // found argument
      QString text = QString::fromUtf8(yytext);

      addAnchor(text);
      addToOutput(text);

      BEGIN( Comment );
   }

<AnchorLabel>{DOCNL}          {
      // missing argument
      QString text = QString::fromUtf8(yytext);
      warn(yyFileName, yyLineNr, "\\anchor command has no label");

      if (text[0] == '\n') {
         yyLineNr++;
      }

      addToOutput('\n');
      BEGIN( Comment );
   }

<AnchorLabel>.             {
      // invalid character for anchor label
      warn(yyFileName, yyLineNr, "Invalid or missing anchor label");
      BEGIN(Comment);
   }


  /* ----- handle arguments of the preformatted block commands ------- */

<FormatBlock>{CMD}("endverbatim"|"endlatexonly"|"endhtmlonly"|"endxmlonly"|"enddocbookonly"|"endrtfonly"|"endmanonly"|"enddot"|"endcode"|"endmsc"|"endvhdlflow")/{NW} {
      // possible ends
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);

      if (text.mid(4) == blockName)  {
         // found end of the block
         BEGIN(Comment);
      }
   }

<FormatBlock>{CMD}"enduml"              {
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);

      if (blockName == "startuml")   {
         // found end of the block
         BEGIN(Comment);
      }
   }

<FormatBlock>[^ \@\*\/\\\n]*     {
      // some word
      QString text = QString::fromUtf8(yytext);

      addToOutput(text);
   }

<FormatBlock>{DOCNL}          {
      // new line
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '\n') {
         yyLineNr++;
      }

      addToOutput('\n');
   }

<FormatBlock>"/*"          {
      // */ (editor syntax fix)
      // start of a C-comment
      QString text = QString::fromUtf8(yytext);

      s_commentCount++;
      addToOutput(text);
   }

<FormatBlock>"*/"          {
      // end of a C-comment
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);

      s_commentCount--;

      if (s_commentCount < 0 && blockName != "verbatim") {
         warn(yyFileName, yyLineNr, "Found */ without matching /* while inside a \\%s block. "
            "Perhaps there is a missing \\end%s?\n", csPrintable(blockName), csPrintable(blockName));
      }
   }

<FormatBlock>.             {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);
      addToOutput(text[0]);
   }

<FormatBlock><<EOF>>          {
      QString endTag = "@end" + blockName;

      if (blockName == "startuml") {
         endTag = "enduml";
      }

      warn(yyFileName, yyLineNr, "Reached end of comment while inside a @%s block, check for missing @%s tag",
                 csPrintable(blockName), csPrintable(endTag) );

      yyterminate();
   }


  /* ----- handle arguments of if/ifnot commands ------- */

<GuardParam>{B}*"("                     {
      QString text = QString::fromUtf8(yytext);
      s_guardExpr  = text;
      s_roundCount = 1;
      BEGIN(GuardExpr);
   }

<GuardExpr>[^()]*                       {
      QString text = QString::fromUtf8(yytext);
      s_guardExpr += text;
   }

<GuardExpr>"("                          {
      QString text = QString::fromUtf8(yytext);
      s_guardExpr += text;
      s_roundCount++;
   }

<GuardExpr>")"                          {
      QString text = QString::fromUtf8(yytext);
      s_guardExpr += text;
      s_roundCount--;

      if (s_roundCount == 0) {
         handleGuard(s_guardExpr);
      }
   }

<GuardExpr>\n                           {
      warn(yyFileName, yyLineNr, "Invalid expression '%s' for guard", csPrintable(s_guardExpr));
      unput(yytext[0]);
      BEGIN(GuardParam);
   }

<GuardParam>{B}*[a-z_A-Z0-9.\-]+            {
      // parameter of if/ifnot guard
      QString text = QString::fromUtf8(yytext);
      handleGuard(text);
   }

<GuardParam>{DOCNL}        {
      // end of argument
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '\n') {
         yyLineNr++;
      }

      // next line is commented out due to bug620924
      // addToOutput('\n');

      BEGIN( Comment );
   }

<GuardParam>{LC}        {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<GuardParam>.           {
      // ignore other stuff
      QString text = QString::fromUtf8(yytext);
      addToOutput(text[0]);
   }

<GuardParamEnd>{B}*{DOCNL}       {
      s_spaceBeforeIf.resize(0);
      BEGIN(Comment);
   }

<GuardParamEnd>{B}*        {
      if (! s_spaceBeforeIf.isEmpty()) {
         // needed for 665313 in combation with bug620924

         addToOutput(s_spaceBeforeIf);
      }

      s_spaceBeforeIf.resize(0);
      BEGIN(Comment);
   }

<GuardParamEnd>.        {
      unput(yytext[0]);
      BEGIN(Comment);
   }

  /* ----- handle skipping of conditional sections ------- */

<SkipGuardedSection>{CMD}"ifnot"/{NW}  {
      guardType = Guard_IfNot;
      BEGIN( GuardParam );
   }

<SkipGuardedSection>{CMD}"if"/{NW}  {
      guardType = Guard_If;
      BEGIN( GuardParam );
   }

<SkipGuardedSection>{CMD}"endif"/{NW}  {
      if (s_guards.isEmpty()) {
         warn(yyFileName,yyLineNr, "Found @endif without matching start command");

       } else {
         GuardedSection s   = s_guards.pop();
         bool parentVisible = s.parentVisible();

         if (parentVisible) {
            enabledSectionFound = true;
            BEGIN( GuardParamEnd );
         }
      }
   }

<SkipGuardedSection>{CMD}"else"/{NW}   {
      if (s_guards.isEmpty()) {
         warn(yyFileName, yyLineNr, "Found @else without matching start command");

      } else {

         if (! enabledSectionFound && s_guards.top().parentVisible()) {
            s_guards.pop();
            s_guards.push(GuardedSection(true,true));

            enabledSectionFound = true;
            BEGIN( GuardParamEnd );
         }
      }
   }

<SkipGuardedSection>{CMD}"elseif"/{NW}  {
      if (s_guards.isEmpty()) {
         warn(yyFileName,yyLineNr, "Found @elseif without matching start command");

       } else {

         if (! enabledSectionFound && s_guards.top().parentVisible()) {
            guardType = Guard_If;
            s_guards.pop();

            BEGIN( GuardParam );
         }
      }
   }

<SkipGuardedSection>{DOCNL}      {
      // skip line
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '\n') {
         yyLineNr++;
      }

      // addToOutput('\n');
   }

<SkipGuardedSection>[^ \\@\n]+      {
      // skip non-special characters
   }

<SkipGuardedSection>.         {
      // any other character
   }


  /* ----- handle skipping of internal section ------- */

<SkipInternal>{DOCNL}         {
      // skip line
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '\n')  {
         yyLineNr++;
      }

      addToOutput('\n');
   }

<SkipInternal>[@\\]"if"/[ \t]       {
      s_condCount++;
   }

<SkipInternal>[@\\]"ifnot"/[ \t]    {
      s_condCount++;
   }

<SkipInternal>[@\\]/"endif"      {
      s_condCount--;

      if (s_condCount < 0 )   {
         // handle conditional section around \internal, see bug607743

         unput('\\');
         BEGIN(Comment);
      }
   }

<SkipInternal>[@\\]/"section"[ \t]  {
      if (s_sectionLevel > 0 ) {
         unput('\\');
         BEGIN(Comment);
      }
   }

<SkipInternal>[@\\]/"subsection"[ \t]  {
      if (s_sectionLevel > 1)  {
         unput('\\');
         BEGIN(Comment);
      }
   }

<SkipInternal>[@\\]/"subsubsection"[ \t]  {
      if (s_sectionLevel > 2) {
         unput('\\');
         BEGIN(Comment);
      }
   }

<SkipInternal>[@\\]/"paragraph"[ \t]   {
      if (s_sectionLevel > 3) {
         unput('\\');
         BEGIN(Comment);
      }
   }

<SkipInternal>[@\\]"endinternal"[ \t]*  {
      addToOutput("\\endinternal ");
      BEGIN(Comment);
   }

<SkipInternal>[^ \\@\n]+      {
      // skip non-special characters
   }

<SkipInternal>.            {
      // any other character
   }

  /* ----- handle argument of name command ------- */

<NameParam>{DOCNL}         {
      // end of argument
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '\n') {
         yyLineNr++;
      }

      addToOutput('\n');
      BEGIN( Comment );
   }

<NameParam>{LC}            {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
      s_memberGroupHeader+=' ';
   }

<NameParam>.            {
      // ignore other stuff
      QString text = QString::fromUtf8(yytext);
      s_memberGroupHeader  += text[0];
      current->m_entryName += text[0];
   }


  /* ----- handle argument of ingroup command ------- */

<InGroupParam>{LABELID}          {
      // group id
      QString text = QString::fromUtf8(yytext);

      current->m_groups.append(Grouping(text, Grouping::GROUPING_INGROUP) );
      inGroupParamFound = true;
   }

<InGroupParam>{DOCNL}         {
      // missing argument
      QString text = QString::fromUtf8(yytext);

      if (! inGroupParamFound) {
         warn(yyFileName,yyLineNr, "Missing group name for \\ingroup command");
      }

      if (text[0] == '\n')   {
         yyLineNr++;
      }

      addToOutput('\n');
      BEGIN( Comment );
   }

<InGroupParam>{LC}         {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
   }

<InGroupParam>.            {
      // ignore other stuff
      QString text = QString::fromUtf8(yytext);
      addToOutput(text[0]);
   }


  /* ----- handle argument of fn command ------- */

<FnParam>{DOCNL}        {
      // end of fn declaration, pass to langParser as a "prototype"
      QString text = QString::fromUtf8(yytext);

      if (braceCount == 0) {
         if (text[0] == '\n')   {
            yyLineNr++;
         }

         addToOutput('\n');

         if (langParser != nullptr) {
            // not used for clang parsing
            langParser->parsePrototype(functionProto);
         }

         BEGIN( Comment );
      }
   }

<FnParam>{LC}           {
      // line continuation
      yyLineNr++;
      functionProto += ' ';
   }

<FnParam>[^@\\\n()]+          {
      // non-special characters
      QString text = QString::fromUtf8(yytext);
      functionProto += text;
   }

<FnParam>"("            {
      QString text = QString::fromUtf8(yytext);
      functionProto += text;
      braceCount++;
   }

<FnParam>")"            {
      QString text = QString::fromUtf8(yytext);
      functionProto += text;
      braceCount--;
   }

<FnParam>.           {
      // add other stuff
      QString text = QString::fromUtf8(yytext);
      functionProto += text[0];
   }


  /* ----- handle argument of overload command ------- */

<OverloadParam>{DOCNL}        {
      // end of overload declaration, pass to langParser as a "prototype"
      QString text = QString::fromUtf8(yytext);
      if (text[0] == '\n')  {
         yyLineNr++;
      }

      if (functionProto.trimmed().isEmpty()) {
         // plain overload command
         addToOutput(theTranslator->trOverloadText());
         addToOutput('\n');

      }  else   {
         // overload declaration
         makeStructuralIndicator(Entry::OVERLOADDOC_SEC);

         if (langParser != nullptr) {
            // not used for clang parsing
            langParser->parsePrototype(functionProto);
         }
      }

      BEGIN( Comment );
   }

<OverloadParam>{LC}        {
      // line continuation
      yyLineNr++;
      functionProto += ' ';
   }

<OverloadParam>.        {
      // add other stuff
      QString text = QString::fromUtf8(yytext);
      functionProto += text[0];
   }


  /* ----- handle argument of inherit command ------- */

<InheritParam>({ID}("::"|"."))*{ID}    {
      // found argument
      QString text = QString::fromUtf8(yytext);

      current->extends.append(BaseInfo(removeRedundantWhiteSpace(text), Public, Normal));
      BEGIN( Comment );
   }

<InheritParam>{DOCNL}         {
      // missing argument
      QString text = QString::fromUtf8(yytext);

      warn(yyFileName, yyLineNr, "\\inherit command has no argument");

      if (text[0] == '\n')  {
         yyLineNr++;
      }

      addToOutput('\n');
      BEGIN( Comment );
   }

<InheritParam>.            {
      // invalid character for anchor label
      warn(yyFileName, yyLineNr, "Invalid or missing name for \\inherit command");
      BEGIN(Comment);
   }


  /* ----- handle argument of extends and implements commands ------- */

<ExtendsParam>({ID}("::"|"."))*{ID}    {
      // found argument
      QString text = QString::fromUtf8(yytext);
      current->extends.append(BaseInfo(removeRedundantWhiteSpace(text),Public,Normal));
      BEGIN( Comment );
   }

<ExtendsParam>{DOCNL}         {
      // missing argument
      QString text = QString::fromUtf8(yytext);
      warn(yyFileName,yyLineNr, "\\extends or \\implements command has no argument");

      if (text[0] == '\n')   {
         yyLineNr++;
      }

      addToOutput('\n');
      BEGIN( Comment );
   }

<ExtendsParam>.            {
      // ignore other stuff
   }

  /* ----- handle language specific sections ------- */

<SkipLang>[\\@]"~"[a-zA-Z-]*        {
      /* language switch */
      QString text = QString::fromUtf8(yytext);
      QString langId = text.mid(2);

      if (langId.isEmpty() || Config::getEnum("output-language").compare(langId, Qt::CaseInsensitive) == 0) {
         // enable language specific section
         BEGIN(Comment);
      }
   }

<SkipLang>[^*@\\\n]*          {
      /* any character not a *, @, backslash or new line */
   }

<SkipLang>{DOCNL}          {
      /* new line in verbatim block */
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '\n') {
         yyLineNr++;
      }
   }

<SkipLang>.             {
      /* any other character */
   }


  /* ----- handle arguments of the cite command ------- */

<CiteLabel>{CITEID}        {
      // found argument
      QString text = QString::fromUtf8(yytext);

      addCite();
      addToOutput(text);
      BEGIN(Comment);
   }

<CiteLabel>{DOCNL}         {
      // missing argument
      QString text = QString::fromUtf8(yytext);
      warn(yyFileName, yyLineNr, "\\cite command has no label" );

      if (text[0] == '\n')   {
         yyLineNr++;
      }

      addToOutput('\n');
      BEGIN( Comment );
   }

<CiteLabel>.            {
      // invalid character for cite label
      warn(yyFileName,yyLineNr, "Invalid or missing cite label");
      BEGIN(Comment);
   }


  /* ----- handle argument of the copydoc command ------- */

<CopyDoc><<EOF>>        |
<CopyDoc>{DOCNL}        {
      QString text = QString::fromUtf8(yytext);
      if (text[0] == '\n')   {
         yyLineNr++;
      }

      addToOutput('\n');

      setOutput(OutputMainDoc);
      addToOutput("\\copydetails ");
      addToOutput(s_copyDocArg);

      addToOutput("\n");

      BEGIN(Comment);
   }

<CopyDoc>[^\n\\]+          {
      QString text = QString::fromUtf8(yytext);
      s_copyDocArg += text;
      addToOutput(text);
   }

<CopyDoc>.           {
      QString text = QString::fromUtf8(yytext);
      s_copyDocArg += text;
      addToOutput(text);
   }

%%

static bool handleBrief(const QString &)
{
   setOutput(OutputBrief);
   return false;
}

static bool handleFn(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::MEMBERDOC_SEC);
   functionProto.resize(0);
   braceCount = 0;
   BEGIN(FnParam);

   return stop;
}

static bool handleDef(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::DEFINEDOC_SEC);
   functionProto.resize(0);
   BEGIN(FnParam);
   return stop;
}

static bool handleOverload(const QString &)
{
   functionProto.resize(0);
   BEGIN(OverloadParam);
   return false;
}

static bool handleEnum(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::ENUMDOC_SEC);
   BEGIN(EnumDocArg1);
   return stop;
}

static bool handleDefGroup(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::GROUPDOC_SEC);
   current->groupDocType = Entry::GROUPDOC_NORMAL;
   BEGIN( GroupDocArg1 );

   return stop;
}

static bool handleAddToGroup(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::GROUPDOC_SEC);
   current->groupDocType = Entry::GROUPDOC_ADD;
   BEGIN( GroupDocArg1 );

   return stop;
}

static bool handleWeakGroup(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::GROUPDOC_SEC);
   current->groupDocType = Entry::GROUPDOC_WEAK;
   BEGIN( GroupDocArg1 );
   return stop;
}

static bool handleNamespace(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::NAMESPACEDOC_SEC);
   BEGIN( NameSpaceDocArg1 );
   return stop;
}

static bool handlePackage(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::PACKAGEDOC_SEC);
   BEGIN( PackageDocArg1 );
   return stop;
}

static bool handleClass(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::CLASSDOC_SEC);
   BEGIN( ClassDocArg1 );
   return stop;
}

static bool handleHeaderFile(const QString &)
{
   BEGIN( ClassDocArg2 );
   return false;
}

static bool handleProtocol(const QString &)
{
   // Obj-C protocol
   bool stop = makeStructuralIndicator(Entry::PROTOCOLDOC_SEC);
   BEGIN( ClassDocArg1 );
   return stop;
}

static bool handleCategory(const QString &)
{
   // Obj-C category
   bool stop = makeStructuralIndicator(Entry::CATEGORYDOC_SEC);
   BEGIN( CategoryDocArg1 );
   return stop;
}

static bool handleUnion(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::UNIONDOC_SEC);
   BEGIN( ClassDocArg1 );
   return stop;
}

static bool handleStruct(const QString &)
{
  bool stop=makeStructuralIndicator(Entry::STRUCTDOC_SEC);
  BEGIN( ClassDocArg1 );
  return stop;
}

static bool handleInterface(const QString &)
{
  bool stop = makeStructuralIndicator(Entry::INTERFACEDOC_SEC);
  BEGIN( ClassDocArg1 );
  return stop;
}

static bool handleIdlException(const QString &)
{
  bool stop = makeStructuralIndicator(Entry::EXCEPTIONDOC_SEC);
  BEGIN( ClassDocArg1 );
  return stop;
}

static bool handlePage(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::PAGEDOC_SEC);
   BEGIN( PageDocArg1 );
   return stop;
}

static bool handleMainpage(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::MAINPAGEDOC_SEC);

   if (! stop) {
      current->m_entryName = "mainpage";
   }

   BEGIN( PageDocArg2 );
   return stop;
}

static bool handleFile(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::FILEDOC_SEC);

   if (! stop) {
      current->m_entryName = yyFileName;
   }

   BEGIN( FileDocArg1 );
   return stop;
}

static bool handleParam(const QString &)
{
  // process param and retval arguments to escape leading underscores
  // in case of markdown processing

  addToOutput("@param ");
  BEGIN( ParamArg1 );

  return false;
}

static bool handleRetval(const QString &)
{
  addToOutput("@retval ");
  BEGIN( ParamArg1 );

  return false;
}

static bool handleDir(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::DIRDOC_SEC);

   if (! stop) {
      current->m_entryName = yyFileName;
   }

   BEGIN( FileDocArg1 );
   return stop;
}

static bool handleExample(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::EXAMPLE_SEC);

   if (! stop) {
      current->m_entryName = yyFileName;
   }

   BEGIN( FileDocArg1 );
   return stop;
}

static bool handleDetails(const QString &)
{
   if (inContext != OutputBrief) {
      // treat @details outside brief description as a new paragraph
      addToOutput("\n\n");
   }

   setOutput(OutputMainDoc);
   return false;
}

static bool handleName(const QString &)
{
   bool stop = makeStructuralIndicator(Entry::MEMBERGRP_SEC);

   if (! stop) {
      s_memberGroupHeader.resize(0);
      BEGIN( NameParam );

      if (s_memberGroupId != DOX_NOGROUP) {
         // end of previous member group

         closeGroup(current,yyFileName,yyLineNr,true);
       }
  }

  return stop;
}

static bool handleTodo(const QString &)
{
   newXRefKind = XRef_Todo;
   setOutput(OutputXRef);
   xrefKind = XRef_Todo;
   return false;
}

static bool handleTest(const QString &)
{
   newXRefKind = XRef_Test;
   setOutput(OutputXRef);
   xrefKind = XRef_Test;
   return false;
}

static bool handleBug(const QString &)
{
   newXRefKind = XRef_Bug;
   setOutput(OutputXRef);
   xrefKind = XRef_Bug;
   return false;
}

static bool handleDeprecated(const QString &)
{
   newXRefKind = XRef_Deprecated;
   setOutput(OutputXRef);
   xrefKind = XRef_Deprecated;
   return false;
}

static bool handleXRefItem(const QString &)
{
   newXRefKind = XRef_Item;
   BEGIN(XRefItemParam1);
   return false;
}

static bool handleParBlock(const QString &)
{
   if (s_insideParBlock)   {
      warn(yyFileName,yyLineNr, "Found \\parblock command while already in a parblock");
   }

   if (! s_spaceBeforeCmd.isEmpty()) {
      addToOutput(s_spaceBeforeCmd);
      s_spaceBeforeCmd.resize(0);
   }

   addToOutput("@parblock ");
   s_insideParBlock = true;

   return false;
}

static bool handleEndParBlock(const QString &)
{
   if (! s_insideParBlock) {
     warn(yyFileName,yyLineNr, "Found \\endparblock command without matching \\parblock");
   }

   addToOutput("@endparblock");
   setOutput(OutputMainDoc);    // to end a parblock inside a xrefitem like context
   s_insideParBlock = false;

   return false;
}

static bool handleRelated(const QString &)
{
   BEGIN(RelatesParam1);
   return false;
}

static bool handleRelatedAlso(const QString &)
{
   current->relatesType = Duplicate;
   BEGIN(RelatesParam1);
   return false;
}

static bool handleMemberOf(const QString &)
{
   current->relatesType = MemberOf;
   BEGIN(RelatesParam1);
   return false;
}

static bool handleRefItem(const QString &)
{
   addToOutput("@refitem ");
   BEGIN(LineParam);
   return false;
}

static bool handleSection(const QString &s)
{
   setOutput(OutputMainDoc);
   addToOutput("@"+s+" ");
   BEGIN(SectionLabel);

   if (s == "section") {
      s_sectionLevel = 1;

   } else if (s == "subsection") {
      s_sectionLevel = 2;

   } else if (s == "subsubsection") {
      s_sectionLevel = 3;

   } else if (s == "paragraph") {
      s_sectionLevel = 4;

   }

   return false;
}

static bool handleSubpage(const QString &s)
{
   if (current->section != Entry::EMPTY_SEC && current->section != Entry::PAGEDOC_SEC &&
         current->section != Entry::MAINPAGEDOC_SEC) {

      warn(yyFileName,yyLineNr, "Found \\subpage command in a comment block that is not marked as a page");
  }

  if (! s_spaceBeforeCmd.isEmpty()) {
     addToOutput(s_spaceBeforeCmd);
      s_spaceBeforeCmd.resize(0);
  }

   addToOutput("@"+s+" ");
   BEGIN(SubpageLabel);

   return false;
}

static bool handleAnchor(const QString &s)
{
   addToOutput("@"+s+" ");
   BEGIN(AnchorLabel);
   return false;
}

static bool handleCite(const QString &s)
{
   if (! s_spaceBeforeCmd.isEmpty()) {
     addToOutput(s_spaceBeforeCmd);
     s_spaceBeforeCmd.resize(0);
   }

   addToOutput("@" + s + " ");
   BEGIN(CiteLabel);
   return false;
}

static bool handleFormatBlock(const QString &s)
{
   addToOutput("@" + s + " ");

   blockName      = s;
   s_commentCount = 0;
   BEGIN(FormatBlock);

   return false;
}

static bool handleAddIndex(const QString &)
{
   addToOutput("@addindex ");
   BEGIN(LineParam);
   return false;
}

static bool handleIf(const QString &)
{
   enabledSectionFound = false;
   guardType = Guard_If;
   s_spaceBeforeIf = s_spaceBeforeCmd;
   BEGIN(GuardParam);
   return false;
}

static bool handleIfNot(const QString &)
{
   enabledSectionFound = false;
   guardType = Guard_IfNot;
   s_spaceBeforeIf = s_spaceBeforeCmd;
   BEGIN(GuardParam);
   return false;
}

static bool handleElseIf(const QString &)
{
   if (s_guards.isEmpty()) {
      warn(yyFileName,yyLineNr, "Found \\else without matching start command");

   } else {
      guardType = enabledSectionFound ? Guard_Skip : Guard_If;
      BEGIN(GuardParam);
   }

   return false;
}

static bool handleElse(const QString &)
{
   if (s_guards.isEmpty()) {
      warn(yyFileName,yyLineNr, "Found \\else without matching start command");

   } else {
      BEGIN( SkipGuardedSection );
   }

   return false;
}

static bool handleEndIf(const QString &)
{
   if (s_guards.isEmpty())   {
      warn(yyFileName,yyLineNr, "Found \\endif without matching start command");

   } else {
      s_guards.pop();
   }

   enabledSectionFound = false;
   if (! s_spaceBeforeCmd.isEmpty()) {
      addToOutput(s_spaceBeforeCmd);
      s_spaceBeforeCmd.resize(0);
   }

   BEGIN( GuardParamEnd );
   return false;
}

static bool handleIngroup(const QString &)
{
   inGroupParamFound = false;
   BEGIN( InGroupParam );
   return false;
}

static bool handleNoSubGrouping(const QString &)
{
   current->subGrouping = false;
   return false;
}

static bool handleShowInitializer(const QString &)
{
   current->initLines = 100000;  // ON
   return false;
}

static bool handleHideInitializer(const QString &)
{
   current->initLines = 0;
   return false;
}

static bool handleCallgraph(const QString &)
{
   current->callGraph = true;
   return false;
}

static bool handleHideCallgraph(const QString &)
{
   current->callGraph = false;
   return false;
}

static bool handleCallergraph(const QString &)
{
   current->callerGraph = true;
   return false;
}

static bool handleHideCallergraph(const QString &)
{
   current->callerGraph = false;
   return false;
}

static bool handleInternal(const QString &)
{
   if (s_internalDocs) {
      addToOutput("\\internal ");
      s_processInternalDocs = true;

   } else {
      // make sure some whitespace before an \internal command
      // is not treated as "documentation"

      if (current->getData(EntryKey::Main_Docs).trimmed().isEmpty()) {
         current->setData(EntryKey::Main_Docs,   "");
      }

      s_condCount = 0;
      BEGIN( SkipInternal );
  }

  return false;
}

static bool handleLineBr(const QString &)
{
   addToOutput('\n');
   return false;
}

static bool handleStatic(const QString &)
{
   endBrief();
   current->stat = true;
   return false;
}

static bool handlePure(const QString &)
{
   endBrief();
   current->virt = Pure;
   return false;
}

static bool handlePrivate(const QString &)
{
  current->protection = Private;
  return false;
}

static bool handlePrivateSection(const QString &)
{
   current->protection = protection = Private;
   return false;
}

static bool handleProtected(const QString &)
{
   current->protection = Protected;
   return false;
}

static bool handleProtectedSection(const QString &)
{
   current->protection = protection = Protected ;
   return false;
}

static bool handlePublic(const QString &)
{
   current->protection = Public;
   return false;
}

static bool handlePublicSection(const QString &)
{
   current->protection = protection = Public;
   return false;
}

static bool handleToc(const QString &)
{
   if (current->section == Entry::PAGEDOC_SEC || current->section==Entry::MAINPAGEDOC_SEC) {
      current->stat = true;  // we 'abuse' stat to pass whether or the TOC is enabled
   }
   return false;
}

static bool handleInherit(const QString &)
{
   BEGIN(InheritParam);
   return false;
}

static bool handleExtends(const QString &)
{
   BEGIN(ExtendsParam);
   return false;
}

static bool handleCopyBrief(const QString &)
{
   if (current->getData(EntryKey::Brief_Docs).isEmpty() && current->getData(EntryKey::Main_Docs).isEmpty()) {

      // if we do not have a brief or detailed description yet,
      // then the @copybrief should end up in the brief description.
      // otherwise it will be copied inline (see bug691315 & bug700788)

      setOutput(OutputBrief);
   }

   if (! s_spaceBeforeCmd.isEmpty()) {
      addToOutput(s_spaceBeforeCmd);
      s_spaceBeforeCmd.resize(0);
   }

   addToOutput("\\copybrief ");
   return false;
}

static bool handleCopyDetails(const QString &)
{
   setOutput(OutputMainDoc);
   if (! s_spaceBeforeCmd.isEmpty()) {
     addToOutput(s_spaceBeforeCmd);
     s_spaceBeforeCmd.resize(0);
   }

   addToOutput("\\copydetails ");
   return false;
}

static bool handleCopyDoc(const QString &)
{
   setOutput(OutputBrief);
   if (! s_spaceBeforeCmd.isEmpty()) {
     addToOutput(s_spaceBeforeCmd);
     s_spaceBeforeCmd.resize(0);
   }

   addToOutput("\\copybrief ");
   s_copyDocArg.resize(0);
   BEGIN(CopyDoc);

   return false;
}

static void checkFormula()
{
   if (YY_START == ReadFormulaShort || YY_START == ReadFormulaLong) {
      warn(yyFileName, yyLineNr, "End of comment block while inside formula.");
   }
}

// main entry point
bool parseCommentBlock(ParserInterface *parser, QSharedPointer<Entry> curEntry, const QString &comment,
                  const QString &fileName, int &lineNr, bool isBrief, bool isAutoBrief, bool isInbody,
                  Protection &r_protection, int &r_position, bool &r_newEntryNeeded )
{
   initParser();
   s_guards.clear();

   if (comment.isEmpty()) {
      // avoid empty strings
      return false;
   }

   langParser     = parser;
   current        = curEntry;
   s_inputString    = comment;
   yyFileName     = fileName;
   yyLineNr       = lineNr;
   briefEndsAtDot = isAutoBrief;
   inBody         = isInbody;

   protection     = r_protection;
   s_inputPosition  = r_position;

   xrefKind       = XRef_None;
   xrefAppendFlag = false;
   insidePre      = false;
   s_needNewEntry = false;
   s_parseMore    = false;

   s_outputXRef   = "";
   s_inputString.append(" ");

   if (isBrief || isAutoBrief) {
      setOutput(OutputBrief);

   } else {
      setOutput(OutputMainDoc);
   }

   s_condCount    = 0;
   s_sectionLevel = 0;

   s_spaceBeforeCmd.clear();
   s_spaceBeforeIf.clear();

   if (! current->getData(EntryKey::Main_Docs).isEmpty()) {
      // separate detailed doc fragments
      current->appendData(EntryKey::Main_Docs,  "\n\n");
   }

   if (! current->getData(EntryKey::Inbody_Docs).isEmpty() && isInbody) {
      // separate in body fragments
      current->appendData(EntryKey::Inbody_Docs, "\n\n");
   }

   yyrestart(yyin);
   BEGIN( Comment );
   yylex();

   setOutput(OutputMainDoc);

   if (YY_START == OverloadParam) {
      // comment ended with \overload
      addToOutput(theTranslator->trOverloadText());
   }

   if (! s_guards.isEmpty()) {
      warn(yyFileName, yyLineNr, "Documentation block ended in the middle of a conditional section");
   }

   if (s_insideParBlock) {
      warn(yyFileName, yyLineNr, "Documentation block ended while inside a \\parblock. Missing \\endparblock");
   }

   // removes blank lines from the detailed docs
   current->setData(EntryKey::Main_Docs, trimEmptyLines(current->getData(EntryKey::Main_Docs), current->docLine));

   if (current->section == Entry::FILEDOC_SEC && current->getData(EntryKey::Main_Docs).isEmpty()) {
      // to allow a comment block with just a @file command
      current->setData(EntryKey::Main_Docs, "\n\n");
   }

   if (current->section == Entry::MEMBERGRP_SEC && s_memberGroupId == DOX_NOGROUP) {
      // @name section but no group started yet
      openGroup(current, yyFileName, yyLineNr);
   }

   if (Doxy_Globals::markdownSupport) {
      current->setData(EntryKey::Brief_Docs,  processMarkdown(fileName, lineNr, current, current->getData(EntryKey::Brief_Docs)  ));
      current->setData(EntryKey::Main_Docs,   processMarkdown(fileName, lineNr, current, current->getData(EntryKey::Main_Docs)   ));
      current->setData(EntryKey::Inbody_Docs, processMarkdown(fileName, lineNr, current, current->getData(EntryKey::Inbody_Docs) ));
   }

   checkFormula();
   r_protection = protection;

   groupAddDocs(curEntry);
   r_newEntryNeeded = s_needNewEntry;

   if (s_parseMore && r_position == s_inputPosition) {
      // did not proceed during this call, do not continue or there will be an infinate loop
      s_parseMore = false;
   }

   if (s_parseMore) {
      r_position = s_inputPosition;
   } else {
      r_position = 0;
   }

   lineNr = yyLineNr;

   return s_parseMore;
}

void groupEnterFile(const QString &fileName, int)
{
   s_autoGroupStack.clear();
   s_memberGroupId = DOX_NOGROUP;
   s_memberGroupDocs.clear();
   s_memberGroupRelates.clear();
   s_compoundName = fileName;
}

void groupLeaveFile(const QString &fileName, int line)
{
   s_memberGroupId = DOX_NOGROUP;
   s_memberGroupRelates.clear();
   s_memberGroupDocs.clear();

   if (! s_autoGroupStack.isEmpty()) {
      warn(fileName, line, "End of file while inside a group\n");
   }
}

void groupEnterCompound(const QString &fileName, int line, const QString &name)
{
   if (s_memberGroupId != DOX_NOGROUP) {
      warn(fileName, line, "Try to put compound %s inside a member group\n", csPrintable(name));
   }

   s_memberGroupId = DOX_NOGROUP;
   s_memberGroupRelates.clear();
   s_memberGroupDocs.clear();
   s_compoundName = name;

   int i = s_compoundName.indexOf('(');

   if (i != -1) {
      // strip category (Obj-C)
      s_compoundName=s_compoundName.left(i);
   }

   if (s_compoundName.isEmpty()) {
      s_compoundName=fileName;
   }
}

void groupLeaveCompound(const QString &, int, const QString &)
{
   s_memberGroupId = DOX_NOGROUP;
   s_memberGroupRelates.resize(0);
   s_memberGroupDocs.resize(0);
   s_compoundName.resize(0);
}

static int findExistingGroup(int &groupId, const QSharedPointer<MemberGroupInfo> info)
{
   for (auto di = Doxy_Globals::memGrpInfoDict.begin(); di != Doxy_Globals::memGrpInfoDict.end(); ++di) {

      auto mi = *di;

      if (s_compoundName == mi->compoundName && ! mi->header.isEmpty() &&
               mi->header.compare(info->header, Qt::CaseInsensitive) == 0) {

         // same file or scope, not a nameless group, same header name
         return (int)di.key();    // put the item in this group
      }
   }

   groupId++; // start new group
   return groupId;
}

void openGroup(QSharedPointer<Entry> e, const QString &, int)
{
   if (e->section == Entry::GROUPDOC_SEC) {
      // auto group
      s_autoGroupStack.push( QMakeShared<Grouping>(e->m_entryName, e->groupingPri()) );

   } else {
      // start of a member group

      if (s_memberGroupId == DOX_NOGROUP) {
         // no group started yet
         static int curGroupId = 0;

         QSharedPointer<MemberGroupInfo> info = QMakeShared<MemberGroupInfo>();

         info->header = s_memberGroupHeader.trimmed();
         info->compoundName = s_compoundName;
         s_memberGroupId = findExistingGroup(curGroupId, info);

         Doxy_Globals::memGrpInfoDict.insert(s_memberGroupId, info);

         s_memberGroupRelates = e->getData(EntryKey::Related_Class);
         e->mGrpId = s_memberGroupId;
      }
   }
}

void closeGroup(QSharedPointer<Entry> e, const QString &fileName, int line, bool foundInline)
{
   if (s_memberGroupId != DOX_NOGROUP) {
      // end of member group

      QSharedPointer<MemberGroupInfo> info = Doxy_Globals::memGrpInfoDict.value(s_memberGroupId);

      if (info) {
         // known group
         info->doc     = s_memberGroupDocs;
         info->docFile = fileName;
         info->docLine = line;
      }

      s_memberGroupId = DOX_NOGROUP;
      s_memberGroupRelates.resize(0);
      s_memberGroupDocs.resize(0);

      if (! foundInline) {
         e->mGrpId = DOX_NOGROUP;
      }

   } else if (! s_autoGroupStack.isEmpty()) {
      // end of auto group
      QSharedPointer<Grouping> grp = s_autoGroupStack.pop();

      // see bug577005: we should not remove the last group for e
      if (! foundInline) {
         e->m_groups.removeLast();
      }

      if (! foundInline) {
         initGroupInfo(e);
      }
   }
}

void initGroupInfo(QSharedPointer<Entry> e)
{
   e->mGrpId  = s_memberGroupId;
   e->setData(EntryKey::Related_Class, s_memberGroupRelates);

   if (! s_autoGroupStack.isEmpty()) {
      e->m_groups.append(*s_autoGroupStack.top());
   }
}

static void groupAddDocs(QSharedPointer<Entry> e)
{
   if (e->section == Entry::MEMBERGRP_SEC) {
      s_memberGroupDocs = e->getData(EntryKey::Brief_Docs).trimmed();
      e->setData(EntryKey::Main_Docs, trimEmptyLines(e->getData(EntryKey::Main_Docs), e->docLine));

      if (! s_memberGroupDocs.isEmpty() && ! e->getData(EntryKey::Main_Docs).isEmpty()) {
         s_memberGroupDocs+="\n\n";
      }

      s_memberGroupDocs += e->getData(EntryKey::Main_Docs);
      QSharedPointer<MemberGroupInfo> info = Doxy_Globals::memGrpInfoDict.value(s_memberGroupId);

      if (info) {
         info->doc      = s_memberGroupDocs;
         info->docFile  = e->getData(EntryKey::MainDocs_File);
         info->docLine  = e->docLine;
         info->setRefItems(e->m_specialLists);
      }

      e->setData(EntryKey::Brief_Docs, "");
      e->setData(EntryKey::Main_Docs,  "");
   }
}

static void handleGuard(const QString &expr)
{
   CondParser prs;
   bool sectionEnabled = prs.parse(yyFileName, yyLineNr, expr.trimmed());
   bool parentEnabled  = true;

   if (! s_guards.isEmpty()) {
      parentEnabled = s_guards.top().isEnabled();
   }

   if (parentEnabled) {
      if ( (sectionEnabled && guardType == Guard_If) || (! sectionEnabled && guardType == Guard_IfNot)) {
         // section is visible
         s_guards.push(GuardedSection(true, true));
         enabledSectionFound = true;
         BEGIN( GuardParamEnd );

      } else {
         // section is invisible
         if (guardType != Guard_Skip) {
            s_guards.push(GuardedSection(false, true));
         }

         BEGIN( SkipGuardedSection );
      }

   } else {
      // invisible because of parent
      s_guards.push(GuardedSection(false, false));
      BEGIN( SkipGuardedSection );
   }
}
