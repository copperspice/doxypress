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

#include <QByteArray>
#include <QCache>
#include <QCryptographicHash>
#include <QDateTime>
#include <QHash>
#include <QRegExp>
#include <QTextCodec>

#include <stdlib.h>
#include <errno.h>
#include <math.h>

#include <classlist.h>
#include <config.h>
#include <defargs.h>
#include <dirdef.h>
#include <doxygen.h>
#include <doxy_build_info.h>
#include <entry.h>
#include <example.h>
#include <filename.h>
#include <growbuf.h>
#include <htmlentity.h>
#include <htmlhelp.h>
#include <image.h>
#include <language.h>
#include <message.h>
#include <memberlist.h>
#include <namespacedef.h>
#include <membername.h>
#include <portable.h>
#include <parserintf.h>
#include <reflist.h>
#include <searchindex.h>
#include <textdocvisitor.h>
#include <util.h>

// must appear after the previous include - resolve soon 
#include <doxy_globals.h>

struct FindFileCacheElem {
   FindFileCacheElem(FileDef *fd, bool ambig) : fileDef(fd), isAmbig(ambig) 
   {}

   FileDef *fileDef;
   bool isAmbig;
};

#define ENABLE_TRACINGSUPPORT 0

#if defined(_OS_MAC_) && ENABLE_TRACINGSUPPORT
#define TRACINGSUPPORT
#endif

#ifdef TRACINGSUPPORT
#include <execinfo.h>
#include <unistd.h>
#endif

const int MAX_STACK_SIZE = 1000;

static QHash<QString, MemberDef *>         s_resolvedTypedefs;
static QHash<QString, Definition *>        s_visitedNamespaces;
static QCache<QString, FindFileCacheElem>  s_findFileDefCache;
static QHash<QString, int>                 s_extLookup;

// forward declaration
static QSharedPointer<ClassDef> getResolvedClassRec(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, const char *n, 
               QSharedPointer<MemberDef> *pTypeDef, QByteArray *pTemplSpec, QByteArray *pResolvedType );

int isAccessibleFromWithExpScope(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, 
               QSharedPointer<Definition> item, const QByteArray &explicitScopePart);

// ** selects one of the name to sub-dir mapping algorithms that is used
// to select a sub directory when CREATE_SUBDIRS is set to YES.

#define ALGO_COUNT 1
#define ALGO_CRC16 2
#define ALGO_MD5   3

// ** 
#define MAP_ALGO      ALGO_MD5
// #define MAP_ALGO   ALGO_COUNT
// #define MAP_ALGO   ALGO_CRC16


// ** debug support for matchArguments
#define DOX_MATCH
#define DOX_NOMATCH

// #define DOX_MATCH     printf("Match at line %d\n",__LINE__);
// #define DOX_NOMATCH   printf("Nomatch at line %d\n",__LINE__);

// ** 
#define REL_PATH_TO_ROOT "../../"

#define HEXTONUM(x) (((x)>='0' && (x)<='9') ? ((x)-'0') :       \
                     ((x)>='a' && (x)<='f') ? ((x)-'a'+10) :    \
                     ((x)>='A' && (x)<='F') ? ((x)-'A'+10) : 0)


//------------------------------------------------------------------------
// TextGeneratorOLImpl implementation
//------------------------------------------------------------------------

TextGeneratorOLImpl::TextGeneratorOLImpl(OutputDocInterface &od) : m_od(od)
{
}

void TextGeneratorOLImpl::writeString(const char *s, bool keepSpaces) const
{
   if (s == 0) {
      return;
   }
   
   if (keepSpaces) {
      const char *p = s;

      if (p) {
         char cs[2];
         char c;
         cs[1] = '\0';

         while ((c = *p++)) {
            if (c == ' ') {
               m_od.writeNonBreakableSpace(1);
            } else {
               cs[0] = c, m_od.docify(cs);
            }
         }
      }

   } else {
      m_od.docify(s);

   }
}

void TextGeneratorOLImpl::writeBreak(int indent) const
{
   m_od.lineBreak("typebreak");
   int i;
   for (i = 0; i < indent; i++) {
      m_od.writeNonBreakableSpace(3);
   }
}

void TextGeneratorOLImpl::writeLink(const char *extRef, const char *file, const char *anchor, const char *text) const
{
   m_od.writeObjectLink(extRef, file, anchor, text);
}


// an inheritance tree of depth of 100000 should be enough
const int maxInheritanceDepth = 100000;

/*!
  Removes all anonymous scopes from string s
  Possible examples:
\verbatim
   "bla::@10::blep"      => "bla::blep"
   "bla::@10::@11::blep" => "bla::blep"
   "@10::blep"           => "blep"
   " @10::blep"          => "blep"
   "@9::@10::blep"       => "blep"
   "bla::@1"             => "bla"
   "bla::@1::@2"         => "bla"
   "bla @1"              => "bla"
\endverbatim
 */
QByteArray removeAnonymousScopes(const QByteArray &s)
{
   QByteArray result;

   if (s.isEmpty()) {
      return result;
   }

   static QRegExp re("[ :]*@[0-9]+[: ]*");
   int i;
   int len;
   int sl = s.length();
   int p  = 0;

   while ((i = re.indexIn(s, p)) != -1) {
      len = re.matchedLength();

      result += s.mid(p, i - p);

      int c   = i;
      bool b1 = false;
      bool b2 = false;

      while (c < i + len && s.at(c) != '@')  {
         if (s.at(c++) == ':') {
            b1 = true;
         }
      }

      c = i + len - 1;
      while (c >= i && s.at(c) != '@')  {
         if (s.at(c--) == ':') {
            b2 = true;
         }
      }

      if (b1 && b2) {
         result += "::";
      }

      p = i + len;
   }

   result += s.right(sl - p);
  
   return result;
}

// replace anonymous scopes with __anonymous__ or replacement if provided
QByteArray replaceAnonymousScopes(const QByteArray &s, const char *replacement)
{
   QByteArray result;

   if (s.isEmpty()) {
      return result;
   }

   static QRegExp re("@[0-9]+");
   int i;
   int len;
   int sl = s.length();
   int p = 0;

   while ((i = re.indexIn(s, p)) != -1) {
      len = re.matchedLength();

      result += s.mid(p, i - p);

      if (replacement) {
         result += replacement;
      } else {
         result += "__anonymous__";
      }

      p = i + len;
   }

   result += s.right(sl - p);
   
   return result;
}

// strip anonymous left hand side part of the scope
QByteArray stripAnonymousNamespaceScope(const QByteArray &s)
{
   int i, p = 0, l;
   QByteArray newScope;

   int sl = s.length();

   while ((i = getScopeFragment(s, p, &l)) != -1) {   

      if (Doxygen::namespaceSDict->find(s.left(i + l)) != 0) {

         if (s.at(i) != '@') {
            if (! newScope.isEmpty()) {
               newScope += "::";
            }

            newScope += s.mid(i, l);
         }

      } else if (i < sl) {
         if (! newScope.isEmpty()) {
            newScope += "::";
         }

         newScope += s.right(sl - i);
         break;
      }

      p = i + l;
   }  

   return newScope;
}

void writePageRef(OutputDocInterface &od, const char *cn, const char *mn)
{
   od.pushGeneratorState();

   od.disable(OutputGenerator::Html);
   od.disable(OutputGenerator::Man);
   
   if (Config_getBool("PDF_HYPERLINKS")) {
      od.disable(OutputGenerator::Latex);
   }

   if (Config_getBool("RTF_HYPERLINKS")) {
      od.disable(OutputGenerator::RTF);
   }

   od.startPageRef();
   od.docify(theTranslator->trPageAbbreviation());
   od.endPageRef(cn, mn);

   od.popGeneratorState();
}

/*! Generate a place holder for a position in a list. Used for
 *  translators to be able to specify different elements orders
 *  depending on whether text flows from left to right or visa versa.
 */
QByteArray generateMarker(int id)
{
   const int maxMarkerStrLen = 20;
   char result[maxMarkerStrLen];

   qsnprintf(result, maxMarkerStrLen, "@%d", id);

   return result;
}

static QString stripFromPath(const QString &path, QStringList &list)
{
   // look at all the strings in the list and strip the longest match 
   QString retval = path;

   unsigned int length = 0;
   
   for (auto prefix : list) {
      
      if (prefix.length() > length) { 
         if (path.startsWith(prefix, Qt::CaseInsensitive)) { 
            length = prefix.length();
            retval = path.right(path.length() - prefix.length());
         }
      }    
   } 

   return retval;
}

/*! strip part of \a path if it matches
 *  one of the paths in the Config_getList("STRIP_FROM_PATH") list
 */
QString stripFromPath(const QString &path)
{
   return stripFromPath(path, Config_getList("STRIP_FROM_PATH"));
}

/*! strip part of \a path if it matches
 *  one of the paths in the Config_getList("INCLUDE_PATH") list
 */
QString stripFromIncludePath(const QString &path)
{
   return stripFromPath(path, Config_getList("STRIP_FROM_INC_PATH"));
}

/*! try to determine if \a name is a source or a header file name by looking
 * at the extension. A number of variations are allowed in both upper and lower case 
 */
int guessSection(const char *name)
{
   QByteArray n = QByteArray(name).toLower();

   // source
   if (n.right(2) == ".c"    || 
       n.right(3) == ".cc"   ||
       n.right(4) == ".cxx"  ||
       n.right(4) == ".cpp"  ||
       n.right(4) == ".c++"  ||
       n.right(5) == ".java" ||
       n.right(2) == ".m"    ||
       n.right(2) == ".M"    ||
       n.right(3) == ".mm"   ||
       n.right(3) == ".ii"   || // inline
       n.right(4) == ".ixx"  ||
       n.right(4) == ".ipp"  ||
       n.right(4) == ".i++"  ||
       n.right(4) == ".inl"  ||
       n.right(4) == ".xml") 
   {
      return Entry::SOURCE_SEC;
   }

   // header
   if (n.right(2) == ".h"   || 
       n.right(3) == ".hh"  ||
       n.right(4) == ".hxx" ||
       n.right(4) == ".hpp" ||
       n.right(4) == ".h++" ||
       n.right(4) == ".idl" ||
       n.right(4) == ".ddl" ||
       n.right(5) == ".pidl") 
   {
      return Entry::HEADER_SEC;
   }

   return 0;
}

QByteArray resolveTypeDef(Definition *context, const QByteArray &qualifiedName, Definition **typedefContext)
{
   QByteArray result;

   if (qualifiedName.isEmpty()) {     
      return result;
   }

   Definition *mContext = context;
   if (typedefContext) {
      *typedefContext = context;
   }

   // see if the qualified name has a scope part
   int scopeIndex = qualifiedName.lastIndexOf("::");
   QByteArray resName = qualifiedName;

   if (scopeIndex != -1) { // strip scope part for the name
      resName = qualifiedName.right(qualifiedName.length() - scopeIndex - 2);

      if (resName.isEmpty()) {        
         return result;
      }
   }

   QSharedPointer<MemberDef> md;

   while (mContext && md == 0) {
      // step 1: get the right scope
      Definition *resScope = mContext;

      if (scopeIndex != -1) {
         // split-off scope part
         QByteArray resScopeName = qualifiedName.left(scopeIndex);
         
         // look-up scope in context
         int is, ps = 0;
         int l;

         while ((is = getScopeFragment(resScopeName, ps, &l)) != -1) {
            QByteArray qualScopePart = resScopeName.mid(is, l);
            QByteArray tmp = resolveTypeDef(mContext, qualScopePart);

            if (!tmp.isEmpty()) {
               qualScopePart = tmp;
            }

            resScope = resScope->findInnerCompound(qualScopePart).data();
           
            if (resScope == 0) {
               break;
            }
            ps = is + l;
         }
      }
      
      // step 2: get the member
      if (resScope) { 
         // no scope or scope found in the current context    
         MemberNameSDict *mnd = 0;

         if (resScope->definitionType() == Definition::TypeClass) {
            mnd = Doxygen::memberNameSDict;
         } else {
            mnd = Doxygen::functionNameSDict;
         }

         QSharedPointer<MemberName> mn = mnd->find(resName);

         if (mn) {
            int minDist = -1;           
               
            for (auto tmd : *mn) {   

               if (tmd->isTypedef()) {
                  int dist = isAccessibleFrom(resScope, 0, tmd.data());

                  if (dist != -1 && (md == 0 || dist < minDist)) {
                     md = tmd;
                     minDist = dist;
                  }
               }
            }
         }
      }

      mContext = mContext->getOuterScope();
   }

   // step 3: get the member's type
   if (md) {

      result = md->typeString();
      QByteArray args = md->argsString();

      if (args.indexOf(")(") != -1) { 
         // typedef of a function/member pointer
         result += args;

      } else if (args.indexOf('[') != -1) { 
         // typedef of an array
         result += args;
      }

      if (typedefContext) {
         *typedefContext = md->getOuterScope();
      } 
   }

   return result;
}

/*! Get a class definition given its name.
 *  Returns 0 if the class is not found.
 */
QSharedPointer<ClassDef> getClass(const char *n)
{
   if (n == 0 || n[0] == '\0') {
      return QSharedPointer<ClassDef>();
   }

   QByteArray name = n;
   QSharedPointer<ClassDef> result = Doxygen::classSDict->find(name);
  
   return result;
}

QSharedPointer<NamespaceDef> getResolvedNamespace(const char *name)
{
   if (name == 0 || name[0] == '\0') {
      return QSharedPointer<NamespaceDef>();
   }

   QByteArray subst = Doxygen::namespaceAliasDict[name];

   if (! subst.isEmpty()) {
      int count = 0; 
      
      // recursion detection guard
      QByteArray newSubst;

      while ( ! (newSubst = Doxygen::namespaceAliasDict[subst]).isEmpty() && count < 10) {
         subst = newSubst;
         count++;
      }

      if (count == 10) {
         warn_uncond("possible recursive namespace alias detected for %s!\n", name);
      }

      return Doxygen::namespaceSDict->find(subst.constData());

   } else {
      return Doxygen::namespaceSDict->find(name);
   }
}


/*! Returns the class representing the value of the typedef represented by \a md
 *  within file \a fileScope.
 *
 *  Example: typedef A T; will return the class representing A if it is a class.
 *
 *  Example: typedef int T; will return 0, since "int" is not a class.
 */
ClassDef *newResolveTypedef(FileDef *fileScope, MemberDef *md,  MemberDef **pMemType, QByteArray *pTemplSpec,
                            QByteArray *pResolvedType, ArgumentList *actTemplParams)
{
   bool isCached = md->isTypedefValCached(); // value already cached

   if (isCached) {
     
      if (pTemplSpec) {
         *pTemplSpec    = md->getCachedTypedefTemplSpec();
      }

      if (pResolvedType) {
         *pResolvedType = md->getCachedResolvedTypedef();
      }

      return md->getCachedTypedefVal();
   }
  
   QByteArray qname = md->qualifiedName();
   if (s_resolvedTypedefs.contains(qname)) {
      return 0;   // typedef already done
   }

   // put on the trace list
   s_resolvedTypedefs.insert(qname, md); 

   ClassDef *typeClass = md->getClassDef();
   QByteArray type = md->typeString();

    // get the "value" of the typedef
   if (typeClass && typeClass->isTemplate() && actTemplParams && actTemplParams->count() > 0) { 
      type = substituteTemplateArgumentsInString(type, typeClass->templateArguments(), actTemplParams);
   }

   QByteArray typedefValue = type;
   int tl = type.length();
   int ip = tl - 1; // remove * and & at the end

   while (ip >= 0 && (type.at(ip) == '*' || type.at(ip) == '&' || type.at(ip) == ' ')) {
      ip--;
   }

   type = type.left(ip + 1);
   type = stripPrefix(type, "const ");     // strip leading "const"
   type = stripPrefix(type, "struct ");    // strip leading "struct"
   type = stripPrefix(type, "union ");     // strip leading "union"

   int sp = 0;
   tl = type.length(); 

   // length may have been changed
   while (sp < tl && type.at(sp) == ' ') {
      sp++;
   }

   MemberDef *memTypeDef = nullptr;
   QSharedPointer<ClassDef> result = getResolvedClassRec(md->getOuterScope(), fileScope, type, &memTypeDef, 0, pResolvedType);

   // if type is a typedef then return what it resolves to.
   if (memTypeDef && memTypeDef->isTypedef()) {
      result = newResolveTypedef(fileScope, memTypeDef, pMemType, pTemplSpec);
      goto done;

   } else if (memTypeDef && memTypeDef->isEnumerate() && pMemType) {
      *pMemType = memTypeDef;
   }
   
   if (result == 0) {
      // try unspecialized version if type is template
      int si = type.lastIndexOf("::");
      int i  = type.indexOf('<');

      if (si == -1 && i != -1) { // typedef of a template => try the unspecialized version
         if (pTemplSpec) {
            *pTemplSpec = type.mid(i);
         }

         result = getResolvedClassRec(md->getOuterScope(), fileScope, type.left(i), 0, 0, pResolvedType);
        
      } else if (si != -1) { // A::B
         i = type.indexOf('<', si);

         if (i == -1) { 
            // Something like A<T>::B => lookup A::B
            i = type.length();

         } else { 
            // Something like A<T>::B<S> => lookup A::B, spec=<S>
            if (pTemplSpec) {
               *pTemplSpec = type.mid(i);
            }
         }

         result = getResolvedClassRec(md->getOuterScope(), fileScope, stripTemplateSpecifiersFromScope(type.left(i), false), 0, 0, pResolvedType);
      }
      
   }

done:
   if (pResolvedType) {
      if (result) {
         *pResolvedType = result->qualifiedName();
         //printf("*pResolvedType=%s\n",pResolvedType->data());
         if (sp > 0) {
            pResolvedType->prepend(typedefValue.left(sp));
         }
         if (ip < tl - 1) {
            pResolvedType->append(typedefValue.right(tl - ip - 1));
         }
      } else {
         *pResolvedType = typedefValue;
      }
   }

   // remember computed value for next time
   if (result && result->getDefFileName() != "<code>")
      // this check is needed to prevent that temporary classes that are
      // introduced while parsing code fragments are being cached here.
   {
      //printf("setting cached typedef %p in result %p\n",md,result);
      //printf("==> %s (%s,%d)\n",result->name().data(),result->getDefFileName().data(),result->getDefLine());
      //printf("*pResolvedType=%s\n",pResolvedType?pResolvedType->data():"<none>");
      md->cacheTypedefVal(result,
                          pTemplSpec ? *pTemplSpec : QByteArray(),
                          pResolvedType ? *pResolvedType : QByteArray()
                         );
   }

   s_resolvedTypedefs.remove(qname); // remove from the trace list

   return result;
}

/*! Substitutes a simple unqualified \a name within \a scope. Returns the
 *  value of the typedef or \a name if no typedef was found.
 */
static QByteArray substTypedef(Definition *scope, FileDef *fileScope, const QByteArray &name, 
                               MemberDef **pTypeDef = 0)
{
   QByteArray result = name;

   if (name.isEmpty()) {
      return result;
   }

   // lookup scope fragment in the symbol map
   auto di = Doxygen::symbolMap->find(name);

   if (di == Doxygen::symbolMap->end()) {
      // could not find any matching symbols
      return 0;   
   }

   int minDistance = 10000; // init at "infinite"

   MemberDef *bestMatch = nullptr;

   QSharedPointer<QList<Definition *>> dl;  
   dl = *di;  
    
    // search for the best match  
   for (auto d : *dl) {
      // only look at members

      if (d->definitionType() == Definition::TypeMember) {
         // that are also typedefs
         MemberDef *md = dynamic_cast<MemberDef *>(d);   

         if (md->isTypedef()) { 
            // d is a typedef, test accessibility of typedef within scope

            int distance = isAccessibleFromWithExpScope(scope, fileScope, d, "");

            if (distance != -1 && distance < minDistance) {
               // definition is accessible and a better match
            
               minDistance = distance;
               bestMatch = md;
            }
         }
      }
    }
   

   if (bestMatch) {
      result = bestMatch->typeString();

      if (pTypeDef) {
         *pTypeDef = bestMatch;
      }
   }
   
   return result;
}

static QSharedPointer<Definition> endOfPathIsUsedClass(StringMap<QSharedPointer<Definition>> *cl, const QByteArray &localName)
{
   if (cl) {     

      for (auto cd : *cl) {
         if (cd->localName() == localName) {
            return cd;
         }
      }
   }

   return QSharedPointer<Definition>();
}

/*! Starting with scope \a start, the string \a path is interpreted as
 *  a part of a qualified scope name (e.g. A::B::C), and the scope is
 *  searched. If found the scope definition is returned, otherwise 0
 *  is returned.
 */
static Definition *followPath(Definition *start, FileDef *fileScope, const QByteArray &path)
{
   int is, ps;
   int l;

   Definition *current = start;
   ps = 0;
   
   // for each part of the explicit scope

   while ((is = getScopeFragment(path, ps, &l)) != -1) {
      // try to resolve the part if it is a typedef
      MemberDef *typeDef = nullptr;

      QByteArray qualScopePart = substTypedef(current, fileScope, path.mid(is, l), &typeDef);
      
      if (typeDef) {
         ClassDef *type = newResolveTypedef(fileScope, typeDef);

         if (type) {           
            return type;
         }
      }

      QSharedPointer<Definition> next = current->findInnerCompound(qualScopePart);
     
      if (next == 0) { // failed to follow the path
        
         if (current->definitionType() == Definition::TypeNamespace) {

            auto temp = &(((NamespaceDef *)current)->getUsedClasses());
            next = endOfPathIsUsedClass(temp, qualScopePart);

         } else if (current->definitionType() == Definition::TypeFile) {

            auto temp = ((FileDef *)current)->getUsedClasses();
            next = endOfPathIsUsedClass(temp, qualScopePart);
         }

         current = next.data();

         if (current == 0) {
            break;
         }

      } else { 
         // continue to follow scope
         current = next.data();         
      }

      ps = is + l;
   }
 
   // path could be followed

   return current; 
}

bool accessibleViaUsingClass(const StringMap<QSharedPointer<Definition>> *cl, FileDef *fileScope,
                             Definition *item, const QByteArray &explicitScopePart = "" )
{
   if (cl) { 
      // see if the class was imported via a using statement    
      bool explicitScopePartEmpty = explicitScopePart.isEmpty();
           
      for (auto ucd : *cl) {

         Definition *sc;

         if (explicitScopePartEmpty) {
            sc = ucd.data();  

         } else {
            sc = followPath(ucd.data(), fileScope, explicitScopePart);

         }

         if (sc && sc == item) {
            return true;
         }         
      }
   }

   return false;
}

bool accessibleViaUsingNamespace(const NamespaceSDict *nl, FileDef *fileScope, Definition *item,
                                 const QByteArray &explicitScopePart = "")
{
   static QHash<QString, void *> visitedDict;

   if (nl) { 
      // check used namespaces for the class 

      for (auto und : *nl) {       
         Definition *sc;

         if (explicitScopePart.isEmpty()) {
            sc = und.data();  

         } else {
            sc = followPath(und.data(), fileScope, explicitScopePart);

         }

         if (sc && item->getOuterScope() == sc) {           
            return true;
         }

         QByteArray key = und->name();

         if (! visitedDict.contains(key)) {
            visitedDict.insert(key, (void *)0x08);

            if (accessibleViaUsingNamespace(&(und->getUsedNamespaces()), fileScope, item, explicitScopePart)) {             
               return true;
            }

            visitedDict.remove(key);
         } 
        
      }
   }

   return false;
}

/** Helper class representing the stack of items considered while resolving
 *  the scope.
 */
class AccessStack
{
 public:
   AccessStack() : m_index(0)
   {}

   void push(Definition *scope, FileDef *fileScope, Definition *item) {
      if (m_index < MAX_STACK_SIZE) {
         m_elements[m_index].scope     = scope;
         m_elements[m_index].fileScope = fileScope;
         m_elements[m_index].item      = item;
         m_index++;
      }
   }
   void push(Definition *scope, FileDef *fileScope, Definition *item, const QByteArray &expScope) {
      if (m_index < MAX_STACK_SIZE) {
         m_elements[m_index].scope     = scope;
         m_elements[m_index].fileScope = fileScope;
         m_elements[m_index].item      = item;
         m_elements[m_index].expScope  = expScope;
         m_index++;
      }
   }
   void pop() {
      if (m_index > 0) {
         m_index--;
      }
   }
   bool find(Definition *scope, FileDef *fileScope, Definition *item) {
      int i = 0;
      for (i = 0; i < m_index; i++) {
         AccessElem *e = &m_elements[i];
         if (e->scope == scope && e->fileScope == fileScope && e->item == item) {
            return true;
         }
      }
      return false;
   }
   bool find(Definition *scope, FileDef *fileScope, Definition *item, const QByteArray &expScope) {
      int i = 0;
      for (i = 0; i < m_index; i++) {
         AccessElem *e = &m_elements[i];
         if (e->scope == scope && e->fileScope == fileScope && e->item == item && e->expScope == expScope) {
            return true;
         }
      }
      return false;
   }

 private:
   /** Element in the stack. */
   struct AccessElem {
      Definition *scope;
      FileDef *fileScope;
      Definition *item;
      QByteArray expScope;
   };
   int m_index;
   AccessElem m_elements[MAX_STACK_SIZE];
};

/* Returns the "distance" (=number of levels up) from item to scope, or -1
 * if item in not inside scope.
 */
int isAccessibleFrom(Definition *scope, FileDef *fileScope, Definition *item)
{
   //printf("<isAccesibleFrom(scope=%s,item=%s itemScope=%s)\n",
   //    scope->name().data(),item->name().data(),item->getOuterScope()->name().data());

   static AccessStack accessStack;
   if (accessStack.find(scope, fileScope, item)) {
      return -1;
   }
   accessStack.push(scope, fileScope, item);

   int result = 0; // assume we found it
   int i;

   Definition *itemScope = item->getOuterScope();

   bool memberAccessibleFromScope =
      (item->definitionType() == Definition::TypeMember &&                 // a member
       itemScope && itemScope->definitionType() == Definition::TypeClass  && // of a class
       scope->definitionType() == Definition::TypeClass &&                 // accessible
       ((ClassDef *)scope)->isAccessibleMember((MemberDef *)item)          // from scope
      );

   bool nestedClassInsideBaseClass =
      (item->definitionType() == Definition::TypeClass &&                  // a nested class
       itemScope && itemScope->definitionType() == Definition::TypeClass && // inside a base
       scope->definitionType() == Definition::TypeClass &&                 // class of scope
       ((ClassDef *)scope)->isBaseClass((ClassDef *)itemScope, true)
      );

   if (itemScope == scope || memberAccessibleFromScope || nestedClassInsideBaseClass) {
      //printf("> found it\n");

      if (nestedClassInsideBaseClass) {
         result++;   // penalty for base class to prevent
      }
      // this is preferred over nested class in this class
      // see bug 686956

   } else if (scope == Doxygen::globalScope) {

      if (fileScope) {
         StringMap<QSharedPointer<Definition>> *cl = fileScope->getUsedClasses();

         if (accessibleViaUsingClass(cl, fileScope, item)) {
            //printf("> found via used class\n");
            goto done;
         }

         NamespaceSDict *nl = fileScope->getUsedNamespaces();

         if (accessibleViaUsingNamespace(nl, fileScope, item)) {
            //printf("> found via used namespace\n");
            goto done;
         }
      }
      //printf("> reached global scope\n");
      result = -1; // not found in path to globalScope

   } else { // keep searching
      // check if scope is a namespace, which is using other classes and namespaces

      if (scope->definitionType() == Definition::TypeNamespace) {
         NamespaceDef *nscope = (NamespaceDef *)scope;
         
         StringMap<QSharedPointer<Definition>> cl = nscope->getUsedClasses();

         if (accessibleViaUsingClass(&cl, fileScope, item)) {            
            goto done;
         }

         NamespaceSDict nl = nscope->getUsedNamespaces();

         if (accessibleViaUsingNamespace(&nl, fileScope, item)) {            
            goto done;
         }
      }

      // repeat for the parent scope
      i = isAccessibleFrom(scope->getOuterScope(), fileScope, item);     
      result = (i == -1) ? -1 : i + 2;
   }

done:
   accessStack.pop();
   //Doxygen::lookupCache.insert(key,new int(result));
   return result;
}


/* Returns the "distance" (=number of levels up) from item to scope, or -1
 * if item in not in this scope. The explicitScopePart limits the search
 * to scopes that match \a scope (or its parent scope(s)) plus the explicit part.
 * Example:
 *
 * class A { public: class I {}; };
 * class B { public: class J {}; };
 *
 * - Looking for item=='J' inside scope=='B' will return 0.
 * - Looking for item=='I' inside scope=='B' will return -1
 *   (as it is not found in B nor in the global scope).
 * - Looking for item=='A::I' inside scope=='B', first the match B::A::I is tried but
 *   not found and then A::I is searched in the global scope, which matches and
 *   thus the result is 1.
 */
int isAccessibleFromWithExpScope(Definition *scope, FileDef *fileScope,
                                 Definition *item, const QByteArray &explicitScopePart)
{
   if (explicitScopePart.isEmpty()) {
      // handle degenerate case where there is no explicit scope.
      return isAccessibleFrom(scope, fileScope, item);
   }

   static AccessStack accessStack;
   if (accessStack.find(scope, fileScope, item, explicitScopePart)) {
      return -1;
   }
   accessStack.push(scope, fileScope, item, explicitScopePart);


   //printf("  <isAccessibleFromWithExpScope(%s,%s,%s)\n",scope?scope->name().data():"<global>",
   //                                      item?item->name().data():"<none>",
   //                                      explicitScopePart.data());

   int result = 0; // assume we found it

   Definition *newScope = followPath(scope, fileScope, explicitScopePart);

   if (newScope) { 
      // explicitScope is inside scope => newScope is the result
      Definition *itemScope = item->getOuterScope();

      //printf("    scope traversal successful %s<->%s!\n",itemScope->name().data(),newScope->name().data());
      //if (newScope && newScope->definitionType()==Definition::TypeClass)
      //{
      //  ClassDef *cd = (ClassDef *)newScope;
      //  printf("---> Class %s: bases=%p\n",cd->name().data(),cd->baseClasses());
      //}

      if (itemScope == newScope) { 
         // exact match of scopes => distance==0
       
      } else if (itemScope && newScope && itemScope->definitionType() == Definition::TypeClass &&
                 newScope->definitionType() == Definition::TypeClass &&
                 ((ClassDef *)newScope)->isBaseClass((ClassDef *)itemScope, true, 0) ) {

         // inheritance is also ok. Example: looking for B::I, where
         // class A { public: class I {} };
         // class B : public A {}
         // but looking for B::I, where
         // class A { public: class I {} };
         // class B { public: class I {} };
         // will find A::I, so we still prefer a direct match and give this one a distance of 1

         result = 1;
         
      } else {
         int i = -1;

         if (newScope->definitionType() == Definition::TypeNamespace) {
            s_visitedNamespaces.insert(newScope->name(), newScope);

            // this part deals with the case where item is a class
            // A::B::C but is explicit referenced as A::C, where B is imported
            // in A via a using directive.
           
            NamespaceDef *nscope = (NamespaceDef *)newScope;
            StringMap<QSharedPointer<Definition>> cl = nscope->getUsedClasses();
                              
            for (auto cd : cl) {                  
               if (cd == item) {                    
                  goto done;
               }
            }
            
            NamespaceSDict nl = nscope->getUsedNamespaces();
          
            for (auto nd : nl) {
               if (! s_visitedNamespaces.contains(nd->name())) {                    
                  i = isAccessibleFromWithExpScope(scope, fileScope, item, nd->name());
                  if (i != -1) {                        
                     goto done;
                  }
               }
            }
         
         }

         // repeat for the parent scope
         if (scope != Doxygen::globalScope) {
            i = isAccessibleFromWithExpScope(scope->getOuterScope(), fileScope, item, explicitScopePart);
         }
         
         result = (i == -1) ? -1 : i + 2;
      }

   } else { 
      // failed to resolve explicitScope
      
      if (scope->definitionType() == Definition::TypeNamespace) {
         NamespaceDef *nscope = (NamespaceDef *)scope;
         NamespaceSDict nl = nscope->getUsedNamespaces();

         if (accessibleViaUsingNamespace(&nl, fileScope, item, explicitScopePart)) {
            //printf("> found in used namespace\n");
            goto done;
         }
      }

      if (scope == Doxygen::globalScope) {
         if (fileScope) {
            NamespaceSDict *nl = fileScope->getUsedNamespaces();
            if (accessibleViaUsingNamespace(nl, fileScope, item, explicitScopePart)) {
               //printf("> found in used namespace\n");
               goto done;
            }
         }
         //printf("> not found\n");
         result = -1;
      } else { // continue by looking into the parent scope
         int i = isAccessibleFromWithExpScope(scope->getOuterScope(), fileScope,
                                              item, explicitScopePart);
         //printf("> result=%d\n",i);
         result = (i == -1) ? -1 : i + 2;
      }
   }

done:
   //printf("  > result=%d\n",result);
   accessStack.pop();
   //Doxygen::lookupCache.insert(key,new int(result));
   return result;
}

int computeQualifiedIndex(const QByteArray &name)
{
   int i = name.indexOf('<');

   return name.lastIndexOf("::", i == -1 ? name.length() : i);
}

static void getResolvedSymbol(Definition *scope, FileDef *fileScope, Definition *d, const QByteArray &explicitScopePart, 
                              ArgumentList *actTemplParams, int &minDistance, ClassDef *&bestMatch, MemberDef *&bestTypedef,
                              QByteArray &bestTemplSpec, QByteArray &bestResolvedType)
{
   // only look at classes and members that are enums or typedefs

   if (d->definitionType() == Definition::TypeClass || (d->definitionType() == Definition::TypeMember &&
          (((MemberDef *)d)->isTypedef() || ((MemberDef *)d)->isEnumerate()) ) ) {

      s_visitedNamespaces.clear();

      // test accessibility of definition within scope.
      int distance = isAccessibleFromWithExpScope(scope, fileScope, d, explicitScopePart);
     
      if (distance != -1) { // definition is accessible
         // see if we are dealing with a class or a typedef

         if (d->definitionType() == Definition::TypeClass) { 
            // d is a class
            ClassDef *cd = (ClassDef *)d;
            
            if (!cd->isTemplateArgument()) {
               // skip classes whice are only there to 
               // represent a template argument
                          
               if (distance < minDistance) { 
                  // found a definition that is "closer"
                  minDistance = distance;
                  bestMatch = cd;
                  bestTypedef = 0;
                  bestTemplSpec.resize(0);
                  bestResolvedType = cd->qualifiedName();

               } else if (distance == minDistance &&
                          fileScope && bestMatch &&
                          fileScope->getUsedNamespaces() &&
                          d->getOuterScope()->definitionType() == Definition::TypeNamespace &&
                          bestMatch->getOuterScope() == Doxygen::globalScope) {

                  // in case the distance is equal it could be that a class X
                  // is defined in a namespace and in the global scope. When searched
                  // in the global scope the distance is 0 in both cases. We have
                  // to choose one of the definitions: we choose the one in the
                  // namespace if the fileScope imports namespaces and the definition
                  // found was in a namespace while the best match so far isn't.
                  // Just a non-perfect heuristic but it could help in some situations
                  // (kdecore code is an example).

                  minDistance = distance;
                  bestMatch = cd;
                  bestTypedef = 0;
                  bestTemplSpec.resize(0);
                  bestResolvedType = cd->qualifiedName();
               }
            } else {
               
            }

         } else if (d->definitionType() == Definition::TypeMember) {
            MemberDef *md = (MemberDef *)d;
            
            if (md->isTypedef()) { 
               // d is a typedef
               QByteArray args = md->argsString();

               if (args.isEmpty()) {
                  // do not expand "typedef t a[4];"                  

                  // we found a symbol at this distance, but if it didn't
                  // resolve to a class, we still have to make sure that
                  // something at a greater distance does not match, since
                  // that symbol is hidden by this one.

                  if (distance < minDistance) {
                     QByteArray spec;
                     QByteArray type;

                     minDistance = distance;
                     MemberDef *enumType = 0;

                     ClassDef *cd = newResolveTypedef(fileScope, md, &enumType, &spec, &type, actTemplParams);
                     if (cd) { // type resolves to a class
                        //printf("      bestTypeDef=%p spec=%s type=%s\n",md,spec.data(),type.data());
                        bestMatch = cd;
                        bestTypedef = md;
                        bestTemplSpec = spec;
                        bestResolvedType = type;
                     } else if (enumType) { // type resolves to a enum
                        //printf("      is enum\n");
                        bestMatch = 0;
                        bestTypedef = enumType;
                        bestTemplSpec = "";
                        bestResolvedType = enumType->qualifiedName();
                     } else if (md->isReference()) { // external reference
                        bestMatch = 0;
                        bestTypedef = md;
                        bestTemplSpec = spec;
                        bestResolvedType = type;
                     } else {
                        bestMatch = 0;
                        bestTypedef = md;
                        bestTemplSpec.resize(0);
                        bestResolvedType.resize(0);
                        //printf("      no match\n");
                     }

                  } else {
                     //printf("      not the best match %d min=%d\n",distance,minDistance);
                  }

               } else {
                  //printf("     not a simple typedef\n")
               }

            } else if (md->isEnumerate()) {
               if (distance < minDistance) {
                  minDistance = distance;
                  bestMatch = 0;
                  bestTypedef = md;
                  bestTemplSpec = "";
                  bestResolvedType = md->qualifiedName();
               }
            }
         }

      }  else {
         //printf("  Not accessible!\n");

      }
   }   
}

/* Find the fully qualified class name referred to by the input class
 * or typedef name against the input scope.
 * Loops through scope and each of its parent scopes looking for a
 * match against the input name. Can recursively call itself when
 * resolving typedefs.
 */
static ClassDef *getResolvedClassRec(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, const char *n, 
                  QSharedPointer<MemberDef> *pTypeDef, QByteArray *pTemplSpec, QByteArray *pResolvedType )
{
   QByteArray name;
   QByteArray explicitScopePart;
   QByteArray strippedTemplateParams;

   name = stripTemplateSpecifiersFromScope(removeRedundantWhiteSpace(n), true, &strippedTemplateParams);

   ArgumentList actTemplParams;

   if (!strippedTemplateParams.isEmpty()) { 
      // template part that was stripped
      stringToArgumentList(strippedTemplateParams, &actTemplParams);
   }

   int qualifierIndex = computeQualifiedIndex(name);
   
   if (qualifierIndex != -1) { 
      // qualified name, split off the explicit scope part
      explicitScopePart = name.left(qualifierIndex);

      // todo: improve namespace alias substitution
      replaceNamespaceAliases(explicitScopePart, explicitScopePart.length());
      name = name.mid(qualifierIndex + 2);
   }

   if (name.isEmpty()) {     
      return 0; 
   }

   QSharedPointer<QList<Definition *>> dl = Doxygen::symbolMap->value(name);

   // the -g (for C# generics) and -p (for ObjC protocols) are now already
   // stripped from the key used in the symbolMap, so that is not needed here.

   if (dl == 0) {
      dl = Doxygen::symbolMap->value(name + "-p");

      if (dl == 0) {      
         return 0;
      }
      
   }
 
   bool hasUsingStatements = (fileScope && ((fileScope->getUsedNamespaces() && fileScope->getUsedNamespaces()->count() > 0) ||
                     (fileScope->getUsedClasses() && fileScope->getUsedClasses()->count() > 0)) );

   // Since it is often the case that the same name is searched in the same
   // scope over an over again (especially for the linked source code generation)
   // we use a cache to collect previous results. This is possible since the
   // result of a lookup is deterministic. As the key we use the concatenated
   // scope, the name to search for and the explicit scope prefix. The speedup
   // achieved by this simple cache can be enormous.

   int scopeNameLen = scope->name().length() + 1;
   int nameLen = name.length() + 1;
   int explicitPartLen = explicitScopePart.length();

   int fileScopeLen;

   if (hasUsingStatements) {
       fileScopeLen = 1 + fileScope->getFilePath().length();

   } else { 
      fileScopeLen = 0;

   }
  
   QByteArray key = scope->name( ) + "+" + name + "+" + explicitScopePart;
  
   // if a file scope is given and contains using statements we should also use the file part
   // in the key (as a class name can be in two different namespaces and a using statement in
   // a file can select one of them).

   if (hasUsingStatements) {
      key += "+" + fileScope->name();
   }
  
   LookupInfo *pval = Doxygen::lookupCache->object(key);
   
   if (pval) {
    
      if (pTemplSpec) {
         *pTemplSpec = pval->templSpec;
      }

      if (pTypeDef) {
         *pTypeDef = pval->typeDef;
      }

      if (pResolvedType) {
         *pResolvedType = pval->resolvedType;
      }
      
      return pval->classDef;

   } else {
      // not found, we already add a 0 to avoid the possibility of endless recursion.   
      Doxygen::lookupCache->insert(key, new LookupInfo);
   }

   ClassDef *bestMatch    = 0;
   MemberDef *bestTypedef = 0;

   QByteArray bestTemplSpec;
   QByteArray bestResolvedType;

   int minDistance = 10000; // init at "infinite"
  
   // not a unique name         
   int count = 0;
           
   for (auto d : *dl) {               
      getResolvedSymbol(scope, fileScope, d, explicitScopePart, &actTemplParams,
                        minDistance, bestMatch, bestTypedef, bestTemplSpec, bestResolvedType);

      ++count;
   }
 
   if (pTypeDef) {
      *pTypeDef = bestTypedef;
   }

   if (pTemplSpec) {
      *pTemplSpec = bestTemplSpec;
   }

   if (pResolvedType) {
      *pResolvedType = bestResolvedType;
   }

   pval = Doxygen::lookupCache->object(key);

   if (pval) {
      pval->classDef     = bestMatch;
      pval->typeDef      = bestTypedef;
      pval->templSpec    = bestTemplSpec;
      pval->resolvedType = bestResolvedType;

   } else {
      Doxygen::lookupCache->insert(key, new LookupInfo(bestMatch, bestTypedef, bestTemplSpec, bestResolvedType));

   }
  
   return bestMatch;
}

/* Find the fully qualified class name referred to by the input class
 * or typedef name against the input scope.
 * Loops through scope and each of its parent scopes looking for a
 * match against the input name.
 */
QSharedPointer<ClassDef> getResolvedClass(QSharedPointer<Definition> scope, QSharedPointer<FileDef> fileScope, const char *n, 
                     QSharedPointer<MemberDef> *pTypeDef, QByteArray *pTemplSpec, bool mayBeUnlinkable, bool mayBeHidden, QByteArray *pResolvedType)
{  
   s_resolvedTypedefs.clear();

   if (scope == 0 || (scope->definitionType() != Definition::TypeClass && scope->definitionType() != Definition::TypeNamespace ) ||
            (scope->getLanguage() == SrcLangExt_Java && QByteArray(n).indexOf("::") != -1) ) {
      scope = Doxygen::globalScope;
   }
  
   QSharedPointer<ClassDef>result;
  
   result = getResolvedClassRec(scope, fileScope, n, pTypeDef, pTemplSpec, pResolvedType);
  
   if (result == 0)  {
      // for nested classes imported via tag files, the scope may not
      // present, so we check the class name directly as well. See also bug701314   
      result = getClass(n);
   }

   if (! mayBeUnlinkable && result && !result->isLinkable()) {
      if (! mayBeHidden || !result->isHidden()) {         
         result = QSharedPointer<ClassDef>(); // don't link to artificial/hidden classes unless explicitly allowed
      }
   }
  
   return result;
}

static bool findOperator(const QByteArray &s, int i)
{
   int b = s.lastIndexOf("operator", i);
 
  if (b == -1) {
      return false;   // not found
   }

   b += 8;

   while (b < i)  {
      // check if there are only spaces in between
      // the operator and the >
   
      if (! isspace((uchar)s.at(b))) {
         return false;
      }

      b++;
   }

   return true;
}

static bool findOperator2(const QByteArray &s, int i)
{
   int b = s.lastIndexOf("operator", i);
   if (b == -1) {
      return false;   // not found
   }

   b += 8;

   while (b < i)  {
      // check if there are only non-ascii
      // characters in front of the operator
   
      if (isId((uchar)s.at(b))) {
         return false;
      }

      b++;
   }

   return true;
}

static const char constScope[]   = { 'c', 'o', 'n', 's', 't', ':' };
static const char virtualScope[] = { 'v', 'i', 'r', 't', 'u', 'a', 'l', ':' };

// Note: this function is not reentrant due to the use of static buffer!
QByteArray removeRedundantWhiteSpace(const QByteArray &s)
{
   static bool cliSupport = Config_getBool("CPP_CLI_SUPPORT");
  
   if (s.isEmpty()) {
      return s;
   }

   static GrowBuf growBuf;
  
   growBuf.clear();
   uint i;
   uint l = s.length();
   uint csp = 0;
   uint vsp = 0;
   char c;

   for (i = 0; i < l; i++) {

   nextChar:
      c = s.at(i);

      // search for "const"
      if (csp < 6 && c == constScope[csp] && // character matches substring "const"
            (csp > 0 ||                   // if it is the first character
             i == 0  ||                   // the previous may not be a digit
             !isId(s.at(i - 1))) ) {
         csp++;

      } else { // reset counter
         csp = 0;
      }

      // search for "virtual"
      if (vsp < 8 && c == virtualScope[vsp] && // character matches substring "virtual"
            (vsp > 0 ||                     // if it is the first character
             i == 0  ||                     // the previous may not be a digit
             !isId(s.at(i - 1)) ) ) {
         vsp++;

      } else { // reset counter
         vsp = 0;
      }

      if (c == '"') { 
         // quoted string
         i++;

         growBuf.addChar(c);

         while (i < l) {
            char cc = s.at(i);
            growBuf.addChar(cc);

            if (cc == '\\') { // escaped character
               growBuf.addChar(s.at(i + 1));
               i += 2;

            } else if (cc == '"') { // end of string
               i++;

               goto nextChar;

            } else { // any other character
               i++;

            }
        }

      } else if (i < l - 2 && c == '<' && 
                 (isId(s.at(i + 1)) || isspace((uchar)s.at(i + 1))) && (i < 8 || !findOperator(s, i)) ) {

         // string in front is not "operator"
         growBuf.addChar('<');
         growBuf.addChar(' ');

      } else if (i > 0 && c == '>' && 
                 (isId(s.at(i - 1)) || isspace((uchar)s.at(i - 1)) || s.at(i - 1) == '*' || s.at(i - 1) == '&') &&          
                 (i < 8 || !findOperator(s, i)) ) {

          // prev char is an id char or space
         growBuf.addChar(' ');
         growBuf.addChar('>');

      } else if (i > 0 && c == ',' && ! isspace((uchar)s.at(i - 1))
                 && ((i < l - 1 && (isId(s.at(i + 1)) || s.at(i + 1) == '[')) 
                     || (i < l - 2 && s.at(i + 1) == '$' && isId(s.at(i + 2))) 
                     || (i < l - 3 && s.at(i + 1) == '&' && s.at(i + 2) == '$' && isId(s.at(i + 3))))) { 

         // for PHP
         growBuf.addChar(',');
         growBuf.addChar(' ');

      } else if (i > 0 && ( (s.at(i - 1) == ')' && isId(c)) || (c == '\''  && s.at(i - 1) == ' ')) ) {
         growBuf.addChar(' ');
         growBuf.addChar(c);

      } else if (c == 't' && csp == 5  &&  i < l -1  &&  ! (isId(s.at(i + 1)) ||
                   s.at(i + 1) == ')' || s.at(i + 1) == ',' ) )  {
         // prevent const ::A from being converted to const::A
      
         growBuf.addChar('t');
         growBuf.addChar(' ');

         if (s.at(i + 1) == ' ') {
            i++;
         }

         csp = 0;

      } else if (c == ':' && csp == 6)  {
         // replace const::A by const ::A
      
         growBuf.addChar(' ');
         growBuf.addChar(':');
         csp = 0;

      } else if (c == 'l' && vsp == 7 && i < l -1 && 
                     !(isId(s.at(i + 1)) || s.at(i + 1) == ')' || s.at(i + 1) == ',')  ) {

         // prevent virtual ::A from being converted to virtual::A
      
         growBuf.addChar('l');
         growBuf.addChar(' ');

         if (s.at(i + 1) == ' ') {
            i++;
         }

         vsp = 0;

      } else if (c == ':' && vsp == 8)  {
         // replace virtual::A by virtual ::A
      
         growBuf.addChar(' ');
         growBuf.addChar(':');

         vsp = 0;

      } else if (! isspace((uchar)c) || 
                 ( i > 0 && i < l - 1 &&   
                   (isId(s.at(i - 1)) || s.at(i - 1) == ')' || s.at(i - 1) == ',' || s.at(i - 1) == '>' || s.at(i - 1) == ']') &&
                   (isId(s.at(i + 1)) ||
                    (i < l - 2 && s.at(i + 1) == '$' && isId(s.at(i + 2))) ||
                    (i < l - 3 && s.at(i + 1) == '&' && s.at(i + 2) == '$' && isId(s.at(i + 3))) ) ) ) {

         if (c == '\t') {
            c = ' ';
         }

         if (c == '*' || c == '&' || c == '@' || c == '$') {
            //uint rl=result.length();
            uint rl = growBuf.getPos();

            if ((rl > 0 && (isId(growBuf.at(rl - 1)) || growBuf.at(rl - 1) == '>')) &&
                  ((c != '*' && c != '&') || !findOperator2(s, i)) ) {

               // avoid splitting operator* and operator->* and operator&
               growBuf.addChar(' ');
            }

         } else if (c == '-') {
            uint rl = growBuf.getPos();

            if (rl > 0 && growBuf.at(rl - 1) == ')' && i < l - 1 && s.at(i + 1) == '>') { 
               // trailing return type ')->' => ') ->'
               growBuf.addChar(' ');
            }
         }

         growBuf.addChar(c);
         if (cliSupport && (c == '^' || c == '%') && i > 1 && isId(s.at(i - 1)) && !findOperator(s, i) ) {
            growBuf.addChar(' '); // C++/CLI: Type^ name and Type% name
         }
      }
   }

   growBuf.addChar(0);
  
   return growBuf.get();
}

/**
 * Returns the position in the string where a function parameter list
 * begins, or -1 if one is not found.
 */
int findParameterList(const QByteArray &name)
{
   int pos = -1;
   int templateDepth = 0;

   do {
      if (templateDepth > 0) {
         int nextOpenPos = name.lastIndexOf('>', pos);
         int nextClosePos = name.lastIndexOf('<', pos);
         if (nextOpenPos != -1 && nextOpenPos > nextClosePos) {
            ++templateDepth;
            pos = nextOpenPos - 1;
         } else if (nextClosePos != -1) {
            --templateDepth;
            pos = nextClosePos - 1;
         } else { // more >'s than <'s, see bug701295
            return -1;
         }
      } else {
         int lastAnglePos = name.lastIndexOf('>', pos);
         int bracePos = name.lastIndexOf('(', pos);
         if (lastAnglePos != -1 && lastAnglePos > bracePos) {
            ++templateDepth;
            pos = lastAnglePos - 1;
         } else {
            int bp = bracePos > 0 ? name.lastIndexOf('(', bracePos - 1) : -1;
            // bp test is to allow foo(int(&)[10]), but we need to make an exception for operator()
            return bp == -1 || (bp >= 8 && name.mid(bp - 8, 10) == "operator()") ? bracePos : bp;
         }
      }
   } while (pos != -1);
   return -1;
}

bool rightScopeMatch(const QByteArray &scope, const QByteArray &name)
{
   int sl = scope.length();
   int nl = name.length();

   return (name == scope || // equal
           (scope.right(nl) == name && // substring
            sl - nl > 1 && scope.at(sl - nl - 1) == ':' && scope.at(sl - nl - 2) == ':'));
}

bool leftScopeMatch(const QByteArray &scope, const QByteArray &name)
{
   int sl = scope.length();
   int nl = name.length();

   return (name == scope || // equal
           (scope.left(nl) == name && // substring
            sl > nl + 1 && scope.at(nl) == ':' && scope.at(nl + 1) == ':') );
}

void linkifyText(const TextGeneratorIntf &out, Definition *scope, FileDef *fileScope, Definition *self,
                 const char *text, bool autoBreak, bool external, bool keepSpaces, int indentLevel)
{
   static QRegExp regExp("[a-z_A-Z\\x80-\\xFF][~!a-z_A-Z0-9$\\\\.:\\x80-\\xFF]*");
   static QRegExp regExpSplit("(?!:),");

   QByteArray txtStr = text;
   int strLen = txtStr.length();
  
   int matchLen;
   int index = 0;
   int newIndex;
   int skipIndex = 0;
   int floatingIndex = 0;

   if (strLen == 0) {
      return;
   }

   // read a word from the text string
   while ((newIndex = regExp.indexIn(txtStr, index)) != -1 && (newIndex == 0 ||
             !(txtStr.at(newIndex - 1) >= '0' && txtStr.at(newIndex - 1) <= '9')) ) {

      // avoid matching part of hex numbers
      // add non-word part to the result

      matchLen = regExp.matchedLength();

      floatingIndex += newIndex - skipIndex + matchLen;
      bool insideString = false;
      int i;

      for (i = index; i < newIndex; i++) {
         if (txtStr.at(i) == '"') {
            insideString = !insideString;
         }
      }
      
      if (strLen > 35 && floatingIndex > 30 && autoBreak) { 
         // try to insert a split point
         QByteArray splitText = txtStr.mid(skipIndex, newIndex - skipIndex);
         int splitLength = splitText.length();
         int offset = 1;

         i = regExpSplit.indexIn(splitText, 0);

         if (i == -1) {
            i = splitText.indexOf('<');
            if (i != -1) {
               offset = 0;
            }
         }

         if (i == -1) {
            i = splitText.indexOf('>');
         }

         if (i == -1) {
            i = splitText.indexOf(' ');
         }
         
         if (i != -1) { 
            // add a link-break at i in case of Html output
            out.writeString(splitText.left(i + offset), keepSpaces);
            out.writeBreak(indentLevel == 0 ? 0 : indentLevel + 1);
            out.writeString(splitText.right(splitLength - i - offset), keepSpaces);
            floatingIndex = splitLength - i - offset + matchLen;

         } else {
            out.writeString(splitText, keepSpaces);
         }

      } else {
         //ol.docify(txtStr.mid(skipIndex,newIndex-skipIndex));
         out.writeString(txtStr.mid(skipIndex, newIndex - skipIndex), keepSpaces);
      }

      // get word from string
      QByteArray word = txtStr.mid(newIndex, matchLen);
      QByteArray matchWord = substitute(substitute(word, "\\", "::"), ".", "::");

      bool found = false;

      if (! insideString) {
         ClassDef     *cd = 0;
         FileDef      *fd = 0;
         MemberDef    *md = 0;
         NamespaceDef *nd = 0;
         GroupDef     *gd = 0;
        
         MemberDef *typeDef = 0;
         cd = getResolvedClass(scope, fileScope, matchWord, &typeDef);

         if (typeDef) { 
            // First look at typedef then class, see bug 584184.
           
            if (external ? typeDef->isLinkable() : typeDef->isLinkableInProject()) {
               if (typeDef->getOuterScope() != self) { out.writeLink(typeDef->getReference(),
                                typeDef->getOutputFileBase(), typeDef->anchor(), word);
                  found = true;
               }
            }
         }

         if (!found && (cd || (cd = getClass(matchWord)))) {
            //printf("Found class %s\n",cd->name().data());
            // add link to the result
            if (external ? cd->isLinkable() : cd->isLinkableInProject()) {
               if (cd != self) {
                  out.writeLink(cd->getReference(), cd->getOutputFileBase(), cd->anchor(), word);
                  found = true;
               }
            }

         } else if ((cd = getClass(matchWord + "-p"))) { // search for Obj-C protocols as well
            // add link to the result
            if (external ? cd->isLinkable() : cd->isLinkableInProject()) {
               if (cd != self) {
                  out.writeLink(cd->getReference(), cd->getOutputFileBase(), cd->anchor(), word);
                  found = true;
               }
            }
           
         }

         int m = matchWord.lastIndexOf("::");
         QByteArray scopeName;

         if (scope && (scope->definitionType() == Definition::TypeClass ||
             scope->definitionType() == Definition::TypeNamespace) ) {
            scopeName = scope->name();

         } else if (m != -1) {
            scopeName = matchWord.left(m);
            matchWord = matchWord.mid(m + 2);
         }
         
         if (!found && getDefs(scopeName, matchWord, 0, md, cd, fd, nd, gd) && 
               (external ? md->isLinkable() : md->isLinkableInProject()) ) {
        
            if (md != self && (self == 0 || md->name() != self->name())) {
               // name check is needed for overloaded members, where getDefs just returns one
            
               out.writeLink(md->getReference(), md->getOutputFileBase(), md->anchor(), word);              
               found = true;
            }
         }
      }

      if (!found) { 
         // add word to the result
         out.writeString(word, keepSpaces);
      }
      // set next start point in the string
     
      skipIndex = index = newIndex + matchLen;
   }
  
   out.writeString(txtStr.right(txtStr.length() - skipIndex), keepSpaces);
}

void writeExample(OutputList &ol, ExampleSDict *ed)
{
   QByteArray exampleLine = theTranslator->trWriteList(ed->count());

   //bool latexEnabled = ol.isEnabled(OutputGenerator::Latex);
   //bool manEnabled   = ol.isEnabled(OutputGenerator::Man);
   //bool htmlEnabled  = ol.isEnabled(OutputGenerator::Html);

   QRegExp marker("@[0-9]+");
   int index = 0, newIndex, matchLen;

   auto iter = ed->begin();

   // now replace all markers in inheritLine with links to the classes
   while ((newIndex = marker.indexIn(exampleLine, index)) != -1) {
      matchLen = marker.matchedLength();
      
      ol.parseText(exampleLine.mid(index, newIndex - index));     
      QSharedPointer<Example> e = *iter;

      if (e) {
         ol.pushGeneratorState();   

         ol.disable(OutputGenerator::Latex);
         ol.disable(OutputGenerator::RTF);

         // link for Html / man         
         ol.writeObjectLink(0, e->file, e->anchor, e->name);
         ol.popGeneratorState();

         ol.pushGeneratorState();
         
         ol.disable(OutputGenerator::Man);
         ol.disable(OutputGenerator::Html);

         // link for Latex / pdf with anchor because the sources
         // are not hyperlinked (not possible with a verbatim environment)

         ol.writeObjectLink(0, e->file, 0, e->name);  
         ol.popGeneratorState();
      }

      index = newIndex + matchLen;
   }

   ol.parseText(exampleLine.right(exampleLine.length() - index));
   ol.writeString(".");
}

QByteArray argListToString(ArgumentList *al, bool useCanonicalType, bool showDefVals)
{
   QByteArray result;

   if (al == nullptr) {
      return result;
   }
      
   result += "(";

   auto nextItem = al->begin();

   for (auto a : *al)  {
      ++nextItem;

      QByteArray type1 = useCanonicalType && ! a.canType.isEmpty() ? a.canType : a.type;
      QByteArray type2;

      // hack to deal with function pointers
      int i = type1.indexOf(")("); 

      if (i != -1) {
         type2 = type1.mid(i);
         type1 = type1.left(i);
      }

      if (! a.attrib.isEmpty()) {
         result += a.attrib + " ";
      }

      if (! a.name.isEmpty() || ! a.array.isEmpty()) {
         result += type1 + " " + a.name + type2 + a.array;
      } else {
         result += type1 + type2;
      }

      if (! a.defval.isEmpty() && showDefVals) {
         result += "=" + a.defval;
      }
      
      if (nextItem != al->end()) {
         result += ", ";
      }
   }

   result += ")";

   if (al->constSpecifier) {
      result += " const";
   }

   if (al->volatileSpecifier) {
      result += " volatile";
   }

   if (!al->trailingReturnType.isEmpty()) {
      result += " -> " + al->trailingReturnType;
   }

   if (al->pureSpecifier) {
      result += " =0";
   }

   return removeRedundantWhiteSpace(result);
}

QByteArray tempArgListToString(const ArgumentList *al, SrcLangExt lang)
{
   QByteArray result;

   if (al == 0) {
      return result;
   }

   result = "<";

   auto nextItem = al->begin();

   for (auto a : *al)  {
      ++nextItem;
 
      if (! a.name.isEmpty()) { 
         // add template argument name

         if (a.type.left(4) == "out") { // C# covariance
            result += "out ";

         } else if (a.type.left(3) == "in") { // C# contravariance
            result += "in ";
         }
     
         if (lang == SrcLangExt_Java || lang == SrcLangExt_CSharp) {
            result += a.type + " ";
         }

         result += a.name;

      } else { 
         // extract name from type
         int i = a.type.length() - 1;

         while (i >= 0 && isId(a.type.at(i))) {
            i--;
         }
     
        if (i > 0) {
            result += a.type.right(a.type.length() - i - 1);
         } else { 
            // nothing found -> take whole name
            result += a.type;
         }
      }
 
      if (nextItem != al->end()) {
         result += ", ";
      }
   }

   result += ">";

   return removeRedundantWhiteSpace(result);
}

// compute the HTML anchors for a list of members
void setAnchors(MemberList *ml)
{ 
   if (ml == 0) {
      return;
   }
 
   for (auto md : *ml) {
      if (!md->isReference()) {         
         md->setAnchor();        
      }
   }
}


/*! takes the \a buf of the given length \a len and converts CR LF (DOS)
 * or CR (MAC) line ending to LF (Unix).  Returns the length of the
 * converted content (i.e. the same as \a len (Unix, MAC) or
 * smaller (DOS).
 */
int filterCRLF(char *buf, int len)
{
   int src = 0;    // source index
   int dest = 0;   // destination index
   char c;         // current character

   while (src < len) {
      c = buf[src++];            // Remember the processed character.

      if (c == '\r') {           // CR to be solved (MAC, DOS)
         c = '\n';                // each CR to LF
         if (src < len && buf[src] == '\n') {
            ++src;   // skip LF just after CR (DOS)
         }

      } else if ( c == '\0' && src < len - 1) { // filter out internal \0 characters, as it will confuse the parser
         c = ' ';                 // turn into a space
      }

      buf[dest++] = c;           // copy the (modified) character to dest
   }

   return dest;                 // length of the valid part of the buf
}

static QString getFilterFromList(const char *name, const QStringList &filterList, bool &found)
{
   found = false;

   // compare the file name to the filter pattern list  
   for (auto filterStr : filterList) {
      QString fs = filterStr;

      int i_equals = fs.indexOf('=');

      if (i_equals != -1) {
         QString filterPattern = fs.left(i_equals);

         QRegExp fpat(filterPattern, portable_fileSystemIsCaseSensitive(), QRegExp::Wildcard);

         if (fpat.indexIn(name) != -1) {
            // found a match
            QString filterName = fs.mid(i_equals + 1);

            if (filterName.indexOf(' ') != -1) {
               // add quotes if the name has spaces
               filterName = "\"" + filterName + "\"";
            }

            found = true;
            return filterName;
         }
      }
   }

   // no match
   return "";
}

/*! looks for a filter for the file \a name.  Returns the name of the filter
 *  if there is a match for the file name, otherwise an empty string.
 *  In case \a inSourceCode is true then first the source filter list is
 *  considered.
 */
QString getFileFilter(const char *name, bool isSourceCode)
{
   // sanity check
   if (name == 0) {
      return "";
   }

   QStringList &filterSrcList = Config_getList("FILTER_SOURCE_PATTERNS");
   QStringList &filterList    = Config_getList("FILTER_PATTERNS");

   QString filterName;
   bool found = false;

   if (isSourceCode && !filterSrcList.isEmpty()) {
      // first look for source filter pattern list
      filterName = getFilterFromList(name, filterSrcList, found);
   }

   if (! found && filterName.isEmpty()) {
      // then look for filter pattern list
      filterName = getFilterFromList(name, filterList, found);
   }

   if (! found) {
      // then use the generic input filter
      return Config_getString("INPUT_FILTER");

   } else {
      return filterName;
   }
}

QByteArray transcodeCharacterStringToUTF8(const QByteArray &input)
{
   bool error = false;

   static QByteArray inputEncoding = Config_getString("INPUT_ENCODING");
   const char *outputEncoding = "UTF-8";
 
   if (inputEncoding.isEmpty() || qstricmp(inputEncoding, outputEncoding) == 0) {
      return input;
   }

   QTextCodec *temp = QTextCodec::codecForName(inputEncoding);
   
   if (! temp) {
      err("Unsupported character conversion: '%s'->'%s'\n", inputEncoding.constData(), outputEncoding);
      return input;
   }

   return temp->toUnicode(input).toUtf8();   
}

/*! reads a file with name \a name and returns it as a string. If \a filter
 *  is true the file will be filtered by any user specified input filter.
 *  If \a name is "-" the string will be read from standard input.
 */
QByteArray fileToString(const char *name, bool filter, bool isSourceCode)
{
   if (name == 0 || name[0] == 0) {
      return 0;
   }

   QFile f;

   bool fileOpened = false;

   if (name[0] == '-' && name[1] == 0) { 
      // read from stdin
      fileOpened = f.open(stdin, QIODevice::ReadOnly);

      if (fileOpened) {
        
         QByteArray contents;         
         contents = f.readAll();            

         int totalSize = filterCRLF(contents.data(), contents.size());
         contents.resize(totalSize);
         
         contents.append('\n');    // to help the scanner

         return contents;
      }

   } else { // read from file
      QFileInfo fi(name);

      if (! fi.exists() || ! fi.isFile()) {
         err("File `%s' not found\n", name);
         return "";
      }

      BufStr buf(fi.size());
      fileOpened = readInputFile(name, buf, filter, isSourceCode);

      if (fileOpened) {
         int s = buf.size();

         if (s > 1 && buf.at(s - 2) != '\n') {
            buf.at(s - 1) = '\n';
            buf.addChar(0);
         }

         return buf.data();
      }
   }

   if (! fileOpened) {
      err("Unable to open file `%s' for reading\n", name);
   }

   return "";
}

QByteArray dateToString(bool includeTime)
{
   QDateTime current = QDateTime::currentDateTime();
   return theTranslator->trDateTime(current.date().year(),
                                    current.date().month(),
                                    current.date().day(),
                                    current.date().dayOfWeek(),
                                    current.time().hour(),
                                    current.time().minute(),
                                    current.time().second(),
                                    includeTime);
}

static QString yearToString()
{
   const QDate &d = QDate::currentDate();

   QString result;
   result = QString("%1").arg(d.year());

   return result;
}

// recursive function that returns the number of branches in the
// inheritance tree that the base class `bcd' is below the class `cd'
int minClassDistance(const ClassDef *cd, const ClassDef *bcd, int level)
{
   if (bcd->categoryOf())  {
      // use class that is being extended in case of, an Objective-C category
   
      bcd = bcd->categoryOf();
   }

   if (cd == bcd) {
      return level;
   }

   if (level == 256) {
      warn_uncond("class %s seem to have a recursive inheritance relation!\n", cd->name().data());
      return -1;
   }

   int m = maxInheritanceDepth;

   if (cd->baseClasses()) {
      
      for (auto bcdi : *cd->baseClasses()) {
         int mc = minClassDistance(bcdi->classDef, bcd, level + 1);
         if (mc < m) {
            m = mc;
         }

         if (m < 0) {
            break;
         }
      }
   }

   return m;
}

Protection classInheritedProtectionLevel(ClassDef *cd, ClassDef *bcd, Protection prot, int level)
{
   if (bcd->categoryOf())  {
      // use class that is being extended in case of
      // an Objective-C category
   
      bcd = bcd->categoryOf();
   }

   if (cd == bcd) {
      return prot;
   }

   if (level == 256) {
      err("Internal issue found in class %s: recursive inheritance relation problem." 
            "Please submit a bug report\n", cd->name().data() );

   } else if (cd->baseClasses()) {

      for (auto bcdi : *cd->baseClasses()) {

         if (prot == Private) {
            break;
         }

         Protection baseProt = classInheritedProtectionLevel(bcdi->classDef, bcd, bcdi->prot, level + 1);

         if (baseProt == Private) {
            prot = Private;

         } else if (baseProt == Protected) {
            prot = Protected;
         }
      }
   }
   
   return prot;
}

//static void printArgList(ArgumentList *al)
//{
//  if (al==0) return;
//  ArgumentListIterator ali(*al);
//  Argument *a;
//  printf("(");
//  for (;(a=ali.current());++ali)
//  {
//    printf("t=`%s' n=`%s' v=`%s' ",a->type.data(),!a->name.isEmpty()>0?a->name.data():"",!a->defval.isEmpty()>0?a->defval.data():"");
//  }
//  printf(")");
//}

#ifndef NEWMATCH
// strip any template specifiers that follow className in string s
static QByteArray trimTemplateSpecifiers(const QByteArray &namespaceName, 
            const QByteArray &className, const QByteArray &s)
{  
   QByteArray scopeName = mergeScopes(namespaceName, className);
   ClassDef *cd = getClass(scopeName);

   if (cd == 0) {
      return s;   // should not happen, but guard anyway.
   }

   QByteArray result = s;

   int i = className.length() - 1;

   if (i >= 0 && className.at(i) == '>') { // template specialization
      // replace unspecialized occurrences in s, with their specialized versions.
      int count = 1;
      int cl = i + 1;

      while (i >= 0) {
         char c = className.at(i);

         if (c == '>') {
            count++, i--;

         } else if (c == '<') {
            count--;
            if (count == 0) {
               break;
            }

         } else {
            i--;
         }
      }

      QByteArray unspecClassName = className.left(i);
      int l = i;
      int p = 0;

      while ((i = result.indexOf(unspecClassName, p)) != -1) {
         if (result.at(i + l) != '<') { // unspecialized version

            result = result.left(i) + className + result.right(result.length() - i - l);
            l = cl;
         }
         p = i + l;
      }
   }

   QByteArray qualName = cd->qualifiedNameWithTemplateParameters();

   //printf("QualifiedName = %s\n",qualName.data());
   // We strip the template arguments following className (if any)

   if (! qualName.isEmpty()) { 
      // there is a class name
      int is, ps = 0;
      int p = 0, l, i;

      while ((is = getScopeFragment(qualName, ps, &l)) != -1) {
         QByteArray qualNamePart = qualName.right(qualName.length() - is);

         while ((i = result.indexOf(qualNamePart, p)) != -1) {
            int ql = qualNamePart.length();
            result = result.left(i) + cd->name() + result.right(result.length() - i - ql);
            p = i + cd->name().length();
         }
         ps = is + l;
      }
   }

   return result.trimmed();
}

/*!
 * @param pattern pattern to look for
 * @param s string to search in
 * @param p position to start
 * @param len resulting pattern length
 * @returns position on which string is found, or -1 if not found
 */
static int findScopePattern(const QByteArray &pattern, const QByteArray &s, int p, int *len)
{
   int sl = s.length();
   int pl = pattern.length();
   int sp = 0;
   *len = 0;

   while (p < sl) {
      sp = p; // start of match
      int pp = 0; // pattern position

      while (p < sl && pp < pl) {
         if (s.at(p) == '<') { // skip template arguments while matching
            int bc = 1;
           
            p++;

            while (p < sl) {
               if (s.at(p) == '<') {
                  bc++;

               } else if (s.at(p) == '>') {
                  bc--;
                  if (bc == 0) {
                     p++;
                     break;
                  }
               }
               
               p++;
            }
         } else if (s.at(p) == pattern.at(pp)) {
            //printf("match at position p=%d pp=%d c=%c\n",p,pp,s.at(p));
            p++;
            pp++;

         } else { // no match
            //printf("restarting at %d c=%c pat=%s\n",p,s.at(p),pattern.data());
            p = sp + 1;
            break;
         }
      }

      if (pp == pl) { // whole pattern matches
         *len = p - sp;
         return sp;
      }
   }

   return -1;
}

static QByteArray trimScope(const QByteArray &name, const QByteArray &s)
{
   int scopeOffset = name.length();
   QByteArray result = s;

   do { // for each scope
      QByteArray tmp;
      QByteArray scope = name.left(scopeOffset) + "::";
      
      int i, p = 0, l;

      while ((i = findScopePattern(scope, result, p, &l)) != -1) { // for each occurrence
         tmp += result.mid(p, i - p); // add part before pattern
         p = i + l;
      }

      tmp += result.right(result.length() - p); // add trailing part

      scopeOffset = name.lastIndexOf("::", scopeOffset - 1);
      result = tmp;
   } while (scopeOffset > 0);
   
   return result;
}
#endif

void trimBaseClassScope(SortedList<BaseClassDef *> *bcl, QByteArray &s, int level = 0)
{ 
   for (auto bcd : *bcl) {
      ClassDef *cd = bcd->classDef;
      
      int spos = s.indexOf(cd->name() + "::");

      if (spos != -1) {
         s = s.left(spos) + s.right(
                s.length() - spos - cd->name().length() - 2
             );
      }
      
      if (cd->baseClasses()) {
         trimBaseClassScope(cd->baseClasses(), s, level + 1);
      }
   }
}

#if 0
/*! if either t1 or t2 contains a namespace scope, then remove that
 *  scope. If neither or both have a namespace scope, t1 and t2 remain
 *  unchanged.
 */
static void trimNamespaceScope(QByteArray &t1, QByteArray &t2, const QByteArray &nsName)
{
   int p1 = t1.length();
   int p2 = t2.length();

   for (;;) {
      int i1 = p1 == 0 ? -1 : t1.lastIndexOf("::", p1);
      int i2 = p2 == 0 ? -1 : t2.lastIndexOf("::", p2);
      if (i1 == -1 && i2 == -1) {
         return;
      }
      if (i1 != -1 && i2 == -1) { // only t1 has a scope
         QByteArray scope = t1.left(i1);
         replaceNamespaceAliases(scope, i1);

         int so = nsName.length();
         do {
            QByteArray fullScope = nsName.left(so);
            if (!fullScope.isEmpty() && !scope.isEmpty()) {
               fullScope += "::";
            }
            fullScope += scope;
            if (!fullScope.isEmpty() && Doxygen::namespaceSDict[fullScope] != 0) { // scope is a namespace
               t1 = t1.right(t1.length() - i1 - 2);
               return;
            }
            if (so == 0) {
               so = -1;
            } else if ((so = nsName.lastIndexOf("::", so - 1)) == -1) {
               so = 0;
            }
         } while (so >= 0);
      } else if (i1 == -1 && i2 != -1) { // only t2 has a scope
         QByteArray scope = t2.left(i2);
         replaceNamespaceAliases(scope, i2);

         int so = nsName.length();
         do {
            QByteArray fullScope = nsName.left(so);
            if (!fullScope.isEmpty() && !scope.isEmpty()) {
               fullScope += "::";
            }
            fullScope += scope;
            if (!fullScope.isEmpty() && Doxygen::namespaceSDict[fullScope] != 0) { // scope is a namespace
               t2 = t2.right(t2.length() - i2 - 2);
               return;
            }
            if (so == 0) {
               so = -1;
            } else if ((so = nsName.lastIndexOf("::", so - 1)) == -1) {
               so = 0;
            }
         } while (so >= 0);
      }
      p1 = qMax(i1 - 2, 0);
      p2 = qMax(i2 - 2, 0);
   }
}
#endif

static void stripIrrelevantString(QByteArray &target, const QByteArray &str)
{
   if (target == str) {
      target.resize(0);
      return;
   }

   int i, p = 0;
   int l = str.length();
   bool changed = false;

   while ((i = target.indexOf(str, p)) != -1) {
      bool isMatch = (i == 0 || ! isId(target.at(i - 1))) && // not a character before str
                     (i + l == (int)target.length() || !isId(target.at(i + l))); // not a character after str

      if (isMatch) {
         int i1 = target.indexOf('*', i + l);
         int i2 = target.indexOf('&', i + l);

         if (i1 == -1 && i2 == -1) {
            // strip str from target at index i
            target = target.left(i) + target.right(target.length() - i - l);
            changed = true;
            i -= l;

         } else if ((i1 != -1 && i < i1) || (i2 != -1 && i < i2)) { // str before * or &
            // move str to front
            target = str + " " + target.left(i) + target.right(target.length() - i - l);
            changed = true;
            i++;
         }
      }

      p = i + l;
   }

   if (changed) {
      target = target.trimmed();
   }
}

/*! According to the C++ spec and Ivan Vecerina:

  Parameter declarations  that differ only in the presence or absence
  of const and/or volatile are equivalent.

  So the following example, show what is stripped by this routine
  for const. The same is done for volatile.

  \code
  const T param     ->   T param          // not relevant
  const T& param    ->   const T& param   // const needed
  T* const param    ->   T* param         // not relevant
  const T* param    ->   const T* param   // const needed
  \endcode
 */
void stripIrrelevantConstVolatile(QByteArray &s)
{
   //printf("stripIrrelevantConstVolatile(%s)=",s.data());
   stripIrrelevantString(s, "const");
   stripIrrelevantString(s, "volatile");
   //printf("%s\n",s.data());
}

#ifndef NEWMATCH
static bool matchArgument(const Argument *srcA, const Argument *dstA, const QByteArray &className,
                          const QByteArray &namespaceName, NamespaceSDict *usingNamespaces, 
                          StringMap<QSharedPointer<Definition>> *usingClasses)
{
   // TODO: resolve any typedefs names that are part of srcA->type
   //       before matching. This should use className and namespaceName
   //       and usingNamespaces and usingClass to determine which typedefs
   //       are in-scope, so it will not be very efficient :-(

   QByteArray srcAType = trimTemplateSpecifiers(namespaceName, className, srcA->type);
   QByteArray dstAType = trimTemplateSpecifiers(namespaceName, className, dstA->type);
   QByteArray srcAName = srcA->name.trimmed();
   QByteArray dstAName = dstA->name.trimmed();

   srcAType = stripPrefix(srcAType, "class ");
   dstAType = stripPrefix(dstAType, "class ");

   // allow distinguishing "const A" from "const B" even though
   // from a syntactic point of view they would be two names of the same
   // type "const". This is not fool prove of course, but should at least
   // catch the most common cases.

   if ((srcAType == "const" || srcAType == "volatile") && !srcAName.isEmpty()) {
      srcAType += " ";
      srcAType += srcAName;
   }

   if ((dstAType == "const" || dstAType == "volatile") && !dstAName.isEmpty()) {
      dstAType += " ";
      dstAType += dstAName;
   }

   if (srcAName == "const" || srcAName == "volatile") {
      srcAType += srcAName;
      srcAName.resize(0);

   } else if (dstA->name == "const" || dstA->name == "volatile") {
      dstAType += dstA->name;
      dstAName.resize(0);
   }

   stripIrrelevantConstVolatile(srcAType);
   stripIrrelevantConstVolatile(dstAType);

   // strip typename keyword
   if (qstrncmp(srcAType, "typename ", 9) == 0) {
      srcAType = srcAType.right(srcAType.length() - 9);
   }
   if (qstrncmp(dstAType, "typename ", 9) == 0) {
      dstAType = dstAType.right(dstAType.length() - 9);
   }

   srcAType = removeRedundantWhiteSpace(srcAType);
   dstAType = removeRedundantWhiteSpace(dstAType);
   

   if (srcA->array != dstA->array) { 
      // nomatch for char[] against char
      DOX_NOMATCH
      return false;
   }

   if (srcAType != dstAType) { 
      // check if the argument only differs on name

      // remove a namespace scope that is only in one type
      // (assuming a using statement was used)
      //printf("Trimming %s<->%s: %s\n",srcAType.data(),dstAType.data(),namespaceName.data());
      //trimNamespaceScope(srcAType,dstAType,namespaceName);
      //printf("After Trimming %s<->%s\n",srcAType.data(),dstAType.data());
    

      // strip redundant scope specifiers
      if (!className.isEmpty()) {
         srcAType = trimScope(className, srcAType);
         dstAType = trimScope(className, dstAType);

         //printf("trimScope: `%s' <=> `%s'\n",srcAType.data(),dstAType.data());

         ClassDef *cd;
         if (!namespaceName.isEmpty()) {
            cd = getClass(namespaceName + "::" + className);
         } else {
            cd = getClass(className);
         }
         if (cd && cd->baseClasses()) {
            trimBaseClassScope(cd->baseClasses(), srcAType);
            trimBaseClassScope(cd->baseClasses(), dstAType);
         }
         //printf("trimBaseClassScope: `%s' <=> `%s'\n",srcAType.data(),dstAType.data());
      }

      if (!namespaceName.isEmpty()) {
         srcAType = trimScope(namespaceName, srcAType);
         dstAType = trimScope(namespaceName, dstAType);
      }

      if (usingNamespaces && usingNamespaces->count() > 0) {
        
         for (auto nd : *usingNamespaces) {
            srcAType = trimScope(nd->name(), srcAType);
            dstAType = trimScope(nd->name(), dstAType);
         }
      }

      if (usingClasses && usingClasses->count() > 0) {       
         for (auto cd : *usingClasses) {
            srcAType = trimScope(cd->name(), srcAType);
            dstAType = trimScope(cd->name(), dstAType);
         }
      }
 
      if (!srcAName.isEmpty() && !dstA->type.isEmpty() && (srcAType + " " + srcAName) == dstAType) {
         DOX_MATCH
         return true;

      } else if (!dstAName.isEmpty() && !srcA->type.isEmpty() && (dstAType + " " + dstAName) == srcAType) {
         DOX_MATCH
         return true;
      }

      uint srcPos = 0, dstPos = 0;
      bool equal = true;

      while (srcPos < srcAType.length() && dstPos < dstAType.length() && equal) {
         equal = srcAType.at(srcPos) == dstAType.at(dstPos);

         if (equal) {
            srcPos++, dstPos++;
         }
      }
      uint srcATypeLen = srcAType.length();
      uint dstATypeLen = dstAType.length();
      if (srcPos < srcATypeLen && dstPos < dstATypeLen) {
         // if nothing matches or the match ends in the middle or at the
         // end of a string then there is no match
         if (srcPos == 0 || dstPos == 0) {
            DOX_NOMATCH
            return false;
         }

         if (isId(srcAType.at(srcPos)) && isId(dstAType.at(dstPos))) {
            //printf("partial match srcPos=%d dstPos=%d!\n",srcPos,dstPos);
            // check if a name if already found -> if no then there is no match
            if (!srcAName.isEmpty() || !dstAName.isEmpty()) {
               DOX_NOMATCH
               return false;
            }
            // types only
            while (srcPos < srcATypeLen && isId(srcAType.at(srcPos))) {
               srcPos++;
            }
            while (dstPos < dstATypeLen && isId(dstAType.at(dstPos))) {
               dstPos++;
            }
            if (srcPos < srcATypeLen ||
                  dstPos < dstATypeLen ||
                  (srcPos == srcATypeLen && dstPos == dstATypeLen)
               ) {
               DOX_NOMATCH
               return false;
            }
         } else {
            // otherwise we assume that a name starts at the current position.
            while (srcPos < srcATypeLen && isId(srcAType.at(srcPos))) {
               srcPos++;
            }
            while (dstPos < dstATypeLen && isId(dstAType.at(dstPos))) {
               dstPos++;
            }

            // if nothing more follows for both types then we assume we have
            // found a match. Note that now `signed int' and `signed' match, but
            // seeing that int is not a name can only be done by looking at the
            // semantics.

            if (srcPos != srcATypeLen || dstPos != dstATypeLen) {
               DOX_NOMATCH
               return false;
            }
         }
      } else if (dstPos < dstAType.length()) {
         if (!isspace((uchar)dstAType.at(dstPos))) { // maybe the names differ
            if (!dstAName.isEmpty()) { // dst has its name separated from its type
               DOX_NOMATCH
               return false;
            }
            while (dstPos < dstAType.length() && isId(dstAType.at(dstPos))) {
               dstPos++;
            }
            if (dstPos != dstAType.length()) {
               DOX_NOMATCH
               return false; // more than a difference in name -> no match
            }
         } else { // maybe dst has a name while src has not
            dstPos++;
            while (dstPos < dstAType.length() && isId(dstAType.at(dstPos))) {
               dstPos++;
            }
            if (dstPos != dstAType.length() || !srcAName.isEmpty()) {
               DOX_NOMATCH
               return false; // nope not a name -> no match
            }
         }
      } else if (srcPos < srcAType.length()) {
         if (!isspace((uchar)srcAType.at(srcPos))) { // maybe the names differ
            if (!srcAName.isEmpty()) { // src has its name separated from its type
               DOX_NOMATCH
               return false;
            }
            while (srcPos < srcAType.length() && isId(srcAType.at(srcPos))) {
               srcPos++;
            }
            if (srcPos != srcAType.length()) {
               DOX_NOMATCH
               return false; // more than a difference in name -> no match
            }
         } else { // maybe src has a name while dst has not
            srcPos++;
            while (srcPos < srcAType.length() && isId(srcAType.at(srcPos))) {
               srcPos++;
            }
            if (srcPos != srcAType.length() || !dstAName.isEmpty()) {
               DOX_NOMATCH
               return false; // nope not a name -> no match
            }
         }
      }
   }
   DOX_MATCH
   return true;
}

/*!
 * Matches the arguments list srcAl with the argument list dstAl
 * Returns true if the argument lists are equal. Two argument list are
 * considered equal if the number of arguments is equal and the types of all
 * arguments are equal. Furthermore the const and volatile specifiers
 * stored in the list should be equal.
 */
bool matchArguments(ArgumentList *srcAl, ArgumentList *dstAl, const char *cl, const char *ns, bool checkCV,
                    NamespaceSDict *usingNamespaces, StringMap<QSharedPointer<Definition>> *usingClasses)
{
   QByteArray className = cl;
   QByteArray namespaceName = ns;

   if (srcAl == 0 || dstAl == 0) {
      bool match = srcAl == dstAl; // at least one of the members is not a function

      if (match) {
         DOX_MATCH
         return true;

      } else {
         DOX_NOMATCH
         return false;
      }
   }

   // handle special case with void argument
   if (srcAl->count() == 0 && dstAl->count() == 1 && dstAl->first().type == "void" ) {
      // special case for finding match between func() and func(void)

      Argument a;
      a.type = "void";
      srcAl->append(a);

      DOX_MATCH
      return true;
   }

   if ( dstAl->count() == 0 && srcAl->count() == 1 && srcAl->first().type == "void" ) {
      // special case for finding match between func(void) and func()

      Argument a;
      a.type = "void";
      dstAl->append(a);

      DOX_MATCH
      return true;
   }

   if (srcAl->count() != dstAl->count()) {
      DOX_NOMATCH
      return false; // different number of arguments -> no match
   }

   if (checkCV) {
      if (srcAl->constSpecifier != dstAl->constSpecifier) {
         DOX_NOMATCH
         return false; // one member is const, the other not -> no match
      }

      if (srcAl->volatileSpecifier != dstAl->volatileSpecifier) {
         DOX_NOMATCH
         return false; // one member is volatile, the other not -> no match
      }
   }

   // so far the argument list could match, so we need to compare the types of all arguments  
   auto item = dstAl->begin();

   for (auto &srcA : *srcAl ) {      
      if (! matchArgument(&srcA, &(*item), className, namespaceName, usingNamespaces, usingClasses)) {
         DOX_NOMATCH
         return false;
      }

      ++item;
   }

   DOX_MATCH

   return true; // all arguments match
}

#endif

#if 0
static QByteArray resolveSymbolName(FileDef *fs, Definition *symbol, QByteArray &templSpec)
{
   assert(symbol != 0);
   if (symbol->definitionType() == Definition::TypeMember && ((MemberDef *)symbol)->isTypedef())  {
      // if symbol is a typedef then try to resolve it
   
      MemberDef *md = 0;
      ClassDef *cd = newResolveTypedef(fs, (MemberDef *)symbol, &md, &templSpec);

      if (cd) {
         return cd->qualifiedName() + templSpec;

      } else if (md) {
         return md->qualifiedName();
      }
   }

   return symbol->qualifiedName();
}
#endif

static QByteArray stripDeclKeywords(const QByteArray &s)
{
   int i = s.indexOf(" class ");
   if (i != -1) {
      return s.left(i) + s.mid(i + 6);
   }

   i = s.indexOf(" typename ");
   if (i != -1) {
      return s.left(i) + s.mid(i + 9);
   }

   i = s.indexOf(" union ");
   if (i != -1) {
      return s.left(i) + s.mid(i + 6);
   }

   i = s.indexOf(" struct ");
   if (i != -1) {
      return s.left(i) + s.mid(i + 7);
   }

   return s;
}

// forward decl for circular dependencies
static QByteArray extractCanonicalType(Definition *d, FileDef *fs, QByteArray type);

QByteArray getCanonicalTemplateSpec(Definition *d, FileDef *fs, const QByteArray &spec)
{
   QByteArray templSpec = spec.trimmed();

   if (! templSpec.isEmpty() && templSpec.at(0) == '<') {
      templSpec = "< " + extractCanonicalType(d, fs, templSpec.right(templSpec.length() - 1).trimmed());
   }

   QByteArray resolvedType = resolveTypeDef(d, templSpec);

   if (!resolvedType.isEmpty()) { // not known as a typedef either
      templSpec = resolvedType;
   }
   
   return templSpec;
}


static QByteArray getCanonicalTypeForIdentifier(Definition *d, FileDef *fs, const QByteArray &word, 
                                                QByteArray *tSpec, int count = 0)
{
   if (count > 10) {
      return word;   // oops recursion
   }

   QByteArray symName, scope, result, templSpec, tmpName;
 
   if (tSpec && !tSpec->isEmpty()) {
      templSpec = stripDeclKeywords(getCanonicalTemplateSpec(d, fs, *tSpec));
   }

   if (word.lastIndexOf("::") != -1 && !(tmpName = stripScope(word)).isEmpty()) {
      symName = tmpName; // name without scope
   } else {
      symName = word;
   }

   ClassDef *cd = 0;
   MemberDef *mType = 0;
   QByteArray ts;
   QByteArray resolvedType;

   // lookup class / class template instance
   cd = getResolvedClass(d, fs, word + templSpec, &mType, &ts, true, true, &resolvedType);

   bool isTemplInst = cd && !templSpec.isEmpty();

   if (! cd && ! templSpec.isEmpty()) {
      // class template specialization not known, look up class template
      cd = getResolvedClass(d, fs, word, &mType, &ts, true, true, &resolvedType);
   }

   if (cd && cd->isUsedOnly()) {
      cd = 0;   // ignore types introduced by usage relations
   }
 
   if (cd) { 
      // resolves to a known class type

      if (cd == d && tSpec) {
         *tSpec = "";
      }

      if (mType && mType->isTypedef()) { // but via a typedef
         result = resolvedType + ts; // the +ts was added for bug 685125

      } else {
         if (isTemplInst) {
            // spec is already part of class type
            templSpec = "";
            if (tSpec) {
               *tSpec = "";
            }
         } else if (!ts.isEmpty() && templSpec.isEmpty()) {
            // use formal template args for spec
            templSpec = stripDeclKeywords(getCanonicalTemplateSpec(d, fs, ts));
         }

         result = removeRedundantWhiteSpace(cd->qualifiedName() + templSpec);

         if (cd->isTemplate() && tSpec) { //
            if (!templSpec.isEmpty()) { // specific instance
               result = cd->name() + templSpec;
            } else { // use template type
               result = cd->qualifiedNameWithTemplateParameters();
            }
            // template class, so remove the template part (it is part of the class name)
            *tSpec = "";
         } else if (ts.isEmpty() && !templSpec.isEmpty() && cd && !cd->isTemplate() && tSpec) {
            // obscure case, where a class is used as a template, but doxygen think it is
            // not (could happen when loading the class from a tag file).
            *tSpec = "";
         }
      }

   } else if (mType && mType->isEnumerate()) { // an enum
      result = mType->qualifiedName();

   } else if (mType && mType->isTypedef()) { // a typedef     
      if (word != mType->typeString()) {
         result = getCanonicalTypeForIdentifier(d, fs, mType->typeString(), tSpec, count + 1);
      } else {
         result = mType->typeString();
      }

   } else { // fallback
      resolvedType = resolveTypeDef(d, word);
     
      if (resolvedType.isEmpty()) { // not known as a typedef either
         result = word;
      } else {
         result = resolvedType;
      }
   }
   
   return result;
}

static QByteArray extractCanonicalType(Definition *d, FileDef *fs, QByteArray type)
{
   type = type.trimmed();

   // strip const and volatile keywords that are not relevant for the type
   stripIrrelevantConstVolatile(type);

   // strip leading keywords
   type = stripPrefix(type, "class ");
   type = stripPrefix(type, "struct ");
   type = stripPrefix(type, "union ");
   type = stripPrefix(type, "enum ");
   type = stripPrefix(type, "typename ");

   type = removeRedundantWhiteSpace(type);

  
   QByteArray canType;
   QByteArray templSpec, word;
   int i, p = 0, pp = 0;

   while ((i = extractClassNameFromType(type, p, word, templSpec)) != -1) {
      // foreach identifier in the type
        
      if (i > pp) {
         canType += type.mid(pp, i - pp);
      }

      QByteArray ct = getCanonicalTypeForIdentifier(d, fs, word, &templSpec);

      // in case the ct is empty it means that "word" represents scope "d"
      // and this does not need to be added to the canonical
      // type (it is redundant), so/ we skip it. This solves problem 589616.

      if (ct.isEmpty() && type.mid(p, 2) == "::") {
         p += 2;
      } else {
         canType += ct;
      }
      
      if (!templSpec.isEmpty())  {
         // if we didn't use up the templSpec already
         // (i.e. type is not a template specialization)
         // then resolve any identifiers inside.
      
         static QRegExp re("[a-z_A-Z\\x80-\\xFF][a-z_A-Z0-9\\x80-\\xFF]*");
         int tp = 0, tl, ti;

         // for each identifier template specifier
         while ((ti = re.indexIn(templSpec, tp)) != -1) {
            tl = re.matchedLength();

            canType += templSpec.mid(tp, ti - tp);
            canType += getCanonicalTypeForIdentifier(d, fs, templSpec.mid(ti, tl), 0);
            tp = ti + tl; 
         }

         canType += templSpec.right(templSpec.length() - tp);
      }

      pp = p;
   }

   canType += type.right(type.length() - pp);

   return removeRedundantWhiteSpace(canType);
}

static QByteArray extractCanonicalArgType(Definition *d, FileDef *fs, const Argument *arg)
{
   QByteArray type = arg->type.trimmed();
   QByteArray name = arg->name;
 
   if ((type == "const" || type == "volatile") && !name.isEmpty()) {
      // name is part of type => correct
      type += " ";
      type += name;
   }

   if (name == "const" || name == "volatile") {
      // name is part of type => correct
      if (!type.isEmpty()) {
         type += " ";
      }
      type += name;
   }

   if (!arg->array.isEmpty()) {
      type += arg->array;
   }

   return extractCanonicalType(d, fs, type);
}

static bool matchArgument2(Definition *srcScope, FileDef *srcFileScope, Argument *srcA, 
                           Definition *dstScope, FileDef *dstFileScope, Argument *dstA)
{
   QByteArray sSrcName = " " + srcA->name;
   QByteArray sDstName = " " + dstA->name;
   QByteArray srcType  = srcA->type;
   QByteArray dstType  = dstA->type;

   stripIrrelevantConstVolatile(srcType);
   stripIrrelevantConstVolatile(dstType);

 
   if (sSrcName == dstType.right(sSrcName.length())) {
      // case "unsigned int" <-> "unsigned int i"
      srcA->type += sSrcName;
      srcA->name = "";
      srcA->canType = ""; // invalidate cached type value

   } else if (sDstName == srcType.right(sDstName.length())) {
      // case "unsigned int i" <-> "unsigned int"
      dstA->type += sDstName;
      dstA->name = "";
      dstA->canType = ""; // invalidate cached type value
   }

   if (srcA->canType.isEmpty()) {
      srcA->canType = extractCanonicalArgType(srcScope, srcFileScope, srcA);
   }

   if (dstA->canType.isEmpty()) {
      dstA->canType = extractCanonicalArgType(dstScope, dstFileScope, dstA);
   }

   if (srcA->canType == dstA->canType) {
      DOX_MATCH
      return true;

   } else {
      //printf("   Canonical types do not match [%s]<->[%s]\n",
      //    srcA->canType.data(),dstA->canType.data());
      DOX_NOMATCH
      return false;
   }
}


// algorithm for argument matching
bool matchArguments2(QSharedPointer<Definition> srcScope, FileDef *srcFileScope, ArgumentList *srcAl,
                     QSharedPointer<Definition> dstScope, FileDef *dstFileScope, ArgumentList *dstAl, bool checkCV )
{   
   assert(srcScope != 0 && dstScope != 0);

   if (srcAl == 0 || dstAl == 0) {
      bool match = srcAl == dstAl; // at least one of the members is not a function

      if (match) {
         DOX_MATCH
         return true;
      } else {
         DOX_NOMATCH
         return false;
      }
   }

   // handle special case with void argument
   if (srcAl->count() == 0 && dstAl->count() == 1 && dstAl->first().type == "void" ) {
      // special case for finding match between func() and func(void)
      Argument a;
      a.type = "void";

      srcAl->append(a);

      DOX_MATCH
      return true;
   }

   if (dstAl->count() == 0 && srcAl->count() == 1 && srcAl->first().type == "void" ) {
      // special case for finding match between func(void) and func()
      Argument a;
      a.type = "void";

      dstAl->append(a);

      DOX_MATCH
      return true;
   }

   if (srcAl->count() != dstAl->count()) {
      DOX_NOMATCH
      return false; // different number of arguments -> no match
   }

   if (checkCV) {
      if (srcAl->constSpecifier != dstAl->constSpecifier) {
         DOX_NOMATCH
         return false; // one member is const, the other not -> no match
      }

      if (srcAl->volatileSpecifier != dstAl->volatileSpecifier) {
         DOX_NOMATCH
         return false; // one member is volatile, the other not -> no match
      }
   }

   // so far the argument list could match, so we need to compare the types of all arguments
   auto item = dstAl->begin();

   for (auto &srcA : *srcAl ) {

      if (! matchArgument2(srcScope, srcFileScope, &srcA,  dstScope, dstFileScope, &(*item))) {
         DOX_NOMATCH
         return false;
      }

      ++item;
   }


   DOX_MATCH
   return true; // all arguments match
}

// merges the initializer of two argument lists
// pre:  the types of the arguments in the list should match.
void mergeArguments(ArgumentList *srcAl, ArgumentList *dstAl, bool forceNameOverwrite)
{
   if (srcAl == 0 || dstAl == 0 || srcAl->count() != dstAl->count()) {
      return; // invalid argument lists -> do not merge
   }

   auto dstA = dstAl->begin();

   for (auto &srcA : *srcAl) {

      if (srcA.defval.isEmpty() && ! dstA->defval.isEmpty()) {
         srcA.defval = dstA->defval;

      } else if (! srcA.defval.isEmpty() && dstA->defval.isEmpty()) {
         dstA->defval = srcA.defval;
      }

      // fix wrongly detected const or volatile specifiers before merging.
      // example: "const A *const" is detected as type="const A *" name="const"
      if (srcA.name == "const" || srcA.name == "volatile") {
         srcA.type += " " + srcA.name;
         srcA.name.resize(0);
      }

      if (dstA->name == "const" || dstA->name == "volatile") {
         dstA->type += " " + dstA->name;
         dstA->name.resize(0);
      }

      if (srcA.type == dstA->type) {
         if (srcA.name.isEmpty() && ! dstA->name.isEmpty()) {           
            srcA.type = dstA->type;
            srcA.name = dstA->name;

         } else if (! srcA.name.isEmpty() && dstA->name.isEmpty()) {          
            dstA->type = srcA.type;
            dstA->name = srcA.name;      // copperspice (code fixed)

         } else if (! srcA.name.isEmpty() && ! dstA->name.isEmpty()) {
            if (forceNameOverwrite) {
               srcA.name = dstA->name;

            } else {
               if (srcA.docs.isEmpty() && ! dstA->docs.isEmpty()) {
                  srcA.name = dstA->name;

               } else if (!srcA.docs.isEmpty() && dstA->docs.isEmpty()) {
                  dstA->name = srcA.name;
               }
            }
         }

      } else {
         //printf("2. merging '%s':'%s' <-> '%s':'%s'\n",srcA->type.data(),srcA->name.data(),dstA->type.data(),dstA->name.data());
         srcA.type = srcA.type.trimmed();
         dstA->type = dstA->type.trimmed();

         if (srcA.type + " " + srcA.name == dstA->type) { // "unsigned long:int" <-> "unsigned long int:bla"
            srcA.type += " " + srcA.name;
            srcA.name = dstA->name;

         } else if (dstA->type + " " + dstA->name == srcA.type) { // "unsigned long int bla" <-> "unsigned long int"
            dstA->type += " " + dstA->name;
            dstA->name = srcA.name;

         } else if (srcA.name.isEmpty() && ! dstA->name.isEmpty()) {
            srcA.name = dstA->name;

         } else if (dstA->name.isEmpty() && !srcA.name.isEmpty()) {
            dstA->name = srcA.name;
         }
      }

      int i1 = srcA.type.indexOf("::"), 
          i2 = dstA->type.indexOf("::"),
          j1 = srcA.type.length() - i1 - 2,
          j2 = dstA->type.length() - i2 - 2;

      if (i1 != -1 && i2 == -1 && srcA.type.right(j1) == dstA->type) {       
         dstA->type = srcA.type.left(i1 + 2) + dstA->type;
         dstA->name = srcA.name;    // copperspice (code fixed)

      } else if (i1 == -1 && i2 != -1 && dstA->type.right(j2) == srcA.type) {     
         srcA.type = dstA->type.left(i2 + 2) + srcA.type;
         srcA.name = dstA->name;
      }

      if (srcA.docs.isEmpty() && ! dstA->docs.isEmpty()) {
         srcA.docs = dstA->docs;

      } else if (dstA->docs.isEmpty() && !srcA.docs.isEmpty()) {
         dstA->docs = srcA.docs;
      }

      ++dstA;
   }
}

static void findMembersWithSpecificName(MemberName *mn, const char *args, bool checkStatics, FileDef *currentFile,
                                        bool checkCV, const char *forceTagFile, QList<MemberDef *> &members)
{
   for (auto md : *mn) {
      FileDef  *fd = md->getFileDef();
      GroupDef *gd = md->getGroupDef();
    
      if (((gd && gd->isLinkable()) || (fd && fd->isLinkable()) || md->isReference()) && 
         md->getNamespaceDef() == 0 && md->isLinkable() && 
         (!checkStatics || (!md->isStatic() && !md->isDefine()) || currentFile == 0 || fd == currentFile) ) {

         // statics must appear in the same file      
         bool match = true;
         ArgumentList *argList = 0;

         if (args && !md->isDefine() && qstrcmp(args, "()") != 0) {
            argList = new ArgumentList;
            ArgumentList *mdAl = md->argumentList();

            stringToArgumentList(args, argList);

            match = matchArguments2(md->getOuterScope(), fd, mdAl, 
                       Doxygen::globalScope, fd, argList,checkCV);

            delete argList;
            argList = 0;
         }

         if (match && (forceTagFile == 0 || md->getReference() == forceTagFile)) {           
            members.append(md.data());
         }
      }
   }
}

/*!
 * Searches for a member definition given its name `memberName' as a string.
 * memberName may also include a (partial) scope to indicate the scope
 * in which the member is located.
 *
 * The parameter `scName' is a string representing the name of the scope in
 * which the link was found.
 *
 * In case of a function args contains a string representation of the
 * argument list. Passing 0 means the member has no arguments.
 * Passing "()" means any argument list will do, but "()" is preferred.
 *
 * The function returns true if the member is known and documented or
 * false if it is not.
 * If true is returned parameter `md' contains a pointer to the member
 * definition. Furthermore exactly one of the parameter `cd', `nd', or `fd'
 * will be non-zero:
 *   - if `cd' is non zero, the member was found in a class pointed to by cd.
 *   - if `nd' is non zero, the member was found in a namespace pointed to by nd.
 *   - if `fd' is non zero, the member was found in the global namespace of
 *     file fd.
 */
bool getDefs(const QByteArray &scName, const QByteArray &mbName, const char *args, QSharedPointer<MemberDef> &md,
             QSharedPointer<ClassDef> &cd, QSharedPointer<FileDef> &fd, QSharedPointer<NamespaceDef> &nd, 
             QSharedPointer<GroupDef> *&gd, bool forceEmptyScope,  QSharedPointer<FileDef>currentFile, 
             bool checkCV, const char *forceTagFile )
{
   fd = QSharedPointer<FileDef>();
   md = QSharedPointer<MemberDef>();
   cd = QSharedPointer<ClassDef>();
   nd = QSharedPointer<NamespaceDef>();
   gd = QSharedPointer<GroupDef>();

   if (mbName.isEmpty()) {
      return false;   /* empty name => nothing to link */
   }

   QByteArray scopeName = scName;
   QByteArray memberName = mbName;

   scopeName = substitute(scopeName, "\\", "::"); // for PHP
   memberName = substitute(memberName, "\\", "::"); // for PHP

   int is, im = 0, pm = 0;

   // strip common part of the scope from the scopeName
   while ((is = scopeName.lastIndexOf("::")) != -1 && (im = memberName.indexOf("::", pm)) != -1 &&
          (scopeName.right(scopeName.length() - is - 2) == memberName.mid(pm, im - pm))) {

      scopeName = scopeName.left(is);

      pm = im + 2;
   }
 
   QByteArray mName = memberName;
   QByteArray mScope;

   if (memberName.left(9) != "operator " && (im = memberName.lastIndexOf("::")) != -1 &&
         im < (int)memberName.length() - 2) {

         // treat operator conversion methods
         // as a special case
         // not A::

      mScope = memberName.left(im);
      mName  = memberName.right(memberName.length() - im - 2);
   }

   // handle special the case where both scope name and member scope are equal
   if (mScope == scopeName) {
      scopeName.resize(0);
   }

   QSharedPointer<MemberName> mn = Doxygen::memberNameSDict->find(mName);
 
   if ((!forceEmptyScope || scopeName.isEmpty()) && mn && !(scopeName.isEmpty() && mScope.isEmpty())) {
      // this was changed for bug638856, forceEmptyScope => empty scopeName
     
      int scopeOffset = scopeName.length();

      do {
         QByteArray className = scopeName.left(scopeOffset);

         if (!className.isEmpty() && !mScope.isEmpty()) {
            className += "::" + mScope;
         } else if (!mScope.isEmpty()) {
            className = mScope;
         }

         MemberDef *tmd = 0;
         QSharedPointer<ClassDef> fcd = getResolvedClass(Doxygen::globalScope, 0, className, &tmd);
        
         // todo: fill in correct fileScope
         if (fcd &&  fcd->isLinkable() ) {
            // is it a documented class
           
            int mdist = maxInheritanceDepth;
            ArgumentList *argList = 0;

            if (args) {
               argList = new ArgumentList;
               stringToArgumentList(args, argList);
            }

            for (auto mmd : *mn) { 
               if (!mmd->isStrongEnumValue()) {
                  ArgumentList *mmdAl = mmd->argumentList();

                  bool match = args == 0 || matchArguments2(mmd->getOuterScope(), mmd->getFileDef(), 
                                    mmdAl, fcd, fcd->getFileDef(), argList, checkCV );
                  
                  if (match) {
                     QSharedPointer<ClassDef> mcd = mmd->getClassDef();

                     if (mcd) {
                        int m = minClassDistance(fcd, mcd);

                        if (m < mdist && mcd->isLinkable()) {
                           mdist = m;
                           cd = mcd;
                           md = mmd;
                        }
                     }
                  }
               }
            }

            if (argList) {
               delete argList;
               argList = 0;
            }

            if (mdist == maxInheritanceDepth && args && qstrcmp(args, "()") == 0) {
               // no exact match found, but if args="()" an arbitrary member will do            
                  
               for (auto mmd : *mn) {                  
                  QSharedPointer<ClassDef> mcd = mmd->getClassDef();
                  
                  if (mcd) {
                     int m = minClassDistance(fcd, mcd);

                     if (m < mdist) {
                        
                        mdist = m;
                        cd = mcd;
                        md = mmd;
                     }
                  }
                  
               }
            }
            
            if (mdist < maxInheritanceDepth) {
               if (!md->isLinkable() || md->isStrongEnumValue()) {
                  md = 0; // avoid returning things we cannot link to
                  cd = 0;
                  return false; // match found, but was not linkable

               } else {
                  gd = md->getGroupDef();
                  if (gd) {
                     cd = 0;
                  }
                  return true; /* found match */
               }
            }
         }

         if (tmd && tmd->isEnumerate() && tmd->isStrong()) {       
            // scoped enum
            
            MemberList *tml = tmd->enumFieldList();

            if (tml) {              
               for (auto emd : *tml) { 
                  if (emd->localName() == mName) {
                     if (emd->isLinkable()) {
                        cd = tmd->getClassDef();
                        md = emd;
                        return true;

                     } else {
                        cd = 0;
                        md = 0;
                        return false;
                     }
                  }
               }
            }
         }

         /* go to the parent scope */
         if (scopeOffset == 0) {
            scopeOffset = -1;

         } else if ((scopeOffset = scopeName.lastIndexOf("::", scopeOffset - 1)) == -1) {
            scopeOffset = 0;
         }

      } while (scopeOffset >= 0);

   }

   if (mn && scopeName.isEmpty() && mScope.isEmpty()) { 
      // Maybe a related function?     
   
      MemberDef *fuzzy_mmd = 0;
      ArgumentList *argList = 0;

      bool hasEmptyArgs = args && qstrcmp(args, "()") == 0;

      if (args) {
         stringToArgumentList(args, argList = new ArgumentList);
      }    

      MemberDef *temp = nullptr; 

      for (auto item : *mn) { 
         temp = item.data();

         if (! item->isLinkable() || (! item->isRelated() && ! item->isForeign()) || ! item->getClassDef()) {
            continue;
         }

         if (! args) {
            break;
         }

         QByteArray className = item->getClassDef()->name();

         ArgumentList *mmdAl = item->argumentList();

         if (matchArguments2(item->getOuterScope(), item->getFileDef(), mmdAl,
                             Doxygen::globalScope, item->getFileDef(), argList, checkCV)) {
            break;
         }

         if (! fuzzy_mmd && hasEmptyArgs) {
            fuzzy_mmd = item.data();
         }
      }

      if (argList) {
         delete argList, argList = 0;
      }
  
      if (! temp) {     
         temp = fuzzy_mmd;
      }

      if (temp && ! temp->isStrongEnumValue()) {
         md = temp;
         cd = temp->getClassDef();

         return true;
      }
   }

   // maybe an namespace, file or group member ?   
   if ((mn = Doxygen::functionNameSDict->find(mName))) { 
      // name is known      
      QSharedPointer<NamespaceDef> fnd;
      int scopeOffset = scopeName.length();

      do {
         QByteArray namespaceName = scopeName.left(scopeOffset);

         if (! namespaceName.isEmpty() && !mScope.isEmpty()) {
            namespaceName += "::" + mScope;

         } else if (! mScope.isEmpty()) {
            namespaceName = mScope;
         }
         
         if (! namespaceName.isEmpty() && (fnd = Doxygen::namespaceSDict->find(namespaceName)) && fnd->isLinkable()) {
            
            bool found = false;
           
            for (auto mmd : *mn) {

               if ( found) {
                  break;
               }
               
               MemberDef *emd = mmd->getEnumScope();

               if (emd && emd->isStrong()) {
                 
                  if (emd->getNamespaceDef() == fnd && rightScopeMatch(mScope, emd->localName())) {                    
                     nd = fnd;
                     md = mmd;

                     found = true;

                  } else {
                     md = 0;
                     cd = 0;

                     return false;
                  }

               } else if (mmd->getNamespaceDef() == fnd) {
                  // namespace is found
                  bool match = true;
                  ArgumentList *argList = 0;

                  if (args && qstrcmp(args, "()") != 0) {
                     argList = new ArgumentList;
                     ArgumentList *mmdAl = mmd->argumentList();

                     stringToArgumentList(args, argList);
                     match = matchArguments2(mmd->getOuterScope(), mmd->getFileDef(), mmdAl,
                                fnd, mmd->getFileDef(), argList, checkCV); 
                  }

                  if (match) {
                     nd = fnd;
                     md = mmd;
                     found = true;
                  }

                  if (args) {
                     delete argList;
                     argList = 0;
                  }
               }
            }

            if (! found && args && !qstrcmp(args, "()")) {
               // no exact match found, but if args="()" an arbitrary member will do
                         
               for (auto mmd : *mn) {

                  if (found) {
                     break;
                  }

                  if (mmd->getNamespaceDef() == fnd) {
                     nd = fnd;
                     md = mmd;
                     found = true;
                  }
               }
            }

            if (found) {
               if (!md->isLinkable()) {
                  md = 0;          // avoid returning things we cannot link to
                  nd = 0;
                  return false;    // match found but not linkable

               } else {
                  gd = md->getGroupDef();

                  if (gd && gd->isLinkable()) {
                     nd = 0;
                  } else {
                     gd = 0;
                  }
                  return true;
               }
            }

         } else {
            //printf("not a namespace\n");
            bool found = false;
          
            for (auto mmd : *mn) {

               if ( found) {
                  break;
               }

               MemberDef *tmd = mmd->getEnumScope();

               int ni = namespaceName.lastIndexOf("::");
               bool notInNS = tmd && ni == -1 && tmd->getNamespaceDef() == 0 && (mScope.isEmpty() || mScope == tmd->name());
               bool sameNS  = tmd && tmd->getNamespaceDef() && namespaceName.left(ni) == tmd->getNamespaceDef()->name();

               if (tmd && tmd->isStrong() && (notInNS || sameNS) && namespaceName.length() > 0) { 
                  // enum is part of namespace so this should not be empty                 
                  md = mmd;
                  fd = mmd->getFileDef();
                  gd = mmd->getGroupDef();

                  if (gd && gd->isLinkable()) {
                     fd = 0;

                  } else {
                     gd = 0;
                  }
                  
                  return true;
               }
            }
         }

         if (scopeOffset == 0) {
            scopeOffset = -1;
         } else if ((scopeOffset = scopeName.lastIndexOf("::", scopeOffset - 1)) == -1) {
            scopeOffset = 0;
         }

      } while (scopeOffset >= 0);

      // no scope => global function
      {
         QList<MemberDef *> members;

         // search for matches with strict static checking
         findMembersWithSpecificName(mn.data(), args, true, currentFile, checkCV, forceTagFile, members);

         if (members.count() == 0) { // nothing found
            // search again without strict static checking
            findMembersWithSpecificName(mn.data(), args, false, currentFile, checkCV, forceTagFile, members);
         }
 
         if (members.count() != 1 && args && !qstrcmp(args, "()")) {
            // no exact match found, but if args="()" an arbitrary member will do          

            for (auto md : *mn) {
               fd = md->getFileDef();
               gd = md->getGroupDef();

               MemberDef *tmd = md->getEnumScope();

               if ((gd && gd->isLinkable()) || (fd && fd->isLinkable()) || (tmd && tmd->isStrong())) {
                  members.append(md.data());
               }
            }
         }

         if (members.count() > 0) { // at least one match
            if (currentFile) {
             
               for (auto md : members) {              
                  if (md->getFileDef() && md->getFileDef()->name() == currentFile->name()) {
                     break; // found match in the current file
                  }
               }

               if (!md) { // member not in the current file
                  md = members.last();
               }

            } else {
               md = members.last();
            }
         }
 
         if (md && (md->getEnumScope() == 0 || ! md->getEnumScope()->isStrong())) {
            // found a matching global member, that is not a scoped enum value (or uniquely matches)
         
            fd = md->getFileDef();
            gd = md->getGroupDef();

            if (gd && gd->isLinkable()) {
               fd = 0;
            } else {
               gd = 0;
            }

            return true;
         }
      }
   }

   // nothing found
   return false;
}

/*!
 * Searches for a scope definition given its name as a string via parameter
 * `scope`.
 *
 * The parameter `docScope` is a string representing the name of the scope in
 * which the `scope` string was found.
 *
 * The function returns true if the scope is known and documented or
 * false if it is not.
 * If true is returned exactly one of the parameter `cd`, `nd`
 * will be non-zero:
 *   - if `cd` is non zero, the scope was a class pointed to by cd.
 *   - if `nd` is non zero, the scope was a namespace pointed to by nd.
 */
static bool getScopeDefs(const char *docScope, const char *scope, ClassDef *&cd, NamespaceDef *&nd)
{
   cd = 0;
   nd = 0;

   QByteArray scopeName = scope;

   if (scopeName.isEmpty()) {
      return false;
   }

   bool explicitGlobalScope = false;

   if (scopeName.length() > 1 && scopeName.at(0) == ':' && scopeName.at(1) == ':') {
      scopeName = scopeName.right(scopeName.length() - 2);
      explicitGlobalScope = true;
   }

   QByteArray docScopeName = docScope;
   int scopeOffset = explicitGlobalScope ? 0 : docScopeName.length();

   do { // for each possible docScope (from largest to and including empty)
      QByteArray fullName = scopeName;

      if (scopeOffset > 0) {
         fullName.prepend(docScopeName.left(scopeOffset) + "::");
      }

      if (((cd = getClass(fullName)) || (cd = getClass(fullName + "-p")) ) && cd->isLinkable()) {
         return true; 

      } else if ((nd = Doxygen::namespaceSDict->find(fullName).data()) && nd->isLinkable()) {
         return true; 
      }

      if (scopeOffset == 0) {
         scopeOffset = -1;

      } else if ((scopeOffset = docScopeName.lastIndexOf("::", scopeOffset - 1)) == -1) {
         scopeOffset = 0;
      }

   } while (scopeOffset >= 0);

   return false;
}

static bool isLowerCase(QByteArray &s)
{
   uchar *p = (uchar *)s.data();
   if (p == 0) {
      return true;
   }
   int c;
   while ((c = *p++)) if (!islower(c)) {
         return false;
      }
   return true;
}

/*! Returns an object to reference to given its name and context
 *  @post return value true implies *resContext!=0 or *resMember!=0
 */
bool resolveRef(/* in */  const char *scName,
                          /* in */  const char *name,
                          /* in */  bool inSeeBlock,
                          /* out */ Definition **resContext,
                          /* out */ MemberDef  **resMember,
                          bool lookForSpecialization,
                          FileDef *currentFile,
                          bool checkScope
               )
{
   //printf("resolveRef(scope=%s,name=%s,inSeeBlock=%d)\n",scName,name,inSeeBlock);
   QByteArray tsName = name;

   //bool memberScopeFirst = tsName.find('#')!=-1;

   QByteArray fullName = substitute(tsName, "#", "::");
   if (fullName.indexOf("anonymous_namespace{") == -1) {
      fullName = removeRedundantWhiteSpace(substitute(fullName, ".", "::"));
   } else {
      fullName = removeRedundantWhiteSpace(fullName);
   }

   int bracePos = findParameterList(fullName);
   int endNamePos = bracePos != -1 ? bracePos : fullName.length();
   int scopePos = fullName.lastIndexOf("::", endNamePos);
   bool explicitScope = fullName.left(2) == "::" && // ::scope or #scope
                        (scopePos > 2 ||            // ::N::A
                         tsName.left(2) == "::" ||  // ::foo in local scope
                         scName == 0                // #foo  in global scope
                        );

   // default result values
   *resContext = 0;
   *resMember = 0;

   if (bracePos == -1) { // simple name
      ClassDef *cd = 0;
      NamespaceDef *nd = 0;

      // the following if() was commented out for releases in the range
      // 1.5.2 to 1.6.1, but has been restored as a result of bug report 594787.
      if (!inSeeBlock && scopePos == -1 && isLowerCase(tsName)) {
         // link to lower case only name => do not try to autolink
         return false;
      }

      //printf("scName=%s fullName=%s\n",scName,fullName.data());

      // check if this is a class or namespace reference
      if (scName != fullName && getScopeDefs(scName, fullName, cd, nd)) {
         if (cd) { // scope matches that of a class
            *resContext = cd;
         } else { // scope matches that of a namespace
            assert(nd != 0);
            *resContext = nd;
         }
         return true;
      } else if (scName == fullName || (!inSeeBlock && scopePos == -1))
         // nothing to link => output plain text
      {
         //printf("found scName=%s fullName=%s scName==fullName=%d "
         //    "inSeeBlock=%d scopePos=%d!\n",
         //    scName,fullName.data(),scName==fullName,inSeeBlock,scopePos);
         return false;
      }
      // continue search...
   }

   // extract userscope+name
   QByteArray nameStr = fullName.left(endNamePos);
   if (explicitScope) {
      nameStr = nameStr.mid(2);
   }

   // extract arguments
   QByteArray argsStr;
   if (bracePos != -1) {
      argsStr = fullName.right(fullName.length() - bracePos);
   }

   // strip template specifier
   // TODO: match against the correct partial template instantiation
   int templPos = nameStr.indexOf('<');
   bool tryUnspecializedVersion = false;

   if (templPos != -1 && nameStr.indexOf("operator") == -1) {
      int endTemplPos = nameStr.lastIndexOf('>');

      if (endTemplPos != -1) {
         if (!lookForSpecialization) {
            nameStr = nameStr.left(templPos) + nameStr.right(nameStr.length() - endTemplPos - 1);
         } else {
            tryUnspecializedVersion = true;
         }
      }
   }

   QByteArray scopeStr = scName;

   MemberDef    *md = 0;
   ClassDef     *cd = 0;
   FileDef      *fd = 0;
   NamespaceDef *nd = 0;
   GroupDef     *gd = 0;

   // check if nameStr is a member or global   
   if (getDefs(scopeStr, nameStr, argsStr,
               md, cd, fd, nd, gd,
                                    //scopePos==0 && !memberScopeFirst, // forceEmptyScope
               explicitScope,       // replaces prev line due to bug 600829
               currentFile,
               true                 // checkCV
              )
      ) 
   {
      //printf("after getDefs checkScope=%d nameStr=%s cd=%p nd=%p\n",checkScope,nameStr.data(),cd,nd);
      if (checkScope && md && md->getOuterScope() == Doxygen::globalScope &&
            !md->isStrongEnumValue() && (!scopeStr.isEmpty() || nameStr.indexOf("::") > 0)) {

         // we did find a member, but it is a global one while we were explicitly
         // looking for a scoped variable. See bug 616387 for an example why this check is needed.
         // note we do need to support autolinking to "::symbol" hence the >0
         //printf("not global member!\n");

         *resContext = 0;
         *resMember = 0;
         return false;
      }

      //printf("after getDefs md=%p cd=%p fd=%p nd=%p gd=%p\n",md,cd,fd,nd,gd);

      if (md) {
         *resMember = md;
         *resContext = md;
      } else if (cd) {
         *resContext = cd;
      } else if (nd) {
         *resContext = nd;
      } else if (fd) {
         *resContext = fd;
      } else if (gd) {
         *resContext = gd;
      } else         {
         *resContext = 0;
         *resMember = 0;
         return false;
      }
      //printf("member=%s (md=%p) anchor=%s linkable()=%d context=%s\n",
      //    md->name().data(),md,md->anchor().data(),md->isLinkable(),(*resContext)->name().data());
      return true;

   } else if (inSeeBlock && !nameStr.isEmpty() && (gd = Doxygen::groupSDict->find(nameStr).data())) {
      // group link
      *resContext = gd;
      return true;

   } else if (tsName.indexOf('.') != -1) { 
      // maybe a link to a file

      bool ambig;

      fd = findFileDef(Doxygen::inputNameDict, tsName, ambig);

      if (fd && !ambig) {
         *resContext = fd;
         return true;
      }
   }

   if (tryUnspecializedVersion) {
      return resolveRef(scName, name, inSeeBlock, resContext, resMember, false, 0, checkScope);
   }
   if (bracePos != -1) { // Try without parameters as well, could be a contructor invocation
      *resContext = getClass(fullName.left(bracePos));
      if (*resContext) {
         return true;
      }
   }
   
   return false;
}

QByteArray linkToText(SrcLangExt lang, const char *link, bool isFileName)
{
   //static bool optimizeOutputJava = Config_getBool("OPTIMIZE_OUTPUT_JAVA");
   QByteArray result = link;

   if (! result.isEmpty()) {
      // replace # by ::

      result = substitute(result, "#", "::");
      // replace . by ::
      if (! isFileName && result.indexOf('<') == -1) {
         result = substitute(result, ".", "::");
      }

      // strip leading :: prefix if present
      if (result.length() > 1 && result.at(0) == ':' && result.at(1) == ':') {
         result = result.right(result.length() - 2);
      }

      QByteArray sep = getLanguageSpecificSeparator(lang);

      if (sep != "::") {
         result = substitute(result, "::", sep);
      }
   }
   return result;
}

#if 0
/*
 * generate a reference to a class, namespace or member.
 * `scName' is the name of the scope that contains the documentation
 * string that is returned.
 * `name' is the name that we want to link to.
 * `name' may have five formats:
 *    1) "ScopeName"
 *    2) "memberName()"    one of the (overloaded) function or define
 *                         with name memberName.
 *    3) "memberName(...)" a specific (overloaded) function or define
 *                         with name memberName
 *    4) "::name           a global variable or define
 *    4) "\#memberName     member variable, global variable or define
 *    5) ("ScopeName::")+"memberName()"
 *    6) ("ScopeName::")+"memberName(...)"
 *    7) ("ScopeName::")+"memberName"
 * instead of :: the \# symbol may also be used.
 */

bool generateRef(OutputDocInterface &od, const char *scName,
                 const char *name, bool inSeeBlock, const char *rt)
{
   //printf("generateRef(scName=%s,name=%s,inSee=%d,rt=%s)\n",scName,name,inSeeBlock,rt);

   Definition *compound;
   MemberDef *md = nullptr;

   // create default link text
   QByteArray linkText = linkToText(rt, false);

   if (resolveRef(scName, name, inSeeBlock, &compound, &md)) {
      if (md && md->isLinkable()) { // link to member
         od.writeObjectLink(md->getReference(),
                            md->getOutputFileBase(),
                            md->anchor(), linkText);
         // generate the page reference (for LaTeX)
         if (!md->isReference()) {
            writePageRef(od, md->getOutputFileBase(), md->anchor());
         }
         return true;
      } else if (compound && compound->isLinkable()) { // link to compound
         if (rt == 0 && compound->definitionType() == Definition::TypeGroup) {
            linkText = ((GroupDef *)compound)->groupTitle();
         }
         if (compound && compound->definitionType() == Definition::TypeFile) {
            linkText = linkToText(rt, true);
         }
         od.writeObjectLink(compound->getReference(),
                            compound->getOutputFileBase(),
                            0, linkText);
         if (!compound->isReference()) {
            writePageRef(od, compound->getOutputFileBase(), 0);
         }
         return true;
      }
   }
   od.docify(linkText);
   return false;
}
#endif

bool resolveLink(const char *scName, const char *lr, bool xx, Definition **resContext, QByteArray &resAnchor)
{
   *resContext = 0;

   QByteArray linkRef = lr;
   
   FileDef  *fd;
   GroupDef *gd;
   PageDef  *pd;
   ClassDef *cd;
   DirDef   *dir;
   NamespaceDef *nd;
   SectionInfo *si = 0;

   bool ambig;

   if (linkRef.isEmpty()) { // no reference name!
      return false;

   } else if ((pd = Doxygen::pageSDict->find(linkRef).data())) { 
      // link to a page
      GroupDef *gd = pd->getGroupDef();

      if (gd) {
         if (!pd->name().isEmpty()) {
            si = Doxygen::sectionDict->find(pd->name()).data();
         }

         *resContext = gd;
         if (si) {
            resAnchor = si->label;
         }

      } else {
         *resContext = pd;
      }

      return true;

   } else if ((si = Doxygen::sectionDict->find(linkRef).data())) {
      *resContext = si->definition;
      resAnchor = si->label;
      return true;

   } else if ((pd = Doxygen::exampleSDict->find(linkRef).data())) { // link to an example
      *resContext = pd;
      return true;

   } else if ((gd = Doxygen::groupSDict->find(linkRef).data())) { // link to a group
      *resContext = gd;
      return true;

   } else if ((fd = findFileDef(Doxygen::inputNameDict, linkRef, ambig)) // file link 
              && fd->isLinkable()) {
      *resContext = fd;
      return true;

   } else if ((cd = getClass(linkRef))) { // class link
      *resContext = cd;
      resAnchor = cd->anchor();
      return true;

   } else if ((cd = getClass(linkRef + "-p"))) { // Obj-C protocol link
      *resContext = cd;
      resAnchor = cd->anchor();
      return true;

   }
  
   else if ((nd = Doxygen::namespaceSDict->find(linkRef).data())) {
      *resContext = nd;
      return true;

   } else if ((dir = Doxygen::directories.find(QFileInfo(linkRef).absoluteFilePath().toUtf8() + "/").data()) && dir->isLinkable()) { 
      // TODO: make this location independent like filedefs

      *resContext = dir;
      return true;

   } else { 
      // probably a member reference
      MemberDef *md = nullptr;
      bool res = resolveRef(scName, lr, true, resContext, &md);

      if (md) {
         resAnchor = md->anchor();
      }

      return res;
   }
}

// General function that generates the HTML code for a reference to some
// file, class or member from text `lr' within the context of class `clName'.
// This link has the text 'lt' (if not 0), otherwise `lr' is used as a
// basis for the link's text returns true if a link could be generated.

bool generateLink(OutputDocInterface &od, const char *clName, const char *lr, bool inSeeBlock, const char *lt)
{
   
   Definition *compound;   
   QByteArray anchor, linkText = linkToText(SrcLangExt_Unknown, lt, false);
  
   if (resolveLink(clName, lr, inSeeBlock, &compound, anchor)) {

      if (compound) { // link to compound
         if (lt == 0 && anchor.isEmpty() &&                    /* compound link */
               compound->definitionType() == Definition::TypeGroup /* is group */
            ) {
            linkText = ((GroupDef *)compound)->groupTitle(); // use group's title as link

         } else if (compound->definitionType() == Definition::TypeFile) {
            linkText = linkToText(compound->getLanguage(), lt, true);
         }

         od.writeObjectLink(compound->getReference(), compound->getOutputFileBase(), anchor, linkText);

         if (! compound->isReference()) {
            writePageRef(od, compound->getOutputFileBase(), anchor);
         }

      } else {
         err("%s:%d: Internal error: resolveLink successful but no compound found!", __FILE__, __LINE__);
      }

      return true;

   } else { 
      // link could not be found
      od.docify(linkText);
      return false;
   }
}

void generateFileRef(OutputDocInterface &od, const char *name, const char *text)
{
   //printf("generateFileRef(%s,%s)\n",name,text);
   QByteArray linkText = text ? text : name;
   //FileInfo *fi;
   FileDef *fd;
   bool ambig;
   if ((fd = findFileDef(Doxygen::inputNameDict, name, ambig)) &&
         fd->isLinkable())
      // link to documented input file
   {
      od.writeObjectLink(fd->getReference(), fd->getOutputFileBase(), 0, linkText);
   } else {
      od.docify(linkText);
   }
}

//----------------------------------------------------------------------

#if 0
QByteArray substituteClassNames(const QByteArray &s)
{
   int i = 0, l, p;
   QByteArray result;
   if (s.isEmpty()) {
      return result;
   }
   QRegExp r("[a-z_A-Z][a-z_A-Z0-9]*");
   while ((p = r.match(s, i, &l)) != -1) {
      QByteArray *subst;
      if (p > i) {
         result += s.mid(i, p - i);
      }
      if ((subst = substituteDict[s.mid(p, l)])) {
         result += *subst;
      } else {
         result += s.mid(p, l);
      }
      i = p + l;
   }
   result += s.mid(i, s.length() - i);
   return result;
}
#endif

/** Cache element for the file name to FileDef mapping cache. */
QSharedPointer<FileDef> findFileDef(const FileNameDict *fnDict, const char *n, bool &ambig)
{
   ambig = false;

   if (n == 0) {
      return QSharedPointer<FileDef>();
   }

   const int maxAddrSize = 20;
   char addr[maxAddrSize];

   qsnprintf(addr, maxAddrSize, "%p:", fnDict);

   QByteArray key = addr;
   key += n;

   FindFileCacheElem *cachedResult = s_findFileDefCache.object(key);
   
   if (cachedResult) {
      ambig = cachedResult->isAmbig;
      
      return cachedResult->fileDef;

   } else {
      cachedResult = new FindFileCacheElem(0, false);
   }

   QByteArray name = QDir::cleanPath(n).toUtf8();
   QString path;

   int slashPos;

   FileName *fn;
   if (name.isEmpty()) {
      goto exit;
   }

   slashPos = qMax(name.lastIndexOf('/'), name.lastIndexOf('\\'));

   if (slashPos != -1) {
      path = name.left(slashPos + 1);
      name = name.right(name.length() - slashPos - 1);      
   }

   if (name.isEmpty()) {
      goto exit;
   }

   //
   fn = (*fnDict)[name].data();

   if (fn) {      
      if (fn->count() == 1) {
         FileDef *fd = fn->first();

#if defined(_WIN32) || defined(__MACOSX__) 
         // Windows or MacOSX
         bool isSamePath = fd->getPath().right(path.length()).toLower() == path.toLower();
#else 
         // Unix
         bool isSamePath = fd->getPath().right(path.length()) == path;
#endif

         if (path.isEmpty() || isSamePath) {
            cachedResult->fileDef = fd;
            s_findFileDefCache.insert(key, cachedResult);
           
            return fd;
         }

      } else { // file name alone is ambiguous
         int count = 0;
       
         FileDef *lastMatch = 0;
         QString pathStripped = stripFromIncludePath(path);
         
         for (auto fd : *fn) {
            QString fdStripPath = stripFromIncludePath(fd->getPath());

            if (path.isEmpty() || fdStripPath.right(pathStripped.length()) == pathStripped) {
               count++;
               lastMatch = fd;
            }
         }
         
         ambig = (count > 1);
         cachedResult->isAmbig = ambig;
         cachedResult->fileDef = lastMatch;

         s_findFileDefCache.insert(key, cachedResult);

         return lastMatch;
      }   
   }

exit:   
   s_findFileDefCache.insert(key, cachedResult);
  
   return QSharedPointer<FileDef>();
}

QByteArray showFileDefMatches(const FileNameDict *fnDict, const char *n)
{
   QByteArray result;
   QByteArray name = n;
   QByteArray path;

   int slashPos = qMax(name.lastIndexOf('/'), name.lastIndexOf('\\'));

   if (slashPos != -1) {
      path = name.left(slashPos + 1);
      name = name.right(name.length() - slashPos - 1);
   }

   FileName *fn;
   fn = (*fnDict)[name].data();  

   if (fn) { 
      for (auto fd : *fn) {
         if (path.isEmpty() || fd->getPath().right(path.length()) == path) {
            result += "   " + fd->getFilePath() + "\n";
         }
      }
   }

   return result;
}

/// substitute all occurrences of \a old in \a str by \a dest
QByteArray substitute(const QByteArray &origString, const QByteArray &oldWord, const QByteArray &newWord)
{  
   QByteArray retval = origString;   
   retval.replace(oldWord, newWord);

   return retval;
}

QByteArray substituteKeywords(const QByteArray &s, const char *title, const char *projName, 
                              const char *projNum, const char *projBrief)
{
   QByteArray result = s;

   if (title) {
      result = substitute(result, "$title", title);
   }

   result = substitute(result, "$datetime",       dateToString(true));
   result = substitute(result, "$date",           dateToString(false));
   result = substitute(result, "$year",           yearToString().toUtf8());
   result = substitute(result, "$doxygenversion", versionString);
   result = substitute(result, "$projectname",    projName);
   result = substitute(result, "$projectnumber",  projNum);
   result = substitute(result, "$projectbrief",   projBrief);
   result = substitute(result, "$projectlogo",   stripPath(Config_getString("PROJECT_LOGO")));

   return result;
}

/*! Returns the character index within \a name of the first prefix
 *  in Config_getList("IGNORE_PREFIX") that matches \a name at the left hand side,
 *  or zero if no match was found
 */
int getPrefixIndex(const QByteArray &name)
{
   if (name.isEmpty()) {
      return 0;
   }

   static QStringList &sl = Config_getList("IGNORE_PREFIX");  

   for (auto s : sl) { 
      QByteArray temp = s.toUtf8();
        
      const char *ps = temp.constData();
      const char *pd = name.constData();

      int i = 0;

      while (*ps != 0 && *pd != 0 && *ps == *pd) {
         ps++, pd++, i++;
      }

      if (*ps == 0 && *pd != 0) {
         return i;
      }      
   }

   return 0;
}

static void initBaseClassHierarchy(SortedList<BaseClassDef *> *bcl)
{
   if (bcl == 0) {
      return;
   }
 
   for (auto item : *bcl) {
      ClassDef *cd = item->classDef;

      if (cd->baseClasses() == 0) { 
         // no base classes => new root
         initBaseClassHierarchy(cd->baseClasses());
      }

      cd->visited = false;
   }
}

bool classHasVisibleChildren(ClassDef *cd)
{
   SortedList<BaseClassDef *> *bcl;

   if (cd->subClasses() == 0) {
      return false;
   }

   bcl = cd->subClasses(); 

   for (auto item : *bcl) {
      if (item->classDef->isVisibleInHierarchy()) {
         return true;
      }
   }

   return false;
}

void initClassHierarchy(ClassSDict *cl)
{  
   for (auto cd : *cl) {
      cd->visited = false;
      initBaseClassHierarchy(cd->baseClasses());
   }
}

bool hasVisibleRoot(SortedList<BaseClassDef *> *bcl)
{
   if (bcl) {

      for (auto item : *bcl) { 
         ClassDef *cd = item->classDef;

         if (cd->isVisibleInHierarchy()) {
            return true;
         }

         hasVisibleRoot(cd->baseClasses());
      }
   }

   return false;
}

// note that this function is not reentrant due to the use of static growBuf!
QByteArray escapeCharsInString(const char *name, bool allowDots, bool allowUnderscore)
{
   static bool caseSenseNames    = Config_getBool("CASE_SENSE_NAMES");
   static bool allowUnicodeNames = Config_getBool("ALLOW_UNICODE_NAMES");
   static GrowBuf growBuf;

   growBuf.clear();
   char c;
   const char *p = name;

   while ((c = *p++) != 0) {
      switch (c) {
         case '_':
            if (allowUnderscore) {
               growBuf.addChar('_');
            } else {
               growBuf.addStr("__");
            }
            break;
         case '-':
            growBuf.addChar('-');
            break;
         case ':':
            growBuf.addStr("_1");
            break;
         case '/':
            growBuf.addStr("_2");
            break;
         case '<':
            growBuf.addStr("_3");
            break;
         case '>':
            growBuf.addStr("_4");
            break;
         case '*':
            growBuf.addStr("_5");
            break;
         case '&':
            growBuf.addStr("_6");
            break;
         case '|':
            growBuf.addStr("_7");
            break;
         case '.':
            if (allowDots) {
               growBuf.addChar('.');
            } else {
               growBuf.addStr("_8");
            }
            break;
         case '!':
            growBuf.addStr("_9");
            break;
         case ',':
            growBuf.addStr("_00");
            break;
         case ' ':
            growBuf.addStr("_01");
            break;
         case '{':
            growBuf.addStr("_02");
            break;
         case '}':
            growBuf.addStr("_03");
            break;
         case '?':
            growBuf.addStr("_04");
            break;
         case '^':
            growBuf.addStr("_05");
            break;
         case '%':
            growBuf.addStr("_06");
            break;
         case '(':
            growBuf.addStr("_07");
            break;
         case ')':
            growBuf.addStr("_08");
            break;
         case '+':
            growBuf.addStr("_09");
            break;
         case '=':
            growBuf.addStr("_0A");
            break;
         case '$':
            growBuf.addStr("_0B");
            break;
         case '\\':
            growBuf.addStr("_0C");
            break;
         default:
            if (c < 0) {
               char ids[5];
               const unsigned char uc = (unsigned char)c;
               bool doEscape = true;
               if (allowUnicodeNames && uc <= 0xf7) {
                  const char *pt = p;
                  ids[ 0 ] = c;
                  int l = 0;
                  if ((uc & 0xE0) == 0xC0) {
                     l = 2; // 11xx.xxxx: >=2 byte character
                  }
                  if ((uc & 0xF0) == 0xE0) {
                     l = 3; // 111x.xxxx: >=3 byte character
                  }
                  if ((uc & 0xF8) == 0xF0) {
                     l = 4; // 1111.xxxx: >=4 byte character
                  }
                  doEscape = l == 0;
                  for (int m = 1; m < l && !doEscape; ++m) {
                     unsigned char ct = (unsigned char) * pt;
                     if (ct == 0 || (ct & 0xC0) != 0x80) { // invalid unicode character
                        doEscape = true;
                     } else {
                        ids[ m ] = *pt++;
                     }
                  }
                  if ( !doEscape ) { // got a valid unicode character
                     ids[ l ] = 0;
                     growBuf.addStr( ids );
                     p += l - 1;
                  }
               }
               if (doEscape) { // not a valid unicode char or escaping needed
                  static char map[] = "0123456789ABCDEF";
                  unsigned char id = (unsigned char)c;
                  ids[0] = '_';
                  ids[1] = 'x';
                  ids[2] = map[id >> 4];
                  ids[3] = map[id & 0xF];
                  ids[4] = 0;
                  growBuf.addStr(ids);
               }
            } else if (caseSenseNames || !isupper(c)) {
               growBuf.addChar(c);
            } else {
               growBuf.addChar('_');
               growBuf.addChar(tolower(c));
            }
            break;
      }
   }
   growBuf.addChar(0);
   return growBuf.get();
}

/*! This function determines the file name on disk of an item
 *  given its name, which could be a class name with template
 *  arguments, so special characters need to be escaped.
 */
QString convertNameToFile(const char *name, bool allowDots, bool allowUnderscore)
{
   static bool shortNames    = Config_getBool("SHORT_NAMES");
   static bool createSubdirs = Config_getBool("CREATE_SUBDIRS");

   QString result;

   if (shortNames) { 
      // use short names only
      static QHash<QString, int> usedNames;
     
      static int count = 1;
      int num;

      auto value = usedNames.find(name);     

      if (value != usedNames.end()) {
         usedNames.insert(name, count);
         num = count++;

      } else {
         num = *value;
      }

      result = QString("a%1").arg(num, 5, 10, QChar('0'));

   } else { 
      // long names
      result = escapeCharsInString(name, allowDots, allowUnderscore);
      int resultLen = result.length();

      if (resultLen >= 128) { 
         // prevent names that cannot be created
         // third algorithm based on MD5 hash
      
         QByteArray sigStr;
         sigStr = QCryptographicHash::hash(result.toUtf8(), QCryptographicHash::Md5).toHex();  

         result = result.left(128 - 32) + sigStr;
      }
   }

   if (createSubdirs) {
      int l1Dir = 0;
      int l2Dir = 0;

#if MAP_ALGO == ALGO_COUNT
      // old algorithm, has the problem that after regeneration the
      // output can be located in a different dir.

      if (Doxygen::htmlDirMap == 0) {
         Doxygen::htmlDirMap = new QHash<QString, int>();         
      }

      static int curDirNum = 0;
      int *dirNum = Doxygen::htmlDirMap->find(result);

      if (dirNum == 0) { 
         // new name
         Doxygen::htmlDirMap->insert(result, new int(curDirNum));
         l1Dir = (curDirNum) & 0xf;        // bits 0-3
         l2Dir = (curDirNum >> 4) & 0xff;  // bits 4-11
         curDirNum++;

      } else { 
         // existing name
         l1Dir = (*dirNum) & 0xf;          // bits 0-3
         l2Dir = ((*dirNum) >> 4) & 0xff;  // bits 4-11
      }

#elif MAP_ALGO == ALGO_CRC16
      // second algorithm based on CRC-16 checksum
      int dirNum = qChecksum(result, result.length());

      l1Dir = dirNum & 0xf;
      l2Dir = (dirNum >> 4) & 0xff;

#elif MAP_ALGO == ALGO_MD5
      // third algorithm based on MD5 hash
 
      QByteArray sigStr;
      sigStr = QCryptographicHash::hash(result.toUtf8(), QCryptographicHash::Md5);  

      l1Dir = sigStr[14] & 0xf;
      l2Dir = sigStr[15];
#endif

      result = QString("d%1/d%2/").arg(l1Dir, 0, 16).arg(l2Dir, 2, 16, QChar('0')) + result;
   }
   
   return result;
}

QByteArray relativePathToRoot(const char *name)
{
   QByteArray result;

   if (Config_getBool("CREATE_SUBDIRS")) {
      if (name == 0) {
         return REL_PATH_TO_ROOT;

      } else {
         QByteArray n = name;
         int i = n.lastIndexOf('/');

         if (i != -1) {
            result = REL_PATH_TO_ROOT;
         }
      }
   }
   return result;
}

void createSubDirs(QDir &d)
{
   if (Config_getBool("CREATE_SUBDIRS")) {
      // create 4096 subdirectories

      int l1;
      int l2;

      for (l1 = 0; l1 < 16; l1++) {
         QString temp;
         QString("d%1").arg(l1, 0, 16);

         d.mkdir(temp);

         for (l2 = 0; l2 < 256; l2++) {
            QString temp;
            QString("d%1/d%2").arg(l1, 0, 16).arg(l2, 2, 16, QChar('0'));

            d.mkdir(temp);
         }
      }
   }
}

/*! Input is a scopeName, output is the scopename split into a
 *  namespace part (as large as possible) and a classname part.
 */
void extractNamespaceName(const QByteArray &scopeName, QByteArray &className, QByteArray &namespaceName, bool allowEmptyClass)
{
   int i, p;
   QByteArray clName = scopeName;
   
   QSharedPointer<NamespaceDef> nd;

   if (! clName.isEmpty() && (nd = getResolvedNamespace(clName)) && getClass(clName) == 0) {
      // the whole name is a namespace (and not a class)
      namespaceName = nd->name();

      className.resize(0);
      goto done;
   }

   p = clName.length() - 2;
   while (p >= 0 && (i = clName.lastIndexOf("::", p)) != -1)
      // see if the first part is a namespace (and not a class)
   {
      //printf("Trying %s\n",clName.left(i).data());
      if (i > 0 && (nd = getResolvedNamespace(clName.left(i))) && getClass(clName.left(i)) == 0) {
         //printf("found!\n");
         namespaceName = nd->name();
         className = clName.right(clName.length() - i - 2);
         goto done;
      }
      p = i - 2; // try a smaller piece of the scope
   }
  
   // not found, so we just have to guess.
   className = scopeName;
   namespaceName.resize(0);

done:
   if (className.isEmpty() && !namespaceName.isEmpty() && !allowEmptyClass) {
      // class and namespace with the same name, correct to return the class.
      className = namespaceName;
      namespaceName.resize(0);
   }
   //printf("extractNamespace `%s' => `%s|%s'\n",scopeName.data(),
   //       className.data(),namespaceName.data());
   if (/*className.right(2)=="-g" ||*/ className.right(2) == "-p") {
      className = className.left(className.length() - 2);
   }
   return;
}

QByteArray insertTemplateSpecifierInScope(const QByteArray &scope, const QByteArray &templ)
{
   QByteArray result = scope;

   if (!templ.isEmpty() && scope.indexOf('<') == -1) {
      int si, pi = 0;
      ClassDef *cd = 0;

      while ( (si = scope.indexOf("::", pi)) != -1 && ! getClass(scope.left(si) + templ) &&
         ((cd = getClass(scope.left(si))) == 0 || cd->templateArguments() == 0) ) {
         //printf("Tried `%s'\n",(scope.left(si)+templ).data());
         pi = si + 2;
      }

      if (si == -1) { // not nested => append template specifier
         result += templ;

      } else { // nested => insert template specifier before after first class name
         result = scope.left(si) + templ + scope.right(scope.length() - si);
      }
   }
   
   return result;
}

// new version by Davide Cesari which also works for Fortran
QByteArray stripScope(const char *name)
{
   QByteArray result = name;
   int l = result.length();
   int p;

   bool done = false;
   bool skipBracket = false; // if brackets do not match properly, ignore them altogether

   int count = 0;

   do {
      p = l - 1; // start at the end of the string

      while (p >= 0 && count >= 0) {
         char c = result.at(p);

         switch (c) {
            case ':':
               // only exit in the case of ::
               //printf("stripScope(%s)=%s\n",name,result.right(l-p-1).data());
               if (p > 0 && result.at(p - 1) == ':') {
                  return result.right(l - p - 1);
               }
               p--;
               break;
            case '>':
               if (skipBracket) { // we don't care about brackets
                  p--;
               } else { // count open/close brackets
                  if (p > 0 && result.at(p - 1) == '>') { // skip >> operator
                     p -= 2;
                     break;
                  }
                  count = 1;
                  
                  p--;
                  bool foundMatch = false;

                  while (p >= 0 && !foundMatch) {
                     c = result.at(p--);

                     switch (c) {
                        case '>':
                           count++;
                           break;
                        case '<':
                           if (p > 0) {
                              if (result.at(p - 1) == '<') { // skip << operator
                                 p--;
                                 break;
                              }
                           }
                           count--;
                           foundMatch = count == 0;
                           break;
                        default:
                           //printf("c=%c count=%d\n",c,count);
                           break;
                     }
                  }
               }
               //printf("pos > = %d\n",p+1);
               break;
            default:
               p--;
         }
      }
      done = count == 0 || skipBracket; // reparse if brackets do not match
      skipBracket = true;
   } while (!done); // if < > unbalanced repeat ignoring them
   //printf("stripScope(%s)=%s\n",name,name);
   return name;
}


/*! Converts a string to an XML-encoded string */
QByteArray convertToXML(const QString &s)
{
   static GrowBuf growBuf;
   growBuf.clear();

   if (s == 0) {
      return "";
   }

   QByteArray temp = s.toUtf8();

   const char *p = temp.constData();
   char c;

   while ((c = *p++)) {
      switch (c) {
         case '<':
            growBuf.addStr("&lt;");
            break;
         case '>':
            growBuf.addStr("&gt;");
            break;
         case '&':
            growBuf.addStr("&amp;");
            break;
         case '\'':
            growBuf.addStr("&apos;");
            break;
         case '"':
            growBuf.addStr("&quot;");
            break;
         case  1:
         case  2:
         case  3:
         case  4:
         case  5:
         case  6:
         case  7:
         case  8:
         case 11:
         case 12:
         case 13:
         case 14:
         case 15:
         case 16:
         case 17:
         case 18:
         case 19:
         case 20:
         case 21:
         case 22:
         case 23:
         case 24:
         case 25:
         case 26:
         case 27:
         case 28:
         case 29:
         case 30:
         case 31:
            break; // skip invalid XML characters (see http://www.w3.org/TR/2000/REC-xml-20001006#NT-Char)
         default:
            growBuf.addChar(c);
            break;
      }
   }
   growBuf.addChar(0);
   return growBuf.get();
}

/*! Converts a string to a HTML-encoded string */
QByteArray convertToHtml(const char *s, bool keepEntities)
{
   static GrowBuf growBuf;
   growBuf.clear();
   if (s == 0) {
      return "";
   }
   const char *p = s;
   char c;
   while ((c = *p++)) {
      switch (c) {
         case '<':
            growBuf.addStr("&lt;");
            break;
         case '>':
            growBuf.addStr("&gt;");
            break;
         case '&':
            if (keepEntities) {
               const char *e = p;
               char ce;
               while ((ce = *e++)) {
                  if (ce == ';' || (!(isId(ce) || ce == '#'))) {
                     break;
                  }
               }
               if (ce == ';') { // found end of an entity
                  // copy entry verbatim
                  growBuf.addChar(c);
                  while (p < e) {
                     growBuf.addChar(*p++);
                  }
               } else {
                  growBuf.addStr("&amp;");
               }
            } else {
               growBuf.addStr("&amp;");
            }
            break;
         case '\'':
            growBuf.addStr("&#39;");
            break;
         case '"':
            growBuf.addStr("&quot;");
            break;
         default:
            growBuf.addChar(c);
            break;
      }
   }
   growBuf.addChar(0);
   return growBuf.get();
}

QByteArray convertToJSString(const char *s)
{
   static GrowBuf growBuf;
   growBuf.clear();
   if (s == 0) {
      return "";
   }
   const char *p = s;
   char c;
   while ((c = *p++)) {
      switch (c) {
         case '"':
            growBuf.addStr("\\\"");
            break;
         case '\\':
            growBuf.addStr("\\\\");
            break;
         default:
            growBuf.addChar(c);
            break;
      }
   }
   growBuf.addChar(0);
   return convertCharEntitiesToUTF8(growBuf.get());
}


QByteArray convertCharEntitiesToUTF8(const QByteArray &s)
{
   QByteArray result;
   static QRegExp entityPat("&[a-zA-Z]+[0-9]*;");

   if (s.length() == 0) {
      return result;
   }

   static GrowBuf growBuf;
   growBuf.clear();
   int p, i = 0, l;

   while ((p = entityPat.indexIn(s, i)) != -1) {
      l = entityPat.matchedLength();

      if (p > i) {
         growBuf.addStr(s.mid(i, p - i));
      }

      QByteArray entity = s.mid(p, l);
      DocSymbol::SymType symType = HtmlEntityMapper::instance()->name2sym(entity);
      const char *code = 0;

      if (symType != DocSymbol::Sym_Unknown && (code = HtmlEntityMapper::instance()->utf8(symType))) {
         growBuf.addStr(code);
      } else {
         growBuf.addStr(s.mid(p, l));
      }

      i = p + l;
   }

   growBuf.addStr(s.mid(i, s.length() - i));
   growBuf.addChar(0);
   
   return growBuf.get();
}

/*! Returns the standard string that is generated when the \\overload
 * command is used.
 */
QByteArray getOverloadDocs()
{
   return theTranslator->trOverloadText();
   //"This is an overloaded member function, "
   //       "provided for convenience. It differs from the above "
   //       "function only in what argument(s) it accepts.";
}

void addMembersToMemberGroup(MemberList *ml, MemberGroupSDict **ppMemberGroupSDict, Definition *context)
{
   assert(context != 0);
   
   if (ml == 0) {
      return;
   }
   
   uint index = 0;
   
   for (auto md : *ml) {

      if (md->isEnumerate()) { 
         // insert enum value of this enum into groups
         MemberList *fmdl = md->enumFieldList();

         if (fmdl != 0) {           
            for (auto fmd : *fmdl) {
               int groupId = fmd->getMemberGroupId();

               if (groupId != -1) {
                  QSharedPointer<MemberGroupInfo> info = Doxygen::memGrpInfoDict[groupId];
                 
                  if (info) {
                     if (*ppMemberGroupSDict == 0) {
                        *ppMemberGroupSDict = new MemberGroupSDict;                        
                     }

                     QSharedPointer<MemberGroup> mg = (*ppMemberGroupSDict)->find(groupId);

                     if (mg == 0) {
                        mg = QSharedPointer<MemberGroup>(new MemberGroup(context, groupId, info->header, info->doc, info->docFile));
                        (*ppMemberGroupSDict)->insert(groupId, mg);
                     }

                     mg->insertMember(fmd); // insert in member group
                     fmd->setMemberGroup(mg.data());
                  }
               }
            }
         }
      }

      int groupId = md->getMemberGroupId();

      if (groupId != -1) {
         QSharedPointer<MemberGroupInfo> info = Doxygen::memGrpInfoDict[groupId];

         if (info) {

            if (*ppMemberGroupSDict == 0) {
               *ppMemberGroupSDict = new MemberGroupSDict;               
            }

            QSharedPointer<MemberGroup> mg = (*ppMemberGroupSDict)->find(groupId);

            if (mg) {
               mg = QSharedPointer<MemberGroup>(new MemberGroup( context, groupId, info->header, info->doc, info->docFile));
               (*ppMemberGroupSDict)->insert(groupId, mg);
            }

            md = ml->takeAt(index);     // remove from member list
            mg->insertMember(md);       // insert in member group
            mg->setRefItems(info->m_sli);

            md->setMemberGroup(mg.data());

            continue;
         }
      }
     
      ++index;
   }
}

/*! Extracts a (sub-)string from \a type starting at \a pos that
 *  could form a class. The index of the match is returned and the found
 *  class \a name and a template argument list \a templSpec. If -1 is returned
 *  there are no more matches.
 */
int extractClassNameFromType(const QByteArray &type, int &pos, QByteArray &name, QByteArray &templSpec, SrcLangExt lang)
{
   static const QRegExp re_norm("[a-z_A-Z\\x80-\\xFF][a-z_A-Z0-9:\\x80-\\xFF]*");
   static const QRegExp re_ftn("[a-z_A-Z\\x80-\\xFF][()=_a-z_A-Z0-9:\\x80-\\xFF]*");
   QRegExp re;

   name.resize(0);
   templSpec.resize(0);

   int i;
   int l;
   int typeLen = type.length();

   if (typeLen > 0) {

      if (lang == SrcLangExt_Fortran) {

         if (typeLen <= pos) {
            return -1;

         } else if (type.at(pos) == ',') {
            return -1;

         }

         if (type.left(4).toLower() == "type") {
            re = re_norm;

         } else {
            re = re_ftn;

         }

      } else {
         re = re_norm;

      }

      if ((i = re.indexIn(type, pos)) != -1) { 
         // for each class name in the type
         l = re.matchedLength();

         int ts = i + l;
         int te = ts;
         int tl = 0;

         while (ts < typeLen && type.at(ts) == ' ') {
            ts++, tl++;   // skip any whitespace
         }

         if (ts < typeLen && type.at(ts) == '<') { // assume template instance
            // locate end of template
            te = ts + 1;
            int brCount = 1;

            while (te < typeLen && brCount != 0) {

               if (type.at(te) == '<') {
                  if (te < typeLen - 1 && type.at(te + 1) == '<') {
                     te++;
                  } else {
                     brCount++;
                  }
               }

               if (type.at(te) == '>') {
                  if (te < typeLen - 1 && type.at(te + 1) == '>') {
                     te++;
                  } else {
                     brCount--;
                  }
               }
               te++;
            }
         }

         name = type.mid(i, l);

         if (te > ts) {
            templSpec = type.mid(ts, te - ts), tl += te - ts;
            pos = i + l + tl;

         } else { // no template part
            pos = i + l;
         }

         
         return i;
      }
   }

   pos = typeLen;
  
   return -1;
}

QByteArray normalizeNonTemplateArgumentsInString(const QByteArray &name, Definition *context, const ArgumentList *formalArgs)
{
   // skip until <
   int p = name.indexOf('<');

   if (p == -1) {
      return name;
   }

   p++;
   QByteArray result = name.left(p);

   static QRegExp re("[a-z_A-Z\\x80-\\xFF][a-z_A-Z0-9\\x80-\\xFF]*");
   int l, i;

   // for each identifier in the template part (e.g. B<T> -> T)
   while ((i = re.indexIn(name, p)) != -1) {
      l = re.matchedLength();

      result += name.mid(p, i - p);
      QByteArray n = name.mid(i, l);
      bool found = false;

      if (formalArgs) { 
         // check that n is not a formal template argument
                
         for (auto formArg : *formalArgs) {   
            if (found) {
               break;
            }

            found = formArg.name == n;
         }
      }

      if (!found) {
         // try to resolve the type
         QSharedPointer<ClassDef> cd = getResolvedClass(context, 0, n);

         if (cd) {
            result += cd->name();
         } else {
            result += n;
         }

      } else {
         result += n;
      }

      p = i + l;
   }

   result += name.right(name.length() - p);
   
   return removeRedundantWhiteSpace(result);
}


/*! Substitutes any occurrence of a formal argument from argument list
 *  \a formalArgs in \a name by the corresponding actual argument in
 *  argument list \a actualArgs. The result after substitution
 *  is returned as a string. The argument \a name is used to
 *  prevent recursive substitution.
 */
QByteArray substituteTemplateArgumentsInString(const QByteArray &name, ArgumentList *formalArgs, ArgumentList *actualArgs)
{  
   if (formalArgs == 0) {
      return name;
   }

   QByteArray result;

   static QRegExp re("[a-z_A-Z\\x80-\\xFF][a-z_A-Z0-9\\x80-\\xFF]*");
   int p = 0, l, i;

   // for each identifier in the base class name (e.g. B<T> -> B and T)

   while ((i = re.indexIn(name, p)) != -1) {
      l = re.matchedLength();

      result += name.mid(p, i - p);
      QByteArray n = name.mid(i, l);
 
      // if n is a template argument, then we substitute it
      // for its template instance argument.
      bool found = false;
     
      auto act_iter = actualArgs->begin();

      for (auto formArg : *formalArgs) {   

         if (found) {
            break;
         }

         Argument *actArg = nullptr; 

         if (act_iter != actualArgs->end()) {   
            actArg = &(*act_iter);
         }

         if (formArg.type.left(6) == "class " && formArg.name.isEmpty()) {
            formArg.name = formArg.type.mid(6);
            formArg.type = "class";
         }

         if (formArg.type.left(9) == "typename " && formArg.name.isEmpty()) {
            formArg.name = formArg.type.mid(9);
            formArg.type = "typename";
         }


         if (formArg.type == "class" || formArg.type == "typename" || formArg.type.startsWith("template")) {
          
            if (formArg.name == n && actArg != nullptr  && ! actArg->type.isEmpty()) { 
               // base class is a template argument
               // replace formal argument with the actual argument of the instance

               if (! leftScopeMatch(actArg->type, n)) {
                  // the scope guard is to prevent recursive lockup for template<class A> class C : public<A::T>,
                  // where A::T would become A::T::T here, // since n==A and actArg->type==A::T
                                 
                  if (actArg->name.isEmpty()) {
                     result += actArg->type + " ";
                     found = true;

                  } else {
                     // for case where the actual arg is something like "unsigned int"
                     // the "int" part is in actArg->name
                  
                     result += actArg->type + " " + actArg->name + " ";
                     found = true;
                  }
               }

            } else if (formArg.name == n && !formArg.defval.isEmpty() && formArg.defval != name) {
               /* to prevent recursion */
                     
               result += substituteTemplateArgumentsInString(formArg.defval, formalArgs, actualArgs) + " ";
               found = true;
            }

         } else if (formArg.name == n && ! formArg.defval.isEmpty() && formArg.defval != name) {
            /* to prevent recursion */
                   
            result += substituteTemplateArgumentsInString(formArg.defval, formalArgs, actualArgs) + " ";
            found = true;
         }

         if (act_iter != actualArgs->end()) {
            ++act_iter;
         }       

      }

      if (! found) {
         result += n;
      }

      p = i + l;
   }

   result += name.right(name.length() - p);

   return result.trimmed();
}

/*! Makes a deep copy of the list of argument lists \a srcLists.
 *  Will allocate memory, that is owned by the caller.
 */
QList<ArgumentList> *copyArgumentLists(const QList<ArgumentList> *srcLists)
{
   assert(srcLists != 0);

   QList<ArgumentList> *dstLists = new QList<ArgumentList>;
  
   for (auto sl : *srcLists ) {      
      dstLists->append(sl);     
   }

   return dstLists;
}

/*! Strips template specifiers from scope \a fullName, except those
 *  that make up specialized classes. The switch \a parentOnly
 *  determines whether or not a template "at the end" of a scope
 *  should be considered, e.g. with \a parentOnly is \c true, A<T>::B<S> will
 *  try to strip \<T\> and not \<S\>, while \a parentOnly is \c false will
 *  strip both unless A<T> or B<S> are specialized template classes.
 */
QByteArray stripTemplateSpecifiersFromScope(const QByteArray &fullName, bool parentOnly, QByteArray *pLastScopeStripped)
{
   QByteArray result;

   int p = 0;
   int l = fullName.length();
   int i = fullName.indexOf('<');

   while (i != -1) {

      int e = i + 1;
      bool done = false;
      int count = 1;

      while (e < l && ! done) {
         char c = fullName.at(e++);

         if (c == '<') {
            count++;

         } else if (c == '>') {
            count--;
            done = count == 0;
         }
      }
      int si = fullName.indexOf("::", e);

      if (parentOnly && si == -1) {
         break;
      }
      // we only do the parent scope, so we stop here if needed

      result += fullName.mid(p, i - p);

      if (getClass(result + fullName.mid(i, e - i)) != 0) { 
         result += fullName.mid(i, e - i);
         //printf("  2:result+=%s\n",fullName.mid(i,e-i-1).data());

      } else if (pLastScopeStripped) {
         //printf("  last stripped scope '%s'\n",fullName.mid(i,e-i).data());
         *pLastScopeStripped = fullName.mid(i, e - i);
      }

      p = e;
      i = fullName.indexOf('<', p);
   }

   result += fullName.right(l - p);
  
   return result;
}

/*! Merges two scope parts together. The parts may (partially) overlap.
 *  Example1: \c A::B and \c B::C will result in \c A::B::C <br>
 *  Example2: \c A and \c B will be \c A::B <br>
 *  Example3: \c A::B and B will be \c A::B
 *
 *  @param leftScope the left hand part of the scope.
 *  @param rightScope the right hand part of the scope.
 *  @returns the merged scope.
 */
QByteArray mergeScopes(const QByteArray &leftScope, const QByteArray &rightScope)
{
   // case leftScope=="A" rightScope=="A::B" => result = "A::B"
   if (leftScopeMatch(rightScope, leftScope)) {
      return rightScope;
   }
   QByteArray result;
   int i = 0, p = leftScope.length();

   // case leftScope=="A::B" rightScope=="B::C" => result = "A::B::C"
   // case leftScope=="A::B" rightScope=="B" => result = "A::B"
   bool found = false;
   while ((i = leftScope.lastIndexOf("::", p)) != -1) {
      if (leftScopeMatch(rightScope, leftScope.right(leftScope.length() - i - 2))) {
         result = leftScope.left(i + 2) + rightScope;
         found = true;
      }
      p = i - 1;
   }
   if (found) {
      return result;
   }

   // case leftScope=="A" rightScope=="B" => result = "A::B"
   result = leftScope;
   if (!result.isEmpty() && !rightScope.isEmpty()) {
      result += "::";
   }
   result += rightScope;
   return result;
}

/*! Returns a fragment from scope \a s, starting at position \a p.
 *
 *  @param s the scope name as a string.
 *  @param p the start position (0 is the first).
 *  @param l the resulting length of the fragment.
 *  @returns the location of the fragment, or -1 if non is found.
 */
int getScopeFragment(const QByteArray &s, int p, int *l)
{
   int sl = s.length();
   int sp = p;
   int count = 0;

   bool done;

   if (sp >= sl) {
      return -1;
   }

   while (sp < sl) {
      char c = s.at(sp);

      if (c == ':') {
         sp++, p++;

      } else {
         break;
      }
   }

   bool goAway = false;

   while (sp < sl) {
      char c = s.at(sp);

      switch (c) {
         case ':': 
            // found next part
            goAway = true;
            break;

         case '<': 
            // skip template specifier
            count = 1;
            sp++;

            done = false;

            while (sp < sl && !done) {
               // TODO: deal with << and >> operators!
               char c = s.at(sp++);

               switch (c) {
                  case '<':
                     count++;
                     break;

                  case '>':
                     count--;

                     if (count == 0) {
                        done = true;
                     }
                     break;

                  default:
                     break;
               }
            }
            break;

         default:
            sp++;
            break;
      }

      if (goAway) {
         break;
      }

   }

   *l = sp - p;
  
   return p;
}

PageDef *addRelatedPage(const char *name, const QByteArray &ptitle, const QByteArray &doc, QList<SectionInfo> * /*anchors*/,
                        const char *fileName, int startLine, const QList<ListItemInfo> *sli, GroupDef *gd,
                        TagInfo *tagInfo, SrcLangExt lang)
{
   QSharedPointer<PageDef> pd;

   if ((pd = Doxygen::pageSDict->find(name)) && ! tagInfo) {
      // append documentation block to the page
      pd->setDocumentation(doc, fileName, startLine);


   } else { 
      // new page
      QByteArray baseName = name;

      if (baseName.right(4) == ".tex") {
         baseName = baseName.left(baseName.length() - 4);

      } else if (baseName.right(Doxygen::htmlFileExtension.length()) == Doxygen::htmlFileExtension) {
         baseName = baseName.left(baseName.length() - Doxygen::htmlFileExtension.length());
      }

      QByteArray title = ptitle.trimmed();
      pd = QSharedPointer<PageDef>(new PageDef(fileName, startLine, baseName, doc, title));

      pd->setRefItems(sli);
      pd->setLanguage(lang);

      if (tagInfo) {
         pd->setReference(tagInfo->tagName);
         pd->setFileName(tagInfo->fileName, true);

      } else {
         pd->setFileName( qPrintable(convertNameToFile(pd->name(), false, true)), false);

      }
      
      Doxygen::pageSDict->insert(baseName, pd);

      if (gd) {
         gd->addPage(pd);
      }

      if (! pd->title().isEmpty()) {
        
         // a page name is a label as well
         QByteArray file;

         if (gd) {
            file = gd->getOutputFileBase();

         } else {
            file = pd->getOutputFileBase();

         }

         QSharedPointer<SectionInfo> si = Doxygen::sectionDict->find(pd->name());

         if (si) {
            if (si->lineNr != -1) {
               warn(file, -1, "multiple use of section label '%s', (first occurrence: %s, line %d)", 
                    pd->name().data(), si->fileName.data(), si->lineNr);

            } else {
               warn(file, -1, "multiple use of section label '%s', (first occurrence: %s)", 
                    pd->name().data(), si->fileName.data());
            }

         } else {
            si = QSharedPointer<SectionInfo>(new SectionInfo(file, -1, pd->name(), pd->title(), SectionInfo::Page, 0, pd->getReference()));           
            Doxygen::sectionDict->insert(pd->name(), si);
         }
      }
   }

   return pd.data();
}

void addRefItem(const QList<ListItemInfo> *sli, const char *key, 
                const char *prefix, const char *name, const char *title, const char *args, Definition *scope)
{   
   if (sli && key && key[0] != '@') { 
      // check for @ to skip anonymous stuff (see bug427012)
    
      for (auto lii : *sli) {   
         auto refList = Doxygen::xrefLists->find(lii.type);

         if (refList !=   Doxygen::xrefLists->end()   && ( (lii.type != "todo" || Config_getBool("GENERATE_TODOLIST")) &&
                          (lii.type != "test"       || Config_getBool("GENERATE_TESTLIST")) &&
                          (lii.type != "bug"        || Config_getBool("GENERATE_BUGLIST"))  &&
                          (lii.type != "deprecated" || Config_getBool("GENERATE_DEPRECATEDLIST")) ) ) {

            // either not a built-in list or the list is enabled
            RefItem *item = refList->getRefItem(lii.itemId);
            assert(item != 0);

            item->prefix = prefix;
            item->scope  = scope;
            item->name   = name;
            item->title  = title;
            item->args   = args;

            refList->insertIntoList(key, item);
         }
      }
   }
}

bool recursivelyAddGroupListToTitle(OutputList &ol, Definition *d, bool root)
{
   SortedList<GroupDef *> *groups = d->partOfGroups();

   if (groups) { 
      // write list of group to which this definition belongs

      if (root) { 
         ol.pushGeneratorState();
         ol.disableAllBut(OutputGenerator::Html);
         ol.writeString("<div class=\"ingroups\">");
      }
    
      bool first = true;

      for (auto gd : *groups) {

         if (recursivelyAddGroupListToTitle(ol, gd, false)) {
            ol.writeString(" &raquo; ");
         }

         if (! first) {
            ol.writeString(" &#124; ");
         } else {
            first = false;
         }

         ol.writeObjectLink(gd->getReference(), gd->getOutputFileBase(), 0, gd->groupTitle());
      }

      if (root) {
         ol.writeString("</div>");
         ol.popGeneratorState();
      }

      return true;
   }
   return false;
}

void addGroupListToTitle(OutputList &ol, Definition *d)
{
   recursivelyAddGroupListToTitle(ol, d, true);
}

void filterLatexString(QTextStream &t, const char *str, bool insideTabbing, bool insidePre, bool insideItem)
{
   if (str == 0) {
      return;
   }
   const unsigned char *p = (const unsigned char *)str;
   unsigned char c;
   unsigned char pc = '\0';

   while (*p) {
      c = *p++;

      if (insidePre) {
         switch (c) {
            case '\\':
               t << "\\(\\backslash\\)";
               break;
            case '{':
               t << "\\{";
               break;
            case '}':
               t << "\\}";
               break;
            case '_':
               t << "\\_";
               break;
            default:
               t << (char)c;
         }
      } else {
         switch (c) {
            case '#':
               t << "\\#";
               break;
            case '$':
               t << "\\$";
               break;
            case '%':
               t << "\\%";
               break;
            case '^':
               t << "$^\\wedge$";
               break;
            case '&':
               t << "\\&";
               break;
            case '*':
               t << "$\\ast$";
               break;
            case '_':
               if (!insideTabbing) {
                  t << "\\+";
               }
               t << "\\_";
               if (!insideTabbing) {
                  t << "\\+";
               }
               break;
            case '{':
               t << "\\{";
               break;
            case '}':
               t << "\\}";
               break;
            case '<':
               t << "$<$";
               break;
            case '>':
               t << "$>$";
               break;
            case '|':
               t << "$\\vert$";
               break;
            case '~':
               t << "$\\sim$";
               break;
            case '[':
               if (Config_getBool("PDF_HYPERLINKS") || insideItem) {
                  t << "\\mbox{[}";
               } else {
                  t << "[";
               }
               break;
            case ']':
               if (pc == '[') {
                  t << "$\\,$";
               }
               if (Config_getBool("PDF_HYPERLINKS") || insideItem) {
                  t << "\\mbox{]}";
               } else {
                  t << "]";
               }
               break;
            case '-':
               t << "-\\/";
               break;
            case '\\':
               t << "\\textbackslash{}";
               break;
            case '"':
               t << "\\char`\\\"{}";
               break;
            case '\'':
               t << "\\textquotesingle{}";
               break;

            default:
               //if (!insideTabbing && forceBreaks && c!=' ' && *p!=' ')
               if (!insideTabbing &&
                     ((c >= 'A' && c <= 'Z' && pc != ' ' && pc != '\0') || (c == ':' && pc != ':') || (pc == '.' && isId(c)))
                  ) {
                  t << "\\+";
               }
               t << (char)c;
         }
      }
      pc = c;
   }
}

QByteArray rtfFormatBmkStr(const char *name)
{
   static QByteArray nextTag("AAAAAAAAAA");
   static QHash<QString, QByteArray> tagDict;
  
   // To overcome the 40-character tag limitation, we substitute a short arbitrary string for the name
   // supplied, and keep track of the correspondence between names and strings.

   QByteArray key( name );
   auto tag = tagDict.find(key);

   if (tag != tagDict.end()) {
      // This particular name has not yet been added to the list. Add it, associating it with the
      // next tag value, and increment the next tag
    
      tagDict.insert(key, nextTag);
      tag = tagDict.find(key);

      // this is the increment part
      char *tempTag = nextTag.data() + nextTag.length() - 1;

      for ( unsigned int i = 0; i < nextTag.length(); ++i, --tempTag ) {
         if ( ( ++(*tempTag) ) > 'Z' ) {
            *tempTag = 'A';

         } else {
            // since there was no carry, we can stop now
            break;
         }
      }
   }

   return *tag;
}

QString stripExtension(QString fName)
{
   QString result = fName;

   if (result.right(Doxygen::htmlFileExtension.length()) == Doxygen::htmlFileExtension) {
      result = result.left(result.length() - Doxygen::htmlFileExtension.length());
   }

   return result;
}


void replaceNamespaceAliases(QByteArray &scope, int i)
{
   while (i > 0) {
      QByteArray ns = scope.left(i);
      QByteArray s  = Doxygen::namespaceAliasDict[ns];

      if (! s.isEmpty()) {
         scope = s + scope.right(scope.length() - i);
         i = s.length();
      }

      if (i > 0 && ns == scope.left(i)) {
         break;
      }
   }
}

QByteArray stripPath(const char *s)
{
   QByteArray result = s;
   int i = result.lastIndexOf('/');

   if (i != -1) {
      result = result.mid(i + 1);
   }

   i = result.lastIndexOf('\\');

   if (i != -1) {
      result = result.mid(i + 1);
   }

   return result;
}

/** returns \c true iff string \a s contains word \a w */
bool containsWord(const QByteArray &s, const QByteArray &word)
{
   static QRegExp wordExp("[a-z_A-Z\\x80-\\xFF]+");
   int p = 0, i, l;

   while ((i = wordExp.indexIn(s, p)) != -1) {
      l = wordExp.matchedLength();

      if (s.mid(i, l) == word) {
         return true;
      }

      p = i + l;
   }

   return false;
}

bool findAndRemoveWord(QByteArray &s, const QByteArray &word)
{
   static QRegExp wordExp("[a-z_A-Z\\x80-\\xFF]+");
   int p = 0, i, l;

   while ((i = wordExp.indexIn(s, p)) != -1) {
      l = wordExp.matchedLength();

      if (s.mid(i, l) == word) {
         if (i > 0 && isspace((uchar)s.at(i - 1))) {
            i--, l++;

         } else if (i + l < s.length() && isspace(s.at(i + l))) {
            l++;

         }

         s = s.left(i) + s.mid(i + l); // remove word + spacing
         return true;
      }
      p = i + l;

   }
   return false;
}

/** Special version of QByteArray::trimmed() that only strips
 *  completely blank lines.
 *  @param s the string to be stripped
 *  @param docLine the line number corresponding to the start of the
 *         string. This will be adjusted based on the number of lines stripped
 *         from the start.
 *  @returns The stripped string.
 */
QByteArray stripLeadingAndTrailingEmptyLines(const QByteArray &s, int &docLine)
{
   const char *p = s.data();
   if (p == 0) {
      return 0;
   }

   // search for leading empty lines
   int i = 0, li = -1, l = s.length();
   char c;
   while ((c = *p++)) {
      if (c == ' ' || c == '\t' || c == '\r') {
         i++;
      } else if (c == '\n') {
         i++, li = i, docLine++;
      } else {
         break;
      }
   }

   // search for trailing empty lines
   int b = l - 1, bi = -1;
   p = s.data() + b;
   while (b >= 0) {
      c = *p;
      p--;
      if (c == ' ' || c == '\t' || c == '\r') {
         b--;
      } else if (c == '\n') {
         bi = b, b--;
      } else {
         break;
      }
   }

   // return whole string if no leading or trailing lines where found
   if (li == -1 && bi == -1) {
      return s;
   }

   // return substring
   if (bi == -1) {
      bi = l;
   }
   if (li == -1) {
      li = 0;
   }
   if (bi <= li) {
      return 0;   // only empty lines
   }
   return s.mid(li, bi - li);
}

#if 0
void stringToSearchIndex(const QByteArray &docBaseUrl, const QByteArray &title,
                         const QByteArray &str, bool priority, const QByteArray &anchor)
{
   static bool searchEngine = Config_getBool("SEARCHENGINE");

   if (searchEngine) {
      Doxygen::searchIndex->setCurrentDoc(title, docBaseUrl, anchor);
      static QRegExp wordPattern("[a-z_A-Z\\x80-\\xFF][a-z_A-Z0-9\\x80-\\xFF]*");
      int i, p = 0, l;

      while ((i = wordPattern.indexIn(str, p)) != -1) {
         l = wordPattern.matchedLength();

         Doxygen::searchIndex->addWord(str.mid(i, l), priority);
         p = i + l;
      }
   }
}
#endif

static struct Lang2ExtMap {
   const char *langName;
   const char *parserName;
   SrcLangExt parserId;
}

g_lang2extMap[] = {
   //  language       parser           parser option
   { "idl",          "c",             SrcLangExt_IDL      },
   { "java",         "c",             SrcLangExt_Java     },
   { "javascript",   "c",             SrcLangExt_JS       },
   { "csharp",       "c",             SrcLangExt_CSharp   },
   { "d",            "c",             SrcLangExt_D        },
   { "php",          "c",             SrcLangExt_PHP      },
   { "objective-c",  "c",             SrcLangExt_ObjC     },
   { "c",            "c",             SrcLangExt_Cpp      },
   { "c++",          "c",             SrcLangExt_Cpp      },
   { "python",       "python",        SrcLangExt_Python   },
   { "fortran",      "fortran",       SrcLangExt_Fortran  },
   { "fortranfree",  "fortranfree",   SrcLangExt_Fortran  },
   { "fortranfixed", "fortranfixed",  SrcLangExt_Fortran  },  
   { "dbusxml",      "dbusxml",       SrcLangExt_XML      },
   { "tcl",          "tcl",           SrcLangExt_Tcl      },
   { "md",           "md",            SrcLangExt_Markdown },
   { "make",         "make",          SrcLangExt_Make     },
   { 0,              0,               (SrcLangExt)0       }
};

bool updateLanguageMapping(const QByteArray &extension, const QByteArray &language)
{
   const Lang2ExtMap *p = g_lang2extMap;
   QByteArray langName = language.toLower();

   while (p->langName) {
      if (langName == p->langName) {
         break;
      }
      p++;
   }

   if (! p->langName) {
      return false;
   }

   // found the language
   SrcLangExt parserId = p->parserId;
   QByteArray extName = extension.toLower();

   if (extName.isEmpty()) {
      return false;
   }

   if (extName.at(0) != '.') {
      extName.prepend(".");
   }

   if (s_extLookup.contains(extension)) { 
      // language was already register for this ext
      s_extLookup.remove(extension);
   }
   
   s_extLookup.insert(extName, parserId);

   if (! Doxygen::parserManager->registerExtension(extName, p->parserName)) {

      msg("Unable to assign extension %-4s (%-7s) for %-7s language, currently unsupported\n",
          extName.constData(), p->parserName, language.constData());
   }

   return true;
}

void initDefaultExtensionMapping()
{
    //                  extension      parser id
   updateLanguageMapping(".dox",      "c");
   updateLanguageMapping(".txt",      "c");
   updateLanguageMapping(".doc",      "c");
   updateLanguageMapping(".c",        "c");
   updateLanguageMapping(".C",        "c");
   updateLanguageMapping(".cc",       "c");
   updateLanguageMapping(".CC",       "c");
   updateLanguageMapping(".cxx",      "c");
   updateLanguageMapping(".cpp",      "c");
   updateLanguageMapping(".c++",      "c");
   updateLanguageMapping(".ii",       "c");
   updateLanguageMapping(".ixx",      "c");
   updateLanguageMapping(".ipp",      "c");
   updateLanguageMapping(".i++",      "c");
   updateLanguageMapping(".inl",      "c");
   updateLanguageMapping(".h",        "c");
   updateLanguageMapping(".H",        "c");
   updateLanguageMapping(".hh",       "c");
   updateLanguageMapping(".HH",       "c");
   updateLanguageMapping(".hxx",      "c");
   updateLanguageMapping(".hpp",      "c");
   updateLanguageMapping(".h++",      "c");
   updateLanguageMapping(".idl",      "idl");
   updateLanguageMapping(".ddl",      "idl");
   updateLanguageMapping(".odl",      "idl");
   updateLanguageMapping(".java",     "java");
   updateLanguageMapping(".as",       "javascript");
   updateLanguageMapping(".js",       "javascript");
   updateLanguageMapping(".cs",       "csharp");
   updateLanguageMapping(".d",        "d");
   updateLanguageMapping(".php",      "php");
   updateLanguageMapping(".php4",     "php");
   updateLanguageMapping(".php5",     "php");
   updateLanguageMapping(".inc",      "php");
   updateLanguageMapping(".phtml",    "php");
   updateLanguageMapping(".m",        "objective-c");
   updateLanguageMapping(".M",        "objective-c");
   updateLanguageMapping(".mm",       "objective-c");
   updateLanguageMapping(".py",       "python");
   updateLanguageMapping(".f",        "fortran");
   updateLanguageMapping(".for",      "fortran");
   updateLanguageMapping(".f90",      "fortran");  
   updateLanguageMapping(".tcl",      "tcl");  
   updateLanguageMapping(".md",       "md");
   updateLanguageMapping(".markdown", "md");
   updateLanguageMapping(".mk",       "make");

   //updateLanguageMapping(".xml",   "dbusxml");
}

SrcLangExt getLanguageFromFileName(const QByteArray fileName)
{
   int i = fileName.lastIndexOf('.');

   if (i != -1) { 
      // name has an extension
      QByteArray extStr = fileName.right(fileName.length() - i).toLower();

      if (! extStr.isEmpty()) { 
         // non-empty extension
         auto pVal = s_extLookup.find(extStr);

         if (pVal != s_extLookup.end() ) { 
            // listed extension           
            return (SrcLangExt) * pVal;
         }
      }
   }
   
   return SrcLangExt_Cpp; // not listed => assume C-ish language.
}

MemberDef *getMemberFromSymbol(Definition *scope, FileDef *fileScope, const char *n)
{
   if (scope == 0 || (scope->definitionType() != Definition::TypeClass && 
          scope->definitionType() != Definition::TypeNamespace) ) {
      scope = Doxygen::globalScope;
   }

   QByteArray name = n;

   if (name.isEmpty()) {
      // no name was given
      return 0;   
   }

   auto di = Doxygen::symbolMap->find(name);

   if (di == Doxygen::symbolMap->end()) {
      // could not find any matching symbols
      return 0;   
   }

   // mostly copied from getResolvedClassRec()
   QByteArray explicitScopePart;
   int qualifierIndex = computeQualifiedIndex(name);

   if (qualifierIndex != -1) {
      explicitScopePart = name.left(qualifierIndex);
      replaceNamespaceAliases(explicitScopePart, explicitScopePart.length());
      name = name.mid(qualifierIndex + 2);
   }
 
   int minDistance = 10000;

   MemberDef *bestMatch = nullptr;

   QSharedPointer<QList<Definition *>> dl;  
   dl = *di;  
   
   // find the closest matching definition   
   for (auto d : *dl) { 

      if (d->definitionType() == Definition::TypeMember) {
         s_visitedNamespaces.clear();

         int distance = isAccessibleFromWithExpScope(scope, fileScope, d, explicitScopePart);

         if (distance != -1 && distance < minDistance) {
            minDistance = distance;

            MemberDef *md = dynamic_cast<MemberDef *>(d);  
            bestMatch = md;            
         }
      }
   }

   return bestMatch;
}

/*! Returns true iff the given name string appears to be a typedef in scope. */
bool checkIfTypedef(Definition *scope, FileDef *fileScope, const char *n)
{
   MemberDef *bestMatch = getMemberFromSymbol(scope, fileScope, n);

   if (bestMatch && bestMatch->isTypedef()) {
      return true;   // closest matching symbol is a typedef
   } else {
      return false;
   }
}

const char *writeUtf8Char(QTextStream &t, const char *s)
{
   char c = *s++;
   t << c;

   if (c < 0) { // multibyte character
      if (((uchar)c & 0xE0) == 0xC0) {
         t << *s++; // 11xx.xxxx: >=2 byte character
      }

      if (((uchar)c & 0xF0) == 0xE0) {
         t << *s++; // 111x.xxxx: >=3 byte character
      }

      if (((uchar)c & 0xF8) == 0xF0) {
         t << *s++; // 1111.xxxx: >=4 byte character
      }

      if (((uchar)c & 0xFC) == 0xF8) {
         t << *s++; // 1111.1xxx: >=5 byte character
      }

      if (((uchar)c & 0xFE) == 0xFC) {
         t << *s++; // 1111.1xxx: 6 byte character
      }
   }

   return s;
}

int nextUtf8CharPosition(const QByteArray &utf8Str, int len, int startPos)
{
   int bytes = 1;
   if (startPos >= len) {
      return len;
   }

   char c = utf8Str[startPos];

   if (c < 0) { // multibyte utf-8 character

      if (((uchar)c & 0xE0) == 0xC0) {
         bytes++; // 11xx.xxxx: >=2 byte character
      }

      if (((uchar)c & 0xF0) == 0xE0) {
         bytes++; // 111x.xxxx: >=3 byte character
      }

      if (((uchar)c & 0xF8) == 0xF0) {
         bytes++; // 1111.xxxx: >=4 byte character
      }

      if (((uchar)c & 0xFC) == 0xF8) {
         bytes++; // 1111.1xxx: >=5 byte character
      }

      if (((uchar)c & 0xFE) == 0xFC) {
         bytes++; // 1111.1xxx: 6 byte character
      }

   } else if (c == '&') { // skip over character entities
      static QRegExp re1("&#[0-9]+;");     // numerical entity
      static QRegExp re2("&[A-Z_a-z]+;");  // named entity

      int l1, l2;

      int i1 = re1.indexIn(utf8Str, startPos);
      int i2 = re2.indexIn(utf8Str, startPos);

      l1 = re1.matchedLength();
      l2 = re2.matchedLength();

      if (i1 != -1) {
         bytes = l1;

      } else if (i2 != -1) {
         bytes = l2;
      }
   }
   return startPos + bytes;
}

QByteArray parseCommentAsText(QSharedPointer<Definition> scope, const MemberDef *md,
                              const QByteArray &doc, const QByteArray &fileName, int lineNr)
{
   QByteArray s;

   if (doc.isEmpty()) {
      return s.data();
   }

printf("\n\n BROOM  Did we get HERE W1");

   QTextStream t(&s);
   DocNode *root = validatingParseDoc(fileName, lineNr, scope, (MemberDef *)md, doc, false, false);

printf("\n BROOM  Did we get HERE W2");


   TextDocVisitor *visitor = new TextDocVisitor(t);
   root->accept(visitor);

   delete visitor;
   delete root;

   QByteArray result = convertCharEntitiesToUTF8(s.data());
   int i = 0;
   int charCnt = 0;
   int l = result.length();

   bool addEllipsis = false;

   while ((i = nextUtf8CharPosition(result, l, i)) < l) {
      charCnt++;
      if (charCnt >= 80) {
         break;
      }
   }

   if (charCnt >= 80) { 
      // try to truncate the string

      while ((i = nextUtf8CharPosition(result, l, i)) < l && charCnt < 100) {
         charCnt++;

         if (result.at(i) >= 0 && isspace(result.at(i))) {
            addEllipsis = true;

         } else if (result.at(i) == ',' || result.at(i) == '.' || result.at(i) == '?') {
            break;
         }
      }
   }

   if (addEllipsis || charCnt == 100) {
      result = result.left(i) + "...";
   }

   return result.data();
}

static QHash<QString, void *> aliasesProcessed;

static QByteArray expandAliasRec(const QByteArray s, bool allowRecursion = false);

struct Marker {
   Marker(int p, int n, int s) : pos(p), number(n), size(s) {}
   int pos; // position in the string
   int number; // argument number
   int size; // size of the marker
};

/** For a string \a s that starts with a command name, returns the character
 *  offset within that string representing the first character after the
 *  command. For an alias with argument, this is the offset to the
 *  character just after the argument list.
 *
 *  Examples:
 *  - s=="a b"      returns 1
 *  - s=="a{2,3} b" returns 6
 *  = s=="#"        returns 0
 */
static int findEndOfCommand(const char *s)
{
   const char *p = s;
   char c;
   int i = 0;
   if (p) {
      while ((c = *p) && isId(c)) {
         p++;
      }
      if (c == '{') {
         QByteArray args = extractAliasArgs(p, 0);
         i += args.length();
      }
      i += p - s;
   }
   return i;
}

/** Replaces the markers in an alias definition \a aliasValue
 *  with the corresponding values found in the comma separated argument
 *  list \a argList and the returns the result after recursive alias expansion.
 */
static QByteArray replaceAliasArguments(const QByteArray &aliasValue, const QByteArray &argList)
{
   // first make a list of arguments from the comma separated argument list
   QList<QByteArray> args;
   
   int i, l = (int)argList.length();
   int s = 0;

   for (i = 0; i < l; i++) {
      char c = argList.at(i);

      if (c == ',' && (i == 0 || argList.at(i - 1) != '\\')) {
         args.append(argList.mid(s, i - s));
         s = i + 1; // start of next argument

      } else if (c == '@' || c == '\\') {
         // check if this is the start of another aliased command (see bug704172)
         i += findEndOfCommand(argList.data() + i + 1);
      }
   }

   if (l > s) {
      args.append(argList.right(l - s));
   }
  
   // next we look for the positions of the markers and add them to a list
   QList<Marker> markerList;
 
   l = aliasValue.length();
   int markerStart = 0;
   int markerEnd = 0;

   for (i = 0; i < l; i++) {
      if (markerStart == 0 && aliasValue.at(i) == '\\') { // start of a \xx marker
         markerStart = i + 1;

      } else if (markerStart > 0 && aliasValue.at(i) >= '0' && aliasValue.at(i) <= '9') {
         // read digit that make up the marker number
         markerEnd = i + 1;

      } else {
         if (markerStart > 0 && markerEnd > markerStart) { // end of marker
            int markerLen = markerEnd - markerStart;

            // include backslash
            markerList.append(Marker(markerStart - 1, atoi(aliasValue.mid(markerStart, markerLen)), markerLen + 1));            
         }

         markerStart = 0; // outside marker
         markerEnd = 0;
      }
   }

   if (markerStart > 0) {
      markerEnd = l;
   }

   if (markerStart > 0 && markerEnd > markerStart) {
      int markerLen = markerEnd - markerStart;

      // include backslash
      markerList.append(Marker(markerStart - 1, atoi(aliasValue.mid(markerStart, markerLen)), markerLen + 1));
      
   }

   // then we replace the markers with the corresponding arguments in one pass
   QByteArray result;
   int p = 0;

   for (auto m : markerList) {   
      result += aliasValue.mid(p, m.pos - p);
     
      if (m.number > 0 && m.number <= args.count()) { 
         // valid number
         result += expandAliasRec(args.at(m.number - 1), true);         
      }

      p = m.pos + m.size; // continue after the marker
   }

   result += aliasValue.right(l - p); // append remainder

   // expand the result again
   result = substitute(result, "\\{", "{");
   result = substitute(result, "\\}", "}");
   result = expandAliasRec(substitute(result, "\\,", ","));

   return result;
}

static QByteArray escapeCommas(const QByteArray &s)
{
   QByteArray retval = s;
   retval.replace(",", "\\,");
    
   return retval;
}

static QByteArray expandAliasRec(const QByteArray s, bool allowRecursion)
{
   QByteArray result;
   static QRegExp cmdPat("[\\\\@][a-z_A-Z][a-z_A-Z0-9]*");

   QByteArray value = s;

   int i;
   int len;
   int p = 0;

   while ((i = cmdPat.indexIn(value, p)) != -1) {
      len = cmdPat.matchedLength();
      result += value.mid(p, i - p);

      QByteArray args = extractAliasArgs(value, i + len);

      bool hasArgs = ! args.isEmpty();            // found directly after command
      int argsLen  =   args.length();
    
      QString cmd = value.mid(i + 1, len - 1);
      QString cmdNoArgs = cmd;

      int numArgs = 0;

      if (hasArgs) {
         numArgs = countAliasArguments(args);

         cmd = cmd + QString("{%1}").arg(numArgs);    // alias name + {n}
      }

      QByteArray aliasText = Doxygen::aliasDict.value(cmd);


      if (numArgs > 1 &&  aliasText.isEmpty()) {
         // in case there is no command with numArgs parameters, but there is a command with 1 parameter
         // we also accept all text as the argument of that command (so you do not have to escape commas)

         aliasText = Doxygen::aliasDict.value(cmdNoArgs + "{1}");

         if (! aliasText.isEmpty()) {
            cmd = cmdNoArgs + "{1}";
            args = escapeCommas(args);    // everything is seen as one argument
         }
      }
     
      if ((allowRecursion || ! aliasesProcessed.contains(cmd)) && ! aliasText.isEmpty()) { 
         // expand the alias
         
         if (! allowRecursion) {
            aliasesProcessed.insert(cmd, (void *)0x8);
         }

         QByteArray val = aliasText;
         if (hasArgs) {
            val = replaceAliasArguments(val, args);            
         }

         result += expandAliasRec(val);
         if (!allowRecursion) {
            aliasesProcessed.remove(cmd);
         }

         p = i + len;
         if (hasArgs) {
            p += argsLen + 2;
         }

      } else { 
         // command is not an alias        
         result += value.mid(i, len);
         p = i + len;
      }
   }

   result += value.right(value.length() - p);
 
   return result;
}

int countAliasArguments(const QByteArray argList)
{
   int count = 1;
   int len   = argList.length();
   
   for (int k = 0; k < len; k++) {
      char c = argList.at(k);

      if (c == ',' && (k == 0 || argList.at(k - 1) != '\\')) {
         count++;

      } else if (c == '@' || c == '\\') {
         // check if this is the start of another aliased command (see bug704172)
         k += findEndOfCommand(argList.data() + k + 1);
      }
   }
   
   return count;
}

QByteArray extractAliasArgs(const QByteArray &args, int pos)
{
   int i;
   int bc = 0;

   char prevChar = 0;

   if (args.length() > pos && args.at(pos) == '{') { 

      // alias has argument
      for (i = pos; i < args.length(); i++) {

         if (prevChar != '\\') {
            if (args.at(i) == '{') {
               bc++;
            }

            if (args.at(i) == '}') {
               bc--;
            }

            prevChar = args.at(i);

         } else {
            prevChar = 0;

         }

         if (bc == 0) {           
            return args.mid(pos + 1, i - pos - 1);
         }
      }
   }

   return "";
}

QByteArray resolveAliasCmd(const QByteArray aliasCmd)
{
   QByteArray result;

   aliasesProcessed.clear(); 
   result = expandAliasRec(aliasCmd);
   
   return result;
}

QByteArray expandAlias(const QByteArray &aliasName, const QByteArray &aliasValue)
{
   QByteArray result;
   aliasesProcessed.clear();

   // avoid expanding this command recursively
   aliasesProcessed.insert(aliasName, (void *)0x8);

   // expand embedded commands   
   result = expandAliasRec(aliasValue);
   
   return result;
}

void writeTypeConstraints(OutputList &ol, Definition *d, ArgumentList *al)
{
   if (al == 0) {
      return;
   }

   ol.startConstraintList(theTranslator->trTypeConstraints());
  
   for (auto a : *al) {
      ol.startConstraintParam();
      ol.parseText(a.name);
      ol.endConstraintParam();
      ol.startConstraintType();
      linkifyText(TextGeneratorOLImpl(ol), d, 0, 0, a.type);
      ol.endConstraintType();
      ol.startConstraintDocs();
      ol.generateDoc(d->docFile(), d->docLine(), d, 0, a.docs, true, false);
      ol.endConstraintDocs();
   }

   ol.endConstraintList();
}

void stackTrace()
{
#ifdef TRACINGSUPPORT
   void *backtraceFrames[128];
   int frameCount = backtrace(backtraceFrames, 128);
   static char cmd[40960];

   char *p = cmd;
   p += sprintf(p, "/usr/bin/atos -p %d ", (int)getpid());

   for (int x = 0; x < frameCount; x++) {
      p += sprintf(p, "%p ", backtraceFrames[x]);
   }

   fprintf(stderr, "========== STACKTRACE START ==============\n");
   if (FILE *fp = popen(cmd, "r")) {
      char resBuf[512];

      while (size_t len = fread(resBuf, 1, sizeof(resBuf), fp)) {
         fwrite(resBuf, 1, len, stderr);
      }

      pclose(fp);
   }

   fprintf(stderr, "============ STACKTRACE END ==============\n");
   
#endif
}

static int transcodeCharacterBuffer(const char *fileName, BufStr &srcBuf, int size,
                                    const char *inputEncoding, const char *outputEncoding)
{
   if (inputEncoding == 0 || outputEncoding == 0) {
      return size;
   }

   if (qstricmp(inputEncoding, outputEncoding) == 0) {
      return size;
   }

   QTextCodec *inCodec  = QTextCodec::codecForName(inputEncoding);
   QTextCodec *outCodec = QTextCodec::codecForName(outputEncoding);
  
   if (! inCodec || ! outCodec) {
      err("Unsupported character conversion: '%s'->'%s': %s\n"
          "Check the INPUT_ENCODING setting in the config file\n",
          inputEncoding, outputEncoding, strerror(errno));

      exit(1);
   }

   QString temp = inCodec->toUnicode(srcBuf.data());
   QByteArray output = outCodec->fromUnicode(temp);
      
   uint newSize = output.size();

   srcBuf.shrink(newSize);
   strncpy(srcBuf.data(), output.constData(), newSize);
   
   return newSize;
}

//! read a file name \a fileName and optionally filter and transcode it
bool readInputFile(const char *fileName, BufStr &inBuf, bool filter, bool isSourceCode)
{
   // try to open file
   int size = 0;
  
   QFileInfo fi(fileName);
   if (! fi.exists()) {
      return false;
   }
 
   QString filterName = getFileFilter(fileName, isSourceCode);

   if (filterName.isEmpty() || ! filter) {
      QFile f(fileName);

      if (! f.open(QIODevice::ReadOnly)) {
         err("Could not open file %s\n", fileName);
         return false;
      }

      size = fi.size();

      // read the file
      inBuf.skip(size);

      if (f.read(inBuf.data(), size) != size) {
         err("Problem while reading file %s\n", fileName);
         return false;
      }

   } else {
      QString cmd = filterName + " \"" + fileName + "\"";
      Debug::print(Debug::ExtCmd, 0, "Executing popen(`%s`)\n", qPrintable(cmd));

      FILE *f = popen(qPrintable(cmd), "r");

      if (!f) {
         err("Could not execute filter %s\n", filterName.data());
         return false;
      }

      const int bufSize = 1024;
      char buf[bufSize];
      int numRead;

      while ((numRead = (int)fread(buf, 1, bufSize, f)) > 0) {         
         inBuf.addArray(buf, numRead), size += numRead;
      }

      pclose(f);

      inBuf.at(inBuf.curPos()) = '\0';
      Debug::print(Debug::FilterOutput, 0, "Filter output\n");
      Debug::print(Debug::FilterOutput, 0, "-------------\n%s\n-------------\n", inBuf.data());
   }

   int start = 0;
   if (size >= 2 && ((inBuf.at(0) == -1 && inBuf.at(1) == -2) ||  (inBuf.at(0) == -2 && inBuf.at(1) == -1) )) { 
      // UCS-2 encoded file
      transcodeCharacterBuffer(fileName, inBuf, inBuf.curPos(), "UCS-2", "UTF-8");

   } else if (size >= 3 && (uchar)inBuf.at(0) == 0xEF && (uchar)inBuf.at(1) == 0xBB &&
              (uchar)inBuf.at(2) == 0xBF) { 

      // UTF-8 encoded file
      inBuf.dropFromStart(3); // remove UTF-8 BOM: no translation needed

   } else { 
      // transcode according to the INPUT_ENCODING setting
      // do character transcoding if needed.

      transcodeCharacterBuffer(fileName, inBuf, inBuf.curPos(), Config_getString("INPUT_ENCODING"), "UTF-8");

   }

   //inBuf.addChar('\n'); /* to prevent problems under Windows ? */

   // and translate CR's
   size = inBuf.curPos() - start;
   int newSize = filterCRLF(inBuf.data() + start, size);
  
   if (newSize != size) { // we removed chars
      inBuf.shrink(newSize); // resize the array      
   }

   inBuf.addChar(0);
   return true;
}

// Replace %word by word in title
QByteArray filterTitle(const QByteArray &title)
{
   QByteArray tf;

   static QRegExp re("%[A-Z_a-z]");
   int p = 0, i, l;

   while ((i = re.indexIn(title, p)) != -1) {
      l = re.matchedLength();

      tf += title.mid(p, i - p);
      tf += title.mid(i + 1, l - 1); // skip %

      p = i + l;
   }

   tf += title.right(title.length() - p);

   return tf;
}

// returns true if the name of the file represented by `fi' matches
// one of the file patterns in the `patList' list.

bool patternMatch(const QFileInfo &fi, const QStringList *patList)
{
   bool found = false;

   if (patList) {
     
      QString fn  = fi.fileName();
      QString fp  = fi.filePath();
      QString afp = fi.absoluteFilePath();
    
      for ( auto pattern : *patList ) {

         if (! pattern.isEmpty()) {
            int i = pattern.indexOf('=');

            if (i != -1) {
               pattern = pattern.left(i);   // strip of the extension specific filter name
            }

#if defined(_WIN32) || defined(__MACOSX__) 
            // Windows or MacOSX
            QRegExp re(pattern, Qt::CaseInsensitive, QRegExp::Wildcard);  
#else       
            // unix
            QRegExp re(pattern, Qt::CaseSensitive, QRegExp::Wildcard);    
#endif

            found = re.indexIn(fn) != -1 || re.indexIn(fp) != -1 || re.indexIn(afp) != -1;

            if (found) {
               break;
            }            
         }
      }
   }

   return found;
}

QByteArray externalLinkTarget()
{
   static bool extLinksInWindow = Config_getBool("EXT_LINKS_IN_WINDOW");

   if (extLinksInWindow) {
      return "target=\"_blank\" ";
   } else {
      return "";
   }
}

QByteArray externalRef(const QByteArray &relPath, const QByteArray &ref, bool href)
{
   QByteArray result;

   if (!ref.isEmpty()) {
      QByteArray dest = Doxygen::tagDestinationDict[ref];

      if (! dest.isEmpty()) {
         result = dest;
         int l  = result.length();

         if (! relPath.isEmpty() && l > 0 && result.at(0) == '.') {
            // relative path -> prepend relPath.
            result.prepend(relPath);
         }

         if (!href) {
            result.prepend("doxygen=\"" + ref + ":");
         }

         if (l > 0 && result.at(l - 1) != '/') {
            result += '/';
         }

         if (!href) {
            result.append("\" ");
         }
      }

   } else {
      result = relPath;

   }
   return result;
}

/** Writes the intensity only bitmap representated by \a data as an image to
 *  directory \a dir using the colors defined by HTML_COLORSTYLE_*.
 */
void writeColoredImgData(ColoredImgDataItem data)
{
   static int hue   = Config_getInt("HTML_COLORSTYLE_HUE");
   static int sat   = Config_getInt("HTML_COLORSTYLE_SAT");
   static int gamma = Config_getInt("HTML_COLORSTYLE_GAMMA");
  
   QString fileName = data.path + "/" + data.name;

   QFile f(fileName);

   if (f.open(QIODevice::WriteOnly)) {

      ColoredImage image(data.width, data.height, data.content, data.alpha, sat, hue, gamma);  
      QByteArray buffer = image.convert();
 
      if (f.write(buffer) == -1) {
         fprintf(stderr, "Error occureced while writing file %s, %d\n", qPrintable(fileName), f.error());
      }

      f.close();

   } else {
      fprintf(stderr, "Warning: Unable to save image file %s, %d\n", qPrintable(fileName), f.error());

   }

   const char *temp = strdup(qPrintable(data.name));
   Doxygen::indexList->addImageFile(temp);
   
}

/** Replaces any markers of the form \#\#AA in input string \a str
 *  by new markers of the form \#AABBCC, where \#AABBCC represents a
 *  valid color, based on the intensity represented by hex number AA
 *  and the current HTML_COLORSTYLE_* settings.
 */
QByteArray replaceColorMarkers(const char *str)
{
   QByteArray result;
   QByteArray s = str;

   if (s.isEmpty()) {
      return result;
   }

   static QRegExp re("##[0-9A-Fa-f][0-9A-Fa-f]");
   static const char hex[] = "0123456789ABCDEF";
   static int hue   = Config_getInt("HTML_COLORSTYLE_HUE");
   static int sat   = Config_getInt("HTML_COLORSTYLE_SAT");
   static int gamma = Config_getInt("HTML_COLORSTYLE_GAMMA");
   int i, l, sl = s.length(), p = 0;

   while ((i = re.indexIn(s, p)) != -1) {
      l = re.matchedLength();

      result += s.mid(p, i - p);
      QByteArray lumStr = s.mid(i + 2, l - 2);

      double r, g, b;
      int red, green, blue;
      int level = HEXTONUM(lumStr[0]) * 16 + HEXTONUM(lumStr[1]);
      ColoredImage::hsl2rgb(hue / 360.0, sat / 255.0, pow(level / 255.0, gamma / 100.0), &r, &g, &b);

      red   = (int)(r * 255.0);
      green = (int)(g * 255.0);
      blue  = (int)(b * 255.0);
      char colStr[8];
      colStr[0] = '#';
      colStr[1] = hex[red >> 4];
      colStr[2] = hex[red & 0xf];
      colStr[3] = hex[green >> 4];
      colStr[4] = hex[green & 0xf];
      colStr[5] = hex[blue >> 4];
      colStr[6] = hex[blue & 0xf];
      colStr[7] = 0;
      //printf("replacing %s->%s (level=%d)\n",lumStr.data(),colStr,level);
      result += colStr;
      p = i + l;
   }
   result += s.right(sl - p);
   return result;
}

/** Copies the contents of file with name \a src to the newly created
 *  file with name \a dest. Returns true if successful.
 */
bool copyFile(const QByteArray &src, const QByteArray &dest)
{
   QFile sf(src);
   if (sf.open(QIODevice::ReadOnly)) {
      QFileInfo fi(src);
      QFile df(dest);

      if (df.open(QIODevice::WriteOnly)) {
         char *buffer = new char[fi.size()];
         sf.read(buffer, fi.size());

         df.write(buffer, fi.size());
         df.flush();

         delete[] buffer;

      } else {
         err("could not write to file %s\n", dest.data());
         return false;
      }
   } else {
      err("could not open user specified file %s\n", src.data());
      return false;
   }
   return true;
}

/** Returns the section of text, in between a pair of markers.
 *  Full lines are returned, excluding the lines on which the markers appear.
 */
QByteArray extractBlock(const QByteArray text, const QByteArray marker)
{
   QByteArray result;
   int p = 0, i;
   bool found = false;

   // find the character positions of the markers
   int m1 = text.indexOf(marker);
   if (m1 == -1) {
      return result;
   }
   int m2 = text.indexOf(marker, m1 + marker.length());
   if (m2 == -1) {
      return result;
   }

   // find start and end line positions for the markers
   int l1 = -1, l2 = -1;

   while (! found && (i = text.indexOf('\n', p)) != -1) {
      found = (p <= m1 && m1 < i); // found the line with the start marker
      p = i + 1;
   }

   l1 = p;
   int lp = i;

   if (found) {
      while ((i = text.indexOf('\n', p)) != -1) {
         if (p <= m2 && m2 < i) { // found the line with the end marker
            l2 = p;
            break;
         }

         p = i + 1;
         lp = i;
      }
   }

   if (l2 == -1) { // marker at last line without newline (see bug706874)
      l2 = lp;
   }
   
   return l2 > l1 ? text.mid(l1, l2 - l1) : QByteArray();
}

/** Returns a string representation of \a lang. */
QByteArray langToString(SrcLangExt lang)
{
   switch (lang) {
      case SrcLangExt_Unknown:
         return "Unknown";

      case SrcLangExt_IDL:
         return "IDL";

      case SrcLangExt_Java:
         return "Java";

      case SrcLangExt_CSharp:
         return "C#";

      case SrcLangExt_D:
         return "D";

      case SrcLangExt_PHP:
         return "PHP";

      case SrcLangExt_ObjC:
         return "Objective-C";

      case SrcLangExt_Cpp:
         return "C++";

      case SrcLangExt_JS:
         return "Javascript";

      case SrcLangExt_Python:
         return "Python";

      case SrcLangExt_Fortran:
         return "Fortran";

      case SrcLangExt_XML:
         return "XML";

      case SrcLangExt_Tcl:
         return "Tcl";

      case SrcLangExt_Markdown:
         return "Markdown";
   }
   return "Unknown";
}

/** Returns the scope separator to use given the programming language \a lang */
QByteArray getLanguageSpecificSeparator(SrcLangExt lang, bool classScope)
{
   if (lang == SrcLangExt_Java || lang == SrcLangExt_CSharp || lang == SrcLangExt_Python) {
      return ".";

   } else if (lang == SrcLangExt_PHP && !classScope) {
      return "\\";

   } else {
      return "::";

   }
}

/** Corrects URL \a url according to the relative path \a relPath.
 *  Returns the corrected URL. For absolute URLs no correction will be done.
 */
QByteArray correctURL(const QByteArray &url, const QByteArray &relPath)
{
   QByteArray result = url;
   if (!relPath.isEmpty() &&
         url.left(5) != "http:" && url.left(6) != "https:" &&
         url.left(4) != "ftp:"  && url.left(5) != "file:") {
      result.prepend(relPath);
   }
   return result;
}

bool protectionLevelVisible(Protection prot)
{
   static bool extractPrivate = Config_getBool("EXTRACT_PRIVATE");
   static bool extractPackage = Config_getBool("EXTRACT_PACKAGE");

   return (prot != Private && prot != Package)  || (prot == Private && extractPrivate) || (prot == Package && extractPackage);
}

QByteArray stripIndentation(const QByteArray &s)
{
   if (s.isEmpty()) {
      return s;   // empty string -> we're done
   }

   //printf("stripIndentation:\n%s\n------\n",s.data());
   // compute minimum indentation over all lines
   const char *p = s.data();
   char c;
   int indent = 0;
   int minIndent = 1000000; // "infinite"
   bool searchIndent = true;
   static int tabSize = Config_getInt("TAB_SIZE");
   while ((c = *p++)) {
      if      (c == '\t') {
         indent += tabSize - (indent % tabSize);
      } else if (c == '\n') {
         indent = 0, searchIndent = true;
      } else if (c == ' ') {
         indent++;
      } else if (searchIndent) {
         searchIndent = false;
         if (indent < minIndent) {
            minIndent = indent;
         }
      }
   }

   // no indent to remove -> we're done
   if (minIndent == 0) {
      return s;
   }

   // remove minimum indentation for each line
   QByteArray result;
   p = s.data();
   indent = 0;
   while ((c = *p++)) {
      if (c == '\n') { // start of new line
         indent = 0;
         result += c;
      } else if (indent < minIndent) { // skip until we reach minIndent
         if (c == '\t') {
            int newIndent = indent + tabSize - (indent % tabSize);
            int i = newIndent;
            while (i > minIndent) { // if a tab crosses the minIndent boundary fill the rest with spaces
               result += ' ';
               i--;
            }
            indent = newIndent;
         } else { // space
            indent++;
         }
      } else { // copy anything until the end of the line
         result += c;
      }
   }

   result += '\0';
   return result.data();
}

bool fileVisibleInIndex(FileDef *fd, bool &genSourceFile)
{
   static bool allExternals = Config_getBool("ALLEXTERNALS");
   bool isDocFile = fd->isDocumentationFile();
   genSourceFile = !isDocFile && fd->generateSourceFile();

   return ( ((allExternals && fd->isLinkable()) || fd->isLinkableInProject()) && !isDocFile);
}

void addDocCrossReference(QSharedPointer<MemberDef> src, QSharedPointer<MemberDef> dst)
{
   static bool referencedByRelation = Config_getBool("REFERENCED_BY_RELATION");
   static bool referencesRelation   = Config_getBool("REFERENCES_RELATION");
   static bool callerGraph          = Config_getBool("CALLER_GRAPH");
   static bool callGraph            = Config_getBool("CALL_GRAPH");
   
   if (dst->isTypedef() || dst->isEnumerate()) {
      return;   // don't add types
   }

   if ((referencedByRelation || callerGraph || dst->hasCallerGraph()) && src->showInCallGraph() ) {
      dst->addSourceReferencedBy(src);
      MemberDef *mdDef = dst->memberDefinition();

      if (mdDef) {
         mdDef->addSourceReferencedBy(src);
      }

      MemberDef *mdDecl = dst->memberDeclaration();
      if (mdDecl) {
         mdDecl->addSourceReferencedBy(src);
      }
   }

   if ((referencesRelation || callGraph || src->hasCallGraph()) && src->showInCallGraph()) {
      src->addSourceReferences(dst);
      MemberDef *mdDef = src->memberDefinition();

      if (mdDef) {
         mdDef->addSourceReferences(dst);
      }

      MemberDef *mdDecl = src->memberDeclaration();

      if (mdDecl) {
         mdDecl->addSourceReferences(dst);
      }
   }
}

/*! @brief Get one unicode character as an unsigned integer from utf-8 string
 *
 * @param s utf-8 encoded string
 * @param idx byte position of given string \a s.
 * @return the unicode codepoint, 0 - MAX_UNICODE_CODEPOINT
 * @see getNextUtf8OrToLower()
 * @see getNextUtf8OrToUpper()
 */
uint getUtf8Code( const QByteArray &s, int idx )
{
   const int length = s.length();
   if (idx >= length) {
      return 0;
   }
  
   const uint c0 = (uchar)s.at(idx);
   if ( c0 < 0xC2 || c0 >= 0xF8 ) { // 1 byte character
      return c0;
   }
  
   if (idx + 1 >= length) {
      return 0;
   }
  
   const uint c1 = ((uchar)s.at(idx + 1)) & 0x3f;
   if ( c0 < 0xE0 ) { // 2 byte character
      return ((c0 & 0x1f) << 6) | c1;
   }
   if (idx + 2 >= length) {
      return 0;
   }

   const uint c2 = ((uchar)s.at(idx + 2)) & 0x3f;
   if ( c0 < 0xF0 ) { // 3 byte character
      return ((c0 & 0x0f) << 12) | (c1 << 6) | c2;
   }
   if (idx + 3 >= length) {
      return 0;
   }

   // 4 byte character
   const uint c3 = ((uchar)s.at(idx + 3)) & 0x3f;
   return ((c0 & 0x07) << 18) | (c1 << 12) | (c2 << 6) | c3;
}


/*! @brief Returns one unicode character as an unsigned integer
 *  from utf-8 string, making the character lower case if it was upper case.
 *
 * @param s utf-8 encoded string
 * @param idx byte position of given string \a s.
 * @return the unicode codepoint, 0 - MAX_UNICODE_CODEPOINT, excludes 'A'-'Z'
 * @see getNextUtf8Code()
*/
uint getUtf8CodeToLower( const QByteArray &s, int idx )
{
   const uint v = getUtf8Code( s, idx );
   return v < 0x7f ? tolower( v ) : v;
}


/*! @brief Returns one unicode character as ian unsigned interger
 *  from utf-8 string, making the character upper case if it was lower case.
 *
 * @param s utf-8 encoded string
 * @param idx byte position of given string \a s.
 * @return the unicode codepoint, 0 - MAX_UNICODE_CODEPOINT, excludes 'A'-'Z'
 * @see getNextUtf8Code()
 */
uint getUtf8CodeToUpper( const QByteArray &s, int idx )
{
   const uint v = getUtf8Code( s, idx );
   return v < 0x7f ? toupper( v ) : v;
}

bool namespaceHasVisibleChild(NamespaceDef *nd, bool includeClasses)
{
   if (nd->getNamespaceSDict()) {
     
      for (auto cnd : *nd->getNamespaceSDict()) {   
         if (cnd->isLinkableInProject() && cnd->localName().indexOf('@') == -1) {
            return true;

         } else if (namespaceHasVisibleChild(cnd.data(), includeClasses)) {
            return true;
         }
      }
   }

   if (includeClasses && nd->getClassSDict()) {     
      for (auto cd : *nd->getClassSDict()) {  
         if (cd->isLinkableInProject() && cd->templateMaster() == 0) {
            return true;
         }
      }
   }

   return false;
}

bool classVisibleInIndex(ClassDef *cd)
{
   static bool allExternals = Config_getBool("ALLEXTERNALS");
   return (allExternals && cd->isLinkable()) || cd->isLinkableInProject();
}

QByteArray extractDirection(QString docs)
{
   QRegExp re("\\[[^\\]]+\\]"); 
   int len = 0;

   if (re.indexIn(docs, 0) == 0) {
      len = re.matchedLength();
     
      int  inPos  = docs.indexOf("in", 1, Qt::CaseInsensitive);
      int outPos  = docs.indexOf("out", 1, Qt::CaseInsensitive);

      bool input  = inPos != -1 &&  inPos < len;
      bool output = outPos != -1 && outPos < len;

      if (input || output) {
         // in,out attributes
         docs = docs.mid(len); // strip attributes

         if (input && output) {
            return "[in,out]";

         } else if (input) {
            return "[in]";

         } else if (output) {
            return "[out]";
         }
      }
   }
   return QByteArray();
}

/** Computes for a given list type \a inListType, which are the
 *  the corresponding list type(s) in the base class that are to be
 *  added to this list.
 *
 *  So for public inheritance, the mapping is 1-1, so outListType1=inListType
 *  Private members are to be hidden completely.
 *
 *  For protected inheritance, both protected and public members of the
 *  base class should be joined in the protected member section.
 *
 *  For private inheritance, both protected and public members of the
 *  base class should be joined in the private member section.
 */
void convertProtectionLevel(MemberListType inListType, Protection inProt, int *outListType1, int *outListType2)
{
   static bool extractPrivate = Config_getBool("EXTRACT_PRIVATE");
   // default representing 1-1 mapping
   *outListType1 = inListType;
   *outListType2 = -1;

   if (inProt == Public) {
      switch (inListType) // in the private section of the derived class,
         // the private section of the base class should not
         // be visible
      {
         case MemberListType_priMethods:
         case MemberListType_priStaticMethods:
         case MemberListType_priSlots:
         case MemberListType_priAttribs:
         case MemberListType_priStaticAttribs:
         case MemberListType_priTypes:
            *outListType1 = -1;
            *outListType2 = -1;
            break;
         default:
            break;
      }
   } else if (inProt == Protected) { // Protected inheritance
      switch (inListType) // in the protected section of the derived class,
         // both the public and protected members are shown
         // as protected
      {
         case MemberListType_pubMethods:
         case MemberListType_pubStaticMethods:
         case MemberListType_pubSlots:
         case MemberListType_pubAttribs:
         case MemberListType_pubStaticAttribs:
         case MemberListType_pubTypes:
         case MemberListType_priMethods:
         case MemberListType_priStaticMethods:
         case MemberListType_priSlots:
         case MemberListType_priAttribs:
         case MemberListType_priStaticAttribs:
         case MemberListType_priTypes:
            *outListType1 = -1;
            *outListType2 = -1;
            break;

         case MemberListType_proMethods:
            *outListType2 = MemberListType_pubMethods;
            break;
         case MemberListType_proStaticMethods:
            *outListType2 = MemberListType_pubStaticMethods;
            break;
         case MemberListType_proSlots:
            *outListType2 = MemberListType_pubSlots;
            break;
         case MemberListType_proAttribs:
            *outListType2 = MemberListType_pubAttribs;
            break;
         case MemberListType_proStaticAttribs:
            *outListType2 = MemberListType_pubStaticAttribs;
            break;
         case MemberListType_proTypes:
            *outListType2 = MemberListType_pubTypes;
            break;
         default:
            break;
      }
   } else if (inProt == Private) {
      switch (inListType) // in the private section of the derived class,
         // both the public and protected members are shown
         // as private
      {
         case MemberListType_pubMethods:
         case MemberListType_pubStaticMethods:
         case MemberListType_pubSlots:
         case MemberListType_pubAttribs:
         case MemberListType_pubStaticAttribs:
         case MemberListType_pubTypes:
         case MemberListType_proMethods:
         case MemberListType_proStaticMethods:
         case MemberListType_proSlots:
         case MemberListType_proAttribs:
         case MemberListType_proStaticAttribs:
         case MemberListType_proTypes:
            *outListType1 = -1;
            *outListType2 = -1;
            break;

         case MemberListType_priMethods:
            if (extractPrivate) {
               *outListType1 = MemberListType_pubMethods;
               *outListType2 = MemberListType_proMethods;
            } else {
               *outListType1 = -1;
               *outListType2 = -1;
            }
            break;
         case MemberListType_priStaticMethods:
            if (extractPrivate) {
               *outListType1 = MemberListType_pubStaticMethods;
               *outListType2 = MemberListType_proStaticMethods;
            } else {
               *outListType1 = -1;
               *outListType2 = -1;
            }
            break;
         case MemberListType_priSlots:
            if (extractPrivate) {
               *outListType1 = MemberListType_pubSlots;
               *outListType1 = MemberListType_proSlots;
            } else {
               *outListType1 = -1;
               *outListType2 = -1;
            }
            break;
         case MemberListType_priAttribs:
            if (extractPrivate) {
               *outListType1 = MemberListType_pubAttribs;
               *outListType2 = MemberListType_proAttribs;
            } else {
               *outListType1 = -1;
               *outListType2 = -1;
            }
            break;
         case MemberListType_priStaticAttribs:
            if (extractPrivate) {
               *outListType1 = MemberListType_pubStaticAttribs;
               *outListType2 = MemberListType_proStaticAttribs;
            } else {
               *outListType1 = -1;
               *outListType2 = -1;
            }
            break;
         case MemberListType_priTypes:
            if (extractPrivate) {
               *outListType1 = MemberListType_pubTypes;
               *outListType2 = MemberListType_proTypes;
            } else {
               *outListType1 = -1;
               *outListType2 = -1;
            }
            break;
         default:
            break;
      }
   }  
}

bool mainPageHasTitle()
{
   if (Doxygen::mainPage == 0) {
      return false;
   }
  
   if (Doxygen::mainPage->title().isEmpty()) {
      return false;
   }

   if (Doxygen::mainPage->title().toLower() == "notitle") {
      return false;
   }

   return true;
}

QString stripPrefix(QString input, const char *prefix) 
{
   QString retval = input;

   if (input.startsWith(prefix)) {
      retval = retval.remove(0, strlen(prefix));
   }

   return retval;
}

QByteArray stripPrefix(QByteArray input, const char *prefix) 
{
   QByteArray retval = input;

   if (input.startsWith(prefix)) {
      retval = retval.remove(0, strlen(prefix));
   }

   return retval;
}
