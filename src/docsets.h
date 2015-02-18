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
   void addContentsItem(bool isDir, const QString &name, const char *ref, const char *file, const char *anchor, 
                        bool separateIndex, bool addToNavIndex, QSharedPointer<Definition> def) override;

   void addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md, const char *sectionAnchor, 
                     const char *title);

   void addIndexFile(const char *name);

   void addImageFile(const char *) {}
   void addStyleSheetFile(const char *) {}

 private:
   void writeToken(QTextStream &t, QSharedPointer<Definition> d, const QByteArray &type, const QByteArray &lang,
                   const char *scope = 0, const char *anchor = 0, const QByteArray &decl = QByteArray() ); 

   struct NodeDef {
      NodeDef(bool d, const QByteArray &n, const QByteArray &r, const QByteArray &f, const QByteArray &a, int i) :
         isDir(d), name(n), ref(r), file(f), anchor(a), id(i)
      {}

      bool isDir;
      QByteArray name;
      QByteArray ref;
      QByteArray file;
      QByteArray anchor;
      int id;
   };

   QByteArray indent();

   QFile *m_nf;
   QFile *m_tf;

   QTextStream m_nts;
   QTextStream m_tts;

   int m_dc;
   int m_id;

   QVector<bool> m_firstNode;

   StringMap<QSharedPointer<NodeDef>> m_nodes;
   StringMap<void *> m_scopes;
};

#endif 

