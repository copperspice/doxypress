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

#include <arguments.h>
#include <classlist.h>
#include <config.h>
#include <defargs.h>
#include <groupdef.h>
#include <language.h>
#include <outputlist.h>
#include <util.h>

static int compItems(const QSharedPointer<ClassDef> &c1, const QSharedPointer<ClassDef> &c2)
{
   static bool b = Config::getBool("sort-by-scope-name");

   if (b) {
      return c1->name().compare(c2->name(), Qt::CaseInsensitive);

   } else {
      return c1->className().compare(c2->className(), Qt::CaseInsensitive);      
   }
}

int ClassSDict::compareMapValues(const QSharedPointer<ClassDef> &item1, const QSharedPointer<ClassDef> &item2) const
{
   return compItems(item1, item2);
}

bool ClassSDict::declVisible(const ClassDef::CompoundType *filter) const
{
   static bool hideUndocClasses    = Config::getBool("hide-undoc-classes");
   static bool extractLocalClasses = Config::getBool("extract-local-classes");

   if (count() > 0) {
      ClassSDict::Iterator sdi(*this);

      QSharedPointer<ClassDef> cd;

      for (sdi.toFirst(); (cd = sdi.current()); ++sdi) {
         if (cd->name().indexOf('@') == -1 && (filter == 0 || *filter == cd->compoundType()) ) {
            bool isLink = cd->isLinkable();

            if (isLink || (! hideUndocClasses && (! cd->isLocal() || extractLocalClasses) ) ) {
               return true;
            }
         }
      }
   }

   return false;
}

void ClassSDict::writeDeclaration(OutputList &ol, const ClassDef::CompoundType *filter, const QString &header, bool localNames)
{
   static bool extractPrivate = Config::getBool("extract-private");

   if (count() > 0) {

      ClassSDict::Iterator sdi(*this);

      QSharedPointer<ClassDef> cd;
      bool found = false;

      for (sdi.toFirst(); (cd = sdi.current()); ++sdi) {
         
         if (cd->name().indexOf('@') == -1 && ! cd->isExtension() && (cd->protection() != Private || extractPrivate) &&
               (filter == 0 || *filter == cd->compoundType()) ) {
            cd->writeDeclarationLink(ol, found, header, localNames);
         }
      }

      if (found) {
         ol.endMemberList();
      }
   }
}

void ClassSDict::writeDocumentation(OutputList &ol, Definition *container)
{
   static bool fortranOpt = Config::getBool("optimize-fortran");

   static bool inlineGroupedClasses = Config::getBool("inline-grouped-classes");
   static bool inlineSimpleClasses  = Config::getBool("inline-simple-struct");

   if (! inlineGroupedClasses && !inlineSimpleClasses) {
      return;
   }

   if (count() > 0) {
      bool found = false;
           
      for (auto cd : *this) {
         if (cd->name().indexOf('@') == -1 && cd->isLinkableInProject() && cd->isEmbeddedInOuterScope() &&
               (container == 0 || cd->partOfGroups() == 0) ) {

            if (! found) {
               ol.writeRuler();
               ol.startGroupHeader();
               ol.parseText(fortranOpt ? theTranslator->trTypeDocumentation() : theTranslator->trClassDocumentation());
               ol.endGroupHeader();
               found = true;
            }

            cd->writeInlineDocumentation(ol);
         }
      }
   }
}

void GenericsSDict::insert(const QByteArray &key, QSharedPointer<ClassDef> cd)
{
   int i = key.indexOf('<');

   if (i == -1) {
      return;
   }

   ArgumentList argList;
   stringToArgumentList(key.mid(i), &argList);

   int c = argList.count();

   if (c == 0) {
      return;
   }

   QSharedPointer<QHash<long, QSharedPointer<ClassDef>>> collection = m_dict.find(key.left(i));

   if (collection == 0) {
      // new hash

      collection = QSharedPointer<QHash<long, QSharedPointer<ClassDef>>> (new QHash<long, QSharedPointer<ClassDef>>());       

      // add new hash to m_dict
      m_dict.insert(key.left(i), collection);
   }

   if (! collection->contains(c)) {
      // add the collection
      collection->insert(c, cd);
   }
}

QSharedPointer<ClassDef> GenericsSDict::find(const QByteArray &key)
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
         stringToArgumentList(key.mid(i), &argList);

         int c = argList.count();
         return collection->value(c);
      }
   }

   return QSharedPointer<ClassDef>();
}

