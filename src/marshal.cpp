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

#include <cassert>

#include <arguments.h>
#include <definition.h>
#include <entry.h>
#include <example.h>
#include <groupdef.h>
#include <marshal.h>
#include <memberlist.h>
#include <section.h>
#include <stringmap.h>


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

void marshalUInt64(StorageIntf *s, uint64_t v)
{
   marshalUInt(s, uint(v >> 32));
   marshalUInt(s, uint(v & 0xFFFFFFFF));
}

void marshalBool(StorageIntf *s, bool b)
{
   char c = b;
   s->write(&c, sizeof(char));
}

void marshalQByteArray(StorageIntf *s, const QByteArray &str)
{
   uint l = str.length();
   marshalUInt(s, l);

   if (l > 0) {
      s->write(str.data(), l);
   }
}

void marshalQString(StorageIntf *s, const QString &str)
{
   QByteArray temp = str.toUtf8();

   uint l = temp.length();
   marshalUInt(s, l);

   if (l > 0) {
      s->write(temp.constData(), l);
   }
}

void marshalArgumentList(StorageIntf *s, ArgumentList argList)
{
   ArgumentList::marshal(s, &argList);
}

void marshalArgumentLists(StorageIntf *s, QList<ArgumentList> *argLists)
{
   if (argLists == 0) {
      marshalUInt(s, NULL_LIST); 

   } else {
      marshalUInt(s, argLists->count());
    
      for (auto al : *argLists) {
         marshalArgumentList(s, al);
      }
   }
}

void marshalBaseInfoList(StorageIntf *s, QList<BaseInfo> *baseList)
{
   if (baseList == 0) {
      marshalUInt(s, NULL_LIST); 

   } else {
      marshalUInt(s, baseList->count());
      
      for (auto bi : *baseList) {
         marshalQByteArray(s, bi.name);
         marshalInt(s, (int)bi.prot);
         marshalInt(s, (int)bi.virt);
      }
   }
}

void marshalGroupingList(StorageIntf *s, QList<Grouping> *groups)
{
   if (groups == 0) {
      marshalUInt(s, NULL_LIST); 

   } else {
      marshalUInt(s, groups->count());
     
      for (auto g : *groups) {
         marshalQByteArray(s, g.groupname);
         marshalInt(s, (int)g.pri);
      }
   }
}

void marshalSectionInfoList(StorageIntf *s, QList<SectionInfo> *anchors)
{
   if (anchors == 0) {
      marshalUInt(s, NULL_LIST); 

   } else {
      marshalUInt(s, anchors->count());
  
      for (auto si : *anchors) {
         marshalQByteArray(s, si.label);
         marshalQByteArray(s, si.title);
         marshalQByteArray(s, si.ref);
         marshalInt(s, (int)si.type);
         marshalQByteArray(s, si.fileName);
         marshalInt(s, si.lineNr);
         marshalInt(s, si.level);
      }
   }
}

void marshalItemInfoList(StorageIntf *s, QList<ListItemInfo> *sli)
{
   if (sli == 0) {
      marshalUInt(s, NULL_LIST); 

   } else {
      marshalUInt(s, sli->count());
   
      for (auto item : *sli) {
         marshalQByteArray(s, item.type);
         marshalInt(s, item.itemId);
      }
   }
}

void marshalObjPointer(StorageIntf *s, void *obj)
{
   char *b = (char *)&obj;
   s->write(b, sizeof(void *));
}

void marshalSectionDict(StorageIntf *s, SectionDict *sections)
{
   if (sections == 0) {
      marshalUInt(s, NULL_LIST); 

   } else {
      marshalUInt(s, sections->count());
     
      for (auto item = sections->begin(); item != sections->end(); ++item) {
         marshalQString(s, item.key());
         marshalObjPointer(s, item.value().data() );
      }
   }
}

void marshalMemberSDict(StorageIntf *s, MemberSDict *memberSDict)
{
   if (memberSDict == 0) {
      marshalUInt(s, NULL_LIST); 

   } else {
      marshalUInt(s, memberSDict->count());
 
      MemberDef *md;
      int count = 0;

      for (auto item = memberSDict->begin(); item != memberSDict->end(); ++item) {
         marshalQString(s, item.key());
         marshalObjPointer(s, item.value().data());
         count++;
      }

      assert(count == memberSDict->count());
   }
}

void marshalDocInfo(StorageIntf *s, DocInfo *docInfo)
{
   if (docInfo == 0) {
      marshalUInt(s, NULL_LIST); 

   } else {
      marshalUInt(s, 1);
      marshalQByteArray(s, docInfo->doc);
      marshalInt(s, docInfo->line);
      marshalQByteArray(s, docInfo->file);
   }
}

void marshalBriefInfo(StorageIntf *s, BriefInfo *briefInfo)
{
   if (briefInfo == 0) {
      marshalUInt(s, NULL_LIST);

   } else {
      marshalUInt(s, 1);
      marshalQByteArray(s, briefInfo->doc);
      marshalQByteArray(s, briefInfo->tooltip);
      marshalInt(s, briefInfo->line);
      marshalQByteArray(s, briefInfo->file);
   }
}

void marshalBodyInfo(StorageIntf *s, BodyInfo *bodyInfo)
{
   if (bodyInfo == 0) {
      marshalUInt(s, NULL_LIST); 

   } else {
      marshalUInt(s, 1);
      marshalInt(s, bodyInfo->startLine);
      marshalInt(s, bodyInfo->endLine);
      marshalObjPointer(s, bodyInfo->fileDef);
   }
}

void marshalGroupList(StorageIntf *s, SortedList<GroupDef *> *groupList)
{
   if (groupList == 0) {
      marshalUInt(s, NULL_LIST); 

   } else {
      marshalUInt(s, groupList->count());
    
      for (auto item : *groupList) {
         marshalObjPointer(s, item);
      }
   }
}

void marshalMemberList(StorageIntf *s, MemberList *ml)
{
   if (ml == 0) {
      marshalUInt(s, NULL_LIST); 

   } else {
      marshalUInt(s, ml->count());
      
      uint count = 0;
     
      for (auto item : *ml) {
         marshalObjPointer(s, item);
         count++;
      }

      assert(count == ml->count());

      ml->marshal(s);
   }
}

void marshalExampleSDict(StorageIntf *s, ExampleSDict *ed)
{
   if (ed == 0) {
      marshalUInt(s, NULL_LIST); // null pointer representation

   } else {
      marshalUInt(s, ed->count());         

      for (auto item = ed->begin(); item != ed->end(); ++item) {        
         marshalQString(s, item.key());

         QSharedPointer<Example> e = item.value();
         marshalQByteArray(s, e->anchor);
         marshalQByteArray(s, e->name);
         marshalQByteArray(s, e->file);
      }
   }
}

void marshalMemberLists(StorageIntf *s, StringMap<QSharedPointer<MemberList>> *mls)
{
   if (mls == 0) {
      marshalUInt(s, NULL_LIST); 

   } else {
      marshalUInt(s, mls->count());
  
      for (auto item = mls->begin(); item != mls->end(); ++item) {
         marshalQString(s, item.key());
         marshalObjPointer(s, item.value().data()); 
      }
   }
}

void marshalEntry(StorageIntf *s, Entry *e)
{
   marshalUInt(s, HEADER);
   marshalQByteArray(s, e->name);
   marshalQByteArray(s, e->type);
   marshalInt(s, e->section);
   marshalInt(s, (int)e->protection);
   marshalInt(s, (int)e->mtype);
   marshalUInt64(s, e->spec);
   marshalInt(s, e->initLines);
   marshalBool(s, e->stat);
   marshalBool(s, e->explicitExternal);
   marshalBool(s, e->proto);
   marshalBool(s, e->subGrouping);
   marshalBool(s, e->callGraph);
   marshalBool(s, e->callerGraph);
   marshalInt(s, (int)e->virt);

   marshalQByteArray(s, e->args);
   marshalQByteArray(s, e->bitfields);

   marshalArgumentList(s, e->argList);
   marshalArgumentLists(s, e->tArgLists);

   marshalQString(s, e->program);
   marshalQString(s, e->initializer);

   marshalQByteArray(s, e->includeFile);
   marshalQByteArray(s, e->includeName);
   marshalQByteArray(s, e->doc);
   marshalInt(s, e->docLine);
   marshalQByteArray(s, e->docFile);
   marshalQByteArray(s, e->brief);
   marshalInt(s, e->briefLine);
   marshalQByteArray(s, e->briefFile);
   marshalQByteArray(s, e->inbodyDocs);
   marshalInt(s, e->inbodyLine);
   marshalQByteArray(s, e->inbodyFile);
   marshalQByteArray(s, e->relates);
   marshalInt(s, e->relatesType);
   marshalQByteArray(s, e->read);
   marshalQByteArray(s, e->write);
   marshalQByteArray(s, e->inside);
   marshalQByteArray(s, e->exception);
   marshalArgumentList(s, e->typeConstr);
   marshalInt(s, e->bodyLine);
   marshalInt(s, e->endBodyLine);
   marshalInt(s, e->mGrpId);
   marshalBaseInfoList(s, e->extends);
   marshalGroupingList(s, e->groups);
   marshalSectionInfoList(s, e->anchors);
   marshalQByteArray(s, e->fileName);
   marshalInt(s, e->startLine);
   marshalItemInfoList(s, e->sli);
   marshalInt(s, (int)e->lang);
   marshalBool(s, e->hidden);
   marshalBool(s, e->artificial);
   marshalInt(s, (int)e->groupDocType);
   marshalQByteArray(s, e->id);
}

void marshalEntryTree(StorageIntf *s, Entry *e)
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

uint64_t unmarshalUInt64(StorageIntf *s)
{
   uint64_t result = uint64_t(unmarshalUInt(s)) << 32;
   result |= unmarshalUInt(s);
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
   
   return result;
}

ArgumentList *unmarshalArgumentList(StorageIntf *s)
{
   return ArgumentList::unmarshal(s);
}

QList<ArgumentList> *unmarshalArgumentLists(StorageIntf *s)
{
   uint i;
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return 0; 
   }

   QList<ArgumentList> *result = new QList<ArgumentList>;
   
   assert(count < 1000000);
   
   for (i = 0; i < count; i++) {
      result->append(*unmarshalArgumentList(s));
   }

   return result;
}

QList<BaseInfo> *unmarshalBaseInfoList(StorageIntf *s)
{
   uint i;
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return 0; 
   }

   QList<BaseInfo> *result = new QList<BaseInfo>;  
   assert(count < 1000000);

   for (i = 0; i < count; i++) {
      QByteArray name   = unmarshalQByteArray(s);

      Protection prot = (Protection)unmarshalInt(s);
      Specifier virt  = (Specifier)unmarshalInt(s);

      result->append(BaseInfo(name, prot, virt));
   }
   return result;
}

QList<Grouping> *unmarshalGroupingList(StorageIntf *s)
{
   uint i;
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return 0; 
   }

   QList<Grouping> *result = new QList<Grouping>;
   assert(count < 1000000);

   for (i = 0; i < count; i++) {
      QByteArray name = unmarshalQByteArray(s);
      Grouping::GroupPri_t prio = (Grouping::GroupPri_t)unmarshalInt(s);

      result->append(Grouping(name, prio));
   }

   return result;
}

QList<SectionInfo> *unmarshalSectionInfoList(StorageIntf *s)
{
   uint i;
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return 0;
   }

   QList<SectionInfo> *result = new QList<SectionInfo>;  
   assert(count < 1000000);

   for (i = 0; i < count; i++) {
      QByteArray label = unmarshalQByteArray(s);
      QByteArray title = unmarshalQByteArray(s);
      QByteArray ref   = unmarshalQByteArray(s);

      SectionInfo::SectionType type = (SectionInfo::SectionType)unmarshalInt(s);

      QByteArray fileName = unmarshalQByteArray(s);

      int lineNr = unmarshalInt(s);
      int level  = unmarshalInt(s);
      result->append(SectionInfo(fileName, lineNr, label, title, type, level, ref));
   }
   return result;
}

QList<ListItemInfo> *unmarshalItemInfoList(StorageIntf *s)
{
   uint i;
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return 0;  
   }

   QList<ListItemInfo> *result = new QList<ListItemInfo>;

   assert(count < 1000000);

   for (i = 0; i < count; i++) {
      ListItemInfo lii;
      lii.type   = unmarshalQByteArray(s);
      lii.itemId = unmarshalInt(s);

      result->append(lii);
   }
   return result;
}

void *unmarshalObjPointer(StorageIntf *s)
{
   void *result;
   s->read((char *)&result, sizeof(void *));

   return result;
}

SectionDict *unmarshalSectionDict(StorageIntf *s)
{
   uint i;
   uint count = unmarshalUInt(s);
   
   if (count == NULL_LIST) {
      return 0;  
   }

   SectionDict *result = new SectionDict;
   assert(count < 1000000);

   for (i = 0; i < count; i++) {
      QByteArray key  = unmarshalQByteArray(s);
      QSharedPointer<SectionInfo> si ((SectionInfo *)unmarshalObjPointer(s));
      
      result->insert(key, si);
   }
   return result;
}

MemberSDict *unmarshalMemberSDict(StorageIntf *s)
{
   uint i;
   uint count = unmarshalUInt(s);
 
   if (count == NULL_LIST) {      
      return 0; 
   }
   MemberSDict *result = new MemberSDict;
   assert(count < 1000000);

   for (i = 0; i < count; i++) {
      
      QByteArray key    = unmarshalQByteArray(s);          
      QSharedPointer<MemberDef> md ((MemberDef *)unmarshalObjPointer(s));
      
      result->insert(key, md);
   }

   return result;
}

DocInfo *unmarshalDocInfo(StorageIntf *s)
{
   uint count = unmarshalUInt(s);
   if (count == NULL_LIST) {
      return 0;
   }

   DocInfo *result = new DocInfo;
   result->doc  = unmarshalQByteArray(s);
   result->line = unmarshalInt(s);
   result->file = unmarshalQByteArray(s);

   return result;
}

BriefInfo *unmarshalBriefInfo(StorageIntf *s)
{
   uint count = unmarshalUInt(s);
   if (count == NULL_LIST) {
      return 0;
   }

   BriefInfo *result = new BriefInfo;
   result->doc     = unmarshalQByteArray(s);
   result->tooltip = unmarshalQByteArray(s);
   result->line    = unmarshalInt(s);
   result->file    = unmarshalQByteArray(s);
   return result;
}

BodyInfo *unmarshalBodyInfo(StorageIntf *s)
{
   uint count = unmarshalUInt(s);
   if (count == NULL_LIST) {
      return 0;
   }

   BodyInfo *result = new BodyInfo;
   result->startLine = unmarshalInt(s);
   result->endLine   = unmarshalInt(s);
   result->fileDef   = (FileDef *)unmarshalObjPointer(s);

   return result;
}

SortedList<GroupDef *> *unmarshalGroupList(StorageIntf *s)
{
   uint i;
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return 0;   
   }

   assert(count < 1000000);

   SortedList<GroupDef *> *result = new SortedList<GroupDef *>;

   for (i = 0; i < count; i++) {
      GroupDef *gd = (GroupDef *)unmarshalObjPointer(s);
      result->append(gd);
   }

   return result;
}

MemberList *unmarshalMemberList(StorageIntf *s)
{
   uint i;
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return 0;
   }

   MemberList *result = new MemberList;
   assert(count < 1000000);

   for (i = 0; i < count; i++) {
      MemberDef *md = (MemberDef *)unmarshalObjPointer(s);
      result->append(md);
   }

   result->unmarshal(s);
   return result;
}

ExampleSDict *unmarshalExampleSDict(StorageIntf *s)
{
   uint i;
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return 0;
   }

   ExampleSDict *result = new ExampleSDict;
   assert(count < 1000000);

   for (i = 0; i < count; i++) {
      QByteArray key = unmarshalQByteArray(s);
      QSharedPointer<Example> e (new Example);

      e->anchor = unmarshalQByteArray(s);
      e->name   = unmarshalQByteArray(s);
      e->file   = unmarshalQByteArray(s);

      result->insert(key, e);
   }

   return result;
}

StringMap<QSharedPointer<MemberList>> *unmarshalMemberLists(StorageIntf *s)
{
   uint i;
   uint count = unmarshalUInt(s);

   if (count == NULL_LIST) {
      return 0;
   }

   StringMap<QSharedPointer<MemberList>> *result = new StringMap<QSharedPointer<MemberList>>();
   assert(count < 1000000);

   for (i = 0; i < count; i++) {
      QByteArray key = unmarshalQByteArray(s);

      QSharedPointer<MemberList> ml ((MemberList *)unmarshalObjPointer(s));
      result->insert(key, ml);
   }

   return result;
}

Entry *unmarshalEntry(StorageIntf *s)
{
   Entry *e = new Entry;

   uint header = unmarshalUInt(s);
   assert(header == HEADER);

   e->name             = unmarshalQByteArray(s);
   e->type             = unmarshalQByteArray(s);
   e->section          = unmarshalInt(s);
   e->protection       = (Protection)unmarshalInt(s);
   e->mtype            = (MethodTypes)unmarshalInt(s);
   e->spec             = unmarshalUInt64(s);
   e->initLines        = unmarshalInt(s);
   e->stat             = unmarshalBool(s);
   e->explicitExternal = unmarshalBool(s);
   e->proto            = unmarshalBool(s);
   e->subGrouping      = unmarshalBool(s);
   e->callGraph        = unmarshalBool(s);
   e->callerGraph      = unmarshalBool(s);
   e->virt             = (Specifier)unmarshalInt(s);
   e->args             = unmarshalQByteArray(s);
   e->bitfields        = unmarshalQByteArray(s);
  
   e->argList          = *unmarshalArgumentList(s);      // CopperSpice - check for memory leak
   e->tArgLists        = unmarshalArgumentLists(s);

   e->program          = unmarshalQByteArray(s);
   e->initializer      = unmarshalQByteArray(s);

   e->includeFile      = unmarshalQByteArray(s);
   e->includeName      = unmarshalQByteArray(s);
   e->doc              = unmarshalQByteArray(s);
   e->docLine          = unmarshalInt(s);
   e->docFile          = unmarshalQByteArray(s);
   e->brief            = unmarshalQByteArray(s);
   e->briefLine        = unmarshalInt(s);
   e->briefFile        = unmarshalQByteArray(s);
   e->inbodyDocs       = unmarshalQByteArray(s);
   e->inbodyLine       = unmarshalInt(s);
   e->inbodyFile       = unmarshalQByteArray(s);
   e->relates          = unmarshalQByteArray(s);
   e->relatesType      = (RelatesType)unmarshalInt(s);
   e->read             = unmarshalQByteArray(s);
   e->write            = unmarshalQByteArray(s);
   e->inside           = unmarshalQByteArray(s);
   e->exception        = unmarshalQByteArray(s);      
   e->typeConstr       = *unmarshalArgumentList(s);      // CopperSpice -check for memory leak
   e->bodyLine         = unmarshalInt(s);
   e->endBodyLine      = unmarshalInt(s);
   e->mGrpId           = unmarshalInt(s);
   delete e->extends;
   e->extends          = unmarshalBaseInfoList(s);
   delete e->groups;
   e->groups           = unmarshalGroupingList(s);
   delete e->anchors;
   e->anchors          = unmarshalSectionInfoList(s);
   e->fileName         = unmarshalQByteArray(s);
   e->startLine        = unmarshalInt(s);
   e->sli              = unmarshalItemInfoList(s);
   e->lang             = (SrcLangExt)unmarshalInt(s);
   e->hidden           = unmarshalBool(s);
   e->artificial       = unmarshalBool(s);
   e->groupDocType     = (Entry::GroupDocType)unmarshalInt(s);
   e->id               = unmarshalQByteArray(s);
   return e;
}

Entry *unmarshalEntryTree(StorageIntf *s)
{
   Entry *e = unmarshalEntry(s);

   uint count = unmarshalUInt(s);
   uint i;

   for (i = 0; i < count; i++) {
      e->addSubEntry(unmarshalEntryTree(s));
   }
   return e;
}
