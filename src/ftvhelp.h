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

/*
 * ftvhelp.h,v 1.0 2000/09/06 16:09:00 
 * Kenney Wong <kwong@ea.com> 
 * Folder Tree View for offline help on browsers that do not support HTML Help
*/ 

#ifndef FTVHELP_H
#define FTVHELP_H

#include <QList>
#include <QString>
#include <QTextStream>

#include <index.h>
#include <definition.h>

struct FTVNode;

/** A class that generates a dynamic tree view side panel
 */
class FTVHelp : public IndexIntf
{
 public:
   FTVHelp(bool lti);
   ~FTVHelp();

   void initialize();
   void finalize();
   void incContentsDepth();
   void decContentsDepth();

   void addContentsItem(bool isDir, const QString &name, const QString &ref, const QString &file, const QString &anchor,
                        bool separateIndex, bool addToNavIndex, QSharedPointer<Definition> def) override;
  
   void addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md, const QString &, const QString &) override {}

   void addIndexFile(const QString &) override {}
   void addImageFile(const QString &) override {}
   void addStyleSheetFile(const QString &) override {}
  
   void generateTreeViewInline(QTextStream &t);
   static void generateTreeViewImages();
   void generateTreeViewScripts();

 private:   
   void generateTree(QTextStream &t, const QList<FTVNode *> &nl, int level, int maxLevel, int &index);
  
   QString generateIndentLabel(FTVNode *n, int level);
   void generateIndent(QTextStream &t, FTVNode *n, bool opened);
   void generateLink(QTextStream &t, FTVNode *n);
   
   QList<FTVNode *> *m_indentNodes;

   int m_indent;
   bool m_topLevelIndex;
};

struct NavIndexEntry {
   NavIndexEntry(const QString &url, const QString &id) 
      : m_url(url), m_indexId(id)
   {}

   QString m_url;
   QString m_indexId;
};

#endif
