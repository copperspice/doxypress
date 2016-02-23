/*************************************************************************
*                                                                         
* Copyright (C) 2012-2016 Barbara Geller & Ansel Sermersheim                                                         
* All rights reserved.                                                    
*                                                                         
*                                                                         
* GNU Free Documentation License                                          
* This file may be used under the terms of the GNU Free Documentation     
* License version 1.3 as published by the Free Software Foundation        
* and appearing in the file included in the packaging of this file.       
*                                                                         
*                                                                         
*************************************************************************/

#ifndef USER_TESTS_H
#define USER_TESTS_H


class TestTabs
{
   public:
      /**
      * 	Retrieves a list of integers  ( has a tab, causes a problem ? #1 )
      * 	caller is responsible for deleting them
      *
      * @param myvar	some purpose or other
      * @return list of integers
      */
      int* doSomething(long myvar) const;

};

class TestFriend
{
   public:
      // my cause a warning about return types not argreeing #2
      friend class myFriend;

};

class TestTypeDef
{
   public:
      typedef int myTypeDef;
      myTypeDef someMethod(); 
};


#endif      // some comment after the end of the header file (does this cause a problem?  #4)