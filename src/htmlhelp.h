/************************************************************************
*
* Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
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

#ifndef HTMLHELP_H
#define HTMLHELP_H

#include <QByteArray>
#include <QFile>
#include <QHash>
#include <QStringList>
#include <QTextCodec>
#include <QTextStream>
#include <QSet>

#include <index.h>
#include <stringmap.h>

class Definition;
class HtmlHelp;

// manages a two level index in alphabetical order
class HtmlHelpIndex
{
 public:
   HtmlHelpIndex(HtmlHelp *help);

   void addItem(const QString &first, const QString &second, const QString &url, const QString &anchor, bool hasLink, bool reversed);

   size_t size() const {
      return m_indexFieldDict.count();
   }

   void writeFields(QTextStream &t);

 private:
   IndexFieldSDict m_indexFieldDict;
   HtmlHelp *m_help;
};

// Class representing a field in the HTML help index
struct IndexField {
   QString name;
   QString url;
   QString anchor;

   bool link;
   bool reversed;
};

/** A class that generated the HTML Help specific files.
 *
 *  These files can be used with the Microsoft HTML Help workshop
 *  to generate compressed HTML files (.chm).
 */
class HtmlHelp : public IndexIntf
{
   /*! used in imageNumber param of HTMLHelp::addContentsItem() function
       to specify document icon in tree view.
       Writes \<param name="ImageNumber" value="xx"\> in .HHC file. */

   enum ImageNumber {
      BOOK_CLOSED = 1,   BOOK_OPEN,
      BOOK_CLOSED_NEW,   BOOK_OPEN_NEW,
      FOLDER_CLOSED,     FOLDER_OPEN,
      FOLDER_CLOSED_NEW, FOLDER_OPEN_NEW,
      QUERY,             QUERY_NEW,
      TEXT,              TEXT_NEW,
      WEB_DOC,           WEB_DOC_NEW,
      WEB_LINK,          WEB_LINK_NEW,
      INFO,              INFO_NEW,
      LINK,              LINK_NEW,
      BOOKLET,           BOOKLET_NEW,
      EMAIL,             EMAIL_NEW,
      EMAIL2,            EMAIL2_NEW,
      IMAGE,             IMAGE_NEW,
      AUDIO,             AUDIO_NEW,
      MUSIC,             MUSIC_NEW,
      VIDEO,             VIDEO_NEW,
      INDEX,             INDEX_NEW,
      IDEA,              IDEA_NEW,
      NOTE,              NOTE_NEW,
      TOOL,              TOOL_NEW
   };

 public:
   //static HtmlHelp *getInstance();
   HtmlHelp();
   ~HtmlHelp();

   void initialize() override;
   void finalize() override;
   void incContentsDepth() override;
   void decContentsDepth() override;

   void addContentsItem(bool isDir, const QString &name, const QString &ref, const QString &file, const QString &anchor,
         bool unused, QSharedPointer<Definition> def, DirType category) override;

   void addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md, const QString &sectionAnchor,
         const QString &title) override;

   void addIndexFile(const QString &name) override;
   void addImageFile(const QString &name) override;

   void addStyleSheetFile(const QString &name) override {
      (void) name;
   }

   QString recode(const QString &s);

 private:
   void createProjectFile();

   static HtmlHelp *theInstance;
   HtmlHelpIndex m_index;

   QFile cf;
   QFile kf;

   int dc;
   bool ctsItemPresent;

   QTextStream cts;
   QTextStream kts;

   QStringList indexFiles;
   QStringList imageFiles;
   QSet<QString> indexFileDict;


   QTextCodec *m_toNewCodec;
};

#endif

