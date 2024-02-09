/************************************************************************
*
* Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
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

#ifndef DOCSETS_H
#define DOCSETS_H

#include <index.h>
#include <stringmap.h>

#include <QFile>
#include <QTextStream>
#include <QSet>

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

   void initialize() override;
   void finalize() override;
   void incContentsDepth() override;
   void decContentsDepth() override;
   void addContentsItem(bool isDir, const QString &name, const QString &ref, const QString &file, const QString &anchor,
                  bool unused, QSharedPointer<Definition> def, DirType category) override;

   void addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md, const QString &sectionAnchor,
                  const QString &title) override;

   void addIndexFile(const QString &name) override {
      (void) name;
   }

   void addImageFile(const QString &name) override {
      (void) name;
   }

   void addStyleSheetFile(const QString &name) override {
      (void) name;
   }

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

   QFile m_nf;
   QFile m_tf;

   QTextStream m_nts;
   QTextStream m_tts;

   int m_dc;
   int m_id;

   QVector<bool> m_firstNode;

   StringMap<QSharedPointer<NodeDef>> m_nodes;
   QSet<QString> m_scopes;
};

#endif

