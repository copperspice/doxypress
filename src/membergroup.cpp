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

#include <QCryptographicHash>

#include <membergroup.h>

#include <doxy_globals.h>
#include <docparser.h>
#include <entry.h>
#include <groupdef.h>
#include <language.h>
#include <outputlist.h>
#include <util.h>

MemberGroup::MemberGroup()
{
}

MemberGroup::MemberGroup(QSharedPointer<Definition> parent, int id, const QString &hdr, const QString &docText,
                  const QString &docFile, int docLine)
   : m_parent(parent)
{
   memberList      = QMakeShared<MemberList>(MemberListType_memberGroup);

   grpHeader       = hdr;
   doc             = docText;
   m_docFile       = docFile;

   m_docLine       = docLine;
   grpId           = id;

   inSameSection   = true;
}

MemberGroup::~MemberGroup()
{
}

void MemberGroup::insertMember(QSharedPointer<MemberDef> md)
{
   QSharedPointer<MemberDef> firstMd;

   if ( ! memberList->isEmpty() )  {
      firstMd = memberList->first();
   }

   if (inSameSection && firstMd && firstMd->getSectionList(m_parent) != md->getSectionList(m_parent)) {
      inSameSection = false;

   } else if (inDeclSection == 0) {
      inDeclSection = md->getSectionList(m_parent);

   }

   memberList->append(md);

   // copy the group of the first member in the memberGroup
   QSharedPointer<GroupDef> gd;

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

void MemberGroup::writeDeclarations(OutputList &ol, QSharedPointer<ClassDef> cd,
                  QSharedPointer<NamespaceDef> nd, QSharedPointer<FileDef> fd,
                  QSharedPointer<GroupDef> gd, bool showInline)
{
   QString ldoc = doc;

   if (! ldoc.isEmpty()) {
      ldoc.prepend("<a name=\"" + anchor() + "\" id=\"" + anchor() + "\"></a>");
   }

   memberList->writeDeclarations(ol, cd, nd, fd, gd, grpHeader, ldoc, false, showInline);
}

void MemberGroup::writePlainDeclarations(OutputList &ol, QSharedPointer<ClassDef> cd, QSharedPointer<NamespaceDef> nd,
                  QSharedPointer<FileDef> fd, QSharedPointer<GroupDef> gd,
                  QSharedPointer<ClassDef> inheritedFrom, const QString &inheritId )
{
   memberList->writePlainDeclarations(ol, cd, nd, fd, gd, inheritedFrom, inheritId);
}

void MemberGroup::writeDocumentation(OutputList &ol, const QString &scopeName, QSharedPointer<Definition> container,
                  bool showEnumValues, bool showInline)
{
   memberList->writeDocumentation(ol, scopeName, container, QString(), showEnumValues, showInline);
}

void MemberGroup::writeDocumentationPage(OutputList &ol, const QString &scopeName, QSharedPointer<Definition> container)
{
   memberList->writeDocumentationPage(ol, scopeName, container);
}

void MemberGroup::setAnonymousEnumType()
{
  memberList->setAnonymousEnumType();
}
void MemberGroup::addGroupedInheritedMembers(OutputList &ol, QSharedPointer<ClassDef> cd, MemberListType lt,
                  QSharedPointer<ClassDef> inheritedFrom, const QString &inheritId)
{
   for (auto md : *memberList) {
      QSharedPointer<MemberList> list = md->getSectionList(m_parent);

      if (list != nullptr && lt == list->listType()) {

         MemberList ml(lt);
         ml.append(md);
         ml.countDecMembers();

         ml.writePlainDeclarations(ol, cd, QSharedPointer<NamespaceDef>(), QSharedPointer<FileDef>(),
                                   QSharedPointer<GroupDef>(), inheritedFrom, inheritId);
      }
   }
}

int MemberGroup::countGroupedInheritedMembers(MemberListType lt)
{
   int count = 0;

   for (auto md : *memberList) {

      if (lt == md->getSectionList(m_parent)->listType()) {
         ++count;
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
      QSharedPointer<MemberGroup> self = sharedFrom(this);
      inDeclSection->addMemberGroup(self);
   }
}

void MemberGroup::countDecMembers()
{
   memberList->countDecMembers();
}

void MemberGroup::countDocMembers()
{
   memberList->countDocMembers();
}

int MemberGroup::countInheritableMembers(QSharedPointer<ClassDef> inheritedFrom) const
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

               omd->setBriefDescription(item->briefDescription(), item->briefFile(), item->briefLine());
               omd->setDocumentation(item->documentation(), item->docFile(), item->docLine());
               omd->setInbodyDocumentation(item->inbodyDocumentation(), item->inbodyFile(), item->inbodyLine());
            }
         }

         break;
      }
   }
}

/*

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

*/

int MemberGroup::numDecMembers() const
{
   return memberList->numDecMembers();
}

int MemberGroup::numDecEnumValues() const
{
  return memberList->numDecEnumValues();
}

int MemberGroup::numDocMembers() const
{
   return memberList->numDocMembers();
}

int MemberGroup::numDocEnumValues() const
{
  return memberList->numDocEnumValues();
}

void MemberGroup::setInGroup(bool b)
{
   memberList->setInGroup(b);
}

QString MemberGroup::anchor() const
{
   QString locHeader = grpHeader;

   if (locHeader.isEmpty()) {
      locHeader = "[NOHEADER]";
   }

   QByteArray data = QCryptographicHash::hash(locHeader.toUtf8(), QCryptographicHash::Md5).toHex();
   QString sigStr  = QString::fromLatin1(data);

   return "amgrp" + sigStr;
}

void MemberGroup::addListReferences(QSharedPointer<Definition> def)
{
   memberList->addListReferences(def);

   if (! m_xrefListItems.isEmpty() && def) {
      QString name = def->getOutputFileBase() + "#" + anchor();
      addRefItem(m_xrefListItems, name, theTranslator->trGroup(true, true), name, grpHeader, QString(), def);
   }
}

void MemberGroup::findSectionsInDocumentation()
{
   QSharedPointer<MemberGroup> self = sharedFrom(this);
   docFindSections(doc, QSharedPointer<Definition>(), self, m_docFile);

   memberList->findSectionsInDocumentation();
}

void MemberGroup::setRefItems(const QVector<ListItemInfo> &list)
{
   m_xrefListItems = list;
}

void MemberGroup::writeTagFile(QTextStream &tagFile)
{
   memberList->writeTagFile(tagFile);
}

void MemberGroupInfo::setRefItems(const QVector<ListItemInfo> &list)
{
   if (list.isEmpty()) {
      return;
   }

   m_listInfo = list;
}
