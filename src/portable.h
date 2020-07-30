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

#ifndef PORTABLE_H
#define PORTABLE_H

#include <qglobal.h>
#include <QString>

#include <sys/types.h>
#include <stdio.h>

#if defined(_WIN32)
   using portable_off_t = __int64;
#else
   using portable_off_t = off_t;
#endif

QString        portable_commandExtension();
int            portable_system(const QString &command, const QString &args, bool commandHasConsole = true);
void           portable_sleep(int ms);
uint           portable_pid();
char           portable_pathListSeparator();

QString        portable_getenv(const QString &variable);
void           portable_setenv(const QString &variable, const QString &value);
void           portable_unsetenv(const QString &variable);

portable_off_t portable_fseek(FILE *f, portable_off_t offset, int whence);
portable_off_t portable_ftell(FILE *f);

void           portable_sysTimerStart();
void           portable_sysTimerStop();
double         portable_getSysElapsedTime();

Qt::CaseSensitivity  portable_fileSystemIsCaseSensitive();

#endif

