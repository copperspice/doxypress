/*************************************************************************
*
* Copyright (C) 2012-2019 Barbara Geller & Ansel Sermersheim
*
* GNU Free Documentation License
*
*************************************************************************/

#ifndef TEST_RETURN_H
#define TEST_RETURN_H

class test_ReturnType 
{
   using iterator        = void *;
   using const_iterator  = const void *;

   public:
      iterator begin() {
         return m_data.begin();
      }

      const_iterator begin() const {
         return m_data.begin();
      }

      void return_IsVoid();
      bool return_IsBool();
};

#endif