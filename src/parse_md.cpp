/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim 
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

#include <QRegExp>
#include <QFileInfo>
#include <QHash>

#include <stdio.h>

#include <parse_md.h>

#include <config.h>
#include <commentscan.h>
#include <commentcnv.h>
#include <doxy_globals.h>
#include <entry.h>
#include <message.h>
#include <util.h>

struct LinkRef {
   LinkRef(const QString &l, const QString &t) : link(l), title(t) {}
   QString link;
   QString title;
};

enum Alignment { AlignNone, AlignLeft, AlignCenter, AlignRight };

static QHash<QString, LinkRef> g_linkRefs;
static QSharedPointer<Entry>   g_current;
static QString  g_fileName;
static int      g_lineNr;

// In case a markdown page starts with a level1 header, that header is used as a title of 
// of the page, in effect making it a level0 header. So the level of all other sections 
// will need to be corrected. This flag is true if corrections are needed. 
// static bool g_correctSectionLevel;

const int codeBlockIndent = 4;

static void processInline(QString &out, const QString &processText, int size);

// is character allowed before an emphasis section
static bool isOpenEmphChar(const QChar &text)
{
   bool retval = false;

   if (text == '\n' || text ==' ' || text =='\'' || text =='<' || text =='{'  ||text =='(' || 
      text == '['  || text ==',' || text ==':'  || text ==';') {

      retval = true;
   }

   return retval;
}

// is character an escape that prevents ending an emphasis section
// for example *bla (*.txt) is cool*
static bool ignoreCloseEmphChar(const QChar &text)
{
   bool retval = false;

   if ( text =='(' || text =='{' || text =='['  || text =='<' || text =='=' || 
        text =='+' || text =='-' || text =='\\' || text =='@') {

      retval = true;
   }
  
   return retval;
}

// is character in data part of an identifier
static bool isIdChar(const QChar &text)
{
   bool retval = false;

   if ( (text >= 'a' && text <= 'z') || (text >= 'A' && text <= 'Z') || (text >= '0' && text <= '9') ||
         text.unicode() >= 0x80) {

      retval = true;
   }
  
   return retval;
}

// escape characters that have a special meaning later on
static QString escapeSpecialChars(const QString &text)
{
   if (text.isEmpty()) {
      return "";
   }

   QString retval;   

   for (auto c : text) {

      switch (c.unicode()) {
         case '<':
            retval += "\\<";
            break;

         case '>':
            retval += "\\>";
            break;

         case '\\':
            retval += "\\\\";
            break;

         case '@':
            retval += "\\@";
            break;

         default:
            retval += c;
            break;
      }
   }

   return retval;
}

/** helper function to convert presence of left and/or right alignment markers
 *  to a alignment value
 */
static Alignment markersToAlignment(bool leftMarker, bool rightMarker)
{
   if (leftMarker && rightMarker) {
      return AlignCenter;

   } else if (leftMarker) {
      return AlignLeft;

   } else if (rightMarker) {
      return AlignRight;

   } else {
      return AlignNone;
   }
}


// Check if data contains a block command. If so returned the command which ends the block. 
// If not an empty string is returned. Note When offset>0 character position -1 will be inspected.
//
// Checks for and skip the following block commands:
// {@code .. { .. } .. }
// \dot .. \enddot
// \code .. \endcode
// \msc .. \endmsc
// \f$..\f$
// \f[..\f]
// \f{..\f}
// \verbatim..\endverbatim
// \latexonly..\endlatexonly
// \htmlonly..\endhtmlonly
// \xmlonly..\endxmlonly
// \rtfonly..\endrtfonly
// \manonly..\endmanonly

static QString isBlockCommand(const QString &data, int offset, int size, const QString &pristineChars1)
{
   QString retval = "";

   bool openBracket = false;
   bool isEscaped   = false;

   if (offset > 0 && ! pristineChars1.isEmpty())  {    
      QChar char1 = pristineChars1[0];

      openBracket = char1 == '{';
      isEscaped   = char1 == '\\' || char1 == '@';
   }

   if (isEscaped) {
      return retval;
   }

   int end = 1;

   while (end < size && (data[end] >= 'a' && data[end] <= 'z')) {
      end++;
   }

   if (end == 1) {
      return retval;
   }

   QString blockName = data.mid(1, end - 1);       

   if (blockName == "code" && openBracket) {
      return "}";

   } else if (blockName == "dot"         ||
              blockName == "code"        ||
              blockName == "msc"         ||
              blockName == "verbatim"    ||
              blockName == "latexonly"   ||
              blockName == "htmlonly"    ||
              blockName == "xmlonly"     ||
              blockName == "rtfonly"     ||
              blockName == "manonly"     ||
              blockName == "docbookonly" ) {

      return "end" + blockName;

   } else if (blockName == "startuml") {
      return "enduml";

   } else if (blockName == "f" && end < size) {
      if (data[end] == '$') {
         return "f$";

      } else if (data[end] == '[') {
         return "f]";

      } else if (data[end] == '}') {
         return "f}";

      }
   }

   return retval;
}

/** looks for the next emph char, skipping other constructs, and
 *  stopping when either it is found, or we are at the end of a paragraph.
 */
static int findEmphasisChar(const QString &data, int size, QChar c, int c_size)
{
   int i = 1;

   while (i < size) {

      while (i < size && data[i] != c && data[i] != '`' &&
             data[i] != '\\' && data[i] != '@' && data[i] != '\n') {
         i++;
      }
      
      // not counting escaped chars or characters that are unlikely
      // to appear as the end of the emphasis char

      int len = data.length();

      QChar charA = 0;   
      if (! i -1 > len) {
         charA = data[i - 1];
      }

      if (i > 0 && ignoreCloseEmphChar(charA)) {
         i++;
         continue;

      } else {
         // get length of emphasis token
         int tokenLen = 0;

         while (i + tokenLen < size && data[i + tokenLen] == c) {
            tokenLen++;
         }

         if (tokenLen > 0) {

            QChar charB = 0;   
            if (! i + tokenLen > len) {
               charB = data[i + tokenLen];
            }

            if (tokenLen != c_size || (i < size - tokenLen && isIdChar(charB))) { 
               // to prevent touching some_underscore_identifier
               i = i + tokenLen;
               continue;
            }

            return i; // found it
         }
      }

      // skipping a code span
      if (i < len && data[i] == '`') {
         int snb = 0;

         while (i < size && data[i] == '`') {
            snb++;
            i++;
         }

         // find same pattern to end the span
         int enb = 0;

         while (i < size && enb < snb) {
            if (data[i] == '`') {
               enb++;
            }

            if (snb == 1 && data[i] == '\'') {
               break;   // ` ended by '
            }

            i++;
         }

      } else if (i < len && (data[i] == '@' || data[i] == '\\') ) {
         // skip over blocks that should not be processed, passing pristineChars1 as 4th
         QString endBlockName = isBlockCommand(data.mid(i), i, size - i, data.mid(i - 1, 1) );

         QChar charC = 0;   
         if (! i + 1 > len) {
            charC = data[i + 1];
         }

         if (! endBlockName.isEmpty()) {
            i++;
            int l = endBlockName.length();

            while (i < size - l) {
               if ((data[i] == '\\' || data[i] == '@') && data[i - 1] != '\\' && data[i - 1] != '@') { 

                  if (data.mid(i + 1).startsWith(endBlockName)) { 
                     break;
                  }
               }
               i++;
            }

         } else if (i < size - 1 && isIdChar(charC)) { 
            // @cmd, stop processing, see bug 690385
            return 0;

         } else {
            i++;

         }

      } else if (i < len && data[i] == '\n') { 
         // end * or _ at paragraph boundary

         i++;
         while (i < size && data[i] == ' ') {
            i++;
         }

         if (i >= size || data[i] == '\n') {
            return 0;   // empty line -> paragraph
         }

      } else { 
         // should not get here
         i++;
      }

   }

   return 0;
}

/** process single emphasis */
static int processEmphasis1(QString &out, const QString &data, int size, QChar c)
{
   int i = 0;
   int len;

   /* skipping one symbol if coming from emph3 */
   if (size > 1 && data[0] == c && data[1] == c) {
      i = 1;
   }

   while (i < size) {
      len = findEmphasisChar(data.mid(i), size - i, c, 1);

      if (len == 0) {
         return 0;
      }

      i += len;

      if (i >= size) {
         return 0;
      }

      if (i + 1 < size && data[i + 1] == c) {
         i++;
         continue;
      }

      if (data[i] == c && data[i - 1] != ' ' && data[i - 1] != '\n') {
         out += "<em>";
         processInline(out, data, i);
         out += "</em>";

         return i + 1;
      }
   }

   return 0;
}

/** process double emphasis */
static int processEmphasis2(QString &out, const QString &data, int size, QChar c)
{
   int i = 0;
   int len;

   while (i < size) {
      len = findEmphasisChar(data.mid(i), size - i, c, 2);

      if (len == 0) {
         return 0;
      }

      i += len;

      if (i + 1 < size && data[i] == c && data[i + 1] == c && i && data[i - 1] != ' ' && data[i - 1] != '\n') {
         out += "<strong>";
         processInline(out, data, i);
         out += "</strong>";

         return i + 2;
      }

      i++;
   }

   return 0;
}

/** Parsing tripple emphasis
 *  Finds the first closing tag, and delegates to the other emph
 */
static int processEmphasis3(QString &out, const QString &data, int size, QChar c, const QString pristineChars2)
{
   int i = 0, len;

   while (i < size) {
      len = findEmphasisChar(data.mid(i), size - i, c, 3);

      if (len == 0) {
         return 0;
      }

      i += len;

      /* skip whitespace preceded symbols */
      if (data[i] != c || data[i - 1] == ' ' || data[i - 1] == '\n') {
         continue;
      }

      if (i + 2 < size && data[i + 1] == c && data[i + 2] == c) {
         out += "<em><strong>";
         processInline(out, data, i);

         out += "</strong></em>";

         return i + 3;

      } else if (i + 1 < size && data[i + 1] == c) {
         // double symbol found, handing over to emphasis1 
         len = processEmphasis1(out, pristineChars2 + data, size + 2, c);

         if (len == 0) {
            return 0;
         } else {
            return len - 2;
         }

      } else {
         // single symbol found, handing over to emphasis2 
         len = processEmphasis2(out, pristineChars2.mid(1) + data, size + 1, c);

         if (len == 0) {
            return 0;

         } else {
            return len - 1;
         }
      }
   }

   return 0;
}

/** Process ndash and mdashes */
static int processNmdash(QString &out, const QString &data, int off, int size, const QString &pristineChars8)
{ 
   int i     = 1;
   int count = 1;

   if (i < size && data[i] == '-') { // found --
      count++;
      i++;
   }

   if (i < size && data[i] == '-') { // found ---
      count++;
      i++;
   }

   if (i < size && data[i] == '-') { // found ----
      count++;
   }

   if (count == 2 && (off < 8 || pristineChars8 != "operator") ) { 
      // -- => ndash
      out += "&ndash;";
      return 2;

   } else if (count == 3) { 
      // --- => mdash
      out += "&mdash;";
      return 3;
   }

   // not an ndash or mdash

   return 0;
}

/** Process quoted section "...", can contain one embedded newline */
static int processQuoted(QString &out, const QString &data, int, int size)
{
   int i  = 1;
   int nl = 0;

   while (i < size && data[i] != '"' && nl < 2) {
      if (data[i] == '\n') {
         nl++;
      }
      i++;
   }

   if (i < size && data[i] == '"' && nl < 2) {
      out += data.mid(0, i + 1);
      return i + 1;
   }

   // not a quoted section
   return 0;
}

/** Process an HTML tag. Note that <pre>..</pre> are treated specially, in
 *  the sense that all code inside is written unprocessed
 */
static int processHtmlTag(QString &out, const QString &data, int offset, int size, const QString &pristineChars1)
{
   if (offset > 0 && pristineChars1 == "\\") {
      return 0;   // escaped <
   }

   // find the end of the html tag
   int i = 1;
   int l = 0;


   QChar charA = 0;   
   if (! i > data.length()) {
      charA = data[i];
   }

   // compute length of the tag name
   while (i < size && isIdChar(charA)) {
      i++;   
      l++;
   }

   QString tagName = data.mid(1, i - 1);   

   if (tagName.toLower() == "pre") { 
      bool insideStr = false;

      while (i < size - 6) {
         QChar c = data[i];

         if (! insideStr && c == '<') {
            // potential start of html tag

            if (data[i + 1] == '/' && data[i + 2].toLower() == 'p' && data[i + 3].toLower() == 'r' &&
                  data[i + 4].toLower() == 'e' && data[i + 5].toLower() == '>') {

               // found </pre> tag, copy from start to end of tag
               out += data.mid(0, i + 6);
              
               return i + 6;
            }

         } else if (insideStr && c == '"') {
            if (data[i - 1] != '\\') {
               insideStr = false;
            }

         } else if (c == '"') {
            insideStr = true;
         }

         i++;
      }

   } else { 
      // some other html tag

      if (l > 0 && i < size) {
         if (data[i] == '/' && i < size - 1 && data[i + 1] == '>') { 
            // <bla/>
            
            out += data.mid(0, i + 2);
            return i + 2;

         } else if (data[i] == '>') { 
            // <bla>
            
            out += data.mid(0, i + 1);
            return i + 1;

         } else if (data[i] == ' ') { 
            // <bla attr=...

            i++;
            bool insideAttr = false;

            while (i < size) {
               if (!insideAttr && data[i] == '"') {
                  insideAttr = true;

               } else if (data[i] == '"' && data[i - 1] != '\\') {
                  insideAttr = false;

               } else if (!insideAttr && data[i] == '>') { 
                  // found end of tag
                 
                  out += data.mid(0, i + 1);
                  return i + 1;
               }
               i++;
            }
         }
      }
   }

   // not a valid html tag
   return 0;
}

static int processEmphasis(QString &out, const QString &data, int offset, int size, const QString &pristineChars1)
{
   int len = data.length();
      
   QChar charA = 0;   
   if (! pristineChars1.isEmpty()) {
      charA = pristineChars1[0];
   }

   QChar data0 = 0;   
   if (len > 0) {
      data0 = data[0];
   }

   QChar data1 = 0;   
   if (len > 1) {
      data1 = data[1];
   }

   QChar data2 = 0;   
   if (len > 2) {
      data2 = data[2];
   }
    
   QChar data3 = 0;   
   if (len > 3) {
      data3 = data[3];
   }

   // invalid char before * or _, invalid char after * or _, invalid char after ** or __
   if ( (offset > 0 && ! isOpenEmphChar(charA)) || 
        (size > 1 && data0 != data1 && ! isIdChar(data1)) || (size > 2 && data0 == data1 && ! isIdChar(data2))) {  

      return 0;
   }
  
   int ret;
   
   if (size > 2 && data1 != data0) { 
      // _bla or *bla
      // whitespace can not follow an opening emphasis

      if (data1 == ' ' || data1 == '\n' || (ret = processEmphasis1(out, data.mid(1), size - 1, data0)) == 0) {
         return 0;
      }

      return ret + 1;
   }

   if (size > 3 && data1 == data0 && data2 != data0) { 
      // __bla or **bla

      if (data2 == ' ' || data2 == '\n' || (ret = processEmphasis2(out, data.mid(2), size - 2, data0)) == 0) {
         return 0;
      }

      return ret + 2;
   }

   if (size > 4 && data1 == data0 && data2 == data0 && data3 != data0) { 
      // ___bla or ***bla
      // passing pristineChars2 as 5th

      if (data3 == ' ' || data3 == '\n' || (ret = processEmphasis3(out, data.mid(3), size - 3, 
                  data0, data.mid(1, 2) )) == 0) {

         return 0;
      }

      return ret + 3;
   }

   return 0;
}

static int processLink(QString &out, const QString &data, int, int size)
{
   QString content;
   QString link;
   QString title;

   int contentStart, contentEnd, linkStart, titleStart, titleEnd;
   bool isImageLink = false;
   bool isToc = false;
   int i = 1;

   if (data[0] == '!') {
      isImageLink = true;
      if (size < 2 || data[1] != '[') {
         return 0;
      }
      i++;
   }

   contentStart = i;
   int level = 1;
   int nl = 0;

   // find the matching ]
   while (i < size) {
      if (data[i - 1] == '\\') { 
         // skip escaped characters

      } else if (data[i] == '[') {
         level++;

      } else if (data[i] == ']') {
         level--;
         if (level <= 0) {
            break;
         }

      } else if (data[i] == '\n') {
         nl++;
         if (nl > 1) {
            return 0;   // only allow one newline in the content
         }
      }
      i++;
   }

   if (i >= size) {
      return 0;   // premature end of comment -> no link
   }

   contentEnd = i;   
   content = data.mid(contentStart, contentEnd - contentStart);   
  
   if (! isImageLink && content.isEmpty()) {
      return 0;   // no link text
   }
   i++; // skip over ]

   // skip whitespace
   while (i < size && data[i] == ' ') {
      i++;
   }

   if (i < size && data[i] == '\n') { 
      // one newline allowed here
      i++;

      // skip more whitespace
      while (i < size && data[i] == ' ') {
         i++;
      }
   }

   bool explicitTitle = false;
   if (i < size && data[i] == '(') { 
      // inline link
      i++;

      while (i < size && data[i] == ' ') {
         i++;
      }

      if (i < size && data[i] == '<') {
         i++;
      }

      linkStart = i;
      nl = 0;
      int braceCount = 1;

      while (i < size && data[i] != '\'' && data[i] != '"' && braceCount > 0) {
         if (data[i] == '\n') {
            // unexpected EOL
            nl++;

            if (nl > 1) {
               return 0;
            }

         } else if (data[i] == '(') {
            braceCount++;

         } else if (data[i] == ')') {
            braceCount--;

         }

         if (braceCount > 0) {
           i++;
         }        
      }

      if (i >= size || data[i] == '\n') {
         return 0;
      }

      link = data.mid(linkStart, i - linkStart);      
      link = link.trimmed();
      
      if (link.isEmpty()) {
         return 0;
      }

      if (link.at(link.length() - 1) == '>') {
         link = link.left(link.length() - 1);
      }

      // optional title
      if (data[i] == '\'' || data[i] == '"') {
         QChar c = data[i];

         i++;
         titleStart = i;
         nl = 0;

         while (i < size && data[i] != ')') {
            if (data[i] == '\n') {
               if (nl > 1) {
                  return 0;
               }
               nl++;
            }
            i++;
         }

            if (i >= size) {
            return 0;
         }

         titleEnd = i - 1;
         // search back for closing marker
         while (titleEnd > titleStart && data[titleEnd] == ' ') {
            titleEnd--;
         }

         if (data[titleEnd] == c) { 
            // found it
            title = data.mid(titleStart, titleEnd - titleStart);   

         } else {
            return 0;
         }
      }
      i++;

   } else if (i < size && data[i] == '[') { 
      // reference link
      i++;
      linkStart = i;
      nl = 0;

      // find matching ]
      while (i < size && data[i] != ']') {
         if (data[i] == '\n') {
            nl++;
            if (nl > 1) {
               return 0;
            }
         }
         i++;
      }

      if (i >= size) {
         return 0;
      }

      // extract link    
      link = data.mid(linkStart, i - linkStart);        
      link = link.trimmed();

      if (link.isEmpty()) { 
         // shortcut link
         link = content;
      }

      // lookup reference
      auto lr = g_linkRefs.find(link.toLower());

      if (lr != g_linkRefs.end()) { 
         // found it
         link  = lr->link;
         title = lr->title;      

      } else { 
         // reference not found        
         return 0;
      }
      i++;

   } else if (i < size && data[i] != ':' && !content.isEmpty()) { 
      // minimal link ref notation [some id]

      auto lr = g_linkRefs.find(content.toLower());

      if (lr != g_linkRefs.end()) { 
         // found it
         link  = lr->link;
         title = lr->title;
         explicitTitle = true;
         i = contentEnd;

      } else if (content == "TOC") {
         isToc = true;
         i = contentEnd;

      } else {
         return 0;
      }
      i++;
   } else {
      return 0;
   }
  
   if (isToc) { 
      // special case for [TOC]
      if (g_current) {
         g_current->stat = true;
      }

   } else if (isImageLink) {
      bool ambig;
      QSharedPointer<FileDef> fd;

      if (link.indexOf("@ref ") != -1 || link.indexOf("\\ref ") != -1 || (fd = findFileDef(Doxy_Globals::imageNameDict, link, ambig)))
         // assume DoxyPress symbol link or local image link

      {
         out += "@image html ";
         out += link.mid(fd ? 0 : 5);

         if (!explicitTitle && !content.isEmpty()) {
            out += " \"";
            out += content;
            out += "\"";

         } else if ((content.isEmpty() || explicitTitle) && !title.isEmpty()) {
            out += " \"";
            out += title;
            out += "\"";
         }

      } else {
         out += "<img src=\"";
         out += link;
         out += "\" alt=\"";
         out += content;
         out += "\"";

         if (! title.isEmpty()) {
            out += " title=\"";
            out += substitute(title.simplified(), "\"", "&quot;");
            out += "\"";
         }

        out += "/>";
      }

   } else {
      SrcLangExt lang = getLanguageFromFileName(link);
      int lp = -1;

      if ((lp = link.indexOf("@ref ")) != -1 || (lp = link.indexOf("\\ref ")) != -1 || lang == SrcLangExt_Markdown) {
         // assume DoxyPress symbol link
      
         if (lp == -1) { // link to markdown page
            out += "@ref ";
         }

         out += link;
         out += " \"";

         if (explicitTitle && !title.isEmpty()) {
            out += title;

         } else {
            out += content;
         }

         out += "\"";

      } else if (link.indexOf('/') != -1 || link.indexOf('.') != -1 || link.indexOf('#') != -1) {
         // file/url link
         out += "<a href=\"";
         out += link;
         out += "\"";

         if (! title.isEmpty()) {
            out += " title=\"";
            out += substitute(title.simplified(), "\"", "&quot;");
            out += "\"";
         }

         out += ">";
         out += content.simplified();
         out += "</a>";

      } else { 
         // avoid link to e.g. F[x](y)        
         return 0;
      }
   }
   return i;
}

/** '`' parsing a code span (assuming codespan != 0) */
static int processCodeSpan(QString &out, const QString &data, int /*offset*/, int size)
{
   int end, nb = 0, i, f_begin, f_end;

   /* counting the number of backticks in the delimiter */
   while (nb < size && data[nb] == '`') {
      nb++;
   }

   /* finding the next delimiter */
   i = 0;
   int nl = 0;
   for (end = nb; end < size && i < nb && nl < 2; end++) {
      if (data[end] == '`') {
         i++;
      } else if (data[end] == '\n') {
         i = 0;
         nl++;
      } else {
         i = 0;
      }
   }

   if (i < nb && end >= size) {
      return 0;  // no matching delimiter
   }

   if (nl == 2) { 
      // too many newlines inside the span
      return 0;
   }

   // trimming outside whitespaces
   f_begin = nb;
   while (f_begin < end && data[f_begin] == ' ') {
      f_begin++;
   }

   f_end = end - nb;
   while (f_end > nb && data[f_end - 1] == ' ') {
      f_end--;
   }

   if (nb == 1) { // check for closing ' followed by space within f_begin..f_end
      i = f_begin;

      while (i < f_end - 1) {

         QChar charA = 0;   
         if (! i + 1 > data.length()) {
            charA = data[i + 1];
         }

         if (data[i] == '\'' && ! isIdChar(charA)) { 
            // reject `some word' and not `it's cool`
            return 0;
         }

         i++;
      }
   }

   /* real code span */
   if (f_begin < f_end) {

      QString codeFragment = data.mid(f_begin, f_end - f_begin);      
      out += "<tt>";
      out += escapeSpecialChars(codeFragment);
      out += "</tt>";
   }

   return end;
}

static int processSpecialCommand(QString &out, const QString &data, int offset, int size, const QString &pristineChars1)
{
   int i = 1;

   // passing pristineChars1 as 4th
   QString endBlockName = isBlockCommand(data, offset, size, pristineChars1);

   if (! endBlockName.isEmpty()) {
      int l = endBlockName.length();

      while (i < (size - l)) {
         if ((data[i] == '\\' || data[i] == '@') && data[i - 1] != '\\' && data[i - 1] != '@') { 
            // not escaped
            
            if (data.mid(i+1).startsWith(endBlockName)) {
               out += data.mid(0, i + 1 + l);
               return i + 1 + l;
            }
         }

         i++;
      }
   }

   if (size > 1 && data[0] == '\\') {
      QChar c = data[1];

      if (c == '[' || c == ']' || c == '*' ||
            c == '!' || c == '(' || c == ')' || c == '`' || c == '_') {

         if (c == '-' && size > 3 && data[2] == '-' && data[3] == '-') { 
            // \---
            out += data.mid(1, 3);
            return 4;

         } else if (c == '-' && size > 2 && data[2] == '-') { 
            // \--
            out += data.mid(1, 2);
            return 3;

         } else if (c =='-') {
            // \- 
            out += c;

         }

         out += c;
         return 2;
      }
   }

   return 0;
}

static void processInline(QString &out, const QString &processText, int size)
{
   int i     = 0;
   int index = 0;

   QSet<QChar> keys;
   keys.insert('_');   // processEmphasis
   keys.insert('*');   // processEmphasis
   keys.insert('`');   // processCodeSpan
   keys.insert('\\');  // processSpecialCommand
   keys.insert('@');   // processSpecialCommand
   keys.insert('[');   // processLink
   keys.insert('!');   // processLink
   keys.insert('<');   // processHtmlTag
   keys.insert('-');   // processNmdash
   keys.insert('"');   // processQuoted
 
   while (i < size) {
    
      while (index < size)  {  
         if (keys.contains(processText[index])) { 
            break;         
         }        

         index++;
      }
   
      out += processText.mid(i, index - i);

      if (index >= size) {
         break;
      }

      int skipCount;

      switch (processText[index].unicode()) {

         case '_':
            // last one is passing pristineChars1
            skipCount = processEmphasis(out, processText.mid(index), index, size - index, processText.mid(index - 1, 1));
            break;

         case '*':
            // last one is passing pristineChars1
            skipCount = processEmphasis(out, processText.mid(index), index, size - index, processText.mid(index - 1, 1));
            break;

         case '`':            
            skipCount = processCodeSpan(out, processText.mid(index), index, size - index);
            break;

         case '\\':
            // last one is passing pristineChars1
            skipCount = processSpecialCommand(out, processText.mid(index), index, size - index, processText.mid(index - 1, 1));
            break;

         case '@':
            // last one is passing pristineChars1
            skipCount = processSpecialCommand(out, processText.mid(index), index, size - index, processText.mid(index - 1, 1));
            break;

         case '[':
            skipCount = processLink(out, processText.mid(index), index, size - index);
            break;

         case '!':
            skipCount = processLink(out,processText.mid(index), index, size - index);
            break;

         case '<':
            // last one is passing pristineChars1
            skipCount = processHtmlTag(out, processText.mid(index), index, size - index, processText.mid(index - 1, 1));
            break;

         case '-':
            // last one is  passing pristineChars8
            skipCount = processNmdash(out, processText.mid(index), index, size - index, processText.mid(index - 8, 8));
            break;

         case '"':
            skipCount = processQuoted(out, processText.mid(index), index, size - index);
            break;

      }
    
      if (skipCount == 0) {
         i = index;
         index++;
         

      } else {     
         i = index + skipCount;   
         index = i;
      }
   }
}

/** returns whether the line is a text-style hdr underline */
static int isHeaderline(const QString &data, int size)
{
   int i = 0;
   int c = 0;

   while (i < size && data[i] == ' ') {
      i++;
   }

   // test of level 1 header
   if (i < size && data[i] == '=') {
      while (i < size && data[i] == '=') {
         i++, c++;
      }

      while (i < size && data[i] == ' ') {
         i++;
      }

      return (c > 1 && (i >= size || data[i] == '\n')) ? 1 : 0;
   }

   // test of level 2 header
   if (i < size && data[i] == '-') {
      while (i < size && data[i] == '-') {
         i++, c++;
      }

      while (i < size && data[i] == ' ') {
         i++;
      }

      return (c > 1 && (i >= size || data[i] == '\n')) ? 2 : 0;
   }

   return 0;
}

/** returns true if this line starts a block quote */
static bool isBlockQuote(const QString &data, int size, int indent)
{
   int i = 0;
   while (i < size && data[i] == ' ') {
      i++;
   }
   if (i < indent + codeBlockIndent) { // could be a quotation
      // count >'s and skip spaces
      int level = 0;

      while (i < size && (data[i] == '>' || data[i] == ' ')) {
         if (data[i] == '>') {
            level++;
         }
         i++;
      }

      // last characters should be a space or newline,
      // so a line starting with >= does not match
      return level > 0 && i < size && ((data[i - 1] == ' ') || data[i] == '\n');

   } else { 
      // too much indentation -> code block
      return false;
   }
}

/** returns end of the link ref if this is indeed a link reference. */
static int isLinkRef(const QString &data, int size, QString &refid, QString &link, QString &title)
{
   // format: start with [some text]:

   int i = 0; 
   int refIdStart;
   int len = data.length();

   while (i < size && i < len && data[i] == ' ') {
      i++;
   }

   if (i >= size || i >= len || data[i] != '[') {
      return 0;
   }
   i++;   

   refIdStart = i;


   while (i < size && i < len && data[i] != '\n' && data[i] != ']') {
      i++;
   }

   if (i >= size || i >= len ||data[i] != ']') {
      return 0;
   }
  
   refid = data.mid(refIdStart, i - refIdStart); 

   if (refid.isEmpty()) {
      return 0;
   }
   i++;

   if (i >= size || i >= len || data[i] != ':') {
      return 0;
   }
   i++;

   // format: whitespace* \n? whitespace* (<url> | url)
   while (i < size && data[i] == ' ') {
      i++;
   }

   if (i < size && data[i] == '\n') {
      i++;

      while (i < size && data[i] == ' ') {
         i++;
      }
   }

   if (i >= size) {
      return 0;
   }

   if (i < size && data[i] == '<') {
      i++;
   }

   int linkStart = i;
   while (i < size && data[i] != ' ' && data[i] != '\n') {
      i++;
   }

   int linkEnd = i;
   if (i < size && data[i] == '>') {
      i++;
   }

   if (linkStart == linkEnd) {
      return 0;   // empty link
   }

   link = data.mid(linkStart, linkEnd - linkStart); 

   if (link == "@ref" || link == "\\ref") {
      int argStart = i;
      while (i < size && data[i] != '\n' && data[i] != '"') {
         i++;
      }

      QString refArg = data.mid(argStart, i - argStart); 
      link += refArg;
   }

   title.resize(0);

   // format: (whitespace* \n? whitespace* ( 'title' | "title" | (title) ))?
   int eol = 0;
   while (i < size && data[i] == ' ') {
      i++;
   }
   if (i < size && data[i] == '\n') {
      eol = i;
      i++;
      while (i < size && data[i] == ' ') {
         i++;
      }
   }

   if (i >= size) {     
      return i; // end of buffer while looking for the optional title
   }

   QChar c = data[i];

   if (c == '\'' || c == '"' || c == '(') { 
      // optional title present?
      
      i++;
      if (c == '(') {
         c = ')';   // replace c by end character
      }

      int titleStart = i;

      // search for end of the line
      while (i < size && data[i] != '\n') {
         i++;
      }
      eol = i;

      // search back to matching character
      int end = i - 1;
      while (end > titleStart && data[end] != c) {
         end--;
      }

      if (end > titleStart) {
         title = data.mid(titleStart, end - titleStart); 
      }

   }

   while (i < size && data[i] == ' ') {
      i++;
   }

   if (i >= size) {
      return i;   // end of buffer while ref id was found
   } else if (eol) {
      return eol;   // end of line while ref id was found
   }

   return 0;                            // invalid link ref
}

static int isHRuler(const QString &data, int size)
{
   int i = 0;

   if (size > 0 && data[size - 1] == '\n') {
      size--;   // ignore newline character
   }
   while (i < size && data[i] == ' ') {
      i++;
   }
   if (i >= size) {
      return 0;   // empty line
   }

   QChar c = data[i];

   if (c != '*' && c != '-' && c != '_') {
      return 0; // not a hrule character
   }

   int n = 0;

   while (i < size) {
      if (data[i] == c) {
         n++; // count rule character
      } else if (data[i] != ' ') {
         return 0; // line contains non hruler characters
      }
      i++;
   }
   return n >= 3; // at least 3 characters needed for a hruler
}

static QString extractTitleId(QString &title)
{   
   static QRegExp r2("\\{#[a-z_A-Z][a-z_A-Z0-9\\-]*\\}");

   int l = 0;
   int i = r2.indexIn(title);
   l     = r2.matchedLength();

   if (i != -1 && title.mid(i + l).trimmed().isEmpty()) { 
      // found {#id} style id
      QString id = title.mid(i + 2, l - 3);
      title = title.left(i);

      return id;
   }
  
   return "";
}


static int isAtxHeader(const QString &data, int size, QString &header, QString &id)
{
   int i = 0;
   int end;
   int level = 0;
   int blanks = 0;

   // find start of header text and determine heading level
   while (i < size && data[i] == ' ') {
      i++;
   }
   if (i >= size || data[i] != '#') {
      return 0;
   }
   while (i < size && level < 6 && data[i] == '#') {
      i++, level++;
   }
   while (i < size && data[i] == ' ') {
      i++, blanks++;
   }
   if (level == 1 && blanks == 0) {
      return 0; // special case to prevent #someid seen as a header (see bug 671395)
   }

   // find end of header text
   end = i;
   while (end < size && data[end] != '\n') {
      end++;
   }
   while (end > i && (data[end - 1] == '#' || data[end - 1] == ' ')) {
      end--;
   }

   // store result
   header = data.mid(i, end - i); 
   id = extractTitleId(header);

   if (! id.isEmpty()) { 
      // strip #'s between title and id

      i = header.length() - 1;

      while (i >= 0 && (header.at(i) == '#' || header.at(i) == ' ')) {
         i--;
      }

      header = header.left(i + 1);
   }

   return level;
}

#define isLiTag(i) \
   (data[(i)]=='<' && \
   (data[(i)+1]=='l' || data[(i)+1]=='L') && \
   (data[(i)+2]=='i' || data[(i)+2]=='I') && \
   (data[(i)+3]=='>'))

// compute the indent from the start of the input, excluding list markers
// such as -, -#, *, +, 1., and <li>
static int computeIndentExcludingListMarkers(const QString &data)
{
   int i        = 0;
   int indent   = 0;
   bool isDigit = false;
   bool isLi    = false;

   bool listMarkerSkipped = false;
   int size = data.length();

   if (data.trimmed().isEmpty()) {
      return -1;
   }

   while (i < size && (data[i] == ' ' || 
           (! listMarkerSkipped &&                                  // first list marker
            (data[i] == '+' || data[i] == '-' || data[i] == '*' ||  // unordered list char
             (data[i] == '#' && i > 0 && data[i - 1] == '-')    ||  // -# item
             (isDigit = (data[i] >= '1' && data[i] <= '9'))     ||  // ordered list marker?
             (isLi = (i < size - 3 && isLiTag(i)))                  // <li> tag
            ) ) ) ) {

      if (isDigit) { 
         // skip over ordered list marker '10. '
         int j = i + 1;

         while (j < size && ((data[j] >= '0' && data[j] <= '9') || data[j] == '.')) {

            if (data[j] == '.') { 
               // should be end of the list marker

               if (j < size - 1 && data[j + 1] == ' ') { 
                  // valid list marker
                  listMarkerSkipped = true;
                  indent += j + 1 - i;
                  i = j + 1;
                  break;

               } else { 
                  // not a list marker
                  break;
               }
            }
            j++;
         }

      } else if (isLi) {
         // skip over <li>

         i += 3;
         indent += 3;
         listMarkerSkipped = true;

      } else if (data[i] == '-' && i < size - 2 && data[i + 1] == '#' && data[i + 2] == ' ') {
         // case "-# "

         listMarkerSkipped = true; // only a single list marker is accepted
         i++; // skip over #
         indent++;

      } else if (data[i] != ' ' && i < size - 1 && data[i + 1] == ' ') {
         // case "- " or "+ " or "* "

         listMarkerSkipped = true; // only a single list marker is accepted
      }

      if (data[i] != ' ' && ! listMarkerSkipped) {
         // end of indent

         break;
      }

      indent++, i++;
   }

   return indent;
}

static bool isFencedCodeBlock(const QString &data, int size, int refIndent, QString &lang, int &start, int &end, int &offset)
{
   // rules: at least 3 ~~~, end of the block same amount of ~~~'s, otherwise return false

   int i = 0;
   int indent = 0;
   int startTildes = 0;

   while (i < size && data[i] == ' ') {
      indent++, i++;
   }

   if (indent >= refIndent + 4) {
      return false;   // part of code block
   }

   char tildaChar = '~';

   if (i < size && data[i] == '`') {
      tildaChar = '`';
   }

   while (i < size && data[i] == tildaChar) {
      startTildes++;
      i++;
   }

   if (startTildes < 3) {
      return false;   // not enough tildes
   }

   if (i < size && data[i] == '{') {
      i++;   // skip over optional {
   }

   int startLang = i;
   while (i < size && (data[i] != '\n' && data[i] != '}' && data[i] != ' ')) {
      i++;
   }

   lang = data.mid(startLang, i - startLang); 
   while (i < size && data[i] != '\n') {
      i++;   // proceed to the end of the line
   }

   start = i;

   while (i < size) {
      if (data[i] == tildaChar) {
         end = i - 1;
         int endTildes = 0;

         while (i < size && data[i] == tildaChar) {
            endTildes++;
            i++;
         }
         while (i < size && data[i] == ' ') {
            i++;
         }
         if (i == size || data[i] == '\n') {
            offset = i;
            return endTildes == startTildes;
         }
      }
      i++;
   }
   return false;
}

static bool isCodeBlock(const QString &data, int &indent, int priorLine1_Indent, int priorLine2_Indent)
{   
   // determine the indent for the current line
   int count = 0;
   int size  = data.length();
  
   while (count < size && data[count] == ' ') {
      count++;      
   }

   if (count < codeBlockIndent) {  
      // line is not indented enough    
      return false;
   }

   if (count >= size || data[count] == '\n') {
      // current line is empty
      return false;
   }   
  
   if (priorLine1_Indent == -1 && priorLine2_Indent != -1)   {
      // there are at least 2 preceding lines, priorLine1 is empty
          
      indent = priorLine2_Indent;

      // if the difference is > 4 spaces then this is a code block
      return count >= indent + codeBlockIndent;
     
   } else if (priorLine1_Indent == -1 && priorLine2_Indent == -1)  {
      // priorLine1 is empty and priorLine2 is empty

      // if the difference is > 4 spaces then this is the start of a code block
      return count >= indent + codeBlockIndent;

   } else {     
      // priorLine1 is not empty, not a code block
      return false;

   }
}

/** Finds the location of the table's contains in the string \a data.
 *  Only one line will be inspected.
 *  @param[in] data pointer to the string buffer.
 *  @param[in] size the size of the buffer.
 *  @param[out] start offset of the first character of the table content
 *  @param[out] end   offset of the last character of the table content
 *  @param[out] columns number of table columns found
 *  @returns The offset until the next line in the buffer.
 */
int findTableColumns(const QString &data, int size, int &start, int &end, int &columns)
{
   int i = 0, n = 0;
   int eol;
   // find start character of the table line
   while (i < size && data[i] == ' ') {
      i++;
   }
   if (i < size && data[i] == '|' && data[i] != '\n') {
      i++, n++;   // leading | does not count
   }
   start = i;

   // find end character of the table line
   while (i < size && data[i] != '\n') {
      i++;
   }
   eol = i + 1;
   i--;
   while (i > 0 && data[i] == ' ') {
      i--;
   }
   if (i > 0 && data[i - 1] != '\\' && data[i] == '|') {
      i--, n++;   // trailing or escaped | does not count
   }
   end = i;

   // count columns between start and end
   columns = 0;
   if (end > start) {
      i = start;
      while (i <= end) { // look for more column markers
         if (data[i] == '|' && (i == 0 || data[i - 1] != '\\')) {
            columns++;
         }
         if (columns == 1) {
            columns++;   // first | make a non-table into a two column table
         }
         i++;
      }
   }
   if (n == 2 && columns == 0) { // table row has | ... |
      columns++;
   }
   //printf("findTableColumns(start=%d,end=%d,columns=%d) eol=%d\n",
   //    start,end,columns,eol);
   return eol;
}

/** Returns true iff data points to the start of a table block */
static bool isTableBlock(const QString &data, int size)
{
   int cc0, start, end;

   // the first line should have at least two columns separated by '|'
   int i = findTableColumns(data, size, start, end, cc0);
   if (i >= size || cc0 < 1) {
      //printf("isTableBlock: no |'s in the header\n");
      return false;
   }

   int cc1;
   int ret = findTableColumns(data.mid(i), size - i, start, end, cc1);
   int j = i + start;

   // separator line should consist of |, - and : and spaces only
   while (j <= end + i) {
      if (data[j] != ':' && data[j] != '-' && data[j] != '|' && data[j] != ' ') {
         //printf("isTableBlock: invalid character '%c'\n",data[j]);
         return false; // invalid characters in table separator
      }
      j++;
   }
   if (cc1 != cc0) { // number of columns should be same as previous line
      return false;
   }

   i += ret; // goto next line
   int cc2;
   findTableColumns(data.mid(i), size - i, start, end, cc2);

   return cc1 == cc2;
}

static int writeTableBlock(QString &out, const QString &data, int size)
{
   int i = 0, j, k;
   int columns, start, end, cc;

   i = findTableColumns(data, size, start, end, columns);

   out += "<table>";

   // write table header, in range [start..end]
   out += "<tr>";

   int headerStart = start;
   int headerEnd = end;

   // read cell alignments
   int ret = findTableColumns(data.mid(i), size - i, start, end, cc);
   k = 0;

   Alignment *columnAlignment = new Alignment[columns];

   bool leftMarker = false, rightMarker = false;
   bool startFound = false;
   j = start + i;

   while (j <= end + i) {
      if (!startFound) {
         if (data[j] == ':') {
            leftMarker = true;
            startFound = true;
         }
         if (data[j] == '-') {
            startFound = true;
         }
         //printf("  data[%d]=%c startFound=%d\n",j,data[j],startFound);
      }
      if      (data[j] == '-') {
         rightMarker = false;
      } else if (data[j] == ':') {
         rightMarker = true;
      }
      if (j <= end + i && (data[j] == '|' && (j == 0 || data[j - 1] != '\\'))) {
         if (k < columns) {
            columnAlignment[k] = markersToAlignment(leftMarker, rightMarker);
            //printf("column[%d] alignment=%d\n",k,columnAlignment[k]);
            leftMarker = false;
            rightMarker = false;
            startFound = false;
         }
         k++;
      }
      j++;
   }

   if (k < columns) {
      columnAlignment[k] = markersToAlignment(leftMarker, rightMarker);
      
   }

   // proceed to next line
   i += ret;

   int m = headerStart;
   for (k = 0; k < columns; k++) {
      out += "<th";

      switch (columnAlignment[k]) {
         case AlignLeft:
            out += " align=\"left\"";
            break;

         case AlignRight:
            out += " align=\"right\"";
            break;

         case AlignCenter:
            out += " align=\"center\"";
            break;

         case AlignNone:
            break;
      }

      out += ">";

      while (m <= headerEnd && (data[m] != '|' || (m > 0 && data[m - 1] == '\\'))) {
         out += data[m++];
      }

      m++;
   }

   out += "\n</th>\n";

   // write table cells
   while (i < size) {
      int ret = findTableColumns(data.mid(i), size - i, start, end, cc);
      
      if (cc != columns) {
         break;   // end of table
      }

      out += "<tr>";
      j = start + i;

      int columnStart = j;
      k = 0;

      while (j <= end + i) {
         if (j == columnStart) {
            out += "<td";

            switch (columnAlignment[k]) {
               case AlignLeft:
                  out += " align=\"left\"";
                  break;

               case AlignRight:
                  out += " align=\"right\"";
                  break;

               case AlignCenter:
                  out += " align=\"center\"";
                  break;

               case AlignNone:
                  break;
            }
            out += ">";
         }

         if (j <= end + i && (data[j] == '|' && (j == 0 || data[j - 1] != '\\'))) {
            columnStart = j + 1;
            k++;

         } else {
            out += data[j];
         }
         j++;
      }

      out += '\n';

      // proceed to next line
      i += ret;
   }

   out += "</table> ";

   delete[] columnAlignment;

   return i;
}

void writeOneLineHeaderOrRuler(QString &out, const QString &data, int size)
{
   int level;

   QString header;
   QString id;

   if (isHRuler(data, size)) {
      out += "<hr>\n";

   } else if ((level = isAtxHeader(data, size, header, id))) {
      
      QString hTag;

      if (level < 5 && ! id.isEmpty()) {

         SectionInfo::SectionType type = SectionInfo::Anchor;

         switch (level) {
            case 1:
               out += "@section ";
               type = SectionInfo::Section;
               break;

            case 2:
               out += "@subsection ";
               type = SectionInfo::Subsection;
               break;

            case 3:
               out += "@subsubsection ";
               type = SectionInfo::Subsubsection;
               break;

            default:
               out += "@paragraph ";
               type = SectionInfo::Paragraph;
               break;
         }

         out += id;
         out += " ";
         out += header;
         out += "\n";

         QSharedPointer<SectionInfo> si = Doxy_Globals::sectionDict->find(id);

         if (si) {
            if (si->lineNr != -1) {
               warn(g_fileName, g_lineNr, "Multiple use of section label '%s', (first occurrence: %s, line %d)", 
                   qPrintable(header), csPrintable(si->fileName), si->lineNr);

            } else {
               warn(g_fileName, g_lineNr, "Multiple use of section label '%s', (first occurrence: %s)", 
                    qPrintable(header), csPrintable(si->fileName));
            }

         } else {
            si = QSharedPointer<SectionInfo> (new SectionInfo(g_fileName, g_lineNr, id, header, type, level));

            if (g_current) {
               g_current->anchors->append(*si);
            }

            Doxy_Globals::sectionDict->insert(id, si);
         }

      } else {
         if (! id.isEmpty()) {
            out += "\\anchor " + id + "\n";
         }
         
         hTag = QString("h%1").arg(level);

         out += "<" + hTag + ">";
         out += header;
         out += "</" + hTag + ">\n";      
      }

   } else { 
      // nothing interesting -> just output the line
      out += data.mid(0, size);
   }
}

static int writeBlockQuote(QString &out, const QString &data, int size)
{
   int l;
   int i = 0;
   int curLevel = 0;
   int end = 0;

   while (i < size) {
      // find end of this line
      end = i + 1;
      while (end <= size && data[end - 1] != '\n') {
         end++;
      }

      int j = i;
      int level = 0;
      int indent = i;

      // compute the quoting level
      while (j < end && (data[j] == ' ' || data[j] == '>')) {
         if (data[j] == '>') {
            level++;
            indent = j + 1;

         } else if (j > 0 && data[j - 1] == '>') {
            indent = j + 1;
         }
         j++;
      }
      if (j > 0 && data[j - 1] == '>' &&
            !(j == size || data[j] == '\n')) { // disqualify last > if not followed by space
         indent--;
         j--;
      }
      if (level > curLevel) { // quote level increased => add start markers
         for (l = curLevel; l < level; l++) {
           out += "<blockquote>\n";
         }

      } else if (level < curLevel) { // quote level descreased => add end markers
         for (l = level; l < curLevel; l++) {
            out += "</blockquote>\n";
         }
      }
      curLevel = level;
      if (level == 0) {
         break;   // end of quote block
      }

      // copy line without quotation marks
      out +=  data.mid(indent, end - indent);

      // proceed with next line
      i = end;
   }

   // end of comment within blockquote => add end markers
   for (l = 0; l < curLevel; l++) {
      out += "</blockquote>\n";
   }

   return i;
}

static int writeCodeBlock(QString &out, const QString &data, int size, int refIndent)
{
   int i = 0;
   int end;

   out += "@verbatim\n";
   int emptyLines = 0;

   while (i < size) {
      // find end of this line
      end = i + 1;
      while (end <= size && data[end - 1] != '\n') {
         end++;
      }

      int j = i;
      int indent = 0;

      while (j < end && data[j] == ' ') {
         j++;
         indent++;
      }

      if (j == end - 1) { 
         // found an empty line

         emptyLines++;
         i = end;

      } else if (indent >= refIndent + codeBlockIndent) { 
         // there is enough indent to contine the code block

         while (emptyLines > 0) { 
            // write skipped empty lines, add empty line
            out += "\n";
            emptyLines--;
         }

         // add code line minus the indent
         out += data.mid(i + refIndent + codeBlockIndent, end - i - refIndent - codeBlockIndent);
         i = end;

      } else { 
         // end of code block
         break;
      }
   }

   out += "@endverbatim\n";

   while (emptyLines > 0) { 
      // write skipped empty lines, add empty line
      out += "\n";
      emptyLines--;
   }
  
   return i;
}

// start searching for the end of the line start at offset \a i
// keeping track of possible blocks that need to to skipped.
static void findEndOfLine(QString &out, const QString &data, int size, int &pi, int &i, int &end)
{
   // find end of the line
   int nb = 0;
   end = i + 1;

   while (end <= size && data[end - 1] != '\n') {
      // while looking for the end of the line we might encounter a block
      // that needs to be passed unprocessed.

      if ((data[end - 1] == '\\' || data[end - 1] == '@') && 
            (end <= 1 || (data[end - 2] != '\\' && data[end - 2] != '@')) ) {

         // not escaped
         
         // 4th is passing pristineChars1
         QString endBlockName = isBlockCommand(data.mid(end - 1), end - 1, size - (end - 1), data.mid(end - 2, 1));
         end++;

         if (! endBlockName.isEmpty()) {
            int l = endBlockName.length();

            for (; end < size - l - 1; end++) { 
               // search for end of block marker

               if ((data[end] == '\\' || data[end] == '@') && data[end - 1] != '\\' && data[end - 1] != '@') {

                  if (data.mid(end + 1).startsWith(endBlockName)){

                     if (pi != -1) {    
                        // output previous line if available
                        out += data.mid(pi, i - pi);
                     }

                     // found end marker, skip over this block

                     out += data.mid(i, end + l + 1 - i);
                     pi = -1;
                     i = end + l + 1; // continue after block
                     end = i + 1;
                     break;
                  }
               }
            }
         }

      } else if (nb == 0 && data[end - 1] == '<' && end < size - 6 &&
                 (end <= 1 || (data[end - 2] != '\\' && data[end - 2] != '@')) ) {

         if ( data[end].toLower() == 'p' && data[end + 1].toLower() == 'r' &&
               data[end + 2].toLower() == 'e' && data[end + 3] == '>') { 
         
            // <pre> tag
            if (pi != -1) { 
               // output previous line if available
               out += data.mid(pi, i - pi);
            }

            // output part until <pre>
            out += data.mid(i, end - 1 - i);

            // output part until </pre>, last one is passing pristineChars1
            i = end - 1 + processHtmlTag(out, data.mid(end - 1), end - 1, size - end + 1, data.mid(end - 2));

            pi  = -1;
            end = i + 1;
            break;

         } else {
            end++;
         }

      } else if (nb == 0 && data[end - 1] == '`') {

         while (end <= size && data[end - 1] == '`') {
            end++, nb++;
         }

      } else if (nb > 0 && data[end - 1] == '`') {
         int enb = 0;

         while (end <= size && data[end - 1] == '`') {
            end++;
            enb++;
         }

         if (enb == nb) {
            nb = 0;
         }

      } else {
         end++;
      }
   }   
}

static void writeFencedCodeBlock(QString &out, const QString &data, const QString &lang_t, int blockStart, int blockEnd)
{
   QString lang = lang_t;

   if (! lang.isEmpty() && lang.at(0) == '.') {
      lang = lang.mid(1);
   }

   out += "@code";

   if (! lang.isEmpty()) {
      out += "{" + lang + "}";
   }

   out += data.mid(blockStart, blockEnd - blockStart);
   out += "\n";
   out += "@endcode";
}

static QString processQuotations(const QString &str, int refIndent)
{
   QString out;

   const QChar *data = str.constData();
   const QChar *ptr  = data;
   int size = str.length();

   int i   = 0;
   int end = 0;
   int pi  = -1;
   int blockStart;
   int blockEnd;
   int blockOffset;

   QString lang;

   while (i < size) {

      findEndOfLine(out, str.mid(data - ptr), size, pi, i, end);

      // line is now found at [i..end)

      if (pi != -1) {

         if (isFencedCodeBlock(str.mid(data - ptr + pi), size - pi, refIndent, lang, blockStart, blockEnd, blockOffset)) {

            writeFencedCodeBlock(out, str.mid(data - ptr + pi), lang, blockStart, blockEnd);
            i   = pi + blockOffset;
            pi  = -1;
            end = i + 1;
            continue;

         } else if (isBlockQuote(str.mid(data - ptr + pi), i - pi, refIndent)) {

            i   = pi + writeBlockQuote(out, str.mid(data - ptr + pi), size - pi);
            pi  = -1;
            end = i + 1;
            continue;

         } else {
            out += str.mid(data - ptr + pi, i - pi);
         }
      }

      pi = i;
      i = end;
   }

   if (pi != -1 && pi < size) { // deal with the last line
      if (isBlockQuote(str.mid(data - ptr + pi), size - pi, refIndent)) {
         writeBlockQuote(out, str.mid(data - ptr + pi), size - pi);

      } else {
         out += str.mid(data - ptr + pi, size - pi);
      }
   }
  
   return out;
}

static QString processBlocks(const QString &str, int indent)
{
   QString out;

   const QChar *data = str.constData();   
   int size = str.length();

   int i  = 0;
   int pi = -1;

   int ref;
   int level;

   QString id;
   QString link;
   QString title;

   int blockIndent = indent;

   // get number of spaces in the first line
   int end = 1;
   int sp  = 0;

   while (end <= size && data[end - 1] != '\n') {
      if (data[end - 1] == ' ') {
         sp++;
      }

      end++;
   }
 
   int priorLine2_Indent  = -1;   
   int priorLine1_Indent  = -1;  
   int currentLine_Indent = -1;

   // process each line
   while (i < size) {

      findEndOfLine(out, str, size, pi, i, end);
      // line is now found at [i .. end - 1]

      priorLine2_Indent  = priorLine1_Indent;      
      priorLine1_Indent  = currentLine_Indent;
      currentLine_Indent = computeIndentExcludingListMarkers( str.mid(i, end - i) );
          
      if (pi != -1) {
         int blockStart;
         int blockEnd;
         int blockOffset;

         QString lang;
         blockIndent = indent;
         
         if ((level = isHeaderline(str.mid(i), size - i)) > 0) {            

            while (pi < size && data[pi] == ' ') {
               pi++;
            }

            QString header = str.mid(pi, i - pi - 1);
            QString id = extractTitleId(header);
          
            if (! header.isEmpty()) {

               if (! id.isEmpty()) {
                  out += level == 1 ? "@section " : "@subsection ";
                  out += id;
                  out += " ";
                  out += header;
                  out += "\n\n";

                  QSharedPointer<SectionInfo> si (Doxy_Globals::sectionDict->find(id));

                  if (si) {
                     if (si->lineNr != -1) {
                        warn(g_fileName, g_lineNr, "multiple use of section label '%s', (first occurrence: %s, line %d)", 
                             qPrintable(header), qPrintable(si->fileName), si->lineNr);

                     } else {
                        warn(g_fileName, g_lineNr, "multiple use of section label '%s', (first occurrence: %s)", 
                             qPrintable(header), qPrintable(si->fileName));
                     }

                  } else {
                     si = QSharedPointer<SectionInfo> (new SectionInfo(g_fileName, g_lineNr, id, header, 
                                          level == 1 ? SectionInfo::Section : SectionInfo::Subsection, level));

                     if (g_current) {
                        g_current->anchors->append(*si); 
                     }

                     Doxy_Globals::sectionDict->insert(id, si);
                  }

               } else {
                  out += level == 1 ? "<h1>" : "<h2>";
                  out += header;
                  out += level == 1 ? "\n</h1>\n" : "\n</h2>\n";
               }

            } else {
               out += "<hr>\n";
            }

            pi = -1;
            i  = end;

            end = i + 1;

            continue;

         } else if ((ref = isLinkRef(str.mid(pi), size - pi, id, link, title))) {         

            g_linkRefs.insert(id.toLower(), LinkRef(link, title));

            i   = ref + pi;
            pi  = -1;
            end = i + 1;

         } else if (isFencedCodeBlock(str.mid(pi), size - pi, indent, lang, blockStart, blockEnd, blockOffset)) {
         
            writeFencedCodeBlock(out, str.mid(pi), lang, blockStart, blockEnd);
            i   = pi + blockOffset;
            pi  = -1;
            end = i + 1;
            continue;

         } else  {               
            bool isCB = isCodeBlock(str.mid(i), blockIndent, priorLine1_Indent, priorLine2_Indent);

            if (isCB) { 
               // skip previous line since it is empty

               i   += writeCodeBlock(out, str.mid(i), size - i, blockIndent);
               pi  = -1;
               end = i + 1;
               continue;

            } else if (isTableBlock(str.mid(pi), size - pi)) {

               i   = pi + writeTableBlock(out, str.mid(pi), size - pi);
               pi  = -1;
               end = i + 1;
               continue;

            } else {
               writeOneLineHeaderOrRuler(out, str.mid(pi), i - pi);

            }
         }
      }
      pi = i;
      i = end;
   }
 
   if (pi != -1 && pi < size) { 
      // deal with the last line

      if (isLinkRef(str.mid(pi), size - pi, id, link, title)) {         
         g_linkRefs.insert(id.toLower(), LinkRef(link, title));

      } else {
         writeOneLineHeaderOrRuler(out, str.mid(pi), size - pi);

      }
   }
 
   return out;
}

// returns TRUE if text starts with \@page or \@mainpage command
static bool isExplicitPage(const QString &text)
{   
   if (! text.isEmpty())  {

      int i    = 0;
      int size = text.length();

      while (i < size && (text[i] == ' ' || text[i] == '\n')) {
         i++;
      }      

      if (i < size - 1) {
         // no remaining chars in text

      } else {  
         if (text[i] == '\\' || text[i] == '@') {  

            QString tmp = text.mid(i + 1, 8);

            if (tmp.startsWith("page ") || (tmp == "mainpage") ) {
               return true;
            }  
         }         
      }
   }
   
   return false;
}

static QString extractPageTitle(QString &docs, QString &id)
{
   int ln = 0;

   // first non-empty line
   QString title;

   const QChar *data = docs.constData();
   const QChar *ptr  = data;   

   int size = docs.size();
   int i = 0;
   
   while (i < size && (data[i] == ' ' || data[i] == '\n')) {
      if (data[i] == '\n') {
         ln++;
      }
      i++;
   }

   if (i >= size) {
      return "";
   }

   int end1 = i + 1;

   while (end1 < size && data[end1 - 1] != '\n') {
      end1++;
   }

  
   if (end1 < size) {
      ln++;
      // second line form end1..end2
      int end2 = end1 + 1;
     
      while (end2 < size && data[end2 - 1] != '\n') {
         end2++;
      }

      if ( isHeaderline(docs.mid(data - ptr + end1), size - end1) ) {
         title = docs.mid(data - ptr + i, end1 - i - 1); 

         QString lns;
         lns.fill('\n', ln);

         docs = lns + docs.mid(end2);
         id = extractTitleId(title);
         
         return title;
      }
   }

   if (i < end1 && isAtxHeader(docs.mid(data - ptr + i), end1 - i, title, id) > 0) {
      docs = docs.mid(end1);
   }
   
   return title;
}

static QString detab(const QString &s, int &refIndent)
{
   static int tabSize = Config::getInt("tab-size");

   QString out;

   const QChar *data = s.constData();
   int size = s.length();
  
   int i   = 0;
   int col = 0;

   const int maxIndent = 1000000; // value representing infinity
   int minIndent = maxIndent;

   while (i < size) {
      QChar c = data[i++];

      switch (c.unicode()) {
         case '\t': { 
            // expand tab

            int stop = tabSize - (col % tabSize);
            col += stop;

            while (stop--) {
               out += ' ';
            }
         }
         break;

         case '\n': 
            // reset colomn counter

            out += c;
            col = 0;
            break;

         case ' ': 
            // increment column counter

            out += c;
            col++;
            break;

         default: 
            // non-whitespace => update minIndent
            out += c;        

            if (col < minIndent) {
               minIndent = col;
            }
            col++;
      }
   }

   if (minIndent != maxIndent) {
      refIndent = minIndent;

   } else {
      refIndent = 0;

   }

   return out;
}

QString processMarkdown(const QString &fileName, const int lineNr, QSharedPointer<Entry> e, const QString &input)
{  
   g_linkRefs.clear();

   g_current  = e;
   g_fileName = fileName;
   g_lineNr   = lineNr;
 
   if (input.isEmpty()) {
      return input;
   }
  
   QString out; 
   int refIndent;

   // replace tabs with spaces
   QString s = detab(input, refIndent);

   if (s.trimmed().isEmpty()) {
      return "";
   }

   // process quotation blocks (as these may contain other blocks)
   s = processQuotations(s, refIndent);

   // process block items (headers, rules, code blocks, references)
   s = processBlocks(s, refIndent);

   // process the inline markup (links, emphasis and code spans)
   processInline(out, s, s.length());

   return out;
}

QString markdownFileNameToId(const QString &fileName)
{
   QString baseFn  = stripFromPath(QFileInfo(fileName).absoluteFilePath());

   int i = baseFn.lastIndexOf('.');
   if (i != -1) {
      baseFn = baseFn.left(i);
   }

   QString baseName = substitute(substitute(baseFn, " ", "_"), "/", "_");

   return "md_" + baseName;
}

void MarkdownFileParser::parseInput(const QString &fileName, const QString &fileBuf, QSharedPointer<Entry> root,
                                    enum ParserMode mode, QStringList &includedFiles, bool useClang)
{
   QSharedPointer<Entry> current = QMakeShared<Entry>();

   current->lang = SrcLangExt_Markdown;
   current->fileName = fileName;
   current->docFile  = fileName;
   current->docLine  = 1;

   QString docs = fileBuf;
   QString id;
   QString title   = extractPageTitle(docs, id).trimmed();
   QString titleFn = QFileInfo(fileName).baseName();
   QString fn      = QFileInfo(fileName).fileName();

   static QString mdfileAsMainPage = Config::getString("mdfile-mainpage");

   if (id.isEmpty()) {
      id = markdownFileNameToId(fileName);
   }   

   if (! isExplicitPage(docs))  {


      if (! mdfileAsMainPage.isEmpty() && (fn == mdfileAsMainPage || 
                  QFileInfo(fileName).absoluteFilePath() == QFileInfo(mdfileAsMainPage).absoluteFilePath()) )  {

          // name reference, file reference with path
         docs.prepend("@mainpage " + title + "\n");

      } else if (id == "mainpage" || id == "index") {

         if (title.isEmpty()) {
            title = titleFn;
         }

         docs.prepend("@mainpage " + title + "\n");

      } else {

         if (title.isEmpty()) {
            title = titleFn;
         }

         docs.prepend("@page " + id + " " + title + "\n");
      }
   }

   int lineNr   = 1;
   int position = 0;

   // even without markdown support enabled, we still parse markdown files as such
   bool markdownEnabled = Doxy_Globals::markdownSupport;
   Doxy_Globals::markdownSupport = true;

   bool needsEntry = false;
   Protection prot = Public;

   while (parseCommentBlock(this, current, docs, fileName,lineNr,
             false,     // isBrief
             false,     // javadoc autobrief
             false,     // inBodyDocs
             prot,      // protection
             position,
             needsEntry)) {

      if (needsEntry) {
         QString docFile = current->docFile;
         root->addSubEntry(current, root);

         current = QMakeShared<Entry>();
         current->lang = SrcLangExt_Markdown;
         current->docFile = docFile;
         current->docLine = lineNr;
      }
   }

   if (needsEntry) {
      root->addSubEntry(current, root);
   }

   // restore setting
   Doxy_Globals::markdownSupport = markdownEnabled; 
}

void MarkdownFileParser::parseCode(CodeOutputInterface &codeOutIntf, const QString &scopeName, const QString &input,
                                   SrcLangExt lang, bool isExampleBlock, const QString &exampleName, QSharedPointer<FileDef> fileDef,
                                   int startLine, int endLine, bool inlineFragment, QSharedPointer<MemberDef> memberDef,
                                   bool showLineNumbers, QSharedPointer<Definition> searchCtx, bool collectXRefs )
{
   ParserInterface *pIntf = Doxy_Globals::parserManager->getParser("*.cpp");

   if (pIntf != this) {
      pIntf->parseCode(codeOutIntf, scopeName, input, lang, isExampleBlock, exampleName,
                       fileDef, startLine, endLine, inlineFragment, memberDef, showLineNumbers,
                       searchCtx, collectXRefs);
   }
}

void MarkdownFileParser::resetCodeParserState()
{
   ParserInterface *pIntf = Doxy_Globals::parserManager->getParser("*.cpp");

   if (pIntf != this) {
      pIntf->resetCodeParserState();
   }
}

void MarkdownFileParser::parsePrototype(const QString &text)
{
   ParserInterface *pIntf = Doxy_Globals::parserManager->getParser("*.cpp");

   if (pIntf != this) {
      pIntf->parsePrototype(text);
   }
}

