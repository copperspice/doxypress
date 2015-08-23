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

#include <QFile>

#include <stdlib.h>
#include <cassert>

#include <outputgen.h>
#include <message.h>
#include <portable.h>

OutputGenerator::OutputGenerator()
{
   m_filePtr = nullptr;
   active    = true;   
}

OutputGenerator::~OutputGenerator()
{
   delete m_filePtr;   
}

void OutputGenerator::startPlainFile(const QString &name)
{
   m_fileName = m_dir + "/" + name;
   m_filePtr  = new QFile(m_fileName);

   if (! m_filePtr) {
      err("Unable to create file %s\n", qPrintable(m_fileName));
      exit(1);
   }

   if (! m_filePtr->open(QIODevice::WriteOnly)) {   
      err("Unable to open file for writing %s, error: %d\n", qPrintable(m_fileName), m_filePtr->error());
      exit(1);
   }

   m_textStream.setDevice(m_filePtr);
}

void OutputGenerator::endPlainFile()
{
   m_textStream.setDevice(0);

   delete m_filePtr;

   m_filePtr  = nullptr;
   m_fileName = "";
}

void OutputGenerator::pushGeneratorState()
{
   genStack.push( isEnabled() );   
}

void OutputGenerator::popGeneratorState()
{   
   bool lb = genStack.pop();    
 
   if (lb) {
      enable();

   } else {
      disable();

   }  
}

