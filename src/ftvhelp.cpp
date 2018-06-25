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

#include <QHash>
#include <QFile>
#include <QFileInfo>

#include <stdio.h>
#include <stdlib.h>

#include <ftvhelp.h>

#include <config.h>
#include <doxy_globals.h>
#include <docparser.h>
#include <htmlgen.h>
#include <htmldocvisitor.h>
#include <language.h>
#include <layout.h>
#include <message.h>
#include <resourcemgr.h>
#include <util.h>

#define MAX_INDENT 1024

static int folderId = 1;

struct FTVNode {
   FTVNode(bool dir, const QString &r, const QString &f, const QString &a, const QString &n,
                  bool navIndex, QSharedPointer<Definition> df, DirType cat)
      : isLast(true), isDir(dir), ref(r), file(f), anchor(a), name(n), index(0), parent(0),
        addToNavIndex(navIndex), def(df), category(cat)
   { }

   int computeTreeDepth(int level) const;
   int numNodesAtLevel(int level, int maxLevel) const;
   int index;

   bool isLast;
   bool isDir;
   bool addToNavIndex;

   QString ref;
   QString file;
   QString anchor;
   QString name;

   QList<FTVNode *> children;
   FTVNode *parent;

   QSharedPointer<Definition> def;

   DirType category;
};

int FTVNode::computeTreeDepth(int level) const
{
   int maxDepth = level;

   for (auto n : children) {
      if (n->children.count() > 0) {
         int d = n->computeTreeDepth(level + 1);

         if (d > maxDepth) {
            maxDepth = d;
         }
      }
   }
   return maxDepth;
}

int FTVNode::numNodesAtLevel(int level, int maxLevel) const
{
   int num = 0;

   if (level < maxLevel) {
      // this node
      num++;

      for (auto n : children) {
         num += n->numNodesAtLevel(level + 1, maxLevel);
      }
   }
   return num;
}

/*! Constructs an ftv help object.
 *  The object has to be initialized before it can be used
 */
FTVHelp::FTVHelp(bool data)
{
   /* initial depth */
   m_indentNodes = new QList<FTVNode *>[MAX_INDENT];

   m_indent = 0;
   m_topLevelIndex = data;
}

FTVHelp::~FTVHelp()
{
   for (int k = 0; k < MAX_INDENT; ++k)  {
      for (auto item : m_indentNodes[k]) {
         delete item;
      }
   }

   delete[] m_indentNodes;
}

/*! This will create a folder tree view table of contents file (tree.js).
 *  \sa finalize()
 */
void FTVHelp::initialize()
{
}

/*! Finalizes the FTV help. This will finish and close the contents file (index.js).
 *  \sa initialize()
 */
void FTVHelp::finalize()
{
   generateTreeViewImages();
   generateTreeViewScripts();
}

/*! Increase the level of the contents hierarchy.
 *  This will start a new sublist in contents file.
 *  \sa decContentsDepth()
 */
void FTVHelp::incContentsDepth()
{
   QList<FTVNode *> &nl = m_indentNodes[m_indent];

   m_indent++;
   assert(m_indent < MAX_INDENT);
}

/*! Decrease the level of the contents hierarchy. This will end the current sublist.
 *  \sa incContentsDepth()
 */
void FTVHelp::decContentsDepth()
{
   assert(m_indent > 0);

   if (m_indent > 0) {
      m_indent--;

      QList<FTVNode *> &nl = m_indentNodes[m_indent];

      if (nl.isEmpty()) {
         // subsection with no parent, resolve by not doing the else

      } else {
         FTVNode *parent = nl.last();

         if (parent) {
            QList<FTVNode *> &kids = m_indentNodes[m_indent + 1];

            while (! kids.isEmpty()) {
               parent->children.append(kids.takeAt(0));
            }
         }
      }
   }
}

/*! Add a list item to the contents file.
 *  \param isDir          true if the item is a directory, false if it is text
 *  \param name           name of the item
 *  \param ref            URL of to the item
 *  \param file           file containing the definition of the item
 *  \param anchor         anchor within the file
 *  \param addToNavIndex  add this entry to the quick navigation index
 *  \param def            definition corresponding to this entry
 */
void FTVHelp::addContentsItem(bool isDir, const QString &name, const QString &ref, const QString &file, const QString &anchor,
                              bool addToNavIndex, QSharedPointer<Definition> def, DirType category)
{
   QList<FTVNode *> *nl = &m_indentNodes[m_indent];

   FTVNode *newNode = new FTVNode(isDir, ref, file, anchor, name, addToNavIndex, def, category);

   if (! nl->isEmpty()) {
      nl->last()->isLast = false;
   }

   nl->append(newNode);
   newNode->index = nl->count() - 1;

   if (m_indent > 0) {
      QList<FTVNode *> &pnl = m_indentNodes[m_indent - 1];

      if (def) {
         // page, class, method, etc
         newNode->parent = pnl.last();

      } else {
         // most likely a section, subsection

         if (pnl.isEmpty()) {
            // must test for this condition in decContentsDepth()
            fprintf(stderr, "Error: Page (%s) contains a subsection (%s) with no parent section\n", csPrintable(file), csPrintable(name) );

         } else {
            newNode->parent = pnl.last();
         }
      }
   }

}

static QString node2URL(FTVNode *n, bool overruleFile = false, bool srcLink = false)
{
   QString url = n->file;

   if (! url.isEmpty() && url.at(0) == '!') {
      // relative URL, remove leading !
      url = url.mid(1);

   } else if ( !url.isEmpty() && url.at(0) == '^') {
      // absolute URL, skip, keep ^ in the output

   } else {
      // local file (with optional anchor)

      if (overruleFile && n->def && n->def->definitionType() == Definition::TypeFile) {
         QSharedPointer<FileDef> fd = n->def.dynamicCast<FileDef>();

         if (srcLink) {
            url = fd->getSourceFileBase();
         } else {
            url = fd->getOutputFileBase();
         }
      }

      url += Doxy_Globals::htmlFileExtension;
      if ( !n->anchor.isEmpty()) {
         url += "#" + n->anchor;
      }
   }
   return url;
}

QString FTVHelp::generateIndentLabel(FTVNode *n, int level)
{
   QString result;

   if (n->parent) {
      result = generateIndentLabel(n->parent, level + 1);
   }

   result += QString::number(n->index) + "_";

   return result;
}

void FTVHelp::generateIndent(QTextStream &t, FTVNode *n, bool opened)
{
   int indent = 0;
   FTVNode *p = n->parent;

   while (p) {
      indent++;
      p = p->parent;
   }

   if (n->isDir) {
      QString dir = opened ? "&#9660;" : "&#9658;";
      t << "<span style=\"width:" << (indent * 16) << "px;display:inline-block;\">&#160;</span>"
        << "<span id=\"arr_" << generateIndentLabel(n, 0) << "\" class=\"arrow\" ";
      t << "onclick=\"toggleFolder('" << generateIndentLabel(n, 0) << "')\"";
      t << ">" << dir
        << "</span>";

   } else {
      t << "<span style=\"width:" << ((indent + 1) * 16) << "px;display:inline-block;\">&#160;</span>";

   }
}

void FTVHelp::generateLink(QTextStream &t, FTVNode *n)
{
   if (n->file.isEmpty()) {
      // no link
      t << "<b>" << convertToHtml(n->name) << "</b>";

   } else {
      // link into other frame

      if (!n->ref.isEmpty()) {
         // link to entity imported via tag file
         t << "<a class=\"elRef\" ";
         t << externalLinkTarget() << externalRef("", n->ref, false);

      } else {
         // local link
         t << "<a class=\"el\" ";
      }

      t << "href=\"";
      t << externalRef("", n->ref, true);
      t << node2URL(n);

      if (m_topLevelIndex) {
         t << "\" target=\"basefrm\">";
      } else {
         t << "\" target=\"_self\">";
      }

      t << convertToHtml(n->name);
      t << "</a>";

      if (! n->ref.isEmpty()) {
         t << "&#160;[external]";
      }
   }
}

static void generateBriefDoc(QTextStream &t, QSharedPointer<Definition> def)
{
   QString brief = def->briefDescription(true);

   if (! brief.isEmpty()) {
      DocNode *root = validatingParseDoc(def->briefFile(), def->briefLine(),
                  def, QSharedPointer<MemberDef>(), brief, false, false, "", true, true);

      QString relPath = relativePathToRoot(def->getOutputFileBase());

      HtmlCodeGenerator htmlGen(t, relPath);
      HtmlDocVisitor *visitor = new HtmlDocVisitor(t, htmlGen, def);
      root->accept(visitor);

      delete visitor;
      delete root;
   }
}

void FTVHelp::generateTree(QTextStream &t, const QList<FTVNode *> &nl, int level, int maxLevel, int &index, enum PageType outputType)
{
   static bool isStyleBB = Config::getBool("bb-style");
   static bool javaOpt   = Config::getBool("optimize-java");
   static bool pythonOpt = Config::getBool("optimize-python");

   for (auto n : nl) {
      t << "<tr id=\"row_" << generateIndentLabel(n, 0) << "\"";

      if ((index & 1) == 0) {
         // even row
         t << " class=\"even\"";
      }

      if (level >= maxLevel) {
         // item invisible by default
         t << " style=\"display:none;\"";

      } else {
         // item visible by default
         index++;

      }

      t << "><td class=\"entry\">";

      bool nodeOpened = level + 1 < maxLevel;
      generateIndent(t, n, nodeOpened);

      if (n->isDir) {
         if (n->def && n->def->definitionType() == Definition::TypeGroup) {
            // no icon

         } else if (n->def && n->def->definitionType() == Definition::TypePage) {
            // no icon

         } else if (n->def && n->def->definitionType() == Definition::TypeNamespace) {

            if (javaOpt || pythonOpt) {
               t << "<span class=\"icona\"><span class=\"icon\">P</span></span>";
            } else {
               t << "<span class=\"icona\"><span class=\"icon\">N</span></span>";
            }

         } else if (n->def && n->def->definitionType() == Definition::TypeClass) {
            t << "<span class=\"icona\"><span class=\"icon\">C</span></span>";

         } else {
            t << "<span id=\"img_" << generateIndentLabel(n, 0)
              << "\" class=\"iconf"
              << (nodeOpened ? "open" : "closed")
              << "\" onclick=\"toggleFolder('" << generateIndentLabel(n, 0)
              << "')\">&#160;</span>";
         }

         generateLink(t, n);
         t << "</td>";

         if (isStyleBB && outputType == FTVHelp::Modules) {
            // modules.html only

            QString moduleDesc;
            auto iter = Doxy_Globals::g_moduleHint.find(n->def->docFile());

            if (iter != Doxy_Globals::g_moduleHint.end()) {
               moduleDesc = iter.value();
            }

            t << "<td class=\"hint\">";
            t << moduleDesc;
            t << "</td>";
         }

         // brief description
         t << "<td class=\"desc\">";

         if (n->def) {
            generateBriefDoc(t, n->def);
         }

         t << "</td></tr>" << endl;

         folderId++;
         generateTree(t, n->children, level + 1, maxLevel, index, outputType);

      } else {
         // leaf node
         QSharedPointer<FileDef> srcRef;

         if (n->def && n->def->definitionType() == Definition::TypeFile && (n->def.dynamicCast<FileDef>())->generateSourceFile()) {
            srcRef = n->def.dynamicCast<FileDef>();
         }

         if (srcRef) {
            t << "<a href=\"" << srcRef->getSourceFileBase()
              << Doxy_Globals::htmlFileExtension
              << "\">";
         }

         if (n->def && n->def->definitionType() == Definition::TypeGroup) {
            // no icon

         } else if (n->def && n->def->definitionType() == Definition::TypePage) {
            // no icon

         } else if (n->def && n->def->definitionType() == Definition::TypeNamespace) {

            if (javaOpt || pythonOpt) {
               t << "<span class=\"icona\"><span class=\"icon\">P</span></span>";
            } else {
               t << "<span class=\"icona\"><span class=\"icon\">N</span></span>";
            }

         } else if (n->def && n->def->definitionType() == Definition::TypeClass) {
            t << "<span class=\"icona\"><span class=\"icon\">C</span></span>";

         } else {
            t << "<span class=\"icondoc\"></span>";
         }

         if (srcRef) {
            t << "</a>";
         }

         generateLink(t, n);
         t << "</td>";

         if (isStyleBB && outputType == FTVHelp::Modules) {
            // modules.html only

            QString moduleDesc;
            auto iter = Doxy_Globals::g_moduleHint.find(n->def->docFile());

            if (iter != Doxy_Globals::g_moduleHint.end()) {
               moduleDesc = iter.value();
            }

            t << "<td class=\"hint\">";
            t << moduleDesc;
            t << "</td>";
         }

         // brief description
         t << "<td class=\"desc\">";

         if (n->def) {
            generateBriefDoc(t, n->def);
         }

         t << "</td></tr>" << endl;
      }
   }
}

static QString pathToNode(FTVNode *node)
{
   QString result;

   bool isFirst = true;

   while (node != nullptr) {

      if (isFirst) {
         result = QString::number(node->index);
         isFirst = false;

      } else {
         result = QString::number(node->index) + "," + result;

      }

      node = node->parent;
   }

   return result;
}

static bool dupOfParent(const FTVNode *n)
{
   if (n->parent == 0) {
      return false;
   }

   if (n->file == n->parent->file) {
      return true;
   }

   return false;
}

static void generateJSLink(QTextStream &t, FTVNode *n)
{
   if (n->file.isEmpty()) {
      // no link

      t << "\"" << convertToJSString(n->name) << "\", null, ";

   } else {
      // link into other page

      t << "\"" << convertToJSString(n->name) << "\", \"";
      t << externalRef("", n->ref, true);
      t << node2URL(n);
      t << "\", ";
   }
}

static QString convertFileId2Var(const QString  &fileId)
{
   QString  varId = fileId;
   int i = varId.lastIndexOf('/');

   if (i >= 0) {
      varId = varId.mid(i + 1);
   }

   return substitute(varId, "-", "_");
}

static bool generateJSTree(SortedList<NavIndexEntry *> &navIndex, QTextStream &t, const QList<FTVNode *> &nl,
                  int level, bool &omitComma)
{
   static QString htmlOutput   = Config::getString("html-output");
   static QString mainPageName = Config::getFullName(Config::getString("main-page-name"));
   static bool mainPageOmit    = Config::getBool("main-page-omit");

   QString indentStr;
   indentStr.fill(' ', level * 2);

   bool found = false;

   for (auto node : nl)  {
      // terminate previous entry
      if (! omitComma) {
         t << "," << endl;
      }
      omitComma = false;

      // start entry
      if (! found) {
         t << "[" << endl;
      }
      found = true;

      if (node->addToNavIndex) {
         // add entry to the navigation index

         if (node->def && node->def->definitionType() == Definition::TypeFile) {
            QSharedPointer<FileDef> fd = node->def.dynamicCast<FileDef>();

            if (! mainPageName.isEmpty() && fd->getFilePath() == mainPageName) {
               // do not add this file to the navIndex, for \files

            } else {
               bool doc = docFileVisibleInIndex(fd);
               bool src = srcFileVisibleInIndex(fd);

               if (node->category == DirType::File && doc) {
                  navIndex.inSort(new NavIndexEntry(node2URL(node, true, false), pathToNode(node)));
               }

               if (node->category == DirType::FileSource && src) {
                  navIndex.inSort(new NavIndexEntry(node2URL(node, true, true), pathToNode(node)));
               }
            }

         } else {

            if (mainPageOmit && node->def && node->def == Doxy_Globals::mainPage) {
               // do not add this file to the navIndex

            } else {
               navIndex.inSort(new NavIndexEntry(node2URL(node), pathToNode(node)));

            }
         }
      }

      // store items for pages (annotated, modules, namespaces, files)
      bool firstChild   = true;
      bool showMainPage = true;

      if (node->def && node->def->definitionType() == Definition::TypeFile) {
         QSharedPointer<FileDef> fd = node->def.dynamicCast<FileDef>();

         if (! mainPageName.isEmpty() && fd->getFilePath() == mainPageName) {
            // do not add this file to the navIndex, for \files

            showMainPage = false;
            omitComma    = true;
         }

      } else {

         if (mainPageOmit && node->def == Doxy_Globals::mainPage) {
            // omit index page from being displayed in the treeview entry

            showMainPage = false;
            omitComma    = true;
         }
      }

      if (showMainPage) {

         t << indentStr << "  [ ";
         generateJSLink(t, node);

         bool emptySection = ! generateJSTree(navIndex, t, node->children, level + 1, firstChild);

         if (emptySection) {
            t << "null ]";

         } else {
            t << endl << indentStr << "  ] ]";

         }
      }

   }

   return found;
}

// adjust for display
static void reSortNodes(QList<FTVNode *> &nodeList)
{
   std::stable_sort(nodeList.begin(), nodeList.end(), [](FTVNode *a, FTVNode *b) {

      if (a->def == nullptr || b->def == nullptr) {
         // not a pageDef
         return a->index < b->index;
      }

      int sortId_A = a->def->getSortId();
      int sortId_B = b->def->getSortId();

      if (sortId_A != -1 && sortId_B != -1 && sortId_A != sortId_B)  {
         return sortId_A < sortId_B;

      } else if (sortId_A == -1)  {
         return false;

      } else if (sortId_B == -1)  {
         return true;

      } else  {
         return a->def->getInputOrderId() < b->def->getInputOrderId();
      }

   } );

   int counter = 0;

   for (auto node : nodeList) {


/* broom (on hold, sort test)
if  (node->file.contains("getting-started")) {

//    (node->file.contains("build-from-source") || node->file.contains("build-options") || node->file.contains("requirements-") ||
//       node->file.contains("implicit") || node->file.contains("unicode") ||
//       node->file.contains("main-dev") || node->file.contains("sample-project") || item->file.contains("faq") ) {

      printf("\n  file: %-20s   Name: %-20s   Alpha: %-3d", csPrintable(item->file), csPrintable(node->name), node->index );

      if (node->def) {
         printf("  Our OrderId: %-3d",  node->def->getInputOrderId() );
         printf("  New sortId: %-3d",   node->def->getSortId() );
      }
}
*/

      node->index = counter;

      if (node->file == "index") {
         // do not count this page
      } else {
         counter++;
      }

      if (node->children.count() != 0 ) {
         reSortNodes(node->children);
      }
   }
}

static void generateJSNavTree(QList<FTVNode *> &nodeList)
{
   QString htmlOutput = Config::getString("html-output");

   // new js
   QFile f(htmlOutput + "/navtreedata.js");
   SortedList<NavIndexEntry *> navIndex;

   if (f.open(QIODevice::WriteOnly)) {

      QTextStream t(&f);
      t << "var NAVTREE =" << endl;
      t << "[" << endl;
      t << "  [ ";

      QString projName = Config::getString("project-name");

      if (projName.isEmpty()) {
         if (Doxy_Globals::mainPage && ! Doxy_Globals::mainPage->title().isEmpty()) {
            // use title of main page as root
            t << "\"" << convertToJSString(Doxy_Globals::mainPage->title()) << "\", ";

         } else {
            // use default section title as root
            LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::MainPage);
            t << "\"" << convertToJSString(lne->title()) << "\", ";
         }

      } else {
         // use PROJECT_NAME as root tree element
         t << "\"" << convertToJSString(projName) << "\", ";
      }

      t << "\"index" << Doxy_Globals::htmlFileExtension << "\", ";

      // add one special entry for index page
      navIndex.inSort(new NavIndexEntry("index" + Doxy_Globals::htmlFileExtension, ""));

      // add one special entry for related pages, written as a child of index.html
      navIndex.inSort(new NavIndexEntry("pages" + Doxy_Globals::htmlFileExtension, ""));

      // adjust for display output
      reSortNodes(nodeList);

      bool omitComma = true;
      generateJSTree(navIndex, t, nodeList, 1, omitComma);

      if (omitComma) {
         t << "]" << endl;
      } else {
         t << endl << "  ] ]" << endl;
      }

      t << "];" << endl << endl;

      int subIndex  = 0;
      int elemCount = 0;
      const int maxElemCount = 250;

      // new js
      QFile fsidx(htmlOutput + "/navtreeindex0.js");

      if (fsidx.open(QIODevice::WriteOnly)) {

         QTextStream tsidx(&fsidx);

         t << "var NAVTREEINDEX =" << endl;
         t << "[" << endl;

         tsidx << "var NAVTREEINDEX" << subIndex << " =" << endl;
         tsidx << "{" << endl;

         omitComma      = true;
         auto nextItem  = navIndex.begin();

         for (auto e : navIndex)  {
            // for each entry
            ++nextItem;

            if (elemCount == 0) {
               if (! omitComma) {
                  t << "," << endl;

               } else {
                  omitComma = false;
               }

               t << "\"" << e->m_url << "\"";
            }

            tsidx << "\"" << e->m_url << "\":[" << e->m_indexId << "]";

            if (nextItem != navIndex.end() && elemCount < maxElemCount - 1) {
               // not the last entry
               tsidx << ",";
            }

            tsidx << endl;

            elemCount++;

            if (nextItem != navIndex.end() && elemCount >= maxElemCount) {
               // switch to new sub-index
               tsidx << "};" << endl;

               elemCount = 0;
               fsidx.close();

               subIndex++;

               fsidx.setFileName(htmlOutput + "/navtreeindex" + QString::number(subIndex) + ".js");

               if (! fsidx.open(QIODevice::WriteOnly)) {
                  break;
               }

               tsidx.setDevice(&fsidx);
               tsidx << "var NAVTREEINDEX" << subIndex << " =" << endl;
               tsidx << "{" << endl;
            }
         }

         tsidx << "};" << endl;
         t << endl << "];" << endl;
      }

      t << endl << "var SYNCONMSG = '"  << theTranslator->trPanelSyncTooltip(false) << "';";
      t << endl << "var SYNCOFFMSG = '" << theTranslator->trPanelSyncTooltip(true)  << "';";
   }

   ResourceMgr::instance().copyResourceAs("html/navtree.js", htmlOutput, "navtree.js");
}

// style images
void FTVHelp::generateTreeViewImages()
{
   QString htmlOutput = Config::getString("html-output");

   const ResourceMgr &rm = ResourceMgr::instance();

   rm.copyResourceAs("html/doc.luma",          htmlOutput, "doc.png");
   rm.copyResourceAs("html/folderopen.luma",   htmlOutput, "folderopen.png");
   rm.copyResourceAs("html/folderclosed.luma", htmlOutput, "folderclosed.png");
   rm.copyResourceAs("html/splitbar.lum",      htmlOutput, "splitbar.png");
}

// style scripts
void FTVHelp::generateTreeViewScripts()
{
   QString htmlOutput = Config::getString("html-output");

   // generate navtree.js and navtreeindex.js
   generateJSNavTree(m_indentNodes[0]);

   ResourceMgr::instance().copyResourceAs("html/resize.js",   htmlOutput, "resize.js");
   ResourceMgr::instance().copyResourceAs("html/navtree.css", htmlOutput, "navtree.css");
}

// write tree inside page
void FTVHelp::generateTreeViewInline(QTextStream &t, enum PageType outputType)
{
   int preferredNumEntries = Config::getInt("html-index-num-entries");

   t << "<div class=\"directory\">\n";

   int d = 1;
   int depth = 1;

   // adjust for display output
   reSortNodes(m_indentNodes[0]);

   for (auto n : m_indentNodes[0] ) {

      if (n->children.count() > 0) {
         d = n->computeTreeDepth(2);

         if (d > depth) {
            depth = d;
         }
      }
   }

   int preferredDepth = depth;

   // write level selector
   if (depth > 1) {
      t << "<div class=\"levels\">[";
      t << theTranslator->trDetailLevel();
      t << " ";

      int i;
      for (i = 1; i <= depth; i++) {
         t << "<span onclick=\"javascript:toggleLevel(" << i << ");\">" << i << "</span>";
      }

      t << "]</div>";

      if (preferredNumEntries > 0) {
         preferredDepth = 1;

         for (int i = 1; i <= depth; i++) {
            int num = 0;

            for (auto n : m_indentNodes[0] ) {
               num += n->numNodesAtLevel(0, i);
            }

            if (num <= preferredNumEntries) {
               preferredDepth = i;
            } else {
               break;
            }
         }
      }
   }

   t << "<table class=\"directory\">\n";

   int index = 0;
   generateTree(t, m_indentNodes[0], 0, preferredDepth, index, outputType);

   t << "</table>\n";
   t << "</div><!-- directory -->\n" << endl;
}
