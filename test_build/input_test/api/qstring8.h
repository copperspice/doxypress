/************************************************************************
*
* Copyright (c) 2014-2025 Barbara Geller & Ansel Sermersheim
*
* DoxyPress is free software. You can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
* https://www.gnu.org/licenses/
*
*************************************************************************/

#ifndef QSTRING8_H
#define QSTRING8_H

#include <cs_string.h>
#include <qglobal.h>

class QString8
{
   public:
      QString8() = default;

      QString8(const QString8 &other) = delete;
      QString8(QString8 &&other) = default;

      QString8::QString8 ( QChar32 c );

      // methods
      void clear();

      bool endsWith(const QString8 &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

      bool isEmpty() const;

      QString8 left(size_type numOfChars) const;
      QString8 leftJustified(size_type width, QChar32 fill = UCHAR(' '), bool trunc = false) const;

      bool startsWith(const QString8 &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

      void truncate(size_type length);

      // operators
      QString8 &operator=(const QString8 &other) = default;
      QString8 &operator=(QString8 && other) = default;

      QString8 &operator=(QChar32 c);

      QString8 &operator+=(QChar32 c);
      QString8 &operator+=(QChar32::SpecialCharacter c);
      QString8 &operator+= (const QString8 & other);

      QChar32 operator[](size_type index) const;

      // test for nested class
      class EmptyClass{ };
};


#endif