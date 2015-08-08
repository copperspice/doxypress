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

#ifndef ENTRY_H
#define ENTRY_H

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

/** This class stores information about an inheritance relation
 */
struct BaseInfo {
   /*! Creates an object representing an inheritance relation */

   BaseInfo(const QString &n, Protection p, Specifier v) 
      : name(n), prot(p), virt(v) {}

   QString    name;      //!< the name of the base class
   Protection prot;      //!< inheritance type
   Specifier  virt;      //!< virtualness
};

/** This struct is used to capture the tag file information
 *  for an Entry.
 */
struct TagInfo {
   QString tagName;
   QString fileName;
   QString anchor;
};

/** Represents an unstructured piece of information, about an
 *  entity found in the sources.
 *
 *  parseMain() in scanner.l will generate a tree of these entries.
 */
class Entry
{
 public:

   /*! Kind of entries that are supported */
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

   // class specifiers (add new items to the end)
   static const uint64_t Template        = (1ULL << 0);
   static const uint64_t Generic         = (1ULL << 1);
   static const uint64_t Ref             = (1ULL << 2);
   static const uint64_t Value           = (1ULL << 3);
   static const uint64_t Interface       = (1ULL << 4);
   static const uint64_t Struct          = (1ULL << 5);
   static const uint64_t Union           = (1ULL << 6);
   static const uint64_t Exception       = (1ULL << 7);
   static const uint64_t Protocol        = (1ULL << 8);
   static const uint64_t Category        = (1ULL << 9);
   static const uint64_t SealedClass     = (1ULL << 10);
  
   static const uint64_t Enum            = (1ULL << 12); // for Java-style enums
   static const uint64_t Service         = (1ULL << 13); // UNO IDL
   static const uint64_t Singleton       = (1ULL << 14); // UNO IDL
   static const uint64_t ForwardDecl     = (1ULL << 15); // forward declarad template classes

   // member specifiers (add new items to the beginning)
   static const uint64_t Deprecated          = (1ULL << 19);
   static const uint64_t PrivateGettable     = (1ULL << 20); // C# private getter
   static const uint64_t ProtectedGettable   = (1ULL << 21); // C# protected getter
   static const uint64_t PrivateSettable     = (1ULL << 22); // C# private setter
   static const uint64_t ProtectedSettable   = (1ULL << 23); // C# protected setter
   static const uint64_t Inline              = (1ULL << 24);
   static const uint64_t Explicit            = (1ULL << 25);
   static const uint64_t Mutable             = (1ULL << 26);
   static const uint64_t Settable            = (1ULL << 27);
   static const uint64_t Gettable            = (1ULL << 28);
  
   static const uint64_t Abstract        = (1ULL << 32);
   static const uint64_t Addable         = (1ULL << 33);
   static const uint64_t Removable       = (1ULL << 34);
   static const uint64_t Raisable        = (1ULL << 35);
   static const uint64_t Override        = (1ULL << 36);
   static const uint64_t New             = (1ULL << 37);
   static const uint64_t Sealed          = (1ULL << 38);
   static const uint64_t Initonly        = (1ULL << 39);
   static const uint64_t Optional        = (1ULL << 40);
   static const uint64_t Required        = (1ULL << 41);
   static const uint64_t NonAtomic       = (1ULL << 42);
   static const uint64_t Copy            = (1ULL << 43);
   static const uint64_t Retain          = (1ULL << 44);
   
   static const uint64_t Strong          = (1ULL << 46);
   static const uint64_t Weak            = (1ULL << 47);
   static const uint64_t Unretained      = (1ULL << 48);
   static const uint64_t Alias           = (1ULL << 49);
   static const uint64_t ConstExp        = (1ULL << 50);
   static const uint64_t Default         = (1ULL << 51);
   static const uint64_t Delete          = (1ULL << 52);
   static const uint64_t NoExcept        = (1ULL << 53);
   static const uint64_t Attribute       = (1ULL << 54); // UNO IDL attribute
   static const uint64_t Property        = (1ULL << 55); // UNO IDL property
   static const uint64_t Readonly        = (1ULL << 56); // on UNO IDL attribute or property
   static const uint64_t Bound           = (1ULL << 57); // on UNO IDL attribute or property
   static const uint64_t Constrained     = (1ULL << 58); // on UNO IDL property
   static const uint64_t Transient       = (1ULL << 59); // on UNO IDL property
   static const uint64_t MaybeVoid       = (1ULL << 60); // on UNO IDL property
   static const uint64_t MaybeDefault    = (1ULL << 61); // on UNO IDL property
   static const uint64_t MaybeAmbiguous  = (1ULL << 62); // on UNO IDL property
   static const uint64_t Published       = (1ULL << 63); // UNO IDL keyword

   struct SpecifierFlags {

      SpecifierFlags() {
         // ensures this struc has no virtual methods
         static_assert(std::is_standard_layout<SpecifierFlags>::value == true, "Struct SpecifierFlags can not have virutal methods");

         // set ever bit field to zero or false
         memset(this, 0, sizeof(SpecifierFlags));
      } 

      SpecifierFlags & operator|=(const SpecifierFlags &other) {
         // old ones
         this->spec |= other.spec;

         // new ones
         this->m_isAbstractClass |= other.m_isAbstractClass;

         this->m_isReadable   |= other.m_isReadable;
         this->m_isWritable   |= other.m_isWritable;
         this->m_isReset      |= other.m_isReset;       
         this->m_isNotify     |= other.m_isNotify;
         this->m_isRevision   |= other.m_isRevision;
         this->m_isDesignable |= other.m_isDesignable;
         this->m_isScriptable |= other.m_isScriptable;
         this->m_isStored     |= other.m_isStored;
         this->m_isUser       |= other.m_isUser;
         this->m_isConstant   |= other.m_isConstant;
         this->m_isFinal      |= other.m_isFinal;

         this->m_isAssign     |= other.m_isAssign;

         return *this;
      }

      QByteArray toQByteArray() const {

         QByteArray retval; 
         retval.resize(sizeof(SpecifierFlags));

         memcpy(retval.data(), this, sizeof(SpecifierFlags));
       
         return retval;
      }

      static SpecifierFlags fromQByteArray(const QByteArray &data)  {

         SpecifierFlags retval;
         memcpy(&retval, data.constData(), sizeof(SpecifierFlags));

         return retval;
      }

      // old flags, move to new data type
      uint64_t spec;            // !< class/member specifiers

      // class specifiers
      int m_isAbstractClass    : 1;

      // member specifiers 
      int m_isReadable         : 1;    
      int m_isWritable         : 1;
      int m_isReset            : 1;
      int m_isNotify           : 1;
      int m_isRevision         : 1;
      int m_isDesignable       : 1;
      int m_isScriptable       : 1;
      int m_isStored           : 1;
      int m_isUser             : 1;
      int m_isConstant         : 1;
      int m_isFinal            : 1;

      int m_isAssign           : 1;
   };

   enum GroupDocType {
      GROUPDOC_NORMAL,        //!< defgroup
      GROUPDOC_ADD,           //!< addgroup
      GROUPDOC_WEAK           //!< weakgroup
   };                         //!< kind of group

   Entry();
   Entry(const Entry &);
   ~Entry();

   /*! Returns the static size of the Entry (so excluding any dynamic memory) */
   int getSize();

   void addSpecialListItem(const QString &listName, int index);
   void createNavigationIndex(QSharedPointer<EntryNav> rootNav, FileStorage *storage, QSharedPointer<FileDef> fd, 
                              QSharedPointer<Entry> self);

   // while parsing a file these function can be used to navigate/build the tree
   void setParent(QSharedPointer<Entry> parent) {
      m_parent = parent;
   }

   /*! Returns the parent for this Entry or 0 if this entry has no parent. */
   QSharedPointer<Entry> parent() const {
      return m_parent.toStrongRef();
   }

   /*! Returns the list of children for this Entry
    *  @see addSubEntry() and removeSubEntry()
    */
   const QList<QSharedPointer<Entry>> children() const {
      return m_sublist;
   }

   /*! Adds entry \a e as a child to this entry */
   void addSubEntry (QSharedPointer<Entry> e, QSharedPointer<Entry> self) ;

   /*! Removes entry \a e from the list of children.    *  
    */
   void removeSubEntry(QSharedPointer<Entry> e);

   /*! Restore the state of this Entry to the default value it has
    *  at construction time.
    */
   void reset();
 
 public:
   // identification
    
   TagInfo      *tagInfo;    //!< tag file info
   ArgumentList  argList;    //!< member arguments as a list 
   ArgumentList  typeConstr; //!< where clause (C#) for type constraints
   
   RelatesType  relatesType; //!< how relates is handled
   Specifier    virt;        //!< virtualness of the entry
   Protection   protection;  //!< class protection
   MethodTypes  mtype;       //!< signal, slot, (dcop) method, or property
   GroupDocType groupDocType;
   SrcLangExt   lang;        //!< programming language in which this entry was found 
   
   SpecifierFlags m_specFlags; 

   int  section;             //!< entry type (see Sections);
   int  initLines;           //!< define/variable initializer lines to show
   int  docLine;             //!< line number at which the documentation was found
   int  briefLine;           //!< line number at which the brief desc. was found
   int  inbodyLine;          //!< line number at which the body doc was found
   int  bodyLine;            //!< line number of the definition in the source
   int  endBodyLine;         //!< line number where the definition ends
   int  mGrpId;              //!< member group id
   int  startLine;           //!< start line of entry in the source
   int  startColumn;         //!< start column of entry in the source

   bool stat;                //!< static ?
   bool explicitExternal;    //!< explicitly defined as external?
   bool proto;               //!< prototype ?
   bool subGrouping;         //!< automatically group class members?
   bool callGraph;           //!< do we need to draw the call graph?
   bool callerGraph;         //!< do we need to draw the caller graph?
  
   QList<ArgumentList> *tArgLists;    //!< template argument declarations
   QList<BaseInfo>      extends;      //!< list of base classes
   QList<Grouping>     *groups;       //!< list of groups this entry belongs to
   QList<SectionInfo>  *anchors;      //!< list of anchors defined in this entry
   QList<ListItemInfo> *sli;          //!< special lists (test/todo/bug/deprecated/..) this entry is in

   QString	 type;        //!< member type
   QString	 name;        //!< member name
   QString   args;        //!< member argument string
   QString   bitfields;   //!< member's bit fields  
   QString	 program;     //!< the program text
   QString   initializer; //!< initial value (for variables)
   QString   includeFile; //!< include file (2 arg of \\class, must be unique)
   QString   includeName; //!< include name (3 arg of \\class)
   QString   doc;         //!< documentation block (partly parsed)
   QString   docFile;     //!< file in which the documentation was found
   QString   brief;       //!< brief description (doc block)  
   QString   briefFile;   //!< file in which the brief desc. was found
   QString   inbodyDocs;  //!< documentation inside the body of a function 
   QString   inbodyFile;  //!< file in which the body doc was found
   QString   relates;     //!< related class (doc block)

   QString   m_read;        //!< property read 
   QString   m_write;       //!< property write

   // copperspice - additional properties
   QString   m_reset;
   QString   m_notify;
   QString   m_revision;
   QString   m_designable;
   QString   m_scriptable;
   QString   m_stored;
   QString   m_user;   

   QString   inside;       //!< name of the class in which documents are found
   QString   exception;    //!< throw specification
   QString	 fileName;     //!< file this entry was extracted from
   QString   id;           //!< libclang id   
      
   bool      hidden;       //!< does this represent an entity that is hidden from the output
   bool      artificial;   //!< Artificially introduced item

   static int  num;        //!< counts the total number of entries

   /// return the command name used to define GROUPDOC_SEC
   const char *groupDocCmd() const {
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

   QWeakPointer<Entry> m_parent;               //!< parent node in the tree
   QList<QSharedPointer<Entry>>  m_sublist;    //!< entries that are children 

   Entry &operator=(const Entry &);
};

/** Wrapper for a node in the Entry tree.
 *
 *  Allows navigating through the Entry tree and load and storing Entry
 *  objects persistently to disk.
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
