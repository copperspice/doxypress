/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim 
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
      return "";
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

/*! @brief Html attribute list iterator */
class HtmlAttribListIterator : public QListIterator<HtmlAttrib>
{
 public:
   HtmlAttribListIterator(const HtmlAttribList &l) : QListIterator<HtmlAttrib>(l) {}
};

#endif

