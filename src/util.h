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

#ifndef UTIL_H
#define UTIL_H

#include <QList>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include <ctype.h>
#include <types.h>

#include <stringmap.h>
#include <docparser.h>

class ArgumentList;
class BaseClassDef;
class BufStr;
class ClassDef;
class ClassSDict;
class Definition;
class ExampleSDict;
class FileDef;
class FileNameDict;
class FTextStream;
class GroupDef;
class MemberList;
class MemberDef;
class MemberGroupSDict;
class MemberNameInfoSDict;
class NamespaceDef;
class NamespaceSDict;
class OutputList;
class OutputDocInterface;
class PageDef;

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
   virtual void writeString(const char *, bool) const = 0;
   virtual void writeBreak(int indent) const = 0;
   virtual void writeLink(const char *extRef, const char *file, const char *anchor, const char *text ) const = 0; };

/** Implements TextGeneratorIntf for an OutputDocInterface stream. */
class TextGeneratorOLImpl : public TextGeneratorIntf
{
 public:
   virtual ~TextGeneratorOLImpl() {}
   TextGeneratorOLImpl(OutputDocInterface &od);
   void writeString(const char *s, bool keepSpaces) const;
   void writeBreak(int indent) const;
   void writeLink(const char *extRef, const char *file, const char *anchor, const char *text ) const; private:
   OutputDocInterface &m_od;
};

//--------------------------------------------------------------------

/** @brief maps a unicode character code to a list of T::ElementType's
 */
template<class T>
class LetterToIndexMap : public LongMap<QSharedPointer<T>>
{
 public:
   LetterToIndexMap() 
   {      
   }

   void append(uint letter, typename T::ElementType *elem) {

      if (! this->contains(letter)) {      
         QSharedPointer<T> data (new T(letter));
         this->insert(letter, data);

         data->append(elem);

      }  else  {
         auto data = this->find(letter);
         data->append(elem);

      }     
   }

 private:
   int compareValues(const T *l1, const T *l2) const {
      return (int)l1->letter() - (int)l2->letter();
   }
};

//--------------------------------------------------------------------

QByteArray langToString(SrcLangExt lang);
QByteArray getLanguageSpecificSeparator(SrcLangExt lang, bool classScope = false);

//--------------------------------------------------------------------

void linkifyText(const TextGeneratorIntf &ol,
                 Definition *scope,
                 FileDef *fileScope,
                 Definition *self,
                 const char *text,
                 bool autoBreak = false,
                 bool external = true,
                 bool keepSpaces = false,
                 int indentLevel = 0
                );

void setAnchors(MemberList *ml);

QByteArray fileToString(const char *name, bool filter = false, bool isSourceCode = false);

QByteArray dateToString(bool);

bool getDefs(const QByteArray &scopeName,
             const QByteArray &memberName,
             const char *,
             MemberDef *&md,
             ClassDef *&cd,
             FileDef *&fd,
             NamespaceDef *&nd,
             GroupDef *&gd,
             bool forceEmptyScope = false,
             FileDef *currentFile = 0,
             bool checkCV = false,
             const char *forceTagFile = 0
            );

QByteArray getFileFilter(const char *name, bool isSourceCode);

bool resolveRef(/* in */  const char *scName,
                          /* in */  const char *name,
                          /* in */  bool inSeeBlock,
                          /* out */ Definition **resContext,
                          /* out */ MemberDef  **resMember,
                          /* in */  bool lookForSpecializations = true,
                          /* in */  FileDef *currentFile = 0,
                          /* in */  bool checkScope = false
               );

bool resolveLink(/* in */  const char *scName,
                           /* in */  const char *lr,
                           /* in */  bool inSeeBlock,
                           /* out */ Definition **resContext,
                           /* out */ QByteArray &resAnchor
                );

//bool generateRef(OutputDocInterface &od,const char *, const char *,bool inSeeBlock,const char * =0);

bool generateLink(OutputDocInterface &od, const char *,
                  const char *, bool inSeeBlock, const char *);

void generateFileRef(OutputDocInterface &od, const char *, const char *linkTxt = 0);

void writePageRef(OutputDocInterface &od, const char *cn, const char *mn);

QByteArray getCanonicalTemplateSpec(Definition *d, FileDef *fs, const QByteArray &spec);

bool matchArguments2(Definition *srcScope, FileDef *srcFileScope, ArgumentList *srcAl,
                     Definition *dstScope, FileDef *dstFileScope, ArgumentList *dstAl, bool checkCV );

void mergeArguments(ArgumentList *, ArgumentList *, bool forceNameOverwrite = false);

QByteArray substituteClassNames(const QByteArray &s);

QByteArray substitute(const QByteArray &s, const QByteArray &src, const QByteArray &dst);

QByteArray clearBlock(const char *s, const char *begin, const char *end);

QByteArray selectBlock(const QByteArray &s, const QByteArray &name, bool which);

QByteArray resolveDefines(const char *n);

ClassDef *getClass(const char *key);

ClassDef *getResolvedClass(Definition *scope,
                           FileDef *fileScope,
                           const char *key,
                           MemberDef **pTypeDef = 0,
                           QByteArray *pTemplSpec = 0,
                           bool mayBeUnlinkable = false,
                           bool mayBeHidden = false,
                           QByteArray *pResolvedType = 0);

NamespaceDef *getResolvedNamespace(const char *key);

FileDef *findFileDef(const FileNameDict *fnDict, const char *n, bool &ambig);

QByteArray showFileDefMatches(const FileNameDict *fnDict, const char *n);

int guessSection(const char *name);

inline bool isId(int c)
{
   return c == '_' || c >= 128 || c < 0 || isalnum(c);
}

QByteArray removeRedundantWhiteSpace(const QByteArray &s);

QByteArray argListToString(ArgumentList *al, bool useCanonicalType = false, bool showDefVals = true);

QByteArray tempArgListToString(ArgumentList *al, SrcLangExt lang);

QByteArray generateMarker(int id);

void writeExample(OutputList &ol, ExampleSDict *el);

QByteArray stripAnonymousNamespaceScope(const QByteArray &s);

QByteArray stripFromPath(const QByteArray &path);

QByteArray stripFromIncludePath(const QByteArray &path);

bool rightScopeMatch(const QByteArray &scope, const QByteArray &name);

bool leftScopeMatch(const QByteArray &scope, const QByteArray &name);

QByteArray substituteKeywords(const QByteArray &s, const char *title,
                              const char *projName, const char *projNum, const char *projBrief);

int getPrefixIndex(const QByteArray &name);

QByteArray removeAnonymousScopes(const QByteArray &s);

QByteArray replaceAnonymousScopes(const QByteArray &s, const char *replacement = 0);

void initClassHierarchy(ClassSDict *cl);

bool hasVisibleRoot(SortedList<BaseClassDef *> *bcl);

bool classHasVisibleChildren(ClassDef *cd);
bool namespaceHasVisibleChild(NamespaceDef *nd, bool includeClasses);
bool classVisibleInIndex(ClassDef *cd);

int minClassDistance(const ClassDef *cd, const ClassDef *bcd, int level = 0);
Protection classInheritedProtectionLevel(ClassDef *cd, ClassDef *bcd, Protection prot = Public, int level = 0);

QByteArray convertNameToFile(const char *name, bool allowDots = false, bool allowUnderscore = false);

void extractNamespaceName(const QByteArray &scopeName, QByteArray &className, QByteArray &namespaceName,
                          bool allowEmptyClass = false);

QByteArray insertTemplateSpecifierInScope(const QByteArray &scope, const QByteArray &templ);

QByteArray stripScope(const char *name);

QByteArray convertToHtml(const char *s, bool keepEntities = true);

QByteArray convertToXML(const char *s);

QByteArray convertToJSString(const char *s);

QByteArray getOverloadDocs();

void addMembersToMemberGroup(/* in */     MemberList *ml,
      /* in,out */ MemberGroupSDict **ppMemberGroupSDict,
      /* in */     Definition *context);

int extractClassNameFromType(const QByteArray &type, int &pos,
                             QByteArray &name, QByteArray &templSpec, SrcLangExt = SrcLangExt_Unknown);

QByteArray normalizeNonTemplateArgumentsInString(const QByteArray &name,Definition *context,
                                                 const ArgumentList *formalArgs);

QByteArray substituteTemplateArgumentsInString(const QByteArray &name, ArgumentList *formalArgs,
                                               ArgumentList *actualArgs);

QList<ArgumentList> *copyArgumentLists(const QList<ArgumentList> *srcLists);

QByteArray stripTemplateSpecifiersFromScope(const QByteArray &fullName, bool parentOnly = true,
                                            QByteArray *lastScopeStripped = 0);

QByteArray resolveTypeDef(Definition *d, const QByteArray &name, Definition **typedefContext = 0);

QByteArray mergeScopes(const QByteArray &leftScope, const QByteArray &rightScope);

int getScopeFragment(const QByteArray &s, int p, int *l);

int filterCRLF(char *buf, int len);

void addRefItem(const QList<ListItemInfo> *sli, const char *prefix, const char *key,
                const char *name, const char *title, const char *args, Definition *scope);

PageDef *addRelatedPage(const char *name, const QByteArray &ptitle,
                        const QByteArray &doc, QList<SectionInfo> *anchors,
                        const char *fileName, int startLine,
                        const QList<ListItemInfo> *sli,
                        GroupDef *gd = 0,
                        TagInfo *tagInfo = 0,
                        SrcLangExt lang = SrcLangExt_Unknown );

QByteArray escapeCharsInString(const char *name, bool allowDots, bool allowUnderscore = false);

void addGroupListToTitle(OutputList &ol, Definition *d);

void filterLatexString(FTextStream &t, const char *str, bool insideTabbing = false,
                       bool insidePre = false, bool insideItem = false);

QByteArray rtfFormatBmkStr(const char *name);

QByteArray linkToText(SrcLangExt lang, const char *link, bool isFileName);

QByteArray stripExtension(const char *fName);

void replaceNamespaceAliases(QByteArray &scope, int i);

int isAccessibleFrom(Definition *scope, FileDef *fileScope, Definition *item);

int isAccessibleFromWithExpScope(Definition *scope, FileDef *fileScope, Definition *item,
                                 const QByteArray &explicitScopePart);

int computeQualifiedIndex(const QByteArray &name);

void addDirPrefix(QByteArray &fileName);

QByteArray relativePathToRoot(const char *name);

void createSubDirs(QDir &d);

QByteArray stripPath(const char *s);

bool containsWord(const QByteArray &s, const QByteArray &word);

bool findAndRemoveWord(QByteArray &s, const QByteArray &word);

QByteArray stripLeadingAndTrailingEmptyLines(const QByteArray &s, int &docLine);

//void stringToSearchIndex(const QByteArray &docUrlBase,const QByteArray &title,
//                         const QByteArray &str, bool priority=false,
//                         const QByteArray &anchor="");

bool updateLanguageMapping(const QByteArray &extension, const QByteArray &parser);
SrcLangExt getLanguageFromFileName(const QByteArray fileName);
void initDefaultExtensionMapping();

MemberDef *getMemberFromSymbol(Definition *scope, FileDef *fileScope,
                               const char *n);
bool checkIfTypedef(Definition *scope, FileDef *fileScope, const char *n);

ClassDef *newResolveTypedef(FileDef *fileScope, MemberDef *md,
                            MemberDef **pMemType = 0, QByteArray *pTemplSpec = 0,
                            QByteArray *pResolvedType = 0,
                            ArgumentList *actTemplParams = 0);

QByteArray parseCommentAsText(const Definition *scope, const MemberDef *member, const QByteArray &doc, const QByteArray &fileName, int lineNr);

QByteArray transcodeCharacterStringToUTF8(const QByteArray &input);

QByteArray recodeString(const QByteArray &str, const char *fromEncoding, const char *toEncoding);

QByteArray extractAliasArgs(const QByteArray &args, int pos);

int countAliasArguments(const QByteArray argList);

//QByteArray replaceAliasArguments(const QByteArray &aliasValue,const QByteArray &argList);

QByteArray resolveAliasCmd(const QByteArray aliasCmd);
QByteArray expandAlias(const QByteArray &aliasName, const QByteArray &aliasValue);

void writeTypeConstraints(OutputList &ol, Definition *d, ArgumentList *al);

QByteArray convertCharEntitiesToUTF8(const QByteArray &s);

void stackTrace();

bool readInputFile(const char *fileName, BufStr &inBuf, bool filter = true, bool isSourceCode = false);

QByteArray filterTitle(const QByteArray &title);

bool patternMatch(const QFileInfo &fi, const QStringList *patList);

QByteArray externalLinkTarget();
QByteArray externalRef(const QByteArray &relPath, const QByteArray &ref, bool href);
int nextUtf8CharPosition(const QByteArray &utf8Str, int len, int startPos);
const char *writeUtf8Char(FTextStream &t, const char *s);


/** Data associated with a HSV colored image. */
struct ColoredImgDataItem {
   const char *name;
   unsigned short width;
   unsigned short height;
   unsigned char *content;
   unsigned char *alpha;
};

void writeColoredImgData(const char *dir, ColoredImgDataItem data[]);
QByteArray replaceColorMarkers(const char *str);

bool copyFile(const QByteArray &src, const QByteArray &dest);
QByteArray extractBlock(const QByteArray text, const QByteArray marker);

QByteArray correctURL(const QByteArray &url, const QByteArray &relPath);

QByteArray processMarkup(const QByteArray &s);

bool protectionLevelVisible(Protection prot);

QByteArray stripIndentation(const QByteArray &s);

bool fileVisibleInIndex(FileDef *fd, bool &genSourceFile);

void addDocCrossReference(MemberDef *src, MemberDef *dst);

uint getUtf8Code( const QByteArray &s, int idx );
uint getUtf8CodeToLower( const QByteArray &s, int idx );
uint getUtf8CodeToUpper( const QByteArray &s, int idx );

QByteArray extractDirection(QByteArray &docs);

void convertProtectionLevel(
   MemberListType inListType,
   Protection inProt,
   int *outListType1,
   int *outListType2
);

bool mainPageHasTitle();

#endif

