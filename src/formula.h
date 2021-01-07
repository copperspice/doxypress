/************************************************************************
*
* Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
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

#ifndef FORMULA_H
#define FORMULA_H

#include <QList>
#include <QHash>

/** Class representing a formula in the output. */
class Formula
{
 public:
   Formula(const QString &text);

   int getId();
   QString getFormulaText() const {
      return form;
   }

 private:
   int number;
   QString form;
};

/** A list of Formula objects. */
class FormulaList : public QList<Formula>
{
 public:
   void generateBitmaps(const QString &path);
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
