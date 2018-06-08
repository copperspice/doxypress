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

#include <QRegularExpression>

#include <pagedef.h>

#include <config.h>
#include <docparser.h>
#include <doxy_globals.h>
#include <groupdef.h>
#include <language.h>
#include <outputlist.h>
#include <util.h>

PageDef::PageDef(const QString &f, int l, const QString &name, const QString &d, const QString &t)
   : Definition(f, l, 1, name), m_title(t)
{
   static const bool shortNames = Config::getBool("short-names");

   setDocumentation(d, f, l);

   m_subPageDict  = new PageSDict();
   m_nestingLevel = 0;
   m_showToc      = false;
   m_fileName     = convertNameToFile_X(name, false, true);
}

PageDef::~PageDef()
{
   delete m_subPageDict;
}

void PageDef::findSectionsInDocumentation()
{
   QSharedPointer<PageDef> self = sharedFrom(this);
   docFindSections(documentation(), self, QSharedPointer<MemberGroup>(), docFile());
}

QSharedPointer<GroupDef> PageDef::getGroupDef() const
{
   SortedList<QSharedPointer<GroupDef>> *groups = partOfGroups();

   if (groups) {
       return groups->first();

   } else {
      return QSharedPointer<GroupDef>();

   }
}

QString PageDef::getOutputFileBase() const
{
   if (getGroupDef()) {
      return getGroupDef()->getOutputFileBase();

   } else {
      return m_fileName;

   }
}

void PageDef::setFileName(const QString &name)
{
   m_fileName = name;
}

void PageDef::addInnerCompound(QSharedPointer<Definition> d)
{
   QSharedPointer<PageDef> self = sharedFrom(this);

   if (d->definitionType() == Definition::TypePage) {

      QSharedPointer<PageDef> pd = d.dynamicCast<PageDef>();
      assert(pd);

      m_subPageDict->insert(pd->name(), pd);

      d->setOuterScope(self);

      if (this == Doxy_Globals::mainPage) {
         pd->setNestingLevel(m_nestingLevel);

      } else {
         pd->setNestingLevel(m_nestingLevel + 1);

      }
   }
}

bool PageDef::hasParentPage() const
{
   return getOuterScope() && getOuterScope()->definitionType() == Definition::TypePage;
}


QString PageDef::pathFragment_Internal() const
{
   return title();
}

void PageDef::writeTagFile(QTextStream &tagFile)
{
   bool found = (name() == "citelist");

   if (! found ) {
      for (auto item : Doxy_Globals::xrefLists) {

         if (item.listName() == name()) {
            found = true;
            break;
         }
      }
   }

   if (! found) {
      // not one of the generated related pages

      tagFile << "  <compound kind=\"page\">" << endl;
      tagFile << "    <name>" << name() << "</name>" << endl;
      tagFile << "    <title>" << convertToXML(title()) << "</title>" << endl;
      tagFile << "    <filename>" << convertToXML(getOutputFileBase()) << "</filename>" << endl;

      writeDocAnchorsToTagFile(tagFile);

      tagFile << "  </compound>" << endl;
   }
}

void PageDef::writeDocumentation(OutputList &ol)
{
   QSharedPointer<PageDef> self = sharedFrom(this);

   static bool generateTreeView = Config::getBool("generate-treeview");

   QString pageName;
   QString manPageName;

   pageName    = escapeCharsInString(name(), false, true);
   manPageName = escapeCharsInString(name(), true, true);

   ol.pushGeneratorState();

   if (m_nestingLevel > 0) {
      // do not generate sub page output for RTF and LaTeX, these are part of their parent page

      ol.disableAll();
      ol.enable(OutputGenerator::Man);
      ol.enable(OutputGenerator::Html);
   }

   ol.pushGeneratorState();

   //
   ol.disableAllBut(OutputGenerator::Man);
   startFile(ol, getOutputFileBase(), manPageName, title(), HLI_Pages, !generateTreeView);
   ol.enableAll();

   ol.disable(OutputGenerator::Man);
   startFile(ol, getOutputFileBase(), pageName, title(), HLI_Pages, !generateTreeView);
   ol.popGeneratorState();

   if (! generateTreeView) {
      if (getOuterScope() != Doxy_Globals::globalScope && ! Config::getBool("disable-index")) {
         getOuterScope()->writeNavigationPath(ol);
      }
      ol.endQuickIndices();
   }

   QSharedPointer<SectionInfo> si = Doxy_Globals::sectionDict.find(name());

   // save old generator state and write title only to Man generator
   ol.pushGeneratorState();

   ol.disableAllBut(OutputGenerator::Man);
   ol.startTitleHead(manPageName);
   ol.endTitleHead(manPageName, manPageName);

   if (si) {
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Man);
      ol.writeString(" - ");
      ol.popGeneratorState();

      if (si->title != manPageName) {
         ol.generateDoc(docFile(), docLine(), self, QSharedPointer<MemberDef>(), si->title, true, false, "", true, false);
         ol.endSection(si->label, si->type);
      }
   }
   ol.popGeneratorState();

   // for Latex the section is already generated as a chapter in the index!
   ol.pushGeneratorState();

   //2.{
   ol.disable(OutputGenerator::Latex);
   ol.disable(OutputGenerator::RTF);
   ol.disable(OutputGenerator::Man);

   if (! title().isEmpty() && ! name().isEmpty() && si != nullptr) {

      ol.startPageDoc(si->title);
      startTitle(ol, getOutputFileBase(), self);

      ol.generateDoc(docFile(), docLine(), self, QSharedPointer<MemberDef>(), si->title, true, false, "", true, false);

      endTitle(ol, getOutputFileBase(), name());

   } else {
      ol.startPageDoc("");
   }

   ol.startContents();
   ol.popGeneratorState();

   if (m_showToc && hasSections()) {
      writeToc(ol);
   }

   writePageDocumentation(ol);
   ol.endPageDoc();

   if (generateTreeView && getOuterScope() != Doxy_Globals::globalScope && ! Config::getBool("disable-index")) {
      ol.endContents();
      endFileWithNavPath(getOuterScope(), ol);

   } else {
      endFile(ol);
   }

   ol.popGeneratorState();

   Doxy_Globals::indexList.addIndexItem(self, QSharedPointer<MemberDef>(), 0, filterTitle(title()));
}

void PageDef::writePageDocumentation(OutputList &ol)
{
   QSharedPointer<PageDef> self = sharedFrom(this);
   bool markdownEnabled = Doxy_Globals::markdownSupport;

   if (getLanguage() == SrcLangExt_Markdown) {
      Doxy_Globals::markdownSupport = true;
   }

   ol.startTextBlock();

   QString docStr = documentation() + inbodyDocumentation();

   if (hasBriefDescription() && ! Doxy_Globals::sectionDict.find(name())) {
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Man);
      ol.writeString(" - ");
      ol.popGeneratorState();
   }

   ol.generateDoc(docFile(), docLine(), self, QSharedPointer<MemberDef>(),
                  docStr, true, false);

   ol.endTextBlock();

   Doxy_Globals::markdownSupport = markdownEnabled;

   if (hasSubPages()) {
      // for printed documentation we write subpages as section's of the parent page.

      ol.pushGeneratorState();
      ol.disableAll();
      ol.enable(OutputGenerator::Latex);
      ol.enable(OutputGenerator::RTF);

      for (auto subPage : *m_subPageDict) {
         SectionInfo::SectionType sectionType = SectionInfo::Paragraph;

         switch (m_nestingLevel) {
            case  0:
               sectionType = SectionInfo::Page;
               break;
            case  1:
               sectionType = SectionInfo::Section;
               break;
            case  2:
               sectionType = SectionInfo::Subsection;
               break;
            case  3:
               sectionType = SectionInfo::Subsubsection;
               break;
            default:
               sectionType = SectionInfo::Paragraph;
               break;
         }

         QString title = subPage->title();
         if (title.isEmpty()) {
            title = subPage->name();
         }

         ol.startSection(subPage->name(), title, sectionType);
         ol.parseText(title);
         ol.endSection(subPage->name(), sectionType);

         Doxy_Globals::subpageNestingLevel++;
         subPage->writePageDocumentation(ol);
         Doxy_Globals::subpageNestingLevel--;
      }

      ol.popGeneratorState();
   }
}

bool PageDef::visibleInIndex() const
{
   static bool externalPages = Config::getBool("external-pages");

   return ! getGroupDef() && (!isReference() || externalPages);
}

bool PageDef::documentedPage() const
{
   return ! getGroupDef() && ! isReference();
}

bool PageDef::hasSubPages() const
{
   return m_subPageDict->count() > 0;
}

void PageDef::setNestingLevel(int l)
{
   m_nestingLevel = l;
}

void PageDef::setShowToc(bool b)
{
   m_showToc |= b;
}

