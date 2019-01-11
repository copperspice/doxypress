/************************************************************************
*
* Copyright (C) 2014-2019 Barbara Geller & Ansel Sermersheim
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

#ifndef HTMLATTRIB_H
#define HTMLATTRIB_H

#include <QString>
#include <QList>

/*! A Html option. A name, value pair */
struct HtmlAttrib {
   QString name;
   QString value;
};

/*! @brief A list of Html attributes.
 *
 * The Html attributes are deeply copied into the list.
 */
class HtmlAttribList : public QList<HtmlAttrib>
{
 public:
   HtmlAttribList() : QList<HtmlAttrib>() {
   }

   ~HtmlAttribList() {
      clear();
   }

   HtmlAttribList(const HtmlAttribList &l) : QList<HtmlAttrib>() {
      operator=(l);
   }

   HtmlAttribList &operator=(const HtmlAttribList &l) {
      clear();
      QList<HtmlAttrib>::operator=(l);
      return *this;
   }

   QString find(const QString &name) const {
      for (auto &item : *this) {
         if (item.name == name) {
            return item.value;
         }
      }
      return QString("");
   }

   QString toString() const {
      QString result;

      for (auto &item : *this) {
         result += " " + item.name + "=\"" + item.value + "\"";
      }
      return result;
   }

 private:
   HtmlAttrib *newValue( HtmlAttrib *v ) const {
      return new HtmlAttrib(*v);
   }
   void deleteValue(HtmlAttrib *v) const {
      delete v;
   }
};

#endif

