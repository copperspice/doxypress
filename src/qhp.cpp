/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch. 
 * Copyright (C) 2008 by Sebastian Pipping.
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

#include <QStringList>
#include <QFile>

#include <string.h>

#include <qhp.h>
#include <qhpxmlwriter.h>
#include <message.h>
#include <config.h>
#include <memberdef.h>
#include <groupdef.h>
#include <doxygen.h>
#include <filedef.h>

// must appear after the previous include - resolve soon 
#include <doxy_globals.h>

static QByteArray makeFileName(const char *withoutExtension)
{
   QByteArray result = withoutExtension;
   if (!result.isEmpty()) {
      if (result.at(0) == '!') { // relative URL -> strip marker
         result = result.mid(1);
      } else { // add specified HTML extension
         result += Doxygen::htmlFileExtension;
      }
   }
   return result;
}

static QByteArray makeRef(const char *withoutExtension, const char *anchor)
{
   //printf("QHP::makeRef(%s,%s)\n",withoutExtension,anchor);
   if (!withoutExtension) {
      return QByteArray();
   }
   QByteArray result = makeFileName(withoutExtension);
   if (!anchor) {
      return result;
   }
   return result + "#" + anchor;
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
   /*
   <QtHelpProject version="1.0">
     <namespace>mycompany.com.myapplication.1_0</namespace>
     <virtualFolder>doc</virtualFolder>
     <customFilter name="My Application 1.0">
       <filterAttribute>myapp</filterAttribute>
       <filterAttribute>1.0</filterAttribute>
     </customFilter>
     <filterSection>
       <filterAttribute>myapp</filterAttribute>
       <filterAttribute>1.0</filterAttribute>
   ..
   */
   QByteArray nameSpace       = Config_getString("QHP_NAMESPACE");
   QByteArray virtualFolder   = Config_getString("QHP_VIRTUAL_FOLDER");

   m_doc.declaration("1.0", "UTF-8");

   const char *rootAttributes[] =
   { "version", "1.0", 0 };

   m_doc.open("QtHelpProject", rootAttributes);
   m_doc.openCloseContent("namespace", nameSpace);
   m_doc.openCloseContent("virtualFolder", virtualFolder);

   // Add custom filter
   QByteArray filterName = Config_getString("QHP_CUST_FILTER_NAME");

   if (! filterName.isEmpty()) {
      const char *tagAttributes[] = { "name", filterName, 0 };
      m_doc.open("customFilter", tagAttributes);

      QStringList customFilterAttributes = QString(Config_getString("QHP_CUST_FILTER_ATTRS")).split(QChar(' '));

      for (int i = 0; i < (int)customFilterAttributes.count(); i++) {
         m_doc.openCloseContent("filterAttribute", customFilterAttributes[i].toUtf8());
      }

      m_doc.close("customFilter");
   }

   m_doc.open("filterSection");

   // Add section attributes
   QString temp = Config_getString("QHP_SECT_FILTER_ATTRS");
   QStringList sectionFilterAttributes = temp.split(QChar(' '));

   if (! sectionFilterAttributes.contains(QString("doxygen"))) {
      sectionFilterAttributes << "doxygen";
   }

   for (int i = 0; i < (int)sectionFilterAttributes.count(); i++) {
      m_doc.openCloseContent("filterAttribute", sectionFilterAttributes[i].toUtf8());
   }

   m_toc.open("toc");

   // Add extra root node
   QByteArray fullProjectname = getFullProjectName();
   QByteArray indexFile = "index" + Doxygen::htmlFileExtension;

   const char *const attributes[] = {
      "title", fullProjectname,
      "ref",   indexFile,
      NULL
   };

   m_toc.open("section", attributes);
   m_prevSectionTitle = getFullProjectName();
   m_prevSectionLevel = 1;
   m_sectionLevel = 1;

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

   QByteArray fileName = Config_getString("HTML_OUTPUT") + "/" + getQhpFileName();
   QFile file(fileName);
   if (!file.open(QIODevice::WriteOnly)) {
      err("Could not open file %s for writing\n", fileName.data());
      exit(1);
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

void Qhp::addContentsItem(bool /*isDir*/, const QString &name, const char * /*ref*/, const char *file,
                          const char *anchor, bool /* separateIndex */, bool /* addToNavIndex */, Definition * /*def*/)
{
   // Backup difference before modification
   QByteArray f = file;

   if (!f.isEmpty() && f.at(0) == '^') {
      return;   // absolute URL not supported
   }

   int diff = m_prevSectionLevel - m_sectionLevel;

   handlePrevSection();
   setPrevSection(name, f, anchor, m_sectionLevel);

   // Close sections as needed  
   for (; diff > 0; diff--) {
      m_toc.close("section");
   }
}

void Qhp::addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md,
                       const char *sectionAnchor, const char *word)
{
   if (md) { // member
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
      QByteArray level2  = word ? QByteArray(word) : md->name();
      QByteArray contRef = separateMemberPages ? cfname : cfiname;
      QByteArray anchor  = sectionAnchor ? QByteArray(sectionAnchor) : md->anchor();

      QByteArray ref;

      // <keyword name="foo" id="MyApplication::foo" ref="doc.html#foo"/>
      ref = makeRef(contRef, anchor);
      QByteArray id = level1 + "::" + level2;

      const char *attributes[] = {
         "name", level2,
         "id",   id,
         "ref",  ref,
         0};

      m_index.openClose("keyword", attributes);

   } else if (context) { // container
      // <keyword name="Foo" id="Foo" ref="doc.html#Foo"/>
      QByteArray contRef = context->getOutputFileBase();
      QByteArray level1  = word ? QByteArray(word) : context->name();
      QByteArray ref = makeRef(contRef, sectionAnchor);
      const char *attributes[] = {
         "name", level1,
         "id",   level1,
         "ref",  ref,
         0
      };
      m_index.openClose("keyword", attributes);
   }
}

void Qhp::addIndexFile(const char *name)
{
   addFile(name);
}

QByteArray Qhp::getQhpFileName()
{
   return "index.qhp";
}

QByteArray Qhp::getFullProjectName()
{
   QByteArray projectName = Config_getString("PROJECT_NAME");
   QByteArray versionText = Config_getString("PROJECT_NUMBER");
   if (projectName.isEmpty()) {
      projectName = "Root";
   }
   return projectName + (versionText.isEmpty()
                         ? QByteArray("")
                         : QByteArray(" ") + versionText);
}

void Qhp::handlePrevSection()
{
   /*
   <toc>
     <section title="My Application Manual" ref="index.html">
       <section title="Chapter 1" ref="doc.html#chapter1"/>
       <section title="Chapter 2" ref="doc.html#chapter2"/>
       <section title="Chapter 3" ref="doc.html#chapter3"/>
     </section>
   </toc>
   */

   if (m_prevSectionTitle.isNull()) {
      m_prevSectionTitle = " "; // should not happen...
   }

   // We skip "Main Page" as our extra root is pointing to that
   if (!((m_prevSectionLevel == 1) && (m_prevSectionTitle == getFullProjectName()))) {
      QByteArray finalRef = makeRef(m_prevSectionBaseName, m_prevSectionAnchor);

      const char *const attributes[] = {
         "title", m_prevSectionTitle,
         "ref",   finalRef,
         NULL
      };

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

void Qhp::setPrevSection(const QString &title, const char *basename, const char *anchor, int level)
{
   m_prevSectionTitle = title.toUtf8();
   m_prevSectionBaseName = basename;
   m_prevSectionAnchor = anchor;
   m_prevSectionLevel = level;
}

void Qhp::clearPrevSection()
{
   m_prevSectionTitle.resize(0);
   m_prevSectionBaseName.resize(0);
   m_prevSectionAnchor.resize(0);
}

void Qhp::addFile(const char *fileName)
{
   m_files.openCloseContent("file", fileName);
}

void Qhp::addImageFile(const char *fileName)
{
   addFile(fileName);
}

void Qhp::addStyleSheetFile(const char *fileName)
{
   addFile(fileName);
}

