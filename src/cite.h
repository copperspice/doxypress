/*************************************************************************
 *
 * Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim 
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

// This class provides access do the database of bibliographic references through the bibtex backend.
class CiteDict
{
 public:   
   CiteDict();

   /** Insert a citation identified by \a label into the database */
   void insert(const QString &label);

   /** Return the citation info for a given label */
   QString find(const QString &label) const;

   /** Generate the citations page */
   void generatePage() const;

   /** clears the hash */
   void clear();

   /** return true if there are no citations   */
   bool isEmpty() const;

   /* writes the latex code for the bibliography section to text stream */
   void writeLatexBibliography(QTextStream &t);

 private:  
   mutable QMap<QString, QString> m_entries;
   QString m_baseFileName;
};

#endif
