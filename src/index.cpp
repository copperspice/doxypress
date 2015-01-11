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

/** @file
 *  @brief This file contains functions for the various index pages.
 */

#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QRegExp>

#include <stdlib.h>

#include <message.h>
#include <index.h>
#include <doxygen.h>
#include <config.h>
#include <filedef.h>
#include <outputlist.h>
#include <util.h>
#include <groupdef.h>
#include <language.h>
#include <htmlgen.h>
#include <htmlhelp.h>
#include <ftvhelp.h>
#include <dot.h>
#include <pagedef.h>
#include <dirdef.h>
#include <layout.h>
#include <memberlist.h>
#include <classlist.h>
#include <namespacedef.h>
#include <filename.h>
#include <sortedlist.h>

// must appear after the previous include - resolve soon 
#include <doxy_globals.h>

#define MAX_ITEMS_BEFORE_MULTIPAGE_INDEX 200
#define MAX_ITEMS_BEFORE_QUICK_INDEX 30

int annotatedClasses;
int annotatedClassesPrinted;
int hierarchyClasses;
int documentedFiles;
int documentedGroups;
int documentedNamespaces;
int indexedPages;
int documentedClassMembers[CMHL_Total];
int documentedFileMembers[FMHL_Total];
int documentedNamespaceMembers[NMHL_Total];
int documentedHtmlFiles;
int documentedPages;
int documentedDirs;

static int countClassHierarchy();
static void countFiles(int &htmlFiles, int &files);
static int countGroups();
static int countDirs();
static int countNamespaces();
static int countAnnotatedClasses(int *cp);
static void countRelatedPages(int &docPages, int &indexPages);

void countDataStructures()
{
   annotatedClasses           = countAnnotatedClasses(&annotatedClassesPrinted); // "classes" + "annotated"
   hierarchyClasses           = countClassHierarchy();   // "hierarchy"
   countFiles(documentedHtmlFiles, documentedFiles);     // "files"
   countRelatedPages(documentedPages, indexedPages);     // "pages"
   documentedGroups           = countGroups();           // "modules"
   documentedNamespaces       = countNamespaces();       // "namespaces"
   documentedDirs             = countDirs();             // "dirs"
   // "globals"
   // "namespacemembers"
   // "functions"
}

static void startIndexHierarchy(OutputList &ol, int level)
{
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Man);
   ol.disable(OutputGenerator::Html);
   if (level < 6) {
      ol.startIndexList();
   }
   ol.enableAll();
   ol.disable(OutputGenerator::Latex);
   ol.disable(OutputGenerator::RTF);
   ol.startItemList();
   ol.popGeneratorState();
}

static void endIndexHierarchy(OutputList &ol, int level)
{
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Man);
   ol.disable(OutputGenerator::Html);
   if (level < 6) {
      ol.endIndexList();
   }
   ol.enableAll();
   ol.disable(OutputGenerator::Latex);
   ol.disable(OutputGenerator::RTF);
   ol.endItemList();
   ol.popGeneratorState();
}

//----------------------------------------------------------------------------

class MemberIndexList : public QList<MemberDef *>
{
 public: 
   MemberIndexList(uint letter) : m_letter(letter)
   {}

   ~MemberIndexList()
    {}

   int compareValues(const MemberDef *md1, const MemberDef *md2) const {
      int result = qstricmp(md1->name(), md2->name());

      if (result == 0) {
         result = qstricmp(md1->qualifiedName(), md2->qualifiedName());
      }
      return result;
   }

   void insertDef(MemberDef *d) {
      append(d);
   }

   uint letter() const {
      return m_letter;
   }

 private:
   uint m_letter;
};

static LetterToIndexMap<MemberIndexList>   g_memberIndexLetterUsed[CMHL_Total];
static LetterToIndexMap<MemberIndexList>   g_fileIndexLetterUsed[FMHL_Total];
static LetterToIndexMap<MemberIndexList>   g_namespaceIndexLetterUsed[NMHL_Total];

const int maxItemsBeforeQuickIndex = MAX_ITEMS_BEFORE_QUICK_INDEX;

static void startQuickIndexList(OutputList &ol, bool letterTabs = false)
{
   bool fancyTabs = true;
   if (fancyTabs) {
      if (letterTabs) {
         ol.writeString("  <div id=\"navrow4\" class=\"tabs3\">\n");
      } else {
         ol.writeString("  <div id=\"navrow3\" class=\"tabs2\">\n");
      }
      ol.writeString("    <ul class=\"tablist\">\n");
   } else {
      ol.writeString("  <div class=\"qindex\">");
   }
}

static void endQuickIndexList(OutputList &ol)
{
   bool fancyTabs = true;
   if (fancyTabs) {
      ol.writeString("    </ul>\n");
   }
   ol.writeString("  </div>\n");
}

static void startQuickIndexItem(OutputList &ol, const char *l, bool hl, bool compact, bool &first)
{
   bool fancyTabs = true;
   if (!first && compact && !fancyTabs) {
      ol.writeString(" | ");
   }
   first = false;
   if (fancyTabs) {
      ol.writeString("      <li");
      if (hl) {
         ol.writeString(" class=\"current\"");
      }
      ol.writeString("><a ");
   } else {
      if (!compact) {
         ol.writeString("<li>");
      }
      if (hl && compact) {
         ol.writeString("<a class=\"qindexHL\" ");
      } else {
         ol.writeString("<a class=\"qindex\" ");
      }
   }
   ol.writeString("href=\"");
   ol.writeString(l);
   ol.writeString("\">");
   if (fancyTabs) {
      ol.writeString("<span>");
   }
}

static void endQuickIndexItem(OutputList &ol)
{
   bool fancyTabs = true;
   if (fancyTabs) {
      ol.writeString("</span>");
   }
   ol.writeString("</a>");
   if (fancyTabs) {
      ol.writeString("</li>\n");
   }
}

// don't make this static as it is called from a template function and some
// old compilers don't support calls to static functions from a template.
QByteArray fixSpaces(const QByteArray &s)
{
   return substitute(s, " ", "&#160;");
}

void startTitle(OutputList &ol, const char *fileName, Definition *def)
{
   ol.startHeaderSection();
   if (def) {
      def->writeSummaryLinks(ol);
   }
   ol.startTitleHead(fileName);
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Man);
}

void endTitle(OutputList &ol, const char *fileName, const char *name)
{
   ol.popGeneratorState();
   ol.endTitleHead(fileName, name);
   ol.endHeaderSection();
}

void startFile(OutputList &ol, const char *name, const char *manName,
               const char *title, HighlightedItem hli, bool additionalIndices,
               const char *altSidebarName)
{
   static bool disableIndex     = Config_getBool("DISABLE_INDEX");
   ol.startFile(name, manName, title);
   ol.startQuickIndices();
   if (!disableIndex) {
      ol.writeQuickLinks(true, hli, name);
   }
   if (!additionalIndices) {
      ol.endQuickIndices();
   }
   ol.writeSplitBar(altSidebarName ? altSidebarName : name);
   ol.writeSearchInfo();
}

void endFile(OutputList &ol, bool skipNavIndex, bool skipEndContents,
             const QByteArray &navPath)
{
   static bool generateTreeView = Config_getBool("GENERATE_TREEVIEW");
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
   if (!skipNavIndex) {
      if (!skipEndContents) {
         ol.endContents();
      }
      if (generateTreeView) {
         ol.writeString("</div><!-- doc-content -->\n");
      }
   }
   ol.writeFooter(navPath); // write the footer
   ol.popGeneratorState();
   ol.endFile();
}

void endFileWithNavPath(Definition *d, OutputList &ol)
{
   static bool generateTreeView = Config_getBool("GENERATE_TREEVIEW");
   QByteArray navPath;
   if (generateTreeView) {
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);
      ol.writeString("</div><!-- doc-content -->\n");
      ol.popGeneratorState();
      navPath = d->navigationPathAsString();
   }
   endFile(ol, generateTreeView, true, navPath);
}

template<class T>
void addMembersToIndex(T *def, LayoutDocManager::LayoutPart part, const QString &name, const QByteArray &anchor,
                       bool addToIndex = true, bool preventSeparateIndex = false)
{
   bool hasMembers = def->getMemberLists().count() > 0 || def->getMemberGroupSDict() != 0;

   Doxygen::indexList->addContentsItem(hasMembers, name, def->getReference(), def->getOutputFileBase(), anchor,
                                       hasMembers && !preventSeparateIndex, addToIndex, def);

   int numClasses = 0;

   ClassSDict *classes = def->getClassSDict();

   if (classes) {    
      for (auto cd : *classes) {
         if (cd->isLinkable()) {
            numClasses++;
         }
      }
   }
 
   if (hasMembers || numClasses > 0) {

      Doxygen::indexList->incContentsDepth();
 
      for (auto lde : LayoutDocManager::instance().docEntries(part)) { 

         if (lde->kind() == LayoutDocEntry::MemberDef) {
            LayoutDocEntryMemberDef *lmd = (LayoutDocEntryMemberDef *)lde;
            MemberList *ml = def->getMemberList(lmd->type);

            if (ml) {
              
               for (auto md : *ml ) {
                  MemberList *enumList = md->enumFieldList();

                  bool isDir = (enumList != 0 && md->isEnumerate());
                  bool isAnonymous = md->name().indexOf('@') != -1;

                  static bool hideUndocMembers = Config_getBool("HIDE_UNDOC_MEMBERS");
                  static bool extractStatic = Config_getBool("EXTRACT_STATIC");

                  if (!isAnonymous && (!hideUndocMembers || md->hasDocumentation()) && (!md->isStatic() || extractStatic)) {
                     if (md->getOuterScope() == def || md->getOuterScope() == Doxygen::globalScope) {
                        Doxygen::indexList->addContentsItem(isDir, md->name(), md->getReference(), 
                                 md->getOutputFileBase(), md->anchor(), false, addToIndex);

                     } else { // inherited member
                        Doxygen::indexList->addContentsItem(isDir, md->name(), def->getReference(), 
                                 def->getOutputFileBase(), md->anchor(), false, addToIndex);
                     }
                  }

                  if (isDir) {
                     if (!isAnonymous) {
                        Doxygen::indexList->incContentsDepth();
                     }
                   
                     for (auto emd : *enumList ) {
                        if (! hideUndocMembers || emd->hasDocumentation()) {
                           if (emd->getOuterScope() == def || emd->getOuterScope() == Doxygen::globalScope) {
                              Doxygen::indexList->addContentsItem(false, emd->name(), emd->getReference(), 
                                    emd->getOutputFileBase(), emd->anchor(), false, addToIndex);

                           } else { // inherited member
                              Doxygen::indexList->addContentsItem(false, emd->name(), def->getReference(), 
                                    def->getOutputFileBase(), emd->anchor(), false, addToIndex);
                           }
                        }
                     }

                     if (!isAnonymous) {
                        Doxygen::indexList->decContentsDepth();
                     }
                  }
               }
            }
         } else if (lde->kind() == LayoutDocEntry::NamespaceClasses || lde->kind() == LayoutDocEntry::FileClasses ||
                    lde->kind() == LayoutDocEntry::ClassNestedClasses ) {

            if (classes) {
               
               for (auto cd : *classes ) {
                  if (cd->isLinkable() && (cd->partOfGroups() == 0 || def->definitionType() == Definition::TypeGroup)) {
                     static bool inlineSimpleStructs = Config_getBool("INLINE_SIMPLE_STRUCTS");
                     bool isNestedClass = def->definitionType() == Definition::TypeClass;
                     
                     addMembersToIndex(cd.data(), LayoutDocManager::Class, cd->displayName(false), cd->anchor(),
                                       addToIndex && (isNestedClass || (cd->isSimple() && inlineSimpleStructs)),
                                       preventSeparateIndex || cd->isEmbeddedInOuterScope());
                  }
               }
            }
         }
      }

      Doxygen::indexList->decContentsDepth();
   }
}


//----------------------------------------------------------------------------
/*! Generates HTML Help tree of classes */

static void writeClassTree(OutputList &ol, const SortedList<BaseClassDef *> *bcl, bool hideSuper, int level, FTVHelp *ftv, bool addToIndex)
{
   if (bcl == 0) {
      return;
   }
  
   bool started = false;
   
   for (auto item : *bcl) {

      ClassDef *cd = item->classDef;
      
      bool b;
      b = hasVisibleRoot(cd->baseClasses());
      
      if (cd->isVisibleInHierarchy() && b) { 
         // hasVisibleRoot(cd->baseClasses()))
         if (!started) {
            startIndexHierarchy(ol, level);
            if (addToIndex) {
               Doxygen::indexList->incContentsDepth();
            }
            if (ftv) {
               ftv->incContentsDepth();
            }
            started = true;
         }

         ol.startIndexListItem();
        
         bool hasChildren = !cd->visited && !hideSuper && classHasVisibleChildren(cd);
        
         if (cd->isLinkable()) {           
            ol.startIndexItem(cd->getReference(), cd->getOutputFileBase());
            ol.parseText(cd->displayName());
            ol.endIndexItem(cd->getReference(), cd->getOutputFileBase());

            if (cd->isReference()) {
               ol.startTypewriter();
               ol.docify(" [external]");
               ol.endTypewriter();
            }

            if (addToIndex) {
               Doxygen::indexList->addContentsItem(hasChildren, cd->displayName(), cd->getReference(), cd->getOutputFileBase(), cd->anchor());
            }

            if (ftv) {              
               ftv->addContentsItem(hasChildren, cd->displayName(), cd->getReference(), cd->getOutputFileBase(), cd->anchor(), false, false, cd);               
            }

         } else {
            ol.startIndexItem(0, 0);
            ol.parseText(cd->name());
            ol.endIndexItem(0, 0);

            if (addToIndex) {
               Doxygen::indexList->addContentsItem(hasChildren, cd->displayName(), 0, 0, 0);
            }

            if (ftv) {
               ftv->addContentsItem(hasChildren, cd->displayName(), 0, 0, 0, false, false, cd);
            }
         }

         if (hasChildren) {
            //printf("Class %s at %p visited=%d\n",cd->name().data(),cd,cd->visited);
            bool wasVisited = cd->visited;
            cd->visited = true;
           
            writeClassTree(ol, cd->subClasses(), wasVisited, level + 1, ftv, addToIndex);            
         }

         ol.endIndexListItem();
      }
   }
   if (started) {
      endIndexHierarchy(ol, level);

      if (addToIndex) {
         Doxygen::indexList->decContentsDepth();
      }
      if (ftv) {
         ftv->decContentsDepth();
      }
   }
}

//----------------------------------------------------------------------------

static bool dirHasVisibleChildren(DirDef *dd)
{
   if (dd->hasDocumentation()) {
      return true;
   }

   for (auto fd : *dd->getFiles() ) {
      bool genSourceFile;

      if (fileVisibleInIndex(fd, genSourceFile)) {
         return true;
      }
      if (genSourceFile) {
         return true;
      }
   }

   for (auto subdd : dd->subDirs() ) {
      if (dirHasVisibleChildren(subdd)) {
         return true;
      }
   }
   return false;
}

//----------------------------------------------------------------------------
static void writeDirTreeNode(OutputList &ol, DirDef *dd, int level, FTVHelp *ftv, bool addToIndex)
{
   if (level > 20) {
      warn(dd->getDefFileName(), dd->getDefLine(),
           "maximum nesting level exceeded for directory %s: "
           "check for possible recursive directory relation!\n", dd->name().data()
          );
      return;
   }

   if (!dirHasVisibleChildren(dd)) {
      return;
   }

   static bool tocExpand = true; //Config_getBool("TOC_EXPAND");
   bool isDir = dd->subDirs().count() > 0 ||  // there are subdirs
                (tocExpand &&                 // or toc expand and
                 dd->getFiles() && dd->getFiles()->count() > 0 // there are files
                );

   //printf("gd=`%s': pageDict=%d\n",gd->name().data(),gd->pageDict->count());

   if (addToIndex) {
      Doxygen::indexList->addContentsItem(isDir, dd->shortName(), dd->getReference(), dd->getOutputFileBase(), 0, true, true);
      Doxygen::indexList->incContentsDepth();
   }

   if (ftv) {
      ftv->addContentsItem(isDir, dd->shortName(), dd->getReference(), dd->getOutputFileBase(), 0, false, true, dd);
      ftv->incContentsDepth();
   }

   ol.startIndexListItem();
   ol.startIndexItem(dd->getReference(), dd->getOutputFileBase());
   ol.parseText(dd->shortName());
   ol.endIndexItem(dd->getReference(), dd->getOutputFileBase());

   if (dd->isReference()) {
      ol.startTypewriter();
      ol.docify(" [external]");
      ol.endTypewriter();
   }

   // write sub directories
   if (dd->subDirs().count() > 0) {
      startIndexHierarchy(ol, level + 1);
      
      for (auto subdd : dd->subDirs()) {
         writeDirTreeNode(ol, subdd, level + 1, ftv, addToIndex);
      }
      endIndexHierarchy(ol, level + 1);
   }

   FileList *fileList = dd->getFiles();
   int fileCount = 0;

   if (fileList && fileList->count() > 0) {
      
      for (auto fd : *fileList) {
         bool genSourceFile;

         if (fileVisibleInIndex(fd, genSourceFile)) {
            fileCount++;
         } else if (genSourceFile) {
            fileCount++;
         }
      }

      if (fileCount > 0) {
         startIndexHierarchy(ol, level + 1);
         
         for (auto fd : *fileList) {
            bool doc;
            bool src;

            doc = fileVisibleInIndex(fd, src);  

            QByteArray reference;
            QByteArray outputBase;

            if (doc) {
               reference  = fd->getReference();
               outputBase = fd->getOutputFileBase();
            }

            if (doc || src) {
               ol.startIndexListItem();
               ol.startIndexItem(reference, outputBase);
               ol.parseText(fd->displayName());
               ol.endIndexItem(reference, outputBase);
               ol.endIndexListItem();

               if (ftv && (src || doc)) {
                  ftv->addContentsItem(false, fd->displayName(), reference, outputBase, 0, false, false, fd);
               }
            }
         }
         endIndexHierarchy(ol, level + 1);
      }
   }

   if (tocExpand && addToIndex) {
      // write files of this directory

      if (fileCount > 0) {        
         for (auto fd : *fileList) {
            bool doc;
            bool src;

            doc = fileVisibleInIndex(fd, src);

            if (doc) {
               addMembersToIndex(fd, LayoutDocManager::File, fd->displayName(), QByteArray(), true);

            } else if (src) {
               Doxygen::indexList->addContentsItem(false, convertToHtml(fd->name(), true), 0, fd->getSourceFileBase(), 0, false, true, fd);
            }
         }
      }
   }
   ol.endIndexListItem();

   if (addToIndex) {
      Doxygen::indexList->decContentsDepth();
   }
   if (ftv) {
      ftv->decContentsDepth();
   }
}

static void writeDirHierarchy(OutputList &ol, FTVHelp *ftv, bool addToIndex)
{
   if (ftv) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
   }

   static bool fullPathNames = Config_getBool("FULL_PATH_NAMES");
   startIndexHierarchy(ol, 0);

   if (fullPathNames) {
          
      for (auto dd : Doxygen::directories) {      
         if (dd->getOuterScope() == Doxygen::globalScope) {
            writeDirTreeNode(ol, dd.data(), 0, ftv, addToIndex);
         }
      }
   }

   if (ftv) {
  
      for (auto fn : *Doxygen::inputNameList) { 

         for (auto fd : *fn) {
            static bool fullPathNames = Config_getBool("FULL_PATH_NAMES");

            if (!fullPathNames || fd->getDirDef() == 0) { // top level file
               bool doc, src;
               doc = fileVisibleInIndex(fd, src);
               QByteArray reference, outputBase;

               if (doc) {
                  reference = fd->getReference();
                  outputBase = fd->getOutputFileBase();
               }

               if (doc || src) {
                  ftv->addContentsItem(false, fd->displayName(), reference, outputBase, 0, false, false, fd);
               }

               if (addToIndex) {
                  if (doc) {
                     addMembersToIndex(fd, LayoutDocManager::File, fd->displayName(), QByteArray(), true);

                  } else if (src) {
                     Doxygen::indexList->addContentsItem(
                        false, convertToHtml(fd->name(), true), 0,
                        fd->getSourceFileBase(), 0, false, true, fd);
                  }
               }
            }
         }
      }
   }

   endIndexHierarchy(ol, 0);
   if (ftv) {
      ol.popGeneratorState();
   }
}


//----------------------------------------------------------------------------

static void writeClassTreeForList(OutputList &ol, ClassSDict *cl, bool &started, FTVHelp *ftv, bool addToIndex)
{
    for (auto cd : *cl) {
    
      bool b;      
      b = ! hasVisibleRoot(cd->baseClasses());     

      if (b) { 
         //filter on root classes
         if (cd->isVisibleInHierarchy()) { // should it be visible
            if (!started) {
               startIndexHierarchy(ol, 0);
               if (addToIndex) {
                  Doxygen::indexList->incContentsDepth();
               }
               started = true;
            }

            ol.startIndexListItem();

            bool hasChildren = ! cd->visited && classHasVisibleChildren(cd.data());
          
            if (cd->isLinkable()) {
               //printf("Writing class %s isLinkable()=%d isLinkableInProject()=%d cd->templateMaster()=%p\n",
               //    cd->displayName().data(),cd->isLinkable(),cd->isLinkableInProject(),cd->templateMaster());

               ol.startIndexItem(cd->getReference(), cd->getOutputFileBase());
               ol.parseText(cd->displayName());
               ol.endIndexItem(cd->getReference(), cd->getOutputFileBase());

               if (cd->isReference()) {
                  ol.startTypewriter();
                  ol.docify(" [external]");
                  ol.endTypewriter();
               }

               if (addToIndex) {                  
                  // prevents double insertion in Design Unit List
                  Doxygen::indexList->addContentsItem(hasChildren, cd->displayName(), cd->getReference(), 
                                                         cd->getOutputFileBase(), cd->anchor(), false, false);                  
               }

               if (ftv) {
                  ftv->addContentsItem(hasChildren, cd->displayName(), cd->getReference(), cd->getOutputFileBase(), 
                                       cd->anchor(), false, false, cd.data());
               }

            } else {
               ol.startIndexItem(0, 0);
               ol.parseText(cd->displayName());
               ol.endIndexItem(0, 0);

               if (addToIndex) {
                  Doxygen::indexList->addContentsItem(hasChildren, cd->displayName(), 0, 0, 0, false, false);
               }

               if (ftv) {
                  ftv->addContentsItem(hasChildren, cd->displayName(), 0, 0, 0, false, false, cd.data());
               }
            }
            
            if (hasChildren) {
               writeClassTree(ol, cd->subClasses(), cd->visited, 1, ftv, addToIndex);
               cd->visited = true;
            }

            ol.endIndexListItem();
         }
      }
   }
}

static void writeClassHierarchy(OutputList &ol, FTVHelp *ftv, bool addToIndex)
{
   initClassHierarchy(Doxygen::classSDict);
   initClassHierarchy(Doxygen::hiddenClasses);

   if (ftv) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
   }

   bool started = false;
   writeClassTreeForList(ol, Doxygen::classSDict, started, ftv, addToIndex);
   writeClassTreeForList(ol, Doxygen::hiddenClasses, started, ftv, addToIndex);

   if (started) {
      endIndexHierarchy(ol, 0);
      if (addToIndex) {
         Doxygen::indexList->decContentsDepth();
      }
   }
   if (ftv) {
      ol.popGeneratorState();
   }
}

//----------------------------------------------------------------------------

static int countClassesInTreeList(const ClassSDict &cl)
{
   int count = 0;

   for (auto cd : cl) {
      if (!hasVisibleRoot(cd->baseClasses())) { // filter on root classes
         if (cd->isVisibleInHierarchy()) { // should it be visible
            if (cd->subClasses()) { // should have sub classes
               count++;
            }
         }
      }
   }

   return count;
}

static int countClassHierarchy()
{
   int count = 0;

   initClassHierarchy(Doxygen::classSDict);
   initClassHierarchy(Doxygen::hiddenClasses);

   count += countClassesInTreeList(*Doxygen::classSDict);
   count += countClassesInTreeList(*Doxygen::hiddenClasses);

   return count;
}

//----------------------------------------------------------------------------

static void writeHierarchicalIndex(OutputList &ol)
{
   if (hierarchyClasses == 0) {
      return;
   }

   ol.pushGeneratorState();
   //1.{
   ol.disable(OutputGenerator::Man);

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::ClassHierarchy);

   QByteArray title = lne ? lne->title() : theTranslator->trClassHierarchy();
   bool addToIndex  = lne == 0 || lne->visible();

   startFile(ol, "hierarchy", 0, title, HLI_Hierarchy);
   startTitle(ol, 0);
   ol.parseText(title);
   endTitle(ol, 0, 0);
   ol.startContents();
   ol.startTextBlock();

   if (Config_getBool("HAVE_DOT") && Config_getBool("GRAPHICAL_HIERARCHY")) {
      ol.disable(OutputGenerator::Latex);
      ol.disable(OutputGenerator::RTF);
      ol.startParagraph();
      ol.startTextLink("inherits", 0);
      ol.parseText(theTranslator->trGotoGraphicalHierarchy());
      ol.endTextLink();
      ol.endParagraph();
      ol.enable(OutputGenerator::Latex);
      ol.enable(OutputGenerator::RTF);
   }

   ol.parseText(lne ? lne->intro() : theTranslator->trClassHierarchyDescription());
   ol.endTextBlock();

   // ---------------
   // Static class hierarchy for Latex/RTF
   // ---------------
   ol.pushGeneratorState();
   //2.{
   ol.disable(OutputGenerator::Html);
   Doxygen::indexList->disable();

   writeClassHierarchy(ol, 0, addToIndex);

   Doxygen::indexList->enable();
   ol.popGeneratorState();
   //2.}

   // ---------------
   // Dynamic class hierarchical index for HTML
   // ---------------
   ol.pushGeneratorState();
   //2.{
   ol.disableAllBut(OutputGenerator::Html);

   {
      if (addToIndex) {
         Doxygen::indexList->addContentsItem(true, title, 0, "hierarchy", 0, true, true);
      }

      FTVHelp *ftv = new FTVHelp(false);
      writeClassHierarchy(ol, ftv, addToIndex);

      QByteArray outStr;
      FTextStream t(&outStr);

      ftv->generateTreeViewInline(t);
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);      
      ol.writeString(outStr);

      ol.popGeneratorState();

      delete ftv;
   }

   ol.popGeneratorState();
   //2.}
   // ------

   endFile(ol);
   ol.popGeneratorState();
   //1.}
}

//----------------------------------------------------------------------------

static void writeGraphicalClassHierarchy(OutputList &ol)
{
   if (hierarchyClasses == 0) {
      return;
   }

   ol.disableAllBut(OutputGenerator::Html);
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::ClassHierarchy);

   QByteArray title = lne ? lne->title() : theTranslator->trClassHierarchy();
   startFile(ol, "inherits", 0, title, HLI_Hierarchy, false, "hierarchy");
   startTitle(ol, 0);

   ol.parseText(title);
   endTitle(ol, 0, 0);

   ol.startContents();
   ol.startTextBlock();
   ol.startParagraph();
   ol.startTextLink("hierarchy", 0);
   ol.parseText(theTranslator->trGotoTextualHierarchy());
   ol.endTextLink();
   ol.endParagraph();
   ol.endTextBlock();
   DotGfxHierarchyTable g;
   ol.writeGraphicalHierarchy(g);
   endFile(ol);
   ol.enableAll();
}

//----------------------------------------------------------------------------

static void countFiles(int &htmlFiles, int &files)
{
   htmlFiles = 0;
   files     = 0;

   for (auto fn : *Doxygen::inputNameList) {
     
      for (auto fd : *fn) {
         bool doc, src;
         doc = fileVisibleInIndex(fd, src);

         if (doc || src) {
            htmlFiles++;
         }

         if (doc) {
            files++;
         }
      }
   }
}

static void writeSingleFileIndex(OutputList &ol, FileDef *fd)
{  
   bool doc = fd->isLinkableInProject();
   bool src = fd->generateSourceFile();

   bool nameOk = !fd->isDocumentationFile();

   if (nameOk && (doc || src) && !fd->isReference()) {
      QString path;

      if (Config_getBool("FULL_PATH_NAMES")) {
         path = stripFromPath(fd->getPath());
      }

      QString fullName = fd->name();
      if (!path.isEmpty()) {
         if (path.at(path.length() - 1) != '/') {
            fullName.prepend("/");
         }
         fullName.prepend(path);
      }

      ol.startIndexKey();
      ol.docify(path);

      if (doc) {
         ol.writeObjectLink(0, fd->getOutputFileBase(), 0, fd->name());       

      } else {
         ol.startBold();
         ol.docify(fd->name());
         ol.endBold(); 
      }
      if (src) {
         ol.pushGeneratorState();
         ol.disableAllBut(OutputGenerator::Html);
         ol.docify(" ");
         ol.startTextLink(fd->includeName(), 0);
         ol.docify("[");
         ol.parseText(theTranslator->trCode());
         ol.docify("]");
         ol.endTextLink();
         ol.popGeneratorState();
      }

      ol.endIndexKey();

      bool hasBrief = !fd->briefDescription().isEmpty();
      ol.startIndexValue(hasBrief);

      if (hasBrief) {
         
         ol.generateDoc(
            fd->briefFile(), fd->briefLine(),
            fd, 0,
            fd->briefDescription(true),
            false, // index words
            false, // isExample
            0,     // example name
            true,  // single line
            true   // link from index
         );
         
      }

      ol.endIndexValue(fd->getOutputFileBase(), hasBrief);      
   }
}

//----------------------------------------------------------------------------

static void writeFileIndex(OutputList &ol)
{
   if (documentedHtmlFiles == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Man);
   if (documentedFiles == 0) {
      ol.disableAllBut(OutputGenerator::Html);
   }

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::FileList);
   if (lne == 0) {
      lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Files);   // fall back
   }

   QByteArray title = lne ? lne->title() : theTranslator->trFileList();
   bool addToIndex = (lne == 0 || lne->visible());

   startFile(ol, "files", 0, title, HLI_Files);
   startTitle(ol, 0);
   
   ol.parseText(title);
   endTitle(ol, 0, 0);

   ol.startContents();
   ol.startTextBlock();

   if (addToIndex) {
      Doxygen::indexList->addContentsItem(true, title, 0, "files", 0, true, true);
      Doxygen::indexList->incContentsDepth();
   }

   ol.parseText(lne ? lne->intro() : theTranslator->trFileListDescription(Config_getBool("EXTRACT_ALL")));
   ol.endTextBlock();

   // ---------------
   // Flat file index
   // ---------------

   // 1. {
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);

   SortedList<FileList *> outputNameList;
   QHash<QString, FileList *> outputNameDict;
   
   if (Config_getBool("FULL_PATH_NAMES")) {
     
      for (auto fn : *Doxygen::inputNameList ) {
         
         for (auto fd : *fn) {
            QByteArray path = fd->getPath();

            if (path.isEmpty()) {
               path = "[external]";
            }

            FileList *fl = outputNameDict.value(path);

            if (fl) {
               fl->inSort(fd);
               
            } else {
              
               fl = new FileList(path);
               fl->inSort(fd);

               outputNameList.inSort(fl);
               outputNameDict.insert(path, fl);
            }
         }
      }
   }

   ol.startIndexList();
   if (Config_getBool("FULL_PATH_NAMES")) {
    
      for (auto fl : outputNameList ) {         
         for (auto fd : *fl) {
            writeSingleFileIndex(ol, fd);
         }
      }

   } else {
     
      for (auto fn : *Doxygen::inputNameList) {        
         for (auto fd : *fn) {
            writeSingleFileIndex(ol, fd);
         }
      }
   }
   ol.endIndexList();

   // 1. }
   ol.popGeneratorState();

   // ---------------
   // Hierarchical file index for HTML
   // ---------------
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   FTVHelp *ftv = new FTVHelp(false);
   writeDirHierarchy(ol, ftv, addToIndex);
   QByteArray outStr;

   FTextStream t(&outStr);
   ftv->generateTreeViewInline(t);

   ol.writeString(outStr);
   delete ftv;

   ol.popGeneratorState();
   // ------

   if (addToIndex) {
      Doxygen::indexList->decContentsDepth();
   }

   endFile(ol);
   ol.popGeneratorState();
}

//----------------------------------------------------------------------------
static int countNamespaces()
{
   int count = 0;
 
   for (auto nd : *Doxygen::namespaceSDict) {
      if (nd->isLinkableInProject()) {
         count++;
      }
   }
   return count;
}

//----------------------------------------------------------------------------

void writeClassTree(ClassSDict *clDict, FTVHelp *ftv, bool addToIndex, bool globalOnly)
{
   if (clDict) {
    
      for (auto cd : *clDict) {
        
         if (! globalOnly || cd->getOuterScope() == 0 || cd->getOuterScope() == Doxygen::globalScope ) {
            int count = 0;

            if (cd->getClassSDict()) {
              
               for (auto ccd : *cd->getClassSDict()) {
                  if (ccd->isLinkableInProject() && ccd->templateMaster() == 0) {
                     count++;
                  }
               }
            }

            if (classVisibleInIndex(cd.data()) && cd->templateMaster() == 0) { 

               ftv->addContentsItem(count > 0, cd->displayName(false), cd->getReference(),
                                    cd->getOutputFileBase(), cd->anchor(), false, true, cd.data());

               if (addToIndex && (cd->getOuterScope() == 0 || cd->getOuterScope()->definitionType() != Definition::TypeClass)) {
                  addMembersToIndex(cd.data(), LayoutDocManager::Class, cd->displayName(false), 
                                    cd->anchor(), cd->partOfGroups() == 0 && ! cd->isSimple());
               }

               if (count > 0) {
                  ftv->incContentsDepth();
                  writeClassTree(cd->getClassSDict(), ftv, addToIndex, false);
                  ftv->decContentsDepth();
               }
            }
         }
      }
   }
}

static void writeNamespaceTree(NamespaceSDict *nsDict, FTVHelp *ftv, bool rootOnly, bool showClasses, bool addToIndex)
{
   if (nsDict) {
     
      for (auto nd : *nsDict) {
         if (nd->localName().indexOf('@') == -1 && (! rootOnly || nd->getOuterScope() == Doxygen::globalScope)) {

            bool hasChildren = namespaceHasVisibleChild(nd.data(), showClasses);
            bool isLinkable  = nd->isLinkableInProject();

            QByteArray ref;
            QByteArray file;

            if (isLinkable) {
               ref  = nd->getReference();
               file = nd->getOutputFileBase();              
            }

            if ((isLinkable && !showClasses) || hasChildren) {
               ftv->addContentsItem(hasChildren, nd->localName(), ref, file, 0, false, true, nd.data());

               if (addToIndex) {
                  Doxygen::indexList->addContentsItem(hasChildren, nd->localName(), ref, file, QByteArray(),
                                                      hasChildren && !file.isEmpty(), addToIndex);
               }

               //printf("*** writeNamespaceTree count=%d addToIndex=%d showClasses=%d classCount=%d\n",
               //    count,addToIndex,showClasses,classCount);
               if (hasChildren) {
                  if (addToIndex) {
                     Doxygen::indexList->incContentsDepth();
                  }
                  ftv->incContentsDepth();
                  writeNamespaceTree(nd->getNamespaceSDict(), ftv, false, showClasses, addToIndex);
                  if (showClasses) {
                     writeClassTree(nd->getClassSDict(), ftv, addToIndex, false);
                  }
                  ftv->decContentsDepth();
                  if (addToIndex) {
                     Doxygen::indexList->decContentsDepth();
                  }
               }
            }
         }
      }
   }
}


static void writeNamespaceIndex(OutputList &ol)
{
   if (documentedNamespaces == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Man);
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::NamespaceList);

   if (lne == 0) {
      lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Namespaces);   // fall back
   }

   QByteArray title = lne ? lne->title() : theTranslator->trNamespaceList();
   bool addToIndex = lne == 0 || lne->visible();

   startFile(ol, "namespaces", 0, title, HLI_Namespaces);
   startTitle(ol, 0);
   ol.parseText(title);
   endTitle(ol, 0, 0);

   ol.startContents();
   ol.startTextBlock();
   ol.parseText(lne ? lne->intro() : theTranslator->trNamespaceListDescription(Config_getBool("EXTRACT_ALL")));
   ol.endTextBlock();

   bool first = true;

   // ---------------
   // Linear namespace index for Latex/RTF
   // ---------------
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);

   for (auto nd : *Doxygen::namespaceSDict) {

      if (nd->isLinkableInProject()) {
         if (first) {
            ol.startIndexList();
            first = false;
         }
         
         ol.startIndexKey();         
         ol.writeObjectLink(0, nd->getOutputFileBase(), 0, nd->displayName());         
         ol.endIndexKey();

         bool hasBrief = !nd->briefDescription().isEmpty();
         ol.startIndexValue(hasBrief);

         if (hasBrief) {
            
            ol.generateDoc(
               nd->briefFile(), nd->briefLine(), nd.data(), 0, nd->briefDescription(true),
               false, // index words
               false, // isExample
               0,     // example name
               true,  // single line
               true   // link from index
            );
            
         }
         ol.endIndexValue(nd->getOutputFileBase(), hasBrief);

      }
   }
   if (!first) {
      ol.endIndexList();
   }

   ol.popGeneratorState();

   // ---------------
   // Hierarchical namespace index for HTML
   // ---------------
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   {
      if (addToIndex) {
         Doxygen::indexList->addContentsItem(true, title, 0, "namespaces", 0, true, true);
         Doxygen::indexList->incContentsDepth();
      }

      FTVHelp *ftv = new FTVHelp(false);
      writeNamespaceTree(Doxygen::namespaceSDict, ftv, true, false, addToIndex);

      QByteArray outStr;
      FTextStream t(&outStr);
      ftv->generateTreeViewInline(t);
      ol.writeString(outStr);

      delete ftv;
      if (addToIndex) {
         Doxygen::indexList->decContentsDepth();
      }
   }

   ol.popGeneratorState();
   // ------

   endFile(ol);
   ol.popGeneratorState();
}

//----------------------------------------------------------------------------

static int countAnnotatedClasses(int *cp)
{
   int count = 0;
   int countPrinted = 0;
  
   for (auto cd : *Doxygen::classSDict) {

      if (cd->isLinkableInProject() && cd->templateMaster() == 0) {
         if (!cd->isEmbeddedInOuterScope()) {
            countPrinted++;
         }
         count++;
      }
   }

   *cp = countPrinted;

   return count;
}

static void writeAnnotatedClassList(OutputList &ol)
{
   ol.startIndexList();
   
   for (auto cd : *Doxygen::classSDict) {     
      ol.pushGeneratorState();

      if (cd->isEmbeddedInOuterScope()) {
         ol.disable(OutputGenerator::Latex);
         ol.disable(OutputGenerator::RTF);
      }

      if (cd->isLinkableInProject() && cd->templateMaster() == 0) {
         QByteArray type = cd->compoundTypeString();
         ol.startIndexKey();
        
         ol.writeObjectLink(0, cd->getOutputFileBase(), cd->anchor(), cd->displayName());
         ol.endIndexKey();

         bool hasBrief = !cd->briefDescription().isEmpty();
         ol.startIndexValue(hasBrief);

         if (hasBrief) {
            ol.generateDoc(
               cd->briefFile(), cd->briefLine(), cd.data(), 0, cd->briefDescription(true),
               false,  // indexWords
               false,  // isExample
               0,     // example name
               true,  // single line
               true   // link from index
            );
         }
         ol.endIndexValue(cd->getOutputFileBase(), hasBrief);
    
      }

      ol.popGeneratorState();
   }
   ol.endIndexList();
}

static QByteArray letterToLabel(uint startLetter)
{
   char s[11]; // max 0x12345678 + '\0'

   if (startLetter > 0x20 && startLetter <= 0x7f) { // printable ASCII character
      s[0] = (char)startLetter;
      s[1] = 0;

   } else {
      const char hex[] = "0123456789abcdef";
      int i = 0;

      s[i++] = '0';
      s[i++] = 'x';

      if (startLetter > (1 << 24)) { // 4 byte character
         s[i++] = hex[(startLetter >> 28) & 0xf];
         s[i++] = hex[(startLetter >> 24) & 0xf];
      }
      if (startLetter > (1 << 16)) { // 3 byte character
         s[i++] = hex[(startLetter >> 20) & 0xf];
         s[i++] = hex[(startLetter >> 16) & 0xf];
      }
      if (startLetter > (1 << 8)) { // 2 byte character
         s[i++] = hex[(startLetter >> 12) & 0xf];
         s[i++] = hex[(startLetter >> 8) & 0xf];
      }
      // one byte character
      s[i++] = hex[(startLetter >> 4) & 0xf];
      s[i++] = hex[(startLetter >> 0) & 0xf];
      s[i++] = 0;
   }
   return s;
}

//----------------------------------------------------------------------------

/** Special class list where sorting takes IGNORE_PREFIX into account. */
class PrefixIgnoreClassList : public SortedList<ClassDef *>
{
 public:   
   PrefixIgnoreClassList(uint letter) : m_letter(letter) {}

   void insertDef(ClassDef *d) {
      append(d);
   }

   uint letter() const {
      return m_letter;
   }

 private:
   virtual int compareValue(const ClassDef *c1, const ClassDef *c2) const {
      QByteArray n1 = c1->className();
      QByteArray n2 = c2->className();
      return qstricmp (n1.data() + getPrefixIndex(n1), n2.data() + getPrefixIndex(n2));
   }

   uint m_letter;
};

/** Class representing a cell in the alphabetical class index. */
class AlphaIndexTableCell
{
 public:
   AlphaIndexTableCell(int row, int col, uint letter, ClassDef *cd) :
      m_letter(letter), m_class(cd), m_row(row), m_col(col) 
   {     
   }

   ClassDef *classDef() const {
      return m_class;
   }
   uint letter() const {
      return m_letter;
   }
   int row() const {
      return m_row;
   }
   int column() const {
      return m_col;
   }

 private:
   uint m_letter;
   ClassDef *m_class;
   int m_row;
   int m_col;
};

class UsedIndexLetters : public LongMap<uint>
{
 public:
   UsedIndexLetters() : LongMap<uint>()
   {     
   }

   void add(uint letter) {     
      if (! this->contains(letter)) {
         insert(letter, letter);
      }
   }

 private:
   int compareValues( const uint *p1, const uint *p2) const {
      return (int) * p1 - (int) * p2; 
   }
};

// write an alphabetical index of all class with a header for each letter
static void writeAlphabeticalClassList(OutputList &ol)
{
   // What starting letters are used
   UsedIndexLetters indexLettersUsed;

   // first count the number of headers
   uint startLetter = 0;
   int headerItems  = 0;

   for (auto cd : *Doxygen::classSDict) { 
      if (cd->isLinkableInProject() && cd->templateMaster() == 0) {        
         int index = getPrefixIndex(cd->className());
         
         startLetter = getUtf8CodeToUpper(cd->className(), index);
         indexLettersUsed.add(startLetter);
      }
   }
  
   // write quick link index (row of letters)
   QByteArray alphaLinks = "<div class=\"qindex\">";
 
   for (auto pLetter : indexLettersUsed) {
      if (headerItems) {
         alphaLinks += "&#160;|&#160;";
      }

      headerItems++;
      QByteArray li = letterToLabel(pLetter);
      QByteArray ls = QString(QChar(pLetter)).toUtf8();

      alphaLinks += (QByteArray)"<a class=\"qindex\" href=\"#letter_" + li + "\">" + ls + "</a>";
   }

   alphaLinks += "</div>\n";
   ol.writeString(alphaLinks);

   // the number of columns in the table
   const int columns = Config_getInt("COLS_IN_ALPHA_INDEX");

   int i;
   int j;

   int totalItems = headerItems * 2 + annotatedClasses;    // number of items in the table (headers span 2 items)
   int rows = (totalItems + columns - 1) / columns;        // number of rows in the table

   // Keep a list of classes for each starting letter
   LetterToIndexMap<PrefixIgnoreClassList> classesByLetter;
   QList<QList<AlphaIndexTableCell*> *> tableColumns;

   // fill the columns with the class list (row elements in each column,
   // expect for the columns with number >= itemsInLastRow, which get one item less
  
   startLetter = 0;
         
   for (auto cd : *Doxygen::classSDict) { 

      if (cd->isLinkableInProject() && cd->templateMaster() == 0) {
         int index   = getPrefixIndex(cd->className());
         startLetter = getUtf8Code(cd->className(), index);
                       
         classesByLetter.insertElement(startLetter, cd.data());         
      }
   }

   QList<AlphaIndexTableCell *> *tableRows = new QList<AlphaIndexTableCell *>;
   tableColumns.append(tableRows);

   int col = 0;
   int row = 0;
   int maxRows = 0;

   PrefixIgnoreClassList *cl; 

   for (auto cl : classesByLetter) { 

      uint l = cl->letter();

      // add special header cell
      tableRows->append(new AlphaIndexTableCell(row, col, l, (ClassDef *)0x8));
      row++;

      tableRows->append(new AlphaIndexTableCell(row, col, 0, (ClassDef *)0x8));
      row++;

      //        
      ClassDef *cd = cl->first();     

      tableRows->append(new AlphaIndexTableCell(row, col, 0, cd));
      row++;    
                                          
      if (row > maxRows) {
         maxRows = row;            
      }

      if (row >= rows && col < columns) {                                        
        col++;                                 
        row = 0;                      
        
        tableRows = new QList<AlphaIndexTableCell *>;   
        tableColumns.append(tableRows);        
      }          
              
      for (auto cit = cl->begin() +  1; cit != cl->end(); ++cit)  {
         // add normal cell

         cd = *cit;

         tableRows->append(new AlphaIndexTableCell(row, col, 0, cd));
         row++;

         if (row > maxRows) {
            maxRows = row;            
         }
   
         if (row >= rows && col < columns) {                                        
           col++;                                 
           row = 0;                      
           
           tableRows = new QList<AlphaIndexTableCell *>;   
           tableColumns.append(tableRows);        
         }                     
      }
   }


   ol.writeString("<table style=\"margin: 10px; white-space: nowrap;\" align=\"center\" "
                  "width=\"95%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n");

   // generate table
   for (i = 0; i <= maxRows; i++) { 
      // foreach table row
     
      ol.writeString("<tr>");

      // the last column may contain less items then the others
    
      for (j = 0; j < columns; j++) { 
         // foreach table column
              
         AlphaIndexTableCell *cell = nullptr;

         if (j < tableColumns.count()) {

            if (i < tableColumns[j]->count()) {                
               cell = tableColumns[j]->at(i);
            }
         }

         // CopperSpice: if cells are shifted up, add a nullptr cell in the for loop above

         if (cell) {

            if (cell->row() == i) {

               if (cell->letter() != 0) {
                  QByteArray s = letterToLabel(cell->letter());

                  ol.writeString("<td rowspan=\"2\" valign=\"bottom\">");
                  ol.writeString("<a name=\"letter_");
                  ol.writeString(s);
                  ol.writeString("\"></a>");

                  ol.writeString("<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\">"
                                 "<tr>"
                                 "<td><div class=\"ah\">&#160;&#160;");

                  ol.writeString(QString(QChar(cell->letter())).toUtf8());
                  ol.writeString(         "&#160;&#160;</div>"
                                          "</td>"
                                          "</tr>"
                                          "</table>\n");

               } else if (cell->classDef() != (ClassDef *)0x8) {

                  ClassDef * cd = cell->classDef();

                  ol.writeString("<td valign=\"top\">");
                  QByteArray namesp, cname;                   

                  extractNamespaceName(cd->name(), cname, namesp);
                  QByteArray nsDispName;
                  SrcLangExt lang = cd->getLanguage();

                  QByteArray sep = getLanguageSpecificSeparator(lang);

                  if (sep != "::") {
                     nsDispName = substitute(namesp, "::", sep);
                     cname = substitute(cname, "::", sep);

                  } else {
                     nsDispName = namesp;
                  }

                  ol.writeObjectLink(cd->getReference(), cd->getOutputFileBase(), cd->anchor(), cname);

                  if (! namesp.isEmpty()) {
                     ol.docify(" (");

                     QSharedPointer<NamespaceDef> nd = getResolvedNamespace(namesp);

                     if (nd && nd->isLinkable()) {
                        ol.writeObjectLink(nd->getReference(), nd->getOutputFileBase(), 0, nsDispName);

                     } else {
                        ol.docify(nsDispName);

                     }

                     ol.docify(")");
                  }
                  ol.writeNonBreakableSpace(3);
               }


               if (cell->letter() != 0 || cell->classDef() != (ClassDef *)0x8) {
                  ol.writeString("</td>");
               }

            }

         } else {
            ol.writeString("<td></td>");

         }
         
      }

      ol.writeString("</tr>\n");

   }

   ol.writeString("</table>\n");
   ol.writeString(alphaLinks); 
}

static void writeAlphabeticalIndex(OutputList &ol)
{
   if (annotatedClasses == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::ClassIndex);
   QByteArray title = lne ? lne->title() : theTranslator->trCompoundIndex();
   bool addToIndex  = lne == 0 || lne->visible();

   startFile(ol, "classes", 0, title, HLI_Classes);

   startTitle(ol, 0);
   ol.parseText(title);
   endTitle(ol, 0, 0);

   if (addToIndex) {
      Doxygen::indexList->addContentsItem(false, title, 0, "classes", 0, false, true);
   }

   ol.startContents();

   writeAlphabeticalClassList(ol);
   endFile(ol); // contains ol.endContents()

   ol.popGeneratorState();
}

static void writeAnnotatedIndex(OutputList &ol)
{  
   if (annotatedClasses == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Man);

   if (annotatedClassesPrinted == 0) {
      ol.disable(OutputGenerator::Latex);
      ol.disable(OutputGenerator::RTF);
   }

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::ClassList);
   if (lne == 0) {
      lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Classes);   // fall back
   }

   QByteArray title = lne ? lne->title() : theTranslator->trCompoundList();
   bool addToIndex = lne == 0 || lne->visible();

   startFile(ol, "annotated", 0, title, HLI_Annotated);

   startTitle(ol, 0);
   ol.parseText(title);
   endTitle(ol, 0, 0);

   ol.startContents();

   ol.startTextBlock();
   ol.parseText(lne ? lne->intro() : theTranslator->trCompoundListDescription());
   ol.endTextBlock();

   // ---------------
   // Linear class index for Latex/RTF
   // ---------------
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);
   Doxygen::indexList->disable();

   writeAnnotatedClassList(ol);

   Doxygen::indexList->enable();
   ol.popGeneratorState();

   // ---------------
   // Hierarchical class index for HTML
   // ---------------
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   {
      if (addToIndex) {
         Doxygen::indexList->addContentsItem(true, title, 0, "annotated", 0, true, true);
         Doxygen::indexList->incContentsDepth();
      }

      FTVHelp *ftv = new FTVHelp(false);
      writeNamespaceTree(Doxygen::namespaceSDict, ftv, true, true, addToIndex);
      writeClassTree(Doxygen::classSDict, ftv, addToIndex, true);

      QByteArray outStr;
      FTextStream t(&outStr);

      ftv->generateTreeViewInline(t);
      ol.writeString(outStr);

      delete ftv;
      if (addToIndex) {
         Doxygen::indexList->decContentsDepth();
      }
   }

   ol.popGeneratorState();
   // ------

   endFile(ol); // contains ol.endContents()
   ol.popGeneratorState();
}

//----------------------------------------------------------------------------
static void writeClassLinkForMember(OutputList &ol, MemberDef *md, const char *separator, QString &prevClassName)
{
   ClassDef *cd = md->getClassDef();

   if ( cd && prevClassName != cd->displayName()) {
      ol.docify(separator);
      ol.writeObjectLink(md->getReference(), md->getOutputFileBase(), md->anchor(), cd->displayName());
      ol.writeString("\n");
      prevClassName = cd->displayName();
   }
}

static void writeFileLinkForMember(OutputList &ol, MemberDef *md, const char *separator, QString &prevFileName)
{
   FileDef *fd = md->getFileDef();

   if (fd && prevFileName != fd->name()) {
      ol.docify(separator);
      ol.writeObjectLink(md->getReference(), md->getOutputFileBase(), md->anchor(), fd->name());
      ol.writeString("\n");
      prevFileName = fd->name();
   }
}

static void writeNamespaceLinkForMember(OutputList &ol, MemberDef *md, const char *separator, QString &prevNamespaceName)
{
   NamespaceDef *nd = md->getNamespaceDef();

   if (nd && prevNamespaceName != nd->name()) {
      ol.docify(separator);
      ol.writeObjectLink(md->getReference(), md->getOutputFileBase(), md->anchor(), nd->name());
      ol.writeString("\n");
      prevNamespaceName = nd->name();
   }
}

static void writeMemberList(OutputList &ol, bool useSections, int page, 
                            const LetterToIndexMap<MemberIndexList> &memberLists, DefinitionIntf::DefType type)
{
   int index = (int)type;
   assert(index < 3);

   using writeLinkForMember_t = void (*)(OutputList & ol, MemberDef * md, const char *separator, QString &prevNamespaceName);

   // each index tab has its own write function
   static writeLinkForMember_t   writeLinkForMemberMap[3] = {
      &writeClassLinkForMember,
      &writeFileLinkForMember,
      &writeNamespaceLinkForMember
   };

   QByteArray prevName;
   QString prevDefName;

   bool first = true;
   bool firstSection = true;
   bool firstItem = true;
 
   for (QSharedPointer<MemberIndexList> ml : memberLists) {

      if (page != -1) {
         ml = memberLists[page];
      }

      if (ml == 0 || ml->count() == 0) {
         continue;
      }     
      
      for (auto md : *ml) {
         const char *sep;

         bool isFunc = ! md->isObjCMethod() && (md->isFunction() || md->isSlot() || md->isSignal());

         QByteArray name = md->name();
         int startIndex  = getPrefixIndex(name);

         if (QByteArray(name.data() + startIndex) != prevName) { 
            // new entry

            if ((prevName.isEmpty() || tolower(name.at(startIndex)) != tolower(prevName.at(0))) && useSections) { 
               // new section
               if (!firstItem) {
                  ol.endItemListItem();
               }

               if (!firstSection) {
                  ol.endItemList();
               }
               QByteArray cs = letterToLabel(ml->letter());
               QByteArray cl = QString(QChar(ml->letter())).toUtf8();
               QByteArray anchor = (QByteArray)"index_" + cs;
               QByteArray title = (QByteArray)"- " + cl + " -";

               ol.startSection(anchor, title, SectionInfo::Subsection);
               ol.docify(title);
               ol.endSection(anchor, SectionInfo::Subsection);
               ol.startItemList();

               firstSection = false;
               firstItem = true;

            } else if (!useSections && first) {
               ol.startItemList();
               first = false;
            }

            // member name
            if (!firstItem) {
               ol.endItemListItem();
            }

            ol.startItemListItem();
            firstItem = false;
            ol.docify(name);

            if (isFunc) {
               ol.docify("()");
            }

            ol.writeString("\n");

            // link to class
            prevDefName = "";
            sep = ": ";
            prevName = name.data() + startIndex;

         } else { // same entry
            sep = ", ";
            // link to class for other members with the same name
         }

         if (index < 3) {
            // write the link for the specific list type
            writeLinkForMemberMap[index](ol, md, sep, prevDefName);
         }
      }

      if (page != -1) {
         break;
      }
   }

   if (!firstItem) {
      ol.endItemListItem();
   }
   ol.endItemList();
}

//----------------------------------------------------------------------------

void initClassMemberIndices()
{
   int j = 0;
   for (j = 0; j < CMHL_Total; j++) {
      documentedClassMembers[j] = 0;
      g_memberIndexLetterUsed[j].clear();
   }
}

void addClassMemberNameToIndex(MemberDef *md)
{
   static bool hideFriendCompounds = Config_getBool("HIDE_FRIEND_COMPOUNDS");
   ClassDef *cd = 0;

   if (md->isLinkableInProject() && (cd = md->getClassDef())  && cd->isLinkableInProject() && cd->templateMaster() == 0) {
      QByteArray n = md->name();

      int index   = getPrefixIndex(n);
      uint letter = getUtf8CodeToLower(n, index);

      if (! n.isEmpty()) {

         bool isFriendToHide = hideFriendCompounds && (QByteArray(md->typeString()) == "friend class" ||
                                QByteArray(md->typeString()) == "friend struct" || QByteArray(md->typeString()) == "friend union");

         if (!(md->isFriend() && isFriendToHide) && (!md->isEnumValue() || (md->getEnumScope() && !md->getEnumScope()->isStrong()))) {
            g_memberIndexLetterUsed[CMHL_All].insertElement(letter, md);
            documentedClassMembers[CMHL_All]++;
         }

         if (md->isFunction()  || md->isSlot() || md->isSignal()) {
            g_memberIndexLetterUsed[CMHL_Functions].insertElement(letter, md);
            documentedClassMembers[CMHL_Functions]++;

         } else if (md->isVariable()) {
            g_memberIndexLetterUsed[CMHL_Variables].insertElement(letter, md);
            documentedClassMembers[CMHL_Variables]++;

         } else if (md->isTypedef()) {
            g_memberIndexLetterUsed[CMHL_Typedefs].insertElement(letter, md);
            documentedClassMembers[CMHL_Typedefs]++;

         } else if (md->isEnumerate()) {
            g_memberIndexLetterUsed[CMHL_Enums].insertElement(letter, md);
            documentedClassMembers[CMHL_Enums]++;

         } else if (md->isEnumValue() && md->getEnumScope() && !md->getEnumScope()->isStrong()) {
            g_memberIndexLetterUsed[CMHL_EnumValues].insertElement(letter, md);
            documentedClassMembers[CMHL_EnumValues]++;

         } else if (md->isProperty()) {
            g_memberIndexLetterUsed[CMHL_Properties].insertElement(letter, md);
            documentedClassMembers[CMHL_Properties]++;

         } else if (md->isEvent()) {
            g_memberIndexLetterUsed[CMHL_Events].insertElement(letter, md);
            documentedClassMembers[CMHL_Events]++;

         } else if (md->isRelated() || md->isForeign() || (md->isFriend() && !isFriendToHide)) {
            g_memberIndexLetterUsed[CMHL_Related].insertElement(letter, md);
            documentedClassMembers[CMHL_Related]++;
         }
      }
   }
}

//----------------------------------------------------------------------------

void initNamespaceMemberIndices()
{
   int j = 0;

   for (j = 0; j < NMHL_Total; j++) {
      documentedNamespaceMembers[j] = 0;
      g_namespaceIndexLetterUsed[j].clear();
   }
}

void addNamespaceMemberNameToIndex(MemberDef *md)
{
   NamespaceDef *nd = md->getNamespaceDef();

   if (nd && nd->isLinkableInProject() && md->isLinkableInProject()) {
      QByteArray n = md->name();
      int index = getPrefixIndex(n);
      uint letter = getUtf8CodeToLower(n, index);

      if (!n.isEmpty()) {
         if (!md->isEnumValue() || (md->getEnumScope() && !md->getEnumScope()->isStrong())) {
            g_namespaceIndexLetterUsed[NMHL_All].insertElement(letter, md);
            documentedNamespaceMembers[NMHL_All]++;
         }

         if (md->isFunction()) {
            g_namespaceIndexLetterUsed[NMHL_Functions].insertElement(letter, md);
            documentedNamespaceMembers[NMHL_Functions]++;

         } else if (md->isVariable()) {
            g_namespaceIndexLetterUsed[NMHL_Variables].insertElement(letter, md);
            documentedNamespaceMembers[NMHL_Variables]++;

         } else if (md->isTypedef()) {
            g_namespaceIndexLetterUsed[NMHL_Typedefs].insertElement(letter, md);
            documentedNamespaceMembers[NMHL_Typedefs]++;

         } else if (md->isEnumerate()) {
            g_namespaceIndexLetterUsed[NMHL_Enums].insertElement(letter, md);
            documentedNamespaceMembers[NMHL_Enums]++;

         } else if (md->isEnumValue() && md->getEnumScope() && !md->getEnumScope()->isStrong()) {
            g_namespaceIndexLetterUsed[NMHL_EnumValues].insertElement(letter, md);
            documentedNamespaceMembers[NMHL_EnumValues]++;
         }
      }
   }
}

//----------------------------------------------------------------------------

void initFileMemberIndices()
{
   int j = 0;

   for (j = 0; j < NMHL_Total; j++) {
      documentedFileMembers[j] = 0;
      g_fileIndexLetterUsed[j].clear();
   }
}

void addFileMemberNameToIndex(MemberDef *md)
{
   FileDef *fd = md->getFileDef();

   if (fd && fd->isLinkableInProject() && md->isLinkableInProject()) {
      QByteArray n = md->name();

      int index   = getPrefixIndex(n);
      uint letter = getUtf8CodeToLower(n, index);

      if (! n.isEmpty()) {

         if (!md->isEnumValue() || (md->getEnumScope() && !md->getEnumScope()->isStrong())) {
            g_fileIndexLetterUsed[FMHL_All].insertElement(letter, md);
            documentedFileMembers[FMHL_All]++;
         }

         if (md->isFunction()) {
            g_fileIndexLetterUsed[FMHL_Functions].insertElement(letter, md);
            documentedFileMembers[FMHL_Functions]++;

         } else if (md->isVariable()) {
            g_fileIndexLetterUsed[FMHL_Variables].insertElement(letter, md);
            documentedFileMembers[FMHL_Variables]++;

         } else if (md->isTypedef()) {
            g_fileIndexLetterUsed[FMHL_Typedefs].insertElement(letter, md);
            documentedFileMembers[FMHL_Typedefs]++;

         } else if (md->isEnumerate()) {
            g_fileIndexLetterUsed[FMHL_Enums].insertElement(letter, md);
            documentedFileMembers[FMHL_Enums]++;

         } else if (md->isEnumValue() && md->getEnumScope() && !md->getEnumScope()->isStrong()) {
            g_fileIndexLetterUsed[FMHL_EnumValues].insertElement(letter, md);
            documentedFileMembers[FMHL_EnumValues]++;

         } else if (md->isDefine()) {
            g_fileIndexLetterUsed[FMHL_Defines].insertElement(letter, md);
            documentedFileMembers[FMHL_Defines]++;
         }
      }
   }
}

//----------------------------------------------------------------------------

static void writeQuickMemberIndex(OutputList &ol, const LetterToIndexMap<MemberIndexList> &charUsed, uint page,
                                  QByteArray fullName, bool multiPage)
{
   bool first = true;
   startQuickIndexList(ol, true);

   for (auto ml : charUsed) { 
      uint i = ml->letter();

      QByteArray is = letterToLabel(i);
      QByteArray ci = QString(QChar(i)).toUtf8();
      QByteArray anchor;
      QByteArray extension = Doxygen::htmlFileExtension;

      if (!multiPage) {
         anchor = "#index_";

      } else if (first) {
         anchor = fullName + extension + "#index_";

      } else {
         anchor = fullName + "_" + letterToLabel(i) + extension + "#index_";
      }

      startQuickIndexItem(ol, anchor + ci, i == page, true, first);
      ol.writeString(is);
      endQuickIndexItem(ol);
      first = false;
   }

   endQuickIndexList(ol);
}

//----------------------------------------------------------------------------

/** Helper class representing a class member in the navigation menu. */
struct CmhlInfo {
   CmhlInfo(const char *fn, const char *t) : fname(fn), title(t) {}
   const char *fname;
   QByteArray title;
};

static const CmhlInfo *getCmhlInfo(int hl)
{
   static bool fortranOpt = Config_getBool("OPTIMIZE_FOR_FORTRAN");
   
   static CmhlInfo cmhlInfo[] = {
      CmhlInfo("functions",     theTranslator->trAll()),
      CmhlInfo("functions_func",
      fortranOpt ? theTranslator->trSubprograms() :      
      theTranslator->trFunctions()),
      CmhlInfo("functions_vars", theTranslator->trVariables()),
      CmhlInfo("functions_type", theTranslator->trTypedefs()),
      CmhlInfo("functions_enum", theTranslator->trEnumerations()),
      CmhlInfo("functions_eval", theTranslator->trEnumerationValues()),
      CmhlInfo("functions_prop", theTranslator->trProperties()),
      CmhlInfo("functions_evnt", theTranslator->trEvents()),
      CmhlInfo("functions_rela", theTranslator->trRelatedFunctions())
   };
   return &cmhlInfo[hl];
}

static void writeClassMemberIndexFiltered(OutputList &ol, ClassMemberHighlight hl)
{
   if (documentedClassMembers[hl] == 0) {
      return;
   }

   static bool disableIndex = Config_getBool("DISABLE_INDEX");

   bool multiPageIndex = false;
   if (documentedClassMembers[hl] > MAX_ITEMS_BEFORE_MULTIPAGE_INDEX) {
      multiPageIndex = true;
   }

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   QByteArray extension = Doxygen::htmlFileExtension;
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::ClassMembers);

   QByteArray title = lne ? lne->title() : theTranslator->trCompoundMembers();

   if (hl != CMHL_All) {
      title += (QByteArray)" - " + getCmhlInfo(hl)->title;
   }

   bool addToIndex = lne == 0 || lne->visible();

   if (addToIndex) {
      Doxygen::indexList->addContentsItem(multiPageIndex, getCmhlInfo(hl)->title, 0,
                                          getCmhlInfo(hl)->fname, 0, multiPageIndex, true);
      if (multiPageIndex) {
         Doxygen::indexList->incContentsDepth();
      }
   }

   bool first = true;

   for (auto ml : g_memberIndexLetterUsed[hl]) { 
      uint page = ml->letter();
      QByteArray fileName = getCmhlInfo(hl)->fname;

      if (multiPageIndex) {
         if (!first) {
            fileName += "_" + letterToLabel(page);
         }

         QByteArray cs = QString(QChar(page)).toUtf8();
         if (addToIndex) {
            Doxygen::indexList->addContentsItem(false, cs, 0, fileName, 0, false, true);
         }
      }

      bool quickIndex = documentedClassMembers[hl] > maxItemsBeforeQuickIndex;

      ol.startFile(fileName + extension, 0, title);
      ol.startQuickIndices();

      if (!disableIndex) {
         ol.writeQuickLinks(true, HLI_Functions, 0);
         startQuickIndexList(ol);

         // index item for global member list
         startQuickIndexItem(ol,
                             getCmhlInfo(0)->fname + Doxygen::htmlFileExtension, hl == CMHL_All, true, first);
         ol.writeString(fixSpaces(getCmhlInfo(0)->title));
         endQuickIndexItem(ol);

         int i;
         // index items per category member lists

         for (i = 1; i < CMHL_Total; i++) {
            if (documentedClassMembers[i] > 0) {
               startQuickIndexItem(ol, getCmhlInfo(i)->fname + Doxygen::htmlFileExtension, hl == i, true, first);
               ol.writeString(fixSpaces(getCmhlInfo(i)->title));
               //printf("multiPageIndex=%d first=%d fileName=%s file=%s title=%s\n",
               //    multiPageIndex,first,fileName.data(),getCmhlInfo(i)->fname,getCmhlInfo(i)->title.data());
               endQuickIndexItem(ol);
            }
         }

         endQuickIndexList(ol);

         // quick alphabetical index
         if (quickIndex) {
            writeQuickMemberIndex(ol, g_memberIndexLetterUsed[hl], page, getCmhlInfo(hl)->fname, multiPageIndex);
         }
      }

      ol.endQuickIndices();
      ol.writeSplitBar(fileName);
      ol.writeSearchInfo();

      ol.startContents();

      if (hl == CMHL_All) {
         ol.startTextBlock();
         ol.parseText(lne ? lne->intro() : theTranslator->trCompoundMembersDescription(Config_getBool("EXTRACT_ALL")));
         ol.endTextBlock();
      } else {
         // hack to work around a mozilla bug, which refuses to switch to
         // normal lists otherwise
         ol.writeString("&#160;");
      }

      writeMemberList(ol, quickIndex, multiPageIndex ? page : -1, g_memberIndexLetterUsed[hl], Definition::TypeClass);

      endFile(ol);
      first = false;
   }

   if (multiPageIndex && addToIndex) {
      Doxygen::indexList->decContentsDepth();
   }

   ol.popGeneratorState();
}

static void writeClassMemberIndex(OutputList &ol)
{
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::ClassMembers);
   bool addToIndex = lne == 0 || lne->visible();

   if (documentedClassMembers[CMHL_All] > 0 && addToIndex) {
      Doxygen::indexList->addContentsItem(true, lne ? lne->title() : theTranslator->trCompoundMembers(), 0, "functions", 0);
      Doxygen::indexList->incContentsDepth();
   }

   writeClassMemberIndexFiltered(ol, CMHL_All);
   writeClassMemberIndexFiltered(ol, CMHL_Functions);
   writeClassMemberIndexFiltered(ol, CMHL_Variables);
   writeClassMemberIndexFiltered(ol, CMHL_Typedefs);
   writeClassMemberIndexFiltered(ol, CMHL_Enums);
   writeClassMemberIndexFiltered(ol, CMHL_EnumValues);
   writeClassMemberIndexFiltered(ol, CMHL_Properties);
   writeClassMemberIndexFiltered(ol, CMHL_Events);
   writeClassMemberIndexFiltered(ol, CMHL_Related);
 
  if (documentedClassMembers[CMHL_All] > 0 && addToIndex) {
      Doxygen::indexList->decContentsDepth();
   }

}

//----------------------------------------------------------------------------

/** Helper class representing a file member in the navigation menu. */
struct FmhlInfo {
   FmhlInfo(const char *fn, const char *t) : fname(fn), title(t) {}
   const char *fname;
   QByteArray title;
};

static const FmhlInfo *getFmhlInfo(int hl)
{
   static bool fortranOpt = Config_getBool("OPTIMIZE_FOR_FORTRAN");
  
   static FmhlInfo fmhlInfo[] = {
      FmhlInfo("globals",     theTranslator->trAll()),
      FmhlInfo("globals_func",
      fortranOpt ? theTranslator->trSubprograms()  :      
      theTranslator->trFunctions()),
      FmhlInfo("globals_vars", theTranslator->trVariables()),
      FmhlInfo("globals_type", theTranslator->trTypedefs()),
      FmhlInfo("globals_enum", theTranslator->trEnumerations()),
      FmhlInfo("globals_eval", theTranslator->trEnumerationValues()),
      FmhlInfo("globals_defs", theTranslator->trDefines())
   };

   return &fmhlInfo[hl];
}

static void writeFileMemberIndexFiltered(OutputList &ol, FileMemberHighlight hl)
{
   if (documentedFileMembers[hl] == 0) {
      return;
   }

   static bool disableIndex = Config_getBool("DISABLE_INDEX");

   bool multiPageIndex = false;
   if (documentedFileMembers[hl] > MAX_ITEMS_BEFORE_MULTIPAGE_INDEX) {
      multiPageIndex = true;
   }

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   QByteArray extension = Doxygen::htmlFileExtension;
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::FileGlobals);
   QByteArray title = lne ? lne->title() : theTranslator->trFileMembers();
   bool addToIndex = lne == 0 || lne->visible();

   if (addToIndex) {
      Doxygen::indexList->addContentsItem(multiPageIndex, getFmhlInfo(hl)->title, 0,
                                          getFmhlInfo(hl)->fname, 0, multiPageIndex, true);

      if (multiPageIndex) {
         Doxygen::indexList->incContentsDepth();
      }
   }

   bool first = true;
  
   for (auto ml : g_fileIndexLetterUsed[hl]) {
      uint page = ml->letter();
      QByteArray fileName = getFmhlInfo(hl)->fname;

      if (multiPageIndex) {
         if (!first) {
            fileName += "_" + letterToLabel(page);
         }

         QByteArray cs = QString(QChar(page)).toUtf8();
         if (addToIndex) {
            Doxygen::indexList->addContentsItem(false, cs, 0, fileName, 0, false, true);
         }
      }

      bool quickIndex = documentedFileMembers[hl] > maxItemsBeforeQuickIndex;

      ol.startFile(fileName + extension, 0, title);
      ol.startQuickIndices();

      if (!disableIndex) {
         ol.writeQuickLinks(true, HLI_Globals, 0);
         startQuickIndexList(ol);

         // index item for all file member lists
         startQuickIndexItem(ol,
                             getFmhlInfo(0)->fname + Doxygen::htmlFileExtension, hl == FMHL_All, true, first);
         ol.writeString(fixSpaces(getFmhlInfo(0)->title));
         endQuickIndexItem(ol);

         int i;
         // index items for per category member lists
         for (i = 1; i < FMHL_Total; i++) {
            if (documentedFileMembers[i] > 0) {
               startQuickIndexItem(ol, getFmhlInfo(i)->fname + Doxygen::htmlFileExtension, hl == i, true, first);
               ol.writeString(fixSpaces(getFmhlInfo(i)->title));
               endQuickIndexItem(ol);
            }
         }

         endQuickIndexList(ol);

         if (quickIndex) {
            writeQuickMemberIndex(ol, g_fileIndexLetterUsed[hl], page, getFmhlInfo(hl)->fname, multiPageIndex);
         }
      }

      ol.endQuickIndices();
      ol.writeSplitBar(fileName);
      ol.writeSearchInfo();

      ol.startContents();

      if (hl == FMHL_All) {
         ol.startTextBlock();
         ol.parseText(lne ? lne->intro() : theTranslator->trFileMembersDescription(Config_getBool("EXTRACT_ALL")));
         ol.endTextBlock();

      } else {
         // hack to work around a mozilla bug, which refuses to switch to
         // normal lists otherwise
         ol.writeString("&#160;");
      }

      writeMemberList(ol, quickIndex, multiPageIndex ? page : -1, g_fileIndexLetterUsed[hl], Definition::TypeFile);

      endFile(ol);
      first = false;
   }

   if (multiPageIndex && addToIndex) {
      Doxygen::indexList->decContentsDepth();
   }

   ol.popGeneratorState();
}

static void writeFileMemberIndex(OutputList &ol)
{
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::FileGlobals);
   bool addToIndex = lne == 0 || lne->visible();

   if (documentedFileMembers[FMHL_All] > 0 && addToIndex) {
      Doxygen::indexList->addContentsItem(false, lne ? lne->title() : theTranslator->trFileMembers(), 0, "globals", 0);
      Doxygen::indexList->incContentsDepth();
   }

   writeFileMemberIndexFiltered(ol, FMHL_All);
   writeFileMemberIndexFiltered(ol, FMHL_Functions);
   writeFileMemberIndexFiltered(ol, FMHL_Variables);
   writeFileMemberIndexFiltered(ol, FMHL_Typedefs);
   writeFileMemberIndexFiltered(ol, FMHL_Enums);
   writeFileMemberIndexFiltered(ol, FMHL_EnumValues);
   writeFileMemberIndexFiltered(ol, FMHL_Defines);

   if (documentedFileMembers[FMHL_All] > 0 && addToIndex) {
      Doxygen::indexList->decContentsDepth();
   }

}

//----------------------------------------------------------------------------

/** Helper class representing a namespace member in the navigation menu. */
struct NmhlInfo {
   NmhlInfo(const char *fn, const char *t) : fname(fn), title(t) {}
   const char *fname;
   QByteArray title;
};

static const NmhlInfo *getNmhlInfo(int hl)
{
   static bool fortranOpt = Config_getBool("OPTIMIZE_FOR_FORTRAN");
   
   static NmhlInfo nmhlInfo[] = {
      NmhlInfo("namespacemembers",     theTranslator->trAll()),
      NmhlInfo("namespacemembers_func",
      fortranOpt ? theTranslator->trSubprograms()  :     
      theTranslator->trFunctions()),
      NmhlInfo("namespacemembers_vars", theTranslator->trVariables()),
      NmhlInfo("namespacemembers_type", theTranslator->trTypedefs()),
      NmhlInfo("namespacemembers_enum", theTranslator->trEnumerations()),
      NmhlInfo("namespacemembers_eval", theTranslator->trEnumerationValues())
   };

   return &nmhlInfo[hl];
}

//----------------------------------------------------------------------------

static void writeNamespaceMemberIndexFiltered(OutputList &ol,
      NamespaceMemberHighlight hl)
{
   if (documentedNamespaceMembers[hl] == 0) {
      return;
   }

   static bool disableIndex = Config_getBool("DISABLE_INDEX");

   bool multiPageIndex = false;
   if (documentedNamespaceMembers[hl] > MAX_ITEMS_BEFORE_MULTIPAGE_INDEX) {
      multiPageIndex = true;
   }

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   QByteArray extension = Doxygen::htmlFileExtension;
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::NamespaceMembers);
   QByteArray title = lne ? lne->title() : theTranslator->trNamespaceMembers();
   bool addToIndex = lne == 0 || lne->visible();

   if (addToIndex) {
      Doxygen::indexList->addContentsItem(multiPageIndex, getNmhlInfo(hl)->title, 0,
                                          getNmhlInfo(hl)->fname, 0, multiPageIndex, true);

      if (multiPageIndex) {
         Doxygen::indexList->incContentsDepth();
      }
   }

   bool first = true;

   for (auto ml : g_namespaceIndexLetterUsed[hl]) {

      uint page = ml->letter();
      QByteArray fileName = getNmhlInfo(hl)->fname;

      if (multiPageIndex) {
         if (!first) {
            fileName += "_" + letterToLabel(page);
         }

         QByteArray cs = QString(QChar(page)).toUtf8();
         if (addToIndex) {
            Doxygen::indexList->addContentsItem(false, cs, 0, fileName, 0, false, true);
         }
      }
      bool quickIndex = documentedNamespaceMembers[hl] > maxItemsBeforeQuickIndex;

      ol.startFile(fileName + extension, 0, title);
      ol.startQuickIndices();
      if (!disableIndex) {
         ol.writeQuickLinks(true, HLI_NamespaceMembers, 0);
         startQuickIndexList(ol);

         // index item for all namespace member lists
         startQuickIndexItem(ol,
                             getNmhlInfo(0)->fname + Doxygen::htmlFileExtension, hl == NMHL_All, true, first);
         ol.writeString(fixSpaces(getNmhlInfo(0)->title));
         endQuickIndexItem(ol);

         int i;
         // index items per category member lists
         for (i = 1; i < NMHL_Total; i++) {
            if (documentedNamespaceMembers[i] > 0) {
               startQuickIndexItem(ol,
                                   getNmhlInfo(i)->fname + Doxygen::htmlFileExtension, hl == i, true, first);
               ol.writeString(fixSpaces(getNmhlInfo(i)->title));
               endQuickIndexItem(ol);
            }
         }

         endQuickIndexList(ol);

         if (quickIndex) {
            writeQuickMemberIndex(ol, g_namespaceIndexLetterUsed[hl], page, getNmhlInfo(hl)->fname, multiPageIndex);
         }

      }
      ol.endQuickIndices();
      ol.writeSplitBar(fileName);
      ol.writeSearchInfo();

      ol.startContents();

      if (hl == NMHL_All) {
         ol.startTextBlock();
         ol.parseText(lne ? lne->intro() : theTranslator->trNamespaceMemberDescription(Config_getBool("EXTRACT_ALL")));
         ol.endTextBlock();
      } else {
         // hack to work around a mozilla bug, which refuses to switch to
         // normal lists otherwise
         ol.writeString("&#160;");
      }

      writeMemberList(ol, quickIndex, multiPageIndex ? page : -1, g_namespaceIndexLetterUsed[hl], Definition::TypeNamespace);
      endFile(ol);
   }
   if (multiPageIndex && addToIndex) {
      Doxygen::indexList->decContentsDepth();
   }
   ol.popGeneratorState();
}

static void writeNamespaceMemberIndex(OutputList &ol)
{
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::NamespaceMembers);
   bool addToIndex = lne == 0 || lne->visible();

   if (documentedNamespaceMembers[NMHL_All] > 0 && addToIndex) {
      Doxygen::indexList->addContentsItem(false, lne ? lne->title() : theTranslator->trNamespaceMembers(), 0, "namespacemembers", 0);
      Doxygen::indexList->incContentsDepth();
   }

   //bool fortranOpt = Config_getBool("OPTIMIZE_FOR_FORTRAN");
   writeNamespaceMemberIndexFiltered(ol, NMHL_All);
   writeNamespaceMemberIndexFiltered(ol, NMHL_Functions);
   writeNamespaceMemberIndexFiltered(ol, NMHL_Variables);
   writeNamespaceMemberIndexFiltered(ol, NMHL_Typedefs);
   writeNamespaceMemberIndexFiltered(ol, NMHL_Enums);
   writeNamespaceMemberIndexFiltered(ol, NMHL_EnumValues);

   if (documentedNamespaceMembers[NMHL_All] > 0 && addToIndex) {
      Doxygen::indexList->decContentsDepth();
   }

}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

static void writeExampleIndex(OutputList &ol)
{
   if (Doxygen::exampleSDict->count() == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Man);

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Examples);
   QByteArray title = lne ? lne->title() : theTranslator->trExamples();
   bool addToIndex = lne == 0 || lne->visible();

   startFile(ol, "examples", 0, title, HLI_Examples);

   startTitle(ol, 0);
   ol.parseText(title);
   endTitle(ol, 0, 0);

   ol.startContents();

   if (addToIndex) {
      Doxygen::indexList->addContentsItem(true, title, 0, "examples", 0, true, true);
      Doxygen::indexList->incContentsDepth();
   }

   ol.startTextBlock();
   ol.parseText(lne ? lne->intro() : theTranslator->trExamplesDescription());
   ol.endTextBlock();

   ol.startItemList();
 
   for (auto pd : *Doxygen::exampleSDict) {
      ol.startItemListItem();
      QByteArray n = pd->getOutputFileBase();

      if (!pd->title().isEmpty()) {
         ol.writeObjectLink(0, n, 0, pd->title());
         if (addToIndex) {
            Doxygen::indexList->addContentsItem(false, filterTitle(pd->title()), pd->getReference(), n, 0, false, true);
         }

      } else {
         ol.writeObjectLink(0, n, 0, pd->name());
         if (addToIndex) {
            Doxygen::indexList->addContentsItem(false, pd->name(), pd->getReference(), n, 0, false, true);
         }
      }

      ol.endItemListItem();
      ol.writeString("\n");
   }

   ol.endItemList();

   if (addToIndex) {
      Doxygen::indexList->decContentsDepth();
   }
   endFile(ol);
   ol.popGeneratorState();
}

static void countRelatedPages(int &docPages, int &indexPages)
{
   docPages   = 0;
   indexPages = 0;
 
   for (auto pd : *Doxygen::pageSDict) {

      if ( pd->visibleInIndex()) {
         indexPages++;
      }

      if ( pd->documentedPage()) {
         docPages++;
      }
   }
}

static bool mainPageHasOwnTitle()
{
   static QByteArray projectName = Config_getString("PROJECT_NAME");
   QByteArray title;

   if (Doxygen::mainPage) {
      title = filterTitle(Doxygen::mainPage->title());
   }

   return !projectName.isEmpty() && mainPageHasTitle() && qstricmp(title, projectName) != 0;
}

static void writePages(PageDef *pd, FTVHelp *ftv)
{
   //printf("writePages()=%s pd=%p mainpage=%p\n",pd->name().data(),pd,Doxygen::mainPage);
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Pages);
   bool addToIndex = lne == 0 || lne->visible();

   if (!addToIndex) {
      return;
   }

   bool hasSubPages = pd->hasSubPages();
   bool hasSections = pd->hasSections();

   if (pd->visibleInIndex()) {
      QByteArray pageTitle;

      if (pd->title().isEmpty()) {
         pageTitle = pd->name();
      } else {
         pageTitle = filterTitle(pd->title());
      }

      if (ftv) {
         //printf("*** adding %s\n",pageTitle.data());
         ftv->addContentsItem(
            hasSubPages, pageTitle,
            pd->getReference(), pd->getOutputFileBase(),
            0, hasSubPages, true, pd);
      }
      if (addToIndex && pd != Doxygen::mainPage) {
         Doxygen::indexList->addContentsItem(
            hasSubPages, pageTitle,
            pd->getReference(), pd->getOutputFileBase(),
            0, hasSubPages, true);
      }
   }

   if (hasSubPages && ftv) {
      ftv->incContentsDepth();
   }

   bool doIndent = (hasSections || hasSubPages) && (pd != Doxygen::mainPage || mainPageHasOwnTitle());
   if (doIndent) {
      Doxygen::indexList->incContentsDepth();
   }

   if (hasSections) {
      pd->addSectionsToIndex();
   }

   PageSDict *subPages = pd->getSubPages();

   if (subPages) {    
      for (auto subPage : *subPages ) {
         writePages(subPage.data(), ftv);
      }
   }

   if (hasSubPages && ftv) {
      ftv->decContentsDepth();
   }

   if (doIndent) {
      Doxygen::indexList->decContentsDepth();
   }

   //printf("end writePages()=%s\n",pd->title().data());
}

static void writePageIndex(OutputList &ol)
{
   if (indexedPages == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Pages);

   QByteArray title = lne ? lne->title() : theTranslator->trRelatedPages();
   startFile(ol, "pages", 0, title, HLI_Pages);
  
   startTitle(ol, 0);
   ol.parseText(title);
   endTitle(ol, 0, 0);

   ol.startContents();
   ol.startTextBlock();
   ol.parseText(lne ? lne->intro() : theTranslator->trRelatedPagesDescription());
   ol.endTextBlock();

   {
      FTVHelp *ftv = new FTVHelp(false);    

      for (auto pd : *Doxygen::pageSDict) {
         if ((pd->getOuterScope() == 0 || pd->getOuterScope()->definitionType() != Definition::TypePage) && ! pd->isReference() ) {
            writePages(pd.data(), ftv);
         }
      }

      QByteArray outStr;
      FTextStream t(&outStr);
      ftv->generateTreeViewInline(t);
      ol.writeString(outStr);

      delete ftv;
   }

   endFile(ol);
   ol.popGeneratorState();
}

static int countGroups()
{
   int count = 0;
  
   for (auto gd : *Doxygen::groupSDict)   { 
      if (! gd->isReference()) {
         gd->visited = false;
         count++;
      }
   }
   return count;
}

static int countDirs()
{
   int count = 0;

   for (auto dd : Doxygen::directories) {
      if (dd->isLinkableInProject()) {
         dd->visited = false;
         count++;
      }
   }

   return count;
}

void writeGraphInfo(OutputList &ol)
{
   if (!Config_getBool("HAVE_DOT") || !Config_getBool("GENERATE_HTML")) {
      return;
   }

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
   generateGraphLegend(Config_getString("HTML_OUTPUT"));

   bool &stripCommentsStateRef = Config_getBool("STRIP_CODE_COMMENTS");
   bool oldStripCommentsState = stripCommentsStateRef;
   bool &createSubdirs = Config_getBool("CREATE_SUBDIRS");
   bool oldCreateSubdirs = createSubdirs;

   // temporarily disable the stripping of comments for our own code example!
   stripCommentsStateRef = false;

   // temporarily disable create subdirs for linking to our example
   createSubdirs = false;

   startFile(ol, "graph_legend", 0, theTranslator->trLegendTitle().data());
   startTitle(ol, 0);
   ol.parseText(theTranslator->trLegendTitle());
   endTitle(ol, 0, 0);
   ol.startContents();

   QByteArray legendDocs = theTranslator->trLegendDocs();
   int s = legendDocs.indexOf("<center>");
   int e = legendDocs.indexOf("</center>");

   if (Config_getEnum("DOT_IMAGE_FORMAT") == "svg" && s != -1 && e != -1) {
      legendDocs = legendDocs.left(s + 8) + "[!-- SVG 0 --]\n" + legendDocs.mid(e);
      //printf("legendDocs=%s\n",legendDocs.data());
   }
   FileDef fd("", "graph_legend");
   ol.generateDoc("graph_legend", 1, &fd, 0, legendDocs, false, false);

   // restore config settings
   stripCommentsStateRef = oldStripCommentsState;
   createSubdirs = oldCreateSubdirs;

   endFile(ol);
   ol.popGeneratorState();
}

/*!
 * write groups as hierarchical trees
 */
static void writeGroupTreeNode(OutputList &ol, GroupDef *gd, int level, FTVHelp *ftv, bool addToIndex)
{
   //bool fortranOpt = Config_getBool("OPTIMIZE_FOR_FORTRAN");
   
   if (level > 20) {
      warn(gd->getDefFileName(), gd->getDefLine(),
           "maximum nesting level exceeded for group %s: check for possible recursive group relation!\n", gd->name().data()
          );
      return;
   }

   /* Some groups should appear twice under different parent-groups.
    * That is why we should not check if it was visited
    */
   if (/*!gd->visited &&*/ (!gd->isASubGroup() || level > 0) && gd->isVisible() &&
                           (!gd->isReference() || Config_getBool("EXTERNAL_GROUPS")) ) {

      //printf("gd->name()=%s #members=%d\n",gd->name().data(),gd->countMembers());
      // write group info

      bool hasSubGroups = gd->getSubGroups()->count() > 0;
      bool hasSubPages  = gd->getPages()->count() > 0;
      int numSubItems   = 0;
         
      for (auto ml : gd->getMemberLists()) {
         if (ml.listType() & MemberListType_documentationLists) {
            numSubItems += ml.count();
         }
      }

      numSubItems += gd->getNamespaces()->count();
      numSubItems += gd->getClasses()->count();
      numSubItems += gd->getFiles()->count();
      numSubItems += gd->getDirs()->count();
      numSubItems += gd->getPages()->count();
    

      bool isDir = hasSubGroups || hasSubPages || numSubItems > 0;
      //printf("gd=`%s': pageDict=%d\n",gd->name().data(),gd->pageDict->count());

      if (addToIndex) {
         Doxygen::indexList->addContentsItem(isDir, gd->groupTitle(), gd->getReference(), gd->getOutputFileBase(), 0, isDir, true);
         Doxygen::indexList->incContentsDepth();
      }

      if (ftv) {
         ftv->addContentsItem(hasSubGroups, gd->groupTitle(), gd->getReference(), gd->getOutputFileBase(), 0, false, false, gd);
         ftv->incContentsDepth();
      }
  
      ol.startIndexListItem();
      ol.startIndexItem(gd->getReference(), gd->getOutputFileBase());
      ol.parseText(gd->groupTitle());
      ol.endIndexItem(gd->getReference(), gd->getOutputFileBase());

      if (gd->isReference()) {
         ol.startTypewriter();
         ol.docify(" [external]");
         ol.endTypewriter();
      }
 
      for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Group)) {   

         if (lde->kind() == LayoutDocEntry::MemberDef && addToIndex) {

            LayoutDocEntryMemberDef *lmd = (LayoutDocEntryMemberDef *)lde;
            MemberList *ml = gd->getMemberList(lmd->type);

            if (ml) {
               for (auto md : *ml) {
                  MemberList *enumList = md->enumFieldList();
                  bool isDir = enumList != 0 && md->isEnumerate();

                  if (md->isVisible() && md->name().indexOf('@') == -1) {
                     Doxygen::indexList->addContentsItem(isDir, md->name(), md->getReference(), md->getOutputFileBase(), md->anchor(), false, addToIndex);
                  }

                  if (isDir) {
                     Doxygen::indexList->incContentsDepth();
                    
                     for (auto emd : *enumList) {
                        if (emd->isVisible()) {
                           Doxygen::indexList->addContentsItem(false, emd->name(), emd->getReference(), emd->getOutputFileBase(),
                                                               emd->anchor(), false, addToIndex);
                        }
                     }

                     Doxygen::indexList->decContentsDepth();
                  }
               }
            }

         } else if (lde->kind() == LayoutDocEntry::GroupClasses && addToIndex) {
            
            for (auto cd : *gd->getClasses()) {
                if (cd->isVisible() /*&& !nestedClassInSameGroup*/) {                
                  addMembersToIndex(cd.data(), LayoutDocManager::Class, cd->displayName(false), cd->anchor(), addToIndex, true);                 
               }
            }

         } else if (lde->kind() == LayoutDocEntry::GroupNamespaces && addToIndex) {
            
            for (auto nd : *gd->getNamespaces()) {
               if (nd->isVisible()) {
                  Doxygen::indexList->addContentsItem(false, nd->localName(), nd->getReference(),
                                                      nd->getOutputFileBase(), 0, false, false);
               }
            }

         } else if (lde->kind() == LayoutDocEntry::GroupFiles && addToIndex) {
           
            for (auto fd : *gd->getFiles()) {
               if (fd->isVisible()) {
                  Doxygen::indexList->addContentsItem(false, fd->displayName(), fd->getReference(),
                                                      fd->getOutputFileBase(), 0, false, false);
               }
            }

         } else if (lde->kind() == LayoutDocEntry::GroupDirs && addToIndex) {
         
            for (auto dd : *gd->getDirs()) {
               if (dd->isVisible()) {
                  Doxygen::indexList->addContentsItem(false, dd->shortName(), dd->getReference(), 
                                                      dd->getOutputFileBase(), 0, false, false);
               }
            }

         } else if (lde->kind() == LayoutDocEntry::GroupPageDocs && addToIndex) {
       
            for (auto pd : *gd->getPages()) {
               QSharedPointer<SectionInfo> si;

               if (! pd->name().isEmpty()) {
                  si = Doxygen::sectionDict->find(pd->name());
               }

               bool hasSubPages = pd->hasSubPages();
               bool hasSections = pd->hasSections();

               Doxygen::indexList->addContentsItem( hasSubPages || hasSections, convertToHtml(pd->title(), true), gd->getReference(),
                  gd->getOutputFileBase(), si ? si->label.data() : 0, hasSubPages || hasSections, true); 
      
               // addToNavIndex

               if (hasSections || hasSubPages) {
                  Doxygen::indexList->incContentsDepth();
               }
               if (hasSections) {
                  pd->addSectionsToIndex();
               }

               writePages(pd.data(), 0);
               if (hasSections || hasSubPages) {
                  Doxygen::indexList->decContentsDepth();
               }
            }

         } else if (lde->kind() == LayoutDocEntry::GroupNestedGroups) {

            if (gd->getSubGroups()->count() > 0) {
               startIndexHierarchy(ol, level + 1);
             
               for (auto subgd : *gd->getSubGroups()) {
                  writeGroupTreeNode(ol, subgd, level + 1, ftv, addToIndex);
               }

               endIndexHierarchy(ol, level + 1);
            }
         }
      }

      ol.endIndexListItem();

      if (addToIndex) {
         Doxygen::indexList->decContentsDepth();
      }
      if (ftv) {
         ftv->decContentsDepth();
      }
      //gd->visited=true;
   }
}

static void writeGroupHierarchy(OutputList &ol, FTVHelp *ftv, bool addToIndex)
{
   if (ftv) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
   }

   startIndexHierarchy(ol, 0);

   for (auto gd : *Doxygen::groupSDict) {
      writeGroupTreeNode(ol, gd.data(), 0, ftv, addToIndex);
   }

   endIndexHierarchy(ol, 0);

   if (ftv) {
      ol.popGeneratorState();
   }
}

//----------------------------------------------------------------------------

static void writeGroupIndex(OutputList &ol)
{
   if (documentedGroups == 0) {
      return;
   }

   ol.pushGeneratorState();
   // 1.{
   ol.disable(OutputGenerator::Man);
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Modules);
   QByteArray title = lne ? lne->title() : theTranslator->trModules();
   bool addToIndex = lne == 0 || lne->visible();

   startFile(ol, "modules", 0, title, HLI_Modules);
   startTitle(ol, 0);
   ol.parseText(title);
   endTitle(ol, 0, 0);
   ol.startContents();
   ol.startTextBlock();
   ol.parseText(lne ? lne->intro() : theTranslator->trModulesDescription());
   ol.endTextBlock();

   // ---------------
   // Normal group index for Latex/RTF
   // ---------------
   // 2.{
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);
   Doxygen::indexList->disable();

   writeGroupHierarchy(ol, 0, false);

   Doxygen::indexList->enable();
   ol.popGeneratorState();
   // 2.}

   // ---------------
   // interactive group index for HTML
   // ---------------
   // 2.{
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   {
      if (addToIndex) {
         Doxygen::indexList->addContentsItem(true, title, 0, "modules", 0, true, true);
         Doxygen::indexList->incContentsDepth();
      }

      FTVHelp *ftv = new FTVHelp(false);
      writeGroupHierarchy(ol, ftv, addToIndex);
      QByteArray outStr;
      FTextStream t(&outStr);
      ftv->generateTreeViewInline(t);
      ol.disableAllBut(OutputGenerator::Html);
      ol.writeString(outStr);

      delete ftv;
      if (addToIndex) {
         Doxygen::indexList->decContentsDepth();
      }
   }
   ol.popGeneratorState();
   // 2.}

   endFile(ol);
   ol.popGeneratorState();
   // 1.}
}

static void writeUserGroupStubPage(OutputList &ol, LayoutNavEntry *lne)
{
   if (lne->baseFile().left(9) == "usergroup") {
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);
      startFile(ol, lne->baseFile(), 0, lne->title(), HLI_UserGroup);
      startTitle(ol, 0);

      ol.parseText(lne->title());
      endTitle(ol, 0, 0);

      ol.startContents();
   
      int count = 0;

      for (auto entry : lne->children()) {
         if (entry->visible()) {
            count++;
         }
      }

      if (count > 0) {
         ol.writeString("<ul>\n");

         for (auto entry : lne->children()) {
            if (entry->visible()) { 
               ol.writeString("<li><a href=\"" + entry->url() + "\"><span>" + fixSpaces(entry->title()) + "</span></a></li>\n");
            }
         }

         ol.writeString("</ul>\n");
      }
      endFile(ol);
      ol.popGeneratorState();
   }
}

static void writeIndex(OutputList &ol)
{
   static bool fortranOpt = Config_getBool("OPTIMIZE_FOR_FORTRAN");   
   static QByteArray projectName = Config_getString("PROJECT_NAME");

   // save old generator state
   ol.pushGeneratorState();

   QByteArray projPrefix;
   if (!projectName.isEmpty()) {
      projPrefix = projectName + " ";
   }

   // write HTML index
   ol.disableAllBut(OutputGenerator::Html);

   QByteArray defFileName = Doxygen::mainPage ? Doxygen::mainPage->docFile().data() : "[generated]";
   int defLine = Doxygen::mainPage ? Doxygen::mainPage->docLine() : -1;

   QByteArray title;

   if (!mainPageHasTitle()) {
      title = theTranslator->trMainPage();
   } else {
      title = filterTitle(Doxygen::mainPage->title());
   }

   QByteArray indexName = "index";
   ol.startFile(indexName, 0, title);

   if (Doxygen::mainPage) {
      if ( (! projectName.isEmpty() && mainPageHasTitle() && qstricmp(title, projectName) != 0)) { 
         Doxygen::indexList->addContentsItem(Doxygen::mainPage->hasSubPages(), title, 0, indexName, 0, Doxygen::mainPage->hasSubPages(), true);
      }

      // to avoid duplicate entries in the treeview

      if (Doxygen::mainPage->hasSubPages() || Doxygen::mainPage->hasSections()) {
         writePages(Doxygen::mainPage, 0);
      }
   }

   ol.startQuickIndices();
   if (!Config_getBool("DISABLE_INDEX")) {
      ol.writeQuickLinks(true, HLI_Main, 0);
   }

   ol.endQuickIndices();
   ol.writeSplitBar(indexName);
   ol.writeSearchInfo();

   bool headerWritten = false;

   if (Doxygen::mainPage && ! Doxygen::mainPage->title().isEmpty()) {

      if (Doxygen::mainPage->title().toLower() != "notitle") {
         ol.startHeaderSection(); 
         ol.startTitleHead(0);

         ol.generateDoc(Doxygen::mainPage->docFile(), Doxygen::mainPage->docLine(),
                        Doxygen::mainPage, 0, Doxygen::mainPage->title(), true, false, 0, true, false);

         headerWritten = true;
      }

   } else {

      if (! projectName.isEmpty()) {

         ol.startHeaderSection();
         ol.startTitleHead(0);
         ol.parseText(projPrefix + theTranslator->trDocumentation());

         headerWritten = true;
      }
   }

   if (headerWritten) {
      ol.endTitleHead(0, 0);
      ol.endHeaderSection();
   }

   ol.startContents();
   if (Config_getBool("DISABLE_INDEX") && Doxygen::mainPage == 0) {
      ol.writeQuickLinks(false, HLI_Main, 0);
   }
 
   if (Doxygen::mainPage) {
      Doxygen::insideMainPage = true;
      if (Doxygen::mainPage->showToc() && Doxygen::mainPage->hasSections()) {
         Doxygen::mainPage->writeToc(ol);
      }

      ol.startTextBlock();
      ol.generateDoc(defFileName, defLine, Doxygen::mainPage, 0,
                     Doxygen::mainPage->documentation(), true, false);

      ol.endTextBlock();

      Doxygen::insideMainPage = false;
   }

   endFile(ol);
   ol.disable(OutputGenerator::Html);

   // write Latex & Rtf index
   ol.enable(OutputGenerator::Latex);
   ol.enable(OutputGenerator::RTF);

   ol.startFile("refman", 0, 0);
   ol.startIndexSection(isTitlePageStart);
   if (!Config_getString("LATEX_HEADER").isEmpty()) {
      ol.disable(OutputGenerator::Latex);
   }

   if (projPrefix.isEmpty()) {
      ol.parseText(theTranslator->trReferenceManual());
   } else {
      ol.parseText(projPrefix);
   }

   if (!Config_getString("PROJECT_NUMBER").isEmpty()) {
      ol.startProjectNumber();
      ol.generateDoc(defFileName, defLine, Doxygen::mainPage, 0, Config_getString("PROJECT_NUMBER"), false, false);
      ol.endProjectNumber();
   }

   ol.endIndexSection(isTitlePageStart);
   ol.startIndexSection(isTitlePageAuthor);
   ol.parseText(theTranslator->trGeneratedBy());
   ol.endIndexSection(isTitlePageAuthor);
   ol.enable(OutputGenerator::Latex);

   ol.lastIndexPage();
   if (Doxygen::mainPage) {
      ol.startIndexSection(isMainPage);
      if (mainPageHasTitle()) {
         ol.parseText(Doxygen::mainPage->title());
      } else {
         ol.parseText(/*projPrefix+*/theTranslator->trMainPage());
      }
      ol.endIndexSection(isMainPage);
   }

   if (documentedPages > 0) {  
      bool first = Doxygen::mainPage == 0;
     
      for (auto pd : *Doxygen::pageSDict) {

         if (! pd->getGroupDef() && !pd->isReference() && (!pd->hasParentPage() || (Doxygen::mainPage == pd->getOuterScope())) ) {
            bool isCitationPage = pd->name() == "citelist";

            if (isCitationPage) {
               // For LaTeX the bibliograph is already written by \bibliography
               ol.pushGeneratorState();
               ol.disable(OutputGenerator::Latex);
            }

            QByteArray title = pd->title();
            if (title.isEmpty()) {
               title = pd->name();
            }

            ol.startIndexSection(isPageDocumentation);
            ol.parseText(title);
            ol.endIndexSection(isPageDocumentation);

            ol.pushGeneratorState(); // write TOC title (RTF only)
            ol.disableAllBut(OutputGenerator::RTF);
            ol.startIndexSection(isPageDocumentation2);
            ol.parseText(title);
            ol.endIndexSection(isPageDocumentation2);
            ol.popGeneratorState();

            ol.writeAnchor(0, pd->getOutputFileBase());

            ol.writePageLink(pd->getOutputFileBase(), first);
            first = false;

            if (isCitationPage) {
               ol.popGeneratorState();
            }
         }
      }
   }

   if (! Config_getBool("LATEX_HIDE_INDICES")) {      

      if (documentedGroups > 0) {
         ol.startIndexSection(isModuleIndex);
         ol.parseText(/*projPrefix+*/ theTranslator->trModuleIndex());
         ol.endIndexSection(isModuleIndex);
      }

      if (documentedNamespaces > 0) {
         ol.startIndexSection(isNamespaceIndex);
         ol.parseText(/*projPrefix+*/(fortranOpt ? theTranslator->trModulesIndex() : theTranslator->trNamespaceIndex()));
         ol.endIndexSection(isNamespaceIndex);
      }

      if (hierarchyClasses > 0) {
         ol.startIndexSection(isClassHierarchyIndex);
         ol.parseText(/*projPrefix+*/
            (fortranOpt ? theTranslator->trCompoundIndexFortran() :             
             theTranslator->trHierarchicalIndex()
            ));
         ol.endIndexSection(isClassHierarchyIndex);
      }

      if (annotatedClassesPrinted > 0) {
         ol.startIndexSection(isCompoundIndex);
         ol.parseText(/*projPrefix+*/
            (fortranOpt ? theTranslator->trCompoundIndexFortran() :             
             theTranslator->trCompoundIndex()
            ));
         ol.endIndexSection(isCompoundIndex);
      }
      if (documentedFiles > 0) {
         ol.startIndexSection(isFileIndex);
         ol.parseText(/*projPrefix+*/theTranslator->trFileIndex());
         ol.endIndexSection(isFileIndex);
      }
   }
   if (documentedGroups > 0) {
      ol.startIndexSection(isModuleDocumentation);
      ol.parseText(/*projPrefix+*/theTranslator->trModuleDocumentation());
      ol.endIndexSection(isModuleDocumentation);
   }
   if (documentedNamespaces > 0) {
      ol.startIndexSection(isNamespaceDocumentation);
      ol.parseText(/*projPrefix+*/(fortranOpt ? theTranslator->trModuleDocumentation() : theTranslator->trNamespaceDocumentation()));
      ol.endIndexSection(isNamespaceDocumentation);
   }
   if (annotatedClassesPrinted > 0) {
      ol.startIndexSection(isClassDocumentation);
      ol.parseText(/*projPrefix+*/(fortranOpt ? theTranslator->trTypeDocumentation() : theTranslator->trClassDocumentation()));
      ol.endIndexSection(isClassDocumentation);
   }
   if (documentedFiles > 0) {
      ol.startIndexSection(isFileDocumentation);
      ol.parseText(/*projPrefix+*/theTranslator->trFileDocumentation());
      ol.endIndexSection(isFileDocumentation);
   }
   if (Doxygen::exampleSDict->count() > 0) {
      ol.startIndexSection(isExampleDocumentation);
      ol.parseText(/*projPrefix+*/theTranslator->trExampleDocumentation());
      ol.endIndexSection(isExampleDocumentation);
   }
   ol.endIndexSection(isEndIndex);
   endFile(ol);

   if (Doxygen::mainPage) {
      Doxygen::insideMainPage = true;
      ol.disable(OutputGenerator::Man);
      startFile(ol, Doxygen::mainPage->name(), 0, Doxygen::mainPage->title());
      ol.startContents();
      ol.startTextBlock();

      ol.generateDoc(defFileName, defLine, Doxygen::mainPage, 0,
                     Doxygen::mainPage->documentation(), false, false);

      ol.endTextBlock();
      endFile(ol);
      ol.enable(OutputGenerator::Man);
      Doxygen::insideMainPage = false;
   }

   ol.popGeneratorState();
}

static QVector<bool> indexWritten;

static void writeIndexHierarchyEntries(OutputList &ol, const QList<LayoutNavEntry *> &entries)
{   
   for (auto lne : entries) { 
      LayoutNavEntry::Kind kind = lne->kind();
      uint index = (uint)kind;

      if (index >= indexWritten.size()) {
         uint i;
         uint oldSize = indexWritten.size();
         uint newSize = index + 1;

         indexWritten.resize(newSize);

         for (i = oldSize; i < newSize; i++) {   
            indexWritten[i] = false;
         }
      }
      
      bool addToIndex   = (lne == 0 || lne->visible());
      bool needsClosing = false;

      if (! indexWritten.at(index)) {
         switch (kind) {
            case LayoutNavEntry::MainPage:
               msg("Generating index page\n");
               writeIndex(ol);
               break;

            case LayoutNavEntry::Pages:
               msg("Generating page index\n");
               writePageIndex(ol);
               break;

            case LayoutNavEntry::Modules:
               msg("Generating module index\n");
               writeGroupIndex(ol);
               break;

            case LayoutNavEntry::Namespaces: {
               static bool showNamespaces = Config_getBool("SHOW_NAMESPACES");
               if (showNamespaces) {
                  if (documentedNamespaces > 0 && addToIndex) {
                     Doxygen::indexList->addContentsItem(true, lne->title(), 0, 0, 0);
                     Doxygen::indexList->incContentsDepth();
                     needsClosing = true;
                  }

                  if (LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Namespaces) != lne) {
                      // for backward compatibility with old layout file
                     msg("Generating namespace index\n");
                     writeNamespaceIndex(ol);
                  }
               }
            }
            break;

            case LayoutNavEntry::NamespaceList: {
               static bool showNamespaces = Config_getBool("SHOW_NAMESPACES");
               if (showNamespaces) {
                  msg("Generating namespace index\n");
                  writeNamespaceIndex(ol);
               }
            }
            break;

            case LayoutNavEntry::NamespaceMembers:
               msg("Generating namespace member index\n");
               writeNamespaceMemberIndex(ol);
               break;

            case LayoutNavEntry::Classes:
               if (annotatedClasses > 0 && addToIndex) {
                  Doxygen::indexList->addContentsItem(true, lne->title(), 0, 0, 0);
                  Doxygen::indexList->incContentsDepth();
                  needsClosing = true;
               }

               if (LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Classes) != lne) { 
                  // for backward compatibility with old layout file
                  msg("Generating annotated compound index\n");
                  writeAnnotatedIndex(ol);
               }

               break;

            case LayoutNavEntry::ClassList:
               msg("Generating annotated compound index\n");
               writeAnnotatedIndex(ol);
               break;

            case LayoutNavEntry::ClassIndex:
               msg("Generating alphabetical compound index\n");
               writeAlphabeticalIndex(ol);
               break;

            case LayoutNavEntry::ClassHierarchy:
               msg("Generating hierarchical class index\n");
               writeHierarchicalIndex(ol);
               if (Config_getBool("HAVE_DOT") && Config_getBool("GRAPHICAL_HIERARCHY")) {
                  msg("Generating graphical class hierarchy\n");
                  writeGraphicalClassHierarchy(ol);
               }
               break;

            case LayoutNavEntry::ClassMembers:
               msg("Generating member index\n");
               writeClassMemberIndex(ol);
               break;

            case LayoutNavEntry::Files: {
               static bool showFiles = Config_getBool("SHOW_FILES");
               if (showFiles) {
                  if (documentedHtmlFiles > 0 && addToIndex) {
                     Doxygen::indexList->addContentsItem(true, lne->title(), 0, 0, 0);
                     Doxygen::indexList->incContentsDepth();
                     needsClosing = true;
                  }

                  if (LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Files) != lne) { 
                     // for backward compatibility with old layout file
                     msg("Generating file index\n");
                     writeFileIndex(ol);
                  }
               }
            }
            break;
            case LayoutNavEntry::FileList: {
               static bool showFiles = Config_getBool("SHOW_FILES");
               if (showFiles) {
                  msg("Generating file index\n");
                  writeFileIndex(ol);
               }
            }
            break;
            case LayoutNavEntry::FileGlobals:
               msg("Generating file member index\n");
               writeFileMemberIndex(ol);
               break;
            case LayoutNavEntry::Examples:
               msg("Generating example index\n");
               writeExampleIndex(ol);
               break;
            case LayoutNavEntry::User: {
               // prepend a ! or ^ marker to the URL to avoid tampering with it
               QByteArray url = correctURL(lne->url(), "!"); // add ! to relative URL
               bool isRelative = url.at(0) == '!';
               if (!url.isEmpty() && !isRelative) { // absolute URL
                  url.prepend("^"); // prepend ^ to absolute URL
               }
               bool isRef = lne->baseFile().left(4) == "@ref" || lne->baseFile().left(4) == "\\ref";
               Doxygen::indexList->addContentsItem(true, lne->title(), 0, url, 0, false, isRef || isRelative);
            }
            break;
            case LayoutNavEntry::UserGroup:
               if (addToIndex) {
                  QByteArray url = correctURL(lne->url(), "!"); // add ! to relative URL
                  if (!url.isEmpty()) {
                     if (url == "![none]") {
                        Doxygen::indexList->addContentsItem(true, lne->title(), 0, 0, 0, false, false);
                     } else {
                        bool isRelative = url.at(0) == '!';
                        if (!isRelative) { // absolute URL
                           url.prepend("^"); // prepend ^ to absolute URL
                        }
                        bool isRef = lne->baseFile().left(4) == "@ref" || lne->baseFile().left(4) == "\\ref";
                        Doxygen::indexList->addContentsItem(true, lne->title(), 0, url, 0, false, isRef || isRelative);
                     }
                  } else {
                     Doxygen::indexList->addContentsItem(true, lne->title(), 0, lne->baseFile(), 0, true, true);
                  }
                  Doxygen::indexList->incContentsDepth();
                  needsClosing = true;
               }
               writeUserGroupStubPage(ol, lne);
               break;
         }

         if (kind != LayoutNavEntry::User && kind != LayoutNavEntry::UserGroup) { // User entry may appear multiple times
            indexWritten[index] = true;
         }
      }

      writeIndexHierarchyEntries(ol, lne->children());

      if (needsClosing) {
         switch (kind) {
            case LayoutNavEntry::Namespaces:
            case LayoutNavEntry::Classes:
            case LayoutNavEntry::Files:
            case LayoutNavEntry::UserGroup:
               Doxygen::indexList->decContentsDepth();
               break;
            default:
               break;
         }
      }
     
   }
}

void writeIndexHierarchy(OutputList &ol)
{
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry();
   if (lne) {
      writeIndexHierarchyEntries(ol, lne->children());
   }
}
