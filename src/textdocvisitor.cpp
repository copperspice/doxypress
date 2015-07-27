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

#include <QDir>

#include <textdocvisitor.h>
#include <message.h>
#include <util.h>
#include <htmlentity.h>

void TextDocVisitor::visit(DocSymbol *s)
{
   const QString res = HtmlEntityMapper::instance()->html(s->symbol());

   if (! res.isEmpty()) {
      m_t << res;

   } else {
      err("Text: Unsupported HTML-entity found: %s\n", qPrintable(HtmlEntityMapper::instance()->html(s->symbol(), true)) );

   }
}

void TextDocVisitor::filter(const QString &str)
{
   if (str.isEmpty()) {
      return;
   }

   for (auto c : str) { 
      switch (c.unicode()) {
         case '\n':
            m_t << " ";
            break;

         case '\'':
            m_t << "&#39;";
            break;

         case '"':
            m_t << "&quot;";
            break;

         case '<':
            m_t << "&lt;";
            break;

         case '>':
            m_t << "&gt;";
            break;

         case '&':
            m_t << "&amp;";
            break;

         default:
            m_t << c;
      }
   }
}

