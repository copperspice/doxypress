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

#include <stringmap.h>

#include <dirdef.h>
#include <example.h>
#include <groupdef.h>
#include <htmlhelp.h>
#include <pagedef.h>
#include <searchindex.h>

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
   QSharedPointer<SearchDefinitionList> lx = this->find(d->name());
   
   if (lx == nullptr) {
      lx = QSharedPointer<SearchDefinitionList>(new SearchDefinitionList(m_letter));
      StringMap<QSharedPointer<SearchDefinitionList>>::insert(d->name(), lx);
   }
   
   lx->append(d);
}

uint SearchIndexMap::letter() const
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









