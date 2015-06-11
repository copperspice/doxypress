/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
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

#ifndef Translator_CS_H
#define Translator_CS_H

#include <config.h>
#include <translator.h>

QByteArray generateMarker(int id);

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
    * The English LaTeX does not use such commands.  Because of this
    * the empty string is returned in this implementation.
    */
   virtual QByteArray latexLanguageSupportCommand() {
      return QCoreApplication::translate("latexSupport", "").toUtf8();
   }
   
   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return QCoreApplication::translate("doxy-text", "Related Functions").toUtf8();
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return QCoreApplication::translate("doxy-text", "These are not member functions").toUtf8();
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return QCoreApplication::translate("doxy-text", "Detailed Description").toUtf8();
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return QCoreApplication::translate("doxy-text", "Member Typedef Documentation").toUtf8();
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return QCoreApplication::translate("doxy-text", "Member Enumeration Documentation").toUtf8();
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return QCoreApplication::translate("doxy-text", "Member Function Documentation").toUtf8();
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Field Documentation").toUtf8();
      } else {
         return QCoreApplication::translate("doxy-text", "Member Data Documentation").toUtf8();
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return QCoreApplication::translate("doxy-text", "More...").toUtf8();;
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return QCoreApplication::translate("doxy-text", "List of all members").toUtf8();
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return QCoreApplication::translate("doxy-text", "Member List").toUtf8();
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return QCoreApplication::translate("doxy-text", "This is the complete list of members for ").toUtf8();
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return QCoreApplication::translate("doxy-text", ", including all inherited members.").toUtf8();
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {        
      if (s) {      
         return QCoreApplication::translate("doxy-text", "Generated automatically by DoxyPress for %1 from the source code.").arg(s).toUtf8();

      } else {
         return QCoreApplication::translate("doxy-text", "Generated automatically by DoxyPress from the source code.").toUtf8();

      }
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return QCoreApplication::translate("doxy-text", "enum name").toUtf8();;
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return QCoreApplication::translate("doxy-text", "enum value").toUtf8();
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return QCoreApplication::translate("doxy-text", "defined in").toUtf8();
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return QCoreApplication::translate("doxy-text", "Modules").toUtf8();;
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return QCoreApplication::translate("doxy-text", "Class Hierarchy").toUtf8();
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Structures").toUtf8();
      } else {
         return QCoreApplication::translate("doxy-text", "Class List").toUtf8();;
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return QCoreApplication::translate("doxy-text", "File List").toUtf8();
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Fields").toUtf8();
      } else {
         return QCoreApplication::translate("doxy-text", "Class Members").toUtf8();
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Globals").toUtf8();
      } else {
         return QCoreApplication::translate("doxy-text", "File Members").toUtf8();
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return QCoreApplication::translate("doxy-text", "Related Pages").toUtf8();
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return QCoreApplication::translate("doxy-text", "Examples").toUtf8();
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return QCoreApplication::translate("doxy-text", "Search").toUtf8();
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return QCoreApplication::translate("doxy-text", "This inheritance list is sorted roughly, "
                "but not completely, alphabetically:").toUtf8();
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
   
      if (extractAll) {
         return QCoreApplication::translate("doxy-text", "Here is a list of all files with brief descriptions:").toUtf8();
      } else {
         return QCoreApplication::translate("doxy-text", "Here is a list of all documented files with brief descriptions:").toUtf8();
      }    
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", 
               "Here are the data structures with brief descriptions:").toUtf8();

      } else {
         return QCoreApplication::translate("doxy-text", "Here are the classes, structs, "
                "unions and interfaces with brief descriptions:").toUtf8();
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {         // BROOM
      QByteArray result = "Here is a list of all ";

      if (! extractAll) {
         result += "documented ";
      }
      if (Config::getBool("optimize-c")) {
         result += "struct and union fields";
      } else {
         result += "class members";
      }

      result += " with links to ";

      if (!extractAll) {
         if (Config::getBool("optimize-c")) {
            result += "the struct/union documentation for each field:";
         } else {
            result += "the class documentation for each member:";
         }
      } else {
         if (Config::getBool("optimize-c")) {
            result += "the structures/unions they belong to:";
         } else {
            result += "the classes they belong to:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {      
      QByteArray result;

      if (extractAll) { 

         if (Config::getBool("optimize-c")) {
            result = QCoreApplication::translate("doxy-text", "Here is a list of all functions, variables, "
                  "defines, enums, and typedefs with links to the files they belong to:").toUtf8();
      
         } else {
            result = QCoreApplication::translate("doxy-text", "Here is a list of all documented file members "
                  "with links to the files they belong to:").toUtf8();
         }

      } else  {

         if (Config::getBool("optimize-c")) {
            result = QCoreApplication::translate("doxy-text", "Here is a list of all functions, variables, defines, "
                   "enums, and typedefs with links to the documentation:").toUtf8();
      
         } else { 
            result = QCoreApplication::translate("doxy-text", "Here is a list of all documented file members with "
                  "links to the documentation:").toUtf8();

         }
      }     
   
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return QCoreApplication::translate("doxy-text", "Here is a list of all examples:").toUtf8();
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return QCoreApplication::translate("doxy-text", "Here is a list of all related documentation pages:").toUtf8();
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return QCoreApplication::translate("doxy-text", "Here is a list of all modules:").toUtf8();
   }

   // index titles (the project name is prepended for these)            

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return QCoreApplication::translate("doxy-text", "Documentation").toUtf8();
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return QCoreApplication::translate("doxy-text", "Module Index").toUtf8();
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return QCoreApplication::translate("doxy-text", "Hierarchical Index").toUtf8();
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Structure Index").toUtf8();
      } else {
         return QCoreApplication::translate("doxy-text", "Class Index").toUtf8();
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the list of all files.
    */
   virtual QByteArray trFileIndex() {
      return QCoreApplication::translate("doxy-text", "File Index").toUtf8();
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return QCoreApplication::translate("doxy-text", "Module Documentation").toUtf8();
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Structure Documentation").toUtf8();
      } else {
         return QCoreApplication::translate("doxy-text", "Class Documentation").toUtf8();
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return QCoreApplication::translate("doxy-text", "File Documentation").toUtf8();
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return QCoreApplication::translate("doxy-text", "Example Documentation").toUtf8();
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return QCoreApplication::translate("doxy-text", "Page Documentation").toUtf8();
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return QCoreApplication::translate("doxy-text", "Reference Manual").toUtf8();
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return QCoreApplication::translate("doxy-text", "Macros").toUtf8();
   }

   /*! This is used in the documentation of a file as a header before the list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return QCoreApplication::translate("doxy-text", "Typedefs").toUtf8();
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return QCoreApplication::translate("doxy-text", "Enums").toUtf8();
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return QCoreApplication::translate("doxy-text", "Functions").toUtf8();
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return QCoreApplication::translate("doxy-text", "Variables").toUtf8();
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return QCoreApplication::translate("doxy-text", "Enum Values").toUtf8();
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return QCoreApplication::translate("doxy-text", "Macro Definition Documentation").toUtf8();
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return QCoreApplication::translate("doxy-text", "Typedef Documentation").toUtf8();
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return QCoreApplication::translate("doxy-text", "Enumeration Type Documentation").toUtf8();
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return QCoreApplication::translate("doxy-text", "Function Documentation").toUtf8();
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return QCoreApplication::translate("doxy-text", "Variable Documentation").toUtf8();
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Structures").toUtf8();
      } else {
         return QCoreApplication::translate("doxy-text", "Classes").toUtf8();
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {  

      if (projName) {
         return QCoreApplication::translate("doxy-text", "Generated on %1 for %2 &nbsp; by").arg(date).arg(projName).toUtf8();
      }  else  {
         return QCoreApplication::translate("doxy-text", "Generated on %1 &nbsp; by").arg(date).toUtf8();
      }      
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return QCoreApplication::translate("doxy-text", "Inheritance diagram for %1:").arg(clName).toUtf8();
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return QCoreApplication::translate("doxy-text", "For internal use only.").toUtf8();
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return QCoreApplication::translate("doxy-text", "Warning").toUtf8();
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return QCoreApplication::translate("doxy-text", "Version").toUtf8();
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return QCoreApplication::translate("doxy-text", "Date").toUtf8();
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return QCoreApplication::translate("doxy-text", "Returns").toUtf8();
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return QCoreApplication::translate("doxy-text", "See also").toUtf8();
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return QCoreApplication::translate("doxy-text", "Parameters").toUtf8();
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return QCoreApplication::translate("doxy-text", "Exceptions").toUtf8();
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return QCoreApplication::translate("doxy-text", "Generated by").toUtf8();
   }

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return QCoreApplication::translate("doxy-text", "Namespace List").toUtf8();
   }

   /*! used as an introduction to the namespace list */   
   virtual QByteArray trNamespaceListDescription(bool extractAll) {      

      if (extractAll) {
         return QCoreApplication::translate("doxy-text", "Here is a list of all namespaces with brief descriptions:").toUtf8();
      } else {   
         return QCoreApplication::translate("doxy-text", "Here is a list of all documented namespaces with brief descriptions:").toUtf8();
      }
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return QCoreApplication::translate("doxy-text", "Friends").toUtf8();
   }

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return QCoreApplication::translate("doxy-text", "Friends And Related Function Documentation").toUtf8();
   }  

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName, ClassDef::CompoundType compType, bool isTemplate) {
      QByteArray result = (QByteArray)clName;
                                                                                 //  BROOM 
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
      if (isTemplate) {
         result += " Template";
      }
      result += " Reference";
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      return QCoreApplication::translate("doxy-text", "%1 File Reference").arg(fileName).toUtf8();
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *name) {   
      return QCoreApplication::translate("doxy-text", "%1 Namespace Reference").arg(name).toUtf8();
   }

   virtual QByteArray trPublicMembers() {
      return QCoreApplication::translate("doxy-text", "Public Member Functions").toUtf8();
   }
  
   virtual QByteArray trPublicSignals() {
      return QCoreApplication::translate("doxy-text", "Public Signals").toUtf8();
   }
   virtual QByteArray trProtectedSignals() {
      return QCoreApplication::translate("doxy-text", "Protected Signals").toUtf8();
   }
   virtual QByteArray trPrivateSignals() {
      return QCoreApplication::translate("doxy-text", "Private Signals").toUtf8();
   }

   virtual QByteArray trStaticPublicMembers() {
      return QCoreApplication::translate("doxy-text", "Static Public Member Functions").toUtf8();
   }
   virtual QByteArray trProtectedMembers() {
      return QCoreApplication::translate("doxy-text", "Protected Member Functions").toUtf8();
   }
   virtual QByteArray trPublicSlots() {
      return QCoreApplication::translate("doxy-text", "Public Slots").toUtf8();
   }
   virtual QByteArray trProtectedSlots() {
      return QCoreApplication::translate("doxy-text", "Protected Slots").toUtf8();
   }
   virtual QByteArray trStaticProtectedMembers() {
      return QCoreApplication::translate("doxy-text", "Static Protected Member Functions").toUtf8();
   }
   virtual QByteArray trPrivateMembers() {
      return QCoreApplication::translate("doxy-text", "Private Member Functions").toUtf8();
   }
   virtual QByteArray trPrivateSlots() {
      return QCoreApplication::translate("doxy-text", "Private Slots").toUtf8();
   }
   virtual QByteArray trStaticPrivateMembers() {
      return QCoreApplication::translate("doxy-text", "Static Private Member Functions").toUtf8();
   }

   /*! this function is used to produce a comma-separated list of items.
    *  use generateMarker(i) to indicate where item i should be put.
    */
   virtual QByteArray trWriteList(int numEntries) {                                    //  BROOM 
      QByteArray result;

      // the inherits list contain `numEntries' classes
      for (int i = 0; i < numEntries; i++) {

         // generate placeholders for the class links, entry i in the list
         result += generateMarker(i); 

         // order is left to right, @0, @1, @2, and @3

         if (i != numEntries - 1) {       
            // not at the last entry, add separator

            if (i < numEntries - 2) {     // in the middle of the list
               result += ", ";

            } else {                      // at the second to last entry
               result += ", and ";
            }
         }
      }

      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Inherits " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Inherited by " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Reimplemented from " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Reimplemented in " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return QCoreApplication::translate("doxy-text", "Namespace Members").toUtf8();
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {      // broom
      QByteArray result = "Here is a list of all ";
      if (!extractAll) {
         result += "documented ";
      }
      result += "namespace members with links to ";
      if (extractAll) {
         result += "the namespace documentation for each member:";
      } else {
         result += "the namespaces they belong to:";
      }
      return result;
   }


   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return QCoreApplication::translate("doxy-text", "Namespace Index").toUtf8();
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return QCoreApplication::translate("doxy-text", "Namespace Documentation").toUtf8();
   }

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return QCoreApplication::translate("doxy-text", "Namespaces").toUtf8();
   }

   /*! This is put at the bottom of a class documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFiles(ClassDef::CompoundType compType, bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"The documentation for this ";

      switch (compType) {
         case ClassDef::Class:
            result += "class";
            break;
         case ClassDef::Struct:
            result += "struct";
            break;
         case ClassDef::Union:
            result += "union";
            break;
         case ClassDef::Interface:
            result += "interface";
            break;
         case ClassDef::Protocol:
            result += "protocol";
            break;
         case ClassDef::Category:
            result += "category";
            break;
         case ClassDef::Exception:
            result += "exception";
            break;
         default:
            break;
      }
      result += " was generated from the following file";
      if (single) {
         result += ":";
      } else {
         result += "s:";
      }
      return result;
   }

// QCoreApplication::translate("doxy-text", "").toUtf8();


   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return QCoreApplication::translate("doxy-text", "Return Values").toUtf8();
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return QCoreApplication::translate("doxy-text", "Main Page").toUtf8();
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return QCoreApplication::translate("doxy-text", "p,").toUtf8();
   }
 
   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Definition at line @0 of file @1.";
   }

   virtual QByteArray trDefinedInSourceFile() {
      return "Definition in file @0.";
   }

   virtual QByteArray trDeprecated() {
      return "Deprecated";
   }

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Collaboration diagram for " + clName + ":";
   }

   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Include dependency graph for " + fName + ":";
   }

   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Constructor & Destructor Documentation";
   }

   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Go to the source code of this file.";
   }

   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Go to the documentation of this file.";
   }

   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Precondition";
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
      return "Initial value:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "code";
   }

   virtual QByteArray trGraphicalHierarchy() {
      return "Graphical Class Hierarchy";
   }

   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Go to the graphical class hierarchy";
   }

   virtual QByteArray trGotoTextualHierarchy() {
      return "Go to the textual class hierarchy";
   }

   virtual QByteArray trPageIndex() {
      return "Page Index";
   }

   virtual QByteArray trNote() {
      return "Note";
   }
   virtual QByteArray trPublicTypes() {
      return "Public Types";
   }

   virtual QByteArray trPublicAttribs() {
      if (Config::getBool("optimize-c")) {
         return "Data Fields";
      } else {
         return "Public Attributes";
      }
   }

   virtual QByteArray trStaticPublicAttribs() {
      return "Static Public Attributes";
   }

   virtual QByteArray trProtectedTypes() {
      return "Protected Types";
   }

   virtual QByteArray trProtectedAttribs() {
      return "Protected Attributes";
   }

   virtual QByteArray trStaticProtectedAttribs() {
      return "Static Protected Attributes";
   }

   virtual QByteArray trPrivateTypes() {
      return "Private Types";
   }

   virtual QByteArray trPrivateAttribs() {
      return "Private Attributes";
   }

   virtual QByteArray trStaticPrivateAttribs() {
      return "Static Private Attributes";
   }

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "Todo";
   }

   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Todo List";
   }

   virtual QByteArray trReferencedBy() {
      return "Referenced by";
   }

   virtual QByteArray trRemarks() {
      return "Remarks";
   }

   virtual QByteArray trAttention() {
      return "Attention";
   }

   virtual QByteArray trInclByDepGraph() {
      return "This graph shows which files directly or "
             "indirectly include this file:";
   }
   virtual QByteArray trSince() {
      return "Since";
   }

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Graph Legend";
   }

   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "This page explains how to interpret the graphs which are generated by DoxyPress.<p>\n"
         "Consider the following example:\n"
         "\\code\n"
         "/*! Invisible class because of truncation */\n"
         "class Invisible { };\n\n"
         "/*! Truncated class, inheritance relation is hidden */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Class not documented with DoxyPress comments */\n"
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
         "This will result in the following graph:"
         "<p><center><img alt=\"\" src=\"graph_legend." + Config::getEnum("dot-image-format").toUtf8() + "\"></center></p>\n"
         "<p>\n"
         "The boxes in the above graph have the following meaning:\n"
         "</p>\n"
         "<ul>\n"
         "<li>%A filled gray box represents the struct or class for which the "
         "graph is generated.</li>\n"
         "<li>%A box with a black border denotes a documented struct or class.</li>\n"
         "<li>%A box with a grey border denotes an undocumented struct or class.</li>\n"
         "<li>%A box with a red border denotes a documented struct or class for"
         "which not all inheritance/containment relations are shown. %A graph is "
         "truncated if it does not fit within the specified boundaries.</li>\n"
         "</ul>\n"
         "<p>\n"
         "The arrows have the following meaning:\n"
         "</p>\n"
         "<ul>\n"
         "<li>%A dark blue arrow is used to visualize a public inheritance "
         "relation between two classes.</li>\n"
         "<li>%A dark green arrow is used for protected inheritance.</li>\n"
         "<li>%A dark red arrow is used for private inheritance.</li>\n"
         "<li>%A purple dashed arrow is used if a class is contained or used "
         "by another class. The arrow is labeled with the variable(s) "
         "through which the pointed class or struct is accessible.</li>\n"
         "<li>%A yellow dashed arrow denotes a relation between a template instance and "
         "the template class it was instantiated from. The arrow is labeled with "
         "the template parameters of the instance.</li>\n"
         "</ul>\n";
   }

   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "legend";
   }

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "Test";
   }

   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "Test List";
   }

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Properties";
   }

   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Property Documentation";
   }
  
   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config::getBool("optimize-c")) {
         return "Data Structures";
      } else {
         return "Classes";
      }
   }

   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Package " + name;
   }

   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Package List";
   }

   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Here are the packages with brief descriptions (if available):";
   }

   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Packages";
   }

   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Value:";
   }

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Bug";
   }

   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Bug List";
   }
  
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
      QByteArray result((first_capital ? "Class" : "class"));
      if (!singular) {
         result += "es";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "File" : "file"));
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
      QByteArray result((first_capital ? "Module" : "module"));
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
      QByteArray result((first_capital ? "Global" : "global"));
      if (! singular) {
         result += "s";
      }
      return result;
   }

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Author" : "author"));
      if (! singular) {
         result += "s";
      }
      return result;
   }

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "References";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Implements " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Implemented in " + trWriteList(numEntries) + ".";
   }

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Table of Contents";
   }
  
   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Deprecated List";
   }

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Events";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Event Documentation";
   }

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Package Types";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Package Functions";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Static Package Functions";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Package Attributes";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Static Package Attributes";
   }   

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "All";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Here is the call graph for this function:";
   }

    /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Search Results";
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
         return "Sorry, no documents matching your query.";
      } else if (numDocuments == 1) {
         return "Found <b>1</b> document matching your query.";
      } else {
         return "Found <b>$num</b> documents matching your query. "
                "Showing best matches first.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Matches:";
   }


   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return filename + " Source File";
   }
 
   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Directory Hierarchy";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Directory Documentation";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Directories";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "This directory hierarchy is sorted roughly, "
             "but not completely, alphabetically:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " Directory Reference";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Director" : "director"));
      if (singular) {
         result += "y";
      } else {
         result += "ies";
      }
      return result;
   }
 
   /*! This text is added to the documentation when the \\overload command
    *  is used for a overloaded function.
    */
   virtual QByteArray trOverloadText() {
      return "This is an overloaded member function, "
             "provided for convenience. It differs from the above "
             "function only in what argument(s) it accepts.";
   }
  

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Here is the caller graph for this function:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Enumerator Documentation";
   }

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Member Function/Subroutine Documentation";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Data Types List";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Data Fields";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Here are the data types with brief descriptions:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Here is a list of all ";
      if (!extractAll) {
         result += "documented ";
      }
      result += "data types members";
      result += " with links to ";
      if (!extractAll) {
         result += "the data structure documentation for each member";
      } else {
         result += "the data types they belong to:";
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
      return "Data Type Documentation";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Functions/Subroutines";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Function/Subroutine Documentation";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Data Types";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Modules List";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Here is a list of all ";
      if (!extractAll) {
         result += "documented ";
      }
      result += "modules with brief descriptions:";
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
      result += " Reference";
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Module Reference";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Module Members";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Here is a list of all ";
      if (!extractAll) {
         result += "documented ";
      }
      result += "module members with links to ";
      if (extractAll) {
         result += "the module documentation for each member:";
      } else {
         result += "the modules they belong to:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Modules Index";
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
      QByteArray result = (QByteArray)"The documentation for this ";
      switch (compType) {
         case ClassDef::Class:
            result += "module";
            break;
         case ClassDef::Struct:
            result += "type";
            break;
         case ClassDef::Union:
            result += "union";
            break;
         case ClassDef::Interface:
            result += "interface";
            break;
         case ClassDef::Protocol:
            result += "protocol";
            break;
         case ClassDef::Category:
            result += "category";
            break;
         case ClassDef::Exception:
            result += "exception";
            break;
         default:
            break;
      }
      result += " was generated from the following file";
      if (single) {
         result += ":";
      } else {
         result += "s:";
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
      QByteArray result((first_capital ? "Subprogram" : "subprogram"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Type Constraints";
   }
   
   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return QByteArray(name) + " Relation";
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Loading...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Global Namespace";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Searching...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "No Matches";
   } 

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
      return (QByteArray)"Includes file in " + name;
   }
   
   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Bibliography";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Directory dependency graph for ") + name + ":";
   }
  
   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "detail level";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Template Parameters";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "and " + number + " more...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "The documentation for this enum was generated from the following file";
      if (!single) {
         result += "s";
      }
      result += ":";
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return QByteArray(name) + " Enum Reference";
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " inherited from " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Additional Inherited Members";
   }
  
   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      QByteArray opt = enable ? "enable" : "disable";
      return "click to " + opt + " panel synchronisation";
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "Provided by category @1.";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "Extends class @1.";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "Class Methods";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "Instance Methods";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "Method Documentation";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      return "Design Overview";
   }
   
   /** old style UNO IDL services: implemented interfaces */
   virtual QByteArray trInterfaces() {
      return "Exported Interfaces";
   }

   /** old style UNO IDL services: inherited services */
   virtual QByteArray trServices() {
      return "Included Services";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroups() {
      return "Constant Groups";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroupReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Constant Group Reference";
      return result;
   }

   /** UNO IDL service page title */
   virtual QByteArray trServiceReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " Service Reference";
      return result;
   }

   /** UNO IDL singleton page title */
   virtual QByteArray trSingletonReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " Singleton Reference";
      return result;
   }

   /** UNO IDL service page */
   virtual QByteArray trServiceGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"The documentation for this service "
                          "was generated from the following file";
      if (single) {
         result += ":";
      } else {
         result += "s:";
      }
      return result;
   }

   /** UNO IDL singleton page */
   virtual QByteArray trSingletonGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"The documentation for this singleton "
                          "was generated from the following file";
      if (single) {
         result += ":";
      } else {
         result += "s:";
      }
      return result;
   }
   
};

#endif
