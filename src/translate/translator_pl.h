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

#ifndef TRANSLATOR_PL_H
#define TRANSLATOR_PL_H

class TranslatorPolish : public TranslatorAdapter_1_8_2
{
 public:

   // --- Language control methods -------------------

   /*! Used for identification of the language. May resemble
    * the string returned by latexBabelPackage(), but it is not used
    * for the same purpose. The identification should not be translated.
    * It should be replaced by the name of the language in English
    * (e.g. Czech, Japanese, Russian, etc.). It should be equal to
    * the identification in language.h.
    */
   QByteArray idLanguage() {
      return "polish";
   }
   /*! Used to get the command(s) for the language support. This method
    *  was designed for languages which do not prefer babel package.
    *  If this methods returns empty string, then the latexBabelPackage()
    *  method is used to generate the command for using the babel package.
    */
   QByteArray latexLanguageSupportCommand() {
      return "\\usepackage{polski}\n"
             "\\usepackage[T1]{fontenc}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   QByteArray trRelatedFunctions() {
      return "Funkcje powiązane";
   }

   /*! subscript for the related functions. */
   QByteArray trRelatedSubscript() {
      return "(Zauważ, że to nie są metody klas.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   QByteArray trDetailedDescription() {
      return "Opis szczegółowy";
   }

   /*! header that is put before the list of typedefs. */
   QByteArray trMemberTypedefDocumentation() {
      return "Dokumentacja składowych definicji typu";
   }

   /*! header that is put before the list of enumerations. */
   QByteArray trMemberEnumerationDocumentation() {
      return "Dokumentacja składowych wyliczanych";
   }

   /*! header that is put before the list of member functions. */
   QByteArray trMemberFunctionDocumentation() {
      return "Dokumentacja funkcji składowych";
   }

   /*! header that is put before the list of member attributes. */
   QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Dokumentacja pól";
      } else {
         return "Dokumentacja atrybutów składowych";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   QByteArray trMore() {
      return "Więcej...";
   }

   /*! put in the class documentation */
   QByteArray trListOfAllMembers() {
      return "Lista wszystkich składowych";
   }

   /*! used as the title of the "list of all members" page of a class */
   QByteArray trMemberList() {
      return "Lista składowych";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   QByteArray trThisIsTheListOfAllMembers() {
      return "To jest kompletna lista składowych dla ";
   }

   /*! this is the remainder of the sentence after the class name */
   QByteArray trIncludingInheritedMembers() {
      return ", uwzględniająca wszystkie dziedziczone składowe.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Wygenerowano automatycznie z kodu źródłowego programem Doxygen";
      if (s) {
         result += (QByteArray)" dla " + s;
      }
      result += ".";
      return result;
   }

   /*! put after an enum name in the list of all members */
   QByteArray trEnumName() {
      return "nazwa wyliczenia";
   }

   /*! put after an enum value in the list of all members */
   QByteArray trEnumValue() {
      return "wartość wyliczenia";
   }

   /*! put after an undocumented member in the list of all members */
   QByteArray trDefinedIn() {
      return "zdefiniowana w";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   QByteArray trModules() {
      return "Moduły";
   }

   /*! This is put above each page as a link to the class hierarchy */
   QByteArray trClassHierarchy() {
      return "Hierarchia klas";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Struktury danych";
      } else {
         return "Lista klas";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   QByteArray trFileList() {
      return "Lista plików";
   }

   /*! This is put above each page as a link to the list of all verbatim headers */
   QByteArray trHeaderFiles() {
      return "Pliki nagłówkowe";
   }

   /*! This is put above each page as a link to all members of compounds. */
   QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Pola danych";
      } else {
         return "Składowe klas";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Globalne";
      } else {
         return "Składowe plików";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   QByteArray trRelatedPages() {
      return "Dodatkowe strony";
   }

   /*! This is put above each page as a link to all examples. */
   QByteArray trExamples() {
      return "Przykłady";
   }

   /*! This is put above each page as a link to the search engine. */
   QByteArray trSearch() {
      return "Szukaj";
   }

   /*! This is an introduction to the class hierarchy. */
   QByteArray trClassHierarchyDescription() {
      return "Ta lista dziedziczenia posortowana jest z grubsza, "
             "choć nie całkowicie, alfabetycznie:";
   }

   /*! This is an introduction to the list with all files. */
   QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Tutaj znajduje się lista wszystkich ";
      if (!extractAll) {
         result += "udokumentowanych ";
      }
      result += "plików z ich krótkimi opisami:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   QByteArray trCompoundListDescription() {

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Tutaj znajdują się struktury danych wraz z ich krótkimi opisami:";
      } else {
         return "Tutaj znajdują się klasy, struktury, "
                "unie i interfejsy wraz z ich krótkimi opisami:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Tutaj znajduje się lista wszystkich ";
      if (!extractAll) {
         result += "udokumentowanych ";
      }
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "pól struktur i unii";
      } else {
         result += "składowych";
      }
      result += " wraz z odnośnikami do ";
      if (!extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "dokumentacji struktur/unii dla każdego pola:";
         } else {
            result += "dokumentacji klas dla każdej składowej:";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "struktur/unii, do których dane pole należy:";
         } else {
            result += "klas, do których dana składowa należy:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Tutaj znajduje się lista wszystkich ";
      if (!extractAll) {
         result += "udokumentowanych ";
      }
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "funkcji, zmiennych, makr, wyliczeń i definicji typów";
      } else {
         result += "składowych plików";
      }
      result += " wraz z odnośnikami do ";
      if (extractAll) {
         result += "plików, do których one należą:";
      } else {
         result += "dokumentacji:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all header files. */
   QByteArray trHeaderFilesDescription() {
      return "Tutaj znajdują się pliki nagłówkowe tworzące API:";
   }

   /*! This is an introduction to the page with the list of all examples */
   QByteArray trExamplesDescription() {
      return "Tutaj znajduje się lista wszystkich przykładów:";
   }

   /*! This is an introduction to the page with the list of related pages */
   QByteArray trRelatedPagesDescription() {
      return "Tutaj znajduje się lista wszystkich stron dokumentacji:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   QByteArray trModulesDescription() {
      return "Tutaj znajduje się lista wszystkich grup:";
   }

   /*! This sentences is used in the annotated class/file lists if no brief
    * description is given.
    */
   QByteArray trNoDescriptionAvailable() {
      return "Brak opisu";
   }

   // index titles (the project name is prepended for these)


   /*! This is used in HTML as the title of index.html. */
   QByteArray trDocumentation() {
      return "Dokumentacja";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   QByteArray trModuleIndex() {
      return "Indeks grup";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   QByteArray trHierarchicalIndex() {
      return "Indeks hierarchiczny";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Indeks struktur danych";
      } else {
         return "Indeks klas";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   QByteArray trFileIndex() {
      return "Indeks plików";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   QByteArray trModuleDocumentation() {
      return "Dokumentacja grup";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Dokumentacja struktur danych";
      } else {
         return "Dokumentacja klas";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   QByteArray trFileDocumentation() {
      return "Dokumentacja plików";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   QByteArray trExampleDocumentation() {
      return "Dokumentacja przykładów";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   QByteArray trPageDocumentation() {
      return "Dokumentacja stron";
   }

   /*! This is used in LaTeX as the title of the document */
   QByteArray trReferenceManual() {
      return "Podręcznik";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   QByteArray trDefines() {
      return "Definicje";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   QByteArray trTypedefs() {
      return "Definicje typów";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   QByteArray trEnumerations() {
      return "Wyliczenia";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   QByteArray trFunctions() {
      return "Funkcje";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   QByteArray trVariables() {
      return "Zmienne";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   QByteArray trEnumerationValues() {
      return "Wartości wyliczeń";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   QByteArray trDefineDocumentation() {
      return "Dokumentacja definicji";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   QByteArray trTypedefDocumentation() {
      return "Dokumentacja definicji typów";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   QByteArray trEnumerationTypeDocumentation() {
      return "Dokumentacja typów wyliczanych";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   QByteArray trFunctionDocumentation() {
      return "Dokumentacja funkcji";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   QByteArray trVariableDocumentation() {
      return "Dokumentacja zmiennych";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Struktury danych";
      } else {
         return "Komponenty";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Wygenerowano " + date;
      if (projName) {
         result += (QByteArray)" dla " + projName;
      }
      result += (QByteArray)" programem";
      return result;
   }

   /*! this text is put before a class diagram */
   QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Diagram dziedziczenia dla " + clName;
   }

   /*! this text is generated when the \\internal command is used. */
   QByteArray trForInternalUseOnly() {
      return "Tylko do użytku wewnętrznego.";
   }

   /*! this text is generated when the \\reimp command is used. */
   QByteArray trReimplementedForInternalReasons() {
      return "Reimplementowana z wewnętrzych przyczyn; nie dotyczy API.";
   }

   /*! this text is generated when the \\warning command is used. */
   QByteArray trWarning() {
      return "Ostrzeżenie";
   }

   /*! this text is generated when the \\bug command is used. */
   QByteArray trBugsAndLimitations() {
      return "Błędy i ograniczenia";
   }

   /*! this text is generated when the \\version command is used. */
   QByteArray trVersion() {
      return "Wersja";
   }

   /*! this text is generated when the \\date command is used. */
   QByteArray trDate() {
      return "Data";
   }

   /*! this text is generated when the \\return command is used. */
   QByteArray trReturns() {
      return "Zwraca";
   }

   /*! this text is generated when the \\sa command is used. */
   QByteArray trSeeAlso() {
      return "Zobacz również";
   }

   /*! this text is generated when the \\param command is used. */
   QByteArray trParameters() {
      return "Parametry";
   }

   /*! this text is generated when the \\exception command is used. */
   QByteArray trExceptions() {
      return "Wyjątki";
   }

   /*! this text is used in the title page of a LaTeX document. */
   QByteArray trGeneratedBy() {
      return "Wygenerowano przez";
   }

   // new since 0.49-990307

   /*! used as the title of page containing all the index of all namespaces. */
   QByteArray trNamespaceList() {
      return "Lista przestrzeni nazw";
   }

   /*! used as an introduction to the namespace list */
   QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Tutaj znajdują się wszystkie ";
      if (!extractAll) {
         result += "udokumentowane ";
      }
      result += "przestrzenie nazw wraz z ich krótkimi opisami:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   QByteArray trFriends() {
      return "Przyjaciele";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   QByteArray trRelatedFunctionDocumentation() {
      return "Dokumentacja przyjaciół i funkcji związanych";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   QByteArray trCompoundReference(const char *clName,
                                  ClassDef::CompoundType compType,
                                  bool isTemplate) {
      QByteArray result = "Dokumentacja";
      if (isTemplate) {
         result += " szablonu";
      }
      switch (compType) {
         case ClassDef::Class:
            result += " klasy ";
            break;
         case ClassDef::Struct:
            result += " struktury ";
            break;
         case ClassDef::Union:
            result += " unii ";
            break;
         case ClassDef::Interface:
            result += " interfejsu ";
            break;
         case ClassDef::Protocol:
            result += " protokołu ";
            break;
         case ClassDef::Category:
            result += " kategorii ";
            break;
         case ClassDef::Exception:
            result += " wyjątku ";
            break;
         default:
            break;
      }
      result += (QByteArray)clName;
      return result;
   }

   /*! used as the title of the HTML page of a file */
   QByteArray trFileReference(const char *fileName) {
      QByteArray result = "Dokumentacja pliku ";
      result += fileName;
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = "Dokumentacja przestrzeni nazw ";
      result += namespaceName;
      return result;
   }

   /* these are for the member sections of a class, struct or union */
   QByteArray trPublicMembers() {
      return "Metody publiczne";
   }
   QByteArray trPublicSlots() {
      return "Sloty publiczne";
   }
   QByteArray trSignals() {
      return "Sygnały";
   }
   QByteArray trStaticPublicMembers() {
      return "Statyczne metody publiczne";
   }
   QByteArray trProtectedMembers() {
      return "Metody chronione";
   }
   QByteArray trProtectedSlots() {
      return "Sloty chronione";
   }
   QByteArray trStaticProtectedMembers() {
      return "Statyczne metody chronione";
   }
   QByteArray trPrivateMembers() {
      return "Metody prywatne";
   }
   QByteArray trPrivateSlots() {
      return "Sloty prywatne";
   }
   QByteArray trStaticPrivateMembers() {
      return "Statyczne metody prywatne";
   }

   /*! this function is used to produce a comma-separated list of items.
    *  use generateMarker(i) to indicate where item i should be put.
    */
   QByteArray trWriteList(int numEntries) {
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
   QByteArray trInheritsList(int numEntries) {
      return "Dziedziczy " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   QByteArray trInheritedByList(int numEntries) {
      return "Dziedziczona przez " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   QByteArray trReimplementedFromList(int numEntries) {
      return "Reimplementowana z " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   QByteArray trReimplementedInList(int numEntries) {
      return "Reimplementowana w " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   QByteArray trNamespaceMembers() {
      return "Składowe przestrzeni nazw";
   }

   /*! This is an introduction to the page with all namespace members */
   QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Tutaj znajduje się lista wszystkich ";
      if (!extractAll) {
         result += "udokumentowanych ";
      }
      result += "składowych wraz z odnośnikami do ";
      if (extractAll) {
         result += "dokumentacji przestrzeni nazw dla każdej składowej:";
      } else {
         result += "przestrzeni nazw do których składowe te należą:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   QByteArray trNamespaceIndex() {
      return "Indeks przestrzeni nazw";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   QByteArray trNamespaceDocumentation() {
      return "Dokumentacja przestrzeni nazw";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   QByteArray trNamespaces() {
      return "Przestrzenie nazw";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990728
   //////////////////////////////////////////////////////////////////////////

   /*! This is put at the bottom of a class documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   QByteArray trGeneratedFromFiles(ClassDef::CompoundType compType,
                                   bool single) {
      // here s is one of " Class", " Struct" or " Union"
      // single is true implies a single file
      QByteArray result = (QByteArray)"Dokumentacja dla te";
      switch (compType) {
         case ClassDef::Class:
            result += "j klasy";
            break;
         case ClassDef::Struct:
            result += "j struktury";
            break;
         case ClassDef::Union:
            result += "j unii";
            break;
         case ClassDef::Interface:
            result += "go interfejsu";
            break;
         case ClassDef::Protocol:
            result += "go protokołu";
            break;
         case ClassDef::Category:
            result += "j kategorii";
            break;
         case ClassDef::Exception:
            result += "go wyjątku";
            break;
         default:
            break;
      }
      result += " została wygenerowana z plik";
      if (single) {
         result += "u:";
      } else {
         result += "ów:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   QByteArray trReturnValues() {
      return "Zwracane wartości";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   QByteArray trMainPage() {
      return "Strona główna";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   QByteArray trPageAbbreviation() {
      return "str.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   QByteArray trSources() {
      return "Źródła";
   }
   QByteArray trDefinedAtLineInSourceFile() {
      return "Definicja w linii @0 pliku @1.";
   }
   QByteArray trDefinedInSourceFile() {
      return "Definicja w pliku @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   QByteArray trDeprecated() {
      return "Do wycofania";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Diagram współpracy dla " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Wykres zależności załączania dla " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   QByteArray trConstructorDocumentation() {
      return "Dokumentacja konstruktora i destruktora";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   QByteArray trGotoSourceCode() {
      return "Idź do kodu źródłowego tego pliku.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   QByteArray trGotoDocumentation() {
      return "Idź do dokumentacji tego pliku.";
   }
   /*! Text for the \\pre command */
   QByteArray trPrecondition() {
      return "Warunek wstępny";
   }
   /*! Text for the \\post command */
   QByteArray trPostcondition() {
      return "Warunek końcowy";
   }
   /*! Text for the \\invariant command */
   QByteArray trInvariant() {
      return "Niezmiennik";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   QByteArray trInitialValue() {
      return "Wartość początkowa:";
   }
   /*! Text used the source code in the file index */
   QByteArray trCode() {
      return "kod źródłowy";
   }
   QByteArray trGraphicalHierarchy() {
      return "Graficzna hierarchia klas";
   }
   QByteArray trGotoGraphicalHierarchy() {
      return "Idź do graficznej hierarchi klas";
   }
   QByteArray trGotoTextualHierarchy() {
      return "Idź do tekstowej hierarchi klas";
   }
   QByteArray trPageIndex() {
      return "Indeks stron";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   QByteArray trNote() {
      return "Nota";
   }
   QByteArray trPublicTypes() {
      return "Typy publiczne";
   }
   QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Pola danych";
      } else {
         return "Atrybuty publiczne";
      }
   }
   QByteArray trStaticPublicAttribs() {
      return "Statyczne atrybuty publiczne";
   }
   QByteArray trProtectedTypes() {
      return "Typy chronione";
   }
   QByteArray trProtectedAttribs() {
      return "Atrybuty chronione";
   }
   QByteArray trStaticProtectedAttribs() {
      return "Statyczne atrybuty chronione";
   }
   QByteArray trPrivateTypes() {
      return "Typy prywatne";
   }
   QByteArray trPrivateAttribs() {
      return "Atrybuty prywatne";
   }
   QByteArray trStaticPrivateAttribs() {
      return "Statyczne atrybuty prywatne";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a todo item */
   QByteArray trTodo() {
      return "Do zrobienia";
   }
   /*! Used as the header of the todo list */
   QByteArray trTodoList() {
      return "Lista rzeczy do zrobienia";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   QByteArray trReferencedBy() {
      return "Odwołania w";
   }
   QByteArray trRemarks() {
      return "Spostrzeżenia";
   }
   QByteArray trAttention() {
      return "Uwaga";
   }
   QByteArray trInclByDepGraph() {
      return "Ten wykres pokazuje, które pliki bezpośrednio lub "
             "pośrednio załączają ten plik:";
   }
   QByteArray trSince() {
      return "Od";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   QByteArray trLegendTitle() {
      return "Legenda wykresu";
   }
   /*! page explaining how the dot graph's should be interpreted */
   QByteArray trLegendDocs() {
      return
         "Ta strona wyjaśnia jak interpretować wykresy, które są wygenerowane "
         "przez doxygen.<p>\n"
         "Rozważ następujący przykład:\n"
         "\\code\n"
         "/*! Klasa Niewidzialna z powodu okrojenia */\n"
         "class Niewidzialna { };\n\n"
         "/*! Klasa Okrojona, relacja dziedziczenia jest ukryta */\n"
         "class Okrojona : public Niewidzialna { };\n\n"
         "/* Klasa nie udokumentowana komentarzami doxygen */\n"
         "class Nieudokumentowana { };\n\n"
         "/*! Klasa, która jest dziedziczona publicznie */\n"
         "class PublicznaBaza : public Okrojona { };\n\n"
         "/*! A template class */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Klasa, która jest dziedziczona przy użyciu dziedziczenia chronionego */\n"
         "class ChronionaBaza { };\n\n"
         "/*! Klasa, która jest dziedziczona prywatnie */\n"
         "class PrywatnaBaza { };\n\n"
         "/*! Klasa, która jest użyta przez klasę Dziedziczona */\n"
         "class Uzyta { };\n\n"
         "/*! Superklasa, która dziedziczy kilka innych klas */\n"
         "class Dziedziczona : public PublicznaBaza,\n"
         "                     protected ChronionaBaza,\n"
         "                     private PrywatnaBaza,\n"
         "                     public Nieudokumentowana,\n"
         "                     public Templ<int>\n"
         "{\n"
         "  private:\n"
         "    Uzyta *m_usedClass;\n"
         "};\n"
         "\\endcode\n"
         "Rezultat na następującym wykresie:"
         "<p><center><img src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center></p>\n"
         "<p>\n"
         "Prostokąty w powyższym wykresie mają następujące znaczenie:\n"
         "</p>\n"
         "<ul>\n"
         "<li>Wypełniony czarny prostokąt reprezentuje strukturę lub klasę dla "
         "której został wygenerowany wykres.</li>\n"
         "<li>Prostokąt z czarną obwolutą oznacza udokumentowaną strukturę lub klasę.</li>\n"
         "<li>Prostokąt z szarą obwolutą oznacza nieudokumentowaną strukturę lub klasę.</li>\n"
         "<li>Prostokąt z czerwoną obwolutą oznacza udokumentowaną strukturę lub klasę dla\n"
         "której nie są pokazane wszystkie relacje dziedziczenia/zawierania. Wykres jest "
         "okrojony, jeśli nie mieści się w określonych brzegach.</li>\n"
         "</ul>\n"
         "<p>\n"
         "Strzałki mają następujące znaczenie:\n"
         "<p>\n"
         "<ul>\n"
         "<li>Ciemno niebieska strzałka jest używana do wizualizacji relacji "
         "dziedziczenia publicznego pomiędzy dwiema klasami.</li>\n"
         "<li>Ciemno zielona strzałka jest używana dla dziedziczenia chronionego.</li>\n"
         "<li>Ciemno czerwona strzałka jest używana dla dziedziczenia prywatnego.</li>\n"
         "<li>Fioletowa przerywana strzałka jest używana jeśli klasa jest zawarta "
         "lub użyta przez inną klasę. Strzałka jest podpisana zmienną(ymi) "
         "przez które wskazywana klasa lub struktura jest dostępna. </li>\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   QByteArray trLegend() {
      return "legenda";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   QByteArray trTest() {
      return "Test";
   }
   /*! Used as the header of the test list */
   QByteArray trTestList() {
      return "Lista testu";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Właściwości";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Dokumentacja właściwości";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java interfaces in the summary section of Java packages */
   virtual QByteArray trInterfaces() {
      return "Interfejsy";
   }
   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Struktury Danych";
      } else {
         return "Klasy";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Pakiet " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Lista Pakietów";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Oto lista pakietów wraz z krótkim opisem (o ile jest dostępny):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Pakiety";
   }
   /*! Used as a chapter title for Latex & RTF output */
   virtual QByteArray trPackageDocumentation() {
      return "Dokumentacja Pakietu";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Wartość:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Błąd";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Lista błędów";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.6-20010422
   //////////////////////////////////////////////////////////////////////////

   /*! Used as ansicpg for RTF file */
   virtual QByteArray trRTFansicp() {
      return "1250";
   }

   /*! Used as ansicpg for RTF fcharset */
   virtual QByteArray trRTFCharSet() {
      return "238";
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
      QByteArray result((first_capital ? "Klas" : "klas"));
      result += (singular ? "a" : "y");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Plik" : "plik"));
      if (!singular) {
         result += "i";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Przestrze" : "przestrze"));
      result += (singular ? "ń" : "nie");
      result += " nazw";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Grupa" : "grupa"));
      result += (singular ? "a" : "y");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Stron" : "stron"));
      result += (singular ? "a" : "y");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Składow" : "składow"));
      result += (singular ? "a" : "e");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trField(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Pol" : "pol"));
      result += (singular ? "e" : "a");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Global" : "global"));
      result += (singular ? "ny" : "ne");
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Auto" : "auto"));
      result += (singular) ? "r" : "rzy";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Odwołuje się do";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////


   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Implementuje " + trWriteList(numEntries) + ".";
   }

   virtual QByteArray trImplementedInList(int numEntries) {
      return "Implementowany w " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Spis treści";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Lista elementów do wycofania";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Zdarzenia";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Dokumentacja zdarzeń";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Typy pakietu";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Funkcje pakietu";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Statyczne funkcje pakietu";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Atrybuty pakietu";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Statyczne atrybuty pakietu";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "All";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Oto graf wywołań dla tej funkcji:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Wyniki szukania";
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
         return "Niestety żaden dokument nie pasuje do twojego zapytania.";
      } else if (numDocuments == 1) {
         return "Znaleziono <b>1</b> dokument pasujący do twojego zapytania.";
      }
      int count = numDocuments % 10;
      if ((count >= 2) && (count <= 4)) {
         return "Znaleziono <b>$num</b> dokumenty pasujące do twojego zapytania. "
                "Najlepiej pasujące dokumenty wyświetlane są na początku listy.";
      } else {
         return "Znaleziono <b>$num</b> dokumentów pasujących do twojego zapytania. "
                "Najlepiej pasujące dokumenty wyświetlane są na początku listy.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Pasujące słowa:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return "Plik źródłowy " + filename;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Struktura katalogów";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Dokumentacja katalogów";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Katalogi";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Ta struktura katalogów jest posortowana jest z grubsza, "
             "choć nie całkowicie, alfabetycznie:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = "Dokumentacja katalogu ";
      result += dirName;
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Katalog" : "katalog"));
      if (! singular) {
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
      return "To jest metoda przeciążona, udostępniona dla wygody. "
             "Różni się od powyższej metody tylko zestawem akceptowanych argumentów.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Oto graf wywoływań tej funkcji:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Dokumentacja wyliczeń";   //TODO check if it is correct translation
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Dokumentacja składowej funkcji/podprogramu";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Lista typów danych";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Pola danych";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Tutaj znajdują się typy danych z ich krótkimi opisami:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Tutaj znajduje się lista wszystkich ";
      if (!extractAll) {
         result += "udokumentowanych ";
      }
      result += "składowych typów danych";
      result += " wraz z odnośnikami do ";
      if (!extractAll) {
         result += "dokumentacji struktury danych dla każdej składowej";
      } else {
         result += "typów danych, do których dana składowa należy:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Indeks typów danych";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Dokumentacja typów danych";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Funkcje/podprogramy";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Dokumentacja funkcji/podprogramu";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Typy danych";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Lista modułów";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Tutaj znajduje się lista wszystkich ";
      if (!extractAll) {
         result += "udokumentowanych ";
      }
      result += "modułów z ich krótkimi opisami:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = "Dokumentacja";
      if (isTemplate) {
         result += " szablonu";
      }
      switch (compType) {
         case ClassDef::Class:
            result += " modułu ";
            break;
         case ClassDef::Struct:
            result += " typu ";
            break;
         case ClassDef::Union:
            result += " unii ";
            break;
         case ClassDef::Interface:
            result += " interfejsu ";
            break;
         case ClassDef::Protocol:
            result += " protokołu ";
            break;
         case ClassDef::Category:
            result += " kategorii ";
            break;
         case ClassDef::Exception:
            result += " wyjątku ";
            break;
         default:
            break;
      }
      result += (QByteArray)clName;
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = "Dokumentacja modułu ";
      result += namespaceName;
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Składowe modułu";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Tutaj znajduje się lista wszystkich ";
      if (!extractAll) {
         result += "udokumentowanych ";
      }
      result += "składowych modułów wraz z odnośnikami do ";
      if (extractAll) {
         result += "dokumentacji modułu dla każdej składowej:";
      } else {
         result += "modułów do których składowe te należą:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Indeks modułu";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Moduł" : "moduł"));
      if (!singular) {
         result += "y";
      }
      return result;
   }
   /*! This is put at the bottom of a module documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFilesFortran(ClassDef::CompoundType compType,
         bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"Dokumentacja dla te";
      switch (compType) {
         case ClassDef::Class:
            result += "go modułu";
            break;
         case ClassDef::Struct:
            result += "go typu";
            break;
         case ClassDef::Union:
            result += "j unii";
            break;
         case ClassDef::Interface:
            result += "go interfejsu";
            break;
         case ClassDef::Protocol:
            result += "go protokołu";
            break;
         case ClassDef::Category:
            result += "j kategorii";
            break;
         case ClassDef::Exception:
            result += "go wyjątku";
            break;
         default:
            break;
      }
      result += " została wygenerowana z plik";
      if (single) {
         result += "u:";
      } else {
         result += "ów:";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Typ" : "typ"));
      if (!singular) {
         result += "y";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Podprogram" : "podprogram"));
      if (!singular) {
         result += "y";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Więzy typów"; //TODO check if it is correct translation
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return "Relcja " + QByteArray(name);
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Wczytywanie...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Globalna przestrzeń nazw";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Szukanie...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Brak dopasowań";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Plik w " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Zawiera plik w " + name;
   }
 
   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Odwołania do literatury";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Wykres zależności katalogu dla ") + name + ":";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.0
   //////////////////////////////////////////////////////////////////////////

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "poziom szczegółów";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Parametry Szablonu";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "i " + number + " więcej...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "Dokumentacja dla tego wyliczenia została wygenerowana z plik";
      if (single) {
         result += "u:";
      } else {
         result += "ów:";
      }
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      QByteArray result = "Dokumentacja wyliczenia ";
      result += name;
      return result;
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " dziedziczone z " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Dodatkowe Dziedziczone Składowe";
   }

   //////////////////////////////////////////////////////////////////////////

};

#endif
