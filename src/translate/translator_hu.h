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

#ifndef TRANSLATOR_HU_H
#define TRANSLATOR_HU_H

class TranslatorHungarian : public TranslatorAdapter_1_4_6
{
 private:
   const char *zed(char c) {
      switch (c & ~('a' ^ 'A')) {
         case 'B':
         case 'C':
         case 'D':
         case 'F':
         case 'G':
         case 'H':
         case 'J':
         case 'K':
         case 'L':
         case 'M':
         case 'N':
         case 'P':
         case 'Q':
         case 'R':
         case 'S':
         case 'T':
         case 'V':
         case 'W':
         case 'X':
         case 'Z':
            return "  ";
         default:
            return "z ";
      }
   }
 public:

   // --- Language control methods -------------------
   virtual QByteArray idLanguage() {
      return "hungarian";
   }
   /*! Used to get the command(s) for the language support. This method
    *  was designed for languages which do not prefer babel package.
    *  If this methods returns empty string, then the latexBabelPackage()
    *  method is used to generate the command for using the babel package.
    */
   virtual QByteArray latexLanguageSupportCommand() {
      return "\\usepackage[T2A]{fontenc}\n"
             "\\usepackage[magyar]{babel}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Kapcsolódó függvények";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Figyelem! Ezek a függvények nem tagjai az osztálynak!)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Részletes leírás";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Típusdefiníció-tagok dokumentációja";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Enumeráció-tagok dokumentációja";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Tagfüggvények dokumentációja";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Adatmezők dokumentációja";
      } else {
         return "Adattagok dokumentációja";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Részletek...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "A tagok teljes listája";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Taglista";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "A(z) ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return " osztály tagjainak teljes listája, az örökölt tagokkal együtt.";
   }

   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Ezt a dokumentációt a Doxygen készítette ";
      if (s) {
         result += (QByteArray)" a" + zed(s[0]) + s + (QByteArray)" projekthez";
      }
      result += " a forráskódból.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "enum";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "enum-érték";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "definiálja:";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Modulok";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Osztályhierarchia";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Adatszerkezetek";
      } else {
         return "Osztálylista";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Fájllista";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Adatmezők";
      } else {
         return "Osztálytagok";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Globális elemek";
      } else {
         return "Fájlelemek";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Kapcsolódó lapok";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Példák";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Keresés";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Majdnem (de nem teljesen) betűrendbe szedett "
             "leszármazási lista:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Az összes ";
      if (!extractAll) {
         result += "dokumentált ";
      }
      result += "fájl listája rövid leírásokkal:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Az összes adatszerkezet listája rövid leírásokkal:";
      } else {
         return "Az összes osztály, struktúra, unió és interfész "
                "listája rövid leírásokkal:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Az összes ";
      if (!extractAll) {
         result += "dokumentált ";
      }
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "struktúra- és úniómező";
      } else {
         result += "osztálytag";
      }
      result += " listája, valamint hivatkozás ";
      if (!extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "a megfelelő struktúra-/úniódokumentációra minden mezőnél:";
         } else {
            result += "a megfelelő osztálydokumentációra minden tagnál:";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "a struktúrákra/úniókra, amikhez tartoznak:";
         } else {
            result += "az osztályokra, amikhez tartoznak:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Az összes ";
      if (!extractAll) {
         result += "dokumentált ";
      }

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "függvény, változó, makródefiníció, enumeráció és típusdefiníció";
      } else {
         result += "fájlelem";
      }
      result += " listája, valamint hivatkozás ";
      if (extractAll) {
         result += "a fájlokra, amikhez tartoznak:";
      } else {
         result += "a dokumentációra:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "A példák listája:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "A kapcsolódó dokumentációk listája:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "A modulok listája:";
   }

   // index titles (the project name is prepended for these)


   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Dokumentáció";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Modulmutató";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Hierarchikus mutató";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Adatszerkezet-mutató";
      } else {
         return "Osztálymutató";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Fájlmutató";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Modulok dokumentációja";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Adatszerkezetek dokumentációja";
      } else {
         return "Osztályok dokumentációja";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Fájlok dokumentációja";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Példák dokumentációja";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Kapcsolódó dokumentációk";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Referencia kézikönyv";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Makródefiníciók";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Típusdefiníciók";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Enumerációk";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Függvények";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Változók";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Enumeráció-értékek";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Makródefiníciók dokumentációja";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Típusdefiníciók dokumentációja";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Enumerációk dokumentációja";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Függvények dokumentációja";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Változók dokumentációja";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Adatszerkezetek";
      } else {
         return "Osztályok";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"";
      if (projName) {
         result += (QByteArray)"Projekt: " + projName;
      }
      result += (QByteArray)" Készült: " + date + " Készítette: ";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"A" + zed(clName[0]) + clName + " osztály származási diagramja:";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "CSAK BELSŐ HASZNÁLATRA!";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Figyelmeztetés";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Verzió";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Dátum";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Visszatérési érték";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Lásd még";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Paraméterek";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Kivételek";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Készítette";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Névtérlista";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Az összes ";
      if (!extractAll) {
         result += "dokumentált ";
      }
      result += "névtér listája rövid leírásokkal:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Barátok";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Barát és kapcsolódó függvények dokumentációja";
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
            result += " osztály";
            break;
         case ClassDef::Struct:
            result += " struktúra";
            break;
         case ClassDef::Union:
            result += " unió";
            break;
         case ClassDef::Interface:
            result += " interfész";
            break;
         case ClassDef::Protocol:
            result += " protocol";
            break; // translate me!
         case ClassDef::Category:
            result += " category";
            break; // translate me!
         case ClassDef::Exception:
            result += " kivétel";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += "sablon-";
      }
      result += "referencia";
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = fileName;
      result += " fájlreferencia";
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " névtér-referencia";
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Publikus tagfüggvények";
   }
   virtual QByteArray trPublicSlots() {
      return "Publikus rések";
   }
   virtual QByteArray trSignals() {
      return "Szignálok";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Statikus publikus tagfüggvények";
   }
   virtual QByteArray trProtectedMembers() {
      return "Védett tagfüggvények";
   }
   virtual QByteArray trProtectedSlots() {
      return "Védett rések";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Statikus védett tagfüggvények";
   }
   virtual QByteArray trPrivateMembers() {
      return "Privát tagfüggvények";
   }
   virtual QByteArray trPrivateSlots() {
      return "Privát rések";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Statikus privát tagfüggvények";
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
               result += " és ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Ősök: " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Leszármazottak: " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Újraimplementált ősök: " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Újraimplementáló leszármazottak: " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Névtértagok";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Az összes ";
      if (!extractAll) {
         result += "dokumentált ";
      }
      result += "névtér tagjainak listája, valamint hivatkozás ";
      if (extractAll) {
         result += " a megfelelő névtér dokumentációra minden tagnál:";
      } else {
         result += " a névterekre, amelynek tagjai:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Névtérmutató";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Névterek dokumentációja";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Névterek";
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
      QByteArray result = (QByteArray)"Ez a dokumentáció ";
      switch (compType) {
         case ClassDef::Class:
            result += "az osztályról";
            break;
         case ClassDef::Struct:
            result += "a struktúráról";
            break;
         case ClassDef::Union:
            result += "az unióról";
            break;
         case ClassDef::Interface:
            result += "az interfészről";
            break;
         case ClassDef::Protocol:
            result += "protocol";
            break; // translate me!
         case ClassDef::Category:
            result += "category";
            break; // translate me!
         case ClassDef::Exception:
            result += "a kivételről";
            break;
         default:
            break;
      }
      result += " a következő fájl";
      if (!single) {
         result += "ok";
      }
      result += " alapján készült:";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Visszatérési értékek";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Főoldal";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "o.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Definíció a(z) @1 fájl @0. sorában.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Definíció a(z) @0 fájlban.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Ellenjavallt";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"A" + zed(clName[0]) + clName + " osztály együttműködési diagramja:";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"A" + zed(fName[0]) + fName + " definíciós fájl függési gráfja:";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Konstruktorok és destruktorok dokumentációja";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Ugrás a fájl forráskódjához.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Ugrás a fájl dokumentációjához.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Előfeltétel";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Utófeltétel";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Invariáns";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Kezdő érték:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "forráskód";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Osztályhierarchia-ábra";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Ugrás az osztályhierarchia-ábrához";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Ugrás az szöveges osztályhierarchiához";
   }
   virtual QByteArray trPageIndex() {
      return "Oldalmutató";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Megjegyzés";
   }
   virtual QByteArray trPublicTypes() {
      return "Publikus típusok";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Adatmezők";
      } else {
         return "Publikus attribútumok";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Statikus publikus attribútumok";
   }
   virtual QByteArray trProtectedTypes() {
      return "Védett típusok";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Védett attribútumok";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Statikus védett attribútumok";
   }
   virtual QByteArray trPrivateTypes() {
      return "Privát típusok";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Privát attribútumok";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Statikus privát attribútumok";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a todo item */
   virtual QByteArray trTodo() {
      return "Tennivaló";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Tennivalók listája";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Hivatkozások:";
   }
   virtual QByteArray trRemarks() {
      return "Megjegyzések";
   }
   virtual QByteArray trAttention() {
      return "Figyelem";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Ez az ábra azt mutatja, hogy mely fájlok ágyazzák be "
             "közvetve vagy közvetlenül ezt a fájlt:";
   }
   virtual QByteArray trSince() {
      return "Először bevezetve";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Jelmagyarázat";
   }
   /*! page explaining how the dot graph's should be interpreted */
   virtual QByteArray trLegendDocs() {
      return
         "Ez az oldal elmagyarázza hogyan kell értelmezni a "
         "doxygen által készített ábrákat.<p>\n"
         "Vegyük a következő példát:\n"
         "\\code\n"
         "/*! Vágás miatt nem látható osztály */\n"
         "class Invisible { };\n\n"
         "/*! Levágott osztály, származása rejtett */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Doxygen kommentekkel nem dokumentált osztály */\n"
         "class Undocumented { };\n\n"
         "/*! Publikus származtatásal levezetett osztály */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! Egy sablonosztály */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Védett származtatásal levezetett osztály */\n"
         "class ProtectedBase { };\n\n"
         "/*! Privát származtatásal levezetett osztály */\n"
         "class PrivateBase { };\n\n"
         "/*! Osztály, melyet a származtatott osztály használ */\n"
         "class Used { };\n\n"
         "/*! Osztály, mely több másiknak leszármazottja */\n"
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
         "Ha a konfigurációs fájl \\c MAX_DOT_GRAPH_HEIGHT elemének értékét "
         "240-re állítjuk, az eredmény a következő ábra lesz:"
         "<p><center><img src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
         "<p>\n"
         "Az ábrán levő dobozok jelentése:\n"
         "<ul>\n"
         "<li>Kitöltött fekete doboz jelzi azt az osztályt vagy struktúrát,"
         "amelyről az ábra szól.\n"
         "<li>Fekete keret jelzi a dokumentált osztályokat és struktúrákat.\n"
         "<li>Szürke keret jelzi a nem dokumentált osztályokat és struktúrákat.\n"
         "<li>Piros keret jelzi azokat az osztályokat és struktúrákat, amelyeknél vágás miatt nem látható "
         "az összes leszármaztatási kapcsolat. Egy ábra vágásra kerül, ha nem fér bele "
         "a megadott tartományba."
         "</ul>\n"
         "A nyilak jelentése:\n"
         "<ul>\n"
         "<li>Sötétkék nyíl jelzi a publikus származtatást.\n"
         "<li>Sötétzöld nyíl jelzi a védett származtatást.\n"
         "<li>Sötétvörös nyíl jelzi a privát származtatást.\n"
         "<li>Lila szaggatott nyíl jelzi, ha az osztály egy másikat használ vagy tartalmaz. "
         "A nyíl felirata jelzi a változó(k) nevét, amelyeken keresztül a másik osztály kapcsolódik.\n"
         "<li>Sárga szaggatott nyíl jelzi a kapcsolatot a sablonpéldány és a példányosított "
         "osztálysablon között. A nyíl felirata jelzi a pélány sablonparamétereit.\n"
         "</ul>\n";

   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "Jelmagyarázat";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "Teszt";
   }
   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "Tesztlista";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Tulajdonságok";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Tulajdonságok dokumentációjka";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Adatszerkezetek";
      } else {
         return "Osztályok";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return name + (QByteArray)" csomag";
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Csomaglista";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "A csomagok rövid leírásai (amennyiben léteznek):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Csomagok";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Érték:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Hiba";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Hiba lista";
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
      return "1250";
   }


   /*! Used as ansicpg for RTF fcharset
    *  \see trRTFansicp() for a table of possible values.
    */
   virtual QByteArray trRTFCharSet() {
      return "0";
   }

   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "Tárgymutató";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool /*singular*/) {
      QByteArray result((first_capital ? "Osztály" : "osztály"));
      //if (!singular)  result+="es";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool /*singular*/) {
      QByteArray result((first_capital ? "Fájl" : "fájl"));
      //if (!singular)  result+="s";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool /*singular*/) {
      QByteArray result((first_capital ? "Névtér" : "névtér"));
      //if (!singular)  result+="s";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool /*singular*/) {
      QByteArray result((first_capital ? "Csoport" : "csoport"));
      //if (!singular)  result+="s";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool /*singular*/) {
      QByteArray result((first_capital ? "Oldal" : "oldal"));
      //if (!singular)  result+="s";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool /*singular*/) {
      QByteArray result((first_capital ? "Tag" : "tag"));
      //if (!singular)  result+="s";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool /*singular*/) {
      QByteArray result((first_capital ? "Globális elem" : "globális elem"));
      //if (!singular)  result+="s";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Szerző" : "szerző"));
      if (!singular) {
         result += "k";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Hivatkozások";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Megvalósítja a következőket: " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Megvalósítják a következők: " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Tartalomjegyzék";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Ellenjavallt elemek listája";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Események";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Események dokumentációja";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Csomag típusok";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Csomag függvények";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Statikus csomag függvények";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Csomag attribútumok";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Statikus csomag attribútumok";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Összes";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "A függvény hívási gráfja:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "A keresés eredménye";
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
         return "Sajnos egy dokumentum sem felelt meg a keresési feltételeknek.";
      } else if (numDocuments == 1) {
         return "<b>1</b> dokumentum felelt meg a keresési feltételeknek.";
      } else {
         return "<b>$num</b> dokumentum felelt meg a keresési feltételeknek."
                "Elsőnek a legjobb találatok vannak feltüntetve.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Találatok:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return filename + " Forrásfájl";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Könyvtárhierarchia";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Könyvtárak dokumentációja";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Könyvtárak";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Majdnem (de nem teljesen) betűrendbe szedett "
             "könyvtárhierarchia:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " könyvtárreferencia";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool /*singular*/) {
      QByteArray result((first_capital ? "Könyvtár" : "könyvtár"));
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
      return "Ez egy túlterhelt tagfüggvény."
             "A fenti függvénytől csak argumentumaiban különbözik.";
   }
};

#endif
