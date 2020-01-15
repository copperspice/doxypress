/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
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

   void open(const QString &elementName, const QMap<QString, QString> &attributes = (QMap<QString, QString>()));
   void openClose(const QString &elementName, const QMap<QString, QString> &attributes = (QMap<QString, QString>()));
   void openCloseContent(const QString &elementName, const QString &content);
   void close(const QString &elementName);
   void declaration(const QString &version, const QString &encoding);

   static QString dupEscaped(const QString &source);

 private:
   void indent();
   void newLine();
   void openPureHelper(const QString &elementName, const QMap<QString, QString> &attributes, bool close);
   void openPure(const QString &elementName, const QMap<QString, QString> &attributes = (QMap<QString, QString>()));
   void openClosePure(const QString &elementName, const QMap<QString, QString> &attributes = (QMap<QString, QString>()));
   void closePure(const QString &elementName);

   QByteArray m_backend;
   QTextStream m_out;
   int  m_indentLevel;
   bool m_curLineIndented;
   bool m_compress;

};

#endif
