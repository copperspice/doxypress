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
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#include <QDir>
#include <QRegExp>

#include <stdlib.h>

#include <bufstr.h>
#include <config.h>
#include <dirdef.h>
#include <docparser.h>
#include <doxy_build_info.h>
#include <doxy_globals.h>
#include <logos.h>
#include <diagram.h>
#include <dot.h>
#include <htmlgen.h>
#include <htmlhelp.h>
#include <language.h>
#include <message.h>
#include <htmldocvisitor.h>
#include <searchindex.h>
#include <pagedef.h>
#include <layout.h>
#include <image.h>
#include <ftvhelp.h>
#include <resourcemgr.h>
#include <util.h>

//#define DBG_HTML(x) x;
#define DBG_HTML(x)

static QByteArray g_header;
static QByteArray g_footer;
static QByteArray g_mathjax_code;

static void writeClientSearchBox(QTextStream &t, const QString &relPath)
{
   t << "        <div id=\"MSearchBox\" class=\"MSearchBoxInactive\">\n";
   t << "        <span class=\"left\">\n";
   t << "          <img id=\"MSearchSelect\" src=\"" << relPath << "search/mag_sel.png\"\n";
   t << "               onmouseover=\"return searchBox.OnSearchSelectShow()\"\n";
   t << "               onmouseout=\"return searchBox.OnSearchSelectHide()\"\n";
   t << "               alt=\"\"/>\n";
   t << "          <input type=\"text\" id=\"MSearchField\" value=\""
     << theTranslator->trSearch() << "\" accesskey=\"S\"\n";

   t << "               onfocus=\"searchBox.OnSearchFieldFocus(true)\" \n";
   t << "               onblur=\"searchBox.OnSearchFieldFocus(false)\" \n";
   t << "               onkeyup=\"searchBox.OnSearchFieldChange(event)\"/>\n";
   t << "          </span><span class=\"right\">\n";
   t << "            <a id=\"MSearchClose\" href=\"javascript:searchBox.CloseResultsWindow()\">"
     << "<img id=\"MSearchCloseImg\" border=\"0\" src=\"" << relPath << "search/close.png\" alt=\"\"/></a>\n";

   t << "          </span>\n";
   t << "        </div>\n";
}

static void writeServerSearchBox(QTextStream &t, const QString &relPath, bool highlightSearch)
{
   static bool externalSearch = Config::getBool("external-search");

   t << "        <div id=\"MSearchBox\" class=\"MSearchBoxInactive\">\n";
   t << "          <div class=\"left\">\n";
   t << "            <form id=\"FSearchBox\" action=\"" << relPath;

   if (externalSearch) {
      t << "search" << Doxygen::htmlFileExtension;
   } else {
      t << "search.php";
   }

   t << "\" method=\"get\">\n";
   t << "              <img id=\"MSearchSelect\" src=\"" << relPath << "search/mag.png\" alt=\"\"/>\n";

   if (! highlightSearch) {
      t << "              <input type=\"text\" id=\"MSearchField\" name=\"query\" value=\""
        << theTranslator->trSearch() << "\" size=\"20\" accesskey=\"S\" \n";
      t << "                     onfocus=\"searchBox.OnSearchFieldFocus(true)\" \n";
      t << "                     onblur=\"searchBox.OnSearchFieldFocus(false)\"/>\n";
      t << "            </form>\n";
      t << "          </div><div class=\"right\"></div>\n";
      t << "        </div>\n";
   }
}

/// Clear a text block \a s from \a begin to \a end markers
QString clearBlock(const QString &output, const QString &begin, const QString &end)
{
   QString retval = output;            

   while (true) {
      int beginPos = retval.indexOf(begin);
      int endPos   = retval.indexOf(end, beginPos);
   
      if (beginPos == -1 || endPos == -1 ) {
         break;
      }
   
      int len = (endPos + end.length()) - beginPos;
      retval.replace(beginPos, len, ""); 
   }

   return retval;     
}

static QString selectBlock(const QString &s, const QString &name, bool enable)
{  
   const QString begin   = "<!--BEGIN " + name + "-->";
   const QString end     = "<!--END " + name + "-->";

   const QString nobegin = "<!--BEGIN !" + name + "-->";
   const QString noend   = "<!--END !" + name + "-->";

   QString result = s;

   if (enable) {
      result = result.replace(begin, "");
      result = result.replace(end, "");
      result = clearBlock(result, nobegin, noend);

   } else {
      result = result.replace(nobegin, "");
      result = result.replace(noend, "");
      result = clearBlock(result, begin, end);
   }

   return result;
}

static QByteArray getSearchBox(bool serverSide, const QString &relPath, bool highlightSearch)
{
   QByteArray result;
   QTextStream t(&result);

   if (serverSide) {
      writeServerSearchBox(t, relPath, highlightSearch);
   } else {
      writeClientSearchBox(t, relPath);
   }

   return result;
}

static QString removeEmptyLines(const QString &s)
{
   QString retval;
   QString line;  
  
   for (auto letter : s) {         
      line.append(letter);

      if (letter == '\n')  {

         if (line.trimmed() != "") {
            retval.append(line);
         }

         line = "";
       }    
   }

   if (line.trimmed() != "") {
      retval.append(line);
   }
   
   return retval;
}

static QString substituteHtmlKeywords(const QByteArray &output, const QString &title, 
                  const QString &relPath = QString(), const QString &navPath = QString())
{
   // Build CSS/Javascript tags depending on treeview, search engine settings  

   QByteArray generatedBy;
   QByteArray treeViewCssJs;
   QByteArray searchCssJs;
   QByteArray searchBox;
   QByteArray mathJaxJs;
   
   static QString projectName    = Config::getString("project-name");
   static QString projectVersion = Config::getString("project-version");
   static QString projectBrief   = Config::getString("project-brief");
   static QString projectLogo    = Config::getString("project-logo");

   static bool timeStamp         = Config::getBool("html-timestamp");
   static bool treeView          = Config::getBool("generate-treeview");
   static bool searchEngine      = Config::getBool("html-search");
   static bool serverBasedSearch = Config::getBool("server-based-search");

   static bool mathJax           = Config::getBool("use-mathjax");
   static QString mathJaxFormat  = Config::getEnum("mathjax-format");
   static bool disableIndex      = Config::getBool("disable-index");

   static bool hasProjectName    = ! projectName.isEmpty();
   static bool hasProjectVersion = ! projectVersion.isEmpty();
   static bool hasProjectBrief   = ! projectBrief.isEmpty();
   static bool hasProjectLogo    = ! projectLogo.isEmpty();

   static bool titleArea = (hasProjectName || hasProjectBrief || hasProjectLogo || (disableIndex && searchEngine));
    
   // always first
   QString cssFile = "doxy_style.css"; 

   QString extraCssText = "";
   const QStringList extraCssFile = Config::getList("html-stylesheets");

   for (auto fileName : extraCssFile) {          

      if (! fileName.isEmpty()) {
         QFileInfo fi(fileName);

         if (fi.exists()) {
            extraCssText += "<link href=\"$relpath^" + stripPath(qPrintable(fileName)) + "\" rel=\"stylesheet\" type=\"text/css\"/>\n";
         }
      }
   }

   if (timeStamp) {
      generatedBy = theTranslator->trGeneratedAt(dateToString(true).toUtf8(), convertToHtml(projectName.toUtf8()));

   } else {
      generatedBy = theTranslator->trGeneratedBy();
   }

   if (treeView) {
      treeViewCssJs = "<link href=\"$relpath^navtree.css\" rel=\"stylesheet\" type=\"text/css\"/>\n"
                      "<script type=\"text/javascript\" src=\"$relpath^resize.js\"></script>\n"
                      "<script type=\"text/javascript\" src=\"$relpath^navtreedata.js\"></script>\n"
                      "<script type=\"text/javascript\" src=\"$relpath^navtree.js\"></script>\n"
                      "<script type=\"text/javascript\">\n"
                      "  $(document).ready(initResizable);\n"
                      "  $(window).load(resizeHeight);\n"
                      "</script>";
   }

   if (searchEngine) {
      searchCssJs = "<link href=\"$relpath^search/search.css\" rel=\"stylesheet\" type=\"text/css\"/>\n";

      if (! serverBasedSearch) {
         searchCssJs += "<script type=\"text/javascript\" src=\"$relpath^search/searchdata.js\"></script>\n";
      }
      searchCssJs += "<script type=\"text/javascript\" src=\"$relpath^search/search.js\"></script>\n";

      if (!serverBasedSearch) {
         searchCssJs += "<script type=\"text/javascript\">\n"
                        "  $(document).ready(function() { init_search(); });\n"
                        "</script>";
      } else {
         searchCssJs += "<script type=\"text/javascript\">\n"
                        "  $(document).ready(function() {\n"
                        "    if ($('.searchresults').length > 0) { searchBox.DOMSearchField().focus(); }\n"
                        "  });\n"
                        "</script>\n";

         // OPENSEARCH_PROVIDER
         searchCssJs += "<link rel=\"search\" href=\"" + relPath +
                        "search_opensearch.php?v=opensearch.xml\" "
                        "type=\"application/opensearchdescription+xml\" title=\"" +
                        (hasProjectName ? projectName : QByteArray("DoxyPress")) +
                        "\"/>";         
      }
      searchBox = getSearchBox(serverBasedSearch, relPath, false);
   }

   if (mathJax) {
      QString path = Config::getString("mathjax-relpath");

      if (! path.isEmpty() && path.at(path.length() - 1) != '/') {
         path += "/";
      }

      if (path.isEmpty() || path.left(2) == "..") {
         // relative path
         path.prepend(relPath);
      }

      mathJaxJs = "<script type=\"text/x-mathjax-config\">\n"
                  "  MathJax.Hub.Config({\n"
                  "    extensions: [\"tex2jax.js\"";

      const QStringList mathJaxExtensions = Config::getList("mathjax-extensions");
     
      for (auto item : mathJaxExtensions) {
         mathJaxJs += ", \"" + item + ".js\"";         
      }

      if (mathJaxFormat.isEmpty()) {
         mathJaxFormat = "HTML-CSS";
      }

      mathJaxJs += "],\n"
                   "    jax: [\"input/TeX\",\"output/" + mathJaxFormat + "\"],\n"
                   "});\n";

      if (! g_mathjax_code.isEmpty()) {
         mathJaxJs += g_mathjax_code;
         mathJaxJs += "\n";
      }

      mathJaxJs += "</script>";
      mathJaxJs += "<script src=\"" + path + "MathJax.js\"></script>\n";
   }

   QString result = output.constData();

   // first substitute generic keywords
   if (! title.isEmpty()) {
      result = result.replace("$title", convertToHtml(title));
   }

   result = result.replace("$datetimeHHMM",   dateTimeHHMM());
   result = result.replace("$datetime",       dateToString(true));
   result = result.replace("$date",           dateToString(false));
   result = result.replace("$year",           yearToString());

   result = result.replace("$doxygenversion", versionString);

   result = result.replace("$projectname",    convertToHtml(projectName));  
   result = result.replace("$projectversion", convertToHtml(projectVersion));
   result = result.replace("$projectbrief",   convertToHtml(projectBrief));
   result = result.replace("$projectlogo",    stripPath(projectLogo));

   // additional HTML only keywords
   result = result.replace("$navpath",         navPath);
   result = result.replace("$stylesheet",      cssFile);
   result = result.replace("$extrastylesheet", extraCssText);
   result = result.replace("$treeview",        treeViewCssJs);
   result = result.replace("$searchbox",       searchBox);
   result = result.replace("$search",          searchCssJs);
   result = result.replace("$mathjax",         mathJaxJs);
   result = result.replace("$generatedby",     generatedBy);

   result = result.replace("$relpath$", relPath); //<-- obsolete: for backwards compatibility only
   result = result.replace("$relpath^", relPath); //<-- must be last

   // additional HTML only conditional blocks
   result = selectBlock(result, "DISABLE_INDEX",     disableIndex);
   result = selectBlock(result, "GENERATE_TREEVIEW", treeView);
   result = selectBlock(result, "SEARCHENGINE",      searchEngine);
   result = selectBlock(result, "TITLEAREA",         titleArea);
   result = selectBlock(result, "PROJECT_NAME",      hasProjectName);
   result = selectBlock(result, "PROJECT_VERSION",   hasProjectVersion);
   result = selectBlock(result, "PROJECT_BRIEF",     hasProjectBrief);
   result = selectBlock(result, "PROJECT_LOGO",      hasProjectLogo);

   result = removeEmptyLines(result);

   return result;
}

HtmlCodeGenerator::HtmlCodeGenerator(QTextStream &t, const QByteArray &relPath)
   : m_col(0), m_relPath(relPath), m_streamX(t)
{  
}

void HtmlCodeGenerator::setRelativePath(const QByteArray &path)
{
   m_relPath = path;
}

void HtmlCodeGenerator::codify(const QByteArray &str)
{
   static int tabSize = Config::getInt("tab-size");

   if (! str.isEmpty()) {
      const char *p = str.constData();

      char c;
      int spacesToNextTabStop;

      while (*p) {
         c = *p++;

         switch (c) {

            case '\t':
               spacesToNextTabStop = tabSize - (m_col % tabSize);
               m_streamX << QString(spacesToNextTabStop, ' ');

               m_col += spacesToNextTabStop;

               break;

            case '\n':
               m_streamX << "\n";
               m_col = 0;
               break;

            case '\r':
               break;

            case '<':
               m_streamX << "&lt;";
               m_col++;
               break;

            case '>':
               m_streamX << "&gt;";
               m_col++;
               break;

            case '&':
               m_streamX << "&amp;";
               m_col++;
               break;

            case '\'':
               m_streamX << "&#39;";
               m_col++; // &apos; is not valid XHTML
               break;

            case '"':
               m_streamX << "&quot;";
               m_col++;
               break;

            case '\\':
               if (*p == '<') {
                  m_streamX << "&lt;";
                  p++;
               } else if (*p == '>') {
                  m_streamX << "&gt;";
                  p++;

               } else {
                  m_streamX << "\\";
               }

               m_col++;
               break;

            default:
               p = writeUtf8Char(m_streamX, p - 1);
               m_col++;
               break;
         }
      }

   }
}

void HtmlCodeGenerator::docify(const QByteArray &str)
{
   if (! str.isEmpty()) {
      const char *p = str.constData();
      char c;

      while (*p) {
         c = *p++;

         switch (c) {
            case '<':
               m_streamX << "&lt;";
               break;
            case '>':
               m_streamX << "&gt;";
               break;
            case '&':
               m_streamX << "&amp;";
               break;
            case '"':
               m_streamX << "&quot;";
               break;
            case '\\':
               if (*p == '<') {
                  m_streamX << "&lt;";
                  p++;
               } else if (*p == '>') {
                  m_streamX << "&gt;";
                  p++;
               } else {
                  m_streamX << "\\";
               }
               break;
            default:
               m_streamX << c;
         }
      }      
   }
}

void HtmlCodeGenerator::writeLineNumber(const char *ref, const QByteArray &filename, const char *anchor, int l)
{
   const int maxLineNrStr = 10;
   char lineNumber[maxLineNrStr];
   char lineAnchor[maxLineNrStr];

   qsnprintf(lineNumber, maxLineNrStr, "%5d", l);
   qsnprintf(lineAnchor, maxLineNrStr, "l%05d", l);

   m_streamX << "<div class=\"line\">";
   m_streamX << "<a name=\"" << lineAnchor << "\"></a><span class=\"lineno\">";

   if (! filename.isEmpty()) {
      _writeCodeLink("line", ref, filename, anchor, lineNumber, 0);
   } else {
      codify(lineNumber);
   }

   m_streamX << "</span>";
   m_streamX << "&#160;";
}

void HtmlCodeGenerator::writeCodeLink(const QByteArray &ref, const QByteArray &f, const QByteArray &anchor,
                                      const QByteArray &name, const QByteArray &tooltip)
{    
   _writeCodeLink("code", ref, f, anchor, name, tooltip);
}

void HtmlCodeGenerator::_writeCodeLink(const QByteArray &className, const QByteArray &ref, const QByteArray &f,
                                       const QByteArray &anchor, const QByteArray &name, const QByteArray &tooltip)
{
   if (! ref.isEmpty()) {
      m_streamX << "<a class=\"" << className << "Ref\" ";
      m_streamX << externalLinkTarget() << externalRef(m_relPath, ref, false);

   } else {
      m_streamX << "<a class=\"" << className << "\" ";
   }

   m_streamX << "href=\"";
   m_streamX << externalRef(m_relPath, ref, true);

   if (! f.isEmpty()) {
      m_streamX << f << Doxygen::htmlFileExtension;
   }

   if (! anchor.isEmpty()) {
      m_streamX << "#" << anchor;
   }

  m_streamX << "\"";

   if (! tooltip.isEmpty()) {
      m_streamX << " title=\"" << tooltip << "\"";
   }

   m_streamX << ">";
   docify(name);

   m_streamX << "</a>";
   m_col += qstrlen(name);
}

void HtmlCodeGenerator::writeTooltip(const char *id, const DocLinkInfo &docInfo, const QByteArray &decl, 
                                     const QByteArray &desc, const SourceLinkInfo &defInfo, const SourceLinkInfo &declInfo)
{
   m_streamX << "<div class=\"ttc\" id=\"" << id << "\">";
   m_streamX << "<div class=\"ttname\">";

   if (! docInfo.url.isEmpty()) {
      m_streamX << "<a href=\"";
      m_streamX << externalRef(m_relPath, docInfo.ref, true);
      m_streamX << docInfo.url << Doxygen::htmlFileExtension;

      if (!docInfo.anchor.isEmpty()) {
         m_streamX << "#" << docInfo.anchor;
      }

      m_streamX << "\">";
   }

   docify(docInfo.name);

   if (! docInfo.url.isEmpty()) {
      m_streamX << "</a>";
   }

   m_streamX << "</div>";

   if (! decl.isEmpty()) {
      m_streamX << "<div class=\"ttdeci\">";
      docify(decl);
      m_streamX << "</div>";
   }

   if (! desc.isEmpty()) {
      m_streamX << "<div class=\"ttdoc\">";
      m_streamX << desc; // desc is already HTML escaped
      m_streamX << "</div>";
   }

   if (! defInfo.file.isEmpty()) {
      m_streamX << "<div class=\"ttdef\"><b>Definition:</b> ";
      if (!defInfo.url.isEmpty()) {
         m_streamX << "<a href=\"";
         m_streamX << externalRef(m_relPath, defInfo.ref, true);
         m_streamX << defInfo.url << Doxygen::htmlFileExtension;
         if (!defInfo.anchor.isEmpty()) {
            m_streamX << "#" << defInfo.anchor;
         }
         m_streamX << "\">";
      }

      m_streamX << defInfo.file << ":" << defInfo.line;
      if (!defInfo.url.isEmpty()) {
         m_streamX << "</a>";
      }
      m_streamX << "</div>";
   }

   if (! declInfo.file.isEmpty()) {
      m_streamX << "<div class=\"ttdecl\"><b>Declaration:</b> ";
      if (!declInfo.url.isEmpty()) {
         m_streamX << "<a href=\"";
         m_streamX << externalRef(m_relPath, declInfo.ref, true);
         m_streamX << declInfo.url << Doxygen::htmlFileExtension;
         if (!declInfo.anchor.isEmpty()) {
            m_streamX << "#" << declInfo.anchor;
         }
         m_streamX << "\">";
      }
      m_streamX << declInfo.file << ":" << declInfo.line;
      if (!declInfo.url.isEmpty()) {
         m_streamX << "</a>";
      }
      m_streamX << "</div>";
   }

   m_streamX << "</div>" << endl;
}


void HtmlCodeGenerator::startCodeLine(bool hasLineNumbers)
{  
   if (! hasLineNumbers) {
      m_streamX << "<div class=\"line\">";
   }

   m_col = 0;   
}

void HtmlCodeGenerator::endCodeLine()
{
   m_streamX << "</div>\n";  
}

void HtmlCodeGenerator::startFontClass(const char *s)
{
   m_streamX << "<span class=\"" << s << "\">";   
}

void HtmlCodeGenerator::endFontClass()
{
   m_streamX << "</span>";
}

void HtmlCodeGenerator::writeCodeAnchor(const char *anchor)
{ 
   m_streamX << "<a name=\"" << anchor << "\"></a>";   
}

HtmlGenerator::HtmlGenerator() : OutputGenerator()
{
   m_dir = Config::getString("html-output");
   m_emptySection = false; 
}

HtmlGenerator::~HtmlGenerator()
{  
}

void HtmlGenerator::init()
{
   QString dname = Config::getString("html-output");

   QDir d(dname);

   if (! d.exists() && ! d.mkdir(dname)) {
      err("Unable to create output directory %s\n", qPrintable(dname));      
      exit(1);
   }
   
   QString htmlHeader = Config::getString("html-header");

   if (! htmlHeader.isEmpty()) {
      g_header = fileToString(htmlHeader);      
   } else {
      g_header = ResourceMgr::instance().getAsString("html/header.html");
   }

   QString htmlFooter = Config::getString("html-footer");

   if (! htmlFooter.isEmpty()) {
      g_footer = fileToString(htmlFooter);      
   } else {
      g_footer = ResourceMgr::instance().getAsString("html/footer.html");
   }

   if (Config::getBool("use-mathjax")) {
      QString temp = Config::getString("mathjax-codefile");

      if (! temp.isEmpty()) {
         g_mathjax_code = fileToString(temp);         
      }
   }
   createSubDirs(d);

   ResourceMgr &mgr = ResourceMgr::instance();
   mgr.copyResourceAs("html/tabs.css",  dname, "tabs.css");
   mgr.copyResourceAs("html/jquery.js", dname, "jquery.js");

   if (Config::getBool("interactive-svg")) {
      mgr.copyResourceAs("html/svgpan.js", dname, "svgpan.js");
   }

   QString fileName = dname + "/dynsections.js";   
   QFile f(fileName);

   if (f.open(QIODevice::WriteOnly)) {

      QByteArray resource = mgr.getAsString("html/dynsections.js");

      if (! resource.isEmpty()) {
         QTextStream t(&f);
         t << resource.constData();

         if (Config::getBool("source-code") && Config::getBool("source-tooltips")) {
            t << endl <<
              "$(document).ready(function() {\n"
              "  $('.code,.codeRef').each(function() {\n"
              "    $(this).data('powertip',$('#'+$(this).attr('href').replace(/.*\\//,'').replace(/[^a-z_A-Z0-9]/g,'_')).html());\n"
              "    $(this).powerTip({ placement: 's', smartPlacement: true, mouseOnToPopup: true });\n"
              "  });\n"
              "});\n";
         }

      } else {
         err("Resource file dynsections.js was not found");

      }

   } else { 
      err("Unable to open file for writing %s, error: %d\n", qPrintable(fileName), f.error());

   }
  
}

/// Additional initialization after indices have been created
void HtmlGenerator::writeTabData()
{
   Doxygen::indexList->addStyleSheetFile("tabs.css");
   QString dname = Config::getString("html-output");

   ResourceMgr &mgr = ResourceMgr::instance();

   //writeColoredImgData(dname,colored_tab_data);
   mgr.copyResourceAs("html/tab_a.lum",    dname, "tab_a.png");
   mgr.copyResourceAs("html/tab_b.lum",    dname, "tab_b.png");
   mgr.copyResourceAs("html/tab_h.lum",    dname, "tab_h.png");
   mgr.copyResourceAs("html/tab_s.lum",    dname, "tab_s.png");
   mgr.copyResourceAs("html/nav_h.lum",    dname, "nav_h.png");
   mgr.copyResourceAs("html/nav_f.lum",    dname, "nav_f.png");
   mgr.copyResourceAs("html/bc_s.luma",    dname, "bc_s.png");
   mgr.copyResourceAs("html/doxygen.luma", dname, "doxygen.png");
   mgr.copyResourceAs("html/closed.luma",  dname, "closed.png");
   mgr.copyResourceAs("html/open.luma",    dname, "open.png");
   mgr.copyResourceAs("html/bdwn.luma",    dname, "bdwn.png");
   mgr.copyResourceAs("html/sync_on.luma", dname, "sync_on.png");
   mgr.copyResourceAs("html/sync_off.luma",dname, "sync_off.png");    
   mgr.copyResourceAs("html/nav_g.png",    dname, "nav_g.png");
}

void HtmlGenerator::writeSearchData(const QString &dir)
{
   static bool serverBasedSearch = Config::getBool("server-based-search");
  
   ResourceMgr &mgr = ResourceMgr::instance();

   mgr.copyResourceAs("html/search_l.png", dir, "search_l.png");
   Doxygen::indexList->addImageFile("search/search_l.png");

   mgr.copyResourceAs("html/search_m.png", dir, "search_m.png");
   Doxygen::indexList->addImageFile("search/search_m.png");

   mgr.copyResourceAs("html/search_r.png", dir, "search_r.png");
   Doxygen::indexList->addImageFile("search/search_r.png");

   if (serverBasedSearch) {
      mgr.copyResourceAs("html/mag.png", dir, "mag.png");
      Doxygen::indexList->addImageFile("search/mag.png");

   } else {
      mgr.copyResourceAs("html/close.png", dir, "close.png");
      Doxygen::indexList->addImageFile("search/close.png");

      mgr.copyResourceAs("html/mag_sel.png", dir, "mag_sel.png");
      Doxygen::indexList->addImageFile("search/mag_sel.png");
   }

   QString outputName = Config::getString("html-output") + "/search";

   QString fileName = outputName + "/search.css";
   QFile f(fileName);

   if (f.open(QIODevice::WriteOnly)) {

      QByteArray resData = mgr.getAsString("html/search.css");
      
      if (! resData.isEmpty()) {

         QTextStream t(&f);
         QByteArray searchCss = replaceColorMarkers(resData);
         searchCss.replace("$doxygenversion", versionString);

         if (Config::getBool("disable-index")) {
            // move up the search box if there are no tabs
            searchCss = substitute(searchCss, "margin-top: 8px;", "margin-top: 0px;");
         }

         t << searchCss;
         Doxygen::indexList->addStyleSheetFile("search/search.css");
      }

   } else {
      err("Unable to open file for writing %s, error: %d\n", qPrintable(fileName), f.error()); 
   }
}

void HtmlGenerator::writeStyleSheetFile(QFile &file)
{
   QTextStream t(&file);
 
   QByteArray resData = ResourceMgr::instance().getAsString("html/doxy_style.css");

   if (resData.isEmpty()) { 
      fprintf(stderr, "\n\nIssue loading the default stylesheet file.\nPlease submit a bug report to " 
              " the developers at doxypress@copperspice.org\n");        

   } else {
      resData.replace("$doxygenversion", versionString);
      t << replaceColorMarkers(resData);

   }
}

void HtmlGenerator::writeHeaderFile(QFile &file)
{
   QTextStream t(&file);

   t << "<!-- HTML header for DoxyPress " << versionString << "-->" << endl;
   t << ResourceMgr::instance().getAsString("html/header.html");
}

void HtmlGenerator::writeFooterFile(QFile &file)
{
   QTextStream t(&file);

   t << "<!-- HTML footer for DoxyPress " << versionString << "-->" <<  endl;
   t << ResourceMgr::instance().getAsString("html/footer.html");
}

void HtmlGenerator::startFile(const QString &name, const QString &, const QString &title)
{   
   QString fileName = name;

   m_lastTitle    = title;
   m_relativePath = relativePathToRoot(fileName);

   if (fileName.right(Doxygen::htmlFileExtension.length()) != Doxygen::htmlFileExtension) {
      fileName += Doxygen::htmlFileExtension;
   }

   startPlainFile(fileName);
  
   m_codeGen = QMakeShared<HtmlCodeGenerator> (m_textStream, m_relativePath.toUtf8());
  
   //
   Doxygen::indexList->addIndexFile(fileName);

   m_lastFile = fileName;
   m_textStream << substituteHtmlKeywords(g_header, filterTitle(title), m_relativePath);

   m_textStream << "<!-- " << theTranslator->trGeneratedBy() << " DoxyPress " << versionString << " -->" << endl;
   
   static bool searchEngine = Config::getBool("html-search");

   if (searchEngine /*&& !generateTreeView*/) {
      m_textStream << "<script type=\"text/javascript\">\n";

      m_textStream << "var searchBox = new SearchBox(\"searchBox\", \""
        << m_relativePath << "search\",false,'" << theTranslator->trSearch() << "');\n";

      m_textStream << "</script>\n";
   }
  
   m_sectionCount = 0;
}

void HtmlGenerator::writeSearchInfo(QTextStream &t_stream, const QString &relPath)
{
   static bool searchEngine      = Config::getBool("html-search");
   static bool serverBasedSearch = Config::getBool("server-based-search");

   if (searchEngine && !serverBasedSearch) {
      (void)relPath;

      t_stream << "<!-- window showing the filter options -->\n";
      t_stream << "<div id=\"MSearchSelectWindow\"\n";
      t_stream << "     onmouseover=\"return searchBox.OnSearchSelectShow()\"\n";
      t_stream << "     onmouseout=\"return searchBox.OnSearchSelectHide()\"\n";
      t_stream << "     onkeydown=\"return searchBox.OnSearchSelectKey(event)\">\n";
      t_stream << "</div>\n";
      t_stream << "\n";
      t_stream << "<!-- iframe showing the search results (closed by default) -->\n";
      t_stream << "<div id=\"MSearchResultsWindow\">\n";
      t_stream << "<iframe src=\"javascript:void(0)\" frameborder=\"0\" \n";
      t_stream << "        name=\"MSearchResults\" id=\"MSearchResults\">\n";
      t_stream << "</iframe>\n";
      t_stream << "</div>\n";
      t_stream << "\n";
   }
}

void HtmlGenerator::writeSearchInfo()
{
   writeSearchInfo(m_textStream, m_relativePath);
}

QByteArray HtmlGenerator::writeLogoAsString(const QString &path)
{
   static bool timeStamp = Config::getBool("html-timestamp");
   QString result;

   if (timeStamp) {
      result += theTranslator->trGeneratedAt(dateToString(true).toUtf8(), Config::getString("project-name").toUtf8()); 

   } else {
      result += theTranslator->trGeneratedBy();
   }

   result += "&#160;\n<a href=\"http://www.doxygen.org/index.html\">\n"
             "<img class=\"footer\" src=\"";

   result += path;
   result += "doxygen.png\" alt=\"doxygen\"/></a> ";
   result += versionString;
   result += " ";

   return result.toUtf8();
}

void HtmlGenerator::writeLogo()
{
   m_textStream << writeLogoAsString(m_relativePath);
}

void HtmlGenerator::writePageFooter(QTextStream &t_stream, const QString &lastTitle, const QString &relPath, const QString &navPath)
{
   t_stream << substituteHtmlKeywords(g_footer, lastTitle, relPath, navPath);
}

void HtmlGenerator::writeFooter(const QString &navPath)
{
   writePageFooter(m_textStream, m_lastTitle, m_relativePath, navPath);
}

void HtmlGenerator::endFile()
{
   endPlainFile();
}

void HtmlGenerator::startProjectNumber()
{
   m_textStream << "<h3 class=\"version\">";
}

void HtmlGenerator::endProjectNumber()
{
   m_textStream << "</h3>";
}

void HtmlGenerator::writeStyleInfo(int part)
{   
   if (part == 0) {
      // write default style sheet
      startPlainFile("doxy_style.css");       
      QByteArray resData = ResourceMgr::instance().getAsString("html/doxy_style.css");

      if (resData.isEmpty()) { 
         fprintf(stderr, "\n\nIssue loading the default stylesheet file.\nPlease submit a bug report to " 
               " the developers at doxypress@copperspice.org\n");        

      } else {
         resData.replace("$doxygenversion", versionString);
         m_textStream << replaceColorMarkers(resData);

      }

      endPlainFile();

      Doxygen::indexList->addStyleSheetFile("doxy_style.css");     

      // part two
      static const QStringList extraCssFile = Config::getList("html-stylesheets");

      for (auto fileName : extraCssFile) {
         
         if (! fileName.isEmpty()) {
            QFileInfo fi(fileName);

            if (fi.exists()) {
               Doxygen::indexList->addStyleSheetFile(fi.fileName());
            }
         }
      }
   }
}

void HtmlGenerator::startDoxyAnchor(const char *, const char *, const char *anchor, const char *, const char *)
{
   m_textStream << "<a class=\"anchor\" id=\"" << anchor << "\"></a>";
}

void HtmlGenerator::endDoxyAnchor(const char *, const char *)
{
}

//void HtmlGenerator::newParagraph()
//{
//  m_textStream << endl << "<p>" << endl;
//}

void HtmlGenerator::startParagraph()
{
   m_textStream << endl << "<p>";
}

void HtmlGenerator::endParagraph()
{
   m_textStream << "</p>" << endl;
}

void HtmlGenerator::writeString(const char *text)
{
   m_textStream << text;
}

void HtmlGenerator::startIndexListItem()
{
   m_textStream << "<li>";
}

void HtmlGenerator::endIndexListItem()
{
   m_textStream << "</li>" << endl;
}

void HtmlGenerator::startIndexItem(const QByteArray &ref, const QByteArray &f)
{   
   if (! ref.isEmpty() || ! f.isEmpty()) {

      if (! ref.isEmpty()) {
         m_textStream << "<a class=\"elRef\" ";
         m_textStream << externalLinkTarget() << externalRef(m_relativePath, ref, false);

      } else {
         m_textStream << "<a class=\"el\" ";
      }

      m_textStream << "href=\"";
      m_textStream << externalRef(m_relativePath, ref, true);

      if (! f.isEmpty()) {
         m_textStream << f << Doxygen::htmlFileExtension << "\">";
      }

   } else {
      m_textStream << "<b>";
   }
}

void HtmlGenerator::endIndexItem(const QByteArray &ref, const QByteArray &f)
{  
   if (! ref.isEmpty() || ! f.isEmpty()) {
      m_textStream << "</a>";

   } else {
      m_textStream << "</b>";
   }
}

void HtmlGenerator::writeStartAnnoItem(const char *, const QByteArray &f, const QByteArray &path, const char *name)
{
   m_textStream << "<li>";

   if (! path.isEmpty()) {
      docify(path);
   }

   m_textStream << "<a class=\"el\" href=\"" << f << Doxygen::htmlFileExtension << "\">";
   docify(name);

   m_textStream << "</a> ";
}

void HtmlGenerator::writeObjectLink(const QByteArray &ref, const QByteArray &f, const QByteArray &anchor, const QByteArray &name)
{
   if (! ref.isEmpty()) {
      m_textStream << "<a class=\"elRef\" ";
      m_textStream << externalLinkTarget() << externalRef(m_relativePath, ref, false);

   } else {
      m_textStream << "<a class=\"el\" ";
   }

   m_textStream << "href=\"";
   m_textStream << externalRef(m_relativePath, ref, true);

   if (! f.isEmpty()) {
      m_textStream << f << Doxygen::htmlFileExtension;
   }

   if (! anchor.isEmpty()) {
      m_textStream << "#" << anchor;
   }

   m_textStream << "\">";
   docify(name);

   m_textStream << "</a>";
}

void HtmlGenerator::startTextLink(const QByteArray &f, const QByteArray &anchor)
{
   m_textStream << "<a href=\"";

   if (! f.isEmpty()) {
      m_textStream << m_relativePath << f << Doxygen::htmlFileExtension;
   }

   if (! anchor.isEmpty()) {
      m_textStream << "#" << anchor;
   }

   m_textStream << "\">";
}

void HtmlGenerator::endTextLink()
{
   m_textStream << "</a>";
}

void HtmlGenerator::startHtmlLink(const QString &url)
{
   static bool generateTreeView = Config::getBool("generate-treeview");
   m_textStream << "<a ";

   if (generateTreeView) {
      m_textStream << "target=\"top\" ";
   }

   m_textStream << "href=\"";

   if (! url.isEmpty()) {
      m_textStream << url;
   }

   m_textStream << "\">";
}

void HtmlGenerator::endHtmlLink()
{
   m_textStream << "</a>";
}

void HtmlGenerator::startGroupHeader(int extraIndentLevel)
{
   if (extraIndentLevel == 2) {
      m_textStream << "<h4 class=\"groupheader\">";

   } else if (extraIndentLevel == 1) {
      m_textStream << "<h3 class=\"groupheader\">";

   } else { // extraIndentLevel==0
      m_textStream << "<h2 class=\"groupheader\">";
   }
}

void HtmlGenerator::endGroupHeader(int extraIndentLevel)
{
   if (extraIndentLevel == 2) {
      m_textStream << "</h4>" << endl;

   } else if (extraIndentLevel == 1) {
      m_textStream << "</h3>" << endl;

   } else {
      m_textStream << "</h2>" << endl;
   }
}

void HtmlGenerator::startSection(const char *lab, const char *, SectionInfo::SectionType type)
{
   switch (type) {
      case SectionInfo::Page:
         m_textStream << "\n\n<h1>";
         break;
      case SectionInfo::Section:
         m_textStream << "\n\n<h2>";
         break;
      case SectionInfo::Subsection:
         m_textStream << "\n\n<h3>";
         break;
      case SectionInfo::Subsubsection:
         m_textStream << "\n\n<h4>";
         break;
      case SectionInfo::Paragraph:
         m_textStream << "\n\n<h5>";
         break;
      default:
         assert(0);
         break;
   }

   m_textStream << "<a class=\"anchor\" id=\"" << lab << "\"></a>";
}

void HtmlGenerator::endSection(const char *, SectionInfo::SectionType type)
{
   switch (type) {
      case SectionInfo::Page:
         m_textStream << "</h1>";
         break;
      case SectionInfo::Section:
         m_textStream << "</h2>";
         break;
      case SectionInfo::Subsection:
         m_textStream << "</h3>";
         break;
      case SectionInfo::Subsubsection:
         m_textStream << "</h4>";
         break;
      case SectionInfo::Paragraph:
         m_textStream << "</h5>";
         break;
      default:
         assert(0);
         break;
   }
}

void HtmlGenerator::docify(const QByteArray &str)
{
   docify(str, false);
}

void HtmlGenerator::docify(const QByteArray &str, bool inHtmlComment)
{
   if (! str.isEmpty()) {
      const char *p = str.constData();
      char c;

      while (*p) {
         c = *p++;
         switch (c) {
            case '<':
               m_textStream << "&lt;";
               break;
            case '>':
               m_textStream << "&gt;";
               break;
            case '&':
               m_textStream << "&amp;";
               break;
            case '"':
               m_textStream << "&quot;";
               break;
            case '-':
               if (inHtmlComment) {
                  m_textStream << "&#45;";
               } else {
                  m_textStream << "-";
               }
               break;
            case '\\':
               if (*p == '<') {
                  m_textStream << "&lt;";
                  p++;
               } else if (*p == '>') {
                  m_textStream << "&gt;";
                  p++;
               } else {
                  m_textStream << "\\";
               }
               break;
            default:
               m_textStream << c;
         }
      }
   }
}

void HtmlGenerator::writeChar(char c)
{
   char cs[2];
   cs[0] = c;
   cs[1] = 0;

   docify(cs);
}

static void startSectionHeader(QTextStream &t_stream, const QString &relPath, int sectionCount)
{ 
   static bool dynamicSections = Config::getBool("html-dynamic-sections");

   if (dynamicSections) {
      t_stream << "<div id=\"dynsection-" << sectionCount << "\" "
        "onclick=\"return toggleVisibility(this)\" "
        "class=\"dynheader closed\" "
        "style=\"cursor:pointer;\">" << endl;

      t_stream << "  <img id=\"dynsection-" << sectionCount << "-trigger\" src=\""
        << relPath << "closed.png\" alt=\"+\"/> ";

   } else {
      t_stream << "<div class=\"dynheader\">" << endl;

   }
}

static void endSectionHeader(QTextStream &t_stream)
{
   // m_stream << "<!-- endSectionHeader -->";

   t_stream << "</div>" << endl;
}

static void startSectionSummary(QTextStream &t_stream, int sectionCount)
{
   //t << "<!-- startSectionSummary -->";
   static bool dynamicSections = Config::getBool("html-dynamic-sections");

   if (dynamicSections) {
      t_stream << "<div id=\"dynsection-" << sectionCount << "-summary\" "
         "class=\"dynsummary\" "
         "style=\"display:block;\">" << endl;
   }
}

static void endSectionSummary(QTextStream &t_stream)
{
   //t << "<!-- endSectionSummary -->";
   static bool dynamicSections = Config::getBool("html-dynamic-sections");

   if (dynamicSections) {
      t_stream << "</div>" << endl;
   }
}

static void startSectionContent(QTextStream &t_stream, int sectionCount)
{
   //t << "<!-- startSectionContent -->";
   static bool dynamicSections = Config::getBool("html-dynamic-sections");

   if (dynamicSections) {
      t_stream << "<div id=\"dynsection-" << sectionCount << "-content\" "
        "class=\"dyncontent\" "
        "style=\"display:none;\">" << endl;

   } else {
      t_stream << "<div class=\"dyncontent\">" << endl;
   }
}

static void endSectionContent(QTextStream &t_stream)
{
   //t << "<!-- endSectionContent -->";
   t_stream << "</div>" << endl;
}

void HtmlGenerator::startClassDiagram()
{
   startSectionHeader(m_textStream, m_relativePath, m_sectionCount);
}

void HtmlGenerator::endClassDiagram(const ClassDiagram &d, const char *fname, const char *name)
{
   endSectionHeader(m_textStream);
   startSectionSummary(m_textStream, m_sectionCount);

   endSectionSummary(m_textStream);
   startSectionContent(m_textStream, m_sectionCount);

   m_textStream << " <div class=\"center\">" << endl;
   m_textStream << "  <img src=\"";
   m_textStream << m_relativePath << fname << ".png\" usemap=\"#";
   docify(name);

   m_textStream << "_map\" alt=\"\"/>" << endl;
   m_textStream << "  <map id=\"";
   docify(name);

   m_textStream << "_map\" name=\"";
   docify(name);

   m_textStream << "_map\">" << endl;
   d.writeImage(m_textStream, m_dir, m_relativePath, fname);

   m_textStream << " </div>";
   endSectionContent(m_textStream);

   m_sectionCount++;
}


void HtmlGenerator::startMemberList()
{
   DBG_HTML(m_textStream << "<!-- startMemberList -->" << endl)
}

void HtmlGenerator::endMemberList()
{
   DBG_HTML(m_textStream << "<!-- endMemberList -->" << endl)
}

// anonymous type:
//  0 = single column right aligned
//  1 = double column left aligned
//  2 = single column left aligned
void HtmlGenerator::startMemberItem(const char *anchor, int annoType, const QByteArray &inheritId)
{
   DBG_HTML(m_textStream << "<!-- startMemberItem() -->" << endl)

   if (m_emptySection) {
      m_textStream << "<table class=\"memberdecls\">" << endl;
      m_emptySection = false;
   }

   m_textStream << "<tr class=\"memitem:" << anchor;
   if (! inheritId.isEmpty()) {
      m_textStream << " inherit " << inheritId;
   }

   m_textStream << "\">";

   switch (annoType) {
      case 0:
         m_textStream << "<td class=\"memItemLeft\" align=\"right\" valign=\"top\">";
         break;
      case 1:
         m_textStream << "<td class=\"memItemLeft\" >";
         break;
      case 2:
         m_textStream << "<td class=\"memItemLeft\" valign=\"top\">";
         break;
      default:
         m_textStream << "<td class=\"memTemplParams\" colspan=\"2\">";
         break;
   }
}

void HtmlGenerator::endMemberItem()
{
   m_textStream << "</td></tr>";
   m_textStream << endl;
}

void HtmlGenerator::startMemberTemplateParams()
{
}

void HtmlGenerator::endMemberTemplateParams(const char *anchor, const  QByteArray &inheritId)
{
   m_textStream << "</td></tr>" << endl;
   m_textStream << "<tr class=\"memitem:" << anchor;

   if (! inheritId.isEmpty()) {
      m_textStream << " inherit " << inheritId;
   }

   m_textStream << "\"><td class=\"memTemplItemLeft\" align=\"right\" valign=\"top\">";
}


void HtmlGenerator::insertMemberAlign(bool templ)
{
   DBG_HTML(m_textStream << "<!-- insertMemberAlign -->" << endl)
   QByteArray className = templ ? "memTemplItemRight" : "memItemRight";

   m_textStream << "&#160;</td><td class=\"" << className << "\" valign=\"bottom\">";
}

void HtmlGenerator::startMemberDescription(const char *anchor, const  QByteArray &inheritId)
{
   DBG_HTML(m_textStream << "<!-- startMemberDescription -->" << endl)

   if (m_emptySection) {
      m_textStream << "<table class=\"memberdecls\">" << endl;
      m_emptySection = false;
   }

   m_textStream << "<tr class=\"memdesc:" << anchor;

   if (! inheritId.isEmpty()) {
      m_textStream << " inherit " << inheritId;
   }

   m_textStream << "\"><td class=\"mdescLeft\">&#160;</td><td class=\"mdescRight\">";
}

void HtmlGenerator::endMemberDescription()
{
   DBG_HTML(m_textStream << "<!-- endMemberDescription -->" << endl)
   m_textStream << "<br /></td></tr>" << endl;
}

void HtmlGenerator::startMemberSections()
{
   DBG_HTML(m_textStream << "<!-- startMemberSections -->" << endl)
   m_emptySection = true; 

   // we postpone writing <table> until we actually
   // write a row to prevent empty tables, which
   // are not valid XHTML!
}

void HtmlGenerator::endMemberSections()
{
   DBG_HTML(m_textStream << "<!-- endMemberSections -->" << endl)

   if (! m_emptySection) {
      m_textStream << "</table>" << endl;
   }
}

void HtmlGenerator::startMemberHeader(const char *anchor)
{
   DBG_HTML(m_textStream << "<!-- startMemberHeader -->" << endl)

   if (! m_emptySection) {
      m_textStream << "</table>";
      m_emptySection = true;
   }

   if (m_emptySection) {
      m_textStream << "<table class=\"memberdecls\">" << endl;
      m_emptySection = false;
   }

   m_textStream << "<tr class=\"heading\"><td colspan=\"2\"><h2 class=\"groupheader\">";

   if (anchor) {
      m_textStream << "<a name=\"" << anchor << "\"></a>" << endl;
   }
}

void HtmlGenerator::endMemberHeader()
{
   DBG_HTML(m_textStream << "<!-- endMemberHeader -->" << endl)
   m_textStream << "</h2></td></tr>" << endl;
}

void HtmlGenerator::startMemberSubtitle()
{
   DBG_HTML( << "<!-- startMemberSubtitle -->" << endl)
   m_textStream << "<tr><td class=\"ititle\" colspan=\"2\">";
}

void HtmlGenerator::endMemberSubtitle()
{
   DBG_HTML(m_textStream << "<!-- endMemberSubtitle -->" << endl)
   m_textStream << "</td></tr>" << endl;
}

void HtmlGenerator::startIndexList()
{
   m_textStream << "<table>"  << endl;
}

void HtmlGenerator::endIndexList()
{
   m_textStream << "</table>" << endl;
}

void HtmlGenerator::startIndexKey()
{
   // inserted 'class = ...', 02 jan 2002, jh
   m_textStream << "  <tr><td class=\"indexkey\">";
}

void HtmlGenerator::endIndexKey()
{
   m_textStream << "</td>";
}

void HtmlGenerator::startIndexValue(bool)
{
   // inserted 'class = ...', 02 jan 2002, jh
   m_textStream << "<td class=\"indexvalue\">";
}

void HtmlGenerator::endIndexValue(const char *, bool)
{
   m_textStream << "</td></tr>" << endl;
}

void HtmlGenerator::startMemberDocList()
{
   DBG_HTML(m_textStream << "<!-- startMemberDocList -->" << endl;)
}

void HtmlGenerator::endMemberDocList()
{
   DBG_HTML(m_textStream << "<!-- endMemberDocList -->" << endl;)
}

void HtmlGenerator::startMemberDoc(const char *, const char *, const char *, const char *, bool)
{
   DBG_HTML(m_textStream << "<!-- startMemberDoc -->" << endl;)

   m_textStream << "\n<div class=\"memitem\">" << endl;
   m_textStream << "<div class=\"memproto\">" << endl;
}

void HtmlGenerator::startMemberDocPrefixItem()
{
   DBG_HTML(m_textStream << "<!-- startMemberDocPrefixItem -->" << endl;)
   m_textStream << "<div class=\"memtemplate\">" << endl;
}

void HtmlGenerator::endMemberDocPrefixItem()
{
   DBG_HTML(m_textStream << "<!-- endMemberDocPrefixItem -->" << endl;)
   m_textStream << "</div>" << endl;
}

void HtmlGenerator::startMemberDocName(bool /*align*/)
{
   DBG_HTML(m_textStream << "<!-- startMemberDocName -->" << endl;)

   m_textStream << "      <table class=\"memname\">" << endl;

   m_textStream << "        <tr>" << endl;
   m_textStream << "          <td class=\"memname\">";
}

void HtmlGenerator::endMemberDocName()
{
   DBG_HTML(m_textStream << "<!-- endMemberDocName -->" << endl;)
   m_textStream << "</td>" << endl;
}

void HtmlGenerator::startParameterList(bool openBracket)
{
   DBG_HTML(m_textStream << "<!-- startParameterList -->" << endl;)

   m_textStream << "          <td>";

   if (openBracket) {
      m_textStream << "(";
   }

   m_textStream << "</td>" << endl;
}

void HtmlGenerator::startParameterType(bool first, const QByteArray &key)
{
   if (first) {
      DBG_HTML(m_textStream << "<!-- startFirstParameterType -->" << endl;)
      m_textStream << "          <td class=\"paramtype\">";

   } else {
      DBG_HTML(m_textStream << "<!-- startParameterType -->" << endl;)
      m_textStream << "        <tr>" << endl;
      m_textStream << "          <td class=\"paramkey\">";

      if (! key.isEmpty()) {
         m_textStream << key;
      }

      m_textStream << "</td>" << endl;
      m_textStream << "          <td></td>" << endl;
      m_textStream << "          <td class=\"paramtype\">";
   }
}

void HtmlGenerator::endParameterType()
{
   DBG_HTML(m_textStream << "<!-- endParameterType -->" << endl;)
   m_textStream << "&#160;</td>" << endl;
}

void HtmlGenerator::startParameterName(bool /*oneArgOnly*/)
{
   DBG_HTML(m_textStream << "<!-- startParameterName -->" << endl;)
   m_textStream << "          <td class=\"paramname\">";
}

void HtmlGenerator::endParameterName(bool last, bool emptyList, bool closeBracket)
{
   DBG_HTML(m_textStream << "<!-- endParameterName -->" << endl;)

   if (last) {
      if (emptyList) {
         if (closeBracket) {
            m_textStream << "</td><td>)";
         }
         m_textStream << "</td>" << endl;
         m_textStream << "          <td>";

      } else {
         m_textStream << "&#160;</td>" << endl;
         m_textStream << "        </tr>" << endl;
         m_textStream << "        <tr>" << endl;
         m_textStream << "          <td></td>" << endl;
         m_textStream << "          <td>";
         if (closeBracket) {
            m_textStream << ")";
         }
         m_textStream << "</td>" << endl;
         m_textStream << "          <td></td><td>";
      }

   } else {
      m_textStream << "</td>" << endl;
      m_textStream << "        </tr>" << endl;
   }
}

void HtmlGenerator::endParameterList()
{
   DBG_HTML(m_textStream << "<!-- endParameterList -->" << endl;)

   m_textStream << "</td>" << endl;
   m_textStream << "        </tr>" << endl;
}

void HtmlGenerator::exceptionEntry(const QByteArray &prefix, bool closeBracket)
{
   DBG_HTML(m_textStream << "<!-- exceptionEntry -->" << endl;)
   m_textStream << "</td>" << endl;
   m_textStream << "        </tr>" << endl;
   m_textStream << "        <tr>" << endl;
   m_textStream << "          <td align=\"right\">";

   // colspan 2 so it gets both parameter type and parameter name columns

   if (! prefix.isEmpty()) {
      m_textStream << prefix << "</td><td>(</td><td colspan=\"2\">";

   } else if (closeBracket) {
      m_textStream << "</td><td>)</td><td></td><td>";

   } else {
      m_textStream << "</td><td></td><td colspan=\"2\">";
   }
}

void HtmlGenerator::endMemberDoc(bool hasArgs)
{
   DBG_HTML(m_textStream << "<!-- endMemberDoc -->" << endl;)

   if (!hasArgs) {
      m_textStream << "        </tr>" << endl;
   }

   m_textStream << "      </table>" << endl;
   // m_textStream << "</div>" << endl;
}

void HtmlGenerator::startDotGraph()
{
   startSectionHeader(m_textStream, m_relativePath, m_sectionCount);
}

void HtmlGenerator::endDotGraph(const DotClassGraph &g)
{
   bool generateLegend = Config::getBool("generate-legend");
   bool umlLook = Config::getBool("uml-look");

   endSectionHeader(m_textStream);
   startSectionSummary(m_textStream, m_sectionCount);
   endSectionSummary(m_textStream);
   startSectionContent(m_textStream, m_sectionCount);

   g.writeGraph(m_textStream, GOF_BITMAP, EOF_Html, m_dir, m_fileName, m_relativePath, true, true, m_sectionCount);

   if (generateLegend && ! umlLook) {
      m_textStream << "<center><span class=\"legend\">[";
      startHtmlLink(m_relativePath + "graph_legend" + Doxygen::htmlFileExtension);

      m_textStream << theTranslator->trLegend();
      endHtmlLink();

      m_textStream << "]</span></center>";
   }

   endSectionContent(m_textStream);
   m_sectionCount++;
}

void HtmlGenerator::startInclDepGraph()
{
   startSectionHeader(m_textStream, m_relativePath, m_sectionCount);
}

void HtmlGenerator::endInclDepGraph(const DotInclDepGraph &g)
{
   endSectionHeader(m_textStream);
   startSectionSummary(m_textStream, m_sectionCount);
   endSectionSummary(m_textStream);
   startSectionContent(m_textStream, m_sectionCount);

   g.writeGraph(m_textStream, GOF_BITMAP, EOF_Html, m_dir, m_fileName, m_relativePath, true, m_sectionCount);

   endSectionContent(m_textStream);
   m_sectionCount++;
}

void HtmlGenerator::startGroupCollaboration()
{
   startSectionHeader(m_textStream, m_relativePath, m_sectionCount);
}

void HtmlGenerator::endGroupCollaboration(const DotGroupCollaboration &g)
{
   endSectionHeader(m_textStream);
   startSectionSummary(m_textStream, m_sectionCount);
   endSectionSummary(m_textStream);
   startSectionContent(m_textStream, m_sectionCount);

   g.writeGraph(m_textStream, GOF_BITMAP, EOF_Html, m_dir, m_fileName, m_relativePath, true, m_sectionCount);

   endSectionContent(m_textStream);
   m_sectionCount++;
}

void HtmlGenerator::startCallGraph()
{
   startSectionHeader(m_textStream, m_relativePath, m_sectionCount);
}

void HtmlGenerator::endCallGraph(const DotCallGraph &g)
{
   endSectionHeader(m_textStream);
   startSectionSummary(m_textStream, m_sectionCount);
   endSectionSummary(m_textStream);
   startSectionContent(m_textStream, m_sectionCount);

   g.writeGraph(m_textStream, GOF_BITMAP, EOF_Html, m_dir, m_fileName, m_relativePath, true, m_sectionCount);

   endSectionContent(m_textStream);
   m_sectionCount++;
}

void HtmlGenerator::startDirDepGraph()
{
   startSectionHeader(m_textStream, m_relativePath, m_sectionCount);
}

void HtmlGenerator::endDirDepGraph(const DotDirDeps &g)
{
   endSectionHeader(m_textStream);
   startSectionSummary(m_textStream, m_sectionCount);
   endSectionSummary(m_textStream);
   startSectionContent(m_textStream, m_sectionCount);

   g.writeGraph(m_textStream, GOF_BITMAP, EOF_Html, m_dir, m_fileName, m_relativePath, true, m_sectionCount);

   endSectionContent(m_textStream);
   m_sectionCount++;
}

void HtmlGenerator::writeGraphicalHierarchy(const DotGfxHierarchyTable &g)
{
   g.writeGraph(m_textStream, m_dir, m_fileName);
}

void HtmlGenerator::startMemberGroupHeader(bool)
{
   m_textStream << "<tr><td colspan=\"2\"><div class=\"groupHeader\">";
}

void HtmlGenerator::endMemberGroupHeader()
{
   m_textStream << "</div></td></tr>" << endl;
}

void HtmlGenerator::startMemberGroupDocs()
{
   m_textStream << "<tr><td colspan=\"2\"><div class=\"groupText\">";
}

void HtmlGenerator::endMemberGroupDocs()
{
   m_textStream << "</div></td></tr>" << endl;
}

void HtmlGenerator::startMemberGroup()
{
}

void HtmlGenerator::endMemberGroup(bool)
{
}

void HtmlGenerator::startIndent()
{
   DBG_HTML(m_textStream << "<!-- startIndent -->" << endl;)

   m_textStream << "<div class=\"memdoc\">\n";
}

void HtmlGenerator::endIndent()
{
   DBG_HTML(m_textStream << "<!-- endIndent -->" << endl;)
   m_textStream << endl << "</div>" << endl << "</div>" << endl;
}

void HtmlGenerator::addIndexItem(const char *, const char *)
{
}

void HtmlGenerator::writeNonBreakableSpace(int n)
{ 
   for (int i = 0; i < n; i++) {
      m_textStream << "&#160;";
   }
}

void HtmlGenerator::startSimpleSect(SectionTypes, const QByteArray &filename, const char *anchor, const char *title)
{
   m_textStream << "<dl><dt><b>";

   if (! filename.isEmpty()) {
      writeObjectLink(0, filename, anchor, title);

   } else {
      docify(title);
   }

   m_textStream << "</b></dt>";
}

void HtmlGenerator::endSimpleSect()
{
   m_textStream << "</dl>";
}

void HtmlGenerator::startParamList(ParamListTypes, const char *title)
{
   m_textStream << "<dl><dt><b>";
   docify(title);

   m_textStream << "</b></dt>";
}

void HtmlGenerator::endParamList()
{
   m_textStream << "</dl>";
}

void HtmlGenerator::writeDoc(DocNode *n, QSharedPointer<Definition> ctx, QSharedPointer<MemberDef> md)
{
   assert(m_codeGen);

   HtmlDocVisitor *visitor = new HtmlDocVisitor(m_textStream, *m_codeGen, ctx);
   n->accept(visitor);

   delete visitor;
}

static void startQuickIndexList(QTextStream &t_stream, bool compact, bool topLevel = true)
{
   if (compact) {

      if (topLevel) {
         t_stream << "  <div id=\"navrow1\" class=\"tabs\">\n";

      } else {
         t_stream << "  <div id=\"navrow2\" class=\"tabs2\">\n";
      }

      t_stream << "    <ul class=\"tablist\">\n";

   } else {
      t_stream << "<ul>";
   }
}

static void endQuickIndexList(QTextStream &t_stream, bool compact)
{
   if (compact) {
      t_stream << "    </ul>\n";
      t_stream << "  </div>\n";

   } else {
      t_stream << "</ul>\n";
   }
}

static void startQuickIndexItem(QTextStream &t_stream, const QByteArray &l, bool hl, bool x, const QString &relPath)
{
   t_stream << "      <li";

   if (hl) {
      t_stream << " class=\"current\"";
   }
   t_stream << ">";

   if (! l.isEmpty()) {
      t_stream << "<a href=\"" << correctURL(l, relPath) << "\">";
   }
   t_stream << "<span>";
}

static void endQuickIndexItem(QTextStream &t_stream, const QByteArray &l)
{
   t_stream << "</span>";

   if (! l.isEmpty()) {
      t_stream << "</a>";
   }

   t_stream << "</li>\n";
}

static QByteArray fixSpaces(const QByteArray &s)
{
   return substitute(s, " ", "&#160;");
}

static bool quickLinkVisible(LayoutNavEntry::Kind kind)
{
   static bool showFiles      = Config::getBool("show-file-page");
   static bool showNamespaces = Config::getBool("show-namespace-page");

   switch (kind) {
      case LayoutNavEntry::MainPage:
         return true;
      case LayoutNavEntry::User:
         return true;
      case LayoutNavEntry::UserGroup:
         return true;
      case LayoutNavEntry::Pages:
         return indexedPages > 0;
      case LayoutNavEntry::Modules:
         return documentedGroups > 0;
      case LayoutNavEntry::Namespaces:
         return documentedNamespaces > 0 && showNamespaces;
      case LayoutNavEntry::NamespaceList:
         return documentedNamespaces > 0 && showNamespaces;
      case LayoutNavEntry::NamespaceMembers:
         return documentedNamespaceMembers[NMHL_All] > 0;
      case LayoutNavEntry::Classes:
         return annotatedClasses > 0;
      case LayoutNavEntry::ClassList:
         return annotatedClasses > 0;
      case LayoutNavEntry::ClassIndex:
         return annotatedClasses > 0;
      case LayoutNavEntry::ClassHierarchy:
         return hierarchyClasses > 0;
      case LayoutNavEntry::ClassMembers:
         return documentedClassMembers[CMHL_All] > 0;
      case LayoutNavEntry::Files:
         return documentedHtmlFiles > 0 && showFiles;
      case LayoutNavEntry::FileList:
         return documentedHtmlFiles > 0 && showFiles;
      case LayoutNavEntry::FileGlobals:
         return documentedFileMembers[FMHL_All] > 0;
      //case LayoutNavEntry::Dirs:             return documentedDirs>0;
      case LayoutNavEntry::Examples:
         return Doxygen::exampleSDict->count() > 0;
   }
   return false;
}

static void renderQuickLinksAsTree(QTextStream &t_stream, const QString &relPath, LayoutNavEntry *root)
{
   int count = 0;

   for (auto entry : root->children()) {   
      if (entry->visible() && quickLinkVisible(entry->kind())) {
         count++;
      }
   }

   if (count > 0) { // at least one item is visible
      startQuickIndexList(t_stream, false);
      
      for (auto entry : root->children()) {  
         if (entry->visible() && quickLinkVisible(entry->kind())) {
            QByteArray url = entry->url();

            t_stream << "<li><a href=\"" << relPath << url << "\"><span>";
            t_stream << fixSpaces(entry->title());
            t_stream << "</span></a>\n";

            // recursive into child list
            renderQuickLinksAsTree(t_stream, relPath, entry);
            t_stream << "</li>";
         }
      }

      endQuickIndexList(t_stream, false);
   }
}


static void renderQuickLinksAsTabs(QTextStream &t_stream, const QString &relPath, LayoutNavEntry *hlEntry, 
                  LayoutNavEntry::Kind kind, bool highlightParent, bool highlightSearch)
{
   if (hlEntry->parent()) { 
      // first draw the tabs for the parent of hlEntry
      renderQuickLinksAsTabs(t_stream, relPath, hlEntry->parent(), kind, highlightParent, highlightSearch);
   }

   if (hlEntry->parent() && hlEntry->parent()->children().count() > 0) { // draw tabs for row containing hlEntry
     bool topLevel = hlEntry->parent()->parent() == 0;
     int count = 0;    

      for (auto entry : hlEntry->parent()->children()) {  
         if (entry->visible() && quickLinkVisible(entry->kind())) {
            count++;
         }
      }

      if (count > 0) { 
         // at least one item is visible
         startQuickIndexList(t_stream, true, topLevel);

         for (auto entry : hlEntry->parent()->children()) {  
            if (entry->visible() && quickLinkVisible(entry->kind())) {
               QByteArray url = entry->url();

               startQuickIndexItem(t_stream, url, entry == hlEntry  &&
                                   (entry->children().count() > 0 || (entry->kind() == kind && !highlightParent) ), true, relPath);

               t_stream << fixSpaces(entry->title());
               endQuickIndexItem(t_stream, url);
            }
         }

         if (hlEntry->parent() == LayoutDocManager::instance().rootNavEntry()) { // first row is special as it contains the search box
            static bool searchEngine      = Config::getBool("html-search");
            static bool serverBasedSearch = Config::getBool("server-based-search");

            if (searchEngine) {
               t_stream << "      <li>\n";

               if (!serverBasedSearch) { // pure client side search
                  writeClientSearchBox(t_stream, relPath);
                  t_stream << "      </li>\n";

               } else { // server based search
                  writeServerSearchBox(t_stream, relPath, highlightSearch);
                  if (!highlightSearch) {
                     t_stream << "      </li>\n";
                  }
               }
            }

            if (!highlightSearch) // on the search page the index will be ended by the
               // page itself
            {
               endQuickIndexList(t_stream, true);
            }
         } else { // normal case for other rows than first one
            endQuickIndexList(t_stream, true);
         }
      }
   }
}

static void writeDefaultQuickLinks(QTextStream &t_stream, bool compact, HighlightedItem hli, 
                  const QString &file, const QString &relPath)
{
   LayoutNavEntry *root = LayoutDocManager::instance().rootNavEntry();
   LayoutNavEntry::Kind kind = (LayoutNavEntry::Kind) - 1;
   LayoutNavEntry::Kind altKind = (LayoutNavEntry::Kind) - 1; // fall back for the old layout file

   bool highlightParent = false;

   switch (hli) { 
      // map HLI enums to LayoutNavEntry::Kind enums

      case HLI_Main:
         kind = LayoutNavEntry::MainPage;
         break;

      case HLI_Modules:
         kind = LayoutNavEntry::Modules;
         break;
 
      case HLI_Namespaces:
         kind = LayoutNavEntry::NamespaceList;
         altKind = LayoutNavEntry::Namespaces;
         break;
      case HLI_Hierarchy:
         kind = LayoutNavEntry::ClassHierarchy;
         break;
      case HLI_Classes:
         kind = LayoutNavEntry::ClassIndex;
         altKind = LayoutNavEntry::Classes;
         break;
      case HLI_Annotated:
         kind = LayoutNavEntry::ClassList;
         altKind = LayoutNavEntry::Classes;
         break;
      case HLI_Files:
         kind = LayoutNavEntry::FileList;
         altKind = LayoutNavEntry::Files;
         break;
      case HLI_NamespaceMembers:
         kind = LayoutNavEntry::NamespaceMembers;
         break;
      case HLI_Functions:
         kind = LayoutNavEntry::ClassMembers;
         break;
      case HLI_Globals:
         kind = LayoutNavEntry::FileGlobals;
         break;
      case HLI_Pages:
         kind = LayoutNavEntry::Pages;
         break;
      case HLI_Examples:
         kind = LayoutNavEntry::Examples;
         break;
      case HLI_UserGroup:
         kind = LayoutNavEntry::UserGroup;
         break;
      case HLI_ClassVisible:
         kind = LayoutNavEntry::ClassList;
         altKind = LayoutNavEntry::Classes;
         highlightParent = true;
         break;
      case HLI_NamespaceVisible:
         kind = LayoutNavEntry::NamespaceList;
         altKind = LayoutNavEntry::Namespaces;
         highlightParent = true;
         break;
      case HLI_FileVisible:
         kind = LayoutNavEntry::FileList;
         altKind = LayoutNavEntry::Files;
         highlightParent = true;
         break;
      case HLI_None:
         break;
      case HLI_Search:
         break;
   }

   if (compact) {
      // find highlighted index item
      QString temp;

      if (kind == LayoutNavEntry::UserGroup) {
         temp = file;
      }

      LayoutNavEntry *hlEntry = root->find(kind, temp);

      if (! hlEntry && altKind != (LayoutNavEntry::Kind) - 1) {
         hlEntry = root->find(altKind);
         kind = altKind;
      }

      if (! hlEntry) { 
         // highlighted item not found in the index! -> just show the level 1 index...
         highlightParent = true;
         hlEntry = root->children().first();

         if (hlEntry == 0) {
            // argl, empty index
            return; 
         }
      }

      if (kind == LayoutNavEntry::UserGroup) {
         LayoutNavEntry *e = hlEntry->children().first();
         if (e) {
            hlEntry = e;
         }
      }

      renderQuickLinksAsTabs(t_stream, relPath, hlEntry, kind, highlightParent, hli == HLI_Search);

   } else {
      renderQuickLinksAsTree(t_stream, relPath, root);
   }
}

void HtmlGenerator::endQuickIndices()
{
   m_textStream << "</div><!-- top -->" << endl;
}

QByteArray HtmlGenerator::writeSplitBarAsString(const QString &name, const QString &relpath)
{
   static bool generateTreeView = Config::getBool("generate-treeview");
   QString result;

   // write split bar
   if (generateTreeView) {

      result =    "<div id=\"side-nav\" class=\"ui-resizable side-nav-resizable\">\n"
                  "  <div id=\"nav-tree\">\n"
                  "    <div id=\"nav-tree-contents\">\n"
                  "      <div id=\"nav-sync\" class=\"sync\"></div>\n"
                  "    </div>\n"
                  "  </div>\n"
                  "  <div id=\"splitbar\" style=\"-moz-user-select:none;\" \n"
                  "       class=\"ui-resizable-handle\">\n"
                  "  </div>\n"
                  "</div>\n"
                  "<script type=\"text/javascript\">\n"
                  "$(document).ready(function(){initNavTree('" + name + Doxygen::htmlFileExtension + "','" + relpath +
                  "');});\n</script>\n<div id=\"doc-content\">\n";
   }

   return result.toUtf8();
}

void HtmlGenerator::writeSplitBar(const QString &name)
{
   m_textStream << writeSplitBarAsString(name, m_relativePath);
}

void HtmlGenerator::writeNavigationPath(const char *s)
{
   m_textStream << substitute(s, "$relpath^", m_relativePath.toUtf8());
}

void HtmlGenerator::startContents()
{
   m_textStream << "<div class=\"contents\">" << endl;
}

void HtmlGenerator::endContents()
{
   m_textStream << "</div><!-- contents -->" << endl;
}

void HtmlGenerator::writeQuickLinks(bool compact, HighlightedItem hli, const QString &file)
{
   writeDefaultQuickLinks(m_textStream, compact, hli, file, m_relativePath);
}

// PHP based search script
void HtmlGenerator::writeSearchPage()
{
   static bool generateTreeView  = Config::getBool("generate-treeview");
   static bool disableIndex      = Config::getBool("disable-index");

   static QString projectName = Config::getString("project-name");
   static QString htmlOutput  = Config::getString("html-output");

   // OPENSEARCH_PROVIDER {
   QString configFileName = htmlOutput + "/search_config.php";
   QFile cf(configFileName);

   if (cf.open(QIODevice::WriteOnly)) {

      QTextStream t_stream(&cf);

      t_stream << "<script language=\"php\">\n\n";
      t_stream << "$config = array(\n";
      t_stream << "  'PROJECT_NAME' => \"" << convertToHtml(projectName) << "\",\n";
      t_stream << "  'GENERATE_TREEVIEW' => " << (generateTreeView ? "true" : "false") << ",\n";
      t_stream << "  'DISABLE_INDEX' => " << (disableIndex ? "true" : "false") << ",\n";
      t_stream << ");\n\n";
      t_stream << "$translator = array(\n";
      t_stream << "  'search_results_title' => \"" << theTranslator->trSearchResultsTitle() << "\",\n";
      t_stream << "  'search_results' => array(\n";
      t_stream << "    0 => \"" << theTranslator->trSearchResults(0) << "\",\n";
      t_stream << "    1 => \"" << theTranslator->trSearchResults(1) << "\",\n";
      t_stream << "    2 => \"" << substitute(theTranslator->trSearchResults(2), "$", "\\$") << "\",\n";
      t_stream << "  ),\n";
      t_stream << "  'search_matches' => \"" << theTranslator->trSearchMatches() << "\",\n";
      t_stream << "  'search' => \"" << theTranslator->trSearch() << "\",\n";
      t_stream << "  'split_bar' => \"" << substitute(substitute(writeSplitBarAsString("search", ""), "\"", "\\\""), "\n", "\\n") << "\",\n";
      t_stream << "  'logo' => \"" << substitute(substitute(writeLogoAsString(""), "\"", "\\\""), "\n", "\\n") << "\",\n";
      t_stream << ");\n\n";
      t_stream << "</script>\n";

   } else {
      err("Unable to open file for writing %s, error: %d\n", qPrintable(configFileName), cf.error()); 

   }

   ResourceMgr::instance().copyResourceAs("html/search_functions.php",  htmlOutput, "search_functions.php");
   ResourceMgr::instance().copyResourceAs("html/search_opensearch.php", htmlOutput, "search_opensearch.php");

   QString fileName = htmlOutput + "/search.php";
   QFile f(fileName);

   if (f.open(QIODevice::WriteOnly)) {
      QTextStream t_stream(&f);

      t_stream << substituteHtmlKeywords(g_header, "Search");

      t_stream << "<!-- " << theTranslator->trGeneratedBy() << " DoxyPress "
        << versionString << " -->" << endl;
      t_stream << "<script type=\"text/javascript\">\n";
      t_stream << "var searchBox = new SearchBox(\"searchBox\", \""
        << "search\",false,'" << theTranslator->trSearch() << "');\n";
      t_stream << "</script>\n";

      if (! Config::getBool("disable-index")) {
         writeDefaultQuickLinks(t_stream, true, HLI_Search, QString(), QString());

      } else {
         t_stream << "</div>" << endl;
      }

      t_stream << "<script language=\"php\">\n";
      t_stream << "require_once \"search_functions.php\";\n";
      t_stream << "main();\n";
      t_stream << "</script>\n";

      // Write empty navigation path, to make footer connect properly
      if (generateTreeView) {
         t_stream << "</div><!-- doc-contents -->\n";        
      }

      writePageFooter(t_stream, "Search", QString(), QString());
   }

   QString scriptName = htmlOutput + "/search/search.js";
   QFile sf(scriptName);

   if (sf.open(QIODevice::WriteOnly)) {
      QTextStream t_stream(&sf);
      t_stream << ResourceMgr::instance().getAsString("html/extsearch.js");

   } else {
      err("Unable to open file for writing %s, error: %d\n", scriptName.constData(), f.error());
   }
}

void HtmlGenerator::writeExternalSearchPage()
{
   static bool generateTreeView = Config::getBool("generate-treeview");
   QString fileName = Config::getString("html-output") + "/search" + Doxygen::htmlFileExtension;

   QFile f(fileName);

   if (f.open(QIODevice::WriteOnly)) {
      QTextStream t_stream(&f);

      t_stream << substituteHtmlKeywords(g_header, "Search");

      t_stream << "<!-- " << theTranslator->trGeneratedBy() << " DoxyPress "
        << versionString << " -->" << endl;
      t_stream << "<script type=\"text/javascript\">\n";
      t_stream << "var searchBox = new SearchBox(\"searchBox\", \""
        << "search\",false,'" << theTranslator->trSearch() << "');\n";
      t_stream << "</script>\n";

      if (! Config::getBool("disable-index")) {
         writeDefaultQuickLinks(t_stream, true, HLI_Search, QString(), QString());

         t_stream << "            <input type=\"text\" id=\"MSearchField\" name=\"query\" value=\"\" size=\"20\" accesskey=\"S\""
                "onfocus=\"searchBox.OnSearchFieldFocus(true)\" onblur=\"searchBox.OnSearchFieldFocus(false)\"/>\n";

         t_stream << "            </form>\n";
         t_stream << "          </div><div class=\"right\"></div>\n";
         t_stream << "        </div>\n";
         t_stream << "      </li>\n";
         t_stream << "    </ul>\n";
         t_stream << "  </div>\n";
         t_stream << "</div>\n";

      } else {
         t_stream << "</div>" << endl;
      }

      t_stream << writeSplitBarAsString("search", QString());
      t_stream << "<div class=\"header\">" << endl;
      t_stream << "  <div class=\"headertitle\">" << endl;
      t_stream << "    <div class=\"title\">" << theTranslator->trSearchResultsTitle() << "</div>" << endl;
      t_stream << "  </div>" << endl;
      t_stream << "</div>" << endl;
      t_stream << "<div class=\"contents\">" << endl;

      t_stream << "<div id=\"searchresults\"></div>" << endl;
      t_stream << "</div>" << endl;

      if (generateTreeView) {
         t_stream << "</div><!-- doc-contents -->" << endl;
      }

      writePageFooter(t_stream, "Search", QString(), QString());

   } else {
      err("Unable to open file for writing %s, error: %d\n", qPrintable(fileName), f.error());

   }

   QString scriptName = Config::getString("html-output") + "/search/search.js";
   QFile sf(scriptName);

   if (sf.open(QIODevice::WriteOnly)) {
      QTextStream t_stream(&sf);

      t_stream << "var searchResultsText=["
        << "\"" << theTranslator->trSearchResults(0) << "\","
        << "\"" << theTranslator->trSearchResults(1) << "\","
        << "\"" << theTranslator->trSearchResults(2) << "\"];" << endl;

      t_stream << "var serverUrl=\"" << Config::getString("searchengine-url") << "\";" << endl;
      t_stream << "var tagMap = {" << endl;

      bool first = true;

      // add search mappings
      const QStringList searchMappings = Config::getList("search-mappings");
     
      for (auto mapLine : searchMappings) { 
        
         int eqPos = mapLine.indexOf('=');

         if (eqPos != -1) { 
            // tag command contains a destination

            QString tagName  = mapLine.left(eqPos).trimmed();
            QString destName = mapLine.right(mapLine.length() - eqPos - 1).trimmed();

            if (! tagName.isEmpty()) {
               if (!first) {
                  t_stream << "," << endl;
               }

               t_stream << "  \"" << tagName << "\": \"" << destName << "\"";
               first = false;
            }
         }
        
      }

      if (! first) {
         t_stream << endl;
      }

      t_stream << "};" << endl << endl;
      t_stream << ResourceMgr::instance().getAsString("html/extsearch.js");
      t_stream << endl;
      t_stream << "$(document).ready(function() {" << endl;
      t_stream << "  var query = trim(getURLParameter('query'));" << endl;
      t_stream << "  if (query) {" << endl;
      t_stream << "    searchFor(query,0,20);" << endl;
      t_stream << "  } else {" << endl;
      t_stream << "    var results = $('#results');" << endl;
      t_stream << "    results.html('<p>" << theTranslator->trSearchResults(0) << "</p>');" << endl;
      t_stream << "  }" << endl;
      t_stream << "});" << endl;

   } else {
      err("Unable to open file for writing %s, error: %d\n", scriptName.constData(), sf.error());
   }
}

void HtmlGenerator::startConstraintList(const char *header)
{
   m_textStream << "<div class=\"typeconstraint\">" << endl;
   m_textStream << "<dl><dt><b>" << header << "</b></dt><dd>" << endl;
   m_textStream << "<table border=\"0\" cellspacing=\"2\" cellpadding=\"0\">" << endl;
}

void HtmlGenerator::startConstraintParam()
{
   m_textStream << "<tr><td valign=\"top\"><em>";
}

void HtmlGenerator::endConstraintParam()
{
   m_textStream << "</em></td>";
}

void HtmlGenerator::startConstraintType()
{
   m_textStream << "<td>&#160;:</td><td valign=\"top\"><em>";
}

void HtmlGenerator::endConstraintType()
{
   m_textStream << "</em></td>";
}

void HtmlGenerator::startConstraintDocs()
{
   m_textStream << "<td>&#160;";
}

void HtmlGenerator::endConstraintDocs()
{
   m_textStream << "</td></tr>" << endl;
}

void HtmlGenerator::endConstraintList()
{
   m_textStream << "</table>" << endl;
   m_textStream << "</dl>" << endl;
   m_textStream << "</div>" << endl;
}

void HtmlGenerator::lineBreak(const QByteArray &style)
{
   if (! style.isEmpty()) {
      m_textStream << "<br class=\"" << style << "\" />" << endl;

   } else {
      m_textStream << "<br />" << endl;
   }
}

void HtmlGenerator::startHeaderSection()
{
   m_textStream << "<div class=\"header\">" << endl;
}

void HtmlGenerator::startTitleHead(const char *)
{
   m_textStream << "  <div class=\"headertitle\">" << endl;
   startTitle();
}

void HtmlGenerator::endTitleHead(const char *, const char *)
{
   endTitle();
   m_textStream << "  </div>" << endl;
}

void HtmlGenerator::endHeaderSection()
{
   m_textStream << "</div><!--header-->" << endl;
}

void HtmlGenerator::startInlineHeader()
{
   if (m_emptySection) {
      m_textStream << "<table class=\"memberdecls\">" << endl;
      m_emptySection = false;
   }

   m_textStream << "<tr><td colspan=\"2\"><h3>";
}

void HtmlGenerator::endInlineHeader()
{
   m_textStream << "</h3></td></tr>" << endl;
}

void HtmlGenerator::startMemberDocSimple()
{
   DBG_HTML(m_textStream << "<!-- startMemberDocSimple -->" << endl;)
   m_textStream << "<table class=\"fieldtable\">" << endl;
   m_textStream << "<tr><th colspan=\"3\">" << theTranslator->trCompoundMembers() << "</th></tr>" << endl;
}

void HtmlGenerator::endMemberDocSimple()
{
   DBG_HTML(m_textStream << "<!-- endMemberDocSimple -->" << endl;)
   m_textStream << "</table>" << endl;
}

void HtmlGenerator::startInlineMemberType()
{
   DBG_HTML(m_textStream << "<!-- startInlineMemberType -->" << endl;)
   m_textStream << "<tr><td class=\"fieldtype\">" << endl;
}

void HtmlGenerator::endInlineMemberType()
{
   DBG_HTML(m_textStream << "<!-- endInlineMemberType -->" << endl;)
   m_textStream << "</td>" << endl;
}

void HtmlGenerator::startInlineMemberName()
{
   DBG_HTML(m_textStream << "<!-- startInlineMemberName -->" << endl;)
   m_textStream << "<td class=\"fieldname\">" << endl;
}

void HtmlGenerator::endInlineMemberName()
{
   DBG_HTML(m_textStream << "<!-- endInlineMemberName -->" << endl;)
   m_textStream << "</td>" << endl;
}

void HtmlGenerator::startInlineMemberDoc()
{
   DBG_HTML(m_textStream << "<!-- startInlineMemberDoc -->" << endl;)
   m_textStream << "<td class=\"fielddoc\">" << endl;
}

void HtmlGenerator::endInlineMemberDoc()
{
   DBG_HTML(m_textStream << "<!-- endInlineMemberDoc -->" << endl;)
   m_textStream << "</td></tr>" << endl;
}

void HtmlGenerator::startLabels()
{
   DBG_HTML(m_textStream << "<!-- startLabels -->" << endl;)
   m_textStream << "<span class=\"mlabels\">";
}

void HtmlGenerator::writeLabel(const char *l, bool /*isLast*/)
{
   DBG_HTML(m_textStream << "<!-- writeLabel(" << l << ") -->" << endl;) 
   m_textStream << "<span class=\"mlabel\">" << l << "</span>";
}

void HtmlGenerator::endLabels()
{
   DBG_HTML(m_textStream << "<!-- endLabels -->" << endl;)
   m_textStream << "</span>";
}

void HtmlGenerator::writeInheritedSectionTitle(const char *id, const QByteArray &ref, const char *file, const char *anchor, 
                                               const char *title, const char *name)
{
   DBG_HTML(m_textStream << "<!-- writeInheritedSectionTitle -->" << endl;)
   QByteArray a = anchor;

   if (! a.isEmpty()) {
      a.prepend("#");
   }

   QByteArray classLink = QByteArray("<a class=\"el\" href=\"");

   if (! ref.isEmpty()) {
      classLink += externalLinkTarget() + externalRef(m_relativePath, ref, true);
   } else {
      classLink += m_relativePath;
   }

   classLink += file + Doxygen::htmlFileExtension.toUtf8() + a;
   classLink += QByteArray("\">") + convertToHtml(name, false) + "</a>";

   m_textStream << "<tr class=\"inherit_header " << id << "\">"
     << "<td colspan=\"2\" onclick=\"javascript:toggleInherit('" << id << "')\">"
     << "<img src=\"" << m_relativePath << "closed.png\" alt=\"-\"/>&#160;"
     << theTranslator->trInheritedFrom(convertToHtml(title, false), classLink)
     << "</td></tr>" << endl;
}

void HtmlGenerator::writeSummaryLink(const QByteArray &file, const char *anchor, const char *title, bool first)
{
   if (first) {
      m_textStream << "  <div class=\"summary\">\n";

   } else {
      m_textStream << " &#124;\n";
   }

   m_textStream << "<a href=\"";

   if (! file.isEmpty()) {
      m_textStream << m_relativePath << file;
      m_textStream << Doxygen::htmlFileExtension;

   } else {
      m_textStream << "#";
      m_textStream << anchor;
   }

   m_textStream << "\">";
   m_textStream << title;
   m_textStream << "</a>";
}

void HtmlGenerator::endMemberDeclaration(const char *anchor, const  QByteArray &inheritId)
{
   m_textStream << "<tr class=\"separator:" << anchor;

   if (! inheritId.isEmpty() ) {
      m_textStream << " inherit " << inheritId;
   }

   m_textStream << "\"><td class=\"memSeparator\" colspan=\"2\">&#160;</td></tr>\n";
}

void HtmlGenerator::setCurrentDoc(QSharedPointer<Definition> context, const char *anchor, bool isSourceFile)
{
   if (Doxygen::searchIndex) {
      Doxygen::searchIndex->setCurrentDoc(context, anchor, isSourceFile);
   }
}

void HtmlGenerator::addWord(const QString &word, bool hiPriority)
{
   if (Doxygen::searchIndex) {
      Doxygen::searchIndex->addWord(word, hiPriority);
   }
}

