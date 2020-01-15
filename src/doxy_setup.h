/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
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

#ifndef DOXY_SETUP_H
#define DOXY_SETUP_H

#include <QFile>
#include <QString>
#include <QStringList>

struct CommandLine {

   QString dateTimeStr;

   QString configName;
   QString layoutName;
   QString debugLabel;
   QString formatName;

   QString rtfExt;
   QString rtfStyle;

   QString htmlHead;
   QString htmlFoot;
   QString htmlStyle;

   QString latexHead;
   QString latexFoot;
   QString latexStyle;

   bool generateLayout = false;
   bool generateDoxy   = true;
};

void initDoxyPress();

CommandLine parseCommandLine(QStringList argList);
void readConfiguration(struct CommandLine cmdArgs);

void checkConfiguration();
void adjustConfiguration();

void searchInputFiles();

void processFiles();
void generateOutput();

void readFormulaRepository();
void shutDownDoxyPress();

void initDefaultLangMapping();
void initUserLangMapping();
void addCodeOnlyMappings();

bool openOutputFile(const QString &outFile, QFile &f);
bool updateLanguageMapping(const QString &extension, const QString &parser, bool userParser = false);

#endif