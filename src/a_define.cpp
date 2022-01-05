/************************************************************************
*
* Copyright (c) 2014-2022 Barbara Geller & Ansel Sermersheim
* Copyright (c) 1997-2014 Dimitri van Heesch
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

#include <a_define.h>
#include <config.h>

A_Define::A_Define()
{  
   lineNr       = 1;
   columnNr     = 1;
   nargs        = -1;

   undef        = false;
   varArgs      = false;
   isPredefined = false;
   nonRecursive = false;
}

bool A_Define::hasDocumentation()
{
   static const bool extractAll = Config::getBool("extract-all");

   if (! m_definition.isEmpty() && (! doc.isEmpty() || extractAll )) { 
      return true;
   }

   return false;
}
