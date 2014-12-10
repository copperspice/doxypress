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

#ifndef TRANSLATOR_HR_H
#define TRANSLATOR_HR_H

class TranslatorCroatian : public TranslatorAdapter_1_8_2
{
 private:

 public:
   QByteArray idLanguage() {
      return "croatian";
   }
   QByteArray latexLanguageSupportCommand() {
      return "\\usepackage[croatian]{babel}\n";
   }
   QByteArray trRelatedFunctions() {
      return "Povezane funkcije";
   }
   QByteArray trRelatedSubscript() {
      return "(To nisu member funkcije.)";
   }
   QByteArray trDetailedDescription() {
      return "Detaljno objašnjenje";
   }
   QByteArray trMemberTypedefDocumentation() {
      return "Dokumentacija typedef članova";
   }
   QByteArray trMemberEnumerationDocumentation() {
      return "Dokumentacija enumeracijskih članova";
   }
   QByteArray trMemberFunctionDocumentation() {
      return "Dokumentacija funkcija";
   }
   QByteArray trMemberDataDocumentation() {
      return "Documentacija varijabli";
   }
   QByteArray trMore() {
      return "Opširnije...";
   }
   QByteArray trListOfAllMembers() {
      return "Popis svih članova";
   }
   QByteArray trMemberList() {
      return "Popis članova.";
   }
   QByteArray trThisIsTheListOfAllMembers() {
      return "Ovo je popis svih članova";
   }
   QByteArray trIncludingInheritedMembers() {
      return ", uključujući naslijeđene članove.";
   }
   QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "napravljeno automatski Doxygen-om";
      if (s) {
         result += (QByteArray)" za " + s;
      }
      result += " iz programskog koda.";
      return result;
   }
   QByteArray trEnumName() {
      return "enum ime";
   }
   QByteArray trEnumValue() {
      return "enum vrijednost";
   }
   QByteArray trDefinedIn() {
      return "definirano u";
   }
   QByteArray trModules() {
      return "Moduli";
   }
   QByteArray trClassHierarchy() {
      return "Stablo klasa";
   }
   QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Sve strukture";
      } else {
         return "Sve klase";
      }
   }
   QByteArray trFileList() {
      return "Popis datoteka";
   }
   QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Svi članovi struktura";
      } else {
         return "Svi članovi klasa";
      }
   }
   QByteArray trFileMembers() {
      return "članovi klasa u datoteci";
   }
   QByteArray trRelatedPages() {
      return "Stranice povezane s ovom";
   }
   QByteArray trExamples() {
      return "Primjeri";
   }
   QByteArray trSearch() {
      return "Traži";
   }
   QByteArray trClassHierarchyDescription() {
      return "Stablo naslijeđivanja je složeno "
             "približno po abecedi:";
   }
   QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Popis svih ";
      if (!extractAll) {
         result += "dokumentiranih ";
      }
      result += "datoteka, s kratkim opisom:";
      return result;
   }
   QByteArray trCompoundListDescription() {
      return "Popis svih klasa, unija i struktura "
             "s kratkim opisom :";
   }
   QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Popis svih ";
      if (!extractAll) {
         result += "dokumentiranih ";
      }

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "članova klasa s linkovima na ";
      } else {
         result += "članova struktura s linkovima na ";
      }

      if (!extractAll) {
         result += "dokumentaciju svakog člana:";
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "dokumentaciju klase :";
         } else {
            result += "dokumentaciju strukture";
         }
      }
      return result;
   }
   QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Popis svih ";
      if (!extractAll) {
         result += "dokumentiranih ";
      }
      result += "članova s linkovima na ";
      if (extractAll) {
         result += "dokumentaciju datoteke u kojima se nalaze:";
      } else {
         result += "datoteke u kojima se nalaze:";
      }
      return result;
   }
   QByteArray trExamplesDescription() {
      return "Popis primjera :";
   }
   QByteArray trRelatedPagesDescription() {
      return "Popis povezanih stranica:";
   }
   QByteArray trModulesDescription() {
      return "Popis svih modula:";
   }

   QByteArray trDocumentation() {
      return "Dokumentacija";
   }
   QByteArray trModuleIndex() {
      return "Kazalo modula";
   }
   QByteArray trHierarchicalIndex() {
      return "Hijerarhijsko kazalo";
   }
   QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Kazalo struktura podataka";
      } else {
         return "Skupno kazalo ";
      }
   }
   QByteArray trFileIndex() {
      return "Kazalo datoteka";
   }
   QByteArray trModuleDocumentation() {
      return "Dokumentacija modula";
   }
   QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Dokumentacija struktura podataka";
      } else {
         return "Dokumentacija klasa";
      }
   }
   QByteArray trFileDocumentation() {
      return "Dokumentacija datoteka";
   }
   QByteArray trExampleDocumentation() {
      return "Dokumentacija primjera";
   }
   QByteArray trPageDocumentation() {
      return "Dokumentacija vezane stranice";
   }
   QByteArray trReferenceManual() {
      return "Priručnik";
   }

   QByteArray trDefines() {
      return "Definicije";
   }
   QByteArray trTypedefs() {
      return "Typedef-ovi";
   }
   QByteArray trEnumerations() {
      return "Enumeracije";
   }
   QByteArray trFunctions() {
      return "Funkcije";
   }
   QByteArray trVariables() {
      return "Varijable";
   }
   QByteArray trEnumerationValues() {
      return "Vrijednosti enumeracija";
   }
   QByteArray trDefineDocumentation() {
      return "Dokumentacija definicija";
   }
   QByteArray trTypedefDocumentation() {
      return "Dokumentacija typedef-a";
   }
   QByteArray trEnumerationTypeDocumentation() {
      return "Dokumentacija enumeracijskog tipa";
   }
   QByteArray trFunctionDocumentation() {
      return "Dokumentacije funkcija";
   }
   QByteArray trVariableDocumentation() {
      return "Dokumentacija varijable";
   }
   QByteArray trCompounds() {
      return "Strukture";
   }
   QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Napravljeno " + date;
      if (projName) {
         result += (QByteArray)" projekt: " + projName;
      }
      result += " generator: ";
      return result;
   }
   QByteArray trClassDiagram(const char *clName) {
      return QByteArray("Dijagram klasa za ") + clName;
   }
   QByteArray trForInternalUseOnly() {
      return "Isključivo za internu uporabu.";
   }
   QByteArray trWarning() {
      return "Upozorenje";
   }
   QByteArray trVersion() {
      return "Verzija";
   }
   QByteArray trDate() {
      return "Datum";
   }
   QByteArray trReturns() {
      return "Povratne vrijednosti";
   }
   QByteArray trSeeAlso() {
      return "Vidi također";
   }
   QByteArray trParameters() {
      return "Parametri";
   }
   QByteArray trExceptions() {
      return "Iznimke";
   }
   QByteArray trGeneratedBy() {
      return "Generirao";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   QByteArray trNamespaceList() {
      return "Popis imenika";
   }
   QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Popis svih ";
      if (!extractAll) {
         result += "dokumentiranih ";
      }
      result += "imenika s kratkim opisom:";
      return result;
   }
   QByteArray trFriends() {
      return "Friend-ovi ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   QByteArray trRelatedFunctionDocumentation() {
      return "Dokumentacija povezanih funkcija";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   QByteArray trCompoundReference(const char *clName,
                                  ClassDef::CompoundType compType,
                                  bool /*isTemplate*/)
   // used as the title of the HTML page of a class/struct/union
   {
      QByteArray result = "Opis ";
      switch (compType) {
         case ClassDef::Class:
            result += " klase ";
            break;
         case ClassDef::Struct:
            result += " strukture ";
            break;
         case ClassDef::Union:
            result += " unije ";
            break;
         case ClassDef::Interface:
            result += " sučelja (interface) ";
            break;
         case ClassDef::Protocol:
            result += " protokola ";
            break;
         case ClassDef::Category:
            result += " kategorije ";
            break;
         case ClassDef::Exception:
            result += " iznimke (exception) ";
            break;
         default:
            break;
      }
      result += clName;
      return result;
   }
   QByteArray trFileReference(const char *fileName)
   // used as the title of the HTML page of a file
   {
      QByteArray result = "Opis datoteke ";
      result += fileName;
      return result;
   }
   QByteArray trNamespaceReference(const char *namespaceName)
   // used as the title of the HTML page of a namespace
   {
      QByteArray result = "Opis imenika ";
      result += namespaceName;

      return result;
   }

   // these are for the member sections of a class, struct or union
   QByteArray trPublicMembers() {
      return "Public članovi";
   }
   QByteArray trPublicSlots() {
      return "Public slotovi";
   }
   QByteArray trSignals() {
      return "Signali";
   }
   QByteArray trStaticPublicMembers() {
      return "Static public članovi";
   }
   QByteArray trProtectedMembers() {
      return "Protected članovi";
   }
   QByteArray trProtectedSlots() {
      return "Protected slotovi";
   }
   QByteArray trStaticProtectedMembers() {
      return "Static protected članovi";
   }
   QByteArray trPrivateMembers() {
      return "Privatni članovi";
   }
   QByteArray trPrivateSlots() {
      return "Privatni slotovi";
   }
   QByteArray trStaticPrivateMembers() {
      return "Statični privatni članovi";
   }
   // end of member sections

   QByteArray trWriteList(int numEntries) {
      // this function is used to produce a comma-separated list of items.
      // use generateMarker(i) to indicate where item i should be put.
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
               result += "";
            }
         }
      }
      return result;
   }

   QByteArray trInheritsList(int numEntries)
   // used in class documentation to produce a list of base classes,
   // if class diagrams are disabled.
   {
      return "Naslijeđuje od " + trWriteList(numEntries) + ".";
   }
   QByteArray trInheritedByList(int numEntries)
   // used in class documentation to produce a list of super classes,
   // if class diagrams are disabled.
   {
      return "Naslijeđena u " + trWriteList(numEntries) + ".";
   }
   QByteArray trReimplementedFromList(int numEntries)
   // used in member documentation blocks to produce a list of
   // members that are hidden by this one.
   {
      return "Reimplementirano od  " + trWriteList(numEntries) + ".";
   }
   QByteArray trReimplementedInList(int numEntries) {
      // used in member documentation blocks to produce a list of
      // all member that overwrite the implementation of this member.
      return "Reimplementacija u " + trWriteList(numEntries) + ".";
   }

   QByteArray trNamespaceMembers()
   // This is put above each page as a link to all members of namespaces.
   {
      return "članovi imenika";
   }
   QByteArray trNamespaceMemberDescription(bool extractAll)
   // This is an introduction to the page with all namespace members
   {
      QByteArray result = "Lista svih ";
      if (!extractAll) {
         result += "dokumentiranih ";
      }
      result += "članova imenika s linkovima na ";
      if (extractAll) {
         result += "imeničku dokumentaciju svakog člana:";
      } else {
         result += "imenike kojima pripadaju:";
      }
      return result;
   }
   QByteArray trNamespaceIndex()
   // This is used in LaTeX as the title of the chapter with the
   // index of all namespaces.
   {
      return "Kazalo imenika";
   }
   QByteArray trNamespaceDocumentation()
   // This is used in LaTeX as the title of the chapter containing
   // the documentation of all namespaces.
   {
      return "Dokumentacija namespace-a";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   QByteArray trNamespaces() {
      return "Imenici";
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
      QByteArray result = "Dokumentacija ";
      switch (compType) {
         case ClassDef::Class:
            result += "klase";
            break;
         case ClassDef::Struct:
            result += "strukture";
            break;
         case ClassDef::Union:
            result += "unije";
            break;
         case ClassDef::Interface:
            result += "sučelja (interface)";
            break;
         case ClassDef::Protocol:
            result += "protokola";
            break;
         case ClassDef::Category:
            result += "kategorije";
            break;
         case ClassDef::Exception:
            result += "iznimke (exception)";
            break;
         default:
            break;
      }
      result += " je napravljena iz " + trFile(false, single) + ": ";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   QByteArray trReturnValues() {
      return "Povratna vrijednost";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   QByteArray trMainPage() {
      return "Glavna stranica";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   QByteArray trPageAbbreviation() {
      return "str.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991106
   //////////////////////////////////////////////////////////////////////////

   QByteArray trDefinedAtLineInSourceFile() {
      return "Definirano u liniji @0 datoteke @1.";
   }
   QByteArray trDefinedInSourceFile() {
      return "Definirano u datoteci @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   QByteArray trDeprecated() {
      return "Zastarjelo";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Kolaboracijski dijagram za " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Graf include međuovisnosti za " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   QByteArray trConstructorDocumentation() {
      return "Dokumentacija konstruktora i destruktora ";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   QByteArray trGotoSourceCode() {
      return "Izvorni kod";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   QByteArray trGotoDocumentation() {
      return "Dokumenacija za ovu datoteku.";
   }
   /*! Text for the \\pre command */
   QByteArray trPrecondition() {
      return "Preduvjeti";
   }
   /*! Text for the \\post command */
   QByteArray trPostcondition() {
      return "Postuvjeti";
   }
   /*! Text for the \\invariant command */
   QByteArray trInvariant() {
      return "Invarijanta";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   QByteArray trInitialValue() {
      return "Početna vrijednost:";
   }
   /*! Text used the source code in the file index */
   QByteArray trCode() {
      return "kod";
   }
   QByteArray trGraphicalHierarchy() {
      return "Grafičko stablo klasa";
   }
   QByteArray trGotoGraphicalHierarchy() {
      return "Grafičko stablo klasa";
   }
   QByteArray trGotoTextualHierarchy() {
      return "Tekstualno stablo klasa";
   }
   QByteArray trPageIndex() {
      return "Indeks stranice";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   QByteArray trNote() {
      return "Primjedba";
   }
   QByteArray trPublicTypes() {
      return "Public tipovi";
   }
   QByteArray trPublicAttribs() {
      return "Public atributi";
   }
   QByteArray trStaticPublicAttribs() {
      return "Static public atributi";
   }
   QByteArray trProtectedTypes() {
      return "Protected tipovi";
   }
   QByteArray trProtectedAttribs() {
      return "Protected atributi";
   }
   QByteArray trStaticProtectedAttribs() {
      return "Static protected atributi";
   }
   QByteArray trPrivateTypes() {
      return "Private tipovi";
   }
   QByteArray trPrivateAttribs() {
      return "Private atributi";
   }
   QByteArray trStaticPrivateAttribs() {
      return "Static private atributi";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a todo item */
   virtual QByteArray trTodo() {
      return "Za uraditi";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Ostalo za uraditi";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Referencirano od";
   }
   virtual QByteArray trRemarks() {
      return "Napomene";
   }
   virtual QByteArray trAttention() {
      return "Pažnja";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Ovaj graf pokazuje koje datoteke izravno "
             "ili neizravno uključuju ovu datoteku:";
   }
   virtual QByteArray trSince() {
      return "Od";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Legenda";
   }
   /*! page explaining how the dot graph's should be interpreted */
   virtual QByteArray trLegendDocs() {
      return
         "Ova stranica objašnjava kako interpretirati grafikone koje je generirao "
         "doxygen.<p>\n"
         "Na primjer:\n"
         "\\code\n"
         "/*! Nevidljiva klasa (neće stati na graf date visine) */\n"
         "class Invisible { };\n\n"
         "/*! Odrezana klasa, inheritance je skriven (klase se vidi na grafu, ali ne i sve bazne klase) */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Nedokumentirana klasa */\n"
         "class Undocumented { };\n\n"
         "/*! Klasa koja je naslijeđena public  */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! A template class */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Klasa koje je naslijeđena protected */\n"
         "class ProtectedBase { };\n\n"
         "/*! Klasa koje je naslijeđena private */\n"
         "class PrivateBase { };\n\n"
         "/*! Klasa koja se koristi agregacijom  */\n"
         "class Used { };\n\n"
         "/*! Super klasa koja naslijeđuje/koristi ostale */\n"
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
         "To će rezultirati grafikonom:"
         "<p><center><img src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
         "<p>\n"
         "Pravokutnici imaju slijedeće značenje:\n"
         "<ul>\n"
         "<li>Puni crni predstavlja klasu za koji je napravljen graf.\n"
         "<li>Pravokutnik s crnim rubom predstavlja dokumentiranu klasu.\n"
         "<li>Pravokutnik s sivim rubom predstavlja nedokumentiranu klasu.\n"
         "<li>Pravokutnik s crvenim rubom predstavlja dokumentiranu klasu\n"
         "Za koju nije prikazan graf naslijeđivanja. Graf je odrezan "
         "ako ne stane unutar određenih granica."
         "</ul>\n"
         "Strelice imaju slijedeće značenje:\n"
         "<ul>\n"
         "<li>Tamnoplava strelica označava public naslijeđivanje.\n"
         "<li>Tamnozelena strelica označava protected naslijeđivanje.\n"
         "<li>Tamnocrvena strelica označava private naslijeđivanje.\n"
         "<li>Ljubičasta isprekidana strelica se koristi ako je klasa dio "
         "druge klase ili ako se klasa koristi u drugoj klasi. Natpis na "
         "strelici je ime varijable u drugoj klasi\n"
         "Strelica je označena imenom varijable.\n"
         "<li>Žuta isprekidana strelica označava relaciju između template instance "
         "i template klase. Označena je imenom template parametra\n"
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
      return "Test lista";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Svojstva (property)";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Dokumentacija svojstava";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      return "Klase";
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Paket " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Lista paketa";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Paketi s kratkim opisom (ukoliko postoji):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Paketi";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Vrijednost:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Greška";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Popis grešaka";
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
      return "238";
   }
   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "Sadržaj";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Klas" : "klas"));
      result += (singular ? "a" : "e");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Datotek" : "datotek"));
      result += (singular ? "a" : "e");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result;
      if (singular) {
         result = ((first_capital ? "Imenik" : "imenik"));
      } else {
         result = ((first_capital ? "Imenici" : "imenici"));
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Grup" : "grup"));
      result += (singular ? "a" : "e");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Stranic" : "stranic"));
      result += (singular ? "a" : "e");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool, bool singular) {
      QByteArray result("član");
      if (!singular) {
         result += "ovi";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "G" : "g"));
      if ( singular ) {
         result += "lobalna varijabla";
      } else {
         result += "lobalne varijable";
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
      if (!singular) {
         result += "i";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Reference";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Implementira " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implementation this member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Implementirano u " + trWriteList(numEntries) + ".";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Sadržaj";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Popis zastarjelih metoda";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Događaji";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Dokumentacija događaja";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Tipovi u paketu";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Funkcije u paketu";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Statičke funkcije u paketu";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Atributi u paketu";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Statički atributi u paketu";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
   *  to link to the unfiltered list of all members.
   */
   virtual QByteArray trAll() {
      return "Sve";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Ovo je dijagram poziva za ovu funkciju:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
   *  results.
   */
   virtual QByteArray trSearchResultsTitle() {
      return "Rezultati pretrage";
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
         return "Nema dokumenta koji odgovaraju vašem upitu";
      } else if (numDocuments == 1) {
         return "Nađen <b>1</b> dokument koji odgovara vašem upitu.";
      } else if (numDocuments < 5) {
         // Croatian (AFAIK all Slavic languages except Macedonian and Bulgarian)
         // have different plural form for 2,3,4.
         return "Nađena <b>$num</b> dokumenta koji odgovaraju vašem upitu."
                "Najbolji su prikazani prvi.";
      } else {
         return "Nađeno <b>$num</b> dokumenata koji odgovaraju vašem upitu."
                "Najbolji su prikazani prvi.";
      }
   }
   /*! This string is put before the list of matched words, for each search
   *  result. What follows is the list of words that matched the query.
   */
   virtual QByteArray trSearchMatches() {
      return "Pronađeno:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return "Izvorni kod datoteke " + filename;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Stablo direktorija";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Dokumentacija direktorija";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Direktoriji";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Stablo direktorija sortirano abecednim redom:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = "Opis direktorija ";
      result += dirName;
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Direktorij" : "direktorij"));
      if (!singular) {
         result += "i";
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
      return "Ovo je preopterećena funkcija (overload). "
             "Razlikuje se od navedene metode "
             "samo po vrsti argumenata koje prihvaća.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Ovo je graf funkcija koje pozivaju ovu funkciju:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Dokumentacija enumeracija";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Dokumentacija member funkcija/subrutina";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Lista tipova podataka";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Polja";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Tipovi podataka s kratkim opisom:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Lista svih ";
      if (!extractAll) {
         result += "dokumentiranih ";
      }
      result += "polja";
      result += " s linkovima na ";
      if (!extractAll) {
         result += "dokumentaciju struktura podataka za svako polje";
      } else {
         result += "strukture kojima pripadaju:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
   * annotated compound index (Fortran).
   */
   virtual QByteArray trCompoundIndexFortran() {
      return "Kazalo data tipova";
   }

   /*! This is used in LaTeX as the title of the chapter containing
   *  the documentation of all data types (Fortran).
   */
   virtual QByteArray trTypeDocumentation() {
      return "Dokumentacija tipova podataka";
   }

   /*! This is used in the documentation of a file as a header before the
   *  list of (global) subprograms (Fortran).
   */
   virtual QByteArray trSubprograms() {
      return "Funkcije/Subrutine";
   }

   /*! This is used in the documentation of a file/namespace before the list
   *  of documentation blocks for subprograms (Fortran)
   */
   virtual QByteArray trSubprogramDocumentation() {
      return "Documentacija funkcija/subrutina";
   }

   /*! This is used in the documentation of a file/namespace/group before
   *  the list of links to documented compounds (Fortran)
   */
   virtual QByteArray trDataTypes() {
      return "Tipovi podataka";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Popis modula";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Lista svih ";
      if (!extractAll) {
         result += "dokumentiranih ";
      }
      result += "modula s kratkim opisom:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      switch (compType) {
         case ClassDef::Class:
            result += " Modul";
            break;
         case ClassDef::Struct:
            result += " Tip";
            break;
         case ClassDef::Union:
            result += " Unija";
            break;
         case ClassDef::Interface:
            result += " Sučelje";
            break;
         case ClassDef::Protocol:
            result += " Protokol";
            break;
         case ClassDef::Category:
            result += " Kategorija";
            break;
         case ClassDef::Exception:
            result += " Iznimka";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += " Predložak";
      }
      result += " Referenca";
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " - Sadržaj modula";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "članovi modula";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Lista svih ";
      if (!extractAll) {
         result += "dokumentiranih ";
      }
      result += "članova modula s linkovima na ";
      if (extractAll) {
         result += "dokumentaciju modula za svaki član:";
      } else {
         result += "modul kojem pripadaju:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
   *  index of all modules (Fortran).
   */
   virtual QByteArray trModulesIndex() {
      return "Kazalo modula";
   }

   /*! This is used for translation of the word that will possibly
   *  be followed by a single name or by a list of names
   *  of the category.
   */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Modul" : "modul"));
      if (!singular) {
         result += "i";
      }
      return result;
   }
   /*! This is put at the bottom of a module documentation page and is
   *  followed by a list of files that were used to generate the page.
   */
   virtual QByteArray trGeneratedFromFilesFortran(ClassDef::CompoundType compType,
         bool) {
      // here s is one of " Module", " Struct" or " Union"
      // single is true implies a single file
      QByteArray result = (QByteArray)"Dokumentacija ovog ";
      switch (compType) {
         case ClassDef::Class:
            result += "modula";
            break;
         case ClassDef::Struct:
            result += "tipa";
            break;
         case ClassDef::Union:
            result += "unije";
            break;
         case ClassDef::Interface:
            result += "sučelja";
            break;
         case ClassDef::Protocol:
            result += "protokola";
            break;
         case ClassDef::Category:
            result += "kategorije";
            break;
         case ClassDef::Exception:
            result += "iznimke";
            break;
         default:
            break;
      }
      result += " je napravljena iz :";
      return result;
   }
   /*! This is used for translation of the word that will possibly
   *  be followed by a single name or by a list of names
   *  of the category.
   */
   virtual QByteArray trType(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Tip" : "tip"));
      if (!singular) {
         result += "ovi";
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
         result += "i";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Ograničenja tipova (Type Constraints)";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return QByteArray("Relacije ") + QByteArray(name);
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Učitavam...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Globalni namespace";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Tražim...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Nema traženih podataka";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Datoteka u " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Uključuje datotake u " + name;
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
      static const char *days[]   = { "Pon", "Uto", "Sri", "Čet", "Pet", "Sub", "Ned" };
      static const char *months[] = { "Sje", "Velj", "Ožu", "Tra", "Svi", "Lip", "Srp", "Kol", "Ruj", "Lis", "Stu", "Pro" };
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
      return "Bibliografija";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Direktoriji o kojima ovisi ") + name + ":";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.0
   //////////////////////////////////////////////////////////////////////////

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "razina detalja";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Parametri predloška";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "i još " + number + " ...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "Dokumatacija za ovu enumeraciju je generirana iz ";
      if (!single) {
         result += "datoteka:";
      } else {
         result += "datoteke:";
      }
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return "Opis enumeracije " + QByteArray(name);
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " naslijeđeni od " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Dodatni naslijeđeni članovi";
   }

   //////////////////////////////////////////////////////////////////////////

};

#endif


