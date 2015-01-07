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

#include <outputlist.h>
#include <outputgen.h>
#include <config.h>
#include <message.h>
#include <definition.h>
#include <docparser.h>

/*! \file
 *  This class represents a list of output generators that work in "parallel".
 *  The class only knows about the abstract base class OutputGenerators.
 *  All output is produced by calling a method of this class, which forwards
 *  the call to all output generators.
 */

OutputList::OutputList(bool)
{
   //printf("OutputList::OutputList()\n");
}

OutputList::~OutputList()
{
   //printf("OutputList::~OutputList()\n");
}

void OutputList::add(OutputGenerator *og)
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

bool OutputList::generateDoc(const char *fileName, int startLine, Definition *ctx, MemberDef *md, const QByteArray &docStr,
                             bool indexWords, bool isExample, const char *exampleName, bool singleLine, bool linkFromIndex)
{
   int count = 0;

   if (docStr.isEmpty()) {
      return true;
   }


   for (auto item : m_outputs ) {
      if (item->isEnabled()) {
         count++;
      }
   }

   if (count == 0) {
      return true;   // no output formats enabled.
   }

   DocRoot *root = 0;
   root = validatingParseDoc(fileName, startLine, ctx, md, docStr, indexWords, isExample, exampleName, singleLine, linkFromIndex);
   writeDoc(root, ctx, md);

   bool isEmpty = root->isEmpty();
   delete root;

   return isEmpty;
}

void OutputList::writeDoc(DocRoot *root, Definition *ctx, MemberDef *md)
{
   for (auto item : m_outputs ) {
      if (item->isEnabled()) {
         item->writeDoc(root, ctx, md);
      }
   }  
}

bool OutputList::parseText(const QByteArray &textStr)
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
         item->writeDoc(root, 0, 0);
      }
   }

   bool isEmpty = root->isEmpty();
   delete root;

   return isEmpty;
}


// Create some overloaded definitions of the forall function.
// Using template functions here would have made it a little less
// portable (I guess).

// zero arguments
void OutputList::forall(void (OutputGenerator::*func)())
{
   for (auto item : m_outputs ) {
      if (item->isEnabled()) {
         (item->*func)();
      }
   }
}

// one argument
#define FORALL1(a1,p1)                                        \
void OutputList::forall(void (OutputGenerator::*func)(a1),a1) \
{                                                             \
  for (auto item : m_outputs ) {                              \
    if (item->isEnabled()) (item->*func)(p1);                 \
  }                                                           \
}

// two arguments
#define FORALL2(a1,a2,p1,p2)                                        \
void OutputList::forall(void (OutputGenerator::*func)(a1,a2),a1,a2) \
{                                                                   \
  for (auto item : m_outputs ) {                                    \
    if (item->isEnabled()) (item->*func)(p1,p2);                    \
  }                                                                 \
}

// three arguments
#define FORALL3(a1,a2,a3,p1,p2,p3)                                        \
void OutputList::forall(void (OutputGenerator::*func)(a1,a2,a3),a1,a2,a3) \
{                                                                         \
  for (auto item : m_outputs ) {                                          \
    if (item->isEnabled()) (item->*func)(p1,p2,p3);                       \
  }                                                                       \
}

// four arguments
#define FORALL4(a1,a2,a3,a4,p1,p2,p3,p4)                                        \
void OutputList::forall(void (OutputGenerator::*func)(a1,a2,a3,a4),a1,a2,a3,a4) \
{                                                                               \
  for (auto item : m_outputs ) {                                                \
    if (item->isEnabled()) (item->*func)(p1,p2,p3,p4);                          \
  }                                                                             \
}

// five arguments
#define FORALL5(a1,a2,a3,a4,a5,p1,p2,p3,p4,p5)                                        \
void OutputList::forall(void (OutputGenerator::*func)(a1,a2,a3,a4,a5),a1,a2,a3,a4,a5) \
{                                                                                     \
  for (auto item : m_outputs ) {                                                      \
    if (item->isEnabled()) (item->*func)(p1,p2,p3,p4,p5);                             \
  }                                                                                   \
}

// six arguments
#define FORALL6(a1,a2,a3,a4,a5,a6,p1,p2,p3,p4,p5,p6)                                  \
void OutputList::forall(void (OutputGenerator::*func)(a1,a2,a3,a4,a5,a6),a1,a2,a3,a4,a5,a6) \
{                                                                                     \
  for (auto item : m_outputs ) {                                                      \
    if (item->isEnabled()) (item->*func)(p1,p2,p3,p4,p5,p6);                          \
  }                                                                                   \
}

// seven arguments
#define FORALL7(a1,a2,a3,a4,a5,a6,a7,p1,p2,p3,p4,p5,p6,p7)                      \
void OutputList::forall(void (OutputGenerator::*func)(a1,a2,a3,a4,a5,a6,a7),a1,a2,a3,a4,a5,a6,a7) \
{                                                                                     \
  for (auto item : m_outputs ) {                                                      \
    if (item->isEnabled()) (item->*func)(p1,p2,p3,p4,p5,p6,p7);                       \
  }                                                                                   \
}


// eight arguments
#define FORALL8(a1,a2,a3,a4,a5,a6,a7,a8,p1,p2,p3,p4,p5,p6,p7,p8)                      \
void OutputList::forall(void (OutputGenerator::*func)(a1,a2,a3,a4,a5,a6,a7,a8),a1,a2,a3,a4,a5,a6,a7,a8) \
{                                                                                     \
   for (auto item : m_outputs ) {                                                     \
    if (item->isEnabled()) (item->*func)(p1,p2,p3,p4,p5,p6,p7,p8);                    \
  }                                                                                   \
}

// now instantiate only the ones we need

FORALL1(const char *a1, a1)
FORALL1(char a1, a1)
FORALL1(int a1, a1)
FORALL1(const DotClassGraph &a1, a1)
FORALL1(const DotInclDepGraph &a1, a1)
FORALL1(const DotCallGraph &a1, a1)
FORALL1(const DotDirDeps &a1, a1)
FORALL1(const DotGfxHierarchyTable &a1, a1)
FORALL1(const DotGroupCollaboration &a1, a1)
FORALL1(SectionTypes a1, a1)


FORALL1(bool a1, a1)
FORALL2(bool a1, int a2, a1, a2)
FORALL2(bool a1, bool a2, a1, a2)
FORALL2(const char *a1, bool a2, a1, a2)
FORALL4(const char *a1, const char *a2, const char *a3, bool a4, a1, a2, a3, a4)

FORALL2(const QString &a1, bool a2, a1, a2);


FORALL2(int a1, bool a2, a1, a2)
FORALL2(bool a1, const char *a2, a1, a2)
FORALL2(ParamListTypes a1, const char *a2, a1, a2)
FORALL1(IndexSections a1, a1)
FORALL2(const char *a1, const char *a2, a1, a2)
FORALL2(const char *a1, int a2, a1, a2)
FORALL2(const char *a1, SectionInfo::SectionType a2, a1, a2)
FORALL3(bool a1, HighlightedItem a2, const char *a3, a1, a2, a3)
FORALL3(bool a1, bool a2, bool a3, a1, a2, a3)
FORALL3(const ClassDiagram &a1, const char *a2, const char *a3, a1, a2, a3)
FORALL3(const char *a1, const char *a2, const char *a3, a1, a2, a3)
FORALL3(const char *a1, const char *a2, bool a3, a1, a2, a3)
FORALL3(const char *a1, int a2, const char *a3, a1, a2, a3)
FORALL3(const char *a1, const char *a2, SectionInfo::SectionType a3, a1, a2, a3)
FORALL3(uchar a1, uchar a2, uchar a3, a1, a2, a3)
FORALL3(Definition *a1, const char *a2, bool a3, a1, a2, a3)
FORALL4(SectionTypes a1, const char *a2, const char *a3, const char *a4, a1, a2, a3, a4)
FORALL4(const char *a1, const char *a2, const char *a3, const char *a4, a1, a2, a3, a4)
FORALL4(const char *a1, const char *a2, const char *a3, int a4, a1, a2, a3, a4)
FORALL5(const char *a1, const char *a2, const char *a3, const char *a4, const char *a5, a1, a2, a3, a4, a5)
FORALL5(const char *a1, const char *a2, const char *a3, const char *a4, bool a5, a1, a2, a3, a4, a5)
FORALL6(const char *a1, const char *a2, const char *a3, const char *a4, const char *a5, const char *a6, a1, a2, a3, a4, a5, a6)
FORALL6(const char *a1, const DocLinkInfo &a2, const char *a3, const char *a4, const SourceLinkInfo &a5,
        const SourceLinkInfo &a6, a1, a2, a3, a4, a5, a6)

