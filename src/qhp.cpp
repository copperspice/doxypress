/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim 
 * Copyright (C) 2008 by Sebastian Pipping.
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

#include <QStringList>
#include <QFile>

#include <string.h>

#include <qhp.h>

#include <config.h>
#include <doxy_globals.h>
#include <filedef.h>
#include <message.h>
#include <qhpxmlwriter.h>

static QString makeFileName(const QString &withoutExtension)
{
   QString result = withoutExtension;

   if (! result.isEmpty()) {
      if (result.at(0) == '!') { 
         // relative URL -> strip marker
         result = result.mid(1);

      } else { 
         // add specified HTML extension
         result += Doxy_Globals::htmlFileExtension;
      }
   }

   return result;
}

static QString makeRef(const QString &withoutExtension, const QString &anchor)
{
   QString retval;

   if (! withoutExtension.isEmpty()) {
      return retval;
   }

   retval = makeFileName(withoutExtension);

   if (! anchor.isEmpty()) {
      return retval;
   }

   return retval + "#" + anchor;
}

Qhp::Qhp() : m_prevSectionLevel(0), m_sectionLevel(0), m_skipMainPageSection(false)
{
   m_doc.setIndentLevel(0);
   m_toc.setIndentLevel(2);
   m_index.setIndentLevel(2);
   m_files.setIndentLevel(2);
}

Qhp::~Qhp()
{
   clearPrevSection();
}

void Qhp::initialize()
{   
   QString nameSpace     = Config::getString("qhp-namespace");
   QString virtualFolder = Config::getString("qhp-virtual-folder");
 
   QMap<QString, QString> rootAttributes;
   rootAttributes.insert("version", "1.0");

   m_doc.declaration("1.0", "UTF-8"); 
   m_doc.open("QtHelpProject", rootAttributes);
   m_doc.openCloseContent("namespace", nameSpace);
   m_doc.openCloseContent("virtualFolder", virtualFolder);

   // Add custom filter
   QString filterName = Config::getString("qhp-cust-filter-name");

   if (! filterName.isEmpty()) {
    
      QMap<QString, QString> tagAttributes;
      tagAttributes.insert("name", filterName);

      m_doc.open("customFilter", tagAttributes);

      //
      QStringList customFilterAttributes = Config::getList("qhp-cust-attrib");

      for (int i = 0; i < customFilterAttributes.count(); i++) {
         m_doc.openCloseContent("filterAttribute", customFilterAttributes[i]);
      }

      m_doc.close("customFilter");
   }

   m_doc.open("filterSection");

   // Add section attributes
   QStringList sectionFilterAttributes = Config::getList("qhp-sect-attrib");

   for (int i = 0; i < sectionFilterAttributes.count(); i++) {
      m_doc.openCloseContent("filterAttribute", sectionFilterAttributes[i]);
   }

   m_toc.open("toc");

   // Add extra root node
   QString fullProjectName = getFullProjectName();
   QString indexFile       = "index" + Doxy_Globals::htmlFileExtension;
 
   QMap<QString, QString> attributes;
   attributes.insert("title", fullProjectName);
   attributes.insert("ref", indexFile);

   m_toc.open("section", attributes);

   m_prevSectionTitle = fullProjectName;
   m_prevSectionLevel = 1;
   m_sectionLevel     = 1;

   m_index.open("keywords");
   m_files.open("files");
}

void Qhp::finalize()
{
   // Finish TOC
   handlePrevSection();

   for (int i = m_prevSectionLevel; i > 0; i--) {
      m_toc.close("section");
   }

   m_toc.close("toc");
   m_doc.insert(m_toc);

   // Finish index
   m_index.close("keywords");
   m_doc.insert(m_index);

   // Finish files
   m_files.close("files");
   m_doc.insert(m_files);

   m_doc.close("filterSection");
   m_doc.close("QtHelpProject");

   QString fileName = Config::getString("html-output") + "/" + getQhpFileName();
   QFile file(fileName);

   if (! file.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", qPrintable(fileName), file.error());
      Doxy_Work::stopDoxyPress();
   }

   m_doc.dumpTo(file);
}

void Qhp::incContentsDepth()
{
   m_sectionLevel++;
}

void Qhp::decContentsDepth()
{
   if (m_sectionLevel <= 0 || (m_sectionLevel == 1 && m_skipMainPageSection)) {
      m_skipMainPageSection = false;
      return;
   }

   m_sectionLevel--;
}

void Qhp::addContentsItem(bool, const QString &name, const QString &, const QString &file,
                          const QString &anchor, bool unused, QSharedPointer<Definition>)
{
   // Backup difference before modification
   QString f = file;

   if (! f.isEmpty() && f.at(0) == '^') {
      return;   // absolute URL not supported
   }

   int diff = m_prevSectionLevel - m_sectionLevel;

   handlePrevSection();
   setPrevSection(name, f, anchor, m_sectionLevel);

   // close sections as needed  
   for (; diff > 0; diff--) {
      m_toc.close("section");
   }
}

void Qhp::addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md,
                       const QString &sectionAnchor, const QString &word)
{
   if (md) { 
      // member
      static bool separateMemberPages = Config::getBool("separate-member-pages");

      if (context == 0) { 
         // global member

         if (md->getGroupDef()) {
            context = md->getGroupDef();

         } else if (md->getFileDef()) {
            context = md->getFileDef();

         }
      }

      if (context == 0) {
         // should not happen
         return;   
      }

      QString cfname  = md->getOutputFileBase();
      QString cfiname = context->getOutputFileBase();

      QString level1  = context->name();
      QString level2;

      if (word.isEmpty()) {
         level2 = md->name();

      } else {
         level2 = word;         
      }

      QString contRef;

      if (separateMemberPages) { 
         contRef = cfname;
      } else {
         contRef = cfiname;
      }

      QString anchor;

      if (! sectionAnchor.isEmpty()) {
         anchor = sectionAnchor;

      } else {
         anchor =md->anchor();

      }

      QString ref = makeRef(contRef, anchor);
      QString id  = level1 + "::" + level2;

      QMap<QString, QString> attributes;
      attributes.insert("name", level2);
      attributes.insert("id",   id);
      attributes.insert("ref",  ref);

      m_index.openClose("keyword", attributes);

   } else if (context) { 
      // container
      // <keyword name="Foo" id="Foo" ref="doc.html#Foo"/>

      QString contRef = context->getOutputFileBase();     
      QString level1;

      if (! word.isEmpty()) {
         level1 = word;

      } else {
         level1 = context->name();

      }

      QString ref = makeRef(contRef, sectionAnchor);
     
      QMap<QString, QString> attributes;
      attributes.insert("name", level1);
      attributes.insert("id",   level1);
      attributes.insert("ref",  ref);

      m_index.openClose("keyword", attributes);
   }
}

void Qhp::addIndexFile(const QString &name)
{
   addFile(name);
}

QString Qhp::getQhpFileName()
{
   return "index.qhp";
}

QString Qhp::getFullProjectName()
{
   QString projectName    = Config::getString("project-name");
   QString projectVersion = Config::getString("project-version");

   if (projectName.isEmpty()) {
      projectName = "Root";
   }

   if (projectVersion.isEmpty()) { 
      return projectName; 
   } else {
     return projectName + " " + projectVersion;
   } 
}

void Qhp::handlePrevSection()
{
   if (m_prevSectionTitle.isNull()) {
      m_prevSectionTitle = " "; 
   }

   // skip "Main Page" as our extra root is pointing to that
   if (! ((m_prevSectionLevel == 1) && (m_prevSectionTitle == getFullProjectName()))) {
      QString finalRef = makeRef(m_prevSectionBaseName, m_prevSectionAnchor);
   
      QMap<QString, QString> attributes;
      attributes.insert("title", m_prevSectionTitle);
      attributes.insert("ref", finalRef);

      if (m_prevSectionLevel < m_sectionLevel) {
         // Section with children
         m_toc.open("section", attributes);

      } else {
         // Section without children
         m_toc.openClose("section", attributes);
      }

   } else {
      m_skipMainPageSection = true;
   }

   clearPrevSection();
}

void Qhp::setPrevSection(const QString &title, const QString &basename, const QString &anchor, int level)
{
   m_prevSectionTitle    = title;
   m_prevSectionBaseName = basename;
   m_prevSectionAnchor   = anchor;
   m_prevSectionLevel    = level;
}

void Qhp::clearPrevSection()
{
   m_prevSectionTitle = "";
   m_prevSectionBaseName.resize(0);
   m_prevSectionAnchor.resize(0);
}

void Qhp::addFile(const QString &fileName)
{
   m_files.openCloseContent("file", fileName);
}

void Qhp::addImageFile(const QString &fileName)
{
   addFile(fileName);
}

void Qhp::addStyleSheetFile(const QString &fileName)
{
   addFile(fileName);
}

