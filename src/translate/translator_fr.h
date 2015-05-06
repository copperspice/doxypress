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

#ifndef TRANSLATOR_FR_H
#define TRANSLATOR_FR_H

class TranslatorFrench : public Translator
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
      return "french";
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
      return "\\usepackage[french]{babel}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Fonctions associées";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Notez que ce ne sont pas des fonctions membres)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Description détaillée";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Documentation des définitions de type membres";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Documentation des énumérations membres";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Documentation des fonctions membres";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Documentation des champs";
      } else {
         return "Documentation des données membres";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Plus de détails...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Liste de tous les membres";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Liste des membres";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Liste complète des membres de ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", y compris les membres hérités :";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Généré automatiquement par DoxyPress";
      if (s) {
         result += (QByteArray)" pour " + s;
      }
      result += " à partir du code source.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "énumération";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "valeur énumérée";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "défini dans";
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
      return "Hiérarchie des classes";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config::getBool("optimize-c")) {
         return "Structures de données";
      } else {
         return "Liste des classes";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Liste des fichiers";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config::getBool("optimize-c")) {
         return "Champs de donnée";
      } else {
         return "Membres de classe";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config::getBool("optimize-c")) {
         return "Variables globale";
      } else {
         return "Membres de fichier";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Pages associées";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Exemples";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Recherche";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Cette liste d'héritage est classée "
             "approximativement par ordre alphabétique :";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Liste de tous les fichiers ";
      if (!extractAll) {
         result += "documentés ";
      }
      result += "avec une brève description :";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config::getBool("optimize-c")) {
         return "Liste des structures de données avec une brève description :";
      } else {
         return "Liste des classes, structures, "
                "unions et interfaces avec une brève description :";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Liste de tous les ";
      if (Config::getBool("optimize-c")) {
         result += "champs de structure et d'union ";
      } else {
         result += "membres de classe ";
      }
      if (!extractAll) {
         result += "documentés ";
      }
      result += "avec des liens vers ";
      if (!extractAll) {
         if (Config::getBool("optimize-c")) {
            result += "la documentation de structure/union de chaque champ :";
         } else {
            result += "la documentation de classe de chaque membre :";
         }
      } else {
         if (Config::getBool("optimize-c")) {
            result += "les structures/unions auxquelles ils appartiennent :";
         } else {
            result += "les classes auxquelles ils appartiennent :";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Liste ";

      if (Config::getBool("optimize-c")) {
         result += "de toutes les fonctions, variables, macros, enumérations, et définitions de type ";
      } else {
         result += "de tous les membres de fichier ";
      }
      if (!extractAll) {
         result += "documentés ";
      }
      result += "avec des liens vers ";
      if (extractAll) {
         result += "les fichiers auxquels ils appartiennent :";
      } else {
         result += "la documentation :";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Liste de tous les exemples :";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Liste de toutes les pages de documentation associées :";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Liste de tous les modules :";
   }

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Documentation";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Index des modules";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Index hiérarchique";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config::getBool("optimize-c")) {
         return "Index des structures de données";
      } else {
         return "Index des classes";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Index des fichiers";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Documentation des modules";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Documentation des structures de données";
      } else {
         return "Documentation des classes";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Documentation des fichiers";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Documentation des exemples";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Documentation des pages associées";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Manuel de référence";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Macros";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Définitions de type";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Énumérations";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Fonctions";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Variables";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Valeurs énumérées";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Documentation des macros";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Documentation des définitions de type";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Documentation du type de l'énumération";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Documentation des fonctions";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Documentation des variables";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config::getBool("optimize-c")) {
         return "Structures de données";
      } else {
         return "Classes";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Généré le " + date;
      if (projName) {
         result += (QByteArray)" pour " + projName;
      }
      result += (QByteArray)" par";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Graphe d'héritage de " + clName + ":";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Pour un usage interne uniquement.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Avertissement";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Version";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Date";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Renvoie";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Voir également";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Paramètres";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Exceptions";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Généré par";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Liste des espaces de nommage";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Liste de tous les espaces de nommage ";
      if (!extractAll) {
         result += "documentés ";
      }
      result += "avec une brève description:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Amis";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Documentation des fonctions amies et associées";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result = "Référence ";
      if (isTemplate) {
         result += "du modèle ";
      }
      result += "de ";
      switch (compType) {
         case ClassDef::Class:
            result += "la classe ";
            break;
         case ClassDef::Struct:
            result += "la structure ";
            break;
         case ClassDef::Union:
            result += "l'union ";
            break;
         case ClassDef::Interface:
            result += "l'interface ";
            break;
         case ClassDef::Protocol:
            result += "le protocol ";
            break;
         case ClassDef::Category:
            result += "la catégorie ";
            break;
         case ClassDef::Exception:
            result += "l'exception ";
            break;
         default:
            break;
      }
      result += (QByteArray)clName;
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = "Référence du fichier ";
      result += fileName;
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = "Référence de l'espace de nommage ";
      result += namespaceName;
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Fonctions membres publiques";
   }
   virtual QByteArray trPublicSlots() {
      return "Connecteurs publics";
   }
   virtual QByteArray trSignals() {
      return "Signaux";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Fonctions membres publiques statiques";
   }
   virtual QByteArray trProtectedMembers() {
      return "Fonctions membres protégées";
   }
   virtual QByteArray trProtectedSlots() {
      return "Connecteurs protégés";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Fonctions membres protégées statiques";
   }
   virtual QByteArray trPrivateMembers() {
      return "Fonctions membres privées";
   }
   virtual QByteArray trPrivateSlots() {
      return "Connecteurs privés";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Fonctions membres privées statiques";
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
               result += ", et ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Est dérivée de " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Dérivée par " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Réimplémentée à partir de " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Réimplémentée dans " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Membres de l'espace de nommage";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Liste de tous les membres des espaces de nommage ";
      if (!extractAll) {
         result += "documentés ";
      }
      result += "avec des liens vers ";
      if (extractAll) {
         result += "la documentation de namespace de chaque membre :";
      } else {
         result += "les espaces de nommage auxquels ils appartiennent :";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Index des espaces de nommage";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Documentation des espaces de nommage";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Espaces de nommage";
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
      bool female = true;
      QByteArray result = (QByteArray)"La documentation de ";
      switch (compType) {
         case ClassDef::Class:
            result += "cette classe";
            break;
         case ClassDef::Struct:
            result += "cette structure";
            break;
         case ClassDef::Union:
            result += "cette union";
            break;
         case ClassDef::Interface:
            result += "cette interface";
            break;
         case ClassDef::Protocol:
            result += "ce protocol";
            female = false;
            break;
         case ClassDef::Category:
            result += "cette catégorie";
            break;
         case ClassDef::Exception:
            result += "cette exception";
            break;
         default:
            break;
      }
      if (female) {
         result += " a été générée à partir ";
      } else {
         result += " a été généré à partir ";
      }
      if (single) {
         result += "du fichier suivant :";
      } else {
         result += "des fichiers suivants :";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Valeurs retournées";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Page principale";
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
      return "Définition à la ligne @0 du fichier @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Définition dans le fichier @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Obsolète";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Graphe de collaboration de " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Graphe des dépendances par inclusion de " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Documentation des constructeurs et destructeur";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Aller au code source de ce fichier.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Aller à la documentation de ce fichier.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Précondition";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Postcondition";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Invariant";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Valeur initiale :";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "code";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Graphe hiérarchique des classes";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Aller au graphe hiérarchique des classes";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Aller à la hiérarchie des classes en mode texte";
   }
   virtual QByteArray trPageIndex() {
      return "Index des pages";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Note";
   }
   virtual QByteArray trPublicTypes() {
      return "Types publics";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config::getBool("optimize-c")) {
         return "Champs de données";
      } else {
         return "Attributs publics";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Attributs publics statiques";
   }
   virtual QByteArray trProtectedTypes() {
      return "Types protégés";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Attributs protégés";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Attributs protégés statiques";
   }
   virtual QByteArray trPrivateTypes() {
      return "Types privés";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Attributs privés";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Attributs privés statiques";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "A faire";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Liste des choses à faire";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Référencé par";
   }
   virtual QByteArray trRemarks() {
      return "Remarques";
   }
   virtual QByteArray trAttention() {
      return "Attention";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Ce graphe montre quels fichiers incluent directement "
             "ou indirectement ce fichier :";
   }
   virtual QByteArray trSince() {
      return "Depuis";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Légende du graphe";
   }
   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Cette page explique comment interpréter les graphes générés "
         "par DoxyPress.<p>\n"
         "Considérez l'exemple suivant :\n"
         "\\code\n"
         "/*! Classe invisible à cause d'une troncature */\n"
         "class Invisible { };\n\n"
         "/*! Classe tronquée, la relation d'héritage est masquée */\n"
         "class Truncated : public Invisible { };\n\n"
         "/*! Classe non documentée avec des commentaires DoxyPress */\n"
         "class Undocumented { };\n\n"
         "/*! Classe dérivée par héritage public */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! Un modèle de classe */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Classe dérivée par héritage protégé */\n"
         "class ProtectedBase { };\n\n"
         "/*! Classe dérivée par héritage privé */\n"
         "class PrivateBase { };\n\n"
         "/*! Classe utilisée par la classe dérivée */\n"
         "class Used { };\n\n"
         "/*! Super-classe qui hérite de plusieurs autres classes */\n"
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
         "Cela aboutira au graphe suivant :"
         "<p><center><img alt=\"\" src=\"graph_legend." + Config::getEnum("dot-image-format").toUtf8() + "\"></center></p>\n"
         "<p>\n"
         "Les rectangles du graphe ci-dessus ont la signification suivante :\n"
         "<ul>\n"
         "<li>Un rectangle plein noir représente la structure ou la classe pour laquelle "
         "le graphe est généré.\n"
         "<li>Un rectangle avec un bord noir indique une classe ou une structure documentée.\n"
         "<li>Un rectangle avec un bord gris indique une classe ou une structure non documentée.\n"
         "<li>Un rectangle avec un bord rouge indique une structure ou une classe documentée\n"
         "pour laquelle des relations d'héritage ou de collaboration manquent. Un graphe est "
         "tronqué s'il n'entre pas dans les limites spécifiées."
         "</ul>\n"
         "Les flèches ont la signification suivante :\n"
         "<ul>\n"
         "<li>Une flèche bleu foncé est utilisée pour visualiser une relation d'héritage publique "
         "entre deux classes.\n"
         "<li>Une flèche vert foncé est utilisée pour une relation d'héritage protégée.\n"
         "<li>Une flèche rouge foncé est utilisée pour une relation d'héritage privée.\n"
         "<li>Une flèche violette en pointillés est utilisée si une classe est contenue ou "
         "utilisée par une autre classe. La flèche est étiquetée avec la ou les variable(s) "
         "qui permettent d'accéder à la classe ou structure pointée. \n"
         "<li>Une flèche jaune en pointillés indique une relation entre un modèle d'instance et "
         "le modèle de classe duquel il est instancié. La flèche est étiquetée avec "
         "les paramètres de modèle de l'instance.\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "légende";
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
      return "Liste des tests";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Propriétés";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Documentation des propriétés";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config::getBool("optimize-c")) {
         return "Structures de données";
      } else {
         return "Classes";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Paquetage " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Liste des paquetages";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Liste des paquetages avec une brève description (si disponible) :";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Paquetages";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Valeur :";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Bogue";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Liste des bogues";
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
      return "Index";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Classe" : "classe"));
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
      QByteArray result((first_capital ? "Fichier" : "fichier"));
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
      QByteArray result((first_capital ? "Espace" : "espace"));
      if (!singular) {
         result += "s";
      }
      result += " de nommage";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Groupe" : "groupe"));
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
      QByteArray result((first_capital ? "Page" : "page"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Membre" : "membre"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Globa" : "globa"));
      if (!singular) {
         result += "ux(ales)";
      } else {
         result += "l(e)";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Auteur" : "auteur"));
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
      return "Références";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Implémente " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Implémenté dans " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Table des matières";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Liste des éléments obsolètes";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Événements";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Documentation des événements";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Types de paquetage";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Fonctions de paquetage";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Fonctions statiques de paquetage";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Attributs de paquetage";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Attributs statiques de paquetage";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Tout";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Voici le graphe d'appel pour cette fonction :";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Résultats de la recherche";
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
         return "Désolé, aucun document ne correspond à votre requête.";
      } else if (numDocuments == 1) {
         return "Trouvé <b>1</b> document correspondant à votre requête.";
      } else {
         return "Trouvé  <b>$num</b> documents correspondant à votre requête. "
                "Classé par ordre de pertinence décroissant.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Correspondances :";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return " Fichier source de " + filename;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Hiérarchie de répertoires";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Documentation des répertoires";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Répertoires";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Cette hiérarchie de répertoire est triée approximativement, "
             "mais pas complètement, par ordre alphabétique :";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray  result = "Répertoire de référence de ";
      result += dirName;
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Répertoire" : "répertoire"));
      if (singular) {
         result += "";
      } else {
         result += "s";
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
      return "Ceci est une fonction membre surchargée, "
             "proposée par commodité. Elle diffère de la fonction "
             "ci-dessus uniquement par le(s) argument(s) qu'elle accepte.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Voici le graphe des appelants de cette fonction :";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Documentation des énumérations";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Documentation des fonctions/subroutines membres";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Liste des types de données";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Champs de données";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Liste des types de données avec une brève description :";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Liste de tous les membres de types de données ";
      if (!extractAll) {
         result += "documentés ";
      }
      result += "avec des liens vers ";
      if (!extractAll) {
         result += "la documentation de la structure des données de chaque membre :";
      } else {
         result += "les types des données auxquels ils appartiennent :";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Index du type de données";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Documentation du type de données";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Fonctions/Subroutines";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Documentation de la fonction/subroutine";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Les types de données";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Liste des modules";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Liste de tous les modules ";
      if (!extractAll) {
         result += "documentés ";
      }
      result += "avec une brève description :";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = "Réference ";
      if (isTemplate) {
         result += "du modèle ";
      }
      switch (compType) {
         case ClassDef::Class:
            result += "du module ";
            break;
         case ClassDef::Struct:
            result += "du type ";
            break;
         case ClassDef::Union:
            result += "de l'union ";
            break;
         case ClassDef::Interface:
            result += "de l'interface ";
            break;
         case ClassDef::Protocol:
            result += "du protocole ";
            break;
         case ClassDef::Category:
            result += "de la catégorie ";
            break;
         case ClassDef::Exception:
            result += "de l'exception ";
            break;
         default:
            break;
      }
      result += (QByteArray)clName;
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = "Référence du module ";
      result += namespaceName;
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Membres du module";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Liste de tous les membres ";
      if (!extractAll) {
         result += "documentés ";
      }
      result += "du module avec des liens vers ";
      if (extractAll) {
         result += "la documentation du module de chaque membre :";
      } else {
         result += "les modules auxquels ils appartiennent :";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Index des modules";
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
      QByteArray result = (QByteArray)"La documentation de ";
      switch (compType) {
         case ClassDef::Class:
            result += "ce module";
            break;
         case ClassDef::Struct:
            result += "ce type";
            break;
         case ClassDef::Union:
            result += "cette union";
            break;
         case ClassDef::Interface:
            result += "cette interface";
            break;
         case ClassDef::Protocol:
            result += "ce protocole";
            break;
         case ClassDef::Category:
            result += "cette catégorie";
            break;
         case ClassDef::Exception:
            result += "cette exception";
            break;
         default:
            break;
      }
      result += " a été générée à partir ";
      if (single) {
         result += "du fichier suivant :";
      } else {
         result += "des fichiers suivants :";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Type" : "type"));
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
      QByteArray result((first_capital ? "Sous-programme" : "sous-programme"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Contraintes de type";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return "Relation " + QByteArray(name);
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Chargement...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Espace de nommage global";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Recherche...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Aucune correspondance";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Fichier dans " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Inclut le fichier dans " + name;
   }
 
   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Références bibliographiques";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Graphe des dépendances de répertoires pour ") + name + ":";
   }
 
   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "Niveau de détails";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Paramètres du template";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "et " + number + " de plus...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "La documentation pour cette énumération a été générée à partir ";
      if (!single) {
         result += "du fichier suivant";
      } else {
         result += "des fichiers suivants";
      }
      result += " :";
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return QByteArray(name) + " Référence de l'énumération";
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " hérités de " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Membres hérités additionnels";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      QByteArray opt = enable ? "activer" : "désactiver";
      return "cliquez pour " + opt + " la synchronisation du panel";
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "Déclarée dans la catégorie @1.";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "Dérive la classe @1.";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "Méthodes de classe";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "Méthodes d'instance";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "Documentation des méthodes";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      return "Vue d'ensemble";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.4
   //////////////////////////////////////////////////////////////////////////

   /** old style UNO IDL services: implemented interfaces */
   virtual QByteArray trInterfaces() {
      return "Interfaces exportées";
   }

   /** old style UNO IDL services: inherited services */
   virtual QByteArray trServices() {
      return "Services inclus";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroups() {
      return "Groupes constants";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroupReference(const char *namespaceName) {
      QByteArray result = "Référence du groupe constant ";
      result += namespaceName;
      return result;
   }
   /** UNO IDL service page title */
   virtual QByteArray trServiceReference(const char *sName) {
      QByteArray result = "Référence du service ";
      result += (QByteArray)sName;
      return result;
   }
   /** UNO IDL singleton page title */
   virtual QByteArray trSingletonReference(const char *sName) {
      QByteArray result = "Référence du singleton ";
      result += (QByteArray)sName;
      return result;
   }
   /** UNO IDL service page */
   virtual QByteArray trServiceGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"La documentation pour ce service "
                          "a été générée par ";
      if (single) {
         result += "le fichier suivant :";
      } else {
         result += "les fichiers suivants :";
      }
      return result;
   }
   /** UNO IDL singleton page */
   virtual QByteArray trSingletonGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"La documentation pour ce singleton "
                          "a été générée par ";
      if (single) {
         result += "le fichier suivant :";
      } else {
         result += "les fichiers suivants :";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////

};

#endif