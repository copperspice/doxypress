/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim
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

#include <QHash>
#include <QRegularExpression>
#include <QString>

// used for table column width calculation
const int rtf_pageWidth = 8748;

extern QString rtf_title;
extern QString rtf_subject;
extern QString rtf_comments;
extern QString rtf_company;
extern QString rtf_logoFilename;
extern QString rtf_author;
extern QString rtf_manager;
extern QString rtf_documentType;
extern QString rtf_documentId;
extern QString rtf_keywords;

struct RTFListItemInfo {
   bool isEnum;
   int number;
};

const int rtf_maxIndentLevels = 10;

extern RTFListItemInfo rtf_listItemInfo[rtf_maxIndentLevels];

struct Rtf_Style_Default {

   Rtf_Style_Default(const char *x1, const char *x2, const char *x3) {
      style_name       = QString::fromLatin1(x1);
      style_reference  = QString::fromLatin1(x2);
      style_definition = QString::fromLatin1(x3);
   }

   QString style_name;
   QString style_reference;
   QString style_definition;
};

extern char rtf_Style_Reset[];
extern Rtf_Style_Default rtf_Style_Default[];

struct StyleData {
   // elements of this type are stored in dictionary Rtf_Style

   // to define a tag in the header reference + definition is required
   // to use a tag in the body of the document only reference is required

   unsigned int m_index;         // index in style-sheet, i.e. number in s-clause
   QString      m_reference;     // everything required to apply the style
   QString      m_definition;    // aditional tags like \snext and style name

   StyleData(const QString &reference, const QString &definition);
   StyleData();

   ~StyleData();

   bool setStyle(const QString &s, const QString &styleName);
};

extern QHash<QString, StyleData> rtf_Style;

void loadExtensions(const QString &name);
void loadStylesheet(const QString &name, QHash<QString, StyleData> &dict);

#endif
