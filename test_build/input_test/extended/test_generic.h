/************************************************************************
*
* Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
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

#ifndef TEST_GENERIC_H
#define USER_GENERIC_H

/// docs for test_slot
class test_Slot
{
   public:
      CS_SLOT_1(Public, inline const QMap<QString, bool> &serviceOwner(const QString & name) const)
      CS_SLOT_2(serviceOwner)

      CS_SLOT_1(Public, inline void setOffset(qreal dx))
      CS_SLOT_2(setXOffset)

      inline QString color();
      const int &shape();
};

inline void test_Slot::setOffset(qreal dx)
{
   int foo;
   foo = 1 + 1;
}

inline QString color()
{
   return "blue";
}

#endif