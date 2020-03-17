/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
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

   enum PageType{ 
      Default,
      Modules
   };

   void initialize() override;
   void finalize() override;
   void incContentsDepth() override;
   void decContentsDepth()override;

   void addContentsItem(bool isDir, const QString &name, const QString &ref, const QString &file, const QString &anchor,
                        bool addToNavIndex, QSharedPointer<Definition> def, DirType category = DirType::None) override;

   void addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md, const QString &, const QString &) override {
      (void) context;
      (void) md;
   }

   void addIndexFile(const QString &) override {}
   void addImageFile(const QString &) override {}
   void addStyleSheetFile(const QString &) override {}
  
   void generateTreeViewInline(QTextStream &t, enum PageType outputType = Default);
   static void generateTreeViewImages();
   void generateTreeViewScripts();

 private:   
   void generateTree(QTextStream &t, const QList<FTVNode *> &nl, int level, int maxLevel, int &index, enum PageType outputType = Default);
  
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
