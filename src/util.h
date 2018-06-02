/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim
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

#ifndef UTIL_H
#define UTIL_H

#include <QList>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QTextStream>

#include <ctype.h>
#include <types.h>

#include <arguments.h>
#include <classdef.h>
#include <docparser.h>
#include <filedef.h>
#include <groupdef.h>
#include <membergroup.h>
#include <pagedef.h>
#include <outputlist.h>
#include <stringmap.h>
#include <sortedlist_fwd.h>

class BaseClassDef;
class Definition;
class FileNameDict;
class MemberList;
class MemberDef;
class NamespaceDef;
class OutputDocInterface;

struct ListItemInfo;
struct SectionInfo;
struct TagInfo;

// Abstract interface for a hyperlinked text fragment
class TextGeneratorIntf
{
 public:
   virtual ~TextGeneratorIntf() {}

   virtual void writeString(const QString &, bool) const = 0;
   virtual void writeBreak(int indent) const = 0;
   virtual void writeLink(const QString &extRef, const QString &file, const QString &anchor, const QString &text) const = 0;
};

// Implements TextGeneratorIntf for an OutputDocInterface stream
class TextGeneratorOLImpl : public TextGeneratorIntf
{
 public:
   TextGeneratorOLImpl(OutputDocInterface &od);
   virtual ~TextGeneratorOLImpl() {}

   void writeString(const QString &text, bool keepSpaces) const override;
   void writeBreak(int indent) const override;
   void writeLink(const QString &extRef, const QString &file, const QString &anchor, const QString &text) const override;

  private:
   OutputDocInterface &m_od;
};

// maps a letter to a QList of T::ElementType
template<class T>
class LetterToIndexMap : public QMap<QChar, QSharedPointer<T>>
{
 public:
   LetterToIndexMap()
   { }

   template<class E>
   void insertElement(QChar letter, E elem) {
      auto iter = this->find(letter);

      if (iter != this->end()) {
         QSharedPointer<T> data = iter.value();
         data->insertDef(elem);

      } else {
         QSharedPointer<T> data (new T(letter));

         this->insert(letter, data);
         data->insertDef(elem);
      }
   }
};

inline bool isId(QChar c)
{
   if (c == '_') {
      return true;

   } else if (c >= 128) {
      return true;

   } else if ( c.isLetterOrNumber() ) {
      return true;

   }

   return false;
}

// Data associated with a HSV colored image.
struct ColoredImgDataItem {
   QString  path;
   QString  name;
   unsigned short width;
   unsigned short height;
   const unsigned char *content;
   const unsigned char *alpha;
};

void    addDirPrefix(QByteArray &fileName);
void    addMembersToMemberGroup(QSharedPointer<MemberList> ml, MemberGroupSDict &memberGroupSDict, QSharedPointer<Definition> context);
void    addGroupListToTitle(OutputList &ol, QSharedPointer<Definition> d);

void    addRefItem(const QVector<ListItemInfo> &list, const QString &key, const QString &prefix,
                  const QString &name, const QString &title, const QString &args, QSharedPointer<Definition> scope);

QString argListToString(const ArgumentList &al, bool useCanonicalType = false, bool showDefVals = true);

QChar   charToLower(const QString &s, int index);
QChar   charToUpper(const QString &s, int index);

bool    classVisibleInIndex(QSharedPointer<ClassDef> cd);
bool    classHasVisibleChildren(QSharedPointer<ClassDef> cd);
int     computeQualifiedIndex(const QString &name);
void    createSubDirs(QDir &d);

QString convertNameToFile_X(const QString &name, bool allowDots = false, bool allowUnderscore = false);
QString convertToHtml(const QString &s,  bool keepEntities  = true);
QString convertToLatex(const QString &s, bool insideTabbing = false, bool keepSpaces = false);
QString convertToXML(const QString &s);
QString convertToJSString(const QString &s);
bool    checkExtension(const QString &fName, const QString &ext);

QString dateToString(bool includeTime);
QString dateTimeHHMM();
int     determineSection(const QString &name);

QString escapeCharsInString(const QString &name, bool allowDots, bool allowUnderscore = false);
QString externalLinkTarget();
QString externalRef(const QString &relPath, const QString &ref, bool href);
QString extractAliasArgs(const QString &args, int pos);
void    extractNamespaceName(const QString &scopeName, QString &className, QString &namespaceName, bool allowEmptyClass = false);
int     extractClassNameFromType(const QString &type, int &pos, QString &name, QString &templSpec, SrcLangExt = SrcLangExt_Unknown);

QByteArray extractDirection(QString docs);

QString filterTitle(const QString &title);
QString filterCRLF(const QString &buffer);
QString fileToString(const QString &name, bool filter = false, bool isSourceCode = false);
bool    findAndRemoveWord(QString &s, const QString &word);

void    filterLatexString(QTextStream &t, const QString &str, bool insideTabbing = false,
                  bool insidePre = false, bool insideItem = false, bool keepSpaces = false);

bool    getDefs(const QString &scName, const QString &mbName, const QString &args, QSharedPointer<MemberDef> &md,
                  QSharedPointer<ClassDef> &cd, QSharedPointer<FileDef> &fd, QSharedPointer<NamespaceDef> &nd,
                  QSharedPointer<GroupDef> &gd, bool forceEmptyScope = false, QSharedPointer<FileDef> currentFile = QSharedPointer<FileDef>(),
                  bool checkCV = false, const QString &forceTagFile = QString() );

QString getLanguageSpecificSeparator(SrcLangExt lang, bool classScope = false);
QString getFileFilter(const QString &name, bool isSourceCode);
int     getScopeFragment(const QString &s, int p, int *l);
int     getPrefixIndex(const QString &name);
bool    generateLink(OutputDocInterface &od, const QString &clName, const QString &lr, bool inSeeBlock, const QString &lt);
void    generateFileRef(OutputDocInterface &od, const QString &name, const QString &text = QString() );

QByteArray getCanonicalTemplateSpec(QSharedPointer<Definition> d, QSharedPointer<FileDef> fs, const QByteArray &spec);

bool    hasVisibleRoot(SortedList<BaseClassDef *> *bcl);


QString langToString(SrcLangExt lang);
QString latexEscapeLabelName(const QString &data,  bool insideTabbing);
QString latexEscapeIndexChars(const QString &data, bool insideTabbing);
QString latexEscapePDFString(const QString &data);
QString linkToText(SrcLangExt lang, const QString &link, bool isFileName);
bool    leftScopeMatch(const QString &scope, const QString &name);

void    linkifyText(const TextGeneratorIntf &ol, QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope,
                  QSharedPointer<Definition> self, const QString &text, bool autoBreak = false, bool external = true,
                  bool keepSpaces = false, int indentLevel = 0);

bool    matchArguments2(QSharedPointer<Definition> srcScope, QSharedPointer<FileDef> srcFileScope,
                  const ArgumentList &srcArgList, QSharedPointer<Definition> dstScope, QSharedPointer<FileDef> dstFileScope,
                  const ArgumentList &dstArgList, bool checkCV);

void    mergeArguments(ArgumentList &srcArgList, ArgumentList &dstArgList, bool forceNameOverwrite = false);

QString parseCommentAsText(QSharedPointer<const Definition> scope, QSharedPointer<const MemberDef> member,
                  const QString &doc, const QString &fileName, int lineNr);

bool    resolveRef(const QString &scName, const QString &tsname, bool inSeeBlock, QSharedPointer<Definition> *resContext,
                  QSharedPointer<MemberDef> *resMember, bool useBaseTemplateOnly = false,
                  QSharedPointer<FileDef> currentFile = QSharedPointer<FileDef>(), bool checkScope = false);

bool    resolveLink(const QString &scName, const QString &linkRef, bool inSeeBlock, QSharedPointer<Definition> *resContext, QString &resAnchor);
bool    rightScopeMatch(const QString &scope, const QString &name);

QString readInputFile(const QString &fileName);
bool    readInputFile(const QString &fileName, QString &fileContents, bool filter = true, bool isSourceCode = false);
void    replaceNamespaceAliases(QString &scope, int i);

QString resolveAliasCmd(const QString &aliasCmd);
QString removeRedundantWhiteSpace(const QString &s, bool makePretty = true);
QString removeAnonymousScopes(const QString &s);
QString replaceAnonymousScopes(const QString &s, const QString &replacement = QString() );
QString resolveTypeDef(QSharedPointer<Definition> d, const QString &name, QSharedPointer<Definition> *typedefContext = nullptr);
QString rtfFormatBmkStr(const QString &name);
QString renameNS_Aliases(const QString &scope, bool fromTo = true);
QString relativePathToRoot(const QString &name);

QString stripScope(const QString &name);
QString stripTemplateSpecifiersFromScope(const QString &fullName, bool parentOnly = true, QString *lastScopeStripped = nullptr);
QString stripExtensionGeneral(const QString &fName, const QString &ext);
QString stripExtension(const QString &fName);
QString stripAnonymousNamespaceScope(const QString &s);
QString stripFromPath(const QString &path);
QString stripFromIncludePath(const QString &path);
QString stripIndentation(const QString &s);
QString stripPrefix(QString input, const QByteArray &prefix);
QString stripPath(const QString &s);

QString substituteTemplateArgumentsInString(const QString &name, const ArgumentList &formalArgs, const ArgumentList &actualArgs);
QString showFileDefMatches(const FileNameDict &fnDict, const QString &xName);
bool    srcFileVisibleInIndex(QSharedPointer<FileDef> fd);
void    stackTrace();
void    setAnchors(QSharedPointer<MemberList> ml);

QByteArray stripPrefix(QByteArray input, const QByteArray &prefix);

QString substitute(const QString &origString, const QString &oldWord, const QString &newWord);
QString substituteOne(const QString &origString, QChar oldChar, const QString &newWord);

QString transcodeToQString(const QByteArray &input);
QString tempArgListToString(const ArgumentList &al, SrcLangExt lang);

void writeExample(OutputList &ol, const ExampleSDict &el);
void writeTypeConstraints(OutputList &ol, QSharedPointer<Definition> d, ArgumentList &argList);
void writePageRef(OutputDocInterface &od, const QString &cn, const QString &mn);
void writeExtraLatexPackages(QTextStream &t);

QString yearToString();


// **
QSharedPointer<PageDef> addRelatedPage(const QString &name, const QString &ptitle, const QString &doc,
                  const QString &fileName, int startLine, const QVector<ListItemInfo> &list,
                  QSharedPointer<GroupDef> gd = QSharedPointer<GroupDef>(),
                  const TagInfo &tagInfo = TagInfo(), SrcLangExt lang = SrcLangExt_Unknown);

QSharedPointer<FileDef> findFileDef(const FileNameDict *fnDict, const QString &name, bool &ambig);

QSharedPointer<ClassDef> getResolvedClass(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, const QString &key,
                  QSharedPointer<MemberDef> *pTypeDef = 0, QString *pTemplSpec = nullptr, bool mayBeUnlinkable = false,
                  bool mayBeHidden = false, QString *pResolvedType = nullptr );

QSharedPointer<ClassDef>     getClass(const QString &key);
QSharedPointer<NamespaceDef> getResolvedNamespace(const QString &key);
QSharedPointer<MemberDef>    getMemberFromSymbol(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, const QByteArray &name);

QSharedPointer<ClassDef> newResolveTypedef(QSharedPointer<FileDef> fileScope, QSharedPointer<MemberDef> md,
                  QSharedPointer<MemberDef> *pMemType = 0, QString *pTemplSpec = nullptr, QString *pResolvedType = nullptr,
                  ArgumentList *actTemplParams = 0);

Protection getProtection(const QString &data);

Protection classInheritedProtectionLevel(QSharedPointer<ClassDef> cd, QSharedPointer<ClassDef> bcd,
                  Protection prot = Public, int level = 0);

// method located in doxy_setup.cpp
SrcLangExt getLanguageFromFileName(const QString &fileName);


//  *******

void initClassHierarchy(ClassSDict *cl);
bool namespaceHasVisibleChild(QSharedPointer<NamespaceDef> nd, bool includeClasses);
int minClassDistance(QSharedPointer<const ClassDef> cd, QSharedPointer<const ClassDef> bcd, int level = 0);
QString insertTemplateSpecifierInScope(const QString &scope, const QString &templ);
QString normalizeNonTemplateArgumentsInString(const QString &name, QSharedPointer<Definition> context, const ArgumentList &formalArgList);
QString mergeScopes(const QString &leftScope, const QString &rightScope);
int isAccessibleFrom(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, QSharedPointer<Definition> item);
QString trimEmptyLines(const QString &s, int &docLine);
bool checkIfTypedef(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope,const QString &name);
int countAliasArguments(const QString &argList);
QString expandAlias(const QString &aliasName, const QString &aliasValue);
QString convertCharEntities(const QString &s);
bool patternMatch(const QFileInfo &fi, const QStringList &patList);
void writeColoredImgData(ColoredImgDataItem data);
QString replaceColorMarkers(const QString &str);
bool copyFile(const QString &src, const QString &dest);
QString extractBlock(const QString &text, const QString &marker);
QString correctURL(const QString &url, const QString &relPath);
bool protectionLevelVisible(Protection prot);
bool docFileVisibleInIndex(QSharedPointer<FileDef> fd);
void addDocCrossReference(QSharedPointer<MemberDef> src, QSharedPointer<MemberDef> dst);

void convertProtectionLevel(MemberListType inListType,Protection inProt, int *outListType1, int *outListType2);
bool mainPageHasTitle();

#endif

