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

#ifndef TRANSLATOR_TR_H
#define TRANSLATOR_TR_H

class TranslatorTurkish : public TranslatorAdapter_1_7_5
{
 public:

   // --- Language control methods -------------------

   /*! Used for identification of the language. The identification
    * should not be translated. It should be replaced by the name
    * of the language in English using lower-case characters only
    * (e.g. "czech", "japanese", "russian", etc.). It should be equal to
    * the identification used in language.cpp.
    */
   virtual QByteArray idLanguage() {
      return "turkish";
   }

   /*! Used to get the LaTeX command(s) for the language support.
    *  This method should return string with commands that switch
    *  LaTeX to the desired language.  For example
    *  <pre>"\\usepackage[german]{babel}\n"
    *  </pre>
    *  or
    *  <pre>"\\usepackage{polski}\n"
    *  "\\usepackage[latin2]{inputenc}\n"
    *  "\\usepackage[T1]{fontenc}\n"
    *  </pre>
    *
    * The English LaTeX does not use such commands.  Because of this
    * the empty string is returned in this implementation.
    */
   virtual QByteArray latexLanguageSupportCommand() {
      return "";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related fonksiyonlar. */
   virtual QByteArray trRelatedFunctions() {
      return "İlgili Fonksiyonlar";
   }

   /*! subscript for the related fonksiyonlar. */
   virtual QByteArray trRelatedSubscript() {
      return "(Not: Bu fonksiyonlar üye fonksiyon değildir.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Ayrıntılı tanımlama";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Üye Typedef Dokümantasyonu";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Üye Enümerasyon Dokümantasyonu";
   }

   /*! header that is put before the list of member fonksiyonlar. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Üye Fonksiyon Dokümantasyonu";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Alan Dokümantasyonu";
      } else {
         return "Üye Veri Dokümantasyonu";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Ayrıntılar...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Tüm üyelerin listesi";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Üye Listesi";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Şu sınıfın tüm üyelerinin listesidir: ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return "  (kalıtımla gelen üyeleri de içerir).";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Doxygen tarafından";
      if (s) {
         result += s + (QByteArray)" için ";
      }
      result += " kaynak koddan otomatik üretilmiştir.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "enum adı";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "enum değeri";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "tanımlandığı yer";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Modüller";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Sınıf Hiyerarşisi";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Veri Yapıları";
      } else {
         return "Sınıf Listesi";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Dosya Listesi";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Veri Alanları";
      } else {
         return "Sınıf Üyeleri";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Globaller";
      } else {
         return "Dosya Üyeleri";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "İlgili Sayfalar";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Örnekler";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Ara";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Bu kalıtım listesi tamamen olmasa da yaklaşık "
             "olarak alfabetik sıraya konulmuştur:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Bu liste tüm ";
      if (!extractAll) {
         result += "dokümante edilmiş ";
      }
      result += "dosyaları kısa açıklamalarıyla göstermektedir:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Kısa tanımlarıyla birlikte veri yapıları:";
      } else {
         return "Kısa tanımlarıyla sınıflar, yapılar (struct), "
                "birleşimler(union) ve arayüzler:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Bu liste tüm ";
      if (!extractAll) {
         result += "dokümante edilmiş ";
      }
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "yapı(struct) ve birleşim(union) yapılarını içerir";
      } else {
         result += "sınıf üyelerini içerir";
      }
      result += " ve ";
      if (!extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "her alanı yapı(struct)/birleşim(union) dokümantasyonuna bağlar:";
         } else {
            result += "her üye için sınıf dokümantasyonuna bağlar:";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "ait olduğu yapı(struct)/birleşime(union) bağlar:";
         } else {
            result += "ait olduğu sınıfa bağlar:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Bu liste tüm ";
      if (!extractAll) {
         result += "dokümante edilmiş ";
      }

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "fonksiyonları, değişkenleri, makroları, enümerasyonları ve typedef\'leri içerir";
      } else {
         result += "dosya üyelerini içerir";
      }
      result += " ve ";
      if (extractAll) {
         result += "ait olduğu dosyalar bağlar:";
      } else {
         result += "dokümantasyona bağlar:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Tüm örneklerin listesi aşağıdadır:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "İlgili tüm dokümantasyon sayfalarının listesi aşağıdadır:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Tüm modüllerin listesi aşağıdadır:";
   }

   // index titles (the project name is prepended for these)

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Dokümantasyonu";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Modül İndeksi";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Hiyerarşik İndeksi";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Veri Yapıları İndeksi";
      } else {
         return "Sınıf İndeksi";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Dosya İndeksi";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Modül Dokümantasyonu";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Veri Yapıları Dokümantasyonu";
      } else {
         return "Sınıf Dokümantasyonu";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Dosya Dokümantasyonu";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Örnek Dokümantasyonu";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Sayfa Dokümantasyonu";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Referans Kitabı";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Makrolar";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Typedef\'ler";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Enümerasyonlar";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Fonksiyonlar";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Değişkenler";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Enümeratör";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Makro Dokümantasyonu";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Typedef Dokümantasyonu";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Enümerasyon Tipi Dokümantasyonu";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Fonksiyon Dokümantasyonu";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Değişken Dokümantasyonu";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Veri Yapıları";
      } else {
         return "Sınıflar";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = projName + (QByteArray)" için " + date;
      if (projName) {
         result += (QByteArray)" tarihinde ";
      }
      result += (QByteArray)" üreten: ";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return clName + (QByteArray)" için kalıtım şeması:";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "İç kullanıma ayrılmıştır.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Uyarı";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Sürüm";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Tarih";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Döndürdüğü değer";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Ayrıca Bakınız";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Parametreler";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "İstisnalar";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Oluşturan";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Namespace Listesi";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Tüm ";
      if (!extractAll) {
         result += "dokümante edilmiş ";
      }
      result += "namespace\'lerin kısa tanımlarıyla listesi aşağıdadır:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Arkadaşları";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Arkadaş Ve İlgili Fonksiyon Dokümantasyonu";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      switch (compType) {
         case ClassDef::Class:
            result += " Sınıf";
            break;
         case ClassDef::Struct:
            result += " Yapı(Struct)";
            break;
         case ClassDef::Union:
            result += " Birleşim(Union)";
            break;
         case ClassDef::Interface:
            result += " Arayüz(Interface)";
            break;
         case ClassDef::Protocol:
            result += " Protokol";
            break;
         case ClassDef::Category:
            result += " Kategori";
            break;
         case ClassDef::Exception:
            result += " İstisna";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += " Şablon";
      }
      result += " Referans";
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = fileName;
      result += " Dosya Referansı";
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Namespace Referansı";
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Public Üye Fonksiyonlar";
   }
   virtual QByteArray trPublicSlots() {
      return "Public Slotlar";
   }
   virtual QByteArray trSignals() {
      return "Sinyaller";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Statik Public Üye Fonksiyonlar";
   }
   virtual QByteArray trProtectedMembers() {
      return "Korumalı Üye Fonksiyonlar";
   }
   virtual QByteArray trProtectedSlots() {
      return "Korumalı Slotlar";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Statik Korumalı Üye Fonksiyonlar";
   }
   virtual QByteArray trPrivateMembers() {
      return "Özel Üye Fonksiyonlar";
   }
   virtual QByteArray trPrivateSlots() {
      return "Özel Slotlar";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Statik Özel Üye Fonksiyonlar";
   }

   /*! this function is used to produce a comma-separated list of items.
    *  use generateMarker(i) to indicate where item i should be put.
    */
   virtual QByteArray trWriteList(int numEntries) {
      QByteArray result;
      int i;
      // the inherits list contain `numEntries' classes
      for (i = 0; i < numEntries; i++) {
         // use generateMarker to generate placeholders for the class links!
         result += generateMarker(i); // generate marker for entry i in the list
         // (order is left to right)

         if (i != numEntries - 1) { // not the last entry, so we need a separator
            if (i < numEntries - 2) { // not the fore last entry
               result += ", ";
            } else {            // the fore last entry
               result += " ve ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Şu sınıflardan türemiştir : " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Şu sınıfların atasıdır : " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Şu üyeleri yeniden tanımlar : " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Şu üyeler tarafından yeniden tanımlanmıştır : " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Namespace Üyeleri";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Bu liste tüm ";
      if (!extractAll) {
         result += "dokümante edilmiş ";
      }
      result += "namespace üyelerini listeler ve ";
      if (extractAll) {
         result += "her üye için üye dokümantasyonuna bağlar:";
      } else {
         result += "ait olduğu namespace\'lere bağlar:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Namespace İndeksi";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Namespace Dokümantasyonu";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Namespace\'ler";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990728
   //////////////////////////////////////////////////////////////////////////

   /*! This is put at the bottom of a class documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFiles(ClassDef::CompoundType compType,
                                           bool single) {
      // here s is one of " Class", " Struct" or " Union"
      // single is true implies a single file
      QByteArray result = (QByteArray)"Bu ";
      switch (compType) {
         case ClassDef::Class:
            result += "sınıf";
            break;
         case ClassDef::Struct:
            result += "yapı(struct)";
            break;
         case ClassDef::Union:
            result += "birleşim(union)";
            break;
         case ClassDef::Interface:
            result += "arayüz(interface)";
            break;
         case ClassDef::Protocol:
            result += "protokol";
            break;
         case ClassDef::Category:
            result += "kategori";
            break;
         case ClassDef::Exception:
            result += "istisna";
            break;
         default:
            break;
      }
      result += " için dokümantasyon aşağıdaki dosya";
      if (single) {
         result += "";
      } else {
         result += "lar";
      }
      result += "dan üretilmiştir:";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Döndürdüğü değerler";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Ana Sayfa";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "sf.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "@1 dosyasının @0 numaralı satırında tanımlanmıştır.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "@0 dosyasında tanımlanmıştır.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Kullanımdan kalkmıştır";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return clName + (QByteArray)" için işbirliği (collaboration) şeması:";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return fName + (QByteArray)" için içerme bağımlılık grafiği:";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Constructor & Destructor Dokümantasyonu";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Bu dosyanın kaynak koduna git.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Bu dosyanın dokümantasyonuna git.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Ön şart";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Son şart (Postcondition)";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Değişmez(Invariant)";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "İlk değer:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "kod";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Grafiksel Sınıf Hiyerarşisi";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Grafiksel sınıf hiyerarşisine git";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Metinsel sınıf hiyerarşisine git";
   }
   virtual QByteArray trPageIndex() {
      return "Sayfa İndeksi";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Not";
   }
   virtual QByteArray trPublicTypes() {
      return "Public Tipler";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Veri Alanları";
      } else {
         return "Public Özellikler(attribute)";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Statik Public Özellikler(attribute)";
   }
   virtual QByteArray trProtectedTypes() {
      return "Korumalı Tipler";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Korumalı Özellikler(attribute)";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Statik Korumalı Attributes";
   }
   virtual QByteArray trPrivateTypes() {
      return "Özel Tipler";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Özel Özellikler(attribute)";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Statik Özel Özellikler(attribute)";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "Yapılacak";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Yapılacaklar Listesi";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Referans veren";
   }
   virtual QByteArray trRemarks() {
      return "Dipnotlar";
   }
   virtual QByteArray trAttention() {
      return "Dikkat";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Bu şekil hangi dosyaların doğrudan ya da "
             "dolaylı olarak bu dosyayı içerdiğini gösterir:";
   }
   virtual QByteArray trSince() {
      return "Şu zamandan beri";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Şekil Lejantı";
   }
   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Bu dosya doxygen tarafından üretilen grafiklerin nasıl anlaşılacağını "
         "açıklar.<p>\n"
         "Aşağıdaki örneğe bakalım:\n"
         "\\code\n"
         "/*! Sadeleşme yüzünden görünmeyen sınıf */\n"
         "class Invisible { };\n\n"
         "/*! Sadeleşmiş sınıf, kalıtım ilişkisi gizli */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Doxygen yorumlarıyla dokümante edilmemiş sınıf */\n"
         "class Undocumented { };\n\n"
         "/*! Public kalıtımla türetilen sınıf */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! Bir şablon sınıfı */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Korumalı kalıtımla türetilen sınıf */\n"
         "class ProtectedBase { };\n\n"
         "/*! Özel kalıtımla türetilen sınıf */\n"
         "class PrivateBase { };\n\n"
         "/*! Türetilen sınıf tarafından kullanılan sınıf */\n"
         "class Used { };\n\n"
         "/*! Pekçok sınıftan türeyen bir sınıf */\n"
         "class Inherited : public PublicBase,\n"
         "                  protected ProtectedBase,\n"
         "                  private PrivateBase,\n"
         "                  public Undocumented,\n"
         "                  public Templ<int>\n"
         "{\n"
         "  private:\n"
         "    Used *m_usedClass;\n"
         "};\n"
         "\\endcode\n"
         "Bu kod aşağıdaki şemayı oluşturur:"
         "<p><center><img alt=\"\" src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
         "<p>\n"
         "Yukarıdaki şemadaki kutular aşağıda açıklanmıştır:\n"
         "<ul>\n"
         "<li>Dolu gri kutu şemanın ait olduğu yapı(struct) ya da sınıfı "
         "gösterir.\n"
         "<li>Siyah çerçeveli bir kutu dokümante edilmiş bir yapı(struct) ya da sınıfı gösterir.\n"
         "<li>Gri çerçeveli bir kutu dokümante edilmemiş bir yapı(struct) ya da sınıfı gösterir.\n"
         "<li>Kırmızı çerçeveli bir kutu tüm kalıtım/içerme ilişkileri gösterilmemiş ve dokümante "
         "edilmiş bir yapı(struct) ya da sınıfı gösterir. %A şema belirlenen sınırlara "
         "sığmıyorsa sadeleştirilir.\n"
         "</ul>\n"
         "Okların anlamı aşağıdaki gibidir:\n"
         "<ul>\n"
         "<li>Koyu mavi ok iki sınıf arasındaki public kalıtım ilişkisini "
         "göstermekte kullanılır.\n"
         "<li>Koyu yeşil ok korumalı kalıtımı gösterir.\n"
         "<li>Koyu kırmızı ok özel kalıtımı gösterir.\n"
         "<li>Mor kesikli çizgi bir sınıfın diğeri tarafından içeriliyor ya da kullanılıyor "
         "olduğunu gösterir. Ok işaret edilen sınıfın hangi değişken(ler) tarafından erişildiğini "
         "gösteren etiketle işaretleniştir.\n"
         "<li>Sarı kesikli çizgi şablondan üretilen bir sınıf ve ilgili şablon sınıfı "
         "arasındaki ilişkiyi gösterir. Ok türeyen sınıfın şablon parametreleriyle "
         "etiketlenmiştir.\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "lejant";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "Test";
   }
   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "Test Listesi";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Özellikler";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Özellik Dokümantasyonu";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Veri Yapıları";
      } else {
         return "Sınıflar";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Paket " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Paket Listesi";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Paketler ve kısa tanımları (var ise):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Paketler";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Değer:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Hata";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Hata Listesi";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.6
   //////////////////////////////////////////////////////////////////////////

   /*! Used as ansicpg for RTF file
    *
    * The following table shows the correlation of Charset name, Charset Value and
    * <pre>
    * Codepage number:
    * Charset Name       Charset Value(hex)  Codepage number
    * ------------------------------------------------------
    * DEFAULT_CHARSET           1 (x01)
    * SYMBOL_CHARSET            2 (x02)
    * OEM_CHARSET             255 (xFF)
    * ANSI_CHARSET              0 (x00)            1252
    * RUSSIAN_CHARSET         204 (xCC)            1251
    * EE_CHARSET              238 (xEE)            1250
    * GREEK_CHARSET           161 (xA1)            1253
    * TURKISH_CHARSET         162 (xA2)            1254
    * BALTIC_CHARSET          186 (xBA)            1257
    * HEBREW_CHARSET          177 (xB1)            1255
    * ARABIC _CHARSET         178 (xB2)            1256
    * SHIFTJIS_CHARSET        128 (x80)             932
    * HANGEUL_CHARSET         129 (x81)             949
    * GB2313_CHARSET          134 (x86)             936
    * CHINESEBIG5_CHARSET     136 (x88)             950
    * </pre>
    *
    */
   virtual QByteArray trRTFansicp() {
      return "1254";
   }


   /*! Used as ansicpg for RTF fcharset
    *  \see trRTFansicp() for a table of possible values.
    */
   virtual QByteArray trRTFCharSet() {
      return "162";
   }

   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "İndeks";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Sınıf" : "sınıf"));
      if (!singular) {
         result += "lar";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Dosya" : "dosya"));
      if (!singular) {
         result += "lar";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Namespace" : "namespace"));
      if (!singular) {
         result += "\'ler";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Grup" : "grup"));
      if (!singular) {
         result += "lar";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Sayfa" : "sayfa"));
      if (!singular) {
         result += "lar";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Üye" : "üye"));
      if (!singular) {
         result += "ler";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Global değişken" : "global değişken"));
      if (!singular) {
         result += "ler";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Yazar" : "yazar"));
      if (!singular) {
         result += "lar";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Referanslar";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Şu üyeleri gerçekler: " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Şu üyelerce gerçeklenir: " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "İçindekiler";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Kullanımdan Kalkanlar Listesl";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Olaylar";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Olay Dokümantasyonu";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Paket Tipleri";
   }
   /*! Used as a heading for a list of Java class fonksiyonlar with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Paket Fonksiyonlar";
   }
   /*! Used as a heading for a list of static Java class fonksiyonlar with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Static Pakat Fonksiyonları";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Packet Özellikleri(attribute)";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Statik Packet Özellikleri(attribute)";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Tümü";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Bu fonksiyon için çağırılma şeması:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Arama sonuçları";
   }
   /*! This string is put just before listing the search results. The
    *  text can be different depending on the number of documents found.
    *  Inside the text you can put the special marker $num to insert
    *  the number representing the actual number of search results.
    *  The @a numDocuments parameter can be either 0, 1 or 2, where the
    *  value 2 represents 2 or more matches. HTML markup is allowed inside
    *  the returned string.
    */
   virtual QByteArray trSearchResults(int numDocuments) {
      if (numDocuments == 0) {
         return "Üzgünüm, sorgunuzla eşleşen doküman bulunamadı.";
      } else if (numDocuments == 1) {
         return "Sorgunuzla eşleşen <b>1</b> doküman bulundu.";
      } else {
         return "Sorgunuzla eşleşen <b>$num</b> doküman bulundu. "
                "Önce en iyi eşleşmeler gösteriliyor.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Eşleşme:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return filename + " Kaynak Dosyası";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Dizin Hiyerarşisi";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Dizin Dokümantasyonu";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Dizinler";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Bu dizin hiyerarşisi tamamen olmasa da yaklaşık "
             "olarak alfabetik sıraya konulmuştur:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " Dizin Referansı";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Dizin" : "dizin"));
      if (!singular) {
         result += "ler";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.1
   //////////////////////////////////////////////////////////////////////////

   /*! This text is added to the documentation when the \\overload command
    *  is used for a overloaded function.
    */
   virtual QByteArray trOverloadText() {
      return "Bu çok anlamlı (overloaded) bir üye fonksiyondur, "
             "tamamlık açısından burada verilmektedir. Yukarıdaki fonksiyondan "
             "sadece aldığı argüman(lar) açısından farklıdır.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Bu fonksiyon için çağırılma şeması:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Enümerasyon Dokümantasyonu";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Üye Fonksiyon/Subroutine Dokümantasyonu";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Veri Tipleri Listesi";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Veri Alanları";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Kısa tanımlarıyla veri tipleri:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Bu liste tüm ";
      if (!extractAll) {
         result += "dokümante edilmiş ";
      }
      result += "veri tiplerini içerir ve ";
      if (!extractAll) {
         result += "her üyeyi veri yapısı dokümantasyonuna bağlar:";
      } else {
         result += "ait oldukları veri tiplerine bağlar:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Veri Tipi İndeksi";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Veri Tipi Dokümantasyonu";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Fonksiyonlar/Subroutine\'ler";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Fonksiyon/Subroutine Dokümantasyonu";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Veri Tipleri";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Modüller Listesi";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Bu liste tüm ";
      if (!extractAll) {
         result += "dokümante edilmiş ";
      }
      result += "modülleri kısa tanımlarıya içerir:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      switch (compType) {
         case ClassDef::Class:
            result += " Modül";
            break;
         case ClassDef::Struct:
            result += " Tip";
            break;
         case ClassDef::Union:
            result += " Birleşim(Union)";
            break;
         case ClassDef::Interface:
            result += " Arayüz";
            break;
         case ClassDef::Protocol:
            result += " Protokol";
            break;
         case ClassDef::Category:
            result += " Kategori";
            break;
         case ClassDef::Exception:
            result += " İstisna";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += " Şablon";
      }
      result += " Referans";
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Modül Referansı";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Modül Üyeleri";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Bu liste tüm ";
      if (!extractAll) {
         result += "dokümante edilmiş ";
      }
      result += "modül üyelerini içerir ve ";
      if (extractAll) {
         result += "her üyeyi modül dokümantasyonuna bağlar:";
      } else {
         result += "ait oldukları modüllere bağlar:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Modüller Indeksi";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Modül" : "modül"));
      if (!singular) {
         result += "ler";
      }
      return result;
   }
   /*! This is put at the bottom of a module documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFilesFortran(ClassDef::CompoundType compType,
         bool single) {
      // here s is one of " Module", " Struct" or " Union"
      // single is true implies a single file
      QByteArray result = (QByteArray)"Bu ";
      switch (compType) {
         case ClassDef::Class:
            result += "modül";
            break;
         case ClassDef::Struct:
            result += "tip";
            break;
         case ClassDef::Union:
            result += "birleşim(union)";
            break;
         case ClassDef::Interface:
            result += "arayüz";
            break;
         case ClassDef::Protocol:
            result += "protokol";
            break;
         case ClassDef::Category:
            result += "kategori";
            break;
         case ClassDef::Exception:
            result += "istisna";
            break;
         default:
            break;
      }
      result += " için dokümantasyon aşağıdaki dosya";
      if (single) {
         result += ":";
      } else {
         result += "lar";
      }
      result = "dan üretilmiştir:";
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Tip" : "tip"));
      if (!singular) {
         result += "ler";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Alt program" : "alt program"));
      if (!singular) {
         result += "lar";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Tip Sınırlamaları";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return QByteArray(name) + " İlişkisi";
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Yüklüyor...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "En Üst Seviye";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Arıyor...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Eşleşme Yok";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)(name) + (QByteArray)" dizinindeki dosya";
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)(name) + (QByteArray)" dizinindeki dosyayı kapsıyor";
   }
};

#endif
