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

#ifndef HTML_ENTITY_H
#define HTML_ENTITY_H

#include <QHash>
#include <QString>
#include <QTextStream>

#include <docparser.h>

class HtmlEntityMapper
{
 public:
   static HtmlEntityMapper *instance();
   static void deleteInstance();

   DocSymbol::SymType name2sym(const QString &symName) const;
   QString rawString(DocSymbol::SymType symb, bool useInPrintf = false) const;
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
