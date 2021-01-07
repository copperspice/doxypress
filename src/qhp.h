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

#ifndef QHP_H
#define QHP_H

#include <index.h>
#include <qhpxmlwriter.h>

class Qhp : public IndexIntf
{
 public:
   Qhp();
   ~Qhp();
 
   void initialize() override;
   void finalize() override;
   void incContentsDepth() override;
   void decContentsDepth() override;

   void addContentsItem(bool isDir, const QString &name, const QString &ref, const QString &file, const QString &anchor,
                  bool unused, QSharedPointer<Definition> def, DirType category) override;

   void addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md, 
                  const QString &sectionAnchor, const QString &title) override;

   void addIndexFile(const QString &name) override;
   void addImageFile(const QString &name) override;
   void addStyleSheetFile(const QString &name) override;
  
   static QString getQhpFileName();

 private:
   void handlePrevSection();
   void clearPrevSection();
   void setPrevSection(const QString &title, const QString &basename, const QString &anchor, int level);
   void addFile(const QString &fileName);

   static QString getFullProjectName();

   QhpXmlWriter m_doc;
   QhpXmlWriter m_toc;
   QhpXmlWriter m_index;
   QhpXmlWriter m_files;

   QString m_prevSectionTitle;
   QString m_prevSectionBaseName;
   QString m_prevSectionAnchor;

   int m_prevSectionLevel;
   int m_sectionLevel;

   bool m_skipMainPageSection;
};

#endif

