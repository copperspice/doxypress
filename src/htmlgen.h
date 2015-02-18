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

#ifndef HTMLGEN_H
#define HTMLGEN_H

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QTextStream>

#include <outputgen.h>

#define PREFRAG_START   "<div class=\"fragment\">"
#define PREFRAG_END     "</div><!-- fragment -->"

class HtmlCodeGenerator : public CodeOutputInterface
{
 public:
   HtmlCodeGenerator(QTextStream &t, const QByteArray &relPath);

   void setRelativePath(const QByteArray &path);
   void codify(const QByteArray &text) override;

   void writeCodeLink(const QByteArray &ref, const QByteArray &file, const QByteArray &anchor, 
                      const QByteArray &name, const QByteArray &tooltip) override;

   void writeTooltip(const char *id, const DocLinkInfo &docInfo, const QByteArray &decl, const QByteArray &desc, 
                     const SourceLinkInfo &defInfo, const SourceLinkInfo &declInfo) override;

   void writeLineNumber(const char *, const QByteArray &, const char *, int) override;
   void startCodeLine(bool);
   void endCodeLine();
   void startFontClass(const char *s);
   void endFontClass();
   void writeCodeAnchor(const char *anchor);

   void setCurrentDoc(QSharedPointer<Definition> d, const char *, bool) override {}
   void addWord(const QString &name, bool) override {}

 private:
   void _writeCodeLink(const QByteArray &className, const QByteArray &ref, const QByteArray &file, const QByteArray &anchor, 
                       const QByteArray &name, const QByteArray &tooltip);

   void docify(const QByteArray &str);

   QTextStream &m_streamX;  

   int m_col;  
   QByteArray m_relPath;
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
   static void writeSearchInfo(QTextStream &t, const QByteArray &relPath);
   static void writeSearchData(const char *dir);
   static void writeSearchPage();
   static void writeExternalSearchPage();
   static QByteArray writeLogoAsString(const char *path);
   static QByteArray writeSplitBarAsString(const char *name, const char *relpath);

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
 
   void codify(const QByteArray &text) override {
      m_codeGen->codify(text);
   }

   void writeCodeLink(const QByteArray &ref, const QByteArray &file, const QByteArray &anchor, 
                      const QByteArray &name, const QByteArray &tooltip) override {
      m_codeGen->writeCodeLink(ref, file, anchor, name, tooltip);
   }

   void writeLineNumber(const char *ref, const QByteArray &file, const char *anchor, int lineNumber) {
      m_codeGen->writeLineNumber(ref, file, anchor, lineNumber);
   }

   void writeTooltip(const char *id, const DocLinkInfo &docInfo, const QByteArray &decl, const QByteArray &desc, 
                     const SourceLinkInfo &defInfo, const SourceLinkInfo &declInfo ) override {
      m_codeGen->writeTooltip(id, docInfo, decl, desc, defInfo, declInfo);
   }

   void startCodeLine(bool hasLineNumbers) {
      m_codeGen->startCodeLine(hasLineNumbers);
   }

   void endCodeLine() {
      m_codeGen->endCodeLine();
   }

   void startFontClass(const char *s) {
      m_codeGen->startFontClass(s);
   }

   void endFontClass() {
      m_codeGen->endFontClass();
   }

   void writeCodeAnchor(const char *anchor) {
      m_codeGen->writeCodeAnchor(anchor);
   }

   // **
   void setCurrentDoc(QSharedPointer<Definition> context, const char *anchor, bool isSourceFile) override;
   void addWord(const QString &word, bool hiPriority) override;

   void writeDoc(DocNode *, QSharedPointer<Definition> ctx, QSharedPointer<MemberDef> md) override;

   void startFile(const char *name, const char *manName, const char *title);
   void writeFooter(const char *navPath);
   void endFile();
   void clearBuffer();
   void writeSearchInfo();

   void startIndexSection(IndexSections) {}
   void endIndexSection(IndexSections) {}
   void writePageLink(const char *, bool) {}
   void startProjectNumber();
   void endProjectNumber();
   void writeStyleInfo(int part);
   void startTitleHead(const char *);
   void endTitleHead(const char *, const char *);
  
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
   
   void startIndexItem(const QByteArray &ref, const QByteArray &file) override;
   void endIndexItem(const QByteArray &ref, const QByteArray &file) override;
   void docify(const QByteArray &text) override;

   void writeObjectLink(const QByteArray &ref, const QByteArray &file, const QByteArray &anchor, const QByteArray &name) override;

   void startTextLink(const QByteArray &file, const QByteArray &anchor) override;

   void endTextLink();
   void startHtmlLink(const QByteArray &url) override;
   void endHtmlLink();
 
   void startMemberSections();
   void endMemberSections();
   void startHeaderSection();
   void endHeaderSection();
   void startMemberHeader(const char *);
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

   void startMemberItem(const char *anchor, int, const QByteArray &inheritId) override;
   void endMemberItem();
   void startMemberTemplateParams();
   void endMemberTemplateParams(const char *anchor, const QByteArray &inheritId) override;

   void startMemberGroupHeader(bool);
   void endMemberGroupHeader();
   void startMemberGroupDocs();
   void endMemberGroupDocs();
   void startMemberGroup();
   void endMemberGroup(bool);

   void insertMemberAlign(bool);
   void startMemberDescription(const char *anchor, const QByteArray &inheritId) override;
   void endMemberDescription();

   void startMemberDeclaration() 
   {
   }

   void endMemberDeclaration(const char *anchor, const QByteArray &inheritId) override;

   void writeInheritedSectionTitle(const char *id, const QByteArray &ref, const char *file, 
                                   const char *anchor, const char *title, const char *name) override;

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

   void startTypewriter() {
      m_textStream << "<code>";
   }

   void endTypewriter()   {
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
   void writeAnchor(const char *, const char *name) {
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

   void writeEndAnnoItem(const char *) {
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

   void startDescTable(const char *title)   
   {
      m_textStream << "<table class=\"fieldtable\">" << endl
        << "<tr><th colspan=\"2\">" << title << "</th></tr>";
   }

   void endDescTable() {
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
  
   void lineBreak(const QByteArray &style) override ;
   void writeChar(char c);
   void startMemberDoc(const char *, const char *, const char *, const char *, bool);
   void endMemberDoc(bool);

   void startDoxyAnchor(const char *fName, const char *manName, const char *anchor, const char *name, const char *args); 
   void endDoxyAnchor(const char *fName, const char *anchor);

   void writeLatexSpacing()
   {
   }
  
   void writeStartAnnoItem(const char *type, const QByteArray &file, const QByteArray &path, const char *name) override;   
   void startSimpleSect(SectionTypes, const QByteArray &, const char *, const char *) override ;

   void endSimpleSect();
   void startParamList(ParamListTypes, const char *);
   void endParamList();
   
   void startSection(const char *, const char *, SectionInfo::SectionType);
   void endSection(const char *, SectionInfo::SectionType);
   void addIndexItem(const char *, const char *);
   void startIndent();
   void endIndent();
   void writeSynopsis() {}
   void startClassDiagram();
   void endClassDiagram(const ClassDiagram &, const char *, const char *);
   void startPageRef() {}
   void endPageRef(const QByteArray &, const QByteArray &) override {}
   void startQuickIndices() {}
   void endQuickIndices();
   void writeSplitBar(const char *name);
   void writeNavigationPath(const char *s);
   void writeLogo();
   void writeQuickLinks(bool compact, HighlightedItem hli, const char *file);
   void writeSummaryLink(const QByteArray &file, const char *anchor, const char *title, bool first) override;
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
   void startParameterType(bool first, const QByteArray &key) override;
   void endParameterType();
   void startParameterName(bool);
   void endParameterName(bool last, bool emptyList, bool closeBracket);
   void startParameterList(bool);
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

 private:
   static void writePageFooter(QTextStream &t, const QByteArray &, const QByteArray &, const QByteArray &);
   QByteArray lastTitle;
   QByteArray lastFile;
   QByteArray relPath;

   void docify(const QByteArray &text, bool inHtmlComment);

   HtmlGenerator &operator=(const HtmlGenerator &g);
   HtmlGenerator(const HtmlGenerator &g);

   int m_sectionCount;
   bool m_emptySection;

   QSharedPointer<HtmlCodeGenerator> m_codeGen;
};

#endif
