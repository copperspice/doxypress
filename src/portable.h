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

#ifndef PORTABLE_H
#define PORTABLE_H

#include <qglobal.h>
#include <QString>

#include <sys/types.h>
#include <stdio.h>

#if defined(_WIN32)
typedef __int64 portable_off_t;
#else
typedef off_t portable_off_t;
#endif

/** @file
 *  @brief Portable versions of functions that are platform dependent.
 */

int            portable_system(const QString &command, const QString &args, bool commandHasConsole = true);
uint           portable_pid();
QString        portable_getenv(const QString &variable);
void           portable_setenv(const QString &variable, const QString &value);
void           portable_unsetenv(const QString &variable);
portable_off_t portable_fseek(FILE *f, portable_off_t offset, int whence);
portable_off_t portable_ftell(FILE *f);
char           portable_pathListSeparator();
QString        portable_commandExtension();

Qt::CaseSensitivity  portable_fileSystemIsCaseSensitive();

void           portable_sysTimerStart();
void           portable_sysTimerStop();
double         portable_getSysElapsedTime();
void           portable_sleep(int ms);

#endif

