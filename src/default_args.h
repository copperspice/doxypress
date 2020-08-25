/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
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

#ifndef DEFARGS_H
#define DEFARGS_H

#include <QByteArray>

#include <arguments.h>

ArgumentList stringToArgumentList(const QString &argsString, const ArgumentList &argList = ArgumentList());
ArgumentList stringToArgumentList(const QString &argsString, const ArgumentList &argList, QString &extraTypeChars);

#endif
