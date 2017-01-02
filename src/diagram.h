/*************************************************************************
 *
 * Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim 
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

#ifndef DIAGRAM_H
#define DIAGRAM_H

#include <QString>
#include <QTextStream>

class ClassDef;
class TreeDiagram;

/** Class representing a built-in class diagram. */
class ClassDiagram
{
 public:
   ClassDiagram(QSharedPointer<ClassDef> root);
   ~ClassDiagram();

   void writeFigure(QTextStream &t, const QString &path, const QString &file) const;
   void writeImage(QTextStream &t, const QString &path, const QString &relPath, const QString &file, bool generateMap = true) const;

 private:
   TreeDiagram *base;
   TreeDiagram *super;
};

#endif

