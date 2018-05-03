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

#ifndef RESOURCEMGR_H
#define RESOURCEMGR_H

#include <QByteArray>
#include <QString>

// manages resources compiled into an executable
class ResourceMgr
{
 public:
   static ResourceMgr &instance();
   QByteArray getAsString(const QString &fName) const;

   // Copies a registered resource to a given target directory under a given target name
   bool copyResourceAs(const QString &fName, const QString &targetDir, const QString &targetName) const;

 private:
   ResourceMgr();
   ~ResourceMgr();

   enum Type { Verbatim, Luminance, LumAlpha, CSS };
};

#endif
