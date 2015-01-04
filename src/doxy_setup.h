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

#ifndef DOXY_SETUP_H
#define DOXY_SETUP_H

#include <QStringList>

struct CommandLine {

   QString configName;
   QString layoutName;
   QString debugLabel;
   QString formatName;

   QString rtfStyle;

   QString htmlHead;
   QString htmlFoot;
   QString htmlStyle;

   QString latexHead;
   QString latexFoot;
   QString latexStyle;

   bool genConfig = false;  
   bool genLayout = false;   
};

void initDoxygen();

struct CommandLine parseCommandLine(QStringList argList);
void readConfiguration(struct CommandLine cmdArgs);

void checkConfiguration();
void adjustConfiguration();

void searchInputFiles();      
void parseInput();
void generateOutput();
void readAliases();
void readFormulaRepository();
void cleanUpDoxygen();

#endif