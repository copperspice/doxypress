/*************************************************************************
 *
 * Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim 
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

#include <QRegExp>
#include <QString>

#include <stdlib.h>

#include <cppvalue.h>
#include <constexp.h>

CPPValue parseOctal()
{
  long val = 0;

   QString tmp = g_strToken;

   QRegExp reg("[A-Za-z]");    
   tmp.replace(reg, "");

   val = g_strToken.toLong(nullptr, 8); 
 
   return CPPValue(val);
}

CPPValue parseDecimal()
{
   long val = 0;

   QString tmp = g_strToken;

   QRegExp reg("[A-Za-z]");    
   tmp.replace(reg, "");

   val = tmp.toLong(nullptr, 10);

   return CPPValue(val);
}

CPPValue parseHexadecimal()
{
  long val = 0;

   QString tmp = g_strToken;

   QRegExp reg("[A-Za-z]");    
   tmp.replace(reg, "");

   val = g_strToken.toLong(nullptr, 16);   

   return CPPValue(val);
}

CPPValue parseFloat()
{
   double val = 0;

   QString tmp = g_strToken;

   QRegExp reg("[A-Za-z]");    
   tmp.replace(reg, "");

   val = g_strToken.toDouble();

   return CPPValue(val);
}

CPPValue parseCharacter()
{
   // does not work for '\n' and the alike

   if (g_strToken[1] == '\\') {

      switch (g_strToken[2].unicode()) {
         case 'n':
            return CPPValue((long)'\n');
         case 't':
            return CPPValue((long)'\t');
         case 'v':
            return CPPValue((long)'\v');
         case 'b':
            return CPPValue((long)'\b');
         case 'r':
            return CPPValue((long)'\r');
         case 'f':
            return CPPValue((long)'\f');
         case 'a':
            return CPPValue((long)'\a');
         case '\\':
            return CPPValue((long)'\\');
         case '?':
            return CPPValue((long)'\?');
         case '\'':
            return CPPValue((long)'\'');
         case '"':
            return CPPValue((long)'"');

         case '0':  // fall through
         case '1':  // fall through
         case '2':  // fall through
         case '3':  // fall through
         case '4':  // fall through
         case '5':  // fall through
         case '6':  // fall through
         case '7':  // fall through
            return parseOctal();

         case 'x':
         case 'X':
            return parseHexadecimal();

         default:
            printf("Invalid escape sequence %s found\n", qPrintable(g_strToken));
            return CPPValue(0L);
      }
   }

   return CPPValue(static_cast<long>(g_strToken[1].unicode()));
}

