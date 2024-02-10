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

#include <memberlist.h>

#include <config.h>
#include <docparser.h>
#include <language.h>
#include <membergroup.h>
#include <message.h>
#include <namespacedef.h>
#include <outputlist.h>
#include <util.h>

#include <QRegularExpression>

MemberList::MemberList()
   : m_listType(MemberListType_pubMethods)
{
   m_memberGroupList = nullptr;

/*
   m_varCnt     = 0;
   m_funcCnt    = 0;
   m_enumCnt    = 0;
   m_enumValCnt = 0;
   m_typeCnt    = 0;
   m_protoCnt   = 0;
   m_defCnt     = 0;
   m_friendCnt  = 0;
*/

   m_numDecMembers    = -1; // special value indicating that value needs to be computed
   m_numDecEnumValues = 0;

   m_numDocMembers    = -1; // special value indicating that value needs to be computed
   m_numDocEnumValues = 0;
   m_inGroup = false;
   m_inFile  = false;
}

MemberList::MemberList(MemberListType lt)
   : m_listType(lt)
{
   m_memberGroupList = nullptr;

/*
   m_varCnt     = 0;
   m_funcCnt    = 0;
   m_enumCnt    = 0;
   m_enumValCnt = 0;
   m_typeCnt    = 0;
   m_protoCnt   = 0;
   m_defCnt     = 0;
   m_friendCnt  = 0;
*/

   m_numDecMembers    = -1; // special value indicating that value needs to be computed
   m_numDecEnumValues = 0;

   m_numDocMembers    = -1; // special value indicating that value needs to be computed
   m_numDocEnumValues = 0;

   m_inGroup = false;
   m_inFile  = false;
}

MemberList::~MemberList()
{
   delete m_memberGroupList;
}

void MemberList::append(QSharedPointer<MemberDef> md)
{
   SortedList<QSharedPointer<MemberDef>>::append(md);
}

// insert after position index (not a key based insert)
void MemberList::insert(uint index, QSharedPointer<MemberDef> md)
{
   SortedList<QSharedPointer<MemberDef>>::insert(index, md);
}

int MemberList::countInheritableMembers(QSharedPointer<ClassDef> inheritedFrom) const
{
   int count = 0;

   for ( auto md : *this ) {
      if (md->isBriefSectionVisible()) {

         if (! md->isFriend() && md->memberType() != MemberDefType::EnumValue) {

            if (md->memberType() == MemberDefType::Function) {
               if (! md->isReimplementedBy(inheritedFrom)) {
                  ++count;
               }
            } else {
               ++count;
            }
         }
      }
   }

   if (m_memberGroupList) {
      for (auto &mg : *m_memberGroupList) {
         count += mg.countInheritableMembers(inheritedFrom);
      }
   }

   return count;
}

/*! Count the number of members in this list which are visible in
 *  the declaration part of a compound documentation page.
 */
void MemberList::countDecMembers()
{
   static const bool extractAll = Config::getBool("extract-all");

   if (m_numDecMembers != -1) {
      return;
   }

//   m_varCnt  = m_funcCnt = m_enumCnt = m_enumValCnt = 0;
//   m_typeCnt = m_protoCnt = m_defCnt = m_friendCnt = 0;

   m_numDecMembers = 0;

   for (auto md : *this) {

      if (md->isBriefSectionVisible()) {

         if (md->isFriend()) {
            // m_friendCnt++;
            ++m_numDecMembers;
         }

         switch (md->memberType()) {
            case MemberDefType::Variable:
            case MemberDefType::Event:
            case MemberDefType::Property:
               // m_varCnt++;
               ++m_numDecMembers;
               break;

            // necessary to get this to show up in declarations section?
            case MemberDefType::Interface:
            case MemberDefType::Service:
            case MemberDefType::Function:
            case MemberDefType::Signal:
            case MemberDefType::DCOP:
            case MemberDefType::Slot:
               if (! md->isRelated() || md->getClassDef()) {
                  // m_funcCnt++;
                  ++m_numDecMembers;
               }
               break;

            case MemberDefType::Enumeration:
               // m_enumCnt++;
               ++m_numDecMembers;
               break;

            case MemberDefType::EnumValue:
               ++m_numDecEnumValues;
               ++m_numDecMembers;

               break;

            case MemberDefType::Typedef:
               // m_typeCnt++;
               ++m_numDecMembers;
               break;

            case MemberDefType::Define:
               if (extractAll || ! md->argsString().isEmpty() ||
                      ! md->getInitializer().isEmpty() || md->hasDocumentation()) {
                  // m_defCnt++;
                  ++m_numDecMembers;
               }
               break;

            default:
               err("Unknown member type found for member %s\n", csPrintable(md->name()));
         }
      }
   }

   if (m_memberGroupList) {

      for (auto &mg : *m_memberGroupList) {
/*
         m_varCnt        += mg.varCount();
         m_funcCnt       += mg.funcCount();
         m_enumCnt       += mg.enumCount();
         m_enumValCnt    += mg.enumValueCount();
         m_typeCnt       += mg.typedefCount();
         m_protoCnt      += mg.protoCount();
         m_defCnt        += mg.defineCount();
         m_friendCnt     += mg.friendCount();
*/

         mg.countDecMembers();
         m_numDecMembers    += mg.numDecMembers();
         m_numDecEnumValues += mg.numDecEnumValues();
      }
   }
}

void MemberList::countDocMembers()
{
   if (m_numDocMembers != -1) {
      // everything is already counted
      return;
   }

   m_numDocMembers = 0;

   for (auto md : *this) {

      if (md->isDetailedSectionVisible(m_inGroup, m_inFile)) {
         // do not count enum values since they do not produce entries of their own

         if (md->memberType()== MemberDefType::EnumValue) {

            ++m_numDocEnumValues;
         }
         ++m_numDocMembers;
      }
   }

   if (m_memberGroupList) {
      for (auto &mg : *m_memberGroupList) {
         mg.countDocMembers();
         m_numDocMembers    += mg.numDocMembers();
         m_numDocEnumValues += mg.numDocEnumValues();
      }
   }
}

int MemberList::countEnumValues(QSharedPointer<MemberDef> md, bool setAnonEnumType) const
{
   int numEnumVars = 0;

   QString name(md->name());
   int pos = name.lastIndexOf("::");

   if (pos != -1) {
      name = name.right(name.length() - pos - 2);   // strip scope, might not be required
   }

   if (name.startsWith('@')) {
      // anonymous enum => append variables

      for (auto value_md : *this) {

         if (value_md->typeString().contains(name)) {
            ++numEnumVars;

            if (setAnonEnumType) {
               value_md->setAnonymousEnumType(md);
            }
         }
      }
   }

   return numEnumVars;
}

void MemberList::setAnonymousEnumType()
{
   for (auto &md : *this) {

      if (md->isBriefSectionVisible()) {
         QString name(md->name());
         int pos = name.lastIndexOf("::");

         if (pos != -1) {
            name = name.right(name.length() - pos - 2);
         }

         if (md->memberType() == MemberDefType::Enumeration && name.startsWith('@')) {
            QSharedPointer<MemberList> mfl = md->enumFieldList();

            if (mfl != nullptr) {

               for (auto &vmd : *mfl) {

                  QString vtype = vmd->typeString();

                  if (vtype.contains(name)) {
                     vmd->setAnonymousEnumType(md);
                  }
               }
            }
         }
      }
   }

   if (m_memberGroupList != nullptr) {
      for (auto &mg : *m_memberGroupList) {
         mg.setAnonymousEnumType();
      }
   }
}

bool MemberList::declVisible() const
{
   for (auto md : *this) {

      if (md->isBriefSectionVisible()) {

         switch (md->memberType()) {

            case MemberDefType::Define:
            case MemberDefType::Typedef:
            case MemberDefType::Variable:
            case MemberDefType::Function:
            case MemberDefType::Signal:
            case MemberDefType::Slot:
            case MemberDefType::DCOP:
            case MemberDefType::Property:
            case MemberDefType::Interface:
            case MemberDefType::Service:
            case MemberDefType::Event:
               return true;

            case MemberDefType::Enumeration:

               if (countEnumValues(md, false) == 0) {
                  // show enum here
                  return true;
               }
               break;

            case MemberDefType::EnumValue:

               if (m_inGroup) {
                  return true;
               }
               break;
         }
      }
   }

   return false;
}

void MemberList::writePlainDeclarations(OutputList &ol, QSharedPointer<ClassDef> cd, QSharedPointer<NamespaceDef> nd,
                  QSharedPointer<FileDef> fd, QSharedPointer<GroupDef> gd, QSharedPointer<ClassDef> inheritedFrom,
                  const QString &inheritId )
{
   static const bool briefMemberDesc  = Config::getBool("brief-member-desc");

   countDecMembers();

   if (numDecMembers() <= numDecEnumValues()) {
      // no members in this list
      return;
   }

   ol.pushGeneratorState();
   bool first = true;

   for (auto md : *this) {

      if ((inheritedFrom == nullptr || ! md->isReimplementedBy(inheritedFrom)) && md->isBriefSectionVisible()) {

         switch (md->memberType()) {
            //  case MemberDefType::Prototype:

            case MemberDefType::Define:
            case MemberDefType::Typedef:
            case MemberDefType::Variable:
            case MemberDefType::Function:
            case MemberDefType::Signal:
            case MemberDefType::Slot:
            case MemberDefType::DCOP:
            case MemberDefType::Property:
            case MemberDefType::Interface:
            case MemberDefType::Service:
            case MemberDefType::Event: {

               if (first) {
                  ol.startMemberList();
                  first = false;
               }

               md->writeDeclaration(ol, cd, nd, fd, gd, m_inGroup, inheritedFrom, inheritId);
               break;
            }

            case MemberDefType::Enumeration: {

               // if this is an anonymous enum and there are variables of this enum then
               // enumVars = 0, do not show the enum here

               if (countEnumValues(md, true) == 0) {
                  // show enum here

                  if (first) {
                     ol.startMemberList();
                     first = false;
                  }

                  ol.startMemberDeclaration();
                  ol.startMemberItem(md->anchor(), 0, inheritId);

                  bool detailsLinkable = md->isDetailedSectionLinkable();
                  if (! detailsLinkable) {
                     ol.startDoxyAnchor(md->getOutputFileBase(), QString(), md->anchor(), md->name(), QString());
                  }

                  ol.writeString("enum ");

                  if (md->getLanguage() == SrcLangExt_Cpp && md->isStrong()) {
                     if (md->isEnumStruct()) {
                        ol.writeString("struct ");

                     } else {
                        ol.writeString("class ");

                     }
                  }
                  ol.insertMemberAlign();
                  md->writeEnumDeclaration(ol, cd, nd, fd, gd);

                  if (! detailsLinkable) {
                     ol.endDoxyAnchor(md->getOutputFileBase(), md->anchor());
                  }

                  ol.endMemberItem();

                  if (! md->briefDescription().isEmpty() && briefMemberDesc) {

                     DocRoot *rootNode = validatingParseDoc(md->briefFile(), md->briefLine(),
                           cd, md, md->briefDescription(), true, false, QString(), true, false);

                     if (rootNode && ! rootNode->isEmpty()) {
                        ol.startMemberDescription(md->anchor());
                        ol.writeDoc(rootNode, cd, md);

                        if (md->isDetailedSectionLinkable()) {
                           ol.disableAllBut(OutputGenerator::Html);
                           ol.docify(" ");
                           ol.startTextLink(md->getOutputFileBase(), md->anchor());
                           ol.parseText(theTranslator->trMore());
                           ol.endTextLink();
                           ol.enableAll();
                        }
                        ol.endMemberDescription();
                     }
                     delete rootNode;
                  }

                  ol.endMemberDeclaration(md->anchor(), inheritId);
               }

               md->warnIfUndocumented();
               break;
            }

            case MemberDefType::EnumValue: {
               if (m_inGroup) {
                  if (first) {
                     ol.startMemberList();
                     first = false;
                  }
                  md->writeDeclaration(ol, cd, nd, fd, gd, m_inGroup, inheritedFrom, inheritId);
               }
            }
            break;
         }
      }
   }

   // handle members that are inside anonymous compounds and for which
   // no variables of the anonymous compound type exist.

   if (cd) {

      for (auto md : *this) {
         if (md->fromAnonymousScope() && ! md->anonymousDeclShown()) {
            md->setFromAnonymousScope(false);

            if (md->isBriefSectionVisible()) {

               if (first) {
                  ol.startMemberList();
                  first = false;
               }

               md->writeDeclaration(ol, cd, nd, fd, gd, m_inGroup);
            }

            md->setFromAnonymousScope(true);
         }
      }
   }

   if (! first) {
      ol.endMemberList();
   }

   ol.popGeneratorState();
}

/** Writes the list of members to the output.
 *  @param ol Output       list to write to
 *  @param cd              non-null if this list is part of class documentation.
 *  @param nd              non-null if this list is part of namespace documentation.
 *  @param fd              non-null if this list is part of file documentation.
 *  @param gd              non-null if this list is part of group documentation.
 *  @param title           Title to use for the member list.
 *  @param subtitle        Sub title to use for the member list.
 *  @param showEnumValues  Obsolete, always set to false.
 *  @param showInline      if set to true if title is rendered differently
 *  @param inheritedFrom   if not 0, the list is shown inside the given class as inherited members
 *                         parameter cd points to the class containing the members.
 *  @param lt              Type of list that is inherited from
 */
void MemberList::writeDeclarations(OutputList &ol, QSharedPointer<ClassDef> cd, QSharedPointer<NamespaceDef> nd,
                  QSharedPointer<FileDef> fd, QSharedPointer<GroupDef> gd, const QString &title, const QString &subtitle,
                  bool showEnumValues, bool showInline, QSharedPointer<ClassDef> inheritedFrom, MemberListType lt)
{
   (void) showEnumValues;

   QString inheritId;

   // count members shown in this section
   countDecMembers();

   QSharedPointer<Definition> ctx = cd;

   if (ctx == nullptr && nd) {
      ctx = nd;
   }

   if (ctx == nullptr && gd) {
      ctx = gd;
   }

   if (ctx == nullptr && fd) {
      ctx = fd;
   }

   int num = numDecMembers();
   int numEnumValues = numDecEnumValues();

   if (inheritedFrom) {

      if (cd != nullptr && cd->countMembersIncludingGrouped(m_listType, inheritedFrom, true) > 0 ) {
         ol.pushGeneratorState();
         ol.disableAllBut(OutputGenerator::Html);

         inheritId = substitute(listTypeAsString(lt), "-", "_") + "_" + stripPath(cd->getOutputFileBase());

         if (! title.isEmpty()) {
            ol.writeInheritedSectionTitle(inheritId, cd->getReference(), cd->getOutputFileBase(),
                                          cd->anchor(), title, cd->displayName() );
         }

         ol.popGeneratorState();
      }

   } else if (num > numEnumValues) {

      if (! title.isEmpty()) {

         if (showInline) {
            ol.startInlineHeader();
         } else {
            ol.startMemberHeader(listTypeAsString(m_listType));
         }

         ol.parseText(title);

         if (showInline) {
            ol.endInlineHeader();
         } else {
            ol.endMemberHeader();
         }
      }

      if (! subtitle.isEmpty()) {
         QString st = subtitle;
         st = st.trimmed();

         if (! st.isEmpty()) {
            ol.startMemberSubtitle();
            ol.generateDoc("[generated]", -1, ctx, QSharedPointer<MemberDef>(), subtitle,
                  false, false, QString(), false, false);

            ol.endMemberSubtitle();
         }
      }
   }

   if (num > numEnumValues) {
      writePlainDeclarations(ol, cd, nd, fd, gd, inheritedFrom, inheritId);

      if (m_memberGroupList) {

         for (auto &mg : *m_memberGroupList) {
            bool hasHeader = ! mg.header().isEmpty() && mg.header() != "[NOHEADER]";

            if (inheritId.isEmpty()) {
               ol.startMemberGroupHeader(hasHeader);

               if (hasHeader) {
                  ol.parseText(mg.header());
               }

               ol.endMemberGroupHeader();

               if (! mg.documentation().isEmpty()) {
                  ol.startMemberGroupDocs();
                  ol.generateDoc(mg.docFile(), mg.docLine(), ctx, QSharedPointer<MemberDef>(), mg.documentation() + "\n", false, false);
                  ol.endMemberGroupDocs();
               }
               ol.startMemberGroup();
            }

            mg.writePlainDeclarations(ol, cd, nd, fd, gd, inheritedFrom, inheritId);

            if (inheritId.isEmpty()) {
               ol.endMemberGroup(hasHeader);
            }
         }
      }
   }

   if (inheritedFrom && cd) {
      // also add members of this list type, that are grouped together
      // in a separate list in class 'inheritedFrom'
      cd->addGroupedInheritedMembers(ol, m_listType, inheritedFrom, inheritId);
   }
}

void MemberList::writeDocumentation(OutputList &ol, const QString &scopeName, QSharedPointer<Definition> container,
                  const QString &title, bool showEnumValues, bool showInline)
{
   QSharedPointer<MemberList> self = sharedFrom(this);

   countDocMembers();

   if (numDocMembers() == 0) {
      return;
   }

   if (! showEnumValues && (numDocMembers() <= numDocEnumValues())) {
      return;
   }

   if (! title.isEmpty()) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
      ol.writeRuler();
      ol.popGeneratorState();
      ol.startGroupHeader(showInline ? 2 : 0);
      ol.parseText(title);
      ol.endGroupHeader(showInline ? 2 : 0);
   }

   ol.startMemberDocList();

   // count number of overloaded members  <total, count>
   QHash<QString, QPair<uint, uint>> cntOverloads;

   for (auto md : *this) {

      if (md->isDetailedSectionVisible(m_inGroup, container->definitionType() == Definition::TypeFile)
            && ! (md->isEnumValue() && ! showInline)) {

         auto iter = cntOverloads.find(md->name());

         if (iter != cntOverloads.end()) {
            iter.value().first += 1;

         } else {
           cntOverloads.insert(md->name(), {1, 1});
         }
      }
   }

   for (auto md : *this) {

      if (md->isDetailedSectionVisible(m_inGroup,container->definitionType()==Definition::TypeFile)
            && ! (md->isEnumValue() && ! showInline)) {

         auto iter = cntOverloads.find(md->name());
         auto [total, count] = iter.value();

         md->writeDocumentation(self, total, count, ol, scopeName, container, m_inGroup, showEnumValues, showInline);
         iter.value().second += 1;
      }
   }

   if (m_memberGroupList) {
      for (auto &mg : *m_memberGroupList) {
         mg.writeDocumentation(ol, scopeName, container, showEnumValues, showInline);
      }
   }

   ol.endMemberDocList();
}

// members in a table
void MemberList::writeSimpleDocumentation(OutputList &ol, QSharedPointer<Definition> container)
{
   countDocMembers();

   if (numDocMembers() <= numDocEnumValues()) {
      return;
   }

   QSharedPointer<ClassDef> cd;

   if (container && container->definitionType() == Definition::TypeClass) {
      cd = container.dynamicCast<ClassDef>();
   }

   ol.startMemberDocSimple(cd && cd->isJavaEnum());

   for (auto md : *this) {
      md->writeMemberDocSimple(ol, container);
   }

   ol.endMemberDocSimple(cd && cd->isJavaEnum());
}

// separate member pages
void MemberList::writeDocumentationPage(OutputList &ol, const QString &scopeName, QSharedPointer<Definition> container)
{
   static bool generateTreeView = Config::getBool("generate-treeview");

   QSharedPointer<MemberList> self = sharedFrom(this);

   // count number of overloaded members  <total, count>
   QHash<QString, QPair<uint, uint>> cntOverloads;

   for (auto md : *this) {

      if (md->isDetailedSectionLinkable()) {
         auto iter = cntOverloads.find(md->name());

         if (iter != cntOverloads.end()) {
            iter.value().first += 1;

         } else {
            cntOverloads.insert(md->name(), {1, 1});

         }
      }
   }

   for (auto md : *this) {
      if (md->isDetailedSectionLinkable()) {
         auto iter = cntOverloads.find(md->name());
         auto [total, count] = iter.value();

         QString diskName = md->getOutputFileBase();
         QString title    = md->qualifiedName();

         startFile(ol, diskName, md->name(), title, HLI_None, ! generateTreeView, diskName);
         if (! generateTreeView) {
            container->writeNavigationPath(ol);
            ol.endQuickIndices();
         }
         ol.startContents();

         if (generateTreeView) {
            md->writeDocumentation(self, total, count, ol, scopeName, container, m_inGroup);
            iter.value().second += 1;

            ol.endContents();
            endFileWithNavPath(container, ol);

         } else {
            ol.writeString("<table cellspacing=\"0\" cellpadding=\"0\" border=\"0\">\n"
                           "  <tr>\n"
                           "   <td valign=\"top\">\n");

            container->writeQuickMemberLinks(ol, md);

            ol.writeString("   </td>\n");
            ol.writeString("   <td valign=\"top\" class=\"mempage\">\n");

            md->writeDocumentation(self, total, count, ol, scopeName, container, m_inGroup);
            iter.value().second += 1;

            ol.writeString("    </td>\n");
            ol.writeString("  </tr>\n");
            ol.writeString("</table>\n");

            endFile(ol, false, false);
         }
      }

      if (m_memberGroupList) {
         for (auto &mg : *m_memberGroupList) {
            mg.writeDocumentationPage(ol, scopeName, container);
         }
      }
   }
}

void MemberList::addMemberGroup(QSharedPointer<MemberGroup> mg)
{
   if (m_memberGroupList == nullptr) {
      m_memberGroupList = new QList<MemberGroup>;
   }

   m_memberGroupList->append(*mg);
}

void MemberList::addListReferences(QSharedPointer<Definition> def)
{
   for (auto md : *this) {
      if (md->getGroupDef() == nullptr || def->definitionType() == Definition::TypeGroup) {

         md->addListReference(def);
         QSharedPointer<MemberList> enumFields = md->enumFieldList();

         if (md->memberType() == MemberDefType::Enumeration && enumFields) {
            for (auto vmd : *enumFields) {
               vmd->addListReference(def);
            }
         }
      }
   }

   if (m_memberGroupList) {
       for (auto &mg : *m_memberGroupList) {
         mg.addListReferences(def);
      }
   }
}

void MemberList::findSectionsInDocumentation()
{
   for (auto md : *this) {
      md->findSectionsInDocumentation();
   }

   if (m_memberGroupList) {
      for (auto &mg : *m_memberGroupList) {
         mg.findSectionsInDocumentation();
      }
   }
}

QString MemberList::listTypeAsString(MemberListType type)
{
   QString retval;

   switch (type) {
      case MemberListType_pubMethods:
         retval = "pub-methods";
         break;

      case MemberListType_proMethods:
         retval = "pro-methods";
         break;

      case MemberListType_pacMethods:
         retval = "pac-methods";
         break;

      case MemberListType_priMethods:
         retval = "pri-methods";
         break;

      case MemberListType_pubStaticMethods:
         retval = "pub-static-methods";
         break;

      case MemberListType_proStaticMethods:
         retval = "pro-static-methods";
         break;

      case MemberListType_pacStaticMethods:
         retval = "pac-static-methods";
         break;

      case MemberListType_priStaticMethods:
         retval = "pri-static-methods";
         break;

      // *
      case MemberListType_pubSignals:
         retval = "pub-signals";
         break;

      case MemberListType_proSignals:
         retval = "pro-signals";
         break;

      case MemberListType_priSignals:
         retval = "pri-signals";
         break;

      case MemberListType_pubSlots:
         retval = "pub-slots";
         break;

      case MemberListType_proSlots:
         retval = "pro-slots";
         break;

      case MemberListType_priSlots:
         retval = "pri-slots";
         break;


      // *
      case MemberListType_pubAttribs:
         retval = "pub-attribs";
         break;

      case MemberListType_proAttribs:
         retval = "pro-attribs";
         break;

      case MemberListType_pacAttribs:
         retval = "pac-attribs";
         break;

      case MemberListType_priAttribs:
         retval = "pri-attribs";
         break;

      case MemberListType_pubStaticAttribs:
         retval = "pub-static-attribs";
         break;

      case MemberListType_proStaticAttribs:
         retval = "pro-static-attribs";
         break;

      case MemberListType_pacStaticAttribs:
         retval = "pac-static-attribs";
         break;

      case MemberListType_priStaticAttribs:
         retval = "pri-static-attribs";
         break;

      // *
      case MemberListType_pubTypes:
         retval = "pub-types";
         break;

      case MemberListType_proTypes:
         retval = "pro-types";
         break;

      case MemberListType_pacTypes:
         retval = "pac-types";
         break;

      case MemberListType_priTypes:
         retval = "pri-types";
         break;

      // *
      case MemberListType_pubTypedefs:
         retval = "typedefs";
         break;

      case MemberListType_proTypedefs:
         retval = "typedefs";
         break;

      case MemberListType_pacTypedefs:
         retval = "typedefs";
         break;

      case MemberListType_priTypedefs:
         retval = "typedefs";
         break;

      // *
      case MemberListType_services:
         retval = "services";
         break;

      case MemberListType_interfaces:
         retval = "interfaces";
         break;

      case MemberListType_related:
         retval = "related";
         break;

      case MemberListType_friends:
         retval = "friends";
         break;

      case MemberListType_dcopMethods:
         retval = "dcop-methods";
         break;

      case MemberListType_properties:
         retval = "properties";
         break;

      case MemberListType_events:
         retval = "events";
         break;

      // *
      case MemberListType_decDefineMembers:
         retval = "define-members";
         break;

      case MemberListType_decProtoMembers:
         retval = "proto-members";
         break;

      case MemberListType_decTypedefMembers:
         retval = "typedef-members";
         break;

      case MemberListType_decEnumMembers:
         retval = "enum-members";
         break;

      case MemberListType_decFuncMembers:
         retval = "func-members";
         break;

      case MemberListType_decVarMembers:
         retval = "var-members";
         break;

      case MemberListType_decEnumValMembers:
         retval = "enumval-members";
         break;

      // *
      case MemberListType_decPubSignalMembers:
         retval = "pub-signal-members";
         break;

      case MemberListType_decProSignalMembers:
         retval = "pro-signal-members";
         break;

      case MemberListType_decPriSignalMembers:
         retval = "pri-signal-members";
         break;

      case MemberListType_decPubSlotMembers:
         retval = "pub-slot-members";
         break;

      case MemberListType_decProSlotMembers:
         retval = "pro-slot-members";
         break;

      case MemberListType_decPriSlotMembers:
         retval = "pri-slot-members";
         break;


      // *
      case MemberListType_decEventMembers:
         retval = "event-members";
         break;

      case MemberListType_decFriendMembers:
         retval = "friend-members";
         break;

      case MemberListType_decPropMembers:
         retval = "prop-members";
         break;

      case MemberListType_enumFields:
         retval = "enum-fields";
         break;

      case MemberListType_memberGroup:
         retval = "member-group";
         break;

      default:
         break;
   }

   return retval;
}

void MemberList::writeTagFile(QTextStream &tagFile)
{
   for (auto md : *this) {
      md->writeTagFile(tagFile);

      if (md->memberType() == MemberDefType::Enumeration && md->enumFieldList() && ! md->isStrong()) {

         for (auto vmd : *md->enumFieldList()) {
            vmd->writeTagFile(tagFile);
         }
      }
   }

   if (m_memberGroupList) {
      for (auto &mg : *m_memberGroupList) {
         mg.writeTagFile(tagFile);
      }
   }
}

int MemberSDict::compareMapValues(const QSharedPointer<MemberDef> &c1, const QSharedPointer<MemberDef> &c2) const
{
   int cmp = c1->name().compare(c2->name(), Qt::CaseInsensitive);

   if (cmp) {
      return cmp;
   } else {
      return c1->getDefLine() - c2->getDefLine();
   }
}
