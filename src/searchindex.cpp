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

#include <QFile>
#include <QRegExp>
#include <QTextStream>

#include <ctype.h>
#include <assert.h>

#include <classlist.h>
#include <classdef.h>
#include <config.h>
#include <doxy_globals.h>
#include <doxy_build_info.h>
#include <filename.h>
#include <filedef.h>
#include <growbuf.h>
#include <groupdef.h>
#include <language.h>
#include <message.h>
#include <memberdef.h>
#include <membername.h>
#include <namespacedef.h>
#include <pagedef.h>
#include <resourcemgr.h>
#include <searchindex.h>
#include <util.h>

// file format: (all multi-byte values are stored in big endian format)
//   4 byte header
//   256*256*4 byte index (4 bytes)
//   for each index entry: a zero terminated list of words
//   for each word: a \0 terminated string + 4 byte offset to the stats info
//   padding bytes to align at 4 byte boundary
//   for each word: the number of urls (4 bytes)
//               + for each url containing the word 8 bytes statistics
//                 (4 bytes index to url string + 4 bytes frequency counter)
//   for each url: a \0 terminated string

const int numIndexEntries = 256 * 256;

IndexWord::IndexWord(const QString &word) : m_word(word)
{  
}

void IndexWord::addUrlIndex(int idx, bool hiPriority)
{  
   QSharedPointer<URLInfo> ui = m_urls.value(idx);

   if (! ui) {     
      ui =  QMakeShared<URLInfo>(idx, 0);
      m_urls.insert(idx, ui);
   }

   ui->freq += 2;

   if (hiPriority) {
      ui->freq |= 1;   // mark as high priority document
   }
}

SearchIndex::SearchIndex() : SearchIndexIntf(Internal), m_urlIndex(-1)
{
   int i;
  
   for (i = 0; i < numIndexEntries; i++) {
      m_index.append(QList<IndexWord *>());
   }
}

SearchIndex::~SearchIndex()
{
   for (auto item : m_words) {
      delete item;
   }
}

void SearchIndex::setCurrentDoc(QSharedPointer<Definition> ctx, const QString &anchor, bool isSourceFile)
{
   if (ctx == 0) {
      return;
   }

   assert(! isSourceFile || ctx->definitionType() == Definition::TypeFile);
   
   QString url = isSourceFile ? ctx.dynamicCast<FileDef>()->getSourceFileBase() : ctx->getOutputFileBase();
   url += Doxy_Globals::htmlFileExtension.toUtf8();

   QString baseUrl = url;

   if (! anchor.isEmpty()) {
      url += "#" + anchor;
   }

   if (! isSourceFile) {
      baseUrl = url;
   }

   QString name = ctx->qualifiedName();

   if (ctx->definitionType() == Definition::TypeMember) {
      QSharedPointer<MemberDef> md = ctx.dynamicCast<MemberDef>();

      if (md->getLanguage() == SrcLangExt_Fortran) {
         name.prepend(theTranslator->trSubprogram(true, true));

      } else {
         name.prepend(theTranslator->trMember(true, true) + " ");
      }  

   } else { 
      // compound type
      SrcLangExt lang = ctx->getLanguage();

      QString sep = getLanguageSpecificSeparator(lang);

      if (sep != "::") {
         name = substitute(name, "::", sep);
      }

      switch (ctx->definitionType()) {
         case Definition::TypePage: {
            QSharedPointer<PageDef> pd = ctx.dynamicCast<PageDef>();

            if (!pd->title().isEmpty()) {
               name = theTranslator->trPage(true, true) + " " + pd->title();
            } else {
               name = theTranslator->trPage(true, true) + " " + pd->name();
            }
         }
         break;

         case Definition::TypeClass: {
            QSharedPointer<ClassDef> cd = ctx.dynamicCast<ClassDef>();
            name.prepend(cd->compoundTypeString() + " ");
         }
         break;

         case Definition::TypeNamespace: {

            if (lang == SrcLangExt_Java || lang == SrcLangExt_CSharp) {
               name = theTranslator->trPackage(name);

            } else if (lang == SrcLangExt_Fortran) {
               name.prepend(theTranslator->trModule(true, true) + " ");

            } else {
               name.prepend(theTranslator->trNamespace(true, true) + " ");
            }
         }
         break;

         case Definition::TypeGroup: {
            QSharedPointer<GroupDef> gd = ctx.dynamicCast<GroupDef>();

            if (! gd->groupTitle().isEmpty()) {
               name = theTranslator->trGroup(true, true) + " " + gd->groupTitle();

            } else {
               name.prepend(theTranslator->trGroup(true, true) + " ");
            }
         }

         break;

         default:
            break;
      }
   }

   auto pIndex = m_url2IdMap.find(baseUrl);

   if (pIndex == m_url2IdMap.end()) {
      ++m_urlIndex;

      m_url2IdMap.insert(baseUrl, m_urlIndex);
      m_urls.insert(m_urlIndex, QSharedPointer<URL>(new URL(name, url)));

   } else {
      m_urls.insert(*pIndex, QSharedPointer<URL>(new URL(name, url)));
   }

}

static int charsToIndex(const QString &word)
{
   if (word.length() < 2) {
      return -1;
   }
 
   // hashing that allows for substring searching
   uint c1 = word[0].unicode(); 
   uint c2 = word[1].unicode();
   
   return c1 * 256 + c2;
}

void SearchIndex::addWord(const QString &word, bool hiPriority, bool recurse)
{
   if (word.isEmpty()) {
      return;
   }

   static QRegExp nextPart("[_a-z:][A-Z]");

   QString wStr = word.toLower();   
   IndexWord *w = m_words.value(wStr);

   if (! w) {
      int idx = charsToIndex(wStr);
     
      if (idx < 0) {
         return;
      }

      w = new IndexWord(wStr);         
      m_index[idx].append(w);
      m_words.insert(wStr, w);
   }

   w->addUrlIndex(m_urlIndex, hiPriority);

   int i;
   bool found = false;

   if (! recurse) { 
      // the first time we check if we can strip the prefix
      i = getPrefixIndex(word.toUtf8());

      if (i > 0) {
         addWord(word.mid(i), hiPriority, true);
         found = true;
      }
   }

   if (! found) { 
      // no prefix stripped
      i = nextPart.indexIn(word);

      if (i >= 1) {       
         addWord(word.mid(i+1), hiPriority, true);
      }
   }
}

void SearchIndex::addWord(const QString &word, bool hiPriority)
{
   addWord(word, hiPriority, false);
}

static void writeInt(QFile &f, int index)
{
   f.putChar(((uint)index) >> 24);
   f.putChar((((uint)index) >> 16) & 0xff);
   f.putChar((((uint)index) >> 8) & 0xff);
   f.putChar(((uint)index) & 0xff);
}

static void writeString(QFile &file, const QString &str)
{  
   file.write(str.toUtf8());
   file.putChar(0);
}

void SearchIndex::write(const QString &fileName)
{
   int i;
   int size = 4; 

   // for the header
   size += 4 * numIndexEntries; 

   // for the index
   int wordsOffset = size;

   // pass 1: compute the size of the wordlist
   for (i = 0; i < numIndexEntries; i++) {
      QList<IndexWord *> &wlist = m_index[i];

      if (! wlist.isEmpty()) {
 
         for (auto iw : wlist) {    
            int ws = iw->word().length() + 1;

            // word + url info list offset
            size += ws + 4; 
         }

         // zero list terminator
         size += 1; 
      }
   }

   // pass 2: compute the offsets in the index
   int indexOffsets[numIndexEntries];
   int offset = wordsOffset;

   for (i = 0; i < numIndexEntries; i++) {
      QList<IndexWord *> &wlist = m_index[i];

      if (! wlist.isEmpty()) {
         indexOffsets[i] = offset;

         for (auto iw : wlist) {  
            offset += iw->word().length() + 1;

            // word + offset to url info array
            offset += 4; 
         }

         // zero list terminator
         offset += 1; 

      } else {
         indexOffsets[i] = 0;
      }
   }

   int padding = size;
   size = (size + 3) & ~3; // round up to 4 byte boundary
   padding = size - padding;

   // 
   int *wordStatOffsets = new int[m_words.count()];
   int count = 0;

   // third pass: compute offset to stats info for each word
   for (i = 0; i < numIndexEntries; i++) {   
      QList<IndexWord *> &wlist = m_index[i];

      if (! wlist.isEmpty()) {
        
         for (auto iw : wlist) { 
            wordStatOffsets[count++] = size;
            size += 4 + iw->urls().count() * 8; 
         }
      }
   }

   int *urlOffsets = new int[m_urls.count()];   

   for (auto iter = m_urls.begin(); iter != m_urls.end(); ++iter) {    
      urlOffsets[iter.key()] = size;
      size += iter.value()->name.length() + 1 + iter.value()->url.length() + 1;
   }
  
   QFile f(fileName);

   if (f.open(QIODevice::WriteOnly)) {
      // write header
      f.putChar('D');
      f.putChar('O');
      f.putChar('X');
      f.putChar('S');

      // write index
      for (i = 0; i < numIndexEntries; i++) {
         writeInt(f, indexOffsets[i]);
      }

      // write word lists
      count = 0;

      for (i = 0; i < numIndexEntries; i++) {
         QList<IndexWord *> &wlist = m_index[i];

         if (! wlist.isEmpty()) {           
            for (auto item : wlist) {
               writeString(f, item->word());
               writeInt(f, wordStatOffsets[count++]);
            }

            f.putChar(0);
         }
      }

      // write extra padding bytes
      for (i = 0; i < padding; i++) {
         f.putChar(0);
      }

      // write word statistics
      for (i = 0; i < numIndexEntries; i++) {
         QList<IndexWord *> &wlist = m_index[i];

         if (! wlist.isEmpty()) {          

            for (auto item1 : wlist) {            
               int numUrls = item1->urls().count();
               writeInt(f, numUrls);
              
               for (auto item2 : item1->urls() ) {
                  writeInt(f, urlOffsets[item2->urlIdx]);
                  writeInt(f, item2->freq);
               }
            }
         }
      }

      // write urls
     
      for (auto item : m_urls) {
         writeString(f, item->name);
         writeString(f, item->url);
      }
   }

   delete[] urlOffsets;
   delete[] wordStatOffsets;
}


// the following part is for writing an external search index
struct SearchDocEntry {
   QString type;
   QString name;
   QString args;
   QString extId;
   QString url;

   GrowBuf  importantText;
   GrowBuf  normalText;
};

SearchIndexExternal::SearchIndexExternal() : SearchIndexIntf(External)
{
}

SearchIndexExternal::~SearchIndexExternal()
{
}

static QString definitionToName(QSharedPointer<Definition> ctx)
{
   if (ctx && ctx->definitionType() == Definition::TypeMember) {
      QSharedPointer<MemberDef> md = ctx.dynamicCast<MemberDef>();

      if (md->isFunction()) {
         return "function";
      } else if (md->isSlot()) {
         return "slot";
      } else if (md->isSignal()) {
         return "signal";
      } else if (md->isVariable()) {
         return "variable";
      } else if (md->isTypedef()) {
         return "typedef";
      } else if (md->isEnumerate()) {
         return "enum";
      } else if (md->isEnumValue()) {
         return "enumvalue";
      } else if (md->isProperty()) {
         return "property";
      } else if (md->isEvent()) {
         return "event";
      } else if (md->isRelated() || md->isForeign()) {
         return "related";
      } else if (md->isFriend()) {
         return "friend";
      } else if (md->isDefine()) {
         return "define";
      }

   } else if (ctx) {
      switch (ctx->definitionType()) {
         case Definition::TypeClass:
            return ctx.dynamicCast<ClassDef>()->compoundTypeString();

         case Definition::TypeFile:
            return "file";
         case Definition::TypeNamespace:
            return "namespace";
         case Definition::TypeGroup:
            return "group";
         case Definition::TypePackage:
            return "package";
         case Definition::TypePage:
            return "page";
         case Definition::TypeDir:
            return "dir";
         default:
            break;
      }
   }
   return "unknown";
}

void SearchIndexExternal::setCurrentDoc(QSharedPointer<Definition> ctx, const QString &anchor, bool isSourceFile)
{
   QString extId = stripPath(Config::getString("external-search-id"));

   QString baseName = isSourceFile ? ctx.dynamicCast<FileDef>()->getSourceFileBase() : ctx->getOutputFileBase();
   QString url = baseName + Doxy_Globals::htmlFileExtension.toUtf8();

   if (! anchor.isEmpty()) {
      url += "#" + anchor;
   }

   QString key = extId + ";" + url;

   m_current = m_docEntries.find(key);

   if (! m_current) {
      QSharedPointer<SearchDocEntry> e(new SearchDocEntry);

      e->type = isSourceFile ? "source" : definitionToName(ctx);
      e->name = ctx->qualifiedName();

      if (ctx->definitionType() == Definition::TypeMember) {
         e->args = ctx.dynamicCast<MemberDef>()->argsString();
      }

      e->extId = extId;
      e->url  = url;

      m_current = e;
      m_docEntries.insert(key, e);      
   }
}

void SearchIndexExternal::addWord(const QString &word, bool hiPriority)
{
   if (word.isEmpty() || ! isId(word[0].unicode()) || m_current == 0) {
      return;
   }

   GrowBuf *pText = hiPriority ? &m_current->importantText : &m_current->normalText;

   if (pText->getPos() > 0) {
      pText->addChar(' ');
   }

   pText->addStr(word); 
}

void SearchIndexExternal::write(const QString &fileName)
{
   QFile f(fileName);

   if (f.open(QIODevice::WriteOnly)) {
      QTextStream t(&f);
      t << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
      t << "<add>" << endl;
      
      for (auto doc : m_docEntries) {
         doc->normalText.addChar(0);    // make sure buffer ends with a 0 terminator
         doc->importantText.addChar(0); // make sure buffer ends with a 0 terminator

         t << "  <doc>" << endl;
         t << "    <field name=\"type\">"     << doc->type << "</field>" << endl;
         t << "    <field name=\"name\">"     << convertToXML(doc->name) << "</field>" << endl;
         if (!doc->args.isEmpty()) {
            t << "    <field name=\"args\">"     << convertToXML(doc->args) << "</field>" << endl;
         }
         if (!doc->extId.isEmpty()) {
            t << "    <field name=\"tag\">"      << convertToXML(doc->extId)  << "</field>" << endl;
         }
         t << "    <field name=\"url\">"      << convertToXML(doc->url)  << "</field>" << endl;
         t << "    <field name=\"keywords\">" << convertToXML(doc->importantText.get())  << "</field>" << endl;
         t << "    <field name=\"text\">"     << convertToXML(doc->normalText.get())     << "</field>" << endl;
         t << "  </doc>" << endl;
      }
      t << "</add>" << endl;

   } else {
      err("Failed to open file %s for writing!\n", qPrintable(fileName));
   }
}


// the following part is for the javascript based search engine

#define SEARCH_INDEX_ALL         0
#define SEARCH_INDEX_CLASSES     1
#define SEARCH_INDEX_NAMESPACES  2
#define SEARCH_INDEX_FILES       3
#define SEARCH_INDEX_FUNCTIONS   4
#define SEARCH_INDEX_VARIABLES   5
#define SEARCH_INDEX_TYPEDEFS    6
#define SEARCH_INDEX_ENUMS       7
#define SEARCH_INDEX_ENUMVALUES  8
#define SEARCH_INDEX_PROPERTIES  9
#define SEARCH_INDEX_EVENTS     10
#define SEARCH_INDEX_RELATED    11
#define SEARCH_INDEX_DEFINES    12
#define SEARCH_INDEX_GROUPS     13
#define SEARCH_INDEX_PAGES      14
#define NUM_SEARCH_INDICES      15

class SearchDefinitionList : public QList<QSharedPointer<Definition>>
{
 public:
   SearchDefinitionList(uint letter) : m_letter(letter) 
   {}

   uint letter() const {
      return m_letter;
   }

 private:
   uint m_letter;
};

class SearchIndexMap : public StringMap<QSharedPointer<SearchDefinitionList>>
{
 public:   
   SearchIndexMap(uint letter) 
      : StringMap<QSharedPointer<SearchDefinitionList>>(Qt::CaseInsensitive), m_letter(letter)
   { }

   ~SearchIndexMap()
   { }

   void insertDef(QSharedPointer<Definition> d) {

      QSharedPointer<SearchDefinitionList> lx = this->find(d->name());

      if (! lx) {
         lx = QSharedPointer<SearchDefinitionList>(new SearchDefinitionList(m_letter));
         StringMap<QSharedPointer<SearchDefinitionList>>::insert(d->name(), lx);
      }

      lx->append(d);
   }

   uint letter() const {
      return m_letter;
   }

 private:
   int compareMapValues(const QSharedPointer<SearchDefinitionList> &md1, 
                                    const QSharedPointer<SearchDefinitionList> &md2) const override{
      QString n1 = md1->first()->localName();
      QString n2 = md2->first()->localName();

      return n1.compare(n2, Qt::CaseInsensitive);
   }

   uint m_letter;
};

static void addMemberToSearchIndex(LetterToIndexMap<SearchIndexMap> symbols[NUM_SEARCH_INDICES],
                                   int symbolCount[NUM_SEARCH_INDICES], QSharedPointer<MemberDef> md)
{
   static bool hideFriendCompounds = Config::getBool("hide-friend-compounds");
   bool isLinkable = md->isLinkable();

   QSharedPointer<ClassDef>     cd;
   QSharedPointer<NamespaceDef> nd;
   QSharedPointer<FileDef>      fd;
   QSharedPointer<GroupDef>     gd;

   if (isLinkable &&  (((cd = md->getClassDef()) && cd->isLinkable() && cd->templateMaster() == 0) || 
                       ((gd = md->getGroupDef()) && gd->isLinkable())) ) {

      QString n = md->name();

      if (! n.isEmpty()) {
         uint letter = getUtf8CodeToLower(n, 0);
         bool isFriendToHide = hideFriendCompounds && (md->typeString() == "friend class" ||
                                md->typeString() == "friend struct" || md->typeString() == "friend union");

         if (!(md->isFriend() && isFriendToHide)) {
            symbols[SEARCH_INDEX_ALL].insertElement(letter, md);
            symbolCount[SEARCH_INDEX_ALL]++;
         }

         if (md->isFunction() || md->isSlot() || md->isSignal()) {
            symbols[SEARCH_INDEX_FUNCTIONS].insertElement(letter, md);
            symbolCount[SEARCH_INDEX_FUNCTIONS]++;

         } else if (md->isVariable()) {
            symbols[SEARCH_INDEX_VARIABLES].insertElement(letter, md);
            symbolCount[SEARCH_INDEX_VARIABLES]++;

         } else if (md->isTypedef()) {
            symbols[SEARCH_INDEX_TYPEDEFS].insertElement(letter, md);
            symbolCount[SEARCH_INDEX_TYPEDEFS]++;

         } else if (md->isEnumerate()) {
            symbols[SEARCH_INDEX_ENUMS].insertElement(letter, md);
            symbolCount[SEARCH_INDEX_ENUMS]++;

         } else if (md->isEnumValue()) {
            symbols[SEARCH_INDEX_ENUMVALUES].insertElement(letter, md);
            symbolCount[SEARCH_INDEX_ENUMVALUES]++;

         } else if (md->isProperty()) {
            symbols[SEARCH_INDEX_PROPERTIES].insertElement(letter, md);
            symbolCount[SEARCH_INDEX_PROPERTIES]++;

         } else if (md->isEvent()) {
            symbols[SEARCH_INDEX_EVENTS].insertElement(letter, md);
            symbolCount[SEARCH_INDEX_EVENTS]++;

         } else if (md->isRelated() || md->isForeign() || (md->isFriend() && !isFriendToHide)) {
            symbols[SEARCH_INDEX_RELATED].insertElement(letter, md);
            symbolCount[SEARCH_INDEX_RELATED]++;
         }
      }

   } else if (isLinkable &&
              (((nd = md->getNamespaceDef()) && nd->isLinkable()) ||
               ((fd = md->getFileDef())      && fd->isLinkable())) ) {

      QString n = md->name();

      if (!n.isEmpty()) {
         uint letter = getUtf8CodeToLower(n, 0);
         symbols[SEARCH_INDEX_ALL].insertElement(letter, md);
         symbolCount[SEARCH_INDEX_ALL]++;

         if (md->isFunction()) {
            symbols[SEARCH_INDEX_FUNCTIONS].insertElement(letter, md);
            symbolCount[SEARCH_INDEX_FUNCTIONS]++;

         } else if (md->isVariable()) {
            symbols[SEARCH_INDEX_VARIABLES].insertElement(letter, md);
            symbolCount[SEARCH_INDEX_VARIABLES]++;

         } else if (md->isTypedef()) {
            symbols[SEARCH_INDEX_TYPEDEFS].insertElement(letter, md);
            symbolCount[SEARCH_INDEX_TYPEDEFS]++;

         } else if (md->isEnumerate()) {
            symbols[SEARCH_INDEX_ENUMS].insertElement(letter, md);
            symbolCount[SEARCH_INDEX_ENUMS]++;

         } else if (md->isEnumValue()) {
            symbols[SEARCH_INDEX_ENUMVALUES].insertElement(letter, md);
            symbolCount[SEARCH_INDEX_ENUMVALUES]++;

         } else if (md->isDefine()) {
            symbols[SEARCH_INDEX_DEFINES].insertElement(letter, md);
            symbolCount[SEARCH_INDEX_DEFINES]++;
         }
      }
   }
}

// see also function convertToId() in search.js, which should match in behaviour
static QString searchId(const QString &str)
{  
   QString result;

   for ( auto c : str) {          

      if (c.isLetterOrNumber()) { 
         // simply alpha numerical character
         result += c.toLower();

      } else { 
         // other 'unprintable' characters                
         result += QString("_%1").arg(c.unicode(), 2, 16, QChar('0') );
      }
   }

   return result;
}

static int g_searchIndexCount[NUM_SEARCH_INDICES];

static LetterToIndexMap<SearchIndexMap> g_searchIndexSymbols[NUM_SEARCH_INDICES];

static const char *g_searchIndexName[NUM_SEARCH_INDICES] = {
   "all",
   "classes",
   "namespaces",
   "files",
   "functions",
   "variables",
   "typedefs",
   "enums",
   "enumvalues",
   "properties",
   "events",
   "related",
   "defines",
   "groups",
   "pages"
};


class SearchIndexCategoryMapping
{
 public:
   SearchIndexCategoryMapping() {
      categoryLabel[SEARCH_INDEX_ALL]        = theTranslator->trAll();
      categoryLabel[SEARCH_INDEX_CLASSES]    = theTranslator->trClasses();
      categoryLabel[SEARCH_INDEX_NAMESPACES] = theTranslator->trNamespace(true, false);
      categoryLabel[SEARCH_INDEX_FILES]      = theTranslator->trFile(true, false);
      categoryLabel[SEARCH_INDEX_FUNCTIONS]  = theTranslator->trFunctions();
      categoryLabel[SEARCH_INDEX_VARIABLES]  = theTranslator->trVariables();
      categoryLabel[SEARCH_INDEX_TYPEDEFS]   = theTranslator->trTypedefs();
      categoryLabel[SEARCH_INDEX_ENUMS]      = theTranslator->trEnumerations();
      categoryLabel[SEARCH_INDEX_ENUMVALUES] = theTranslator->trEnumerationValues();
      categoryLabel[SEARCH_INDEX_PROPERTIES] = theTranslator->trProperties();
      categoryLabel[SEARCH_INDEX_EVENTS]     = theTranslator->trEvents();
      categoryLabel[SEARCH_INDEX_RELATED]    = theTranslator->trFriends();
      categoryLabel[SEARCH_INDEX_DEFINES]    = theTranslator->trDefines();
      categoryLabel[SEARCH_INDEX_GROUPS]     = theTranslator->trGroup(true, false);
      categoryLabel[SEARCH_INDEX_PAGES]      = theTranslator->trPage(true, false);
   }

   QString categoryLabel[NUM_SEARCH_INDICES];
};

void writeJavascriptSearchIndex()
{
   if (! Config::getBool("generate-html")) {
      return;
   }

   // index classes
   for (auto cd : *Doxy_Globals::classSDict) {

      uint letter = getUtf8CodeToLower(cd->localName(), 0);

      if (cd->isLinkable() && isId(letter)) {
         g_searchIndexSymbols[SEARCH_INDEX_ALL].insertElement(letter, cd);
         g_searchIndexSymbols[SEARCH_INDEX_CLASSES].insertElement(letter, cd);

         g_searchIndexCount[SEARCH_INDEX_ALL]++;
         g_searchIndexCount[SEARCH_INDEX_CLASSES]++;
      }
   }

   // index namespaces  
   for (auto nd : *Doxy_Globals::namespaceSDict) {
      uint letter = getUtf8CodeToLower(nd->name(), 0);

      if (nd->isLinkable() && isId(letter)) {
         g_searchIndexSymbols[SEARCH_INDEX_ALL].insertElement(letter, nd);
         g_searchIndexSymbols[SEARCH_INDEX_NAMESPACES].insertElement(letter, nd);

         g_searchIndexCount[SEARCH_INDEX_ALL]++;
         g_searchIndexCount[SEARCH_INDEX_NAMESPACES]++;
      }
   }

   // index files 
   for (auto fn : *Doxy_Globals::inputNameList) {
     
      for (auto fd : *fn) { 
         uint letter = getUtf8CodeToLower(fd->name(), 0);

         if (fd->isLinkable() && isId(letter)) {
            g_searchIndexSymbols[SEARCH_INDEX_ALL].insertElement(letter, fd);
            g_searchIndexSymbols[SEARCH_INDEX_FILES].insertElement(letter, fd);

            g_searchIndexCount[SEARCH_INDEX_ALL]++;
            g_searchIndexCount[SEARCH_INDEX_FILES]++;
         }
      }
   }

   // index class members, for each member name
   for (auto mn : *Doxy_Globals::memberNameSDict) {
     
      // for each member definition
      for (auto md : *mn) {  
         addMemberToSearchIndex(g_searchIndexSymbols, g_searchIndexCount, md);
      }
   }


   // index file/namespace members, for each member name          
   for (auto mn : *Doxy_Globals::functionNameSDict) {

      // for each member definition         
      for (auto md : *mn) { 
         addMemberToSearchIndex(g_searchIndexSymbols, g_searchIndexCount, md);
      }
   }

   // index groups
   for (auto gd : *Doxy_Globals::groupSDict) {

      if (gd->isLinkable()) {
         QString title = gd->groupTitle();

         if (! title.isEmpty()) { 
            // TODO: able searching for all word in the title

            QChar charCode = title.at(0);
            uint letter = charCode.toLower().unicode();

            if (isId(letter)) {
               g_searchIndexSymbols[SEARCH_INDEX_ALL].insertElement(letter, gd);
               g_searchIndexSymbols[SEARCH_INDEX_GROUPS].insertElement(letter, gd);

               g_searchIndexCount[SEARCH_INDEX_ALL]++;
               g_searchIndexCount[SEARCH_INDEX_GROUPS]++;
            }
         }
      }
   }

   // index pages 
   for (auto pd : *Doxy_Globals::pageSDict) {

      if (pd->isLinkable()) {
         QString title = pd->title();

         if (! title.isEmpty()) {
            QChar charCode = title.at(0);
            uint letter = charCode.toLower().unicode();

            if (isId(letter)) {
               g_searchIndexSymbols[SEARCH_INDEX_ALL].insertElement(letter, pd);
               g_searchIndexSymbols[SEARCH_INDEX_PAGES].insertElement(letter, pd);
               g_searchIndexCount[SEARCH_INDEX_ALL]++;
               g_searchIndexCount[SEARCH_INDEX_PAGES]++;
            }
         }
      }
   }

   if (Doxy_Globals::mainPage) {
      QString title = Doxy_Globals::mainPage->title();

      if (! title.isEmpty()) {
         QChar charCode = title.at(0);
         uint letter = charCode.toLower().unicode();

         if (isId(letter)) {
            g_searchIndexSymbols[SEARCH_INDEX_ALL].insertElement(letter, Doxy_Globals::mainPage);
            g_searchIndexSymbols[SEARCH_INDEX_PAGES].insertElement(letter, Doxy_Globals::mainPage);
            g_searchIndexCount[SEARCH_INDEX_ALL]++;
            g_searchIndexCount[SEARCH_INDEX_PAGES]++;
         }
      }
   }
    
   // write index files
   QString searchDirName = Config::getString("html-output") + "/search";

   for (int i = 0; i < NUM_SEARCH_INDICES; i++) { 
      // for each index    
      int p = 0;
      
      for (auto sl : g_searchIndexSymbols[i]) {   
         // for each letter
         QString baseName = QString("%1_%2").arg(g_searchIndexName[i]).arg(p);

         QString fileName     = searchDirName + "/" + baseName + ".html";
         QString dataFileName = searchDirName + "/" + baseName + ".js";

         QFile outFile(fileName);
         QFile dataOutFile(dataFileName);

         if (outFile.open(QIODevice::WriteOnly) && dataOutFile.open(QIODevice::WriteOnly)) {
            {
               QTextStream t(&outFile);

               t << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\""
                 " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">" << endl;
               t << "<html><head><title></title>" << endl;
               t << "<meta http-equiv=\"Content-Type\" content=\"text/xhtml;charset=UTF-8\"/>" << endl;
               t << "<meta name=\"generator\" content=\"DoxyPress " << versionString << "\"/>" << endl;
               t << "<link rel=\"stylesheet\" type=\"text/css\" href=\"search.css\"/>" << endl;
               t << "<script type=\"text/javascript\" src=\"" << baseName << ".js\"></script>" << endl;
               t << "<script type=\"text/javascript\" src=\"search.js\"></script>" << endl;
               t << "</head>" << endl;
               t << "<body class=\"SRPage\">" << endl;
               t << "<div id=\"SRIndex\">" << endl;
               t << "<div class=\"SRStatus\" id=\"Loading\">" << theTranslator->trLoading() << "</div>" << endl;
               t << "<div id=\"SRResults\"></div>" << endl; // here the results will be inserted
               t << "<script type=\"text/javascript\"><!--" << endl;
               t << "createResults();" << endl; // this function will insert the results
               t << "--></script>" << endl;

               t << "<div class=\"SRStatus\" id=\"Searching\">"
                 << theTranslator->trSearching() << "</div>" << endl;

               t << "<div class=\"SRStatus\" id=\"NoMatches\">"
                 << theTranslator->trNoMatches() << "</div>" << endl;

               t << "<script type=\"text/javascript\"><!--" << endl;
               t << "document.getElementById(\"Loading\").style.display=\"none\";" << endl;
               t << "document.getElementById(\"NoMatches\").style.display=\"none\";" << endl;
               t << "var searchResults = new SearchResults(\"searchResults\");" << endl;
               t << "searchResults.Search();" << endl;
               t << "--></script>" << endl;
               t << "</div>" << endl; // SRIndex
               t << "</body>" << endl;
               t << "</html>" << endl;
            }

            QTextStream ti(&dataOutFile);

            ti << "var searchData=" << endl;
            // format
            // searchData[] = array of items
            // searchData[x][0] = id
            // searchData[x][1] = [ name + child1 + child2 + .. ]
            // searchData[x][1][0] = name as shown
            // searchData[x][1][y+1] = info for child y
            // searchData[x][1][y+1][0] = url
            // searchData[x][1][y+1][1] = 1 => target="_parent"
            // searchData[x][1][y+1][2] = scope

            ti << "[" << endl;

            bool firstEntry = true;           
            int itemCount   = 0;
         
            for (auto dl : *sl) { 
               QSharedPointer<Definition> d = dl->first();
               
               if (! firstEntry) {
                  ti << "," << endl;
               }
               firstEntry = false;

               QString dispName = d->localName();

               if (d->definitionType() == Definition::TypeGroup) {
                  dispName = d.dynamicCast<GroupDef>()->groupTitle();

               } else if (d->definitionType() == Definition::TypePage) {
                  dispName = d.dynamicCast<PageDef>()->title();

               }

               ti << "  ['" << searchId(dispName) << "',['"
                  << convertToXML(dispName) << "',[";

               if (dl->count() == 1) { 
                  // item with a unique name
                  QSharedPointer<MemberDef> md;

                  bool isMemberDef = d->definitionType() == Definition::TypeMember;
                  if (isMemberDef) {
                     md = d.dynamicCast<MemberDef>();
                  }

                  QString anchor = d->anchor();

                  ti << "'" << externalRef("../", d->getReference(), true)
                     << d->getOutputFileBase() << Doxy_Globals::htmlFileExtension;
                  if (!anchor.isEmpty()) {
                     ti << "#" << anchor;
                  }
                  ti << "',";

                  static bool extLinksInWindow = Config::getBool("external-links-in-window");
                  if (!extLinksInWindow || d->getReference().isEmpty()) {
                     ti << "1,";
                  } else {
                     ti << "0,";
                  }

                  if (d->getOuterScope() != Doxy_Globals::globalScope) {
                     ti << "'" << convertToXML(d->getOuterScope()->name()) << "'";

                  } else if (md) {
                     QSharedPointer<FileDef> fd = md->getBodyDef();

                     if (fd == 0) {
                        fd = md->getFileDef();
                     }
                     if (fd) {
                        ti << "'" << convertToXML(fd->localName()) << "'";
                     }

                  } else {
                     ti << "''";
                  }

                  ti << "]]";

               } else { 
                  // multiple items with the same name
                  bool overloadedFunction = false;
                  int childCount = 0;

                  QSharedPointer<Definition> next;
                  QSharedPointer<Definition> prevScope;  
 
                  auto nextIter = dl->begin();   

                  for (auto d : *dl)  {               
                     QSharedPointer<Definition> scope = d->getOuterScope();

                     if (nextIter != dl->end()) {  
                        ++nextIter;   
                     }

                     if (nextIter == dl->end()) {  
                        next = QSharedPointer<Definition>();

                     } else {
                        next = *nextIter;

                     }
                  
                     QSharedPointer<Definition> nextScope;
                     QSharedPointer<MemberDef>  md;

                     bool isMemberDef = d->definitionType() == Definition::TypeMember;

                     if (isMemberDef) {
                        md = d.dynamicCast<MemberDef>();
                     }

                     if (next) {
                        nextScope = next->getOuterScope();
                     }

                     QString anchor = d->anchor();

                     if (childCount > 0) {
                        ti << "],[";
                     }
                     ti << "'" << externalRef("../", d->getReference(), true)
                        << d->getOutputFileBase() << Doxy_Globals::htmlFileExtension;

                     if (!anchor.isEmpty()) {
                        ti << "#" << anchor;
                     }
                     ti << "',";

                     static bool extLinksInWindow = Config::getBool("external-links-in-window");
                     if (! extLinksInWindow || d->getReference().isEmpty()) {
                        ti << "1,";
                     } else {
                        ti << "0,";
                     }

                     bool found = false;
                     overloadedFunction = ((prevScope != 0 && scope == prevScope) || (scope && scope == nextScope)) 
                                             && md && (md->isFunction() || md->isSlot());

                     QString prefix;

                     if (md) {
                        prefix = convertToXML(md->localName());
                     }

                     if (overloadedFunction) {
                        // overloaded member function
                        prefix += convertToXML(md->argsString());
                        // show argument list to disambiguate overloaded functions

                     } else if (md) {
                        // unique member function
                        prefix += "()"; // only to show it is a function
                     }

                     QString name;
                     if (d->definitionType() == Definition::TypeClass) {

                        name  = convertToXML(d.dynamicCast<ClassDef>()->displayName());
                        found = true;

                     } else if (d->definitionType() == Definition::TypeNamespace) {

                        name  = convertToXML(d.dynamicCast<NamespaceDef>()->displayName());
                        found = true;

                     } else if (scope == 0 || scope == Doxy_Globals::globalScope) { 
                        // in global scope

                        if (md) {
                           QSharedPointer<FileDef> fd = md->getBodyDef();

                           if (fd == 0) {
                              fd = md->getFileDef();
                           }

                           if (fd) {
                              if (! prefix.isEmpty()) {
                                 prefix += ":&#160;";
                              }
                              name = prefix + convertToXML(fd->localName());
                              found = true;
                           }
                        }

                     } else if (md && (md->getClassDef() || md->getNamespaceDef())) {
                        // member in class or namespace scope
                     
                        SrcLangExt lang = md->getLanguage();
                        name = convertToXML(d->getOuterScope()->qualifiedName())
                               + getLanguageSpecificSeparator(lang) + prefix;

                        found = true;

                     } else if (scope) {
                        // some thing else? -> show scope

                        name = prefix + convertToXML(scope->name());
                        found = true;
                     }

                     if (! found) { 
                        // fallback
                        name = prefix + "(" + theTranslator->trGlobalNamespace() + ")";
                     }

                     ti << "'" << name << "'";

                     prevScope = scope;
                     childCount++;

                  }

                  ti << "]]";
               }

               ti << "]";
               itemCount++;
            }

            if (! firstEntry) {
               ti << endl;
            }

            ti << "];" << endl;

         } else {
            err("Unable to open file for writing %s, error: %d\n", qPrintable(fileName), outFile.error());
          
         }

         ++p;
      }
   }

   {
      QFile f(searchDirName + "/searchdata.js");

      if (f.open(QIODevice::WriteOnly)) {
         QTextStream t(&f);

         t << "var indexSectionsWithContent =" << endl;
         t << "{" << endl;
         bool first = true;
         int j = 0;

         for (int i = 0; i < NUM_SEARCH_INDICES; i++) {

            if (g_searchIndexCount[i] > 0) {

               if (!first) {
                  t << "," << endl;
               }
               t << "  " << j << ": \"";

               for (auto sl : g_searchIndexSymbols[i]) {
                  // for each letter
                  t << QString( QChar( sl->letter() ) ).toUtf8();
               }
               t << "\"";
               first = false;
               j++;
            }
         }

         if (!first) {
            t << "\n";
         }

         t << "};" << endl << endl;
         t << "var indexSectionNames =" << endl;
         t << "{" << endl;

         first = true;
         j = 0;

         for (int i = 0; i < NUM_SEARCH_INDICES; i++) {
            if (g_searchIndexCount[i] > 0) {
               if (!first) {
                  t << "," << endl;
               }

               t << "  " << j << ": \"" << g_searchIndexName[i] << "\"";
               first = false;
               j++;
            }
         }

         if (! first) {
            t << "\n";
         }

         t << "};" << endl << endl;
         t << "var indexSectionLabels =" << endl;
         t << "{" << endl;

         first = true;
         static SearchIndexCategoryMapping map;
         j = 0;

         for (int i = 0; i < NUM_SEARCH_INDICES; i++) {
            if (g_searchIndexCount[i] > 0) {

               if (!first) {
                  t << "," << endl;
               }

               t << "  " << j << ": \"" << convertToXML(map.categoryLabel[i]) << "\"";
               first = false;
               j++;
            }
         }

         if (!first) {
            t << "\n";
         }

         t << "};" << endl << endl;
      }

      ResourceMgr::instance().copyResourceAs("html/search.js", searchDirName, "search.js");
   }

   {
      QFile f(searchDirName + "/nomatches.html");

      if (f.open(QIODevice::WriteOnly)) {
         QTextStream t(&f);
         t << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" "
           "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">" << endl;
         t << "<html><head><title></title>" << endl;
         t << "<meta http-equiv=\"Content-Type\" content=\"text/xhtml;charset=UTF-8\"/>" << endl;
         t << "<link rel=\"stylesheet\" type=\"text/css\" href=\"search.css\"/>" << endl;
         t << "<script type=\"text/javascript\" src=\"search.js\"></script>" << endl;
         t << "</head>" << endl;
         t << "<body class=\"SRPage\">" << endl;
         t << "<div id=\"SRIndex\">" << endl;
         t << "<div class=\"SRStatus\" id=\"NoMatches\">"
           << theTranslator->trNoMatches() << "</div>" << endl;
         t << "</div>" << endl;
         t << "</body>" << endl;
         t << "</html>" << endl;
      }
   }

   Doxy_Globals::indexList->addStyleSheetFile("search/search.js");
}

void initSearchIndexer()
{
   static bool searchEngine      = Config::getBool("html-search");
   static bool serverBasedSearch = Config::getBool("search-server-based");
   static bool externalSearch    = Config::getBool("search-external");

   if (searchEngine && serverBasedSearch) {
      if (externalSearch) { // external tools produce search index and engine
         Doxy_Globals::searchIndex = new SearchIndexExternal;

      } else {
         // DoxyPress produces search index and engine
         Doxy_Globals::searchIndex = new SearchIndex;
      }

   } else { 
      // no search engine or pure javascript based search function
      Doxy_Globals::searchIndex = 0;
   }
}

void finializeSearchIndexer()
{
   delete Doxy_Globals::searchIndex;
}
