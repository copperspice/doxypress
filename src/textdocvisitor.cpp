/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
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

#include <QDir>

#include <textdocvisitor.h>

#include <emoji_entity.h>
#include <htmlentity.h>
#include <message.h>
#include <util.h>

void TextDocVisitor::visit(DocSymbol *s)
{
   const QString res = HtmlEntityMapper::instance()->html(s->symbol());

   if (! res.isEmpty()) {
      m_t << res;

   } else {
      err("Text, Unsupported HTML entity found: %s\n", csPrintable(HtmlEntityMapper::instance()->html(s->symbol(), true)) );

   }
}

void TextDocVisitor::visit(DocEmoji *s)
{
  const QString result = EmojiEntityMapper::instance()->name(s->index());

  if (! result.isEmpty()) {
    filter(result);
  } else {
    filter(s->name());
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

/*
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
*/

         default:
            m_t << c;
      }
   }
}

