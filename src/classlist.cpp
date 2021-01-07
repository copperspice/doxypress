/************************************************************************
*
* Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
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

#include <classlist.h>

#include <arguments.h>
#include <config.h>
#include <default_args.h>
#include <groupdef.h>
#include <language.h>
#include <outputlist.h>
#include <util.h>

static int compItems(const QSharedPointer<ClassDef> &c1, const QSharedPointer<ClassDef> &c2)
{
   static const bool sortByScopeName = Config::getBool("sort-by-scope-name");

   if (sortByScopeName) {
      return c1->name().compare(c2->name(), Qt::CaseInsensitive);

   } else {
      return c1->className().compare(c2->className(), Qt::CaseInsensitive);
   }
}

int ClassSDict::compareMapValues(const QSharedPointer<ClassDef> &item1, const QSharedPointer<ClassDef> &item2) const
{
   return compItems(item1, item2);
}

bool ClassSDict::declVisible(const enum CompoundType *filter) const
{
   static const bool hideUndocClasses    = Config::getBool("hide-undoc-classes");
   static const bool extractLocalClasses = Config::getBool("extract-local-classes");

   if (count() > 0) {
      QSharedPointer<ClassDef> cd;

      for (auto cd : *this) {

         if (! cd->name().contains('@') && (filter == 0 || *filter == cd->compoundType()) ) {
            bool isLink = cd->isLinkable();

            if (isLink || (! hideUndocClasses && (! cd->isLocal() || extractLocalClasses) ) ) {
               return true;
            }
         }
      }
   }

   return false;
}

void ClassSDict::writeDeclaration(OutputList &ol, const enum CompoundType *filter, const QString &header, bool localNames)
{
   static const bool extractPrivate = Config::getBool("extract-private");

   if (count() > 0) {
      QSharedPointer<ClassDef> cd;
      bool found = false;

      for (auto cd : *this) {

         if (! cd->name().contains('@') && ! cd->isExtension() && (cd->protection() != Private || extractPrivate) &&
               (filter == 0 || *filter == cd->compoundType()) ) {
            cd->writeDeclarationLink(ol, found, header, localNames);
         }
      }

      if (found) {
         ol.endMemberList();
      }
   }
}

void ClassSDict::writeDocumentation(OutputList &ol, QSharedPointer<Definition> container)
{
   static const bool optimizeFortran      = Config::getBool("optimize-fortran");
   static const bool inlineGroupedClasses = Config::getBool("inline-grouped-classes");
   static const bool inlineSimpleClasses  = Config::getBool("inline-simple-struct");

   if (! inlineGroupedClasses && ! inlineSimpleClasses) {
      return;
   }

   if (count() > 0) {
      bool found = false;

      for (auto cd : *this) {
         if (! cd->name().contains('@') && cd->isLinkableInProject() && cd->isEmbeddedInOuterScope() &&
               (container == 0 || cd->partOfGroups() == 0) ) {

            if (! found) {
               ol.writeRuler();
               ol.startGroupHeader();
               ol.parseText(optimizeFortran ? theTranslator->trTypeDocumentation() : theTranslator->trClassDocumentation());
               ol.endGroupHeader();
               found = true;
            }

            cd->writeInlineDocumentation(ol);
         }
      }
   }
}

void GenericsSDict::insert(const QString &key, QSharedPointer<ClassDef> cd)
{
   int i = key.indexOf('<');

   if (i == -1) {
      return;
   }

   ArgumentList argList;
   argList = stringToArgumentList(key.mid(i));

   int c = argList.count();

   if (c == 0) {
      return;
   }

   QSharedPointer<QHash<long, QSharedPointer<ClassDef>>> collection = m_dict.find(key.left(i));

   if (collection == 0) {
      // new hash
      collection = QMakeShared<QHash<long, QSharedPointer<ClassDef>>>();

      // add new hash to m_dict
      m_dict.insert(key.left(i), collection);
   }

   if (! collection->contains(c)) {
      // add the collection
      collection->insert(c, cd);
   }
}

QSharedPointer<ClassDef> GenericsSDict::find(const QString &key)
{
   int i = key.indexOf('<');

   if (i == -1) {
       QSharedPointer<QHash<long, QSharedPointer<ClassDef>>> collection = m_dict.find(key);

      if (collection && collection->count() == 1) {
         return collection->begin().value();
      }

   } else {
      QSharedPointer<QHash<long, QSharedPointer<ClassDef>>> collection = m_dict.find(key.left(i));

      if (collection) {
         ArgumentList argList;
         argList = stringToArgumentList(key.mid(i));

         int c = argList.count();
         return collection->value(c);
      }
   }

   return QSharedPointer<ClassDef>();
}

