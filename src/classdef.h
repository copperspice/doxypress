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

#ifndef CLASSDEF_H
#define CLASSDEF_H

#include <QList>
#include <QHash>
#include <QSet>
#include <QTextStream>

#include <arguments.h>
#include <definition.h>
#include <entry.h>
#include <example.h>
#include <groupdef.h>
#include <memberdef.h>
#include <memberlist.h>
#include <membergroup.h>
#include <membername.h>
#include <namespacedef.h>
#include <outputlist.h>

class ConstraintClassDef;
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
    *                   This overwrites the file DoxyPress normally
    *                   generates based on the compound type & name.
    *  \param isSymbol  If true this class name is added as a publicly
    *                   visible (and referencable) symbol.
    *  \param isJavaEnum If true this class is actually a Java enum.
    *                    I didn't add this to CompoundType to avoid having
    *                    to adapt all translators.
    */
   ClassDef(const QString &fileName, int startLine, int startColumn, const QString &name, CompoundType ct,
            const QString &ref = QString(), const QString &fName = QString(), bool isSymbol = true, bool isJavaEnum = false);
  
   /** Destroys a compound definition. */
   ~ClassDef();

   /** Used for RTTI, this is a class */
   DefType definitionType() const override {
      return TypeClass;
   }

   /** Returns the unique base name (without extension) of the class's file on disk */
   QString getOutputFileBase() const;
   QString getInstanceOutputFileBase() const;
   QString getFileBase() const;

   /** Returns the base name for the source code file */
   QString getSourceFileBase() const override;

   /** If this class originated from a tagfile, this will return the tag file reference */
   QString getReference() const override;

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
   QString compoundTypeString() const;

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
   QSharedPointer<NamespaceDef> getNamespaceDef() const;

   /** Returns the file in which this compound's definition can be found.
    *  Should not return 0 (but it might be a good idea to check anyway).
    */
   QSharedPointer<FileDef> getFileDef() const;

   /** Returns the Java package this class is in or 0 if not applicable.
    */

   QSharedPointer<MemberDef> getMemberByName(const QString &) const;

   /** Returns true iff \a bcd is a direct or indirect base class of this
    *  class. This function will recusively traverse all branches of the
    *  inheritance tree.
    */
   bool isBaseClass(QSharedPointer<ClassDef> bcd, bool followInstances, int level = 0);

   /** Returns true iff \a bcd is a direct or indirect sub class of this
    *  class.
    */
   bool isSubClass(QSharedPointer<ClassDef> bcd, int level = 0);

   /** returns true iff \a md is a member of this class or of the
    *  the public/protected members of a base class
    */
   bool isAccessibleMember(QSharedPointer<MemberDef> md);

   /** Returns a sorted dictionary with all template instances found for
    *  this template class. Returns 0 if not a template or no instances.
    */
   QHash<QString, QSharedPointer<ClassDef>> *getTemplateInstances() const;

   /** Returns the template master of which this class is an instance.
    *  Returns 0 if not applicable.
    */
   QSharedPointer<ClassDef> templateMaster() const;

   /** Returns true if this class is a template */
   bool isTemplate() const;

   IncludeInfo *includeInfo() const;

   UsesClassDict *usedImplementationClasses() const;
   UsesClassDict *usedByImplementationClasses() const;
   UsesClassDict *usedInterfaceClasses() const;

   QHash<QString, QSharedPointer<ConstraintClassDef>> templateTypeConstraints() const;

   bool isTemplateArgument() const;

   /** Returns the definition of a nested compound if
    *  available, or 0 otherwise.
    *  @param name The name of the nested compound
    */
   virtual QSharedPointer<Definition> findInnerCompound(const QString &name) override;

   /** Returns the template parameter lists that form the template
    *  declaration of this class.
    *
    *  Example: <code>template<class T> class TC {};</code>
    *  will return a list with one ArgumentList containing one argument
    *  with type="class" and name="T".
    */
   void getTemplateParameterLists(QList<ArgumentList> &lists) const;

   QString qualifiedNameWithTemplateParameters(QList<ArgumentList> *actualParams = 0, int *actualParamIndex = 0) const;

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
   QSharedPointer<ClassDef> categoryOf() const;

   /** Returns the name of the class including outer classes, but not
    *  including namespaces.
    */
   QString className() const;

   /** Returns the members in the list identified by \a lt */
   QSharedPointer<MemberList> getMemberList(MemberListType lt);

   /** Returns the list containing the list of members sorted per type */
   const QList<QSharedPointer<MemberList>> &getMemberLists() const;

   /** Returns the member groups defined for this class */
   MemberGroupSDict *getMemberGroupSDict() const;

   QHash<QString, int> *getTemplateBaseClassNames() const;

   QSharedPointer<ClassDef> getVariableInstance(const char *templSpec);

   bool isUsedOnly() const;

   QString anchor() const;
   bool isEmbeddedInOuterScope() const;

   bool isSimple() const;

   const SortedList<QSharedPointer<ClassDef>>   *taggedInnerClasses() const;
   QSharedPointer<ClassDef> tagLessReference() const;

   QSharedPointer<MemberDef> isSmartPointer() const;

   bool isJavaEnum() const;

   bool isGeneric() const;
   bool isAnonymous() const;
   const ClassSDict *innerClasses() const;
   QString title() const;

   QString generatedFromFiles() const;
   const FileList &usedFiles() const;

   const ArgumentList *typeConstraints() const;
   const ExampleSDict *exampleList() const;
   bool hasExamples() const;
   QString getMemberListFileName() const;
   bool subGrouping() const;

   void insertBaseClass(QSharedPointer<ClassDef> cd, const QString &name, Protection p, Specifier s, const QString &t = QString() );
   void insertSubClass(QSharedPointer<ClassDef> cd, Protection p, Specifier s, const QString &t = QString() );
   void setIncludeFile(QSharedPointer<FileDef> fd, const char *incName, bool local, bool force);
   void insertMember(QSharedPointer<MemberDef> );
   void insertUsedFile(QSharedPointer<FileDef> fd);
   bool addExample(const char *anchor, const char *name, const char *file);
   void mergeCategory(QSharedPointer<ClassDef> category);
   void setNamespace(QSharedPointer<NamespaceDef> nd);
   void setFileDef(QSharedPointer<FileDef> fd);
   void setSubGrouping(bool enabled);
   void setProtection(Protection p);

   void setGroupDefForAllMembers(QSharedPointer<GroupDef> g, Grouping::GroupPri_t pri, const QString &fileName, 
                  int startLine, bool hasDocs);

   void addInnerCompound(QSharedPointer<Definition> d) override; 

   QSharedPointer<ClassDef> insertTemplateInstance(const QString &fileName, int startLine, int startColumn,
                  const QString &templSpec, bool &freshInstance);

   void addUsedClass(QSharedPointer<ClassDef> cd, const QString &accessName, Protection prot);
   void addUsedByClass(QSharedPointer<ClassDef> cd, const QString &accessName, Protection prot);
   void setIsStatic(bool b);
   void setCompoundType(CompoundType t);
   void setClassName(const QString &name);
   void setClassSpecifier(Entry::SpecifierFlags spec);

   void setTemplateArguments(ArgumentList *al);
   void setTemplateBaseClassNames(QHash<QString, int> *templateNames);
   void setTemplateMaster(QSharedPointer<ClassDef> tm);
   void setTypeConstraints(ArgumentList *al);
   void addMembersToTemplateInstance(QSharedPointer<ClassDef> cd, const char *templSpec);
   void makeTemplateArgument(bool b = true);
   void setCategoryOf(QSharedPointer<ClassDef> cd);
   void setUsedOnly(bool b);

   void addTaggedInnerClass(QSharedPointer<ClassDef> cd);
   void setTagLessReference(QSharedPointer<ClassDef> cd);
   void setName(const QString &name);

   // actions
   void findSectionsInDocumentation();
   void addMembersToMemberGroup();
   void addListReferences();
   void addTypeConstraints();
   void computeAnchors();
   void mergeMembers();
   void distributeMemberGroupDocumentation();
   void writeDocumentation(OutputList &ol);
   void writeDocumentationForInnerClasses(OutputList &ol);
   void writeMemberPages(OutputList &ol);
   void writeMemberList(OutputList &ol);

   void writeDeclaration(OutputList &ol, QSharedPointer<MemberDef> md, bool inGroup, 
                  QSharedPointer<ClassDef> inheritedFrom, const QString &inheritId);

   void writeQuickMemberLinks(OutputList &ol, MemberDef *md) const;
   void writeSummaryLinks(OutputList &ol);
   void reclassifyMember(QSharedPointer<MemberDef> md, MemberType t);
   void writeInlineDocumentation(OutputList &ol);
   void writeDeclarationLink(OutputList &ol, bool &found, const QString &header, bool localNames);
   void removeMemberFromLists(QSharedPointer<MemberDef> md);
   void addGroupedInheritedMembers(OutputList &ol, MemberListType lt, QSharedPointer<ClassDef> inheritedFrom, const QString &inheritId);
   int countMembersIncludingGrouped(MemberListType lt, QSharedPointer<ClassDef> inheritedFrom, bool additional);
   int countInheritanceNodes();
   void writeTagFile(QTextStream &);

   bool visited;

 protected:
   void addUsedInterfaceClasses(QSharedPointer<MemberDef> md, const char *typeStr);
   bool hasNonReferenceSuperClass();
   void showUsedFiles(OutputList &ol);

 private:
   void writeDocumentationContents(OutputList &ol, const QString  &pageTitle);
   void internalInsertMember(QSharedPointer<MemberDef> md, Protection prot, bool addToAllList);
   void addMemberToList(MemberListType lt, QSharedPointer<MemberDef> md, bool isBrief);

   QSharedPointer<MemberList> createMemberList(MemberListType lt);

   void writeInheritedMemberDeclarations(OutputList &ol, MemberListType lt, int lt2,
                  const QString &title, QSharedPointer<ClassDef> inheritedFrom,
                  bool invert, bool showAlways, QHash<void *, void *> *visitedClasses);

   void writeMemberDeclarations(OutputList &ol, MemberListType lt, const QString &title,
                  const QString &subTitle = 0, bool showInline = false,
                  QSharedPointer<ClassDef> inheritedFrom = QSharedPointer<ClassDef>(), int lt2 = -1, bool invert = false,
                  bool showAlways = false, QHash<void *, void *> *visitedClasses = 0);

   void writeMemberDocumentation(OutputList &ol, MemberListType lt, const QString &title, bool showInline = false);
   void writeSimpleMemberDocumentation(OutputList &ol, MemberListType lt);
   void writePlainMemberDeclaration(OutputList &ol, MemberListType lt, bool inGroup, QSharedPointer<ClassDef> inheritedFrom, 
                  const QString &inheritId);

   void writeBriefDescription(OutputList &ol, bool exampleFlag);
   void writeDetailedDescription(OutputList &ol, const QString &pageType, bool exampleFlag,
                  const QString &title, const QString &anchor = QString());

   void writeIncludeFiles(OutputList &ol);
   //void writeAllMembersLink(OutputList &ol);
   void writeInheritanceGraph(OutputList &ol);
   void writeCollaborationGraph(OutputList &ol);
   void writeMemberGroups(OutputList &ol, bool showInline = false);
   void writeNestedClasses(OutputList &ol, const QString &title);
   void writeInlineClasses(OutputList &ol);
   void startMemberDeclarations(OutputList &ol);
   void endMemberDeclarations(OutputList &ol);
   void startMemberDocumentation(OutputList &ol);
   void endMemberDocumentation(OutputList &ol);
   void writeAuthorSection(OutputList &ol);
   void writeMoreLink(OutputList &ol, const QString &anchor);
   void writeDetailedDocumentationBody(OutputList &ol);

   int countAdditionalInheritedMembers();
   void writeAdditionalInheritedMembers(OutputList &ol);
   void addClassAttributes(OutputList &ol);
   int countMemberDeclarations(MemberListType lt, QSharedPointer<ClassDef> inheritedFrom,
                               int lt2, bool invert, bool showAlways, QHash<void *, void *> *visitedClasses);

   int countInheritedDecMembers(MemberListType lt, QSharedPointer<ClassDef> inheritedFrom, bool invert, bool showAlways,
                                QHash<void *, void *> *visitedClasses);

   void getTitleForMemberListType(MemberListType type, QString &title, QString &subtitle);
   QString includeStatement() const;

   void addTypeConstraint(const QString &typeConstraint, const QString &type);
  
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
   QSharedPointer<NamespaceDef> m_nspace;

   /*! File this class is defined in */
   QSharedPointer<FileDef> m_fileDef;

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

   /*! The inner classes contained in this class. Will be 0 if there are  no inner classes.
    */
   ClassSDict *m_innerClasses;

   /* classes for the collaboration diagram */
   UsesClassDict *m_usesImplClassDict;
   UsesClassDict *m_usedByImplClassDict;
   UsesClassDict *m_usesIntfClassDict;

   QHash<QString, QSharedPointer<ConstraintClassDef>> m_constraintClassDict;

   /*! Template instances that exists of this class, the key in the
    *  dictionary is the template argument list.
    */
   QHash<QString, QSharedPointer<ClassDef>> *m_templateInstances;

   /*! Template instances that exists of this class, as defined by variables.
    *  We do NOT want to document these individually. The key in the
    *  dictionary is the template argument list.
    */
   QHash<QString, QSharedPointer<ClassDef>> *m_variableInstances;

   QHash<QString, int> *m_templBaseClassNames;

   /*! The class this class is an instance of. */
   QSharedPointer<ClassDef> m_templateMaster;

   /*! local class name which could be a typedef'ed alias name. */
   QString m_className;

   /*! If this class is a Objective-C category, then this points to the
    *  class which is extended.
    */
   QSharedPointer<ClassDef> m_categoryOf;

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
   QSharedPointer<MemberDef> m_arrowOperator;

   SortedList<QSharedPointer<ClassDef>> *m_taggedInnerClasses;
   QSharedPointer<ClassDef> m_tagLessRef;

   /** Does this class represent a Java style enum? */
   bool m_isJavaEnum;

   bool m_isGeneric;
   bool m_isAnonymous;

   Entry::SpecifierFlags m_spec;
};

/** Class that contains information about a usage relation.
 */
class UsesClassDef 
{
 public:
   UsesClassDef(QSharedPointer<ClassDef> cd) : m_classDef(cd) {      
      m_containment = true;
   }

   ~UsesClassDef() 
   { }

   void addAccessor(const QString &s) {
      if (m_accessors.contains(s)) {
         m_accessors.insert(s);        
      }
   }

   /** Class definition this relation uses. */
   QSharedPointer<ClassDef> m_classDef;

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
   BaseClassDef(QSharedPointer<ClassDef> cd, const QString &n, Protection p, Specifier v, const QString &t) 
      : classDef(cd), usedName(n), prot(p), virt(v), templSpecifiers(t)
   {}

   /** Class definition which this relation inherits from. */
   QSharedPointer<ClassDef> classDef;

   /** name used in the inheritance list
    * (may be a typedef name instead of the class name)
    */
   QString usedName;

   /** Protection level of the inheritance relation:
    *  Public, Protected, or Private
    */
   Protection prot;

   /** Virtualness of the inheritance relation:
    *  Normal, or Virtual
    */
   Specifier virt;

   /** Template arguments used for the base class */
   QString templSpecifiers;
};


/** Class that contains information about a type constraint relations.
 */
struct ConstraintClassDef
{
   ConstraintClassDef(QSharedPointer<ClassDef> cd) 
      : classDef(cd) {}

   void addAccessor(const QString &s)  {
      if (! m_accessors.contains(s)) {
         m_accessors.insert(s);
      }
   }

  /** Class definition that this relation uses. */
  QSharedPointer<ClassDef> classDef;

  /** Dictionary of member types names that form the edge labels of the
   *  constraint relation.
   */
  QSet<QString> m_accessors;
};

#endif
