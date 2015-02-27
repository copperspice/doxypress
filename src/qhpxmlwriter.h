/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * Copyright (C) 2008 by Sebastian Pipping.
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
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

#ifndef QHPXMLWRITER_H
#define QHPXMLWRITER_H

#include <QByteArray>
#include <QString>
#include <QFile>
#include <QTextStream>

class QhpXmlWriter{

 public:
   QhpXmlWriter();
   ~QhpXmlWriter();

   void setIndentLevel(int level);
   void setCompressionEnabled(bool enabled);
   void insert(QhpXmlWriter const &source);
   void dumpTo(QFile &file);

   void open(const QString &elementName, const QMap<QString, QString> &attributes = QMap<QString, QString>());
   void openClose(const QString &elementName, const QMap<QString, QString> &attributes = QMap<QString, QString>());
   void openCloseContent(const QString &elementName, const QString &content);
   void close(const QString &elementName);
   void declaration(const char *version, const char *encoding);

   static char *dupEscaped(const char *source);

 private:
   void indent();
   void newLine();
   void openPureHelper(const QString &elementName, const QMap<QString, QString> &attributes, bool close);
   void openPure(const QString &elementName, const QMap<QString, QString> &attributes = QMap<QString, QString>());
   void openClosePure(const QString &elementName, const QMap<QString, QString> &attributes = QMap<QString, QString>());
   void closePure(const QString &elementName);

   QByteArray m_backend;
   QTextStream m_out;
   int  m_indentLevel;
   bool m_curLineIndented;
   bool m_compress;

};

#endif
