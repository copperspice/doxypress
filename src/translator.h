/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim 
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

/** Abstract base class for translatable text fragments. */
class Translator
{
 public:

   virtual ~Translator() {}
    
   virtual QString latexLanguageSupportCommand() = 0;

   virtual QString trRelatedFunctions() = 0;
   virtual QString trRelatedSubscript() = 0;
   virtual QString trDetailedDescription() = 0;
   virtual QString trMemberTypedefDocumentation() = 0;
   virtual QString trMemberEnumerationDocumentation() = 0;
   virtual QString trMemberFunctionDocumentation() = 0;
   virtual QString trMemberDataDocumentation() = 0;
   virtual QString trMore() = 0;
   virtual QString trListOfAllMembers() = 0;
   virtual QString trMemberList() = 0;
   virtual QString trThisIsTheListOfAllMembers() = 0;
   virtual QString trIncludingInheritedMembers() = 0;
   virtual QString trGeneratedAutomatically(const QString &s) = 0;
   virtual QString trEnumName() = 0;
   virtual QString trEnumValue() = 0;
   virtual QString trDefinedIn() = 0;

   virtual QString trPublicTypedefs() = 0;   
   virtual QString trProtectedTypedefs() = 0;
   virtual QString trPrivateTypedefs() = 0;  

   // quick reference sections
   virtual QString trModules() = 0;
   virtual QString trClassHierarchy() = 0;
   virtual QString trCompoundList() = 0;
   virtual QString trFileList() = 0;

   virtual QString trCompoundMembers() = 0;
   virtual QString trFileMembers() = 0;
   virtual QString trRelatedPages() = 0;
   virtual QString trFileSource() = 0;
   virtual QString trExamples() = 0;
   virtual QString trSearch() = 0;
   virtual QString trClassHierarchyDescription() = 0;
   virtual QString trFileListDescription(bool extractAll) = 0;
   virtual QString trCompoundListDescription() = 0;
   virtual QString trCompoundMembersDescription(bool extractAll) = 0;
   virtual QString trFileMembersDescription(bool extractAll) = 0;   
   virtual QString trFileSourceDescription() = 0;
   virtual QString trExamplesDescription() = 0;
   virtual QString trRelatedPagesDescription() = 0;
   virtual QString trModulesDescription() = 0;
      
   virtual QString trDocumentation() = 0;
   virtual QString trModuleIndex() = 0;
   virtual QString trHierarchicalIndex() = 0;
   virtual QString trCompoundIndex() = 0;
   virtual QString trFileIndex() = 0;
   virtual QString trModuleDocumentation() = 0;
   virtual QString trClassDocumentation() = 0;
   virtual QString trFileDocumentation() = 0;
   virtual QString trExampleDocumentation() = 0;
   virtual QString trPageDocumentation() = 0;
   virtual QString trReferenceManual() = 0;
   virtual QString trDefines() = 0;
  
   virtual QString trTypedefs() = 0;
   virtual QString trEnumerations() = 0;
   virtual QString trFunctions() = 0;
   virtual QString trVariables() = 0;
   virtual QString trEnumerationValues() = 0;
   virtual QString trDefineDocumentation() = 0; 
   virtual QString trTypedefDocumentation() = 0;
   virtual QString trEnumerationTypeDocumentation() = 0;
   virtual QString trFunctionDocumentation() = 0;
   virtual QString trVariableDocumentation() = 0;
   virtual QString trCompounds() = 0;
   virtual QString trGeneratedAt(const QString &date, const QString &projName) = 0;

   virtual QString trClassDiagram(const QString &clName) = 0;
   virtual QString trForInternalUseOnly() = 0;  
   virtual QString trWarning() = 0;   
   virtual QString trVersion() = 0;
   virtual QString trDate() = 0;
   virtual QString trReturns() = 0;
   virtual QString trSeeAlso() = 0;
   virtual QString trParameters() = 0;
   virtual QString trExceptions() = 0;
   virtual QString trGeneratedBy() = 0;
   virtual QString trNamespaceList() = 0;
   virtual QString trNamespaceListDescription(bool extractAll) = 0;
   virtual QString trFriends() = 0; 
   virtual QString trRelatedFunctionDocumentation() = 0;
   virtual QString trCompoundReference(const QString &clName, ClassDef::CompoundType compType, bool isTemplate) = 0;

   virtual QString trFileReference(const QString &fileName) = 0;
   virtual QString trNamespaceReference(const QString &namespaceName) = 0;

   virtual QString trPublicMembers() = 0;  
   virtual QString trProtectedMembers() = 0;
   virtual QString trPrivateMembers() = 0;    
 
   virtual QString trPublicSignals() = 0;
   virtual QString trProtectedSignals() = 0;
   virtual QString trPrivateSignals() = 0;

   virtual QString trPublicSlots() = 0;
   virtual QString trProtectedSlots() = 0;  
   virtual QString trPrivateSlots() = 0;
   
   virtual QString trStaticPublicMembers() = 0;
   virtual QString trStaticProtectedMembers() = 0;
   virtual QString trStaticPrivateMembers() = 0;

   virtual QString trWriteList(int numEntries) = 0;
   virtual QString trInheritsList(int numEntries) = 0;
   virtual QString trInheritedByList(int numEntries) = 0;
   virtual QString trReimplementedFromList(int numEntries) = 0;
   virtual QString trReimplementedInList(int numEntries) = 0;

   virtual QString trNamespaceMembers() = 0;
   virtual QString trNamespaceMemberDescription(bool extractAll) = 0;
   virtual QString trNamespaceIndex() = 0;
   virtual QString trNamespaceDocumentation() = 0;
   virtual QString trNamespaces() = 0;
  
   virtual QString trGeneratedFromFiles(ClassDef::CompoundType compType, bool single) = 0;  
   virtual QString trReturnValues() = 0;
   virtual QString trMainPage() = 0;
   virtual QString trPageAbbreviation() = 0;   
   virtual QString trDefinedAtLineInSourceFile() = 0;
   virtual QString trDefinedInSourceFile() = 0;
   virtual QString trDeprecated() = 0; 
   virtual QString trCollaborationDiagram(const QString &clName) = 0;
   virtual QString trInclDepGraph(const QString &fName) = 0;
   virtual QString trConstructorDocumentation() = 0;
   virtual QString trGotoSourceCode() = 0;
   virtual QString trGotoDocumentation() = 0;
   virtual QString trPrecondition() = 0;
   virtual QString trPostcondition() = 0;
   virtual QString trInvariant() = 0;
   virtual QString trInitialValue() = 0;
   virtual QString trCode() = 0;

   virtual QString trGraphicalHierarchy() = 0;
   virtual QString trGotoGraphicalHierarchy() = 0;
   virtual QString trGotoTextualHierarchy() = 0;
   virtual QString trPageIndex() = 0;

   virtual QString trNote() = 0;
   virtual QString trPublicTypes() = 0;
   virtual QString trPublicAttribs() = 0;
   virtual QString trStaticPublicAttribs() = 0;
   virtual QString trProtectedTypes() = 0;
   virtual QString trProtectedAttribs() = 0;
   virtual QString trStaticProtectedAttribs() = 0;
   virtual QString trPrivateTypes() = 0;
   virtual QString trPrivateAttribs() = 0;
   virtual QString trStaticPrivateAttribs() = 0;

   virtual QString trTodo() = 0;
   virtual QString trTodoList() = 0;

   virtual QString trReferencedBy() = 0;
   virtual QString trRemarks() = 0;
   virtual QString trAttention() = 0;
   virtual QString trInclByDepGraph() = 0;
   virtual QString trSince() = 0;

   virtual QString trLegendTitle() = 0;
   virtual QString trLegendDocs(const QString &dotFormat) = 0;
   virtual QString trLegend() = 0;

   virtual QString trTest() = 0;
   virtual QString trTestList() = 0;

   virtual QString trProperties() = 0;
   virtual QString trPropertyDocumentation() = 0;

   virtual QString trClasses() = 0;
   virtual QString trPackage(const QString &name) = 0;

   virtual QString trPackageList() = 0;
   virtual QString trPackageListDescription() = 0;
   virtual QString trPackages() = 0;

   virtual QString trDefineValue() = 0;
   virtual QString trBug() = 0;
   virtual QString trBugList() = 0;

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
   virtual QString trRTFansicp() = 0;

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
   virtual QString trRTFCharSet() = 0;
   virtual QString trRTFGeneralIndex() = 0;

   virtual QString trClass(bool first_capital, bool singular) = 0;
   virtual QString trFile(bool first_capital, bool singular) = 0;
   virtual QString trNamespace(bool first_capital, bool singular) = 0;
   virtual QString trGroup(bool first_capital, bool singular) = 0;
   virtual QString trPage(bool first_capital, bool singular) = 0;
   virtual QString trMember(bool first_capital, bool singular) = 0;

   virtual QString trGlobal(bool first_capital, bool singular) = 0;
   virtual QString trAuthor(bool first_capital, bool singular) = 0;
   virtual QString trReferences() = 0;

   virtual QString trImplementedFromList(int numEntries) = 0;
   virtual QString trImplementedInList(int numEntries) = 0;
   virtual QString trRTFTableOfContents() = 0;
   virtual QString trDeprecatedList() = 0;
   virtual QString trEvents() = 0;
   virtual QString trEventDocumentation() = 0; 
   virtual QString trPackageTypes() = 0;
   virtual QString trPackageMembers() = 0;
   virtual QString trStaticPackageMembers() = 0;
   virtual QString trPackageAttribs() = 0;
   virtual QString trStaticPackageAttribs() = 0; 
   virtual QString trAll() = 0;
   virtual QString trCallGraph() = 0;

   virtual QString trSearchResultsTitle() = 0;
   virtual QString trSearchResults(int numDocuments) = 0;
   virtual QString trSearchMatches() = 0;
   virtual QString trSourceFile(const QString &filename) = 0;
   virtual QString trDirIndex() = 0;
   virtual QString trDirDocumentation() = 0;
   virtual QString trDirectories() = 0;
   virtual QString trDirDescription() = 0;
   virtual QString trDirReference(const QString &dirName) = 0;
   virtual QString trDir(bool first_capital, bool singular) = 0;  
   virtual QString trOverloadText() = 0;
   virtual QString trCallerGraph() = 0;
   virtual QString trEnumerationValueDocumentation() = 0;
   virtual QString trMemberFunctionDocumentationFortran() = 0;
   virtual QString trCompoundListFortran() = 0;
   virtual QString trCompoundMembersFortran() = 0;
   virtual QString trCompoundListDescriptionFortran() = 0;
   virtual QString trCompoundMembersDescriptionFortran(bool extractAll) = 0;
   virtual QString trCompoundIndexFortran() = 0;
   virtual QString trTypeDocumentation() = 0;
   virtual QString trSubprograms() = 0;
   virtual QString trSubprogramDocumentation() = 0;
   virtual QString trDataTypes() = 0;
   virtual QString trModulesList() = 0;
   virtual QString trModulesListDescription(bool extractAll) = 0;

   virtual QString trCompoundReferenceFortran(const QString &clName, ClassDef::CompoundType 
                  compType, bool isTemplate) = 0;

   virtual QString trModuleReference(const QString &namespaceName) = 0;
   virtual QString trModulesMembers() = 0;
   virtual QString trModulesMemberDescription(bool extractAll) = 0;
   virtual QString trModulesIndex() = 0;
   virtual QString trModule(bool first_capital, bool singular) = 0;
   virtual QString trGeneratedFromFilesFortran(ClassDef::CompoundType compType, bool single) = 0;
   virtual QString trType(bool first_capital, bool singular) = 0;
   virtual QString trSubprogram(bool first_capital, bool singular) = 0;
   virtual QString trTypeConstraints() = 0;

   virtual QString trDirRelation(const QString &name) = 0;
   virtual QString trLoading() = 0;
   virtual QString trGlobalNamespace() = 0;
   virtual QString trSearching() = 0;
   virtual QString trNoMatches() = 0;
    
   virtual QString trFileIn(const QString &name) = 0;
   virtual QString trIncludesFileIn(const QString &name) = 0;
 
   virtual QString trCiteReferences() = 0;
   virtual QString trCopyright() = 0;
   virtual QString trDirDepGraph(const QString &name) = 0;

   virtual QString trDetailLevel() = 0;
   virtual QString trTemplateParameters() = 0;
   virtual QString trAndMore(const QString &number) = 0;
   virtual QString trEnumGeneratedFromFiles(bool single) = 0;
   virtual QString trEnumReference(const QString &name) = 0;
   virtual QString trInheritedFrom(const QString &members, const QString &what) = 0;
   virtual QString trAdditionalInheritedMembers() = 0;

   virtual QString trPanelSyncTooltip(bool enable) = 0;
   virtual QString trProvidedByCategory() = 0;
   virtual QString trExtendsClass() = 0;
   virtual QString trClassMethods() = 0;
   virtual QString trInstanceMethods() = 0;
   virtual QString trMethodDocumentation() = 0;
   virtual QString trDesignOverview() = 0;

   virtual QString trInterfaces() = 0;
   virtual QString trServices() = 0;
   virtual QString trConstantGroups() = 0;
   virtual QString trConstantGroupReference(const QString &namespaceName) = 0;
   virtual QString trServiceReference(const QString &sName) = 0;
   virtual QString trSingletonReference(const QString &sName) = 0;
   virtual QString trServiceGeneratedFromFiles(bool single) = 0;
   virtual QString trSingletonGeneratedFromFiles(bool single) = 0;
};

#endif
