/*************************************************************************
*
* Copyright (C) 2012-2018 Barbara Geller & Ansel Sermersheim
* All rights reserved.
*
* GNU Free Documentation License
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