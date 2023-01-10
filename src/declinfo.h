/************************************************************************
*
* Copyright (c) 2014-2023 Barbara Geller & Ansel Sermersheim
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

#ifndef DECLINFO_H
#define DECLINFO_H

#include <QByteArray>

#include <types.h>

#include <stdio.h>

extern void parseFuncDecl(const QString &decl, SrcLangExt lang, QString &clName, QString &type,
                  QString &name, QString &args, QString &funcTempList, QString &exceptions);
#endif
