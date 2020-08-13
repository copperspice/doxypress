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

#ifndef ENTRY_H
#define ENTRY_H

#include <bitset>

#include <QFlatMap>
#include <QString>
#include <QVector>

#include <types.h>
#include <arguments.h>
#include <doxy_shared.h>
#include <section.h>

class FileDef;
struct ListItemInfo;

// stores information about an inheritance relationship
struct BaseInfo {
   BaseInfo(const QString &n, Protection p, Specifier v)
      : name(n), prot(p), virt(v)
   { }

   QString    name;      // name of the base class
   Protection prot;      // inheritance type
   Specifier  virt;      // virtualness
};

// store information about the tag file for an entry
struct TagInfo {

   bool isEmpty() const {
      if (tag_Name.isEmpty() && tag_FileName.isEmpty() && tag_Anchor.isEmpty()) {
         return true;
      } else {
         return false;
      }
   }

   void clear() {
      tag_Name     = "";
      tag_FileName = "";
      tag_Anchor   = "";
   }

   QString tag_Name;
   QString tag_FileName;
   QString tag_Anchor;
};

// used in m_entryMap
enum class EntryKey {
   File_Name,                      // file name this entry was extracted from

   Brief_Docs,                     // brief description (doc block)
   Brief_File,                     // file in which the brief desc. was found
   Main_Docs,                      // documentation block (partly parsed)                doc
   MainDocs_File,                  // file in which the documentation was found          docFile
   Inbody_Docs,                    // documentation inside the body of a function        inbodyDocs
   Inbody_File,                    // file in which the body doc was found               inbodyFile
   Class_Name,                     // name of the class in which documents are found     inside

   Include_File,                   // include file (2nd arg of \\class, must be unique)
   Include_Name,                   // include name (3rd arg of \\class)
   Source_Text,                    // the program text                                    m_program
   Clang_Id,                       // libclang id

   Member_Type,                    // member type                                         type
   Member_Args,                    // member argument string                              args
   Member_Bitfields,               // member's bit fields                                 bitfields

   Exception_Spec,                 // throw specification                                 exception
   Related_Class,                  // related class (doc block)                           relates
   Initial_Value,                  // initial value (for variables)                       Initializer

   // copperspice properties
   Read_Property,
   Write_Property,
   Reset_Property,
   Notify_Property,
   Revision_Property,
   Designable_Property,
   Scriptable_Property,
   Stored_Property,
   User_Property
};

// store information about an entity found in the source code
class Entry : public EnableSharedFromThis
{
  public:

   // supported entry types
   enum Sections {
      CLASS_SEC        = 0x00000001,
      NAMESPACE_SEC    = 0x00000010,
      COMPOUND_MASK    = CLASS_SEC,
      SCOPE_MASK       = COMPOUND_MASK | NAMESPACE_SEC,

      CLASSDOC_SEC     = 0x00000020,
      STRUCTDOC_SEC    = 0x00000040,
      UNIONDOC_SEC     = 0x00000080,
      EXCEPTIONDOC_SEC = 0x00000100,
      INTERFACEDOC_SEC = 0x00000200,
      PROTOCOLDOC_SEC  = 0x00000400,
      CATEGORYDOC_SEC  = 0x00000800,
      SERVICEDOC_SEC   = 0x00001000,
      SINGLETONDOC_SEC = 0x00002000,

      COMPOUNDDOC_MASK = CLASSDOC_SEC | STRUCTDOC_SEC | UNIONDOC_SEC |
                         EXCEPTIONDOC_SEC | INTERFACEDOC_SEC | PROTOCOLDOC_SEC |
                         CATEGORYDOC_SEC | SERVICEDOC_SEC | SINGLETONDOC_SEC,

      SOURCE_SEC       = 0x00004000,
      HEADER_SEC       = 0x00008000,
      FILE_MASK        = SOURCE_SEC | HEADER_SEC,

      CONCEPTDOC_SEC   = 0x00100000,
      DEFINEDOC_SEC    = 0x00200000,
      DIRDOC_SEC       = 0x00300000,
      ENUMDOC_SEC      = 0x00400000,
      FILEDOC_SEC      = 0x00500000,
      GROUPDOC_SEC     = 0x00600000,
      MAINPAGEDOC_SEC  = 0x00700000,
      MEMBERDOC_SEC    = 0x00800000,
      NAMESPACEDOC_SEC = 0x00900000,
      OVERLOADDOC_SEC  = 0x00a00000,
      PACKAGEDOC_SEC   = 0x00b00000,
      PAGEDOC_SEC      = 0x00c00000,
      VARIABLEDOC_SEC  = 0x00d00000,

      DEFINE_SEC       = 0x00e00000,
      ENUM_SEC         = 0x00f00000,
      EMPTY_SEC        = 0x01000000,
      EXAMPLE_SEC      = 0x02000000,
      FUNCTION_SEC     = 0x03000000,
      INCLUDE_SEC      = 0x04000000,
      MEMBERGRP_SEC    = 0x05000000,
      OBJCIMPL_SEC     = 0x06000000,
      PACKAGE_SEC      = 0x07000000,
      TYPEDEF_SEC      = 0x08000000,
      USINGDECL_SEC    = 0x09000000,
      USINGDIR_SEC     = 0x0a000000,
      VARIABLE_SEC     = 0x0b000000,

      EXPORTED_INTERFACE_SEC = 0x0c000000,
      INCLUDED_SERVICE_SEC   = 0x0d000000,
      EXAMPLE_LINENO_SEC     = 0x0e000000,
   };

   enum Virtue {
      // class specifiers
      AbstractClass,
      Exception,
      Generic,
      Interface,
      Protocol,
      Ref,
      SealedClass,
      Struct,
      Template,
      Union,
      Value,

      Category,
      Enum,                    // Java-style enums
      Service,                 // UNO IDL
      Singleton,               // UNO IDL
      ForwardDecl,

      // method properties for CopperSpice
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
      Final_Property,

      Assign,

      // member specifiers
      Deprecated,
      PrivateGettable,         // C# private getter
      ProtectedGettable,       // C# protected getter
      PrivateSettable,         // C# private setter
      ProtectedSettable,       // C# protected setter

      // atrributes
      NoDiscard,
      NoReturn,

      Inline,
      Final,
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
      ConstExpr,
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

   // group types for docs
   enum GroupDocType {
      GROUPDOC_NORMAL,        // defgroup
      GROUPDOC_ADD,           // addtogroup
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

   void addSpecialListItem(const QString &listName, int index);
   void createNavigationIndex(QSharedPointer<FileDef> fd);

   // returns the list of children for this Entry
   const QVector<QSharedPointer<Entry>> &children() const {
      return m_sublist;
   }

   QSharedPointer<FileDef> fileDef() const {
      return m_fileDef;
   }

   void setFileDef(QSharedPointer<FileDef> fd) {
      m_fileDef = fd;
   }

   // returns the parent for this Entry or null_ptr if this entry has no parent
   QSharedPointer<Entry> parent() const {
      return m_parent.toStrongRef();
   }

   void setParent(QSharedPointer<Entry> parent) {
      m_parent = parent;
   }

   void setSection(int tmpSection) {
      section = tmpSection;
   }


   // ** sub entries

   // adds entry E as a child to this entry
   void addSubEntry (QSharedPointer<Entry> e, QSharedPointer<Entry> dummy = QSharedPointer<Entry>());

   // Removes entry E from the list of children
   void removeSubEntry(QSharedPointer<Entry> e);

   // restore the state of this Entry to the default value when constructed
   void reset();


   // ** methdods for EnteryKey flatMap

   QString getData(EntryKey key) const {
      return m_entryMap.value(key);
   }

   void setData(EntryKey key, QString data)  {

      if (! data.isEmpty() || m_entryMap.contains(key)) {
         m_entryMap.insert(key, data);
      }
   }

   void appendData(EntryKey key, QString data)  {
      auto iter = m_entryMap.find(key);

      if (iter == m_entryMap.end()) {
         m_entryMap.insert(key, data);

      } else {
         iter.value() += data;

      }
   }

   void prependData(EntryKey key, QString data)  {

      auto iter = m_entryMap.find(key);

      if (iter == m_entryMap.end()) {
         m_entryMap.insert(key, data);

      } else {
         iter.value().prepend(data);

      }
   }

   TagInfo       m_tagInfo;        // tag file info
   ArgumentList  argList;          // member arguments as a list
   ArgumentList  typeConstr;       // where clause (C#) for type constraints

   RelatesType   relatesType;      // how relates is handled
   Specifier     virt;             // virtualness of the entry
   Protection    protection;       // class protection
   MethodType    mtype;            // signal, slot, (dcop) method, or property
   GroupDocType  groupDocType;
   SrcLangExt    m_srcLang;        // programming language in which this entry was found
   Traits        m_traits;

   int  section;                   // entry type (see Sections);
   int  initLines;                 // define/variable initializer lines to show

   bool referencedByRelation;      //  do we need to show the referenced by relation?
   bool referencesRelation;        //  do we need to show the references relation?

   int  docLine;                   // line number at which the documentation was found
   int  briefLine;                 // line number at which the brief desc. was found
   int  inbodyLine;                // line number at which the body doc was found

   int  startBodyLine;             // line number of the definition in the source
   int  endBodyLine;               // line number where the definition ends

   int  mGrpId;                    // member group id
   int  startLine;                 // start line of entry in the source
   int  startColumn;               // start column of entry in the source

   LocalToc localToc;

   bool stat;                      // static ?
   bool explicitExternal;          // explicitly defined as external ?
   bool proto;                     // prototype ?
   bool subGrouping;               // automatically group class members ?
   bool callGraph;                 // do we need to draw the call graph ?
   bool callerGraph;               // do we need to draw the caller graph ?
   bool hidden;                    // does this represent an entity that is hidden from the output
   bool artificial;                // artificially introduced item

   QString m_entryName;            // entry name

   QVector<ArgumentList>  m_templateArgLists;    // template argument declarations
   QVector<BaseInfo>      extends;               // list of base classes
   QVector<Grouping>      m_groups;              // list of groups this entry belongs to
   QVector<SectionInfo>   m_anchors;             // list of anchors defined in this entry
   QVector<ListItemInfo>  m_specialLists;        // special lists (test/todo/bug/deprecated/..)

   // return command name used to define GROUPDOC_SEC
   QString groupDocCmd() const {

      switch ( groupDocType ) {
         case GROUPDOC_NORMAL:
            return QString("\\defgroup");

         case GROUPDOC_ADD:
            return QString("\\addtogroup");

         case GROUPDOC_WEAK:
            return QString("\\weakgroup");

         default:
            return QString("unknown group command");
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

   // from old miniEntry
   QSharedPointer<Entry> entry()  {
      QSharedPointer<Entry> self = sharedFrom(this);
      return self;
   }

 private:
   Entry &operator=(const Entry &);

   QFlatMap<EntryKey, QString> m_entryMap;        // contains details about this entry

   QWeakPointer<Entry> m_parent;                  // parent node in the tree
   QVector<QSharedPointer<Entry>> m_sublist;      // entries which are children

   QSharedPointer<FileDef> m_fileDef;             // file associated with the entry
};

#endif