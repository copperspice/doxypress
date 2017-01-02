/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim 
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

#ifndef RTFGEN_H
#define RTFGEN_H

#include <QFile>

#include <outputgen.h>

/** Generator for RTF output. */
class RTFGenerator : public OutputGenerator
{
 public:
   RTFGenerator();
   ~RTFGenerator();

   static void init();
   static void writeStyleSheetFile(QFile &f);
   static void writeExtensionsFile(QFile &file);

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
      if (o == RTF) {
         enable();
      }
   }

   void disableIf(OutputType o) override {
      if (o == RTF) {
         disable();
      }
   }

   void disableIfNot(OutputType o) override {
      if (o != RTF) {
         disable();
      }
   }

   bool isEnabled(OutputType o) override {
      return (o == RTF && active);
   }

   OutputGenerator *get(OutputType o) override {
      return (o == RTF) ? this : 0;
   }

   void writeDoc(DocNode *, QSharedPointer<Definition> ctx, QSharedPointer<MemberDef> md) override;

   void startFile(const QString &name, const QString &manName, const QString &title) override;
   void writeSearchInfo() override {}
   void writeFooter(const QString &) override {}
   void endFile() override;
   void clearBuffer();

   void startIndexSection(IndexSections) override;
   void endIndexSection(IndexSections) override;
   void writePageLink(const QString &, bool) override;
   void startProjectNumber() override;
   void endProjectNumber() override;
   void writeStyleInfo(int part) override;
   void startTitleHead(const QString &) override;
   void startTitle() override;
   void endTitleHead(const QString &, const QString &name) override;
   void endTitle() override {}

   void newParagraph();

   void startParagraph() override;
   void endParagraph() override;
   void writeString(const QString &text) override;
   void startIndexListItem() override;
   void endIndexListItem() override;
   void startIndexList() override;
   void endIndexList() override;
   void startIndexKey() override;
   void endIndexKey() override;
   void startIndexValue(bool) override;
   void endIndexValue(const QString &, bool) override;
   void startItemList() override;
   void endItemList() override;

   void startIndexItem(const QString &ref, const QString &file) override;
   void endIndexItem(const QString &ref, const QString &file) override;

   void docify(const QString &text) override;
   void codify(const QString &text) override;

   void writeObjectLink(const QString &ref, const QString &file, const QString &anchor, const QString &name) override;

   void writeCodeLink(const QString &ref, const QString &file, const QString &anchor, 
                      const QString &name, const QString &tooltip) override;  

   void writeTooltip(const QString &, const DocLinkInfo &, const QString &, const QString &, 
                     const SourceLinkInfo &, const SourceLinkInfo & ) override {}

   void startTextLink(const QString &f, const QString &anchor) override;
   void endTextLink() override;
   void startHtmlLink(const QString &url) override;
   void endHtmlLink() override;

   void startTypewriter()  override {
      m_textStream << "{\\f2 ";
   }

   void endTypewriter()   override {
      m_textStream << "}";
   }

   void startGroupHeader(int) override;
   void endGroupHeader(int) override;
  
   void startItemListItem() override;
   void endItemListItem() override;

   void startMemberSections() override { }
   void endMemberSections()   override { }
   void startHeaderSection()  override { }
   void endHeaderSection()    override { }

   void startMemberHeader(const QString &) override {
      startGroupHeader(false);
   }

   void endMemberHeader() override {
      endGroupHeader(false);
   }

   void startMemberSubtitle() override;
   void endMemberSubtitle() override;
   void startMemberDocList() override {}
   void endMemberDocList() override {}
   void startMemberList() override;
   void endMemberList() override;
   void startInlineHeader() override;
   void endInlineHeader() override;
   void startAnonTypeScope(int) override {}
   void endAnonTypeScope(int) override {}
   void startMemberItem(const QString &, int, const QString &) override;
   void endMemberItem() override;
   void startMemberTemplateParams() override {}
   void endMemberTemplateParams(const QString &, const QString &) override {}
   void insertMemberAlign(bool) override {}

   void writeRuler() override {
      rtfwriteRuler_thin();
   }

   void writeAnchor(const QString &fileName, const QString &name) override;
   void startCodeFragment() override;
   void endCodeFragment() override;

   void writeLineNumber(const QString &, const QString &, const QString &, int l) override {
      m_textStream << QString("%1").arg(l, 5) << " ";
   }

   void startCodeLine(bool) override {
      col = 0;
   }

   void endCodeLine() override {
      lineBreak();
   }

   void startEmphasis()  override {
      m_textStream << "{\\i ";
   }

   void endEmphasis() override {
      m_textStream << "}";
   }

   void startBold() override  {
      m_textStream << "{\\b ";
   }

   void endBold() override {
      m_textStream << "}";
   }

   void startDescription() override;
   void endDescription() override;
   void startDescItem() override;
   void endDescItem() override;

   void lineBreak(const QString &style = 0) override;

   void startMemberDoc(const QString &, const QString &, const QString &, const QString &, bool) override;
   void endMemberDoc(bool) override;
   void startDoxyAnchor(const QString &, const QString &, const QString &, const QString &, const QString &) override;
   void endDoxyAnchor(const QString &, const QString &) override;

   void writeChar(char c) override;
   void writeLatexSpacing() override {};      
   
   void writeStartAnnoItem(const QString &type, const QString &file, const QString &path, const QString &name) override;
   void writeEndAnnoItem(const QString &name) override;

   void startSubsection() override;
   void endSubsection() override;
   void startSubsubsection() override;
   void endSubsubsection() override;

   void startCenter()  override {
      m_textStream << "{\\qc" << endl;
   }

   void endCenter() override {
      m_textStream << "}";
   }

   void startSmall() override {
      m_textStream << "{\\sub ";
   }

   void endSmall() override {
      m_textStream << "}";
   }

   void startMemberDescription(const QString &, const QString &) override;
   void endMemberDescription() override;
   void startMemberDeclaration() override {}
   void endMemberDeclaration(const QString &, const QString &) override {}

   void writeInheritedSectionTitle(const QString &, const QString &, const QString &,
                                   const QString &, const QString &, const QString &) override {}

   void startDescList(SectionTypes);

   void startSimpleSect(SectionTypes, const QString &, const QString &, const QString &) override;
   void endSimpleSect() override;
   void startParamList(ParamListTypes, const QString &) override;
   void endParamList() override;

   //void writeDescItem();
   void startDescForItem() override;
   void endDescForItem() override;
   void startSection(const QString &, const QString &, SectionInfo::SectionType) override;
   void endSection(const QString &, SectionInfo::SectionType) override;
   void addIndexItem(const QString &, const QString &) override;
   void startIndent() override;
   void endIndent() override;
   void writeSynopsis() override {}
   void startClassDiagram() override;
   void endClassDiagram(const ClassDiagram &, const QString &filename, const QString &name) override;
   void startPageRef() override;
   void endPageRef(const QString &, const QString &) override;
   void startQuickIndices() override {}
   void endQuickIndices() override {}
   void writeSplitBar(const QString &) override {}
   void writeNavigationPath(const QString &) override {}
   void writeLogo() override {}
   void writeQuickLinks(bool, HighlightedItem, const QString &) override {}
   void writeSummaryLink(const QString &, const QString &, const QString &, bool) override {}
   void startContents() override {}
   void endContents() override {}
   void writeNonBreakableSpace(int) override;

   void startEnumTable() override;
   void endEnumTable() override;
   void startDescTableTitle() override;
   void endDescTableTitle() override;
   void startDescTableData() override;
   void endDescTableData() override;

   void startDotGraph() override;
   void endDotGraph(const DotClassGraph &) override;
   void startInclDepGraph() override;
   void endInclDepGraph(const DotInclDepGraph &) override;
   void startGroupCollaboration() override;
   void endGroupCollaboration(const DotGroupCollaboration &g) override;
   void startCallGraph() override;
   void endCallGraph(const DotCallGraph &) override;
   void startDirDepGraph() override;
   void endDirDepGraph(const DotDirDeps &g) override;
   void writeGraphicalHierarchy(const DotGfxHierarchyTable &) override { }

   void startMemberGroupHeader(bool) override;
   void endMemberGroupHeader() override;
   void startMemberGroupDocs() override;
   void endMemberGroupDocs() override;
   void startMemberGroup() override;
   void endMemberGroup(bool) override;

   void startTextBlock(bool dense) override;
   void endTextBlock(bool) override;
   void lastIndexPage() override;

   void startMemberDocPrefixItem() override {}
   void endMemberDocPrefixItem() override {}
   void startMemberDocName(bool) override {}
   void endMemberDocName() override {}
   void startParameterType(bool, const QString &) override;
   void endParameterType() override;
   void startParameterName(bool) override {}
   void endParameterName(bool, bool, bool)  override{}
   void startParameterList(bool) override {}
   void endParameterList() override {}
   void exceptionEntry(const QString &, bool) override;

   void startConstraintList(const QString &) override;
   void startConstraintParam() override;
   void endConstraintParam() override;
   void startConstraintType() override;
   void endConstraintType() override;
   void startConstraintDocs() override;
   void endConstraintDocs() override;
   void endConstraintList() override;

   void startMemberDocSimple() override;
   void endMemberDocSimple() override;
   void startInlineMemberType() override;
   void endInlineMemberType() override;
   void startInlineMemberName() override;
   void endInlineMemberName() override;
   void startInlineMemberDoc() override;
   void endInlineMemberDoc() override;

   void startLabels() override;
   void writeLabel(const QString &l, bool isLast) override;
   void endLabels() override;

   void startFontClass(const QString &) override {}
   void endFontClass() override {}

   void writeCodeAnchor(const QString &) override {}
   void setCurrentDoc(QSharedPointer<Definition> d, const QString &, bool) override {}
   void addWord(const QString &word, bool hiPriority) override {}

   static bool preProcessFileInplace(const QString &path, const QString &name);

 private:
   RTFGenerator(const RTFGenerator &);
   RTFGenerator &operator=(const RTFGenerator &);

   QString rtf_BList_DepthStyle();
   QString rtf_CList_DepthStyle();
   QString rtf_EList_DepthStyle();
   QString rtf_LCList_DepthStyle();
   QString rtf_DList_DepthStyle();
   QString rtf_Code_DepthStyle();
   void incrementIndentLevel();
   void decrementIndentLevel();

   int  col;
   bool m_prettyCode;

   bool m_bstartedBody;     // has startbody been called yet?
   int  m_listLevel;        // RTF does not really have a addative indent...manually set list level.
   bool m_omitParagraph;    // should a the next paragraph command be ignored?
   int  m_numCols;          // number of columns in a table

   QString relPath;

   void beginRTFDocument();
   void beginRTFChapter();
   void beginRTFSection();
   void rtfwriteRuler_doubleline();
   void rtfwriteRuler_emboss();
   void rtfwriteRuler_thick();
   void rtfwriteRuler_thin();
   void writeRTFReference(const QString &label);
};

#endif
