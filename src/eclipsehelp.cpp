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
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#include <config.h>
#include <eclipsehelp.h>
#include <doxy_globals.h>
#include <message.h>
#include <util.h>

EclipseHelp::EclipseHelp() : m_depth(0), m_endtag(false), m_openTags(0), m_tocfile(0)
{
}

EclipseHelp::~EclipseHelp()
{
}

void EclipseHelp::indent()
{
   int i;
   for (i = 0; i < m_depth; i++) {
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
   // -- read path prefix from the configuration
   // m_pathprefix = Config::getString("eclipse-pathprefix");
   // if (m_pathprefix.isEmpty()) m_pathprefix = "html/";

   // -- open the contents file
   QString name = Config::getString("html-output") + "/toc.xml";
   m_tocfile = new QFile(name);

   if (!m_tocfile->open(QIODevice::WriteOnly)) {
      err("Could not open file %s for writing\n", qPrintable(name));
      exit(1);
   }

   // -- initialize its text stream
   m_tocstream.setDevice(m_tocfile);
   
   // -- write the opening tag
   QString title = Config::getString("project-name");

   if (title.isEmpty()) {
      title = "DoxyPress generated documentation";
   }

   m_tocstream << "<toc label=\"" << convertToXML(title)
               << "\" topic=\"" << convertToXML(m_pathprefix)
               << "index" << Doxygen::htmlFileExtension << "\">" << endl;
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
   closedTag(); // -- close previous tag

   // -- write ending tag
   --m_depth;
   m_tocstream << "</toc>" << endl;

   // -- close the content file
   m_tocstream.setDevice(0);
   m_tocfile->close();
   delete m_tocfile;
   m_tocfile = 0;

   QString name = Config::getString("html-output") + "/plugin.xml";
   QFile pluginFile(name);

   if (pluginFile.open(QIODevice::WriteOnly)) {
      QString docId = Config::getString("eclipse-doc-id");

      QTextStream t(&pluginFile);
      t << "<plugin name=\""  << docId << "\" id=\"" << docId << "\"" << endl;
      t << "        version=\"1.0.0\" provider-name=\"Doxygen\">" << endl;
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
void EclipseHelp::addContentsItem(bool, const QString &name, const char *, const char *file, const char *anchor,
                  bool, bool, QSharedPointer<Definition>)
{
   // -- write the topic tag
   closedTag();

   if (file) {
      switch (file[0]) { // check for special markers (user defined URLs)
         case '^':
            // URL not supported by eclipse toc.xml
            break;

         case '!':
            indent();
            m_tocstream << "<topic label=\"" << convertToXML(name) << "\"";
            m_tocstream << " href=\"" << convertToXML(m_pathprefix) << &file[1] << "\"";
            m_endtag = true;
            break;

         default:
            indent();
            m_tocstream << "<topic label=\"" << convertToXML(name) << "\"";
            m_tocstream << " href=\"" << convertToXML(m_pathprefix)
                        << file << Doxygen::htmlFileExtension;
            if (anchor) {
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

void EclipseHelp::addIndexItem(QSharedPointer<Definition> d, QSharedPointer<MemberDef> md, const char * , const char *)
{
}

void EclipseHelp::addIndexFile(const char * /* name */)
{
}

void EclipseHelp::addImageFile(const char * /* name */)
{
}

void EclipseHelp::addStyleSheetFile(const char * /* name */)
{
}

