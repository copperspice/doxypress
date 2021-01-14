/************************************************************************
*
* Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
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

#include <QCryptographicHash>
#include <QProcess>
#include <QRegularExpression>

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
#include <outputlist.h>
#include <parse_base.h>
#include <util.h>

class Definition_Private
{
 public:
   Definition_Private();
   ~Definition_Private();

   void init(const QString &df, const QString &n);

   SectionDict m_sectionDict;                             // dictionary of all sections
   QVector<QSharedPointer <SectionInfo>> m_sectionList;   // list of sections, definiton order

   MemberSDict m_sourceRefByDict;
   MemberSDict m_sourceRefsDict;

   QVector<ListItemInfo> m_xrefListItems;
   SortedList<QSharedPointer<GroupDef>> *partOfGroups;

   DocInfo    m_details;       // not exported
   DocInfo    m_inbodyDocs;    // not exported
   BriefInfo  m_brief;         // not exported

   // data associated with description found in the body
   int m_body_startLine;                      // line number of the start of the definition
   int m_body_endLine;                        // line number of the end of the definition
   QSharedPointer<FileDef> m_body_fileDef;    // file definition containing the function body

   QString briefSignatures;
   QString docSignatures;

   QString localName;         // local (unqualified) name of the definition

   QString qualifiedName;
   QString ref;               // reference to external documentation

   bool hidden;
   bool isArtificial;

   QSharedPointer<Definition> outerScope;

   // where the item was found
   QString defFileName;
   QString defFileExt;

   SrcLangExt lang;

   QString id;                // clang unique id
};

Definition_Private::Definition_Private()
   : partOfGroups(0), m_body_startLine(-1), m_body_endLine(-1), hidden(false),
     isArtificial(false), lang(SrcLangExt_Unknown)
{
}

Definition_Private::~Definition_Private()
{
   delete partOfGroups;
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

   partOfGroups    = nullptr;
   outerScope      = Doxy_Globals::globalScope;

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
         pattern = pattern.left(pattern.length() - 1);
         forceEnd = true;
      }

      if (pattern.indexOf('*') != -1) {
         // wildcard mode
         QRegularExpression regExp(pattern.replace("*", ".*"));
         QRegularExpressionMatch match = regExp.match(symName);

         if (match.hasMatch()) {
            int i  = match.capturedStart() - symName.begin();
            int pl = match.capturedLength();
            int sl = symName.length();

            // check if it is a whole word match
            if ((i == 0 || pattern.startsWith('*') || (! isId(symName.at(i - 1))  && ! forceStart)) &&
                  (i + pl == sl || pattern.endsWith('*') || (! isId(symName.at(i + pl)) && ! forceEnd)) ) {
               return true;
            }
         }

      } else if (! pattern.isEmpty()) {
         // match words
         int i = symName.indexOf(pattern);

         if (i != -1) {
            // we have a match
            int pl = pattern.length();
            int sl = symName.length();

            // check if it is a whole word match
            if ((i == 0  || (! isId(symName.at(i - 1))  && ! forceStart)) && (i + pl == sl ||
                     (! isId(symName.at(i + pl)) && ! forceEnd)) ) {
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
   m_name      = d.m_name;
   m_requires  = d.m_requires;

   m_defLine   = d.m_defLine;
   m_defColumn = d.m_defColumn;

   m_private  = new Definition_Private;
   *m_private = *d.m_private;

   m_private->partOfGroups = nullptr;

   m_private->m_sectionDict      = d.m_private->m_sectionDict;
   m_private->m_sectionList      = d.m_private->m_sectionList;
   m_private->m_sourceRefByDict  = d.m_private->m_sourceRefByDict;
   m_private->m_sourceRefsDict   = d.m_private->m_sourceRefsDict;

   if (d.m_private->partOfGroups) {
      for (auto gd : *d.m_private->partOfGroups) {
         makePartOfGroup(gd);
      }
   }

   setRefItems(d.m_private->m_xrefListItems);

   m_private->m_details        = d.m_private->m_details;
   m_private->m_inbodyDocs     = d.m_private->m_inbodyDocs;
   m_private->m_brief          = d.m_private->m_brief;

   m_private->m_body_startLine = d.m_private->m_body_startLine;
   m_private->m_body_endLine   = d.m_private->m_body_endLine;
   m_private->m_body_fileDef   = d.m_private->m_body_fileDef;

   m_isPhrase = d.m_isPhrase;

   if (m_isPhrase) {
      addToMap(m_name);
   }
}

Definition::~Definition()
{
   if (m_private) {
      delete m_private;
      m_private = nullptr;
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

void Definition::addSectionsToDefinition(const QVector<SectionInfo> &anchorList)
{
   QSharedPointer<Definition> self = sharedFrom(this);

   for (auto &si : anchorList) {
      QSharedPointer<SectionInfo> gsi (Doxy_Globals::sectionDict.find(si.label));

      if (! gsi) {
         gsi = QMakeShared<SectionInfo>(si);
         Doxy_Globals::sectionDict.insert(si.label, gsi);
      }

      if (m_private->m_sectionDict.find(gsi->label) == nullptr) {

         m_private->m_sectionDict.insert(gsi->label, gsi);
         gsi->definition = self;

         m_private->m_sectionList.append(gsi);
      }
   }
}

bool Definition::hasSections() const
{
   for (auto &si : m_private->m_sectionDict) {
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

   auto nextItem = m_private->m_sectionList.begin();

   // by definition order
   for (auto si : m_private->m_sectionList) {

      if (si->type == SectionInfo::Section || si->type == SectionInfo::Subsection ||
            si->type == SectionInfo::Subsubsection || si->type == SectionInfo::Paragraph) {

         int nextLevel = si->type;
         int i;

         if (nextLevel > level) {
            for (i = level; i < nextLevel; i++) {
               Doxy_Globals::indexList.incContentsDepth();
            }

         } else if (nextLevel < level) {
            for (i = nextLevel; i < level; i++) {
               Doxy_Globals::indexList.decContentsDepth();
            }
         }

         QString title = si->title;

         if (title.isEmpty()) {
            title = si->label;
         }

         // determine if there is a next level inside this item
         ++nextItem;
         bool isDir = false;

         if (nextItem != m_private->m_sectionList.end())  {
            isDir = (*nextItem)->type > nextLevel;
         }

         Doxy_Globals::indexList.addContentsItem(isDir, title, getReference(), getOutputFileBase(), si->label, addToNavIndex);
         level = nextLevel;
      }
   }

   while (level > 1) {
      Doxy_Globals::indexList.decContentsDepth();
      level--;
   }
}

void Definition::writeDocAnchorsToTagFile(QTextStream &tagFile) const
{
   // by definition order
   for (auto si : m_private->m_sectionList) {

      if (! si->generated && si->ref.isEmpty()) {

         if (definitionType() == TypeMember) {
            tagFile << "  ";
         }

         tagFile << "    <docanchor file=\"" << si->fileName << "\"";

         if (! si->title.isEmpty()) {
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

   QByteArray data = QCryptographicHash::hash(docStr.toUtf8(), QCryptographicHash::Md5).toHex();
   QString sigStr  = QString::fromLatin1(data);

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

      if (m_private->m_details.doc.isEmpty()) {
         // new detailed description
         m_private->m_details.doc = doc;

      } else if (atTop) {
         // another detailed description, append it to the start
         m_private->m_details.doc = doc + "\n\n" + m_private->m_details.doc;

      } else {
         // another detailed description, append it to the end
         m_private->m_details.doc += "\n\n" + doc;

      }

      if (docLine != -1) {
         // store location if valid
         m_private->m_details.file = docFile;
         m_private->m_details.line = docLine;

      } else {
         m_private->m_details.file = docFile;
         m_private->m_details.line = 1;
      }
   }
}

void Definition::setBriefDescription(const QString &b, const QString &briefFile, int briefLine)
{
   static const QString outputLanguage = Config::getEnum("output-language");
   static const bool needsDot          = (outputLanguage != "Japanese" && outputLanguage != "Chinese" && outputLanguage != "Korean");

   QString brief = b.trimmed();

   brief = trimEmptyLines(brief, briefLine);
   brief = brief.trimmed();
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

      if (! m_private->m_brief.doc.isEmpty()) {
         setDocumentation(brief, briefFile, briefLine, false, true);

      } else {
         m_private->m_brief.doc = brief;

         if (briefLine != -1) {
            m_private->m_brief.file = briefFile;
            m_private->m_brief.line = briefLine;

         } else {
            m_private->m_brief.file = briefFile;
            m_private->m_brief.line = 1;
         }
      }
   }
}

void Definition::_setInbodyDocumentation(const QString &doc, const QString &inbodyFile, int inbodyLine)
{
   if (m_private->m_inbodyDocs.doc.isEmpty()) {
      // fresh inbody docs
      m_private->m_inbodyDocs.doc  = doc;
      m_private->m_inbodyDocs.file = inbodyFile;
      m_private->m_inbodyDocs.line = inbodyLine;

   } else {
      // another inbody documentation fragment, append this to the end
      m_private->m_inbodyDocs.doc += "\n\n" + doc;
   }
}

void Definition::setInbodyDocumentation(const QString &d, const QString &inbodyFile, int inbodyLine)
{
   if (d.isEmpty()) {
      return;
   }

   _setInbodyDocumentation(d, inbodyFile, inbodyLine);
}


/*! Reads a fragment of code from file fileName from startLine to  endLine (inclusive).
 * The fragment is stored in result. Return false if the code fragment could not be found
 *
 * The file is scanned for a opening bracket ('{') from \a startLine onward
 * The line actually containing the bracket is returned via startLine.
 * The file is scanned for a closing bracket ('}') from \a endLine backward.
 * The line actually containing the bracket is returned via endLine.
 */
static bool readCodeFragment(const QString &fileName, int &startLine, int &endLine, QString &result)
{
   static const bool filterSourceFiles = Config::getBool("filter-source-files");

   if (fileName.isEmpty()) {
      return false;
   }

   SrcLangExt lang = getLanguageFromFileName(fileName);

   QString tmpResult;
   QString filter = getFileFilter(fileName, true);

   QByteArray buffer;

   if (filterSourceFiles && ! filter.isEmpty()) {

      QStringList cmdList;
      cmdList.append(fileName);

      QProcess task;
      task.start(filter, cmdList);

      while (task.waitForReadyRead(-1)) {
         buffer.append(task.readAllStandardOutput());
      }

      if (task.exitStatus() != QProcess::NormalExit) {
         err("Unable to execute %s\n", csPrintable(fileName));
         return false;
      }

    } else {
      QFile f(fileName);

      if (! f.open(QIODevice::ReadOnly)) {
         err("Unable to open file %s for reading, OS Error #: %d\n", csPrintable(fileName), f.error());
         return false;
      }

      buffer = f.readAll();
      f.close();
   }

   // for TCL, Python, and Fortran no bracket search is possible
   bool found = (lang == SrcLangExt_Tcl) || (lang == SrcLangExt_Python) || (lang == SrcLangExt_Fortran);

   const char *ptr = buffer.constData();

   if (ptr) {
      int c      = 0;
      int lineNr = 1;

      // skip until the startLine has reached

      while (lineNr < startLine && *ptr) {
         while ((c = *ptr++) != '\n' && c != 0) {
            // skip
         }

         lineNr++;

         if (found && c == '\n') {
            c = '\0';
         }
      }

      if (*ptr) {
         found = true;

         // skip over newline
         if (c == '\n') {
             c = *ptr;
             ++ptr;
         }

         // copy until end of line
         if (c != '\0') {
            tmpResult += c;
         }

         startLine = lineNr;

         constexpr const int maxLineLength = 4096;
         char lineStr[maxLineLength];

         do {
            int size_read;

            do {
               // read up to maxLineLength-1 bytes, the last byte being zero
               int i = 0;

               while ((c = *ptr++) && i < maxLineLength - 1) {
                  lineStr[i] = c;
                  ++i;

                  if (c == '\n') {
                     break;
                  }
               }

               lineStr[i] = '\0';
               size_read  = i;

               tmpResult += lineStr;

            } while (size_read == (maxLineLength - 1));

            lineNr++;

         } while (lineNr <= endLine && *ptr);

         // strip stuff after closing bracket
         int newLineIndex = tmpResult.lastIndexOf('\n');
         int braceIndex   = tmpResult.lastIndexOf('}');

         if (braceIndex > newLineIndex) {
            tmpResult.truncate(braceIndex + 1);
         }

         endLine = lineNr - 1;
      }
   }

   result = tmpResult;

   if (! result.isEmpty() && ! result.endsWith('\n')) {
      result += "\n";
   }

   return found;
}

QString Definition::getSourceFileBase() const
{
   static const bool sourceCode = Config::getBool("source-code");

   QString fn;

   if (sourceCode && m_private->m_body_startLine != -1 && m_private->m_body_fileDef) {
      fn = m_private->m_body_fileDef->getSourceFileBase();
   }

   return fn;
}

QString Definition::getSourceAnchor() const
{
   QString anchorStr;

   if (m_private->m_body_startLine != -1) {

      if (Htags::useHtags) {
         anchorStr = QString("L%1").formatArg(m_private->m_body_startLine);

      } else {
         anchorStr = QString("l%1").formatArg(m_private->m_body_startLine, 5, 10, QChar('0'));
      }
   }

   return anchorStr;
}

/*! Write a reference to the source code defining this definition */
void Definition::writeSourceDef(OutputList &ol, const QString &)
{
   static const bool latexSourceCode   = Config::getBool("latex-source-code");
   static const bool rtfSourceCode     = Config::getBool("rtf-source-code");
   static const bool docbookSourceCode = Config::getBool("docbook-program-listing");

   ol.pushGeneratorState();

   QString fn = getSourceFileBase();

   if (! fn.isEmpty()) {
      QString refText = theTranslator->trDefinedAtLineInSourceFile();

      int lineMarkerPos = refText.indexOf("@0");
      int fileMarkerPos = refText.indexOf("@1");

      if (lineMarkerPos != -1 && fileMarkerPos != -1) {
         // should always pass this
         QString lineStr = QString("%1").formatArg(m_private->m_body_startLine);

         QString anchorStr = getSourceAnchor();
         ol.startParagraph("definition");

         if (lineMarkerPos < fileMarkerPos) {
            // line marker before file marker, write text left from linePos marker
            ol.parseText(refText.left(lineMarkerPos));
            ol.pushGeneratorState();

            ol.disable(OutputGenerator::Man);

            if (! latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            if (! docbookSourceCode) {
               ol.disable(OutputGenerator::Docbook);
            }

            if (! rtfSourceCode) {
               ol.disable(OutputGenerator::RTF);
            }

            // write line link (HTML, LaTeX, Docbook, RTF)
            ol.writeObjectLink(QString(), fn, anchorStr, lineStr);
            ol.enableAll();

            ol.disable(OutputGenerator::Html);

            if (latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            if (docbookSourceCode) {
               ol.disable(OutputGenerator::Docbook);
            }

            if (rtfSourceCode) {
               ol.disable(OutputGenerator::RTF);
            }

            // write normal text (Latex, Man, RTF)
            ol.docify(lineStr);
            ol.popGeneratorState();

            // write text between markers
            ol.parseText(refText.mid(lineMarkerPos + 2, fileMarkerPos - lineMarkerPos - 2));

            ol.pushGeneratorState();

            ol.disable(OutputGenerator::Man);

            if (! latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            if (! docbookSourceCode) {
               ol.disable(OutputGenerator::Docbook);
            }

            if (! rtfSourceCode) {
               ol.disable(OutputGenerator::RTF);
            }

            // write file link (HTML, LaTeX optionally, RTF optionally)
            ol.writeObjectLink(QString(), fn, QString(), m_private->m_body_fileDef->name());
            ol.enableAll();

            ol.disable(OutputGenerator::Html);
            if (latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            if (docbookSourceCode) {
               ol.disable(OutputGenerator::Docbook);
            }

            if (rtfSourceCode) {
               ol.disable(OutputGenerator::RTF);
            }

            // write normal text (Man/RTF, Latex optionally)
            ol.docify(m_private->m_body_fileDef->name());
            ol.popGeneratorState();

            // write text right from file marker
            ol.parseText(refText.right(refText.length() - fileMarkerPos - 2));

         } else {
            // file marker before line marker, write text left from file marker

            ol.parseText(refText.left(fileMarkerPos));
            ol.pushGeneratorState();

            ol.disable(OutputGenerator::Man);

            if (! latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            if (! docbookSourceCode) {
               ol.disable(OutputGenerator::Docbook);
            }

            if (! rtfSourceCode) {
               ol.disable(OutputGenerator::RTF);
            }

            // write file link (HTML only)
            ol.writeObjectLink(QString(), fn, QString(), m_private->m_body_fileDef->name());

            ol.enableAll();

            ol.disable(OutputGenerator::Html);

            if (latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            if (docbookSourceCode) {
               ol.disable(OutputGenerator::Docbook);
            }

            if (rtfSourceCode) {
               ol.disable(OutputGenerator::RTF);
            }

            // write normal text (RTF/Latex/Man only)
            ol.docify(m_private->m_body_fileDef->name());
            ol.popGeneratorState();

            // write text between markers
            ol.parseText(refText.mid(fileMarkerPos + 2, lineMarkerPos - fileMarkerPos - 2));

            ol.pushGeneratorState();
            ol.disable(OutputGenerator::Man);
            ol.disableAllBut(OutputGenerator::Html);

            if (latexSourceCode) {
               ol.enable(OutputGenerator::Latex);
            }

            if (docbookSourceCode) {
               ol.disable(OutputGenerator::Docbook);
            }

            if (rtfSourceCode) {
               ol.enable(OutputGenerator::RTF);
            }

            // write line link (HTML only)
            ol.writeObjectLink(QString(), fn, anchorStr, lineStr);
            ol.enableAll();

            ol.disable(OutputGenerator::Html);

            if (latexSourceCode) {
               ol.disable(OutputGenerator::Latex);
            }

            if (docbookSourceCode) {
               ol.disable(OutputGenerator::Docbook);
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
         err("Invalid translation markers in trDefinedAtLineInSourceFile\n");
      }
   }

   ol.popGeneratorState();
}

void Definition::setBodySegment(int blineStart, int blineEnd)
{
   m_private->m_body_startLine = blineStart;
   m_private->m_body_endLine   = blineEnd;
}

void Definition::setBodyDef(QSharedPointer<FileDef> fd)
{
   m_private->m_body_fileDef = fd;
}

bool Definition::hasSources() const
{
   return m_private->m_body_fileDef &&
         m_private->m_body_startLine != -1 && m_private->m_body_endLine >= m_private->m_body_startLine;
}

// Write code of this definition into the documentation
void Definition::writeInlineCode(OutputList &ol, const QString &scopeName)
{
   static const bool inlineSources = Config::getBool("inline-source");

   QSharedPointer<Definition> self = sharedFrom(this);

   ol.pushGeneratorState();

   if (inlineSources && hasSources()) {
      QString codeFragment;

      int actualStart = m_private->m_body_startLine;
      int actualEnd   = m_private->m_body_endLine;

      if (readCodeFragment(m_private->m_body_fileDef->getFilePath(), actualStart, actualEnd, codeFragment) ) {

         ParserInterface *pIntf = Doxy_Globals::parserManager.getParser(m_private->defFileExt);
         pIntf->resetCodeParserState();

         QSharedPointer<MemberDef> thisMd;

         if (definitionType() == TypeMember) {
            thisMd = self.dynamicCast<MemberDef>();
         }

         ol.startCodeFragment("DoxyCode");
         pIntf->parseCode(ol, scopeName, codeFragment, m_private->lang, false, QString(),
                  m_private->m_body_fileDef, actualStart, actualEnd, true, thisMd, true);

         ol.endCodeFragment("DoxyCode");
      }
   }

   ol.popGeneratorState();
}

// write a reference to the source code fragments in which this definition is used
void Definition::_writeSourceRefList(OutputList &ol, const QString &scopeName,
                  const QString &text, const MemberSDict &members)
{
   static const bool sourceCode        = Config::getBool("source-code");
   static const bool docbookSourceCode = Config::getBool("docbook-program-listing");
   static const bool latexSourceCode   = Config::getBool("latex-source-code");
   static const bool refLinkSource     = Config::getBool("ref-link-source");
   static const bool rtfSourceCode     = Config::getBool("rtf-source-code");

   ol.pushGeneratorState();

   if (! members.isEmpty()) {

      ol.startParagraph("reference");
      ol.parseText(text);
      ol.docify(" ");

      auto iter = members.begin();

      //
      QString ldefLine = theTranslator->trWriteList(members.count());

      static QRegularExpression regExp_marker("@[0-9]+");

      QString::const_iterator current_iter = ldefLine.constBegin();
      QString::const_iterator start_iter   = ldefLine.constBegin();

      QRegularExpressionMatch match = regExp_marker.match(ldefLine);

      // now replace all markers in inheritLine with links to the classes
      while (match.hasMatch()) {

         start_iter = match.capturedStart();

         ol.parseText(QStringView(current_iter, start_iter));

         QSharedPointer<MemberDef> md = iter.value();

         if (md) {
            QString scope = md->getScopeString();
            QString name  = md->name();

            if (! scope.isEmpty() && scope != scopeName) {
               name.prepend(scope + getLanguageSpecificSeparator(m_private->lang));
            }

            if (! md->isObjCMethod() && (md->isFunction() || md->isSlot() || md->isPrototype() || md->isSignal() )) {
               name += "()";
            }

            if (sourceCode && ! (md->isLinkable() && !refLinkSource) && md->getStartBodyLine() != -1 && md->getBodyDef()) {
               // for HTML write a real link

               ol.pushGeneratorState();
               // ol.disableAllBut(OutputGenerator::Html);

               ol.disable(OutputGenerator::Man);

               if (! latexSourceCode) {
                  ol.disable(OutputGenerator::Latex);
               }

               if (! docbookSourceCode) {
                 ol.disable(OutputGenerator::Docbook);
               }

               if (! rtfSourceCode) {
                  ol.disable(OutputGenerator::RTF);
               }

               QString anchorStr;
               anchorStr = QString("l%1").formatArg(md->getStartBodyLine(), 5, 10, QChar('0'));

               ol.writeObjectLink(QString(), md->getBodyDef()->getSourceFileBase(), anchorStr, name);
               ol.popGeneratorState();

               // for the other output formats just mention the name
               ol.pushGeneratorState();
               ol.disable(OutputGenerator::Html);

               if (latexSourceCode) {
                  ol.disable(OutputGenerator::Latex);
               }

               if (docbookSourceCode) {
                 ol.disable(OutputGenerator::Docbook);
               }

               if (rtfSourceCode) {
                  ol.disable(OutputGenerator::RTF);
               }

               ol.docify(name);
               ol.popGeneratorState();

            } else if (md->isLinkable()) {
               // for HTML write a real link
               ol.pushGeneratorState();

               // ol.disableAllBut(OutputGenerator::Html);
               ol.disable(OutputGenerator::Man);

               if (! latexSourceCode) {
                  ol.disable(OutputGenerator::Latex);
               }

              if (! docbookSourceCode) {
                 ol.disable(OutputGenerator::Docbook);
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

               if (docbookSourceCode) {
                 ol.disable(OutputGenerator::Docbook);
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

         current_iter = match.capturedEnd();
         match = regExp_marker.match(ldefLine, current_iter);
      }

      ol.parseText(QStringView(current_iter, ldefLine.constEnd()));

      ol.writeString(".");
      ol.endParagraph();
   }

   ol.popGeneratorState();
}

void Definition::writeSourceReffedBy(OutputList &ol, const QString &scopeName)
{
   static const bool referencedByRelation = Config::getBool("ref-by-relation");

   if (referencedByRelation) {
      _writeSourceRefList(ol, scopeName, theTranslator->trReferencedBy(), m_private->m_sourceRefByDict);
   }
}

void Definition::writeSourceRefs(OutputList &ol, const QString &scopeName)
{
   static const bool referencesRelation = Config::getBool("ref-relation");

   if (referencesRelation) {
      _writeSourceRefList(ol, scopeName, theTranslator->trReferences(), m_private->m_sourceRefsDict);
   }
}

bool Definition::hasDocumentation() const
{
   static const bool extractAll = Config::getBool("extract-all");

   bool hasDocs = (! m_private->m_details.doc.isEmpty()    || ! m_private->m_brief.doc.isEmpty() ||
                   ! m_private->m_inbodyDocs.doc.isEmpty() || extractAll);

   return hasDocs;
}

bool Definition::hasUserDocumentation() const
{
   bool hasDocs = (! m_private->m_details.doc.isEmpty() || ! m_private->m_brief.doc.isEmpty() ||
                   ! m_private->m_inbodyDocs.doc.isEmpty() );

   return hasDocs;
}

void Definition::addSourceReferencedBy(QSharedPointer<MemberDef> md)
{
   if (md) {
      QString name  = md->name();
      QString scope = md->getScopeString();

      if (! scope.isEmpty()) {
         name.prepend(scope + "::");
      }

      if (m_private->m_sourceRefByDict.find(name) == nullptr) {
         m_private->m_sourceRefByDict.insert(name, md);
      }
   }
}

void Definition::addSourceReferences(QSharedPointer<MemberDef> md)
{
   if (md) {
      QString name  = md->name();
      QString scope = md->getScopeString();

      if (! scope.isEmpty()) {
         name.prepend(scope + "::");
      }

      if (m_private->m_sourceRefsDict.find(name) == nullptr) {
         m_private->m_sourceRefsDict.insert(name, md);
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
         return QString("");

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
   if (m_private->partOfGroups == nullptr) {
      m_private->partOfGroups = new SortedList<QSharedPointer<GroupDef>>;
   }

   m_private->partOfGroups->append(gd);
}

const QVector<ListItemInfo> &Definition::getRefItems() const
{
   return m_private->m_xrefListItems;
}

QVector<ListItemInfo> &Definition::getRefItems()
{
   return m_private->m_xrefListItems;
}

void Definition::setRefItems(const QVector<ListItemInfo> &list)
{
   m_private->m_xrefListItems = list;
}

void Definition::mergeRefItems(QSharedPointer<Definition> def)
{
   QVector<ListItemInfo> &xrefList = def->getRefItems();

   for (auto &item_A : xrefList) {

      bool found = false;

      for (auto &item_B : m_private->m_xrefListItems) {
         if (item_A.type == item_B.type && item_A.itemId == item_B.itemId) {

            found = true;
            break;
         }
      }

      if (! found) {
         m_private->m_xrefListItems.append(item_A);
      }

   }
}

int Definition::_getXRefListId(const QString &listName) const
{
   for (auto &item : m_private->m_xrefListItems) {
      if (item.type == listName) {
         return item.itemId;
      }
   }

   return -1;
}

QString Definition::convertNameToFile(const QString &name, bool allowDots) const
{
   if (! m_private->ref.isEmpty()) {
      return name;

   } else {
      return convertNameToFile_internal(name, allowDots);

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

         if (tName.endsWith("-p")) {
            tName.chop(2);
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
void Definition::writeToc(OutputList &ol, const LocalToc &localToc) const
{
   if (m_private->m_sectionList.isEmpty()) {
      return;
   }

   if (localToc.isHtmlEnabled()) {

      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);
      ol.writeString("<div class=\"toc\">");
      ol.writeString("<h3>");
      ol.writeString(theTranslator->trRTFTableOfContents());
      ol.writeString("</h3>\n");
      ol.writeString("<ul>");

      int maxLevel = localToc.htmlLevel();
      int level    = 1;

      bool activeLevel[5] = { false, false, false, false, false };

      QString cs;

      // by definition order
      for (auto si : m_private->m_sectionList) {

         if (si->type == SectionInfo::Section || si->type == SectionInfo::Subsection    ||
               si->type == SectionInfo::Subsubsection || si->type == SectionInfo::Paragraph) {

            int nextLevel = si->type;

            if (nextLevel > level) {
               for (int index = level; index < nextLevel; ++index) {
                  if (index < maxLevel) {
                     ol.writeString("<ul>");
                  }
               }

            } else if (nextLevel < level) {
               for (int index = level; index > nextLevel; --index) {
                  if (index <= maxLevel && activeLevel[index]) {
                     ol.writeString("</li>\n");
                  }

                  activeLevel[index] = false;
                  if (index <= maxLevel) {
                     ol.writeString("</ul>\n");
                  }
               }
            }

            cs = QString::number(nextLevel);

            if (nextLevel <= maxLevel && activeLevel[nextLevel]) {
               ol.writeString("</li>\n");
            }

            QString titleDoc = convertToHtml(si->title);
            if (nextLevel <= maxLevel) {
               ol.writeString("<li class=\"level" + cs + "\"><a href=\"#" + si->label + "\">" + (si->title.isEmpty() ? si->label : titleDoc) + "</a>");
            }

            activeLevel[nextLevel] = true;
            level = nextLevel;
         }
      }

      if (level > maxLevel) {
         level = maxLevel;
      }

      while (level > 1 && level <= maxLevel) {
         if (activeLevel[level]) {
            ol.writeString("</li>\n");
         }

         activeLevel[level] = false;
         ol.writeString("</ul>\n");
         --level;
      }

      if (level <= maxLevel && activeLevel[level])  {
         ol.writeString("</li>\n");
      }

      activeLevel[level] = false;
      ol.writeString("</ul>\n");
      ol.writeString("</div>\n");
      ol.popGeneratorState();
   }

   if (localToc.isDocbookEnabled()) {
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Docbook);

      ol.writeString("    <toc>\n");
      ol.writeString("    <title>" + theTranslator->trRTFTableOfContents() + "</title>\n");

      int maxLevel = localToc.docbookLevel();
      int level    = 1;

      for (auto si : m_private->m_sectionList) {
         if (si->type == SectionInfo::Section || si->type == SectionInfo::Subsection    ||
               si->type == SectionInfo::Subsubsection || si->type == SectionInfo::Paragraph) {

            int nextLevel = si->type;

            if (nextLevel > level) {
               for (int index = level; index < nextLevel; ++index) {

                  if (index < maxLevel) {
                     ol.writeString("    <tocdiv>\n");
                  }
               }

            } else if (nextLevel < level) {
               for (int index = level; index > nextLevel; --index) {

                  if (index <= maxLevel) {
                    ol.writeString("    </tocdiv>\n");
                  }
               }
            }

            if (nextLevel <= maxLevel) {
               QString titleDoc = convertToDocBook(si->title);
               ol.writeString("      <tocentry>" + (si->title.isEmpty() ? si->label:titleDoc) + "</tocentry>\n");
            }

            level = nextLevel;
         }
      }

      if (level > maxLevel) {
         level = maxLevel;
      }

      while (level>1 && level <= maxLevel) {
         ol.writeString("</tocdiv>\n");
         --level;
       }

       ol.writeString("    </toc>\n");
       ol.popGeneratorState();
   }

   if (localToc.isLatexEnabled()) {
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Latex);
      int maxLevel = localToc.latexLevel();

      ol.writeString("\\etocsetnexttocdepth{" + QString::number(maxLevel) + "}\n");

      ol.writeString("\\localtableofcontents\n");
      ol.popGeneratorState();
   }
}

SectionDict &Definition::getSectionDict() const
{
  return m_private->m_sectionDict;
}

QString Definition::phraseName() const
{
   return m_phraseName;
}

QString Definition::documentation() const
{
   return m_private->m_details.doc;
}

int Definition::docLine() const
{
   return m_private->m_details.line;
}

QString Definition::docFile() const
{
   QString retval = m_private->m_details.file;

   if (retval.isEmpty() ) {
      retval = "<" + m_name + ">";
   }

   return retval;
}

// remove some text from the brief description
QString abbreviate(const QString &brief, const QString &name)
{
   static const QStringList abbreviateBrief = Config::getList("abbreviate-brief");

   QString className = name;
   QString::const_iterator iter = className.lastIndexOfFast("::");

   if (iter != className.constEnd()) {
      className =  QStringView(iter + 2, className.constEnd());
   }

   QString result = brief.trimmed();

   // strip trailing period
   if (result.endsWith('.')) {
      result.chop(1);
   }

   // strip any predefined prefix
   for (auto prefix : abbreviateBrief) {
      // replace $name with entity name
      prefix.replace("$name", className);
      prefix += " ";

      if (result.startsWith(prefix)) {
         result = result.mid(prefix.length());

      } else if (prefix.contains("<")) {
         // brief has no <T> where as className does
         static QRegularExpression regexp("<.*>");
         prefix.replace(regexp, "");

         if (result.startsWith(prefix)) {
            result = result.mid(prefix.length());
         }
      }
   }

   if (! result.isEmpty()) {
      result = upperCaseFirstLetter(std::move(result));
   }

   return result;
}

QString Definition::briefDescription(bool doAbbreviate) const
{
   QString retval;

   if (! m_private->m_brief.doc.isEmpty())  {

      if (doAbbreviate) {
         retval = abbreviate(m_private->m_brief.doc, displayName());

      } else {
         retval = m_private->m_brief.doc;

      }
   }

   return retval;
}

QString Definition::briefDescriptionAsTooltip() const
{
   QSharedPointer<const Definition> self = sharedFrom(this);

   if (m_private->m_brief.tooltip.isEmpty() && ! m_private->m_brief.doc.isEmpty()) {
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

         m_private->m_brief.tooltip = parseCommentAsText(scope, md, m_private->m_brief.doc,
               m_private->m_brief.file, m_private->m_brief.line);

         reentering = false;
      }
   }

   return m_private->m_brief.tooltip;
}

int Definition::briefLine() const
{
   return m_private->m_brief.line;
}

QString Definition::briefFile() const
{
   QString retval = m_private->m_brief.file;

   if (retval.isEmpty() ) {
      retval = "<" + m_name + ">";
   }

   return retval;
}

QString Definition::inbodyDocumentation() const
{
   return m_private->m_inbodyDocs.doc;
}

int Definition::inbodyLine() const
{
   return m_private->m_inbodyDocs.line;
}

QString Definition::inbodyFile() const
{
   QString retval = m_private->m_inbodyDocs.file;

   if (retval.isEmpty() ) {
      retval = "<" + m_name + ">";
   }

   return retval;
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
   return m_private->m_body_startLine;
}

int Definition::getEndBodyLine() const
{
   return m_private->m_body_endLine;
}

QSharedPointer<FileDef> Definition::getBodyDef() const
{
   return m_private->m_body_fileDef;
}

SortedList<QSharedPointer<GroupDef>> *Definition::partOfGroups() const
{
   return m_private->partOfGroups;
}

bool Definition::isLinkableViaGroup() const
{
   SortedList<QSharedPointer<GroupDef>> *gl = partOfGroups();

   if (gl) {
      for (auto gd : *gl) {
         if (gd->isLinkable()) {
            return true;
         }
      }
   }

  return false;
}

QSharedPointer<Definition> Definition::findInnerCompound(const QString &) const
{
   return QSharedPointer<Definition>();
}

QSharedPointer<Definition> Definition::getOuterScope() const
{
   return m_private->outerScope;
}

const MemberSDict &Definition::getReferencesMembers() const
{
   return m_private->m_sourceRefsDict;
}

const MemberSDict &Definition::getReferencedByMembers() const
{
   return m_private->m_sourceRefByDict;
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
   static const bool briefMemberDesc = Config::getBool("brief-member-desc");

   return ! briefDescription().isEmpty() && briefMemberDesc;
}

