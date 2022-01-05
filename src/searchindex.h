/************************************************************************
*
* Copyright (c) 2014-2022 Barbara Geller & Ansel Sermersheim
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

#ifndef SEARCHINDEX_H
#define SEARCHINDEX_H

#include <QHash>
#include <QList>
#include <QVector>

#include <stringmap.h>

class Definition;
class MemberDef;
class SearchDocEntry;

void initSearchIndexer();

// server side search index

struct URL {
   URL(const QString &n, const QString &u) : name(n), url(u) {}
   QString name;
   QString url;
};

struct URLInfo {
   URLInfo(int idx, int f) : urlIdx(idx), freq(f) {}
   int urlIdx;
   int freq;
};

class SearchDefinitionList : public QList<QSharedPointer<Definition>>
{
   public:
      SearchDefinitionList(QChar letter) : m_letter(letter)
      {}

      QChar letter() const {
         return m_letter;
      }

   private:
      QChar m_letter;
};

class IndexWord
{
 public:
   IndexWord(const QString &word);
   void addUrlIndex(int, bool);

   const QHash<long, QSharedPointer<URLInfo>> &urls() const {
      return m_urls;
   }

   QString word() const {
      return m_word;
   }

 private:
   QString m_word;
   QHash<long, QSharedPointer<URLInfo>> m_urls;
};

class SearchIndex_Base
{
 public:
   enum Kind { Internal, External };

   SearchIndex_Base(Kind k) : m_kind(k) {}

   virtual ~SearchIndex_Base() {}

   virtual void setCurrentDoc(QSharedPointer<Definition> ctx, const QString &anchor, bool isSourceFile) = 0;
   virtual void addWord(const QString &word, bool hiPriority) = 0;
   virtual void write(const QString &file) = 0;

   Kind kind() const {
      return m_kind;
   }

 private:
   Kind m_kind;
};

class SearchIndex : public SearchIndex_Base
{
 public:
   SearchIndex();
   ~SearchIndex();

   void setCurrentDoc(QSharedPointer<Definition> ctx, const QString &anchor, bool isSourceFile) override;
   void addWord(const QString &word, bool hiPriority) override;
   void write(const QString &file) override;

 private:
   void addWord(const QString &word, bool hiPrio, bool recurse);

   QMap<QString, QSharedPointer<IndexWord>> m_words;

   QHash<QString,int> m_url2IdMap;
   QHash<long, QSharedPointer<URL>> m_urls;

   int m_urlIndex;
};

class SearchIndex_External : public SearchIndex_Base
{

 public:
   SearchIndex_External();
   ~SearchIndex_External();
   void setCurrentDoc(QSharedPointer<Definition> ctx, const QString &anchor, bool isSourceFile) override;
   void addWord(const QString &word, bool hiPriority) override;
   void write(const QString &file) override;

 private:
   StringMap<QSharedPointer<SearchDocEntry>> m_docEntries;
   QSharedPointer<SearchDocEntry> m_current;

};

void writeJavascriptSearchIndex();

#endif
