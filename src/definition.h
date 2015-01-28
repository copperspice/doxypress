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

#ifndef DEFINITION_H
#define DEFINITION_H

#include <QList>
#include <QHash>
#include <QSharedPointer>
#include <QTextStream>

#include <doxy_shared.h>
#include <sortedlist.h>
#include <types.h>

class DefinitionImpl;
class FileDef;
class GroupDef;
class MemberSDict;
class MemberDef;
class OutputList;

struct ListItemInfo;
struct SectionInfo;

/** Data associated with a detailed description. */
struct DocInfo {
   QByteArray doc;
   int line;
   QByteArray file;
};

/** Data associated with a brief description. */
struct BriefInfo {
   QByteArray doc;
   QByteArray tooltip;
   int line;
   QByteArray file;
};

/** Data associated with description found in the body. */
struct BodyInfo {
   int      startLine;   //!< line number of the start of the definition
   int      endLine;     //!< line number of the end of the definition

   QSharedPointer<FileDef> fileDef;     //!< file definition containing the function body
};

/** Abstract interface for a Definition or DefinitionList */
class DefinitionIntf : public EnableSharedFromThis
{
 public:
   DefinitionIntf() { }

   virtual ~DefinitionIntf() { }

   /*! Types of derived classes */
   enum DefType {
      TypeClass      = 0,
      TypeFile       = 1,
      TypeNamespace  = 2,
      TypeMember     = 3,
      TypeGroup      = 4,
      TypePackage    = 5,
      TypePage       = 6,
      TypeDir        = 7,
      TypeSymbolList = 8
   };

   /*! Use this for dynamic inspection of the type of the derived class */
   virtual DefType definitionType() const = 0;
};

/** The common base class of all entity definitions found in the sources.
 *
 *  This can be a class or a member function, or a file, or a namespace, etc.
 *  Use definitionType() to find which type of definition this is.
 */
class Definition : public DefinitionIntf
{
 public:
  
   Definition(
      const char *defFileName, int defLine, int defColumn,
      const char *name, const char *b = 0, const char *d = 0,
      bool isSymbol = true);
 
   virtual ~Definition();

   // ----  getters -----
  
   /*! Returns the name of the definition */
   const QByteArray &name() const {
      return m_name;
   }

   /*! Returns the name of the definition as it appears in the output */
   virtual QString displayName(bool includeScope = true) const = 0;

   /*! Returns the local name without any scope qualifiers. */
   QByteArray localName() const;

   /*! Returns the fully qualified name of this definition
    */
   virtual QByteArray qualifiedName() const;

   /*! Returns the name of this definition as it appears in the symbol map.
    */
   QByteArray symbolName() const;

   /*! Returns the base file name (without extension) of this definition.
    *  as it is referenced to/written to disk.
    */
   virtual QByteArray getOutputFileBase() const = 0;

   /*! Returns the anchor within a page where this item can be found */
   virtual QByteArray anchor() const = 0;

   /*! Returns the name of the source listing of this definition. */
   virtual QByteArray getSourceFileBase() const;

   /*! Returns the anchor of the source listing of this definition. */
   virtual QByteArray getSourceAnchor() const;

   /*! Returns the detailed description of this definition */
   virtual QByteArray documentation() const;

   /*! Returns the line number at which the detailed documentation was found. */
   int docLine() const;

   /*! Returns the file in which the detailed documentation block was found.
    *  This can differ from getDefFileName().
    */
   QByteArray docFile() const;

   /*! Returns the brief description of this definition. This can include commands. */
   virtual QByteArray briefDescription(bool abbreviate = false) const;

   /*! Returns a plain text version of the brief description suitable for use
    *  as a tool tip.
    */
   QByteArray briefDescriptionAsTooltip()  const;

   /*! Returns the line number at which the brief description was found. */
   int briefLine() const;

   /*! Returns the documentation found inside the body of a member */
   QByteArray inbodyDocumentation() const;

   /*! Returns the file in which the in body documentation was found */
   QByteArray inbodyFile() const;

   /*! Returns the line at which the first in body documentation
       part was found */
   int inbodyLine() const;

   /*! Returns the file in which the brief description was found.
    *  This can differ from getDefFileName().
    */
   QByteArray briefFile() const;

   /*! returns the file in which this definition was found */
   QByteArray getDefFileName() const;

   /*! returns the extension of the file in which this definition was found */
   QByteArray getDefFileExtension() const;

   /*! returns the line number at which the definition was found */
   int getDefLine() const {
      return m_defLine;
   }

   /*! returns the column number at which the definition was found */
   int getDefColumn() const {
      return m_defColumn;
   }

   /*! Returns true iff the definition is documented
    *  (which could be generated documentation)
    *  @see hasUserDocumentation()
    */
   virtual bool hasDocumentation() const;

   /*! Returns true iff the definition is documented by the user. */
   virtual bool hasUserDocumentation() const;

   /*! Returns true iff it is possible to link to this item within this
    *  project.
    */
   virtual bool isLinkableInProject() const = 0;

   /*! Returns true iff it is possible to link to this item. This can
    *  be a link to another project imported via a tag file.
    */
   virtual bool isLinkable() const = 0;

   /*! Returns true iff the name is part of this project and
    *  may appear in the output
    */
   virtual bool isVisibleInProject() const;

   /*! Returns true iff the name may appear in the output */
   virtual bool isVisible() const;

   /*! Returns true iff this item is supposed to be hidden from the output. */
   bool isHidden() const;

   /*! returns true if this entity was artificially introduced, for
    *  instance because it is used to show a template instantiation relation.
    */
   bool isArtificial() const;

   /*! If this definition was imported via a tag file, this function
    *  returns the tagfile for the external project. This can be
    *  translated into an external link target via
    *  Doxygen::tagDestinationDict
    */
   virtual QByteArray getReference() const;

   /*! Returns true if this definition is imported via a tag file. */
   virtual bool isReference() const;

   /*! Returns the first line of the body of this item (applicable to classes and
    *  functions).
    */
   int getStartBodyLine() const;

   /*! Returns the last line of the body of this item (applicable to classes and
    *  functions).
    */
   int getEndBodyLine() const;

   /*! Returns the file in which the body of this item is located or 0 if no
    *  body is available.
    */
   QSharedPointer<FileDef> getBodyDef() const;

   /** Returns the programming language this definition was written in. */
   SrcLangExt getLanguage() const;

   SortedList<QSharedPointer<GroupDef>> *partOfGroups() const;

   QList<ListItemInfo> *xrefListItems() const;

   virtual QSharedPointer<Definition> findInnerCompound(const char *name);
   virtual QSharedPointer<Definition> getOuterScope() const;

   MemberSDict *getReferencesMembers() const;
   MemberSDict *getReferencedByMembers() const;

   bool hasSections() const;
   bool hasSources() const;

   /** returns true if this class has a brief description */
   bool hasBriefDescription() const;

   QByteArray id() const;

   //-----------------------------------------------------------------------------------
   // ----  setters -----
   //-----------------------------------------------------------------------------------

   /*! Sets a new \a name for the definition */
   virtual void setName(const char *name);

   /*! Sets a unique id for the symbol. Used for libclang integration. */
   void setId(const char *name);

   /*! Sets the documentation of this definition to \a d. */
   virtual void setDocumentation(const char *d, const char *docFile, int docLine, bool stripWhiteSpace = true);

   /*! Sets the brief description of this definition to \a b.
    *  A dot is added to the sentence if not available.
    */
   virtual void setBriefDescription(const char *b, const char *briefFile, int briefLine);

   /*! Set the documentation that was found inside the body of an item.
    *  If there was already some documentation set, the new documentation
    *  will be appended.
    */
   virtual void setInbodyDocumentation(const char *d, const char *docFile, int docLine);

   /*! Sets the tag file id via which this definition was imported. */
   void setReference(const char *r);

   /*! Add the list of anchors that mark the sections that are found in the
    * documentation.
    */
   void addSectionsToDefinition(QList<SectionInfo> *anchorList);

   // source references
   void setBodySegment(int bls, int ble);
   void setBodyDef(QSharedPointer<FileDef> fd);
   void addSourceReferencedBy(QSharedPointer<MemberDef>d);
   void addSourceReferences(QSharedPointer<MemberDef>d);

   void setRefItems(const QList<ListItemInfo> *sli);
   void mergeRefItems(QSharedPointer<Definition> d);

   virtual void addInnerCompound(QSharedPointer<Definition> d)
   {};   

   virtual void setOuterScope(QSharedPointer<Definition> d);
   virtual void setHidden(bool b);

   void setArtificial(bool b);
   void setLanguage(SrcLangExt lang);

   //-----------------------------------------------------------------------------------
   // --- actions ----
   //-----------------------------------------------------------------------------------

   QString convertNameToFile(const char *name, bool allowDots = false) const;

   void writeSourceDef(OutputList &ol, const char *scopeName);
   void writeInlineCode(OutputList &ol, const char *scopeName);
   void writeSourceRefs(OutputList &ol, const char *scopeName);
   void writeSourceReffedBy(OutputList &ol, const char *scopeName);
   void makePartOfGroup(QSharedPointer<GroupDef> gd);

   //void writePathFragment(OutputList &ol) const;

   void writeNavigationPath(OutputList &ol) const;
   QByteArray navigationPathAsString() const;

   virtual void writeQuickMemberLinks(OutputList &, QSharedPointer<MemberDef> md) const 
   {}

   virtual void writeSummaryLinks(OutputList &) 
   {}

   QByteArray pathFragment() const;

   /*! Writes the documentation anchors of the definition to
    *  the Doxygen::tagFile stream.
    */
   void writeDocAnchorsToTagFile(QTextStream &);
   void setLocalName(const QString &name);

   void addSectionsToIndex();
   void writeToc(OutputList &ol);

 protected:

   Definition(const Definition &d);

 private:
   void addToMap(const QByteArray &name);
 
   void setSymbolName(const QByteArray &name);

   int  _getXRefListId(const char *listName) const;
   void _writeSourceRefList(OutputList &ol, const char *scopeName,const QByteArray &text, MemberSDict *members, bool);

   void _setBriefDescription(const char *b, const char *briefFile, int briefLine);
   void _setDocumentation(const char *d, const char *docFile, int docLine, bool stripWhiteSpace, bool atTop);
   void _setInbodyDocumentation(const char *d, const char *docFile, int docLine);
   bool _docsAlreadyAdded(const QByteArray &doc, QByteArray &sigList);

   DefinitionImpl *m_impl; // internal structure holding all private data
   QByteArray m_name;

   bool m_isSymbol;

   QByteArray m_symbolName;
   int m_defLine;
   int m_defColumn;
};

/** A list of Definition objects. */
class DefinitionList : public QList<QSharedPointer<Definition>>, public DefinitionIntf
{
 public:
   ~DefinitionList() {}

   DefType definitionType() const {
      return TypeSymbolList;
   }

   int compareValues(QSharedPointer<const Definition> item1, QSharedPointer<const Definition> item2) const {
      return qstricmp(item1->name(), item2->name());
   }

};

/** An iterator for Definition objects in a DefinitionList. */
class DefinitionListIterator : public QListIterator<QSharedPointer<Definition>>
{
 public:
   DefinitionListIterator(const DefinitionList &list)
      : QListIterator<QSharedPointer<Definition>>(list)
   {}

   ~DefinitionListIterator() {}
};

/** Reads a fragment from file \a fileName starting with line \a startLine
 *  and ending with line \a endLine. The result is returned as a string
 *  via \a result. The function returns true if successful and false
 *  in case of an error.
 */
bool readCodeFragment(const char *fileName, int &startLine, int &endLine, QByteArray &result);

#endif
