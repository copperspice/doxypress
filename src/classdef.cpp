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

#include <QFile>
#include <QRegExp>

#include <stdio.h>

#include <arguments.h>
#include <classdef.h>
#include <classlist.h>
#include <config.h>
#include <dot.h>
#include <defargs.h>
#include <debug.h>
#include <docparser.h>
#include <doxygen.h>
#include <diagram.h>
#include <entry.h>
#include <example.h>
#include <filedef.h>
#include <htmlhelp.h>
#include <language.h>
#include <layout.h>
#include <memberlist.h>
#include <membergroup.h>
#include <membername.h>
#include <message.h>
#include <namespacedef.h>
#include <outputlist.h>
#include <searchindex.h>
#include <groupdef.h>
#include <util.h>

//-----------------------------------------------------------------------------

/** Private data associated with a ClassDef object. */
class ClassDefImpl
{
 public:
   ClassDefImpl();
   ~ClassDefImpl();

   void init(const char *defFileName, const char *name, const QByteArray &ctStr, const char *fName);

   /*! file name that forms the base for the output file containing the
    *  class documentation. For compatibility with Qt (e.g. links via tag
    *  files) this name cannot be derived from the class name directly.
    */
   QByteArray fileName;

   /*! Include information about the header file should be included
    *  in the documentation. 0 by default, set by setIncludeFile().
    */
   IncludeInfo *incInfo;

   /*! List of base class (or super-classes) from which this class derives
    *  directly.
    */
   SortedList<BaseClassDef *> *m_parents;

   /*! List of sub-classes that directly derive from this class
    */
   SortedList<BaseClassDef *> *inheritedBy;

   /*! Namespace this class is part of
    *  (this is the inner most namespace in case of nested namespaces)
    */
   NamespaceDef  *nspace;

   /*! File this class is defined in */
   FileDef *fileDef;

   /*! List of all members (including inherited members) */
   MemberNameInfoSDict *allMemberNameInfoSDict;

   /*! Template arguments of this class */
   ArgumentList *tempArgs;

   /*! Type constraints for template parameters */
   ArgumentList *typeConstraints;

   /*! Files that were used for generating the class documentation. */
   FileList files;

   /*! Examples that use this class */
   ExampleSDict *exampleSDict;

   /*! Holds the kind of "class" this is. */
   ClassDef::CompoundType compType;

   /*! The protection level in which this class was found.
    *  Typically Public, but for nested classes this can also be Protected
    *  or Private.
    */
   Protection prot;

   /*! The inner classes contained in this class. Will be 0 if there are
    *  no inner classes.
    */
   ClassSDict *innerClasses;

   /* classes for the collaboration diagram */
   UsesClassDict *usesImplClassDict;
   UsesClassDict *usedByImplClassDict;
   UsesClassDict *usesIntfClassDict;

   /*! Template instances that exists of this class, the key in the
    *  dictionary is the template argument list.
    */
   QHash<QString, ClassDef> *templateInstances;

   /*! Template instances that exists of this class, as defined by variables.
    *  We do NOT want to document these individually. The key in the
    *  dictionary is the template argument list.
    */
   QHash<QString, ClassDef> *variableInstances;

   QHash<QString, int> *templBaseClassNames;

   /*! The class this class is an instance of. */
   ClassDef *templateMaster;

   /*! local class name which could be a typedef'ed alias name. */
   QByteArray className;

   /*! If this class is a Objective-C category, then this points to the
    *  class which is extended.
    */
   ClassDef *categoryOf;

   QList<MemberList *> memberLists;

   /* user defined member groups */
   MemberGroupSDict *memberGroupSDict;

   /*! Is this an abstact class? */
   bool isAbstract;

   /*! Is the class part of an unnamed namespace? */
   bool isStatic;

   /*! true if classes members are merged with those of the base classes. */
   bool membersMerged;

   /*! true if the class is defined in a source file rather than a header file. */
   bool isLocal;

   bool isTemplArg;

   /*! Does this class group its user-grouped members
    *  as a sub-section of the normal (public/protected/..)
    *  groups?
    */
   bool subGrouping;

   /** Reason of existence is a "use" relation */
   bool usedOnly;

   /** Is this a simple (non-nested) C structure? */
   bool isSimple;

   /** Does this class overloaded the -> operator? */
   MemberDef *arrowOperator;

   SortedList<ClassDef *> *taggedInnerClasses;
   ClassDef *tagLessRef;

   /** Does this class represent a Java style enum? */
   bool isJavaEnum;

   bool isGeneric;
   bool isAnonymous;

   uint64_t spec;
};

void ClassDefImpl::init(const char *defFileName, const char *name, const QByteArray &ctStr, const char *fName)
{
   if (fName) {
      fileName = stripExtension(fName);
   } else {
      fileName = ctStr + name;
   }

   exampleSDict = 0;
   m_parents     = 0;
   inheritedBy  = 0;
   allMemberNameInfoSDict = 0;
   incInfo = 0;
   tempArgs = 0;
   typeConstraints = 0;
   prot = Public;
   nspace = 0;
   fileDef = 0;
   usesImplClassDict = 0;
   usedByImplClassDict = 0;
   usesIntfClassDict = 0;
   memberGroupSDict = 0;
   innerClasses = 0;
   subGrouping = Config_getBool("SUBGROUPING");
   templateInstances = 0;
   variableInstances = 0;
   templateMaster = 0;
   templBaseClassNames = 0;
   isAbstract = false;
   isStatic = false;
   isTemplArg = false;
   membersMerged = false;
   categoryOf = 0;
   usedOnly = false;
   isSimple = Config_getBool("INLINE_SIMPLE_STRUCTS");
   arrowOperator = 0;
   taggedInnerClasses = 0;
   tagLessRef = 0;
   spec = 0;

   // we cannot use getLanguage at this point, as setLanguage has not been called.
   SrcLangExt lang = getLanguageFromFileName(defFileName);
   if ((lang == SrcLangExt_Cpp || lang == SrcLangExt_ObjC) &&
         guessSection(defFileName) == Entry::SOURCE_SEC) {
      isLocal = true;
   } else {
      isLocal = false;
   }

   isGeneric = (lang == SrcLangExt_CSharp || lang == SrcLangExt_Java) && QByteArray(name).indexOf('<') != -1;
   isAnonymous = QByteArray(name).indexOf('@') != -1;
}

ClassDefImpl::ClassDefImpl()
{
}

ClassDefImpl::~ClassDefImpl()
{
   delete m_parents;
   delete inheritedBy;
   delete allMemberNameInfoSDict;
   delete exampleSDict;
   delete usesImplClassDict;
   delete usedByImplClassDict;
   delete usesIntfClassDict;
   delete incInfo;
   delete memberGroupSDict;
   delete innerClasses;
   delete templateInstances;
   delete variableInstances;
   delete templBaseClassNames;
   delete tempArgs;
   delete typeConstraints;
   delete taggedInnerClasses;
}

// constructs a new class definition
ClassDef::ClassDef(
   const char *defFileName, int defLine, int defColumn,
   const char *nm, CompoundType ct,
   const char *lref, const char *fName,
   bool isSymbol, bool isJavaEnum)
   : Definition(defFileName, defLine, defColumn, removeRedundantWhiteSpace(nm), 0, 0, isSymbol)
{
   visited = false;
   setReference(lref);
   m_impl = new ClassDefImpl;
   m_impl->compType = ct;
   m_impl->isJavaEnum = isJavaEnum;
   m_impl->init(defFileName, name(), compoundTypeString(), fName);
}

// destroy the class definition
ClassDef::~ClassDef()
{
   delete m_impl;
}

QByteArray ClassDef::getMemberListFileName() const
{
   return convertNameToFile(compoundTypeString() + name() + "-members");
}

QByteArray ClassDef::displayName(bool includeScope) const
{
   //static bool optimizeOutputForJava = Config_getBool("OPTIMIZE_OUTPUT_JAVA");
   SrcLangExt lang = getLanguage();

   QByteArray n;

   if (includeScope) {
      n = qualifiedNameWithTemplateParameters();
   } else {
      n = className();
   }

   QByteArray sep = getLanguageSpecificSeparator(lang);

   if (sep != "::") {
      n = substitute(n, "::", sep);
   }

   if (m_impl->compType == ClassDef::Protocol && n.right(2) == "-p") {
      n = "<" + n.left(n.length() - 2) + ">";
   }

   if (n.indexOf('@') != -1) {
      return removeAnonymousScopes(n);
   } else {
      return n;
   }
}

// inserts a base/super class in the inheritance list
void ClassDef::insertBaseClass(ClassDef *cd, const char *n, Protection p, Specifier s, const char *t)
{   
   if (m_impl->m_parents == 0) {
      m_impl->m_parents = new SortedList<BaseClassDef *>;
   }

   m_impl->m_parents->append(new BaseClassDef(cd, n, p, s, t));
   m_impl->isSimple = false;
}

// inserts a derived/sub class in the inherited-by list
void ClassDef::insertSubClass(ClassDef *cd, Protection p, Specifier s, const char *t)
{
   //printf("*** insert sub class %s into %s\n",cd->name().data(),name().data());

   static bool extractPrivate = Config_getBool("EXTRACT_PRIVATE");
   if (! extractPrivate && cd->protection() == Private) {
      return;
   }

   if (m_impl->inheritedBy == 0) {
      m_impl->inheritedBy = new SortedList<BaseClassDef *>;     
   }

   SortedList<BaseClassDef *> *temp = m_impl->inheritedBy;
   temp->inSort(new BaseClassDef(cd, 0, p, s, t));

   m_impl->isSimple = false;
}

void ClassDef::addMembersToMemberGroup()
{
   for (auto item : m_impl->memberLists ) {
      if ((item->listType()&MemberListType_detailedLists) == 0) {
         ::addMembersToMemberGroup(item, &m_impl->memberGroupSDict, this);
      }
   }

   // add members inside sections to their groups
   if (m_impl->memberGroupSDict) {
  
      for (auto item : *m_impl->memberGroupSDict ) {
         if (item->allMembersInSameSection() && m_impl->subGrouping) {           
            item->addToDeclarationSection();
         }
      }
   }
}

// adds new member definition to the class
void ClassDef::internalInsertMember(MemberDef *md, Protection prot, bool addToAllList)
{   
   if (md->isHidden()) {
      return;
   }

   if (1 /*!isReference()*/) // changed to 1 for showing members of external
      // classes when HAVE_DOT and UML_LOOK are enabled.
   {
      bool isSimple = false;

      /********************************************/
      /* insert member in the declaration section */
      /********************************************/
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
            case MemberType_Signal:       // Qt specific
               addMemberToList(MemberListType_signals, md, true);
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
            case MemberType_Slot:         // Qt specific
               switch (prot) {
                  case Protected:
                  case Package:           // slots in packages are not possible!
                     addMemberToList(MemberListType_proSlots, md, true);
                     break;
                  case Public:
                     addMemberToList(MemberListType_pubSlots, md, true);
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
                  } else { // function
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
               } else { // not static
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
                           isSimple = !md->isFunctionPtr();
                           break;
                        case Private:
                           addMemberToList(MemberListType_priAttribs, md, true);
                           break;
                     }
                  } else if (md->isTypedef() || md->isEnumerate() || md->isEnumValue()) {
                     switch (prot) {
                        case Protected:
                           addMemberToList(MemberListType_proTypes, md, true);
                           break;
                        case Package:
                           addMemberToList(MemberListType_pacTypes, md, true);
                           break;
                        case Public:
                           addMemberToList(MemberListType_pubTypes, md, true);
                           isSimple = QByteArray(md->typeString()).indexOf(")(") == -1;
                           break;
                        case Private:
                           addMemberToList(MemberListType_priTypes, md, true);
                           break;
                     }
                  } else { // member function
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
      if (!isSimple) { // not a simple field -> not a simple struct
         m_impl->isSimple = false;
      }
      //printf("adding %s simple=%d total_simple=%d\n",name().data(),isSimple,m_impl->isSimple);

      /*******************************************************/
      /* insert member in the detailed documentation section */
      /*******************************************************/
      if ((md->isRelated() && protectionLevelVisible(prot)) || md->isFriend()) {
         addMemberToList(MemberListType_relatedMembers, md, false);
      } else {
         switch (md->memberType()) {
            case MemberType_Service: // UNO IDL
               addMemberToList(MemberListType_serviceMembers, md, false);
               break;
            case MemberType_Interface: // UNO IDL
               addMemberToList(MemberListType_interfaceMembers, md, false);
               break;
            case MemberType_Property:
               addMemberToList(MemberListType_propertyMembers, md, false);
               break;
            case MemberType_Event:
               addMemberToList(MemberListType_eventMembers, md, false);
               break;
            case MemberType_Signal: // fall through
            case MemberType_DCOP:
               addMemberToList(MemberListType_functionMembers, md, false);
               break;
            case MemberType_Slot:
               if (protectionLevelVisible(prot)) {
                  addMemberToList(MemberListType_functionMembers, md, false);
               }
               break;
            default: // any of the other members
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
                           MemberList *ml = createMemberList(MemberListType_constructors);
                           ml->append(md);
                        } else {
                           addMemberToList(MemberListType_functionMembers, md, false);
                        }
                        break;
                     case MemberType_Variable:
                        addMemberToList(MemberListType_variableMembers, md, false);
                        break;
                     default:
                        err("Unexpected member type %d found!\n", md->memberType());
                  }
               }
               break;
         }
      }

      /*************************************************/
      /* insert member in the appropriate member group */
      /*************************************************/
      // Note: this must be done AFTER inserting the member in the
      // regular groups
      //addMemberToGroup(md,groupId);

   }

   if (md->virtualness() == Pure) {
      m_impl->isAbstract = true;
   }

   if (md->name() == "operator->") {
      m_impl->arrowOperator = md;
   }

   //::addClassMemberNameToIndex(md);
   if (addToAllList && ! (Config_getBool("HIDE_FRIEND_COMPOUNDS") && md->isFriend() &&  (QByteArray(md->typeString()) == "friend class" || 
            QByteArray(md->typeString()) == "friend struct" || QByteArray(md->typeString()) == "friend union"))) {
      
      MemberInfo mi = MemberInfo((MemberDef *)md, prot, md->virtualness(), false);
      QSharedPointer<MemberNameInfo> mni;

      if (m_impl->allMemberNameInfoSDict == 0) {
         m_impl->allMemberNameInfoSDict = new MemberNameInfoSDict();        
      }

      if ((mni = m_impl->allMemberNameInfoSDict->find(md->name()))) {
         mni->append(mi);

      } else {
         mni = QSharedPointer<MemberNameInfo> (new MemberNameInfo(md->name()));
         mni->append(mi);

         m_impl->allMemberNameInfoSDict->insert(mni->memberName(), mni);
      }
   }
}

void ClassDef::insertMember(MemberDef *md)
{
   internalInsertMember(md, md->protection(), true);
}

// compute the anchors for all members
void ClassDef::computeAnchors()
{   
   for (auto ml : m_impl->memberLists ) {
      if ((ml->listType()&MemberListType_detailedLists) == 0) {
         setAnchors(ml);
      }
   }

   if (m_impl->memberGroupSDict) {
      for (auto mg : *m_impl->memberGroupSDict ) {
         mg->setAnchors();
      }
   }
}

void ClassDef::distributeMemberGroupDocumentation()
{
   if (m_impl->memberGroupSDict) {
      for (auto mg : *m_impl->memberGroupSDict ) {
         mg->distributeMemberGroupDocumentation();
      }
   }
}

void ClassDef::findSectionsInDocumentation()
{
   docFindSections(documentation(), this, 0, docFile());

   if (m_impl->memberGroupSDict) {    
      for (auto item : *m_impl->memberGroupSDict) {
         item->findSectionsInDocumentation();
      }
   }
 
   for (auto item : m_impl->memberLists) {
      if ((item->listType()&MemberListType_detailedLists) == 0) {
         item->findSectionsInDocumentation();
      }
   }
}


// add a file name to the used files set
void ClassDef::insertUsedFile(FileDef *fd)
{
   if (fd == 0) {
      return;
   }

   if (! m_impl->files.contains(fd)) {
      m_impl->files.append(fd);
   }

   if (m_impl->templateInstances) {     
      for (auto item : *m_impl->templateInstances) {
         item.insertUsedFile(fd);
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
         ol.docify(qPrintable(s));

         if (it.hasNext()) {
            ol.docify(", ");
         }
      }

      ol.docify("]");
      ol.endTypewriter();
   }
}

void ClassDef::setIncludeFile(FileDef *fd, const char *includeName, bool local, bool force)
{
   //printf("ClassDef::setIncludeFile(%p,%s,%d,%d)\n",fd,includeName,local,force);
   if (!m_impl->incInfo) {
      m_impl->incInfo = new IncludeInfo;
   }

   if ((includeName && m_impl->incInfo->includeName.isEmpty()) ||
         (fd != 0 && m_impl->incInfo->fileDef == 0)
      ) {
      //printf("Setting file info\n");
      m_impl->incInfo->fileDef     = fd;
      m_impl->incInfo->includeName = includeName;
      m_impl->incInfo->local       = local;
   }
   if (force && includeName) {
      m_impl->incInfo->includeName = includeName;
      m_impl->incInfo->local       = local;
   }
}

// TODO: fix this: a nested template class can have multiple outer templates
//ArgumentList *ClassDef::outerTemplateArguments() const
//{
//  int ti;
//  ClassDef *pcd=0;
//  int pi=0;
//  if (m_impl->tempArgs) return m_impl->tempArgs;
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

static void searchTemplateSpecs(/*in*/  Definition *d,
                                /*out*/ QList<ArgumentList> &result,
                                /*out*/ QByteArray &name,
                                /*in*/  SrcLangExt lang)
{
   if (d->definitionType() == Definition::TypeClass) {

      if (d->getOuterScope()) {
         searchTemplateSpecs(d->getOuterScope(), result, name, lang);
      }

      ClassDef *cd = (ClassDef *)d;

      if (! name.isEmpty()) {
         name += "::";
      }

      QByteArray clName = d->localName();

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

static void writeTemplateSpec(OutputList &ol, Definition *d, const QByteArray &type, SrcLangExt lang)
{
   QList<ArgumentList> specs;
   QByteArray name;

   searchTemplateSpecs(d, specs, name, lang);

   if (specs.count() > 0) { 
      // class has template scope specifiers
      ol.startSubsubsection();    

      for (auto al : specs ) {
         ol.docify("template<");

         QListIterator<Argument> ali(*al);
         Argument *a;

         while ((a = ali.current())) {
            ol.docify(a->type);

            if (!a->name.isEmpty()) {
               ol.docify(" ");
               ol.docify(a->name);
            }

            if (a->defval.length() != 0) {
               ol.docify(" = ");
               ol.docify(a->defval);
            }

            ++ali;
            a = ali.current();

            if (a) {
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
   if (hasBriefDescription()) {
      ol.startParagraph();
      ol.generateDoc(briefFile(), briefLine(), this, 0, briefDescription(), true, false, 0, true, false);
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
   static bool repeatBrief = Config_getBool("REPEAT_BRIEF");

   ol.startTextBlock();

   if (getLanguage() == SrcLangExt_Cpp) {
      writeTemplateSpec(ol, this, compoundTypeString(), getLanguage());
   }

   // repeat brief description
   if (!briefDescription().isEmpty() && repeatBrief) {
      ol.generateDoc(briefFile(), briefLine(), this, 0, briefDescription(), false, false);
   }
   if (! briefDescription().isEmpty() && repeatBrief && ! documentation().isEmpty()) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
      ol.writeString("\n\n");
      ol.popGeneratorState();
   }

   // write documentation
   if (! documentation().isEmpty()) {
      ol.generateDoc(docFile(), docLine(), this, 0, documentation(), true, false);
   }

   // write type constraints
   writeTypeConstraints(ol, this, m_impl->typeConstraints);

   // write examples
   if (hasExamples() && m_impl->exampleSDict) {
      ol.startSimpleSect(BaseOutputDocInterface::Examples, 0, 0, theTranslator->trExamples() + ": ");
      ol.startDescForItem();
      //ol.startParagraph();

      writeExample(ol, m_impl->exampleSDict);

      //ol.endParagraph();
      ol.endDescForItem();
      ol.endSimpleSect();
   }
   //ol.newParagraph();
   writeSourceDef(ol, name());
   ol.endTextBlock();
}

bool ClassDef::hasDetailedDescription() const
{
   static bool repeatBrief = Config_getBool("REPEAT_BRIEF");
   static bool sourceBrowser = Config_getBool("SOURCE_BROWSER");
   return ((!briefDescription().isEmpty() && repeatBrief) ||
           !documentation().isEmpty() ||
           (sourceBrowser && getStartBodyLine() != -1 && getBodyDef()));
}

// write the detailed description for this class
void ClassDef::writeDetailedDescription(OutputList &ol, const QByteArray &/*pageType*/, bool exampleFlag,
                                        const QByteArray &title, const QByteArray &anchor)
{
   if (hasDetailedDescription() || exampleFlag) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
      ol.writeRuler();
      ol.popGeneratorState();

      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);
      ol.writeAnchor(0, anchor.isEmpty() ? QByteArray("details") : anchor);
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

QByteArray ClassDef::generatedFromFiles() const
{
   QByteArray result;
   SrcLangExt lang = getLanguage();
   if (lang == SrcLangExt_Fortran) {
      result = theTranslator->trGeneratedFromFilesFortran(
                  getLanguage() == SrcLangExt_ObjC && m_impl->compType == Interface ? Class : m_impl->compType,
                  m_impl->files.count() == 1);
   } else if (isJavaEnum()) {
      result = theTranslator->trEnumGeneratedFromFiles(m_impl->files.count() == 1);
   } else if (m_impl->compType == Service) {
      result = theTranslator->trServiceGeneratedFromFiles(m_impl->files.count() == 1);
   } else if (m_impl->compType == Singleton) {
      result = theTranslator->trSingletonGeneratedFromFiles(m_impl->files.count() == 1);
   } else {
      result = theTranslator->trGeneratedFromFiles(
                  getLanguage() == SrcLangExt_ObjC && m_impl->compType == Interface ? Class : m_impl->compType,
                  m_impl->files.count() == 1);
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
   
   for (auto fd : m_impl->files) {
      if (first) {
         first = false;
         ol.startItemList();
      }

      ol.startItemListItem();
      QByteArray path = fd->getPath();
      if (Config_getBool("FULL_PATH_NAMES")) {
         ol.docify(stripFromPath(path));
      }

      QByteArray fname = fd->name();
      if (!fd->getVersion().isEmpty()) { // append version if available
         fname += " (" + fd->getVersion() + ")";
      }

      // for HTML
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);
      if (fd->generateSourceFile()) {
         ol.writeObjectLink(0, fd->getSourceFileBase(), 0, fname);
      } else if (fd->isLinkable()) {
         ol.writeObjectLink(fd->getReference(), fd->getOutputFileBase(), 0,
                            fname);
      } else {
         ol.docify(fname);
      }
      ol.popGeneratorState();

      // for other output formats
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Html);
      if (fd->isLinkable()) {
         ol.writeObjectLink(fd->getReference(), fd->getOutputFileBase(), 0,
                            fname);
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

   if (m_impl->inheritedBy) {
      for (auto ibcd : *m_impl->inheritedBy) {
         ClassDef *icd = ibcd->classDef;

         if ( icd->isVisibleInHierarchy()) {
            count++;
         }
      }
   }

   if (m_impl->m_parents) {     
      for (auto ibcd : *m_impl->m_parents) {
         ClassDef *icd = ibcd->classDef;

         if ( icd->isVisibleInHierarchy()) {
            count++;
         }
      }
   }

   return count;
}

void ClassDef::writeInheritanceGraph(OutputList &ol)
{
   // count direct inheritance relations
   const int count = countInheritanceNodes();

   bool renderDiagram = false;
   if (Config_getBool("HAVE_DOT") && (Config_getBool("CLASS_DIAGRAMS") || Config_getBool("CLASS_GRAPH"))) {
      // write class diagram using dot   
      DotClassGraph inheritanceGraph(this, DotNode::Inheritance);

      if (!inheritanceGraph.isTrivial() && !inheritanceGraph.isTooBig()) {
         ol.pushGeneratorState();
         ol.disable(OutputGenerator::Man);
         ol.startDotGraph();
         ol.parseText(theTranslator->trClassDiagram(displayName()));
         ol.endDotGraph(inheritanceGraph);
         ol.popGeneratorState();
         renderDiagram = true;
      }

   } else if (Config_getBool("CLASS_DIAGRAMS") && count > 0) {
      // write class diagram using build-in generator
   
      ClassDiagram diagram(this); // create a diagram of this class.
      ol.startClassDiagram();
      ol.disable(OutputGenerator::Man);
      ol.parseText(theTranslator->trClassDiagram(displayName()));
      ol.enable(OutputGenerator::Man);
      ol.endClassDiagram(diagram, getOutputFileBase(), displayName());
      renderDiagram = true;
   }

   if (renderDiagram) // if we already show the inheritance relations graphically,
      // then hide the text version
   {
      ol.disableAllBut(OutputGenerator::Man);
   }

   if (m_impl->m_parents && m_impl->m_parents->count() > 0) {
      ol.startParagraph();
      //parseText(ol,theTranslator->trInherits()+" ");

      QByteArray inheritLine = theTranslator->trInheritsList(m_impl->m_parents->count());
      QRegExp marker("@[0-9]+");
      int index = 0, newIndex, matchLen;

      // now replace all markers in inheritLine with links to the classes      
      while ((newIndex = marker.indexIn(inheritLine, index)) != -1) {

         matchLen = marker.matchedLength();

         ol.parseText(inheritLine.mid(index, newIndex - index));
         bool ok;

         uint entryIndex = inheritLine.mid(newIndex + 1, matchLen - 1).toUInt(&ok);
         BaseClassDef *bcd = m_impl->m_parents->at(entryIndex);

         if (ok && bcd) {
            ClassDef *cd = bcd->classDef;

            // use the class name but with the template arguments as given
            // in the inheritance relation
            QByteArray displayName = insertTemplateSpecifierInScope( cd->displayName(), bcd->templSpecifiers);

            if (cd->isLinkable()) {
               ol.writeObjectLink(cd->getReference(), cd->getOutputFileBase(), cd->anchor(), displayName);
            } else {
               ol.docify(displayName);
            }

         } else {
            err("invalid marker %d in inherits list!\n", entryIndex);
         }

         index = newIndex + matchLen;
      }
      ol.parseText(inheritLine.right(inheritLine.length() - index));
      ol.endParagraph();
   }

   // write subclasses
   if (m_impl->inheritedBy && m_impl->inheritedBy->count() > 0) {
      ol.startParagraph();
      QByteArray inheritLine = theTranslator->trInheritedByList(m_impl->inheritedBy->count());
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

         BaseClassDef *bcd = m_impl->inheritedBy->at(entryIndex);

         if (ok && bcd) {
            ClassDef *cd = bcd->classDef;

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
   if (Config_getBool("HAVE_DOT") /*&& Config_getBool("COLLABORATION_GRAPH")*/) {
      DotClassGraph usageImplGraph(this, DotNode::Collaboration);

      if (!usageImplGraph.isTrivial()) {
         ol.pushGeneratorState();
         ol.disable(OutputGenerator::Man);
         ol.startDotGraph();
         ol.parseText(theTranslator->trCollaborationDiagram(displayName()));
         ol.endDotGraph(usageImplGraph);
         ol.popGeneratorState();
      }
   }
}

QByteArray ClassDef::includeStatement() const
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
   if (m_impl->incInfo /*&& Config_getBool("SHOW_INCLUDE_FILES")*/) {

      QByteArray nm = m_impl->incInfo->includeName.isEmpty() ? (m_impl->incInfo->fileDef 
            ? m_impl->incInfo->fileDef->docName().data() : "" ) : m_impl->incInfo->includeName.data();

      if (!nm.isEmpty()) {
         ol.startParagraph();
         ol.startTypewriter();
         ol.docify(includeStatement());

         SrcLangExt lang = getLanguage();
         bool isIDLorJava = lang == SrcLangExt_IDL || lang == SrcLangExt_Java;

         if (m_impl->incInfo->local || isIDLorJava) {
            ol.docify("\"");
         } else {
            ol.docify("<");
         }

         ol.pushGeneratorState();
         ol.disable(OutputGenerator::Html);
         ol.docify(nm);
         ol.disableAllBut(OutputGenerator::Html);
         ol.enable(OutputGenerator::Html);

         if (m_impl->incInfo->fileDef) {
            ol.writeObjectLink(0, m_impl->incInfo->fileDef->includeName(), 0, nm);
         } else {
            ol.docify(nm);
         }
         ol.popGeneratorState();
         if (m_impl->incInfo->local || isIDLorJava) {
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
   if (m_impl->allMemberNameInfoSDict && !Config_getBool("OPTIMIZE_OUTPUT_FOR_C") ) {
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

   if (m_impl->memberGroupSDict) {          
      for (auto mg : *m_impl->memberGroupSDict)  {
         if (!mg->allMembersInSameSection() || !m_impl->subGrouping) { // group is in its own section
            mg->writeDeclarations(ol, this, 0, 0, 0, showInline);

         } else { 
            // add this group to the corresponding member section
            
         }
      }
   }
}

void ClassDef::writeNestedClasses(OutputList &ol, const QByteArray &title)
{
   // nested classes
   if (m_impl->innerClasses) {
      m_impl->innerClasses->writeDeclaration(ol, 0, title, true);
   }
}

void ClassDef::writeInlineClasses(OutputList &ol)
{
   if (m_impl->innerClasses) {
      m_impl->innerClasses->writeDocumentation(ol, this);
   }
}

void ClassDef::startMemberDocumentation(OutputList &ol)
{
   //printf("%s: ClassDef::startMemberDocumentation()\n",name().data());
   if (Config_getBool("SEPARATE_MEMBER_PAGES")) {
      ol.disable(OutputGenerator::Html);
      Doxygen::suppressDocWarnings = true;
   }
}

void ClassDef::endMemberDocumentation(OutputList &ol)
{
   //printf("%s: ClassDef::endMemberDocumentation()\n",name().data());
   if (Config_getBool("SEPARATE_MEMBER_PAGES")) {
      ol.enable(OutputGenerator::Html);
      Doxygen::suppressDocWarnings = false;
   }
}

void ClassDef::startMemberDeclarations(OutputList &ol)
{
   //printf("%s: ClassDef::startMemberDeclarations()\n",name().data());
   ol.startMemberSections();
}

void ClassDef::endMemberDeclarations(OutputList &ol)
{
   //printf("%s: ClassDef::endMemberDeclarations()\n",name().data());
   static bool inlineInheritedMembers = Config_getBool("INLINE_INHERITED_MEMB");

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
   ol.parseText(theTranslator->trGeneratedAutomatically(Config_getString("PROJECT_NAME")));
   ol.popGeneratorState();
}


void ClassDef::writeSummaryLinks(OutputList &ol)
{
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
   
   bool first = true;
   SrcLangExt lang = getLanguage();  
   
   for (auto lde : LayoutDocManager::instance().docEntries(LayoutDocManager::Class)) {

      if (lde->kind() == LayoutDocEntry::ClassNestedClasses && m_impl->innerClasses  && m_impl->innerClasses->declVisible()) {
         LayoutDocEntrySection *ls = (LayoutDocEntrySection *)lde;
         ol.writeSummaryLink(0, "nested-classes", ls->title(lang), first);
         first = false;

      } else if (lde->kind() == LayoutDocEntry::ClassAllMembersLink && m_impl->allMemberNameInfoSDict && 
                 !Config_getBool("OPTIMIZE_OUTPUT_FOR_C") ) {

         ol.writeSummaryLink(getMemberListFileName(), "all-members-list", theTranslator->trListOfAllMembers(), first);
         first = false;

      } else if (lde->kind() == LayoutDocEntry::MemberDecl) {
         LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
         MemberList *ml = getMemberList(lmd->type);

         if (ml && ml->declVisible()) {
            ol.writeSummaryLink(0, MemberList::listTypeAsString(ml->listType()), lmd->title(lang), first);
            first = false;
         }
      }
   }
  
   if (!first) {
      ol.writeString("  </div>\n");
   }

   ol.popGeneratorState();
}

void ClassDef::writeTagFile(FTextStream &tagFile)
{
   if (!isLinkableInProject()) {
      return;
   }

   tagFile << "  <compound kind=\"" << compoundTypeString();
   tagFile << "\"";

   if (isObjectiveC()) {
      tagFile << " objc=\"yes\"";
   }

   tagFile << ">" << endl;
   tagFile << "    <name>" << convertToXML(name()) << "</name>" << endl;
   tagFile << "    <filename>" << convertToXML(getOutputFileBase()) << Doxygen::htmlFileExtension << "</filename>" << endl;

   if (!anchor().isEmpty()) {
      tagFile << "    <anchor>" << convertToXML(anchor()) << "</anchor>" << endl;
   }

   QByteArray idStr = id();
   if (!idStr.isEmpty()) {
      tagFile << "    <clangid>" << convertToXML(idStr) << "</clangid>" << endl;
   }

   if (m_impl->tempArgs) {
      for (auto a : *m_impl->tempArgs) {
         tagFile << "    <templarg>" << convertToXML(a.name) << "</templarg>" << endl;
      }
   }

   if (m_impl->m_parents) {
     
      for (auto ibcd : *m_impl->m_parents) {
         ClassDef *cd = ibcd->classDef;

         if (cd && cd->isLinkable()) {
            if (!Config_getString("GENERATE_TAGFILE").isEmpty()) {
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
            if (m_impl->innerClasses) {
              
               for (auto innerCd : *m_impl->innerClasses) {

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
            MemberList *ml = getMemberList(lmd->type);
            if (ml) {
               ml->writeTagFile(tagFile);
            }
         }
         break;

         case LayoutDocEntry::MemberGroups: {
            if (m_impl->memberGroupSDict) {
               for (auto mg : *m_impl->memberGroupSDict) {
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
   bool isSimple = m_impl->isSimple;

   ol.addIndexItem(name(), 0);
 
   // part 1: anchor and title
   QByteArray s = compoundTypeString() + " " + name();

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
            if (!isSimple) {
               writeMemberDeclarations(ol, lmd->type, lmd->title(lang), lmd->subtitle(lang), true);
            }
         }
         break;
         case LayoutDocEntry::MemberGroups:
            if (!isSimple) {
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

void ClassDef::writeMoreLink(OutputList &ol, const QByteArray &anchor)
{
   // TODO: clean up this mess by moving it to
   // the output generators...
   static bool pdfHyperlinks = Config_getBool("PDF_HYPERLINKS");
   static bool rtfHyperlinks = Config_getBool("RTF_HYPERLINKS");
   static bool usePDFLatex   = Config_getBool("USE_PDFLATEX");

   // HTML only
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
   ol.docify(" ");
   ol.startTextLink(getOutputFileBase(),
                    anchor.isEmpty() ? QByteArray("details") : anchor);
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
   static bool extractPrivate      = Config_getBool("EXTRACT_PRIVATE");
   static bool hideUndocClasses = Config_getBool("HIDE_UNDOC_CLASSES");
   static bool extractLocalClasses = Config_getBool("EXTRACT_LOCAL_CLASSES");
   bool linkable = isLinkable();
   return (!isAnonymous() && !isExtension() &&
           (protection() !=::Private || extractPrivate) &&
           (linkable || (!hideUndocClasses && (!isLocal() || extractLocalClasses)))
          );
}

void ClassDef::writeDeclarationLink(OutputList &ol, bool &found, const char *header, bool localNames)
{
   //static bool fortranOpt = Config_getBool("OPTIMIZE_FOR_FORTRAN");
   //static bool vhdlOpt    = Config_getBool("OPTIMIZE_OUTPUT_VHDL");
   SrcLangExt lang = getLanguage();

   if (visibleInParentsDeclList()) {

      if (! found) { // first class
         ol.startMemberHeader("nested-classes");

         if (header) {
            ol.parseText(header);

         } else {
            ol.parseText(lang == SrcLangExt_Fortran ?
                         theTranslator->trDataTypes() :
                         theTranslator->trCompounds());
         }

         ol.endMemberHeader();
         ol.startMemberList();
         found = true;
      }

      ol.startMemberDeclaration();
      ol.startMemberItem(anchor(), false);
      QByteArray ctype = compoundTypeString();
      QByteArray cname = displayName(!localNames);

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
      if (! briefDescription().isEmpty() && Config_getBool("BRIEF_MEMBER_DESC")) {
         DocRoot *rootNode = validatingParseDoc(briefFile(), briefLine(), this, 0,
                                                briefDescription(), false, false, 0, true, false);

         if (rootNode && !rootNode->isEmpty()) {
            ol.startMemberDescription(anchor());
            ol.writeDoc(rootNode, this, 0);

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

void ClassDef::writeDocumentationContents(OutputList &ol, const QByteArray & /*pageTitle*/)
{
   ol.startContents();

   QByteArray pageType = " ";
   pageType += compoundTypeString();
   toupper(pageType.at(1));

   Doxygen::indexList->addIndexItem(this, 0);

   if (Doxygen::searchIndex) {
      Doxygen::searchIndex->setCurrentDoc(this, anchor(), false);
      Doxygen::searchIndex->addWord(localName(), true);
   }

   bool exampleFlag = hasExamples();

   //---------------------------------------- start flexible part -------------------------------

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
            err("Internal inconsistency: member %d should not be part of "
                "LayoutDocManager::Class entry list\n", lde->kind());
            break;
      }
   }

   ol.endContents();
}

QByteArray ClassDef::title() const
{
   QByteArray pageTitle;
   SrcLangExt lang = getLanguage();

   if (lang == SrcLangExt_Fortran) {
      pageTitle = theTranslator->trCompoundReferenceFortran(displayName(),
                  m_impl->compType,
                  m_impl->tempArgs != 0);         

   } else if (isJavaEnum()) {
      pageTitle = theTranslator->trEnumReference(displayName());

   } else if (m_impl->compType == Service) {
      pageTitle = theTranslator->trServiceReference(displayName());

   } else if (m_impl->compType == Singleton) {
      pageTitle = theTranslator->trSingletonReference(displayName());

   } else {
      if (Config_getBool("HIDE_COMPOUND_REFERENCE")) {
         pageTitle = displayName();
      } else {
         pageTitle = theTranslator->trCompoundReference(displayName(),
                     m_impl->compType == Interface && getLanguage() == SrcLangExt_ObjC ? Class : m_impl->compType,
                     m_impl->tempArgs != 0);
      }
   }
   return pageTitle;
}

// write all documentation for this class
void ClassDef::writeDocumentation(OutputList &ol)
{
   static bool generateTreeView = Config_getBool("GENERATE_TREEVIEW");
   //static bool fortranOpt = Config_getBool("OPTIMIZE_FOR_FORTRAN");
   //static bool vhdlOpt    = Config_getBool("OPTIMIZE_OUTPUT_VHDL");
   QByteArray pageTitle = title();

   startFile(ol, getOutputFileBase(), name(), pageTitle, HLI_ClassVisible, !generateTreeView);
   if (!generateTreeView) {
      if (getOuterScope() != Doxygen::globalScope) {
         writeNavigationPath(ol);
      }
      ol.endQuickIndices();
   }

   startTitle(ol, getOutputFileBase(), this);
   ol.parseText(pageTitle);
   addClassAttributes(ol);
   addGroupListToTitle(ol, this);
   endTitle(ol, getOutputFileBase(), displayName());
   writeDocumentationContents(ol, pageTitle);

   endFileWithNavPath(this, ol);

   if (Config_getBool("SEPARATE_MEMBER_PAGES")) {
      writeMemberPages(ol);
   }
}

void ClassDef::writeMemberPages(OutputList &ol)
{
   ///////////////////////////////////////////////////////////////////////////
   //// Member definitions on separate pages
   ///////////////////////////////////////////////////////////////////////////

   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
   
   for (auto ml : m_impl->memberLists) {
      ml->countDocMembers();

      if (ml->numDocMembers() > 0 && (ml->listType()&MemberListType_detailedLists)) {
         ml->writeDocumentationPage(ol, displayName(), this);
      }
   }

   ol.popGeneratorState();
}

void ClassDef::writeQuickMemberLinks(OutputList &ol, MemberDef *currentMd) const
{
   static bool createSubDirs = Config_getBool("CREATE_SUBDIRS");

   ol.writeString("      <div class=\"navtab\">\n");
   ol.writeString("        <table>\n");

   if (m_impl->allMemberNameInfoSDict) {
      MemberNameInfoSDict::Iterator mnili(*m_impl->allMemberNameInfoSDict);
      MemberNameInfo *mni;

      for (; (mni = mnili.current()); ++mnili) {
         MemberNameInfoIterator mnii(*mni);
         MemberInfo *mi;

         for (mnii.toFirst(); (mi = mnii.current()); ++mnii) {
            MemberDef *md = mi->memberDef;

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
                  ol.writeString(md->getOutputFileBase() + Doxygen::htmlFileExtension + "#" + md->anchor());
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
   if (m_impl->innerClasses) {
      ClassSDict::Iterator cli(*m_impl->innerClasses);
      ClassDef *innerCd;
      for (cli.toFirst(); (innerCd = cli.current()); ++cli) {
         if (innerCd->isLinkableInProject() && innerCd->templateMaster() == 0 &&
               protectionLevelVisible(innerCd->protection()) &&
               !innerCd->isEmbeddedInOuterScope()
            ) {
            msg("Generating docs for nested compound %s...\n", qPrint(innerCd->name()));
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
   static bool cOpt    = Config_getBool("OPTIMIZE_OUTPUT_FOR_C");
   //static bool vhdlOpt = Config_getBool("OPTIMIZE_OUTPUT_VHDL");
   static bool generateTreeView = Config_getBool("GENERATE_TREEVIEW");
   if (m_impl->allMemberNameInfoSDict == 0 || cOpt) {
      return;
   }
   // only for HTML
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);

   QByteArray memListFile = getMemberListFileName();
   startFile(ol, memListFile, memListFile, theTranslator->trMemberList(),
             HLI_ClassVisible, !generateTreeView, getOutputFileBase());
   if (!generateTreeView) {
      if (getOuterScope() != Doxygen::globalScope) {
         writeNavigationPath(ol);
      }
      ol.endQuickIndices();
   }
   startTitle(ol, 0);
   ol.parseText(displayName() + " " + theTranslator->trMemberList());
   endTitle(ol, 0, 0);
   ol.startContents();
   ol.startParagraph();
   ol.parseText(theTranslator->trThisIsTheListOfAllMembers());
   ol.writeObjectLink(getReference(), getOutputFileBase(), anchor(), displayName());
   ol.parseText(theTranslator->trIncludingInheritedMembers());
   ol.endParagraph();

   //ol.startItemList();
   ol.writeString("<table class=\"directory\">\n");

   int idx = 0;
   //MemberNameInfo *mni=m_impl->allMemberNameInfoList->first();
   MemberNameInfoSDict::Iterator mnii(*m_impl->allMemberNameInfoSDict);
   MemberNameInfo *mni;

   for (mnii.toFirst(); (mni = mnii.current()); ++mnii) {
      MemberNameInfoIterator it(*mni);
      MemberInfo *mi;

      for (; (mi = it.current()); ++it) {
         MemberDef *md = mi->memberDef;
         ClassDef  *cd = md->getClassDef();
         Protection prot = mi->prot;
         Specifier virt = md->virtualness();

         //printf("%s: Member %s of class %s md->protection()=%d mi->prot=%d prot=%d inherited=%d\n",
         //    name().data(),md->name().data(),cd->name().data(),md->protection(),mi->prot,prot,mi->inherited);

         if (cd && !md->name().isEmpty() && md->name()[0] != '@') {
            bool memberWritten = false;
            if (cd->isLinkable() && md->isLinkable())
               // create a link to the documentation
            {
               QByteArray name = mi->ambiguityResolutionScope + md->name();
               //ol.writeListItem();
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
                  ol.writeObjectLink(md->getReference(),
                                     md->getOutputFileBase(),
                                     md->anchor(), md->name());
               } else {
                  //Definition *bd = md->getGroupDef();
                  //if (bd==0) bd=cd;
                  ol.writeObjectLink(md->getReference(),
                                     md->getOutputFileBase(),
                                     md->anchor(), name);

                  if ( md->isFunction() || md->isSignal() || md->isSlot() ||
                        (md->isFriend() && md->argsString())) {
                     ol.docify(md->argsString());
                  } else if (md->isEnumerate()) {
                     ol.parseText(" " + theTranslator->trEnumName());
                  } else if (md->isEnumValue()) {
                     ol.parseText(" " + theTranslator->trEnumValue());
                  } else if (md->isTypedef()) {
                     ol.docify(" typedef");
                  } else if (md->isFriend() && !qstrcmp(md->typeString(), "friend class")) {
                     ol.docify(" class");
                  }
                  //ol.writeString("\n");
               }
               ol.writeString("</td>");
               memberWritten = true;
            } else if (!cd->isArtificial() &&
                       !Config_getBool("HIDE_UNDOC_MEMBERS") &&
                       (protectionLevelVisible(md->protection()) || md->isFriend())
                      ) // no documentation,
               // generate link to the class instead.
            {
               //ol.writeListItem();
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
               if (!md->isObjCMethod()) {
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
                     cd->displayName());
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
               ol.writeObjectLink(cd->getReference(),
                                  cd->getOutputFileBase(),
                                  cd->anchor(),
                                  md->category() ?
                                  md->category()->displayName() :
                                  cd->displayName());
               ol.writeString("</td>");
               ol.writeString("<td class=\"entry\">");
            }

            SrcLangExt lang = md->getLanguage();


            bool x1 = getLanguage() == SrcLangExt_IDL && (md->isOptional() || md->isAttribute() || md->isUNOProperty());

            bool x2 = (prot != Public || (virt != Normal && getLanguage() != SrcLangExt_ObjC) ||
                               md->isFriend()  || md->isRelated() || md->isExplicit() ||
                               md->isMutable() || (md->isInline() && Config_getBool("INLINE_INFO")) ||
                               md->isSignal()  || md->isSlot() ||  x1  || md->isStatic()  );

            if ( x2 && memberWritten) {

               ol.writeString("<span class=\"mlabel\">");
               QStringList sl;

               if (md->isFriend()) {
                  sl.append("friend");

               } else if (md->isRelated()) {
                  sl.append("related");

               } else {
                  if (Config_getBool("INLINE_INFO") && md->isInline()) {
                     sl.append("inline");
                  }
                  if (md->isExplicit()) {
                     sl.append("explicit");
                  }
                  if (md->isMutable()) {
                     sl.append("mutable");
                  }
                  if (prot == Protected) {
                     sl.append("protected");
                  } else if (prot == Private) {
                     sl.append("private");
                  } else if (prot == Package) {
                     sl.append("package");
                  }
                  if (virt == Virtual && getLanguage() != SrcLangExt_ObjC) {
                     sl.append("virtual");
                  } else if (virt == Pure) {
                     sl.append("pure virtual");
                  }
                  if (md->isStatic()) {
                     sl.append("static");
                  }
                  if (md->isSignal()) {
                     sl.append("signal");
                  }
                  if (md->isSlot()) {
                     sl.append("slot");
                  }
                  // this is the extra member page
                  if (md->isOptional()) {
                     sl.append("optional");
                  }
                  if (md->isAttribute()) {
                     sl.append("attribute");
                  }
                  if (md->isUNOProperty()) {
                     sl.append("property");
                  }
                  if (md->isReadonly()) {
                     sl.append("readonly");
                  }
                  if (md->isBound()) {
                     sl.append("bound");
                  }
                  if (md->isRemovable()) {
                     sl.append("removable");
                  }
                  if (md->isConstrained()) {
                     sl.append("constrained");
                  }
                  if (md->isTransient()) {
                     sl.append("transient");
                  }
                  if (md->isMaybeVoid()) {
                     sl.append("maybevoid");
                  }
                  if (md->isMaybeDefault()) {
                     sl.append("maybedefault");
                  }
                  if (md->isMaybeAmbiguous()) {
                     sl.append("maybeambiguous");
                  }
               }

               auto nextItem = sl.begin();
              
               for (auto s : sl ) { 
                  ++nextItem;

                  ol.docify(s);                  

                  if (nextItem != sl.end()) {
                     ol.writeString("</span><span class=\"mlabel\">");
                  }
               }

               ol.writeString("</span>");
            }

            if (memberWritten) {
               ol.writeString("</td>");
               ol.writeString("</tr>\n");
            }
         }
      }
   }
   //ol.endItemList();

   ol.writeString("</table>");

   endFile(ol);
   ol.popGeneratorState();
}


// add a reference to an example
bool ClassDef::addExample(const char *anchor, const char *nameStr,
                          const char *file)
{
   if (m_impl->exampleSDict == 0) {
      m_impl->exampleSDict = new ExampleSDict;      
   }

   if (! m_impl->exampleSDict->find(nameStr)) {
      Example *e = new Example;

      e->anchor = anchor;
      e->name   = nameStr;
      e->file   = file;

      m_impl->exampleSDict->insert(nameStr, e);

      return true;
   }
   return false;
}

// returns true if this class is used in an example
bool ClassDef::hasExamples() const
{
   bool result = false;
   if (m_impl->exampleSDict) {
      result = m_impl->exampleSDict->count() > 0;
   }
   return result;
}


void ClassDef::setTemplateArguments(ArgumentList *al)
{
   if (al == 0) {
      return;
   }

   if (!m_impl->tempArgs) {
      delete m_impl->tempArgs;   // delete old list if needed
   }

   m_impl->tempArgs = new ArgumentList;
 
   for (auto a : *al) {
      m_impl->tempArgs->append(new Argument(*a));
   }
}

void ClassDef::setTypeConstraints(ArgumentList *al)
{
   if (al == 0) {
      return;
   }

   if (!m_impl->typeConstraints) {
      delete m_impl->typeConstraints;
   }

   m_impl->typeConstraints = new ArgumentList;
   ArgumentListIterator ali(*al);
   Argument *a;

   for (; (a = ali.current()); ++ali) {
      m_impl->typeConstraints->append(new Argument(*a));
   }
}

/*! Returns \c true iff this class or a class inheriting from this class
 *  is \e not defined in an external tag file.
 */
bool ClassDef::hasNonReferenceSuperClass()
{
   bool found = !isReference() && isLinkableInProject() && !isHidden();

   if (found) {
      return true; // we're done if this class is not a reference
   }

   if (m_impl->inheritedBy) {
      QListIterator<BaseClassDef *> bcli(*m_impl->inheritedBy);

      for ( ; bcli.current() && !found ; ++bcli ) { // for each super class
         ClassDef *bcd = bcli.current()->classDef;
         // recurse into the super class branch
         found = found || bcd->hasNonReferenceSuperClass();

         if (!found) {
            // look for template instances that might have non-reference super classes
            QHash<QString, ClassDef> *cil = bcd->getTemplateInstances();

            if (cil) {
               QHashIterator<QSting, ClassDef> tidi(*cil);

               for ( ; tidi.current() && !found ; ++tidi) { // for each template instance
                  // recurse into the template instance branch
                  found = found || tidi.current()->hasNonReferenceSuperClass();
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
void ClassDef::writeDeclaration(OutputList &ol, MemberDef *md, bool inGroup,
                                ClassDef *inheritedFrom, const char *inheritId)
{
   //printf("ClassName=`%s' inGroup=%d\n",name().data(),inGroup);

   ol.docify(compoundTypeString());
   QByteArray cn = displayName(false);
   if (!cn.isEmpty()) {
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
   if (m_impl->memberGroupSDict) {
      MemberGroupSDict::Iterator mgli(*m_impl->memberGroupSDict);
      MemberGroup *mg;
      for (; (mg = mgli.current()); ++mgli) {
         mg->setInGroup(inGroup);
         mg->writePlainDeclarations(ol, this, 0, 0, 0, inheritedFrom, inheritId);
      }
   }

   QListIterator<LayoutDocEntry> eli(
      LayoutDocManager::instance().docEntries(LayoutDocManager::Class));
   LayoutDocEntry *lde;

   for (eli.toFirst(); (lde = eli.current()); ++eli) {
      if (lde->kind() == LayoutDocEntry::MemberDecl) {
         LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
         writePlainMemberDeclaration(ol, lmd->type, inGroup, inheritedFrom, inheritId);
      }
   }
}

/*! a link to this class is possible within this project */
bool ClassDef::isLinkableInProject() const
{
   static bool extractLocal   = Config_getBool("EXTRACT_LOCAL_CLASSES");
   static bool extractStatic  = Config_getBool("EXTRACT_STATIC");
   static bool hideUndoc      = Config_getBool("HIDE_UNDOC_CLASSES");

   if (m_impl->templateMaster) {
      return m_impl->templateMaster->isLinkableInProject();
   } else {
      return !name().isEmpty() &&                    /* has a name */
             !isArtificial() && !isHidden() &&            /* not hidden */
             !isAnonymous() &&                            /* not anonymous */
             protectionLevelVisible(m_impl->prot)      && /* private/internal */
             (!m_impl->isLocal      || extractLocal)   && /* local */
             (hasDocumentation()    || !hideUndoc)     && /* documented */
             (!m_impl->isStatic     || extractStatic)  && /* static */
             !isReference();                              /* not an external reference */
   }
}

bool ClassDef::isLinkable() const
{
   if (m_impl->templateMaster) {
      return m_impl->templateMaster->isLinkable();
   } else {
      return isLinkableInProject() || isReference();
   }
}


/*! the class is visible in a class diagram, or class hierarchy */
bool ClassDef::isVisibleInHierarchy()
{
   static bool allExternals     = Config_getBool("ALLEXTERNALS");
   static bool hideUndocClasses = Config_getBool("HIDE_UNDOC_CLASSES");
   static bool extractStatic    = Config_getBool("EXTRACT_STATIC");

   return // show all classes or a subclass is visible
      (allExternals || hasNonReferenceSuperClass()) &&
      // and not an anonymous compound
      !isAnonymous() &&
      // not an artificially introduced class
      /*!isArtificial() &&*/  // 1.8.2: allowed these to appear
      // and not privately inherited
      protectionLevelVisible(m_impl->prot) &&
      // documented or shown anyway or documentation is external
      (hasDocumentation() ||
       !hideUndocClasses ||
       (m_impl->templateMaster && m_impl->templateMaster->hasDocumentation()) ||
       isReference()
      ) &&
      // is not part of an unnamed namespace or shown anyway
      (!m_impl->isStatic || extractStatic);
}

bool ClassDef::hasDocumentation() const
{
   return Definition::hasDocumentation();
}

//----------------------------------------------------------------------
// recursive function:
// returns true iff class definition `bcd' represents an (in)direct base
// class of class definition `cd'.

bool ClassDef::isBaseClass(ClassDef *bcd, bool followInstances, int level)
{
   bool found = false;
   //printf("isBaseClass(cd=%s) looking for %s\n",name().data(),bcd->name().data());
   if (level > 256) {
      err("Possible recursive class relation while inside %s and looking for base class %s\n", qPrint(name()), qPrint(bcd->name()));
      return false;
   }
   if (baseClasses()) {
      // Beware: trying to optimise the iterator away using ->first() & ->next()
      // causes bug 625531

      QListIterator<BaseClassDef *> bcli(*baseClasses());

      for ( ; bcli.current() && !found ; ++bcli) {
         ClassDef *ccd = bcli.current()->classDef;

         if (!followInstances && ccd->templateMaster()) {
            ccd = ccd->templateMaster();
         }
         //printf("isBaseClass() baseclass %s\n",ccd->name().data());
         if (ccd == bcd) {
            found = true;
         } else {
            found = ccd->isBaseClass(bcd, followInstances, level + 1);
         }
      }
   }
   return found;
}

//----------------------------------------------------------------------

bool ClassDef::isSubClass(ClassDef *cd, int level)
{
   bool found = false;
   if (level > 256) {
      err("Possible recursive class relation while inside %s and looking for derived class %s\n", qPrint(name()), qPrint(cd->name()));
      return false;
   }
   if (subClasses()) {
      QListIterator<BaseClassDef *> bcli(*subClasses());

      for ( ; bcli.current() && !found ; ++bcli) {
         ClassDef *ccd = bcli.current()->classDef;
         if (ccd == cd) {
            found = true;
         } else {
            found = ccd->isSubClass(cd, level + 1);
         }
      }
   }
   return found;
}

//----------------------------------------------------------------------------

static bool isStandardFunc(MemberDef *md)
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
   if (m_impl->membersMerged) {
      return;
   }

   //static bool optimizeOutputForJava = Config_getBool("OPTIMIZE_OUTPUT_JAVA");
   //static bool vhdlOpt = Config_getBool("OPTIMIZE_OUTPUT_VHDL");
   SrcLangExt lang = getLanguage();
   QByteArray sep = getLanguageSpecificSeparator(lang, true);
   int sepLen = sep.length();

   m_impl->membersMerged = true;
   //printf("  mergeMembers for %s\n",name().data());
   bool inlineInheritedMembers = Config_getBool("INLINE_INHERITED_MEMB" );

   if (baseClasses()) {
      //printf("  => has base classes!\n");
      QListIterator<BaseClassDef *> bcli(*baseClasses());
      BaseClassDef *bcd;

      for ( ; (bcd = bcli.current()) ; ++bcli ) {
         ClassDef *bClass = bcd->classDef;

         // merge the members in the base class of this inheritance branch first
         bClass->mergeMembers();

         MemberNameInfoSDict *srcMnd  = bClass->memberNameInfoSDict();
         MemberNameInfoSDict *dstMnd  = m_impl->allMemberNameInfoSDict;

         if (srcMnd) {
            MemberNameInfoSDict::Iterator srcMnili(*srcMnd);
            MemberNameInfo *srcMni;
            for ( ; (srcMni = srcMnili.current()) ; ++srcMnili) {
               //printf("    Base member name %s\n",srcMni->memberName());
               MemberNameInfo *dstMni;
               if (dstMnd != 0 && (dstMni = dstMnd->find(srcMni->memberName())))
                  // a member with that name is already in the class.
                  // the member may hide or reimplement the one in the sub class
                  // or there may be another path to the base class that is already
                  // visited via another branch in the class hierarchy.
               {
                  MemberNameInfoIterator srcMnii(*srcMni);
                  MemberInfo *srcMi;
                  for ( ; (srcMi = srcMnii.current()) ; ++srcMnii ) {
                     MemberDef *srcMd = srcMi->memberDef;
                     bool found = false;
                     bool ambigue = false;
                     bool hidden = false;

                     MemberNameInfoIterator dstMnii(*dstMni);
                     MemberInfo *dstMi;
                     ClassDef *srcCd = srcMd->getClassDef();

                     for ( ; (dstMi = dstMnii.current()) && !found; ++dstMnii ) {
                        MemberDef *dstMd = dstMi->memberDef;
                        if (srcMd != dstMd) { // different members
                           ClassDef *dstCd = dstMd->getClassDef();
                           //printf("  Is %s a base class of %s?\n",srcCd->name().data(),dstCd->name().data());
                           if (srcCd == dstCd || dstCd->isBaseClass(srcCd, true))
                              // member is in the same or a base class
                           {
                              ArgumentList *srcAl = srcMd->argumentList();
                              ArgumentList *dstAl = dstMd->argumentList();
                              found = matchArguments2(
                                         srcMd->getOuterScope(), srcMd->getFileDef(), srcAl,
                                         dstMd->getOuterScope(), dstMd->getFileDef(), dstAl,
                                         true
                                      );
                              //printf("  Yes, matching (%s<->%s): %d\n",
                              //    argListToString(srcMd->argumentList()).data(),
                              //    argListToString(dstMd->argumentList()).data(),
                              //    found);
                              hidden = hidden  || !found;

                           } else // member is in a non base class => multiple inheritance
                              // using the same base class

                           {
                              //printf("$$ Existing member %s %s add scope %s\n",
                              //    dstMi->ambiguityResolutionScope.data(),
                              //    dstMd->name().data(),
                              //    dstMi->scopePath.left(dstMi->scopePath.find("::")+2).data());

                              QByteArray scope = dstMi->scopePath.left(dstMi->scopePath.indexOf(sep) + sepLen);
                              if (scope != dstMi->ambiguityResolutionScope.left(scope.length())) {
                                 dstMi->ambiguityResolutionScope.prepend(scope);
                              }
                              ambigue = true;
                           }
                        } else { // same members
                           // do not add if base class is virtual or
                           // if scope paths are equal or
                           // if base class is an interface (and thus implicitly virtual).
                           //printf("same member found srcMi->virt=%d dstMi->virt=%d\n",srcMi->virt,dstMi->virt);
                           if ((srcMi->virt != Normal && dstMi->virt != Normal) ||
                                 bClass->name() + sep + srcMi->scopePath == dstMi->scopePath ||
                                 dstMd->getClassDef()->compoundType() == Interface
                              ) {
                              found = true;
                           } else // member can be reached via multiple paths in the
                              // inheritance tree
                           {
                              //printf("$$ Existing member %s %s add scope %s\n",
                              //    dstMi->ambiguityResolutionScope.data(),
                              //    dstMd->name().data(),
                              //    dstMi->scopePath.left(dstMi->scopePath.find("::")+2).data());

                              QByteArray scope = dstMi->scopePath.left(dstMi->scopePath.indexOf(sep) + sepLen);
                              if (scope != dstMi->ambiguityResolutionScope.left(scope.length())) {
                                 dstMi->ambiguityResolutionScope.prepend(scope);
                              }
                              ambigue = true;
                           }
                        }
                     }
                     //printf("member %s::%s hidden %d ambigue %d srcMi->ambigClass=%p\n",
                     //    srcCd->name().data(),srcMd->name().data(),hidden,ambigue,srcMi->ambigClass);

                     // TODO: fix the case where a member is hidden by inheritance
                     //       of a member with the same name but with another prototype,
                     //       while there is more than one path to the member in the
                     //       base class due to multiple inheritance. In this case
                     //       it seems that the member is not reachable by prefixing a
                     //       scope name either (according to my compiler). Currently,
                     //       this case is shown anyway.
                     if (!found && srcMd->protection() != Private && !srcMd->isFriend()) {
                        Protection prot = srcMd->protection();
                        if (bcd->prot == Protected && prot == Public) {
                           prot = bcd->prot;
                        } else if (bcd->prot == Private) {
                           prot = bcd->prot;
                        }

                        if (inlineInheritedMembers) {
                           if (!isStandardFunc(srcMd)) {
                              //printf("    insertMember `%s'\n",srcMd->name().data());
                              internalInsertMember(srcMd, prot, false);
                           }
                        }

                        Specifier virt = srcMi->virt;
                        if (srcMi->virt == Normal && bcd->virt != Normal) {
                           virt = bcd->virt;
                        }

                        MemberInfo *newMi = new MemberInfo(srcMd, prot, virt, true);
                        newMi->scopePath = bClass->name() + sep + srcMi->scopePath;
                        if (ambigue) {
                           //printf("$$ New member %s %s add scope %s::\n",
                           //     srcMi->ambiguityResolutionScope.data(),
                           //     srcMd->name().data(),
                           //     bClass->name().data());

                           QByteArray scope = bClass->name() + sep;
                           if (scope != srcMi->ambiguityResolutionScope.left(scope.length())) {
                              newMi->ambiguityResolutionScope = scope + srcMi->ambiguityResolutionScope;
                           }
                        }
                        if (hidden) {
                           if (srcMi->ambigClass == 0) {
                              newMi->ambigClass = bClass;
                              newMi->ambiguityResolutionScope = bClass->name() + sep;

                           } else {
                              newMi->ambigClass = srcMi->ambigClass;
                              newMi->ambiguityResolutionScope = srcMi->ambigClass->name() + sep;
                           }
                        }

                        dstMni->append(newMi);
                     }
                  }

               } else { // base class has a member that is not in the sub class => copy
                  // create a deep copy of the list (only the MemberInfo's will be
                  // copied, not the actual MemberDef's)
                  MemberNameInfo *newMni = 0;
                  newMni = new MemberNameInfo(srcMni->memberName());

                  // copy the member(s) from the base to the sub class
                  MemberNameInfoIterator mnii(*srcMni);
                  MemberInfo *mi;
                  for (; (mi = mnii.current()); ++mnii) {
                     if (!mi->memberDef->isFriend()) { // don't inherit friends
                        Protection prot = mi->prot;
                        if (bcd->prot == Protected) {
                           if (prot == Public) {
                              prot = Protected;
                           }
                        } else if (bcd->prot == Private) {
                           prot = Private;
                        }
                        //printf("%s::%s: prot=%d bcd->prot=%d result=%d\n",
                        //    name().data(),mi->memberDef->name().data(),mi->prot,
                        //    bcd->prot,prot);

                        if (mi->prot != Private) {
                           Specifier virt = mi->virt;
                           if (mi->virt == Normal && bcd->virt != Normal) {
                              virt = bcd->virt;
                           }

                           if (inlineInheritedMembers) {
                              if (!isStandardFunc(mi->memberDef)) {
                                 //printf("    insertMember `%s'\n",mi->memberDef->name().data());
                                 internalInsertMember(mi->memberDef, prot, false);
                              }
                           }
                           //printf("Adding!\n");
                           MemberInfo *newMi = new MemberInfo(mi->memberDef, prot, virt, true);
                           newMi->scopePath = bClass->name() + sep + mi->scopePath;
                           newMi->ambigClass = mi->ambigClass;
                           newMi->ambiguityResolutionScope = mi->ambiguityResolutionScope;
                           newMni->append(newMi);
                        }
                     }
                  }

                  if (dstMnd == 0) {
                     m_impl->allMemberNameInfoSDict = new MemberNameInfoSDict();                     
                     dstMnd = m_impl->allMemberNameInfoSDict;
                  }

                  // add it to the dictionary
                  dstMnd->append(newMni->memberName(), newMni);
               }
            }
         }
      }
   }
   //printf("  end mergeMembers\n");
}

//----------------------------------------------------------------------------

/*! Merges the members of a Objective-C category into this class.
 */
void ClassDef::mergeCategory(ClassDef *category)
{
   static bool extractLocalMethods = Config_getBool("EXTRACT_LOCAL_METHODS");
   bool makePrivate = category->isLocal();
   // in case extract local methods is not enabled we don't add the methods
   // of the category in case it is defined in the .m file.
   if (makePrivate && !extractLocalMethods) {
      return;
   }
   bool isExtension = category->isExtension();

   category->setCategoryOf(this);
   if (isExtension) {
      category->setArtificial(true);

      // copy base classes/protocols from extension
      if (category->baseClasses()) {
         QListIterator<BaseClassDef *> bcli(*category->baseClasses());
         BaseClassDef *bcd;

         for ( ; (bcd = bcli.current()) ; ++bcli ) {
            insertBaseClass(bcd->classDef, bcd->usedName, bcd->prot, bcd->virt, bcd->templSpecifiers);
            // correct bcd->classDef so that they do no longer derive from
            // category, but from this class!
            if (bcd->classDef->subClasses()) {
               QListIterator<BaseClassDef *> scli(*bcd->classDef->subClasses());
               BaseClassDef *scd;

               for ( ; (scd = scli.current()) ; ++scli ) {
                  if (scd->classDef == category) {
                     scd->classDef = this;
                  }
               }
            }
         }
      }

   }
   // make methods private for categories defined in the .m file
   //printf("%s::mergeCategory makePrivate=%d\n",name().data(),makePrivate);

   MemberNameInfoSDict *srcMnd  = category->memberNameInfoSDict();
   MemberNameInfoSDict *dstMnd  = m_impl->allMemberNameInfoSDict;

   if (srcMnd && dstMnd) {
      MemberNameInfoSDict::Iterator srcMnili(*srcMnd);
      MemberNameInfo *srcMni;
      for ( ; (srcMni = srcMnili.current()) ; ++srcMnili) {
         MemberNameInfo *dstMni = dstMnd->find(srcMni->memberName());
         if (dstMni) { // method is already defined in the class
            //printf("Existing member %s\n",srcMni->memberName());
            MemberInfo *dstMi = dstMni->getFirst();
            MemberInfo *srcMi = srcMni->getFirst();
            //if (dstMi)
            //{
            //  Protection prot = dstMi->prot;
            //  if (makePrivate || isExtension)
            //  {
            //    prot = Private;
            //    removeMemberFromLists(dstMi->memberDef);
            //    internalInsertMember(dstMi->memberDef,prot,false);
            //  }
            //}
            if (srcMi && dstMi) {
               combineDeclarationAndDefinition(srcMi->memberDef, dstMi->memberDef);
               dstMi->memberDef->setCategory(category);
               dstMi->memberDef->setCategoryRelation(srcMi->memberDef);
               srcMi->memberDef->setCategoryRelation(dstMi->memberDef);
            }
         } else { // new method name
            //printf("New member %s\n",srcMni->memberName());
            // create a deep copy of the list
            MemberNameInfo *newMni = 0;
            newMni = new MemberNameInfo(srcMni->memberName());

            // copy the member(s) from the category to this class
            MemberNameInfoIterator mnii(*srcMni);
            MemberInfo *mi;
            for (; (mi = mnii.current()); ++mnii) {
              
               Protection prot = mi->prot;
              
               MemberDef *newMd = mi->memberDef->deepCopy();
               
               newMd->moveTo(this);

               MemberInfo *newMi = new MemberInfo(newMd, prot, mi->virt, mi->inherited);
               newMi->scopePath = mi->scopePath;
               newMi->ambigClass = mi->ambigClass;
               newMi->ambiguityResolutionScope = mi->ambiguityResolutionScope;
               newMni->append(newMi);

               // also add the newly created member to the global members list
               if (newMd) {
                  MemberName *mn;
                  QByteArray name = newMd->name();
                  if ((mn = Doxygen::memberNameSDict->find(name))) {
                     mn->append(newMd);
                  } else {
                     mn = new MemberName(newMd->name());
                     mn->append(newMd);
                     Doxygen::memberNameSDict->append(name, mn);
                  }
               }

               newMd->setCategory(category);
               newMd->setCategoryRelation(mi->memberDef);
               mi->memberDef->setCategoryRelation(newMd);
               if (makePrivate || isExtension) {
                  newMd->makeImplementationDetail();
               }
               internalInsertMember(newMd, prot, false);
            }

            // add it to the dictionary
            dstMnd->append(newMni->memberName(), newMni);
         }
      }
   }
}

//----------------------------------------------------------------------------

void ClassDef::addUsedClass(ClassDef *cd, const char *accessName, Protection prot)
{
   static bool extractPrivate = Config_getBool("EXTRACT_PRIVATE");
   static bool umlLook = Config_getBool("UML_LOOK");
   if (prot == Private && !extractPrivate) {
      return;
   }

   //printf("%s::addUsedClass(%s,%s)\n",name().data(),cd->name().data(),accessName);
   if (m_impl->usesImplClassDict == 0) {
      m_impl->usesImplClassDict = new UsesClassDict();      
   }

   UsesClassDef *ucd = m_impl->usesImplClassDict->find(cd->name());
   if (ucd == 0) {
      ucd = new UsesClassDef(cd);
      m_impl->usesImplClassDict->insert(cd->name(), ucd);

      //printf("Adding used class %s to class %s via accessor %s\n",
      //    cd->name().data(),name().data(),accessName);
   }

   QByteArray acc = accessName;
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

void ClassDef::addUsedByClass(ClassDef *cd, const char *accessName,
                              Protection prot)
{
   static bool extractPrivate = Config_getBool("EXTRACT_PRIVATE");
   static bool umlLook = Config_getBool("UML_LOOK");

   if (prot == Private && !extractPrivate) {
      return;
   }

   //printf("%s::addUsedByClass(%s,%s)\n",name().data(),cd->name().data(),accessName);
   if (m_impl->usedByImplClassDict == 0) {
      m_impl->usedByImplClassDict = new UsesClassDict();      
   }

   UsesClassDef *ucd = m_impl->usedByImplClassDict->find(cd->name());

   if (ucd == 0) {
      ucd = new UsesClassDef(cd);
      m_impl->usedByImplClassDict->insert(cd->name(), ucd);
      //printf("Adding used by class %s to class %s\n",
      //    cd->name().data(),name().data());
   }

   QByteArray acc = accessName;
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

QByteArray ClassDef::compoundTypeString() const
{
   if (getLanguage() == SrcLangExt_Fortran) {
      switch (m_impl->compType) {
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
      switch (m_impl->compType) {
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

QByteArray ClassDef::getOutputFileBase() const
{
   static bool inlineGroupedClasses = Config_getBool("INLINE_GROUPED_CLASSES");
   static bool inlineSimpleClasses  = Config_getBool("INLINE_SIMPLE_STRUCTS");

   if (! Doxygen::generatingXmlOutput) {
      Definition *scope = 0;

      if (inlineGroupedClasses && partOfGroups() != 0) {
         // point to the group that embeds this class

         SortedList<GroupDef *> *temp = partOfGroups();
         GroupDef def = temp->at(0);

         return def.getOutputFileBase();

      } else if (inlineSimpleClasses && m_impl->isSimple && partOfGroups() != 0) {
         // point to simple struct inside a group

         SortedList<GroupDef *> *temp = partOfGroups();
         GroupDef def = temp->at(0);

         return def.getOutputFileBase();

      } else if (inlineSimpleClasses && m_impl->isSimple && (scope = getOuterScope())) {

         if (scope == Doxygen::globalScope && getFileDef() && getFileDef()->isLinkableInProject()) { // simple struct embedded in file
            return getFileDef()->getOutputFileBase();

         } else if (scope->isLinkableInProject()) { // simple struct embedded in other container (namespace/group/class)
            return getOuterScope()->getOutputFileBase();
         }
      }
   }

   if (m_impl->templateMaster) {
      // point to the template of which this class is an instance
      return m_impl->templateMaster->getOutputFileBase();
   } else if (isReference()) {
      // point to the external location
      return m_impl->fileName;
   } else {
      // normal locally defined class
      return convertNameToFile(m_impl->fileName);
   }
}

QByteArray ClassDef::getInstanceOutputFileBase() const
{
   if (isReference()) {
      return m_impl->fileName;
   } else {
      return convertNameToFile(m_impl->fileName);
   }
}

QByteArray ClassDef::getFileBase() const
{
   if (m_impl->templateMaster) {
      return m_impl->templateMaster->getFileBase();
   } else {
      return m_impl->fileName;
   }
}

QByteArray ClassDef::getSourceFileBase() const
{
   if (m_impl->templateMaster) {
      return m_impl->templateMaster->getSourceFileBase();
   } else {
      return Definition::getSourceFileBase();
   }
}

void ClassDef::setGroupDefForAllMembers(GroupDef *gd, Grouping::GroupPri_t pri, const QByteArray &fileName, int startLine, bool hasDocs)
{
   gd->addClass(this);
   //printf("ClassDef::setGroupDefForAllMembers(%s)\n",gd->name().data());
   if (m_impl->allMemberNameInfoSDict == 0) {
      return;
   }
   MemberNameInfoSDict::Iterator mnili(*m_impl->allMemberNameInfoSDict);
   MemberNameInfo *mni;
   for (; (mni = mnili.current()); ++mnili) {
      MemberNameInfoIterator mnii(*mni);
      MemberInfo *mi;
      for (mnii.toFirst(); (mi = mnii.current()); ++mnii) {
         MemberDef *md = mi->memberDef;
         md->setGroupDef(gd, pri, fileName, startLine, hasDocs);
         gd->insertMember(md, true);
         ClassDef *innerClass = md->getClassDefOfAnonymousType();
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
     
     if (m_impl->innerClasses == 0) {
         m_impl->innerClasses = new ClassSDict();
     }
     
     m_impl->innerClasses->insert(d->localName(), cd);
   }
}

Definition *ClassDef::findInnerCompound(const char *name)
{
   Definition *result = 0;

   if (name == 0) {
      return 0;
   }

   if (m_impl->innerClasses) {
      result = m_impl->innerClasses->find(name);
   }

   return result;
}

ClassDef *ClassDef::insertTemplateInstance(const QByteArray &fileName,
      int startLine, int startColumn, const QByteArray &templSpec, bool &freshInstance)
{
   freshInstance = false;
   if (m_impl->templateInstances == 0) {
      m_impl->templateInstances = new QHash<QString, ClassDef>();
   }

   ClassDef *templateClass = m_impl->templateInstances->find(templSpec);

   if (templateClass == 0) {
      Debug::print(Debug::Classes, 0, "      New template instance class `%s'`%s'\n", name().data(), templSpec.data());
      QByteArray tcname = removeRedundantWhiteSpace(localName() + templSpec);

      templateClass = new ClassDef(fileName, startLine, startColumn, tcname, ClassDef::Class);

      templateClass->setTemplateMaster(this);
      templateClass->setOuterScope(getOuterScope());
      templateClass->setHidden(isHidden());
      m_impl->templateInstances->insert(templSpec, templateClass);
      freshInstance = true;
   }
   return templateClass;
}

ClassDef *ClassDef::getVariableInstance(const char *templSpec)
{
   if (m_impl->variableInstances == 0) {
      m_impl->variableInstances = new QHash<QString, ClassDef>();
   }

   ClassDef *templateClass = m_impl->variableInstances->find(templSpec);

   if (templateClass == 0) {
      Debug::print(Debug::Classes, 0, "      New template variable instance class `%s'`%s'\n", qPrint(name()), qPrint(templSpec));
      QByteArray tcname = removeRedundantWhiteSpace(name() + templSpec);
      templateClass = new ClassDef("<code>", 1, 1, tcname, ClassDef::Class, 0, 0, false);
      templateClass->addMembersToTemplateInstance( this, templSpec );
      templateClass->setTemplateMaster(this);
      m_impl->variableInstances->insert(templSpec, templateClass);
   }
   return templateClass;
}

void ClassDef::setTemplateBaseClassNames(QHash<QString, int> *templateNames)
{
   if (templateNames == 0) {
      return;
   }
   if (m_impl->templBaseClassNames == 0) {
      m_impl->templBaseClassNames = new QHash<QString, int>();
   }

   // make a deep copy of the dictionary.
   QHashIterator<QString, int> qdi(*templateNames);

   for (; qdi.current(); ++qdi) {
      if (m_impl->templBaseClassNames->find(qdi.currentKey()) == 0) {
         m_impl->templBaseClassNames->insert(qdi.currentKey(), new int(*qdi.current()));
      }
   }
}

QHash<QString, int> *ClassDef::getTemplateBaseClassNames() const
{
   return m_impl->templBaseClassNames;
}

void ClassDef::addMembersToTemplateInstance(ClassDef *cd, const char *templSpec)
{
   //printf("%s::addMembersToTemplateInstance(%s,%s)\n",name().data(),cd->name().data(),templSpec);
   if (cd->memberNameInfoSDict() == 0) {
      return;
   }
   MemberNameInfoSDict::Iterator mnili(*cd->memberNameInfoSDict());
   MemberNameInfo *mni;

   for (; (mni = mnili.current()); ++mnili) {

      MemberNameInfoIterator mnii(*mni);
      MemberInfo *mi;

      for (mnii.toFirst(); (mi = mnii.current()); ++mnii) {
         ArgumentList *actualArguments = new ArgumentList;
         stringToArgumentList(templSpec, actualArguments);
         MemberDef *md = mi->memberDef;

         MemberDef *imd = md->createTemplateInstanceMember(cd->templateArguments(), actualArguments);
         delete actualArguments;

         //printf("%s->setMemberClass(%p)\n",imd->name().data(),this);
         imd->setMemberClass(this);
         imd->setTemplateMaster(md);
         imd->setDocumentation(md->documentation(), md->docFile(), md->docLine());
         imd->setBriefDescription(md->briefDescription(), md->briefFile(), md->briefLine());
         imd->setInbodyDocumentation(md->inbodyDocumentation(), md->inbodyFile(), md->inbodyLine());
         imd->setMemberSpecifiers(md->getMemberSpecifiers());
         imd->setMemberGroupId(md->getMemberGroupId());
         insertMember(imd);
         //printf("Adding member=%s %s%s to class %s templSpec %s\n",
         //    imd->typeString(),imd->name().data(),imd->argsString(),
         //    imd->getClassDef()->name().data(),templSpec);
         // insert imd in the list of all members
         //printf("Adding member=%s class=%s\n",imd->name().data(),name().data());
         MemberName *mn = Doxygen::memberNameSDict->find(imd->name());
         if (mn == 0) {
            mn = new MemberName(imd->name());
            Doxygen::memberNameSDict->append(imd->name(), mn);
         }
         mn->append(imd);
      }
   }
}

QByteArray ClassDef::getReference() const
{
   if (m_impl->templateMaster) {
      return m_impl->templateMaster->getReference();
   } else {
      return Definition::getReference();
   }
}

bool ClassDef::isReference() const
{
   if (m_impl->templateMaster) {
      return m_impl->templateMaster->isReference();
   } else {
      return Definition::isReference();
   }
}

void ClassDef::getTemplateParameterLists(QList<ArgumentList> &lists) const
{
   Definition *d = getOuterScope();

   if (d) {
      if (d->definitionType() == Definition::TypeClass) {
         ClassDef *cd = (ClassDef *)d;
         cd->getTemplateParameterLists(lists);
      }
   }

   if (templateArguments()) {
      lists.append(templateArguments());
   }
}

QByteArray ClassDef::qualifiedNameWithTemplateParameters(
   QList<ArgumentList> *actualParams, int *actualParamIndex) const
{
   //static bool optimizeOutputJava = Config_getBool("OPTIMIZE_OUTPUT_JAVA");
   static bool hideScopeNames = Config_getBool("HIDE_SCOPE_NAMES");
   //printf("qualifiedNameWithTemplateParameters() localName=%s\n",localName().data());
   QByteArray scName;
   Definition *d = getOuterScope();
   if (d) {
      if (d->definitionType() == Definition::TypeClass) {
         ClassDef *cd = (ClassDef *)d;
         scName = cd->qualifiedNameWithTemplateParameters(actualParams, actualParamIndex);
      } else if (!hideScopeNames) {
         scName = d->qualifiedName();
      }
   }

   SrcLangExt lang = getLanguage();
   QByteArray scopeSeparator = getLanguageSpecificSeparator(lang);
   if (!scName.isEmpty()) {
      scName += scopeSeparator;
   }

   bool isSpecialization = localName().indexOf('<') != -1;

   QByteArray clName = className();
   //bool isGeneric = getLanguage()==SrcLangExt_CSharp;
   //if (isGeneric && clName.right(2)=="-g")
   //{
   //  clName = clName.left(clName.length()-2);
   //}
   //printf("m_impl->lang=%d clName=%s isSpecialization=%d\n",getLanguage(),clName.data(),isSpecialization);

   scName += clName;
   ArgumentList *al = 0;

   if (templateArguments()) {
      if (actualParams && *actualParamIndex < (int)actualParams->count()) {
         al = actualParams->at(*actualParamIndex);
         if (!isSpecialization) {
            scName += tempArgListToString(al, lang);
         }
         (*actualParamIndex)++;
      } else {
         if (!isSpecialization) {
            scName += tempArgListToString(templateArguments(), lang);
         }
      }
   }
   //printf("qualifiedNameWithTemplateParameters: scope=%s qualifiedName=%s\n",name().data(),scName.data());
   return scName;
}

QByteArray ClassDef::className() const
{
   if (m_impl->className.isEmpty()) {
      return localName();
   } else {
      return m_impl->className;
   }
};

void ClassDef::setClassName(const char *name)
{
   m_impl->className = name;
}

void ClassDef::addListReferences()
{
   SrcLangExt lang = getLanguage();
   if (!isLinkableInProject()) {
      return;
   }
   //printf("ClassDef(%s)::addListReferences()\n",name().data());
   {
      QList<ListItemInfo> *xrefItems = xrefListItems();
      addRefItem(xrefItems, qualifiedName(), 
                 lang == SrcLangExt_Fortran ? theTranslator->trType(true, true) : theTranslator->trClass(true, true),
                 getOutputFileBase(), displayName(), 0, this );
   }

   if (m_impl->memberGroupSDict) {
      MemberGroupSDict::Iterator mgli(*m_impl->memberGroupSDict);
      MemberGroup *mg;
      for (; (mg = mgli.current()); ++mgli) {
         mg->addListReferences(this);
      }
   }

   QListIterator<MemberList> mli(m_impl->memberLists);
   MemberList *ml;

   for (mli.toFirst(); (ml = mli.current()); ++mli) {
      if (ml->listType()&MemberListType_detailedLists) {
         ml->addListReferences(this);
      }
   }
}

MemberDef *ClassDef::getMemberByName(const QByteArray &name) const
{
   MemberDef *xmd = 0;
   if (m_impl->allMemberNameInfoSDict) {
      MemberNameInfo *mni = m_impl->allMemberNameInfoSDict->find(name);
      if (mni) {
         const int maxInheritanceDepth = 100000;
         int mdist = maxInheritanceDepth;
         MemberNameInfoIterator mnii(*mni);
         MemberInfo *mi;
         for (mnii.toFirst(); (mi = mnii.current()); ++mnii) {
            ClassDef *mcd = mi->memberDef->getClassDef();
            int m = minClassDistance(this, mcd);
            //printf("found member in %s linkable=%d m=%d\n",
            //    mcd->name().data(),mcd->isLinkable(),m);
            if (m < mdist && mcd->isLinkable()) {
               mdist = m;
               xmd = mi->memberDef;
            }
         }
      }
   }
   //printf("getMemberByName(%s)=%p\n",name.data(),xmd);
   return xmd;
}

bool ClassDef::isAccessibleMember(MemberDef *md)
{
   return md->getClassDef() && isBaseClass(md->getClassDef(), true);
}

MemberList *ClassDef::createMemberList(MemberListType lt)
{
   QListIterator<MemberList> mli(m_impl->memberLists);
   MemberList *ml;

   for (mli.toFirst(); (ml = mli.current()); ++mli) {
      if (ml->listType() == lt) {
         return ml;
      }
   }

   // not found, create a new member list
   ml = new MemberList(lt);
   m_impl->memberLists.append(ml);

   return ml;
}

MemberList *ClassDef::getMemberList(MemberListType lt)
{
   QListIterator<MemberList> mli(m_impl->memberLists);
   MemberList *ml;

   for (; (ml = mli.current()); ++mli) {
      if (ml->listType() == lt) {
         return ml;
      }
   }
   return 0;
}

void ClassDef::addMemberToList(MemberListType lt, MemberDef *md, bool isBrief)
{
   MemberList *ml = createMemberList(lt); 
   ml->append(md);

   // for members in the declaration lists we set the section, needed for member grouping
   if ((ml->listType() & MemberListType_detailedLists) == 0) {
      md->setSectionList(this, ml);
   }
}

int ClassDef::countMemberDeclarations(MemberListType lt, ClassDef *inheritedFrom,
                                      int lt2, bool invert, bool showAlways, QHash<void *, void *> *visitedClasses)
{
   //printf("%s: countMemberDeclarations for %d and %d\n",name().data(),lt,lt2);
   int count = 0;
   MemberList *ml  = getMemberList(lt);
   MemberList *ml2 = getMemberList((MemberListType)lt2);

   if (getLanguage() != SrcLangExt_VHDL) { // use specific declarations function
      if (ml) {
         ml->countDecMembers();
         count += ml->numDecMembers();
         //printf("-> ml=%d\n",ml->numDecMembers());
      }
      if (ml2) {
         ml2->countDecMembers();
         count += ml2->numDecMembers();
         //printf("-> ml2=%d\n",ml2->numDecMembers());
      }
      // also include grouped members that have their own section in the class (see bug 722759)
      if (inheritedFrom && m_impl->memberGroupSDict) {
         MemberGroupSDict::Iterator mgli(*m_impl->memberGroupSDict);
         MemberGroup *mg;
         for (; (mg = mgli.current()); ++mgli) {
            count += mg->countGroupedInheritedMembers(lt);
            if (lt2 != 1) {
               count += mg->countGroupedInheritedMembers((MemberListType)lt2);
            }
         }
      }

      static bool inlineInheritedMembers = Config_getBool("INLINE_INHERITED_MEMB");

      if (! inlineInheritedMembers) {
         // show inherited members as separate lists
         count += countInheritedDecMembers(lt, inheritedFrom, invert, showAlways, visitedClasses);
      }
   }

   return count;
}


int ClassDef::countInheritedDecMembers(MemberListType lt, ClassDef *inheritedFrom, bool invert, bool showAlways, QHash<void *, void *> *visitedClasses)
{
   int inhCount = 0;
   int count    = countMembersIncludingGrouped(lt, inheritedFrom, false);
   bool process = count > 0;

   //printf("%s: countInheritedDecMembers: lt=%d process=%d count=%d invert=%d\n",
   //    name().data(),lt,process,count,invert);

   if ((process ^ invert) || showAlways) {

      if (m_impl->m_parents) {

         for (auto ibcd : *m_impl->m_parents) {
            ClassDef *icd = ibcd->classDef;
            int lt1;
            int lt2;

            if (icd->isLinkable()) {
               convertProtectionLevel(lt, ibcd->prot, &lt1, &lt2);
               
               if (! visitedClasses->contains(icd)) {
                  // guard for multiple virtual inheritance
                  visitedClasses->insert(icd, icd); 

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

void ClassDef::getTitleForMemberListType(MemberListType type,
      QByteArray &title, QByteArray &subtitle)
{
   SrcLangExt lang = getLanguage();
   QListIterator<LayoutDocEntry> eli(LayoutDocManager::instance().docEntries(LayoutDocManager::Class));
   LayoutDocEntry *lde;

   for (eli.toFirst(); (lde = eli.current()); ++eli) {
      if (lde->kind() == LayoutDocEntry::MemberDecl) {
         LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;
         if (lmd->type == type) {
            title = lmd->title(lang);
            subtitle = lmd->subtitle(lang);
            return;
         }
      }
   }
   title = "";
   subtitle = "";
}

int ClassDef::countAdditionalInheritedMembers()
{
   int totalCount = 0;

   QListIterator<LayoutDocEntry> eli(LayoutDocManager::instance().docEntries(LayoutDocManager::Class));
   LayoutDocEntry *lde;

   for (eli.toFirst(); (lde = eli.current()); ++eli) {

      if (lde->kind() == LayoutDocEntry::MemberDecl) {
         LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;

         if (lmd->type != MemberListType_friends) { // friendship is not inherited                       
            QHash<void *, void *> visited();
            totalCount += countInheritedDecMembers(lmd->type, this, true, false, &visited);            
         }
      }
   }

   return totalCount;
}

void ClassDef::writeAdditionalInheritedMembers(OutputList &ol)
{
   //printf("**** writeAdditionalInheritedMembers()\n");
   QListIterator<LayoutDocEntry> eli(LayoutDocManager::instance().docEntries(LayoutDocManager::Class));
   LayoutDocEntry *lde;

   for (eli.toFirst(); (lde = eli.current()); ++eli) {

      if (lde->kind() == LayoutDocEntry::MemberDecl) {
         LayoutDocEntryMemberDecl *lmd = (LayoutDocEntryMemberDecl *)lde;

         if (lmd->type != MemberListType_friends) {
            QHash<void *, void *> visited;
            writeInheritedMemberDeclarations(ol, lmd->type, -1, lmd->title(getLanguage()), this, true, false, &visited);
         }
      }
   }
}

int ClassDef::countMembersIncludingGrouped(MemberListType lt,
      ClassDef *inheritedFrom, bool additional)
{
   int count = 0;
   MemberList *ml = getMemberList(lt);

   if (ml) {
      count = ml->countInheritableMembers(inheritedFrom);
   }

   //printf("%s:countMembersIncludingGrouped: count=%d\n",name().data(),count);
   if (m_impl->memberGroupSDict) {
      MemberGroupSDict::Iterator mgli(*m_impl->memberGroupSDict);
      MemberGroup *mg;
      for (; (mg = mgli.current()); ++mgli) {
         bool hasOwnSection = !mg->allMembersInSameSection() ||
                              !m_impl->subGrouping; // group is in its own section
         if ((additional && hasOwnSection) || (!additional && !hasOwnSection)) {
            count += mg->countGroupedInheritedMembers(lt);
         }
      }
   }
   //printf("%s:countMembersIncludingGrouped(lt=%d,%s)=%d\n",
   //    name().data(),lt,ml?ml->listTypeAsString(ml->listType()).data():"<none>",count);
   return count;
}

void ClassDef::writeInheritedMemberDeclarations(OutputList &ol,
      MemberListType lt, int lt2, const QByteArray &title,
      ClassDef *inheritedFrom, bool invert, bool showAlways,
      QPtrDict<void> *visitedClasses)
{
   ol.pushGeneratorState();
   ol.disableAllBut(OutputGenerator::Html);
   int count = countMembersIncludingGrouped(lt, inheritedFrom, false);
   bool process = count > 0;
   //printf("%s: writeInheritedMemberDec: lt=%d process=%d invert=%d always=%d\n",
   //    name().data(),lt,process,invert,showAlways);
   if ((process ^ invert) || showAlways) {

      if (m_impl->m_parents) {
         QListIterator<BaseClassDef *> it(*m_impl->m_parents);
         BaseClassDef *ibcd;

         for (it.toFirst(); (ibcd = it.current()); ++it) {
            ClassDef *icd = ibcd->classDef;
            if (icd->isLinkable()) {
               int lt1, lt3;
               convertProtectionLevel(lt, ibcd->prot, &lt1, &lt3);
               if (lt2 == -1 && lt3 != -1) {
                  lt2 = lt3;
               }
               //printf("%s:convert %d->(%d,%d) prot=%d\n",icd->name().data(),lt,lt1,lt2,ibcd->prot);
               if (visitedClasses->find(icd) == 0) {
                  visitedClasses->insert(icd, icd); // guard for multiple virtual inheritance
                  if (lt1 != -1) {
                     icd->writeMemberDeclarations(ol, (MemberListType)lt1,
                                                  title, QByteArray(), false, inheritedFrom, lt2, false, true, visitedClasses);
                  }
               } else {
                  //printf("%s: class already visited!\n",icd->name().data());
               }
            }
         }
      }
   }
   ol.popGeneratorState();
}

void ClassDef::writeMemberDeclarations(OutputList &ol, MemberListType lt, const QByteArray &title,
                                       const char *subTitle, bool showInline, ClassDef *inheritedFrom, int lt2,
                                       bool invert, bool showAlways, QHash<void *, void *> *visitedClasses)
{
   //printf("%s: ClassDef::writeMemberDeclarations lt=%d lt2=%d\n",name().data(),lt,lt2);

   MemberList *ml  = getMemberList(lt);
   MemberList *ml2 = getMemberList((MemberListType)lt2);
      
   QByteArray tt = title, st = subTitle;

   if (ml) {
      //printf("  writeDeclaration type=%d count=%d\n",lt,ml->numDecMembers());
      ml->writeDeclarations(ol, this, 0, 0, 0, tt, st, false, showInline, inheritedFrom, lt);
      tt.resize(0);
      st.resize(0);
   }

   if (ml2) {
      //printf("  writeDeclaration type=%d count=%d\n",lt2,ml2->numDecMembers());
      ml2->writeDeclarations(ol, this, 0, 0, 0, tt, st, false, showInline, inheritedFrom, lt);
   }

   static bool inlineInheritedMembers = Config_getBool("INLINE_INHERITED_MEMB");

   if (!inlineInheritedMembers) { // show inherited members as separate lists
      QHash<void *, void *> visited;
      writeInheritedMemberDeclarations(ol, lt, lt2, title, inheritedFrom ? inheritedFrom : this,
                                       invert, showAlways, visitedClasses == 0 ? &visited : visitedClasses);
   }
 
}

void ClassDef::addGroupedInheritedMembers(OutputList &ol, MemberListType lt,
      ClassDef *inheritedFrom, const QByteArray &inheritId)
{
   //printf("** %s::addGroupedInheritedMembers(%p) inheritId=%s\n",name().data(),m_impl->memberGroupSDict,inheritId.data());
   if (m_impl->memberGroupSDict) {
      MemberGroupSDict::Iterator mgli(*m_impl->memberGroupSDict);
      MemberGroup *mg;
      for (; (mg = mgli.current()); ++mgli) {
         if (!mg->allMembersInSameSection() || !m_impl->subGrouping) { // group is in its own section
            mg->addGroupedInheritedMembers(ol, this, lt, inheritedFrom, inheritId);
         }
      }
   }
}

void ClassDef::writeMemberDocumentation(OutputList &ol, MemberListType lt, const QByteArray &title, bool showInline)
{
   //printf("%s: ClassDef::writeMemberDocumentation()\n",name().data());
   MemberList *ml = getMemberList(lt);
   if (ml) {
      ml->writeDocumentation(ol, displayName(), this, title, false, showInline);
   }
}

void ClassDef::writeSimpleMemberDocumentation(OutputList &ol, MemberListType lt)
{
   //printf("%s: ClassDef::writeSimpleMemberDocumentation()\n",name().data());
   MemberList *ml = getMemberList(lt);
   if (ml) {
      ml->writeSimpleDocumentation(ol, this);
   }
}

void ClassDef::writePlainMemberDeclaration(OutputList &ol,
      MemberListType lt, bool inGroup,
      ClassDef *inheritedFrom, const char *inheritId)
{
   //printf("%s: ClassDef::writePlainMemberDeclaration()\n",name().data());
   MemberList *ml = getMemberList(lt);
   if (ml) {
      ml->setInGroup(inGroup);
      ml->writePlainDeclarations(ol, this, 0, 0, 0, inheritedFrom, inheritId);
   }
}

bool ClassDef::isLocal() const
{
   return m_impl->isLocal;
}

ClassSDict *ClassDef::getClassSDict()
{
   return m_impl->innerClasses;
}

ClassDef::CompoundType ClassDef::compoundType() const
{
   return m_impl->compType;
}

SortedList<BaseClassDef *> *ClassDef::baseClasses() const
{
   return m_impl->m_parents;
}

SortedList<BaseClassDef *> *ClassDef::subClasses() const
{
   return m_impl->inheritedBy;
}

MemberNameInfoSDict *ClassDef::memberNameInfoSDict() const
{
   return m_impl->allMemberNameInfoSDict;
}

Protection ClassDef::protection() const
{
   return m_impl->prot;
}

ArgumentList *ClassDef::templateArguments() const
{
   return m_impl->tempArgs;
}

NamespaceDef *ClassDef::getNamespaceDef() const
{
   return m_impl->nspace;
}

FileDef *ClassDef::getFileDef() const
{
   return m_impl->fileDef;
}

QHash<QString, ClassDef> *ClassDef::getTemplateInstances() const
{
   return m_impl->templateInstances;
}

ClassDef *ClassDef::templateMaster() const
{
   return m_impl->templateMaster;
}

bool ClassDef::isTemplate() const
{
   return m_impl->tempArgs != 0;
}

IncludeInfo *ClassDef::includeInfo() const
{
   return m_impl->incInfo;
}

UsesClassDict *ClassDef::usedImplementationClasses() const
{
   return m_impl->usesImplClassDict;
}

UsesClassDict *ClassDef::usedByImplementationClasses() const
{
   return m_impl->usedByImplClassDict;
}

UsesClassDict *ClassDef::usedInterfaceClasses() const
{
   return m_impl->usesIntfClassDict;
}

bool ClassDef::isTemplateArgument() const
{
   return m_impl->isTemplArg;
}

bool ClassDef::isAbstract() const
{
   return m_impl->isAbstract || (m_impl->spec & Entry::Abstract);
}

bool ClassDef::isFinal() const
{
   return m_impl->spec & Entry::Final;
}

bool ClassDef::isSealed() const
{
   return m_impl->spec & Entry::Sealed;
}

bool ClassDef::isPublished() const
{
   return m_impl->spec & Entry::Published;
}

bool ClassDef::isForwardDeclared() const
{
   return m_impl->spec & Entry::ForwardDecl;
}

bool ClassDef::isObjectiveC() const
{
   return getLanguage() == SrcLangExt_ObjC;
}

bool ClassDef::isCSharp() const
{
   return getLanguage() == SrcLangExt_CSharp;
}

ClassDef *ClassDef::categoryOf() const
{
   return m_impl->categoryOf;
}

const QList<MemberList> &ClassDef::getMemberLists() const
{
   return m_impl->memberLists;
}

MemberGroupSDict *ClassDef::getMemberGroupSDict() const
{
   return m_impl->memberGroupSDict;
}

void ClassDef::setNamespace(NamespaceDef *nd)
{
   m_impl->nspace = nd;
}

void ClassDef::setFileDef(FileDef *fd)
{
   m_impl->fileDef = fd;
}

void ClassDef::setSubGrouping(bool enabled)
{
   m_impl->subGrouping = enabled;
}

void ClassDef::setProtection(Protection p)
{
   m_impl->prot = p;
}

void ClassDef::setIsStatic(bool b)
{
   m_impl->isStatic = b;
}

void ClassDef::setCompoundType(CompoundType t)
{
   m_impl->compType = t;
}

void ClassDef::setTemplateMaster(ClassDef *tm)
{
   m_impl->templateMaster = tm;
}

void ClassDef::makeTemplateArgument(bool b)
{
   m_impl->isTemplArg = b;
}

void ClassDef::setCategoryOf(ClassDef *cd)
{
   m_impl->categoryOf = cd;
}

void ClassDef::setUsedOnly(bool b)
{
   m_impl->usedOnly = b;
}

bool ClassDef::isUsedOnly() const
{
   return m_impl->usedOnly;
}

bool ClassDef::isSimple() const
{
   return m_impl->isSimple;
}

MemberDef *ClassDef::isSmartPointer() const
{
   return m_impl->arrowOperator;
}

void ClassDef::reclassifyMember(MemberDef *md, MemberType t)
{
   md->setMemberType(t);
  
   for(auto ml : m_impl->memberLists) {
      ml->removeOne(md);
   }

   insertMember(md);
}

QByteArray ClassDef::anchor() const
{
   QByteArray anc;

   if (isEmbeddedInOuterScope() && !Doxygen::generatingXmlOutput) {
      if (m_impl->templateMaster) {
         // point to the template of which this class is an instance
         anc = m_impl->templateMaster->getOutputFileBase();
      } else if (isReference()) {
         // point to the external location
         anc = m_impl->fileName;
      } else {
         // normal locally defined class
         anc = convertNameToFile(m_impl->fileName);
      }
   }
   return anc;
}

bool ClassDef::isEmbeddedInOuterScope() const
{
   static bool inlineGroupedClasses = Config_getBool("INLINE_GROUPED_CLASSES");
   static bool inlineSimpleClasses = Config_getBool("INLINE_SIMPLE_STRUCTS");

   Definition *container = getOuterScope();

   bool containerLinkable = container &&
      ( 
         (container == Doxygen::globalScope && getFileDef() && getFileDef()->isLinkableInProject()) || // global class in documented file
         container->isLinkableInProject() // class in documented scope
      );

   // inline because of INLINE_GROUPED_CLASSES=YES ?

   bool b1 = (inlineGroupedClasses && partOfGroups() != 0); // a grouped class

   // inline because of INLINE_SIMPLE_STRUCTS=YES ?

   bool b2 = (inlineSimpleClasses && m_impl->isSimple && // a simple class
              (containerLinkable || // in a documented container
               partOfGroups() != 0  // or part of a group 
              ) );

   //printf("%s::isEmbeddedInOuterScope(): inlineGroupedClasses=%d "
   //       "inlineSimpleClasses=%d partOfGroups()=%p m_impl->isSimple=%d "
   //       "getOuterScope()=%s b1=%d b2=%d\n",
   //    name().data(),inlineGroupedClasses,inlineSimpleClasses,
   //    partOfGroups().pointer(),m_impl->isSimple,getOuterScope()?getOuterScope()->name().data():"<none>",b1,b2);
   return b1 || b2;  // either reason will do
}

const SortedList<ClassDef *> *ClassDef::taggedInnerClasses() const
{
   return m_impl->taggedInnerClasses;
}

void ClassDef::addTaggedInnerClass(ClassDef *cd)
{
   if (m_impl->taggedInnerClasses == 0) {
      m_impl->taggedInnerClasses = new SortedList<ClassDef *>;
   }

   m_impl->taggedInnerClasses->append(cd);
}

ClassDef *ClassDef::tagLessReference() const
{
   return m_impl->tagLessRef;
}

void ClassDef::setTagLessReference(ClassDef *cd)
{
   m_impl->tagLessRef = cd;
}

void ClassDef::removeMemberFromLists(MemberDef *md)
{ 
   for(auto ml : m_impl->memberLists) {
      ml->removeOne(md);
   }
}

bool ClassDef::isJavaEnum() const
{
   return m_impl->isJavaEnum;
}

bool ClassDef::isGeneric() const
{
   return m_impl->isGeneric;
}

void ClassDef::setClassSpecifier(uint64_t spec)
{
   m_impl->spec = spec;
}

bool ClassDef::isExtension() const
{
   QByteArray n = name();
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
   return m_impl->innerClasses;
}

const FileList &ClassDef::usedFiles() const
{
   return m_impl->files;
}

const ArgumentList *ClassDef::typeConstraints() const
{
   return m_impl->typeConstraints;
}

const ExampleSDict *ClassDef::exampleList() const
{
   return m_impl->exampleSDict;
}

bool ClassDef::subGrouping() const
{
   return m_impl->subGrouping;
}

void ClassDef::setName(const char *name)
{
   m_impl->isAnonymous = QByteArray(name).indexOf('@') != -1;
   Definition::setName(name);
}

bool ClassDef::isAnonymous() const
{
   return m_impl->isAnonymous;
}
