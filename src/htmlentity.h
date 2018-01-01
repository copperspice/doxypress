/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim 
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

#ifndef HTMLENTITY_H
#define HTMLENTITY_H

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QTextStream>

#include <docparser.h>

/** @brief Singleton helper class to map html entities to other formats */
class HtmlEntityMapper
{
 public:
   static HtmlEntityMapper *instance();
   static void deleteInstance();

   DocSymbol::SymType name2sym(const QString &symName) const;
   QString utf8(DocSymbol::SymType symb, bool useInPrintf = false) const;
   QString html(DocSymbol::SymType symb, bool useInPrintf = false) const;
   QString xml(DocSymbol::SymType symb) const;
   QString docbook(DocSymbol::SymType symb) const;
   QString latex(DocSymbol::SymType symb) const;
   QString man(DocSymbol::SymType symb) const;
   QString rtf(DocSymbol::SymType symb) const;
   const DocSymbol::PerlSymb *perl(DocSymbol::SymType symb) const;
   void  writeXMLSchema(QTextStream &t);

 private:
   void  validate();
   HtmlEntityMapper();
   ~HtmlEntityMapper();

   static HtmlEntityMapper *s_instance;
   QHash<QString, int>      m_name2sym;
};

#endif
