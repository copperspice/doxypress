/************************************************************************
*
* Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
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

#ifndef LATEXGEN_H
#define LATEXGEN_H

#include <language.h>
#include <outputgen.h>

class QFile;

#define LATEX_STYLE_EXTENSION ".sty"

class LatexCodeGenerator : public CodeGenerator
{
   public:
      LatexCodeGenerator(QTextStream &t, const QString &relPath, const QString &sourceFile);

      // overrides
      void codify(const QString &text) override;
      void writeCodeLink(const QString &ref, const QString &file, const QString &anchor,
                  const QString &name, const QString &tooltip) override;

      void writeTooltip(const QString &, const DocLinkInfo &, const QString &, const QString &,
                  const SourceLinkInfo &, const SourceLinkInfo &) override {}

      void writeLineNumber(const QString &, const QString &, const QString &, int) override;;
      void startCodeLine(bool) override;
      void endCodeLine() override;
      void startFontClass(const QString &) override;
      void endFontClass() override;
      void writeCodeAnchor(const QString &) override {}
      void setCurrentDoc(QSharedPointer<Definition>, const QString &, bool) override {}
      void addWord(const QString &, bool) override {}

      void startCodeFragment(const QString &style) override;
      void endCodeFragment(const QString &style) override;

      // not part of CodeGenerator
      void incUsedTableLevel() {
         ++m_usedTableLevel;
      }

      void decUsedTableLevel() {
         --m_usedTableLevel;
      }

      int usedTableLevel() const {
         return m_usedTableLevel;
      }

   private:
      void _writeCodeLink(const QString &className, const QString &ref, const QString &file,
                  const QString &anchor, const QString &name, const QString &tooltip);

      void docify(const QString &str);

      QString m_relPath;
      QString m_sourceFileName;

      bool m_streamSet  = false;
      bool m_prettyCode       = false;
      bool m_doxyCodeLineOpen = false;

      int m_col;
      int m_usedTableLevel;

      QTextStream  &m_t;
};

class LatexGenerator : public OutputGenerator
{
 public:
   LatexGenerator();

   void init();

   static void writeStyleSheetFile(QFile &f);
   static void writeHeaderFile(QFile &f);
   static void writeFooterFile(QFile &f);

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
      if (o == Latex) {
         enable();
      }
   }

   void disableIf(OutputType o)  override{
      if (o == Latex) {
         disable();
      }
   }

   void disableIfNot(OutputType o) override {
      if (o != Latex) {
         disable();
      }
   }

   bool isEnabled(OutputType o) override {
      return (o == Latex && active);
   }

   OutputGenerator *get(OutputType o)  override {
      return (o == Latex) ? this : 0;
   }

   void codify(const QString &text) override {
      m_codeGen->codify(text);
   }

   void writeCodeLink(const QString &ref, const QString &file, const QString &anchor,
                  const QString &name, const QString &tooltip) override {
      m_codeGen->writeCodeLink(ref, file, anchor, name, tooltip);
   }

   void writeLineNumber(const QString &ref,const QString &file,const QString &anchor, int lineNumber) override {
      m_codeGen->writeLineNumber(ref, file, anchor, lineNumber);
   }

   void writeTooltip(const QString &id, const DocLinkInfo &docInfo, const QString &decl,
                  const QString &desc, const SourceLinkInfo &defInfo, const SourceLinkInfo &declInfo) override  {
      m_codeGen->writeTooltip(id, docInfo, decl, desc, defInfo, declInfo);
   }

   void startCodeLine(bool hasLineNumbers) override  {
      m_codeGen->startCodeLine(hasLineNumbers);
   }

   void endCodeLine() override {
      m_codeGen->endCodeLine();
   }

   void startFontClass(const QString &s) override {
      m_codeGen->startFontClass(s);
   }

   void endFontClass()  override {
      m_codeGen->endFontClass();
   }

   void writeCodeAnchor(const QString &anchor) override {
      m_codeGen->writeCodeAnchor(anchor);
   }

   void startCodeFragment(const QString &style) override {
      m_codeGen->startCodeFragment(style);
   }

   void endCodeFragment(const QString &style) override {
      m_codeGen->endCodeFragment(style);
   }

   //
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
   void endProjectNumber() override {}
   void writeStyleInfo(int part) override;

   void startTitleHead(const QString &) override;
   void startTitle() override;
   void endTitleHead(const QString &, const QString &name) override;

   void endTitle() override {
      m_textStream << "}";
   }

   void newParagraph();

   void startParagraph(const QString &className) override;
   void endParagraph() override;
   void writeString(const QString &text) override;
   void startIndexListItem() override {}
   void endIndexListItem() override {}

   void startIndexList() override {
      m_textStream << "\\begin{DoxyCompactList}"    << endl;
   }

   void endIndexList()  override  {
      m_textStream << "\\end{DoxyCompactList}"      << endl;
   }

   void startIndexKey() override;
   void endIndexKey() override;
   void startIndexValue(bool) override;
   void endIndexValue(const QString &, bool) override;

   void startItemList() override {
      m_textStream << "\\begin{DoxyCompactItemize}" << endl;
   }

   void endItemList() override {
      m_textStream << "\\end{DoxyCompactItemize}"   << endl;
   }

   void startIndexItem(const QString &ref, const QString &file) override;
   void endIndexItem(const QString &ref, const QString &file) override;

   void docify(const QString &text) override;

   void writeObjectLink(const QString &ref, const QString &file, const QString &anchor, const QString &name) override;

   void startTextLink(const QString &, const QString &) override;
   void endTextLink() override;
   void startHtmlLink(const QString &url) override;
   void endHtmlLink() override;

   void startTypewriter()  override {
      m_textStream << "{\\ttfamily ";
   }

   void endTypewriter()    override {
      m_textStream << "}";
   }

   void startGroupHeader(int) override;
   void endGroupHeader(int) override;

   void startItemListItem() override {
      m_textStream << "\\item " << endl;
   }

   void endItemListItem() override {}

   void startMemberSections() override {}
   void endMemberSections() override {}
   void startHeaderSection() override {}
   void endHeaderSection() override {}
   void startMemberHeader(const QString &, int) override;
   void endMemberHeader() override;
   void startMemberSubtitle() override {}
   void endMemberSubtitle() override {}
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
   void startMemberTemplateParams() override;
   void endMemberTemplateParams(const QString &, const QString &) override;

   void startMemberGroupHeader(bool) override;
   void endMemberGroupHeader() override;
   void startMemberGroupDocs() override;
   void endMemberGroupDocs() override;
   void startMemberGroup() override;
   void endMemberGroup(bool) override;

   void insertMemberAlign(bool) override {}
   void insertMemberAlignLeft(int, bool) override {}

   void writeRuler() override {
      m_textStream << endl << endl;
   }

   void writeAnchor(const QString &fileName, const QString &name) override;

   void startEmphasis() override {
      m_textStream << "{\\em ";
   }

   void endEmphasis() override  {
      m_textStream << "}";
   }

   void startBold() override {
      m_textStream << "{\\bfseries ";
   }

   void endBold() override {
      m_textStream << "}";
   }

   void startDescription() override;
   void endDescription() override;
   void startDescItem() override;
   void endDescItem() override;

   void lineBreak(const QString &style = QString()) override;

   void startMemberDoc(const QString &clName, const QString &memName, const QString &anchor, const QString &title,
                  int memCount, int memTotal, bool showInline) override;

   void endMemberDoc(bool) override;

   void startDoxyAnchor(const QString &, const QString &, const QString &, const QString &, const QString &) override;
   void endDoxyAnchor(const QString &, const QString &) override;

   void writeChar(char c) override;

   void writeLatexSpacing() override {
      m_textStream << "\\hspace{0.3cm}";
   }

   void writeStartAnnoItem(const QString &type, const QString &file, const QString &path, const QString &name) override;
   void writeEndAnnoItem(const QString &name) override;

   void startSubsection() override {
      m_textStream << "\\subsection*{";
   }

   void endSubsection() override {
      m_textStream << "}" << endl;
   }

   void startSubsubsection() override {
      m_textStream << "\\subsubsection*{";
   }

   void endSubsubsection() override {
      m_textStream << "}" << endl;
   }

   void startCenter() override {
      m_textStream << "\\begin{center}" << endl;
   }

   void endCenter() override {
      m_textStream << "\\end{center}" << endl;
   }

   void startSmall() override {
      m_textStream << "\\footnotesize ";
   }

   void endSmall() override {
      m_textStream << "\\normalsize ";
   }

   void startMemberDescription(const QString &, const QString &, bool) override;
   void endMemberDescription() override;
   void startMemberDeclaration() override{}

   void endMemberDeclaration(const QString &, const QString &) override {}

   void writeInheritedSectionTitle(const QString &, const QString &, const QString &,
                                   const QString &, const QString &, const QString &) override {}

   void startDescList(SectionTypes)  {
      m_textStream << "\\begin{Desc}\n\\item[";
   }

   void endDescList() {
      m_textStream << "\\end{Desc}" << endl;
   }

   void startSimpleSect(SectionTypes, const QString &, const QString &, const QString &) override;
   void endSimpleSect() override;
   void startParamList(ParamListTypes, const QString &title) override;
   void endParamList() override;

   void startDescForItem() override {
      m_textStream << "\\par" << endl;
   }

   void endDescForItem()  override { }
   void startSection(const QString &, const QString &, SectionInfo::SectionType) override;
   void endSection(const QString &, SectionInfo::SectionType) override;
   void addIndexItemName(const QString &, const QString &) override;
   void startIndent() override { }
   void endIndent() override { }
   void writeSynopsis() override { }
   void startClassDiagram() override;
   void endClassDiagram(const ClassDiagram &, const QString &, const QString &) override;
   void startPageRef() override;
   void endPageRef(const QString &, const QString &) override ;
   void startQuickIndices() override { }
   void endQuickIndices() override { }
   void writeSplitBar(const QString &) override { }
   void writeNavigationPath(const QString &) override { }
   void writeLogo() override { }
   void writeQuickLinks(bool, HighlightedItem, const QString &) override { }
   void writeSummaryLink(const QString &, const QString &, const QString &, bool) override { }
   void startContents() override { }
   void endContents() override { }
   void writeNonBreakableSpace(int) override;

   void startEnumTable() override {
      QString title1 = theTranslator->trEnumerationValues();
      QString title2 = theTranslator->trDocumentation();

      m_textStream << "\\tabulinesep=1mm\n\\begin{longtabu}spread 0pt [c]{*{" << 2 << "}{|X[-1]}|}\n";

      m_textStream << "\\rowcolor{\\tableheadbgcolor}"
                   << "\\cellcolor{\\tableheadbgcolor}\\textbf{ " << title1 << "}&"
                   << "\\cellcolor{\\tableheadbgcolor}\\textbf{ " << title2 << "}"
                   << "\\\\\\cline{1-2}\n\\endfirsthead\n\\hline\n\\endfoot\n\\hline";
   }

   void endEnumTable() override {
      m_textStream << "\\end{longtabu}\n";
   }

   void startDescTable(const QString &title) override;
   void endDescTable() override;
   void startDescTableRow() override;
   void endDescTableRow() override;
   void startDescTableTitle() override;
   void endDescTableTitle() override;
   void startDescTableData() override;
   void endDescTableData() override;
   void lastIndexPage() override;

   void startDotGraph() override;
   void endDotGraph(const DotClassGraph &) override;
   void startInclDepGraph() override;
   void endInclDepGraph(const DotInclDepGraph &) override;
   void startCallGraph() override;
   void startGroupCollaboration() override;
   void endGroupCollaboration(const DotGroupCollaboration &g) override;
   void endCallGraph(const DotCallGraph &) override;
   void startDirDepGraph() override;
   void endDirDepGraph(const DotDirDeps &g) override;
   void writeGraphicalHierarchy(const DotGfxHierarchyTable &) override {}

   void startTextBlock(bool) override {}
   void endTextBlock(bool) override {}

   void startMemberDocPrefixItem() override {}

   void endMemberDocPrefixItem() override {
      m_textStream << "\\\\" << endl;
   }

   void startMemberDocName(bool) override {}
   void endMemberDocName() override {}
   void startParameterType(bool, const QString &) override;
   void endParameterType() override;
   void startParameterName(bool) override;
   void endParameterName(bool, bool, bool) override;
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

   void setCurrentDoc(QSharedPointer<Definition>, const QString &, bool) override {
   }

   void addWord(const QString &word, bool hiPriority) override {
      (void) word;
      (void) hiPriority;
   }

 private:
   LatexGenerator(const LatexGenerator &);
   LatexGenerator &operator=(const LatexGenerator &);

   QString modifyKeywords(const QString &s);
   QString m_relPath;

   bool m_insideTabbing;
   bool m_firstDescItem;
   bool m_disableLinks;
   bool m_templateMemberItem;
   bool m_prettyCode;

   int m_indent;

   QSharedPointer<LatexCodeGenerator> m_codeGen;
};

#endif
