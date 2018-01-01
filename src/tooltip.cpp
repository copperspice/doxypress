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

#include <QHash>

#include <tooltip.h>

#include <config.h>
#include <definition.h>
#include <doxy_globals.h>
#include <outputgen.h>
#include <util.h>

TooltipManager *TooltipManager::s_theInstance = 0;

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
   QString res = file;

   for (auto &c : res) {
 
      if (! isId(c.unicode())) {
         c = '_';
      }     
   }

   return res;
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

   if (! m_tooltipInfo.contains(id)) {
      m_tooltipInfo.insert(id, d);
   }
}

void TooltipManager::writeTooltips(CodeOutputInterface &ol)
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
         decl = md->declaration();

         if (! decl.isEmpty() && decl.at(0) == '@') {
            // hide enum values
            decl.resize(0);
         }
      }

      ol.writeTooltip(qPrintable(iter.key()), docInfo, decl, item->briefDescriptionAsTooltip(), defInfo, declInfo);
      iter++;
   }
}
