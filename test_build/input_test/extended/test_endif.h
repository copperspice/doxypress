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

#ifndef TEST_ENDIF_H
#define TEST_ENDIF_H

/// test for warning requarding the comment after the endif
class test_Endif
{

   /* comment one */


#if test
#endif      /// internal comment two
            /// more things to document


};

#endif      // comment after the end of the header file (does this cause a problem? Unable to repeat)
