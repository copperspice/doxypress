/************************************************************************
*
* Copyright (c) 2014-2025 Barbara Geller & Ansel Sermersheim
*
* Copyright (c) 1997-2014 Dimitri van Heesch
*
* DoxyPress is free software. You can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
* https://www.gnu.org/licenses/
*
*************************************************************************/

#include <conceptdef.h>

#include <docparser.h>
#include <doxy_globals.h>
#include <language.h>
#include <outputlist.h>
#include <util.h>

ConceptDef::ConceptDef(const QString &defFileName, int defLine, int defColumn, const QString &fullName,
            const QString &tag, const QString &tagFileName)
   : Definition(defFileName, defLine, defColumn, fullName)
{
   (void) tagFileName;

   m_fileName = convertNameToFile_internal(fullName, false, true);
   m_title    += fullName + " Concept";

   setReference(tag);
}

ConceptDef::~ConceptDef()
{
}

QString ConceptDef::getOutputFileBase() const
{
   return "concept" + m_fileName;
}


/*

void ConceptDef::writeTagFile(QTextStream &tagFile)
{
   bool found = (name() == "citelist");

   if (! found ) {
      for (auto item : Doxy_Globals::xrefLists) {

         if (item.listName() == name()) {
            found = true;
            break;
         }
      }
   }

   if (! found) {
      // not one of the generated related pages

      tagFile << "  <compound kind=\"page\">" << endl;
      tagFile << "    <name>" << name() << "</name>" << endl;
      tagFile << "    <title>" << convertToXML(title()) << "</title>" << endl;
      tagFile << "    <filename>" << convertToXML(getOutputFileBase()) << "</filename>" << endl;

      writeDocAnchorsToTagFile(tagFile);

      tagFile << "  </compound>" << endl;
   }
}

*/

QString ConceptDef::conceptDecl() const
{
   QString retval = "template<";

   const ArgumentList &argList = getTemplateArgumentList();
   int count = argList.size();

   for (const auto &item : argList) {
      retval += item.type;

      if (! item.name.isEmpty()) {
         retval += " " + item.name;
      }

      if (item.defval.length() != 0) {
         retval += " = " + item.defval;
      }

      --count;
      if (count != 0) {
         retval += ", ";
      }
   }

   retval += ">\n";

   // line 2
   retval += "concept " + name() +  " = " + getConstraint();

   return retval;
}

void ConceptDef::setIncludeFile(QSharedPointer<FileDef> fd, const QString &includeName, bool local, bool force)
{
   if ((! includeName.isEmpty() && m_incInfo.includeName.isEmpty()) || (fd != nullptr && m_incInfo.fileDef == nullptr) ) {
      m_incInfo.fileDef     = fd;
      m_incInfo.includeName = includeName;
      m_incInfo.local       = local;
   }

   if (force && ! includeName.isEmpty()) {
      m_incInfo.includeName = includeName;
      m_incInfo.local       = local;
   }
}

void ConceptDef::writeIncludeFiles(OutputList &ol)
{
   QString nm;

   if (m_incInfo.includeName.isEmpty()) {

      if (m_incInfo.fileDef) {
         nm = m_incInfo.fileDef->docName();
      }

   } else {
      nm = m_incInfo.includeName;

   }

   if (! nm.isEmpty()) {
      ol.startParagraph();
      ol.startTypewriter();
      ol.docify("#include ");

      if (m_incInfo.local) {
         ol.docify("\"");
      } else {
         ol.docify("<");
      }

      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
      ol.docify(nm);
      ol.disableAllBut(OutputGenerator::Html);
      ol.enable(OutputGenerator::Html);

      if (m_incInfo.fileDef) {
         ol.writeObjectLink(QString(), m_incInfo.fileDef->includeName(), QString(), nm);
      } else {
         ol.docify(nm);
      }

      ol.popGeneratorState();

      if (m_incInfo.local) {
         ol.docify("\"");
      } else {
         ol.docify(">");
      }

      ol.endTypewriter();
      ol.endParagraph();
   }
}

void ConceptDef::writeDocumentation(OutputList &ol)
{
   static bool generateTreeView = Config::getBool("generate-treeview");

   QSharedPointer<ConceptDef> self = sharedFrom(this);

   QString pageName  = escapeCharsInString(name(), false, true);
   QString pageTitle = title();

   startFile(ol, getOutputFileBase(), name(), pageTitle, HLI_ConceptVisible, ! generateTreeView);

   if (! generateTreeView) {
      if (getOuterScope() != Doxy_Globals::globalScope) {
         writeNavigationPath(ol);
      }

      ol.endQuickIndices();
   }

   startTitle(ol, getOutputFileBase(), self);
   ol.parseText(pageTitle);

   addGroupListToTitle(ol, self);
   endTitle(ol, getOutputFileBase(), displayName());

   //
   ol.startContents();

   Doxy_Globals::indexList.addIndexItem(self, QSharedPointer<MemberDef>());

   if (Doxy_Globals::searchIndexBase != nullptr) {
      Doxy_Globals::searchIndexBase->setCurrentDoc(self, anchor(), false);
      Doxy_Globals::searchIndexBase->addWord(localName(), true);
   }

   writeBriefDescription(ol);
   writeIncludeFiles(ol);
   writeDetailedDescription(ol, theTranslator->trDetailedDescription(), QString());

   writeAuthorSection(ol);

   ol.endContents();
   endFileWithNavPath(self, ol);
}

void ConceptDef::writeBriefDescription(OutputList &ol)
{
   QSharedPointer<ConceptDef> self = sharedFrom(this);

   if (hasBriefDescription()) {
      ol.startParagraph();

      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Man);
      ol.writeString(" - ");
      ol.popGeneratorState();

      ol.generateDoc(briefFile(), briefLine(), self, QSharedPointer<MemberDef>(),
                  briefDescription(), true, false, QString(), true, false);

      ol.pushGeneratorState();

      ol.disable(OutputGenerator::RTF);
      ol.writeString(" \n");
      ol.enable(OutputGenerator::RTF);
      ol.popGeneratorState();

      if (hasDetailedDescription()) {
         writeMoreLink(ol, anchor());
      }

      ol.endParagraph();
   }

   ol.writeSynopsis();
}

void ConceptDef::writeDetailedDescription(OutputList &ol, const QString &title, const QString &anchor)
{
   if (hasDetailedDescription()) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
      ol.writeRuler();
      ol.popGeneratorState();

      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);

      ol.writeAnchor(QString(), anchor.isEmpty() ? QString("details") : anchor);
      ol.popGeneratorState();

      if (! anchor.isEmpty()) {
         ol.pushGeneratorState();
         ol.disable(OutputGenerator::Html);
         ol.disable(OutputGenerator::Man);
         ol.writeAnchor(getOutputFileBase(), anchor);
         ol.popGeneratorState();
      }

      ol.startGroupHeader();
      ol.parseText(title);
      ol.endGroupHeader();

      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Man);
      ol.startSubsubsection();
      ol.popGeneratorState();

      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Man);
      ol.startTextBlock();
      ol.popGeneratorState();

      const ArgumentList &argList = getTemplateArgumentList();

      ol.docify("template<");

      int count = argList.size();

      for (const auto &item : argList) {
         ol.docify(item.type);

         if (! item.name.isEmpty()) {
            ol.docify(" ");
            ol.docify(item.name);
         }

         if (item.defval.length() != 0) {
            ol.docify(" = ");
            ol.docify(item.defval);
         }

         --count;
         if (count != 0) {
            ol.docify(", ");
         }
      }

      ol.docify(">");
      ol.lineBreak();

      //
      ol.docify("concept " + name() +  " = " + getConstraint());
      ol.lineBreak();

      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Man);
      ol.endSubsubsection();
      ol.popGeneratorState();

      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Man);
      ol.lineBreak();
      ol.endTextBlock();
      ol.popGeneratorState();

      writeDetailedDocumentationBody(ol);
   }
}

void ConceptDef::writeDetailedDocumentationBody(OutputList &ol)
{
   static const bool repeatBrief = Config::getBool("repeat-brief");

   QSharedPointer<ConceptDef> self = sharedFrom(this);

   const QString docText = documentation();
   ol.startTextBlock();

   // repeat brief description
   QString brief = briefDescription();

   if (! brief.isEmpty() && repeatBrief) {
      ol.generateDoc(briefFile(), briefLine(), self, QSharedPointer<MemberDef>(), brief, false, false);

      if (! docText.isEmpty()) {

         ol.pushGeneratorState();
         ol.disable(OutputGenerator::Html);

         ol.writeString("\n\n");
         ol.popGeneratorState();
      }
   }

   // write documentation
   if (! docText.isEmpty()) {
      ol.generateDoc(docFile(), docLine(), self, QSharedPointer<MemberDef>(), docText, true, false);
   }

   writeSourceDef(ol, name());
   ol.endTextBlock();
}

void ConceptDef::writeTagFile(QTextStream &tagFile)
{
  tagFile << "  <compound kind=\"concept\">\n";
  tagFile << "    <name>" << convertToXML(name()) << "</name>\n";

  QString fn = getOutputFileBase();
  addHtmlExtensionIfMissing(fn);

  tagFile << "    <filename>" << convertToXML(fn) << "</filename>\n";

  QString idStr = id();

  if (! idStr.isEmpty()) {
    tagFile << "    <clangid>" << convertToXML(idStr) << "</clangid>\n";
  }

  writeDocAnchorsToTagFile(tagFile);
  tagFile << "  </compound>\n";
}

bool ConceptDef::hasDetailedDescription() const
{
   static const bool repeatBrief = Config::getBool("repeat-brief");
   static const bool sourceCode  = Config::getBool("source-code");

   return ((! briefDescription().isEmpty() && repeatBrief) || ! documentation().isEmpty() ||
           (sourceCode && getStartBodyLine() != -1 && getBodyDef()));
}

void ConceptDef::writeAuthorSection(OutputList &ol)
{
   static const QString projectName = Config::getString("project-name");

   // write Author section (Man only)
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Man);

   ol.writeString("\n");
   ol.startGroupHeader();
   ol.parseText(theTranslator->trAuthor(true, true));
   ol.endGroupHeader();
   ol.parseText(theTranslator->trGeneratedAutomatically(projectName));

   ol.popGeneratorState();
}


void ConceptDef::writeMoreLink(OutputList &ol, const QString &anchor)
{
   static const bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static const bool rtfHyperlinks = Config::getBool("rtf-hyperlinks");
   static const bool usePDFLatex   = Config::getBool("latex-pdf");

   // HTML only
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
   ol.docify(" ");
   ol.startTextLink(getOutputFileBase(), anchor.isEmpty() ? "details" : anchor);
   ol.parseText(theTranslator->trMore());
   ol.endTextLink();
   ol.popGeneratorState();

   if (! anchor.isEmpty()) {
      ol.pushGeneratorState();

      // LaTeX + RTF
      ol.disable(OutputGenerator::Html);
      ol.disable(OutputGenerator::Man);

      if (! (usePDFLatex && pdfHyperlinks)) {
         ol.disable(OutputGenerator::Latex);
      }
      if (! rtfHyperlinks) {
         ol.disable(OutputGenerator::RTF);
      }

      ol.docify(" ");
      ol.startTextLink(getOutputFileBase(), anchor);
      ol.parseText(theTranslator->trMore());
      ol.endTextLink();

      // RTF only
      ol.disable(OutputGenerator::Latex);
      ol.writeString("\\par");
      ol.popGeneratorState();
   }
}

const ArgumentList &ConceptDef::getTemplateArgumentList() const
{
   return m_templateArgs;
}

void ConceptDef::setTemplateArgumentList(const ArgumentList &al)
{
   m_templateArgs = al;
}

void ConceptDef::writeDeclarationLink(OutputList &ol, bool &found, const QString &header, bool localNames)
{
   static const bool briefMemberDesc = Config::getBool("brief-member-desc");

   QSharedPointer<ConceptDef> self = sharedFrom(this);

   if (isLinkable()) {
      if (! found) {
         // first concept
         ol.startMemberHeader("concepts");

         if (! header.isEmpty()) {
            ol.parseText(header);

         } else {
            theTranslator->trConcepts();
         }

         ol.endMemberHeader();
         ol.startMemberList();

         found = true;
      }

      ol.startMemberDeclaration();
      ol.startMemberItem(anchor(), 0);
      ol.writeString("concept ");

      QString cname = displayName(! localNames);
      ol.insertMemberAlign();

      if (isLinkable()) {
         ol.writeObjectLink(getReference(), getOutputFileBase(), anchor(), cname);

      } else {
         ol.startBold();
         ol.docify(cname);
         ol.endBold();
      }

      ol.endMemberItem();

      // add the brief description if available
      if (! briefDescription().isEmpty() && briefMemberDesc) {

         DocRoot *rootNode = validatingParseDoc(briefFile(), briefLine(), self, QSharedPointer<MemberDef>(),
               briefDescription(), false, false, QString(), true, false);

         if (! rootNode->isEmpty()) {
            ol.startMemberDescription(anchor());
            ol.writeDoc(rootNode, self, QSharedPointer<MemberDef>());
            ol.endMemberDescription();
         }
      }

      ol.endMemberDeclaration(anchor(), QString());
   }
}

void ConceptDef::findSectionsInDocumentation()
{
   QSharedPointer<ConceptDef> self = sharedFrom(this);

   docFindSections(briefDescription(), self, QSharedPointer<MemberGroup>(), docFile());
   docFindSections(documentation(), self, QSharedPointer<MemberGroup>(), docFile());
}

bool ConceptSDict::declVisible() const
{
   static const bool hideUndocClasses = Config::getBool("hide-undoc-classes");

   for (const auto &conceptDef : *this) {
      bool isLink = conceptDef->isLinkable();

      if (isLink || ! hideUndocClasses) {
         return true;
      }
   }

   return false;
}

void ConceptSDict::writeDeclaration(OutputList &ol, const QString &header, bool localNames)
{
   bool found = false;

   for (const auto &conceptDef : *this) {
      conceptDef->writeDeclarationLink(ol, found, header, localNames);
   }
   if (found) {
      ol.endMemberList();
   }
}
