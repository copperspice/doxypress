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

/// Class documentation is from the DoxyPress documentation
class test_Enum
{
  public:
    enum fruit { pear, orange };

    /*! Another enum, with inline docs */
    enum veggies
    {
      celery,   /*!< docs for value one */
      carrot    /*!< docs for value two */
    };

   /// Documentation for the Cajun Spices enum
   enum class CajunSpices {
      hungarian_paprika,
      hot_paprika,
      oregano,
      thyme
   };

   /// Documentation for the Moroccan Spices enum
   enum class MoroccanSpices {
      allspice,
      cinnamon,
      ginger,
      red_pepper,     ///< using New Mexico red peppers gives a better flavor
      thyme
   };

};

/*! \class test_Enum
 * The class description.
 */


/*! \enum test_Enum::fruit
 * A description of the fruit enum.
 */


/*! \var test_Enum::fruit  test_Enum::pear
 * Description of the first fruit which is a pear.
 */


/*! \var test_Enum::CajunSpices  test_Enum::CajunSpices::thyme
 * use dried thyme
 */


/*! \var test_Enum::MoroccanSpices  test_Enum::MoroccanSpices::thyme
 * use French thyme if available
 */

