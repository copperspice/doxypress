/************************************************************************
*
* Copyright (C) 2014-2019 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
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

#include <QRegularExpression>

#include <stdio.h>
#include <assert.h>

#include <memberdef.h>

#include <arguments.h>
#include <code_cstyle.h>
#include <config.h>
#include <default_args.h>
#include <docparser.h>
#include <dot.h>
#include <doxy_globals.h>
#include <example.h>
#include <language.h>
#include <membergroup.h>
#include <message.h>
#include <outputlist.h>
#include <parse_base.h>
#include <objcache.h>
#include <util.h>

int MemberDef::s_indentLevel = 0;

static QString addTemplateNames(const QString &s, const QString  &n, const QString  &t)
{
   QString result;
   QString clRealName = n;

   int p = 0;
   int i;

   if ((i = clRealName.indexOf('<')) != -1) {
      // strip template specialization
      clRealName = clRealName.left(i);
   }

   if ((i = clRealName.lastIndexOf("::")) != -1) {
      clRealName = clRealName.right(clRealName.length() - i - 2);
   }

   while ((i = s.indexOf(clRealName, p)) != -1) {
      result += s.mid(p, i - p);
      uint j = clRealName.length() + i;

      if (s.length() == j || (s.at(j) != '<' && ! isId(s.at(j)) )) {
         result += clRealName + t;
      } else {
         result += clRealName;
      }

      p = i + clRealName.length();
   }

   result += s.right(s.length() - p);

   return result;
}

static bool writeDefArgumentList(OutputList &ol, QSharedPointer<Definition> scopeDef, QSharedPointer<MemberDef> md)
{
   const ArgumentList *tmp = nullptr;      // ok as a ptr

   if (md->isDocsForDefinition())  {
      tmp = &md->getArgumentList();

   } else {
      tmp = &md->getDeclArgumentList();

   }

   const ArgumentList &defArgList = *tmp;

   if (md->isSignal() || md->isSlot() || md->isFunction() || md->isFunctionPtr()) {
     // keep going and document

   } else {
      // no arguments
      return false;

   }

   // simple argument list for tcl
   if (md->getLanguage() == SrcLangExt_Tcl) {

      if (defArgList.count() == 0) {
         return false;
      }

      ol.endMemberDocName();

      ol.startParameterList(false);
      ol.startParameterType(true, 0);
      ol.endParameterType();

      ol.startParameterName(false);

      for (auto &a : defArgList) {

         if (a.defval.isEmpty()) {
            ol.docify(a.name + " ");

         } else {
            ol.docify("?" + a.name + "? ");
         }
      }

      ol.endParameterName(true, false, false);
      return true;
   }

   if (! md->isDefine()) {
      ol.docify(" ");
   }

   ol.pushGeneratorState();
   // ol.disableAllBut(OutputGenerator::Html);

   bool htmlOn  = ol.isEnabled(OutputGenerator::Html);
   bool latexOn = ol.isEnabled(OutputGenerator::Latex);

   {
      // html and latex
      if (htmlOn) {
         ol.enable(OutputGenerator::Html);
      }

      if (latexOn) {
         ol.enable(OutputGenerator::Latex);
      }

      ol.endMemberDocName();
      ol.startParameterList(! md->isObjCMethod());
   }

   ol.enableAll();
   ol.disable(OutputGenerator::Html);
   ol.disable(OutputGenerator::Latex);

   {
      // other formats
      if (! md->isObjCMethod()) {
         ol.docify("(");   // start argument list
      }

      ol.endMemberDocName();
   }

   ol.popGeneratorState();

   QString cName;

   if (scopeDef) {
      cName  = scopeDef->name();
      int il = cName.indexOf('<');
      int ir = cName.lastIndexOf('>');

      QSharedPointer<ClassDef> cd = scopeDef.dynamicCast<ClassDef>();

      const ArgumentList &tmpList = cd->getTemplateArgumentList();

      if (il != -1 && ir != -1 && ir > il) {
         cName = cName.mid(il, ir - il + 1);

      } else if (scopeDef->definitionType() == Definition::TypeClass && ! tmpList.listEmpty()) {
         cName = tempArgListToString(tmpList, scopeDef->getLanguage());

      } else {
         // no template specifier
         cName = "";
      }
   }

   bool first = true;
   bool paramTypeStarted = false;
   bool isDefine = md->isDefine();

   auto next = defArgList.begin();

   for (const Argument &arg : defArgList)  {

      ++next;

      if (isDefine || first) {
         ol.startParameterType(first, "");
         paramTypeStarted = true;

         if (isDefine) {
            ol.endParameterType();
            ol.startParameterName(true);
         }
      }

      QRegularExpression regExp_A("\\)\\(");
      QRegularExpression regExp_B("\\(.*\\*");

      int vp = arg.type.indexOf(regExp_A);
      int wp = arg.type.indexOf(regExp_B);

      if (! arg.attrib.isEmpty() && ! md->isObjCMethod()) {
         // argument has an IDL attribute
         ol.docify(arg.attrib + " ");
      }

      // use the following to put the function pointer type before the name
      bool hasFuncPtrType = false;

      if (hasFuncPtrType) {
         // argument type is a function pointer
         QString argType = arg.type.left(vp);

         argType = arg.type.left(wp);

         if (md->isObjCMethod()) {
            argType.prepend("(");
            argType.append(")");
         }

         if (! cName.isEmpty()) {
            argType = addTemplateNames(argType, scopeDef->name(), cName);
         }

         // added 01/2016
         argType = renameNS_Aliases(argType);

         linkifyText(TextGeneratorOLImpl(ol), scopeDef, md->getBodyDef(), md, argType);

      } else {
         // non-function pointer type
         QString argType = arg.type;

         if (md->isObjCMethod()) {
            argType.prepend("(");
            argType.append(")");
         }

         if (arg.type != "...") {
            if (! cName.isEmpty()) {
               argType = addTemplateNames(argType, scopeDef->name(), cName);
            }

            // added 01/2016
            argType = renameNS_Aliases(argType);

            linkifyText(TextGeneratorOLImpl(ol), scopeDef, md->getBodyDef(), md, argType);
         }
      }

      if (! isDefine) {
         if (paramTypeStarted) {
            ol.endParameterType();
            paramTypeStarted = false;
         }

         ol.startParameterName(defArgList.count() < 2);
      }

      if (hasFuncPtrType) {
         ol.docify(arg.type.mid(wp, vp - wp));
      }

      if (! arg.name.isEmpty() || arg.type == "...") {
         // argument has a name

         ol.disable(OutputGenerator::Latex);
         ol.disable(OutputGenerator::Html);
         ol.docify(" ");                         // man pages

         if (htmlOn) {
            ol.enable(OutputGenerator::Html);
         }

         ol.disable(OutputGenerator::Man);
         ol.startEmphasis();
         ol.enable(OutputGenerator::Man);

         if (latexOn) {
            ol.enable(OutputGenerator::Latex);
         }

         if (arg.name.isEmpty()) {
            ol.docify(arg.type);
         } else {
            ol.docify(arg.name);
         }

         ol.disable(OutputGenerator::Man);
         ol.disable(OutputGenerator::Latex);
         ol.endEmphasis();
         ol.enable(OutputGenerator::Man);

         if (latexOn) {
            ol.enable(OutputGenerator::Latex);
         }
      }

      if (! arg.array.isEmpty()) {
         ol.docify(arg.array);
      }

      if (hasFuncPtrType)  {
         // write the part of the argument type that comes after the name
         linkifyText(TextGeneratorOLImpl(ol), scopeDef, md->getBodyDef(), md, arg.type.right(arg.type.length() - vp));
      }

      if (! arg.defval.isEmpty()) {
         // write the default value
         QString n = arg.defval;

         if (! cName.isEmpty()) {
            n = addTemplateNames(n, scopeDef->name(), cName);
         }

         ol.docify(" = ");

         ol.startTypewriter();
         linkifyText(TextGeneratorOLImpl(ol), scopeDef, md->getBodyDef(), md, n, false, true, true);   // different
         ol.endTypewriter();
      }

      if (next != defArgList.end())  {

         if (! md->isObjCMethod()) {
            // there are more arguments
            ol.docify(", ");
         }

         if (! isDefine) {
            QString key;

            if (md->isObjCMethod() && next->attrib.length() >= 2) {
               // strip [ and ]

               key = next->attrib.mid(1, next->attrib.length() - 2);
               if (key != ",") {
                  key += ":";   // for normal keywords add colon
               }
            }

            ol.endParameterName(false, false, ! md->isObjCMethod());
            if (paramTypeStarted) {
               ol.endParameterType();
            }

            ol.startParameterType(false, key);
            paramTypeStarted = true;

         } else {
            // isDefine
            ol.endParameterName(false, false, true);
         }
      }

      first = false;
   }

   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);
   ol.disable(OutputGenerator::Latex);

   if (! md->isObjCMethod()) {
      ol.docify(")");   // end argument list
   }

   ol.enableAll();
   if (htmlOn) {
      ol.enable(OutputGenerator::Html);
   }

   if (latexOn) {
      ol.enable(OutputGenerator::Latex);
   }

   if (first) {
      ol.startParameterName(defArgList.count() < 2);
   }

   ol.endParameterName(true, defArgList.count() < 2, ! md->isObjCMethod());
   ol.popGeneratorState();

   if (! md->extraTypeChars().isEmpty() ) {
      ol.docify(md->extraTypeChars());
   }

   if (defArgList.constSpecifier) {
      ol.docify(" const");
   }

   if (defArgList.volatileSpecifier) {
      ol.docify(" volatile");
   }

   if (defArgList.refSpecifier == RefType::LValueRef) {
      ol.docify(" &");

   } else if (defArgList.refSpecifier == RefType::RValueRef) {
      ol.docify(" &&");
   }

   if (! defArgList.trailingReturnType.isEmpty()) {
      linkifyText(TextGeneratorOLImpl(ol), scopeDef, md->getBodyDef(), md, defArgList.trailingReturnType);
   }

   return true;
}

static void writeExceptionListImpl(OutputList &ol, QSharedPointer<ClassDef> cd, QSharedPointer<MemberDef> md,
                  const QString &exception)
{
   // this is an ordinary exception spec - there must be a '('

   int index = exception.indexOf('(');

   if (index != -1) {
      ol.exceptionEntry(exception.left(index), false);
      ++index;

      // paren in second column so skip it here

      for (int comma = exception.indexOf(',', index); comma != -1; ) {
         ++comma; // include comma
         linkifyText(TextGeneratorOLImpl(ol), cd, md->getBodyDef(), md, exception.mid(index, comma - index));
         ol.exceptionEntry(0, false);
         index = comma;
         comma = exception.indexOf(',', index);
      }

      int close = exception.indexOf(')', index);
      if (close != -1) {
         QString type = removeRedundantWhiteSpace(exception.mid(index, close - index));
         linkifyText(TextGeneratorOLImpl(ol), cd, md->getBodyDef(), md, type);
         ol.exceptionEntry(0, true);

      } else {
         warn(md->getDefFileName(), md->getDefLine(), "missing ) in exception list on member %s", csPrintable(md->name()));
      }

   } else { // Java Exception
      ol.docify(" ");
      linkifyText(TextGeneratorOLImpl(ol), cd, md->getBodyDef(), md, exception);
   }
}

static void writeExceptionList(OutputList &ol, QSharedPointer<ClassDef> cd, QSharedPointer<MemberDef> md)
{
   QString exception(md->excpString().trimmed());
   int len = exception.length();

   if (len > 0 )  {
      if (exception.at(0) == '{' ) {

         // this is an UNO IDL attribute - need special handling
         int index = exception.indexOf(';');

         int oldIndex = 1;

         while (-1 != index) { // there should be no more than 2 (set / get)
            // omit '{' and ';' -> "set raises (...)"
            writeExceptionListImpl(ol, cd, md, exception.mid(oldIndex, index - oldIndex));
            oldIndex = index + 1;
            index = exception.indexOf(';', oldIndex);
         }

         // the rest is now just '}' - omit that

      } else {
         writeExceptionListImpl(ol, cd, md, exception);

      }
   }
}

static void writeTemplatePrefix(OutputList &ol, const ArgumentList &argList)
{
   ol.docify("template<");

   auto nextItem = argList.begin();

   for (auto &a : argList) {
      ol.docify(a.type);
      ol.docify(" ");
      ol.docify(a.name);

      if (a.defval.length() != 0) {
         ol.docify(" = ");
         ol.docify(a.defval);
      }

      ++nextItem;

      if (nextItem != argList.end()) {
         ol.docify(", ");
      }
   }

   ol.docify("> ");
}

class MemberDefImpl
{
 public:
   MemberDefImpl();
   ~MemberDefImpl();

   // def must be a raw pointer

   void init(Definition *def, const QString &t, const QString &a, const QString &e, Protection p,
              Specifier v, bool s, Relationship r, MemberType mt,
              const ArgumentList &templateArgList, const ArgumentList &defArgList);

   QSharedPointer<ClassDef>     classDef;         // member of or related to
   QSharedPointer<FileDef>      fileDef;          // member of file definition
   QSharedPointer<NamespaceDef> nspace;           // the namespace this member is in.
   QSharedPointer<MemberDef>    enumScope;        // the enclosing scope, if this is an enum field
   QSharedPointer<MemberDef>    annEnumType;      // the anonymous enum that is the type of this member

   bool livesInsideEnum;
   QSharedPointer<MemberList>   enumFields;       // enumeration fields

   QSharedPointer<MemberDef>    redefines;        // the parent member
   QSharedPointer<MemberList>   redefinedBy;      // the list of members that redefine this one

   QSharedPointer<MemberDef>    memDef;           // member definition for this declaration
   QSharedPointer<MemberDef>    memDec;           // member declaration for this definition
   QSharedPointer<ClassDef>     relatedAlso;      // points to class marked by relatedAlso

   ExampleSDict exampleSDict;                     // a dictionary of all examples for quick access

   QString type;             // return actual type
   QString accessorType;     // return type that tell how to get to this member

   QSharedPointer<ClassDef> accessorClass;        // class that this member accesses (for anonymous types)

   QString m_args;          // function arguments/variable array specifiers
   QString def;             // member definition in code (fully qualified name)
   QString anc;             // HTML anchor name

   Specifier virt;          // normal/virtual/pure virtual
   Protection prot;         // protection type [Public/Protected/Private]

   QString decl;            // member declaration in class
   QString bitfields;       // struct member bitfields

   // copperspice properties
   QString m_read;
   QString m_write;
   QString m_reset;
   QString m_notify;

   QString exception;                // exceptions that can be thrown
   QString initializer;
   QString extraTypeChars;           // extra type info found after the argument list
   QString enumBaseType;             // base type of the enum (C++11)
   int initLines;                    // number of lines in the initializer

   Entry::Traits m_memberTraits;     // specifiers for this member

   MemberType mtype;                 // returns the kind of member
   int maxInitLines;                 // when the initializer will be displayed
   int userInitLines;                // result of explicit \hideinitializer or \showinitializer

   QSharedPointer<MemberDef> annMemb;

   ArgumentList  m_defArgList;       // argument list of this member definition
   ArgumentList  m_declArgList;      // argument list of this member declaration
   ArgumentList  m_templateArgList;  // template argument list of function template
   ArgumentList  m_typeConstraints;  // type constraints for template parameters

   // lists of template argument lists for template functions in nested template classes
   QVector<ArgumentList> m_defaultTemplateArgLists;

   QSharedPointer<MemberDef> templateMaster;

   // if the member has an anonymous compound as its type then this is computed by
   // getClassDefOfAnonymousType() and cached here
   QSharedPointer<ClassDef> cachedAnonymousType;

   QMap<QSharedPointer<Definition>, QSharedPointer<MemberList>> classSectionSDict;

   QSharedPointer<MemberDef> groupAlias;           // Member containing the definition

   int grpId;                                      // group id
   QSharedPointer<MemberGroup> memberGroup;        // group's member definition
   QSharedPointer<GroupDef> group;                 // group in which this member is in
   Grouping::GroupPri_t grouppri;                  // priority of this definition

   QString groupFileName;                          // file where this grouping was defined
   int groupStartLine;                             // line where this grouping was defined

   QSharedPointer<MemberDef> groupMember;

   bool isTypedefValCached;
   QSharedPointer<ClassDef> cachedTypedefValue;
   QString cachedTypedefTemplSpec;
   QString cachedResolvedType;

   // documentation inheritance
   QSharedPointer<MemberDef> docProvider;

   // to store the output file base from tag files
   QString explicitOutputFileBase;

   // objective-c
   bool implOnly; // function found in implementation but not in the interface
   bool hasDocumentedParams;
   bool hasDocumentedReturnType;
   bool isDMember;

   Relationship m_related;   // relationship of this to the class

   bool stat;                // is it a static function?
   bool proto;               // is it a prototype;
   bool docEnumValues;       // is an enum with documented enum values
   bool annScope;            // member is part of an anonymous scope
   bool annUsed;
   bool hasCallGraph;
   bool hasCallerGraph;
   bool explExt;             // member was explicitly declared external
   bool tspec;               // member is a template specialization
   bool groupHasDocs;        // true if the entry that caused the grouping was documented
   bool docsForDefinition;   // true => documentation block is put before definition.

   QSharedPointer<ClassDef> category;
   QSharedPointer<MemberDef> categoryRelation;
};

MemberDefImpl::MemberDefImpl() :
   enumFields(0), redefinedBy(0), category(0), categoryRelation(0)
{
}

MemberDefImpl::~MemberDefImpl()
{
}

void MemberDefImpl::init(Definition *def, const QString &t, const QString &a, const QString &e, Protection p,
                  Specifier v, bool s,  Relationship r, MemberType mt,
                  const ArgumentList &templateArgList, const ArgumentList &defArgList)
{
   redefines     = QSharedPointer<MemberDef>();
   relatedAlso   = QSharedPointer<ClassDef>();
   redefinedBy   = QSharedPointer<MemberList>();
   accessorClass = QSharedPointer<ClassDef>();

   memDef        = QSharedPointer<MemberDef>();
   memDec        = QSharedPointer<MemberDef>();
   group         = QSharedPointer<GroupDef>();

   grpId           = -1;
   livesInsideEnum = false;
   hasCallGraph    = false;
   hasCallerGraph  = false;
   initLines       = 0;
   type            = t;

   if (mt == MemberType_Typedef) {
      type = stripPrefix(type, "typedef ");
   }

   type   = removeRedundantWhiteSpace(type);
   m_args = removeRedundantWhiteSpace(a);

   if (type.isEmpty()) {
      decl = def->name() + m_args;
   } else {
      decl = type + " " + def->name() + m_args;
   }

   memberGroup = QSharedPointer<MemberGroup>();
   virt        = v;
   prot        = p;
   m_related   = r;
   stat        = s;
   mtype       = mt;
   exception   = e;
   proto       = false;
   annScope    = false;
   annUsed     = false;

   m_memberTraits = Entry::Traits{};

   annEnumType = QSharedPointer<MemberDef>();
   groupAlias  = QSharedPointer<MemberDef>();

   explExt = false;
   tspec   = false;

   cachedAnonymousType = QSharedPointer<ClassDef>();

   maxInitLines  = Config::getInt("max-init-lines");
   userInitLines = -1;
   docEnumValues = false;

   // copy function template arguments (if any)
   m_templateArgList = templateArgList;

   // copy function definition arguments (if any)
   m_defArgList = defArgList;

   // convert function declaration arguments (if any)
   m_declArgList = ArgumentList();

   if (! m_args.isEmpty()) {
      m_declArgList = stringToArgumentList(m_args, m_declArgList, extraTypeChars);
   }

   templateMaster = QSharedPointer<MemberDef>();

   classSectionSDict.clear();

   docsForDefinition  = true;
   isTypedefValCached = false;
   cachedTypedefValue = QSharedPointer<ClassDef>();

   implOnly = false;

   groupMember = QSharedPointer<MemberDef>();

   hasDocumentedParams = false;
   hasDocumentedReturnType = false;
   docProvider = QSharedPointer<MemberDef>();

   isDMember = def->getDefFileName().endsWith(".d", Qt::CaseInsensitive);
}

/*! Creates a new member definition.
 *
 * \param df  File containing the definition of this member.
 * \param dl  Line at which the member definition was found.
 * \param dc  column at which the member definition was found.
 * \param t   string representing the type of the member.
 * \param na  string representing the name of the member.
 * \param a   string representing the arguments of the member.
 * \param e   string representing the throw clause of the members.
 * \param p   protection context of the member
 * \param v   degree of `virtualness' of the member, possible values are: Normal, Virtual, Pure
 * \param s   boolean that is true if the member is static.
 * \param r   The relationship between the class and the member.
 * \param mt  The kind of member. See #MemberType for a list of all types.
 * \param tal The template arguments of this member.
 * \param al  The arguments of this member. This is a structured form of the string past as argument a.
 */

MemberDef::MemberDef(const QString &df, int dl, int dc, const QString &t, const QString &na,
                     const QString &a, const QString &e, Protection p, Specifier v, bool s,
                     Relationship r, MemberType mt, const ArgumentList &tal, const ArgumentList &al)
   : Definition(df, dl, dc, removeRedundantWhiteSpace(na)), visited(false), m_impl(new MemberDefImpl)
{
   m_impl->init(this, t, a, e, p, v, s, r, mt, tal, al);

   m_flow_count          = 1;
   m_isLinkableCached    = 0;
   m_isConstructorCached = 0;
   m_isDestructorCached  = 0;
}

MemberDef::MemberDef(const MemberDef &md)
   : Definition(md), visited(false), m_impl(new MemberDefImpl(*md.m_impl))
{
   m_flow_count          = md.m_flow_count;
   m_isLinkableCached    = 0;
   m_isConstructorCached = 0;
   m_isDestructorCached  = 0;
}

MemberDef &MemberDef::operator=(const MemberDef &)
{
   return *this;
}

QSharedPointer<MemberDef> MemberDef::deepCopy() const
{
   // make a copy of the object
   QSharedPointer<MemberDef> result  = QSharedPointer<MemberDef>(new MemberDef(*this));

   // clear pointers owned by object
   result->m_impl->redefinedBy       = QSharedPointer<MemberList>();

   result->m_impl->enumFields.clear();

   // replace pointers owned by the object by deep copies
   if (m_impl->redefinedBy) {
      for (auto md: *m_impl->redefinedBy) {
         result->insertReimplementedBy(md);
      }
   }

   result->m_impl->exampleSDict = m_impl->exampleSDict;

   if (m_impl->enumFields) {
      for (auto md: *m_impl->enumFields) {
         result->insertEnumField(md);
      }
   }

   result->m_impl->m_defArgList              = m_impl->m_defArgList;
   result->m_impl->m_declArgList             = m_impl->m_declArgList;
   result->m_impl->m_templateArgList         = m_impl->m_templateArgList;
   result->m_impl->m_typeConstraints         = m_impl->m_typeConstraints;
   result->m_impl->m_defaultTemplateArgLists = m_impl->m_defaultTemplateArgLists;

   result->m_impl->classSectionSDict = m_impl->classSectionSDict;

   return result;
}

void MemberDef::moveTo(QSharedPointer<Definition> scopeDef)
{
   setOuterScope(scopeDef);

   if (scopeDef->definitionType() == Definition::TypeClass) {
      m_impl->classDef = scopeDef.dynamicCast<ClassDef>();

   } else if (scopeDef->definitionType() == Definition::TypeFile) {
      m_impl->fileDef = scopeDef.dynamicCast<FileDef>();

   } else if (scopeDef->definitionType() == Definition::TypeNamespace) {
      m_impl->nspace = scopeDef.dynamicCast<NamespaceDef>();
   }

   m_isLinkableCached    = 0;
   m_isConstructorCached = 0;
}


/*! Destroys the member definition. */
MemberDef::~MemberDef()
{
}

void MemberDef::setReimplements(QSharedPointer<MemberDef> md)
{
   m_impl->redefines = md;
}

void MemberDef::insertReimplementedBy(QSharedPointer<MemberDef> md)
{
   if (m_impl->templateMaster) {
      m_impl->templateMaster->insertReimplementedBy(md);
   }

   if (m_impl->redefinedBy == 0) {
      m_impl->redefinedBy = QMakeShared<MemberList>(MemberListType_redefinedBy);
   }

   if (m_impl->redefinedBy->lastIndexOf(md) == -1) {
      m_impl->redefinedBy->inSort(md);
   }
}

QSharedPointer<MemberDef> MemberDef::reimplements() const
{
   return m_impl->redefines;
}

QSharedPointer<MemberList> MemberDef::reimplementedBy() const
{
   return m_impl->redefinedBy;
}

bool MemberDef::isReimplementedBy(QSharedPointer<ClassDef> cd) const
{
   if (cd && m_impl->redefinedBy) {

      for (auto md : *m_impl->redefinedBy) {
         QSharedPointer<ClassDef> mcd = md->getClassDef();

         if (mcd) {
            if (cd == mcd || cd->isBaseClass(mcd, true)) {
               return true;
            }
         }
      }
   }
   return false;
}

void MemberDef::insertEnumField(QSharedPointer<MemberDef> md)
{
   if (m_impl->enumFields == 0) {
      m_impl->enumFields = QMakeShared<MemberList>(MemberListType_enumFields);
   }

   m_impl->enumFields->append(md);
}

bool MemberDef::addExample(const QString &anchor, const QString &nameStr, const QString &file)
{
   if (m_impl->exampleSDict.find(nameStr) == 0) {
      QSharedPointer<Example> e = QMakeShared<Example>();

      e->anchor = anchor;
      e->name   = nameStr;
      e->file   = file;
      m_impl->exampleSDict.insert(nameStr, e);

      return true;
   }

   return false;
}

bool MemberDef::hasExamples()
{
   return (! m_impl->exampleSDict.isEmpty() );
}

QString MemberDef::getOutputFileBase() const
{
   static const bool separateMemberPages = Config::getBool("separate-member-pages");
   static const bool inlineSimpleClasses = Config::getBool("inline-simple-struct");

   QString baseName;

   if (! m_impl->explicitOutputFileBase.isEmpty()) {
      return m_impl->explicitOutputFileBase;

   } else if (m_impl->templateMaster) {
      return m_impl->templateMaster->getOutputFileBase();

   } else if (m_impl->group) {
      baseName = m_impl->group->getOutputFileBase();

   } else if (m_impl->classDef) {
      baseName = m_impl->classDef->getOutputFileBase();

      if (inlineSimpleClasses && m_impl->classDef->isSimple()) {
         return baseName;
      }

   } else if (m_impl->nspace) {
      baseName = m_impl->nspace->getOutputFileBase();

   } else if (m_impl->fileDef) {
      baseName = m_impl->fileDef->getOutputFileBase();

   }

   if (baseName.isEmpty()) {
      warn(getDefFileName(), getDefLine(), "Internal problem, member %s does not belong to any container", csPrintable(name()));
      return QString("dummy");

   } else if (separateMemberPages && isDetailedSectionLinkable()) {
      if (getEnumScope()) {
         // enum value, which is part of enum's documentation
         baseName += "_" + getEnumScope()->anchor();

      } else {
         baseName += "_" + anchor();
      }
   }

   return baseName;
}

QString MemberDef::getReference() const
{
   QString ref = Definition::getReference();

   if (! ref.isEmpty()) {
      return ref;
   }

   if (m_impl->templateMaster) {
      return m_impl->templateMaster->getReference();

   } else if (m_impl->group) {
      return m_impl->group->getReference();

   } else if (m_impl->classDef) {
      return m_impl->classDef->getReference();

   } else if (m_impl->nspace) {
      return m_impl->nspace->getReference();

   } else if (m_impl->fileDef) {
      return m_impl->fileDef->getReference();
   }

   return QString("");
}

QString MemberDef::anchor() const
{
   QString result = m_impl->anc;

   if (m_impl->groupAlias) {
      return m_impl->groupAlias->anchor();
   }

   if (m_impl->templateMaster) {
      return m_impl->templateMaster->anchor();
   }

   if (m_impl->enumScope && m_impl->enumScope != this) { // avoid recursion for C#'s public enum E { E, F }
      result.prepend(m_impl->enumScope->anchor());
   }

   if (m_impl->group) {
      if (m_impl->groupMember) {
         result = m_impl->groupMember->anchor();
      } else if (getReference().isEmpty()) {
         result.prepend("g");
      }
   }
   return result;
}

void MemberDef::computeLinkableInProject() const
{
   static bool extractStatic = Config::getBool("extract-static");

   // linkable
   m_isLinkableCached = 2;

   if (isHidden()) {
      m_isLinkableCached = 1;
      return;
   }

   if (m_impl->templateMaster) {
      m_isLinkableCached = m_impl->templateMaster->isLinkableInProject() ? 2 : 1;
      return;
   }

   if (name().isEmpty() || name().startsWith('@')) {
      // not a valid or a dummy name
      m_isLinkableCached = 1;
      return;
   }

   if (! hasDocumentation() || isReference()) {
      // no documentation
      m_isLinkableCached = 1;
      return;
   }

   if (m_impl->group && !m_impl->group->isLinkableInProject()) {
      // group but group not linkable
      m_isLinkableCached = 1;
      return;
   }

   if (! m_impl->group && m_impl->classDef && ! m_impl->classDef->isLinkableInProject()) {
      // in class but class not linkable
      m_isLinkableCached = 1;
      return;
   }

   if (! m_impl->group && m_impl->nspace && ! m_impl->m_related && !m_impl->nspace->isLinkableInProject()) {
      m_isLinkableCached = 1; // in namespace but namespace not linkable
      return;
   }

   if (! m_impl->group && !m_impl->nspace && ! m_impl->m_related && !m_impl->classDef &&
         m_impl->fileDef && !m_impl->fileDef->isLinkableInProject()) {

      m_isLinkableCached = 1;    // in file (and not in namespace) but file not linkable
      return;
   }

   if (! protectionLevelVisible(m_impl->prot) && ! isFriend() ) {
      m_isLinkableCached = 1;    // hidden due to protection
      return;
   }

   if (m_impl->stat && m_impl->classDef == 0 && ! extractStatic) {
      m_isLinkableCached = 1;    // hidden due to staticness
      return;
   }

   return;    // linkable
}

void MemberDef::setDocumentation(const QString &d, const QString &docFile, int docLine, bool stripWhiteSpace, bool atTop)
{
   Definition::setDocumentation(d, docFile, docLine, stripWhiteSpace, atTop);
   m_isLinkableCached = 0;
}

void MemberDef::setBriefDescription(const QString &b, const QString &briefFile, int briefLine)
{
   Definition::setBriefDescription(b, briefFile, briefLine);
   m_isLinkableCached = 0;
}

void MemberDef::setInbodyDocumentation(const QString &d, const QString &inbodyFile, int inbodyLine)
{
   Definition::setInbodyDocumentation(d, inbodyFile, inbodyLine);
   m_isLinkableCached = 0;
}

void MemberDef::setHidden(bool b)
{
   Definition::setHidden(b);
   m_isLinkableCached = 0;
}

bool MemberDef::isLinkableInProject() const
{
   if (m_isLinkableCached == 0) {
      computeLinkableInProject();
   }
   assert(m_isLinkableCached > 0);

   return m_isLinkableCached == 2;
}

bool MemberDef::isLinkable() const
{
   if (m_impl->templateMaster) {
      return m_impl->templateMaster->isLinkable();
   } else {
      return isLinkableInProject() || isReference();
   }
}

void MemberDef::writeLink(OutputList &ol, QSharedPointer<ClassDef> cd, QSharedPointer<NamespaceDef> nd,
                          QSharedPointer<FileDef> fd, QSharedPointer<GroupDef> gd, bool onlyText)
{
   static bool hideScopeNames = Config::getBool("hide-scope-names");

   SrcLangExt lang = getLanguage();

   QString sep = getLanguageSpecificSeparator(lang, true);
   QString n   = name();

   if (! hideScopeNames) {
      if (m_impl->enumScope && m_impl->livesInsideEnum) {
         n.prepend(m_impl->enumScope->displayName() + sep);
      }

      if (m_impl->classDef && gd && ! isRelated()) {
         n.prepend(m_impl->classDef->displayName() + sep);

      } else if (m_impl->nspace && (gd || fd)) {
         n.prepend(m_impl->nspace->displayName() + sep);
      }
   }

   if (isObjCMethod()) {
      if (isStatic()) {
         ol.docify("+ ");
      } else {
         ol.docify("- ");
      }
   }

   if (! onlyText && isLinkable()) {
      // write link

      if (m_impl->mtype == MemberType_EnumValue && getGroupDef() == 0 &&      // enum value is not grouped
            getEnumScope() && getEnumScope()->getGroupDef()) {

         // but its container is
         QSharedPointer<GroupDef> enumValGroup = getEnumScope()->getGroupDef();

         ol.writeObjectLink(enumValGroup->getReference(), enumValGroup->getOutputFileBase(), anchor(), n);

      } else {
         ol.writeObjectLink(getReference(), getOutputFileBase(), anchor(), n);
      }

   } else {
      // write only text
      ol.startBold();
      ol.docify(n);
      ol.endBold();
   }
}

/*! If this member has an anonymous class/struct/union as its type, then
 *  this method will return the ClassDef that describes this return type.
 */
QSharedPointer<ClassDef> MemberDef::getClassDefOfAnonymousType()
{
   if (m_impl->cachedAnonymousType) {
      return m_impl->cachedAnonymousType;
   }

   QString cname;

   if (getClassDef() != 0) {
      cname = getClassDef()->name();

   } else if (getNamespaceDef() != 0) {
      cname = getNamespaceDef()->name();

   }

   // search for the last anonymous scope in the member type
   QSharedPointer<ClassDef> annoClassDef;

   QString xType(m_impl->type);
   xType = stripPrefix(xType, "friend ");

   static QRegularExpression regExp("@[0-9]+");
   QRegularExpressionMatch match = regExp.match(xType);

   int len = match.capturedLength(0);

   if (match.hasMatch()) {
      // found anonymous scope in type
      auto xLeft  = match.capturedStart();
      auto xRight = match.capturedEnd();

      // extract anonymous scope
      while (xLeft != xType.begin()) {
         QChar c = *xLeft;

         if (isId(c) || c == ':' || c == '@') {
            --xLeft;
         } else {
            ++xLeft;
            break;
         }
      }

      while (xRight != xType.end()) {
         QChar c = *xRight;

         if (isId(c) || c == ':' || c == '@') {
            ++xRight;
         }  else {
            break;
         }
      }

      QStringView annName = QStringView(xLeft, xRight);

      // if inside a class or namespace try to prepend the scope name
      if (! cname.isEmpty() && annName.left(cname.length() + 2) != cname + "::") {
         QString ts   = stripAnonymousNamespaceScope(cname + "::" + annName);
         annoClassDef = getClass(ts);
      }

      // if not found yet try without scope name
      if (annoClassDef == 0) {
         QString ts   = stripAnonymousNamespaceScope(annName);
         annoClassDef = getClass(ts);
      }
   }

   m_impl->cachedAnonymousType = annoClassDef;

   return annoClassDef;
}

/*! This methods returns true if the brief section (also known as declaration section)
 *  is visible in the documentation.
 */
bool MemberDef::isBriefSectionVisible() const
{
   static const bool extractStatic       = Config::getBool("extract-static");
   static const bool hideUndocMembers    = Config::getBool("hide-undoc-members");
   static const bool briefMemberDesc     = Config::getBool("brief-member-desc");
   static const bool repeatBrief         = Config::getBool("repeat-brief");
   static const bool hideFriendCompounds = Config::getBool("hide-friend-compounds");
   static const bool alwaysDetailedSec   = Config::getBool("always-detailed-sec");

   QSharedPointer<MemberGroupInfo> info = Doxy_Globals::memGrpInfoDict[m_impl->grpId];

   bool hasDocs = hasDocumentation() || (m_impl->grpId != -1 && !(info->doc.isEmpty() && info->header.isEmpty()));

   // only include static members with file/namespace scope if enabled in the project file
   bool visibleIfStatic = ! (getClassDef() == 0 && isStatic() && ! extractStatic );

   // only include members which are documented or HIDE_UNDOC_MEMBERS is NO in the project file
   bool visibleIfDocumented = (! hideUndocMembers || hasDocs || isDocumentedFriendClass() );

   // hide members with no detailed description and brief description disabled
   bool fnTest1 = (hideUndocMembers    && documentation().isEmpty());
   bool fnTest2 = (! briefMemberDesc   && ! repeatBrief);
   bool fnTest3 = ! (alwaysDetailedSec && ! briefDescription().isEmpty());

   bool visibleIfEnabled = ! (fnTest1 && fnTest2 && fnTest3);

   // Hide friend (class|struct|union) declarations if HIDE_FRIEND_COMPOUNDS is true
   bool visibleIfFriendCompound = ! (hideFriendCompounds && isFriend() && (m_impl->type == "friend class" ||
                  m_impl->type == "friend struct" || m_impl->type == "friend union" ) );

   // only include members that are non-private unless extract_private is
   // set to YES or the member is part of a group
   bool visibleIfPrivate = (protectionLevelVisible(protection()) || isFriend() );

   // true if this member is a constructor or destructor
   bool cOrDTor = isConstructor() || isDestructor();

   // hide default constructors or destructors (no args) without documentation
   bool visibleIfNotDefaultCDTor = !( cOrDTor &&
               (m_impl->m_defArgList.listEmpty() || m_impl->m_defArgList.first().type == "void" ) && ! hasDocs);

   bool visible = visibleIfStatic && visibleIfDocumented && visibleIfEnabled && visibleIfPrivate &&
                  visibleIfNotDefaultCDTor && visibleIfFriendCompound && ! m_impl->annScope && ! isHidden();

   return visible;
}

QString MemberDef::getDeclType() const
{
   QString ltype(m_impl->type);

   if (m_impl->mtype == MemberType_Typedef) {
      ltype.prepend("typedef ");
   }

   if (isAlias()) {
      ltype = "using";
   }

   // strip `friend' keyword from ltype
   ltype = stripPrefix(ltype, "friend ");

   if (ltype == "@") {
      // rename type from enum values
      ltype = "";

   } else {
      if (isObjCMethod()) {
         ltype.prepend("(");
         ltype.append(")");
      }
   }
   return ltype;
}

void MemberDef::writeDeclaration(OutputList &ol, QSharedPointer<ClassDef> cd, QSharedPointer<NamespaceDef> nd,
                  QSharedPointer<FileDef> fd, QSharedPointer<GroupDef> gd, bool inGroup,
                  QSharedPointer<ClassDef> inheritedFrom, const QString &inheritId)
{
   // hide enum value since they appear already as part of the enum, unless they are explicitly grouped
   if (! inGroup && m_impl->mtype == MemberType_EnumValue) {
      return;
   }

   QSharedPointer<MemberDef> self = sharedFrom(this);
   QSharedPointer<Definition> d;

   assert (cd != 0 || nd != 0 || fd != 0 || gd != 0); // member should belong to something

   if (cd) {
      d = cd;

   } else if (nd) {
      d = nd;

   } else if (fd) {
      d = fd;

   } else {
      d = gd;

   }

   if (d == gd)  {
      QSharedPointer<Definition> tmp;

      if (tmp = getClassDef()) {
         d = tmp;

      } else if (tmp = getNamespaceDef()) {
         d = tmp;

      } else if (tmp = getFileDef())   {
         d = tmp;

      }
   }

   //_writeTagData(compoundType);
   _addToSearchIndex();

   QString cname  = d->name();
   QString cdname = d->displayName();
   QString cfname = getOutputFileBase();

   // search for the last anonymous scope in the member type
   QSharedPointer<ClassDef> annoClassDef = getClassDefOfAnonymousType();

   ol.startMemberDeclaration();

   // start a new member declaration
   bool isAnonymous = annoClassDef || m_impl->annMemb || m_impl->annEnumType;

   int x = 0;

   if (isAnonymous)  {
      x = 1;

   } else if (! m_impl->m_templateArgList.listEmpty()) {
      x = 3;

   }

   if (isDeprecated()) {
      ol.startMemberItem(anchor(), x, inheritId, true);
   } else {
      ol.startMemberItem(anchor(), x, inheritId, false);
   }

   // if there is no detailed description, write the anchor here
   bool detailsVisible = isDetailedSectionLinkable();

   // only write anchors for member which have no details and are
   // rendered inside the group page or are not grouped at all

   bool writeAnchor = (inGroup || m_impl->group == 0) && ! detailsVisible && !m_impl->annMemb;

   if (writeAnchor) {
      QString doxyArgs = argsString();
      QString doxyName = name();

      if (! cname.isEmpty()) {
         doxyName.prepend( (cdname + getLanguageSpecificSeparator(getLanguage())) );
      }

      ol.startDoxyAnchor(cfname, cname, anchor(), doxyName, doxyArgs);
   }

   if (! detailsVisible) {
      ol.pushGeneratorState();
      ol.disable(OutputGenerator::Man);
      ol.disable(OutputGenerator::Latex);
      ol.docify("\n");
      ol.popGeneratorState();
   }

   if (annoClassDef || m_impl->annMemb) {
      for (int j = 0; j < s_indentLevel; j++) {
         ol.writeNonBreakableSpace(3);
      }
   }

   // *** write template lists
   if (! m_impl->m_templateArgList.listEmpty() && getLanguage() == SrcLangExt_Cpp) {
      if (! isAnonymous) {
         ol.startMemberTemplateParams();
      }

      writeTemplatePrefix(ol, m_impl->m_templateArgList);
      if (! isAnonymous) {
         ol.endMemberTemplateParams(anchor(), inheritId);
      }
   }

   // *** write type
   QString ltype(m_impl->type);

   if (m_impl->mtype == MemberType_Typedef) {
      ltype.prepend("typedef ");
   }

   if (isAlias()) {
      ltype = "using";
   }

   // strip `friend' keyword from ltype
   ltype = stripPrefix(ltype, "friend ");

   static QRegularExpression regExp("@[0-9]+");
   QRegularExpressionMatch match = regExp.match(ltype);

   bool endAnonScopeNeeded = false;

   if (match.hasMatch()) {
      // member has an anonymous type

      int i = match.capturedStart() - ltype.constBegin();
      int l = match.capturedLength();

      if (annoClassDef) {
         // type is an anonymous compound
         int ir = i + l;

         ol.startAnonTypeScope(s_indentLevel++);
         annoClassDef->writeDeclaration(ol, m_impl->annMemb, inGroup, inheritedFrom, inheritId);

         ol.startMemberItem(anchor(), 2, inheritId);
         int j;

         for (j = 0; j < s_indentLevel - 1; j++) {
            ol.writeNonBreakableSpace(3);
         }

         QString varName = ltype.right(ltype.length() - ir).trimmed();

         ol.docify("}");

         if (varName.isEmpty() && (name().isEmpty() || name().startsWith('@'))) {
            ol.docify(";");

         } else if (! varName.isEmpty() && (varName.startsWith('*') || varName.startsWith('&'))) {
            ol.docify(" ");
            ol.docify(varName);
         }

         endAnonScopeNeeded = true;

      } else {
         if (getAnonymousEnumType()) {
            // type is an anonymous enum

            linkifyText(TextGeneratorOLImpl(ol), d, getBodyDef(), self, ltype.left(i));
            getAnonymousEnumType()->writeEnumDeclaration(ol, cd, nd, fd, gd);

            linkifyText(TextGeneratorOLImpl(ol), d, m_impl->fileDef, self, ltype.right(ltype.length() - i - l), true);  // different

         } else {
            ltype = ltype.left(i) + " { ... } " + removeAnonymousScopes(ltype.right(ltype.length() - i - l));
            linkifyText(TextGeneratorOLImpl(ol), d, getBodyDef(), self, ltype);
         }
      }

   } else if (ltype == "@") {
      // rename type from enum values
      ltype = "";

   } else {
      if (isObjCMethod()) {
         ltype.prepend("(");
         ltype.append(")");
      }

      linkifyText(TextGeneratorOLImpl(ol), d, getBodyDef(), self, ltype);
   }

   bool htmlOn = ol.isEnabled(OutputGenerator::Html);

   if (htmlOn && !ltype.isEmpty()) {
      ol.disable(OutputGenerator::Html);
   }

   if (! ltype.isEmpty()) {
      ol.docify(" ");
   }

   if (htmlOn) {
      ol.enable(OutputGenerator::Html);
   }

   if (m_impl->annMemb) {
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);
      ol.writeNonBreakableSpace(3);
      ol.popGeneratorState();

   } else {
      ol.insertMemberAlign(! m_impl->m_templateArgList.listEmpty());
   }

   static const bool extractPrivate      = Config::getBool("extract-private");
   static const bool extractStatic       = Config::getBool("extract-static");
   static const bool separateMemberPages = Config::getBool("separate-member-pages");
   static const bool briefMemberDesc     = Config::getBool("brief-member-desc");

   // write name
   if (! name().isEmpty() && name().at(0) != '@') {
      // hide anonymous stuff

      if (! (name().isEmpty() || name().at(0) == '@') && (hasDocumentation() || isReference()) &&
            ! (m_impl->prot == Private && ! extractPrivate && ! isFriend()) &&
            ! (isStatic() && m_impl->classDef == 0 && ! extractStatic)) {

         if (m_impl->annMemb) {

            m_impl->annMemb->writeLink(ol, m_impl->annMemb->getClassDef(), m_impl->annMemb->getNamespaceDef(),
                  m_impl->annMemb->getFileDef(), m_impl->annMemb->getGroupDef() );

            m_impl->annMemb->setAnonymousUsed();
            setAnonymousUsed();

         } else {
            QSharedPointer<ClassDef> rcd;

            if (isReference() && m_impl->classDef) {
               rcd = m_impl->classDef;
            }
            writeLink(ol, rcd, nd, fd, gd);
         }

      } else if (isDocumentedFriendClass()) {
         // if the member is an undocumented friend declaration for some class,
         // then maybe we can link to the class

         writeLink(ol, getClass(name()), QSharedPointer<NamespaceDef>(), QSharedPointer<FileDef>(), QSharedPointer<GroupDef>());

      } else {
         // there is a brief member description and brief member
         // descriptions are enabled or there is no detailed description

         if (m_impl->annMemb) {
            m_impl->annMemb->setAnonymousUsed();
            setAnonymousUsed();
         }

         QSharedPointer<ClassDef> rcd;
         if (isReference() && m_impl->classDef) {
            rcd = m_impl->classDef;
         }

         writeLink(ol, rcd, nd, fd, gd, true);
      }
   }

   // add to index
   if (isEnumerate() && ! name().isEmpty() && name().at(0) == '@') {
      // do not add to index

   } else {
      Doxy_Globals::indexList.addIndexItem(d, self);

   }

   // *** write arguments
   if (! argsString().isEmpty() && ! isObjCMethod()) {

      if (! isDefine() && ! isTypedef()) {
         ol.writeString(" ");
      }

      QString param5;

      if (isDefine()) {
         param5 = substitute(argsString(), ",", ", ");

      } else if (isTypedef()) {
         param5 = substitute(argsString(), ")(", ") (");

      } else {
         param5 = argsString();

      }

      linkifyText(TextGeneratorOLImpl(ol), d, getBodyDef(), self, param5, m_impl->annMemb, true, false, s_indentLevel); // different
   }

   // *** write exceptions
   if (!  excpString().isEmpty() ) {
      ol.writeString(" ");
      ol.docify(excpString());
   }

   // *** write bitfields
   if (! m_impl->bitfields.isEmpty()) {
      // add bitfields
      linkifyText(TextGeneratorOLImpl(ol), d, getBodyDef(), self, m_impl->bitfields);

   } else if (hasOneLineInitializer() ) {
      // add initializer

      if (!isDefine()) {
         ol.writeString(" ");
         linkifyText(TextGeneratorOLImpl(ol), d, getBodyDef(), self, m_impl->initializer.simplified());

      } else {
         ol.writeNonBreakableSpace(3);
         linkifyText(TextGeneratorOLImpl(ol), d, getBodyDef(), self, m_impl->initializer);
      }

   } else if (isAlias()) {
      // using template alias
      ol.writeString(" = ");
      linkifyText(TextGeneratorOLImpl(ol), d, getBodyDef(), self, m_impl->type);
   }

   if ((isObjCMethod() || isObjCProperty()) && isImplementation()) {
      ol.startTypewriter();
      ol.docify(" [implementation]");
      ol.endTypewriter();
   }

   if (isProperty() && (isSettable() || isGettable() || isPrivateSettable() || isPrivateGettable() ||
                        isProtectedSettable() || isProtectedGettable())) {

      ol.writeLatexSpacing();
      ol.startTypewriter();
      ol.docify(" [");

      QStringList sl;

      if (isGettable()) {
         sl.append("get");
      }

      if (isProtectedGettable()) {
         sl.append("protected get");
      }

      if (isSettable()) {
         sl.append("set");
      }

      if (isProtectedSettable()) {
         sl.append("protected set");
      }

      if (extractPrivate) {
         if (isPrivateGettable()) {
            sl.append("private get");
         }

         if (isPrivateSettable()) {
            sl.append("private set");
         }
      }

      auto nextItem = sl.begin();

      for (auto s : sl) {
         ol.docify(s);

         ++nextItem;

         if (nextItem != sl.end()) {
            ol.docify(", ");
         }
      }

      ol.docify("]");
      ol.endTypewriter();
   }

   if (isEvent() && (isAddable() || isRemovable() || isRaisable())) {
      ol.writeLatexSpacing();
      ol.startTypewriter();
      ol.docify(" [");

      QStringList sl;

      if (isAddable()) {
         sl.append("add");
      }
      if (isRemovable()) {
         sl.append("remove");
      }
      if (isRaisable()) {
         sl.append("raise");
      }

      auto nextItem = sl.begin();

      for (auto s : sl) {
         ol.docify(s);
         ++nextItem;

         if (nextItem != sl.end()) {
            ol.docify(", ");
         }
      }

      ol.docify("]");
      ol.endTypewriter();
   }

   if (writeAnchor) {
      ol.endDoxyAnchor(cfname, anchor());
   }

   ol.endMemberItem();
   if (endAnonScopeNeeded) {
      ol.endAnonTypeScope(--s_indentLevel);
   }

   // write brief description
   if (! briefDescription().isEmpty() && briefMemberDesc) {

      DocRoot *rootNode = validatingParseDoc(briefFile(), briefLine(), getOuterScope() ? getOuterScope() : d,
                  self, briefDescription(), true, false, "", true, false);

      if (rootNode && ! rootNode->isEmpty()) {
         ol.startMemberDescription(anchor(), inheritId);

         // write the brief description
         ol.writeDoc(rootNode, getOuterScope() ? getOuterScope() : d, self);

         if (detailsVisible) {

            ol.pushGeneratorState();
            ol.disableAllBut(OutputGenerator::Html);
            //ol.endEmphasis();

            ol.docify(" ");

            if (inheritedFrom || separateMemberPages || (m_impl->group != 0 && gd == 0) ||
                  (m_impl->nspace != 0 && nd == 0) ) {

               // forward link to the page, group, or namespace
               ol.startTextLink(getOutputFileBase(), anchor());

            } else {
               // local link
               ol.startTextLink(0, anchor());
            }

            ol.parseText(theTranslator->trMore());
            ol.endTextLink();
            //ol.startEmphasis();
            ol.popGeneratorState();
         }

         // for RTF we need to add an extra empty paragraph
         ol.pushGeneratorState();
         ol.disableAllBut(OutputGenerator::RTF);
         ol.startParagraph();
         ol.endParagraph();
         ol.popGeneratorState();
         ol.endMemberDescription();
      }
      delete rootNode;
   }

   ol.endMemberDeclaration(anchor(), inheritId);

   warnIfUndocumented();
}

bool MemberDef::isDetailedSectionLinkable() const
{
   static const bool extractAll         = Config::getBool("extract-all");
   static const bool extractStatic      = Config::getBool("extract-static");
   static const bool alwaysDetailedSec  = Config::getBool("always-detailed-sec");
   static const bool repeatBrief        = Config::getBool("repeat-brief");
   static const bool briefMemberDesc    = Config::getBool("brief-member-desc");
   static const bool hideUndocMembers   = Config::getBool("hide-undoc-members");
   static const bool hideFriendCompound = Config::getBool("hide-friend-compounds");

   // the member has details documentation for any of the following reasons

   bool docFilter = ( extractAll || ! documentation().isEmpty() || ! inbodyDocumentation().isEmpty() );

   docFilter = ( docFilter || (m_impl->mtype == MemberType_Enumeration && m_impl->docEnumValues) );
   docFilter = ( docFilter || (m_impl->mtype == MemberType_EnumValue && ! briefDescription().isEmpty() ) );

   // has brief docs, visible in detailed section or they are explicitly not shown in brief section
   bool temp_a = (! briefDescription().isEmpty() && (alwaysDetailedSec && (repeatBrief || ! briefMemberDesc) ) );
   bool temp_b = (hasMultiLineInitializer() && ! hideUndocMembers);

   // has one or more documented arguments
   bool temp_c = (! m_impl->m_defArgList.listEmpty() && m_impl->m_defArgList.hasDocumentation() );

   bool isAttribute = m_impl->m_memberTraits.hasTrait(Entry::Virtue::Attribute);
   bool isProperty  = m_impl->m_memberTraits.hasTrait(Entry::Virtue::Property);

   docFilter = ( docFilter || temp_a || temp_b || temp_c || (isAttribute || isProperty) || Doxy_Globals::userComments);


   // not a global static or global statics should be extracted
   bool staticFilter = getClassDef() != 0 || ! isStatic() || extractStatic;

   // only include members which are non-private unless extract_private is set or the member is part of a group
   bool privateFilter = protectionLevelVisible(protection()) || isFriend();


   // (not used ) member is part of an anonymous scope that is the type of another member in the list
   // bool inAnonymousScope = ! briefDescription().isEmpty() && annUsed;


   // Step E: hide friend (class|struct|union) member if HIDE_FRIEND_COMPOUNDS is set
   bool temp_e = (m_impl->type == "friend class" || m_impl->type == "friend struct" || m_impl->type == "friend union");
   bool friendCompoundFilter = ( ! hideFriendCompound || ! isFriend() || ! (isAttribute || isProperty) );

   bool result = ( docFilter && staticFilter && privateFilter && friendCompoundFilter && ! isHidden() );

   return result;
}

bool MemberDef::isDetailedSectionVisible(bool inGroup, bool inFile) const
{
   static const bool separateMemPages    = Config::getBool("separate-member-pages");
   static const bool inlineSimpleStructs = Config::getBool("inline-simple-struct");
   static const bool hideUndocMembers    = Config::getBool("hide-undoc-members");

   bool groupFilter = getGroupDef() == 0 || inGroup || separateMemPages;
   bool fileFilter  = getNamespaceDef() == 0 || ! inFile;

   bool simpleFilter = (hasBriefDescription() || ! hideUndocMembers) && inlineSimpleStructs &&
                       getClassDef() != 0 && getClassDef()->isSimple();

   bool visible = isDetailedSectionLinkable() && groupFilter && fileFilter && ! isReference();
   bool result = visible || simpleFilter;

   return result;
}

void MemberDef::getLabels(QStringList &sl, QSharedPointer<Definition> container) const
{
   static const bool isInlineInfo   = Config::getBool("inline-info");
   static const bool extractPrivate = Config::getBool("extract-private");

   Specifier lvirt = virtualness();
   SrcLangExt lang = getLanguage();

   if (true) {

/*
      ( (! isObjCMethod() || isOptional() || isRequired()) &&
         (protection() != Public || lvirt != Normal ||
          isFriend() || isRelated() || (isInline() && inlineInfo) || isSignal() || isSlot() || isStatic() ||
          (m_impl->classDef && m_impl->classDef != container && container->definitionType() == TypeClass) ||
          (m_impl->memSpec & ~Entry::Inline) != 0 ) ) {
*/

      if (isFriend()) {
         sl.append("friend");

      } else if (isRelated()) {
         sl.append("related");

      } else {

         if (isInlineInfo && isInline()) {
            sl.append("inline");
         }

         if (isDeprecated()) {
            sl.append("deprecated");
         }

         if (isExplicit()) {
            sl.append("explicit");
         }

         if (isMutable()) {
            sl.append("mutable");
         }

         if (isStatic()) {
            sl.append("static");
         }

         if (isGettable()) {
            sl.append("get");
         }

         if (isProtectedGettable()) {
            sl.append("protected get");
         }

         if (isSettable()) {
            sl.append("set");
         }

         if (isProtectedSettable()) {
            sl.append("protected set");
         }

         if (extractPrivate) {

            if (isPrivateGettable()) {
               sl.append("private get");
            }

            if (isPrivateSettable()) {
               sl.append("private set");
            }
         }

         if (isAddable()) {
            sl.append("add");
         }

         if (! isUNOProperty() && isRemovable()) {
            sl.append("remove");
         }

         if (isRaisable()) {
            sl.append("raise");
         }

         if (isReadable()) {
            sl.append("read");
         }

         if (isWritable()) {
            sl.append("write");
         }

         if (isNotify()) {
           sl.append("notify");
         }

         if (isReset()) {
           sl.append("reset");
         }

         if (isRevision()) {
           sl.append("revision");
         }

         if (isDesignable()) {
           sl.append("designable");
         }

         if (isScriptable()) {
           sl.append("scriptable");
         }

         if (isStored()) {
           sl.append("stored");
         }

         if (isUser()) {
           sl.append("user");
         }

         if (isConstant()) {
           sl.append("constant");
         }

         if (isFinal_Property()) {
            sl.append("final_property");
         }

         if (isAbstract()) {
            sl.append("abstract");
         }

         if (isFinal()) {
            sl.append("final");
         }

         if (isOverride()) {
            sl.append("override");
         }

         if (isInitonly()) {
            sl.append("initonly");
         }

         if (isSealed()) {
            sl.append("sealed");
         }

         if (isNew()) {
            sl.append("new");
         }

         if (isOptional()) {
            sl.append("optional");
         }

         if (isRequired()) {
            sl.append("required");
         }

         if (isNonAtomic()) {
            sl.append("nonatomic");

         } else if (isObjCProperty()) {
            sl.append("atomic");

         }

         // mutual exclusive Objective 2.0 property attributes
         if (isAssign()) {
            sl.append("assign");

         } else if (isCopy()) {
            sl.append("copy");

         } else if (isRetain()) {
            sl.append("retain");

         } else if (isWeak()) {
            sl.append("weak");

         } else if (isStrong()) {
            sl.append("strong");

         } else if (isUnretained()) {
            sl.append("unsafe_unretained");

         }

         if (! isObjCMethod()) {

            if (protection() == Protected) {
               sl.append("protected");

            } else if (protection() == Private) {
               sl.append("private");

            } else if (protection() == Package) {
               sl.append("package");

            }

            if (isConstExpr()) {
               sl.append("constexpr");
            }

            if (lvirt == Virtual) {
               sl.append("virtual");

            } else if (lvirt == Pure) {
               sl.append("pure virtual");
            }

            if (isSignal()) {
               sl.append("signal");
            }

            if (isSlot()) {
               sl.append("slot");
            }

            if (isDefault()) {
               sl.append("default");
            }

            if (isDelete()) {
               sl.append("delete");
            }

            if (isNoExcept()) {
               sl.append("noexcept");
            }

            if (isAttribute()) {
               sl.append("attribute");
            }

            if (isUNOProperty()) {
               sl.append("property");
            }

            if (isReadonly()) {
               sl.append("readonly");
            }

            if (isBound()) {
               sl.append("bound");
            }

            if (isUNOProperty() && isRemovable()) {
               sl.append("removable");
            }

            if (isConstrained()) {
               sl.append("constrained");
            }

            if (isTransient()) {
               sl.append("transient");
            }

            if (isMaybeVoid()) {
               sl.append("maybevoid");
            }

            if (isMaybeDefault()) {
               sl.append("maybedefault");
            }

            if (isMaybeAmbiguous()) {
               sl.append("maybeambiguous");
            }

            if (isPublished()) {
               sl.append("published");   // enum
            }
         }

         if (isObjCProperty() && isImplementation()) {
            sl.append("implementation");
         }
      }

      if (m_impl->classDef && container->definitionType() == TypeClass && m_impl->classDef != container && ! isRelated()) {
         sl.append("inherited");
      }

   } else if (isObjCMethod() && isImplementation()) {
      sl.append("implementation");

   }
}

void MemberDef::_writeCallGraph(OutputList &ol)
{
   QSharedPointer<MemberDef> self = sharedFrom(this);

   static const bool haveDot = Config::getBool("have-dot");

   // write call graph
   if (m_impl->hasCallGraph && (isFunction() || isSlot() || isSignal()) && haveDot) {

      DotCallGraph callGraph(self, false);

      if (callGraph.isTooBig()) {
         warn_uncond("Call graph for '%s' not generated, too many nodes. Consider increasing "
                     "DOT_GRAPH_MAX_NODES.\n", csPrintable(qualifiedName()));

      } else if (! callGraph.isTrivial()) {
         msg("Generating call graph for function %s\n", csPrintable(qualifiedName()));

         ol.disable(OutputGenerator::Man);
         ol.startCallGraph();

         ol.startParagraph();
         ol.parseText(theTranslator->trCallGraph());
         ol.endParagraph();

         ol.endCallGraph(callGraph);
         ol.enableAll();
      }
   }
}

void MemberDef::_writeCallerGraph(OutputList &ol)
{
   QSharedPointer<MemberDef> self = sharedFrom(this);

   static const bool haveDot = Config::getBool("have-dot");

   if (m_impl->hasCallerGraph && (isFunction() || isSlot() || isSignal()) && haveDot) {

      DotCallGraph callerGraph(self, true);

      if (callerGraph.isTooBig()) {
         warn_uncond("Caller graph for '%s' was not generated, too many nodes. "
                     "Consider increasing DOT_GRAPH_MAX_NODES.\n", csPrintable(qualifiedName()));

      } else if (! callerGraph.isTrivial() && !callerGraph.isTooBig()) {
         msg("Generating caller graph for function %s\n", csPrintable(qualifiedName()));

         ol.disable(OutputGenerator::Man);
         ol.startCallGraph();

         ol.startParagraph();
         ol.parseText(theTranslator->trCallerGraph());
         ol.endParagraph();

         ol.endCallGraph(callerGraph);
         ol.enableAll();
      }
   }
}

void MemberDef::_writeReimplements(OutputList &ol)
{
   QSharedPointer<MemberDef> bmd = reimplements();
   QSharedPointer<ClassDef> bcd;

   if (bmd && (bcd = bmd->getClassDef())) {
      // write class that contains a member that is reimplemented by this one

      if (bcd->isLinkable()) {
         ol.startParagraph();
         QString reimplFromLine;

         if (bmd->virtualness() != Pure && bcd->compoundType() != CompoundType::Interface) {
            reimplFromLine = theTranslator->trReimplementedFromList(1);

         } else {
            reimplFromLine = theTranslator->trImplementedFromList(1);

         }

         int markerPos = reimplFromLine.indexOf("@0");

         if (markerPos != -1) {
            // should always pass this
            ol.parseText(reimplFromLine.left(markerPos)); //text left from marker

            if (bmd->isLinkable()) {
               // replace marker with link

               ol.writeObjectLink(bmd->getReference(), bmd->getOutputFileBase(), bmd->anchor(), bcd->displayName());

               if ( bmd->isLinkableInProject() ) {
                  writePageRef(ol, bmd->getOutputFileBase(), bmd->anchor());
               }

            } else {
               ol.writeObjectLink(bcd->getReference(), bcd->getOutputFileBase(), 0, bcd->displayName());

               if ( bcd->isLinkableInProject() ) {
                  writePageRef(ol, bcd->getOutputFileBase(), bcd->anchor());
               }
            }

            ol.parseText(reimplFromLine.right(reimplFromLine.length() - markerPos - 2)); // text right from marker

         } else {
            err("Translation has no marker in trReimplementsFromList()\n");
         }

         ol.endParagraph();
      }
   }
}

void MemberDef::_writeReimplementedBy(OutputList &ol)
{
   QSharedPointer<MemberList> bml = reimplementedBy();

   if (bml) {
      uint count = 0;
      QSharedPointer<ClassDef> bcd;

      for (auto bmd : *bml)  {

         // count the members that directly inherit from md and for
         // which the member and class are visible in the docs.

         bcd = bmd->getClassDef();
         if (! bcd) {
            break;
         }


         if ( bmd->isLinkable() && bcd->isLinkable() ) {
            count++;
         }
      }

      QSharedPointer<MemberDef> bmd;
      auto iter_bmd = bml->begin();

      if (count > 0) {
         // write the list of classes that overwrite this member
         ol.startParagraph();

         QString reimplInLine;

         if (m_impl->virt == Pure || (m_impl->classDef && m_impl->classDef->compoundType() == CompoundType::Interface)) {
            reimplInLine = theTranslator->trImplementedInList(count);

         } else {
            reimplInLine = theTranslator->trReimplementedInList(count);
         }

         static QRegularExpression regExp_marker("@[0-9]+");
         QRegularExpressionMatch match = regExp_marker.match(reimplInLine);

         QString::const_iterator iter_i = reimplInLine.constBegin();
         QString::const_iterator iter_new;

         // replace all markers in reimplInLine with links to the classes
         while (match.hasMatch()) {
            iter_new = match.capturedStart();

            ol.parseText(QStringView(iter_i, iter_new));

            bool ok;
            uint entryIndex = QString(iter_new + 1, match.capturedEnd()).toInteger<uint>(&ok);

            count = 0;

            if (ok) {
               // find the entryIndex-th documented entry in the inheritance list.
               for (iter_bmd = bml->end() - 1; (bmd = *iter_bmd) && (bcd = bmd->getClassDef()); --iter_bmd) {

                  if (bmd->isLinkable() && bcd->isLinkable()) {
                     if (count == entryIndex) {
                        break;
                     }

                     ++count;
                  }
               }

               if (bcd && bmd) {
                  // write link for marker
                  ol.writeObjectLink(bmd->getReference(), bmd->getOutputFileBase(), bmd->anchor(), bcd->displayName());

                  if (bmd->isLinkableInProject() ) {
                     writePageRef(ol, bmd->getOutputFileBase(), bmd->anchor());
                  }
               }
            }

            ++iter_bmd;

            iter_i = match.capturedEnd();
            match = regExp_marker.match(reimplInLine, iter_i);
         }

         ol.parseText(QStringView(iter_i, reimplInLine.constEnd()));
         ol.endParagraph();
      }
   }
}

void MemberDef::_writeCategoryRelation(OutputList &ol)
{
   if (m_impl->classDef) { // this should be a member of a class/category

      QString text;
      QString ref;
      QString file;
      QString anc;
      QString name;

      int i = -1;

      if (m_impl->categoryRelation && m_impl->categoryRelation->isLinkable()) {
         if (m_impl->category) {
            // this member is in a normal class and implements method categoryRelation from category
            // so link to method 'categoryRelation' with 'provided by category 'category' text.
            text = theTranslator->trProvidedByCategory();
            name = m_impl->category->displayName();

         } else if (m_impl->classDef->categoryOf()) {
            // this member is part of a category so link to the corresponding class member of the class we extend
            // so link to method 'categoryRelation' with 'extends class 'classDef->categoryOf()'
            text = theTranslator->trExtendsClass();
            name = m_impl->classDef->categoryOf()->displayName();
         }

         i = text.indexOf("@1");

         if (i != -1) {
            QSharedPointer<MemberDef> md = m_impl->categoryRelation;

            ref  = md->getReference();
            file = md->getOutputFileBase();
            anc  = md->anchor();
         }
      }
      if (i != -1 && !name.isEmpty()) {
         ol.startParagraph();
         ol.parseText(text.left(i));
         ol.writeObjectLink(ref, file, anc, name);
         ol.parseText(text.mid(i + 2));
         ol.endParagraph();
      }
   }
}

void MemberDef::_writeExamples(OutputList &ol)
{
   // write the list of examples that use this member
   if (hasExamples()) {
      ol.startSimpleSect(BaseOutputDocInterface::Examples, 0, 0, theTranslator->trExamples() + ": ");
      ol.startDescForItem();
      writeExample(ol, m_impl->exampleSDict);
      ol.endDescForItem();
      ol.endSimpleSect();
   }
}

void MemberDef::_writeTypeConstraints(OutputList &ol)
{
   QSharedPointer<MemberDef> self = sharedFrom(this);

   if (! m_impl->m_typeConstraints.listEmpty()) {
      writeTypeConstraints(ol, self, m_impl->m_typeConstraints);
   }
}

void MemberDef::_writeEnumValues(OutputList &ol, QSharedPointer<Definition> container, const QString &cfname,
                  const QString &ciname, const QString &cname)
{
   if (isEnumerate()) {
      bool first = true;
      QSharedPointer<MemberList> fmdl = enumFieldList();

      if (fmdl) {

         for (auto fmd : *fmdl) {

            if (fmd->isLinkable()) {
               // for enums which have documentation, show the enum value name & the documentation
               // enum value is documented as a "\var"

               if (first) {
                  // table for enum values
                  ol.startEnumTable();
                  first = false;
               }

               ol.startDescTableRow();
               ol.addIndexItem(fmd->name(), ciname);
               ol.addIndexItem(ciname, fmd->name());

               Doxy_Globals::indexList.addIndexItem(container, fmd);

               ol.startDescTableTitle();

               ol.startDoxyAnchor(cfname, cname, fmd->anchor(), fmd->name(), fmd->argsString());
               ol.docify(fmd->name());

               ol.disableAllBut(OutputGenerator::Man);
               ol.writeString(" ");
               ol.enableAll();

               ol.endDoxyAnchor(cfname, fmd->anchor());
               ol.endDescTableTitle();

               ol.startDescTableData();

               bool hasBrief   = ! fmd->briefDescription().isEmpty();
               bool hasDetails = ! fmd->documentation().isEmpty();

               if (hasBrief) {
                  ol.generateDoc(fmd->briefFile(), fmd->briefLine(), getOuterScope() ? getOuterScope() : container,
                                 fmd, fmd->briefDescription(), true, false);
               }

               if (hasDetails) {
                  ol.generateDoc(fmd->docFile(), fmd->docLine(), getOuterScope() ? getOuterScope() : container,
                                 fmd, fmd->documentation() + "\n", true, false);
               }

               ol.endDescTableData();
               ol.endDescTableRow();
            }
         }
      }

      if (! first) {
         ol.endEnumTable();
      }
   }
}

QString MemberDef::displayDefinition() const
{
   QString ldef  = definition();
   QString title = name();

   if (isEnumerate()) {

      if (! title.isEmpty() && title.at(0) == '@') {
         ldef = title = "anonymous enum";

         if (!m_impl->enumBaseType.isEmpty()) {
            ldef += " : " + m_impl->enumBaseType;
         }

      } else {
         ldef.prepend("enum ");
      }

   } else if (isEnumValue()) {
      if (! ldef.isEmpty() && ldef.at(0) == '@') {
         ldef = ldef.mid(2);
      }
   }

   static QRegularExpression regExp("@[0-9]+");
   QRegularExpressionMatch match = regExp.match(ldef);

   int pos      = -1;
   int matchLen = 0;

   if (match.hasMatch()) {
      pos      = match.capturedStart() - ldef.constBegin();
      matchLen = match.capturedLength();
   }

   if (pos != -1) {
      // replace anonymous parts with { ... }

      int si = ldef.indexOf(' ');
      int ei = pos + matchLen;
      int pi = -1;

      if (si == -1) {
         si = 0;
      }

      match = regExp.match(ldef, match.capturedEnd());

      while (match.hasMatch()) {
         pi       = match.capturedStart() - ldef.constBegin();
         matchLen = match.capturedLength();

         pos = pi;
         ei  = pos + matchLen;

         match = regExp.match(ldef, match.capturedEnd());
      }

      int ni = ldef.indexOf("::", si);
      if (ni >= ei) {
         ei = ni + 2;
      }

      ldef = ldef.left(si) + " { ... } " + ldef.right(ldef.length() - ei);
   }

   QSharedPointer<ClassDef> cd = getClassDef();

   if (cd && cd->isObjectiveC()) {
      // strip scope name

      int ep = ldef.indexOf("::");

      if (ep != -1) {
         int sp = ldef.lastIndexOf(' ', ep);

         if (sp != -1) {
            ldef = ldef.left(sp + 1) + ldef.mid(ep + 2);
         }
      }

      // strip keywords
      int dp = ldef.indexOf(':');

      if (dp != -1) {
         ldef = ldef.left(dp + 1);
      }

      matchLen = ldef.length();
      pos      = matchLen - pos;

      while (pos >= 0 && (isId(ldef.at(pos)) || ldef.at(pos) == ':')) {
         --pos;
      }

      while (pos >= 0 &&  ldef.at(pos).isSpace() ) {
         --pos;
      }

      if (pos > 0) {
         // insert braches around the type
         QString tmp("(" + ldef.left(pos + 1) + ")" + ldef.mid(pos + 1));
         ldef = tmp;
      }

      if (isStatic()) {
         ldef.prepend("+ ");
      } else {
         ldef.prepend("- ");
      }
   }

   SrcLangExt lang = getLanguage();
   QString sep = getLanguageSpecificSeparator(lang, true);

   return substitute(ldef, "::", sep);
}

void MemberDef::_writeGroupInclude(OutputList &ol, bool inGroup)
{
   // only write out the include file if this is not part of a class or file definition
   static const bool showGroupedMembInc = Config::getBool("show-grouped-members-inc");

   QSharedPointer<FileDef> fd = getFileDef();
   QString nm;

   if (fd) {
      nm = getFileDef()->docName();
   }

   if (inGroup && fd && showGroupedMembInc && !nm.isEmpty()) {
      ol.startParagraph();
      ol.startTypewriter();

      SrcLangExt lang = getLanguage();

      bool isIDLorJava = lang == SrcLangExt_IDL || lang == SrcLangExt_Java;
      if (isIDLorJava) {
         ol.docify("import ");
      } else {
         ol.docify("#include ");
      }

      if (isIDLorJava) {
         ol.docify("\"");
      } else {
         ol.docify("<");
      }

      if (fd && fd->isLinkable()) {
         ol.writeObjectLink(fd->getReference(), fd->getOutputFileBase(), fd->anchor(), nm);
      } else {
         ol.docify(nm);
      }

      if (isIDLorJava) {
         ol.docify("\"");
      } else {
         ol.docify(">");
      }

      ol.endTypewriter();
      ol.endParagraph();
   }
}

/*! Writes the "detailed documentation" section of this member to all active output formats
 */
void MemberDef::writeDocumentation(QSharedPointer<MemberList> ml, OutputList &ol, const QString &scName,
                  QSharedPointer<Definition> container, bool inGroup, bool showEnumValues, bool showInline)
{
   QSharedPointer<MemberDef> self = sharedFrom(this);

   static const bool hideScopeNames  = Config::getBool("hide-scope-names");
   static const bool repeatBrief     = Config::getBool("repeat-brief");
   static const bool briefMemberDesc = Config::getBool("brief-member-desc");

   // if this member is in a group find the real scope name
   bool hasParameterList = false;
   bool inFile  = container->definitionType() == Definition::TypeFile;
   bool hasDocs = isDetailedSectionVisible(inGroup, inFile);

   QSharedPointer<ClassDef> cd = getClassDef();

   if (! hasDocs) {
      return;
   }

   if (isEnumValue() && ! showEnumValues) {
      return;
   }

   SrcLangExt lang = getLanguage();
   QString sep     = getLanguageSpecificSeparator(lang, true);

   QString scopeName = scName;
   QString memAnchor = anchor();
   QString ciname    = container->displayName();

   QSharedPointer<Definition> scopedContainer = container;

   if (container->definitionType() == TypeGroup) {
      QSharedPointer<Definition> tmp;

      if (tmp = getClassDef()) {
         scopeName = tmp->displayName();
         scopedContainer = tmp;

      } else if (tmp = getNamespaceDef()) {
         scopeName = tmp->displayName();
         scopedContainer = tmp;

      } else if (tmp = getFileDef()) {
         scopeName = tmp->displayName();
         scopedContainer = tmp;

      }

      ciname = container.dynamicCast<GroupDef>()->groupTitle();

   } else if (container->definitionType() == TypeFile && getNamespaceDef() && lang != SrcLangExt_Python) {
      // member is in a namespace but is written as part of the file documentation as well
      // make sure its label is unique
      memAnchor.prepend("file_");
   }

   QString cname  = container->name();
   QString cfname = getOutputFileBase();

   QString fullMemberName = name();           // member name with scope
   QString memberArgs     = argsString();     // argument list, detailed docs
   QString ldef           = definition();     // member name, detailed docs
   QString title          = name();           // method list

   // prepend scope if there is any. TODO: make this optional for C only docs
   if (! scopeName.isEmpty()) {
      fullMemberName.prepend((scopeName + sep));
   }

   if (isEnumerate() && ! title.isEmpty() ) {

      if (title.at(0) == '@') {
         ldef = title = "anonymous enum";

         if (! m_impl->enumBaseType.isEmpty()) {
            ldef += " : " + m_impl->enumBaseType;
         }

      } else {
         ldef.prepend("enum ");
      }

   } else if (isEnumValue() && ! ldef.isEmpty()) {

      if (ldef.at(0) == '@') {
         ldef = ldef.mid(2);
      }

   } else if (isFunction()) {
      title += "()";

   }

   ol.pushGeneratorState();

   //
   ldef = renameNS_Aliases(ldef);

   static QRegularExpression regExp("@[0-9]+");
   QRegularExpressionMatch match = regExp.match(ldef);

   int matchLen;
   bool htmlEndLabelTable = false;

   QStringList sl;
   getLabels(sl, scopedContainer);

   if ((isVariable() || isTypedef()) && match.hasMatch()) {
      // find enum type and insert it in the definition
      bool found = false;

      for (auto vmd : *ml) {
         if (found) {
            break;
         }

         if (vmd->isEnumerate() && match.captured() == vmd->name()) {
            ol.startDoxyAnchor(cfname, cname, memAnchor, fullMemberName, memberArgs);
            ol.startMemberDoc(ciname, name(), memAnchor, name(), showInline);

            linkifyText(TextGeneratorOLImpl(ol), scopedContainer, getBodyDef(), self, QStringView(ldef.constBegin(), match.capturedStart()));

            vmd->writeEnumDeclaration(ol, getClassDef(), getNamespaceDef(), getFileDef(), getGroupDef());
            linkifyText(TextGeneratorOLImpl(ol), scopedContainer, getBodyDef(), self, QStringView(match.capturedEnd(), ldef.constEnd()));

            found = true;
         }
      }

      if (! found) {
         // anonymous compound

         ol.startDoxyAnchor(cfname, cname, memAnchor, fullMemberName, memberArgs);
         ol.startMemberDoc(ciname, name(), memAnchor, name(), showInline);

         // search for the last anonymous compound name in the definition
         auto iter_s = ldef.indexOfFast(' ');

         if (iter_s == ldef.constEnd()) {
            iter_s = ldef.constBegin();
         }

         QString::const_iterator iter_e = match.capturedEnd();
         match = regExp.match(ldef, iter_e);

         while (match.hasMatch()) {
            iter_e  = match.capturedEnd();
            match   = regExp.match(ldef, iter_e);
         }

         // first si characters of ldef contain compound type name
         ol.startMemberDocName(isObjCMethod());
         ol.docify(QStringView(ldef.constBegin(), iter_s));
         ol.docify(" { ... } ");

         // last ei characters of ldef contain pointer/reference specifiers
         auto iter_n = ldef.indexOfFast("::", iter_s);

         if (iter_n >= iter_e) {
            iter_e = iter_n + 2;
         }

         linkifyText(TextGeneratorOLImpl(ol), scopedContainer, getBodyDef(), self, QStringView(iter_e, ldef.constEnd()));
      }

   } else {
      // not an enum value or anonymous compound
      // write member name and documentation

      ol.startDoxyAnchor(cfname, cname, memAnchor, fullMemberName, memberArgs);
      ol.startMemberDoc(ciname, name(), memAnchor, title, showInline);

      QSharedPointer<ClassDef> cd     = getClassDef();
      QSharedPointer<NamespaceDef> nd = getNamespaceDef();

      if (! hideScopeNames) {
         bool first = true;

         if (! m_impl->m_defaultTemplateArgLists.isEmpty() && lang == SrcLangExt_Cpp) {

            // definition has explicit template parameter declarations
            for (auto &tal : m_impl->m_defaultTemplateArgLists) {

               if (tal.count() > 0) {
                  if (! first) {
                     ol.docify(" ");
                  }

                  ol.startMemberDocPrefixItem();
                  writeTemplatePrefix(ol, tal);
                  ol.endMemberDocPrefixItem();
               }
            }

         } else if (lang == SrcLangExt_Cpp)  {
            // definition gets template parameters from its class
            // (since no definition was found)

            if (cd && ! isTemplateSpecialization()) {

               QVector<ArgumentList> tempParamLists;
               cd->getTemplateParameterLists(tempParamLists);

               for (auto &tal : tempParamLists) {

                  if (tal.count() > 0) {

                     if (! first) {
                        ol.docify(" ");
                     }

                     ol.startMemberDocPrefixItem();
                     writeTemplatePrefix(ol, tal);
                     ol.endMemberDocPrefixItem();
                  }
               }
            }

            if (! m_impl->m_templateArgList.listEmpty()) {
               // function template prefix

               ol.startMemberDocPrefixItem();
               writeTemplatePrefix(ol, m_impl->m_templateArgList);
               ol.endMemberDocPrefixItem();
            }
         }
      }

      if (sl.count() > 0) {
         ol.pushGeneratorState();
         ol.disableAll();

         ol.enable(OutputGenerator::Html);
         ol.writeString("<table class=\"mlabels\">\n");
         ol.writeString("  <tr>\n");
         ol.writeString("  <td class=\"mlabels-left\">\n");
         ol.popGeneratorState();
         htmlEndLabelTable = true;
      }

      ol.startMemberDocName(isObjCMethod());

      if (cd && cd->isObjectiveC()) {
         // strip scope name
         int ep = ldef.indexOf("::");

         if (ep != -1) {
            int sp = ldef.lastIndexOf(' ', ep);
            if (sp != -1) {
               ldef = ldef.left(sp + 1) + ldef.mid(ep + 2);
            } else {
               ldef = ldef.mid(ep + 2);
            }
         }

         // strip keywords
         int dp = ldef.indexOf(':');

         if (dp != -1) {
            ldef = ldef.left(dp + 1);
         }

         int l = ldef.length();
         int i = l - 1;

         while (i >= 0 && (isId(ldef.at(i)) || ldef.at(i) == ':')) {
            i--;
         }

         while (i >= 0 && ldef.at(i).isSpace() ) {
            i--;
         }

         if (i > 0) {
            // insert braces around the type
            QString tmp("(" + ldef.left(i + 1) + ")" + ldef.mid(i + 1));
            ldef = tmp;
         }

         if (isStatic()) {
            ldef.prepend("+ ");
         } else {
            ldef.prepend("- ");
         }
      }

      linkifyText(TextGeneratorOLImpl(ol), scopedContainer, getBodyDef(), self, substitute(ldef, "::", sep));

      QSharedPointer<Definition> scopeDef = cd;
      if (scopeDef == nullptr) {
         scopeDef = nd;
      }

      // writes argList for member
      hasParameterList = writeDefArgumentList(ol, scopeDef, self);

      if (hasOneLineInitializer()) {
         // add initializer

         if (! isDefine()) {
            ol.docify(" ");

            QString init = m_impl->initializer.simplified();
            linkifyText(TextGeneratorOLImpl(ol), scopedContainer, getBodyDef(), self, init);

         } else {
            ol.writeNonBreakableSpace(3);
            linkifyText(TextGeneratorOLImpl(ol), scopedContainer, getBodyDef(), self, m_impl->initializer);
         }
      }

      if (! excpString().isEmpty() ) {
         // add exception list
         writeExceptionList(ol, cd, self);
         hasParameterList = true;             // call endParameterList below
      }
   }

   // all outputs other than HTML
   ol.pushGeneratorState();
   ol.disable(OutputGenerator::Html);

   if (sl.count() > 0) {
      ol.startLabels();

      auto nextItem = sl.begin();

      for (auto s : sl) {
         ++nextItem;
         ol.writeLabel(s, nextItem != sl.end());
      }

      ol.endLabels();
   }

   ol.popGeneratorState();

   if (hasParameterList) {
      ol.endParameterList();
      ol.endMemberDoc(true);

   } else {
      ol.endMemberDocName();
      ol.endMemberDoc(false);
   }

   // for HTML write the labels here
   ol.pushGeneratorState();
   ol.disableAll();
   ol.enable(OutputGenerator::Html);

   QList<QPair<QString, QString>> showProperties;

   if (htmlEndLabelTable) {
      ol.writeString("  </td>\n");
      ol.writeString("  <td class=\"mlabels-right\">\n");
      ol.startLabels();

      auto nextItem  = sl.begin();

      for (auto s : sl) {
         bool isProperty = false;

         QPair<QString, QString> temp;
         temp.first  = s;
         temp.second = "";

         if (s == "read")  {
            temp.second = getPropertyRead();
            isProperty  = true;

         } else if (s == "write") {
            temp.second = getPropertyWrite();
            isProperty  = true;

         } else if (s == "reset") {
            temp.second = getPropertyReset();
            isProperty  = true;

         } else if (s == "notify") {
            temp.second = getPropertyNotify();
            isProperty  = true;

         } else if (s == "revision") {
            isProperty  = true;

         } else if (s == "designable" || s == "scriptable" || s == "stored" || s == "user") {
            isProperty  = true;

         } else if (s == "constant") {
            isProperty  = true;

         } else if (s == "final_property") {
            temp.first  = "final";
            isProperty  = true;

         }

         ++nextItem;

         if (isProperty) {
            showProperties.append(temp);

         } else {
            ol.writeLabel(s, nextItem != sl.end());

         }
      }

      ol.endLabels();
      ol.writeString("  </td>\n");
      ol.writeString("  </tr>\n");
      ol.writeString("</table>\n");
   }
   ol.writeString("</div>");
   ol.popGeneratorState();

   //
   ol.endDoxyAnchor(cfname, memAnchor);
   ol.startIndent();

   _writeGroupInclude(ol, inGroup);

   // write multi-line initializer (if any)
   if (hasMultiLineInitializer()) {
      ol.startBold();

      if (m_impl->mtype == MemberType_Define) {
         ol.parseText(theTranslator->trDefineValue());
      } else {
         ol.parseText(theTranslator->trInitialValue());
      }

      ol.endBold();

      ParserInterface *pIntf = Doxy_Globals::parserManager.getParser(getDefFileExtension());
      pIntf->resetCodeParserState();

      ol.startCodeFragment();

      if (m_impl->initializer.startsWith("=")) {
         m_impl->initializer = m_impl->initializer.mid(1).trimmed();
      }

      pIntf->parseCode(ol, scopeName, m_impl->initializer, lang, false, 0, getFileDef(), -1, -1, true, self, false, self);
      ol.endCodeFragment();
   }

   QString brief    = briefDescription();
   QString detailed = documentation();

   const ArgumentList *docArgList = &m_impl->m_defArgList;      // ok as a ptr

   if (m_impl->templateMaster) {
      brief      = m_impl->templateMaster->briefDescription();
      detailed   = m_impl->templateMaster->documentation();
      docArgList = &m_impl->templateMaster->getArgumentList();
   }

   /* write brief description */
   if (! brief.isEmpty() && (repeatBrief || ! briefMemberDesc) ) {
      ol.startParagraph();

      ol.generateDoc(briefFile(), briefLine(), scopedContainer, self,
                  brief, false, false, "", true, false);

      ol.endParagraph();
   }

   if (! detailed.isEmpty() || ! inbodyDocumentation().isEmpty()) {
      // write detailed description and inbody docs

      ol.generateDoc(docFile(), docLine(), scopedContainer, self,
                  detailed + "\n", true, false);

      if (! inbodyDocumentation().isEmpty()) {
         ol.generateDoc(inbodyFile(), inbodyLine(), scopedContainer, self,
                  inbodyDocumentation() + "\n", true, false);
      }

   } else if (! brief.isEmpty() && (repeatBrief || ! briefMemberDesc)) {
      // inbody docs

      if (! inbodyDocumentation().isEmpty()) {
         ol.generateDoc(inbodyFile(), inbodyLine(), scopedContainer, self,
                  inbodyDocumentation() + "\n", true, false);
      }
   }

   if (docArgList != 0 && docArgList->hasDocumentation()) {
      QString paramDocs;

      // convert the parameter documentation into a list of @param commands
      for (auto &a : *docArgList) {
         if (a.hasDocumentation()) {
            QString direction = extractDirection(a.docs);
            paramDocs += "@param" + direction + " " + a.name + " " + a.docs;
         }
      }

      // feed the result to the documentation parser
      ol.generateDoc(docFile(), docLine(), scopedContainer,
         self,         // memberDef
         paramDocs,    // docStr
         true,         // indexWords
         false         // isExample
      );
   }

   // HTML only, write a property table
   ol.pushGeneratorState();
   ol.disableAll();
   ol.enable(OutputGenerator::Html);

   if (showProperties.size() > 0 ) {
      // start table for properties
      ol.writeString("<table class=\"fieldtable\">\n");
      ol.writeString("  <tr><th>" + theTranslator->trProperties() + "</th><th>" + theTranslator->trClassMethods() + "</th></tr>\n");

      for (auto iter = showProperties.begin(); iter != showProperties.end(); ++iter) {

         QPair<QString, QString> temp = *iter;

         ol.writeString("  <tr><td class=\"fieldname\">");

         ol.writeString(temp.first);
         ol.writeString("  </td><td class=\"fielddoc\">");

         ol.writeString(temp.second);
         ol.writeString("  </td></tr>\n");
      }

      ol.writeString("</table>\n");
   }
   ol.popGeneratorState();

   //
   _writeEnumValues(ol, scopedContainer, cfname, ciname, cname);
   _writeReimplements(ol);
   _writeReimplementedBy(ol);
   _writeCategoryRelation(ol);
   _writeExamples(ol);
   _writeTypeConstraints(ol);

   writeSourceDef(ol, cname);
   writeSourceRefs(ol, cname);
   writeSourceReffedBy(ol, cname);
   writeInlineCode(ol, cname);

   _writeCallGraph(ol);
   _writeCallerGraph(ol);

   if (Doxy_Globals::userComments) {
      ol.pushGeneratorState();
      ol.disableAllBut(OutputGenerator::Html);

      QString cmd = "<? $root=$_SERVER['DOCUMENT_ROOT']; "
                       "passthru(\"$root/doxynotes --lookup " +
                       getOutputFileBase() + ":" + anchor() + "\") ?>";

      ol.writeString(cmd);
      ol.popGeneratorState();
   }

   ol.endIndent();

   // enable LaTeX again
   // if Config::getBool("extract-all") && ! hasDocs) ol.enable(OutputGenerator::Latex);
   ol.popGeneratorState();

   warnIfUndocumentedParams();
}

// strip scope and field name from the type
// example: "struct N::S.v.c" will become "struct v"
static QString simplifyTypeForTable(const QString &s)
{
   QString ts = removeAnonymousScopes(s);

   if (ts.endsWith("::")) {
      ts = ts.left(ts.length() - 2);
   }

   static QRegularExpression regExp("[A-Z_a-z0-9]+::");
   QRegularExpressionMatch match = regExp.match(ts);

   int matchLen;

   while (match.hasMatch()) {

      QStringView a = QStringView(ts.constBegin(), match.capturedStart());
      QStringView b = QStringView(match.capturedEnd(), ts.constEnd());

      ts = a + b;
      match = regExp.match(ts);
   }

   int i = ts.lastIndexOf('.');
   if (i != -1) {
      ts = ts.left(i);
   }

   i = ts.lastIndexOf('.');
   if (i != -1) {
      ts = ts.right(ts.length() - i - 1);
   }

   return ts;
}

QString MemberDef::fieldType() const
{
   QString type = m_impl->accessorType;

   if (type.isEmpty()) {
      type = m_impl->type;
   }

   if (isTypedef()) {
      type.prepend("typedef ");
   }
   return simplifyTypeForTable(type);
}

void MemberDef::writeMemberDocSimple(OutputList &ol, QSharedPointer<Definition> container)
{
   QSharedPointer<MemberDef> self       = sharedFrom(this);
   QSharedPointer<Definition> scopeDef  = getOuterScope();

   QString doxyName  = name();
   QString doxyArgs  = argsString();
   QString memAnchor = anchor();
   QString cfname    = getOutputFileBase();
   QString cname;

   if (scopeDef) {
      cname = scopeDef->name();
   }

   if (doxyName.startsWith('@')) {
      doxyName = "__unnamed__";
   }

   QSharedPointer<ClassDef> cd = m_impl->accessorClass;

   if (container && container->definitionType()==Definition::TypeClass &&
         ! container.dynamicCast<ClassDef>()->isJavaEnum()) {

      ol.startInlineMemberType();
      ol.startDoxyAnchor(cfname, cname, memAnchor, doxyName, doxyArgs);

      QString ts = fieldType();

      if (cd)  {
         // cd points to an anonymous struct pointed to by this member, add a link to it from the type column
         int i = 0;

         if (ts.startsWith("struct ")) {
            ol.writeString(ts);
            i = 7;

         } else if (ts.startsWith("union ")) {
            ol.writeString(ts);
            i = 6;

         } else if (ts.startsWith("class ")) {
            ol.writeString(ts);
            i = 6;
         }

         ol.writeObjectLink(cd->getReference(), cd->getOutputFileBase(), cd->anchor(), ts.mid(i));

      } else {
         // use standard auto linking
         linkifyText(TextGeneratorOLImpl(ol), scopeDef, getBodyDef(), self, ts);
      }

      ol.endDoxyAnchor(cfname, memAnchor);
      ol.endInlineMemberType();
   }

   ol.startInlineMemberName();
   ol.docify(doxyName);

   if (isVariable() && ! argsString().isEmpty() && ! isObjCMethod()) {
      linkifyText(TextGeneratorOLImpl(ol), getOuterScope(), getBodyDef(), self, argsString());
   }

   if (! m_impl->bitfields.isEmpty()) { // add bitfields
      linkifyText(TextGeneratorOLImpl(ol), getOuterScope(), getBodyDef(), self, m_impl->bitfields);
   }
   ol.endInlineMemberName();

   ol.startInlineMemberDoc();

   QString brief     = briefDescription();
   QString detailed  = documentation();

   /* write brief description */
   if (! brief.isEmpty()) {
      ol.generateDoc(briefFile(), briefLine(), getOuterScope() ? getOuterScope() : container, self,
                     brief, false, false, "", true, false);
   }

   /* write detailed description */
   if (! detailed.isEmpty()) {
      ol.generateDoc(docFile(), docLine(), getOuterScope() ? getOuterScope() : container, self,
                     detailed + "\n", false, false, "", false, false);

   }

   ol.endInlineMemberDoc();
}

QString MemberDef::memberTypeName() const
{
   QString retval;

   switch (m_impl->mtype) {
      case MemberType_Define:
         retval = "macro definition";
         break;

      case MemberType_Function:
         retval = "function";
         break;

      case MemberType_Variable:
         retval = "variable";
         break;

      case MemberType_Typedef:
         retval = "typedef";
         break;

      case MemberType_Enumeration:
         retval = "enumeration";
         break;

      case MemberType_EnumValue:
         retval = "enumvalue";
         break;

      case MemberType_Signal:
         retval = "signal";
         break;

      case MemberType_Slot:
         retval = "slot";
         break;

      case MemberType_DCOP:
         retval = "dcop";
         break;

      case MemberType_Property:
         retval = "property";
         break;

      case MemberType_Event:
         retval = "event";
         break;

      case MemberType_Interface:
         retval = "interface";
         break;

      case MemberType_Service:
         retval = "service";
         break;

      default:
         retval = "unknown";
   }

   if (isFriend()) {
      retval.prepend("friend ");
   }

   return retval;
}

void MemberDef::warnIfUndocumented()
{
   static const bool extractAll = Config::getBool("extract-all");

   QSharedPointer<ClassDef>     cd = getClassDef();
   QSharedPointer<NamespaceDef> nd = getNamespaceDef();
   QSharedPointer<FileDef>      fd = getFileDef();
   QSharedPointer<GroupDef>     gd = getGroupDef();

   QSharedPointer<Definition> def;

   QString type;

   if (cd) {
      type = "class";
      def  = cd;

   } else if (nd) {

      if (nd->getLanguage() == SrcLangExt_Fortran) {
         type = "module";
      } else {
         type = "namespace";
      }

      def = nd;

   } else if (gd) {
      type = "group";
      def  = gd;

   } else {
      type = "file";
      def  = fd;

   }

   if ((! hasUserDocumentation() && ! extractAll) && ! isFriendClass() && name().indexOf('@') == -1 &&
                  def != nullptr && def->name().indexOf('@') == -1 &&
                  protectionLevelVisible(m_impl->prot) && ! isReference() && ! isDeleted() ) {

      QString returnType = typeString();
      QString memberType = memberTypeName();

      if (returnType.isEmpty() && memberType == "function" && type == "class") {
         // constructor or destructor

         warn_undoc(getDefFileName(),getDefLine(), "Member %s::%s%s is undocumented.",
                 csPrintable(def->name()), csPrintable(name()), csPrintable(argsString()));

     } else if (! returnType.isEmpty() && (memberType == "function" || memberType == "signal" || memberType == "slot")
               && type == "class") {
         // class method

         warn_undoc(getDefFileName(),getDefLine(), "Member %s %s::%s%s (%s) is undocumented.",
                 csPrintable(returnType), csPrintable(def->name()), csPrintable(name()), csPrintable(argsString()),
                 csPrintable(memberType));

      } else {
        warn_undoc(getDefFileName(),getDefLine(), "Member %s %s%s (%s) in %s %s is undocumented.",
                 csPrintable(returnType), csPrintable(name()), csPrintable(argsString()), csPrintable(memberType),
                 csPrintable(type), csPrintable(def->name()));
      }

   } else if (! isDetailedSectionLinkable()) {
      warnIfUndocumentedParams();

  }
}

void MemberDef::warnIfUndocumentedParams()
{
   static const bool extractAll     = Config::getBool("extract-all");
   static const bool warnUndoc      = Config::getBool("warn-undoc");
   static const bool warnUndocParam = Config::getBool("warn-undoc-param");

   if (! extractAll && warnUndoc &&  warnUndocParam && ! isReference() && ! Doxy_Globals::suppressDocWarnings) {

      if (!hasDocumentedParams()) {
         warn_doc_error(getDefFileName(), getDefLine(), "Parameters for member %s are not fully documented",
                     csPrintable(qualifiedName()));
      }

      if (!hasDocumentedReturnType() && isFunction() && hasDocumentation()) {
         warn_doc_error(getDefFileName(), getDefLine(), "Return type of member %s is not documented",
                     csPrintable(qualifiedName()));
      }
   }
}

bool MemberDef::isFriendClass() const
{
   return (isFriend() &&  (m_impl->type == "friend class" || m_impl->type == "friend struct" ||
                  m_impl->type == "friend union"));
}

bool MemberDef::isDocumentedFriendClass() const
{
   QSharedPointer<ClassDef> fcd;

   QString baseName = name();
   int i = baseName.indexOf('<');

   if (i != -1) {
      baseName = baseName.left(i);
   }

   return (isFriendClass() && (fcd = getClass(baseName)) && fcd->isLinkable());
}

bool MemberDef::isDeleted() const
{
   return (m_impl->m_defArgList.isDeleted);
}

bool MemberDef::hasDocumentation() const
{
   return Definition::hasDocumentation() ||
          (m_impl->mtype == MemberType_Enumeration && m_impl->docEnumValues) ||    // has enum values
          m_impl->m_defArgList.hasDocumentation();                                 // has doc arguments
}

void MemberDef::setMemberGroup(QSharedPointer<MemberGroup> grp)
{
   m_impl->memberGroup = grp;
}

bool MemberDef::visibleMemberGroup(bool hideNoHeader)
{
   return m_impl->memberGroup != 0 && (! hideNoHeader || m_impl->memberGroup->header() != "[NOHEADER]");
}

QString MemberDef::getScopeString() const
{
   QString result;

   if (getClassDef()) {
      result = getClassDef()->displayName();

   } else if (getNamespaceDef()) {
      result = getNamespaceDef()->displayName();

   }

   return result;
}

void MemberDef::setAnchor()
{
   QString memAnchor = name();

   if (! m_impl->m_args.isEmpty()) {
      memAnchor += m_impl->m_args;
   }

   memAnchor.prepend(definition());

   // method name is now included twice, currently done for backward compatibility

   // include number of template arguments as well, to distinguish between two template
   // specializations that only differ in the template parameters

   if (! m_impl->m_templateArgList.listEmpty())   {
      QString tmp = QString("%1:").formatArg(m_impl->m_templateArgList.count());
      memAnchor.prepend(tmp);
   }

   // convert to md5 hash
   QByteArray data = QCryptographicHash::hash(memAnchor.toUtf8(), QCryptographicHash::Md5).toHex();
   QString sigStr  = QString::fromLatin1(data);

   m_impl->anc = "a" + sigStr;
}

void MemberDef::setGroupDef(QSharedPointer<GroupDef> gd, Grouping::GroupPri_t pri, const QString &fileName,
                  int startLine, bool hasDocs, QSharedPointer<MemberDef> member)
{
   m_impl->group = gd;
   m_impl->grouppri = pri;
   m_impl->groupFileName  = fileName;
   m_impl->groupStartLine = startLine;
   m_impl->groupHasDocs = hasDocs;
   m_impl->groupMember  = member;
   m_isLinkableCached   = 0;
}

void MemberDef::setEnumScope(QSharedPointer<MemberDef> md, bool livesInsideEnum)
{
   m_impl->enumScope = md;
   m_impl->livesInsideEnum = livesInsideEnum;

   if (md->getGroupDef()) {
      m_impl->group = md->getGroupDef();
      m_impl->grouppri = md->getGroupPri();
      m_impl->groupFileName = md->getGroupFileName();
      m_impl->groupStartLine = md->getGroupStartLine();
      m_impl->groupHasDocs = md->getGroupHasDocs();
      m_isLinkableCached = 0;
   }
}

void MemberDef::setMemberClass(QSharedPointer<ClassDef> cd)
{
   m_impl->classDef      = cd;
   m_isLinkableCached    = 0;
   m_isConstructorCached = 0;

   setOuterScope(cd);
}

void MemberDef::setNamespace(QSharedPointer<NamespaceDef> nd)
{
   m_impl->nspace = nd;

   setOuterScope(nd);
}

QSharedPointer<MemberDef> MemberDef::createTemplateInstanceMember(const ArgumentList &formalArgs,
                  const ArgumentList &actualArgs)
{
   ArgumentList actualArgList;
   actualArgList = m_impl->m_defArgList;

   // replace formal arguments with actuals
   for (auto &arg : actualArgList) {
      arg.type = substituteTemplateArgumentsInString(arg.type, formalArgs, actualArgs);
   }

   actualArgList.trailingReturnType = substituteTemplateArgumentsInString(actualArgList.trailingReturnType,
                  formalArgs, actualArgs);

   QString methodName = name();

   if (methodName.startsWith("operator ")) {
      // conversion operator
      methodName = substituteTemplateArgumentsInString(methodName, formalArgs, actualArgs);
   }

   QSharedPointer<MemberDef> imd = QMakeShared<MemberDef>(getDefFileName(), getDefLine(), getDefColumn(),
         substituteTemplateArgumentsInString(m_impl->type, formalArgs, actualArgs), methodName,
         substituteTemplateArgumentsInString(m_impl->m_args, formalArgs, actualArgs),
         m_impl->exception, m_impl->prot, m_impl->virt, m_impl->stat, m_impl->m_related, m_impl->mtype,
         ArgumentList(), ArgumentList());

   imd->setArgumentList(actualArgList);
   imd->setDefinition(substituteTemplateArgumentsInString(m_impl->def, formalArgs, actualArgs));
   imd->setBodyDef(getBodyDef());
   imd->setBodySegment(getStartBodyLine(), getEndBodyLine());

   return imd;
}

bool MemberDef::hasOneLineInitializer() const
{
   return !m_impl->initializer.isEmpty() && m_impl->initLines == 0 && // one line initializer
          ((m_impl->maxInitLines > 0 && m_impl->userInitLines == -1) || m_impl->userInitLines > 0); // enabled by default or explicitly
}

bool MemberDef::hasMultiLineInitializer() const
{
   return m_impl->initLines > 0 &&  ((m_impl->initLines < m_impl->maxInitLines && m_impl->userInitLines == -1)
           || m_impl->initLines < m_impl->userInitLines );
}

void MemberDef::setInitializer(const QString &initializer)
{
   m_impl->initializer = initializer;

   int l = m_impl->initializer.length();
   int p = l - 1;

   while (p >= 0 && m_impl->initializer.at(p).isSpace() ) {
      p--;
   }

   m_impl->initializer = m_impl->initializer.left(p + 1);
   m_impl->initLines   = m_impl->initializer.count('\n');
}

void MemberDef::addListReference(QSharedPointer<Definition> d)
{
   static bool optimizeOutputForC = Config::getBool("optimize-c");

   // static bool hideScopeNames     = Config::getBool("hide-scope-names");
   // static bool optimizeOutputJava = Config::getBool("optimize-java");
   // static bool fortranOpt         = Config::getBool("optimize-fortran");

   SrcLangExt lang = getLanguage();

   visited = true;
   if (!isLinkableInProject()) {
      return;
   }

   QString memLabel;

   if (optimizeOutputForC) {
      memLabel = theTranslator->trGlobal(true, true);

   } else if (lang == SrcLangExt_Fortran) {
      memLabel = theTranslator->trSubprogram(true, true);

   } else {
      memLabel = theTranslator->trMember(true, true);
   }

   QString memName = name();
   QSharedPointer<Definition> pd = getOuterScope();

   QString pdName = pd->definitionType() == Definition::TypeClass ?
                       pd.dynamicCast<ClassDef>()->displayName() : pd->name();

   QString sep = getLanguageSpecificSeparator(lang, true);
   QString memArgs;

   if (! isRelated() ) {

      if (isObjCMethod()) {
         memName = "[" + pd->name() + " " + name() + "]";

      } else {
         if (pd != Doxy_Globals::globalScope) {
            memName.prepend(pdName + sep);
         }

         memArgs = argsString();
      }
   }

   const QVector<ListItemInfo> &xrefItems = getRefItems();

   if (! xrefItems.isEmpty()) {
      // argsString is needed for overloaded functions

      addRefItem(xrefItems, qualifiedName() + argsString(), memLabel, getOutputFileBase() + "#" + anchor(),
                  memName, memArgs, pd);
   }
}

QSharedPointer<MemberList> MemberDef::getSectionList(QSharedPointer<Definition> d) const
{
   return m_impl->classSectionSDict.value(d);
}

void MemberDef::setSectionList(QSharedPointer<Definition> d, QSharedPointer<MemberList> sl)
{
   m_impl->classSectionSDict.insert(d, sl);
}

Specifier MemberDef::virtualness(int count) const
{
   if (count > 25) {
      warn(getDefFileName(), getDefLine(),
           "Internal inconsistency: recursion detected in overload relation for member %s", csPrintable(name()));

      return Normal;
   }

   Specifier v = m_impl->virt;
   QSharedPointer<MemberDef> rmd = reimplements();

   while (rmd && v == Normal) {
      v = rmd->virtualness(count + 1) == Normal ? Normal : Virtual;
      rmd = rmd->reimplements();
   }
   return v;
}

void MemberDef::writeTagFile(QTextStream &tagFile)
{
   if (! isLinkableInProject()) {
      return;
   }

   tagFile << "    <member kind=\"";

   if (isFriend()) {
      tagFile << "friend";
   }

   switch (m_impl->mtype) {
      case MemberType_Define:
         tagFile << "define";
         break;

      case MemberType_EnumValue:
         tagFile << "enumvalue";
         break;

      case MemberType_Property:
         tagFile << "property";
         break;

      case MemberType_Event:
         tagFile << "event";
         break;

      case MemberType_Variable:
         tagFile << "variable";
         break;

      case MemberType_Typedef:
         tagFile << "typedef";
         break;

      case MemberType_Enumeration:
         tagFile << "enumeration";
         break;

      case MemberType_Function:
         tagFile << "function";
         break;

      case MemberType_Signal:
         tagFile << "signal";
         break;

      case MemberType_DCOP:
         tagFile << "dcop";
         break;

      case MemberType_Slot:
         tagFile << "slot";
         break;

      case MemberType_Interface:
         tagFile << "interface";
         break;

      case MemberType_Service:
         tagFile << "service";
         break;
   }

   if (m_impl->prot != Public) {
      tagFile << "\" protection=\"";
      if (m_impl->prot == Protected) {
         tagFile << "protected";
      } else if (m_impl->prot == Package) {
         tagFile << "package";
      } else { /* Private */
         tagFile << "private";
      }
   }

   if (m_impl->virt != Normal) {
      tagFile << "\" virtualness=\"";
      if (m_impl->virt == Virtual) {
         tagFile << "virtual";
      } else { /* Pure */
         tagFile << "pure";
      }
   }

   if (isStatic()) {
      tagFile << "\" static=\"yes";
   }

   tagFile << "\">" << endl;

   if (typeString() != "@") {
      tagFile << "      <type>" << convertToXML(typeString()) << "</type>" << endl;
   }

   tagFile << "      <name>" << convertToXML(name()) << "</name>" << endl;
   tagFile << "      <anchorfile>" << convertToXML(getOutputFileBase() + Doxy_Globals::htmlFileExtension)
           << "</anchorfile>" << endl;

   tagFile << "      <anchor>" << convertToXML(anchor()) << "</anchor>" << endl;

   QString idStr = id();
   if (! idStr.isEmpty()) {
      tagFile << "      <clangid>" << convertToXML(idStr) << "</clangid>" << endl;
   }

   tagFile << "      <arglist>" << convertToXML(argsString()) << "</arglist>" << endl;

   if (isStrong()) {
      QSharedPointer<MemberList> fmdl = m_impl->enumFields;

      if (fmdl) {
         for (auto fmd : *fmdl) {
            if (! fmd->isReference()) {
               tagFile << "      <enumvalue file=\"" << convertToXML(getOutputFileBase() + Doxy_Globals::htmlFileExtension);
               tagFile << "\" anchor=\"" << convertToXML(fmd->anchor());

               QString idStr = fmd->id();
               if (! idStr.isEmpty()) {
                  tagFile << "\" clangid=\"" << convertToXML(idStr);
               }

               tagFile  << "\">" << convertToXML(fmd->name()) << "</enumvalue>" << endl;
            }
         }
      }
   }

   writeDocAnchorsToTagFile(tagFile);
   tagFile << "    </member>" << endl;
}

void MemberDef::computeIsConstructor() const
{
   m_isConstructorCached = 1;

   if (m_impl->classDef) {
      if (m_impl->isDMember) { // for D
         m_isConstructorCached = name() == "this" ? 2 : 1;
         return;

      } else if (getLanguage() == SrcLangExt_PHP) { // for PHP
         m_isConstructorCached = name() == "__construct" ? 2 : 1;
         return;

      } else if (name() == "__init__" &&
                 getLanguage() == SrcLangExt_Python) { // for Python
         m_isConstructorCached = 2; // true
         return;

      } else if (getLanguage() == SrcLangExt_Tcl) { // for Tcl
         m_isConstructorCached = name() == "constructor" ? 2 : 1;
         return;

      } else { // for other languages
         QString locName = m_impl->classDef->localName();
         int i = locName.indexOf('<');

         if (i == -1) { // not a template class
            m_isConstructorCached = name() == locName ? 2 : 1;
         } else {
            m_isConstructorCached = name() == locName.left(i) ? 2 : 1;
         }

         return;
      }
   }
}

bool MemberDef::isConstructor() const
{
   if (m_isConstructorCached == 0) {
      computeIsConstructor();
   }
   assert(m_isConstructorCached > 0);

   return m_isConstructorCached == 2;
}

void MemberDef::computeIsDestructor() const
{
   bool isDestructor;

   if (m_impl->isDMember) { // for D
      isDestructor = name() == "~this";

   } else if (getLanguage() == SrcLangExt_PHP) { // for PHP
      isDestructor = name() == "__destruct";

   } else if (getLanguage() == SrcLangExt_Tcl) { // for Tcl
      isDestructor = name() == "destructor";

   } else if (name() == "__del__" &&
              getLanguage() == SrcLangExt_Python) { // for Python
      isDestructor = true;

   } else { // other languages
      isDestructor = (name().indexOf('~') != -1 || name().indexOf('!') != -1) && name().indexOf("operator") == -1;
       // The ! is for C++/CLI
   }

   m_isDestructorCached = isDestructor ? 2 : 1;
}

bool MemberDef::isDestructor() const
{
   if (m_isDestructorCached == 0) {
      computeIsDestructor();
   }

   assert(m_isDestructorCached > 0);

   return m_isDestructorCached == 2;
}

void MemberDef::writeEnumDeclaration(OutputList &typeDecl, QSharedPointer<ClassDef> cd,
                  QSharedPointer<NamespaceDef> nd, QSharedPointer<FileDef> fd, QSharedPointer<GroupDef> gd)
{
   int enumMemCount = 0;

   QSharedPointer<MemberList> fmdl = m_impl->enumFields;
   uint numVisibleEnumValues = 0;

   if (fmdl) {
      for (auto fmd : *fmdl) {
         if (fmd->isBriefSectionVisible()) {
            numVisibleEnumValues++;
         }
      }
   }

   if (numVisibleEnumValues == 0 && !isBriefSectionVisible()) {
      return;
   }

   QString n = name();
   int i = n.lastIndexOf("::");
   if (i != -1) {
      n = n.right(n.length() - i - 2);   // strip scope (TODO: is this needed?)
   }

   if (n[0] != '@') {
      // not an anonymous enum

      if (isLinkableInProject() || hasDocumentedEnumValues()) {
         //_writeTagData(compoundType);
         _addToSearchIndex();
         writeLink(typeDecl, cd, nd, fd, gd);

      } else {
         typeDecl.startBold();
         typeDecl.docify(n);
         typeDecl.endBold();
      }
      typeDecl.writeChar(' ');
   }

   if (!m_impl->enumBaseType.isEmpty()) {
      typeDecl.writeChar(':');
      typeDecl.writeChar(' ');
      typeDecl.docify(m_impl->enumBaseType);
      typeDecl.writeChar(' ');
   }

   int enumValuesPerLine = Config::getInt("enum-values-per-line");

   if (numVisibleEnumValues > 0 && enumValuesPerLine > 0) {
      typeDecl.docify("{ ");

      if (fmdl) {

         auto mli = fmdl->begin();
         QSharedPointer<MemberDef> fmd = *mli;

         bool fmdVisible = fmd ? fmd->isBriefSectionVisible() : true;

         while (mli != fmdl->end()) {

            if (fmdVisible) {
               /* in html we start a new line after a number of items */

               if (numVisibleEnumValues > enumValuesPerLine && (enumMemCount % enumValuesPerLine) == 0 ) {
                  typeDecl.pushGeneratorState();
                  typeDecl.disableAllBut(OutputGenerator::Html);
                  typeDecl.enable(OutputGenerator::Latex);
                  typeDecl.lineBreak();
                  typeDecl.disable(OutputGenerator::Latex);
                  typeDecl.writeString("&#160;&#160;");
                  typeDecl.popGeneratorState();
               }

               if (fmd->hasDocumentation()) { // enum value has docs
                  //fmd->_writeTagData(compoundType);
                  fmd->_addToSearchIndex();
                  fmd->writeLink(typeDecl, cd, nd, fd, gd);

               } else {
                  // no docs for this enum value
                  typeDecl.startBold();
                  typeDecl.docify(fmd->name());
                  typeDecl.endBold();
               }

               if (fmd->hasOneLineInitializer()) { // enum value has initializer
                  //typeDecl.writeString(" = ");
                  typeDecl.writeString(" ");
                  typeDecl.parseText(fmd->initializer());
               }
            }

            bool prevVisible = fmdVisible;

            ++mli;

            if (mli != fmdl->end()) {
               fmd = *mli;

               if (fmdVisible = fmd->isBriefSectionVisible()) {
                  typeDecl.writeString(", ");
               }

            }

            if (prevVisible) {
               typeDecl.disable(OutputGenerator::Man);
               typeDecl.writeString("\n"); // to prevent too long lines in LaTeX
               typeDecl.enable(OutputGenerator::Man);
               enumMemCount++;
            }
         }

         if (numVisibleEnumValues > enumValuesPerLine) {
            typeDecl.pushGeneratorState();
            typeDecl.disableAllBut(OutputGenerator::Html);
            typeDecl.lineBreak();
            typeDecl.popGeneratorState();
         }
      }
      typeDecl.docify(" }");
   }
}

void MemberDef::setArgumentList(const ArgumentList &al)
{
   m_impl->m_defArgList = al;
}

void MemberDef::setDeclArgumentList(const ArgumentList &al)
{
   m_impl->m_declArgList = al;
}

void MemberDef::setTypeConstraints(const ArgumentList &al)
{
   m_impl->m_typeConstraints = al;
}

void MemberDef::setTemplateParameterLists(const QVector<ArgumentList> &lists)
{
   m_impl->m_defaultTemplateArgLists = lists;
}

void MemberDef::setType(const QString &t)
{
   m_impl->type = t;
}

void MemberDef::setAccessorType(QSharedPointer<ClassDef> cd, const QString &t)
{
   m_impl->accessorClass = cd;
   m_impl->accessorType  = t;
}

QSharedPointer<ClassDef> MemberDef::accessorClass() const
{
   return m_impl->accessorClass;
}

void MemberDef::findSectionsInDocumentation()
{
   QSharedPointer<MemberDef> self = sharedFrom(this);
   docFindSections(documentation(), self, QSharedPointer<MemberGroup>(), docFile());
}

void MemberDef::enableCallGraph(bool e)
{
   m_impl->hasCallGraph = e;

   if (e) {
      Doxy_Globals::parseSourcesNeeded = true;
   }
}

void MemberDef::enableCallerGraph(bool e)
{
   m_impl->hasCallerGraph = e;
   if (e) {
      Doxy_Globals::parseSourcesNeeded = true;
   }
}

bool MemberDef::isObjCMethod() const
{
   if (m_impl->classDef && m_impl->classDef->isObjectiveC() && isFunction()) {
      return true;
   }
   return false;
}

bool MemberDef::isObjCProperty() const
{
   if (m_impl->classDef && m_impl->classDef->isObjectiveC() && isProperty()) {
      return true;
   }
   return false;
}

QString MemberDef::qualifiedName() const
{
   if (isObjCMethod()) {
      QString qm;

      if (isStatic()) {
         qm = "+";
      } else {
         qm = "-";
      }
      qm += "[";
      qm += m_impl->classDef->name() + " ";
      qm += name();
      qm += "]";
      return qm;

   } else if (m_impl->enumScope && m_impl->enumScope->isStrong()) {
      return m_impl->enumScope->qualifiedName() +
             getLanguageSpecificSeparator(getLanguage()) +
             localName();
   } else {
      return Definition::qualifiedName();
   }
}

void MemberDef::setTagInfo(const TagInfo &tag)
{
   if (! tag.isEmpty()) {
      m_impl->anc = tag.tag_Anchor;
      setReference(tag.tag_Name);
      m_impl->explicitOutputFileBase = stripExtension(tag.tag_FileName);
   }
}

QString MemberDef::objCMethodName(bool localLink, bool showStatic) const
{
   QString qm;

   if (showStatic) {
      if (isStatic()) {
         qm = "+ ";
      } else {
         qm = "- ";
      }
   }

   qm += name();

   if (!localLink) {
      // link to method of same class
      qm += " (";
      qm += m_impl->classDef->name();
      qm += ")";
   }
   return qm;
}

QString MemberDef::declaration() const
{
   return m_impl->decl;
}

QString MemberDef::definition() const
{
   return m_impl->def;
}

QString MemberDef::extraTypeChars() const
{
   return m_impl->extraTypeChars;
}

QString MemberDef::typeString() const
{
   return m_impl->type;
}

QString MemberDef::argsString() const
{
   return m_impl->m_args;
}

QString MemberDef::excpString() const
{
   return m_impl->exception;
}

QString MemberDef::bitfieldString() const
{
   return m_impl->bitfields;
}

QString MemberDef::initializer() const
{
   return m_impl->initializer;
}

int MemberDef::initializerLines() const
{
   return m_impl->initLines;
}

Entry::Traits MemberDef::getMemberTraits() const
{
   return m_impl->m_memberTraits;
}

QSharedPointer<ClassDef> MemberDef::getClassDef() const
{
   return m_impl->classDef;
}

QSharedPointer<FileDef> MemberDef::getFileDef() const
{
   return m_impl->fileDef;
}

QSharedPointer<NamespaceDef> MemberDef::getNamespaceDef() const
{
   return m_impl->nspace;
}

QString MemberDef::getPropertyRead() const
{
   return m_impl->m_read;
}

QString MemberDef::getPropertyWrite() const
{
   return m_impl->m_write;
}

QString MemberDef::getPropertyReset() const
{
   return m_impl->m_reset;
}

QString MemberDef::getPropertyNotify() const
{
   return m_impl->m_notify;
}

QSharedPointer<GroupDef> MemberDef::getGroupDef() const
{
   return m_impl->group;
}

Grouping::GroupPri_t MemberDef::getGroupPri() const
{
   return m_impl->grouppri;
}

QString MemberDef::getGroupFileName() const
{
   return m_impl->groupFileName;
}

int MemberDef::getGroupStartLine() const
{
   return m_impl->groupStartLine;
}

bool MemberDef::getGroupHasDocs() const
{
   return m_impl->groupHasDocs;
}

Protection MemberDef::protection() const
{
   return m_impl->prot;
}

MemberType MemberDef::memberType() const
{
   return m_impl->mtype;
}

bool MemberDef::isSignal() const
{
   return m_impl->mtype == MemberType_Signal;
}

bool MemberDef::isSlot() const
{
   return m_impl->mtype == MemberType_Slot;
}

bool MemberDef::isVariable() const
{
   return m_impl->mtype == MemberType_Variable;
}

bool MemberDef::isEnumerate() const
{
   return m_impl->mtype == MemberType_Enumeration;
}

bool MemberDef::isEnumValue() const
{
   return m_impl->mtype == MemberType_EnumValue;
}

bool MemberDef::isTypedef() const
{
   return m_impl->mtype == MemberType_Typedef;
}

bool MemberDef::isFunction() const
{
   return m_impl->mtype == MemberType_Function;
}

bool MemberDef::isFunctionPtr() const
{
   return m_impl->mtype == MemberType_Variable && argsString().indexOf(")(") != -1;
}

bool MemberDef::isDefine() const
{
   return m_impl->mtype == MemberType_Define;
}

bool MemberDef::isFriend() const
{
   return m_impl->m_related == Relationship::Friend;
}

bool MemberDef::isDCOP() const
{
   return m_impl->mtype == MemberType_DCOP;
}

bool MemberDef::isProperty() const
{
   return m_impl->mtype == MemberType_Property;
}

bool MemberDef::isEvent() const
{
   return m_impl->mtype == MemberType_Event;
}

bool MemberDef::isRelated() const
{
   return m_impl->m_related == Relationship::Related;
}

bool MemberDef::isForeign() const
{
   return m_impl->m_related == Relationship::Foreign;
}

bool MemberDef::isStatic() const
{
   return m_impl->stat;
}

bool MemberDef::isInline() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Inline);
}

bool MemberDef::isExplicit() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Explicit);
}

bool MemberDef::isDeprecated() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Deprecated);
}

bool MemberDef::isMutable() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Mutable);
}

bool MemberDef::isGettable() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Gettable);
}

bool MemberDef::isPrivateGettable() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::PrivateGettable);
}

bool MemberDef::isProtectedGettable() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::ProtectedGettable);
}

bool MemberDef::isSettable() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Settable);
}

bool MemberDef::isPrivateSettable() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::PrivateSettable);
}

bool MemberDef::isProtectedSettable() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::ProtectedSettable);
}

bool MemberDef::isAddable() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Addable);
}

bool MemberDef::isRemovable() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Removable);
}

bool MemberDef::isRaisable() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Raisable);
}

bool MemberDef::isReadable() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Readable);
}

bool MemberDef::isWritable() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Writable);
}

bool MemberDef::isNotify() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Notify);
}

bool MemberDef::isReset() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Reset);
}

bool MemberDef::isRevision() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Revision);
}

bool MemberDef::isDesignable() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Designable);
}

bool MemberDef::isScriptable() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Scriptable);
}

bool MemberDef::isStored() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Stored);
}

bool MemberDef::isUser() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::User);
}

bool MemberDef::isConstant() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Constant);
}

bool MemberDef::isFinal_Property() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Final_Property);
}

bool MemberDef::isFinal() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Final);
}

bool MemberDef::isNew() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::New);
}

bool MemberDef::isSealed() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Sealed);
}

bool MemberDef::isOverride() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Override);
}

bool MemberDef::isInitonly() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Initonly);
}

bool MemberDef::isAbstract() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Abstract);
}

bool MemberDef::isOptional() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Optional);
}

bool MemberDef::isRequired() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Required);
}

bool MemberDef::isNonAtomic() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::NonAtomic);
}

bool MemberDef::isCopy() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Copy);
}

bool MemberDef::isAssign() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Assign);
}

bool MemberDef::isRetain() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Retain);
}

bool MemberDef::isWeak() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Weak);
}

bool MemberDef::isStrong() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Strong);
}

bool MemberDef::isStrongEnumValue() const
{
   return m_impl->mtype == MemberType_EnumValue && m_impl->enumScope && m_impl->enumScope->isStrong();
}

bool MemberDef::isUnretained() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Unretained);
}

bool MemberDef::isAlias() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Alias);
}

bool MemberDef::isConstExpr() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::ConstExpr);
}

bool MemberDef::isDefault() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Default);
}

bool MemberDef::isDelete() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Delete);
}

bool MemberDef::isNoExcept() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::NoExcept);
}

bool MemberDef::isAttribute() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Attribute);
}

bool MemberDef::isUNOProperty() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Property);
}

bool MemberDef::isReadonly() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Readonly);
}

bool MemberDef::isBound() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Bound);
}

bool MemberDef::isConstrained() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Constrained);
}

bool MemberDef::isTransient() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Transient);
}

bool MemberDef::isMaybeVoid() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::MaybeVoid);
}

bool MemberDef::isMaybeDefault() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::MaybeDefault);
}

bool MemberDef::isMaybeAmbiguous() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::MaybeAmbiguous);
}

bool MemberDef::isPublished() const
{
   return m_impl->m_memberTraits.hasTrait(Entry::Virtue::Published);
}

bool MemberDef::isImplementation() const
{
   return m_impl->implOnly;
}

bool MemberDef::isExternal() const
{
   return m_impl->explExt;
}

bool MemberDef::isTemplateSpecialization() const
{
   return m_impl->tspec;
}

bool MemberDef::hasDocumentedParams() const
{
   return m_impl->hasDocumentedParams;
}

bool MemberDef::hasDocumentedReturnType() const
{
   return m_impl->hasDocumentedReturnType;
}

bool MemberDef::showInCallGraph() const
{
   return isFunction() ||
          isSlot() ||
          isConstructor() ||
          isDestructor() ||
          isObjCMethod();
}

QSharedPointer<ClassDef> MemberDef::relatedAlso() const
{
   return m_impl->relatedAlso;
}

bool MemberDef::hasDocumentedEnumValues() const
{
   return m_impl->docEnumValues;
}

QSharedPointer<MemberDef> MemberDef::getAnonymousEnumType() const
{
   return m_impl->annEnumType;
}

bool MemberDef::isDocsForDefinition() const
{
   return m_impl->docsForDefinition;
}

QSharedPointer<MemberDef> MemberDef::getEnumScope() const
{
   return m_impl->enumScope;
}

QSharedPointer<MemberList> MemberDef::enumFieldList() const
{
   return m_impl->enumFields;
}

bool MemberDef::isPrototype() const
{
   return m_impl->proto;
}

const ArgumentList &MemberDef::getArgumentList() const
{
   return m_impl->m_defArgList;
}

ArgumentList &MemberDef::getArgumentList()
{
   return m_impl->m_defArgList;
}

const ArgumentList &MemberDef::getDeclArgumentList() const
{
   return m_impl->m_declArgList;
}

const ArgumentList &MemberDef::getTypeConstraints() const
{
   return m_impl->m_typeConstraints;
}

const ArgumentList &MemberDef::getTemplateArgumentList() const
{
   return m_impl->m_templateArgList;
}

ArgumentList &MemberDef::getTemplateArgumentList()
{
   return m_impl->m_templateArgList;
}

const QVector<ArgumentList> &MemberDef::getTemplateParameterLists() const
{
   return m_impl->m_defaultTemplateArgLists;
}

int MemberDef::getMemberGroupId() const
{
   return m_impl->grpId;
}

QSharedPointer<MemberGroup> MemberDef::getMemberGroup() const
{
   return m_impl->memberGroup;
}

bool MemberDef::fromAnonymousScope() const
{
   return m_impl->annScope;
}

bool MemberDef::anonymousDeclShown() const
{
   return m_impl->annUsed;
}

void MemberDef::setAnonymousUsed()
{
   m_impl->annUsed = true;
}

bool MemberDef::hasCallGraph() const
{
   return m_impl->hasCallGraph;
}

bool MemberDef::hasCallerGraph() const
{
   return m_impl->hasCallerGraph;
}

QSharedPointer<MemberDef> MemberDef::templateMaster() const
{
   return m_impl->templateMaster;
}

bool MemberDef::isTypedefValCached() const
{
   return m_impl->isTypedefValCached;
}

QSharedPointer<ClassDef> MemberDef::getCachedTypedefVal() const
{
   return m_impl->cachedTypedefValue;
}

QString MemberDef::getCachedTypedefTemplSpec() const
{
   return m_impl->cachedTypedefTemplSpec;
}

QString MemberDef::getCachedResolvedTypedef() const
{
   return m_impl->cachedResolvedType;
}

QSharedPointer<MemberDef> MemberDef::memberDefinition() const
{
   return m_impl->memDef;
}

QSharedPointer<MemberDef> MemberDef::memberDeclaration() const
{
   return m_impl->memDec;
}

QSharedPointer<MemberDef> MemberDef::inheritsDocsFrom() const
{
   return m_impl->docProvider;
}

QSharedPointer<MemberDef> MemberDef::getGroupAlias() const
{
   return m_impl->groupAlias;
}

void MemberDef::setMemberType(MemberType t)
{
   m_impl->mtype      = t;
   m_isLinkableCached = 0;
}

void MemberDef::setDefinition(const QString &d)
{
   m_impl->def = d;
}

void MemberDef::setFileDef(QSharedPointer<FileDef> fd)
{
   m_impl->fileDef = fd;
   m_isLinkableCached = 0;
   m_isConstructorCached = 0;
   m_isDestructorCached = 0;
}

void MemberDef::setProtection(Protection p)
{
   m_impl->prot = p;
   m_isLinkableCached = 0;
}

void MemberDef::setMemberTraits(Entry::Traits traits)
{
   m_impl->m_memberTraits = traits;
}

void MemberDef::mergeMemberTraits(Entry::Traits traits)
{
   m_impl->m_memberTraits |= traits;
}

void MemberDef::setBitfields(const QString &s)
{
   m_impl->bitfields = s.simplified();
}

void MemberDef::setMaxInitLines(int lines)
{
   if (lines != -1) {
      m_impl->userInitLines = lines;
   }
}

void MemberDef::setExplicitExternal(bool b)
{
   m_impl->explExt = b;
}

void MemberDef::setPropertyRead(const QString &data)
{
   m_impl->m_read = data;
}

void MemberDef::setPropertyWrite(const QString &data)
{
   m_impl->m_write = data;
}

void MemberDef::setPropertyReset(const QString &data)
{
   m_impl->m_reset = data;
}

void MemberDef::setPropertyNotify(const QString &data)
{
   m_impl->m_notify = data;
}

void MemberDef::setTemplateSpecialization(bool b)
{
   m_impl->tspec = b;
}

void MemberDef::setFriend(bool isFriend)
{
   if (isFriend) {
      m_impl->m_related  = Relationship::Friend;
   }
}

void MemberDef::makeFriend()
{
   m_impl->m_related  = Relationship::Friend;
   m_isLinkableCached = 0;
}

void MemberDef::makeRelated()
{
   m_impl->m_related  = Relationship::Related;
   m_isLinkableCached = 0;
}

void MemberDef::makeForeign()
{
   m_impl->m_related  = Foreign;
   m_isLinkableCached = 0;
}

void MemberDef::setHasDocumentedParams(bool b)
{
   m_impl->hasDocumentedParams = b;
}

void MemberDef::setHasDocumentedReturnType(bool b)
{
   m_impl->hasDocumentedReturnType = b;
}

void MemberDef::setInheritsDocsFrom(QSharedPointer<MemberDef> md)
{
   m_impl->docProvider = md;
}

void MemberDef::setArgsString(const QString &as)
{
   m_impl->m_args = removeRedundantWhiteSpace(as);
}

void MemberDef::setRelatedAlso(QSharedPointer<ClassDef> cd)
{
   m_impl->relatedAlso = cd;
}

void MemberDef::setEnumClassScope(QSharedPointer<ClassDef> cd)
{
   m_impl->classDef      = cd;
   m_isLinkableCached    = 0;
   m_isConstructorCached = 0;
}

void MemberDef::setDocumentedEnumValues(bool value)
{
   m_impl->docEnumValues = value;
}

void MemberDef::setAnonymousEnumType(QSharedPointer<MemberDef> md)
{
   m_impl->annEnumType = md;
}

void MemberDef::setPrototype(bool p)
{
   m_impl->proto = p;
}

void MemberDef::setMemberGroupId(int id)
{
   m_impl->grpId = id;
}

void MemberDef::makeImplementationDetail()
{
   m_impl->implOnly = true;
}

void MemberDef::setFromAnonymousScope(bool b)
{
   m_impl->annScope = b;
}

void MemberDef::setFromAnonymousMember(QSharedPointer<MemberDef> m)
{
   m_impl->annMemb = m;
}

QSharedPointer<MemberDef> MemberDef::fromAnonymousMember() const
{
   return m_impl->annMemb;
}

void MemberDef::setTemplateMaster(QSharedPointer<MemberDef> mt)
{
   m_impl->templateMaster = mt;
   m_isLinkableCached = 0;
}

void MemberDef::setDocsForDefinition(bool b)
{
   m_impl->docsForDefinition = b;
}

void MemberDef::setGroupAlias(QSharedPointer<MemberDef> md)
{
   m_impl->groupAlias = md;
}

void MemberDef::invalidateTypedefValCache()
{
   m_impl->isTypedefValCached = false;
}

void MemberDef::setMemberDefinition(QSharedPointer<MemberDef> md)
{
   m_impl->memDef = md;
}

void MemberDef::setMemberDeclaration(QSharedPointer<MemberDef> md)
{
   m_impl->memDec = md;
}

QSharedPointer<ClassDef> MemberDef::category() const
{
   return m_impl->category;
}

void MemberDef::setCategory(QSharedPointer<ClassDef> def)
{
   m_impl->category = def;
}

QSharedPointer<MemberDef> MemberDef::categoryRelation() const
{
   return m_impl->categoryRelation;
}

void MemberDef::setCategoryRelation(QSharedPointer<MemberDef> md)
{
   m_impl->categoryRelation = md;
}

void MemberDef::setEnumBaseType(const QString &type)
{
   m_impl->enumBaseType = type;
}

QString MemberDef::enumBaseType() const
{
   return m_impl->enumBaseType;
}

void MemberDef::cacheTypedefVal(QSharedPointer<ClassDef> val, const QString &templSpec, const QString &resolvedType)
{
   m_impl->isTypedefValCached = true;
   m_impl->cachedTypedefValue = val;
   m_impl->cachedTypedefTemplSpec = templSpec;
   m_impl->cachedResolvedType = resolvedType;
}

void MemberDef::copyArgumentNames(QSharedPointer<MemberDef> bmd)
{
   // part 1
   const ArgumentList &arguments1 = bmd->getArgumentList();
   auto iter = m_impl->m_defArgList.begin();

   for (auto &argSrc : arguments1) {
      if (iter == m_impl->m_defArgList.end()) {
         break;
      }

      iter->name = argSrc.name;
      ++iter;
   }

   // part 2
   const ArgumentList &arguments2 = bmd->getDeclArgumentList();
   iter = m_impl->m_declArgList.begin();

   for (auto &argSrc : arguments2) {
      if (iter == m_impl->m_declArgList.end()) {
         break;
      }

      iter->name = argSrc.name;
      ++iter;
   }
}

static void invalidateCachedTypesInArgumentList(ArgumentList &al)
{
   for (auto &a : al) {
      a.canType = "";
   }
}

void MemberDef::invalidateCachedArgumentTypes()
{
   invalidateCachedTypesInArgumentList(m_impl->m_defArgList);
   invalidateCachedTypesInArgumentList(m_impl->m_declArgList);
}

void MemberDef::addFlowKeyWord()
{
   m_flow_count++;
}

int MemberDef::numberOfFlowKeyWords() const
{
   return m_flow_count;
}

QString MemberDef::displayName(bool unused) const
{
   return Definition::name();
}

void MemberDef::_addToSearchIndex()
{
   // write search index info
  QSharedPointer<MemberDef> self = sharedFrom(this);

   if (Doxy_Globals::searchIndexBase != nullptr && isLinkableInProject()) {
      Doxy_Globals::searchIndexBase->setCurrentDoc(self, anchor(), false);
      QString ln = localName(), qn = qualifiedName();
      Doxy_Globals::searchIndexBase->addWord(ln, true);

      if (ln != qn) {
         Doxy_Globals::searchIndexBase->addWord(qn, true);

         if (getClassDef()) {
            Doxy_Globals::searchIndexBase->addWord(getClassDef()->displayName(), true);

         } else if (getNamespaceDef()) {
            Doxy_Globals::searchIndexBase->addWord(getNamespaceDef()->displayName(), true);
         }
      }
   }
}

static void transferArgumentDocumentation(ArgumentList &decArgList, ArgumentList &defArgList)
{
   auto iter = defArgList.begin();

   for (auto &decItem : decArgList) {
      auto &defItem = *iter;            // obtains the value of defArgList

      if (decItem.docs.isEmpty() && ! defItem.docs.isEmpty()) {
         decItem.docs = defItem.docs;

      } else if (defItem.docs.isEmpty() && ! decItem.docs.isEmpty()) {
         defItem.docs = decItem.docs;
      }

      ++ iter;

      if (iter == defArgList.end()) {
         break;
      }
   }
}

void combineDeclarationAndDefinition(QSharedPointer<MemberDef> mdec, QSharedPointer<MemberDef> mdef)
{
   if ( (mdef->isFunction() && ! mdef->isStatic() && ! mdef->isPrototype()) ||
        (mdef->isVariable() && ! mdef->isExternal() && !mdef->isStatic()) ) {

      // returns a const reference, copies to a local
      ArgumentList mdefAl = mdef->getArgumentList();
      ArgumentList mdecAl = mdec->getArgumentList();

      if (matchArguments2(mdef->getOuterScope(), mdef->getFileDef(), mdefAl,
                          mdec->getOuterScope(), mdec->getFileDef(), mdecAl, true ) ) {

         // match found, first merge argument documentation
         transferArgumentDocumentation(mdecAl, mdefAl);

         /* copy documentation between function definition and declaration */
         if (! mdec->briefDescription().isEmpty()) {
            mdef->setBriefDescription(mdec->briefDescription(), mdec->briefFile(), mdec->briefLine());

         } else if (! mdef->briefDescription().isEmpty()) {
            mdec->setBriefDescription(mdef->briefDescription(), mdef->briefFile(), mdef->briefLine());

         }

         if (! mdef->documentation().isEmpty()) {
            mdec->setDocumentation(mdef->documentation(), mdef->docFile(), mdef->docLine());
            mdec->setDocsForDefinition(mdef->isDocsForDefinition());

            ArgumentList mdefAlComb = stringToArgumentList(mdef->argsString());

            transferArgumentDocumentation(mdefAl, mdefAlComb);
            mdec->setArgumentList(mdefAlComb);


         } else if (! mdec->documentation().isEmpty()) {
            mdef->setDocumentation(mdec->documentation(), mdec->docFile(), mdec->docLine());
            mdef->setDocsForDefinition(mdec->isDocsForDefinition());

            ArgumentList mdecAlComb = stringToArgumentList(mdec->argsString());

            transferArgumentDocumentation(mdecAl, mdecAlComb);
            mdef->setDeclArgumentList(mdecAlComb);

         }

         if (! mdef->inbodyDocumentation().isEmpty()) {
            mdec->setInbodyDocumentation(mdef->inbodyDocumentation(), mdef->inbodyFile(), mdef->inbodyLine());

         } else if (! mdec->inbodyDocumentation().isEmpty()) {
            mdef->setInbodyDocumentation(mdec->inbodyDocumentation(), mdec->inbodyFile(), mdec->inbodyLine());

         }

         if (mdec->getStartBodyLine() != -1 && mdef->getStartBodyLine() == -1) {

            mdef->setBodySegment(mdec->getStartBodyLine(), mdec->getEndBodyLine());
            mdef->setBodyDef(mdec->getBodyDef());

         } else if (mdef->getStartBodyLine() != -1 && mdec->getStartBodyLine() == -1) {

            mdec->setBodySegment(mdef->getStartBodyLine(), mdef->getEndBodyLine());
            mdec->setBodyDef(mdef->getBodyDef());
         }

         mdec->mergeMemberTraits(mdef->getMemberTraits());
         mdef->mergeMemberTraits(mdec->getMemberTraits());

         // copy group info
         if (mdec->getGroupDef() == nullptr && mdef->getGroupDef() != nullptr) {
            mdec->setGroupDef(mdef->getGroupDef(), mdef->getGroupPri(), mdef->docFile(),
                              mdef->docLine(), mdef->hasDocumentation(), mdef);

         } else if (mdef->getGroupDef() == nullptr && mdec->getGroupDef() != nullptr) {
            mdef->setGroupDef(mdec->getGroupDef(), mdec->getGroupPri(), mdec->docFile(),
                              mdec->docLine(), mdec->hasDocumentation(), mdec);
         }

         mdec->mergeRefItems(mdef);
         mdef->mergeRefItems(mdec);

         mdef->setMemberDeclaration(mdec);
         mdec->setMemberDefinition(mdef);

         mdef->enableCallGraph(mdec->hasCallGraph() || mdef->hasCallGraph());
         mdef->enableCallerGraph(mdec->hasCallerGraph() || mdef->hasCallerGraph());
         mdec->enableCallGraph(mdec->hasCallGraph() || mdef->hasCallGraph());
         mdec->enableCallerGraph(mdec->hasCallerGraph() || mdef->hasCallerGraph());
      }
   }
}

QString MemberDef::briefDescription(bool abbr) const
{
   if (m_impl->templateMaster) {
      return m_impl->templateMaster->briefDescription(abbr);
   } else {
      return Definition::briefDescription(abbr);
   }
}

QString MemberDef::documentation() const
{
   if (m_impl->templateMaster) {
      return m_impl->templateMaster->documentation();
   } else {
      return Definition::documentation();
   }
}

bool MemberDef::isFriendToHide() const
{
   static bool hideFriendCompounds = Config::getBool("hide-friend-compounds");

   bool isFriendToHide = hideFriendCompounds && (m_impl->type == "friend class"  ||
                  m_impl->type == "friend struct" || m_impl->type == "friend union");

   return isFriendToHide;
}

bool MemberDef::isNotFriend() const
{
   return ! (isFriend() && isFriendToHide());
}

bool MemberDef::isFunctionOrSignalSlot() const
{
   return isFunction() || isSlot() || isSignal();
}

bool MemberDef::isRelatedOrFriend() const
{
   return isRelated() || isForeign() || (isFriend() && ! isFriendToHide());
}

bool MemberDef::isReference() const
{
  return Definition::isReference() || (m_impl->templateMaster && m_impl->templateMaster->isReference());
}


// end of the file
