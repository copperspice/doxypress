/************************************************************************
*
* Copyright (C) 2014-2019 Barbara Geller & Ansel Sermersheim
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

#ifndef HTMLGEN_H
#define HTMLGEN_H

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QTextStream>

#include <language.h>
#include <outputgen.h>

#define PREFRAG_START   "<div class=\"fragment\">"
#define PREFRAG_END     "</div><!-- fragment -->"

class HtmlCodeGenerator : public CodeOutputInterface
{
 public:
   HtmlCodeGenerator(QTextStream &t, const QString &relPath);

   void setRelativePath(const QString &path);
   void codify(const QString &text) override;

   void writeCodeLink(const QString &ref, const QString &file, const QString &anchor,
                      const QString &name, const QString &tooltip) override;

   void writeTooltip(const QString &id, const DocLinkInfo &docInfo, const QString &decl, const QString &desc,
                     const SourceLinkInfo &defInfo, const SourceLinkInfo &declInfo) override;

   void writeLineNumber(const QString &, const QString &, const QString &, int) override;
   void startCodeLine(bool) override;
   void endCodeLine() override;
   void startFontClass(const QString &) override;
   void endFontClass() override;
   void writeCodeAnchor(const QString &anchor) override;

   void setCurrentDoc(QSharedPointer<Definition> d, const QString &, bool) override {}
   void addWord(const QString &name, bool) override {}

 private:
   void _writeCodeLink(const QString &className, const QString &ref, const QString &file,
                  const QString &anchor, const QString &name, const QString &tooltip);

   void docify(const QString &text);

   int m_col;
   QString m_relPath;
   QTextStream &m_streamX;
};

/** Generator for HTML output */
class HtmlGenerator : public OutputGenerator
{
 public:
   HtmlGenerator();
   virtual ~HtmlGenerator();

   static void init();
   static void writeStyleSheetFile(QFile &f);
   static void writeHeaderFile(QFile &f);
   static void writeFooterFile(QFile &f);
   static void writeTabData();
   static void writeSearchInfo(QTextStream &t, const QString &relPath);
   static void writeSearchData(const QString &dir);
   static void writeSearchPage();
   static void writeExternalSearchPage();

   static QString writeLogoAsString(const QString &path);
   static QString writeSplitBarAsString(const QString &name, const QString &relpath);

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

   void enableIf(OutputType o) override {
      if (o == Html) {
         enable();
      }
   }

   void disableIf(OutputType o) override {
      if (o == Html) {
         disable();
      }
   }

   void disableIfNot(OutputType o) override {
      if (o != Html) {
         disable();
      }
   }

   bool isEnabled(OutputType o) override {
      return (o == Html && active);
   }

   OutputGenerator *get(OutputType o) override {
      return (o == Html) ? this : 0;
   }

   void codify(const QString &text) override {
      m_codeGen->codify(text);
   }

   void writeCodeLink(const QString &ref, const QString &file, const QString &anchor,
                      const QString &name, const QString &tooltip) override {
      m_codeGen->writeCodeLink(ref, file, anchor, name, tooltip);
   }

   void writeLineNumber(const QString &ref, const QString &file, const QString &anchor, int lineNumber) override {
      m_codeGen->writeLineNumber(ref, file, anchor, lineNumber);
   }

   void writeTooltip(const QString &id, const DocLinkInfo &docInfo, const QString &decl, const QString &desc,
                     const SourceLinkInfo &defInfo, const SourceLinkInfo &declInfo ) override {
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

   void writeCodeAnchor(const QString &anchor) override {
      m_codeGen->writeCodeAnchor(anchor);
   }

   // **
   void setCurrentDoc(QSharedPointer<Definition> context, const QString &anchor, bool isSourceFile) override;
   void addWord(const QString &word, bool hiPriority) override;

   void writeDoc(DocNode *, QSharedPointer<Definition> ctx, QSharedPointer<MemberDef> md) override;

   void startFile(const QString &name, const QString &manName, const QString &title) override;
   void writeFooter(const QString &navPath) override;
   void endFile() override;
   void clearBuffer();
   void writeSearchInfo() override;

   void startIndexSection(IndexSections) override {}
   void endIndexSection(IndexSections)override {}
   void writePageLink(const QString &, bool) override {}
   void startProjectNumber()override ;
   void endProjectNumber() override;
   void writeStyleInfo(int part) override;

   void startTitleHead(const QString &) override;
   void endTitleHead(const QString &, const QString &) override;

   void startParagraph(const QString &className) override;
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

   void startIndexItem(const QString &ref, const QString &file) override;
   void endIndexItem(const QString &ref, const QString &file) override;
   void docify(const QString &text) override;

   void writeObjectLink(const QString &ref, const QString &file, const QString &anchor, const QString &name) override;
   void startTextLink(const QString &file, const QString &anchor) override;

   void endTextLink() override;
   void startHtmlLink(const QString &url) override;
   void endHtmlLink() override;

   void startMemberSections() override;
   void endMemberSections() override;
   void startHeaderSection() override;
   void endHeaderSection() override;
   void startMemberHeader(const QString &) override;
   void endMemberHeader() override;
   void startMemberSubtitle() override;
   void endMemberSubtitle() override;
   void startMemberDocList() override;
   void endMemberDocList() override;
   void startMemberList() override;
   void endMemberList() override;
   void startInlineHeader() override;
   void endInlineHeader() override;

   void startAnonTypeScope(int) override {}
   void endAnonTypeScope(int) override {}

   void startMemberItem(const QString &anchor, int, const QString &inheritId, bool deprecated = false) override;
   void endMemberItem() override;
   void startMemberTemplateParams() override;
   void endMemberTemplateParams(const QString &anchor, const QString &inheritId) override;

   void startMemberGroupHeader(bool) override;
   void endMemberGroupHeader() override;
   void startMemberGroupDocs() override;
   void endMemberGroupDocs() override;
   void startMemberGroup() override;
   void endMemberGroup(bool) override;

   void insertMemberAlign(bool) override;
   void startMemberDescription(const QString &anchor, const QString &inheritId) override;
   void endMemberDescription() override;

   void startMemberDeclaration()override { }
   void endMemberDeclaration(const QString &anchor, const QString &inheritId) override;

   void writeInheritedSectionTitle(const QString &id, const QString &ref, const QString &file,
                                   const QString &anchor, const QString &title, const QString &name) override;

   void startGroupHeader(int) override;
   void endGroupHeader(int) override;

   // **
   void startTitle() override {
      m_textStream << "<div class=\"title\">";
   }

   void endTitle() override {
      m_textStream << "</div>";
   }

   void startItemList()  override {
      m_textStream << "<ul>"  << endl;
   }

   void endItemList() override  {
      m_textStream << "</ul>" << endl;
   }

   void startTypewriter() override {
      m_textStream << "<code>";
   }

   void endTypewriter()   override {
      m_textStream << "</code>";
   }

   void startItemListItem() override {
      m_textStream << "<li>";
   }

   void endItemListItem() override {
      m_textStream << "</li>\n";
   }

   void writeRuler() override  {
      m_textStream << "<hr/>";
   }
   void writeAnchor(const QString &, const QString &name) override {
      m_textStream << "<a name=\"" << name << "\" id=\"" << name << "\"></a>";
   }

   void startCodeFragment() override {
      m_textStream << PREFRAG_START;
   }

   void endCodeFragment()  override {
      m_textStream << PREFRAG_END;
   }

   void startEmphasis() override {
      m_textStream << "<em>";
   }

   void endEmphasis() override {
      m_textStream << "</em>";
   }

   void startBold() override {
      m_textStream << "<b>";
   }

   void endBold()  override {
      m_textStream << "</b>";
   }

   void startDescription() override {
      m_textStream << endl << "<dl>" << endl;
   }

   void endDescription() override  {
      m_textStream << endl << "</dl>\n" << endl;
   }

   void startDescItem() override {
      m_textStream << "<dt>";
   }

   void endDescItem() override  {
      m_textStream << "</dt>";
   }

   void startDescForItem() override {
      m_textStream << "<dd>";
   }

   void endDescForItem() override  {
      m_textStream << "</dd>\n";
   }

   void writeEndAnnoItem(const QString &) override {
      m_textStream << endl;
   }

   void startSubsection() override {
      m_textStream << "<h2>";
   }
   void endSubsection() override {
      m_textStream << "</h2>" << endl;
   }

   void startSubsubsection() override {
      m_textStream << "<h3>";
   }

   void endSubsubsection() override  {
      m_textStream << "</h3>" << endl;
   }

   void startCenter()  override {
      m_textStream << "<center>" << endl;
   }

   void endCenter() override {
      m_textStream << "</center>" << endl;
   }

   void startSmall() override {
      m_textStream << "<small>" << endl;
   }

   void endSmall() override {
      m_textStream << "</small>" << endl;
   }

   void startEnumTable() override {
      QString title1 = theTranslator->trEnumerationValues();
      QString title2 = theTranslator->trDocumentation();

      m_textStream << "<table class=\"fieldtable\">" << endl
           << "<tr><th>" << title1 << "</th><th>" << title2 << "</th></tr>";

   }

   void endEnumTable() override {
      m_textStream << "</table>" << endl;
   }

   void startDescTable(const QString &title) override;
   void endDescTable() override;
   void startDescTableRow() override;
   void endDescTableRow() override;
   void startDescTableTitle();
   void endDescTableTitle();
   void startDescTableData();
   void endDescTableData();

   void startTextBlock(bool) override {
      m_textStream << "<div class=\"textblock\">";
   }

   void endTextBlock(bool) override {
      m_textStream << "</div>";
   }

   void lineBreak(const QString &style) override;

   void writeChar(char c) override;
   void startMemberDoc(const QString &clName, const QString &memName, const QString &anchor,
                  const QString &title, bool showInline) override;

   void endMemberDoc(bool) override;

   void startDoxyAnchor(const QString &fName, const QString &manName, const QString &anchor,
                  const QString &name, const QString &args) override;

   void endDoxyAnchor(const QString &fName, const QString &anchor) override;
   void writeLatexSpacing() override { }

   void writeStartAnnoItem(const QString &type, const QString &file, const QString &path, const QString &name) override;
   void startSimpleSect(SectionTypes, const QString &, const QString &, const QString &) override;

   void endSimpleSect() override;
   void startParamList(ParamListTypes, const QString &) override;
   void endParamList() override;

   void startSection(const QString &, const QString &, SectionInfo::SectionType) override;
   void endSection(const QString &, SectionInfo::SectionType) override;
   void addIndexItem(const QString &, const QString &) override;
   void startIndent() override;
   void endIndent() override;
   void writeSynopsis() override { }
   void startClassDiagram() override;
   void endClassDiagram(const ClassDiagram &, const QString &, const QString &) override;
   void startPageRef() override {}
   void endPageRef(const QString &, const QString &) override {}
   void startQuickIndices() override {}
   void endQuickIndices() override;
   void writeSplitBar(const QString &name) override;
   void writeNavigationPath(const QString &s) override;
   void writeLogo() override;
   void writeQuickLinks(bool compact, HighlightedItem hli, const QString &file) override;
   void writeSummaryLink(const QString &file, const QString &anchor, const QString &title, bool first) override;
   void startContents() override;
   void endContents() override;
   void startPageDoc(const QString &pageTitle) override;
   void endPageDoc() override;
   void writeNonBreakableSpace(int) override;

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
   void writeGraphicalHierarchy(const DotGfxHierarchyTable &g) override;

   void lastIndexPage() override {}

   void startMemberDocPrefixItem() override;
   void endMemberDocPrefixItem() override;
   void startMemberDocName(bool) override;
   void endMemberDocName() override;
   void startParameterType(bool first, const QString &key) override;
   void endParameterType() override;
   void startParameterName(bool) override;
   void endParameterName(bool last, bool emptyList, bool closeBracket) override;
   void startParameterList(bool) override;
   void endParameterList() override;

   void exceptionEntry(const QString &, bool) override;

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

 private:
   static void writePageFooter(QTextStream &t, const QString &, const QString &, const QString &);

   QString m_lastTitle;
   QString m_relativePath;
   QString m_lastFile;

   void docify(const QString &text, bool inHtmlComment);

   HtmlGenerator &operator=(const HtmlGenerator &g);
   HtmlGenerator(const HtmlGenerator &g);

   int m_sectionCount;
   bool m_emptySection;

   QSharedPointer<HtmlCodeGenerator> m_codeGen;
};

#endif
