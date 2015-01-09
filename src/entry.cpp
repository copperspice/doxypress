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
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#include <QFile>

#include <stdlib.h>

#include <arguments.h>
#include <doxygen.h>
#include <entry.h>
#include <filestorage.h>
#include <marshal.h>
#include <section.h>
#include <util.h>

int Entry::num = 0;

Entry::Entry()
{
   num++;

   m_parent = 0;
   section  = Entry::EMPTY_SEC;
  
   extends   = new QList<BaseInfo>;   
   groups    = new QList<Grouping>; 
   anchors   = new QList<SectionInfo>;    // Doxygen::sectionDict takes ownership of the items
       
   tArgLists  = 0;
   mGrpId     = -1;
   tagInfo    = 0;
   sli        = 0;

   relatesType = Simple;
   hidden      = false;

   groupDocType = GROUPDOC_NORMAL;

   reset();  
}

Entry::Entry(const Entry &e)
{  
   num++;

   section     = e.section;
   type        = e.type;
   name        = e.name;
   tagInfo     = e.tagInfo;
   protection  = e.protection;
   mtype       = e.mtype;
   spec        = e.spec;
   initLines   = e.initLines;
   stat        = e.stat;
   explicitExternal = e.explicitExternal;
   proto       = e.proto;
   subGrouping = e.subGrouping;
   callGraph   = e.callGraph;
   callerGraph = e.callerGraph;
   virt        = e.virt;
   args        = e.args;
   bitfields   = e.bitfields;

   argList     = e.argList; 
   tArgLists   = 0;     
   
   program     = e.program;
   initializer = e.initializer;
   includeFile = e.includeFile;
   includeName = e.includeName;
   doc         = e.doc;
   docLine     = e.docLine;
   docFile     = e.docFile;
   brief       = e.brief;
   briefLine   = e.briefLine;
   briefFile   = e.briefFile;
   inbodyDocs  = e.inbodyDocs;
   inbodyLine  = e.inbodyLine;
   inbodyFile  = e.inbodyFile;
   relates     = e.relates;
   relatesType = e.relatesType;
   read        = e.read;
   write       = e.write;
   inside      = e.inside;
   exception   = e.exception;   
   bodyLine    = e.bodyLine;
   endBodyLine = e.endBodyLine;
   mGrpId      = e.mGrpId;

   extends     = new QList<BaseInfo>;   
   groups      = new QList<Grouping>;  
   anchors     = new QList<SectionInfo>;

   fileName    = e.fileName;
   startLine   = e.startLine;
   startColumn = e.startColumn;

   if (e.sli) {
      sli = new QList<ListItemInfo>;          

      for (auto item : *e.sli) { 
         sli->append(item);
      }

   } else {
      sli = 0;
   }

   lang         = e.lang;
   hidden       = e.hidden;
   artificial   = e.artificial;
   groupDocType = e.groupDocType;
   id           = e.id;
   m_parent     = e.m_parent;
    
   // copy of the child entry list  
   for (auto item : e.m_sublist) { 
      m_sublist.append(new Entry(*item));
   }

   // copy base class list
   for (auto item : *e.extends) { 
      extends->append(item);
   }

   // copy group list
   for (auto item : *e.groups) { 
      groups->append(item);
   }

   for (auto item : *e.anchors) { 
      anchors->append(item);
   }

   // copy type contraint list  
   typeConstr  = e.typeConstr;       
   
   // copy template argument lists
   if (e.tArgLists) {
      tArgLists = copyArgumentLists(e.tArgLists);
   }
}

Entry::~Entry()
{
   // our children.
   delete extends;
   delete groups;
   delete anchors;  
   delete tArgLists;
   delete tagInfo;   
   delete sli;

   num--;
}

void Entry::addSubEntry(Entry *current)
{
   current->m_parent = this;

   m_sublist.append(current);
}

void Entry::reset()
{
   name.resize(0);
   type.resize(0);
   args.resize(0);
   bitfields.resize(0);
   exception.resize(0);
   program.resize(0);
   includeFile.resize(0);
   includeName.resize(0);
   doc.resize(0);
   docFile.resize(0);
   docLine = -1;
   relates.resize(0);
   relatesType = Simple;
   brief.resize(0);
   briefFile.resize(0);
   briefLine = -1;
   inbodyDocs.resize(0);
   inbodyFile.resize(0);
   inbodyLine = -1;
   inside.resize(0);
   fileName.resize(0);
   initializer.resize(0);

   initLines = -1;
   startLine = 1;
   startColumn = 1;
   bodyLine = -1;
   endBodyLine = -1;
   mGrpId = -1;

   callGraph   = false;
   callerGraph = false;

   section = Entry::EMPTY_SEC;
   mtype   = Method;
   virt    = Normal;
   stat    = false;
   proto   = false;
   explicitExternal = false;
   spec  = 0;
   lang = SrcLangExt_Unknown;
   hidden = false;
   artificial = false;
   subGrouping = true;
   protection = Public;
   groupDocType = GROUPDOC_NORMAL;
   id.resize(0);

   m_sublist.clear();

   extends->clear();
   groups->clear();
   anchors->clear();
   argList.clear();

   if (tagInfo)    {
      delete tagInfo;
      tagInfo = 0;
   }

   if (tArgLists)  {
      delete tArgLists;
      tArgLists = 0;
   }

   if (sli) {
      delete sli;
      sli = 0;
   }
   
   typeConstr = ArgumentList();     
}


int Entry::getSize()
{
   return sizeof(Entry);
}

void Entry::createSubtreeIndex(EntryNav *nav, FileStorage *storage, FileDef *fd)
{
   EntryNav *childNav = new EntryNav(nav, this);

   nav->addChild(childNav);
   childNav->setFileDef(fd);
   childNav->saveEntry(this, storage);
    
   for (auto childNode : m_sublist) {
      childNode->createSubtreeIndex(childNav, storage, fd);
   }
   
   m_sublist.clear();
   
}

void Entry::createNavigationIndex(EntryNav *rootNav, FileStorage *storage, FileDef *fd)
{
   createSubtreeIndex(rootNav, storage, fd);
}

void Entry::addSpecialListItem(const char *listName, int itemId)
{
   if (sli == 0) {
      sli = new QList<ListItemInfo>;      
   }

   ListItemInfo *ili = new ListItemInfo;
   ili->type   = listName;
   ili->itemId = itemId;

   sli->append(*ili);
}

void Entry::removeSubEntry(Entry *e)
{
   // called from lex code, appears to be used when parsing Fortran only

   int i = m_sublist.indexOf(e);

   if (i != -1) {
      m_sublist.removeAt(i);
   }  
}

EntryNav::EntryNav(EntryNav *parent, Entry *e)
   : m_parent(parent), m_section(e->section), m_type(e->type), m_name(e->name), 
     m_fileDef(0), m_lang(e->lang), m_info(0), m_offset(-1), m_noLoad(false)
{
   if (e->tagInfo) {
      m_tagInfo = new TagInfo;

      m_tagInfo->tagName  = e->tagInfo->tagName;
      m_tagInfo->fileName = e->tagInfo->fileName;
      m_tagInfo->anchor   = e->tagInfo->anchor;
   
   } else {
      m_tagInfo = 0;

   }
}

EntryNav::~EntryNav()
{ 
   delete m_info;
   delete m_tagInfo;
}

void EntryNav::addChild(EntryNav *e)
{  
   m_subList.append(e);
}

bool EntryNav::loadEntry(FileStorage *storage)
{
   if (m_noLoad) {
      return true;
   }

   if (m_offset == -1) {
      //printf("offset not set!\n");
      return false;
   }
   
   if (! storage->seek(m_offset)) {
      //printf("seek failed!\n");
      return false;
   }

   if (m_info) {
      delete m_info;
   }

   m_info = unmarshalEntry(storage);
   m_info->name = m_name;
   m_info->type = m_type;
   m_info->section = m_section;

   return true;
}

bool EntryNav::saveEntry(Entry *e, FileStorage *storage)
{
   m_offset = storage->pos();
   
   marshalEntry(storage, e);
   return true;
}

void EntryNav::releaseEntry()
{
   if (! m_noLoad) {
      //printf("EntryNav::releaseEntry %p\n",m_info);
      delete m_info;
      m_info = 0;
   }
}

void EntryNav::setEntry(Entry *e)
{
   delete m_info;
   m_info = e;

   //printf("EntryNav::setEntry %p\n",e);

   m_noLoad = true;
}

