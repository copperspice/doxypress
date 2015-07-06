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
   ~ManGenerator();

   void enable() {
      if (! genStack.isEmpty()) {
         active = genStack.top();

      } else {
         active = true;
      }
   }

   void disable() {
      active = false;
   }

   void enableIf(OutputType o)  {
      if (o == Man) {
         enable();
      }
   }
   void disableIf(OutputType o) {
      if (o == Man) {
         disable();
      }
   }
   void disableIfNot(OutputType o) {
      if (o != Man) {
         disable();
      }
   }
   bool isEnabled(OutputType o) {
      return (o == Man && active);
   }
   OutputGenerator *get(OutputType o) {
      return (o == Man) ? this : 0;
   }

   void writeDoc(DocNode *, QSharedPointer<Definition> ctx, QSharedPointer<MemberDef> md);

   static void init();
   void startFile(const QString &name, const QString &manName, const QString &title) override;
   void writeSearchInfo() {}
   void writeFooter(const QString &) override {}
   void endFile();
   void clearBuffer();

   void startIndexSection(IndexSections) {}
   void endIndexSection(IndexSections) {}
   void writePageLink(const QString &, bool) override {}
   void startProjectNumber() {}
   void endProjectNumber() {}
   void writeStyleInfo(int) {}
   void startTitleHead(const QString &) override {}
   void endTitleHead(const QString &, const QString &) override;
   void startTitle();
   void endTitle();

   void newParagraph();
   void startParagraph();
   void endParagraph();
   void writeString(const QString &text);
   void startIndexListItem() {}
   void endIndexListItem() {}
   void startIndexList() {}

   void endIndexList()   {
      newParagraph();
   }

   void startIndexKey() {}
   void endIndexKey()   {}
   void startIndexValue(bool) {}
   void endIndexValue(const QString &, bool)  override {}
   void startItemList()  {}

   void endItemList()    {
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
   void endTextLink() override{}
   void startHtmlLink(const QString &url) override;
   void endHtmlLink();

   void startTypewriter() {
      m_textStream << "\\fC";
      firstCol = false;
   }
   void endTypewriter()   {
      m_textStream << "\\fP";
      firstCol = false;
   }
   void startGroupHeader(int);
   void endGroupHeader(int);
   void startMemberSections() {}
   void endMemberSections() {}
   void startHeaderSection() {}
   void endHeaderSection();
   void startMemberHeader(const QString &) override;
   void endMemberHeader();
   void insertMemberAlign(bool) {}
   void startMemberSubtitle() {}
   void endMemberSubtitle() {}
  
   //void writeListItem();
   void startItemListItem();
   void endItemListItem();
   void startMemberDocList() {}
   void endMemberDocList() {}
   void startMemberList();
   void endMemberList();
   void startInlineHeader();
   void endInlineHeader();
   void startAnonTypeScope(int);
   void endAnonTypeScope(int);
   void startMemberItem(const QString &, int, const QString &) override;
   void endMemberItem();
   void startMemberTemplateParams() {}
   void endMemberTemplateParams(const QString &, const QString &) override {}

   void startMemberGroupHeader(bool);
   void endMemberGroupHeader();
   void startMemberGroupDocs();
   void endMemberGroupDocs();
   void startMemberGroup();
   void endMemberGroup(bool);

   void writeRuler()    {}
   void writeAnchor(const QString &, const QString &) override {}
   void startCodeFragment();
   void endCodeFragment();
   void writeLineNumber(const QString &, const QString &, const QString &, int l) override {
      m_textStream << l << " ";
   }
   void startCodeLine(bool) {}
   void endCodeLine() {
      codify("\n");
      col = 0;
   }
   void startEmphasis() {
      m_textStream << "\\fI";
      firstCol = false;
   }
   void endEmphasis()   {
      m_textStream << "\\fP";
      firstCol = false;
   }
   void startBold()     {
      m_textStream << "\\fB";
      firstCol = false;
   }
   void endBold()       {
      m_textStream << "\\fP";
      firstCol = false;
   }
   void startDescription() {}
   void endDescription()   {}
   void startDescItem();
   void endDescItem();

   void lineBreak(const QString &) override {
      m_textStream << "\n.br" << endl;
   }

   void writeChar(char c);
   void startMemberDoc(const QString &, const QString &, const QString &, const QString &, bool) override;
   void endMemberDoc(bool);
   void startDoxyAnchor(const QString &, const QString &, const QString &, const QString &, const QString &) override;
   void endDoxyAnchor(const QString &, const QString &) override {}
   void writeLatexSpacing() {}

   void writeStartAnnoItem(const QString &type, const QString &file, const QString &path, const QString &name) override;

   void writeEndAnnoItem(const QString &) override {
      m_textStream << endl;
      firstCol = true;
   }

   void startSubsection();
   void endSubsection();
   void startSubsubsection();
   void endSubsubsection();
   void startCenter()        {}
   void endCenter()          {}
   void startSmall()         {}
   void endSmall()           {}

   void startMemberDescription(const QString &, const QString &) override {
      m_textStream << "\n.RI \"\\fI";
      firstCol = false;
   }

   void endMemberDescription()   {
      m_textStream << "\\fP\"";
      firstCol = false;
   }

   void startMemberDeclaration() {}
   void endMemberDeclaration(const QString &, const QString &) override {}

   void writeInheritedSectionTitle(const QString &, const QString &, const QString &, const QString &, 
                  const QString &, const QString &) override {}

   void startDescList(SectionTypes);
   void endDescList() {}

   void startSimpleSect(SectionTypes, const QString &, const QString &, const QString &) override;
   void endSimpleSect();
   void startParamList(ParamListTypes, const QString &title) override;
   void endParamList();

   //void writeDescItem();
   void startDescForItem();
   void endDescForItem();
   void startSection(const QString &, const QString &, SectionInfo::SectionType) override;
   void endSection(const QString &, SectionInfo::SectionType) override;
   void addIndexItem(const QString &, const QString &)  override {}
   void startIndent()        {}
   void endIndent()          {}
   void writeSynopsis();
   void startClassDiagram() {}
   void endClassDiagram(const ClassDiagram &, const QString &, const QString &) override {}
   void startPageRef() {}
   void endPageRef(const QString &, const QString &) override {}
   void startQuickIndices() {}
   void endQuickIndices() {}
   void writeSplitBar(const QString &) override {}
   void writeNavigationPath(const QString &) override {}
   void writeLogo() {}
   void writeQuickLinks(bool, HighlightedItem, const QString &) override {}
   void writeSummaryLink(const QString &, const QString &, const QString &, bool) override {}
   void startContents() {}
   void endContents() {}

   void writeNonBreakableSpace(int n) {
      
      for (int i = 0; i < n; i++) {
         m_textStream << " ";
      }
   }

   void startEnumTable() {
      startSimpleSect(EnumValues, 0, 0, theTranslator->trEnumerationValues());
      startDescForItem();
   }
   void endEnumTable() {
      endDescForItem();
      endSimpleSect();
   }
   void startDescTableTitle() {
      startItemListItem();
      startBold();
      startEmphasis();
      endItemListItem();
   }
   void endDescTableTitle() {
      endEmphasis();
      endBold();
   }
   void startDescTableData() {
      m_textStream << endl;
      firstCol = true;
   }
   void endDescTableData() {}

   void startDotGraph() {}
   void endDotGraph(const DotClassGraph &) {}
   void startInclDepGraph() {}
   void endInclDepGraph(const DotInclDepGraph &) {}
   void startGroupCollaboration() {}
   void endGroupCollaboration(const DotGroupCollaboration &) {}
   void startCallGraph() {}
   void endCallGraph(const DotCallGraph &) {}
   void startDirDepGraph() {}
   void endDirDepGraph(const DotDirDeps &) {}
   void writeGraphicalHierarchy(const DotGfxHierarchyTable &) {}

   void startTextBlock(bool) {}
   void endTextBlock(bool) {}
   void lastIndexPage() {}

   void startMemberDocPrefixItem() {}
   void endMemberDocPrefixItem() {}
   void startMemberDocName(bool) {}
   void endMemberDocName() {}
   void startParameterType(bool, const QString &) override {}
   void endParameterType() {}
   void startParameterName(bool) {}
   void endParameterName(bool, bool, bool) {}
   void startParameterList(bool) {}
   void endParameterList() {}
   void exceptionEntry(const QString &, bool) override {}

   void startFontClass(const QString &) override {}
   void endFontClass() {}

   void startConstraintList(const QString &) override;
   void startConstraintParam();
   void endConstraintParam();
   void startConstraintType();
   void endConstraintType();
   void startConstraintDocs();
   void endConstraintDocs();
   void endConstraintList();

   void startMemberDocSimple();
   void endMemberDocSimple();
   void startInlineMemberType();
   void endInlineMemberType();
   void startInlineMemberName();
   void endInlineMemberName();
   void startInlineMemberDoc();
   void endInlineMemberDoc();

   void startLabels();
   void writeLabel(const QString &l, bool isLast) override;
   void endLabels();

   void writeCodeAnchor(const QString &) override {}

   void setCurrentDoc(QSharedPointer<Definition> d, const QString &, bool) override {}
   void addWord(const QString &word, bool hiPriority) override {}

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
