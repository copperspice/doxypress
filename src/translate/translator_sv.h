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

#ifndef TRANSLATOR_SE_H
#define TRANSLATOR_SE_H

class TranslatorSwedish : public Translator
{
 public:

   virtual QByteArray idLanguage() {
      return "swedish";
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
      return "\\usepackage[swedish]{babel}\n";
   }

   virtual QByteArray trRelatedFunctions() {
      return "Besläktade funktioner";
   }

   virtual QByteArray trRelatedSubscript() {
      return "(Observera att dessa inte är medlemsfunktioner)";
   }

   virtual QByteArray trDetailedDescription() {
      return "Detaljerad beskrivning";
   }

   virtual QByteArray trMemberTypedefDocumentation() {
      return "Dokumentation av typdefinierade medlemmar";
   }

   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Dokumentation av egenuppräknande medlemmar";
   }

   virtual QByteArray trMemberFunctionDocumentation() {
      return "Dokumentation av medlemsfunktioner";
   }

   virtual QByteArray trMemberDataDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Fält dokumentation";
      } else {
         return "Dokumentation av datamedlemmar";
      }
   }

   virtual QByteArray trMore() {
      return "Mer...";
   }

   virtual QByteArray trListOfAllMembers() {
      return "Lista över alla medlemmar";
   }

   virtual QByteArray trMemberList() {
      return "Medlemslista";
   }

   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Det här är en fullständig lista över medlemmar för ";
   }

   virtual QByteArray trIncludingInheritedMembers() {
      return " med alla ärvda medlemmar.";
   }

   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Automatiskt skapad av Doxygen";
      if (s) {
         result += (QByteArray)" för " + s;
      }
      result += " från källkoden.";
      return result;
   }

   virtual QByteArray trEnumName() {
      return "enum namn";
   }

   virtual QByteArray trEnumValue() {
      return "enum värde";
   }

   virtual QByteArray trDefinedIn() {
      return "definierad i";
   }

   /*
       QByteArray trVerbatimText(const char *f)
       { return (QByteArray)"Detta är den ordagranna texten från inkluderingsfilen "+f; }
   */
   virtual QByteArray trModules() {
      return "Moduler";
   }

   virtual QByteArray trClassHierarchy() {
      return "Klasshierarki";
   }

   virtual QByteArray trCompoundList() {
      if (Config::getBool("optimize-c")) {
         return "Datastrukturer";
      } else {
         return "Sammansatt klasslista";
      }
   }

   virtual QByteArray trFileList() {
      return "Fillista";
   }

   virtual QByteArray trCompoundMembers() {
      if (Config::getBool("optimize-c")) {
         return "Data fält";
      } else {
         return "Sammansatta klassmedlemmar";
      }
   }

   virtual QByteArray trFileMembers() {
      if (Config::getBool("optimize-c")) {
         return "Globala symboler";
      } else {
         return "Filmedlemmar";
      }
   }

   virtual QByteArray trRelatedPages() {
      return "Besläktade sidor";
   }

   virtual QByteArray trExamples() {
      return "Exempel";
   }

   virtual QByteArray trSearch() {
      return "Sök";
   }

   virtual QByteArray trClassHierarchyDescription() {
      return "Denna lista över arv är grovt, men inte helt, "
             "sorterad i alfabetisk ordning:";
   }

   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Här följer en lista över alla ";
      if (!extractAll) {
         result += "dokumenterade ";
      }
      result += "filer, med en kort beskrivning:";
      return result;
   }

   virtual QByteArray trCompoundListDescription() {

      if (Config::getBool("optimize-c")) {
         return "Här följer datastrukturerna med korta beskrivningar:";
      } else {
         return "Här följer klasserna, strukterna, unionerna och "
                "gränssnitten med korta beskrivningar:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Här är en lista över alla ";
      if (!extractAll) {
         result += "dokumenterade ";
      }
      if (Config::getBool("optimize-c")) {
         result += "struktur- och unions-fält";
      } else {
         result += "klassmedlemmar";
      }
      result += " med länkar till ";
      if (!extractAll) {
         if (Config::getBool("optimize-c")) {
            result += "struktur/unions-dokumentationen för varje fält:";
         } else {
            result += "klassdokumentationen för varje medlem:";
         }
      } else {
         if (Config::getBool("optimize-c")) {
            result += " strukturerna/unionerna de tillhör:";
         } else {
            result += "klasserna de tillhör:";
         }
      }
      return result;
   }

   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Här följer en lista över alla ";
      if (!extractAll) {
         result += "dokumenterade ";
      }

      if (Config::getBool("optimize-c")) {
         result += "funktioner, variabler, definitioner, enumerationer "
                   "och typdefinitioner";
      } else {
         result += "filmedlemmar";
      }
      result += " med länkar till ";
      if (extractAll) {
         result += "filerna som de tillhör:";
      } else {
         result += "dokumentationen:";
      }
      return result;
   }

   virtual QByteArray trExamplesDescription() {
      return "Här följer en lista med alla exempel:";
   }

   virtual QByteArray trRelatedPagesDescription() {
      return "Här följer en lista över alla besläktade dokumentationssidor:";
   }

   virtual QByteArray trModulesDescription() {
      return "Här följer en lista över alla moduler:";
   }

   virtual QByteArray trDocumentation() {
      return "Dokumentation";
   }

   virtual QByteArray trModuleIndex() {
      return "Modulindex";
   }

   virtual QByteArray trHierarchicalIndex() {
      return "Hierarkiskt Index";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config::getBool("optimize-c")) {
         return "Index över datastrukturer";
      } else {
         return "Index över sammensatta typer";
      }
   }

   virtual QByteArray trFileIndex() {
      return "Filindex";
   }

   virtual QByteArray trModuleDocumentation() {
      return "Dokumentation över moduler";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Dokumentation över datastrukturer";
      } else {
         return "Documentation över klasser";
      }
   }

   virtual QByteArray trFileDocumentation() {
      return "Dokumentation över filer";
   }

   virtual QByteArray trExampleDocumentation() {
      return "Dokumentation över exempel";
   }

   virtual QByteArray trPageDocumentation() {
      return "Dokumentation av sidor";
   }

   virtual QByteArray trReferenceManual() {
      return "Referensmanual";
   }

   virtual QByteArray trDefines() {
      return "Definitioner";
   }
   virtual QByteArray trTypedefs() {
      return "Typdefinitioner";
   }
   virtual QByteArray trEnumerations() {
      return "Egenuppräknande typer";
   }
   virtual QByteArray trFunctions() {
      return "Funktioner";
   }
   virtual QByteArray trVariables() {
      return "Variabler";
   }

   virtual QByteArray trEnumerationValues() {
      return "Egenuppräknade typers värden";
   }

   virtual QByteArray trDefineDocumentation() {
      return "Dokumentation över definitioner";
   }

   virtual QByteArray trTypedefDocumentation() {
      return "Dokumentation över typdefinitioner";
   }

   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Dokumentation över egenuppräknande typer"; ;
   }

   virtual QByteArray trFunctionDocumentation() {
      return "Dokumentation över funktioner";
   }

   virtual QByteArray trVariableDocumentation() {
      return "Dokumentation över variabler";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config::getBool("optimize-c")) {
         return "Datastrukturer";
      } else {
         return "Sammansättning";
      }
   }

   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Skapad " + date;
      if (projName) {
         result += (QByteArray)" för " + projName;
      }
      result += (QByteArray)" av";
      return result;
   }

   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Klassdiagram för " + clName;
   }

   virtual QByteArray trForInternalUseOnly() {
      return "Endast för internt bruk.";
   }

   virtual QByteArray trWarning() {
      return "Varning";
   }

   virtual QByteArray trVersion() {
      return "Version";
   }

   virtual QByteArray trDate() {
      return "Datum";
   }

   virtual QByteArray trReturns() {
      return "Returnerar";
   }

   virtual QByteArray trSeeAlso() {
      return "Se även";
   }

   virtual QByteArray trParameters() {
      return "Parametrar";
   }

   virtual QByteArray trExceptions() {
      return "Undantag";
   }

   virtual QByteArray trGeneratedBy() {
      return "Skapad av";
   }

   // new since 0.49-990307

   virtual QByteArray trNamespaceList() {
      return "Namnrymdslista";
   }

   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Här är en lista över alla ";
      if (!extractAll) {
         result += "dokumenterade ";
      }
      result += "namnrymder med en kort beskrivning:";
      return result;
   }

   virtual QByteArray trFriends() {
      return "Vänner";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Vänners och besläktade funktioners dokumentation";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result = (QByteArray)clName + " ";
      switch (compType) {
         case ClassDef::Class:
            result += " klass";
            break;
         case ClassDef::Struct:
            result += " strukt";
            break;
         case ClassDef::Union:
            result += " union";
            break;
         case ClassDef::Interface:
            result += " gränssnitt";
            break;
         case ClassDef::Protocol:
            result += " protocol";
            break; // translate me!
         case ClassDef::Category:
            result += " category";
            break; // translate me!
         case ClassDef::Exception:
            result += " undantag";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += "template";
      }
      result += "referens";
      return result;
   }

   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = fileName;
      result += " filreferens";
      return result;
   }

   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " namnrymdreferens";
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Publika medlemmar";
   }
   virtual QByteArray trPublicSlots() {
      return "Publika slots";
   }
   virtual QByteArray trSignals() {
      return "Signaler";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Statiska  publika medlemmar";
   }
   virtual QByteArray trProtectedMembers() {
      return "Skyddade medlemmar";
   }
   virtual QByteArray trProtectedSlots() {
      return "Skyddade slots";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Statiska skyddade medlemmar";
   }
   virtual QByteArray trPrivateMembers() {
      return "Privata medlemmar";
   }
   virtual QByteArray trPrivateSlots() {
      return "Privata slots";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Statiska privata medlemmar";
   }
   // end of member sections

   virtual QByteArray trWriteList(int numEntries) {
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
               result += ", och ";
            }
         }
      }
      return result;
   }

   virtual QByteArray trInheritsList(int numEntries)
   // used in class documentation to produce a list of base classes,
   // if class diagrams are disabled.
   {
      return "Ärver " + trWriteList(numEntries) + ".";
   }
   virtual QByteArray trInheritedByList(int numEntries)
   // used in class documentation to produce a list of super classes,
   // if class diagrams are disabled.
   {
      return "Ärvd av " + trWriteList(numEntries) + ".";
   }
   virtual QByteArray trReimplementedFromList(int numEntries)
   // used in member documentation blocks to produce a list of
   // members that are hidden by this one.
   {
      return "Återskapad från " + trWriteList(numEntries) + ".";
   }
   virtual QByteArray trReimplementedInList(int numEntries) {
      // used in member documentation blocks to produce a list of
      // all member that overwrite the implementation of this member.
      return "Återskapad i " + trWriteList(numEntries) + ".";
   }

   virtual QByteArray trNamespaceMembers() {
      return "Namnrymdsmedlemmar";
   }
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Här är en lista över alla ";
      if (!extractAll) {
         result += "dokumenterade ";
      }
      result += "namnrymdsmedlemmar med länkar till ";
      if (extractAll) {
         result += " namnrymd-dokumentationen för varje medlem:";
      } else {
         result += "de namnrymder de tillhör:";
      }
      return result;
   }

   virtual QByteArray trNamespaceIndex() {
      return "Namnrymdsindex";
   }

   virtual QByteArray trNamespaceDocumentation() {
      return "Namnrymd-dokumentation";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Namnrymder";
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
      QByteArray result = (QByteArray)"Dokumentationen för ";
      switch (compType) {
         case ClassDef::Class:
            result += "denna klass ";
            break;
         case ClassDef::Struct:
            result += "denna strukt ";
            break;
         case ClassDef::Union:
            result += "denna union ";
            break;
         case ClassDef::Interface:
            result += "detta gränssnitt ";
            break;
         case ClassDef::Protocol:
            result += "protocol";
            break; // translate me!
         case ClassDef::Category:
            result += "category";
            break; // translate me!
         case ClassDef::Exception:
            result += "detta undantag ";
            break;
         default:
            break;
      }
      result += "var genererad från följande fil";
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
      return "Returvärden";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Huvudsida";
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

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Definition på rad @0 i filen @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Definition i filen @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Föråldrad";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Samarbetsdiagram för " + clName + ":";
   }

   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Include-beroendediagram för " + fName + ":";
   }

   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Dokumentation av konstruktorer och destruktorer";
   }

   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Gå till denna fils källkod.";
   }

   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Gå till denna fils dokumentation.";
   }

   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Förhandsvillkor";
   }

   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Resultat"; //"Postcondition";
   }

   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Invariant";
   }

   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Begynnelsevärde:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "källkod";
   }

   virtual QByteArray trGraphicalHierarchy() {
      return "Grafisk klasshierarki";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Gå till den grafiska klasshierarkin";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Gå till den textbaserade klasshierarkin";
   }
   virtual QByteArray trPageIndex() {
      return "Sidindex";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Notera";
   }
   virtual QByteArray trPublicTypes() {
      return "Publika typer";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config::getBool("optimize-c")) {
         return "Datafält";
      } else {
         return "Publika attribut";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Statiska publika attribut";
   }
   virtual QByteArray trProtectedTypes() {
      return "Skyddade typer";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Skyddade attribut";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Statiska skyddade attribut";
   }
   virtual QByteArray trPrivateTypes() {
      return "Privata typer";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Privata attribut";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Statiska privata attribut";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "Att-göra";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Att-göra lista";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Refererad av";
   }
   virtual QByteArray trRemarks() {
      return "Lägg märke till";
   }
   virtual QByteArray trAttention() {
      return "Observera";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Den här grafen visar vilka filer som direkt eller "
             "indirekt inkluderar denna filen.";
   }
   virtual QByteArray trSince() {
      return "Sedan";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Grafförklaring";
   }

   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Den här sidan förklarar hur man ska tolka de grafer som doxygen "
         "genererar.<p>\n"
         "Tag följande exempel:\n"
         "\\code\n"
         "/*! Osynlig klass på grund av stympning */\n"
         "class Invisible { };\n\n"
         "/*! Stympad klass, ärvningsrelationen är dold */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Klass utan doxygen-kommentarer */\n"
         "class Undocumented { };\n\n"
         "/*! Klass som ärvs med publikt arv */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! En template-klass */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Klass som ärvs med skyddat arv */\n"
         "class ProtectedBase { };\n\n"
         "/*! Klass som ärvs med privat arv */\n"
         "class PrivateBase { };\n\n"
         "/*! Klass som används av Inherited klassen */\n"
         "class Used { };\n\n"
         "/*! Super klassen som ärver ett antal andra klasser */\n"
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
         "Om \\c MAX_DOT_GRAPH_HEIGHT är satt till 240 i konfigurationsfilen, "
         "kommer följande graf att generas:"
         "<p><center><img alt=\"\" src=\"graph_legend." + Config::getEnum("dot-image-format").toUtf8() + "\"></center>\n"
         "<p>\n"
         "Rektanglarna i den ovanstående grafen har följande betydelser:\n"
         "<ul>\n"
         "<li>%En fylld svart rektangel representerar den strukt eller klass "
         "som har genererat grafen.\n"
         "<li>%En rektangel med svart kant symboliserar en dokumenterad "
         "strukt eller klass.\n"
         "<li>%En rektangel med grå kant symboliserar en odokumenterad strukt "
         "eller klass.\n"
         "<li>%En klass med röd kant symboliserar en strukt eller klass där "
         "alla dess relationer inte visas. %En graf stympas om den inte får "
         "plats inom de givna gränserna.\n"
         "</ul>\n"
         "Pilarna har följande betydelser:\n"
         "<ul>\n"
         "<li>%En mörkblå pil används för att visualisera en publik arvrelation "
         "mellan två klasser.\n"
         "<li>%En mörkgrön pil används för en skyddad arvsrelation.\n"
         "<li>%En mörkröd pil används för en privat arvsrelation.\n"
         "<li>%En sträckad lila pil används om en klass är innesluten eller "
         "använd av en annan klass. Vid pilen står namnet på den eller de "
         "variabler som klassen pilen pekar på kommer åt.\n"
         "<li>%En sträckad gul pil symboliserar förhållandet mellan en "
         "template-instans och template-klassen, som den instantierades från.\n"
         "Vid pilen står instansens template-parametrar.\n"
         "</ul>\n";
   }

   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "förklaring";
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
      return "Testlista";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Egenskaper";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Egenskapsdokumentation";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

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
      return (QByteArray)"Paket " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Paketlista";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Här är en lista över paketen med en kort beskrivning "
             "(om sådan finns):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Paket";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Värde:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Bugg";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Bugglista";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.6
   //////////////////////////////////////////////////////////////////////////

   /*! Used as ansicpg for RTF file
   * (used table extract:)
   * <pre>
    * Charset Name       Charset Value(hex)  Codepage number
    * ------------------------------------------------------
    * ANSI_CHARSET              0 (x00)            1252
   * </pre>
   */
   virtual QByteArray trRTFansicp() {
      return "1252";
   }

   /*! Used as ansicpg for RTF fcharset */
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
      QByteArray result((first_capital ? "Klass" : "klass"));
      if (!singular) {
         result += "er";
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
      QByteArray result((first_capital ? "Namnrymd" : "namnrymd"));
      if (!singular) {
         result += "er";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Grupp" : "grupp"));
      if (!singular) {
         result += "er";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Sid" : "sid"));
      if (singular) {
         result += "a";
      } else {
         result += "or";
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
         result += "mar";
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
         result += "er";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool /*singular*/) {
      QByteArray result((first_capital ? "Författare" : "författare"));
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Referenser";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Implementerar " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Implementerad i " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Innehållsförteckning";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Lista över föråldrade";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Händelser";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Händelse Dokumentation";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Paket typer";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Paket funktioner";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Statiska paket funktioner";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Paket attribut";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Statiska paket attribut";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Alla";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Här är anropnings diagrammet för den här funktionen:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Sökresultat";
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
         return "Tyvärr finns det inga dokument som matchar din sökning.";
      } else if (numDocuments == 1) {
         return "Hittade <b>1</b> dokument som matchar din sökning.";
      } else {
         return "Hittade <b>$num</b> dokument som matchar din sökning. "
                "Visar de bästa träffarna först.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Träffar:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return "Källkodsfilen " + filename;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Katalogstruktur";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Katalogdokumentation";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Kataloger";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Den här katalogen är grovt sorterad, "
             "men inte helt, i alfabetisk ordning:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " Katalogreferens";
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
      return "Det här är en överlagrad medlemsfunktion "
             "tillhandahållen för bekvämlighet. Den enda som "
             "skiljer sig från ovanstående funktion är vilka "
             "argument den tar emot.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Här är katalog-grafen för denna funktion:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Uppräknings-dokumentation";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Memlems-function/Subroutins Dokumentation";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Datatyplista";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Data fält";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Här är datatyperna med en kort beskrivning:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Här är en lista av alla ";
      if (!extractAll) {
         result += "dokumenterade ";
      }
      result += "datatyps medlemmar";
      result += " med länkar till ";
      if (!extractAll) {
         result += "datastrukturs documentation för varje medlem";
      } else {
         result += "klasserna de hör till:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Datatyps Index";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Dataryps Dokumentation";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Funktions/Subroutins";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Funktion/Subroutin Dokumentation";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Datatyper";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Modullista";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Här är en lista på alla ";
      if (!extractAll) {
         result += "dokumenterade ";
      }
      result += "moduler med en kort beskrivning:";
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
            result += " Typ";
            break;
         case ClassDef::Union:
            result += " Union";
            break;
         case ClassDef::Interface:
            result += " Gränssnitt";
            break;
         case ClassDef::Protocol:
            result += " Protokoll";
            break;
         case ClassDef::Category:
            result += " Kategori";
            break;
         case ClassDef::Exception:
            result += " Undantag";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += " Mall";
      }
      result += " Referens";
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Modul Referens";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Modul Medlemmar";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Här är en lista på alla ";
      if (!extractAll) {
         result += "documented ";
      }
      result += "modul medlemmar med länkar till ";
      if (extractAll) {
         result += "modul dokumentationen för varje medlem:";
      } else {
         result += "modulerna de hör till:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Modul Index";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      return createNoun(first_capital, singular, "modul", "er");
   }
   /*! This is put at the bottom of a module documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFilesFortran(ClassDef::CompoundType compType,
         bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"Dokumentationen för denna ";
      switch (compType) {
         case ClassDef::Class:
            result += "modul";
            break;
         case ClassDef::Struct:
            result += "typ";
            break;
         case ClassDef::Union:
            result += "union";
            break;
         case ClassDef::Interface:
            result += "gränssnitt";
            break;
         case ClassDef::Protocol:
            result += "protokoll";
            break;
         case ClassDef::Category:
            result += "kategori";
            break;
         case ClassDef::Exception:
            result += "undantag";
            break;
         default:
            break;
      }
      result += " genererades från följade fil";
      if (single) {
         result += ":";
      } else {
         result += "er:";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool first_capital, bool singular) {
      return createNoun(first_capital, singular, "typ", "er");
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool /*singular*/) {
      QByteArray result((first_capital ? "Subprogram" : "subprogram"));
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Typbegränsningar";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return QByteArray(name) + " Relation";
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Laddar...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Globalnamnrymd";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Söker...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Inga träffar";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Fil i " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Inkluderar fil i " + name;
   }

   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Bibliografiska Referenser";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Katalogberoendegraf för ") + name + ":";
   }
   
   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "detaljnivå";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Mall Parametrar";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "och " + number + " mera...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "Dokumentationen för denna enum var genererad från föjlande fil";
      if (!single) {
         result += "er";
      }
      result += ":";
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return QByteArray(name) + " Enum Referens";
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " ärvd ifrån " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Ytterliga Ärvda Medlemmar";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      QByteArray opt = enable ? "aktivera" : "inaktivera";
      return "klicka för att " + opt + " panel synkronisering";
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "Tillhandahålls av kategori @1.";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "Utökar klass @1.";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "Klassmetoder";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "Instansmetoder";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "Metoddokumentation";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      return "Designöversikt";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.4
   //////////////////////////////////////////////////////////////////////////

   /** old style UNO IDL services: implemented interfaces */
   virtual QByteArray trInterfaces() {
      return "Exporterade Interface";
   }

   /** old style UNO IDL services: inherited services */
   virtual QByteArray trServices() {
      return "Inkuderade Tjänster";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroups() {
      return "Konstant Grupper";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroupReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Konstant Grupp Referens";
      return result;
   }
   /** UNO IDL service page title */
   virtual QByteArray trServiceReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " Tjänstereferens";
      return result;
   }
   /** UNO IDL singleton page title */
   virtual QByteArray trSingletonReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " Singleton Referens";
      return result;
   }
   /** UNO IDL service page */
   virtual QByteArray trServiceGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"Dokumentationen för denna tjänst "
                          "genererades från följande fil";
      if (single) {
         result += ":";
      } else {
         result += "er:";
      }
      return result;
   }
   /** UNO IDL singleton page */
   virtual QByteArray trSingletonGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"Dokumentationen för denna singleton"
                          "genererades från följande fil";
      if (single) {
         result += ":";
      } else {
         result += "er:";
      }
      return result;
   }

   /*---------- For internal use: ----------------------------------------*/
 protected:
   /*! For easy flexible-noun implementation.
    *  \internal
    */
   QByteArray createNoun(bool first_capital, bool singular,
                         const char *base, const char *plurSuffix) {
      QByteArray result(base);
      if (first_capital) {
         result.at(0) = toupper(result.at(0));
      }
      if (!singular) {
         result += plurSuffix;
      }
      return result;
   }


};

#endif
