/*************************************************************************
*
* Copyright (C) 2012-2019 Barbara Geller & Ansel Sermersheim
*
* GNU Free Documentation License
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
