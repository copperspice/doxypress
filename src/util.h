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
#include <bufstr.h>
#include <classdef.h>
#include <docparser.h>
#include <filedef.h>
#include <groupdef.h>
#include <membergroup.h>
#include <pagedef.h>
#include <stringmap.h>
#include <outputlist.h>

class BaseClassDef;
class ClassSDict;
class Definition;
class ExampleSDict;
class FileNameDict;
class MemberList;
class MemberDef;
class MemberNameInfoSDict;
class NamespaceDef;
class NamespaceSDict;
class OutputDocInterface;

struct ListItemInfo;
struct SectionInfo;
struct TagInfo;

template <class T>
class SortedList;


/*! \file
 *  \brief A bunch of utility functions.
 */

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

inline bool isId(int c)
{
   if (c == '_') {
      return true;

   } else if (c >= 128) {
      return true;

   } else if (c < 0) { 
      return true;
      
   } else if (isalnum(c)) {
      return true;
   
   }

   return false;
}

// **

bool classVisibleInIndex(QSharedPointer<ClassDef> cd);
bool classHasVisibleChildren(QSharedPointer<ClassDef> cd);

QString dateToString(bool);
QString dateTimeHHMM();
QString yearToString();

bool getDefs(const QByteArray &scName, const QByteArray &mbName, const QByteArray &args, QSharedPointer<MemberDef> &md,
                  QSharedPointer<ClassDef> &cd, QSharedPointer<FileDef> &fd, QSharedPointer<NamespaceDef> &nd, 
                  QSharedPointer<GroupDef> &gd, bool forceEmptyScope = false, QSharedPointer<FileDef> currentFile = QSharedPointer<FileDef>(), 
                  bool checkCV = false, const QByteArray &forceTagFile = QByteArray() );

QString getFileFilter(const QByteArray &name, bool isSourceCode);

int getPrefixIndex(const QByteArray &name);

bool generateLink(OutputDocInterface &od, const QByteArray &clName, const QByteArray &lr, bool inSeeBlock, const QByteArray &lt);
void generateFileRef(OutputDocInterface &od, const QByteArray &name, const QByteArray &text = QByteArray() );

QByteArray getLanguageSpecificSeparator(SrcLangExt lang, bool classScope = false);
QByteArray getCanonicalTemplateSpec(QSharedPointer<Definition> d, QSharedPointer<FileDef> fs, const QByteArray &spec);

QByteArray fileToString(const QString &name, bool filter = false, bool isSourceCode = false);

QByteArray langToString(SrcLangExt lang);

void linkifyText(const TextGeneratorIntf &ol, QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, 
                 QSharedPointer<Definition> self, const QByteArray &text, bool autoBreak = false, bool external = true,
                 bool keepSpaces = false,  int indentLevel = 0);

bool matchArguments2(QSharedPointer<Definition> srcScope, QSharedPointer<FileDef> srcFileScope, ArgumentList *srcAl,
                  QSharedPointer<Definition> dstScope, QSharedPointer<FileDef> dstFileScope, ArgumentList *dstAl, bool checkCV );

void mergeArguments(ArgumentList *, ArgumentList *, bool forceNameOverwrite = false);


QSharedPointer<ClassDef> getClass(const QString &key);

QSharedPointer<ClassDef> getResolvedClass(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, const QString &key, 
                  QSharedPointer<MemberDef> *pTypeDef = 0, QByteArray *pTemplSpec = 0, bool mayBeUnlinkable = false, 
                  bool mayBeHidden = false, QByteArray *pResolvedType = 0);

QSharedPointer<NamespaceDef> getResolvedNamespace(const QString &key);
QSharedPointer<FileDef> findFileDef(const FileNameDict *fnDict, const QString &name, bool &ambig);

QByteArray showFileDefMatches(const FileNameDict *fnDict, const QByteArray &xName);

int guessSection(const QByteArray &name);

QByteArray argListToString(ArgumentList *al, bool useCanonicalType = false, bool showDefVals = true);
QString tempArgListToString(const ArgumentList *al, SrcLangExt lang);

void writeExample(OutputList &ol, ExampleSDict *el);

bool rightScopeMatch(const QString &scope, const QString &name);
bool leftScopeMatch(const QString&scope, const QString &name);


bool resolveRef(const QByteArray &scName, const QByteArray &tsname, bool inSeeBlock, QSharedPointer<Definition> *resContext, 
                  QSharedPointer<MemberDef> *resMember, bool lookForSpecializations = true, 
                  QSharedPointer<FileDef> currentFile = QSharedPointer<FileDef>(), bool checkScope = false);

bool resolveLink(const QString &scName, const QString &linkRef, bool inSeeBlock, QSharedPointer<Definition> *resContext, QString &resAnchor);

QString removeRedundantWhiteSpace(const QString &s);
QByteArray removeAnonymousScopes(const QByteArray &s);
QByteArray resolveDefines(const char *n);
QByteArray replaceAnonymousScopes(const QByteArray &s, const QByteArray &replacement = QByteArray() );

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

QByteArray convertToHtml(const QString &s, bool keepEntities = true);
QByteArray convertToXML(const QString &s);
QByteArray convertToJSString(const QString &s);

QByteArray getOverloadDocs();

void addMembersToMemberGroup(QSharedPointer<MemberList> ml, MemberGroupSDict **ppMemberGroupSDict, QSharedPointer<Definition> context);

int extractClassNameFromType(const QByteArray &type, int &pos,
                   QByteArray &name, QByteArray &templSpec, SrcLangExt = SrcLangExt_Unknown);

QByteArray normalizeNonTemplateArgumentsInString(const QByteArray &name, QSharedPointer<Definition> context, const ArgumentList *formalArgs);

QByteArray substituteTemplateArgumentsInString(const QByteArray &name, ArgumentList *formalArgs, ArgumentList *actualArgs);

QList<ArgumentList> *copyArgumentLists(const QList<ArgumentList> *srcLists);

QString stripTemplateSpecifiersFromScope(const QString &fullName, bool parentOnly = true, QString *lastScopeStripped = 0);

QByteArray resolveTypeDef(QSharedPointer<Definition> d, const QByteArray &name, 
                  QSharedPointer<Definition> *typedefContext = nullptr);

QString mergeScopes(const QString &leftScope, const QString &rightScope);

int getScopeFragment(const QString &s, int p, int *l);

int filterCRLF(char *buf, int len);

void addRefItem(const QList<ListItemInfo> *sli, const QString &key, const QString &prefix, 
                  const QString &name, const QString &title, const QString &args, QSharedPointer<Definition> scope);

QSharedPointer<PageDef> addRelatedPage(const QByteArray &name, const QString &ptitle, const QByteArray &doc, QList<SectionInfo> *anchors,
                  const QByteArray &fileName, int startLine, const QList<ListItemInfo> *sli, 
                  QSharedPointer<GroupDef> gd = QSharedPointer<GroupDef>(),
                  TagInfo *tagInfo = 0, SrcLangExt lang = SrcLangExt_Unknown );

QByteArray escapeCharsInString(const QString &name, bool allowDots, bool allowUnderscore = false);

void addGroupListToTitle(OutputList &ol, QSharedPointer<Definition> d);

void filterLatexString(QTextStream &t, const QString &str, bool insideTabbing = false,
                  bool insidePre = false, bool insideItem = false);

QString rtfFormatBmkStr(const QString &name);

QByteArray linkToText(SrcLangExt lang, const QByteArray &link, bool isFileName);

QString stripExtension(QString fName);

void replaceNamespaceAliases(QByteArray &scope, int i);

int isAccessibleFrom(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, QSharedPointer<Definition> item);

int isAccessibleFromWithExpScope(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, 
                  QSharedPointer<Definition> item, const QByteArray &explicitScopePart);

int computeQualifiedIndex(const QByteArray &name);

void addDirPrefix(QByteArray &fileName);

QByteArray relativePathToRoot(const QString &name);

void createSubDirs(QDir &d);

QByteArray stripPath(const QString &s);
bool containsWord(const QByteArray &s, const QByteArray &word);
bool findAndRemoveWord(QByteArray &s, const QByteArray &word);

QByteArray stripLeadingAndTrailingEmptyLines(const QByteArray &s, int &docLine);

bool updateLanguageMapping(const QString &extension, const QString &parser);
SrcLangExt getLanguageFromFileName(const QString &fileName);
void initDefaultExtensionMapping();

QSharedPointer<MemberDef> getMemberFromSymbol(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, const QByteArray &name);
bool checkIfTypedef(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope,const QByteArray &name);

QSharedPointer<ClassDef> newResolveTypedef(QSharedPointer<FileDef> fileScope, QSharedPointer<MemberDef> md, 
                  QSharedPointer<MemberDef> *pMemType = 0, QByteArray *pTemplSpec = 0, QByteArray *pResolvedType = 0, 
                  ArgumentList *actTemplParams = 0);

QByteArray parseCommentAsText(QSharedPointer<Definition> scope, QSharedPointer<MemberDef> member, 
                  const QByteArray &doc, const QByteArray &fileName, int lineNr);

QByteArray transcodeCharacterStringToUTF8(const QByteArray &input);

QByteArray recodeString(const QByteArray &str, const char *fromEncoding, const char *toEncoding);

QByteArray extractAliasArgs(const QByteArray &args, int pos);

int countAliasArguments(const QByteArray argList);

//QByteArray replaceAliasArguments(const QByteArray &aliasValue,const QByteArray &argList);

QByteArray resolveAliasCmd(const QByteArray aliasCmd);
QByteArray expandAlias(const QByteArray &aliasName, const QByteArray &aliasValue);

void writeTypeConstraints(OutputList &ol, QSharedPointer<Definition> d, ArgumentList *al);

QByteArray convertCharEntitiesToUTF8(const QByteArray &s);

void stackTrace();

bool readInputFile(const QByteArray &fileName, BufStr &inBuf, bool filter = true, bool isSourceCode = false);

QByteArray filterTitle(const QString &title);

bool patternMatch(const QFileInfo &fi, const QStringList &patList);

QByteArray externalLinkTarget();
QByteArray externalRef(const QString &relPath, const QByteArray &ref, bool href);
int nextUtf8CharPosition(const QByteArray &utf8Str, int len, int startPos);

const char *writeUtf8Char(QTextStream &t, const char *s);
void writePageRef(OutputDocInterface &od, const char *cn, const char *mn);

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
QByteArray replaceColorMarkers(const QByteArray &str);

bool copyFile(const QString &src, const QString &dest);
QByteArray extractBlock(const QByteArray &text, const QByteArray &marker);

QByteArray correctURL(const QString &url, const QString &relPath);

QByteArray processMarkup(const QByteArray &s);

bool protectionLevelVisible(Protection prot);

QByteArray stripIndentation(const QByteArray &s);

bool fileVisibleInIndex(QSharedPointer<FileDef> fd, bool &genSourceFile);

void addDocCrossReference(QSharedPointer<MemberDef> src, QSharedPointer<MemberDef> dst);

uint getUtf8Code( const QString &s, int idx );
uint getUtf8CodeToLower( const QString &s, int idx );
uint getUtf8CodeToUpper( const QString &s, int idx );

QByteArray extractDirection(QString docs);

void convertProtectionLevel(MemberListType inListType,Protection inProt, int *outListType1, int *outListType2);

bool mainPageHasTitle();

QString stripPrefix(QString input, const QByteArray &prefix); 
QByteArray stripPrefix(QByteArray input, const QByteArray &prefix);

Protection getProtection(const QByteArray &data);

#endif

