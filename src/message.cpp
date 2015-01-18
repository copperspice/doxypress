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

#include <stdarg.h>
#include <stdio.h>

#include <config.h>
#include <doxygen.h>
#include <portable.h>
#include <filedef.h>
#include <message.h>
#include <util.h>

// must appear after the previous include - resolve soon 
#include <doxy_globals.h>

/** Helper struct representing a mapping from debug label to a debug ID */
struct LabelMap {
   const char *name;
   Debug::DebugMask event;
};

/** Class representing a mapping from debug labels to debug IDs. */
class LabelMapper
{
 public:
   LabelMapper();
 
   Debug::DebugMask *find(const char *s) const {
      if (s == 0) {
         return 0;
      }
      return m_map.value(s);
   }

 private:
   QHash<QString, Debug::DebugMask *> m_map;
};

Debug::DebugMask Debug::curMask = Debug::Quiet;
int Debug::curPrio = 0;

static QByteArray outputFormat;
static FILE *warnFile = stderr;
static LabelMapper s_labelMapper;

static const char *warning_str = "Warning: ";


static LabelMap s_labels[] = {
   { "findmembers",  Debug::FindMembers  },
   { "functions",    Debug::Functions    },
   { "variables",    Debug::Variables    },
   { "preprocessor", Debug::Preprocessor },
   { "classes",      Debug::Classes      },
   { "commentcnv",   Debug::CommentCnv   },
   { "commentscan",  Debug::CommentScan  },
   { "validate",     Debug::Validate     },
   { "printtree",    Debug::PrintTree    },
   { "time",         Debug::Time         },
   { "extcmd",       Debug::ExtCmd       },
   { "markdown",     Debug::Markdown     },
   { "filteroutput", Debug::FilterOutput },
   { "lex",          Debug::Lex          },
   { 0,             (Debug::DebugMask)0  }
};

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
LabelMapper::LabelMapper() 
{     
   LabelMap *p = s_labels;

   while (p->name) {
      m_map.insert(p->name, new Debug::DebugMask(p->event));
      p++;
   }
}

// **
void Debug::print(DebugMask mask, int prio, const char *fmt, ...)
{
   if ((curMask & mask) && prio <= curPrio) {
      va_list args;
      va_start(args, fmt);
      vfprintf(stdout, fmt, args);
      va_end(args);
   }
}

static int labelToEnumValue(const QString &data)
{
   QByteArray label = data.toUtf8();
   Debug::DebugMask *event = s_labelMapper.find(label.toLower());

   if (event) {
      return *event;

   } else {
      return 0;
   }
}

int Debug::setFlag(const QString &label)
{
   int retVal = labelToEnumValue(label);
   curMask = (DebugMask)(curMask | labelToEnumValue(label));

   return retVal;
}

void Debug::clearFlag(const char *lab)
{
   curMask = (DebugMask)(curMask & ~labelToEnumValue(lab));
}

void Debug::setPriority(int p)
{
   curPrio = p;
}

bool Debug::isFlagSet(DebugMask mask)
{
   return (curMask & mask) != 0;
}

void Debug::printFlags(void)
{
   int i;

   for (i = 0; i < (int)(sizeof(s_labels) / sizeof(*s_labels)); i++) {
      if (s_labels[i].name) {
         msg("\t%s\n", s_labels[i].name);
      }
   }   
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

   // if the user wants a line break, make it happen
   outputFormat.replace("\\n", "\n");
   outputFormat.replace("\\t", "\t");

   if (! Config_getString("WARN_LOGFILE").isEmpty()) {
      warnFile = fopen(Config_getString("WARN_LOGFILE"), "w");
   }

   if (! warnFile) { 
      // point to something valid
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
   va_start(args, fmt_q);

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

