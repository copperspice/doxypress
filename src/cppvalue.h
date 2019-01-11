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

#ifndef CPPVALUE_H
#define CPPVALUE_H

#include <stdio.h>

//  class representing a C-preprocessor value
class CPPValue
{
 public:
   enum Type { Int, Float };

   CPPValue(long val = 0) : type(Int) {
      v.l = val;
   }

   CPPValue(double val) : type(Float) {
      v.d = val;
   }

   operator double () const {
      return type == Int ? (double)v.l : v.d;
   }

   operator long ()   const {
      return type == Int ? v.l : (long)v.d;
   }

   bool isInt() const {
      return type == Int;
   }

   void print() const {
      if (type == Int) {
         printf("(%ld)\n", v.l);

      } else {
         printf("(%f)\n", v.d);
      }
   }

 private:
   Type type;

   union {
      double d;
      long l;
   } v;
};

CPPValue parseOctal();
CPPValue parseDecimal();
CPPValue parseHexadecimal();
CPPValue parseCharacter();
CPPValue parseFloat();

#endif
