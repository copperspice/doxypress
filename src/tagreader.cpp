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

#include <QHash>
#include <QFileinfo>
#include <QList>
#include <QStack>
#include <QString>
#include <QStringlist>
#include <QXmlAttributes>
#include <QXmlDefaultHandler>
#include <QXmlErrorHandler>
#include <QXmlInputSource>
#include <QXmlLocator>
#include <QXmlParseException>
#include <QXmlSimpleReader>

#include <stdio.h>
#include <stdarg.h>

#include <arguments.h>
#include <classdef.h>
#include <defargs.h>
#include <doxygen.h>
#include <entry.h>
#include <filedef.h>
#include <filename.h>
#include <message.h>
#include <section.h>
#include <tagreader.h>
#include <util.h>

// must appear after the previous include - resolve soon 
#include <doxy_globals.h>

/** Information about an linkable anchor */
class TagAnchorInfo
{
 public:
   TagAnchorInfo(const QByteArray &f, const QByteArray &l, const QByteArray &t = QByteArray())
      : label(l), fileName(f), title(t) 
   {}

   QByteArray label;
   QByteArray fileName;
   QByteArray title;
};

/** List of TagAnchorInfo objects. */
class TagAnchorInfoList : public QList<TagAnchorInfo>
{
 public:
   TagAnchorInfoList() : QList<TagAnchorInfo>() 
   {     
   }

   virtual ~TagAnchorInfoList() {}
};

/** Container for enum values that are scoped within an enum */
class TagEnumValueInfo
{
 public:
   QByteArray name;
   QByteArray file;
   QByteArray anchor;
   QByteArray clangid;
};

/** Container for member specific info that can be read from a tagfile */
class TagMemberInfo
{
 public:
   TagMemberInfo() : prot(Public), virt(Normal), isStatic(false)
   {    
   }

   QByteArray type;
   QByteArray name;
   QByteArray anchorFile;
   QByteArray anchor;
   QByteArray arglist;
   QByteArray kind;
   QByteArray clangId;

   TagAnchorInfoList docAnchors;
   Protection prot;
   Specifier virt;

   bool isStatic;

   QList<TagEnumValueInfo> enumValues;
};

/** Container for class specific info that can be read from a tagfile */
class TagClassInfo
{
 public:
   enum Kind { Class, Struct, Union, Interface, Exception, Protocol, Category, Enum, Service, Singleton };

   TagClassInfo() {
      bases = 0;
      templateArguments = 0;      
      isObjC = false;
   }

   ~TagClassInfo() {
      delete bases;
      delete templateArguments;
   }

   QByteArray name;
   QByteArray filename;
   QByteArray clangId;
   TagAnchorInfoList docAnchors;
   QList<BaseInfo> *bases;
   QList<TagMemberInfo> members;
   QList<QByteArray> *templateArguments;
   QStringList classList;
   Kind kind;
   bool isObjC;
};

/** Container for namespace specific info that can be read from a tagfile */
class TagNamespaceInfo
{
 public:
   TagNamespaceInfo()
   {
   }

   QByteArray name;
   QByteArray filename;
   QByteArray clangId;
   QStringList classList;
   QStringList namespaceList;

   TagAnchorInfoList docAnchors;

   QList<TagMemberInfo> members;
};

/** Container for package specific info that can be read from a tagfile */
class TagPackageInfo
{
 public:
   TagPackageInfo() 
   {      
   }

   QByteArray name;
   QByteArray filename;

   TagAnchorInfoList docAnchors;
   QList<TagMemberInfo> members;

   QStringList classList;
};

/** Container for include info that can be read from a tagfile */
class TagIncludeInfo
{
 public:
   QByteArray id;
   QByteArray name;
   QByteArray text;

   bool isLocal;
   bool isImported;
};

/** Container for file specific info that can be read from a tagfile */
class TagFileInfo
{
 public:
   TagFileInfo() 
   {
   }

   QByteArray name;
   QByteArray path;
   QByteArray filename;

   TagAnchorInfoList docAnchors;

   QList<TagMemberInfo> members;

   QStringList classList;
   QStringList namespaceList;

   QList<TagIncludeInfo> includes;
};

/** Container for group specific info that can be read from a tagfile */
class TagGroupInfo
{
 public:
   TagGroupInfo() 
   {
   }

   QByteArray name;
   QByteArray title;
   QByteArray filename;

   TagAnchorInfoList docAnchors;

   QList<TagMemberInfo> members;

   QStringList subgroupList;
   QStringList classList;
   QStringList namespaceList;
   QStringList fileList;
   QStringList pageList;
   QStringList dirList;
};

/** Container for page specific info that can be read from a tagfile */
class TagPageInfo
{
 public:
   QByteArray name;
   QByteArray title;
   QByteArray filename;

   TagAnchorInfoList docAnchors;
};

/** Container for directory specific info that can be read from a tagfile */
class TagDirInfo
{
 public:
   QByteArray name;
   QByteArray filename;
   QByteArray path;
   QStringList subdirList;
   QStringList fileList;
   TagAnchorInfoList docAnchors;
};

/** Tag file parser.
 *
 *  Reads an XML-structured tagfile and builds up the structure in
 *  memory. The method buildLists() is used to transfer/translate
 *  the structures to the doxygen engine.
 */
class TagFileParser : public QXmlDefaultHandler
{
   enum State { Invalid,
                InClass,
                InFile,
                InNamespace,
                InGroup,
                InPage,
                InMember,
                InEnumValue,
                InPackage,
                InDir,
                InTempArgList
              };

   class StartElementHandler
   {
      typedef void (TagFileParser::*Handler)   (const QXmlAttributes &attrib);

      public:
         StartElementHandler(TagFileParser *parent, Handler h) : m_parent(parent), m_handler(h) 
         {
         }

         void operator()(const QXmlAttributes &attrib) {
            (m_parent->*m_handler)(attrib);
         }

       private:
         TagFileParser *m_parent;
         Handler m_handler;
   };

   class EndElementHandler
   {
      typedef void (TagFileParser::*Handler)();

    public:
      EndElementHandler(TagFileParser *parent, Handler h) : m_parent(parent), m_handler(h) {}
      void operator()() {
         (m_parent->*m_handler)();
      }

    private:
      TagFileParser *m_parent;
      Handler m_handler;
   };

 public:
   TagFileParser(const char *tagName) : m_startElementHandlers(), m_endElementHandlers(),m_tagName(tagName) {    
      m_curClass = 0;
      m_curFile = 0;
      m_curNamespace = 0;
      m_curPackage = 0;
      m_curGroup = 0;
      m_curPage = 0;
      m_curDir = 0;
      m_curMember = 0;
      m_curEnumValue = 0;
      m_curIncludes = 0;
      m_state = Invalid;
      m_locator = 0;
   }

   void setDocumentLocator ( QXmlLocator *locator ) {
      m_locator = locator;
   }

   void setFileName( const QString &fileName ) {
      m_inputFileName = fileName.toUtf8();
   }

   void warn(const char *fmt) {
      ::warn(m_inputFileName, m_locator->lineNumber(), fmt);
   }

   void warn(const char *fmt, const char *s) {
      ::warn(m_inputFileName, m_locator->lineNumber(), fmt, s);
   }

   void startCompound( const QXmlAttributes &attrib ) {
      m_curString = "";

      QString kind   = attrib.value("kind");
      QString isObjC = attrib.value("objc");

      if (kind == "class") {
         m_curClass = new TagClassInfo;
         m_curClass->kind = TagClassInfo::Class;
         m_state = InClass;
      } else if (kind == "struct") {
         m_curClass = new TagClassInfo;
         m_curClass->kind = TagClassInfo::Struct;
         m_state = InClass;
      } else if (kind == "union") {
         m_curClass = new TagClassInfo;
         m_curClass->kind = TagClassInfo::Union;
         m_state = InClass;
      } else if (kind == "interface") {
         m_curClass = new TagClassInfo;
         m_curClass->kind = TagClassInfo::Interface;
         m_state = InClass;
      } else if (kind == "enum") {
         m_curClass = new TagClassInfo;
         m_curClass->kind = TagClassInfo::Enum;
         m_state = InClass;
      } else if (kind == "exception") {
         m_curClass = new TagClassInfo;
         m_curClass->kind = TagClassInfo::Exception;
         m_state = InClass;
      } else if (kind == "protocol") {
         m_curClass = new TagClassInfo;
         m_curClass->kind = TagClassInfo::Protocol;
         m_state = InClass;
      } else if (kind == "category") {
         m_curClass = new TagClassInfo;
         m_curClass->kind = TagClassInfo::Category;
         m_state = InClass;
      } else if (kind == "service") {
         m_curClass = new TagClassInfo;
         m_curClass->kind = TagClassInfo::Service;
         m_state = InClass;

      } else if (kind == "singleton") {
         m_curClass = new TagClassInfo;
         m_curClass->kind = TagClassInfo::Singleton;
         m_state = InClass;

      } else if (kind == "file") {
         m_curFile = new TagFileInfo;
         m_state = InFile;

      } else if (kind == "namespace") {
         m_curNamespace = new TagNamespaceInfo;
         m_state = InNamespace;

      } else if (kind == "group") {
         m_curGroup = new TagGroupInfo;
         m_state = InGroup;

      } else if (kind == "page") {
         m_curPage = new TagPageInfo;
         m_state = InPage;

      } else if (kind == "package") {
         m_curPackage = new TagPackageInfo;
         m_state = InPackage;

      } else if (kind == "dir") {
         m_curDir = new TagDirInfo;
         m_state = InDir;

      } else {
         warn("Unknown compound attribute `%s' found\n", qPrintable(kind) );
         m_state = Invalid;
      }

      if (isObjC == "yes" && m_curClass) {
         m_curClass->isObjC = true;
      }
   }

   void endCompound() {
      switch (m_state) {
         case InClass:
            m_tagFileClasses.append(*m_curClass);
            m_curClass = 0;
            break;

         case InFile:
            m_tagFileFiles.append(*m_curFile);
            m_curFile = 0;
            break;

         case InNamespace:
            m_tagFileNamespaces.append(*m_curNamespace);
            m_curNamespace = 0;
            break;

         case InGroup:
            m_tagFileGroups.append(*m_curGroup);
            m_curGroup = 0;
            break;

         case InPage:
            m_tagFilePages.append(*m_curPage);
            m_curPage = 0;
            break;

         case InDir:
            m_tagFileDirs.append(*m_curDir);
            m_curDir = 0;
            break;

         case InPackage:
            m_tagFilePackages.append(*m_curPackage);
            m_curPackage = 0;
            break;

         default:
            warn("Tag `compound' was not expected\n");
      }
   }

   void startMember( const QXmlAttributes &attrib) {
      m_curMember = new TagMemberInfo;
      m_curMember->kind = attrib.value("kind").toUtf8();

      QByteArray protStr   = attrib.value("protection").toUtf8();
      QByteArray virtStr   = attrib.value("virtualness").toUtf8();
      QByteArray staticStr = attrib.value("static").toUtf8();

      if (protStr == "protected") {
         m_curMember->prot = Protected;

      } else if (protStr == "private") {
         m_curMember->prot = Private;

      }

      if (virtStr == "virtual") {
         m_curMember->virt = Virtual;

      } else if (virtStr == "pure") {
         m_curMember->virt = Pure;

      }

      if (staticStr == "yes") {
         m_curMember->isStatic = true;
      }

      m_stateStack.push(m_state);
      m_state = InMember;
   }

   void endMember() {
      m_state = m_stateStack.pop();

      switch (m_state) {
         case InClass:
            m_curClass->members.append(*m_curMember);
            break;

         case InFile:
            m_curFile->members.append(*m_curMember);
            break;

         case InNamespace:
            m_curNamespace->members.append(*m_curMember);
            break;

         case InGroup:
            m_curGroup->members.append(*m_curMember);
            break;

         case InPackage:
            m_curPackage->members.append(*m_curMember);
            break;

         default:
            warn("Unexpected tag `member' found\n");
            break;
      }
   }

   void startEnumValue( const QXmlAttributes &attrib) {
      if (m_state == InMember) {
         m_curString = "";
         m_curEnumValue = new TagEnumValueInfo;
         m_curEnumValue->file = attrib.value("file").toUtf8();
         m_curEnumValue->anchor = attrib.value("anchor").toUtf8();
         m_curEnumValue->clangid = attrib.value("clangid").toUtf8();

         m_stateStack.push(m_state);
         m_state = InEnumValue;

      } else {
         warn("Found enumvalue tag outside of member tag\n");
      }
   }

   void endEnumValue() {
      m_curEnumValue->name = m_curString.trimmed();

      m_state = m_stateStack.pop();

      if (m_state == InMember) {
         m_curMember->enumValues.append(*m_curEnumValue);
         m_curEnumValue = 0;
      }
   }

   void endDocAnchor() {
      switch (m_state) {
         case InClass:
            m_curClass->docAnchors.append(TagAnchorInfo(m_fileName, m_curString));
            break;
         case InFile:
            m_curFile->docAnchors.append(TagAnchorInfo(m_fileName, m_curString));
            break;
         case InNamespace:
            m_curNamespace->docAnchors.append(TagAnchorInfo(m_fileName, m_curString));
            break;
         case InGroup:
            m_curGroup->docAnchors.append(TagAnchorInfo(m_fileName, m_curString));
            break;
         case InPage:
            m_curPage->docAnchors.append(TagAnchorInfo(m_fileName, m_curString, m_title));
            break;
         case InMember:
            m_curMember->docAnchors.append(TagAnchorInfo(m_fileName, m_curString));
            break;
         case InPackage:
            m_curPackage->docAnchors.append(TagAnchorInfo(m_fileName, m_curString));
            break;
         case InDir:
            m_curDir->docAnchors.append(TagAnchorInfo(m_fileName, m_curString));
            break;
         default:
            warn("Unexpected tag `member' found\n");
            break;
      }
   }

   void endClass() {
      switch (m_state) {
         case InClass:
            m_curClass->classList.append(m_curString);
            break;
         case InFile:
            m_curFile->classList.append(m_curString);
            break;
         case InNamespace:
            m_curNamespace->classList.append(m_curString);
            break;
         case InGroup:
            m_curGroup->classList.append(m_curString);
            break;
         case InPackage:
            m_curPackage->classList.append(m_curString);
            break;
         default:
            warn("Unexpected tag `class' found\n");
            break;
      }
   }

   void endNamespace() {
      switch (m_state) {
         case InNamespace:
            m_curNamespace->classList.append(m_curString);
            break;
         case InFile:
            m_curFile->namespaceList.append(m_curString);
            break;
         case InGroup:
            m_curGroup->namespaceList.append(m_curString);
            break;
         default:
            warn("Unexpected tag `namespace' found\n");
            break;
      }
   }

   void endFile() {
      switch (m_state) {
         case InGroup:
            m_curGroup->fileList.append(m_curString);
            break;
         case InDir:
            m_curDir->fileList.append(m_curString);
            break;
         default:
            warn("Unexpected tag `file' found\n");
            break;
      }
   }

   void endPage() {
      switch (m_state) {
         case InGroup:
            m_curGroup->fileList.append(m_curString);
            break;
         default:
            warn("Unexpected tag `page' found\n");
            break;
      }
   }

   void endDir() {
      switch (m_state) {
         case InDir:
            m_curDir->subdirList.append(m_curString);
            break;
         default:
            warn("Unexpected tag `page' found\n");
            break;
      }
   }

   void startStringValue(const QXmlAttributes &) {
      m_curString = "";
   }

   void startDocAnchor(const QXmlAttributes &attrib ) {
      m_fileName = attrib.value("file").toUtf8();
      m_title = attrib.value("title").toUtf8();
      m_curString = "";
   }

   void endType() {
      if (m_state == InMember) {
         m_curMember->type = m_curString;
      } else {
         warn("Unexpected tag `type' found\n");
      }
   }

   void endName() {
      switch (m_state) {
         case InClass:
            m_curClass->name     = m_curString;
            break;
         case InFile:
            m_curFile->name      = m_curString;
            break;
         case InNamespace:
            m_curNamespace->name = m_curString;
            break;
         case InGroup:
            m_curGroup->name     = m_curString;
            break;
         case InPage:
            m_curPage->name      = m_curString;
            break;
         case InDir:
            m_curDir->name       = m_curString;
            break;
         case InMember:
            m_curMember->name    = m_curString;
            break;
         case InPackage:
            m_curPackage->name   = m_curString;
            break;
         default:
            warn("Unexpected tag `name' found\n");
            break;
      }
   }

   void startBase(const QXmlAttributes &attrib ) {
      m_curString = "";

      if (m_state == InClass && m_curClass) {
         QString protStr = attrib.value("protection");
         QString virtStr = attrib.value("virtualness");

         Protection prot = Public;
         Specifier  virt = Normal;

         if (protStr == "protected") {
            prot = Protected;

         } else if (protStr == "private") {
            prot = Private;

         }

         if (virtStr == "virtual") {
            virt = Virtual;
         }

         if (m_curClass->bases == 0) {
            m_curClass->bases = new QList<BaseInfo>;            
         }

         m_curClass->bases->append(BaseInfo(m_curString, prot, virt));

      } else {
         warn("Unexpected tag `base' found\n");
      }
   }

   void endBase() {
      if (m_state == InClass && m_curClass) {
         m_curClass->bases->last().name = m_curString;

      } else {
         warn("Unexpected tag `base' found\n");
      }
   }

   void startIncludes(const QXmlAttributes &attrib ) {
      if (m_state == InFile && m_curFile) {

         m_curIncludes = new TagIncludeInfo;
         m_curIncludes->id = attrib.value("id").toUtf8();
         m_curIncludes->name = attrib.value("name").toUtf8();
         m_curIncludes->isLocal = attrib.value("local").toUtf8() == "yes" ? true : false;
         m_curIncludes->isImported = attrib.value("imported").toUtf8() == "yes" ? true : false;

         m_curFile->includes.append(* m_curIncludes);

      } else {
         warn("Unexpected tag `includes' found\n");
      }

      m_curString = "";
   }

   void endIncludes() {
      m_curIncludes->text = m_curString;
   }

   void endTemplateArg() {
      if (m_state == InClass && m_curClass) {
         if (m_curClass->templateArguments == 0) {
            m_curClass->templateArguments = new QList<QByteArray>;            
         }

         m_curClass->templateArguments->append(QByteArray(m_curString));

      } else {
         warn("Unexpected tag `templarg' found\n");
      }
   }

   void endFilename() {
      switch (m_state) {
         case InClass:
            m_curClass->filename = m_curString;
            break;
         case InNamespace:
            m_curNamespace->filename = m_curString;
            break;
         case InFile:
            m_curFile->filename      = m_curString;
            break;
         case InGroup:
            m_curGroup->filename     = m_curString;
            break;
         case InPage:
            m_curPage->filename      = m_curString;
            break;
         case InPackage:
            m_curPackage->filename   = m_curString;
            break;
         case InDir:
            m_curDir->filename       = m_curString;
            break;
         default:
            warn("Unexpected tag `filename' found\n");
            break;
      }
   }

   void endPath() {
      switch (m_state) {
         case InFile:
            m_curFile->path          = m_curString;
            break;
         case InDir:
            m_curDir->path           = m_curString;
            break;
         default:
            warn("Unexpected tag `path' found\n");
            break;
      }
   }

   void endAnchor() {
      if (m_state == InMember) {
         m_curMember->anchor = m_curString;
      } else {
         warn("Unexpected tag `anchor' found\n");
      }
   }

   void endClangId() {
      if (m_state == InMember) {
         m_curMember->clangId = m_curString;
      } else if (m_state == InClass) {
         m_curClass->clangId =  m_curString;
      } else if (m_state == InNamespace) {
         m_curNamespace->clangId = m_curString;
      } else {
         warn("warning: Unexpected tag `anchor' found\n");
      }
   }



   void endAnchorFile() {
      if (m_state == InMember) {
         m_curMember->anchorFile = m_curString;
      } else {
         warn("Unexpected tag `anchorfile' found\n");
      }
   }

   void endArglist() {
      if (m_state == InMember) {
         m_curMember->arglist = m_curString;
      } else {
         warn("Unexpected tag `arglist' found\n");
      }
   }
   void endTitle() {
      switch (m_state) {
         case InGroup:
            m_curGroup->title     = m_curString;
            break;
         case InPage:
            m_curPage->title      = m_curString;
            break;
         default:
            warn("Unexpected tag `title' found\n");
            break;
      }
   }

   void endSubgroup() {
      if (m_state == InGroup) {
         m_curGroup->subgroupList.append(m_curString);
      } else {
         warn("Unexpected tag `subgroup' found\n");
      }
   }

   void startIgnoreElement(const QXmlAttributes &) {
   }

   void endIgnoreElement() {
   }

   bool startDocument() {
      m_state = Invalid;

      m_curClass = 0;
      m_curNamespace = 0;
      m_curFile = 0;
      m_curGroup = 0;
      m_curPage = 0;
      m_curPackage = 0;
      m_curDir = 0;

      m_startElementHandlers.insert("compound",    StartElementHandler(this, &TagFileParser::startCompound));
      m_startElementHandlers.insert("member",      StartElementHandler(this, &TagFileParser::startMember));
      m_startElementHandlers.insert("enumvalue",   StartElementHandler(this, &TagFileParser::startEnumValue));
      m_startElementHandlers.insert("name",        StartElementHandler(this, &TagFileParser::startStringValue));
      m_startElementHandlers.insert("base",        StartElementHandler(this, &TagFileParser::startBase));
      m_startElementHandlers.insert("filename",    StartElementHandler(this, &TagFileParser::startStringValue));
      m_startElementHandlers.insert("includes",    StartElementHandler(this, &TagFileParser::startIncludes));
      m_startElementHandlers.insert("path",        StartElementHandler(this, &TagFileParser::startStringValue));
      m_startElementHandlers.insert("anchorfile",  StartElementHandler(this, &TagFileParser::startStringValue));
      m_startElementHandlers.insert("anchor",      StartElementHandler(this, &TagFileParser::startStringValue));
      m_startElementHandlers.insert("clangid",     StartElementHandler(this, &TagFileParser::startStringValue));
      m_startElementHandlers.insert("arglist",     StartElementHandler(this, &TagFileParser::startStringValue));
      m_startElementHandlers.insert("title",       StartElementHandler(this, &TagFileParser::startStringValue));
      m_startElementHandlers.insert("subgroup",    StartElementHandler(this, &TagFileParser::startStringValue));
      m_startElementHandlers.insert("class",       StartElementHandler(this, &TagFileParser::startStringValue));
      m_startElementHandlers.insert("namespace",   StartElementHandler(this, &TagFileParser::startStringValue));
      m_startElementHandlers.insert("file",        StartElementHandler(this, &TagFileParser::startStringValue));
      m_startElementHandlers.insert("dir",         StartElementHandler(this, &TagFileParser::startStringValue));
      m_startElementHandlers.insert("page",        StartElementHandler(this, &TagFileParser::startStringValue));
      m_startElementHandlers.insert("docanchor",   StartElementHandler(this, &TagFileParser::startDocAnchor));
      m_startElementHandlers.insert("tagfile",     StartElementHandler(this, &TagFileParser::startIgnoreElement));
      m_startElementHandlers.insert("templarg",    StartElementHandler(this, &TagFileParser::startStringValue));
      m_startElementHandlers.insert("type",        StartElementHandler(this, &TagFileParser::startStringValue));

      m_endElementHandlers.insert("compound",      EndElementHandler(this, &TagFileParser::endCompound));
      m_endElementHandlers.insert("member",        EndElementHandler(this, &TagFileParser::endMember));
      m_endElementHandlers.insert("enumvalue",     EndElementHandler(this, &TagFileParser::endEnumValue));
      m_endElementHandlers.insert("name",          EndElementHandler(this, &TagFileParser::endName));
      m_endElementHandlers.insert("base",          EndElementHandler(this, &TagFileParser::endBase));
      m_endElementHandlers.insert("filename",      EndElementHandler(this, &TagFileParser::endFilename));
      m_endElementHandlers.insert("includes",      EndElementHandler(this, &TagFileParser::endIncludes));
      m_endElementHandlers.insert("path",          EndElementHandler(this, &TagFileParser::endPath));
      m_endElementHandlers.insert("anchorfile",    EndElementHandler(this, &TagFileParser::endAnchorFile));
      m_endElementHandlers.insert("anchor",        EndElementHandler(this, &TagFileParser::endAnchor));
      m_endElementHandlers.insert("clangid",       EndElementHandler(this, &TagFileParser::endClangId));
      m_endElementHandlers.insert("arglist",       EndElementHandler(this, &TagFileParser::endArglist));
      m_endElementHandlers.insert("title",         EndElementHandler(this, &TagFileParser::endTitle));
      m_endElementHandlers.insert("subgroup",      EndElementHandler(this, &TagFileParser::endSubgroup));
      m_endElementHandlers.insert("class"   ,      EndElementHandler(this, &TagFileParser::endClass));
      m_endElementHandlers.insert("namespace",     EndElementHandler(this, &TagFileParser::endNamespace));
      m_endElementHandlers.insert("file",          EndElementHandler(this, &TagFileParser::endFile));
      m_endElementHandlers.insert("dir",           EndElementHandler(this, &TagFileParser::endDir));
      m_endElementHandlers.insert("page",          EndElementHandler(this, &TagFileParser::endPage));
      m_endElementHandlers.insert("docanchor",     EndElementHandler(this, &TagFileParser::endDocAnchor));
      m_endElementHandlers.insert("tagfile",       EndElementHandler(this, &TagFileParser::endIgnoreElement));
      m_endElementHandlers.insert("templarg",      EndElementHandler(this, &TagFileParser::endTemplateArg));
      m_endElementHandlers.insert("type",          EndElementHandler(this, &TagFileParser::endType));

      return true;
   }

   bool startElement(const QString &, const QString &, const QString &name, const QXmlAttributes &attrib) {

      auto iter = m_startElementHandlers.find(name.toUtf8());

      if (iter != m_startElementHandlers.end())  {

         StartElementHandler &handler = *iter;
         handler(attrib);

      } else {
         warn("Unknown tag `%s' found\n", qPrintable(name)); 

      }

      return true;
   }

   bool endElement( const QString &, const QString &, const QString &name ) {
 
      auto iter = m_endElementHandlers.find(name.toUtf8());

      if (iter != m_endElementHandlers.end()) { 

         EndElementHandler &handler = *iter;
         handler();    

      } else {
         warn("Unknown tag `%s' found\n", qPrintable(name));

      }

      return true;
   }

   bool characters ( const QString &ch ) {
      m_curString += ch.toUtf8();
      return true;
   }

   void dump();
   void buildLists(Entry *root);
   void addIncludes();

 private:
   void buildMemberList(Entry *ce, QList<TagMemberInfo> &members);
   void addDocAnchors(Entry *e, const TagAnchorInfoList &l);

   QList<TagClassInfo>        m_tagFileClasses;
   QList<TagFileInfo>         m_tagFileFiles;
   QList<TagNamespaceInfo>    m_tagFileNamespaces;
   QList<TagGroupInfo>        m_tagFileGroups;
   QList<TagPageInfo>         m_tagFilePages;
   QList<TagPackageInfo>      m_tagFilePackages;
   QList<TagDirInfo>          m_tagFileDirs;

   QHash<QString, StartElementHandler>  m_startElementHandlers;
   QHash<QString, EndElementHandler>    m_endElementHandlers;

   TagClassInfo              *m_curClass;
   TagFileInfo               *m_curFile;
   TagNamespaceInfo          *m_curNamespace;
   TagPackageInfo            *m_curPackage;
   TagGroupInfo              *m_curGroup;
   TagPageInfo               *m_curPage;
   TagDirInfo                *m_curDir;
   TagMemberInfo             *m_curMember;
   TagEnumValueInfo          *m_curEnumValue;
   TagIncludeInfo            *m_curIncludes;

   QByteArray                 m_curString;
   QByteArray                 m_tagName;
   QByteArray                 m_fileName;
   QByteArray                 m_title;
   State                      m_state;

   QStack<State>              m_stateStack;

   QXmlLocator               *m_locator;
   QByteArray                 m_inputFileName;
};

/** Error handler for the XML tag file parser.
 *
 *  Basically dumps all fatal error to stderr using err().
 */
class TagFileErrorHandler : public QXmlErrorHandler
{
 public:
   virtual ~TagFileErrorHandler() {}

   bool warning( const QXmlParseException &) override {
      return false;
   }

   bool error( const QXmlParseException &) override {
      return false;
   }

   bool fatalError( const QXmlParseException &exception ) override {
      err("Fatal error at line %d column %d: %s\n",
          exception.lineNumber(), exception.columnNumber(),
          exception.message().data());
      return false;
   }

   QString errorString() const override {
      return "";
   }

 private:
   QString errorMsg;
};

/*! Dumps the internal structures. For debugging only! */
void TagFileParser::dump()
{
   msg("Result:\n");  

   //  CLASSES  
   for (auto cd : m_tagFileClasses) {
      msg("class `%s'\n",      cd.name.data());
      msg("  filename `%s'\n", cd.filename.data());

      if (cd.bases) {         
         for (auto bi : *cd.bases) {
            msg( "  base: %s \n", bi.name.data() );
         }
      }
     
      for (auto md : cd.members) {
         msg("  member:\n");
         msg("    kind: `%s'\n",    md.kind.data());
         msg("    name: `%s'\n",    md.name.data());
         msg("    anchor: `%s'\n",  md.anchor.data());
         msg("    arglist: `%s'\n", md.arglist.data());
      }
   }

   //  NAMESPACES
   for (auto nd : m_tagFileNamespaces) {
      msg("namespace `%s'\n",  nd.name.data());
      msg("  filename `%s'\n", nd.filename.data());

      QStringList::Iterator it;
      for ( it = nd.classList.begin(); it != nd.classList.end(); ++it ) {
         msg( "  class: %s \n", qPrintable((*it)) );
      }
     
      for (auto md : nd.members) {
         msg("  member:\n");
         msg("    kind: `%s'\n",    md.kind.data());
         msg("    name: `%s'\n",    md.name.data());
         msg("    anchor: `%s'\n",  md.anchor.data());
         msg("    arglist: `%s'\n", md.arglist.data());
      }
   }

   //  FILES  
   for (auto fd : m_tagFileFiles) {
      msg("file `%s'\n",       fd.name.data());
      msg("  filename `%s'\n", fd.filename.data());

      QStringList::Iterator it;
      for ( it = fd.namespaceList.begin(); it != fd.namespaceList.end(); ++it ) {
         msg( "  namespace: %s \n", qPrintable((*it)) );
      }

      for ( it = fd.classList.begin(); it != fd.classList.end(); ++it ) {
         msg( "  class: %s \n", qPrintable((*it)) );
      }
     
      for (auto md : fd.members) {
         msg("  member:\n");
         msg("    kind: `%s'\n",    md.kind.data());
         msg("    name: `%s'\n",    md.name.data());
         msg("    anchor: `%s'\n",  md.anchor.data());
         msg("    arglist: `%s'\n", md.arglist.data());
      }  

      for (auto item : fd.includes) {
         msg("  includes id: %s name: %s\n", item.id.data(), item.name.data());
      }
   }

   //  GROUPS 
   for (auto gd : m_tagFileGroups) {
      msg("group `%s'\n",      gd.name.data());
      msg("  filename `%s'\n", gd.filename.data());

      QStringList::Iterator it;
      for ( it = gd.namespaceList.begin(); it != gd.namespaceList.end(); ++it ) {
         msg( "  namespace: %s \n", qPrintable((*it)) );
      }

      for ( it = gd.classList.begin(); it != gd.classList.end(); ++it ) {
         msg( "  class: %s \n", qPrintable((*it)) );
      }

      for ( it = gd.fileList.begin(); it != gd.fileList.end(); ++it ) {
         msg( "  file: %s \n", qPrintable((*it)) );
      }

      for ( it = gd.subgroupList.begin(); it != gd.subgroupList.end(); ++it ) {
         msg( "  subgroup: %s \n",qPrintable((*it)) );
      }
      for ( it = gd.pageList.begin(); it != gd.pageList.end(); ++it ) {
         msg( "  page: %s \n", qPrintable((*it)) );
      }
   
      for (auto md : gd.members) {
         msg("  member:\n");
         msg("    kind: `%s'\n",    md.kind.data());
         msg("    name: `%s'\n",    md.name.data());
         msg("    anchor: `%s'\n",  md.anchor.data());
         msg("    arglist: `%s'\n", md.arglist.data());
      }
   }

   //  PAGES  
   for (auto pd : m_tagFilePages) {
      msg("page `%s'\n",       pd.name.data());
      msg("  title `%s'\n",    pd.title.data());
      msg("  filename `%s'\n", pd.filename.data());
   }

   //  DIRS
   for (auto dd : m_tagFileDirs) {
      msg("dir `%s'\n",    dd.name.data());
      msg("  path `%s'\n", dd.path.data());

      QStringList::Iterator it;
      for ( it = dd.fileList.begin(); it != dd.fileList.end(); ++it ) {
         msg( "  file: %s \n", qPrintable((*it)) );
      }

      for ( it = dd.subdirList.begin(); it != dd.subdirList.end(); ++it ) {
         msg( "  subdir: %s \n", qPrintable((*it)) );
      }
   }
}

void TagFileParser::addDocAnchors(Entry *e, const TagAnchorInfoList &list)
{   
   for (auto ta : list) {
      if (Doxygen::sectionDict->find(ta.label) == 0) {
         
         QSharedPointer<SectionInfo> si (new SectionInfo(ta.fileName, -1, ta.label, ta.title, SectionInfo::Anchor, 0, m_tagName));
         Doxygen::sectionDict->insert(ta.label, si);

         e->anchors->append(*si);

      } else {
         warn("Duplicate anchor %s found\n", ta.label.data());
      }
   }
}

void TagFileParser::buildMemberList(Entry *ce, QList<TagMemberInfo> &members)
{
   for (auto tmi : members) {
      Entry *me = new Entry;

      me->type  = tmi.type;
      me->name  = tmi.name;
      me->args  = tmi.arglist;

      if (! me->args.isEmpty()) {
         me->argList = ArgumentList();
         stringToArgumentList(me->args.constData(), &(me->argList));
      }

      if (tmi.enumValues.count() > 0) {
         me->spec |= Entry::Strong;        

         for (auto evi : tmi.enumValues) {
            Entry *ev      = new Entry;
            ev->type       = "@";
            ev->name       = evi.name;
            ev->id         = evi.clangid;
            ev->section    = Entry::VARIABLE_SEC;
            TagInfo *ti    = new TagInfo;
            ti->tagName    = m_tagName;
            ti->anchor     = evi.anchor;
            ti->fileName   = evi.file;
            ev->tagInfo    = ti;

            me->addSubEntry(ev);
         }
      }

      me->protection = tmi.prot;
      me->virt       = tmi.virt;
      me->stat       = tmi.isStatic;
      me->fileName   = ce->fileName;
      me->id         = tmi.clangId;

      if (ce->section == Entry::GROUPDOC_SEC) {
         me->groups->append(Grouping(ce->name, Grouping::GROUPING_INGROUP));
      }

      addDocAnchors(me, tmi.docAnchors);
      TagInfo *ti    = new TagInfo;

      ti->tagName    = m_tagName;
      ti->anchor     = tmi.anchor;
      ti->fileName   = tmi.anchorFile;
      me->tagInfo    = ti;

      if (tmi.kind == "define") {
         me->type = "#define";
         me->section = Entry::DEFINE_SEC;

      } else if (tmi.kind == "enumvalue") {
         me->section = Entry::VARIABLE_SEC;
         me->mtype = Method;

      } else if (tmi.kind == "property") {
         me->section = Entry::VARIABLE_SEC;
         me->mtype = Property;

      } else if (tmi.kind == "event") {
         me->section = Entry::VARIABLE_SEC;
         me->mtype = Event;

      } else if (tmi.kind == "variable") {
         me->section = Entry::VARIABLE_SEC;
         me->mtype = Method;

      } else if (tmi.kind == "typedef") {
         me->section = Entry::VARIABLE_SEC; //Entry::TYPEDEF_SEC;
         me->type.prepend("typedef ");
         me->mtype = Method;

      } else if (tmi.kind == "enumeration") {
         me->section = Entry::ENUM_SEC;
         me->mtype = Method;

      } else if (tmi.kind == "function") {
         me->section = Entry::FUNCTION_SEC;
         me->mtype = Method;

      } else if (tmi.kind == "signal") {
         me->section = Entry::FUNCTION_SEC;
         me->mtype = Signal;

      } else if (tmi.kind == "prototype") {
         me->section = Entry::FUNCTION_SEC;
         me->mtype = Method;

      } else if (tmi.kind == "friend") {
         me->section = Entry::FUNCTION_SEC;
         me->type.prepend("friend ");
         me->mtype = Method;

      } else if (tmi.kind == "dcop") {
         me->section = Entry::FUNCTION_SEC;
         me->mtype = DCOP;

      } else if (tmi.kind == "slot") {
         me->section = Entry::FUNCTION_SEC;
         me->mtype = Slot;
      }

      ce->addSubEntry(me);
   }
}

static QByteArray stripPath(const QByteArray &s)
{
   int i = s.lastIndexOf('/');
   if (i != -1) {
      return s.right(s.length() - i - 1);
   } else {
      return s;
   }
}

/*! Injects the info gathered by the XML parser into the Entry tree.
 *  This tree contains the information extracted from the input in a
 *  "unrelated" form.
 */
void TagFileParser::buildLists(Entry *root)
{
   // build class list
   for (auto tci : m_tagFileClasses) {

      Entry *ce = new Entry;
      ce->section = Entry::CLASS_SEC;

      switch (tci.kind) {
         case TagClassInfo::Class:
            break;

         case TagClassInfo::Struct:
            ce->spec = Entry::Struct;
            break;

         case TagClassInfo::Union:
            ce->spec = Entry::Union;
            break;

         case TagClassInfo::Interface:
            ce->spec = Entry::Interface;
            break;

         case TagClassInfo::Enum:
            ce->spec = Entry::Enum;
            break;

         case TagClassInfo::Exception:
            ce->spec = Entry::Exception;
            break;

         case TagClassInfo::Protocol:
            ce->spec = Entry::Protocol;
            break;

         case TagClassInfo::Category:
            ce->spec = Entry::Category;
            break;

         case TagClassInfo::Service:
            ce->spec = Entry::Service;
            break;

         case TagClassInfo::Singleton:
            ce->spec = Entry::Singleton;
            break;
      }

      ce->name = tci.name;

      if (tci.kind == TagClassInfo::Protocol) {
         ce->name += "-p";
      }

      addDocAnchors(ce, tci.docAnchors);
      TagInfo *ti  = new TagInfo;

      ti->tagName  = m_tagName;
      ti->fileName = tci.filename;
      ce->id       = tci.clangId;
      ce->tagInfo  = ti;
      ce->lang     = tci.isObjC ? SrcLangExt_ObjC : SrcLangExt_Unknown;

      // transfer base class list
      if (tci.bases) {         
         ce->extends = *tci.bases;

         delete tci.bases;
         tci.bases = 0;
      }

      if (tci.templateArguments) {
         if (ce->tArgLists == 0) {
            ce->tArgLists = new QList<ArgumentList>;           
         }

         ArgumentList al;
         ce->tArgLists->append(al);

         for (auto argName : *tci.templateArguments) {
            Argument a;

            a.type = "class";
            a.name = argName;

            al.append(a);
         }
      }

      buildMemberList(ce, tci.members);
      root->addSubEntry(ce);
   }

   // build file list   
   for (auto tfi : m_tagFileFiles) {
      Entry *fe = new Entry;
      fe->section = guessSection(tfi.name);
      fe->name    = tfi.name;

      addDocAnchors(fe, tfi.docAnchors);

      TagInfo *ti  = new TagInfo;
      ti->tagName  = m_tagName;
      ti->fileName = tfi.filename;
      fe->tagInfo  = ti;

      QByteArray fullName = m_tagName + ":" + tfi.path + stripPath(tfi.name);
      fe->fileName = fullName;
      
      FileDef *fd = new FileDef(m_tagName + ":" + tfi.path, tfi.name, m_tagName, tfi.filename );
     
      QSharedPointer<FileName> mn (Doxygen::inputNameDict->find(tfi.name));

      if (mn) {
         mn->append(fd);

      } else {       
         mn = QSharedPointer<FileName> (new FileName(fullName, tfi.name));
         mn->append(fd);

         Doxygen::inputNameList->inSort(mn.data());
         Doxygen::inputNameDict->insert(tfi.name, mn);
      }

      buildMemberList(fe, tfi.members);
      root->addSubEntry(fe);
   }

   // build namespace list   
   for (auto tni : m_tagFileNamespaces) {
      Entry *ne    = new Entry;
      ne->section  = Entry::NAMESPACE_SEC;
      ne->name     = tni.name;
      addDocAnchors(ne, tni.docAnchors);

      TagInfo *ti  = new TagInfo;
      ti->tagName  = m_tagName;
      ti->fileName = tni.filename;
      ne->id       = tni.clangId;
      ne->tagInfo  = ti;

      buildMemberList(ne, tni.members);
      root->addSubEntry(ne);
   }

   // build package list 
   for (auto tpgi : m_tagFilePackages) {
      Entry *pe    = new Entry;
      pe->section  = Entry::PACKAGE_SEC;
      pe->name     = tpgi.name;
      addDocAnchors(pe, tpgi.docAnchors);

      TagInfo *ti  = new TagInfo;
      ti->tagName  = m_tagName;
      ti->fileName = tpgi.filename;
      pe->tagInfo  = ti;

      buildMemberList(pe, tpgi.members);
      root->addSubEntry(pe);
   }

   // build group list  
   for (auto tgi : m_tagFileGroups) {
      Entry *ge    = new Entry;
      ge->section  = Entry::GROUPDOC_SEC;
      ge->name     = tgi.name;
      ge->type     = tgi.title;

      addDocAnchors(ge, tgi.docAnchors);

      TagInfo *ti  = new TagInfo;
      ti->tagName  = m_tagName;
      ti->fileName = tgi.filename;
      ge->tagInfo  = ti;

      buildMemberList(ge, tgi.members);
      root->addSubEntry(ge);
   }

   // build page list 
   for (auto tpi : m_tagFilePages) {
      Entry *pe = new Entry;

      // hard sets the section
      if (tpi.filename == "index") { 
         pe->section = Entry::MAINPAGEDOC_SEC;

      } else {
         pe->section = Entry::PAGEDOC_SEC;
      }

      pe->name     = tpi.name;
      pe->args     = tpi.title;

      addDocAnchors(pe, tpi.docAnchors);

      TagInfo *ti  = new TagInfo;
      ti->tagName  = m_tagName;
      ti->fileName = tpi.filename;
      pe->tagInfo  = ti;

      root->addSubEntry(pe);
   }
}

void TagFileParser::addIncludes()
{ 
   for (auto tfi : m_tagFileFiles) {      
      QSharedPointer<FileName> fn (Doxygen::inputNameDict->find(tfi.name));

      if (fn) {
                 
         for (auto fd : *fn) {
            if (fd->getPath() == QByteArray(m_tagName + ":" + tfi.path)) {
                            
               for (auto item : tfi.includes) {
               
                  QSharedPointer<FileName> ifn (Doxygen::inputNameDict->find(item.name));
                  assert(ifn != 0);

                  if (ifn) {                   
                     for (auto ifd : *ifn) {
                        if (ifd->getOutputFileBase() == QByteArray(item.id)) {
                           fd->addIncludeDependency(ifd, item.text, item.isLocal, item.isImported, false);
                        }
                     }
                  }

               }
            }
         }
      }
   }
}

void parseTagFile(Entry *root, const char *fullName)
{
   QFileInfo fi(fullName);

   if (! fi.exists()) {
      return;
   }

   TagFileErrorHandler errorHandler;

   // tagName
   TagFileParser handler(fullName);
   handler.setFileName(fullName);
   
   //
   QFile xmlFile(fullName);   
   QByteArray data = xmlFile.readAll();
  
   QXmlInputSource source;
   source.setData(data);

   QXmlSimpleReader reader;
   reader.setContentHandler(&handler);
   reader.setErrorHandler(&errorHandler);
   reader.parse(source);

   handler.buildLists(root);
   handler.addIncludes();   
}
