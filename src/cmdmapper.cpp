/************************************************************************
*
* Copyright (c) 2014-2022 Barbara Geller & Ansel Sermersheim
* Copyright (c) 1997-2014 Dimitri van Heesch
*
* DoxyPress is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License version 2
* as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
*************************************************************************/

#include <cmdmapper.h>

/** Call representing a mapping from a command name to a command ID. */
struct CommandMap {

   CommandMap(const char *x1, int x2) {
      cmdName = QString::fromLatin1(x1);
      cmdId   = x2;
   }

   QString cmdName;
   int cmdId;
};

CommandMap cmdMap[] = {
   { "a",               CMD_EMPHASIS },
   { "addindex",        CMD_ADDINDEX },
   { "anchor",          CMD_ANCHOR },
   { "anchorname",      CMD_ANCHORNAME },
   { "arg",             CMD_LI },
   { "attention",       CMD_ATTENTION },
   { "author",          CMD_AUTHOR },
   { "authors",         CMD_AUTHORS },
   { "b",               CMD_BOLD },
   { "bypassundocwarn", CMD_BYPASSUNDOCWARN },
   { "c",               CMD_CODE },
   { "cite",            CMD_CITE },
   { "code",            CMD_STARTCODE },
   { "copydoc",         CMD_COPYDOC },
   { "copybrief",       CMD_COPYBRIEF },
   { "copydetails",     CMD_COPYDETAILS },
   { "copyright",       CMD_COPYRIGHT },
   { "date",            CMD_DATE },
   { "docbookonly",     CMD_DBONLY },
   { "dontinclude",     CMD_DONTINCLUDE },
   { "dotfile",         CMD_DOTFILE },
   { "e",               CMD_EMPHASIS },
   { "em",              CMD_EMPHASIS },
   { "endcode",         CMD_ENDCODE },
   { "enddocbookonly",  CMD_ENDDBONLY },
   { "endhtmlonly",     CMD_ENDHTMLONLY },
   { "endlatexonly",    CMD_ENDLATEXONLY },
   { "endlink",         CMD_ENDLINK },
   { "endmanonly",      CMD_ENDMANONLY },
   { "endrtfonly",      CMD_ENDRTFONLY },
   { "endsecreflist",   CMD_ENDSECREFLIST },
   { "endverbatim",     CMD_ENDVERBATIM },
   { "endxmlonly",      CMD_ENDXMLONLY },
   { "exception",       CMD_EXCEPTION },
   { "form",            CMD_FORMULA },
   { "htmlinclude",     CMD_HTMLINCLUDE },
   { "htmlonly",        CMD_HTMLONLY },
   { "image",           CMD_IMAGE },
   { "include",         CMD_INCLUDE },
   { "includelineno",   CMD_INCWITHLINES },
   { "includedoc",      CMD_INCLUDEDOC },
   { "inheritdoc",      CMD_INHERITDOC },
   { "internal",        CMD_INTERNAL },
   { "invariant",       CMD_INVARIANT },
   { "javalink",        CMD_JAVALINK },
   { "latexinclude",    CMD_LATEXINCLUDE },
   { "latexonly",       CMD_LATEXONLY },
   { "li",              CMD_LI },
   { "line",            CMD_LINE },
   { "link",            CMD_LINK },
   { "manonly",         CMD_MANONLY },
   { "n",               CMD_LINEBREAK },
   { "note",            CMD_NOTE },
   { "p",               CMD_CODE },
   { "par",             CMD_PAR },
   { "paragraph",       CMD_PARAGRAPH },
   { "param",           CMD_PARAM },
   { "post",            CMD_POST },
   { "pre",             CMD_PRE },
   { "ref",             CMD_REF },
   { "refitem",         CMD_SECREFITEM },
   { "remark",          CMD_REMARK },
   { "remarks",         CMD_REMARK },
   { "result",          CMD_RETURN },
   { "return",          CMD_RETURN },
   { "returns",         CMD_RETURN },
   { "retval",          CMD_RETVAL },
   { "rtfonly",         CMD_RTFONLY },
   { "sa",              CMD_SA },
   { "secreflist",      CMD_SECREFLIST },
   { "section",         CMD_SECTION },
   { "snippet",         CMD_SNIPPET },
   { "snippetdoc",      CMD_SNIPPETDOC },
   { "snippetlineno",   CMD_SNIPWITHLINES },
   { "sortid",          CMD_SORTID },
   { "subpage",         CMD_SUBPAGE },
   { "subsection",      CMD_SUBSECTION },
   { "subsubsection",   CMD_SUBSUBSECTION },
   { "see",             CMD_SA },
   { "since",           CMD_SINCE },
   { "skip",            CMD_SKIP },
   { "skipline",        CMD_SKIPLINE },
   { "throw",           CMD_EXCEPTION },
   { "throws",          CMD_EXCEPTION },
   { "tparam",          CMD_TPARAM },
   { "until",           CMD_UNTIL },
   { "verbatim",        CMD_VERBATIM },
   { "verbinclude",     CMD_VERBINCLUDE },
   { "warning",         CMD_WARNING },
   { "version",         CMD_VERSION },
   { "xmlonly",         CMD_XMLONLY },
   { "xrefitem",        CMD_XREFITEM },
   { "\\",              CMD_BSLASH },
   { "@",               CMD_AT },
   { "<",               CMD_LESS },
   { "=",               CMD_EQUAL },
   { ">",               CMD_GREATER },
   { "&",               CMD_AMP },
   { "$",               CMD_DOLLAR },
   { "#",               CMD_HASH },
   { "%",               CMD_PERCENT },
   { "|",               CMD_PIPE },
   { ".",               CMD_PUNT },
   { "+",               CMD_PLUS },
   { "-",               CMD_MINUS },
   { "::",              CMD_DCOLON },
   { "\"",              CMD_QUOTE },
   { "--",              CMD_NDASH },
   { "---",             CMD_MDASH },
   { "_internalref",    CMD_INTERNALREF },
   { "_setscope",       CMD_SETSCOPE },
   { "dot",             CMD_DOT },
   { "msc",             CMD_MSC },
   { "mscfile",         CMD_MSCFILE },
   { "startuml",        CMD_STARTUML },
   { "enddot",          CMD_ENDDOT },
   { "endmsc",          CMD_ENDMSC },
   { "enduml",          CMD_ENDUML },
   { "endinternal",     CMD_ENDINTERNAL },
   { "parblock",        CMD_PARBLOCK },
   { "endparblock",     CMD_ENDPARBLOCK },
   { "diafile",         CMD_DIAFILE },
   { "emoji",           CMD_EMOJI },
   { "rtfinclude",      CMD_RTFINCLUDE },
   { "docbookinclude",  CMD_DOCBOOKINCLUDE },
   { "maninclude",      CMD_MANINCLUDE },
   { "xmlinclude",      CMD_XMLINCLUDE },
   { 0,                 0 },
};

CommandMap htmlTagMap[] = {
   { "strong",     HTML_BOLD },
   { "center",     HTML_CENTER },
   { "table",      HTML_TABLE },
   { "caption",    HTML_CAPTION },
   { "small",      HTML_SMALL },
   { "cite",       HTML_CITE },
   { "code",       HTML_CODE },
   { "dfn",        HTML_CODE },
   { "var",        HTML_EMPHASIS },
   { "img",        HTML_IMG },
   { "pre",        HTML_PRE },
   { "sub",        HTML_SUB },
   { "sup",        HTML_SUP },
   { "tr",         HTML_TR },
   { "td",         HTML_TD },
   { "th",         HTML_TH },
   { "ol",         HTML_OL },
   { "ul",         HTML_UL },
   { "li",         HTML_LI },
   { "tt",         XML_C /*HTML_CODE*/ },
   { "kbd",        XML_C /*HTML_CODE*/ },
   { "em",         HTML_EMPHASIS },
   { "hr",         HTML_HR },
   { "dl",         HTML_DL },
   { "dt",         HTML_DT },
   { "dd",         HTML_DD },
   { "br",         HTML_BR },
   { "i",          HTML_EMPHASIS },
   { "a",          HTML_A },
   { "b",          HTML_BOLD },
   { "p",          HTML_P },
   { "h1",         HTML_H1 },
   { "h2",         HTML_H2 },
   { "h3",         HTML_H3 },
   { "h4",         HTML_H4 },
   { "h5",         HTML_H5 },
   { "h6",         HTML_H6 },
   { "span",       HTML_SPAN },
   { "div",        HTML_DIV },
   { "blockquote", HTML_BLOCKQUOTE },
   { "strike",     HTML_STRIKE },
   { "s",          HTML_S },
   { "u",          HTML_UNDERLINE },
   { "ins",        HTML_INS },
   { "del",        HTML_DEL },
   { "details",    HTML_DETAILS },

   { "c",            XML_C },
// { "code",         XML_CODE },          // ambiguous <code> is also a HTML tag
   { "description",  XML_DESCRIPTION },
   { "example",      XML_EXAMPLE },
   { "exception",    XML_EXCEPTION },
   { "include",      XML_INCLUDE },
   { "item",         XML_ITEM },
   { "list",         XML_LIST }, // type="table|bullet|number"
   { "listheader",   XML_LISTHEADER },
   { "para",         XML_PARA },
   { "param",        XML_PARAM },
   { "paramref",     XML_PARAMREF },
   { "typeparam",    XML_TYPEPARAM },
   { "typeparamref", XML_TYPEPARAMREF },
   { "permission",   XML_PERMISSION },
   { "remarks",      XML_REMARKS },
   { "returns",      XML_RETURNS },
   { "see",          XML_SEE },
   { "seealso",      XML_SEEALSO },
   { "summary",      XML_SUMMARY },
   { "term",         XML_TERM },
   { "value",        XML_VALUE },
   { "inheritdoc",   XML_INHERITDOC },
   { "",             0 }
};

Mapper *Mappers::cmdMapper     = new Mapper(cmdMap, true);
Mapper *Mappers::htmlTagMapper = new Mapper(htmlTagMap, false);

Mapper::Mapper(const CommandMap *cm, bool caseSensitive) : m_cs(caseSensitive)
{
   const CommandMap *p = cm;

   while (! p->cmdName.isEmpty()) {
      m_map.insert(p->cmdName, p->cmdId);
      p++;
   }
}

int Mapper::map(const QString &n)
{
   QString name = n;

   if (! m_cs) {
      name = name.toLower();
   }

   int retval  = 0;

   if (name.isEmpty()) {
      retval = 0;

   } else {
      auto result = m_map.find(name);

      if (result != m_map.end()) {
         retval = result.value();
      }
   }

   return retval;
}

QString Mapper::map(const int n)
{
   QString retval;

   for (auto item = m_map.begin(); item != m_map.end(); ++item) {
      int value = item.value();

      if (value == n || (value == (n | SIMPLESECT_BIT))) {
         return item.key();
      }
   }

   return retval;
}

void Mappers::freeMappers()
{
   delete cmdMapper;
   cmdMapper = 0;

   delete htmlTagMapper;
   htmlTagMapper = 0;
}


