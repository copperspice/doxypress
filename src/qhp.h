/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * Copyright (C) 2008 by Sebastian Pipping.
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

#ifndef QHP_H
#define QHP_H

#include <index.h>
#include <qhpxmlwriter.h>

class Qhp : public IndexIntf
{
 public:
   Qhp();
   ~Qhp();

   // BEGIN IndexIntf
   void initialize();
   void finalize();
   void incContentsDepth();
   void decContentsDepth();
   void addContentsItem(bool isDir, const QString &name, const char *ref, const char *file, const char *anchor,
                        bool separateIndex, bool addToNavIndex, Definition *def);

   void addIndexItem(Definition *context, MemberDef *md, const char *sectionAnchor, const char *title);
   void addIndexFile(const char *name);
   void addImageFile(const char *name);
   void addStyleSheetFile(const char *name);
   // END IndexIntf

   static QByteArray getQhpFileName();

 private:
   void handlePrevSection();
   void clearPrevSection();
   void setPrevSection(const QString &title, const char *basename, const char *anchor, int level);
   void addFile(const char *fileName);

   static QByteArray getFullProjectName();

   QhpXmlWriter m_doc;
   QhpXmlWriter m_toc;
   QhpXmlWriter m_index;
   QhpXmlWriter m_files;

   QByteArray m_prevSectionTitle;
   QByteArray m_prevSectionBaseName;
   QByteArray m_prevSectionAnchor;

   int m_prevSectionLevel;
   int m_sectionLevel;

   bool m_skipMainPageSection;
};

#endif // DOXYGEN_QHP_H

