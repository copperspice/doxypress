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

#ifndef USER_TESTS_H
#define USER_TESTS_H


/// docs for test_Tabs
class test_Tabs
{
public:
	/**
	*	Retrieves the list of ints for id.
	*	caller is responsible for deleting the container
	*
	* @param id	id
	* @return list of ints
	*/
	int* doSomething(long id) const;


   /** @brief This is a brief comment for hasABrief() */
   void hasABrief(double foo);
};

/// docs for test_Friend
class test_Friend
{
   public:
      /// does this cause a warning about return types? No issue found.
      friend class myFriend;
};

/// docs for test_TypeDef
class test_TypeDef
{
   public:
      typedef int myTypeDef;
      myTypeDef someMethod();
};


/// base class, test graphViz
class test_dotFruit
{
   public:
      virtual void chew(void);      ///< describe how to chew a fruit
};

/// child class, test graphViz
class test_dotApple : public test_dotFruit
{
   public:
      void chew();
};

#endif
