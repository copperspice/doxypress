/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim
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

#ifndef OUTPUTLIST_H
#define OUTPUTLIST_H

#include <QList>

#include <index.h>
#include <outputgen.h>
#include <section.h>

class ClassDiagram;
class DotClassGraph;
class DotDirDeps;
class DotInclDepGraph;
class DotGfxHierarchyTable;
class DotGroupCollaboration;
class DocRoot;

/** Class representing a list of output generators that are written to
 *  in parallel.
 */
class OutputList : public OutputDocInterface
{
 public:
   OutputList();
   OutputList(const OutputList &dummy) = delete;

   virtual ~OutputList();

   void add(QSharedPointer<OutputGenerator> og);

   uint count() const {
      return m_outputs.count();
   }

   void disableAllBut(OutputGenerator::OutputType o) override;
   void enableAll() override;
   void disableAll() override;
   void disable(OutputGenerator::OutputType o) override;
   void enable(OutputGenerator::OutputType o) override;
   bool isEnabled(OutputGenerator::OutputType o) override;
   void pushGeneratorState() override;
   void popGeneratorState() override;


   //  OutputDocInterface implementation
   bool generateDoc(const QString &fileName, int startLine, QSharedPointer<Definition> ctx, QSharedPointer<MemberDef> md,
                    const QString &docStr,bool indexWords, bool isExample, const QString &exampleName = 0,
                    bool singleLine = false, bool linkFromIndex = false);

   void writeDoc(DocRoot *root, QSharedPointer<Definition> ctx, QSharedPointer<MemberDef> md);

   bool parseText(const QString &textStr) override;

   void startIndexSection(IndexSections is) {
      forall(&OutputGenerator::startIndexSection, is);
   }

   void endIndexSection(IndexSections is) {
      forall(&OutputGenerator::endIndexSection, is);
   }

   void writePageLink(const QString &name, bool first) {
      forall(&OutputGenerator::writePageLink, name, first);
   }

   void startProjectNumber() {
      forall(&OutputGenerator::startProjectNumber);
   }

   void endProjectNumber() {
      forall(&OutputGenerator::endProjectNumber);
   }

   void writeStyleInfo(int part) {
      forall(&OutputGenerator::writeStyleInfo, part);
   }

   void startFile(const QString &name, const QString &manName, const QString &title) {
      forall(&OutputGenerator::startFile, name, manName, title);
   }

   void writeSearchInfo() {
      forall(&OutputGenerator::writeSearchInfo);
   }

   void writeFooter(const QString &navPath) {
      forall(&OutputGenerator::writeFooter, navPath);
   }

   void endFile() {
      forall(&OutputGenerator::endFile);
   }

   void startTitleHead(const QString &fileName) {
      forall(&OutputGenerator::startTitleHead, fileName);
   }

   void endTitleHead(const QString &fileName, const QString &name) {
      forall(&OutputGenerator::endTitleHead, fileName, name);
   }

   void startTitle() override {
      forall(&OutputGenerator::startTitle);
   }

   void endTitle() override {
      forall(&OutputGenerator::endTitle);
   }

   // void newParagraph()
   //    { forall(&OutputGenerator::newParagraph); }

   void startParagraph(const QString &className = "") override {
      forall(&OutputGenerator::startParagraph, className);
   }

   void endParagraph() override {
      forall(&OutputGenerator::endParagraph);
   }

   void writeString(const QString &text) override {
      forall(&OutputGenerator::writeString, text);
   }

   void startIndexListItem() {
      forall(&OutputGenerator::startIndexListItem);
   }

   void endIndexListItem() {
      forall(&OutputGenerator::endIndexListItem);
   }

   void startIndexList() {
      forall(&OutputGenerator::startIndexList);
   }

   void endIndexList() {
      forall(&OutputGenerator::endIndexList);
   }

   void startIndexKey() {
      forall(&OutputGenerator::startIndexKey);
   }

   void endIndexKey() {
      forall(&OutputGenerator::endIndexKey);
   }

   void startIndexValue(bool b) {
      forall(&OutputGenerator::startIndexValue, b);
   }

   void endIndexValue(const QString &name, bool b) {
      forall(&OutputGenerator::endIndexValue, name, b);
   }

   void startItemList() override {
      forall(&OutputGenerator::startItemList);
   }

   void endItemList() override {
      forall(&OutputGenerator::endItemList);
   }

   void startIndexItem(const QString &ref, const QString &file) {
      forall(&OutputGenerator::startIndexItem, ref, file);
   }

   void endIndexItem(const QString &ref, const QString &file) {
      forall(&OutputGenerator::endIndexItem, ref, file);
   }

   void docify(const QString &text)  override {
      forall(&OutputGenerator::docify, text);
   }

   void codify(const QString &s)  override {
      forall(&OutputGenerator::codify, s);
   }

   void writeObjectLink(const QString &ref, const QString &file, const QString &anchor, const QString &name) override {
      forall(&OutputGenerator::writeObjectLink, ref, file, anchor, name);
   }

   void writeCodeLink(const QString &ref, const QString &file, const QString &anchor, const QString &name,
                      const QString &tooltip)  override
   {
      forall(&OutputGenerator::writeCodeLink, ref, file, anchor, name, tooltip);
   }

   void writeTooltip(const QString &id, const DocLinkInfo &docInfo, const QString &decl, const QString &desc,
                     const SourceLinkInfo &defInfo, const SourceLinkInfo &declInfo)  override {
      forall(&OutputGenerator::writeTooltip, id, docInfo, decl, desc, defInfo, declInfo);
   }

   void startTextLink(const QString &file, const QString &anchor)  override{
      forall(&OutputGenerator::startTextLink, file, anchor);
   }

   void endTextLink() override {
      forall(&OutputGenerator::endTextLink);
   }

   void startHtmlLink(const QString &url)  override{
      forall(&OutputGenerator::startHtmlLink, url);
   }

   void endHtmlLink() override {
      forall(&OutputGenerator::endHtmlLink);
   }

   void writeStartAnnoItem(const QString &type, const QString &file, const QString &path, const QString &name) {
      forall(&OutputGenerator::writeStartAnnoItem, type, file, path, name);
   }

   void writeEndAnnoItem(const QString &name) {
      forall(&OutputGenerator::writeEndAnnoItem, name);
   }

   void startTypewriter()  override {
      forall(&OutputGenerator::startTypewriter);
   }

   void endTypewriter()  override {
      forall(&OutputGenerator::endTypewriter);
   }

   void startGroupHeader(int extraLevels = 0) {
      forall(&OutputGenerator::startGroupHeader, extraLevels);
   }

   void endGroupHeader(int extraLevels = 0) {
      forall(&OutputGenerator::endGroupHeader, extraLevels);
   }

   // void writeListItem()
   // { forall(&OutputGenerator::writeListItem); }

   void startItemListItem() override {
      forall(&OutputGenerator::startItemListItem);
   }
   void endItemListItem() override {
      forall(&OutputGenerator::endItemListItem);
   }

   void startMemberSections() {
      forall(&OutputGenerator::startMemberSections);
   }

   void endMemberSections() {
      forall(&OutputGenerator::endMemberSections);
   }
   void startHeaderSection() {
      forall(&OutputGenerator::startHeaderSection);
   }
   void endHeaderSection() {
      forall(&OutputGenerator::endHeaderSection);
   }
   void startMemberHeader(const QString &anchor) {
      forall(&OutputGenerator::startMemberHeader, anchor);
   }
   void endMemberHeader() {
      forall(&OutputGenerator::endMemberHeader);
   }
   void startMemberSubtitle() {
      forall(&OutputGenerator::startMemberSubtitle);
   }
   void endMemberSubtitle() {
      forall(&OutputGenerator::endMemberSubtitle);
   }
   void startMemberDocList() {
      forall(&OutputGenerator::startMemberDocList);
   }
   void endMemberDocList() {
      forall(&OutputGenerator::endMemberDocList);
   }
   void startMemberList() {
      forall(&OutputGenerator::startMemberList);
   }
   void endMemberList() {
      forall(&OutputGenerator::endMemberList);
   }
   void startInlineHeader() {
      forall(&OutputGenerator::startInlineHeader);
   }
   void endInlineHeader() {
      forall(&OutputGenerator::endInlineHeader);
   }
   void startAnonTypeScope(int i1) {
      forall(&OutputGenerator::startAnonTypeScope, i1);
   }

   void endAnonTypeScope(int i1) {
      forall(&OutputGenerator::endAnonTypeScope, i1);
   }

   void startMemberItem(const QString &anchor, int i1, const QString &id = QString() ) {
      forall(&OutputGenerator::startMemberItem, anchor, i1, id);
   }

   void endMemberItem() {
      forall(&OutputGenerator::endMemberItem);
   }

   void startMemberTemplateParams() {
      forall(&OutputGenerator::startMemberTemplateParams);
   }

   void endMemberTemplateParams(const QString &anchor, const QString &inheritId) {
      forall(&OutputGenerator::endMemberTemplateParams, anchor, inheritId);
   }
   void startMemberGroupHeader(bool b) {
      forall(&OutputGenerator::startMemberGroupHeader, b);
   }
   void endMemberGroupHeader() {
      forall(&OutputGenerator::endMemberGroupHeader);
   }
   void startMemberGroupDocs() {
      forall(&OutputGenerator::startMemberGroupDocs);
   }
   void endMemberGroupDocs() {
      forall(&OutputGenerator::endMemberGroupDocs);
   }
   void startMemberGroup() {
      forall(&OutputGenerator::startMemberGroup);
   }
   void endMemberGroup(bool last) {
      forall(&OutputGenerator::endMemberGroup, last);
   }
 
  void insertMemberAlign(bool templ = false) {
      forall(&OutputGenerator::insertMemberAlign, templ);
   }

   void writeRuler() override {
      forall(&OutputGenerator::writeRuler);
   }
 
  void writeAnchor(const QString &fileName, const QString &name) override {
      forall(&OutputGenerator::writeAnchor, fileName, name);
   }

   void startCodeFragment() override {
      forall(&OutputGenerator::startCodeFragment);
   }

   void endCodeFragment() override {
      forall(&OutputGenerator::endCodeFragment);
   }

   void startCodeLine(bool hasLineNumbers) override {
      forall(&OutputGenerator::startCodeLine, hasLineNumbers);
   }
 
   void endCodeLine() override {
      forall(&OutputGenerator::endCodeLine);
   }

   void writeLineNumber(const QString &ref, const QString &file, const QString &anchor, int lineNumber) override {
      forall(&OutputGenerator::writeLineNumber, ref, file, anchor, lineNumber);
   }

   void startEmphasis() override {
      forall(&OutputGenerator::startEmphasis);
   }

   void endEmphasis() override {
      forall(&OutputGenerator::endEmphasis);
   }

   void writeChar(char c) override {
      forall(&OutputGenerator::writeChar, c);
   }

   void startMemberDoc(const QString &clName, const QString &memName,
                       const QString &anchor, const QString &title, bool showInline) {
      forall(&OutputGenerator::startMemberDoc, clName, memName, anchor, title, showInline);
   }
   void endMemberDoc(bool hasArgs) {
      forall(&OutputGenerator::endMemberDoc, hasArgs);
   }

   void startDoxyAnchor(const QString &fName, const QString &manName,
                        const QString &anchor, const QString &name, const QString &args) {
      forall(&OutputGenerator::startDoxyAnchor, fName, manName, anchor, name, args);
   }

   void endDoxyAnchor(const QString &fn, const QString &anchor) {
      forall(&OutputGenerator::endDoxyAnchor, fn, anchor);
   }

   void writeLatexSpacing() {
      forall(&OutputGenerator::writeLatexSpacing);
   }

   void startDescription() override {
      forall(&OutputGenerator::startDescription);
   }

   void endDescription() override {
      forall(&OutputGenerator::endDescription);
   }

   void startDescItem() override {
      forall(&OutputGenerator::startDescItem);
   }

   void endDescItem() override {
      forall(&OutputGenerator::endDescItem);
   }

   void startDescForItem() override {
      forall(&OutputGenerator::startDescForItem);
   }

   void endDescForItem() override {
      forall(&OutputGenerator::endDescForItem);
   }

   void startSubsection() override {
      forall(&OutputGenerator::startSubsection);
   }

   void endSubsection() override {
      forall(&OutputGenerator::endSubsection);
   }

   void startSubsubsection() override {
      forall(&OutputGenerator::startSubsubsection);
   }

   void endSubsubsection() override {
      forall(&OutputGenerator::endSubsubsection);
   }

   void startCenter() override {
      forall(&OutputGenerator::startCenter);
   }

   void endCenter() override {
      forall(&OutputGenerator::endCenter);
   }

   void startSmall() override {
      forall(&OutputGenerator::startSmall);
   }

   void endSmall() override {
      forall(&OutputGenerator::endSmall);
   }

   void lineBreak(const QString &style = 0)  override {
      forall(&OutputGenerator::lineBreak, style);
   }

   void startBold() override {
      forall(&OutputGenerator::startBold);
   }

   void endBold() override {
      forall(&OutputGenerator::endBold);
   }

   void startMemberDescription(const QString &anchor, const QString &inheritId = 0) {
      forall(&OutputGenerator::startMemberDescription, anchor, inheritId);
   }

   void endMemberDescription() {
      forall(&OutputGenerator::endMemberDescription);
   }

   void startMemberDeclaration() {
      forall(&OutputGenerator::startMemberDeclaration);
   }

   void endMemberDeclaration(const QString &anchor, const QString &inheritId) {
      forall(&OutputGenerator::endMemberDeclaration, anchor, inheritId);
   }

   void writeInheritedSectionTitle(const QString &id,   const QString &ref, const QString &file, const QString &anchor,
                                   const QString &title, const QString &name) {
      forall(&OutputGenerator::writeInheritedSectionTitle, id, ref, file, anchor, title, name);
   }

   void startSimpleSect(SectionTypes t, const QString &file, const QString &anchor, const QString &title) override {
      forall(&OutputGenerator::startSimpleSect, t, file, anchor, title);
   }

   void endSimpleSect() override {
      forall(&OutputGenerator::endSimpleSect);
   }

   void startParamList(ParamListTypes t, const QString &title) override {
      forall(&OutputGenerator::startParamList, t, title);
   }

   void endParamList() override {
      forall(&OutputGenerator::endParamList);
   }

   void startIndent() {
      forall(&OutputGenerator::startIndent);
   }

   void endIndent() {
      forall(&OutputGenerator::endIndent);
   }

   void startSection(const QString &lab, const QString &title, SectionInfo::SectionType t) override {
      forall(&OutputGenerator::startSection, lab, title, t);
   }

   void endSection(const QString &lab, SectionInfo::SectionType t) override {
      forall(&OutputGenerator::endSection, lab, t);
   }

   void addIndexItem(const QString &s1, const QString &s2) override {
      forall(&OutputGenerator::addIndexItem, s1, s2);
   }

   void writeSynopsis() {
      forall(&OutputGenerator::writeSynopsis);
   }

   void startClassDiagram() {
      forall(&OutputGenerator::startClassDiagram);
   }

   void endClassDiagram(const ClassDiagram &d, const QString &f, const QString &n) {
      forall(&OutputGenerator::endClassDiagram, d, f, n);
   }

   void startPageRef() override {
      forall(&OutputGenerator::startPageRef);
   }

   void endPageRef(const QString &c, const QString &a) override {
      forall(&OutputGenerator::endPageRef, c, a);
   }

   void startQuickIndices() {
      forall(&OutputGenerator::startQuickIndices);
   }

   void endQuickIndices() {
      forall(&OutputGenerator::endQuickIndices);
   }

   void writeSplitBar(const QString &name) {
      forall(&OutputGenerator::writeSplitBar, name);
   }

   void writeNavigationPath(const QString &s) {
      forall(&OutputGenerator::writeNavigationPath, s);
   }
   void writeLogo() {
      forall(&OutputGenerator::writeLogo);
   }

   void writeQuickLinks(bool compact, HighlightedItem hli, const QString &file) {
      forall(&OutputGenerator::writeQuickLinks, compact, hli, file);
   }

   void writeSummaryLink(const QString &file, const QString &anchor, const QString &title, bool first) {
      forall(&OutputGenerator::writeSummaryLink, file, anchor, title, first);
   }

   void startContents() {
      forall(&OutputGenerator::startContents);
   }

   void endContents() {
      forall(&OutputGenerator::endContents);
   }

   void writeNonBreakableSpace(int num) override {
      forall(&OutputGenerator::writeNonBreakableSpace, num);
   }

   void startEnumTable() override {
      forall(&OutputGenerator::startEnumTable);
   }

   void endEnumTable() override {
      forall(&OutputGenerator::endEnumTable);
   }

   void startDescTableTitle() override {
      forall(&OutputGenerator::startDescTableTitle);
   }

   void endDescTableTitle() override {
      forall(&OutputGenerator::endDescTableTitle);
   }

   void startDescTableData() override {
      forall(&OutputGenerator::startDescTableData);
   }

   void endDescTableData() override {
      forall(&OutputGenerator::endDescTableData);
   }

   void startDotGraph() {
      forall(&OutputGenerator::startDotGraph);
   }

   void endDotGraph(const DotClassGraph &g) {
      forall(&OutputGenerator::endDotGraph, g);
   }

   void startInclDepGraph() {
      forall(&OutputGenerator::startInclDepGraph);
   }

   void endInclDepGraph(const DotInclDepGraph &g) {
      forall(&OutputGenerator::endInclDepGraph, g);
   }

   void startCallGraph() {
      forall(&OutputGenerator::startCallGraph);
   }

   void endCallGraph(const DotCallGraph &g) {
      forall(&OutputGenerator::endCallGraph, g);
   }

   void startDirDepGraph() {
      forall(&OutputGenerator::startDirDepGraph);
   }

   void endDirDepGraph(const DotDirDeps &g) {
      forall(&OutputGenerator::endDirDepGraph, g);
   }

   void startGroupCollaboration() {
      forall(&OutputGenerator::startGroupCollaboration);
   }

   void endGroupCollaboration(const DotGroupCollaboration &g) {
      forall(&OutputGenerator::endGroupCollaboration, g);
   }

   void writeGraphicalHierarchy(const DotGfxHierarchyTable &g) {
      forall(&OutputGenerator::writeGraphicalHierarchy, g);
   }

   void startTextBlock(bool dense = false) {
      forall(&OutputGenerator::startTextBlock, dense);
   }

   void endTextBlock(bool paraBreak = false) {
      forall(&OutputGenerator::endTextBlock, paraBreak);
   }

   void lastIndexPage() {
      forall(&OutputGenerator::lastIndexPage);
   }

   void startMemberDocPrefixItem() {
      forall(&OutputGenerator::startMemberDocPrefixItem);
   }

   void endMemberDocPrefixItem() {
      forall(&OutputGenerator::endMemberDocPrefixItem);
   }

   void startMemberDocName(bool align) {
      forall(&OutputGenerator::startMemberDocName, align);
   }

   void endMemberDocName() {
      forall(&OutputGenerator::endMemberDocName);
   }

   void startParameterType(bool first, const QString &key) {
      forall(&OutputGenerator::startParameterType, first, key);
   }

   void endParameterType() {
      forall(&OutputGenerator::endParameterType);
   }

   void startParameterName(bool one) {
      forall(&OutputGenerator::startParameterName, one);
   }

   void endParameterName(bool last, bool one, bool bracket) {
      forall(&OutputGenerator::endParameterName, last, one, bracket);
   }

   void startParameterList(bool openBracket) {
      forall(&OutputGenerator::startParameterList, openBracket);
   }

   void endParameterList() {
      forall(&OutputGenerator::endParameterList);
   }

   void exceptionEntry(const QString &prefix, bool closeBracket) {
      forall(&OutputGenerator::exceptionEntry, prefix, closeBracket);
   }

   void startConstraintList(const QString &header) {
      forall(&OutputGenerator::startConstraintList, header);
   }

   void startConstraintParam() {
      forall(&OutputGenerator::startConstraintParam);
   }

   void endConstraintParam() {
      forall(&OutputGenerator::endConstraintParam);
   }

   void startConstraintType() {
      forall(&OutputGenerator::startConstraintType);
   }

   void endConstraintType() {
      forall(&OutputGenerator::endConstraintType);
   }

   void startConstraintDocs() {
      forall(&OutputGenerator::startConstraintDocs);
   }

   void endConstraintDocs() {
      forall(&OutputGenerator::endConstraintDocs);
   }

   void endConstraintList() {
      forall(&OutputGenerator::endConstraintList);
   }

   void startMemberDocSimple() {
      forall(&OutputGenerator::startMemberDocSimple);
   }

   void endMemberDocSimple() {
      forall(&OutputGenerator::endMemberDocSimple);
   }

   void startInlineMemberType() {
      forall(&OutputGenerator::startInlineMemberType);
   }

   void endInlineMemberType() {
      forall(&OutputGenerator::endInlineMemberType);
   }

   void startInlineMemberName() {
      forall(&OutputGenerator::startInlineMemberName);
   }
   void endInlineMemberName() {
      forall(&OutputGenerator::endInlineMemberName);
   }
   void startInlineMemberDoc() {
      forall(&OutputGenerator::startInlineMemberDoc);
   }
   void endInlineMemberDoc() {
      forall(&OutputGenerator::endInlineMemberDoc);
   }

   void startLabels() {
      forall(&OutputGenerator::startLabels);
   }

   void writeLabel(const QString &l, bool isLast) {
      forall(&OutputGenerator::writeLabel, l, isLast);
   }

   void endLabels() {
      forall(&OutputGenerator::endLabels);
   }

   void startFontClass(const QString &c) override {
      forall(&OutputGenerator::startFontClass, c);
   }

   void endFontClass() override {
      forall(&OutputGenerator::endFontClass);
   }

   void writeCodeAnchor(const QString &name) override {
      forall(&OutputGenerator::writeCodeAnchor, name);
   }

   void setCurrentDoc(QSharedPointer<Definition> context, const QString &anchor, bool isSourceFile) override {
      forall(&OutputGenerator::setCurrentDoc, context, anchor, isSourceFile);
   }

   void addWord(const QString &word, bool hiPriority) override {
      forall(&OutputGenerator::addWord, word, hiPriority);
   }

   void startPlainFile(const QString &name) {
      for (auto item : m_outputs) {
         if (item->isEnabled()) {
            (item->startPlainFile)(name);
         }
      }
   }

   void endPlainFile() {
      for (auto item : m_outputs) {
         if (item->isEnabled()) {
            (item->endPlainFile)();
         }
      }
   }

 private:
   void debug();
   void clear();

   template<class BaseClass, class... Args, class... Ts>
   void forall( void (BaseClass::*func)(Args...), Ts&&... vs)  {

      for (auto item : m_outputs ) {
         if (item->isEnabled()) {
            ((*item).*func)(vs...);           // BROOM - resolve when we update CS with operator->
         }
      }
   }

   QList<QSharedPointer<OutputGenerator>> m_outputs;
};

#endif
