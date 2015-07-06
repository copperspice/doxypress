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
  
void Debug::print(DebugMask mask, int data, const QString &fmt, ...)
{
   if (curMask & mask) { 
      if (curPriority >= data) {
         va_list args;
         va_start(args, fmt);
         vfprintf(stdout, fmt.toUtf8().constData(), args);
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
 
static void format_warn(const QString &file, int line, const QString &text)
{
   QString fileSubst = file == 0 ? "<unknown>" : file;
   QString lineSubst;

   lineSubst.setNum(line);

   QString textSubst = text;
   QString versionSubst;

   if (! file.isEmpty()) { 
      // get version from file name

      bool ambig;
      QSharedPointer<FileDef> fd = findFileDef(Doxy_Globals::inputNameDict, file, ambig);

      if (fd) {
         versionSubst = fd->getVersion();
      }
   }

   // substitute markers by actual values
   QString msgText = substitute(outputFormat, "$file", fileSubst);
   msgText = substitute(msgText, "$text",    textSubst);
   msgText = substitute(msgText, "$line",    lineSubst);
   msgText = substitute(msgText, "$version", versionSubst) + "\n";
   
   // print resulting message
   fwrite(msgText.data(), 1, msgText.length(), warnFile);
}

static void warn_internal(const QString &tag, const QString &file, int line, const QString &prefix, const QString &fmt, va_list args)
{
   if (! Config::getBool(tag)) {
      // is this warning type disabled
      return;   
   }

   const int bufSize = 40960;

   QByteArray text;
   text.resize(bufSize);

   int len = 0;

   if (! prefix.isEmpty()) {
      qstrncpy(text.data(), prefix.toUtf8().constData(), bufSize);
      len = prefix.length();
   }

   vsnprintf(text.data() + len, bufSize - len, fmt.toUtf8().constData(), args);

   text.resize(bufSize - 1);
   format_warn(file, line, text);
}


// **
void err(const QString &fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   QString temp = "Error: ";   
   vfprintf(warnFile, (temp + fmt).toUtf8().constData(), args);

   va_end(args);
}

void msg(const QString &fmt, ...)
{
   if (! Config::getBool("quiet")) {
    
      va_list args;
      va_start(args, fmt);

      vfprintf(stdout, fmt.toUtf8().constData(), args);
      va_end(args);
   }
}

void warnMsg(const QString &fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   static const QString temp = "Warning: ";      
   vfprintf(warnFile, (temp + fmt).toUtf8().constData(), args);

   va_end(args);
}

void warn(const QString &file, int line, const QString &fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   static const QString warning_str = "Warning: ";
   warn_internal("warnings", file, line, warning_str, fmt, args);

   va_end(args);
}

void va_warn(const QString &file, int line, const QString &fmt, va_list args)
{
   static const QString warning_str = "Warning: ";
   warn_internal("warnings", file, line, warning_str, fmt, args);
}

void warn_simple(const QString &file, int line, const QString &text)
{  
   if (! Config::getBool("warnings")) {
      return;  
   }

   static const QString warning_str = "Warning: ";
   format_warn(file, line, warning_str + text);
}

void warn_undoc(const QString &file, int line, const QString &fmt, ...)
{   
   va_list args;
   va_start(args, fmt);

   static const QString warning_str = "Warning: ";
   warn_internal("warn-undoc", file, line, warning_str, fmt, args);

   va_end(args);
}

void warn_doc_error(const QString &file, int line, const QString &fmt, ...)
{   
   va_list args;
   va_start(args, fmt);

   static QString warning_str = "Warning: ";
   warn_internal("warn-doc-error", file, line, warning_str, fmt, args);

   va_end(args);
}

void warn_uncond(const QString &fmt, ...)
{   
   va_list args;
   va_start(args, fmt);

   static const QString warning_str = "Warning: ";
   vfprintf(warnFile, (warning_str + fmt).toUtf8().constData(), args);

   va_end(args);
}

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
         fprintf(stderr, "--%s lexical analyzer: %s (for: %s)\n", qPrintable(enter_txt), lexName.toUtf8().constData(), 
                  fileName.toUtf8().constData());
      } else {
         fprintf(stderr, "--%s lexical analyzer: %s\n", qPrintable(enter_txt), lexName.toUtf8().constData());
      }

   } else {
       if (! fileName.isEmpty()) {
         Debug::print(Debug::Lex, 0, "%s lexical analyzer: %s (for: %s)\n", qPrintable(enter_txt_uc), 
                  lexName.toUtf8().constData(), fileName.toUtf8().constData());
      } else {
         Debug::print(Debug::Lex, 0, "%s lexical analyzer: %s\n", qPrintable(enter_txt_uc), lexName.toUtf8().constData());
      }
   }
}

