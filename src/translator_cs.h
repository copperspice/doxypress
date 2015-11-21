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
    * The English LaTeX does not use such commands.  Because of this
    * the empty string is returned in this implementation.
    */
   virtual QString latexLanguageSupportCommand() {
      return QCoreApplication::translate("latexSupport", "remove-value");
   }
   
   /*! used in the compound documentation before a list of related functions. */
   virtual QString trRelatedFunctions() {
      return QCoreApplication::translate("doxy-text", "Related Functions");
   }

   /*! subscript for the related functions. */
   virtual QString trRelatedSubscript() {
      return QCoreApplication::translate("doxy-text", "These are not member functions");
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QString trDetailedDescription() {
      return QCoreApplication::translate("doxy-text", "Detailed Description");
   }

   /*! header that is put before the list of typedefs. */
   virtual QString trMemberTypedefDocumentation() {
      return QCoreApplication::translate("doxy-text", "Member Typedef Documentation");
   }

   /*! header that is put before the list of enumerations. */
   virtual QString trMemberEnumerationDocumentation() {
      return QCoreApplication::translate("doxy-text", "Member Enumeration Documentation");
   }

   /*! header that is put before the list of member functions. */
   virtual QString trMemberFunctionDocumentation() {
      return QCoreApplication::translate("doxy-text", "Member Function Documentation");
   }

   /*! header that is put before the list of member attributes. */
   virtual QString trMemberDataDocumentation() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Field Documentation");
      } else {
         return QCoreApplication::translate("doxy-text", "Member Data Documentation");
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QString trMore() {
      return QCoreApplication::translate("doxy-text", "More...");;
   }

   /*! put in the class documentation */
   virtual QString trListOfAllMembers() {
      return QCoreApplication::translate("doxy-text", "List of all members");
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QString trMemberList() {
      return QCoreApplication::translate("doxy-text", "Member List");
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QString trThisIsTheListOfAllMembers() {
      return QCoreApplication::translate("doxy-text", "This is the complete list of members for ");
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QString trIncludingInheritedMembers() {
      return QCoreApplication::translate("doxy-text", ", including all inherited members.");
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QString trGeneratedAutomatically(const QString &s) {        
      if (! s.isEmpty() ) {      
         return QCoreApplication::translate("doxy-text", "Generated automatically by DoxyPress for %1 from the source code.").arg(s);

      } else {
         return QCoreApplication::translate("doxy-text", "Generated automatically by DoxyPress from the source code.");

      }
   }

   /*! put after an enum name in the list of all members */
   virtual QString trEnumName() {
      return QCoreApplication::translate("doxy-text", "enum name");;
   }

   /*! put after an enum value in the list of all members */
   virtual QString trEnumValue() {
      return QCoreApplication::translate("doxy-text", "enum value");
   }

   /*! put after an undocumented member in the list of all members */
   virtual QString trDefinedIn() {
      return QCoreApplication::translate("doxy-text", "defined in");
   }

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QString trModules() {
      return QCoreApplication::translate("doxy-text", "Modules");;
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QString trClassHierarchy() {
      return QCoreApplication::translate("doxy-text", "Class Hierarchy");
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QString trCompoundList() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Structures");
      } else {
         return QCoreApplication::translate("doxy-text", "Class List");
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QString trFileList() {
      return QCoreApplication::translate("doxy-text", "File List");
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QString trCompoundMembers() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Fields");
      } else {
         return QCoreApplication::translate("doxy-text", "Class Members");
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QString trFileMembers() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Globals");
      } else {
         return QCoreApplication::translate("doxy-text", "File Members");
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QString trRelatedPages() {
      return QCoreApplication::translate("doxy-text", "Related Pages");
   }

   /*! This is put above each page as a link to all examples. */
   virtual QString trExamples() {
      return QCoreApplication::translate("doxy-text", "Examples");
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QString trSearch() {
      return QCoreApplication::translate("doxy-text", "Search");
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QString trClassHierarchyDescription() {
      return QCoreApplication::translate("doxy-text", "This inheritance list is sorted roughly, "
                "but not completely, alphabetically:");
   }

   /*! This is an introduction to the list with all files. */
   virtual QString trFileListDescription(bool extractAll) {   
      if (extractAll) {
         return QCoreApplication::translate("doxy-text", "Here is a list of all files with brief "
                  "descriptions:");
      } else {
         return QCoreApplication::translate("doxy-text", "Here is a list of all documented files " 
                  "with brief descriptions:");
      }    
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QString trCompoundListDescription() {

      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", 
               "Here are the data structures with brief descriptions:");

      } else {
         return QCoreApplication::translate("doxy-text", "Here are the classes, structs, "
                "unions and interfaces with brief descriptions:");
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QString trCompoundMembersDescription(bool extractAll) { 
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
   virtual QString trFileMembersDescription(bool extractAll) {      
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

   /*! This is an introduction to the page with the list of all examples */
   virtual QString trExamplesDescription() {
      return QCoreApplication::translate("doxy-text", "Here is a list of all examples:");
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QString trRelatedPagesDescription() {
      return QCoreApplication::translate("doxy-text", "Here is a list of all related documentation pages:");
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QString trModulesDescription() {
      return QCoreApplication::translate("doxy-text", "Here is a list of all modules:");
   }

   // index titles (the project name is prepended for these)            

   /*! This is used in HTML as the title of index.html. */
   virtual QString trDocumentation() {
      return QCoreApplication::translate("doxy-text", "Documentation");
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QString trModuleIndex() {
      return QCoreApplication::translate("doxy-text", "Module Index");
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QString trHierarchicalIndex() {
      return QCoreApplication::translate("doxy-text", "Hierarchical Index");
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QString trCompoundIndex() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Structure Index");
      } else {
         return QCoreApplication::translate("doxy-text", "Class Index");
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the list of all files.
    */
   virtual QString trFileIndex() {
      return QCoreApplication::translate("doxy-text", "File Index");
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QString trModuleDocumentation() {
      return QCoreApplication::translate("doxy-text", "Module Documentation");
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QString trClassDocumentation() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Structure Documentation");
      } else {
         return QCoreApplication::translate("doxy-text", "Class Documentation");
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QString trFileDocumentation() {
      return QCoreApplication::translate("doxy-text", "File Documentation");
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QString trExampleDocumentation() {
      return QCoreApplication::translate("doxy-text", "Example Documentation");
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QString trPageDocumentation() {
      return QCoreApplication::translate("doxy-text", "Page Documentation");
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QString trReferenceManual() {
      return QCoreApplication::translate("doxy-text", "Reference Manual");
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QString trDefines() {
      return QCoreApplication::translate("doxy-text", "Macros");
   }

   virtual QString trPublicTypedefs() {
      return QCoreApplication::translate("doxy-text", "Public Typedefs");
   }

   virtual QString trProtectedTypedefs() {
      return QCoreApplication::translate("doxy-text", "Protected Typedefs");
   }

   virtual QString trPrivateTypedefs() {
      return QCoreApplication::translate("doxy-text", "Private Typedefs");
   }

   /*! This is used in the documentation of a file as a header before the list of typedefs
    */
   virtual QString trTypedefs() {
      return QCoreApplication::translate("doxy-text", "Typedefs");
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QString trEnumerations() {
      return QCoreApplication::translate("doxy-text", "Enums");
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QString trFunctions() {
      return QCoreApplication::translate("doxy-text", "Functions");
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QString trVariables() {
      return QCoreApplication::translate("doxy-text", "Variables");
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QString trEnumerationValues() {
      return QCoreApplication::translate("doxy-text", "Enum Values");
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QString trDefineDocumentation() {
      return QCoreApplication::translate("doxy-text", "Macro Definition Documentation");
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QString trTypedefDocumentation() {
      return QCoreApplication::translate("doxy-text", "Typedef Documentation");
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QString trEnumerationTypeDocumentation() {
      return QCoreApplication::translate("doxy-text", "Enumeration Type Documentation");
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QString trFunctionDocumentation() {
      return QCoreApplication::translate("doxy-text", "Function Documentation");
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QString trVariableDocumentation() {
      return QCoreApplication::translate("doxy-text", "Variable Documentation");
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QString trCompounds() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Structures");
      } else {
         return QCoreApplication::translate("doxy-text", "Classes");
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QString trGeneratedAt(const QString &date, const QString &projName) {  

      if (projName.isEmpty()) {
         return QCoreApplication::translate("doxy-text", "Generated on %1 &nbsp; by").arg(date);         
      } else  {
         return QCoreApplication::translate("doxy-text", "Generated on %1 for %2 &nbsp; by").arg(date).arg(projName);         
      }      
   }

   /*! this text is put before a class diagram */
   virtual QString trClassDiagram(const QString &clName) {
      return QCoreApplication::translate("doxy-text", "Inheritance diagram for %1:").arg(clName);
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QString trForInternalUseOnly() {
      return QCoreApplication::translate("doxy-text", "For internal use only.");
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QString trWarning() {
      return QCoreApplication::translate("doxy-text", "Warning");
   }

   /*! this text is generated when the \\version command is used. */
   virtual QString trVersion() {
      return QCoreApplication::translate("doxy-text", "Version");
   }

   /*! this text is generated when the \\date command is used. */
   virtual QString trDate() {
      return QCoreApplication::translate("doxy-text", "Date");
   }

   /*! this text is generated when the \\return command is used. */
   virtual QString trReturns() {
      return QCoreApplication::translate("doxy-text", "Returns");
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QString trSeeAlso() {
      return QCoreApplication::translate("doxy-text", "See also");
   }

   /*! this text is generated when the \\param command is used. */
   virtual QString trParameters() {
      return QCoreApplication::translate("doxy-text", "Parameters");
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QString trExceptions() {
      return QCoreApplication::translate("doxy-text", "Exceptions");
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QString trGeneratedBy() {
      return QCoreApplication::translate("doxy-text", "Generated by");
   }

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QString trNamespaceList() {
      return QCoreApplication::translate("doxy-text", "Namespace List");
   }

   /*! used as an introduction to the namespace list */   
   virtual QString trNamespaceListDescription(bool extractAll) {      

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
   virtual QString trFriends() {
      return QCoreApplication::translate("doxy-text", "Friends");
   }

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QString trRelatedFunctionDocumentation() {
      return QCoreApplication::translate("doxy-text", "Friends And Related Function Documentation");
   }  

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QString trCompoundReference(const QString &clName, ClassDef::CompoundType compType, bool isTemplate) {                                                                                
      QString result;
      
      switch (compType) {
         case ClassDef::Class:            
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Class Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Class Reference").arg(clName);
            }                
            break;

         case ClassDef::Struct:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Struct Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Struct Reference").arg(clName);
            }       
            break;

         case ClassDef::Union:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Union Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Union Reference").arg(clName);
            }         
            break;

         case ClassDef::Interface:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Interface Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Interface Reference").arg(clName);
            }         
            break;

         case ClassDef::Protocol:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Protocol Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Protocol Reference").arg(clName);
            }         
            break;

         case ClassDef::Category:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Category Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Category Reference").arg(clName);
            }         
            break;

         case ClassDef::Exception:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Exception Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Exception Reference").arg(clName);
            }                    
            break;

         default:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Reference").arg(clName);
            }         
            break;
      }
    
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QString trFileReference(const QString &fileName) {
      return QCoreApplication::translate("doxy-text", "%1 File Reference").arg(fileName);
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QString trNamespaceReference(const QString &name) {   
      return QCoreApplication::translate("doxy-text", "%1 Namespace Reference").arg(name);
   }

   virtual QString trPublicMembers() {
      return QCoreApplication::translate("doxy-text", "Public Member Functions");
   }
  
   virtual QString trPublicSignals() {
      return QCoreApplication::translate("doxy-text", "Public Signals");
   }
   virtual QString trProtectedSignals() {
      return QCoreApplication::translate("doxy-text", "Protected Signals");
   }
   virtual QString trPrivateSignals() {
      return QCoreApplication::translate("doxy-text", "Private Signals");
   }

   virtual QString trStaticPublicMembers() {
      return QCoreApplication::translate("doxy-text", "Static Public Member Functions");
   }
   virtual QString trProtectedMembers() {
      return QCoreApplication::translate("doxy-text", "Protected Member Functions");
   }
   virtual QString trPublicSlots() {
      return QCoreApplication::translate("doxy-text", "Public Slots");
   }
   virtual QString trProtectedSlots() {
      return QCoreApplication::translate("doxy-text", "Protected Slots");
   }
   virtual QString trStaticProtectedMembers() {
      return QCoreApplication::translate("doxy-text", "Static Protected Member Functions");
   }
   virtual QString trPrivateMembers() {
      return QCoreApplication::translate("doxy-text", "Private Member Functions");
   }
   virtual QString trPrivateSlots() {
      return QCoreApplication::translate("doxy-text", "Private Slots");
   }
   virtual QString trStaticPrivateMembers() {
      return QCoreApplication::translate("doxy-text", "Static Private Member Functions");
   }

   /*! this function is used to produce a comma-separated list of items.
    *  use generateMarker(i) to indicate where item i should be put.
    */
   virtual QString trWriteList(int numEntries) {                                  
      QString result;

      // the inherits list contains `numEntries' classes
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

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QString trInheritsList(int numEntries) {      
      return QCoreApplication::translate("doxy-text", "Inherits %1.").arg( trWriteList(numEntries));
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QString trInheritedByList(int numEntries) {
      return QCoreApplication::translate("doxy-text", "Inherited by %1.").arg( trWriteList(numEntries));
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QString trReimplementedFromList(int numEntries) {
      return QCoreApplication::translate("doxy-text", "Reimplemented from %1.").arg( trWriteList(numEntries));
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QString trReimplementedInList(int numEntries) {
      return QCoreApplication::translate("doxy-text", "Reimplemented in %1.").arg( trWriteList(numEntries));
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QString trNamespaceMembers() {
      return QCoreApplication::translate("doxy-text", "Namespace Members");
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QString trNamespaceMemberDescription(bool extractAll) {         
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
   virtual QString trNamespaceIndex() {
      return QCoreApplication::translate("doxy-text", "Namespace Index");
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QString trNamespaceDocumentation() {
      return QCoreApplication::translate("doxy-text", "Namespace Documentation");
   }

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QString trNamespaces() {
      return QCoreApplication::translate("doxy-text", "Namespaces");
   }

   /*! This is put at the bottom of a class documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QString trGeneratedFromFiles(ClassDef::CompoundType compType, bool single) {
      QString result;
      
      switch (compType) {

         case ClassDef::Class:
            if (single) {
              result = QCoreApplication::translate("doxy-text", "The documentation for this class "
                     " was generated from the following file:");
            } else {
              result = QCoreApplication::translate("doxy-text", "The documentation for this class "
                     " was generated from the following files:");
            }
            break;

         case ClassDef::Struct:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this struct "
                     " was generated from the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this struct "
                     " was generated from the following files:");
            }
            break;

         case ClassDef::Union:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this union "
                     " was generated from the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this union "
                     " was generated from the following files:");
            }
            break;

         case ClassDef::Interface:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this interface "
                     " was generated from the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this interface "
                     " was generated from the following files:");
            }           
            break;

         case ClassDef::Protocol:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this protocol "
                     " was generated from the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this protocol "
                     " was generated from the following files:");
            }
            break;

         case ClassDef::Category:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this category "
                     " was generated from the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this category "
                     " was generated from the following files:");
            }
            break;

         case ClassDef::Exception:

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
   virtual QString trReturnValues() {
      return QCoreApplication::translate("doxy-text", "Return Values");
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QString trMainPage() {
      return QCoreApplication::translate("doxy-text", "Main Page");
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QString trPageAbbreviation() {
      return QCoreApplication::translate("doxy-text", "p,");
   }
 
   virtual QString trDefinedAtLineInSourceFile() {        
      return QCoreApplication::translate("doxy-text", "Definition at line @0 of file @1.");          
   }

   virtual QString trDefinedInSourceFile() {               
      return QCoreApplication::translate("doxy-text", "Definition in file @0.");
   }

   virtual QString trDeprecated() {
      return QCoreApplication::translate("doxy-text", "Deprecated");
   }

   /*! this text is put before a collaboration diagram */
   virtual QString trCollaborationDiagram(const QString &clName) {
      return QCoreApplication::translate("doxy-text", "Collaboration diagram for %1:").arg(clName);
   }

   /*! this text is put before an include dependency graph */
   virtual QString trInclDepGraph(const QString &fName) {
      return QCoreApplication::translate("doxy-text", "Include dependency graph for %1:").arg(fName);
   }  

   /*! header that is put before the list of constructor/destructors. */
   virtual QString trConstructorDocumentation() {
      return QCoreApplication::translate("doxy-text", "Constructor & Destructor Documentation");
   }

   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QString trGotoSourceCode() {
      return QCoreApplication::translate("doxy-text", "Go to the source code of this file.");
   }

   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QString trGotoDocumentation() {
      return QCoreApplication::translate("doxy-text", "Go to the documentation of this file.");
   }

   /*! Text for the \\pre command */
   virtual QString trPrecondition() {
      return QCoreApplication::translate("doxy-text", "Precondition");
   }

   /*! Text for the \\post command */
   virtual QString trPostcondition() {
      return QCoreApplication::translate("doxy-text", "Postcondition");
   }
   /*! Text for the \\invariant command */
   virtual QString trInvariant() {
      return QCoreApplication::translate("doxy-text", "Invariant");
   }

   /*! Text shown before a multi-line variable/enum initialization */
   virtual QString trInitialValue() {
      return QCoreApplication::translate("doxy-text", "Initial value:");
   }
   /*! Text used the source code in the file index */
   virtual QString trCode() {
      return QCoreApplication::translate("doxy-text", "code");
   }

   virtual QString trGraphicalHierarchy() {
      return QCoreApplication::translate("doxy-text", "Graphical Class Hierarchy");
   }

   virtual QString trGotoGraphicalHierarchy() {
      return QCoreApplication::translate("doxy-text", "Go to the graphical class hierarchy");
   }

   virtual QString trGotoTextualHierarchy() {
      return QCoreApplication::translate("doxy-text", "Go to the textual class hierarchy");
   }

   virtual QString trPageIndex() {
      return QCoreApplication::translate("doxy-text", "Page Index");
   }

   virtual QString trNote() {
      return QCoreApplication::translate("doxy-text", "Note");
   }
   virtual QString trPublicTypes() {
      return QCoreApplication::translate("doxy-text", "Public Types");
   }

   virtual QString trPublicAttribs() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Fields");
      } else {
         return QCoreApplication::translate("doxy-text", "Public Attributes");
      }
   }

   virtual QString trStaticPublicAttribs() {
      return QCoreApplication::translate("doxy-text", "Static Public Attributes");
   }

   virtual QString trProtectedTypes() {
      return QCoreApplication::translate("doxy-text", "Protected Types");
   }

   virtual QString trProtectedAttribs() {
      return QCoreApplication::translate("doxy-text", "Protected Attributes");
   }

   virtual QString trStaticProtectedAttribs() {
      return QCoreApplication::translate("doxy-text", "Static Protected Attributes");
   }

   virtual QString trPrivateTypes() {
      return QCoreApplication::translate("doxy-text", "Private Types");
   }

   virtual QString trPrivateAttribs() {
      return QCoreApplication::translate("doxy-text", "Private Attributes");
   }

   virtual QString trStaticPrivateAttribs() {
      return QCoreApplication::translate("doxy-text", "Static Private Attributes");
   }

   /*! Used as a marker that is put before a \\todo item */
   virtual QString trTodo() {
      return QCoreApplication::translate("doxy-text", "Todo");
   }

   /*! Used as the header of the todo list */
   virtual QString trTodoList() {
      return QCoreApplication::translate("doxy-text", "Todo List");
   }

   virtual QString trReferencedBy() {
      return QCoreApplication::translate("doxy-text", "Referenced by");
   }

   virtual QString trRemarks() {
      return QCoreApplication::translate("doxy-text", "Remarks");
   }

   virtual QString trAttention() {
      return QCoreApplication::translate("doxy-text", "Attention");
   }

   virtual QString trInclByDepGraph() {
      return QCoreApplication::translate("doxy-text", "This graph shows which files "
                  "directly or indirectly include this file");
   }

   virtual QString trSince() {
      return QCoreApplication::translate("doxy-text", "Since");
   }

   /*! title of the graph legend page */
   virtual QString trLegendTitle() {
      return QCoreApplication::translate("doxy-text", "Graph Legend");
   }

   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QString trLegendDocs(const QString &dotFormat) {     
      
      return QCoreApplication::translate("doxy-text", 
         "This page explains how to interpret the graphs which are generated by DoxyPress.\n\n"
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
         "\n\n"
         "<center><img alt=\"\" src=\"graph_legend.%1\"></center>\n"         
         "\n"
         "The boxes in the above graph have the following meaning:\n\n"
         "<ul>\n"
         "<li>%A filled gray box represents the struct or class for which the "
         "graph is generated.</li>\n"
         "<li>%A box with a black border denotes a documented struct or class.</li>\n"
         "<li>%A box with a grey border denotes an undocumented struct or class.</li>\n"
         "<li>%A box with a red border denotes a documented struct or class for "
         "which not all inheritance/containment relations are shown. %A graph is "
         "truncated if it does not fit within the specified boundaries.</li>\n"
         "</ul>\n"
         "\n"
         "The arrows have the following meaning:\n"
         "\n"
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
         "</ul>\n").arg(dotFormat);
   }

   /*! text for the link to the legend page */
   virtual QString trLegend() {
      return QCoreApplication::translate("doxy-text", "legend");
   }

   /*! Used as a marker that is put before a test item */
   virtual QString trTest() {
      return QCoreApplication::translate("doxy-text", "Test");
   }

   /*! Used as the header of the test list */
   virtual QString trTestList() {
      return QCoreApplication::translate("doxy-text", "Test List");
   }

   /*! Used as a section header for IDL properties */
   virtual QString trProperties() {
      return QCoreApplication::translate("doxy-text", "Properties");
   }

   /*! Used as a section header for IDL property documentation */
   virtual QString trPropertyDocumentation() {
      return QCoreApplication::translate("doxy-text", "Property Documentation");
   }
  
   /*! Used for Java classes in the summary section of Java packages */
   virtual QString trClasses() {
      if (Config::getBool("optimize-c")) {
         return QCoreApplication::translate("doxy-text", "Data Structures");
      } else {
         return QCoreApplication::translate("doxy-text", "Classes");
      }
   }

   /*! Used as the title of a Java package */
   virtual QString trPackage(const QString &name) {      
      return QCoreApplication::translate("doxy-text", "Package %1").arg(name);
   }

   /*! Title of the package index page */
   virtual QString trPackageList() {
      return QCoreApplication::translate("doxy-text", "Package List");
   }

   /*! The description of the package index page */
   virtual QString trPackageListDescription() {
      return QCoreApplication::translate("doxy-text", "Here are the packages with " 
                  "brief descriptions (if available):");
   }

   /*! The link name in the Quick links header for each page */
   virtual QString trPackages() {
      return QCoreApplication::translate("doxy-text", "Packages");
   }

   /*! Text shown before a multi-line define */
   virtual QString trDefineValue() {
      return QCoreApplication::translate("doxy-text", "Value:");
   }

   /*! Used as a marker that is put before a \\bug item */
   virtual QString trBug() {
      return QCoreApplication::translate("doxy-text", "Bug");
   }

   /*! Used as the header of the bug list */
   virtual QString trBugList() {
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
   virtual QString trRTFansicp() {        
      return QCoreApplication::translate("doxy-rtf", "1252", "trRTFansicp");
   }

   /*! Used as ansicpg for RTF fcharset
    *  \see trRTFansicp() for a table of possible values.
    */
   virtual QString trRTFCharSet() {
      return QCoreApplication::translate("doxy-rtf", "0", "trRTFCharSet");
   }

   /*! Used as header RTF general index */
   virtual QString trRTFGeneralIndex() {
      return QCoreApplication::translate("doxy-text", "Index");
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names of the category.
    */
   virtual QString trClass(bool first_capital, bool singular) {        
      if (first_capital) {      

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Class");
         } else  {    
            return QCoreApplication::translate("doxy-text", "Classes"); 
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "class");
         } else  {     
            return QCoreApplication::translate("doxy-text", "classes");
         }
      }     
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QString trFile(bool first_capital, bool singular) {            
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
   virtual QString trNamespace(bool first_capital, bool singular) {
      if (first_capital) {      

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Namespace");
         } else  {    
            return QCoreApplication::translate("doxy-text", "Namespaces"); 
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "namespace");
         } else  {     
            return QCoreApplication::translate("doxy-text", "namespaces");
         }
      }     
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names of the category.
    */
   virtual QString trGroup(bool first_capital, bool singular) {
      if (first_capital) {      

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Module");
         } else  {   
            return QCoreApplication::translate("doxy-text", "Modules"); 
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "module");
         } else  {     
            return QCoreApplication::translate("doxy-text", "modules");
         }
      }     
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names of the category.
    */
   virtual QString trPage(bool first_capital, bool singular) {
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
   virtual QString trMember(bool first_capital, bool singular) {
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
   virtual QString trGlobal(bool first_capital, bool singular) {
      if (first_capital) {      

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Global");
         } else  {  
            return QCoreApplication::translate("doxy-text", "Globals"); 
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "global");
         } else {   
            return QCoreApplication::translate("doxy-text", "globals");
         }
      }
   }     

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QString trAuthor(bool first_capital, bool singular) {
      if (first_capital) {      

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Author");
         } else  {    
            return QCoreApplication::translate("doxy-text", "Authors"); 
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "author");
         } else  {     
            return QCoreApplication::translate("doxy-text", "Authors");
         }
      }
   }     

   /*! This text is put before the list of members referenced by a member
    */
   virtual QString trReferences() {
      return QCoreApplication::translate("doxy-text", "References");
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QString trImplementedFromList(int numEntries) { 
     return QCoreApplication::translate("doxy-text", "Implements %1.").arg(trWriteList(numEntries));     
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QString trImplementedInList(int numEntries) {
      return QCoreApplication::translate("doxy-text", "Implemented %1.").arg( 
                  trWriteList(numEntries));
   }

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QString trRTFTableOfContents() {
      return QCoreApplication::translate("doxy-text", "Table of Contents");
   }
  
   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QString trDeprecatedList() {
      return QCoreApplication::translate("doxy-text", "Deprecated List");
   }

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QString trEvents() {
      return QCoreApplication::translate("doxy-text", "Events");
   }

   /*! Header used for the documentation section of a class' events. */
   virtual QString trEventDocumentation() {
      return QCoreApplication::translate("doxy-text", "Event Documentation");
   }

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QString trPackageTypes() {
      return QCoreApplication::translate("doxy-text", "Package Types");
   }

   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QString trPackageMembers() {
      return QCoreApplication::translate("doxy-text", "Package Functions");
   }

   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QString trStaticPackageMembers() {
      return QCoreApplication::translate("doxy-text", "Static Package Functions");
   }

   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QString trPackageAttribs() {
      return QCoreApplication::translate("doxy-text", "Package Attributes");
   }

   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QString trStaticPackageAttribs() {
      return QCoreApplication::translate("doxy-text", "Static Package Attributes");
   }   

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QString trAll() {
      return QCoreApplication::translate("doxy-text", "All");
   }

   /*! Put in front of the call graph for a function. */
   virtual QString trCallGraph() {
      return QCoreApplication::translate("doxy-text", "Here is the call graph for this function:");
   }

    /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QString trSearchResultsTitle() {
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
   virtual QString trSearchResults(int numDocuments) {            
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
   virtual QString trSearchMatches() {
      return QCoreApplication::translate("doxy-text", "Matches:");
   }

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QString trSourceFile(const QString &filename) {
      return QCoreApplication::translate("doxy-text", "%1 Source File").arg(filename);
   }
 
   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QString trDirIndex() {
      return QCoreApplication::translate("doxy-text", "Directory Hierarchy");
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QString trDirDocumentation() {
      return QCoreApplication::translate("doxy-text", "Directory Documentation");
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   virtual QString trDirectories() {
      return QCoreApplication::translate("doxy-text", "Directories");
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QString trDirDescription() {
      return QCoreApplication::translate("doxy-text", "This directory hierarchy is sorted roughly, "
             "but not completely, alphabetically:");
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QString trDirReference(const QString &dirName) {    
      return QCoreApplication::translate("doxy-text", "%1 Directory Reference").arg(dirName);
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QString trDir(bool first_capital, bool singular) {
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
   virtual QString trOverloadText() {
      return QCoreApplication::translate("doxy-text",  "This is an overloaded member function, "
             "provided for convenience. It differs from the above "
             "function only in what argument(s) it accepts.");
   }
  
   /*! This is used to introduce a caller (or called-by) graph */
   virtual QString trCallerGraph() {
      return QCoreApplication::translate("doxy-text", "Here is the caller graph for this function:");
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QString trEnumerationValueDocumentation() {
      return QCoreApplication::translate("doxy-text", "Enumerator Documentation");
   }

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QString trMemberFunctionDocumentationFortran() {
      return QCoreApplication::translate("doxy-text", "Member Function/Subroutine Documentation");
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QString trCompoundListFortran() {
      return QCoreApplication::translate("doxy-text", "Data Types List");
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QString trCompoundMembersFortran() {
      return QCoreApplication::translate("doxy-text", "Data Fields");
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QString trCompoundListDescriptionFortran() {
      return QCoreApplication::translate("doxy-text", "Here are the data types with brief descriptions:");
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QString trCompoundMembersDescriptionFortran(bool extractAll) {
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
   virtual QString trCompoundIndexFortran() {
      return QCoreApplication::translate("doxy-text", "Data Type Index");
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QString trTypeDocumentation() {
      return QCoreApplication::translate("doxy-text", "Data Type Documentation");
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QString trSubprograms() {
      return QCoreApplication::translate("doxy-text", "Functions/Subroutines");
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QString trSubprogramDocumentation() {
      return QCoreApplication::translate("doxy-text", "Function/Subroutine Documentation");
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QString trDataTypes() {
      return QCoreApplication::translate("doxy-text", "Data Types");
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QString trModulesList() {
      return QCoreApplication::translate("doxy-text", "Modules List");
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QString trModulesListDescription(bool extractAll) {      
      if (extractAll) {
         return QCoreApplication::translate("doxy-text", "Here is a list of all modules with brief descriptions:");
      } else {
         return QCoreApplication::translate("doxy-text", "Here is a list of all documented modules with brief descriptions:");
      }
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QString trCompoundReferenceFortran(const QString &clName, ClassDef::CompoundType compType,bool isTemplate) {
      QString result;            

      switch (compType) {
         case ClassDef::Class:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Module Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Module Reference").arg(clName);
            }         
            break;

         case ClassDef::Struct:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Type Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Type Reference").arg(clName);
            }
            break;

         case ClassDef::Union:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Union Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Union Reference").arg(clName);
            }
            break;

         case ClassDef::Interface:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Interface Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Interface Reference").arg(clName);
            }
            break;

         case ClassDef::Protocol:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Protocol Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1  Protocol Reference").arg(clName);
            }
            break;

         case ClassDef::Category:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Category Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Category Reference").arg(clName);
            }
            break;

         case ClassDef::Exception:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Exception Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Exception Reference").arg(clName);
            }
            break;
  
          default:
            if (isTemplate) {
               result = QCoreApplication::translate("doxy-text", "%1 Template Reference").arg(clName);
            } else {
               result = QCoreApplication::translate("doxy-text", "%1 Reference").arg(clName);
            }
            break;
      }
           
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QString trModuleReference(const QString &namespaceName) {      
      return QCoreApplication::translate("doxy-text", "%1 Module Reference").arg(namespaceName);
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QString trModulesMembers() {
      return QCoreApplication::translate("doxy-text", "Module Members");
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QString trModulesMemberDescription(bool extractAll) {       
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
   virtual QString trModulesIndex() {
      return QCoreApplication::translate("doxy-text", "Modules Index");
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names of the category.
    */
   virtual QString trModule(bool first_capital, bool singular) {        

      if (first_capital) {      

         if (singular) {
            return QCoreApplication::translate("doxy-text", "Module");
         } else  {     
            return QCoreApplication::translate("doxy-text", "Modules"); 
         }

      } else {

         if (singular) {
            return QCoreApplication::translate("doxy-text", "module");
         } else  {     
            return QCoreApplication::translate("doxy-text", "modules");
         }
      }   
   }

   /*! This is put at the bottom of a module documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QString trGeneratedFromFilesFortran(ClassDef::CompoundType compType, bool single) {   
      QString result;

      switch (compType) {
         case ClassDef::Class:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this module was generated from "
                     "the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this module was generated from "
                     "the following files:");
            }
            break;
         case ClassDef::Struct:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this type was generated from "
                     "the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this type was generated from "
                     "the following files:");
            } 
            break;
         case ClassDef::Union:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this union was generated from "
                     "the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this union was generated from "
                     "the following files:");
            } 
            break;
         case ClassDef::Interface:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this interface was generated from "
                     "the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this interface was generated from "
                     "the following files:");
            } 
            break;
         case ClassDef::Protocol:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this protocol was generated from "
                     "the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this protocol was generated from "
                     "the following files:");
            }
            break;
         case ClassDef::Category:
            if (single) {
               result = QCoreApplication::translate("doxy-text", "The documentation for this category was generated from "
                     "the following file:");
            } else {
               result = QCoreApplication::translate("doxy-text", "The documentation for this category was generated from "
                     "the following files:");
            } 
            break;
         case ClassDef::Exception:
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
   virtual QString trType(bool first_capital, bool singular) {  
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
   virtual QString trSubprogram(bool first_capital, bool singular) {
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
   virtual QString trTypeConstraints() {
      return QCoreApplication::translate("doxy-text", "Type Constraints");
   }
   
   /*! directory relation for \a name */
   virtual QString trDirRelation(const QString &name) {
      return QCoreApplication::translate("doxy-text", "%1 Relation").arg(name);
   }

   /*! Loading message shown when loading search results */
   virtual QString trLoading() {
      return QCoreApplication::translate("doxy-text", "Loading...");
   }

   /*! Label used for search results in the global namespace */
   virtual QString trGlobalNamespace() {
      return QCoreApplication::translate("doxy-text", "Global Namespace");
   }

   /*! Message shown while searching */
   virtual QString trSearching() {
      return QCoreApplication::translate("doxy-text", "Searching...");
   }

   /*! Text shown when no search results are found */
   virtual QString trNoMatches() {
      return QCoreApplication::translate("doxy-text", "No Matches");
   } 

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QString trFileIn(const QString &name) {      
      return QCoreApplication::translate("doxy-text", "File in %1").arg(name);
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QString trIncludesFileIn(const QString &name) {     
      return QCoreApplication::translate("doxy-text", "Includes file in %1").arg(name);
   }
   
   /*! Header for the page with bibliographic citations */
   virtual QString trCiteReferences() {
      return QCoreApplication::translate("doxy-text", "Bibliography");
   }

   /*! Text for copyright paragraph */
   virtual QString trCopyright() {
      return QCoreApplication::translate("doxy-text", "Copyright");
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QString trDirDepGraph(const QString &name) {
      return QCoreApplication::translate("doxy-text", "Directory dependency graph for %1:").arg(name);
   }
  
   /*! Detail level selector shown for hierarchical indices */
   virtual QString trDetailLevel() {
      return QCoreApplication::translate("doxy-text", "detail level");
   }

   /*! Section header for list of template parameters */
   virtual QString trTemplateParameters() {
      return QCoreApplication::translate("doxy-text", "Template Parameters");
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QString trAndMore(const QString &number) {  
      return QCoreApplication::translate("doxy-text", "and %1 more...").arg(number);
   }

   /*! Used file list for a Java enum */
   virtual QString trEnumGeneratedFromFiles(bool single) {
      if (single) {
         return QCoreApplication::translate("doxy-text", "The documentation for this enum was generated "
                  "from the following file:"); 
      } else {
         return QCoreApplication::translate("doxy-text", "The documentation for this enum was generated "
                  " from the following files:");
      }
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QString trEnumReference(const QString &name) {      
      return QCoreApplication::translate("doxy-text", "%1 Enum Reference").arg(name);
   }

   /*! Used for a section containing inherited members */
   virtual QString trInheritedFrom(const QString &members, const QString &what) {    
      return QCoreApplication::translate("doxy-text", "%1 inherited from %2").arg(members).arg(what);
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QString trAdditionalInheritedMembers() {
      return QCoreApplication::translate("doxy-text", "Additional Inherited Members");
   }
  
   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QString trPanelSyncTooltip(bool enable) {        
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
   virtual QString trProvidedByCategory() {         
      return QCoreApplication::translate("doxy-text", "Provided by category @1.");
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QString trExtendsClass() {     
      return QCoreApplication::translate("doxy-text", "Extends class @1");
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QString trClassMethods() {
      return QCoreApplication::translate("doxy-text", "Class Methods");
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QString trInstanceMethods() {
      return QCoreApplication::translate("doxy-text", "Instance Methods");
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QString trMethodDocumentation() {
      return QCoreApplication::translate("doxy-text", "Method Documentation");
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QString trDesignOverview() {
      return QCoreApplication::translate("doxy-text", "Design Overview");
   }
   
   /** old style UNO IDL services: implemented interfaces */
   virtual QString trInterfaces() {
      return QCoreApplication::translate("doxy-text", "Exported Interfaces");
   }

   /** old style UNO IDL services: inherited services */
   virtual QString trServices() {
      return QCoreApplication::translate("doxy-text", "Included Services");
   }

   /** UNO IDL constant groups */
   virtual QString trConstantGroups() {
      return QCoreApplication::translate("doxy-text", "Constant Groups");
   }

   /** UNO IDL constant groups */
   virtual QString trConstantGroupReference(const QString &namespaceName) {
      return QCoreApplication::translate("doxy-text", "%1 Constant Group Reference").arg(namespaceName);
   }

   /** UNO IDL service page title */
   virtual QString trServiceReference(const QString &sName) {
      return QCoreApplication::translate("doxy-text", "%1 Service Reference").arg(sName);
   }

   /** UNO IDL singleton page title */
   virtual QString trSingletonReference(const QString &sName) {
      return QCoreApplication::translate("doxy-text", "%1 Singleton Reference").arg(sName);
   }

   /** UNO IDL service page */
   virtual QString trServiceGeneratedFromFiles(bool single) {
      if (single) {
         return QCoreApplication::translate("doxy-text", "The documentation for this service "
                          "was generated from the following file:");

      } else {
        return QCoreApplication::translate("doxy-text", "The documentation for this service "
                          "was generated from the following files:");
      }
   }

   /** UNO IDL singleton page */
   virtual QString trSingletonGeneratedFromFiles(bool single) {           
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
