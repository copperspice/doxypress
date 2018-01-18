/*************************************************************************
*
* Copyright (C) 2012-2018 Barbara Geller & Ansel Sermersheim
* Copyright (C) 2014 Digia Plc
* All rights reserved.
*
* GNU Free Documentation License
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