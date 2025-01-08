/************************************************************************
*
* Copyright (c) 2014-2025 Barbara Geller & Ansel Sermersheim
*
* Copyright (c) 1997-2014 Dimitri van Heesch
*
* DoxyPress is free software. You can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
* https://www.gnu.org/licenses/
*
*************************************************************************/

#ifndef CMDMAPPER_H
#define CMDMAPPER_H

#include <QHash>

struct CommandMap;

const int SIMPLESECT_BIT = 0x1000;

enum CommandType {
   CMD_UNKNOWN         = 0,
   CMD_ADDINDEX        = 1,
   CMD_AMP             = 2,
   CMD_ANCHOR          = 3,
   CMD_ANCHORNAME      = 4,      // cs
   CMD_AT              = 5,
   CMD_ATTENTION       = 6  | SIMPLESECT_BIT,
   CMD_AUTHOR          = 7  | SIMPLESECT_BIT,
   CMD_AUTHORS         = 8  | SIMPLESECT_BIT,
   CMD_BOLD            = 9,
   CMD_BSLASH          = 10,
   CMD_BYPASSUNDOCWARN = 11,
   CMD_CITE            = 12,
   CMD_CODE            = 13,
   CMD_COPYBRIEF       = 14,
   CMD_COPYDETAILS     = 15,
   CMD_COPYDOC         = 16,
   CMD_COPYRIGHT       = 17 | SIMPLESECT_BIT,
   CMD_DATE            = 18 | SIMPLESECT_BIT,
   CMD_DBONLY          = 19,
   CMD_DCOLON          = 20,
   CMD_DIAFILE         = 21,
   CMD_DOCBOOKINCLUDE  = 22,
   CMD_DOLLAR          = 23,
   CMD_DONTINCLUDE     = 24,
   CMD_DOT             = 25,
   CMD_DOTFILE         = 26,
   CMD_EMOJI           = 27,
   CMD_EMPHASIS        = 28,
   CMD_ENDCODE         = 29,
   CMD_ENDDBONLY       = 30,
   CMD_ENDDOT          = 31,
   CMD_ENDHTMLONLY     = 32,
   CMD_ENDINTERNAL     = 33,
   CMD_ENDLATEXONLY    = 34,
   CMD_ENDLINK         = 35,
   CMD_ENDMANONLY      = 36,
   CMD_ENDMSC          = 37,
   CMD_ENDPARBLOCK     = 38,
   CMD_ENDRTFONLY      = 39,
   CMD_ENDSECREFLIST   = 40,
   CMD_ENDUML          = 41,
   CMD_ENDVERBATIM     = 42,
   CMD_ENDXMLONLY      = 43,
   CMD_EQUAL           = 44,
   CMD_EXCEPTION       = 45 | SIMPLESECT_BIT,
   CMD_FORMULA         = 46,
   CMD_GREATER         = 47,
   CMD_HASH            = 48,
   CMD_HTMLINCLUDE     = 49,
   CMD_HTMLONLY        = 50,
   CMD_IMAGE           = 51,
   CMD_INCLUDE         = 52,
   CMD_INCLUDEDOC      = 53,
   CMD_INCWITHLINES    = 54,
   CMD_INHERITDOC      = 55,
   CMD_INTERNAL        = 56,
   CMD_INTERNALREF     = 57,
   CMD_INVARIANT       = 58 | SIMPLESECT_BIT,
   CMD_JAVALINK        = 59,
   CMD_LATEXINCLUDE    = 60,
   CMD_LATEXONLY       = 61,
   CMD_LESS            = 62,
   CMD_LI              = 63,
   CMD_LINE            = 64,
   CMD_LINEBREAK       = 65,
   CMD_LINK            = 66,
   CMD_MANINCLUDE      = 67,
   CMD_MANONLY         = 68,
   CMD_MDASH           = 69,
   CMD_MINUS           = 70,
   CMD_MSC             = 71,
   CMD_MSCFILE         = 72,
   CMD_NDASH           = 73,
   CMD_NOTE            = 74 | SIMPLESECT_BIT,
   CMD_PAR             = 75 | SIMPLESECT_BIT,
   CMD_PARAGRAPH       = 76,
   CMD_PARAM           = 77 | SIMPLESECT_BIT,
   CMD_PARBLOCK        = 78,
   CMD_PERCENT         = 79,
   CMD_PIPE            = 80,
   CMD_PLUS            = 81,
   CMD_POST            = 82 | SIMPLESECT_BIT,
   CMD_PRE             = 83 | SIMPLESECT_BIT,
   CMD_PUNT            = 84,
   CMD_QUOTE           = 85,
   CMD_REF             = 86,
   CMD_REMARK          = 87 | SIMPLESECT_BIT,
   CMD_RETURN          = 88 | SIMPLESECT_BIT,
   CMD_RETVAL          = 89 | SIMPLESECT_BIT,
   CMD_RTFINCLUDE      = 90,
   CMD_RTFONLY         = 91,
   CMD_SA              = 92 | SIMPLESECT_BIT,
   CMD_SECREFITEM      = 93,
   CMD_SECREFLIST      = 94,
   CMD_SECTION         = 95,
   CMD_SETSCOPE        = 96,
   CMD_SINCE           = 97 | SIMPLESECT_BIT,
   CMD_SKIP            = 98,
   CMD_SKIPLINE        = 99,
   CMD_SNIPPET         = 100,
   CMD_SNIPPETDOC      = 101,
   CMD_SNIPWITHLINES   = 102,
   CMD_SORTID          = 103,      // cs
   CMD_STARTCODE       = 104,
   CMD_STARTUML        = 105,
   CMD_SUBPAGE         = 106,
   CMD_SUBSECTION      = 107,
   CMD_SUBSUBSECTION   = 108,
   CMD_TPARAM          = 109 | SIMPLESECT_BIT,
   CMD_UNTIL           = 110,
   CMD_VERBATIM        = 111,
   CMD_VERBINCLUDE     = 112,
   CMD_VERSION         = 113 | SIMPLESECT_BIT,
   CMD_WARNING         = 114 | SIMPLESECT_BIT,
   CMD_XMLINCLUDE      = 115,
   CMD_XMLONLY         = 116,
   CMD_XREFITEM        = 117 | SIMPLESECT_BIT,
   CMD_FORCE_OUTPUT    = 118,      // cs
};

enum HtmlTagType {
   HTML_UNKNOWN    = 0,
   HTML_A          = 1,
   HTML_BLOCKQUOTE = 2,
   HTML_BOLD       = 3,
   HTML_BR         = 4,
   HTML_CAPTION    = 5,
   HTML_CENTER     = 6,
   HTML_CITE       = 7,
   HTML_CODE       = 8,
   HTML_DD         = 9,
   HTML_DEL        = 10,
   HTML_DETAILS    = 11,
   HTML_DIV        = 12,
   HTML_DL         = 13,
   HTML_DT         = 14,
   HTML_EMPHASIS   = 15,
   HTML_H1         = 16,
   HTML_H2         = 17,
   HTML_H3         = 18,
   HTML_H4         = 19,
   HTML_H5         = 20,
   HTML_H6         = 21,
   HTML_HR         = 22,
   HTML_IMG        = 23,
   HTML_INS        = 24,
   HTML_LI         = 25,
   HTML_OL         = 26,
   HTML_P          = 27,
   HTML_PRE        = 28,
   HTML_S          = 29,
   HTML_SMALL      = 30,
   HTML_SPAN       = 31,
   HTML_STRIKE     = 32,
   HTML_SUB        = 33,
   HTML_SUP        = 34,
   HTML_TABLE      = 35,
   HTML_TD         = 36,
   HTML_TH         = 37,
   HTML_TR         = 38,
   HTML_UL         = 39,
   HTML_UNDERLINE  = 40,

   XML_CmdMask      = 0x100,

   XML_C            = XML_CmdMask + 0,
   XML_CODE         = XML_CmdMask + 1,
   XML_DESCRIPTION  = XML_CmdMask + 2,
   XML_EXAMPLE      = XML_CmdMask + 3,
   XML_EXCEPTION    = XML_CmdMask + 4,
   XML_INCLUDE      = XML_CmdMask + 5,
   XML_ITEM         = XML_CmdMask + 6,
   XML_LIST         = XML_CmdMask + 7,
   XML_LISTHEADER   = XML_CmdMask + 8,
   XML_PARA         = XML_CmdMask + 9,
   XML_PARAM        = XML_CmdMask + 10,
   XML_PARAMREF     = XML_CmdMask + 11,
   XML_PERMISSION   = XML_CmdMask + 12,
   XML_REMARKS      = XML_CmdMask + 13,
   XML_RETURNS      = XML_CmdMask + 14,
   XML_SEE          = XML_CmdMask + 15,
   XML_SEEALSO      = XML_CmdMask + 16,
   XML_SUMMARY      = XML_CmdMask + 17,
   XML_TERM         = XML_CmdMask + 18,
   XML_TYPEPARAM    = XML_CmdMask + 19,
   XML_TYPEPARAMREF = XML_CmdMask + 20,
   XML_VALUE        = XML_CmdMask + 21,
   XML_INHERITDOC   = XML_CmdMask + 22
};

/** Class representing a mapping from command names to command IDs. */
class Mapper
{
 public:
   Mapper(const CommandMap *cm, bool caseSensitive);

   int map(const QString &n);
   QString map(const int n);

 private:
   QHash<QString, int> m_map;
   bool m_cs;
};

/** Class representing a namespace for the HTML command mappers. */
struct Mappers {
   static void freeMappers();
   static Mapper *cmdMapper;
   static Mapper *htmlTagMapper;
};

#endif
