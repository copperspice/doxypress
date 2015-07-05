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

#ifndef TYPES_H
#define TYPES_H

/** @file
 *  @brief This file contains a number of basic enums and types.
 */

/** Protection level of members */
enum Protection   { Public, Protected, Private, Package } ;

/** Virtualness of a member. */
enum Specifier    { Normal, Virtual, Pure } ;

/** Kind of method */
enum MethodTypes  { Method, Signal, Slot, DCOP, Property, Event };

/** Type of member relation */
enum RelatesType  { Simple, Duplicate, MemberOf };

/** Kind of member relationship */
enum Relationship { Member, Related, Foreign };

/** Language as given by extension */
enum SrcLangExt {
   SrcLangExt_Unknown  = 0x00000,
   SrcLangExt_IDL      = 0x00008,
   SrcLangExt_Java     = 0x00010,
   SrcLangExt_CSharp   = 0x00020,
   SrcLangExt_D        = 0x00040,
   SrcLangExt_PHP      = 0x00080,
   SrcLangExt_ObjC     = 0x00100,
   SrcLangExt_Cpp      = 0x00200,
   SrcLangExt_JS       = 0x00400,
   SrcLangExt_Python   = 0x00800,
   SrcLangExt_Fortran  = 0x01000,   
   SrcLangExt_XML      = 0x04000,
   SrcLangExt_Tcl      = 0x08000,
   SrcLangExt_Markdown = 0x10000, 
   SrcLangExt_Make     = 0x20000
};

/** Grouping info */
struct Grouping {
   /** Grouping priority */
   enum GroupPri_t {
      GROUPING_LOWEST,
      GROUPING_AUTO_WEAK = GROUPING_LOWEST,     //!< membership in group was defined via \@weakgroup
      GROUPING_AUTO_ADD,     //!< membership in group was defined via \@add[to]group
      GROUPING_AUTO_DEF,     //!< membership in group was defined via \@defgroup
      GROUPING_AUTO_HIGHEST = GROUPING_AUTO_DEF,
      GROUPING_INGROUP,      //!< membership in group was defined by \@ingroup
      GROUPING_HIGHEST = GROUPING_INGROUP
   };

   static const char *getGroupPriName( GroupPri_t priority ) {
      switch ( priority ) {
         case GROUPING_AUTO_WEAK:
            return "@weakgroup";
         case GROUPING_AUTO_ADD:
            return "@addtogroup";
         case GROUPING_AUTO_DEF:
            return "@defgroup";
         case GROUPING_INGROUP:
            return "@ingroup";
      }
      return "???";
   }

   Grouping( const QString &gn, GroupPri_t p ) : groupname(gn), pri(p) {}
   Grouping( const Grouping &g ) : groupname(g.groupname), pri(g.pri) {}

   QString groupname;   //!< name of the group
   GroupPri_t pri;         //!< priority of this definition

};

struct ListItemInfo {
   QString type;
   int itemId;
};

enum MemberListType {  
   MemberListType_detailedLists      = 0x1000,
   MemberListType_declarationLists   = 0x2000,
   MemberListType_documentationLists = 0x4000,

   MemberListType_pubMethods              = 0,
   MemberListType_proMethods              = 1,
   MemberListType_pacMethods              = 2,
   MemberListType_priMethods              = 3,
   MemberListType_pubStaticMethods        = 4,
   MemberListType_proStaticMethods        = 5,
   MemberListType_pacStaticMethods        = 6,
   MemberListType_priStaticMethods        = 7,

   MemberListType_pubSignals              = 8,
   MemberListType_proSignals              = 9,                                        
   MemberListType_priSignals              = 10,
   MemberListType_pubSlots                = 11,
   MemberListType_proSlots                = 12,
   MemberListType_priSlots                = 13,
  
   MemberListType_pubAttribs              = 14,
   MemberListType_proAttribs              = 15,
   MemberListType_pacAttribs              = 16,
   MemberListType_priAttribs              = 17,
   MemberListType_pubStaticAttribs        = 18,
   MemberListType_proStaticAttribs        = 19,
   MemberListType_pacStaticAttribs        = 20,
   MemberListType_priStaticAttribs        = 21,
   MemberListType_pubTypes                = 22,
   MemberListType_proTypes                = 23,
   MemberListType_pacTypes                = 24,
   MemberListType_priTypes                = 25,

   MemberListType_pubTypedefs             = 26,
   MemberListType_proTypedefs             = 27,
   MemberListType_priTypedefs             = 28,

   MemberListType_services                = 29,
   MemberListType_interfaces              = 30,
   MemberListType_properties              = 31,
   MemberListType_events                  = 32,
   MemberListType_friends                 = 33,
   MemberListType_related                 = 34,
   
   MemberListType_dcopMethods             = 35,
    
   MemberListType_typedefMembers          = 36 + MemberListType_detailedLists,
   MemberListType_enumMembers             = 37 + MemberListType_detailedLists,
   MemberListType_enumValMembers          = 38 + MemberListType_detailedLists,
   MemberListType_functionMembers         = 39 + MemberListType_detailedLists,
   MemberListType_relatedMembers          = 40 + MemberListType_detailedLists,
   MemberListType_variableMembers         = 41 + MemberListType_detailedLists,
   MemberListType_propertyMembers         = 42 + MemberListType_detailedLists,
   MemberListType_eventMembers            = 43 + MemberListType_detailedLists,
   MemberListType_constructors            = 44 + MemberListType_detailedLists,
   MemberListType_allMembersList          = 45,

   //
   MemberListType_decDefineMembers        = 46 + MemberListType_declarationLists,
   MemberListType_decProtoMembers         = 47 + MemberListType_declarationLists,
   MemberListType_decTypedefMembers       = 48 + MemberListType_declarationLists,
   MemberListType_decEnumMembers          = 49 + MemberListType_declarationLists,
   MemberListType_decFuncMembers          = 50 + MemberListType_declarationLists,
   MemberListType_decVarMembers           = 51 + MemberListType_declarationLists,
   MemberListType_decEnumValMembers       = 52 + MemberListType_declarationLists,

   MemberListType_decPubSlotMembers       = 53 + MemberListType_declarationLists,
   MemberListType_decProSlotMembers       = 54 + MemberListType_declarationLists,
   MemberListType_decPriSlotMembers       = 55 + MemberListType_declarationLists,
   MemberListType_decPubSignalMembers     = 56 + MemberListType_declarationLists,
   MemberListType_decProSignalMembers     = 57 + MemberListType_declarationLists,   
   MemberListType_decPriSignalMembers     = 58 + MemberListType_declarationLists,

   MemberListType_decEventMembers         = 59 + MemberListType_declarationLists,
   MemberListType_decFriendMembers        = 60 + MemberListType_declarationLists,
   MemberListType_decPropMembers          = 61 + MemberListType_declarationLists,

   //
   MemberListType_docDefineMembers        = 62 + MemberListType_documentationLists,
   MemberListType_docProtoMembers         = 63 + MemberListType_documentationLists,
   MemberListType_docTypedefMembers       = 64 + MemberListType_documentationLists,
   MemberListType_docEnumMembers          = 65 + MemberListType_documentationLists,
   MemberListType_docFuncMembers          = 66 + MemberListType_documentationLists,
   MemberListType_docVarMembers           = 67 + MemberListType_documentationLists,
   MemberListType_docEnumValMembers       = 68 + MemberListType_documentationLists,

   MemberListType_docPubSlotMembers       = 69 + MemberListType_documentationLists,
   MemberListType_docProSlotMembers       = 70 + MemberListType_documentationLists,
   MemberListType_docPriSlotMembers       = 71 + MemberListType_documentationLists,
   MemberListType_docPubSignalMembers     = 72 + MemberListType_documentationLists,
   MemberListType_docProSignalMembers     = 73 + MemberListType_documentationLists,
   MemberListType_docPriSignalMembers     = 74 + MemberListType_documentationLists,

   MemberListType_docEventMembers         = 75 + MemberListType_documentationLists,
   MemberListType_docFriendMembers        = 76 + MemberListType_documentationLists,
   MemberListType_docPropMembers          = 77 + MemberListType_documentationLists,

   MemberListType_redefinedBy             = 78,
   MemberListType_enumFields              = 79,
   MemberListType_memberGroup             = 80,
  
   // this one is for the detailed section on the class page
   MemberListType_interfaceMembers        = 81 + MemberListType_detailedLists,   
   MemberListType_serviceMembers          = 82 + MemberListType_detailedLists,
};

enum MemberType {
   MemberType_Define,
   MemberType_Function,
   MemberType_Variable,
   MemberType_Typedef,
   MemberType_Enumeration,
   MemberType_EnumValue,
   MemberType_Signal,
   MemberType_Slot,
   MemberType_Friend,
   MemberType_DCOP,
   MemberType_Property,
   MemberType_Event,
   MemberType_Interface,
   MemberType_Service,
};

enum FortranFormat {
   FortranFormat_Unknown,
   FortranFormat_Free,
   FortranFormat_Fixed
};

#endif
