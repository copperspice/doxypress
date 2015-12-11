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

#ifndef PAGEDEF_H
#define PAGEDEF_H

#include <QTextStream>

#include <definition.h>

class PageSDict;
class OutputList;

template <typename T>
class StringMap;

/** @brief A model of a page symbol. */
class PageDef : public Definition
{
 public:
   PageDef(const QString &f, int l, const QString &n, const QString &d, const QString &t);
   ~PageDef();

   // setters
   void setFileName(const QString &name, bool dontEscape);
   void setShowToc(bool b);

   // getters
   DefType definitionType() const {
      return TypePage;
   }

   bool isLinkableInProject() const {
      return /*hasDocumentation() &&*/ !isReference();
   }

   bool isLinkable() const {
      return isLinkableInProject() || isReference();
   }

   // functions to get a uniform interface with Definitions
   QString getOutputFileBase() const override;

   QString anchor() const  override{
      return "";
   }

   void findSectionsInDocumentation();
   QString title() const {
      return m_title;
   }

   QSharedPointer<GroupDef> getGroupDef() const;

   PageSDict *getSubPages() const {
      return m_subPageDict;
   }

   void addInnerCompound(QSharedPointer<Definition> d) override; 

   bool visibleInIndex() const;
   bool documentedPage() const;
   bool hasSubPages() const;
   bool hasParentPage() const;

   bool showToc() const {
      return m_showToc;
   }

   void setPageScope(QSharedPointer<Definition> d) {
      m_pageScope = d;
   }

   QSharedPointer<Definition> getPageScope() const {
      return m_pageScope;
   }

   QString displayName(bool = true) const override {
      return !m_title.isEmpty() ? m_title : Definition::name();
   }

   void writeDocumentation(OutputList &ol);
   void writeTagFile(QTextStream &);

 private:
   void setNestingLevel(int l);
   void writePageDocumentation(OutputList &ol);

   QString m_fileName;
   QString m_title;

   PageSDict *m_subPageDict;                 // list of pages in the group
   QSharedPointer<Definition> m_pageScope;
   int m_nestingLevel;
   bool m_showToc;
};

#endif

