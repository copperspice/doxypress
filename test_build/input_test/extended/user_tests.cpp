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

#include "user_tests.h"

testTypeDef::myTypeDef testTypeDef::someMethod()
{
   /// empty method is ok, does this cause a return type mismatch?
}


/**
Documenting class test_Tag.

\if MY_TAG
This text should not appear in the output since MY_TAG would need to be true, and it is not set.  BAD
\endif
\ifnot MY_TAG
This text will appear in the output since MY_TAG is false.  ( GOOD, this should show up in the output )
\endif
*/
class test_Tag
{
};


/// Test overflow issue for Class_A through Class_E
class Class_B : public Class_A
{
   public:
      /** Documentation for method b1 */
      void b1();
};


class Class_C : public Class_B
{
   public:
};


template <class T1, class T2>
class Class_D : public Class_C
{
   public:
};


template <class T>
class Class_E : public Class_D<T, T>
{
   public:
};

