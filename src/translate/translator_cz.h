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

#ifndef TRANSLATOR_CZ_H
#define TRANSLATOR_CZ_H

class TranslatorCzech : public Translator
{
 public:
   // --- Language control methods -------------------

   virtual QByteArray idLanguage() {
      return "czech";
   }

   virtual QByteArray latexLanguageSupportCommand() {
      return "\\usepackage[T2A]{fontenc}\n"
             "\\usepackage[czech]{babel}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Související funkce";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Uvedené funkce nejsou členskými funkcemi.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Detailní popis";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Dokumentace k členským typům";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Dokumentace k členským výčtům";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Dokumentace k metodám";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Dokumentace k položkám";
      } else {
         return "Dokumentace k datovým členům";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Seznam všech členů";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Seznam členů třídy";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Zde naleznete úplný seznam členů třídy ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", včetně všech zděděných členů.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result("Generováno automaticky programem Doxygen "
                        "ze zdrojových textů");
      if (s) {
         result += QByteArray(" projektu ") + s;
      }
      result += ".";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "jméno výčtu";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "hodnota výčtu";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "definován v";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Moduly";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Hierarchie tříd";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datové struktury";
      } else {
         return "Seznam tříd";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Seznam souborů";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datové položky";
      } else {
         return "Seznam členů tříd";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Globální symboly";
      } else {
         return "Symboly v souborech";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Ostatní stránky";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Příklady";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Hledat";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Zde naleznete seznam, vyjadřující vztah dědičnosti tříd. "
             "Je seřazen přibližně (ale ne úplně) podle abecedy:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Zde naleznete seznam všech ";
      if (!extractAll) {
         result += "dokumentovaných ";
      }
      result += "souborů se stručnými popisy:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Následující seznam obsahuje identifikace datových "
                "struktur a jejich stručné popisy:";
      } else {
         return "Následující seznam obsahuje především identifikace "
                "tříd, ale nacházejí se zde i další netriviální prvky, "
                "jako jsou struktury (struct), unie (union) a rozhraní "
                "(interface). V seznamu jsou uvedeny jejich stručné "
                "popisy:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Zde naleznete seznam všech ";
      if (!extractAll) {
         result += "dokumentovaných ";
      }

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "položek struktur (struct) a unií (union) ";
      } else {
         result += "členů tříd ";
      }

      result += "s odkazy na ";

      if (extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "dokumentaci struktur/unií, ke kterým příslušejí:";
         } else {
            result += "dokumentaci tříd, ke kterým příslušejí:";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "struktury/unie, ke kterým příslušejí:";
         } else {
            result += "třídy, ke kterým příslušejí:";
         }
      }

      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Zde naleznete seznam všech ";
      if (!extractAll) {
         result += "dokumentovaných ";
      }

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "funkcí, proměnných, maker, výčtů a definic typů (typedef) "
                   "s odkazy na ";
      } else {
         result += "symbolů, které jsou definovány na úrovni svých souborů. "
                   "Pro každý symbol je uveden odkaz na ";
      }

      if (extractAll) {
         result += "soubory, ke kterým příslušejí:";
      } else {
         result += "dokumentaci:";
      }

      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Zde naleznete seznam všech příkladů:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Následující seznam odkazuje na další stránky projektu:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Zde naleznete seznam všech modulů:";
   }

   // index titles (the project name is prepended for these)


   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Dokumentace";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Rejstřík modulů";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Rejstřík hierarchie tříd";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Rejstřík datových struktur";
      } else {
         return "Rejstřík tříd";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Rejstřík souborů";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Dokumentace modulů";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Dokumentace datových struktur";
      } else {
         return "Dokumentace tříd";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Dokumentace souborů";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Dokumentace příkladů";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Dokumentace souvisejících stránek";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Referenční příručka";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Definice maker";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Definice typů";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Výčty";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Funkce";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Proměnné";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Hodnoty výčtu";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Dokumentace k definicím maker";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Dokumentace definic typů";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Dokumentace výčtových typů";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Dokumentace funkcí";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Dokumentace proměnných";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datové struktry";
      } else {
         return "Třídy";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result("Generováno ");
      result += date;
      if (projName) {
         result += QByteArray(" pro projekt ") + projName;
      }
      result += " programem";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return QByteArray("Diagram dědičnosti pro třídu ") + clName;
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Pouze pro vnitřní použití.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Pozor";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Verze";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Datum";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Návratová hodnota";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Viz také";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Parametry";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Výjimky";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Generováno programem";
   }

   // new since 0.49-990307

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Seznam prostorů jmen";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Zde naleznete seznam všech ";
      if (!extractAll) {
         result += "dokumentovaných ";
      }
      result += "prostorů jmen se stručným popisem:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Friends";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Dokumentace k friends";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result("Dokumentace ");
      if (isTemplate) {
         result += "šablony ";
      }
      switch (compType) {
         case ClassDef::Class:
            result += "třídy ";
            break;
         case ClassDef::Struct:
            result += "struktury ";
            break;
         case ClassDef::Union:
            result += "unie ";
            break;
         case ClassDef::Interface:
            result += "rozhraní ";
            break;
         case ClassDef::Protocol:
            result += "protokolu ";
            break;
         case ClassDef::Category:
            result += "kategorie ";
            break;
         case ClassDef::Exception:
            result += "výjimky ";
            break;
         default:
            break;
      }
      result += clName;
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result("Dokumentace souboru ");
      result += fileName;
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result("Dokumentace prostoru jmen ");
      result += namespaceName;
      return result;
   }

   /*
    *  these are for the member sections of a class, struct or union
    */
   virtual QByteArray trPublicMembers() {
      return "Veřejné metody";
   }
   virtual QByteArray trPublicSlots() {
      return "Veřejné sloty";
   }
   virtual QByteArray trSignals() {
      return "Signály";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Statické veřejné metody";
   }
   virtual QByteArray trProtectedMembers() {
      return "Chráněné metody";
   }
   virtual QByteArray trProtectedSlots() {
      return "Chráněné sloty";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Statické chráněné metody";
   }
   virtual QByteArray trPrivateMembers() {
      return "Privátní metody";
   }
   virtual QByteArray trPrivateSlots() {
      return "Privátní sloty";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Statické privátní metody";
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
               result += " a ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      QByteArray result("Dědí z ");
      result += (numEntries == 1) ? "bázové třídy " : "bázových tříd ";
      result += trWriteList(numEntries) + ".";
      return result;
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      QByteArray result("Zděděna ");
      result += (numEntries == 1) ? "třídou " : "třídami ";
      result += trWriteList(numEntries) + ".";
      return result;
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      QByteArray result("Reimplementuje stejnojmenný prvek z ");
      result += trWriteList(numEntries) + ".";
      return result;
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      QByteArray result("Reimplementováno v ");
      result += trWriteList(numEntries) + ".";
      return result;
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Symboly v prostorech jmen";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Zde naleznete seznam všech ";
      if (!extractAll) {
         result += "dokumentovaných ";
      }
      result += "symbolů, které jsou definovány ve svých prostorech jmen. "
                "U každého je uveden odkaz na ";
      if (extractAll) {
         result += "dokumentaci příslušného prostoru jmen:";
      } else {
         result += "příslušný prostor jmen:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Rejstřík prostorů jmen";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Dokumentace prostorů jmen";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Prostory jmen";
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
      QByteArray result = (QByteArray)"Dokumentace pro ";
      switch (compType) {
         case ClassDef::Class:
            result += "tuto třídu";
            break;
         case ClassDef::Struct:
            result += "tuto strukturu (struct)";
            break;
         case ClassDef::Union:
            result += "tuto unii (union)";
            break;
         case ClassDef::Interface:
            result += "toto rozhraní";
            break;
         case ClassDef::Protocol:
            result += "tento protokol ";
            break;
         case ClassDef::Category:
            result += "tuto kategorii ";
            break;
         case ClassDef::Exception:
            result += "tuto výjimku";
            break;
         default:
            break;
      }
      result += " byla generována z ";
      if (single) {
         result += "následujícího souboru:";
      } else {
         result += "následujících souborů:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Vracené hodnoty";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Hlavní stránka";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "s.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Definice je uvedena na řádku @0 v souboru @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Definice v souboru @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Zastaralé";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Diagram tříd pro " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Graf závislostí na vkládaných souborech "
             "pro " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Dokumentace konstruktoru a destruktoru";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Zobrazit zdrojový text tohoto souboru.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Zobrazit dokumentaci tohoto souboru.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Precondition";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Postcondition";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Invariant";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Initializer:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "zdrojový text";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Grafické zobrazení hierarchie tříd";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Zobrazit grafickou podobu hierarchie tříd";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Zobrazit textovou podobu hierarchie tříd";
   }
   virtual QByteArray trPageIndex() {
      return "Rejstřík stránek";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Poznámka";
   }
   virtual QByteArray trPublicTypes() {
      return "Veřejné typy";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datové položky";
      } else {
         return "Veřejné atributy";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Statické veřejné atributy";
   }
   virtual QByteArray trProtectedTypes() {
      return "Chráněné typy";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Chráněné atributy";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Statické chráněné atributy";
   }
   virtual QByteArray trPrivateTypes() {
      return "Privátní typy";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Privátní atributy";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Statické privátní atributy";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a todo item */
   virtual QByteArray trTodo() {
      return "Plánované úpravy";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Seznam plánovaných úprav";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Používá se v";
   }
   virtual QByteArray trRemarks() {
      return "Poznámky"; // ??? not checked in a context
   }
   virtual QByteArray trAttention() {
      return "Upozornění"; // ??? not checked in a context
   }
   virtual QByteArray trInclByDepGraph() {
      return "Následující graf ukazuje, které soubory přímo nebo "
             "nepřímo vkládají tento soubor:";
   }
   virtual QByteArray trSince() {
      return "Od"; // ??? not checked in a context
   }

   ////////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Vysvětlivky ke grafu";
   }
   /*! page explaining how the dot graph's should be interpreted */
   virtual QByteArray trLegendDocs() {
      return
         "Zde naleznete vysvětlení, jak mají být interpretovány grafy, "
         "které byly generovány programem doxygen.<p>\n"
         "Uvažujte následující příklad:\n"
         "\\code\n"
         "/*! Neviditelná třída, která se v grafu nezobrazuje, protože "
         "došlo k ořezání grafu. */\n"
         "class Invisible { };\n\n"
         "/*! Třída, u které došlo k ořezání grafu. Vztah dědičnosti "
         "je skryt. */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Třída, která není dokumentována komentáři programu doxygen. */\n"
         "class Undocumented { };\n\n"
         "/*! Bázová třída děděná veřejně (public inheritance). */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! Šablona třídy. */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Bázová třída, použitá pro chráněné dědění "
         "(protected inheritance). */\n"
         "class ProtectedBase { };\n\n"
         "/*! Bázová třída, využitá pro privátní dědění "
         "(private inheritance). */\n"
         "class PrivateBase { };\n\n"
         "/*! Třída, která je využívána třídou Inherited. */\n"
         "class Used { };\n\n"
         "/*! Odvozená třída, která dědí z více tříd. */\n"
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
         "K výše uvedenému bude vygenerován následující graf:"
         "<p><center><img src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
         "<p>\n"
         "Bloky (tj. uzly) v uvedeném grafu mají následující význam:\n"
         "<ul>\n"
         "<li>Šedě vyplněný obdélník reprezentuje strukturu nebo třídu, "
         "pro kterou byl graf generován.\n"
         "<li>Obdélník s černým obrysem označuje dokumentovanou "
         "strukturu nebo třídu.\n"
         "<li>Obdélník s šedým obrysem označuje nedokumentovanou "
         "strukturu nebo třídu.\n"
         "<li>Obdélník s červeným obrysem označuje dokumentovanou "
         "strukturu nebo třídu, pro kterou\n"
         "nejsou zobrazeny všechny vztahy dědičnosti nebo obsažení. "
         "Graf je ořezán v případě, kdy jej\n"
         "není možné umístit do vymezeného prostoru.\n"
         "</ul>\n"
         "Šipky (tj. hrany grafu) mají následující význam:\n"
         "<ul>\n"
         "<li>Tmavě modrá šipka se používá pro označení vztahu veřejné "
         "dědičnosti (public) mezi dvěma třídami.\n"
         "<li>Tmavě zelená šipka označuje vztah chráněné dědičnosti "
         "(protected).\n"
         "<li>Tmavě červená šipka označuje vztah privátní dědičnosti "
         "(private).\n"
         "<li>Purpurová šipka kreslená čárkovaně se používá v případě, "
         "kdy je třída obsažena v jiné třídě,\n"
         "nebo kdy je používána jinou třídou. Je označena identifikátorem "
         "jedné nebo více proměných, přes které\n"
         "je třída nebo struktura zpřístupněna.\n"
         "<li>Žlutá šipka kreslená čárkovaně vyjadřuje vztah mezi instancí šablony "
         "a šablonou třídy, na základě které byla\n"
         "instance šablony vytvořena. V popisu šipky jsou uvedeny  příslušné"
         " parametry šablony.\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "vysvětlivky";
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
      return "Seznam testů";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Vlastnosti";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Dokumentace k vlastnosti";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      return "Třídy";
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return QByteArray("Balík ") + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Seznam balíků";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Zde naleznete seznam balíků se stručným popisem "
             "(pokud byl uveden):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Balíky";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Hodnota:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Chyba";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Seznam chyb";
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
      return "Rejstřík";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Tříd" : "tříd"));
      result += singular ? "a" : "y";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Soubor" : "soubor"));
      if (!singular) {
         result += "y";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Prostor" : "prostor"));
      if (!singular) {
         result += "y";
      }
      result += " jmen";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Skupin" : "skupin"));
      result += singular ? "a" : "y";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Stránk" : "stránk"));
      result += singular ? "a" : "y";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Člen" : "člen"));
      if (!singular) {
         result += "y";
      }
      return result;
   }

   /*! ??? Jak to prelozit? Bylo by dobre, kdyby se ozval nekdo,
    * kdo to pouziva.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Global" : "global"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Auto" : "auto"));
      result += (singular) ? "r" : "ři";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Odkazuje se na";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Implementuje " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Implementováno v " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Obsah";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Seznam zastaralých prvků";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Události";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Dokumentace událostí";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Typy v balíku";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Funkce v balíku";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Statické funkce v balíku";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Atributy balíku";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Statické atributy balíku";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Vše";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Tato funkce volá...";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Výsledky vyhledávání";
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
         return "Lituji. Vašemu dotazu neodpovídá žádný dokument.";
      } else if (numDocuments == 1) {
         return "Nalezen jediný dokument, který vyhovuje vašemu dotazu.";
      } else {
         return "Nalezeno <b>$num</b> dokumentů, které vyhovují vašemu "
                "dotazu. Nejlépe odpovídající dokumenty jsou zobrazeny "
                "jako první.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Nalezená slova:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return QByteArray("Zdrojový soubor ") + filename;
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////


   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Hierarchie adresářů";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Dokumentace k adresářům";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Adresáře";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Následující hierarchie adresářů je zhruba, "
             "ale ne úplně, řazena podle abecedy:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = "Reference k adresáři ";
      result += dirName;
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Adresář" : "adresář"));
      if ( ! singular) {
         result += "e";
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
      return "Jde o přetíženou (overloaded) metodu, "
             "která má usnadnit používání. Od výše uvedené metody se liší "
             "pouze jinak zadávanými argumenty.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trCallerGraph() {
      return "Tuto funkci volají...";
   }


   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Dokumentace výčtových hodnot";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Dokumentace členských funkcí/podprogramů";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Seznam datových typů";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Datová pole";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Datové typy se stručnými popisy:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {

      QByteArray result = "Následuje seznam všech ";
      if (!extractAll) {
         result += "dokumentovaných ";
      }
      result += "složek datových typů";
      result += " s odkazy na ";
      if (!extractAll) {
         result += "dokumentaci datové struktury pro každou složku:";
      } else {
         result += "příslušné datové typy:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Rejstřík datových typů";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Dokumentace k datovým typům";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Funkce/podprogramy";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Dokumentace funkce/podprogramu";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Datové typy";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Seznam modulů";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Následuje seznam všech ";
      if (!extractAll) {
         result += "dokumentovaných ";
      }
      result += "modulů se stručnými popisy:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result("Dokumentace ");
      if (isTemplate) {
         result += "šablony ";
      }
      switch (compType) {
         case ClassDef::Class:
            result += "třídy ";
            break;
         case ClassDef::Struct:
            result += "typu ";
            break;
         case ClassDef::Union:
            result += "unie ";
            break;
         case ClassDef::Interface:
            result += "rozhraní ";
            break;
         case ClassDef::Protocol:
            result += "protokolu ";
            break;
         case ClassDef::Category:
            result += "kategorie ";
            break;
         case ClassDef::Exception:
            result += "výjimky ";
            break;
         default:
            break;
      }
      result += clName;
      return result;

   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = "Dokumentace modulu ";
      result += namespaceName;
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Části modulu";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Následuje seznam všech ";
      if (!extractAll) {
         result += "dokumentovaných ";
      }
      result += "částí modulů s odkazy ";
      if (extractAll) {
         result += "na dokumentaci modulu pro danou část:";
      } else {
         result += "na moduly, ke kterým část patří:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Rejstřík modulů";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Modul" : "modul"));
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
      // here s is one of " Module", " Struct" or " Union"
      // single is true implies a single file
      QByteArray result = (QByteArray)"Dokumentace ";
      switch (compType) {
         case ClassDef::Class:
            result += "k tomuto modulu";
            break;
         case ClassDef::Struct:
            result += "k tomuto typu";
            break;
         case ClassDef::Union:
            result += "k této unii";
            break;
         case ClassDef::Interface:
            result += "k tomuto rozhraní";
            break;
         case ClassDef::Protocol:
            result += "k tomuto protokolu";
            break;
         case ClassDef::Category:
            result += "k této kategorii";
            break;
         case ClassDef::Exception:
            result += "k této výjimce";
            break;
         default:
            break;
      }
      result += " byla vygenerována z ";
      if (single) {
         result += "následujícího souboru:";
      } else {
         result += "následujících souborů:";
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

   /*! C# Type Contraint list */
   virtual QByteArray trTypeConstraints() {
      return "Omezení typů (Type Constraints)";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return "Relace " + QByteArray(name);
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Načítám...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Globální prostor jmen";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Vyhledávám...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Nic se nenašlo";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Soubor v " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Vkládá (include) soubor z " + name;
   }
  
   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Odkazy na literaturu";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Graf závislosti na adresářích pro  ") + name + ":";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.0
   //////////////////////////////////////////////////////////////////////////

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "úroveň detailů";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Parametry šablon";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "a " + number + " další(ch)...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "Dokumentace pro tento výčet byla generována z ";
      if (single) {
         result += "následujícího souboru:";
      } else {
         result += "následujících souborů:";
      }
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return "Reference k výčtu " + QByteArray(name);
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " dědí se z " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Další zděděné členy";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      QByteArray opt = enable ? "povolit" : "zakázat";
      return opt + " synchronizaci panelů";
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "Deklarováno v kategorii @1.";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "Rozšiřuje třídu @1.";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "Metody třídy";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "Metody instance";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "Dokumentace metody";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      return "Návrhové schéma";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.4
   //////////////////////////////////////////////////////////////////////////

   /** old style UNO IDL services: implemented interfaces */
   virtual QByteArray trInterfaces() {
      return "Exportovaná rozhraní";
   }

   /** old style UNO IDL services: inherited services */
   virtual QByteArray trServices() {
      return "Začleněné služby";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroups() {
      return "Konstantní skupiny";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroupReference(const char *namespaceName) {
      QByteArray result = "Konstantní skupiny z ";
      result += namespaceName;
      return result;
   }
   /** UNO IDL service page title */
   virtual QByteArray trServiceReference(const char *sName) {
      QByteArray result = "Popis služby ";
      result += sName;
      return result;
   }
   /** UNO IDL singleton page title */
   virtual QByteArray trSingletonReference(const char *sName) {
      QByteArray result = "Popis singletonu ";
      result += sName;
      return result;
   }
   /** UNO IDL service page */
   virtual QByteArray trServiceGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = "Dokumentace k této službě byla generována ";
      if (single) {
         result += "z následujícího souboru:";
      } else {
         result += "z následujících souborů:";
      }
      return result;
   }
   /** UNO IDL singleton page */
   virtual QByteArray trSingletonGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = "Dokumentace k tomuto singletonu byla generována ";
      if (single) {
         result += "z následujícího souboru:";
      } else {
         result += "z následujících souborů:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////

};

#endif // TRANSLATOR_CZ_H
