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

#include <QCoreApplication>
#include <QByteArray>
#include <QTranslator>

#include <doxy_setup.h>
#include <language.h>
#include <message.h>

int main(int argc, char **argv)
{      
   // passed parameters
   QStringList argList;

   for (int k = 1; k < argc; ++k)   {
      QString value = argv[k];     
      argList.append(value);      
   }
     
   struct CommandLine cmdArgs;
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
      fprintf(stderr, "\n\nProblem occured while loading the %s translation file\n", qPrintable(transFile) );

   }   
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
      retval = "doxy_am.hy";

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
      retval = "doxy_zh-tw.qm";

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

   } else if (outputLanguage == "farsi (persian)") {
      retval = "doxy_fa.qm";   

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
      retval = "doxyp_vi.qm";  
  
   }

   // no match found
   if (retval.isEmpty()) {
      warn_uncond("Output language %s was not specified, using English\n", qPrintable(outputLanguage));      
      retval = "doxy_en.qm";
   } 

   return retval; 
}
 
  
  

 
