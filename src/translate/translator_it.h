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

#ifndef TRANSLATOR_IT_H
#define TRANSLATOR_IT_H

class TranslatorItalian : public TranslatorAdapter_1_8_2
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
      return "italian";
   }

   /*! Used to get the LaTeX command(s) for the language support. This method
    *  was designed for languages which do wish to use a babel package.
    */
   virtual QByteArray latexLanguageSupportCommand() {
      return "\\usepackage[italian]{babel}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   QByteArray trRelatedFunctions() {
      return "Funzioni collegate";
   }

   /*! subscript for the related functions. */
   QByteArray trRelatedSubscript() {
      return "(Si noti che queste non sono funzioni membro.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   QByteArray trDetailedDescription() {
      return "Descrizione dettagliata";
   }

   /*! header that is put before the list of typedefs. */
   QByteArray trMemberTypedefDocumentation() {
      return "Documentazione delle ridefinizioni dei tipi (typedef)";
   }

   /*! header that is put before the list of enumerations. */
   QByteArray trMemberEnumerationDocumentation() {
      return "Documentazione dei tipi enumerati (enum)";
   }

   /*! header that is put before the list of member functions. */
   QByteArray trMemberFunctionDocumentation() {
      return "Documentazione delle funzioni membro";
   }

   /*! header that is put before the list of member attributes. */
   QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Documentazione dei campi";
      } else {
         return "Documentazione dei membri dato";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   QByteArray trMore() {
      return "Continua...";
   }

   /*! put in the class documentation */
   QByteArray trListOfAllMembers() {
      return "Elenco di tutti i membri";
   }

   /*! used as the title of the "list of all members" page of a class */
   QByteArray trMemberList() {
      return "Elenco dei membri";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   QByteArray trThisIsTheListOfAllMembers() {
      return "Questo è l'elenco completo di tutti i membri di ";
   }

   /*! this is the remainder of the sentence after the class name */
   QByteArray trIncludingInheritedMembers() {
      return ", inclusi tutti i membri ereditati.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Generato automaticamente da Doxygen";
      if (s) {
         result += (QByteArray)" per " + s;
      }
      result += " a partire dal codice sorgente.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   QByteArray trEnumName() {
      return "nome di tipo enumerato";
   }

   /*! put after an enum value in the list of all members */
   QByteArray trEnumValue() {
      return "valore di tipo enumerato";
   }

   /*! put after an undocumented member in the list of all members */
   QByteArray trDefinedIn() {
      return "definito in";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   QByteArray trModules() {
      return "Moduli";
   }

   /*! This is put above each page as a link to the class hierarchy */
   QByteArray trClassHierarchy() {
      return "Gerarchia delle classi";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Strutture dati";
      } else {
         return "Elenco dei tipi composti";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   QByteArray trFileList() {
      return "Elenco dei file";
   }

   /*! This is put above each page as a link to all members of compounds. */
   QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Campi dei dati";
      } else {
         return "Membri dei composti";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Elementi globali";
      } else {
         return "Elementi dei file";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   QByteArray trRelatedPages() {
      return "Pagine collegate";
   }

   /*! This is put above each page as a link to all examples. */
   QByteArray trExamples() {
      return "Esempi";
   }

   /*! This is put above each page as a link to the search engine. */
   QByteArray trSearch() {
      return "Cerca";
   }

   /*! This is an introduction to the class hierarchy. */
   QByteArray trClassHierarchyDescription() {
      return "Questo elenco di ereditarietà è ordinato "
             "approssimativamente, ma non completamente, in ordine alfabetico:";
   }

   /*! This is an introduction to the list with all files. */
   QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Questo è un elenco ";
      if (!extractAll) {
         result += "dei file documentati ";
      } else {
         result += "di tutti i file ";
      }
      result += "con una loro breve descrizione:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   QByteArray trCompoundListDescription() {

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Queste sono le strutture dati con una loro breve descrizione:";
      } else {
         return "Queste sono le classi, le struct, le union e le interfacce con una loro breve descrizione:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Questo è un elenco ";
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         if (!extractAll) {
            result += "delle struct e delle union documentate ";
         } else {
            result += "di tutte le struct e le union ";
         }
      } else {
         if (!extractAll) {
            result += "dei membri documentati ";
         } else {
            result += "di tutti i membri ";
         }
      }
      result += "con collegamenti alla documentazione ";
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         if (extractAll) {
            result += "della struct/union per ciascun campo:";
         } else {
            result += "delle struct/union a cui appartengono:";
         }
      } else {
         if (extractAll) {
            result += "della classe a cui appartengono:";
         } else {
            result += "delle classi a cui appartengono:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Questo è un elenco ";
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         if (!extractAll) {
            result += "delle funzioni, delle variabili, delle define, dei tipi enumerati e delle ridefinizioni dei tipi (typedef) documentate ";
         } else {
            result += "di tutte le funzioni, variabili, define, tipi enumerati, e ridefinizioni dei tipi (typedef) ";
         }
      } else {
         if (!extractAll) {
            result += "degli elementi documentati dei file ";
         } else {
            result += "di tutti gli elementi dei file ";
         }
      }
      result += "con collegamenti alla documentazione";
      if (extractAll) {
         result += " del file a cui appartengono:";
      } else {
         result += ":";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   QByteArray trExamplesDescription() {
      return "Questo è l'elenco di tutti gli esempi:";
   }

   /*! This is an introduction to the page with the list of related pages */
   QByteArray trRelatedPagesDescription() {
      return "Questo è l'elenco di tutte le pagine di documentazione collegate:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   QByteArray trModulesDescription() {
      return "Questo è l'elenco di tutti i moduli:";
   }

   // index titles (the project name is prepended for these)

   /*! This is used in HTML as the title of index.html. */
   QByteArray trDocumentation() {
      return "Documentazione";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   QByteArray trModuleIndex() {
      return "Indice dei moduli";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   QByteArray trHierarchicalIndex() {
      return "Indice della gerarchia";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Indice delle strutture dati";
      } else {
         return "Indice dei tipi composti";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   QByteArray trFileIndex() {
      return "Indice dei file";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   QByteArray trModuleDocumentation() {
      return "Documentazione dei moduli";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   QByteArray trClassDocumentation() {
      return "Documentazione delle classi";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   QByteArray trFileDocumentation() {
      return "Documentazione dei file";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   QByteArray trExampleDocumentation() {
      return "Documentazione degli esempi";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   QByteArray trPageDocumentation() {
      return "Documentazione delle pagine tra loro collegate ";
   }

   /*! This is used in LaTeX as the title of the document */
   QByteArray trReferenceManual() {
      return "Manuale di riferimento";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   QByteArray trDefines() {
      return "Definizioni";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   QByteArray trTypedefs() {
      return "Ridefinizioni di tipo (typedef)";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   QByteArray trEnumerations() {
      return "Tipi enumerati (enum)";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   QByteArray trFunctions() {
      return "Funzioni";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   QByteArray trVariables() {
      return "Variabili";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   QByteArray trEnumerationValues() {
      return "Valori del tipo enumerato";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   QByteArray trDefineDocumentation() {
      return "Documentazione delle definizioni";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   QByteArray trTypedefDocumentation() {
      return "Documentazione delle ridefinizioni di tipo (typedef)";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   QByteArray trEnumerationTypeDocumentation() {
      return "Documentazione dei tipi enumerati";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   QByteArray trFunctionDocumentation() {
      return "Documentazione delle funzioni";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   QByteArray trVariableDocumentation() {
      return "Documentazione delle variabili";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Strutture dati";
      } else {
         return "Composti";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Generato " + date;
      if (projName) {
         result += (QByteArray)" per " + projName;
      }
      result += (QByteArray)" da";
      return result;
   }

   /*! this text is put before a class diagram */
   QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Diagramma delle classi per " + clName;
   }

   /*! this text is generated when the \\internal command is used. */
   QByteArray trForInternalUseOnly() {
      return "Solo per uso interno.";
   }

   /*! this text is generated when the \\warning command is used. */
   QByteArray trWarning() {
      return "Avvertimento";
   }

   /*! this text is generated when the \\version command is used. */
   QByteArray trVersion() {
      return "Versione";
   }

   /*! this text is generated when the \\date command is used. */
   QByteArray trDate() {
      return "Data";
   }

   /*! this text is generated when the \\return command is used. */
   QByteArray trReturns() {
      return "Restituisce";
   }

   /*! this text is generated when the \\sa command is used. */
   QByteArray trSeeAlso() {
      return "Si veda anche";
   }

   /*! this text is generated when the \\param command is used. */
   QByteArray trParameters() {
      return "Parametri";
   }

   /*! this text is generated when the \\exception command is used. */
   QByteArray trExceptions() {
      return "Eccezioni";
   }

   /*! this text is used in the title page of a LaTeX document. */
   QByteArray trGeneratedBy() {
      return "Generato da";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   QByteArray trNamespaceList() {
      return "Lista dei namespace";
   }

   /*! used as an introduction to the namespace list */
   QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Questa è l'elenco ";
      if (!extractAll) {
         result += "dei namespace documentati, ";
      } else {
         result += "di tutti i namespace ";
      }
      result += "con una loro breve descrizione:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   QByteArray trFriends() {
      return "Friend";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   QByteArray trRelatedFunctionDocumentation() {
      return "Documentazione dei friend e delle funzioni collegate";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   QByteArray trCompoundReference(const char *clName,
                                  ClassDef::CompoundType compType,
                                  bool isTemplate) {
      QByteArray result = "Riferimenti per ";
      if (isTemplate) {
         result = "Template per ";
      }
      switch (compType) {
         case ClassDef::Class:
            result += "la classe ";
            break;
         case ClassDef::Struct:
            result += "la struct ";
            break;
         case ClassDef::Union:
            result += "la union ";
            break;
         case ClassDef::Interface:
            result += "l'interfaccia ";
            break;
         case ClassDef::Protocol:
            result += "il protocollo ";
            break;
         case ClassDef::Category:
            result += "la categoria ";
            break;
         case ClassDef::Exception:
            result += "l'eccezione ";
            break;
         default:
            break;
      }
      result += (QByteArray)clName;
      return result;

   }

   /*! used as the title of the HTML page of a file */
   QByteArray trFileReference(const char *fileName) {
      QByteArray result = "Riferimenti per il file ";
      result += (QByteArray)fileName;
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = "Riferimenti per il namespace ";
      result += (QByteArray)namespaceName;
      return result;
   }

   /* these are for the member sections of a class, struct or union */
   QByteArray trPublicMembers() {
      return "Membri pubblici";
   }
   QByteArray trPublicSlots() {
      return "Slot pubblici";
   }
   QByteArray trSignals() {
      return "Signal";
   }
   QByteArray trStaticPublicMembers() {
      return "Membri pubblici statici";
   }
   QByteArray trProtectedMembers() {
      return "Membri protetti";
   }
   QByteArray trProtectedSlots() {
      return "Slot protetti";
   }
   QByteArray trStaticProtectedMembers() {
      return "Membri protetti statici";
   }
   QByteArray trPrivateMembers() {
      return "Membri privati";
   }
   QByteArray trPrivateSlots() {
      return "Slot privati";
   }
   QByteArray trStaticPrivateMembers() {
      return "Membri privati statici";
   }

   /*! this function is used to produce a comma-separated list of items.
    *  use generateMarker(i) to indicate where item i should be put.
    */
   QByteArray trWriteList(int numEntries) {
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
               result += ", e ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   QByteArray trInheritsList(int numEntries) {
      return "Eredita da " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   QByteArray trInheritedByList(int numEntries) {
      return "Base per " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   QByteArray trReimplementedFromList(int numEntries) {
      return "Reimplementa " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   QByteArray trReimplementedInList(int numEntries) {
      return "Reimplementata in " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   QByteArray trNamespaceMembers() {
      return "Membri dei namespace";
   }

   /*! This is an introduction to the page with all namespace members */
   QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Questa è la lista ";
      if (!extractAll) {
         result += "dei membri documentati del namespace, ";
      } else {
         result += "di tutti i membri del namespace ";
      }
      result += "con collegamenti ";
      if (extractAll) {
         result += "alla documentazione del namespace per ciascun membro:";
      } else {
         result += "ai namespace a cui appartengono:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   QByteArray trNamespaceIndex() {
      return "Indice dei namespace";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   QByteArray trNamespaceDocumentation() {
      return "Documentazione dei namespace";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   QByteArray trNamespaces() {
      return "Namespace";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990728
   //////////////////////////////////////////////////////////////////////////

   /*! This is put at the bottom of a class documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   QByteArray trGeneratedFromFiles(ClassDef::CompoundType compType,
                                   bool single) {
      // here s is one of " Class", " Struct" or " Union"
      // single is true implies a single file
      QByteArray result = (QByteArray)"La documentazione per quest";
      switch (compType) {
         case ClassDef::Class:
            result += "a classe";
            break;
         case ClassDef::Struct:
            result += "a struct";
            break;
         case ClassDef::Union:
            result += "a union";
            break;
         case ClassDef::Interface:
            result += "a interfaccia";
            break;
         case ClassDef::Protocol:
            result += "o protocollo";
            break;
         case ClassDef::Category:
            result += "a categoria";
            break;
         case ClassDef::Exception:
            result += "a eccezione";
            break;
         default:
            break;
      }
      result += " è stata generata a partire ";
      if (single) {
         result += "dal seguente file:";
      } else {
         result += "dai seguenti file:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   QByteArray trReturnValues() {
      return "Valori di ritorno";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   QByteArray trMainPage() {
      return "Pagina Principale";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   QByteArray trPageAbbreviation() {
      return "pag.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   QByteArray trDefinedAtLineInSourceFile() {
      return "Definizione alla linea @0 del file @1.";
   }
   QByteArray trDefinedInSourceFile() {
      return "Definizione nel file @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   QByteArray trDeprecated() {
      return "Deprecato";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Diagramma di collaborazione per " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Grafo delle dipendenze di inclusione per " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   QByteArray trConstructorDocumentation() {
      return "Documentazione dei costruttori e dei distruttori";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   QByteArray trGotoSourceCode() {
      return "Vai al codice sorgente di questo file.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   QByteArray trGotoDocumentation() {
      return "Vai alla documentazione di questo file.";
   }
   /*! Text for the \\pre command */
   QByteArray trPrecondition() {
      return "Precondizione";
   }
   /*! Text for the \\post command */
   QByteArray trPostcondition() {
      return "Postcondizione";
   }
   /*! Text for the \\invariant command */
   QByteArray trInvariant() {
      return "Invariante";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   QByteArray trInitialValue() {
      return "Valore iniziale:";
   }
   /*! Text used the source code in the file index */
   QByteArray trCode() {
      return "codice";
   }
   QByteArray trGraphicalHierarchy() {
      return "Grafico della gerarchia delle classi";
   }
   QByteArray trGotoGraphicalHierarchy() {
      return "Vai al grafico della gerarchia delle classi";
   }
   QByteArray trGotoTextualHierarchy() {
      return "Vai alla gerarchia delle classi (testuale)";
   }
   QByteArray trPageIndex() {
      return "Indice delle pagine";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   QByteArray trNote() {
      return "Nota";
   }
   QByteArray trPublicTypes() {
      return "Tipi pubblici";
   }
   QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Campi";
      } else {
         return "Attributi pubblici";
      }
   }

   QByteArray trStaticPublicAttribs() {
      return "Attributi pubblici statici";
   }
   QByteArray trProtectedTypes() {
      return "Tipi protetti";
   }
   QByteArray trProtectedAttribs() {
      return "Attributi protetti";
   }
   QByteArray trStaticProtectedAttribs() {
      return "Attributi protetti statici";
   }
   QByteArray trPrivateTypes() {
      return "Tipi privati";
   }
   QByteArray trPrivateAttribs() {
      return "Attributi privati";
   }
   QByteArray trStaticPrivateAttribs() {
      return "Attributi privati statici";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a todo item */
   virtual QByteArray trTodo() {
      return "Da fare";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Elenco delle cose da fare";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Referenziato da";
   }
   virtual QByteArray trRemarks() {
      return "Osservazioni";
   }
   virtual QByteArray trAttention() {
      return "Attenzione";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Questo grafo mostra quali altri file includono direttamente o indirettamente questo file:";
   }
   virtual QByteArray trSince() {
      return "A partire da";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Legenda del grafo";
   }
   /*! page explaining how the dot graph's should be interpreted */
   virtual QByteArray trLegendDocs() {
      return
         "Questa pagina spiega come interpretare i grafi generati da doxygen.<p>\n"
         "Si consideri l'esempio seguente:\n"
         "\\code\n"
         "/*! Classe invisibile per troncamento */\n"
         "class Invisible { };\n\n"
         "/*! Classe tronca, la relazione di ereditarietà è nascosta */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Classe non documentata con i commenti speciali di doxygen*/\n"
         "class Undocumented { };\n\n"
         "/*! Classe estesa mediante ereditarietà pubblica */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! Classe templatizzata */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Classe estesa mediante ereditarietà protetta*/\n"
         "class ProtectedBase { };\n\n"
         "/*! Classe estesa mediante ereditarietà privata*/\n"
         "class PrivateBase { };\n\n"
         "/*! Classe utilizzata dalla classe Inherited */\n"
         "class Used { };\n\n"
         "/*! Classe che eredita da varie classi*/\n"
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
         "Verrà prodotto il grafo seguente:"
         "<p><center><img src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center></p>\n"
         "<p>\n"
         "I riquadri nel grafo qui sopra hanno il seguente significato:\n"
         "</p>\n"
         "<ul>\n"
         "<li>Il riquadro grigio pieno rappresenta la struct o la classe per la quale il grafo è stato generato.</li>\n"
         "<li>Un riquadro con un bordo nero denota una struct o una classe documentata.</li>\n"
         "<li>Un riquadro con un bordo grigio indica una struct o una classe non documentata.</li>\n"
         "<li>Un riquadro con un bordo rosso indica una struct o una classe per la quale non sono mostrate tutte le relazioni di ereditarietà/contenimento (un grafo viene troncato se non rientra nei limiti prestabiliti).</li>\n"
         "</ul>\n"
         "<p>\n"
         "Le frecce hanno il seguente significato:\n"
         "</p>\n"
         "<ul>\n"
         "<li>Una freccia blu scuro indica una relazione di ereditarietà pubblica tra due classi.</li>\n"
         "<li>Una freccia verde indica un'ereditarietà protetta.</li>\n"
         "<li>Una freccia rossa indica un'ereditarietà privata.</li>\n"
         "<li>Una freccia viola tratteggiata indica che una classe è contenuta o usata da un'altra classe."
         " La freccia viene etichettata con la o le variabili attraverso cui la struct o la classe puntata dalla freccia è accessibile.</li>\n"
         "<li>Una freccia gialla tratteggiata indica la relazione tra una istanza di un template e la classe templatizzata da cui è stata istanziata."
         " La freccia viene etichettata con i parametri di template dell'istanza.</li>\n"
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
      return "Lista dei test";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Proprietà";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Documentazione delle proprietà";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Strutture dati";
      } else {
         return "Classi";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Package " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Lista dei package";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Questi sono i package e una loro breve descrizione (se disponibile):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Package";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Valore:";
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
      return "Lista dei bug";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.6
   //////////////////////////////////////////////////////////////////////////

   /*! Used as ansicpg for RTF file */
   virtual QByteArray trRTFansicp() {
      return "1252";
   }

   /*! Used as ansicpg for RTF fcharset */
   virtual QByteArray trRTFCharSet() {
      return "0";
   }

   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "Indice";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Class" : "class"));
      result += (singular ? "e" : "i");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool) {
      QByteArray result((first_capital ? "File" : "file"));
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool) {
      QByteArray result((first_capital ? "Namespace" : "namespace"));
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Grupp" : "grupp"));
      result += (singular ? "o" : "i");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Pagin" : "pagin"));
      result += (singular ? "a" : "e");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Membr" : "membr"));
      result += (singular ? "o" : "i");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Global" : "global"));
      result += (singular ? "e" : "i");
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Autor" : "autor"));
      result += (singular ? "e" : "i");
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Referenzia";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Implementa " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implementation this member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Implementato in " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Sommario";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Lista degli elementi deprecati";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Eventi";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Documentazione degli eventi";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Tipi con visibilità di package";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Funzioni con visibilità di package";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Funzioni statiche con visibilità di package";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Attributi con visibilità di package";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Attributi statici con visibilità di package";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Tutto";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Questo è il grafo delle chiamate per questa funzione:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Risultati della ricerca";
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
         return "Nessun documento soddisfa la tua richiesta.";
      } else if (numDocuments == 1) {
         return "Trovato <b>1</b> documento che soddisfa la tua richiesta.";
      } else {
         return "Trovati <b>$num</b> documenti che soddisfano la tua richiesta. "
                "Le corrispondenze migliori sono in testa.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Corrispondenze:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return " File sorgente " + filename ;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Gerarchia delle directory";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Documentazione delle directory";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Directory";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Questa gerarchia di directory è approssimativamente, "
             "ma non completamente, ordinata in ordine alfabetico:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = "Riferimenti per la directory ";
      result += dirName;
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool) {
      QByteArray result((first_capital ? "Directory" : "directory"));
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.1
   //////////////////////////////////////////////////////////////////////////

   /*! This text is added to the documentation when the \\overload command
    *  is used for a overloaded function.
    */
   virtual QByteArray trOverloadText() {
      return "Questa è una funzione membro sovraccaricata (overloaded), "
             "fornita per comodità. Differisce dalla funzione di cui sopra "
             "unicamente per gli argomenti passati.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Questo è il grafo dei chiamanti di questa funzione:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Documentazione dei tipi enumerati";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Documentazione delle funzioni membro/subroutine";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Elenco dei tipi di dato";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Membri dei tipi di dato";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Questi sono i tipi dato con una loro breve descrizione:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Questa è una lista di tutti i membri ";
      if (!extractAll) {
         result += "documentati ";
      }
      result += "dei tipi di dato con collegamenti ";
      if (!extractAll) {
         result += "alla documentazione della struttura dati per ciascun membro:";
      } else {
         result += "ai tipi dato a cui appartengono:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Indice dei tipi dati";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Documentazione dei tipi dato";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Funzioni/Subroutine";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Documentazione funzioni/subroutine";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Tipi di dato";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Elenco dei moduli";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Questa è una lista di tutti i moduli ";
      if (!extractAll) {
         result += "documentati ";
      }
      result += "con una loro breve descrizione:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = "Riferimenti per ";
      if (isTemplate) {
         result = "Template per ";
      }
      switch (compType) {
         case ClassDef::Class:
            result += " il modulo";
            break;
         case ClassDef::Struct:
            result += " il tipo dato";
            break;
         case ClassDef::Union:
            result += " l'union";
            break;
         case ClassDef::Interface:
            result += " l'nterfaccia";
            break;
         case ClassDef::Protocol:
            result += " il protocollo";
            break;
         case ClassDef::Category:
            result += " la categoria";
            break;
         case ClassDef::Exception:
            result += " l'eccezione";
            break;
         default:
            break;
      }
      result += (QByteArray)clName;
      return result;
   }

   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = "Riferimenti per il modulo ";
      result += namespaceName;
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Membri del modulo";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Questo è un elenco di tutti i membri dei moduli ";
      if (!extractAll) {
         result += "documentati ";
      }
      result += "con collegamenti ";
      if (extractAll) {
         result += "alla documentazione del modulo per ciascun membro:";
      } else {
         result += "al modulo a cui appartengono:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Indice dei moduli";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Modul" : "modul"));
      if (singular) {
         result += "o";
      } else {
         result += "i";
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
      QByteArray result = (QByteArray)"La documentazione per quest";
      switch (compType) {
         case ClassDef::Class:
            result += "o modulo";
            break;
         case ClassDef::Struct:
            result += "o tipo";
            break;
         case ClassDef::Union:
            result += "a union";
            break;
         case ClassDef::Interface:
            result += "a interfaccia";
            break;
         case ClassDef::Protocol:
            result += "o protocollo";
            break;
         case ClassDef::Category:
            result += "a categoria";
            break;
         case ClassDef::Exception:
            result += "a eccezione";
            break;
         default:
            break;
      }
      result += " è stata generata a partire ";
      if (single) {
         result += "dal seguente file:";
      } else {
         result += "dai seguenti file:";
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
         result += "o";
      } else {
         result += "i";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Sottoprogramm" : "sottoprogramm"));
      if (singular) {
         result += "a";
      } else {
         result += "i";
      }
      return result;
   }

   /*! C# Type Contraint list */
   virtual QByteArray trTypeConstraints() {
      return "Vincoli dei tipi";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return "Relazione per " + QByteArray(name);
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Caricamento in corso...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Namespace globale";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Ricerca in corso...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Nessun risultato";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"File in " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Include il file in " + name;
   }

   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Riferimenti bibliografici";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Grafo di dipendenza delle directory per ") + name + ":";
   }

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "livello di dettaglio";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Parametri dei template";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "e altri " + number + " ...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "La documentazione per questo tipo enumerato è stata generata";
      if (!single) {
         result += " dai seguenti";
      } else {
         result += " dal seguente";
      }
      result += " file:";
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return QByteArray("Riferimenti per il tipo enumerato ") + QByteArray(name);
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " ereditati da " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Altri membri ereditati";
   }

};

#endif


