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

#ifndef USER_GROUP_H
#define USER_GROUP_H

/** This class contains a group of constructors.
  * The following notation will translate 'code' to behave like it says <code>true</code>
*/


class test_Group1
{
   public:

/** @name Group of Constructors
 *  Type safe and safer than exposing a var arg constructor.
 */

/** @{ */

   /** constructor with no parameters */
   groupTest1();

   groupTest1(long);             /**< constructor comment with two stars  */

   groupTest1(QString);          /*!< constructor comment with one star and one exclamation  */

   groupTest1(bool);             ///< constructor comment with three slashes

   groupTest1(int);              //!< constructor comment two slashes and one exclamation

/** @} */

};

/** Test a single comment for a group, project setting 'duplicate group documentation' must be enabled */
class test_Group2
{
   public:
   //! This is a comment applied to each member of the group
   ///@{
   int groupA()
      { return 35; }

   int groupB()
      { return 42; }
   ///@}

}

#endif