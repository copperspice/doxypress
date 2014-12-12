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

#include <QFile>

#include <stdlib.h>
#include <cassert>

#include <outputgen.h>
#include <message.h>
#include <portable.h>

OutputGenerator::OutputGenerator()
{
   file   = 0;
   active = true;   
}

OutputGenerator::~OutputGenerator()
{
   delete file;   
}

void OutputGenerator::startPlainFile(const char *name)
{
   fileName = dir + "/" + name;
   file = new QFile(fileName);

   if (! file) {
      err("Could not create file object for %s\n", fileName.data());
      exit(1);
   }

   if (! file->open(QIODevice::WriteOnly)) {
      err("Could not open file %s for writing\n", fileName.data());
      exit(1);
   }

   t.setDevice(file);
}

void OutputGenerator::endPlainFile()
{
   t.setDevice(0);
   delete file;

   file = 0;
   fileName.resize(0);
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

