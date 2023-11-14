/************************************************************************
*
* Copyright (c) 2014-2023 Barbara Geller & Ansel Sermersheim
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

#include <QCoreApplication>
#include <QByteArray>
#include <QLocale>
#include <QTranslator>

#include <doxy_setup.h>
#include <language.h>
#include <message.h>

static void loadTranslationFile();
static QLocale::Language getEnumLang();

int main(int argc, char **argv)
{
   // passed parameters
   QStringList argList;

   for (int k = 1; k < argc; ++k)   {
      QString value = QString::fromUtf8(argv[k]);
      argList.append(value);
   }

   // loads the project default values
   Config::load_Defaults();

   CommandLine cmdArgs;
   cmdArgs = parseCommandLine(argList);

   // read project file
   readConfiguration(cmdArgs);
   initDoxyPress();

   // set up for translations
   QCoreApplication myApp(argc, argv);
   loadTranslationFile();

   processFiles();
   generateOutput();

   return 0;
}

Translator_Cs translator_Cs;
Translator *theTranslator = &translator_Cs;

void loadTranslationFile()
{
   static const QString qmPath    = ":/resources/translation";
   static QTranslator *translator = new QTranslator;

   QString transFile = getLanguage();

   if (translator->load(transFile, qmPath)) {
      QCoreApplication::installTranslator(translator);

   } else {
      fprintf(stderr, "\nWarning: Problem occurred while loading the '%s' translation file\n\n", csPrintable(transFile) );

   }

   // set locale for things like days of the week, months, etc
   QLocale locale(getEnumLang(), QLocale::system().country());
   QLocale::setDefault(locale);
}

QString getLanguage()
{
   static const QString outputLanguage = Config::getEnum("output-language").toLower();
   QString retval;

   if (outputLanguage == "english") {
      retval = "doxy_en.qm";

   } else if (outputLanguage == "afrikaans") {
      // was za
      retval = "doxy_af.qm";

   } else if (outputLanguage == "arabic") {
      retval = "doxy_ar.qm";

   } else if (outputLanguage == "armenian") {
      // was am
      retval = "doxy_am.qm";

   } else if (outputLanguage == "brazilian") {
      // was br
      retval = "doxy_pt_br.qm";

   } else if (outputLanguage == "catalan") {
      retval = "doxy_ca.qm";

   } else if (outputLanguage == "chinese") {
      // was cn
      retval = "doxy_zh.qm";

   } else if (outputLanguage == "chinese-traditional") {
      // was tw
      retval = "doxy_zh_tw.qm";

   } else if (outputLanguage == "croatian") {
      retval = "doxy_hr.qm";

   } else if (outputLanguage == "czech") {
      retval = "doxy_cz.qm";

   } else if (outputLanguage == "danish") {
      // was dk
      retval = "doxy_da.qm";

   } else if (outputLanguage == "dutch") {
      retval = "doxy_nl.qm";

   } else if (outputLanguage == "esperanto") {
      retval = "doxy_eo.qm";

   } else if (outputLanguage == "finnish") {
      retval = "doxy_fi.qm";

   } else if (outputLanguage == "french") {
      retval = "doxy_fr.qm";

   } else if (outputLanguage == "german") {
      retval = "doxy_de.qm";

   } else if (outputLanguage == "greek") {
      // was gr
      retval = "doxy_el.qm";

   } else if (outputLanguage == "hungarian") {
      retval = "doxy_hu.qm";

   } else if (outputLanguage == "italian") {
      retval = "doxy_it.qm";

   } else if (outputLanguage == "indonesian") {
      retval = "doxy_id.qm";

   } else if (outputLanguage == "japanese") {
      // was jp
      retval = "doxy_ja.qm";

   } else if (outputLanguage == "japanese-en") {
      // was je
      retval = "doxy_ja-en.qm";

   } else if (outputLanguage == "korean") {
      // was kr
      retval = "doxy_ko.qm";

   } else if (outputLanguage == "korean-en") {
      // was ke
      retval = "doxy_ko_en.qm";

   } else if (outputLanguage == "latvian") {
      retval = "doxy_lv.qm";

   } else if (outputLanguage == "lithuanian") {
      retval = "doxy_lt.qm";

   } else if (outputLanguage == "macedonian") {
      retval = "doxy_mk.qm";

   } else if (outputLanguage == "norwegian") {
      // was no
      retval = "doxy_nb.qm";

   } else if (outputLanguage == "persian") {
      retval = "doxy_fa.qm";

   } else if (outputLanguage == "polish") {
      retval = "doxy_pl.qm";

   } else if (outputLanguage == "portuguese") {
      retval = "doxy_pt.qm";

   } else if (outputLanguage == "romanian") {
      retval = "doxy_ro.qm";

   } else if (outputLanguage == "russian") {
      retval = "doxy_ru.qm";

   } else if (outputLanguage == "serbian") {
      retval = "doxy_sr.qm";

   } else if (outputLanguage == "serbian-cyrillic") {
      // was sc
      retval = "doxy_sr_cyrillic.qm";

   } else if (outputLanguage == "slovak") {
      retval = "doxy_sk.qm";

   } else if (outputLanguage == "slovenian") {
      // was si
      retval = "doxy_sl.qm";

   } else if (outputLanguage == "spanish") {
      retval = "doxy_es.qm";

   } else if (outputLanguage == "swedish") {
      retval = "doxy_sv.qm";

   } else if (outputLanguage == "turkish") {
      retval = "doxy_tr.qm";

   } else if (outputLanguage == "ukrainian") {
      // was ua
      retval = "doxy_uk.qm";

   } else if (outputLanguage == "vietnamese") {
      retval = "doxy_vi.qm";

   }

   // no match found
   if (retval.isEmpty()) {
      warn_uncond("Output language %s was not specified, using English\n", csPrintable(outputLanguage));
      retval = "doxy_en.qm";
   }

   return retval;
}

QLocale::Language getEnumLang()
{
   static const QString outputLanguage = Config::getEnum("output-language").toLower();
   QLocale::Language retval = QLocale::Language::English;

   if (outputLanguage == "english") {
      retval = QLocale::Language::English;

   } else if (outputLanguage == "afrikaans") {
      retval = QLocale::Language::Afrikaans;

   } else if (outputLanguage == "arabic") {
      retval = QLocale::Language::Arabic;

   } else if (outputLanguage == "armenian") {
      retval = QLocale::Language::Armenian;

   } else if (outputLanguage == "brazilian") {
      retval = QLocale::Language::Portuguese;

   } else if (outputLanguage == "catalan") {
      retval = QLocale::Language::Catalan;

   } else if (outputLanguage == "chinese") {
      retval = QLocale::Language::Chinese;

   } else if (outputLanguage == "chinese-traditional") {
      retval = QLocale::Language::Chinese;

   } else if (outputLanguage == "croatian") {
      retval = QLocale::Language::Croatian;

   } else if (outputLanguage == "czech") {
      retval = QLocale::Language::Czech;

   } else if (outputLanguage == "danish") {
      retval = QLocale::Language::Danish;

   } else if (outputLanguage == "dutch") {
      retval = QLocale::Language::Dutch;

   } else if (outputLanguage == "esperanto") {
      retval = QLocale::Language::Esperanto;

   } else if (outputLanguage == "finnish") {
      retval = QLocale::Language::Finnish;

   } else if (outputLanguage == "french") {
      retval = QLocale::Language::French;

   } else if (outputLanguage == "german") {
      retval = QLocale::Language::German;

   } else if (outputLanguage == "greek") {
      retval = QLocale::Language::Greek;

   } else if (outputLanguage == "hungarian") {
      retval = QLocale::Language::Hungarian;

   } else if (outputLanguage == "italian") {
      retval = QLocale::Language::Italian;

   } else if (outputLanguage == "indonesian") {
      retval = QLocale::Language::Indonesian;

   } else if (outputLanguage == "japanese") {
      retval = QLocale::Language::Japanese;

   } else if (outputLanguage == "japanese-en") {
      retval = QLocale::Language::Japanese;

   } else if (outputLanguage == "korean") {
      retval = QLocale::Language::Korean;

   } else if (outputLanguage == "korean-en") {
      retval = QLocale::Language::Korean;

   } else if (outputLanguage == "latvian") {
      retval = QLocale::Language::Latvian;

   } else if (outputLanguage == "lithuanian") {
      retval = QLocale::Language::Lithuanian;

   } else if (outputLanguage == "macedonian") {
      retval = QLocale::Language::Macedonian;

   } else if (outputLanguage == "norwegian") {
      retval = QLocale::Language::NorwegianBokmal;

   } else if (outputLanguage == "persian)") {
      retval = QLocale::Language::Persian;

   } else if (outputLanguage == "polish") {
      retval = QLocale::Language::Polish;

   } else if (outputLanguage == "portuguese") {
      retval = QLocale::Language::Portuguese;

   } else if (outputLanguage == "romanian") {
      retval = QLocale::Language::Romanian;

   } else if (outputLanguage == "russian") {
      retval = QLocale::Language::Russian;

   } else if (outputLanguage == "serbian") {
      retval = QLocale::Language::Serbian;

   } else if (outputLanguage == "serbian-cyrillic") {
      retval = QLocale::Language::Serbian;

   } else if (outputLanguage == "slovak") {
      retval = QLocale::Language::Slovak;

   } else if (outputLanguage == "slovenian") {
      retval = QLocale::Language::Slovenian;

   } else if (outputLanguage == "spanish") {
      retval = QLocale::Language::Spanish;

   } else if (outputLanguage == "swedish") {
      retval = QLocale::Language::Swedish;

   } else if (outputLanguage == "turkish") {
      retval = QLocale::Language::Turkish;

   } else if (outputLanguage == "ukrainian") {
      retval = QLocale::Language::Ukrainian;

   } else if (outputLanguage == "vietnamese") {
      retval = QLocale::Language::Vietnamese;

   }

   return retval;
}