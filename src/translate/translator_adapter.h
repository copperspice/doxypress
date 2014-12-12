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

#ifndef TRANSLATOR_ADAPTER_H
#define TRANSLATOR_ADAPTER_H

#include <dox_build_info.h>
#include <translator.h>

/** Base of the translator adapter tree.
 *
 *  This abstract class provides access to the english
 *  translations, to be used as a substitute for not implemented
 *  local translations.
 */
class TranslatorAdapterBase : public Translator
{
 protected:
   virtual ~TranslatorAdapterBase() {}
   TranslatorEnglish english;

   /*! An auxiliary inline method used by the updateNeededMessage()
    *  for building a warning message.
    */
   inline QByteArray createUpdateNeededMessage(const QByteArray &languageName,
         const QByteArray &versionString) {
      return QByteArray("The selected output language \"")
             + languageName
             + "\" has not been updated\nsince "
             + versionString
             + ".  As a result some sentences may appear in English.\n\n";
   }

 public:
   /*! This method is used to generate a warning message to signal
    *  the user that the translation of his/her language of choice
    *  needs updating.  It must be implemented by the translator
    *  adapter class (pure virtual).
    *
    *  \sa createUpdateNeededMessage()
    */
   virtual QByteArray updateNeededMessage() = 0;

};

class TranslatorAdapter_1_8_4 : public TranslatorAdapterBase
{
 public:
   virtual QByteArray updateNeededMessage() {
      return createUpdateNeededMessage(idLanguage(), "release 1.8.4");
   }

   virtual QByteArray trInterfaces() {
      return english.trInterfaces();
   }

   virtual QByteArray trServices() {
      return english.trServices();
   }

   virtual QByteArray trConstantGroups() {
      return english.trConstantGroups();
   }

   virtual QByteArray trConstantGroupReference(const char *namespaceName) {
      return english.trConstantGroupReference(namespaceName);
   }

   virtual QByteArray trServiceReference(const char *sName) {
      return english.trServiceReference(sName);
   }

   virtual QByteArray trSingletonReference(const char *sName) {
      return english.trSingletonReference(sName);
   }

   virtual QByteArray trServiceGeneratedFromFiles(bool single) {
      return english.trServiceGeneratedFromFiles(single);
   }

   virtual QByteArray trSingletonGeneratedFromFiles(bool single) {
      return english.trSingletonGeneratedFromFiles(single);
   }
};

class TranslatorAdapter_1_8_2 : public TranslatorAdapter_1_8_4
{
 public:
   virtual QByteArray updateNeededMessage() {
      return createUpdateNeededMessage(idLanguage(), "release 1.8.2");
   }

   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      return english.trPanelSynchronisationTooltip(enable);
   }

   virtual QByteArray trProvidedByCategory() {
      return english.trProvidedByCategory();
   }

   virtual QByteArray trExtendsClass() {
      return english.trExtendsClass();
   }

   virtual QByteArray trClassMethods() {
      return english.trClassMethods();
   }

   virtual QByteArray trInstanceMethods() {
      return english.trInstanceMethods();
   }

   virtual QByteArray trMethodDocumentation() {
      return english.trMethodDocumentation();
   }

   virtual QByteArray trDesignOverview() {
      return english.trDesignOverview();
   }
};


/** Adapter class for languages that only contain translations up to
 *  version 1.8.0.
 */
class TranslatorAdapter_1_8_0 : public TranslatorAdapter_1_8_2
{
 public:
   virtual QByteArray updateNeededMessage() {
      return createUpdateNeededMessage(idLanguage(), "release 1.8.0");
   }

   virtual QByteArray trDetailLevel() {
      return english.trDetailLevel();
   }

   virtual QByteArray trTemplateParameters() {
      return english.trTemplateParameters();
   }

   virtual QByteArray trAndMore(const QByteArray &number) {
      return english.trAndMore(number);
   }

   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      return english.trEnumGeneratedFromFiles(single);
   }

   virtual QByteArray trEnumReference(const char *name) {
      return english.trEnumReference(name);
   }

   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return english.trInheritedFrom(members, what);
   }

   virtual QByteArray trAdditionalInheritedMembers() {
      return english.trAdditionalInheritedMembers();
   }

};

/** Adapter class for languages that only contain translations up to
 *  version 1.7.5.
 */
class TranslatorAdapter_1_7_5 : public TranslatorAdapter_1_8_0
{
 public:
   virtual QByteArray updateNeededMessage() {
      return createUpdateNeededMessage(idLanguage(), "release 1.7.5");
   }

   virtual QByteArray trCiteReferences() {
      return english.trCiteReferences();
   }

   virtual QByteArray trCopyright() {
      return english.trCopyright();
   }

   virtual QByteArray trDirDepGraph(const char *name) {
      return english.trDirDepGraph(name);
   }
};

/** Adapter class for languages that only contain translations up to
 *  version 1.6.3.
 */
class TranslatorAdapter_1_6_3 : public TranslatorAdapter_1_7_5
{
 public:
   virtual QByteArray updateNeededMessage() {
      return createUpdateNeededMessage(idLanguage(), "release 1.6.3");
   }

   virtual QByteArray trFileIn(const char *name) {
      return english.trFileIn(name);
   }
   virtual QByteArray trIncludesFileIn(const char *name) {
      return english.trIncludesFileIn(name);
   }
   virtual QByteArray trDateTime(int year, int month, int day, int dayOfWeek,
                                 int hour, int minutes, int seconds,
                                 bool includeTime) {
      return english.trDateTime(year, month, day, dayOfWeek, hour, minutes, seconds, includeTime);
   }
};

/** Adapter class for languages that only contain translations up to
 *  version 1.6.0.
 */
class TranslatorAdapter_1_6_0 : public TranslatorAdapter_1_6_3
{
 public:
   virtual QByteArray updateNeededMessage() {
      return createUpdateNeededMessage(idLanguage(), "release 1.6.0");
   }

   virtual QByteArray trDirRelation(const char *name) {
      return english.trDirRelation(name);
   }

   virtual QByteArray trLoading() {
      return english.trLoading();
   }

   virtual QByteArray trGlobalNamespace() {
      return english.trGlobalNamespace();
   }

   virtual QByteArray trSearching() {
      return english.trSearching();
   }

   virtual QByteArray trNoMatches() {
      return english.trNoMatches();
   }
};

/** Adapter class for languages that only contain translations up to
 *  version 1.5.4
 */
class TranslatorAdapter_1_5_4 : public TranslatorAdapter_1_6_0
{
 public:
   virtual QByteArray updateNeededMessage() {
      return createUpdateNeededMessage(idLanguage(), "release 1.5.4");
   }

   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return english.trMemberFunctionDocumentationFortran();
   }

   virtual QByteArray trCompoundListFortran() {
      return english.trCompoundListFortran();
   }

   virtual QByteArray trCompoundMembersFortran() {
      return english.trCompoundMembersFortran();
   }

   virtual QByteArray trCompoundListDescriptionFortran() {
      return english.trCompoundListDescriptionFortran();
   }

   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      return english.trCompoundMembersDescriptionFortran(extractAll);
   }

   virtual QByteArray trCompoundIndexFortran() {
      return english.trCompoundIndexFortran();
   }

   virtual QByteArray trTypeDocumentation() {
      return english.trTypeDocumentation();
   }

   virtual QByteArray trSubprograms() {
      return english.trSubprograms();
   }

   virtual QByteArray trSubprogramDocumentation() {
      return english.trSubprogramDocumentation();
   }

   virtual QByteArray trDataTypes() {
      return english.trDataTypes();
   }

   virtual QByteArray trModulesList() {
      return english.trModulesList();
   }

   virtual QByteArray trModulesListDescription(bool extractAll) {
      return english.trModulesListDescription(extractAll);
   }

   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      return english.trCompoundReferenceFortran(clName, compType, isTemplate);
   }

   virtual QByteArray trModuleReference(const char *namespaceName) {
      return english.trModuleReference(namespaceName);
   }

   virtual QByteArray trModulesMembers() {
      return english.trModulesMembers();
   }

   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      return english.trModulesMemberDescription(extractAll);
   }

   virtual QByteArray trModulesIndex() {
      return english.trModulesIndex();
   }

   virtual QByteArray trModule(bool first_capital, bool singular) {
      return english.trModule(first_capital, singular);
   }

   virtual QByteArray trGeneratedFromFilesFortran(ClassDef::CompoundType compType,
         bool single) {
      return english.trGeneratedFromFilesFortran(compType, single);
   }

   virtual QByteArray trType(bool first_capital, bool singular) {
      return english.trType(first_capital, singular);
   }

   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      return english.trSubprogram(first_capital, singular);
   }

   virtual QByteArray trTypeConstraints() {
      return english.trTypeConstraints();
   }
};

/** Adapter class for languages that only contain translations up to
 *  version 1.4.6
 */
class TranslatorAdapter_1_4_6 : public TranslatorAdapter_1_5_4
{
 public:
   virtual QByteArray updateNeededMessage() {
      return createUpdateNeededMessage(idLanguage(), "release 1.4.6");
   }

   virtual QByteArray trCallerGraph() {
      return english.trCallerGraph();
   }
   virtual QByteArray trEnumerationValueDocumentation() {
      return english.trEnumerationValueDocumentation();
   }
};

#endif

