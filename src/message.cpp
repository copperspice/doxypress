/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
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

#include <QDateTime>

#include <stdarg.h>
#include <stdio.h>

#include <config.h>
#include <doxy_globals.h>
#include <filedef.h>
#include <message.h>
#include <portable.h>
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
   tempMap.insert("time",         Debug::Time         );
   tempMap.insert("extcmd",       Debug::ExtCmd       );
   tempMap.insert("markdown",     Debug::Markdown     );
   tempMap.insert("filteroutput", Debug::FilterOutput );  
   tempMap.insert("lex",          Debug::Lex          );    

   return tempMap;
}

int Debug::labelToEnum(const QString &data)
{   
   Debug::DebugMask type = m_map.value(data.toLower());

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

void Debug::setPriority(int data)
{
   curPriority = data;
}

bool Debug::isFlagSet(DebugMask mask)
{
   return (curMask & mask) != 0;
}

void Debug::printFlags()
{ 
   QList<QString> list = m_map.keys();
   std::sort(list.begin(), list.end());

   for (auto item : list)  {      
      printf("\t%s\n", qPrintable(item));      
   }   
}
  
void Debug::print(DebugMask mask, int data, const char *fmt, ...)
{
   if (curMask & mask) { 
      if (curPriority >= data) {
         va_list args;
         va_start(args, fmt);
         vfprintf(stdout, fmt, args);
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
      warnFile = fopen(qPrintable(logFN), "w");
   }

   if (! warnFile) { 
      // point to something valid
      warnFile = stderr;
   }
}
 
static void format_warn(const char *file, int line, const char *text)
{
   QByteArray fileSubst = file == 0 ? "<unknown>" : file;
   QByteArray lineSubst;

   lineSubst.setNum(line);

   QByteArray textSubst = text;
   QByteArray versionSubst;

   if (file) { // get version from file name
      bool ambig;
      QSharedPointer<FileDef> fd = findFileDef(Doxy_Globals::inputNameDict, file, ambig);

      if (fd) {
         versionSubst = fd->getVersion();
      }
   }

   // substitute markers by actual values
   QByteArray msgText = substitute(outputFormat.toUtf8(), "$file", fileSubst);
   msgText = substitute(msgText, "$text",    textSubst);
   msgText = substitute(msgText, "$line",    lineSubst);
   msgText = substitute(msgText, "$version", versionSubst) + "\n";
   
   // print resulting message
   fwrite(msgText.data(), 1, msgText.length(), warnFile);
}

static void warn_internal(const char *tag, const char *file, int line, const char *prefix, const char *fmt, va_list args)
{
   if (! Config::getBool(tag)) {
      // is this warning type disabled
      return;   
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

   QByteArray temp = "Error: ";   
   vfprintf(warnFile, (temp + fmt).constData(), args);

   va_end(args);
}

void msg(const char *fmt, ...)
{
   if (! Config::getBool("quiet")) {
    
      va_list args;
      va_start(args, fmt);

      vfprintf(stdout, fmt, args);
      va_end(args);
   }
}

void warnMsg(const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   QByteArray temp = "Warning: ";   
   vfprintf(warnFile, (temp + fmt).constData(), args);

   va_end(args);
}

void warn(const char *file, int line, const char *fmt, ...)
{
   static const char *warning_str = "Warning: ";

   va_list args;
   va_start(args, fmt);

   warn_internal("warnings", file, line, warning_str, fmt, args);

   va_end(args);
}

void va_warn(const char *file, int line, const char *fmt, va_list args)
{
   static const char *warning_str = "Warning: ";
   warn_internal("warnings", file, line, warning_str, fmt, args);
}

void warn_simple(const char *file, int line, const char *text)
{
   static const char *warning_str = "Warning: ";

   if (! Config::getBool("warnings")) {
      return;  
   }

   format_warn(file, line, QByteArray(warning_str) + text);
}

void warn_undoc(const char *file, int line, const char *fmt, ...)
{
   static const char *warning_str = "Warning: ";

   va_list args;
   va_start(args, fmt);

   warn_internal("warn-undoc", file, line, warning_str, fmt, args);

   va_end(args);
}

void warn_doc_error(const char *file, int line, const char *fmt, ...)
{
   static const char *warning_str = "Warning: ";

   va_list args;
   va_start(args, fmt);

   warn_internal("warn-doc-error", file, line, warning_str, fmt, args);

   va_end(args);
}

void warn_doc_error(const char *file, int line, QString fmt_q, ...)
{
   static const char *warning_str = "Warning: ";

   QByteArray fmt = fmt_q.toLatin1();

   va_list args;
   va_start(args, fmt_q);

   warn_internal("warn-doc-error", file, line, warning_str, fmt.constData(), args);

   va_end(args);
}

void warn_uncond(const char *fmt, ...)
{
   static const char *warning_str = "Warning: ";

   va_list args;
   va_start(args, fmt);

   vfprintf(warnFile, (QByteArray(warning_str) + fmt).data(), args);

   va_end(args);
}

void printlex(int dbg, bool enter, const char *lexName, const char *fileName)
{
   const char *enter_txt    = "entering";
   const char *enter_txt_uc = "Entering";

   if (! enter) {
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

