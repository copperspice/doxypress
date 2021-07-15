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

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QHash>

#include <docbookgen.h>

#include <arguments.h>
#include <config.h>
#include <default_args.h>
#include <diagram.h>
#include <docbookvisitor.h>
#include <docparser.h>
#include <dot.h>
#include <doxy_globals.h>
#include <doxy_build_info.h>
#include <groupdef.h>
#include <language.h>
#include <message.h>
#include <membergroup.h>
#include <parse_base.h>
#include <util.h>

#include <stdlib.h>

inline void writeDocbookString(QTextStream &t, const QString &text)
{
   t << convertToDocBook(text);
}

inline void writeDocbookCodeString(QTextStream &t, const QString &s, int &col)
{
   static const int tabSize = Config::getInt("tab-size");

   for (auto c : s) {

      switch (c.unicode()) {
         case '\t': {
            int spacesToNextTabStop = tabSize - (col % tabSize);

            col += spacesToNextTabStop;
            while (spacesToNextTabStop--) {
               t << "&#32;";
            }
            break;
         }

         case ' ':
            t << "&#32;";
            ++col;
            break;

         case '<':
            t << "&lt;";
            ++col;
            break;

         case '>':
            t << "&gt;";
            ++col;
            break;

         case '&':
            t << "&amp;";
            ++col;
            break;

         case '\'':
            t << "&apos;";
            col++;
            break;

         case '"':
            t << "&quot;";
            ++col;
            break;

         case '\007':
            // bell
            t << "^G";
            ++col;
            break;

         case '\014':
            // form feed
            t << "^L";
            ++col;
            break;

         default:
            t << c;
            ++col;
            break;
      }
   }
}

void writeDocbookLink(QTextStream &t, const QString &compoundId, const QString &anchorId, const QString &text )
{
   t << "<link linkend=\"" << stripPath(compoundId);

   if (! anchorId.isEmpty()) {
      t << "_1" << anchorId;
   }

   t << "\"";
   t << ">";
   writeDocbookString(t, text);
   t << "</link>";
}

static void addIndexTerm(QTextStream &t, QString primary, QString secondary)
{
   t << "<indexterm><primary>";
   t << convertToDocBook(primary);
   t << "</primary>";

   if (! secondary.isEmpty()) {
      t << "<secondary>";
      t << convertToDocBook(secondary);
      t << "</secondary>";
   }

   t << "</indexterm>" << endl;
}

DocbookCodeGenerator::DocbookCodeGenerator(QTextStream &t, const QString &relPath, const QString &sourceFile)
   : m_streamCode(t), m_insideCodeLine(false), m_insideSpecialHL(false), m_lineNumber(-1), m_col(0),
      m_relPath(relPath), m_sourceFileName(sourceFile)
{
   m_prettyCode = Config::getBool("docbook-program-listing");
}

void DocbookCodeGenerator::codify(const QString &text)
{
   writeDocbookCodeString(m_streamCode, text, m_col);
}

void DocbookCodeGenerator::writeCodeLink(const QString &ref, const QString &file, const QString &anchor,
   const QString &name, const QString &tooltip)
{

   (void) ref;
   (void) tooltip;

   writeDocbookLink(m_streamCode, file, anchor, name);
   m_col += name.length();
}

void DocbookCodeGenerator::writeCodeLinkLine(const QString &, const QString &file,
   const QString &, const QString &name, const QString &)
{
   m_streamCode << "<anchor xml:id=\"_" << stripExtensionGeneral(stripPath(file), ".xml");
   m_streamCode << "_1l";

   writeDocbookString(m_streamCode, name);

   m_streamCode << "\"/>";
   m_col += name.size();
}

void DocbookCodeGenerator::writeTooltip(const QString &, const DocLinkInfo &, const QString &,
   const QString &, const SourceLinkInfo &, const SourceLinkInfo &)
{
}

void DocbookCodeGenerator::startCodeLine(bool)
{
/*
   if (m_lineNumber != -1) {
      if (! m_refId.isEmpty()) {
         m_streamCode << "<link linkend=\"" << m_refId << "\">";
      }

      m_streamCode << m_lineNumber << " ";
      if (!m_refId.isEmpty()) {
         m_streamCode << "</link>";
      }
   }
*/

   m_insideCodeLine = true;
   m_col = 0;
}

void DocbookCodeGenerator::endCodeLine()
{
   if (m_insideCodeLine) {
      m_streamCode << endl;
   }

   m_refId.clear();
   m_external.clear();

   m_lineNumber     = -1;
   m_insideCodeLine = false;
}

void DocbookCodeGenerator::startFontClass(const QString &colorClass)
{
   m_streamCode << "<emphasis role=\"" << colorClass << "\">";
   m_insideSpecialHL = true;
}

void DocbookCodeGenerator::endFontClass()
{
   m_streamCode << "</emphasis>";         // non DocBook
   m_insideSpecialHL = false;
}

void DocbookCodeGenerator::writeCodeAnchor(const QString &)
{
}

void DocbookCodeGenerator::writeLineNumber(const QString &ref, const QString &fileName, const QString &anchorId, int lineNumber)
{
   m_insideCodeLine = true;

   if (m_prettyCode) {

      QString newLineNumber = QString("%1").formatArg(lineNumber, 5, 10, '0');

      if (! fileName.isEmpty() && ! m_sourceFileName.isEmpty()) {

         writeCodeLinkLine(ref, m_sourceFileName, anchorId, newLineNumber, QString());
         writeCodeLink(ref, fileName, anchorId, newLineNumber, QString());

      } else {
         codify(newLineNumber);
      }

   } else {
      m_streamCode << lineNumber << " ";
   }

   m_col = 0;
}

void DocbookCodeGenerator::setCurrentDoc(QSharedPointer<Definition>, const QString &, bool)
{
}

void DocbookCodeGenerator::addWord(const QString &, bool)
{
}

void DocbookCodeGenerator::finish()
{
   endCodeLine();
}

void DocbookCodeGenerator::startCodeFragment(const QString &)
{
   m_streamCode << "<programlisting>";
}

void DocbookCodeGenerator::endCodeFragment(const QString &)
{
   endCodeLine();
   m_streamCode << "</programlisting>";
}

DocbookGenerator::DocbookGenerator()
   : OutputGenerator(), m_inListItem{false}, m_inSimpleSect{false}
{
   static const QString docbookOutput = Config::getString("docbook-output");
   static const bool docbookProgram   = Config::getBool("docbook-program-listing");

   m_outputDir     = docbookOutput;      // parent class
   m_prettyCode    = docbookProgram;
   m_denseText     = false;
   m_inGroup       = false;
   m_inDetail      = false;
   m_descTable     = false;
   m_firstMember   = false;

   m_levelListItem = 0;
   m_inLevel = -1;
}

void DocbookGenerator::init()
{
  static const QString dirName = Config::getString("docbook-output");

  QDir d(dirName);

  if (! d.exists() && ! d.mkdir(dirName)) {
      err("Docbook Generator, unable to create output directory %s\n", csPrintable(dirName));
      Doxy_Work::stopDoxyPress();
  }

  createSubDirs(d);
}

void DocbookGenerator::addIndexItem(const QString &primary, const QString &secondary)
{
   addIndexTerm(m_textStream, primary, secondary);
}

void DocbookGenerator::startExamples()
{
   m_textStream << "<simplesect><title>";
   docify(theTranslator->trExamples());
   m_textStream << "</title>";
}

void DocbookGenerator::endExamples()
{
   m_textStream << "</simplesect>" << endl;
}

void DocbookGenerator::startFile(const QString &name, const QString &, const QString &)
{
   QString fileName = name;
   QString fileType = "section";

   if (fileName == "refman") {
      fileName = "index";
      fileType = "book";

   } else if (fileName == "index") {
      fileName = "mainpage";
      fileType = "chapter";
   }

   QString pageName = fileName;
   m_relPath = relativePathToRoot(fileName);

   if (! fileName.endsWith(".xml")) {
      fileName += ".xml";
   }

   startPlainFile(fileName);

   m_codeGen = QMakeShared<DocbookCodeGenerator>(m_textStream, m_relPath, stripPath(fileName));

   m_textStream << "<?xml version='1.0' encoding='UTF-8' standalone='no'?>" << endl;;
   m_textStream << "<" << fileType << " xmlns=\"http://docbook.org/ns/docbook\" version=\"5.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"";

   if (! pageName.isEmpty()) {
      m_textStream << " xml:id=\"_" <<  stripPath(pageName) << "\"";
   }

   m_textStream << " xml:lang=\"" << theTranslator->trLanguageId() << "\"";
   m_textStream << ">" << endl;
}

void DocbookGenerator::endFile()
{
   if (m_inDetail) {
      m_textStream << "</section>" << endl;
   }

   m_inDetail = false;

   while (m_inLevel != -1) {
      m_textStream << "</section>" << endl;
      --m_inLevel;
   }

   if (m_inGroup) {
      m_textStream << "</section>" << endl;
   }
   m_inGroup = false;

   QString fileType = "section";
   QString fileName = m_codeGen->sourceFileName();

   if (fileName == "index.xml") {
      fileType = "book";

   } else if (fileName == "mainpage.xml") {
      fileType = "chapter";
   }

   m_textStream << "</" << fileType << ">" << endl;

   endPlainFile();
}

void DocbookGenerator::startIndexSection(IndexSections is)
{
   static const QString projectName = Config::getString("project-name");

   switch (is) {
      case isTitlePageStart: {
         m_textStream << "    <info>" << endl;
         m_textStream << "    <title>" << convertToDocBook(projectName) << "</title>" << endl;
         m_textStream << "    </info>" << endl;
      }
      break;

      case isTitlePageAuthor:
         break;

      case isMainPage:
         m_textStream << "<chapter>" << endl;
         m_textStream << "    <title>";
         break;

      case isModuleIndex:
      case isDirIndex:
      case isNamespaceIndex:
      case isClassHierarchyIndex:
         break;

      case isCompoundIndex:
         // m_textStream << "{"; //Class Index}\n"
         break;

      case isFileIndex:
         // Annotated File Index}\n"
         break;

      case isPageIndex:
         // Annotated Page Index}\n"
         break;

      case isModuleDocumentation:
         m_textStream << "<chapter>\n";
         m_textStream << "    <title>";
         break;

      case isDirDocumentation:
         m_textStream << "<chapter>\n";
         m_textStream << "    <title>";
         break;

      case isNamespaceDocumentation:
         m_textStream << "<chapter>\n";
         m_textStream << "    <title>";
         break;

      case isClassDocumentation:
         m_textStream << "<chapter>\n";
         m_textStream << "    <title>";
         break;

      case isFileDocumentation:
         m_textStream << "<chapter>\n";
         m_textStream << "    <title>";
         break;

      case isExampleDocumentation:
         m_textStream << "<chapter>\n";
         m_textStream << "    <title>";
         break;

      case isPageDocumentation:
         break;

      case isPageDocumentation2:
         break;

      case isEndIndex:
         break;
   }
}

void DocbookGenerator::endIndexSection(IndexSections is)
{
   static const bool sourceCode = Config::getBool("source-code");

   switch (is) {
      case isTitlePageStart:
      case isTitlePageAuthor:
         break;

      case isMainPage:
         m_textStream << "</title>" << endl;
         m_textStream << "    <xi:include href=\"mainpage.xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;
         m_textStream << "</chapter>" << endl;
         break;

      case isModuleIndex:
         // m_textStream << "</chapter>" << endl;
         break;

      case isDirIndex:
         // m_textStream << "<xi:include href=\"dirs.xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>";
         // m_textStream << "</chapter>" << endl;
         break;

      case isNamespaceIndex:
         // m_textStream << "<xi:include href=\"namespaces.xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>";
         // m_textStream << "</chapter>" << endl;
         break;

      case isClassHierarchyIndex:
         // m_textStream << "<xi:include href=\"hierarchy.xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>";
         // m_textStream<< "</chapter>" << endl;
         break;

      case isCompoundIndex:
         // m_textStream << "</chapter>" << endl;
         break;

      case isFileIndex:
         // m_textStream << "<xi:include href=\"files.xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>";
         // m_textStream << "</chapter>" << endl;
         break;

      case isPageIndex:
         // m_textStream << "<xi:include href=\"pages.xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>";
         // m_textStream << "</chapter>" << endl;
         break;

      case isModuleDocumentation: {
         m_textStream << "</title>" << endl;

         for (auto &gd : Doxy_Globals::groupSDict) {
            if (! gd->isReference()) {
               m_textStream << "    <xi:include href=\"" << gd->getOutputFileBase() << ".xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;
            }
         }
      }

      m_textStream << "</chapter>\n";
      break;

      case isDirDocumentation: {
         m_textStream<< "</title>" << endl;

         for (auto &dd : Doxy_Globals::directories) {
            if (dd->isLinkableInProject()) {
               m_textStream << "    <xi:include href=\"" << dd->getOutputFileBase() << ".xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;
            }
         }
      }

      m_textStream<< "</chapter>\n";
      break;

      case isNamespaceDocumentation: {
         m_textStream << "</title>" << endl;

         for (auto &nd : Doxy_Globals::namespaceSDict) {
            if (nd->isLinkableInProject()) {
               m_textStream << "<xi:include href=\"" << nd->getOutputFileBase() << ".xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;
            }
         }
      }

      m_textStream << "</chapter>\n";
      break;

      case isClassDocumentation: {
         m_textStream << "</title>" << endl;

         for (auto &cd : Doxy_Globals::classSDict) {
            if (cd->isLinkableInProject() && cd->templateMaster() == 0 && !cd->isEmbeddedInOuterScope()) {
               m_textStream << "    <xi:include href=\"" << cd->getOutputFileBase() << ".xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;
            }
         }
      }

      m_textStream << "</chapter>\n";
      break;

      case isFileDocumentation: {
         m_textStream << "</title>" << endl;

         bool isFirst = true;

         for (const auto &fn : Doxy_Globals::inputNameList) {
            for (const auto &fd : *fn) {
               if (fd->isLinkableInProject()) {
                  if (isFirst) {
                     m_textStream << "    <xi:include href=\"" << fd->getOutputFileBase() << ".xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;

                     if (sourceCode && m_prettyCode && fd->generateSourceFile()) {
                        m_textStream << "    <xi:include href=\"" << fd->getSourceFileBase() << ".xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;
                     }
                     isFirst = false;

                  } else {
                     m_textStream << "    <xi:include href=\"" << fd->getOutputFileBase() << ".xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;

                     if (sourceCode && m_prettyCode && fd->generateSourceFile()) {
                        m_textStream << "    <xi:include href=\"" << fd->getSourceFileBase() << ".xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;
                     }
                  }
               }
            }
         }
      }

      m_textStream << "</chapter>\n";
      break;

      case isExampleDocumentation: {
         m_textStream << "</title>" << endl;

         for (auto &pd : Doxy_Globals::exampleSDict) {
            m_textStream << "    <xi:include href=\"" << pd->getOutputFileBase() << ".xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;
         }
      }

      m_textStream << "</chapter>\n";
      break;

      case isPageDocumentation:
      case isPageDocumentation2:
         break;

      case isEndIndex:
         m_textStream << "<index/>" << endl;
         break;
   }
}

void DocbookGenerator::writePageLink(const QString &name, bool)
{
   for (auto &pd : Doxy_Globals::pageSDict) {

      if (! pd->getGroupDef() && ! pd->isReference() && pd->name() == stripPath(name)) {
         m_textStream << "<chapter>\n";

         if (pd->hasTitle()) {
            m_textStream << "    <title>" << convertToDocBook(pd->title()) << "</title>" << endl;
         } else {
            m_textStream << "    <title>" << convertToDocBook(pd->name()) << "</title>" << endl;
         }

         m_textStream << "    <xi:include href=\"" << pd->getOutputFileBase() << ".xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;
         m_textStream << "</chapter>\n";
      }
   }
}

void DocbookGenerator::writeDoc(DocNode *node, QSharedPointer<Definition>, QSharedPointer<MemberDef>)
{
   DocbookDocVisitor *visitor = new DocbookDocVisitor(m_textStream, *this);
   node->accept(visitor);

   delete visitor;
}

void DocbookGenerator::startParagraph(const QString &)
{
   m_textStream << "<para>" << endl;
}

void DocbookGenerator::endParagraph()
{
   m_textStream << "</para>" << endl;
}

void DocbookGenerator::writeString(const QString &text)
{
   m_textStream << text;
}

void DocbookGenerator::startMemberHeader(const QString &, int)
{
   m_textStream << "<simplesect>" << endl;
   m_inSimpleSect[m_levelListItem] = true;
   m_textStream << "    <title>";
}

void DocbookGenerator::endMemberHeader()
{
   m_textStream << "    </title>" << endl;
}

void DocbookGenerator::docify(const QString &text)
{
   m_textStream << convertToDocBook(text);
}

void DocbookGenerator::writeObjectLink(const QString &ref, const QString &file, const QString &anchor, const QString &name)
{
   (void) ref;

   if (! anchor.isEmpty()) {

      if (! file.isEmpty()) {
         m_textStream << "<link linkend=\"_" << stripPath(file) << "_1" << anchor << "\">";

      } else {
         m_textStream << "<link linkend=\"_" << anchor << "\">";

      }

   } else {
         m_textStream << "<link linkend=\"_" << stripPath(file) << "\">";
   }

   docify(name);
   m_textStream << "</link>";
}

void DocbookGenerator::startMemberList()
{
   m_textStream << "        <itemizedlist>" << endl;
   ++m_levelListItem;
}

void DocbookGenerator::endMemberList()
{
   if (m_inListItem[m_levelListItem]) {
      m_textStream << "</listitem>" << endl;
   }

   m_inListItem[m_levelListItem] = false;
   m_textStream << "        </itemizedlist>" << endl;
   m_levelListItem = (m_levelListItem > 0 ?  m_levelListItem - 1 : 0);

   if (m_inSimpleSect[m_levelListItem]) {
      m_textStream << "</simplesect>" << endl;
   }

   m_inSimpleSect[m_levelListItem] = false;
}

void DocbookGenerator::startMemberItem(const QString &, int, const QString &, bool)
{
   if (m_inListItem[m_levelListItem]) {
      m_textStream << "</listitem>" << endl;
   }

   m_textStream << "            <listitem><para>";
   m_inListItem[m_levelListItem] = true;
}

void DocbookGenerator::endMemberItem()
{
   m_textStream << "</para>" << endl;
}

void DocbookGenerator::startBold()
{
   m_textStream << "<emphasis role=\"strong\">";
}

void DocbookGenerator::endBold()
{
   m_textStream << "</emphasis>";
}

void DocbookGenerator::startGroupHeader(int extraIndentLevel)
{
   m_firstMember = true;

   if (m_inSimpleSect[m_levelListItem]) {
      m_textStream << "</simplesect>" << endl;
   }

   m_inSimpleSect[m_levelListItem] = false;
   if (m_inLevel != -1) {
      m_inGroup = true;
   }

   if (m_inLevel == extraIndentLevel) {
      m_textStream << "</section>" << endl;
   }

   m_inLevel = extraIndentLevel;
   m_textStream << "<section>" << endl;
   m_textStream << "<title>";
}

void DocbookGenerator::writeRuler()
{
   if (m_inGroup) {
      m_textStream << "</section>" << endl;
   }

   m_inGroup = false;
}

void DocbookGenerator::endGroupHeader(int)
{
   m_textStream << "</title>" << endl;
}

void DocbookGenerator::startParameterList(bool openBracket)
{
   if (openBracket) {
      m_textStream << "(";
   }
}

void DocbookGenerator::endParameterList()
{
}

void DocbookGenerator::writeNonBreakableSpace(int n)
{
   for (int i = 0; i < n; ++i) {
      m_textStream << " ";
   }
}

void DocbookGenerator::lineBreak(const QString &)
{
   m_textStream << endl;
}

void DocbookGenerator::startTypewriter()
{
   if (!m_denseText) {
      m_textStream << "<computeroutput>";
   }
}

void DocbookGenerator::endTypewriter()
{
   if (! m_denseText) {
      m_textStream << "</computeroutput>" << endl;
   }
}

void DocbookGenerator::startTextBlock(bool dense)
{
   if (dense) {
      m_denseText = true;
      m_textStream << "<programlisting>";
   }
}
void DocbookGenerator::endTextBlock(bool)
{
   if (m_denseText) {
      m_denseText = false;
      m_textStream << "</programlisting>";
   }
}

void DocbookGenerator::startMemberDoc(const QString &clname, const QString &memname, const QString &,
            const QString &title, int memCount, int memTotal, bool)
{
   m_textStream << "    <section>" << endl;
   m_textStream << "    <title>" << convertToDocBook(title);

   if (memTotal > 1) {
      m_textStream << "<computeroutput>[" << memCount << "/" << memTotal << "]</computeroutput>";
   }

   m_textStream << "</title>" << endl;

   if (! memname.isEmpty() && ! memname.startsWith('@')) {
      addIndexTerm(m_textStream, memname, clname);
      addIndexTerm(m_textStream, clname, memname);
   }
}
void DocbookGenerator::endMemberDoc(bool)
{
   m_textStream << "</computeroutput></para>";
}

void DocbookGenerator::startTitleHead(const QString &)
{
   m_textStream << "<title>";
}

void DocbookGenerator::endTitleHead(const QString &, const QString &name)
{
   m_textStream << "</title>" << endl;

   if (! name.isEmpty()) {
      addIndexTerm(m_textStream, name, QString());
   }
}

void DocbookGenerator::startDoxyAnchor(const QString &fName, const QString &,
            const QString &anchor, const QString &, const QString &)
{
   if (! m_inListItem[m_levelListItem] && !m_descTable) {
      if (! m_firstMember) {
         m_textStream << "    </section>";
      }
      m_firstMember = false;
   }

   if (! anchor.isEmpty()) {
      m_textStream << "<anchor xml:id=\"_" << stripPath(fName) << "_1" << anchor << "\"/>";
   }
}

void DocbookGenerator::endDoxyAnchor(const QString &, const QString &)
{
}

void DocbookGenerator::startMemberDocName(bool)
{
   m_textStream << "<para><computeroutput>";
}

void DocbookGenerator::endMemberDocName()
{
}

void DocbookGenerator::startMemberGroupHeader(bool)
{
   m_textStream << "<simplesect><title>";
}

void DocbookGenerator::endMemberGroupHeader()
{
   m_textStream << "</title>" << endl;
}

void DocbookGenerator::startMemberGroup()
{
}

void DocbookGenerator::endMemberGroup(bool)
{
   m_textStream << "</simplesect>" << endl;
}

void DocbookGenerator::startClassDiagram()
{
   m_textStream << "<para>";
}

void DocbookGenerator::endClassDiagram(const ClassDiagram &d, const QString &fileName, const QString &)
{
   m_textStream << "    <informalfigure>" << endl;
   m_textStream << "        <mediaobject>" << endl;
   m_textStream << "            <imageobject>" << endl;

   m_textStream << "                <imagedata width=\"50%\" align=\"center\" valign=\"middle\" scalefit=\"0\" fileref=\""
                << m_relPath << fileName << ".png\">" << "</imagedata>" << endl;

   m_textStream << "            </imageobject>" << endl;

   d.writeImage(m_textStream, m_outputDir, m_relPath, fileName, false);

   m_textStream << "        </mediaobject>" << endl;
   m_textStream << "    </informalfigure>" << endl;
   m_textStream << "</para>" << endl;
}

void  DocbookGenerator::startLabels()
{
}

void  DocbookGenerator::writeLabel(const QString &labelName, bool isLast)
{
   m_textStream << "<computeroutput>[" << labelName << "]</computeroutput>";

   if (! isLast) {
      m_textStream << ", ";
   }
}

void  DocbookGenerator::endLabels()
{
}

void DocbookGenerator::startSubsubsection()
{
   m_textStream << "<simplesect><title>";
}

void DocbookGenerator::endSubsubsection()
{
   m_textStream << "</title></simplesect>" << endl;
}

void DocbookGenerator::writeChar(char c)
{
   QString tmp = QChar(c);
   docify(tmp);
}

void DocbookGenerator::startMemberDocPrefixItem()
{
   m_textStream << "<computeroutput>";
}

void DocbookGenerator::endMemberDocPrefixItem()
{
   m_textStream << "</computeroutput>";
}

void DocbookGenerator::exceptionEntry(const QString &prefix, bool closeBracket)
{
   if (! prefix.isEmpty()) {
      m_textStream << " " << prefix << "(";

   } else if (closeBracket) {
      m_textStream << ")";
   }

   m_textStream << " ";
}

void DocbookGenerator::startParameterName(bool)
{
   m_textStream << " ";
}

void DocbookGenerator::endParameterName(bool last, bool, bool closeBracket)
{
   if (last) {
      if (closeBracket) {
         m_textStream << ")";
      }
   }
}

void DocbookGenerator::startMemberTemplateParams()
{
}

void DocbookGenerator::endMemberTemplateParams(const QString &, const QString &)
{
   m_textStream << "</para>";
   m_textStream << "<para>";
}

void DocbookGenerator::startSection(const QString &lab, const QString &, SectionInfo::SectionType)
{
   m_textStream << "    <section xml:id=\"_" << stripPath(lab) << "\">";
   m_textStream << "<title>";
}

void DocbookGenerator::endSection(const QString &, SectionInfo::SectionType)
{
   m_textStream << "</title>";
   m_textStream << "    </section>";
}

void DocbookGenerator::startDescTable(const QString &title)
{
   int ncols = 2;

   m_textStream << "<informaltable frame=\"all\">" << endl;

   if (! title.isEmpty()) {
      m_textStream << "<title>" << convertToDocBook(title) << "</title>" << endl;
   }

   m_textStream << "    <tgroup cols=\"" << ncols << "\" align=\"left\" colsep=\"1\" rowsep=\"1\">" << endl;

   for (int i = 0; i < ncols; ++i) {
      m_textStream << "      <colspec colname='c" << i + 1 << "'/>\n";
   }

   m_textStream << "<tbody>\n";
   m_descTable = true;
}

void DocbookGenerator::endDescTable()
{
   m_textStream << "    </tbody>" << endl;
   m_textStream << "    </tgroup>" << endl;
   m_textStream << "</informaltable>" << endl;

   m_descTable = false;
}

void DocbookGenerator::startDescTableRow()
{
   m_textStream << "<row>";
   m_textStream << "<entry>";
}

void DocbookGenerator::endDescTableRow()
{
   m_textStream << "</row>";
}

void DocbookGenerator::startDescTableTitle()
{
}

void DocbookGenerator::endDescTableTitle()
{
}

void DocbookGenerator::startDescTableData()
{
   m_textStream << "</entry><entry>";
}

void DocbookGenerator::endDescTableData()
{
   m_textStream << "</entry>";
}

void DocbookGenerator::startGroupCollaboration()
{
}

void DocbookGenerator::endGroupCollaboration(const DotGroupCollaboration &g)
{
   g.writeGraph(m_textStream, GOF_BITMAP, EOF_DocBook, m_outputDir, m_fileName, m_relPath, false);
}

void DocbookGenerator::startDotGraph()
{
}

void DocbookGenerator::endDotGraph(const DotClassGraph &g)
{
   g.writeGraph(m_textStream, GOF_BITMAP, EOF_DocBook, m_outputDir, m_fileName, m_relPath, true, false);
}

void DocbookGenerator::startInclDepGraph()
{
}

void DocbookGenerator::endInclDepGraph(const DotInclDepGraph &g)
{
   g.writeGraph(m_textStream, GOF_BITMAP, EOF_DocBook, m_outputDir, m_fileName, m_relPath, false);
}

void DocbookGenerator::startCallGraph()
{
}

void DocbookGenerator::endCallGraph(const DotCallGraph &g)
{
   g.writeGraph(m_textStream, GOF_BITMAP, EOF_DocBook, m_outputDir, m_fileName, m_relPath, false);
}

void DocbookGenerator::startDirDepGraph()
{
}

void DocbookGenerator::endDirDepGraph(const DotDirDeps &g)
{
   g.writeGraph(m_textStream, GOF_BITMAP, EOF_DocBook, m_outputDir, m_fileName, m_relPath, false);
}

void DocbookGenerator::startMemberDocList()
{
}

void DocbookGenerator::endMemberDocList()
{
   m_inGroup = true;
}

void DocbookGenerator::startConstraintList(const QString &header)
{
   m_textStream << "<simplesect><title>";
   docify(header);
   m_textStream << "</title>" << endl;
}

void DocbookGenerator::startConstraintParam()
{
   m_textStream << "<para><emphasis role=\"strong\">";
}

void DocbookGenerator::endConstraintParam()
{
}

void DocbookGenerator::startConstraintType()
{
   m_textStream << ":";
}

void DocbookGenerator::endConstraintType()
{
   m_textStream << "</emphasis></para>" << endl;
}

void DocbookGenerator::startConstraintDocs()
{
}

void DocbookGenerator::endConstraintDocs()
{
}

void DocbookGenerator::endConstraintList()
{
   m_textStream << "</simplesect>" << endl;
}
