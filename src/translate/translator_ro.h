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

#ifndef TRANSLATOR_RO_H
#define TRANSLATOR_RO_H


class TranslatorRomanian : public Translator
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
      return "romanian";
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
      return "\\usepackage[romanian]{babel}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Funcţii înrudite";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Atenţie: acestea nu sunt funcţii membre.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Descriere Detaliată";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Documentaţia Definiţiilor de Tipuri (typedef) Membre";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Documentaţia Enumerărilor Membre";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Documentaţia Funcţiilor Membre";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Documentaţia Câmpurilor";
      } else {
         return "Documentaţia Datelor Membre";
      }

   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Mai mult...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Lista tuturor membrilor";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Lista Membrilor";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Lista completă a membrilor din ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", inclusiv a tuturor membrilor moşteniţi.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Generat automat de Doxygen";
      if (s) {
         result += (QByteArray)" pentru " + s;
      }
      result += " din codul sursă.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "nume enumerare";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "valoare enumerare";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "definit în";
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
      return "Ierarhia Claselor";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Structuri de Date";
      } else {
         return "Lista Claselor";
      }

   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Lista fişierelor";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Câmpurile de Date";
      } else {
         return "Membrii Componenţi"; //cu articol hotarat
      }

   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Globale";
      } else {
         return "Membrii din Fişier"; //cu articol hotarat
      }

   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Pagini înrudite";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Exemple";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Caută";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Această listă de moşteniri este sortată în general, "
             "dar nu complet, în ordine alfabetică:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Lista tuturor ";
      result += "fişierelor";
      if (!extractAll) {
         result += " documentate";
      }
      result += ", cu scurte descrieri:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Lista structurilor de date, cu scurte descrieri:";
      } else {
         return "Lista claselor, structurilor, uniunilor şi interfeţelor"
                ", cu scurte descrieri:";
      }

   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Lista tuturor ";

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "câmpurilor ";
         if (!extractAll) {
            result += " documentate ";
         }
         result += "din structuri si uniuni ";
      } else {
         result += "membrilor ";
         if (!extractAll) {
            result += "documentaţi ";
         }
         result += "din toate clasele ";
      }
      result += ", cu legături către ";
      if (!extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "documentaţia structurii/uniunii pentru fiecare câmp în parte:";
         } else {
            result += "documentaţia clasei pentru fiecare membru în parte:";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "structurile/uniunile de care aparţin:";
         } else {
            result += "clasele de care aparţin:";
         }
      }

      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Lista tuturor ";
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "funcţiilor, variabilelor, definiţiilor, enumerărilor şi definiţiilor de tip";
         if (!extractAll) {
            result += " documentate";
         }
      } else {
         result += "membrilor ";
         if (!extractAll) {
            result += "documentaţi ";
         }
         result += "din toate fişierele";
      }
      result += ", cu legături către ";
      if (extractAll) {
         result += "fişierele de care aparţin:";
      } else {
         result += "documentaţia aferentă:";
      }

      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Lista tuturor exemplelor:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Lista tuturor documentaţiilor înrudite:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Lista tuturor modulelor:";
   }

   // index titles (the project name is prepended for these)


   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Documentaţie";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Indexul Modulelor";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Index Ierarhic";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Indexul Structurilor de Date";
      } else {
         return "Indexul Claselor";
      }

   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Indexul Fişierelor";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Documentaţia Modulelor";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Documentaţia Structurilor de Date";
      } else {
         return "Documentaţia Claselor";
      }

   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Documentaţia Fişierelor";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Documentaţia Exemplelor";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Documentaţii înrudite";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Manual de utilizare";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Definiţii";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Definiţii de tipuri";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Enumerări";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Funcţii";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Variabile";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Valori de enumerări";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Documentaţia definiţiilor";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Documentaţia definiţiilor de tipuri";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Documentaţia enumerărilor";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Documentaţia funcţiilor";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Documentaţia variabilelor";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Structuri de Date";
      } else {
         return "Membri";
      }

   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Generat " + date;
      if (projName) {
         result += (QByteArray)" pentru " + projName;
      }
      result += (QByteArray)" de către";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Diagrama de relaţii pentru " + clName;
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Doar pentru uz intern.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Atenţie";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Versiunea";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Data";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Întoarce";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Vezi şi";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Parametri";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Excepţii";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Generat de";
   }

   // new since 0.49-990307

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Lista de Namespace-uri";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Lista tuturor ";
      result += "namespace-urilor ";
      if (!extractAll) {
         result += "documentate ";
      }
      result += ", cu scurte descrieri:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Prieteni";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Documentaţia funcţiilor prietene sau înrudite";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result = "Referinţă la ";
      switch (compType) {
         case ClassDef::Class:
            result += "clasa ";
            break;
         case ClassDef::Struct:
            result += "structura ";
            break;
         case ClassDef::Union:
            result += "uniunea ";
            break;
         case ClassDef::Interface:
            result += "interfaţa ";
            break;
         case ClassDef::Protocol:
            result += "protocolul ";
            break;
         case ClassDef::Category:
            result += "categoria ";
            break;
         case ClassDef::Exception:
            result += "excepţia ";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += " (Template) ";
      }
      result += (QByteArray)clName;

      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = "Referinţă la fişierul ";
      result += fileName;
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = "Referinţă la Namespace-ul ";
      result += namespaceName;
      return result;
   }

   /* these are for the member sections of a class, struct or union */
   virtual QByteArray trPublicMembers() {
      return "Metode Publice";
   }
   virtual QByteArray trPublicSlots() {
      return "Conectori (slots) Publici";
   }
   virtual QByteArray trSignals() {
      return "Semnale";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Metode Statice Publice";
   }
   virtual QByteArray trProtectedMembers() {
      return "Metode Protejate";
   }
   virtual QByteArray trProtectedSlots() {
      return "Conectori (slots) Protejaţi";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Metode Statice Protejate";
   }
   virtual QByteArray trPrivateMembers() {
      return "Metode Private";
   }
   virtual QByteArray trPrivateSlots() {
      return "Conectori (slots) Privaţi";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Metode Statice Private";
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
               result += " şi ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Moşteneşte " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Moştenit de " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Reimplementat din " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Reimplementat în " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Membrii Namespace-ului";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Lista tuturor membrilor ";
      if (!extractAll) {
         result += "documentaţi ";
      }
      result += "din toate namespace-urile, cu legături către ";

      if (extractAll) {
         result += "documentaţia namespace-ului pentru fiecare membru în parte:";
      } else {
         result += "namespace-urile de care aparţin:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Indexul Namespace-ului";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Documentaţia Namespace-ului";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Namespace-uri";
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
      QByteArray result = (QByteArray)"Documentaţia pentru această ";
      switch (compType) {
         case ClassDef::Class:
            result += "clasă";
            break;
         case ClassDef::Struct:
            result += "structură";
            break;
         case ClassDef::Union:
            result += "uniune";
            break;
         case ClassDef::Interface:
            result += "interfaţă";
            break;
         case ClassDef::Protocol:
            result += "protocol";
            break;
         case ClassDef::Category:
            result += "categorie";
            break;
         case ClassDef::Exception:
            result += "excepţie";
            break;
         default:
            break;
      }
      result += " a fost generată din fişier";
      if (single) {
         result += "ul:";
      } else {
         result += "ele:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Valori returnate";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Pagina principală";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "pg.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Definiţia în linia @0 a fişierului @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Definiţia în fişierul @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Învechită(Deprecated)";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Diagrama de relaţii pentru " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Graful dependenţelor prin incluziune pentru " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Documentaţia pentru Constructori şi Destructori";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Vezi sursele.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Vezi documentaţia.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Precondiţie";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Postcondiţie";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Invariant";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Valoarea iniţială:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "cod";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Ierarhia Claselor în mod grafic";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Vezi ierarhia claselor în mod grafic";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Vezi ierarhia claselor în mod text";
   }
   virtual QByteArray trPageIndex() {
      return "Indexul Paginilor";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Notă";
   }
   virtual QByteArray trPublicTypes() {
      return "Tipuri Publice";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Câmpuri de Date";
      } else {
         return "Atribute Publice";
      }

   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Atribute Statice Publice";
   }
   virtual QByteArray trProtectedTypes() {
      return "Tipuri Protejate";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Atribute Protejate";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Atribute Statice Protejate";
   }
   virtual QByteArray trPrivateTypes() {
      return "Tipuri Private";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Atribute Private";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Atribute Statice Private";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a todo item */
   virtual QByteArray trTodo() {
      return "De făcut";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Lista lucrurilor de făcut";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Semnalat de";
   }
   virtual QByteArray trRemarks() {
      return "Observaţii";
   }
   virtual QByteArray trAttention() {
      return "Atenţie";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Acest graf arată care fişiere includ, "
             "direct sau indirect, acest fişier:";
   }
   virtual QByteArray trSince() {
      return "Din";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Legenda grafului";
   }
   /*! page explaining how the dot graph's should be interpreted */
   virtual QByteArray trLegendDocs() {
      return
         "Această pagină arată modul în care trebuie să interpretaţi "
         "grafurile generate de doxygen.<p>\n"
         "Consideraţi următorul exemplu:\n"
         "\\code\n"
         "/*! Clasă invizibilă, tăiată din cauza depăşirii spaţiului */\n"
         "class Invisible { };\n\n"
         "/*! Altă clasă tăiată, relaţia de moştenire este ascunsă */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Clasă necomentată în stil doxygen */\n"
         "class Undocumented { };\n\n"
         "/*! Clasă care este moştenită în mod public */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! Clasă template */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Clasă care este moştenită în mod protejat */\n"
         "class ProtectedBase { };\n\n"
         "/*! Clasă care este moştenită în mod privat */\n"
         "class PrivateBase { };\n\n"
         "/*! Clasă care este folosită de clasa Inherited */\n"
         "class Used { };\n\n"
         "/*! Superclasă care moşteneşte un număr de alte clase */\n"
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
         "Dacă tagul \\c MAX_DOT_GRAPH_HEIGHT din fişierul de configurare "
         "este setat la 200, acesta este graful rezultat:"
         "<p><center><img src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
         "<p>\n"
         "Căsuţele din partea de sus au următoarea semnificaţie:\n"
         "<ul>\n"
         "<li>O căsuţă neagră reprezintă structura sau clasa pentru care "
         "graful este generat.\n"
         "<li>O căsuţă cu marginea neagră reprezintă o structură sau o clasă documentate.\n"
         "<li>O căsuţă cu marginea gri reprezintă o structură sau o clasă nedocumentate.\n"
         "<li>O căsuţă cu marginea roşie reprezintă o structură sau o clasă documentate, pentru\n"
         "care nu toate relaţiile de moştenire/incluziune sunt arătate. Un graf este "
         "tăiat dacă nu încape în marginile specificate."
         "</ul>\n"
         "Săgeţile au următoarea semnificaţie:\n"
         "<ul>\n"
         "<li>O săgeată de un albastru închis este folosită când avem o relaţie de "
         "moştenire publică între două clase.\n"
         "<li>O săgeată de un verde închis este folosită când avem o moştenire protejată.\n"
         "<li>O săgeată de un roşu închis este folosită când avem o moştenire privată.\n"
         "<li>O săgeată violetă punctată este folosită pentru o clasă conţinută sau folosită "
         "de o altă clasă. Săgeata este marcată cu variabila(e) "
         "prin care este accesibilă clasa sau structura spre care este îndreptată. \n"
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
      return "Test";
   }
   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "Listă de teste";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Proprietăţi";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Documentaţia Proprietăţilor";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Structuri de Date";
      } else {
         return "Clase";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Pachet " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Lista Pachetelor";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Lista pachetelor, însoţită de scurte explicaţii, acolo unde acestea există:";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Pachete";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Valoare:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Problema (Bug)";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Lista de Probleme (Bugs)";
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
      return "1250"; //EASTEUROPE_CHARSET
   }


   /*! Used as ansicpg for RTF fcharset
    *  \see trRTFansicp() for a table of possible values.
    */
   virtual QByteArray trRTFCharSet() {
      return "238";
   }

   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "Index";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Clas" : "clas"));
      result += singular ? "a" : "ele";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Fişier" : "fişier"));
      result += singular ? "ul" : "ele";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Namespace" : "namespace"));
      result += singular ? "-ul" : "-urile";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Grupu" : "grupu"));
      result += singular ? "l" : "rile";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Pagin" : "pagin"));
      result += singular ? "a" : "ile";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Membr" : "membr"));
      result += singular ? "ul" : "ii";
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
      QByteArray result((first_capital ? "Autor" : "autor"));
      result += singular ? "ul" : "ii";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Referinţe";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Implementează " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Implementat în " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Cuprins";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Lista elementelor învechite (deprecated)";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Evenimente";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Documentaţia aferentă evenimentelor";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Tipuri în pachet";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Funcţii în pachet";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Funcţii statice în pachet";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Atribute în pachet";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Atribute statice în pachet";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Toate";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Graful de apel al acestei funcţii:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Rezultatele căutarii";
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
         return "Din păcate nu am găsit nici un document care să corespundă cererii.";
      } else if (numDocuments == 1) {
         return "Am găsit <b>1</b> document corespunzând cererii.";
      } else {
         return "Am găsit <b>$num</b> documente corespunzând cererii. "
                "Lista documentelor găsite, sortate după relevanţă.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Găsite:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return " Fişierul sursă " + filename;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Ierarhia directoarelor";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Documentaţia directoarelor";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Directoare";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Această ierarhie de directoare este sortată în general, "
             "dar nu complet, în ordine alfabetică:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray  result = "Director-referinţă ";
      result += dirName;
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Directo" : "directo"));
      if (singular) {
         result += "r";
      } else {
         result = "are";
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
      return "Aceasta este o funcţie membră suprascrisă. "
             "Diferă de funcţia de mai sus "
             "doar prin argumentele acceptate.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Graful de apeluri pentru această funcţie:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Documentaţia Enumeratorilor";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Documentaţia Funcţiei Membre/Subrutinei";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Lista Tipurilor de Date";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Câmpuri de date";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Tipurile de date, cu scurte descrieri:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Lista tuturor tipurilor de date ";
      if (!extractAll) {
         result += "documentate ";
      }
      result += " cu legături către ";
      if (!extractAll) {
         result += "documentaţia structurii de date pentru fiecare membru";
      } else {
         result += "tipurile de date de care aparţin:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Indexul Tipurilor de Date";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Documentaţia Tipurilor de Date";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Funcţii/Subrutine";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Documentaţia Funcţiilor/Subrutinelor";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Tipuri de Date";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Lista Modulelor";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Lista tuturor modulelor ";
      if (!extractAll) {
         result += "documentate ";
      }
      result += "cu scurte descrieri:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = "Referinţă la ";
      switch (compType) {
         case ClassDef::Class:
            result += "Modulul ";
            break;
         case ClassDef::Struct:
            result += "Tipul ";
            break;
         case ClassDef::Union:
            result += "Uniunea ";
            break;
         case ClassDef::Interface:
            result += "Interfaţa ";
            break;
         case ClassDef::Protocol:
            result += "Protocolul ";
            break;
         case ClassDef::Category:
            result += "Categoria ";
            break;
         case ClassDef::Exception:
            result += "Excepţia ";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += "(Template) ";
      }
      result += (QByteArray)clName;
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = "Referinţă la Modulul ";
      result += namespaceName;
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Membrii Modulului";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Lista tuturor membrilor ";
      if (!extractAll) {
         result += "documentaţi ai ";
      }
      result += "modulului cu legături către ";
      if (extractAll) {
         result += "documentaţia modulului pentru fiecare membru:";
      } else {
         result += "modulele de care aparţin:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Indexul Modulelor";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Modul" : "modul"));
      if (singular) {
         result += "ul";
      } else {
         result += "ele";
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
      QByteArray result = (QByteArray)"Documentaţia ";
      switch (compType) {
         case ClassDef::Class:
            result += "modulului";
            break;
         case ClassDef::Struct:
            result += "tipului";
            break;
         case ClassDef::Union:
            result += "uniunii";
            break;
         case ClassDef::Interface:
            result += "interfeţei";
            break;
         case ClassDef::Protocol:
            result += "protocolului";
            break;
         case ClassDef::Category:
            result += "categoriei";
            break;
         case ClassDef::Exception:
            result += "excepţiei";
            break;
         default:
            break;
      }
      result += " a fost generată din următo";
      if (single) {
         result += "rul fişier:";
      } else {
         result += "arele fişiere:";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Tip" : "tip"));
      if (singular) {
         result += "ul";
      } else {
         result += "urile";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Subprogram" : "subprogram"));
      if (singular) {
         result += "ul";
      } else {
         result += "ele";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Constrângerile de Tip";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return QByteArray(name) + " Relație";
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Se încarcă...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Namespace Global";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Căutare...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Niciun rezultat";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Fișierul din " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Include fișierul din " + name;
   }

   /** Compiles a date string.
    *  @param year Year in 4 digits
    *  @param month Month of the year: 1=January
    *  @param day Day of the Month: 1..31
    *  @param dayOfWeek Day of the week: 1=Monday..7=Sunday
    *  @param hour Hour of the day: 0..23
    *  @param minutes Minutes in the hour: 0..59
    *  @param seconds Seconds within the minute: 0..59
    *  @param includeTime Include time in the result string?
    */
   virtual QByteArray trDateTime(int year, int month, int day, int dayOfWeek,
                                 int hour, int minutes, int seconds,
                                 bool includeTime) {
      static const char *days[]   = { "Luni", "Marți", "Miercuri", "Joi", "Vineri", "Sâmbătă", "Duminică" };
      static const char *months[] = { "Ian", "Feb", "Mar", "Apr", "Mai", "Iun", "Iul", "Aug", "Sep", "Oct", "Noi", "Dec" };
      QByteArray sdate;
      sdate.sprintf("%s %s %d %d", days[dayOfWeek - 1], months[month - 1], day, year);
      if (includeTime) {
         QByteArray stime;
         stime.sprintf(" %.2d:%.2d:%.2d", hour, minutes, seconds);
         sdate += stime;
      }
      return sdate;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.7.5
   //////////////////////////////////////////////////////////////////////////

   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Referințe Bibliografice";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Grafic de dependență a directoarelor pentru ") + name + ":";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.0
   //////////////////////////////////////////////////////////////////////////

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "nivel de detaliu";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Parametri Template";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "și încă " + number;
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "Documentația pentru acest enum a fost generată din ";
      if (single) {
         result += "următorul fișier:";
      } else {
         result += "următoarele fișiere:";
      }
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return QByteArray(name) + " Referință Enum";
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " moștenit(e) din " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Membri Moșteniți Adiționali";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      QByteArray opt = enable ? "activa" : "dezactiva";
      return "apasă click pentru a " + opt + " sincronizarea panourilor";
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "Furnizat de categoria @1.";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "Extinde clasa @1.";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "Metodele Clasei";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "Metodele Instanței";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "Documentația Metodelor";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      return "Vedere de Ansamblu a Designului";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.4
   //////////////////////////////////////////////////////////////////////////

   /** old style UNO IDL services: implemented interfaces */
   virtual QByteArray trInterfaces() {
      return "Interfețe exportate";
   }

   /** old style UNO IDL services: inherited services */
   virtual QByteArray trServices() {
      return "Servicii Incluse";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroups() {
      return "Grupuri Constante";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroupReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Referință Grup Constant";
      return result;
   }
   /** UNO IDL service page title */
   virtual QByteArray trServiceReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " Referință Serviciu";
      return result;
   }
   /** UNO IDL singleton page title */
   virtual QByteArray trSingletonReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " Referință Singleton";
      return result;
   }
   /** UNO IDL service page */
   virtual QByteArray trServiceGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"Documentația pentru acest serviciu "
                          "a fost generată din ";
      if (single) {
         result += "următorul fișier:";
      } else {
         result += "următoarele fișiere:";
      }
      return result;
   }
   /** UNO IDL singleton page */
   virtual QByteArray trSingletonGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"Documentația pentru acest singleton "
                          "a fost generată din ";
      if (single) {
         result += "următorul fișier:";
      } else {
         result += "următoarele fișiere:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////

};

#endif
