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

#ifndef TRANSLATOR_NO_H
#define TRANSLATOR_NO_H

class TranslatorNorwegian : public TranslatorAdapter_1_4_6
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
      return "norwegian";
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
    */
   virtual QByteArray latexLanguageSupportCommand() {
      return
         "\\usepackage[norwegian]{babel}\n"
         "\\usepackage[T1]{fontenc}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Relaterte funksjoner";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Merk at disse ikke er medlemsfunksjoner.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Detaljert beskrivelse";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Medlemstypedef-dokumentasjon";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Medlemsenumerasjon-dokumentasjon";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Medlemsfunksjon-dokumentasjon";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Feltdokumentasjon";
      } else {
         return "Medlemsdata-dokumentasjon";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Mer...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Liste over alle medlemmer";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Medlemsliste";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Dette er den fullstendige listen over medlemmer for ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", alle arvede medlemmer inkludert.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Generert automatisk av DoxyPress";
      if (s) {
         result += (QByteArray)" for " + s;
      }
      result += " fra kildekoden.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "enum-navn";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "enum-verdi";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "definert i";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Moduler";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Klassehierarki";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config::getBool("optimize-c")) {
         return "Datastrukturer";
      } else {
         return "Klasseliste";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Fil-liste";
   }

   /*! This is put above each page as a link to the list of all verbatim headers */
   virtual QByteArray trHeaderFiles() {
      return "Header-filer";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config::getBool("optimize-c")) {
         return "Datafelt";
      } else {
         return "Klassemedlemmer";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config::getBool("optimize-c")) {
         return "Globale";
      } else {
         return "Filmedlemmer";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Relaterte sider";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Eksempler";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Søk";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Denne arvelisten er grovsortert alfabetisk "
             "(ikke nødvendigvis korrekt):";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Her er en liste over alle ";
      if (!extractAll) {
         result += "dokumenterte ";
      }
      result += "filer med korte beskrivelser:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config::getBool("optimize-c")) {
         return "Her er datastrukturene med korte beskrivelser:";
      } else {
         return "Her er klasser, struct'er, "
                "unioner og interface'er med korte beskrivelser:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Her er en liste over alle ";
      if (!extractAll) {
         result += "dokumenterte ";
      }
      if (Config::getBool("optimize-c")) {
         result += "struct- og unionfelter";
      } else {
         result += "klassemedlemmer";
      }
      result += " med koblinger til ";
      if (!extractAll) {
         if (Config::getBool("optimize-c")) {
            result += "struct/union dokumentasjon for hvert felt:";
         } else {
            result += "klassedokumentasjonen for hvert medlem:";
         }
      } else {
         if (Config::getBool("optimize-c")) {
            result += "struct'ene/unionene de hører til:";
         } else {
            result += "klassene de hører til:";
         }
      }
      return result;
   }
   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Her er en liste over alle ";
      if (!extractAll) {
         result += "dokumenterte ";
      }

      if (Config::getBool("optimize-c")) {
         result += "funksjoner, variabler, definisjoner, enum'er, og typedef'er";
      } else {
         result += "filmedlemmer";
      }
      result += " med koblinger til ";
      if (extractAll) {
         result += "filene de hører til:";
      } else {
         result += "dokumentasjonen:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all header files. */
   virtual QByteArray trHeaderFilesDescription() {
      return "Her er alle header-filene som utgjør API'et:";
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Her er en liste over alle eksemplene:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Her er en liste over alle relaterte dokumentasjonssider:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Her er en liste over alle moduler:";
   }

   /*! This sentences is used in the annotated class/file lists if no brief
    * description is given.
    */
   virtual QByteArray trNoDescriptionAvailable() {
      return "Ingen beskrivelse tilgjengelig";
   }

   // index titles (the project name is prepended for these)


   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Dokumentasjon";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Modulindeks";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Hierarkisk indeks";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config::getBool("optimize-c")) {
         return "Datastrukturindeks";
      } else {
         return "Klasseindeks";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Filindeks";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Moduldokumentasjon";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Datastrukturdokumentasjon";
      } else {
         return "Klassedokumentasjon";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Fildokumentasjon";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Eksempeldokumentasjon";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Sidedokumentasjon";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Referansemanual";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Definisjoner";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Typedef'er";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Enumerasjoner";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Funksjoner";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Variabler";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Enumerasjonsverdier";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Define-dokumentasjon";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Typedef-dokumentasjon";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Enumerasjontype dokumentasjon";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Enumerasjonsverdi dokumentasjon";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Funksjonsdokumentasjon";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Variabeldokumentasjon";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config::getBool("optimize-c")) {
         return "Datastrukturer";
      } else {
         return "Klasser";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Generert " + date;
      if (projName) {
         result += (QByteArray)" for " + projName;
      }
      result += (QByteArray)" av";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Arvediagram for " + clName + ":";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Kun for intern bruk.";
   }

   /*! this text is generated when the \\reimp command is used. */
   virtual QByteArray trReimplementedForInternalReasons() {
      return "Reimplementert av interne grunner; API er ikke påvirket.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Advarsel";
   }

   /*! this text is generated when the \\bug command is used. */
   virtual QByteArray trBugsAndLimitations() {
      return "Feil og begrensninger";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Versjon";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Dato";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Returnerer";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Se også";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Parametre";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Unntak";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Generert av";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Navneromsliste";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Her er en liste over alle ";
      if (!extractAll) {
         result += "dokumenterte ";
      }
      result += "navnerom med korte beskrivelser:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Venner";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Venner og relatert funksjonsdokumentasjon";
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
            result += " Klasse";
            break;
         case ClassDef::Struct:
            result += " Struct";
            break;
         case ClassDef::Union:
            result += " Union";
            break;
         case ClassDef::Interface:
            result += " Grensesnitt";
            break;
         case ClassDef::Exception:
            result += " Unntak";
            break;
         case ClassDef::Protocol:
            result += " Protocol";
            break;
         case ClassDef::Category:
            result += " Category";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += " Mal";
      }
      result += " Referanse";
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = fileName;
      result += " filreferanse";
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " navneromsreferanse";
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Public medlemsfunksjoner";
   }
   virtual QByteArray trPublicSlots() {
      return "Public slots";
   }
   virtual QByteArray trSignals() {
      return "Signaler";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Statiske public medlemsfunksjoner";
   }
   virtual QByteArray trProtectedMembers() {
      return "Protected memdlemsfunksjoner";
   }
   virtual QByteArray trProtectedSlots() {
      return "Protected slots";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Statiske protected medlemsfunksjoner";
   }
   virtual QByteArray trPrivateMembers() {
      return "Private medlemsfunksjoner";
   }
   virtual QByteArray trPrivateSlots() {
      return "Private slots";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Statiske private medlemsfunksjoner";
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
               result += ", og ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Arver " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Arvet av " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Reimplementert fra " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Reimplementert i " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Navneromsmedlemmer";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Her er en liste over alle ";
      if (!extractAll) {
         result += "dokumenterte ";
      }
      result += "navneromsmedlemmer med koblinger til ";
      if (extractAll) {
         result += "navneromsdokumentasjonen for hvert medlem:";
      } else {
         result += "navnerommet de hører til:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Navneromsindeks";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Navneromsdokumentasjon";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Navnerom";
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
      QByteArray result = (QByteArray)"Dokumentasjonen for ";
      switch (compType) {
         case ClassDef::Class:
            result += "denne klasse";
            break;
         case ClassDef::Struct:
            result += "denne struct";
            break;
         case ClassDef::Union:
            result += "denne union";
            break;
         case ClassDef::Interface:
            result += "dette interface";
            break;
         case ClassDef::Exception:
            result += "dette unntak";
            break;
         case ClassDef::Protocol:
            result += "denne protocol";
            break;
         case ClassDef::Category:
            result += "denne category";
            break;
         default:
            break;
      }
      result += " ble generert fra følgende fil";
      if (single) {
         result += ":";
      } else {
         result += "er:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Returverdier";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Hovedside";
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

   virtual QByteArray trSources() {
      return "Kilder";
   }
   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Definisjon på linje @0 i filen @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Definisjon i filen @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Foreldet";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Samarbeidsdiagram for " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Avhengighetsgraf for " + fName + ":";
   }

   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Konstruktør- & destruktør-dokumentasjon";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Gå til kildekoden til denne filen.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Gå til dokumentasjonen til denne filen.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Forhåndsbetingelse";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Etterbetingelse";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Invariant";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Startverdi:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "kode";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Grafisk klassehierarki";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Gå til det grafiske klasse hierarkiet";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Gå til tekst-klassehierarki";
   }
   virtual QByteArray trPageIndex() {
      return "Innhold";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Notat";
   }
   virtual QByteArray trPublicTypes() {
      return "Public typer";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config::getBool("optimize-c")) {
         return "Datafelt";
      } else {
         return "Public attributter";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Statiske public attributter";
   }
   virtual QByteArray trProtectedTypes() {
      return "Protected typer";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Protected attributter";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Statiske protected attributter";
   }
   virtual QByteArray trPrivateTypes() {
      return "Private typer";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Private attributter";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Statiske private attributter";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a todo item */
   virtual QByteArray trTodo() {
      return "Todo";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Todo-liste";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Referert av";
   }
   virtual QByteArray trRemarks() {
      return "Merknader";
   }
   virtual QByteArray trAttention() {
      return "Viktig";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Denne grafen viser hvilke filer som direkte eller "
             "indirekte inkluderer denne filen:";
   }
   virtual QByteArray trSince() {
      return "Siden";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Symbolforklaring";
   }
   /*! page explaining how the dot graph's should be interpreted */
   virtual QByteArray trLegendDocs() {
      return
         "Denne siden forklarer hvordan man tolker grafene DoxyPress genererer.<p>\n"
         "Vi baserer oss på følgende eksempel:\n"
         "\\code\n"
         "/*! Usynlig klasse pga. trunkasjon */\n"
         "class Invisible { };\n\n"
         "/*! Trunkert klasse, arve-relasjon er skjult */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Klasse som ikke er dokumentert med DoxyPress-kommentarer */"
         "class Undocumented { };\n\n"
         "/*! Klasse med public-arv */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! A template class */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Klasse med protected-arv */\n"
         "class ProtectedBase { };\n\n"
         "/*! Klasse med private-arv */\n"
         "class PrivateBase { };\n\n"
         "/*! Klasse som blir brukt av klassen Inherited */\n"
         "class Used { };\n\n"
         "/*! Super-klasse som arver flere andre klasser */\n"
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
         "Hvis \\c MAX_DOT_GRAPH_HEIGHT er satt til 200 i "
         "konfigurasjonsfila vil dette resultere i følgende graf:"
         "<p><center><img src=\"graph_legend." + Config::getEnum("dot-image-format").toUtf8() + "\"></center>\n"
         "<p>\n"
         "Boksene i grafen over betyr følgende:\n"
         "<ul>\n"
         "<li>En fylt svart boks representerer klassen grafen "
         "er generert for.\n"
         "<li>En boks med svart ramme angir en dokumentert struct eller klasse.\n"
         "<li>En boks med grå ramme angir en udokumentert struct eller klasse.\n"
         "<li>En boks med rød ramme angir en dokumentert struct eller klasse "
         "der ikke alle relasjoner er vist. En graf blir trunkert om den ikke "
         "passer innenfor de spesifiserte rammene.\n"
         "</ul>\n"
         "Pilene i grafen har følgende betydning:\n"
         "</ul>\n"
         "<li>En mørk blå pil brukes til å visualisere public-arv mellom to klasser.\n"
         "<li>En mørk grønn pil brukes for protected-arv.\n"
         "<li>En mørk rød pil angir private-arv.\n"
         "<li>En stiplet lilla pil angir at en klasse er inkludert eller brukt "
         "i en annen klasse. Pilen er merket med variablen(e) klassen "
         "er tilgjengelig gjennom.\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "symbolforklaring";
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
      return "Test-liste";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Egenskaper";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Egenskaps-dokumentasjon";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java interfaces in the summary section of Java packages */
   virtual QByteArray trInterfaces() {
      return "Grensesnitt";
   }
   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config::getBool("optimize-c")) {
         return "Datastrukturer";
      } else {
         return "Klasser";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Package " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Pakke-liste";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Her er pakkene med korte beskrivelser (om tilgjengelig):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Pakker";
   }
   /*! Used as a chapter title for Latex & RTF output */
   virtual QByteArray trPackageDocumentation() {
      return "Pakke-dokumentasjon";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Verdi:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Feil";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Feil-liste";
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
      QByteArray result((first_capital ? "Klasse" : "klasse"));
      if (!singular) {
         result += "r";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Fil" : "fil"));
      if (!singular) {
         result += "er";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Navnerom" : "navnerom"));
      if (!singular) {
         result += "";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Gruppe" : "gruppe"));
      if (!singular) {
         result += "r";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Side" : "side"));
      if (!singular) {
         result += "r";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Medlem" : "medlem"));
      if (!singular) {
         result += "mer";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trField(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Felt" : "felt"));
      if (!singular) {
         result += "";
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
         result += "e";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Forfatter" : "forfatter"));
      if (!singular) {
         result += "e";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Referanser";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Implementerer " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Implementert i " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Innholdsfortegnelse";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Liste over foreldede enheter";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Hendelser";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Hendelsesdokumentasjon";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Pakketyper";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Pakkefunksjoner";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Statiske Pakkefunksjoner";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Pakkeattributter";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Statiske Pakkeattributter";
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
      return "Her er kall-grafen for denne funksjonen:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Søkeresultater";
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
         return "Beklager, men ingen dokumenter ble funnet.";
      } else if (numDocuments == 1) {
         return "Fant <b>ett</b> dokument som passet ditt søk.";
      } else {
         return "Fant <b>$num</b> dokumenter som passet ditt søk. "
                "Viser beste treff først.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Treff:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
        */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return "Kildefil " + filename;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Katalog hierarki";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Katalogdokumentasjon";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Kataloger";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Denne katalogen er grovsortert alfabetisk "
             "(ikke nødvendigvis korrekt).";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " Katalog referanse";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Katalog" : "katalog"));
      if (!singular) {
         result += "er";
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
      return "Dette er en overloaded medlemsfunksjon, "
             "generert for deg. Den skiller seg ut fra "
             "funksjonen ovenfor i argument(ene) den aksepterer.";
   }
};

#endif
