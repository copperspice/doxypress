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

#ifndef TRANSLATOR_ID_H
#define TRANSLATOR_ID_H

/*
 * Translasi berikut didasarkan pada versi translator_en.h dalam
 * Doxygen 1.7.5.1.
 *
 * Penterjemah: Adhi Hargo <cadmus_sw at yahoo.com>
 */
class TranslatorIndonesian : public TranslatorAdapter_1_8_0
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
      return "indonesian";
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
      return "\\usepackage[bahasa]{babel}";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Fungsi-fungsi Terkait";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Perhatikan bahwa fungsi-fungsi tersebut bukan fungsi anggota.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Keterangan Lengkap";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Dokumentasi Anggota: Tipe";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Dokumentasi Anggota: Enumerasi";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Dokumentasi Anggota: Fungsi";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Dokumentasi Variabel";
      } else {
         return "Dokumentasi Anggota: Data";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Selengkapnya...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Daftar semua anggota";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Daftar anggota";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Berikut ini daftar lengkap anggota untuk ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", termasuk semua anggota yang diwariskan.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Dibangkitkan secara otomatis oleh Doxygen";
      if (s) {
         result += (QByteArray)" untuk " + s;
      }
      result += " dari kode sumber.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "nama enumerasi";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "nilai enumerasi";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "didefinisikan di";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Modul";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Hierarki Kelas";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Struktur Data";
      } else {
         return "Daftar Kelas";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Daftar File";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Variabel Data";
      } else {
         return "Daftar Anggota Kelas";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Daftar Definisi Global";
      } else {
         return "Daftar Anggota File";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Halaman Terkait";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Contoh";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Cari";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Hierarki kelas ini diurutkan kurang-lebih"
             " berdasarkan abjad:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Berikut ini daftar seluruh file";
      if (!extractAll) {
         result += " yang didokumentasikan";
      }
      result += ", dengan penjelasan singkat:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Berikut ini daftar struktur data, dengan penjelasan singkat:";
      } else {
         return "Berikut ini daftar kelas, struct, union, dan interface, dengan penjelasan singkat:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Berikut ini daftar seluruh ";
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "variabel anggota struct dan union";
      } else {
         result += "kelas";
      }
      if (!extractAll) {
         result += " yang didokumentasikan";
      }
      result += ", dengan tautan ke ";
      if (!extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "dokumentasi struct/union untuk setiap variabel:";
         } else {
            result += "dokumentasi kelas untuk setiap anggota:";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "struct/union yang memuatnya:";
         } else {
            result += "kelas yang memuatnya:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Berikut ini daftar ";

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "fungsi, variabel, makro definisi, enumerasi, dan tipe";
      } else {
         result += "file";
      }
      if (!extractAll) {
         result += " yang didokumentasikan";
      }
      result += ", dengan tautan ke ";
      if (extractAll) {
         result += "file yang memuatnya:";
      } else {
         result += "dokumentasinya:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Berikut ini daftar semua contoh:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Berikut ini daftar semua halaman dokumentasi yang terkait:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Berikut ini daftar semua modul:";
   }

   // index titles (the project name is prepended for these)

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Dokumentasi";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Indeks Modul";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Indeks Hierarki Kelas";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Indeks Struktur Data";
      } else {
         return "Indeks Kelas";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Indeks File";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Dokumentasi Modul";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Dokumentasi Struktur Data";
      } else {
         return "Dokumentasi Kelas";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Dokumentasi File";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Dokumentasi Contoh";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Dokumentasi Halaman";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Dokumen Referensi";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Makro Definisi";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Definisi Tipe";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Enumerasi";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Fungsi";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Variabel";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Nilai enumerasi";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Dokumentasi Makro Definisi";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Dokumentasi Definisi Tipe";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Dokumentasi Tipe Enumerasi";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Dokumentasi Fungsi";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Dokumentasi Variabel";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Struktur Data";
      } else {
         return "Kelas";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Dibangkitkan pada tanggal " + date;
      if (projName) {
         result += (QByteArray)" untuk " + projName;
      }
      result += (QByteArray)" oleh";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Diagram hierarki kelas untuk " + clName + ":";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Hanya untuk digunakan secara internal.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Peringatan";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Versi";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Tanggal";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Mengembalikan";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Lihat juga";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Parameter";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Eksepsi";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Dibangkitkan oleh";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Daftar Namespace";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Berikut ini daftar namespace";
      if (!extractAll) {
         result += " yang didokumentasikan";
      }
      result += ", dengan keterangan singkat:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Friend";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Dokumentasi Friend Dan Fungsi Terkait";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result = "Referensi";
      switch (compType) {
         case ClassDef::Class:
            result += " Kelas ";
            break;
         case ClassDef::Struct:
            result += " Struct ";
            break;
         case ClassDef::Union:
            result += " Union ";
            break;
         case ClassDef::Interface:
            result += " Interface ";
            break;
         case ClassDef::Protocol:
            result += " Protokol ";
            break;
         case ClassDef::Category:
            result += " Kategori ";
            break;
         case ClassDef::Exception:
            result += " Eksepsi ";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += " Template ";
      }
      result += (QByteArray)clName;
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = "Referensi File ";
      result += fileName;
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = "Referensi Namespace ";
      result += namespaceName;
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Fungsi Anggota Publik";
   }
   virtual QByteArray trPublicSlots() {
      return "Slot Publik";
   }
   virtual QByteArray trSignals() {
      return "Signal";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Fungsi Anggota Publik Statis";
   }
   virtual QByteArray trProtectedMembers() {
      return "Fungsi Anggota Diproteksi";
   }
   virtual QByteArray trProtectedSlots() {
      return "Slot Diproteksi";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Fungsi Anggota Diproteksi Statis";
   }
   virtual QByteArray trPrivateMembers() {
      return "Fungsi Anggota Privat";
   }
   virtual QByteArray trPrivateSlots() {
      return "Slot Privat";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Fungsi Anggota Privat Statis";
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
               result += ", dan ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Mewarisi " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Diwariskan ke " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Di-reimplementasikan dari " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Diimplementasikan ulang di " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Anggota Namespace";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Berikut ini daftar anggota namespace";
      if (!extractAll) {
         result += " yang didokumentasikan";
      }
      result += ", dengan tautan ke ";
      if (extractAll) {
         result += "dokumentasi namespace untuk setiap anggota:";
      } else {
         result += "namespace yang memuatnya:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Indeks Namespace";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Dokumentasi Namespace";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Daftar Namespace";
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
      QByteArray result = (QByteArray)"Dokumentasi untuk ";
      switch (compType) {
         case ClassDef::Class:
            result += "kelas";
            break;
         case ClassDef::Struct:
            result += "struct";
            break;
         case ClassDef::Union:
            result += "union";
            break;
         case ClassDef::Interface:
            result += "interface";
            break;
         case ClassDef::Protocol:
            result += "protokol";
            break;
         case ClassDef::Category:
            result += "kategori";
            break;
         case ClassDef::Exception:
            result += "eksepsi";
            break;
         default:
            break;
      }
      result += " ini dibangkitkan dari file";
      result += single ? "" : "-file";
      result += " berikut:";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Nilai kembali";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Halaman Utama";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "hal.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Definisi pada baris @0 dalam file @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Definisi dalam file @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Kadaluarsa";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Diagram kolaborasi untuk " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Bagan kebergantungan pemuatan untuk " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Dokumentasi Konstruktor & Destruktor";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Ke kode sumber file ini.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Ke dokumentasi file ini.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Kondisi Awal";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Kondisi Akhir";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Invarian";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Nilai awal:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "kode";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Hierarki Kelas Secara Grafis";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Ke bagan grafis hierarki kelas";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Ke bagan tekstual hierarki kelas";
   }
   virtual QByteArray trPageIndex() {
      return "Indeks Halaman";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Catatan";
   }
   virtual QByteArray trPublicTypes() {
      return "Tipe Publik";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Variabel Data";
      } else {
         return "Atribut Publik";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Atribut Publik Statis";
   }
   virtual QByteArray trProtectedTypes() {
      return "Tipe Diproteksi";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Atribut Diproteksi";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Atribut Diproteksi Statis";
   }
   virtual QByteArray trPrivateTypes() {
      return "Tipe Privat";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Atribut Privat";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Atribut Privat Statis";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "Tugas";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Daftar Tugas";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Direferensikan oleh";
   }
   virtual QByteArray trRemarks() {
      return "Keterangan";
   }
   virtual QByteArray trAttention() {
      return "Perhatian";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Bagan ini menunjukkan file-file yang memuat"
             " atau menggunakan file ini baik secara langsung maupun"
             " tidak langsung:";
   }
   virtual QByteArray trSince() {
      return "Sejak";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Keterangan Bagan";
   }
   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Halaman ini berisi keterangan cara membaca bagan yang dibangkitkan "
         "oleh doxygen.<p>\n"
         "Contoh:\n"
         "\\code\n"
         "/*! Invisible class because of truncation */\n"
         "class Invisible { };\n\n"
         "/*! Truncated class, inheritance relation is hidden */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Class not documented with doxygen comments */\n"
         "class Undocumented { };\n\n"
         "/*! Class that is inherited using public inheritance */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! A template class */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Class that is inherited using protected inheritance */\n"
         "class ProtectedBase { };\n\n"
         "/*! Class that is inherited using private inheritance */\n"
         "class PrivateBase { };\n\n"
         "/*! Class that is used by the Inherited class */\n"
         "class Used { };\n\n"
         "/*! Super class that inherits a number of other classes */\n"
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
         "Apabila tag \\c MAX_DOT_GRAPH_HEIGHT di file konfigurasi "
         "diset ke 240 kode di atas akan menghasilkan bagan berikut:"
         "<p><center><img alt=\"\" src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
         "<p>\n"
         "Kotak-kotak pada bagan di atas mempunyai arti sebagai berikut:\n"
         "<ul>\n"
         "<li>%Kotak hitam merupakan struct atau kelas yang bersangkutan.\n"
         "<li>%Kotak berbingkai hitam adalah struct atau kelas yang mempunyai dokumentasi.\n"
         "<li>%Kotak dengan bingkai abu-abu adalah struct atau kelas tanpa dokumentasi.\n"
         "<li>%Kotak dengan bingkai merah merupakan struct atau kelas yang didokumentasikan tetapi"
         "tidak semua relasinya ditampilkan. %Sebuah bagan "
         "akan terpotong apabila lebih besar daripada ukuran yang ditentukan.\n"
         "</ul>\n"
         "Arti dari tanda-tanda panah adalah sebagai berikut:\n"
         "<ul>\n"
         "<li>%Panah biru tua menandakan pewarisan publik.\n"
         "<li>%Panah hijau tua untuk pewarisan diproteksi.\n"
         "<li>%Panah merah tua untuk pewarisan privat.\n"
         "<li>%Panah ungu putus-putus menandakan kelas tersebut berhubungan dengan kelas lain. "
         "Panah tersebut diberi judul sesuai dengan kelas atau struct tujuan.\n"
         "<li>%Panah kuning putus-putus menandakan hubungan antara sebuah template kelas dan "
         "instance dari template tersebut. Panah tersebut diberi judul sesuai dengan "
         "parameter template yang digunakan.\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "keterangan";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "Tes";
   }
   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "Daftar Tes";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Daftar Property";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Dokumentasi Property";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Struktur Data";
      } else {
         return "Daftar Kelas";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Paket " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Daftar Paket";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Berikut ini daftar paket, dengan keterangan singkat (apabila tersedia):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Daftar Paket";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Nilai:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Bug";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Daftar Bug";
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
      return "1252";
   }


   /*! Used as ansicpg for RTF fcharset
    *  \see trRTFansicp() for a table of possible values.
    */
   virtual QByteArray trRTFCharSet() {
      return "0";
   }

   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "Indeks";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Kelas" : "kelas"));
      if (!singular) {
         result += "-kelas";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "File" : "file"));
      if (!singular) {
         result += "-file";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool) {
      QByteArray result((first_capital ? "Namespace" : "namespace"));
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Kelompok" : "kelompok"));
      if (!singular) {
         result += "-kelompok";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Halaman" : "halaman"));
      if (!singular) {
         result += "-halaman";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Anggota" : "anggota"));
      if (!singular) {
         result += "-anggota";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Definisi" : "definisi"));
      if (!singular) {
         result += "-definisi";
      }
      result += " global";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool) {
      QByteArray result((first_capital ? "Penulis" : "penulis"));
      //if (!singular)  result+="s";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Referensi";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Mengimplementasikan " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Diimplementasikan di " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Daftar Isi";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Daftar Kadaluarsa";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Event";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Dokumentasi Event";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Jenis Paket";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Daftar Fungsi Paket";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Daftar Fungsi Statis Paket";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Daftar Atribut Paket";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Daftar Atribut Statis Paket";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Semua";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Berikut ini bagan fungsi-terpanggil untuk fungsi ini:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Hasil Pencarian";
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
         return "Maaf, tidak ada dokumen yang cocok.";
      } else if (numDocuments == 1) {
         return "Ditemukan <b>1</b> dokumen yang cocok.";
      } else {
         return "Ditemukan <b>$num</b> documents yang cocok. "
                "Menampilkan hasil terbaik lebih awal.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Kecocokan:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return "Kode Sumber:" + filename;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Hierarki Direktori";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Dokumentasi Direktori";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Daftar Direktori";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Struktur direktori ini diurutkan hampir berdasarkan abjad:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = "Referensi Direktori ";
      result += dirName;
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool) {
      QByteArray result((first_capital ? "Direktori" : "direktori"));
      //if (singular) result+="y"; else result+="ies";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.1
   //////////////////////////////////////////////////////////////////////////

   /*! This text is added to the documentation when the \\overload command
    *  is used for a overloaded function.
    */
   virtual QByteArray trOverloadText() {
      return "Ini adalah fungsi anggota yang di-overload"
             " untuk kemudahan. Perbedaannya dengan fungsi di atas"
             " hanya pada parameter-parameter yang diterima.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Berikut ini bagan fungsi-pemanggil untuk fungsi ini:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Dokumentasi Nilai Enumerasi";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Dokumentasi Fungsi/Subrutin Anggota";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Daftar Tipe Data";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Variabel Data";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Berikut daftar tipe data, dengan penjelasan singkat:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Berikut daftar semua anggota tipe data ";
      if (!extractAll) {
         result += "terdokumentasi ";
      }
      result += " dengan tautan ke ";
      if (!extractAll) {
         result += "dokumentasi struktur data untuk setiap anggota:";
      } else {
         result += "tipe data yang memuatnya:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Indeks Tipe Data";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Dokumentasi Tipe Data";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Fungsi/Subrutin";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Dokumentasi Fungsi/Subrutin";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Tipe Data";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Modules List";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Berikut daftar semua modul";
      if (!extractAll) {
         result += " terdokumentasi";
      }
      result += ", dengan penjelasan singkat:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = "Referensi ";
      if (isTemplate) {
         result += "Template ";
      }
      switch (compType) {
         case ClassDef::Class:
            result += "Modul ";
            break;
         case ClassDef::Struct:
            result += "Tipe ";
            break;
         case ClassDef::Union:
            result += "Union ";
            break;
         case ClassDef::Interface:
            result += "Antarmuka ";
            break;
         case ClassDef::Protocol:
            result += "Protokol ";
            break;
         case ClassDef::Category:
            result += "Kategori ";
            break;
         case ClassDef::Exception:
            result += "Eksepsi ";
            break;
         default:
            break;
      }
      result += (QByteArray)clName;
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = "Referensi Modul ";
      result += namespaceName;
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Anggota Modul";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Berikut daftar semua anggota modul ";
      if (!extractAll) {
         result += "terdokumentasi ";
      }
      result += "dengan tautan ke ";
      if (extractAll) {
         result += "dokumentasi modul untuk setiap anggota:";
      } else {
         result += "modul yang memuatnya:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Indeks Modul";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Modul" : "modul"));
      if (!singular) {
         result += "-modul";
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
      QByteArray result = (QByteArray)"Dokumentasi untuk ";
      switch (compType) {
         case ClassDef::Class:
            result += "module";
            break;
         case ClassDef::Struct:
            result += "type";
            break;
         case ClassDef::Union:
            result += "union";
            break;
         case ClassDef::Interface:
            result += "interface";
            break;
         case ClassDef::Protocol:
            result += "protocol";
            break;
         case ClassDef::Category:
            result += "category";
            break;
         case ClassDef::Exception:
            result += "exception";
            break;
         default:
            break;
      }
      result += " ini dibangkitkan dari file";
      if (!single) {
         result += "-file ";
      }
      result += "berikut:";
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Tipe" : "tipe"));
      if (!singular) {
         result += "-tipe";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Subprogram" : "subprogram"));
      if (!singular) {
         result += "-subprogram";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Batasan Tipe";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return "Relasi " + QByteArray(name);
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Memuat...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Namespace Global";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Mencari...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Tidak Ditemukan";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"File dimuat dalam " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Memuat file dalam " + name;
   }
   
   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Daftar Pustaka";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Hak Cipta";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Bagan dependensi directori untuk ") + name + ":";
   }

};

#endif
