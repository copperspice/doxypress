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

#ifndef CONCEPTDEF_H
#define CONCEPTDEF_H

#include <arguments.h>
#include <definition.h>
#include <filedef.h>

class OutputList;

class ConceptDef : public Definition
{
   public:
      ConceptDef(const QString &defFileName, int defLine, int defColumn, const QString &fullName,
            const QString &tag = QString(), const QString &tagFileName = QString());

      ~ConceptDef();

      QString anchor() const override {
         return QString();
      }

      QString conceptDecl() const;

      const ArgumentList &getTemplateArgumentList() const;

      DefType definitionType() const override {
         return TypeConcept;
      }

      QSharedPointer<FileDef> getFileDef() const {
         return m_fileDef;
      }

      const IncludeInfo &getIncludeInfo() const {
         return m_incInfo;
      }

      const QString &getInitializer() const {
         return m_initializer;
      }

      QString getOutputFileBase() const override;

      bool isLinkable() const override {
         return isLinkableInProject() || isReference();
      }

      bool isLinkableInProject() const override {
         return ! isReference();
      }

      void setFileDef(QSharedPointer<FileDef> fd) {
         m_fileDef = fd;
      }

      void setMemberGroupId(int id) {
         m_groupId = id;
      }

      void setInitializer(const QString &str) {
         m_initializer = str;
      }

      void setTemplateArgumentList(const ArgumentList &al);

      QString title() const {
         return m_title;
      }

      void setIncludeFile(QSharedPointer<FileDef> fd, const QString &incName, bool local, bool force);
      void writeDeclarationLink(OutputList &ol, bool &found, const QString &header, bool localNames);
      void writeMoreLink(OutputList &ol, const QString &anchor);

/*
      void setLocalToc(const LocalToc &tl);

      LocalToc localToc() const {
         return m_localToc;
      }

      void writeTagFile(QTextStream &);
*/

   QString displayName(bool = true) const override {
      return ! m_title.isEmpty() ? m_title : Definition::name();
   }

   void writeDocumentation(OutputList &ol);
   void writeTagFile(QTextStream &tagFile);
   void findSectionsInDocumentation();
   bool hasDetailedDescription() const;

   private:
      void writeBriefDescription(OutputList &ol);
      void writeAuthorSection(OutputList &ol);
      void writeIncludeFiles(OutputList &ol);
      void writeDetailedDescription(OutputList &ol, const QString &title, const QString &anchor);
      void writeDetailedDocumentationBody(OutputList &ol);

      QString m_fileName;
      QString m_title;
      QString m_initializer;

      int m_groupId;

      // Template arguments for this concept
      ArgumentList m_templateArgs;

      QSharedPointer<FileDef> m_fileDef;

      // Include information about the header file
      IncludeInfo m_incInfo;

      // LocalToc m_localToc;
};

#endif
