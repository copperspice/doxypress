/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * All rights reserved.    
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License version 2
 * is hereby granted. No representations are made about the suitability of
 * this software for any purpose. It is provided "as is" without express or
 * implied warranty. See the GNU General Public License for more details.
 *
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#ifndef RTFSTYLE_H
#define RTFSTYLE_H

#include <QRegExp>
#include <QHash>

// used for table column width calculation
const int rtf_pageWidth = 8748;

extern QByteArray rtf_title;
extern QByteArray rtf_subject;
extern QByteArray rtf_comments;
extern QByteArray rtf_company;
extern QByteArray rtf_logoFilename;
extern QByteArray rtf_author;
extern QByteArray rtf_manager;
extern QByteArray rtf_documentType;
extern QByteArray rtf_documentId;
extern QByteArray rtf_keywords;

struct RTFListItemInfo {
   bool isEnum;
   int number;
};

const int rtf_maxIndentLevels = 10;

extern RTFListItemInfo rtf_listItemInfo[rtf_maxIndentLevels];

struct Rtf_Style_Default {
   const char *name;
   const char *reference;
   const char *definition;
};

extern char rtf_Style_Reset[];
extern Rtf_Style_Default rtf_Style_Default[];

struct StyleData {
   // elements of this type are stored in dictionary Rtf_Style
   //
   // to define a tag in the header reference + definition is required
   // to use a tag in the body of the document only reference is required

   unsigned index;   // index in style-sheet, i.e. number in s-clause
   char *reference;  // everything required to apply the style
   char *definition; // aditional tags like \snext and style name

   StyleData(const char *reference, const char *definition);
   StyleData();

   ~StyleData();

   bool setStyle(const char *s, const char *styleName);

   static const QRegExp s_clause;
};

extern QHash<QString, StyleData> rtf_Style;

void loadExtensions(const QString &name);
void loadStylesheet(const QString &name, QHash<QString, StyleData> &dict);

#endif
