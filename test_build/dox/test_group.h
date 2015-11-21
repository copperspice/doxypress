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

   /** constructor with no parameters */
   groupTest1();

   groupTest2(long);             /**< constructor comment with two stars  */

   groupTest3(QString);          /*!< constructor comment with one star and one exclamation  */

   groupTest4(bool);             ///< constructor comment with three slashes

   groupTest5(int);              //!< constructor comment two slashes and one exclamation 

   /// Normal documentation on line one.
   /// Continued standard documentation on line two.
   documentTest();

   bbTest1();
      // Documentation on line one.
      // Continue with some *italic* using a star and then _italic_ with an underscore.
      // This is a link {Valid Date Values and Their Representations}.
      // Show text in !bold-good! and now fail a !bold-bad1(! and then fail again !bold-bad2 ! end test.

   bbTest2();
      // Return 'tiny font' text.
      // Sample text: (1) 'Date's   'Date'd   'Date'ed   'Date'th    'Date'ing
      // The next link is split on two lines. See {Valid Date Values and
      // Their Representations} for details.

   // INFO: this comment which should not appear
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


