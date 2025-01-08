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

#include <tooltip.h>

#include <config.h>
#include <definition.h>
#include <doxy_globals.h>
#include <outputgen.h>
#include <util.h>

TooltipManager *TooltipManager::s_theInstance = nullptr;

TooltipManager::TooltipManager()
{
}

TooltipManager::~TooltipManager()
{
}

TooltipManager *TooltipManager::instance()
{
   if (!s_theInstance) {
      s_theInstance = new TooltipManager;
   }

   return s_theInstance;
}

void TooltipManager::clearTooltips()
{
   m_tooltipInfo.clear();
}

static QString escapeId(const QString &file)
{
   QString retval;

   for (QChar c : file) {

      if (isId(c)) {
         retval.append(c);
      } else {
         retval.append('_');
      }
   }

   return retval;
}

void TooltipManager::addTooltip(QSharedPointer<Definition> d)
{
   static bool sourceTooltips = Config::getBool("source-tooltips");

   if (! sourceTooltips) {
      return;
   }

   QString id = d->getOutputFileBase();
   int i = id.lastIndexOf('/');

   if (i != -1) {
      id = id.right(id.length() - i - 1); // strip path (for CREATE_SUBDIRS=YES)
   }

   id += escapeId(Doxy_Globals::htmlFileExtension);

   QString anc = d->anchor();
   if (! anc.isEmpty()) {
      id += "_" + anc;
   }

   id.prepend("a");

   if (! m_tooltipInfo.contains(id)) {
      m_tooltipInfo.insert(id, d);
   }
}

void TooltipManager::writeTooltips(CodeGenerator &ol)
{
   QHash<QString, QSharedPointer<Definition>>::iterator iter = m_tooltipInfo.begin();

   for (auto item : m_tooltipInfo) {
      DocLinkInfo docInfo;

      docInfo.name   = item->qualifiedName();
      docInfo.ref    = item->getReference();
      docInfo.url    = item->getOutputFileBase();
      docInfo.anchor = item->anchor();

      SourceLinkInfo defInfo;

      if (item->getBodyDef() && item->getStartBodyLine() != -1) {
         defInfo.file    = item->getBodyDef()->name();
         defInfo.line    = item->getStartBodyLine();
         defInfo.url     = item->getSourceFileBase();
         defInfo.anchor  = item->getSourceAnchor();
      }

      SourceLinkInfo declInfo;
      QString decl;

      if (item->definitionType() == Definition::TypeMember) {
         QSharedPointer<MemberDef> md = item.dynamicCast<MemberDef>();
         if (! md->isAnonymous()) {
            decl = md->declaration();
         }
      }

      ol.writeTooltip(iter.key(), docInfo, decl, item->briefDescriptionAsTooltip(), defInfo, declInfo);
      iter++;
   }
}
