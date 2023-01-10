/************************************************************************
*
* Copyright (c) 2014-2023 Barbara Geller & Ansel Sermersheim
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

#ifndef QMAP_H
#define QMAP_H

#include <map>

template <typename Key, typename Val, typename C>
class QMap
{
   public:
      QMap() = default;

      QMap(const QMap<Key, Val, C> &other) = default;
      QMap(QMap<Key, Val, C> &&other)      = default;

      explicit QMap(const std::map<Key, Val, C> &other)
         : m_data(other) {}

      ~QMap() = default;

      // methods
      void clear() {
         m_data.clear();
      }

      bool contains(const Key &key) const {
         return m_data.find(key) != m_data.end();
      }

   private:
      std::map<Key, Val, C> m_data;

};

#endif