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

#ifndef CLASSDEF_H
#define CLASSDEF_H

#include <QList>
#include <QHash>
#include <QSet>
#include <QTextStream>

#include <arguments.h>
#include <definition.h>
#include <example.h>
#include <groupdef.h>
#include <memberdef.h>
#include <memberlist.h>
#include <membergroup.h>
#include <membername.h>
#include <namespacedef.h>
#include <outputlist.h>

class ClassDict;
class FileDef;
class PackageDef;
class StringDict;
class UsesClassDict;

struct IncludeInfo;

template <class T>
class SortedList;

/** A class representing a compound symbol
 *
 *  A compound symbol can be a class, struct, union, interface, service, singleton, or exception.
 *  \note This class should be renamed to CompoundDef
 */
class ClassDef : public Definition
{
 public:
   /** The various compound types */
   enum CompoundType { Class,     //=Entry::CLASS_SEC,
                       Struct,    //=Entry::STRUCT_SEC,
                       Union,     //=Entry::UNION_SEC,
                       Interface, //=Entry::INTERFACE_SEC,
                       Protocol,  //=Entry::PROTOCOL_SEC,
                       Category,  //=Entry::CATEGORY_SEC,
                       Exception, //=Entry::EXCEPTION_SEC
                       Service,   //=Entry::CLASS_SEC
                       Singleton, //=Entry::CLASS_SEC
                     };

   /** Creates a new compound definition
    *  \param fileName  full path and file name in which this compound was
    *                   found.
    *  \param startLine line number where the definition of this compound
    *                   starts.
    *  \param startColumn column number where the definition of this compound
    *                   starts.
    *  \param name      the name of this compound (including scope)
    *  \param ct        the kind of Compound
    *  \param ref       the tag file from which this compound is extracted
    *                   or 0 if the compound doesn't come from a tag file
    *  \param fName     the file name as found in the tag file.
    *                   This overwrites the file that doxygen normally
    *                   generates based on the compound type & name.
    *  \param isSymbol  If true this class name is added as a publicly
    *                   visible (and referencable) symbol.
    *  \param isJavaEnum If true this class is actually a Java enum.
    *                    I didn't add this to CompoundType to avoid having
    *                    to adapt all translators.
    */
   ClassDef(const char *fileName, int startLine, int startColumn, const char *name, CompoundType ct,
            const char *ref = 0, QString fName = QString(), bool isSymbol = true, bool isJavaEnum = false);
  
   /** Destroys a compound definition. */
   ~ClassDef();

   /** Used for RTTI, this is a class */
   DefType definitionType() const override {
      return TypeClass;
   }

   /** Returns the unique base name (without extension) of the class's file on disk */
   QByteArray getOutputFileBase() const;
   QString getInstanceOutputFileBase() const;
   QByteArray getFileBase() const;

   /** Returns the base name for the source code file */
   QByteArray getSourceFileBase() const;

   /** If this class originated from a tagfile, this will return the tag file reference */
   QByteArray getReference() const;

   /** Returns true if this class is imported via a tag file */
   bool isReference() const;

   /** Returns true if this is a local class definition, see EXTRACT_LOCAL_CLASSES */
   bool isLocal() const;

   /** returns the classes nested into this class */
   ClassSDict *getClassSDict();

   /** returns true if this class has documentation */
   bool hasDocumentation() const;

   /** returns true if this class has a non-empty detailed description */
   bool hasDetailedDescription() const;

   /** Returns the name as it is appears in the documentation */
   QString displayName(bool = true) const override;

   /** Returns the type of compound this is, i.e. class/struct/union/.. */
   CompoundType compoundType() const;

   /** Returns the type of compound as a string */
   QByteArray compoundTypeString() const;

   /** Returns the list of base classes from which this class directly
    *  inherits.
    */
   SortedList<BaseClassDef *> *baseClasses() const;

   /** Returns the list of sub classes that directly derive from this class
    */
   SortedList<BaseClassDef *> *subClasses() const;

   /** Returns a dictionary of all members. This includes any inherited
    *  members. Members are sorted alphabetically.
    */
   MemberNameInfoSDict *memberNameInfoSDict() const;

   /** Return the protection level (Public,Protected,Private) in which
    *  this compound was found.
    */
   Protection protection() const;

   /** returns true iff a link is possible to this item within this project.
    */
   bool isLinkableInProject() const;

   /** return true iff a link to this class is possible (either within
    *  this project, or as a cross-reference to another project).
    */
   bool isLinkable() const;

   /** the class is visible in a class diagram, or class hierarchy */
   bool isVisibleInHierarchy();

   /** show this class in the declaration section of its parent? */
   bool visibleInParentsDeclList() const;

   /** Returns the template arguments of this class
    *  Will return 0 if not applicable.
    */
   const ArgumentList *templateArguments() const;
   ArgumentList *templateArguments();

   /** Returns the namespace this compound is in, or 0 if it has a global
    *  scope.
    */
   NamespaceDef *getNamespaceDef() const;

   /** Returns the file in which this compound's definition can be found.
    *  Should not return 0 (but it might be a good idea to check anyway).
    */
   FileDef      *getFileDef() const;

   /** Returns the Java package this class is in or 0 if not applicable.
    */

   MemberDef    *getMemberByName(const QByteArray &) const;

   /** Returns true iff \a bcd is a direct or indirect base class of this
    *  class. This function will recusively traverse all branches of the
    *  inheritance tree.
    */
   bool isBaseClass(ClassDef *bcd, bool followInstances, int level = 0);

   /** Returns true iff \a bcd is a direct or indirect sub class of this
    *  class.
    */
   bool isSubClass(ClassDef *bcd, int level = 0);

   /** returns true iff \a md is a member of this class or of the
    *  the public/protected members of a base class
    */
   bool isAccessibleMember(MemberDef *md);

   /** Returns a sorted dictionary with all template instances found for
    *  this template class. Returns 0 if not a template or no instances.
    */
   QHash<QString, ClassDef> *getTemplateInstances() const;

   /** Returns the template master of which this class is an instance.
    *  Returns 0 if not applicable.
    */
   ClassDef *templateMaster() const;

   /** Returns true if this class is a template */
   bool isTemplate() const;

   IncludeInfo *includeInfo() const;

   UsesClassDict *usedImplementationClasses() const;

   UsesClassDict *usedByImplementationClasses() const;

   UsesClassDict *usedInterfaceClasses() const;

   bool isTemplateArgument() const;

   /** Returns the definition of a nested compound if
    *  available, or 0 otherwise.
    *  @param name The name of the nested compound
    */
   virtual QSharedPointer<Definition> findInnerCompound(const char *name) override;

   /** Returns the template parameter lists that form the template
    *  declaration of this class.
    *
    *  Example: <code>template<class T> class TC {};</code>
    *  will return a list with one ArgumentList containing one argument
    *  with type="class" and name="T".
    */
   void getTemplateParameterLists(QList<ArgumentList> &lists) const;

   QByteArray qualifiedNameWithTemplateParameters(QList<ArgumentList> *actualParams = 0, int *actualParamIndex = 0) const;

   /** Returns true if there is at least one pure virtual member in this
    *  class.
    */
   bool isAbstract() const;

   /** Returns true if this class is implemented in Objective-C */
   bool isObjectiveC() const;

   /** Returns true if this class is implemented in C# */
   bool isCSharp() const;

   /** Returns true if this class is marked as final */
   bool isFinal() const;

   /** Returns true if this class is marked as sealed */
   bool isSealed() const;

   /** Returns true if this class is marked as published */
   bool isPublished() const;

   /** Returns true if this class represents an Objective-C 2.0 extension (nameless category) */
   bool isExtension() const;

   /** Returns true if this class represents a forward declaration of a template class */
   bool isForwardDeclared() const;

   /** Returns the class of which this is a category (Objective-C only) */
   ClassDef *categoryOf() const;

   /** Returns the name of the class including outer classes, but not
    *  including namespaces.
    */
   QByteArray className() const;

   /** Returns the members in the list identified by \a lt */
   MemberList *getMemberList(MemberListType lt);

   /** Returns the list containing the list of members sorted per type */
   const QList<QSharedPointer<MemberList>> &getMemberLists() const;

   /** Returns the member groups defined for this class */
   MemberGroupSDict *getMemberGroupSDict() const;

   QHash<QString, int> *getTemplateBaseClassNames() const;

   ClassDef *getVariableInstance(const char *templSpec);

   bool isUsedOnly() const;

   QByteArray anchor() const;
   bool isEmbeddedInOuterScope() const;

   bool isSimple() const;

   const SortedList<QSharedPointer<ClassDef>>   *taggedInnerClasses() const;
   ClassDef *tagLessReference() const;

   MemberDef *isSmartPointer() const;

   bool isJavaEnum() const;

   bool isGeneric() const;
   bool isAnonymous() const;
   const ClassSDict *innerClasses() const;
   QByteArray title() const;

   QByteArray generatedFromFiles() const;
   const FileList &usedFiles() const;

   const ArgumentList *typeConstraints() const;
   const ExampleSDict *exampleList() const;
   bool hasExamples() const;
   QString getMemberListFileName() const;
   bool subGrouping() const;

   void insertBaseClass(ClassDef *, const char *name, Protection p, Specifier s, const char *t = 0);
   void insertSubClass(ClassDef *, Protection p, Specifier s, const char *t = 0);
   void setIncludeFile(FileDef *fd, const char *incName, bool local, bool force);
   void insertMember(MemberDef *);
   void insertUsedFile(FileDef *);
   bool addExample(const char *anchor, const char *name, const char *file);
   void mergeCategory(ClassDef *category);
   void setNamespace(NamespaceDef *nd);
   void setFileDef(FileDef *fd);
   void setSubGrouping(bool enabled);
   void setProtection(Protection p);
   void setGroupDefForAllMembers(GroupDef *g, Grouping::GroupPri_t pri, const QByteArray &fileName, int startLine, bool hasDocs);

   void addInnerCompound(QSharedPointer<Definition> d) override; 

   ClassDef *insertTemplateInstance(const QByteArray &fileName, int startLine, int startColumn,
                                    const QByteArray &templSpec, bool &freshInstance);

   void addUsedClass(ClassDef *cd, const char *accessName, Protection prot);
   void addUsedByClass(ClassDef *cd, const char *accessName, Protection prot);
   void setIsStatic(bool b);
   void setCompoundType(CompoundType t);
   void setClassName(const char *name);
   void setClassSpecifier(uint64_t spec);

   void setTemplateArguments(ArgumentList *al);
   void setTemplateBaseClassNames(QHash<QString, int> *templateNames);
   void setTemplateMaster(ClassDef *tm);
   void setTypeConstraints(ArgumentList *al);
   void addMembersToTemplateInstance(ClassDef *cd, const char *templSpec);
   void makeTemplateArgument(bool b = true);
   void setCategoryOf(ClassDef *cd);
   void setUsedOnly(bool b);

   void addTaggedInnerClass(QSharedPointer<ClassDef> cd);
   void setTagLessReference(QSharedPointer<ClassDef> cd);
   void setName(const char *name);

   // actions
   void findSectionsInDocumentation();
   void addMembersToMemberGroup();
   void addListReferences();
   void computeAnchors();
   void mergeMembers();
   void sortMemberLists();
   void distributeMemberGroupDocumentation();
   void writeDocumentation(OutputList &ol);
   void writeDocumentationForInnerClasses(OutputList &ol);
   void writeMemberPages(OutputList &ol);
   void writeMemberList(OutputList &ol);
   void writeDeclaration(OutputList &ol, MemberDef *md, bool inGroup, ClassDef *inheritedFrom, const char *inheritId);
   void writeQuickMemberLinks(OutputList &ol, MemberDef *md) const;
   void writeSummaryLinks(OutputList &ol);
   void reclassifyMember(MemberDef *md, MemberType t);
   void writeInlineDocumentation(OutputList &ol);
   void writeDeclarationLink(OutputList &ol, bool &found, const char *header, bool localNames);
   void removeMemberFromLists(MemberDef *md);
   void addGroupedInheritedMembers(OutputList &ol, MemberListType lt, ClassDef *inheritedFrom, const QByteArray &inheritId);
   int countMembersIncludingGrouped(MemberListType lt, ClassDef *inheritedFrom, bool additional);
   int countInheritanceNodes();
   void writeTagFile(QTextStream &);

   bool visited;

 protected:
   void addUsedInterfaceClasses(MemberDef *md, const char *typeStr);
   bool hasNonReferenceSuperClass();
   void showUsedFiles(OutputList &ol);

 private:
   void writeDocumentationContents(OutputList &ol, const QByteArray &pageTitle);
   void internalInsertMember(MemberDef *md, Protection prot, bool addToAllList);
   void addMemberToList(MemberListType lt, MemberDef *md, bool isBrief);

   QSharedPointer<MemberList> createMemberList(MemberListType lt);

   void writeInheritedMemberDeclarations(OutputList &ol, MemberListType lt, int lt2,
                                         const QByteArray &title, ClassDef *inheritedFrom,
                                         bool invert, bool showAlways,
                                         QHash<void *, void *> *visitedClasses);

   void writeMemberDeclarations(OutputList &ol, MemberListType lt, const QByteArray &title,
                                const char *subTitle = 0, bool showInline = false,
                                ClassDef *inheritedFrom = 0, int lt2 = -1, bool invert = false,
                                bool showAlways = false, QHash<void *, void *> *visitedClasses = 0);

   void writeMemberDocumentation(OutputList &ol, MemberListType lt, const QByteArray &title, bool showInline = false);
   void writeSimpleMemberDocumentation(OutputList &ol, MemberListType lt);
   void writePlainMemberDeclaration(OutputList &ol, MemberListType lt, bool inGroup, ClassDef *inheritedFrom, const char *inheritId);
   void writeBriefDescription(OutputList &ol, bool exampleFlag);
   void writeDetailedDescription(OutputList &ol, const QByteArray &pageType, bool exampleFlag,
                                 const QByteArray &title, const QByteArray &anchor = QByteArray());

   void writeIncludeFiles(OutputList &ol);
   //void writeAllMembersLink(OutputList &ol);
   void writeInheritanceGraph(OutputList &ol);
   void writeCollaborationGraph(OutputList &ol);
   void writeMemberGroups(OutputList &ol, bool showInline = false);
   void writeNestedClasses(OutputList &ol, const QByteArray &title);
   void writeInlineClasses(OutputList &ol);
   void startMemberDeclarations(OutputList &ol);
   void endMemberDeclarations(OutputList &ol);
   void startMemberDocumentation(OutputList &ol);
   void endMemberDocumentation(OutputList &ol);
   void writeAuthorSection(OutputList &ol);
   void writeMoreLink(OutputList &ol, const QByteArray &anchor);
   void writeDetailedDocumentationBody(OutputList &ol);

   int countAdditionalInheritedMembers();
   void writeAdditionalInheritedMembers(OutputList &ol);
   void addClassAttributes(OutputList &ol);
   int countMemberDeclarations(MemberListType lt, ClassDef *inheritedFrom,
                               int lt2, bool invert, bool showAlways, QHash<void *, void *> *visitedClasses);

   int countInheritedDecMembers(MemberListType lt, ClassDef *inheritedFrom, bool invert, bool showAlways,
                                QHash<void *, void *> *visitedClasses);

   void getTitleForMemberListType(MemberListType type, QByteArray &title, QByteArray &subtitle);
   QByteArray includeStatement() const;
  
   /*! file name that forms the base for the output file containing the
    *  class documentation. For compatibility with Qt (e.g. links via tag
    *  files) this name cannot be derived from the class name directly.
    */
   QString m_fileName;

   /*! Include information about the header file should be included
    *  in the documentation. 0 by default, set by setIncludeFile().
    */
   IncludeInfo *m_incInfo;

   /*! List of base class (or super-classes) from which this class derives
    *  directly.
    */
   SortedList<BaseClassDef *> *m_parents;

   /*! List of sub-classes that directly derive from this class
    */
   SortedList<BaseClassDef *> *m_inheritedBy;

   /*! Namespace this class is part of
    *  (this is the inner most namespace in case of nested namespaces)
    */
   NamespaceDef  *m_nspace;

   /*! File this class is defined in */
   FileDef *m_fileDef;

   /*! List of all members (including inherited members) */
   MemberNameInfoSDict *m_allMemberNameInfoSDict;

   /*! Template arguments of this class */
   ArgumentList m_tempArgs;

   /*! Type constraints for template parameters */
   ArgumentList m_typeConstraints;

   /*! Files that were used for generating the class documentation. */
   FileList m_files;

   /*! Examples that use this class */
   ExampleSDict *m_exampleSDict;

   /*! Holds the kind of "class" this is. */
   ClassDef::CompoundType m_compType;

   /*! The protection level in which this class was found.
    *  Typically Public, but for nested classes this can also be Protected
    *  or Private.
    */
   Protection m_prot;

   /*! The inner classes contained in this class. Will be 0 if there are
    *  no inner classes.
    */
   ClassSDict *m_innerClasses;

   /* classes for the collaboration diagram */
   UsesClassDict *m_usesImplClassDict;
   UsesClassDict *m_usedByImplClassDict;
   UsesClassDict *m_usesIntfClassDict;

   /*! Template instances that exists of this class, the key in the
    *  dictionary is the template argument list.
    */
   QHash<QString, ClassDef> *m_templateInstances;

   /*! Template instances that exists of this class, as defined by variables.
    *  We do NOT want to document these individually. The key in the
    *  dictionary is the template argument list.
    */
   QHash<QString, ClassDef> *m_variableInstances;

   QHash<QString, int> *m_templBaseClassNames;

   /*! The class this class is an instance of. */
   ClassDef *m_templateMaster;

   /*! local class name which could be a typedef'ed alias name. */
   QByteArray m_className;

   /*! If this class is a Objective-C category, then this points to the
    *  class which is extended.
    */
   ClassDef *m_categoryOf;

   QList<QSharedPointer<MemberList>> m_memberLists;

   /* user defined member groups */
   MemberGroupSDict *m_memberGroupSDict;

   /*! Is this an abstact class? */
   bool m_isAbstract;

   /*! Is the class part of an unnamed namespace? */
   bool m_isStatic;

   /*! true if classes members are merged with those of the base classes. */
   bool m_membersMerged;

   /*! true if the class is defined in a source file rather than a header file. */
   bool m_isLocal;

   bool m_isTemplArg;

   /*! Does this class group its user-grouped members
    *  as a sub-section of the normal (public/protected/..)
    *  groups?
    */
   bool m_subGrouping;

   /** Reason of existence is a "use" relation */
   bool m_usedOnly;

   /** Is this a simple (non-nested) C structure? */
   bool m_isSimple;

   /** Does this class overloaded the -> operator? */
   MemberDef *m_arrowOperator;

   SortedList<QSharedPointer<ClassDef>> *m_taggedInnerClasses;
   ClassDef *m_tagLessRef;

   /** Does this class represent a Java style enum? */
   bool m_isJavaEnum;

   bool m_isGeneric;
   bool m_isAnonymous;

   uint64_t m_spec;
};

/** Class that contains information about a usage relation.
 */
class UsesClassDef 
{
 public:
   UsesClassDef(ClassDef *cd) : m_classDef(cd) {      
      m_containment = true;
   }

   ~UsesClassDef() 
   { }

   void addAccessor(const char *s) {
      if (m_accessors.contains(s)) {
         m_accessors.insert(s);        
      }
   }

   /** Class definition this relation uses. */
   ClassDef *m_classDef;

   /** Dictionary of member variable names that form the edge labels of the
    *  usage relation.
    */
   QSet<QString> m_accessors;

   /** Template arguments used for the base class */
   QByteArray m_templSpecifiers;

   bool m_containment;
};

/** Dictionary of usage relations.
 */
class UsesClassDict : public QHash<QString, UsesClassDef>
{
 public:
   UsesClassDict() : QHash<QString, UsesClassDef>()
   { }

   ~UsesClassDict()
   {}
};

/** Iterator class to iterate over a dictionary of usage relations.
 */
class UsesClassDictIterator : public QHashIterator<QString, UsesClassDef>
{
 public:
   UsesClassDictIterator(const QHash<QString, UsesClassDef> &d)
      : QHashIterator<QString, UsesClassDef>(d) {}

   ~UsesClassDictIterator() {}
};

/** Class that contains information about an inheritance relation
 */
struct BaseClassDef {
   BaseClassDef(ClassDef *cd, const char *n, Protection p, Specifier v, const char *t) :
      classDef(cd), usedName(n), prot(p), virt(v), templSpecifiers(t)
   {}

   /** Class definition which this relation inherits from. */
   ClassDef *classDef;

   /** name used in the inheritance list
    * (may be a typedef name instead of the class name)
    */
   QByteArray usedName;

   /** Protection level of the inheritance relation:
    *  Public, Protected, or Private
    */
   Protection prot;

   /** Virtualness of the inheritance relation:
    *  Normal, or Virtual
    */
   Specifier virt;

   /** Template arguments used for the base class */
   QByteArray templSpecifiers;
};

#endif
