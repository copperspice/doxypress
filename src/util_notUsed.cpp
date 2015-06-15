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

#if 0

static void printArgList(ArgumentList *al)
{
  if (al==0) return;
  ArgumentListIterator ali(*al);
  Argument *a;
  printf("(");
  for (;(a=ali.current());++ali)
  {
    printf("t=`%s' n=`%s' v=`%s' ",a->type.data(),!a->name.isEmpty()>0?a->name.data():"",!a->defval.isEmpty()>0?a->defval.data():"");
  }
  printf(")");
}


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
            if (!fullScope.isEmpty() && Doxy_Globals::namespaceSDict[fullScope] != 0) { // scope is a namespace
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
            if (!fullScope.isEmpty() && Doxy_Globals::namespaceSDict[fullScope] != 0) { // scope is a namespace
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

static QByteArray resolveSymbolName(QSharedPointer<FileDef> fs, QSharedPointer<Definition> symbol, QByteArray &templSpec)
{
   assert(symbol != 0);
   if (symbol->definitionType() == Definition::TypeMember && ((MemberDef *)symbol)->isTypedef())  {
      // if symbol is a typedef then try to resolve it
   
      QSharedPointer<MemberDef> md;
      QSharedPointer<ClassDef> cd = newResolveTypedef(fs, (MemberDef *)symbol, &md, &templSpec);

      if (cd) {
         return cd->qualifiedName() + templSpec;

      } else if (md) {
         return md->qualifiedName();
      }
   }

   return symbol->qualifiedName();
}

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

bool generateRef(OutputDocInterface &od, const char *scName, const char *name, bool inSeeBlock, const char *rt)
{ 
   QSharedPointer<Definition> compound;
   QSharedPointer<MemberDef> md = QSharedPointer<MemberDef>(); 

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

void stringToSearchIndex(const QByteArray &docBaseUrl, const QByteArray &title,
                         const QByteArray &str, bool priority, const QByteArray &anchor)
{
   static bool searchEngine = Config::getBool("html-search")

   if (searchEngine) {
      Doxy_Globals::searchIndex->setCurrentDoc(title, docBaseUrl, anchor);
      static QRegExp wordPattern("[a-z_A-Z\\x80-\\xFF][a-z_A-Z0-9\\x80-\\xFF]*");
      int i, p = 0, l;

      while ((i = wordPattern.indexIn(str, p)) != -1) {
         l = wordPattern.matchedLength();

         Doxy_Globals::searchIndex->addWord(str.mid(i, l), priority);
         p = i + l;
      }
   }
}

/*!
 * Matches the arguments list srcAl with the argument list dstAl
 * Returns true if the argument lists are equal. Two argument list are
 * considered equal if the number of arguments is equal and the types of all
 * arguments are equal. Furthermore the const and volatile specifiers
 * stored in the list should be equal.
 */

bool matchArguments(ArgumentList *srcAl, ArgumentList *dstAl, const QByteArray &cl, const QByteArray &ns, bool checkCV,
                    NamespaceSDict *usingNamespaces, StringMap<QSharedPointer<Definition>> *usingClasses)
{
   QByteArray className     = cl;
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
