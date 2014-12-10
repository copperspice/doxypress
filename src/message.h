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
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>

#include <stdio.h>
#include <stdarg.h>

void err(const char *fmt, ...);
void msg(const char *fmt, ...);

void initWarningFormat();

void printlex(int dbg, bool enter, const char *lexName, const char *fileName);

void warn(const char *file, int line, const char *fmt, ...);
void warn_simple(const char *file, int line, const char *text);
void warn_undoc(const char *file, int line, const char *fmt, ...);

void warn_doc_error(const char *file, int line, const char *fmt, ...);
void warn_doc_error(const char *file, int line, QString fmt_q, ...);

void warn_uncond(const char *fmt, ...);
void va_warn(const char *file, int line, const char *fmt, va_list args);


inline const char *qPrint(const char *s)
{
  if (s) {
   return s;
  } else {
   return "";
  }
}

#endif
