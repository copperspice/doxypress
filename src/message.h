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
                    ExtCmd       = 0x00000400,
                    Markdown     = 0x00000800,
                    FilterOutput = 0x00001000,
                    Lex          = 0x00002000
   };

   static void print(DebugMask mask, int prio, const QString fmt, ...);
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

// **
void err(const QString fmt, ...);
void errAll(const QString fmt, ...);

void errNoPrefix(const QString fmt, ...);
void errNoPrefixAll(const QString fmt, ...);

void msg(const QString fmt, ...);

void warnMsg(const QString fmt, ...);
void warnAll(const QString fmt, ...);

void initWarningFormat();

// **
void printlex(int dbg, bool enter, const QString &lexName, const QString &fileName);

// **
void warn(const QString &file, int line, const QString fmt, ...);
void warn_doc_error(const QString &file, int line, const QString fmt, ...);
void warn_simple(const QString &file, int line, const QString &text);
void warn_undoc(const QString &file, int line, const QString fmt, ...);

void warn_uncond(const QString fmt, ...);
void va_warn(const QString &file, int line, const QString &fmt, va_list args);

#endif
