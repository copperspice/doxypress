/*************************************************************************
*
* Copyright (C) 2012-2018 Barbara Geller & Ansel Sermersheim
* All rights reserved.
*
* GNU Free Documentation License
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