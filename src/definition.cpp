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

#include <QRegExp>
#include <QCryptographicHash>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <config.h>
#include <definition.h>
#include <doxy_globals.h>
#include <language.h>
#include <message.h>
#include <marshal.h>
#include <memberlist.h>
#include <namespacedef.h>
#include <outputlist.h>
#include <code.h>
#include <util.h>
#include <groupdef.h>
#include <pagedef.h>
#include <section.h>
#include <htags.h>
#include <parser_base.h>
#include <filedef.h>
#include <dirdef.h>

#define START_MARKER 0x4445465B    // DEF[
#define END_MARKER   0x4445465D    // DEF]

class Definition_Private
{
 public:
   Definition_Private();
   ~Definition_Private();

   void init(const char *df, const char *n);

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

   QByteArray   briefSignatures;
   QByteArray   docSignatures;

   QString localName;      // local (unqualified) name of the definition
   
   QString qualifiedName;
   QByteArray ref;         // reference to external documentation

   bool hidden;
   bool isArtificial;

   QSharedPointer<Definition> outerScope;  

   // where the item was found
   QByteArray defFileName;
   QByteArray defFileExt;

   SrcLangExt lang;

   QByteArray id; // clang unique id
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

void Definition_Private::init(const char *df, const char *n)
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

static bool matchExcludedSymbols(const char *name)
{
   static const QStringList exclSyms = Config::getList("exclude-symbols");

   if (exclSyms.count() == 0) {
      // nothing specified
      return false;   
   }

   QByteArray symName = name;

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
            if ((i == 0 || pattern.at(0) == '*' || (! isId(symName.at(i - 1))  && ! forceStart)) &&
                  (i + pl == sl || pattern.at(i + pl) == '*' || (! isId(symName.at(i + pl)) && ! forceEnd)) ) {             
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
            if ((i == 0  || (! isId(symName.at(i - 1))  && ! forceStart)) && (i + pl == sl || (!isId(symName.at(i + pl)) && !forceEnd)) ) {               
               return true;
            }
         }
      }      
   }
   
   return false;
}

void Definition::addToMap(const QByteArray &name)
{   
   QByteArray symbolName = name;
   int index = computeQualifiedIndex(symbolName);

   if (index != -1) {
      symbolName = symbolName.mid(index + 2);
   }

   if (! symbolName.isEmpty()) {                       
      Doxy_Globals::symbolMap().insertMulti(symbolName, this);         
      this->setSymbolName(symbolName);
   }
}

Definition::Definition(const char *df, int dl, int dc, const char *name, const char *b, const char *d, bool isSymbol)
{
   m_name      = name;
   m_defLine   = dl;
   m_defColumn = dc;

   m_private = new Definition_Private;
   m_private->init(df, name);

   m_isSymbol = isSymbol;

   if (isSymbol) {
      addToMap(name);
   }

   // id's used in ftvhelp
   m_inputOrderId = -1;
   m_sortId = -1;

   _setBriefDescription(b, df, dl);
   _setDocumentation(d, df, dl, true, false);

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

   m_isSymbol = d.m_isSymbol;
   if (m_isSymbol) {
      addToMap(m_name);
   }
}

Definition::~Definition()
{  
   if (m_private) {
      delete m_private;
      m_private = 0;
   }

   if (! Doxy_Globals::g_programExit)  {
      auto di = Doxy_Globals::symbolMap().find(m_symbolName);
   
      while (di != Doxy_Globals::symbolMap().end() && di.key() == m_symbolName)  {      
   
         if (di.value() == this) {
            di = Doxy_Globals::symbolMap().erase(di);
   
         }  else {
            ++di;
   
         }      
      }
   }
}

void Definition::setName(const char *name)
{
   if (name == 0) {
      return;
   }
   m_name = name;
}

void Definition::setId(const char *id)
{
   QSharedPointer<Definition> self = sharedFrom(this);

   if (id == 0) {
      return;
   }
  
   m_private->id = id; 
   Doxy_Globals::clangUsrMap.insert(id, self);
}

QByteArray Definition::id() const
{
   return m_private->id;
}

void Definition::addSectionsToDefinition(QList<SectionInfo> *anchorList)
{
   QSharedPointer<Definition> self = sharedFrom(this);

   if (! anchorList) {
      return;
   }

   for (auto si : *anchorList) {   
    
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

void Definition::addSectionsToIndex()
{ 
   int level = 1;

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

         Doxy_Globals::indexList->addContentsItem(true, title, getReference(), getOutputFileBase(), si->label, false, true);
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

      if (! si->generated) {
        
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

bool Definition::_docsAlreadyAdded(const QByteArray &doc, QByteArray &sigList)
{  
   // to avoid mismatches due to differences in indenting, we first remove
   // double whitespaces...

   QByteArray docStr = doc.simplified();

   QByteArray sigStr;
   sigStr = QCryptographicHash::hash(docStr, QCryptographicHash::Md5).toHex();  

   if (sigList.indexOf(sigStr) == -1) { 
      // new docs, add signature to prevent re-adding it
      sigList += ":" + sigStr;
      return false;

   } else {
      return true;

   }
}

void Definition::_setDocumentation(const QByteArray  &tDoc, const QByteArray &docFile, int docLine,
                                   bool stripWhiteSpace, bool atTop)
{

   QByteArray doc = tDoc;

   if (doc.isEmpty()) {
      return;
   }

   if (stripWhiteSpace) {
      doc = stripLeadingAndTrailingEmptyLines(doc, docLine);
   }

   if (! _docsAlreadyAdded(doc, m_private->docSignatures)) {
     
      if (m_private->details == 0) {
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

void Definition::setDocumentation(const char *d, const char *docFile, int docLine, bool stripWhiteSpace)
{
   if (d == 0) {
      return;
   }

   _setDocumentation(d, docFile, docLine, stripWhiteSpace, false);
}

#define uni_isupper(c) (QChar(c).category()==QChar::Letter_Uppercase)

// do a UTF-8 aware search for the last real character and return true
// if that is a multibyte one.
static bool lastCharIsMultibyte(const QByteArray &s)
{
   int l = s.length();
   int p = 0;
   int pp = -1;

   while ((p = nextUtf8CharPosition(s, l, p)) < l) {
      pp = p;
   }

   if (pp == -1 || ((uchar)s[pp]) < 0x80) {
      return false;
   }

   return true;
}

void Definition::_setBriefDescription(const char *b, const char *briefFile, int briefLine)
{
   static QString outputLanguage = Config::getEnum("output-language");
   static bool needsDot = (outputLanguage != "Japanese" && outputLanguage != "Chinese" && outputLanguage != "Korean");

   QByteArray brief = b;
   brief = brief.trimmed();

   if (brief.isEmpty()) {
      return;
   }

   int bl = brief.length();

   if (bl > 0 && needsDot) { 
      // add punctuation if needed
      int c = brief.at(bl - 1);

      switch (c) {
         case '.':
         case '!':
         case '?':
         case '>':
         case ':':
         case ')':
            break;

         default:
            if (uni_isupper(brief.at(0)) && ! lastCharIsMultibyte(brief)) {
               brief += '.';
            }
            break;
      }
   }

   if (!_docsAlreadyAdded(brief, m_private->briefSignatures)) {

      if (m_private->brief && !m_private->brief->doc.isEmpty()) {         
         _setDocumentation(brief, briefFile, briefLine, false, true);

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
   } else {
      //printf("do nothing!\n");
   }
}

void Definition::setBriefDescription(const char *b, const char *briefFile, int briefLine)
{
   if (b == 0) {
      return;
   }
   _setBriefDescription(b, briefFile, briefLine);
}

void Definition::_setInbodyDocumentation(const char *doc, const char *inbodyFile, int inbodyLine)
{
   if (m_private->inbodyDocs == 0) {
      m_private->inbodyDocs = new DocInfo;
   }
   if (m_private->inbodyDocs->doc.isEmpty()) { // fresh inbody docs
      m_private->inbodyDocs->doc  = doc;
      m_private->inbodyDocs->file = inbodyFile;
      m_private->inbodyDocs->line = inbodyLine;
   } else { // another inbody documentation fragment, append this to the end
      m_private->inbodyDocs->doc += QByteArray("\n\n") + doc;
   }
}

void Definition::setInbodyDocumentation(const char *d, const char *inbodyFile, int inbodyLine)
{
   if (d == 0) {
      return;
   }
   _setInbodyDocumentation(d, inbodyFile, inbodyLine);
}

/*! Reads a fragment of code from file \a fileName starting at
 * line \a startLine and ending at line \a endLine (inclusive). The fragment is
 * stored in \a result. If false is returned the code fragment could not be
 * found.
 *
 * The file is scanned for a opening bracket ('{') from \a startLine onward
 * The line actually containing the bracket is returned via startLine.
 * The file is scanned for a closing bracket ('}') from \a endLine backward.
 * The line actually containing the bracket is returned via endLine.
 * Note that for VHDL code the bracket search is not done.
 */
bool readCodeFragment(const char *fileName, int &startLine, int &endLine, QByteArray &result)
{
   static bool filterSourceFiles = Config::getBool("filter-source-files");
   static int tabSize = Config::getInt("tab-size");
   
   if (fileName == 0 || fileName[0] == 0) {
      return false;   // not a valid file name
   }

   QString filter = getFileFilter(fileName, true);

   FILE *f = 0;
   bool usePipe = !filter.isEmpty() && filterSourceFiles;

   SrcLangExt lang = getLanguageFromFileName(fileName);

   if (! usePipe) { 
      // no filter given or wanted
      f = fopen(fileName, "r");

   } else { 
      // use filter
      QString cmd = filter + " \"" + fileName + "\"";    
      f = popen(cmd.toUtf8(), "r");
   }

   bool found = lang == SrcLangExt_Tcl | lang == SrcLangExt_Python || lang == SrcLangExt_Fortran;

   // for TCL, Python, and Fortran no bracket search is possible

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

         while (lineNr <= endLine && ! feof(f) && !found) {
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
            // For code with more than one line,
            // fill the line with spaces until we are at the right column
            // so that the opening brace lines up with the closing brace
            if (endLine != startLine) {
               QByteArray spaces;
               spaces.fill(' ', col);
               result += spaces;
            }

            // copy until end of line            
            if (c) {
               result += c;
            }

            startLine = lineNr;

            if (c == ':') {
               result += cn;
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
                  result += lineStr;
               } while (size_read == (maxLineLength - 1));

               lineNr++;
            } while (lineNr <= endLine && !feof(f));

            // strip stuff after closing bracket
            int newLineIndex = result.lastIndexOf('\n');
            int braceIndex   = result.lastIndexOf('}');

            if (braceIndex > newLineIndex) {
               result.truncate(braceIndex + 1);
            }

            endLine = lineNr - 1;
         }
      }
      if (usePipe) {
         pclose(f);
         Debug::print(Debug::FilterOutput, 0, "Filter output\n");
         Debug::print(Debug::FilterOutput, 0, "-------------\n%s\n-------------\n", result.data());
      } else {
         fclose(f);
      }
   }
   result = transcodeCharacterStringToUTF8(result);
   //fprintf(stderr,"readCodeFragement(%d-%d)=%s\n",startLine,endLine,result.data());
   return found;
}

QByteArray Definition::getSourceFileBase() const
{
   assert(definitionType() != Definition::TypeFile); // file overloads this method

   QByteArray fn;
   static bool sourceBrowser = Config::getBool("source-code");

   if (sourceBrowser && m_private->body && m_private->body->startLine != -1 && m_private->body->fileDef) {
      fn = m_private->body->fileDef->getSourceFileBase();
   }

   return fn;
}

QByteArray Definition::getSourceAnchor() const
{
   const int maxAnchorStrLen = 20;
   char anchorStr[maxAnchorStrLen];
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
void Definition::writeSourceDef(OutputList &ol, const char *)
{
   static bool latexSourceCode = Config::getBool("latex-source-code");
   ol.pushGeneratorState();
  
   QByteArray fn = getSourceFileBase();

   if (! fn.isEmpty()) {
      QString refText = theTranslator->trDefinedAtLineInSourceFile();

      int lineMarkerPos = refText.indexOf("@0");
      int fileMarkerPos = refText.indexOf("@1");

      if (lineMarkerPos != -1 && fileMarkerPos != -1) { 
         // should always pass this.
         QString lineStr;
         lineStr = QString("%1").arg(m_private->body->startLine);

         QByteArray anchorStr = getSourceAnchor();
         ol.startParagraph();

         if (lineMarkerPos < fileMarkerPos) { // line marker before file marker
            // write text left from linePos marker
            ol.parseText(refText.left(lineMarkerPos));
            ol.pushGeneratorState();
            ol.disable(OutputGenerator::RTF);
            ol.disable(OutputGenerator::Man);

            if (!latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            // write line link (HTML, LaTeX optionally)
            ol.writeObjectLink(0, fn, anchorStr, lineStr.toUtf8());
            ol.enableAll();
            ol.disable(OutputGenerator::Html);

            if (latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            // write normal text (Man/RTF, Latex optionally)
            ol.docify(lineStr);
            ol.popGeneratorState();

            // write text between markers
            ol.parseText(refText.mid(lineMarkerPos + 2, fileMarkerPos - lineMarkerPos - 2));

            ol.pushGeneratorState();
            ol.disable(OutputGenerator::RTF);
            ol.disable(OutputGenerator::Man);
            if (!latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }
            // write file link (HTML, LaTeX optionally)
            ol.writeObjectLink(0, fn, 0, m_private->body->fileDef->name());
            ol.enableAll();

            ol.disable(OutputGenerator::Html);
            if (latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
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
            ol.disable(OutputGenerator::RTF);
            ol.disable(OutputGenerator::Man);
            if (!latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }
            // write file link (HTML only)
            ol.writeObjectLink(0, fn, 0, m_private->body->fileDef->name());

            ol.enableAll();
            ol.disable(OutputGenerator::Html);
            if (latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }
            // write normal text (Latex/Man only)
            ol.docify(m_private->body->fileDef->name());
            ol.popGeneratorState();

            // write text between markers
            ol.parseText(refText.mid(fileMarkerPos + 2, lineMarkerPos - fileMarkerPos - 2));

            ol.pushGeneratorState();
            ol.disable(OutputGenerator::RTF);
            ol.disable(OutputGenerator::Man);
            if (! latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }
            ol.disableAllBut(OutputGenerator::Html);
            // write line link (HTML only)
            ol.writeObjectLink(0, fn, anchorStr, lineStr.toUtf8());
            ol.enableAll();
            ol.disable(OutputGenerator::Html);

            if (latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            // write normal text (Latex/Man only)
            ol.docify(lineStr);
            ol.popGeneratorState();

            // write text right from linePos marker
            ol.parseText(refText.right(refText.length() - lineMarkerPos - 2));
         }
         ol.endParagraph();

      } else {
         err("Error: Invalid translation markers in 'trDefinedAtLineInSourceFile'\n");
      }
   }
   ol.popGeneratorState();
}

void Definition::setBodySegment(int bls, int ble)
{
   //printf("setBodySegment(%d,%d) for %s\n",bls,ble,name().data());
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
void Definition::writeInlineCode(OutputList &ol, const char *scopeName)
{
   QSharedPointer<Definition> self = sharedFrom(this);

   static bool inlineSources = Config::getBool("inline-source");
   ol.pushGeneratorState();
   
   if (inlineSources && hasSources()) {
      QByteArray codeFragment;

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
void Definition::_writeSourceRefList(OutputList &ol, const char *scopeName,
                                     const QString &text, MemberSDict *members, bool /*funcOnly*/)
{
   static bool latexSourceCode = Config::getBool("latex-source-code");
   static bool sourceBrowser   = Config::getBool("source-code");
   static bool refLinkSource   = Config::getBool("ref-link-source");

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
            QByteArray scope = md->getScopeString();
            QByteArray name  = md->name();
           
            if (!scope.isEmpty() && scope != scopeName) {
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

               if (!latexSourceCode) {
                  ol.disable(OutputGenerator::Latex);
               }

               const int maxLineNrStr = 10;
               char anchorStr[maxLineNrStr];
               qsnprintf(anchorStr, maxLineNrStr, "l%05d", md->getStartBodyLine());

               //printf("Write object link to %s\n",md->getBodyDef()->getSourceFileBase().data());
               ol.writeObjectLink(0, md->getBodyDef()->getSourceFileBase(), anchorStr, name);
               ol.popGeneratorState();

               // for the other output formats just mention the name
               ol.pushGeneratorState();
               ol.disable(OutputGenerator::Html);
               if (latexSourceCode) {
                  ol.disable(OutputGenerator::Latex);
               }
               ol.docify(name);
               ol.popGeneratorState();
            } else if (md->isLinkable() /*&& d && d->isLinkable()*/) {
               // for HTML write a real link
               ol.pushGeneratorState();
               //ol.disableAllBut(OutputGenerator::Html);
               ol.disable(OutputGenerator::RTF);
               ol.disable(OutputGenerator::Man);
               if (!latexSourceCode) {
                  ol.disable(OutputGenerator::Latex);
               }

               ol.writeObjectLink(md->getReference(),
                                  md->getOutputFileBase(),
                                  md->anchor(), name);
               ol.popGeneratorState();

               // for the other output formats just mention the name
               ol.pushGeneratorState();
               ol.disable(OutputGenerator::Html);
               if (latexSourceCode) {
                  ol.disable(OutputGenerator::Latex);
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

void Definition::writeSourceReffedBy(OutputList &ol, const char *scopeName)
{
   if (Config::getBool("ref-by-relation")) {
      _writeSourceRefList(ol, scopeName, theTranslator->trReferencedBy(), m_private->sourceRefByDict, false);
   }
}

void Definition::writeSourceRefs(OutputList &ol, const char *scopeName)
{
   if (Config::getBool("ref-relation")) {
      _writeSourceRefList(ol, scopeName, theTranslator->trReferences(), m_private->sourceRefsDict, true);
   }
}

bool Definition::hasDocumentation() const
{
   static bool extractAll = Config::getBool("extract-all");
   // static bool sourceBrowser = Config::getBool("source-code");

   bool hasDocs = (m_private->details    && !m_private->details->doc.isEmpty())    || 
                  (m_private->brief      && !m_private->brief->doc.isEmpty())      ||
                  (m_private->inbodyDocs && !m_private->inbodyDocs->doc.isEmpty()) ||  extractAll; 

      //  ||   // extract everything
      //       (sourceBrowser && m_private->body && m_private->body->startLine!=-1 && m_private->body->fileDef)
    
   return hasDocs;
}

bool Definition::hasUserDocumentation() const
{
   bool hasDocs = (m_private->details    && !m_private->details->doc.isEmpty()) ||
                  (m_private->brief      && !m_private->brief->doc.isEmpty())   ||
                  (m_private->inbodyDocs && !m_private->inbodyDocs->doc.isEmpty());

   return hasDocs;
}


void Definition::addSourceReferencedBy(QSharedPointer<MemberDef> md)
{
   if (md) {
      QByteArray name  = md->name();
      QByteArray scope = md->getScopeString();

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
   QByteArray name  = md->name();
   QByteArray scope = md->getScopeString();

   if (md) {
      QByteArray name  = md->name();
      QByteArray scope = md->getScopeString();

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
   
      for (auto lii : *sli) {
         m_private->xrefListItems->append(lii);
      }
   }
}

void Definition::mergeRefItems(QSharedPointer<Definition> d)
{
   //printf("%s::mergeRefItems()\n",name().data());
   QList<ListItemInfo> *xrefList = d->xrefListItems();

   if (xrefList != 0) {
      // deep copy the list
      if (m_private->xrefListItems == 0) {
         m_private->xrefListItems = new QList<ListItemInfo>;         
      }
    
      for (auto lii : *xrefList) {  
         if (_getXRefListId(lii.type) == -1) {
            m_private->xrefListItems->append(lii);
         }
      }
   }
}

int Definition::_getXRefListId(const char *listName) const
{
   if (m_private->xrefListItems) {     
      for (auto lii : *m_private->xrefListItems) {  
         if (qstrcmp(lii.type, listName) == 0) {
            return lii.itemId;
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

      if (definitionType() == Definition::TypeGroup && ! ((const GroupDef *)this)->groupTitle().isEmpty()) {
         result += ((const GroupDef *)this)->groupTitle();

      } else if (definitionType() == Definition::TypePage && ! ((const PageDef *)this)->title().isEmpty()) {
         result += ((const PageDef *)this)->title();

      } else {
         result += m_private->localName;
      }

   } else {
      result += m_private->localName;
   }

   return result;
}


// TODO: move to htmlgen
/*! Returns the string used in the footer for $navpath when
 *  GENERATE_TREEVIEW is enabled
 */
QString Definition::navigationPathAsString() const
{
   QString result;
   QSharedPointer<Definition> outerScope = getOuterScope();

   QString locName = localName();

   if (outerScope && outerScope != Doxy_Globals::globalScope) {
      result += outerScope->navigationPathAsString();

   } else if (definitionType() == Definition::TypeFile && ((const FileDef *)this)->getDirDef()) {
      result += ((const FileDef *)this)->getDirDef()->navigationPathAsString();

   }

   result += "<li class=\"navelem\">";

   if (isLinkable()) {
      if (definitionType() == Definition::TypeGroup && ! ((const GroupDef *)this)->groupTitle().isEmpty()) {
         result += "<a class=\"el\" href=\"$relpath^" + getOutputFileBase() + Doxy_Globals::htmlFileExtension + "\">" +
                   convertToHtml(((const GroupDef*)this)->groupTitle()) + "</a>";

      } else if (definitionType() == Definition::TypePage && ! ((const PageDef *)this)->title().isEmpty()) {
         result += "<a class=\"el\" href=\"$relpath^" + getOutputFileBase() + Doxy_Globals::htmlFileExtension + "\">" +
                   convertToHtml(((const PageDef*)this)->title()) + "</a>";

      } else if (definitionType() == Definition::TypeClass) {
         QString name = locName;

         if (name.right(2) == "-p" /*|| name.right(2)=="-g"*/) {
            name = name.left(name.length() - 2);
         }

         result += "<a class=\"el\" href=\"$relpath^" + getOutputFileBase() + Doxy_Globals::htmlFileExtension;
         if (! anchor().isEmpty()) {
            result += "#" + anchor();
         }

         result+="\">" + convertToHtml(name) + "</a>";

      } else {
         result += "<a class=\"el\" href=\"$relpath^" + getOutputFileBase() + Doxy_Globals::htmlFileExtension + "\">" +
                   convertToHtml(locName) + "</a>";
      }

   } else {
      result+="<b>" + convertToHtml(locName) + "</b>";
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
   navPath += "<div id=\"nav-path\" class=\"navpath\">\n"
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
   int l;
   char cs[2];
   cs[1] = '\0';

   bool inLi[5] = { false, false, false, false };   

   // by definition order  
   for (auto si : m_private->sectionList) {   

      if (si->type == SectionInfo::Section || si->type == SectionInfo::Subsection    ||
            si->type == SectionInfo::Subsubsection || si->type == SectionInfo::Paragraph) {
        
         int nextLevel = si->type;

         if (nextLevel > level) {
            for (l = level; l < nextLevel; l++) {
               ol.writeString("<ul>");
            }

         } else if (nextLevel < level) {
            for (l = level; l > nextLevel; l--) {
               if (inLi[l]) {
                  ol.writeString("</li>\n");
               }

               inLi[l] = false;
               ol.writeString("</ul>\n");
            }
         }

         cs[0] = '0' + nextLevel;
         if (inLi[nextLevel]) {
            ol.writeString("</li>\n");
         }
     
         QByteArray titleDoc = convertToHtml(si->title);
         ol.writeString("<li class=\"level" + QByteArray(cs) + "\"><a href=\"#" + si->label + "\">" + (si->title.isEmpty() ? si->label:titleDoc) + "</a>");
   
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

QByteArray Definition::symbolName() const
{
   return m_symbolName;
}

QByteArray Definition::documentation() const
{
   return m_private->details ? m_private->details->doc : QByteArray("");
}

int Definition::docLine() const
{
   return m_private->details ? m_private->details->line : 1;
}

QByteArray Definition::docFile() const
{
   return m_private->details ? m_private->details->file : QByteArray("<" + m_name + ">");
}

// strips w from s if s starts with w
static bool stripWord(QByteArray &s, QByteArray w)
{
   bool success = false;

   if (s.left(w.length()) == w) {
      success = true;
      s = s.right(s.length() - w.length());
   }

   return success;
}

// some quasi intelligent brief description abbreviator :^)
QByteArray abbreviate(const char *s, const char *name)
{
   QByteArray scopelessName = name;
   int i = scopelessName.lastIndexOf("::");

   if (i != -1) {
      scopelessName = scopelessName.mid(i + 2);
   }

   QByteArray result = s;
   result = result.trimmed();

   // strip trailing .
   if (!result.isEmpty() && result.at(result.length() - 1) == '.') {
      result = result.left(result.length() - 1);
   }

   // strip any predefined prefix
   const QStringList briefDescAbbrev = Config::getList("abbreviate-brief");
  
   for (auto s : briefDescAbbrev) {     
      s.replace(QRegExp("\\$name"), scopelessName);     
      
      // replace $name with entity name
      s += " ";

      stripWord(result, s.toUtf8());      
   }

   // capitalize first word
   if (! result.isEmpty()) {
      int c = result[0];
      if (c >= 'a' && c <= 'z') {
         c += 'A' - 'a';
      }

      result[0] = c;
   }
   return result;
}

QByteArray Definition::briefDescription(bool abbr) const
{
   return m_private->brief ? (abbr ? abbreviate(m_private->brief->doc, displayName().toUtf8()) : m_private->brief->doc) : QByteArray("");
}

QByteArray Definition::briefDescriptionAsTooltip() const
{
   QSharedPointer<Definition> self = sharedFrom(this);

   if (m_private->brief) {
      if (m_private->brief->tooltip.isEmpty() && ! m_private->brief->doc.isEmpty()) {
         static bool reentering = false;

         if (! reentering) {
            QSharedPointer<MemberDef> md = definitionType() == TypeMember ? self.dynamicCast<MemberDef>() : QSharedPointer<MemberDef>();

            QSharedPointer<Definition> scope;

            if (definitionType() == TypeMember) {
               scope = getOuterScope();

            } else {
               scope = self;

            }

            // prevent requests for tooltips while parsing a tooltip
            reentering = true; 

            m_private->brief->tooltip = parseCommentAsText(scope, md, m_private->brief->doc, m_private->brief->file, m_private->brief->line);
            reentering = false;
         }
      }
      return m_private->brief->tooltip;
   }

   return QByteArray("");
}

int Definition::briefLine() const
{
   return m_private->brief ? m_private->brief->line : 1;
}

QByteArray Definition::briefFile() const
{
   return m_private->brief ? m_private->brief->file : QByteArray("<" + m_name + ">");
}

QByteArray Definition::inbodyDocumentation() const
{
   return m_private->inbodyDocs ? m_private->inbodyDocs->doc : QByteArray("");
}

int Definition::inbodyLine() const
{
   return m_private->inbodyDocs ? m_private->inbodyDocs->line : 1;
}

QByteArray Definition::inbodyFile() const
{
   return m_private->inbodyDocs ? m_private->inbodyDocs->file : QByteArray("<" + m_name + ">");
}

QByteArray Definition::getDefFileName() const
{
   return m_private->defFileName;
}

QByteArray Definition::getDefFileExtension() const
{
   return m_private->defFileExt;
}

bool Definition::isHidden() const
{
   return m_private->hidden;
}

bool Definition::isVisibleInProject() const
{
   return isLinkableInProject() && !m_private->hidden;
}

bool Definition::isVisible() const
{
   return isLinkable() && !m_private->hidden;
}

bool Definition::isArtificial() const
{
   return m_private->isArtificial;
}

QByteArray Definition::getReference() const
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

void Definition::setReference(const char *r)
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
   m_private->localName = name.toUtf8();
}

void Definition::setLanguage(SrcLangExt lang)
{
   m_private->lang = lang;
}

void Definition::setSymbolName(const QByteArray &name)
{
   m_symbolName = name;
}

bool Definition::hasBriefDescription() const
{
   static bool briefMemberDesc = Config::getBool("brief-member-desc");
   return ! briefDescription().isEmpty() && briefMemberDesc;
}

