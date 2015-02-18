/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
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

#ifndef FORMULA_H
#define FORMULA_H

#include <QList>
#include <QHash>

/** Class representing a formula in the output. */
class Formula
{
 public:
   Formula(const char *text);
   ~Formula();
   int getId();
   QByteArray getFormulaText() const {
      return form;
   }

 private:
   int number;
   QByteArray form;
};

/** A list of Formula objects. */
class FormulaList : public QList<Formula>
{
 public:
   void generateBitmaps(const char *path);
};

/** Iterator for Formula objects in a FormulaList. */
class FormulaListIterator : public QListIterator<Formula>
{
 public:
   FormulaListIterator(const FormulaList &l) :
      QListIterator<Formula>(l) {}
};

/** Unsorted dictionary of Formula objects. */
class FormulaDict : public QHash<QString, Formula>
{
 public:
   FormulaDict() : QHash<QString, Formula>() {}

   ~FormulaDict() {}
};

#endif
