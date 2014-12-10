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

#ifndef TRANSLATOR_FI_H
#define TRANSLATOR_FI_H

class TranslatorFinnish : public TranslatorAdapter_1_6_0
{
 public:
   /*! This method is used to generate a warning message to signal
    *  the user that the translation of his/her language of choice
    *  needs updating.
    */
   /*virtual QByteArray updateNeededMessage()
   {
     return "The Finnish translator is really obsolete.\n"
            "It was not updated since version 1.0.0.  As a result,\n"
            "some sentences may appear in English.\n\n";
   }*/

   // --- Language control methods -------------------

   /*! Used for identification of the language. The identification
    * should not be translated. It should be replaced by the name
    * of the language in English using lower-case characters only
    * (e.g. "czech", "japanese", "russian", etc.). It should be equal to
    * the identification used in language.cpp.
    */
   virtual QByteArray idLanguage() {
      return "finnish";
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
      return "\\usepackage[finnish]{babel}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Liittyvät funktiot";   // "Related Functions";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Huomaa, että nämä eivät ole jäsenfunktioita.)";   // "(Note that these are not member functions.)"
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Yksityiskohtainen selite";   // "Detailed Description"
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation()
   // header that is put before the list of typedefs.
   {
      return "Jäsentyyppimäärittelyiden dokumentaatio";   // "Member Typedef Documentation"
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Jäsenluettelotyyppien dokumentaatio";   // "Member Enumeration Documentation"
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Jäsenfunktioiden dokumentaatio";   // "Member Function Documentation"
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Kenttien dokumentaatio"; // "Field Documentation";
      } else {
         return "Jäsendatan dokumentaatio"; // "Member Data Documentation"
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Lisää...";   // "More..."
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Lista kaikista jäsenistä";   // "List of all members."
   }
   virtual QByteArray trMemberList()

   /*! used as the title of the "list of all members" page of a class */
   {
      return "Jäsenlista";   // "Member List"
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Tämä on lista kaikista jäsenistä luokassa ";   // "This is the complete list of members for "
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", sisältäen kaikki perityt jäsenet.";   // ", including all inherited members."
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = (QByteArray)"Automaattisesti luotu Doxygenilla "
                          "lähdekoodista projektille " + s; // "Generated automatically by Doxygen" ... "for" ... "from the sourcecode"
      //if (s) result+=(QByteArray)" voor "+s;
      // tässä on ongelma, kuinka taivuttaa parametria, esim. "Jcad"+"in"; "IFC2VRML konversio"+"n"
      // mutta ratkaistaan ongelma kätevästi kaksoispisteellä -> "Jcad:n" / "IFC2VRML konversio:n"
      // lopputulos on vähemmän kökkö ja täysin luettava, mutta ei kuitenkaan täydellinen.
      //
      // kierretään ongelma taivuttamalla sanaa projekti :)
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "luettelotyypin nimi";   // "enum name"
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "luettelotyypin arvo";   // "enum value"
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "määritelty";   // "defined in"
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Moduulit";   // "Modules"
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Luokkahierarkia";   // "Class Hierarchy"
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Tietueet"; // "Data Structures"
      } else {
         return "Luokkalista"; // "Class List"
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Tiedostolista";   // "File List"
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Tietueen kentät"; // "Data Fields"
      } else {
         return "Luokan jäsenet"; // "Class Members"
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Globaalit"; // "Globals"
      } else {
         return "Tiedoston jäsenet"; // "File Members"
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Liittyvät sivut";   // "Related Pages"
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Esimerkit";   // "Examples"
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Etsi";   // "Search"
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Tämä periytymislista on päätasoltaan aakkostettu " // "This inheritance list is sorted roughly, "
             "mutta alijäsenet on aakkostettu itsenäisesti:"; // "but not completely, alphabetically:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Tässä on lista kaikista ";
      if (!extractAll) {
         result += "dokumentoiduista ";   // "documented "
      }
      result += "tiedostoista lyhyen selitteen kanssa:"; // "files with brief descriptions:"
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Tässä ovat tietueet lyhyen selitteen kanssa:"; // "Here are the data structures with brief descriptions:"
      } else {
         return "Tässä ovat luokat, tietueet ja " // "Here are the classes, structs and "
                "yhdisteet lyhyen selitteen kanssa:"; // "unions with brief descriptions:"
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Tässä on lista kaikista "; // "Here is a list of all "
      if (!extractAll) {
         result += "dokumentoiduista "; // "documented "
      }
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "tietuiden ja yhdisteiden kentistä"; // "struct and union fields"
      } else {
         result += "luokkien jäsenistä"; // "class members"
      }
      result += " linkitettyinä "; // " with links to "
      if (!extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "jokaisen kentän tietueen/yhdisteen dokumentaatioon:"; // "the struct/union documentation for each field:"
         } else {
            result += "jokaisen jäsenen luokkadokumentaatioon:"; // "the class documentation for each member:"
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "tietueisiin/yhdisteisiin, joihin ne kuuluvat:"; // "the structures/unions they belong to:"
         } else {
            result += "luokkiin, joihin ne kuuluvat"; //"the classes they belong to:"
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Tässä on lista kaikista "; // "Here is a list of all "
      if (!extractAll) {
         result += "dokumentoiduista ";   // "documented "
      }

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "funktioista, muuttujista, määrittelyistä, luettelotyypeistä ja tyyppimäärittelyistä"; // "functions, variables, defines, enums, and typedefs"
      } else {
         result += "tiedoston jäsenistä"; // "file members"
      }
      result += " linkitettyinä "; // " with links to "
      if (extractAll) {
         result += "tiedostoihin, joihin ne kuuluvat:";   // "the files they belong to:"
      } else {
         result += "dokumentaatioon:";   // "the documentation:"
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Tässä on lista kaikista esimerkeistä:";   //  "Here is a list of all examples:"
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Tässä on lista kaikista liittyvistä dokumentaatiosivuista:";   // "Here is a list of all related documentation pages:"
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Tässä on lista kaikista moduleista:";   // "Here is a list of all modules:"
   }

   // index titles (the project name is prepended for these)

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Dokumentaatio";   // "Documentation"
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Moduuliluettelo";   // "Module Index"
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Hierarkinen luettelo";   // "Hierarchical Index"
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Tietueluettelo"; // "Data Structure Index"
      } else {
         return "Luokkaluettelo"; // "Class Index"
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Tiedostoluettelo";   // "File Index"
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Moduulien dokumentaatio";   // "Module Documentation"
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Tietueiden dokumentaatio"; // "Data Structure Documentation"
      } else {
         return "Luokkien dokumentaatio"; // "Class Documentation"
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Tiedostojen dokumentaatio";   // "File Documentation"
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Esimerkkien dokumentaatio";   // "Example Documentation"
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Sivujen dokumentaatio";   // "Page Documentation"
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Käsikirja";   // "Reference Manual"
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Määrittelyt";   // "Defines"
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Tyyppimäärittelyt";   // "Typedefs"
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Luettelotyypit";   // "Enumerations"
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Funktiot";   // "Functions"
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Muuttujat";   // "Variables"
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Luettelotyyppien arvot";   // "Enumerator"
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Määritysten dokumentointi";   // "Define Documentation"
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Tyyppimääritysten dokumentaatio";   // "Typedef Documentation"
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Luettelotyyppien dokumentaatio";   // "Enumeration Type Documentation"
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Funktioiden dokumentaatio";   // "Function Documentation"
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Muuttujien dokumentaatio";   // "Variable Documentation"
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Tietueet"; // "Data Structures"
      } else {
         return "Luokat"; // "Classes"
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      // funktio on hiukan vaikea kääntää prepositioihin sidotun rakenteen vuoksi.
      // Muutetaan siis lauserakennetta suomalaisemmaksi
      // Generated on $date for $project by:
      // -> Generated for $project on $date by:
      QByteArray result = (QByteArray)"Generoinut ";
      if (projName) {
         result += (QByteArray)"projektille " + projName + " ";
      }
      result += (QByteArray)date + " ";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return "Luokan " + (QByteArray)clName + " luokkakaavio"; // "Inheritance diagram for "
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Vain sisäiseen käyttöön.";   // "For internal use only."
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Varoitus";   // "Warning"
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Versio";   // "Version"
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Päiväys";   // "Date"
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Palauttaa";   // "Returns"
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Katso myös";   // "See also"
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Parametrit";   // "Parameters"
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Poikkeukset";   // "Exceptions"
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Generoinut";   // "Generated by"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Nimiavaruus Lista";   // "Namespace List"
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Tässä on lista kaikista "; // "Here is a list of all "
      if (!extractAll) {
         result += "dokumentoiduista ";   // "documented "
      }
      result += "nimiavaruuksista lyhyen selitteen kanssa:"; // "namespaces with brief descriptions:"
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Ystävät";   // "Friends"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Ystävät ja niihin liittyvien funktioiden dokumentaatio";   // "Friends And Related Function Documentation"
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
            result += " luokka";
            break; // " Class"
         case ClassDef::Struct:
            result += " tietue";
            break; // " Struct"
         case ClassDef::Union:
            result += " yhdiste";
            break; // " Union"
         case ClassDef::Interface:
            result += " rajapinta";
            break; // " Interface"
         case ClassDef::Protocol:
            result += " protokolla";
            break; // " Protocol"
         case ClassDef::Category:
            result += " kategoria";
            break; // " Category"
         case ClassDef::Exception:
            result += " poikkeus";
            break; // " Exception"
         default:
            break;
      }
      if (isTemplate) {
         result += "malli";   // " Template"
      }
      result += "referenssi"; // " Reference"
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = fileName;
      result += " tiedostoreferenssi"; // " File Reference"
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " nimiavaruusreferenssi"; // " Namespace Reference"
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Julkiset jäsenfunktiot";   // "Public Member Functions"
   }
   virtual QByteArray trPublicSlots() {
      return "Julkiset vastineet";   // "Public Slots"
   }
   virtual QByteArray trSignals() {
      return "Signaalit";   // "Signals"
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Staattiset julkiset jäsenfunktiot";   // "Static Public Member Functions"
   }
   virtual QByteArray trProtectedMembers() {
      return "Suojatut jäsenfunktiot";   // "Protected Member Functions"
   }
   virtual QByteArray trProtectedSlots() {
      return "Suojatut vastineet";   // "Protected Slots"
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Staattiset suojatut jäsenfunktiot";   // "Static Protected Member Functions"
   }
   virtual QByteArray trPrivateMembers() {
      return "Yksityiset jäsenfunktiot";   // "Private Member Functions"
   }
   virtual QByteArray trPrivateSlots() {
      return "Yksityiset vastineet";   // "Private Slots"
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Staattiset yksityiset jäsenfunktiot";   // "Static Private Member Functions"
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
               result += " ja ";   // ", and "
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return (QByteArray)"Periytyy " + (numEntries > 1 ? "luokista " : "luokasta ") + trWriteList(numEntries) + "."; // "Inherits "
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return (QByteArray)"Periytetään " + (numEntries > 1 ? "luokissa " : "luokassa ") + trWriteList(numEntries) + "."; // "Inherited by "
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return (QByteArray)"Uudelleentoteuttaa " + (numEntries > 1 ? "luokat " : "luokan  ") + trWriteList(numEntries) + "."; // "Reimplemented from "
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return (QByteArray)"Uudelleentoteutetaan " + (numEntries > 1 ? "luokissa " : "luokassa ") + trWriteList(numEntries) + "."; // "Reimplemented in "
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Nimiavaruuden jäsenet";   // "Namespace Members"
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Tässä on lista kaikista "; // "Here is a list of all "
      if (!extractAll) {
         result += "dokumentoiduista ";   // "documented "
      }
      result += "nimiavaruuden jäsenistä linkitettynä "; // "namespace members with links to "
      if (extractAll) {
         result += "nimiavaruuden dokumentaatioon johon ne kuuluvat:";   // "the namespace documentation for each member:";
      } else {
         result += "nimiavaruuksiin joihin ne kuuluvat:";   // "the namespaces they belong to:"
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Nimiavaruuksien luettelo";   // "Namespace Index"
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Nimiavaruuden dokumentaatio";   // "Namespace Documentation"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Nimiavaruudet";
   } // "Namespaces"; }

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
      QByteArray result = (QByteArray)"Dokumentaatio tälle "; // "The documentation for this "
      switch (compType) {
         case ClassDef::Class:
            result += "luokalle";
            break; // "class"
         case ClassDef::Struct:
            result += "tietueelle";
            break; // "struct"
         case ClassDef::Union:
            result += "yhdisteelle";
            break; // "union"
         case ClassDef::Interface:
            result += "rajapinnalle";
            break; // "interface"
         case ClassDef::Protocol:
            result += "protokollalle";
            break; // "protocol"
         case ClassDef::Category:
            result += "kategorialle";
            break; // "category"
         case ClassDef::Exception:
            result += "poikkeukselle";
            break; // "exception"
         default:
            break;
      }
      if (single) {
         result += " luotiin seuraavasta tiedostosta:";   // " was generated from the following file"
      } else {
         result += " luotiin seuraavista tiedostoista:";   // ":" or "s:"
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Paluuarvot";   // "Return values"
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Pääsivu";   // "Main Page"
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "s.";   // "p."
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Määrittely tiedoston @1 rivillä @0."; //  "Definition at line @0 of file @1."
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Määrittely tiedostossa @0."; // "Definition in file @0."
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Vanhentunut"; // "Deprecated"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Yhteistyökaavio luokalle " + clName + ":"; // "Collaboration diagram for "+clName+":"
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Sisällytysriippuvuuskaavio tiedostolle " + fName + ":"; // "Include dependency graph for "+fName+":"
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Rakentajien & purkajien dokumentaatio"; // "Constructor & Destructor Documentation";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Siirry tämän tiedoston lähdekoodiin."; // "Go to the source code of this file."
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Siirry tämän tiedoston dokumentaatioon."; // "Go to the documentation of this file."
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Esiehto"; //"Precondition"
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Jälkiehto"; // "Postcondition"
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Invariantti"; // vai "Pysyväisväittämä"? "Invariant"
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Alkuarvo:"; // "Initial value:"
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "koodi"; // "code"
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Graafinen luokkahierarkia"; // "Graphical Class Hierarchy"
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Siirry graafiseen luokkahierarkiaan"; // "Go to the graphical class hierarchy"
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Siirry tekstimuotoiseen luokkahierarkiaan"; // "Go to the textual class hierarchy"
   }
   virtual QByteArray trPageIndex() {
      return "Sivuhakemisto"; // "Page Index"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      // FIXME: Missähän merkityksessä tätä käytetään?
      return "Huomautus"; // "Note"
   }
   virtual QByteArray trPublicTypes() {
      return "Julkiset tyypit"; // "Public Types"
   }
   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Tietueen kentät"; // "Data Fields"
      } else {
         return "Julkiset attribuutit"; // "Public Attributes"
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Staattiset julkiset attribuutit"; // "Static Public Attributes"
   }
   virtual QByteArray trProtectedTypes() {
      return "Suojellut tyypit"; // "Protected Types"
   }
   virtual QByteArray trProtectedAttribs() {
      return "Suojellut attribuutit"; // "Protected Attributes"
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Staattiset suojellut attribuutit"; // "Static Protected Attributes"
   }
   virtual QByteArray trPrivateTypes() {
      return "Yksityiset tyypit"; // "Private Types"
   }
   virtual QByteArray trPrivateAttribs() {
      return "Yksityiset attribuutit"; // "Private Attributes"
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Staattiset yksityiset attribuutit"; // "Static Private Attributes"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "Vielä tehtävänä"; // "Todo"
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Tehtävälista"; // "Todo List"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Viitattu"; // "Referenced by"
   }
   virtual QByteArray trRemarks() {
      return "Huomioita"; // "Remarks"
   }
   virtual QByteArray trAttention() {
      return "Huomio"; // "Attention"
   }
   virtual QByteArray trInclByDepGraph() {
      return "Tämä kaavio näyttää, mitkä tiedostot suorasti "
             "tai epäsuorasti sisällyttävät tämän tiedoston";
      // "This graph shows which files directly or "
      // "indirectly include this file:"
   }
   virtual QByteArray trSince() {
      // FIXME: Missä merkityksessä tätä käytetään?
      return "Lähtien"; // "Since"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Kaavion selite"; // "Graph Legend"
   }
   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Tämä sivu selittää, kuinka doxygenin luomia kaavioita tulkitaan.<p>\n"
         // "This page explains how to interpret the graphs that are generated "
         // "by doxygen.<p>\n"
         "Ajattele seuraavaa esimerkkiä:\n"
         // "Consider the following example:\n"
         "\\code\n"
         "/*! Näkymätön luokka katkaisun vuoksi */\n"
         // "/*! Invisible class because of truncation */\n"
         "class Nakymaton { };\n\n"
         // "class Invisible { };\n\n"
         "/*! Katkaistu luokka, periytymissuhde on piilotettu */\n"
         // "/*! Truncated class, inheritance relation is hidden */\n"
         "class Katkaistu : public Nakymaton { };\n\n"
         // "class Truncated : public Invisible { };\n\n"
         "/* Luokkaa ei ole dokumentoitu doxygen-kommenteilla */\n"
         // "/* Class not documented with doxygen comments */\n"
         "class Dokumentoimaton { };\n\n"
         // "class Undocumented { };\n\n"
         "/*! Julkista periyttämistä käyttäen periytetty luokka */\n"
         // "/*! Class that is inherited using public inheritance */\n"
         "class JulkinenKanta : public Katkaistu { };\n\n"
         // "class PublicBase : public Truncated { };\n\n"
         "/*! Malliluokka */\n"
         // "/*! A template class */\n"
         "template<class T> class Malli { };\n\n"
         // "template<class T> class Templ { };\n\n"
         "/*! Suojeltua periytymistä käyttäen periytetty luokka */\n"
         // "/*! Class that is inherited using protected inheritance */\n"
         "class SuojeltuKanta { };\n\n"
         // "class ProtectedBase { };\n\n"
         "/*! Yksityistä periytymistä käyttäen periytetty luokka */\n"
         // "/*! Class that is inherited using private inheritance */\n"
         "class YksityisKanta { };\n\n"
         // "class PrivateBase { };\n\n"
         "/*! Luokka jota periytetty luokka käyttää */\n"
         // "/*! Class that is used by the Inherited class */\n"
         "class Kaytetty { };\n\n"
         // "class Used { };\n\n"
         "/*! Kantaluokka joka periytyy useasta muusta luokasta */\n"
         // "/*! Super class that inherits a number of other classes */\n"
         "class Periytetty : public JulkinenKanta,\n"
         "                 : protected SuojeltuKanta,\n"
         "                 : private YksityisKanta,\n"
         "                 : public Dokumentoimaton,\n"
         "                 : public Malli<int>\n"
         "{\n"
         "  private:\n"
         "    Kaytetty *m_kaytettyLuokka;\n"
         "}\n";
      // "class Inherited : public PublicBase,\n"
      // "                  protected ProtectedBase,\n"
      // "                  private PrivateBase,\n"
      // "                  public Undocumented,\n"
      // "                  public Templ<int>\n"
      // "{\n"
      // "  private:\n"
      // "    Used *m_usedClass;\n"
      // "};\n"
      "\\endcode\n"
      "Tuloksena on seuraavanlainen kaavio:"
      //"This will result in the following graph:"
      "<p><center><img alt=\"\" src=\"graph_legend."
      +Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
      "<p>\n"
      "Ylläolevassa kaaviossa laatikoilla on seuraavat merkitykset\n:"
      // "The boxes in the above graph have the following meaning:\n"
      "<ul>\n"
      "<li>Täytetty harmaa laatikko esittää tietuetta tai luokkaa, jolle "
      // "<li>%A filled gray box represents the struct or class for which the "
      "kaavio on luotu.\n"
      // "graph is generated.\n"
      "<li>Mustareunainen laatikko merkitsee dokumentoitua tietuetta tai luokkaa.\n"
      // "<li>%A box with a black border denotes a documented struct or class.\n"
      "<li>Harmaareunainen laatikko merkitsee dokumentoimatonta tietuetta tai luokkaa.\n"
      // "<li>%A box with a grey border denotes an undocumented struct or class.\n"
      "<li>Punareunainen laatikko merkistee dokumentoitua luokkaa tai structia "
      // "<li>%A box with a red border denotes a documented struct or class for"
      "jolle ei näytetä kaikkia periytymis-/sisällyssuhteita. Kaavio "
      // "which not all inheritance/containment relations are shown. %A graph is "
      "katkaistaan, jos se ei mahdu määriteltyjen rajojen sisään.\n"
      // "truncated if it does not fit within the specified boundaries.\n"
      "</ul>\n"
      "Nuolilla on seuraavat merkitykset:\n"
      // "The arrows have the following meaning:\n"
      "<ul>\n"
      "<li>Tummansinistä nuolta käytetään osoittamaan julkista periytymis"
      // "<li>%A dark blue arrow is used to visualize a public inheritance "
      "suhdetta kahden luokan välillä.\n"
      // "relation between two classes.\n"
      "<li>Tummanvihreää nuolta käytetään suojellussa periytymisessä.\n"
      // "<li>%A dark green arrow is used for protected inheritance.\n"
      "<li>Tummanpunaista nuolta käytetään yksityisessä periytymisessä.\n"
      // "<li>%A dark red arrow is used for private inheritance.\n"
      "<li>Purppuranväristä katkoviivaa käytetään, jos luokka sisältyy tai "
      // "<li>%A purple dashed arrow is used if a class is contained or used "
      "on toisen luokan käyttämä. Nuoli nimetään sillä muuttujalla/muuttujilla "
      // "by another class. The arrow is labeled with the variable(s) "
      "jonka läpi osoitettua luokkaa tai tietuetta käytetään.\n"
      // "through which the pointed class or struct is accessible.\n"
      "<li>Keltainen katkoviivalla piirretty nuoli merkitsee suhdetta mallin esiintymän ja "
      // "<li>%A yellow dashed arrow denotes a relation between a template instance and "
      "malliluokan välillä. Nuoli nimetään "
      // "the template class it was instantiated from. The arrow is labeled with "
      "mallin esiintymän malliparametrilla.\n"
      // "the template parameters of the instance.\n"
      "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "selite"; // "legend"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "Testi"; // "Test"
   }
   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "Testilista"; // "Test List"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Ominaisuudet"; // "Properties"
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Ominaisuuksien dokumentaatio"; // "Property Documentation"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Tietueet"; // "Data Structures"
      } else {
         return "Luokat"; // "Classes"
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Paketti " + name; // "Package "
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Pakettilista"; // "Package List"
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Tässä ovat paketit lyhyiden selitysten kanssa (jos saatavilla):"; // "Here are the packages with brief descriptions (if available):"
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Paketit"; // "Packages"
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Arvo:"; // "Value:"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Bugi"; // "Bug"
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Bugilista"; // "Bug List"
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
      return "Hakemisto"; // "Index"
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Luokka" : "luokka")); // "Class" / "class"
      if (!singular) {
         result = (first_capital ? "Luokat" : "luokat");   // "+es" -> "Classes" / "classes"
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Tiedosto" : "tiedosto")); // "File" / "file"
      if (!singular) {
         result += "t";   // "+s" -> "Files" / "files"
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Nimiavaruus" : "nimiavaruus")); // "Namespace" / "namespace"
      if (!singular) {
         result = (first_capital ? "Nimiavaruudet" : "nimiavaruudet");   // "+s"
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Ryhmä" : "ryhmä")); // "Group" / "group"
      if (!singular) {
         result += "t";   // "+s"
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Sivu" : "sivu")); // "Page" / "page"
      if (!singular) {
         result += "t";   // "+s"
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Jäsen" : "jäsen")); // "Member" / "member"
      if (!singular) {
         result += "et";   // "+s"
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Globaali" : "globaali")); // "Global" / "global"
      if (!singular) {
         result += "t";   // "+s"
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Tekijä" : "tekijä")); // "Author" / "author"
      if (!singular) {
         result += "t";   // "+s"
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Viittaukset"; // "References"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Toteuttaa luokan " + trWriteList(numEntries) + "."; // "Implements "
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Toteutettu luokassa " + trWriteList(numEntries) + "."; // "Implemented in "
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Sisällysluettelo"; // "Table of Contents"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Vanhentuneiden lista"; // "Deprecated List"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Tapahtumat"; // "Events"
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Tapahtumien dokumentaatio"; // "Event Documentation"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Paketin tyypit"; // "Package Types"
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Paketin funktiot"; // "Package Functions"
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Paketin staattiset funktiot"; // "Static Package Functions"
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Paketin attribuutit"; // "Package Attributes"
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Paketin staattiset attribuutit"; // "Static Package Attributes"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Kaikki"; // "All"
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Tässä on kutsukaavio tälle funktiolle:"; // "Here is the call graph for this function:"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Hakutulokset"; // "Search Results"
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
         return "Valitettavasti yksikään dokumentti ei vastannut hakuasi."; // "Sorry, no documents matching your query."
      } else if (numDocuments == 1) {
         return "Löytyi <b>1</b> dokumentti, joka vastasi hakuasi."; // "Found <b>1</b> document matching your query.";
      } else {
         return "Löytyi <b>$num</b> dokumenttia, jotka vastasivat hakuasi. " // "Found <b>$num</b> documents matching your query. "
                "Parhaat tulokset näytetään ensin."; // "Showing best matches first."
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Osumat:"; // "Matches:"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return filename + " lähdekooditiedosto"; // " Source File"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Hakemistohierarkia";   // "Directory Hierarchy"
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Hakemistojen dokumentaatio";   // "Directory Documentation"
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Hakemistot";   // "Directories"
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Tämä hakemistohierarkia on järjestetty aakkosellisesti tasoittain:";
      //This directory hierarchy is sorted roughly, "
      // "but not completely, alphabetically:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " hakemistoreferenssi";
      return result;
   }
   // " Directory Reference"

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Hakemisto" : "hakemisto")); // "Director" / "director"
      if (singular) {
         result += "";
      } else {
         result += "t";   // "+y" / "+ies"
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
      return "Tämä on ylikuormitettu jäsenfunktio, ja se tarjotaan "
             "käytön helpottamiseksi. Se eroaa ylläolevasta "
             "funktiosta ainoastaan hyväksymiltään parametreilta.";
      // "This is an overloaded member function, "
      // "provided for convenience. It differs from the above "
      // "function only in what argument(s) it accepts."
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Tässä on kutsukaavio tälle funktiolle:"; // "Here is the caller graph for this function:"
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Luettelotyypin dokumentaatio";   // "Enumerator Documentation"
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Jäsenfunktioiden/aliohjelmien dokumentaatio";   // "Member Function/Subroutine Documentation"
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Tietotyyppien lista";   // "Data Types List"
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Kentät";   // "Data Fields";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Tässä ovat tietotyypit lyhyiden selitysten kanssa:";   // "Here are the data types with brief descriptions:"
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Tässä on lista kaikista "; // "Here is a list of all "
      if (!extractAll) {
         result += "dokumentoiduista "; // "documented "
      }
      result += "tietotyyppien jäsenistä"; // "data types members"
      result += ", sekä linkit "; // " with links to "
      if (!extractAll) {
         result += "tietueen dokumentaatioon jokaiselle jäsenelle"; // "the data structure documentation for each member"
      } else {
         result += "tietotyyppeihin, joihin ne kuuluvat:"; // "the data types they belong to:"
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Tietotyyppien hakemisto";   // "Data Type Index"
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Tietotyyppien dokumentaatio";   // "Data Type Documentation"
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Funktiot/aliohjelmat";   // "Functions/Subroutines"
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Funktioiden/aliohjelmien dokumentaatio";   // "Function/Subroutine Documentation"
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Tietotyypit";   // "Data Types"
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Moduulilista";   // "Modules List"
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Tässä on lista kaikista "; // "Here is a list of all "
      if (!extractAll) {
         result += "dokumentoiduista ";   // "documented "
      }
      result += "moduuleista lyhyiden selitysten kanssa:"; // "modules with brief descriptions:"
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      switch (compType) {
         case ClassDef::Class:
            result += " moduuli";
            break; // " Module"
         case ClassDef::Struct:
            result += " tyyppi";
            break; // " Type"
         case ClassDef::Union:
            result += " yhdiste";
            break; // " Union"
         case ClassDef::Interface:
            result += " rajapinta";
            break; // " Interface"
         case ClassDef::Protocol:
            result += " protokolla";
            break; // " Protocol"
         case ClassDef::Category:
            result += " kategoria";
            break; // " Category"
         case ClassDef::Exception:
            result += " poikkeus";
            break; // " Exception"
         default:
            break;
      }
      if (isTemplate) {
         result += "malli";   // " Template"
      }
      result += "referenssi"; // " Reference"
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " moduulin referenssi"; // " Module Reference"
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Moduulin jäsenet";   // "Module Members"
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Tässä on lista moduulin kaikista "; // "Here is a list of all "
      if (!extractAll) {
         result += "dokumentoiduista";   // "documented "
      }
      result += "jäsenistä, sekä linkit "; // "module members with links to "
      if (extractAll) {
         result += "moduulin dokumentaatioon jokaiselle jäsenelle:"; // "the module documentation for each member:"
      } else {
         result += "moduuleihin, joihin ne kuuluvat:"; // "the modules they belong to:"
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Moduulien hakemisto";   // "Modules Index"
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Moduuli" : "moduuli")); // "Module" / "module"
      if (!singular) {
         result += "t";   // "+s"
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
      QByteArray result = (QByteArray)"Dokumentaatio tälle "; // "The documentation for this "
      switch (compType) {
         case ClassDef::Class:
            result += "moduulille";
            break; // "module"
         case ClassDef::Struct:
            result += "tyypille";
            break; // "type"
         case ClassDef::Union:
            result += "yhdisteelle";
            break; // "union"
         case ClassDef::Interface:
            result += "rajapinnalle";
            break; // "interface"
         case ClassDef::Protocol:
            result += "protokollalle";
            break; // "protocol"
         case ClassDef::Category:
            result += "kategorialle";
            break; // "category"
         case ClassDef::Exception:
            result += "poikkeukselle";
            break; // "exception"
         default:
            break;
      }
      result += " luotiin "; // " was generated from the following file"
      if (single) {
         result += "seuraavasta tiedostosta:";
      } else {
         result += "seuraavista tiedostoista:";   // ":" / "s:"
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Tyyppi" : "tyyppi")); // "Type" / "type"
      if (!singular) {
         result = (first_capital ? "Tyypit" : "tyypit");   // "+s"
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Aliohjelma" : "aliohjelma")); // "Subprogram" / "subprogram"
      if (!singular) {
         result += "t";   // "+s"
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Tyyppien rajoitteet"; // "Type Constraints"
   }

};

#endif
