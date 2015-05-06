/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
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

#include <config.h>
#include <util.h>
#include <language.h>
#include <message.h>
#include <translator.h>
#include <translator_en.h>

#include <translate/translator_adapter.h>

#ifdef LANG_NL
#include "translate/translator_nl.h"
#endif

#ifdef LANG_AM
#include "translate/translator_am.h"
#endif

#ifdef LANG_SV
#include "translate/translator_sv.h"
#endif

#ifdef LANG_CZ
#include "translate/translator_cz.h"
#endif

#ifdef LANG_FR
#include "translate/translator_fr.h"
#endif

#ifdef LANG_ID
#include "translate/translator_id.h"
#endif

#ifdef LANG_IT
#include "translate/translator_it.h"
#endif

#ifdef LANG_DE
#include "translate/translator_de.h"
#endif

#ifdef LANG_JP
#include "translate/translator_jp.h"
#endif

#ifdef LANG_JE
#include "translate/translator_je.h"
#endif

#ifdef LANG_ES
#include "translate/translator_es.h"
#endif

#ifdef LANG_EO
#include "translate/translator_eo.h"
#endif
#ifdef LANG_FI
#include "translate/translator_fi.h"
#endif
#ifdef LANG_RU
#include "translate/translator_ru.h"
#endif
#ifdef LANG_HR
#include "translate/translator_hr.h"
#endif
#ifdef LANG_PL
#include "translate/translator_pl.h"
#endif
#ifdef LANG_PT
#include "translate/translator_pt.h"
#endif
#ifdef LANG_HU
#include "translate/translator_hu.h"
#endif
#ifdef LANG_KE
#include "translate/translator_ke.h"
#endif
#ifdef LANG_KR
#include "translate/translator_kr.h"
#endif
#ifdef LANG_RO
#include "translate/translator_ro.h"
#endif
#ifdef LANG_SI
#include "translate/translator_si.h"
#endif
#ifdef LANG_CN
#include "translate/translator_cn.h"
#endif
#ifdef LANG_TW
#include "translate/translator_tw.h"
#endif
#ifdef LANG_NO
#include "translate/translator_no.h"
#endif
#ifdef LANG_BR
#include "translate/translator_br.h"
#endif
#ifdef LANG_DK
#include "translate/translator_dk.h"
#endif
#ifdef LANG_SK
#include "translate/translator_sk.h"
#endif
#ifdef LANG_UA
#include "translate/translator_ua.h"
#endif
#ifdef LANG_GR
#include "translate/translator_gr.h"
#endif
#ifdef LANG_SR
#include "translate/translator_sr.h"
#endif
#ifdef LANG_CA
#include "translate/translator_ca.h"
#endif

//#ifdef LANG_JS
//#include "translator_js.h"
//#endif

#ifdef LANG_LT
#include "translate/translator_lt.h"
#endif
#ifdef LANG_LV
#include "translate/translator_lv.h"
#endif
#ifdef LANG_ZA
#include "translate/translator_za.h"
#endif
#ifdef LANG_AR
#include "translate/translator_ar.h"
#endif
#ifdef LANG_FA
#include "translate/translator_fa.h"
#endif
#ifdef LANG_MK
#include "translate/translator_mk.h"
#endif
#ifdef LANG_SC
#include "translate/translator_sc.h"
#endif
#ifdef LANG_VI
#include "translate/translator_vi.h"
#endif
#ifdef LANG_TR
#include "translate/translator_tr.h"
#endif

#define L_EQUAL(a) ! langName.compare(a, Qt::CaseInsensitive)

Translator *theTranslator = 0;

bool setTranslator(const QString &langName)
{
   if (L_EQUAL("english")) {
      theTranslator = new TranslatorEnglish;
   }

#ifdef LANG_NL
   else if (L_EQUAL("dutch")) {
      theTranslator = new TranslatorDutch;
   }
#endif

#ifdef LANG_AM
   else if (L_EQUAL("armenian")) {
      theTranslator = new TranslatorArmenian;
   }
#endif

#ifdef LANG_SV
   else if (L_EQUAL("swedish")) {
      theTranslator = new TranslatorSwedish;
   }
#endif

#ifdef LANG_CZ
   else if (L_EQUAL("czech")) {
      theTranslator = new TranslatorCzech;
   }
#endif
#ifdef LANG_FR
   else if (L_EQUAL("french")) {
      theTranslator = new TranslatorFrench;
   }
#endif
#ifdef LANG_ID
   else if (L_EQUAL("indonesian")) {
      theTranslator = new TranslatorIndonesian;
   }
#endif
#ifdef LANG_IT
   else if (L_EQUAL("italian")) {
      theTranslator = new TranslatorItalian;
   }
#endif
#ifdef LANG_DE
   else if (L_EQUAL("german")) {
      theTranslator = new TranslatorGerman;
   }
#endif
#ifdef LANG_JP
   else if (L_EQUAL("japanese")) {
      theTranslator = new TranslatorJapanese;
   }
#endif
#ifdef LANG_JE
   else if (L_EQUAL("japanese-en")) {
      theTranslator = new TranslatorJapaneseEn;
   }
#endif
#ifdef LANG_ES
   else if (L_EQUAL("spanish")) {
      theTranslator = new TranslatorSpanish;
   }
#endif
#ifdef LANG_FI
   else if (L_EQUAL("finnish")) {
      theTranslator = new TranslatorFinnish;
   }
#endif
#ifdef LANG_RU
   else if (L_EQUAL("russian")) {
      theTranslator = new TranslatorRussian;
   }
#endif
#ifdef LANG_HR
   else if (L_EQUAL("croatian")) {
      theTranslator = new TranslatorCroatian;
   }
#endif
#ifdef LANG_PL
   else if (L_EQUAL("polish")) {
      theTranslator = new TranslatorPolish;
   }
#endif
#ifdef LANG_PT
   else if (L_EQUAL("portuguese")) {
      theTranslator = new TranslatorPortuguese;
   }
#endif
#ifdef LANG_HU
   else if (L_EQUAL("hungarian")) {
      theTranslator = new TranslatorHungarian;
   }
#endif
#ifdef LANG_KR
   else if (L_EQUAL("korean")) {
      theTranslator = new TranslatorKorean;
   }
#endif
#ifdef LANG_KE
   else if (L_EQUAL("korean-en")) {
      theTranslator = new TranslatorKoreanEn;
   }
#endif
#ifdef LANG_RO
   else if (L_EQUAL("romanian")) {
      theTranslator = new TranslatorRomanian;
   }
#endif
#ifdef LANG_SI
   else if (L_EQUAL("slovene")) {
      theTranslator = new TranslatorSlovene;
   }
#endif
#ifdef LANG_CN
   else if (L_EQUAL("chinese")) {
      theTranslator = new TranslatorChinese;
   }
#endif
#ifdef LANG_TW
   else if (L_EQUAL("chinese-traditional")) {
      theTranslator = new TranslatorChinesetraditional;
   }
#endif
#ifdef LANG_NO
   else if (L_EQUAL("norwegian")) {
      theTranslator = new TranslatorNorwegian;
   }
#endif
#ifdef LANG_BR
   else if (L_EQUAL("brazilian")) {
      theTranslator = new TranslatorBrazilian;
   }
#endif
#ifdef LANG_DK
   else if (L_EQUAL("danish")) {
      theTranslator = new TranslatorDanish;
   }
#endif
#ifdef LANG_SK
   else if (L_EQUAL("slovak")) {
      theTranslator = new TranslatorSlovak;
   }
#endif
#ifdef LANG_UA
   else if (L_EQUAL("ukrainian")) {
      theTranslator = new TranslatorUkrainian;
   }
#endif
#ifdef LANG_GR
   else if (L_EQUAL("greek")) {
      theTranslator = new TranslatorGreek;
   }
#endif
#ifdef LANG_SR
   else if (L_EQUAL("serbian")) {
      theTranslator = new TranslatorSerbian;
   }
#endif
#ifdef LANG_SC
   else if (L_EQUAL("serbian-cyrillic") || L_EQUAL("serbiancyr")) {
      /* serbiancyr for consistency with older versions */
      theTranslator = new TranslatorSerbianCyrillic;
   }
#endif
#ifdef LANG_CA
   else if (L_EQUAL("catalan")) {
      theTranslator = new TranslatorCatalan;
   }
#endif
#ifdef LANG_LT
   else if (L_EQUAL("lithuanian")) {
      theTranslator = new TranslatorLithuanian;
   }
#endif
#ifdef LANG_LV
   else if (L_EQUAL("latvian")) {
      theTranslator = new TranslatorLatvian;
   }
#endif
#ifdef LANG_ZA
   else if (L_EQUAL("afrikaans")) {
      theTranslator = new TranslatorAfrikaans;
   }
#endif
#ifdef LANG_AR
   else if (L_EQUAL("arabic")) {
      theTranslator = new TranslatorArabic;
   }
#endif
#ifdef LANG_FA
   else if (L_EQUAL("persian") || L_EQUAL("farsi")) {
      theTranslator = new TranslatorPersian;
   }
#endif
#ifdef LANG_MK
   else if (L_EQUAL("macedonian")) {
      theTranslator = new TranslatorMacedonian;
   }
#endif
#ifdef LANG_VI
   else if (L_EQUAL("vietnamese")) {
      theTranslator = new TranslatorVietnamese;
   }
#endif
#ifdef LANG_TR
   else if (L_EQUAL("turkish")) {
      theTranslator = new TranslatorTurkish;
   }
#endif
#ifdef LANG_EO
   else if (L_EQUAL("esperanto")) {
      theTranslator = new TranslatorEsperanto;
   }
#endif

   else {
      theTranslator = new TranslatorEnglish;
      return false;
   }

   QByteArray msg = theTranslator->updateNeededMessage();

   if (! msg.isEmpty()) {
      warn_uncond(msg);
   }
   return true;
}
