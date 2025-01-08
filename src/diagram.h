/************************************************************************
*
* Copyright (c) 2014-2025 Barbara Geller & Ansel Sermersheim
*
* Copyright (c) 1997-2014 Dimitri van Heesch
*
* DoxyPress is free software. You can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
* https://www.gnu.org/licenses/
*
*************************************************************************/

#ifndef DIAGRAM_H
#define DIAGRAM_H

#include <QString>
#include <QTextStream>

class ClassDef;
class TreeDiagram;

// Class representing a built in class diagram
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

