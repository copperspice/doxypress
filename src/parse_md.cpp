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

#include <QRegularExpression>
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
   LinkRef(const QString &l, const QString &t)
      : link(l), title(t)
   {}

   QString link;
   QString title;
};

enum Alignment { AlignNone, AlignLeft, AlignCenter, AlignRight };

static QHash<QString, LinkRef> g_linkRefs;
static QSharedPointer<Entry>   g_current;
static QString                 g_fileName;
static int                     g_lineNr;

// If a markdown page starts with a level1 header, this header is used as a title of the page.
// This makes it a level0 header. So the level of all other sections will need to be corrected.
// This flag is true if corrections are needed.
// static bool g_correctSectionLevel;

const int codeBlockIndent = 4;

static void processInline(QString &out, QStringView processText, QString::const_iterator iter_size);

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

static bool extraChar(const QChar &text)
{
   bool retval = false;

   if (text == '-' || text == '+' || text == '!' || text == '?' || text == '$' ||
       text == '@' || text == '&' || text == '*' || text == '%') {

      retval = true;
   }

   return retval;
}

// escape characters that have a special meaning later on
static QString escapeSpecialChars(QStringView text)
{
   QString retval;

   if (text.isEmpty()) {
      return retval;
   }

   bool insideQuote = false;
   QChar prevChar;

   for (auto c : text) {

      switch (c.unicode()) {

         case '"':
            if (prevChar != '\\')  {
               insideQuote = !insideQuote;
            }

            retval += c;
            break;

         case '<':
            if (! insideQuote) {
               retval += '\\';
            }

            retval += "<";
            break;

         case '>':
            if (! insideQuote) {
               retval += '\\';
            }

            retval += ">";
            break;

         case '\\':
            if (! insideQuote) {
               retval += '\\';
            }

            retval += "\\";
            break;

         case '@':
            if (! insideQuote) {
               retval += '\\';
            }

            retval += "@";
            break;

         default:
            retval += c;
            break;
      }

      prevChar = c;
   }

   return retval;
}

// convert presence of left and/or right alignment markers to a alignment value
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


// Check if data contains a block command, if so returned the command which ends the block.
// If not an empty string is returned. Note When offset > 0 character position -1 will be inspected.
//
// Checks for and skip the following block commands:
// {@code .. { .. } .. }
// \dot   .. \enddot
// \code  .. \endcode
// \msc   .. \endmsc
// \f$    .. \f$
// \f[    .. \f]
// \f{    .. \f}
// \verbatim  .. \endverbatim
// \latexonly .. \endlatexonly
// \htmlonly  .. \endhtmlonly
// \xmlonly   .. \endxmlonly
// \rtfonly   .. \endrtfonly
// \manonly   .. \endmanonly

static QString isBlockCommand(QStringView data, QString::const_iterator size, QStringView pristineChars1)
{
   QString retval;

   bool openBracket = false;
   bool isEscaped   = false;

   if (! pristineChars1.isEmpty())  {
      QChar char1 = pristineChars1[0];

      openBracket = char1 == '{';
      isEscaped   = char1 == '\\' || char1 == '@';
   }

   if (isEscaped) {
      return retval;
   }

   QString::const_iterator iter_end = data.constBegin() + 1;

   while (iter_end != data.constEnd() && (*iter_end >= 'a' && *iter_end <= 'z')) {
      ++iter_end;
   }

   if (iter_end == data.constBegin() + 1) {
      return retval;
   }

   QString blockName = QStringView(data.constBegin() + 1, iter_end);

   if (blockName == "code" && openBracket) {
      retval = "}";

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

      retval = "end" + blockName;

   } else if (blockName == "startuml") {
      retval = "enduml";

   } else if (blockName == "f" && iter_end != data.constEnd()) {
      QChar c = *iter_end;

      if (c == '$') {
         retval = "f$";

      } else if (c == '[') {
         retval = "f]";

      } else if (c == '{') {
         retval = "f}";

      }
   }

   return retval;
}

/** looks for the next emph char, skipping other constructs, and
 *  stopping when either it is found, or we are at the end of a paragraph
 */
static int findEmphasisChar(QStringView data, QChar c, int c_size)
{
   QString::const_iterator iter_i    = data.constBegin() + 1;
   QString::const_iterator iter_size = data.constEnd();

   while (iter_i < iter_size) {

      while (iter_i < iter_size) {
         QChar ch = *iter_i;

         if (ch == c || ch == '`' || ch == '\\' || ch == '@' || ch == '\n') {
            break;
         }

         ++iter_i;
      }

      // not counting escaped chars or characters that are unlikely
      // to appear as the end of the emphasis char
      QChar charA = '\0';

      if (iter_i != data.constBegin()) {
         charA = iter_i[-1];
      }

      if (ignoreCloseEmphChar(charA)) {
         ++iter_i;
         continue;

      } else {
         // get length of emphasis token
         int tokenLen = 0;

         while (iter_i + tokenLen < iter_size && iter_i[tokenLen] == c) {
            ++tokenLen;
         }

         if (tokenLen > 0) {

            QChar charB = '\0';
            if (iter_i + tokenLen != iter_size) {
               charB = iter_i[tokenLen];
            }

            if (tokenLen != c_size || (iter_i < iter_size - tokenLen && isIdChar(charB))) {
               // to prevent touching some_underscore_identifier
               iter_i = iter_i + tokenLen;
               continue;
            }

            return iter_i - data.constBegin();       // found it
         }
      }

      // skipping a code span
      if (iter_i < data.constEnd() && *iter_i == '`') {
         int snb = 0;

         while (iter_i < iter_size && *iter_i == '`') {
            ++snb;
            ++iter_i;
         }

         // find same pattern to end the span
         int enb = 0;

         while (iter_i < iter_size && enb < snb) {
            if (*iter_i == '`') {
               ++enb;
            }

            if (snb == 1 && *iter_i == '\'') {
               break;   // ` ended by '
            }

            ++iter_i;
         }

      } else if (iter_i < data.constEnd() && (*iter_i == '@' || *iter_i == '\\') ) {
         // skip over blocks that should not be processed, passing pristineChars1 as 4th
         QString endBlockName = isBlockCommand(QStringView(iter_i, data.constEnd()), iter_size, QStringView(iter_i -1, iter_i));

         QChar charC = '\0';

         if (iter_i + 1 <= data.constEnd()) {
            charC = iter_i[1];
         }

         if (! endBlockName.isEmpty()) {
            ++iter_i;

            while (iter_i < iter_size) {
               QChar ch = *iter_i;

               if ((ch == '\\' || ch == '@') && iter_i[-1] != '\\' && iter_i[-1] != '@') {

                  QStringView tmp = QStringView(iter_i +1, data.constEnd());

                  if (tmp.startsWith(endBlockName)) {
                     break;
                  }
               }

               ++iter_i;
            }

         } else if (iter_i < iter_size - 1 && isIdChar(charC)) {
            // @cmd, stop processing, see bug 690385
            return 0;

         } else {
            ++iter_i;

         }

      } else if (iter_i < data.constEnd() && *iter_i == '\n') {
         // end * or _ at paragraph boundary
         ++iter_i;

         while (iter_i < iter_size && *iter_i == ' ') {
            ++iter_i;
         }

         if (iter_i >= iter_size || *iter_i == '\n') {
            return 0;   // empty line -> paragraph
         }

      } else {
         // should not get here
         ++iter_i;
      }
   }

   return 0;
}

// process single emphasis
static int processEmphasis1(QString &out, QStringView data, QChar c)
{
   QString::const_iterator iter_i    = data.constBegin();
   QString::const_iterator iter_size = data.constEnd();

   int len;

   // skipping one symbol if coming from emph3
   if (iter_i + 1 < iter_size && *iter_i == c && iter_i[1] == c) {
      ++iter_i;
   }

   while (iter_i != iter_size) {
      len = findEmphasisChar(QStringView(iter_i, data.constEnd()), c, 1);

      if (len == 0) {
         return 0;
      }

      iter_i += len;

      if (iter_i == iter_size) {
         return 0;
      }

      if (iter_i + 1 < iter_size && iter_i[1] == c) {
         ++iter_i;
         continue;
      }

      if (*iter_i == c && iter_i[-1] != ' ' && iter_i[-1] != '\n') {
         out += "<em>";
         processInline(out, data, iter_i);
         out += "</em>";

         return (iter_i - data.constBegin()) + 1;
      }
   }

   return 0;
}

/** process double emphasis */
static int processEmphasis2(QString &out, QStringView data, QChar c)
{
   QString::const_iterator iter_i    = data.constBegin();
   QString::const_iterator iter_size = data.constEnd();

   int len;

   while (iter_i != iter_size) {
      len = findEmphasisChar(QStringView(iter_i, data.constEnd()), c, 2);

      if (len == 0) {
         return 0;
      }

      iter_i += len;

      if (iter_i != data.constBegin() && iter_i + 1 < iter_size && *iter_i == c) {

         QChar ch_next = iter_i[1];
         QChar ch_prev = iter_i[-1];

         if (ch_next == c && ch_prev != ' ' && ch_prev != '\n') {
            out += "<strong>";
            processInline(out, data, iter_i);
            out += "</strong>";

            return (iter_i - data.constBegin()) + 2;
         }
      }

      ++iter_i;
   }

   return 0;
}

/** Parsing tripple emphasis
 *  Finds the first closing tag, and delegates to the other emph
 */
static int processEmphasis3(QString &out, QStringView data, QChar c, QStringView pristineChars2)
{
   QString::const_iterator iter_i    = data.constBegin();
   QString::const_iterator iter_size = data.constEnd();

   int len;

   while (iter_i != iter_size) {
      len = findEmphasisChar(QStringView(iter_i, data.constEnd()), c, 3);

      if (len == 0) {
         return 0;
      }

      iter_i += len;

      /* skip whitespace preceded symbols */
      if (*iter_i != c || iter_i[-1] == ' ' || iter_i[-1] == '\n') {
         continue;
      }

      if (iter_i + 2 != iter_size && iter_i[1] == c && iter_i[2] == c) {
         out += "<em><strong>";
         processInline(out, data, iter_i);
         out += "</strong></em>";

         return (iter_i - data.constBegin()) + 3;

      } else if (iter_i + 1 != iter_size && iter_i[1] == c) {
         // double symbol found, handing over to emphasis1

          QString tmp = pristineChars2 + data;
          len = processEmphasis1(out, tmp, c);

         if (len == 0) {
            return 0;
         } else {
            return len - 2;
         }

      } else {
         // single symbol found, handing over to emphasis2

         QString tmp = pristineChars2.mid(1) + data;
         len = processEmphasis2(out, tmp, c);

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
static int processNmdash(QString &out, QStringView data, QString::const_iterator iter_size, QStringView pristineChars8)
{
   QString::const_iterator iter_i = data.constBegin() + 1;
   int count = 1;

   if (iter_i != iter_size && *iter_i == '-') { // found --
      ++count;
      ++iter_i;
   }

   if (iter_i != iter_size && *iter_i == '-') { // found ---
      ++count;
      ++iter_i;
   }

   if (iter_i != iter_size && *iter_i == '-') { // found ----
      ++count;
   }

   if (count == 2 && (pristineChars8 != "operator") ) {
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
static int processQuoted(QString &out, QStringView &data, QString::const_iterator iter_size)
{
   QString::const_iterator iter_i = data.constBegin() + 1;
   int nl = 0;

   while (iter_i < iter_size && *iter_i != '"' && nl < 2) {

      if (*iter_i == '\n') {
         nl++;
      }

      ++iter_i;
   }

   if (iter_i != iter_size && *iter_i == '"' && nl < 2) {
      out += QStringView(data.constBegin(), iter_i + 1);
      return (iter_i - data.constBegin()) + 1;
   }

   // not a quoted section
   return 0;
}

/** Process an HTML tag. Note that <pre>..</pre> are treated specially, in
 *  the sense that all code inside is written unprocessed
 */
static int processHtmlTag(QString &out, QStringView data, QString::const_iterator iter_size, QStringView pristineChars1)
{
   if (pristineChars1 == "\\") {
      return 0;   // escaped <
   }

   // find the end of the html tag
   QString::const_iterator iter     = data.constBegin();
   QString::const_iterator iter_end = iter_size;

   if (iter != iter_end) {
      ++iter;
   }

   // compute length of the tag name
   while (iter != iter_end && isIdChar(*iter))  {
      ++iter;
   }

   QStringView tagName = QStringView(data.begin() + 1, iter);

   if (tagName.toLower() == "pre") {
      bool insideStr = false;

      while (iter != iter_end) {
         QChar c = *iter;

         if (! insideStr && c == '<') {
            // potential end of pre html element
            QStringView tmp = QStringView(iter + 1, iter_end);

            if (tmp.startsWith("/pre>", Qt::CaseInsensitive)) {
               // found </pre> element - copy start element, contents, ending element
               out += QStringView(data.begin(), iter + 6);

               return (iter - data.begin()) + 6;
            }

         } else if (insideStr && c == '"') {

            if (iter[-1] != '\\') {
               insideStr = false;
            }

         } else if (c == '"') {
            insideStr = true;
         }

         iter++;
      }

   } else {
      // some other html element
      QChar c = *iter;

      if (iter != iter_end) {
         if (c == '/' && iter + 1 != iter_end && iter[1] == '>') {
            // <bla/>

            out += QStringView(data.begin(), iter + 2);
            return (iter - data.begin()) + 2;

         } else if (c == '>') {
            // <bla>

            out += QStringView(data.begin(), iter + 1);
            return (iter - data.begin()) + 1;

         } else if (c == ' ') {
            // <bla attr=...

            ++iter;
            bool insideAttr = false;

            while (iter != iter_end) {
               c = *iter;

               if (! insideAttr && c == '"') {
                  insideAttr = true;

               } else if (c == '"' && iter[-1] != '\\') {
                  insideAttr = false;

               } else if (! insideAttr && c == '>') {
                  // found end of element

                  out += QStringView(data.begin(), iter + 1);
                  return (iter - data.begin()) + 1;
               }

               ++iter;
            }
         }
      }
   }

   // not a valid html tag
   return 0;
}

static int processEmphasis(QString &out, QStringView data, QString::const_iterator iter_size, QStringView pristineChars1)
{
   int len = data.length();

   QChar charA = '\0';
   if (! pristineChars1.isEmpty()) {
      charA = pristineChars1[0];
   }

   QChar data0 = '\0';
   if (len > 0) {
      data0 = data[0];
   }

   QChar data1 = '\0';
   if (len > 1) {
      data1 = data[1];
   }

   QChar data2 = '\0';
   if (len > 2) {
      data2 = data[2];
   }

   QChar data3 = '\0';
   if (len > 3) {
      data3 = data[3];
   }

   // invalid char before * or _, invalid char after * or _, invalid char after ** or __
   if ( (! charA.isNull() && ! isOpenEmphChar(charA)) ||
        (data.constBegin() + 1 < iter_size && data0 != data1 && ! (isIdChar(data1) || extraChar(data1) || data1 == '[')) ||
        (data.constBegin() + 2 < iter_size && data0 == data1 && ! (isIdChar(data2) || extraChar(data2) || data2 == '[')) ) {

      return 0;
   }

   int ret;

   if (len >= 2 && data.constBegin() + 2 < iter_size && data1 != data0) {
      // _bla or *bla
      // whitespace can not follow an opening emphasis
      QStringView s1 = QStringView(data.constBegin() + 1, data.constEnd());

      if (data1 == ' ' || data1 == '\n' || (ret = processEmphasis1(out, s1, data0)) == 0) {
         return 0;
      }

      return ret + 1;
   }

   if (len >= 3 && data.constBegin() + 3 < iter_size && data1 == data0 && data2 != data0) {
      // __bla or **bla
      QStringView s1 = QStringView(data.constBegin() + 2, data.constEnd());

      if (data2 == ' ' || data2 == '\n' || (ret = processEmphasis2(out, s1, data0)) == 0) {
         return 0;
      }

      return ret + 2;
   }

   if (len >= 4 && data.constBegin() + 4 < iter_size && data1 == data0 && data2 == data0 && data3 != data0) {
      // ___bla or ***bla
      // passing pristineChars2 as 5th

      QStringView s1 = QStringView(data.constBegin() + 3, data.constEnd());
      QStringView s2 = QStringView(data.constBegin() + 1, data.constBegin() + 3);

      if (data3 == ' ' || data3 == '\n' || (ret = processEmphasis3(out, s1, data0, s2)) == 0) {
         return 0;
      }

      return ret + 3;
   }

   return 0;
}

static int processLink(QString &out, QStringView data, QString::const_iterator iter_size)
{
   QString content;
   QString link;
   QString title;

   QString::const_iterator iter_i = data.constBegin();

   QString::const_iterator iter_contentStart;
   QString::const_iterator iter_contentEnd;
   QString::const_iterator iter_linkStart;
   QString::const_iterator iter_titleStart;
   QString::const_iterator iter_titleEnd;

   bool isImageLink = false;
   bool isToc = false;

   int level = 1;
   int nl    = 0;

   if (*iter_i == '!') {
      isImageLink = true;

      ++iter_i;

      if (iter_i == iter_size || *iter_i != '[') {
         return 0;
      }
   }

   ++iter_i;
   iter_contentStart = iter_i;

   // find the matching ]
   while (iter_i != iter_size) {
      QChar c = *iter_i;

      if (iter_i[-1] == '\\') {
         // skip escaped characters

      } else if (c == '[') {
         level++;

      } else if (c == ']') {
         level--;

         if (level <= 0) {
            break;
         }

      } else if (c == '\n') {
         nl++;

         if (nl > 1) {
            return 0;   // only allow one newline in the content
         }
      }

      ++iter_i;
   }

   if (iter_i == iter_size) {
      // premature end of comment -> no link
      return 0;
   }

   iter_contentEnd = iter_i;
   content = QStringView(iter_contentStart, iter_contentEnd);

   if (! isImageLink && content.isEmpty()) {
      return 0;   // no link text
   }

   ++iter_i;    // skip over ]

   // skip whitespace
   while (iter_i != iter_size && *iter_i == ' ') {
      ++iter_i;
   }

   if (iter_i != iter_size && *iter_i == '\n') {
      // one newline allowed here
      ++iter_i;

      // skip more whitespace
      while (iter_i != iter_size && *iter_i == ' ') {
         ++iter_i;
      }
   }

   bool explicitTitle = false;
   if (iter_i != iter_size && *iter_i == '(') {
      // inline link
      ++iter_i;

      while (iter_i != iter_size && *iter_i == ' ') {
         ++iter_i;
      }

      if (iter_i != iter_size && *iter_i == '<') {
         ++iter_i;
      }

      iter_linkStart = iter_i;
      nl = 0;

      int braceCount = 1;

      while (iter_i != iter_size && *iter_i != '\'' && *iter_i != '"' && braceCount > 0) {
         QChar c = *iter_i;

         if (c == '\n') {
            // unexpected EOL
            nl++;

            if (nl > 1) {
               return 0;
            }

         } else if (c == '(') {
            braceCount++;

         } else if (c == ')') {
            braceCount--;

         }

         if (braceCount > 0) {
           ++iter_i;
         }
      }

      if (iter_i == iter_size|| *iter_i == '\n') {
         return 0;
      }

      link = QStringView(iter_linkStart, iter_i).trimmed();

      if (link.isEmpty()) {
         return 0;
      }

      if (link.last() == '>') {
         link.chop(1);
      }

      // optional title
      QChar c = *iter_i;

      if (c == '\'' || c == '"') {
         ++iter_i;

         iter_titleStart = iter_i;
         nl = 0;

         while (iter_i != iter_size && *iter_i != ')') {

            if (*iter_i == '\n') {
               if (nl > 1) {
                  return 0;
               }

               nl++;
            }

            ++iter_i;
         }

         if (iter_i == iter_size) {
            return 0;
         }

         iter_titleEnd = iter_i - 1;

         // search back for closing marker
         while (iter_titleEnd != iter_titleStart && *iter_titleEnd == ' ') {
            iter_titleEnd--;
         }

         if (*iter_titleEnd == c) {
            // found it
            title = QStringView(iter_titleStart, iter_titleEnd);

         } else {
            return 0;
         }
      }

      ++iter_i;

   } else if (iter_i != iter_size && *iter_i == '[') {
      // reference link
      ++iter_i;

      iter_linkStart = iter_i;
      nl = 0;

      // find matching ]
      while (iter_i != iter_size && *iter_i != ']') {

         if (*iter_i == '\n') {
            nl++;

            if (nl > 1) {
               return 0;
            }
         }

         ++iter_i;
      }

      if (iter_i == iter_size) {
         return 0;
      }

      // extract link
      link = QStringView(iter_linkStart, iter_i).trimmed();

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

      ++iter_i;

   } else if (iter_i != iter_size && *iter_i != ':' && ! content.isEmpty()) {
      // minimal link ref notation [some id]

      auto lr = g_linkRefs.find(content.toLower());

      if (lr != g_linkRefs.end()) {
         // found it
         link  = lr->link;
         title = lr->title;

         explicitTitle = true;
         iter_i = iter_contentEnd;

      } else if (content == "TOC") {
         isToc = true;
         iter_i = iter_contentEnd;

      } else {
         return 0;
      }

      ++iter_i;

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

      if (link.contains("@ref ") || link.contains("\\ref ") || (fd = findFileDef(&Doxy_Globals::imageNameDict, link, ambig))) {
         // assume DoxyPress symbol link or local image link

         out += "@image html ";
         out += link.mid(fd ? 0 : 5);

         if (! explicitTitle && ! content.isEmpty()) {
            out += " \"";
            out += content;
            out += "\"";

         } else if ((content.isEmpty() || explicitTitle) && ! title.isEmpty()) {
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

         if (lp == -1) {
            // link to markdown page
            out += "@ref ";
         }

         out += link;
         out += " \"";

         if (explicitTitle && ! title.isEmpty()) {
            out += title;

         } else {
            out += content;
         }

         out += "\"";

      } else if (link.contains('/') || link.contains('.') || link.contains('#')) {
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

         content = content.simplified();
         processInline(out, content, content.constEnd());

         out += "</a>";

      } else {
         // avoid link to e.g. F[x](y)
         return 0;
      }
   }

   return iter_i - data.constBegin();
}

/** '`' parsing a code span (assuming codespan != 0) */
static int processCodeSpan(QString &out, QStringView data, QString::const_iterator iter_size)
{
   QString::const_iterator iter_end;

   QString::const_iterator iter_i  = data.constBegin();
   QString::const_iterator iter_nb = data.constBegin();

   QString::const_iterator iter_f_begin;
   QString::const_iterator iter_f_end;

   int nl = 0;

   /* counting the number of backticks in the delimiter */
   while (iter_nb < iter_size && *iter_nb == '`') {
      ++iter_nb;
   }

   /* finding the next delimiter */
   for (iter_end = iter_nb; iter_end < iter_size && iter_i < iter_nb && nl < 2; ++iter_end) {

      if (*iter_end == '`') {
         ++iter_i;

      } else if (*iter_end == '\n') {
         iter_i  = data.constBegin();
         nl++;

      } else {
         iter_i = data.constBegin();
      }
   }

   if (iter_i < iter_nb && iter_end >= iter_size) {
      return 0;  // no matching delimiter
   }

   if (nl == 2) {
      // too many newlines inside the span
      return 0;
   }

   // trimming outside whitespaces
   iter_f_begin = iter_nb;
   while (iter_f_begin < iter_end && *iter_f_begin == ' ') {
      ++iter_f_begin;
   }

   iter_f_end = iter_end - (iter_nb - data.constBegin());

   while (iter_f_end > iter_nb && iter_f_end[-1] == ' ') {
      --iter_f_end;
   }

   if (iter_nb == data.constBegin() + 1) {
      // check for closing ' followed by space within f_begin..f_end
      iter_i = iter_f_begin;

      while (iter_i < iter_f_end - 1) {

         QChar charA = 0;
         if (iter_i + 1 != data.constEnd()) {
            charA = iter_i[1];
         }

         if (*iter_i == '\'' && ! isIdChar(charA)) {
            // reject `some word' and not `it's cool`
            return 0;
         }

         ++iter_i;
      }
   }

   /* real code span */
   if (iter_f_begin < iter_f_end) {

      QStringView codeFragment = QStringView(iter_f_begin, iter_f_end);
      out += "<tt>";
      out += escapeSpecialChars(codeFragment);
      out += "</tt>";
   }

   return iter_end - data.constBegin();
}

static int processSpecialCommand(QString &out, QStringView data, QString::const_iterator iter_size, QStringView pristineChars1)
{
   QString::const_iterator iter_i = data.constBegin() + 1;

   // passing pristineChars1 as 4th
   QString endBlockName = isBlockCommand(data, iter_size, pristineChars1);

   if (! endBlockName.isEmpty()) {
      int len = endBlockName.length();
      QString::const_iterator iter_tmp = iter_size - len;

      while (iter_i < iter_tmp) {
         QChar ch     = *iter_i;
         QChar prevCh = iter_i[-1];

         if ((ch == '\\' || ch == '@') && prevCh != '\\' && prevCh != '@') {
            // not escaped

            if (QStringView(iter_i + 1, data.constEnd()).startsWith(endBlockName)) {
               out += QStringView(data.constBegin(), iter_i + 1 + len);
               return ( (iter_i - data.constBegin() ) + 1 + len);
            }
         }

         ++iter_i;
      }
   }

   QString::const_iterator iter_tmp = data.constBegin();

   if (iter_tmp != iter_size) {
      QChar c = *iter_tmp;

      if (c != '\\') {
         return 0;
      }

      ++iter_tmp;

      if (iter_tmp == iter_size) {
         return 0;
      }

      c = *iter_tmp;

      if (c == '[' || c == ']' || c == '*' || c == '!' || c == '(' || c == ')' || c == '`' || c == '_') {

         out += c;
         return 2;

      } else if (c == '-') {

         QChar ch_2 = '\0';
         QChar ch_3 = '\0';

         ++iter_tmp;

         if (iter_tmp == iter_size) {
            return 0;
         }

         ch_2 = *iter_tmp;


         ++iter_tmp;

         if (iter_tmp != iter_size) {
            ch_3 = *iter_tmp;
         }

         if (ch_2 == '-' && ch_3 == '-') {
            // \---

            out += "---";
            return 4;

         } else if (ch_2 == '-') {
            // \--

            out += "--";
            return 3;
         }
      }
   }

   return 0;
}

static void processInline(QString &out, const QStringView processText, QString::const_iterator iter_size)
{
   QString::const_iterator iter_i      = processText.constBegin();
   QString::const_iterator iter_index  = processText.constBegin();

   static QSet<QChar> keys;

   if (keys.isEmpty()) {
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
   }

   while (iter_i < iter_size) {

      while (iter_index < iter_size)  {

         if (keys.contains(*iter_index)) {
            break;
         }

         ++iter_index;
      }

      out += QStringView(iter_i, iter_index);

      if (iter_index >= iter_size || iter_index == processText.constEnd()) {
         // done
         break;
      }

      int skipCount;

      QStringView s1 = QStringView(iter_index, processText.constEnd());

      QStringView s2;
      if (iter_index != processText.constBegin()) {
         s2 = QStringView(iter_index - 1, iter_index);
      }

      switch (iter_index->unicode()) {

         case '_':
            // last one is passing pristineChars1
            skipCount = processEmphasis(out, s1, iter_size, s2);
            break;

         case '*':
            // last one is passing pristineChars1
            skipCount = processEmphasis(out, s1, iter_size, s2);
            break;

         case '`':
            skipCount = processCodeSpan(out, s1, iter_size);
            break;

         case '\\':
            // last one is passing pristineChars1
            skipCount = processSpecialCommand(out, s1, iter_size, s2);
            break;

         case '@':
            // last one is passing pristineChars1
            skipCount = processSpecialCommand(out, s1, iter_size, s2);
            break;

         case '[':
            skipCount = processLink(out, s1, iter_size);
            break;

         case '!':
            skipCount = processLink(out, s1, iter_size);
            break;

         case '<':
            // last one is passing pristineChars1
            skipCount = processHtmlTag(out, s1, iter_size, s2);
            break;

         case '-':
            // last one is  passing pristineChars8
            skipCount = processNmdash(out, s1, iter_size, QStringView(iter_index - 8, iter_index));
            break;

         case '"':
            skipCount = processQuoted(out, s1, iter_size);
            break;
      }

      if (skipCount == 0) {
         iter_i  = iter_index;
         ++iter_index;

      } else {
         iter_i = iter_index + skipCount;
         iter_index = iter_i;
      }
   }
}

/** returns whether the line is a text-style hdr underline */
static int isHeaderline(QStringView data)
{
   QString::const_iterator iter = data.constBegin();
   int cnt = 0;

   while (iter != data.constEnd() && *iter == ' ') {
      ++iter;
   }

   // test of level 1 header
   if (iter != data.constEnd() && *iter == '=') {
      while (iter != data.constEnd() && *iter == '=') {
         ++iter;
         ++cnt;
      }

      while (iter != data.constEnd() && *iter == ' ') {
         ++iter;
      }

      return (cnt > 1 && (iter == data.constEnd() || *iter == '\n')) ? 1 : 0;
   }

   // test of level 2 header
   if (iter != data.constEnd() && *iter == '-') {
      while (iter != data.constEnd() && *iter == '-') {
         ++iter;
         ++cnt;
      }

      while (iter != data.constEnd() && *iter == ' ') {
         ++iter;
      }

      return (cnt > 1 && (iter == data.constEnd() || *iter == '\n')) ? 2 : 0;
   }

   return 0;
}

/** returns true if this line starts a block quote */
static bool isBlockQuote(QStringView data, QString::const_iterator iter_size, int indent)
{
   QString::const_iterator iter_i = data.constBegin();
   int count = 0;

   while (iter_i < iter_size && *iter_i == ' ') {
      ++iter_i;
      ++count;
   }

   if (count < indent + codeBlockIndent) {
      // could be a quotation, count >'s and skip spaces
      int level = 0;

      while (iter_i < iter_size && (*iter_i == '>' || *iter_i == ' ')) {
         if (*iter_i == '>') {
            level++;
         }

         ++iter_i;
      }

      // last characters should be a space or newline, so a line starting with >= does not match
      return level > 0 && iter_i < iter_size && (iter_i[-1] == ' ' || *iter_i == '\n');

   } else {
      // too much indentation -> code block
      return false;
   }
}

/** returns end of the link ref if this is indeed a link reference. */
static int isLinkRef(QStringView data, QString &refid, QString &link, QString &title)
{
   // format: start with [some text]:

   QString::const_iterator iter_size = data.constEnd();
   QString::const_iterator iter_i    = data.constBegin();

   QString::const_iterator iter_refIdStart;

   while (iter_i < iter_size && *iter_i == ' ') {
      ++iter_i;
   }

   if (iter_i >= iter_size || *iter_i != '[') {
      return 0;
   }
   ++iter_i;

   iter_refIdStart = iter_i;

   while (iter_i < iter_size && *iter_i != '\n' && *iter_i != ']') {
      ++iter_i;
   }

   if (iter_i >= iter_size || *iter_i != ']') {
      return 0;
   }

   refid = QStringView(iter_refIdStart, iter_i);

   if (refid.isEmpty()) {
      return 0;
   }

   ++iter_i;

   if (iter_i >= iter_size || *iter_i != ':') {
      return 0;
   }

   ++iter_i;

   // format: whitespace* \n? whitespace* (<url> | url)
   while (iter_i < iter_size && *iter_i == ' ') {
      ++iter_i;
   }

   if (iter_i < iter_size && *iter_i == '\n') {
      ++iter_i;

      while (iter_i < iter_size && *iter_i == ' ') {
         ++iter_i;
      }
   }

   if (iter_i >= iter_size) {
      return 0;
   }

   if (iter_i < iter_size && *iter_i == '<') {
      ++iter_i;
   }

   QString::const_iterator iter_linkStart = iter_i;

   while (iter_i < iter_size && *iter_i != ' ' && *iter_i != '\n') {
      ++iter_i;
   }

   QString::const_iterator iter_linkEnd = iter_i;

   if (iter_i < iter_size && *iter_i == '>') {
       ++iter_i;
   }

   if (iter_linkStart == iter_linkEnd) {
      return 0;   // empty link
   }

   link = QStringView(iter_linkStart, iter_linkEnd);

   if (link == "@ref" || link == "\\ref") {
      QString::const_iterator argStart = iter_i;

      while (iter_i < iter_size && *iter_i != '\n' && *iter_i != '"') {
          ++iter_i;
      }

      QString refArg = QStringView(argStart, iter_i);
      link += refArg;
   }

   title.resize(0);

   // format: (whitespace* \n? whitespace* ( 'title' | "title" | (title) ))?
   QString::const_iterator iter_eol = data.constBegin();

   while (iter_i < iter_size && *iter_i == ' ') {
      ++iter_i;
   }

   if (iter_i < iter_size && *iter_i == '\n') {
      iter_eol = iter_i;
       ++iter_i;

      while (iter_i < iter_size && *iter_i == ' ') {
         ++iter_i;
      }
   }

   if (iter_i >= iter_size) {
      // end of buffer while looking for the optional title
      return iter_i - data.constBegin();
   }

   QChar c = *iter_i;

   if (c == '\'' || c == '"' || c == '(') {
      // optional title present?

      ++iter_i;

      if (c == '(') {
         c = ')';                 // replace c by end character
      }

      QString::const_iterator iter_titleStart = iter_i;

      // search for end of the line
      while (iter_i < iter_size && *iter_i != '\n') {
         ++iter_i;;
      }
      iter_eol = iter_i;

      // search back to matching character
      QString::const_iterator iter_end = iter_i - 1;

      while (iter_end > iter_titleStart && *iter_end != c) {
         --iter_end;
      }

      if (iter_end > iter_titleStart) {
         title = QStringView(iter_titleStart, iter_end);
      }
   }

   while (iter_i < iter_size && *iter_i == ' ') {
      ++iter_i;
   }

   if (iter_i >= iter_size) {
      return iter_i - data.constBegin();       // end of buffer while ref id was found

   } else if (iter_eol != data.constBegin()) {
      return iter_eol - data.constBegin();     // end of line while ref id was found
   }

   return 0;          // invalid link ref
}

static int isHRuler(QStringView data, int size)
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
         n++;          // count rule character

      } else if (data[i] != ' ') {
         return 0;     // line contains non hruler characters
      }

      i++;
   }

   return n >= 3;     // at least 3 characters needed for a hruler
}

static QString extractTitleId(QString &title)
{
   static QRegularExpression regExp("\\{#[a-z_A-Z][a-z_A-Z0-9\\-]*\\}");
   QRegularExpressionMatch match = regExp.match(title);

   QString retval;

   if (match.hasMatch()) {
      QStringView tmp1 = QStringView(match.capturedEnd(), title.constEnd()).trimmed();

      if (tmp1.isEmpty()) {
         // found {#id} style id

         retval = QStringView(match.capturedStart() + 2, match.capturedEnd() - 1);
         title  = QStringView(title.constBegin(), match.capturedStart());

         return retval;
      }
   }

   return retval;
}

static int isAtxHeader(QStringView data, int size, QString &header, QString &id)
{
   int i = 0;
   int end;
   int level  = 0;
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

// compute the indent from the start of the input, excluding list markers
// such as -, -#, *, +, 1., and <li>
static int computeIndentExcludingListMarkers(QStringView data)
{
   if (data.trimmed().isEmpty()) {
      return -1;
   }

   QString::const_iterator iter_i    = data.constBegin();
   QString::const_iterator iter_size = data.constEnd();
   int indent = 0;

   bool isDigit  = false;
   bool isList   = false;
   bool skipList = false;

   while (iter_i < iter_size) {
      QChar c = *iter_i;

      QChar nextC_1 = '\0';
      QChar nextC_2 = '\0';
      QChar nextC_3 = '\0';

      if (iter_i  + 1 < iter_size) {
         nextC_1 = iter_i[1];

         if (iter_i  + 2 < iter_size) {
            nextC_2 = iter_i[2];

            if (iter_i  + 3 < iter_size) {
               nextC_3 = iter_i[3];
            }
         }
      }

      if (c != ' ') {

         if (skipList) {
             // first list marker
            break;
         }

         if (c == '+' || c == '-' || c == '*' || (c == '#' && iter_i > data.constBegin() && iter_i[-1] == '-'))   {
            // unordered list char or -# item

         } else {
            // ordered list marker?
            isDigit = (c >= '1' && c <= '9');

            // <li> tag
            if (c =='<' && (nextC_1 == 'l' || nextC_1 == 'L') && (nextC_2 =='i' || nextC_2 == 'I') && (nextC_3 =='>')) {
               isList = true;
            } else {
               isList = false;
            }

            if (! isDigit && ! isList) {
               break;
            }
         }
      }

      if (isDigit) {
         // skip over ordered list marker '10. '
         QString::const_iterator iter_j = iter_i + 1;

         while (iter_j < iter_size) {
            QChar ch = *iter_j;

            if ((ch >= '0' && ch <= '9') || ch == '.') {
               // go on
            } else {
               break;
            }

            if (ch == '.') {
               // should be end of the list marker

               if (iter_j + 1 < iter_size && iter_j[1] == ' ') {
                  // valid list marker

                  indent  += iter_j + 1 - iter_i;
                  iter_i   = iter_j + 1;
                  skipList = true;

                  break;

               } else {
                  // not a list marker
                  break;
               }
            }

            ++iter_j;
         }

      } else if (isList) {
         // skip over <li>

         iter_i  += 3;
         indent  += 3;
         skipList = true;

      } else if (c == '-' && nextC_1 == '#' && nextC_2 == ' ') {
         // case "-# "

         ++iter_i;                 // skip over #
         ++indent;
         skipList = true;          // only a single list marker is accepted

      } else if (c != ' ' && nextC_1 == ' ') {
         // case "- " or "+ " or "* "

         skipList = true;          // only a single list marker is accepted
      }

      if (c != ' ' && ! skipList) {
         // end of indent
         break;
      }

      ++iter_i;
      ++indent;
   }

   return indent;
}

static bool isFencedCodeBlock(QStringView data, int refIndent, QString &lang,
                  QString::const_iterator &iter_start, QString::const_iterator &iter_end, int &offset)
{
   // rules: at least 3 ~~~, end of the block same amount of ~~~'s, otherwise return false

   QString::const_iterator iter_i    = data.constBegin();
   QString::const_iterator iter_size = data.constEnd();
   QString::const_iterator iter_startLang;

   int  indent      = 0;
   int  startTildes = 0;
   char tildaChar   = '~';

   while (iter_i < iter_size && *iter_i == ' ') {
      ++indent;
      ++iter_i;
   }

   if (indent >= refIndent + 4) {
      return false;   // part of code block
   }

   if (iter_i < iter_size && *iter_i == '`') {
      tildaChar = '`';
   }

   while (iter_i < iter_size && *iter_i == tildaChar) {
      ++startTildes;
      ++iter_i;;
   }

   if (startTildes < 3) {
      return false;        // not enough tildes
   }

   if (iter_i < iter_size && *iter_i == '{') {
      ++iter_i;            // skip over optional {
   }

   iter_startLang = iter_i;

   while (iter_i < iter_size && (*iter_i != '\n' && *iter_i != '}' && *iter_i!= ' ')) {
      ++iter_i;
   }

   lang = QStringView(iter_startLang, iter_i);

   while (iter_i < iter_size && *iter_i != '\n') {
      ++iter_i;           // proceed to the end of the line
   }

   iter_start = iter_i;

   while (iter_i < iter_size) {

      if (*iter_i == tildaChar) {

         iter_end = iter_i - 1;
         int endTildes = 0;

         while (iter_i < iter_size && *iter_i == tildaChar) {
            ++endTildes;
            ++iter_i;
         }

         while (iter_i < iter_size && *iter_i == ' ') {
            ++iter_i;;
         }

         if (iter_i == iter_size || *iter_i == '\n') {
            offset = iter_i - data.constBegin();
            return endTildes == startTildes;
         }
      }

      ++iter_i;
   }

   return false;
}

static bool isCodeBlock(QStringView data, int &indent, int priorLine1_indent, int priorLine2_indent)
{
   QString::const_iterator iter_i    = data.constBegin();
   QString::const_iterator iter_size = data.constEnd();

   // determine the indent for the current line
   int count = 0;

   while (iter_i < iter_size && *iter_i == ' ') {
      ++iter_i;
      ++count;
   }

   if (count < codeBlockIndent) {
      // line is not indented enough
      return false;
   }

   if (iter_i >= iter_size || *iter_i == '\n') {
      // current line is empty
      return false;
   }

   if (priorLine1_indent == -1 && priorLine2_indent != -1)   {
      // there are at least 2 preceding lines, priorLine1 is empty

      indent = priorLine2_indent;

      // if the difference is > 4 spaces then this is a code block
      return count >= indent + codeBlockIndent;

   } else if (priorLine1_indent == -1 && priorLine2_indent == -1)  {
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
QString::const_iterator findTableColumns(QStringView data, QString::const_iterator &iter_start,
                  QString::const_iterator &iter_end, int &columns)
{
   QString::const_iterator iter_size = data.constEnd();
   QString::const_iterator iter_i    = data.constBegin();
   QString::const_iterator iter_eol;

   int n = 0;

   // find start character of the table line
   while (iter_i < iter_size && *iter_i == ' ') {
      ++iter_i;
   }

   if (iter_i < iter_size && *iter_i == '|' && *iter_i != '\n') {
      // leading | does not count
      ++iter_i;
      ++n;
   }
   iter_start = iter_i;

   // find end character of the table line
   while (iter_i < iter_size && *iter_i != '\n') {
      ++iter_i;
   }

   iter_eol = iter_i + 1;
   --iter_i;

   while (iter_i > data.constBegin() && *iter_i == ' ') {
      --iter_i;
   }

   if (iter_i > data.constBegin() && iter_i[-1] != '\\' && *iter_i == '|') {
      // trailing or escaped | does not count
      --iter_i;
      ++n;
   }

   iter_end = iter_i;

   // count columns between start and end
   columns = 0;

   if (iter_end > iter_start) {
      iter_i = iter_start;

      while (iter_i <= iter_end) {
         // look for more column markers
         if (*iter_i == '|' && (iter_i == data.constBegin() || iter_i[-1] != '\\')) {
            ++columns;
         }

         if (columns == 1) {
            // first | make a non-table into a two column table
            ++columns;
         }

         ++iter_i;
      }
   }

   if (n == 2 && columns == 0) {
      // table row has | ... |
      ++columns;
   }

   return iter_eol;
}

/** Returns true iff data points to the start of a table block */
static bool isTableBlock(QStringView data)
{
   QString::const_iterator iter_size = data.constEnd();
   QString::const_iterator iter_i;
   QString::const_iterator iter_j;
   QString::const_iterator iter_start;
   QString::const_iterator iter_end;
   QString::const_iterator iter_ret;

   int cc0;
   int cc1;
   int cc2;

   // the first line should have at least two columns separated by '|'
   iter_i = findTableColumns(data, iter_start, iter_end, cc0);

   if (iter_i >= iter_size || cc0 < 1) {
      return false;
   }

   iter_ret = findTableColumns(QStringView(iter_i, data.constEnd()), iter_start, iter_end, cc1);
   iter_j   = iter_start;

   // separator line should consist of |, - and : and spaces only
   while (iter_j <= iter_end) {
      QChar c = *iter_j;

      if (c != ':' && c != '-' && c != '|' && c != ' ') {
         // invalid characters in table separator
         return false;
      }

      ++iter_j;
   }

   if (cc1 != cc0) {
      // number of columns should be same as previous line
      return false;
   }

   findTableColumns(QStringView(iter_ret, data.constEnd()), iter_start, iter_end, cc2);

   return cc1 == cc2;
}

static int writeTableBlock(QString &out, QStringView data)
{
   QString::const_iterator iter_size = data.constEnd();
   QString::const_iterator iter_i    = data.constBegin();
   QString::const_iterator iter_j;
   QString::const_iterator iter_start;
   QString::const_iterator iter_end;
   QString::const_iterator iter_ret;

   int k;
   int columns;
   int cc;

   iter_i = findTableColumns(QStringView(data.constBegin(), data.constEnd()), iter_start, iter_end, columns);
   out += "<table>";

   // write table header, in range [start..end]
   out += "<tr>";

   QString::const_iterator headerStart = iter_start;
   QString::const_iterator headerEnd   = iter_end;

   // read cell alignments
   iter_ret = findTableColumns(QStringView(iter_i, data.constEnd()), iter_start, iter_end, cc);
   k = 0;

   std::vector<Alignment> columnAlignment(columns);

   bool leftMarker  = false;
   bool rightMarker = false;
   bool startFound  = false;

   iter_j = iter_start;

   while (iter_j <= iter_end) {
      QChar c = *iter_j;

      if (! startFound) {

         if (c == ':') {
            leftMarker = true;
            startFound = true;
         }

         if (c == '-') {
            startFound = true;
         }
      }

      if (c == '-') {
         rightMarker = false;

      } else if (c == ':') {
         rightMarker = true;
      }

      if (c == '|' && (iter_j == data.constBegin() || iter_j[-1] != '\\')) {

         if (k < columns) {
            columnAlignment[k] = markersToAlignment(leftMarker, rightMarker);
            leftMarker  = false;
            rightMarker = false;
            startFound  = false;
         }

         k++;
      }

      ++iter_j;
   }

   if (k < columns) {
      columnAlignment[k] = markersToAlignment(leftMarker, rightMarker);
   }

   // proceed to next line
   iter_i = iter_ret;
   QString::const_iterator iter_m  = headerStart;

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

      while (iter_m <= headerEnd && (*iter_m != '|' || (iter_m > data.constBegin() && iter_m[-1] == '\\'))) {
         out += *iter_m;
         ++iter_m;
      }

      ++iter_m;
   }

   out += "\n</th>\n";

   // write table cells
   while (iter_i < iter_size) {
      iter_ret = findTableColumns(QStringView(iter_i, data.constEnd()), iter_start, iter_end, cc);

      if (cc != columns) {
         break;   // end of table
      }

      out += "<tr>";
      iter_j = iter_start;

      QString::const_iterator columnStart = iter_j;
      k = 0;

      while (iter_j <= iter_end) {
         if (iter_j == columnStart) {
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

         if (iter_j <= iter_end && (*iter_j == '|' && (iter_j == data.constBegin() || iter_j[-1] != '\\'))) {
            columnStart = iter_j + 1;
            k++;

         } else {
            out += *iter_j;
         }

         ++iter_j;
      }

      out += '\n';

      // proceed to next line
      iter_i = iter_ret;
   }

   out += "</table> ";

   return iter_i - data.constBegin();
}

void writeOneLineHeaderOrRuler(QString &out, QStringView data, QString::const_iterator iter_size)
{
   int level;

   QString header;
   QString id;

   int size = iter_size - data.constBegin();

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

         QSharedPointer<SectionInfo> si = Doxy_Globals::sectionDict.find(id);

         if (si) {
            if (si->lineNr != -1) {
               warn(g_fileName, g_lineNr, "Multiple use of section label '%s', (first occurrence: %s, line %d)",
                   csPrintable(header), csPrintable(si->fileName), si->lineNr);

            } else {
               warn(g_fileName, g_lineNr, "Multiple use of section label '%s', (first occurrence: %s)",
                    csPrintable(header), csPrintable(si->fileName));
            }

         } else {
            si = QSharedPointer<SectionInfo> (new SectionInfo(g_fileName, g_lineNr, id, header, type, level));

            if (g_current) {
               g_current->m_anchors.append(*si);
            }

            Doxy_Globals::sectionDict.insert(id, si);
         }

      } else {
         if (! id.isEmpty()) {
            out += "\\anchor " + id + "\n";
         }

         hTag = QString("h%1").formatArg(level);

         out += "<" + hTag + ">";
         out += header;
         out += "</" + hTag + ">\n";
      }

   } else {
      // nothing interesting, just output the line
      out += QStringView(data.constBegin(), iter_size);

   }
}

static QString::const_iterator writeBlockQuote(QString &out, QStringView data)
{
   QString::const_iterator iter_i    = data.constBegin();
   QString::const_iterator iter_size = data.constEnd();
   QString::const_iterator iter_end  = data.constBegin();

   QString::const_iterator iter_j;
   QString::const_iterator iter_indent;

   int curLevel = 0;
   int level    = 0;

   while (iter_i < iter_size) {
      // find end of this line
      iter_end = iter_i + 1;

      while (iter_end <= iter_size && iter_end[-1] != '\n') {
         ++iter_end;
      }

      iter_j      = iter_i;
      iter_indent = iter_i;

      level = 0;

      // compute the quoting level
      while (iter_j < iter_end  && (*iter_j == ' ' || *iter_j == '>')) {

         if (*iter_j == '>') {
            level++;
            iter_indent = iter_j + 1;

         } else if (iter_j > data.constBegin() && iter_j[-1] == '>') {
            iter_indent = iter_j + 1;
         }

         ++iter_j;
      }

      if (iter_j > data.constBegin() && iter_j[-1] == '>' && ! (iter_j == iter_size || *iter_j == '\n')) {
         // disqualify last > if not followed by space
         --iter_indent;
         --iter_j;
      }

      if (level > curLevel) {
         // quote level increased => add start markers

         for (int l = curLevel; l < level; l++) {
           out += "<blockquote>\n";
         }

      } else if (level < curLevel) {
         // quote level descreased => add end markers

         for (int l = level; l < curLevel; l++) {
            out += "</blockquote>\n";
         }
      }

      curLevel = level;

      if (level == 0) {
         break;            // end of quote block
      }

      // copy line without quotation marks
      out += QStringView(iter_indent, iter_end);

      // proceed with next line
      iter_i = iter_end;
   }

   // end of comment within blockquote => add end markers
   for (int l = 0; l < curLevel; l++) {
      out += "</blockquote>\n";
   }

   return iter_i;
}

static int writeCodeBlock(QString &out, QStringView data, int refIndent)
{
   QString::const_iterator iter_i    = data.constBegin();
   QString::const_iterator iter_size = data.constEnd();
   QString::const_iterator iter_end;
   QString::const_iterator iter_j;

   int emptyLines = 0;
   int indent     = 0;

   out += "@verbatim\n";

   while (iter_i < iter_size) {
      // find end of this line
      iter_end = iter_i + 1;

      while (iter_end <= iter_size && iter_end[-1] != '\n') {
         ++iter_end;
      }

      if (iter_end > iter_size) {
         iter_end = iter_size;
      }

      iter_j = iter_i;
      indent = 0;

      while (iter_j < iter_end && *iter_j == ' ') {
         ++iter_j;
         ++indent;
      }

      if (iter_j == iter_end - 1) {
         // found an empty line

         ++emptyLines;
         iter_i = iter_end;

      } else if (indent >= refIndent + codeBlockIndent) {
         // there is enough indent to contine the code block

         while (emptyLines > 0) {
            // write skipped empty lines, add empty line
            out += "\n";
            --emptyLines;
         }

         // add code line minus the indent
         out    += QStringView(iter_i + refIndent + codeBlockIndent, iter_end);
         iter_i = iter_end;

      } else {
         // end of code block
         break;
      }
   }

   out += "@endverbatim\n";

   while (emptyLines > 0) {
      // write skipped empty lines, add empty line
      out += "\n";
      --emptyLines;
   }

   return iter_i - data.constBegin();
}

// start searching for the end of the line start at offset iter_i
// keeping track of possible blocks that need to to skipped
static void findEndOfLine(QString &out, QStringView data, QString::const_iterator &iter_prev,
                  QString::const_iterator &iter_i, QString::const_iterator &iter_end)
{
   // find end of the line

   iter_end = iter_i + 1;
   int nb   = 0;

   while (iter_end != data.constEnd() && iter_end[-1] != '\n') {
      // while looking for the end of the line we might encounter a block
      // that needs to be passed unprocessed.

      QChar prevCh_1 = iter_end[-1];
      QChar prevCh_2 = '\0';

      if (iter_end - 1 != data.constBegin()) {
         prevCh_2 = iter_end[-2];
      }

      if ((prevCh_1 == '\\' || prevCh_1  == '@') && (prevCh_2 != '\\' && prevCh_2 != '@')) {
         // not escaped

         // 4th is passing pristineChars1
         QStringView s1 = QStringView(iter_end - 1, data.constEnd());
         QStringView s2 = QStringView(iter_end - 2, iter_end - 1);

         QString endBlockName = isBlockCommand(s1, s1.constEnd(), s2);
         ++iter_end;

         if (! endBlockName.isEmpty()) {
            int len = endBlockName.length();

            for (  ; iter_end != data.end(); ++iter_end) {
               // search for end of block marker
               QChar ch       = *iter_end;
               QChar prevCh_1 = iter_end[-1];

               if ((ch == '\\' || ch  == '@') && prevCh_1 != '\\' && prevCh_1 != '@') {

                  QStringView s3 = QStringView(iter_end + 1, data.constEnd());

                  if (s3.startsWith(endBlockName)){

                     if (iter_prev != data.constEnd()) {
                        // output previous line if available
                        out += QStringView(iter_prev, iter_i);
                     }

                     // found end marker, skip over this block
                     out += QStringView(iter_i, iter_end + len + 1);

                     iter_prev = data.constEnd();
                     iter_i    = iter_end + len + 1;             // continue after block
                     iter_end  = iter_i + 1;

                     break;
                  }
               }
            }
         }

      } else if (nb == 0 && prevCh_1 == '<' && (prevCh_2 != '\\' && prevCh_2 != '@')) {

         if (QStringView(iter_end, iter_end + 4).toLower() == "pre>") {
            // <pre> tag

            if (iter_prev != data.constEnd()) {
               // output previous line if available
               out += QStringView(iter_prev, iter_i);
            }

            // output part until <pre>
            out += QStringView(iter_i, iter_end - 1);

            // output part until </pre>, last one is passing pristineChars1
            QStringView s4 = QStringView(iter_end - 1, data.constEnd());

            iter_i = iter_end - 1 + processHtmlTag(out, s4, s4.constEnd(), QStringView(iter_end - 2, data.constEnd()));

            iter_prev = data.constEnd();
            iter_end  = iter_i + 1;
            break;

         } else {
            ++iter_end;
         }

      } else if (nb == 0 && prevCh_1 == '`') {

         while (iter_end != data.constEnd() && prevCh_1 == '`') {
            prevCh_1 = *iter_end;

            ++iter_end;
            ++nb;
         }

      } else if (nb > 0 && prevCh_1 == '`') {
         int end_nb = 0;

         while (iter_end != data.constEnd() && prevCh_1 == '`') {
            prevCh_1 = *iter_end;

            ++iter_end;
            ++end_nb;
         }

         if (end_nb == nb) {
            nb = 0;
         }

      } else {
         ++iter_end;
      }
   }
}

static void writeFencedCodeBlock(QString &out, QStringView data, const QString &lang_t,
                  QString::const_iterator iter_blockStart, QString::const_iterator iter_blockEnd)
{
   QString lang = lang_t;

   if (! lang.isEmpty() && lang.startsWith('.')) {
      lang = lang.mid(1);
   }

   out += "@code";

   if (! lang.isEmpty()) {
      out += "{" + lang + "}";
   }

   out += QStringView(iter_blockStart, iter_blockEnd);
   out += "\n";
   out += "@endcode";
}

static QString processQuotations(QStringView str, int refIndent)
{
   QString retval;
   QString lang;

   QString::const_iterator iter_i    = str.constBegin();
   QString::const_iterator iter_prev = str.constEnd();
   QString::const_iterator iter_end  = str.constBegin();

   QString::const_iterator iter_blockStart;
   QString::const_iterator iter_blockEnd;

   int blockOffset;

   while (iter_i != str.constEnd()) {
      findEndOfLine(retval, str, iter_prev, iter_i, iter_end);

      // line is now found at [i..end)
      if (iter_prev != str.constEnd()) {
         QStringView s1 = QStringView(iter_prev, str.constEnd());

         if (isFencedCodeBlock(s1, refIndent, lang, iter_blockStart, iter_blockEnd, blockOffset)) {

            writeFencedCodeBlock(retval, s1, lang, iter_blockStart, iter_blockEnd);
            iter_i    = iter_prev + blockOffset;
            iter_prev = str.constEnd();
            iter_end  = iter_i + 1;
            continue;


         } else if (isBlockQuote(s1, iter_i, refIndent)) {

            iter_i    = writeBlockQuote(retval, s1);
            iter_prev = str.constEnd();
            iter_end  = iter_i + 1;
            continue;

         } else {
            retval += QStringView(iter_prev, iter_i);
         }
      }

      iter_prev = iter_i;
      iter_i    = iter_end;
   }

   if (iter_prev != str.constEnd()) {
      // deal with the last line
      QStringView s1 = QStringView(iter_prev, str.constEnd());

      if (isBlockQuote(s1, s1.constEnd(), refIndent)) {
         writeBlockQuote(retval, s1);

      } else {
         retval += s1;
      }
   }

   return retval;
}

static QString processBlocks(QStringView str, int indent)
{
   QString retval;

   QString::const_iterator iter_i    = str.constBegin();
   QString::const_iterator iter_prev = str.constEnd();
   QString::const_iterator iter_end  = str.constBegin();

   QString::const_iterator iter_blockStart;
   QString::const_iterator iter_blockEnd;

   QString id;
   QString link;
   QString title;

   int ref;
   int level;

   int blockOffset;
   int blockIndent = indent;

   // get number of spaces in the first line
   int sp = 0;

   QChar prevCh = *iter_end;
   ++iter_end;

   while (iter_end <= str.constEnd() && prevCh != '\n') {

      if (prevCh == ' ') {
         ++sp;
      }

      prevCh = *iter_end;
      ++iter_end;
   }

   int priorLine2_Indent  = -1;
   int priorLine1_Indent  = -1;
   int currentLine_Indent = -1;

   // process each line
   while (iter_i != str.constEnd()) {
      findEndOfLine(retval, str, iter_prev, iter_i, iter_end);

      // line is now found at [i .. end - 1]

      priorLine2_Indent  = priorLine1_Indent;
      priorLine1_Indent  = currentLine_Indent;
      currentLine_Indent = computeIndentExcludingListMarkers(QStringView(iter_i, iter_end));

      if (iter_prev != str.constEnd()) {

         QString lang;
         blockIndent = indent;

         QStringView s1 = QStringView(iter_prev, str.constEnd());
         QStringView s2 = QStringView(iter_i,    str.constEnd());

         level = isHeaderline(QStringView(iter_i, str.constEnd()));

         if (level > 0) {

            while (iter_prev != str.constEnd() && *iter_prev == ' ') {
               ++iter_prev;
            }

            QString header = QStringView(iter_prev, iter_i - 1);
            QString id = extractTitleId(header);

            if (! header.isEmpty()) {

               if (! id.isEmpty()) {
                  retval += level == 1 ? "@section " : "@subsection ";
                  retval += id;
                  retval += " ";
                  retval += header;
                  retval += "\n\n";

                  QSharedPointer<SectionInfo> si (Doxy_Globals::sectionDict.find(id));

                  if (si) {
                     if (si->lineNr != -1) {
                        warn(g_fileName, g_lineNr, "multiple use of section label '%s', (first occurrence: %s, line %d)",
                             csPrintable(header), csPrintable(si->fileName), si->lineNr);

                     } else {
                        warn(g_fileName, g_lineNr, "multiple use of section label '%s', (first occurrence: %s)",
                             csPrintable(header), csPrintable(si->fileName));
                     }

                  } else {
                     si = QSharedPointer<SectionInfo> (new SectionInfo(g_fileName, g_lineNr, id, header,
                                          level == 1 ? SectionInfo::Section : SectionInfo::Subsection, level));

                     if (g_current) {
                        g_current->m_anchors.append(*si);
                     }

                     Doxy_Globals::sectionDict.insert(id, si);
                  }

               } else {
                  retval += level == 1 ? "<h1>" : "<h2>";
                  retval += header;
                  retval += level == 1 ? "\n</h1>\n" : "\n</h2>\n";
               }

            } else {
               retval += "<hr>\n";
            }

            iter_prev = str.constEnd();
            iter_i    = iter_end;
            iter_end  = iter_i + 1;

            continue;

         } else if ((ref = isLinkRef(s1, id, link, title))) {

            g_linkRefs.insert(id.toLower(), LinkRef(link, title));

            iter_i    = iter_prev + ref;
            iter_prev = str.constEnd();
            iter_end  = iter_i + 1;

         } else if (isFencedCodeBlock(s1, indent, lang, iter_blockStart, iter_blockEnd, blockOffset)) {

            writeFencedCodeBlock(retval, s1, lang, iter_blockStart, iter_blockEnd);
            iter_i    = iter_prev + blockOffset;
            iter_prev = str.constEnd();
            iter_end  = iter_i + 1;
            continue;

         } else {
            bool isCB = isCodeBlock(s2, blockIndent, priorLine1_Indent, priorLine2_Indent);

            if (isCB) {
               // skip previous line since it is empty

               iter_i    += writeCodeBlock(retval, s2, blockIndent);
               iter_prev = str.constEnd();
               iter_end  = iter_i + 1;
               continue;

            } else if (isTableBlock(s1)) {

               iter_i    = iter_prev + writeTableBlock(retval, s1);
               iter_prev = str.constEnd();
               iter_end  = iter_i + 1;
               continue;

            } else {
               writeOneLineHeaderOrRuler(retval, s1, iter_i);

            }
         }
      }

      iter_prev = iter_i;
      iter_i    = iter_end;
   }

   if (iter_prev != str.constEnd()) {
      // deal with the last line
      QStringView s1 = QStringView(iter_prev, str.constEnd());

      if (isLinkRef(s1, id, link, title)) {
         g_linkRefs.insert(id.toLower(), LinkRef(link, title));

      } else {
         writeOneLineHeaderOrRuler(retval, s1, s1.constEnd());

      }
   }

   return retval;
}

// returns TRUE if text starts with \@page or \@mainpage command
static bool isExplicitPage(QStringView text)
{
   if (! text.isEmpty())  {

      QString::const_iterator iter_i   = text.constBegin();
      QString::const_iterator iter_end = text.constEnd();

      while (iter_i != iter_end && (*iter_i == ' ' || *iter_i == '\n')) {
         ++iter_i;
      }

      if (iter_i == iter_end) {
         // no remaining chars in text

      } else {
         QChar c = *iter_i;

         if (c == '\\' || c == '@') {
            QStringView tmp = QStringView(iter_i + 1, iter_end);

            if (tmp.startsWith("page ") || tmp.startsWith("mainpage")) {
               return true;
            }
         }
      }
   }

   return false;
}

static QString extractPageTitle(QString &docs, QString &id)
{
   // first non-empty line
   QString title;

   QString::const_iterator iter      = docs.constBegin();
   QString::const_iterator iter_end  = docs.constEnd();

   int maxLen = 0;

   while (iter != iter_end) {
      QChar c = *iter;

      if (c == ' ') {
         // do nothing

      } else if (c == '\n') {
         ++maxLen;

      } else {
         break;

      }

      ++iter;
   }

   if (iter == iter_end) {
      return QString("");
   }

   QString::const_iterator iter_endA = iter + 1;

   while (iter_endA != iter_end && iter_endA[-1] != '\n') {
      ++iter_endA;
   }

   if (iter_endA != iter_end) {
      ++maxLen;

      // second line form end1..end2
      QString::const_iterator iter_endB = iter_endA + 1;

      while (iter_endB != iter_end && iter_endB[-1] != '\n') {
         ++iter_endB;
      }

      int level = isHeaderline(QStringView(iter_endA, iter_end));

      if (level != 0) {
         title = QStringView(iter, iter_end - 1);

         QString lns(maxLen, '\n');

         docs = lns + QStringView(iter_endB, iter_end);      // modify passed values
         id   = extractTitleId(title);

         return title;
      }
   }

   if (iter < iter_endA) {
      int level = isAtxHeader(QStringView(iter, iter_end), (iter_endA - iter), title, id);

      if (level > 0) {
         docs = QStringView(iter_endA, iter_end);               // modify passed values
      }
   }

   return title;
}

static QString detab(QStringView str, int &refIndent)
{
   static const int tabSize = Config::getInt("tab-size");

   QString retval;
   int col = 0;

   const int maxIndent = 1000000;          // value representing infinity
   int minIndent       = maxIndent;

   for (QChar c : str) {

      switch (c.unicode()) {
         case '\t': {
            // expand tab

            int stop = tabSize - (col % tabSize);
            col += stop;

            while (stop--) {
               retval += ' ';
            }
         }
         break;

         case '\n':
            // reset colomn counter

            retval += c;
            col = 0;
            break;

         case ' ':
            // increment column counter

            retval += c;
            col++;
            break;

         default:
            // non-whitespace => update minIndent
            retval += c;

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

   return retval;
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
      return out;
   }

   // process quotation blocks (as these may contain other blocks)
   s = processQuotations(s, refIndent);

   // process block items (headers, rules, code blocks, references)
   s = processBlocks(s, refIndent);

   // process the inline markup (links, emphasis and code spans)
   processInline(out, s, s.constEnd());

   return out;
}

QString markdownFileNameToId(const QString &fileName)
{
   QString baseFn = stripFromPath(QFileInfo(fileName).absoluteFilePath());

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
   static const QString mdfileAsMainPage = Config::getString("mdfile-mainpage");
   QSharedPointer<Entry> current = QMakeShared<Entry>();

   current->setData(EntryKey::File_Name, fileName);
   current->setData(EntryKey::MainDocs_File, fileName);
   current->docLine  = 1;
   current->m_srcLang = SrcLangExt_Markdown;

   QString id;
   QString docs    = fileBuf;
   QString title   = extractPageTitle(docs, id).trimmed();

   QString titleFn = QFileInfo(fileName).baseName();
   QString fn      = QFileInfo(fileName).fileName();

   if (id.isEmpty()) {
      id = markdownFileNameToId(fileName);
   }

   if (! isExplicitPage(docs))  {

      if (! mdfileAsMainPage.isEmpty() && (fn == mdfileAsMainPage ||
                  QFileInfo(fileName).absoluteFilePath() == QFileInfo(mdfileAsMainPage).absoluteFilePath()) )  {

         // tag option set to use md file as mainpage
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

   // even without markdown support enabled, still parse an md file as markdown
   bool markdownEnabled = Doxy_Globals::markdownSupport;
   Doxy_Globals::markdownSupport = true;

   bool needsEntry = false;
   Protection prot = Public;

   while (parseCommentBlock(this, current, docs, fileName,lineNr,
             false, false, false, prot, position, needsEntry)) {

      if (needsEntry) {
         QString docFile = current->getData(EntryKey::MainDocs_File);
         root->addSubEntry(current, root);

         current = QMakeShared<Entry>();

         current->setData(EntryKey::MainDocs_File, docFile);
         current->docLine   = lineNr;
         current->m_srcLang = SrcLangExt_Markdown;
      }
   }

   if (needsEntry) {
      root->addSubEntry(current, root);
   }

   // restore setting
   Doxy_Globals::markdownSupport = markdownEnabled;
}

void MarkdownFileParser::parseCode(CodeOutputInterface &codeOutIntf, const QString &scopeName,
                  const QString &input, SrcLangExt lang, bool isExampleBlock, const QString &exampleName,
                  QSharedPointer<FileDef> fileDef, int startLine, int endLine, bool inlineFragment,
                  QSharedPointer<MemberDef> memberDef, bool showLineNumbers, QSharedPointer<Definition> searchCtx,
                  bool collectXRefs )
{
   ParserInterface *pIntf = Doxy_Globals::parserManager.getParser("*.cpp");

   if (pIntf != this) {
      pIntf->parseCode(codeOutIntf, scopeName, input, lang, isExampleBlock, exampleName,
                       fileDef, startLine, endLine, inlineFragment, memberDef, showLineNumbers,
                       searchCtx, collectXRefs);
   }
}

void MarkdownFileParser::resetCodeParserState()
{
   ParserInterface *pIntf = Doxy_Globals::parserManager.getParser("*.cpp");

   if (pIntf != this) {
      pIntf->resetCodeParserState();
   }
}

void MarkdownFileParser::parsePrototype(const QString &text)
{
   ParserInterface *pIntf = Doxy_Globals::parserManager.getParser("*.cpp");

   if (pIntf != this) {
      pIntf->parsePrototype(text);
   }
}

