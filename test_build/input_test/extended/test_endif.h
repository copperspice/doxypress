/*************************************************************************
*
* Copyright (C) 2012-2016 Barbara Geller & Ansel Sermersheim
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
