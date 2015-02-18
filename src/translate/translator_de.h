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

#ifndef TRANSLATOR_DE_H
#define TRANSLATOR_DE_H

class TranslatorGerman : public TranslatorAdapter_1_8_4
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
      return "german";
   }

   /*! Used to get the LaTeX command(s) for the language support. This method
    *  was designed for languages which do wish to use a babel package.
    */
   virtual QByteArray latexLanguageSupportCommand() {
      return "\\usepackage[ngerman]{babel}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Verwandte Funktionen";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Es handelt sich hierbei nicht um Elementfunktionen.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Ausführliche Beschreibung";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Dokumentation der benutzerdefinierten Datentypen";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Dokumentation der Aufzählungstypen";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Dokumentation der Elementfunktionen";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      /* No difference if "OPTIMIZE_OUTPUT_FOR_C" is set! */
      return "Dokumentation der Datenelemente";
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Mehr ...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Aufstellung aller Elemente";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Elementverzeichnis";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Vollständige Aufstellung aller Elemente für ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return " einschließlich aller geerbten Elemente.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Automatisch erzeugt von Doxygen";
      if (s) {
         result += (QByteArray)" für " + s;
      }
      result += " aus dem Quellcode.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "enum-Bezeichner";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "enum-Wert";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "Definiert in";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Module";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Klassenhierarchie";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datenstrukturen";
      } else {
         return "Auflistung der Klassen";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Auflistung der Dateien";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datenstruktur-Elemente";
      } else {
         return "Klassen-Elemente";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers()
   /* No difference if "OPTIMIZE_OUTPUT_FOR_C" is set! */
   {
      return "Datei-Elemente";
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Zusätzliche Informationen";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Beispiele";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Suchen";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Die Liste der Ableitungen ist -mit Einschränkungen- "
             "alphabetisch sortiert:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Hier folgt die Aufzählung aller ";
      if (!extractAll) {
         result += "dokumentierten ";
      }
      result += "Dateien mit einer Kurzbeschreibung:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Hier folgt die Aufzählung aller Datenstrukturen "
                "mit einer Kurzbeschreibung:";
      } else {
         return "Hier folgt die Aufzählung aller Klassen, Strukturen, "
                "Varianten und Schnittstellen mit einer Kurzbeschreibung:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Hier folgt die Aufzählung aller ";
      if (!extractAll) {
         result += "dokumentierten ";
      }
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "Strukturen und Varianten";
      } else {
         result += "Klassenelemente";
      }
      result += " mit Verweisen auf ";
      if (!extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "die Dokumentation zu jedem Element:";
         } else {
            result += "die Klassendokumentation zu jedem Element:";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "die zugehörigen Elemente:";
         } else {
            result += "die zugehörigen Klassen:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Hier folgt die Aufzählung aller ";
      if (!extractAll) {
         result += "dokumentierten ";
      }
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "Funktionen, Variablen, Makros, Aufzählungen und Typdefinitionen";
      } else {
         result += "Dateielemente";
      }
      result += " mit Verweisen auf ";
      if (extractAll) {
         result += "die Dokumentation zu jedem Element:";
      } else {
         result += "die zugehörigen Dateien:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Hier folgt eine Liste mit allen Beispielen:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Hier folgt eine Liste mit zusammengehörigen Themengebieten:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Hier folgt die Aufzählung aller Module:";
   }

   // index titles (the project name is prepended for these)

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Dokumentation";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Modul-Verzeichnis";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Hierarchie-Verzeichnis";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datenstruktur-Verzeichnis";
      } else {
         return "Klassen-Verzeichnis";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Datei-Verzeichnis";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Modul-Dokumentation";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datenstruktur-Dokumentation";
      } else {
         return "Klassen-Dokumentation";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Datei-Dokumentation";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Dokumentation der Beispiele";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Zusätzliche Informationen";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Nachschlagewerk";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Makrodefinitionen";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Typdefinitionen";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Aufzählungen";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Funktionen";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Variablen";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Aufzählungswerte";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Makro-Dokumentation";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Dokumentation der benutzerdefinierten Typen";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Dokumentation der Aufzählungstypen";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Dokumentation der Funktionen";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Variablen-Dokumentation";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datenstrukturen";
      } else {
         return "Klassen";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Erzeugt am " + date;
      if (projName) {
         result += (QByteArray)" für " + projName;
      }
      result += (QByteArray)" von";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Klassendiagramm für " + clName + ":";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Nur für den internen Gebrauch.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Warnung";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Version";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Datum";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Rückgabe";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Siehe auch";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Parameter";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Ausnahmebehandlung";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Erzeugt von";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Liste aller Namensbereiche";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Liste aller ";
      if (!extractAll) {
         result += "dokumentierten ";
      }
      result += "Namensbereiche mit Kurzbeschreibung:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Freundbeziehungen";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Freundbeziehungen und Funktionsdokumentation";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result = (QByteArray)clName + " ";
      if (isTemplate) {
         result += "Template-";
      }
      switch (compType) {
         case ClassDef::Class:
            result += "Klassen";
            break;
         case ClassDef::Struct:
            result += "Struktur";
            break;
         case ClassDef::Union:
            result += "Varianten";
            break;
         case ClassDef::Interface:
            result += "Schnittstellen";
            break;
         case ClassDef::Protocol:
            result += "Protokoll";
            break;
         case ClassDef::Category:
            result += "Kategorie";
            break;
         case ClassDef::Exception:
            result += "Ausnahmen";
            break;
         default:
            break;
      }
      result += "referenz";
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = fileName;
      result += "-Dateireferenz";
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += "-Namensbereichsreferenz";
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Öffentliche Methoden";
   }

   virtual QByteArray trPublicSlots() {
      return "Öffentliche Slots";
   }

   virtual QByteArray trSignals() {
      return "Signale";
   }

   virtual QByteArray trStaticPublicMembers() {
      return "Öffentliche, statische Methoden";
   }

   virtual QByteArray trProtectedMembers() {
      return "Geschützte Methoden";
   }

   virtual QByteArray trProtectedSlots() {
      return "Geschützte Slots";
   }

   virtual QByteArray trStaticProtectedMembers() {
      return "Geschützte, statische Methoden";
   }

   virtual QByteArray trPrivateMembers() {
      return "Private Methoden";
   }

   virtual QByteArray trPrivateSlots() {
      return "Private Slots";
   }

   virtual QByteArray trStaticPrivateMembers() {
      return "Private, statische Methoden";
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
               result += " und ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Abgeleitet von " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Basisklasse für " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Erneute Implementation von " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Erneute Implementation in " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Elemente eines Namensbereiches";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Hier folgt die Aufzählung aller ";
      if (!extractAll) {
         result += "dokumentierten ";
      }
      result += "Namensbereichselemente mit Verweisen auf ";
      if (extractAll) {
         result += "die Namensbereichsdokumentation für jedes Element:";
      } else {
         result += "die zugehörigen Dateien:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Verzeichnis der Namensbereiche";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Dokumentation der Namensbereiche";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Namensbereiche";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990728
   //////////////////////////////////////////////////////////////////////////

   /*! This is put at the bottom of a class documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFiles(ClassDef::CompoundType compType,
                                           bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"Die Dokumentation für diese";
      switch (compType) {
         case ClassDef::Class:
            result += " Klasse";
            break;
         case ClassDef::Struct:
            result += " Struktur";
            break;
         case ClassDef::Union:
            result += " Variante";
            break;
         case ClassDef::Interface:
            result += " Schnittstelle";
            break;
         case ClassDef::Protocol:
            result += "s Protokoll";
            break;
         case ClassDef::Category:
            result += " Kategorie";
            break;
         case ClassDef::Exception:
            result += " Ausnahme";
            break;
         default:
            break;
      }
      result += " wurde erzeugt aufgrund der Datei";
      if (single) {
         result += ":";
      } else {
         result += "en:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Rückgabewerte";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Hauptseite";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "S.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Definiert in Zeile @0 der Datei @1.";
   }

   virtual QByteArray trDefinedInSourceFile() {
      return "Definiert in Datei @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Veraltet";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Zusammengehörigkeiten von " + clName + ":";
   }

   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Include-Abhängigkeitsdiagramm für " + fName + ":";
   }

   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Beschreibung der Konstruktoren und Destruktoren";
   }

   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "gehe zum Quellcode dieser Datei";
   }

   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "gehe zur Dokumentation dieser Datei";
   }

   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Vorbedingung";
   }

   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Nachbedingung";
   }

   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Invariante";
   }

   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Initialisierung:";
   }

   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "code";
   }

   virtual QByteArray trGraphicalHierarchy() {
      return "Grafische Darstellung der Klassenhierarchie";
   }

   virtual QByteArray trGotoGraphicalHierarchy() {
      return "gehe zur grafischen Darstellung der Klassenhierarchie";
   }

   virtual QByteArray trGotoTextualHierarchy() {
      return "gehe zur textbasierten Darstellung der Klassenhierarchie";
   }

   virtual QByteArray trPageIndex() {
      return "Seitenindex";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   //RK: had to change here because of the new command \remark
   virtual QByteArray trNote() {
      return "Zu beachten";
   }

   virtual QByteArray trPublicTypes() {
      return "Öffentliche Typen";
   }

   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datenfelder";
      } else {
         return "Öffentliche Attribute";
      }
   }

   virtual QByteArray trStaticPublicAttribs() {
      return "Statische öffentliche Attribute";
   }

   virtual QByteArray trProtectedTypes() {
      return "Geschützte Typen";
   }

   virtual QByteArray trProtectedAttribs() {
      return "Geschützte Attribute";
   }

   virtual QByteArray trStaticProtectedAttribs() {
      return "Statische, geschützte Attribute";
   }

   virtual QByteArray trPrivateTypes() {
      return "Private Typen";
   }

   virtual QByteArray trPrivateAttribs() {
      return "Private Attribute";
   }

   virtual QByteArray trStaticPrivateAttribs() {
      return "Statische, private Attribute";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "Noch zu erledigen";
   }

   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Ausstehende Aufgaben";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Wird benutzt von";
   }

   virtual QByteArray trRemarks() {
      return "Bemerkungen";
   }

   virtual QByteArray trAttention() {
      return "Achtung";
   }

   virtual QByteArray trInclByDepGraph() {
      return "Dieser Graph zeigt, welche Datei direkt oder "
             "indirekt diese Datei enthält:";
   }

   virtual QByteArray trSince() {
      return "Seit";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Erklärung des Graphen";
   }

   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Diese Seite erklärt die Interpretation der von doxygen "
         "erzeugten Graphen.<p>\n"
         "Beispiel:\n"
         "\\code\n"
         "/*! Wegen Verkürzung unsichtbare Klasse */\n"
         "class Invisible { };\n\n"
         "/*! Klasse verkürzt dargestellt, Vererbungsbeziehung ist versteckt */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Nicht mit doxygen-Kommentaren dokumentierte Klasse */\n"
         "class Undocumented { };\n\n"
         "/*! Mithilfe öffentlicher Vererbung vererbte Klasse */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! Eine Template-Klasse */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Mithilfe geschützter Vererbung vererbte Klasse */\n"
         "class ProtectedBase { };\n\n"
         "/*! Mithilfe privater Vererbung vererbte Klasse */\n"
         "class PrivateBase { };\n\n"
         "/*! Von der Klasse Inherited benutzte Klasse */\n"
         "class Used { };\n\n"
         "/*! Superklasse, die von mehreren anderen Klassen erbt */\n"
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
         "Dies liefert den folgenden Graphen:"
         "<p><center><img alt=\"\" src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center></p>\n"
         "<p>\n"
         "Die Rechtecke in obigem Graphen bedeuten:\n"
         "</p>\n"
         "<ul>\n"
         "<li>Ein grau gefülltes Rechteck stellt die Struktur oder "
         "Klasse dar, für die der Graph erzeugt wurde.</li>\n"
         "<li>Ein Rechteck mit schwarzem Rahmen kennzeichnet eine dokumentierte "
         "Struktur oder Klasse.</li>\n"
         "<li>Ein Rechteck mit grauem Rahmen kennzeichnet eine undokumentierte "
         "Struktur oder Klasse.</li>\n"
         "<li>Ein Rechteck mit rotem Rahmen kennzeichnet eine dokumentierte "
         "Struktur oder Klasse, für die nicht alle Vererbungs-/"
         "Enthaltenseinsbeziehungen dargestellt werden. Ein Graph wird gekürzt, "
         "wenn er nicht in die angegebenen Schranken passt.</li>\n"
         "</ul>\n"
         "<p>\n"
         "Die Pfeile bedeuten:\n"
         "</p>\n"
         "<ul>\n"
         "<li>Ein dunkelblauer Pfeil stellt eine öffentliche Vererbungsbeziehung "
         "zwischen zwei Klassen dar.</li>\n"
         "<li>Ein dunkelgrüner Pfeil stellt geschützte Vererbung dar.</li>\n"
         "<li>Ein dunkelroter Pfeil stellt private Vererbung dar.</li>\n"
         "<li>Ein gestrichelter violetter Pfeil bedeutet, dass eine Klasse in "
         "einer anderen enthalten ist oder von einer anderen benutzt wird. Am "
         "Pfeil stehen die Variable(n), mit deren Hilfe auf die Struktur oder "
         "Klasse an der Pfeilspitze zugegriffen werden kann.</li>\n"
         "<li>Ein gestrichelter gelber Pfeil kennzeichnet eine Verknüpfung "
         "zwischen einer Template-Instanz und der Template-Klasse von welcher "
         "es abstammt. Neben dem Pfeil sind die Template-Parameter aufgeführt.</li>\n"
         "</ul>\n";
   }

   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "Legende";
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
      return "Test-Liste";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Propertys";
   }

   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Dokumentation der Propertys";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datenstrukturen";
      } else {
         return "Klassen";
      }
   }

   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Paket " + name;
   }

   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Paketliste";
   }

   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Hier folgen die Pakete mit einer Kurzbeschreibung (wenn verfügbar):";
   }

   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Pakete";
   }

   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Wert:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Fehler";
   }

   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Liste der bekannten Fehler";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.6-20010422
   //////////////////////////////////////////////////////////////////////////

   /*! Used as ansicpg for RTF file. See translator_en.h for details. */
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
      return "Index";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool, bool singular) {
      QByteArray result("Klasse");
      if (!singular) {
         result += "n";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool, bool singular) {
      QByteArray result("Datei");
      if (!singular) {
         result += "en";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool, bool singular) {
      QByteArray result("Namensbereich");
      if (!singular) {
         result += "e";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool, bool singular) {
      QByteArray result("Gruppe");
      if (!singular) {
         result += "n";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool, bool singular) {
      QByteArray result("Seite");
      if (!singular) {
         result += "n";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool, bool singular) {
      QByteArray result("Element");
      if (!singular) {
         result += "e";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Global" : "global")); // FIXME
      if (!singular) {
         result += "";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7-20010524
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages.
    */
   virtual QByteArray trAuthor(bool, bool singular) {
      QByteArray result("Autor");
      if (!singular) {
         result += "en";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Benutzt";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Implementiert " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Implementiert in " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Inhaltsverzeichnis";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Veraltete Elemente";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Ereignisse";
   }

   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Ereignisdokumentation";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Pakettypen";
   }

   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Paketfunktionen";
   }

   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Statische Paketfunktionen";
   }

   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Paketattribute";
   }

   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Statische Paketattribute";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Alle";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Hier ist ein Graph, der zeigt, was diese Funktion aufruft:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Suchergebnisse";
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
         return "Es wurden keine Dokumente zu Ihrer Suchanfrage gefunden.";
      } else if (numDocuments == 1) {
         return "Es wurde <b>1</b> Dokument zu Ihrer Suchanfrage gefunden.";
      } else {
         return "Es wurden <b>$num</b> Dokumente zu Ihrer Suchanfrage "
                "gefunden. Die besten Treffer werden zuerst angezeigt.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Treffer:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return filename + " Quellcode";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Verzeichnishierarchie";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Verzeichnisdokumentation";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Verzeichnisse";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Diese Verzeichnishierarchie ist -mit Einschränkungen- "
             "alphabetisch sortiert:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += "-Verzeichnisreferenz";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool, bool singular) {
      QByteArray result("Verzeichnis");
      if (!singular) {
         result += "se";
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
      return "Dies ist eine überladene Methode, die der Bequemlichkeit "
             "wegen bereitgestellt wird. Sie unterscheidet sich von der "
             "obigen Funktion nur in den Argumenten die sie unterstützt.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Hier ist ein Graph der zeigt, wo diese Funktion aufgerufen wird:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Dokumentation der Aufzählungswerte";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Elementfunktionen/Unterroutinen-Dokumentation";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Datentyp-Liste";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Datenfelder";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Hier folgen die Datentypen mit Kurzbeschreibungen:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Hier folgt eine Liste aller ";
      if (!extractAll) {
         result += "dokumentierten ";
      }
      result += "Datentypelemente";
      result += " mit Links ";
      if (!extractAll) {
         result += "zur Datenstruktur-Dokumentation für jedes Element";
      } else {
         result += "zu den Datentypen, zu denen sie gehören:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Datentyp-Index";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Datentyp-Dokumentation";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Funktionen/Unterroutinen";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Funktionen/Unterroutinen-Dokumentation";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Datentypen";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Modulliste";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Hier folgt eine Liste aller ";
      if (!extractAll) {
         result += "dokumentierten ";
      }
      result += "Module mit ihren Kurzbeschreibungen:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      result += "-";
      switch (compType) {
         case ClassDef::Class:
            result += "Modul";
            break;
         case ClassDef::Struct:
            result += "Typ";
            break;
         case ClassDef::Union:
            result += "Union";
            break;
         case ClassDef::Interface:
            result += "Interface";
            break;
         case ClassDef::Protocol:
            result += "Protokoll";
            break;
         case ClassDef::Category:
            result += "Kategorie";
            break;
         case ClassDef::Exception:
            result += "Ausnahmen";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += "-Template";
      }
      result += "-Referenz";
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += "-Modul-Referenz";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Modul-Elemente";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Hier folgt eine Liste aller ";
      if (!extractAll) {
         result += "dokumentierten ";
      }
      result += "Modulelemente mit Links ";
      if (extractAll) {
         result += "zur Moduldokumentation für jedes Element:";
      } else {
         result += "zu den Modulen, zu denen sie gehören:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Modul-Index";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool /*first_capital*/, bool singular) {
      QByteArray result("Modul");
      if (!singular) {
         result += "e";
      }
      return result;
   }

   /*! This is put at the bottom of a module documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFilesFortran(ClassDef::CompoundType compType,
         bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"Die Dokumentation für ";
      switch (compType) {
         case ClassDef::Class:
            result += "dieses Modul";
            break;
         case ClassDef::Struct:
            result += "diesen Typ";
            break;
         case ClassDef::Union:
            result += "diese Union";
            break;
         case ClassDef::Interface:
            result += "dieses Interface";
            break;
         case ClassDef::Protocol:
            result += "dieses Protokoll";
            break;
         case ClassDef::Category:
            result += "diese Kategorie";
            break;
         case ClassDef::Exception:
            result += "diese Ausnahme";
            break;
         default:
            break;
      }
      if (single) {
         result += " wurde aus der folgenden Datei erzeugt:";
      } else {
         result += " wurde aus den folgenden Dateien erzeugt:";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool /*first_capital*/, bool singular) {
      QByteArray result("Typ");
      if (!singular) {
         result += "en";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool /*first_capital*/, bool singular) {
      QByteArray result("Unterprogramm");
      if (!singular) {
         result += "e";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Type Constraints";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return "Bezug " + QByteArray(name);
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Lade ...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Globaler Namensbereich";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Suche ...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Keine Treffer";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Datei in " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Include-Dateien in " + name;
   }

   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Literaturverzeichnis";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Diagramm der Verzeichnisabhängigkeiten für ") + name + ":";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.0
   //////////////////////////////////////////////////////////////////////////

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "Detailebene";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Template-Parameter";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "und " + number + " mehr ...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "Die Dokumentation für diesen enum wurde aus ";
      if (single) {
         result += "der folgenden Datei";
      } else {
         result += "den folgenden Dateien";
      }
      result += " generiert:";
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return QByteArray(name) + " Enum-Referenz";
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " geerbt von " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Weitere Geerbte Elemente";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a tooltip for the toggle button thatappears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      QByteArray opt = enable ? "einzuschalten" : "auszuschalten";
      return "Klicken um Panelsynchronisation " + opt;
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "Bestandteil der Kategorie @1.";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "Erweitert Klasse @1.";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "Klassenmethoden";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "Instanzmethoden";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "Methodendokumentation";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      return "Übersicht";
   }

   //////////////////////////////////////////////////////////////////////////

};

#endif
