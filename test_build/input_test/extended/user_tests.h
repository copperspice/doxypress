/*************************************************************************
*
* Copyright (C) 2012-2017 Barbara Geller & Ansel Sermersheim
* All rights reserved.
*
* GNU Free Documentation License
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
