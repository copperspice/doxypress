/************************************************************************
*
* Copyright (C) 2014-2019 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
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
