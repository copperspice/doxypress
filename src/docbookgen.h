/************************************************************************
*
* Copyright (c) 2014-2025 Barbara Geller & Ansel Sermersheim
*
* Copyright (c) 1997-2014 Dimitri van Heesch
*
* DoxyPress is free software. You can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
* https://www.gnu.org/licenses/
*
*************************************************************************/

#ifndef DOCBOOKGEN_H
#define DOCBOOKGEN_H

#include <outputgen.h>

class DocbookCodeGenerator : public CodeGenerator
{
 public:
   DocbookCodeGenerator(QTextStream &t, const QString &relPath, const QString &sourceFile);

   void finish();

   QString sourceFileName() const {
      return m_sourceFileName;
   }

   void writeCodeLinkLine(const QString &ref, const QString &file, const QString &anchor,
               const QString &name, const QString &tooltip);

   // overrides
   void addWord(const QString &, bool) override;
   void codify(const QString &text)  override;
   void setCurrentDoc(QSharedPointer<Definition>, const QString &, bool) override;

   void writeCodeAnchor(const QString &) override;

   void writeCodeLink(const QString &ref, const QString &file, const QString &anchor,
               const QString &name, const QString &tooltip) override;

   void writeLineNumber(const QString &ref, const QString &compId, const QString &anchorId, int lineNumber) override;

   void writeTooltip(const QString &id, const DocLinkInfo &docInfo, const QString &decl, const QString &desc,
               const SourceLinkInfo &, const SourceLinkInfo &) override;

   void startCodeLine(bool) override;
   void endCodeLine() override;
   void startFontClass(const QString &colorClass) override;
   void endFontClass() override;

   void startCodeFragment(const QString &) override;
   void endCodeFragment(const QString &) override;

 private:
   QTextStream &m_streamCode;

   bool m_insideCodeLine;
   bool m_insideSpecialHL;
   bool m_prettyCode;

   int m_lineNumber;
   int m_col;

   QString m_relPath;
   QString m_refId;
   QString m_external;
   QString m_sourceFileName;
};

class DocbookGenerator : public OutputGenerator
{
 public:
   DocbookGenerator();

   void init();

   void addIndexItem(const QString &, const QString &);
   void startExamples();
   void endExamples();

   OutputType type() const {
      return OutputType::Docbook;
   }

   // from codegenerator
   void addWord(const QString &, bool) override { }

   void codify(const QString &text) override {
      m_codeGen->codify(text);
   }

   void setCurrentDoc(QSharedPointer<Definition>, const QString &, bool) override { }

   void writeCodeAnchor(const QString &anchor) override {
      m_codeGen->writeCodeAnchor(anchor);
   }

   void writeCodeLink(const QString &ref, const QString &file, const QString &anchor,
            const QString &name, const QString &tooltip) override {
      m_codeGen->writeCodeLink(ref, file, anchor, name, tooltip);
   }

   void writeLineNumber(const QString &ref, const QString &file, const QString &anchor, int lineNumber) override {
      m_codeGen->writeLineNumber(ref, file, anchor, lineNumber);
   }

   void writeTooltip(const QString &id, const DocLinkInfo &docInfo, const QString &decl, const QString &desc,
            const SourceLinkInfo &defInfo, const SourceLinkInfo &declInfo) override {
      m_codeGen->writeTooltip(id, docInfo, decl, desc, defInfo, declInfo);
   }

   void startCodeLine(bool hasLineNumbers) override {
      m_codeGen->startCodeLine(hasLineNumbers);
   }

   void endCodeLine() override {
      m_codeGen->endCodeLine();
   }

   void startFontClass(const QString &s) override {
      m_codeGen->startFontClass(s);
   }

   void endFontClass() override {
      m_codeGen->endFontClass();
   }

   // from DocGenerator
   void addIndexItemName(const QString &, const QString &) override { }
   void docify(const QString &text) override;
   void lineBreak(const QString &) override;
   void writeAnchor(const QString &, const QString &) override { }

   void writeChar(char) override;
   void writeNonBreakableSpace(int) override;
   void writeObjectLink(const QString &ref, const QString &file, const QString &anchor, const QString &name) override;
   void writeRuler() override;
   void writeString(const QString &) override;

   void startItemList() override { }
   void endItemList() override { }

   void startItemListItem() override { }
   void endItemListItem() override { }

   void startParagraph(const QString &) override;
   void endParagraph() override;
   void startHtmlLink(const QString &) override { }
   void endHtmlLink() override { }
   void startBold() override;
   void endBold() override;
   void startTypewriter() override;
   void endTypewriter() override;
   void startEmphasis() override { }
   void endEmphasis() override { }

   void startCodeFragment(const QString &style) override {
      m_codeGen->startCodeFragment(style);
   }

   void endCodeFragment(const QString &style) override {
      m_codeGen->endCodeFragment(style);
   }

   void startDescription() override { }
   void endDescription() override { }
   void startDescItem() override { }
   void endDescItem() override { }
   void startDescForItem() override { }
   void endDescForItem() override { }

   void startCenter() override { }
   void endCenter() override { }
   void startSmall() override { }
   void endSmall() override { }
   void startSimpleSect(SectionTypes, const QString &, const QString &, const QString &) override { }
   void endSimpleSect() override { }
   void startParamList(ParamListTypes, const QString &) override { }
   void endParamList() override { }

   void startTitle() override { }
   void endTitle() override { }

   void startSection(const QString &, const QString &, SectionInfo::SectionType) override;
   void endSection(const QString &, SectionInfo::SectionType) override;

   void startEnumTable() override { }
   void endEnumTable() override { }

   void startDescTable(const QString &) override;
   void endDescTable() override;

   void startDescTableRow() override;
   void endDescTableRow() override;
   void startDescTableTitle() override;
   void endDescTableTitle() override;
   void startDescTableData() override;
   void endDescTableData() override;
   void startTextLink(const QString &, const QString &) override { }
   void endTextLink() override { }
   void startPageRef() override { }
   void endPageRef(const QString &, const QString &) override { }
   void startSubsection() override { }
   void endSubsection() override { }
   void startSubsubsection() override;
   void endSubsubsection() override;

   // from outputgenerator
   void enable() override   {
      if (! genStack.isEmpty()) {
         active = genStack.top();

      } else {
         active = true;
      }
   }

   void disable() override {
      active = false;
   }

   void enableIf(OutputType o) override {
      if (o == OutputType::Docbook) {
         enable();
      }
   }

   void disableIf(OutputType o) override {
      if (o == OutputType::Docbook) {
         disable();
      }
   }

   void disableIfNot(OutputType o) override {
      if (o != OutputType::Docbook) {
         disable();
      }
   }

   bool isEnabled(OutputType o) override {
      return (o == OutputType::Docbook && active);
   }

   OutputGenerator *get(OutputType o) override {
      return (o == OutputType::Docbook) ? this : nullptr;
   }

   void writeDoc(DocNode *node, QSharedPointer<Definition>, QSharedPointer<MemberDef>) override;

   void writeStartAnnoItem(const QString &, const QString &, const QString &, const QString &) override { }
   void writeEndAnnoItem(const QString &) override { }

   void writeFooter(const QString &) override { }
   void writeGraphicalHierarchy(const DotGfxHierarchyTable &) override { }
   void writeLabel(const QString &, bool) override;
   void writeLatexSpacing() override { }
   void writeLogo() override { }

   void writeInheritedSectionTitle(const QString &, const QString &, const QString &, const QString &,
            const QString &, const QString &) override { }

   void writeNavigationPath(const QString &) override { }
   void writePageLink(const QString &, bool) override;
   void writeQuickLinks(bool, HighlightedItem, const QString &) override { }

   void writeSearchInfo() override { }
   void writeSplitBar(const QString &) override { }
   void writeStyleInfo(int) override { }
   void writeSynopsis() override { }
   void writeSummaryLink(const QString &, const QString &, const QString &, bool) override { }

   void exceptionEntry(const QString &, bool) override;
   void insertMemberAlign(bool) override { }
   void insertMemberAlignLeft(int, bool) override { }
   void lastIndexPage() override { }

   void startPageDoc(const QString &) override { }
   void endPageDoc() override { }
   void startFile(const QString &name, const QString &manName, const QString &title) override;
   void endFile() override;

   void startIndexSection(IndexSections) override;
   void endIndexSection(IndexSections) override;
   void startProjectNumber() override { }
   void endProjectNumber() override { }

   void startTitleHead(const QString &) override;
   void endTitleHead(const QString &fileName, const QString &name) override;
   void startIndexListItem() override { }
   void endIndexListItem() override { }
   void startIndexList() override { }
   void endIndexList() override { }
   void startIndexKey() override { }
   void endIndexKey() override { }
   void startIndexValue(bool) override { }
   void endIndexValue(const QString &, bool) override { }

   void startIndexItem(const QString &, const QString &) override { }
   void endIndexItem(const QString &, const QString &) override { }

   void startMemberSections() override { }
   void endMemberSections() override { }
   void startHeaderSection() override { }
   void endHeaderSection() override { }

   void startMemberHeader(const QString &, int) override;
   void endMemberHeader() override;

   void startMemberSubtitle() override { }
   void endMemberSubtitle() override { }
   void startMemberDocList() override;
   void endMemberDocList() override;
   void startMemberList() override;
   void endMemberList() override;
   void startInlineHeader() override { }
   void endInlineHeader() override { }

   void startAnonTypeScope(int) override { }
   void endAnonTypeScope(int) override { }

   void startMemberItem(const QString &, int, const QString &, bool deprecated = false) override;
   void endMemberItem() override;
   void startMemberTemplateParams() override;
   void endMemberTemplateParams(const QString &, const QString &) override;

   void startMemberGroupHeader(bool) override;
   void endMemberGroupHeader() override;
   void startMemberGroupDocs() override { }
   void endMemberGroupDocs() override { }
   void startMemberGroup() override;
   void endMemberGroup(bool) override;

   void startGroupHeader(int) override;
   void endGroupHeader(int) override;

   void startTextBlock(bool) override;
   void endTextBlock(bool) override;

   void startMemberDoc(const QString &clName, const QString &memName, const QString &anchor, const QString &title,
                  int memCount, int memTotal, bool showInline) override;
   void endMemberDoc(bool) override;

   void startDoxyAnchor(const QString &fName, const QString &manName, const QString &anchor,
            const QString &name, const QString &args) override;
   void endDoxyAnchor(const QString &fileName, const QString &anchor) override;

   void startMemberDescription(const QString &, const QString &, bool) override { }
   void endMemberDescription() override { }

   void startMemberDeclaration() override { }
   void endMemberDeclaration(const QString &, const QString &) override { }

   void startClassDiagram() override;
   void endClassDiagram(const ClassDiagram &, const QString &, const QString &) override;
   void startDotGraph() override;
   void endDotGraph(const DotClassGraph &g) override;
   void startInclDepGraph() override;
   void endInclDepGraph(const DotInclDepGraph &g) override;
   void startGroupCollaboration() override;
   void endGroupCollaboration(const DotGroupCollaboration &g) override;
   void startCallGraph() override;
   void endCallGraph(const DotCallGraph &g) override;
   void startDirDepGraph() override;
   void endDirDepGraph(const DotDirDeps &g) override;

   void startQuickIndices() override { }
   void endQuickIndices() override { }

   void startContents() override { }
   void endContents() override { }
   void startIndent() override { }
   void endIndent() override { }

   void startMemberDocPrefixItem() override;
   void endMemberDocPrefixItem() override;
   void startMemberDocName(bool) override;
   void endMemberDocName() override;
   void startParameterType(bool, const QString &) override { }
   void endParameterType() override { }

   void startParameterName(bool) override;
   void endParameterName(bool last, bool emptyList, bool closeBracket) override;
   void startParameterList(bool) override;
   void endParameterList() override;

   void startConstraintList(const QString &) override;
   void startConstraintParam() override;
   void endConstraintParam() override;
   void startConstraintType() override;
   void endConstraintType() override;
   void startConstraintDocs() override;
   void endConstraintDocs() override;
   void endConstraintList() override;

   void startMemberDocSimple(bool) override { }
   void endMemberDocSimple(bool) override { }
   void startInlineMemberType() override { }
   void endInlineMemberType() override { }
   void startInlineMemberName() override { }
   void endInlineMemberName() override { }
   void startInlineMemberDoc() override { }
   void endInlineMemberDoc() override { }

   void startLabels() override;
   void endLabels() override;

 private:
   QString m_relPath;

   bool m_prettyCode;
   bool m_denseText;
   bool m_inGroup;
   bool m_inDetail;
   bool m_descTable;
   bool m_firstMember;

   bool m_inListItem[20];
   bool m_inSimpleSect[20];

   int  m_levelListItem;
   int  m_inLevel;

   QSharedPointer<DocbookCodeGenerator> m_codeGen;
};

#endif
