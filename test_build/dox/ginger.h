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

class Ginger {

   public:
      Ginger();

      enum Brands { Chinese, Jamaican, Thai, Yellow }; 

      using AmountType = double;

      void setAmount(double amt);   
      double getAmount();     

      bool isGrated();
      bool isChopped();

      inline bool isPowder() {
         return false;
      }

      static bool isSpice();

   protected:
      QString someProtected();         
                          
   private:
      QString somePrivate();    

      double m_amount;      
};