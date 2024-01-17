/************************************************************************
*
* Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
* Copyright (c) 1997-2014 Dimitri van Heesch
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

#include <stringmap.h>

#include <conceptdef.h>
#include <dirdef.h>
#include <example.h>
#include <groupdef.h>
#include <htmlhelp.h>
#include <membergroup.h>
#include <membername.h>
#include <pagedef.h>
#include <searchindex.h>

int ConceptSDict::compareMapValues(const QSharedPointer<ConceptDef> &item1, const QSharedPointer<ConceptDef> &item2) const
{
   return item1->name().compare(item2->name(), Qt::CaseInsensitive);
}

int DirSDict::compareMapValues(const QSharedPointer<DirDef> &item1, const QSharedPointer<DirDef> &item2) const
{
   return item1->shortName().compare(item2->shortName(), Qt::CaseInsensitive);
}

int ExampleSDict::compareMapValues(const QSharedPointer<Example> &item1, const QSharedPointer<Example> &item2) const
{
   return item1->name.compare(item2->name, Qt::CaseInsensitive);
}

int GroupSDict::compareMapValues(const QSharedPointer<GroupDef> &item1, const QSharedPointer<GroupDef> &item2) const
{
   return item1->groupTitle().compare(item2->groupTitle());
}

int IndexFieldSDict::compareMapValues(const QSharedPointer<IndexField> &item1,
                  const QSharedPointer<IndexField> &item2) const
{
   return item1->name.compare(item2->name, Qt::CaseInsensitive);
}

int MemberGroupSDict::compareMapValues(const QSharedPointer<MemberGroup> &item1, const QSharedPointer<MemberGroup> &item2) const {
   return item1->groupId() - item2->groupId();
}

int MemberNameInfoSDict::compareMapValues(const QSharedPointer<MemberNameInfo> &item1,
                  const QSharedPointer<MemberNameInfo> &item2) const
{
   return item1->memberName().compare(item2->memberName(), Qt::CaseInsensitive );
}

int PageSDict::compareMapValues(const QSharedPointer<PageDef> &i1, const QSharedPointer<PageDef> &i2) const
{
   return i1->name().compare(i2->name(), Qt::CaseInsensitive);
}

void SearchIndexMap ::insertDef(QSharedPointer<Definition> d)
{
   QSharedPointer<SearchDefinitionList> lx = this->find(d->localName());

   if (lx == nullptr) {
      lx = QSharedPointer<SearchDefinitionList>(new SearchDefinitionList(m_letter));
      StringMap<QSharedPointer<SearchDefinitionList>>::insert(d->localName(), lx);
   }

   lx->append(d);
}

QChar SearchIndexMap::letter() const
{
   return m_letter;
}

int SearchIndexMap::compareMapValues(const QSharedPointer<SearchDefinitionList> &md1,
                                    const QSharedPointer<SearchDefinitionList> &md2) const
{
   QString n1 = md1->first()->localName();
   QString n2 = md2->first()->localName();

   return n1.compare(n2, Qt::CaseInsensitive);
}


