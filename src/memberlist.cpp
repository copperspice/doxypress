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

#include <QRegExp>

#include <classdef.h>
#include <config.h>
#include <docparser.h>
#include <filedef.h>
#include <groupdef.h>
#include <language.h>
#include <memberlist.h>
#include <membergroup.h>
#include <message.h>
#include <namespacedef.h>
#include <outputlist.h>
#include <util.h>

MemberList::MemberList() 
   : m_listType(MemberListType_pubMethods)
{
   memberGroupList = 0;
   m_varCnt = 0;
   m_funcCnt = 0;
   m_enumCnt = 0;
   m_enumValCnt = 0;
   m_typeCnt = 0;
   m_protoCnt = 0;
   m_defCnt = 0;
   m_friendCnt = 0;
   m_numDecMembers = -1; // special value indicating that value needs to be computed
   m_numDocMembers = -1; // special value indicating that value needs to be computed

   m_inGroup   = false;
   m_inFile    = false;  
}

MemberList::MemberList(MemberListType lt) 
   : m_listType(lt)
{
   memberGroupList = 0;
   m_varCnt = 0;
   m_funcCnt = 0;
   m_enumCnt = 0;
   m_enumValCnt = 0;
   m_typeCnt = 0;
   m_protoCnt = 0;
   m_defCnt = 0;
   m_friendCnt = 0;
   m_numDecMembers = -1; // special value indicating that value needs to be computed
   m_numDocMembers = -1; // special value indicating that value needs to be computed

   m_inGroup  = false;
   m_inFile   = false; 
}

MemberList::~MemberList()
{
   delete memberGroupList;
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

         if (md->memberType() != MemberType_Friend && md->memberType() != MemberType_EnumValue) {
            
            if (md->memberType() == MemberType_Function) {
               if (! md->isReimplementedBy(inheritedFrom)) {
                  count++;
               }
            } else {
               count++;
            }
         }
      }
   }

   if (memberGroupList) {
   
      for (auto mg : *memberGroupList) {
         count += mg.countInheritableMembers(inheritedFrom);
      }
   }
 
   return count;
}

/*! Count the number of members in this list that are visible in
 *  the declaration part of a compound's documentation page.
 */
void MemberList::countDecMembers(bool countEnumValues, QSharedPointer<GroupDef> gd)
{
   if (m_numDecMembers != -1) {
      return;
   }

   //printf("----- countDecMembers count=%d ----\n",count());
   m_varCnt  = m_funcCnt = m_enumCnt = m_enumValCnt = 0;
   m_typeCnt = m_protoCnt = m_defCnt = m_friendCnt = 0;
   m_numDecMembers = 0;

   for (auto md : *this) {
      
      if (md->isBriefSectionVisible()) {
         switch (md->memberType()) {
            case MemberType_Variable:    // fall through
            case MemberType_Event:       // fall through
            case MemberType_Property:
               m_varCnt++, m_numDecMembers++;
               break;

            // apparently necessary to get this to show up in declarations section?
            case MemberType_Interface:   // fall through
            case MemberType_Service:     // fall through
            case MemberType_Function:    // fall through
            case MemberType_Signal:      // fall through
            case MemberType_DCOP:        // fall through
            case MemberType_Slot:
               if (!md->isRelated() || md->getClassDef()) {
                  m_funcCnt++, m_numDecMembers++;
               }
               break;

            case MemberType_Enumeration:
               m_enumCnt++, m_numDecMembers++;
               break;
            case MemberType_EnumValue:
               if (countEnumValues) {
                  m_enumValCnt++, m_numDecMembers++;
               }
               break;
            case MemberType_Typedef:
               m_typeCnt++, m_numDecMembers++;
               break;

            //case MemberType_Prototype:   m_protoCnt++,m_numDecMembers++; break;

            case MemberType_Define:
               if (Config::getBool("extract-all") || ! md->argsString().isEmpty() ||
                      ! md->initializer().isEmpty() || md->hasDocumentation()) {
                  m_defCnt++, m_numDecMembers++;
               }
               break;

            case MemberType_Friend:
               m_friendCnt++, m_numDecMembers++;
               break;

            default:
               err("Unknown member type found for member `%s'\n", qPrintable(md->name()));
         }
      }
   }
   if (memberGroupList) {
     
      for (auto mg : *memberGroupList) {
         mg.countDecMembers(gd);
         m_varCnt += mg.varCount();
         m_funcCnt += mg.funcCount();
         m_enumCnt += mg.enumCount();
         m_enumValCnt += mg.enumValueCount();
         m_typeCnt += mg.typedefCount();
         m_protoCnt += mg.protoCount();
         m_defCnt += mg.defineCount();
         m_friendCnt += mg.friendCount();
         m_numDecMembers += mg.numDecMembers();
      }
   }
   
}

void MemberList::countDocMembers(bool countEnumValues)
{
   if (m_numDocMembers != -1) {
      return;   // used cached value
   }

   m_numDocMembers = 0;
  
   for (auto md : *this) {

      if (md->isDetailedSectionVisible(m_inGroup, m_inFile)) {       
         // do not count enum values, since they do not produce entries of their own

         if (countEnumValues || md->memberType() != MemberType_EnumValue) {
            m_numDocMembers++;
         }
      }
   }

   if (memberGroupList) {     
      for (auto mg : *memberGroupList) {
         mg.countDocMembers();
         m_numDocMembers += mg.numDocMembers();
      }
   }   
}

bool MemberList::declVisible() const
{
   for (auto md : *this) {

      if (md->isBriefSectionVisible()) {

         switch (md->memberType()) {

            case MemberType_Define:    // fall through
            case MemberType_Typedef:   // fall through
            case MemberType_Variable:  // fall through
            case MemberType_Function:  // fall through
            case MemberType_Signal:    // fall through
            case MemberType_Slot:      // fall through
            case MemberType_DCOP:      // fall through
            case MemberType_Property:  // fall through
            case MemberType_Interface: // fall through
            case MemberType_Service:   // fall through
            case MemberType_Event:
               return true;

            case MemberType_Enumeration: {
               int enumVars = 0;

               QString name(md->name());

               int i = name.lastIndexOf("::");

               if (i != -1) {
                  name = name.right(name.length() - i - 2);   // strip scope (TODO: is this needed?)
               }

               if (name[0] == '@') { 
                  // anonymous enum => append variables

                  for (auto vmd : *this) {                  
                     QString vtype = vmd->typeString();

                     if (vtype.contains(name)) {
                        enumVars++;
                     }
                  }
               }

               // if this is an anonymous enum and there are variables of this
               // enum type (i.e. enumVars>0), then we do not show the enum here.

               if (enumVars == 0) { 
                  // show enum here
                  return true;
               }
            }

            break;
            case MemberType_Friend:
               return true;
            case MemberType_EnumValue: {
               if (m_inGroup) {
                  return true;
               }
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
   countDecMembers();

   if (numDecMembers() == 0) {
      // no members in this list
      return; 
   }

   ol.pushGeneratorState();

   bool first = true;
 
   for (auto md : *this) {

      if ((inheritedFrom == 0 || !md->isReimplementedBy(inheritedFrom)) && md->isBriefSectionVisible()) {
         
         switch (md->memberType()) {
            case MemberType_Define:    // fall through
            //case MemberType_Prototype: // fall through
            case MemberType_Typedef:   // fall through
            case MemberType_Variable:  // fall through
            case MemberType_Function:  // fall through
            case MemberType_Signal:    // fall through
            case MemberType_Slot:      // fall through
            case MemberType_DCOP:      // fall through
            case MemberType_Property:  // fall through
            case MemberType_Interface: // fall through
            case MemberType_Service:   // fall through

            case MemberType_Event: {
               if (first) {
                  ol.startMemberList(), first = false;
               }
               md->writeDeclaration(ol, cd, nd, fd, gd, m_inGroup, inheritedFrom, inheritId);
               break;
            }

            case MemberType_Enumeration: {
               int enumVars = 0;
             
               QString name(md->name());
               int i = name.lastIndexOf("::");

               if (i != -1) {
                  name = name.right(name.length() - i - 2);   // strip scope (TODO: is this needed?)
               }

               if (name[0] == '@') { // anonymous enum => append variables
                 
                  for (auto vmd : *this) {    
                     QString vtype = vmd->typeString();

                     if (vtype.contains(name)) {
                        enumVars++;
                        vmd->setAnonymousEnumType(md);
                     }
                  }
               }

               // if this is an anonymous enum and there are variables of this
               // enum type (i.e. enumVars>0), then we do not show the enum here.
               if (enumVars == 0) { // show enum here
                  //printf("Enum!!\n");

                  if (first) {
                     ol.startMemberList();
                     first = false;
                  }

                  ol.startMemberDeclaration();
                  ol.startMemberItem(md->anchor(), 0, inheritId);

                  bool detailsLinkable = md->isDetailedSectionLinkable();
                  if (!detailsLinkable) {
                     ol.startDoxyAnchor(md->getOutputFileBase(), 0, md->anchor(), md->name(), QString());
                  }

                  ol.writeString("enum ");
                  ol.insertMemberAlign();
                  md->writeEnumDeclaration(ol, cd, nd, fd, gd);

                  if (! detailsLinkable) {
                     ol.endDoxyAnchor(md->getOutputFileBase(), md->anchor());
                  }
                 
                  if (! md->briefDescription().isEmpty() && Config::getBool("brief-member-desc")) {

                     DocRoot *rootNode = validatingParseDoc(md->briefFile(), md->briefLine(),
                                            cd, md, md->briefDescription(), true, false, "", true, false);

                     if (rootNode && ! rootNode->isEmpty()) {
                        ol.startMemberDescription(md->anchor());
                        ol.writeDoc(rootNode, cd, md);

                        if (md->isDetailedSectionLinkable()) {
                           ol.disableAllBut(OutputGenerator::Html);
                           ol.docify(" ");
                           ol.startTextLink(md->getOutputFileBase(),
                                            md->anchor());
                           ol.parseText(theTranslator->trMore());
                           ol.endTextLink();
                           ol.enableAll();
                        }
                        ol.endMemberDescription();
                     }
                     delete rootNode;
                  }

                  ol.endMemberDeclaration(md->anchor(), inheritId);
                  ol.endMemberItem();
               }

               md->warnIfUndocumented();
               break;
            }

            case MemberType_Friend:
               if (inheritedFrom == 0) {
                  if (first) {
                     ol.startMemberList();
                     first = false;
                  }
                  md->writeDeclaration(ol, cd, nd, fd, gd, m_inGroup, inheritedFrom, inheritId);
                  break;
               }
            case MemberType_EnumValue: {
               if (m_inGroup) {
                  //printf("EnumValue!\n");
                  if (first) {
                     ol.startMemberList(), first = false;
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
         if (md->fromAnonymousScope() && !md->anonymousDeclShown()) {
            md->setFromAnonymousScope(false);

            //printf("anonymous compound members\n");

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

   if (!first) {
      ol.endMemberList();
   }

   ol.popGeneratorState();   
}

/** Writes the list of members to the output.
 *  @param ol Output list to write to
 *  @param cd non-null if this list is part of class documentation.
 *  @param nd non-null if this list is part of namespace documentation.
 *  @param fd non-null if this list is part of file documentation.
 *  @param gd non-null if this list is part of group documentation.
 *  @param title           Title to use for the member list.
 *  @param subtitle        Sub title to use for the member list.
 *  @param showEnumValues  Obsolete, always set to false.
 *  @param showInline      if set to true if title is rendered differently
 *  @param inheritedFrom   if not 0, the list is shown inside the given class as inherited members
 *         parameter cd points to the class containing the members.
 *  @param lt              Type of list that is inherited from
 */
void MemberList::writeDeclarations(OutputList &ol, QSharedPointer<ClassDef> cd, QSharedPointer<NamespaceDef> nd, 
                  QSharedPointer<FileDef> fd, QSharedPointer<GroupDef> gd, const QString &title, const QString &subtitle, 
                  bool showEnumValues, bool showInline, QSharedPointer<ClassDef> inheritedFrom, MemberListType lt)
{   
   QString inheritId;

   countDecMembers(false, gd); // count members shown in this section

   QSharedPointer<Definition> ctx = cd;

   if (ctx == 0 && nd) {
      ctx = nd;
   }

   if (ctx == 0 && gd) {
      ctx = gd;
   }

   if (ctx == 0 && fd) {
      ctx = fd;
   }

   int num = numDecMembers();
   if (inheritedFrom) {
     
      if ( cd && cd->countMembersIncludingGrouped(m_listType, inheritedFrom, true) > 0 ) {
         ol.pushGeneratorState();
         ol.disableAllBut(OutputGenerator::Html);

         inheritId = substitute(listTypeAsString(lt), "-", "_") + "_" + stripPath(cd->getOutputFileBase());

         if (! title.isEmpty()) {
            ol.writeInheritedSectionTitle(inheritId, cd->getReference(), cd->getOutputFileBase(), 
                                          cd->anchor(), title, qPrintable(cd->displayName()) );
         }

         ol.popGeneratorState();
      }

   } else if (num > 0) {

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
            ol.generateDoc("[generated]", -1, ctx, QSharedPointer<MemberDef>(), subtitle, false, false, 0, false, false);
            ol.endMemberSubtitle();
         }
      }
   }

   if (num > 0) {     
      writePlainDeclarations(ol, cd, nd, fd, gd, inheritedFrom, inheritId);      
     
      if (memberGroupList) {

         for (auto mg : *memberGroupList) {   
 
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
      // also add members that of this list type, that are grouped together
      // in a separate list in class 'inheritedFrom'
      cd->addGroupedInheritedMembers(ol, m_listType, inheritedFrom, inheritId);
   }
   
}

void MemberList::writeDocumentation(OutputList &ol, const QString &scopeName, QSharedPointer<Definition> container,
                                    const QString &title, bool showEnumValues, bool showInline)
{
   countDocMembers(showEnumValues); 

   if (numDocMembers() == 0) {
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
  
   for (auto md : *this) {    
      md->writeDocumentation(this, ol, scopeName, container, m_inGroup, showEnumValues, showInline);
   }

   if (memberGroupList) {     
      for (auto mg : *memberGroupList) {    
         mg.writeDocumentation(ol, scopeName, container, showEnumValues, showInline);
      }
   }

   ol.endMemberDocList();
}

// members in a table
void MemberList::writeSimpleDocumentation(OutputList &ol, QSharedPointer<Definition> container)
{
   countDocMembers(false);
   
   if (numDocMembers() == 0) {
      return;
   }

   ol.startMemberDocSimple();
  
   for (auto md : *this) {    
      md->writeMemberDocSimple(ol, container);
   }
   ol.endMemberDocSimple();
}

// separate member pages
void MemberList::writeDocumentationPage(OutputList &ol, const QString &scopeName, QSharedPointer<Definition> container)
{
   static bool generateTreeView = Config::getBool("generate-treeview");
  
   for (auto md : *this) {    
      if (md->isDetailedSectionLinkable()) {
         QString diskName = md->getOutputFileBase();
         QString title = md->qualifiedName();

         startFile(ol, diskName, md->name(), title, HLI_None, !generateTreeView, diskName);
         if (!generateTreeView) {
            container->writeNavigationPath(ol);
            ol.endQuickIndices();
         }
         ol.startContents();

         if (generateTreeView) {
            md->writeDocumentation(this, ol, scopeName, container, m_inGroup);
            ol.endContents();
            endFileWithNavPath(container, ol);

         } else {
            ol.writeString("<table cellspacing=\"0\" cellpadding=\"0\" border=\"0\">\n"
                           "  <tr>\n"
                           "   <td valign=\"top\">\n");

            container->writeQuickMemberLinks(ol, md);

            ol.writeString("   </td>\n");
            ol.writeString("   <td valign=\"top\" class=\"mempage\">\n");

            md->writeDocumentation(this, ol, scopeName, container, m_inGroup);

            ol.writeString("    </td>\n");
            ol.writeString("  </tr>\n");
            ol.writeString("</table>\n");

            endFile(ol);
         }
      }

      if (memberGroupList) {
         //printf("MemberList::writeDocumentation()  --  member groups\n");
       
         for (auto mg : *memberGroupList) {    
            mg.writeDocumentationPage(ol, scopeName, container);
         }
      }
   }
}

void MemberList::addMemberGroup(MemberGroup *mg)
{
   if (memberGroupList == 0) {
      memberGroupList = new QList<MemberGroup>;
   }

   memberGroupList->append(*mg);
}

void MemberList::addListReferences(QSharedPointer<Definition> def)
{  
   for (auto md : *this) {    
      if (md->getGroupDef() == 0 || def->definitionType() == Definition::TypeGroup) {

         md->addListReference(def);
         QSharedPointer<MemberList> enumFields = md->enumFieldList();

         if (md->memberType() == MemberType_Enumeration && enumFields) {                      
            for (auto vmd : *enumFields) {                  
               vmd->addListReference(def);
            }
         }
      }
   }

   if (memberGroupList) {      
       for (auto mg : *memberGroupList) {    
         mg.addListReferences(def);
      }
   }
}

void MemberList::findSectionsInDocumentation()
{   
   for (auto md : *this) {    
      md->findSectionsInDocumentation();
   }

   if (memberGroupList) {
      for (auto mg : *memberGroupList) {   
         mg.findSectionsInDocumentation();
      }
   }
}

QString MemberList::listTypeAsString(MemberListType type)
{
   switch (type) {
      case MemberListType_pubMethods:
         return "pub-methods";
      case MemberListType_proMethods:
         return "pro-methods";
      case MemberListType_pacMethods:
         return "pac-methods";
      case MemberListType_priMethods:
         return "pri-methods";
      case MemberListType_pubStaticMethods:
         return "pub-static-methods";
      case MemberListType_proStaticMethods:
         return "pro-static-methods";
      case MemberListType_pacStaticMethods:
         return "pac-static-methods";
      case MemberListType_priStaticMethods:
         return "pri-static-methods";

      case MemberListType_pubSignals:
         return "pub-signals";
      case MemberListType_proSignals:
         return "pro-signals";
      case MemberListType_priSignals:
         return "pri-signals";
      case MemberListType_pubSlots:
         return "pub-slots";
      case MemberListType_proSlots:
         return "pro-slots";
      case MemberListType_priSlots:
         return "pri-slots";

      case MemberListType_pubAttribs:
         return "pub-attribs";
      case MemberListType_proAttribs:
         return "pro-attribs";
      case MemberListType_pacAttribs:
         return "pac-attribs";
      case MemberListType_priAttribs:
         return "pri-attribs";
      case MemberListType_pubStaticAttribs:
         return "pub-static-attribs";
      case MemberListType_proStaticAttribs:
         return "pro-static-attribs";
      case MemberListType_pacStaticAttribs:
         return "pac-static-attribs";
      case MemberListType_priStaticAttribs:
         return "pri-static-attribs";
      case MemberListType_pubTypes:
         return "pub-types";
      case MemberListType_proTypes:
         return "pro-types";
      case MemberListType_pacTypes:
         return "pac-types";
      case MemberListType_priTypes:
         return "pri-types";

      case MemberListType_pubTypedefs:
         return "typedefs";
      case MemberListType_proTypedefs:
         return "typedefs";
      case MemberListType_priTypedefs:
         return "typedefs";
      
      case MemberListType_services:
         return "services";
      case MemberListType_interfaces:
         return "interfaces";

      case MemberListType_related:
         return "related";  
      case MemberListType_friends:
         return "friends";

      case MemberListType_dcopMethods:
         return "dcop-methods";
      case MemberListType_properties:
         return "properties";
      case MemberListType_events:
         return "events";
         
      case MemberListType_decDefineMembers:
         return "define-members";
      case MemberListType_decProtoMembers:
         return "proto-members";
      case MemberListType_decTypedefMembers:
         return "typedef-members";
      case MemberListType_decEnumMembers:
         return "enum-members";
      case MemberListType_decFuncMembers:
         return "func-members";
      case MemberListType_decVarMembers:
         return "var-members";
      case MemberListType_decEnumValMembers:
         return "enumval-members";

      case MemberListType_decPubSignalMembers:
         return "pub-signal-members";
      case MemberListType_decProSignalMembers:
         return "pro-signal-members";
      case MemberListType_decPriSignalMembers:
         return "pri-signal-members";
      case MemberListType_decPubSlotMembers:
         return "pub-slot-members";
      case MemberListType_decProSlotMembers:
         return "pro-slot-members";
      case MemberListType_decPriSlotMembers:
         return "pri-slot-members";

      case MemberListType_decEventMembers:
         return "event-members";
      case MemberListType_decFriendMembers:
         return "friend-members";
      case MemberListType_decPropMembers:
         return "prop-members";
      case MemberListType_enumFields:
         return "enum-fields";
      case MemberListType_memberGroup:
         return "member-group";
      default:
         break;
   }

   return "";
}

void MemberList::writeTagFile(QTextStream &tagFile)
{ 
   for (auto md : *this) {     
      md->writeTagFile(tagFile);    
   }

   if (memberGroupList) { 
      for (auto mg : *memberGroupList) {   
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
