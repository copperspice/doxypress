/************************************************************************
*
* Copyright (c) 2014-2025 Barbara Geller & Ansel Sermersheim
*
* Copyright (c) 1997-2014 Dimitri van Heesch
*
* DoxyPress is free software. You can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
* https://www.gnu.org/licenses/
*
*************************************************************************/

#ifndef MEMBERDEF_H
#define MEMBERDEF_H

#include <definition.h>
#include <entry.h>
#include <types.h>

#include <QCryptographicHash>
#include <QList>
#include <QStringList>
#include <QTextStream>
#include <QVector>

#include <sys/types.h>

class ArgumentList;
class ClassDef;
class ExampleSDict;
class FileDef;
class GroupDef;
class GroupDef;
class MemberDefImpl;
class MemberGroup;
class MemberList;
class NamespaceDef;
class OutputList;

struct TagInfo;

/** A model of a class/file/namespace member symbol. */
class MemberDef : public Definition
{
 public:
   MemberDef(const QString &defFileName, int defLine, int defColumn, const QString &type, const QString &memberName,
             const QString &args, const QString &excp, Protection prot, Specifier virt, bool isMemberStatic,
             Relationship related, MemberDefType memberType, const ArgumentList &tal, const ArgumentList &al);

   ~MemberDef();

   DefType definitionType() const override {
      return TypeMember;
   }

   // move this member into a different scope
   QSharedPointer<MemberDef> deepCopy() const;
   void moveTo(QSharedPointer<Definition> scope);

   // link id
   QString getOutputFileBase() const override;
   QString getReference() const override;
   QString anchor() const override;

   QString declaration() const;
   QString definition() const;
   QString typeString() const;
   QString argsString() const;
   QString excpString() const;
   QString bitfieldString() const;
   QString extraTypeChars() const;
   QString getInitializer() const;

   int initializerLines() const;
   Entry::Traits getMemberTraits() const;

   QSharedPointer<MemberList> getSectionList(QSharedPointer<Definition> d) const;
   QString displayDefinition() const;

   // scope query members
   QSharedPointer<ClassDef> getClassDef() const;
   QSharedPointer<FileDef>  getFileDef() const;
   QSharedPointer<NamespaceDef> getNamespaceDef() const;
   QSharedPointer<ClassDef> accessorClass() const;

   // property get
   QString getPropertyRead() const;
   QString getPropertyWrite() const;

   // copperspice - additional properties
   QString getPropertyReset() const;
   QString getPropertyNotify() const;

   // querying the grouping definition
   QSharedPointer<GroupDef> getGroupDef() const;
   Grouping::GroupPri_t getGroupPri() const;
   QString getGroupFileName() const;
   int getGroupStartLine() const;
   bool getGroupHasDocs() const;
   QString qualifiedName() const override;
   QString objCMethodName(bool localLink, bool showStatic) const;

   // direct kind info
   Protection protection() const;
   Specifier virtualness(int count = 0) const;
   MemberDefType memberType() const;
   QString memberTypeName() const;

   // get methods
   bool isSignal() const;
   bool isSlot() const;
   bool isVariable() const;
   bool isEnumerate() const;
   bool isEnumValue() const;
   bool isTypedef() const;
   bool isFunction() const;
   bool isFunctionPtr() const;
   bool isDefine() const;
   bool isFriend() const;
   bool isDCOP() const;
   bool isProperty() const;
   bool isEvent() const;
   bool isRelated() const;
   bool isForeign() const;
   bool isStatic() const;
   bool isInline() const;
   bool isExplicit() const;
   bool isNoDiscard() const;
   bool isNoReturn() const;
   bool isDeprecated() const;
   bool isMutable() const;
   bool isGettable() const;
   bool isPrivateGettable() const;
   bool isProtectedGettable() const;
   bool isSettable() const;
   bool isPrivateSettable() const;
   bool isProtectedSettable() const;

   bool isReadable() const;
   bool isWritable() const;
   bool isNotify() const;
   bool isReset() const;
   bool isRevision() const;
   bool isDesignable() const;
   bool isScriptable() const;
   bool isStored() const;
   bool isUser() const;
   bool isConstant() const;
   bool isFinal_Property() const;

   bool isAddable() const;
   bool isFinal() const;
   bool isRemovable() const;
   bool isRaisable() const;
   bool isAbstract() const;
   bool isOverride() const;
   bool isInitonly() const;
   bool isOptional() const;
   bool isRequired() const;
   bool isNonAtomic() const;
   bool isCopy() const;
   bool isAssign() const;
   bool isRetain() const;
   bool isWeak() const;
   bool isStrong() const;
   bool isEnumStruct() const;
   bool isUnretained() const;
   bool isNew() const;
   bool isSealed() const;
   bool isImplementation() const;
   bool isExternal() const;
   bool isAlias() const;
   bool isConstExpr() const;
   bool isDefault() const;
   bool isDelete() const;
   bool isNoExcept() const;
   bool isAttribute() const;            // UNO IDL attribute
   bool isUNOProperty() const;          // UNO IDL property
   bool isReadonly() const;
   bool isBound() const;
   bool isConstrained() const;
   bool isTransient() const;
   bool isMaybeVoid() const;
   bool isMaybeDefault() const;
   bool isMaybeAmbiguous() const;
   bool isPublished() const;            // UNO IDL published
   bool isTemplateSpecialization() const;
   bool hasDocumentedParams() const;
   bool hasDocumentedReturnType() const;
   bool isObjCMethod() const;
   bool isObjCProperty() const;
   bool isConstructor() const;
   bool isDestructor() const;
   bool hasOneLineInitializer() const;
   bool hasMultiLineInitializer() const;
   bool protectionVisible() const;
   bool showInCallGraph() const;
   bool isStrongEnumValue() const;

   // derived get methods
   bool isFriendToHide() const;
   bool isNotFriend() const;
   bool isFunctionOrSignalSlot() const;
   bool isRelatedOrFriend() const;

   //
   void setUsingDeclaration(bool enable);
   bool isUsingDeclaration() const;

   // output info
   bool isAnonymous() const;

   bool isLinkableInProject() const override;
   bool isLinkable() const override;

   bool hasDocumentation() const override;
   bool isDeleted() const;

   bool isBriefSectionVisible() const;
   bool isDetailedSectionVisible(bool inGroup, bool inFile) const;
   bool isDetailedSectionLinkable() const;
   bool isFriendClass() const;
   bool isDocumentedFriendClass() const;

   QSharedPointer<MemberDef>  reimplements() const;
   QSharedPointer<MemberList> reimplementedBy() const;
   bool isReimplementedBy(QSharedPointer<ClassDef> cd) const;

   QSharedPointer<ClassDef> relatedAlso() const;

   bool hasDocumentedEnumValues() const;
   QSharedPointer<MemberDef> getAnonymousEnumType() const;
   bool isDocsForDefinition() const;
   QSharedPointer<MemberDef> getEnumScope() const;
   QSharedPointer<MemberList> enumFieldList() const;

   void setEnumBaseType(const QString &type);
   QString enumBaseType() const;

   bool hasExamples();
   ExampleSDict *getExamples() const;
   bool isPrototype() const;

   // argument related members
   void setArgumentList(const ArgumentList &al);
   void setDeclArgumentList(const ArgumentList &al);
   void setTypeConstraints(const ArgumentList &al);
   void setTemplateParameterLists(const QVector<ArgumentList> &lists);

   const ArgumentList &getArgumentList() const;
   ArgumentList &getArgumentList();

   const ArgumentList &getDeclArgumentList() const;

   const ArgumentList &getTemplateArgumentList() const;
   ArgumentList &getTemplateArgumentList();

   const ArgumentList &getTypeConstraints() const;

   const QVector<ArgumentList> &getTemplateParameterLists() const;

   // member group related members
   int getMemberGroupId() const;
   QSharedPointer<MemberGroup> getMemberGroup() const;

   bool fromAnonymousScope() const;
   bool anonymousDeclShown() const;
   QSharedPointer<MemberDef> fromAnonymousMember() const;

   // callgraph related members
   bool hasCallGraph() const;
   bool hasCallerGraph() const;
   bool visibleMemberGroup(bool hideNoHeader);

   // referenced related members
   bool hasReferencesRelation() const;
   bool hasReferencedByRelation() const;

   QSharedPointer<MemberDef> templateMaster() const;
   QString getScopeString() const;
   QSharedPointer<ClassDef> getClassDefOfAnonymousType();

   // cached typedef functions
   bool isTypedefValCached() const;
   QSharedPointer<ClassDef> getCachedTypedefVal() const;
   QString getCachedTypedefTemplSpec() const;
   QString getCachedResolvedTypedef() const;

   QSharedPointer<MemberDef> memberDefinition() const;
   QSharedPointer<MemberDef> memberDeclaration() const;
   QSharedPointer<MemberDef> inheritsDocsFrom() const;
   QSharedPointer<MemberDef> getGroupAlias() const;

   QSharedPointer<ClassDef> category() const;
   QSharedPointer<MemberDef> categoryRelation() const;

   QString displayName(bool = true) const override;
   QString getDeclType() const;

   void getLabels(QStringList &sl, QSharedPointer<Definition> container) const;

   //
   QString documentation() const override;
   QString briefDescription(bool abbr = false) const override;
   QString fieldType() const;
   bool isReference() const override;

   QString getDeclFileName() const;
   void setDeclFile(const QString &df, int line, int column);

   int getDeclLine() const;
   int getDeclColumn() const;

   void setMemberType(MemberDefType t);
   void setDefinition(const QString &d);
   void setFileDef(QSharedPointer<FileDef> fd);
   void setAnchor();
   void setProtection(Protection p);

   void setMemberTraits(Entry::Traits traits);
   void mergeMemberTraits(Entry::Traits traits);

   void setInitializer(const QString &i);
   void setBitfields(const QString &s);
   void setMaxInitLines(int lines);
   void setMemberClass(QSharedPointer<ClassDef> cd);
   void setSectionList(QSharedPointer<Definition> d, QSharedPointer<MemberList> sl);

   void setGroupDef(QSharedPointer<GroupDef> gd, Grouping::GroupPri_t pri,
                    const QString &fileName, int startLine, bool hasDocs,
                    QSharedPointer<MemberDef> member = QSharedPointer<MemberDef>());

   void setExplicitExternal(bool b);


   void setName(const QString &name) override;

   // property set
   void setPropertyRead(const QString &data);
   void setPropertyWrite(const QString &data);

   // copperspice - additional properties
   void setPropertyReset(const QString &data);
   void setPropertyNotify(const QString &data);

   void setTemplateSpecialization(bool b);

   void setFriend(bool isFriend);
   void makeFriend();
   void makeRelated();
   void makeForeign();
   void setHasDocumentedParams(bool b);
   void setHasDocumentedReturnType(bool b);
   void setInheritsDocsFrom(QSharedPointer<MemberDef> md);
   void setTagInfo(const TagInfo &tag);
   void setArgsString(const QString &as);

   // relation to other members
   void setReimplements(QSharedPointer<MemberDef> md);
   void insertReimplementedBy(QSharedPointer<MemberDef> md);

   void setRelatedAlso(QSharedPointer<ClassDef> cd);

   // enumeration specific members
   void insertEnumField(QSharedPointer<MemberDef> md);
   void setEnumScope(QSharedPointer<MemberDef> md, bool livesInsideEnum = false);
   void setEnumClassScope(QSharedPointer<ClassDef> cd);
   void setDocumentedEnumValues(bool value);
   void setAnonymousEnumType(QSharedPointer<MemberDef> md);

   // example related members
   bool addExample(const QString &anchor, const QString &name, const QString &file);

   // prototype related members
   void setPrototype(bool isProto, const QString &df, int line, int column);

   //
   void setType(const QString &t);
   void setAccessorType(QSharedPointer<ClassDef> cd, const QString &t);

   // namespace related members
   void setNamespace(QSharedPointer<NamespaceDef> nd);

   // member group related members
   void setMemberGroup(QSharedPointer<MemberGroup> grp);
   void setMemberGroupId(int id);
   void makeImplementationDetail();

   // anonymous scope members
   void setFromAnonymousScope(bool b);
   void setFromAnonymousMember(QSharedPointer<MemberDef> m);

   void enableCallGraph(bool e);
   void enableCallerGraph(bool e);

   void enableReferencedByRelation(bool e);
   void enableReferencesRelation(bool e);
   void setTemplateMaster(QSharedPointer<MemberDef> mt);
   void addListReference(QSharedPointer<Definition> d);
   void setDocsForDefinition(bool b);
   void setGroupAlias(QSharedPointer<MemberDef> md);

   void cacheTypedefVal(QSharedPointer<ClassDef> val, const QString &templSpec, const QString &resolvedType);
   void invalidateTypedefValCache();

   void invalidateCachedArgumentTypes();

   // declaration <-> definition relation
   void setMemberDefinition(QSharedPointer<MemberDef> md);
   void setMemberDeclaration(QSharedPointer<MemberDef> md);

   void setAnonymousUsed();
   void copyArgumentNames(QSharedPointer<MemberDef> bmd);

   void setCategory(QSharedPointer<ClassDef> cd);
   void setCategoryRelation(QSharedPointer<MemberDef> md);

   void setDocumentation(const QString &d, const QString &docFile, int docLine,
            bool stripWhiteSpace = true, bool atTop = false) override;

   void setBriefDescription(const QString &b, const QString &briefFile, int briefLine)  override;
   void setInbodyDocumentation(const QString &d, const QString &inbodyFile, int inbodyLine) override;

   void setHidden(bool b) override;
   void findSectionsInDocumentation();

   // output generation
   void writeTemplatePrefix(OutputList &ol, QSharedPointer<Definition> def,
         const ArgumentList &al, bool writeRequiresClause = true);

   void writeDeclaration(OutputList &ol, QSharedPointer<ClassDef> cd, QSharedPointer<NamespaceDef> nd,
                  QSharedPointer<FileDef> fd, QSharedPointer<GroupDef> gd,
                  bool inGroup, QSharedPointer<ClassDef> inheritFrom = QSharedPointer<ClassDef>(),
                  const QString &inheritId = QString());

   void writeDocumentation(QSharedPointer<MemberList> ml, int memCount, int memTotal, OutputList &ol, const QString &scopeName,
                  QSharedPointer<Definition> container,
                  bool inGroup, bool showEnumValues = false, bool showInline = false);

   void writeMemberDocSimple(OutputList &ol, QSharedPointer<Definition> container);

   void writeEnumDeclaration(OutputList &typeDecl, QSharedPointer<ClassDef> cd,
                  QSharedPointer<NamespaceDef> nd, QSharedPointer<FileDef> fd, QSharedPointer<GroupDef> gd);

   void writeTagFile(QTextStream &);

   void warnIfUndocumented();
   void warnIfUndocumentedParams();
   QString removeReturnKeywords();

   QSharedPointer<MemberDef> createTemplateInstanceMember(const ArgumentList &formalArgs, const ArgumentList &actualArgs);

   bool visited;

 protected:
   void flushToDisk() const;
   void loadFromDisk() const;

 private:
   MemberDef(const MemberDef &);

   void lock() const;
   void unlock() const;
   void saveToDisk() const;
   void makeResident() const;

   void computeLinkableInProject() const;
   void computeIsConstructor() const;
   void computeIsDestructor() const;

   void writeGroupInclude(OutputList &ol, bool inGroup);
   void writeCallGraph(OutputList &ol);
   void writeCallerGraph(OutputList &ol);

   void writeReimplements(OutputList &ol);
   void writeReimplementedBy(OutputList &ol);

   void writeExamples(OutputList &ol);
   void writeTypeConstraints(OutputList &ol);

   void writeEnumValues(OutputList &ol, QSharedPointer<Definition> container,
                  const QString &cfname, const QString &ciname, const QString &cname);

   void writeCategoryRelation(OutputList &ol);
   void writeTagData(const DefType);

   void addToSearchIndex();

   void writeLink(OutputList &ol, QSharedPointer<ClassDef> cd, QSharedPointer<NamespaceDef> nd,
                  QSharedPointer<FileDef> fd, QSharedPointer<GroupDef> gd, bool onlyText = false);

   static int s_indentLevel;

   // unsure if this is needed
   MemberDef &operator=(const MemberDef &);

   QScopedPointer<MemberDefImpl> m_impl;

   mutable uchar m_isLinkableCached;       // 0 = not cached, 1 = false, 2 = true
   mutable uchar m_isConstructorCached;    // 0 = not cached, 1 = false, 2 = true
   mutable uchar m_isDestructorCached;     // 0 = not cached, 1 = false, 2 = true

   bool m_isAnonymous;
   bool m_usingDeclaration;
};

void combineDeclarationAndDefinition(QSharedPointer<MemberDef> mdec, QSharedPointer<MemberDef> mdef);

#endif
