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

#include <QCryptographicHash>

#include <classdef.h>
#include <doxygen.h>
#include <docparser.h>
#include <entry.h>
#include <filedef.h>
#include <groupdef.h>
#include <language.h>
#include <marshal.h>
#include <membergroup.h>
#include <memberlist.h>
#include <namespacedef.h>
#include <outputlist.h>
#include <util.h>

MemberGroup::MemberGroup()
{
}

MemberGroup::MemberGroup(Definition *parent, int id, const char *hdr, const char *d, const char *docFile)
{
   //printf("New member group id=%d header=%s desc=%s\n",id,hdr,d);
   memberList      = new MemberList(MemberListType_memberGroup);
   grpId           = id;
   grpHeader       = hdr;
   doc             = d;
   scope           = 0;
   inSameSection   = true;
   inDeclSection   = 0;
   m_numDecMembers = -1;
   m_numDocMembers = -1;
   m_parent        = parent;
   m_docFile       = docFile;
   m_xrefListItems = 0;
   //printf("Member group docs=`%s'\n",doc.data());
}

MemberGroup::~MemberGroup()
{
   delete memberList;
}

void MemberGroup::insertMember(MemberDef *md)
{  
   MemberDef *firstMd = memberList->first();

   if (inSameSection && memberList->count() > 0 && firstMd->getSectionList(m_parent) != md->getSectionList(m_parent)) {
      inSameSection = false;

   } else if (inDeclSection == 0) {
      inDeclSection = md->getSectionList(m_parent);
      //printf("inDeclSection=%p type=%d\n",inDeclSection,inDeclSection->listType());
   }

   memberList->append(md);

   // copy the group of the first member in the memberGroup
   GroupDef *gd;

   if (firstMd && (gd = firstMd->getGroupDef())) {
      md->setGroupDef(gd, firstMd->getGroupPri(), firstMd->getGroupFileName(), 
                      firstMd->getGroupStartLine(), firstMd->getGroupHasDocs());

      gd->insertMember(md);
   }
}

void MemberGroup::setAnchors()
{
   ::setAnchors(memberList);
}

void MemberGroup::writeDeclarations(OutputList &ol, ClassDef *cd, NamespaceDef *nd, FileDef *fd, GroupDef *gd, bool showInline)
{  
   QByteArray ldoc = doc;

   if (!ldoc.isEmpty()) {
      ldoc.prepend("<a name=\"" + anchor() + "\" id=\"" + anchor() + "\"></a>");
   }

   memberList->writeDeclarations(ol, cd, nd, fd, gd, grpHeader, ldoc, false, showInline);
}

void MemberGroup::writePlainDeclarations(OutputList &ol, ClassDef *cd, NamespaceDef *nd, FileDef *fd, 
                                         GroupDef *gd, ClassDef *inheritedFrom, const char *inheritId )
{
   //printf("MemberGroup::writePlainDeclarations() memberList->count()=%d\n",memberList->count());
   memberList->writePlainDeclarations(ol, cd, nd, fd, gd, inheritedFrom, inheritId);
}

void MemberGroup::writeDocumentation(OutputList &ol, const char *scopeName,
                                     Definition *container, bool showEnumValues, bool showInline)
{
   memberList->writeDocumentation(ol, scopeName, container, 0, showEnumValues, showInline);
}

void MemberGroup::writeDocumentationPage(OutputList &ol, const char *scopeName, Definition *container)
{
   memberList->writeDocumentationPage(ol, scopeName, container);
}

void MemberGroup::addGroupedInheritedMembers(OutputList &ol, ClassDef *cd, MemberListType lt,    
                                             ClassDef *inheritedFrom, const QByteArray &inheritId)
{  
   for (auto md : *memberList) {   
      //printf("matching %d == %d\n",lt,md->getSectionList(m_parent)->listType());

      if (lt == md->getSectionList(m_parent)->listType()) {
         MemberList ml(lt);
         ml.append(md);
         ml.writePlainDeclarations(ol, cd, 0, 0, 0, inheritedFrom, inheritId);
      }
   }
}

int MemberGroup::countGroupedInheritedMembers(MemberListType lt)
{
   //printf("** countGroupedInheritedMembers()\n");
   int count = 0;  

   for (auto md : *memberList) {  
      //printf("matching %d == %d\n",lt,md->getSectionList(m_parent)->listType());

      if (lt == md->getSectionList(m_parent)->listType()) {
         count++;
      }
   }
   return count;
}


/*! Add this group as a subsection of the declaration section, instead
 *  of rendering it in its own section
 */
void MemberGroup::addToDeclarationSection()
{
   if (inDeclSection) {    
      inDeclSection->addMemberGroup(this);
   }
}

int MemberGroup::countDecMembers(GroupDef *gd)
{
   if (m_numDecMembers == -1) { 
      /* number of member not cached */
      memberList->countDecMembers(gd);
      m_numDecMembers = memberList->numDecMembers();
   }
   return m_numDecMembers;
}

int MemberGroup::countDocMembers()
{
   if (m_numDocMembers == -1) {
      memberList->countDocMembers();
      m_numDocMembers = memberList->numDocMembers();
   }
   return m_numDocMembers;
}

int MemberGroup::countInheritableMembers(ClassDef *inheritedFrom) const
{
   return memberList->countInheritableMembers(inheritedFrom);
}

void MemberGroup::distributeMemberGroupDocumentation()
{
   for (auto item : *memberList) {        
      // find the first member of the group with documentation

      if (! item->documentation().isEmpty() || ! item->briefDescription().isEmpty() || ! item->inbodyDocumentation().isEmpty() ) {         

         // distribute docs of item to other members of the list            
         for (auto omd : *memberList) {  

            if (item != omd && omd->documentation().isEmpty() && omd->briefDescription().isEmpty() && omd->inbodyDocumentation().isEmpty() ) {
               //printf("Copying documentation to member %s\n",omd->name().data());

               omd->setBriefDescription(item->briefDescription(), item->briefFile(), item->briefLine());
               omd->setDocumentation(item->documentation(), item->docFile(), item->docLine());
               omd->setInbodyDocumentation(item->inbodyDocumentation(), item->inbodyFile(), item->inbodyLine());
            }
         }

         break;
      }
   }   
}

int MemberGroup::varCount() const
{
   return memberList->varCount();
}

int MemberGroup::funcCount() const
{
   return memberList->funcCount();
}

int MemberGroup::enumCount() const
{
   return memberList->enumCount();
}

int MemberGroup::enumValueCount() const
{
   return memberList->enumValueCount();
}

int MemberGroup::typedefCount() const
{
   return memberList->typedefCount();
}

int MemberGroup::protoCount() const
{
   return memberList->protoCount();
}

int MemberGroup::defineCount() const
{
   return memberList->defineCount();
}

int MemberGroup::friendCount() const
{
   return memberList->friendCount();
}

int MemberGroup::numDecMembers() const
{
   return memberList->numDecMembers();
}

int MemberGroup::numDocMembers() const
{
   return memberList->numDocMembers();
}

void MemberGroup::setInGroup(bool b)
{
   memberList->setInGroup(b);
}

QByteArray MemberGroup::anchor() const
{     
   QByteArray locHeader = grpHeader;

   if (locHeader.isEmpty()) {
      locHeader = "[NOHEADER]";
   }

   QByteArray sigStr;
   sigStr = QCryptographicHash::hash(locHeader, QCryptographicHash::Md5).toHex();

   return "amgrp" + sigStr;
}

void MemberGroup::addListReferences(Definition *def)
{
   memberList->addListReferences(def);
   if (m_xrefListItems && def) {
      QByteArray name = def->getOutputFileBase() + "#" + anchor();
      addRefItem(m_xrefListItems, name, theTranslator->trGroup(true, true), name, grpHeader, 0, def);
   }
}

void MemberGroup::findSectionsInDocumentation()
{
   docFindSections(doc, 0, this, m_docFile);
   memberList->findSectionsInDocumentation();
}

void MemberGroup::marshal(StorageIntf *s)
{
   marshalMemberList(s, memberList);
   marshalObjPointer(s, inDeclSection); // reference only
   marshalInt(s, grpId);
   marshalQByteArray(s, grpHeader);
   marshalQByteArray(s, fileName);
   marshalObjPointer(s, scope);
   marshalQByteArray(s, doc);
   marshalBool(s, inSameSection);
   marshalInt(s, m_numDecMembers);
   marshalInt(s, m_numDocMembers);
   marshalObjPointer(s, m_parent);
   marshalQByteArray(s, m_docFile);
   marshalItemInfoList (Doxygen::symbolStorage, m_xrefListItems);
}

void MemberGroup::unmarshal(StorageIntf *s)
{
   memberList      = unmarshalMemberList(s);
   inDeclSection   = (MemberList *)unmarshalObjPointer(s);
   grpId           = unmarshalInt(s);
   grpHeader       = unmarshalQByteArray(s);
   fileName        = unmarshalQByteArray(s);
   scope           = (Definition *)unmarshalObjPointer(s);
   doc             = unmarshalQByteArray(s);
   inSameSection   = unmarshalBool(s);
   m_numDecMembers = unmarshalInt(s);
   m_numDocMembers = unmarshalInt(s);
   m_parent        = (Definition *)unmarshalObjPointer(s);
   m_docFile       = unmarshalQByteArray(s);
   m_xrefListItems = unmarshalItemInfoList (Doxygen::symbolStorage);
}

void MemberGroup::setRefItems(const QList<ListItemInfo> *sli)
{
   if (sli) {
      // deep copy the list
      if (m_xrefListItems == 0) {
         m_xrefListItems = new QList<ListItemInfo>;        
      }    

      for (auto item : *sli ) {
         m_xrefListItems->append(ListItemInfo(item));
      }
   }
}

void MemberGroup::writeTagFile(FTextStream &tagFile)
{
   memberList->writeTagFile(tagFile);
}

//--------------------------------------------------------------------------

void MemberGroupInfo::setRefItems(const QList<ListItemInfo> *sli)
{
   if (! sli) {
      return;
   }

   if (m_sli == 0) {
      m_sli = new QList<ListItemInfo>;      
   }

   for (auto item : *sli ) {
      m_sli->append(ListItemInfo(item));
   }
}
