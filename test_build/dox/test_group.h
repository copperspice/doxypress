/*************************************************************************
*
* Copyright (C) 2012-2015 Barbara Geller & Ansel Sermersheim
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

/** This class contains a group of constructors. 
  * The following notation will translate 'code' to behave like it says <code>true</code> 
*/


class groupTest
{
   public:

/** @name Group of Constructors
 *  Type safe and safer than exposing a var arg constructor.
 */

/** @{ */

   /** constructor a */
   groupTest1();

   groupTest2(long);             /**< constructor b  */

   groupTest3(QString);          /*!< constructor e  */

   groupTest4(bool);             ///< constructor c  broom

   groupTest5(int);              //!< constructor d

   /// Normal documentation on line one.
   /// Continued standard documentation on line two.
   documentTest();

   bbTest();
      // bb documentation line one.
      // continue with *italic* or _italic_
      // Return 'tiny font' line three.
      // See {Valid Date Values and Their Representations} for details
      // show me a !bold!   


   // INFO: bb comment which should not appear
   someMethodwithNoDocs();

/** @} */
   
};


class Fruit 
{
   public:
      virtual void chew(void);      ///< describe how to chew a fruit
};

class Apple : public Fruit
{
   public:
      void chew();                 

};



