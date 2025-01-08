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

#ifndef MSC_H
#define MSC_H

#include <QString>
#include <QTextStream>

enum MscOutputFormat {
   MSC_BITMAP,
   MSC_EPS,
   MSC_SVG
};

void writeMscGraphFromFile(const QString &inFile, const QString &outDir, const QString &outFile,
      MscOutputFormat format);

QString getMscImageMapFromFile(const QString &nFile, const QString &outDir, const QString &relPath,
      const QString &contxt);

void writeMscImageMapFromFile(QTextStream &t, const QString &inFile, const QString &outDir, const QString &relPath,
      const QString &baseName, const QString &context, MscOutputFormat format );

#endif

