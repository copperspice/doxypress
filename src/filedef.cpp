/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch. 
 * Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim 
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

#include <filedef.h>

#include <config.h>
#include <docparser.h>
#include <doxy_globals.h>
#include <dot.h>
#include <entry.h>
#include <groupdef.h>
#include <htags.h>
#include <layout.h>
#include <language.h>
#include <message.h>
#include <outputlist.h>
#include <parse_base.h>
#include <parse_clang.h>
#include <util.h>

/** Class implementing CodeOutputInterface by throwing away everything. */
class DevNullCodeDocInterface : public CodeOutputInterface
{
 public:
   void codify(const QString &) override {}

   void writeCodeLink(const QString &, const QString &, const QString &, 
                  const QString &, const QString &) override  {}

   void writeTooltip(const QString &, const DocLinkInfo &, const QString &,
                  const QString &, const SourceLinkInfo &, const SourceLinkInfo & ) override {}

   void writeLineNumber(const QString &, const QString &, const QString &, int)  override {}
   void startCodeLine(bool) override {}
   void endCodeLine() override {}
   void startFontClass(const QString &) override {}
   void endFontClass() override {}
   void writeCodeAnchor(const QString &) override {}
   void linkableSymbol(int, QString &, QSharedPointer<Definition>(), QSharedPointer<Definition>()) {}
   void setCurrentDoc(QSharedPointer<Definition> d, const QString &, bool) override {}
   void addWord(const QString &, bool) override {}
};

/*! create a new file definition, where \a p is the file path,
    \a nm the file name, and \a lref is an HTML anchor name if the
    file was read from a tag file or 0 otherwise
*/
FileDef::FileDef(const QString &p, const QString &nm, const QString &lref, const QString &dn)
   : Definition(p + nm, 1, 1, nm)
{
   static const bool fullPathNames = Config::getBool("full-path-names");

   m_path     = p;
   m_filePath = m_path + nm;
   m_fileName = nm;
   m_diskName = dn;

   if (m_diskName.isEmpty()) {
      m_diskName = nm;
   }

   setReference(lref);
      
   m_package   = QSharedPointer<PackageDef>();
   m_isSource  = determineSection(nm) == Entry::SOURCE_SEC;
   m_docname   = nm;
   m_dir       = QSharedPointer<DirDef>();

   if (fullPathNames) {
      m_docname.prepend(stripFromPath(m_path));
   }

   setLanguage(getLanguageFromFileName(name()));
   
   acquireFileVersion();
   m_subGrouping = Config::getBool("allow-sub-grouping");
}

/*! destroy the file definition */
FileDef::~FileDef()
{       
}

/*! Compute the HTML anchor names for all members in the class */
void FileDef::computeAnchors()
{
   QSharedPointer<MemberList> ml = getMemberList(MemberListType_allMembersList);

   if (ml) {
      setAnchors(ml);
   }
}

void FileDef::distributeMemberGroupDocumentation()
{
   for (auto mg : m_memberGroupSDict) {
      mg->distributeMemberGroupDocumentation();
   }
}

void FileDef::findSectionsInDocumentation()
{
   QSharedPointer<FileDef> self = sharedFrom(this);
   docFindSections(documentation(), self, QSharedPointer<MemberGroup>(), docFile());
  
   for (auto mg : m_memberGroupSDict) {
      mg->findSectionsInDocumentation();
   }   

   for (auto ml : m_memberLists) {
      if (ml->listType() & MemberListType_declarationLists) {
         ml->findSectionsInDocumentation();
      }
   }
}

bool FileDef::hasDetailedDescription() const
{
   static const bool repeatBrief   = Config::getBool("repeat-brief");
   static const bool sourceBrowser = Config::getBool("source-code");

   // avail empty section

   return ((! briefDescription().isEmpty() && repeatBrief) ||
            ! documentation().trimmed().isEmpty() || (sourceBrowser && getStartBodyLine() != -1 && getBodyDef()) );
}

void FileDef::writeTagFile(QTextStream &tagFile)
{
   tagFile << "  <compound kind=\"file\">" << endl;
   tagFile << "    <name>" << convertToXML(name()) << "</name>" << endl;
   tagFile << "    <path>" << convertToXML(getPath()) << "</path>" << endl;
   tagFile << "    <filename>" << convertToXML(getOutputFileBase()) << "</filename>" << endl;

   if (m_includeList.count() > 0) {
      
      for (auto item : m_includeList) {
         if (! item.indirect) {
            QSharedPointer<FileDef> fd = item.fileDef;

            if (fd && fd->isLinkable() && !fd->isReference()) {
               bool isIDLorJava = false;
               SrcLangExt lang = fd->getLanguage();

               isIDLorJava = lang == SrcLangExt_IDL || lang == SrcLangExt_Java;
               const char *locStr = (item.local    || isIDLorJava) ? "yes" : "no";
               const char *impStr = (item.imported || isIDLorJava) ? "yes" : "no";

               tagFile << "    <includes id=\""
                       << convertToXML(fd->getOutputFileBase()) << "\" "
                       << "name=\"" << convertToXML(fd->name()) << "\" "
                       << "local=\"" << locStr << "\" "
                       << "imported=\"" << impStr << "\">"
                       << convertToXML(item.includeName)
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
            for (auto cd : m_classSDict) {
               if (cd->isLinkableInProject()) {
                  tagFile << "    <class kind=\"" << cd->compoundTypeString() <<
                          "\">" << convertToXML(cd->name()) << "</class>" << endl;
               }
            }
            
         }
         break;

         case LayoutDocEntry::FileNamespaces: 
         {
            for (auto nd : m_namespaceSDict) {
               if (nd->isLinkableInProject()) {
                  tagFile << "    <namespace>" << convertToXML(nd->name()) << "</namespace>" << endl;
               }               
            }
         }
         break;

         case LayoutDocEntry::MemberDecl: 
         {
            LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
            QSharedPointer<MemberList> ml = getMemberList(lmd->type);

            if (ml) {
               ml->writeTagFile(tagFile);
            }
         }
         break;

         case LayoutDocEntry::MemberGroups: {
        
            for (auto mg : m_memberGroupSDict) {
               mg->writeTagFile(tagFile);
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

void FileDef::writeDetailedDescription(OutputList &ol, const QString &title)
{
   QSharedPointer<FileDef> self = sharedFrom(this);

   static const bool sourceCode      = Config::getBool("source-code");
   static const bool repeatBrief     = Config::getBool("repeat-brief"); 
   static const bool latexSourceCode = Config::getBool("latex-source-code");
   static const bool rtfSourceCode   = Config::getBool("rtf-source-code");

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

      if (! briefDescription().isEmpty() && repeatBrief ) {
         ol.generateDoc(briefFile(), briefLine(), self, QSharedPointer<MemberDef>(), briefDescription(), false, false);
      }

      if (! briefDescription().isEmpty() && repeatBrief  && ! documentation().isEmpty()) {
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

      if (! documentation().isEmpty()) {
         ol.generateDoc(docFile(), docLine(), self, QSharedPointer<MemberDef>(), documentation() + "\n", true, false);
      }
      
      if (sourceCode) {
         //if Latex enabled and LATEX_SOURCE_CODE is not, then skip

         ol.pushGeneratorState();
         if (ol.isEnabled(OutputGenerator::Latex) && ! latexSourceCode) {
            ol.disable(OutputGenerator::Latex);
         }

         if (ol.isEnabled(OutputGenerator::RTF) && ! rtfSourceCode) {
            ol.disable(OutputGenerator::RTF);
         }

         ol.startParagraph("definition");

         QString refText = theTranslator->trDefinedInSourceFile();
         int fileMarkerPos = refText.indexOf("@0");

         if (fileMarkerPos != -1) { 
            // should always pass this

            ol.parseText(refText.left(fileMarkerPos));                          //text left from marker 1
            ol.writeObjectLink(0, getSourceFileBase(), 0, name());
            ol.parseText(refText.right( refText.length() - fileMarkerPos - 2)); // text right from marker 2

         } else {
            err("Invalid translation markers in trDefinedInSourceFile\n");

         }         

         ol.endParagraph();        
         ol.popGeneratorState();
      }

      ol.endTextBlock();
   }
}

void FileDef::writeBriefDescription(OutputList &ol)
{
   static const bool repeatBrief = Config::getBool("repeat-brief");
   QSharedPointer<FileDef> self  = sharedFrom(this);

   if (hasBriefDescription()) {
      DocRoot *rootNode = validatingParseDoc(briefFile(), briefLine(), self, QSharedPointer<MemberDef>(),
                                             briefDescription(), true, false, "", true, false);

      if (rootNode && !rootNode->isEmpty()) {
         ol.startParagraph();

         ol.pushGeneratorState();
         ol.disableAllBut(OutputGenerator::Man);
         ol.writeString(" - ");
         ol.popGeneratorState();

         ol.writeDoc(rootNode, self, QSharedPointer<MemberDef>());

         ol.pushGeneratorState();
         ol.disable(OutputGenerator::RTF);
         ol.writeString(" \n");
         ol.enable(OutputGenerator::RTF);

         if (repeatBrief || ! documentation().isEmpty()) {
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
   if (m_includeList.count() > 0) {
      ol.startTextBlock(true);
     
      for (auto item : m_includeList) {
         if (! item.indirect) {
            QSharedPointer<FileDef> fd = item.fileDef;
            bool isIDLorJava = false;

            if (fd) {
               SrcLangExt lang = fd->getLanguage();
               isIDLorJava = lang == SrcLangExt_IDL || lang == SrcLangExt_Java;
            }

            ol.startTypewriter();

            if (isIDLorJava) { 
               // IDL/Java include
               ol.docify("import ");

            } else if (item.imported) { 
               // Objective-C include
               ol.docify("#import ");

            } else { // C/C++ include
               ol.docify("#include ");
            }

            if (item.local || isIDLorJava) {
               ol.docify("\"");
            } else {
               ol.docify("<");
            }

            ol.disable(OutputGenerator::Html);
            ol.docify(item.includeName);
            ol.enableAll();
            ol.disableAllBut(OutputGenerator::Html);

            // use the include file name as it appears in the file
            // we could also we the name as it is used within doxypress
            // then we should have used fd->docName() instead of ii->includeName

            if (fd && fd->isLinkable()) {
               ol.writeObjectLink(fd->getReference(),  
                  fd->generateSourceFile() ? fd->includeName() : fd->getOutputFileBase(), 0, item.includeName);

            } else {
               ol.docify(item.includeName);
            }

            ol.enableAll();
            if (item.local || isIDLorJava) {
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
   QSharedPointer<FileDef> self = sharedFrom(this);

   if (Config::getBool("have-dot") /*&& Config::getBool("dot-include")*/) {
     
      DotInclDepGraph incDepGraph(self, false);

      if (incDepGraph.isTooBig()) {
         warn_uncond("Include graph for '%s' not generated, too many nodes. Consider increasing DOT_GRAPH_MAX_NODES.\n", csPrintable(name()));

      } else if (! incDepGraph.isTrivial()) {
         ol.startTextBlock();
         ol.disable(OutputGenerator::Man);
         ol.startInclDepGraph();
         ol.parseText(theTranslator->trInclDepGraph(name()));
         ol.endInclDepGraph(incDepGraph);
         ol.enableAll();
         ol.endTextBlock(true);
      }      
   }
}

void FileDef::writeIncludedByGraph(OutputList &ol)
{
   QSharedPointer<FileDef> self = sharedFrom(this);

   if (Config::getBool("have-dot") /*&& Config::getBool("dot-included-by")*/) {      
      DotInclDepGraph incDepGraph(self, true);

      if (incDepGraph.isTooBig()) {
         warn_uncond("Included by graph for '%s' not generated, too many nodes. "
                     " Consider increasing 'DOT GRAPH MAX NODES'\n", qPrintable(name()));

      } else if (! incDepGraph.isTrivial()) {
         ol.startTextBlock();
         ol.disable(OutputGenerator::Man);
         ol.startInclDepGraph();
         ol.parseText(theTranslator->trInclByDepGraph());
         ol.endInclDepGraph(incDepGraph);
         ol.enableAll();
         ol.endTextBlock(true);
      }

   }
}

void FileDef::writeSourceLink(OutputList &ol)
{   
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

void FileDef::writeNamespaceDeclarations(OutputList &ol, const QString &title, bool isConstantGroup)
{
   // write list of namespaces 
   m_namespaceSDict.writeDeclaration(ol, title, isConstantGroup);   
}

void FileDef::writeClassDeclarations(OutputList &ol, const QString &title)
{
   // write list of classes  
   m_classSDict.writeDeclaration(ol, 0, title, false);   
}

void FileDef::writeInlineClasses(OutputList &ol)
{
   // temporarily undo the disbling could be done by startMemberDocumentation()
   // as a result of setting SEPARATE MEMBER PAGES to YES; see bug730512

   QSharedPointer<FileDef> self = sharedFrom(this);

   bool isEnabled = ol.isEnabled(OutputGenerator::Html);
   ol.enable(OutputGenerator::Html);
   
   m_classSDict.writeDocumentation(ol, self);
   
   // restore the initial state if needed
   if (! isEnabled) {
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
   if (Config::getBool("separate-member-pages")) {
      ol.disable(OutputGenerator::Html);
      Doxy_Globals::suppressDocWarnings = true;
   }
}

void FileDef::endMemberDocumentation(OutputList &ol)
{
   if (Config::getBool("separate-member-pages")) {
      ol.enable(OutputGenerator::Html);
      Doxy_Globals::suppressDocWarnings = false;
   }
}

void FileDef::writeMemberGroups(OutputList &ol)
{
   /* write user defined member groups */
   QSharedPointer<FileDef> self = sharedFrom(this);
  
   for (auto mg : m_memberGroupSDict) {
      if ((! mg->allMembersInSameSection() || ! m_subGrouping) && mg->header() != "[NOHEADER]") {
         mg->writeDeclarations(ol, QSharedPointer<ClassDef>(), QSharedPointer<NamespaceDef>(), self, QSharedPointer<GroupDef>());
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
   ol.parseText(theTranslator->trGeneratedAutomatically(Config::getString("project-name")));
   ol.popGeneratorState();
}

void FileDef::writeSummaryLinks(OutputList &ol)
{
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
  
   bool first = true;
   SrcLangExt lang = getLanguage();
  
   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::File)) {

      if ((lde->kind() == LayoutDocEntry::FileClasses && m_classSDict.declVisible()) ||
            (lde->kind() == LayoutDocEntry::FileNamespaces && m_namespaceSDict.declVisible()) ) {

         LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
         QString label = lde->kind() == LayoutDocEntry::FileClasses ? "nested-classes" : "namespaces";
         ol.writeSummaryLink("", label, ls->title(lang), first);
         first = false;

      } else if (lde->kind() == LayoutDocEntry::MemberDecl) {
         LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
         QSharedPointer<MemberList> ml = getMemberList(lmd->type);

         if (ml && ml->declVisible()) {
            ol.writeSummaryLink("", MemberList::listTypeAsString(ml->listType()), lmd->title(lang), first);
            first = false;
         }
      }
   }

   if (! first) {
      ol.writeString("  </div>\n");
   }
   ol.popGeneratorState();
}

/*! Write the documentation page for this file to the file of output 
    generators \a ol.
*/
void FileDef::writeDocumentation(OutputList &ol)
{
   QSharedPointer<FileDef> self = sharedFrom(this);
   static bool generateTreeView = Config::getBool("generate-treeview");

   QString versionTitle;

   if (! m_fileVersion.isEmpty()) {
      versionTitle = ("(" + m_fileVersion + ")");
   }

   QString title     = m_docname + versionTitle;
   QString pageTitle = theTranslator->trFileReference(m_docname);

   if (getDirDef()) {
      startFile(ol, getOutputFileBase(), name(), pageTitle, HLI_FileVisible, !generateTreeView);

      if (! generateTreeView) {
         getDirDef()->writeNavigationPath(ol);
         ol.endQuickIndices();
      }

      QString pageTitleShort = theTranslator->trFileReference(name());
      startTitle(ol, getOutputFileBase(), self);
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);
      ol.parseText(pageTitleShort); // Html only
      ol.enableAll();
      ol.disable(OutputGenerator::Html);
      ol.parseText(pageTitle); // other output formats
      ol.popGeneratorState();
      addGroupListToTitle(ol, self);
      endTitle(ol, getOutputFileBase(), title);

   } else {
      startFile(ol, getOutputFileBase(), name(), pageTitle, HLI_FileVisible, !generateTreeView);

      if (! generateTreeView) {
         ol.endQuickIndices();
      }

      startTitle(ol, getOutputFileBase(), self);
      ol.parseText(pageTitle);
      addGroupListToTitle(ol, self);
      endTitle(ol, getOutputFileBase(), title);
   }

   ol.startContents();

   if (! m_fileVersion.isEmpty()) {
      ol.disableAllBut(OutputGenerator::Html);
      ol.startProjectNumber();
      ol.docify(versionTitle);
      ol.endProjectNumber();
      ol.enableAll();
   }

   if (Doxy_Globals::searchIndexBase != nullptr) {
      Doxy_Globals::searchIndexBase->setCurrentDoc(self, anchor(), false);
      Doxy_Globals::searchIndexBase->addWord(localName(), true);
   } 

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
  
   ol.endContents();
  
   endFileWithNavPath(self, ol);

   if (Config::getBool("separate-member-pages")) {
      QSharedPointer<MemberList> ml = getMemberList(MemberListType_allMembersList);

      if (ml) {
         ml->sort();
      }
      writeMemberPages(ol);
   }
}

void FileDef::writeMemberPages(OutputList &ol)
{
   QSharedPointer<FileDef> self = sharedFrom(this);

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
  
   for (auto ml : m_memberLists) {
      if (ml->listType()&MemberListType_documentationLists) {
         ml->writeDocumentationPage(ol, name(), self);
      }
   }

   ol.popGeneratorState();
}

void FileDef::writeQuickMemberLinks(OutputList &ol, QSharedPointer<MemberDef> currentMd) const
{
   static bool createSubDirs = Config::getBool("create-subdirs");

   ol.writeString("      <div class=\"navtab\">\n");
   ol.writeString("        <table>\n");

   QSharedPointer<MemberList> allMemberList = getMemberList(MemberListType_allMembersList);

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
               ol.writeString(md->getOutputFileBase() + Doxy_Globals::htmlFileExtension + "#" + md->anchor());
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
   QSharedPointer<FileDef> self = sharedFrom(this);

   static const bool generateTreeView  = Config::getBool("generate-treeview");
   static const bool filterSourceFiles = Config::getBool("filter-source-files");
   static const bool latexSourceCode   = Config::getBool("latex-source-code");
   static const bool rtfSourceCode     = Config::getBool("rtf-source-code");

   DevNullCodeDocInterface devNullIntf;
   QString title = m_docname;

   if (! m_fileVersion.isEmpty()) {
      title += (" (" + m_fileVersion + ")");
   }

   QString pageTitle = theTranslator->trSourceFile(title);
   ol.disable(OutputGenerator::Man);

   if (! latexSourceCode) {
      ol.disable(OutputGenerator::Latex);
   }

   if (! rtfSourceCode) {
      ol.disable(OutputGenerator::RTF);
   }

   bool isDocFile = isDocumentationFile();
   bool genSourceFile = isDocFile && generateSourceFile();

   if (getDirDef()) {

      startFile(ol, getSourceFileBase(), QString(), pageTitle, HLI_FileVisible, ! generateTreeView, 
                  isDocFile && genSourceFile ? QString() : getOutputFileBase());

      if (! generateTreeView) {
         getDirDef()->writeNavigationPath(ol);
         ol.endQuickIndices();
      }

      startTitle(ol, getSourceFileBase());
      ol.parseText(name());
      endTitle(ol, getSourceFileBase(), title);

   } else {
      startFile(ol, getSourceFileBase(), QString(), pageTitle, HLI_FileVisible, false,
                isDocFile && genSourceFile ? QString() : getOutputFileBase());

      startTitle(ol, getSourceFileBase());
      ol.parseText(title);
      endTitle(ol, getSourceFileBase(), 0);
   }

   ol.startContents();

   if (isLinkable()) {
      if (latexSourceCode) {
         ol.disable(OutputGenerator::Latex);
      }

      if (rtfSourceCode) {
         ol.disable(OutputGenerator::RTF);
      }

      ol.startTextLink(getOutputFileBase(), 0);
      ol.parseText(theTranslator->trGotoDocumentation());
      ol.endTextLink();

      if (latexSourceCode) {
         ol.enable(OutputGenerator::Latex);
      }

      if (rtfSourceCode) {
         ol.enable(OutputGenerator::RTF);
      }
   }

   // user specified  
   static bool clangParsing = Config::getBool("clang-parsing");
   auto srcLang = getLanguage();

   if (clangParsing && (srcLang == SrcLangExt_Cpp || srcLang == SrcLangExt_ObjC)) {

      ol.startCodeFragment();

      if (! sameTu) {
         ClangParser::instance()->start(getFilePath(), filesInSameTu, QSharedPointer<Entry>());

      } else {
         ClangParser::instance()->switchToFile(getFilePath());
      }
 
      ClangParser::instance()->writeSources(ol, self); 
      ol.endCodeFragment();

   } else  {
      // use lex
   
      ParserInterface *pIntf = Doxy_Globals::parserManager.getParser(getDefFileExtension());
      pIntf->resetCodeParserState();
      ol.startCodeFragment();

      bool needs2PassParsing = Doxy_Globals::parseSourcesNeeded && ! filterSourceFiles &&                         
                  ! getFileFilter(getFilePath(), true).isEmpty(); 

      // need to parse (filtered) sources for cross-references, however the user wants to show sources as-is
      // and there is a filter used while parsing

      if (needs2PassParsing) {
         // parse code for cross-references only (see bug707641)
         pIntf->parseCode(devNullIntf, 0, fileToString(getFilePath(), true, true), getLanguage(), false, 0, self);
      }

      pIntf->parseCode(ol, 0, fileToString(getFilePath(), filterSourceFiles, true),
                       srcLang, false, 0, self, -1, -1, false,
                       QSharedPointer<MemberDef>(), true, QSharedPointer<Definition>(), ! needs2PassParsing);
                      
      ol.endCodeFragment();
   }
  
   ol.endContents();
   endFileWithNavPath(self, ol);
   ol.enableAll();
}

void FileDef::parseSource(bool sameTu, QStringList &filesInSameTu)
{
   QSharedPointer<FileDef> self  = sharedFrom(this);
   static bool filterSourceFiles = Config::getBool("filter-source-files");

   DevNullCodeDocInterface devNullIntf;  

   static bool clangParsing = Config::getBool("clang-parsing");
   auto srcLang = getLanguage();

   if (clangParsing && (srcLang == SrcLangExt_Cpp || srcLang == SrcLangExt_ObjC)) {
      // use clang parser
 
      if (! sameTu) {
         ClangParser::instance()->start(getFilePath(), filesInSameTu, QSharedPointer<Entry>());

      } else {
         ClangParser::instance()->switchToFile(getFilePath());
      }

      ClangParser::instance()->writeSources(devNullIntf, self);

   } else {
      // use lex parser    
   
      ParserInterface *pIntf = Doxy_Globals::parserManager.getParser(getDefFileExtension());
      pIntf->resetCodeParserState();
      pIntf->parseCode(devNullIntf, 0, fileToString(getFilePath(), filterSourceFiles, true), srcLang, false, 0, self);
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
   QSharedPointer<FileDef> self = sharedFrom(this);
   MemberGroupSDict *temp = &m_memberGroupSDict;

   for (auto ml : m_memberLists) {
      if (ml->listType() & MemberListType_declarationLists) {
         ::addMembersToMemberGroup(ml, &temp, self);    
      }
   }

   // add members inside sections to their groups
   for (auto mg : m_memberGroupSDict) {
      if (mg->allMembersInSameSection() && m_subGrouping) {           
         mg->addToDeclarationSection();
      }
   }
 
}

/*! Adds member definition \a md to the list of all members of this file */
void FileDef::insertMember(QSharedPointer<MemberDef> md)
{
   if (md->isHidden()) {
      return;
   }

   QSharedPointer<MemberList> allMemberList = getMemberList(MemberListType_allMembersList);

   if (allMemberList && allMemberList->lastIndexOf(md) != -1) { 
      return;
   }

   if (allMemberList == nullptr) {   
      m_memberLists.append(QMakeShared<MemberList>(MemberListType_allMembersList));
      allMemberList = m_memberLists.last();
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

         if (  md->getClassDef() ) {
           err("FileDef::insertMembers(): member `%s' with class scope `%s' inserted in file scope `%s'\n",
             qPrintable(md->name()), qPrintable(md->getClassDef()->name()), qPrintable(name()) );

         } else  { 

           err("FileDef::insertMembers(): member `%s' with class scope `%s' inserted in file scope `%s'\n",
             qPrintable(md->name()), "<global>", qPrintable(name()) );
         }
   }

}

/*! Adds compound definition \a cd to the list of all compounds of this file */
void FileDef::insertClass(QSharedPointer<ClassDef> cd)
{
   if (cd->isHidden()) {
      return;
   } 
 
   m_classSDict.insert(cd->name(), cd);   
}

/*! Adds namespace definition \a nd to the list of all compounds of this file */
void FileDef::insertNamespace(QSharedPointer<NamespaceDef> nd)
{
   if (nd->isHidden()) {
      return;
   }

   if (! nd->name().isEmpty() && m_namespaceSDict.find(nd->name()) == 0) {    
      m_namespaceSDict.insert(nd->name(), nd);      
   }
}

QString FileDef::name() const
{
   if (Config::getBool("full-path-names")) {
      return m_fileName;
   } else {
      return Definition::name();
   }
}

void FileDef::addSourceRef(int line, QSharedPointer<Definition> d, QSharedPointer<MemberDef> md)
{   
   if (d) {          
      m_srcDefDict.insert(line, d);

      if (md) {
         m_srcMemberDict.insert(line, md);
      }      
   }
}

QSharedPointer<Definition> FileDef::getSourceDefinition(int lineNr) const
{
   QSharedPointer<Definition> result; 
   result = m_srcDefDict.value(lineNr);
    
   return result;
}

QSharedPointer<MemberDef> FileDef::getSourceMember(int lineNr) const
{
   QSharedPointer<MemberDef> result;
   result = m_srcMemberDict.value(lineNr);
     
   return result;
}

void FileDef::addUsingDirective(QSharedPointer<NamespaceDef> nd)
{
   if (! m_usingDirList.find(nd->qualifiedName()) ) {
      m_usingDirList.insert(nd->qualifiedName(), nd);
   }   
}

NamespaceSDict *FileDef::getUsedNamespaces()
{
   return &m_usingDirList;
}

void FileDef::addUsingDeclaration(QSharedPointer<Definition> d)
{
   if (! m_usingDeclList.find(d->qualifiedName()) ) {
      m_usingDeclList.insert(d->qualifiedName(), d);
   }
}

void FileDef::addIncludeDependency(QSharedPointer<FileDef> fd, const QString &incName, bool local, bool imported, bool indirect)
{
   QString iName;

   if (fd) {
      iName = fd->getFilePath();

   } else {
      iName = incName;
   }

   if (! iName.isEmpty() && ! m_includeDict.contains(iName)) {

      IncludeInfo ii;
      ii.fileDef     = fd;
      ii.includeName = incName;
      ii.local       = local;
      ii.imported    = imported;
      ii.indirect    = indirect;

      m_includeList.append(ii);
      m_includeDict.insert(iName, ii);
   }
}

void FileDef::addIncludedUsingDirectives()
{
   if (visited) {
      return;
   }

   visited = true;
  
   if (! m_includeList.isEmpty()) { 
      // file contains #includes
            
      for (auto &item : m_includeList) {
         // foreach #include which is a known file

         if (item.fileDef && ! item.fileDef->visited) {                
            // recurse into this file
            item.fileDef->addIncludedUsingDirectives();
         }
      }
                 
      for (auto incList : m_includeList) {

         if (incList.fileDef && incList.fileDef != this) {
            // add using directives
            NamespaceSDict &unl = incList.fileDef->m_usingDirList;
                      
            for (auto nd : unl) {
               // append each using directive found in a #include file
                              
               if (m_usingDirList.find(nd->qualifiedName()) == 0) {                    
                  m_usingDirList.insert(nd->qualifiedName(), nd);
               }
            }
           
            // add using declarations
            StringMap<QSharedPointer<Definition>> &udl = incList.fileDef->m_usingDeclList;
                     
            for (auto d : udl) {                           
               if (m_usingDeclList.find(d->qualifiedName()) == 0) {
                  m_usingDeclList.insert(d->qualifiedName(), d);
               }
            }
           
         }
      }      
   }   
}

void FileDef::addIncludedByDependency(QSharedPointer<FileDef> fd, const QString &incName, bool local, bool imported)
{
   QString iName = fd ? fd->getFilePath() : incName;

   if (! iName.isEmpty() && ! m_includedByDict.contains(iName)) {
     
      IncludeInfo ii;
      ii.fileDef     = fd;
      ii.includeName = incName;
      ii.local       = local;
      ii.imported    = imported;
      ii.indirect    = false;

      m_includedByList.append(ii);
      m_includedByDict.insert(iName, ii);
   }
}

bool FileDef::isIncluded(const QString &name) const
{
   if (name.isEmpty()) {
      return false;
   }

   return ! m_includeDict.contains(name);
}

bool FileDef::generateSourceFile() const
{
   static bool sourceBrowser   = Config::getBool("source-code");
   static bool verbatimHeaders = Config::getBool("verbatim-headers");   
      
   bool retval = ! isReference();

   if (retval) {
      retval = (sourceBrowser || (verbatimHeaders && determineSection(name()) == Entry::HEADER_SEC));     
   }

   if (retval) {      
      retval = isDocumentationFile();      
   }     

   return retval;
}

bool FileDef::isDocumentationFile() const
{
   const QStringList suffixExclude = Config::getList("suffix-exclude-navtree");

   QFileInfo fi(name());
   QString suffix = fi.suffix();

   bool retval = true;

   if (suffixExclude.contains(suffix))  {
      // exclude file extension
      retval = false;
   }

   return retval;
}

void FileDef::addListReferences()
{   
   QSharedPointer<FileDef> self = sharedFrom(this);

   const QList<ListItemInfo> &xrefItems = getRefItems();

   addRefItem(xrefItems, getOutputFileBase(), theTranslator->trFile(true, true), getOutputFileBase(), name(), 
              "", QSharedPointer<Definition>());
      
   for (auto mg : m_memberGroupSDict) {
      mg->addListReferences(self);
   }
   
   for (auto ml : m_memberLists) {
      if (ml->listType() & MemberListType_documentationLists) {
         ml->addListReferences(self);
      }
   }
}

static int findMatchingPart(const QString &path, const QString dir)
{
   int si1;
   int pos1 = 0;
   int pos2 = 0;

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

static QSharedPointer<DirEntryTree> findDirNode(QSharedPointer<DirEntryTree> root, const QString &name)
{  
   for (auto de : root->children()) {

      if (de->kind() == DirEntry::Dir) {
         QSharedPointer<DirEntryTree> dir = de.dynamicCast<DirEntryTree>();

         QString dirName = dir->name();
         int sp = findMatchingPart(name, dirName);

         if (sp > 0) { 
            // match found

            if (sp == dirName.length()) { 
               // whole directory matches, recurse into the directory
               return findDirNode(dir, name.mid(dirName.length() + 1));

            } else { // partial match => we need to split the path into three parts
               QString baseName      = dirName.left(sp);
               QString oldBranchName = dirName.mid(sp + 1);
               QString newBranchName = name.mid(sp + 1);

               // strip file name from path
               int newIndex = newBranchName.lastIndexOf('/');

               if (newIndex > 0) {
                  newBranchName = newBranchName.left(newIndex);
               }
              
               QSharedPointer<DirEntryTree> base      = QMakeShared<DirEntryTree>(root, baseName);
               QSharedPointer<DirEntryTree> newBranch = QMakeShared<DirEntryTree>(base, newBranchName);

               dir->reParent(base);
               dir->rename(oldBranchName);
               base->addChild(dir);
               base->addChild(newBranch);
               dir->setLast(false);

               // remove DirEntry container from list (without deleting it)               
               root->children().removeOne(dir);
              
               // add new branch to the root
               if (! root->children().isEmpty()) {
                  root->children().last()->setLast(false);
               }

               root->addChild(base);
               return newBranch;
            }
         }
      }
   }

   int si = name.lastIndexOf('/');

   if (si == -1) { 
      // no subdir, put the file under the root node.
      return root; 

   } else { 
      // need to create a subdir
      QString baseName = name.left(si);

      QSharedPointer<DirEntryTree> newBranch = QMakeShared<DirEntryTree>(root, baseName);
      if (! root->children().isEmpty()) {
         root->children().last()->setLast(false);
      }

      root->addChild(newBranch);
      return newBranch;
   }
}

static void mergeFileDef(QSharedPointer<DirEntryTree> root, QSharedPointer<FileDef> fd)
{  
   QString filePath = fd->getFilePath();
 
   QSharedPointer<DirEntryTree> dirNode = findDirNode(root, filePath);

   if (! dirNode->children().isEmpty()) {
      dirNode->children().last()->setLast(false);
   }

   QSharedPointer<DirEntry> e = QMakeShared<DirEntry>(dirNode, fd);
   dirNode->addChild(e);
}

static void addDirsAsGroups(QSharedPointer<DirEntryTree> root, QSharedPointer<GroupDef> parent, int level)
{
   QSharedPointer<GroupDef> gd;

   if (root->kind() == DirEntry::Dir) {
      gd = QMakeShared<GroupDef>("[generated]", 1, root->path(), root->name());

      if (parent) {
         parent->addGroup(gd);
         gd->makePartOfGroup(parent);

      } else {
         Doxy_Globals::groupSDict.insert(root->path(), gd);
      }
   }
  
   for (auto de : root->children()) {
      if (de->kind() == DirEntry::Dir) {
         QSharedPointer<DirEntryTree> dir = de.dynamicCast<DirEntryTree>();
         addDirsAsGroups(dir, gd, level + 1);
      }
   }
}

#if 0
   void generateFileTree()
   {
      QSharedPointer<DirEntryTree> root = QMakeShared<DirEntryTree>(0, "root");
      root->setLast(true);
     
      for (auto fn : *Doxy_Globals::inputNameList ) {    
         for (auto fd :*fn) { 
            mergeFileDef(root, fd);
         }
      }
    
      addDirsAsGroups(root, QSharedPointer<GroupDef>(), 0);
   
      delete root;
   }
#endif

void FileDef::combineUsingRelations()
{
   if (visited) {
      return;   // already done
   }

   visited = true;
       
   for (auto nd : m_usingDirList) {
      nd->combineUsingRelations();
   }

   for (auto nd : m_usingDirList) {
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

void FileDef::acquireFileVersion()
{
   QString vercmd = Config::getString("file-version-filter");

   if (! vercmd.isEmpty() && ! m_filePath.isEmpty() && 
               m_filePath != "generated" && m_filePath != "graph_legend") {

      msg("Version of %s : ", csPrintable(m_filePath));

      QString cmd = vercmd + " \"" + m_filePath + "\"";
      Debug::print(Debug::ExtCmd, 0, "Executing popen(`%s`)\n", csPrintable(cmd));

      FILE *f = popen(csPrintable(cmd), "r");

      if (! f) {
         err("Unable to execute %s\n", csPrintable(vercmd));
         return;
      }

      const int bufSize = 1024;

      QByteArray buf;
      buf.resize(bufSize);

      int numRead = fread(buf.data(), 1, bufSize - 1, f);
      pclose(f);

      if (numRead > 0 && numRead < bufSize) {
         buf.resize(numRead);

         m_fileVersion = QByteArray(buf, numRead).trimmed();

         if (! m_fileVersion.isEmpty()) {
            msg("%s\n", qPrintable(m_fileVersion));
            return;
         }
      }

      msg("No version available\n");
   }
}

QString FileDef::getSourceFileBase() const
{
   if (Htags::useHtags) {
      return Htags::path2URL(m_filePath);
   } else {
      return (convertNameToFile(m_diskName) + "_source");
   }
}

/*! Returns the name of the verbatim copy of this file (if any) */
QString FileDef::includeName() const
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
   m_memberLists.append(QMakeShared<MemberList>(lt));
   
   return m_memberLists.last();
}

void FileDef::addMemberToList(MemberListType lt, QSharedPointer<MemberDef> md)
{
   QSharedPointer<FileDef> self  = sharedFrom(this);
   QSharedPointer<MemberList> ml = createMemberList(lt);  

   static bool sortBriefDocs  = Config::getBool("sort-brief-docs");
   static bool sortMemberDocs = Config::getBool("sort-member-docs");

   bool isSorted = false;

   if (sortBriefDocs && (ml->listType() & MemberListType_declarationLists)) {
      isSorted = true;
   } else if (sortMemberDocs && (ml->listType() & MemberListType_documentationLists)) {
      isSorted = true;
   }

   if (isSorted) {
      ml->inSort(md);
   } else {
      ml->append(md);
   }

   if (lt & MemberListType_documentationLists) {
      ml->setInFile(true);
   }

   if (ml->listType() & MemberListType_declarationLists) {
      md->setSectionList(self, ml);
   }
}

QSharedPointer<MemberList> FileDef::getMemberList(MemberListType lt) const
{
   for (auto ml : m_memberLists) {
      if (ml->listType() == lt) {
         return ml;
      }
   }

   return QSharedPointer<MemberList>();
}

void FileDef::writeMemberDeclarations(OutputList &ol, MemberListType lt, const QString &title)
{ 
   QSharedPointer<FileDef> self = sharedFrom(this);
   QSharedPointer<MemberList> ml = getMemberList(lt);

   if (ml) {     
      ml->writeDeclarations(ol, QSharedPointer<ClassDef>(), QSharedPointer<NamespaceDef>(), self, 
                           QSharedPointer<GroupDef>(), title, 0);      
   }
}

void FileDef::writeMemberDocumentation(OutputList &ol, MemberListType lt, const QString &title)
{
   QSharedPointer<FileDef> self  = sharedFrom(this);
   QSharedPointer<MemberList> ml = getMemberList(lt);

   if (ml) {
      ml->writeDocumentation(ol, name(), self, title);
   }
}

bool FileDef::isLinkableInProject() const
{
   static bool showFiles = Config::getBool("show-file-page");

   return hasDocumentation() && ! isReference() && (showFiles || isLinkableViaGroup());
}

static void getAllIncludeFilesRecursively(QSet<QString> &filesVisited, QSharedPointer<const FileDef> fd, QStringList &incFiles)
{
   if (fd->includeFileList()) {

      for (auto &item : *fd->includeFileList() ) {   
         if (item.fileDef && ! item.fileDef->isReference() && ! filesVisited.contains(item.fileDef->getFilePath())) {

            incFiles.append(item.fileDef->getFilePath());
            filesVisited.insert(item.fileDef->getFilePath());

            getAllIncludeFilesRecursively(filesVisited, item.fileDef, incFiles);
         }
      }
   }
}

void FileDef::getAllIncludeFilesRecursively(QStringList &incFiles) const
{
   QSharedPointer<const FileDef> self = sharedFrom(this);

   QSet<QString> includes;
   ::getAllIncludeFilesRecursively(includes, self, incFiles);
}

QString FileDef::title() const
{
   return theTranslator->trFileReference(name());
}

QString FileDef::fileVersion() const
{
   return m_fileVersion;
}
