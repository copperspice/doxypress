/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch. 
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
 * All rights reserved.    
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License version 2
 * is hereby granted. No representations are made about the suitability of
 * this software for any purpose. It is provided "as is" without express or
 * implied warranty. See the GNU General Public License for more details.
 *
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#include <a_define.h>
#include <config.h>

A_Define::A_Define()
{
   fileDef  = 0;
   lineNr   = 1;
   columnNr = 1;
   nargs    = -1;

   undef        = false;
   varArgs      = false;
   isPredefined = false;
   nonRecursive = false;
}

A_Define::A_Define(const A_Define &d)
   : name(d.name), definition(d.definition), fileName(d.fileName)
{
   //name=d.name; definition=d.definition; fileName=d.fileName;
   lineNr   = d.lineNr;
   columnNr = d.columnNr;
   nargs    = d.nargs;
   undef    = d.undef;
   varArgs  = d.varArgs;

   isPredefined = d.isPredefined;
   nonRecursive = d.nonRecursive;
   fileDef      = 0;
}

A_Define::~A_Define()
{
}

bool A_Define::hasDocumentation()
{
   if (! definition.isEmpty() && (! doc.isEmpty() || Config_getBool("EXTRACT_ALL") )) { 
      return true;
   }

   return false;
}
