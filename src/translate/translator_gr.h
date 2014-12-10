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

#ifndef TRANSLATOR_GR_H
#define TRANSLATOR_GR_H

class TranslatorGreek : public TranslatorAdapter_1_8_4
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
      return "greek";
   }

   virtual QByteArray latexLanguageSupportCommand() {
      //return "\\usepackage[greek,english]{babel}\n\\usepackage[iso-8859-7]{inputenc}\n";
      return "\\usepackage[greek,english]{babel}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Σχετικές συναρτήσεις";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Σημειώστε ότι αυτές δεν είναι συναρτήσεις μέλη.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Λεπτομερής Περιγραφή";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Τεκμηρίωση Μελών Typedef";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Τεκμηρίωση Απαριθμήσεων Μελών";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Τεκμηρίωση Συναρτήσεων Μελών";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Τεκμηρίωση Πεδίων";
      } else {
         return "Τεκμηρίωση Δεδομένων Μελών";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Περισσότερα...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Λίστα όλων των μελών";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Λίστα μελών";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Ακολουθεί η πλήρης λίστα των μελών της";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", περιλαμβανομένων όλων των κληρονομημένων μελών.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Δημιουργήθηκε αυτόματα από το Doxygen";
      if (s) {
         result += (QByteArray)" για " + s;
      }
      result += " από τον πηγαίο κώδικα.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "όνομα απαρύθμισης";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "τιμή απαρίθμησης";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "ορισμένο στο ";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Κομμάτια";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Ιεραρχία Κλάσεων";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Δομές Δεδομένων";
      } else {
         return "Λίστα Κλάσεων";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Λίστα Αρχείων";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Μέλη Δομών Δεδομένων";
      } else {
         return "Μέλη Κλάσεων";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Καθολικοί ορισμοί";
      } else {
         return "Μέλη Αρχείων";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Σχετικές Σελίδες";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Παραδείγματα";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Αναζήτηση";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Αυτή η λίστα κληρονομικότητας είναι μερικώς ταξινομημένη αλφαβητικά:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Ακολουθεί μια λίστα όλων των ";
      if (!extractAll) {
         result += "τεκμηριωμένων ";
      }
      result += "αρχείων με σύντομες περιγραφές:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Ακολουθούνε οι δομές δεδομένων με σύντομες περιγραφές:";
      } else {
         return "Ακολουθούνε οι κλάσεις, δομές, "
                "ενώσεις και οι διασυνδέσεις με σύντομες περιγραφές:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Ακολουθεί η λίστα όλων των ";
      if (!extractAll) {
         result += "τεκμηριωμένων ";
      }
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "μελών δομών δεδομένων και ενώσεων ";
      } else {
         result += "κλάσεων";
      }
      result += " με συνδέσμους ";
      if (!extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "στην τεκμηρίωση των δομών/ενώσεων για κάθε πεδίο:";
         } else {
            result += "στην τεκμηρίωση των κλάσεων για κάθε πεδίο:";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "στις δομές/ενώσεις που ανήκουν:";
         } else {
            result += "στις κλάσεις που ανήκουν:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Ακολουθεί η λίστα όλων των ";
      if (!extractAll) {
         result += "τεκμηριωμένων ";
      }

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "συναρτήσεων, μεταβλητών, ορισμών, απαριθμήσεων, και ορισμών τύπων";
      } else {
         result += "μελών αρχείων";
      }
      result += " με συνδέσμους ";
      if (extractAll) {
         result += "στα αρχεία που ανήκουν:";
      } else {
         result += "στην τεκμηρίωση:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Ακολουθεί η λίστα με τα παραδείγματα:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Ακολουθεί η λίστα όλων των σχετικών σελίδων τεκμηρίωσης:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Ακολουθεί η λίστα όλων των μονάδων:";
   }

   // index titles (the project name is prepended for these)


   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Τεκμηρίωση";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Ευρετήριο μονάδων";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Ιεραρχικό Ευρετήριο";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Ευρετήριο δομών δεδομένων";
      } else {
         return "Συμπαγές Ευρετήριο";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Ευρετήτιο Αρχείων";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Τεκμηρίωση Μονάδων";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Τεκμηρίωση Δομών Δεδομένων";
      } else {
         return "Τεκμηρίωση Κλάσεων";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Τεκμηρίωση Αρχείων";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Τεκμηρίωση Παραδειγμάτων";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Τεκμηρίωση Σελίδων";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Εγχειρίδιο Αναφοράς";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Ορισμοί";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Ορισμοί Τύπων";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Απαριθμήσεις";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Συναρτήσεις";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Μεταβλητές";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Τιμές Απαριθμήσεων";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Τεκμηρίωση Ορισμών";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Τεκμηρίωση Ορισμών Τύπων";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Τεκμηρίωση Απαριθμήσεων";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Τεκμηρίωση Συναρτήσεων";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Τεκμηρίωση Μεταβλητών";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Δομές Δεδομένων";
      } else {
         return "Κλάσεις";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Δημιουργήθηκε στις " + date;
      if (projName) {
         result += (QByteArray)" για " + projName;
      }
      result += (QByteArray)" από";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Διάγραμμα κληρονομικότητας για την " + clName + ":";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Μόνο για εσωτερική χρήση.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Προειδοποίηση";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Έκδοση";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Ημ/νια";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Επιστρέφει";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Κοιτάξτε επίσης ";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Παράμετροι";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Εξαίρεση";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Δημιουργήθηκε από ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Λίστα Χώρων Ονομάτων";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Ακολουθέι η λίστα όλων των ";
      if (!extractAll) {
         result += "τεκμηριωμένων ";
      }
      result += "Χώρων Ονομάτων με σύντομες περιγραφές:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Φίλοι";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Τεκμηρίωση Φιλικών και Συσχετιζόμενων Συναρτήσεων";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result = (QByteArray)"Αναφορά";
      if (isTemplate) {
         result += " Προτύπου";
      }
      switch (compType) {
         case ClassDef::Class:
            result += " Κλάσης ";
            break;
         case ClassDef::Struct:
            result += " Δομής ";
            break;
         case ClassDef::Union:
            result += " Ένωσης ";
            break;
         case ClassDef::Interface:
            result += " Διασύνδεσης ";
            break;
         case ClassDef::Protocol:
            result += " Πρωτοκόλλου ";
            break;
         case ClassDef::Category:
            result += " Κατηγορίας ";
            break;
         case ClassDef::Exception:
            result += " Εξαίρεσης ";
            break;
         default:
            break;
      }
      result += clName;
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = "Αναφορά Αρχείου ";
      result += fileName;
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = "Αναφορά Χώρου Ονομάτων ";
      result += namespaceName;
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Δημόσιες Μέθοδοι";
   }
   virtual QByteArray trPublicSlots() {
      return "Δημόσια Slots";
   }
   virtual QByteArray trSignals() {
      return "Σήματα";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Στατικές Δημόσιες Μέδοδοι";
   }
   virtual QByteArray trProtectedMembers() {
      return "Προστατευμένες Μέδοδοι";
   }
   virtual QByteArray trProtectedSlots() {
      return "Προστατευμένα Slots";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Στατικές Προστατευμένες Μέδοδοι";
   }
   virtual QByteArray trPrivateMembers() {
      return "Ιδιωτικές Μέθοδοι";
   }
   virtual QByteArray trPrivateSlots() {
      return "Ιδιοτικά Slots";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Στατικές Ιδιωτικές Μέθοδοι";
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
               result += ", και ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Κληρονομεί " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Κληρονομείται από " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Επαναϋλοποιείται από " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Επαναϋλοποιείται στην " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Μέλη Χώρου Ονομάτων";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Ακολουθεί η λίστα όλων των ";
      if (!extractAll) {
         result += "τεκμηριωμένων ";
      }
      result += "μελών χώρων ονομάτων με συνδέσμους ";
      if (extractAll) {
         result += "στην τεκμηρίωση του χώρου ονομάτων για κάθε μέλος:";
      } else {
         result += "στους χώρους ονομάτων που ανήκουν:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Ευρετήριο Χώρων Ονομάτων";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Τεκμηρίωση Χώρων Ονομάτων";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Χώροι Ονομάτων";
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
      QByteArray result = (QByteArray)"Η τεκμηρίωση για ";
      switch (compType) {
         case ClassDef::Class:
            result += "αυτή την κλάση";
            break;
         case ClassDef::Struct:
            result += "αυτή τη δομή";
            break;
         case ClassDef::Union:
            result += "αυτή την ένωση";
            break;
         case ClassDef::Interface:
            result += "αυτή τη διασύνδεση";
            break;
         case ClassDef::Protocol:
            result += "αυτό το πρωτόκολλο";
            break;
         case ClassDef::Category:
            result += "αυτή την κατηγορία";
            break;
         case ClassDef::Exception:
            result += "αυτή την εξαίρεση";
            break;
         default:
            break;
      }
      result += " δημιουργήθηκε απο τ";
      if (single) {
         result += "ο ακόλουθο αρχείο:";
      } else {
         result += "α ακόλουθα αρχεία:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Τιμές Επιστροφής";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Κύρια Σελίδα";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "σελ.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Ορισμός στη γραμμή @0 του αρχείου @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Ορισμός στο αρχείο @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Αποσυρμένο";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Δίαγραμμα Συνεργασίας για την κλάση " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Διάγραμμα εξάρτησης αρχείου συμπερίληψης για το " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Τεκμηρίωση Constructor & Destructor";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Πήγαινε στον πηγαίο κώδικα του αρχείου.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Πήγαινε στην τεκμηρίωση του αρχείου.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Προϋποθέσεις";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Μεταϋποθέσεις";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Αμετάβλητα";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Αρχική τιμή:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "κώδικας";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Γραφική Αναπαράσταση Της Ιεραρχίας Των Κλάσεων";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Πήγαινε στην γραφική αναπαράσταση της ιεραρχίας των κλάσεων";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Πήγαινε στην με κείμενο αναπαράσταση της ιεραρχίας των κλάσεων";
   }
   virtual QByteArray trPageIndex() {
      return "Ευρετήριο Σελίδων";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Σημείωση";
   }
   virtual QByteArray trPublicTypes() {
      return "Δημόσιοι Τυποι";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Πεδία Δεδομένων";
      } else {
         return "Δημόσια Χαρακτηριστικά";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Στατικά Δημόσια Χαρακτηριστικά";
   }
   virtual QByteArray trProtectedTypes() {
      return "Προστατευμένοι Τύποι";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Προστατευμένα Χαρακτηριστικά";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Στατικά Προστατευμένα Χαρακτηριστικά";
   }
   virtual QByteArray trPrivateTypes() {
      return "Ιδιωτικοί Τύποι";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Ιδιωτικα Χαρακτηριστικά";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Στατικά Ιδιωτικα Χαρακτηριστικά";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "Προς Υλοποίηση";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Λίστα των Προς Υλοποίηση";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Αναφορά από";
   }
   virtual QByteArray trRemarks() {
      return "Παρατήρηση";
   }
   virtual QByteArray trAttention() {
      return "Προσοχή";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Το διάγραμμα αυτό παρουσιάζει ποιά αρχεία άμεσα ή "
             "έμμεσα περιλαμβάνουν αυτό το αρχείο:";
   }
   virtual QByteArray trSince() {
      return "Από";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Υπόμνημα Διαγραμμάτων";
   }
   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Αυτή η σελίδα εξηγεί το πως ερμηνεύονται τα διαγράμματα που δημιουργούνται "
         "από το doxygen.<p>\n"
         "Θεωρείστε το παρακάτω παράδειγμα:"
         "\\code\n"
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
         "Αυτό οδηγεί στο επόμενο διάγραμμα:"
         "<p><center><img alt=\"\" src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center></p>\n"
         "<p>\n"
         "Τα κουτιά στο παραπάνω διάγραμμα έχουν την ακόλουθη σημασία:\n"
         "</p>\n"
         "<ul>\n"
         "<li>Ένα γεμισμένο μαύρο κουτί αναπαριστά τη δομή ή την κλάση για την οποία"
         "αφορά το διάγραμμα.\n"
         "<li>Ένα κουτί με μαύρο περίγραμμα αναπαριστά μία τεκμηριωμένη δομή ή κλάση.\n"
         "<li>Ένα κουτί με γκρίζο περίγραμμα αναπαριστά μία μη τεκμηριωμένη δομή ή κλάση.\n"
         "<li>Ένα κουτί με κόκκινο περίγραμμα αναπαριστά μία τεκμηριωμένη δομή ή κλάση για "
         "την οποία δεν παρουσιάζονται όλες οι σχέσεις κληρονομικότητας και περιεχομένου. %Ένα "
         "διάγραμμα μειώνεται αν δεν χωράει στις ορισμένες διαστάσεις."
         "</ul>\n"
         "<p>\n"
         "Τα βέλη έχουν τις ακόλουθες σημασίες:\n"
         "</p>\n"
         "<ul>\n"
         "<li>Ένα βαθύ μπλε βέλος χρησιμοποιείται για να παρουσιάσει μία δημόσια σχέση κληρονομικότητας "
         "μεταξύ δύο κλάσεων.</li>\n"
         "<li>Ένα βαθύ πράσινο βέλος χρησιμοποιείται για προστατευμένη κληρονομικότητα.</li>\n"
         "<li>Ένα βαθύ κόκκινο βέλος χρησιμοποιείται για ιδιωτική κληρονομικότητα.</li>\n"
         "<li>Ένα μωβ διακεκομένο βέλος χρησιμοποιείται αν μία κλάση περιέχεται ή χρησιμοποιείται "
         "από μία άλλη κλάση. Το βέλος ονομάζεται από το όνομα της μεταβλητής(ων) "
         "μέσω της οποίας η κλάση ή δομή είναι προσβάσιμη.</li>\n"
         "<li>Ένα κίτρινο διακεκομμένο βέλος χρησιμοποιείται για μια σχέση μεταξύ ενός template αντικειμένου και "
         "της template κλάσης από την οποία δημιουργήθηκε. Το βέλος ονομάζεται με τις παραμέτρους του template "
         "του αντικειμένου.</li>\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "υπόμνημα";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "Δοκιμαστικό";
   }
   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "Λίστα Δοκιμαστικών";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Ιδιότητες";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Τεκμηρίωση Ιδιοτήτων";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Δομές Δεδομένων";
      } else {
         return "Κλάσεις";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Πακέτο " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Λίστα Πακέτων";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Ακολουθεί η λίστα των πακέτων με σύντομη περιγραφή (αν υπάρχει):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Πακέτα";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Τιμή:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Πρόβλημα";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Λίστα Προβλημάτων";
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
      return "1253";
   }


   /*! Used as ansicpg for RTF fcharset
    */
   virtual QByteArray trRTFCharSet() {
      return "161";
   }

   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "Ευρετήριο";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Κλάση" : "κλάση"));
      if (!singular) {
         result += "";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Αρχεί" : "αρχεί"));
      if (!singular) {
         result += "α";
      } else {
         result += "ο";
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
      QByteArray result((first_capital ? "Ομάδ" : "ομάδ"));
      if (!singular) {
         result += "ες";
      } else {
         result += "α";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Σελίδ" : "σελίδ"));
      if (!singular) {
         result += "ες";
      } else {
         result += "α";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Μέλ" : "μέλ"));
      if (!singular) {
         result += "η";
      } else {
         result += "ος";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Καθολικ" : "καθολικ"));
      if (!singular) {
         result += "ές";
      } else {
         result += "ή";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Συγραφ" : "συγραφ"));
      if (!singular) {
         result += "";
      } else {
         result += "έας";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Αναφορές";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Υλοποιεί " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Υλοποιείται από " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Πίνακας Περιεχομένων";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Λίστα Καταργημένων";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Συμβάντα";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Τεκμηρίωση Συμβάντων";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Τύποι Πακέτων";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Συναρτήσεις Πακέτου";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Στατικές Συναρτήσεις Πακέτου";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Μεταβλητές Πακέτου";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Στατικές Μεταβλητές Πακέτου";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Όλα";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Το διάγραμμα δείχνει ποιές συναρτήσεις καλούνται από αυτή:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Αποτελέσματα Αναζήτησης";
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
         return "Συγγνώμη, δεν υπάρχει κείμενο που να ταιριάζει με την αίτησή σας.";
      } else if (numDocuments == 1) {
         return "Βρέθηκε <b>1</b> κείμενο που ταιριάζει με την αίτησή σας.";
      } else {
         return "Βρέθηκαν <b>$num</b> κείμενα που ταιριάζουν με την αίτησή σας. "
                "Πρώτα εμφανίζονται τα κείμενα που ταιριάζουν πιο πολύ.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Ταίριαξαν:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return "Αρχείο κώδικα " + filename;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Ιεραρχία Καταλόγου";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Τεκμηρίωση Καταλόγου";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Κατάλογοι";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Η ιεραρχία καταλόγων ταξινομήθηκε αλφαβητικά, αλλά όχι πολύ αυστηρά:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = QByteArray("Αναφορά του Καταλόγου ") + dirName;
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Κατάλογο" : "κατάλογο"));
      if (singular) {
         result += "ς";
      } else {
         result += "ι";
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
      return "Αυτή είναι μια υπερφορτωμένη συνάρτηση-μέλος, "
             "που παρέχεται για ευκολία. Διαφέρει από την παραπάνω "
             "συνάρτηση μόνο στον τύπο των παραμέτρων που δέχεται.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Το διάγραμμα δείχνει από ποιές συναρτήσεις καλείται αυτή η συνάρτηση:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Enumerator Documentation";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Τεκμηρίωση Συνάρτησης/Υπορουτίνας Μέλους";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Λίστα Τύπων Δεδομένων";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Πεδία Δεδομένων";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Εδώ είναι οι τύποι δεδομένων με σύντομη περιγραφή:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Εδώ είναι η λίστα όλων των ";
      if (!extractAll) {
         result += "τεκμηριωμένων ";
      }
      result += "μελών τύπων δεδομένων";
      result += " με συνδέσεις ";
      if (!extractAll) {
         result += "στην τεκμηρίωση της δομής δεδομένων για κάθε μέλος";
      } else {
         result += "στους τύπους δεδομένων που ανήκουν:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Περιεχόμενα Τύπων Δεδομένων";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Τεκμηρίωση Τύπων Δεδομένων";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Συναρτήσεις/Υπορουτίνες";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Τεκμηρίωση Συνάρτησης/Υπορουτίνας";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Τύποι Δεδομένων";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Λίστα Υπομονάδων";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Εδώ είναι μια λίστα με όλες τις ";
      if (!extractAll) {
         result += "τεκμηριωμένες ";
      }
      result += "υπομονάδες με σύντομή περιγραφή:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      switch (compType) {
         case ClassDef::Class:
            result += " Υπομονάδα";
            break;
         case ClassDef::Struct:
            result += " Τύπος";
            break;
         case ClassDef::Union:
            result += " Ένωση";
            break;
         case ClassDef::Interface:
            result += " Interface";
            break;
         case ClassDef::Protocol:
            result += " Πρωτόκολλο";
            break;
         case ClassDef::Category:
            result += " Κατηγορία";
            break;
         case ClassDef::Exception:
            result += " Εξαίρεση";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += " Πρότυπο";
      }
      result += " Δήλωση";
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Δηλώσεις Υπομονάδων";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Υπομονάδες Μέλη";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Εδώ είναι μια λίστα με όλες τις ";
      if (!extractAll) {
         result += "τεκμηριωμένες ";
      }
      result += "μέλη υπομονάδες με συνδέσεις ";
      if (extractAll) {
         result += "στην τεκμηρίωση της υπομονάδας για κάθε μέλος:";
      } else {
         result += "στις υπομονάδες που ανήκουν:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Περιεχόμενα Υπομονάδων";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Υπομονάδ" : "υπομονάδ"));
      if (!singular) {
         result += "ες";
      } else {
         result += "α";
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
      QByteArray result = (QByteArray)"Η τεκμηρίωση για ";
      switch (compType) {
         case ClassDef::Class:
            result += "αυτή την υπομονάδα";
            break;
         case ClassDef::Struct:
            result += "αυτό τον τύπο δεδομένων";
            break;
         case ClassDef::Union:
            result += "αυτή την ένωση";
            break;
         case ClassDef::Interface:
            result += "αυτό το interface";
            break;
         case ClassDef::Protocol:
            result += "αυτό το πρωτόκολλο";
            break;
         case ClassDef::Category:
            result += "αυτή την κατηγορία";
            break;
         case ClassDef::Exception:
            result += "αυτή την εξαίρεση";
            break;
         default:
            break;
      }
      result += " δημιουργήθηκε από ";
      if (single) {
         result += "το παρακάτω αρχείο:";
      } else {
         result += "τα παρακάτω αρχεία:";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Τύπο" : "τύπο"));
      if (!singular) {
         result += "ι";
      } else {
         result += "ος";
      }
      result += first_capital ? " Δεδομένων" : "  δεδομένων";
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Υποπρ" : "υποπρ"));
      if (!singular) {
         result += "ογράμματα";
      } else {
         result += "όγραμμα";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Περιορισμοί Τύπων Δεδομένων";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return "Σχέση του " + QByteArray(name);
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Φόρτωση...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Γενικός χώρος ονομάτων";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Αναζήτηση...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Δεν βρέθηκαν αποτελέσματα αναζήτησης";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Αρχείο σε " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Εσωκλείει το αρχείο στο " + name;
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
      static const char *days[]   = { "Δευ", "Τρι", "Τετ", "Πεμ", "Παρ", "Σαβ", "Κυρ" };
      static const char *months[] = { "Ιαν", "Φεβ", "Μαρ", "Απρ", "Μαι", "Ιουν", "Ιουλ", "Αυγ", "Σεπ", "Οκτ", "Νοε", "Δεκ" };
      QByteArray sdate;
      sdate.sprintf("%s %.2d %s %d", days[dayOfWeek - 1], day, months[month - 1], year);
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
      return "Αναφορές Βιβλιογραφίας";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Διάγραμμα εξάρτησης φακέλων για ") + name + ":";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.0
   //////////////////////////////////////////////////////////////////////////

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "Επίπεδο λεπτομέρειας";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Παράμετροι Προτύπου";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "και " + number + " ακόμη...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "Η τεκμηρίωση αυτής της απαρίθμησης δημιουργήθηκε από ";
      if (single) {
         result += "τo αρχείo";
      } else {
         result += "τα αρχεία";
      }
      result += ":";
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return QByteArray("Αναφορά Απαρίθμησης ") + QByteArray(name);
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " κληρονόμησαν από " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Επιπρόσθετες Κληρονομημένες Μέθοδοι";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      QByteArray opt = enable ? "ενεργοποιήσετε" : "απενεργοποιήσετε";
      return "Κάντε κλικ για να " + opt + " το συγχρονισμό του παραθύρου";
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "Παρέχεται από την κατηγορία @1.";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "Κληρονομει την κλάση @1.";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "Μέθοδοι Κλάσης";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "Μέθοδοι Αντικειμένου";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "Τεκμηρίωση Μεθόδου";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      return "Επισκόπηση σχεδίασης";
   }


   /* Future strategy of changes
   	Many words have more clear meaning in English and not in Greek:
   	template -> πρότυπο
   	instance -> αντικείμενο? (μπορεί να μιλάμε για template instantiation ή class instantiation)
   	interface -> διασύνδεση
   */

   //////////////////////////////////////////////////////////////////////////

};

#endif
