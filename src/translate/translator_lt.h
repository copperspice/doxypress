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

#ifndef TRANSLATOR_LT_H
#define TRANSLATOR_LT_H


class TranslatorLithuanian : public TranslatorAdapter_1_4_6
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
      return "lithuanian";
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
      return "\\usepackage[T2A]{fontenc}\n"
             "\\usepackage[lithuanian]{babel}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Susiję Funkcijos";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "Atkreipkite dėmesį, čią ne metodai";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Smulkus aprašymas";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Tipo Aprašymo Dokumentacija";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Išvardinimo Dokumentacija";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Metodų Dokumentacija";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Lauko aprašymas";
      } else {
         return "Atributų Dokumentacija";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Daugiau...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Visų narių sąrašas";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Narių sąrašas";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Tai galutinis narių sąrašas ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", įtraukiant visus paveldėtus narius.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Automatiškai sugeneruota Doxygen įrankiu";
      if (s) {
         result += (QByteArray)" " + s;
      }
      result += " iš programos kodo.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "išvardinimo vardas";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "išvardinimo reikšmė";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "apibrėžta";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Moduliai";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Klasių hierarchija";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config::getBool("optimize-c")) {
         return "Duomenų Struktūros";
      } else {
         return "Klasės";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Failai";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config::getBool("optimize-c")) {
         return "Duomenų Laukai";
      } else {
         return "Klasių Nariai";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config::getBool("optimize-c")) {
         return "Globalūs Nariai";
      } else {
         return "Failų Nariai";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Susiję Puslapiai";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Pavyzdžiai";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Paieška";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Šis paveldėjimo sąrašas yra beveik surikiuotas "
             "abėcėlės tvarka:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Visų ";
      if (!extractAll) {
         result += "dokumentuotų ";
      }
      result += "failų sąrašas su trumpais aprašymais:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config::getBool("optimize-c")) {
         return "Duomenų struktūros su trumpais aprašymais:";
      } else {
         return "Klasės, struktūros, "
                "sąjungos ir sąsajos su trumpais aprašymais:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Visų ";
      if (!extractAll) {
         result += "dokumentuotų ";
      }
      if (Config::getBool("optimize-c")) {
         result += "struktūrų ir sąjungų laukų sąrašas";
      } else {
         result += "klasių nariai";
      }
      result += " su ryšiais į ";
      if (!extractAll) {
         if (Config::getBool("optimize-c")) {
            result += "struktūrų/sąjungų kiekvieno lauko dokumentaciją:";
         } else {
            result += "klasės dokumentacija kiekvienam nariui:";
         }
      } else {
         if (Config::getBool("optimize-c")) {
            result += "struktūros/sąjungos, kurios priklauso:";
         } else {
            result += "klasės, kurios priklauso:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Sąrašas visų ";
      if (!extractAll) {
         result += "dokumentuotų ";
      }

      if (Config::getBool("optimize-c")) {
         result += "funkcijų, kintamųjų, apibrėžimų, išvardinimų, ir tipų apibrėžimų";
      } else {
         result += "globalių narių";
      }
      result += " su ryšiais į ";
      if (extractAll) {
         result += "failus, kuriems jie priklauso:";
      } else {
         result += "dokumentacija:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Visų pavyzdžių sąrašas:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Visų susijusių dokumentacijos puslapių sąrašas:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Visų modulių sąrašas:";
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
      return "Modulio Indeksas";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Hierarchijos Indeksas";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config::getBool("optimize-c")) {
         return "Duomenų Struktūros Indeksas";
      } else {
         return "Klasės Indeksas";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Failo Indeksas";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Modulio Dokumentacija";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Duomenų Struktūros Dokumentacija";
      } else {
         return "Klasės Dokumentacija";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Failo Dokumentacija";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Pavyzdžio Dokumentacija";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Puslapio Dokumentacija";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Informacinis Vadovas";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Apibrėžimai";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Tipų apibrėžimai";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Išvardinimai";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Funkcijos";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Kintamieji";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Išvardinimų reikšmės";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Apibrėžimų Dokumentacija";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Tipų apibrėžimų Dokumentacija";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Išvardinimo Tipo Dokumentacija";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Funkcijos Dokumentacija";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Kintamojo Dokumentacija";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config::getBool("optimize-c")) {
         return "Duomenų struktųros";
      } else {
         return "Klasės";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Sugeneruota " + date; /*FIXME*/
      if (projName) {
         result += (QByteArray)" " + projName;   /*FIXME*/
      }
      result += (QByteArray)" "; /*FIXME*/
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Paveldimumo diagrama " + clName + ":"; /*FIXME*/
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Tiktai vidiniam naudojimui.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Įspėjimas";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Versija";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Data";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Gražina";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Taip pat žiūrėti";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Parametrai";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Išimtys";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Sugeneruota";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Varų Srities Sąrašas";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Sąrašas visų ";
      if (!extractAll) {
         result += "dokumentuotų ";
      }
      result += "vardų sričių su trumpais aprašymais:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Draugai";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Draugiškų Ir Susijusių Funkcijų Dokumentacija";
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
            result += " Klasė";
            break;
         case ClassDef::Struct:
            result += " Struktūra";
            break;
         case ClassDef::Union:
            result += " Sąjunga";
            break;
         case ClassDef::Interface:
            result += " Sąsaja";
            break;
         case ClassDef::Protocol:
            result += " Protokolas";
            break;
         case ClassDef::Category:
            result += " Kategorija";
            break;
         case ClassDef::Exception:
            result += " Išimtis";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += " Šablonas";
      }
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = fileName;
      result += " Failo Nuoroda";
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Vardų Srities Nuoroda";
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Vieši Metodai";
   }
   virtual QByteArray trPublicSlots() {
      return "Vieši Slotai";   /*FIXME*/
   }
   virtual QByteArray trSignals() {
      return "Signalai";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Statiniai Vieši Metodai";
   }
   virtual QByteArray trProtectedMembers() {
      return "Apsaugoti Metodai";
   }
   virtual QByteArray trProtectedSlots() {
      return "Apsaugoti Slotai";   /*FIXME*/
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Statiniai Apsaugoti Metodai";
   }
   virtual QByteArray trPrivateMembers() {
      return "Privatatūs Metodai";
   }
   virtual QByteArray trPrivateSlots() {
      return "Privatūs Slotai";   /*FIXME*/
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Statiniai Privatūs Metodai";
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
               result += ", ir ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Paveldi " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Paveldėta " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Perkrauna metodą iš " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Metodas perkraunamas " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Vardų Srities Nariai";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Sąrašas visų ";
      if (!extractAll) {
         result += "dokumentuotų ";
      }
      result += "vardų srities narių su nuorodomis į ";
      if (extractAll) {
         result += "vardų srities dokumentaciją kiekvienam nariui:";
      } else {
         result += "vardų sritis, kurioms jos priklauso:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Vardų Srities Indeksas";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Vardų Srities Dokumentacija";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Vardų Sritys";
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
            result += "šiai klasei";
            break;
         case ClassDef::Struct:
            result += "šiai struktūrai";
            break;
         case ClassDef::Union:
            result += "šiai sąjungai";
            break;
         case ClassDef::Interface:
            result += "šiai sąsajai";
            break;
         case ClassDef::Protocol:
            result += "šiam protocolui";
            break;
         case ClassDef::Category:
            result += "šiai kategorijai";
            break;
         case ClassDef::Exception:
            result += "šiai išimčiai";
            break;
         default:
            break;
      }
      result += " sugeneruota iš ";
      if (single) {
         result += "šio failo:";
      } else {
         result += "šių failų:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Gražinamos reikšmės";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Pagrindinis Puslapis";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "psl.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Apibrėžimas failo @1 eilutėje @0.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Apibrėžimas faile @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Smerktina";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Bendradarbiavimo diagrama " + clName + ":"; /*FIXME*/
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Įtraukimo priklausomybių diagrama " + fName + ":"; /*FIXME*/
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Konstruktoriaus ir Destruktoriaus Dokumentacija";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Eiti į šio failo programos kodą.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Eiti į šio failo dokumentaciją.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Išakstinė sąlyga";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Postcondition";/*FIXME*/
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Pastovus";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Pradinė reikšmė:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "tekstas";/*FIXME*/
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Grafinė Klasės Hierarchija";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Eiti į grafinę klasės hierarchiją";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Eiti į tekstinę klasės hierarchiją";
   }
   virtual QByteArray trPageIndex() {
      return "Puslapio Indeksas";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Pastaba";
   }
   virtual QByteArray trPublicTypes() {
      return "Vieši Tipai";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config::getBool("optimize-c")) {
         return "Duomenų Laukai";
      } else {
         return "Vieši Atributai";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Statiniai Vieši Atributai";
   }
   virtual QByteArray trProtectedTypes() {
      return "Apsaugoti Tipai";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Apsaugoti Atributai";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Statiniai Apsaugoti Atributai";
   }
   virtual QByteArray trPrivateTypes() {
      return "Privatūs Tipai";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Privatūs Atributai";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Statiniai Privatūs Atributai";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "Daryti";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Tolimesni Darbai";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Naudojamas";
   }
   virtual QByteArray trRemarks() {
      return "Pastabos";
   }
   virtual QByteArray trAttention() {
      return "Attention";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Šis grafas rodo, kuris failas tiesiogiai ar "
             "netiesiogiai įtraukia šį failą:";
   }
   virtual QByteArray trSince() {
      return "Nuo";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Grafo Legenda";
   }
   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Šis puslapis paaiškina, kaip interpretuoti sugeneruotus grafus "
         "su Doxygen įrankiu.<p>\n"
         "Panagrinėkite pavyzdį:\n"
         "\\kodo pradžia\n"
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
         "If the \\c MAX_DOT_GRAPH_HEIGHT tag in the configuration file "
         "is set to 240 this will result in the following graph:"
         "<p><center><img alt=\"\" src=\"graph_legend." + Config::getEnum("dot-image-format").toUtf8() + "\"></center>\n"
         "<p>\n"
         "The boxes in the above graph have the following meaning:\n"
         "<ul>\n"
         "<li>%A filled black box represents the struct or class for which the "
         "graph is generated.\n"
         "<li>%A box with a black border denotes a documented struct or class.\n"
         "<li>%A box with a grey border denotes an undocumented struct or class.\n"
         "<li>%A box with a red border denotes a documented struct or class for"
         "which not all inheritance/containment relations are shown. %A graph is "
         "truncated if it does not fit within the specified boundaries.\n"
         "</ul>\n"
         "The arrows have the following meaning:\n"
         "<ul>\n"
         "<li>%A dark blue arrow is used to visualize a public inheritance "
         "relation between two classes.\n"
         "<li>%A dark green arrow is used for protected inheritance.\n"
         "<li>%A dark red arrow is used for private inheritance.\n"
         "<li>%A purple dashed arrow is used if a class is contained or used "
         "by another class. The arrow is labeled with the variable(s) "
         "through which the pointed class or struct is accessible.\n"
         "<li>%A yellow dashed arrow denotes a relation between a template instance and "
         "the template class it was instantiated from. The arrow is labeled with "
         "the template parameters of the instance.\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "legenda";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "Testas";
   }
   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "Testo Sąrašas";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Savybės";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Savybės Dokumentacija";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config::getBool("optimize-c")) {
         return "Duomenų Struktūros";
      } else {
         return "Klasės";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Paketas " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Paketo Sąrašas";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Paketai su trumpu aprašymu:";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Paketai";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Reikšmė:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Klaida";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Klaidų Sąrašas";
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
      return "1257";
   }


   /*! Used as ansicpg for RTF fcharset
    *  \see trRTFansicp() for a table of possible values.
    */
   virtual QByteArray trRTFCharSet() {
      return "186";
   }

   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "Indeksas";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Klasė" : "klasė"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Faila" : "faila"));
      if (!singular) {
         result += "i";
      } else {
         result += "s";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Vardų srit" : "vardų srit"));
      if (!singular) {
         result += "ys";
      } else {
         result += "is";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Grupė" : "grupė"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Puslapi" : "puslapi"));
      if (!singular) {
         result += "ai";
      } else {
         result += "s";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Nar" : "nar"));
      if (!singular) {
         result += "iai";
      } else {
         result += "ys";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Global" : "global"));
      if (!singular) {
         result += "ūs";
      } else {
         result += "us";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Autori" : "autori"));
      if (!singular) {
         result += "ai";
      } else {
         result += "us";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Ryšiai";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Realizuoja " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Realizuota " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Turinys";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Smerktinumų Sąrašas";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Įvykiai";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Įvykio Dokumentacija";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Paketo Tipai";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Paketo Funkcijos";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Statinės Paketo Funkcijos";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Paketo Atributai";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Statiniai Paketo Atributai";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Visi";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Funkcijos kvietimo grafas:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Paieškos Rezultatai";
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
         return "Atsiprašome, jokių dokumentų nerasta pagal Jūsų užklausą.";
      } else if (numDocuments == 1) {
         return "Surasta <b>1</b> dokumentas, atitinkantis Jūsų užklausą.";
      } else {
         return "Surasta <b>$num</b> dokumentų, atitinkančių Jūsų užklausą. "
                "Pirmiausiai rodomi labiausiai tenkinantys užklausą.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Atitikmenys:";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return filename + " išeities tekstas";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Direktorijų hierarchija";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Direktorijų dokumentacija";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Direktorijos";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Ši direktorjų strūktūra grubiai surikiuota abėcėlės tvarka:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " Directorijos aprašas";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Direktorij" : "direktorij"));
      if (singular) {
         result += "a";
      } else {
         result += "os";
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
      return "Perkraunamas metodas sukurtas patogumui. "
             "Jis skiriasi nuo aukščiau minėto tik argumetais.";
   }


};

#endif
