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

#ifndef MARSHAL_H
#define MARSHAL_H

#include <QFile>
#include <QByteArray>
#include <QString>
#include <QVector>

#include <store.h>
#include <section.h>

class ArgumentList;
class Entry;

struct BaseInfo;
struct BriefInfo;
struct Grouping;
struct SectionInfo;
struct ListItemInfo;

#define NULL_LIST 0xffffffff

void marshalInt(StorageIntf *s,             int v);
void marshalUInt(StorageIntf *s,            uint v);
void marshalUInt64(StorageIntf *s,          uint64_t v);
void marshalBool(StorageIntf *s,            bool b);
void marshalObjPointer(StorageIntf *s,      void *obj);

void marshalQByteArray(StorageIntf *s,      const QByteArray &str);
void marshalQString(StorageIntf *s,         const QString &str);

void marshalArgumentList(StorageIntf *s,    const ArgumentList &argList);
void marshalArgumentLists(StorageIntf *s,   const QVector<ArgumentList> &argLists);

void marshalBaseInfoList(StorageIntf *s,    QVector<BaseInfo> *baseList);
void marshalGroupingList(StorageIntf *s,    QVector<Grouping> *groups);

void marshalSectionInfoList(StorageIntf *s, QVector<SectionInfo> *anchors);
void marshalItemInfoList(StorageIntf *s,    const QVector<ListItemInfo> &list);

void marshalBriefInfo(StorageIntf *s,       BriefInfo *briefInfo);
void marshalEntry(StorageIntf *s,           QSharedPointer<Entry> e);
void marshalEntryTree(StorageIntf *s,       QSharedPointer<Entry> e);

int                     unmarshalInt(StorageIntf *s);
uint                    unmarshalUInt(StorageIntf *s);
uint64_t                unmarshalUInt64(StorageIntf *s);
bool                    unmarshalBool(StorageIntf *s);
void                   *unmarshalObjPointer(StorageIntf *s);

QByteArray              unmarshalQByteArray(StorageIntf *s);
QString                 unmarshalQString(StorageIntf *s);

ArgumentList            unmarshalArgumentList(StorageIntf *s);
QVector<ArgumentList>   unmarshalArgumentLists(StorageIntf *s);

QVector<BaseInfo>       unmarshalBaseInfoList(StorageIntf *s);
QVector<Grouping>       unmarshalGroupingList(StorageIntf *s);
QVector<SectionInfo>    unmarshalSectionInfoList(StorageIntf *s);
QVector<ListItemInfo>   unmarshalItemInfoList(StorageIntf *s);

BriefInfo              *unmarshalBriefInfo(StorageIntf *s);

QSharedPointer<Entry>   unmarshalEntry(StorageIntf *s);
QSharedPointer<Entry>   unmarshalEntryTree(StorageIntf *s);

#endif
