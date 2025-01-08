/************************************************************************
*
* Copyright (c) 2014-2025 Barbara Geller & Ansel Sermersheim
*
* Copyright (c) 1997-2014 Dimitri van Heesch
*
* DoxyPress is free software. You can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
* https://www.gnu.org/licenses/
*
*************************************************************************/

#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <translator.h>
#include <translator_cs.h>

// using the CS i18n system
extern Translator_Cs translator_Cs;
extern Translator *theTranslator;      // can be removed when theTranslator is changed

QString getLanguage();

// place holder for a position in a list, used in translators
inline QString generateMarker(int id)
{
   QString retval = QString::number(id);
   retval.prepend('@');

   return retval;
}

#endif
