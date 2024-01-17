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

#ifndef CODE_CSTYLE_H
#define CODE_CSTYLE_H

#include <QByteArray>
#include <QSharedPointer>

#include <types.h>

class CodeGenerator;
class FileDef;
class MemberDef;
class Definition;

void parseCCode(CodeGenerator &, const QString &, const QString &, SrcLangExt lang, bool isExample,
                const QString &exName, QSharedPointer<FileDef> fd, int startLine, int endLine, bool inlineFragment,
                QSharedPointer<MemberDef> memberDef, bool showLineNumbers, QSharedPointer<Definition> searchCtx, bool collectXRefs);

void resetCCodeParserState();
void codeFreeScanner();

#endif
