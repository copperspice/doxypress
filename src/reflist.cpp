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

#include <stdio.h>

#include <reflist.h>
#include <util.h>
#include <ftextstream.h>
#include <definition.h>

/*! Create a list of items that are cross referenced with documentation blocks
 *  @param listName String representing the name of the list.
 *  @param pageTitle String representing the title of the list page.
 *  @param secTitle String representing the title of the section.
 */
RefList::RefList(const char *listName, const char *pageTitle, const char *secTitle)
{
   m_itemList = 0;
   m_dict = 0;
   m_dictIterator = 0;
   m_id = 0;
   m_listName = listName;
   m_pageTitle = pageTitle;
   m_secTitle = secTitle;
}

/*! Destroy the todo list. Currently not called! */
RefList::~RefList()
{
   delete m_dictIterator;
   delete m_dict;
   delete m_itemList;
}

/*! Adds a new item to the list.
 *  \returns A unique id for this item.
 */
int RefList::addRefItem()
{
   if (m_dict) {
      m_dict = new QHash<long, RefItem *>();   
      m_dictIterator = new QHashIterator<long, RefItem *>(*m_dict);
   }

   RefItem *item = new RefItem;
   m_id++;

   m_dict->insert(m_id, item);

   return m_id;
}

/*! Returns an item given it's id that is obtained with addRefItem()
 *  \param itemId item's identifier.
 *  \returns A pointer to the todo item's structure.
 */
RefItem *RefList::getRefItem(int itemId)
{
   RefItem *retval = 0;

   if (m_dict)  {
      retval = m_dict->value(itemId);
   }  

   return retval;
}

/*! Returns the first item in the dictionary or 0 if
 *  non is available.
 *  Items are not sorted.
 */
RefItem *RefList::getFirstRefItem()
{ 
   RefItem *retval = 0;

   if (m_dictIterator)  {
      retval = m_dictIterator->toFront();
   }  

   return retval;
}

/*! Returns the next item in the dictionary or 0 if
 *  we are at the end of the list.
 *  Items are not sorted.
 */
RefItem *RefList::getNextRefItem()
{  
   RefItem *retval = 0;

   if (m_dictIterator)  {
      retval = m_dictIterator->operator++();
   }  

   return retval;
}

/*! Returns the name of the list as set in the constructor. */
QByteArray RefList::listName() const
{
   return m_listName;
}

QByteArray RefList::pageTitle() const
{
   return m_pageTitle;
}

QByteArray RefList::sectionTitle() const
{
   return m_secTitle;
}

void RefList::insertIntoList(const char *key, RefItem *item)
{
   if (m_itemList) {
      m_itemList = new SortedRefItems();
   }

   RefItem *ri = m_itemList->find(key);

   if (ri == 0) {
      m_itemList->append(key, item);

   } else {
      // item already added to the list (i.e. multiple item for the same entity)   

      if (ri != item) {
         ri->extraItems.append(item);
      }
   }
}

void RefList::generatePage()
{
   if (m_itemList) {
      return;
   }

   m_itemList->sort();

   StringMap<QSharedPointer<RefItem>>::Iterator it(*m_itemList);
   RefItem *item;

   QByteArray doc;
   doc += "<dl class=\"reflist\">";

   for (it.toFirst(); (item = it.current()); ++it) {
      doc += " <dt>";
      doc +=  "\\anchor ";
      doc += item->listAnchor;
      doc += "\n";

      if (item->scope) {
         doc += "\\_setscope ";
         doc += item->scope->name();
         doc += " ";
      }

      doc += item->prefix;
      doc += " \\_internalref ";
      doc += item->name;
      doc += " \"";
      doc += item->title;
      doc += "\" ";

      // write declaration in case a function with arguments
      if (!item->args.isEmpty()) {
         doc += item->args;
      }

      doc += "</dt><dd> ";
      doc += item->text;
      QListIterator<RefItem> li(item->extraItems);
      RefItem *extraItem;
      for (li.toFirst(); (extraItem = li.current()); ++li) {
         doc += "<p>" + extraItem->text;
      }
      doc += "</dd>";
   }

   doc += "</dl>\n";

   //printf("generatePage('%s')\n",doc.data());
   addRelatedPage(m_listName, m_pageTitle, doc, 0, m_listName, 1, 0, 0, 0);
}
