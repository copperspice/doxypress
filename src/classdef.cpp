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

#include <QFile>
#include <QRegExp>

#include <stdio.h>

#include <classdef.h>
#include <classlist.h>
#include <config.h>
#include <dot.h>
#include <defargs.h>
#include <docparser.h>
#include <doxy_globals.h>
#include <diagram.h>
#include <entry.h>
#include <htmlhelp.h>
#include <language.h>
#include <layout.h>
#include <message.h>
#include <searchindex.h>
#include <util.h>

// constructs a new class definition
ClassDef::ClassDef(const QString &defFileName, int defLine, int defColumn, const QString &x_name, CompoundType ct,
                   const QString &lref, const QString &fName, bool isSymbol, bool isJavaEnum)
   : Definition(defFileName, defLine, defColumn, removeRedundantWhiteSpace(x_name), 0, 0, isSymbol)
{
   visited = false;
   setReference(lref);
   
   m_compType   = ct;
   m_isJavaEnum = isJavaEnum;

   QString tname = name();   
   const QString ctStr = compoundTypeString();

   if (! fName.isEmpty()) {
      m_fileName = stripExtension(fName);

   } else {
      m_fileName = ctStr + tname;

   }

   m_exampleSDict = 0;
   m_parents      = 0;
   m_inheritedBy  = 0;
   m_incInfo      = 0;
   m_prot         = Public;
     
   m_allMemberNameInfoSDict = 0;

   m_usesImplClassDict   = 0;
   m_usedByImplClassDict = 0;
   m_usesIntfClassDict   = 0;
   m_memberGroupSDict    = 0;
   m_innerClasses        = 0;

   m_subGrouping = Config::getBool("allow-sub-grouping");
   m_templateInstances   = 0;
   m_variableInstances   = 0;   
   m_templBaseClassNames = 0;

   m_isAbstract    = false;
   m_isStatic      = false;
   m_isTemplArg    = false;
   m_membersMerged = false;  
   m_usedOnly      = false;

   m_isSimple = Config::getBool("inline-simple-struct");
   
   m_taggedInnerClasses = 0; 
   m_spec = Entry::SpecifierFlags{};  

   // can not use getLanguage at this point, setLanguage() has not been called
   SrcLangExt lang = getLanguageFromFileName(defFileName);

   if ((lang == SrcLangExt_Cpp || lang == SrcLangExt_ObjC) && guessSection(defFileName) == Entry::SOURCE_SEC) {
      m_isLocal = true;
   } else {
      m_isLocal = false;
   }

   m_isGeneric   = (lang == SrcLangExt_CSharp || lang == SrcLangExt_Java) && tname.indexOf('<') != -1;
   m_isAnonymous = tname.indexOf('@') != -1;
}

ClassDef::~ClassDef()
{
   delete m_parents;
   delete m_inheritedBy;
   delete m_allMemberNameInfoSDict;
   delete m_exampleSDict;
   delete m_usesImplClassDict;
   delete m_usedByImplClassDict;
   delete m_usesIntfClassDict;
   delete m_incInfo;
   delete m_memberGroupSDict;
   delete m_innerClasses;
   delete m_templateInstances;
   delete m_variableInstances;
   delete m_templBaseClassNames;
   delete m_taggedInnerClasses;
}

QString ClassDef::getMemberListFileName() const
{
   return convertNameToFile(compoundTypeString() + name() + "-members");
}

QString ClassDef::displayName(bool includeScope) const
{
   // static bool optimizeOutputForJava = Config::getBool("optimize-java");
   SrcLangExt lang = getLanguage();

   QString n;

   if (includeScope) {
      n = qualifiedNameWithTemplateParameters();
   } else {
      n = className();
   }

   QString sep = getLanguageSpecificSeparator(lang);

   if (sep != "::") {
      n = substitute(n, "::", sep);
   }

   if (m_compType == ClassDef::Protocol && n.right(2) == "-p") {
      n = "<" + n.left(n.length() - 2) + ">";
   }

   if (n.indexOf('@') != -1) {
      return removeAnonymousScopes(n);
   } else {
      return n;
   }
}

// inserts a base/super class in the inheritance list
void ClassDef::insertBaseClass(QSharedPointer<ClassDef> cd, const QString &n, Protection p, Specifier s, const QString &t)
{   
   if (m_parents == 0) {
      m_parents = new SortedList<BaseClassDef *>;
   }

   m_parents->append(new BaseClassDef(cd, n, p, s, t));
   m_isSimple = false;
}

// inserts a derived/sub class in the inherited-by list
void ClassDef::insertSubClass(QSharedPointer<ClassDef> cd, Protection p, Specifier s, const QString &t)
{
   static bool extractPrivate = Config::getBool("extract-private");

   if (! extractPrivate && cd->protection() == Private) {
      return;
   }

   if (m_inheritedBy == 0) {
      m_inheritedBy = new SortedList<BaseClassDef *>;     
   }

   SortedList<BaseClassDef *> *temp = m_inheritedBy;
   temp->inSort(new BaseClassDef(cd, 0, p, s, t));

   m_isSimple = false;
}

void ClassDef::addMembersToMemberGroup()
{
   QSharedPointer<ClassDef> self = sharedFrom(this);

   for (auto item : m_memberLists ) {

      if ( (item->listType() & MemberListType_detailedLists) == 0) {
         ::addMembersToMemberGroup(item, &m_memberGroupSDict, self);
      }
   }

   // add members inside sections to their groups
   if (m_memberGroupSDict) {
  
      for (auto item : *m_memberGroupSDict ) {
         if (item->allMembersInSameSection() && m_subGrouping) {   
            item->addToDeclarationSection();
         }
      }
   }
}

// adds new member definition to the class
void ClassDef::internalInsertMember(QSharedPointer<MemberDef> md, Protection prot, bool addToAllList)
{   
   if (md->isHidden()) {
      return;
   }
    
   bool isSimple = false;

   if (md->isRelated() && protectionLevelVisible(prot)) {
      addMemberToList(MemberListType_related, md, true);

   } else if (md->isFriend()) {
      addMemberToList(MemberListType_friends, md, true);

   } else {
      switch (md->memberType()) {
         case MemberType_Service:      // UNO IDL
            addMemberToList(MemberListType_services, md, true);
            break;

         case MemberType_Interface:    // UNO IDL
            addMemberToList(MemberListType_interfaces, md, true);
            break;

         case MemberType_DCOP:         // KDE2 specific
            addMemberToList(MemberListType_dcopMethods, md, true);
            break;

         case MemberType_Property:
            addMemberToList(MemberListType_properties, md, true);
            break;

         case MemberType_Event:
            addMemberToList(MemberListType_events, md, true);
            break;

         case MemberType_Signal:       // Qt and CS specific                        
            switch (prot) {
               case Public:
                  addMemberToList(MemberListType_pubSignals, md, true);
                  break;

               case Protected:
               case Package:           // signals in packages are not possible       
                  addMemberToList(MemberListType_proSignals, md, true);
                  break;
              
               case Private:
                  addMemberToList(MemberListType_priSignals, md, true);
                  break;
            }
            break;

         case MemberType_Slot:         // Qt and CS specific
            switch (prot) {
               case Public:
                  addMemberToList(MemberListType_pubSlots, md, true);
                  break;

               case Protected:
               case Package:           // slots in packages are not possible
                  addMemberToList(MemberListType_proSlots, md, true);
                  break;
             
               case Private:
                  addMemberToList(MemberListType_priSlots, md, true);
                  break;
            }
            break;

         default: // any of the other members

            if (md->isStatic()) {
               if (md->isVariable()) {
                  switch (prot) {
                     case Protected:
                        addMemberToList(MemberListType_proStaticAttribs, md, true);
                        break;
                     case Package:
                        addMemberToList(MemberListType_pacStaticAttribs, md, true);
                        break;
                     case Public:
                        addMemberToList(MemberListType_pubStaticAttribs, md, true);
                        break;
                     case Private:
                        addMemberToList(MemberListType_priStaticAttribs, md, true);
                        break;
                  }

               } else { 
                  // function

                  switch (prot) {
                     case Protected:
                        addMemberToList(MemberListType_proStaticMethods, md, true);
                        break;
                     case Package:
                        addMemberToList(MemberListType_pacStaticMethods, md, true);
                        break;
                     case Public:
                        addMemberToList(MemberListType_pubStaticMethods, md, true);
                        break;
                     case Private:
                        addMemberToList(MemberListType_priStaticMethods, md, true);
                        break;
                  }
               }

            } else { 
               // not static

               if (md->isVariable()) {
                  switch (prot) {
                     case Protected:
                        addMemberToList(MemberListType_proAttribs, md, true);
                        break;

                     case Package:
                        addMemberToList(MemberListType_pacAttribs, md, true);
                        break;

                     case Public:
                        addMemberToList(MemberListType_pubAttribs, md, true);
                        isSimple = ! md->isFunctionPtr();
                        break;

                     case Private:
                        addMemberToList(MemberListType_priAttribs, md, true);
                        break;
                  }

               } else if (md->isTypedef()) {

                  switch (prot) {
                     case Protected:
                        addMemberToList(MemberListType_proTypedefs, md, true);
                        break;

//                     case Package:
//                        addMemberToList(MemberListType_pacTypes, md, true);
//                        break;

                     case Public:
                        addMemberToList(MemberListType_pubTypedefs, md, true);
                        isSimple = md->typeString().indexOf(")(") == -1;
                        break;

                     case Private:
                        addMemberToList(MemberListType_priTypedefs, md, true);
                        break;
                  }


               } else if (md->isEnumerate() || md->isEnumValue()) {

                  switch (prot) {
                     case Protected:
                        addMemberToList(MemberListType_proTypes, md, true);
                        break;

                     case Package:
                        addMemberToList(MemberListType_pacTypes, md, true);
                        break;

                     case Public:
                        addMemberToList(MemberListType_pubTypes, md, true);
                        isSimple = md->typeString().indexOf(")(") == -1;
                        break;

                     case Private:
                        addMemberToList(MemberListType_priTypes, md, true);
                        break;
                  }


               } else { 
                  // member function

                  switch (prot) {
                     case Protected:
                        addMemberToList(MemberListType_proMethods, md, true);
                        break;
                     case Package:
                        addMemberToList(MemberListType_pacMethods, md, true);
                        break;
                     case Public:
                        addMemberToList(MemberListType_pubMethods, md, true);
                        break;
                     case Private:
                        addMemberToList(MemberListType_priMethods, md, true);
                        break;
                  }
               }
            }
            break;
      }
   } 

   if (! isSimple) {
      // not a simple field -> not a simple struct
      m_isSimple = false;
   }
   
   // insert member in the detailed documentation section      

   if ((md->isRelated() && protectionLevelVisible(prot)) || md->isFriend()) {
      addMemberToList(MemberListType_relatedMembers, md, false);

   } else {
      switch (md->memberType()) {
         case MemberType_Service:      // UNO IDL
            addMemberToList(MemberListType_serviceMembers, md, false);
            break;

         case MemberType_Interface:    // UNO IDL
            addMemberToList(MemberListType_interfaceMembers, md, false);
            break;

         case MemberType_Property:
            addMemberToList(MemberListType_propertyMembers, md, false);
            break;

         case MemberType_Event:
            addMemberToList(MemberListType_eventMembers, md, false);
            break;


         case MemberType_DCOP:
            addMemberToList(MemberListType_functionMembers, md, false);
            break;

         case MemberType_Signal:      
            if (protectionLevelVisible(prot)) {
               addMemberToList(MemberListType_functionMembers, md, false);
            }
            break;

         case MemberType_Slot:
            if (protectionLevelVisible(prot)) {
               addMemberToList(MemberListType_functionMembers, md, false);
            }
            break;

         default: 
            // any of the other members

            if (protectionLevelVisible(prot)) {
               switch (md->memberType()) {
                  case MemberType_Typedef:
                     addMemberToList(MemberListType_typedefMembers, md, false);
                     break;

                  case MemberType_Enumeration:
                     addMemberToList(MemberListType_enumMembers, md, false);
                     break;

                  case MemberType_EnumValue:
                     addMemberToList(MemberListType_enumValMembers, md, false);
                     break;

                  case MemberType_Function:
                     if (md->isConstructor() || md->isDestructor()) {
                        QSharedPointer<MemberList> ml = createMemberList(MemberListType_constructors);
                        ml->append(md);

                     } else {
                        addMemberToList(MemberListType_functionMembers, md, false);
                     }

                     break;

                  case MemberType_Variable:
                     addMemberToList(MemberListType_variableMembers, md, false);
                     break;

                  default:
                     err("Unexpected member type %d found\n", md->memberType());
               }
            }
            break;
      }
   }
      
   // insert member in the appropriate member group 
   // do this ONLY AFTER inserting the member in the regular groups, addMemberToGroup(md,groupId);
   
   if (md->virtualness() == Pure) {
      m_isAbstract = true;
   }

   if (md->name() == "operator->") {
      m_arrowOperator = md;
   }
   
   if (addToAllList && ! (Config::getBool("hide-friend-compounds") && md->isFriend() &&  (md->typeString() == "friend class" || 
            md->typeString() == "friend struct" || md->typeString() == "friend union"))) {
      
      MemberInfo mi = MemberInfo(md, prot, md->virtualness(), false);
      QSharedPointer<MemberNameInfo> mni;

      if (m_allMemberNameInfoSDict == 0) {
         m_allMemberNameInfoSDict = new MemberNameInfoSDict();        
      }

      if ((mni = m_allMemberNameInfoSDict->find(md->name()))) {
         mni->append(mi);

      } else {
         mni = QSharedPointer<MemberNameInfo> (new MemberNameInfo(md->name()));
         mni->append(mi);

         m_allMemberNameInfoSDict->insert(mni->memberName(), mni);
      }
   }
}

void ClassDef::insertMember(QSharedPointer<MemberDef> md)
{
   internalInsertMember(md, md->protection(), true);
}

// compute the anchors for all members
void ClassDef::computeAnchors()
{   
   for (auto ml : m_memberLists ) {
      if ((ml->listType()&MemberListType_detailedLists) == 0) {
         setAnchors(ml);
      }
   }

   if (m_memberGroupSDict) {
      for (auto mg : *m_memberGroupSDict ) {
         mg->setAnchors();
      }
   }
}

void ClassDef::distributeMemberGroupDocumentation()
{
   if (m_memberGroupSDict) {
      for (auto mg : *m_memberGroupSDict ) {
         mg->distributeMemberGroupDocumentation();
      }
   }
}

void ClassDef::findSectionsInDocumentation()
{
   QSharedPointer<ClassDef> self = sharedFrom(this);
   docFindSections(documentation(), self, 0, docFile());

   if (m_memberGroupSDict) {    
      for (auto item : *m_memberGroupSDict) {
         item->findSectionsInDocumentation();
      }
   }
 
   for (auto item : m_memberLists) {
      if ((item->listType()&MemberListType_detailedLists) == 0) {
         item->findSectionsInDocumentation();
      }
   }
}


// add a file name to the used files set
void ClassDef::insertUsedFile(QSharedPointer<FileDef> fd)
{
   if (fd == 0) {
      return;
   }

   if (! m_files.contains(fd)) {
      m_files.append(fd);
   }

   if (m_templateInstances) {     
      for (auto &item : *m_templateInstances) {
         item->insertUsedFile(fd);
      }
   }
}

static void writeInheritanceSpecifier(OutputList &ol, BaseClassDef *bcd)
{
   if (bcd->prot != Public || bcd->virt != Normal) {
      ol.startTypewriter();
      ol.docify(" [");

      QStringList sl;

      if (bcd->prot == Protected) {
         sl.append("protected");

      } else if (bcd->prot == Private) {
         sl.append("private");

      }

      if (bcd->virt == Virtual) {
         sl.append("virtual");
      }

      QStringListIterator it(sl);

      while (it.hasNext()) {

         QString s = it.next();
         ol.docify(s);

         if (it.hasNext()) {
            ol.docify(", ");
         }
      }

      ol.docify("]");
      ol.endTypewriter();
   }
}

void ClassDef::setIncludeFile(QSharedPointer<FileDef> fd, const char *includeName, bool local, bool force)
{   
   if (!m_incInfo) {
      m_incInfo = new IncludeInfo;
   }

   if ((includeName && m_incInfo->includeName.isEmpty()) || (fd != 0 && m_incInfo->fileDef == 0) ) {

      m_incInfo->fileDef     = fd;
      m_incInfo->includeName = includeName;
      m_incInfo->local       = local;
   }

   if (force && includeName) {
      m_incInfo->includeName = includeName;
      m_incInfo->local       = local;
   }
}

// TODO: fix this: a nested template class can have multiple outer templates
//ArgumentList *ClassDef::outerTemplateArguments() const
//{
//  int ti;
//  ClassDef *pcd=0;
//  int pi=0;
//  if (m_tempArgs) return m_tempArgs;
//  // find the outer most class scope
//  while ((ti=name().find("::",pi))!=-1 &&
//      (pcd=getClass(name().left(ti)))==0
//        ) pi=ti+2;
//  if (pcd)
//  {
//    return pcd->templateArguments();
//  }
//  return 0;
//}

static void searchTemplateSpecs(QSharedPointer<Definition> d, QList<ArgumentList> &result,
                                QString &name, SrcLangExt lang)
{
   if (d->definitionType() == Definition::TypeClass) {

      if (d->getOuterScope()) {
         searchTemplateSpecs(d->getOuterScope(), result, name, lang);
      }

      QSharedPointer<ClassDef> cd = d.dynamicCast<ClassDef>();

      if (! name.isEmpty()) {
         name += "::";
      }

      QString clName = d->localName();

      if (clName.right(2) == "-p") {
         clName = clName.left(clName.length() - 2);
      }

      name += clName;
      bool isSpecialization = d->localName().indexOf('<') != -1;

      if (cd->templateArguments()) {
         result.append(* (cd->templateArguments()) );

         if (! isSpecialization) {
            name += tempArgListToString(cd->templateArguments(), lang);
         }
      }

   } else {
      name += d->qualifiedName();
   }
}

static void writeTemplateSpec(OutputList &ol, QSharedPointer<Definition> d, const QString &type, SrcLangExt lang)
{
   QList<ArgumentList> specs;
   QString name;

   searchTemplateSpecs(d, specs, name, lang);

   if (specs.count() > 0) { 
      // class has template scope specifiers
      ol.startSubsubsection();    

      for (auto al : specs ) {
         ol.docify("template<");
    
         auto nextItem = al.begin();

         for (auto a : al) { 
            ++nextItem;

            ol.docify(a.type);

            if (! a.name.isEmpty()) {
               ol.docify(" ");
               ol.docify(a.name);
            }

            if (a.defval.length() != 0) {
               ol.docify(" = ");
               ol.docify(a.defval);
            }

            if (nextItem != al.end()) {                               
               ol.docify(", ");
            }
         }

         ol.docify(">");
         ol.pushGeneratorState();
         ol.disableAllBut(OutputGenerator::Html);
         ol.lineBreak();
         ol.popGeneratorState();
      }

      ol.docify(type.toLower() + " " + name);
      ol.endSubsubsection();
      ol.writeString("\n");
   }
}

void ClassDef::writeBriefDescription(OutputList &ol, bool exampleFlag)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);

   if (hasBriefDescription()) {
      ol.startParagraph();
      ol.generateDoc(briefFile(), briefLine(), self, QSharedPointer<MemberDef>(), briefDescription(), true, false, 0, true, false);
      ol.pushGeneratorState();

      ol.disable(OutputGenerator::RTF);
      ol.writeString(" \n");
      ol.enable(OutputGenerator::RTF);
      ol.popGeneratorState();

      if (hasDetailedDescription() || exampleFlag) {
         writeMoreLink(ol, anchor());
      }

      ol.endParagraph();
   }
   ol.writeSynopsis();
}

void ClassDef::writeDetailedDocumentationBody(OutputList &ol)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);

   static bool repeatBrief = Config::getBool("repeat-brief");
   const QString docText = documentation();

   ol.startTextBlock();

   if (getLanguage() == SrcLangExt_Cpp) {
      writeTemplateSpec(ol, self, compoundTypeString(), getLanguage());
   }

   // repeat brief description
   if (! briefDescription().isEmpty() && repeatBrief) {
      ol.generateDoc(briefFile(), briefLine(), self, QSharedPointer<MemberDef>(), briefDescription(), false, false);
   }

   if (! briefDescription().isEmpty() && repeatBrief && ! docText.isEmpty()) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
      ol.writeString("\n\n");
      ol.popGeneratorState();
   }

   // write documentation
   if (! docText.isEmpty()) {
      ol.generateDoc(docFile(), docLine(), self, QSharedPointer<MemberDef>(), docText, true, false);
   }

   // write type constraints
   writeTypeConstraints(ol, self, &m_typeConstraints);

   // write examples
   if (hasExamples() && m_exampleSDict) {
      ol.startSimpleSect(BaseOutputDocInterface::Examples, 0, 0, theTranslator->trExamples() + ": ");
      ol.startDescForItem();
      //ol.startParagraph();

      writeExample(ol, m_exampleSDict);

      //ol.endParagraph();
      ol.endDescForItem();
      ol.endSimpleSect();
   }

   // ol.newParagraph();
   writeSourceDef(ol, name());
   ol.endTextBlock();
}

bool ClassDef::hasDetailedDescription() const
{
   static bool repeatBrief   = Config::getBool("repeat-brief");
   static bool sourceBrowser = Config::getBool("source-code");

   return ((! briefDescription().isEmpty() && repeatBrief) || ! documentation().isEmpty() ||
           (sourceBrowser && getStartBodyLine() != -1 && getBodyDef()));
}

// write the detailed description for this class
void ClassDef::writeDetailedDescription(OutputList &ol, const QString &, bool exampleFlag,
                                        const QString &title, const QString &anchor)
{
   if (hasDetailedDescription() || exampleFlag) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
      ol.writeRuler();
      ol.popGeneratorState();

      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);
      ol.writeAnchor(0, anchor.isEmpty() ? QString("details") : anchor);
      ol.popGeneratorState();

      if (!anchor.isEmpty()) {
         ol.pushGeneratorState();
         ol.disable(OutputGenerator::Html);
         ol.disable(OutputGenerator::Man);
         ol.writeAnchor(getOutputFileBase(), anchor);
         ol.popGeneratorState();
      }

      ol.startGroupHeader();
      ol.parseText(title);
      ol.endGroupHeader();

      writeDetailedDocumentationBody(ol);

   } else {
      //writeTemplateSpec(ol,this,pageType);
   }
}

QString ClassDef::generatedFromFiles() const
{
   QString result;
   SrcLangExt lang = getLanguage();

   if (lang == SrcLangExt_Fortran) {
      result = theTranslator->trGeneratedFromFilesFortran(getLanguage() == SrcLangExt_ObjC && 
                  m_compType == Interface ? Class : m_compType, m_files.count() == 1);

   } else if (isJavaEnum()) {
      result = theTranslator->trEnumGeneratedFromFiles(m_files.count() == 1);

   } else if (m_compType == Service) {
      result = theTranslator->trServiceGeneratedFromFiles(m_files.count() == 1);

   } else if (m_compType == Singleton) {
      result = theTranslator->trSingletonGeneratedFromFiles(m_files.count() == 1);

   } else {
      result = theTranslator->trGeneratedFromFiles(
                  getLanguage() == SrcLangExt_ObjC && m_compType == Interface ? Class : m_compType, m_files.count() == 1);
   }

   return result;
}

void ClassDef::showUsedFiles(OutputList &ol)
{
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Man);

   ol.writeRuler();
   ol.parseText(generatedFromFiles());

   bool first = true;
   
   for (auto fd : m_files) {
      if (first) {
         first = false;
         ol.startItemList();
      }

      ol.startItemListItem();
      QString path = fd->getPath();

      if (Config::getBool("full-path-names")) {
         ol.docify(stripFromPath(path));
      }

      QString fname = fd->name();
      if (! fd->getVersion().isEmpty()) { // append version if available
         fname += " (" + fd->getVersion() + ")";
      }

      // for HTML
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);

      if (fd->generateSourceFile()) {
         ol.writeObjectLink(0, fd->getSourceFileBase(), 0, fname);
      } else if (fd->isLinkable()) {
         ol.writeObjectLink(fd->getReference(), fd->getOutputFileBase(), 0, fname);
      } else {
         ol.docify(fname);
      }
      ol.popGeneratorState();

      // for other output formats
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
      if (fd->isLinkable()) {
         ol.writeObjectLink(fd->getReference(), fd->getOutputFileBase(), 0, fname);
      } else {
         ol.docify(fname);
      }
      ol.popGeneratorState();

      ol.endItemListItem();
   }
   if (!first) {
      ol.endItemList();
   }

   ol.popGeneratorState();
}

int ClassDef::countInheritanceNodes()
{
   int count = 0;

   if (m_inheritedBy) {
      for (auto ibcd : *m_inheritedBy) {
         QSharedPointer<ClassDef> icd = ibcd->classDef;

         if ( icd->isVisibleInHierarchy()) {
            count++;
         }
      }
   }

   if (m_parents) {     
      for (auto ibcd : *m_parents) {
         QSharedPointer<ClassDef> icd = ibcd->classDef;

         if ( icd->isVisibleInHierarchy()) {
            count++;
         }
      }
   }

   return count;
}

void ClassDef::writeInheritanceGraph(OutputList &ol)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);

   // count direct inheritance relations
   const int count = countInheritanceNodes();

   bool renderDiagram = false;

   if (Config::getBool("have-dot") && (Config::getBool("class-diagrams") || Config::getBool("class-graph"))) {

      // write class diagram using dot   
      DotClassGraph inheritanceGraph(self, DotNode::Inheritance);

      if (!inheritanceGraph.isTrivial() && !inheritanceGraph.isTooBig()) {
         ol.pushGeneratorState();
         ol.disable(OutputGenerator::Man);
         ol.startDotGraph();
         ol.parseText(theTranslator->trClassDiagram(qPrintable(displayName())));
         ol.endDotGraph(inheritanceGraph);
         ol.popGeneratorState();
         renderDiagram = true;
      }

   } else if (Config::getBool("class-diagrams") && count > 0) {
      // write class diagram using build-in generator
  
      ClassDiagram diagram(self); // create a diagram of this class
      ol.startClassDiagram();

      ol.disable(OutputGenerator::Man);
      ol.parseText(theTranslator->trClassDiagram( qPrintable(displayName())));

      ol.enable(OutputGenerator::Man);
      ol.endClassDiagram(diagram, getOutputFileBase(), qPrintable(displayName()));
      renderDiagram = true;
   }

   if (renderDiagram)  {
      // if we already show the inheritance relations graphically, then hide the text version   
      ol.disableAllBut(OutputGenerator::Man);
   }

   if (m_parents && m_parents->count() > 0) {
      ol.startParagraph();
     
      QString inheritLine = theTranslator->trInheritsList(m_parents->count());
      QRegExp marker("@[0-9]+");
      int index = 0, newIndex, matchLen;

      // now replace all markers in inheritLine with links to the classes      
      while ((newIndex = marker.indexIn(inheritLine, index)) != -1) {

         matchLen = marker.matchedLength();

         ol.parseText(inheritLine.mid(index, newIndex - index));
         bool ok;

         uint entryIndex = inheritLine.mid(newIndex + 1, matchLen - 1).toUInt(&ok);
         BaseClassDef *bcd = m_parents->at(entryIndex);

         if (ok && bcd) {
            QSharedPointer<ClassDef> cd = bcd->classDef;

            // use the class name but with the template arguments as given
            // in the inheritance relation
            QString displayName = insertTemplateSpecifierInScope(cd->displayName(), bcd->templSpecifiers);

            if (cd->isLinkable()) {
               ol.writeObjectLink(cd->getReference(), cd->getOutputFileBase(), cd->anchor(), displayName);
            } else {
               ol.docify(displayName);
            }

         } else {
            err("Invalid marker %d in inherits list\n", entryIndex);
         }

         index = newIndex + matchLen;
      }
      ol.parseText(inheritLine.right(inheritLine.length() - index));
      ol.endParagraph();
   }

   // write subclasses
   if (m_inheritedBy && m_inheritedBy->count() > 0) {
      ol.startParagraph();
      QString inheritLine = theTranslator->trInheritedByList(m_inheritedBy->count());
      QRegExp marker("@[0-9]+");

      int index = 0;
      int newIndex;
      int matchLen;

      // now replace all markers in inheritLine with links to the classes
      while ((newIndex = marker.indexIn(inheritLine, index)) != -1) {

         matchLen = marker.matchedLength();

         ol.parseText(inheritLine.mid(index, newIndex - index));

         bool ok;
         uint entryIndex = inheritLine.mid(newIndex + 1, matchLen - 1).toUInt(&ok);

         BaseClassDef *bcd = m_inheritedBy->at(entryIndex);

         if (ok && bcd) {
            QSharedPointer<ClassDef> cd = bcd->classDef;

            if (cd->isLinkable()) {
               ol.writeObjectLink(cd->getReference(), cd->getOutputFileBase(), cd->anchor(), cd->displayName());
            } else {
               ol.docify(cd->displayName());
            }

            writeInheritanceSpecifier(ol, bcd);
         }

         index = newIndex + matchLen;
      }
      ol.parseText(inheritLine.right(inheritLine.length() - index));
      ol.endParagraph();
   }

   if (renderDiagram) {
      ol.enableAll();
   }
}

void ClassDef::writeCollaborationGraph(OutputList &ol)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);

   if (Config::getBool("have-dot") /* && Config::getBool("dot-collaboration")*/) {
      DotClassGraph usageImplGraph(self, DotNode::Collaboration);

      if (!usageImplGraph.isTrivial()) {
         ol.pushGeneratorState();
         ol.disable(OutputGenerator::Man);
         ol.startDotGraph();
         ol.parseText(theTranslator->trCollaborationDiagram(qPrintable(displayName())));
         ol.endDotGraph(usageImplGraph);
         ol.popGeneratorState();
      }
   }
}

QString ClassDef::includeStatement() const
{
   SrcLangExt lang = getLanguage();
   bool isIDLorJava = (lang == SrcLangExt_IDL || lang == SrcLangExt_Java);

   if (isIDLorJava) {
      return "import";

   } else if (isObjectiveC()) {
      return "#import ";

   } else {
      return "#include ";
   }
}

void ClassDef::writeIncludeFiles(OutputList &ol)
{
   if (m_incInfo) {

      QString nm = m_incInfo->includeName.isEmpty() ? (m_incInfo->fileDef 
            ? m_incInfo->fileDef->docName() : "" ) : m_incInfo->includeName;

      if (! nm.isEmpty()) {
         ol.startParagraph();
         ol.startTypewriter();
         ol.docify(includeStatement());

         SrcLangExt lang = getLanguage();
         bool isIDLorJava = lang == SrcLangExt_IDL || lang == SrcLangExt_Java;

         if (m_incInfo->local || isIDLorJava) {
            ol.docify("\"");
         } else {
            ol.docify("<");
         }

         ol.pushGeneratorState();
         ol.disable(OutputGenerator::Html);
         ol.docify(nm);
         ol.disableAllBut(OutputGenerator::Html);
         ol.enable(OutputGenerator::Html);

         if (m_incInfo->fileDef) {
            ol.writeObjectLink(0, m_incInfo->fileDef->includeName(), 0, nm);
         } else {
            ol.docify(nm);
         }
         ol.popGeneratorState();
         if (m_incInfo->local || isIDLorJava) {
            ol.docify("\"");
         } else {
            ol.docify(">");
         }
         if (isIDLorJava) {
            ol.docify(";");
         }
         ol.endTypewriter();
         ol.endParagraph();
      }
   }
}

#if 0
void ClassDef::writeAllMembersLink(OutputList &ol)
{
   // write link to list of all members (HTML only)
   if (m_allMemberNameInfoSDict && ! Config::getBool("optimize-c") ) {
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);
      ol.startParagraph();
      ol.startTextLink(getMemberListFileName(), 0);
      ol.parseText(theTranslator->trListOfAllMembers());
      ol.endTextLink();
      ol.endParagraph();
      ol.enableAll();
      ol.popGeneratorState();
   }
}
#endif

void ClassDef::writeMemberGroups(OutputList &ol, bool showInline)
{
   // write user defined member groups
   QSharedPointer<ClassDef> self = sharedFrom(this);

   if (m_memberGroupSDict) {          
      for (auto mg : *m_memberGroupSDict)  {
         if (! mg->allMembersInSameSection() || !m_subGrouping) { // group is in its own section
            mg->writeDeclarations(ol, self, QSharedPointer<NamespaceDef>(), QSharedPointer<FileDef>(), 
                     QSharedPointer<GroupDef>(), showInline);

         } else { 
            // add this group to the corresponding member section
            
         }
      }
   }
}

void ClassDef::writeNestedClasses(OutputList &ol, const QString &title)
{
   // nested classes
   if (m_innerClasses) {
      m_innerClasses->writeDeclaration(ol, 0, title, true);
   }
}

void ClassDef::writeInlineClasses(OutputList &ol)
{
   if (m_innerClasses) {
      m_innerClasses->writeDocumentation(ol, this);
   }
}

void ClassDef::startMemberDocumentation(OutputList &ol)
{
   if (Config::getBool("separate-member-pages")) {
      ol.disable(OutputGenerator::Html);
      Doxy_Globals::suppressDocWarnings = true;
   }
}

void ClassDef::endMemberDocumentation(OutputList &ol)
{ 
   if (Config::getBool("separate-member-pages")) {
      ol.enable(OutputGenerator::Html);
      Doxy_Globals::suppressDocWarnings = false;
   }
}

void ClassDef::startMemberDeclarations(OutputList &ol)
{
   ol.startMemberSections();
}

void ClassDef::endMemberDeclarations(OutputList &ol)
{
   static bool inlineInheritedMembers = Config::getBool("inline-inherited-member");

   if (!inlineInheritedMembers && countAdditionalInheritedMembers() > 0) {
      ol.startMemberHeader("inherited");
      ol.parseText(theTranslator->trAdditionalInheritedMembers());
      ol.endMemberHeader();
      writeAdditionalInheritedMembers(ol);
   }
   ol.endMemberSections();
}

void ClassDef::writeAuthorSection(OutputList &ol)
{
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Man);
   ol.writeString("\n");
   ol.startGroupHeader();
   ol.parseText(theTranslator->trAuthor(true, true));
   ol.endGroupHeader();
   ol.parseText(theTranslator->trGeneratedAutomatically(Config::getString("project-name")));
   ol.popGeneratorState();
}


void ClassDef::writeSummaryLinks(OutputList &ol)
{
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
   
   bool first = true;
   SrcLangExt lang = getLanguage();  
   
   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Class)) {

      if (lde->kind() == LayoutDocEntry::ClassNestedClasses && m_innerClasses  && m_innerClasses->declVisible()) {
         LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
         ol.writeSummaryLink("", "nested-classes", ls->title(lang), first);
         first = false;

      } else if (lde->kind() == LayoutDocEntry::ClassAllMembersLink && m_allMemberNameInfoSDict &&  ! Config::getBool("optimize-c") ) {
         ol.writeSummaryLink(getMemberListFileName(), "all-members-list", theTranslator->trListOfAllMembers(), first);
         first = false;

      } else if (lde->kind() == LayoutDocEntry::MemberDecl) {
         LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
         QSharedPointer<MemberList> ml = getMemberList(lmd->type);

         if (ml && ml->declVisible()) {
            ol.writeSummaryLink("", MemberList::listTypeAsString(ml->listType()), lmd->title(lang), first);
            first = false;
         }
      }
   }
  
   if (! first) {
      ol.writeString("  </div>\n");
   }

   ol.popGeneratorState();
}

void ClassDef::writeTagFile(QTextStream &tagFile)
{
   if (! isLinkableInProject()) {
      return;
   }

   tagFile << "  <compound kind=\"" << compoundTypeString();
   tagFile << "\"";

   if (isObjectiveC()) {
      tagFile << " objc=\"yes\"";
   }

   tagFile << ">" << endl;
   tagFile << "    <name>" << convertToXML(name()) << "</name>" << endl;
   tagFile << "    <filename>" << convertToXML(getOutputFileBase()) << Doxy_Globals::htmlFileExtension << "</filename>" << endl;

   if (!anchor().isEmpty()) {
      tagFile << "    <anchor>" << convertToXML(anchor()) << "</anchor>" << endl;
   }

   QString idStr = id();
   if (! idStr.isEmpty()) {
      tagFile << "    <clangid>" << convertToXML(idStr) << "</clangid>" << endl;
   }

   for (auto a : m_tempArgs) {
      tagFile << "    <templarg>" << convertToXML(a.name) << "</templarg>" << endl;      
   }

   if (m_parents) {
     
      for (auto ibcd : *m_parents) {
         QSharedPointer<ClassDef> cd = ibcd->classDef;

         if (cd && cd->isLinkable()) {

            if (! Config::getString("generate_tagfile").isEmpty()) {
               tagFile << "    <base";

               if (ibcd->prot == Protected) {
                  tagFile << " protection=\"protected\"";

               } else if (ibcd->prot == Private) {
                  tagFile << " protection=\"private\"";
               }

               if (ibcd->virt == Virtual) {
                  tagFile << " virtualness=\"virtual\"";
               }
               tagFile << ">" << convertToXML(cd->name()) << "</base>" << endl;
            }
         }
      }
   }
  
   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Class)) {
      switch (lde->kind()) {

         case LayoutDocEntry::ClassNestedClasses: {
            if (m_innerClasses) {
              
               for (auto innerCd : *m_innerClasses) {

                  if (innerCd->isLinkableInProject() && innerCd->templateMaster() == 0 && protectionLevelVisible(innerCd->protection()) &&
                        ! innerCd->isEmbeddedInOuterScope() ) {
                     tagFile << "    <class kind=\"" << innerCd->compoundTypeString() << "\">" 
                             << convertToXML(innerCd->name()) << "</class>" << endl;
                  }
               }
            }
         }
         break;

         case LayoutDocEntry::MemberDecl: {
            LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
            QSharedPointer<MemberList> ml = getMemberList(lmd->type);

            if (ml) {
               ml->writeTagFile(tagFile);
            }
         }
         break;

         case LayoutDocEntry::MemberGroups: {
            if (m_memberGroupSDict) {
               for (auto mg : *m_memberGroupSDict) {
                  mg->writeTagFile(tagFile);
               }
            }
         }
         break;

         default:
            break;
      }
   }
   writeDocAnchorsToTagFile(tagFile);
   tagFile << "  </compound>" << endl;
}

/** Write class documentation inside another container (i.e. a group) */
void ClassDef::writeInlineDocumentation(OutputList &ol)
{
   bool isSimple = m_isSimple;

   ol.addIndexItem(name(), 0);
 
   // part 1: anchor and title
   QString s = compoundTypeString() + " " + name();

   // part 1a
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
   {
      // only HTML only
      ol.writeAnchor(0, anchor());
      ol.startMemberDoc(0, 0, 0, 0, false);
      ol.startMemberDocName(false);
      ol.parseText(s);
      ol.endMemberDocName();
      ol.endMemberDoc(false);
      ol.writeString("</div>");
      ol.startIndent();
   }
   ol.popGeneratorState();

   // part 1b
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);
   ol.disable(OutputGenerator::Man);
   {
      // for LaTeX/RTF only
      ol.writeAnchor(getOutputFileBase(), anchor());
   }
   ol.popGeneratorState();

   // part 1c
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);
   {
      // for LaTeX/RTF/Man
      ol.startGroupHeader(1);
      ol.parseText(s);
      ol.endGroupHeader(1);
   }
   ol.popGeneratorState();

   SrcLangExt lang = getLanguage();

   // part 2: the header and detailed description

   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Class)) {

      switch (lde->kind()) {
         case LayoutDocEntry::BriefDesc: {
            // since we already shown the brief description in the
            // declaration part of the container, so we use this to
            // show the details on top.
            writeDetailedDocumentationBody(ol);
         }
         break;

         case LayoutDocEntry::ClassInheritanceGraph:
            writeInheritanceGraph(ol);
            break;

         case LayoutDocEntry::ClassCollaborationGraph:
            writeCollaborationGraph(ol);
            break;

         case LayoutDocEntry::MemberDeclStart:
            if (!isSimple) {
               startMemberDeclarations(ol);
            }
            break;

         case LayoutDocEntry::MemberDecl: {
            LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;

            if (! isSimple) {
               writeMemberDeclarations(ol, lmd->type, lmd->title(lang), lmd->subtitle(lang), true);
            }
         }
         break;
         case LayoutDocEntry::MemberGroups:
            if (! isSimple) {
               writeMemberGroups(ol, true);
            }
            break;
         case LayoutDocEntry::MemberDeclEnd:
            if (!isSimple) {
               endMemberDeclarations(ol);
            }
            break;
         case LayoutDocEntry::MemberDefStart:
            if (!isSimple) {
               startMemberDocumentation(ol);
            }
            break;
         case LayoutDocEntry::MemberDef: {
            LayoutDocEntryMemberDef *lmd = (LayoutDocEntryMemberDef *)lde;
            if (isSimple) {
               writeSimpleMemberDocumentation(ol, lmd->type);
            } else {
               writeMemberDocumentation(ol, lmd->type, lmd->title(lang), true);
            }
         }
         break;
         case LayoutDocEntry::MemberDefEnd:
            if (!isSimple) {
               endMemberDocumentation(ol);
            }
            break;
         default:
            break;
      }
   }

   // part 3: close the block
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
   {
      // HTML only
      ol.endIndent();
   }
   ol.popGeneratorState();
}

void ClassDef::writeMoreLink(OutputList &ol, const QString &anchor)
{
   // TODO: clean up by moving it to the output generators
   static bool pdfHyperlinks = Config::getBool("latex-hyper-pdf");
   static bool rtfHyperlinks = Config::getBool("rtf-hyperlinks");
   static bool usePDFLatex   = Config::getBool("latex-pdf");

   // HTML only
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
   ol.docify(" ");
   ol.startTextLink(getOutputFileBase(), anchor.isEmpty() ? "details" : anchor);
   ol.parseText(theTranslator->trMore());
   ol.endTextLink();
   ol.popGeneratorState();

   if (!anchor.isEmpty()) {
      ol.pushGeneratorState();

      // LaTeX + RTF
      ol.disable(OutputGenerator::Html);
      ol.disable(OutputGenerator::Man);
      if (!(usePDFLatex && pdfHyperlinks)) {
         ol.disable(OutputGenerator::Latex);
      }
      if (!rtfHyperlinks) {
         ol.disable(OutputGenerator::RTF);
      }
      ol.docify(" ");
      ol.startTextLink(getOutputFileBase(), anchor);
      ol.parseText(theTranslator->trMore());
      ol.endTextLink();

      // RTF only
      ol.disable(OutputGenerator::Latex);
      ol.writeString("\\par");
      ol.popGeneratorState();
   }
}

bool ClassDef::visibleInParentsDeclList() const
{
   static bool extractPrivate      = Config::getBool("extract-private");
   static bool hideUndocClasses    = Config::getBool("hide-undoc-classes");
   static bool extractLocalClasses = Config::getBool("extract-local-classes");

   bool linkable = isLinkable();
   return (!isAnonymous() && !isExtension() &&
           (protection() !=::Private || extractPrivate) &&
           (linkable || (!hideUndocClasses && (!isLocal() || extractLocalClasses)))
          );
}

void ClassDef::writeDeclarationLink(OutputList &ol, bool &found, const QString &header, bool localNames)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);

   // static bool fortranOpt = Config::getBool("optimize-fortran");
   SrcLangExt lang = getLanguage();

   if (visibleInParentsDeclList()) {

      if (! found) { 
         // first class
         ol.startMemberHeader("nested-classes");

         if (! header.isEmpty()) {
            ol.parseText(header);

         } else {
            ol.parseText(lang == SrcLangExt_Fortran ? theTranslator->trDataTypes() : theTranslator->trCompounds());

         }

         ol.endMemberHeader();
         ol.startMemberList();
         found = true;
      }

      ol.startMemberDeclaration();
      ol.startMemberItem(anchor(), false);

      QString ctype = compoundTypeString();
      QString cname = displayName(! localNames);

      ol.writeString(ctype);
      ol.writeString(" ");
      ol.insertMemberAlign();

      if (isLinkable()) {
         ol.writeObjectLink(getReference(), getOutputFileBase(), anchor(), cname );

      } else {
         ol.startBold();
         ol.docify(cname);
         ol.endBold();
      }

      ol.endMemberItem();

      // add the brief description if available
      if (! briefDescription().isEmpty() && Config::getBool("brief-member-desc")) {

         DocRoot *rootNode = validatingParseDoc(briefFile(), briefLine(), self, QSharedPointer<MemberDef>(),
                                                briefDescription(), false, false, "", true, false);

         if (rootNode && ! rootNode->isEmpty()) {
            ol.startMemberDescription(anchor());
            ol.writeDoc(rootNode, self, QSharedPointer<MemberDef>());

            if (isLinkableInProject()) {
               writeMoreLink(ol, anchor());
            }

            ol.endMemberDescription();
         }
         delete rootNode;
      }
      ol.endMemberDeclaration(anchor(), 0);
   }
}

void ClassDef::addClassAttributes(OutputList &ol)
{
   QStringList sl;
   if (isFinal()) {
      sl.append("final");
   }
   if (isSealed()) {
      sl.append("sealed");
   }
   if (isAbstract()) {
      sl.append("abstract");
   }
   if (getLanguage() == SrcLangExt_IDL && isPublished()) {
      sl.append("published");
   }

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   if (sl.count() > 0) {
      ol.startLabels();

      auto nextItem = sl.begin();

      for (auto s : sl) {         
         ++nextItem;                   
         ol.writeLabel(qPrintable(s), nextItem != sl.end());         
      }

      ol.endLabels();
   }

   ol.popGeneratorState();
}

void ClassDef::writeDocumentationContents(OutputList &ol, const QString &pageTitle)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);

   ol.startContents();

   QString pageType = " " + compoundTypeString();
  
   Doxy_Globals::indexList->addIndexItem(self, QSharedPointer<MemberDef>());

   if (Doxy_Globals::searchIndex) {
      Doxy_Globals::searchIndex->setCurrentDoc(self, anchor(), false);
      Doxy_Globals::searchIndex->addWord(localName(), true);
   }

   bool exampleFlag = hasExamples();
 
   // start flexible part 
   SrcLangExt lang = getLanguage();

   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Class) ) { 

      switch (lde->kind()) {

         case LayoutDocEntry::BriefDesc:
            writeBriefDescription(ol, exampleFlag);
            break;

         case LayoutDocEntry::ClassIncludes:
            writeIncludeFiles(ol);
            break;

         case LayoutDocEntry::ClassInheritanceGraph:
            writeInheritanceGraph(ol);
            break;

         case LayoutDocEntry::ClassCollaborationGraph:
            writeCollaborationGraph(ol);
            break;

         case LayoutDocEntry::ClassAllMembersLink:
            //writeAllMembersLink(ol); // this is now part of the summary links
            break;

         case LayoutDocEntry::MemberDeclStart:
            startMemberDeclarations(ol);
            break;

         case LayoutDocEntry::MemberGroups:
            writeMemberGroups(ol);
            break;

         case LayoutDocEntry::MemberDecl: {
            LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
            writeMemberDeclarations(ol, lmd->type, lmd->title(lang), lmd->subtitle(lang));
         }
         break;

         case LayoutDocEntry::ClassNestedClasses: {
            LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
            writeNestedClasses(ol, ls->title(lang));
         }
         break;

         case LayoutDocEntry::MemberDeclEnd:
            endMemberDeclarations(ol);
            break;

         case LayoutDocEntry::DetailedDesc: {
            LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
            writeDetailedDescription(ol, pageType, exampleFlag, ls->title(lang));
         }
         break;

         case LayoutDocEntry::MemberDefStart:
            startMemberDocumentation(ol);
            break;

         case LayoutDocEntry::ClassInlineClasses:
            writeInlineClasses(ol);
            break;

         case LayoutDocEntry::MemberDef: {
            LayoutDocEntryMemberDef *lmd = (LayoutDocEntryMemberDef *)lde;
            writeMemberDocumentation(ol, lmd->type, lmd->title(lang));
         }
         break;

         case LayoutDocEntry::MemberDefEnd:
            endMemberDocumentation(ol);
            break;

         case LayoutDocEntry::ClassUsedFiles:
            showUsedFiles(ol);
            break;

         case LayoutDocEntry::AuthorSection:
            writeAuthorSection(ol);
            break;

         case LayoutDocEntry::NamespaceNestedNamespaces:
         case LayoutDocEntry::NamespaceNestedConstantGroups:
         case LayoutDocEntry::NamespaceClasses:
         case LayoutDocEntry::NamespaceInlineClasses:
         case LayoutDocEntry::FileClasses:
         case LayoutDocEntry::FileNamespaces:
         case LayoutDocEntry::FileConstantGroups:
         case LayoutDocEntry::FileIncludes:
         case LayoutDocEntry::FileIncludeGraph:
         case LayoutDocEntry::FileIncludedByGraph:
         case LayoutDocEntry::FileSourceLink:
         case LayoutDocEntry::FileInlineClasses:
         case LayoutDocEntry::GroupClasses:
         case LayoutDocEntry::GroupInlineClasses:
         case LayoutDocEntry::GroupNamespaces:
         case LayoutDocEntry::GroupDirs:
         case LayoutDocEntry::GroupNestedGroups:
         case LayoutDocEntry::GroupFiles:
         case LayoutDocEntry::GroupGraph:
         case LayoutDocEntry::GroupPageDocs:
         case LayoutDocEntry::DirSubDirs:
         case LayoutDocEntry::DirFiles:

         case LayoutDocEntry::DirGraph:
            err("Internal problem: Member %d should not be part of LayoutDocManager::Class entry list\n", lde->kind());
            break;
      }
   }

   ol.endContents();
}

QString ClassDef::title() const
{
   QString pageTitle;
   SrcLangExt lang = getLanguage();

   if (lang == SrcLangExt_Fortran) {
      pageTitle = theTranslator->trCompoundReferenceFortran( qPrintable(displayName()), m_compType, ! m_tempArgs.isEmpty() );         

   } else if (isJavaEnum()) {
      pageTitle = theTranslator->trEnumReference(qPrintable(displayName()));

   } else if (m_compType == Service) {
      pageTitle = theTranslator->trServiceReference(qPrintable(displayName()));

   } else if (m_compType == Singleton) {
      pageTitle = theTranslator->trSingletonReference(qPrintable(displayName()));

   } else {
      if (Config::getBool("hide-compound-ref")) {
         pageTitle = displayName();

      } else {         
         ClassDef::CompoundType compType;
    
         if (m_compType == Interface && getLanguage() == SrcLangExt_ObjC)  { 
            compType = ClassDef::Class;

         } else  { 
            compType = m_compType;

         } 

         pageTitle = theTranslator->trCompoundReference(qPrintable(displayName()), compType, ! m_tempArgs.isEmpty() );
      }
   }

   return pageTitle;
}

// write all documentation for this class
void ClassDef::writeDocumentation(OutputList &ol)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);

   static bool generateTreeView = Config::getBool("generate-treeview");
   // static bool fortranOpt = Config::getBool("optimize-fortran");

   QString pageTitle = title();
   startFile(ol, getOutputFileBase(), name(), pageTitle, HLI_ClassVisible, ! generateTreeView);

   if (! generateTreeView) {
      if (getOuterScope() != Doxy_Globals::globalScope) {
         writeNavigationPath(ol);
      }
      ol.endQuickIndices();
   }

   startTitle(ol, getOutputFileBase(), this);
   ol.parseText(pageTitle);

   addClassAttributes(ol);
   addGroupListToTitle(ol, self);

   endTitle(ol, getOutputFileBase(), displayName());
   writeDocumentationContents(ol, pageTitle);
   endFileWithNavPath(self, ol);

   if (Config::getBool("separate-member-pages")) {
      writeMemberPages(ol);
   }
}

void ClassDef::writeMemberPages(OutputList &ol)
{  
   QSharedPointer<ClassDef> self = sharedFrom(this);

   // Member definitions on separate pages  
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
   
   for (auto ml : m_memberLists) {
      ml->countDocMembers();

      if (ml->numDocMembers() > 0 && (ml->listType() & MemberListType_detailedLists)) {
         ml->writeDocumentationPage(ol, qPrintable(displayName()), self);
      }
   }

   ol.popGeneratorState();
}

void ClassDef::writeQuickMemberLinks(OutputList &ol, MemberDef *currentMd) const
{
   static bool createSubDirs = Config::getBool("create-subdirs");

   ol.writeString("      <div class=\"navtab\">\n");
   ol.writeString("        <table>\n");

   if (m_allMemberNameInfoSDict) {
    
      for (auto mni : *m_allMemberNameInfoSDict) {

         for (auto mi : *mni) {
            QSharedPointer<MemberDef> md = mi.memberDef;

            if (md->getClassDef() == this && md->isLinkable() && !md->isEnumValue()) {
               ol.writeString("          <tr><td class=\"navtab\">");

               if (md->isLinkableInProject()) {
                  if (md == currentMd) { // selected item => highlight
                     ol.writeString("<a class=\"qindexHL\" ");
                  } else {
                     ol.writeString("<a class=\"qindex\" ");
                  }
                  ol.writeString("href=\"");
                  if (createSubDirs) {
                     ol.writeString("../../");
                  }
                  ol.writeString(md->getOutputFileBase() + Doxy_Globals::htmlFileExtension + "#" + md->anchor());
                  ol.writeString("\">");
                  ol.writeString(convertToHtml(md->name()));
                  ol.writeString("</a>");
               }
               ol.writeString("</td></tr>\n");
            }
         }
      }
   }

   ol.writeString("        </table>\n");
   ol.writeString("      </div>\n");
}

void ClassDef::writeDocumentationForInnerClasses(OutputList &ol)
{
   // write inner classes after the parent, so the tag files contain
   // the definition in proper order!
   if (m_innerClasses) {
    
      for (auto innerCd : *m_innerClasses) {
         if (innerCd->isLinkableInProject() && innerCd->templateMaster() == 0 &&
               protectionLevelVisible(innerCd->protection()) && !innerCd->isEmbeddedInOuterScope() ) {

            msg("Generating docs for nested compound %s\n", qPrintable(innerCd->name()));
            innerCd->writeDocumentation(ol);
            innerCd->writeMemberList(ol);
         }

         innerCd->writeDocumentationForInnerClasses(ol);
      }
   }
}

// write the list of all (inherited) members for this class
void ClassDef::writeMemberList(OutputList &ol)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);

   static bool cOpt = Config::getBool("optimize-c");
   static bool generateTreeView = Config::getBool("generate-treeview");

   if (m_allMemberNameInfoSDict == 0 || cOpt) {
      return;
   }

   // only for HTML
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   QString memListFile = getMemberListFileName();

   startFile(ol, memListFile, memListFile, theTranslator->trMemberList(),
             HLI_ClassVisible, !generateTreeView, getOutputFileBase());

   if (!generateTreeView) {
      if (getOuterScope() != Doxy_Globals::globalScope) {
         writeNavigationPath(ol);
      }
      ol.endQuickIndices();
   }

   startTitle(ol, 0);
   ol.parseText((displayName() + " " + theTranslator->trMemberList()));
   endTitle(ol, 0, 0);

   ol.startContents();
   ol.startParagraph();
   ol.parseText(theTranslator->trThisIsTheListOfAllMembers());
   ol.writeObjectLink(getReference(), getOutputFileBase(), anchor(), displayName());
   ol.parseText(theTranslator->trIncludingInheritedMembers());
   ol.endParagraph();
 
   ol.writeString("<table class=\"directory\">\n");

   int idx = 0;
  
   for (auto mni : *m_allMemberNameInfoSDict) {

      for (auto mi : *mni) {
         QSharedPointer<MemberDef> md = mi.memberDef;
         QSharedPointer<ClassDef>  cd = md->getClassDef();

         Protection prot = mi.prot;
         Specifier virt = md->virtualness();
        
         if (cd && ! md->name().isEmpty() && md->name()[0] != '@') {
            bool memberWritten = false;

            if (cd->isLinkable() && md->isLinkable()) {
               // create a link to the documentation            
               QString name = mi.ambiguityResolutionScope + md->name();

               ol.writeString("  <tr");
               if ((idx & 1) == 0) {
                  ol.writeString(" class=\"even\"");
               }

               idx++;
               ol.writeString("><td class=\"entry\">");

               if (cd->isObjectiveC()) {
                  if (md->isObjCMethod()) {

                     if (md->isStatic()) {
                        ol.writeString("+&#160;</td><td>");
                     } else {
                        ol.writeString("-&#160;</td><td>");
                     }

                  } else {
                     ol.writeString("</td><td class=\"entry\">");
                  }
               }
               if (md->isObjCMethod()) {
                  ol.writeObjectLink(md->getReference(), md->getOutputFileBase(), md->anchor(), md->name()); 

               } else {
                  ol.writeObjectLink(md->getReference(), md->getOutputFileBase(), md->anchor(), name);

                  if ( md->isFunction() || md->isSignal() || md->isSlot() || (md->isFriend() && ! md->argsString().isEmpty() )) {
                     ol.docify(md->argsString());

                  } else if (md->isEnumerate()) {
                     ol.parseText(" " + theTranslator->trEnumName());

                  } else if (md->isEnumValue()) {
                     ol.parseText(" " + theTranslator->trEnumValue());

                  } else if (md->isTypedef()) {
                     ol.docify(" typedef");

                  } else if (md->isFriend() && md->typeString() != "friend class") {
                     ol.docify(" class");
                  }

               }
               ol.writeString("</td>");
               memberWritten = true;

            } else if (! cd->isArtificial() && ! Config::getBool("hide-undoc-members") &&
                       (protectionLevelVisible(md->protection()) || md->isFriend()))  {

               // no documentation, generate link to the class instead
           
               ol.writeString("  <tr bgcolor=\"#f0f0f0\"");
               if ((idx & 1) == 0) {
                  ol.writeString(" class=\"even\"");
               }

               idx++;

               ol.writeString("><td class=\"entry\">");
               if (cd->isObjectiveC()) {
                  if (md->isObjCMethod()) {
                     if (md->isStatic()) {
                        ol.writeString("+&#160;</td><td class=\"entry\">");
                     } else {
                        ol.writeString("-&#160;</td><td class=\"entry\">");
                     }
                  } else {
                     ol.writeString("</td><td class=\"entry\">");
                  }
               }

               ol.startBold();
               ol.docify(md->name());
               ol.endBold();

               if (! md->isObjCMethod()) {
                  if ( md->isFunction() || md->isSignal() || md->isSlot() ) {
                     ol.docify(md->argsString());

                  } else if (md->isEnumerate()) {
                     ol.parseText(" " + theTranslator->trEnumName());

                  } else if (md->isEnumValue()) {
                     ol.parseText(" " + theTranslator->trEnumValue());

                  } else if (md->isTypedef()) {
                     ol.docify(" typedef");
                  }
               }

               ol.writeString(" (");
               ol.parseText(theTranslator->trDefinedIn() + " ");

               if (cd->isLinkable()) {
                  ol.writeObjectLink(
                     cd->getReference(),
                     cd->getOutputFileBase(),
                     cd->anchor(),
                     qPrintable(cd->displayName()));

               } else {
                  ol.startBold();
                  ol.docify(cd->displayName());
                  ol.endBold();
               }
               ol.writeString(")");
               ol.writeString("</td>");
               memberWritten = true;
            }

            if (memberWritten) {
               ol.writeString("<td class=\"entry\">");

               ol.writeObjectLink(cd->getReference(), cd->getOutputFileBase(), cd->anchor(),
                                  md->category() ? md->category()->displayName() : cd->displayName());

               ol.writeString("</td>");
               ol.writeString("<td class=\"entry\">");
            }
            
            QStringList sl;
            md->getLabels(sl, self);   

            // not needed           
            sl.removeOne("inherited");                                

            if (memberWritten) {

               if (sl.count() > 0) {       
                  ol.writeString("<span class=\"mlabels\">");   
                 
                  for (auto s : sl)  {                  
                     ol.writeString("<span class=\"mlabel\">");                               
                     ol.docify(s);                                  
                     ol.writeString("</span>");                 
                  }
   
                  ol.writeString("</span>");
               }
            
               ol.writeString("</td>");
               ol.writeString("</tr>\n");
            }
         }
      }
   }
  
   ol.writeString("</table>");

   endFile(ol);
   ol.popGeneratorState();
}


// add a reference to an example
bool ClassDef::addExample(const QString &anchor, const QString &nameStr, const QString &file)
{
   if (m_exampleSDict == 0) {
      m_exampleSDict = new ExampleSDict;      
   }

   if (! m_exampleSDict->find(nameStr)) {
      QSharedPointer<Example> e (new Example);

      e->anchor = anchor;
      e->name   = nameStr;
      e->file   = file;

      m_exampleSDict->insert(nameStr, e);

      return true;
   }
   return false;
}

// returns true if this class is used in an example
bool ClassDef::hasExamples() const
{
   bool result = false;

   if (m_exampleSDict) {
      result = m_exampleSDict->count() > 0;
   }
   return result;
}

void ClassDef::addTypeConstraint(const QString &typeConstraint, const QString &type)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);

   static bool hideUndocRelation = Config::getBool("hide-undoc-relations");
   
   if (typeConstraint.isEmpty() || type.isEmpty()) {
      return;
   }

   QSharedPointer<ClassDef> cd = getResolvedClass(self, getFileDef(), typeConstraint);

   if (cd == nullptr && ! hideUndocRelation) {
      cd = QMakeShared<ClassDef>(getDefFileName(), getDefLine(), getDefColumn(), typeConstraint, ClassDef::Class);
      cd->setUsedOnly(true);
      cd->setLanguage(getLanguage());

      Doxy_Globals::hiddenClasses->insert(typeConstraint, cd);
   }
  
   if (cd != nullptr) {        
      QSharedPointer<ConstraintClassDef> ccd = m_constraintClassDict.value(typeConstraint);
   
      if (ccd == nullptr) {
         ccd = QMakeShared<ConstraintClassDef>(cd);
         m_constraintClassDict.insert(typeConstraint, ccd);
      }
      
      ccd->addAccessor(type); 
   }
}

// Java Type Constrains: A<T extends C & I>
void ClassDef::addTypeConstraints()
{   
   for (auto a : m_tempArgs) {
   
      if (! a.typeConstraint.isEmpty()) {
         QString typeConstraint;
         int index = 0;
         int p = 0;

         while ((index = a.typeConstraint.indexOf('&', p)) != -1) { 
            // typeConstraint="A &I" for C<T extends A & I> 
         
            typeConstraint = a.typeConstraint.mid(p, index - p).trimmed();
            addTypeConstraint(typeConstraint, a.type);
            p = index + 1;
         }
   
         typeConstraint = a.typeConstraint.right(a.typeConstraint.length() - p).trimmed();
         addTypeConstraint(typeConstraint, a.type);
      }
   }

}

// C# Type Constraints: D<T> where T : C, I
void ClassDef::setTypeConstraints(ArgumentList *al)
{
   if (al == 0) {
      return;
   }
  
   m_typeConstraints = ArgumentList();
  
   for (auto a : *al) {
      m_typeConstraints.append(a);
   }
}

void ClassDef::setTemplateArguments(ArgumentList *al)
{
   if (al == 0) {
      return;
   }
 
   m_tempArgs = ArgumentList();

   for (auto a : *al) {
      m_tempArgs.append(a);
   }
}

/*! Returns \c true if this class or a class inheriting from this class
 *  is \e not defined in an external tag file.
 */
bool ClassDef::hasNonReferenceSuperClass()
{
   bool found = (! isReference() && isLinkableInProject() && ! isHidden());

   if (found) {
      // this class is not a reference
      return true; 
   }

   if (m_inheritedBy) {     

      for (auto bcli : *m_inheritedBy) {
         if (found) {
            break;
         }

         // for each super class
         QSharedPointer<ClassDef> bcd = bcli->classDef;

         // recurse into the super class branch
         found = found || bcd->hasNonReferenceSuperClass();

         if (! found) {
            // look for template instances that might have non-reference super classes
            QHash<QString, QSharedPointer<ClassDef>> *cil = bcd->getTemplateInstances();

            if (cil) {              

               for (auto tidi : *cil) {
                  // for each template instance, recurse into the template instance branch                                

                  found = tidi->hasNonReferenceSuperClass();

                  if (found) {
                     break;
                  }

               }
            }
         }
      }
   }

   return found;
}

/*! called from MemberDef::writeDeclaration() to (recusively) write the
 *  definition of an anonymous struct, union or class.
 */
void ClassDef::writeDeclaration(OutputList &ol, QSharedPointer<MemberDef> md, bool inGroup, 
                  QSharedPointer<ClassDef> inheritedFrom, const QString &inheritId)
{   
   QSharedPointer<ClassDef> self = sharedFrom(this);

   ol.docify(compoundTypeString());
   QString cn = displayName(false);

   if (! cn.isEmpty()) {
      ol.docify(" ");

      if (md && isLinkable()) {
         ol.writeObjectLink(0, 0, md->anchor(), cn);

      } else {
         ol.startBold();
         ol.docify(cn);
         ol.endBold();
      }
   }

   ol.docify(" {");
   ol.endMemberItem();

   // write user defined member groups
   if (m_memberGroupSDict) {     
      for (auto mg : *m_memberGroupSDict){
         mg->setInGroup(inGroup);
         mg->writePlainDeclarations(ol, self, QSharedPointer<NamespaceDef>(), QSharedPointer<FileDef>(),
                                    QSharedPointer<GroupDef>(), inheritedFrom, inheritId);
      }
   }
   
   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Class)) {
      if (lde->kind() == LayoutDocEntry::MemberDecl) {
         LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
         writePlainMemberDeclaration(ol, lmd->type, inGroup, inheritedFrom, inheritId);
      }
   }
}

/*! a link to this class is possible within this project */
bool ClassDef::isLinkableInProject() const
{
   static bool extractLocal   = Config::getBool("extract-local-classes");
   static bool extractStatic  = Config::getBool("extract-static");
   static bool hideUndoc      = Config::getBool("hide-undoc-classes");

   if (m_templateMaster) {
      return m_templateMaster->isLinkableInProject();

   } else {
      return ! name().isEmpty() && ! isArtificial() && !isHidden() && ! isAnonymous() && protectionLevelVisible(m_prot) &&
             (! m_isLocal || extractLocal)  && (hasDocumentation() || !hideUndoc) && (! m_isStatic || extractStatic) && !isReference(); 
   }
}

bool ClassDef::isLinkable() const
{
   if (m_templateMaster) {
      return m_templateMaster->isLinkable();
   } else {
      return isReference() || isLinkableInProject();
   }
}

/*! the class is visible in a class diagram, or class hierarchy */
bool ClassDef::isVisibleInHierarchy()
{
   static bool allExternals     = Config::getBool("all-externals");
   static bool hideUndocClasses = Config::getBool("hide-undoc-classes");
   static bool extractStatic    = Config::getBool("extract-static");

   bool retval =  (allExternals || hasNonReferenceSuperClass()); 

   if (retval && ! isAnonymous() && protectionLevelVisible(m_prot)) {

      if ( hasDocumentation() || ! hideUndocClasses || (m_templateMaster && m_templateMaster->hasDocumentation()) || isReference() )   {

         if (! m_isStatic || extractStatic) {
            // true

         } else {
            retval = false;
         }

      } else {
         retval = false;

      }   

   } else {
      retval = false;

   }

   return retval;
}

bool ClassDef::hasDocumentation() const
{
   return Definition::hasDocumentation();
}

// recursive function:
// returns true iff class definition `bcd' represents an (in)direct base
// class of class definition `cd'.

bool ClassDef::isBaseClass(QSharedPointer<ClassDef> bcd, bool followInstances, int level)
{
   bool found = false;
  
   if (level > 256) {
      err("Possible recursive class relation while inside %s and looking for base class %s\n", qPrintable(name()), qPrintable(bcd->name()));
      return false;
   }

   if (baseClasses()) {
      // Beware: trying to optimise the iterator away using ->first() & ->next()
      // causes bug 625531   

      for (auto bcli : *baseClasses() ) {
         QSharedPointer<ClassDef> ccd = bcli->classDef;

         if (! followInstances && ccd->templateMaster()) {
            ccd = ccd->templateMaster();
         }
         
         if (ccd == bcd) {
            found = true;
         } else {
            found = ccd->isBaseClass(bcd, followInstances, level + 1);
         }
      }
   }
   return found;
}

bool ClassDef::isSubClass(QSharedPointer<ClassDef> cd, int level)
{
   bool found = false;

   if (level > 256) {
      err("Possible recursive class relation while inside %s and looking for derived class %s\n", qPrintable(name()), qPrintable(cd->name()));
      return false;
   }

   if (subClasses()) { 

      for (auto bcli : *subClasses()) {

         if (found) {
            break;
         }

         QSharedPointer<ClassDef> ccd = bcli->classDef;

         if (ccd == cd) {
            found = true;
         } else {
            found = ccd->isSubClass(cd, level + 1);
         }
      }
   }
   return found;
}

static bool isStandardFunc(QSharedPointer<MemberDef> md)
{
   return md->name() == "operator=" || // assignment operator
          md->isConstructor() ||     // constructor
          md->isDestructor();        // destructor
}

/*!
 * recusively merges the `all members' lists of a class base
 * with that of this class. Must only be called for classes without
 * subclasses!
 */
void ClassDef::mergeMembers()
{
   if (m_membersMerged) {
      return;
   }

   // static bool optimizeOutputForJava = Config::getBool("optimize-java");
   
   SrcLangExt lang = getLanguage();
   QString sep = getLanguageSpecificSeparator(lang, true);
   int sepLen  = sep.length();

   m_membersMerged = true;
  
   bool inlineInheritedMembers = Config::getBool("inline-inherited-member");

   if (baseClasses()) {
      
      for (auto bcd : *baseClasses() ) {
         QSharedPointer<ClassDef> bClass = bcd->classDef;

         // merge the members in the base class of this inheritance branch first
         bClass->mergeMembers();

         MemberNameInfoSDict *srcMnd  = bClass->memberNameInfoSDict();
         MemberNameInfoSDict *dstMnd  = m_allMemberNameInfoSDict;

         if (srcMnd) {

            for (auto srcMni : *srcMnd) {
             
               QSharedPointer<MemberNameInfo> dstMni;

               if (dstMnd != 0 && (dstMni = dstMnd->find(srcMni->memberName()))) {
                  // a member with that name is already in the class
                  // the member may hide or reimplement the one in the sub class
                  // or there may be another path to the base class that is already
                  // visited via another branch in the class hierarchy

                                
                  for (auto srcMi : *srcMni) {
                     QSharedPointer<MemberDef> srcMd = srcMi.memberDef;

                     bool found   = false;
                     bool ambigue = false;
                     bool hidden  = false;
                    
                     QSharedPointer<ClassDef> srcCd = srcMd->getClassDef();
                   
                     for (auto dstMi : *dstMni) {

                        if (found) {
                           break;
                        }

                        QSharedPointer<MemberDef> dstMd = dstMi.memberDef;

                        if (srcMd != dstMd) { 
                           // different members
                           QSharedPointer<ClassDef> dstCd = dstMd->getClassDef();
                         
                           if (srcCd == dstCd || dstCd->isBaseClass(srcCd, true)) {
                              // member is in the same or a base class
                           
                              ArgumentList *srcAl = srcMd->argumentList();
                              ArgumentList *dstAl = dstMd->argumentList();

                              found = matchArguments2(srcMd->getOuterScope(), srcMd->getFileDef(), srcAl,
                                         dstMd->getOuterScope(), dstMd->getFileDef(), dstAl, true );
                             
                              hidden = hidden  || !found;

                           } else  {
                              // member is in a non base class => multiple inheritance
                              // using the same base class
                           
                              QString scope = dstMi.scopePath.left(dstMi.scopePath.indexOf(sep) + sepLen);

                              if (scope != dstMi.ambiguityResolutionScope.left(scope.length())) {
                                 dstMi.ambiguityResolutionScope.prepend(scope);
                              }

                              ambigue = true;
                           }

                        } else { 
                           // same members do not add if base class is virtual or if scope paths are 
                           // are equal or if base class is an interface (and thus implicitly virtual)
                           
                           if ((srcMi.virt != Normal && dstMi.virt != Normal) ||
                                    bClass->name() + sep + srcMi.scopePath == dstMi.scopePath ||
                                    dstMd->getClassDef()->compoundType() == Interface) {
                              found = true;

                           } else  {
                              // member can be reached via multiple paths in the inheritance tree                                                     

                              QString scope = dstMi.scopePath.left(dstMi.scopePath.indexOf(sep) + sepLen);

                              if (scope != dstMi.ambiguityResolutionScope.left(scope.length())) {
                                 dstMi.ambiguityResolutionScope.prepend(scope);
                              }

                              ambigue = true;
                           }
                        }
                     }
                    
                     // TODO: fix the case where a member is hidden by inheritance
                     //       of a member with the same name but with another prototype,
                     //       while there is more than one path to the member in the
                     //       base class due to multiple inheritance. In this case
                     //       it seems that the member is not reachable by prefixing a
                     //       scope name either (according to my compiler). Currently,
                     //       this case is shown anyway.

                     if (! found && srcMd->protection() != Private && ! srcMd->isFriend()) {
                        Protection prot = srcMd->protection();

                        if (bcd->prot == Protected && prot == Public) {
                           prot = bcd->prot;
                        } else if (bcd->prot == Private) {
                           prot = bcd->prot;
                        }

                        if (inlineInheritedMembers) {
                           if (! isStandardFunc(srcMd)) {                              
                              internalInsertMember(srcMd, prot, false);
                           }
                        }

                        Specifier virt = srcMi.virt;
                        if (srcMi.virt == Normal && bcd->virt != Normal) {
                           virt = bcd->virt;
                        }

                        MemberInfo newMi = MemberInfo(srcMd, prot, virt, true);
                        newMi.scopePath = bClass->name() + sep + srcMi.scopePath;

                        if (ambigue) {
                          
                           QString scope = bClass->name() + sep;
                           if (scope != srcMi.ambiguityResolutionScope.left(scope.length())) {
                              newMi.ambiguityResolutionScope = scope + srcMi.ambiguityResolutionScope;
                           }
                        }

                        if (hidden) {
                           if (srcMi.ambigClass == 0) {
                              newMi.ambigClass = bClass;
                              newMi.ambiguityResolutionScope = bClass->name() + sep;

                           } else {
                              newMi.ambigClass = srcMi.ambigClass;
                              newMi.ambiguityResolutionScope = srcMi.ambigClass->name() + sep;
                           }
                        }

                        dstMni->append(newMi);
                     }
                  }

               } else { 
                  // base class has a member that is not in the sub class
                  // create a deep copy of the list (only the MemberInfo's will be copied, not the actual MemberDef's)

                  QSharedPointer<MemberNameInfo> newMni = QMakeShared<MemberNameInfo>(srcMni->memberName());

                  // copy the member(s) from the base to the sub class
               
                  for (auto mi : *srcMni )   {
                     if (! mi.memberDef->isFriend()) { 
                        // do not inherit friends
                        Protection prot = mi.prot;

                        if (bcd->prot == Protected) {
                           if (prot == Public) {
                              prot = Protected;
                           }

                        } else if (bcd->prot == Private) {
                           prot = Private;
                        }
                       
                        if (mi.prot != Private) {
                           Specifier virt = mi.virt;

                           if (mi.virt == Normal && bcd->virt != Normal) {
                              virt = bcd->virt;
                           }

                           if (inlineInheritedMembers) {
                              if (! isStandardFunc(mi.memberDef)) {                                 
                                 internalInsertMember(mi.memberDef, prot, false);
                              }
                           }
                           
                           MemberInfo newMi = MemberInfo(mi.memberDef, prot, virt, true);
                           newMi.scopePath  = bClass->name() + sep + mi.scopePath;
                           newMi.ambigClass = mi.ambigClass;
                           newMi.ambiguityResolutionScope = mi.ambiguityResolutionScope;
                           newMni->append(newMi);
                        }
                     }
                  }

                  if (dstMnd == 0) {
                     m_allMemberNameInfoSDict = new MemberNameInfoSDict();                     
                     dstMnd = m_allMemberNameInfoSDict;
                  }

                  // add it to the dictionary
                  dstMnd->insert(newMni->memberName(), newMni);
               }
            }
         }
      }
   }   
}

/*! Merges the members of a Objective-C category into this class.
 */
void ClassDef::mergeCategory(QSharedPointer<ClassDef> category)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);
   static bool extractLocalMethods = Config::getBool("extract-local-methods");

   bool makePrivate = category->isLocal();
   // in case extract local methods is not enabled we don't add the methods
   // of the category in case it is defined in the .m file.
   if (makePrivate && !extractLocalMethods) {
      return;
   }
   
   bool isExtension = category->isExtension();

   category->setCategoryOf(self);

   if (isExtension) {
      category->setArtificial(true);

      // copy base classes/protocols from extension
      if (category->baseClasses()) {
         
         for (auto bcd : *category->baseClasses() ) { 
            insertBaseClass(bcd->classDef, bcd->usedName, bcd->prot, bcd->virt, bcd->templSpecifiers);

            // correct bcd->classDef so that they do no longer derive from
            // category, but from this class

            if (bcd->classDef->subClasses()) {
              
               for (auto scd : *bcd->classDef->subClasses() ) {
                  if (scd->classDef == category) {
                     scd->classDef = self;
                  }
               }
            }
         }
      }

   }

   // make methods private for categories defined in the .m file
  
   MemberNameInfoSDict *srcMnd  = category->memberNameInfoSDict();
   MemberNameInfoSDict *dstMnd  = m_allMemberNameInfoSDict;

   if (srcMnd && dstMnd) {     

      for (auto &srcMni : *srcMnd )   {
         QSharedPointer<MemberNameInfo> dstMni = dstMnd->find(srcMni->memberName());

         if (dstMni) { 
            // method is already defined in the class           

            if (! dstMni->isEmpty() && ! srcMni->isEmpty()) {

               MemberInfo &dstMi = dstMni->first();
               MemberInfo &srcMi = srcMni->first();
                      
               combineDeclarationAndDefinition(srcMi.memberDef, dstMi.memberDef);

               dstMi.memberDef->setCategory(category);
               dstMi.memberDef->setCategoryRelation(srcMi.memberDef);
               srcMi.memberDef->setCategoryRelation(dstMi.memberDef);
            }

         } else { 
            // new method name            
            // create a deep copy of the list

            QSharedPointer<ClassDef> self = sharedFrom(this);
            QSharedPointer<MemberNameInfo> newMni (new MemberNameInfo(srcMni->memberName()));

            // copy the member(s) from the category to this class
                         
            for (auto &mi : *srcMni )   {
               Protection prot = mi.prot;

               // deepCopy() is in memberDef.cpp                 
               QSharedPointer<MemberDef> newMd(mi.memberDef->deepCopy());              
              
               // also add the newly created member to the global members list
               if (newMd) {
                  newMd->moveTo(self);

                  MemberInfo newMi = MemberInfo(newMd, prot, mi.virt, mi.inherited);

                  newMi.scopePath  = mi.scopePath;
                  newMi.ambigClass = mi.ambigClass;
                  newMi.ambiguityResolutionScope = mi.ambiguityResolutionScope;

                  newMni->append(newMi);

                  QSharedPointer<MemberName> mn;
                  QString name = newMd->name();

                  if ((mn = Doxy_Globals::memberNameSDict->find(name))) {
                     mn->append(newMd);

                  } else {
                     mn = QSharedPointer<MemberName> (new MemberName(newMd->name()));
                     mn->append(newMd);

                     Doxy_Globals::memberNameSDict->insert(name, mn);
                  }

                  newMd->setCategory(category);
                  newMd->setCategoryRelation(mi.memberDef);
                  mi.memberDef->setCategoryRelation(newMd);

                  if (makePrivate || isExtension) {
                     newMd->makeImplementationDetail();
                  }

                  internalInsertMember(newMd, prot, false);
               }
            }

            // add it to the dictionary
            dstMnd->insert(newMni->memberName(), newMni);
         }
      }
   }
}

void ClassDef::addUsedClass(QSharedPointer<ClassDef> cd, const QString &accessName, Protection prot)
{
   static bool extractPrivate = Config::getBool("extract-private");
   static bool umlLook = Config::getBool("uml-look");

   if (prot == Private && ! extractPrivate) {
      return;
   }
   
   if (m_usesImplClassDict == 0) {
      m_usesImplClassDict = new UsesClassDict();      
   }

   auto ucd = m_usesImplClassDict->find(cd->name());

   if (ucd == m_usesImplClassDict->end()) {      
      m_usesImplClassDict->insert(cd->name(), UsesClassDef(cd));
      
      ucd = m_usesImplClassDict->find(cd->name());
   }

   QString acc = accessName;

   if (umlLook) {
      switch (prot) {
         case Public:
            acc.prepend("+");
            break;

         case Private:
            acc.prepend("-");
            break;

         case Protected:
            acc.prepend("#");
            break;

         case Package:
            acc.prepend("~");
            break;
      }
   }

   ucd->addAccessor(acc);
}

void ClassDef::addUsedByClass(QSharedPointer<ClassDef> cd, const QString &accessName, Protection prot)
{
   static bool extractPrivate = Config::getBool("extract-private");
   static bool umlLook = Config::getBool("uml-look");

   if (prot == Private && ! extractPrivate) {
      return;
   }
   
   if (m_usedByImplClassDict == 0) {
      m_usedByImplClassDict = new UsesClassDict();      
   }


   auto ucd = m_usedByImplClassDict->find(cd->name());

   if (ucd == m_usedByImplClassDict->end()) {      
      m_usedByImplClassDict->insert(cd->name(), UsesClassDef(cd));
      
      ucd = m_usedByImplClassDict->find(cd->name());
   }

   QString acc = accessName;

   if (umlLook) {
      switch (prot) {
         case Public:
            acc.prepend("+");
            break;
         case Private:
            acc.prepend("-");
            break;
         case Protected:
            acc.prepend("#");
            break;
         case Package:
            acc.prepend("~");
            break;
      }
   }
   ucd->addAccessor(acc);
}

QString ClassDef::compoundTypeString() const
{
   if (getLanguage() == SrcLangExt_Fortran) {

      switch (m_compType) {
         case Class:
            return "module";
         case Struct:
            return "type";
         case Union:
            return "union";
         case Interface:
            return "interface";
         case Protocol:
            return "protocol";
         case Category:
            return "category";
         case Exception:
            return "exception";
         default:
            return "unknown";
      }

   } else {

      switch (m_compType) {
         case Class:
            return isJavaEnum() ? "enum" : "class";
         case Struct:
            return "struct";
         case Union:
            return "union";
         case Interface:
            return getLanguage() == SrcLangExt_ObjC ? "class" : "interface";
         case Protocol:
            return "protocol";
         case Category:
            return "category";
         case Exception:
            return "exception";
         case Service:
            return "service";
         case Singleton:
            return "singleton";
         default:
            return "unknown";
      }
   }
}

QString ClassDef::getOutputFileBase() const
{
   static bool inlineGroupedClasses = Config::getBool("inline-grouped-classes");
   static bool inlineSimpleClasses  = Config::getBool("inline-simple-struct");

   if (! Doxy_Globals::generatingXmlOutput) {
      QSharedPointer<Definition> scope;

      if (inlineGroupedClasses && partOfGroups() != 0) {
         // point to the group that embeds this class

         SortedList<QSharedPointer<GroupDef>> *temp = partOfGroups();
         QSharedPointer<GroupDef> def = temp->at(0);

         return def->getOutputFileBase();

      } else if (inlineSimpleClasses && m_isSimple && partOfGroups() != 0) {
         // point to simple struct inside a group

         SortedList<QSharedPointer<GroupDef>> *temp = partOfGroups();
         QSharedPointer<GroupDef> def = temp->at(0);

         return def->getOutputFileBase();

      } else if (inlineSimpleClasses && m_isSimple && (scope = getOuterScope())) {

         if (scope == Doxy_Globals::globalScope && getFileDef() && getFileDef()->isLinkableInProject()) { 
            // simple struct embedded in file
            return getFileDef()->getOutputFileBase();

         } else if (scope->isLinkableInProject()) {       
            // simple struct embedded in other container (namespace/group/class)
            return getOuterScope()->getOutputFileBase();
         }
      }
   }

   if (m_templateMaster) {
      // point to the template of which this class is an instance
      return m_templateMaster->getOutputFileBase();

   } else if (isReference()) {
      // point to the external location
      return m_fileName;

   } else {
      // normal locally defined class
      return convertNameToFile(m_fileName);
   }
}

QString ClassDef::getInstanceOutputFileBase() const
{
   if (isReference()) {
      return m_fileName;

   } else {
      return convertNameToFile(qPrintable(m_fileName));
   }
}

QString ClassDef::getFileBase() const
{
   if (m_templateMaster) {
      return m_templateMaster->getFileBase();
   } else {
      return m_fileName;
   }
}

QString ClassDef::getSourceFileBase() const
{
   if (m_templateMaster) {
      return m_templateMaster->getSourceFileBase();
   } else {
      return Definition::getSourceFileBase();
   }
}

void ClassDef::setGroupDefForAllMembers(QSharedPointer<GroupDef> gd, Grouping::GroupPri_t pri, const QString &fileName, 
                  int startLine, bool hasDocs)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);
   gd->addClass(self);  
   
   if (m_allMemberNameInfoSDict == 0) {
      return;
   }
  
   for (auto mni : *m_allMemberNameInfoSDict) {    

      for (auto mi : *mni) {
         QSharedPointer<MemberDef> md = mi.memberDef;
         md->setGroupDef(gd, pri, fileName, startLine, hasDocs);
         gd->insertMember(md, true);

         QSharedPointer<ClassDef> innerClass = md->getClassDefOfAnonymousType();
         if (innerClass) {
            innerClass->setGroupDefForAllMembers(gd, pri, fileName, startLine, hasDocs);
         }
      }
   }
}

void ClassDef::addInnerCompound(QSharedPointer<Definition> d)
{  
   if (d->definitionType() == Definition::TypeClass)  {   

     QSharedPointer<ClassDef> cd = d.dynamicCast<ClassDef>();
     assert(cd);    
     
     if (m_innerClasses == 0) {
         m_innerClasses = new ClassSDict();
     }
     
     m_innerClasses->insert(d->localName(), cd);
   }
}

QSharedPointer<Definition> ClassDef::findInnerCompound(const QString &name)
{
   QSharedPointer<Definition> result;

   if (name.isEmpty()) {
      return result;
   }

   if (m_innerClasses) {
      result = m_innerClasses->find(name);
   }

   return result;
}

QSharedPointer<ClassDef> ClassDef::insertTemplateInstance(const QString &fileName, int startLine, 
                  int startColumn, const QString &templSpec, bool &freshInstance)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);
   freshInstance = false;

   if (m_templateInstances == 0) {
      m_templateInstances = new QHash<QString, QSharedPointer<ClassDef>>();
   }

   auto templateClass = m_templateInstances->find(templSpec);

   if (templateClass == m_templateInstances->end()) {
      Debug::print(Debug::Classes, 0, "      New template instance class `%s'`%s'\n", csPrintable(name()), csPrintable(templSpec));
      QString tcname = removeRedundantWhiteSpace(localName() + templSpec);

      QSharedPointer<ClassDef> temp = QMakeShared<ClassDef>(fileName, startLine, startColumn, tcname, ClassDef::Class); 

      temp->setTemplateMaster(self);
      temp->setOuterScope(getOuterScope());
      temp->setHidden(isHidden());

      m_templateInstances->insert(templSpec, temp);
      freshInstance = true;

      templateClass = m_templateInstances->find(templSpec);
   }

   return *templateClass;
}

QSharedPointer<ClassDef> ClassDef::getVariableInstance(const QString &templSpec)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);

   if (m_variableInstances == 0) {
      m_variableInstances = new QHash<QString, QSharedPointer<ClassDef>>();
   }

   auto templateClass = m_variableInstances->find(templSpec);

   if (templateClass == m_variableInstances->end()) {
      Debug::print(Debug::Classes, 0, "      New template variable instance class `%s'`%s'\n", qPrintable(name()), qPrintable(templSpec));
      QString tcname = removeRedundantWhiteSpace(name() + templSpec);

      QSharedPointer<ClassDef> temp = QMakeShared<ClassDef>("<code>", 1, 1, tcname, ClassDef::Class, nullptr, "", false);

      temp->addMembersToTemplateInstance(self, templSpec);
      temp->setTemplateMaster(self);

      m_variableInstances->insert(templSpec, temp);
      templateClass = m_variableInstances->find(templSpec);
   }

   return *templateClass;
}

void ClassDef::setTemplateBaseClassNames(QHash<QString, int> *templateNames)
{
   if (templateNames == 0) {
      return;
   }

   if (m_templBaseClassNames == 0) {
      m_templBaseClassNames = new QHash<QString, int>();
   }

   // make a deep copy of the dictionary
   for (auto qdi = templateNames->begin(); qdi != templateNames->end(); ++qdi) {
      if (! m_templBaseClassNames->contains(qdi.key())) {
         m_templBaseClassNames->insert(qdi.key(), qdi.value());
      }
   }
}

QHash<QString, int> *ClassDef::getTemplateBaseClassNames() const
{
   return m_templBaseClassNames;
}

void ClassDef::addMembersToTemplateInstance(QSharedPointer<ClassDef> cd, const QString &templSpec)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);

   if (cd->memberNameInfoSDict() == 0) {
      return;
   }
  
   for (auto mni : *cd->memberNameInfoSDict()) {

      for (auto mi : *mni) {
         ArgumentList *actualArguments = new ArgumentList;
         stringToArgumentList(templSpec, actualArguments);

         QSharedPointer<MemberDef> md = mi.memberDef;
         QSharedPointer<MemberDef> imd(md->createTemplateInstanceMember(cd->templateArguments(), actualArguments));
         delete actualArguments;
         
         imd->setMemberClass(self);
         imd->setTemplateMaster(md);
         imd->setDocumentation(md->documentation(), md->docFile(), md->docLine());
         imd->setBriefDescription(md->briefDescription(), md->briefFile(), md->briefLine());
         imd->setInbodyDocumentation(md->inbodyDocumentation(), md->inbodyFile(), md->inbodyLine());
         imd->setMemberSpecifiers(md->getMemberSpecifiers());
         imd->setMemberGroupId(md->getMemberGroupId());

         insertMember(imd);
        
         QSharedPointer<MemberName> mn = Doxy_Globals::memberNameSDict->find(imd->name());

         if (! mn) {
            mn = QSharedPointer<MemberName> (new MemberName(imd->name()));
            Doxy_Globals::memberNameSDict->insert(imd->name(), mn);
         }

         mn->append(imd);
      }
   }
}

QString ClassDef::getReference() const
{
   if (m_templateMaster) {
      return m_templateMaster->getReference();
   } else {
      return Definition::getReference();
   }
}

bool ClassDef::isReference() const
{
   if (m_templateMaster) {
      return m_templateMaster->isReference();
   } else {
      return Definition::isReference();
   }
}

void ClassDef::getTemplateParameterLists(QList<ArgumentList> &lists) const
{
   QSharedPointer<Definition> d = getOuterScope();

   if (d) {
      if (d->definitionType() == Definition::TypeClass) {
         QSharedPointer<ClassDef> cd = d.dynamicCast<ClassDef>();
         cd->getTemplateParameterLists(lists);
      }
   }

   if ( templateArguments() != 0) {
      lists.append(*templateArguments());
   }
}

QString ClassDef::qualifiedNameWithTemplateParameters(QList<ArgumentList> *actualParams, int *actualParamIndex) const
{
   //static bool optimizeOutputJava = Config::getBool("optimize-java");
   static bool hideScopeNames = Config::getBool("hide-scope-names");

   QString scName;
   QSharedPointer<Definition> d = getOuterScope();

   if (d) {
      if (d->definitionType() == Definition::TypeClass) {
         QSharedPointer<ClassDef> cd = d.dynamicCast<ClassDef>();
         scName = cd->qualifiedNameWithTemplateParameters(actualParams, actualParamIndex);

      } else if (! hideScopeNames) {
         scName = d->qualifiedName();
      }
   }

   SrcLangExt lang = getLanguage();
   QString scopeSeparator = getLanguageSpecificSeparator(lang);

   if (!scName.isEmpty()) {
      scName += scopeSeparator;
   }

   bool isSpecialization = localName().indexOf('<') != -1;

   QString clName = className();

   scName += clName;
   ArgumentList al;

   if (templateArguments()) {
      if (actualParams && *actualParamIndex < (int)actualParams->count()) {
         al = actualParams->at(*actualParamIndex);

         if (! isSpecialization) {
            scName += tempArgListToString(&al, lang);
         }

         (*actualParamIndex)++;

      } else {
         if (! isSpecialization) {
            scName += tempArgListToString(templateArguments(), lang);
         }
      }
   }

   return scName;
}

QString ClassDef::className() const
{
   if (m_className.isEmpty()) {
      return localName();
   } else {
      return m_className;
   }
};

void ClassDef::setClassName(const QString &name)
{
   m_className = name;
}

void ClassDef::addListReferences()
{
   QSharedPointer<ClassDef> self = sharedFrom(this);
   SrcLangExt lang = getLanguage();

   if (! isLinkableInProject()) {
      return;
   }

   QList<ListItemInfo> *xrefItems = xrefListItems();

   addRefItem(xrefItems, qualifiedName(), 
                 lang == SrcLangExt_Fortran ? theTranslator->trType(true, true) : theTranslator->trClass(true, true),
                 getOutputFileBase(), displayName(), 0, self);
   
   if (m_memberGroupSDict) {     
      for (auto mg : *m_memberGroupSDict) {   
         mg->addListReferences(self);
      }
   }
 
   for (auto ml : m_memberLists) {
      if (ml->listType() & MemberListType_detailedLists) {
         ml->addListReferences(self);
      }
   }
}

QSharedPointer<MemberDef> ClassDef::getMemberByName(const QString &name) const
{
   QSharedPointer<const ClassDef> self = sharedFrom(this);
   QSharedPointer<MemberDef> xmd;

   if (m_allMemberNameInfoSDict) {
      QSharedPointer<MemberNameInfo> mni = m_allMemberNameInfoSDict->find(name);

      if (mni) {
         const int maxInheritanceDepth = 100000;
         int mdist = maxInheritanceDepth;
        
         for (auto mi : *mni) {  
            QSharedPointer<ClassDef> mcd = mi.memberDef->getClassDef();

            int m = minClassDistance(self, mcd);
          
            if (m < mdist && mcd->isLinkable()) {
               mdist = m;
               xmd = mi.memberDef;
            }
         }
      }
   }
   
   return xmd;
}

bool ClassDef::isAccessibleMember(QSharedPointer<MemberDef> md)
{
   return md->getClassDef() && isBaseClass(md->getClassDef(), true);
}

QSharedPointer<MemberList> ClassDef::createMemberList(MemberListType lt)
{  
   for (auto ml : m_memberLists) {
      if (ml->listType() == lt) {
         return ml;
      }
   }

   // no memberList was found, create a new memberList
   QSharedPointer<MemberList> ml (new MemberList(lt));
   m_memberLists.append(ml);

   return ml;
}

QSharedPointer<MemberList> ClassDef::getMemberList(MemberListType lt)
{  
   for (auto ml : m_memberLists) {
      if (ml->listType() == lt) {
         return ml;
      }
   }

   return QSharedPointer<MemberList>();
}

void ClassDef::addMemberToList(MemberListType lt, QSharedPointer<MemberDef> md, bool isBrief)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);

   QSharedPointer<MemberList> ml = createMemberList(lt); 

   static bool sortBriefDocs  = Config::getBool("sort-brief-docs");
   static bool sortMemberDocs = Config::getBool("sort-member-docs");

   bool isSorted = false;

   if (isBrief && sortBriefDocs) {
      isSorted = true;
   } else if (! isBrief && sortMemberDocs) {
      isSorted = true;
   }

   if (isSorted) {
      ml->inSort(md);
   } else {
      ml->append(md);
   }

   // for members in the declaration lists we set the section, needed for member grouping
   if ((ml->listType() & MemberListType_detailedLists) == 0) {
      md->setSectionList(self, ml);
   }
}

int ClassDef::countMemberDeclarations(MemberListType lt, QSharedPointer<ClassDef> inheritedFrom,
                                      int lt2, bool invert, bool showAlways, QSet<QSharedPointer<ClassDef>> *visitedClasses)
{
   int count = 0;

   QSharedPointer<MemberList> ml  = getMemberList(lt);
   QSharedPointer<MemberList> ml2 = getMemberList((MemberListType)lt2);

   if (ml) {
      ml->countDecMembers();
      count += ml->numDecMembers();     
   }

   if (ml2) {
      ml2->countDecMembers();
      count += ml2->numDecMembers();
   }

   // also include grouped members that have their own section in the class (see bug 722759)
   if (inheritedFrom && m_memberGroupSDict) {       
      for (auto mg : *m_memberGroupSDict) {  
         count += mg->countGroupedInheritedMembers(lt);

         if (lt2 != 1) {
            count += mg->countGroupedInheritedMembers((MemberListType)lt2);
         }
      }
   }

   static bool inlineInheritedMembers = Config::getBool("inline-inherited-member");

   if (! inlineInheritedMembers) {
      // show inherited members as separate lists
      count += countInheritedDecMembers(lt, inheritedFrom, invert, showAlways, visitedClasses);
   }

   return count;
}

int ClassDef::countInheritedDecMembers(MemberListType lt, QSharedPointer<ClassDef> inheritedFrom, bool invert, 
                  bool showAlways, QSet<QSharedPointer<ClassDef>> *visitedClasses)
{
   int inhCount = 0;
   int count    = countMembersIncludingGrouped(lt, inheritedFrom, false);
   bool process = count > 0;
   
   if ((process ^ invert) || showAlways) {

      if (m_parents) {

         for (auto ibcd : *m_parents) {
            QSharedPointer<ClassDef> icd = ibcd->classDef;
            int lt1;
            int lt2;

            if (icd->isLinkable()) {
               convertProtectionLevel(lt, ibcd->prot, &lt1, &lt2);
               
               if (! visitedClasses->contains(icd)) {
                  // guard for multiple virtual inheritance
                  visitedClasses->insert(icd); 

                  if (lt1 != -1) {
                     inhCount += icd->countMemberDeclarations((MemberListType)lt1, inheritedFrom, lt2, false, true, visitedClasses);
                  }
               }
            }
         }
      }
   }
   return inhCount;
}

void ClassDef::getTitleForMemberListType(MemberListType type, QString &title, QString &subtitle)
{
   SrcLangExt lang = getLanguage();
  
   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Class) ) {  
      if (lde->kind() == LayoutDocEntry::MemberDecl) {
         LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;

         if (lmd->type == type) {
            title = lmd->title(lang);
            subtitle = lmd->subtitle(lang);
            return;
         }
      }
   }

   title    = "";
   subtitle = "";
}

int ClassDef::countAdditionalInheritedMembers()
{
   QSharedPointer<ClassDef> self = sharedFrom(this);
   int totalCount = 0;
 
   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Class)) {

      if (lde->kind() == LayoutDocEntry::MemberDecl) {
         LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;

         if (lmd->type != MemberListType_friends) { 
            // friendship is not inherited                       
            QSet<QSharedPointer<ClassDef>> visited;

            totalCount += countInheritedDecMembers(lmd->type, self, true, false, &visited);            
         }
      }
   }

   return totalCount;
}

void ClassDef::writeAdditionalInheritedMembers(OutputList &ol)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);

  for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Class)) {

      if (lde->kind() == LayoutDocEntry::MemberDecl) {
         LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;

         if (lmd->type != MemberListType_friends) {
            QSet<QSharedPointer<ClassDef>> visited;
            writeInheritedMemberDeclarations(ol, lmd->type, -1, lmd->title(getLanguage()), self, true, false, &visited);
         }
      }
   }
}

int ClassDef::countMembersIncludingGrouped(MemberListType lt, QSharedPointer<ClassDef> inheritedFrom, bool additional)
{
   int count = 0;
   QSharedPointer<MemberList> ml = getMemberList(lt);

   if (ml) {
      count = ml->countInheritableMembers(inheritedFrom);
   }

   if (m_memberGroupSDict) {
      
      for (auto mg : *m_memberGroupSDict) { 
         bool hasOwnSection = ! mg->allMembersInSameSection() || !m_subGrouping; 

         // group is in its own section
         if ((additional && hasOwnSection) || (!additional && !hasOwnSection)) {
            count += mg->countGroupedInheritedMembers(lt);
         }
      }
   }
   
   return count;
}

void ClassDef::writeInheritedMemberDeclarations(OutputList &ol, MemberListType lt, int lt2, const QString &title,
                                                QSharedPointer<ClassDef> inheritedFrom, bool invert, bool showAlways, 
                                                QSet<QSharedPointer<ClassDef>> *visitedClasses)
{
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   int count = countMembersIncludingGrouped(lt, inheritedFrom, false);
   bool process = count > 0;
   
   if ((process ^ invert) || showAlways) {

      if (m_parents) {       

         for (auto ibcd : *m_parents) {
            QSharedPointer<ClassDef> icd = ibcd->classDef;

            if (icd->isLinkable()) {
               int lt1, lt3;
               convertProtectionLevel(lt, ibcd->prot, &lt1, &lt3);

               if (lt2 == -1 && lt3 != -1) {
                  lt2 = lt3;
               }
               
               if (! visitedClasses->contains(icd)) {
                  // guard for multiple virtual inheritance
                  visitedClasses->insert(icd); 

                  if (lt1 != -1) {
                     icd->writeMemberDeclarations(ol, (MemberListType)lt1, title, QString(), false, 
                                                  inheritedFrom, lt2, false, true, visitedClasses);
                  }              
               }
            }
         }
      }
   }
   ol.popGeneratorState();
}

void ClassDef::writeMemberDeclarations(OutputList &ol, MemberListType lt, const QString &title,
                                       const QString &subTitle, bool showInline, QSharedPointer<ClassDef> inheritedFrom, int lt2,
                                       bool invert, bool showAlways, QSet<QSharedPointer<ClassDef>> *visitedClasses)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);

   QSharedPointer<MemberList> ml  = getMemberList(lt);
   QSharedPointer<MemberList> ml2 = getMemberList((MemberListType)lt2);
      
   QString tt = title;
   QString st = subTitle;

   if (ml) {     
      ml->writeDeclarations(ol, self, QSharedPointer<NamespaceDef>(), QSharedPointer<FileDef>(),
                            QSharedPointer<GroupDef>(), tt, st, false, showInline, inheritedFrom, lt);
      tt.resize(0);
      st.resize(0);
   }

   if (ml2) {
      ml2->writeDeclarations(ol, self,  QSharedPointer<NamespaceDef>(), QSharedPointer<FileDef>(),
                             QSharedPointer<GroupDef>(), tt, st, false, showInline, inheritedFrom, lt);
   }

   static bool inlineInheritedMembers = Config::getBool("inline-inherited-member");

   if (! inlineInheritedMembers) { 
      // show inherited members as separate lists

      if (visitedClasses == nullptr) { 
         QSet<QSharedPointer<ClassDef>> visited;

         writeInheritedMemberDeclarations(ol, lt, lt2, title, inheritedFrom ? inheritedFrom : self,
                                       invert, showAlways, &visited);

      } else {
         writeInheritedMemberDeclarations(ol, lt, lt2, title, inheritedFrom ? inheritedFrom : self,
                                       invert, showAlways, visitedClasses);

      }

   } 
}

void ClassDef::addGroupedInheritedMembers(OutputList &ol, MemberListType lt, QSharedPointer<ClassDef> inheritedFrom, 
                                          const QString &inheritId)
{   
   QSharedPointer<ClassDef> self = sharedFrom(this);

   if (m_memberGroupSDict) {     
      for (auto mg : *m_memberGroupSDict) {
         if (! mg->allMembersInSameSection() || ! m_subGrouping) { 
            // group is in its own section
            mg->addGroupedInheritedMembers(ol, self, lt, inheritedFrom, inheritId);
         }
      }
   }
}

void ClassDef::writeMemberDocumentation(OutputList &ol, MemberListType lt, const QString &title, bool showInline)
{ 
   QSharedPointer<ClassDef> self = sharedFrom(this);
   QSharedPointer<MemberList> ml = getMemberList(lt);

   if (ml) {
      ml->writeDocumentation(ol, qPrintable(displayName()), self, title, false, showInline); 
   }
}

void ClassDef::writeSimpleMemberDocumentation(OutputList &ol, MemberListType lt)
{ 
   QSharedPointer<ClassDef> self = sharedFrom(this);
   QSharedPointer<MemberList> ml = getMemberList(lt);

   if (ml) {
      ml->writeSimpleDocumentation(ol, self);
   }
}

void ClassDef::writePlainMemberDeclaration(OutputList &ol, MemberListType lt, bool inGroup,
                                           QSharedPointer<ClassDef> inheritedFrom, const QString &inheritId)
{
   QSharedPointer<ClassDef> self = sharedFrom(this);
   QSharedPointer<MemberList> ml = getMemberList(lt);

   if (ml) {
      ml->setInGroup(inGroup);
      ml->writePlainDeclarations(ol, self, QSharedPointer<NamespaceDef>(), QSharedPointer<FileDef>(),
                                 QSharedPointer<GroupDef>() , inheritedFrom, inheritId);
   }
}

bool ClassDef::isLocal() const
{
   return m_isLocal;
}

ClassSDict *ClassDef::getClassSDict()
{
   return m_innerClasses;
}

ClassDef::CompoundType ClassDef::compoundType() const
{
   return m_compType;
}

SortedList<BaseClassDef *> *ClassDef::baseClasses() const
{
   return m_parents;
}

SortedList<BaseClassDef *> *ClassDef::subClasses() const
{
   return m_inheritedBy;
}

MemberNameInfoSDict *ClassDef::memberNameInfoSDict() const
{
   return m_allMemberNameInfoSDict;
}

Protection ClassDef::protection() const
{
   return m_prot;
}

const ArgumentList *ClassDef::templateArguments() const
{
   if (m_tempArgs.isEmpty())  {
      return 0;
   } else { 
      return &m_tempArgs;
   }
}

ArgumentList *ClassDef::templateArguments()
{
   if (m_tempArgs.isEmpty())  {
      return 0;

   } else { 
      return &m_tempArgs;
   }
}

QSharedPointer<NamespaceDef> ClassDef::getNamespaceDef() const
{
   return m_nspace;
}

QSharedPointer<FileDef> ClassDef::getFileDef() const
{
   return m_fileDef;
}

QHash<QString, QSharedPointer<ClassDef>> *ClassDef::getTemplateInstances() const
{
   return m_templateInstances;
}

QSharedPointer<ClassDef> ClassDef::templateMaster() const
{
   return m_templateMaster;
}

bool ClassDef::isTemplate() const
{
   return ! m_tempArgs.isEmpty();
}

IncludeInfo *ClassDef::includeInfo() const
{
   return m_incInfo;
}

UsesClassDict *ClassDef::usedImplementationClasses() const
{
   return m_usesImplClassDict;
}

UsesClassDict *ClassDef::usedByImplementationClasses() const
{
   return m_usedByImplClassDict;
}

UsesClassDict *ClassDef::usedInterfaceClasses() const
{
   return m_usesIntfClassDict;
}

QHash<QString, QSharedPointer<ConstraintClassDef>> ClassDef::templateTypeConstraints() const
{
   return m_constraintClassDict;
}

bool ClassDef::isTemplateArgument() const
{
   return m_isTemplArg;
}

bool ClassDef::isAbstract() const
{
   return m_isAbstract || (m_spec.spec & Entry::Abstract);
}

bool ClassDef::isFinal() const
{
   return m_spec.m_isFinal;
}

bool ClassDef::isSealed() const
{
   return m_spec.spec & Entry::Sealed;
}

bool ClassDef::isPublished() const
{
   return m_spec.spec & Entry::Published;
}

bool ClassDef::isForwardDeclared() const
{
   return m_spec.spec & Entry::ForwardDecl;
}

bool ClassDef::isObjectiveC() const
{
   return getLanguage() == SrcLangExt_ObjC;
}

bool ClassDef::isCSharp() const
{
   return getLanguage() == SrcLangExt_CSharp;
}

QSharedPointer<ClassDef> ClassDef::categoryOf() const
{
   return m_categoryOf;
}

const QList<QSharedPointer<MemberList>> &ClassDef::getMemberLists() const
{
   return m_memberLists;
}

MemberGroupSDict *ClassDef::getMemberGroupSDict() const
{
   return m_memberGroupSDict;
}

void ClassDef::setNamespace(QSharedPointer<NamespaceDef> nd)
{
   m_nspace = nd;
}

void ClassDef::setFileDef(QSharedPointer<FileDef> fd)
{
   m_fileDef = fd;
}

void ClassDef::setSubGrouping(bool enabled)
{
   m_subGrouping = enabled;
}

void ClassDef::setProtection(Protection p)
{
   m_prot = p;
}

void ClassDef::setIsStatic(bool b)
{
   m_isStatic = b;
}

void ClassDef::setCompoundType(CompoundType t)
{
   m_compType = t;
}

void ClassDef::setTemplateMaster(QSharedPointer<ClassDef> tm)
{
   m_templateMaster = tm;
}

void ClassDef::makeTemplateArgument(bool b)
{
   m_isTemplArg = b;
}

void ClassDef::setCategoryOf(QSharedPointer<ClassDef> cd)
{
   m_categoryOf = cd;
}

void ClassDef::setUsedOnly(bool b)
{
   m_usedOnly = b;
}

bool ClassDef::isUsedOnly() const
{
   return m_usedOnly;
}

bool ClassDef::isSimple() const
{
   return m_isSimple;
}

QSharedPointer<MemberDef> ClassDef::isSmartPointer() const
{
   return m_arrowOperator;
}

void ClassDef::reclassifyMember(QSharedPointer<MemberDef> md, MemberType t)
{
   md->setMemberType(t);
  
   for(auto ml : m_memberLists) {
      ml->removeOne(md);
   }

   insertMember(md);
}

QString ClassDef::anchor() const
{
   QString anc;

   if (isEmbeddedInOuterScope() && !Doxy_Globals::generatingXmlOutput) {
      if (m_templateMaster) {
         // point to the template of which this class is an instance
         anc = m_templateMaster->getOutputFileBase();

      } else if (isReference()) {
         // point to the external location
         anc = m_fileName;

      } else {
         // normal locally defined class
         anc = convertNameToFile(m_fileName);
      }
   }
   return anc;
}

bool ClassDef::isEmbeddedInOuterScope() const
{
   static bool inlineGroupedClasses = Config::getBool("inline-grouped-classes");
   static bool inlineSimpleClasses  = Config::getBool("inline-simple-struct");

   QSharedPointer<Definition> container = getOuterScope();

   bool containerLinkable = container &&
      ( (container == Doxy_Globals::globalScope && getFileDef() && getFileDef()->isLinkableInProject()) || container->isLinkableInProject() );

   // inline because of INLINE_GROUPED_CLASSES=YES ?
   bool b1 = (inlineGroupedClasses && partOfGroups() != 0); // a grouped class

   // inline because of INLINE_SIMPLE_STRUCTS=YES ?
   bool b2 = (inlineSimpleClasses && m_isSimple && (containerLinkable || partOfGroups() != 0   ) );

   return b1 || b2;  // either reason will do
}

const SortedList< QSharedPointer<ClassDef>> *ClassDef::taggedInnerClasses() const
{
   return m_taggedInnerClasses;
}

void ClassDef::addTaggedInnerClass(QSharedPointer<ClassDef> cd)
{
   if (m_taggedInnerClasses == 0) {
      m_taggedInnerClasses = new SortedList<QSharedPointer<ClassDef>>;
   }

   m_taggedInnerClasses->append(cd);
}

QSharedPointer<ClassDef> ClassDef::tagLessReference() const
{
   return m_tagLessRef;
}

void ClassDef::setTagLessReference(QSharedPointer<ClassDef> cd)
{
   m_tagLessRef = cd;
}

void ClassDef::removeMemberFromLists(QSharedPointer<MemberDef> md)
{ 
   for(auto ml : m_memberLists) {
      ml->removeOne(md);
   }
}

bool ClassDef::isJavaEnum() const
{
   return m_isJavaEnum;
}

bool ClassDef::isGeneric() const
{
   return m_isGeneric;
}

void ClassDef::setClassSpecifier(Entry::SpecifierFlags spec)
{
   m_spec = spec;
}

bool ClassDef::isExtension() const
{
   QString n = name();
   int si = n.indexOf('(');
   int ei = n.indexOf(')');

   bool retval = false;

   if (ei > si)  {
      n = n.mid(si + 1, ei - si - 1).trimmed();
      retval = n.isEmpty();
   }

   return retval;
}

const ClassSDict *ClassDef::innerClasses() const
{
   return m_innerClasses;
}

const FileList &ClassDef::usedFiles() const
{
   return m_files;
}

const ArgumentList *ClassDef::typeConstraints() const
{
   return &m_typeConstraints;
}

const ExampleSDict *ClassDef::exampleList() const
{
   return m_exampleSDict;
}

bool ClassDef::subGrouping() const
{
   return m_subGrouping;
}

void ClassDef::setName(const QString &name)
{
   m_isAnonymous = name.indexOf('@') != -1;
   Definition::setName(name);
}

bool ClassDef::isAnonymous() const
{
   return m_isAnonymous;
}
