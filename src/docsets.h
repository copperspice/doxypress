/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim 
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

#ifndef DOCSETS_H
#define DOCSETS_H

#include <QFile>
#include <QTextStream>

#include <index.h>
#include <stringmap.h>

class Definition;

/** A class that generates docset files.
 *
 *  These files can be used to create context help
 *  for use within Apple's Xcode 3.0 development environment
 */
class DocSets  : public IndexIntf
{

 public:
   DocSets();
   ~DocSets();

   void initialize();
   void finalize();
   void incContentsDepth();
   void decContentsDepth();
   void addContentsItem(bool isDir, const QString &name, const QString &ref, const QString &file, const QString &anchor, 
                        bool unused, QSharedPointer<Definition> def) override;

   void addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md, const QString &sectionAnchor, 
                     const QString &title);

   void addIndexFile(const QString &name) override {}
   void addImageFile(const QString &name) override {}
   void addStyleSheetFile(const QString &name) override {}

 private:
   void writeToken(QTextStream &t, QSharedPointer<Definition> d, const QString &type, const QString &lang,
                   const QString &scope = QString(), const QString &anchor = QString(), const QString &decl = QString() ); 

   struct NodeDef {
      NodeDef(bool d, const QString &n, const QString &r, const QString &f, const QString &a, int i) :
         isDir(d), name(n), ref(r), file(f), anchor(a), id(i)
      {}

      bool isDir;
      QString name;
      QString ref;
      QString file;
      QString anchor;
      int id;
   };

   QString indent();

   QFile *m_nf;
   QFile *m_tf;

   QTextStream m_nts;
   QTextStream m_tts;

   int m_dc;
   int m_id;

   QVector<bool> m_firstNode;

   StringMap<QSharedPointer<NodeDef>> m_nodes;
   QSet<QString> m_scopes;
};

#endif 

