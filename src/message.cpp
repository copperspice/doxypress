/************************************************************************
*
* Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
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

#include <stdarg.h>
#include <stdio.h>

#include <message.h>

#include <config.h>
#include <doxy_globals.h>
#include <util.h>

int Debug::curMask     = 0;
int Debug::curPriority = 0;

QHash<QString, Debug::DebugMask> debugMap();
QHash<QString, Debug::DebugMask> Debug::m_map = debugMap();

QHash<QString, Debug::DebugMask> debugMap()
{
   QHash<QString, Debug::DebugMask> tempMap;

   tempMap.insert("findmembers",  Debug::FindMembers  );
   tempMap.insert("functions",    Debug::Functions    );
   tempMap.insert("variables",    Debug::Variables    );
   tempMap.insert("preprocessor", Debug::Preprocessor );
   tempMap.insert("classes",      Debug::Classes      );
   tempMap.insert("commentcnv",   Debug::CommentCnv   );
   tempMap.insert("commentscan",  Debug::CommentScan  );
   tempMap.insert("validate",     Debug::Validate     );
   tempMap.insert("printtree",    Debug::PrintTree    );
   tempMap.insert("extcmd",       Debug::ExtCmd       );
   tempMap.insert("markdown",     Debug::Markdown     );
   tempMap.insert("filteroutput", Debug::FilterOutput );
   tempMap.insert("lex",          Debug::Lex          );

   return tempMap;
}

int Debug::labelToEnum(const QString &text)
{
   Debug::DebugMask type = m_map.value(text.toLower());

   if (type) {
      return type;

   } else {
      return 0;
   }
}

int Debug::setFlag(const QString &label)
{
   int retval = labelToEnum(label);
   curMask = curMask | retval;

   return retval;
}

void Debug::clearFlag(const QString &label)
{
   curMask = curMask & ~labelToEnum(label);
}

void Debug::setPriority(int text)
{
   curPriority = text;
}

bool Debug::isFlagSet(DebugMask mask)
{
   return (curMask & mask) != 0;
}

void Debug::printFlags()
{
   QList<QString> list = m_map.keys();
   std::sort(list.begin(), list.end());
   int cnt = 1;

   for (auto item : list)  {
      if (cnt == 5 || cnt == 10 || cnt == 15) {
         printf("\t%-15s\n", csPrintable(item));
      } else  {
         printf("\t%-15s", csPrintable(item));
      }

      cnt++;
   }

   printf("\n");
}

void Debug::print(DebugMask mask, int data, const QString fmt, ...)
{
   if (curMask & mask) {
      if (curPriority >= data) {
         va_list args;
         va_start(args, fmt);
         vfprintf(stdout, fmt.constData(), args);
         va_end(args);
      }
   }
}


// start of message output, next two are defined by the project configuration
static QString outputFormat;
static FILE *warnFile = stderr;

void initWarningFormat()
{
   outputFormat = Config::getString("warn-format");

   // if the user wants a line break, make it happen
   outputFormat.replace("\\n", "\n");
   outputFormat.replace("\\t", "\t");

   QString logFN = Config::getString("warn-logfile");

   if (! logFN.isEmpty()) {
      warnFile = fopen(csPrintable(logFN), "w");
   }

   if (! warnFile) {
      // point to something valid
      warnFile = stderr;
   }
}

static void format_warn(const QString &file, int line, const QString &text)
{
   QString fileSubst = file;

   if (file.isEmpty() ) {
      fileSubst = "<unknown>";
   }

   QString lineSubst = QString::number(line);
   QString textSubst = text;
   QString versionSubst;

   if (! file.isEmpty()) {
      // get version from file name

      bool ambig;
      QSharedPointer<FileDef> fd = findFileDef(&Doxy_Globals::inputNameDict, file, ambig);

      if (fd) {
         versionSubst = fd->getVersion();
      }
   }

   // substitute markers by actual values
   QString msgText = substitute(outputFormat, "$file", fileSubst);
   msgText = substitute(msgText, "$text",    textSubst);
   msgText = substitute(msgText, "$line",    lineSubst);
   msgText = substitute(msgText, "$version", versionSubst) + "\n";

   // message sent to warnFile
   fwrite(msgText.constData(), 1, msgText.length(), warnFile);
}

static void warn_internal(const QString &tag, const QString &file, int line, const QString &prefix,
                  const QString &fmt, va_list args)
{
   if (! Config::getBool(tag)) {
      // this warning type disabled
      return;
   }

   va_list argsCopy;
   va_copy(argsCopy, args);

   const int bufSize = 40960;

   QByteArray text = prefix.constData();
   int len  = text.length();

   text.resize(bufSize);

   int actual_len = vsnprintf(text.data() + len, bufSize - len, fmt.constData(), argsCopy);
   text.resize(len + actual_len);

   va_end(argsCopy);
   if (len + actual_len >= bufSize) {
      // buffer was too small
      vsnprintf(text.data() + len, text.size() - len, fmt.constData(), args);
   }

   format_warn(file, line, text);
}

// **
void err(const QString fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   const QString tmp = "Error: " + fmt;

   // message sent to warnFile
   vfprintf(warnFile, tmp.constData(), args);

   va_end(args);
}

void errAll(const QString fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   const QString tmp = "Error: " + fmt;

   // message sent to warnFile
   vfprintf(warnFile, tmp.constData(), args);

   if (warnFile != stderr) {
      // message displayed on screen
      vfprintf(stderr, tmp.constData(), args);
   }

   va_end(args);
}

void errNoPrefix(const QString fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   // message sent to warnFile
   vfprintf(warnFile, fmt.constData(), args);

   va_end(args);
}

void errNoPrefixAll(const QString fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   // message sent to warnFile
   vfprintf(warnFile, fmt.constData(), args);

   if (warnFile != stderr) {
      // message displayed on screen
      vfprintf(stderr, fmt.constData(), args);
   }

   va_end(args);
}

void warnMsg(const QString fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   const QString tmp = "Warning: " + fmt;

   // message sent to warnFile
   vfprintf(warnFile, tmp.constData(), args);

   va_end(args);
}

void warnAll(const QString fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   const QString tmp = "Warning: " + fmt;

   // message sent to warnFile
   vfprintf(warnFile, tmp.constData(), args);

   if (warnFile != stderr) {
      // message displayed on screen
      vfprintf(stderr, tmp.constData(), args);
   }

   va_end(args);
}


// **
void msg(const QString fmt, ...)
{
   if (! Config::getBool("quiet")) {
      va_list args;
      va_start(args, fmt);

      // message displayed on screen
      vfprintf(stdout, fmt.constData(), args);

      va_end(args);
   }
}

void warn(const QString &file, int line, const QString fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   static const QString temp = "Warning: ";

   // message sent to warnFile
   warn_internal("warnings", file, line, temp, fmt, args);

   va_end(args);
}

void warn_doc_error(const QString &file, int line, const QString fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   static const QString temp = "Warning: ";

   // message sent to warnFile
   warn_internal("warn-doc-error", file, line, temp, fmt, args);

   va_end(args);
}

void warn_simple(const QString &file, int line, const QString &text)
{
   if (! Config::getBool("warnings")) {
      return;
   }

   const QString tmp = "Warning: " + text;

   // message sent to warnFile
   format_warn(file, line, tmp);
}

void warn_undoc(const QString &file, int line, const QString fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   static const QString temp = "Warning: ";

   // message sent to warnFile
   warn_internal("warn-undoc", file, line, temp, fmt, args);

   va_end(args);
}

// **
void warn_uncond(const QString fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   const QString tmp = "Warning: " + fmt;

   // message sent to warnFile
   vfprintf(warnFile, tmp.constData(), args);

   va_end(args);
}

void va_warn(const QString &file, int line, const QString &fmt, va_list args)
{
   static const QString tmp = "Warning: ";

   // message sent to warnFile
   warn_internal("warnings", file, line, tmp, fmt, args);
}

// **
void printlex(int dbg, bool enter, const QString &lexName, const QString &fileName)
{
   QString enter_txt    = "entering";
   QString enter_txt_uc = "Entering";

   if (! enter) {
      enter_txt    = "finished";
      enter_txt_uc = "Finished";
   }

   if (dbg) {

      if (! fileName.isEmpty()) {
         fprintf(stderr, "--%s lexical analyzer: %s (for: %s)\n", csPrintable(enter_txt), csPrintable(lexName),
                  csPrintable(fileName));
      } else {
         fprintf(stderr, "--%s lexical analyzer: %s\n", csPrintable(enter_txt), csPrintable(lexName));
      }

   } else {
       if (! fileName.isEmpty()) {
         Debug::print(Debug::Lex, 0, "%s lexical analyzer: %s (for: %s)\n", csPrintable(enter_txt_uc),
                  csPrintable(lexName), csPrintable(fileName));
      } else {
         Debug::print(Debug::Lex, 0, "%s lexical analyzer: %s\n", csPrintable(enter_txt_uc),
                  csPrintable(lexName));
      }
   }
}
