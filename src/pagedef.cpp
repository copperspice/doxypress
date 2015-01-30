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
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#include <QRegExp>

#include <config.h>
#include <docparser.h>
#include <doxy_globals.h>
#include <doxygen.h>
#include <groupdef.h>
#include <language.h>
#include <namespacedef.h>
#include <outputlist.h>
#include <pagedef.h>
#include <reflist.h>
#include <util.h>

PageDef::PageDef(const char *f, int l, const char *n, const char *d, const char *t)
   : Definition(f, l, 1, n), m_title(t)
{
   setDocumentation(d, f, l);

   m_subPageDict  = new PageSDict();   
   m_nestingLevel = 0;
   m_showToc      = false;
}

PageDef::~PageDef()
{
   delete m_subPageDict;
}

void PageDef::findSectionsInDocumentation()
{
   QSharedPointer<PageDef> self = sharedFrom(this);
   docFindSections(documentation(), self, 0, docFile());
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

QByteArray PageDef::getOutputFileBase() const
{
   if (getGroupDef()) {
      return getGroupDef()->getOutputFileBase();

   } else {
      return m_fileName;

   }
}

void PageDef::setFileName(const char *name, bool dontEscape)
{
   static bool shortNames = Config_getBool("SHORT_NAMES");

   if (shortNames && !dontEscape) {
      m_fileName = convertNameToFile(name).toUtf8();

   } else {
      m_fileName = name;
   }
}

void PageDef::addInnerCompound(QSharedPointer<Definition> d)
{
   QSharedPointer<PageDef> self = sharedFrom(this);

   if (d->definitionType() == Definition::TypePage) {

      QSharedPointer<PageDef> pd = d.dynamicCast<PageDef>();
      assert(pd);    
           
      m_subPageDict->insert(pd->name(), pd);

      d->setOuterScope(self);

      if (this == Doxygen::mainPage) {
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

void PageDef::writeTagFile(QTextStream &tagFile)
{
   bool found = (name() == "citelist");
   
   if (! found ) {      
      for (auto item : *Doxygen::xrefLists) {         
   
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

   static bool generateTreeView = Config_getBool("GENERATE_TREEVIEW");
   
   QByteArray pageName;
   QByteArray manPageName;

   pageName    = escapeCharsInString(name(), false, true);
   manPageName = escapeCharsInString(name(), true, true);

   ol.pushGeneratorState();
   //1.{

   if (m_nestingLevel > 0) {
      //&& // a sub page
      //(Doxygen::mainPage==0 || getOuterScope()!=Doxygen::mainPage) // and not a subpage of the mainpage
     
      // do not generate sub page output for RTF and LaTeX, as these are
      // part of their parent page
      ol.disableAll();
      ol.enable(OutputGenerator::Man);
      ol.enable(OutputGenerator::Html);
   }

   ol.pushGeneratorState();

   //2.{
   ol.disableAllBut(OutputGenerator::Man);
   startFile(ol, getOutputFileBase(), manPageName, title(), HLI_Pages, !generateTreeView);
   ol.enableAll();

   ol.disable(OutputGenerator::Man);
   startFile(ol, getOutputFileBase(), pageName, title(), HLI_Pages, !generateTreeView);
   ol.popGeneratorState();
   //2.}

   if (! generateTreeView) {
      if (getOuterScope() != Doxygen::globalScope && !Config_getBool("DISABLE_INDEX")) {
         getOuterScope()->writeNavigationPath(ol);
      }
      ol.endQuickIndices();
   }

   QSharedPointer<SectionInfo> si = Doxygen::sectionDict->find(name());

   // save old generator state and write title only to Man generator
   ol.pushGeneratorState();

   //2.{
   ol.disableAllBut(OutputGenerator::Man);
   ol.startTitleHead(manPageName);
   ol.endTitleHead(manPageName, manPageName);

   if (si) {
      ol.generateDoc(docFile(), docLine(), self, QSharedPointer<MemberDef>(), si->title, true, false, 0, true, false);
      ol.endSection(si->label, si->type);
   }
   ol.popGeneratorState();
   //2.}

   // for Latex the section is already generated as a chapter in the index!
   ol.pushGeneratorState();

   //2.{
   ol.disable(OutputGenerator::Latex);
   ol.disable(OutputGenerator::RTF);
   ol.disable(OutputGenerator::Man);

   if (! title().isEmpty() && !name().isEmpty() && si != 0) {

      startTitle(ol, getOutputFileBase(), this);
      ol.generateDoc(docFile(), docLine(), self, QSharedPointer<MemberDef>(), si->title, true, false, 0, true, false);
      
      endTitle(ol, getOutputFileBase(), name());
   }

   ol.startContents();
   ol.popGeneratorState();
   //2.}

   if (m_showToc && hasSections()) {
      writeToc(ol);
   }

   writePageDocumentation(ol);

   if (generateTreeView && getOuterScope() != Doxygen::globalScope && ! Config_getBool("DISABLE_INDEX")) {
      ol.endContents();
      endFileWithNavPath(getOuterScope(), ol);

   } else {
      endFile(ol);
   }

   ol.popGeneratorState();
   //1.}

   Doxygen::indexList->addIndexItem(self, QSharedPointer<MemberDef>(), 0, filterTitle(title()));
}
 
void PageDef::writePageDocumentation(OutputList &ol)
{
   QSharedPointer<PageDef> self = sharedFrom(this);
   bool markdownEnabled = Doxygen::markdownSupport;

   if (getLanguage() == SrcLangExt_Markdown) {
      Doxygen::markdownSupport = true;
   }

   ol.startTextBlock();

   ol.generateDoc(docFile(), docLine(), self, QSharedPointer<MemberDef>(), 
                  documentation() + inbodyDocumentation(), true, false );

   ol.endTextBlock();

   Doxygen::markdownSupport = markdownEnabled;

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

         QByteArray title = subPage->title();
         if (title.isEmpty()) {
            title = subPage->name();
         }

         ol.startSection(subPage->name(), title, sectionType);
         ol.parseText(title);
         ol.endSection(subPage->name(), sectionType);

         Doxygen::subpageNestingLevel++;
         subPage->writePageDocumentation(ol);
         Doxygen::subpageNestingLevel--;
      }

      ol.popGeneratorState();
   }
}

bool PageDef::visibleInIndex() const
{
   static bool externalPages = Config_getBool("EXTERNAL_PAGES");

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
   m_showToc = b;
}

