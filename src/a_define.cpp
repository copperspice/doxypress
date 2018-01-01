/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim
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
