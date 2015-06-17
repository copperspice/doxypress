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

#ifndef SEARCHINDEX_H
#define SEARCHINDEX_H

#include <QHash>
#include <QList>
#include <QVector>

#include <stringmap.h>

class Definition;
class MemberDef;
class SearchDocEntry;

/*! Initialize the search indexer */
void initSearchIndexer();

/*! Cleanup the search indexer */
void finializeSearchIndexer();

//------- server side search index ----------------------

struct URL {
   URL(const char *n, const char *u) : name(n), url(u) {}
   QByteArray name;
   QByteArray url;
};

struct URLInfo {
   URLInfo(int idx, int f) : urlIdx(idx), freq(f) {}
   int urlIdx;
   int freq;
};

class IndexWord
{
 public:
   IndexWord(const char *word);
   void addUrlIndex(int, bool);

   const QHash<long, QSharedPointer<URLInfo>> &urls() const {
      return m_urls;
   }

   QByteArray word() const {
      return m_word;
   }

 private:
   QByteArray m_word;
   QHash<long, QSharedPointer<URLInfo>> m_urls;
};

class SearchIndexIntf
{
 public:
   enum Kind { Internal, External };

   SearchIndexIntf(Kind k) : m_kind(k) {}

   virtual ~SearchIndexIntf() {}

   virtual void setCurrentDoc(QSharedPointer<Definition> ctx, const char *anchor, bool isSourceFile) = 0;
   virtual void addWord(const QString &word, bool hiPriority) = 0;
   virtual void write(const QString &file) = 0;

   Kind kind() const {
      return m_kind;
   }

 private:
   Kind m_kind;
};

class SearchIndex : public SearchIndexIntf
{
 public:
   SearchIndex();
   ~SearchIndex();

   void setCurrentDoc(QSharedPointer<Definition> ctx, const char *anchor, bool isSourceFile) override;
   void addWord(const QString &word, bool hiPriority) override;
   void write(const QString &file) override;

 private:
   void addWord(const QString &word, bool hiPrio, bool recurse);

   QHash<QString, IndexWord *>  m_words;         // merge these two containers (broom -- on hold)
   QVector<QList<IndexWord *>>  m_index;

   QHash<QString,int> m_url2IdMap;

   QHash<long, QSharedPointer<URL>> m_urls;

   int m_urlIndex;
};


class SearchIndexExternal : public SearchIndexIntf
{
  
 public:
   SearchIndexExternal();
   ~SearchIndexExternal();
   void setCurrentDoc(QSharedPointer<Definition> ctx, const char *anchor, bool isSourceFile) override;
   void addWord(const QString &word, bool hiPriority) override;
   void write(const QString &file) override;

 private:   
   StringMap<QSharedPointer<SearchDocEntry>> m_docEntries;
   QSharedPointer<SearchDocEntry> m_current;

};

//------- client side search index ----------------------

void writeJavascriptSearchIndex();

#endif
