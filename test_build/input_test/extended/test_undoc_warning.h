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

#ifndef INCLUDE_USER_UNDOC_PARAM
#define INCLUDE_USER_UNDOC_PARAM

/** @brief User requested test for undocumented parameter warning */
class user_undoc_param
{
public:

   /**
     * Return TRUE if something.
     * The whatjamacallit is specified somehow.
     *
    * @param[in] pObject the object
    * @param[in] name the name
    * @param[in] date the start date to query
    * @return TRUE if something, FALSE otherwise
    */
   RWBoolean do_something(const SomeClass* pObject, const RWCString& name, int date);
};

#endif