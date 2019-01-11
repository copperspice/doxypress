/************************************************************************
*
* Copyright (C) 2014-2019 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
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

#ifndef CONSTEXP_H
#define CONSTEXP_H

#include <QByteArray>

#include <cppvalue.h>

extern bool parseconstexp(const QString &fileName, int line, const QString &s);
extern int constexpYYparse();
extern int constexpYYdebug;
extern QString  g_strToken;
extern CPPValue g_resultValue;
extern QString  g_constExpFileName;
extern int      g_constExpLineNr;

#endif
