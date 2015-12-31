/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim 
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
      if (o == RTF) {
         enable();
      }
   }
   void disableIf(OutputType o) {
      if (o == RTF) {
         disable();
      }
   }
   void disableIfNot(OutputType o) {
      if (o != RTF) {
         disable();
      }
   }
   bool isEnabled(OutputType o) {
      return (o == RTF && active);
   }
   OutputGenerator *get(OutputType o) {
      return (o == RTF) ? this : 0;
   }

   void writeDoc(DocNode *, QSharedPointer<Definition> ctx, QSharedPointer<MemberDef> md);

   void startFile(const QString &name, const QString &manName, const QString &title) override;
   void writeSearchInfo() {}
   void writeFooter(const QString &) override {}
   void endFile();
   void clearBuffer();

   void startIndexSection(IndexSections);
   void endIndexSection(IndexSections);
   void writePageLink(const QString &, bool) override;
   void startProjectNumber();
   void endProjectNumber();
   void writeStyleInfo(int part);
   void startTitleHead(const QString &) override;
   void startTitle();
   void endTitleHead(const QString &, const QString &name) override;
   void endTitle() {}

   void newParagraph();
   void startParagraph();
   void endParagraph();
   void writeString(const QString &text);
   void startIndexListItem();
   void endIndexListItem();
   void startIndexList();
   void endIndexList();
   void startIndexKey();
   void endIndexKey();
   void startIndexValue(bool);
   void endIndexValue(const QString &, bool) override;
   void startItemList();
   void endItemList();

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
   void endTextLink();
   void startHtmlLink(const QString &url) override;
   void endHtmlLink();

   void startTypewriter() {
      m_textStream << "{\\f2 ";
   }

   void endTypewriter()   {
      m_textStream << "}";
   }

   void startGroupHeader(int);
   void endGroupHeader(int);
  
   void startItemListItem();
   void endItemListItem();

   void startMemberSections() {}
   void endMemberSections() {}
   void startHeaderSection() {}
   void endHeaderSection() {}

   void startMemberHeader(const QString &) override {
      startGroupHeader(false);
   }

   void endMemberHeader() {
      endGroupHeader(false);
   }

   void startMemberSubtitle();
   void endMemberSubtitle();
   void startMemberDocList() {}
   void endMemberDocList() {}
   void startMemberList();
   void endMemberList();
   void startInlineHeader();
   void endInlineHeader();
   void startAnonTypeScope(int) {}
   void endAnonTypeScope(int) {}
   void startMemberItem(const QString &, int, const QString &) override;
   void endMemberItem();
   void startMemberTemplateParams() {}
   void endMemberTemplateParams(const QString &, const QString &) override {}
   void insertMemberAlign(bool) {}

   void writeRuler() {
      rtfwriteRuler_thin();
   }

   void writeAnchor(const QString &fileName, const QString &name) override;
   void startCodeFragment();
   void endCodeFragment();
   void writeLineNumber(const QString &, const QString &, const QString &, int l) override {
      m_textStream << QString("%1").arg(l, 5) << " ";
   }
   void startCodeLine(bool) {
      col = 0;
   }
   void endCodeLine() {
      lineBreak();
   }
   void startEmphasis() {
      m_textStream << "{\\i ";
   }
   void endEmphasis()   {
      m_textStream << "}";
   }
   void startBold()     {
      m_textStream << "{\\b ";
   }
   void endBold()       {
      m_textStream << "}";
   }
   void startDescription();
   void endDescription();
   void startDescItem();
   void endDescItem();

   void lineBreak(const QString &style = 0) override;

   void startMemberDoc(const QString &, const QString &, const QString &, const QString &, bool) override;
   void endMemberDoc(bool);
   void startDoxyAnchor(const QString &, const QString &, const QString &, const QString &, const QString &) override;
   void endDoxyAnchor(const QString &, const QString &) override;
   void writeChar(char c);

   void writeLatexSpacing() {};      
   
   void writeStartAnnoItem(const QString &type, const QString &file, const QString &path, const QString &name) override;
   void writeEndAnnoItem(const QString &name) override;

   void startSubsection();
   void endSubsection();
   void startSubsubsection();
   void endSubsubsection();
   void startCenter()      {
      m_textStream << "{\\qc" << endl;
   }
   void endCenter()        {
      m_textStream << "}";
   }
   void startSmall()       {
      m_textStream << "{\\sub ";
   }
   void endSmall()         {
      m_textStream << "}";
   }

   void startMemberDescription(const QString &, const QString &) override;
   void endMemberDescription();
   void startMemberDeclaration() {}
   void endMemberDeclaration(const QString &, const QString &) override {}

   void writeInheritedSectionTitle(const QString &, const QString &, const QString &,
                                   const QString &, const QString &, const QString &) override {}

   void startDescList(SectionTypes);
   void startSimpleSect(SectionTypes, const QString &, const QString &, const QString &) override;
   void endSimpleSect();
   void startParamList(ParamListTypes, const QString &) override;
   void endParamList();

   //void writeDescItem();
   void startDescForItem();
   void endDescForItem();
   void startSection(const QString &, const QString &, SectionInfo::SectionType) override;
   void endSection(const QString &, SectionInfo::SectionType) override;
   void addIndexItem(const QString &, const QString &) override;
   void startIndent();
   void endIndent();
   void writeSynopsis()     {}
   void startClassDiagram();
   void endClassDiagram(const ClassDiagram &, const QString &filename, const QString &name) override ;
   void startPageRef();
   void endPageRef(const QString &, const QString &) override;
   void startQuickIndices() {}
   void endQuickIndices() {}
   void writeSplitBar(const QString &) override {}
   void writeNavigationPath(const QString &) override {}
   void writeLogo() {}
   void writeQuickLinks(bool, HighlightedItem, const QString &) override {}
   void writeSummaryLink(const QString &, const QString &, const QString &, bool) override {}
   void startContents() {}
   void endContents() {}
   void writeNonBreakableSpace(int);

   void startEnumTable();
   void endEnumTable();
   void startDescTableTitle();
   void endDescTableTitle();
   void startDescTableData();
   void endDescTableData();

   void startDotGraph();
   void endDotGraph(const DotClassGraph &);
   void startInclDepGraph();
   void endInclDepGraph(const DotInclDepGraph &);
   void startGroupCollaboration();
   void endGroupCollaboration(const DotGroupCollaboration &g);
   void startCallGraph();
   void endCallGraph(const DotCallGraph &);
   void startDirDepGraph();
   void endDirDepGraph(const DotDirDeps &g);
   void writeGraphicalHierarchy(const DotGfxHierarchyTable &) {}

   void startMemberGroupHeader(bool);
   void endMemberGroupHeader();
   void startMemberGroupDocs();
   void endMemberGroupDocs();
   void startMemberGroup();
   void endMemberGroup(bool);

   void startTextBlock(bool dense);
   void endTextBlock(bool);
   void lastIndexPage();

   void startMemberDocPrefixItem() {}
   void endMemberDocPrefixItem() {}
   void startMemberDocName(bool) {}
   void endMemberDocName() {}
   void startParameterType(bool, const QString &) override;
   void endParameterType();
   void startParameterName(bool) {}
   void endParameterName(bool, bool, bool) {}
   void startParameterList(bool) {}
   void endParameterList() {}
   void exceptionEntry(const QString &, bool) override ;

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

   void startFontClass(const QString &) override {}
   void endFontClass() {}

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
