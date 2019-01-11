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

