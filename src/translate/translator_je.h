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
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#ifndef TRANSLATOR_JE_H
#define TRANSLATOR_JE_H

class TranslatorJapaneseEn : public TranslatorEnglish
{
 public:
   virtual QByteArray idLanguage() {
      return "japanese-en";
   }
   virtual QByteArray latexLanguageSupportCommand() {
      return "platex";
   }
   virtual QByteArray trRTFansicp() {
      return "932";
   }

   /*! Used as ansicpg for RTF fcharset
    *  \see trRTFansicp() for a table of possible values.
    */
   virtual QByteArray trRTFCharSet() {
      return "128";
   }
};

#endif
