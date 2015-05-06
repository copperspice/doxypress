/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
 * Copyright (C) 2008 by Sebastian Pipping. 
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

#include <qhpxmlwriter.h>
#include <util.h>

QhpXmlWriter::QhpXmlWriter()
   : m_out(&m_backend), m_indentLevel(0), m_curLineIndented(false), m_compress(false)
{
}

QhpXmlWriter::~QhpXmlWriter()
{
}

void QhpXmlWriter::setIndentLevel(int level)
{
   m_indentLevel = level;
}

void QhpXmlWriter::setCompressionEnabled(bool enabled)
{
   m_compress = enabled;
}

void QhpXmlWriter::insert(QhpXmlWriter const &source)
{
   m_out << source.m_backend.data();
}

void QhpXmlWriter::dumpTo(QFile &file)
{
   file.write(m_backend.data(), m_backend.length());
}

void QhpXmlWriter::open(const QString &elementName, const QMap<QString, QString> &attributes)
{
   indent();
   openPure(elementName, attributes);
   newLine();
   m_indentLevel++;
}

void QhpXmlWriter::openClose(const QString &elementName, const QMap<QString, QString> &attributes)
{
   indent();
   openClosePure(elementName, attributes);
   newLine();
}

void QhpXmlWriter::openCloseContent(const QString &elementName, const QString &content)
{
   indent();
   openPure(elementName);
   m_out << convertToXML(content);
   closePure(elementName);
   newLine();
}

void QhpXmlWriter::close(const QString &elementName)
{
   m_indentLevel--;
   indent();
   closePure(elementName);
   newLine();
}

void QhpXmlWriter::declaration(char const *version, char const *encoding)
{
   m_out << "<?xml version=\"" << version << "\" encoding=\"" << encoding << "\"?>";
   newLine();
}

void QhpXmlWriter::indent()
{
   if (m_curLineIndented) {
      return;
   }
   for (int i = 0; i < m_indentLevel; i++) {
      m_out << "  ";
   }
   m_curLineIndented = true;
}

void QhpXmlWriter::newLine()
{
   if (! m_compress) {
      m_out << "\n";
      m_curLineIndented = false;
   }
}

void QhpXmlWriter::openPureHelper(const QString &elementName, const QMap<QString, QString> &attributes, bool close)
{
   m_out << "<" << elementName;
    
   for (auto iter = attributes.begin(); iter != attributes.end(); ++iter) {   

      QString key   = iter.key();
      QString value = iter.value();

      if (value.isEmpty()) {
         continue;
      }

      m_out << " " << key << "=\"" << convertToXML(value) << "\"";
   }


   if (close) {
      m_out << " /";
   }

   m_out << ">";
}

void QhpXmlWriter::openPure(const QString &elementName, const QMap<QString, QString> &attributes)
{
   openPureHelper(elementName, attributes, false);
}

void QhpXmlWriter::openClosePure(const QString &elementName, const QMap<QString, QString> &attributes)
{
   openPureHelper(elementName, attributes, true);
}

void QhpXmlWriter::closePure(const QString &elementName)
{
   m_out << "</" << elementName << ">";
}

