/************************************************************************
*
* Copyright (c) 2014-2023 Barbara Geller & Ansel Sermersheim
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

#ifndef COMMENTSCAN_H
#define COMMENTSCAN_H

#include <types.h>

class Entry;
class ParserInterface;

/** @file
 *  @brief Interface for the comment block parser */

/** Invokes the comment block parser with the request to parse a
 *  single comment block.
 *  @param[in] parser The language parse that invoked this function.
 *         The comment block parse may invoke
 *         ParserInterface::parsePrototype() in order to parse
 *         the argument of a @@fn command.
 *  @param[in] curEntry The Entry to which the comment block belongs.
 *         Any information (like documentation) that is found in
 *         the comment block will be stored in this entry.
 *  @param[in] comment A string representing the actual comment block.
 *         Note that leading *'s are already stripped from the comment block.
 *  @param[in] fileName The name of the file in which the comment is found.
 *         Mainly used for producing warnings.
 *  @param[in,out] lineNr The line number at which the comment block was found.
 *         When the function returns it will be set to the last line parsed.
 *  @param[in] isBrief true iff this comment block represents a brief description.
 *  @param[in] isJavaDocStyle true iff this comment block is in "JavaDoc" style.
 *         This means that it starts as a brief description until the end of
 *         the sentences is found and then proceeds as a detailed description.
 *  @param[in] isInbody true iff this comment block is located in the body of
 *         a function.
 *  @param[in,out] prot The protection level in which this comment block was
 *         found. Commands in the comment block may override this.
 *  @param[in,out] position The character position within \a comment where the
 *         comment block starts. Typically used in case the comment block
 *         contains multiple structural commands.
 *  @param[out] newEntryNeeded Boolean that is true if the comment block parser
 *         finds that a the comment block finishes the entry and a new one
 *         needs to be started.
 *  @returns true if the comment requires further processing. The
 *         parameter \a newEntryNeeded will typically be true in this case and
 *         \a position will indicate the offset inside the \a comment string
 *         where to proceed parsing. false indicates no further processing is
 *         needed.
 */
bool parseCommentBlock(ParserInterface *parser, QSharedPointer<Entry> curEntry, const QString &comment,
                       const QString &fileName, int &lineNr, bool isBrief, bool isJavaDocStyle,
                       bool isInbody, Protection &prot, int &position, bool &newEntryNeeded);

void groupEnterFile(const QString &file, int line);
void groupLeaveFile(const QString &file, int line);
void groupLeaveCompound(const QString &file, int line, const QString &name);
void groupEnterCompound(const QString &file, int line, const QString &name);

void openGroup(QSharedPointer<Entry> e, const QString &file, int line, bool forceOpen = false);
void closeGroup(QSharedPointer<Entry> e, const QString &file, int line, bool isInline = false, bool forceClose = false);
void initGroupInfo(QSharedPointer<Entry> e);

#endif
