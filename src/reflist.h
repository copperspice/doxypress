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

#ifndef REFLIST_H
#define REFLIST_H

#include <QHash>
#include <QList>

#include <stringmap.h>
#include <sortedlist.h>

class Definition;

/** This struct represents an item in the list of references. */
struct RefItem {
   RefItem() : scope(0) 
   {}

   QByteArray text;           //!< text of the item.
   QByteArray listAnchor;     //!< anchor in the list
   QByteArray prefix;         //!< type prefix for the name  
   QByteArray name;           //!< name of the entity containing the reference
   QByteArray title;          //!< display name of the entity
   QByteArray args;           //!< optional arguments for the entity (if function)

   QSharedPointer<Definition> scope;         //!< scope to use for references.  
};


/** List of cross-referenced items
 *
 *  This class represents a list of items that are put
 *  at a certain point in the documentation by some special command
 *  and are collected in a list. The items cross-reference the
 *  documentation and the list.
 *
 *  Examples are the todo list, the test list and the bug list,
 *  introduced by the \\todo, \\test, and \\bug commands respectively.
 */
class RefList
{
 public:
   int      addRefItem();
   RefItem *getRefItem(int todoItemId);
   RefItem *getFirstRefItem();
   RefItem *getNextRefItem();

   QByteArray listName() const;
   QByteArray pageTitle() const;
   QByteArray sectionTitle() const;

   RefList(const char *listName, const char *pageTitle, const char *secTitle);
   ~RefList();

   void insertIntoList(const char *key, RefItem *item);
   void generatePage();

 private:
   int m_id;

   QByteArray m_listName;
   QByteArray m_pageTitle;
   QByteArray m_secTitle;  

   QMap<QByteArray, QList<RefItem>> m_itemMap;
  
   QHash<long, RefItem *> *m_dict;
   QHashIterator<long, RefItem *> *m_dictIterator;
};

#endif
