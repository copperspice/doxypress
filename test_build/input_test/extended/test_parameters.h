/*************************************************************************
*
* Copyright (C) 2012-2018 Barbara Geller & Ansel Sermersheim
* All rights reserved.
*
* GNU Free Documentation License
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

