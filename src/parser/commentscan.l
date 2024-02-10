/*************************************************************************
 *
 * Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
 * Copyright (c) 1997-2014 Dimitri van Heesch
 *
*************************************************************************/

%{

#include <commentscan.h>

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

#include <QFile>
#include <QStack>
#include <QVector>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define YY_NO_INPUT 1

// forward declarations
static bool handleBrief(const QString &str, const QStringList &list);
static bool handleFn(const QString &str, const QStringList &list);
static bool handleProperty(const QString &str, const QStringList &list);
static bool handleDef(const QString &str, const QStringList &list);
static bool handleOverload(const QString &str, const QStringList &list);
static bool handleEnum(const QString &str, const QStringList &list);
static bool handleDefGroup(const QString &str, const QStringList &list);
static bool handleAddToGroup(const QString &str, const QStringList &list);
static bool handleWeakGroup(const QString &str, const QStringList &list);
static bool handleNamespace(const QString &str, const QStringList &list);
static bool handlePackage(const QString &str, const QStringList &list);
static bool handleClass(const QString &str, const QStringList &list);
static bool handleConcept(const QString &str, const QStringList &list);
static bool handleHeaderFile(const QString &str, const QStringList &list);
static bool handleProtocol(const QString &str, const QStringList &list);
static bool handleCategory(const QString &str, const QStringList &list);
static bool handleUnion(const QString &str, const QStringList &list);
static bool handleStruct(const QString &str, const QStringList &list);
static bool handleInterface(const QString &str, const QStringList &list);
static bool handleIdlException(const QString &str, const QStringList &list);
static bool handlePage(const QString &str, const QStringList &list);
static bool handleMainpage(const QString &str, const QStringList &list);
static bool handleFile(const QString &str, const QStringList &list);
static bool handleDir(const QString &str, const QStringList &list);
static bool handleExample(const QString &str, const QStringList &list);
static bool handleDetails(const QString &str, const QStringList &list);
static bool handleName(const QString &str, const QStringList &list);
static bool handleTodo(const QString &str, const QStringList &list);
static bool handleTest(const QString &str, const QStringList &list);
static bool handleBug(const QString &str, const QStringList &list);
static bool handleSubpage(const QString &str, const QStringList &list);
static bool handleDeprecated(const QString &str, const QStringList &list);
static bool handleXRefItem(const QString &str, const QStringList &list);
static bool handleRelated(const QString &str, const QStringList &list);
static bool handleRelatedAlso(const QString &str, const QStringList &list);
static bool handleMemberOf(const QString &str, const QStringList &list);
static bool handleRefItem(const QString &str, const QStringList &list);
static bool handleSection(const QString &str, const QStringList &list);
static bool handleAnchor(const QString &str, const QStringList &list);
static bool handleCite(const QString &str, const QStringList &list);
static bool handleFormatBlock(const QString &str, const QStringList &list);
static bool handleAddIndex(const QString &str, const QStringList &list);
static bool handleIf(const QString &str, const QStringList &list);
static bool handleIfNot(const QString &str, const QStringList &list);
static bool handleElseIf(const QString &str, const QStringList &list);
static bool handleElse(const QString &str, const QStringList &list);
static bool handleEndIf(const QString &str, const QStringList &list);
static bool handleIngroup(const QString &str, const QStringList &list);
static bool handleNoSubGrouping(const QString &str, const QStringList &list);
static bool handleShowInitializer(const QString &str, const QStringList &list);
static bool handleHideInitializer(const QString &str, const QStringList &list);
static bool handleCallgraph(const QString &str, const QStringList &list);
static bool handleHideCallgraph(const QString &str, const QStringList &list);
static bool handleCallergraph(const QString &str, const QStringList &list);
static bool handleHideCallergraph(const QString &str, const QStringList &list);
static bool handleReferencedByRelation(const QString &str, const QStringList &list);
static bool handleHideReferencedByRelation(const QString &str, const QStringList &list);
static bool handleReferencesRelation(const QString &str, const QStringList &list);
static bool handleHideReferencesRelation(const QString &str, const QStringList &list);
static bool handleInternal(const QString &str, const QStringList &list);
static bool handleStatic(const QString &str, const QStringList &list);
static bool handlePure(const QString &str, const QStringList &list);
static bool handlePrivate(const QString &str, const QStringList &list);
static bool handlePrivateSection(const QString &str, const QStringList &list);
static bool handleProtected(const QString &str, const QStringList &list);
static bool handleProtectedSection(const QString &str, const QStringList &list);
static bool handlePublic(const QString &str, const QStringList &list);
static bool handlePublicSection(const QString &str, const QStringList &list);
static bool handleToc(const QString &str, const QStringList &list);
static bool handleInherit(const QString &str, const QStringList &list);
static bool handleExtends(const QString &str, const QStringList &list);
static bool handleCopyDoc(const QString &str, const QStringList &list);
static bool handleCopyBrief(const QString &str, const QStringList &list);
static bool handleCopyDetails(const QString &str, const QStringList &list);
static bool handleParBlock(const QString &str, const QStringList &list);
static bool handleEndParBlock(const QString &str, const QStringList &list);
static bool handleParam(const QString &str, const QStringList &list);
static bool handleRetval(const QString &str, const QStringList &list);
//   static void handleGuard(const QString &str, const QStringList &list);

static void checkFormula();

using DocCmdFunc = bool (*)(const QString &str, const QStringList &list);

struct DocCmdMap
{
   DocCmdMap(const char *x1, DocCmdFunc x2, bool x3) {
      cmdName   = QString::fromUtf8(x1);
      handler   = x2;
      endsBrief = x3;
   }

   QString cmdName;
   DocCmdFunc handler;
   bool endsBrief;
};

// map of command to handler function
static DocCmdMap docCmdMap[] =
{
  // command name      handler function         ends brief description
  { "addindex",        &handleAddIndex,                 false },
  { "addtogroup",      &handleAddToGroup,               false },
  { "anchor",          &handleAnchor,                   true  },
  { "arg",             nullptr,                         true  },
  { "attention",       nullptr,                         true  },
  { "author",          nullptr,                         true  },
  { "authors",         nullptr,                         true  },
  { "brief",           &handleBrief,                    false },
  { "bug",             &handleBug,                      false },
  { "callergraph",     &handleCallergraph,              false },
  { "callgraph",       &handleCallgraph,                false },
  { "category",        &handleCategory,                 false },
  { "cite",            &handleCite,                     false },
  { "class",           &handleClass,                    false },
  { "code",            &handleFormatBlock,              true  },
  { "concept",         &handleConcept,                  false },
  { "copybrief",       &handleCopyBrief,                false },
  { "copydetails",     &handleCopyDetails,              true  },
  { "copydoc",         &handleCopyDoc,                  true  },
  { "copyright",       nullptr,                         true  },
  { "date",            nullptr,                         true  },
  { "def",             &handleDef,                      false },
  { "defgroup",        &handleDefGroup,                 false },
  { "deprecated",      &handleDeprecated,               false },
  { "details",         &handleDetails,                  true  },
  { "dir",             &handleDir,                      false },
  { "docbookinclude",  nullptr,                         false },
  { "docbookonly",     &handleFormatBlock,              false },
  { "dot",             &handleFormatBlock,              true  },
  { "dotfile",         nullptr,                         true  },
  { "else",            &handleElse,                     false },
  { "elseif",          &handleElseIf,                   false },
  { "endif",           &handleEndIf,                    false },
  { "endparblock",     &handleEndParBlock,              true  },
  { "enum",            &handleEnum,                     false },
  { "example",         &handleExample,                  false },
  { "exception",       nullptr,                         true  },
  { "extends",         &handleExtends,                  true  },
  { "file",            &handleFile,                     false },
  { "fn",              &handleFn,                       false },
  { "group",           &handleDefGroup,                 false },
  { "headerfile",      &handleHeaderFile,               false },
  { "hidecallergraph", &handleHideCallergraph,          false },
  { "hidecallgraph",   &handleHideCallgraph,            false },
  { "hideinitializer", &handleHideInitializer,          false },
  { "hiderefby",       &handleHideReferencedByRelation, false },
  { "hiderefs",        &handleHideReferencesRelation,   false },
  { "htmlinclude",     nullptr,                         false },
  { "htmlonly",        &handleFormatBlock,              false },
  { "idlexcept",       &handleIdlException,             false },
  { "if",              &handleIf,                       false },
  { "ifnot",           &handleIfNot,                    false },
  { "image",           nullptr,                         true  },
  { "implements",      &handleExtends,                  true  },
  { "include",         nullptr,                         true  },
  { "includelineno",   nullptr,                         true  },
  { "ingroup",         &handleIngroup,                  false },
  { "inherit",         &handleInherit,                  true  },
  { "interface",       &handleInterface,                false },
  { "internal",        &handleInternal,                 true  },
  { "invariant",       nullptr,                         true  },
  { "latexinclude",    nullptr,                         false },
  { "latexonly",       &handleFormatBlock,              false },
  { "li",              nullptr,                         true  },
  { "line",            nullptr,                         true  },
  { "mainpage",        &handleMainpage,                 false },
  { "maninclude",      nullptr,                         false },
  { "manonly",         &handleFormatBlock,              false },
  { "memberof",        &handleMemberOf,                 true  },
  { "msc",             &handleFormatBlock,              true  },
  { "name",            &handleName,                     false },
  { "namespace",       &handleNamespace,                false },
  { "nosubgrouping",   &handleNoSubGrouping,            false },
  { "note",            nullptr,                         true  },
  { "overload",        &handleOverload,                 false },
  { "package",         &handlePackage,                  false },
  { "page",            &handlePage,                     false },
  { "par",             nullptr,                         true  },
  { "paragraph",       &handleSection,                  true  },
  { "param",           &handleParam,                    true  },
  { "parblock",        &handleParBlock,                 true  },
  { "post",            nullptr,                         true  },
  { "pre",             nullptr,                         true  },
  { "private",         &handlePrivate,                  false },
  { "privatesection",  &handlePrivateSection,           false },
  { "property",        &handleProperty,                 false },
  { "protected",       &handleProtected,                false },
  { "protectedsection",&handleProtectedSection,         false },
  { "protocol",        &handleProtocol,                 false },
  { "public",          &handlePublic,                   false },
  { "publicsection",   &handlePublicSection,            false },
  { "pure",            &handlePure,                     false },
  { "refitem",         &handleRefItem,                  true  },
  { "related",         &handleRelated,                  true  },
  { "relatedalso",     &handleRelatedAlso,              true  },
  { "relates",         &handleRelated,                  true  },
  { "relatesalso",     &handleRelatedAlso,              true  },
  { "remark",          nullptr,                         true  },
  { "remarks",         nullptr,                         true  },
  { "result",          nullptr,                         true  },
  { "return",          nullptr,                         true  },
  { "returns",         nullptr,                         true  },
  { "retval",          &handleRetval,                   true  },
  { "rtfinclude",      nullptr,                         false },
  { "rtfonly",         &handleFormatBlock,              false },
  { "sa",              nullptr,                         true  },
  { "section",         &handleSection,                  true  },
  { "see",             nullptr,                         true  },
  { "short",           &handleBrief,                    false },
  { "showinitializer", &handleShowInitializer,          false },
  { "showrefby",       &handleReferencedByRelation,     false },
  { "showrefs",        &handleReferencesRelation,       false },
  { "since",           nullptr,                         true  },
  { "snippet",         nullptr,                         true  },
  { "snippetlineno",   nullptr,                         true  },
  { "startuml",        &handleFormatBlock,              true  },
  { "static",          &handleStatic,                   false },
  { "struct",          &handleStruct,                   false },
  { "subpage",         &handleSubpage,                  true  },
  { "subsection",      &handleSection,                  true  },
  { "subsubsection",   &handleSection,                  true  },
  { "tableofcontents", &handleToc,                      false },
  { "test",            &handleTest,                     false },
  { "throw",           nullptr,                         true  },
  { "throws",          nullptr,                         true  },
  { "todo",            &handleTodo,                     false },
  { "tparam",          nullptr,                         true  },
  { "typedef",         &handleFn,                       false },
  { "union",           &handleUnion,                    false },
  { "until",           nullptr,                         true  },
  { "var",             &handleFn,                       false },
  { "verbatim",        &handleFormatBlock,              true  },
  { "verbinclude",     nullptr,                         false },
  { "version",         nullptr,                         true  },
  { "warning",         nullptr,                         true  },
  { "weakgroup",       &handleWeakGroup,                false },
  { "xmlinclude",      nullptr,                         false },
  { "xmlonly",         &handleFormatBlock,              false },
  { "xrefitem",        &handleXRefItem,                 false },
  { nullptr,           nullptr,                         false }
};

//  Maps a command name (as found in a comment block) onto a specific handler function
class DocCmdMapper
{
  public:
    struct Cmd {
      DocCmdFunc func;
      bool endsBrief;
    };

    // maps a command name to a handler function
    static Cmd *map(const QString &name) {
      return instance()->find(name);
    }

    // release the instance
    static void freeInstance() {
      delete s_instance;
      s_instance = nullptr;
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

      while (! p->cmdName.isEmpty()) {
         if (m_map.contains(p->cmdName)) {
            err("DocCmdMapper: command %s already added\n", csPrintable(p->cmdName));
            Doxy_Work::stopDoxyPress();
         }

         Cmd *cmd       = new Cmd;
         cmd->func      = p->handler;
         cmd->endsBrief = p->endsBrief;

         m_map.insert(p->cmdName, cmd);
         ++p;
      }
   }

   Cmd *find(const QString &name) {
      return m_map.value(name);
   }

   QHash<QString, Cmd *> m_map;
   static DocCmdMapper *s_instance;
};

DocCmdMapper *DocCmdMapper::s_instance = nullptr;

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
      : m_enabled(enabled), m_parentVisible(parentVisible)
   { }

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

static void groupAddDocs(QSharedPointer<Entry> e);

static QString          s_inputString;         // input string
static QString          s_currentCommand;
static int              s_inputPosition;       // read pointer
static int              s_prevPosition;
static char            *s_bufferPosition;

static QString          yyFileName;            // file name that is read from
static int              yyLineNr;              // line number in the input
static bool             inBody;                // was the comment found inside the body of a function?
static OutputContext    inContext;             // are we inside the brief, details or xref part
static bool             briefEndsAtDot;        // does the brief description stop at a dot
static QString          formulaText;           // Running text of a formula
static QString          formulaEnv;            // environment name
static int              formulaNewLines;       // amount of new lines in the formula

static QString          s_outputXRef;          // tmp argument of todo/test/../xrefitem commands
static QString          s_blockName;           // preformatted block name (e.g. verbatim, latexonly,...)
static XRefKind         xrefKind;              // kind of cross-reference command
static XRefKind         newXRefKind;
static GuardType        s_guardType;           // kind of guard for conditional section
static bool             s_isEnabledSection;
static QString          s_functionProto;       // function prototype

static bool             s_needNewEntry;
// static int           s_docBlockContext;

static QString          s_sectionLabel;
static QString          s_sectionTitle;
static int              s_sectionLevel;
static QString          xrefItemKey;
static QString          newXRefItemKey;
static QString          xrefItemTitle;
static QString          xrefListTitle;
static Protection       s_protection;

static bool             xrefAppendFlag;
static bool             inGroupParamFound;
static int              s_braceCount;
static bool             insidePre;
static bool             s_parseMore;

static int              s_condCount;
static int              s_commentCount;
static QString          s_spaceBeforeCmd;
static QString          s_spaceBeforeIf;
static QString          s_copyDocArg;
static QString          s_guardExpr;
static int              s_roundCount;
static int              s_htmlDetails;
static bool             s_insideParBlock;

static int              s_openCount;
static int              s_memberGroupId = DOX_NOGROUP;
static QString          s_memberGroupHeader;
static QString          s_memberGroupDocs;
static QString          s_memberGroupRelates;
static QString          s_compoundName;

static QString          s_htmlAnchor;
static bool             s_isHtmlAnchor;

static bool             s_internalDocs;
static bool             s_processInternalDocs = false;

static ParserInterface        *langParser;        // the language parser calling us

static QSharedPointer<Entry>  s_docsEntry;        // which entry
static EntryKey               s_docsEnum;         // which enum in EntryKey (brief, main, inbody)

static QStack<GuardedSection> s_guards;           // tracks nested conditional sections (if, ifnot, ..)
static QSharedPointer<Entry>  current;            // working entry

static QStack<QSharedPointer<Grouping>> s_autoGroupStack;

// double declared
static void yyunput (int c, char *buf_ptr);

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
    case Entry::CATEGORYDOC_SEC:
    case Entry::CLASSDOC_SEC:
    case Entry::CONCEPTDOC_SEC:
    case Entry::DEFINEDOC_SEC:
    case Entry::DIRDOC_SEC:
    case Entry::ENUMDOC_SEC:
    case Entry::EXAMPLE_SEC:
    case Entry::EXCEPTIONDOC_SEC:
    case Entry::FILEDOC_SEC:
    case Entry::GROUPDOC_SEC:
    case Entry::MAINPAGEDOC_SEC:
    case Entry::MEMBERDOC_SEC:
    case Entry::MEMBERGRP_SEC:
    case Entry::NAMESPACEDOC_SEC:
    case Entry::OVERLOADDOC_SEC:
    case Entry::PACKAGEDOC_SEC:
    case Entry::PAGEDOC_SEC:
    case Entry::PROTOCOLDOC_SEC:
    case Entry::STRUCTDOC_SEC:
    case Entry::UNIONDOC_SEC:
    case Entry::VARIABLEDOC_SEC:
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
      current->docLine   = yyLineNr;

      current->setData(EntryKey::File_Name, yyFileName);

      return false;
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

   ListItemInfo *listItem = nullptr;

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
      item->text += s_outputXRef;

   } else {
      // new item

      int itemId  = refList->addRefItem();

      // if we have already an item from the same list type (e.g. a second @todo)
      // in the same Entry (i.e. lii!=0) then we reuse its link anchor.

      QString anchorLabel = QString("_%1%2").formatArg(listName).formatArg(itemId, 6, 10, QChar('0'));

      RefItem *item = refList->getRefItem(itemId);
      assert(item != 0);

      item->text       = s_outputXRef;
      item->listAnchor = anchorLabel;

      docEntry->addSpecialListItem(listName, itemId);

      QString cmdString = QString(" \\xrefitem %1 %2.").formatArg(listName).formatArg(itemId);

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

   s_outputXRef.clear();
}

// Adds a formula text to the list/dictionary of formulas if it was
// not already added. Returns the label of the formula.
static QString addFormula()
{
   QString formLabel;

   auto f = Doxy_Globals::formulaDict.find(formulaText);

   if (f == Doxy_Globals::formulaDict.end()) {
      Formula tmp = Formula(formulaText);

      Doxy_Globals::formulaList.append(tmp);
      Doxy_Globals::formulaDict.insert(formulaText, tmp);

      formLabel = QString("\\form#%1").formatArg(tmp.getId());

      Doxy_Globals::formulaNameDict.insert(formLabel, tmp);
      f = Doxy_Globals::formulaDict.find(formulaText);

   } else {
      formLabel = QString("\\form#%1").formatArg(f->getId());

   }

   for (int i = 0; i < formulaNewLines; ++i) {
      // add fake newlines to keep the warnings correctly aligned
      formLabel += "@_fakenl";
   }

   return formLabel;
}

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

   if (si != nullptr) {

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

      // for a section the si->fileName is the physical file name, this will be replaced with the
      // section name in doctokenizer processSection()
   }
}

static void addCite()
{
   QString text = QString::fromUtf8(yytext);
   if (text.startsWith('"')) {
      text.chop(1);
      text = text.mid(1);
   }

   Doxy_Globals::citeDict.insert(text);
}


static void lineCount()
{
   // commentscan, parse_py

   for (const char *p = yytext; *p; ++p) {
      yyLineNr += (*p == '\n');
   }
}

// strip trailing whitespace (excluding newlines) from string s
static void stripTrailingWhiteSpace(QString &str)
{
   if (str.isEmpty()) {
      return;
   }

   QString::const_iterator iter       = str.constEnd() - 1;
   QString::const_iterator iter_start = str.constBegin();
   QString::const_iterator iter_end   = str.constEnd();

   QChar c;

   while (true) {
      c = *iter;
      QStringView tmp = QStringView(iter, iter_end);

      if (c == ' ' || c == '\t' || c == '\r') {
         // normal whitespace

      } else if (tmp.endsWith("\\internal_linebr")) {
         // special line break marker

         iter -= 15;

      } else if (c == '\n') {
         // normal newline

      } else {
         // non-whitespace, done
         break;
      }

      if (iter == str.constBegin()) {
         break;
      }

      --iter;
   }


   // update the string
   str = QString(iter_start, iter+1);
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
            // ensure the current entry is marked
            current->m_traits.setTrait(Entry::Virtue::Deprecated);

            addXRefItem("deprecated", theTranslator->trDeprecated(), theTranslator->trDeprecatedList(), xrefAppendToPrev);
            break;

         case XRef_Item:
            // user defined list
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
               current->setData(EntryKey::Brief_File, yyFileName);
               current->briefLine = yyLineNr;
            }
         }

         if (current->getData(EntryKey::Brief_Docs).trimmed().isEmpty())  {
            // only want one brief description even if multiple are given
            s_docsEntry = current;
            s_docsEnum  = EntryKey::Brief_Docs;

         } else {

            if (! current->getData(EntryKey::Main_Docs).isEmpty()) {
               // when appending parts add a new line
               current->appendData(EntryKey::Main_Docs, "\n");
            }

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
      si = QMakeShared<SectionInfo>(yyFileName, yyLineNr, anchorName, QString(), SectionInfo::Anchor, 0);
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

static void unputString(const QString &str) {

   auto iter     = str.storage_rbegin();
   auto iter_end = str.storage_rend();

   while (iter != iter_end) {
      unput(*iter);

      ++iter;
   }
}

static void endBrief(bool isOutput = true)
{
   if (! current->getData(EntryKey::Brief_Docs).trimmed().isEmpty()) {
      // only go to the detailed description if we found a brief description and not just whitespace

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
#define  YY_INPUT(buf,result,max_size) result = yyread(buf, max_size);

static int yyread(char *buf, int max_size)
{
   s_bufferPosition = buf;
   s_prevPosition   = s_inputPosition;

   //
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

/* start command character */
CMD             ("\\"|"@")
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
CENTER          [cC][eE][nN][tT][eE][rR]
DIV             [dD][iI][vV]
DETAILS         [dD][eE][tT][aA][iI][lL][sS]
DETAILEDHTML    {CENTER}|{DIV}|{PRE}|{UL}|{TABLE}|{OL}|{DL}|{P}|[Hh][1-6]|{IMG}|{HR}|{PARA}
DETAILEDHTMLOPT {CODE}
SUMMARY         [sS][uU][mM][mM][aA][rR][yY]
REMARKS         [rR][eE][mM][aA][rR][kK][sS]
ANCHORHTML      [aA]{BN}*
ANCHORID        ([iI][dD]|[nN][aA][mM][eE])"="("\""{LABELID}"\""|"'"{LABELID}"'"|{LABELID})
BN              [ \t\n\r]
BL              [ \t\r]*"\n"
B               [ \t]
BS              ^(({B}*"/""/")?)(({B}*"*"+)?){B}*
ATTR            ({B}+[^>\n]*)?
DOCNL           "\n"|"\\internal_linebr"
LC              "\\"{B}*"\n"
NW              [^a-z_A-Z0-9]
FILESCHAR       [a-z_A-Z0-9\x80-\xFF\\:\\\/\-\+@&#]
FILEECHAR       [a-z_A-Z0-9\x80-\xFF\-\+@&#]
FILE            ({FILESCHAR}*{FILEECHAR}+("."{FILESCHAR}*{FILEECHAR}+)*)|("\""[^\n\"]*"\"")
ID              [$a-z_A-Z\x80-\xFF][$a-z_A-Z0-9\x80-\xFF]*
LABELID         [a-z_A-Z\x80-\xFF][a-z_A-Z0-9\x80-\xFF\-]*
CITESCHAR       [a-z_A-Z0-9\x80-\xFF\-\?]
CITEECHAR       [a-z_A-Z0-9\x80-\xFF\-\+:\/\?]*
CITEID          {CITESCHAR}{CITEECHAR}*("."{CITESCHAR}{CITEECHAR}*)*|"\""{CITESCHAR}{CITEECHAR}*("."{CITESCHAR}{CITEECHAR}*)*"\""
SCOPEID         {ID}({ID}*{BN}*"::"{BN}*)*({ID}?)
SCOPENAME       "$"?(({ID}?{BN}*("::"|"."){BN}*)*)((~{BN}*)?{ID})
TMPLSPEC        "<"{BN}*[^>]+{BN}*">"
MAILADDR         [a-z_A-Z0-9.+\-]+"@"[a-z_A-Z0-9\-]+("."[a-z_A-Z0-9\-]+)+[a-z_A-Z0-9\-]+
RCSTAG          "$"{ID}":"[^\n$]+"$"

%option never-interactive
%option nounistd
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
%x ConceptDocArg1
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
%x ReadFormulaRound
%x ReadFormulaLong
%x AnchorLabel
%x HtmlComment
%x HtmlAnchor
%x SkipLang
%x CiteLabel
%x CopyDoc
%x GuardExpr
%x CdataSection

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
   *   mail address
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

<Comment>"<"{DETAILS}{ATTR}">"          {
      // start of a HTML style details description
      QString text = QString::fromUtf8(yytext);

      ++s_htmlDetails;

      setOutput(OutputMainDoc);
      addToOutput(text);
   }

<Comment>"</"{DETAILS}">"               {
      // end of a HTML style details description
      QString text = QString::fromUtf8(yytext);

      if (s_htmlDetails != 0) {
         --s_htmlDetails;
      }

      addToOutput(text);
   }

<Comment>"<"{ANCHORHTML}                     {
   // potential start of HTML anchor
   QString text = QString::fromUtf8(yytext);

   s_htmlAnchor   = text;
   s_isHtmlAnchor = false;
   BEGIN(HtmlAnchor);
}

<HtmlAnchor>{ANCHORID}                        {
   // only labels that can be converted to doxypress anchor
   QString text = QString::fromUtf8(yytext);
   s_htmlAnchor += text;

   QString tag(text);

   int pos  = tag.indexOf("=");
   QChar ch = tag[pos + 1];

   QString id;

   if (ch == '\'' || ch == '"')  {
      // valid start

     int end = tag.indexOf(ch, pos + 2);

     if (end != -1) {
         // found matching end, extract id
         id = tag.mid(pos + 2, end - pos - 2);
         addAnchor(id);
     }

   } else {
     id = tag.mid(pos + 1);
     addAnchor(id);
   }

   if (! id.isEmpty() && ! s_isHtmlAnchor) {
     // only use first analogous to what is in docparser
     addToOutput("@anchor ");
     addToOutput(id);
     addToOutput(" ");

     s_isHtmlAnchor = true;
   }
 }

<HtmlAnchor>("\""[^\n\"]*"\""|"'"[^\n']*"'") {
   QString text = QString::fromUtf8(yytext);
   s_htmlAnchor += text;
}

<HtmlAnchor>">"|"/>"       {
   QString text = QString::fromUtf8(yytext);

   if (! s_isHtmlAnchor) {
     addToOutput(s_htmlAnchor);
     addToOutput(text);

   } else if (text.length() == 1) {
     // to keep <a></a> pairs, otherwise single </a> present
     addToOutput("<a>");

   }

   BEGIN(Comment);
 }

<HtmlAnchor>{DOCNL}   {
   // newline
   QString text = QString::fromUtf8(yytext);

   s_htmlAnchor += text;

   if (text[0] == '\n')  {
      ++yyLineNr;
   }
}

<HtmlAnchor>.         {
   // catch-all for anything else
   QString text = QString::fromUtf8(yytext);
   s_htmlAnchor += text;
}

<Comment>"<"{REMARKS}">"          {
      // start of a .NET XML style detailed description
      QString text = QString::fromUtf8(yytext);
      setOutput(OutputMainDoc);
      addToOutput(text);
   }

<Comment>"</"{REMARKS}">"        {
      // end of a brief or detailed description
      QString text = QString::fromUtf8(yytext);

      setOutput(OutputMainDoc);
      addToOutput(text);
   }

<Comment>"<"{SUMMARY}">"         {
      // start of a .NET XML style brief description
      QString text = QString::fromUtf8(yytext);

      if (s_htmlDetails == 0) {
         setOutput(OutputBrief);
      }

      addToOutput(text);
   }

<Comment>"</"{SUMMARY}">"        {
      // end of a .NET XML style detailed description
      QString text = QString::fromUtf8(yytext);

      addToOutput(text);

      if (s_htmlDetails == 0) {
         setOutput(OutputMainDoc);
      }
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

<Comment>"<!\[CDATA\["        {
      BEGIN(CdataSection);
   }

<Comment>{B}*{CMD}"endinternal"{B}*    {
      addToOutput(" \\endinternal ");

      if (! s_processInternalDocs) {
         warn(yyFileName, yyLineNr, "Found \\endinternal without matching \\internal");
      }

      s_processInternalDocs = false;
   }

<Comment>{B}*"\\internal_linebr"{B}*            {
      // preserve spacing around \\internal_linebr
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<Comment>{B}*{CMD}"bypassundocwarn"{B}* {
      if (! current->m_entryName.isEmpty()) {
         current->m_traits.setTrait(Entry::Virtue::BypassUndocWarn);
      }
   }

<Comment>{B}*{CMD}"force_output"{B}* {
      // simulate calling addToOutPut()
      QString text = QString::fromUtf8(yytext);
      s_docsEntry->appendData(EntryKey::Main_Docs, text);
   }

<Comment>{B}*{CMD}"sortid"{B}+[0-9]+    {
      // simulate calling addToOutPut()
      QString text = QString::fromUtf8(yytext);
      s_docsEntry->appendData(EntryKey::Main_Docs, text);
   }

<Comment>{B}*{CMD}[a-z_A-Z]+"{"[a-zA-Z_,:0-9\. ]*"}"{B}*  |
<Comment>{B}*{CMD}[a-z_A-Z]+{B}*    {
      // might be a valid command
      QString text = QString::fromUtf8(yytext);

      /* handle `\f{` and `@f{` as special cases */
      int idx = text.indexOf('{');

      if ((idx > 1) && (text[idx - 1] == 'f') && (text[idx - 2] == '\\' || text[idx - 2] =='@')) {
         REJECT;
      }

      QStringView view = QStringView(text).trimmed();

      QString cmdName;
      QStringList optList;

      if (view.contains('{')) {
         // cmd { options }

         QString tmp = view.mid(1, 2).toLower();

         if (tmp == "f{")  {
            // handle "\f{" and "@f{" elsewhere
            REJECT;
         }

         static QRegularExpression regexp("[\\\\@](\\w+)\\s*{([^}]+)}");
         QRegularExpressionMatch match = regexp.match(view);

         // remove {CMD}
         cmdName = match.captured(1);
         optList = match.captured(2).split(',');

      } else {
         // no options
         cmdName = view.mid(1);

      }

      DocCmdMapper::Cmd *cmdPtr = DocCmdMapper::map(cmdName);

      if (cmdPtr) {
         // special action is required

         int i = 0;
         while (text[i] == ' ' || text[i] == '\t') {
            ++i;
         }

         s_spaceBeforeCmd = text.left(i);

         if (cmdPtr->endsBrief && ! (inContext == OutputXRef && cmdName == "parblock")) {
            briefEndsAtDot = false;

            // this command forces the end of brief description
            setOutput(OutputMainDoc);
         }

         if (cmdPtr->func && cmdPtr->func(cmdName, optList)) {
            // handler wants to stop and reenter this parser

            s_parseMore = true;

            s_inputPosition = s_prevPosition + (yy_bp - s_bufferPosition);
            yyterminate();

         } else if (cmdPtr->func == nullptr) {
            // command without handler is processed later by docparser
            addToOutput(text);
         }

      } else {
         // some command was found, just pass it on
         addToOutput(text);
      }
   }

<Comment>{B}*({CMD}{CMD})"f"[$\[{]  {
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

      // this command forces the end of brief description
      setOutput(OutputMainDoc);

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

<Comment>{B}*{CMD}"f("        {
      // start of a inline formula
      formulaText     = QString();
      formulaNewLines = 0;

      BEGIN(ReadFormulaRound);
   }

<Comment>{B}*{CMD}"f["        {
      // start of a block formula,

      // command forces the end of brief description
      setOutput(OutputMainDoc);

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

      if (inContext != OutputXRef) {
         // this command forces the end of brief description
         setOutput(OutputMainDoc);
      }

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

<Comment>"."[,:;]         {
      // . with some syntax like "e.g.," or "e.g.:"
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

<Comment>(\n|\\internal_linebr)({B}*(\n|\\internal_linebr))+    {
      // at least one blank line or blank line command

      QString text = QString::fromUtf8(yytext);

      if (inContext == OutputXRef) {
         // need to put the newlines after ending the XRef section

         if (! s_insideParBlock) {
            setOutput(OutputMainDoc);
         }

         for (int i = 0; i < text.length();  ) {
            if (text[i] == '\n') {
               addToOutput('\n');
               ++i;

            } else if (text.mid(i) == "\\internal_linebr")  {
               addToOutput("\\internal_linebr");
               i += 16;

            } else  {
               ++i;
            }
         }

      } else if (inContext != OutputBrief) {

         for (int i = 0; i< text.length(); ) {
            if (text[i] == '\n') {
               addToOutput('\n');
               i++;

            } else if (text.mid(i) == "\\internal_linebr")  {
               addToOutput("\\internal_linebr");
               i += 16;

            } else  {
               i++;

            }
         }

         setOutput(OutputMainDoc);

      } else {
          // inContext == OutputBrief, switch from brief to main docs
         endBrief(true);
      }

      lineCount();
   }

<Comment>"."            {
      // potential end of a JavaDoc style comment
      QString text = QString::fromUtf8(yytext);
      addToOutput(text[0]);

      if (briefEndsAtDot) {
         briefEndsAtDot = false;
         setOutput(OutputMainDoc);
      }
   }

<Comment>{DOCNL}  {
      // newline
      QString text = QString::fromUtf8(yytext);

      addToOutput('\n');

      if (text[0] == '\n') {
         ++yyLineNr;
      }
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
         addToOutput('\n');
         ++yyLineNr;
      }
   }

<HtmlComment>[^\\\n\-]+          {
      // ignore unimportant characters
   }

<HtmlComment>.             {
      // ignore every else
   }

<CdataSection>"\]\]>"      {
      BEGIN( Comment );
   }

<CdataSection>{DOCNL}      {
      QString text = QString::fromUtf8(yytext);

      addToOutput('\n');

      if (text[0] == '\n') {
         ++yyLineNr;
      }
   }

<CdataSection>[<>&]        {
      // the special XML characters for iwhich the CDATA section is especially used
      QString text = QString::fromUtf8(yytext);
      addToOutput('\\');
      addToOutput(text[0]);
   }

<CdataSection>[^\\\n\]<>&]+  {
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);
   }

<CdataSection>.    {
      QString text = QString::fromUtf8(yytext);
      addToOutput(text[0]);
   }


 /* --------------   Rules for handling formulas ---------------- */

<ReadFormulaShort>{CMD}"f$"      {
      // end of inline formula
      formulaText += "$";
      addToOutput(" " + addFormula());
      BEGIN(Comment);
   }

<ReadFormulaRound>{CMD}"f)"       {
      // end of inline formula
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

<ReadFormulaLong,ReadFormulaShort,ReadFormulaRound>[^\\@\n]+ {
      // any non-special character
      QString text = QString::fromUtf8(yytext);
      formulaText += text;
   }

<ReadFormulaLong,ReadFormulaShort,ReadFormulaRound>\n   {
      // new line
      QString text = QString::fromUtf8(yytext);

      formulaNewLines++;
      formulaText += text[0];
      yyLineNr++;
   }

<ReadFormulaLong,ReadFormulaShort,ReadFormulaRound>.     {
      // any other character
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
      unputString(text);

      BEGIN( Comment );
   }

<EnumDocArg1>.             {
      // ignore other stuff
   }


  /* ------------ handle argument of namespace command --------------- */

<NameSpaceDocArg1>{SCOPENAME}       {
      // handle argument
      QString text  = QString::fromUtf8(yytext);

      lineCount();
      current->m_entryName = substitute(removeRedundantWhiteSpace(text),".", "::");

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
      unputString(text);

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
      unputString(text);


      BEGIN( Comment );
   }

<PackageDocArg1>.          {
      // ignore other stuff
   }


  /* ------ handle argument of class/struct/union command --------------- */

<ClassDocArg1>{SCOPENAME}{TMPLSPEC}     {
      QString text = QString::fromUtf8(yytext);

      lineCount();
      current->m_entryName = substitute(removeRedundantWhiteSpace(text),".","::");

      BEGIN( ClassDocArg2 );
   }

<ClassDocArg1>{SCOPENAME}     {
      // first argument
      QString text = QString::fromUtf8(yytext);

      lineCount();
      current->m_entryName = substitute(text,".","::");

      if (current->section == Entry::PROTOCOLDOC_SEC) {
         current->m_entryName += "-p";
      }

      // prepend outer scope name
      BEGIN( ClassDocArg2 );
   }

<CategoryDocArg1>{SCOPENAME}{B}*"("[^\)]+")" {
      QString text  = QString::fromUtf8(yytext);

      lineCount();
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

      warn(yyFileName, yyLineNr, "Missing argument after \\%s",  csPrintable(s_currentCommand));
      unputString(text);

      BEGIN( Comment );
   }

<ClassDocArg1,CategoryDocArg1>.     {
      // ignore other stuff
   }

<ClassDocArg2>{DOCNL}         {
      QString text = QString::fromUtf8(yytext);
      unputString(text);

      BEGIN( Comment );
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
      unputString(text);

      BEGIN( Comment );
   }

<ClassDocArg3>.               {
      // ignore other stuff
   }

  /* --------- handle arguments of concept command ------------------- */

<ConceptDocArg1>{SCOPENAME}   {
      // first argument, concept name
      QString text = QString::fromUtf8(yytext);
      current->m_entryName = substitute(text, ".", "::");

      BEGIN(Comment);
   }

<ConceptDocArg1>{LC}          {
      ++yyLineNr;
      addToOutput('\n');
   }

<ConceptDocArg1>{DOCNL}       {
      // missing argument
      QString text = QString::fromUtf8(yytext);

      warn(yyFileName, yyLineNr, "Missing argument after \\concept");
      unputString(text);

      BEGIN( Comment );
   }

<ConceptDocArg1>.             {
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
      ++yyLineNr;
      addToOutput('\n');
   }

<GroupDocArg1>{DOCNL}         {
      // missing argument
      QString text = QString::fromUtf8(yytext);

      warn(yyFileName, yyLineNr, "Missing group name after %s", csPrintable(current->groupDocCmd()) );
      unputString(text);

      BEGIN( Comment );
   }

<GroupDocArg1>.               {
      // ignore other stuff
   }

<GroupDocArg2>"\\"{B}*"\n"    {
      // line continuation
      ++yyLineNr;
      addToOutput('\n');
   }

<GroupDocArg2>[^\n\\]+        {
      // title (stored in type)
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Type, text.trimmed());
   }

<GroupDocArg2>{DOCNL}         {
      QString text = QString::fromUtf8(yytext);

      if (current->groupDocType == Entry::GROUPDOC_NORMAL && current->getData(EntryKey::Member_Type).isEmpty()) {
         // defgroup requires second argument
         warn(yyFileName, yyLineNr, "Missing title after \\defgroup %s", csPrintable(current->m_entryName) );
      }

      unputString(text);

      BEGIN( Comment );
   }

<GroupDocArg2>.            {
      // title (stored in type)
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Type, text.trimmed());
   }

  /* --------- handle arguments of page/mainpage command ------------------- */

<PageDocArg1>{FILE}        {
      // first argument, page name
      QString text = QString::fromUtf8(yytext);

      current->m_entryName = stripQuotes(text);
      current->setData(EntryKey::Member_Args, QString());

      BEGIN( PageDocArg2 );
   }

<PageDocArg1>{LC}          {
      ++yyLineNr;
      addToOutput('\n');
   }

<PageDocArg1>{DOCNL}       {
      // missing argument
      QString text = QString::fromUtf8(yytext);

      warn(yyFileName, yyLineNr, "Missing argument after \\page");
      unputString(text);

      BEGIN( Comment );
   }

<PageDocArg1>.             {
      // ignore other stuff
   }

<PageDocArg2>{DOCNL}       {
      // second argument; page title
      QString text = QString::fromUtf8(yytext);
      unputString(text);

      BEGIN( Comment );
   }

<PageDocArg2>{CMD}[<>]        {

      QString text = QString::fromUtf8(yytext);

      text = substitute(substitute(text, "@<","&lt;"),  "@>", "&gt;");
      text = substitute(substitute(text, "\\<","&lt;"), "\\>","&gt;");

      current->appendData(EntryKey::Member_Args, text);
   }

<PageDocArg2>.             {
      QString text = QString::fromUtf8(yytext);
      current->appendData(EntryKey::Member_Args, text);
   }


  /* --------- handle arguments of the param command ------------ */
<ParamArg1>{ID}/{B}*","   {
      QString text = QString::fromUtf8(yytext);

      addToOutput(text);
   }

<ParamArg1>","   {
      addToOutput(" , ");
   }

<ParamArg1>{DOCNL}  {
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '\n') {
         ++yyLineNr;
      }

      addToOutput(" ");
    }

<ParamArg1>{ID}  {
      QString text = QString::fromUtf8(yytext);

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
      unputString(text);

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
      newXRefItemKey = QString::fromUtf8(yytext);
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
         ++yyLineNr;
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
         ++yyLineNr;
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
         ++yyLineNr;
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

      warn(yyFileName, yyLineNr, "Missing argument of \\%s command", csPrintable(s_currentCommand));
      unputString(text);

      BEGIN( Comment );
   }

<RelatesParam1>.        {
      // ignore other stuff
   }

  /* ----- handle arguments of the relates(also)/addindex commands ----- */

<LineParam>{DOCNL}         {
      // end of argument
      QString text = QString::fromUtf8(yytext);
      unputString(text);

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
      // first argument
      QString text = QString::fromUtf8(yytext);

      s_sectionLabel = text;

      addToOutput(text);
      s_sectionTitle.clear();

      BEGIN(SectionTitle);
   }

<SectionLabel>{DOCNL}         {
      // missing argument
      QString text = QString::fromUtf8(yytext);

      warn(yyFileName, yyLineNr, "\\section command has no label");

      if (text[0] == '\n') {
         ++yyLineNr;
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

      BEGIN(Comment);
   }

<SectionTitle>[^\n@\\]*"\\\\internal_linebr"      {
      // escaped end of section title
      QString text = QString::fromUtf8(yytext);

      s_sectionTitle += text;
   }

<SectionTitle>[^\n@\\]*/"\\internal_linebr"     {
      // end of section title
      QString text = QString::fromUtf8(yytext);

      addSection();
      addToOutput(text);
      BEGIN(Comment);
   }

<SectionTitle>{LC}         {
      // line continuation
      ++yyLineNr;
      addToOutput('\n');
   }

<SectionTitle>[^\n@\\]*          {
      // any character without special meaning
      QString text = QString::fromUtf8(yytext);

      s_sectionTitle += text;
      addToOutput(text);
   }

<SectionTitle>({CMD}{CMD}){ID}    {
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
         ++yyLineNr;
      }

      addToOutput('\n');

      BEGIN( Comment );
   }

<SubpageLabel>.      {
      unput(yytext[0]);
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
         ++yyLineNr;
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

      if (text.mid(4) == s_blockName)  {
         // found end of the block
         BEGIN(Comment);
      }
   }

<FormatBlock>{CMD}"enduml"              {
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);

      if (s_blockName == "startuml")   {
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
         ++yyLineNr;
      }

      addToOutput('\n');
   }

<FormatBlock>"/*"          {
      // */ (editor syntax fix)
      // start of a C-comment
      QString text = QString::fromUtf8(yytext);

      if (! (s_blockName == "code" || s_blockName == "verbatim")) {
         ++s_commentCount;
      }
      addToOutput(text);
   }

<FormatBlock>"*/"          {
      // end of a C-comment
      QString text = QString::fromUtf8(yytext);
      addToOutput(text);

      if (! (s_blockName == "code" || s_blockName == "verbatim")) {
         s_commentCount--;

         if (s_commentCount < 0) {
            warn(yyFileName, yyLineNr, "Found */ without matching /* while inside a \\%s block. "
               "Perhaps there is a missing \\end%s?\n", csPrintable(s_blockName), csPrintable(s_blockName));
         }
      }
   }

<FormatBlock>.             {
      // */ (editor syntax fix)
      QString text = QString::fromUtf8(yytext);
      addToOutput(text[0]);
   }

<FormatBlock><<EOF>>          {
      QString endTag = "end" + s_blockName;

      if (s_blockName == "startuml") {
         endTag = "enduml";
      }

      warn(yyFileName, yyLineNr, "Reached end of comment while inside a \\%s block, check for missing \\%s tag",
                 csPrintable(s_blockName), csPrintable(endTag) );

      yyterminate();
   }


  /* ----- handle arguments of if/ifnot commands ------- */

<GuardParam>{B}*"("                     {
      s_guardExpr  = QString::fromUtf8(yytext);
      s_roundCount = 1;

      BEGIN(GuardExpr);
   }

<GuardExpr>[^()]*                       {
      s_guardExpr += QString::fromUtf8(yytext);
      lineCount();
   }

<GuardExpr>"("                          {
      s_guardExpr += QString::fromUtf8(yytext);
      ++s_roundCount;
   }

<GuardExpr>")"                          {
      s_guardExpr += QString::fromUtf8(yytext);
      --s_roundCount;

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
         ++yyLineNr;
      }

      BEGIN( Comment );
   }

<GuardParam>{LC}        {
      // line continuation
      ++yyLineNr;
      addToOutput('\n');
   }

<GuardParam>.           {
      // ignore other stuff
      QString text = QString::fromUtf8(yytext);
      addToOutput(text[0]);
   }

<GuardParamEnd>{B}*{DOCNL}       {
      QString text = QString::fromUtf8(yytext);

      lineCount();
      s_spaceBeforeIf.clear();

      BEGIN(Comment);
   }

<GuardParamEnd>{B}*        {
      if (! s_spaceBeforeIf.isEmpty()) {
         addToOutput(s_spaceBeforeIf);
      }

      s_spaceBeforeIf.clear();
      BEGIN(Comment);
   }

<GuardParamEnd>.        {
      unput(yytext[0]);
      BEGIN(Comment);
   }

  /* ----- handle skipping of conditional sections ------- */

<SkipGuardedSection>{CMD}"ifnot"/{NW}  {
      s_guardType = Guard_IfNot;
      BEGIN( GuardParam );
   }

<SkipGuardedSection>{CMD}"if"/{NW}  {
      s_guardType = Guard_If;
      BEGIN( GuardParam );
   }

<SkipGuardedSection>{CMD}"endif"/{NW}  {
      if (s_guards.isEmpty()) {
         warn(yyFileName,yyLineNr, "Found \\endif without matching start command");

       } else {
         GuardedSection s   = s_guards.pop();
         bool parentVisible = s.parentVisible();

         if (parentVisible) {
            s_isEnabledSection = true;
            BEGIN( GuardParamEnd );
         }
      }
   }

<SkipGuardedSection>{CMD}"else"/{NW}   {
      if (s_guards.isEmpty()) {
         warn(yyFileName, yyLineNr, "Found \\else without matching start command");

      } else {

         if (! s_isEnabledSection && s_guards.top().parentVisible()) {
            s_guards.pop();
            s_guards.push(GuardedSection(true,true));

            s_isEnabledSection = true;
            BEGIN( GuardParamEnd );
         }
      }
   }

<SkipGuardedSection>{CMD}"elseif"/{NW}  {
      if (s_guards.isEmpty()) {
         warn(yyFileName,yyLineNr, "Found \\elseif without matching start command");

       } else {

         if (! s_isEnabledSection && s_guards.top().parentVisible()) {
            s_guardType = Guard_If;
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
         // handle conditional section around \internal

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
      unputString(text);

      BEGIN(Comment);
   }

<NameParam>{LC}            {
      // line continuation
      yyLineNr++;
      addToOutput('\n');
      s_memberGroupHeader += ' ';
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

      unputString(text);

      BEGIN(Comment);
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

      if (s_braceCount == 0) {
         unputString(text);

         if (langParser != nullptr) {
            // not used for clang parsing
            langParser->parsePrototype(s_functionProto);
         }

         BEGIN( Comment );
      }
   }

<FnParam>{LC}           {
      // line continuation
      yyLineNr++;
      s_functionProto += ' ';
   }

<FnParam>[^@\\\n()]+          {
      // non-special characters
      s_functionProto += QString::fromUtf8(yytext);
   }

<FnParam>"("            {
      s_functionProto += QString::fromUtf8(yytext);
      ++s_braceCount;
   }

<FnParam>")"            {
      s_functionProto += QString::fromUtf8(yytext);
      --s_braceCount;
   }

<FnParam>.           {
      // add other stuff
      QString text = QString::fromUtf8(yytext);
      s_functionProto += text[0];
   }


  /* ----- handle argument of overload command ------- */

<OverloadParam>{DOCNL}        {
      // end of overload declaration, pass to langParser as a "prototype"
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '\n')  {
         yyLineNr++;
      }

      if (s_functionProto.trimmed().isEmpty()) {
         // plain overload command
         addToOutput(theTranslator->trOverloadText());
         addToOutput('\n');

      }  else   {
         // overload declaration
         makeStructuralIndicator(Entry::OVERLOADDOC_SEC);

         if (langParser != nullptr) {
            // not used for clang parsing
            langParser->parsePrototype(s_functionProto);
         }
      }

      BEGIN( Comment );
   }

<OverloadParam>{LC}        {
      // line continuation
      yyLineNr++;
      s_functionProto += ' ';
   }

<OverloadParam>.        {
      // add other stuff
      QString text = QString::fromUtf8(yytext);
      s_functionProto += text[0];
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

      warn(yyFileName,yyLineNr, "Missing argument for \\%s command", csPrintable(s_currentCommand));
      unputString(text);

      BEGIN( Comment );
   }

<ExtendsParam>.            {
      // ignore other stuff
   }


  /* ----- handle language specific sections ------- */

<SkipLang>[\\@]"~"[a-zA-Z-]*        {
      // language switch
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
      // new line in verbatim block
      QString text = QString::fromUtf8(yytext);

      if (text[0] == '\n') {
         ++yyLineNr;
      }
   }

<SkipLang>.             {
      // any other character
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

      warn(yyFileName, yyLineNr, "\\cite command has no label");
      unputString(text);

      BEGIN( Comment );
   }

<CiteLabel>.            {
      // invalid character for cite label
      warn(yyFileName, yyLineNr, "Invalid or missing cite label");
      BEGIN(Comment);
   }


  /* ----- handle argument of the copydoc command ------- */

<CopyDoc><<EOF>>        {
      setOutput(OutputMainDoc);
      addToOutput(" \\ilinebr\\ilinebr\\copydetails ");
      addToOutput(s_copyDocArg);

      addToOutput("\n");

      BEGIN(Comment);
   }

<CopyDoc>{DOCNL}        {
      QString text = QString::fromUtf8(yytext);
      if (text[0] == '\n') {
         ++yyLineNr;
      }

      if (s_braceCount == 0) {

         setOutput(OutputMainDoc);
         addToOutput(" \\ilinebr\\ilinebr\\copydetails ");
         addToOutput(s_copyDocArg);

         addToOutput("\n");

         BEGIN(Comment);
      }
   }

<CopyDoc>{LC}              {
      // line continuation
      ++yyLineNr;
   }

<CopyDoc>[^@\\\n()]+          {
      QString text = QString::fromUtf8(yytext);

      s_copyDocArg += text;
      addToOutput(text);
   }

<CopyDoc>"("                  {
      QString text = QString::fromUtf8(yytext);

      s_copyDocArg += text;
      addToOutput(text);

      ++s_braceCount;
   }

<CopyDoc>")"                  {
      QString text = QString::fromUtf8(yytext);

      s_copyDocArg += text;
      addToOutput(text);

      --s_braceCount;
   }

<CopyDoc>.           {
      QString text = QString::fromUtf8(yytext);

      s_copyDocArg += text;
      addToOutput(text);
   }

%%

static bool handleBrief(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   setOutput(OutputBrief);
   return false;
}

static bool handleProperty(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   bool stop = makeStructuralIndicator(Entry::MEMBERDOC_SEC);
   s_functionProto.clear();
   s_braceCount = 0;
   BEGIN(FnParam);

   current->mtype = MethodType::Property;

   return stop;
}

static bool handleFn(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   bool stop = makeStructuralIndicator(Entry::MEMBERDOC_SEC);
   s_functionProto.clear();
   s_braceCount = 0;
   BEGIN(FnParam);

   return stop;
}

static bool handleDef(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   bool stop = makeStructuralIndicator(Entry::DEFINEDOC_SEC);
   s_functionProto.clear();
   BEGIN(FnParam);

   return stop;
}

static bool handleOverload(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   s_functionProto.clear();
   BEGIN(OverloadParam);

   return false;
}

static bool handleEnum(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   bool stop = makeStructuralIndicator(Entry::ENUMDOC_SEC);
   BEGIN(EnumDocArg1);

   return stop;
}

static bool handleDefGroup(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   bool stop = makeStructuralIndicator(Entry::GROUPDOC_SEC);
   current->groupDocType = Entry::GROUPDOC_NORMAL;
   BEGIN( GroupDocArg1 );

   return stop;
}

static bool handleAddToGroup(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   bool stop = makeStructuralIndicator(Entry::GROUPDOC_SEC);
   current->groupDocType = Entry::GROUPDOC_ADD;
   BEGIN( GroupDocArg1 );

   return stop;
}

static bool handleWeakGroup(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   bool stop = makeStructuralIndicator(Entry::GROUPDOC_SEC);
   current->groupDocType = Entry::GROUPDOC_WEAK;
   BEGIN( GroupDocArg1 );

   return stop;
}

static bool handleNamespace(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   bool stop = makeStructuralIndicator(Entry::NAMESPACEDOC_SEC);
   BEGIN( NameSpaceDocArg1 );

   return stop;
}

static bool handlePackage(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   bool stop = makeStructuralIndicator(Entry::PACKAGEDOC_SEC);
   BEGIN( PackageDocArg1 );

   return stop;
}

static bool handleClass(const QString &str, const QStringList &list)
{
   (void) list;

   bool stop = makeStructuralIndicator(Entry::CLASSDOC_SEC);
   s_currentCommand = str;
   BEGIN( ClassDocArg1 );

   return stop;
}

static bool handleConcept(const QString &str, const QStringList &list)
{
   (void) list;

   bool stop = makeStructuralIndicator(Entry::CONCEPTDOC_SEC);
   s_currentCommand = str;
   BEGIN( ConceptDocArg1 );

   return stop;
}

static bool handleHeaderFile(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   BEGIN( ClassDocArg2 );
   return false;
}

static bool handleProtocol(const QString &str, const QStringList &list)
{
   (void) list;

   // Obj-C protocol
   bool stop = makeStructuralIndicator(Entry::PROTOCOLDOC_SEC);
   s_currentCommand = str;
   BEGIN( ClassDocArg1 );

   return stop;
}

static bool handleCategory(const QString &str, const QStringList &list)
{
   (void) list;

   // Obj-C category
   bool stop = makeStructuralIndicator(Entry::CATEGORYDOC_SEC);
   s_currentCommand = str;
   BEGIN( CategoryDocArg1 );

   return stop;
}

static bool handleUnion(const QString &str, const QStringList &list)
{
   (void) list;

   bool stop = makeStructuralIndicator(Entry::UNIONDOC_SEC);
   s_currentCommand = str;
   BEGIN( ClassDocArg1 );

   return stop;
}

static bool handleStruct(const QString &str, const QStringList &list)
{
   (void) list;

   bool stop=makeStructuralIndicator(Entry::STRUCTDOC_SEC);
   s_currentCommand = str;
   BEGIN( ClassDocArg1 );

   return stop;
}

static bool handleInterface(const QString &str, const QStringList &list)
{
   (void) list;

   bool stop = makeStructuralIndicator(Entry::INTERFACEDOC_SEC);
   s_currentCommand = str;
   BEGIN( ClassDocArg1 );

   return stop;
}

static bool handleIdlException(const QString &str, const QStringList &list)
{
   (void) list;

   bool stop = makeStructuralIndicator(Entry::EXCEPTIONDOC_SEC);
   s_currentCommand = str;
   BEGIN( ClassDocArg1 );

   return stop;
}

static bool handlePage(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   bool stop = makeStructuralIndicator(Entry::PAGEDOC_SEC);
   BEGIN( PageDocArg1 );

   return stop;
}

static bool handleMainpage(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   bool stop = makeStructuralIndicator(Entry::MAINPAGEDOC_SEC);

   if (stop) {
      current->m_entryName = QString();
   } else {
      current->m_entryName = "mainpage";
   }

   setOutput(OutputMainDoc);
   BEGIN( PageDocArg2 );

   return stop;
}

static bool handleFile(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   bool stop = makeStructuralIndicator(Entry::FILEDOC_SEC);

   if (! stop) {
      current->m_entryName = yyFileName;
   }

   BEGIN( FileDocArg1 );

   return stop;
}

static bool handleParam(const QString &str, const QStringList &list)
{
  // process param and retval arguments to escape leading underscores
  // in case of markdown processing

   (void) str;
   (void) list;

   addToOutput("@param ");
   BEGIN( ParamArg1 );

   return false;
}

static bool handleRetval(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   addToOutput("@retval ");
   BEGIN( ParamArg1 );

   return false;
}

static bool handleDir(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   bool stop = makeStructuralIndicator(Entry::DIRDOC_SEC);

   if (! stop) {
      current->m_entryName = yyFileName;
   }

   BEGIN( FileDocArg1 );

   return stop;
}

static bool handleExample(const QString &str, const QStringList &list)
{
   (void) str;

   Entry::Sections section = Entry::EXAMPLE_SEC;

   for (auto item : list) {
      QString opt = item.trimmed().toLower();

      if (opt == "lineno") {
         section = Entry::EXAMPLE_LINENO_SEC;

      } else {
         warn(yyFileName, yyLineNr, "Unsupported option '%s' for command '\\%s'", csPrintable(opt), csPrintable(str));
      }
   }

   bool stop = makeStructuralIndicator(section);

   if (! stop) {
      current->m_entryName = yyFileName;
   }

   BEGIN( FileDocArg1 );

   return stop;
}

static bool handleDetails(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   if (inContext != OutputBrief) {
      // treat @details outside brief description as a new paragraph
      addToOutput("\\internal_linebr\\internal_linebr");
   }

   setOutput(OutputMainDoc);

   return false;
}

static bool handleName(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   bool stop = makeStructuralIndicator(Entry::MEMBERGRP_SEC);

   if (! stop) {
      s_memberGroupHeader.resize(0);
      BEGIN( NameParam );

      if (s_memberGroupId != DOX_NOGROUP) {
         // end of previous member group
         closeGroup(current, yyFileName, yyLineNr, true, true);
       }
   }

   return stop;
}

static bool handleTodo(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   newXRefKind = XRef_Todo;
   setOutput(OutputXRef);
   xrefKind = XRef_Todo;

   return false;
}

static bool handleTest(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   newXRefKind = XRef_Test;
   setOutput(OutputXRef);
   xrefKind = XRef_Test;

   return false;
}

static bool handleBug(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   newXRefKind = XRef_Bug;
   setOutput(OutputXRef);
   xrefKind = XRef_Bug;

   return false;
}

static bool handleDeprecated(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   newXRefKind = XRef_Deprecated;
   setOutput(OutputXRef);
   xrefKind = XRef_Deprecated;

   return false;
}

static bool handleXRefItem(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   newXRefKind = XRef_Item;
   BEGIN(XRefItemParam1);

   return false;
}

static bool handleParBlock(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

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

static bool handleEndParBlock(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   if (! s_insideParBlock) {
      warn(yyFileName,yyLineNr, "Found \\endparblock command without matching \\parblock");
   }

   addToOutput("@endparblock");
   setOutput(OutputMainDoc);    // to end a parblock inside a xrefitem like context
   s_insideParBlock = false;

   return false;
}

static bool handleRelated(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   if (! current->getData(EntryKey::Related_Class).isEmpty()) {
      warn(yyFileName,yyLineNr,
                  "Found multiple \\relates, \\relatesalso or \\memberof commands in a comment block, using last definition");
   }

   current->relatesType = Simple;
   BEGIN(RelatesParam1);

   return false;
}

static bool handleRelatedAlso(const QString &str, const QStringList &list)
{

   (void) list;

   if (! current->getData(EntryKey::Related_Class).isEmpty()) {
      warn(yyFileName, yyLineNr,
                  "Found multiple \\relates, \\relatesalso or \\memberof commands in a comment block, using last definition");
   }

   current->relatesType = Duplicate;
   s_currentCommand = str;
   BEGIN(RelatesParam1);

   return false;
}

static bool handleMemberOf(const QString &str, const QStringList &list)
{
   (void) list;

   if (! current->getData(EntryKey::Related_Class).isEmpty()) {
      warn(yyFileName, yyLineNr,
                  "Found multiple \\relates, \\relatesalso or \\memberof commands in a comment block, using last definition");
   }

   current->relatesType = MemberOf;
   s_currentCommand = str;
   BEGIN(RelatesParam1);

   return false;
}

static bool handleRefItem(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   addToOutput("@refitem ");
   BEGIN(LineParam);

   return false;
}

static bool handleSection(const QString &str, const QStringList &list)
{
   (void) list;

   setOutput(OutputMainDoc);
   addToOutput("@" + str + " ");

   BEGIN(SectionLabel);

   if (str == "section") {
      s_sectionLevel = 1;

   } else if (str == "subsection") {
      s_sectionLevel = 2;

   } else if (str == "subsubsection") {
      s_sectionLevel = 3;

   } else if (str == "paragraph") {
      s_sectionLevel = 4;

   }

   return false;
}

static bool handleSubpage(const QString &str, const QStringList &list)
{
   (void) list;

   if (current->section != Entry::EMPTY_SEC && current->section != Entry::PAGEDOC_SEC &&
         current->section != Entry::MAINPAGEDOC_SEC) {

      warn(yyFileName, yyLineNr, "Found \\subpage command in a comment block that is not marked as a page");
  }

  if (! s_spaceBeforeCmd.isEmpty()) {
     addToOutput(s_spaceBeforeCmd);
      s_spaceBeforeCmd.clear();
  }

   addToOutput("@" + str + " ");
   BEGIN(SubpageLabel);

   return false;
}

static bool handleAnchor(const QString &str, const QStringList &list)
{
   (void) list;

   addToOutput("@" + str + " ");
   BEGIN(AnchorLabel);

   return false;
}

static bool handleCite(const QString &str, const QStringList &list)
{
   (void) list;

   if (! s_spaceBeforeCmd.isEmpty()) {
     addToOutput(s_spaceBeforeCmd);
     s_spaceBeforeCmd.resize(0);
   }

   addToOutput("@" + str + " ");
   BEGIN(CiteLabel);

   return false;
}

static bool handleFormatBlock(const QString &str, const QStringList &list)
{
   if (! s_spaceBeforeCmd.isEmpty()) {
      addToOutput(s_spaceBeforeCmd);
      s_spaceBeforeCmd.clear();
   }
   if (list.isEmpty()) {
      addToOutput("@" + str + " ");

   } else {
      addToOutput("@" + str + "{" + list.join(",") + "} ");
   }

   s_blockName    = str;
   s_commentCount = 0;
   BEGIN(FormatBlock);

   return false;
}

static bool handleAddIndex(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   addToOutput("@addindex ");

   BEGIN(LineParam);

   return false;
}

static bool handleIf(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   s_isEnabledSection = false;
   s_guardType        = Guard_If;
   s_spaceBeforeIf    = s_spaceBeforeCmd;
   BEGIN(GuardParam);

   return false;
}

static bool handleIfNot(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   s_isEnabledSection = false;
   s_guardType        = Guard_IfNot;
   s_spaceBeforeIf    = s_spaceBeforeCmd;

   BEGIN(GuardParam);

   return false;
}

static bool handleElseIf(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   if (s_guards.isEmpty()) {
      warn(yyFileName,yyLineNr, "Found \\else without matching start command");

   } else {
      s_guardType     = s_isEnabledSection ? Guard_Skip : Guard_If;
      s_spaceBeforeIf = s_spaceBeforeCmd;
      BEGIN(GuardParam);
   }

   return false;
}

static bool handleElse(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   if (s_guards.isEmpty()) {
      warn(yyFileName,yyLineNr, "Found \\else without matching start command");

   } else {
      s_spaceBeforeIf = s_spaceBeforeCmd;
      BEGIN( SkipGuardedSection );
   }

   return false;
}

static bool handleEndIf(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   if (s_guards.isEmpty())   {
      warn(yyFileName,yyLineNr, "Found \\endif without matching start command");

   } else {
      s_guards.pop();
   }

   s_isEnabledSection = false;
   if (! s_spaceBeforeCmd.isEmpty()) {
      addToOutput(s_spaceBeforeCmd);
      s_spaceBeforeCmd.resize(0);
   }

   BEGIN( GuardParamEnd );

   return false;
}

static bool handleIngroup(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   inGroupParamFound = false;
   BEGIN( InGroupParam );

   return false;
}

static bool handleNoSubGrouping(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->subGrouping = false;

   return false;
}

static bool handleShowInitializer(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->initLines = 100000;  // ON
   return false;
}

static bool handleHideInitializer(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->initLines = 0;

   return false;
}

static bool handleCallgraph(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->callGraph = true;

   return false;
}

static bool handleHideCallgraph(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->callGraph = false;
   return false;
}

static bool handleCallergraph(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->callerGraph = true;
   return false;
}

static bool handleHideCallergraph(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->callerGraph = false;

   return false;
}

static bool handleReferencedByRelation(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->referencedByRelation = true;

   return false;
}

static bool handleHideReferencedByRelation(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->referencedByRelation = false;

   return false;
}

static bool handleReferencesRelation(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->referencesRelation = true;

   return false;
}

static bool handleHideReferencesRelation(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->referencesRelation = false;

   return false;
}

static bool handleInternal(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   if (s_internalDocs) {
      addToOutput(" \\internal ");
      s_processInternalDocs = true;

   } else {
      // make sure some whitespace before an \internal command
      // is not treated as "documentation"

      if (current->getData(EntryKey::Main_Docs).trimmed().isEmpty()) {
         current->setData(EntryKey::Main_Docs, QString());
      }

      s_condCount = 0;
      BEGIN( SkipInternal );
  }

  return false;
}

static bool handleStatic(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   endBrief();
   current->m_static = true;

   return false;
}

static bool handlePure(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   endBrief();
   current->virt = Specifier::Pure;

   return false;
}

static bool handlePrivate(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->protection = Protection::Private;

   return false;
}

static bool handlePrivateSection(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->protection = (s_protection = Protection::Private);

   return false;
}

static bool handleProtected(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->protection = Protection::Protected;

   return false;
}

static bool handleProtectedSection(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->protection = (s_protection = Protection::Protected);

   return false;
}

static bool handlePublic(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->protection = Protection::Public;

   return false;
}

static bool handlePublicSection(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   current->protection = (s_protection = Protection::Public);
   return false;
}

static bool handleToc(const QString &str, const QStringList &list)
{
   (void) str;

   if (current->section == Entry::PAGEDOC_SEC || current->section == Entry::MAINPAGEDOC_SEC) {

      for (auto &item : list) {
         QString option = item.trimmed().toLower();

         int level = 5;
         int i     = option.find(':');

         if (i > 0) {
            // found something like "html:4"
            bool ok = false;
            level = option.mid(i + 1).toInteger<int>(&ok);

            if (ok) {
               level  = (level >  5 ? 5 : level);
               level  = (level <= 0 ? 5 : level);
               option = option.left(i).trimmed();

            } else {
               warn(yyFileName, yyLineNr, "Unknown option level specified with \\tableofcontents: `%s'", csPrintable(option));
               option = "";
            }
         }

         if (! option.isEmpty()) {
            if (option == "html") {
               current->localToc.enableHtml(level);

            } else if (option == "latex") {
               current->localToc.enableLatex(level);

            } else if (option == "xml") {
               current->localToc.enableXml(level);

            } else if (option == "docbook") {
               current->localToc.enableDocbook(level);

            } else {
               warn(yyFileName, yyLineNr, "Unknown option specified with \\tableofcontents: `%s'", csPrintable(option));
            }
         }
      }

      if (current->localToc.nothingEnabled()) {
         // for backward compatibility
         current->localToc.enableHtml(5);
         current->localToc.enableXml(5);
      }
   }

   return false;
}

static bool handleInherit(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   BEGIN(InheritParam);

   return false;
}

static bool handleExtends(const QString &str, const QStringList &list)
{
   (void) list;
   s_currentCommand = str;
   BEGIN(ExtendsParam);

   return false;
}

static bool handleCopyBrief(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   if (current->getData(EntryKey::Brief_Docs).isEmpty() && current->getData(EntryKey::Main_Docs).isEmpty()) {

      // if we do not have a brief or detailed description yet,
      // then the @copybrief should end up in the brief description,
      // otherwise it will be copied inline

      setOutput(OutputBrief);
   }

   if (! s_spaceBeforeCmd.isEmpty()) {
      addToOutput(s_spaceBeforeCmd);
      s_spaceBeforeCmd.resize(0);
   }

   addToOutput("\\copybrief ");
   return false;
}

static bool handleCopyDetails(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

   setOutput(OutputMainDoc);

   if (! s_spaceBeforeCmd.isEmpty()) {
     addToOutput(s_spaceBeforeCmd);
     s_spaceBeforeCmd.resize(0);
   }

   addToOutput("\\copydetails ");
   return false;
}

static bool handleCopyDoc(const QString &str, const QStringList &list)
{
   (void) str;
   (void) list;

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
   if (YY_START == ReadFormulaShort || YY_START == ReadFormulaRound  || YY_START == ReadFormulaLong) {
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

   if (Doxy_Globals::markdownSupport) {

      s_inputString = processMarkdown(fileName, lineNr, QSharedPointer<Entry>(), comment);

      QStringView tmp(s_inputString);

      while (tmp.startsWith(" ")) {
         tmp = tmp.mid(1);
      }

      while (tmp.startsWith("\n")) {
         tmp = tmp.mid(1);
      }

      if (tmp.startsWith("<br>")) {
         tmp = tmp.mid(4);
      }

      s_inputString = QString(tmp);


   } else {
      s_inputString  = comment;
   }

   s_inputString.append(" ");

   yyFileName       = fileName;
   yyLineNr         = lineNr;
   langParser       = parser;
   current          = curEntry;
   current->docLine = (lineNr > 1 ? lineNr : 1);

   briefEndsAtDot   = isAutoBrief;
   inBody           = isInbody;
   s_protection     = r_protection;
   s_inputPosition  = r_position;
   s_outputXRef     = QString();
   xrefKind         = XRef_None;

   xrefAppendFlag   = false;
   insidePre        = false;
   s_needNewEntry   = false;
   s_parseMore      = false;

   if (! isBrief && ! isAutoBrief && ! current->getData(EntryKey::Main_Docs).isEmpty()) {
      // add newline separator between detailed comment blocks
      current->appendData(EntryKey::Main_Docs, "\n");
   }

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
      current->appendData(EntryKey::Main_Docs, "\n\n");
   }

   if (! current->getData(EntryKey::Inbody_Docs).isEmpty() && isInbody) {
      // separate in body fragments
      current->appendData(EntryKey::Inbody_Docs, "\n\n");
   }

   yyrestart(yyin);
   BEGIN(Comment);
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
      openGroup(current, yyFileName, yyLineNr, true);
   }

   checkFormula();
   r_protection = s_protection;

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
   s_openCount = 0;
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

   } else if (s_openCount > 0)  {
      warn(fileName, line, "End of file with unbalanced group command\n");
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

void openGroup(QSharedPointer<Entry> e, const QString &, int, bool forceOpen)
{
   if (! forceOpen) {
      ++s_openCount;
   }

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

void closeGroup(QSharedPointer<Entry> e, const QString &fileName, int line, bool isInline, bool forceClose)
{
   if (! forceClose) {

      if (s_openCount < 1) {
         warn(fileName, line, "Unbalanced Group Command");
      } else {
         --s_openCount;
      }
   }

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

      if (! isInline) {
         e->mGrpId = DOX_NOGROUP;
      }

   } else if (! s_autoGroupStack.isEmpty()) {
      // end of auto group
      QSharedPointer<Grouping> grp = s_autoGroupStack.pop();

      if (! isInline && ! e->m_groups.empty()) {
         e->m_groups.removeLast();
      }

      if (! isInline) {
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
      if ( (sectionEnabled && s_guardType == Guard_If) || (! sectionEnabled && s_guardType == Guard_IfNot)) {
         // section is visible
         s_guards.push(GuardedSection(true, true));
         s_isEnabledSection = true;
         BEGIN( GuardParamEnd );

      } else {
         // section is invisible
         if (s_guardType != Guard_Skip) {
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
