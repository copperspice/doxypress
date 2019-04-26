/*************************************************************************
*
* Copyright (C) 2012-2019 Barbara Geller & Ansel Sermersheim
*
* GNU Free Documentation License
*
*************************************************************************/

#ifndef TEST_GENERIC_H
#define USER_GENERIC_H

/// docs for test_slot
class test_Slot
{
   public:     
      CS_SLOT_1(Public, inline const QMap<QString, bool> &serviceOwner(const QString & name) const)
      CS_SLOT_2(serviceOwner)    

      CS_SLOT_1(Public, inline void setOffset(qreal dx))
      CS_SLOT_2(setXOffset)

      inline QString color();
      const int &shape();
};


inline void test_Slot::setOffset(qreal dx)
{
   int foo;
   foo = 1 + 1;  
}

inline QString color()
{
   return "blue";
}


#endif