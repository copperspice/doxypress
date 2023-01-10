/************************************************************************
*
* Copyright (c) 2014-2023 Barbara Geller & Ansel Sermersheim
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

#ifndef TEST_RETURN_H
#define TEST_RETURN_H

class test_ReturnType
{
   using iterator        = void *;
   using const_iterator  = const void *;

   public:
      iterator begin() {
         return m_data.begin();
      }

      const_iterator begin() const {
         return m_data.begin();
      }

      void return_IsVoid();
      bool return_IsBool();
};

#endif