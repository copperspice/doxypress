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

/** This class contains a group of constructors */

class groupTest
{
   public:

/** @name Group of Constructors
 *  Type safe and safer than exposing a var arg constructor.
 */

/** @{ */

   /** constructor a - this works */
   groupTest();

   groupTest(long);             /**< constructor b  */

   groupTest(QString);          /*!< constructor e  */

   groupTest(bool);             ///< constructor c

   groupTest(int);              //!< constructor d

/** @} */

};