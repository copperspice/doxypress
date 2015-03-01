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

#ifndef TRANSLATOR_SI_H
#define TRANSLATOR_SI_H

class TranslatorSlovene : public TranslatorAdapter_1_4_6
{
 protected:
   friend class TranslatorAdapterBase;
   virtual ~TranslatorSlovene() {}

 public:
   QByteArray idLanguage() {
      return "slovene";
   }
   /*! Used to get the command(s) for the language support. This method
    *  was designed for languages which do not prefer babel package.
    *  If this methods returns empty string, then the latexBabelPackage()
    *  method is used to generate the command for using the babel package.
    */
   QByteArray latexLanguageSupportCommand() {
      return "\\usepackage[slovene]{babel}\n\\usepackage[T1]{fontenc}\n";
   }
   QByteArray trRelatedFunctions() {
      return "Povezane funkcije";
   }
   QByteArray trRelatedSubscript() {
      return "(To niso metode.)";
   }
   QByteArray trDetailedDescription() {
      return "Podroben opis";
   }
   QByteArray trMemberTypedefDocumentation() {
      return "Opis uporabniško definiranih tipov";
   }
   QByteArray trMemberEnumerationDocumentation() {
      return "Opis komponent  naštevnih tipov";
   }
   /*     QByteArray trEnumerationValueDocumentation() */
   /*     { return "Opis vrednosti naštevnih tipov (enum) "; } */
   QByteArray trMemberFunctionDocumentation() {
      return "Opis metod";
   }
   QByteArray trMemberDataDocumentation() {
      return "Opis atributov";
   }
   QByteArray trMore() {
      return "...";
   }
   QByteArray trListOfAllMembers() {
      return "Seznam vseh metod / atributov";
   }
   QByteArray trMemberList() {
      return " - seznam metod in atributov.";
   }
   QByteArray trThisIsTheListOfAllMembers() {
      return "Seznam metod razreda ";
   }
   QByteArray trIncludingInheritedMembers() {
      return ", vključujoč dedovane metode in atribute.";
   }
   QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "zgenerirano z Doxygen-om";
      if (s) {
         result += (QByteArray)" za " + s;
      }
      result += " iz izvorne kode.";
      return result;
   }
   QByteArray trEnumName() {
      return "naštevno ime";
   }
   QByteArray trEnumValue() {
      return "naštevna vrednost";
   }
   QByteArray trDefinedIn() {
      return "definirano v";
   }
   QByteArray trModules() {
      return "moduli";
   }
   QByteArray trClassHierarchy() {
      return "dedovalna hierarhija";
   }
   QByteArray trCompoundList() {
      return "kratek opis razredov";
   }
   QByteArray trFileList() {
      return "seznam datotek";
   }
   /*     QByteArray trHeaderFiles() */
   /*     { return "'Header' datoteka"; } */
   QByteArray trCompoundMembers() {
      return "metode in atributi";
   }
   QByteArray trFileMembers() {
      return "komponente v datotekah";
   }
   QByteArray trRelatedPages() {
      return "dodatni opisi";
   }
   QByteArray trExamples() {
      return "Primeri";
   }
   QByteArray trSearch() {
      return "Išči";
   }
   QByteArray trClassHierarchyDescription() {
      return "Hierarhično drevo je (okvirno) sortirano po abecedi. ";
   }
   QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Seznam vseh ";
      if (!extractAll) {
         result += "dokumentiranih ";
      }
      result += "datotek s kratkim opisom:";
      return result;
   }
   QByteArray trCompoundListDescription() {
      return "Seznam razredov, množic in struktur "
             "s kratkim opisom :";
   }
   QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Seznam vseh ";
      if (!extractAll) {
         result += "dokumentiranih ";
      }
      result += "metod in atributov s povezavami na ";
      if (extractAll) {
         result += "opis posamezne metode in/ali atributa:";
      } else {
         result += "opis razreda :";
      }
      return result;
   }
   QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Seznam ";
      if (!extractAll) {
         result += "dokumentiranih ";
      }
      result += "entitet v datotekah ";
      if (extractAll) {
         result += "skupaj z opisom datoteke v kateri se nahajajo:";
      } else {
         result += "s povezavami na datoteke v katerih se nahajajo:";
      }
      return result;
   }
   /*     QByteArray trHeaderFilesDescription() */
   /*     { return "Seznam header datotek, ki tvorijo aplikacijski vmesnik (API) :"; } */
   QByteArray trExamplesDescription() {
      return "Seznam primerov :";
   }
   QByteArray trRelatedPagesDescription() {
      return "Seznam strani z dodatnimi opisi:";
   }
   QByteArray trModulesDescription() {
      return "Seznam modulov:";
   }
   /*     QByteArray trNoDescriptionAvailable() */
   /*     { return "Opis ni dostopen"; } */

   QByteArray trDocumentation() {
      return "Dokumentacija";
   }
   QByteArray trModuleIndex() {
      return "seznam modulov";
   }
   QByteArray trHierarchicalIndex() {
      return "Hierarhični indeks";
   }
   QByteArray trCompoundIndex() {
      return "abecedni seznam";
   }
   QByteArray trFileIndex() {
      return "seznam datotek";
   }
   QByteArray trModuleDocumentation() {
      return "Dokumentacija modulov";
   }
   QByteArray trClassDocumentation() {
      return "Opis razreda";
   }
   QByteArray trFileDocumentation() {
      return "Opis datoteke";
   }
   QByteArray trExampleDocumentation() {
      return "Opis primera";
   }
   QByteArray trPageDocumentation() {
      return "Opis povezanih strani";
   }
   QByteArray trReferenceManual() {
      return "Priročnik";
   }

   QByteArray trDefines() {
      return "Makro deklaracije";
   }
   QByteArray trTypedefs() {
      return "Uporabniško definirani tipi";
   }
   QByteArray trEnumerations() {
      return "Naštevni tipi";
   }
   QByteArray trFunctions() {
      return "Funkcije";
   }
   QByteArray trVariables() {
      return "Spremenljivke";
   }
   QByteArray trEnumerationValues() {
      return "Vrednosti naštevnih tipov";
   }
   QByteArray trDefineDocumentation() {
      return "Opis makro definicije";
   }
   QByteArray trTypedefDocumentation() {
      return "Opis uporabniško definiranega tipa";
   }
   QByteArray trEnumerationTypeDocumentation() {
      return "Opis naštevnega (enum) tipa";
   }
   QByteArray trFunctionDocumentation() {
      return "Opis funkcije";
   }
   QByteArray trVariableDocumentation() {
      return "Opis spremenljivke";
   }
   QByteArray trCompounds() {
      return "Strukture";
   }
   QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Generirano " + date;
      if (projName) {
         result += (QByteArray)" projekt: " + projName;
      }
      result += (QByteArray)" generator: ";
      return result;
   }
   QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Diagram razredov za " + clName;
   }
   QByteArray trForInternalUseOnly() {
      return "Samo za interno uporabo.";
   }
   /*     QByteArray trReimplementedForInternalReasons() */
   /*     { return "Ponovno implementirano zaradi internih razlogov. " */
   /*              "Nima vpliva na API.";  */
   /*     } */
   QByteArray trWarning() {
      return "Opozorilo";
   }
   /*     QByteArray trBugsAndLimitations() */
   /*     { return "Napake in omejtive"; } */
   QByteArray trVersion() {
      return "Verzija";
   }
   QByteArray trDate() {
      return "Datum";
   }
   QByteArray trReturns() {
      return "Rezultat(i)";
   }
   QByteArray trSeeAlso() {
      return "Glej";
   }
   QByteArray trParameters() {
      return "Parametri";
   }
   QByteArray trExceptions() {
      return "Prekinitve";
   }
   QByteArray trGeneratedBy() {
      return "Izdelano s pomočjo";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   QByteArray trNamespaceList() {
      return "imenski prostori";
   }
   QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Seznam ";
      if (!extractAll) {
         result += "dokumentiranih ";
      }
      result += "imenskih prostorov z opisom:";
      return result;
   }
   QByteArray trFriends() {
      return "Prijatelji (Friends) ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   QByteArray trRelatedFunctionDocumentation() {
      return "Podatki o poveznih funkcijah";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   QByteArray trCompoundReference(const char *clName,
                                  ClassDef::CompoundType compType,
                                  bool /*isTemplate*/)
   // used as the title of the HTML page of a class/struct/union
   {
      QByteArray result = "";
      switch (compType) {
         case ClassDef::Class:
            result += " Razred ";
            break;
         case ClassDef::Struct:
            result += " Struktura ";
            break;
         case ClassDef::Union:
            result += " Množica ";
            break;
         case ClassDef::Interface:
            result += " IDL vmesnik ";
            break;
         case ClassDef::Protocol:
            result += " protocol ";
            break; // translate me!
         case ClassDef::Category:
            result += " category ";
            break; // translate me!
         case ClassDef::Exception:
            result += " IDL prekinitev ";
            break;
         default:
            break;
      }
      result += (QByteArray)clName;

      return result;
   }
   QByteArray trFileReference(const char *fileName)
   // used as the title of the HTML page of a file
   {
      QByteArray result = "Datoteka ";
      result += fileName;
      return result;
   }
   QByteArray trNamespaceReference(const char *namespaceName)
   // used as the title of the HTML page of a namespace
   {
      QByteArray result = "Imenski prostor ";
      result += namespaceName;

      return result;
   }

   // these are for the member sections of a class, struct or union
   QByteArray trPublicMembers() {
      return "Javne metode";
   }
   QByteArray trPublicSlots() {
      return "Public slotovi";
   }
   QByteArray trSignals() {
      return "Programske prekinitve";
   }
   QByteArray trStaticPublicMembers() {
      return "Statične javne metode in atributi";
   }
   QByteArray trProtectedMembers() {
      return "Zaščitene metode in atributi";
   }
   QByteArray trProtectedSlots() {
      return "Zaščiteni sloti";
   }
   QByteArray trStaticProtectedMembers() {
      return "Statične zaščitene metode in atributi";
   }
   QByteArray trPrivateMembers() {
      return "Skrite metode in atributi";
   }
   QByteArray trPrivateSlots() {
      return "Skriti slotovi";
   }
   QByteArray trStaticPrivateMembers() {
      return "Statične skrite metode in atributi";
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
               result += " in ";
            }
         }
      }
      return result;
   }

   QByteArray trInheritsList(int numEntries)
   // used in class documentation to produce a list of base classes,
   // if class diagrams are disabled.
   {
      return "Deduje od " + trWriteList(numEntries) + ".";
   }
   QByteArray trInheritedByList(int numEntries)
   // used in class documentation to produce a list of super classes,
   // if class diagrams are disabled.
   {
      return "Naslijeðena u " + trWriteList(numEntries) + ".";
   }
   QByteArray trReimplementedFromList(int numEntries)
   // used in member documentation blocks to produce a list of
   // members that are hidden by this one.
   {
      return "Skrije implementacijo iz " + trWriteList(numEntries) + ".";
   }
   QByteArray trReimplementedInList(int numEntries) {
      // used in member documentation blocks to produce a list of
      // all member that overwrite the implementation of this member.
      return "Metodo skrijejo implementacije v razredih " + trWriteList(numEntries) + ".";
   }

   QByteArray trNamespaceMembers()
   // This is put above each page as a link to all members of namespaces.
   {
      return "elementi imenskega prostora";
   }
   QByteArray trNamespaceMemberDescription(bool extractAll)
   // This is an introduction to the page with all namespace members
   {
      QByteArray result = "Seznam vseh ";
      if (!extractAll) {
         result += "dokumentiranih ";
      }
      result += "elementov imenskega prostora s povezavami na ";
      if (extractAll) {
         result += "opis vsakega elementa:";
      } else {
         result += "imenski prostor, ki mu pripadajo:";
      }
      return result;
   }
   QByteArray trNamespaceIndex()
   // This is used in LaTeX as the title of the chapter with the
   // index of all namespaces.
   {
      return "Indeks imenskih prostorov";
   }
   QByteArray trNamespaceDocumentation()
   // This is used in LaTeX as the title of the chapter containing
   // the documentation of all namespaces.
   {
      return "Podatki o imenskih prostorih";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Imenski prostori";
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
      QByteArray result = (QByteArray)"Opis ";
      switch (compType) {
         case ClassDef::Class:
            result += "razreda";
            break;
         case ClassDef::Struct:
            result += "strukture";
            break;
         case ClassDef::Union:
            result += "unije";
            break;
         case ClassDef::Interface:
            result += "vmesnika (interface)";
            break;
         case ClassDef::Protocol:
            result += "protocol";
            break; // translate me!
         case ClassDef::Category:
            result += "category";
            break; // translate me!
         case ClassDef::Exception:
            result += "prekinitve (exception)";
            break;
         default:
            break;
      }
      result += " je zgrajen na podlagi naslednj";
      if (single) {
         result += "e ";
      } else {
         result += "ih";
      }
      result += " datotek";
      if (single) {
         result += "e :";
      } else {
         result += " :";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   QByteArray trReturnValues() {
      return "Povratna vrednost";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   QByteArray trMainPage() {
      return "prva stran";
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

   /*     QByteArray trSources() */
   /*     { */
   /*       return "Izvorne datoteke"; */
   /*     } */
   QByteArray trDefinedAtLineInSourceFile() {
      return "Definirano v @0 vrstici datoteke @1.";
   }
   QByteArray trDefinedInSourceFile() {
      return "Definirano v datoteki @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   QByteArray trDeprecated() {
      return "Zastarelo";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Kolaboracijski diagram razreda " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Graf prikazuje seznam datotek, "
             "ki jih datoteka \"" + fName + "\" "
             "direktno ali indirektno vključuje. Pravokotniki ponazarjajo datoteke, puščice "
             "predstavljajo relacije med njimi. "
             "črn pravokotnik ponazarja datoteko " + fName + ". Puščice A->B ponazarjajo "
             "usmerjeno relacijo \"A vključuje B\"."
             ;
   }
   /*! header that is put before the list of constructor/destructors. */
   QByteArray trConstructorDocumentation() {
      return "Opis konstruktorjev in destruktorjev ";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   QByteArray trGotoSourceCode() {
      return "izvorna koda";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   QByteArray trGotoDocumentation() {
      return "dokumenacija tekoče datoteke.";
   }
   /*! Text for the \pre command */
   QByteArray trPrecondition() {
      return "Predpogoji (preconditions)";
   }
   /*! Text for the \post command */
   QByteArray trPostcondition() {
      return "Naknadni pogoji (posconditions)";
   }
   /*! Text for the \invariant command */
   QByteArray trInvariant() {
      return "Invarianta";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   QByteArray trInitialValue() {
      return "Začetna vrednost / definicija :";
   }
   /*! Text used the source code in the file index */
   QByteArray trCode() {
      return "koda";
   }
   QByteArray trGraphicalHierarchy() {
      return "Hierarhija razredov v grafični obliki";
   }
   QByteArray trGotoGraphicalHierarchy() {
      return "Dedovalna hierarhija v grafični obliki";
   }
   QByteArray trGotoTextualHierarchy() {
      return "Dedovalna hierarhija v tekstovni obliki";
   }
   QByteArray trPageIndex() {
      return "Indeks strani";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   QByteArray trNote() {
      return "Opomba";
   }
   QByteArray trPublicTypes() {
      return "Javni tipi";
   }
   QByteArray trPublicAttribs() {
      return "Javni atributi";
   }
   QByteArray trStaticPublicAttribs() {
      return "Statični javni atributi";
   }
   QByteArray trProtectedTypes() {
      return "Zaščiteni tipi";
   }
   QByteArray trProtectedAttribs() {
      return "Zaščiteni atributi";
   }
   QByteArray trStaticProtectedAttribs() {
      return "Statični zaščiteni tipi";
   }
   QByteArray trPrivateTypes() {
      return "Skriti tipi";
   }
   QByteArray trPrivateAttribs() {
      return "Skriti atributi";
   }
   QByteArray trStaticPrivateAttribs() {
      return "Statični skriti atributi";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a todo item */
   virtual QByteArray trTodo() {
      return "TODO";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Seznam nedokončanih opravil";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Uporabniki entitete:  ";
   }
   virtual QByteArray trRemarks() {
      return "Opomba";
   }
   virtual QByteArray trAttention() {
      return "Pozor";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Graf prikazuje datoteke, ki posredno ali neposredno "
             "vključujejo tekočo datoteko. Pravokotniki simbolizirajo datoteke, "
             "puščice pa relacije med datotekami. Tekoča datoteka je prikazana "
             "kot pravokotnik s črno podlago, ostale pa kot pravokotnik brez podlage. "
             "Smer puščice A->B definira relacijo \"A vključuje B\". "
             "Vse datoteke, ki torej mejijo na tekočo (t.j. obstaja povezava med črnim in "
             "praznim pravokotnikom), jo direktno vključujejo, medtem, ko jo ostale vključujejo "
             "le posredno. "
             ;
   }
   virtual QByteArray trSince() {
      return "Od";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Legenda grafa";
   }
   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Tekoča stran pojasnjuje način interpretacije grafov, ki jih izriše "
         "doxygen.<p>\n"
         "Poglejmo si naslednji primer:\n"
         "\\code\n"
         "/*! Nevide razred zaradi rezanja */\n"
         "class Invisible { };\n\n"
         "/*! Odrezan razred, dedovalna relacija je skrita */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* razred, ki ni opisan z doxygen komentarji */\n"
         "class Undocumented { };\n\n"
         "/*! Razred, ki ga dedujemo s pomočjo javnega dedovanja */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! A template class */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Razred, ki ga dedujemo s pomočjo zaščitenega dedovanja */\n"
         "class ProtectedBase { };\n\n"
         "/*! Razred, ki ga dedujemo s pomočjo skritega dedovanja */\n"
         "class PrivateBase { };\n\n"
         "/*! Razred, ki ga uporablja dedovani razred */\n"
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
         "If the \\c MAX_DOT_GRAPH_HEIGHT tag in the configuration file "
         "is set to 200 this will result in the following graph:"
         "<p><center><img src=\"graph_legend." + Config::getEnum("dot-image-format").toUtf8() + "\"></center>\n"
         "<p>\n"
         "The boxes in the above graph have the following meaning:\n"
         "<ul>\n"
         "<li>%A filled black box represents the struct or class for which the "
         "graph is generated.\n"
         "<li>%A box with a black border denotes a documented struct or class.\n"
         "<li>%A box with a grey border denotes an undocumented struct or class.\n"
         "<li>%A box with a red border denotes a documented struct or class for\n"
         "which not all inheritance/containment relations are shown. %A graph is "
         "truncated if it does not fit within the specified boundaries."
         "</ul>\n"
         "The arrows have the following meaning:\n"
         "<ul>\n"
         "<li>%A dark blue arrow is used to visualize a public inheritance "
         "relation between two classes.\n"
         "<li>%A dark green arrow is used for protected inheritance.\n"
         "<li>%A dark red arrow is used for private inheritance.\n"
         "<li>%A purple dashed arrow is used if a class is contained or used "
         "by another class. The arrow is labeled with the variable(s) "
         "through which the pointed class or struct is accessible. \n"
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
      return "Test List";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "IDL Lastnosti";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Opis IDL lastnosti";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java interfaces in the summary section of Java packages */
   /*     virtual QByteArray trInterfaces() */
   /*     { */
   /*       return "Vmesniki"; */
   /*     } */
   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config::getBool("optimize-c")) {
         return "Podatkovne strukture";
      } else {
         return "Razredi";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"JAVA paket " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Seznam JAVA paketov";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Seznam JAVA paketov in njihovih kratkih opisov v primeru, da obstajajo:";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "JAVA paketi";
   }
   /*! Used as a chapter title for Latex & RTF output */
   /*     virtual QByteArray trPackageDocumentation() */
   /*     { */
   /*       return "Opisi JAVA paketov"; */
   /*     } */
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Vrednost:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Programska napaka";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Seznam programskih napak";
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
      return "Indeks";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Razred" : "razred"));
      if (!singular) {
         result += "i";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Datotek" : "datotek"));
      if (!singular) {
         result += "e";
      } else {
         result += "a";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Imenski prostor" : "imenski prostor"));
      if (!singular) {
         result += "i";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Skupina" : "skupina"));
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
      QByteArray result((first_capital ? "Stran" : "stran"));
      if (!singular) {
         result += "i";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Element" : "element"));
      if (!singular) {
         result += "i";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   /*     virtual QByteArray trField(bool first_capital, bool singular) */
   /*     {  */
   /*       QByteArray result((first_capital ? "Polj" : "polj")); */
   /*       if (!singular)  result+="a"; */
   /*       else result += "e"; */
   /*       return result;  */
   /*     } */

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
      QByteArray result((first_capital ? "Avtor" : "avtor"));
      if (!singular) {
         result += "ji";
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
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Implementirano v " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Vsebina";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Seznam opuščenih";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Dogodki";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Opisi dogodkov";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Tipi paketov";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Funkcije paketa";   /* don't know the context */
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Statične funkcije paketa";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Atributi paketa";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Statični atributi paketa";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Vse";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Graf klicev tekoče funkcije:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Rezultat(i) iskanja";
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
         return "Oprostite, noben dokument ne ustreza vašemu povpraševanju.";
      } else if (numDocuments == 1) {
         return "Našel sem <b>1</b> dokument, ki ustreza vašemu povpraševanju.";
      } else if (numDocuments == 2) {
         return "Našel sem <b>2</b> dokumenta, ki ustrezata vašemu povpraševanju.";
      } else {
         return "Našel sem <b>$num</b> dokumentov, ki ustrezajo vašemu povpraševanju. "
                "Dokumenti z najboljšo stopnjo ujemanja se nahajajo na začetku.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Zadetki:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return filename + " izvorna koda";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Hierarhija imenikov";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Opisi imenikov";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Imeniki";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Imeniška hierarhija je urejena v glavnem, toda ne popolnoma, po abecedi, ";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " imeniške reference"; /* not sure for context */
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Imenik" : "imenik"));
      if (singular) {
         result += "i";
      } else {
         result += "";
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
      return "To je ponovno definirana metoda, " /* don't know Slovene expresion for overloaded */
             "podana je zaradi priročnosti. Metoda se od predhodnje razlikuje "
             "samo v številu in/ali tipu formalnih argumentov.";
   }
};

#endif
