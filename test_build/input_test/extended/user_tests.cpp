/*************************************************************************
*
* Copyright (C) 2012-2019 Barbara Geller & Ansel Sermersheim
*
* GNU Free Documentation License
*
*************************************************************************/

testTypeDef::myTypeDef testTypeDef::someMethod()
{
   /// empty method is ok, does this cause a return type mismatch?
}



/**
Documenting class TagTest.

\if MY_TAG
This text should not appear in the output since MY_TAG would need to be true, and it is not set.  BAD
\endif
\ifnot MY_TAG
This text will appear in the output since MY_TAG is false.  ( GOOD, this should show up in the output )
\endif
*/
class TagTest
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

