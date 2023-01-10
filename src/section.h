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

#ifndef SECTION_H
#define SECTION_H

class Definition;

/** Class representing a section in a page */
struct SectionInfo {

   enum SectionType { Page          = 0,
                      Section       = 1,
                      Subsection    = 2,
                      Subsubsection = 3,
                      Paragraph     = 4,
                      Anchor        = 5,
                      Table         = 6
   };

   SectionInfo(const QString &f, const int line, const QString &anchor, const QString &t, SectionType secT,
      int lev, const QString &r = QString())
      : label(anchor), title(t), ref(r), fileName(f), lineNr(line), level(lev), generated(false),
        type(secT), dupAnchor_cnt(0)
   { }

   QString label;
   QString title;
   QString ref;
   QString fileName;

   int lineNr;
   int level;
   bool generated;

   SectionType type;
   QSharedPointer<Definition> definition;

   int dupAnchor_cnt;
   QString dupAnchor_fName;
};

#endif
