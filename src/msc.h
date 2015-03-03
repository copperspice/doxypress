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

#ifndef MSC_H
#define MSC_H

#include <QByteArray>
#include <QTextStream>


enum MscOutputFormat { MSC_BITMAP , MSC_EPS, MSC_SVG };

void writeMscGraphFromFile(const QString &inFile, const QString &outDir, const QString &outFile, MscOutputFormat format);

QString getMscImageMapFromFile(const QString &nFile, const QString &outDir, const QString &relPath, const QByteArray &contxt);

void writeMscImageMapFromFile(QTextStream &t, const QString &inFile, const QString &outDir, const QString &relPath,
                  const QString &baseName, const QByteArray &context, MscOutputFormat format );

#endif

