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

#ifndef LATEXGEN_H
#define LATEXGEN_H

#include <outputgen.h>

class QFile;

/** Generator for LaTeX output. */
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

   void writeDoc(DocNode *, QSharedPointer<Definition> ctx, QSharedPointer<MemberDef> md); 

   void startFile(const QString &name, const QString &manName, const QString &title) override;
   void writeSearchInfo() {}
   void writeFooter(const QString &) override {}
   void endFile();
   void clearBuffer();

   void startIndexSection(IndexSections);
   void endIndexSection(IndexSections);
   void writePageLink(const char *, bool);
   void startProjectNumber();
   void endProjectNumber() {}
   void writeStyleInfo(int part);
   void startTitleHead(const char *);
   void startTitle();
   void endTitleHead(const char *, const char *name);
   void endTitle()   {
      m_textStream << "}";
   }

   void newParagraph();
   void startParagraph();
   void endParagraph();
   void writeString(const char *text);
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
   void endIndexValue(const char *, bool);

   void startItemList()  {
      m_textStream << "\\begin{DoxyCompactItemize}" << endl;
   }

   void endItemList()    {
      m_textStream << "\\end{DoxyCompactItemize}"   << endl;
   }

   void startIndexItem(const QByteArray &ref, const QByteArray &file) override;
   void endIndexItem(const QByteArray &ref, const QByteArray &file) override;

   void docify(const QByteArray &text) override;
   void codify(const QByteArray &text) override;

   void writeObjectLink(const QByteArray &ref, const QByteArray &file, const QByteArray &anchor, const QByteArray &name) override;

   void writeCodeLink(const QByteArray &ref, const QByteArray &file, const QByteArray &anchor, 
                      const QByteArray &name, const QByteArray &tooltip) override;

   void writeTooltip(const char *, const DocLinkInfo &, const QByteArray &, const QByteArray &, const SourceLinkInfo &, 
                     const SourceLinkInfo & ) override {}

   void startTextLink(const QByteArray &, const QByteArray &) override;
   void endTextLink();
   void startHtmlLink(const QString &url) override;
   void endHtmlLink();

   void startTypewriter() {
      m_textStream << "{\\ttfamily ";
   }
   void endTypewriter()   {
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
   void startMemberHeader(const char *);
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
   void startMemberItem(const char *, int, const QByteArray &) override;
   void endMemberItem();
   void startMemberTemplateParams();
   void endMemberTemplateParams(const char *, const QByteArray &) override;

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

   void writeAnchor(const char *fileName, const char *name);
   void startCodeFragment();
   void endCodeFragment();
   void writeLineNumber(const char *, const QByteArray &, const char *, int l) override;
   void startCodeLine(bool hasLineNumbers);
   void endCodeLine();

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

   void lineBreak(const QByteArray &style = 0) override ;
   void startMemberDoc(const char *, const char *, const char *, const char *, bool);
   void endMemberDoc(bool);
   void startDoxyAnchor(const char *, const char *, const char *, const char *, const char *);
   void endDoxyAnchor(const char *, const char *);
   void writeChar(char c);

   void writeLatexSpacing() {
      m_textStream << "\\hspace{0.3cm}";
   }

   void writeStartAnnoItem(const char *type, const QByteArray &file, const QByteArray &path, const char *name) override;
   void writeEndAnnoItem(const char *name) override;

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
   void startMemberDescription(const char *, const QByteArray &) override;
   void endMemberDescription();
   void startMemberDeclaration() {}

   void endMemberDeclaration(const char *, const QByteArray &) override {}

   void writeInheritedSectionTitle(const char *, const QByteArray &, const char *,
                                   const char *, const char *, const char *) override {}

   void startDescList(SectionTypes)     {
      m_textStream << "\\begin{Desc}\n\\item[";
   }

   void endDescList()       {
      m_textStream << "\\end{Desc}" << endl;
   }

   void startSimpleSect(SectionTypes, const QByteArray &, const char *, const char *) override;
   void endSimpleSect();
   void startParamList(ParamListTypes, const char *title);
   void endParamList();

   void startDescForItem()     {
      m_textStream << "\\par" << endl;
   }

   void endDescForItem()       {}
   void startSection(const char *, const char *, SectionInfo::SectionType);
   void endSection(const char *, SectionInfo::SectionType);
   void addIndexItem(const char *, const char *);
   void startIndent()       {}
   void endIndent()         {}
   void writeSynopsis()     {}
   void startClassDiagram();
   void endClassDiagram(const ClassDiagram &, const char *, const char *);
   void startPageRef();
   void endPageRef(const QByteArray &, const QByteArray &) override ;
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

   void startDescTable(const char *title) {
      startSimpleSect(EnumValues, 0, 0, title);
      startDescForItem();
      m_textStream << "\\begin{description}" << endl;
   }
   void endDescTable() {
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
   void endMemberDocPrefixItem() {}
   void startMemberDocName(bool) {}
   void endMemberDocName() {}
   void startParameterType(bool, const QByteArray &) override ; 
   void endParameterType();
   void startParameterName(bool);
   void endParameterName(bool, bool, bool);
   void startParameterList(bool) override;
   void endParameterList();
   void exceptionEntry(const QByteArray &, bool) override;

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

   void startFontClass(const char *); // {}
   void endFontClass(); // {}

   void writeCodeAnchor(const char *) {}
   void setCurrentDoc(QSharedPointer<Definition> d, const char *, bool) override {}
   void addWord(const QString &word, bool hiPriority) override {}

 private:
   LatexGenerator(const LatexGenerator &);
   LatexGenerator &operator=(const LatexGenerator &);
   void escapeLabelName(const char *s);
   void escapeMakeIndexChars(const char *s);
   int col;
   bool insideTabbing;
   bool firstDescItem;
   bool disableLinks;
   QByteArray relPath;
   QByteArray sourceFileName;
   int m_indent;
   bool templateMemberItem;
   bool m_prettyCode;
};

#endif
