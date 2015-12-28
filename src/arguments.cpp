/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
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

/*! the argument list is documented if one of its arguments is documented
 */
bool ArgumentList::hasDocumentation() const
{
   for (auto &item : *this)  {

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

      a.attrib  = unmarshalQString(s);
      a.type    = unmarshalQString(s);
      a.canType = unmarshalQString(s);
      a.name    = unmarshalQString(s);
      a.array   = unmarshalQString(s);
      a.defval  = unmarshalQString(s);
      a.docs    = unmarshalQString(s);
      a.typeConstraint = unmarshalQString(s);

      result->append(a);
   }

   result->constSpecifier     = unmarshalBool(s);
   result->volatileSpecifier  = unmarshalBool(s);
   result->pureSpecifier      = unmarshalBool(s);
   result->trailingReturnType = unmarshalQString(s);
   result->isDeleted          = unmarshalBool(s);

   return result;
}

void ArgumentList::marshal(StorageIntf *s, const ArgumentList &argList)
{     
   marshalUInt(s, argList.count());    
   
   for (auto &item : argList)  {
      marshalQString(s, item.attrib);
      marshalQString(s, item.type);
      marshalQString(s, item.canType);
      marshalQString(s, item.name);
      marshalQString(s, item.array);
      marshalQString(s, item.defval);
      marshalQString(s, item.docs);
      marshalQString(s, item.typeConstraint);
   }
      
   marshalBool(s, argList.constSpecifier);
   marshalBool(s, argList.volatileSpecifier);
   marshalBool(s, argList.pureSpecifier);
   marshalQString(s, argList.trailingReturnType);
   marshalBool(s, argList.isDeleted);   
}

