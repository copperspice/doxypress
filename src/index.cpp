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

#include <QDateTime>
#include <QDir>
#include <QRegularExpression>
#include <QTextStream>

#include <stdlib.h>
#include <set>

#include <index.h>

#include <config.h>
#include <dirdef.h>
#include <dot.h>
#include <doxy_globals.h>
#include <ftvhelp.h>
#include <groupdef.h>
#include <htmlgen.h>
#include <language.h>
#include <layout.h>
#include <message.h>
#include <outputlist.h>
#include <sortedlist.h>
#include <util.h>

#define MAX_ITEMS_BEFORE_MULTIPAGE_INDEX 200
#define MAX_ITEMS_BEFORE_QUICK_INDEX     30

static int s_annotatedClassesPrinted;

static int s_annotatedCount;
static int s_hierarchyCount;
static int s_conceptCount;

static int s_documentedDirs;
static int s_documentedGroups;
static int s_documentedNamespaces;

static int s_documentedClassMembers[CMHL_Total];
static int s_documentedFileMembers[FMHL_Total];
static int s_documentedNamespaceMembers[NMHL_Total];

static int countClassAnnotated();
static int countClassHierarchy();
static int countConcepts();
static int countGroups();
static int countDirs();
static int countNamespaces();

static void countFiles();
static void countRelatedPages();

static QVector<bool> s_indexWritten = QVector<bool>(LayoutNavEntry::LastValue+1, false);

void countDataStructures()
{
   // "classes" + "annotated"
   s_annotatedCount = countClassAnnotated();

   // "hierarchy"
   s_hierarchyCount = countClassHierarchy();

   // "concepts"
   s_conceptCount = countConcepts();

   // "dirs"
   s_documentedDirs = countDirs();

   // "modules"
   s_documentedGroups = countGroups();

   // "namespaces"
   s_documentedNamespaces = countNamespaces();

   // "files"
   countFiles();

   // "pages"
   countRelatedPages();

   // "globals"
   // "namespacemembers"
   // "functions"
}

int getCount(IndexTotals symbol)
{
   switch (symbol) {
      case IndexTotals::AnnotatedCount:
         return s_annotatedCount;

      case IndexTotals::HierarchyCount:
         return s_hierarchyCount;

      case IndexTotals::ConceptCount:
         return s_conceptCount;

      case IndexTotals::DocumentedDirs:
         return s_documentedDirs;

      case IndexTotals::DocumentedGroups:
         return s_documentedGroups;

      case IndexTotals::DocumentedNamespaces:
         return s_documentedNamespaces;

      case IndexTotals::DocumentedClassMembers:
         return s_documentedClassMembers[CMHL_All];

      case IndexTotals::DocumentedFileMembers:
         return s_documentedFileMembers[FMHL_All];

      case IndexTotals::DocumentedNamespaceMembers:
         return s_documentedNamespaceMembers[NMHL_All];
   }

   return 0;
}

static int countClassAnnotated()
{
   int count = 0;
   int countPrinted = 0;

   for (auto cd : Doxy_Globals::classSDict) {

      if (cd->isLinkableInProject() && cd->templateMaster() == nullptr) {
         if (! cd->isEmbeddedInOuterScope()) {
            ++countPrinted;
         }

         ++count;
      }
   }

   s_annotatedClassesPrinted = countPrinted;

   return count;
}

static int countClassesInTreeList(const ClassSDict &cl)
{
   int count = 0;

   for (auto cd : cl) {

      // filter on root classes
      if (! hasVisibleRoot(cd->baseClasses())) {

         if (cd->isVisibleInHierarchy()) {
            // should have sub classes

            if (cd->subClasses()) {
               ++count;
            }
         }
      }
   }

   return count;
}

static int countClassHierarchy()
{
   int count = 0;

   initClassHierarchy(&Doxy_Globals::classSDict);
   initClassHierarchy(&Doxy_Globals::hiddenClasses);

   count += countClassesInTreeList(Doxy_Globals::classSDict);
   count += countClassesInTreeList(Doxy_Globals::hiddenClasses);

   return count;
}

static int countConcepts()
{
   int count = 0;

   for (auto conceptdef : Doxy_Globals::conceptSDict) {
      if (conceptdef->isLinkableInProject()) {
         ++count;
      }
   }

   return count;
}

static int countDirs()
{
   int count = 0;

   for (auto dd : Doxy_Globals::directories) {
      if (dd->isLinkableInProject()) {
         dd->visited = false;
         ++count;
      }
   }

   return count;
}

static int countGroups()
{
   int count = 0;

   for (auto gd : Doxy_Globals::groupSDict)  {
      if (! gd->isReference()) {
         gd->visited = false;
         ++count;
      }
   }

   return count;
}

static int countNamespaces()
{
   int count = 0;

   for (auto &nd : Doxy_Globals::namespaceSDict) {
      if (nd->isLinkableInProject()) {
         ++count;
      }
   }

   return count;
}

static void countFiles()
{
   Doxy_Globals::documentedHtmlFiles = 0;
   Doxy_Globals::documentedSrcFiles  = 0;
   Doxy_Globals::documentedFiles     = 0;

   for (auto &fNameList : Doxy_Globals::inputNameList) {

      for (auto fd : *fNameList) {
         bool doc = docFileVisibleInIndex(fd);
         bool src = srcFileVisibleInIndex(fd);

         if (doc || src) {
            ++Doxy_Globals::documentedHtmlFiles;
         }

         if (src) {
            ++Doxy_Globals::documentedSrcFiles;
         }

         if (doc) {
            ++Doxy_Globals::documentedFiles;
         }
      }
   }
}

static void countRelatedPages()
{
   Doxy_Globals::documentedPages = 0;
   Doxy_Globals::indexedPages    = 0;

   for (auto &pd : Doxy_Globals::pageSDict) {

      if ( pd->visibleInIndex()) {
         Doxy_Globals::indexedPages++;
      }

      if ( pd->documentedPage()) {
         Doxy_Globals::documentedPages++;
      }
   }
}

static void startIndexHierarchy(OutputList &ol, int level)
{
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);
   ol.disable(OutputGenerator::Man);

   if (level < 6) {
      ol.startIndexList();
   }

   ol.popGeneratorState();
   ol.pushGeneratorState();

   ol.disable(OutputGenerator::Docbook);
   ol.disable(OutputGenerator::Latex);
   ol.disable(OutputGenerator::RTF);
   ol.startItemList();
   ol.popGeneratorState();
}

static void endIndexHierarchy(OutputList &ol, int level)
{
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);
   ol.disable(OutputGenerator::Man);

   if (level < 6) {
      ol.endIndexList();
   }

   ol.popGeneratorState();
   ol.pushGeneratorState();

   ol.disable(OutputGenerator::Docbook);
   ol.disable(OutputGenerator::Latex);
   ol.disable(OutputGenerator::RTF);
   ol.endItemList();
   ol.popGeneratorState();
}

class MemberIndexList : public QList<QSharedPointer<MemberDef>>
{
 public:
   MemberIndexList(QChar letter)
      : m_letter(letter)
   { }

   ~MemberIndexList()
   { }

   int compareValues(const QSharedPointer<MemberDef> md1, const QSharedPointer<MemberDef> md2) const {
      int result = md1->name().compare(md2->name(), Qt::CaseInsensitive);

      if (result == 0) {
         result = md1->qualifiedName().compare(md2->qualifiedName(), Qt::CaseInsensitive);
      }

      return result;
   }

   void insertDef(QSharedPointer<MemberDef> data) {
      append(data);
   }

   QChar letter() const {
      return m_letter;
   }

 private:
   QChar m_letter;
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

static void startQuickIndexItem(OutputList &ol, const QString &link, bool hl, bool compact)
{
   ol.writeString("      <li");

   if (hl) {
      ol.writeString(" class=\"current\"");
   }

   ol.writeString("><a ");
   ol.writeString("href=\"");
   ol.writeString(link);
   ol.writeString("\">");
   ol.writeString("<span>");


   (void) compact;

/*
   bool fancyTabs = true;

   if (! first && compact && ! fancyTabs) {
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
      if (! compact) {
         ol.writeString("<li>");
      }
      if (hl && compact) {
         ol.writeString("<a class=\"qindexHL\" ");
      } else {
         ol.writeString("<a class=\"qindex\" ");
      }
   }

   ol.writeString("href=\"");
   ol.writeString(link);
   ol.writeString("\">");

   if (fancyTabs) {
      ol.writeString("<span>");
   }
*/

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

QString fixSpaces(const QString &s)
{
   return substitute(s, " ", "&#160;");
}

void startTitle(OutputList &ol, const QString &fileName, QSharedPointer<Definition> def)
{
   ol.startHeaderSection();

   if (def) {
      def->writeSummaryLinks(ol);
   }

   ol.startTitleHead(fileName);
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Man);
}

void endTitle(OutputList &ol, const QString &fileName, const QString &name)
{
   ol.popGeneratorState();
   ol.endTitleHead(fileName, name);
   ol.endHeaderSection();
}

void startFile(OutputList &ol, const QString &name, const QString &manName, const QString &title, HighlightedItem hli,
               bool additionalIndices, const QString &altSidebarName)
{
   static bool disableIndex = Config::getBool("disable-index");

   ol.startFile(name, manName, title);
   ol.startQuickIndices();

   if (! disableIndex) {
      ol.writeQuickLinks(true, hli, name);
   }

   if (! additionalIndices) {
      ol.endQuickIndices();
   }

   if (altSidebarName.isEmpty()) {
      ol.writeSplitBar(name);
   } else {
      ol.writeSplitBar(altSidebarName);
   }

   ol.writeSearchInfo();
   resetDotNodeNumbering();
}

void endFile(OutputList &ol, bool skipNavIndex, bool skipEndContents, const QString &navPath)
{
   static bool generateTreeView = Config::getBool("generate-treeview");

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   if (! skipNavIndex) {
      if (! skipEndContents) {
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

void endFileWithNavPath(QSharedPointer<Definition> d, OutputList &ol)
{
   static const bool generateTreeView = Config::getBool("generate-treeview");

   QString navPath;

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
void addMembersToIndex(QSharedPointer<T> def, LayoutDocManager::LayoutPart part, const QString &name, const QString &anchor,
                       bool preventSeparateIndex = false, bool addToIndex = true, DirType category = DirType::None)
{
   static bool inlineSimpleStructs = Config::getBool("inline-simple-struct");
   static bool hideNavtreeMembers  = Config::getBool("hide-navtree-members");

   bool hasMembers = def->getMemberLists().count() > 0 || def->getMemberGroupSDict().count() > 0;

   Doxy_Globals::indexList.addContentsItem(hasMembers, name, def->getReference(), def->getOutputFileBase(), anchor,
                  addToIndex, def, category);

   int numClasses = 0;
   const ClassSDict &classes = def->getClassSDict();

   for (auto cd : classes) {
      if (cd->isLinkable()) {
         numClasses++;
      }
   }

   if (hasMembers || numClasses > 0) {

      Doxy_Globals::indexList.incContentsDepth();

      for (auto lde : LayoutDocManager::instance().docEntries(part)) {

         if (lde->kind() == LayoutDocEntry::MemberDef) {
            LayoutDocEntryMemberDef *lmd  = (LayoutDocEntryMemberDef *)lde;
            QSharedPointer<MemberList> ml = def->getMemberList(lmd->type);

            if (hideNavtreeMembers) {
               // do not add members to the navtree

            } else if (ml) {
               for (auto md : *ml) {
                  QSharedPointer<MemberList> enumList = md->enumFieldList();

                  bool isDir = (enumList != 0 && md->isEnumerate());
                  bool isAnonymous = md->name().contains('@');

                  static bool hideUndocMembers = Config::getBool("hide-undoc-members");
                  static bool extractStatic    = Config::getBool("extract-static");

                  if (! isAnonymous && (! hideUndocMembers || md->hasDocumentation()) && (! md->isStatic() || extractStatic)) {

                     if (md->getOuterScope() == def || md->getOuterScope() == Doxy_Globals::globalScope) {
                        // includes free functions

                        Doxy_Globals::indexList.addContentsItem(isDir, md->name(), md->getReference(),
                                 md->getOutputFileBase(), md->anchor(), addToIndex, md);

                     } else {
                        // inherited member

                        Doxy_Globals::indexList.addContentsItem(isDir, md->name(), def->getReference(),
                                 def->getOutputFileBase(), md->anchor(), addToIndex, md);
                     }
                  }

                  if (isDir) {
                     if (! isAnonymous) {
                        Doxy_Globals::indexList.incContentsDepth();
                     }

                     for (auto emd : *enumList ) {

                        if (! hideUndocMembers || emd->hasDocumentation()) {
                           if (emd->getOuterScope() == def || emd->getOuterScope() == Doxy_Globals::globalScope) {
                              Doxy_Globals::indexList.addContentsItem(false, emd->name(), emd->getReference(),
                                    emd->getOutputFileBase(), emd->anchor(), addToIndex, emd);

                           } else {
                              // inherited member
                              Doxy_Globals::indexList.addContentsItem(false, emd->name(), def->getReference(),
                                    def->getOutputFileBase(), emd->anchor(), addToIndex, emd);
                           }
                        }
                     }

                     if (! isAnonymous) {
                        Doxy_Globals::indexList.decContentsDepth();
                     }
                  }
               }
            }

         } else if (lde->kind() == LayoutDocEntry::NamespaceClasses || lde->kind() == LayoutDocEntry::FileClasses ||
                    lde->kind() == LayoutDocEntry::ClassNestedClasses ) {

            for (auto cd : classes ) {
               if (cd->isLinkable() && (cd->partOfGroups() == 0 || def->definitionType() == Definition::TypeGroup)) {

                  bool separateIndex = (preventSeparateIndex || cd->isEmbeddedInOuterScope());
                  bool isNestedClass = (def->definitionType() == Definition::TypeClass);
                  bool addToNavIndex = (addToIndex && (isNestedClass || (cd->isSimple() && inlineSimpleStructs)));

                  addMembersToIndex(cd, LayoutDocManager::Class, cd->displayName(false), cd->anchor(),
                           separateIndex, addToNavIndex);
               }
            }
         }
      }

      Doxy_Globals::indexList.decContentsDepth();
   }
}

// Generates HTML Help tree of classes
static void writeBaseClassTree(OutputList &ol, const SortedList<BaseClassDef *> *bcl, bool hideSuper, int level, FTVHelp *ftv, bool addToIndex)
{
   if (bcl == nullptr) {
      return;
   }

   bool started = false;

   for (auto item : *bcl) {
      QSharedPointer<ClassDef> cd = item->classDef;

      bool b;
      b = hasVisibleRoot(cd->baseClasses());

      if (cd->isVisibleInHierarchy() && b) {
         // hasVisibleRoot(cd->baseClasses()))

         if (! started) {
            startIndexHierarchy(ol, level);

            if (addToIndex) {
               Doxy_Globals::indexList.incContentsDepth();
            }

            if (ftv) {
               ftv->incContentsDepth();
            }

            started = true;
         }

         ol.startIndexListItem();

         bool hasChildren = ! cd->isVisited() && ! hideSuper && classHasVisibleChildren(cd);

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
               Doxy_Globals::indexList.addContentsItem(hasChildren, cd->displayName(), cd->getReference(),
                        cd->getOutputFileBase(), cd->anchor(), false, cd);
            }

            if (ftv) {
               ftv->addContentsItem(hasChildren, cd->displayName(), cd->getReference(), cd->getOutputFileBase(),
                        cd->anchor(), false, cd);
            }

         } else {
            ol.startIndexItem(QString(), QString());
            ol.parseText(cd->name());
            ol.endIndexItem(QString(), QString());

            if (addToIndex) {
               Doxy_Globals::indexList.addContentsItem(hasChildren, cd->displayName(), QString(), QString(), QString(), false, cd);
            }

            if (ftv) {
               ftv->addContentsItem(hasChildren, cd->displayName(), QString(), QString(), QString(), false, cd);
            }
         }

         if (hasChildren) {
            bool wasVisited = cd->isVisited();
            cd->setVisited(true);

            writeBaseClassTree(ol, cd->subClasses(), wasVisited, level + 1, ftv, addToIndex);
         }

         ol.endIndexListItem();
      }
   }
   if (started) {
      endIndexHierarchy(ol, level);

      if (addToIndex) {
         Doxy_Globals::indexList.decContentsDepth();
      }
      if (ftv) {
         ftv->decContentsDepth();
      }
   }
}

static bool dirHasVisibleChildren(QSharedPointer<DirDef> dd)
{
   if (dd->hasDocumentation()) {
      return true;
   }

   for (const auto fd : dd->getFiles() ) {

      if (docFileVisibleInIndex(fd)) {
         return true;
      }

      if (srcFileVisibleInIndex(fd)) {
         return true;
      }
   }

   for (const auto subdd : dd->subDirs() ) {
      if (dirHasVisibleChildren(subdd)) {
         return true;
      }
   }

   return false;
}

static void writeDirTreeNode(OutputList &ol, QSharedPointer<DirDef> dd, int level, FTVHelp *ftv, bool addToIndex, DirType category)
{
   if (level > 20) {
      warn(dd->getDefFileName(), dd->getDefLine(), "Maximum nesting level exceeded for directory %s: "
           ", verify possible recursive directory relation\n", csPrintable(dd->name()));
      return;
   }

   if (! dirHasVisibleChildren(dd)) {
      return;
   }

   static bool tocExpand = true;    // Config::getBool("toc-expand");

   bool isDir = dd->subDirs().count() > 0 || (tocExpand && dd->getFiles().count() > 0);

   if (addToIndex) {
      Doxy_Globals::indexList.addContentsItem(isDir, dd->shortName(), dd->getReference(), dd->getOutputFileBase(), "", true, dd);
      Doxy_Globals::indexList.incContentsDepth();
   }

   if (ftv) {
      ftv->addContentsItem(isDir, dd->shortName(), dd->getReference(), dd->getOutputFileBase(), "", true, dd);
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

      for (const auto subdd : dd->subDirs()) {
         writeDirTreeNode(ol, subdd, level + 1, ftv, addToIndex, category);
      }
      endIndexHierarchy(ol, level + 1);
   }

   FileList fileList = dd->getFiles();
   int fileCount = 0;

   if (fileList.count() > 0) {

      for (const auto fd : fileList) {
         bool doc = docFileVisibleInIndex(fd);
         bool src = srcFileVisibleInIndex(fd);

         if (category == DirType::File && doc) {
            ++fileCount;

         } else if (category == DirType::FileSource && src) {
            ++fileCount;
         }
      }

      if (fileCount > 0) {
         startIndexHierarchy(ol, level + 1);

         for (auto fd : fileList) {
            bool doc = docFileVisibleInIndex(fd);
            bool src = srcFileVisibleInIndex(fd);

            QString reference;
            QString outputBase;

            if (doc) {
               reference  = fd->getReference();
               outputBase = fd->getOutputFileBase();
            }

            if (category == DirType::File && doc) {
               ol.startIndexListItem();
               ol.startIndexItem(reference, outputBase);
               ol.parseText(fd->displayName());
               ol.endIndexItem(reference, outputBase);
               ol.endIndexListItem();

               if (ftv) {
                  ftv->addContentsItem(false, fd->displayName(), reference, outputBase, "", false, fd, category);
               }

            } else if (category == DirType::FileSource && src) {
               ol.startIndexListItem();
               ol.startIndexItem(reference, fd->getSourceFileBase());
               ol.parseText(fd->displayName());
               ol.endIndexItem(reference, fd->getSourceFileBase());
               ol.endIndexListItem();

               if (ftv) {
                  ftv->addContentsItem(false, fd->displayName(), reference, fd->getSourceFileBase(), "", false, fd, category);
               }
            }
         }

         endIndexHierarchy(ol, level + 1);
      }
   }

   if (tocExpand && addToIndex) {
      // write files of this directory

      if (fileCount > 0) {

         for (auto fd : fileList) {
            bool doc = docFileVisibleInIndex(fd);
            bool src = srcFileVisibleInIndex(fd);

            if (category == DirType::File && doc) {
               addMembersToIndex(fd, LayoutDocManager::File, fd->displayName(), QString(), true, true, category);

            } else if (category == DirType::FileSource && src) {
               Doxy_Globals::indexList.addContentsItem(false, convertToHtml(fd->name(), true), "",
                     fd->getSourceFileBase(), "", true, fd, category);
            }
         }
      }
   }
   ol.endIndexListItem();

   if (addToIndex) {
      Doxy_Globals::indexList.decContentsDepth();
   }

   if (ftv) {
      ftv->decContentsDepth();
   }
}

static void writeDirTree(OutputList &ol, FTVHelp *ftv, bool addToIndex, DirType category)
{
   static const bool fullPathNames = Config::getBool("full-path-names");
   static QString mainPageName     = Config::getFullName(Config::getString("main-page-name"));

   if (ftv) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
   }

   startIndexHierarchy(ol, 0);

   if (fullPathNames) {
      for (auto dd : Doxy_Globals::directories) {
         if (dd->getOuterScope() == Doxy_Globals::globalScope) {
            writeDirTreeNode(ol, dd, 0, ftv, addToIndex, category);
         }
      }
   }

   if (ftv) {

      for (auto &fn : Doxy_Globals::inputNameList) {

         for (auto fd : *fn) {

            if (! mainPageName.isEmpty() && fd->getFilePath() == mainPageName) {
               // do not include the mainPage in the File List
               continue;
            }

            if (! fullPathNames || fd->getDirDef() == 0) {
               // top level file
               bool doc = docFileVisibleInIndex(fd);
               bool src = srcFileVisibleInIndex(fd);

               QString reference;
               QString outputBase;

               if (doc) {
                  reference  = fd->getReference();
                  outputBase = fd->getOutputFileBase();
               }

               if (category == DirType::File && doc) {

                  ftv->addContentsItem(false, fd->displayName(), reference, outputBase, QString(), false, fd, category);

                  if (addToIndex) {
                     addMembersToIndex(fd, LayoutDocManager::File, fd->displayName(), QString(), true, true, category);
                  }

               } else if (category == DirType::FileSource && src) {
                  ftv->addContentsItem(false, fd->displayName(), reference, fd->getSourceFileBase(), QString(), false, fd, category);

                  if (addToIndex) {
                     Doxy_Globals::indexList.addContentsItem(false, convertToHtml(fd->name(), true),
                        QString(), fd->getSourceFileBase(), QString(), true, fd, category);
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

static void writeClassTreeForList(OutputList &ol, ClassSDict *cl, bool &started, FTVHelp *ftv, bool addToIndex)
{
    for (auto cd : *cl) {

      bool b = ! hasVisibleRoot(cd->baseClasses());

      if (b) {
         // filter on root classes

         if (cd->isVisibleInHierarchy()) {
            // should it be visible

            if (!started) {
               startIndexHierarchy(ol, 0);

               if (addToIndex) {
                  Doxy_Globals::indexList.incContentsDepth();
               }
               started = true;
            }

            ol.startIndexListItem();

            bool hasChildren = ! cd->isVisited() && classHasVisibleChildren(cd);

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
                  // prevents double insertion in Design Unit List
                  Doxy_Globals::indexList.addContentsItem(hasChildren, cd->displayName(), cd->getReference(),
                                       cd->getOutputFileBase(), cd->anchor(), false, cd);
               }

               if (ftv) {
                  ftv->addContentsItem(hasChildren, cd->displayName(), cd->getReference(), cd->getOutputFileBase(),
                                       cd->anchor(), false, cd);
               }

            } else {
               // no link, use the name instead of the displayName, this will show the namespace::class
               ol.startIndexItem(QString(), QString());
               ol.parseText(cd->name());
               ol.endIndexItem(QString(), QString());

               if (addToIndex) {
                  Doxy_Globals::indexList.addContentsItem(hasChildren, cd->name(), QString(), QString(), QString(), false, cd);
               }

               if (ftv) {
                  ftv->addContentsItem(hasChildren, cd->name(), QString(), QString(), QString(), false, cd);
               }
            }

            if (hasChildren) {
               writeBaseClassTree(ol, cd->subClasses(), cd->isVisited(), 1, ftv, addToIndex);
               cd->setVisited(true);
            }

            ol.endIndexListItem();
         }
      }
   }
}

static void writeClassHierarchy(OutputList &ol, FTVHelp *ftv, bool addToIndex)
{
   initClassHierarchy(&Doxy_Globals::classSDict);
   initClassHierarchy(&Doxy_Globals::hiddenClasses);

   if (ftv) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
   }

   bool started = false;
   writeClassTreeForList(ol, &Doxy_Globals::classSDict, started, ftv, addToIndex);
   writeClassTreeForList(ol, &Doxy_Globals::hiddenClasses, started, ftv, addToIndex);

   if (started) {
      endIndexHierarchy(ol, 0);
      if (addToIndex) {
         Doxy_Globals::indexList.decContentsDepth();
      }
   }

   if (ftv) {
      ol.popGeneratorState();
   }
}

static void writeHierarchicalIndex(OutputList &ol)
{
   if (s_hierarchyCount == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Docbook);
   ol.disable(OutputGenerator::Man);

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::ClassHierarchy);

   QString title   = lne ? lne->title() : theTranslator->trClassHierarchy();
   bool addToIndex = (lne == nullptr || lne->visible());

   startFile(ol, "hierarchy", QString(), title, HLI_Hierarchy);

   startTitle(ol, QString());
   ol.parseText(title);
   endTitle(ol,  QString(),  QString());

   ol.startContents();
   ol.startTextBlock();

   if (Config::getBool("have-dot") && Config::getBool("dot-hierarchy")) {

      ol.pushGeneratorState();

      ol.disable(OutputGenerator::Docbook);
      ol.disable(OutputGenerator::Latex);
      ol.disable(OutputGenerator::RTF);
      ol.startParagraph();

      ol.startTextLink("inherits", QString());
      ol.parseText(theTranslator->trGotoGraphicalHierarchy());
      ol.endTextLink();
      ol.endParagraph();

      ol.popGeneratorState();
   }

   ol.parseText(lne ? lne->intro() : theTranslator->trClassHierarchyDescription());
   ol.endTextBlock();

   // Static class hierarchy for Latex/RTF
   ol.pushGeneratorState();

   ol.disable(OutputGenerator::Html);
   Doxy_Globals::indexList.disable();

   writeClassHierarchy(ol, nullptr, addToIndex);

   Doxy_Globals::indexList.enable();
   ol.popGeneratorState();

   // ---------------
   // Dynamic class hierarchical index for HTML
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   {
      if (addToIndex) {
         Doxy_Globals::indexList.addContentsItem(true, title, "", "hierarchy", "", true);
      }

      FTVHelp *ftv = new FTVHelp(false);
      writeClassHierarchy(ol, ftv, addToIndex);

      QString outStr;
      QTextStream t(&outStr);

      ftv->generateTreeViewInline(t);

      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);
      ol.writeString(outStr);

      ol.popGeneratorState();

      delete ftv;
   }

   ol.popGeneratorState();

   endFile(ol, false, false);
   ol.popGeneratorState();
}

static void writeGraphicalClassHierarchy(OutputList &ol)
{
   if (s_hierarchyCount == 0) {
      return;
   }

   ol.disableAllBut(OutputGenerator::Html);
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::ClassHierarchy);

   QString title = lne ? lne->title() : theTranslator->trClassHierarchy();
   startFile(ol, "inherits", QString(), title, HLI_Hierarchy, false, "hierarchy");
   startTitle(ol, QString());

   ol.parseText(title);
   endTitle(ol, QString(), QString());

   ol.startContents();
   ol.startTextBlock();
   ol.startParagraph();
   ol.startTextLink("hierarchy", QString());
   ol.parseText(theTranslator->trGotoTextualHierarchy());
   ol.endTextLink();
   ol.endParagraph();
   ol.endTextBlock();
   DotGfxHierarchyTable g;
   ol.writeGraphicalHierarchy(g);
   endFile(ol, false, false);
   ol.enableAll();
}

static void writeSingleFileIndex(OutputList &ol, QSharedPointer<FileDef> fd)
{
   bool doc     = fd->isLinkableInProject();
   bool src     = fd->generateSourceFile();
   bool addFile = fd->isDocumentationFile();

   if (addFile && (doc || src) && ! fd->isReference()) {
      // add the file entry to the navtree and file list
      QString path;

      if (Config::getBool("full-path-names")) {
         path = stripFromPath(fd->getPath());
      }

      QString fullName = fd->name();

      if (! path.isEmpty()) {
         if (path.at(path.length() - 1) != '/') {
            fullName.prepend("/");
         }
         fullName.prepend(path);
      }

      ol.startIndexKey();
      ol.docify(path);

      if (doc) {
         ol.writeObjectLink(QString(), fd->getOutputFileBase(), QString(), fd->name());

      } else {
         ol.startBold();
         ol.docify(fd->name());
         ol.endBold();
      }
      if (src) {
         ol.pushGeneratorState();
         ol.disableAllBut(OutputGenerator::Html);
         ol.docify(" ");
         ol.startTextLink(fd->includeName(), QString());
         ol.docify("[");
         ol.parseText(theTranslator->trCode());
         ol.docify("]");
         ol.endTextLink();
         ol.popGeneratorState();
      }

      ol.endIndexKey();

      bool hasBrief = ! fd->briefDescription().isEmpty();
      ol.startIndexValue(hasBrief);

      if (hasBrief) {
         ol.generateDoc(fd->briefFile(), fd->briefLine(), fd, QSharedPointer<MemberDef>(), fd->briefDescription(true),
               false, false, QString(), true, true);
      }

      ol.endIndexValue(fd->getOutputFileBase(), hasBrief);
   }
}

static void writeFileIndex(OutputList &ol)
{
   // enabled from build options, file command

   if (Doxy_Globals::documentedHtmlFiles == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Docbook);
   ol.disable(OutputGenerator::Man);

   if (Doxy_Globals::documentedFiles == 0) {
      ol.disableAllBut(OutputGenerator::Html);
   }

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::FileList);
   if (lne == nullptr) {
      // fall back
      lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Files);
   }

   QString title = lne ? lne->title() : theTranslator->trFileList();
   bool addToIndex = (lne == nullptr || lne->visible());

   startFile(ol, "files", QString(), title, HLI_Files);
   startTitle(ol, QString());

   ol.parseText(title);
   endTitle(ol, QString(), QString());

   ol.startContents();
   ol.startTextBlock();

   if (addToIndex) {
      Doxy_Globals::indexList.addContentsItem(true, title, QString(), "files", QString(), true);
      Doxy_Globals::indexList.incContentsDepth();
   }

   ol.parseText(lne ? lne->intro() : theTranslator->trFileListDescription(Config::getBool("extract-all")));
   ol.endTextBlock();

   // flat file index
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);

   QMap<QString, QSharedPointer<FileList>> outputNameMap;

   if (Config::getBool("full-path-names")) {

      for (auto &fn : Doxy_Globals::inputNameList ) {

         for (auto fd : *fn) {
            QString path = fd->getPath();

            if (path.isEmpty()) {
               path = "[external]";
            }

            QSharedPointer<FileList> fl = outputNameMap.value(path);

            if (fl) {
               fl->inSort(fd);

            } else {

               fl = QMakeShared<FileList>();
               fl->inSort(fd);

               outputNameMap.insert(path, fl);
            }
         }
      }
   }

   ol.startIndexList();

   if (Config::getBool("full-path-names")) {

      for (auto fl : outputNameMap) {
         for (auto fd : *fl) {
            writeSingleFileIndex(ol, fd);
         }
      }

   } else {
      // show minimal file name on files.html

      for (auto &fn : Doxy_Globals::inputNameList) {
         for (auto fd : *fn) {
            writeSingleFileIndex(ol, fd);
         }
      }
   }

   ol.endIndexList();
   ol.popGeneratorState();
   ol.pushGeneratorState();

   // file index for HTML
   ol.disableAllBut(OutputGenerator::Html);

   FTVHelp *ftv = new FTVHelp(false);

   // add list of file names to the navtree
   writeDirTree(ol, ftv, addToIndex, DirType::File);

   QString outStr;
   QTextStream t(&outStr);

   ftv->generateTreeViewInline(t);
   ol.writeString(outStr);
   delete ftv;

   ol.popGeneratorState();

   if (addToIndex) {
      Doxy_Globals::indexList.decContentsDepth();
   }

   endFile(ol, false, false);
   ol.popGeneratorState();
}

static void writeSingleFileSourceIndex(OutputList &ol, QSharedPointer<FileDef> fd)
{
   bool doc     = fd->isLinkableInProject();
   bool src     = fd->generateSourceFile();
   bool addFile = fd->isDocumentationFile();

   if (addFile && (doc || src) && ! fd->isReference()) {
      // add the file entry to the navtree and file list
      QString path;

      if (Config::getBool("full-path-names")) {
         path = stripFromPath(fd->getPath());
      }

      QString fullName = fd->name();

      if (! path.isEmpty()) {
         if (path.at(path.length() - 1) != '/') {
            fullName.prepend("/");
         }
         fullName.prepend(path);
      }

      ol.startIndexKey();
      ol.docify(path);

      if (doc) {
         ol.writeObjectLink(QString(), fd->getOutputFileBase(), QString(), fd->name());

      } else {
         ol.startBold();
         ol.docify(fd->name());
         ol.endBold();
      }

      if (src) {
         ol.pushGeneratorState();
         ol.disableAllBut(OutputGenerator::Html);
         ol.docify(" ");
         ol.startTextLink(fd->includeName(), QString());
         ol.docify("[");
         ol.parseText(theTranslator->trCode());
         ol.docify("]");
         ol.endTextLink();
         ol.popGeneratorState();
      }

      ol.endIndexKey();

      bool hasBrief = ! fd->briefDescription().isEmpty();
      ol.startIndexValue(hasBrief);

      if (hasBrief) {
         ol.generateDoc(fd->briefFile(), fd->briefLine(), fd, QSharedPointer<MemberDef>(), fd->briefDescription(true),
               false, false, QString(), true, true);
      }

      ol.endIndexValue(fd->getOutputFileBase(), hasBrief);
   }
}

static void writeFileSourceIndex(OutputList &ol)
{
   static const bool fullPathNames = Config::getBool("full-path-names");

   if (Doxy_Globals::documentedSrcFiles == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Man);

   if (Doxy_Globals::documentedFiles == 0) {
      ol.disableAllBut(OutputGenerator::Html);
   }

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::FileSource);

   QString title   = lne ? lne->title() : theTranslator->trFileSource();
   bool addToIndex = (lne == nullptr || lne->visible());

   startFile(ol, "filesource", QString(), title, HLI_FileSource);

   startTitle(ol, QString());
   ol.parseText(title);
   endTitle(ol, QString(), QString());

   ol.startContents();
   ol.startTextBlock();

   if (addToIndex) {
      Doxy_Globals::indexList.addContentsItem(true, title, "", "filesource", "", true);
      Doxy_Globals::indexList.incContentsDepth();
   }

   ol.parseText(lne ? lne->intro() : theTranslator->trFileSourceDescription());
   ol.endTextBlock();

   // flat file index
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);

   QMap<QString, QSharedPointer<FileList>> outputNameMap;

   if (fullPathNames) {

      for (auto &fn : Doxy_Globals::inputNameList ) {

         for (auto fd : *fn) {
            QString path = fd->getPath();

            if (path.isEmpty()) {
               path = "[external]";
            }

            QSharedPointer<FileList> fl = outputNameMap.value(path);

            if (fl) {
               fl->inSort(fd);

            } else {

               fl = QMakeShared<FileList>();
               fl->inSort(fd);

               outputNameMap.insert(path, fl);
            }
         }
      }
   }

   ol.startIndexList();

   if (fullPathNames) {
      for (auto fl : outputNameMap) {
         for (auto fd : *fl) {
            writeSingleFileSourceIndex(ol, fd);
         }
      }

   } else {
      // show minimal file name on filesource.html

      for (auto &fn : Doxy_Globals::inputNameList) {
         for (auto fd : *fn) {
            writeSingleFileSourceIndex(ol, fd);
         }
      }
   }

   ol.endIndexList();
   ol.popGeneratorState();
   ol.pushGeneratorState();

   // file index for HTML
   ol.disableAllBut(OutputGenerator::Html);

   FTVHelp *ftv = new FTVHelp(false);

   // add list of file names to the navtree
   writeDirTree(ol, ftv, addToIndex, DirType::FileSource);

   QString outStr;
   QTextStream t(&outStr);

   ftv->generateTreeViewInline(t);
   ol.writeString(outStr);
   delete ftv;

   ol.popGeneratorState();

   if (addToIndex) {
      Doxy_Globals::indexList.decContentsDepth();
   }

   endFile(ol, false, false);
   ol.popGeneratorState();
}

static void writeClassTree(const ClassSDict &clDict, FTVHelp *ftv, bool addToIndex, bool globalOnly)
{
   for (auto cd : clDict) {

      if (! globalOnly || cd->getOuterScope() == 0 || cd->getOuterScope() == Doxy_Globals::globalScope ) {
         int count = 0;

         for (auto ccd : cd->getClassSDict()) {
            if (ccd->isLinkableInProject() && ccd->templateMaster() == 0) {
               count++;
            }
         }

         // passing cd->displayName(false) will strip the template parameters on at least annotated.html
         // modify to show based on a new project tag (2/2/2016)

         if (classVisibleInIndex(cd) && cd->templateMaster() == 0) {

            ftv->addContentsItem(count > 0, cd->displayName(false), cd->getReference(),
                  cd->getOutputFileBase(), cd->anchor(), true, cd);

            if (addToIndex && (cd->getOuterScope() == 0 || cd->getOuterScope()->definitionType() != Definition::TypeClass)) {

               bool tmp_addToIndex = cd->partOfGroups() == 0 && ! cd->isSimple();
               addMembersToIndex(cd, LayoutDocManager::Class, cd->displayName(false), cd->anchor(), false, tmp_addToIndex);
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

static void writeNamespaceTree(const NamespaceSDict &nsDict, FTVHelp *ftv, bool rootOnly, bool showClasses, bool addToIndex)
{
   for (auto &nd : nsDict) {
      if (nd->localName().indexOf('@') == -1 && (! rootOnly || nd->getOuterScope() == Doxy_Globals::globalScope)) {

         bool hasChildren = namespaceHasVisibleChild(nd, showClasses);
         bool isLinkable  = nd->isLinkableInProject();

         QString ref;
         QString file;

         if (isLinkable) {
            ref  = nd->getReference();
            file = nd->getOutputFileBase();
         }

         if ((isLinkable && ! showClasses) || hasChildren) {

            ftv->addContentsItem(hasChildren, nd->localName(), ref, file, "", true, nd);

            if (addToIndex) {
               Doxy_Globals::indexList.addContentsItem(hasChildren, nd->localName(), ref, file, QString(), addToIndex, nd);
            }

            if (hasChildren) {
               if (addToIndex) {
                  Doxy_Globals::indexList.incContentsDepth();
               }

               ftv->incContentsDepth();
               writeNamespaceTree(nd->getNamespaceSDict(), ftv, false, showClasses, addToIndex);

               if (showClasses) {
                  writeClassTree(nd->getClassSDict(), ftv, addToIndex, false);
               }

               ftv->decContentsDepth();
               if (addToIndex) {
                  Doxy_Globals::indexList.decContentsDepth();
               }
            }
         }
      }
   }
}

static void writeNamespaceIndex(OutputList &ol)
{
   if (s_documentedNamespaces == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Docbook);
   ol.disable(OutputGenerator::Man);
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::NamespaceList);

   if (lne == nullptr) {
      // fall back
      lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Namespaces);
   }

   QString title   = lne ? lne->title() : theTranslator->trNamespaceList();
   bool addToIndex = lne == nullptr || lne->visible();

   startFile(ol, "namespaces", QString(), title, HLI_Namespaces);

   startTitle(ol, QString());
   ol.parseText(title);
   endTitle(ol, QString(), QString());

   ol.startContents();
   ol.startTextBlock();
   ol.parseText(lne ? lne->intro() : theTranslator->trNamespacesListDescription(Config::getBool("extract-all")));
   ol.endTextBlock();

   bool firstEntry = true;

   // namespace index for Latex/RTF
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);

   for (auto &nd : Doxy_Globals::namespaceSDict) {

      if (nd->isLinkableInProject()) {

         if (firstEntry) {
            ol.startIndexList();
            firstEntry = false;
         }

         ol.startIndexKey();
         ol.writeObjectLink(QString(), nd->getOutputFileBase(), QString(), nd->displayName());
         ol.endIndexKey();

         bool hasBrief = ! nd->briefDescription().isEmpty();
         ol.startIndexValue(hasBrief);

         if (hasBrief) {
            ol.generateDoc(nd->briefFile(), nd->briefLine(), nd, QSharedPointer<MemberDef>(), nd->briefDescription(true),
               false, false, QString(), true, true);
         }

         ol.endIndexValue(nd->getOutputFileBase(), hasBrief);
      }
   }

   if (! firstEntry) {
      ol.endIndexList();
   }

   ol.popGeneratorState();

   // Hierarchical namespace index for HTML
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   {
      if (addToIndex) {
         Doxy_Globals::indexList.addContentsItem(true, title, QString(), "namespaces", QString(), true);
         Doxy_Globals::indexList.incContentsDepth();
      }

      FTVHelp *ftv = new FTVHelp(false);
      writeNamespaceTree(Doxy_Globals::namespaceSDict, ftv, true, false, addToIndex);

      QString outStr;
      QTextStream t(&outStr);

      ftv->generateTreeViewInline(t);
      ol.writeString(outStr);
      delete ftv;

      if (addToIndex) {
         Doxy_Globals::indexList.decContentsDepth();
      }
   }

   ol.popGeneratorState();
   endFile(ol, false, false);

   ol.popGeneratorState();
}

static void writeAnnotatedClassList(OutputList &ol)
{
   if (! Doxy_Globals::classSDict.isEmpty()) {
      ol.startIndexList();
   }

   for (auto cd : Doxy_Globals::classSDict) {
      ol.pushGeneratorState();

      if (cd->isEmbeddedInOuterScope()) {
         ol.disable(OutputGenerator::Docbook);
         ol.disable(OutputGenerator::Latex);
         ol.disable(OutputGenerator::RTF);
      }

      if (cd->isLinkableInProject() && cd->templateMaster() == nullptr) {
         ol.startIndexKey();

         ol.writeObjectLink(QString(), cd->getOutputFileBase(), cd->anchor(), cd->displayName());
         ol.endIndexKey();

         bool hasBrief = ! cd->briefDescription().isEmpty();
         ol.startIndexValue(hasBrief);

         if (hasBrief) {
            ol.generateDoc(cd->briefFile(), cd->briefLine(), cd, QSharedPointer<MemberDef>(), cd->briefDescription(true),
               false, false, QString(), true, true);
         }

         ol.endIndexValue(cd->getOutputFileBase(), hasBrief);
      }

      ol.popGeneratorState();
   }

   if (! Doxy_Globals::classSDict.isEmpty()) {
      ol.endIndexList();
   }
}

static QString letterToLabel(QChar startLetter)
{
   QString retval;

   if (startLetter > 0x20 && startLetter <= 0x7f) {
      // printable ASCII character
      retval = startLetter;

   } else {
      retval = QString::number(startLetter.unicode(), 16);

      if (retval.size() % 2) {
         retval.prepend("0x0");
      } else {
         retval.prepend("0x");
      }
   }

   return retval;
}

/** Special class list where sorting takes IGNORE_PREFIX into account. */
class PrefixIgnoreClassList : public SortedList<QSharedPointer<ClassDef>>
{
 public:
   PrefixIgnoreClassList(QChar letter)
      : m_letter(letter) { }

   void insertDef(QSharedPointer<ClassDef> data) {
      iterator location;
      location = std::lower_bound(this->begin(), this->end(), data, &PrefixIgnoreClassList::compareListValues);

      this->insert(location, data);
   }

   QChar letter() const {
      return m_letter;
   }

 private:
   static bool compareListValues(const QSharedPointer<ClassDef> c1, const QSharedPointer<ClassDef> c2) {
      QString n1 = c1->className();
      QString n2 = c2->className();

      QString tmp1 = n1.mid( getPrefixIndex(n1) );
      QString tmp2 = n2.mid( getPrefixIndex(n2) );

      int i = tmp1.compare(tmp2, Qt::CaseInsensitive);

      return i < 0;
   }

   QChar m_letter;
};

/** Class representing a cell in the alphabetical class index. */
class AlphaIndexTableCell
{
 public:
   AlphaIndexTableCell(int row, int col, QChar letter, QSharedPointer<ClassDef> cd)
      : m_letter(letter), m_class(cd), m_row(row), m_col(col)
   {
   }

   QSharedPointer<ClassDef> classDef() const {
      return m_class;
   }

   QChar letter() const {
      return m_letter;
   }

   int row() const {
      return m_row;
   }

   int column() const {
      return m_col;
   }

 private:
   QChar m_letter;
   QSharedPointer<ClassDef> m_class;
   int m_row;
   int m_col;
};

// write an alphabetical index of all class with a header for each letter
static void writeAlphabeticalClassList(OutputList &ol)
{
   // letters which are used
   std::set<QChar> lettersUsed;

   // list of classes for each letter
   LetterToIndexMap<PrefixIgnoreClassList> classesByLetter;

   // count the number of headers
   QChar startLetter;
   int headerItems  = 0;

   for (auto cd : Doxy_Globals::classSDict) {
      if (cd->isLinkableInProject() && cd->templateMaster() == 0) {
         int index   = getPrefixIndex(cd->className());
         startLetter = charToLower(cd->className(), index);

         lettersUsed.insert(startLetter);
         classesByLetter.insertElement(startLetter, cd);
      }
   }

   // write quick link index (row of letters)
   QString alphaLinks = "<div class=\"qindex\">";

   for (auto ch : lettersUsed) {
      if (headerItems) {
         alphaLinks += "&#160;|&#160;";
      }

      headerItems++;
      alphaLinks += "<a class=\"qindex\" href=\"#letter_" + letterToLabel(ch) + "\">" + ch + "</a>";
   }

   alphaLinks += "</div>\n";
   ol.writeString(alphaLinks);

   // number of columns in the table
   const int columns = Config::getInt("cols-in-index");

   int i;
   int j;

   int totalItems = headerItems * 2 + s_annotatedCount;    // number of items in the table (headers span 2 items)
   int rows = (totalItems + columns - 1) / columns;        // number of rows in the table

   // fill the columns with the class list row elements in each column,
   // expect for the columns with number >= itemsInLastRow, which get one item less
   QList<QList<AlphaIndexTableCell*> *> tableColumns;

   QList<AlphaIndexTableCell *> *tableRows = new QList<AlphaIndexTableCell *>;
   tableColumns.append(tableRows);

   int col     = 0;
   int row     = 0;
   int maxRows = 0;

   static QSharedPointer<ClassDef> dummyContext = QMakeShared<ClassDef>("", 0, 0, "dummyContext-index", CompoundType::Class);

   for (auto cl : classesByLetter) {
      QChar l = cl->letter();

      // add special header cell
      tableRows->append(new AlphaIndexTableCell(row, col, l, dummyContext));
      row++;

      tableRows->append(new AlphaIndexTableCell(row, col, 0, dummyContext));
      row++;

      //
      QSharedPointer<ClassDef> cd = cl->first();

      tableRows->append(new AlphaIndexTableCell(row, col, 0, cd));
      row++;

      if (row > maxRows) {
         maxRows = row;
      }

      if (row >= rows && col < columns) {
        ++col;
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
           ++col;
           row = 0;

           tableRows = new QList<AlphaIndexTableCell *>;
           tableColumns.append(tableRows);
         }
      }
   }

   ol.writeString("<table class=\"classindex\">\n");

   // generate table
   for (i = 0; i <= maxRows; i++) {

      // each table row
      ol.writeString("<tr>");

      // last column may contain less items then the others
      for (j = 0; j < columns; j++) {
         // each table column

         AlphaIndexTableCell *cell = nullptr;

         if (j < tableColumns.count()) {

            if (i < tableColumns[j]->count()) {
               cell = tableColumns[j]->at(i);
            }
         }

         // if cells are shifted up, add a nullptr cell in the for loop above

         if (cell) {

            if (cell->row() == i) {

               if (cell->letter() != 0) {
                  QString s = letterToLabel(cell->letter());

                  ol.writeString("<td rowspan=\"2\" valign=\"bottom\">");
                  ol.writeString("<a name=\"letter_");
                  ol.writeString(s);
                  ol.writeString("\"></a>");

                  ol.writeString("<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\">"
                                 "<tr>"
                                 "<td><div class=\"ah\">&#160;&#160;");

                  ol.writeString(cell->letter());
                  ol.writeString("&#160;&#160;</div>"
                                 "</td>"
                                 "</tr>"
                                 "</table>\n");
                  ol.writeString("</td>");

               } else if (cell->classDef() != dummyContext) {

                  QSharedPointer<ClassDef> cd = cell->classDef();

                  ol.writeString("<td valign=\"top\">");
                  QString namesp, cname;

                  extractNamespaceName(cd->name(), cname, namesp);
                  QString nsDispName;
                  SrcLangExt lang = cd->getLanguage();

                  QString sep = getLanguageSpecificSeparator(lang);

                  if (sep != "::") {
                     nsDispName = substitute(namesp, "::", sep);
                     cname = substitute(cname, "::", sep);

                  } else {
                     nsDispName = namesp;

                  }

                  nsDispName = renameNS_Aliases(nsDispName);

                  ol.writeObjectLink(cd->getReference(), cd->getOutputFileBase(), cd->anchor(), cname);

                  if (! namesp.isEmpty()) {
                     ol.docify(" (");

                     QSharedPointer<NamespaceDef> nd = getResolvedNamespace(namesp);

                     if (nd && nd->isLinkable()) {
                        ol.writeObjectLink(nd->getReference(), nd->getOutputFileBase(), QString(), nsDispName);

                     } else {
                        ol.docify(nsDispName);

                     }

                     ol.docify(")");
                  }

                  ol.writeNonBreakableSpace(3);
                  ol.writeString("</td>");

               } else {
                  // ol.writeString("<td>");
                  // ol.writeString("</td>");

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
   if (s_annotatedCount == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::ClassIndex);

   QString title    = lne ? lne->title() : theTranslator->trCompoundIndex();
   bool addToIndex  = lne == nullptr || lne->visible();

   startFile(ol, "classes", QString(), title, HLI_Classes);

   startTitle(ol, QString());
   ol.parseText(title);
   endTitle(ol, QString(), QString());

   if (addToIndex) {
      Doxy_Globals::indexList.addContentsItem(false, title, "", "classes", "", true);
   }

   ol.startContents();

   writeAlphabeticalClassList(ol);
   endFile(ol, false, false);

   ol.popGeneratorState();
}

static void writeAnnotatedIndex(OutputList &ol)
{
   if (s_annotatedCount == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Man);

   if (s_annotatedClassesPrinted == 0) {
      ol.disable(OutputGenerator::Latex);
      ol.disable(OutputGenerator::RTF);
   }

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::ClassList);
   if (lne == nullptr) {
      // fall back
      lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Classes);
   }

   QString title   = lne ? lne->title() : theTranslator->trCompoundList();
   bool addToIndex = lne == nullptr || lne->visible();

   startFile(ol, "annotated", QString(), title, HLI_Annotated);

   startTitle(ol, QString());
   ol.parseText(title);
   endTitle(ol, QString(), QString());

   ol.startContents();

   ol.startTextBlock();
   ol.parseText(lne ? lne->intro() : theTranslator->trCompoundListDescription());
   ol.endTextBlock();

   // Linear class index for Latex/RTF
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);
   Doxy_Globals::indexList.disable();

   writeAnnotatedClassList(ol);

   Doxy_Globals::indexList.enable();

   ol.popGeneratorState();

   // Hierarchical class index for HTML
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   {
      if (addToIndex) {
         Doxy_Globals::indexList.addContentsItem(true, title, "", "annotated", "", true);
         Doxy_Globals::indexList.incContentsDepth();
      }

      FTVHelp *ftv = new FTVHelp(false);
      writeNamespaceTree(Doxy_Globals::namespaceSDict, ftv, true, true, addToIndex);
      writeClassTree(Doxy_Globals::classSDict, ftv, addToIndex, true);

      QString outStr;
      QTextStream t(&outStr);

      ftv->generateTreeViewInline(t);
      ol.writeString(outStr);

      delete ftv;

      if (addToIndex) {
         Doxy_Globals::indexList.decContentsDepth();
      }
   }

   ol.popGeneratorState();
   endFile(ol, false, false);

   ol.popGeneratorState();
}

static QString writeClassLink(OutputList &ol, QSharedPointer<MemberDef> md, const QString &separator, const QString &prevName)
{
   QString retval = prevName;
   QSharedPointer<ClassDef> cd = md->getClassDef();

   if (cd) {
      QString tName = cd->displayName();

      if (prevName != tName) {
         retval = tName;

         ol.docify(separator);
         ol.writeObjectLink(md->getReference(), md->getOutputFileBase(), md->anchor(), tName);
         ol.writeString("\n");
      }
   }

   return retval;
}

static bool conceptVisibleInIndex(QSharedPointer<ConceptDef> conceptDef)
{
   static bool allExternals = Config::getBool("all-externals");
   return (allExternals && conceptDef->isLinkable()) || conceptDef->isLinkableInProject();
}

static void writeConceptTree(const ConceptSDict &conceptDict, FTVHelp *ftv, bool addToIndex, bool globalOnly)
{
   for (auto conceptDef : conceptDict) {

      if (! globalOnly || conceptDef->getOuterScope() == 0 || conceptDef->getOuterScope() == Doxy_Globals::globalScope ) {

         if (conceptVisibleInIndex(conceptDef)) {

            ftv->addContentsItem(false, conceptDef->name(), conceptDef->getReference(),
                  conceptDef->getOutputFileBase(), conceptDef->anchor(), true, conceptDef);

            if (addToIndex) {
                 Doxy_Globals::indexList.addContentsItem(false, conceptDef->name(), conceptDef->getReference(),
                      conceptDef->getOutputFileBase(), conceptDef->anchor(), true, conceptDef);
            }
         }
      }
   }
}

static void writeConceptIndex(OutputList &ol)
{
   if (s_conceptCount == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Man);

   if (s_annotatedClassesPrinted == 0) {
      ol.disable(OutputGenerator::Latex);
      ol.disable(OutputGenerator::RTF);
   }

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::ConceptList);

   QString title   = lne ? lne->title() : theTranslator->trConceptList();
   bool addToIndex = (lne == nullptr || lne->visible());

   startFile(ol, "concepts", QString(), title, HLI_Concepts);

   startTitle(ol, QString());
   ol.parseText(title);
   endTitle(ol, QString(), QString());

   ol.startContents();

   ol.startTextBlock();
   ol.parseText(lne ? lne->intro() : theTranslator->trConceptListDescription());
   ol.endTextBlock();

   bool firstEntry = true;

   // concept index for Latex/RTF
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);

   for (auto &conceptDef : Doxy_Globals::conceptSDict) {

      if (conceptDef->isLinkableInProject()) {

         if (firstEntry) {
            ol.startIndexList();
            firstEntry = false;
         }

         ol.startIndexKey();
         ol.writeObjectLink(QString(), conceptDef->getOutputFileBase(), QString(), conceptDef->displayName());
         ol.endIndexKey();

         bool hasBrief = ! conceptDef->briefDescription().isEmpty();
         ol.startIndexValue(hasBrief);

         if (hasBrief) {
            ol.generateDoc(conceptDef->briefFile(), conceptDef->briefLine(), conceptDef, QSharedPointer<MemberDef>(),
                  conceptDef->briefDescription(true), false, false, QString(), true, true);
         }

         ol.endIndexValue(conceptDef->getOutputFileBase(), hasBrief);
      }
   }

   if (! firstEntry) {
      ol.endIndexList();
   }

   ol.popGeneratorState();

   // Hierarchical class index for HTML
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   {
      if (addToIndex) {
         Doxy_Globals::indexList.addContentsItem(true, title, "", "concepts", "", true);
         Doxy_Globals::indexList.incContentsDepth();
      }

      FTVHelp *ftv = new FTVHelp(false);
      writeConceptTree(Doxy_Globals::conceptSDict, ftv, addToIndex, false);

      QString outStr;
      QTextStream t(&outStr);

      ftv->generateTreeViewInline(t);
      ol.writeString(outStr);
      delete ftv;

      if (addToIndex) {
         Doxy_Globals::indexList.decContentsDepth();
      }
   }

   ol.popGeneratorState();
   endFile(ol, false, false);

   ol.popGeneratorState();
}

static QString writeFileLink(OutputList &ol, QSharedPointer<MemberDef> md, const QString &separator, const QString &prevName)
{
   QString retval = prevName;
   QSharedPointer<FileDef> fd = md->getFileDef();

   if (fd) {
      QString tName = fd->name();

      if (prevName != tName) {
         retval = tName;
         tName  = renameNS_Aliases(tName);

         ol.docify(separator);
         ol.writeObjectLink(md->getReference(), md->getOutputFileBase(), md->anchor(), tName);
         ol.writeString("\n");
      }
   }

   return retval;
}

static QString writeNamespaceLink(OutputList &ol, QSharedPointer<MemberDef> md, const QString &separator, const QString &prevName)
{
   QString retval = prevName;
   QSharedPointer<NamespaceDef> nd = md->getNamespaceDef();

   if (nd) {
      QString tName = nd->name();

      if (prevName != tName) {
         retval = tName;

         // added 01/2016
         tName = renameNS_Aliases(tName);

         ol.docify(separator);
         ol.writeObjectLink(md->getReference(), md->getOutputFileBase(), md->anchor(), tName);
         ol.writeString("\n");
      }
   }

   return retval;
}

static void writeMemberList(OutputList &ol, bool useSections, int page,
                  const LetterToIndexMap<MemberIndexList> &memberLists, DefinitionIntf::DefType definitionType)
{
   QString prevName;
   QString prevDefName;

   bool firstEntry   = true;
   bool firstSection = true;
   bool firstItem    = true;

   for (QSharedPointer<MemberIndexList> ml : memberLists) {

      if (page != -1) {
         ml = memberLists[page];
      }

      if (ml == nullptr || ml->count() == 0) {
         continue;
      }

      for (auto md : *ml) {
         QString sep;

      bool isFunc = ! md->isObjCMethod() && (md->isFunction() || md->isSlot() || md->isSignal());

         QString name   = md->name();
         int startIndex = getPrefixIndex(name);

         if (name.mid(startIndex) != prevName) {
            // new entry

            if ((prevName.isEmpty() || name.at(startIndex).toLower() != prevName.at(0).toLower()) && useSections) {
               // new section

               if (! firstItem) {
                  ol.endItemListItem();
               }

               if (! firstSection) {
                  ol.endItemList();
               }

               QString cs = letterToLabel(ml->letter());
               QString cl = ml->letter();

               QString anchor = "index_" + convertToId(cs);
               QString title  = "- " + cl + " -";

               ol.startSection(anchor, title, SectionInfo::Subsection);
               ol.docify(title);
               ol.endSection(anchor, SectionInfo::Subsection);

               ol.startItemList();

               firstSection = false;
               firstItem    = true;

            } else if (! useSections && firstEntry) {
               ol.startItemList();
               firstEntry = false;
            }

            // member name
            if (! firstItem) {
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
            sep         = ": ";
            prevName    = name.mid(startIndex);

         } else {
            // same entry, link to class for other members with the same name
            sep = ", ";

         }

         // write the link for the specific list type
         switch (definitionType)  {
            case Definition::TypeClass:
               prevDefName = writeClassLink(ol, md, sep, prevDefName);
               break;

            case Definition::TypeFile:
               prevDefName = writeFileLink(ol, md, sep, prevDefName);
               break;

            case Definition::TypeNamespace:
               prevDefName = writeNamespaceLink(ol, md, sep, prevDefName);
               break;

            default:
               break;
         }
      }

      if (page != -1) {
         break;
      }
   }

   if (! firstItem) {
      ol.endItemListItem();
   }

   ol.endItemList();
}

void initClassMemberIndices()
{
   int j = 0;

   for (j = 0; j < CMHL_Total; j++) {
      s_documentedClassMembers[j] = 0;
      g_memberIndexLetterUsed[j].clear();
   }
}

void addClassMemberNameToIndex(QSharedPointer<MemberDef> md)
{
   static bool hideFriendCompounds = Config::getBool("hide-friend-compounds");
   QSharedPointer<ClassDef> cd;

   if (md->isLinkableInProject() && (cd = md->getClassDef())  && cd->isLinkableInProject() && cd->templateMaster() == 0) {
      QString n = md->name();

      int index    = getPrefixIndex(n);
      QChar letter = charToLower(n, index);

      if (! n.isEmpty()) {

         bool isFriendToHide = hideFriendCompounds && (md->typeString() == "friend class" ||
                                md->typeString() == "friend struct" || md->typeString() == "friend union");

         if (! (md->isFriend() && isFriendToHide) && (!md->isEnumValue() || (md->getEnumScope() && !md->getEnumScope()->isStrong()))) {
            g_memberIndexLetterUsed[CMHL_All].insertElement(letter, md);
            s_documentedClassMembers[CMHL_All]++;
         }

         if (md->isFunction() || md->isSlot() || md->isSignal()) {
            g_memberIndexLetterUsed[CMHL_Functions].insertElement(letter, md);
            s_documentedClassMembers[CMHL_Functions]++;

         } else if (md->isVariable()) {
            g_memberIndexLetterUsed[CMHL_Variables].insertElement(letter, md);
            s_documentedClassMembers[CMHL_Variables]++;

         } else if (md->isTypedef()) {
            g_memberIndexLetterUsed[CMHL_Typedefs].insertElement(letter, md);
            s_documentedClassMembers[CMHL_Typedefs]++;

         } else if (md->isEnumerate()) {
            g_memberIndexLetterUsed[CMHL_Enums].insertElement(letter, md);
            s_documentedClassMembers[CMHL_Enums]++;

         } else if (md->isEnumValue() && md->getEnumScope() && !md->getEnumScope()->isStrong()) {
            g_memberIndexLetterUsed[CMHL_EnumValues].insertElement(letter, md);
            s_documentedClassMembers[CMHL_EnumValues]++;

         } else if (md->isProperty()) {
            g_memberIndexLetterUsed[CMHL_Properties].insertElement(letter, md);
            s_documentedClassMembers[CMHL_Properties]++;

         } else if (md->isEvent()) {
            g_memberIndexLetterUsed[CMHL_Events].insertElement(letter, md);
            s_documentedClassMembers[CMHL_Events]++;
         }

         if (md->isRelated() || md->isForeign() || (md->isFriend() && ! isFriendToHide)) {
            // might also be a function
            g_memberIndexLetterUsed[CMHL_Related].insertElement(letter, md);
            s_documentedClassMembers[CMHL_Related]++;
         }
      }
   }
}

void initNamespaceMemberIndices()
{
   for (int j = 0; j < NMHL_Total; j++) {
      s_documentedNamespaceMembers[j] = 0;
      g_namespaceIndexLetterUsed[j].clear();
   }
}

void addNamespaceMemberNameToIndex(QSharedPointer<MemberDef> md)
{
   QSharedPointer<NamespaceDef> nd = md->getNamespaceDef();

   if (nd && nd->isLinkableInProject() && md->isLinkableInProject()) {

      QString n    = md->name();
      int index    = getPrefixIndex(n);
      QChar letter = charToLower(n, index);

      if (! n.isEmpty()) {
         if (! md->isEnumValue() || (md->getEnumScope() && ! md->getEnumScope()->isStrong())) {
            g_namespaceIndexLetterUsed[NMHL_All].insertElement(letter, md);
            s_documentedNamespaceMembers[NMHL_All]++;
         }

         if (md->isFunction()) {
            g_namespaceIndexLetterUsed[NMHL_Functions].insertElement(letter, md);
            s_documentedNamespaceMembers[NMHL_Functions]++;

         } else if (md->isVariable()) {
            g_namespaceIndexLetterUsed[NMHL_Variables].insertElement(letter, md);
            s_documentedNamespaceMembers[NMHL_Variables]++;

         } else if (md->isTypedef()) {
            g_namespaceIndexLetterUsed[NMHL_Typedefs].insertElement(letter, md);
            s_documentedNamespaceMembers[NMHL_Typedefs]++;

         } else if (md->isEnumerate()) {
            g_namespaceIndexLetterUsed[NMHL_Enums].insertElement(letter, md);
            s_documentedNamespaceMembers[NMHL_Enums]++;

         } else if (md->isEnumValue() && md->getEnumScope() && !md->getEnumScope()->isStrong()) {
            g_namespaceIndexLetterUsed[NMHL_EnumValues].insertElement(letter, md);
            s_documentedNamespaceMembers[NMHL_EnumValues]++;
         }
      }
   }
}

void initFileMemberIndices()
{
   int j = 0;

   for (j = 0; j < NMHL_Total; j++) {
      s_documentedFileMembers[j] = 0;
      g_fileIndexLetterUsed[j].clear();
   }
}

void addFileMemberNameToIndex(QSharedPointer<MemberDef> md)
{
   QSharedPointer<FileDef> fd = md->getFileDef();

   if (fd && fd->isLinkableInProject() && md->isLinkableInProject()) {
      QString n = md->name();

      int index    = getPrefixIndex(n);
      QChar letter = charToLower(n, index);

      if (! n.isEmpty()) {

         if (!md->isEnumValue() || (md->getEnumScope() && !md->getEnumScope()->isStrong())) {
            g_fileIndexLetterUsed[FMHL_All].insertElement(letter, md);
            s_documentedFileMembers[FMHL_All]++;
         }

         if (md->isFunction()) {
            g_fileIndexLetterUsed[FMHL_Functions].insertElement(letter, md);
            s_documentedFileMembers[FMHL_Functions]++;

         } else if (md->isVariable()) {
            g_fileIndexLetterUsed[FMHL_Variables].insertElement(letter, md);
            s_documentedFileMembers[FMHL_Variables]++;

         } else if (md->isTypedef()) {
            g_fileIndexLetterUsed[FMHL_Typedefs].insertElement(letter, md);
            s_documentedFileMembers[FMHL_Typedefs]++;

         } else if (md->isEnumerate()) {
            g_fileIndexLetterUsed[FMHL_Enums].insertElement(letter, md);
            s_documentedFileMembers[FMHL_Enums]++;

         } else if (md->isEnumValue() && md->getEnumScope() && !md->getEnumScope()->isStrong()) {
            g_fileIndexLetterUsed[FMHL_EnumValues].insertElement(letter, md);
            s_documentedFileMembers[FMHL_EnumValues]++;

         } else if (md->isDefine()) {
            g_fileIndexLetterUsed[FMHL_Defines].insertElement(letter, md);
            s_documentedFileMembers[FMHL_Defines]++;
         }
      }
   }
}

static void writeQuickMemberIndex(OutputList &ol, const LetterToIndexMap<MemberIndexList> &charUsed,
                  QChar page, QString fullName, bool multiPage)
{
   startQuickIndexList(ol, true);

   for (auto ml : charUsed) {
      QChar i = ml->letter();

      QString is = letterToLabel(i);
      QString ci = QChar(i);

      QString anchor;
      QString extension = Doxy_Globals::htmlFileExtension;

      if (multiPage) {
         anchor = fullName + "_" + letterToLabel(i) + extension + "#index_";

      } else  {
         // only one page
         anchor = "#index_";
      }

      startQuickIndexItem(ol, anchor + convertToId(ci), i == page, true);
      ol.writeString(is);
      endQuickIndexItem(ol);
   }

   endQuickIndexList(ol);
}

/** Helper class representing a class member in the navigation menu. */
struct CmhlInfo {
   CmhlInfo(const QString &fn, const QString &t)
      : fname(fn), title(t), link(fn) { }

   QString fname;
   QString title;
   QString link;
};

static CmhlInfo *getCmhlInfo(int hl)
{
   static bool fortranOpt = Config::getBool("optimize-fortran");

   static CmhlInfo cmhlInfo[] = {
      CmhlInfo("functions_all",     theTranslator->trAll()),
      CmhlInfo("functions_func",    fortranOpt ? theTranslator->trSubprograms() : theTranslator->trFunctions()),
      CmhlInfo("functions_vars",    theTranslator->trVariables()),
      CmhlInfo("functions_type",    theTranslator->trTypedefs()),
      CmhlInfo("functions_enum",    theTranslator->trEnumerations()),
      CmhlInfo("functions_eval",    theTranslator->trEnumerationValues()),
      CmhlInfo("functions_prop",    theTranslator->trProperties()),
      CmhlInfo("functions_event",   theTranslator->trEvents()),
      CmhlInfo("functions_related", theTranslator->trRelatedFunctions())
   };

   return &cmhlInfo[hl];
}

static void writeClassMemberIndexFiltered(OutputList &ol, ClassMemberHighlight hl)
{
   if (s_documentedClassMembers[hl] == 0) {
      return;
   }

   static bool disableIndex = Config::getBool("disable-index");

   bool multiPageIndex = false;
   if (s_documentedClassMembers[hl] > MAX_ITEMS_BEFORE_MULTIPAGE_INDEX) {
      multiPageIndex = true;
   }

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   QString extension = Doxy_Globals::htmlFileExtension;
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::ClassMembers);

   QString title = lne ? lne->title() : theTranslator->trCompoundMembers();

   if (hl != CMHL_All) {
      title += " - " + getCmhlInfo(hl)->title;
   }

   bool addToIndex = lne == nullptr || lne->visible();

   if (addToIndex) {
      Doxy_Globals::indexList.addContentsItem(multiPageIndex, getCmhlInfo(hl)->title, "",
                  getCmhlInfo(hl)->link, "", true);

      if (multiPageIndex) {
         Doxy_Globals::indexList.incContentsDepth();
      }
   }

   for (auto ml : g_memberIndexLetterUsed[hl]) {
      QChar page = ml->letter();
      QString fileName = getCmhlInfo(hl)->fname;

      if (multiPageIndex) {
         fileName += "_" + letterToLabel(page);

         QString cs = QChar(page);
         if (addToIndex) {
            Doxy_Globals::indexList.addContentsItem(false, cs, "", fileName, "", true);
         }
      }

      bool quickIndex = s_documentedClassMembers[hl] > maxItemsBeforeQuickIndex;

      ol.startFile(fileName + extension, QString(), title);
      ol.startQuickIndices();

      if (! disableIndex) {
         ol.writeQuickLinks(true, HLI_Functions, QString());

         startQuickIndexList(ol);

         // index item for global member list
         startQuickIndexItem(ol, getCmhlInfo(CMHL_All)->link + Doxy_Globals::htmlFileExtension, hl == CMHL_All, true);
         ol.writeString(fixSpaces(getCmhlInfo(CMHL_All)->title));
         endQuickIndexItem(ol);

         // index items per category member lists
         for (int i = 1; i < CMHL_Total; i++) {
            if (s_documentedClassMembers[i] > 0) {
               startQuickIndexItem(ol, getCmhlInfo(i)->link + Doxy_Globals::htmlFileExtension, hl == i, true);

               ol.writeString(fixSpaces(getCmhlInfo(i)->title));
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
         ol.parseText(lne ? lne->intro() : theTranslator->trCompoundMembersDescription(Config::getBool("extract-all")));
         ol.endTextBlock();

      } else {
         // work around mozilla problem, refuses to switch to normal lists otherwise
         ol.writeString("&#160;");
      }

      writeMemberList(ol, quickIndex, multiPageIndex ? page.unicode() : -1, g_memberIndexLetterUsed[hl], Definition::TypeClass);

      endFile(ol, false, false);
   }

   if (multiPageIndex && addToIndex) {
      Doxy_Globals::indexList.decContentsDepth();
   }

   ol.popGeneratorState();
}

static void ClassMemberRedirect(QString fName, QString link)
{
   // open the file
   QString ext = Doxy_Globals::htmlFileExtension;
   QString outputName = Config::getString("html-output") + "/" + fName + ext;

   QFile fout(outputName);

   if (! fout.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fName), fout.error());
      Doxy_Work::stopDoxyPress();
   }

   // Write the header
   QTextStream t_stream(&fout);

   t_stream << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n";
   t_stream << "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n";
   t_stream << "<head>\n";
   t_stream << "<meta http-equiv=\"refresh\" content=\"0;" + link + ext + "\">\n";
   t_stream << "</head>\n</html>\n";

   t_stream.flush();
   fout.close();
}

static void writeClassMemberIndex(OutputList &ol)
{
   static const bool generateHtml = Config::getBool("generate-html");

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::ClassMembers);
   bool addToIndex = lne == nullptr || lne->visible();

   if (s_documentedClassMembers[CMHL_All] > 0 && addToIndex) {
      Doxy_Globals::indexList.addContentsItem(true, lne ? lne->title() : theTranslator->trCompoundMembers(), "", "functions_all", "");
      Doxy_Globals::indexList.incContentsDepth();
   }

   if (generateHtml) {
      // figure out the correct names for the links

      for (int k = 0; k < CMHL_Total; k++) {
         QString fName = getCmhlInfo(k)->fname;
         QString link  = fName;

         if (s_documentedClassMembers[k] > MAX_ITEMS_BEFORE_MULTIPAGE_INDEX) {

            // (part 1) what is is the first letter of the list
            const LetterToIndexMap<MemberIndexList> &charUsed = g_memberIndexLetterUsed[k];

            auto value = *(charUsed.begin());
            QChar firstLetter = value->letter();

            link += QString("_") + QChar(firstLetter);

            // save back
            getCmhlInfo(k)->link = link;

            // (part 2) create redirect page
            ClassMemberRedirect(fName, link);

         }  else {
            // save unchanged link
            getCmhlInfo(k)->link = link;
         }
      }
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

  if (s_documentedClassMembers[CMHL_All] > 0 && addToIndex) {
      Doxy_Globals::indexList.decContentsDepth();
   }
}

/** Helper class representing a file member in the navigation menu. */
struct FmhlInfo {
   FmhlInfo(const QString &fn, const QString &t)
      : fname(fn), title(t), link(fn)
   {
   }

   QString fname;
   QString title;
   QString link;
};

static FmhlInfo *getFmhlInfo(int hl)
{
   static bool fortranOpt = Config::getBool("optimize-fortran");

   static FmhlInfo fmhlInfo[] = {
      FmhlInfo("globals",      theTranslator->trAll()),
      FmhlInfo("globals_func", fortranOpt ? theTranslator->trSubprograms() : theTranslator->trFunctions()),
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
   if (s_documentedFileMembers[hl] == 0) {
      return;
   }

   static bool disableIndex = Config::getBool("disable-index");

   bool multiPageIndex = false;
   if (s_documentedFileMembers[hl] > MAX_ITEMS_BEFORE_MULTIPAGE_INDEX) {
      multiPageIndex = true;
   }

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   QString extension = Doxy_Globals::htmlFileExtension;

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::FileGlobals);

   QString title   = lne ? lne->title() : theTranslator->trFileMembers();
   bool addToIndex = lne == nullptr || lne->visible();

   if (addToIndex) {
      Doxy_Globals::indexList.addContentsItem(multiPageIndex, getFmhlInfo(hl)->title, "", getFmhlInfo(hl)->fname, "", true);

      if (multiPageIndex) {
         Doxy_Globals::indexList.incContentsDepth();
      }
   }

   for (auto ml : g_fileIndexLetterUsed[hl]) {
      QChar page = ml->letter();
      QString fileName = getFmhlInfo(hl)->fname;

      if (multiPageIndex) {
         fileName += "_" + letterToLabel(page);

         const QString cs = QChar(page);
         if (addToIndex) {
            Doxy_Globals::indexList.addContentsItem(false, cs, "", fileName, "", true);
         }
      }

      bool quickIndex = s_documentedFileMembers[hl] > maxItemsBeforeQuickIndex;

      ol.startFile(fileName + extension, QString(), title);
      ol.startQuickIndices();

      if (! disableIndex) {
         ol.writeQuickLinks(true, HLI_Globals, QString());
         startQuickIndexList(ol);

         // index item for all file member lists
         startQuickIndexItem(ol, getFmhlInfo(0)->fname + Doxy_Globals::htmlFileExtension, hl == FMHL_All, true);

         ol.writeString(fixSpaces(getFmhlInfo(0)->title));
         endQuickIndexItem(ol);

         // index items for per category member lists
         for (int i = 1; i < FMHL_Total; i++) {
            if (s_documentedFileMembers[i] > 0) {
               startQuickIndexItem(ol, getFmhlInfo(i)->fname + Doxy_Globals::htmlFileExtension, hl == i, true);
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
         ol.parseText(lne ? lne->intro() : theTranslator->trFileMembersDescription(Config::getBool("extract-all")));
         ol.endTextBlock();

      } else {
         // work around mozilla problem, refuses to switch to normal lists otherwise
         ol.writeString("&#160;");
      }

      writeMemberList(ol, quickIndex, multiPageIndex ? page.unicode() : -1, g_fileIndexLetterUsed[hl], Definition::TypeFile);

      endFile(ol, false, false);
   }

   if (multiPageIndex && addToIndex) {
      Doxy_Globals::indexList.decContentsDepth();
   }

   ol.popGeneratorState();
}

static void writeFileMemberIndex(OutputList &ol)
{
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::FileGlobals);
   bool addToIndex = lne == nullptr || lne->visible();

   if (s_documentedFileMembers[FMHL_All] > 0 && addToIndex) {
      Doxy_Globals::indexList.addContentsItem(false, lne ? lne->title() : theTranslator->trFileMembers(), "", "globals", "");
      Doxy_Globals::indexList.incContentsDepth();
   }

   // figure out the correct names for the links
   for (int k = 0; k < FMHL_Total; k++) {

      QString fName = getFmhlInfo(k)->fname;
      QString link  = fName;

      if (s_documentedFileMembers[k] > MAX_ITEMS_BEFORE_MULTIPAGE_INDEX) {

         // (part 1) what is is the first letter of the list
         const LetterToIndexMap<MemberIndexList> &charUsed = g_fileIndexLetterUsed[k];

         auto value = *(charUsed.begin());
         QChar firstLetter = value->letter();

         link += QString("_") + QChar(firstLetter);

         // save back
         getFmhlInfo(k)->link = link;

         // (part 2) create redirect page
         ClassMemberRedirect(fName, link);

      }  else {
         // save unchanged link
         getFmhlInfo(k)->link = link;
      }
   }

   writeFileMemberIndexFiltered(ol, FMHL_All);
   writeFileMemberIndexFiltered(ol, FMHL_Functions);
   writeFileMemberIndexFiltered(ol, FMHL_Variables);
   writeFileMemberIndexFiltered(ol, FMHL_Typedefs);
   writeFileMemberIndexFiltered(ol, FMHL_Enums);
   writeFileMemberIndexFiltered(ol, FMHL_EnumValues);
   writeFileMemberIndexFiltered(ol, FMHL_Defines);

   if (s_documentedFileMembers[FMHL_All] > 0 && addToIndex) {
      Doxy_Globals::indexList.decContentsDepth();
   }
}

/** Helper class representing a namespace member in the navigation menu. */
struct NmhlInfo {
   NmhlInfo(const QString &fn, const QString &t) : fname(fn), title(t) {}

   QString fname;
   QString title;
};

static const NmhlInfo *getNmhlInfo(int hl)
{
   static bool fortranOpt = Config::getBool("optimize-fortran");

   static NmhlInfo nmhlInfo[] = {
      NmhlInfo("namespacemembers",      theTranslator->trAll()),
      NmhlInfo("namespacemembers_func", fortranOpt ? theTranslator->trSubprograms() : theTranslator->trFunctions()),
      NmhlInfo("namespacemembers_vars", theTranslator->trVariables()),
      NmhlInfo("namespacemembers_type", theTranslator->trTypedefs()),
      NmhlInfo("namespacemembers_enum", theTranslator->trEnumerations()),
      NmhlInfo("namespacemembers_eval", theTranslator->trEnumerationValues())
   };

   return &nmhlInfo[hl];
}

static void writeNamespaceMemberIndexFiltered(OutputList &ol, NamespaceMemberHighlight hl)
{
   if (s_documentedNamespaceMembers[hl] == 0) {
      return;
   }

   static bool disableIndex = Config::getBool("disable-index");

   bool multiPageIndex = false;
   if (s_documentedNamespaceMembers[hl] > MAX_ITEMS_BEFORE_MULTIPAGE_INDEX) {
      multiPageIndex = true;
   }

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   QString extension = Doxy_Globals::htmlFileExtension;
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::NamespaceMembers);

   QString title   = lne ? lne->title() : theTranslator->trNamespaceMembers();
   bool addToIndex = lne == nullptr || lne->visible();

   if (addToIndex) {
      Doxy_Globals::indexList.addContentsItem(multiPageIndex, getNmhlInfo(hl)->title, "",
                  getNmhlInfo(hl)->fname, "", true);

      if (multiPageIndex) {
         Doxy_Globals::indexList.incContentsDepth();
      }
   }

   for (auto ml : g_namespaceIndexLetterUsed[hl]) {

      QChar page = ml->letter();
      QString fileName = getNmhlInfo(hl)->fname;

      if (multiPageIndex) {
         fileName += "_" + letterToLabel(page);

         const QString cs = QChar(page);
         if (addToIndex) {
            Doxy_Globals::indexList.addContentsItem(false, cs, "", fileName, "", true);
         }
      }
      bool quickIndex = s_documentedNamespaceMembers[hl] > maxItemsBeforeQuickIndex;

      ol.startFile(fileName + extension, QString(), title);
      ol.startQuickIndices();

      if (! disableIndex) {
         ol.writeQuickLinks(true, HLI_NamespaceMembers, QString());
         startQuickIndexList(ol);

         // index item for all namespace member lists
         startQuickIndexItem(ol, getNmhlInfo(0)->fname + Doxy_Globals::htmlFileExtension, hl == NMHL_All, true);
         ol.writeString(fixSpaces(getNmhlInfo(0)->title));
         endQuickIndexItem(ol);

         // index items per category member lists
         for (int i = 1; i < NMHL_Total; i++) {
            if (s_documentedNamespaceMembers[i] > 0) {
               startQuickIndexItem(ol, getNmhlInfo(i)->fname + Doxy_Globals::htmlFileExtension, hl == i, true);
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
         ol.parseText(lne ? lne->intro() : theTranslator->trNamespaceMemberDescription(Config::getBool("extract-all")));
         ol.endTextBlock();

      } else {
         // work around mozilla problem, refuses to switch to normal lists otherwise
         ol.writeString("&#160;");
      }

      writeMemberList(ol, quickIndex, multiPageIndex ? page.unicode() : -1, g_namespaceIndexLetterUsed[hl], Definition::TypeNamespace);
      endFile(ol, false, false);
   }

   if (multiPageIndex && addToIndex) {
      Doxy_Globals::indexList.decContentsDepth();
   }

   ol.popGeneratorState();
}

static void writeNamespaceMemberIndex(OutputList &ol)
{
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::NamespaceMembers);
   bool addToIndex = lne == nullptr || lne->visible();

   if (s_documentedNamespaceMembers[NMHL_All] > 0 && addToIndex) {
      Doxy_Globals::indexList.addContentsItem(false, lne ? lne->title() : theTranslator->trNamespaceMembers(),
                  "", "namespacemembers", "");

      Doxy_Globals::indexList.incContentsDepth();
   }

   // bool fortranOpt = Config::getBool("optimize-fortran");

   writeNamespaceMemberIndexFiltered(ol, NMHL_All);
   writeNamespaceMemberIndexFiltered(ol, NMHL_Functions);
   writeNamespaceMemberIndexFiltered(ol, NMHL_Variables);
   writeNamespaceMemberIndexFiltered(ol, NMHL_Typedefs);
   writeNamespaceMemberIndexFiltered(ol, NMHL_Enums);
   writeNamespaceMemberIndexFiltered(ol, NMHL_EnumValues);

   if (s_documentedNamespaceMembers[NMHL_All] > 0 && addToIndex) {
      Doxy_Globals::indexList.decContentsDepth();
   }
}

static void writeExampleIndex(OutputList &ol)
{
   if (Doxy_Globals::exampleSDict.count() == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Docbook);
   ol.disable(OutputGenerator::Man);

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Examples);

   QString title = lne ? lne->title() : theTranslator->trExamples();
   bool addToIndex = lne == nullptr || lne->visible();

   startFile(ol, "examples", QString(), title, HLI_Examples);

   startTitle(ol, QString());
   ol.parseText(title);
   endTitle(ol, QString(), QString());

   ol.startContents();

   if (addToIndex) {
      Doxy_Globals::indexList.addContentsItem(true, title, QString(), "examples", QString(), true);
      Doxy_Globals::indexList.incContentsDepth();
   }

   ol.startTextBlock();
   ol.parseText(lne ? lne->intro() : theTranslator->trExamplesDescription());
   ol.endTextBlock();

   ol.startItemList();

   for (auto &pd : Doxy_Globals::exampleSDict) {
      ol.startItemListItem();
      QString n = pd->getOutputFileBase();

      if (! pd->title().isEmpty()) {
         ol.writeObjectLink(QString(), n, QString(), pd->title());

         if (addToIndex) {
            Doxy_Globals::indexList.addContentsItem(false, filterTitle(pd->title()), pd->getReference(), n, QString(), true, pd);
         }

      } else {
         ol.writeObjectLink(QString(), n, QString(), pd->name());
         if (addToIndex) {
            Doxy_Globals::indexList.addContentsItem(false, pd->name(), pd->getReference(), n, QString(), true, pd);
         }
      }

      ol.endItemListItem();
      ol.writeString("\n");
   }

   ol.endItemList();

   if (addToIndex) {
      Doxy_Globals::indexList.decContentsDepth();
   }
   endFile(ol, false, false);
   ol.popGeneratorState();
}

static bool mainPageHasOwnTitle()
{
   static QString projectName = Config::getString("project-name");
   QString title;

   if (Doxy_Globals::mainPage) {
      title = filterTitle(Doxy_Globals::mainPage->title());
   }

   return ! projectName.isEmpty() && mainPageHasTitle() && title.compare(projectName, Qt::CaseInsensitive) != 0;
}

static void writePages(QSharedPointer<PageDef> pd, FTVHelp *ftv)
{
   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Pages);

   bool addToIndex = (lne == nullptr || lne->visible());

   if (! addToIndex) {
      return;
   }

   bool hasSubPages = pd->hasSubPages();
   bool hasSections = pd->hasSections();

   if (pd->visibleInIndex()) {
      QString pageTitle;

      if (pd->title().isEmpty()) {
         pageTitle = pd->name();

      } else {
         pageTitle = filterTitle(pd->title());
      }

      if (ftv) {
         ftv->addContentsItem(hasSubPages, pageTitle, pd->getReference(), pd->getOutputFileBase(),
                  "", true, pd);
      }

      if (addToIndex && pd != Doxy_Globals::mainPage) {
         Doxy_Globals::indexList.addContentsItem(hasSubPages || hasSections, pageTitle, pd->getReference(),
                  pd->getOutputFileBase(), "", true, pd);
      }
   }

   if (hasSubPages && ftv) {
      ftv->incContentsDepth();
   }

   bool doIndent = (hasSections || hasSubPages) && (pd != Doxy_Globals::mainPage || mainPageHasOwnTitle());
   if (doIndent) {
      Doxy_Globals::indexList.incContentsDepth();
   }

   if (hasSections) {

      if (pd == Doxy_Globals::mainPage) {
         // do not add sections from mainpage to treeview
         pd->addSectionsToIndex(false);

      } else {
         pd->addSectionsToIndex(true);
      }
   }

   PageSDict *subPages = pd->getSubPages();

   if (subPages) {
      for (auto subPage : *subPages) {
         writePages(subPage, ftv);
      }
   }

   if (hasSubPages && ftv) {
      ftv->decContentsDepth();
   }

   if (doIndent) {
      Doxy_Globals::indexList.decContentsDepth();
   }
}

static void writePageIndex(OutputList &ol)
{
   if (Doxy_Globals::indexedPages == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Pages);

   QString title = lne ? lne->title() : theTranslator->trRelatedPages();
   startFile(ol, "pages", QString(), title, HLI_Pages);

   startTitle(ol, QString());
   ol.parseText(title);
   endTitle(ol, QString(), QString());

   ol.startContents();
   ol.startTextBlock();
   ol.parseText(lne ? lne->intro() : theTranslator->trRelatedPagesDescription());
   ol.endTextBlock();

   FTVHelp *ftv = new FTVHelp(false);

   for (auto &pd : Doxy_Globals::pageSDict) {
      if ((pd->getOuterScope() == 0 || pd->getOuterScope()->definitionType() != Definition::TypePage) && ! pd->isReference() ) {
         writePages(pd, ftv);
      }
   }

   QString outStr;

   QTextStream t(&outStr);
   ftv->generateTreeViewInline(t);
   ol.writeString(outStr);

   delete ftv;

   endFile(ol, false, false);
   ol.popGeneratorState();
}

void writeGraphInfo(OutputList &ol)
{
   static const bool generateHtml = Config::getBool("generate-html");

   if (! generateHtml) {
      return;
   }

   // used in the doxypress test build to show sample translations
   static const QString projectDesc = Config::getString("project-brief");
   static const bool haveDot        = Config::getBool("have-dot");

   if (! haveDot && projectDesc != "Sample files to test the functionality of DoxyPress")  {
      return;
   }

   static const bool stripCommentsStateRef = Config::getBool("strip-code-comments");
   static const bool createSubdirs         = Config::getBool("create-subdirs");

   // temporarily disable the stripping of comments for our own code example
   Config::setBool("strip-code-comments", false);

   // temporarily disable create subdirs for linking to our example
   Config::setBool("create-subdirs", false);

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   generateGraphLegend(Config::getString("html-output"));

   bool oldStripCommentsState  = stripCommentsStateRef;
   bool oldCreateSubdirs       = createSubdirs;

   startFile(ol, "graph_legend", QString(), theTranslator->trLegendTitle());

   startTitle(ol, QString());
   ol.parseText(theTranslator->trLegendTitle());

   endTitle(ol, QString(), QString());
   ol.startContents();

   //
   static const QString imageExt = Config::getEnum("dot-image-extension");
   QString legendDocs = theTranslator->trLegendDocs(imageExt);

   int s = legendDocs.indexOf("<center>");
   int e = legendDocs.indexOf("</center>");

   if (imageExt == "svg" && s != -1 && e != -1) {
      legendDocs = legendDocs.left(s + 8) + "[!-- SVG 0 --]\n" + legendDocs.mid(e);
   }

   QSharedPointer<FileDef> fd = QMakeShared<FileDef>("", "graph_legend");
   ol.generateDoc("graph_legend", 1, fd, QSharedPointer<MemberDef>(), legendDocs, false, false);

   // restore config settings
   Config::setBool("strip-code-comments", oldStripCommentsState);
   Config::setBool("create-subdirs", oldCreateSubdirs);

   endFile(ol, false, false);
   ol.popGeneratorState();
}

/*!
 * write groups as hierarchical trees
 */
static void writeGroupTreeNode(OutputList &ol, QSharedPointer<GroupDef> gd, int level, FTVHelp *ftv, bool addToIndex)
{
   // bool fortranOpt = Config::getBool("optimize-fortran");

   if (level > 20) {
      warn(gd->getDefFileName(), gd->getDefLine(), "Maximum nesting level exceeded for group %s: check for possible "
         "recursive group relation\n", csPrintable(gd->name()) );

      return;
   }

   // Some groups should appear twice under different parent-groups. Do not check if it was visted
   if ( (! gd->isASubGroup() || level > 0) && gd->isVisible() &&
        (! gd->isReference() || Config::getBool("external-groups")) ) {

      // write group info
      bool hasSubGroups = gd->getSubGroups()->count() > 0;
      bool hasSubPages  = gd->getPages()->count() > 0;
      int numSubItems   = 0;

      for (auto ml : gd->getMemberLists()) {
         if (ml->listType() & MemberListType_documentationLists) {
            numSubItems += ml->count();
         }
      }

      numSubItems += gd->getNamespaces().count();
      numSubItems += gd->getClasses().count();
      numSubItems += gd->getFiles().count();
      numSubItems += gd->getDirs()->count();
      numSubItems += gd->getPages()->count();

      bool isDir = hasSubGroups || hasSubPages || numSubItems > 0;

      if (addToIndex) {
         Doxy_Globals::indexList.addContentsItem(isDir, gd->groupTitle(), gd->getReference(), gd->getOutputFileBase(),
                  "", true, gd);

         Doxy_Globals::indexList.incContentsDepth();
      }

      if (ftv) {
         ftv->addContentsItem(hasSubGroups, gd->groupTitle(), gd->getReference(), gd->getOutputFileBase(), "", false, gd);
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
            QSharedPointer<MemberList> ml = gd->getMemberList(lmd->type);

            if (ml) {
               for (auto md : *ml) {
                  QSharedPointer<MemberList> enumList = md->enumFieldList();
                  bool isDir = enumList != 0 && md->isEnumerate();

                  if (md->isVisible() && md->name().indexOf('@') == -1) {
                     Doxy_Globals::indexList.addContentsItem(isDir, md->name(), md->getReference(), md->getOutputFileBase(),
                           md->anchor(), addToIndex, md);
                  }

                  if (isDir) {
                     Doxy_Globals::indexList.incContentsDepth();

                     for (auto emd : *enumList) {
                        if (emd->isVisible()) {
                           Doxy_Globals::indexList.addContentsItem(false, emd->name(), emd->getReference(), emd->getOutputFileBase(),
                                 emd->anchor(), addToIndex, emd);
                        }
                     }

                     Doxy_Globals::indexList.decContentsDepth();
                  }
               }
            }


         } else if (lde->kind() == LayoutDocEntry::GroupClasses && addToIndex) {

            for (auto cd : gd->getClasses()) {
                if (cd->isVisible()) {
                  addMembersToIndex(cd, LayoutDocManager::Class, cd->displayName(false), cd->anchor(), true, addToIndex);
               }
            }

         } else if (lde->kind() == LayoutDocEntry::GroupNamespaces && addToIndex) {

            for (const auto &nd : gd->getNamespaces()) {
               if (nd->isVisible()) {
                  Doxy_Globals::indexList.addContentsItem(false, nd->localName(), nd->getReference(),
                        nd->getOutputFileBase(), QString(), false, nd);
               }
            }

         } else if (lde->kind() == LayoutDocEntry::GroupFiles && addToIndex) {

            for (const auto fd : gd->getFiles()) {
               if (fd->isVisible()) {
                  Doxy_Globals::indexList.addContentsItem(false, fd->displayName(), fd->getReference(),
                        fd->getOutputFileBase(), QString(), false, fd);
               }
            }

         } else if (lde->kind() == LayoutDocEntry::GroupDirs && addToIndex) {

            for (const auto dd : *gd->getDirs()) {
               if (dd->isVisible()) {
                  Doxy_Globals::indexList.addContentsItem(false, dd->shortName(), dd->getReference(),
                        dd->getOutputFileBase(), QString(), false, dd);
               }
            }

         } else if (lde->kind() == LayoutDocEntry::GroupPageDocs && addToIndex) {

            for (auto pd : *gd->getPages()) {
               QSharedPointer<SectionInfo> si;

               if (! pd->name().isEmpty()) {
                  si = Doxy_Globals::sectionDict.find(pd->name());
               }

               bool hasSubPages = pd->hasSubPages();
               bool hasSections = pd->hasSections();

               Doxy_Globals::indexList.addContentsItem( hasSubPages || hasSections, convertToHtml(pd->title(), true),
                     gd->getReference(), gd->getOutputFileBase(), si ? si->label : "", true);

               // addToNavIndex
               if (hasSections || hasSubPages) {
                  Doxy_Globals::indexList.incContentsDepth();
               }

               if (hasSections) {
                  // always add these sections
                  pd->addSectionsToIndex(true);
               }

               writePages(pd, 0);
               if (hasSections || hasSubPages) {
                  Doxy_Globals::indexList.decContentsDepth();
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
         Doxy_Globals::indexList.decContentsDepth();
      }

      if (ftv) {
         ftv->decContentsDepth();
      }
   }
}

static void writeGroupHierarchy(OutputList &ol, FTVHelp *ftv, bool addToIndex)
{
   if (ftv) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
   }

   startIndexHierarchy(ol, 0);

   for (auto gd : Doxy_Globals::groupSDict) {
      writeGroupTreeNode(ol, gd, 0, ftv, addToIndex);
   }

   endIndexHierarchy(ol, 0);

   if (ftv) {
      ol.popGeneratorState();
   }
}

static void writeGroupIndex(OutputList &ol)
{
   // modules.html

   if (s_documentedGroups == 0) {
      return;
   }

   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Docbook);
   ol.disable(OutputGenerator::Man);

   LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::Modules);

   QString title = lne ? lne->title() : theTranslator->trModules();
   bool addToIndex = lne == nullptr || lne->visible();

   startFile(ol, "modules", QString(), title, HLI_Modules);

   startTitle(ol, QString());
   ol.parseText(title);
   endTitle(ol, QString(), QString());

   ol.startContents();

   ol.startTextBlock();

   if (Config::getBool("bb-style")) {
      ol.parseText("Here is a list of all components:");

   } else if (lne) {
      ol.parseText(lne->intro());

   } else {
      ol.parseText(theTranslator->trModulesDescription());
   }

   ol.endTextBlock();

   // Normal group index for Latex/RTF
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);
   Doxy_Globals::indexList.disable();

   writeGroupHierarchy(ol, nullptr, false);

   Doxy_Globals::indexList.enable();
   ol.popGeneratorState();

   // interactive group index for HTML
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   {
      if (addToIndex) {
         Doxy_Globals::indexList.addContentsItem(true, title, "", "modules", "", true);
         Doxy_Globals::indexList.incContentsDepth();
      }

      FTVHelp *ftv = new FTVHelp(false);
      writeGroupHierarchy(ol, ftv, addToIndex);

      QString outStr;

      QTextStream t(&outStr);
      ftv->generateTreeViewInline(t, FTVHelp::Modules);
      ol.disableAllBut(OutputGenerator::Html);
      ol.writeString(outStr);

      delete ftv;

      if (addToIndex) {
         Doxy_Globals::indexList.decContentsDepth();
      }
   }

   ol.popGeneratorState();

   endFile(ol, false, false);
   ol.popGeneratorState();
}

static void writeUserGroupStubPage(OutputList &ol, LayoutNavEntry *lne)
{
   if (lne->baseFile().startsWith("usergroup")) {
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);

      startFile(ol, lne->baseFile(), QString(), lne->title(), HLI_UserGroup);
      startTitle(ol, QString());

      ol.parseText(lne->title());
      endTitle(ol, QString(), QString());

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

      endFile(ol, false, false);
      ol.popGeneratorState();
   }
}

static void writeIndex(OutputList &ol)
{
   static const QString projectName    = Config::getString("project-name");
   static const QString projectVersion = Config::getString("project-version");

   static const bool optimizeFortran   = Config::getBool("optimize-fortran");
   static const bool latexHideIndices  = Config::getBool("latex-hide-indices");
   static const bool showNamespacePage = Config::getBool("show-namespace-page");
   static const bool showFilePage      = Config::getBool("show-file-page");
   static const bool disableIndex      = Config::getBool("disable-index");

   // save old generator state
   ol.pushGeneratorState();

   QString projPrefix;
   if (! projectName.isEmpty()) {
      projPrefix = projectName + " ";
   }

   // write HTML index mainpage
   ol.disableAllBut(OutputGenerator::Html);

   QString defFileName = "[generated]";
   int defLine         = -1;

   if (Doxy_Globals::mainPage != nullptr) {
      defFileName = Doxy_Globals::mainPage->docFile();
      defLine     = Doxy_Globals::mainPage->docLine();
   }

   QString title;

   if (mainPageHasTitle()) {

      if (Doxy_Globals::mainPage != nullptr) {
         title = filterTitle(Doxy_Globals::mainPage->title());
      }

   } else {
      title = theTranslator->trMainPage();
   }

   QString indexName = "index";
   ol.startFile(indexName, QString(), title);

   if (Doxy_Globals::mainPage != nullptr) {

      if ( (! projectName.isEmpty() && mainPageHasTitle() && title.compare(projectName, Qt::CaseInsensitive) != 0)) {

         Doxy_Globals::indexList.addContentsItem(Doxy_Globals::mainPage->hasSubPages(), title, "", indexName, "",
                  true, Doxy_Globals::mainPage);
      }

      // to avoid duplicate entries in the treeview

      if (Doxy_Globals::mainPage->hasSubPages() || Doxy_Globals::mainPage->hasSections()) {
         writePages(Doxy_Globals::mainPage, nullptr);
      }
   }

   ol.startQuickIndices();
   if (! disableIndex) {
      ol.writeQuickLinks(true, HLI_Main, QString());
   }

   ol.endQuickIndices();
   ol.writeSplitBar(indexName);
   ol.writeSearchInfo();

   bool headerWritten = false;

   if (Doxy_Globals::mainPage != nullptr) {
      if (! Doxy_Globals::mainPage->title().isEmpty()) {

         if (Doxy_Globals::mainPage->title().toLower() != "notitle") {
            ol.startPageDoc(Doxy_Globals::mainPage->title());
         } else {
            ol.startPageDoc("");
         }

      } else {
            ol.startPageDoc(projectName);
      }
   }

   if (Doxy_Globals::mainPage != nullptr && ! Doxy_Globals::mainPage->title().isEmpty()) {

      if (Doxy_Globals::mainPage->title().toLower() != "notitle") {
         ol.startHeaderSection();
         ol.startTitleHead(QString());

         ol.generateDoc(Doxy_Globals::mainPage->docFile(), Doxy_Globals::mainPage->getStartBodyLine(), Doxy_Globals::mainPage,
                        QSharedPointer<MemberDef>(), Doxy_Globals::mainPage->title(), true, false, QString(), true, false);

         headerWritten = true;
      }

   } else if (! projectName.isEmpty()) {

      ol.startHeaderSection();
      ol.startTitleHead(QString());
      ol.parseText(projPrefix + theTranslator->trDocumentation());

      headerWritten = true;
   }

   if (headerWritten) {
      ol.endTitleHead(QString(), QString());
      ol.endHeaderSection();
   }

   ol.startContents();
   if (disableIndex && Doxy_Globals::mainPage == nullptr) {
      ol.writeQuickLinks(false, HLI_Main, QString());
   }

   if (Doxy_Globals::mainPage) {
      Doxy_Globals::insideMainPage = true;

      if (Doxy_Globals::mainPage->localToc().isHtmlEnabled() && Doxy_Globals::mainPage->hasSections()) {
         Doxy_Globals::mainPage->writeToc(ol, Doxy_Globals::mainPage->localToc());
      }

      ol.startTextBlock();
      ol.generateDoc(defFileName, defLine, Doxy_Globals::mainPage, QSharedPointer<MemberDef>(),
                     Doxy_Globals::mainPage->documentation(), true, false);

      ol.endTextBlock();
      ol.endPageDoc();

      Doxy_Globals::insideMainPage = false;
   }

   endFile(ol, false, false);
   ol.disable(OutputGenerator::Html);

   // write Docbook, Latex & Rtf index
   ol.enable(OutputGenerator::Docbook);
   ol.enable(OutputGenerator::Latex);
   ol.enable(OutputGenerator::RTF);

   ol.startFile("refman", QString(), QString());
   ol.startIndexSection(isTitlePageStart);

   if (! Config::getString("latex-header").isEmpty()) {
      ol.disable(OutputGenerator::Latex);
   }

   ol.disable(OutputGenerator::Docbook);

   if (projPrefix.isEmpty()) {
      ol.parseText(theTranslator->trReferenceManual());
   } else {
      ol.parseText(projPrefix);
   }

   if (! projectVersion.isEmpty()) {
      ol.startProjectNumber();
      ol.generateDoc(defFileName, defLine, Doxy_Globals::mainPage, QSharedPointer<MemberDef>(),
                     projectVersion, false, false);

      ol.endProjectNumber();
   }

   ol.endIndexSection(isTitlePageStart);
   ol.startIndexSection(isTitlePageAuthor);
   ol.parseText(theTranslator->trGeneratedBy());
   ol.endIndexSection(isTitlePageAuthor);

   ol.enable(OutputGenerator::Docbook);
   ol.enable(OutputGenerator::Latex);

   ol.lastIndexPage();

   if (Doxy_Globals::mainPage != nullptr) {
      ol.startIndexSection(isMainPage);

      if (mainPageHasTitle()) {
         ol.parseText(Doxy_Globals::mainPage->title());
      } else {
         ol.parseText(/*projPrefix+*/theTranslator->trMainPage());
      }
      ol.endIndexSection(isMainPage);
   }

   if (Doxy_Globals::documentedPages > 0) {
      bool firstEntry = (Doxy_Globals::mainPage == nullptr);

      for (auto &pd : Doxy_Globals::pageSDict) {

         if (! pd->getGroupDef() && ! pd->isReference() && (! pd->hasParentPage() || (Doxy_Globals::mainPage == pd->getOuterScope())) ) {
            bool isCitationPage = pd->name() == "citelist";

            if (isCitationPage) {
               // For LaTeX the bibliograph is already written by \bibliography
               ol.pushGeneratorState();
               ol.disable(OutputGenerator::Latex);
            }

            QString title = pd->title();
            if (title.isEmpty()) {
               title = pd->name();
            }

            ol.disable(OutputGenerator::Docbook);

            ol.startIndexSection(isPageDocumentation);
            ol.parseText(title);
            ol.endIndexSection(isPageDocumentation);

            ol.enable(OutputGenerator::Docbook);

            // write TOC title (RTF only)
            ol.pushGeneratorState();
            ol.disableAllBut(OutputGenerator::RTF);

            ol.startIndexSection(isPageDocumentation2);
            ol.parseText(title);
            ol.endIndexSection(isPageDocumentation2);

            ol.popGeneratorState();

            ol.writeAnchor(QString(), pd->getOutputFileBase());

            ol.writePageLink(pd->getOutputFileBase(), firstEntry);
            firstEntry = false;

            if (isCitationPage) {
               ol.popGeneratorState();
            }
         }
      }
   }

   ol.disable(OutputGenerator::Docbook);

   if (! latexHideIndices) {

      if (s_documentedGroups > 0) {
         ol.startIndexSection(isModuleIndex);
         ol.parseText(/*projPrefix+*/ theTranslator->trModuleIndex());
         ol.endIndexSection(isModuleIndex);
      }

      if (showNamespacePage && s_documentedNamespaces > 0) {
         ol.startIndexSection(isNamespaceIndex);
         ol.parseText(/*projPrefix+*/(optimizeFortran ? theTranslator->trModulesIndex() : theTranslator->trNamespaceIndex()));
         ol.endIndexSection(isNamespaceIndex);
      }

      if (s_hierarchyCount > 0) {
         ol.startIndexSection(isClassHierarchyIndex);
         ol.parseText(/*projPrefix+*/
            (optimizeFortran ? theTranslator->trCompoundIndexFortran() : theTranslator->trHierarchicalIndex()));
         ol.endIndexSection(isClassHierarchyIndex);
      }

      if (s_annotatedClassesPrinted > 0) {
         ol.startIndexSection(isCompoundIndex);
         ol.parseText(/*projPrefix+*/
            (optimizeFortran ? theTranslator->trCompoundIndexFortran() : theTranslator->trCompoundIndex()));
         ol.endIndexSection(isCompoundIndex);
      }

      if (showFilePage &&  Doxy_Globals::documentedFiles > 0) {
         // enabled from build options, file command

         ol.startIndexSection(isFileIndex);
         ol.parseText(/*projPrefix+*/theTranslator->trFileIndex());
         ol.endIndexSection(isFileIndex);
      }
   }

   ol.enable(OutputGenerator::Docbook);

   if (s_documentedGroups > 0) {
      ol.startIndexSection(isModuleDocumentation);
      ol.parseText(/*projPrefix+*/theTranslator->trModuleDocumentation());
      ol.endIndexSection(isModuleDocumentation);
   }

   if (s_documentedNamespaces > 0) {
      ol.startIndexSection(isNamespaceDocumentation);
      ol.parseText(/*projPrefix+*/(optimizeFortran ? theTranslator->trModuleDocumentation() : theTranslator->trNamespaceDocumentation()));
      ol.endIndexSection(isNamespaceDocumentation);
   }

   if (s_annotatedClassesPrinted > 0) {
      ol.startIndexSection(isClassDocumentation);
      ol.parseText(/*projPrefix+*/(optimizeFortran ? theTranslator->trTypeDocumentation() : theTranslator->trClassDocumentation()));
      ol.endIndexSection(isClassDocumentation);
   }

   if (Doxy_Globals::documentedFiles > 0) {
      ol.startIndexSection(isFileDocumentation);
      ol.parseText(/*projPrefix+*/theTranslator->trFileDocumentation());
      ol.endIndexSection(isFileDocumentation);
   }

   if (Doxy_Globals::exampleSDict.count() > 0) {
      ol.startIndexSection(isExampleDocumentation);
      ol.parseText(/*projPrefix+*/theTranslator->trExampleDocumentation());
      ol.endIndexSection(isExampleDocumentation);
   }

   ol.endIndexSection(isEndIndex);
   endFile(ol, false, false);

   if (Doxy_Globals::mainPage != nullptr) {
      Doxy_Globals::insideMainPage = true;
      ol.disable(OutputGenerator::Man);

      startFile(ol, Doxy_Globals::mainPage->name(), QString(), Doxy_Globals::mainPage->title());

      ol.startContents();
      ol.startTextBlock();

      ol.generateDoc(defFileName, defLine, Doxy_Globals::mainPage, QSharedPointer<MemberDef>(),
                     Doxy_Globals::mainPage->documentation(), false, false);

      ol.endTextBlock();
      endFile(ol, false, false);

      ol.enable(OutputGenerator::Man);
      Doxy_Globals::insideMainPage = false;
   }

   ol.popGeneratorState();
}

static void writeIndexHierarchyEntries(OutputList &ol, const QList<LayoutNavEntry *> &entries)
{
   for (auto lne : entries) {

      bool addToIndex   = (lne != nullptr && lne->visible());
      bool needsClosing = false;

      LayoutNavEntry::Kind kind = lne->kind();

      if (! s_indexWritten.at(kind)) {

         switch (kind) {
            case LayoutNavEntry::MainPage:
               msg("Generating main page\n");
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
               static bool showNamespaces = Config::getBool("show-namespace-page");

               if (showNamespaces) {
                  if (s_documentedNamespaces > 0 && addToIndex) {
                     Doxy_Globals::indexList.addContentsItem(true, lne->title(), "", lne->baseFile(), "");
                     Doxy_Globals::indexList.incContentsDepth();
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
               static bool showNamespaces = Config::getBool("show-namespace-page");
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
               if (s_annotatedCount > 0 && addToIndex) {
                  Doxy_Globals::indexList.addContentsItem(true, lne->title(), "", lne->baseFile(), "");
                  Doxy_Globals::indexList.incContentsDepth();
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

               if (Config::getBool("have-dot") && Config::getBool("dot-hierarchy")) {
                  msg("Generating graphical class hierarchy\n");
                  writeGraphicalClassHierarchy(ol);
               }
               break;

            case LayoutNavEntry::ClassMembers:
               msg("Generating member index\n");
               writeClassMemberIndex(ol);
               break;

            case LayoutNavEntry::Concepts:
               if (s_conceptCount > 0 && addToIndex) {
                    Doxy_Globals::indexList.addContentsItem(true, lne->title(), "", lne->baseFile(), "");
                    Doxy_Globals::indexList.incContentsDepth();
                    needsClosing = true;
               }
               break;

            case LayoutNavEntry::ConceptList:
               msg("Generating concept index\n");
               writeConceptIndex(ol);
               break;

            case LayoutNavEntry::Files: {
               // enabled from build options, file command
               static const bool showFilePage = Config::getBool("show-file-page");

               if (showFilePage) {
                  if (Doxy_Globals::documentedHtmlFiles > 0 && addToIndex) {
                     Doxy_Globals::indexList.addContentsItem(true, lne->title(), "", lne->baseFile(), "");
                     Doxy_Globals::indexList.incContentsDepth();
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
               // enabled from build options, file command
               static const bool showFilePage = Config::getBool("show-file-page");

               if (showFilePage) {
                  msg("Generating file index\n");
                  writeFileIndex(ol);
               }
            }
            break;

            case LayoutNavEntry::FileGlobals:
               msg("Generating file member index\n");
               writeFileMemberIndex(ol);
               break;

            case LayoutNavEntry::FileSource: {
               // enabled from source code
               static const bool showSource = Config::getBool("source-code");

               if (showSource)  {
                  msg("Generating file source index\n");
                  writeFileSourceIndex(ol);
               }

            }
            break;

            case LayoutNavEntry::Examples:
               msg("Generating example index\n");
               writeExampleIndex(ol);
               break;

            case LayoutNavEntry::User: {
               // prepend a ! or ^ marker to the URL to avoid tampering with it

               QString url = correctURL(lne->url(), "!"); // add ! to relative URL
               bool isRelative = url.at(0) == '!';

               if (! url.isEmpty() && !isRelative) { // absolute URL
                  url.prepend("^"); // prepend ^ to absolute URL
               }

               bool isRef = lne->baseFile().startsWith("@ref") || lne->baseFile().startsWith("\\ref");
               Doxy_Globals::indexList.addContentsItem(true, lne->title(), "", url, "", isRef || isRelative);
            }

            break;

            case LayoutNavEntry::UserGroup:
               if (addToIndex) {
                  QString url = correctURL(lne->url(), "!"); // add ! to relative URL

                  if (! url.isEmpty()) {
                     if (url == "![none]") {
                        Doxy_Globals::indexList.addContentsItem(true, lne->title(), QString(), QString(), QString(), false);

                     } else {
                        bool isRelative = url.at(0) == '!';

                        if (!isRelative) {
                           // absolute URL
                           url.prepend("^"); // prepend ^ to absolute URL
                        }

                        bool isRef = lne->baseFile().startsWith("@ref") || lne->baseFile().startsWith("\\ref");
                        Doxy_Globals::indexList.addContentsItem(true, lne->title(), "", url, "", isRef || isRelative);
                     }

                  } else {
                     Doxy_Globals::indexList.addContentsItem(true, lne->title(), "", lne->baseFile(), "", true);
                  }

                  Doxy_Globals::indexList.incContentsDepth();
                  needsClosing = true;
               }

               writeUserGroupStubPage(ol, lne);
               break;

            case LayoutNavEntry::None:
               break;
         }

         if (kind != LayoutNavEntry::User && kind != LayoutNavEntry::UserGroup) {
            // entry may appear multiple times
            s_indexWritten[kind] = true;
         }
      }

      writeIndexHierarchyEntries(ol, lne->children());

      if (needsClosing) {
         Doxy_Globals::indexList.decContentsDepth();
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

