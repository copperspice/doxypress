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

#include <eclipsehelp.h>

#include <config.h>
#include <doxy_globals.h>
#include <message.h>
#include <util.h>

EclipseHelp::EclipseHelp() : m_depth(0), m_endtag(false), m_openTags(0)
{
}

EclipseHelp::~EclipseHelp()
{
}

void EclipseHelp::indent()
{
   for (int i = 0; i < m_depth; i++) {
      m_tocstream << "  ";
   }
}

void EclipseHelp::closedTag()
{
   if (m_endtag) {
      m_tocstream << "/>" << endl;
      m_endtag = false;
   }
}

void EclipseHelp::openedTag()
{
   if (m_endtag) {
      m_tocstream << ">" << endl;
      m_endtag = false;
      ++m_openTags;
   }
}

/*!
 * \brief Initialize the Eclipse generator
 *
 * This method opens the XML TOC file and writes headers of the files.
 * \sa finalize()
 */
void EclipseHelp::initialize()
{
   // open the contents file
   static const QString name  = Config::getString("html-output") + "/toc.xml";
   QString title = Config::getString("project-name");

   // read path prefix from the configuration
   // m_pathprefix = Config::getString("eclipse-pathprefix");
   // if (m_pathprefix.isEmpty()) m_pathprefix = "html/";

   // open the contents file
   m_tocfile.setFileName(name);

   if (! m_tocfile.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing\n", csPrintable(name));
      Doxy_Work::stopDoxyPress();
   }

   // initialize text stream
   m_tocstream.setDevice(&m_tocfile);
   
   // write the opening tag 
   if (title.isEmpty()) {
      title = "DoxyPress generated documentation";
   }

   m_tocstream << "<toc label=\"" << convertToXML(title)
               << "\" topic=\"" << convertToXML(m_pathprefix)
               << "index" << Doxy_Globals::htmlFileExtension << "\">" << endl;
   ++ m_depth;
}

/*!
 * \brief Finish generation of the Eclipse specific help files
 *
 * This method writes footers of the files and closes them.
 * \sa initialize()
 */
void EclipseHelp::finalize()
{
   static const QString name = Config::getString("html-output") + "/plugin.xml";

   // close previous tag
   closedTag(); 

   // write ending tag
   --m_depth;
   m_tocstream << "</toc>" << endl;

   // close the content file
   m_tocstream.setDevice(0);

   m_tocfile.close();  
   m_tocfile.setFileName("");
 
   QFile pluginFile(name);

   if (pluginFile.open(QIODevice::WriteOnly)) {
      QString docId = Config::getString("eclipse-doc-id");

      QTextStream t(&pluginFile);
      t << "<plugin name=\""  << docId << "\" id=\"" << docId << "\"" << endl;
      t << "        version=\"1.0.0\" provider-name=\"DoxyPress\">" << endl;
      t << "  <extension point=\"org.eclipse.help.toc\">" << endl;
      t << "    <toc file=\"toc.xml\" primary=\"true\" />" << endl;
      t << "  </extension>" << endl;
      t << "</plugin>" << endl;
   }
}

/*!
 * \brief Increase the level of content hierarchy
 */
void EclipseHelp::incContentsDepth()
{
   openedTag();
   ++m_depth;
}

/*!
 * \brief Decrease the level of content hierarchy
 *
 * It closes currently opened topic tag.
 */
void EclipseHelp::decContentsDepth()
{
   // -- end of the opened topic
   closedTag();
   --m_depth;

   if (m_openTags == m_depth) {
      --m_openTags;
      indent();
      m_tocstream << "</topic>" << endl;
   }
}

/*!
 * \brief Add an item to the content
 *
 * @param isDir Flag whether the argument \a file is a directory or a file entry
 * @param name Name of the item
 * @param ref URL of the item
 * @param file Name of a file which the item is defined in (without extension)
 * @param anchor Name of an anchor of the item.
 * @param separateIndex not used.
 * @param addToNavIndex not used.
 * @param def not used.
 */
void EclipseHelp::addContentsItem(bool, const QString &name, const QString &, const QString &file, const QString &anchor,
                  bool unused, QSharedPointer<Definition>, DirType category)
{
   // -- write the topic tag
   closedTag();

   if (! file.isEmpty() ) {

      switch (file[0].unicode()) { // check for special markers (user defined URLs)
         case '^':
            // URL not supported by eclipse toc.xml
            break;

         case '!':
            indent();
            m_tocstream << "<topic label=\"" << convertToXML(name) << "\"";
            m_tocstream << " href=\"" << convertToXML(m_pathprefix) << file.mid(1) << "\"";
            m_endtag = true;
            break;

         default:
            indent();
            m_tocstream << "<topic label=\"" << convertToXML(name) << "\"";
            m_tocstream << " href=\"" << convertToXML(m_pathprefix)
                        << file << Doxy_Globals::htmlFileExtension;

            if (! anchor.isEmpty()) {
               m_tocstream << "#" << anchor;
            }

            m_tocstream << "\"";
            m_endtag = true;
            break;
      }

   } else {
      indent();
      m_tocstream << "<topic label=\"" << convertToXML(name) << "\"";
      m_endtag = true;
   }
}

void EclipseHelp::addIndexItem(QSharedPointer<Definition> d, QSharedPointer<MemberDef> md, const QString &, const QString &)
{
}

void EclipseHelp::addIndexFile(const QString &name)
{
}

void EclipseHelp::addImageFile(const QString &name)
{
}

void EclipseHelp::addStyleSheetFile(const QString &name)
{
}

