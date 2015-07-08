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

#ifndef FORTRANCODE_H
#define FORTRANCODE_H

#include <QByteArray>

#include <types.h>

class CodeOutputInterface;
class FileDef;
class MemberDef;
class Definition;

void parseFortranCode(CodeOutputInterface &, const QString &, const QString &, bool, const QString &, 
                  QSharedPointer<FileDef> fd, int startLine, int endLine, bool inlineFragment,
                  QSharedPointer<MemberDef> memberDef, bool showLineNumbers, QSharedPointer<Definition> searchCtx,
                  bool collectRefs, FortranFormat format);

void resetFortranCodeParserState();
void codeFreeScanner();

#endif
