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

#include <QList>
#include <QRegExp>

#include <stdio.h>
#include <stdlib.h>

#include <config.h>
#include <doxygen.h>
#include <doxy_globals.h>
#include <filedef.h>
#include <groupdef.h>
#include <htmlhelp.h>
#include <language.h>
#include <message.h>
#include <memberdef.h>
#include <qtextcodec.h>
#include <stringmap.h>
#include <util.h>

/** Class representing a field in the HTML help index. */
struct IndexField {
   QString name;
   QByteArray url;
   QByteArray anchor;

   bool link;
   bool reversed;
};

/** Sorted dictionary of IndexField objects. */
class IndexFieldSDict : public StringMap<QSharedPointer<IndexField>>
{
 public:
   IndexFieldSDict() : StringMap<QSharedPointer<IndexField>>() {}
   ~IndexFieldSDict() {}

 private:
   int compareMapValues(const QSharedPointer<IndexField> &item1, const QSharedPointer<IndexField> &item2) const override {
      return item1->name.compare(item2->name, Qt::CaseInsensitive);
   }
};

/** A helper class for HtmlHelp that manages a two level index in
 *  alphabetical order.
 */
class HtmlHelpIndex
{
 public:
   HtmlHelpIndex(HtmlHelp *help);
   ~HtmlHelpIndex();

   void addItem(const char *first, const char *second, const char *url, const char *anchor, bool hasLink, bool reversed);
   void writeFields(QTextStream &t);

 private:
   IndexFieldSDict *dict;
   HtmlHelp *m_help;
};

/*! Constructs a new HtmlHelp index */
HtmlHelpIndex::HtmlHelpIndex(HtmlHelp *help) : m_help(help)
{
   dict = new IndexFieldSDict;   
}

/*! Destroys the HtmlHelp index */
HtmlHelpIndex::~HtmlHelpIndex()
{
   delete dict;
}

/*! Stores an item in the index if it is not already present.
 *  Items are stored in alphetical order, by sorting on the
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
void HtmlHelpIndex::addItem(const char *level1, const char *level2, const char *url, const char *anchor, 
                            bool hasLink, bool reversed)
{
   QString key = level1;

   if (level2) {
      key += (QString)"?" + level2;
   }

   if (key.indexOf(QRegExp("@[0-9]+")) != -1) { 
      // skip anonymous stuff
      return;
   }

   if (dict->find(key) == 0) { 
      // new key
   
      QSharedPointer<IndexField> f (new IndexField);

      f->name     = key;
      f->url      = url;
      f->anchor   = anchor;
      f->link     = hasLink;
      f->reversed = reversed;

      dict->insert(key, f);
   }
}

static QByteArray field2URL(QSharedPointer<IndexField> f, bool checkReversed)
{
   QByteArray result = f->url + Doxygen::htmlFileExtension;

   if (! f->anchor.isEmpty() && (!checkReversed || f->reversed)) {
      result += "#" + f->anchor;
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

   auto nextItem = dict->begin();

   for (auto f : *dict) {
      QString level1;
      QString level2;

      int i;

      // strange code but this is ok for now
      ++nextItem; 

      if ((i = f->name.indexOf('?')) != -1) {

         level1 = f->name.left(i);
         level2 = f->name.right(f->name.length() - i - 1);

      } else {
         level1  = f->name;

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

         if ( nextItem != dict->end() ) {

            QString nextLevel1;
            QSharedPointer<IndexField> fnext = *nextItem;
              
            nextLevel1 = fnext->name.left(fnext->name.indexOf('?'));            
            
            if (level1 != nextLevel1) {
               level2 = "";
            }

         } else {
            level2 = "";

         }
        
         if (level2.isEmpty()) {
            t << "  <LI><OBJECT type=\"text/sitemap\">";
            t << "<param name=\"Local\" value=\"" << field2URL(f, true);
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
      if (!level2Started && !level2.isEmpty()) {
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

//----------------------------------------------------------------------------

HtmlHelp *HtmlHelp::theInstance = 0;

/*! Constructs an html object.
 *  The object has to be \link initialize() initialized\endlink before it can
 *  be used.
 */
HtmlHelp::HtmlHelp() : indexFileDict()
{
   /* initial depth */
   dc = 0;
   cf = 0;
   kf = 0;

   index = new HtmlHelpIndex(this);  
}

HtmlHelp::~HtmlHelp()
{  
   delete index;
}

static QHash<QString, QByteArray *> s_languageDict;         

/*! This will create a contents file (index.hhc) and a index file (index.hhk)
 *  and write the header of those files.
 *  It also creates a project file (index.hhp)
 *  \sa finalize()
 */
void HtmlHelp::initialize()
{
   const char *str = Config_getString("CHM_INDEX_ENCODING");

   if (! str) {
      str = "Windows-1250";   // use safe and likely default
   }

   m_toNewCodec = QTextCodec::codecForName(str);

   if (m_toNewCodec == nullptr) {
      err("Unsupported character conversion for CHM_INDEX_ENCODING: '%s'\n", str);
      exit(1);
   }

   /* open the contents file */
   QByteArray fName = Config_getString("HTML_OUTPUT") + "/index.hhc";
   cf = new QFile(fName);

   if (! cf->open(QIODevice::WriteOnly)) {
      err("Could not open file %s for writing\n", fName.data());
      exit(1);
   }

   /* Write the header of the contents file */
   cts.setDevice(cf);

   cts << "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n"
       "<HTML><HEAD></HEAD><BODY>\n"
       "<OBJECT type=\"text/site properties\">\n"
       "<param name=\"FrameName\" value=\"right\">\n"
       "</OBJECT>\n"
       "<UL>\n";

   /* open the contents file */
   fName = Config_getString("HTML_OUTPUT") + "/index.hhk";
   kf = new QFile(fName);

   if (!kf->open(QIODevice::WriteOnly)) {
      err("Could not open file %s for writing\n", fName.data());
      exit(1);
   }

   /* Write the header of the contents file */
   kts.setDevice(kf);
   kts << "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n"
       "<HTML><HEAD></HEAD><BODY>\n"
       "<OBJECT type=\"text/site properties\">\n"
       "<param name=\"FrameName\" value=\"right\">\n"
       "</OBJECT>\n"
       "<UL>\n";

   /* language codes for Html help
      0x405 Czech
      0x406 Danish
      0x413 Dutch
      0xC09 English (Australia)
      0x809 English (Britain)
      0x1009 English (Canada)
      0x1809 English (Ireland)
      0x1409 English (New Zealand)
      0x1C09 English (South Africa)
      0x409 English (United States)
      0x40B Finnish
      0x40C French
      0x407 German
      0x408 Greece
      0x40E Hungarian
      0x410 Italian
      0x814 Norwegian
      0x415 Polish
      0x816 Portuguese(Portugal)
      0x416 Portuguese(Brazil)
      0x419 Russian
      0x80A Spanish(Mexico)
      0xC0A Spanish(Modern Sort)
      0x40A Spanish(Traditional Sort)
      0x41D Swedish
      0x41F Turkey
      0x411 Japanese
      0x412 Korean
      0x804 Chinese (PRC)
      0x404 Chinese (Taiwan)

      New LCIDs:
    0x421 Indonesian
    0x41A Croatian
    0x418 Romanian
    0x424 Slovenian
    0x41B Slovak
    0x422 Ukrainian
    0x81A Serbian (Serbia, Latin)
    0x403 Catalan
    0x426 Latvian
    0x427 Lithuanian
    0x436 Afrikaans
    0x42A Vietnamese
    0x429 Persian (Iran)
    0xC01 Arabic (Egypt) - I don't know which version of arabic is used inside translator_ar.h ,
      so I have chosen Egypt at random

   */
 
   s_languageDict.clear();
   s_languageDict.insert("czech",       new QByteArray("0x405 Czech"));
   s_languageDict.insert("danish",      new QByteArray("0x406 Danish"));
   s_languageDict.insert("dutch",       new QByteArray("0x413 Dutch"));
   s_languageDict.insert("finnish",     new QByteArray("0x40B Finnish"));
   s_languageDict.insert("french",      new QByteArray("0x40C French"));
   s_languageDict.insert("german",      new QByteArray("0x407 German"));
   s_languageDict.insert("greek",       new QByteArray("0x408 Greece"));
   s_languageDict.insert("hungarian",   new QByteArray("0x40E Hungarian"));
   s_languageDict.insert("italian",     new QByteArray("0x410 Italian"));
   s_languageDict.insert("norwegian",   new QByteArray("0x814 Norwegian"));
   s_languageDict.insert("polish",      new QByteArray("0x415 Polish"));
   s_languageDict.insert("portuguese",  new QByteArray("0x816 Portuguese(Portugal)"));
   s_languageDict.insert("brazilian",   new QByteArray("0x416 Portuguese(Brazil)"));
   s_languageDict.insert("russian",     new QByteArray("0x419 Russian"));
   s_languageDict.insert("spanish",     new QByteArray("0x40A Spanish(Traditional Sort)"));
   s_languageDict.insert("swedish",     new QByteArray("0x41D Swedish"));
   s_languageDict.insert("turkish",     new QByteArray("0x41F Turkey"));
   s_languageDict.insert("japanese",    new QByteArray("0x411 Japanese"));
   s_languageDict.insert("japanese-en", new QByteArray("0x411 Japanese"));
   s_languageDict.insert("korean",      new QByteArray("0x412 Korean"));
   s_languageDict.insert("korean-en",   new QByteArray("0x412 Korean"));
   s_languageDict.insert("chinese",     new QByteArray("0x804 Chinese (PRC)"));
   s_languageDict.insert("chinese-traditional", new QByteArray("0x404 Chinese (Taiwan)"));

   // new LCIDs
   s_languageDict.insert("indonesian",  new QByteArray("0x412 Indonesian"));
   s_languageDict.insert("croatian",    new QByteArray("0x41A Croatian"));
   s_languageDict.insert("romanian",    new QByteArray("0x418 Romanian"));
   s_languageDict.insert("slovene",     new QByteArray("0x424 Slovenian"));
   s_languageDict.insert("slovak",      new QByteArray("0x41B Slovak"));
   s_languageDict.insert("ukrainian",   new QByteArray("0x422 Ukrainian"));
   s_languageDict.insert("serbian",     new QByteArray("0x81A Serbian (Serbia, Latin)"));
   s_languageDict.insert("catalan",     new QByteArray("0x403 Catalan"));
   s_languageDict.insert("lithuanian",  new QByteArray("0x427 Lithuanian"));
   s_languageDict.insert("afrikaans",   new QByteArray("0x436 Afrikaans"));
   s_languageDict.insert("vietnamese",  new QByteArray("0x42A Vietnamese"));
   s_languageDict.insert("persian",     new QByteArray("0x429 Persian (Iran)"));
   s_languageDict.insert("arabic",      new QByteArray("0xC01 Arabic (Egypt)"));
   s_languageDict.insert("latvian",     new QByteArray("0x426 Latvian"));
   s_languageDict.insert("macedonian",  new QByteArray("0x042f Macedonian (Former Yugoslav Republic of Macedonia)"));
   s_languageDict.insert("armenian",    new QByteArray("0x42b Armenian"));

   //Code for Esperanto should be as shown below but the htmlhelp compiler 1.3 does not support this
   // (and no newer version is available).
   //So do a fallback to the default language (see getLanguageString())
   //s_languageDict.insert("esperanto",   new QByteArray("0x48f Esperanto"));
   s_languageDict.insert("serbian-cyrillic", new QByteArray("0xC1A Serbian (Serbia, Cyrillic)"));
}


static QByteArray getLanguageString()
{
   if (!theTranslator->idLanguage().isEmpty()) {
      QByteArray *s = s_languageDict[theTranslator->idLanguage()];
      if (s) {
         return *s;
      }
   }
   // default language
   return "0x409 English (United States)";
}



void HtmlHelp::createProjectFile()
{
   /* Write the project file */
   QByteArray fName = Config_getString("HTML_OUTPUT") + "/index.hhp";
   QFile f(fName);

   if (f.open(QIODevice::WriteOnly)) {
      QTextStream t(&f);

      QByteArray indexName = "index" + Doxygen::htmlFileExtension;

      t << "[OPTIONS]\n";
      if (!Config_getString("CHM_FILE").isEmpty()) {
         t << "Compiled file=" << Config_getString("CHM_FILE") << "\n";
      }
      t << "Compatibility=1.1\n"
        "Full-text search=Yes\n"
        "Contents file=index.hhc\n"
        "Default Window=main\n"
        "Default topic=" << indexName << "\n"
        "Index file=index.hhk\n"
        "Language=" << getLanguageString() << endl;

      if (Config_getBool("BINARY_TOC")) {
         t << "Binary TOC=YES\n";
      }

      if (Config_getBool("GENERATE_CHI")) {
         t << "Create CHI file=YES\n";
      }

      t << "Title=" << recode(Config_getString("PROJECT_NAME")) << endl << endl;

      t << "[WINDOWS]" << endl;

      // NOTE: the 0x10387e number is a set of bits specifying the buttons
      //       which should appear in the CHM viewer; that specific value
      //       means "show all buttons including the font-size one";
      //       the font-size one is not normally settable by the HTML Help Workshop
      //       utility but the way to set it is described here:
      //          http://support.microsoft.com/?scid=kb%3Ben-us%3B240062&x=17&y=18
      // NOTE: the 0x70387e number in addition to the above the Next and Prev button
      //       are shown. They can only be shown in case of a binary toc.
      //          dee http://www.mif2go.com/xhtml/htmlhelp_0016_943addingtabsandtoolbarbuttonstohtmlhelp.htm#Rz108x95873
      //       Value has been taken from htmlhelp.h file of the HTML Help Workshop

      if (Config_getBool("BINARY_TOC")) {
         t << "main=\"" << recode(Config_getString("PROJECT_NAME")) << "\",\"index.hhc\","
           "\"index.hhk\",\"" << indexName << "\",\"" <<
           indexName << "\",,,,,0x23520,,0x70387e,,,,,,,,0" << endl << endl;

      } else {
         t << "main=\"" << recode(Config_getString("PROJECT_NAME")) << "\",\"index.hhc\","
           "\"index.hhk\",\"" << indexName << "\",\"" <<
           indexName << "\",,,,,0x23520,,0x10387e,,,,,,,,0" << endl << endl;
      }

      t << "[FILES]" << endl;

      for (auto s : indexFiles) {
         t << s << endl;
      }

      uint i;
      for (i = 0; i < imageFiles.count(); i++) {
         t << imageFiles.at(i) << endl;
      }

      f.close();

   } else {
      err("Could not open file %s for writing\n", fName.data());
   }
}

void HtmlHelp::addIndexFile(const char *s)
{
   if (indexFileDict.contains(s)) {
      indexFiles.append(s);
      indexFileDict.insert(s, (void *)0x8);
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
   cf->close();
   delete cf;

   index->writeFields(kts);

   // end the index file
   kts << "</UL>\n";
   kts << "</BODY>\n";
   kts << "</HTML>\n";
   kts.setDevice(0);
   kf->close();
   delete kf;

   createProjectFile();
   s_languageDict.clear();
}

/*! Increase the level of the contents hierarchy.
 *  This will start a new unnumbered HTML list in contents file.
 *  \sa decContentsDepth()
 */
void HtmlHelp::incContentsDepth()
{
   int i;

   for (i = 0; i < dc + 1; i++) {
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
   int i;

   for (i = 0; i < dc; i++) {
      cts << "  ";
   }

   cts << "</UL>\n";
   --dc;
}

QByteArray HtmlHelp::recode(const QString &s)
{
   return m_toNewCodec->fromUnicode(s);
}

/*! Add an list item to the contents file.
 *  \param isDir boolean indicating if this is a dir or file entry
 *  \param name the name of the item.
 *  \param ref  the URL of to the item.
 *  \param file the file in which the item is defined.
 *  \param anchor the anchor of the item.
 *  \param separateIndex not used.
 *  \param addToNavIndex not used.
 *  \param def not used.
 */
void HtmlHelp::addContentsItem(bool isDir, const QString &name, const char *ref, const char *file, const char *anchor, 
            bool seperateIndex, bool addToNavIndex, QSharedPointer<Definition> def)
{
   // If we're using a binary toc then folders cannot have links.
   // Tried this and I didn't see any problems, when not using
   // the resetting of file and anchor the TOC works better
   // (prev / next button)
  
   int i;

   for (i = 0; i < dc; i++) {
      cts << "  ";
   }

   cts << "<LI><OBJECT type=\"text/sitemap\">";
   cts << "<param name=\"Name\" value=\"" << convertToHtml(recode(name), true) << "\">";

   if (file) {    // made file optional param - KPW
      if (file && (file[0] == '!' || file[0] == '^')) { // special markers for user defined URLs
         cts << "<param name=\"";
         if (file[0] == '^') {
            cts << "URL";
         } else {
            cts << "Local";
         }

         cts << "\" value=\"";
         cts << &file[1];

      } else {
         cts << "<param name=\"Local\" value=\"";
         cts << file << Doxygen::htmlFileExtension;
         if (anchor) {
            cts << "#" << anchor;
         }
      }
      cts << "\">";
   }

   cts << "<param name=\"ImageNumber\" value=\"";

   if (isDir) { // added - KPW
      cts << (int)BOOK_CLOSED ;
   } else {
      cts << (int)TEXT;
   }

   cts << "\">";
   cts << "</OBJECT>\n";
}


void HtmlHelp::addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md, const char *sectionAnchor, const char *word)
{
   if (md) {
      static bool separateMemberPages = Config_getBool("SEPARATE_MEMBER_PAGES");

      if (context == 0) { // global member
         if (md->getGroupDef()) {
            context = md->getGroupDef();

         } else if (md->getFileDef()) {
            context = md->getFileDef();
         }
      }

      if (context == 0) {
         return;   // should not happen
      }

      QByteArray cfname  = md->getOutputFileBase();
      QByteArray cfiname = context->getOutputFileBase();
      QByteArray level1  = context->name();
      QByteArray level2  = md->name();
      QByteArray contRef = separateMemberPages ? cfname : cfiname;
      QByteArray memRef  = cfname;
      QByteArray anchor  = sectionAnchor ? QByteArray(sectionAnchor) : md->anchor();
      index->addItem(level1, level2, contRef, anchor, true, false);
      index->addItem(level2, level1, memRef, anchor, true, true);

   } else if (context) {
      QByteArray level1  = word ? QByteArray(word) : context->name();
      index->addItem(level1, 0, context->getOutputFileBase(), sectionAnchor, true, false);
   }
}

void HtmlHelp::addImageFile(const char *fileName)
{
   if (!imageFiles.contains(fileName)) {
      imageFiles.append(fileName);
   }
}

