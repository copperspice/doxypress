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

#ifndef HTAGS_H
#define HTAGS_H

#include <QString>

class QByteArray;

/** This class is a namespace for HTAGS related functions */
struct Htags {
   static bool useHtags;
   static bool loadFilemap(const QString &htmldir);
   static QByteArray path2URL(const QString &path);
   static bool execute(const QString &htmldir);
};

#endif
