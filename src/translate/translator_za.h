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

#ifndef TRANSLATOR_ZA_H
#define TRANSLATOR_ZA_H

class TranslatorAfrikaans : public TranslatorAdapter_1_6_0
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
      return "afrikaans";
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
    * The Afrikaans LaTeX does not use such commands.  Because of this
    * the empty string is returned in this implementation.
    */
   virtual QByteArray latexLanguageSupportCommand() {
      //should we use return "\\usepackage[afrikaans]{babel}\n";
      // not sure - for now return an empty string
      return "";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Verwante Funksies";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Let daarop dat hierdie nie lede funksies is nie.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Detail Beskrywing";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Lede Typedef Dokumentasie";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Lede Enumerasie Dokumentasie";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Lede Funksie Dokumentasie";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Veld Dokumentasie";
      } else {
         return "Lede Data Dokumentasie";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Meer detail...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Lys van alle lede";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Lede Lys";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Hierdie is 'n volledige lys van lede vir ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", insluitend alle afgeleide lede.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Automaties gegenereer deur Doxygen";
      if (s) {
         result += (QByteArray)" vir " + s;
      }
      result += " van die bron kode af.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "enum naam";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "enum waarde";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "gedefinieër in";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Modules";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Klas Hierargie";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Data Strukture";
      } else {
         return "Klas Lys";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Leër Lys";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Data Velde";
      } else {
         return "Klas Lede";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Globals";
      } else {
         return "Leër Lede";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Verwante Bladsye";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Voorbeelde";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Soek";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Hierdie afgeleide lys word rofweg gesorteer: ";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Hier is 'n lys van alle  ";
      if (!extractAll) {
         result += "gedokumenteerde ";
      }
      result += "leërs met kort beskrywings:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return " Data strukture met kort beskrywings:";
      } else {
         return "Klasse, structs, "
                "unions en intervlakke met kort beskrywings:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "'n Lys van alle ";
      if (!extractAll) {
         result += "gedokumenteerde ";
      }
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "struct en union velde";
      } else {
         result += "klas lede";
      }
      result += " met skakels na ";
      if (!extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "die struct/union dokumentasie vir elke veld:";
         } else {
            result += "die klas dokumentasie vir elke lid:";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "die structures/unions waaraan hulle behoort:";
         } else {
            result += "die klasse waaraan hulle behoort:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "'n Lys van alle ";
      if (!extractAll) {
         result += "gedokumenteerde ";
      }

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "funksies, veranderlikes, defines, enums, en typedefs";
      } else {
         result += "leër lede";
      }
      result += " met skakels na ";
      if (extractAll) {
         result += "die leërs waaraan hulle behoort:";
      } else {
         result += "die dokumentasie:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "'n Lys van alle voorbeelde:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "'n Lys van alle verwante dokumentasie:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "'n Lys van alle modules:";
   }

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Dokumentasie";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Module Indeks";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Hierargiese Indeks";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Data Strukture Indeks";
      } else {
         return "Klas Indeks";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Leër Indeks";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Module Dokumentasie";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Data Strukture Dokumentasie";
      } else {
         return "Klas Dokumentasie";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Leër Dokumentasie";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Voorbeeld Dokumentasie";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Bladsy Dokumentasie";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Verwysings Handleiding";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Definiesies";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Typedefs";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Enumerations";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Funksies";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Veranderlikes";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Enumeration waardes";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Define Documentation";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Typedef Dokumentasie";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Enumeration Type Dokumentasie";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Funksie Dokumentasie";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Veranderlike Dokumentasie";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Data Strukture";
      } else {
         return "Klasse";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Gegenereer op " + date;
      if (projName) {
         result += (QByteArray)" vir " + projName;
      }
      result += (QByteArray)" deur";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Afleidings diagram vir " + clName + ":";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Slegs vir interne gebruik.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Waarskuwing";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Weergawe";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Datum";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Returns";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Sien ook";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Parameters";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Exceptions";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Gegenereer deur";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Namespace Lys";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "'n Lys van alle ";
      if (!extractAll) {
         result += "gedokumenteerde ";
      }
      result += "namespaces met kort beskrywings:";
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
      return "Friends En Verwante Funksie Dokumentasie";
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
            result += " klas";
            break;
         case ClassDef::Struct:
            result += " Struct";
            break;
         case ClassDef::Union:
            result += " Union";
            break;
         case ClassDef::Interface:
            result += " Intervlak";
            break;
         case ClassDef::Protocol:
            result += " Protocol";
            break;
         case ClassDef::Category:
            result += " Kategorie";
            break;
         case ClassDef::Exception:
            result += " Exception";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += " Template";
      }
      result += " Verwysing";
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = fileName;
      result += " Leër Verwysing";
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Namespace Verwysing";
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Publieke Lede Funksies";
   }
   virtual QByteArray trPublicSlots() {
      return "Publieke Slots";
   }
   virtual QByteArray trSignals() {
      return "Signals";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Statiese Publieke Lede Funksies";
   }
   virtual QByteArray trProtectedMembers() {
      return "Beskermde Lede Funksies";
   }
   virtual QByteArray trProtectedSlots() {
      return "Beskermde Slots";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Statiese Beskermde Lede Funksies";
   }
   virtual QByteArray trPrivateMembers() {
      return "Private Lede Funksies";
   }
   virtual QByteArray trPrivateSlots() {
      return "Private Slots";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Statiese Private Lede Funksies";
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
               result += ", en ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Afgelei van" + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Afgelei van" + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Hergeimplimenteer van " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Hergeimplimenter in " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Namespace Lede";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "'n Lys van alle ";
      if (!extractAll) {
         result += "gedokumenteerde ";
      }
      result += "namespace lede met skakels na ";
      if (extractAll) {
         result += "die namespace dokumentasie vir elke lid:";
      } else {
         result += "die namespaces waaraan hulle behoort:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Namespace Indeks";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Namespace Dokumentasie";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Namespaces";
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
      QByteArray result = (QByteArray)"Die dokumentasie vir hierdie ";
      switch (compType) {
         case ClassDef::Class:
            result += "klas";
            break;
         case ClassDef::Struct:
            result += "struct";
            break;
         case ClassDef::Union:
            result += "union";
            break;
         case ClassDef::Interface:
            result += "intervlak";
            break;
         case ClassDef::Protocol:
            result += "protokol";
            break;
         case ClassDef::Category:
            result += "kategorie";
            break;
         case ClassDef::Exception:
            result += "eksepsie";
            break;
         default:
            break;
      }
      result += " is gegenereer vanaf die volgende leër";
      if (single) {
         result += ":";
      } else {
         result += "s:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Return waardes";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Hoof Bladsy";
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
      return "Gedefinieër by lyn @0 van leër @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Definisie in leër @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Verouderd";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Samewerkings diagram vir " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Insluitings afhanklikheid diagram vir " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Konstruktor & Destruktor Dokumentasie";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Skakel na die bron kode van hierdie leër.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Skakel na die dokumentasie van hierdie leër.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Prekondisie";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Postkondisie";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Invariant";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Oorspronklike waarde:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "kode";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Grafiese Klasse Hierargie";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Skakel na die grafiese klasse hierargie";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Skakel na die teks klasse hierargie";
   }
   virtual QByteArray trPageIndex() {
      return "Bladsy Indeks";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Nota";
   }
   virtual QByteArray trPublicTypes() {
      return "Publieke Tipes";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Data Velde";
      } else {
         return "Publieke Public Attributes";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Statiese Publieke Attribute";
   }
   virtual QByteArray trProtectedTypes() {
      return "Beskermde Tipes";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Beskermde Attribute";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Statiese Beskermde Attribute";
   }
   virtual QByteArray trPrivateTypes() {
      return "Private Tipes";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Private Attribute";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Statiese Private Attribute";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "Aksies";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Aksie Lys";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Verwysing van";
   }
   virtual QByteArray trRemarks() {
      return "Opmerkings";
   }
   virtual QByteArray trAttention() {
      return "Aandag";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Hierdie diagram verduidelik watter leërs direk of"
             "indirek hierdie leër insluit:";
   }
   virtual QByteArray trSince() {
      return "Sederd";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Diagram beskrywing";
   }
   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Hierdie bladsy beskryf die diagram konvensies wat gebruik word "
         "deur doxygen.<p>\n"
         "in hierdie voorbeeld:\n"
         "\\code\n"
         "/*! Onsigbare klas weens afkorting */\n"
         "class Invisible { };\n\n"
         "/*! Afgekorte klas, afgeleide verwantskap word versteek */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Ongedokumenteerde Klas, geen doxygen kommentaar nie */\n"
         "class Undocumented{ };\n\n"
         "/*! 'n Klas wat afgelei is met 'n publieke verwantskap */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! 'n template klas */\n"
         "template<class T> class Templ { };\n\n"
         "/*! 'n Klas wat afgelei is met 'n beskermde verwantskap */\n"
         "class ProtectedBase { };\n\n"
         "/*! 'n Klas wat afgelei is met 'n private verwantskap */\n"
         "class PrivateBase { };\n\n"
         "/*! 'n Klas wat gebrui word deur die Afgeleide klas */\n"
         "class GebruikMy { };\n\n"
         "/*! 'n Super klas wat afgelei word van 'n aantal basis klasse */\n"
         "class Inherited : public PublicBase,\n"
         "                  protected ProtectedBase,\n"
         "                  private PrivateBase,\n"
         "                  public Ongedokumenteer,\n"
         "                  public Templ<int>\n"
         "{\n"
         "  private:\n"
         "    Used *m_usedClass;\n"
         "};\n"
         "\\endcode\n"
         "As die \\c MAX_DOT_GRAPH_HEIGHT merker in die konfigurasie leër "
         "aan 240 gelyk gestel is, word die volgende diagram geproduseer:"
         "<p><center><img alt=\"\" src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
         "<p>\n"
         "Die reghoeke in die diagram het die volgende betekenis:\n"
         "<ul>\n"
         "<li>%'n Soliede swart reghoek verteenwoordig die klas waarvoor "
         "die diagram gegenereer is.\n"
         "<li>%'n Reghoek met 'n swart omlyning verteenwoordig 'n gedokumenteerde klas.\n"
         "<li>%'n Reghoek met 'n grys omlyning verteenwoordig 'n ongedokumenteerde klas.\n"
         "<li>%'n Reghoek met 'n rooi omlyning verteenwoordig 'n gedokumenteerde klas waarvoor"
         "alle verwante klasse (afgeleide of gebruik) nie getoon word nie. %'n Diagram word "
         "op hierie manier afgekort as dit nie in die gespesifiseerde raam pas nie.\n"
         "</ul>\n"
         "Die pyltjies het die volgende betekenis:\n"
         "<ul>\n"
         "<li>%'n Donker blou pyltjie verteenwoordig 'n publieke afgeleide "
         "verwantskap tussen twee klasse.\n"
         "<li>%'n Donker groen pyltjie word gebruik vir 'n beskermde verwantskap.\n"
         "<li>%'n Donker rooi pyltjie verteenwoordig private verwantskappe.\n"
         "<li>%'n Pers pyltjie word gebruik as 'n klas gebruik of bevat word "
         "deur 'n ander klas. Die pyltjie word gemerk met die veranderlike(s) waar deur "
         "die verwysde klass verkrygbaar is.\n"
         "<li>%'n Geel stippel pyl verteenwoordig die verwantslap tussen 'n template instansie en "
         "die template waarvan die klas vervaardig is. Die pyltjie word gemerk met die "
         "template parameters van die instansie.\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "beskrywing";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "Toets";
   }
   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "Toets Lys";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Eienskappe";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Eienskap Dokumentasie";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Data Strukture";
      } else {
         return "Klasse";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Pakket " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Pakket Lys";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Die pakkette met kort beskrywings (indien beskikbaar):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Pakkette";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Waarde:";
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
      return "Bug Lys";
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
      QByteArray result((first_capital ? "Klas" : "klas"));
      if (!singular) {
         result += "se";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Leër" : "leër"));
      if (!singular) {
         result += "s";
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
         result += "s";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Groep" : "groep"));
      if (!singular) {
         result += "e";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Bladsy" : "bladsy"));
      if (!singular) {
         result += "e";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Lid" : "lid"));
      if (!singular) {
         result = (first_capital ? "Lede" : "lede");
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
      QByteArray result((first_capital ? "Outeur" : "outeur"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Verwysings";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Implimenteer " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Geimplimenteer in " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Inhoudsopgawe";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Verouderde Lys";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Events";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Event Dokumentasie";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Pakket Tipes";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Pakket Funksies";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Statiese Pakket Funksies";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Pakket Eienskappe";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Statiese Pakket Eienskappe";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Alle Lede";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "'n gebruiks diagram vir hierdie funksie:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Soektog Resultate";
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
         return "Geen dokumente na gelang van jou navraag nie.";
      } else if (numDocuments == 1) {
         return "Die soektog het <b>1</b> dokument gevind na gelang van jou navraag.";
      } else {
         return "Die soektog het <b>$num</b> dokumente gevind na gelang van jou navraag. "
                "Beste resultate eerste.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Teikens:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return filename + " Bron kode Leër";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Directory Hiërargie";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Directory Documentasie";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Directories";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Hierdie directory hiërargie is min of meer alfabeties "
             "gesorteer:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " Directory Verwysing";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Director" : "director"));
      if (singular) {
         result += "y";
      } else {
         result += "ies";
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
      return "Hierdie is 'n oorlaaide lede funksie, "
             "vertoon vir volledigheid. Dit verskil slegs van die bogegewe "
             "funksie in die argument(e) wat dit aanvaar.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Hier is die roep skema vir die funksie:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Enumerator Dokumentasie";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Lede Funksie/Subroetine Dokumentasie";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Data Tipes Lys";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Data Velde";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Hier is die data tipes met kort beskrywings:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Hier is 'n lys van alle ";
      if (!extractAll) {
         result += "gedokumenteerde ";
      }
      result += "data tipe lede";
      result += " met skakels na ";
      if (!extractAll) {
         result += "die data strukture dokumentasie vir elke lid";
      } else {
         result += "die data tipes waaraan hulle behoort:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Data Tipe Indeks";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Data Tipe Dokumentasie";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Funksies/Subroetines";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Funksies/Subroetine Dokumentasie";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Data Tipes";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Modules Lys";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Hier is 'n lys van alle ";
      if (!extractAll) {
         result += "gedokumenteerde ";
      }
      result += "modules met kort beskrywings:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      switch (compType) {
         case ClassDef::Class:
            result += " Module";
            break;
         case ClassDef::Struct:
            result += " Tipe";
            break;
         case ClassDef::Union:
            result += " Unie";
            break;
         case ClassDef::Interface:
            result += " Interflak";
            break;
         case ClassDef::Protocol:
            result += " Protokol";
            break;
         case ClassDef::Category:
            result += " Kategorie";
            break;
         case ClassDef::Exception:
            result += " Eksepsie";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += " Template";
      }
      result += " Bron";
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Module Bron";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Module Lede";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Hier is 'n lys van alle ";
      if (!extractAll) {
         result += "gedokumenteerde ";
      }
      result += "module lede met skakels na ";
      if (extractAll) {
         result += "die module dokumentasie vir elke lid:";
      } else {
         result += "die modules waaraan hulle behoort:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Modules Indeks";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Module" : "module"));
      if (!singular) {
         result += "s";
      }
      return result;
   }
   /*! This is put at the bottom of a module documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFilesFortran(ClassDef::CompoundType compType,
         bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"The documentation for this ";
      switch (compType) {
         case ClassDef::Class:
            result += "module";
            break;
         case ClassDef::Struct:
            result += "tipe";
            break;
         case ClassDef::Union:
            result += "unie";
            break;
         case ClassDef::Interface:
            result += "Interflak";
            break;
         case ClassDef::Protocol:
            result += "protokcol";
            break;
         case ClassDef::Category:
            result += "kategorie";
            break;
         case ClassDef::Exception:
            result += "eksepsie";
            break;
         default:
            break;
      }
      result += " is gegenereer vanaf die foldende leer";
      if (single) {
         result += ":";
      } else {
         result += "s:";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Tipe" : "tipe"));
      if (!singular) {
         result += "s";
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
         result += "me";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Tipe Limiete";
   }

};

#endif
