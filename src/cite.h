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

#ifndef CITE_H
#define CITE_H

#include <QHash>
#include <QTextStream>

/// String constants for citations
struct CiteConsts {
   static const QString fileName;
   static const QString anchorPrefix;
};

/// Citation-related data.
struct CiteInfo {
   CiteInfo(const  QString & t_label, const QString &t_text = QString(), const  QString &t_fullText = QString(), 
                  const QString &t_ref = QString() )
      : label(t_label), text(t_text), fullText(t_fullText), ref(t_ref) {
   }

   CiteInfo(const CiteInfo &o) {
      label    = o.label;
      text     = o.text;
      fullText = o.fullText;
      ref      = o.ref;
   }

   QString label;
   QString text;
   QString fullText;
   QString ref;
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
   void insert(const QString &label);

   /** Return the citation info for a given \a label */
   CiteInfo *find(const QString &label) const;

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
   void writeLatexBibliography(QTextStream &t);

 private:  
   QHash<QString, CiteInfo> m_entries;
   QString m_baseFileName;
};

#endif
