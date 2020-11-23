/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
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

#ifndef MANGEN_H
#define MANGEN_H

#include <QFile>
#include <QString>

#include <language.h>
#include <outputgen.h>

/** Generator for Man page output. */
class ManGenerator : public OutputGenerator
{
 public:
   ManGenerator();

   void init();

   void enable() override {
      if (! genStack.isEmpty()) {
         active = genStack.top();

      } else {
         active = true;
      }
   }

   void disable() override {
      active = false;
   }

   void enableIf(OutputType o)  override {
      if (o == Man) {
         enable();
      }
   }

   void disableIf(OutputType o) override {
      if (o == Man) {
         disable();
      }
   }

   void disableIfNot(OutputType o) override {
      if (o != Man) {
         disable();
      }
   }

   bool isEnabled(OutputType o) override {
      return (o == Man && active);
   }

   OutputGenerator *get(OutputType o) override {
      return (o == Man) ? this : 0;
   }

   void writeDoc(DocNode *, QSharedPointer<Definition> ctx, QSharedPointer<MemberDef> md) override;

   void startFile(const QString &name, const QString &manName, const QString &title) override;
   void writeSearchInfo() override {}
   void writeFooter(const QString &) override {}
   void endFile() override;

   void clearBuffer();

   void startIndexSection(IndexSections) override {}
   void endIndexSection(IndexSections) override {}
   void writePageLink(const QString &, bool) override {}
   void startProjectNumber() override {}
   void endProjectNumber() override {}
   void writeStyleInfo(int) override {}

   void startTitleHead(const QString &) override {}
   void endTitleHead(const QString &, const QString &) override;
   void startTitle() override;
   void endTitle() override;

   void newParagraph();

   void startParagraph(const QString &className) override;
   void endParagraph() override;
   void writeString(const QString &text) override;
   void startIndexListItem() override {}
   void endIndexListItem() override {}
   void startIndexList() override {}

   void endIndexList() override {
      newParagraph();
   }

   void startIndexKey() override {}
   void endIndexKey()   override {}
   void startIndexValue(bool) override {}
   void endIndexValue(const QString &, bool) override {}
   void startItemList() override {}

   void endItemList()  override {
      newParagraph();
   }

   void startIndexItem(const QString &ref, const QString &file) override;
   void endIndexItem(const QString &ref, const QString &file) override;

   void docify(const QString &text) override;
   void codify(const QString &text) override;

   void writeObjectLink(const QString &ref, const QString &file, const QString &anchor, const QString &name) override;
   void writeCodeLink(const QString &ref, const QString &file, const QString &anchor,
                      const QString &name, const QString &tooltip) override;

   void writeTooltip(const QString &, const DocLinkInfo &, const QString &, const QString &, const SourceLinkInfo &,
                     const SourceLinkInfo &) override {}

   void startTextLink(const QString &, const QString &) override {}
   void endTextLink() override {}
   void startHtmlLink(const QString &url) override;
   void endHtmlLink() override;

   void startTypewriter()  override {
      m_textStream << "\\fC";
      firstCol = false;
   }

   void endTypewriter()   override {
      m_textStream << "\\fP";
      firstCol = false;
   }

   void startGroupHeader(int) override;
   void endGroupHeader(int) override;
   void startMemberSections() override {}
   void endMemberSections() override {}
   void startHeaderSection() override {}
   void endHeaderSection() override;
   void startMemberHeader(const QString &) override;
   void endMemberHeader() override;
   void insertMemberAlign(bool) override {}
   void startMemberSubtitle() override {}
   void endMemberSubtitle() override {}

   //void writeListItem();
   void startItemListItem() override;
   void endItemListItem() override;
   void startMemberDocList() override {}
   void endMemberDocList() override {}
   void startMemberList() override;
   void endMemberList() override;
   void startInlineHeader() override;
   void endInlineHeader() override;
   void startAnonTypeScope(int) override;
   void endAnonTypeScope(int) override;
   void startMemberItem(const QString &, int, const QString &, bool deprecated = false) override;
   void endMemberItem() override;
   void startMemberTemplateParams() override {}
   void endMemberTemplateParams(const QString &, const QString &) override {}

   void startMemberGroupHeader(bool) override;
   void endMemberGroupHeader() override;
   void startMemberGroupDocs() override;
   void endMemberGroupDocs() override;
   void startMemberGroup() override;
   void endMemberGroup(bool) override;

   void writeRuler() override {}
   void writeAnchor(const QString &, const QString &) override {}
   void startCodeFragment(const QString &style) override;
   void endCodeFragment(const QString &style) override;

   void writeLineNumber(const QString &, const QString &, const QString &, int l) override {
      m_textStream << l << " ";
   }

   void startCodeLine(bool) override {}

   void endCodeLine() override {
      codify("\n");
      col = 0;
   }

   void startEmphasis() override {
      m_textStream << "\\fI";
      firstCol = false;
   }

   void endEmphasis()  override {
      m_textStream << "\\fP";
      firstCol = false;
   }

   void startBold()  override {
      m_textStream << "\\fB";
      firstCol = false;
   }

   void endBold() override {
      m_textStream << "\\fP";
      firstCol = false;
   }

   void startDescription() override {}
   void endDescription() override   {}
   void startDescItem() override;
   void endDescItem() override;

   void lineBreak(const QString &) override {
      m_textStream << "\n.br" << endl;
   }

   void writeChar(char c) override;
   void startMemberDoc(const QString &, const QString &, const QString &, const QString &, bool) override;
   void endMemberDoc(bool) override;
   void startDoxyAnchor(const QString &, const QString &, const QString &, const QString &, const QString &) override;
   void endDoxyAnchor(const QString &, const QString &) override {}
   void writeLatexSpacing() override {}

   void writeStartAnnoItem(const QString &type, const QString &file, const QString &path, const QString &name) override;

   void writeEndAnnoItem(const QString &) override {
      m_textStream << endl;
      firstCol = true;
   }

   void startSubsection() override;
   void endSubsection() override;
   void startSubsubsection() override;
   void endSubsubsection() override;
   void startCenter() override {}
   void endCenter()   override {}
   void startSmall()  override {}
   void endSmall()    override {}

   void startMemberDescription(const QString &, const QString &) override {
      m_textStream << "\n.RI \"";
      firstCol = false;
   }

   void endMemberDescription() override {
      m_textStream << "\"";
      firstCol = false;
   }

   void startMemberDeclaration() override {}
   void endMemberDeclaration(const QString &, const QString &) override {}

   void writeInheritedSectionTitle(const QString &, const QString &, const QString &, const QString &,
                  const QString &, const QString &) override {}

   void startDescList(SectionTypes);
   void endDescList() {}

   void startSimpleSect(SectionTypes, const QString &, const QString &, const QString &) override;
   void endSimpleSect() override;
   void startParamList(ParamListTypes, const QString &title) override;
   void endParamList() override;

   //void writeDescItem();
   void startDescForItem() override;
   void endDescForItem() override;

   void startSection(const QString &, const QString &, SectionInfo::SectionType) override;
   void endSection(const QString &, SectionInfo::SectionType) override;
   void addIndexItemName(const QString &, const QString &)  override {}
   void startIndent() override{}
   void endIndent()  override{}

   void writeSynopsis() override;
   void startClassDiagram() override {}
   void endClassDiagram(const ClassDiagram &, const QString &, const QString &) override {}
   void startPageRef() override {}
   void endPageRef(const QString &, const QString &) override {}

   void startQuickIndices() override {}
   void endQuickIndices() override {}
   void writeSplitBar(const QString &) override {}
   void writeNavigationPath(const QString &) override {}
   void writeLogo() override {}
   void writeQuickLinks(bool, HighlightedItem, const QString &) override {}
   void writeSummaryLink(const QString &, const QString &, const QString &, bool) override {}
   void startContents()  override {}
   void endContents() override {}

   void writeNonBreakableSpace(int n) override {
      for (int i = 0; i < n; i++) {
         m_textStream << " ";
      }
   }

   void startEnumTable() override {
      startSimpleSect(EnumValues, 0, 0, theTranslator->trEnumerationValues());
      startDescForItem();
   }

   void endEnumTable()  override{
      endDescForItem();
      endSimpleSect();
   }

   void startDescTable(const QString &title) override {
      startSimpleSect(EnumValues, 0, 0, title);
      startDescForItem();
   }

   void endDescTable() override {
      endDescForItem();
      endSimpleSect();
   }

   void startDescTableRow() override {};
   void endDescTableRow() override {};

   void startDescTableTitle() override {
      startItemListItem();
      startBold();
      startEmphasis();
      endItemListItem();
   }

   void endDescTableTitle() override {
      endEmphasis();
      endBold();
   }

   void startDescTableData() override{
      m_textStream << endl;
      firstCol = true;
   }

   void endDescTableData() override {}

   void startDotGraph() override {}
   void endDotGraph(const DotClassGraph &) override {}
   void startInclDepGraph() override {}
   void endInclDepGraph(const DotInclDepGraph &) override {}
   void startGroupCollaboration() override  {}
   void endGroupCollaboration(const DotGroupCollaboration &) override {}
   void startCallGraph() override {}
   void endCallGraph(const DotCallGraph &) override {}
   void startDirDepGraph() override {}
   void endDirDepGraph(const DotDirDeps &) override {}
   void writeGraphicalHierarchy(const DotGfxHierarchyTable &)  override {}

   void startTextBlock(bool) override {}
   void endTextBlock(bool) override {}
   void lastIndexPage() override {}

   void startMemberDocPrefixItem() override {}
   void endMemberDocPrefixItem() override {}
   void startMemberDocName(bool) override {}
   void endMemberDocName() override {}
   void startParameterType(bool, const QString &) override {}
   void endParameterType() override {}
   void startParameterName(bool) override {}
   void endParameterName(bool, bool, bool) override {}
   void startParameterList(bool) override {}
   void endParameterList() override {}
   void exceptionEntry(const QString &, bool) override {}

   void startFontClass(const QString &) override {}
   void endFontClass() override {}

   void startConstraintList(const QString &) override;
   void startConstraintParam() override;
   void endConstraintParam() override;
   void startConstraintType() override;
   void endConstraintType() override;
   void startConstraintDocs() override;
   void endConstraintDocs() override;
   void endConstraintList() override;

   void startMemberDocSimple(bool isEnum) override;
   void endMemberDocSimple(bool isEnum) override;
   void startInlineMemberType() override;
   void endInlineMemberType() override;
   void startInlineMemberName() override;
   void endInlineMemberName() override;
   void startInlineMemberDoc() override;
   void endInlineMemberDoc() override;

   void startLabels() override;
   void writeLabel(const QString &l, bool isLast) override;
   void endLabels() override;

   void writeCodeAnchor(const QString &) override { }

   void setCurrentDoc(QSharedPointer<Definition> def, const QString &, bool) override {
      (void) def;
   }

   void addWord(const QString &word, bool hiPriority) override {
      (void) word;
      (void) hiPriority;
   }

 private:
   bool firstCol;
   bool paragraph;
   int col;
   bool upperCase;
   bool insideTabbing;
   bool inHeader;

   ManGenerator(const ManGenerator &g);
   ManGenerator &operator=(const ManGenerator &g);
};

#endif
