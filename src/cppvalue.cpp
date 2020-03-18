/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
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

#include <QRegularExpression>
#include <QString>

#include <stdlib.h>

#include <cppvalue.h>
#include <constexp.h>

CPPValue parseOctal()
{
   QString tmp = g_strToken;

   static QRegularExpression regExp("[A-Za-z]");
   tmp.replace(regExp, "");

   long retval = g_strToken.toInteger<long>(nullptr, 8);

   return CPPValue(retval);
}

CPPValue parseDecimal()
{
   QString tmp = g_strToken;

   static QRegularExpression regExp("[A-Za-z]");
   tmp.replace(regExp, "");

   long retval = tmp.toInteger<long>(nullptr, 10);

   return CPPValue(retval);
}

CPPValue parseHexadecimal()
{
   QString tmp = g_strToken;

   static QRegularExpression regExp("[A-Za-z]");
   tmp.replace(regExp, "");

   long retval = g_strToken.toInteger<long>(nullptr, 16);

   return CPPValue(retval);
}

CPPValue parseFloat()
{
   QString tmp = g_strToken;

   static QRegularExpression regExp("[A-Za-z]");
   tmp.replace(regExp, "");

   double retval = g_strToken.toDouble();

   return CPPValue(retval);
}

CPPValue parseCharacter()
{
   // does not work for '\n' and the like

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

         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
            return parseOctal();

         case 'x':
         case 'X':
            return parseHexadecimal();

         default:
            printf("Invalid escape sequence %s found\n", csPrintable(g_strToken));
            return CPPValue(0L);
      }
   }

   return CPPValue(static_cast<long>(g_strToken[1].unicode()));
}

