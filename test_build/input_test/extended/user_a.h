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

#ifndef INCLUDE_USER_DUPLICATE
#define INCLUDE_USER_DUPLICATE

#include <exception>

/**
 * @class user_duplicate
 *
 * @brief Some simple brief notation for user_a file
 *
 */
class user_duplicate
{
   public:
      friend class lime;
      friend class lemon;

      class std::exception someMethod();
};

#endif