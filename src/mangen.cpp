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

#include <QDir>
#include <QTextStream>

#include <stdlib.h>
#include <string.h>

#include <mangen.h>
#include <config.h>
#include <docparser.h>
#include <doxy_globals.h>
#include <language.h>
#include <mandocvisitor.h>
#include <message.h>
#include <util.h>

//   http://www.cubic.org/source/archive/fileform/txt/man/

static QString getSubdir()
{
   QString dirName = Config::getString("man-subdir");

   if (dirName.isEmpty()) {
      dirName = "man" + Config::getString("man-extension");
   }

   return dirName;
}

ManGenerator::ManGenerator()
   : OutputGenerator()
{
   m_dir = Config::getString("man-output") + "/" + getSubdir();

   col = 0;

   firstCol  = true;
   paragraph = true;
   upperCase = false;

   insideTabbing = false;
   inHeader      = false;
}

ManGenerator::~ManGenerator()
{
}

//void ManGenerator::append(const OutputGenerator *g)
//{
//  QByteArray r=g->getContents();
//  if (upperCase)
//    t << r.upper();
//  else
//    t << r;
//  if (!r.isEmpty())
//    firstCol = r.at(r.length()-1)=='\n';
//  else
//    firstCol = ((ManGenerator *)g)->firstCol;
//  col+=((ManGenerator *)g)->col;
//  inHeader=((ManGenerator *)g)->inHeader;
//  paragraph=false;
//}

void ManGenerator::init()
{
   QString manOutput = Config::getString("man-output");

   QDir d(manOutput);

   if (! d.exists() && ! QDir::current().mkpath(manOutput)) {
      err("Unable to create Man output directory %s\n", csPrintable(manOutput));
      Doxy_Work::stopDoxyPress();
   }

   QString subdir = getSubdir();
   d.setPath(manOutput + "/" + subdir);

   if (! d.exists() && ! QDir::current().mkpath(manOutput + "/" + subdir)) {
      err("Unable to create output directory %s/%s\n", csPrintable(manOutput), csPrintable(subdir));
      Doxy_Work::stopDoxyPress();
   }

   createSubDirs(d);
}

static QString buildFileName(const QString &name)
{
   static const QString manExtension = "." + Config::getString("man-extension");

   if (name.isEmpty()) {
      return QString("noname");
   }

   QString fname;
   QString::const_iterator iter = name.constBegin();

   while (iter != name.constEnd()) {
      QChar c = *iter;
      ++iter;

      switch (c.unicode()) {
         case ':':
            fname += "_";

            if (iter != name.constEnd() && *iter == ':') {
               ++iter;
            }
            break;

         case '<':
         case '>':
         case '&':
         case '*':
         case '!':
         case '^':
         case '~':
         case '%':
         case '+':
         case '/':
            fname += "_";
            break;

         default:
            fname += c;
      }
   }

   if (fname.right(manExtension.length()) != manExtension) {
      fname += manExtension;
   }

   return fname;
}

void ManGenerator::startFile(const QString &, const QString &manName, const QString &title)
{
   startPlainFile(buildFileName(manName));
   firstCol = true;
}

void ManGenerator::endFile()
{
   m_textStream << endl;
   endPlainFile();
}

void ManGenerator::endTitleHead(const QString &, const QString &name)
{
   QString extension = Config::getString("man-extension");

   m_textStream << ".TH \"" << name << "\" " << extension << " \""
     << dateToString(false) << "\" \"";

   QString projectName    = Config::getString("project-name");
   QString projectVersion = Config::getString("project-version");

   if (! projectVersion.isEmpty()) {
      m_textStream << "Version " << projectVersion<< "\" \"";
   }

   if (projectName.isEmpty()) {
      m_textStream << "DoxyPress";

   } else {
      m_textStream << projectName;

   }

   m_textStream << "\" \\\" -*- nroff -*-" << endl;
   m_textStream << ".ad l" << endl;
   m_textStream << ".nh" << endl;
   m_textStream << ".SH NAME" << endl;
   m_textStream << name;

   firstCol  = false;
   paragraph = true;
   inHeader  = true;
}

void ManGenerator::newParagraph()
{
   if (! paragraph) {
      if (! firstCol) {
         m_textStream << endl;
      }

      m_textStream << ".PP" << endl;
      firstCol = true;
   }

   paragraph = true;
}

void ManGenerator::startParagraph(const QString &className)
{
   if (! paragraph) {
      if (! firstCol) {
         m_textStream << endl;
      }

      m_textStream << ".PP" << endl;
      firstCol = true;
   }

   paragraph = true;
}

void ManGenerator::endParagraph()
{
}

void ManGenerator::writeString(const QString &text)
{
   docify(text);
}

void ManGenerator::startIndexItem(const QString &, const QString &)
{
}

void ManGenerator::endIndexItem(const QString &, const QString &)
{
}

void ManGenerator::writeStartAnnoItem(const QString &, const QString &, const QString &, const QString &)
{
}

void ManGenerator::writeObjectLink(const QString &, const QString &, const QString &, const QString &name)
{
   startBold();
   docify(name);
   endBold();
}

void ManGenerator::writeCodeLink(const QString &, const QString &, const QString &,
                                 const QString &name, const QString &)
{
   docify(name);
}

void ManGenerator::startHtmlLink(const QString &)
{
}

void ManGenerator::endHtmlLink()
{
}

//void ManGenerator::writeMailLink(const QString &url)
//{
//  docify(url);
//}

void ManGenerator::startGroupHeader(int)
{
   if (!firstCol) {
      m_textStream << endl;
   }

   m_textStream << ".SH \"";
   upperCase = true;
   firstCol  = false;
}

void ManGenerator::endGroupHeader(int)
{
   m_textStream << "\"\n.PP " << endl;
   firstCol  = true;
   paragraph = true;
   upperCase = false;
}

void ManGenerator::startMemberHeader(const QString &)
{
   if (! firstCol) {
      m_textStream << endl;
   }

   m_textStream << ".SS \"";
}

void ManGenerator::endMemberHeader()
{
   m_textStream << "\"\n";
   firstCol  = true;
   paragraph = false;
}

void ManGenerator::docify(const QString &text)
{
   if (text.isEmpty()) {
      return;
   }

   for (auto c : text) {

      switch (c.unicode()) {
         case '-':
            m_textStream << "\\-";
            break;

         case '.':
            m_textStream << "\\&.";
            break;

         case '\\':
            m_textStream << "\\\\";
            col++;
            break;

         case '\n':
            m_textStream << "\n";
            col = 0;
            break;

         case '\"':
            c = '\'';
            // no break is correct

         default:
            m_textStream << c;
            col++;
            break;
      }

      firstCol = (c == '\n');
   }

   paragraph = false;
}

void ManGenerator::codify(const QString &str)
{
   if (str.isEmpty()) {
      return;
   }

   int spacesToNextTabStop;

   for (auto c : str) {

      switch (c.unicode()) {
         case '.':
            m_textStream << "\\&.";
            break;

         case '\t':
            spacesToNextTabStop = Config::getInt("tab-size") - (col %  Config::getInt("tab-size"));

            m_textStream << QString(spacesToNextTabStop, ' ');
            col += spacesToNextTabStop;

            break;

         case '\n':
            m_textStream << "\n";
            firstCol = true;
            col = 0;
            break;

         case '\\':
            m_textStream << "\\";
            col++;
            break;

         case '\"':  // no break is correct

         default:
            m_textStream << c;
            firstCol = false;
            col++;
            break;
      }
   }

   paragraph = false;
}

void ManGenerator::writeChar(char c)
{
   firstCol = (c == '\n');

   if (firstCol) {
      col = 0;
   } else {
      col++;
   }

   switch (c) {
      case '\\':
         m_textStream << "\\\\";
         break;
      case '\"':
         c = '\''; // no break!
      default:
         m_textStream << c;
         break;
   }

   paragraph = false;
}

void ManGenerator::startDescList(SectionTypes)
{
   if (! firstCol) {
      m_textStream << endl << ".PP" << endl;
      firstCol  = true;
      paragraph = true;
      col = 0;
   }

   paragraph = false;
   startBold();
}

void ManGenerator::startTitle()
{
   if (! firstCol) {
      m_textStream << endl;
   }

   m_textStream << ".SH \"";
   firstCol  = false;
   paragraph = false;
}

void ManGenerator::endTitle()
{
   m_textStream << "\"";
}

void ManGenerator::startItemListItem()
{
   if (! firstCol) {
      m_textStream << endl;
   }

   m_textStream << ".TP" << endl;
   firstCol  = true;
   paragraph = false;
   col = 0;
}

void ManGenerator::endItemListItem()
{
}

void ManGenerator::startCodeFragment()
{
   newParagraph();
   m_textStream << ".nf" << endl;
   firstCol  = true;
   paragraph = false;
}

void ManGenerator::endCodeFragment()
{
   if (!firstCol) {
      m_textStream << endl;
   }
   m_textStream << ".fi" << endl;
   firstCol = true;
   paragraph = false;
   col = 0;
}

void ManGenerator::startMemberDoc(const QString &, const QString &, const QString &, const QString &, bool)
{
   if (! firstCol) {
      m_textStream << endl;
   }

   m_textStream << ".SS \"";
   firstCol  = false;
   paragraph = false;
}

void ManGenerator::startDoxyAnchor(const QString &, const QString &manName, const QString &, const QString &name, const QString &)
{
   if ( ! Config::getBool("man-links") ) {
      return;
   }

   // name of the link file is derived from the name of the anchor
   // truncate after an (optional) ::

   QString baseName = name;
   int i = baseName.lastIndexOf("::");

   if (i != -1) {
      baseName = baseName.right(baseName.length() - i - 2);
   }

   // remove dangerous characters and append suffix, then add dir prefix
   QString fname = m_dir + "/" + buildFileName(baseName);
   QFile linkfile(fname);

   // only create file if it does not exist
   if (! linkfile.open( QIODevice::ReadOnly ) ) {
      if ( linkfile.open( QIODevice::WriteOnly ) ) {
         QTextStream linkstream;

         linkstream.setDevice(&linkfile);
         linkstream << ".so " << getSubdir() << "/" << buildFileName(manName) << endl;
      }
   }

   linkfile.close();
}

void ManGenerator::endMemberDoc(bool)
{
   m_textStream << "\"\n";
}

void ManGenerator::startSubsection()
{
   if (! firstCol) {
      m_textStream << endl;
   }

   m_textStream << ".SS \"";
   firstCol = false;
   paragraph = false;
}

void ManGenerator::endSubsection()
{
   m_textStream << "\"";
}

void ManGenerator::startSubsubsection()
{
   if (!firstCol) {
      m_textStream << endl;
   }

   m_textStream << "\n.SS \"";
   firstCol = false;
   paragraph = false;
}

void ManGenerator::endSubsubsection()
{
   m_textStream << "\"";
}

void ManGenerator::writeSynopsis()
{
   if (! firstCol) {
      m_textStream << endl;
   }

   m_textStream << ".SH SYNOPSIS\n.br\n.PP\n";
   firstCol  = true;
   paragraph = false;
}

void ManGenerator::startDescItem()
{
   if (! firstCol) {
      m_textStream << endl;
   }

   m_textStream << ".IP \"";
   firstCol = false;
}

//void ManGenerator::endDescTitle()
//{
//  endBold();
//  paragraph=true;
//}

void ManGenerator::startDescForItem()
{
   if (! firstCol) {
      m_textStream << endl;
   }

   if (! paragraph) {
      m_textStream << ".in -1c" << endl;
   }

   m_textStream << ".in +1c" << endl;
   firstCol = true;
   paragraph = false;
   col = 0;
}

void ManGenerator::endDescForItem()
{
}

void ManGenerator::endDescItem()
{
   m_textStream << "\" 1c" << endl;
   firstCol = true;
}

void ManGenerator::startAnonTypeScope(int indentLevel)
{
   if (indentLevel == 0) {
      insideTabbing = true;
   }
}

void ManGenerator::endAnonTypeScope(int indentLevel)
{
   if (indentLevel == 0) {
      insideTabbing = false;
   }
}

void ManGenerator::startMemberItem(const QString &, int, const QString &, bool deprecated)
{
   if (firstCol && !insideTabbing) {
      m_textStream << ".in +1c\n";
   }
   m_textStream << "\n.ti -1c\n.RI \"";
   firstCol = false;
}

void ManGenerator::endMemberItem()
{
   m_textStream << "\"\n.br";
}

void ManGenerator::startMemberList()
{
   if (!insideTabbing) {
      m_textStream << "\n.in +1c";
      firstCol = false;
   }
}

void ManGenerator::endMemberList()
{
   if (!insideTabbing) {
      m_textStream << "\n.in -1c";
      firstCol = false;
   }
}

void ManGenerator::startMemberGroupHeader(bool)
{
   m_textStream << "\n.PP\n.RI \"\\fB";
}

void ManGenerator::endMemberGroupHeader()
{
   m_textStream << "\\fP\"\n.br\n";
   firstCol = true;
}

void ManGenerator::startMemberGroupDocs()
{
}

void ManGenerator::endMemberGroupDocs()
{
   m_textStream << "\n.PP";
}

void ManGenerator::startMemberGroup()
{
   m_textStream << "\n.in +1c";
}

void ManGenerator::endMemberGroup(bool)
{
   m_textStream << "\n.in -1c";
   firstCol = false;
}

void ManGenerator::startSection(const QString &, const QString &, SectionInfo::SectionType type)
{
   if (! inHeader) {
      switch (type) {
         case SectionInfo::Page:
            startGroupHeader(false);
            break;

         case SectionInfo::Section:
            startGroupHeader(false);
            break;

         case SectionInfo::Subsection:
            startMemberHeader(0);
            break;

         case SectionInfo::Subsubsection:
            startMemberHeader(0);
            break;

         case SectionInfo::Paragraph:
            startMemberHeader(0);
            break;

         default:
            assert(0);
            break;
      }
   }
}

void ManGenerator::endSection(const QString &, SectionInfo::SectionType type)
{
   if (! inHeader ) {
      switch (type) {
         case SectionInfo::Page:
            endGroupHeader(0);
            break;
         case SectionInfo::Section:
            endGroupHeader(0);
            break;
         case SectionInfo::Subsection:
            endMemberHeader();
            break;
         case SectionInfo::Subsubsection:
            endMemberHeader();
            break;
         case SectionInfo::Paragraph:
            endMemberHeader();
            break;
         default:
            assert(0);
            break;
      }

   } else {
      m_textStream << "\n";
      firstCol  = true;
      paragraph = false;
      inHeader  = false;
   }
}

void ManGenerator::startSimpleSect(SectionTypes, const QString &, const QString &, const QString &title)
{
   if (! firstCol) {
      m_textStream << endl << ".PP" << endl;
      firstCol  = true;
      paragraph = true;
      col = 0;
   }

   paragraph = false;
   startBold();
   docify(title);
   endBold();
   paragraph = true;
}

void ManGenerator::endSimpleSect()
{
}

void ManGenerator::startParamList(ParamListTypes, const QString &title)
{
   if (!firstCol) {
      m_textStream << endl << ".PP" << endl;
      firstCol = true;
      paragraph = true;
      col = 0;
   }

   paragraph = false;
   startBold();
   docify(title);
   endBold();
   paragraph = true;
}

void ManGenerator::endParamList()
{
}

void ManGenerator::writeDoc(DocNode *n, QSharedPointer<Definition> ctx, QSharedPointer<MemberDef> md)
{
   ManDocVisitor *visitor = new ManDocVisitor(m_textStream, *this, ctx ? ctx->getDefFileExtension() : QString(""));
   n->accept(visitor);
   delete visitor;

   firstCol  = false;
   paragraph = false;
}

void ManGenerator::startConstraintList(const QString &header)
{
   if (! firstCol) {
      m_textStream << endl << ".PP" << endl;
      firstCol = true;
      paragraph = true;
      col = 0;
   }

   paragraph = false;
   startBold();
   docify(header);
   endBold();
   paragraph = true;
}

void ManGenerator::startConstraintParam()
{
   startItemListItem();
   startEmphasis();
}

void ManGenerator::endConstraintParam()
{
   endEmphasis();
   endItemListItem();
   m_textStream << " : ";
}

void ManGenerator::startConstraintType()
{
   startEmphasis();
}

void ManGenerator::endConstraintType()
{
   endEmphasis();
}

void ManGenerator::startConstraintDocs()
{
}

void ManGenerator::endConstraintDocs()
{
   m_textStream << endl;
   firstCol = true;
}

void ManGenerator::endConstraintList()
{
}

void ManGenerator::startInlineHeader()
{
   if (!firstCol) {
      m_textStream << endl << ".PP" << endl << ".in -1c" << endl;
   }
   m_textStream << ".RI \"\\fB";
}

void ManGenerator::endInlineHeader()
{
   m_textStream << "\\fP\"" << endl << ".in +1c" << endl;
   firstCol = false;
}

void ManGenerator::startMemberDocSimple(bool isEnum)
{
   if (! firstCol) {
      m_textStream << endl << ".PP" << endl;
   }

   m_textStream << "\\fB";

   if (isEnum) {
      docify(theTranslator->trEnumerationValues());
   } else {
      docify(theTranslator->trCompoundMembers());
   }

   m_textStream << ":\\fP" << endl;
   m_textStream << ".RS 4" << endl;
}

void ManGenerator::endMemberDocSimple(bool isEnum)
{
   if (! firstCol) {
      m_textStream << endl;
   }

   m_textStream << ".RE" << endl;
   m_textStream << ".PP" << endl;
   firstCol = true;
}

void ManGenerator::startInlineMemberType()
{
}

void ManGenerator::endInlineMemberType()
{
   m_textStream << " ";
}

void ManGenerator::startInlineMemberName()
{
   m_textStream << "\\fI";
}

void ManGenerator::endInlineMemberName()
{
   m_textStream << "\\fP ";
}

void ManGenerator::startInlineMemberDoc()
{
}

void ManGenerator::endInlineMemberDoc()
{
   if (! firstCol) {
      m_textStream << endl;
   }
   m_textStream << ".br" << endl;
   m_textStream << ".PP" << endl;
   firstCol = true;
}

void ManGenerator::startLabels()
{
}

void ManGenerator::writeLabel(const QString &l, bool isLast)
{
   m_textStream << "\\fC [" << l << "]\\fP";
   if (! isLast) {
      m_textStream << ", ";
   }
}

void ManGenerator::endLabels()
{
}

void ManGenerator::endHeaderSection()
{
}
