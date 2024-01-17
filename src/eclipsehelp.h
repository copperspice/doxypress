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

/*
 * eclipsehelp.h
 *
 * Created: 7.11.2009
 * Author: ondrej
 *
 */

#ifndef ECLIPSEHELP_H
#define ECLIPSEHELP_H

#include <QFile>
#include <QString>
#include <QTextStream>

#include <index.h>
#include <definition.h>

/** Generator for Eclipse help files.
 *
 * This class generates the Eclipse specific help files.
 * These files can be used to generate a help plugin readable
 * by the Eclipse IDE.
 */
class EclipseHelp : public IndexIntf
{
 public:
   EclipseHelp();
   virtual ~EclipseHelp();

   void initialize() override;
   void finalize() override;
   void incContentsDepth() override;
   void decContentsDepth() override;
   void addContentsItem(bool isDir, const QString &name, const QString &ref, const QString &file, 
                  const QString &anchor, bool unused, QSharedPointer<Definition> def, DirType category) override;
  
   void addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md, const QString &sectionAnchor, 
                  const QString &title) override;

   void addIndexFile(const QString &name) override;
   void addImageFile(const QString &name) override;
   void addStyleSheetFile(const QString &name) override;

 private:
   int  m_depth;
   bool m_endtag;
   int  m_openTags;

   QFile       m_tocfile;
   QTextStream m_tocstream;
   QString     m_pathprefix;

   // avoid copying
   EclipseHelp(const EclipseHelp &);
   EclipseHelp &operator = (const EclipseHelp &);

   // formatting
   void indent();
   void closedTag();
   void openedTag();
};

#endif
