/*************************************************************************
 *
 * Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim
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

#include <QFile>

#include <stdlib.h>

#include <entry.h>

#include <arguments.h>
#include <doxy_globals.h>
#include <filestorage.h>
#include <marshal.h>
#include <section.h>
#include <util.h>

int Entry::m_EntryCount = 0;

Entry::Entry()
{
   m_EntryCount++;

   m_parent.clear();

   section      = Entry::EMPTY_SEC;
   mGrpId       = -1;
   relatesType  = Simple;
   hidden       = false;
   groupDocType = GROUPDOC_NORMAL;

   reset();
}

Entry::Entry(const Entry &e)
{
   m_EntryCount++;

   section          = e.section;
   type             = e.type;
   name             = e.name;
   m_tagInfoEntry   = e.m_tagInfoEntry;
   protection       = e.protection;
   mtype            = e.mtype;
   m_traits         = e.m_traits;
   initLines        = e.initLines;
   stat             = e.stat;
   explicitExternal = e.explicitExternal;
   proto            = e.proto;
   subGrouping      = e.subGrouping;
   callGraph        = e.callGraph;
   callerGraph      = e.callerGraph;
   virt             = e.virt;
   args             = e.args;
   bitfields        = e.bitfields;
   argList          = e.argList;

   m_program        = e.m_program;
   initializer      = e.initializer;
   includeFile      = e.includeFile;
   includeName      = e.includeName;
   doc              = e.doc;
   docLine          = e.docLine;
   docFile          = e.docFile;
   brief            = e.brief;
   briefLine        = e.briefLine;
   briefFile        = e.briefFile;
   inbodyDocs       = e.inbodyDocs;
   inbodyLine       = e.inbodyLine;
   inbodyFile       = e.inbodyFile;
   relates          = e.relates;
   relatesType      = e.relatesType;

   m_read           = e.m_read;
   m_write          = e.m_write;
   m_reset          = e.m_reset;
   m_notify         = e.m_notify;
   m_revision       = e.m_revision;
   m_designable     = e.m_designable;
   m_scriptable     = e.m_scriptable;
   m_stored         = e.m_stored;
   m_user           = e.m_user;

   inside           = e.inside;
   exception        = e.exception;
   bodyLine         = e.bodyLine;
   endBodyLine      = e.endBodyLine;
   mGrpId           = e.mGrpId;

   fileName         = e.fileName;
   startLine        = e.startLine;
   startColumn      = e.startColumn;

   m_specialLists   = e.m_specialLists;

   lang             = e.lang;
   hidden           = e.hidden;
   artificial       = e.artificial;
   groupDocType     = e.groupDocType;
   id               = e.id;
   m_parent         = e.m_parent;

   // deep copy of the child entry list
   for (auto item : e.m_sublist) {
      m_sublist.append(QMakeShared<Entry>(*item));
   }

   // copy base class list
   for (auto item : e.extends) {
      extends.append(item);
   }

   m_templateArgLists = e.m_templateArgLists;
   m_groups    = e.m_groups;
   m_anchors   = e.m_anchors;

   typeConstr  = e.typeConstr;
}

Entry::~Entry()
{
   m_EntryCount--;
}

void Entry::addSubEntry(QSharedPointer<Entry> child, QSharedPointer<Entry> self)
{
   if (self != this) {
      throw std::runtime_error("Internal Issue: passed parameter was not equal to the current object (Entry::addSubEntry)");
   }

   child->m_parent = self;
   m_sublist.append(child);
}

void Entry::reset()
{
   static const bool dotCallGraph = Config::getBool("dot-call");
   static const bool dotCalledBy  = Config::getBool("dot-called-by");

   m_tagInfoEntry.clear();
   argList.clear();
   typeConstr.clear();

   relatesType  = Simple;
   virt         = Normal;
   protection   = Public;
   mtype        = Method;
   groupDocType = GROUPDOC_NORMAL;
   lang         = SrcLangExt_Unknown;

   m_traits.clear();

   section = Entry::EMPTY_SEC;
   initLines    = -1;
   docLine      = -1;

   briefLine    = -1;
   inbodyLine   = -1;
   bodyLine     = -1;
   endBodyLine  = -1;
   mGrpId       = -1;

   startLine    = 1;
   startColumn  = 1;

   stat             = false;
   explicitExternal = false;
   proto            = false;
   subGrouping      = true;

   callGraph    = dotCallGraph;
   callerGraph  = dotCalledBy;

   type         = "";
   name         = "";
   args         = "";
   bitfields    = "";
   m_program    = "";
   initializer  = "";
   includeFile  = "";
   includeName  = "";
   doc          = "";
   docFile      = "";
   brief        = "";
   briefFile    = "";
   inbodyDocs   = "";
   inbodyFile   = "";
   relates      = "";

   // missing m_read, m_write, etc

   inside       = "";
   exception    = "";
   fileName     = "";
   id           = "";

   hidden       = false;
   artificial   = false;

   m_templateArgLists.clear();
   extends.clear();
   m_groups.clear();
   m_anchors.clear();
   m_specialLists.clear();

   m_sublist.clear();
}

int Entry::getSize()
{
   return sizeof(Entry);
}

void Entry::createSubtreeIndex(QSharedPointer<EntryNav> nav, FileStorage &storage,
                  QSharedPointer<FileDef> fd, QSharedPointer<Entry> self)
{
   assert(self == this);
   QSharedPointer<EntryNav> childNav = QMakeShared<EntryNav>(nav, self);

   nav->addChild(childNav);

   childNav->setFileDef(fd);
   childNav->saveEntry(self, storage);

   for (auto childNode : m_sublist) {
      childNode->createSubtreeIndex(childNav, storage, fd, childNode);
   }

   m_sublist.clear();
}

void Entry::createNavigationIndex(QSharedPointer<EntryNav> rootNav, FileStorage &storage,
                  QSharedPointer<FileDef> fd, QSharedPointer<Entry> self)
{
   createSubtreeIndex(rootNav, storage, fd, self);
}

void Entry::addSpecialListItem(const QString &listName, int itemId)
{
   ListItemInfo item;

   item.type   = listName;
   item.itemId = itemId;

   m_specialLists.append(item);
}

void Entry::removeSubEntry(QSharedPointer<Entry> e)
{
   // called from lex code, appears to be used when parsing Fortran only

   int i = m_sublist.indexOf(e);

   if (i != -1) {
      m_sublist.removeAt(i);
   }
}

EntryNav::EntryNav(QSharedPointer<EntryNav> parent, QSharedPointer<Entry> e)
   : m_parent(parent), m_section(e->section), m_type(e->type), m_name(e->name),
     m_fileDef(0), m_lang(e->lang), m_offset(-1), m_noLoad(false)
{

   m_tagInfoNav = e->m_tagInfoEntry;
}

EntryNav::~EntryNav()
{
}

void EntryNav::addChild( QSharedPointer<EntryNav> e)
{
   m_subList.append(e);
}

bool EntryNav::loadEntry(FileStorage &storage)
{
   if (m_noLoad) {
      return true;
   }

   if (m_offset == -1) {
      return false;
   }

   if (! storage.seek(m_offset)) {
      return false;
   }

   m_info = unmarshalEntry(&storage);

   m_info->name    = m_name;
   m_info->type    = m_type;
   m_info->section = m_section;

   return true;
}

bool EntryNav::saveEntry(QSharedPointer<Entry> e, FileStorage &storage)
{
   m_offset = storage.pos();
   marshalEntry(&storage, e);

   return true;
}

void EntryNav::releaseEntry()
{
   if (! m_noLoad) {
      m_info =  QSharedPointer<Entry>();
   }
}

void EntryNav::setEntry(QSharedPointer<Entry> e)
{
   m_info   = e;
   m_noLoad = true;
}
