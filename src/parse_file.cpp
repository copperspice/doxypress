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

#include <parse_file.h>

#include <outputgen.h>

void FileParser::parseCode(CodeOutputInterface &codeOutIntf, const QString &, const QString &input,
            SrcLangExt, bool, const QString &,  QSharedPointer<FileDef> fd, int startLine, int endLine,
            bool, QSharedPointer<MemberDef> md, bool showLineNumbers, QSharedPointer<Definition> context, bool)
{
   (void) md;
   (void) context;

   int lineNr = startLine != -1 ? startLine : 1;
   int length = input.length();
   int i = 0;

   while (i < length && (endLine == -1 || lineNr <= endLine)) {
      int j = i;

      while (j < length && input[j] != '\n') {
         j++;
      }

      QString lineStr = input.mid(i, j - i);
      codeOutIntf.startCodeLine(fd && showLineNumbers);

      if (fd  && showLineNumbers) {
         codeOutIntf.writeLineNumber(0, 0, 0, lineNr);
      }

      if (! lineStr.isEmpty()) {
         codeOutIntf.codify(lineStr);
      }

      codeOutIntf.endCodeLine();
      lineNr++;
      i = j + 1;
   }
}

