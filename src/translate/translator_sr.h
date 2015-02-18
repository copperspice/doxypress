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

#ifndef TRANSLATOR_SR_H
#define TRANSLATOR_SR_H

class TranslatorSerbian : public TranslatorAdapter_1_6_0
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
      return "serbian";
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
      QByteArray result = "\\usepackage[serbian]{babel}\n";
      return result;
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions()
   //! Čini se da je ovako manje loše nego "Povezane funkcije",
   //! što uopšte ne izgleda dobro jer ta kartica sadrži prijatelje i globalne funkcije
   {
      return "Relevantne funkcije";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(To nisu funkcije članice.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Opširniji opis";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Dokumentacija unutrašnjih definicija tipa";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation()
   //! Ovo je u skladu sa "unutrašnja klasa" što se može videti u knjizi.
   {
      return "Dokumentacija unutrašnjih nabrajanja";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Dokumentacija funkcija članica";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Dokumentacija polja";
      } else {
         return "Dokumentacija atributa";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Još...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Spisak svih članova";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Spisak članova";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Ovo je spisak svih članova ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", uključujući nasleđene članove.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Napravljeno automatski korišćenjem alata Doxygen";
      if ( s ) {
         result += (QByteArray)" za projekat " + s;
      }
      result += " od izvornog koda.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "ime nabrajanja ";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "vrednost nabrojane konstante";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "definicija u";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Moduli";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Hijerarhija klasa";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Spisak struktura";
      } else {
         return "Spisak klasa";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Spisak datoteka";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Sva polja struktura";
      } else {
         return "Svi članovi klasa";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      //if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C"))
      //{
      // return "Članovi datoteke"; // TODO: translate me
      //}
      //else
      //{
      return "Članovi datoteke";
      //}
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Stranice koje imaju veze sa ovom stranicom";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Primeri";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Traži";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Hijerahija klasa uređena približno po abecedi:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Spisak svih ";
      if (!extractAll) {
         result += "dokumentovanih ";
      }
      result += "datoteka, sa kratkim opisima:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Spisak struktura sa kratkim opisima:";
      } else {
         return "Spisak klasa, struktura, unija i interfejsa sa kratkim opisima:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Spisak svih ";
      if (!extractAll) {
         result += "dokumentovanih ";
      }
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "članova struktura/unija";
      } else {
         result += "članova klasa";
      }
      result += " sa vezama ka ";
      if (extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "dokumentaciji svakog polja strukture/unije:";
         } else {
            result += "dokumentaciji svakog člana klase:";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "strukturama/unijama kojima pripadaju:";
         } else {
            result += "klasama kojima pripadaju:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Spisak svih ";
      if (!extractAll) {
         result += "dokumentovanih ";
      }

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "funkcija, promenljivih, makro zamena, nabrajanja i definicija tipa";
      } else {
         result += "članova";
      }
      result += " sa vezama ka ";
      if (extractAll) {
         result += "datotekama u kojima se nalaze:";
      } else {
         result += "dokumentaciji:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Spisak svih primera:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Spisak stranica koje imaju veze sa ovom stranicom:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Spisak svih modula:";
   }

   // index titles (the project name is prepended for these)

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Dokumentacija";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Indeks modula";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Hijerarhijski sadržaj";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Spisak struktura/unija";
      } else {
         return "Spisak klasa";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Indeks datoteka";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Dokumentacija modula";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Dokumentacija stuktura/unija";
      } else {
         return "Dokumentacija klasa";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Dokumentacija datoteke";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Dokumentacija primera";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Dokumentacija stranice";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Priručnik";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Makro zamene";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Definicije tipa";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Nabrajanja";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Funkcije";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Promenljive";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Vrednosti nabrojanih konstanti";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Dokumentacija makro zamene";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Dokumentacija definicije tipa";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Dokumentacija nabrajanja";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Dokumentacija funkcije";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Dokumentacija promenljive";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Strukture i unije";
      } else {
         return "Klase, strukture i unije";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"[" + date + "] Napravljeno automatski ";
      if ( projName ) {
         result += (QByteArray)" za projekat " + projName;
      }
      result += (QByteArray)" upotrebom ";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return QByteArray("Dijagram nasleđivanja za klasu ") + clName + ":";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Samo za unutrašnju upotrebu.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Upozorenje";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Verzija";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Datum";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Vrednost funkcije";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Takođe pogledati";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Parametri";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Izuzeci";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Napravio";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Spisak prostora imena";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Spisak svih ";
      if (!extractAll) {
         result += "dokumentovanih ";
      }
      result += "prostora imena sa kratkim opisom:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Prijatelji";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Dokumentacija prijatelja i relevantnih funkcija";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result( "Dokumentacija " );
      switch (compType) {
         case ClassDef::Class:
            result += "klase ";
            break;
         case ClassDef::Struct:
            result += "strukture ";
            break;
         case ClassDef::Union:
            result += "unije ";
            break;
         case ClassDef::Interface:
            result += "interfejsa ";
            break;
         case ClassDef::Protocol:
            result += "protokola ";
            break;
         case ClassDef::Category:
            result += "kategorije ";
            break;
         case ClassDef::Exception:
            result += "izuzetka ";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += "šablona ";
      }
      result += clName;
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = "Opis datoteke ";
      result += fileName;
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = "Opis prostora imena ";
      result += namespaceName;
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Javni članovi";
   }
   virtual QByteArray trPublicSlots() {
      return "Javni slotovi";
   }
   virtual QByteArray trSignals() {
      return "Signali";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Zajednički javni članovi";
   }
   virtual QByteArray trProtectedMembers() {
      return "Zaštićeni članovi";
   }
   virtual QByteArray trProtectedSlots() {
      return "Zaštićeni slotovi";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Zajednički zaštićeni članovi";
   }
   virtual QByteArray trPrivateMembers() {
      return "Privatni članovi";
   }
   virtual QByteArray trPrivateSlots() {
      return "Privatni slotovi";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Zajednički privatni članovi";
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
               result += " i ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int /*numEntries*/) {
      return "Spisak osnovnih klasa: ";
   }

   /*! used in class documentation to produce a list of derived classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int /*numEntries*/) {
      return "Spisak izvedenih klasa: ";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Menja definiciju iz " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      //! Ako već ne možemo jednu reč (redefinicija), da uskladimo sa prethodnim i izbacimo upotrebu roda
      return "Definicija je izmenjena u " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Članovi prostora imena";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Spisak svih ";
      if (!extractAll) {
         result += "dokumentovanih ";
      }
      result += "članova prostora imena sa vezama prema ";
      if (extractAll) {
         result += "dokumentaciji svakog člana prostora imena: ";
      } else {
         result += "prostorima imena kojima pripadaju: ";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Indeks prostora imena";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Dokumentacija prostora imena";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Prostori imena";
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
      QByteArray result = (QByteArray)"Dokumentacija ";
      switch (compType) {
         case ClassDef::Class:
            result += "ove klase";
            break;
         case ClassDef::Struct:
            result += "ove strukture";
            break;
         case ClassDef::Union:
            result += "ove unije";
            break;
         case ClassDef::Interface:
            result += "ovog interfejsa";
            break;
         case ClassDef::Protocol:
            result += "ovog protokola";
            break;
         case ClassDef::Category:
            result += "ove kategorije";
            break;
         case ClassDef::Exception:
            result += "ovog izuzetka";
            break;
         default:
            break;
      }
      result += " je napravljena na osnovu ";
      if (single) {
         result += "datoteke ";
      } else {
         result += "sledećih datoteka:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Karakteristične vrednosti funkcije";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Glavna strana";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "str.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      //! Izbacujemo rod
      return "Definicija je u redu @0 datoteke @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      //! Izbacujemo rod
      return "Definicija je u datoteci @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Zastarelo";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Klasni dijagram za " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Graf zavisnosti datoteka za " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Dokumentacija konstruktora i destruktora";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Izvorni kod.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Dokumentacija.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Preduslovi";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Stanje po izvršenju";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Invarijanta";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Početna vrednost:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "programski kod";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Hijerarhija klasa u obliku grafa";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Prikaz hijerarhije klasa u obliku grafa";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Prikaz hijerarhije klasa u obliku nazubljenog teksta";
   }
   virtual QByteArray trPageIndex() {
      return "Indeks stranice";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Beleška";
   }
   virtual QByteArray trPublicTypes() {
      return "Javni tipovi";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Polja";
      } else {
         return "Javni članovi";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Zajednički javni članovi";
   }
   virtual QByteArray trProtectedTypes() {
      return "Zaštićeni tipovi";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Zaštićeni članovi";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Zajednički zaštićeni članovi";
   }
   virtual QByteArray trPrivateTypes() {
      return "Privatni tipovi";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Privatni članovi";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Zajednički privatni članovi";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "Uraditi";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Spisak stvari koje treba uraditi";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      //! Izbegavanje roda. Uskladjivanje sa trReferences
      return "Korisnici: ";
   }
   virtual QByteArray trRemarks() {
      return "Napomene";
   }
   virtual QByteArray trAttention() {
      return "Pažnja";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Ovaj graf pokazuje koje datoteke direktno "
             "ili indirektno uključuju ovu datoteku: ";
   }
   virtual QByteArray trSince() {
      return "Od";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Objašnjenje korišćenih simbola";
   }
   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Ova stranica objašnjava kako tumačiti grafikone koje je napravio "
         "doxygen.<p>\n"
         "Na primer:\n"
         "\\code\n"
         "/*! Klasa nevidljiva zbog trenutnih ograničenja */\n"
         "class Invisible { };\n\n"
         "/*! Klasa kojoj se ne vidi način izvođenja */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Klasa bez doxygen komentara */\n"
         "class Undocumented { };\n\n"
         "/*! Klasa izvedena iz osnovne klase javnim izvođenjem */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! Šablonska klasa */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Klasa izvedena iz osnovne klase zaštićenim izvođenjem */\n"
         "class ProtectedBase { };\n\n"
         "/*! Klasa izvedena iz osnovne klase privatnim izvođenjem */\n"
         "class PrivateBase { };\n\n"
         "/*! Klasa korišćena u nekoj/nekim od drugih klasa */\n"
         "class Used { };\n\n"
         "/*! Klasa izvedena iz više osnovnih klasa */\n"
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
         "Ako je \\c MAX_DOT_GRAPH_HEIGHT tag u konfiguracionoj datoteci "
         "postavljen na \\c 200 graf izvođenja će izgledati ovako:"
         "<p><center><img src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
         "Graf će biti odsečen ako ne stane unutar zadatih granica.\n"
         "<p>\n"
         "Pravougaonici imaju sledeća značenja:\n"
         "<ul>\n"
         "<li>Puni sivi predstavlja strukturu ili klasu za koju je graf napravljen.\n"
         "<li>Sa crnom ivicom predstavlja dokumentovanu strukturu ili klasu.\n"
         "<li>Sa sivom ivicom predstavlja strukturu ili klasu bez doxygen komentara.\n"
         "<li>Sa crvenom ivicom predstavlja dokumentovanu strukturu ili klasu\n"
         "za koju nisu prikazani svi relevantni grafovi.\n"
         "</ul>"
         "Strelice imaju sledeća značenja:\n"
         "<ul>\n"
         "<li>Tamnoplava strelica označava javno izvođenje.\n"
         "<li>Tamnozelena strelica označava zaštićeno izvođenje.\n"
         "<li>Tamnocrvena strelica označava privatno izvođenje.\n"
         "<li>Ljubičasta isprekidana strelica označava da je klasa sadržana "
         "ili korišćena u drugoj klasi. Strelica je označena imenom atributa "
         "preko koga se pristupa klasi/strukturi na koju pokazuje.\n"
         "<li>Žuta isprekidana strelica označava vezu između primerka šablona i"
         " šablona klase od kojeg je primerak napravljen. "
         "Strelica je označena stvarnim argumentima šablona.\n"
         "</ul>\n"
         ;
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "Objašnjenje korišćenih simbola";
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
      return "Spisak testova";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Osobine";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Dokumentacija osobina";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Strukture i unije";
      } else {
         return "Klase";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Paket " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Spisak paketa";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Paketi s kratkim opisom (ukoliko postoji):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Paketi";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Vrednost:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Greška";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Spisak grešaka";
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
      return "238";
   }

   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "Sadržaj";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result( (first_capital ? "Klas" : "klas") );
      result += (singular ? "a" : "e");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Datotek" : "datotek"));
      result += (singular ? "a" : "e");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Prostor" : "prostor"));
      result += (singular ? "" : "i");
      result += " imena";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Grup" : "grup"));
      result += (singular ? "a" : "e");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Stran" : "stran"));
      result += (singular ? "a" : "e");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Član" : "član"));
      result += (singular ? "" : "ovi");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Globalni " : "globalni "));
      result += (singular ? "podatak" : "podaci");
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Autor" : "autor"));
      result += (singular ? "" : "i");
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Koristi";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int /*numEntries*/) {
      //! "Definiše" je previše kratko, ispada sa de definišu same apstraktne klase
      return "Definiše apstraktnu funkciju deklarisanu u ";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int /*numEntries*/) {
      //! Izbegavanje roda
      return "Definicija u ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
   *  of Contents.
        */
   virtual QByteArray trRTFTableOfContents() {
      return "Sadržaj";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Spisak zastarelih stvari";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Događaji";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Dokumentacija događaja";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Tipovi u paketu";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Funkcije u paketu";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Statičke funkcije u paketu";  // Zajednicke funkcije u paketu
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Atributi u paketu"; // Clanovi u paketu
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Statički atributi u paketu";  // Zajednicki clanovi u paketu
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Sve";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Graf poziva iz ove funkcije:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Rezultati pretraživanja";
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
         return "Nema dokumenata koji odgovaraju Vašem upitu.";
      } else if (numDocuments == 1) {
         return "Nađen je <b>1</b> dokument koji odgovara vašem upitu.";
      } else if (numDocuments < 5) {
         return "Nađena su <b>$num</b> dokumenta koji odgovaraju vašem upitu."
                " Najbolji su prikazani prvi.";
      } else {
         return "Nađeno je <b>$num</b> dokumenata koji odgovaraju vašem upitu."
                " Najbolji su prikazani prvi.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Pronađeno:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return "Izvorni kod datoteke " + filename;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Hijerarhija direktorijuma";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Dokumentacija direktorijuma";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Direktorijumi";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Hijerarhija direktorijuma uređena približno po abecedi:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += "Opis direktorijuma";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Direktorijum" : "direktorijum"));
      if (!singular) {
         result += "i";
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
      return "Ovo je funkcija prekopljenog imena, razlikuje se "
             "od gore navedene samo po argumentima koje prihvata.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      //! Možda je bolje "Graf pozivalaca ove funkcije"
      return "Graf funkcija koje pozivaju ovu funkciju:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Dokumentacija enum vrednosti";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Dokumentacija funkcija i procedura";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   /* TODO: Koji je prevod za Compound u Fortran kontekstu */
   virtual QByteArray trCompoundListFortran() {
      return "Složeni tipovi podataka";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Polja u složenim tipovima podataka";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Spisak složenih tipova podataka sa kratkim opisima:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = " Spisak svih ";
      if (!extractAll) {
         result += "dokumentovanih ";
      }
      result += "polja složenih tipova podataka";
      result += " sa vezama ka ";
      if (!extractAll) {
         result += "dokumentaciji strukture podataka za svakog člana";
      } else {
         result += "složenim tipovima podataka kojima pripadaju:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Sadržaj složenog tipa podataka";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Dokumentacija tipova podataka";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Funkcije i procedure";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Dokumentacija funkcija i procedura";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Složeni tipovi podataka";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Spisak modula";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Spisak svih ";
      if (!extractAll) {
         result += "dokumentovanih ";
      }
      result += "modula sa kratkim opisima:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      switch (compType) {
         case ClassDef::Class:
            result += " Modul";
            break;
         case ClassDef::Struct:
            result += " Tip";
            break;
         case ClassDef::Union:
            result += " Unija";
            break;
         case ClassDef::Interface:
            result += " Interfejs";
            break;
         case ClassDef::Protocol:
            result += " Protokol";
            break;
         case ClassDef::Category:
            result += " Kategorija";
            break;
         case ClassDef::Exception:
            result += " Izuzetak";
            break;
         default:
            break;
      }
      result += " - sažet pregled";
      if (isTemplate) {
         result += " šablona";
      }
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " - sažet pregled modula";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Članovi modula";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Spisak svih ";
      if (!extractAll) {
         result += "dokumentovanih ";
      }
      result += "članova modula sa vezama ka ";
      if (extractAll) {
         result += "dokumentaciji za svakog člana modula:";
      } else {
         result += "modulima kojima pripadaju:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Spisak modula";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Modul" : "modul"));
      if (!singular) {
         result += "i";
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
      QByteArray result = (QByteArray)"Dokumentacija za ovaj ";
      switch (compType) {
         case ClassDef::Class:
            result += "modul";
            break;
         case ClassDef::Struct:
            result += "tip";
            break;
         case ClassDef::Union:
            result += "uniju";
            break;
         case ClassDef::Interface:
            result += "interfejs";
            break;
         case ClassDef::Protocol:
            result += "protokol";
            break;
         case ClassDef::Category:
            result += "kategoriju";
            break;
         case ClassDef::Exception:
            result += "izuzetak";
            break;
         default:
            break;
      }
      result += " napravljena je automatski od sledeć";
      if (single) {
         result += "e datoteke:";
      } else {
         result += "ih datoteka:";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Tip" : "tip"));
      if (!singular) {
         result += "ovi";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Procedura" : "procedura"));
      if (!singular) {
         result = (first_capital ? "Procedure" : "procedure");
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Ograničenja tipova";
   }

   //////////////////////////////////////////////////////////////////////////
   // following methods have no corresponding entry in translator_en.h
   //////////////////////////////////////////////////////////////////////////

   //      /*! This is put above each page as a link to the list of all verbatim headers */
   //     virtual QByteArray trHeaderFiles()
   //     { return "Zaglavlja"; }
   //
   //     /*! This is an introduction to the page with the list of all header files. */
   //     virtual QByteArray trHeaderFilesDescription()
   //     { return "Zaglavlja koje izgraduju API:"; }
   //
   //     /*! This sentences is used in the annotated class/file lists if no brief
   //      * description is given.
   //      */
   //     virtual QByteArray trNoDescriptionAvailable()
   //     { return "Opis nije dostupan"; }
   //
   //     /*! this text is generated when the \\reimp command is used. */
   //     virtual QByteArray trReimplementedForInternalReasons()
   //     { return decode("Preuradeno zbog unutrasnjih razloga; Nema uticaja na API." ); }
   //
   //     /*! this text is generated when the \\bug command is used. */
   //     virtual QByteArray trBugsAndLimitations()
   //     { return "Greske i ogranicenja"; }
   //
   //     virtual QByteArray trSources()
   //     {
   //       return decode("Izvorne datoteke" );
   //     }
   //
   //     /*! Used for Java interfaces in the summary section of Java packages */
   //     virtual QByteArray trInterfaces()
   //     {
   //       return "Interfejsi";  //!< Radna okruzenja. Ali to je dve reci.
   //     }
   //
   //     /*! Used as a chapter title for Latex & RTF output */
   //     virtual QByteArray trPackageDocumentation()
   //     {
   //       return "Dokumentacija paketa";
   //     }
   //
   //     /*! This is used for translation of the word that will possibly
   //      *  be followed by a single name or by a list of names
   //      *  of the category.
   //      */
   //     virtual QByteArray trField(bool first_capital, bool singular)
   //     {
   //       QByteArray result((first_capital ? "Polj" : "polj"));
   //       result+= (singular ? "e" : "a");
   //       return result;
   //     }

};

#endif
