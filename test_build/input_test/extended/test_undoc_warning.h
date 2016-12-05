#ifndef INCLUDED_APM
#define INCLUDED_APM

class user_apm
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