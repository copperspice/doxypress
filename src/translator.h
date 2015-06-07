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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <classdef.h>

/** Abstract base class for all translatable text fragments. */
class Translator
{
 public:

   /*! This method is used to provide warning message that is displayed
    *  if the user chooses a language whose translation is not up to date.
    *  It is implemented by the adapter classes.
    */
   virtual QByteArray updateNeededMessage() {
      return "";
   }

   virtual ~Translator() {}
 
   // --- Language control methods -------------------

   virtual QByteArray idLanguage() = 0;
   virtual QByteArray latexLanguageSupportCommand() = 0;

   // --- Language translation methods -------------------

   virtual QByteArray trRelatedFunctions() = 0;
   virtual QByteArray trRelatedSubscript() = 0;
   virtual QByteArray trDetailedDescription() = 0;
   virtual QByteArray trMemberTypedefDocumentation() = 0;
   virtual QByteArray trMemberEnumerationDocumentation() = 0;
   virtual QByteArray trMemberFunctionDocumentation() = 0;
   virtual QByteArray trMemberDataDocumentation() = 0;
   virtual QByteArray trMore() = 0;
   virtual QByteArray trListOfAllMembers() = 0;
   virtual QByteArray trMemberList() = 0;
   virtual QByteArray trThisIsTheListOfAllMembers() = 0;
   virtual QByteArray trIncludingInheritedMembers() = 0;
   virtual QByteArray trGeneratedAutomatically(const char *s) = 0;
   virtual QByteArray trEnumName() = 0;
   virtual QByteArray trEnumValue() = 0;
   virtual QByteArray trDefinedIn() = 0;

   // quick reference sections
   virtual QByteArray trModules() = 0;
   virtual QByteArray trClassHierarchy() = 0;
   virtual QByteArray trCompoundList() = 0;
   virtual QByteArray trFileList() = 0;

   // virtual QByteArray trHeaderFiles() = 0;
   // virtual QByteArray trHeaderFilesDescription() = 0;
   // virtual QByteArray trNoDescriptionAvailable() = 0;

   virtual QByteArray trCompoundMembers() = 0;
   virtual QByteArray trFileMembers() = 0;
   virtual QByteArray trRelatedPages() = 0;
   virtual QByteArray trExamples() = 0;
   virtual QByteArray trSearch() = 0;
   virtual QByteArray trClassHierarchyDescription() = 0;
   virtual QByteArray trFileListDescription(bool extractAll) = 0;
   virtual QByteArray trCompoundListDescription() = 0;
   virtual QByteArray trCompoundMembersDescription(bool extractAll) = 0;
   virtual QByteArray trFileMembersDescription(bool extractAll) = 0;   
   virtual QByteArray trExamplesDescription() = 0;
   virtual QByteArray trRelatedPagesDescription() = 0;
   virtual QByteArray trModulesDescription() = 0;
   
   // index titles (the project name is prepended for these)

   virtual QByteArray trDocumentation() = 0;
   virtual QByteArray trModuleIndex() = 0;
   virtual QByteArray trHierarchicalIndex() = 0;
   virtual QByteArray trCompoundIndex() = 0;
   virtual QByteArray trFileIndex() = 0;
   virtual QByteArray trModuleDocumentation() = 0;
   virtual QByteArray trClassDocumentation() = 0;
   virtual QByteArray trFileDocumentation() = 0;
   virtual QByteArray trExampleDocumentation() = 0;
   virtual QByteArray trPageDocumentation() = 0;
   virtual QByteArray trReferenceManual() = 0;
   virtual QByteArray trDefines() = 0;
  
   virtual QByteArray trTypedefs() = 0;
   virtual QByteArray trEnumerations() = 0;
   virtual QByteArray trFunctions() = 0;
   virtual QByteArray trVariables() = 0;
   virtual QByteArray trEnumerationValues() = 0;
   virtual QByteArray trDefineDocumentation() = 0; 
   virtual QByteArray trTypedefDocumentation() = 0;
   virtual QByteArray trEnumerationTypeDocumentation() = 0;
   virtual QByteArray trFunctionDocumentation() = 0;
   virtual QByteArray trVariableDocumentation() = 0;
   virtual QByteArray trCompounds() = 0;
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) = 0;

   // virtual QByteArray trFuncProtos() = 0;
   // virtual QByteArray trFunctionPrototypeDocumentation() = 0;
   // virtual QByteArray trWrittenBy() = 0;
   // virtual QByteArray trReimplementedForInternalReasons() = 0;
   // virtual QByteArray trBugsAndLimitations() = 0;

   virtual QByteArray trClassDiagram(const char *clName) = 0;
   virtual QByteArray trForInternalUseOnly() = 0;  
   virtual QByteArray trWarning() = 0;   
   virtual QByteArray trVersion() = 0;
   virtual QByteArray trDate() = 0;
   virtual QByteArray trReturns() = 0;
   virtual QByteArray trSeeAlso() = 0;
   virtual QByteArray trParameters() = 0;
   virtual QByteArray trExceptions() = 0;
   virtual QByteArray trGeneratedBy() = 0;
   virtual QByteArray trNamespaceList() = 0;
   virtual QByteArray trNamespaceListDescription(bool extractAll) = 0;
   virtual QByteArray trFriends() = 0; 
   virtual QByteArray trRelatedFunctionDocumentation() = 0;
   virtual QByteArray trCompoundReference(const char *clName, ClassDef::CompoundType compType, bool isTemplate) = 0;

   virtual QByteArray trFileReference(const char *fileName) = 0;
   virtual QByteArray trNamespaceReference(const char *namespaceName) = 0;

   virtual QByteArray trPublicMembers() = 0;  
   virtual QByteArray trProtectedMembers() = 0;
   virtual QByteArray trPrivateMembers() = 0;    
 
   virtual QByteArray trPublicSignals() = 0;
   virtual QByteArray trProtectedSignals() = 0;
   virtual QByteArray trPrivateSignals() = 0;

   virtual QByteArray trPublicSlots() = 0;
   virtual QByteArray trProtectedSlots() = 0;  
   virtual QByteArray trPrivateSlots() = 0;
   
   virtual QByteArray trStaticPublicMembers() = 0;
   virtual QByteArray trStaticProtectedMembers() = 0;
   virtual QByteArray trStaticPrivateMembers() = 0;

   virtual QByteArray trWriteList(int numEntries) = 0;
   virtual QByteArray trInheritsList(int numEntries) = 0;
   virtual QByteArray trInheritedByList(int numEntries) = 0;
   virtual QByteArray trReimplementedFromList(int numEntries) = 0;
   virtual QByteArray trReimplementedInList(int numEntries) = 0;

   virtual QByteArray trNamespaceMembers() = 0;
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) = 0;
   virtual QByteArray trNamespaceIndex() = 0;
   virtual QByteArray trNamespaceDocumentation() = 0;
   virtual QByteArray trNamespaces() = 0;
  
   virtual QByteArray trGeneratedFromFiles(ClassDef::CompoundType compType, bool single) = 0;  
   virtual QByteArray trReturnValues() = 0;
   virtual QByteArray trMainPage() = 0;
   virtual QByteArray trPageAbbreviation() = 0;   
   virtual QByteArray trDefinedAtLineInSourceFile() = 0;
   virtual QByteArray trDefinedInSourceFile() = 0;
   virtual QByteArray trDeprecated() = 0; 
   virtual QByteArray trCollaborationDiagram(const char *clName) = 0;
   virtual QByteArray trInclDepGraph(const char *fName) = 0;
   virtual QByteArray trConstructorDocumentation() = 0;
   virtual QByteArray trGotoSourceCode() = 0;
   virtual QByteArray trGotoDocumentation() = 0;
   virtual QByteArray trPrecondition() = 0;
   virtual QByteArray trPostcondition() = 0;
   virtual QByteArray trInvariant() = 0;
   virtual QByteArray trInitialValue() = 0;
   virtual QByteArray trCode() = 0;

   virtual QByteArray trGraphicalHierarchy() = 0;
   virtual QByteArray trGotoGraphicalHierarchy() = 0;
   virtual QByteArray trGotoTextualHierarchy() = 0;
   virtual QByteArray trPageIndex() = 0;

   virtual QByteArray trNote() = 0;
   virtual QByteArray trPublicTypes() = 0;
   virtual QByteArray trPublicAttribs() = 0;
   virtual QByteArray trStaticPublicAttribs() = 0;
   virtual QByteArray trProtectedTypes() = 0;
   virtual QByteArray trProtectedAttribs() = 0;
   virtual QByteArray trStaticProtectedAttribs() = 0;
   virtual QByteArray trPrivateTypes() = 0;
   virtual QByteArray trPrivateAttribs() = 0;
   virtual QByteArray trStaticPrivateAttribs() = 0;

   virtual QByteArray trTodo() = 0;
   virtual QByteArray trTodoList() = 0;

   virtual QByteArray trReferencedBy() = 0;
   virtual QByteArray trRemarks() = 0;
   virtual QByteArray trAttention() = 0;
   virtual QByteArray trInclByDepGraph() = 0;
   virtual QByteArray trSince() = 0;

   virtual QByteArray trLegendTitle() = 0;
   virtual QByteArray trLegendDocs() = 0;
   virtual QByteArray trLegend() = 0;

   virtual QByteArray trTest() = 0;
   virtual QByteArray trTestList() = 0;

   virtual QByteArray trProperties() = 0;
   virtual QByteArray trPropertyDocumentation() = 0;

   virtual QByteArray trClasses() = 0;
   virtual QByteArray trPackage(const char *name) = 0;

   virtual QByteArray trPackageList() = 0;
   virtual QByteArray trPackageListDescription() = 0;
   virtual QByteArray trPackages() = 0;

   // virtual QByteArray trPackageDocumentation() = 0;

   virtual QByteArray trDefineValue() = 0;
   virtual QByteArray trBug() = 0;
   virtual QByteArray trBugList() = 0;

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
   virtual QByteArray trRTFansicp() = 0;

   /*! Character sets
    *  <pre>
    *   0 — ANSI
    *   1 — Default
    *   2 — Symbol
    *   3 — Invalid
    *  77 — Mac
    * 128 — Shift Jis
    * 129 — Hangul
    * 130 — Johab
    * 134 — GB2312
    * 136 — Big5
    * 161 — Greek
    * 162 — Turkish
    * 163 — Vietnamese
    * 177 — Hebrew
    * 178 — Arabic
    * 179 — Arabic Traditional
    * 180 — Arabic user
    * 181 — Hebrew user
    * 186 — Baltic
    * 204 — Russian
    * 222 — Thai
    * 238 — Eastern European
    * 254 — PC 437
    * 255 — OEM
    * </pre>
    */
   virtual QByteArray trRTFCharSet() = 0;
   virtual QByteArray trRTFGeneralIndex() = 0;

   // Translation of the word

   virtual QByteArray trClass(bool first_capital, bool singular) = 0;
   virtual QByteArray trFile(bool first_capital, bool singular) = 0;
   virtual QByteArray trNamespace(bool first_capital, bool singular) = 0;
   virtual QByteArray trGroup(bool first_capital, bool singular) = 0;
   virtual QByteArray trPage(bool first_capital, bool singular) = 0;
   virtual QByteArray trMember(bool first_capital, bool singular) = 0;

   // virtual QByteArray trField(bool first_capital, bool singular) = 0;

   virtual QByteArray trGlobal(bool first_capital, bool singular) = 0;
   virtual QByteArray trAuthor(bool first_capital, bool singular) = 0;
   virtual QByteArray trReferences() = 0;

   virtual QByteArray trImplementedFromList(int numEntries) = 0;
   virtual QByteArray trImplementedInList(int numEntries) = 0;
   virtual QByteArray trRTFTableOfContents() = 0;
   virtual QByteArray trDeprecatedList() = 0;
   virtual QByteArray trEvents() = 0;
   virtual QByteArray trEventDocumentation() = 0; 
   virtual QByteArray trPackageTypes() = 0;
   virtual QByteArray trPackageMembers() = 0;
   virtual QByteArray trStaticPackageMembers() = 0;
   virtual QByteArray trPackageAttribs() = 0;
   virtual QByteArray trStaticPackageAttribs() = 0; 
   virtual QByteArray trAll() = 0;
   virtual QByteArray trCallGraph() = 0;
  
   //virtual QByteArray trSearchForIndex() = 0;

   virtual QByteArray trSearchResultsTitle() = 0;
   virtual QByteArray trSearchResults(int numDocuments) = 0;
   virtual QByteArray trSearchMatches() = 0;
   virtual QByteArray trSourceFile(QByteArray &filename) = 0;
   virtual QByteArray trDirIndex() = 0;
   virtual QByteArray trDirDocumentation() = 0;
   virtual QByteArray trDirectories() = 0;
   virtual QByteArray trDirDescription() = 0;
   virtual QByteArray trDirReference(const char *dirName) = 0;
   virtual QByteArray trDir(bool first_capital, bool singular) = 0;  
   virtual QByteArray trOverloadText() = 0;
   virtual QByteArray trCallerGraph() = 0;
   virtual QByteArray trEnumerationValueDocumentation() = 0;
   virtual QByteArray trMemberFunctionDocumentationFortran() = 0;
   virtual QByteArray trCompoundListFortran() = 0;
   virtual QByteArray trCompoundMembersFortran() = 0;
   virtual QByteArray trCompoundListDescriptionFortran() = 0;
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) = 0;
   virtual QByteArray trCompoundIndexFortran() = 0;
   virtual QByteArray trTypeDocumentation() = 0;
   virtual QByteArray trSubprograms() = 0;
   virtual QByteArray trSubprogramDocumentation() = 0;
   virtual QByteArray trDataTypes() = 0;
   virtual QByteArray trModulesList() = 0;
   virtual QByteArray trModulesListDescription(bool extractAll) = 0;

   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType, bool isTemplate) = 0;

   virtual QByteArray trModuleReference(const char *namespaceName) = 0;
   virtual QByteArray trModulesMembers() = 0;
   virtual QByteArray trModulesMemberDescription(bool extractAll) = 0;
   virtual QByteArray trModulesIndex() = 0;
   virtual QByteArray trModule(bool first_capital, bool singular) = 0;
   virtual QByteArray trGeneratedFromFilesFortran(ClassDef::CompoundType compType, bool single) = 0;
   virtual QByteArray trType(bool first_capital, bool singular) = 0;
   virtual QByteArray trSubprogram(bool first_capital, bool singular) = 0;
   virtual QByteArray trTypeConstraints() = 0;

   virtual QByteArray trDirRelation(const char *name) = 0;
   virtual QByteArray trLoading() = 0;
   virtual QByteArray trGlobalNamespace() = 0;
   virtual QByteArray trSearching() = 0;
   virtual QByteArray trNoMatches() = 0;
  
   //virtual QByteArray trDirDependency(const char *name) = 0;
   virtual QByteArray trFileIn(const char *name) = 0;
   virtual QByteArray trIncludesFileIn(const char *name) = 0;
 
   virtual QByteArray trCiteReferences() = 0;
   virtual QByteArray trCopyright() = 0;
   virtual QByteArray trDirDepGraph(const char *name) = 0;

   virtual QByteArray trDetailLevel() = 0;
   virtual QByteArray trTemplateParameters() = 0;
   virtual QByteArray trAndMore(const QByteArray &number) = 0;
   virtual QByteArray trEnumGeneratedFromFiles(bool single) = 0;
   virtual QByteArray trEnumReference(const char *name) = 0;
   virtual QByteArray trInheritedFrom(const char *members, const char *what) = 0;
   virtual QByteArray trAdditionalInheritedMembers() = 0;

   virtual QByteArray trPanelSynchronisationTooltip(bool enable) = 0;
   virtual QByteArray trProvidedByCategory() = 0;
   virtual QByteArray trExtendsClass() = 0;
   virtual QByteArray trClassMethods() = 0;
   virtual QByteArray trInstanceMethods() = 0;
   virtual QByteArray trMethodDocumentation() = 0;
   virtual QByteArray trDesignOverview() = 0;

   virtual QByteArray trInterfaces() = 0;
   virtual QByteArray trServices() = 0;
   virtual QByteArray trConstantGroups() = 0;
   virtual QByteArray trConstantGroupReference(const char *namespaceName) = 0;
   virtual QByteArray trServiceReference(const char *sName) = 0;
   virtual QByteArray trSingletonReference(const char *sName) = 0;
   virtual QByteArray trServiceGeneratedFromFiles(bool single) = 0;
   virtual QByteArray trSingletonGeneratedFromFiles(bool single) = 0;
};

#endif
