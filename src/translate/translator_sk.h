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

#ifndef TRANSLATOR_SK_H
#define TRANSLATOR_SK_H

class TranslatorSlovak : public Translator
{
 public:
   // --- Language control methods -------------------

   virtual QByteArray idLanguage() {
      return "slovak";
   }

   virtual QByteArray latexLanguageSupportCommand() {
      return "\\usepackage[slovak]{babel}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Súvisiace funkcie";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Uvedené funkcie niesú členskými funkciami.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Detailný popis";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Dokumentácia k členským typom";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Dokumentácia k členským enumeráciám";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Dokumentácia k metódam";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Dokumentácia k položkám";
      } else {
         return "Dokumentácia k dátovým členom";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Zoznam všetkých členov";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Zoznam členov triedy";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Tu nájdete úplný zoznam členov triedy ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", vrátane všetkých zdedených členov.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *	parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result("Generované automaticky programom Doxygen "
                        "zo zdrojových textov");
      if (s) {
         result += (QByteArray)" projektu " + s;
      }
      result += ".";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "meno enumerácie";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "hodnota enumerácie";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "definovaný v";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *	compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Moduly";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Hierarchia tried";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Dátové štruktúry";
      } else {
         return "Zoznam tried";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Zoznam súborov";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Dátové položky";
      } else {
         return "Zoznam členov tried";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Globálne symboly";
      } else {
         return "Symboly v súboroch";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Ostatné stránky";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Príklady";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Hľadať";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Tu nájdete zoznam, vyjadrujúci vzťah dedičnosti tried. "
             "Je zoradený približne (ale nie úplne) podľa abecedy:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result("Tu nájdete zoznam všetkých ");
      if (!extractAll) {
         result += "dokumentovaných ";
      }
      result += "súborov so stručnými popismi:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Nasledujúci zoznam obsahuje identifikáciu dátových "
                "štruktúr a ich stručné popisy:";
      } else {
         return "Nasledujúci zoznam obsahuje predovšetkým identifikáciu "
                "tried, ale nachádzajú sa tu i ďalšie netriviálne prvky, "
                "ako sú štruktúry (struct), uniony (union) a rozhrania "
                "(interface). V zozname sú uvedené ich stručné "
                "popisy:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Tu nájdete zoznam všetkých ";
      if (!extractAll) {
         result += "dokumentovaných ";
      }

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "položiek štruktúr (struct) a unionov (union) ";
      } else {
         result += "členov tried ";
      }

      result += "s odkazmi na ";

      if (!extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "dokumentáciu štruktúr/unionov, ku ktorým prislúchajú:";
         } else {
            result += "dokumentáciu tried, ku ktorým prislúchajú:";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "štruktúry/uniony, ku ktorým prislúchajú:";
         } else {
            result += "triedy, ku ktorým prislúchajú:";
         }
      }

      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Tu nájdete zoznam všetkých ";
      if (!extractAll) {
         result += "dokumentovaných ";
      }

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "funkcií, premenných, makier, enumerácií a definícií typov (typedef) "
                   "s odkazmi na ";
      } else {
         result += "symbolov, ktoré sú definované na úrovni svojich súborov. "
                   "Pre každý symbol je uvedený odkaz na ";
      }

      if (extractAll) {
         result += "súbory, ku ktorým prislúchajú:";
      } else {
         result += "dokumentáciu:";
      }

      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Tu nájdete zoznam všetkých príkladov:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Nasledujúci zoznam odkazuje na ďalšie stránky projektu, "
             "ktoré majú charakter usporiadaných zoznamov informácií, "
             "pozbieraných z rôznych miest v zdrojových súboroch:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Tu nájdete zoznam všetkých modulov:";
   }

   // index titles (the project name is prepended for these)

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Dokumentácia";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Register modulov";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Register hierarchie tried";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Register dátových štruktúr";
      } else {
         return "Register tried";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Register súborov";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *	the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Dokumentácia modulov";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *	the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      return "Dokumentácia tried";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *	the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Dokumentácia súborov";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *	the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Dokumentácia príkladov";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *	the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Dokumentácia súvisiacich stránok";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Referenčná príručka";
   }

   /*! This is used in the documentation of a file as a header before the
    *	list of defines
    */
   virtual QByteArray trDefines() {
      return "Definícia makier";
   }

   /*! This is used in the documentation of a file as a header before the
    *	list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Definícia typov";
   }

   /*! This is used in the documentation of a file as a header before the
    *	list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Enumerácie";
   }

   /*! This is used in the documentation of a file as a header before the
    *	list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Funkcie";
   }

   /*! This is used in the documentation of a file as a header before the
    *	list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Premenné";
   }

   /*! This is used in the documentation of a file as a header before the
    *	list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Hodnoty enumerácií";
   }

   /*! This is used in the documentation of a file before the list of
    *	documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Dokumentácia k definíciám makier";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *	of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Dokumentácia definícií typov";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *	of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Dokumentácia enumeračných typov";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *	of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Dokumentácia funkcií";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *	of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Dokumentácia premenných";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *	the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Dátové štruktúry";
      } else {
         return "Triedy";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *	the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Generované " + date;
      if (projName) {
         result += (QByteArray)" pre projekt " + projName;
      }
      result += (QByteArray)" programom";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Diagram dedičnosti pre triedu " + clName;
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Iba pre interné použitie.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Pozor";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Verzia";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Dátum";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Návratová hodnota";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Viz tiež";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Parametre";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Výnimky";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Generované programom";
   }

   // new since 0.49-990307

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Zoznam priestorov mien";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Tu nájdete zoznam všetkých ";
      if (!extractAll) {
         result += "dokumentovaných ";
      }
      result += "priestorov mien so stručným popisom:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *	friends of a class
    */
   virtual QByteArray trFriends() {
      return "Priatelia (friends)";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Dokumentácia k priateľom (friends)";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result("Dokumentácia ");
      if (isTemplate) {
         result += "šablóny ";
      }
      switch (compType) {
         case ClassDef::Class:
            result += "triedy ";
            break;
         case ClassDef::Struct:
            result += "štruktúry ";
            break;
         case ClassDef::Union:
            result += "unionu ";
            break;
         case ClassDef::Interface:
            result += "rozhrania ";
            break;
         case ClassDef::Protocol:
            result += "protokol ";
            break;
         case ClassDef::Category:
            result += "kategória ";
            break;
         case ClassDef::Exception:
            result += "výnimky ";
            break;
         default:
            break;
      }
      result += clName;
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result("Dokumentácia súboru ");
      result += fileName;
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result("Dokumentácia priestoru mien ");
      result += namespaceName;
      return result;
   }

   /* these are for the member sections of a class, struct or union */
   virtual QByteArray trPublicMembers() {
      return "Verejné metódy";
   }
   virtual QByteArray trPublicSlots() {
      return "Verejné sloty";
   }
   virtual QByteArray trSignals() {
      return "Signály";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Statické verejné metódy";
   }
   virtual QByteArray trProtectedMembers() {
      return "Chránené metódy";
   }
   virtual QByteArray trProtectedSlots() {
      return "Chránené sloty";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Statické chránené metódy";
   }
   virtual QByteArray trPrivateMembers() {
      return "Privátne metódy";
   }
   virtual QByteArray trPrivateSlots() {
      return "Privátne sloty";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Statické privátne metódy";
   }

   /*! this function is used to produce a comma-separated list of items.
    *	use generateMarker(i) to indicate where item i should be put.
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
            } else {			 // the fore last entry
               result += " a ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *	if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      QByteArray result("Dedí od ");
      result += (numEntries == 1) ? "bázovej triedy " : "bázových tried ";
      result += trWriteList(numEntries) + ".";
      return result;
   }

   /*! used in class documentation to produce a list of super classes,
    *	if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      QByteArray result("Zdedená ");
      result += (numEntries == 1) ? "triedou " : "triedami ";
      result += trWriteList(numEntries) + ".";
      return result;
   }

   /*! used in member documentation blocks to produce a list of
    *	members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      QByteArray result("Reimplementuje ");
      result += (numEntries == 1) ? "metódu triedy " : "metódy tried ";
      result += trWriteList(numEntries) + ".";
      return result;
   }

   /*! used in member documentation blocks to produce a list of
    *	all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      QByteArray result("Reimplementované ");
      result += (numEntries == 1) ? "triedou " : "triedami ";
      result += trWriteList(numEntries) + ".";
      return result;
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Symboly v priestoroch mien";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Tu nájdete zoznam všetkých ";
      if (!extractAll) {
         result += "dokumentovaných ";
      }
      result += "symbolov, ktoré sú definované vo svojich priestoroch mien. "
                "U každého je uvedený odkaz na ";
      if (extractAll) {
         result += "dokumentáciu príslušného priestoru mien:";
      } else {
         result += "príslušný priestor mien:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *	index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Register priestorov mien";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *	the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Dokumentácia priestorov mien";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *	namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Priestory mien";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990728
   //////////////////////////////////////////////////////////////////////////

   /*! This is put at the bottom of a class documentation page and is
    *	followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFiles(ClassDef::CompoundType compType,
                                           bool single) {
      // here s is one of " Class", " Struct" or " Union"
      // single is true implies a single file
      QByteArray result = (QByteArray)"Dokumentácia pre ";
      switch (compType) {
         case ClassDef::Class:
            result += "túto triedu";
            break;
         case ClassDef::Struct:
            result += "túto štruktúru (struct)";
            break;
         case ClassDef::Union:
            result += "tento union";
            break;
         case ClassDef::Interface:
            result += "toto rozhranie";
            break;
         case ClassDef::Protocol:
            result += "protokol";
            break;
         case ClassDef::Category:
            result += "kategória";
            break;
         case ClassDef::Exception:
            result += "túto výnimku";
            break;
         default:
            break;
      }
      result += " bola generovaná z ";
      if (single) {
         result += "nasledujúceho súboru:";
      } else {
         result += "nasledujúcich súborov:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Návratové hodnoty";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Hlavná stránka";
   }

   /*! This is used in references to page that are put in the LaTeX
    *	documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "s.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Definícia je uvedená na riadku @0 v súbore @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Definícia v súbore @0.";
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
      return (QByteArray)"Diagram tried pre " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Graf závislostí na vkladaných súboroch "
             "pre " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Dokumentácia konštruktoru a deštruktoru";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Zobraziť zdrojový text tohoto súboru.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Zobraziť dokumentáciu tohoto súboru.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Prepodmienka";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Postpodmienka";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Invariant";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Inicializátor:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "zdrojový text";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Grafické zobrazenie hierarchie tried";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Zobraziť grafickú podobu hierarchie tried";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Zobraziť textovú podobu hierarchie tried";
   }
   virtual QByteArray trPageIndex() {
      return "Register stránok";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Poznámka";
   }
   virtual QByteArray trPublicTypes() {
      return "Verejné typy";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Dátové položky";
      } else {
         return "Verejné atribúty";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Statické verejné atribúty";
   }
   virtual QByteArray trProtectedTypes() {
      return "Chránené typy";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Chránené atribúty";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Statické chránené atribúty";
   }
   virtual QByteArray trPrivateTypes() {
      return "Privátne typy";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Privátne atribúty";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Statické privátne atribúty";
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
      return "Zoznam plánovaných úprav";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Používa sa v";
   }
   virtual QByteArray trRemarks() {
      return "Poznámky";
   }
   virtual QByteArray trAttention() {
      return "Upozornenie";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Nasledujúci graf ukazuje, ktoré súbory priamo alebo "
             "nepriamo vkladajú tento súbor:";
   }
   virtual QByteArray trSince() {
      return "Od";
   }

   ////////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Vysvetlivky ku grafu";
   }
   /*! page explaining how the dot graph's should be interpreted */
   virtual QByteArray trLegendDocs() {
      return
         "Tu nájdete vysvetlenie, ako majú byť interpretované grafy, "
         "ktoré boli generované programom doxygen.<p>\n"
         "Uvažujte nasledujúci príklad:\n"
         "\\code\n"
         "/*! Neviditelná trieda, ktorá sa v grafe nezobrazuje, pretože "
         "došlo k orezaniu grafu. */\n"
         "class Invisible { };\n\n"
         "/*! Trieda, u ktorej došlo k orezaniu grafu. Vzťah dedičnosti "
         "je skrytý. */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Trieda, ktorá nieje dokumentovaná komentármi programu doxygen. */\n"
         "class Undocumented { };\n\n"
         "/*! Odvodená trieda s verejným (public) dedením bázovej triedy. */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! Šablóna triedy. */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Odvodená trieda s chráneným (protected) dedením bázovej triedy. */\n"
         "class ProtectedBase { };\n\n"
         "/*! Odvodená trieda s privátnym dedením bázovej triedy. */\n"
         "class PrivateBase { };\n\n"
         "/*! Trieda, ktorá je využívaná triedou Inherited. */\n"
         "class Used { };\n\n"
         "/*! Odvodená trieda, ktorá rôznym spôsobom dedí od viacerých bázových "
         "tried. */\n"
         "class Inherited : public PublicBase,\n"
         "                  protected ProtectedBase,\n"
         "                  private PrivateBase,\n"
         "                  public Undocumented,\n"
         "                  public Templ<int>\n"
         "{\n"
         "  private:\n"
         "	 Used *m_usedClass;\n"
         "};\n"
         "\\endcode\n"
         "K vyššie uvedenému bude vygenerovaný nasledujúci graf:"
         "<p><center><img src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
         "<p>\n"
         "Bloky (tj. uzly) v uvedenom grafe majú nasledujúci význam:\n"
         "<ul>\n"
         "<li>Čierne vyplnený obdĺžnik reprezentuje štruktúru alebo triedu, "
         "pre ktorú bol graf generovaný.\n"
         "<li>Obdĺžnik s čiernym obrysom označuje dokumentovanú "
         "štruktúru alebo triedu.\n"
         "<li>Obdĺžnik so šedým obrysom označuje nedokumentovanú "
         "štruktúru alebo triedu.\n"
         "<li>Obdĺžnik s červeným obrysom označuje dokumentovanú "
         "štruktúru alebo triedu, pre ktorú\n"
         "niesú zobrazené všetky vzťahy dedičnosti alebo obsiahnutia. "
         "Graf je orezaný v prípade, kedy ho\n"
         "nieje možné umiestniť do vymedzených hraníc.\n"
         "</ul>\n"
         "Šípky (tj. hrany grafu) majú nasledujúcí význam:\n"
         "<ul>\n"
         "<li>Tmavo modrá šípka sa používa pre označenie vzťahu verejnej "
         "dedičnosti medzi dvoma triedami.\n"
         "<li>Tmavo zelená šípka označuje vzťah chránenej dedičnosti "
         "(protected).\n"
         "<li>Tmavo červená šípka označuje vzťah privátnej dedičnosti.\n"
         "<li>Purpurová šípka kreslená čiarkovane sa používa v prípade, "
         "ak je trieda obsiahnutá v inej triede,\n"
         "alebo ak je používaná inou triedou. Je označená identifikátorom "
         "jednej alebo viacerých premenných (objektov), cez ktoré\n"
         "je trieda alebo štruktúra zprístupnena.\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "vysvetlivky";
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
      return "Zoznam testov";
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
      return "Dokumentácia k vlastnosti";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      return "Triedy";
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Balík " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Zoznam balíkov";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Tu nájdete zoznam balíkov so stručným popisom "
             "(pokiaľ bol uvedený):";
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
      return "Zoznam chýb";
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
      return "Index";
   }

   /*! This is used for translation of the word that will possibly
    *	be followed by a single name or by a list of names
    *	of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Tried" : "tried"));
      result += (singular ? "a" : "y");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *	be followed by a single name or by a list of names
    *	of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Súbor" : "súbor"));
      if (!singular) {
         result += "y";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *	be followed by a single name or by a list of names
    *	of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Priestor" : "priestor"));
      if (!singular) {
         result += "y";
      }
      result += " mien";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *	be followed by a single name or by a list of names
    *	of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Skupin" : "skupin"));
      result += (singular ? "a" : "y");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *	be followed by a single name or by a list of names
    *	of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Stránk" : "stránk"));
      result += (singular ? "a" : "y");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *	be followed by a single name or by a list of names
    *	of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Člen" : "člen"));
      if (!singular) {
         result += "y";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *	be followed by a single name or by a list of names
    *	of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Globáln" : "globáln"));
      result += (singular ? "y" : "e");
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *	for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Auto" : "auto"));
      result += (singular) ? "r" : "ri";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Odkazuje sa na";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *	members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Implementuje " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *	all members that implement this member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Implementované v " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *	of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Obsah";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *	flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Zastarané metódy";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Udalosti";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Dokumentácia udalostí";
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
      return "Funkcie v balíku";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Statické funkcie v balíku";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Atribúty balíku";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Statické atribúty balíku";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Všetko";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Táto funkcia volá...";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Výsledky vyhľadávania";
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
         return "Ľutujem. Vášmu dotazu neodpovedá žiadny dokument.";
      } else if (numDocuments == 1) {
         return "Bol nájdený jediný dokument, ktorý vyhovuje vášmu dotazu.";
      } else {
         return "Bolo nájdených <b>$num</b> dokumentov, ktoré vyhovujú vášmu "
                "dotazu. Najviac odpovedajúce dokumenty sú ako prvé.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Nájdené slová:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return QByteArray("Zdrojový súbor ") + filename;
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////


   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Hierarchia adresárov";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Dokumentácia k adresárom";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Adresáre";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Následujúca hierarchia adresárov je zhruba, "
             "ale nie úplne, zoradená podľa abecedy:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = "Referencia k adresáru ";
      result += dirName;
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Adresár" : "adresár"));
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
      return "Ide o preťaženú (overloaded) metódu, "
             "ktorá má uľahčiť používanie. Od vyššie uvedenej metódy sa odlišuje "
             "iba inak zadávanými argumentami.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trCallerGraph() {
      return "Túto funkciu volajú...";
   }


   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Dokumentácia enumeračných hodnôt";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Dokumentácia členských funkcií/podprogramov";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Zoznam dátových typov";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Dátové polia";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Dátové typy so stručnými popismi:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {

      QByteArray result = "Nasleduje zoznam všetkých ";
      if (!extractAll) {
         result += "dokumentovaných ";
      }
      result += "zložiek dátových typov";
      result += " s odkazmi na ";
      if (!extractAll) {
         result += "dokumentáciu dátovej štruktúry pre každú zložku:";
      } else {
         result += "příslušné dátové typy:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Register dátových typov";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Dokumentácia k dátovým typom";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Funkcie/podprogramy";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Dokumentácia funkcie/podprogramu";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Dátové typy";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Zoznam modulov";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Nasleduje zoznam všetkých ";
      if (!extractAll) {
         result += "dokumentovaných ";
      }
      result += "modulov so stručnými popismi:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result("Dokumentácia ");
      if (isTemplate) {
         result += "šablóny ";
      }
      switch (compType) {
         case ClassDef::Class:
            result += "triedy ";
            break;
         case ClassDef::Struct:
            result += "typu ";
            break;
         case ClassDef::Union:
            result += "únie ";
            break;
         case ClassDef::Interface:
            result += "rozhrania ";
            break;
         case ClassDef::Protocol:
            result += "protokolu ";
            break;
         case ClassDef::Category:
            result += "kategórie ";
            break;
         case ClassDef::Exception:
            result += "výnimky ";
            break;
         default:
            break;
      }
      result += clName;
      return result;

   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = "Dokumentácia modulu ";
      result += namespaceName;
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Časti modulu";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Nasleduje zoznam všetkých ";
      if (!extractAll) {
         result += "dokumentovaných ";
      }
      result += "častí modulov s odkazmi ";
      if (extractAll) {
         result += "na dokumentáciu modulov pre danú časť:";
      } else {
         result += "na moduly, ku ktorým časť patrí:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Register modulov";
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
      QByteArray result = (QByteArray)"Dokumentácia ";
      switch (compType) {
         case ClassDef::Class:
            result += "k tomuto modulu";
            break;
         case ClassDef::Struct:
            result += "k tomuto typu";
            break;
         case ClassDef::Union:
            result += "k tejto únii";
            break;
         case ClassDef::Interface:
            result += "k tomuto rozhraniu";
            break;
         case ClassDef::Protocol:
            result += "k tomuto protokolu";
            break;
         case ClassDef::Category:
            result += "k tejto kategórii";
            break;
         case ClassDef::Exception:
            result += "k tejto výnimke";
            break;
         default:
            break;
      }
      result += " bola vygenerovaná z ";
      if (single) {
         result += "nasledujúceho súboru:";
      } else {
         result += "nasledujúcich súborov:";
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
      return "Obmedzenie typov (Type Constraints)";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return "Relácia " + QByteArray(name);
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Načítam...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Globálny priestor mien";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Vyhľadávam...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Nič sa nenašlo";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Súbor v " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Vkladá (include) súbor z " + name;
   }
  
   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Odkazy na literatúru";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Graf závislosti na priečinkoch pre  ") + name + ":";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.0
   //////////////////////////////////////////////////////////////////////////

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "úroveň detailov";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Parametry šablón";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      QByteArray result("a " + number + " ďaľší");
      if (atoi(number) >= 5) {
         result += "ch";
      }
      return result + "...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "Dokumentácia pre tuto enumeráciu bola generovaná z ";
      if (single) {
         result += "nasledujúceho súboru:";
      } else {
         result += "nasledujúcich súborov:";
      }
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return "Referencia k enumerácii " + QByteArray(name);
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " dedí sa z " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Ďaľšie zdedené členy";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      QByteArray opt = enable ? "povoliť" : "zakázať";
      return opt + " synchronizáciu panelov";
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "Deklarované v kategórii @1.";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "Rozširuje triedu @1.";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "Metódy triedy";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "Metódy inštancie";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "Dokumentácia metódy";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      return "Návrhová schéma";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.4
   //////////////////////////////////////////////////////////////////////////

   /** old style UNO IDL services: implemented interfaces */
   virtual QByteArray trInterfaces() {
      return "Exportované rozhrania";
   }

   /** old style UNO IDL services: inherited services */
   virtual QByteArray trServices() {
      return "Začlenené služby";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroups() {
      return "Konštantné skupiny";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroupReference(const char *namespaceName) {
      QByteArray result = "Konštantné skupiny z ";
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
      QByteArray result = "Dokumentácia k tejto službe bola vygenerovaná ";
      if (single) {
         result += "z nasledujúceho súboru:";
      } else {
         result += "z nasledujúcich súborov:";
      }
      return result;
   }
   /** UNO IDL singleton page */
   virtual QByteArray trSingletonGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = "Dokumentácia k tomuto singletonu bola vygenerovaná ";
      if (single) {
         result += "z nasledujúceho súboru:";
      } else {
         result += "z nasledujúcich súborov:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
};

#endif // TRANSLATOR_SK_H
