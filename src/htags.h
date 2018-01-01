/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim 
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

#ifndef HTAGS_H
#define HTAGS_H

class QByteArray;

/** This class is a namespace for HTAGS related functions */
struct Htags {
   static bool useHtags;
   static bool loadFilemap(const QString &htmldir);
   static QByteArray path2URL(const QString &path);
   static bool execute(const QString &htmldir);
};

#endif 
