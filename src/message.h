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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QHash>

#include <stdio.h>
#include <stdarg.h>

/** Class containing a print function for diagnostics. */
class Debug
{
 public:
   enum DebugMask { Quiet        = 0x00000000,
                    FindMembers  = 0x00000001,
                    Functions    = 0x00000002,
                    Variables    = 0x00000004,
                    Preprocessor = 0x00000008,
                    Classes      = 0x00000010,
                    CommentCnv   = 0x00000020,
                    CommentScan  = 0x00000040,
                    Validate     = 0x00000080,
                    PrintTree    = 0x00000100,
                    Time         = 0x00000200,
                    ExtCmd       = 0x00000400,
                    Markdown     = 0x00000800,
                    FilterOutput = 0x00001000,
                    Lex          = 0x00002000
   };

   static void print(DebugMask mask, int prio, const char *fmt, ...);
   static int  setFlag(const QString &label);
   static void clearFlag(const QString &label);
   static bool isFlagSet(DebugMask mask);
   static void printFlags();
   static void setPriority(int p);
  
 private:
   static int labelToEnum(const QString &data);

   static QHash<QString, Debug::DebugMask> m_map;
   static int curMask;
   static int curPriority;
};

void err(const char *fmt, ...);
void msg(const char *fmt, ...);
void warnMsg(const char *fmt, ...);

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
