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

#ifndef SECTION_H
#define SECTION_H

#include <stringmap.h>

class Definition;

/** Class representing a section in a page */
struct SectionInfo {
   enum SectionType { Page          = 0,
                      Section       = 1,
                      Subsection    = 2,
                      Subsubsection = 3,
                      Paragraph     = 4,
                      Anchor        = 5
                    };

   SectionInfo(const char *f, const int lin, const char *l, const char *t, SectionType st, int lev, const char *r = 0) 
      : label(l), title(t), type(st), ref(r), fileName(f), lineNr(lin), generated(false), level(lev) 
   { }

   SectionInfo(const SectionInfo &s) {
      label = s.label;
      title = s.title;
      type  = s.type;
      ref   = s.ref;

      definition = s.definition;
      fileName   = s.fileName;
      lineNr     = s.lineNr;
      generated  = s.generated;
      level      = s.level;
   }

   ~SectionInfo() {}

   QByteArray label;
   QByteArray title;
   SectionType type;
   QByteArray ref;

   QSharedPointer<Definition> definition;

   QByteArray fileName;
   int lineNr;
   bool generated;
   int level;
};

using SectionDict = StringMap<QSharedPointer<SectionInfo>>;

#endif
