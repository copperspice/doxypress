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

#ifndef PAGEDEF_H
#define PAGEDEF_H

#include <definition.h>
#include <stringmap.h>

class PageSDict;
class OutputList;
class FTextStream;

/** @brief A model of a page symbol. */
class PageDef : public Definition
{
 public:
   PageDef(const char *f, int l, const char *n, const char *d, const char *t);
   ~PageDef();

   // setters
   void setFileName(const char *name, bool dontEscape);
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
   QByteArray getOutputFileBase() const;

   QByteArray anchor() const {
      return QByteArray();
   }

   void findSectionsInDocumentation();
   QByteArray title() const {
      return m_title;
   }

   GroupDef   *getGroupDef() const;
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

   void setPageScope(Definition *d) {
      m_pageScope = d;
   }

   Definition *getPageScope() const {
      return m_pageScope;
   }

   QString displayName(bool = true) const override {
      return !m_title.isEmpty() ? m_title : Definition::name();
   }

   void writeDocumentation(OutputList &ol);
   void writeTagFile(FTextStream &);

 private:
   void setNestingLevel(int l);
   void writePageDocumentation(OutputList &ol);
   QByteArray m_fileName;
   QByteArray m_title;
   PageSDict *m_subPageDict;                 // list of pages in the group
   Definition *m_pageScope;
   int m_nestingLevel;
   bool m_showToc;
};


class PageSDict : public StringMap<QSharedPointer<PageDef>>
{
 public:
   PageSDict() : StringMap<QSharedPointer<PageDef>>() {}
   virtual ~PageSDict() {}

 private:
   int compareValues(const PageDef *i1, const PageDef *i2) const {
      return qstricmp(i1->name(), i2->name());
   }
};

#endif

