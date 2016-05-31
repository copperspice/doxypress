/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim 
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

#ifndef PARSE_BASE_H
#define PARSE_BASE_H

#include <QHash>
#include <QStringList>

#include <doxy_globals.h>
#include <types.h>

class Entry;
class FileDef;
class CodeOutputInterface;
class MemberDef;
class Definition;

/** \brief Abstract interface for programming language parsers.
 *
 *  By implementing the methods of this interface you can add a new language parser to DoxyPress. 
 *  The parser can make use of the comment block parser to parse the contents of special comment blocks.
 */
class ParserInterface
{
 public:
   virtual ~ParserInterface() {}
  
   // Called after all files in a translation unit have been processed   
   virtual void finishTranslationUnit() = 0;

   /** Parses a single input file with the goal to build an Entry tree.
    *  @param[in] fileName    The full name of the file.
    *  @param[in] fileBuf     The contents of the file (zero terminated).
    *  @param[in,out] root    The root of the tree of Entry *nodes
    *             representing the information extracted from the file.
    *  @param[in] sameTranslationUnit true if this file was found in the same
    *             translation unit (in the filesInSameTranslationUnit list
    *             returned for another file).
    *  @param[in,out] filesInSameTranslationUnit other files expected to be
    *              found in the same translation unit (used for libclang)
    */
   virtual void parseInput(const QString &fileName, const QString &fileBuf, QSharedPointer<Entry>root,
                           enum ParserMode mode, QStringList &includeFiles, bool useClang = false) = 0;

   /** Returns true if the language identified by \a extension needs
    *  the C preprocessor to be run before feed the result to the input
    *  parser.
    *  @see parseInput()
    */
   virtual bool needsPreprocessing(const QString &extension) = 0;

   /** Parses a source file or fragment with the goal to produce
    *  highlighted and cross-referenced output.
    *  @param[in] codeOutIntf Abstract interface for writing the result.
    *  @param[in] lang The programming language of the code fragment.
    *  @param[in] scopeName Name of scope to which the code belongs.
    *  @param[in] input Actual code in the form of a string
    *  @param[in] isExampleBlock true iff the code is part of an example.
    *  @param[in] exampleName Name of the example.
    *  @param[in] fileDef File definition to which the code
    *             is associated.
    *  @param[in] startLine Starting line in case of a code fragment.
    *  @param[in] endLine Ending line of the code fragment.
    *  @param[in] inlineFragment Code fragment that is to be shown inline
    *             as part of the documentation.
    *  @param[in] memberDef Member definition to which the code
    *             is associated (non null in case of an inline fragment
    *             for a member).
    *  @param[in] showLineNumbers if set to true and also fileDef is not 0,
    *             line numbers will be added to the source fragement
    *  @param[in] searchCtx context under which search data has to be stored.
    *  @param[in] collectXRefs collect cross-reference relations.
    */
   virtual void parseCode(CodeOutputInterface &codeOutIntf, const QString &scopeName, const QString &input, SrcLangExt lang,
                          bool isExampleBlock, const QString &exampleName = QString(), 
                          QSharedPointer<FileDef> fileDef = QSharedPointer<FileDef>(),
                          int startLine = -1, int endLine = -1, bool inlineFragment = false,
                          QSharedPointer<MemberDef> memberDef = QSharedPointer<MemberDef>(), bool showLineNumbers = true,
                          QSharedPointer<Definition> searchCtx = QSharedPointer<Definition>(), bool collectXRefs = true ) = 0;

   /** Resets the state of the code parser.
    *  Since multiple code fragments can together form a single example, an
    *  explicit function is used to reset the code parser state.
    *  @see parseCode()
    */
   virtual void resetCodeParserState() = 0;

   /** Callback function called by the comment block scanner.
    *  It provides a string \a text containing the prototype of a function
    *  or variable. The parser should parse this and store the information
    *  in the Entry node that corresponds with the node for which the
    *  comment block parser was invoked.
    */
   virtual void parsePrototype(const QString &text) = 0;

};

/** \brief Manages programming language parsers.
 *
 *  This class manages the language parsers in the system. One can
 *  register parsers, and obtain a parser given a file extension.
 */
class ParserManager
{
 public:
   /** Creates the parser manager object.
    */
   ParserManager() : m_defaultParser(0)
   {
   }

   ~ParserManager() {
      delete m_defaultParser;
   }

   void registerDefaultParser(ParserInterface *parser) {
      m_defaultParser = parser;
   }

   /** Registers an additional parser.
    *  @param[in] name      A symbolic name of the parser, i.e. "c", "python", "fortran" 
    *  @param[in] parser    The parser that is to be used for the given name.
    */
   void registerParser(const QString &name, ParserInterface *parser) {
      m_parsers.insert(name, parser);
   }

   /** Registers a file \a extension with a parser with name \a parserName.
    *  Returns true if the extension was successfully registered.
    */
   bool registerExtension(const QString &extension, const QString &parserName) {

      if (extension.isEmpty() || parserName.isEmpty()) {
         return false;
      }

      ParserInterface *intf = m_parsers.value(parserName);

      if (intf == nullptr) {
         return false;
      }

      if (m_extensions.find(extension) !=  m_extensions.end()) { 
         // extension already exists
         m_extensions.remove(extension);
      }

      // add new mapping
      m_extensions.insert(extension, intf);

      return true;
   }

   /** Gets the interface to the parser associated with given \a extension.
    *  If there is no parser explicitly registered for the supplied extension,
    *  the interface to the default parser will be returned.
    */
   ParserInterface *getParser(const QString &extension) {
      QString ext = extension.toLower();

      if (ext.isEmpty()) {
         ext = ".no_extension";
      }

      ParserInterface *intf = m_extensions.value(ext);

      if (intf == 0 && ext.length() > 4) {
         intf = m_extensions.value(ext.left(4));
      }

      if (intf) { 
         return intf; 

      } else {
         return m_defaultParser;
      }      
   }

 private:
   QHash<QString, ParserInterface *> m_parsers;
   QHash<QString, ParserInterface *> m_extensions;

   ParserInterface *m_defaultParser;
};

#endif
