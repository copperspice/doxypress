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
   void startCodeLine(bool);
   void endCodeLine();
   void startFontClass(const QString &) override;
   void endFontClass();
   void writeCodeAnchor(const QString &anchor) override;

   void setCurrentDoc(QSharedPointer<Definition> d, const QString &, bool) override {}
   void addWord(const QString &name, bool) override {}

 private:
   void _writeCodeLink(const QString &className, const QString &ref, const QString &file, 
                  const QString &anchor, const QString &name, const QString &tooltip);

   void docify(const QString &text);

   QTextStream &m_streamX;  

   int m_col;  
   QString m_relPath;
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
      if (o == Html) {
         enable();
      }
   }

   void disableIf(OutputType o) {
      if (o == Html) {
         disable();
      }
   }

   void disableIfNot(OutputType o) {
      if (o != Html) {
         disable();
      }
   }

   bool isEnabled(OutputType o) {
      return (o == Html && active);
   }

   OutputGenerator *get(OutputType o) {
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

   void startCodeLine(bool hasLineNumbers) {
      m_codeGen->startCodeLine(hasLineNumbers);
   }

   void endCodeLine() {
      m_codeGen->endCodeLine();
   }

   void startFontClass(const QString &s) override {
      m_codeGen->startFontClass(s);
   }

   void endFontClass() {
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
   void writeFooter(const QString &navPath);
   void endFile();
   void clearBuffer();
   void writeSearchInfo();

   void startIndexSection(IndexSections) {}
   void endIndexSection(IndexSections) {}
   void writePageLink(const QString &, bool) override {}
   void startProjectNumber();
   void endProjectNumber();
   void writeStyleInfo(int part);
   void startTitleHead(const QString &) override;
   void endTitleHead(const QString &, const QString &) override;
  
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
   
   void startIndexItem(const QString &ref, const QString &file) override;
   void endIndexItem(const QString &ref, const QString &file) override;
   void docify(const QString &text) override;

   void writeObjectLink(const QString &ref, const QString &file, const QString &anchor, const QString &name) override;
   void startTextLink(const QString &file, const QString &anchor) override;

   void endTextLink();
   void startHtmlLink(const QString &url) override;
   void endHtmlLink();
 
   void startMemberSections();
   void endMemberSections();
   void startHeaderSection();
   void endHeaderSection();
   void startMemberHeader(const QString &) override;
   void endMemberHeader();
   void startMemberSubtitle();
   void endMemberSubtitle();
   void startMemberDocList();
   void endMemberDocList();
   void startMemberList();
   void endMemberList();
   void startInlineHeader();
   void endInlineHeader();

   void startAnonTypeScope(int)
   {}

   void endAnonTypeScope(int)
   {}

   void startMemberItem(const QString &anchor, int, const QString &inheritId) override;
   void endMemberItem();
   void startMemberTemplateParams();
   void endMemberTemplateParams(const QString &anchor, const QString &inheritId) override;

   void startMemberGroupHeader(bool);
   void endMemberGroupHeader();
   void startMemberGroupDocs();
   void endMemberGroupDocs();
   void startMemberGroup();
   void endMemberGroup(bool);

   void insertMemberAlign(bool);
   void startMemberDescription(const QString &anchor, const QString &inheritId) override;
   void endMemberDescription();

   void startMemberDeclaration() 
   {
   }

   void endMemberDeclaration(const QString &anchor, const QString &inheritId) override;

   void writeInheritedSectionTitle(const QString &id, const QString &ref, const QString &file, 
                                   const QString &anchor, const QString &title, const QString &name) override;

   void startGroupHeader(int);
   void endGroupHeader(int);


   // **
   void startTitle() {
      m_textStream << "<div class=\"title\">";
   }

   void endTitle() {
      m_textStream << "</div>";
   }

   void startItemList()  {
      m_textStream << "<ul>"  << endl;
   }

   void endItemList()    {
      m_textStream << "</ul>" << endl;
   }

   void startTypewriter() override {
      m_textStream << "<code>";
   }

   void endTypewriter()   override {
      m_textStream << "</code>";
   }
   
   void startItemListItem() {
      m_textStream << "<li>";
   }

   void endItemListItem() {
      m_textStream << "</li>\n";
   }

   void writeRuler()    {
      m_textStream << "<hr/>";
   }
   void writeAnchor(const QString &, const QString &name) override {
      m_textStream << "<a name=\"" << name << "\" id=\"" << name << "\"></a>";
   }

   void startCodeFragment() {
      m_textStream << PREFRAG_START;
   }

   void endCodeFragment()   {
      m_textStream << PREFRAG_END;
   }

   void startEmphasis() {
      m_textStream << "<em>";
   }

   void endEmphasis()   {
      m_textStream << "</em>";
   }
 
   void startBold()     {
      m_textStream << "<b>";
   }
 
   void endBold()       {
      m_textStream << "</b>";
   }

   void startDescription() {
      m_textStream << endl << "<dl>" << endl;
   }

   void endDescription()   {
      m_textStream << endl << "</dl>\n" << endl;
   }

   void startDescItem()    {
      m_textStream << "<dt>";
   }

   void endDescItem()      {
      m_textStream << "</dt>";
   }

   void startDescForItem() {
      m_textStream << "<dd>";
   }

   void endDescForItem()   {
      m_textStream << "</dd>\n";
   }

   void writeEndAnnoItem(const QString &) override {
      m_textStream << endl;
   }

   void startSubsection()    {
      m_textStream << "<h2>";
   }
   void endSubsection()      {
      m_textStream << "</h2>" << endl;
   }

   void startSubsubsection() {
      m_textStream << "<h3>";
   }

   void endSubsubsection()   {
      m_textStream << "</h3>" << endl;
   }

   void startCenter()        {
      m_textStream << "<center>" << endl;
   }

   void endCenter()          {
      m_textStream << "</center>" << endl;
   }

   void startSmall()         {
      m_textStream << "<small>" << endl;
   }

   void endSmall()           {
      m_textStream << "</small>" << endl;
   }

   void startEnumTable()   
   {
      QString title1 = theTranslator->trEnumerationValues();
      QString title2 = theTranslator->trDocumentation();
        
      m_textStream << "<table class=\"fieldtable\">" << endl
           << "<tr><th>" << title1 << "</th><th>" << title2 << "</th></tr>";

   }

   void endEnumTable() {
      m_textStream << "</table>" << endl;
   }

   void startDescTableTitle()  
   {
      m_textStream << "<tr><td class=\"fieldname\">";
   }

   void endDescTableTitle() {
      m_textStream << "&#160;</td>";
   }
    
   void startDescTableData()  
   {
      m_textStream << "<td class=\"fielddoc\">" << endl;
   }

   void endDescTableData() {
      m_textStream << "</td></tr>" << endl;
   }

   void startTextBlock(bool) {
      m_textStream << "<div class=\"textblock\">";
   }

   void endTextBlock(bool) {
      m_textStream << "</div>";
   }
  
   void lineBreak(const QString &style) override ;
   void writeChar(char c);
   void startMemberDoc(const QString &clName, const QString &memName, const QString &anchor, 
                  const QString &title, bool showInline) override;
   void endMemberDoc(bool);

   void startDoxyAnchor(const QString &fName, const QString &manName, const QString &anchor, 
                  const QString &name, const QString &args) override; 

   void endDoxyAnchor(const QString &fName, const QString &anchor) override;

   void writeLatexSpacing()
   {
   }
  
   void writeStartAnnoItem(const QString &type, const QString &file, const QString &path, const QString &name) override;   
   void startSimpleSect(SectionTypes, const QString &, const QString &, const QString &) override;

   void endSimpleSect();
   void startParamList(ParamListTypes, const QString &) override;
   void endParamList();
   
   void startSection(const QString &, const QString &, SectionInfo::SectionType) override;
   void endSection(const QString &, SectionInfo::SectionType) override;
   void addIndexItem(const QString &, const QString &) override;
   void startIndent();
   void endIndent();
   void writeSynopsis() {}
   void startClassDiagram();
   void endClassDiagram(const ClassDiagram &, const QString &, const QString &) override;
   void startPageRef() {}
   void endPageRef(const QString &, const QString &) override {}
   void startQuickIndices() {}
   void endQuickIndices();
   void writeSplitBar(const QString &name);
   void writeNavigationPath(const QString &s) override;
   void writeLogo();
   void writeQuickLinks(bool compact, HighlightedItem hli, const QString &file) override;
   void writeSummaryLink(const QString &file, const QString &anchor, const QString &title, bool first) override;
   void startContents();
   void endContents();
   void writeNonBreakableSpace(int);  

   void startDotGraph();
   void endDotGraph(const DotClassGraph &g);
   void startInclDepGraph();
   void endInclDepGraph(const DotInclDepGraph &g);
   void startGroupCollaboration();
   void endGroupCollaboration(const DotGroupCollaboration &g);
   void startCallGraph();
   void endCallGraph(const DotCallGraph &g);
   void startDirDepGraph();
   void endDirDepGraph(const DotDirDeps &g);
   void writeGraphicalHierarchy(const DotGfxHierarchyTable &g);
 
   void lastIndexPage() {}

   void startMemberDocPrefixItem();
   void endMemberDocPrefixItem();
   void startMemberDocName(bool);
   void endMemberDocName();
   void startParameterType(bool first, const QString &key) override;
   void endParameterType();
   void startParameterName(bool);
   void endParameterName(bool last, bool emptyList, bool closeBracket);
   void startParameterList(bool);
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
   void writeLabel(const QString &l, bool isLast);
   void endLabels();

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
