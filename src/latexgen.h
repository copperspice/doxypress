/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim
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

#ifndef LATEXGEN_H
#define LATEXGEN_H

#include <language.h>
#include <outputgen.h>

class QFile;

class LatexCodeGenerator : public CodeOutputInterface
{
   public:
      LatexCodeGenerator(QTextStream &t, const QString &relPath, const QString &sourceFile);
      LatexCodeGenerator();

      void setTextStream(QTextStream &t);
      void setRelativePath(const QString &path);
      void setSourceFileName(const QString &sourceFileName);

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

   private:
      void _writeCodeLink(const QString &className, const QString &ref, const QString &file,
                  const QString &anchor, const QString &name, const QString &tooltip);

      void docify(const QString &str);
      bool m_streamSet;

      QTextStream  m_t;
      QString      m_relPath;
      QString      m_sourceFileName;
      int          m_col;
      bool         m_prettyCode;
};

class LatexGenerator : public OutputGenerator
{
 public:
   LatexGenerator();
   ~LatexGenerator();

   static void init();
   static void writeStyleSheetFile(QFile &f);
   static void writeHeaderFile(QFile &f);
   static void writeFooterFile(QFile &f);

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
      if (o == Latex) {
         enable();
      }
   }

   void disableIf(OutputType o) {
      if (o == Latex) {
         disable();
      }
   }

   void disableIfNot(OutputType o) {
      if (o != Latex) {
         disable();
      }
   }

   bool isEnabled(OutputType o) {
      return (o == Latex && active);
   }

   OutputGenerator *get(OutputType o) {
      return (o == Latex) ? this : 0;
   }

   // CodeOutputInterface
   void codify(const QString &text) override {
      m_codeGen.codify(text);
   }

   void writeCodeLink(const QString &ref, const QString &file, const QString &anchor,
                  const QString &name, const QString &tooltip) override {
      m_codeGen.writeCodeLink(ref, file, anchor, name, tooltip);
   }

   void writeLineNumber(const QString &ref,const QString &file,const QString &anchor, int lineNumber) override {
      m_codeGen.writeLineNumber(ref, file, anchor, lineNumber);
   }

   void writeTooltip(const QString &id, const DocLinkInfo &docInfo, const QString &decl,
                  const QString &desc, const SourceLinkInfo &defInfo, const SourceLinkInfo &declInfo) override  {
      m_codeGen.writeTooltip(id, docInfo, decl, desc, defInfo, declInfo);
   }

   void startCodeLine(bool hasLineNumbers) override  {
      m_codeGen.startCodeLine(hasLineNumbers);
   }

   void endCodeLine() override {
      m_codeGen.endCodeLine();
   }

   void startFontClass(const QString &s) override {
      m_codeGen.startFontClass(s);
   }

   void endFontClass()  override {
      m_codeGen.endFontClass();
   }

   void writeCodeAnchor(const QString &anchor) override {
      m_codeGen.writeCodeAnchor(anchor);
   }


   //
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
   void endProjectNumber() {}
   void writeStyleInfo(int part);
   void startTitleHead(const QString &) override;
   void startTitle();
   void endTitleHead(const QString &, const QString &name) override;
   void endTitle()   {
      m_textStream << "}";
   }

   void newParagraph();
   void startParagraph();
   void endParagraph();
   void writeString(const QString &text);
   void startIndexListItem() {}
   void endIndexListItem() {}
   void startIndexList() {
      m_textStream << "\\begin{DoxyCompactList}"    << endl;
   }
   void endIndexList()   {
      m_textStream << "\\end{DoxyCompactList}"      << endl;
   }
   void startIndexKey();
   void endIndexKey();
   void startIndexValue(bool);
   void endIndexValue(const QString &, bool) override;

   void startItemList()  {
      m_textStream << "\\begin{DoxyCompactItemize}" << endl;
   }

   void endItemList()    {
      m_textStream << "\\end{DoxyCompactItemize}"   << endl;
   }

   void startIndexItem(const QString &ref, const QString &file) override;
   void endIndexItem(const QString &ref, const QString &file) override;

   void docify(const QString &text) override;

   void writeObjectLink(const QString &ref, const QString &file, const QString &anchor, const QString &name) override;

   void startTextLink(const QString &, const QString &) override;
   void endTextLink();
   void startHtmlLink(const QString &url) override;
   void endHtmlLink();

   void startTypewriter()  override {
      m_textStream << "{\\ttfamily ";
   }
   void endTypewriter()    override {
      m_textStream << "}";
   }

   void startGroupHeader(int);
   void endGroupHeader(int);
   void startItemListItem() {
      m_textStream << "\\item " << endl;
   }
   void endItemListItem()   {}

   void startMemberSections() {}
   void endMemberSections() {}
   void startHeaderSection() {}
   void endHeaderSection() {}
   void startMemberHeader(const QString &) override;
   void endMemberHeader();
   void startMemberSubtitle() {}
   void endMemberSubtitle() {}
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
   void startMemberTemplateParams();
   void endMemberTemplateParams(const QString &, const QString &) override;

   void startMemberGroupHeader(bool);
   void endMemberGroupHeader();
   void startMemberGroupDocs();
   void endMemberGroupDocs();
   void startMemberGroup();
   void endMemberGroup(bool);

   void insertMemberAlign(bool) {}

   void writeRuler() {
      m_textStream << endl << endl;
   }

   void writeAnchor(const QString &fileName, const QString &name) override;
   void startCodeFragment();
   void endCodeFragment();

   void startEmphasis() {
      m_textStream << "{\\em ";
   }

   void endEmphasis()   {
      m_textStream << "}";
   }

   void startBold()     {
      m_textStream << "{\\bfseries ";
   }

   void endBold()       {
      m_textStream << "}";
   }

   void startDescription();
   void endDescription();
   void startDescItem();
   void endDescItem();

   void lineBreak(const QString &style = 0) override ;
   void startMemberDoc(const QString &, const QString &, const QString &, const QString &, bool) override;
   void endMemberDoc(bool);
   void startDoxyAnchor(const QString &, const QString &, const QString &, const QString &, const QString &) override;
   void endDoxyAnchor(const QString &, const QString &) override;
   void writeChar(char c);

   void writeLatexSpacing() {
      m_textStream << "\\hspace{0.3cm}";
   }

   void writeStartAnnoItem(const QString &type, const QString &file, const QString &path, const QString &name) override;
   void writeEndAnnoItem(const QString &name) override;

   void startSubsection() {
      m_textStream << "\\subsection*{";
   }
   void endSubsection() {
      m_textStream << "}" << endl;
   }
   void startSubsubsection() {
      m_textStream << "\\subsubsection*{";
   }
   void endSubsubsection() {
      m_textStream << "}" << endl;
   }
   void startCenter()      {
      m_textStream << "\\begin{center}" << endl;
   }
   void endCenter()        {
      m_textStream << "\\end{center}" << endl;
   }
   void startSmall()       {
      m_textStream << "\\footnotesize ";
   }
   void endSmall()         {
      m_textStream << "\\normalsize ";
   }
   void startMemberDescription(const QString &, const QString &) override;
   void endMemberDescription();
   void startMemberDeclaration() {}

   void endMemberDeclaration(const QString &, const QString &) override {}

   void writeInheritedSectionTitle(const QString &, const QString &, const QString &,
                                   const QString &, const QString &, const QString &) override {}

   void startDescList(SectionTypes)     {
      m_textStream << "\\begin{Desc}\n\\item[";
   }

   void endDescList()       {
      m_textStream << "\\end{Desc}" << endl;
   }

   void startSimpleSect(SectionTypes, const QString &, const QString &, const QString &) override;
   void endSimpleSect();
   void startParamList(ParamListTypes, const QString &title) override;
   void endParamList();

   void startDescForItem()     {
      m_textStream << "\\par" << endl;
   }

   void endDescForItem()       {}
   void startSection(const QString &, const QString &, SectionInfo::SectionType) override;
   void endSection(const QString &, SectionInfo::SectionType) override;
   void addIndexItem(const QString &, const QString &) override;
   void startIndent() {}
   void endIndent()   {}
   void writeSynopsis() {}
   void startClassDiagram();
   void endClassDiagram(const ClassDiagram &, const QString &, const QString &) override;
   void startPageRef();
   void endPageRef(const QString &, const QString &) override ;
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

   void startEnumTable() {
      startSimpleSect(EnumValues, 0, 0, theTranslator->trEnumerationValues());
      startDescForItem();
      m_textStream << "\\begin{description}" << endl;
   }

   void endEnumTable() {
      m_textStream << "\\end{description}" << endl;
      endDescForItem();
      endSimpleSect();
   }

   void startDescTableTitle() {
      m_textStream << "\\item[{\\em " << endl;
   }

   void endDescTableTitle() {
      m_textStream << "}]";
   }

   void startDescTableData() {}
   void endDescTableData() {}
   void lastIndexPage() {}

   void startDotGraph();
   void endDotGraph(const DotClassGraph &);
   void startInclDepGraph();
   void endInclDepGraph(const DotInclDepGraph &);
   void startCallGraph();
   void startGroupCollaboration();
   void endGroupCollaboration(const DotGroupCollaboration &g);
   void endCallGraph(const DotCallGraph &);
   void startDirDepGraph();
   void endDirDepGraph(const DotDirDeps &g);
   void writeGraphicalHierarchy(const DotGfxHierarchyTable &) {}

   void startTextBlock(bool) {}
   void endTextBlock(bool) {}

   void startMemberDocPrefixItem() {}

   void endMemberDocPrefixItem() { 
      m_textStream << "\\\\" << endl; 
   }

   void startMemberDocName(bool) {}
   void endMemberDocName() {}
   void startParameterType(bool, const QString &) override;
   void endParameterType();
   void startParameterName(bool);
   void endParameterName(bool, bool, bool);
   void startParameterList(bool) override;
   void endParameterList();
   void exceptionEntry(const QString &, bool) override;

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

   void setCurrentDoc(QSharedPointer<Definition> d, const QString &, bool) override {}
   void addWord(const QString &word, bool hiPriority) override {}

 private:
   LatexGenerator(const LatexGenerator &);
   LatexGenerator &operator=(const LatexGenerator &);

   QString modifyKeywords(const QString &s);
   QString m_relPath;

   bool insideTabbing;
   bool firstDescItem;
   bool disableLinks;

   int m_indent;
   bool templateMemberItem;
   bool m_prettyCode;

   LatexCodeGenerator m_codeGen;
};

#endif
