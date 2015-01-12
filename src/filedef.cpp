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

#include <config.h>
#include <clangparser.h>
#include <classlist.h>
#include <classdef.h>
#include <dirdef.h>
#include <docparser.h>
#include <doxygen.h>
#include <dot.h>
#include <entry.h>
#include <filename.h>
#include <filedef.h>
#include <groupdef.h>
#include <htags.h>
#include <layout.h>
#include <language.h>
#include <memberdef.h>
#include <message.h>
#include <membergroup.h>
#include <memberlist.h>
#include <namespacedef.h>
#include <outputlist.h>
#include <parserintf.h>
#include <portable.h>
#include <searchindex.h>
#include <settings.h>
#include <util.h>

// must appear after the previous include - resolve soon 
#include <doxy_globals.h>


/** Class implementing CodeOutputInterface by throwing away everything. */
class DevNullCodeDocInterface : public CodeOutputInterface
{
 public:
   virtual void codify(const QByteArray &) override {}

   virtual void writeCodeLink(const QByteArray &, const QByteArray &, const QByteArray &, 
                              const QByteArray &, const QByteArray &) override  {}

   virtual void writeTooltip(const char *, const DocLinkInfo &, const QByteArray &,
                             const QByteArray &, const SourceLinkInfo &, const SourceLinkInfo & ) override
   {}

   virtual void writeLineNumber(const char *, const QByteArray &, const char *, int)  override {}
   virtual void startCodeLine(bool) {}
   virtual void endCodeLine() {}
   virtual void startFontClass(const char *) {}
   virtual void endFontClass() {}
   virtual void writeCodeAnchor(const char *) {}
   virtual void linkableSymbol(int, const char *, Definition *, Definition *) {}
   virtual void setCurrentDoc(Definition *, const char *, bool) {}

   virtual void addWord(const QString &, bool) override
   {}
};

//---------------------------------------------------------------------------

/*! create a new file definition, where \a p is the file path,
    \a nm the file name, and \a lref is an HTML anchor name if the
    file was read from a tag file or 0 otherwise
*/
FileDef::FileDef(const char *p, const char *nm, const char *lref, const char *dn)
   : Definition((QByteArray)p + nm, 1, 1, nm)
{
   m_path = p;
   m_filePath = m_path + nm;
   m_fileName = nm;
   m_diskName = dn;

   if (m_diskName.isEmpty()) {
      m_diskName = nm;
   }
   setReference(lref);
   m_classSDict        = 0;
   m_includeList       = 0;
   m_includeDict       = 0;
   m_includedByList    = 0;
   m_includedByDict    = 0;
   m_namespaceSDict    = 0;
   m_srcDefDict        = 0;
   m_srcMemberDict     = 0;
   m_usingDirList      = 0;
   m_usingDeclList     = 0;
   m_package           = 0;
   m_isSource          = guessSection(nm) == Entry::SOURCE_SEC;
   m_docname           = nm;
   m_dir               = 0;

   if (Config_getBool("FULL_PATH_NAMES")) {
      m_docname.prepend(stripFromPath(m_path).toUtf8());
   }

   setLanguage(getLanguageFromFileName(name()));

   m_memberGroupSDict = 0;
   acquireFileVersion();
   m_subGrouping = Config_getBool("SUBGROUPING");
}

/*! destroy the file definition */
FileDef::~FileDef()
{
   delete m_classSDict;
   delete m_includeDict;
   delete m_includeList;
   delete m_includedByDict;
   delete m_includedByList;
   delete m_namespaceSDict;
   delete m_srcDefDict;
   delete m_srcMemberDict;
   delete m_usingDirList;
   delete m_usingDeclList;
   delete m_memberGroupSDict;
}

/*! Compute the HTML anchor names for all members in the class */
void FileDef::computeAnchors()
{
   MemberList *ml = getMemberList(MemberListType_allMembersList);
   if (ml) {
      setAnchors(ml);
   }
}

void FileDef::distributeMemberGroupDocumentation()
{
   //printf("FileDef::distributeMemberGroupDocumentation()\n");
   if (m_memberGroupSDict) {      
      for (auto mg : *m_memberGroupSDict) {
         mg->distributeMemberGroupDocumentation();
      }
   }
}

void FileDef::findSectionsInDocumentation()
{
   docFindSections(documentation(), this, 0, docFile());

   if (m_memberGroupSDict) {
    for (auto mg : *m_memberGroupSDict) {
         mg->findSectionsInDocumentation();
      }
   } 

   for (auto ml : m_memberLists) {
      if (ml->listType() & MemberListType_declarationLists) {
         ml->findSectionsInDocumentation();
      }
   }
}

bool FileDef::hasDetailedDescription() const
{
   static bool repeatBrief = Config_getBool("REPEAT_BRIEF");
   static bool sourceBrowser = Config_getBool("SOURCE_BROWSER");

   // avail empty section

   return ((! briefDescription().isEmpty() && repeatBrief) ||
            !documentation().trimmed().isEmpty() ||  (sourceBrowser && getStartBodyLine() != -1 && getBodyDef()) );
}

void FileDef::writeTagFile(FTextStream &tagFile)
{
   tagFile << "  <compound kind=\"file\">" << endl;
   tagFile << "    <name>" << convertToXML(name()) << "</name>" << endl;
   tagFile << "    <path>" << convertToXML(getPath()) << "</path>" << endl;
   tagFile << "    <filename>" << convertToXML(getOutputFileBase()) << "</filename>" << endl;

   if (m_includeList && m_includeList->count() > 0) {
      
      for (auto ii : *m_includeList) {
         if (! ii.indirect) {
            FileDef *fd = ii.fileDef;

            if (fd && fd->isLinkable() && !fd->isReference()) {
               bool isIDLorJava = false;
               SrcLangExt lang = fd->getLanguage();

               isIDLorJava = lang == SrcLangExt_IDL || lang == SrcLangExt_Java;
               const char *locStr = (ii.local    || isIDLorJava) ? "yes" : "no";
               const char *impStr = (ii.imported || isIDLorJava) ? "yes" : "no";

               tagFile << "    <includes id=\""
                       << convertToXML(fd->getOutputFileBase()) << "\" "
                       << "name=\"" << convertToXML(fd->name()) << "\" "
                       << "local=\"" << locStr << "\" "
                       << "imported=\"" << impStr << "\">"
                       << convertToXML(ii.includeName)
                       << "</includes>"
                       << endl;
            }
         }
      }
   }
     

   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::File)) {
      switch (lde->kind())  {
      
         case LayoutDocEntry::FileClasses: 
         {
            if (m_classSDict) {              
               for (auto cd : *m_classSDict) {
                  if (cd->isLinkableInProject()) {
                     tagFile << "    <class kind=\"" << cd->compoundTypeString() <<
                             "\">" << convertToXML(cd->name()) << "</class>" << endl;
                  }
               }
            }
         }
         break;

         case LayoutDocEntry::FileNamespaces: 
         {
            if (m_namespaceSDict) {              
               for (auto nd : *m_namespaceSDict) {
                  if (nd->isLinkableInProject()) {
                     tagFile << "    <namespace>" << convertToXML(nd->name()) << "</namespace>" << endl;
                  }
               }
            }
         }
         break;

         case LayoutDocEntry::MemberDecl: 
         {
            LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
            MemberList *ml = getMemberList(lmd->type);

            if (ml) {
               ml->writeTagFile(tagFile);
            }
         }
         break;

         case LayoutDocEntry::MemberGroups: {
            if (m_memberGroupSDict) {
               for (auto mg : *m_memberGroupSDict) {
                  mg->writeTagFile(tagFile);
               }
            }
         }
         break;
         default:
            break;
      }
   }

   writeDocAnchorsToTagFile(tagFile);
   tagFile << "  </compound>" << endl;
}

void FileDef::writeDetailedDescription(OutputList &ol, const QByteArray &title)
{
   if (hasDetailedDescription()) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
      ol.writeRuler();
      ol.popGeneratorState();
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);
      ol.writeAnchor(0, "details");
      ol.popGeneratorState();
      ol.startGroupHeader();
      ol.parseText(title);
      ol.endGroupHeader();

      ol.startTextBlock();
      if (!briefDescription().isEmpty() && Config_getBool("REPEAT_BRIEF")) {
         ol.generateDoc(briefFile(), briefLine(), this, 0, briefDescription(), false, false);
      }
      if (!briefDescription().isEmpty() && Config_getBool("REPEAT_BRIEF") &&
            !documentation().isEmpty()) {
         ol.pushGeneratorState();
         ol.disable(OutputGenerator::Man);
         ol.disable(OutputGenerator::RTF);
         // ol.newParagraph(); // FIXME:PARA
         ol.enableAll();
         ol.disableAllBut(OutputGenerator::Man);
         ol.enable(OutputGenerator::Latex);
         ol.writeString("\n\n");
         ol.popGeneratorState();
      }

      if (!documentation().isEmpty()) {
         ol.generateDoc(docFile(), docLine(), this, 0, documentation() + "\n", true, false);
      }
      
      if (Config_getBool("SOURCE_BROWSER")) {
         //if Latex enabled and LATEX_SOURCE_CODE is not -> skip, bug_738548
         ol.pushGeneratorState();
         if (ol.isEnabled(OutputGenerator::Latex) && !Config_getBool("LATEX_SOURCE_CODE")) {
            ol.disable(OutputGenerator::Latex);
         }

         ol.startParagraph();
         QByteArray refText = theTranslator->trDefinedInSourceFile();
         int fileMarkerPos = refText.indexOf("@0");

         if (fileMarkerPos != -1) { // should always pass this.
            ol.parseText(refText.left(fileMarkerPos)); //text left from marker 1
            ol.writeObjectLink(0, getSourceFileBase(), 0, name());
            ol.parseText(refText.right( refText.length() - fileMarkerPos - 2)); // text right from marker 2
         }

         ol.endParagraph();
         //Restore settings, bug_738548
         ol.popGeneratorState();
      }
      ol.endTextBlock();
   }
}

void FileDef::writeBriefDescription(OutputList &ol)
{
   if (!briefDescription().isEmpty() && Config_getBool("BRIEF_MEMBER_DESC")) {
      DocRoot *rootNode = validatingParseDoc(briefFile(), briefLine(), this, 0,
                                             briefDescription(), true, false, 0, true, false);

      if (rootNode && !rootNode->isEmpty()) {
         ol.startParagraph();
         ol.writeDoc(rootNode, this, 0);
         ol.pushGeneratorState();
         ol.disable(OutputGenerator::RTF);
         ol.writeString(" \n");
         ol.enable(OutputGenerator::RTF);

         if (Config_getBool("REPEAT_BRIEF") ||
               !documentation().isEmpty()
            ) {
            ol.disableAllBut(OutputGenerator::Html);
            ol.startTextLink(0, "details");
            ol.parseText(theTranslator->trMore());
            ol.endTextLink();
         }
         ol.popGeneratorState();
         ol.endParagraph();
      }
      delete rootNode;
   }
   ol.writeSynopsis();
}

void FileDef::writeIncludeFiles(OutputList &ol)
{
   if (m_includeList && m_includeList->count() > 0) {
      ol.startTextBlock(true);
     
      for (auto ii : *m_includeList) {
         if (! ii.indirect) {
            FileDef *fd = ii.fileDef;
            bool isIDLorJava = false;

            if (fd) {
               SrcLangExt lang   = fd->getLanguage();
               isIDLorJava = lang == SrcLangExt_IDL || lang == SrcLangExt_Java;
            }

            ol.startTypewriter();

            if (isIDLorJava) { // IDL/Java include
               ol.docify("import ");

            } else if (ii.imported) { // Objective-C include
               ol.docify("#import ");

            } else { // C/C++ include
               ol.docify("#include ");
            }

            if (ii.local || isIDLorJava) {
               ol.docify("\"");
            } else {
               ol.docify("<");
            }

            ol.disable(OutputGenerator::Html);
            ol.docify(ii.includeName);
            ol.enableAll();
            ol.disableAllBut(OutputGenerator::Html);

            // Here we use the include file name as it appears in the file.
            // we could also we the name as it is used within doxygen,
            // then we should have used fd->docName() instead of ii->includeName
            if (fd && fd->isLinkable()) {
               ol.writeObjectLink(fd->getReference(), 
                                  fd->generateSourceFile() ? fd->includeName() : fd->getOutputFileBase(),
                                  0, ii.includeName);

            } else {
               ol.docify(ii.includeName);
            }

            ol.enableAll();
            if (ii.local || isIDLorJava) {
               ol.docify("\"");

            } else {
               ol.docify(">");
            }

            if (isIDLorJava) {
               ol.docify(";");
            }

            ol.endTypewriter();
            ol.lineBreak();
         }
      }
      ol.endTextBlock();
   }
}

void FileDef::writeIncludeGraph(OutputList &ol)
{
   if (Config_getBool("HAVE_DOT") /*&& Config_getBool("INCLUDE_GRAPH")*/) {
      //printf("Graph for file %s\n",name().data());

      DotInclDepGraph incDepGraph(this, false);

      if (incDepGraph.isTooBig()) {
         warn_uncond("Include graph for '%s' not generated, too many nodes. Consider increasing DOT_GRAPH_MAX_NODES.\n", name().data());

      } else if (!incDepGraph.isTrivial()) {
         ol.startTextBlock();
         ol.disable(OutputGenerator::Man);
         ol.startInclDepGraph();
         ol.parseText(theTranslator->trInclDepGraph(name()));
         ol.endInclDepGraph(incDepGraph);
         ol.enableAll();
         ol.endTextBlock(true);
      }
      //incDepGraph.writeGraph(Config_getString("HTML_OUTPUT"),fd->getOutputFileBase());
   }
}

void FileDef::writeIncludedByGraph(OutputList &ol)
{
   if (Config_getBool("HAVE_DOT") /*&& Config_getBool("INCLUDED_BY_GRAPH")*/) {
      //printf("Graph for file %s\n",name().data());
      DotInclDepGraph incDepGraph(this, true);
      if (incDepGraph.isTooBig()) {
         warn_uncond("Included by graph for '%s' not generated, too many nodes. Consider increasing DOT_GRAPH_MAX_NODES.\n", name().data());
      } else if (!incDepGraph.isTrivial()) {
         ol.startTextBlock();
         ol.disable(OutputGenerator::Man);
         ol.startInclDepGraph();
         ol.parseText(theTranslator->trInclByDepGraph());
         ol.endInclDepGraph(incDepGraph);
         ol.enableAll();
         ol.endTextBlock(true);
      }
      //incDepGraph.writeGraph(Config_getString("HTML_OUTPUT"),fd->getOutputFileBase());
   }
}


void FileDef::writeSourceLink(OutputList &ol)
{
   //printf("%s: generateSourceFile()=%d\n",name().data(),generateSourceFile());
   if (generateSourceFile()) {
      ol.disableAllBut(OutputGenerator::Html);
      ol.startParagraph();
      ol.startTextLink(includeName(), 0);
      ol.parseText(theTranslator->trGotoSourceCode());
      ol.endTextLink();
      ol.endParagraph();
      ol.enableAll();
   }
}

void FileDef::writeNamespaceDeclarations(OutputList &ol, const QByteArray &title,
      bool const isConstantGroup)
{
   // write list of namespaces
   if (m_namespaceSDict) {
      m_namespaceSDict->writeDeclaration(ol, title, isConstantGroup);
   }
}

void FileDef::writeClassDeclarations(OutputList &ol, const QByteArray &title)
{
   // write list of classes
   if (m_classSDict) {
      m_classSDict->writeDeclaration(ol, 0, title, false);
   }
}

void FileDef::writeInlineClasses(OutputList &ol)
{
   // temporarily undo the disbling could be done by startMemberDocumentation()
   // as a result of setting SEPARATE_MEMBER_PAGES to YES; see bug730512
   bool isEnabled = ol.isEnabled(OutputGenerator::Html);
   ol.enable(OutputGenerator::Html);

   if (m_classSDict) {
      m_classSDict->writeDocumentation(ol, this);
   }

   // restore the initial state if needed
   if (!isEnabled) {
      ol.disable(OutputGenerator::Html);
   }
}

void FileDef::startMemberDeclarations(OutputList &ol)
{
   ol.startMemberSections();
}

void FileDef::endMemberDeclarations(OutputList &ol)
{
   ol.endMemberSections();
}

void FileDef::startMemberDocumentation(OutputList &ol)
{
   if (Config_getBool("SEPARATE_MEMBER_PAGES")) {
      ol.disable(OutputGenerator::Html);
      Doxygen::suppressDocWarnings = true;
   }
}

void FileDef::endMemberDocumentation(OutputList &ol)
{
   if (Config_getBool("SEPARATE_MEMBER_PAGES")) {
      ol.enable(OutputGenerator::Html);
      Doxygen::suppressDocWarnings = false;
   }
}

void FileDef::writeMemberGroups(OutputList &ol)
{
   /* write user defined member groups */
   if (m_memberGroupSDict) {
     
      for (auto mg : *m_memberGroupSDict) {
         if ((!mg->allMembersInSameSection() || !m_subGrouping) && mg->header() != "[NOHEADER]") {
            mg->writeDeclarations(ol, 0, 0, this, 0);
         }
      }
   }
}

void FileDef::writeAuthorSection(OutputList &ol)
{
   // write Author section (Man only)
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Man);
   ol.startGroupHeader();
   ol.parseText(theTranslator->trAuthor(true, true));
   ol.endGroupHeader();
   ol.parseText(theTranslator->trGeneratedAutomatically(Config_getString("PROJECT_NAME")));
   ol.popGeneratorState();
}

void FileDef::writeSummaryLinks(OutputList &ol)
{
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
  
   bool first = true;
   SrcLangExt lang = getLanguage();
  
   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::File)) {

      if ((lde->kind() == LayoutDocEntry::FileClasses && m_classSDict && m_classSDict->declVisible()) ||
            (lde->kind() == LayoutDocEntry::FileNamespaces && m_namespaceSDict && m_namespaceSDict->declVisible()) ) {

         LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
         QByteArray label = lde->kind() == LayoutDocEntry::FileClasses ? "nested-classes" : "namespaces";
         ol.writeSummaryLink(QString(""), label, ls->title(lang), first);
         first = false;

      } else if (lde->kind() == LayoutDocEntry::MemberDecl) {
         LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
         MemberList *ml = getMemberList(lmd->type);

         if (ml && ml->declVisible()) {
            ol.writeSummaryLink(QString(""), MemberList::listTypeAsString(ml->listType()), lmd->title(lang), first);
            first = false;
         }
      }
   }

   if (!first) {
      ol.writeString("  </div>\n");
   }
   ol.popGeneratorState();
}

/*! Write the documentation page for this file to the file of output 
    generators \a ol.
*/
void FileDef::writeDocumentation(OutputList &ol)
{
   static bool generateTreeView = Config_getBool("GENERATE_TREEVIEW");

   QByteArray versionTitle;

   if (! m_fileVersion.isEmpty()) {
      versionTitle = ("(" + m_fileVersion + ")");
   }

   QByteArray title = m_docname + versionTitle;
   QByteArray pageTitle = theTranslator->trFileReference(m_docname);

   if (getDirDef()) {
      startFile(ol, getOutputFileBase(), name(), pageTitle, HLI_FileVisible, !generateTreeView);

      if (! generateTreeView) {
         getDirDef()->writeNavigationPath(ol);
         ol.endQuickIndices();
      }

      QByteArray pageTitleShort = theTranslator->trFileReference(name());
      startTitle(ol, getOutputFileBase(), this);
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);
      ol.parseText(pageTitleShort); // Html only
      ol.enableAll();
      ol.disable(OutputGenerator::Html);
      ol.parseText(pageTitle); // other output formats
      ol.popGeneratorState();
      addGroupListToTitle(ol, this);
      endTitle(ol, getOutputFileBase(), title);

   } else {
      startFile(ol, getOutputFileBase(), name(), pageTitle, HLI_FileVisible, !generateTreeView);

      if (!generateTreeView) {
         ol.endQuickIndices();
      }

      startTitle(ol, getOutputFileBase(), this);
      ol.parseText(pageTitle);
      addGroupListToTitle(ol, this);
      endTitle(ol, getOutputFileBase(), title);
   }

   ol.startContents();

   if (!m_fileVersion.isEmpty()) {
      ol.disableAllBut(OutputGenerator::Html);
      ol.startProjectNumber();
      ol.docify(versionTitle);
      ol.endProjectNumber();
      ol.enableAll();
   }

   if (Doxygen::searchIndex) {
      Doxygen::searchIndex->setCurrentDoc(this, anchor(), false);
      Doxygen::searchIndex->addWord(localName(), true);
   }


   //---------------------------------------- start flexible part -------------------------------

   SrcLangExt lang = getLanguage();    

   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::File)) {
      switch (lde->kind()) {
         case LayoutDocEntry::BriefDesc:
            writeBriefDescription(ol);
            break;
         case LayoutDocEntry::MemberDeclStart:
            startMemberDeclarations(ol);
            break;
         case LayoutDocEntry::FileIncludes:
            writeIncludeFiles(ol);
            break;
         case LayoutDocEntry::FileIncludeGraph:
            writeIncludeGraph(ol);
            break;
         case LayoutDocEntry::FileIncludedByGraph:
            writeIncludedByGraph(ol);
            break;
         case LayoutDocEntry::FileSourceLink:
            writeSourceLink(ol);
            break;
         case LayoutDocEntry::FileClasses: {
            LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
            writeClassDeclarations(ol, ls->title(lang));
         }
         break;
         case LayoutDocEntry::FileNamespaces: {
            LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
            writeNamespaceDeclarations(ol, ls->title(lang), false);
         }
         break;
         case LayoutDocEntry::FileConstantGroups: {
            LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
            writeNamespaceDeclarations(ol, ls->title(lang), true);
         }
         break;
         case LayoutDocEntry::MemberGroups:
            writeMemberGroups(ol);
            break;
         case LayoutDocEntry::MemberDecl: {
            LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
            writeMemberDeclarations(ol, lmd->type, lmd->title(lang));
         }
         break;
         case LayoutDocEntry::MemberDeclEnd:
            endMemberDeclarations(ol);
            break;
         case LayoutDocEntry::DetailedDesc: {
            LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
            writeDetailedDescription(ol, ls->title(lang));
         }
         break;
         case LayoutDocEntry::MemberDefStart:
            startMemberDocumentation(ol);
            break;
         case LayoutDocEntry::FileInlineClasses:
            writeInlineClasses(ol);
            break;
         case LayoutDocEntry::MemberDef: {
            LayoutDocEntryMemberDef *lmd = (LayoutDocEntryMemberDef *)lde;
            writeMemberDocumentation(ol, lmd->type, lmd->title(lang));
         }
         break;
         case LayoutDocEntry::MemberDefEnd:
            endMemberDocumentation(ol);
            break;
         case LayoutDocEntry::AuthorSection:
            writeAuthorSection(ol);
            break;
         case LayoutDocEntry::ClassIncludes:
         case LayoutDocEntry::ClassInheritanceGraph:
         case LayoutDocEntry::ClassNestedClasses:
         case LayoutDocEntry::ClassCollaborationGraph:
         case LayoutDocEntry::ClassAllMembersLink:
         case LayoutDocEntry::ClassUsedFiles:
         case LayoutDocEntry::ClassInlineClasses:
         case LayoutDocEntry::NamespaceNestedNamespaces:
         case LayoutDocEntry::NamespaceNestedConstantGroups:
         case LayoutDocEntry::NamespaceClasses:
         case LayoutDocEntry::NamespaceInlineClasses:
         case LayoutDocEntry::GroupClasses:
         case LayoutDocEntry::GroupInlineClasses:
         case LayoutDocEntry::GroupNamespaces:
         case LayoutDocEntry::GroupDirs:
         case LayoutDocEntry::GroupNestedGroups:
         case LayoutDocEntry::GroupFiles:
         case LayoutDocEntry::GroupGraph:
         case LayoutDocEntry::GroupPageDocs:
         case LayoutDocEntry::DirSubDirs:
         case LayoutDocEntry::DirFiles:
         case LayoutDocEntry::DirGraph:
            err("Internal inconsistency: member %d should not be part of "
                "LayoutDocManager::File entry list\n", lde->kind());
            break;
      }
   }

   //---------------------------------------- end flexible part -------------------------------
   ol.endContents();

   endFileWithNavPath(this, ol);

   if (Config_getBool("SEPARATE_MEMBER_PAGES")) {
      MemberList *ml = getMemberList(MemberListType_allMembersList);

      if (ml) {
         ml->sort();
      }
      writeMemberPages(ol);
   }
}

void FileDef::writeMemberPages(OutputList &ol)
{
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
  
   for (auto ml : m_memberLists) {
      if (ml->listType()&MemberListType_documentationLists) {
         ml->writeDocumentationPage(ol, name(), this);
      }
   }

   ol.popGeneratorState();
}

void FileDef::writeQuickMemberLinks(OutputList &ol, MemberDef *currentMd) const
{
   static bool createSubDirs = Config_getBool("CREATE_SUBDIRS");

   ol.writeString("      <div class=\"navtab\">\n");
   ol.writeString("        <table>\n");

   MemberList *allMemberList = getMemberList(MemberListType_allMembersList);

   if (allMemberList) {
      
      for (auto md : *allMemberList) {

         if (md->getFileDef() == this && md->getNamespaceDef() == 0 && md->isLinkable() && !md->isEnumValue()) {
            ol.writeString("          <tr><td class=\"navtab\">");
            if (md->isLinkableInProject()) {
               if (md == currentMd) { // selected item => highlight
                  ol.writeString("<a class=\"qindexHL\" ");
               } else {
                  ol.writeString("<a class=\"qindex\" ");
               }
               ol.writeString("href=\"");
               if (createSubDirs) {
                  ol.writeString("../../");
               }
               ol.writeString(md->getOutputFileBase() + Doxygen::htmlFileExtension + "#" + md->anchor());
               ol.writeString("\">");
               ol.writeString(convertToHtml(md->localName()));
               ol.writeString("</a>");
            }
            ol.writeString("</td></tr>\n");
         }
      }
   }

   ol.writeString("        </table>\n");
   ol.writeString("      </div>\n");
}

/*! Write a source listing of this file to the output */
void FileDef::writeSource(OutputList &ol, bool sameTu, QStringList &filesInSameTu)
{
   static bool generateTreeView  = Config_getBool("GENERATE_TREEVIEW");
   static bool filterSourceFiles = Config_getBool("FILTER_SOURCE_FILES");
   static bool latexSourceCode   = Config_getBool("LATEX_SOURCE_CODE");

   DevNullCodeDocInterface devNullIntf;
   QByteArray title = m_docname;

   if (!m_fileVersion.isEmpty()) {
      title += (" (" + m_fileVersion + ")");
   }

   QByteArray pageTitle = theTranslator->trSourceFile(title);
   ol.disable(OutputGenerator::Man);
   ol.disable(OutputGenerator::RTF);

   if (!latexSourceCode) {
      ol.disable(OutputGenerator::Latex);
   }

   bool isDocFile = isDocumentationFile();
   bool genSourceFile = !isDocFile && generateSourceFile();

   if (getDirDef()) {

      startFile(ol, getSourceFileBase(), 0, pageTitle, HLI_FileVisible,
                !generateTreeView, !isDocFile && genSourceFile ? QByteArray() : getOutputFileBase());

      if (! generateTreeView) {
         getDirDef()->writeNavigationPath(ol);
         ol.endQuickIndices();
      }
      startTitle(ol, getSourceFileBase());
      ol.parseText(name());
      endTitle(ol, getSourceFileBase(), title);
   } else {
      startFile(ol, getSourceFileBase(), 0, pageTitle, HLI_FileVisible, false,
                !isDocFile && genSourceFile ? QByteArray() : getOutputFileBase());
      startTitle(ol, getSourceFileBase());
      ol.parseText(title);
      endTitle(ol, getSourceFileBase(), 0);
   }

   ol.startContents();

   if (isLinkable()) {
      if (latexSourceCode) {
         ol.disable(OutputGenerator::Latex);
      }
      ol.startTextLink(getOutputFileBase(), 0);
      ol.parseText(theTranslator->trGotoDocumentation());
      ol.endTextLink();
      if (latexSourceCode) {
         ol.enable(OutputGenerator::Latex);
      }
   }

   (void)sameTu;
   (void)filesInSameTu;

#if USE_LIBCLANG
   static bool clangAssistedParsing = Config_getBool("CLANG_ASSISTED_PARSING");

   if (clangAssistedParsing && (getLanguage() == SrcLangExt_Cpp || getLanguage() == SrcLangExt_ObjC)) {

      ol.startCodeFragment();

      if (!sameTu) {
         ClangParser::instance()->start(absoluteFilePath(), filesInSameTu);
      } else {
         ClangParser::instance()->switchToFile(getFilePath());
      }

      ClangParser::instance()->writeSources(ol, this);
      ol.endCodeFragment();
   } else
#endif
   {
      ParserInterface *pIntf = Doxygen::parserManager->getParser(getDefFileExtension());
      pIntf->resetCodeParserState();
      ol.startCodeFragment();

      bool needs2PassParsing = 
         Doxygen::parseSourcesNeeded &&                // we need to parse (filtered) sources for cross-references
         !filterSourceFiles &&                         // but user wants to show sources as-is
         !getFileFilter(getFilePath(), true).isEmpty(); // and there is a filter used while parsing

      if (needs2PassParsing) {
         // parse code for cross-references only (see bug707641)
         pIntf->parseCode(devNullIntf, 0, fileToString(getFilePath(), true, true), 
                          getLanguage(),  false, 0, this );
      }
      pIntf->parseCode(ol, 0,
                       fileToString(getFilePath(), filterSourceFiles, true),
                       getLanguage(),      // lang
                       false,              // isExampleBlock
                       0,                  // exampleName
                       this,               // fileDef
                       -1,                 // startLine
                       -1,                 // endLine
                       false,              // inlineFragment
                       0,                  // memberDef
                       true,               // showLineNumbers
                       0,                  // searchCtx
                       !needs2PassParsing  // collectXRefs
                      );
      ol.endCodeFragment();
   }
   ol.endContents();
   endFileWithNavPath(this, ol);
   ol.enableAll();
}

void FileDef::parseSource(bool sameTu, QStringList &filesInSameTu)
{
   static bool filterSourceFiles = Config_getBool("FILTER_SOURCE_FILES");
   DevNullCodeDocInterface devNullIntf;
   (void)sameTu;
   (void)filesInSameTu;

#if USE_LIBCLANG
   static bool clangAssistedParsing = Config_getBool("CLANG_ASSISTED_PARSING");

   if (clangAssistedParsing && (getLanguage() == SrcLangExt_Cpp || getLanguage() == SrcLangExt_ObjC)) {

      if (!sameTu) {
         ClangParser::instance()->start(getFilePath(), filesInSameTu);
      } else {
         ClangParser::instance()->switchToFile(getFilePath());
      }

      ClangParser::instance()->writeSources(devNullIntf, this);

   } else

#endif

   {
      ParserInterface *pIntf = Doxygen::parserManager->getParser(getDefFileExtension());
      pIntf->resetCodeParserState();
      pIntf->parseCode(devNullIntf, 0, fileToString(getFilePath(), filterSourceFiles, true), getLanguage(), false, 0, this);
   }
}

void FileDef::startParsing()
{
}

void FileDef::finishParsing()
{
   ClangParser::instance()->finish();
}

void FileDef::addMembersToMemberGroup()
{ 
   for (auto &ml : m_memberLists) {
      if (ml->listType() & MemberListType_declarationLists) {
         ::addMembersToMemberGroup(ml.data(), &m_memberGroupSDict, this);
      }
   }

   // add members inside sections to their groups
   if (m_memberGroupSDict) {
     for (auto mg : *m_memberGroupSDict) {
         if (mg->allMembersInSameSection() && m_subGrouping) {
            //printf("----> addToDeclarationSection(%s)\n",mg->header().data());
            mg->addToDeclarationSection();
         }
      }
   }
}

/*! Adds member definition \a md to the list of all members of this file */
void FileDef::insertMember(MemberDef *md)
{
   if (md->isHidden()) {
      return;
   }

   MemberList *allMemberList = getMemberList(MemberListType_allMembersList);

   if (allMemberList && allMemberList->lastIndexOf(md) != -1) { 
      return;
   }

   if (allMemberList == nullptr) {   
      m_memberLists.append(QSharedPointer<MemberList>(new MemberList(MemberListType_allMembersList)));
      allMemberList = m_memberLists.last().data();
   }

   allMemberList->append(md);
  
   switch (md->memberType()) {
      case MemberType_Variable:
      case MemberType_Property:
         addMemberToList(MemberListType_decVarMembers, md);
         addMemberToList(MemberListType_docVarMembers, md);
         break;
      case MemberType_Function:
         addMemberToList(MemberListType_decFuncMembers, md);
         addMemberToList(MemberListType_docFuncMembers, md);
         break;
      case MemberType_Typedef:
         addMemberToList(MemberListType_decTypedefMembers, md);
         addMemberToList(MemberListType_docTypedefMembers, md);
         break;
      case MemberType_Enumeration:
         addMemberToList(MemberListType_decEnumMembers, md);
         addMemberToList(MemberListType_docEnumMembers, md);
         break;
      case MemberType_EnumValue:    // enum values are shown inside their enums
         break;
      case MemberType_Define:
         addMemberToList(MemberListType_decDefineMembers, md);
         addMemberToList(MemberListType_docDefineMembers, md);
         break;
      default:
         err("FileDef::insertMembers(): "
             "member `%s' with class scope `%s' inserted in file scope `%s'!\n",
             md->name().data(),
             md->getClassDef() ? md->getClassDef()->name().data() : "<global>",
             name().data());
   }
   //addMemberToGroup(md,groupId);
}

/*! Adds compound definition \a cd to the list of all compounds of this file */
void FileDef::insertClass(QSharedPointer<ClassDef> cd)
{
   if (cd->isHidden()) {
      return;
   }

   if (m_classSDict == 0) {
      m_classSDict = new ClassSDict;
   }
 
   m_classSDict->insert(cd->name(), cd);
   
}

/*! Adds namespace definition \a nd to the list of all compounds of this file */
void FileDef::insertNamespace(QSharedPointer<NamespaceDef> nd)
{
   if (nd->isHidden()) {
      return;
   }

   if (!nd->name().isEmpty() && (m_namespaceSDict == 0 || m_namespaceSDict->find(nd->name()) == 0)) {

      if (m_namespaceSDict == 0) {
         m_namespaceSDict = new NamespaceSDict;
      }
     
      m_namespaceSDict->insert(nd->name(), nd);      
   }
}

QByteArray FileDef::name() const
{
   if (Config_getBool("FULL_PATH_NAMES")) {
      return m_fileName;
   } else {
      return Definition::name();
   }
}

void FileDef::addSourceRef(int line, QSharedPointer<Definition> d, QSharedPointer<MemberDef> md)
{   
   if (d) {
      if (m_srcDefDict == nullptr) {
         m_srcDefDict = new QHash<long, QSharedPointer<Definition>>();
      }

      if (m_srcMemberDict == nullptr) {
         m_srcMemberDict = new QHash<long, QSharedPointer<MemberDef>>();
      }

      m_srcDefDict->insert(line, d);

      if (md) {
         m_srcMemberDict->insert(line, md);
      }
      
   }
}

QSharedPointer<Definition> FileDef::getSourceDefinition(int lineNr) const
{
   QSharedPointer<Definition> result;

   if (m_srcDefDict) {
      result = m_srcDefDict->value(lineNr);
   }
 
   return result;
}

QSharedPointer<MemberDef> FileDef::getSourceMember(int lineNr) const
{
   QSharedPointer<MemberDef> result;

   if (m_srcMemberDict) {
      result = m_srcMemberDict->value(lineNr);
   }
  
   return result;
}

void FileDef::addUsingDirective(QSharedPointer<NamespaceDef> nd)
{
   if (m_usingDirList == 0) {
      m_usingDirList = new NamespaceSDict;
   }

   if (! m_usingDirList->find(nd->qualifiedName()) ) {
      m_usingDirList->insert(nd->qualifiedName(), nd);
   }   
}

NamespaceSDict *FileDef::getUsedNamespaces() const
{
   return m_usingDirList;
}

void FileDef::addUsingDeclaration(QSharedPointer<Definition> d)
{
   if (m_usingDeclList == 0) {
      m_usingDeclList = new StringMap<QSharedPointer<Definition>>();
   }

   if (! m_usingDeclList->find(d->qualifiedName()) ) {
      m_usingDeclList->insert(d->qualifiedName(), d);
   }
}

void FileDef::addIncludeDependency(FileDef *fd, const char *incName, bool local, bool imported, bool indirect)
{
   QByteArray iName;

   if (fd) {
      iName = fd->getFilePath().data();

   } else {
      iName = incName;
   }

   if (! iName.isEmpty() && (! m_includeDict || ! m_includeDict->contains(iName))) {

      if (m_includeDict == 0) {
         m_includeDict   = new QHash<QString, IncludeInfo>;
         m_includeList   = new QList<IncludeInfo>;        
      }

      IncludeInfo ii;
      ii.fileDef     = fd;
      ii.includeName = incName;
      ii.local       = local;
      ii.imported    = imported;
      ii.indirect    = indirect;

      m_includeList->append(ii);
      m_includeDict->insert(iName, ii);
   }
}

void FileDef::addIncludedUsingDirectives()
{
   if (visited) {
      return;
   }

   visited = true;

   NamespaceList nl;

   if (m_includeList) { 
      // file contains #includes

      {         
         for (auto &item : *m_includeList) {

            // foreach #include which is a known file
            if (item.fileDef && ! item.fileDef->visited) {                
               // recurse into this file
               item.fileDef->addIncludedUsingDirectives();
            }
         }
      }

      {      
         for (auto ii : *m_includeList) {

            if (ii.fileDef && ii.fileDef != this) {
               // add using directives
               NamespaceSDict *unl = ii.fileDef->m_usingDirList;

               if (unl) {
                
                  for (auto nd : *unl) {
                     // append each using directive found in a #include file

                     if (m_usingDirList == 0) {
                        m_usingDirList = new NamespaceSDict;
                     }
                   
                     if (m_usingDirList->find(nd->qualifiedName()) == 0) { 
                        // not yet added
                        m_usingDirList->insert(nd->qualifiedName(), nd);
                     }
                  }
               }

               // add using declarations
               StringMap<QSharedPointer<Definition>> *udl = ii.fileDef->m_usingDeclList;

               if (udl) {                  
                  for (auto d : *udl) {
              
                     if (m_usingDeclList == 0) {
                        m_usingDeclList = new StringMap<QSharedPointer<Definition>>();
                     }

                     if (m_usingDeclList->find(d->qualifiedName()) == 0) {
                        m_usingDeclList->insert(d->qualifiedName(), d);
                     }

                  }
               }
            }
         }
      }
   }   
}

void FileDef::addIncludedByDependency(FileDef *fd, const char *incName, bool local, bool imported)
{
   QByteArray iName = fd ? fd->getFilePath().data() : incName;

   if (!iName.isEmpty() && (m_includedByDict == 0 || ! m_includedByDict->contains(iName))) {

      if (m_includedByDict == 0) {
         m_includedByDict = new QHash<QString, IncludeInfo>;
         m_includedByList = new QList<IncludeInfo>;        
      }

      IncludeInfo ii;
      ii.fileDef     = fd;
      ii.includeName = incName;
      ii.local       = local;
      ii.imported    = imported;
      ii.indirect    = false;

      m_includedByList->append(ii);
      m_includedByDict->insert(iName, ii);
   }
}

bool FileDef::isIncluded(const QByteArray &name) const
{
   if (name.isEmpty()) {
      return false;
   }

   return m_includeDict != 0 && ! m_includeDict->contains(name);
}

bool FileDef::generateSourceFile() const
{
   static bool sourceBrowser = Config_getBool("SOURCE_BROWSER");
   static bool verbatimHeaders = Config_getBool("VERBATIM_HEADERS");

   QByteArray extension = name().right(4);

   return ! isReference() && (sourceBrowser || (verbatimHeaders && guessSection(name()) == Entry::HEADER_SEC) ) &&
          extension != ".doc" && extension != ".txt" && extension != ".dox" && extension != ".md" && name().right(9) != ".markdown";
}


void FileDef::addListReferences()
{
   {
      QList<ListItemInfo> *xrefItems = xrefListItems();
      addRefItem(xrefItems, getOutputFileBase(), theTranslator->trFile(true, true),
                 getOutputFileBase(), name(), 0, 0 );
   }

   if (m_memberGroupSDict) {
     for (auto mg : *m_memberGroupSDict) {
         mg->addListReferences(this);
      }
   }
   
   for (auto &ml : m_memberLists) {
      if (ml->listType() & MemberListType_documentationLists) {
         ml->addListReferences(this);
      }
   }
}

static int findMatchingPart(const QByteArray &path, const QByteArray dir)
{
   int si1;
   int pos1 = 0, pos2 = 0;

   while ((si1 = path.indexOf('/', pos1)) != -1) {
      int si2 = dir.indexOf('/', pos2);
     
      if (si2 == -1 && path.mid(pos1, si1 - pos2) == dir.mid(pos2)) { 
         // match at end
         return dir.length();
      }

      if (si1 != si2 || path.mid(pos1, si1 - pos2) != dir.mid(pos2, si2 - pos2)) { 
         // no match in middle
         return qMax(pos1 - 1, 0);
      }

      pos1 = si1 + 1;
      pos2 = si2 + 1;
   }

   return 0;
}

static Directory *findDirNode(Directory *root, const QByteArray &name)
{  
   for (auto de : root->children()) {

      if (de->kind() == DirEntry::Dir) {
         Directory *dir = (Directory *)de;

         QByteArray dirName = dir->name();
         int sp = findMatchingPart(name, dirName);

         if (sp > 0) { 
            // match found

            if ((uint)sp == dirName.length()) { // whole directory matches
               // recurse into the directory
               return findDirNode(dir, name.mid(dirName.length() + 1));

            } else { // partial match => we need to split the path into three parts
               QByteArray baseName     = dirName.left(sp);
               QByteArray oldBranchName = dirName.mid(sp + 1);
               QByteArray newBranchName = name.mid(sp + 1);

               // strip file name from path
               int newIndex = newBranchName.lastIndexOf('/');
               if (newIndex > 0) {
                  newBranchName = newBranchName.left(newIndex);
               }
              
               Directory *base = new Directory(root, baseName);
               Directory *newBranch = new Directory(base, newBranchName);

               dir->reParent(base);
               dir->rename(oldBranchName);
               base->addChild(dir);
               base->addChild(newBranch);
               dir->setLast(false);

               // remove DirEntry container from list (without deleting it)               
               root->children().removeOne(dir);
              
               // add new branch to the root
               if (!root->children().isEmpty()) {
                  root->children().last()->setLast(false);
               }

               root->addChild(base);
               return newBranch;
            }
         }
      }
   }

   int si = name.lastIndexOf('/');
   if (si == -1) { // no subdir
      return root; // put the file under the root node.

   } else { // need to create a subdir
      QByteArray baseName = name.left(si);

      Directory *newBranch = new Directory(root, baseName);
      if (!root->children().isEmpty()) {
         root->children().last()->setLast(false);
      }

      root->addChild(newBranch);
      return newBranch;
   }
}

static void mergeFileDef(Directory *root, FileDef *fd)
{
   QByteArray rootPath = root->name();
   QByteArray filePath = fd->getFilePath();
 
   Directory *dirNode = findDirNode(root, filePath);

   if (!dirNode->children().isEmpty()) {
      dirNode->children().last()->setLast(false);
   }

   DirEntry *e = new DirEntry(dirNode, fd);
   dirNode->addChild(e);
}

static void addDirsAsGroups(Directory *root, GroupDef *parent, int level)
{
   QSharedPointer<GroupDef> gd;

   if (root->kind() == DirEntry::Dir) {
      gd = QSharedPointer<GroupDef>(new GroupDef("[generated]", 1, root->path(), root->name() ));

      if (parent) {
         parent->addGroup(gd.data());
         gd->makePartOfGroup(parent);

      } else {
         Doxygen::groupSDict->insert(root->path(), gd);
      }
   }
  
   for (auto de : root->children()) {
      if (de->kind() == DirEntry::Dir) {
         addDirsAsGroups((Directory *)de, gd.data(), level + 1);
      }
   }
}

void generateFileTree()
{
   Directory *root = new Directory(0, "root");
   root->setLast(true);
  
   for (auto fn : *Doxygen::inputNameList ) {    
      for (auto fd :*fn) { 
         mergeFileDef(root, fd);
      }
   }
 
   addDirsAsGroups(root, 0, 0);

   delete root;
}

void FileDef::combineUsingRelations()
{
   if (visited) {
      return;   // already done
   }

   visited = true;
   if (m_usingDirList) {
     
      for (auto nd : *m_usingDirList) {
         nd->combineUsingRelations();
      }

      for (auto nd : *m_usingDirList) {
         // add used namespaces of namespace nd to this namespace
             
         for (auto und : nd->getUsedNamespaces()) {  
            addUsingDirective(und);
         }
        
         // add used classes of namespace nd to this namespace                 
         for (auto ucd : nd->getUsedClasses()) {
            addUsingDeclaration(ucd);
         }         
      }
   }
}

bool FileDef::isDocumentationFile() const
{
   return name().right(4) == ".doc" ||
          name().right(4) == ".txt" ||
          name().right(4) == ".dox" ||
          name().right(3) == ".md"  ||
          name().right(9) == ".markdown";
}

void FileDef::acquireFileVersion()
{
   QByteArray vercmd = Config_getString("FILE_VERSION_FILTER");

   if (!vercmd.isEmpty() && !m_filePath.isEmpty() && m_filePath != "generated") {
      msg("Version of %s : ", m_filePath.data());

      QByteArray cmd = vercmd + " \"" + m_filePath + "\"";
      Debug::print(Debug::ExtCmd, 0, "Executing popen(`%s`)\n", cmd.data());

      FILE *f = portable_popen(cmd, "r");

      if (!f) {
         err("could not execute %s\n", vercmd.data());
         return;
      }

      const int bufSize = 1024;
      char buf[bufSize];
      int numRead = (int)fread(buf, 1, bufSize - 1, f);
      portable_pclose(f);

      if (numRead > 0 && numRead < bufSize) {
         buf[numRead] = '\0';
         m_fileVersion = QByteArray(buf, numRead).trimmed();

         if (!m_fileVersion.isEmpty()) {
            msg("%s\n", m_fileVersion.data());
            return;
         }
      }

      msg("no version available\n");
   }
}

QByteArray FileDef::getSourceFileBase() const
{
   if (Htags::useHtags) {
      return Htags::path2URL(m_filePath);
   } else {
      return (convertNameToFile(m_diskName) + "_source").toUtf8();
   }
}

/*! Returns the name of the verbatim copy of this file (if any). */
QByteArray FileDef::includeName() const
{
   return getSourceFileBase();
}

QSharedPointer<MemberList> FileDef::createMemberList(MemberListType lt)
{     
   for (auto ml : m_memberLists) {
      if (ml->listType() == lt) {
         return ml;
      }
   }

   // not found, create a new member list   
   m_memberLists.append(QSharedPointer<MemberList>(new MemberList(lt)));
   
   return m_memberLists.last();
}

void FileDef::addMemberToList(MemberListType lt, MemberDef *md)
{
   QSharedPointer<MemberList> ml = createMemberList(lt);  
   ml->append(md);

   if (lt & MemberListType_documentationLists) {
      ml->setInFile(true);
   }

   if (ml->listType() & MemberListType_declarationLists) {
      md->setSectionList(this, ml);
   }
}

MemberList *FileDef::getMemberList(MemberListType lt) const
{
   for (auto &ml : m_memberLists) {
      if (ml->listType() == lt) {
         return const_cast<MemberList *>(ml.data());
      }
   }

   return nullptr;
}

void FileDef::writeMemberDeclarations(OutputList &ol, MemberListType lt, const QByteArray &title)
{ 
   MemberList *ml = getMemberList(lt);

   if (ml) {     
      ml->writeDeclarations(ol, 0, 0, this, 0, title, 0);      
   }
}

void FileDef::writeMemberDocumentation(OutputList &ol, MemberListType lt, const QByteArray &title)
{
   MemberList *ml = getMemberList(lt);
   if (ml) {
      ml->writeDocumentation(ol, name(), this, title);
   }
}

bool FileDef::isLinkableInProject() const
{
   static bool showFiles = Config_getBool("SHOW_FILES");
   return hasDocumentation() && ! isReference() && showFiles;
}

static void getAllIncludeFilesRecursively(QHash<QString, void *> *filesVisited, const FileDef *fd, QStringList &incFiles)
{
   if (fd->includeFileList()) {

      for (auto item : *fd->includeFileList() ) {   
         if (item.fileDef && ! item.fileDef->isReference() && ! filesVisited->contains(item.fileDef->getFilePath())) {

            incFiles.append(item.fileDef->getFilePath());
            filesVisited->insert(item.fileDef->getFilePath(), (void *)0x8);

            getAllIncludeFilesRecursively(filesVisited, item.fileDef, incFiles);
         }
      }
   }
}

void FileDef::getAllIncludeFilesRecursively(QStringList &incFiles) const
{
   QHash<QString, void *> includes;
   ::getAllIncludeFilesRecursively(&includes, this, incFiles);
}

QByteArray FileDef::title() const
{
   return theTranslator->trFileReference(name());
}

QByteArray FileDef::fileVersion() const
{
   return m_fileVersion;
}
