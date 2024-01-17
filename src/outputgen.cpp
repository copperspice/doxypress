/************************************************************************
*
* Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
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
   m_fileName = m_outputDir + "/" + name;
   m_file.setFileName(m_fileName);

   if (! m_file.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(m_fileName), m_file.error());
      Doxy_Work::stopDoxyPress();
   }

   m_textStream.setDevice(&m_file);
}

void OutputGenerator::endPlainFile()
{
   m_textStream.setDevice(nullptr);
   m_fileName = QString();

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
