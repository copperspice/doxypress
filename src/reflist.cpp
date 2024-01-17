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
   m_fileName  = convertNameToFile_internal(listName, false, true);
   m_pageTitle = pageTitle;
   m_secTitle  = secTitle;
}

/*! Adds a new item to the list.
 *  \returns A unique id for this item.
 */
int RefList::addRefItem()
{
   RefItem *item = new RefItem;
   ++m_id;

   m_dict.insert(m_id, item);

   return m_id;
}

/*! Returns an item given it's id that is obtained with addRefItem()
 *  \param itemId item's identifier.
 *  \returns A pointer to the todo item's structure.
 */
RefItem *RefList::getRefItem(int itemId)
{
   RefItem *retval = nullptr;

   retval = m_dict.value(itemId);

   return retval;
}

/*! Returns the first item in the dictionary or 0 if
 *  none is available.
 *  Items are not sorted.
 */
RefItem *RefList::getFirstRefItem()
{
   RefItem *retval = nullptr;

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
   RefItem *retval = nullptr;

   if (m_dictIterator.hasNext())  {
      m_dictIterator.next();
      retval = m_dictIterator.value();
   }

   return retval;
}

bool RefList::isEnabled() const
{
   static const bool todoList  = Config::getBool("generate-todo-list");
   static const bool testList  = Config::getBool("generate-test-list");
   static const bool bugList   = Config::getBool("generate-bug-list");
   static const bool depreList = Config::getBool("generate-deprecate-list");

   if (m_listName == "todo" && ! todoList) {
      return false;

   } else if (m_listName == "test" && ! testList) {
      return false;

   } else if (m_listName == "bug"  && ! bugList) {
      return false;

   } else if (m_listName == "deprecated" && ! depreList) {
      return false;

   }

   return true;
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
   auto iter = m_itemMap.find(key);

   if (iter != m_itemMap.end()) {
      // check if the item is not already in the list
      bool foundItem = iter.value().contains(*item);

      if (! foundItem) {
         iter.value().append(*item);
      }

   } else {
      QList<RefItem> xList;
      xList.append(*item);

      m_itemMap.insert(key, xList);
   }
}

void RefList::generatePage()
{
   if (m_itemMap.isEmpty() || ! isEnabled()) {
      return;
   }

   QMap<QString, QList<RefItem>> titleMap;

   for (auto list : m_itemMap) {
      RefItem &item = list.first();
      titleMap.insert(item.title, list);
   }

   //
   QString doc = "<dl class=\"reflist\">";

   int cnt = 0;

   for (auto list : titleMap) {

      RefItem &item = list.first();

      if (item.name.isEmpty())  {
         continue;
      }

      ++cnt;

      doc += " <dt>";
      doc += "\n";

      if (item.scope && item.scope->name() != "<globalScope>") {
         doc += "\\_setscope ";
         doc += item.scope->name();
         doc += " ";
      }

      doc += item.prefix;
      doc += " \\_internalref ";
      doc += item.name;

      QString escapedTitle = substitute(item.title,"\\","\\\\");

      if (item.scope && (item.scope->definitionType() == Definition::TypeClass ||
                         item.scope->definitionType() == Definition::TypeNamespace ||
                         item.scope->definitionType() == Definition::TypeMember ||
                         item.scope->definitionType() == Definition::TypePackage) ) {

         // prevent Obj-C names from being treated as an emoji
         escapedTitle = substitute(escapedTitle,":","&Colon;");
      }
      doc += " \"" + escapedTitle + "\" ";

      // write declaration in case a function with arguments
      if (! item.args.isEmpty()) {
         QString tmp = item.args;

         tmp = substitute(tmp, "@", "@@");
         tmp = substitute(tmp, "\\","\\\\");

         doc.append(tmp);
      }

      doc += "</dt><dd> \\anchor ";
      doc += item.listAnchor;
      doc += " ";
      doc += item.text;

      for (auto iter = list.begin() + 1; iter != list.end(); ++iter) {
         doc += "<p> \\anchor ";
         doc += iter->listAnchor;
         doc += " ";
         doc += iter->text;
      }

      doc += "</dd>";
   }

   doc += "</dl>\n";
   if (cnt > 0) {

      addRelatedPage(m_listName, m_pageTitle, doc, m_fileName, 1, 1, QVector<ListItemInfo>(),
                  QSharedPointer<GroupDef>(), TagInfo(), true);
   }
}
