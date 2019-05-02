/************************************************************************
*
* Copyright (C) 2014-2019 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
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

#ifndef PAGEDEF_H
#define PAGEDEF_H

#include <QTextStream>

#include <definition.h>

class PageSDict;
class OutputList;

/** @brief A model of a page symbol. */
class PageDef : public Definition
{
  public:
   PageDef(const QString &f, int l, const QString &n, const QString &d, const QString &t);
   ~PageDef();

   // setters
   void setFileName(const QString &name);
   void setLocalToc(const LocalToc &tl);
   // emerald, not ready yet void setShowLineNo(bool);

   // getters
   DefType definitionType() const override {
      return TypePage;
   }

   bool isLinkableInProject() const override {
      return /*hasDocumentation() &&*/ !isReference();
   }

   bool isLinkable() const override {
      return isLinkableInProject() || isReference();
   }

   // functions to get a uniform interface with Definitions
   QString getOutputFileBase() const override;

   QString anchor() const  override{
      return QString("");
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
   bool hasTitle() const;

   LocalToc localToc() const {
      return m_localToc;
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

  protected:
   QString pathFragment_Internal() const override;

  private:
   void setNestingLevel(int l);
   void writePageDocumentation(OutputList &ol);

   QString m_fileName;
   QString m_title;

   PageSDict *m_subPageDict;                 // list of pages in the group
   QSharedPointer<Definition> m_pageScope;
   int m_nestingLevel;

   LocalToc m_localToc;
};

#endif

