/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim 
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

#include <doxy_globals.h>
#include <outputgen.h>
#include <message.h>

OutputGenerator::OutputGenerator()
{
   active = true;   
}

OutputGenerator::~OutputGenerator()
{ 
}

void OutputGenerator::startPlainFile(const QString &name)
{
   m_fileName = m_dir + "/" + name;
   m_file.setFileName(m_fileName);

   if (! m_file.open(QIODevice::WriteOnly)) {   
      err("Unable to open file for writing %s, error: %d\n", csPrintable(m_fileName), m_file.error());
      Doxy_Work::stopDoxyPress();
   }

   m_textStream.setDevice(&m_file);
}

void OutputGenerator::endPlainFile()
{
   m_textStream.setDevice(0);

   m_fileName = "";

   m_file.close();
   m_file.setFileName(m_fileName);
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

