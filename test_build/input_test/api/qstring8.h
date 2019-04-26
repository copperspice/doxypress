/*************************************************************************
*
* Copyright (C) 2012-2019 Barbara Geller & Ansel Sermersheim
*
* GNU Free Documentation License
*
*************************************************************************/

#ifndef QSTRING8_H
#define QSTRING8_H

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