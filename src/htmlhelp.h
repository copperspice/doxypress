/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
 * All rights reserved.    
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License version 2
 * is hereby granted. No representations are made about the suitability of
 * this software for any purpose. It is provided "as is" without express or
 * implied warranty. See the GNU General Public License for more details.
 *
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#ifndef HTMLHELP_H
#define HTMLHELP_H

#include <QStringList>
#include <QHash>
#include <QFile>

#include <index.h>
#include <ftextstream.h>

class Definition;
class HtmlHelpIndex;

/** A class that generated the HTML Help specific files.
 *
 *  These files can be used with the Microsoft HTML Help workshop
 *  to generate compressed HTML files (.chm).
 */
class HtmlHelp  : public IndexIntf
{
   /*! used in imageNumber param of HTMLHelp::addContentsItem() function
       to specify document icon in tree view.
       Writes \<param name="ImageNumber" value="xx"\> in .HHC file. */

   enum ImageNumber {
      BOOK_CLOSED = 1,   BOOK_OPEN,
      BOOK_CLOSED_NEW,   BOOK_OPEN_NEW,
      FOLDER_CLOSED,     FOLDER_OPEN,
      FOLDER_CLOSED_NEW, FOLDER_OPEN_NEW,
      QUERY,            QUERY_NEW,
      TEXT,             TEXT_NEW,
      WEB_DOC,          WEB_DOC_NEW,
      WEB_LINK,         WEB_LINK_NEW,
      INFO,             INFO_NEW,
      LINK,             LINK_NEW,
      BOOKLET,          BOOKLET_NEW,
      EMAIL,            EMAIL_NEW,
      EMAIL2,           EMAIL2_NEW,
      IMAGE,            IMAGE_NEW,
      AUDIO,            AUDIO_NEW,
      MUSIC,            MUSIC_NEW,
      VIDEO,            VIDEO_NEW,
      INDEX,            INDEX_NEW,
      IDEA,             IDEA_NEW,
      NOTE,             NOTE_NEW,
      TOOL,             TOOL_NEW
   };

 public:
   //static HtmlHelp *getInstance();
   HtmlHelp();
   ~HtmlHelp();
   void initialize();
   void finalize();
   void incContentsDepth();
   void decContentsDepth();

   void addContentsItem(bool isDir, const char *name, const char *ref, const char *file, const char *anchor,
                        bool separateIndex, bool addToNavIndex, Definition *def);

   void addIndexItem(Definition *context, MemberDef *md, const char *sectionAnchor, const char *title);
   void addIndexFile(const char *name);
   void addImageFile(const char *);
   void addStyleSheetFile(const char *) {}

 private:
   friend class HtmlHelpIndex;
   void createProjectFile();

   QFile *cf, *kf;
   FTextStream cts, kts;
   HtmlHelpIndex *index;
   int dc;

   QStringList indexFiles;
   QStringList imageFiles;
   QHash<QString, void *> indexFileDict;

   static HtmlHelp *theInstance;
   QByteArray recode(const QByteArray &s);
   void *m_fromUtf8;
};

#endif

