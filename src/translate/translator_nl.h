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

#ifndef TRANSLATOR_NL_H
#define TRANSLATOR_NL_H

class TranslatorDutch : public Translator
{
 public:
   QByteArray idLanguage() {
      return "dutch";
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
   QByteArray latexLanguageSupportCommand() {
      return "\\usepackage[dutch]{babel}\n";
   }
   QByteArray trRelatedFunctions() {
      return "Gerelateerde functies";
   }
   QByteArray trRelatedSubscript() {
      return "(Merk op dat dit geen member functies zijn.)";
   }
   QByteArray trDetailedDescription() {
      return "Gedetailleerde Beschrijving";
   }
   QByteArray trMemberTypedefDocumentation() {
      return "Documentatie van type definitie members";
   }
   QByteArray trMemberEnumerationDocumentation() {
      return "Documentatie van enumeratie members";
   }
   QByteArray trMemberFunctionDocumentation() {
      return "Documentatie van functie members";
   }
   QByteArray trMemberDataDocumentation() {
      return "Documentatie van data members";
   }
   QByteArray trMore() {
      return "Meer...";
   }
   QByteArray trListOfAllMembers() {
      return "Lijst van alle members";
   }
   QByteArray trMemberList() {
      return "Member Lijst";
   }
   QByteArray trThisIsTheListOfAllMembers() {
      return "Dit is de complete lijst van alle members voor";
   }
   QByteArray trIncludingInheritedMembers() {
      return ", inclusief alle overge&euml;rfde members.";
   }
   QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Automatisch gegenereerd door Doxygen";
      if (s) {
         result += (QByteArray)" voor " + s;
      }
      result += " uit de programmatekst.";
      return result;
   }
   QByteArray trEnumName() {
      return "enum naam";
   }
   QByteArray trEnumValue() {
      return "enum waarde";
   }
   QByteArray trDefinedIn() {
      return "gedefinieerd in";
   }
   QByteArray trModules() {
      return "Modules";
   }
   QByteArray trClassHierarchy() {
      return "Klasse Hi&euml;rarchie";
   }
   QByteArray trCompoundList() {
      return "Klasse Lijst";
   }
   QByteArray trFileList() {
      return "Bestandslijst";
   }
   QByteArray trCompoundMembers() {
      return "Klasse Members";
   }
   QByteArray trFileMembers() {
      return "Bestand members";
   }
   QByteArray trRelatedPages() {
      return "Gerelateerde pagina's";
   }
   QByteArray trExamples() {
      return "Voorbeelden";
   }
   QByteArray trSearch() {
      return "Zoeken";
   }
   QByteArray trClassHierarchyDescription() {
      return "Deze inheritance lijst is min of meer alfabetisch "
             "gesorteerd:";
   }
   QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Hieronder volgt de lijst met alle ";
      if (!extractAll) {
         result += "gedocumenteerde ";
      }
      result += "bestanden, elk met een korte beschrijving:";
      return result;
   }
   QByteArray trCompoundListDescription() {
      return "Hieronder volgen de klassen, structs en "
             "unions met voor elk een korte beschrijving:";
   }
   QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Hieronder volgt de lijst met alle ";
      if (!extractAll) {
         result += "gedocumenteerde ";
      }
      result += "klasse members met links naar ";
      if (!extractAll) {
         result += "de klasse documentatie voor elke member:";
      } else {
         result += "de klassen waartoe ze behoren:";
      }
      return result;
   }
   QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Hieronder volgt de lijst met alle ";
      if (!extractAll) {
         result += "gedocumenteerde ";
      }
      result += "bestand members met links naar ";
      if (extractAll) {
         result += "de bestand's documentatie voor elke member:";
      } else {
         result += "de bestanden waartoe ze behoren:";
      }
      return result;
   }
   QByteArray trExamplesDescription() {
      return "Hieronder volgt de lijst met alle voorbeelden:";
   }
   QByteArray trRelatedPagesDescription() {
      return "Hieronder volgt de lijst met alle pagina's die gerelateerde documentatie bevatten:";
   }
   QByteArray trModulesDescription() {
      return "Hieronder volgt de lijst met alle modules:";
   }

   QByteArray trDocumentation() {
      return "Documentatie";
   }
   QByteArray trModuleIndex() {
      return "Module Index";
   }
   QByteArray trHierarchicalIndex() {
      return "Hi&euml;rarchische Index";
   }
   QByteArray trCompoundIndex() {
      return "Klasse Index";
   }
   QByteArray trFileIndex() {
      return "Bestand Index";
   }
   QByteArray trModuleDocumentation() {
      return "Module Documentatie";
   }
   QByteArray trClassDocumentation() {
      return "Klassen Documentatie";
   }
   QByteArray trFileDocumentation() {
      return "Bestand Documentatie";
   }
   QByteArray trExampleDocumentation() {
      return "Documentatie van voorbeelden";
   }
   QByteArray trPageDocumentation() {
      return "Documentatie van gerelateerde pagina's";
   }
   QByteArray trReferenceManual() {
      return "Naslagwerk";
   }

   QByteArray trDefines() {
      return "Macros";
   }
   QByteArray trTypedefs() {
      return "Typedefs";
   }
   QByteArray trEnumerations() {
      return "Enumeraties";
   }
   QByteArray trFunctions() {
      return "Functies";
   }
   QByteArray trVariables() {
      return "Variabelen";
   }
   QByteArray trEnumerationValues() {
      return "Enumeratie waarden";
   }
   QByteArray trDefineDocumentation() {
      return "Documentatie van macro's";
   }
   QByteArray trTypedefDocumentation() {
      return "Documentatie van typedefs";
   }
   QByteArray trEnumerationTypeDocumentation() {
      return "Documentatie van enumeratie types";
   }
   QByteArray trEnumerationValueDocumentation() {
      return "Documentatie van enumeratie waarden";
   }
   QByteArray trFunctionDocumentation() {
      return "Documentatie van functies";
   }
   QByteArray trVariableDocumentation() {
      return "Documentatie van variabelen";
   }
   QByteArray trCompounds() {
      return "Klassen";
   }
   QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Gegenereerd op " + date;
      if (projName) {
         result += (QByteArray)" voor " + projName;
      }
      result += (QByteArray)" door";
      return result;
   }
   QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Klasse diagram voor " + clName;
   }
   QByteArray trForInternalUseOnly() {
      return "Alleen voor intern gebruik.";
   }
   QByteArray trWarning() {
      return "Waarschuwing";
   }
   QByteArray trVersion() {
      return "Versie";
   }
   QByteArray trDate() {
      return "Datum";
   }
   QByteArray trReturns() {
      return "Retourneert";
   }
   QByteArray trSeeAlso() {
      return "Zie ook";
   }
   QByteArray trParameters() {
      return "Parameters";
   }
   QByteArray trExceptions() {
      return "Excepties";
   }
   QByteArray trGeneratedBy() {
      return "Gegenereerd door";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   QByteArray trNamespaceList() {
      return "Namespace Lijst";
   }
   QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Hier is een lijst met alle ";
      if (!extractAll) {
         result += "gedocumenteerde ";
      }
      result += "namespaces met voor elk een korte beschrijving:";
      return result;
   }
   QByteArray trFriends() {
      return "Friends";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   QByteArray trRelatedFunctionDocumentation() {
      return "Documentatie van friends en gerelateerde functies";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   QByteArray trCompoundReference(const char *clName,
                                  ClassDef::CompoundType compType,
                                  bool isTemplate)
   // used as the title of the HTML page of a class/struct/union
   {
      QByteArray result = (QByteArray)clName;
      if (isTemplate) {
         result += " Template";
      }
      switch (compType) {
         case ClassDef::Class:
            result += " Class";
            break;
         case ClassDef::Struct:
            result += " Struct";
            break;
         case ClassDef::Union:
            result += " Union";
            break;
         case ClassDef::Interface:
            result += " Interface";
            break;
         case ClassDef::Protocol:
            result += " Protocol";
            break;
         case ClassDef::Category:
            result += " Category";
            break;
         case ClassDef::Exception:
            result += " Exception";
            break;
         default:
            break;
      }
      result += " Referentie";
      return result;
   }
   QByteArray trFileReference(const char *fileName)
   // used as the title of the HTML page of a file
   {
      QByteArray result = fileName;
      result += " Bestand Referentie";
      return result;
   }
   QByteArray trNamespaceReference(const char *namespaceName)
   // used as the title of the HTML page of a namespace
   {
      QByteArray result = namespaceName;
      result += " Namespace Referentie";
      return result;
   }

   // these are for the member sections of a class, struct or union
   QByteArray trPublicMembers() {
      return "Public Members";
   }
   QByteArray trPublicSlots() {
      return "Public Slots";
   }
   QByteArray trSignals() {
      return "Signals";
   }
   QByteArray trStaticPublicMembers() {
      return "Static Public Members";
   }
   QByteArray trProtectedMembers() {
      return "Protected Members";
   }
   QByteArray trProtectedSlots() {
      return "Protected Slots";
   }
   QByteArray trStaticProtectedMembers() {
      return "Static Protected Members";
   }
   QByteArray trPrivateMembers() {
      return "Private Members";
   }
   QByteArray trPrivateSlots() {
      return "Private Slots";
   }
   QByteArray trStaticPrivateMembers() {
      return "Static Private Members";
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
               result += " en ";
            }
         }
      }
      return result;
   }

   QByteArray trInheritsList(int numEntries)
   // used in class documentation to produce a list of base classes,
   // if class diagrams are disabled.
   {
      return "Erft over van " + trWriteList(numEntries) + ".";
   }
   QByteArray trInheritedByList(int numEntries)
   // used in class documentation to produce a list of super classes,
   // if class diagrams are disabled.
   {
      return "Wordt overge&euml;rfd door " + trWriteList(numEntries) + ".";
   }
   QByteArray trReimplementedFromList(int numEntries)
   // used in member documentation blocks to produce a list of
   // members that are hidden by this one.
   {
      return "Nieuwe implementatie van " + trWriteList(numEntries) + ".";
   }
   QByteArray trReimplementedInList(int numEntries) {
      // used in member documentation blocks to produce a list of
      // all member that overwrite the implementation of this member.
      return "Opnieuw ge&iuml;mplementeerd in " + trWriteList(numEntries) + ".";
   }

   QByteArray trNamespaceMembers()
   // This is put above each page as a link to all members of namespaces.
   {
      return "Namespace Members";
   }
   QByteArray trNamespaceMemberDescription(bool extractAll)
   // This is an introduction to the page with all namespace members
   {
      QByteArray result = "Hier is een lijst van alle ";
      if (!extractAll) {
         result += "gedocumenteerde ";
      }
      result += "namespace members met links naar ";
      if (extractAll) {
         result += "de namespace documentatie voor iedere member:";
      } else {
         result += "de namespaces waartoe ze behoren:";
      }
      return result;
   }
   QByteArray trNamespaceIndex()
   // This is used in LaTeX as the title of the chapter with the
   // index of all namespaces.
   {
      return "Namespace Index";
   }
   QByteArray trNamespaceDocumentation()
   // This is used in LaTeX as the title of the chapter containing
   // the documentation of all namespaces.
   {
      return "Namespace Documentatie";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   QByteArray trNamespaces() {
      return "Namespaces";
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
      QByteArray result = (QByteArray)"De documentatie voor ";
      switch (compType) {
         case ClassDef::Class:
            result += "deze klasse";
            break;
         case ClassDef::Struct:
            result += "deze struct";
            break;
         case ClassDef::Union:
            result += "deze union";
            break;
         case ClassDef::Interface:
            result += "dit interface";
            break;
         case ClassDef::Protocol:
            result += "dit protocol";
            break;
         case ClassDef::Category:
            result += "deze categorie";
            break;
         case ClassDef::Exception:
            result += "deze exceptie";
            break;
         default:
            break;
      }
      result += " is gegenereerd op grond van ";
      if (single) {
         result += "het";
      } else {
         result += "de";
      }
      result += " volgende bestand";
      if (single) {
         result += ":";
      } else {
         result += "en:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   QByteArray trReturnValues() {
      return "Retour waarden";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   QByteArray trMainPage() {
      return "Hoofd Pagina";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   QByteArray trPageAbbreviation() {
      return "p.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991106
   //////////////////////////////////////////////////////////////////////////

   QByteArray trDefinedAtLineInSourceFile() {
      return "De definitie bevindt zich op regel @0 in het bestand @1.";
   }
   QByteArray trDefinedInSourceFile() {
      return "De definitie bevindt zich in het bestand @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   QByteArray trDeprecated() {
      return "Verouderd";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Collaboratie diagram voor " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Include afhankelijkheidsgraaf voor " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   QByteArray trConstructorDocumentation() {
      return "Constructor & Destructor Documentatie";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   QByteArray trGotoSourceCode() {
      return "Ga naar de broncode van dit bestand.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   QByteArray trGotoDocumentation() {
      return "Ga naar de documentatie van dit bestand.";
   }
   /*! Text for the \\pre command */
   QByteArray trPrecondition() {
      return "Preconditie";
   }
   /*! Text for the \\post command */
   QByteArray trPostcondition() {
      return "Postconditie";
   }
   /*! Text for the \\invariant command */
   QByteArray trInvariant() {
      return "Invariant";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   QByteArray trInitialValue() {
      return "Initi&euml;le waarde:";
   }
   /*! Text used the source code in the file index */
   QByteArray trCode() {
      return "code";
   }
   QByteArray trGraphicalHierarchy() {
      return "Grafische Klasse Hi&euml;rarchie";
   }
   QByteArray trGotoGraphicalHierarchy() {
      return "Ga naar de grafische klasse hi&euml;rarchie";
   }
   QByteArray trGotoTextualHierarchy() {
      return "Ga naar de tekstuele klasse hi&euml;rarchie";
   }
   QByteArray trPageIndex() {
      return "Pagina Index";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   QByteArray trNote() {
      return "Noot";
   }
   QByteArray trPublicTypes() {
      return "Public Typen";
   }
   QByteArray trPublicAttribs() {
      return "Public Attributen";
   }
   QByteArray trStaticPublicAttribs() {
      return "Static Public Attributen";
   }
   QByteArray trProtectedTypes() {
      return "Protected Typen";
   }
   QByteArray trProtectedAttribs() {
      return "Protected Attributen";
   }
   QByteArray trStaticProtectedAttribs() {
      return "Static Protected Attributen";
   }
   QByteArray trPrivateTypes() {
      return "Private Typen";
   }
   QByteArray trPrivateAttribs() {
      return "Private Attributen";
   }
   QByteArray trStaticPrivateAttribs() {
      return "Static Private Attributen";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a todo item */
   QByteArray trTodo() {
      return "Todo";
   }
   /*! Used as the header of the todo list */
   QByteArray trTodoList() {
      return "Todo Lijst";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   QByteArray trReferencedBy() {
      return "Wordt aangeroepen door";
   }
   QByteArray trRemarks() {
      return "Opmerkingen";
   }
   QByteArray trAttention() {
      return "Attentie";
   }
   QByteArray trInclByDepGraph() {
      return "Deze graaf geeft aan welke bestanden direct of "
             "indirect afhankelijk zijn van dit bestand:";
   }
   QByteArray trSince() {
      return "Sinds";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   QByteArray trLegendTitle() {
      return "Graaf Legenda";
   }
   /*! page explaining how the dot graph's should be interpreted */
   QByteArray trLegendDocs() {
      return
         "Deze pagina legt uit hoe de grafen die gegenereerd worden door doxygen "
         "ge&iuml;nterpreteerd dienen te worden.<p>\n"
         "Beschouw het volgende voorbeeld:\n"
         "\\code\n"
         "/*! Onzichtbare klasse vanwege afkappen van de graaf */\n"
         "class Invisible { };\n\n"
         "/*! Afgekapte klasse, overervingsrelatie is verborgen */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Klasse is niet gedocumenteerd met doxygen commentaar */\n"
         "class Undocumented { };\n\n"
         "/*! Klasse met public inheritance */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! A template class */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Klasse met protected inheritance */\n"
         "class ProtectedBase { };\n\n"
         "/*! Klasse met private inheritance */\n"
         "class PrivateBase { };\n\n"
         "/*! Klasse die wordt gebruikt door de klasse Inherited */\n"
         "class Used { };\n\n"
         "/*! Super klasse die overerft van een aantal andere klassen */\n"
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
         "Dit voorbeeld zal resulteren in de volgende graaf:"
         "<p><center><img src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
         "<p>\n"
         "De rechthoeken in the bovenstaande graaf hebben de volgende betekenis:\n"
         "<ul>\n"
         "<li>Een gevulde grijze rechthoek representeert de structure of klasse waarvoor "
         "de graaf is gegenereerd.\n"
         "<li>Een rechthoek met een zwarte rand representeert een gedocumenteerde structure of klasse.\n"
         "<li>Een rechthoek met een grijze rand representeert een ongedocumenteerde structure of klasse.\n"
         "<li>Een rechthoek met een rode rand representeert een gedocumenteerde structure or klasse waarvoor\n"
         "niet alle overervings- of gebruiksrelaties konden worden getoond. Een graaf wordt "
         "afgekapt als hij niet past binnen de gespecificeerde grenzen."
         "</ul>\n"
         "De pijlen hebben de volgende betekenis:\n"
         "<ul>\n"
         "<li>Een donkerblauwe pijl visualizeert een public inheritance "
         "relatie tussen twee klassen.\n"
         "<li>Een donkergroene pijl wordt gebruikt voor protected inheritance.\n"
         "<li>Een donkerrode pijl wordt gebruikt voor private inheritance.\n"
         "<li>Een paars gestippelde pijl wordt gebruikt indien een klasse bevat is of gebruikt wordt "
         "door een andere klasse. De pijl is gelabeled met de variable(n) "
         "die toegang geven tot de aangewezen klasse of structure. \n"
         "<li>Een geel gestippelde pijl wordt gebruikt om een relatie tussen een \n"
         "template instantie en een template klasse aan te geven. De pijl is gelabeld met \n"
         "template parameters van de instantie.\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   QByteArray trLegend() {
      return "legenda";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   QByteArray trTest() {
      return "Test";
   }
   /*! Used as the header of the test list */
   QByteArray trTestList() {
      return "Test Lijst";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   QByteArray trProperties() {
      return "Properties";
   }
   /*! Used as a section header for IDL property documentation */
   QByteArray trPropertyDocumentation() {
      return "Property Documentatie";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   QByteArray trClasses() {
      return "Klassen";
   }
   /*! Used as the title of a Java package */
   QByteArray trPackage(const char *name) {
      return (QByteArray)"Package " + name;
   }
   /*! Title of the package index page */
   QByteArray trPackageList() {
      return "Package Lijst";
   }
   /*! The description of the package index page */
   QByteArray trPackageListDescription() {
      return "Hier volgen de packages, elk met een korte beschrijving (indien aanwezig):";
   }
   /*! The link name in the Quick links header for each page */
   QByteArray trPackages() {
      return "Packages";
   }
   /*! Text shown before a multi-line define */
   QByteArray trDefineValue() {
      return "Waarde:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   QByteArray trBug() {
      return "Bug";
   }
   /*! Used as the header of the bug list */
   QByteArray trBugList() {
      return "Bug Lijst";
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
      QByteArray result((first_capital ? "Klasse" : "klass"));
      if (!singular) {
         result += "n";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Bestand" : "bestand"));
      if (!singular) {
         result += "en";
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
         result += "en";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Pagina" : "pagina"));
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
      QByteArray result((first_capital ? "Member" : "member"));
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
      QByteArray result((first_capital ? "Globale member" : "globale member"));
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
      return "Gebruikt";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Implementeert " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implementation this member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Wordt ge&iuml;mplementeerd door " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trRTFTableOfContents() {
      return "Inhoudsopgave";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Deprecated Lijst";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for a section of events found in a C# program
    */
   virtual QByteArray trEvents() {
      return "Events";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Event Documentatie";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Package Types";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Package Functies";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Statische Package Functies";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Package Attributen";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Statische Package Attributen";
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
      return "Hier is de call graaf voor deze functie:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Zoek Resultaten";
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
         return "Helaas, er zijn geen documenten gevonden die aan de zoekopdracht voldoen.";
      } else if (numDocuments == 1) {
         return "Er is <b>1</b> document gevonden dat aan de zoekopdracht voldoet.";
      } else {
         return "Er zijn <b>$num</b> documenten gevonden die aan de zoekopdracht voldoen. "
                "De beste resultaten worden eerst getoond.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Gevonden:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return filename + " Bron Bestand";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Folder Hi&euml;rarchie";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Folder Documentatie";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Folders";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Deze folder hi&euml;rarchie is min of meer alfabetisch "
             "gesorteerd:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " Folder Referentie";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Folder" : "folder"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.1
   //////////////////////////////////////////////////////////////////////////

   /*! This text is added to the documentation when the \\overload command
    *  is used for a function.
    */
   virtual QByteArray trOverloadText() {
      return "Deze functie is overloaded en is beschikbaar gemaakt om het "
             "gebruik te vergemakkelijken. Ze verschilt alleen van de "
             "bovenstaande functie wat betreft de parameterlijst.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallerGraph() {
      return "Hier is de caller graaf voor deze functie:";
   }

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Member Functie/Subroutine Documentatie";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Lijst met data types";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Data velden";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Hieronder volgen de data types elk een korte beschrijving:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Hieronder volgt de lijst met alle ";
      if (!extractAll) {
         result += "gedocumenteerde ";
      }
      result += "data types met links naar ";
      if (!extractAll) {
         result += "de klasse documentatie voor elke member:";
      } else {
         result += "de klassen waartoe ze behoren:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Data Type Index";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Data Type Documentatie";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Functies/Subroutines";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Functie/Subroutine Documentatie";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Data Types";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Module Lijst";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Hieronder volgt de lijst met alle ";
      if (!extractAll) {
         result += "gedocumenteerde ";
      }
      result += "modulen, elk met een korte beschrijving:";
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
            result += " Type";
            break;
         case ClassDef::Union:
            result += " Union";
            break;
         case ClassDef::Interface:
            result += " Interface";
            break;
         case ClassDef::Protocol:
            result += " Protocol";
            break;
         case ClassDef::Category:
            result += " Category";
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
      result += " Referentie";
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Module Referentie";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Module Members";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Hier is een lijst van alle ";
      if (!extractAll) {
         result += "gedocumenteerde ";
      }
      result += "module members met links naar ";
      if (extractAll) {
         result += "de module documentatie voor iedere member:";
      } else {
         result += "de module waartoe ze behoren:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Module Index";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Module" : "module"));
      if (!singular) {
         result += "n";
      }
      return result;
   }
   /*! This is put at the bottom of a module documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFilesFortran(ClassDef::CompoundType compType,
         bool single) {
      // here s is one of " Class", " Struct" or " Union"
      // single is true implies a single file
      QByteArray result = (QByteArray)"De documentatie voor ";
      switch (compType) {
         case ClassDef::Class:
            result += "deze module";
            break;
         case ClassDef::Struct:
            result += "dit type";
            break;
         case ClassDef::Union:
            result += "deze union";
            break;
         case ClassDef::Interface:
            result += "dit interface";
            break;
         case ClassDef::Protocol:
            result += "dit protocol";
            break;
         case ClassDef::Category:
            result += "deze category";
            break;
         case ClassDef::Exception:
            result += "deze exception";
            break;
         default:
            break;
      }
      result += " is gegenereerd op grond van ";
      if (single) {
         result += "het";
      } else {
         result += "de";
      }
      result += " volgende bestand";
      if (single) {
         result += ":";
      } else {
         result += "en:";
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
      QByteArray result((first_capital ? "Subprogramma" : "subprogramma"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   /*! C# Type Contraint list */
   virtual QByteArray trTypeConstraints() {
      return "Type Beperkingen";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return QByteArray(name) + " Relatie";
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Laden...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Globale Namespace";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Zoeken...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Niets gevonden";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Bestand in " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Includeert bestand in " + name;
   }
  
   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Bibliografie";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Folder afhankelijkheidsgraaf voor ") + name + ":";
   }
 

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "detail niveau";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Template Parameters";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "en " + number + " anderen...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "De documentatie voor deze enum is gegenereerd op grond van ";
      if (single) {
         result += "het";
      } else {
         result += "de";
      }
      result += " volgende bestand";
      if (single) {
         result += ":";
      } else {
         result += "en:";
      }
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return QByteArray(name) + " Enum Referentie";
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " overge&euml;rfd van " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Additionele Overge&euml;rfde Members";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      QByteArray opt = enable ? "in" : "uit";
      return "klik hier de paneel synchronisatie " + opt + " the schakelen";
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "Wordt aangeboden door category @1.";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "Uitbereiding van klasse @1.";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "Klasse Methoden";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "Instantie Methoden";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "Methode Documentatie";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      return "Ontwerp Overzicht";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.4
   //////////////////////////////////////////////////////////////////////////

   /** old style UNO IDL services: implemented interfaces */
   virtual QByteArray trInterfaces() {
      return "Ge&euml;porteerde Interfaces";
   }

   /** old style UNO IDL services: inherited services */
   virtual QByteArray trServices() {
      return "Ge&iuml;ncludeerde Services";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroups() {
      return "Konstanten Groepen";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroupReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Konstanten Groepen Referentie";
      return result;
   }
   /** UNO IDL service page title */
   virtual QByteArray trServiceReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " Service Referentie";
      return result;
   }
   /** UNO IDL singleton page title */
   virtual QByteArray trSingletonReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " Singleton Referentie";
      return result;
   }
   /** UNO IDL service page */
   virtual QByteArray trServiceGeneratedFromFiles(bool single) {
      QByteArray result = "De documentatie voor deze service is gegenereerd op grond van ";
      if (single) {
         result += "het";
      } else {
         result += "de";
      }
      result += " volgende bestand";
      if (single) {
         result += ":";
      } else {
         result += "en:";
      }
      return result;
   }
   /** UNO IDL singleton page */
   virtual QByteArray trSingletonGeneratedFromFiles(bool single) {
      QByteArray result = "De documentatie voor deze singleton is gegenereerd op grond van ";
      if (single) {
         result += "het";
      } else {
         result += "de";
      }
      result += " volgende bestand";
      if (single) {
         result += ":";
      } else {
         result += "en:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
};

#endif
