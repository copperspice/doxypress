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

#ifndef LAYOUT_H
#define LAYOUT_H

#include <types.h>

#include <QList>
#include <QTextStream>

class LayoutParser;
class MemberList;

/** @brief Base class representing a piece of a documentation page */
struct LayoutDocEntry {
   virtual ~LayoutDocEntry()
   { }

   enum Kind {
      // Generic items for all pages 0
      MemberGroups,
      MemberDeclStart, MemberDeclEnd, MemberDecl,
      MemberDefStart, MemberDefEnd, MemberDef,
      BriefDesc, DetailedDesc,
      AuthorSection,

      // Class specific items  10
      ClassIncludes, ClassInlineClasses,
      ClassInheritanceGraph, ClassNestedClasses,
      ClassCollaborationGraph, ClassAllMembersLink,
      ClassUsedFiles,

      // Concept specific items
      ConceptDefinition,
      // Namespace specific items   17
      NamespaceNestedNamespaces, NamespaceNestedConstantGroups,
      NamespaceClasses, NamespaceConcepts, NamespaceInlineClasses,

      // File specific items  21
      FileClasses, FileConcepts, FileConstantGroups, FileNamespaces,
      FileIncludes, FileIncludeGraph,
      FileIncludedByGraph, FileSourceLink,
      FileInlineClasses,

      // Group specific items   29
      GroupClasses, GroupConcepts, GroupInlineClasses, GroupNamespaces,
      GroupDirs, GroupNestedGroups, GroupFiles,
      GroupGraph, GroupPageDocs,

      // Directory specific items
      DirSubDirs, DirFiles, DirGraph

   };

   virtual Kind kind() const = 0;
};

/** @brief Represents of a piece of a documentation page without configurable parts */
struct LayoutDocEntrySimple : LayoutDocEntry {
 public:
   LayoutDocEntrySimple(Kind k)
      : m_kind(k)
   { }

   Kind kind() const override {
      return m_kind;
   }

 private:
   Kind m_kind;
};

struct LayoutDocEntrySection: public LayoutDocEntrySimple {
   LayoutDocEntrySection(Kind k, const QString &tl)
      : LayoutDocEntrySimple(k), m_title(tl)
   { }

   QString title(SrcLangExt lang) const;

 private:
   QString m_title;
};

/** @brief Represents of a member declaration list with configurable title and subtitle. */
struct LayoutDocEntryMemberDecl: public LayoutDocEntry {

   LayoutDocEntryMemberDecl(MemberListType tp, const QString &tl, const QString &ss)
      : type(tp), m_title(tl), m_subscript(ss)
   {}

   Kind kind() const  override {
      return MemberDecl;
   }

   MemberListType type;
   QString title(SrcLangExt lang) const;
   QString subtitle(SrcLangExt lang) const;

 private:
   QString m_title;
   QString m_subscript;
};

/** @brief Represents of a member definition list with configurable title. */
struct LayoutDocEntryMemberDef: public LayoutDocEntry {
   LayoutDocEntryMemberDef(MemberListType tp, const QString &tl)
      : type(tp), m_title(tl)
   {}

   Kind kind() const  override {
      return MemberDef;
   }

   MemberListType type;
   QString title(SrcLangExt lang) const;

 private:
   QString m_title;
};

/** @brief Base class for the layout of a navigation item at the top of the HTML pages. */
struct LayoutNavEntry {
 public:
   enum Kind {
      None = -1,
      MainPage,
      Pages,
      Modules,
      Namespaces,
      NamespaceList,
      NamespaceMembers,

      Classes,
      ClassList,
      ClassIndex,
      ClassHierarchy,
      ClassMembers,
      Concepts,
      ConceptList,
      Files,
      FileList,
      FileGlobals,
      FileSource,
      Examples,
      User,
      UserGroup,
      LastValue = UserGroup
   };

   LayoutNavEntry(LayoutNavEntry *parent, Kind k, bool vs, const QString &bf,
                  const QString &tl, const QString &intro)
      : m_parent(parent), m_kind(k), m_visible(vs), m_baseFile(bf), m_title(tl), m_intro(intro) {

   }

   LayoutNavEntry *parent() const   {
      return m_parent;
   }

   Kind kind() const {
      return m_kind;
   }

   QString baseFile() const {
      return m_baseFile;
   }

   QString title() const {
      return m_title;
   }

   QString intro() const {
      return m_intro;
   }

   QString url() const;
   bool visible()  {
      return m_visible;
   }

   void clear()  {
      m_children.clear();
   }

   void addChild(LayoutNavEntry *e) {
      m_children.append(e);
   }

   void prependChild(LayoutNavEntry *e) {
      m_children.prepend(e);
   }

   const QList<LayoutNavEntry *> &children() const {
      return m_children;
   }

   LayoutNavEntry *find(LayoutNavEntry::Kind k, const QString &file = QString()) const;

 private:
   LayoutNavEntry()
      : m_parent(nullptr), m_kind(Kind::None), m_visible(false)
   { }

   LayoutNavEntry *m_parent;
   Kind m_kind;
   bool m_visible;

   QString m_baseFile;
   QString m_title;
   QString m_intro;

   QList<LayoutNavEntry *> m_children;

   friend class LayoutDocManager;
};

/** @brief Singleton providing access to the (user configurable) layout of the documentation */
class LayoutDocManager
{
   public:

      enum LayoutPart {
         Class,
         Concept,
         Directory,
         File,
         Group,
         Namespace,
         Undefined = -1
      };

      // returns a reference to this singleton.
      static LayoutDocManager &instance();

      // returns the list of LayoutDocEntry's in representation order for a
      // given page identified by @a part.
      const QList<LayoutDocEntry *> &docEntries(LayoutPart part) const;

      // returns the (invisible) root of the navigation tree.
      LayoutNavEntry *rootNavEntry() const;

      // parses a user provided layout
      void parse(QTextStream &t, const QString &fileName);
      void init();

   private:
      LayoutDocManager();
      ~LayoutDocManager();

      void addEntry(LayoutPart p, LayoutDocEntry *e);
      void clear(LayoutPart p);

      QList<LayoutDocEntry *> m_docClass;
      QList<LayoutDocEntry *> m_docNamespace;
      QList<LayoutDocEntry *> m_docFile;
      QList<LayoutDocEntry *> m_docGroup;
      QList<LayoutDocEntry *> m_docDirectory;

      LayoutNavEntry *m_docRootNav;

      friend class LayoutParser;
};

void writeDefaultLayoutFile(const QString &fileName);

#endif

