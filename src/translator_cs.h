/************************************************************************
*
* Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
* Copyright (c) 1997-2014 Dimitri van Heesch
*
* DoxyPress is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License version 2
* as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
*************************************************************************/

#ifndef TRANSLATOR_CS_H
#define TRANSLATOR_CS_H

#include <config.h>
#include <translator.h>

QString generateMarker(int id);

class Translator_Cs : public Translator
{
 public:

   /*! Used to get the LaTeX command(s) for the language support.
    *  This method should return a string with commands that switch LaTeX to the desired language.
    *
    *  <pre>"\\usepackage[german]{babel}\n"
    *  </pre>
    *
    *  <pre>"\\usepackage{polski}\n"
    *  "\\usepackage[latin2]{inputenc}\n"
    *  "\\usepackage[T1]{fontenc}\n"
    *  </pre>
    *
    * English LaTeX does not use these commands, return an empty string
    */
   QString latexLanguageSupportCommand() override {
      QString retval = QCoreApplication::translate("latexSupport", "remove-value");

      if (retval == "remove-value") {
         retval = "";
      }

      return retval;
   }

   QString trLanguageId() override {
      return QCoreApplication::translate("doxy-text", "ISO639 language code");
   }

   /*! used in the compound documentation before a list of related functions. */
   QString trRelatedFunctions() override {
      return QCoreApplication::translate("doxy-text", "Related Functions");
   }

   /*! subscript for the related functions. */
   QString trRelatedSubscript() override {
      return QCoreApplication::translate("doxy-text", "These are not member functions");
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   QString trDetailedDescription() override {
      return QCoreApplication::translate("doxy-text", "Detailed Description");
   }

   /*! header that is put before the list of typedefs. */
   QString trMemberTypedefDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Member Typedef Documentation");
   }

   /*! header that is put before the list of enumerations. */
   QString trMemberEnumerationDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Member Enumeration Documentation");
   }

   /*! header that is put before the list of member functions. */
   QString trMemberFunctionDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Member Function Documentation");
   }

   /*! header that is put before the list of member attributes. */
   QString trMemberDataDocumentation() override {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Field Documentation");
      } else {
         return QCoreApplication::translate("doxy-text", "Member Data Documentation");
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   QString trMore() override {
      return QCoreApplication::translate("doxy-text", "More...");;
   }

   /*! put in the class documentation */
   QString trListOfAllMembers() override {
      return QCoreApplication::translate("doxy-text", "List of all members");
   }

   /*! title of the "list of all members" page of a class */
   QString trMemberList() override {
      return QCoreApplication::translate("doxy-text", "Member List");
   }

   /*! first part of a sentence that is followed by a class name */
   QString trThisIsTheListOfAllMembers() override {
      return QCoreApplication::translate("doxy-text", "This is the complete list of members for ");
   }

   /*! remainder of the sentence after the class name */
   QString trIncludingInheritedMembers() override {
      return QCoreApplication::translate("doxy-text", ", including all inherited members.");
   }

   /*! the author sections at the bottom of man pages, s is the project name. */
   QString trGeneratedAutomatically(const QString &s) override {
      if (! s.isEmpty() ) {
         return QCoreApplication::translate("doxy-text", "Generated automatically by DoxyPress for %1 from the source code.").formatArg(s);

      } else {
         return QCoreApplication::translate("doxy-text", "Generated automatically by DoxyPress from the source code.");

      }
   }

   /*! put after an enum name in the list of all members */
   QString trEnumName() override {
      return QCoreApplication::translate("doxy-text", "enum name");;
   }

   /*! put after an enum value in the list of all members */
   QString trEnumValue() override {
      return QCoreApplication::translate("doxy-text", "enum value");
   }

   /*! put after an undocumented member in the list of all members */
   QString trDefinedIn() override {
      return QCoreApplication::translate("doxy-text", "defined in");
   }

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command). */
   QString trModules() override {
      return QCoreApplication::translate("doxy-text", "Modules", "trModules");
   }

   /*! put above each page as a link to the class hierarchy */
   QString trClassHierarchy() override {
      return QCoreApplication::translate("doxy-text", "Class Hierarchy");
   }

   /*! put above each page as a link to the list of annotated classes */
   QString trCompoundList() override {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Structures", "trCompoundList");
      } else {
         return QCoreApplication::translate("doxy-text", "Class List", "trCompoundList");
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   QString trFileList() override {
      return QCoreApplication::translate("doxy-text", "File List");
   }

   /*! This is put above each page as a link to all members of compounds. */
   QString trCompoundMembers() override {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Fields", "trCompoundMembers");
      } else {
         return QCoreApplication::translate("doxy-text", "Class Members", "trCompoundMembers");
      }
   }

   /*! This is put above each page as a link to all members of files. */
   QString trFileMembers() override {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Globals", "trFileMembers");
      } else {
         return QCoreApplication::translate("doxy-text", "File Members", "trFileMembers");
      }
   }

   /*! This is put above each page as a link to all related pages. */
   QString trRelatedPages() override {
      return QCoreApplication::translate("doxy-text", "Related Pages");
   }

   /*! This is put above each page as a link to file source. */
   QString trFileSource() override {
      return QCoreApplication::translate("doxy-text", "File Source");
   }

   /*! This is put above each page as a link to all examples. */
   QString trExamples() override {
      return QCoreApplication::translate("doxy-text", "Examples");
   }

   /*! This is put above each page as a link to the search engine. */
   QString trSearch() override {
      return QCoreApplication::translate("doxy-text", "Search");
   }

   /*! This is an introduction to the class hierarchy. */
   QString trClassHierarchyDescription() override {
      return QCoreApplication::translate("doxy-text", "This inheritance list is sorted roughly, "
                "but not completely, alphabetically:");
   }

   /*! This is an introduction to the list with all files. */
   QString trFileListDescription(bool extractAll) override {
      if (extractAll) {
         return QCoreApplication::translate("doxy-text", "Here is a list of all files with brief "
                  "descriptions:");
      } else {
         return QCoreApplication::translate("doxy-text", "Here is a list of all documented files "
                  "with brief descriptions:");
      }
   }

   /*! This is an introduction to the annotated compound list. */
   QString trCompoundListDescription() override {

      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text",
               "Here are the data structures with brief descriptions:");

      } else {
         return QCoreApplication::translate("doxy-text", "Here are the classes, structs, "
                "unions and interfaces with brief descriptions:");
      }
   }

   /*! This is an introduction to the page with all class members. */
   QString trCompoundMembersDescription(bool extractAll) override {
      QString result;

      if (extractAll) {

         if (Config::getBool("optimize-c")) {
            result = QCoreApplication::translate("doxy-text", "Here is a list of all struct and union "
                  "fields with links to the structures/unions they belong to:");
         } else {
            result = QCoreApplication::translate("doxy-text", "Here is a list of all class members "
                  "with links to the classes they belong to:");
         }

      } else {

         if (Config::getBool("optimize-c")) {
            result = QCoreApplication::translate("doxy-text", "Here is a list of all documented struct "
                  "and union fields with links to the struct/union documentation for each field:");

         } else {
            result = QCoreApplication::translate("doxy-text", "Here is a list of all documented "
                  "class members with links to the class documentation for each member:");
         }

      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   QString trFileMembersDescription(bool extractAll) override {
      QString result;

      if (extractAll) {

         if (Config::getBool("optimize-c")) {
            result = QCoreApplication::translate("doxy-text", "Here is a list of all functions, variables, "
                  "defines, enums, and typedefs with links to the files they belong to:");

         } else {
            result = QCoreApplication::translate("doxy-text", "Here is a list of all documented file members "
                  "with links to the files they belong to:");
         }

      } else  {

         if (Config::getBool("optimize-c")) {
            result = QCoreApplication::translate("doxy-text", "Here is a list of all functions, variables, defines, "
                   "enums, and typedefs with links to the documentation:");

         } else {
            result = QCoreApplication::translate("doxy-text", "Here is a list of all documented file members with "
                  "links to the documentation:");

         }
      }

      return result;
   }

   /*! This is an introduction to the page with the list of all file source */
   QString trFileSourceDescription() override {
      return QCoreApplication::translate("doxy-text", "Here is a list of all source code files with brief descriptions:");
   }

   /*! This is an introduction to the page with the list of all examples */
   QString trExamplesDescription() override {
      return QCoreApplication::translate("doxy-text", "Here is a list of all examples:");
   }

   /*! This is an introduction to the page with the list of related pages */
   QString trRelatedPagesDescription() override {
      return QCoreApplication::translate("doxy-text", "Here is a list of all related documentation pages:");
   }

   /*! This is an introduction to the page with the list of class/file groups */
   QString trModulesDescription() override {
      return QCoreApplication::translate("doxy-text", "Here is a list of all modules:");
   }

   /*! This is an introduction for  concepts */
   QString trConcepts() override {
      return QCoreApplication::translate("doxy-text", "Concepts");
   }

   /*! This is an introduction to the page with the list of concepts */
   QString trConceptList() override {
      return QCoreApplication::translate("doxy-text", "Concept List");
   }

   /*! This is an introduction to the page with the list of class/file groups */
   QString trConceptListDescription() override {
      return QCoreApplication::translate("doxy-text", "Here is a list of all concepts with brief descriptions:");
   }

   // index titles (the project name is prepended for these)

   /*! This is used in HTML as the title of index.html. */
   QString trDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Documentation");
   }

   /*! This is used in LaTeX as the title of the chapter with the index of all groups. */
   QString trModuleIndex() override {
      return QCoreApplication::translate("doxy-text", "Module Index", "trModuleIndex");
   }

   /*! This is used in LaTeX as the title of the chapter with the class hierarchy. */
   QString trHierarchicalIndex() override {
      return QCoreApplication::translate("doxy-text", "Hierarchical Index");
   }

   /*! This is used in LaTeX as the title of the chapter with the annotated compound index. */
   QString trCompoundIndex() override {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Structure Index");
      } else {
         return QCoreApplication::translate("doxy-text", "Class Index");
      }
   }

   /*! used in LaTeX as the title of the chapter with the list of all files. */
   QString trFileIndex() override {
      return QCoreApplication::translate("doxy-text", "File Index");
   }

   /*! used in LaTeX as the title of the chapter containing the documentation of all groups. */
   QString trModuleDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Module Documentation");
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions. */
   QString trClassDocumentation() override {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Structure Documentation");
      } else {
         return QCoreApplication::translate("doxy-text", "Class Documentation");
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   QString trFileDocumentation() override {
      return QCoreApplication::translate("doxy-text", "File Documentation");
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   QString trExampleDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Example Documentation");
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   QString trPageDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Page Documentation");
   }

   /*! This is used in LaTeX as the title of the document */
   QString trReferenceManual() override {
      return QCoreApplication::translate("doxy-text", "Reference Manual");
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   QString trDefines() override {
      return QCoreApplication::translate("doxy-text", "Macros");
   }

   //
   QString trPublicTypedefs() override {
      return QCoreApplication::translate("doxy-text", "Public Typedefs");
   }

   QString trProtectedTypedefs() override {
      return QCoreApplication::translate("doxy-text", "Protected Typedefs");
   }

   QString trPackageTypedefs() override   {
      return QCoreApplication::translate("doxy-text", "Package Typedefs");
   }

   QString trPrivateTypedefs() override {
      return QCoreApplication::translate("doxy-text", "Private Typedefs");
   }

   //
   QString trPublicTypes() override {
      return QCoreApplication::translate("doxy-text", "Public Types");
   }

   QString trProtectedTypes() override {
      return QCoreApplication::translate("doxy-text", "Protected Types");
   }

   QString trPackageTypes() override   {
      return QCoreApplication::translate("doxy-text", "Package Types");
   }

   QString trPrivateTypes() override {
      return QCoreApplication::translate("doxy-text", "Private Types");
   }

   /*! used in the documentation of a file as a header before the list of typedefs */
   QString trTypedefs() override {
      return QCoreApplication::translate("doxy-text", "Typedefs");
   }

   /*! used in the documentation of a file as a header before the list of enumerations */
   QString trEnumerations() override {
      return QCoreApplication::translate("doxy-text", "Enums");
   }

   /*! used in the documentation of a file as a header before the list of (global) functions */
   QString trFunctions() override {
      return QCoreApplication::translate("doxy-text", "Functions");
   }

   /*! used in the documentation of a file as a header before the list of (global) variables */
   QString trVariables() override {
      return QCoreApplication::translate("doxy-text", "Variables");
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables */
   QString trEnumerationValues() override {
      return QCoreApplication::translate("doxy-text", "Enum Values");
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines */
   QString trDefineDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Macro Definition Documentation");
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs */
   QString trTypedefDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Typedef Documentation");
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   QString trEnumerationTypeDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Enumeration Type Documentation");
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   QString trFunctionDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Function Documentation");
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   QString trVariableDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Variable Documentation");
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   QString trCompounds() override {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Structures", "trCompounds");
      } else {
         return QCoreApplication::translate("doxy-text", "Classes", "trCompounds");
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   QString trGeneratedAt(const QString &date, const QString &projName) override {

      if (projName.isEmpty()) {
         return QCoreApplication::translate("doxy-text", "Generated on %1 &nbsp; by").formatArg(date);
      } else  {
         return QCoreApplication::translate("doxy-text", "Generated on %1 for %2 &nbsp; by").formatArg(date).formatArg(projName);
      }
   }

   /*! this text is put before a class diagram */
   QString trClassDiagram(const QString &clName) override {
      return QCoreApplication::translate("doxy-text", "Inheritance diagram for %1:").formatArg(clName);
   }

   /*! this text is generated when the \\internal command is used. */
   QString trForInternalUseOnly() override {
      return QCoreApplication::translate("doxy-text", "For internal use only.");
   }

   /*! this text is generated when the \\warning command is used. */
   QString trWarning() override {
      return QCoreApplication::translate("doxy-text", "Warning");
   }

   /*! this text is generated when the \\version command is used. */
   QString trVersion() override {
      return QCoreApplication::translate("doxy-text", "Version");
   }

   /*! this text is generated when the \\date command is used. */
   QString trDate() override {
      return QCoreApplication::translate("doxy-text", "Date");
   }

   /*! this text is generated when the \\return command is used. */
   QString trReturns() override {
      return QCoreApplication::translate("doxy-text", "Returns");
   }

   /*! this text is generated when the \\sa command is used. */
   QString trSeeAlso() override {
      return QCoreApplication::translate("doxy-text", "See also");
   }

   /*! this text is generated when the \\param command is used. */
   QString trParameters() override {
      return QCoreApplication::translate("doxy-text", "Parameters");
   }

   /*! this text is generated when the \\exception command is used. */
   QString trExceptions() override {
      return QCoreApplication::translate("doxy-text", "Exceptions");
   }

   /*! this text is used in the title page of a LaTeX document. */
   QString trGeneratedBy() override {
      return QCoreApplication::translate("doxy-text", "Generated by");
   }

   /*! used as the title of page containing all the index of all namespaces. */
   QString trNamespaceList() override {
      return QCoreApplication::translate("doxy-text", "Namespace List");
   }

   /*! used as an introduction to the namespace list */
   QString trNamespacesListDescription(bool extractAll) override {

      if (extractAll) {
         return QCoreApplication::translate("doxy-text", "Here is a list of all namespaces with "
                  "brief descriptions:");
      } else {
         return QCoreApplication::translate("doxy-text", "Here is a list of all documented namespaces "
                  "with brief descriptions:");
      }
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   QString trFriends() override {
      return QCoreApplication::translate("doxy-text", "Friends");
   }

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   QString trRelatedFunctionDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Friends And Related Function Documentation");
   }

   /*! used as the title of the HTML page of a class/struct/union */
   QString trCompoundReference(const QString &clName, enum CompoundType compType, bool isTemplate) override {
      QString result;

      switch (compType) {
         case CompoundType::Class:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Class Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Class Reference").formatArg(clName);
            }
            break;

         case CompoundType::Struct:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Struct Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Struct Reference").formatArg(clName);
            }
            break;

         case CompoundType::Union:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Union Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Union Reference").formatArg(clName);
            }
            break;

         case CompoundType::Interface:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Interface Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Interface Reference").formatArg(clName);
            }
            break;

         case CompoundType::Protocol:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Protocol Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Protocol Reference").formatArg(clName);
            }
            break;

         case CompoundType::Category:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Category Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Category Reference").formatArg(clName);
            }
            break;

         case CompoundType::Exception:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Exception Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Exception Reference").formatArg(clName);
            }
            break;

         default:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Reference").formatArg(clName);
            }
            break;
      }

      return result;
   }

   /*! used as the title of the HTML page of a file */
   QString trFileReference(const QString &fileName)  override {
      return QCoreApplication::translate("doxy-text", "%1 File Reference").formatArg(fileName);
   }

   /*! used as the title of the HTML page of a namespace */
   QString trNamespaceReference(const QString &name)  override {
      return QCoreApplication::translate("doxy-text", "%1 Namespace Reference").formatArg(name);
   }

   QString trPublicMembers() override {
      return QCoreApplication::translate("doxy-text", "Public Member Functions");
   }

   QString trPublicSignals() override {
      return QCoreApplication::translate("doxy-text", "Public Signals");
   }

   QString trProtectedSignals() override {
      return QCoreApplication::translate("doxy-text", "Protected Signals");
   }

   QString trPrivateSignals() override {
      return QCoreApplication::translate("doxy-text", "Private Signals");
   }

   QString trStaticPublicMembers() override {
      return QCoreApplication::translate("doxy-text", "Static Public Member Functions");
   }

   QString trProtectedMembers() override {
      return QCoreApplication::translate("doxy-text", "Protected Member Functions");
   }

   QString trPublicSlots() override {
      return QCoreApplication::translate("doxy-text", "Public Slots");
   }

   QString trProtectedSlots() override {
      return QCoreApplication::translate("doxy-text", "Protected Slots");
   }

   QString trStaticProtectedMembers() override {
      return QCoreApplication::translate("doxy-text", "Static Protected Member Functions");
   }

   QString trPrivateMembers() override {
      return QCoreApplication::translate("doxy-text", "Private Member Functions");
   }

   QString trPrivateSlots() override {
      return QCoreApplication::translate("doxy-text", "Private Slots");
   }

   QString trStaticPrivateMembers() override {
      return QCoreApplication::translate("doxy-text", "Static Private Member Functions");
   }

   // returns a comma-separated list of items, use generateMarker(x) to indicate where item x should be put
   QString trWriteList(int numEntries) override {
      QString result;

      for (int i = 0; i < numEntries; i++) {

         // generate placeholders for the class links, entry i in the list
         result += generateMarker(i);

         // order is left to right, @0, @1, @2, and @3

         if (i != numEntries - 1) {
            // not at the last entry, add separator

            if (i < numEntries - 2) {     // in the middle of the list
               result += ", ";

            } else {                      // at the second to last entry
               result += QCoreApplication::translate("doxy-text", ", and ");
            }
         }
      }

      return result;
   }

   // used in class docs to produce a list of base classes,  if class diagrams are disabled.
   QString trInheritsList(int numEntries) override {
      return QCoreApplication::translate("doxy-text", "Inherits %1.").formatArg(trWriteList(numEntries));
   }

   /*! used in class docs to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   QString trInheritedByList(int numEntries) override {
      return QCoreApplication::translate("doxy-text", "Inherited by %1.").formatArg( trWriteList(numEntries));
   }

   // used in member docs to produce a list of members that are hidden by this one
   QString trReimplementedFromList(int numEntries) override {
      return QCoreApplication::translate("doxy-text", "Reimplemented from %1.").formatArg( trWriteList(numEntries));
   }

   // used in member docs to produce a list of all member that overwrite the implementation of this member
   QString trReimplementedInList(int numEntries)  override {
      return QCoreApplication::translate("doxy-text", "Reimplemented in %1.").formatArg( trWriteList(numEntries));
   }

   /*! This is put above each page as a link to all members of namespaces. */
   QString trNamespaceMembers() override {
      return QCoreApplication::translate("doxy-text", "Namespace Members");
   }

   /*! This is an introduction to the page with all namespace members */
   QString trNamespaceMemberDescription(bool extractAll) override {
      if (extractAll) {
         return QCoreApplication::translate("doxy-text", "Here is a list of all namespace members with links "
                  "to the namespace documentation for each member:");
      } else {
         return QCoreApplication::translate("doxy-text", "Here is a list of all documented namespace members "
                  "with links to the namespaces they belong to:");
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   QString trNamespaceIndex() override {
      return QCoreApplication::translate("doxy-text", "Namespace Index");
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   QString trNamespaceDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Namespace Documentation");
   }

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   QString trNamespaces() override {
      return QCoreApplication::translate("doxy-text", "Namespaces", "trNamespaces");
   }

   /*! This is put at the bottom of a class documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   QString trGeneratedFromFiles(enum CompoundType compType, bool single) override {
      QString result;

      switch (compType) {

         case CompoundType::Class:
            if (single) {
              result = QCoreApplication::translate("doxy-text", "The documentation for this class "
                     " was generated from the following file:");
            } else {
              result = QCoreApplication::translate("doxy-text", "The documentation for this class "
                     " was generated from the following files:");
            }
            break;

         case CompoundType::Struct:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this struct "
                     " was generated from the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this struct "
                     " was generated from the following files:");
            }
            break;

         case CompoundType::Union:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this union "
                     " was generated from the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this union "
                     " was generated from the following files:");
            }
            break;

         case CompoundType::Interface:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this interface "
                     " was generated from the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this interface "
                     " was generated from the following files:");
            }
            break;

         case CompoundType::Protocol:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this protocol "
                     " was generated from the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this protocol "
                     " was generated from the following files:");
            }
            break;

         case CompoundType::Category:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this category "
                     " was generated from the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this category "
                     " was generated from the following files:");
            }
            break;

         case CompoundType::Exception:

            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this exception"
                     " was generated from the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this exception"
                     " was generated from the following files:");
            }
            break;

         default:

            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this "
                     " was generated from the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this "
                     " was generated from the following files:");
            }

            break;
      }

      return result;
   }

   /*! This is used as the heading text for the retval command. */
   QString trReturnValues() override {
      return QCoreApplication::translate("doxy-text", "Return Values");
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   QString trMainPage() override {
      return QCoreApplication::translate("doxy-text", "Main Page");
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   QString trPageAbbreviation() override {
      return QCoreApplication::translate("doxy-text", "p,");
   }

   QString trDefinedAtLineInSourceFile() override {
      return QCoreApplication::translate("doxy-text", "Definition at line @0 of file @1.");
   }

   QString trDefinedInSourceFile() override {
      return QCoreApplication::translate("doxy-text", "Definition in file @0.");
   }

   QString trDeprecated() override {
      return QCoreApplication::translate("doxy-text", "Deprecated");
   }

   /*! this text is put before a collaboration diagram */
   QString trCollaborationDiagram(const QString &clName) override {
      return QCoreApplication::translate("doxy-text", "Collaboration diagram for %1:").formatArg(clName);
   }

   /*! this text is put before an include dependency graph */
   QString trInclDepGraph(const QString &fName)  override {
      return QCoreApplication::translate("doxy-text", "Include dependency graph for %1:").formatArg(fName);
   }

   /*! header that is put before the list of constructor/destructors. */
   QString trConstructorDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Constructor & Destructor Documentation");
   }

   /*! Used in the file documentation to point to the corresponding sources. */
   QString trGotoSourceCode() override {
      return QCoreApplication::translate("doxy-text", "Go to the source code of this file.");
   }

   /*! Used in the file sources to point to the corresponding documentation. */
   QString trGotoDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Go to the documentation of this file.");
   }

   /*! Text for the \\pre command */
   QString trPrecondition() override {
      return QCoreApplication::translate("doxy-text", "Precondition");
   }

   /*! Text for the \\post command */
   QString trPostcondition() override {
      return QCoreApplication::translate("doxy-text", "Postcondition");
   }
   /*! Text for the \\invariant command */
   QString trInvariant() override {
      return QCoreApplication::translate("doxy-text", "Invariant");
   }

   /*! Text shown before a multi-line variable/enum initialization */
   QString trInitialValue() override {
      return QCoreApplication::translate("doxy-text", "Initial value:");
   }
   /*! Text used the source code in the file index */
   QString trCode() override {
      return QCoreApplication::translate("doxy-text", "code");
   }

   QString trGraphicalHierarchy() override {
      return QCoreApplication::translate("doxy-text", "Graphical Class Hierarchy");
   }

   QString trGotoGraphicalHierarchy() override {
      return QCoreApplication::translate("doxy-text", "Go to the graphical class hierarchy");
   }

   QString trGotoTextualHierarchy() override {
      return QCoreApplication::translate("doxy-text", "Go to the textual class hierarchy");
   }

   QString trPageIndex() override {
      return QCoreApplication::translate("doxy-text", "Page Index");
   }

   QString trNote() override {
      return QCoreApplication::translate("doxy-text", "Note");
   }

   QString trPublicAttribs() override {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Fields", "trPublicAttribs");
      } else {
         return QCoreApplication::translate("doxy-text", "Public Attributes" , "trPublicAttribs");
      }
   }

   QString trStaticPublicAttribs() override {
      return QCoreApplication::translate("doxy-text", "Static Public Attributes");
   }

   QString trProtectedAttribs() override {
      return QCoreApplication::translate("doxy-text", "Protected Attributes");
   }

   QString trStaticProtectedAttribs() override {
      return QCoreApplication::translate("doxy-text", "Static Protected Attributes");
   }

   QString trPrivateAttribs() override {
      return QCoreApplication::translate("doxy-text", "Private Attributes");
   }

   QString trStaticPrivateAttribs() override {
      return QCoreApplication::translate("doxy-text", "Static Private Attributes");
   }

   /*! Used as a marker that is put before a \\todo item */
   QString trTodo() override {
      return QCoreApplication::translate("doxy-text", "Todo");
   }

   /*! Used as the header of the todo list */
   QString trTodoList() override {
      return QCoreApplication::translate("doxy-text", "Todo List");
   }

   QString trReferencedBy() override {
      return QCoreApplication::translate("doxy-text", "Referenced by");
   }

   QString trRemarks() override {
      return QCoreApplication::translate("doxy-text", "Remarks");
   }

   QString trAttention() override {
      return QCoreApplication::translate("doxy-text", "Attention");
   }

   QString trInclByDepGraph() override {
      return QCoreApplication::translate("doxy-text", "This graph shows which files "
                  "directly or indirectly include this file");
   }

   QString trSince() override {
      return QCoreApplication::translate("doxy-text", "Since");
   }

   /*! title of the graph legend page */
   QString trLegendTitle() override {
      return QCoreApplication::translate("doxy-text", "Graph Legend");
   }

   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the translated text are to prevent link to classes called "A". */
   QString trLegendDocs(const QString &dotFormat) override {
      return QCoreApplication::translate("doxy-text", "Dot Graph legend source code").formatArg(dotFormat);
   }

   /*! text for the link to the legend page */
   QString trLegend() override {
      return QCoreApplication::translate("doxy-text", "legend");
   }

   /*! Used as a marker that is put before a test item */
   QString trTest() override {
      return QCoreApplication::translate("doxy-text", "Test");
   }

   /*! Used as the header of the test list */
   QString trTestList() override {
      return QCoreApplication::translate("doxy-text", "Test List");
   }

   /*! Used as a section header for IDL properties */
   QString trProperties() override {
      return QCoreApplication::translate("doxy-text", "Properties");
   }

   /*! Used as a section header for IDL property documentation */
   QString trPropertyDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Property Documentation");
   }

   /*! Used for Java classes in the summary section of Java packages */
   QString trClasses() override {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Structures", "trClasses");
      } else {
         return QCoreApplication::translate("doxy-text", "Classes", "trClasses");
      }
   }

   /*! Used as the title of a Java package */
   QString trPackage(const QString &name) override {
      return QCoreApplication::translate("doxy-text", "Package %1").formatArg(name);
   }

   /*! Title of the package index page */
   QString trPackageList() override {
      return QCoreApplication::translate("doxy-text", "Package List");
   }

   /*! The description of the package index page */
   QString trPackagesListDescription() override {
      return QCoreApplication::translate("doxy-text", "Here are the packages with "
                  "brief descriptions (if available):");
   }

   /*! The link name in the Quick links header for each page */
   QString trPackages() override {
      return QCoreApplication::translate("doxy-text", "Packages");
   }

   /*! Text shown before a multi-line define */
   QString trDefineValue() override {
      return QCoreApplication::translate("doxy-text", "Value:");
   }

   /*! Used as a marker that is put before a \\bug item */
   QString trBug() override {
      return QCoreApplication::translate("doxy-text", "Bug");
   }

   /*! Used as the header of the bug list */
   QString trBugList() override {
      return QCoreApplication::translate("doxy-text", "Bug List");
   }

   /*! Used as ansi cpg for RTF file
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
   QString trRTFansicp() override {
      return QCoreApplication::translate("doxy-rtf", "1252", "trRTFansicp");
   }

   /*! Used as ansicpg for RTF fcharset
    *  \see trRTFansicp() for a table of possible values.
    */
   QString trRTFCharSet() override {
      return QCoreApplication::translate("doxy-rtf", "0", "trRTFCharSet");
   }

   /*! Used as header RTF general index */
   QString trRTFGeneralIndex() override {
      return QCoreApplication::translate("doxy-text", "Index");
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names of the category.
    */
   QString trClass(bool first_capital, bool singular) override {
      if (first_capital) {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Class", "trClass");
         } else  {
            return QCoreApplication::translate("doxy-text", "Classes", "trClass");
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "class", "trClass");
         } else  {
            return QCoreApplication::translate("doxy-text", "classes", "trClass");
         }
      }
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   QString trFile(bool first_capital, bool singular) override {
      if (first_capital) {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "File");
         } else  {
            return QCoreApplication::translate("doxy-text", "Files");
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "file");
         } else  {
            return QCoreApplication::translate("doxy-text", "files");
         }
      }
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   QString trNamespace(bool first_capital, bool singular) override {
      if (first_capital) {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Namespace", "trNamespace");
         } else  {
            return QCoreApplication::translate("doxy-text", "Namespaces", "trNamespace");
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "namespace", "trNamespace");
         } else  {
            return QCoreApplication::translate("doxy-text", "namespaces", "trNamespace");
         }
      }
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names of the category.
    */
   QString trGroup(bool first_capital, bool singular) override {
      if (first_capital) {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Module", "trGroup");
         } else  {
            return QCoreApplication::translate("doxy-text", "Modules", "trGroup");
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "module", "trGroup");
         } else  {
            return QCoreApplication::translate("doxy-text", "modules", "trGroup");
         }
      }
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names of the category.
    */
   QString trPage(bool first_capital, bool singular) override {
      if (first_capital) {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Page");
         } else  {
            return QCoreApplication::translate("doxy-text", "Pages");
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "page");
         } else  {
            return QCoreApplication::translate("doxy-text", "pages");
         }
      }
   }


   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   QString trMember(bool first_capital, bool singular) override {
      if (first_capital) {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Member");
         } else  {
            return QCoreApplication::translate("doxy-text", "Members");
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "member");
         } else  {
            return QCoreApplication::translate("doxy-text", "members");
         }
      }
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   QString trGlobal(bool first_capital, bool singular) override {
      if (first_capital) {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Global", "trGlobal");
         } else  {
            return QCoreApplication::translate("doxy-text", "Globals", "trGlobal");
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "global", "trGlobal");
         } else {
            return QCoreApplication::translate("doxy-text", "globals", "trGlobal");
         }
      }
   }

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   QString trAuthor(bool first_capital, bool singular) override {
      if (first_capital) {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Author", "trAuthor");
         } else  {
            return QCoreApplication::translate("doxy-text", "Authors", "trAuthor");;
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "author", "trAuthor");
         } else  {
            return QCoreApplication::translate("doxy-text", "authors", "trAuthor");
         }
      }
   }

   // this text is put before the list of members referenced by a member
   QString trReferences() override {
      return QCoreApplication::translate("doxy-text", "References");
   }

   // used in member documentation to produce a list of members whcih are implemented by this one
   QString trImplementedFromList(int numEntries) override {
     return QCoreApplication::translate("doxy-text", "Implements %1.").formatArg(trWriteList(numEntries));
   }

   // used in member documentation to produce a list of all members which implement this abstract member
   QString trImplementedInList(int numEntries) override {
      return QCoreApplication::translate("doxy-text", "Implemented in %1.").formatArg(trWriteList(numEntries));
   }

   // used in RTF documentation as a heading for the Table  of Contents
   QString trRTFTableOfContents() override {
      return QCoreApplication::translate("doxy-text", "Table of Contents");
   }

   // Used as the header of the list of item that have been flagged deprecated
   QString trDeprecatedList() override {
      return QCoreApplication::translate("doxy-text", "Deprecated List");
   }

   // Used as a header for declaration section of the events found in a C# program
   QString trEvents() override {
      return QCoreApplication::translate("doxy-text", "Events");
   }

   /*! Header used for the documentation section of a class' events. */
   QString trEventDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Event Documentation");
   }

   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   QString trPackageMembers() override   {
      return QCoreApplication::translate("doxy-text", "Package Functions");
   }

   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   QString trStaticPackageMembers() override  {
      return QCoreApplication::translate("doxy-text", "Static Package Functions");
   }

   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   QString trPackageAttribs() override {
      return QCoreApplication::translate("doxy-text", "Package Attributes");
   }

   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   QString trStaticPackageAttribs() override {
      return QCoreApplication::translate("doxy-text", "Static Package Attributes");
   }

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   QString trAll() override {
      return QCoreApplication::translate("doxy-text", "All");
   }

   /*! Put in front of the call graph for a function. */
   QString trCallGraph() override {
      return QCoreApplication::translate("doxy-text", "Here is the call graph for this function:");
   }

    /*! This string is used as the title for the page listing the search
    *  results.
    */
   QString trSearchResultsTitle() override {
      return QCoreApplication::translate("doxy-text", "Search Results");
   }

   /*! This string is put just before listing the search results. The
    *  text can be different depending on the number of documents found.
    *  Inside the text you can put the special marker $num to insert
    *  the number representing the actual number of search results.
    *  The @a numDocuments parameter can be either 0, 1 or 2, where the
    *  value 2 represents 2 or more matches. HTML markup is allowed inside
    *  the returned string.
    */
   QString trSearchResults(int numDocuments) override  {
      if (numDocuments == 0) {
         return QCoreApplication::translate("doxy-text",  "Sorry, no documents matching your query.");

      } else if (numDocuments == 1) {
         return QCoreApplication::translate("doxy-text", "Found <b>1</b> document matching your query.");

      } else {
         return QCoreApplication::translate("doxy-text", "Found <b>$num</b> documents matching your query. "
                "Showing best matches first.");
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   QString trSearchMatches() override  {
      return QCoreApplication::translate("doxy-text", "Matches:");
   }

   /*! This is used in HTML as the title of page with source code for file filename
    */
   QString trSourceFile(const QString &filename) override  {
      return QCoreApplication::translate("doxy-text", "%1 Source File").formatArg(filename);
   }

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   QString trDirIndex() override  {
      return QCoreApplication::translate("doxy-text", "Directory Hierarchy");
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   QString trDirDocumentation() override   {
      return QCoreApplication::translate("doxy-text", "Directory Documentation");
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   QString trDirectories() override  {
      return QCoreApplication::translate("doxy-text", "Directories");
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   QString trDirDescription() override {
      return QCoreApplication::translate("doxy-text", "This directory hierarchy is sorted roughly, "
             "but not completely, alphabetically:");
   }

   /*! This returns the title of a directory page. The name of the directory is passed via dirName. */
   QString trDirReference(const QString &dirName) override {
      return QCoreApplication::translate("doxy-text", "%1 Directory Reference").formatArg(dirName);
   }

   /*! returns the word directory with or without starting capital letter, in sigular or plural form. */
   QString trDir(bool first_capital, bool singular) override  {
      if (first_capital) {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Directory");
         } else {
            return QCoreApplication::translate("doxy-text", "Directories");
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "directory");
         } else {
            return QCoreApplication::translate("doxy-text", "directories");
         }
      }
   }

   /*! This text is added to the documentation when the \\overload command
    *  is used for a overloaded function.
    */
   QString trOverloadText() override {
      return QCoreApplication::translate("doxy-text", "This is an overloaded method or function provided for "
             "convenience. It differs from the other overloads only in the argument(s).");
   }

   /*! This is used to introduce a caller (or called-by) graph */
   QString trCallerGraph() override  {
      return QCoreApplication::translate("doxy-text", "Here is the caller graph for this function:");
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   QString trEnumerationValueDocumentation() override {
      return QCoreApplication::translate("doxy-text", "Enumerator Documentation");
   }

   /*! header that is put before the list of member subprograms (Fortran). */
   QString trMemberFunctionDocumentationFortran() override {
      return QCoreApplication::translate("doxy-text", "Member Function/Subroutine Documentation");
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   QString trCompoundListFortran() override  {
      return QCoreApplication::translate("doxy-text", "Data Types List");
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   QString trCompoundMembersFortran() override  {
      return QCoreApplication::translate("doxy-text", "Data Fields", "trCompoundMembersFortran");
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   QString trCompoundListDescriptionFortran() override  {
      return QCoreApplication::translate("doxy-text", "Here are the data types with brief descriptions:");
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   QString trCompoundMembersDescriptionFortran(bool extractAll) override  {
      QString result;

      if (extractAll) {
         result = QCoreApplication::translate("doxy-text", "Here is a list of all data types "
                  "members with links to the data types they belong to:");
      } else {
         result = QCoreApplication::translate("doxy-text", "Here is a list of all documented "
                  "data types members with links to the data structure documentation for each member:");
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   QString trCompoundIndexFortran() override  {
      return QCoreApplication::translate("doxy-text", "Data Type Index");
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   QString trTypeDocumentation() override  {
      return QCoreApplication::translate("doxy-text", "Data Type Documentation");
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   QString trSubprograms() override {
      return QCoreApplication::translate("doxy-text", "Functions/Subroutines");
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   QString trSubprogramDocumentation() override   {
      return QCoreApplication::translate("doxy-text", "Function/Subroutine Documentation");
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   QString trDataTypes() override {
      return QCoreApplication::translate("doxy-text", "Data Types");
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   QString trModulesList() override  {
      return QCoreApplication::translate("doxy-text", "Modules List");
   }

   /*! used as an introduction to the modules list (Fortran) */
   QString trModulesListDescription(bool extractAll) override  {
      if (extractAll) {
         return QCoreApplication::translate("doxy-text", "Here is a list of all modules with brief descriptions:");
      } else {
         return QCoreApplication::translate("doxy-text", "Here is a list of all documented modules with brief descriptions:");
      }
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   QString trCompoundReferenceFortran(const QString &clName, enum CompoundType compType,bool isTemplate) override  {
      QString result;

      switch (compType) {
         case CompoundType::Class:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Module Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Module Reference").formatArg(clName);
            }
            break;

         case CompoundType::Struct:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Type Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Type Reference").formatArg(clName);
            }
            break;

         case CompoundType::Union:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Union Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Union Reference").formatArg(clName);
            }
            break;

         case CompoundType::Interface:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Interface Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Interface Reference").formatArg(clName);
            }
            break;

         case CompoundType::Protocol:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Protocol Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1  Protocol Reference").formatArg(clName);
            }
            break;

         case CompoundType::Category:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Category Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Category Reference").formatArg(clName);
            }
            break;

         case CompoundType::Exception:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Exception Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Exception Reference").formatArg(clName);
            }
            break;

          default:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Template Reference").formatArg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Reference").formatArg(clName);
            }
            break;
      }

      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   QString trModuleReference(const QString &namespaceName) override  {
      return QCoreApplication::translate("doxy-text", "%1 Module Reference").formatArg(namespaceName);
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   QString trModuleMembers() override  {
      return QCoreApplication::translate("doxy-text", "Module Members");
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   QString trModulesMemberDescription(bool extractAll) override  {
      if (extractAll) {
         return QCoreApplication::translate("doxy-text", "Here is a list of all module members with links "
                  "to the module documentation for each member:");
      } else {
         return QCoreApplication::translate("doxy-text", "Here is a list of all documented module members "
                  "with links to the modules they belong to:");
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   QString trModulesIndex() override  {
      return QCoreApplication::translate("doxy-text", "Modules Index", "trModulesIndex");
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names of the category.
    */
   QString trModule(bool first_capital, bool singular) override  {

      if (first_capital) {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Module", "trModule");
         } else  {
            return QCoreApplication::translate("doxy-text", "Modules", "trModule");
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "module", "trModule");
         } else  {
            return QCoreApplication::translate("doxy-text", "modules", "trModule");
         }
      }
   }

   /*! This is put at the bottom of a module documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   QString trGeneratedFromFilesFortran(enum CompoundType compType, bool single) override  {
      QString result;

      switch (compType) {
         case CompoundType::Class:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this module was generated from "
                     "the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this module was generated from "
                     "the following files:");
            }
            break;
         case CompoundType::Struct:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this type was generated from "
                     "the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this type was generated from "
                     "the following files:");
            }
            break;
         case CompoundType::Union:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this union was generated from "
                     "the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this union was generated from "
                     "the following files:");
            }
            break;
         case CompoundType::Interface:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this interface was generated from "
                     "the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this interface was generated from "
                     "the following files:");
            }
            break;
         case CompoundType::Protocol:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this protocol was generated from "
                     "the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this protocol was generated from "
                     "the following files:");
            }
            break;
         case CompoundType::Category:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this category was generated from "
                     "the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this category was generated from "
                     "the following files:");
            }
            break;
         case CompoundType::Exception:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this exception was generated from "
                     "the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this exception was generated from "
                     "the following files:");
            }
            break;
         default:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this was generated from "
                     "the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this was generated from "
                     "the following files:");
            }
            break;
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   QString trType(bool first_capital, bool singular) override {
       if (first_capital) {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Type");
         } else  {
            return QCoreApplication::translate("doxy-text", "Types");
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "type");
         } else  {
            return QCoreApplication::translate("doxy-text", "types");
         }
      }
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   QString trSubprogram(bool first_capital, bool singular) override {
       if (first_capital) {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Subprogram");
         } else  {
            return QCoreApplication::translate("doxy-text", "Subprograms");
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "subprogram");
         } else  {
            return QCoreApplication::translate("doxy-text", "subprograms");
         }
      }
   }

   /*! C# Type Constraint list */
   QString trTypeConstraints() override  {
      return QCoreApplication::translate("doxy-text", "Type Constraints");
   }

   /*! directory relation for name */
   QString trDirRelation(const QString &name) override  {
      return QCoreApplication::translate("doxy-text", "%1 Relation").formatArg(name);
   }

   /*! Loading message shown when loading search results */
   QString trLoading() override  {
      return QCoreApplication::translate("doxy-text", "Loading...");
   }

   /*! Label used for search results in the global namespace */
   QString trGlobalNamespace() override {
      return QCoreApplication::translate("doxy-text", "Global Namespace");
   }

   /*! Message shown while searching */
   QString trSearching() override  {
      return QCoreApplication::translate("doxy-text", "Searching...");
   }

   /*! Text shown when no search results are found */
   QString trNoMatches() override  {
      return QCoreApplication::translate("doxy-text", "No Matches");
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   QString trFileIn(const QString &name) override {
      return QCoreApplication::translate("doxy-text", "File in %1").formatArg(name);
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   QString trIncludesFileIn(const QString &name) override  {
      return QCoreApplication::translate("doxy-text", "Includes file in %1").formatArg(name);
   }

   /*! Header for the page with bibliographic citations */
   QString trCiteReferences() override  {
      return QCoreApplication::translate("doxy-text", "Bibliography");
   }

   /*! Text for copyright paragraph */
   QString trCopyright() override {
      return QCoreApplication::translate("doxy-text", "Copyright");
   }

   /*! Header for the graph showing the directory dependencies */
   QString trDirDepGraph(const QString &name) override  {
      return QCoreApplication::translate("doxy-text", "Directory dependency graph for %1:").formatArg(name);
   }

   /*! Detail level selector shown for hierarchical indices */
   QString trDetailLevel() override {
      return QCoreApplication::translate("doxy-text", "detail level");
   }

   /*! Section header for list of template parameters */
   QString trTemplateParameters() override  {
      return QCoreApplication::translate("doxy-text", "Template Parameters");
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   QString trAndMore(const QString &number) override  {
      return QCoreApplication::translate("doxy-text", "and %1 more...").formatArg(number);
   }

   /*! Used file list for a Java enum */
   QString trEnumGeneratedFromFiles(bool single) override  {
      if (single) {
         return QCoreApplication::translate("doxy-text", "The documentation for this enum was generated "
                  "from the following file:");
      } else {
         return QCoreApplication::translate("doxy-text", "The documentation for this enum was generated "
                  " from the following files:");
      }
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   QString trEnumReference(const QString &name) override  {
      return QCoreApplication::translate("doxy-text", "%1 Enum Reference").formatArg(name);
   }

   /*! Used for a section containing inherited members */
   QString trInheritedFrom(const QString &members, const QString &what) override  {
      return QCoreApplication::translate("doxy-text", "%1 inherited from %2").formatArg(members).formatArg(what);
   }

   /*! Header of the sections with inherited members specific for the  base class(es) */
   QString trAdditionalInheritedMembers() override  {
      return QCoreApplication::translate("doxy-text", "Additional Inherited Members");
   }

   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   QString trPanelSyncTooltip(bool enable) override  {
      if (enable) {
         return QCoreApplication::translate("doxy-text", "click to enable panel synchronization");
      } else {
         return QCoreApplication::translate("doxy-text", "click to disable panel synchronization");
      }
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   QString trProvidedByCategory() override  {
      return QCoreApplication::translate("doxy-text", "Provided by category @0.");
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method. */
   QString trExtendsClass() override  {
      return QCoreApplication::translate("doxy-text", "Extends class @0");
   }

   /*! header of a list of class methods in Objective-C.
    *  similar to static public member functions in C++. */
   QString trClassMethods() override  {
      return QCoreApplication::translate("doxy-text", "Class Methods");
   }

   /*! header of a list of instance methods in Objective-C.
    *  similar to public member functions in C++. */
   QString trInstanceMethods() override  {
      return QCoreApplication::translate("doxy-text", "Instance Methods");
   }

   /*! Used as the header of the member functions of an Objective-C class. */
   QString trMethodDocumentation() override  {
      return QCoreApplication::translate("doxy-text", "Method Documentation");
   }

   /** old style UNO IDL services: implemented interfaces */
   QString trInterfaces() override {
      return QCoreApplication::translate("doxy-text", "Exported Interfaces");
   }

   /** old style UNO IDL services: inherited services */
   QString trServices() override {
      return QCoreApplication::translate("doxy-text", "Included Services");
   }

   /** UNO IDL constant groups */
   QString trConstantGroups() override {
      return QCoreApplication::translate("doxy-text", "Constant Groups");
   }

   /** UNO IDL constant groups */
   QString trConstantGroupReference(const QString &namespaceName)  override  {
      return QCoreApplication::translate("doxy-text", "%1 Constant Group Reference").formatArg(namespaceName);
   }

   /** UNO IDL service page title */
   QString trServiceReference(const QString &sName) override {
      return QCoreApplication::translate("doxy-text", "%1 Service Reference").formatArg(sName);
   }

   /** UNO IDL singleton page title */
   QString trSingletonReference(const QString &sName) override {
      return QCoreApplication::translate("doxy-text", "%1 Singleton Reference").formatArg(sName);
   }

   /** UNO IDL service page */
   QString trServiceGeneratedFromFiles(bool single) override {
      if (single) {
         return QCoreApplication::translate("doxy-text", "The documentation for this service "
                          "was generated from the following file:");

      } else {
        return QCoreApplication::translate("doxy-text", "The documentation for this service "
                          "was generated from the following files:");
      }
   }

   /** UNO IDL singleton page */
   QString trSingletonGeneratedFromFiles(bool single) override  {
      if (single) {
         return QCoreApplication::translate("doxy-text", "The documentation for this singleton "
                          "was generated from the following file:");

      } else {
        return QCoreApplication::translate("doxy-text", "The documentation for this singleton "
                          "was generated from the following files:");
      }
   }
};

#endif
