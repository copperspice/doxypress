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

#ifndef DOXY_SHARED_H
#define DOXY_SHARED_H

#include <QMap>
#include <QSharedPointer>
#include <QWeakPointer>

// ** 
class EnableSharedFromThis 
{
   public: 
      virtual ~EnableSharedFromThis() {};
  
      static QMap<const EnableSharedFromThis *, QWeakPointer<EnableSharedFromThis>> &m_selfMap();
};

template<class T>
void QRegisterSelf(QSharedPointer<T> ptr)
{
   EnableSharedFromThis::m_selfMap().insert(ptr.data(), ptr);
}

template<class T>
QSharedPointer<typename std::enable_if<std::is_base_of<EnableSharedFromThis, T>::value, T>::type> sharedFrom(const T *ptr)
{ 
   QSharedPointer<T> retval;
   auto &map = EnableSharedFromThis::m_selfMap();
   
   auto item = map.find(ptr);    

   if (item != map.end())  {
      retval = item->toStrongRef().template dynamicCast<T>();

   } else {
      std::string className = typeid(*ptr).name();
      throw std::runtime_error("\nDoxyPress::SharedFrom() Class " + className + " was not properly registerd. Please submit a bug report.");
   }
  
   return retval;   
}

// **
template<class T, class ...Ts>
QSharedPointer<typename std::enable_if<! std::is_base_of<EnableSharedFromThis, T>::value, T>::type> QMakeShared(Ts &&...vs)
{
   return QSharedPointer<T> (new T(vs...));
}

template<class T, class ...Ts>
QSharedPointer<typename std::enable_if<std::is_base_of<EnableSharedFromThis, T>::value, T>::type> QMakeShared(Ts &&...vs)
{
   QSharedPointer<T> retval = QSharedPointer<T> (new T(vs...));
   QRegisterSelf(retval);

   return retval;
}

#endif
