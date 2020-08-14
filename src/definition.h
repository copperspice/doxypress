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

#ifndef DEFINITION_H
#define DEFINITION_H

#include <QHash>
#include <QSharedPointer>
#include <QTextStream>
#include <QVector>

#include <doxy_shared.h>
#include <sortedlist.h>
#include <stringmap.h>
#include <types.h>

class Definition_Private;
class FileDef;
class GroupDef;
class MemberSDict;
class MemberDef;
class OutputList;

struct ListItemInfo;
struct SectionInfo;

// Data associated with a detailed description
struct DocInfo {

   DocInfo() : line(1) {};

   int line;
   QString doc;
   QString file;
};

// Data associated with a brief description
struct BriefInfo {

   BriefInfo() : line(1) {};

   int line;
   QString doc;
   QString tooltip;
   QString file;
};

// Abstract interface for a Definition or DefinitionList
class DefinitionIntf : public EnableSharedFromThis
{
 public:
   DefinitionIntf() { }
   virtual ~DefinitionIntf() { }

   // Types of derived classes
   enum DefType {
      TypeClass      = 0,
      TypeConcept    = 1,
      TypeFile       = 2,
      TypeNamespace  = 3,
      TypeMember     = 4,
      TypeGroup      = 5,
      TypePackage    = 6,
      TypePage       = 7,
      TypeDir        = 8,
      TypeSymbolList = 9
   };

   // Use this for dynamic inspection of the type of the derived class
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
   Definition(const QString &defFileName, int defLine, int defColumn, const QString &name,
      const QString &b = QString(), const QString &d = QString(), bool isPhrase = true);

   virtual ~Definition();

   // Returns the name of the definition
   const QString &name() const {
      return m_name;
   }

   // Returns the name of the definition as it appears in the output
   virtual QString displayName(bool includeScope = true) const = 0;

   // Returns the local name without any scope qualifiers
   QString localName() const;

   // Returns the fully qualified name of this definition
   virtual QString qualifiedName() const;

   // Returns the name of this definition as it appears in the glossary map
   QString phraseName() const;

   // Returns the base file name (without extension) of this definition,
   // as it is referenced to/written to disk.

   virtual QString getOutputFileBase() const = 0;

   /*! Returns the anchor within a page where this item can be found */
   virtual QString anchor() const = 0;

   /*! Returns the name of the source listing of this definition. */
   virtual QString getSourceFileBase() const;

   /*! Returns the anchor of the source listing of this definition. */
   virtual QString getSourceAnchor() const;

   /*! Returns the detailed description of this definition */
   virtual QString documentation() const;

   /*! Returns the line number at which the detailed documentation was found. */
   int docLine() const;

   // Returns the file in which the detailed documentation block was found.
   //  This can differ from getDefFileName().

   QString docFile() const;

   /*! Returns the brief description of this definition. This can include commands. */
   virtual QString briefDescription(bool abbreviate = false) const;

   // Returns a plain text version of the brief description suitable for use
   //  as a tool tip.

   QString briefDescriptionAsTooltip()  const;

   // Returns the line number at which the brief description was found.
   int briefLine() const;

   // Returns the documentation found inside the body of a member
   QString inbodyDocumentation() const;

   /*! Returns the file in which the in body documentation was found */
   QString inbodyFile() const;

   /*! Returns the line at which the first in body documentation
       part was found */
   int inbodyLine() const;

   /*! Returns the file in which the brief description was found.
    *  This can differ from getDefFileName().
    */
   QString briefFile() const;

   /*! returns the file in which this definition was found */
   QString getDefFileName() const;

   /*! returns the extension of the file in which this definition was found */
   QString getDefFileExtension() const;

   /*! returns the line number at which the definition was found */
   int getDefLine() const {
      return m_defLine;
   }

   /*! returns the column number at which the definition was found */
   int getDefColumn() const {
      return m_defColumn;
   }

   // Returns true if the definition is documented  (which could be generated documentation)
   virtual bool hasDocumentation() const;

   // Returns true iff the definition is documented by the user.
   virtual bool hasUserDocumentation() const;

   // Returns true iff it is possible to link to this item within this   project.
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
    *  translated into an external link target via Doxy_Globals::tagDestinationDict
    */
   virtual QString getReference() const;

   /*! Returns true if this definition is imported via a tag file. */
   virtual bool isReference() const;

   // Convenience method to return a resolved external link
   QString externalReference(const QString &relPath) const;
   // Returns the first line of the body of this item (applicable to classes and  functions).

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
   bool isLinkableViaGroup() const;

   virtual QSharedPointer<Definition> findInnerCompound(const QString &name) const;
   virtual QSharedPointer<Definition> getOuterScope() const;

   const MemberSDict &getReferencesMembers() const;
   const MemberSDict &getReferencedByMembers() const;

   // returns the section dictionary, only of importance for pagedef
   SectionDict &getSectionDict() const;

   bool hasSections() const;
   bool hasSources() const;

   /** returns true if this class has a brief description */
   bool hasBriefDescription() const;

   QString id() const;

   int getInputOrderId() {
      return m_inputOrderId;
   }

   int getSortId() {
      return m_sortId;
   }

   // Sets a new name for the definition
   virtual void setName(const QString &name);

   // Sets a unique id for the definition. Used for libclang integration.
   void setId(const QString &name);

   // Set a new file name and position
   void setDefFile(const QString & defFile, int defLine, int defColumn);

   // Sets the documentation of this definition to \a d.
   virtual void setDocumentation(const QString &d, const QString &docFile, int docLine,
                  bool stripWhiteSpace = true, bool atTop = false);

   // Sets the brief description of this definition to \a b.
   // A dot is added to the sentence if not available.
   virtual void setBriefDescription(const QString &b, const QString &briefFile, int briefLine);

   // Set the documentation that was found inside the body of an item.
   // If there was already some documentation set, the new documentation will be appended.
   virtual void setInbodyDocumentation(const QString &d, const QString &docFile, int docLine);

   // Sets the tag file id via which this definition was imported.
   void setReference(const QString &r);

   // concepts
   const QString & getRequires() const {
      return m_requires;
   }

   void setRequires(QString text) {
      m_requires = std::move(text);
   }

   const QString & getConstraint() const {
      return m_constraint;
   }

   void setConstraint(QString text) {
      m_constraint = std::move(text);
   }

   // Add the list of anchors that mark the sections that are found in the documentation.
   void addSectionsToDefinition(const QVector<SectionInfo> &anchorList);
   void addSourceReferencedBy(QSharedPointer<MemberDef>d);
   void addSourceReferences(QSharedPointer<MemberDef>d);

   void addSectionsToIndex(bool addToNavIndex);
   virtual void addInnerCompound(QSharedPointer<Definition>) {
   }

   void mergeRefItems(QSharedPointer<Definition> d);

   const QVector<ListItemInfo> &getRefItems() const;
   QVector<ListItemInfo> &getRefItems();

   // source references
   void setBodySegment(int bls, int ble);
   void setBodyDef(QSharedPointer<FileDef> fd);
   void setRefItems(const QVector<ListItemInfo> &list);

   virtual void setOuterScope(QSharedPointer<Definition> d);
   virtual void setHidden(bool b);

   void setArtificial(bool b);
   void setLanguage(SrcLangExt lang);

   void setInputOrderId(int id) {
      m_inputOrderId = id;
   }

   void setSortId(int id) {
      m_sortId = id;
   }

   QString convertNameToFile(const QString &name, bool allowDots = false) const;

   QString navigationPathAsString() const;
   QString pathFragment() const;
   void setLocalName(const QString &name);

   void makePartOfGroup(QSharedPointer<GroupDef> gd);

   void writeSourceDef(OutputList &ol, const QString &scopeName);
   void writeInlineCode(OutputList &ol, const QString &scopeName);
   void writeSourceRefs(OutputList &ol, const QString &scopeName);
   void writeSourceReffedBy(OutputList &ol, const QString &scopeName);
   void writeNavigationPath(OutputList &ol) const;
   void writeDocAnchorsToTagFile(QTextStream &) const;
   void writeToc(OutputList &ol, const LocalToc &localToc) const;

   virtual void writeQuickMemberLinks(OutputList &, QSharedPointer<MemberDef> md) const {
      (void) md;
   }

   virtual void writeSummaryLinks(OutputList &) { }

 protected:
   Definition(const Definition &d);
   virtual QString pathFragment_Internal() const;

 private:
   void addToMap(const QString &name);

   void setPhraseName(const QString &phrase);

   int  _getXRefListId(const QString &listName) const;
   void _writeSourceRefList(OutputList &ol, const QString &scopeName,const QString &text,
                  const MemberSDict &members);

   void _setInbodyDocumentation(const QString &d, const QString &docFile, int docLine);
   bool _docsAlreadyAdded(const QString &doc, QString &sigList);

   Definition_Private *m_private;
   QString m_name;

   // concepts
   QString m_requires;
   QString m_constraint;

   QString m_phraseName;
   bool m_isPhrase;

   int m_defLine;
   int m_defColumn;
   int m_inputOrderId;
   int m_sortId;
};

#endif
