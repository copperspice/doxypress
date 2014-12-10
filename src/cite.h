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

#ifndef CITEDB_H
#define CITEDB_H

#include <QHash>

class FTextStream;

/// String constants for citations
struct CiteConsts {
   static const QByteArray fileName;
   static const QByteArray anchorPrefix;
};

/// Citation-related data.
struct CiteInfo {
   CiteInfo(const char *label_, const char *text_ = 0, const char *fullText_ = 0,
            const char *ref_ = 0) :
      label(label_), text(text_), fullText(fullText_), ref(ref_) {
   }

   CiteInfo(const CiteInfo &o) {
      label    = o.label;
      text     = o.text;
      fullText = o.fullText;
      ref      = o.ref;
   }

   QByteArray label;
   QByteArray text;
   QByteArray fullText;
   QByteArray ref;

};

/**
 * @brief Cite database access class.
 * @details This class provides access do the database of bibliographic
 * references through the bibtex backend.
 */
class CiteDict
{
 public:
   /** Create the database, with an expected maximum of \a size entries */
   CiteDict();

   //    /** Resolve references to citations */
   //    void resolve();

   /** Insert a citation identified by \a label into the database */
   void insert(const char *label);

   /** Return the citation info for a given \a label */
   CiteInfo *find(const char *label) const;

   /** Generate the citations page */
   void generatePage() const;

   /** clears the database */
   void clear();

   /** return true if there are no citations.
    *  Only valid after calling resolve()
    */
   bool isEmpty() const;

   /** writes the latex code for the standard bibliography
    *  section to text stream \a t
    */
   void writeLatexBibliography(FTextStream &t);

 private:  
   QHash<QString, CiteInfo> m_entries;
   QByteArray m_baseFileName;
};

#endif
