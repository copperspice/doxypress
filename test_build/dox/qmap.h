/*************************************************************************
*                                                                         
* Copyright (C) 2012-2016 Barbara Geller & Ansel Sermersheim              
* Copyright (C) 2014 Digia Plc                                            
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

#ifndef QMAP_H
#define QMAP_H

template <class Key, class T>
class QMap
{
   public:
      inline QMap() 
         { }
           
      inline QMap(const QMap<Key, T> &other) 
         { }         
             
      inline ~QMap() 
         { }
};

#endif 