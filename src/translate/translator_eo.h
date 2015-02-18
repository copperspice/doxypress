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

#ifndef TRANSLATOR_EO_H
#define TRANSLATOR_EO_H

class TranslatorEsperanto : public TranslatorAdapter_1_8_4
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
      return "esperanto";
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
      return "\\usepackage[esperanto]{babel}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Rilataj Funkcioj";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Atentu ke tiuj ĉi ne estas membraj funkcioj.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Detala Priskribo";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Dokumentado de la Membraj Tipodifinoj";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Dokumentado de la Membraj Enumeracioj";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Dokumentado de la Membraj Funkcioj";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Kampa Dokumentado";
      } else {
         return "Dokumentado de la Membraj Datumoj";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Pli...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Listo de ĉiuj membroj";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Membra Listo";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Tiu ĉi estas la kompleta membraro de ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", inkluzive ĉiujn hereditajn membrojn.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Generita aŭtomate de Doxygen";
      if (s) {
         result += (QByteArray)" por " + s;
      }
      result += " el la fontkodo.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "enum nomo";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "enum valoro";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "difinita en";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Moduloj";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Klasa Hierarkio";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datumstruktoroj";
      } else {
         return "Klasaro";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Dosieraro";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datumkampoj";
      } else {
         return "Klasaj membroj";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Mallokalaĵoj";
      } else {
         return "Dosieraj Membroj";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Rilataj Paĝoj";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Ekzemploj";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Serĉi";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Tiu ĉi heredada listo estas plimalpli, "
             "sed ne tute, ordigita alfabete:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Jen listo de ĉiuj ";
      if (!extractAll) {
         result += "dokumentitaj ";
      }
      result += "dosieroj kun mallongaj priskriboj:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Jen datumstrukturoj kun mallongaj priskriboj:";
      } else {
         return "Jen la klasoj, strukturoj, kunigoj kaj interfacoj "
                "kun mallongaj priskriboj:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Jen listo de ĉiuj ";
      if (!extractAll) {
         result += "dokumentitaj ";
      }
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "strukturaj kaj kunigaj kampoj";
      } else {
         result += "klasaj membroj";
      }
      result += " kun ligiloj al ";
      if (!extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "la struktura/kuniga dokumentado por ĉiu kampo:";
         } else {
            result += "la klasa dokumentado por ĉiu membro:";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "la strukturoj/kunigoj al kiuj ili apartenas:";
         } else {
            result += "la klasoj al kiuj ili apartenas:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Jen listo de ĉiuj ";
      if (!extractAll) {
         result += "dokumentitaj ";
      }

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "funkcioj, variabloj, difinoj, enumeracioj kaj tipodifinoj";
      } else {
         result += "dosieraj membroj";
      }
      result += " kun ligiloj al ";
      if (extractAll) {
         result += "la dosieroj al kiuj ili apartenas:";
      } else {
         result += "la dokumentado:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Jen listo de ĉiuj la ekzemploj:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Jen listo de ĉiuj rilataj dokumentadaj paĝoj:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Jen listo de ĉiuj la moduloj:";
   }

   // index titles (the project name is prepended for these)

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Dokumentado";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Modula Indekso";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Hierarkia Indekso";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datumstruktura Indekso";
      } else {
         return "Klasa Indekso";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Dosiera Indekso";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Modula Dokumentado";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datumstruktura Dokumentado";
      } else {
         return "Klasa Dokumentado";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Dosiera Dokumentado";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Ekzempla Dokumentado";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Paĝa Dokumentado";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Referenca Manlibro";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Difinoj";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Tipdifinoj";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Enumeracioj";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Funkcioj";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Variabloj";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Enumeraciilo";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Difina Dokumentado";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Tipdifina Dokumentado";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Enumeracitipa Dokumentado";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Funkcia Dokumentado";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Variabla Dokumentado";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datumstrukturoj";
      } else {
         return "Klasoj";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Generita la " + date;
      if (projName) {
         result += (QByteArray)" por " + projName;
      }
      result += (QByteArray)" de";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Heredada diagramo por " + clName + ":";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Nur por ena uzado.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Averto";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Versio";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Dato";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Liveras";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Vido ankaŭ";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Parametroj";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Esceptoj";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Generita de";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Nomspacaro";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Jen listo de ĉiuj ";
      if (!extractAll) {
         result += "dokumentitaj ";
      }
      result += "nomspacoj kun mallongaj priskriboj:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Amikoj";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Dokumentado pri amikoj kaj rilatitaj funkcioj";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result = "Referenco de la ";
      if (isTemplate) {
         result += " ŝablono de la ";
      }
      switch (compType) {
         case ClassDef::Class:
            result += "klaso ";
            break;
         case ClassDef::Struct:
            result += "strukturo ";
            break;
         case ClassDef::Union:
            result += "kunigo ";
            break;
         case ClassDef::Interface:
            result += "interfaco ";
            break;
         case ClassDef::Protocol:
            result += "protokolo ";
            break;
         case ClassDef::Category:
            result += "kategorio ";
            break;
         case ClassDef::Exception:
            result += "escepto ";
            break;
         default:
            break;
      }
      result += (QByteArray)clName;
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = fileName;
      result += " Dosiera referenco";
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Nomspaca referenco";
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Publikaj Membraj Funkcioj";
   }
   virtual QByteArray trPublicSlots() {
      return "Pubikaj Ingoj";
   }
   virtual QByteArray trSignals() {
      return "Signaloj";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Statikaj Publikaj Membraj Funkcioj";
   }
   virtual QByteArray trProtectedMembers() {
      return "Protektitaj Membraj Funkcioj";
   }
   virtual QByteArray trProtectedSlots() {
      return "Protektitaj Ingoj";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Statikaj Protektitaj Membraj Funkcioj";
   }
   virtual QByteArray trPrivateMembers() {
      return "Privataj Membraj Funkcioj";
   }
   virtual QByteArray trPrivateSlots() {
      return "Privataj Ingoj";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Statikaj Privataj Membraj Funkcioj";
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
               result += ", kaj ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Heredas de " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Heredita de " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Rerealigita el " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Rerealigita en " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Nomspacaj Membroj";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Jen listo de ĉiuj ";
      if (!extractAll) {
         result += "dokumentitaj ";
      }
      result += "nomspacaj membroj kun ligiloj al ";
      if (extractAll) {
         result += "la nomspaca dokumentado de ĉiu membro:";
      } else {
         result += "la nomspacoj al kiuj ili apartenas:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Nomspaca Indekso";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Nomspaca Dokumentado";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Nomspacoj";
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
      QByteArray result = (QByteArray)"La dokumentado por tiu ĉi ";
      switch (compType) {
         case ClassDef::Class:
            result += "klaso";
            break;
         case ClassDef::Struct:
            result += "strukturo";
            break;
         case ClassDef::Union:
            result += "kunigo";
            break;
         case ClassDef::Interface:
            result += "interfaco";
            break;
         case ClassDef::Protocol:
            result += "protokolo";
            break;
         case ClassDef::Category:
            result += "kategorio";
            break;
         case ClassDef::Exception:
            result += "escepto";
            break;
         default:
            break;
      }
      result += " generitas el la ";
      if (single) {
         result += "sekva dosiero:";
      } else {
         result += "sekvaj dosieroj:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Liveraĵoj";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Ĉefa Paĝo";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "p.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Difinita sur la lineo @0 de la dosiero @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Difinita en la dosiero @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Evitinda";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Kunlaborada diagramo por " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Inkluzivaĵa dependeca diagramo por " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Konstruila kaj Detruila Dokumentado";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Iri al la fontkodo de tiu ĉi dosiero.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Iri al la dokumentado de tiu ĉi dosiero.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Antaŭkondiĉo";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Postkondiĉo";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Malvariaĵo";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Komenca valoro:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "kodo";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Grafika Klasa Hierarkio";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Iri al la grafika klasa hierarkio";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Iri al la teksta klasa hierarkio";
   }
   virtual QByteArray trPageIndex() {
      return "Paĝa Indekso";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Noto";
   }
   virtual QByteArray trPublicTypes() {
      return "Publikaj Tipoj";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datumkampoj";
      } else {
         return "Publikaj Atributoj";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Statikaj Publikaj Atributoj";
   }
   virtual QByteArray trProtectedTypes() {
      return "Protektitaj Tipoj";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Protektitaj Atributoj";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Statikaj Protektitaj Atributoj";
   }
   virtual QByteArray trPrivateTypes() {
      return "Privataj Tipoj";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Privataj Atributoj";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Statikaj Privataj Atributoj";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "Farendaĵo";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Farendaĵaro";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Referencita de";
   }
   virtual QByteArray trRemarks() {
      return "Rimarkoj";
   }
   virtual QByteArray trAttention() {
      return "Atentu";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Tiu ĉi diagramo montras kiuj dosieroj rekte aŭ malrekte "
             "inkluzivas tiun ĉi dosieron:";
   }
   virtual QByteArray trSince() {
      return "De";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Diagrama Klarigeto";
   }
   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Tiu ĉi paĝo klarigas kiel interpreti la diagramojn generitajn "
         "de doxygen.<p>\n"
         "Konsideru la sekvan ekzemplon:\n"
         "\\code\n"
         "/*! Nevidebla klaso pro trunkado */\n"
         "class Invisible { };\n\n"
         "/*! Trunkita klaso, hereda rilato kaŝiĝas */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Klaso ne dokumentita per komentoj de doxygen */\n"
         "class Undocumented { };\n\n"
         "/*! Klaso de kiu herediĝas per publika heredado */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! Ŝablona klaso */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Klaso de kiu herediĝas per protektita heredado */\n"
         "class ProtectedBase { };\n\n"
         "/*! Klaso de kiu herediĝas per privata heredado */\n"
         "class PrivateBase { };\n\n"
         "/*! Klaso uzata de la klaso Inherited */\n"
         "class Used { };\n\n"
         "/*! Supra klaso kiu heredas de kelkaj aliaj klasoj */\n"
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
         "Tio ĉi liveros la sekvan diagramon:"
         "<p><center><img alt=\"\" src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
         "<p>\n"
         "La skatoloj de la supra diagramo havas la sekvajn signifojn:\n"
         "<ul>\n"
         "<li>%Plene griza skatolo reprezentas la strukturon aŭ klason "
         "kies diagramo generiĝis.\n"
         "<li>%Skatolo kun nigra bordero montras dokumentitan strukturon aŭ klason.\n"
         "<li>%Skatolo kun griza bordero montras nedokumentitan strukturon aŭ klason.\n"
         "<li>%Skatolo kun ruĝa bordero montras dokumentitan strukturon aŭ klason por "
         "kiu ne ĉiuj heredadoj/enhavoj montriĝas. %Diagramo estas trunkota "
         "se ĝi ne adaptiĝas en la donitajn limojn.\n"
         "</ul>\n"
         "La sagoj havas la sekvajn signifojn:\n"
         "<ul>\n"
         "<li>%Malhelblua sago uzatas por montri publika heredado "
         "inter du klasoj.\n"
         "<li>%Malhelverda sago uzatas por protektita heredado.\n"
         "<li>%Malhelruĝa sago uzatas por privata heredado.\n"
         "<li>%Purpura streka sago uzatas se klaso enhavatas aŭ uzatas "
         "de alia klaso. La sago estas etikedatas kun la variablo(j) "
         "tra kiu la montrita klaso aŭ strukturo estas alirebla.\n"
         "<li>%Flava streka sago montras rilato inter ŝablona apero kaj "
         "la ŝablona klaso el kiu ĝi realigitas. La sago etikeditas kun "
         "la parametroj de la ŝablona apero.\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "klarigeto";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "Testo";
   }
   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "Testa Listo";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Atributoj";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Atributa Dokumentado";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Datumstrukturoj";
      } else {
         return "Klasoj";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Pakaĵo " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Pakaĵa Listo";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Jen listo de pakaĵoj kun mallongaj priskriboj (se ekzistas):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Pakaĵoj";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Valoro:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Cimo";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Cima Listo";
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
      return "";
   }


   /*! Used as ansicpg for RTF fcharset
    *  \see trRTFansicp() for a table of possible values.
    */
   virtual QByteArray trRTFCharSet() {
      return "1";
   }

   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "Indekso";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Klaso" : "klaso"));
      if (!singular) {
         result += "j";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Dosiero" : "dosiero"));
      if (!singular) {
         result += "j";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Nomspaco" : "nomspaco"));
      if (!singular) {
         result += "j";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Grupo" : "grupo"));
      if (!singular) {
         result += "j";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Paĝo" : "paĝo"));
      if (!singular) {
         result += "j";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Membro" : "membro"));
      if (!singular) {
         result += "j";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Mallokalaĵo" : "mallokalaĵo"));
      if (!singular) {
         result += "j";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Aŭtoro" : "aŭtoro"));
      if (!singular) {
         result += "j";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Referencoj";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Realigas " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Realigita en " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Enhava Tabelo";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Evitindaĵa Listo";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Eventoj";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Eventa Dokumentado";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Pakaĵaj Tipoj";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Pakaĵaj Funkcioj";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Statikaj Pakaĵaj Funkcioj";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Pakaĵaj Atributoj";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Statikaj Pakaĵaj Atributoj";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Ĉiuj";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Jen la vokdiagramo por tiu ĉi funkcio:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Serĉaj Rezultoj";
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
         return "Pardonu, nenio dokumento kongruas vian peton.";
      } else if (numDocuments == 1) {
         return "Trafita <b>unu</b> dokumenton kongruantan vian peton.";
      } else {
         return "Trafitaj <b>$num</b> dokumentojn kongruantajn vian peton. "
                "Montriĝos plej bonaj kongruoj unue.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Kongruoj:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return filename + " Fonta Dosiero";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Dosieruja Hierarkio";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Dosieruja Dokumentado";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Dosierujoj";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Tiu ĉi dosieruja hierarkio estas plimalpli, "
             "sed ne tute, ordigita alfabete:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " Dosieruja Referenco";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Dosierujo" : "dosierujo"));
      if (!singular) {
         result += "j";
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
      return "Tiu ĉi estas superŝarĝita membra funkcio, "
             "donita por faciligo. Ĝi nur malsamas de la supra "
             "funkcio nur pro la argumento(j) kiujn ili akceptas.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Jen la vokdiagramo por tiu ĉi funkcio:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Enumeraciila Dokumentado";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Dokumentado de Membraj Funkcioj/Subrutinoj";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Datumtipa Listo";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Datumkampoj";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Jen la datumtipoj kun mallongaj priskriboj:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Jen listo de ĉiuj ";
      if (!extractAll) {
         result += "dokumentitaj ";
      }
      result += "datumtipaj membroj";
      result += " kun ligiloj al ";
      if (!extractAll) {
         result += "la datumstruktura dokumentado de ĉiu membro";
      } else {
         result += "la datumtipoj al kiuj ili apartenas:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Datumtipa Indekso";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Datumtipa Dokumentado";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Funkcioj/Subrutinoj";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Funkcia/Subrutina Dokumentado";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Datumtipoj";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Modula Listo";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Jen listo de ĉiuj ";
      if (!extractAll) {
         result += "dokumentitaj ";
      }
      result += "moduloj kun mallongaj priskriboj:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = "Referenco de la ";
      if (isTemplate) {
         result += " ŝablono de la ";
      }
      switch (compType) {
         case ClassDef::Class:
            result += "modulo ";
            break;
         case ClassDef::Struct:
            result += "tipo ";
            break;
         case ClassDef::Union:
            result += "kunigo ";
            break;
         case ClassDef::Interface:
            result += "interfaco ";
            break;
         case ClassDef::Protocol:
            result += "protokolo ";
            break;
         case ClassDef::Category:
            result += "kategorio ";
            break;
         case ClassDef::Exception:
            result += "escepto ";
            break;
         default:
            break;
      }
      result += (QByteArray)clName;
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Modula Referenco";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Modulaj Membroj";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Jen listo de ĉiuj ";
      if (!extractAll) {
         result += "dokumentitaj ";
      }
      result += "modulaj membroj kun ligiloj al la ";
      if (extractAll) {
         result += "modula dokumentado de ĉiu membro:";
      } else {
         result += "moduloj al kiuj ili apartenas:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Indekso de Moduloj";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Modulo" : "modulo"));
      if (!singular) {
         result += "j";
      }
      return result;
   }
   /*! This is put at the bottom of a module documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFilesFortran(ClassDef::CompoundType compType,
         bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"La dokumentado por tiu ĉi ";
      switch (compType) {
         case ClassDef::Class:
            result += "modulo";
            break;
         case ClassDef::Struct:
            result += "tipo";
            break;
         case ClassDef::Union:
            result += "kunigo";
            break;
         case ClassDef::Interface:
            result += "interfaco";
            break;
         case ClassDef::Protocol:
            result += "protokolo";
            break;
         case ClassDef::Category:
            result += "kategorio";
            break;
         case ClassDef::Exception:
            result += "escepto";
            break;
         default:
            break;
      }
      result += " kreiĝis el la ";
      if (single) {
         result += "sekva dosiero:";
      } else {
         result += "sekvaj dosieroj:";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Tipo" : "tipo"));
      if (!singular) {
         result += "j";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Subprogramo" : "subprogramo"));
      if (!singular) {
         result += "j";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Tipaj Limigoj";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return QByteArray(name) + " Rilato";
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Ŝarĝante...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Malloka Nomspaco";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Serĉante...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Nenia kongruo";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Dosiero en " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Inkluzivas dosieron en " + name;
   }
   
   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Bibliografiaj Referencoj";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Kopirajto";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Dosieruja dependa diagramo por ") + name + ":";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.0
   //////////////////////////////////////////////////////////////////////////

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "detala nivelo";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Parametroj de ŝablonoj";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "kaj " + number + " pli...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "La dokumentaro por tiu ĉi enum estis generita el la sekva dosiero";
      if (!single) {
         result += "s";
      }
      result += ":";
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return QByteArray(name) + " Enum Referenco";
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " heredita el " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Kromaj Hereditaj Membroj";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      QByteArray opt = enable ? "aktivigi" : "malaktivigi";
      return "klaku por " + opt + " panelan sinkronigon";
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "Provizita de kategorio @1.";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "Etendi klason @1.";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "Klasaj Metodoj";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "Aperaj Metodoj";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "Dokumentaro de la Metodo";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      return "Fasona Superrigardo";
   }

   //////////////////////////////////////////////////////////////////////////

};

#endif
