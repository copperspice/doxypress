/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim 
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

/** Abstract interface for a hyperlinked text fragment. */
class TextGeneratorIntf
{
 public:
   virtual ~TextGeneratorIntf() {}

   virtual void writeString(const QString &, bool) const = 0;
   virtual void writeBreak(int indent) const = 0;
   virtual void writeLink(const QString &extRef, const QString &file, const QString &anchor, const QString &text) const = 0;
};

/** Implements TextGeneratorIntf for an OutputDocInterface stream. */
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

/** @brief maps a unicode character code to a list of T::ElementType's
 */
template<class T>
class LetterToIndexMap : public LongMap<QSharedPointer<T>>
{
 public:
   LetterToIndexMap() 
   {      
   }

   template<class E>
   void insertElement(uint letter, E elem) {

      if (this->contains(letter)) {      
         auto data = this->find(letter);
         data->insertDef(elem);

      } else {    
         QSharedPointer<T> data (new T(letter));

         this->insert(letter, data);
         data->insertDef(elem);
      }     
   }

 private:
   int compareMapValues(const QSharedPointer<T> &l1, const QSharedPointer<T> &l2) const override {
      return (int)l1->letter() - (int)l2->letter();
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


bool classVisibleInIndex(QSharedPointer<ClassDef> cd);
bool classHasVisibleChildren(QSharedPointer<ClassDef> cd);

QString dateToString(bool);
QString dateTimeHHMM();
QString yearToString();

bool getDefs(const QString &scName, const QString &mbName, const QString &args, QSharedPointer<MemberDef> &md,
                  QSharedPointer<ClassDef> &cd, QSharedPointer<FileDef> &fd, QSharedPointer<NamespaceDef> &nd, 
                  QSharedPointer<GroupDef> &gd, bool forceEmptyScope = false, QSharedPointer<FileDef> currentFile = QSharedPointer<FileDef>(), 
                  bool checkCV = false, const QString &forceTagFile = QString() );

QString getFileFilter(const QString &name, bool isSourceCode);

int getPrefixIndex(const QString &name);

bool generateLink(OutputDocInterface &od, const QString &clName, const QString &lr, bool inSeeBlock, const QString &lt);
void generateFileRef(OutputDocInterface &od, const QString &name, const QString &text = QString() );

QString getLanguageSpecificSeparator(SrcLangExt lang, bool classScope = false);
QByteArray getCanonicalTemplateSpec(QSharedPointer<Definition> d, QSharedPointer<FileDef> fs, const QByteArray &spec);

QString fileToString(const QString &name, bool filter = false, bool isSourceCode = false);

QString langToString(SrcLangExt lang);

void linkifyText(const TextGeneratorIntf &ol, QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, 
                 QSharedPointer<Definition> self, const QString &text, bool autoBreak = false, bool external = true,
                 bool keepSpaces = false,  int indentLevel = 0);

bool matchArguments2(QSharedPointer<Definition> srcScope, QSharedPointer<FileDef> srcFileScope, ArgumentList *srcAl,
                  QSharedPointer<Definition> dstScope, QSharedPointer<FileDef> dstFileScope, ArgumentList *dstAl, bool checkCV );

void mergeArguments(ArgumentList *, ArgumentList *, bool forceNameOverwrite = false);


QSharedPointer<ClassDef> getClass(const QString &key);

QSharedPointer<ClassDef> getResolvedClass(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, const QString &key, 
                  QSharedPointer<MemberDef> *pTypeDef = 0, QString *pTemplSpec = nullptr, bool mayBeUnlinkable = false, 
                  bool mayBeHidden = false, QString *pResolvedType = nullptr );

QSharedPointer<NamespaceDef> getResolvedNamespace(const QString &key);
QSharedPointer<FileDef> findFileDef(const FileNameDict *fnDict, const QString &name, bool &ambig);

QString showFileDefMatches(const FileNameDict *fnDict, const QString &xName);

int determineSection(const QString &name);

QString argListToString(ArgumentList *al, bool useCanonicalType = false, bool showDefVals = true);
QString tempArgListToString(const ArgumentList *al, SrcLangExt lang);

void writeExample(OutputList &ol, ExampleSDict *el);

bool rightScopeMatch(const QString &scope, const QString &name);
bool leftScopeMatch(const QString &scope, const QString &name);

bool resolveRef(const QString &scName, const QString &tsname, bool inSeeBlock, QSharedPointer<Definition> *resContext, 
                  QSharedPointer<MemberDef> *resMember, bool lookForSpecializations = true, 
                  QSharedPointer<FileDef> currentFile = QSharedPointer<FileDef>(), bool checkScope = false);

bool resolveLink(const QString &scName, const QString &linkRef, bool inSeeBlock, QSharedPointer<Definition> *resContext, QString &resAnchor);

QString removeRedundantWhiteSpace(const QString &s);
QString removeAnonymousScopes(const QString &s);
QString replaceAnonymousScopes(const QString &s, const QString &replacement = QString() );

QString stripAnonymousNamespaceScope(const QString &s);
QString stripFromPath(const QString &path);
QString stripFromIncludePath(const QString &path);
void setAnchors(QSharedPointer<MemberList> ml);

QByteArray substituteClassNames(const QByteArray &s);
QString substitute(const QString &s, const QString &src, const QString &dst);

void initClassHierarchy(ClassSDict *cl);

bool hasVisibleRoot(SortedList<BaseClassDef *> *bcl);

bool namespaceHasVisibleChild(QSharedPointer<NamespaceDef> nd, bool includeClasses);


int minClassDistance(QSharedPointer<const ClassDef> cd, QSharedPointer<const ClassDef> bcd, int level = 0);
Protection classInheritedProtectionLevel(QSharedPointer<ClassDef> cd, QSharedPointer<ClassDef> bcd, 
                  Protection prot = Public, int level = 0);

QString convertNameToFile(const QString &name, bool allowDots = false, bool allowUnderscore = false);

void extractNamespaceName(const QString &scopeName, QString &className, QString &namespaceName, bool allowEmptyClass = false);

QString insertTemplateSpecifierInScope(const QString &scope, const QString &templ);

QString stripScope(const QString &name);

QString convertToHtml(const QString &s, bool keepEntities = true);
QString convertToXML(const QString &s);
QString convertToJSString(const QString &s);

void addMembersToMemberGroup(QSharedPointer<MemberList> ml, MemberGroupSDict **ppMemberGroupSDict, QSharedPointer<Definition> context);

int extractClassNameFromType(const QString &type, int &pos, QString &name, QString &templSpec, SrcLangExt = SrcLangExt_Unknown);

QString normalizeNonTemplateArgumentsInString(const QString &name, QSharedPointer<Definition> context, const ArgumentList *formalArgs);

QString substituteTemplateArgumentsInString(const QString &name, ArgumentList *formalArgs, ArgumentList *actualArgs);

QList<ArgumentList> *copyArgumentLists(const QList<ArgumentList> *srcLists);

QString stripTemplateSpecifiersFromScope(const QString &fullName, bool parentOnly = true, QString *lastScopeStripped = 0);

QString resolveTypeDef(QSharedPointer<Definition> d, const QString &name, QSharedPointer<Definition> *typedefContext = nullptr);

QString mergeScopes(const QString &leftScope, const QString &rightScope);

int getScopeFragment(const QString &s, int p, int *l);

QString filterCRLF(const QString &buffer);

void addRefItem(const QList<ListItemInfo> *sli, const QString &key, const QString &prefix, 
                  const QString &name, const QString &title, const QString &args, QSharedPointer<Definition> scope);

QSharedPointer<PageDef> addRelatedPage(const QString &name, const QString &ptitle, const QString &doc, QList<SectionInfo> *anchors,
                  const QString &fileName, int startLine, const QList<ListItemInfo> *sli, 
                  QSharedPointer<GroupDef> gd = QSharedPointer<GroupDef>(),
                  TagInfo *tagInfo = 0, SrcLangExt lang = SrcLangExt_Unknown );

QString escapeCharsInString(const QString &name, bool allowDots, bool allowUnderscore = false);

void addGroupListToTitle(OutputList &ol, QSharedPointer<Definition> d);

void filterLatexString(QTextStream &t, const QString &str, bool insideTabbing = false,
                  bool insidePre = false, bool insideItem = false);

QString rtfFormatBmkStr(const QString &name);

QString linkToText(SrcLangExt lang, const QString &link, bool isFileName);

bool checkExtension(const QString &fName, const QString &ext);
QString stripExtensionGeneral(const QString &fName, const QString &ext);
QString stripExtension(const QString &fName);

QString renameNS_Aliases(const QString &scope, bool fromTo = true);
void replaceNamespaceAliases(QString &scope, int i);

int isAccessibleFrom(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, QSharedPointer<Definition> item);

int computeQualifiedIndex(const QString &name);

void addDirPrefix(QByteArray &fileName);

QByteArray relativePathToRoot(const QString &name);

void createSubDirs(QDir &d);

QString stripPath(const QString &s);
bool containsWord(const QString &s, const QString &word);
bool findAndRemoveWord(QString &s, const QString &word);

QString trimEmptyLines(const QString &s, int &docLine);

QSharedPointer<MemberDef> getMemberFromSymbol(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, const QByteArray &name);
bool checkIfTypedef(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope,const QString &name);

QSharedPointer<ClassDef> newResolveTypedef(QSharedPointer<FileDef> fileScope, QSharedPointer<MemberDef> md, 
                  QSharedPointer<MemberDef> *pMemType = 0, QString *pTemplSpec = nullptr, QString *pResolvedType = nullptr, 
                  ArgumentList *actTemplParams = 0);

QString parseCommentAsText(QSharedPointer<const Definition> scope, QSharedPointer<const MemberDef> member, 
                  const QString &doc, const QString &fileName, int lineNr);

QString transcodeToQString(const QByteArray &input);
QString extractAliasArgs(const QString &args, int pos);

int countAliasArguments(const QString &argList);

QString resolveAliasCmd(const QString &aliasCmd);
QString expandAlias(const QString &aliasName, const QString &aliasValue);

void writeTypeConstraints(OutputList &ol, QSharedPointer<Definition> d, ArgumentList *al);

QString convertCharEntities(const QString &s);

void stackTrace();

QString readInputFile(const QString &fileName);
bool readInputFile(const QString &fileName, QString &fileContents, bool filter = true, bool isSourceCode = false);

QString filterTitle(const QString &title);

bool patternMatch(const QFileInfo &fi, const QStringList &patList);

QString externalLinkTarget();
QString externalRef(const QString &relPath, const QString &ref, bool href);

void writePageRef(OutputDocInterface &od, const QString &cn, const QString &mn);

/** Data associated with a HSV colored image. */
struct ColoredImgDataItem {
   QString  path;
   QString  name;
   unsigned short width;
   unsigned short height;
   const unsigned char *content;
   const unsigned char *alpha;
};

void writeColoredImgData(ColoredImgDataItem data);
QString replaceColorMarkers(const QString &str);

bool copyFile(const QString &src, const QString &dest);
QString extractBlock(const QString &text, const QString &marker);
QString correctURL(const QString &url, const QString &relPath);

bool protectionLevelVisible(Protection prot);

QString stripIndentation(const QString &s);

bool docFileVisibleInIndex(QSharedPointer<FileDef> fd);
bool srcFileVisibleInIndex(QSharedPointer<FileDef> fd);

void addDocCrossReference(QSharedPointer<MemberDef> src, QSharedPointer<MemberDef> dst);

uint getUtf8Code( const QString &s, int idx );
uint getUtf8CodeToLower( const QString &s, int idx );
uint getUtf8CodeToUpper( const QString &s, int idx );

QByteArray extractDirection(QString docs);

void convertProtectionLevel(MemberListType inListType,Protection inProt, int *outListType1, int *outListType2);

bool mainPageHasTitle();

QString stripPrefix(QString input, const QByteArray &prefix); 
QByteArray stripPrefix(QByteArray input, const QByteArray &prefix);

Protection getProtection(const QString &data);

// method located in doxy_setup.cpp
SrcLangExt getLanguageFromFileName(const QString &fileName);

#endif

