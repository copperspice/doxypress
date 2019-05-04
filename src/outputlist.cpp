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

#include <outputlist.h>

#include <config.h>
#include <definition.h>
#include <docparser.h>
#include <message.h>
#include <outputgen.h>

OutputList::OutputList()
{
}

OutputList::~OutputList()
{
}

void OutputList::add(QSharedPointer<OutputGenerator> og)
{
   if (og) {
      m_outputs.append(og);
   }
}

void OutputList::disableAllBut(OutputGenerator::OutputType o)
{
   for (auto item : m_outputs ) {
      item->disableIfNot(o);
   }
}

void OutputList::enableAll()
{
   for (auto item : m_outputs ) {
      item->enable();
   }
}

void OutputList::disableAll()
{
   for (auto item : m_outputs ) {
      item->disable();
   }
}

void OutputList::disable(OutputGenerator::OutputType o)
{
   for (auto item : m_outputs ) {
      item->disableIf(o);
   }
}

void OutputList::enable(OutputGenerator::OutputType o)
{
   for (auto item : m_outputs ) {
      item->enableIf(o);
   }
}

bool OutputList::isEnabled(OutputGenerator::OutputType o)
{
   bool result = false;

   for (auto item : m_outputs ) {
      result = result || item->isEnabled(o);
   }

   return result;
}

void OutputList::pushGeneratorState()
{
   for (auto item : m_outputs ) {
      item->pushGeneratorState();
   }
}

void OutputList::popGeneratorState()
{
   for (auto item : m_outputs ) {
      item->popGeneratorState();
   }
}

bool OutputList::generateDoc(const QString &fileName, int startLine, QSharedPointer<Definition> ctx,
                  QSharedPointer<MemberDef> md, const QString &docStr, bool indexWords, bool isExample,
                  const QString &exampleName, bool singleLine, bool linkFromIndex)
{
   if (docStr.isEmpty()) {
      return true;
   }

   int count = 0;

   for (auto item : m_outputs) {
      if (item->isEnabled()) {
         count++;
      }
   }

   if (count == 0) {
      return true;   // no output formats enabled
   }


   DocRoot *root = nullptr;
   root = validatingParseDoc(fileName, startLine, ctx, md, docStr, indexWords, isExample,
                  exampleName, singleLine, linkFromIndex);

   writeDoc(root, ctx, md);

   bool isEmpty = root->isEmpty();
   delete root;

   return isEmpty;
}

void OutputList::writeDoc(DocRoot *root, QSharedPointer<Definition> ctx, QSharedPointer<MemberDef> md)
{
   for (auto item : m_outputs) {
      if (item->isEnabled()) {
         item->writeDoc(root, ctx, md);
      }
   }
}

bool OutputList::parseText(const QString &textStr)
{
   int count = 0;

   for (auto item : m_outputs) {
      if (item->isEnabled()) {
         count++;
      }
   }

   if (count == 0) {
      // no output formats enabled
      return true;
   }

   DocText *root = validatingParseText(textStr);

   for (auto item : m_outputs) {

      if (item->isEnabled()) {
         item->writeDoc(root, QSharedPointer<Definition>(), QSharedPointer<MemberDef>());
      }
   }

   bool isEmpty = root->isEmpty();
   delete root;

   return isEmpty;
}
