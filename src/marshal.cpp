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

#include <cassert>

#include <marshal.h>

#include <arguments.h>
#include <entry.h>
#include <groupdef.h>

#define HEADER ('D'<<24)+('O'<<16)+('X'<<8)+'!'

void marshalInt(StorageIntf *s, int v)
{
   uchar b[4];
   b[0] = ((uint)v) >> 24;
   b[1] = (((uint)v) >> 16) & 0xff;
   b[2] = (((uint)v) >> 8) & 0xff;
   b[3] = v & 0xff;
   s->write((const char *)b, 4);
}

void marshalUInt(StorageIntf *s, uint v)
{
   uchar b[4];
   b[0] = v >> 24;
   b[1] = (v >> 16) & 0xff;
   b[2] = (v >> 8) & 0xff;
   b[3] = v & 0xff;
   s->write((const char *)b, 4);
}

void marshalTraits(StorageIntf *s, Entry::Traits v)
{
   marshalQByteArray(s, v.toQByteArray());
}

void marshalBool(StorageIntf *s, bool b)
{
   char c = b;
   s->write(&c, sizeof(char));
}

void marshalQByteArray(StorageIntf *s, const QByteArray &str)
{
   uint len = str.length();
   marshalUInt(s, len);

   if (len > 0) {
      s->write(str.constData(), len);
   }
}

void marshalQString(StorageIntf *s, const QString &str)
{
   QByteArray temp = str.toUtf8();

   uint len = temp.length();
   marshalUInt(s, len);

   if (len > 0) {
      s->write(temp.constData(), len);
   }
}

void marshalArgumentList(StorageIntf *s, const ArgumentList &argList)
{
   ArgumentList::marshal(s, argList);
}

void marshalArgumentLists(StorageIntf *s, const QVector<ArgumentList> &argLists)
{
   if (argLists.isEmpty()) {
      marshalUInt(s, NULL_LIST);

   } else {
      marshalUInt(s, argLists.count());

      for (auto &al : argLists) {
         marshalArgumentList(s, al);
      }
   }
}

void marshalBaseInfoList(StorageIntf *s, QVector<BaseInfo> *baseList)
{
   if (baseList == nullptr) {
      marshalUInt(s, NULL_LIST);

   } else {
      marshalUInt(s, baseList->count());

      for (auto &bi : *baseList) {
         marshalQString(s, bi.name);
         marshalInt(s, bi.prot);
         marshalInt(s, bi.virt);
      }
   }
}

void marshalGroupingList(StorageIntf *s, const QVector<Grouping> &groups)
{
   if (groups.isEmpty()) {
      marshalUInt(s, NULL_LIST);

   } else {
      marshalUInt(s, groups.count());

      for (auto &g : groups) {
         marshalQString(s, g.groupname);
         marshalInt(s, g.pri);
      }
   }
}

void marshalSectionInfoList(StorageIntf *s, const QVector<SectionInfo> &anchors)
{
   if (anchors.isEmpty()) {
      marshalUInt(s, NULL_LIST);

   } else {
      marshalUInt(s, anchors.count());

      for (auto &si : anchors) {
         marshalQString(s, si.label);
         marshalQString(s, si.title);
         marshalQString(s, si.ref);
         marshalInt(s,     si.type);
         marshalQString(s, si.fileName);
         marshalInt(s,     si.lineNr);
         marshalInt(s,     si.level);
      }
   }
}

void marshalItemInfoList(StorageIntf *s, const QVector<ListItemInfo> &list)
{
   if (list.isEmpty()) {
      marshalUInt(s, NULL_LIST);

   } else {
      marshalUInt(s, list.count());

      for (auto &item : list) {
         marshalQString(s, item.type);
         marshalInt(s, item.itemId);
      }
   }
}

void marshalObjPointer(StorageIntf *s, void *obj)
{
   char *b = (char *)&obj;
   s->write(b, sizeof(void *));
}

void marshalBriefInfo(StorageIntf *s, BriefInfo *briefInfo)
{
   if (briefInfo == 0) {
      marshalUInt(s, NULL_LIST);

   } else {
      marshalUInt(s, 1);
      marshalQString(s, briefInfo->doc);
      marshalQString(s, briefInfo->tooltip);
      marshalInt(s,     briefInfo->line);
      marshalQString(s, briefInfo->file);
   }
}

void marshalEntry(StorageIntf *s, QSharedPointer<Entry> e)
{
   marshalUInt(s, HEADER);

   marshalQString(s, e->name);
   marshalQString(s, e->type);
   marshalInt(s,     e->section);
   marshalInt(s,     e->protection);
   marshalInt(s,     e->mtype);
   marshalTraits(s,  e->m_traits);
   marshalInt(s,     e->initLines);
   marshalBool(s,    e->stat);
   marshalBool(s,    e->explicitExternal);
   marshalBool(s,    e->proto);
   marshalBool(s,    e->subGrouping);
   marshalBool(s,    e->callGraph);
   marshalBool(s,    e->callerGraph);
   marshalInt(s,     e->virt);
   marshalQString(s, e->args);
   marshalQString(s, e->bitfields);

   marshalArgumentList(s,  e->argList);
   marshalArgumentLists(s, e->m_templateArgLists);

   marshalQString(s, e->m_program);
   marshalQString(s, e->initializer);

   marshalQString(s, e->includeFile);
   marshalQString(s, e->includeName);
   marshalQString(s, e->doc);
   marshalInt(s,     e->docLine);
   marshalQString(s, e->docFile);
   marshalQString(s, e->brief);
   marshalInt(s,     e->briefLine);
   marshalQString(s, e->briefFile);
   marshalQString(s, e->inbodyDocs);
   marshalInt(s,     e->inbodyLine);
   marshalQString(s, e->inbodyFile);
   marshalQString(s, e->relates);
   marshalInt(s,     e->relatesType);

   marshalQString(s, e->m_read);
   marshalQString(s, e->m_write);
   marshalQString(s, e->m_reset);
   marshalQString(s, e->m_notify);
   marshalQString(s, e->m_revision);
   marshalQString(s, e->m_designable);
   marshalQString(s, e->m_scriptable);
   marshalQString(s, e->m_stored);
   marshalQString(s, e->m_user);

   marshalQString(s, e->inside);
   marshalQString(s, e->exception);
   marshalArgumentList(s, e->typeConstr);
   marshalInt(s, e->bodyLine);
   marshalInt(s, e->endBodyLine);
   marshalInt(s, e->mGrpId);

   marshalBaseInfoList(s,    &e->extends);

   marshalGroupingList(s,    e->m_groups);
   marshalSectionInfoList(s, e->m_anchors);

   marshalQString(s,         e->fileName);
   marshalInt(s,             e->startLine);
   marshalItemInfoList(s,    e->m_specialLists);
   marshalInt(s,             e->lang);
   marshalBool(s,            e->hidden);
   marshalBool(s,            e->artificial);
   marshalInt(s,             e->groupDocType);
   marshalQString(s,         e->id);
}

void marshalEntryTree(StorageIntf *s, QSharedPointer<Entry> e)
{
   marshalEntry(s, e);
   marshalUInt(s, e->children().count());

   for (auto child : e->children()) {
      marshalEntryTree(s, child);
   }
}

int unmarshalInt(StorageIntf *s)
{
   uchar b[4];
   s->read((char *)b, 4);
   int result = (int)((((uint)b[0]) << 24) + ((uint)b[1] << 16) + ((uint)b[2] << 8) + (uint)b[3]);

   return result;
}

uint unmarshalUInt(StorageIntf *s)
{
   uchar b[4];
   s->read((char *)b, 4);
   uint result = (((uint)b[0]) << 24) + ((uint)b[1] << 16) + ((uint)b[2] << 8) + (uint)b[3];

   return result;
}

Entry::Traits unmarshalTraits(StorageIntf *s)
{
   QByteArray temp = unmarshalQByteArray(s);
   Entry::Traits result = Entry::Traits::fromQByteArray(temp);

   return result;
}

bool unmarshalBool(StorageIntf *s)
{
   char result;
   s->read(&result, sizeof(result));

   return result;
}

QByteArray unmarshalQByteArray(StorageIntf *s)
{
   uint len = unmarshalUInt(s);

   QByteArray result;
   result.resize(len);

   if (len > 0) {
      s->read(result.data(), len);
   }

   return result;
}

QString unmarshalQString(StorageIntf *s)
{
   uint len = unmarshalUInt(s);

   QByteArray result;
   result.resize(len);

   if (len > 0) {
      s->read(result.data(), len);
   }

   return QString::fromUtf8(result);
}

ArgumentList unmarshalArgumentList(StorageIntf *s)
{
   return ArgumentList::unmarshal(s);
}

QVector<ArgumentList> unmarshalArgumentLists(StorageIntf *s)
{
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return QVector<ArgumentList>();
   }

   QVector<ArgumentList> result;
   assert(count < 1000000);

   for (int i = 0; i < count; i++) {
      result.append(unmarshalArgumentList(s));
   }

   return result;
}

QVector<BaseInfo> unmarshalBaseInfoList(StorageIntf *s)
{
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return QVector<BaseInfo>();
   }

   QVector<BaseInfo> result;
   assert(count < 1000000);

   for (int i = 0; i < count; i++) {
      QString name    = unmarshalQString(s);

      Protection prot = static_cast<Protection>(unmarshalInt(s));
      Specifier virt  = static_cast<Specifier>(unmarshalInt(s));

      result.append(BaseInfo(name, prot, virt));
   }

   return result;
}

QVector<Grouping> unmarshalGroupingList(StorageIntf *s)
{
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return QVector<Grouping>();
   }

   QVector<Grouping> result;
   assert(count < 1000000);

   for (int i = 0; i < count; i++) {
      QString name = unmarshalQString(s);
      Grouping::GroupPri_t prio = (Grouping::GroupPri_t)unmarshalInt(s);

      result.append(Grouping(name, prio));
   }

   return result;
}

QVector<SectionInfo> unmarshalSectionInfoList(StorageIntf *s)
{
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return QVector<SectionInfo>();
   }

   QVector<SectionInfo> result;
   assert(count < 1000000);

   for (int i = 0; i < count; i++) {
      QString label = unmarshalQString(s);
      QString title = unmarshalQString(s);
      QString ref   = unmarshalQString(s);

      SectionInfo::SectionType type = (SectionInfo::SectionType)unmarshalInt(s);
      QString fileName = unmarshalQString(s);

      int lineNr = unmarshalInt(s);
      int level  = unmarshalInt(s);

      result.append(SectionInfo(fileName, lineNr, label, title, type, level, ref));
   }

   return result;
}

QVector<ListItemInfo> unmarshalItemInfoList(StorageIntf *s)
{
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return QVector<ListItemInfo>();
   }

   QVector<ListItemInfo> result;
   assert(count < 1000000);

   for (int i = 0; i < count; i++) {
      ListItemInfo list;
      list.type   = unmarshalQString(s);
      list.itemId = unmarshalInt(s);

      result.append(list);
   }

   return result;
}

void *unmarshalObjPointer(StorageIntf *s)
{
   void *result;
   s->read((char *)&result, sizeof(void *));

   return result;
}

BriefInfo *unmarshalBriefInfo(StorageIntf *s)
{
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return 0;
   }

   BriefInfo *result = new BriefInfo;

   result->doc     = unmarshalQString(s);
   result->tooltip = unmarshalQString(s);
   result->line    = unmarshalInt(s);
   result->file    = unmarshalQString(s);

   return result;
}

QSharedPointer<Entry> unmarshalEntry(StorageIntf *s)
{
   QSharedPointer<Entry> e = QMakeShared<Entry>();

   uint header = unmarshalUInt(s);
   assert(header == HEADER);

   e->name             = unmarshalQString(s);
   e->type             = unmarshalQString(s);
   e->section          = unmarshalInt(s);
   e->protection       = static_cast<Protection>(unmarshalInt(s));
   e->mtype            = static_cast<MethodTypes>(unmarshalInt(s));
   e->m_traits         = unmarshalTraits(s);
   e->initLines        = unmarshalInt(s);
   e->stat             = unmarshalBool(s);
   e->explicitExternal = unmarshalBool(s);
   e->proto            = unmarshalBool(s);
   e->subGrouping      = unmarshalBool(s);
   e->callGraph        = unmarshalBool(s);
   e->callerGraph      = unmarshalBool(s);
   e->virt             = static_cast<Specifier>(unmarshalInt(s));
   e->args             = unmarshalQString(s);
   e->bitfields        = unmarshalQString(s);

   e->argList            = unmarshalArgumentList(s);
   e->m_templateArgLists = unmarshalArgumentLists(s);

   e->m_program        = unmarshalQString(s);
   e->initializer      = unmarshalQString(s);

   e->includeFile      = unmarshalQString(s);
   e->includeName      = unmarshalQString(s);
   e->doc              = unmarshalQString(s);
   e->docLine          = unmarshalInt(s);
   e->docFile          = unmarshalQString(s);
   e->brief            = unmarshalQString(s);
   e->briefLine        = unmarshalInt(s);
   e->briefFile        = unmarshalQString(s);
   e->inbodyDocs       = unmarshalQString(s);
   e->inbodyLine       = unmarshalInt(s);
   e->inbodyFile       = unmarshalQString(s);
   e->relates          = unmarshalQString(s);
   e->relatesType      = static_cast<RelatesType>(unmarshalInt(s));

   e->m_read           = unmarshalQString(s);
   e->m_write          = unmarshalQString(s);
   e->m_reset          = unmarshalQString(s);
   e->m_notify         = unmarshalQString(s);
   e->m_revision       = unmarshalQString(s);
   e->m_designable     = unmarshalQString(s);
   e->m_scriptable     = unmarshalQString(s);
   e->m_stored         = unmarshalQString(s);
   e->m_user           = unmarshalQString(s);

   e->inside           = unmarshalQString(s);
   e->exception        = unmarshalQString(s);
   e->typeConstr       = unmarshalArgumentList(s);
   e->bodyLine         = unmarshalInt(s);
   e->endBodyLine      = unmarshalInt(s);
   e->mGrpId           = unmarshalInt(s);

   e->extends          = unmarshalBaseInfoList(s);
   e->m_groups         = unmarshalGroupingList(s);
   e->m_anchors        = unmarshalSectionInfoList(s);

   e->fileName         = unmarshalQString(s);
   e->startLine        = unmarshalInt(s);
   e->m_specialLists   = unmarshalItemInfoList(s);
   e->lang             = static_cast<SrcLangExt>(unmarshalInt(s));
   e->hidden           = unmarshalBool(s);
   e->artificial       = unmarshalBool(s);
   e->groupDocType     = static_cast<Entry::GroupDocType>(unmarshalInt(s));
   e->id               = unmarshalQString(s);

   return e;
}

QSharedPointer<Entry> unmarshalEntryTree(StorageIntf *s)
{
   QSharedPointer<Entry> e = unmarshalEntry(s);

   uint count = unmarshalUInt(s);

   for (int i = 0; i < count; i++) {
      e->addSubEntry(unmarshalEntryTree(s), e);
   }

   return e;
}
