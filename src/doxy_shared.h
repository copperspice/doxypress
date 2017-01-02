/*************************************************************************
 *
 * Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim 
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

#ifndef DOXY_SHARED_H
#define DOXY_SHARED_H

#include <cassert>

#include <QMap>
#include <QSharedPointer>

// ** part 1
class EnableSharedFromThis : public QEnableSharedFromThis<EnableSharedFromThis>
{
   public:
      virtual ~EnableSharedFromThis() 
      {    
      };
};

template<class T>
QSharedPointer<typename std::enable_if<std::is_base_of<EnableSharedFromThis, T>::value, T>::type> sharedFrom(T *ptr)
{ 
   QSharedPointer<T> retval;

   QSharedPointer<EnableSharedFromThis> tmp = ptr->sharedFromThis();
   retval = tmp.dynamicCast<T>();
     
   return retval;   
}


template<class T>
QSharedPointer<const typename std::enable_if<std::is_base_of<EnableSharedFromThis, T>::value, T>::type> sharedFrom(const T *ptr)
{ 
   QSharedPointer<const T> retval;

   QSharedPointer<const EnableSharedFromThis> tmp = ptr->sharedFromThis();
   retval = tmp.dynamicCast<const T>();
     
   return retval;   
}


#endif
