/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim 
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

#ifndef CODE_CSTYLE_H
#define CODE_CSTYLE_H

#include <QByteArray>
#include <QSharedPointer>

#include <types.h>

class CodeOutputInterface;
class FileDef;
class MemberDef;
class Definition;

void parseCCode(CodeOutputInterface &, const QString &, const QString &, SrcLangExt lang, bool isExample, 
                const QString &exName, QSharedPointer<FileDef> fd, int startLine, int endLine, bool inlineFragment,
                QSharedPointer<MemberDef> memberDef, bool showLineNumbers, QSharedPointer<Definition> searchCtx, bool collectXRefs);

void resetCCodeParserState();
void codeFreeScanner();

#endif
