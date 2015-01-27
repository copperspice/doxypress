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
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#ifndef DIAGRAM_H
#define DIAGRAM_H

#include <qglobal.h>
#include <QTextStream>

class ClassDef;
class TreeDiagram;

/** Class representing a built-in class diagram. */
class ClassDiagram
{
 public:
   ClassDiagram(QSharedPointer<ClassDef> root);
   ~ClassDiagram();

   void writeFigure(QTextStream &t, const char *path, const char *file) const;

   void writeImage(QTextStream &t, const char *path, const char *relPath,
                   const char *file, bool generateMap = true) const;

 private:
   TreeDiagram *base;
   TreeDiagram *super;
};

#endif

