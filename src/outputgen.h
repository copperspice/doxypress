/************************************************************************
*
* Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
* Copyright (c) 1997-2014 Dimitri van Heesch
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

#ifndef OUTPUTGEN_H
#define OUTPUTGEN_H

#include <index.h>
#include <section.h>

#include <QFile>
#include <QStack>
#include <QTextStream>

class ClassDiagram;
class Definition;
class DocNode;
class DotCallGraph;
class DotClassGraph;
class DotDirDeps;
class DotGfxHierarchyTable;
class DotGroupCollaboration;
class DotInclDepGraph;
class GroupDef;
class MemberDef;

struct DocLinkInfo {
   QString name;
   QString ref;
   QString url;
   QString anchor;
};

struct SourceLinkInfo {
   QString file;
   int line;
   QString ref;
   QString url;
   QString anchor;
};

// Output interface for code parser
class CodeGenerator
{
 public:
   virtual ~CodeGenerator()
   { }

   virtual void addWord(const QString &word, bool hiPriority) = 0;
   virtual void codify(const QString &text) = 0;
   virtual void setCurrentDoc(QSharedPointer<Definition> context, const QString &anchor, bool isSourceFile) = 0;

   virtual void writeCodeAnchor(const QString &anchor) = 0;

   virtual void writeCodeLink(const QString &ref, const QString &file, const QString &anchor,
                  const QString &name, const QString &tooltip) = 0;

   virtual void writeLineNumber(const QString &ref, const QString &file, const QString &anchor, int lineNumber) = 0;

   virtual void writeTooltip(const QString &id, const DocLinkInfo &docInfo, const QString &decl, const QString &desc,
                  const SourceLinkInfo &defInfo, const SourceLinkInfo &declInfo ) = 0;

   virtual void startCodeLine(bool hasLineNumbers) = 0;
   virtual void endCodeLine() = 0;

   virtual void startFontClass(const QString &fontName) = 0;
   virtual void endFontClass() = 0;

   virtual void startCodeFragment(const QString &style) = 0;
   virtual void endCodeFragment(const QString &style) = 0;
};

// Base Interface used for generating output outside of the comment blocks
class DocGenerator : public CodeGenerator
{
 public:
   virtual ~DocGenerator() {}

   enum ParamListTypes { Param, RetVal, Exception };
   enum SectionTypes { EnumValues,  Examples};

   virtual bool parseText(const QString &text)  {
      return text.isEmpty();
   }

   virtual void addIndexItemName(const QString &s1, const QString &s2) = 0;
   virtual void docify(const QString &text) = 0;
   virtual void lineBreak(const QString &style) = 0;

   virtual void writeAnchor(const QString &fileName, const QString &name) = 0;
   virtual void writeChar(char c) = 0;
   virtual void writeNonBreakableSpace(int) = 0;
   virtual void writeObjectLink(const QString &ref, const QString &file, const QString &anchor, const QString &name) = 0;
   virtual void writeRuler() = 0;
   virtual void writeString(const QString &text) = 0;

   virtual void startItemList()  = 0;
   virtual void endItemList()    = 0;

   virtual void startItemListItem() = 0;
   virtual void endItemListItem()   = 0;

   virtual void startParagraph(const QString &className) = 0;
   virtual void endParagraph() = 0;

   virtual void startHtmlLink(const QString &url) = 0;
   virtual void endHtmlLink() = 0;

   virtual void startBold() = 0;
   virtual void endBold()   = 0;

   virtual void startTypewriter() = 0;
   virtual void endTypewriter() = 0;

   virtual void startEmphasis() = 0;
   virtual void endEmphasis() = 0;

   virtual void startDescription() = 0;
   virtual void endDescription() = 0;

   virtual void startDescItem() = 0;
   virtual void endDescItem() = 0;

   virtual void startDescForItem() = 0;
   virtual void endDescForItem() = 0;

   virtual void startCenter() = 0;
   virtual void endCenter() = 0;

   virtual void startSmall() = 0;
   virtual void endSmall() = 0;

   virtual void startSimpleSect(SectionTypes t, const QString &file, const QString &anchor, const QString &title) = 0;
   virtual void endSimpleSect() = 0;

   virtual void startParamList(ParamListTypes t, const QString &title) = 0;
   virtual void endParamList() = 0;

   virtual void startTitle() = 0;
   virtual void endTitle()   = 0;

   virtual void startSection(const QString &, const QString &, SectionInfo::SectionType) = 0;
   virtual void endSection(const QString &, SectionInfo::SectionType) = 0;

   virtual void startEnumTable() = 0;
   virtual void endEnumTable() = 0;

   virtual void startDescTable(const QString &title) = 0;
   virtual void endDescTable() = 0;
   virtual void startDescTableRow() = 0;
   virtual void endDescTableRow() = 0;
   virtual void startDescTableTitle() = 0;
   virtual void endDescTableTitle() = 0;
   virtual void startDescTableData() = 0;
   virtual void endDescTableData() = 0;

   virtual void startTextLink(const QString &file, const QString &anchor) = 0;
   virtual void endTextLink() = 0;
   virtual void startPageRef() = 0;
   virtual void endPageRef(const QString &, const QString &) = 0;
   virtual void startSubsection() = 0;
   virtual void endSubsection() = 0;
   virtual void startSubsubsection() = 0;
   virtual void endSubsubsection() = 0;
};

// Abstract output generator
class OutputGenerator : public DocGenerator
{
 public:
   enum OutputType { Html, Latex, Man, RTF, XML, DEF, Perl, Docbook };

   OutputGenerator();
   virtual ~OutputGenerator();

   void startPlainFile(const QString &name);
   void endPlainFile();

   bool isEnabled() const {
      return active;
   }

   void pushGeneratorState();
   void popGeneratorState();

   virtual void enable() = 0;
   virtual void disable() = 0;
   virtual void enableIf(OutputType o) = 0;
   virtual void disableIf(OutputType o) = 0;
   virtual void disableIfNot(OutputType o) = 0;
   virtual bool isEnabled(OutputType o) = 0;

   virtual OutputGenerator *get(OutputType o) = 0;

   virtual void writeDoc(DocNode *, QSharedPointer<Definition> ctx, QSharedPointer<MemberDef> md) = 0;

   virtual void writeStartAnnoItem(const QString &type, const QString &file, const QString &path, const QString &name) = 0;
   virtual void writeEndAnnoItem(const QString &name) = 0;
   virtual void writeFooter(const QString &navPath) = 0;
   virtual void writeGraphicalHierarchy(const DotGfxHierarchyTable &g) = 0;
   virtual void writeLabel(const QString &labelText, bool isLast) = 0;
   virtual void writeLatexSpacing() = 0;
   virtual void writeLogo() = 0;

   virtual void writeInheritedSectionTitle(const QString &id, const QString &ref, const QString &file, const QString &anchor,
                  const QString &title, const QString &name) = 0;

   virtual void writeNavigationPath(const QString &) = 0;
   virtual void writePageLink(const QString &, bool) = 0;
   virtual void writeQuickLinks(bool compact, HighlightedItem hli, const QString &file) = 0;

   virtual void writeSearchInfo() = 0;
   virtual void writeSplitBar(const QString &name) = 0;
   virtual void writeStyleInfo(int part) = 0;
   virtual void writeSynopsis() = 0;
   virtual void writeSummaryLink(const QString &file, const QString &anchor, const QString &title, bool first) = 0;

   virtual void exceptionEntry(const QString &, bool) = 0;
   virtual void insertMemberAlign(bool) = 0;
   virtual void insertMemberAlignLeft(int, bool) = 0;
   virtual void lastIndexPage() = 0;

   virtual void startPageDoc(const QString &pageTitle) {
      (void) pageTitle;
   };

   virtual void endPageDoc() { }

   virtual void startFile(const QString &name, const QString &manName, const QString &title) = 0;
   virtual void endFile() = 0;

   virtual void startProjectNumber() = 0;
   virtual void endProjectNumber() = 0;
   virtual void startTitleHead(const QString &) = 0;
   virtual void endTitleHead(const QString &fileName, const QString &name) = 0;

   virtual void startIndexSection(IndexSections) = 0;
   virtual void endIndexSection(IndexSections) = 0;

   virtual void startIndexListItem() = 0;
   virtual void endIndexListItem()   = 0;
   virtual void startIndexList() = 0;
   virtual void endIndexList()   = 0;

   virtual void startIndexKey() = 0;
   virtual void endIndexKey()   = 0;

   virtual void startIndexValue(bool) = 0;
   virtual void endIndexValue(const QString &, bool) = 0;

   virtual void startIndexItem(const QString &ref, const QString &file) = 0;
   virtual void endIndexItem(const QString &ref, const QString &file) = 0;

   virtual void startMemberSections() = 0;
   virtual void endMemberSections() = 0;
   virtual void startHeaderSection() = 0;
   virtual void endHeaderSection() = 0;
   virtual void startMemberHeader(const QString &anchor, int type) = 0;
   virtual void endMemberHeader() = 0;
   virtual void startMemberSubtitle() = 0;
   virtual void endMemberSubtitle() = 0;
   virtual void startMemberDocList() = 0;
   virtual void endMemberDocList() = 0;
   virtual void startMemberList() = 0;
   virtual void endMemberList() = 0;
   virtual void startInlineHeader() = 0;
   virtual void endInlineHeader() = 0;
   virtual void startAnonTypeScope(int) = 0;
   virtual void endAnonTypeScope(int) = 0;

   virtual void startMemberItem(const QString &anchor, int, const QString &inheritId, bool deprecated = false) = 0;
   virtual void endMemberItem() = 0;
   virtual void startMemberTemplateParams() = 0;
   virtual void endMemberTemplateParams(const QString &, const QString &) = 0;
   virtual void startMemberGroupHeader(bool) = 0;
   virtual void endMemberGroupHeader() = 0;
   virtual void startMemberGroupDocs() = 0;
   virtual void endMemberGroupDocs() = 0;
   virtual void startMemberGroup() = 0;
   virtual void endMemberGroup(bool) = 0;

   virtual void startGroupHeader(int) = 0;
   virtual void endGroupHeader(int) = 0;

   virtual void startTextBlock(bool) = 0;
   virtual void endTextBlock(bool) = 0;

   virtual void startMemberDoc(const QString &clName, const QString &memName, const QString &anchor, const QString &title,
                  int memCount, int memTotal, bool showInline) = 0;
   virtual void endMemberDoc(bool) = 0;

   virtual void startDoxyAnchor(const QString &fName, const QString &manName, const QString &anchor,
                  const QString &name, const QString &args) = 0;
   virtual void endDoxyAnchor(const QString &fileName, const QString &anchor) = 0;

   virtual void startMemberDescription(const QString &anchor, const QString &inheritId, bool type) = 0;
   virtual void endMemberDescription() = 0;
   virtual void startMemberDeclaration() = 0;
   virtual void endMemberDeclaration(const QString &anchor, const QString &inheritId) = 0;

   virtual void startClassDiagram() = 0;
   virtual void endClassDiagram(const ClassDiagram &, const QString &, const QString &) = 0;
   virtual void startDotGraph() = 0;
   virtual void endDotGraph(const DotClassGraph &g) = 0;
   virtual void startInclDepGraph() = 0;
   virtual void endInclDepGraph(const DotInclDepGraph &g) = 0;
   virtual void startGroupCollaboration() = 0;
   virtual void endGroupCollaboration(const DotGroupCollaboration &g) = 0;
   virtual void startCallGraph() = 0;
   virtual void endCallGraph(const DotCallGraph &g) = 0;
   virtual void startDirDepGraph() = 0;
   virtual void endDirDepGraph(const DotDirDeps &g) = 0;

   virtual void startQuickIndices() = 0;
   virtual void endQuickIndices() = 0;
   virtual void startContents() = 0;
   virtual void endContents() = 0;

   virtual void startIndent() = 0;
   virtual void endIndent() = 0;

   virtual void startMemberDocPrefixItem() = 0;
   virtual void endMemberDocPrefixItem() = 0;
   virtual void startMemberDocName(bool) = 0;
   virtual void endMemberDocName() = 0;
   virtual void startParameterType(bool, const QString &key) = 0;
   virtual void endParameterType() = 0;
   virtual void startParameterName(bool) = 0;
   virtual void endParameterName(bool, bool, bool) = 0;
   virtual void startParameterList(bool) = 0;
   virtual void endParameterList() = 0;

   virtual void startConstraintList(const QString &) = 0;
   virtual void startConstraintParam() = 0;
   virtual void endConstraintParam() = 0;
   virtual void startConstraintType() = 0;
   virtual void endConstraintType() = 0;
   virtual void startConstraintDocs() = 0;
   virtual void endConstraintDocs() = 0;
   virtual void endConstraintList() = 0;

   virtual void startMemberDocSimple(bool isEnum) = 0;
   virtual void endMemberDocSimple(bool isEnum) = 0;
   virtual void startInlineMemberType() = 0;
   virtual void endInlineMemberType() = 0;
   virtual void startInlineMemberName() = 0;
   virtual void endInlineMemberName() = 0;
   virtual void startInlineMemberDoc() = 0;
   virtual void endInlineMemberDoc() = 0;

   virtual void startLabels() = 0;
   virtual void endLabels() = 0;

 protected:
   QTextStream m_textStream;

   QFile    m_file;
   QString  m_fileName;
   QString  m_outputDir;
   bool     active;

   QStack<bool> genStack;

 private:
   OutputGenerator(const OutputGenerator &o);
   OutputGenerator &operator=(const OutputGenerator &o);
};

// Interface used for generating documentation
class TextGenerator : public DocGenerator
{
 public:
   virtual ~TextGenerator()
   { }

   virtual void disableAllBut(OutputGenerator::OutputType o) = 0;
   virtual void enableAll() = 0;
   virtual void disableAll() = 0;

   virtual void disable(OutputGenerator::OutputType o) = 0;
   virtual void enable(OutputGenerator::OutputType o) = 0;

   virtual bool isEnabled(OutputGenerator::OutputType o) = 0;

   virtual void pushGeneratorState() = 0;
   virtual void popGeneratorState() = 0;
};

#endif
