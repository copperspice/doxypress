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

#include <QCryptographicHash>
#include <QDir>
#include <QFile>

#include <stdlib.h>

#include <dot.h>

#include <config.h>
#include <defargs.h>
#include <dirdef.h>
#include <docparser.h>
#include <doxy_globals.h>
#include <groupdef.h>
#include <language.h>
#include <message.h>
#include <membergroup.h>
#include <portable.h>
#include <sortedlist.h>
#include <util.h>

static QVector<int> s_newNumber;
static int s_max_newNumber = 0;
static QString g_dotFontPath;

int DotGfxHierarchyTable::m_curNodeNumber;
int DotInclDepGraph::m_curNodeNumber = 0;
int DotClassGraph::m_curNodeNumber   = 0;
int DotCallGraph::m_curNodeNumber    = 0;

#define MAP_CMD "cmapx"

//#define FONTNAME "Helvetica"
#define FONTNAME getDotFontName()
#define FONTSIZE getDotFontSize()

static const char svgZoomHeader[] =
   "<svg id=\"main\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xml:space=\"preserve\" onload=\"init(evt)\">\n"
   "<style type=\"text/css\"><![CDATA[\n"
   ".edge:hover path { stroke: red; }\n"
   ".edge:hover polygon { stroke: red; fill: red; }\n"
   "]]></style>\n"
   "<script type=\"text/javascript\"><![CDATA[\n"
   "var edges = document.getElementsByTagName('g');\n"
   "if (edges && edges.length) {\n"
   "  for (var i=0;i<edges.length;i++) {\n"
   "    if (edges[i].id.substr(0,4)=='edge') {\n"
   "      edges[i].setAttribute('class','edge');\n"
   "    }\n"
   "  }\n"
   "}\n"
   "]]></script>\n"
   "        <defs>\n"
   "                <circle id=\"rim\" cx=\"0\" cy=\"0\" r=\"7\"/>\n"
   "                <circle id=\"rim2\" cx=\"0\" cy=\"0\" r=\"3.5\"/>\n"
   "                <g id=\"zoomPlus\">\n"
   "                        <use xlink:href=\"#rim\" fill=\"#404040\">\n"
   "                                <set attributeName=\"fill\" to=\"#808080\" begin=\"zoomplus.mouseover\" end=\"zoomplus.mouseout\"/>\n"
   "                        </use>\n"
   "                        <path d=\"M-4,0h8M0,-4v8\" fill=\"none\" stroke=\"white\" stroke-width=\"1.5\" pointer-events=\"none\"/>\n"
   "                </g>\n"
   "                <g id=\"zoomMin\">\n"
   "                        <use xlink:href=\"#rim\" fill=\"#404040\">\n"
   "                                <set attributeName=\"fill\" to=\"#808080\" begin=\"zoomminus.mouseover\" end=\"zoomminus.mouseout\"/>\n"
   "                        </use>\n"
   "                        <path d=\"M-4,0h8\" fill=\"none\" stroke=\"white\" stroke-width=\"1.5\" pointer-events=\"none\"/>\n"
   "                </g>\n"
   "                <g id=\"dirArrow\">\n"
   "                        <path fill=\"none\" stroke=\"white\" stroke-width=\"1.5\" d=\"M0,-3.0v7 M-2.5,-0.5L0,-3.0L2.5,-0.5\"/>\n"
   "                </g>\n"
   "               <g id=\"resetDef\">\n"
   "                       <use xlink:href=\"#rim2\" fill=\"#404040\">\n"
   "                               <set attributeName=\"fill\" to=\"#808080\" begin=\"reset.mouseover\" end=\"reset.mouseout\"/>\n"
   "                       </use>\n"
   "               </g>\n"
   "        </defs>\n"
   "\n"
   "<script type=\"text/javascript\">\n"
   ;

static const char svgZoomFooter[] =
   // navigation panel
   "        <g id=\"navigator\" transform=\"translate(0 0)\" fill=\"#404254\">\n"
   "                <rect fill=\"#f2f5e9\" fill-opacity=\"0.5\" stroke=\"#606060\" stroke-width=\".5\" x=\"0\" y=\"0\" width=\"60\" height=\"60\"/>\n"
   // zoom in
   "                <use id=\"zoomplus\" xlink:href=\"#zoomPlus\" x=\"17\" y=\"9\" onmousedown=\"handleZoom(evt,'in')\"/>\n"
   // zoom out
   "                <use id=\"zoomminus\" xlink:href=\"#zoomMin\" x=\"42\" y=\"9\" onmousedown=\"handleZoom(evt,'out')\"/>\n"
   // reset zoom
   "                <use id=\"reset\" xlink:href=\"#resetDef\" x=\"30\" y=\"36\" onmousedown=\"handleReset()\"/>\n"
   // arrow up
   "                <g id=\"arrowUp\" xlink:href=\"#dirArrow\" transform=\"translate(30 24)\" onmousedown=\"handlePan(0,-1)\">\n"
   "                  <use xlink:href=\"#rim\" fill=\"#404040\">\n"
   "                        <set attributeName=\"fill\" to=\"#808080\" begin=\"arrowUp.mouseover\" end=\"arrowUp.mouseout\"/>\n"
   "                  </use>\n"
   "                  <path fill=\"none\" stroke=\"white\" stroke-width=\"1.5\" d=\"M0,-3.0v7 M-2.5,-0.5L0,-3.0L2.5,-0.5\"/>\n"
   "                </g>\n"
   // arrow right
   "                <g id=\"arrowRight\" xlink:href=\"#dirArrow\" transform=\"rotate(90) translate(36 -43)\" onmousedown=\"handlePan(1,0)\">\n"
   "                  <use xlink:href=\"#rim\" fill=\"#404040\">\n"
   "                        <set attributeName=\"fill\" to=\"#808080\" begin=\"arrowRight.mouseover\" end=\"arrowRight.mouseout\"/>\n"
   "                  </use>\n"
   "                  <path fill=\"none\" stroke=\"white\" stroke-width=\"1.5\" d=\"M0,-3.0v7 M-2.5,-0.5L0,-3.0L2.5,-0.5\"/>\n"
   "                </g>\n"
   // arrow down
   "                <g id=\"arrowDown\" xlink:href=\"#dirArrow\" transform=\"rotate(180) translate(-30 -48)\" onmousedown=\"handlePan(0,1)\">\n"
   "                  <use xlink:href=\"#rim\" fill=\"#404040\">\n"
   "                        <set attributeName=\"fill\" to=\"#808080\" begin=\"arrowDown.mouseover\" end=\"arrowDown.mouseout\"/>\n"
   "                  </use>\n"
   "                  <path fill=\"none\" stroke=\"white\" stroke-width=\"1.5\" d=\"M0,-3.0v7 M-2.5,-0.5L0,-3.0L2.5,-0.5\"/>\n"
   "                </g>\n"
   // arrow left
   "                <g id=\"arrowLeft\" xlink:href=\"#dirArrow\" transform=\"rotate(270) translate(-36 17)\" onmousedown=\"handlePan(-1,0)\">\n"
   "                  <use xlink:href=\"#rim\" fill=\"#404040\">\n"
   "                        <set attributeName=\"fill\" to=\"#808080\" begin=\"arrowLeft.mouseover\" end=\"arrowLeft.mouseout\"/>\n"
   "                  </use>\n"
   "                  <path fill=\"none\" stroke=\"white\" stroke-width=\"1.5\" d=\"M0,-3.0v7 M-2.5,-0.5L0,-3.0L2.5,-0.5\"/>\n"
   "                </g>\n"
   "        </g>\n"
   // link to orginial SVG
   "        <svg viewBox=\"0 0 15 15\" width=\"100%\" height=\"30px\" preserveAspectRatio=\"xMaxYMin meet\">\n"
   "         <g id=\"arrow_out\" transform=\"scale(0.3 0.3)\">\n"
   "          <a xlink:href=\"$orgname\" target=\"_base\">\n"
   "           <rect id=\"button\" ry=\"5\" rx=\"5\" y=\"6\" x=\"6\" height=\"38\" width=\"38\"\n"
   "                fill=\"#f2f5e9\" fill-opacity=\"0.5\" stroke=\"#606060\" stroke-width=\"1.0\"/>\n"
   "           <path id=\"arrow\"\n"
   "             d=\"M 11.500037,31.436501 C 11.940474,20.09759 22.043105,11.32322 32.158766,21.979434 L 37.068811,17.246167 C 37.068811,17.246167 37.088388,32 37.088388,32 L 22.160133,31.978069 C 22.160133,31.978069 26.997745,27.140456 26.997745,27.140456 C 18.528582,18.264221 13.291696,25.230495 11.500037,31.436501 z\"\n"
   "             style=\"fill:#404040;\"/>\n"
   "          </a>\n"
   "         </g>\n"
   "        </svg>\n"
   "</svg>\n"
   ;

/*! mapping from protection levels to color names */
static const char *normalEdgeColorMap[] = {
   "midnightblue",  // Public
   "darkgreen",     // Protected
   "firebrick4",    // Private
   "darkorchid3",   // "use" relation
   "grey75",        // Undocumented
   "orange",        // template relation
   "orange"         // type constraint
};

static const char *normalArrowStyleMap[] = {
   "empty",         // Public
   "empty",         // Protected
   "empty",         // Private
   "open",          // "use" relation
   0,               // Undocumented
   0                // template relation
};

static const char *normalEdgeStyleMap[] = {
   "solid",         // inheritance
   "dashed"         // usage
};

static const char *umlEdgeColorMap[] = {
   "midnightblue",  // Public
   "darkgreen",     // Protected
   "firebrick4",    // Private
   "grey25",        // "use" relation
   "grey75",        // Undocumented
   "orange",        // template relation
   "orange"         // type constraint
};

static const char *umlArrowStyleMap[] = {
   "onormal",         // Public
   "onormal",         // Protected
   "onormal",         // Private
   "odiamond",        // "use" relation
   0,                 // Undocumented
   0                  // template relation
};

static const char *umlEdgeStyleMap[] = {
   "solid",         // inheritance
   "solid"          // usage
};

/** Helper struct holding the properties of a edge in a dot graph. */
struct EdgeProperties {
   const char *const *edgeColorMap;
   const char *const *arrowStyleMap;
   const char *const *edgeStyleMap;
};

static EdgeProperties normalEdgeProps = {
   normalEdgeColorMap, normalArrowStyleMap, normalEdgeStyleMap
};

static EdgeProperties umlEdgeProps = {
   umlEdgeColorMap, umlArrowStyleMap, umlEdgeStyleMap
};

static QString getDotFontName()
{
   static QString dotFontName = Config::getString("dot-fontname");

   if (dotFontName.isEmpty()) {
      dotFontName = "Helvetica";
   }

   return dotFontName;
}

static int getDotFontSize()
{
   static int dotFontSize = Config::getInt("dot-fontsize");

   if (dotFontSize < 4) {
      dotFontSize = 4;
   }

   return dotFontSize;
}

static void writeGraphHeader(QTextStream &t, const QString &title = QString())
{
   static bool interactiveSVG = Config::getBool("interactive-svg");

   t << "digraph ";
   if (title.isEmpty()) {
      t << "\"Dot Graph\"";
   } else {
      t << "\"" << convertToXML(title) << "\"";
   }

   t << endl << "{" << endl;
   if (interactiveSVG) // insert a comment to force regeneration when this
      // option is toggled
   {
      t << " // INTERACTIVE_SVG=YES\n";
   }
   if (Config::getBool("dot-transparent")) {
      t << "  bgcolor=\"transparent\";" << endl;
   }
   t << "  edge [fontname=\"" << FONTNAME << "\","
     "fontsize=\"" << FONTSIZE << "\","
     "labelfontname=\"" << FONTNAME << "\","
     "labelfontsize=\"" << FONTSIZE << "\"];\n";
   t << "  node [fontname=\"" << FONTNAME << "\","
     "fontsize=\"" << FONTSIZE << "\",shape=record];\n";
}

static void writeGraphFooter(QTextStream &t)
{
   t << "}" << endl;
}

static QString replaceRef(const QString &buf, const QString relPath, bool urlOnly, const QString &context, 
                  const QString &target = QString())
{
   // search for href="...", store ... part in link
   QString href = "href";
   
   int len = 6;
   int indexS = buf.indexOf("href=\""), indexE;

   if (indexS > 5 && buf.indexOf("xlink:href=\"") != -1) { 
      // XLink href (for SVG)
      indexS -= 6;
      len += 6;
      href.prepend("xlink:");      
   }

   if (indexS >= 0 && (indexE = buf.indexOf('"', indexS + len)) != -1) {
      QString link = buf.mid(indexS + len, indexE - indexS - len);
      QString result;

      if (urlOnly) { // for user defined dot graphs
         if (link.left(5) == "\\ref " || link.left(5) == "@ref ") { // \ref url
            result = href + "=\"";

            // fake ref node to resolve the url
            DocRef *df = new DocRef( (DocNode *) 0, link.mid(5), context );
            result += externalRef(relPath, df->ref(), true);

            if (! df->file().isEmpty()) {
               result += df->file();
               result += Doxy_Globals::htmlFileExtension;
            }

            if (! df->anchor().isEmpty()) {
               result += "#" + df->anchor();
            }

            delete df;
            result += "\"";

         } else {
            result = href + "=\"" + link + "\"";
         }

      } else { // ref$url (external ref via tag file), or $url (local ref)
         int marker = link.indexOf('$');

         if (marker != -1) {
            QString ref = link.left(marker);
            QString url = link.mid(marker + 1);

            if (! ref.isEmpty()) {
               result = externalLinkTarget() + externalRef(relPath, ref, false);
            }

            result += href + "=\"";
            result += externalRef(relPath, ref, true);
            result += url + "\"";

         } else { // should not happen, but handle properly anyway
            result = href + "=\"" + link + "\"";
         }
      }
      if (!target.isEmpty()) {
         result += " target=\"" + target + "\"";
      }

      QString leftPart  = buf.left(indexS);
      QString rightPart = buf.mid(indexE + 1);

      return leftPart + result + rightPart;

   } else {
      return buf;
   }
}

/*! converts the rectangles in a client site image map into a stream
 *  \param t the stream to which the result is written.
 *  \param mapName the name of the map file.
 *  \param relPath the relative path to the root of the output directory
 *                 (used in case CREATE_SUBDIRS is enabled).
 *  \param urlOnly if false the url field in the map contains an external
 *                 references followed by a $ and then the URL.
 *  \param context the context (file, class, or namespace) in which the
 *                 map file was found
 *  \returns true if successful.
 */
static bool convertMapFile(QTextStream &t, const QString &mapName, const QString &relPath, bool urlOnly = false,
                           const QString &context = QString())
{
   QFile f(mapName);

   if (! f.open(QIODevice::ReadOnly)) {                 
      err("Unable to open map file %s\n" 
          "If Graphviz/dot was installed after a previous DoxyPress issue, please delete the output directory and "
          "regenerate the documentation.\n", qPrintable(mapName));

      return false;
   }

   const int maxLineLen = 10240;

   while (! f.atEnd()) { 
      // foreach line

      QByteArray buf;
      buf.resize(maxLineLen);

      int numBytes = f.readLine(buf.data(), maxLineLen);      
      buf.resize(numBytes);

      if (buf.startsWith("<area")) {
         t << replaceRef(buf, relPath, urlOnly, context);         
      }
   }

   return true;
}

inline int reNumberNode(int number, bool doReNumbering)
{
   if (! doReNumbering) {
      return number;

   } else {
      int s = s_newNumber.size();
      if (number >= s) {
         int ns = 0;
         ns = s * 3 / 2 + 5; // new size
         if (number >= ns) { // number still doesn't fit
            ns = number * 3 / 2 + 5;
         }

         s_newNumber.resize(ns);
         for (int i = s; i < ns; i++) { // clear new part of the array
            s_newNumber[i] = 0;
         }
      }
      int i = s_newNumber.at(number);
      if (i == 0) { // not yet mapped
         i = ++s_max_newNumber; // start from 1
         s_newNumber[number] = i;
      }
      return i;
   }
}

static void resetReNumbering()
{
   s_max_newNumber = 0;
   s_newNumber.resize(s_max_newNumber);
}

static void setDotFontPath(const QString &path)
{
   assert(g_dotFontPath.isEmpty());

   g_dotFontPath = portable_getenv("DOTFONTPATH");

   QString newFontPath = Config::getString("dot-font-path");
   QString spath = path;

   if (! newFontPath.isEmpty() && ! spath.isEmpty()) {
      newFontPath.prepend(spath + portable_pathListSeparator());

   } else if (newFontPath.isEmpty() && ! spath.isEmpty()) {
      newFontPath = path;

   } else {
      portable_unsetenv("DOTFONTPATH");
      return;
   }

   portable_setenv("DOTFONTPATH", newFontPath);
}

static void unsetDotFontPath()
{
   if (g_dotFontPath.isEmpty()) {
      portable_unsetenv("DOTFONTPATH");
   } else {
      portable_setenv("DOTFONTPATH", g_dotFontPath);
   }
   g_dotFontPath = "";
}

static bool readBoundingBox(const QString &fileName, int *width, int *height, bool isEps)
{
   QString bb;

   if (isEps) {
      bb = "%%PageBoundingBox:";
   } else {
      bb = "/MediaBox [";
   }

   QFile f(fileName);

   if (! f.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {     
      err("Unable to open file for reading %s, error: %d\n", qPrintable(fileName), f.error());
      return false;
   }

   const int maxLineLen = 1024;
   
   QByteArray buf;
   buf.resize(maxLineLen);

   while (! f.atEnd()) {
      int numBytes = f.readLine(buf.data(), maxLineLen - 1);

      if (numBytes > 0) {
         buf.resize(numBytes);

         int p = buf.indexOf(bb);
         
         if (p != -1) { 
            // found PageBoundingBox or /MediaBox string
            int x;
            int y;

            if ( sscanf(buf.constData() + (p + bb.length() ), "%d %d %d %d", &x, &y, width, height) != 4) {    
               return false;
            }

            return true;
         }

      } else { 
         // read error        
         return false;
      }
   }

   err("Unable to extract bounding box from generated diagram file %s\n", qPrintable(fileName));

   return false;
}

static bool writeVecGfxFigure(QTextStream &out, const QString &baseName, const QString &figureName)
{
   int width  = 400;
   int height = 550;

   static bool usePdfLatex = Config::getBool("latex-pdf");

   if (usePdfLatex) {
      if (! readBoundingBox(figureName + ".pdf", &width, &height, false)) {         
         return false;
      }

   } else {
      if (! readBoundingBox(figureName + ".eps", &width, &height, true)) {        
         return false;
      }
   }
   //printf("Got PDF/EPS size %d,%d\n",width,height);
   int maxWidth  = 350;  /* approx. page width in points, excl. margins */
   int maxHeight = 550;  /* approx. page height in points, excl. margins */

   out << "\\nopagebreak\n"
       "\\begin{figure}[H]\n"
       "\\begin{center}\n"
       "\\leavevmode\n";

   if (width > maxWidth || height > maxHeight) { // figure too big for page
      // c*width/maxWidth > c*height/maxHeight, where c=maxWidth*maxHeight>0
      if (width * maxHeight > height * maxWidth) {
         out << "\\includegraphics[width=" << maxWidth << "pt]";
      } else {
         out << "\\includegraphics[height=" << maxHeight << "pt]";
      }

   } else {
      out << "\\includegraphics[width=" << width << "pt]";
   }

   out << "{" << baseName << "}\n"
       "\\end{center}\n"
       "\\end{figure}\n";

  
   return true;
}

// extract size from a dot generated SVG file
static bool readSVGSize(const QString &fileName, int *width, int *height)
{
   bool found = false;
   QFile f(fileName);

   if (! f.open(QIODevice::ReadOnly)) {
      err("Unable to open file for reading %s, error: %d\n", qPrintable(fileName), f.error());
      return false;
   }

   const int maxLineLen = 4096;
  
   QByteArray buf;
   buf.resize(maxLineLen);

   while (! f.atEnd() && ! found) {
      int numBytes = f.readLine(buf.data(), maxLineLen - 1); 

      if (numBytes > 0) {
         buf.resize(numBytes);

         if (qstrncmp(buf, "<!--zoomable ", 13) == 0) {

            *width  = -1;
            *height = -1;
            sscanf(buf.constData(), "<!--zoomable %d", height);
            
            found = true;

         } else if (sscanf(buf, "<svg width=\"%dpt\" height=\"%dpt\"", width, height) == 2) {
            found = true;
         }

      } else {
         // read error
 
         return false;
      }
   }

   return true;
}

static void writeSVGNotSupported(QTextStream &out)
{
   out << "<p><b>This browser is not able to show SVG: try Firefox, Chrome, Safari, or Opera instead.</b></p>";
}

// check if a reference to a SVG figure can be written and does so if possible.
// return false if not possible (for instance because the SVG file is not yet generated).

static bool writeSVGFigureLink(QTextStream &out, const QString &relPath, const QString &baseName, const QString &absImgName)
{
   int width  = 600;
   int height = 600;

   if (! readSVGSize(absImgName, &width, &height)) {
      return false;
   }

   if (width == -1) {
      if (height <= 60) {
         height = 300;
      } else {
         height += 300;   // add some extra space for zooming
      }
      if (height > 600) {
         height = 600;   // clip to maximum height of 600 pixels
      }

      out << "<div class=\"zoom\">";
      //out << "<object type=\"image/svg+xml\" data=\""
      //out << "<embed type=\"image/svg+xml\" src=\""
      out << "<iframe scrolling=\"no\" frameborder=\"0\" src=\""
          << relPath << baseName << ".svg\" width=\"100%\" height=\"" << height << "\">";

   } else {
      //out << "<object type=\"image/svg+xml\" data=\""
      //out << "<embed type=\"image/svg+xml\" src=\""
      out << "<iframe scrolling=\"no\" frameborder=\"0\" src=\""
          << relPath << baseName << ".svg\" width=\""
          << ((width * 96 + 48) / 72) << "\" height=\""
          << ((height * 96 + 48) / 72) << "\">";
   }

   writeSVGNotSupported(out);

   //out << "</object>";
   //out << "</embed>";

   out << "</iframe>";

   if (width == -1) {
      out << "</div>";
   }

   return true;
}

// since dot silently reproduces the input file when it does not
// support the PNG format, we need to check the result.
static void checkDotResult(const QString &imgName)
{
   if (Config::getEnum("dot-image-format") == "png") {
      FILE *f = fopen(imgName.toUtf8(), "rb");

      if (f) {
         char data[4];

         if (fread(data, 1, 4, f) == 4) {

            if (! (data[1] == 'P' && data[2] == 'N' && data[3] == 'G')) {
               err("Image `%s' produced by dot is not a valid PNG formate\n"
                   "Select a different format in 'DOT IMAGE FORMAT' or install a more recent version of graphviz (1.7+)\n", qPrintable(imgName));
            }

         } else {
            err("Unable to read image `%s' generated by dot\n", qPrintable(imgName));
         }

         fclose(f);

      } else {
         err("Unable to open image `%s' generated by dot\n", qPrintable(imgName));
      }
   }
}

static bool insertMapFile(QTextStream &out, const QString &mapFile, const QString &relPath, const QString &mapLabel)
{
   QFileInfo fi(mapFile);

   if (fi.exists() && fi.size() > 0) { 
      // reuse existing map file

      QString tmpstr;
      QTextStream tmpout(&tmpstr);

      convertMapFile(tmpout, mapFile, relPath);
      if (!tmpstr.isEmpty()) {
         out << "<map name=\"" << mapLabel << "\" id=\"" << mapLabel << "\">" << endl;
         out << tmpstr;
         out << "</map>" << endl;
      }

      return true;
   }

   return false; // no map file yet, need to generate it
}

static void removeDotGraph(const QString &dotName)
{
   static bool dotCleanUp = Config::getBool("dot-cleanup");

   if (dotCleanUp) {            
      QDir d;
      d.remove(dotName);
   }
}


/*! Checks if a file "baseName".md5 exists. If so the contents
 *  are compared with \a md5. If equal false is returned. If the .md5
 *  file does not exist or its contents are not equal to \a md5,
 *  a new .md5 is generated with the \a md5 string as contents.
 */
static bool checkAndUpdateMd5Signature(const QString &baseName, const QString &md5)
{
   QFile f(baseName + ".md5");

   if (f.open(QIODevice::ReadOnly)) {
      // read checksum
      QByteArray md5stored = f.readAll();
      
      // compare checksum
      if (md5 == md5stored) {
         // bail out if equal
         f.close();
         return false;
      }
   }

   f.close();

   // create checksum file
   if (f.open(QIODevice::WriteOnly)) {
      f.write(md5.toUtf8());
      f.close();
   }

   return true;
}

static bool checkDeliverables(const QString &file1, const QString &file2 = QString())
{
   bool file1Ok = true;
   bool file2Ok = true;

   if (! file1.isEmpty()) {
      QFileInfo fi(file1);
      file1Ok = (fi.exists() && fi.size() > 0);
   }

   if (! file2.isEmpty()) {
      QFileInfo fi(file2);
      file2Ok = (fi.exists() && fi.size() > 0);
   }
   return file1Ok && file2Ok;
}

DotRunner::DotRunner(const QString &file, const QString &path, bool checkResult, const QString &imageName)
   : m_file(file), m_path(path), m_checkResult(checkResult), m_imageName(imageName)
{
   static bool dotCleanUp = Config::getBool("dot-cleanup");
   m_cleanUp = dotCleanUp; 
}

void DotRunner::addJob(const QString &format, const QString &output)
{
   QString args = "-T" + format + " -o \"" + output + "\"";   
   m_jobs.append(args);
}

void DotRunner::addPostProcessing(const char *cmd, const char *args)
{
   m_postCmd  = cmd;
   m_postArgs = args;
}

bool DotRunner::run()
{   
   static const QString dotExe    = Config::getString("dot-path") + "dot";
   static const bool multiTargets = Config::getBool("dot-multiple-targets");

   int exitCode      = 0;
 
   QString dotArgs;
     
   QString file      = m_file;
   QString path      = m_path;
   QString imageName = m_imageName;
   QString postCmd   = m_postCmd;
   QString postArgs  = m_postArgs;

   bool checkResult   = m_checkResult;
   bool cleanUp       = m_cleanUp;

   if (multiTargets) {
      dotArgs = "\"" + file + "\"";
     
      for (auto s : m_jobs) {
         dotArgs += ' ';
         dotArgs += s;
      }

      if ((exitCode = portable_system(dotExe, dotArgs, false)) != 0) {
         goto error;
      }

   } else {
      for (auto s : m_jobs) {
         dotArgs = "\"" + file + "\" " + s;

         if ((exitCode = portable_system(dotExe, dotArgs, false)) != 0) {
            goto error;
         }
      }
   }

   if (! postCmd.isEmpty() && portable_system(postCmd, postArgs) != 0) {
      err("Unable to run '%s' as a post-processing step for dot output\n", csPrintable(m_postCmd));
      return false;
   }

   if (checkResult) {
      checkDotResult(imageName);
   }

   if (cleanUp) {     
      m_cleanupItem.file = file;
      m_cleanupItem.path = path;
   }

   return true;

error:
   err("Unable to run Dot, exit code=%d, command='%s', arguments='%s'\n", exitCode, qPrintable(dotExe), qPrintable(dotArgs));

   return false;
}

DotFilePatcher::DotFilePatcher(const QString &patchFile)
   : m_patchFile(patchFile)
{
}

QString DotFilePatcher::file() const
{
   return m_patchFile;
}

int DotFilePatcher::addMap(const QString &mapFile, const QString &relPath, bool urlOnly, const QString &context, const QString &label)
{
   int id = m_maps.count();

   Map map;
   map.mapFile  = mapFile;
   map.relPath  = relPath;
   map.urlOnly  = urlOnly;
   map.context  = context;
   map.label    = label;
   map.zoomable = false;
   map.graphId  = -1;

   m_maps.append(map);

   return id;
}

int DotFilePatcher::addFigure(const QString &baseName, const QString &figureName, bool heightCheck)
{
   int id = m_maps.count();

   Map map;
   map.mapFile  = figureName;
   map.urlOnly  = heightCheck;
   map.label    = baseName;
   map.zoomable = false;
   map.graphId  = -1;

   m_maps.append(map);

   return id;
}

int DotFilePatcher::addSVGConversion(const QString &relPath, bool urlOnly, const QString &context, bool zoomable, int graphId)
{
   int id = m_maps.count();
   Map map;

   map.relPath  = relPath;
   map.urlOnly  = urlOnly;
   map.context  = context;
   map.zoomable = zoomable;
   map.graphId  = graphId;

   m_maps.append(map);

   return id;
}

int DotFilePatcher::addSVGObject(const QString &baseName, const QString &absImgName, const QString &relPath)
{
   int id = m_maps.count();

   Map map;
   map.mapFile  = absImgName;
   map.relPath  = relPath;
   map.label    = baseName;
   map.zoomable = false;
   map.graphId  = -1;

   m_maps.append(map);

   return id;
}

bool DotFilePatcher::run()
{  
   static bool interactiveSVG = Config::getBool("interactive-svg");
   bool isSVGFile = (m_patchFile.right(4) == ".svg");
   int graphId = -1;

   QString relPath;
 
   if (isSVGFile) {
      Map map = m_maps[0]; 

      // there is only one 'map' for a SVG file
      interactiveSVG = interactiveSVG && map.zoomable;
      graphId = map.graphId;
      relPath = map.relPath;     
   }

   QString tmpName   = m_patchFile + ".tmp";   
   QString patchFile = m_patchFile;

   if (! QDir::current().rename(patchFile, tmpName)) {
      err("Unable to rename file %s to %s\n", qPrintable(m_patchFile), qPrintable(tmpName));
      return false;
   }

   QFile fi(tmpName);
   QFile fo(patchFile);

   if (! fi.open(QIODevice::ReadOnly)) {
      err("Unable to open file for updating %s, error: %d\n", qPrintable(tmpName), fi.error());

      QDir::current().rename(tmpName, patchFile);
      return false;
   }

   if (! fo.open(QIODevice::WriteOnly)) {     
      err("Unable to open file for updating %s, error: %d\n", qPrintable(m_patchFile), fo.error());

      QDir::current().rename(tmpName, patchFile);
      return false;
   }

   QTextStream t(&fo);
   const int maxLineLen = 100 * 1024;
   int lineNr = 1;
   int width;
   int height;

   bool insideHeader   = false;
   bool replacedHeader = false;
   bool foundSize      = false;

   while (! fi.atEnd()) { 
      // foreach line

      QByteArray line;
      line.resize(maxLineLen);

      int numBytes = fi.readLine(line.data(), maxLineLen);

      if (numBytes <= 0) {
         break;
      }
      
      line.resize(numBytes);

      int i;
      assert(numBytes < maxLineLen);

      if (isSVGFile) {
         if (interactiveSVG) {

            if (line.indexOf("<svg") != -1 && ! replacedHeader) {
               int count;

               count = sscanf(line.constData(), "<svg width=\"%dpt\" height=\"%dpt\"", &width, &height);
               
               foundSize = count == 2 && (width > 500 || height > 450);
               if (foundSize) {
                  insideHeader = true;
               }
 
           } else if (insideHeader && !replacedHeader && line.indexOf("<title>") != -1) {
               if (foundSize) {
                  // insert special replacement header for interactive SVGs
                  t << "<!--zoomable " << height << " -->\n";
                  t << svgZoomHeader;
                  t << "var viewWidth = " << width << ";\n";
                  t << "var viewHeight = " << height << ";\n";
                  if (graphId >= 0) {
                     t << "var sectionId = 'dynsection-" << graphId << "';\n";
                  }
                  t << "</script>\n";
                  t << "<script xlink:href=\"" << relPath << "svgpan.js\"/>\n";
                  t << "<svg id=\"graph\" class=\"graph\">\n";
                  t << "<g id=\"viewport\">\n";
               }
               insideHeader = false;
               replacedHeader = true;
            }
         }

         if (! insideHeader || ! foundSize) {
            // copy SVG and replace refs, 
            // unless we are inside the header of the SVG.
            // Then we replace it with another header.
         
            Map map = m_maps[0]; // there is only one 'map' for a SVG file
            t << replaceRef(line, map.relPath, map.urlOnly, map.context, "_top");
         }

      } else if ((i = line.indexOf("<!-- SVG")) != -1 || (i = line.indexOf("[!-- SVG")) != -1) {
         //printf("Found marker at %d\n",i);
         int mapId = -1;
         t << line.left(i);

         int n = sscanf(line.data() + i + 1, "!-- SVG %d", &mapId);

         if (n == 1 && mapId >= 0 && mapId < m_maps.count()) {
            int e   = qMax(line.indexOf("--]"), line.indexOf("-->"));
            Map map = m_maps[mapId];

            if (! writeSVGFigureLink(t, map.relPath, map.label, map.mapFile)) {
               err("Unable to extract size from SVG file %s\n", csPrintable(map.mapFile));
            }
            if (e != -1) {
               t << line.mid(e + 3);
            }

         } else { 
            err("Invalid SVG id found in file %s\n", qPrintable(m_patchFile));
            t << line.mid(i);
         }

      } else if ((i = line.indexOf("<!-- MAP")) != -1) {
         int mapId = -1;
         t << line.left(i);
         int n = sscanf(line.data() + i, "<!-- MAP %d", &mapId);

         if (n == 1 && mapId >= 0 && mapId < m_maps.count()) {
            Map map = m_maps[mapId];
         
            t << "<map name=\"" << map.label << "\" id=\"" << map.label << "\">" << endl;
            convertMapFile(t, map.mapFile, map.relPath, map.urlOnly, map.context);
            t << "</map>" << endl;

         } else { // error invalid map id!
            err("Invalid MAP id found in file %s\n", qPrintable(m_patchFile));
            t << line.mid(i);
         }

      } else if ((i = line.indexOf("% FIG")) != -1) {
         int mapId = -1;
         int n = sscanf(line.data() + i + 2, "FIG %d", &mapId);
      
         if (n == 1 && mapId >= 0 && mapId < m_maps.count()) {
            Map map = m_maps[mapId];

            if (! writeVecGfxFigure(t, map.label, map.mapFile)) {
               err("Unable to write FIG %d figure\n", mapId);
               return false;
            }

         } else { // error invalid map id!
            err("Invalid bounding FIG %d in file %s\n", mapId, qPrintable(m_patchFile));
            t << line;
         }

      } else {
         t << line;
      }

      lineNr++;
   }

   fi.close();

   if (isSVGFile && interactiveSVG && replacedHeader) {
      QString orgName = m_patchFile.left(m_patchFile.length() - 4) + "_org.svg";
      
      t << substitute(svgZoomFooter, "$orgname", stripPath(orgName));
      fo.close();

      // keep original SVG file so we can refer to it, we do need to replace
      // dummy link by real ones
      QFile fi(tmpName);
      QFile fo(orgName);

      if (! fi.open(QIODevice::ReadOnly)) {
         err("Unable to open file for reading %s, error: %d\n", qPrintable(tmpName), fi.error());       
         return false;
      }

      if (! fo.open(QIODevice::WriteOnly)) {
         err("Unable to open file for writing %s, error: %d\n", qPrintable(orgName), fi.error());
         return false;
      }

      QTextStream t(&fo);

      while (! fi.atEnd()) { // foreach line
         QByteArray line;
         line.resize(maxLineLen);

         int numBytes = fi.readLine(line.data(), maxLineLen);

         if (numBytes <= 0) {
            break;
         }

         Map map = m_maps[0]; // there is only one 'map' for a SVG file
         t << replaceRef(line, map.relPath, map.urlOnly, map.context, "_top");
      }

      fi.close();
      fo.close();
   }

   // remove temporary file
   QDir::current().remove(tmpName);

   return true;
}
void DotRunnerQueue::enqueue(DotRunner *runner)
{
   QMutexLocker locker(&m_mutex);

   m_queue.enqueue(runner);
   m_bufferNotEmpty.wakeAll();
}

DotRunner *DotRunnerQueue::dequeue()
{
   QMutexLocker locker(&m_mutex);

   while (m_queue.isEmpty()) {
      // wait until something is added to the queue
      m_bufferNotEmpty.wait(&m_mutex);
   }

   DotRunner *result = m_queue.dequeue();

   return result;
}

uint DotRunnerQueue::count() const
{
   QMutexLocker locker(&m_mutex);
   return m_queue.count();
}


DotWorkerThread::DotWorkerThread(DotRunnerQueue *queue)
   : m_queue(queue)
{
}

void DotWorkerThread::run()
{
   DotRunner *runner;

   while ((runner = m_queue->dequeue())) {
      runner->run();
      DotRunner::CleanupItem cleanup = runner->cleanup();

      if (!cleanup.file.isEmpty()) {
         m_cleanupItems.append(cleanup);
      }
   }
}

void DotWorkerThread::cleanup()
{
   for (auto ci : m_cleanupItems) {
      QDir(ci.path).remove(ci.file);
   }
}

DotManager *DotManager::m_theInstance = 0;

DotManager *DotManager::instance()
{
   if (!m_theInstance) {
      m_theInstance = new DotManager;
   }
   return m_theInstance;
}

DotManager::DotManager() 
{
   m_queue = new DotRunnerQueue;

   int i;
   int numThreads = qMin(32, Config::getInt("dot-num-threads"));

   if (numThreads != 1) {
      if (numThreads == 0) {
         numThreads = qMax(2, QThread::idealThreadCount() + 1);
      }

      for (i = 0; i < numThreads; i++) {
         DotWorkerThread *thread = new DotWorkerThread(m_queue);
         thread->start();

         if (thread->isRunning()) {
            m_workers.append(thread);
         } else { // no more threads available!
            delete thread;
         }
      }
      assert(m_workers.count() > 0);
   }
}

DotManager::~DotManager()
{
   delete m_queue;
}

void DotManager::addRun(DotRunner *run)
{
   m_dotRuns.append(run);
}

int DotManager::addMap(const QString &file, const QString &mapFile,
                       const QString &relPath, bool urlOnly, const QString &context, const QString &label)
{
   QSharedPointer<DotFilePatcher> map = m_dotMaps.find(file);

   if (! map) {
      map = QMakeShared<DotFilePatcher>(DotFilePatcher(file));
      m_dotMaps.insert(file, map);
   }

   return map->addMap(mapFile, relPath, urlOnly, context, label);
}

int DotManager::addFigure(const QString &file, const QString &baseName, const QString &figureName, bool heightCheck)
{
  QSharedPointer<DotFilePatcher> map = m_dotMaps.find(file);

   if (! map) {
      map = QMakeShared<DotFilePatcher>(DotFilePatcher(file));
      m_dotMaps.insert(file, map);
   }

   return map->addFigure(baseName, figureName, heightCheck);
}

int DotManager::addSVGConversion(const QString &file, const QString &relPath,
                                 bool urlOnly, const QString &context, bool zoomable,int graphId)
{
   QSharedPointer<DotFilePatcher> map = m_dotMaps.find(file);

   if (! map) {
      map = QMakeShared<DotFilePatcher>(DotFilePatcher(file));
      m_dotMaps.insert(file, map);
   }

   return map->addSVGConversion(relPath, urlOnly, context, zoomable, graphId);
}

int DotManager::addSVGObject(const QString &file, const QString &baseName, const QString &absImgName, const QString &relPath)
{
   QSharedPointer<DotFilePatcher> map = m_dotMaps.find(file);

   if (! map) {
      map = QMakeShared<DotFilePatcher>(DotFilePatcher(file));
      m_dotMaps.insert(file, map);
   }

   return map->addSVGObject(baseName, absImgName, relPath);
}

bool DotManager::run()
{
   uint numDotRuns = m_dotRuns.count();
   uint numDotMaps = m_dotMaps.count();

   if (numDotRuns + numDotMaps > 1) {
      if (m_workers.count() == 0) {
         msg("Generating dot graphs in single threaded mode...\n");

      } else {
         msg("Generating dot graphs using %d parallel threads...\n", qMin(numDotRuns + numDotMaps, (uint)m_workers.count()));
      }
   }

   int i = 1;
   bool setPath = false;

   if (Config::getBool("generate-html")) {
      setDotFontPath(Config::getString("html-output"));
      setPath = true;

   } else if (Config::getBool("generate-latex")) {
      setDotFontPath(Config::getString("latex-output"));
      setPath = true;

   } else if (Config::getBool("generate-rtf")) {
      setDotFontPath(Config::getString("rtf-output"));
      setPath = true;
   }

   portable_sysTimerStart();

   // fill work queue with dot operations
   DotRunner *dr;
   int prev = 1;
     
   if (m_workers.count() == 0) { // no threads to work with

      for (auto dr : m_dotRuns) {
         msg("Running dot for graph %d/%d\n", prev, numDotRuns);
         dr->run();
         prev++;
      }

   } else { // use multiple threads to run instances of dot in parallel
      for (auto dr : m_dotRuns) {
         m_queue->enqueue(dr);
      }

      // wait for the queue to become empty
      while ((i = m_queue->count()) > 0) {
         i = numDotRuns - i;

         while (i >= prev) {
            msg("Running dot for graph %d/%d\n", prev, numDotRuns);
            prev++;
         }
         portable_sleep(100);
      }

      while (numDotRuns >= prev) {
         msg("Running dot for graph %d/%d\n", prev, numDotRuns);
         prev++;
      }

      // signal the workers we are done
      for (i = 0; i < m_workers.count(); i++) {
         m_queue->enqueue(0); // add terminator for each worker
      }

      // wait for the workers to finish
      for (i = 0; i < m_workers.count(); i++) {
         m_workers.at(i)->wait();
      }

      // clean up dot files from main thread
      for (i = 0; i < m_workers.count(); i++) {
         m_workers.at(i)->cleanup();
      }
   }
   portable_sysTimerStop();
   if (setPath) {
      unsetDotFontPath();
   }

   // patch the output file and insert the maps and figures
   i = 1;

   // since patching the svg files may involve patching the header of the SVG
   // (for zoomable SVGs), and patching the .html files requires reading that
   // header after the SVG is patched, we first process the .svg files and
   // then the other files.
   
   for (auto mapItem : m_dotMaps) {
      if (mapItem->file().right(4) == ".svg") {
         msg("Patching output file %d/%d\n", i, numDotMaps);

         if (! mapItem->run()) {
            return false;
         }
         i++;
      }
   }

   for (auto mapItem : m_dotMaps) {
      if (mapItem->file().right(4) != ".svg") {
         msg("Patching output file %d/%d\n", i, numDotMaps);

         if (! mapItem->run()) {
            return false;
         }
         i++;
      }
   }

   return true;
}

/*! helper function that deletes all nodes in a connected graph, given
 *  one of the graph's nodes
 */
static void deleteNodes(DotNode *node, StringMap<QSharedPointer<DotNode>> *skipNodes = 0)
{
   //printf("deleteNodes skipNodes=%p\n",skipNodes);
   static SortedList<DotNode *> deletedNodes;
   
   node->deleteNode(deletedNodes, skipNodes); // collect nodes to be deleted.
   deletedNodes.clear(); // actually remove the nodes.
}

DotNode::DotNode(int n, const QString &label, const QString &tip, const QString &url, bool isRoot, QSharedPointer<ClassDef> cd)
   : m_subgraphId(-1), m_number(n), m_label(label), m_tooltip(tip), m_url(url), m_parents(0), m_children(0),
     m_edgeInfo(0), m_deleted(false), m_written(false), m_hasDoc(false), m_isRoot(isRoot), m_classDef(cd),
     m_visible(false), m_truncated(Unknown), m_distance(1000)
{
}

DotNode::~DotNode()
{
   delete m_children;
   delete m_parents;
   delete m_edgeInfo;
}

void DotNode::addChild(DotNode *n, int edgeColor, int edgeStyle, const QString &edgeLab, const char *edgeURL, int edgeLabCol )
{
   if (m_children == 0) {
      m_children = new QList<DotNode *>;
      m_edgeInfo = new QList<EdgeInfo *>;
   }

   m_children->append(n);

   EdgeInfo *ei = new EdgeInfo;
   ei->m_color = edgeColor;
   ei->m_style = edgeStyle;
   ei->m_label = edgeLab;
   ei->m_url   = edgeURL;

   if (edgeLabCol == -1) {
      ei->m_labColor = edgeColor;
   } else {
      ei->m_labColor = edgeLabCol;
   }

   m_edgeInfo->append(ei);
}

void DotNode::addParent(DotNode *n)
{
   if (m_parents == 0) {
      m_parents = new QList<DotNode *>;
   }
   m_parents->append(n);
}

void DotNode::removeChild(DotNode *n)
{
   if (m_children) {
      m_children->removeOne(n);
   }
}

void DotNode::removeParent(DotNode *n)
{
   if (m_parents) {
      m_parents->removeOne(n);
   }
}

void DotNode::deleteNode(SortedList<DotNode *> &deletedList, StringMap<QSharedPointer<DotNode>> *skipNodes)
{
   if (m_deleted) {
      return;   // avoid recursive loops in case the graph has cycles
   }

   m_deleted = true;

   if (m_parents != 0) { 
      // delete all parent nodes of this node
    
      for (auto pn : *m_parents) {  
         //pn->removeChild(this);
         pn->deleteNode(deletedList, skipNodes);
      }
   }

   if (m_children != 0) { // delete all child nodes of this node    
      for (auto cn : *m_children) {       
         cn->deleteNode(deletedList, skipNodes);
      }
   }

   // add this node to the list of deleted nodes.
   if (skipNodes == 0 || skipNodes->find((char *)this) == 0) {      
      deletedList.append(this);
   }
}

void DotNode::setDistance(int distance)
{
   if (distance < m_distance) {
      m_distance = distance;
   }
}

static QString convertLabel(const QString &label)
{
   QString result = "";

   if (label.isEmpty()) {
      return result;
   }

   QString bBefore("\\_/<({[: =-+@%#~?$"); // break before character set
   QString bAfter(">]),:;|");              // break after  character set

   const QChar *p = label.constData();
 
   QChar c;
   QChar pc = 0;
 
   int len = label.length();

   int charsLeft = len;
   int sinceLast = 0;
   int foldLen   = 17; // ideal text length

   while ((c = *p++) != 0) {
      QString replacement;

      switch (c.unicode()) {
         case '\\':
            replacement = "\\\\";
            break;
         case '\n':
            replacement = "\\n";
            break;
         case '<':
            replacement = "\\<";
            break;
         case '>':
            replacement = "\\>";
            break;
         case '|':
            replacement = "\\|";
            break;
         case '{':
            replacement = "\\{";
            break;
         case '}':
            replacement = "\\}";
            break;
         case '"':
            replacement = "\\\"";
            break;
         default:
            replacement = c;
            break;
      }

      // Some heuristics to insert newlines to prevent too long
      // boxes and at the same time prevent ugly breaks

      if (c == '\n') {
         result += replacement;
         foldLen = (3 * foldLen + sinceLast + 2) / 4;
         sinceLast = 1;

      } else if ((pc != ':' || c != ':') && charsLeft > foldLen / 3 && sinceLast > foldLen && bBefore.contains(c)) {
         result += "\\l";
         result += replacement;
         foldLen = (foldLen + sinceLast + 1) / 2;
         sinceLast = 1;

      } else if (charsLeft > 1 + foldLen / 4 && sinceLast > foldLen + foldLen / 3 && ! c.isUpper() && p->isUpper()) {
         result += replacement;
         result += "\\l";
         foldLen = (foldLen + sinceLast + 1) / 2;
         sinceLast = 0;

      } else if (charsLeft > foldLen / 3 && sinceLast > foldLen && bAfter.contains(c) && (c != ':' || *p != ':')) {
         result += replacement;
         result += "\\l";
         foldLen = (foldLen + sinceLast + 1) / 2;
         sinceLast = 0;

      } else {
         result += replacement;
         sinceLast++;
      }

      charsLeft--;
      pc = c;
   }

   return result;
}

static QString escapeTooltip(const QString &tooltip)
{
   if (tooltip.isEmpty()) {
      return "";
   }

   QString result = tooltip;
   result.replace("\"", "\\\""); 

   return result;
}

static void writeBoxMemberList(QTextStream &t, char prot, QSharedPointer<MemberList> ml, QSharedPointer<ClassDef> scope,
                               bool isStatic = false, const QSet<QString> *skipNames = 0)
{   
   if (ml) {
      int totalCount = 0;
      
      for (auto mma : *ml) {
         if (mma->getClassDef() == scope && (skipNames == 0 || ! skipNames->contains(mma->name()))) {
            totalCount++;
         }
      }

      int count = 0;

      for (auto mma : *ml) {
         if (mma->getClassDef() == scope && (skipNames == 0 || ! skipNames->contains(mma->name()))) {

            static int limit = Config::getInt("uml-limit-num-fields");

            if (limit > 0 && (totalCount > limit * 3 / 2 && count >= limit)) {

               QString temp;
               temp = QString("%1").arg(totalCount - count);

               t << theTranslator->trAndMore(qPrintable(temp)) << "\\l";

               break;

            } else {
               t << prot << " ";
               t << convertLabel(mma->name());

               if (!mma->isObjCMethod() && (mma->isFunction() || mma->isSlot() || mma->isSignal())) {
                  t << "()";
               }

               t << "\\l";
               count++;
            }
         }
      }

      // write member groups within the memberlist
      QList<MemberGroup> *mgl = ml->getMemberGroupList();

      if (mgl) {       
         for (auto mg : *mgl) {
            if (mg.members()) {
               writeBoxMemberList(t, prot, mg.members(), scope, isStatic, skipNames);
            }
         }
      }
   }
}

static QString stripProtectionPrefix(const QString &s)
{
   if (! s.isEmpty() && (s[0] == '-' || s[0] == '+' || s[0] == '~' || s[0] == '#')) {
      return s.mid(1);

   } else {
      return s;
   }

}

void DotNode::writeBox(QTextStream &t, GraphType gt, GraphOutputFormat, bool hasNonReachableChildren, bool reNumber)
{
   const char *labCol = m_url.isEmpty() ? "grey75" :  // non link
      ( (hasNonReachableChildren) ? "red" : "black");

   t << "  Node" << reNumberNode(m_number, reNumber) << " [label=\"";
   static bool umlLook = Config::getBool("uml-look");

   if (m_classDef && umlLook && (gt == Inheritance || gt == Collaboration)) {
      // add names shown as relations to a dictionary, so we do not show them as attributes as well
      QSet<QString> arrowNames;

      if (m_edgeInfo) {
         // for each edge
         
         for (auto ei : *m_edgeInfo) {  
            if (! ei->m_label.isEmpty()) { // labels joined by \n
               int li = ei->m_label.indexOf('\n');
               int p = 0;

               QString label;

               while ((li = ei->m_label.indexOf('\n', p)) != -1) {
                  label = stripProtectionPrefix(ei->m_label.mid(p, li - p));

                  arrowNames.insert(label);
                  p = li + 1;
               }

               label = stripProtectionPrefix(ei->m_label.right(ei->m_label.length() - p));
               arrowNames.insert(label);
            }
         }
      }

      static bool extractPrivate = Config::getBool("extract-private");

      t << "{" << convertLabel(m_label);
      t << "\\n|";

      writeBoxMemberList(t, '+', m_classDef->getMemberList(MemberListType_pubAttribs), m_classDef, false, &arrowNames);
      writeBoxMemberList(t, '+', m_classDef->getMemberList(MemberListType_pubStaticAttribs), m_classDef, true, &arrowNames);
      writeBoxMemberList(t, '+', m_classDef->getMemberList(MemberListType_properties), m_classDef, false, &arrowNames);
      writeBoxMemberList(t, '~', m_classDef->getMemberList(MemberListType_pacAttribs), m_classDef, false, &arrowNames);
      writeBoxMemberList(t, '~', m_classDef->getMemberList(MemberListType_pacStaticAttribs), m_classDef, true, &arrowNames);
      writeBoxMemberList(t, '#', m_classDef->getMemberList(MemberListType_proAttribs), m_classDef, false, &arrowNames);
      writeBoxMemberList(t, '#', m_classDef->getMemberList(MemberListType_proStaticAttribs), m_classDef, true, &arrowNames);

      if (extractPrivate) {
         writeBoxMemberList(t, '-', m_classDef->getMemberList(MemberListType_priAttribs), m_classDef, false, &arrowNames);
         writeBoxMemberList(t, '-', m_classDef->getMemberList(MemberListType_priStaticAttribs), m_classDef, true, &arrowNames);
      }

      t << "|";
      writeBoxMemberList(t, '+', m_classDef->getMemberList(MemberListType_pubMethods), m_classDef);
      writeBoxMemberList(t, '+', m_classDef->getMemberList(MemberListType_pubStaticMethods), m_classDef, true);
      writeBoxMemberList(t, '+', m_classDef->getMemberList(MemberListType_pubSlots), m_classDef);
      writeBoxMemberList(t, '~', m_classDef->getMemberList(MemberListType_pacMethods), m_classDef);
      writeBoxMemberList(t, '~', m_classDef->getMemberList(MemberListType_pacStaticMethods), m_classDef, true);
      writeBoxMemberList(t, '#', m_classDef->getMemberList(MemberListType_proMethods), m_classDef);
      writeBoxMemberList(t, '#', m_classDef->getMemberList(MemberListType_proStaticMethods), m_classDef, true);
      writeBoxMemberList(t, '#', m_classDef->getMemberList(MemberListType_proSlots), m_classDef);

      if (extractPrivate) {
         writeBoxMemberList(t, '-', m_classDef->getMemberList(MemberListType_priMethods), m_classDef);
         writeBoxMemberList(t, '-', m_classDef->getMemberList(MemberListType_priStaticMethods), m_classDef, true);
         writeBoxMemberList(t, '-', m_classDef->getMemberList(MemberListType_priSlots), m_classDef);
      }

      if (m_classDef->getLanguage() != SrcLangExt_Fortran && m_classDef->getMemberGroupSDict()) {
       
         for (auto mg : *m_classDef->getMemberGroupSDict() ) {
            if (mg->members()) {
               writeBoxMemberList(t, '*', mg->members(), m_classDef, false, &arrowNames);
            }
         }
      }
      t << "}";

   } else { // standard look
      t << convertLabel(m_label);
   }

   t << "\",height=0.2,width=0.4";

   if (m_isRoot) {
      t << ",color=\"black\", fillcolor=\"grey75\", style=\"filled\", fontcolor=\"black\"";
   } else {
      static bool dotTransparent = Config::getBool("dot-transparent");

      if (!dotTransparent) {
         t << ",color=\"" << labCol << "\", fillcolor=\"";
         t << "white";
         t << "\", style=\"filled\"";
      } else {
         t << ",color=\"" << labCol << "\"";
      }

      if (! m_url.isEmpty()) {
         int anchorPos = m_url.lastIndexOf('#');

         if (anchorPos == -1) {
            t << ",URL=\"" << m_url << Doxy_Globals::htmlFileExtension << "\"";

         } else {
            t << ",URL=\"" << m_url.left(anchorPos) << Doxy_Globals::htmlFileExtension
              << m_url.right(m_url.length() - anchorPos) << "\"";
         }
      }
      if (!m_tooltip.isEmpty()) {
         t << ",tooltip=\"" << escapeTooltip(m_tooltip) << "\"";
      }
   }
   t << "];" << endl;
}

void DotNode::writeArrow(QTextStream &t, GraphType gt, GraphOutputFormat format, DotNode *cn, EdgeInfo *ei,
                         bool topDown, bool pointBack, bool reNumber)
{
   t << "  Node";
   if (topDown) {
      t << reNumberNode(cn->number(), reNumber);
   } else {
      t << reNumberNode(m_number, reNumber);
   }
   t << " -> Node";
   if (topDown) {
      t << reNumberNode(m_number, reNumber);
   } else {
      t << reNumberNode(cn->number(), reNumber);
   }
   t << " [";

   static bool umlLook = Config::getBool("uml-look");

   const EdgeProperties *eProps = umlLook ? &umlEdgeProps : &normalEdgeProps;
   QString aStyle = eProps->arrowStyleMap[ei->m_color];
   bool umlUseArrow = aStyle == "odiamond";

   if (pointBack && !umlUseArrow) {
      t << "dir=\"back\",";
   }
   t << "color=\"" << eProps->edgeColorMap[ei->m_color]
     << "\",fontsize=\"" << FONTSIZE << "\",";
   t << "style=\"" << eProps->edgeStyleMap[ei->m_style] << "\"";
   if (!ei->m_label.isEmpty()) {
      t << ",label=\" " << convertLabel(ei->m_label) << "\" ";
   }
   if (umlLook &&
         eProps->arrowStyleMap[ei->m_color] &&
         (gt == Inheritance || gt == Collaboration)
      ) {
      bool rev = pointBack;
      if (umlUseArrow) {
         rev = !rev;   // UML use relates has arrow on the start side
      }
      if (rev) {
         t << ",arrowtail=\"" << eProps->arrowStyleMap[ei->m_color] << "\"";
      } else {
         t << ",arrowhead=\"" << eProps->arrowStyleMap[ei->m_color] << "\"";
      }
   }

   if (format == GOF_BITMAP) {
      t << ",fontname=\"" << FONTNAME << "\"";
   }
   t << "];" << endl;
}

void DotNode::write(QTextStream &t, GraphType gt, GraphOutputFormat format, bool topDown,
                    bool toChildren, bool backArrows, bool reNumber)
{
   if (m_written) {
      return;   // node already written to the output
   }

   if (! m_visible) {
      return;   // node is not visible
   }

   writeBox(t, gt, format, m_truncated == Truncated, reNumber);
   m_written = true;

   QList<DotNode *> *nl = toChildren ? m_children : m_parents;

   if (nl) {
      if (toChildren) {       
          auto iter = m_edgeInfo->begin();

         for (auto cn : *nl) {  
            
            if (cn->isVisible()) {               
               writeArrow(t, gt, format, cn, *iter, topDown, backArrows, reNumber);
            }

            cn->write(t, gt, format, topDown, toChildren, backArrows, reNumber);
            ++iter;
         }

      } else { 
         // render parents      

         for (auto pn : *nl) {  

            if (pn->isVisible()) {               
               writeArrow(t, gt, format, pn, pn->m_edgeInfo->at(pn->m_children->indexOf(this)), false, backArrows, reNumber );
            }

            pn->write(t, gt, format, true, false, backArrows, reNumber);
         }
      }
   }   
}

void DotNode::writeXML(QTextStream &t, bool isClassGraph)
{
   t << "      <node id=\"" << m_number << "\">" << endl;
   t << "        <label>" << convertToXML(m_label) << "</label>" << endl;

   if (! m_url.isEmpty()) {  
      int index = m_url.indexOf("$");

      if (index != -1) {
         QString refUrl = m_url.left(index - 1);
         QString url    = m_url.mid(index + 1); 

         t << "        <link refid=\"" << convertToXML(url) << "\"";

         if (! refUrl.isEmpty()) {
            t << " external=\"" << convertToXML(refUrl) << "\"";
         }

         t << "/>" << endl;
      }
   }

   if (m_children) {         
      auto edgeIter = m_edgeInfo->begin();

      for (auto childNode : *m_children) {   
         auto edgeInfo = *edgeIter;

         t << "        <childnode refid=\"" << childNode->m_number << "\" relation=\"";

         if (isClassGraph) {
            switch (edgeInfo->m_color) {

               case EdgeInfo::Blue:
                  t << "public-inheritance";
                  break;

               case EdgeInfo::Green:
                  t << "protected-inheritance";
                  break;

               case EdgeInfo::Red:
                  t << "private-inheritance";
                  break;

               case EdgeInfo::Purple:
                  t << "usage";
                  break;

               case EdgeInfo::Orange:
                  t << "template-instance";
                  break;

               case EdgeInfo::Orange2:
                  t << "type-constraint";
                  break;

               case EdgeInfo::Grey:
                  assert(0);
                  break;
            }

         } else { // include graph
            t << "include";

         }

         t << "\">" << endl;

         if (!edgeInfo->m_label.isEmpty()) {
            int p = 0;
            int ni;

            while ((ni = edgeInfo->m_label.indexOf('\n', p)) != -1) {
               t << "          <edgelabel>"
                 << convertToXML(edgeInfo->m_label.mid(p, ni - p))
                 << "</edgelabel>" << endl;

               p = ni + 1;
            }

            t << "          <edgelabel>"
              << convertToXML(edgeInfo->m_label.right(edgeInfo->m_label.length() - p))
              << "</edgelabel>" << endl;
         }
         t << "        </childnode>" << endl;

         ++edgeIter;
      }
   }

   t << "      </node>" << endl;
}

void DotNode::writeDocbook(QTextStream &t, bool isClassGraph)
{
   t << "      <node id=\"" << m_number << "\">" << endl;
   t << "        <label>" << convertToXML(m_label) << "</label>" << endl;
 
   if (! m_url.isEmpty()) {
      int index = m_url.indexOf("$");

      if (index != -1) {
         QString refUrl = m_url.left(index - 1);
         QString url    = m_url.mid(index + 1); 

         t << "        <link refid=\"" << convertToXML(url) << "\"";

         if (! refUrl.isEmpty()) {
            t << " external=\"" << convertToXML(refUrl) << "\"";
         }

         t << "/>" << endl;
      }
   }

   if (m_children) {    
      auto edgeIter = m_edgeInfo->begin();

      for (auto childNode : *m_children) {   
         auto edgeInfo = *edgeIter;

         t << "        <childnode refid=\"" << childNode->m_number << "\" relation=\"";

         if (isClassGraph) {
            switch (edgeInfo->m_color) {
               case EdgeInfo::Blue:
                  t << "public-inheritance";
                  break;

               case EdgeInfo::Green:
                  t << "protected-inheritance";
                  break;

               case EdgeInfo::Red:
                  t << "private-inheritance";
                  break;

               case EdgeInfo::Purple:
                  t << "usage";
                  break;
               case EdgeInfo::Orange:
                  t << "template-instance";
                  break;

               case EdgeInfo::Orange2:
                  t << "type-constraint";
                  break;

               case EdgeInfo::Grey:
                  assert(0);
                  break;
            }

         } else { // include graph
            t << "include";
         }

         t << "\">" << endl;

         if (!edgeInfo->m_label.isEmpty()) {
            int p = 0;
            int ni;

            while ((ni = edgeInfo->m_label.indexOf('\n', p)) != -1) {
               t << "          <edgelabel>"
                 << convertToXML(edgeInfo->m_label.mid(p, ni - p))
                 << "</edgelabel>" << endl;
               p = ni + 1;
            }

            t << "          <edgelabel>"
              << convertToXML(edgeInfo->m_label.right(edgeInfo->m_label.length() - p))
              << "</edgelabel>" << endl;
         }

         t << "        </childnode>" << endl;

         ++edgeIter;
      }
   }

   t << "      </node>" << endl;
}


void DotNode::writeDEF(QTextStream &t)
{
   const char *nodePrefix = "        node-";

   t << "      node = {" << endl;
   t << nodePrefix << "id    = " << m_number << ';' << endl;
   t << nodePrefix << "label = '" << m_label << "';" << endl;

   if (! m_url.isEmpty()) {
      int index = m_url.indexOf("$");

      if (index != -1) {
         QString refUrl = m_url.left(index - 1);
         QString url    = m_url.mid(index + 1); 

         t << nodePrefix << "link = {" << endl << "  "
           << nodePrefix << "link-id = '" << url << "';" << endl;

         if (! refUrl.isEmpty()) {
            t << "  " << nodePrefix << "link-external = '"
              << refUrl << "';" << endl;
         }

         t << "        };" << endl;
      }
   }

   if (m_children) { 
      auto edgeIter = m_edgeInfo->begin();

      for (auto childNode : *m_children) {   
         auto edgeInfo = *edgeIter;

         t << "        node-child = {" << endl;
         t << "          child-id = '" << childNode->m_number << "';" << endl;
         t << "          relation = ";

         switch (edgeInfo->m_color) {
            case EdgeInfo::Blue:
               t << "public-inheritance";
               break;

            case EdgeInfo::Green:
               t << "protected-inheritance";
               break;

            case EdgeInfo::Red:
               t << "private-inheritance";
               break;

            case EdgeInfo::Purple:
               t << "usage";
               break;

            case EdgeInfo::Orange:
               t << "template-instance";
               break;

            case EdgeInfo::Orange2:
               t << "type-constraint";
               break;

            case EdgeInfo::Grey:
               assert(0);
               break;
         }
         t << ';' << endl;

         if (!edgeInfo->m_label.isEmpty()) {
            t << "          edgelabel = <<_EnD_oF_dEf_TeXt_" << endl
              << edgeInfo->m_label << endl
              << "_EnD_oF_dEf_TeXt_;" << endl;
         }
         t << "        }; /* node-child */" << endl;

         ++edgeIter;
      } 
   }

   t << "      }; /* node */" << endl;
}

void DotNode::clearWriteFlag()
{
   m_written = false;

   if (m_parents != 0) {   
      for (auto pn : *m_parents) {
         if (pn->m_written) {
            pn->clearWriteFlag();
         }
      }
   }
   if (m_children != 0) {
     for (auto cn : *m_children) {
         if (cn->m_written) {
            cn->clearWriteFlag();
         }
      }
   }
}

void DotNode::colorConnectedNodes(int curColor)
{
   if (m_children) {  
      for (auto cn : *m_children) {
         if (cn->m_subgraphId == -1) { // uncolored child node
            cn->m_subgraphId = curColor;
            cn->markAsVisible();
            cn->colorConnectedNodes(curColor);            
         }
      }
   }

   if (m_parents) {
      for (auto pn : *m_parents) {
         if (pn->m_subgraphId == -1) { // uncolored parent node
            pn->m_subgraphId = curColor;
            pn->markAsVisible();
            pn->colorConnectedNodes(curColor);            
         }
      }
   }
}

const DotNode *DotNode::findDocNode() const
{
   if (!m_url.isEmpty()) {
      return this;
   }

   if (m_parents) {
      for (auto pn : *m_parents) {
         if (!pn->m_hasDoc) {
            pn->m_hasDoc = true;
            const DotNode *dn = pn->findDocNode();
            if (dn) {
               return dn;
            }
         }
      }
   }
   if (m_children) {
      for (auto cn : *m_children) {
         if (! cn->m_hasDoc) {
            cn->m_hasDoc = true;
            const DotNode *dn = cn->findDocNode();
            if (dn) {
               return dn;
            }
         }
      }
   }
   return 0;
}

void DotGfxHierarchyTable::writeGraph(QTextStream &out, const QString &path, const QString &fileName) const
{
   if (m_rootSubgraphs->count() == 0) {
      return;
   }

   QDir d(path);

   // store the original directory
   if (! d.exists()) {
      err("Output dir %s does not exist\n", qPrintable(path));
      exit(1);
   }

   // put each connected subgraph of the hierarchy in a row of the HTML output
   out << "<table border=\"0\" cellspacing=\"10\" cellpadding=\"0\">" << endl;

   int count = 0;

   for (auto n : *m_rootSubgraphs) {      
      QString imgExt   = Config::getEnum("dot-image-format");

      QString baseName = QString("inherit_graph_%1").arg(count++);
     
      QString imgName  = baseName + "." + imgExt;
      QString mapName  = baseName + ".map";

      QString absImgName  = d.absolutePath() + "/" + imgName;
      QString absMapName  = d.absolutePath() + "/" + mapName;
      QString absBaseName = d.absolutePath() + "/" + baseName;

      // compute md5 checksum of the graph were are about to generate
      QString theGraph;

      QTextStream md5stream(&theGraph);
      writeGraphHeader(md5stream, theTranslator->trGraphicalHierarchy());
      md5stream << "  rankdir=\"LR\";" << endl;
   
      for (auto node : *m_rootNodes) {   
         if (node->m_subgraphId == n->m_subgraphId) {
            node->clearWriteFlag();
         }
      }
 
      for (auto node : *m_rootNodes) {   
         if (node->m_subgraphId == n->m_subgraphId) {
            node->write(md5stream, DotNode::Hierarchy, GOF_BITMAP, false, true, true, true);
         }
      }

      writeGraphFooter(md5stream);
      resetReNumbering();

      QString sigStr;
      sigStr = QCryptographicHash::hash(theGraph.toUtf8(), QCryptographicHash::Md5).toHex();
  
      bool regenerate = false;

      if (checkAndUpdateMd5Signature(absBaseName, sigStr) || ! checkDeliverables(absImgName, absMapName)) {
         regenerate = true;

         // image was new or has changed
         QString dotName = absBaseName + ".dot";

         QFile f(dotName);
         if (! f.open(QIODevice::WriteOnly)) {
            return;
         }

         QTextStream t(&f);
         t << theGraph;
         f.close();
         resetReNumbering();

         DotRunner *dotRun = new DotRunner(dotName, d.absolutePath(), true, absImgName);
         dotRun->addJob(imgExt, absImgName);
         dotRun->addJob(MAP_CMD, absMapName);
         DotManager::instance()->addRun(dotRun);

      } else {
         removeDotGraph(absBaseName + ".dot");
      }

      Doxy_Globals::indexList->addImageFile(imgName);

      // write image and map in a table row
      QString mapLabel = escapeCharsInString(n->m_label, false);
      out << "<tr><td>";

      if (imgExt == "svg") { // vector graphics
         if (regenerate || ! writeSVGFigureLink(out, QString(), baseName, absImgName)) {
            if (regenerate) {
               DotManager::instance()->addSVGConversion(absImgName, QString(), false, QString(), false, 0);
            }

            int mapId = DotManager::instance()->addSVGObject(fileName, baseName, absImgName, QString());
            out << "<!-- SVG " << mapId << " -->" << endl;
         }

      } else { // normal bitmap
         out << "<img src=\"" << imgName << "\" border=\"0\" alt=\"\" usemap=\"#"
             << mapLabel << "\"/>" << endl;

         if (regenerate || ! insertMapFile(out, absMapName, QString(), mapLabel)) {
            int mapId = DotManager::instance()->addMap(fileName, absMapName, QString(), false, QString(), mapLabel);

            out << "<!-- MAP " << mapId << " -->" << endl;
         }
      }

      out << "</td></tr>" << endl;
   }
   out << "</table>" << endl;
}

void DotGfxHierarchyTable::addHierarchy(DotNode *n, QSharedPointer<ClassDef> cd, bool hideSuper)
{
   if (cd->subClasses()) {
     
      for (auto bcd : *cd->subClasses()) {
         QSharedPointer<ClassDef> bClass = bcd->classDef;

         if (bClass->isVisibleInHierarchy() && hasVisibleRoot(bClass->baseClasses())) {
            DotNode *bn;
  
            if ((bn = m_usedNodes->value(bClass->name()))) { 
               // node already present

               if (n->m_children == 0 || n->m_children->indexOf(bn) == -1) { // no arrow yet
                  n->addChild(bn, bcd->prot);
                  bn->addParent(n);                  
               }
          
            } else {
               QString tmp_url = "";

               if (bClass->isLinkable() && !bClass->isHidden()) {
                  tmp_url = bClass->getReference() + "$" + bClass->getOutputFileBase();
                  if (!bClass->anchor().isEmpty()) {
                     tmp_url += "#" + bClass->anchor();
                  }
               }

               QString tooltip = bClass->briefDescriptionAsTooltip();
               bn = new DotNode(m_curNodeNumber++, bClass->displayName(), tooltip, tmp_url);
               n->addChild(bn, bcd->prot);
               bn->addParent(n);

               m_usedNodes->insert(bClass->name(), bn); // add node to the used list
            }

            if (! bClass->visited && !hideSuper && bClass->subClasses()) {
               bool wasVisited = bClass->visited;
               bClass->visited = true;
               addHierarchy(bn, bClass, wasVisited);
            }
         }
      }
   }
   
}

void DotGfxHierarchyTable::addClassList(ClassSDict *cl)
{
    for (auto cd :*cl) {    

      if (! hasVisibleRoot(cd->baseClasses()) && cd->isVisibleInHierarchy() ) { 
         // root node in the forest
         QString tmp_url = "";

         if (cd->isLinkable() && !cd->isHidden()) {
            tmp_url = cd->getReference() + "$" + cd->getOutputFileBase();

            if (!cd->anchor().isEmpty()) {
               tmp_url += "#" + cd->anchor();
            }
         }
         
         QString tooltip = cd->briefDescriptionAsTooltip();
         DotNode *n = new DotNode(m_curNodeNumber++, cd->displayName(), tooltip, tmp_url);
        
         m_usedNodes->insert(cd->name(), n);
         m_rootNodes->insert(0, n);

         if (! cd->visited && cd->subClasses()) {
            addHierarchy(n, cd, cd->visited);
            cd->visited = true;
         }
      }
   }
}

DotGfxHierarchyTable::DotGfxHierarchyTable()
{
   m_curNodeNumber = 0;

   m_rootNodes = new QList<DotNode *>;
   m_usedNodes = new QHash<QString, DotNode *>; 
 
   m_rootSubgraphs = new SortedList<DotNode *>;

   // build a graph with each class as a node and the inheritance relations as edges
   initClassHierarchy(Doxy_Globals::classSDict);
   initClassHierarchy(Doxy_Globals::hiddenClasses);
   addClassList(Doxy_Globals::classSDict);
   addClassList(Doxy_Globals::hiddenClasses);

   // m_usedNodes now contains all nodes in the graph

   // color the graph into a set of independent subgraphs
   bool done    = false;
   int curColor = 0;

   QListIterator<DotNode *> dnli(*m_rootNodes);

   while (! done) { 
      // there are still nodes to color
      DotNode *n;

      // we are done unless there are still uncolored nodes
      done = true; 
     
      dnli.toBack();

      while (dnli.hasPrevious() ) {
         n = dnli.previous();

         if (n->m_subgraphId == -1) { 
            // still uncolored nodes
            done = false; 

            n->m_subgraphId = curColor;

            n->markAsVisible();
            n->colorConnectedNodes(curColor);
            curColor++;

            DotNode *dn = const_cast<DotNode *>(n->findDocNode());

            if (dn != 0) {
               m_rootSubgraphs->inSort(dn);

            } else {
               m_rootSubgraphs->inSort(n);

            }

         }
      }
   }
}

DotGfxHierarchyTable::~DotGfxHierarchyTable()
{   
   delete m_rootNodes;
   delete m_usedNodes;
   delete m_rootSubgraphs;
}

void DotClassGraph::addClass(QSharedPointer<ClassDef> cd, DotNode *n, int prot, const QString &label, 
                             const QString &usedName, const QString &templSpec, bool base, int distance)
{
   if (Config::getBool("hide-undoc-classes") && ! cd->isLinkable()) {
      return;
   }
  
   int edgeStyle = (! label.isEmpty() || prot==EdgeInfo::Orange || prot==EdgeInfo::Orange2) ? EdgeInfo::Dashed : EdgeInfo::Solid;
   QString className;

   if (! usedName.isEmpty()) { 
      // name is a typedef
      className = usedName;

   } else if (! templSpec.isEmpty()) { 
      // name has a template part
      className = insertTemplateSpecifierInScope(cd->name(), templSpec);

   } else { 
      // just a normal name
      className = cd->displayName();
   }

   DotNode *bn = m_usedNodes->value(className);

   if (bn) { 
      // class already inserted

      if (base) {
         n->addChild(bn, prot, edgeStyle, label);
         bn->addParent(n);

      } else {
         bn->addChild(n, prot, edgeStyle, label);
         n->addParent(bn);
      }

      bn->setDistance(distance);      

   } else { 
      // new class

      QString displayName = className;

      if (Config::getBool("hide-scope-names")) {
         displayName = stripScope(displayName);
      }

      QString tmp_url;
      if (cd->isLinkable() && !cd->isHidden()) {
         tmp_url = cd->getReference() + "$" + cd->getOutputFileBase();
         if (!cd->anchor().isEmpty()) {
            tmp_url += "#" + cd->anchor();
         }
      }
      QString tooltip = cd->briefDescriptionAsTooltip();
      bn = new DotNode(m_curNodeNumber++, displayName, tooltip, tmp_url, false, cd);

      if (base) {
         n->addChild(bn, prot, edgeStyle, label);
         bn->addParent(n);
      } else {
         bn->addChild(n, prot, edgeStyle, label);
         n->addParent(bn);
      }

      bn->setDistance(distance);
      m_usedNodes->insert(className, bn);
     
      buildGraph(cd, bn, base, distance + 1);
   }
}

void DotClassGraph::determineTruncatedNodes(QList<DotNode *> &queue, bool includeParents)
{
   while (queue.count() > 0) {
      DotNode *n = queue.takeAt(0);

      if (n->isVisible() && n->isTruncated() == DotNode::Unknown) {
         bool truncated = false;

         if (n->m_children) {       
            for (auto dn : *n->m_children) {
               if (!dn->isVisible()) {
                  truncated = true;
               } else {
                  queue.append(dn);
               }
            }
         }

         if (n->m_parents && includeParents) {           
            for (auto dn : *n->m_parents) {
               if (! dn->isVisible()) {
                  truncated = true;
               } else {
                  queue.append(dn);
               }
            }
         }
         n->markAsTruncated(truncated);
      }
   }
}

bool DotClassGraph::determineVisibleNodes(DotNode *rootNode, int maxNodes, bool includeParents)
{
   QList<DotNode *> childQueue;
   QList<DotNode *> parentQueue;

   QVector<int> childTreeWidth;
   QVector<int> parentTreeWidth;

   childQueue.append(rootNode);
   if (includeParents) {
      parentQueue.append(rootNode);
   }

   bool firstNode = true; // flag to force reprocessing rootNode in the parent loop

   // despite being marked visible in the child loop
   while ((childQueue.count() > 0 || parentQueue.count() > 0) && maxNodes > 0) {
      static int maxDistance = Config::getInt("dot-graph-max-depth");

      if (childQueue.count() > 0) {
         DotNode *n = childQueue.takeAt(0);
         int distance = n->distance();

         if (!n->isVisible() && distance <= maxDistance) { // not yet processed
            if (distance > 0) {
               int oldSize = childTreeWidth.size();

               if (distance > oldSize) {
                  childTreeWidth.resize(qMax(childTreeWidth.size(), distance));
                  
                  for (int i = oldSize; i < distance; i++) {
                     childTreeWidth[i] = 0;
                  }
               }
               childTreeWidth[distance - 1] += n->label().length();
            }

            n->markAsVisible();
            maxNodes--;

            // add direct children
            if (n->m_children) {
               
               for (auto dn : *n->m_children) {
                  childQueue.append(dn);
               }
            }
         }
      }

      if (includeParents && parentQueue.count() > 0) {
         DotNode *n = parentQueue.takeAt(0);

         if ((!n->isVisible() || firstNode) && n->distance() <= maxDistance) { // not yet processed
            firstNode = false;
            int distance = n->distance();

            if (distance > 0) {
               int oldSize = parentTreeWidth.size();

               if (distance > oldSize) {
                  parentTreeWidth.resize(qMax(parentTreeWidth.size(), distance));                  

                  for (int i = oldSize; i < distance; i++) {
                     parentTreeWidth[i] = 0;
                  }
               }

               parentTreeWidth[distance - 1] += n->label().length();
            }

            n->markAsVisible();
            maxNodes--;

            // add direct parents
            if (n->m_parents) {              
               for (auto dn : *n->m_parents) {
                  parentQueue.append(dn);
               }
            }
         }
      }
   }

   if (Config::getBool("uml-look")) {
      return false;   // UML graph are always top to bottom
   }

   int maxWidth = 0;
   int maxHeight = qMax(childTreeWidth.size(), parentTreeWidth.size());
   uint i;

   for (i = 0; i < childTreeWidth.size(); i++) {
      if (childTreeWidth.at(i) > maxWidth) {
         maxWidth = childTreeWidth.at(i);
      }
   }

   for (i = 0; i < parentTreeWidth.size(); i++) {
      if (parentTreeWidth.at(i) > maxWidth) {
         maxWidth = parentTreeWidth.at(i);
      }
   }

   //printf("max tree width=%d, max tree height=%d\n",maxWidth,maxHeight);
   return maxWidth > 80 && maxHeight < 12; 

   // used metric to decide to render the tree
   // from left to right instead of top to bottom,
   // with the idea to render very wide trees in
   // left to right order.
}

void DotClassGraph::buildGraph(QSharedPointer<ClassDef> cd, DotNode *n, bool base, int distance)
{   
   // ---- Add inheritance relations
   static bool templateRelations = Config::getBool("template-relations");

   if (m_graphType == DotNode::Inheritance || m_graphType == DotNode::Collaboration) {
      SortedList<BaseClassDef *> *bcl = base ? cd->baseClasses() : cd->subClasses();

      if (bcl) {       
         for (auto bcd : *bcl) {            
            addClass(bcd->classDef, n, bcd->prot, "", bcd->usedName, bcd->templSpecifiers, base, distance);
         }
      }
   }

   if (m_graphType == DotNode::Collaboration) {
      // ---- Add usage relations

      UsesClassDict *dict = base ? cd->usedImplementationClasses() : cd->usedByImplementationClasses();

      if (dict) {
        
         for (auto ucd : *dict) {
            QString label;
                     
            bool first    = true;
            int count     = 0;
            int maxLabels = 10;
           
            for (auto s : ucd.m_accessors ) {

               if (count >= maxLabels) {
                  break;
               }                            

               if (first) {
                  label = s;
                  first = false;

               } else {
                  label += "\n" + s;
               }

               ++count;
            }

            if (count == maxLabels) {
               label += "\n...";
            }
            
            addClass(ucd.m_classDef, n, EdgeInfo::Purple, label, 0, ucd.m_templSpecifiers, base, distance);
         }
      }
   }


   if (templateRelations && base) {
      QHash<QString, QSharedPointer<ConstraintClassDef>> dict = cd->templateTypeConstraints();
     
      for (auto ccd : dict) { 
         QString label;
         QString s;

         bool first    = true;
         int count     = 0;
         int maxLabels = 10;

         for (auto s : ccd->m_accessors)  {

            if (count >= maxLabels) {
               break;
            }

            if (first) {
               label = s;
               first = false;

            } else {
               label += "\n" + s;
            }

         }

         if (count == maxLabels) {
            label += "\n...";
         }
         
         addClass(ccd->classDef, n, EdgeInfo::Orange2, label, 0, 0, true, distance);            
         ++count;
      }
     
   }

   // ---- Add template instantiation relations
   if (templateRelations) {

      if (base) { 
         // template relations for base classes
         QSharedPointer<ClassDef> templMaster = cd->templateMaster();

         if (templMaster) { 
            auto iter = templMaster->getTemplateInstances()->begin();

            for (auto templInstance : *templMaster->getTemplateInstances()) {                

               if (templInstance == cd) { 
                  addClass(templMaster, n, EdgeInfo::Orange, iter.key(), 0, 0, true, distance);
               }

               ++iter;
            }
         }

      } else { 
         // template relations for super classes
         QHash<QString, QSharedPointer<ClassDef>> *templInstances = cd->getTemplateInstances();

         if (templInstances) {
            auto iter = templInstances->begin();

            for (auto templInstance : *templInstances) { 
               addClass(templInstance, n, EdgeInfo::Orange, iter.key(), 0, 0, false, distance);

               ++iter;
            }
         }
      }
   }
}

DotClassGraph::DotClassGraph(QSharedPointer<ClassDef> cd, DotNode::GraphType t)
{
   m_graphType = t;

   QString tmp_url = "";

   if (cd->isLinkable() && !cd->isHidden()) {
      tmp_url = cd->getReference() + "$" + cd->getOutputFileBase();
      if (!cd->anchor().isEmpty()) {
         tmp_url += "#" + cd->anchor();
      }
   }

   QString className = cd->displayName();
   QString tooltip   = cd->briefDescriptionAsTooltip();

   // is a root node
   m_startNode = new DotNode(m_curNodeNumber++, className, tooltip, tmp_url, true, cd);

   m_startNode->setDistance(0);
   m_usedNodes = new QHash<QString, DotNode *>;
   m_usedNodes->insert(className, m_startNode);
   
   buildGraph(cd, m_startNode, true, 1);

   if (t == DotNode::Inheritance) {
      buildGraph(cd, m_startNode, false, 1);
   }  

   static int maxNodes = Config::getInt("dot-graph-max-nodes");

   m_lrRank = determineVisibleNodes(m_startNode, maxNodes, t == DotNode::Inheritance);
   QList<DotNode *> openNodeQueue;
   openNodeQueue.append(m_startNode);

   determineTruncatedNodes(openNodeQueue, t == DotNode::Inheritance);

   m_diskName = cd->getFileBase();
}

bool DotClassGraph::isTrivial() const
{
   static bool umlLook = Config::getBool("uml-look");

   if (m_graphType == DotNode::Inheritance) {
      return m_startNode->m_children == 0 && m_startNode->m_parents == 0;
   } else {
      return !umlLook && m_startNode->m_children == 0;
   }
}

bool DotClassGraph::isTooBig() const
{
   static int maxNodes = Config::getInt("dot-graph-max-nodes");

   int numNodes = 0;
   numNodes += m_startNode->m_children ? m_startNode->m_children->count() : 0;

   if (m_graphType == DotNode::Inheritance) {
      numNodes += m_startNode->m_parents ? m_startNode->m_parents->count() : 0;
   }
   return numNodes >= maxNodes;
}

DotClassGraph::~DotClassGraph()
{
   deleteNodes(m_startNode);
   delete m_usedNodes;
}

/*! Computes a 16 byte md5 checksum for a given dot graph.
 *  The md5 checksum is returned as a 32 character ASCII string.
 */
QString computeMd5Signature(DotNode *root, DotNode::GraphType gt, GraphOutputFormat format, bool lrRank,
                               bool renderParents, bool backArrows, const QString &title, QString &graphStr )
{
   bool reNumber = true;
 
   QByteArray buf;

   QTextStream md5stream(&buf);
   writeGraphHeader(md5stream, title);

   if (lrRank) {
      md5stream << "  rankdir=\"LR\";" << endl;
   }

   root->clearWriteFlag();
   root->write(md5stream, gt, format, gt != DotNode::CallGraph && gt != DotNode::Dependency, true, backArrows, reNumber);

   if (renderParents && root->m_parents) {
      for (auto pn : *root->m_parents) {

         if (pn->isVisible()) {
            root->writeArrow(md5stream, gt, format, pn, pn->m_edgeInfo->at(pn->m_children->indexOf(root)),  
                             false, backArrows, reNumber);                             
         }

         pn->write(md5stream,      // stream
                   gt,             // graph type
                   format,         // output format
                   true,           // topDown?
                   false,          // toChildren?
                   backArrows,     // backward pointing arrows?
                   reNumber        // renumber nodes?
                  );
      }
   }

   writeGraphFooter(md5stream);

   QString sigStr;
   sigStr = QCryptographicHash::hash(buf, QCryptographicHash::Md5).toHex();
  
   if (reNumber) {
     resetReNumbering();
   }

   graphStr = buf;
  
   return sigStr;
}

static bool updateDotGraph(DotNode *root, DotNode::GraphType gt, const QString &baseName, GraphOutputFormat format,
                           bool lrRank, bool renderParents, bool backArrows, const QString &title = QString() )
{
   QString theGraph;

   // TODO: write graph to theGraph, then compute md5 checksum
   QString md5 = computeMd5Signature( root, gt, format, lrRank, renderParents, backArrows, title, theGraph);
   QFile f(baseName + ".dot");

   if (f.open(QIODevice::WriteOnly)) {
      QTextStream t(&f);
      t << theGraph;
   }

   return checkAndUpdateMd5Signature(baseName, md5); // graph needs to be regenerated
}

QString DotClassGraph::diskName() const
{
   QString result = m_diskName;

   switch (m_graphType) {
      case DotNode::Collaboration:
         result += "_coll_graph";
         break;
    
      case DotNode::Inheritance:
         result += "_inherit_graph";
         break;

      default:
         assert(0);
         break;
   }

   return result;
}

QString DotClassGraph::writeGraph(QTextStream &out, GraphOutputFormat graphFormat, EmbeddedOutputFormat textFormat,
                  const QString &path, const QString &fileName, const QString &relPath, bool, bool generateImageMap, int graphId) const 
{
   QDir d(path);

   // store the original directory
   if (! d.exists()) {
      err("Output dir %s does not exist\n", qPrintable(path));
      exit(1);
   }

   static bool usePDFLatex = Config::getBool("latex-pdf");

   QString baseName;
   QString mapName;

   switch (m_graphType) {
      case DotNode::Collaboration:
         mapName = "coll_map";
         break;    

      case DotNode::Inheritance:
         mapName = "inherit_map";
         break;

      default:
         assert(0);
         break;
   }

   baseName = convertNameToFile(diskName());

   // derive target file names from baseName
   QString imgExt      = Config::getEnum("dot-image-format");
   QString absBaseName = d.absolutePath() + "/" + baseName;
   QString absDotName  = absBaseName + ".dot";
   QString absMapName  = absBaseName + ".map";
   QString absPdfName  = absBaseName + ".pdf";
   QString absEpsName  = absBaseName + ".eps";
   QString absImgName  = absBaseName + "." + imgExt;

   bool regenerate = false;

   bool ok = false;
   bool x  = updateDotGraph(m_startNode, m_graphType, absBaseName, graphFormat, m_lrRank,
                            m_graphType == DotNode::Inheritance, true, m_startNode->label() );    

   if (x)  {
      ok = true;
      
   } else { 
      QString arg1;

      if (graphFormat == GOF_BITMAP) {
         arg1 = absImgName;

      } else if (usePDFLatex) {
         arg1  = absPdfName;

      } else {
         arg1 = absEpsName;
      }

      QString arg2;
      if (graphFormat == GOF_BITMAP && generateImageMap) {
         arg2 = absMapName;
      }

      bool y = checkDeliverables(arg1, arg2);                                
      
      if (y) {
         ok = false;
      } else {
         ok = true;
      }   
   }

   if (ok) {
      regenerate = true;

      if (graphFormat == GOF_BITMAP) {
         // run dot to create a bitmap image
         
         DotRunner *dotRun = new DotRunner(absDotName, d.absolutePath(), true, absImgName);
         dotRun->addJob(imgExt, absImgName);

         if (generateImageMap) {
            dotRun->addJob(MAP_CMD, absMapName);
         }

         DotManager::instance()->addRun(dotRun);

      } else if (graphFormat == GOF_EPS) { // run dot to create a .eps image
         DotRunner *dotRun = new DotRunner(absDotName, d.absolutePath(), false);

         if (usePDFLatex) {
            dotRun->addJob("pdf", absPdfName);
         } else {
            dotRun->addJob("ps", absEpsName);
         }

         DotManager::instance()->addRun(dotRun);
      }
   }

   Doxy_Globals::indexList->addImageFile(baseName + "." + imgExt);

   if (graphFormat == GOF_BITMAP && textFormat == EOF_DocBook) {
      out << "<para>" << endl;
      out << "    <figure>" << endl;
      out << "        <title>";

      switch (m_graphType) {
         case DotNode::Collaboration:
            out << "Collaboration graph";
            break;

         case DotNode::Inheritance:
            out << "Inheritance graph";
            break;

         default:
            assert(0);
            break;
      }

      out << "</title>" << endl;
      out << "        <mediaobject>" << endl;
      out << "            <imageobject>" << endl;
      out << "                <imagedata";
      out << " width=\"50%\" align=\"center\" valign=\"middle\" scalefit=\"1\" fileref=\"" << relPath << baseName << "." << imgExt << "\">";
      out << "</imagedata>" << endl;
      out << "            </imageobject>" << endl;
      out << "        </mediaobject>" << endl;
      out << "    </figure>" << endl;
      out << "</para>" << endl;

   } else if (graphFormat == GOF_BITMAP && generateImageMap) { // produce HTML to include the image
      QString mapLabel = escapeCharsInString(m_startNode->m_label, false) + "_" + escapeCharsInString(mapName, false);

      if (imgExt == "svg") { // add link to SVG file without map file
         out << "<div class=\"center\">";

         if (regenerate || ! writeSVGFigureLink(out, relPath, baseName, absImgName)) { 
            // need to patch the links in the generated SVG file

            if (regenerate) {
               DotManager::instance()->addSVGConversion(absImgName, relPath, false, QString(), true, graphId);
            }

            int mapId = DotManager::instance()->addSVGObject(fileName, baseName, absImgName, relPath);
            out << "<!-- SVG " << mapId << " -->" << endl;
         }

         out << "</div>" << endl;

      } else { // add link to bitmap file with image map

         out << "<div class=\"center\">";
         out << "<img src=\"" << relPath << baseName << "."
             << imgExt << "\" border=\"0\" usemap=\"#"
             << mapLabel << "\" alt=\"";

         switch (m_graphType) {
            case DotNode::Collaboration:
               out << "Collaboration graph";
               break;
            case DotNode::Inheritance:
               out << "Inheritance graph";
               break;
            default:
               assert(0);
               break;
         }

         out << "\"/>";
         out << "</div>" << endl;
         if (regenerate || !insertMapFile(out, absMapName, relPath, mapLabel)) {
            int mapId = DotManager::instance()->addMap(fileName, absMapName, relPath,
                        false, QString(), mapLabel);
            out << "<!-- MAP " << mapId << " -->" << endl;
         }
      }

   } else if (graphFormat == GOF_EPS) { // produce tex to include the .eps image
      if (regenerate || !writeVecGfxFigure(out, baseName, absBaseName)) {
         int figId = DotManager::instance()->addFigure(fileName, baseName, absBaseName, false /*true*/);
         out << endl << "% FIG " << figId << endl;
      }
   }

   if (! regenerate) {
      removeDotGraph(absDotName);
   }

   return baseName;
}

void DotClassGraph::writeXML(QTextStream &t)
{   
   for (auto node : *m_usedNodes) {
      node->writeXML(t, true);
   }
}

void DotClassGraph::writeDocbook(QTextStream &t)
{
  for (auto node : *m_usedNodes) {
      node->writeDocbook(t, true);
   }
}

void DotClassGraph::writeDEF(QTextStream &t)
{
   for (auto node : *m_usedNodes) {
      node->writeDEF(t);
   }
}

void DotInclDepGraph::buildGraph(DotNode *n, QSharedPointer<FileDef> fd, int distance)
{
   QList<IncludeInfo> *includeFiles = m_inverse ? fd->includedByFileList() : fd->includeFileList();

   if (includeFiles) {
    
      for (auto ii : *includeFiles) {
         QSharedPointer<FileDef> bfd = ii.fileDef;
         QString in  = ii.includeName;
 
         bool doc = true, src = false;

         if (bfd) {
            in  = bfd->getFilePath();
            doc = bfd->isLinkable() && !bfd->isHidden();
            src = bfd->generateSourceFile();
         }

         if (doc || src || ! Config::getBool("hide-undoc-relations")) {
            QString url = "";

            if (bfd) {
               url = bfd->getOutputFileBase();
            }

            if (!doc && src) {
               url = bfd->getSourceFileBase();
            }

            DotNode *bn  = m_usedNodes->value(in);

            if (bn) { // file is already a node in the graph
               n->addChild(bn, 0, 0, 0);
               bn->addParent(n);
               bn->setDistance(distance);

            } else {
               QString tmp_url;
               QString tooltip;

               if (bfd) {
                  tmp_url = doc || src ? bfd->getReference() + "$" + url : QString();
                  tooltip = bfd->briefDescriptionAsTooltip();
               }
               bn = new DotNode(m_curNodeNumber++, ii.includeName, tooltip, tmp_url, false, QSharedPointer<ClassDef>());

               n->addChild(bn, 0, 0, 0);
               bn->addParent(n);
               m_usedNodes->insert(in, bn);
               bn->setDistance(distance);

               if (bfd) {
                  buildGraph(bn, bfd, distance + 1);
               }
            }
         }
      }
   }
}

void DotInclDepGraph::determineVisibleNodes(QList<DotNode *> &queue, int &maxNodes)
{
   while (queue.count() > 0 && maxNodes > 0) {
      static int maxDistance = Config::getInt("dot-graph-max-depth");
      DotNode *n = queue.takeAt(0);

      if (! n->isVisible() && n->distance() <= maxDistance) { // not yet processed
         n->markAsVisible();
         maxNodes--;

         // add direct children
         if (n->m_children) {
           
            for (auto dn : *n->m_children) {
               queue.append(dn);
            }
         }
      }
   }
}

void DotInclDepGraph::determineTruncatedNodes(QList<DotNode *> &queue)
{
   while (queue.count() > 0) {
      DotNode *n = queue.takeFirst();

      if (n->isVisible() && n->isTruncated() == DotNode::Unknown) {
         bool truncated = false;

         if (n->m_children) {           
            for (auto dn : *n->m_children) {
               if (!dn->isVisible()) {
                  truncated = true;

               } else {
                  queue.append(dn);
               }
            }
         }

         n->markAsTruncated(truncated);
      }
   }
}


DotInclDepGraph::DotInclDepGraph(QSharedPointer<FileDef> fd, bool inverse)
{
   m_inverse = inverse;
   assert(fd != 0);

   m_diskName  = fd->getFileBase();

   QString tmp_url = fd->getReference() + "$" + fd->getFileBase();

   // root node
   m_startNode = new DotNode(m_curNodeNumber++, fd->docName(), "", tmp_url,  true );
   m_startNode->setDistance(0);

   m_usedNodes = new QHash<QString, DotNode *>;
   m_usedNodes->insert(fd->getFilePath(), m_startNode);
   buildGraph(m_startNode, fd, 1);

   static int nodes = Config::getInt("dot-graph-max-nodes");
   int maxNodes = nodes;
 
   QList<DotNode *> openNodeQueue;

   openNodeQueue.append(m_startNode);

   determineVisibleNodes(openNodeQueue, maxNodes);
   openNodeQueue.clear();
   openNodeQueue.append(m_startNode);

   determineTruncatedNodes(openNodeQueue);
}

DotInclDepGraph::~DotInclDepGraph()
{
   deleteNodes(m_startNode);
   delete m_usedNodes;
}

QString DotInclDepGraph::diskName() const
{
   QString result = m_diskName;

   if (m_inverse) {
      result += "_dep";
   }

   result += "_incl";

   return convertNameToFile(result);
}

QString DotInclDepGraph::writeGraph(QTextStream &out, GraphOutputFormat graphFormat, EmbeddedOutputFormat textFormat,                                                                  
                                       const QString &path, const QString &fileName, const QString &relPath, bool generateImageMap, int graphId) const
{
   QDir d(path);

   // store the original directory
   if (! d.exists()) {
      err("Output dir %s does not exist\n", qPrintable(path));
      exit(1);
   }
   static bool usePDFLatex = Config::getBool("latex-pdf");

   QString baseName = m_diskName;

   if (m_inverse) {
      baseName += "_dep";
   }

   baseName += "_incl";
   baseName = convertNameToFile(baseName);

   QString mapName = escapeCharsInString(m_startNode->m_label, false);
   if (m_inverse) {
      mapName += "dep";
   }

   QString imgExt      = Config::getEnum("dot-image-format");
   QString absBaseName = d.absolutePath() + "/" + baseName;
   QString absDotName  = absBaseName + ".dot";
   QString absMapName  = absBaseName + ".map";
   QString absPdfName  = absBaseName + ".pdf";
   QString absEpsName  = absBaseName + ".eps";
   QString absImgName  = absBaseName + "." + imgExt;

   bool regenerate = false;
   if (updateDotGraph(m_startNode, DotNode::Dependency, absBaseName, graphFormat, false, false,        
                      m_inverse, m_startNode->label() ) || ! checkDeliverables(graphFormat == GOF_BITMAP ? absImgName :
                            usePDFLatex ? absPdfName : absEpsName,
                            graphFormat == GOF_BITMAP && generateImageMap ? absMapName : QString())) {

      regenerate = true;
      if (graphFormat == GOF_BITMAP) {
         // run dot to create a bitmap image

         DotRunner *dotRun = new DotRunner(absDotName, d.absolutePath(), true, absImgName);
         dotRun->addJob(imgExt, absImgName);

         if (generateImageMap) {
            dotRun->addJob(MAP_CMD, absMapName);
         }

         DotManager::instance()->addRun(dotRun);

      } else if (graphFormat == GOF_EPS) {
         DotRunner *dotRun = new DotRunner(absDotName, d.absolutePath(), false);

         if (usePDFLatex) {
            dotRun->addJob("pdf", absPdfName);
         } else {
            dotRun->addJob("ps", absEpsName);
         }

         DotManager::instance()->addRun(dotRun);
      }
   }
   Doxy_Globals::indexList->addImageFile(baseName + "." + imgExt);

   if (graphFormat == GOF_BITMAP && textFormat == EOF_DocBook) {
      out << "<para>" << endl;
      out << "    <figure>" << endl;
      out << "        <title>Dependency diagram";
      out << "</title>" << endl;
      out << "        <mediaobject>" << endl;
      out << "            <imageobject>" << endl;
      out << "                <imagedata";
      out << " width=\"50%\" align=\"center\" valign=\"middle\" scalefit=\"1\" fileref=\"" << relPath << baseName << "." << imgExt << "\">";
      out << "</imagedata>" << endl;
      out << "            </imageobject>" << endl;
      out << "        </mediaobject>" << endl;
      out << "    </figure>" << endl;
      out << "</para>" << endl;

   } else if (graphFormat == GOF_BITMAP && generateImageMap) {

      if (imgExt == "svg") { 
         // Scalable vector graphics
         out << "<div class=\"center\">";

         if (regenerate || ! writeSVGFigureLink(out, relPath, baseName, absImgName)) { 
            // need to patch the links in the generated SVG file

            if (regenerate) {
               DotManager::instance()->addSVGConversion(absImgName, relPath, false, QString(), true, graphId);
            }

            int mapId = DotManager::instance()->addSVGObject(fileName, baseName, absImgName, relPath);
            out << "<!-- SVG " << mapId << " -->" << endl;
         }
         out << "</div>" << endl;

      } else { // bitmap graphics
         out << "<div class=\"center\"><img src=\"" << relPath << baseName << "." << imgExt << "\" border=\"0\" usemap=\"#" << mapName << "\" alt=\"\"/>";
         out << "</div>" << endl;

         QString absMapName = absBaseName + ".map";

         if (regenerate || !insertMapFile(out, absMapName, relPath, mapName)) {
            int mapId = DotManager::instance()->addMap(fileName, absMapName, relPath,
                        false, QString(), mapName);

            out << "<!-- MAP " << mapId << " -->" << endl;
         }
      }

   } else if (graphFormat == GOF_EPS) { // encapsulated postscript
      if (regenerate || !writeVecGfxFigure(out, baseName, absBaseName)) {
         int figId = DotManager::instance()->addFigure(fileName, baseName, absBaseName, false);
         out << endl << "% FIG " << figId << endl;
      }
   }
   if (! regenerate) {
      removeDotGraph(absDotName);
   }

   return baseName;
}

bool DotInclDepGraph::isTrivial() const
{
   return m_startNode->m_children == 0;
}

bool DotInclDepGraph::isTooBig() const
{
   static int maxNodes = Config::getInt("dot-graph-max-nodes");
   int numNodes = m_startNode->m_children ? m_startNode->m_children->count() : 0;
   return numNodes >= maxNodes;
}

void DotInclDepGraph::writeXML(QTextStream &t)
{  
   for (auto node : *m_usedNodes) { 
      node->writeXML(t, false);
   }
}

void DotInclDepGraph::writeDocbook(QTextStream &t)
{
   for (auto node : *m_usedNodes) { 
      node->writeDocbook(t, false);
   }
}

void DotCallGraph::buildGraph(DotNode *n, QSharedPointer<MemberDef> md, int distance)
{
   MemberSDict *refs = m_inverse ? md->getReferencedByMembers() : md->getReferencesMembers();

   if (refs) {
     
      for (auto rmd : *refs) {
         if (rmd->showInCallGraph()) {
            QString uniqueId;

            uniqueId = rmd->getReference() + "$" + rmd->getOutputFileBase() + "#" + rmd->anchor();
            DotNode *bn  = m_usedNodes->value(uniqueId);

            if (bn) { // file is already a node in the graph
               n->addChild(bn, 0, 0, 0);
               bn->addParent(n);
               bn->setDistance(distance);

            } else {
               QString name;

               if (Config::getBool("hide-scope-names")) {
                  name  = rmd->getOuterScope() == m_scope ? rmd->name() : rmd->qualifiedName();
               } else {
                  name = rmd->qualifiedName();
               }

               QString tooltip = rmd->briefDescriptionAsTooltip();
               bn = new DotNode(m_curNodeNumber++, linkToText(rmd->getLanguage(), name, false), tooltip, uniqueId, false);
               n->addChild(bn, 0, 0, 0);
               bn->addParent(n);
               bn->setDistance(distance);
               m_usedNodes->insert(uniqueId, bn);

               buildGraph(bn, rmd, distance + 1);
            }
         }
      }
   }
}

void DotCallGraph::determineVisibleNodes(QList<DotNode *> &queue, int &maxNodes)
{
   while (queue.count() > 0 && maxNodes > 0) {
      static int maxDistance = Config::getInt("dot-graph-max-depth");
      DotNode *n = queue.takeAt(0);

      if (!n->isVisible() && n->distance() <= maxDistance) { // not yet processed
         n->markAsVisible();
         maxNodes--;

         // add direct children
         if (n->m_children) {         
            for (auto dn : *n->m_children) {
               queue.append(dn);
            }
         }
      }
   }
}

void DotCallGraph::determineTruncatedNodes(QList<DotNode *> &queue)
{
   while (queue.count() > 0) {
      DotNode *n = queue.takeAt(0);

      if (n->isVisible() && n->isTruncated() == DotNode::Unknown) {
         bool truncated = false;

         if (n->m_children) {          
            for (auto dn : *n->m_children) {
               if (!dn->isVisible()) {
                  truncated = true;
               } else {
                  queue.append(dn);
               }
            }
         }
         n->markAsTruncated(truncated);
      }
   }
}

DotCallGraph::DotCallGraph(QSharedPointer<MemberDef> md, bool inverse)
{
   m_inverse = inverse;
   m_diskName = md->getOutputFileBase() + "_" + md->anchor();
   m_scope    = md->getOuterScope();

   QString uniqueId;
   uniqueId = md->getReference() + "$" + md->getOutputFileBase() + "#" + md->anchor();

   QString name;

   if (Config::getBool("hide-scope-names")) {
      name = md->name();
   } else {
      name = md->qualifiedName();
   }

   m_startNode = new DotNode(m_curNodeNumber++, linkToText(md->getLanguage(), name, false), "", uniqueId, true);

   m_startNode->setDistance(0);
   m_usedNodes = new QHash<QString, DotNode *>;
   m_usedNodes->insert(uniqueId, m_startNode);
   buildGraph(m_startNode, md, 1);

   static int nodes = Config::getInt("dot-graph-max-nodes");
   int maxNodes = nodes;
  
   QList<DotNode *> openNodeQueue;
   openNodeQueue.append(m_startNode);

   determineVisibleNodes(openNodeQueue, maxNodes);
   openNodeQueue.clear();
   openNodeQueue.append(m_startNode);

   determineTruncatedNodes(openNodeQueue);
}

DotCallGraph::~DotCallGraph()
{
   deleteNodes(m_startNode);
   delete m_usedNodes;
}

QString DotCallGraph::writeGraph(QTextStream &out, GraphOutputFormat graphFormat, EmbeddedOutputFormat textFormat,
                  const QString &path, const QString &fileName, const QString &relPath, bool generateImageMap, int graphId) const
{
   QDir d(path);

   // store the original directory
   if (! d.exists()) {
      err("Output dir %s does not exist\n", qPrintable(path));
      exit(1);
   }

   static bool usePDFLatex = Config::getBool("latex-pdf");

   QString baseName = m_diskName + (m_inverse ? "_icgraph" : "_cgraph");
   QString mapName  = baseName;

   QString imgExt      = Config::getEnum("dot-image-format");
   QString absBaseName = d.absolutePath() + "/" + baseName;
   QString absDotName  = absBaseName + ".dot";
   QString absMapName  = absBaseName + ".map";
   QString absPdfName  = absBaseName + ".pdf";
   QString absEpsName  = absBaseName + ".eps";
   QString absImgName  = absBaseName + "." + imgExt;

   bool regenerate = false;
   if (updateDotGraph(m_startNode, DotNode::CallGraph, absBaseName, graphFormat,
                      true,         // lrRank
                      false,        // renderParents
                      m_inverse,    // backArrows
                      m_startNode->label() ) ||
                      ! checkDeliverables(graphFormat == GOF_BITMAP ? absImgName :
                            usePDFLatex ? absPdfName : absEpsName,
                            graphFormat == GOF_BITMAP && generateImageMap ? absMapName : QString()) ) {

      regenerate = true;

      if (graphFormat == GOF_BITMAP) {
         // run dot to create a bitmap image

         DotRunner *dotRun = new DotRunner(absDotName, d.absolutePath(), true, absImgName);
         dotRun->addJob(imgExt, absImgName);

         if (generateImageMap) {
            dotRun->addJob(MAP_CMD, absMapName);
         }

         DotManager::instance()->addRun(dotRun);

      } else if (graphFormat == GOF_EPS) {
         // run dot to create a .eps image
         DotRunner *dotRun = new DotRunner(absDotName, d.absolutePath(), false);

         if (usePDFLatex) {
            dotRun->addJob("pdf", absPdfName);
         } else {
            dotRun->addJob("ps", absEpsName);
         }
         DotManager::instance()->addRun(dotRun);

      }
   }
   Doxy_Globals::indexList->addImageFile(baseName + "." + imgExt);

   if (graphFormat == GOF_BITMAP && textFormat == EOF_DocBook) {
      out << "<para>" << endl;
      out << "    <figure>" << endl;
      out << "        <title>Call diagram";
      out << "</title>" << endl;
      out << "        <mediaobject>" << endl;
      out << "            <imageobject>" << endl;
      out << "                <imagedata";
      out << " width=\"50%\" align=\"center\" valign=\"middle\" scalefit=\"1\" fileref=\"" << relPath << baseName << "." << imgExt << "\">";
      out << "</imagedata>" << endl;
      out << "            </imageobject>" << endl;
      out << "        </mediaobject>" << endl;
      out << "    </figure>" << endl;
      out << "</para>" << endl;

   } else if (graphFormat == GOF_BITMAP && generateImageMap) {
      if (imgExt == "svg") { // Scalable vector graphics
         out << "<div class=\"center\">";

         if (regenerate || !writeSVGFigureLink(out, relPath, baseName, absImgName)) { 
            // need to patch the links in the generated SVG file

            if (regenerate) {
               DotManager::instance()->addSVGConversion(absImgName, relPath, false, QString(), true, graphId);
            }

            int mapId = DotManager::instance()->addSVGObject(fileName, baseName, absImgName, relPath);
            out << "<!-- SVG " << mapId << " -->" << endl;
         }

         out << "</div>" << endl;

      } else { // bitmap graphics
         out << "<div class=\"center\"><img src=\"" << relPath << baseName << "."
             << imgExt << "\" border=\"0\" usemap=\"#"
             << mapName << "\" alt=\"";
         out << "\"/>";
         out << "</div>" << endl;

         if (regenerate || !insertMapFile(out, absMapName, relPath, mapName)) {
            int mapId = DotManager::instance()->addMap(fileName, absMapName, relPath,
                        false, QString(), mapName);
            out << "<!-- MAP " << mapId << " -->" << endl;
         }
      }
   } else if (graphFormat == GOF_EPS) { // encapsulated postscript
      if (regenerate || !writeVecGfxFigure(out, baseName, absBaseName)) {
         int figId = DotManager::instance()->addFigure(fileName, baseName, absBaseName, false);
         out << endl << "% FIG " << figId << endl;
      }
   }
   if (! regenerate) {
      removeDotGraph(absDotName);
   }

   return baseName;
}

bool DotCallGraph::isTrivial() const
{
   return m_startNode->m_children == 0;
}

bool DotCallGraph::isTooBig() const
{
   static int maxNodes = Config::getInt("dot-graph-max-nodes");
   int numNodes = m_startNode->m_children ? m_startNode->m_children->count() : 0;
   return numNodes >= maxNodes;
}

DotDirDeps::DotDirDeps(QSharedPointer<DirDef> dir) 
   : m_dir(dir)
{
}

DotDirDeps::~DotDirDeps()
{
}

QString DotDirDeps::writeGraph(QTextStream &out, GraphOutputFormat graphFormat, EmbeddedOutputFormat textFormat,
                  const QString &path, const QString &fileName, const QString &relPath, bool generateImageMap, int graphId) const
{
   QDir d(path);

   // store the original directory
   if (! d.exists()) {
      err("Output dir %s does not exist\n", qPrintable(path));
      exit(1);
   }

   static bool usePDFLatex = Config::getBool("latex-pdf");

   QString baseName = m_dir->getOutputFileBase() + "_dep";
   QString mapName  = escapeCharsInString(baseName, false);

   QString imgExt      = Config::getEnum("dot-image-format");
   QString absBaseName = d.absolutePath() + "/" + baseName;
   QString absDotName  = absBaseName + ".dot";
   QString absMapName  = absBaseName + ".map";
   QString absPdfName  = absBaseName + ".pdf";
   QString absEpsName  = absBaseName + ".eps";
   QString absImgName  = absBaseName + "." + imgExt;

   // compute md5 checksum of the graph were are about to generate
   QString theGraph;
   QTextStream md5stream(&theGraph);
   m_dir->writeDepGraph(md5stream);

   QString sigStr;
   sigStr = QCryptographicHash::hash(theGraph.toUtf8(), QCryptographicHash::Md5).toHex();

   bool regenerate = false;

   bool ok = false;
   bool x  = checkAndUpdateMd5Signature(absBaseName, sigStr); 

   if (x) {
      ok = true;

   } else {

      QString arg1;
      if (graphFormat == GOF_BITMAP) {
         arg1 = absImgName; 

      } else if (usePDFLatex) { 
         arg1 = absPdfName;

      } else {
         arg1 =absEpsName;

      }      

      QString arg2;
      if (graphFormat == GOF_BITMAP && generateImageMap) {
         arg2 = absMapName;
      }

      bool y = checkDeliverables(arg1, arg2);        

      if (y) {
         ok = false;
   
      } else {
         ok = true;         
      }           

   }   

   if (ok) {
      regenerate = true;

      QFile f(absDotName);

      if (! f.open(QIODevice::WriteOnly)) {
         err("Unable to open file for writing %s, error: %d\n", qPrintable(baseName), f.error());
      }
        
      QTextStream t(&f);
      t << theGraph;
      f.close();

      if (graphFormat == GOF_BITMAP) {
         // run dot to create a bitmap image
        
         DotRunner *dotRun = new DotRunner(absDotName, d.absolutePath(), true, absImgName);
         dotRun->addJob(imgExt, absImgName);

         if (generateImageMap) {
            dotRun->addJob(MAP_CMD, absMapName);
         }

         DotManager::instance()->addRun(dotRun);

      } else if (graphFormat == GOF_EPS) {
         DotRunner *dotRun = new DotRunner(absDotName, d.absolutePath(), false);

         if (usePDFLatex) {
            dotRun->addJob("pdf", absPdfName);
         } else {
            dotRun->addJob("ps", absEpsName);
         }
         DotManager::instance()->addRun(dotRun);
      }
   }
   Doxy_Globals::indexList->addImageFile(baseName + "." + imgExt);

   if (graphFormat == GOF_BITMAP && textFormat == EOF_DocBook) {
      out << "<para>" << endl;
      out << "    <figure>" << endl;
      out << "        <title>Directory Dependency diagram";
      out << "</title>" << endl;
      out << "        <mediaobject>" << endl;
      out << "            <imageobject>" << endl;
      out << "                <imagedata";
      out << " width=\"50%\" align=\"center\" valign=\"middle\" scalefit=\"1\" fileref=\"" << relPath << baseName << "." << imgExt << "\">";
      out << "</imagedata>" << endl;
      out << "            </imageobject>" << endl;
      out << "        </mediaobject>" << endl;
      out << "    </figure>" << endl;
      out << "</para>" << endl;

   } else if (graphFormat == GOF_BITMAP && generateImageMap) {
      if (imgExt == "svg") { 
         // Scalable vector graphics
         out << "<div class=\"center\">";

         if (regenerate || ! writeSVGFigureLink(out, relPath, baseName, absImgName)) { 
            // need to patch the links in the generated SVG file

            if (regenerate) {
               DotManager::instance()->addSVGConversion(absImgName, relPath, false, QString(), true, graphId);
            }

            int mapId = DotManager::instance()->addSVGObject(fileName, baseName, absImgName, relPath);
            out << "<!-- SVG " << mapId << " -->" << endl;
         }
         out << "</div>" << endl;

      } else { // bitmap graphics
         out << "<div class=\"center\"><img src=\"" << relPath << baseName << "."
             << imgExt << "\" border=\"0\" usemap=\"#"
             << mapName << "\" alt=\"";
         out << convertToXML(m_dir->displayName());
         out << "\"/>";
         out << "</div>" << endl;

         if (regenerate || !insertMapFile(out, absMapName, relPath, mapName)) {
            int mapId = DotManager::instance()->addMap(fileName, absMapName, relPath,
                        true, QString(), mapName);
            out << "<!-- MAP " << mapId << " -->" << endl;
         }
      }
   } else if (graphFormat == GOF_EPS) {
      if (regenerate || !writeVecGfxFigure(out, baseName, absBaseName)) {
         int figId = DotManager::instance()->addFigure(fileName, baseName, absBaseName, false);
         out << endl << "% FIG " << figId << endl;
      }
   }
   if (!regenerate) {
      removeDotGraph(absDotName);
   }

   return baseName;
}

bool DotDirDeps::isTrivial() const
{
   return m_dir->depGraphIsTrivial();
}


void generateGraphLegend(const QString &path)
{
   QDir d(path);

   // store the original directory
   if (! d.exists()) {
      err("Output dir %s does not exist\n", qPrintable(path));
      exit(1);
   }

   QString theGraph;
   QTextStream md5stream(&theGraph);
   writeGraphHeader(md5stream, theTranslator->trLegendTitle());

   md5stream << "  Node9 [shape=\"box\",label=\"Inherited\",fontsize=\"" << FONTSIZE << "\",height=0.2,width=0.4,fontname=\"" << FONTNAME <<
             "\",fillcolor=\"grey75\",style=\"filled\" fontcolor=\"black\"];\n";
   md5stream << "  Node10 -> Node9 [dir=\"back\",color=\"midnightblue\",fontsize=\"" << FONTSIZE << "\",style=\"solid\",fontname=\"" << FONTNAME <<
             "\"];\n";
   md5stream << "  Node10 [shape=\"box\",label=\"PublicBase\",fontsize=\"" << FONTSIZE << "\",height=0.2,width=0.4,fontname=\"" << FONTNAME <<
             "\",color=\"black\",URL=\"$classPublicBase" << Doxy_Globals::htmlFileExtension << "\"];\n";
   md5stream << "  Node11 -> Node10 [dir=\"back\",color=\"midnightblue\",fontsize=\"" << FONTSIZE << "\",style=\"solid\",fontname=\"" << FONTNAME <<
             "\"];\n";
   md5stream << "  Node11 [shape=\"box\",label=\"Truncated\",fontsize=\"" << FONTSIZE << "\",height=0.2,width=0.4,fontname=\"" << FONTNAME <<
             "\",color=\"red\",URL=\"$classTruncated" << Doxy_Globals::htmlFileExtension << "\"];\n";
   md5stream << "  Node13 -> Node9 [dir=\"back\",color=\"darkgreen\",fontsize=\"" << FONTSIZE << "\",style=\"solid\",fontname=\"" << FONTNAME <<
             "\"];\n";
   md5stream << "  Node13 [shape=\"box\",label=\"ProtectedBase\",fontsize=\"" << FONTSIZE << "\",height=0.2,width=0.4,fontname=\"" << FONTNAME <<
             "\",color=\"black\",URL=\"$classProtectedBase" << Doxy_Globals::htmlFileExtension << "\"];\n";
   md5stream << "  Node14 -> Node9 [dir=\"back\",color=\"firebrick4\",fontsize=\"" << FONTSIZE << "\",style=\"solid\",fontname=\"" << FONTNAME <<
             "\"];\n";
   md5stream << "  Node14 [shape=\"box\",label=\"PrivateBase\",fontsize=\"" << FONTSIZE << "\",height=0.2,width=0.4,fontname=\"" << FONTNAME <<
             "\",color=\"black\",URL=\"$classPrivateBase" << Doxy_Globals::htmlFileExtension << "\"];\n";
   md5stream << "  Node15 -> Node9 [dir=\"back\",color=\"midnightblue\",fontsize=\"" << FONTSIZE << "\",style=\"solid\",fontname=\"" << FONTNAME <<
             "\"];\n";
   md5stream << "  Node15 [shape=\"box\",label=\"Undocumented\",fontsize=\"" << FONTSIZE << "\",height=0.2,width=0.4,fontname=\"" << FONTNAME <<
             "\",color=\"grey75\"];\n";
   md5stream << "  Node16 -> Node9 [dir=\"back\",color=\"midnightblue\",fontsize=\"" << FONTSIZE << "\",style=\"solid\",fontname=\"" << FONTNAME <<
             "\"];\n";
   md5stream << "  Node16 [shape=\"box\",label=\"Templ< int >\",fontsize=\"" << FONTSIZE << "\",height=0.2,width=0.4,fontname=\"" << FONTNAME <<
             "\",color=\"black\",URL=\"$classTempl" << Doxy_Globals::htmlFileExtension << "\"];\n";
   md5stream << "  Node17 -> Node16 [dir=\"back\",color=\"orange\",fontsize=\"" << FONTSIZE << "\",style=\"dashed\",label=\"< int >\",fontname=\"" <<
             FONTNAME << "\"];\n";
   md5stream << "  Node17 [shape=\"box\",label=\"Templ< T >\",fontsize=\"" << FONTSIZE << "\",height=0.2,width=0.4,fontname=\"" << FONTNAME <<
             "\",color=\"black\",URL=\"$classTempl" << Doxy_Globals::htmlFileExtension << "\"];\n";
   md5stream << "  Node18 -> Node9 [dir=\"back\",color=\"darkorchid3\",fontsize=\"" << FONTSIZE <<
             "\",style=\"dashed\",label=\"m_usedClass\",fontname=\"" << FONTNAME << "\"];\n";
   md5stream << "  Node18 [shape=\"box\",label=\"Used\",fontsize=\"" << FONTSIZE << "\",height=0.2,width=0.4,fontname=\"" << FONTNAME <<
             "\",color=\"black\",URL=\"$classUsed" << Doxy_Globals::htmlFileExtension << "\"];\n";
   writeGraphFooter(md5stream);

   QString sigStr;
   sigStr = QCryptographicHash::hash(theGraph.toUtf8(), QCryptographicHash::Md5).toHex();

   QString absBaseName = path + "/graph_legend";
   QString absDotName  = absBaseName + ".dot";
   QString imgExt      = Config::getEnum("dot-image-format");
   QString imgName     = "graph_legend." + imgExt;
   QString absImgName  = absBaseName + "." + imgExt;

   if (checkAndUpdateMd5Signature(absBaseName, sigStr) || ! checkDeliverables(absImgName)) {
      QFile dotFile(absDotName);

      if (! dotFile.open(QIODevice::WriteOnly)) {     
         err("Unable to open file for writing %s, error: %d\n", qPrintable(absDotName), dotFile.error());
         return;
      }

      QTextStream dotText(&dotFile);
      dotText << theGraph;
      dotFile.close();

      // run dot to generate the a bitmap image from the graph
      DotRunner *dotRun = new DotRunner(absDotName, d.absolutePath(), true, absImgName);
      dotRun->addJob(imgExt, absImgName);
      DotManager::instance()->addRun(dotRun);

   } else {
      removeDotGraph(absDotName);
   }
   Doxy_Globals::indexList->addImageFile(imgName);

   if (imgExt == "svg") {
      DotManager::instance()->addSVGObject(absBaseName + Doxy_Globals::htmlFileExtension, "graph_legend", absImgName, QString());
   }
}

void writeDotGraphFromFile(const QString &inFile, const QString &outDir, const QString &outFile, GraphOutputFormat format)
{
   QDir d(outDir);

   if (! d.exists()) {
      err("Output directory %s does not exist\n", qPrintable(outDir));
      exit(1);
   }

   QString imgExt     = Config::getEnum("dot-image-format");;
   QString imgName    = outFile + "." + imgExt;
   QString absImgName = d.absolutePath() + "/" + imgName;
   QString absOutFile = d.absolutePath() + "/" + outFile;

   DotRunner dotRun(inFile, d.absolutePath(), false, qPrintable(absImgName));

   if (format == GOF_BITMAP) {
      dotRun.addJob(imgExt, absImgName);

   } else { 
      // format==GOF_EPS

      if (Config::getBool("latex-pdf")) {
         dotRun.addJob("pdf", absOutFile + ".pdf");

      } else {
         dotRun.addJob("ps", absOutFile + ".eps");

      }
   }

   dotRun.preventCleanUp();
   if (! dotRun.run()) {
      return;
   }

   if (format == GOF_BITMAP) {
      checkDotResult(absImgName);
   }

   Doxy_Globals::indexList->addImageFile(imgName);

}


/*! Writes user defined image map to the output.
 *  \param t text stream to write to
 *  \param inFile just the basename part of the filename
 *  \param outDir output directory
 *  \param relPath relative path the to root of the output dir
 *  \param baseName the base name of the output files
 *  \param context the scope in which this graph is found (for resolving links)
 *  \param graphId a unique id for this graph, use for dynamic sections
 */
void writeDotImageMapFromFile(QTextStream &t, const QString &inFile, const QString &outDir, const QString &relPath, 
                  const QString &baseName, const QString &context, int graphId)
{
   QDir d(outDir);

   if (! d.exists()) {
      err("Output dir %s does not exist\n", qPrintable(outDir));
      exit(1);
   }

   QString mapName    = baseName + ".map";
   QString imgExt     = Config::getEnum("dot-image-format");
   QString imgName    = baseName + "." + imgExt;
   QString absOutFile = d.absolutePath() + "/" + mapName;

   DotRunner dotRun(inFile, d.absolutePath(), false);
   dotRun.addJob(MAP_CMD, absOutFile);
   dotRun.preventCleanUp();

   if (! dotRun.run()) {
      return;
   }

   if (imgExt == "svg") { 
      // vector graphics
    
      QString svgName = outDir + "/" + baseName + ".svg";
      writeSVGFigureLink(t, relPath, baseName, svgName);

      DotFilePatcher patcher(svgName);
      patcher.addSVGConversion(relPath, true, context, true, graphId);
      patcher.run();

   } else { // bitmap graphics
      t << "<img src=\"" << relPath << imgName << "\" alt=\""
        << imgName << "\" border=\"0\" usemap=\"#" << mapName << "\"/>" << endl
        << "<map name=\"" << mapName << "\" id=\"" << mapName << "\">";

      convertMapFile(t, absOutFile, relPath, true, context);

      t << "</map>" << endl;
   }

   d.remove(absOutFile);
}

DotGroupCollaboration::DotGroupCollaboration(QSharedPointer<GroupDef> gd)
{
   m_curNodeId = 0;
   QString tmp_url = gd->getReference() + "$" + gd->getOutputFileBase();

   m_usedNodes = new QHash<QString, DotNode *>;

   m_rootNode = new DotNode(m_curNodeId++, gd->groupTitle(), "", tmp_url, true);
   m_rootNode->markAsVisible();
   m_usedNodes->insert(gd->name(), m_rootNode ); 
   m_diskName = gd->getOutputFileBase();

   buildGraph(gd);
}

DotGroupCollaboration::~DotGroupCollaboration()
{
   delete m_usedNodes;
}

void DotGroupCollaboration::buildGraph(QSharedPointer<GroupDef> gd)
{
   QString tmp_url;  

   // Write parents
   SortedList<QSharedPointer<GroupDef>> *groups = gd->partOfGroups();

   if ( groups ) {      
      for (auto d : *groups) {
         DotNode *nnode = m_usedNodes->value(d->name());

         if ( !nnode ) {
            // add node
            tmp_url = d->getReference() + "$" + d->getOutputFileBase();
            QString tooltip = d->briefDescriptionAsTooltip();

            nnode = new DotNode(m_curNodeId++, d->groupTitle(), tooltip, tmp_url);
            nnode->markAsVisible();
            m_usedNodes->insert(d->name(), nnode );
         }

         tmp_url = "";
         addEdge( nnode, m_rootNode, DotGroupCollaboration::thierarchy, tmp_url, tmp_url );
      }
   }

   // Add subgroups
   if ( gd->getSubGroups() && gd->getSubGroups()->count() ) {
      
      for (auto def : *gd->getSubGroups()) {
         DotNode *nnode = m_usedNodes->value(def->name());

         if ( ! nnode ) {
            // add node
            tmp_url = def->getReference() + "$" + def->getOutputFileBase();

            QString tooltip = def->briefDescriptionAsTooltip();

            nnode = new DotNode(m_curNodeId++, def->groupTitle(), tooltip, tmp_url);
            nnode->markAsVisible();
            m_usedNodes->insert(def->name(), nnode );
         }

         tmp_url = "";
         addEdge( m_rootNode, nnode, DotGroupCollaboration::thierarchy, tmp_url, tmp_url);
      }
   }

   // Write collaboration

   // Add members
   addMemberList( gd->getMemberList(MemberListType_allMembersList) );
   QString htmlEntenstion = Doxy_Globals::htmlFileExtension;

   // Add classes
   if ( gd->getClasses() && gd->getClasses()->count() ) {

      for (auto def : *gd->getClasses()) {
         tmp_url = def->getReference() + "$" + def->getOutputFileBase() + htmlEntenstion;

         if (!def->anchor().isEmpty()) {
            tmp_url += "#" + def->anchor();
         }
         addCollaborationMember(def, tmp_url, DotGroupCollaboration::tclass);
      }
   }

   // Add namespaces
   if ( gd->getNamespaces() && gd->getNamespaces()->count() ) {     
      for (auto def : *gd->getNamespaces()) {
         tmp_url = def->getReference() + "$" + def->getOutputFileBase() + htmlEntenstion;
         addCollaborationMember(def, tmp_url, DotGroupCollaboration::tnamespace );
      }
   }

   // Add files
   if ( gd->getFiles() && gd->getFiles()->count() ) {     
      for (auto def : *gd->getFiles()) {
         tmp_url = def->getReference() + "$" + def->getOutputFileBase() + htmlEntenstion;
         addCollaborationMember(def, tmp_url, DotGroupCollaboration::tfile );
      }
   }

   // Add pages
   if ( gd->getPages() && gd->getPages()->count() ) {


      for (auto def : *gd->getPages()) {
         tmp_url = def->getReference() + "$" + def->getOutputFileBase() + htmlEntenstion;
         addCollaborationMember(def, tmp_url, DotGroupCollaboration::tpages );
      }
   }

   // Add directories
   if ( gd->getDirs() && gd->getDirs()->count() ) {

      for (auto def : *gd->getDirs()) {
         tmp_url = def->getReference() + "$" + def->getOutputFileBase() + htmlEntenstion;
         addCollaborationMember(def, tmp_url, DotGroupCollaboration::tdir );
      }
   }
}

void DotGroupCollaboration::addMemberList(QSharedPointer<MemberList> ml)
{
   if ( !( ml && ml->count()) ) {
      return;
   }
  
   for (auto def : *ml) {
      QString tmp_url = def->getReference() + "$" + def->getOutputFileBase() + Doxy_Globals::htmlFileExtension +
                           + "#" + def->anchor();

      addCollaborationMember(def, tmp_url, DotGroupCollaboration::tmember );
   }
}

DotGroupCollaboration::Edge *DotGroupCollaboration::addEdge(DotNode *x_pNStart, DotNode *x_pNEnd, EdgeType x_eType,
                  const QString &x_label, const QString &x_url)
{
   Edge *newEdge = 0;   

   // search a existing link 
   for (auto item : m_edges) {
      if ( item->pNStart == x_pNStart && newEdge->pNEnd == x_pNEnd && item->eType == x_eType) {
         // edge already found
         newEdge = item;
         break;
      }
   }

   if (newEdge == 0) { 
      // new link
      newEdge = new Edge(x_pNStart, x_pNEnd, x_eType);
      m_edges.append(newEdge);
   }

   if (! x_label.isEmpty()) {
      newEdge->m_links.append(new Link(x_label, x_url));
   }

   return newEdge;
}

void DotGroupCollaboration::addCollaborationMember(QSharedPointer<Definition> def, const QString &url, EdgeType eType )
{
   // Create group nodes
   if (! def->partOfGroups()) {
      return;
   }
  
   QString tmp_str;

   for (auto d : *def->partOfGroups()) {
      DotNode *nnode = m_usedNodes->value(d->name());

      if ( nnode != m_rootNode ) {
         if ( nnode == 0 ) {
            // add node
            tmp_str = d->getReference() + "$" + d->getOutputFileBase();

            QString tooltip = d->briefDescriptionAsTooltip();

            nnode = new DotNode(m_curNodeId++, d->groupTitle(), tooltip, tmp_str);
            nnode->markAsVisible();
            m_usedNodes->insert(d->name(), nnode );
         }

         tmp_str = def->qualifiedName();
         addEdge( m_rootNode, nnode, eType, tmp_str, url );
      }
   }
}


QString DotGroupCollaboration::writeGraph( QTextStream &t, GraphOutputFormat graphFormat, 
      EmbeddedOutputFormat textFormat, const QString &path, const QString &fileName, const QString &relPath,
      bool writeImageMap, int graphId) const
{
   QDir d(path);

   // store the original directory
   if (! d.exists()) {
      err("Output directory %s does not exist\n", qPrintable(path));
      exit(1);
   }

   static bool usePDFLatex = Config::getBool("latex-pdf");

   QString theGraph;
   QTextStream md5stream(&theGraph);

   writeGraphHeader(md5stream, m_rootNode->label());

   // clean write flags
   for (auto pn : *m_usedNodes) {
      pn->clearWriteFlag();
   }

   // write other nodes  
   for (auto pn : *m_usedNodes) {
      pn->write(md5stream, DotNode::Inheritance, graphFormat, true, false, false, false);
   }

   // write edges
   for (auto edge : m_edges) {
      edge->write( md5stream );
   }

   writeGraphFooter(md5stream);
   resetReNumbering();

   QString sigStr;
   sigStr = QCryptographicHash::hash(theGraph.toUtf8(), QCryptographicHash::Md5).toHex();
  
   QString imgExt      = Config::getEnum("dot-image-format");
   QString baseName    = m_diskName;
   QString imgName     = baseName + "." + imgExt;
   QString absPath     = d.absolutePath();
   QString absBaseName = absPath + "/" + baseName;
   QString absDotName  = absBaseName + ".dot";
   QString absImgName  = absBaseName + "." + imgExt;
   QString absMapName  = absBaseName + ".map";
   QString absPdfName  = absBaseName + ".pdf";
   QString absEpsName  = absBaseName + ".eps";

   bool regenerate = false;
   bool ok;

   bool x  = checkAndUpdateMd5Signature(absBaseName, sigStr); 

   if (x) {
      ok = true;

   } else {

      QString arg1;
      if (graphFormat == GOF_BITMAP) {
         arg1 = absImgName; 

      } else if (usePDFLatex) { 
         arg1 = absPdfName;

      } else {
         arg1 =absEpsName;

      }      

      QString arg2;
      if (graphFormat == GOF_BITMAP) {
         arg2 = absMapName;
      }

      bool y = checkDeliverables(arg1, arg2);        

      if (y) {
         ok = false;
   
      } else {
         ok = true;         
      }           

   }  

   if (ok) {
      regenerate = true;

      QFile dotfile(absDotName);
      if (dotfile.open(QIODevice::WriteOnly)) {
         QTextStream tdot(&dotfile);
         tdot << theGraph;
         dotfile.close();
      }

      if (graphFormat == GOF_BITMAP) { 
         // run dot to create a bitmap image
       
         DotRunner *dotRun = new DotRunner(absDotName, d.absolutePath(), false);

         dotRun->addJob(imgExt, absImgName);
         if (writeImageMap) {
            dotRun->addJob(MAP_CMD, absMapName);
         }
         DotManager::instance()->addRun(dotRun);

      } else if (graphFormat == GOF_EPS) {
         DotRunner *dotRun = new DotRunner(absDotName, d.absolutePath(), false);

         if (usePDFLatex) {
            dotRun->addJob("pdf", absPdfName);
         } else {
            dotRun->addJob("ps", absEpsName);
         }
         DotManager::instance()->addRun(dotRun);
      }

   }

   if (graphFormat == GOF_BITMAP && textFormat == EOF_DocBook) {
      t << "<para>" << endl;
      t << "    <figure>" << endl;
      t << "        <title>Group Collaboration diagram";
      t << "</title>" << endl;
      t << "        <mediaobject>" << endl;
      t << "            <imageobject>" << endl;
      t << "                <imagedata";
      t << " width=\"50%\" align=\"center\" valign=\"middle\" scalefit=\"1\" fileref=\"" << relPath << baseName << "." << imgExt << "\">";
      t << "</imagedata>" << endl;
      t << "            </imageobject>" << endl;
      t << "        </mediaobject>" << endl;
      t << "    </figure>" << endl;
      t << "</para>" << endl;

   } else if (graphFormat == GOF_BITMAP && writeImageMap) {
      QString mapLabel = escapeCharsInString(baseName, false);
      t << "<center><table><tr><td>";

      if (imgExt == "svg") {
         t << "<div class=\"center\">";

         if (regenerate || !writeSVGFigureLink(t, relPath, baseName, absImgName)) { 
            // need to patch the links in the generated SVG file

            if (regenerate) {
               DotManager::instance()->addSVGConversion(absImgName, relPath, false, QString(), true, graphId);
            }

            int mapId = DotManager::instance()->addSVGObject(fileName, baseName, absImgName, relPath);
            t << "<!-- SVG " << mapId << " -->" << endl;
         }

         t << "</div>" << endl;

      } else {
         t << "<img src=\"" << relPath << imgName
           << "\" border=\"0\" alt=\"\" usemap=\"#"
           << mapLabel << "\"/>" << endl;
         if (regenerate || !insertMapFile(t, absMapName, relPath, mapLabel)) {
            int mapId = DotManager::instance()->addMap(fileName, absMapName, relPath,
                        false, QString(), mapLabel);
            t << "<!-- MAP " << mapId << " -->" << endl;
         }
      }
      t << "</td></tr></table></center>" << endl;

   } else if (graphFormat == GOF_EPS) {
      if (regenerate || ! writeVecGfxFigure(t, baseName, absBaseName)) {
         int figId = DotManager::instance()->addFigure(fileName, baseName, absBaseName, false);
         t << endl << "% FIG " << figId << endl;
      }
   }

   if (!regenerate) {
      removeDotGraph(absDotName);
   }

   return baseName;
}

void DotGroupCollaboration::Edge::write(QTextStream &t) const
{
   const char *linkTypeColor[] = { "darkorchid3", "orange", "blueviolet", "darkgreen", "firebrick4", "grey75", "midnightblue"};

   QString arrowStyle = "dir=\"none\", style=\"dashed\"";
   t << "  Node" << pNStart->number();
   t << "->";
   t << "Node" << pNEnd->number();

   t << " [shape=plaintext";
   if (m_links.count() > 0) { // there are links
      t << ", ";    
      t << "label=\"";
     
      bool first = true;
      int count = 0;
      const int maxLabels = 10;

      for (auto link : m_links) {

         if (count >= maxLabels) {
            break;
         }

         if (first) {
            first = false;

         } else {
            t << "\\n";
         }

         t << convertLabel(link->label);
         ++count;
      }

      if (count == maxLabels) {
         t << "\\n...";
      }
      t << "\"";

   }

   switch ( eType ) {
      case thierarchy:
         arrowStyle = "dir=\"back\", style=\"solid\"";
         break;
      default:
         t << ", color=\"" << linkTypeColor[eType] << "\"";
         break;
   }
   t << ", " << arrowStyle;
   t << "];" << endl;
}

bool DotGroupCollaboration::isTrivial() const
{
   return m_usedNodes->count() <= 1;
}

void DotGroupCollaboration::writeGraphHeader(QTextStream &t, const QString &title) const
{
   t << "digraph ";
   if (title.isEmpty()) {
      t << "\"Dot Graph\"";
   } else {
      t << "\"" << convertToXML(title) << "\"";
   }

   t << endl;
   t << "{" << endl;

   if (Config::getBool("dot-transparent")) {
      t << "  bgcolor=\"transparent\";" << endl;
   }

   t << "  edge [fontname=\"" << FONTNAME << "\",fontsize=\"" << FONTSIZE << "\","
     "labelfontname=\"" << FONTNAME << "\",labelfontsize=\"" << FONTSIZE << "\"];\n";
   t << "  node [fontname=\"" << FONTNAME << "\",fontsize=\"" << FONTSIZE << "\",shape=record];\n";
   t << "  rankdir=LR;\n";
}

void writeDotDirDepGraph(QTextStream &t, QSharedPointer<DirDef> dd)
{
   t << "digraph \"" << dd->displayName() << "\" {\n";

     if (Config::getBool("dot-transparent")) {
      t << "  bgcolor=transparent;\n";
   }

   t << "  compound=true\n";
   t << "  node [ fontsize=\"" << FONTSIZE << "\", fontname=\"" << FONTNAME << "\"];\n";
   t << "  edge [ labelfontsize=\"" << FONTSIZE << "\", labelfontname=\"" << FONTNAME << "\"];\n";

   QHash<QString, QSharedPointer<DirDef>> dirsInGraph;
   dirsInGraph.insert(dd->getOutputFileBase(), dd);

   if (dd->parent()) {
      t << "  subgraph cluster" << dd->parent()->getOutputFileBase() << " {\n";
      t << "    graph [ bgcolor=\"#ddddee\", pencolor=\"black\", label=\""
        << dd->parent()->shortName()
        << "\" fontname=\"" << FONTNAME << "\", fontsize=\"" << FONTSIZE << "\", URL=\"";
      t << dd->parent()->getOutputFileBase() << Doxy_Globals::htmlFileExtension;
      t << "\"]\n";
   }

   if (dd->isCluster()) {
      t << "  subgraph cluster" << dd->getOutputFileBase() << " {\n";
      t << "    graph [ bgcolor=\"#eeeeff\", pencolor=\"black\", label=\"\""
        << " URL=\"" << dd->getOutputFileBase() << Doxy_Globals::htmlFileExtension
        << "\"];\n";
      t << "    " << dd->getOutputFileBase() << " [shape=plaintext label=\""
        << dd->shortName() << "\"];\n";

      // add nodes for sub directories   
      for (auto sdir : dd->subDirs()) {
         t << "    " << sdir->getOutputFileBase() << " [shape=box label=\""
           << sdir->shortName() << "\"";

         if (sdir->isCluster()) {
            t << " color=\"red\"";
         } else {
            t << " color=\"black\"";
         }

         t << " fillcolor=\"white\" style=\"filled\"";
         t << " URL=\"" << sdir->getOutputFileBase()
           << Doxy_Globals::htmlFileExtension << "\"";
         t << "];\n";

         dirsInGraph.insert(sdir->getOutputFileBase(), sdir);
      }
      t << "  }\n";

   } else {
      t << "  " << dd->getOutputFileBase() << " [shape=box, label=\""
        << dd->shortName() << "\", style=\"filled\", fillcolor=\"#eeeeff\","
        << " pencolor=\"black\", URL=\"" << dd->getOutputFileBase()
        << Doxy_Globals::htmlFileExtension << "\"];\n";
   }

   if (dd->parent()) {
      t << "  }\n";
   }

   // add nodes for other used directories
   for (auto udir : dd->usedDirs()) {
      // for each used dir (=directly used or a parent of a directly used dir)
   
      QSharedPointer<DirDef> usedDir = udir->dir().constCast<DirDef>();
      QSharedPointer<DirDef> dir = dd;

      while (dir) {
        
         if (dir != usedDir && dir->parent() == usedDir->parent() && !usedDir->isParentOf(dd)) {
            // include if both have the same parent (or no parent)
         
            t << "  " << usedDir->getOutputFileBase() << " [shape=box label=\""
              << usedDir->shortName() << "\"";

            if (usedDir->isCluster()) {
               if (! Config::getBool("dot-transparent")) {
                  t << " fillcolor=\"white\" style=\"filled\"";
               }
               t << " color=\"red\"";
            }

            t << " URL=\"" << usedDir->getOutputFileBase()
              << Doxy_Globals::htmlFileExtension << "\"];\n";

            dirsInGraph.insert(usedDir->getOutputFileBase(), usedDir);
            break;
         }

         dir = dir->parent();
      }
   }

   // add relations between all selected directories
   for (auto dir : dirsInGraph) {

      // foreach dir in the graph     
      for (auto udir : dir->usedDirs()) {

         // foreach used dir
         QSharedPointer<DirDef> usedDir = udir->dir().constCast<DirDef>();

         if ((dir != dd || ! udir->inherited()) && (usedDir != dd || ! udir->inherited()) && 
               ! usedDir->isParentOf(dir) && dirsInGraph.contains(usedDir->getOutputFileBase())) { 

            // only point to nodes that are in the graph
            QString relationName;
            relationName = QString("dir_%1_%2").arg(dir->dirCount(), 6, 10, QChar('0')).arg(usedDir->dirCount(), 6, 10, QChar('0'));

            if (Doxy_Globals::dirRelations.find(relationName) == 0) {
               // new relation
               Doxy_Globals::dirRelations.insert(relationName, QSharedPointer<DirRelation>(new DirRelation(relationName, dir, udir)));
            }

            int nrefs = udir->filePairs().count();

            t << "  " << dir->getOutputFileBase() << "->"
              << usedDir->getOutputFileBase();

            t << " [headlabel=\"" << nrefs << "\", labeldistance=1.5";
            t << " headhref=\"" << relationName << Doxy_Globals::htmlFileExtension
              << "\"];\n";
         }
      }
   }

   t << "}\n";

}
