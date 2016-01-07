/*************************************************************************
*                                                                         
* Copyright (C) 2012-2016 Barbara Geller & Ansel Sermersheim                                                         
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

#ifndef LEMONGRASS_H
#define LEMONGRASS_H

class Lemongrass : public CSObject
{
   public:
      int howMuch;
      QString otherNames;

      /** This is the addHoliday. A bit more information about the method.  */
      void addHoliday(int dayNum, int monthName, int year);

      /// This is the monthName method. 
      void monthName();   
}

#endif