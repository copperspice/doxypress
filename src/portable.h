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

int            portable_system(const QByteArray &command, const QByteArray &args, bool commandHasConsole = true);
uint           portable_pid();
const char    *portable_getenv(const char *variable);
void           portable_setenv(const char *variable, const char *value);
void           portable_unsetenv(const char *variable);
portable_off_t portable_fseek(FILE *f, portable_off_t offset, int whence);
portable_off_t portable_ftell(FILE *f);
char           portable_pathSeparator();
char           portable_pathListSeparator();
const char    *portable_ghostScriptCommand();
const char    *portable_commandExtension();

Qt::CaseSensitivity  portable_fileSystemIsCaseSensitive();

void           portable_sysTimerStart();
void           portable_sysTimerStop();
double         portable_getSysElapsedTime();
void           portable_sleep(int ms);
bool           portable_isAbsolutePath(const char *fileName);

#endif

