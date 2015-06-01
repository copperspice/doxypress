/*************************************************************************
*                                                                         
* Copyright (C) 2012-2015 Barbara Geller & Ansel Sermersheim                                                         
* All rights reserved.                                                    
*                                                                         
*                                                                         
* GNU Free Documentation License                                          
* This file may be used under the terms of the GNU Free Documentation     
* License version 1.3 as published by the Free Software Foundation        
* and appearing in the file included in the packaging of this file.       
*                                                                         
*                                                                         
*************************************************************************/

#ifndef GINGER_H
#define GINGER_H

#include "QList.h"
#include "QString.h"

class Ginger : public CSObject
{
   CS_OBJECT(Ginger)    
   
   CS_ENUM(Spices) 
   // title is type QString
     CS_PROPERTY_READ(title,       getTitle)
     CS_PROPERTY_WRITE(title,      setTitle)
     CS_PROPERTY_RESET(title,      resetTitle)
     CS_PROPERTY_NOTIFY(title,     titleChanged)
    
   // not documented at this time
//   CS_PROPERTY_REVISION(title,   31415)
//   CS_PROPERTY_DESIGNABLE(title, true)
//   CS_PROPERTY_SCRIPTABLE(title, isScriptTitle()   )
//   CS_PROPERTY_STORED(title,     m_title.isEmpty() )
//   CS_PROPERTY_USER(title,       10 > 100)
//   CS_PROPERTY_CONSTANT(title)
//   CS_PROPERTY_FINAL(title)
    
   // favorite is type enum Spices
   CS_PROPERTY_READ(favorite,   getFavorite)
   CS_PROPERTY_WRITE(favorite,  setFavorite)
   CS_PROPERTY_NOTIFY(favorite, ready)
           
   // test goes out (1)
   Q_OBJECT  
   Q_ENUM(Spices)      
   Q_PROPERTY(QString title_Qt  READ getTitle  WRITE setTitle)
  
   public:
      Ginger();
      ~Ginger();

      // registration is only reguired if the enum is used in a proptery or a signal
      // CS_REGISTER_ENUM( enum Spices { mint, basil, Salt, Pepper =100, cloves }; )
      enum Spices { mint, basil, Salt, Pepper =100, cloves };

      enum Brands { Chinese, Jamaican, Thai, Yellow }; 

      // test goes out (2)
      enum Spice_Qt { Allspice, Mace }; 

      using AmountType = double;

      void setAmount(double amt);   
      double getAmount();     

      bool isGrated();
      bool isChopped();

      QList<int> takesRValue(QString && dataA);
   
      inline bool isPowder() {
         return false;
      }

      static bool isSpice();

      // (1) methods for title property
      QString getTitle() const {
         return m_title;
      }

      void setTitle(QString data) {        
         m_title = data;        
         emit titleChanged(m_title);
      }

      void resetTitle();     

      CS_SIGNAL_1(Public, void titleChanged(QString title))
      CS_SIGNAL_2(titleChanged, title)

      // (2) methods for favorite property
      Spices getFavorite() const {
         return m_favorite;
      }

      void setFavorite(Spices data) {
         m_favorite = data;
      }

      CS_SLOT_1(Public, void newTitle(QString))
      CS_SLOT_2(newTitle)

      CS_SIGNAL_1(Public, void ready())
      CS_SIGNAL_2(ready)    
      
   protected:
      QString someProtected();    

      CS_SIGNAL_1(Protected, void test_B(int var_s[] ) )
      CS_SIGNAL_2(test_B, var_s)

      CS_SLOT_1(Protected, void someProtectedSlot() )
      CS_SLOT_2(someProtectedSlot) 
                            
   private:
      QString somePrivate();  
      double m_amount;      

      QString m_title;        // property 
      Spices m_favorite;      // property 

      bool isScriptTitle() const;

      CS_SLOT_1(Private, void somePrivateSlot(int temp) )
      CS_SLOT_2(somePrivateSlot)
    
      // more testing
      CS_SIGNAL_1(Private, void test_A( Ginger **var_r ) )
      CS_SIGNAL_2(test_A, var_r)
    
      CS_SLOT_1(Private, void test_C(bool b){setTitle("new title");})
      CS_SLOT_2(test_C)
};

Q_DECLARE_METATYPE(Ginger::Spices)

#endif
