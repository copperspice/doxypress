/************************************************************************
*
* Copyright (c) 2014-2023 Barbara Geller & Ansel Sermersheim
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

#ifndef DOCTOKENIZER_H
#define DOCTOKENIZER_H

#include <QList>

#include <stdio.h>
#include <htmlattrib.h>

class Definition;
class MemberGroup;

enum Tokens {
   TK_WORD          = 1,
   TK_LNKWORD       = 2,
   TK_WHITESPACE    = 3,
   TK_LISTITEM      = 4,
   TK_ENDLIST       = 5,
   TK_COMMAND_AT    = 6,      // commands which start with @
   TK_HTMLTAG       = 7,
   TK_SYMBOL        = 8,
   TK_NEWPARA       = 9,
   TK_RCSTAG        = 10,
   TK_URL           = 11,
   TK_COMMAND_BS    = 12,     // commands which start with a backslash

   RetVal_OK             = 0x10000,
   RetVal_SimpleSec      = 0x10001,
   RetVal_ListItem       = 0x10002,
   RetVal_Section        = 0x10003,
   RetVal_Subsection     = 0x10004,
   RetVal_Subsubsection  = 0x10005,
   RetVal_Paragraph      = 0x10006,
   RetVal_SubParagraph   = 0x10007,
   RetVal_EndList        = 0x10008,
   RetVal_EndPre         = 0x10009,
   RetVal_DescData       = 0x1000A,
   RetVal_DescTitle      = 0x1000B,
   RetVal_EndDesc        = 0x1000C,
   RetVal_TableRow       = 0x1000D,
   RetVal_TableCell      = 0x1000E,
   RetVal_TableHCell     = 0x1000F,
   RetVal_EndTable       = 0x10010,
   RetVal_Internal       = 0x10011,
   RetVal_SwitchLang     = 0x10012,
   RetVal_CloseXml       = 0x10013,
   RetVal_EndBlockQuote  = 0x10014,
   RetVal_CopyDoc        = 0x10015,
   RetVal_EndInternal    = 0x10016,
   RetVal_EndParBlock    = 0x10017,
   RetVal_EndDiv         = 0x10018
};

/** @brief Data associated with a token used by the comment block parser. */
struct TokenInfo {

   TokenInfo()
      : isEnumList(false), indent(0), id(-1), endTag(false), emptyTag(false), paramDir(Unspecified)
   {
   }

   // command token
   QString name;

   // command text (RCS tag)
   QString text;

   // comment blocks

   // list token info
   bool isEnumList;
   int indent;

   // sections
   QString sectionId;

   // simple section
   QString simpleSectName;
   QString simpleSectText;

   // verbatim fragment
   QString verb;

   // xrefitem
   int id;

   // html tag
   HtmlAttribList attribs;
   bool endTag;
   bool emptyTag;

   // whitespace
   QString chars;

   // url
   bool isEMailAddr;

   // param attributes
   enum ParamDir { In = 1, Out = 2, InOut = 3, Unspecified = 0 };
   ParamDir paramDir;
};

// globals
extern TokenInfo *g_token;
extern FILE *doctokenizerYYin;

// helper functions
QString tokToString(int token);

// operations on the scanner
void doctokenizerYYFindSections(const QString &input, QSharedPointer<Definition> def,
                  QSharedPointer<MemberGroup> mg, const QString &fileName);

void doctokenizerYYinit(const QString &input, const QString &fileName);
void doctokenizerYYcleanup();
void doctokenizerYYpushContext();
bool doctokenizerYYpopContext();
int  doctokenizerYYlex();
void doctokenizerYYsetStatePara();
void doctokenizerYYsetStateTitle();
void doctokenizerYYsetStateTitleAttrValue();
void doctokenizerYYsetStateCode();
void doctokenizerYYsetStateXmlCode();
void doctokenizerYYsetStateHtmlOnly();
void doctokenizerYYsetStateManOnly();
void doctokenizerYYsetStateLatexOnly();
void doctokenizerYYsetStateXmlOnly();
void doctokenizerYYsetStateDbOnly();
void doctokenizerYYsetStateRtfOnly();
void doctokenizerYYsetStateVerbatim();
void doctokenizerYYsetStateDot();
void doctokenizerYYsetStateMsc();
void doctokenizerYYsetStateParam();
void doctokenizerYYsetStateXRefItem();
void doctokenizerYYsetStateFile();
void doctokenizerYYsetStatePattern();
void doctokenizerYYsetStateLink();
void doctokenizerYYsetStateCite();
void doctokenizerYYsetStateRef();
void doctokenizerYYsetStateInternalRef();
void doctokenizerYYsetStateText();
void doctokenizerYYsetStateSkipTitle();
void doctokenizerYYsetStateAnchor();
void doctokenizerYYsetInsidePre(bool b);
void doctokenizerYYpushBackHtmlTag(const QString &tag);
void doctokenizerYYsetStateSnippet();
void doctokenizerYYstartAutoList();
void doctokenizerYYendAutoList();
void doctokenizerYYsetStatePlantUML();
void doctokenizerYYsetStateSetScope();
void doctokenizerYYsetStatePlantUMLOpt();
void doctokenizerYYsetStateOptions();
void doctokenizerYYsetStateBlock();
void doctokenizerYYsetStateEmoji();

void setDoctokenLineNum(int lineNum);
int getDoctokenLineNum();

#endif
