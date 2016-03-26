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

#ifndef ENTRY_H
#define ENTRY_H

#include <bitset>

#include <QString>
#include <QFile>
#include <QList>

#include <types.h>

#include <arguments.h>

class EntryNav;
class FileDef;
class FileStorage;
class StorageIntf;

struct SectionInfo;
struct ListItemInfo;

// this class stores information about an inheritance relation
struct BaseInfo {
   /* Creates an object representing an inheritance relation */

   BaseInfo(const QString &n, Protection p, Specifier v)
      : name(n), prot(p), virt(v) {}

   QString    name;      // the name of the base class
   Protection prot;      // inheritance type
   Specifier  virt;      //  virtualness
};

// This struct is used to capture the tag file information for an Entry.
struct TagInfo {
   QString tagName;
   QString fileName;
   QString anchor;
};

/** Represents some information, about an entity found in the source.
 *
 *  parseMain() will generate a tree of these entries.
 */
class Entry
{
 public:

   // supported entries
   enum Sections {
      CLASS_SEC        = 0x00000001,
      NAMESPACE_SEC    = 0x00000010,
      COMPOUND_MASK    = CLASS_SEC,
      SCOPE_MASK       = COMPOUND_MASK | NAMESPACE_SEC,

      CLASSDOC_SEC     = 0x00000800,
      STRUCTDOC_SEC    = 0x00001000,
      UNIONDOC_SEC     = 0x00002000,
      EXCEPTIONDOC_SEC = 0x00004000,
      NAMESPACEDOC_SEC = 0x00008000,
      INTERFACEDOC_SEC = 0x00010000,
      PROTOCOLDOC_SEC  = 0x00020000,
      CATEGORYDOC_SEC  = 0x00040000,
      SERVICEDOC_SEC   = 0x00080000,
      SINGLETONDOC_SEC = 0x00100000,
      COMPOUNDDOC_MASK = CLASSDOC_SEC | STRUCTDOC_SEC | UNIONDOC_SEC |
                         INTERFACEDOC_SEC | EXCEPTIONDOC_SEC | PROTOCOLDOC_SEC |
                         CATEGORYDOC_SEC | SERVICEDOC_SEC | SINGLETONDOC_SEC,

      SOURCE_SEC       = 0x00400000,
      HEADER_SEC       = 0x00800000,
      FILE_MASK        = SOURCE_SEC | HEADER_SEC,

      ENUMDOC_SEC      = 0x01000000,
      ENUM_SEC         = 0x02000000,
      EMPTY_SEC        = 0x03000000,
      PAGEDOC_SEC      = 0x04000000,
      VARIABLE_SEC     = 0x05000000,
      FUNCTION_SEC     = 0x06000000,
      TYPEDEF_SEC      = 0x07000000,
      MEMBERDOC_SEC    = 0x08000000,
      OVERLOADDOC_SEC  = 0x09000000,
      EXAMPLE_SEC      = 0x0a000000,
      VARIABLEDOC_SEC  = 0x0b000000,
      FILEDOC_SEC      = 0x0c000000,
      DEFINEDOC_SEC    = 0x0d000000,
      INCLUDE_SEC      = 0x0e000000,
      DEFINE_SEC       = 0x0f000000,
      GROUPDOC_SEC     = 0x10000000,
      USINGDIR_SEC     = 0x11000000,
      MAINPAGEDOC_SEC  = 0x12000000,
      MEMBERGRP_SEC    = 0x13000000,
      USINGDECL_SEC    = 0x14000000,
      PACKAGE_SEC      = 0x15000000,
      PACKAGEDOC_SEC   = 0x16000000,
      OBJCIMPL_SEC     = 0x17000000,
      DIRDOC_SEC       = 0x18000000,

      EXPORTED_INTERFACE_SEC = 0x19000000,
      INCLUDED_SERVICE_SEC   = 0x1A000000
   };

   enum Virtue {     

      // class specifiers
      AbstractClass,
      Template,
      Generic,
      Ref,
      Value,
      Interface,
      Struct,
      Union,
      Exception,
      Protocol,
      Category,
      SealedClass,

      Enum,                    // Java-style enums
      Service,                 // UNO IDL
      Singleton,               // UNO IDL
      ForwardDecl,             

      // method specifiers     
      Readable,
      Writable,
      Reset,
      Notify,
      Revision,
      Designable,
      Scriptable,
      Stored,
      User,
      Constant,
      Final,
      Assign,

      // member specifiers
      Deprecated,
      PrivateGettable,         // C# private getter
      ProtectedGettable,       // C# protected getter
      PrivateSettable,         // C# private setter
      ProtectedSettable,       // C# protected setter

      Inline,
      Explicit,
      Mutable,
      Settable,
      Gettable,
      Abstract,
      Addable,
      Removable,
      Raisable,
      Override,
      New,
      Sealed,
      Initonly,
      Optional,
      Required,
      NonAtomic,
      Copy,
      Retain,
      Strong,
      Weak,
      Unretained,

      Alias,
      ConstExp,
      Default,
      Delete,
      NoExcept,
      Attribute,               // UNO IDL attribute
      Property,                // UNO IDL property
      Readonly,                // on UNO IDL attribute or property
      Bound,                   // on UNO IDL attribute or property
      Constrained,             // on UNO IDL property
      Transient,               // on UNO IDL property
      MaybeVoid,               // on UNO IDL property
      MaybeDefault,            // on UNO IDL property
      MaybeAmbiguous,          // on UNO IDL property
      Published,               // UNO IDL keyword

      LastVirtue = Published
   };

   // kind of group
   enum GroupDocType {
      GROUPDOC_NORMAL,        // defgroup
      GROUPDOC_ADD,           // addgroup
      GROUPDOC_WEAK           // weakgroup
   };

   class Traits {

      public:
         Traits & operator|=(const Traits &other) {
            this->m_flags |= other.m_flags;

            return *this;
         }

         QByteArray toQByteArray() const {
            return m_flags.to_string().c_str();
         }

         static Traits fromQByteArray(const QByteArray &data)  {
            Traits retval;
            retval.m_flags = std::bitset<Virtue::LastVirtue> { data.constData() };

            return retval;
         }

         void clear()  {
            m_flags.reset();
         }

         bool hasTrait(Entry::Virtue data)  const {
            return m_flags[data];
         }

         void setTrait(Entry::Virtue data, bool value = true)  {
            m_flags[data] = value;
         }

         bool onlyHasTrait(Entry::Virtue data)  const {
            return m_flags[data] && m_flags.count() == 1;
         }

      private:
         std::bitset<Virtue::LastVirtue> m_flags;
   };  

   Entry();
   Entry(const Entry &);
   ~Entry();

   // Returns the static size of the Entry (so excluding any dynamic memory)
   int getSize();

   void addSpecialListItem(const QString &listName, int index);
   void createNavigationIndex(QSharedPointer<EntryNav> rootNav, FileStorage *storage, QSharedPointer<FileDef> fd,
                              QSharedPointer<Entry> self);

   // while parsing a file these function can be used to navigate/build the tree
   void setParent(QSharedPointer<Entry> parent) {
      m_parent = parent;
   }

   // Returns the parent for this Entry or 0 if this entry has no parent.
   QSharedPointer<Entry> parent() const {
      return m_parent.toStrongRef();
   }

   /*! Returns the list of children for this Entry
    *  @see addSubEntry() and removeSubEntry()
    */
   const QList<QSharedPointer<Entry>> children() const {
      return m_sublist;
   }

   // Adds entry \a e as a child to this entry /
   void addSubEntry (QSharedPointer<Entry> e, QSharedPointer<Entry> self) ;

   // Removes entry \a e from the list of children
   void removeSubEntry(QSharedPointer<Entry> e);

   // Restore the state of this Entry to the default value it has at construction time.
   void reset();

 public:  
   TagInfo      *tagInfo;       // tag file info
   ArgumentList  argList;       // member arguments as a list
   ArgumentList  typeConstr;    // where clause (C#) for type constraints

   RelatesType  relatesType;    // how relates is handled
   Specifier    virt;           // virtualness of the entry
   Protection   protection;     // class protection
   MethodTypes  mtype;          // signal, slot, (dcop) method, or property
   GroupDocType groupDocType;
   SrcLangExt   lang;           // programming language in which this entry was found

   Traits m_traits;
   
   static int num;              // counts the total number of entries

   int  section;                // entry type (see Sections);
   int  initLines;              // define/variable initializer lines to show
   int  docLine;                // line number at which the documentation was found
   int  briefLine;              // line number at which the brief desc. was found
   int  inbodyLine;             // line number at which the body doc was found
   int  bodyLine;               // line number of the definition in the source
   int  endBodyLine;            // line number where the definition ends
   int  mGrpId;                 // member group id
   int  startLine;              // start line of entry in the source
   int  startColumn;            // start column of entry in the source

   bool stat;                   // static ?
   bool explicitExternal;       // explicitly defined as external ?
   bool proto;                  // prototype ?
   bool subGrouping;            // automatically group class members ?
   bool callGraph;              // do we need to draw the call graph ?
   bool callerGraph;            // do we need to draw the caller graph ?

   QList<ArgumentList>   *tArgLists;    // template argument declarations
   QList<BaseInfo>        extends;      // list of base classes
   QList<Grouping>       *groups;       // list of groups this entry belongs to
   QList<SectionInfo>    *anchors;      // list of anchors defined in this entry
   QList<ListItemInfo>   *sli;          // special lists (test/todo/bug/deprecated/..)

   QString   type;              // member type
   QString   name;              // member name
   QString   args;              // member argument string
   QString   bitfields;         // member's bit fields
   QString   m_program;         // the program text
   QString   initializer;       // initial value (for variables)
   QString   includeFile;       // include file (2 arg of \\class, must be unique)
   QString   includeName;       // include name (3 arg of \\class)
   QString   doc;               // documentation block (partly parsed)
   QString   docFile;           // file in which the documentation was found
   QString   brief;             // brief description (doc block)
   QString   briefFile;         // file in which the brief desc. was found
   QString   inbodyDocs;        // documentation inside the body of a function
   QString   inbodyFile;        // file in which the body doc was found
   QString   relates;           // related class (doc block)

   QString   m_read;            // property read
   QString   m_write;           // property write

   // copperspice - additional properties
   QString   m_reset;
   QString   m_notify;
   QString   m_revision;
   QString   m_designable;
   QString   m_scriptable;
   QString   m_stored;
   QString   m_user;

   QString   inside;            // name of the class in which documents are found
   QString   exception;         // throw specification
   QString	 fileName;          // file this entry was extracted from
   QString   id;                // libclang id

   bool      hidden;            // does this represent an entity that is hidden from the output
   bool      artificial;        // Artificially introduced item
   
   // return the command name used to define GROUPDOC_SEC
   QString groupDocCmd() const {

      switch ( groupDocType ) {
         case GROUPDOC_NORMAL:
            return "\\defgroup";

         case GROUPDOC_ADD:
            return "\\addgroup";

         case GROUPDOC_WEAK:
            return "\\weakgroup";

         default:
            return "unknown group command";
      }
   }

   Grouping::GroupPri_t groupingPri() const {
      if ( section != GROUPDOC_SEC ) {
         return Grouping::GROUPING_LOWEST;
      }

      switch ( groupDocType ) {
         case GROUPDOC_NORMAL:
            return Grouping::GROUPING_AUTO_DEF;

         case GROUPDOC_ADD:
            return Grouping::GROUPING_AUTO_ADD;

         case GROUPDOC_WEAK:
            return Grouping::GROUPING_AUTO_WEAK;

         default:
            return Grouping::GROUPING_LOWEST;
      }
   }

 private:
   void createSubtreeIndex(QSharedPointer<EntryNav> nav, FileStorage *storage, QSharedPointer<FileDef> fd, QSharedPointer<Entry> self);

   QWeakPointer<Entry> m_parent;               // parent node in the tree
   QList<QSharedPointer<Entry>>  m_sublist;    // entries that are children

   Entry &operator=(const Entry &);
};

/** Wrapper for a node in the Entry tree.
 *
 *  Allows navigating through the Entry tree to load and storing Entry objects persistently to disk
 */
class EntryNav
{
 public:
   EntryNav(QSharedPointer<EntryNav> parent, QSharedPointer<Entry> e);
   ~EntryNav();

   void addChild(QSharedPointer<EntryNav> e);
   bool loadEntry(FileStorage *storage);
   bool saveEntry(QSharedPointer<Entry> e, FileStorage *storage);
   void setEntry(QSharedPointer<Entry> e);
   void releaseEntry();

   void changeSection(int section) {
      m_section = section;
   }

   void setFileDef(QSharedPointer<FileDef> fd) {
      m_fileDef = fd;
   }

   QSharedPointer<Entry> entry() const {
      return m_info;
   }

   int section() const {
      return m_section;
   }

   SrcLangExt lang() const {
      return m_lang;
   }

   const QString &type() const {
      return m_type;
   }

   const QString &name() const {
      return m_name;
   }

   TagInfo *tagInfo() const {
      return m_tagInfo;
   }

   const QList<QSharedPointer<EntryNav>> &children() const {
      return m_subList;
   }

   QSharedPointer<EntryNav> parent() const {
      return m_parent.toStrongRef();
   }

   QSharedPointer<FileDef> fileDef() const {
      return m_fileDef;
   }

 private:

   // navigation
   QWeakPointer<EntryNav> m_parent;              //!< parent node in the tree

   QList<QSharedPointer<EntryNav>>  m_subList;   //!< entries that are children of this one

   // identification
   int          m_section;     //!< entry type (see Sections);
   QString	    m_type;        //!< member type
   QString      m_name;        //!< member name

   TagInfo     *m_tagInfo;     //!< tag file info
   SrcLangExt   m_lang;        //!< programming language in which this entry was found

   QSharedPointer<FileDef> m_fileDef;
   QSharedPointer<Entry> m_info;

   int64_t      m_offset;
   bool         m_noLoad;
};

#endif
