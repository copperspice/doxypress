/************************************************************************
*
* Copyright (c) 2014-2022 Barbara Geller & Ansel Sermersheim
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

#include <QDir>
#include <QRegularExpression>

#include <htmlgen.h>

#include <config.h>
#include <docparser.h>
#include <diagram.h>
#include <dot.h>
#include <doxy_build_info.h>
#include <doxy_globals.h>
#include <ftvhelp.h>
#include <htmldocvisitor.h>
#include <htmlhelp.h>
#include <image.h>
#include <layout.h>
#include <logos.h>
#include <language.h>
#include <message.h>
#include <resourcemgr.h>
#include <util.h>

#include <stdlib.h>

//#define DBG_HTML(x) x;
#define DBG_HTML(x)

static QString g_header;
static QString g_footer;
static QString g_mathjax_code;

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
   static const bool externalSearch = Config::getBool("search-external");

   t << "        <div id=\"MSearchBox\" class=\"MSearchBoxInactive\">\n";
   t << "          <div class=\"left\">\n";
   t << "            <form id=\"FSearchBox\" action=\"" << relPath;

   if (externalSearch) {
      t << "search" << Doxy_Globals::htmlFileExtension;
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

      if (beginPos == -1) {
         break;
      }

      int endPos = retval.indexOf(end, beginPos);

      if (endPos == -1) {
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

static QString getSearchBox(bool serverSide, const QString &relPath, bool highlightSearch)
{
   QString result;
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

static QString substituteHtmlKeywords(const QString &output, const QString &title,
                  const QString &relPath = QString(), const QString &navPath = QString())
{
   // Build CSS/Javascript tags depending on treeview, search engine settings

   static const QDir configDir   = Config::getConfigDir();

   static QString projectName    = Config::getString("project-name");
   static QString projectVersion = Config::getString("project-version");
   static QString projectBrief   = Config::getString("project-brief");
   static QString projectLogo    = Config::getString("project-logo");

   static bool timeStamp         = Config::getBool("html-timestamp");
   static bool treeView          = Config::getBool("generate-treeview");
   static bool searchEngine      = Config::getBool("html-search");
   static bool serverBasedSearch = Config::getBool("search-server-based");

   static bool mathJax           = Config::getBool("use-mathjax");
   static QString mathJaxFormat  = Config::getEnum("mathjax-format");
   static bool disableIndex      = Config::getBool("disable-index");

   static const QStringList extraCssFile      = Config::getList("html-stylesheets");
   static const QStringList mathJaxExtensions = Config::getList("mathjax-extensions");

   static bool hasProjectName    = ! projectName.isEmpty();
   static bool hasProjectVersion = ! projectVersion.isEmpty();
   static bool hasProjectBrief   = ! projectBrief.isEmpty();
   static bool hasProjectLogo    = ! projectLogo.isEmpty();

   static bool titleArea = (hasProjectName || hasProjectBrief || hasProjectLogo || (disableIndex && searchEngine));

   QString generatedBy;
   QString treeViewCssJs;
   QString searchCssJs;
   QString searchBox;
   QString mathJaxJs;

   // always first
   QString cssFile      = "doxypress.css";
   QString extraCssText;

   for (auto fileName : extraCssFile) {

      if (! fileName.isEmpty()) {
         QFileInfo fi(configDir, fileName);

         if (fi.exists()) {
            extraCssText += "<link href=\"$relpath^" + stripPath(fileName) +
                  "\" rel=\"stylesheet\" type=\"text/css\"/>\n";
         } else {
            err("Unable to find stylesheet '%s'\n", csPrintable(fi.absoluteFilePath()));
         }
      }
   }

   if (timeStamp) {
      generatedBy = theTranslator->trGeneratedAt(dateToString(true), convertToHtml(projectName));

   } else {
      generatedBy = theTranslator->trGeneratedBy();
   }

   if (treeView) {
      treeViewCssJs = "<link href=\"$relpath^navtree.css\" rel=\"stylesheet\" type=\"text/css\"/>\n"
                      "<script type=\"text/javascript\" src=\"$relpath^resize.js\"></script>\n"
                      "<script type=\"text/javascript\" src=\"$relpath^navtreedata.js\"></script>\n"
                      "<script type=\"text/javascript\" src=\"$relpath^navtree.js\"></script>\n";
   }

   if (searchEngine) {
      searchCssJs = "<link href=\"$relpath^search/search.css\" rel=\"stylesheet\" type=\"text/css\"/>\n";

      if (! serverBasedSearch) {
         searchCssJs += "<script type=\"text/javascript\" src=\"$relpath^search/searchdata.js\"></script>\n";
      }
      searchCssJs += "<script type=\"text/javascript\" src=\"$relpath^search/search.js\"></script>\n";


      if (! serverBasedSearch) {

         if (disableIndex) {

            searchCssJs += "<script type=\"text/javascript\">\n"
                           "  $(document).ready(function() { init_search(); });\n"
                           "</script>";
         }

      } else {

         if (disableIndex) {

            searchCssJs += "<script type=\"text/javascript\">\n"
                           "  $(document).ready(function() {\n"
                           "    if ($('.searchresults').length > 0) { searchBox.DOMSearchField().focus(); }\n"
                           "  });\n"
                           "</script>\n";
         }

         // OPENSEARCH_PROVIDER
         searchCssJs += "<link rel=\"search\" href=\"" + relPath +
                        "search_opensearch.php?v=opensearch.xml\" "
                        "type=\"application/opensearchdescription+xml\" title=\"" +
                        (hasProjectName ? projectName : "DoxyPress") + "\"/>";
      }
      searchBox = getSearchBox(serverBasedSearch, relPath, false);
   }

   if (mathJax) {
      QString path = Config::getString("mathjax-relpath");

      if (! path.isEmpty() && path.at(path.length() - 1) != '/') {
         path += "/";
      }

      if (path.isEmpty() || path.startsWith("..")) {
         // relative path
         path.prepend(relPath);
      }

      mathJaxJs = "<script type=\"text/x-mathjax-config\">\n"
                  "  MathJax.Hub.Config({\n"
                  "    extensions: [\"tex2jax.js\"";

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
      mathJaxJs += "<script type=\"text/javascript\" async=\"async\" src=\"" + path + "MathJax.js\"></script>\n";
   }

   QString result = output;

   // first substitute generic keywords
   if (! title.isEmpty()) {
      result = result.replace("$title", convertToHtml(title));
   }

   result = result.replace("$datetimeHHMM",     dateTimeHHMM());
   result = result.replace("$datetime",         dateToString(true));
   result = result.replace("$date",             dateToString(false));
   result = result.replace("$year",             yearToString());

   result = result.replace("$doxypressversion", versionString);
   result = result.replace("$doxygenversion",   versionString);      // compatibility

   result = result.replace("$projectname",      convertToHtml(projectName));
   result = result.replace("$projectversion",   convertToHtml(projectVersion));
   result = result.replace("$projectbrief",     convertToHtml(projectBrief));
   result = result.replace("$projectlogo",      stripPath(projectLogo));

   // additional HTML only keywords
   result = result.replace("$navpath",          navPath);
   result = result.replace("$stylesheet",       cssFile);
   result = result.replace("$extrastylesheet",  extraCssText);
   result = result.replace("$treeview",         treeViewCssJs);
   result = result.replace("$searchbox",        searchBox);
   result = result.replace("$search",           searchCssJs);
   result = result.replace("$mathjax",          mathJaxJs);
   result = result.replace("$generatedby",      generatedBy);

   result = result.replace("$relpath$",         relPath);  // backwards compatibility only
   result = result.replace("$relpath^",         relPath);  // keep last

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

HtmlCodeGenerator::HtmlCodeGenerator(QTextStream &t, const QString &relPath)
   : m_streamCode(t), m_streamSet(false), m_lineOpen(false), m_col(0), m_relPath(relPath)
{
}

void HtmlCodeGenerator::codify(const QString &str)
{
   if (str.isEmpty()) {
      return;
   }

   static const int tabSize = Config::getInt("tab-size");

   int spacesToNextTabStop;
   bool isBackSlash = false;

   for (auto c : str) {

      if (isBackSlash) {
         isBackSlash = false;

         m_streamCode << "\\";
         m_col++;
      }

      switch (c.unicode()) {

         case '\t':
            spacesToNextTabStop = tabSize - (m_col % tabSize);
            m_streamCode << QString(spacesToNextTabStop, ' ');

            m_col += spacesToNextTabStop;
            break;

         case '\n':
            m_streamCode << "\n";
            m_col = 0;
            break;

         case '\r':
            break;

         case '<':
            m_streamCode << "&lt;";
            m_col++;
            break;

         case '>':
            m_streamCode << "&gt;";
            m_col++;
            break;

         case '&':
            m_streamCode << "&amp;";
            m_col++;
            break;

         case '"':
            m_streamCode << "&quot;";
            m_col++;
            break;

         case '\'':
            m_streamCode << "&#39;";
            m_col++;                // &apos; is not valid XHTML
            break;

         case '\\':
            isBackSlash = true;
            break;

         default:
            m_streamCode << c;
            m_col++;
            break;
      }

   }
}

void HtmlCodeGenerator::docify(const QString &text)
{
   bool isBackSlash = false;

   for (auto c : text) {

      switch (c.unicode()) {
         case '<':
            m_streamCode << "&lt;";
            break;

         case '>':
            m_streamCode << "&gt;";
            break;

         case '&':
            m_streamCode << "&amp;";
            break;

         case '"':
            m_streamCode << "&quot;";
            break;

         case '\\':

            if (isBackSlash) {
              isBackSlash = false;

              m_streamCode << "\\";

            } else {
              isBackSlash = true;

            }

            break;

         default:
            m_streamCode << c;
      }

      if (isBackSlash && c != '\\') {
         isBackSlash = false;
      }
   }
}

void HtmlCodeGenerator::writeLineNumber(const QString &ref, const QString &filename, const QString &anchor, int len)
{
   const QString lineNumber = QString("%1").formatArg(len, 5, 10);
   const QString lineAnchor = QString("l%1").formatArg(len, 5, 10, QChar('0'));

   if (! m_lineOpen) {
      m_streamCode << "<div class=\"line\">";
      m_lineOpen = true;
   }
   m_streamCode << "<a name=\"" << lineAnchor << "\"></a><span class=\"lineno\">";

   if (! filename.isEmpty()) {
      writeCodeLinkHtml("line", ref, filename, anchor, lineNumber, "");
   } else {
      codify(lineNumber);
   }

   m_streamCode << "</span>";
   m_streamCode << "&#160;";

   m_col = 0;
}

void HtmlCodeGenerator::writeCodeLink(const QString &ref, const QString &f, const QString &anchor,
                  const QString &name, const QString &tooltip)
{
   writeCodeLinkHtml("code", ref, f, anchor, name, tooltip);
}

void HtmlCodeGenerator::writeCodeLinkHtml(const QString &className, const QString &ref, const QString &f,
                  const QString &anchor, const QString &name, const QString &tooltip)
{
   if (! ref.isEmpty()) {
      m_streamCode << "<a class=\"" << className << "Ref\" ";
      m_streamCode << externalLinkTarget() << externalRef(m_relPath, ref, false);

   } else {
      m_streamCode << "<a class=\"" << className << "\" ";
   }

   m_streamCode << "href=\"";
   m_streamCode << externalRef(m_relPath, ref, true);

   if (! f.isEmpty()) {
      m_streamCode << f << Doxy_Globals::htmlFileExtension;
   }

   if (! anchor.isEmpty()) {
      m_streamCode << "#" << anchor;
   }

   m_streamCode << "\"";

   if (! tooltip.isEmpty()) {
      m_streamCode << " title=\"" << convertToHtml(tooltip) << "\"";
   }

   m_streamCode << ">";
   docify(name);

   m_streamCode << "</a>";
   m_col += name.length();
}

void HtmlCodeGenerator::writeTooltip(const QString &id, const DocLinkInfo &docInfo, const QString &decl,
                  const QString &desc, const SourceLinkInfo &defInfo, const SourceLinkInfo &declInfo)
{
   m_streamCode << "<div class=\"ttc\" id=\"" << id << "\">";
   m_streamCode << "<div class=\"ttname\">";

   if (! docInfo.url.isEmpty()) {
      m_streamCode << "<a href=\"";
      m_streamCode << externalRef(m_relPath, docInfo.ref, true);
      m_streamCode << docInfo.url << Doxy_Globals::htmlFileExtension;

      if (! docInfo.anchor.isEmpty()) {
         m_streamCode << "#" << docInfo.anchor;
      }

      m_streamCode << "\">";
   }

   docify(docInfo.name);

   if (! docInfo.url.isEmpty()) {
      m_streamCode << "</a>";
   }

   m_streamCode << "</div>";

   if (! decl.isEmpty()) {
      m_streamCode << "<div class=\"ttdeci\">";
      docify(decl);
      m_streamCode << "</div>";
   }

   if (! desc.isEmpty()) {
      m_streamCode << "<div class=\"ttdoc\">";

      // desc is already HTML escaped but there are still < and > signs
      docify(desc);

      m_streamCode << "</div>";
   }

   if (! defInfo.file.isEmpty()) {
      m_streamCode << "<div class=\"ttdef\"><b>Definition:</b> ";

      if (! defInfo.url.isEmpty()) {
         m_streamCode << "<a href=\"";
         m_streamCode << externalRef(m_relPath, defInfo.ref, true);
         m_streamCode << defInfo.url << Doxy_Globals::htmlFileExtension;
         if (!defInfo.anchor.isEmpty()) {
            m_streamCode << "#" << defInfo.anchor;
         }
         m_streamCode << "\">";
      }

      m_streamCode << defInfo.file << ":" << defInfo.line;
      if (!defInfo.url.isEmpty()) {
         m_streamCode << "</a>";
      }
      m_streamCode << "</div>";
   }

   if (! declInfo.file.isEmpty()) {
      m_streamCode << "<div class=\"ttdecl\"><b>Declaration:</b> ";
      if (!declInfo.url.isEmpty()) {
         m_streamCode << "<a href=\"";
         m_streamCode << externalRef(m_relPath, declInfo.ref, true);
         m_streamCode << declInfo.url << Doxy_Globals::htmlFileExtension;
         if (!declInfo.anchor.isEmpty()) {
            m_streamCode << "#" << declInfo.anchor;
         }
         m_streamCode << "\">";
      }
      m_streamCode << declInfo.file << ":" << declInfo.line;
      if (!declInfo.url.isEmpty()) {
         m_streamCode << "</a>";
      }
      m_streamCode << "</div>";
   }

   m_streamCode << "</div>" << endl;
}

void HtmlCodeGenerator::startCodeLine(bool)
{
   m_col = 0;

   if (! m_lineOpen) {
      m_streamCode << "<div class=\"line\">";
      m_lineOpen = true;
   }
}

void HtmlCodeGenerator::endCodeLine()
{
   if (m_col == 0) {
      m_streamCode << " ";
      ++m_col;
   }
   if (m_lineOpen) {
      m_streamCode << "</div>\n";
      m_lineOpen = false;
   }
}

void HtmlCodeGenerator::startFontClass(const QString &s)
{
   m_streamCode << "<span class=\"" << s << "\">";
}

void HtmlCodeGenerator::endFontClass()
{
   m_streamCode << "</span>";
}

void HtmlCodeGenerator::writeCodeAnchor(const QString &anchor)
{
   m_streamCode << "<a name=\"" << anchor << "\"></a>";
}

void HtmlCodeGenerator::startCodeFragment(const QString &)
{

   m_streamCode << "<div class=\"fragment\">";
}

void HtmlCodeGenerator::endCodeFragment(const QString &)
{
   // endCodeLine checks if an open code line exists, if so closes it
   endCodeLine();

   m_streamCode << "</div><!-- end fragment -->\n";
}

HtmlGenerator::HtmlGenerator() : OutputGenerator()
{
   m_outputDir    = Config::getString("html-output");
   m_emptySection = false;
   m_sectionCount = 0;
}

void HtmlGenerator::init()
{
   static const bool sourceCode       = Config::getBool("source-code");
   static const bool toolTips         = Config::getBool("source-tooltips");

   static const QString htmlDirName   = Config::getString("html-output");
   static const QString htmlHeader    = Config::getString("html-header");
   static const QString htmlFooter    = Config::getString("html-footer");
   static const bool interactiveSvg   = Config::getBool("interactive-svg");

   static bool useMathJax  = Config::getBool("use-mathjax");

   QDir d(htmlDirName);

   if (! d.exists() && ! d.mkdir(htmlDirName)) {
      err("HTML Generator, unable to create output directory %s\n", csPrintable(htmlDirName));
      Doxy_Work::stopDoxyPress();
   }

   if (! htmlHeader.isEmpty()) {
      g_header = fileToString(htmlHeader);
   } else {
      QByteArray data = ResourceMgr::instance().getAsString("html/header.html");
      g_header = QString::fromUtf8(data);
   }

   if (! htmlFooter.isEmpty()) {
      g_footer = fileToString(htmlFooter);
   } else {
      QByteArray data = ResourceMgr::instance().getAsString("html/footer.html");
      g_footer = QString::fromUtf8(data);
   }

   if (useMathJax) {
      static const QString mathJaxCodeFile = Config::getString("mathjax-codefile");

      if (! mathJaxCodeFile.isEmpty()) {
         g_mathjax_code = fileToString(mathJaxCodeFile);
      }
   }
   createSubDirs(d);

   //
   ResourceMgr &mgr = ResourceMgr::instance();

   mgr.copyResourceAs("html/tabs.css", htmlDirName, "tabs.css");
   mgr.copyResourceAs("html/jquery.js", htmlDirName, "jquery.js");

   if (interactiveSvg) {
      mgr.copyResourceAs("html/svgpan.js", htmlDirName, "svgpan.js");
   }

   QString fileName = htmlDirName + "/dynsections.js";
   QFile f(fileName);

   if (f.open(QIODevice::WriteOnly)) {
      QByteArray resource = mgr.getAsString("html/dynsections.js");

      if (! resource.isEmpty()) {
         QTextStream t(&f);
         t << resource;

         if (sourceCode && toolTips) {
            t << endl <<
              "$(document).ready(function() {\n"
              "  $('.code,.codeRef').each(function() {\n"
              "    $(this).data('powertip',$('#a'+$(this).attr('href').replace(/.*\\//,'').replace(/[^a-z_A-Z0-9]/g,'_')).html());\n"
              "    $(this).powerTip({ placement: 's', smartPlacement: true, mouseOnToPopup: true });\n"
              "  });\n"
              "});\n";
         }

      } else {
         err("Unable to find or load the resource file dynsections.js");

      }

   } else {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), f.error());

   }

}

/// Additional initialization after indices have been created
void HtmlGenerator::writeTabData()
{
   Doxy_Globals::indexList.addStyleSheetFile("tabs.css");
   static const QString htmlDirName = Config::getString("html-output");

   ResourceMgr &mgr = ResourceMgr::instance();

   // writeColoredImgData(htmlDirName,colored_tab_data);

   mgr.copyResourceAs("html/tab_a.lum",      htmlDirName, "tab_a.png");
   mgr.copyResourceAs("html/tab_b.lum",      htmlDirName, "tab_b.png");
   mgr.copyResourceAs("html/tab_h.lum",      htmlDirName, "tab_h.png");
   mgr.copyResourceAs("html/tab_s.lum",      htmlDirName, "tab_s.png");
   mgr.copyResourceAs("html/nav_h.lum",      htmlDirName, "nav_h.png");
   mgr.copyResourceAs("html/nav_f.lum",      htmlDirName, "nav_f.png");
   mgr.copyResourceAs("html/bc_s.luma",      htmlDirName, "bc_s.png");
   mgr.copyResourceAs("html/doxypress.luma", htmlDirName, "doxypress.png");
   mgr.copyResourceAs("html/closed.luma",    htmlDirName, "closed.png");
   mgr.copyResourceAs("html/open.luma",      htmlDirName, "open.png");
   mgr.copyResourceAs("html/bdwn.luma",      htmlDirName, "bdwn.png");
   mgr.copyResourceAs("html/sync_on.luma",   htmlDirName, "sync_on.png");
   mgr.copyResourceAs("html/sync_off.luma",  htmlDirName, "sync_off.png");
   mgr.copyResourceAs("html/nav_g.png",      htmlDirName, "nav_g.png");
}

void HtmlGenerator::writeSearchData(const QString &dir)
{
   static const bool serverBasedSearch = Config::getBool("search-server-based");

   ResourceMgr &mgr = ResourceMgr::instance();

   mgr.copyResourceAs("html/search_l.png", dir, "search_l.png");
   Doxy_Globals::indexList.addImageFile("search/search_l.png");

   mgr.copyResourceAs("html/search_m.png", dir, "search_m.png");
   Doxy_Globals::indexList.addImageFile("search/search_m.png");

   mgr.copyResourceAs("html/search_r.png", dir, "search_r.png");
   Doxy_Globals::indexList.addImageFile("search/search_r.png");

   if (serverBasedSearch) {
      mgr.copyResourceAs("html/mag.png", dir, "mag.png");
      Doxy_Globals::indexList.addImageFile("search/mag.png");

   } else {
      mgr.copyResourceAs("html/close.png", dir, "close.png");
      Doxy_Globals::indexList.addImageFile("search/close.png");

      mgr.copyResourceAs("html/mag_sel.png", dir, "mag_sel.png");
      Doxy_Globals::indexList.addImageFile("search/mag_sel.png");
   }

   QString outputName = Config::getString("html-output") + "/search";

   QString fileName = outputName + "/search.css";
   QFile f(fileName);

   if (f.open(QIODevice::WriteOnly)) {
      static bool disableIndex = Config::getBool("disable-index");

      QByteArray data;

      if (disableIndex) {
         data = mgr.getAsString("html/search_nomenu.css");

      } else {
         data = mgr.getAsString("html/search.css");
      }

      QString searchCss = QString::fromUtf8(data);

      if (! searchCss.isEmpty()) {
         QTextStream t(&f);

         searchCss = replaceColorMarkers(searchCss);

         searchCss.replace("$doxypressversion", versionString);
         searchCss.replace("$doxygenversion",   versionString);         // compatibility

         t << searchCss;
         Doxy_Globals::indexList.addStyleSheetFile("search/search.css");
      }

   } else {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), f.error());
   }
}

void HtmlGenerator::writeStyleSheetFile(QFile &file)
{
   QTextStream t(&file);
   QString resData = ResourceMgr::instance().getAsString("html/doxypress.css");

   if (resData.isEmpty()) {
      fprintf(stderr, "\n\nIssue loading the default stylesheet file.\nPlease submit a bug report to "
              "the developers at info@copperspice.com\n");

   } else {
      resData.replace("$doxypressversion", versionString);
      resData.replace("$doxygenversion",   versionString);         // compatibility

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
   static const bool searchEngine = Config::getBool("html-search");

   QString fileName = name;

   m_lastTitle    = title;
   m_relativePath = relativePathToRoot(fileName);

   if (fileName.right(Doxy_Globals::htmlFileExtension.length()) != Doxy_Globals::htmlFileExtension) {
      fileName += Doxy_Globals::htmlFileExtension;
   }

   startPlainFile(fileName);
   m_codeGen = QMakeShared<HtmlCodeGenerator>(m_textStream, m_relativePath);

   //
   Doxy_Globals::indexList.addIndexFile(fileName);

   m_lastFile = fileName;
   m_textStream << substituteHtmlKeywords(g_header, filterTitle(title), m_relativePath);
   m_textStream << "<!-- " << theTranslator->trGeneratedBy() << " DoxyPress " << versionString << " -->" << endl;

   if (searchEngine /*&& ! generateTreeView*/) {
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
   static bool serverBasedSearch = Config::getBool("search-server-based");

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

QString HtmlGenerator::writeLogoAsString(const QString &path)
{
   static bool timeStamp = Config::getBool("html-timestamp");
   QString result;

   if (timeStamp) {
      result += theTranslator->trGeneratedAt(dateToString(true), Config::getString("project-name"));

   } else {
      result += theTranslator->trGeneratedBy();
   }

   result += "&#160;\n<a href=\"https://www.copperspice.com/documentation-doxypress.html\">";
   result += "<img class=\"footer\" src=\"";
   result += path;
   result += "doxypress.png\" alt=\"DoxyPress\"/></a>";
   result += versionString;
   result += " ";

   return result;
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

      startPlainFile("doxypress.css");
      QString resData = ResourceMgr::instance().getAsString("html/doxypress.css");

      if (resData.isEmpty()) {
         fprintf(stderr, "\n\nIssue loading the default stylesheet file.\nPlease submit a bug report to "
               "the developers at info@copperspice.com\n");

      } else {
         resData.replace("$doxypressversion", versionString);
         resData.replace("$doxygenversion",   versionString);            // compatibility

         m_textStream << replaceColorMarkers(resData);
      }

      endPlainFile();
      Doxy_Globals::indexList.addStyleSheetFile("doxypress.css");

      // part two
      static const QDir configDir           = Config::getConfigDir();
      static const QStringList extraCssFile = Config::getList("html-stylesheets");

      for (auto fileName : extraCssFile) {

         if (! fileName.isEmpty()) {
            QFileInfo fi(configDir, fileName);

            if (fi.exists()) {
               Doxy_Globals::indexList.addStyleSheetFile(fi.fileName());
            } else {
               err("Unable to find stylesheet '%s'\n", csPrintable(fi.absoluteFilePath()));
            }
         }
      }

      Doxy_Globals::indexList.addStyleSheetFile("jquery.js");
      Doxy_Globals::indexList.addStyleSheetFile("dynsections.js");

      if (Config::getBool("interactive-svg"))   {
         Doxy_Globals::indexList.addStyleSheetFile("svgpan.js");
      }
   }
}

void HtmlGenerator::startDoxyAnchor(const QString &, const QString &, const QString &anchor, const QString &, const QString &)
{
   m_textStream << "<a class=\"anchor\" id=\"" << anchor << "\"></a>";
}

void HtmlGenerator::endDoxyAnchor(const QString &, const QString &)
{
}

// void HtmlGenerator::newParagraph()
// {
//    m_textStream << endl << "<p>" << endl;
// }

void HtmlGenerator::startParagraph(const QString &className)
{
   if (className.isEmpty()) {
      m_textStream << endl << "<p>";

   } else {
      m_textStream << endl << "<p class=\"" << className << "\">";
   }
}

void HtmlGenerator::endParagraph()
{
   m_textStream << "</p>" << endl;
}

void HtmlGenerator::writeString(const QString &text)
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

void HtmlGenerator::startIndexItem(const QString &ref, const QString &f)
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
         m_textStream << f << Doxy_Globals::htmlFileExtension;
      }

      m_textStream << "\">";

   } else {
      m_textStream << "<b>";
   }
}

void HtmlGenerator::endIndexItem(const QString &ref, const QString &f)
{
   if (! ref.isEmpty() || ! f.isEmpty()) {
      m_textStream << "</a>";

   } else {
      m_textStream << "</b>";
   }
}

void HtmlGenerator::writeStartAnnoItem(const QString &, const QString &f, const QString &path, const QString &name)
{
   m_textStream << "<li>";

   if (! path.isEmpty()) {
      docify(path);
   }

   m_textStream << "<a class=\"el\" href=\"" << f << Doxy_Globals::htmlFileExtension << "\">";
   docify(name);

   m_textStream << "</a> ";
}

void HtmlGenerator::writeObjectLink(const QString &ref, const QString &f, const QString &anchor, const QString &name)
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
      m_textStream << f << Doxy_Globals::htmlFileExtension;
   }

   if (! anchor.isEmpty()) {
      m_textStream << "#" << anchor;
   }

   m_textStream << "\">";
   docify(name);

   m_textStream << "</a>";
}

void HtmlGenerator::startTextLink(const QString &f, const QString &anchor)
{
   m_textStream << "<a href=\"";

   if (! f.isEmpty()) {
      m_textStream << m_relativePath << f << Doxy_Globals::htmlFileExtension;
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

void HtmlGenerator::startSection(const QString &lab, const QString &, SectionInfo::SectionType type)
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

void HtmlGenerator::endSection(const QString &, SectionInfo::SectionType type)
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

void HtmlGenerator::docify(const QString &text)
{
   docify(text, false);
}

void HtmlGenerator::docify(const QString &text, bool inHtmlComment)
{
   bool isBackSlash = false;

   for (auto c : text) {

      switch (c.unicode()) {
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
            if (isBackSlash) {
              isBackSlash = false;

              m_textStream << "\\\\";

            } else {
              isBackSlash = true;

            }
            break;

         default:
            m_textStream << c;
      }

      if (isBackSlash && c != '\\') {
         isBackSlash = false;
      }
   }
}

void HtmlGenerator::writeChar(char c)
{
   QString tmp = QChar(c);
   docify(tmp);
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
   // t_stream << "<!-- startSectionSummary -->";
   static bool dynamicSections = Config::getBool("html-dynamic-sections");

   if (dynamicSections) {
      t_stream << "<div id=\"dynsection-" << sectionCount << "-summary\" "
         "class=\"dynsummary\" "
         "style=\"display:block;\">" << endl;
   }
}

static void endSectionSummary(QTextStream &t_stream)
{
   // t_stream << "<!-- endSectionSummary -->";
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
   // t_stream << "<!-- endSectionContent -->";
   t_stream << "</div>" << endl;
}

void HtmlGenerator::startClassDiagram()
{
   startSectionHeader(m_textStream, m_relativePath, m_sectionCount);
}

void HtmlGenerator::endClassDiagram(const ClassDiagram &d, const QString &fname, const QString &name)
{
   QString imgData;
   QTextStream t_stream(&imgData);
   d.writeImage(t_stream, m_outputDir, m_relativePath, fname);

   //
   endSectionHeader(m_textStream);
   startSectionSummary(m_textStream, m_sectionCount);

   endSectionSummary(m_textStream);
   startSectionContent(m_textStream, m_sectionCount);

   if (! imgData.isEmpty()) {

      m_textStream << " <div class=\"center\">" << endl;
      m_textStream << "  <img src=\"";
      m_textStream << m_relativePath << fname << ".png\" usemap=\"#" << convertToId(name);

      m_textStream << "_map\" alt=\"\"/>" << endl;
      m_textStream << "  <map id=\"" << convertToId(name);

      m_textStream << "_map\" name=\"" << convertToId(name);
      m_textStream << "_map\">" << endl;

      m_textStream << imgData;

      m_textStream << "  </map>" << endl;
      m_textStream << " </div>";

   } else {
      m_textStream << " <div class=\"center\">" << endl;
      m_textStream << "  <img src=\"";
      m_textStream << m_relativePath << fname << ".png\" alt=\"\"/>" << endl;
      m_textStream << " </div>";
  }

   endSectionContent(m_textStream);
   ++m_sectionCount;
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
void HtmlGenerator::startMemberItem(const QString &anchor, int annoType, const QString &inheritId, bool deprecated)
{
   if (m_emptySection) {
      m_textStream << "<table class=\"memberdecls\">" << endl;
      m_emptySection = false;
   }

   if (deprecated) {
      m_textStream << "<tr class=\"deprecated memitem:" << anchor;
   } else {
      m_textStream << "<tr class=\"memitem:" << anchor;
   }

   if (! inheritId.isEmpty()) {
      m_textStream << " inherit " << inheritId;
   }

   m_textStream << "\">";
   insertMemberAlignLeft(annoType, true);
}

void HtmlGenerator::endMemberItem()
{
   m_textStream << "</td></tr>";
   m_textStream << endl;
}

void HtmlGenerator::startMemberTemplateParams()
{
}

void HtmlGenerator::endMemberTemplateParams(const QString &anchor, const QString &inheritId)
{
   m_textStream << "</td></tr>" << endl;
   m_textStream << "<tr class=\"memitem:" << anchor;

   if (! inheritId.isEmpty()) {
      m_textStream << " inherit " << inheritId;
   }

   m_textStream << "\"><td class=\"memTemplItemLeft\" align=\"right\" valign=\"top\">";
}

void HtmlGenerator::insertMemberAlign(bool alignRight)
{
   QString className = alignRight ? QString("memTemplItemRight") : QString("memItemRight");
   m_textStream << "&#160;</td><td class=\"" << className << "\" valign=\"bottom\">";
}

void HtmlGenerator::insertMemberAlignLeft(int annoType, bool alignLeft)
{
   if (! alignLeft) {
      m_textStream << "&#160;</td>";
   }

  switch(annoType)  {
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

void HtmlGenerator::startMemberDescription(const QString &anchor, const QString &inheritId, bool type)
{
   if (m_emptySection) {
      m_textStream << "<table class=\"memberdecls\">" << endl;
      m_emptySection = false;
   }

   m_textStream << "<tr class=\"memdesc:" << anchor;

   if (! inheritId.isEmpty()) {
      m_textStream << " inherit " << inheritId;
   }

   m_textStream << "\"><td class=\"mdescLeft\">&#160;</td>";

   if (type) {
      m_textStream << "<td class=\"mdescLeft\">&#160;</td>";
   }

   m_textStream << "<td class=\"mdescRight\">";
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

   // postpone writing <table> until we actually
   // write a row to prevent empty tables, which are not valid XHTML
}

void HtmlGenerator::endMemberSections()
{
   DBG_HTML(m_textStream << "<!-- endMemberSections -->" << endl)

   if (! m_emptySection) {
      m_textStream << "</table>" << endl;
   }
}

void HtmlGenerator::startMemberHeader(const QString &anchor, int type)
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

   m_textStream << "<tr class=\"heading\"><td colspan=\"" << type << "\"><h2 class=\"groupheader\">";

   if (! anchor.isEmpty()) {
      m_textStream << "<a name=\"" << anchor << "\"></a>" << endl;
   }
}

void HtmlGenerator::endMemberHeader()
{
   m_textStream << "</h2></td></tr>" << endl;
}

void HtmlGenerator::startMemberSubtitle()
{
   m_textStream << "<tr><td class=\"ititle\" colspan=\"2\">";
}

void HtmlGenerator::endMemberSubtitle()
{
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

void HtmlGenerator::endIndexValue(const QString &, bool)
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

void HtmlGenerator::startMemberDoc(const QString &, const QString &, const QString &, const QString &,
            int memCount, int memTotal, bool)
{
   (void) memCount;
   (void) memTotal;

   // (emerald) bypass 'tab' above docs
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

void HtmlGenerator::startParameterType(bool first, const QString &key)
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

void HtmlGenerator::exceptionEntry(const QString &prefix, bool closeBracket)
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

   if (! hasArgs) {
      m_textStream << "        </tr>" << endl;
   }

   m_textStream << "      </table>" << endl;
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

   g.writeGraph(m_textStream, GOF_BITMAP, EOF_Html, m_outputDir, m_fileName, m_relativePath, true, true, m_sectionCount);

   if (generateLegend && ! umlLook) {
      m_textStream << "<center><span class=\"legend\">[";
      startHtmlLink(m_relativePath + "graph_legend" + Doxy_Globals::htmlFileExtension);

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

   g.writeGraph(m_textStream, GOF_BITMAP, EOF_Html, m_outputDir, m_fileName, m_relativePath, true, m_sectionCount);

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

   g.writeGraph(m_textStream, GOF_BITMAP, EOF_Html, m_outputDir, m_fileName, m_relativePath, true, m_sectionCount);

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

   g.writeGraph(m_textStream, GOF_BITMAP, EOF_Html, m_outputDir, m_fileName, m_relativePath, true, m_sectionCount);

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

   g.writeGraph(m_textStream, GOF_BITMAP, EOF_Html, m_outputDir, m_fileName, m_relativePath, true, m_sectionCount);

   endSectionContent(m_textStream);
   m_sectionCount++;
}

void HtmlGenerator::writeGraphicalHierarchy(const DotGfxHierarchyTable &g)
{
   g.writeGraph(m_textStream, m_outputDir, m_fileName);
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
   m_textStream << "<div class=\"memdoc\">\n";
}

void HtmlGenerator::endIndent()
{
   m_textStream << endl << "</div>" << endl << "</div>" << endl;
}

void HtmlGenerator::addIndexItemName(const QString &, const QString &)
{
}

void HtmlGenerator::writeNonBreakableSpace(int n)
{
   for (int i = 0; i < n; i++) {
      m_textStream << "&#160;";
   }
}

void HtmlGenerator::startDescTable(const QString &title)
{
   m_textStream << "<table class=\"fieldtable\">" << endl
                << "<tr><th colspan=\"2\">" << title << "</th></tr>";
}

void HtmlGenerator::endDescTable()
{
   m_textStream << "</table>" << endl;
}

void HtmlGenerator::startDescTableRow()
{
   m_textStream << "<tr>";
}

void HtmlGenerator::endDescTableRow()
{
   m_textStream << "</tr>" << endl;
}

void HtmlGenerator::startDescTableTitle()
{
   m_textStream << "<td class=\"fieldname\">";
}

void HtmlGenerator::endDescTableTitle()
{
   m_textStream << "&#160;</td>";
}

void HtmlGenerator::startDescTableData()
{
   m_textStream << "<td class=\"fielddoc\">";
}

void HtmlGenerator::endDescTableData()
{
   m_textStream << "</td>";
}

void HtmlGenerator::startSimpleSect(SectionTypes, const QString &filename, const QString &anchor, const QString &title)
{
   m_textStream << "<dl><dt><b>";

   if (! filename.isEmpty()) {
      writeObjectLink(QString(), filename, anchor, title);

   } else {
      docify(title);
   }

   m_textStream << "</b></dt>";
}

void HtmlGenerator::endSimpleSect()
{
   m_textStream << "</dl>";
}

void HtmlGenerator::startParamList(ParamListTypes, const QString &title)
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
   (void) md;
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

static void startQuickIndexItem(QTextStream &t_stream, const QString &l, bool hl, bool x, const QString &relPath)
{
   (void) x;

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

static void endQuickIndexItem(QTextStream &t_stream, const QString &l)
{
   t_stream << "</span>";

   if (! l.isEmpty()) {
      t_stream << "</a>";
   }

   t_stream << "</li>\n";
}

static bool quickLinkVisible(LayoutNavEntry::Kind kind)
{
   static bool showFiles      = Config::getBool("show-file-page");
   static bool showNamespaces = Config::getBool("show-namespace-page");

   switch (kind) {
      case LayoutNavEntry::None:
      case LayoutNavEntry::FileSource:
         return false;

      case LayoutNavEntry::MainPage:
         return true;

      case LayoutNavEntry::User:
         return true;

      case LayoutNavEntry::UserGroup:
         return true;

      case LayoutNavEntry::Pages:
         return Doxy_Globals::indexedPages > 0;

      case LayoutNavEntry::Modules:
         return getCount(IndexTotals::DocumentedGroups) > 0;

      case LayoutNavEntry::Namespaces:
      case LayoutNavEntry::NamespaceList:
         return getCount(IndexTotals::DocumentedNamespaces) > 0 && showNamespaces;

      case LayoutNavEntry::NamespaceMembers:
        return getCount(IndexTotals::DocumentedNamespaceMembers) > 0;

      case LayoutNavEntry::Classes:
      case LayoutNavEntry::ClassList:
      case LayoutNavEntry::ClassIndex:
         return getCount(IndexTotals::AnnotatedCount) > 0;

      case LayoutNavEntry::ClassHierarchy:
         return getCount(IndexTotals::HierarchyCount) > 0;

      case LayoutNavEntry::ClassMembers:
         return getCount(IndexTotals::DocumentedClassMembers) > 0;

      case LayoutNavEntry::Concepts:
      case LayoutNavEntry::ConceptList:
         return getCount(IndexTotals::ConceptCount) > 0;

      case LayoutNavEntry::Files:
      case LayoutNavEntry::FileList:
         return Doxy_Globals::documentedHtmlFiles > 0 && showFiles;

      case LayoutNavEntry::FileGlobals:
         return getCount(IndexTotals::DocumentedFileMembers) > 0;

      case LayoutNavEntry::Examples:
         return Doxy_Globals::exampleSDict.count() > 0;
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

   if (count > 0) {
      // at least one item is visible
      startQuickIndexList(t_stream, false);

      for (auto entry : root->children()) {
         if (entry->visible() && quickLinkVisible(entry->kind())) {
            QString url = entry->url();

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
               QString url = entry->url();

               startQuickIndexItem(t_stream, url, entry == hlEntry  &&
                                   (entry->children().count() > 0 || (entry->kind() == kind && ! highlightParent) ), true, relPath);

               t_stream << fixSpaces(entry->title());
               endQuickIndexItem(t_stream, url);
            }
         }

         if (hlEntry->parent() == LayoutDocManager::instance().rootNavEntry()) { // first row is special as it contains the search box
            static bool searchEngine      = Config::getBool("html-search");
            static bool serverBasedSearch = Config::getBool("search-server-based");

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

            if (!highlightSearch) {
               // on the search page the index will be ended by the page itself
               endQuickIndexList(t_stream, true);
            }

         } else {
            // normal case for other rows than first one
            endQuickIndexList(t_stream, true);
         }
      }
   }
}

static void writeDefaultQuickLinks(QTextStream &t_stream, bool compact, HighlightedItem hli,
                  const QString &file, const QString &relPath)
{
   //   static const bool searchEngine      = Config::getBool("html-search");
   //   static const bool serverBasedSearch = Config::getBool("search-server-based");
   //   static const bool externalSearch    = Config::getBool("search-external");

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

      case HLI_NamespaceMembers:
         kind = LayoutNavEntry::NamespaceMembers;
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

      case HLI_Concepts:
         kind = LayoutNavEntry::ConceptList;
         altKind = LayoutNavEntry::Concepts;
         break;

      case HLI_Files:
         kind = LayoutNavEntry::FileList;
         altKind = LayoutNavEntry::Files;
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

      case HLI_FileSource:
         kind = LayoutNavEntry::FileSource;
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

      case HLI_ConceptVisible:
         kind = LayoutNavEntry::ConceptList;
         altKind = LayoutNavEntry::Concepts;
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
         kind    = altKind;
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

QString HtmlGenerator::writeSplitBarAsString(const QString &name, const QString &relpath)
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
                  "$(document).ready(function(){initNavTree('" + name +
                  Doxy_Globals::htmlFileExtension + "','" + relpath +
                  "'); initResizable(); });\n" + "</script>\n" +
                  "<div id=\"doc-content\">\n";
   }

   return result;
}

void HtmlGenerator::writeSplitBar(const QString &name)
{
   m_textStream << writeSplitBarAsString(name, m_relativePath);
}

void HtmlGenerator::writeNavigationPath(const QString &s)
{
   m_textStream << substitute(s, "$relpath^", m_relativePath);
}

void HtmlGenerator::startContents()
{
   m_textStream << "<div class=\"contents\">" << endl;
}

void HtmlGenerator::endContents()
{
   m_textStream << "</div><!-- contents -->" << endl;
}

void HtmlGenerator::startPageDoc(const QString &pageTitle)
{
   // RTL    m_textStream << "<div" << getDirHtmlClassOfPage(pageTitle) << ">";
   (void) pageTitle;
}

void HtmlGenerator::endPageDoc()
{
   // RTL    m_textStream << "</div><!-- PageDoc -->" << endl;
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

   static QString projectName    = Config::getString("project-name");
   static QString htmlOutput     = Config::getString("html-output");

   //
   QString configFileName = htmlOutput + "/search_config.php";
   QFile cf(configFileName);

   if (cf.open(QIODevice::WriteOnly)) {
      QTextStream t_stream(&cf);

      t_stream << "<?php\n\n";
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
      t_stream << "?>\n";

   } else {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(configFileName), cf.error());
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

      t_stream << "<?php\n";
      t_stream << "require_once \"search_functions.php\";\n";
      t_stream << "main();\n";
      t_stream << "?>\n";

      // Write empty navigation path, to make footer connect properly
      if (generateTreeView) {
         t_stream << "</div><!-- doc-content -->\n";
      }

      writePageFooter(t_stream, "Search", QString(), QString());
   }

   QString scriptName = htmlOutput + "/search/search.js";
   QFile sf(scriptName);

   if (sf.open(QIODevice::WriteOnly)) {
      QTextStream t_stream(&sf);
      t_stream << ResourceMgr::instance().getAsString("html/extsearch.js");

   } else {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(scriptName), f.error());
   }
}

void HtmlGenerator::writeExternalSearchPage()
{
   static bool generateTreeView = Config::getBool("generate-treeview");
   QString fileName = Config::getString("html-output") + "/search" + Doxy_Globals::htmlFileExtension;

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
         t_stream << "</div><!-- doc-content -->" << endl;
      }

      writePageFooter(t_stream, "Search", QString(), QString());

   } else {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), f.error());

   }

   QString scriptName = Config::getString("html-output") + "/search/search.js";
   QFile sf(scriptName);

   if (sf.open(QIODevice::WriteOnly)) {
      QTextStream t_stream(&sf);

      t_stream << "var searchResultsText=["
        << "\"" << theTranslator->trSearchResults(0) << "\","
        << "\"" << theTranslator->trSearchResults(1) << "\","
        << "\"" << theTranslator->trSearchResults(2) << "\"];" << endl;

      t_stream << "var serverUrl=\"" << Config::getString("search-external-url") << "\";" << endl;
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
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(scriptName), sf.error());
   }
}

void HtmlGenerator::startConstraintList(const QString &header)
{
   m_textStream << "<div class=\"typeconstraint\">" << endl;
   m_textStream << "<dl><dt><b>" << header << "</b></dt>" << endl;
   m_textStream << "<dd>" << endl;
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
   m_textStream << "</dd>" << endl;
   m_textStream << "</dl>" << endl;
   m_textStream << "</div>" << endl;
}

void HtmlGenerator::lineBreak(const QString &style)
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

void HtmlGenerator::startTitleHead(const QString &)
{
   m_textStream << "  <div class=\"headertitle\">" << endl;
   startTitle();
}

void HtmlGenerator::endTitleHead(const QString &, const QString &)
{
   endTitle();
   m_textStream << "  </div>" << endl;
}

void HtmlGenerator::endHeaderSection()
{
   m_textStream << "  <div class=\"clear-floats\"></div>\n";
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

void HtmlGenerator::startMemberDocSimple(bool isEnum)
{
   m_textStream << "<table class=\"fieldtable\">" << endl;
   m_textStream << "<tr><th colspan=\"" << (isEnum ? "2" : "3") << "\">";
   m_textStream << (isEnum ? theTranslator->trEnumerationValues() : theTranslator->trCompoundMembers())
                << "</th></tr>" << endl;
}

void HtmlGenerator::endMemberDocSimple(bool isEnum)
{
   (void) isEnum;
   m_textStream << "</table>" << endl;
}

void HtmlGenerator::startInlineMemberType()
{
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

void HtmlGenerator::writeLabel(const QString &l, bool)
{
   DBG_HTML(m_textStream << "<!-- writeLabel(" << l << ") -->" << endl;)
   m_textStream << "<span class=\"mlabel\">" << l << "</span>";
}

void HtmlGenerator::endLabels()
{
   DBG_HTML(m_textStream << "<!-- endLabels -->" << endl;)
   m_textStream << "</span>";
}

void HtmlGenerator::writeInheritedSectionTitle(const QString &id, const QString &ref, const QString &file, const QString &anchor,
                                               const QString &title, const QString &name)
{
   DBG_HTML(m_textStream << "<!-- writeInheritedSectionTitle -->" << endl;)
   QString a = anchor;

   if (! a.isEmpty()) {
      a.prepend("#");
   }

   QString classLink = "<a class=\"el\" ";

   if (! ref.isEmpty()) {
      classLink += externalLinkTarget();
      classLink += " href=\"";
      classLink += externalRef(m_relativePath, ref, true);

   } else {
      classLink += "href=\"";
      classLink += m_relativePath;
   }

   classLink += file + Doxy_Globals::htmlFileExtension + a;
   classLink += "\">" + convertToHtml(name, false) + "</a>";

   m_textStream << "<tr class=\"inherit_header " << id << "\">"
     << "<td colspan=\"2\" onclick=\"javascript:toggleInherit('" << id << "')\">"
     << "<img src=\"" << m_relativePath << "closed.png\" alt=\"-\"/>&#160;"
     << theTranslator->trInheritedFrom(convertToHtml(title, false), classLink)
     << "</td></tr>" << endl;
}

void HtmlGenerator::writeSummaryLink(const QString &file, const QString &anchor, const QString &title, bool first)
{
   if (first) {
      m_textStream << "  <div class=\"summary\">\n";

   } else {
      m_textStream << " &#124;\n";

   }

   m_textStream << "<a href=\"";

   if (! file.isEmpty()) {
      m_textStream << m_relativePath << file;
      m_textStream << Doxy_Globals::htmlFileExtension;

   } else {
      m_textStream << "#";
      m_textStream << anchor;
   }

   m_textStream << "\">";
   m_textStream << title;
   m_textStream << "</a>";
}

void HtmlGenerator::endMemberDeclaration(const QString &anchor, const QString &inheritId)
{
   m_textStream << "<tr class=\"separator:" << anchor;

   if (! inheritId.isEmpty() ) {
      m_textStream << " inherit " << inheritId;
   }

   m_textStream << "\"><td class=\"memSeparator\" colspan=\"2\">&#160;</td></tr>\n";
}

void HtmlGenerator::setCurrentDoc(QSharedPointer<Definition> context, const QString &anchor, bool isSourceFile)
{
   if (Doxy_Globals::searchIndexBase != nullptr) {
      Doxy_Globals::searchIndexBase->setCurrentDoc(context, anchor, isSourceFile);
   }
}

void HtmlGenerator::addWord(const QString &word, bool hiPriority)
{
   if (Doxy_Globals::searchIndexBase != nullptr) {
      Doxy_Globals::searchIndexBase->addWord(word, hiPriority);
   }
}

