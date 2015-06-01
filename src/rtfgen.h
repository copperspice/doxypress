/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
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
   //void postProcess(QByteArray &);

   void startIndexSection(IndexSections);
   void endIndexSection(IndexSections);
   void writePageLink(const char *, bool);
   void startProjectNumber();
   void endProjectNumber();
   void writeStyleInfo(int part);
   void startTitleHead(const char *);
   void startTitle();
   void endTitleHead(const char *, const char *name);
   void endTitle() {}

   void newParagraph();
   void startParagraph();
   void endParagraph();
   void writeString(const char *text);
   void startIndexListItem();
   void endIndexListItem();
   void startIndexList();
   void endIndexList();
   void startIndexKey();
   void endIndexKey();
   void startIndexValue(bool);
   void endIndexValue(const char *, bool);
   void startItemList();
   void endItemList();

   void startIndexItem(const QByteArray &ref, const QByteArray &file) override;
   void endIndexItem(const QByteArray &ref, const QByteArray &file) override;

   void docify(const QByteArray &text) override;
   void codify(const QByteArray &text) override;

   void writeObjectLink(const QByteArray &ref, const QByteArray &file, const QByteArray &anchor, const QByteArray &name) override;

   void writeCodeLink(const QByteArray &ref, const QByteArray &file, const QByteArray &anchor, 
                      const QByteArray &name, const QByteArray &tooltip) override;  

   void writeTooltip(const char *, const DocLinkInfo &, const QByteArray &, const QByteArray &, 
                     const SourceLinkInfo &, const SourceLinkInfo & ) override {}

   void startTextLink(const QByteArray &f, const QByteArray &anchor) override;
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
   //void writeListItem();
   void startItemListItem();
   void endItemListItem();

   void startMemberSections() {}
   void endMemberSections() {}
   void startHeaderSection() {}
   void endHeaderSection() {}
   void startMemberHeader(const char *) {
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
   void startMemberItem(const char *, int, const QByteArray &) override;
   void endMemberItem();
   void startMemberTemplateParams() {}
   void endMemberTemplateParams(const char *, const QByteArray &) override {}
   void insertMemberAlign(bool) {}

   void writeRuler() {
      rtfwriteRuler_thin();
   }

   void writeAnchor(const char *fileName, const char *name);
   void startCodeFragment();
   void endCodeFragment();
   void writeLineNumber(const char *, const QByteArray &, const char *, int l) override {
      m_textStream << l << " ";
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

   void lineBreak(const QByteArray &style = 0) override;

   void startMemberDoc(const char *, const char *, const char *, const char *, bool);
   void endMemberDoc(bool);
   void startDoxyAnchor(const char *, const char *, const char *, const char *, const char *);
   void endDoxyAnchor(const char *, const char *);
   void writeChar(char c);

   void writeLatexSpacing() {};      
 // { m_textStream << "\\hspace{0.3cm}"; }

   void writeStartAnnoItem(const char *type, const QByteArray &file, const QByteArray &path, const char *name) override;
   void writeEndAnnoItem(const char *name) override;

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

   void startMemberDescription(const char *, const QByteArray &);
   void endMemberDescription();
   void startMemberDeclaration() {}
   void endMemberDeclaration(const char *, const QByteArray &) {}

   void writeInheritedSectionTitle(const char *, const QByteArray &, const char *,
                                   const char *, const char *, const char *) override {}

   void startDescList(SectionTypes);
   void startSimpleSect(SectionTypes, const QByteArray &, const char *, const char *) override;
   void endSimpleSect();
   void startParamList(ParamListTypes, const char *);
   void endParamList();

   //void writeDescItem();
   void startDescForItem();
   void endDescForItem();
   void startSection(const char *, const char *, SectionInfo::SectionType);
   void endSection(const char *, SectionInfo::SectionType);
   void addIndexItem(const char *, const char *);
   void startIndent();
   void endIndent();
   void writeSynopsis()     {}
   void startClassDiagram();
   void endClassDiagram(const ClassDiagram &, const char *filename, const char *name);
   void startPageRef();
   void endPageRef(const QByteArray &, const QByteArray &) override;
   void startQuickIndices() {}
   void endQuickIndices() {}
   void writeSplitBar(const QString &) override {}
   void writeNavigationPath(const char *) {}
   void writeLogo() {}
   void writeQuickLinks(bool, HighlightedItem, const QString &) override {}
   void writeSummaryLink(const QByteArray &, const char *, const char *, bool) override {}
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
   void startParameterType(bool, const QByteArray &) override;
   void endParameterType();
   void startParameterName(bool) {}
   void endParameterName(bool, bool, bool) {}
   void startParameterList(bool) {}
   void endParameterList() {}
   void exceptionEntry(const QByteArray &, bool) override ;

   void startConstraintList(const char *);
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
   void writeLabel(const char *l, bool isLast);
   void endLabels();

   void startFontClass(const char *) {}
   void endFontClass() {}

   void writeCodeAnchor(const char *) {}
   void setCurrentDoc(QSharedPointer<Definition> d, const char *, bool) override {}
   void addWord(const QString &word, bool hiPriority) override {}

   static bool preProcessFileInplace(const QString &path, const QString &name);

 private:
   RTFGenerator(const RTFGenerator &);
   RTFGenerator &operator=(const RTFGenerator &);

   const char *rtf_BList_DepthStyle();
   const char *rtf_CList_DepthStyle();
   const char *rtf_EList_DepthStyle();
   const char *rtf_LCList_DepthStyle();
   const char *rtf_DList_DepthStyle();
   const char *rtf_Code_DepthStyle();
   void incrementIndentLevel();
   void decrementIndentLevel();
   int  col;

   bool m_bstartedBody;  // has startbody been called yet?
   int  m_listLevel; // // RTF does not really have a addative indent...manually set list level.
   bool m_omitParagraph; // should a the next paragraph command be ignored?
   int  m_numCols; // number of columns in a table
   QByteArray relPath;

   void beginRTFDocument();
   void beginRTFChapter();
   void beginRTFSection();
   void rtfwriteRuler_doubleline();
   void rtfwriteRuler_emboss();
   void rtfwriteRuler_thick();
   void rtfwriteRuler_thin();
   void writeRTFReference(const char *label);

   //char *getMultiByte(int c);
};

#endif
