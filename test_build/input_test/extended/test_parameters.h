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

#ifndef test_Parameters_h
#define test_Parameters_h

/** \class test_Parameters
 * \brief  This is the brief description for the class test_Parameters.
 */
class test_Parameters
{
public:
    /**
     * Returns TRUE if blah
     * @param param1 the date to check
     * @param param2 the param2 string
     * @param param3 the param3 object
     * @param param4 the param4 details
     * @param param5 the param5 information.
     * @param param6 the param6 documentation
     * @return Returns TRUE if blah
     *
     * Pure-virtual method used to blah
     */
    virtual RWBoolean doWork(const RWDate &param1, const RWCString &param2, const RWCString *param3,
            const RWBoolean param4, const RWBoolean param5, const RWBoolean param6) const = 0;

};

#endif

