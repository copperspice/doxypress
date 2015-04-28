/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
 * All rights reserved.    
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License version 2
 * is hereby granted. No representations are made about the suitability of
 * this software for any purpose. It is provided "as is" without express or
 * implied warranty. See the GNU General Public License for more details.
 *
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#include <assert.h>

#include <arguments.h>
#include <marshal.h>

/*! the argument list is documented if one of its
 *  arguments is documented
 */
bool ArgumentList::hasDocumentation() const
{
   for (auto item : *this)  {

      if (item.hasDocumentation()) {
         return true;
      }
   }

   return false;
}

ArgumentList *ArgumentList::unmarshal(StorageIntf *s)
{
   uint i;
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return 0;   
   }

   ArgumentList *result = new ArgumentList;

   assert(count < 1000000);
  
   for (i = 0; i < count; i++) {
      Argument a;

      a.attrib  = unmarshalQByteArray(s);
      a.type    = unmarshalQByteArray(s);
      a.canType = unmarshalQByteArray(s);
      a.name    = unmarshalQByteArray(s);
      a.array   = unmarshalQByteArray(s);
      a.defval  = unmarshalQByteArray(s);
      a.docs    = unmarshalQByteArray(s);
      a.typeConstraint = unmarshalQByteArray(s);

      result->append(a);
   }

   result->constSpecifier     = unmarshalBool(s);
   result->volatileSpecifier  = unmarshalBool(s);
   result->pureSpecifier      = unmarshalBool(s);
   result->trailingReturnType = unmarshalQByteArray(s);

   return result;
}

void ArgumentList::marshal(StorageIntf *s, ArgumentList *argList)
{
   if (argList == 0) {
      marshalUInt(s, NULL_LIST); 

   } else {
      marshalUInt(s, argList->count());

      if (argList->count() > 0) {

         for (auto item : *argList)  {

            marshalQByteArray(s, item.attrib);
            marshalQByteArray(s, item.type);
            marshalQByteArray(s, item.canType);
            marshalQByteArray(s, item.name);
            marshalQByteArray(s, item.array);
            marshalQByteArray(s, item.defval);
            marshalQByteArray(s, item.docs);
            marshalQByteArray(s, item.typeConstraint);
         }
      }

      marshalBool(s, argList->constSpecifier);
      marshalBool(s, argList->volatileSpecifier);
      marshalBool(s, argList->pureSpecifier);
      marshalQByteArray(s, argList->trailingReturnType);
   }
}

