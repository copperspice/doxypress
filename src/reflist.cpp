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

#include <stdio.h>

#include <reflist.h>

#include <definition.h>
#include <util.h>

/*! Create a list of items that are cross referenced with documentation blocks
 *  @param listName String representing the name of the list.
 *  @param pageTitle String representing the title of the list page.
 *  @param secTitle String representing the title of the section.
 */
RefList::RefList(const QString &listName, const QString &pageTitle, const QString &secTitle)
   : m_dictIterator(m_dict)
{      
   m_id = 0;

   m_listName  = listName;
   m_fileName  = convertNameToFile_X(listName, false, true);
   m_pageTitle = pageTitle;
   m_secTitle  = secTitle;
}

/*! Destroy the todo list. Currently not called! */
RefList::~RefList()
{   
}

/*! Adds a new item to the list.
 *  \returns A unique id for this item.
 */
int RefList::addRefItem()
{  
   RefItem *item = new RefItem;
   m_id++;

   m_dict.insert(m_id, item);

   return m_id;
}

/*! Returns an item given it's id that is obtained with addRefItem()
 *  \param itemId item's identifier.
 *  \returns A pointer to the todo item's structure.
 */
RefItem *RefList::getRefItem(int itemId)
{
   RefItem *retval = 0;

   retval = m_dict.value(itemId);
     
   return retval;
}

/*! Returns the first item in the dictionary or 0 if
 *  none is available.
 *  Items are not sorted.
 */
RefItem *RefList::getFirstRefItem()
{ 
   RefItem *retval = 0;

   if (! m_dict.isEmpty())  {
      m_dictIterator.toFront();
      retval = m_dictIterator.value();
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

   if (m_dictIterator.hasNext())  {         
      m_dictIterator.next();
      retval = m_dictIterator.value();
   }  

   return retval;
}

/*! Returns the name of the list as set in the constructor. */
QString RefList::listName() const
{
   return m_listName;
}

QString RefList::fileName() const
{
   return m_fileName;
}

QString RefList::pageTitle() const
{
   return m_pageTitle;
}

QString RefList::sectionTitle() const
{
   return m_secTitle;
}

void RefList::insertIntoList(const QString &key, RefItem *item)
{     
   bool found = m_itemMap.contains(key);

   if (found) {         
      m_itemMap[key].append(*item);
     
   } else {     
      QList<RefItem> xList;
      xList.append(*item);

      m_itemMap.insert(key, xList);
   }
}

void RefList::generatePage()
{  
   if (m_itemMap.isEmpty()) {
      return;
   }

   QMap<QString, QList<RefItem>> titleMap;

   for (auto list : m_itemMap) {
      RefItem &item = list.first();     
      titleMap.insert(item.title, list);
   }

   //
   QString doc;
   doc += "<dl class=\"reflist\">";

   for (auto list : titleMap) {

      RefItem &item = list.first();

      doc += " <dt>";
      doc +=  "\\anchor ";
      doc += item.listAnchor;
      doc += "\n";

      if (item.scope && item.scope->name() != "<globalScope>") {
         doc += "\\_setscope ";
         doc += item.scope->name();
         doc += " ";
      }

      doc += item.prefix;
      doc += " \\_internalref ";
      doc += item.name;
      doc += " \"";
      doc += item.title;
      doc += "\" ";

      // write declaration in case a function with arguments
      if (! item.args.isEmpty()) {
         doc += item.args;
      }

      doc += "</dt><dd> ";
      doc += item.text;
      
      for (auto xx = list.begin() + 1; xx != list.end(); ++xx) {
         doc += "<p>" + xx->text;
      }

      doc += "</dd>";
   }

   doc += "</dl>\n";

   addRelatedPage(m_listName, m_pageTitle, doc, m_fileName, 1, QVector<ListItemInfo>(), QSharedPointer<GroupDef>());
}
