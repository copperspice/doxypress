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
#include <QCryptographicHash>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <definition.h>

#include <code_cstyle.h>
#include <config.h>
#include <doxy_globals.h>
#include <htags.h>
#include <language.h>
#include <message.h>
#include <marshal.h>
#include <outputlist.h>
#include <parse_base.h>
#include <util.h>

#define START_MARKER 0x4445465B    // DEF[
#define END_MARKER   0x4445465D    // DEF]

class Definition_Private
{
 public:
   Definition_Private();
   ~Definition_Private();

   void init(const QString &df, const QString &n);

   SectionDict *sectionDict;                          // dictionary of all sections
   QList<QSharedPointer <SectionInfo>> sectionList;   // list of sections, definiton order

   MemberSDict *sourceRefByDict;
   MemberSDict *sourceRefsDict;

   QList<ListItemInfo> *xrefListItems;
   SortedList<QSharedPointer<GroupDef>> *partOfGroups;

   DocInfo   *details;    // not exported
   DocInfo   *inbodyDocs; // not exported
   BriefInfo *brief;      // not exported
   BodyInfo  *body;       // not exported

   QString briefSignatures;
   QString docSignatures;

   QString localName;      // local (unqualified) name of the definition
   
   QString qualifiedName;
   QString ref;            // reference to external documentation

   bool hidden;
   bool isArtificial;

   QSharedPointer<Definition> outerScope;  

   // where the item was found
   QString defFileName;
   QString defFileExt;

   SrcLangExt lang;

   QString id; // clang unique id
};

Definition_Private::Definition_Private()
   : sectionDict(0), sourceRefByDict(0), sourceRefsDict(0), xrefListItems(0), partOfGroups(0),
     details(0), inbodyDocs(0), brief(0), body(0), hidden(false), isArtificial(false), lang(SrcLangExt_Unknown)
{
}

Definition_Private::~Definition_Private()
{
   delete sectionDict;
   delete sourceRefByDict;
   delete sourceRefsDict;
   delete partOfGroups;
   delete xrefListItems;
   delete brief;
   delete details;
   delete body;
   delete inbodyDocs;
}

void Definition_Private::init(const QString &df, const QString &n)
{
   defFileName = df;
   int lastDot = defFileName.lastIndexOf('.');

   if (lastDot != -1) {
      defFileExt = defFileName.mid(lastDot);
   }

   QString name = n;

   if (name != "<globalScope>") {     
      localName = stripScope(n);

   } else {
      localName = n;
   }
 
   brief           = 0;
   details         = 0;
   body            = 0;
   inbodyDocs      = 0;
   sourceRefByDict = 0;
   sourceRefsDict  = 0;
   sectionDict     = 0,
   outerScope      = Doxy_Globals::globalScope;
   partOfGroups    = 0;
   xrefListItems   = 0;
   hidden          = false;
   isArtificial    = false;
   lang            = SrcLangExt_Unknown;
}

static bool matchExcludedSymbols(const QString &name)
{
   static const QStringList exclSyms = Config::getList("exclude-symbols");

   if (exclSyms.count() == 0) {
      // nothing specified
      return false;   
   }

   QString symName = name;

   for (auto pattern : exclSyms) {          

      bool forceStart = false;
      bool forceEnd   = false;

      if (! pattern.isEmpty() && pattern.at(0) == '^') {
         pattern = pattern.mid(1);
         forceStart = true;
      }

      if (pattern.at(pattern.length() - 1) == '$') {
         pattern = pattern.left(pattern.length() - 1), forceEnd = true;
      }

      if (pattern.indexOf('*') != -1) { 
         // wildcard mode
         QRegExp re(pattern.replace("*", ".*"), Qt::CaseSensitive);

         int i  = re.indexIn(symName, 0);
         int pl = re.matchedLength();   

         if (i != -1) { 
            // wildcard match
            int sl = symName.length();

            // check if it is a whole word match
            if ((i == 0 || pattern.at(0) == '*' || (! isId(symName.at(i - 1).unicode())  && ! forceStart)) &&
                  (i + pl == sl || pattern.at(i + pl) == '*' || (! isId(symName.at(i + pl).unicode()) && ! forceEnd)) ) {      
               return true;
            }
         }

      } else if (! pattern.isEmpty()) { 
         // match words
         int i = symName.indexOf(pattern);

         if (i != -1) { 
            // we have a match!
            int pl = pattern.length();
            int sl = symName.length();

            // check if it is a whole word match
            if ((i == 0  || (! isId(symName.at(i - 1).unicode())  && ! forceStart)) && (i + pl == sl || 
                     (! isId(symName.at(i + pl).unicode()) && ! forceEnd)) ) {               
               return true;
            }
         }
      }      
   }
   
   return false;
}

void Definition::addToMap(const QString &name)
{    
   QString phrase = name;
   int index = computeQualifiedIndex(phrase);

   if (index != -1) {
      phrase = phrase.mid(index + 2);
   }

   if (! phrase.isEmpty()) { 
      // must use a raw pointer since this method is called from a constructor                     
      Doxy_Globals::glossary().insertMulti(phrase, this);         
      this->setPhraseName(phrase);
   }
}

Definition::Definition(const QString &df, int dl, int dc, const QString &name, const QString &briefDoc, 
                  const QString &fullDoc, bool isPhrase)
{
   m_name      = name;
   m_defLine   = dl;
   m_defColumn = dc;

   m_private = new Definition_Private;
   m_private->init(df, name);

   m_isPhrase = isPhrase;

   if (isPhrase) {
      addToMap(name);
   }

   // these ids are used in ftvhelp
   m_inputOrderId = -1;
   m_sortId = -1;

   setBriefDescription(briefDoc, df, dl);
   setDocumentation(fullDoc, df, dl, true, false);

   if (matchExcludedSymbols(name)) {
      m_private->hidden = true;
   }
}

Definition::Definition(const Definition &d) 
   : DefinitionIntf()
{
   m_name    = d.m_name;
   m_defLine = d.m_defLine;

   m_private  = new Definition_Private;
   *m_private = *d.m_private;

   m_private->sectionDict     = 0;
   m_private->sourceRefByDict = 0;
   m_private->sourceRefsDict  = 0;
   m_private->partOfGroups    = 0;
   m_private->xrefListItems   = 0;
   m_private->brief           = 0;
   m_private->details         = 0;
   m_private->body            = 0;
   m_private->inbodyDocs      = 0;

   if (d.m_private->sectionDict) {
      m_private->sectionDict = new SectionDict();
    
      for (auto si : *d.m_private->sectionDict) {  
         m_private->sectionDict->insert(si->label, si);
      }
   }

   m_private->sectionList = d.m_private->sectionList;

   if (d.m_private->sourceRefByDict) {
      m_private->sourceRefByDict = new MemberSDict;
      
      for (auto iter = d.m_private->sourceRefByDict->begin();  iter != d.m_private->sourceRefByDict->end(); ++iter) {   
         m_private->sourceRefByDict->insert(iter.key(), iter.value());
      }
   }

   if (d.m_private->sourceRefsDict) {
      m_private->sourceRefsDict = new MemberSDict;
    
      for (auto iter= d.m_private->sourceRefsDict->begin(); iter != d.m_private->sourceRefsDict->end(); ++iter) {   
         m_private->sourceRefsDict->insert(iter.key(), iter.value());
      }
   }
   if (d.m_private->partOfGroups) {     
      for (auto gd : *d.m_private->partOfGroups) {   
         makePartOfGroup(gd);
      }
   }

   if (d.m_private->xrefListItems) {
      setRefItems(d.m_private->xrefListItems);
   }

   if (d.m_private->brief) {
      m_private->brief = new BriefInfo(*d.m_private->brief);
   }

   if (d.m_private->details) {
      m_private->details = new DocInfo(*d.m_private->details);
   }

   if (d.m_private->body) {
      m_private->body = new BodyInfo(*d.m_private->body);
   }

   if (d.m_private->inbodyDocs) {
      m_private->inbodyDocs = new DocInfo(*d.m_private->inbodyDocs);
   }

   m_isPhrase = d.m_isPhrase;
   if (m_isPhrase) {
      addToMap(m_name);
   }
}

Definition::~Definition()
{  
   if (m_private) {
      delete m_private;
      m_private = 0;
   }

   if (! Doxy_Globals::programExit)  {
      auto iter = Doxy_Globals::glossary().find(m_phraseName);
   
      while (iter != Doxy_Globals::glossary().end() && iter.key() == m_phraseName)  {      
   
         if (iter.value() == this) {
            iter = Doxy_Globals::glossary().erase(iter);
   
         }  else {
            ++iter;
   
         }      
      }
   }
}

void Definition::setName(const QString &name)
{
   if (name.isEmpty()) {
      return;
   }

   m_name = name;
}

void Definition::setId(const QString &id)
{
   QSharedPointer<Definition> self = sharedFrom(this);

   if (id.isEmpty()) {
      return;
   }
  
   m_private->id = id; 
   Doxy_Globals::clangUsrMap.insert(id, self);
}

QString Definition::id() const
{
   return m_private->id;
}

void Definition::addSectionsToDefinition(QList<SectionInfo> *anchorList)
{
   QSharedPointer<Definition> self = sharedFrom(this);

   if (! anchorList) {
      return;
   }

   for (auto &si : *anchorList) {       
      QSharedPointer<SectionInfo> gsi (Doxy_Globals::sectionDict->find(si.label));
      
      if (! gsi) {
         gsi = QMakeShared<SectionInfo>(si);
         Doxy_Globals::sectionDict->insert(si.label, gsi);
      }

      if (m_private->sectionDict == 0) {
         m_private->sectionDict = new SectionDict();
      }

      if (m_private->sectionDict->find(gsi->label) == 0) {
         m_private->sectionDict->insert(gsi->label, gsi);
         gsi->definition = self;

         m_private->sectionList.append(gsi);
      }
   }
}

bool Definition::hasSections() const
{ 
   if (m_private->sectionDict == 0) {
      return false;
   }
  
   for (auto si : *m_private->sectionDict) {   
      if (si->type == SectionInfo::Section || si->type == SectionInfo::Subsection ||
            si->type == SectionInfo::Subsubsection || si->type == SectionInfo::Paragraph) {
         return true;
      }
   }

   return false;
}

void Definition::addSectionsToIndex(bool addToNavIndex)
{ 
   int level = 1;

   auto nextItem = m_private->sectionList.begin();

   // by definition order  
   for (auto si : m_private->sectionList) {   

      if (si->type == SectionInfo::Section || si->type == SectionInfo::Subsection ||
            si->type == SectionInfo::Subsubsection || si->type == SectionInfo::Paragraph) {
    
         int nextLevel = si->type;
         int i;

         if (nextLevel > level) {
            for (i = level; i < nextLevel; i++) {
               Doxy_Globals::indexList->incContentsDepth();
            }

         } else if (nextLevel < level) {
            for (i = nextLevel; i < level; i++) {
               Doxy_Globals::indexList->decContentsDepth();
            }
         }

         QString title = si->title;

         if (title.isEmpty()) {
            title = si->label;
         }

         // determine if there is a next level inside this item
         ++nextItem;
         bool isDir = false;

         if (nextItem != m_private->sectionList.end())  {
            isDir = (*nextItem)->type > nextLevel;
         }
      
         Doxy_Globals::indexList->addContentsItem(isDir, title, getReference(), getOutputFileBase(), si->label, addToNavIndex);
         level = nextLevel;
      }
   }

   while (level > 1) {
      Doxy_Globals::indexList->decContentsDepth();
      level--;
   }
}

void Definition::writeDocAnchorsToTagFile(QTextStream &tagFile)
{   
    // by definition order  
   for (auto si : m_private->sectionList) {   

      if (! si->generated && si->ref.isEmpty()) {
        
         if (definitionType() == TypeMember) {
            tagFile << "  ";
         }

         tagFile << "    <docanchor file=\"" << si->fileName << "\"";

         if (!si->title.isEmpty()) {
            tagFile << " title=\"" << convertToXML(si->title) << "\"";
         }

         tagFile << ">" << si->label << "</docanchor>" << endl;
      }
   }
   
}

bool Definition::_docsAlreadyAdded(const QString &doc, QString &sigList)
{  
   // to avoid mismatches due to differences in indenting, we first remove
   // double whitespaces...

   QString docStr = doc.simplified();

   QString sigStr;
   sigStr = QCryptographicHash::hash(docStr.toUtf8(), QCryptographicHash::Md5).toHex();  

   if (sigList.indexOf(sigStr) == -1) { 
      // new docs, add signature to prevent re-adding it
      sigList += ":" + sigStr;
      return false;

   } else {
      return true;

   }
}

void Definition::setDocumentation(const QString &tDoc, const QString &docFile, int docLine, 
                                   bool stripWhiteSpace, bool atTop)
{
   QString doc = tDoc;

   if (doc.isEmpty()) {
      return;
   }

   if (stripWhiteSpace) {
      doc = trimEmptyLines(doc, docLine);
   }

   if (! _docsAlreadyAdded(doc, m_private->docSignatures)) {
     
      if (m_private->details == nullptr) {
         m_private->details = new DocInfo;
      }

      if (m_private->details->doc.isEmpty()) { 
         // fresh detailed description
         m_private->details->doc = doc;
       
      } else if (atTop) { 
         // another detailed description, append it to the start
         m_private->details->doc = doc + "\n\n" + m_private->details->doc;

      } else { 
         // another detailed description, append it to the end
         m_private->details->doc += "\n\n" + doc;

      }

      if (docLine != -1) { 
         // store location if valid
         m_private->details->file = docFile;
         m_private->details->line = docLine;

      } else {
         m_private->details->file = docFile;
         m_private->details->line = 1;
      }
   }
}

void Definition::setBriefDescription(const QString &b, const QString &briefFile, int briefLine)
{
   static QString outputLanguage = Config::getEnum("output-language");
   static bool needsDot = (outputLanguage != "Japanese" && outputLanguage != "Chinese" && outputLanguage != "Korean");

   QString brief = b.trimmed();
  
   if (brief.isEmpty()) {
      return;
   }
   
   if (needsDot) { 
      // add punctuation if needed

      int len = brief.length();
      int c   = brief.at(len - 1).unicode();

      switch (c) {
         case '.':
         case '!':
         case '?':
         case '>':
         case ':':
         case ')':
            break;

         default:
            // may need to test for multi-byte char at end
            if ( brief.at(0).isUpper()) {
               brief += '.';
            }
            break;
      }
   }

   if (! _docsAlreadyAdded(brief, m_private->briefSignatures)) {

      if (m_private->brief && ! m_private->brief->doc.isEmpty()) {         
         setDocumentation(brief, briefFile, briefLine, false, true);

      } else {
         
         if (m_private->brief == 0) {
            m_private->brief = new BriefInfo;
         }

         m_private->brief->doc = brief;

         if (briefLine != -1) {
            m_private->brief->file = briefFile;
            m_private->brief->line = briefLine;
         } else {
            m_private->brief->file = briefFile;
            m_private->brief->line = 1;
         }
      }  
   }
}

void Definition::_setInbodyDocumentation(const QString &doc, const QString &inbodyFile, int inbodyLine)
{
   if (m_private->inbodyDocs == nullptr) {
      m_private->inbodyDocs = new DocInfo;
   }

   if (m_private->inbodyDocs->doc.isEmpty()) { 
      // fresh inbody docs
      m_private->inbodyDocs->doc  = doc;
      m_private->inbodyDocs->file = inbodyFile;
      m_private->inbodyDocs->line = inbodyLine;

   } else { 
      // another inbody documentation fragment, append this to the end
      m_private->inbodyDocs->doc += "\n\n" + doc;
   }
}

void Definition::setInbodyDocumentation(const QString &d, const QString &inbodyFile, int inbodyLine)
{
   if (d.isEmpty()) {
      return;
   }

   _setInbodyDocumentation(d, inbodyFile, inbodyLine);
}


/*! Reads a fragment of code from file \a fileName starting at
 * line \a startLine and ending at line \a endLine (inclusive). The fragment is
 * stored in \a result. If false is returned the code fragment could not be found
 *
 * The file is scanned for a opening bracket ('{') from \a startLine onward
 * The line actually containing the bracket is returned via startLine.
 * The file is scanned for a closing bracket ('}') from \a endLine backward.
 * The line actually containing the bracket is returned via endLine.
 */
static bool readCodeFragment(const QString &fileName, int &startLine, int &endLine, QString &result)
{
   static bool filterSourceFiles = Config::getBool("filter-source-files");
   static int tabSize = Config::getInt("tab-size");
   
   if (fileName.isEmpty()) {
      return false;   // not a valid file name
   }

   QByteArray tempResult;

   QString filter = getFileFilter(fileName, true);

   FILE *f = 0;
   bool usePipe = ! filter.isEmpty() && filterSourceFiles;

   SrcLangExt lang = getLanguageFromFileName(fileName);

   if (! usePipe) { 
      // no filter given or wanted
      f = fopen(fileName.toUtf8(), "r");

   } else { 
      // use filter
      QString cmd = filter + " \"" + fileName + "\"";    
      f = popen(cmd.toUtf8(), "r");
   }

   // for TCL, Python, and Fortran no bracket search is possible
   bool found = (lang == SrcLangExt_Tcl) || (lang == SrcLangExt_Python) || (lang == SrcLangExt_Fortran);  

   if (f) {
      int c      = 0;
      int col    = 0;
      int lineNr = 1;

      // skip until the startLine has reached

      while (lineNr < startLine && !feof(f)) {
         while ((c = fgetc(f)) != '\n' && c != EOF) /* skip */;
         lineNr++;

         if (found && c == '\n') {
            c = '\0';
         }
      }

      if (! feof(f)) {
         // skip until the opening bracket or lonely : is found
         char cn = 0;

         while (lineNr <= endLine && ! feof(f) && ! found) {
            int pc = 0;

            while ((c = fgetc(f)) != '{' && c != ':' && c != EOF) {

               if (c == '\n') {
                  lineNr++;
                  col = 0;

               } else if (c == '\t') {
                  col += tabSize - (col % tabSize);

               } else if (pc == '/' && c == '/') { 
                  // skip single line comment

                  while ((c = fgetc(f)) != '\n' && c != EOF) {
                     pc = c;
                  }

                  if (c == '\n') {
                     lineNr++, col = 0;
                  }

               } else if (pc == '/' && c == '*') { 
                  // skip C style comment

                  while (((c = fgetc(f)) != '/' || pc != '*') && c != EOF) {
                     if (c == '\n') {
                        lineNr++;
                        col = 0;
                     }

                     pc = c;
                  }

               } else {
                  col++;
               }
               pc = c;
            }

            if (c == ':') {
               cn = fgetc(f);
               if (cn != ':') {
                  found = true;
               }

            } else if (c == '{') { 
               // } so vi matching brackets has no problem
               found = true;
            }
         }
         
         if (found) {
            // For code with more than one line, fill the line with spaces until we are 
            // at the right column so that the opening brace lines up with the closing brace

            if (endLine != startLine) {
               QString spaces;
               spaces.fill(' ', col);

               tempResult += spaces;
            }

            // copy until end of line            
            if (c) {
               tempResult += c;
            }

            startLine = lineNr;

            if (c == ':') {
               tempResult += cn;

               if (cn == '\n') {
                  lineNr++;
               }
            }

            const int maxLineLength = 4096;
            char lineStr[maxLineLength];

            do {               
               int size_read;

               do {
                  // read up to maxLineLength-1 bytes, the last byte being zero
                  char *p = fgets(lineStr, maxLineLength, f);
                 
                  if (p) {
                     size_read = qstrlen(p);

                  } else { // nothing read
                     size_read = -1;
                     lineStr[0] = '\0';
                  }
                  tempResult += lineStr;
               } while (size_read == (maxLineLength - 1));

               lineNr++;

            } while (lineNr <= endLine && ! feof(f));

            // strip stuff after closing bracket
            int newLineIndex = tempResult.lastIndexOf('\n');
            int braceIndex   = tempResult.lastIndexOf('}');

            if (braceIndex > newLineIndex) {
               tempResult.truncate(braceIndex + 1);
            }

            endLine = lineNr - 1;
         }
      }

      if (usePipe) {
         pclose(f);
         Debug::print(Debug::FilterOutput, 0, "Filter output\n");
         Debug::print(Debug::FilterOutput, 0, "-------------\n%s\n-------------\n", qPrintable(tempResult));

      } else {
         fclose(f);
      }
   }

   result = transcodeToQString(tempResult);

   if (! result.isEmpty() && ! result.endsWith('\n')) {
      result += "\n";
   }
   
   return found;
}

QString Definition::getSourceFileBase() const
{
   assert(definitionType() != Definition::TypeFile); // file overloads this method

   QString fn;
   static bool sourceBrowser = Config::getBool("source-code");

   if (sourceBrowser && m_private->body && m_private->body->startLine != -1 && m_private->body->fileDef) {
      fn = m_private->body->fileDef->getSourceFileBase();
   }

   return fn;
}

QString Definition::getSourceAnchor() const
{
   const int maxAnchorStrLen = 20;

   char anchorStr[maxAnchorStrLen];
   anchorStr[0] = '\0';

   if (m_private->body && m_private->body->startLine != -1) {
      if (Htags::useHtags) {
         qsnprintf(anchorStr, maxAnchorStrLen, "L%d", m_private->body->startLine);
      } else {
         qsnprintf(anchorStr, maxAnchorStrLen, "l%05d", m_private->body->startLine);
      }
   }

   return anchorStr;
}

/*! Write a reference to the source code defining this definition */
void Definition::writeSourceDef(OutputList &ol, const QString &)
{
   static bool latexSourceCode = Config::getBool("latex-source-code");
   static bool rtfSourceCode   = Config::getBool("rtf-source-code");

   ol.pushGeneratorState();
  
   QString fn = getSourceFileBase();

   if (! fn.isEmpty()) {
      QString refText = theTranslator->trDefinedAtLineInSourceFile();

      int lineMarkerPos = refText.indexOf("@0");
      int fileMarkerPos = refText.indexOf("@1");

      if (lineMarkerPos != -1 && fileMarkerPos != -1) { 
         // should always pass this.
         QString lineStr;
         lineStr = QString("%1").arg(m_private->body->startLine);

         QString anchorStr = getSourceAnchor();
         ol.startParagraph();

         if (lineMarkerPos < fileMarkerPos) { // line marker before file marker
            // write text left from linePos marker
            ol.parseText(refText.left(lineMarkerPos));
            ol.pushGeneratorState();

            ol.disable(OutputGenerator::Man);

            if (! latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            if (! rtfSourceCode) {
               ol.disable(OutputGenerator::RTF);
            }

            // write line link (HTML, LaTeX optionally, RTF optionally)
            ol.writeObjectLink(0, fn, anchorStr, lineStr);
            ol.enableAll();

            ol.disable(OutputGenerator::Html);

            if (latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            if (rtfSourceCode) {
               ol.disable(OutputGenerator::RTF);
            }

            // write normal text (Man/RTF, Latex optionally)
            ol.docify(lineStr);
            ol.popGeneratorState();

            // write text between markers
            ol.parseText(refText.mid(lineMarkerPos + 2, fileMarkerPos - lineMarkerPos - 2));

            ol.pushGeneratorState();

            ol.disable(OutputGenerator::Man);

            if (! latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            if (! rtfSourceCode) {
               ol.disable(OutputGenerator::RTF);
            }

            // write file link (HTML, LaTeX optionally, RTF optionally)
            ol.writeObjectLink(0, fn, 0, m_private->body->fileDef->name());
            ol.enableAll();

            ol.disable(OutputGenerator::Html);
            if (latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            if (rtfSourceCode) {
               ol.disable(OutputGenerator::RTF);
            }

            // write normal text (Man/RTF, Latex optionally)
            ol.docify(m_private->body->fileDef->name());
            ol.popGeneratorState();

            // write text right from file marker
            ol.parseText(refText.right(refText.length() - fileMarkerPos - 2));

         } else { // file marker before line marker
            // write text left from file marker

            ol.parseText(refText.left(fileMarkerPos));
            ol.pushGeneratorState();

            ol.disable(OutputGenerator::Man);

            if (! latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            if (! rtfSourceCode) {
               ol.disable(OutputGenerator::RTF);
            }

            // write file link (HTML only)
            ol.writeObjectLink(0, fn, 0, m_private->body->fileDef->name());

            ol.enableAll();

            ol.disable(OutputGenerator::Html);

            if (latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            if (rtfSourceCode) {
               ol.disable(OutputGenerator::RTF);
            }

            // write normal text (RTF/Latex/Man only)
            ol.docify(m_private->body->fileDef->name());
            ol.popGeneratorState();

            // write text between markers
            ol.parseText(refText.mid(fileMarkerPos + 2, lineMarkerPos - fileMarkerPos - 2));

            ol.pushGeneratorState();
           
            ol.disable(OutputGenerator::Man);

            if (! latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            if (! rtfSourceCode) {
               ol.disable(OutputGenerator::RTF);
            }

            ol.disableAllBut(OutputGenerator::Html);

            // write line link (HTML only)
            ol.writeObjectLink(0, fn, anchorStr, lineStr);
            ol.enableAll();

            ol.disable(OutputGenerator::Html);

            if (latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            if (rtfSourceCode) {
               ol.disable(OutputGenerator::RTF);
            }

            // write normal text (Latex/Man only)
            ol.docify(lineStr);
            ol.popGeneratorState();

            // write text right from linePos marker
            ol.parseText(refText.right(refText.length() - lineMarkerPos - 2));
         }

         ol.endParagraph();

      } else {
         err("Invalid translation markers in 'trDefinedAtLineInSourceFile'\n");
      }
   }

   ol.popGeneratorState();
}

void Definition::setBodySegment(int bls, int ble)
{   
   if (m_private->body == 0) {
      m_private->body = new BodyInfo;
   }

   m_private->body->startLine = bls;
   m_private->body->endLine = ble;
}

void Definition::setBodyDef(QSharedPointer<FileDef> fd)
{
   if (m_private->body == 0) {
      m_private->body = new BodyInfo;
   }
   m_private->body->fileDef = fd;
}

bool Definition::hasSources() const
{
   return m_private->body && m_private->body->startLine != -1 &&
          m_private->body->endLine >= m_private->body->startLine && m_private->body->fileDef;
}

/*! Write code of this definition into the documentation */
void Definition::writeInlineCode(OutputList &ol, const QString &scopeName)
{
   QSharedPointer<Definition> self = sharedFrom(this);

   static bool inlineSources = Config::getBool("inline-source");
   ol.pushGeneratorState();
   
   if (inlineSources && hasSources()) {
      QString codeFragment;

      int actualStart = m_private->body->startLine, actualEnd = m_private->body->endLine;

      if (readCodeFragment(m_private->body->fileDef->getFilePath(), actualStart, actualEnd, codeFragment) ) {

         ParserInterface *pIntf = Doxy_Globals::parserManager->getParser(m_private->defFileExt);
         pIntf->resetCodeParserState();
        
         QSharedPointer<MemberDef> thisMd;

         if (definitionType() == TypeMember) {
            thisMd = self.dynamicCast<MemberDef>();
         }

         ol.startCodeFragment();
         pIntf->parseCode(ol,               // codeOutIntf
                          scopeName,        // scope
                          codeFragment,     // input
                          m_private->lang,     // lang
                          false,            // isExample
                          nullptr,          // exampleName
                          m_private->body->fileDef,  // fileDef
                          actualStart,      // startLine
                          actualEnd,        // endLine
                          true,             // inlineFragment
                          thisMd,           // memberDef
                          true              // show line numbers
                         );

         ol.endCodeFragment();
      }
   }
   ol.popGeneratorState();
}

/*! Write a reference to the source code fragments in which this
 *  definition is used.
 */
void Definition::_writeSourceRefList(OutputList &ol, const QString &scopeName,
                                     const QString &text, MemberSDict *members, bool)
{
   static const bool sourceBrowser   = Config::getBool("source-code");
   static const bool refLinkSource   = Config::getBool("ref-link-source");
   static const bool latexSourceCode = Config::getBool("latex-source-code");
   static const bool rtfSourceCode   = Config::getBool("rtf-source-code");  
  
   ol.pushGeneratorState();

   if (members) {      

      ol.startParagraph();
      ol.parseText(text);
      ol.docify(" ");

      QString ldefLine = theTranslator->trWriteList(members->count());

      QRegExp marker("@[0-9]+");
      int index = 0, newIndex, matchLen;

      auto iter = members->begin();

      // now replace all markers in inheritLine with links to the classes
      while ((newIndex = marker.indexIn(ldefLine, index)) != -1) {

         matchLen = marker.matchedLength();
      
         ol.parseText(ldefLine.mid(index, newIndex - index));        

         QSharedPointer<MemberDef> md = iter.value();

         if (md) {
            QString scope = md->getScopeString();
            QString name  = md->name();
           
            if (! scope.isEmpty() && scope != scopeName) {
               name.prepend(scope + getLanguageSpecificSeparator(m_private->lang));
            }

            if (!md->isObjCMethod() && (md->isFunction() || md->isSlot() || md->isPrototype() || md->isSignal() )) {
               name += "()";
            }
        
            if (sourceBrowser && !(md->isLinkable() && !refLinkSource) && md->getStartBodyLine() != -1 && md->getBodyDef()) {              
               // for HTML write a real link

               ol.pushGeneratorState();
               //ol.disableAllBut(OutputGenerator::Html);

               ol.disable(OutputGenerator::RTF);
               ol.disable(OutputGenerator::Man);

               if (! latexSourceCode) {
                  ol.disable(OutputGenerator::Latex);
               }

               if (! rtfSourceCode) {
                  ol.disable(OutputGenerator::RTF);
               }

               const int maxLineNrStr = 10;
               char anchorStr[maxLineNrStr];
               qsnprintf(anchorStr, maxLineNrStr, "l%05d", md->getStartBodyLine());
             
               ol.writeObjectLink(0, md->getBodyDef()->getSourceFileBase(), anchorStr, name);
               ol.popGeneratorState();

               // for the other output formats just mention the name
               ol.pushGeneratorState();
               ol.disable(OutputGenerator::Html);

               if (latexSourceCode) {
                  ol.disable(OutputGenerator::Latex);
               }

               if (rtfSourceCode) {
                  ol.disable(OutputGenerator::RTF);
               }

               ol.docify(name);
               ol.popGeneratorState();

            } else if (md->isLinkable() /*&& d && d->isLinkable()*/) {
               // for HTML write a real link
               ol.pushGeneratorState();

               //ol.disableAllBut(OutputGenerator::Html);              
               ol.disable(OutputGenerator::Man);

               if (! latexSourceCode) {
                  ol.disable(OutputGenerator::Latex);
               }

               if (! rtfSourceCode) {
                  ol.disable(OutputGenerator::RTF);
               }

               ol.writeObjectLink(md->getReference(), md->getOutputFileBase(), md->anchor(), name);
               ol.popGeneratorState();

               // for the other output formats just mention the name
               ol.pushGeneratorState();
               ol.disable(OutputGenerator::Html);

               if (latexSourceCode) {
                  ol.disable(OutputGenerator::Latex);
               }

               if (rtfSourceCode) {
                  ol.disable(OutputGenerator::RTF);
               }

               ol.docify(name);
               ol.popGeneratorState();

            } else {
               ol.docify(name);
            }
         }

         index = newIndex + matchLen;
      }
      ol.parseText(ldefLine.right(ldefLine.length() - index));
      ol.writeString(".");
      ol.endParagraph();
   }

   ol.popGeneratorState();
}

void Definition::writeSourceReffedBy(OutputList &ol, const QString &scopeName)
{
   if (Config::getBool("ref-by-relation")) {
      _writeSourceRefList(ol, scopeName, theTranslator->trReferencedBy(), m_private->sourceRefByDict, false);
   }
}

void Definition::writeSourceRefs(OutputList &ol, const QString &scopeName)
{
   if (Config::getBool("ref-relation")) {
      _writeSourceRefList(ol, scopeName, theTranslator->trReferences(), m_private->sourceRefsDict, true);
   }
}

bool Definition::hasDocumentation() const
{
   static bool extractAll = Config::getBool("extract-all");
   // static bool sourceBrowser = Config::getBool("source-code");

   bool hasDocs = (m_private->details    && ! m_private->details->doc.isEmpty())    || 
                  (m_private->brief      && ! m_private->brief->doc.isEmpty())      ||
                  (m_private->inbodyDocs && ! m_private->inbodyDocs->doc.isEmpty()) || extractAll; 

      //  ||   // extract everything
      //       (sourceBrowser && m_private->body && m_private->body->startLine!=-1 && m_private->body->fileDef)
    
   return hasDocs;
}

bool Definition::hasUserDocumentation() const
{
   bool hasDocs = (m_private->details    && ! m_private->details->doc.isEmpty()) ||
                  (m_private->brief      && ! m_private->brief->doc.isEmpty())   ||
                  (m_private->inbodyDocs && ! m_private->inbodyDocs->doc.isEmpty());

   return hasDocs;
}

void Definition::addSourceReferencedBy(QSharedPointer<MemberDef> md)
{
   if (md) {
      QString name  = md->name();
      QString scope = md->getScopeString();

      if (!scope.isEmpty()) {
         name.prepend(scope + "::");
      }

      if (m_private->sourceRefByDict == 0) {
         m_private->sourceRefByDict = new MemberSDict;
      }

      if (m_private->sourceRefByDict->find(name) == 0) {
         m_private->sourceRefByDict->insert(name, md);
      }
   }
}

void Definition::addSourceReferences(QSharedPointer<MemberDef> md)
{
   QString name  = md->name();
   QString scope = md->getScopeString();

   if (md) {
      QString name  = md->name();
      QString scope = md->getScopeString();

      if (!scope.isEmpty()) {
         name.prepend(scope + "::");
      }

      if (m_private->sourceRefsDict == 0) {
         m_private->sourceRefsDict = new MemberSDict;
      }

      if (m_private->sourceRefsDict->find(name) == 0) {
         m_private->sourceRefsDict->insert(name, md);
      }
   }
}

QString Definition::qualifiedName() const
{
   if (! m_private->qualifiedName.isEmpty()) {     
      return m_private->qualifiedName;
   }
   
   if (! m_private->outerScope) {
      if (m_private->localName == "<globalScope>") {       
         return "";

      } else {         
         return m_private->localName;
      }
   }

   if (m_private->outerScope->name() == "<globalScope>") {
      m_private->qualifiedName = m_private->localName;

   } else {
      m_private->qualifiedName = m_private->outerScope->qualifiedName() + getLanguageSpecificSeparator(getLanguage()) + m_private->localName;
   }
   
   return m_private->qualifiedName;
}

void Definition::setOuterScope(QSharedPointer<Definition> d)
{  
   QSharedPointer<Definition> p = m_private->outerScope;
   bool found = false;

   // make sure we are not creating a recursive scope relation
   while (p && ! found) {
      found = (p == d);
      p = p->m_private->outerScope;
   }

   if (! found) {
      m_private->qualifiedName.resize(0); // flush cached scope name
      m_private->outerScope = d;
   }

   m_private->hidden = m_private->hidden || d->isHidden();
}

QString Definition::localName() const
{
   return m_private->localName;
}

void Definition::makePartOfGroup(QSharedPointer<GroupDef> gd)
{
   if (m_private->partOfGroups == 0) {
      m_private->partOfGroups = new SortedList<QSharedPointer<GroupDef>>;
   }

   m_private->partOfGroups->append(gd);
}

void Definition::setRefItems(const QList<ListItemInfo> *sli)
{
   if (sli) {
      // deep copy the list
      if (m_private->xrefListItems == 0) {
         m_private->xrefListItems = new QList<ListItemInfo>;        
      }
   
      for (auto item : *sli) {
         m_private->xrefListItems->append(item);
      }
   }
}

void Definition::mergeRefItems(QSharedPointer<Definition> d)
{
   QList<ListItemInfo> *xrefList = d->xrefListItems();

   if (xrefList != 0) {
      // deep copy the list
      if (m_private->xrefListItems == 0) {
         m_private->xrefListItems = new QList<ListItemInfo>;         
      }
    
      for (auto item : *xrefList) {  
         if (_getXRefListId(item.type) == -1) {
            m_private->xrefListItems->append(item);
         }
      }
   }
}

int Definition::_getXRefListId(const QString &listName) const
{
   if (m_private->xrefListItems) {     
      for (auto &item : *m_private->xrefListItems) {  
         if (item.type == listName) {
            return item.itemId;
         }
      }
   }

   return -1;
}

QList<ListItemInfo> *Definition::xrefListItems() const
{
   return m_private->xrefListItems;
}

QString Definition::convertNameToFile(const QString &name, bool allowDots) const
{
   if (! m_private->ref.isEmpty()) {
      return name;

   } else {
      return ::convertNameToFile(name, allowDots);

   }
}

QString Definition::pathFragment() const
{
   QString result;

   if (m_private->outerScope && m_private->outerScope != Doxy_Globals::globalScope) {
      result = m_private->outerScope->pathFragment();
   }

   if (isLinkable()) {
      if (! result.isEmpty()) {
         result += "/";
      }

      result += pathFragment_Internal();

   } else {
      result += m_private->localName;
   }

   return result;
}

QString Definition::pathFragment_Internal() const
{
   return m_private->localName;
}

// TODO: move to htmlgen
/*! Returns the string used in the footer for $navpath when
 *  GENERATE_TREEVIEW is enabled
 */
QString Definition::navigationPathAsString() const
{
   QString result;
   QSharedPointer<Definition> outerScope = getOuterScope();

   QString tName = localName();

   if (outerScope && outerScope != Doxy_Globals::globalScope) {
      result += outerScope->navigationPathAsString();

   } else if (definitionType() == Definition::TypeFile && ((const FileDef *)this)->getDirDef()) {
      result += ((const FileDef *)this)->getDirDef()->navigationPathAsString();

   }

   result += "<li class=\"navelem\">";

   if (isLinkable()) {

      auto gd = dynamic_cast<const GroupDef *>(this);
      auto pd = dynamic_cast<const PageDef *>(this);

      if (definitionType() == Definition::TypeGroup && ! gd->groupTitle().isEmpty() ) {
    
         result += "<a class=\"el\" href=\"$relpath^" + getOutputFileBase() + Doxy_Globals::htmlFileExtension + "\">" +
                   convertToHtml(gd->groupTitle()) + "</a>";

      } else if (definitionType() == Definition::TypePage && ! pd->title().isEmpty() ) {

         result += "<a class=\"el\" href=\"$relpath^" + getOutputFileBase() + Doxy_Globals::htmlFileExtension + "\">" +
                   convertToHtml(pd->title()) + "</a>";

      } else if (definitionType() == Definition::TypeClass) {
         // class         
    
         if (tName.right(2) == "-p") {
            tName = tName.left(tName.length() - 2);
         }

         result += "<a class=\"el\" href=\"$relpath^" + getOutputFileBase() + Doxy_Globals::htmlFileExtension;
         if (! anchor().isEmpty()) {
            result += "#" + anchor();
         }

         result+="\">" + convertToHtml(tName) + "</a>";

      } else {
         // namespace or something else

         // added 01/2016
         tName = renameNS_Aliases(tName);

         result += "<a class=\"el\" href=\"$relpath^" + getOutputFileBase() + Doxy_Globals::htmlFileExtension + "\">" +
                   convertToHtml(tName) + "</a>";
      }

   } else {
      result+="<b>" + convertToHtml(tName) + "</b>";

   }

   result += "</li>";

   return result;
}

// TODO: move to htmlgen
void Definition::writeNavigationPath(OutputList &ol) const
{
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   QString navPath;
   navPath += "<div id=\"breadcrumb\" class=\"breadcrumb\">\n"
              "  <ul>\n";

   navPath += navigationPathAsString();
   navPath += "  </ul>\n"
              "</div>\n";

   ol.writeNavigationPath(navPath);

   ol.popGeneratorState();
}

// TODO: move to htmlgen
void Definition::writeToc(OutputList &ol)
{  
   if (m_private->sectionList.isEmpty()) {
      return;
   }

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
   ol.writeString("<div class=\"toc\">");
   ol.writeString("<h3>");
   ol.writeString(theTranslator->trRTFTableOfContents());
   ol.writeString("</h3>\n");
   ol.writeString("<ul>");
  
   int level = 1;   
   QString cs;

   bool inLi[5] = { false, false, false, false };   

   // by definition order  
   for (auto si : m_private->sectionList) {   

      if (si->type == SectionInfo::Section || si->type == SectionInfo::Subsection    ||
            si->type == SectionInfo::Subsubsection || si->type == SectionInfo::Paragraph) {
        
         int nextLevel = si->type;

         if (nextLevel > level) {
            for (int l = level; l < nextLevel; l++) {
               ol.writeString("<ul>");
            }

         } else if (nextLevel < level) {
            for (int l = level; l > nextLevel; l--) {
               if (inLi[l]) {
                  ol.writeString("</li>\n");
               }

               inLi[l] = false;
               ol.writeString("</ul>\n");
            }
         }

         cs = QString::number(nextLevel);

         if (inLi[nextLevel]) {
            ol.writeString("</li>\n");
         }
     
         QString titleDoc = convertToHtml(si->title);
         ol.writeString("<li class=\"level" + cs + "\"><a href=\"#" + si->label + "\">" + (si->title.isEmpty() ? si->label : titleDoc) + "</a>");
   
         inLi[nextLevel] = true;
         level = nextLevel;
      }
   }

   while (level > 1) {
      if (inLi[level]) {
         ol.writeString("</li>\n");
      }

      inLi[level] = false;
      ol.writeString("</ul>\n");
      level--;
   }

   if (inLi[level]) {
      ol.writeString("</li>\n");
   }

   inLi[level] = false;
   ol.writeString("</ul>\n");
   ol.writeString("</div>\n");
   ol.popGeneratorState();
}

QString Definition::phraseName() const
{
   return m_phraseName;
}

QString Definition::documentation() const
{
   if (m_private->details) {
      return m_private->details->doc;
   }

   return "";
}

int Definition::docLine() const
{
   return m_private->details ? m_private->details->line : 1;
}

QString Definition::docFile() const
{
   return m_private->details ? m_private->details->file : "<" + m_name + ">";
}

// strips w from s if s starts with w
static bool stripWord(QString &s, QString w)
{
   bool success = false;

   if (s.left(w.length()) == w) {
      success = true;
      s = s.right(s.length() - w.length());
   }

   return success;
}

// some quasi intelligent brief description abbreviator :^)
QString abbreviate(const QString &s, const QString &name)
{
   QString scopelessName = name;
   int i = scopelessName.lastIndexOf("::");

   if (i != -1) {
      scopelessName = scopelessName.mid(i + 2);
   }

   QString result = s;
   result = result.trimmed();

   // strip trailing .
   if (! result.isEmpty() && result.at(result.length() - 1) == '.') {
      result = result.left(result.length() - 1);
   }

   // strip any predefined prefix
   const QStringList briefDescAbbrev = Config::getList("abbreviate-brief");
  
   for (auto s : briefDescAbbrev) {     
      s.replace(QRegExp("\\$name"), scopelessName);     
      
      // replace $name with entity name
      s += " ";

      stripWord(result, s);      
   }

   // capitalize first word
   if (! result.isEmpty()) {  
      result[0] = result[0].toUpper();
   }

   return result;
}

QString Definition::briefDescription(bool abbr) const
{
   QString retval = ""; 

   if (m_private->brief)  {

      if (abbr) {
         retval = abbreviate(m_private->brief->doc, displayName());

      } else {
         retval = m_private->brief->doc; 

      } 
   }

   return retval;
}

QString Definition::briefDescriptionAsTooltip() const
{
   QSharedPointer<const Definition> self = sharedFrom(this);

   if (m_private->brief) {

      if (m_private->brief->tooltip.isEmpty() && ! m_private->brief->doc.isEmpty()) {
         static bool reentering = false;

         if (! reentering) {
            QSharedPointer<const MemberDef> md;

            if (definitionType() == TypeMember) {
               md = self.dynamicCast< const MemberDef>();
            } else {
               md = QSharedPointer<MemberDef>();
            }

            QSharedPointer<const Definition> scope;

            if (definitionType() == TypeMember) {
               scope = getOuterScope();

            } else {
               scope = self;

            }

            // prevent requests for tooltips while parsing a tooltip
            reentering = true; 

            m_private->brief->tooltip = parseCommentAsText(scope, md, m_private->brief->doc, 
                  m_private->brief->file, m_private->brief->line);

            reentering = false;
         }
      }

      return m_private->brief->tooltip;
   }

   return "";
}

int Definition::briefLine() const
{
   return m_private->brief ? m_private->brief->line : 1;
}

QString Definition::briefFile() const
{
   return m_private->brief ? m_private->brief->file : "<" + m_name + ">";
}

QString Definition::inbodyDocumentation() const
{
   return m_private->inbodyDocs ? m_private->inbodyDocs->doc : "";
}

int Definition::inbodyLine() const
{
   return m_private->inbodyDocs ? m_private->inbodyDocs->line : 1;
}

QString Definition::inbodyFile() const
{
   return m_private->inbodyDocs ? m_private->inbodyDocs->file : "<" + m_name + ">";
}

QString Definition::getDefFileName() const
{
   return m_private->defFileName;
}

QString Definition::getDefFileExtension() const
{
   return m_private->defFileExt;
}

bool Definition::isHidden() const
{
   return m_private->hidden;
}

bool Definition::isVisibleInProject() const
{
   return isLinkableInProject() && ! m_private->hidden;
}

bool Definition::isVisible() const
{
   return isLinkable() && ! m_private->hidden;
}

bool Definition::isArtificial() const
{
   return m_private->isArtificial;
}

QString Definition::getReference() const
{
   return m_private->ref;
}

bool Definition::isReference() const
{
   return !m_private->ref.isEmpty();
}

int Definition::getStartBodyLine() const
{
   return m_private->body ? m_private->body->startLine : -1;
}

int Definition::getEndBodyLine() const
{
   return m_private->body ? m_private->body->endLine : -1;
}

QSharedPointer<FileDef> Definition::getBodyDef() const
{
   return m_private->body ? m_private->body->fileDef : QSharedPointer<FileDef>();
}

SortedList<QSharedPointer<GroupDef>> *Definition::partOfGroups() const
{
   return m_private->partOfGroups;
}

QSharedPointer<Definition> Definition::findInnerCompound(const QString &)
{
   return QSharedPointer<Definition>();
}

QSharedPointer<Definition> Definition::getOuterScope() const
{
   return m_private->outerScope;
}

MemberSDict *Definition::getReferencesMembers() const
{
   return m_private->sourceRefsDict;
}

MemberSDict *Definition::getReferencedByMembers() const
{
   return m_private->sourceRefByDict;
}

void Definition::setReference(const QString &r)
{
   m_private->ref = r;
}

SrcLangExt Definition::getLanguage() const
{
   return m_private->lang;
}

void Definition::setHidden(bool b)
{  
   m_private->hidden = m_private->hidden || b;
}

void Definition::setArtificial(bool b)
{
   m_private->isArtificial = b;
}

void Definition::setLocalName(const QString &name)
{
   m_private->localName = name;
}

void Definition::setLanguage(SrcLangExt lang)
{
   m_private->lang = lang;
}

void Definition::setPhraseName(const QString &phrase)
{
   m_phraseName = phrase;
}

bool Definition::hasBriefDescription() const
{
   static bool briefMemberDesc = Config::getBool("brief-member-desc");
   return ! briefDescription().isEmpty() && briefMemberDesc;
}

