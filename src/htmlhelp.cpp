/************************************************************************
*
* Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
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

#include <QList>
#include <QRegularExpression>

#include <stdio.h>
#include <stdlib.h>

#include <htmlhelp.h>

#include <config.h>
#include <doxy_globals.h>
#include <language.h>
#include <message.h>
#include <qtextcodec.h>
#include <util.h>

HtmlHelp *HtmlHelp::theInstance = nullptr;

HtmlHelpIndex::HtmlHelpIndex(HtmlHelp *help)
   : m_help(help)
{
}

/*! Stores an item in the index if it is not already present.
 *  Items are stored in alphabetical order by sorting on the
 *  concatenation of \a level1 and \a level2 (if present).
 *
 *  \param level1 the string at level 1 in the index.
 *  \param level2 the string at level 2 in the index (or 0 if not applicable).
 *  \param url the url of the documentation (without .html extension).
 *  \param anchor the anchor of the documentation within the page.
 *  \param hasLink if true, the url (without anchor) can be used in the
 *         level1 item, when writing the header of a list of level2 items.
 *  \param reversed true if level1 is the member name and level2 the compound
 *         name.
 */
void HtmlHelpIndex::addItem(const QString &level1, const QString &level2, const QString &url, const QString &anchor,
                  bool hasLink, bool reversed)
{
   QString key = level1;

   if (! level2.isEmpty()) {
      key += "?" + level2;
   }

   if (key.indexOf(QRegularExpression("@[0-9]+")) != -1) {
      // skip anonymous stuff
      return;
   }

   QString key_anchor = key;

   if (! anchor.isEmpty())  {
      key_anchor += anchor;
   }

   if (m_indexFieldDict.find(key_anchor) == 0) {
      // new key
      QSharedPointer<IndexField> tmp = QMakeShared<IndexField>();

      tmp->name     = key;
      tmp->url      = url;
      tmp->anchor   = anchor;
      tmp->link     = hasLink;
      tmp->reversed = reversed;

      m_indexFieldDict.insert(key_anchor, tmp);
   }
}

static QString field2URL(QSharedPointer<IndexField> f, bool checkReversed)
{
   QString result = f->url + Doxy_Globals::htmlFileExtension;

   if (! f->anchor.isEmpty() && (! checkReversed || f->reversed)) {
      result += "#" + substitute(f->anchor, ":", "%3A");
   }

   return result;
}

/*! Writes the sorted list of index items into a html like list.
 *
 *  An list of calls with <code>name = level1,level2</code> as follows:
 *  <pre>
 *    a1,b1
 *    a1,b2
 *    a2,b1
 *    a2,b2
 *    a3
 *    a4,b1
 *  </pre>
 *
 *  Will result in the following list:
 *
 *  <pre>
 *    a1       -> link to url if hasLink==true
 *      b1     -> link to url#anchor
 *      b2     -> link to url#anchor
 *    a2       -> link to url if hasLink==true
 *      b1     -> link to url#anchor
 *      b2     -> link to url#anchor
 *    a3       -> link to url if hasLink==true
 *    a4       -> link to url if hasLink==true
 *      b1     -> link to url#anchor
 *  </pre>
 */
void HtmlHelpIndex::writeFields(QTextStream &t)
{
   QString lastLevel1;
   bool level2Started = false;

   auto nextItem = m_indexFieldDict.begin();

   for (auto f : m_indexFieldDict) {
      QString level1;
      QString level2;

      int i;

      // strange code but this is ok for now
      ++nextItem;

      if ((i = f->name.indexOf('?')) != -1) {

         level1 = f->name.left(i);
         level2 = f->name.right(f->name.length() - i - 1);

      } else {
         level1 = f->name;

      }

      if (level1 != lastLevel1) {
         // finish old list at level 2

         if (level2Started) {
            t << "  </UL>" << endl;
         }

         level2Started = false;

         // Added this code so that an item with only one subitem is written
         // without any subitem.
         // For example:
         //   a1, b1 -> will create only a1, not separate subitem for b1
         //   a2, b2
         //   a2, b3

         if ( nextItem != m_indexFieldDict.end() ) {

            QString nextLevel1;
            QSharedPointer<IndexField> fnext = *nextItem;

            nextLevel1 = fnext->name.left(fnext->name.indexOf('?'));

            if (level1 != nextLevel1) {
               level2 = QString();
            }

         } else {
            level2 = QString();

         }

         if (level2.isEmpty()) {
            t << "  <LI><OBJECT type=\"text/sitemap\">";
            t << "<param name=\"Local\" value=\"" << field2URL(f, false);
            t << "\">";
            t << "<param name=\"Name\" value=\"" << m_help->recode(level1) << "\">"
                 "</OBJECT>\n";

         } else {
            if (f->link) {
               t << "  <LI><OBJECT type=\"text/sitemap\">";
               t << "<param name=\"Local\" value=\"" << field2URL(f, true);
               t << "\">";
               t << "<param name=\"Name\" value=\"" << m_help->recode(level1) << "\">"
                    "</OBJECT>\n";

            } else {
               t << "  <LI><OBJECT type=\"text/sitemap\">";
               t << "<param name=\"See Also\" value=\"" << m_help->recode(level1) << "\">";
               t << "<param name=\"Name\" value=\"" << m_help->recode(level1) << "\">"
                    "</OBJECT>\n";
            }
         }
      }

      if (! level2Started && ! level2.isEmpty()) {
         // start new list at level 2
         t << "  <UL>" << endl;
         level2Started = true;

      } else if (level2Started && level2.isEmpty()) {
         // end list at level 2
         t << "  </UL>" << endl;
         level2Started = false;
      }

      if (level2Started) {
         t << "    <LI><OBJECT type=\"text/sitemap\">";
         t << "<param name=\"Local\" value=\"" << field2URL(f, false);
         t << "\">";
         t << "<param name=\"Name\" value=\"" << m_help->recode(level2) << "\">"
           "</OBJECT>\n";
      }

      lastLevel1 = level1;
   }

   if (level2Started) {
      t << "  </UL>" << endl;
   }

}


HtmlHelp::HtmlHelp()
   : m_index(this), dc(0)
{
}

HtmlHelp::~HtmlHelp()
{
}

static QHash<QString, QString> s_languageDict;

/*! This will create a contents file (index.hhc) and a index file (index.hhk)
 *  and write the header of those files. Also creates a project file (index.hhp)
 *  \sa finalize()
 */
void HtmlHelp::initialize()
{
   static const QString chmIndexEnc = Config::getString("chm-index-encoding");
   static const QString htmlOutput  = Config::getString("html-output");

   QString str;

   if (chmIndexEnc.isEmpty()) {
      str = "Windows-1250";   // use safe and likely default
   }

   m_toNewCodec = QTextCodec::codecForName(str.toUtf8());

   if (m_toNewCodec == nullptr) {
      err("Unsupported character conversion for 'CHM INDEX ENCODING' '%s'\n", csPrintable(str));
      Doxy_Work::stopDoxyPress();
   }

   /* open the contents file */
   static const QString fName1 = htmlOutput + "/index.hhc";
   cf.setFileName(fName1);

   if (! cf.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fName1), cf.error());
      Doxy_Work::stopDoxyPress();
   }

   /* Write the header of the contents file */
   cts.setDevice(&cf);

   cts << "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n"
       "<HTML><HEAD></HEAD><BODY>\n"
       "<OBJECT type=\"text/site properties\">\n"
       "<param name=\"FrameName\" value=\"right\">\n"
       "</OBJECT>\n"
       "<UL>\n";

   /* open the contents file */
   static const QString fName2 = htmlOutput + "/index.hhk";
   kf.setFileName(fName2);

   if (! kf.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fName2), kf.error());
      Doxy_Work::stopDoxyPress();
   }

   /* Write the header of the contents file */
   kts.setDevice(&kf);

   kts << "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n"
       "<HTML><HEAD></HEAD><BODY>\n"
       "<OBJECT type=\"text/site properties\">\n"
       "<param name=\"FrameName\" value=\"right\">\n"
       "</OBJECT>\n"
       "<UL>\n";

   // language codes for Html help
   s_languageDict.clear();

   s_languageDict.insert("afrikaans",           "0x436 Afrikaans");
   s_languageDict.insert("arabic",              "0xC01 Arabic (Egypt)");
   s_languageDict.insert("armenian",            "0x42b Armenian");
   s_languageDict.insert("brazilian",           "0x416 Portuguese (Brazil)");
   s_languageDict.insert("catalan",             "0x403 Catalan");
   s_languageDict.insert("chinese",             "0x804 Chinese (PRC)");
   s_languageDict.insert("chinese-traditional", "0x404 Chinese (Taiwan)");
   s_languageDict.insert("croatian",            "0x41A Croatian");
   s_languageDict.insert("czech",               "0x405 Czech");
   s_languageDict.insert("danish",              "0x406 Danish");
   s_languageDict.insert("dutch",               "0x413 Dutch");
   s_languageDict.insert("english",             "0x409 English");
   s_languageDict.insert("farsi (persian)",     "0x429 Farsi");
   s_languageDict.insert("finnish",             "0x40B Finnish");
   s_languageDict.insert("french",              "0x40C French");
   s_languageDict.insert("german",              "0x407 German");
   s_languageDict.insert("greek",               "0x408 Greece");
   s_languageDict.insert("hungarian",           "0x40E Hungarian");
   s_languageDict.insert("indonesian",          "0x412 Indonesian");
   s_languageDict.insert("italian",             "0x410 Italian");
   s_languageDict.insert("japanese",            "0x411 Japanese");
   s_languageDict.insert("japanese-en",         "0x411 Japanese");
   s_languageDict.insert("korean",              "0x412 Korean");
   s_languageDict.insert("korean-en",           "0x412 Korean");
   s_languageDict.insert("latvian",             "0x426 Latvian");
   s_languageDict.insert("lithuanian",          "0x427 Lithuanian");
   s_languageDict.insert("macedonian",          "0x042f Macedonian");
   s_languageDict.insert("norwegian",           "0x814 Norwegian");
   s_languageDict.insert("polish",              "0x415 Polish");
   s_languageDict.insert("portuguese",          "0x816 Portuguese (Portugal)");
   s_languageDict.insert("persian",             "0x429 Persian (Iran)"),
   s_languageDict.insert("romanian",            "0x418 Romanian");
   s_languageDict.insert("russian",             "0x419 Russian");
   s_languageDict.insert("serbian",             "0x81A Serbian (Serbia, Latin)");
   s_languageDict.insert("serbian-cyrillic",    "0xC1A Serbian (Serbia, Cyrillic)");
   s_languageDict.insert("slovene",             "0x424 Slovenian");
   s_languageDict.insert("slovak",              "0x41B Slovak");
   s_languageDict.insert("spanish",             "0x40A Spanish (Traditional Sort)");
   s_languageDict.insert("swedish",             "0x41D Swedish");
   s_languageDict.insert("turkish",             "0x41F Turkey");
   s_languageDict.insert("ukrainian",           "0x422 Ukrainian");
   s_languageDict.insert("vietnamese",          "0x42A Vietnamese");

   // Esperanto should be as shown but the htmlhelp compiler 1.3 does not support it
   // fallback to the default language of English
}

static QString getLanguageString()
{
   static const QString outputLanguage = Config::getEnum("output-language").toLower();

   QString retval = s_languageDict[outputLanguage];

   if (retval.isEmpty()) {
      return QString("0x409 English");
   } else {
      return retval;
   }
}

void HtmlHelp::createProjectFile()
{
   // Write the project file for html Help
   static const bool binaryToc      = Config::getBool("binary-toc");
   static const QString chmFile     = Config::getString("chm-file");
   static const QString fName       = Config::getString("html-output") + "/index.hhp";
   static const bool genChi         = Config::getBool("generate-chi");
   static const QString projectName = Config::getString("project-name");

   QFile f(fName);

   if (f.open(QIODevice::WriteOnly)) {
      QTextStream t(&f);

      QString hhcFile = "\"index.hhc\"";
      QString hhkFile = "\"index.hhk\"";

      bool hhkPresent = m_index.size() > 0;

      if (! ctsItemPresent) {
         hhcFile.clear();
      }

      if (! hhkPresent) {
         hhkFile.clear();
      }

      QString indexName = "index" + Doxy_Globals::htmlFileExtension;

      t << "[OPTIONS]\n";
      if (! chmFile.isEmpty()) {
         t << "Compiled file=" << chmFile << endl;
      }

      t << "Compatibility=1.1\n"
           "Full-text search=Yes\n";

      if (ctsItemPresent) {
        t << "Contents file=index.hhc" << endl;
      }

      t << "Default Window=main\n"
           "Default topic=" << indexName << endl;

      if (hhkPresent) {
        t << "Index file=index.hhk" << endl;
      }

      t << "Language=" << getLanguageString() << endl;

      if (binaryToc) {
         t << "Binary TOC=YES" << endl;
      }

      if (genChi) {
         t << "Create CHI file=YES" << endl;
      }

      t << "Title=" << recode(projectName) << endl << endl;

      t << "[WINDOWS]" << endl;

      // NOTE: the 0x10387e number is a set of bits specifying the buttons
      //       which should appear in the CHM viewer. this value means
      //       "show all buttons including the font-size one".
      //       the font-size one is not normally settable by the HTML Help Workshop
      //       utility but the way to set it is described here:
      //          http://support.microsoft.com/?scid=kb%3Ben-us%3B240062&x=17&y=18
      // NOTE: the 0x70387e number in addition to the above the Next and Prev button
      //       are shown. They can only be shown in case of a binary toc.
      //          dee http://www.mif2go.com/xhtml/htmlhelp_0016_943addingtabsandtoolbarbuttonstohtmlhelp.htm#Rz108x95873
      //       Value has been taken from htmlhelp.h file of the HTML Help Workshop

      if (binaryToc) {
         t << "main=\"" << recode(projectName) << "\"," << hhcFile << ","
           << hhkFile   << ",\"" << indexName << "\",\"" <<
              indexName << "\",,,,,0x23520,,0x70387e,,,,,,,,0" << endl << endl;

      } else {
         t << "main=\"" << recode(projectName) << "\"," << hhcFile << ","
           << hhkFile   << ",\"" << indexName << "\",\"" <<
              indexName << "\",,,,,0x23520,,0x10387e,,,,,,,,0" << endl << endl;
      }

      t << "[FILES]" << endl;

      for (const auto &s : indexFiles) {
         t << s << endl;
      }

      for (const auto &item : imageFiles) {
         t << item << endl;
      }

      f.close();

   } else {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fName), f.error());
   }
}

void HtmlHelp::addIndexFile(const QString &file)
{
   if (! indexFileDict.contains(file)) {
      indexFiles.append(file);
      indexFileDict.insert(file);
   }
}

/*! Finalizes the HTML help. This will finish and close the
 *  contents file (index.hhc) and the index file (index.hhk).
 *  \sa initialize()
 */
void HtmlHelp::finalize()
{
   // end the contents file
   cts << "</UL>\n";
   cts << "</BODY>\n";
   cts << "</HTML>\n";
   cts.setDevice(0);

   cf.close();

   m_index.writeFields(kts);

   // end the index file
   kts << "</UL>\n";
   kts << "</BODY>\n";
   kts << "</HTML>\n";
   kts.setDevice(0);

   kf.close();

   createProjectFile();
   s_languageDict.clear();
}

/*! Increase the level of the contents hierarchy.
 *  This will start a new unnumbered HTML list in contents file.
 *  \sa decContentsDepth()
 */
void HtmlHelp::incContentsDepth()
{
   for (int i = 0; i < dc + 1; ++i) {
      cts << "  ";
   }

   cts << "<UL>\n";
   ++dc;
}

/*! Decrease the level of the contents hierarchy.
 *  This will end the unnumber HTML list.
 *  \sa incContentsDepth()
 */
void HtmlHelp::decContentsDepth()
{
   for (int i = 0; i < dc; ++i) {
      cts << "  ";
   }

   cts << "</UL>\n";
   --dc;
}

QString HtmlHelp::recode(const QString &s)
{
   return m_toNewCodec->fromUnicode(s);
}

/*! Add an list item to the contents file.
 *  \param isDir boolean indicating if this is a dir or file entry
 *  \param name the name of the item.
 *  \param ref  the URL of to the item.
 *  \param file the file in which the item is defined.
 *  \param anchor the anchor of the item.
 *  \param def not used.
 */
void HtmlHelp::addContentsItem(bool isDir, const QString &name, const QString &ref, const QString &file, const QString &anchor,
                  bool unused, QSharedPointer<Definition> def, DirType category)
{
   (void) ref;
   (void) unused;
   (void) def;
   (void) category;

   // If we are using a binary toc then folders cannot have links.
   // Tried this and I did not see any problems, when not using the
   // resetting of file and anchor the TOC works better // (prev / next button)

   static const bool binaryToc = Config::getBool("binary-toc");

   ctsItemPresent = true;

   for (int i = 0; i < dc; i++) {
      cts << "  ";
   }

   cts << "<LI><OBJECT type=\"text/sitemap\">";
   cts << "<param name=\"Name\" value=\"" << convertToHtml(recode(name), true) << "\">";

   if (! file.isEmpty()) {
      // made file optional param

      if (file[0] == '!' || file[0] == '^') {
         // special markers for user defined URLs

         cts << "<param name=\"";
         if (file[0] == '^') {
            cts << "URL";
         } else {
            cts << "Local";
         }

         cts << "\" value=\"";
         cts << file.mid(1);
         cts << "\">";

      } else {

         if (! (binaryToc && isDir)) {
            cts << "<param name=\"Local\" value=\"";
            cts << file << Doxy_Globals::htmlFileExtension;

            if (! anchor.isEmpty()) {
               cts << "#" << anchor;
            }
            cts << "\">";
         }
      }
   }

   cts << "<param name=\"ImageNumber\" value=\"";

   if (isDir) {
      cts << (int)BOOK_CLOSED;
   } else {
      cts << (int)TEXT;
   }

   cts << "\">";
   cts << "</OBJECT>\n";
}

void HtmlHelp::addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md,
                  const QString &sectionAnchor, const QString &word)
{
   if (md) {
      static const bool separateMemberPages = Config::getBool("separate-member-pages");

      if (context == 0) {
         // global member

         if (md->getGroupDef()) {
            context = md->getGroupDef();

         } else if (md->getFileDef()) {
            context = md->getFileDef();
         }
      }

      if (context == nullptr) {
         return;   // should not happen
      }

      QString cfname  = md->getOutputFileBase();
      QString cfiname = context->getOutputFileBase();
      QString level1  = context->name();
      QString level2  = md->name();
      QString contRef = separateMemberPages ? cfname : cfiname;
      QString memRef  = cfname;

      QString anchor;

      if (sectionAnchor.isEmpty()) {
         anchor = md->anchor();
      } else {
         anchor = sectionAnchor;
      }

      m_index.addItem(level1, level2, contRef, anchor, true, false);
      m_index.addItem(level2, level1, memRef, anchor, true, true);

   } else if (context) {
      QString level1;

      if (word.isEmpty()) {
         level1 = context->name();
      } else {
         level1 = word;
      }

      m_index.addItem(level1, QString(), context->getOutputFileBase(), sectionAnchor, true, false);
   }
}

void HtmlHelp::addImageFile(const QString &fileName)
{
   if (! imageFiles.contains(fileName)) {
      imageFiles.append(fileName);
   }
}
