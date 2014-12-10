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

#include <QDateTime>

#include <stdio.h>

#include <config.h>
#include <util.h>
#include <debug.h>
#include <doxygen.h>
#include <portable.h>
#include <filedef.h>
#include <message.h>

static QByteArray outputFormat;

static const char *warning_str = "warning: ";
static const char *error_str   = "error: ";

static FILE *warnFile = stderr;

static void format_warn(const char *file, int line, const char *text)
{
   QByteArray fileSubst = file == 0 ? "<unknown>" : file;
   QByteArray lineSubst;

   lineSubst.setNum(line);

   QByteArray textSubst = text;
   QByteArray versionSubst;

   if (file) { // get version from file name
      bool ambig;
      FileDef *fd = findFileDef(Doxygen::inputNameDict, file, ambig);
      if (fd) {
         versionSubst = fd->getVersion();
      }
   }

   // substitute markers by actual values
   QByteArray msgText =
      substitute(
         substitute(
            substitute(
               substitute(
                  outputFormat,
                  "$file", fileSubst
               ),
               "$text", textSubst
            ),
            "$line", lineSubst
         ),
         "$version", versionSubst
      ) + '\n';

   // print resulting message
   fwrite(msgText.data(), 1, msgText.length(), warnFile);
}

static void do_warn(const char *tag, const char *file, int line, const char *prefix, const char *fmt, va_list args)
{
   if (! Config_getBool(tag)) {
      return;   // warning type disabled
   }

   const int bufSize = 40960;
   char text[bufSize];
   int l = 0;

   if (prefix) {
      qstrncpy(text, prefix, bufSize);
      l = strlen(prefix);
   }

   vsnprintf(text + l, bufSize - l, fmt, args);

   text[bufSize - 1] = '\0';
   format_warn(file, line, text);
}


// ** 
void err(const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   vfprintf(warnFile, (QByteArray(error_str) + fmt).data(), args);

   va_end(args);
}

void msg(const char *fmt, ...)
{
   if (! Config_getBool("QUIET")) {
      if (Debug::isFlagSet(Debug::Time)) {
         printf("%.3f sec: ", ((double)Doxygen::runningTime.elapsed()) / 1000.0);
      }

      va_list args;
      va_start(args, fmt);

      vfprintf(stdout, fmt, args);

      va_end(args);
   }
}

void initWarningFormat()
{
   outputFormat = Config_getString("WARN_FORMAT");

   if (! Config_getString("WARN_LOGFILE").isEmpty()) {
      warnFile = portable_fopen(Config_getString("WARN_LOGFILE"), "w");
   }

   if (!warnFile) { 
      // point it to something valid, because warn() relies on it
      warnFile = stderr;
   }
}


void warn(const char *file, int line, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   do_warn("WARNINGS", file, line, warning_str, fmt, args);

   va_end(args);
}

void va_warn(const char *file, int line, const char *fmt, va_list args)
{
   do_warn("WARNINGS", file, line, warning_str, fmt, args);
}

void warn_simple(const char *file, int line, const char *text)
{
   if (! Config_getBool("WARNINGS")) {
      return;   // warning type disabled
   }

   format_warn(file, line, QByteArray(warning_str) + text);
}

void warn_undoc(const char *file, int line, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   do_warn("WARN_IF_UNDOCUMENTED", file, line, warning_str, fmt, args);

   va_end(args);
}

void warn_doc_error(const char *file, int line, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   do_warn("WARN_IF_DOC_ERROR", file, line, warning_str, fmt, args);

   va_end(args);
}

void warn_doc_error(const char *file, int line, QString fmt_q, ...)
{
   QByteArray fmt = fmt_q.toLatin1();

   va_list args;
   va_start(args, fmt.constData());

   do_warn("WARN_IF_DOC_ERROR", file, line, warning_str, fmt.constData(), args);

   va_end(args);
}

void warn_uncond(const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   vfprintf(warnFile, (QByteArray(warning_str) + fmt).data(), args);

   va_end(args);
}

void printlex(int dbg, bool enter, const char *lexName, const char *fileName)
{
   const char *enter_txt    = "entering";
   const char *enter_txt_uc = "Entering";

   if (!enter) {
      enter_txt    = "finished";
      enter_txt_uc = "Finished";
   }

   if (dbg) {
      if (fileName) {
         fprintf(stderr, "--%s lexical analyzer: %s (for: %s)\n", enter_txt, lexName, fileName);
      } else {
         fprintf(stderr, "--%s lexical analyzer: %s\n", enter_txt, lexName);
      }

   } else {
      if (fileName) {
         Debug::print(Debug::Lex, 0, "%s lexical analyzer: %s (for: %s)\n", enter_txt_uc, lexName, fileName);
      } else {
         Debug::print(Debug::Lex, 0, "%s lexical analyzer: %s\n", enter_txt_uc, lexName);
      }
   }
}

