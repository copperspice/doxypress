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
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#include <QString>

#include <doxy_setup.h>


/*! \file
 *  \brief main entry point for doxygen
 *
 *  This file contains main()
 */
int main(int argc, char **argv)
{    
   // passed parameters
   QStringList argList;

   for (int k = 1; k < argc; ++k)   {
      QString value = argv[k];     
      argList.append(value);      
   }

   initDoxygen();  

   //    
   struct CommandLine cmdArgs;

   cmdArgs = parseCommandLine(argList);
   readConfiguration(cmdArgs);

   //
   checkConfiguration();
   adjustConfiguration();

   parseInput();
   generateOutput();

   return 0;
}

