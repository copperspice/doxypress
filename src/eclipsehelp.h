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

   virtual void initialize();
   virtual void finalize();
   virtual void incContentsDepth();
   virtual void decContentsDepth();
   virtual void addContentsItem(bool isDir, const QString &name, const QString &ref, const QString &file, 
                                const QString &anchor, bool unused, QSharedPointer<Definition> def) override;
  
   virtual void addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md, const QString &sectionAnchor, 
                     const QString &title) override;

   virtual void addIndexFile(const QString &name) override;
   virtual void addImageFile(const QString &name) override;
   virtual void addStyleSheetFile(const QString &name) override;

 private:
   int m_depth;
   bool m_endtag;
   int m_openTags;

   QFile *m_tocfile;
   QTextStream m_tocstream;
   QString m_pathprefix;

   /* -- avoid copying */
   EclipseHelp(const EclipseHelp &);
   EclipseHelp &operator = (const EclipseHelp &);

   /* -- formatting helpers */
   void indent();
   void closedTag();
   void openedTag();
};

#endif
