/************************************************************************
*
* Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
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

#ifndef INDEX_H
#define INDEX_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QSharedPointer>

enum class DirType {
   None,
   File,
   FileSource,
};

class Definition;
class MemberDef;
class OutputList;

// Abstract interface for index generators
class IndexIntf
{
 public:
   virtual ~IndexIntf() {}

   virtual void initialize() = 0;
   virtual void finalize()   = 0;
   virtual void incContentsDepth() = 0;
   virtual void decContentsDepth() = 0;

   virtual void addContentsItem(bool isDir, const QString &name, const QString &ref, const QString &file,
            const QString &anchor, bool addToNavIndex, QSharedPointer<Definition> def, DirType category) = 0;

   virtual void addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md,
            const QString &sectionAnchor, const QString &title) = 0;

   virtual void addIndexFile(const QString &name) = 0;
   virtual void addImageFile(const QString &name) = 0;
   virtual void addStyleSheetFile(const QString &name) = 0;
};

//  implements all methods of IndexIntf and forwards the calls to all items in the list
class IndexList : public IndexIntf
{
 public:
   IndexList() {
      m_enabled = true;
   }

   void addIndex(QSharedPointer<IndexIntf> intf) {
      m_intfs.append(intf);
   }

   void clear() {
      m_intfs.clear();
   }

   void disable() {
      m_enabled = false;
   }

   void enable() {
      m_enabled = true;
   }

   bool isEnabled() const {
      return m_enabled;
   }

   void initialize() override  {
      call_forEach(&IndexIntf::initialize);
   }

   void finalize() override  {
      call_forEach(&IndexIntf::finalize);
   }

   void incContentsDepth() override  {
      if (m_enabled) {
         call_forEach(&IndexIntf::incContentsDepth);
      }
   }

   void decContentsDepth() override {
      if (m_enabled) {
         call_forEach(&IndexIntf::decContentsDepth);
      }
   }

   void addContentsItem(bool isDir, const QString &name, const QString &ref, const QString &file, const QString &anchor,
                  bool addToNavIndex = false, QSharedPointer<Definition> def = QSharedPointer<Definition>(),
                  DirType category = DirType::None) override {

      if (m_enabled)  {
         for (auto item : m_intfs) {
            item->addContentsItem(isDir, name, ref, file, anchor, addToNavIndex, def, category);
         }
      }
   }

   void addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md,
                  const QString &sectionAnchor = QString(), const QString &title = QString()) override {

      if (m_enabled) {
         for (auto item : m_intfs) {
            item->addIndexItem(context, md, sectionAnchor, title);
         }

      }
   }

   void addIndexFile(const QString &name)  override {
      if (m_enabled) {
         call_forEach<const QString &>(&IndexIntf::addIndexFile, name);
      }
   }

   void addImageFile(const QString &name) override {
      if (m_enabled) {
         call_forEach<const QString &>(&IndexIntf::addImageFile, name);
      }
   }

   void addStyleSheetFile(const QString &name) override {
      if (m_enabled) {
         call_forEach<const QString &>(&IndexIntf::addStyleSheetFile, name);
      }
   }

 private:
   void call_forEach(void (IndexIntf::*methodPtr)()) {
      for (auto item : m_intfs) {
         ((*item).*methodPtr)();
      }
   }

   template<typename A1>
   void call_forEach(void (IndexIntf::*methodPtr)(A1), A1 a1) {
       for (auto item : m_intfs) {
         ((*item).*methodPtr)(a1);
      }
   }

   QList<QSharedPointer<IndexIntf>> m_intfs;
   bool m_enabled;
};

enum IndexSections {
   isTitlePageStart,
   isTitlePageAuthor,
   isMainPage,
   isModuleIndex,
   isDirIndex,
   isNamespaceIndex,
   isClassHierarchyIndex,
   isCompoundIndex,
   isFileIndex,
   isPageIndex,
   isModuleDocumentation,
   isDirDocumentation,
   isNamespaceDocumentation,
   isClassDocumentation,
   isFileDocumentation,
   isExampleDocumentation,
   isPageDocumentation,
   isPageDocumentation2,
   isEndIndex
};

enum HighlightedItem {
   HLI_None = 0,
   HLI_Main,
   HLI_Modules,
   HLI_Namespaces,
   HLI_NamespaceMembers,
   HLI_Hierarchy,
   HLI_Classes,
   HLI_Annotated,
   HLI_Concepts,
   HLI_Files,
   HLI_Functions,
   HLI_Globals,
   HLI_Pages,
   HLI_FileSource,
   HLI_Examples,
   HLI_Search,
   HLI_UserGroup,

   HLI_ClassVisible,
   HLI_ConceptVisible,
   HLI_NamespaceVisible,
   HLI_FileVisible,
};

enum ClassMemberHighlight {
   CMHL_All = 0,
   CMHL_Functions,
   CMHL_Variables,
   CMHL_Typedefs,
   CMHL_Enums,
   CMHL_EnumValues,
   CMHL_Properties,
   CMHL_Events,
   CMHL_Related,
   CMHL_Total = CMHL_Related + 1
};

enum FileMemberHighlight {
   FMHL_All = 0,
   FMHL_Functions,
   FMHL_Variables,
   FMHL_Typedefs,
   FMHL_Enums,
   FMHL_EnumValues,
   FMHL_Defines,
   FMHL_Total = FMHL_Defines + 1
};

enum NamespaceMemberHighlight {
   NMHL_All = 0,
   NMHL_Functions,
   NMHL_Variables,
   NMHL_Typedefs,
   NMHL_Enums,
   NMHL_EnumValues,
   NMHL_Total = NMHL_EnumValues + 1
};

enum ClassHighlight {
   CHL_All = 0,
   CHL_Classes,
   CHL_Structs,
   CHL_Unions,
   CHL_Interfaces,
   CHL_Protocols,
   CHL_Categories,
   CHL_Exceptions,
   CHL_Total = CHL_Exceptions + 1
};

enum IndexTotals {
   AnnotatedCount,
   HierarchyCount,
   ConceptCount,
   DocumentedDirs,
   DocumentedGroups,
   DocumentedNamespaces,
   DocumentedClassMembers,
   DocumentedFileMembers,
   DocumentedNamespaceMembers,
};

void writeGraphInfo(OutputList &ol);
void writeIndexHierarchy(OutputList &ol);

void countDataStructures();
int getCount(IndexTotals symbol);

void startTitle(OutputList &ol, const QString &fileName, QSharedPointer<Definition> def = QSharedPointer<Definition>() );
void endTitle(OutputList &ol, const QString &fileName, const QString &name);

void startFile(OutputList &ol, const QString &name, const QString &manName, const QString &title,
                  HighlightedItem hli = HLI_None, bool additionalIndices = false, const QString &altSidebarName = QString() );

void endFile(OutputList &ol, bool skipNavIndex = false, bool skipEndContents = false, const QString &navPath = QString() );

void endFileWithNavPath(QSharedPointer<Definition> d, OutputList &ol);

void initClassMemberIndices();
void initFileMemberIndices();
void initNamespaceMemberIndices();
void addClassMemberNameToIndex(QSharedPointer<MemberDef> md);
void addFileMemberNameToIndex(QSharedPointer<MemberDef> md);
void addNamespaceMemberNameToIndex(QSharedPointer<MemberDef> md);

QString fixSpaces(const QString &s);

#endif
