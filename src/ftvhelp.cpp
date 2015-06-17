/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
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

#include <config.h>
#include <doxy_globals.h>
#include <ftvhelp.h>
#include <message.h>
#include <language.h>
#include <htmlgen.h>
#include <layout.h>
#include <pagedef.h>
#include <docparser.h>
#include <htmldocvisitor.h>
#include <filedef.h>
#include <resourcemgr.h>
#include <util.h>

#define MAX_INDENT 1024

static int folderId = 1;

struct FTVNode {
   FTVNode(bool dir, const char *r, const char *f, const char *a,
           const char *n, bool sepIndex, bool navIndex, QSharedPointer<Definition> df)
      : isLast(true), isDir(dir), ref(r), file(f), anchor(a), name(n), index(0),
        parent(0), separateIndex(sepIndex), addToNavIndex(navIndex), def(df) 
   {       
   }

   int computeTreeDepth(int level) const;
   int numNodesAtLevel(int level, int maxLevel) const;

   bool isLast;
   bool isDir;

   QByteArray ref;
   QByteArray file;
   QByteArray anchor;
   QByteArray name;

   int index;
   QList<FTVNode *> children;

   FTVNode *parent;

   bool separateIndex;
   bool addToNavIndex;

   QSharedPointer<Definition> def;
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

/*! Constructs a ftv help object.
 *  The object has to be \link initialize() initialized\endlink before it can
 *  be used.
 */
FTVHelp::FTVHelp(bool data)
{
   /* initial depth */
   m_indentNodes = new QList<FTVNode *>[MAX_INDENT];
 
   m_indent = 0;
   m_topLevelIndex = data;
}

/*! Destroys the ftv help object. */
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

/*! Finalizes the FTV help. This will finish and close the
 *  contents file (index.js).
 *  \sa initialize()
 */
void FTVHelp::finalize()
{
   generateTreeView();
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

/*! Decrease the level of the contents hierarchy.
 *  This will end the current sublist.
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
            QList<FTVNode *> &children = m_indentNodes[m_indent + 1];
   
            while (! children.isEmpty()) {
               parent->children.append(children.takeAt(0));
            }
         }
      }
   }
}

/*! Add a list item to the contents file.
 *  \param isDir true if the item is a directory, false if it is a text
 *  \param ref  the URL of to the item.
 *  \param file the file containing the definition of the item
 *  \param anchor the anchor within the file
 *  \param name the name of the item
 *  \param separateIndex put the entries in a separate index file
 *  \param addToNavIndex add this entry to the quick navigation index
 *  \param def Definition corresponding to this entry
 */
void FTVHelp::addContentsItem(bool isDir, const QString &name, const char *ref, const char *file, const char *anchor,
                              bool separateIndex, bool addToNavIndex, QSharedPointer<Definition> def)
{
   QList<FTVNode *> *nl = &m_indentNodes[m_indent];

   FTVNode *newNode = new FTVNode(isDir, ref, file, anchor, qPrintable(name), separateIndex, addToNavIndex, def);
   
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
            fprintf(stderr, "Error: Page (%s) contains a subsection (%s) with no parent section\n", file, qPrintable(name) );           
                     
         } else {    
            newNode->parent = pnl.last();      
         }
      }
   }
}

static QByteArray node2URL(FTVNode *n, bool overruleFile = false, bool srcLink = false)
{
   QByteArray url = n->file;

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
      if (!n->anchor.isEmpty()) {
         url += "#" + n->anchor;
      }
   }
   return url;
}

QByteArray FTVHelp::generateIndentLabel(FTVNode *n, int level)
{
   QByteArray result;

   if (n->parent) {
      result = generateIndentLabel(n->parent, level + 1);
   }

   result += QByteArray().setNum(n->index) + "_";
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
      QByteArray dir = opened ? "&#9660;" : "&#9658;";
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

   if (n->file.isEmpty()) { // no link
      t << "<b>" << convertToHtml(n->name) << "</b>";

   } else { 
      // link into other frame
      if (!n->ref.isEmpty()) { // link to entity imported via tag file
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

      if (!n->ref.isEmpty()) {
         t << "&#160;[external]";
      }
   }
}

static void generateBriefDoc(QTextStream &t, QSharedPointer<Definition> def)
{
   QByteArray brief = def->briefDescription(true);
  
   if (! brief.isEmpty()) {
      DocNode *root = validatingParseDoc(def->briefFile(), def->briefLine(),
                                         def, QSharedPointer<MemberDef>(), brief, false, false, 0, true, true);

      QByteArray relPath = relativePathToRoot(def->getOutputFileBase());

      HtmlCodeGenerator htmlGen(t, relPath);
      HtmlDocVisitor *visitor = new HtmlDocVisitor(t, htmlGen, def);
      root->accept(visitor);

      delete visitor;
      delete root;
   }
}

void FTVHelp::generateTree(QTextStream &t, const QList<FTVNode *> &nl, int level, int maxLevel, int &index)
{
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
            t << "<span class=\"icona\"><span class=\"icon\">N</span></span>";

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
         t << "</td><td class=\"desc\">";

         if (n->def) {
            generateBriefDoc(t, n->def);
         }

         t << "</td></tr>" << endl;
         folderId++;

         generateTree(t, n->children, level + 1, maxLevel, index);

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
            t << "<span class=\"icona\"><span class=\"icon\">N</span></span>";

         } else if (n->def && n->def->definitionType() == Definition::TypeClass) {
            t << "<span class=\"icona\"><span class=\"icon\">C</span></span>";

         } else {
            t << "<span class=\"icondoc\"></span>";
         }

         if (srcRef) {
            t << "</a>";
         }

         generateLink(t, n);
         t << "</td><td class=\"desc\">";

         if (n->def) {
            generateBriefDoc(t, n->def);
         }

         t << "</td></tr>" << endl;
      }
   }
}

static QByteArray pathToNode(FTVNode *leaf, FTVNode *n)
{
   QByteArray result;

   if (n->parent) {
      result += pathToNode(leaf, n->parent);
   }

   result += QByteArray().setNum(n->index);

   if (leaf != n) {
      result += ",";
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

static QByteArray convertFileId2Var(const QByteArray &fileId)
{
   QByteArray varId = fileId;
   int i = varId.lastIndexOf('/');

   if (i >= 0) {
      varId = varId.mid(i + 1);
   }

   return substitute(varId, "-", "_");
}

static bool generateJSTree(SortedList<NavIndexEntry *> &navIndex, QTextStream &t, const QList<FTVNode *> &nl, int level, bool &first)
{
   static QString htmlOutput = Config::getString("html-output");

   QByteArray indentStr;
   indentStr.fill(' ', level * 2);

   bool found = false;

   for (auto n : nl)  {
      // terminate previous entry

      if (! first) {
         t << "," << endl;
      }
      first = false;

      // start entry
      if (! found) {
         t << "[" << endl;
      }
      found = true;

      if (n->addToNavIndex) { 
         // add entry to the navigation index

         if (n->def && n->def->definitionType() == Definition::TypeFile) {

            QSharedPointer<FileDef> fd = n->def.dynamicCast<FileDef>();

            bool doc;
            bool src;

            doc = fileVisibleInIndex(fd, src);

            if (doc) {
               navIndex.inSort(new NavIndexEntry(node2URL(n, true, false), pathToNode(n, n)));
            }

            if (src) {
               navIndex.inSort(new NavIndexEntry(node2URL(n, true, true), pathToNode(n, n)));
            }

         } else {
            navIndex.inSort(new NavIndexEntry(node2URL(n), pathToNode(n, n)));
         }
      }

      if (n->separateIndex) { 
         // store items in a separate file for dynamic loading
         bool firstChild = true;

         t << indentStr << "  [ ";
         generateJSLink(t, n);

         if (n->children.count() > 0) { 
            // write children to separate file for dynamic loading
            QByteArray fileId = n->file;

            if (! n->anchor.isEmpty()) {
               fileId += "_" + n->anchor;
            }

            if (dupOfParent(n)) {
               fileId += "_dup";
            }

            QFile f(htmlOutput + "/" + fileId + ".js");

            if (f.open(QIODevice::WriteOnly)) {
               QTextStream tt(&f);

               tt << "var " << convertFileId2Var(fileId) << " =" << endl;
               generateJSTree(navIndex, tt, n->children, 1, firstChild);
               tt << endl << "];";
            }

            t << "\"" << fileId << "\" ]";

         } else { // no children
            t << "null ]";
         }

      } else { 
         // show items in this file
         bool firstChild = true;

         t << indentStr << "  [ ";
         generateJSLink(t, n);

         bool emptySection = ! generateJSTree(navIndex, t, n->children, level + 1, firstChild);

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

   for (auto item : nodeList) {     


/*  broom -- on hold, sort test
if  (item->file.contains("getting-started")) {

//    (item->file.contains("build-from-source") || item->file.contains("build-options") || item->file.contains("requirements-") ||
//    item->file.contains("implicit") || item->file.contains("unicode") ||
//    item->file.contains("main-dev") || item->file.contains("sample-project") || item->file.contains("faq") ) {
     
      printf("\n  file: %-20s   Name: %-20s   Alpha: %-3d", item->file.constData(), item->name.constData(), item->index );

      if (item->def) {         
         printf("  Our OrderId: %-3d",  item->def->getInputOrderId() );  
         printf("  New sortId: %-3d",   item->def->getSortId() );  
      }   
}
*/

      item->index = counter;
      counter++;

      if (item->children.count() != 0 ) {
         QList<FTVNode *> &children = item->children;
         reSortNodes(children);
      }
   }
}

static void generateJSNavTree(QList<FTVNode *> &nodeList)
{
   QString htmlOutput = Config::getString("html-output");

   QFile f(htmlOutput + "/navtreedata.js");
   SortedList<NavIndexEntry *> navIndex;

   if (f.open(QIODevice::WriteOnly)) {
   
      QTextStream t(&f);
      t << "var NAVTREE =" << endl;
      t << "[" << endl;
      t << "  [ ";

      QString projName = Config::getString("project-name");

      if (projName.isEmpty()) {
         if (Doxy_Globals::mainPage && !Doxy_Globals::mainPage->title().isEmpty()) { 
            // Use title of main page as root
            t << "\"" << convertToJSString(Doxy_Globals::mainPage->title()) << "\", ";

         } else { 
            // Use default section title as root
            LayoutNavEntry *lne = LayoutDocManager::instance().rootNavEntry()->find(LayoutNavEntry::MainPage);
            t << "\"" << convertToJSString(lne->title()) << "\", ";
         }

      } else { 
         // use PROJECT_NAME as root tree element
         t << "\"" << convertToJSString(projName) << "\", ";
      }

      t << "\"index" << Doxy_Globals::htmlFileExtension << "\", ";

      // add special entry for index page
      navIndex.inSort(new NavIndexEntry("index" + Doxy_Globals::htmlFileExtension.toUtf8(), ""));

      // related page index, written as a child of index.html
      navIndex.inSort(new NavIndexEntry("pages" + Doxy_Globals::htmlFileExtension.toUtf8(), ""));

      // adjust for display output     
      reSortNodes(nodeList);

      bool first = true;
      generateJSTree(navIndex, t, nodeList, 1, first);

      if (first) {
         t << "]" << endl;
      } else {
         t << endl << "  ] ]" << endl;
      }

      t << "];" << endl << endl;
 
      int subIndex  = 0;
      int elemCount = 0;

      const int maxElemCount = 250;

      //
      QFile fsidx(htmlOutput + "/navtreeindex0.js");

      if (fsidx.open(QIODevice::WriteOnly)) {
         
         QTextStream tsidx(&fsidx);

         t << "var NAVTREEINDEX =" << endl;
         t << "[" << endl;

         tsidx << "var NAVTREEINDEX" << subIndex << " =" << endl;
         tsidx << "{" << endl;

         bool first = true;     

         auto nextItem = navIndex.begin();

         for (auto e : navIndex)  {
            // for each entry
            ++nextItem;

            if (elemCount == 0) {
               if (! first) {
                  t << "," << endl;

               } else {
                  first = false;
               }

               t << "\"" << e->url << "\"";
            }

            tsidx << "\"" << e->url << "\":[" << e->path << "]";
            
            if (nextItem != navIndex.end() && elemCount < maxElemCount - 1) {
               tsidx << ",";   // not last entry
            }

            tsidx << endl;

            elemCount++;

            if (nextItem != navIndex.end() && elemCount >= maxElemCount) {           
               // switch to new sub-index
               tsidx << "};" << endl;

               elemCount = 0;
               fsidx.close();

               subIndex++;

               fsidx.setFileName(htmlOutput + "/navtreeindex" + QByteArray().setNum(subIndex) + ".js");

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

// new style images
void FTVHelp::generateTreeViewImages()
{
   QString htmlOutput = Config::getString("html-output");

   const ResourceMgr &rm = ResourceMgr::instance();

   rm.copyResourceAs("html/doc.luma",          htmlOutput, "doc.png");
   rm.copyResourceAs("html/folderopen.luma",   htmlOutput, "folderopen.png");
   rm.copyResourceAs("html/folderclosed.luma", htmlOutput, "folderclosed.png");
   rm.copyResourceAs("html/arrowdown.luma",    htmlOutput, "arrowdown.png");
   rm.copyResourceAs("html/arrowright.luma",   htmlOutput, "arrowright.png");
   rm.copyResourceAs("html/splitbar.lum",      htmlOutput, "splitbar.png");
}

// new style scripts
void FTVHelp::generateTreeViewScripts()
{
   QString htmlOutput = Config::getString("html-output");

   // generate navtree.js & navtreeindex.js
   generateJSNavTree(m_indentNodes[0]);
   
   ResourceMgr::instance().copyResourceAs("html/resize.js",   htmlOutput, "resize.js");
   ResourceMgr::instance().copyResourceAs("html/navtree.css", htmlOutput, "navtree.css");
}

// write tree inside page
void FTVHelp::generateTreeViewInline(QTextStream &t)
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
   generateTree(t, m_indentNodes[0], 0, preferredDepth, index);

   t << "</table>\n";
   t << "</div><!-- directory -->\n" << endl;
}

// write old style index.html and tree.html
void FTVHelp::generateTreeView()
{
   generateTreeViewImages();
   generateTreeViewScripts();
}

