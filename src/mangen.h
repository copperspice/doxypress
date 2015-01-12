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
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#ifndef MANGEN_H
#define MANGEN_H

#include <outputgen.h>

class QFile;

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

   void writeDoc(DocNode *, Definition *, MemberDef *);

   static void init();
   void startFile(const char *name, const char *manName, const char *title);
   void writeSearchInfo() {}
   void writeFooter(const char *) {}
   void endFile();
   void clearBuffer();

   void startIndexSection(IndexSections) {}
   void endIndexSection(IndexSections) {}
   void writePageLink(const char *, bool) {}
   void startProjectNumber() {}
   void endProjectNumber() {}
   void writeStyleInfo(int) {}
   void startTitleHead(const char *) {}
   void endTitleHead(const char *, const char *);
   void startTitle();
   void endTitle();

   void newParagraph();
   void startParagraph();
   void endParagraph();
   void writeString(const char *text);
   void startIndexListItem() {}
   void endIndexListItem() {}
   void startIndexList() {}

   void endIndexList()   {
      newParagraph();
   }

   void startIndexKey() {}
   void endIndexKey()   {}
   void startIndexValue(bool) {}
   void endIndexValue(const char *, bool)   {}
   void startItemList()  {}

   void endItemList()    {
      newParagraph();
   }

   void startIndexItem(const QByteArray &ref, const QByteArray &file) override;
   void endIndexItem(const QByteArray &ref, const QByteArray &file) override;

   void docify(const QByteArray &text) override;
   void codify(const QByteArray &text) override;

   void writeObjectLink(const QByteArray &ref, const QByteArray &file, const QByteArray &anchor, const QByteArray &name) override;
   void writeCodeLink(const QByteArray &ref, const QByteArray &file, const QByteArray &anchor, 
                      const QByteArray &name, const QByteArray &tooltip) override;

   void writeTooltip(const char *, const DocLinkInfo &, const QByteArray &, const QByteArray &, const SourceLinkInfo &,
                     const SourceLinkInfo &) override {}

   void startTextLink(const QByteArray &, const QByteArray &) override {}
   void endTextLink() override{}
   void startHtmlLink(const QByteArray &url) override;
   void endHtmlLink();

   void startTypewriter() {
      t << "\\fC";
      firstCol = false;
   }
   void endTypewriter()   {
      t << "\\fP";
      firstCol = false;
   }
   void startGroupHeader(int);
   void endGroupHeader(int);
   void startMemberSections() {}
   void endMemberSections() {}
   void startHeaderSection() {}
   void endHeaderSection();
   void startMemberHeader(const char *);
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
   void startMemberItem(const char *, int, const QByteArray &) override;
   void endMemberItem();
   void startMemberTemplateParams() {}
   void endMemberTemplateParams(const char *, const QByteArray &) override {}

   void startMemberGroupHeader(bool);
   void endMemberGroupHeader();
   void startMemberGroupDocs();
   void endMemberGroupDocs();
   void startMemberGroup();
   void endMemberGroup(bool);

   void writeRuler()    {}
   void writeAnchor(const char *, const char *) {}
   void startCodeFragment();
   void endCodeFragment();
   void writeLineNumber(const char *, const QByteArray &, const char *, int l) override {
      t << l << " ";
   }
   void startCodeLine(bool) {}
   void endCodeLine() {
      codify("\n");
      col = 0;
   }
   void startEmphasis() {
      t << "\\fI";
      firstCol = false;
   }
   void endEmphasis()   {
      t << "\\fP";
      firstCol = false;
   }
   void startBold()     {
      t << "\\fB";
      firstCol = false;
   }
   void endBold()       {
      t << "\\fP";
      firstCol = false;
   }
   void startDescription() {}
   void endDescription()   {}
   void startDescItem();
   void endDescItem();

   void lineBreak(const QByteArray &) override {
      t << "\n.br" << endl;
   }

   void writeChar(char c);
   void startMemberDoc(const char *, const char *, const char *, const char *, bool);
   void endMemberDoc(bool);
   void startDoxyAnchor(const char *, const char *, const char *, const char *, const char *);
   void endDoxyAnchor(const char *, const char *) {}
   void writeLatexSpacing() {}

   void writeStartAnnoItem(const char *type, const QByteArray &file, const QByteArray &path, const char *name) override;

   void writeEndAnnoItem(const char *) {
      t << endl;
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

   void startMemberDescription(const char *, const QByteArray &) {
      t << "\n.RI \"\\fI";
      firstCol = false;
   }

   void endMemberDescription()   {
      t << "\\fP\"";
      firstCol = false;
   }

   void startMemberDeclaration() {}
   void endMemberDeclaration(const char *, const QByteArray &) {}
   void writeInheritedSectionTitle(const char *, const QByteArray &, const char *, const char *, const char *, 
                                   const char *) override 
   {}

   void startDescList(SectionTypes);
   void endDescList() 
   {}

   void startSimpleSect(SectionTypes, const QByteArray &, const char *, const char *) override;
   void endSimpleSect();
   void startParamList(ParamListTypes, const char *title);
   void endParamList();

   //void writeDescItem();
   void startDescForItem();
   void endDescForItem();
   void startSection(const char *, const char *, SectionInfo::SectionType);
   void endSection(const char *, SectionInfo::SectionType);
   void addIndexItem(const char *, const char *) {}
   void startIndent()        {}
   void endIndent()          {}
   void writeSynopsis();
   void startClassDiagram() {}
   void endClassDiagram(const ClassDiagram &, const char *, const char *) {}
   void startPageRef() {}
   void endPageRef(const QByteArray &, const QByteArray &)override {}
   void startQuickIndices() {}
   void endQuickIndices() {}
   void writeSplitBar(const char *) {}
   void writeNavigationPath(const char *) {}
   void writeLogo() {}
   void writeQuickLinks(bool, HighlightedItem, const char *) {}
   void writeSummaryLink(const QByteArray &, const char *, const char *, bool) override {}
   void startContents() {}
   void endContents() {}

   void writeNonBreakableSpace(int n) {
      int i;
      for (i = 0; i < n; i++) {
         t << " ";
      }
   }

   void startDescTable(const char *t) {
      startSimpleSect(EnumValues, 0, 0, t);
      startDescForItem();
   }
   void endDescTable() {
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
      t << endl;
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
   void startParameterType(bool, const QByteArray &) override {}
   void endParameterType() {}
   void startParameterName(bool) {}
   void endParameterName(bool, bool, bool) {}
   void startParameterList(bool) {}
   void endParameterList() {}
   void exceptionEntry(const QByteArray &, bool) override {}

   void startFontClass(const char *) {}
   void endFontClass() {}

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

   void writeCodeAnchor(const char *) {}

   void setCurrentDoc(Definition *, const char *, bool) {}

   void addWord(const QString &word, bool hiPriority) override
   {}

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
