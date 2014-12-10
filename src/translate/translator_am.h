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

/*
 * Translation by
 * Armen Tangamyan <armen.tangamyan@anu.edu.au>
 */

#ifndef TRANSLATOR_AM_H
#define TRANSLATOR_AM_H

class TranslatorArmenian : public TranslatorAdapter_1_8_0
{
 public:
   /*! Used for identification of the language. */
   virtual QByteArray idLanguage() {
      return "armenian";
   }

   /* Used to get the command(s) for the language support. */
   virtual QByteArray latexLanguageSupportCommand() {
      return "<pre>\\usepackage[latin]{armtex}\n"
             "\\usepackage[armscii8]{inputenc}\n</pre>";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Դասին վերաբերվող ֆունկցիաներ";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Հաշվի առեք, որ սրանք անդամ ֆունկցիաներ չեն)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Մանրամասն նկարագրություն";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Անդամ տիպի սահմանումներ (typedef)";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Անդամ hամարակալումներ";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Անդամ ֆունկցիաներ";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Դաշտեր";
      } else {
         return "Անդամ տվյալներ";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Մանրամասն...";
   }

   /*! put in the class documentation */
   /* Dosn't use when optimization for C is on. */
   virtual QByteArray trListOfAllMembers() {
      return "Բոլոր անդամների ցուցակը";
   }

   /*! used as the title of the "list of all members" page of a class */
   /* Dosn't use when optimization for C is on. */
   virtual QByteArray trMemberList() {
      return "Անդամների ցուցակ";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   /* Dosn't use when optimization for C is on. */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Սա դասի անդամների ամբողջական ցուցակն է ";
   }

   /*! this is the remainder of the sentence after the class name */
   /* Dosn't use when optimization for C is on. */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", ներառյալ բոլոր ժառանգված անդամները";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Ավտոմատ ստեղծված է ելքային կոդից, Doxygen-ի միջոցով, ";
      if (s) {
         result += s + (QByteArray)" համար:";
      }
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "համարակալման անուն";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "համարակալման արժեք";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "սահմանված";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Մոդուլներ";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Դասերի հիերարխա";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Տվյալների կառուցվածք";
      } else {
         return "Դասերի ցուցակ";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Ֆայլերի ցուցակ";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Տվյալների դաշտեր";
      } else {
         return "Դասի անդամներ";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   /*??*/
   virtual QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Գլոբալներ";
      } else {
         return "Ֆայլի անդամներ";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Նմանատիպ էջեր";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Օրինակներ";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Որոնում";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Այս ժառանգման ցուցակը կոպտորեն է տեսակավորված, "
             "բայց ոչ ամբողջապես, այբբենական կարգով.";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Բոլոր ";
      if (!extractAll) {
         result += "փաստագրված ";
      }
      result += "ֆայլերի մի ցուցակ` կարճ բացատրություններով:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Տվյալների կառուցվածքը` կարճ բացատրություններով.";
      } else {
         return "Դասերը, կառուցվածքները, միավորումները "
                "և ինտերֆեյսները` կարճ բացատրություններով.";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Բոլոր ";
      if (!extractAll) {
         result += "փաստագրված ";
      }
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "կառուցվածքների և միավորումների դաշտերի ";
      } else {
         result += "դասի անդամների ";
      }
      result += "ցուցակը`";
      result += " հղումներով դեպի ";
      if (!extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "կառուցվածք/միավորում փաստագրությունները բոլոր անդամների համար.";
         } else {
            result += "դասի փաստագրությունը բոլոր անդամների համար.";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "կառուցվածքները/միավորումները, որոնց նրանք պատկանում են.";
         } else {
            result += "դասերը, որոնց նրանք պատկանում են.";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Բոլոր ";
      if (!extractAll) {
         result += "փաստագրված ";
      }

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "ֆունկցիաների, փոփոխականների, մակրո-հայտարարությունների, "
                   "համարակալումների և տիպի սահմանումների (typedef)";
      } else {
         result += "ֆայլի անդամների ";
      }
      result += "ցուցակը`";
      result += " հղումներով դեպի ";
      if (extractAll) {
         result += "ֆայլերը, որոնց նրանք պատկանում են.";
      } else {
         result += "փաստագրությունը.";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Բոլոր օրինակների ցուցակը.";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Բոլոր նմանատիպ փաստագրության էջերի ցուցակը.";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Բոլոր մոդուլների ցուցակը.";
   }

   // index titles (the project name is prepended for these)


   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return " - Փաստագրություն";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Մոդուլներ";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Դասակարգումներ";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Տվյալների կառուցվածք";
      } else {
         return "Դասեր";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Ֆայլեր";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Մոդուլներ";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Տվյալների կառուցվածքներ";
      } else {
         return "Դասեր";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Ֆայլեր";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Օրինակներ";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Էջեր";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Հղումների ձեռնարկ";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Մակրոսներ";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Տիպի սահմանումներ (typedef)";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Համարակալումներ";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Ֆունկցիաներ";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Փոփոխականներ";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Հաշվիչ";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Մակրոսներ";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Տիպի սահմանումներ (typedef)";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Համարակալման տիպեր";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Ֆունկցիաներ";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Փոփոխականներ";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Տվյալների կառուցվածք";
      } else {
         return "Դասեր";
      }
   }

   /*! This is used in the documentation of a group before the list of
    *  links to documented files
    */
   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Ստեղծվել է " + date;
      if (projName) {
         result += projName + QByteArray(" -ի համար,");
      }
      result += (QByteArray)" հետևյալ համակարգով.";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return clName + QByteArray(" -ի ժառանգման գծագիրը.");
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Միայն ներքին օգտագործման համար";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Զգուշացում";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Տարբերակ";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Տարեթիվ";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Վերադարձնում է";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Տեսեք նաև";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Պարամետրեր";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Բացառություններ";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Ստեղծված է հետևյալ համակարգի կողմից";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Անունների տարածությունների ցուցակ";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Բոլոր ";
      if (!extractAll) {
         result += "փաստագրված ";
      }
      result += "անունների տարածությունների ցուցակը` կարճ բացատրություններով.";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Ընկերներ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Դասի ընկերներ և կապված ֆունկցիաներ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      if (isTemplate) {
         switch (compType) {
            case ClassDef::Class:
               result += " Դասի";
               break;
            case ClassDef::Struct:
               result += " Կառուցվածքի";
               break;
            case ClassDef::Union:
               result += " Միավորման";
               break;
            case ClassDef::Interface:
               result += " Ինտերֆեյսի";
               break;
            case ClassDef::Protocol:
               result += " Արձանագրության";
               break;
            case ClassDef::Category:
               result += " Դասակարգման";
               break;
            case ClassDef::Exception:
               result += " Բացառության";
               break;
            default:
               break;
         }
         result += " Ձևանմուշներ";
      } else {
         switch (compType) {
            case ClassDef::Class:
               result += " Դաս";
               break;
            case ClassDef::Struct:
               result += " Կառուցվածք";
               break;
            case ClassDef::Union:
               result += " Միավորում";
               break;
            case ClassDef::Interface:
               result += " Ինտերֆեյս";
               break;
            case ClassDef::Protocol:
               result += " Արձանագրություն";
               break;
            case ClassDef::Category:
               result += " Դասակարգում";
               break;
            case ClassDef::Exception:
               result += " Բացառություն";
               break;
            default:
               break;
         }
      }
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      return fileName + QByteArray(" ֆայլեր");
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " անունների տարածություններ";
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Բաց անդամ ֆունկցիաներ";
   }
   virtual QByteArray trPublicSlots() {
      return "Բաց սլոթեր";
   }
   virtual QByteArray trSignals() {
      return "Ազդանշաններ";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Բաց ստատիկ անդամ ֆունկցիաներ";
   }
   virtual QByteArray trProtectedMembers() {
      return "Պաշտպանված անդամ ֆունկցիաներ";
   }
   virtual QByteArray trProtectedSlots() {
      return "Պաշտպանված սլոթեր";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Պաշտպանված ստատիկ անդամ ֆունկցիաներ";
   }
   virtual QByteArray trPrivateMembers() {
      return "Փակ ֆունկցիաներ";
   }
   virtual QByteArray trPrivateSlots() {
      return "Փակ սլոթեր";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Փակ ստատիկ անդամ ֆունկցիաներ";
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
               result += " և ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Հենքային դասեր - " + trWriteList(numEntries) + ":";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Ժառանգորդ դասեր - " + trWriteList(numEntries) + ":";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Վերասահմանված ֆունկցիաներ - " + trWriteList(numEntries) + ":";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Վերասահմանված է " + trWriteList(numEntries) + " ում:";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Անունների տարածության անդամներ";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Բոլոր ";
      if (!extractAll) {
         result += "փաստագրված ";
      }
      result += "անունների տարածության անդամների ցուցակը` "
                "հղումներով դեպի ";
      if (extractAll) {
         result += "բոլոր անդամների անունների տարածության փաստագրությունը.";
      } else {
         result += "անունների տարածությունը, որին նրանք պատկանում են.";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Անունների տարածություններ";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Անունների տարածություն";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Անունների տարածություններ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990728
   //////////////////////////////////////////////////////////////////////////

   /*! This is put at the bottom of a class documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFiles(ClassDef::CompoundType compType,
                                           bool single) {
      QByteArray result = (QByteArray)"Այս ";
      switch (compType) {
         case ClassDef::Class:
            result += "դասի";
            break;
         case ClassDef::Struct:
            result += "կառուցվածքի";
            break;
         case ClassDef::Union:
            result += "միավորման";
            break;
         case ClassDef::Interface:
            result += "ինտերֆեյսի";
            break;
         case ClassDef::Protocol:
            result += "արձանագրության";
            break;
         case ClassDef::Category:
            result += "դասակարգման";
            break;
         case ClassDef::Exception:
            result += "բացառության";
            break;
         default:
            break;
      }
      result += " փաստագրությունը ստեղծվել է հետևյալ ֆայլ";
      if (single) {
         result += "ից.";
      } else {
         result += "երից.";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Վերադարձվող արժեքներ";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Գլխավոր էջ";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "էջ:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991106
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Սահմանումը @1 ֆայլի @0 տողում է:";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Սահմանումը @0 ֆայլում է:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Հնացած է";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)clName + "-ի համագործակցությունների գծագիր.";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)fName + "-ի ներառումների կախվածությունների գծագիր.";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Կառուցիչներ";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Տե'ս այս ֆայլի ելքային կոդը";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Տե'ս այս ֆայլի փաստագրությունը:";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Նախապայման";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Հետպայման";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Անփոփոխ";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Նախնական արժեք";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "Ելքային կոդ";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Գրաֆիկական դասերի հիերարխիա:";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Տե'ս դասերի գրաֆիկական հիերարխիան:";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Տե'ս դասերի տեքստային հիերարխիան:";
   }
   virtual QByteArray trPageIndex() {
      return "էջեր";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Նշում";
   }
   virtual QByteArray trPublicTypes() {
      return "Բաց տիպեր";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Տվյալների դաշտեր";
      } else {
         return "Բաց ատրիբուտներ";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Բաց ստատիկ ատրիբուտներ";
   }
   virtual QByteArray trProtectedTypes() {
      return "Պաշտպանված տիպեր";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Պաշտպանված ատրիբուտներ";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Պաշտպանված ստատիկ ատրիբուտներ";
   }
   virtual QByteArray trPrivateTypes() {
      return "Փակ տիպեր";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Փակ ատրիբուտներ";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Փակ ստատիկ ատրիբուտներ";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a todo item */
   virtual QByteArray trTodo()
   /*??*/
   {
      return "Կատարման ենթակա";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList()
   /*??*/
   {
      return "Խնդիրների ցուցակ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Օգտագործվում է հետևյալում - ";
   }
   virtual QByteArray trRemarks() {
      return "Դիտողություններ";
   }
   virtual QByteArray trAttention() {
      return "Ուշադրություն";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Այս գրաֆը ցույց է տալիս, թե որ ֆայլերն են "
             "ուղղակի կամ անուղղակի ներառում տվյալ ֆայլը.";
   }
   virtual QByteArray trSince()
   /*??*/
   {
      return "Սկսած";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Լեգենդ";
   }
   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Այս էջը նկարագրում է, թե ինչպես մեկնաբանել doxygen-ի ստեղծած գրաֆները:<p>\n"
         "Դիտարկենք հետևյալ օրինակը.\n"
         "\\code\n"
         "/*! Կրճատման հետևանքով անտեսանելի դաս */\n"
         "class Invisible { };\n\n"
         "/*! Կրճատված դաս, ժառանգությունների հարաբերությունը փակ է */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Չփաստագրված դաս */\n"
         "class Undocumented { };\n\n"
         "/*! Բաց ժառանգում */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! Դասի ձևաչափ */\n"
         "template<class T> class Templ {};\n\n"
         "/*! Պաշտպանված ժառանգում */\n"
         "class ProtectedBase { };\n\n"
         "/*! Փակ ժառանգում */\n"
         "class PrivateBase { };\n\n"
         "/*! Դաս, որը օգտագործվում է Inherited դասի կողմից */\n"
         "class Used { };\n\n"
         "/*! Դաս, որը ժառանգում է մի շարք այլ դասերից */\n"
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
         "Սրանով կստանանք հետևյալ գրաֆը."
         "<p><center><img src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
         "<p>\n"
         "Այս գրաֆի ուղղանկյունները ունեն հետևյալ իմաստը.\n"
         "<ul>\n"
         "<li>%A լցոնվաց մոխրագույն ուղղանկյունը ներկայացնում է այն դասը կամ կառուցվածքը, "
         "որի համար ստեղծվել է տվյալ գրաֆը:</li>\n"
         "<li>%A սև եզրերով ուղղանկյունը նշանակում է փաստագրված դաս կամ կարուցվածք:</li>\n"
         "<li>%A մոխրագույն եզրերով ուղղանկյունը նշանակում է չփաստագրված դաս կամ կառուցվածք:</li>\n"
         "<li>%A կարմիր եզրերով ուղղանկյունը նշանակում է դաս կամ կառուցվածք, որի համար\n"
         " ոչ բոլոր ժառանգում/պարունակում կապերն են ցուցադրված: Գրաֆը կրճատված է, "
         "եթե այն չի տեղավորվում նշված սահմաններում:</li>\n"
         "</ul>\n"
         "Սլաքները ունեն հետևյալ իմաստը.\n"
         "<ul>\n"
         "<li>%A մուգ կապույտ սլաքը օգտագործվում է երկու դասերի միջև բաց ժառանգում "
         "կապը ցուցադրելու համար:</li>\n"
         "<li>%A մուգ կանաչ սլաքը օգտագործվում է պաշտպանված ժառանգման համար:</li>\n"
         "<li>%A մուգ կարմիր սլաքը օգտագործվում է փակ ժառանգման համար:</li>\n"
         "<li>%A մանուշակագույն կետագիծ սլաքը օգտագորշվում է, եթե դասը պարունակվում է"
         "այլ դասում կամ օգտագորշվում է այլ դասի կողմից: Սլաքը պիտակավորվաշ է"
         "փոփոխական(ներ)ով, որի միջոցով մատնանշված դասը կամ կառուցվածքը հասանելի է:</li>\n"
         "<li>Դեզին կետագիծ սլաքը ցույց է տալիս ձևանմուշի օրինակի կապը այն ձևանմուշի հետ, "
         "որից այն իրականցվել է. Սլաքը պիտակավորված է օրինակի ձևանմուշային պարամետրերով:</li>\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "լեգենդ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "Թեստ";
   }
   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "Թեստերի ցուցակ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Հատկություններ";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Հատկություններ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Տվյալների կառուցվածք";
      } else {
         return "Դասեր";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Փաթեթ " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Փաթեթների ցուցակ";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Բոլոր փաթեթները` կարճ բացատրություններով (եթե հասանելի են).";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Փաթեթներ";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Արժեքներ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Սխալ";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Սխալների ցուցակ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.6
   //////////////////////////////////////////////////////////////////////////
   /*! Used as ansicpg for RTF file */
   virtual QByteArray trRTFansicp() {
      return "armscii-8";
   }
   /*! Used as ansicpg for RTF fcharset */
   virtual QByteArray trRTFCharSet() {
      return "0";
   }
   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "Ցուցիչ";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         QByteArray result((first_capital ? "Տվյալների կառուցվածք" : "տվյալների կառուցվածք"));
         return result;
      } else {
         QByteArray result((first_capital ? "Դաս" : "դաս"));
         if (!singular) {
            result += "եր";
         }
         return result;
      }
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Ֆայլ" : "ֆայլ"));
      if (!singular) {
         result += "եր";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Անունների տարածություն" : "անունների տարածություն"));
      if (!singular) {
         result += "ներ";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Խ" : "խ"));
      result += (singular ? "ումբ" : "մբեր");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Էջ" : "էջ"));
      if (!singular) {
         result += "եր";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Անդամ" : "անդամ"));
      if (!singular) {
         result += "ներ";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Գլոբալ" : "գլոբալ"));
      if (!singular) {
         result += "ներ";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Հեղինակ" : "հեղինակ"));
      if (!singular) {
         result += "ներ";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Հղումներ - ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Իրագործում է հետևյալ դաս(եր)ի ֆունկցիաները - " + trWriteList(numEntries) + ":";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implementation this member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Իրագործվում է հետևյալում - " + trWriteList(numEntries) + ":";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Բովանդակություն";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Հնացած սահմանումների ցուցակը";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Պատահարներ";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Պատահարների ցուցակը";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Փաթեթի տիպեր";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Փաթեթի ֆունկցիաներ";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Փաթեթի ստատիկ ֆունկցիաներ";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Փաթեթի ատրիբուտներ";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Փաթեթի ստատիկ ատրիբուտներ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Բոլոր";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Այս ֆունկցիայի կանչերի գրաֆը.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Որոնման արդյունքները";
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
         return "Ներեցեք, բայց Ձեր որոնումը արդյունք չտվեց:";
      } else if ( numDocuments == 1 ) {
         return "Հայտնաբերվել է 1 փաստաթուղթ:";
      } else {
         return "Հայտնաբերվել է <b>$num</b> փաստաթուղթ:"
                "Փաստաթղթերը դասակարգված են ըստ համապասխանության";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Որոնման արդյունքներ:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return "Ելակետային ֆայլ " + filename;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Ֆայլադարանների հիերարխիա";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Ֆայլադարաններ";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Ֆայլադրաններ";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Այս ֆայլադարանների հիերարխիան կարգավորված է կոպտորեն, "
             "բայց ոչ ամբողջապես, այբբենական կարգով.";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " Ֆայլադարան";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Ֆայլադարան" : "ֆայլադարան"));
      if (!singular) {
         result += "ներ";
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
      return "Սա վերաբեռնված ֆունկցիա է` տրամադրված հարմարության համար: "
             "Այն տարբերվում է նախնականից միայն արգումնետներով:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Այս ֆունկցիայի կանչերի գրաֆը.";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Համարակալումներ";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////
   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Անդամ ֆունցիաներ/ենթածրագրեր";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Տվյալների տիպերի ցուցակը";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Տվյալների դաշտեր";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Տվյալների տիպերը` կարճ բացատրություններով.";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Բոլոր ";
      if (!extractAll) {
         result += "փաստագրված ";
      }
      result += "տվյալների տիպերի անդամների ցուցակը` հղումներով դեպի ";
      if (!extractAll) {
         result += "բոլոր անդամների տվյալների կառուցվածքի փաստագրությունը";
      } else {
         result += "տվյալների տիպերը, որոնց նրանք պատկանում են";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Տվյալների տիպեր";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Տվյալների տիպեր";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Ֆունկցիաներ/ենթածրագրեր";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Ֆունկցիաներ/ենթածրագրեր";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Տվյալների տիպեր";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Մոդուլների ցուցակ";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Բոլոր";
      if (!extractAll) {
         result += "փաստագրված ";
      }
      result += "մոդուլների ցուցակը` կարճ բացատրություններով.";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      if (!isTemplate) {
         switch (compType) {
            case ClassDef::Class:
               result += " Մոդուլ";
               break;
            case ClassDef::Struct:
               result += " Տիպ";
               break;
            case ClassDef::Union:
               result += " Միավորում";
               break;
            case ClassDef::Interface:
               result += " Ինտերֆեյս";
               break;
            case ClassDef::Protocol:
               result += " Արձանագրություն";
               break;
            case ClassDef::Category:
               result += " Դասակարգում";
               break;
            case ClassDef::Exception:
               result += " Բացառություն";
               break;
            default:
               break;
         }
      } else {
         switch (compType) {
            case ClassDef::Class:
               result += " Մոդուլի";
               break;
            case ClassDef::Struct:
               result += " Տիպի";
               break;
            case ClassDef::Union:
               result += " Միավորման";
               break;
            case ClassDef::Interface:
               result += " Ինտերֆեյսի";
               break;
            case ClassDef::Protocol:
               result += " Արձանագրության";
               break;
            case ClassDef::Category:
               result += " Դասակարգման";
               break;
            case ClassDef::Exception:
               result += " Բացառության";
               break;
            default:
               break;
         }
         result += " Ձևանմուշ";
      }
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      return QByteArray("Մոդուլ ") + namespaceName;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Մոդուլի անդամներ";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Մոդուլի բոլոր ";
      if (!extractAll) {
         result += "փաստագրված ";
      }
      result += "անդամների ցուցակը` հղումներով դեպի ";
      if (extractAll) {
         result += "բոլոր անդամների փաստագրությունները.";
      } else {
         result += "մոդուլները, որոնց նրանք պատկանում են.";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Մոդուլներ";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Մոդուլ" : "մոդուլ"));
      if (!singular) {
         result += "ներ";
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
      QByteArray result = (QByteArray)"Այս ";
      switch (compType) {
         case ClassDef::Class:
            result += "մոդուլի";
            break;
         case ClassDef::Struct:
            result += "տիպի";
            break;
         case ClassDef::Union:
            result += "միավորման";
            break;
         case ClassDef::Interface:
            result += "ինտերֆեյսի";
            break;
         case ClassDef::Protocol:
            result += "արձանագրության";
            break;
         case ClassDef::Category:
            result += "դասակարգման";
            break;
         case ClassDef::Exception:
            result += "բացառության";
            break;
         default:
            break;
      }
      result += " փաստագրությունը ստեղծվել է հետևալ ֆայլ";
      if (single) {
         result += "ից.";
      } else {
         result += "երից.";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Տիպ" : "տիպ"));
      if (!singular) {
         result += "եր";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Ե" : "ե"));
      if (singular) {
         result += "նթածրագիր";
      } else {
         result += "նթածրագրեր";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Տիպերի Սահմանափակումներ";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return QByteArray(name) + " Կապ";
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Բեռնում...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Գլոբալ անունների տարածություն";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Որոնում...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Անարդյունք";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Ֆայլը " + name + " ում";
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Ներառում է ֆայլը " + name + " ում";
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
      static const char *days[]   = { "Երկուշաբթի,", "Երեքշաբթի,", "Չորեքշաբթի,", "Հինգշաբթի,",
                                      "Ուրբաթ,", "Շաբաթ,", "Կիրակի,"
                                    };
      static const char *months[] = { "Հունիսի", "Փետրվարի", "Մարտի", "Ապրրիլի", "Մայիսի", "Հունիսի",
                                      "Հուլիսի", "Օգոստոսի", "Սեպտեմբերի", "Հոկտեբմերի", "Նոյեմբերի", "Դեկտեմբերի"
                                    };
      QByteArray sdate;
      sdate.sprintf("%s %d %s %d", days[dayOfWeek - 1], day, months[month - 1], year);
      if (includeTime) {
         QByteArray stime;
         stime.sprintf(" %.2d:%.2d:%.2d ", hour, minutes, seconds);
         sdate += stime;
      }
      return sdate;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.7.5
   //////////////////////////////////////////////////////////////////////////

   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Գրականություն";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Հեղինակային իրավունք";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return name + QByteArray("-ի ֆայլադարանների կախվածությունների գծագիր:");
   }

};

#endif
