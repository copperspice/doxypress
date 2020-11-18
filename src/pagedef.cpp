/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
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
   setDocumentation(d, f, l);

   m_subPageDict  = new PageSDict();
   m_nestingLevel = 0;
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
   static const bool generateTreeView = Config::getBool("generate-treeview");
   static const bool disableIndex     = Config::getBool("disable-index");

   QSharedPointer<PageDef> self = sharedFrom(this);

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
   startFile(ol, getOutputFileBase(), manPageName, title(), HLI_Pages, ! generateTreeView);
   ol.enableAll();

   ol.disable(OutputGenerator::Man);
   startFile(ol, getOutputFileBase(), pageName, title(), HLI_Pages, ! generateTreeView);
   ol.popGeneratorState();

   if (! generateTreeView) {
      if (getOuterScope() != Doxy_Globals::globalScope && ! disableIndex) {
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
      ol.startPageDoc(QString());
   }

   ol.startContents();
   ol.popGeneratorState();

   if ((m_localToc.isHtmlEnabled() || m_localToc.isLatexEnabled() || m_localToc.isDocbookEnabled()) && hasSections()) {
      writeToc(ol, m_localToc);
   }

   writePageDocumentation(ol);
   ol.endContents();
   ol.endPageDoc();

   if (generateTreeView && getOuterScope() != Doxy_Globals::globalScope && ! disableIndex) {
      endFileWithNavPath(getOuterScope(), ol);

   } else {
      endFile(ol);
   }

   ol.popGeneratorState();

   Doxy_Globals::indexList.addIndexItem(self, QSharedPointer<MemberDef>(), QString(), filterTitle(title()));
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

   ol.generateDoc(docFile(), docLine(), self, QSharedPointer<MemberDef>(), docStr, true, false);
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

         ++Doxy_Globals::subpageNestingLevel;
         subPage->writePageDocumentation(ol);
         --Doxy_Globals::subpageNestingLevel;
      }

      ol.popGeneratorState();
   }
}

bool PageDef::visibleInIndex() const
{
   static const bool externalPages = Config::getBool("external-pages");

   return ! getGroupDef() && (! isReference() || externalPages);
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

void PageDef::setLocalToc(const LocalToc &localToc)
{
  m_localToc = localToc;
}

bool PageDef::hasTitle() const
{
  return ! m_title.isEmpty() && m_title.toLower() != "notitle";
}

