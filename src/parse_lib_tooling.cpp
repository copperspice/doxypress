/************************************************************************
*
* Copyright (c) 2014-2022 Barbara Geller & Ansel Sermersheim
* Copyright (c) 1997-2014 Dimitri van Heesch
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

#include <parse_lib_tooling.h>

#include <config.h>
#include <doxy_globals.h>
#include <default_args.h>
#include <util.h>

static QMap<QString, clang::DeclContext *>       s_parentNodeMap;
static QMultiMap<QString, QSharedPointer<Entry>> s_orphanMap;
static QMap<QString, QSharedPointer<Entry>>      s_conceptMap;
static QSet<QPair<QString, uint>>                s_templateDeclMap;

static int anonNSCount = 0;

static Protection getAccessSpecifier(const clang::Decl *node)
{
   Protection retval = Public;

   switch (node->getAccess()) {

      case clang::AS_none:
         retval = Public;
         break;

      case clang::AS_public:
         retval = Public;
         break;

      case clang::AS_protected:
         retval = Protected;
         break;

      case clang::AS_private:
         retval = Private;
         break;
   }

   return retval;
}

static Protection getAccessSpecifier(const clang::CXXBaseSpecifier *node)
{
   Protection retval = Public;

   switch (node->getAccessSpecifier()) {

      case clang::AS_none:
         retval = Public;
         break;

      case clang::AS_public:
         retval = Public;
         break;

      case clang::AS_protected:
         retval = Protected;
         break;

      case clang::AS_private:
         retval = Private;
         break;
   }

   return retval;
}

static QString getName(const clang::NamedDecl *node)
{
   QString retval = "";

   if (node->getDeclName().isIdentifier()) {

      llvm::StringRef tmp = node->getName();
      retval = QString::fromUtf8(tmp.data(), tmp.size());

   } else {
      // broom - add something here, look at Reid's work

   }

   return retval;
}

static QString getUSR_Decl(const clang::Decl *node)
{
   QString retval;
   llvm::SmallVector<char, 100> buffer;

   if (node->getBeginLoc().isInvalid() && (std::string(node->getDeclKindName()) == "TranslationUnit") ) {
      return QString("TranslationUnit");
   }

   bool ignore = clang::index::generateUSRForDecl(node, buffer);

   if (! ignore) {
      retval = QString::fromUtf8(buffer.data(), buffer.size());
   }

   return retval;
}

static QString getUSR_DeclContext(const clang::DeclContext *node)
{
   QString retval;
   const clang::Decl *d = llvm::dyn_cast<clang::Decl>(node);

   retval = getUSR_Decl(d);

   return retval;
}

static QString getUSR_PP(clang::StringRef srcName, clang::SourceLocation srcLocation, clang::SourceManager &srcManager)
{
   QString retval;
   llvm::SmallVector<char, 100> buffer;

   bool ignore = clang::index::generateUSRForMacro(srcName, srcLocation, srcManager, buffer);

   if (! ignore) {
      retval = QString::fromUtf8(buffer.data(), buffer.size());

      static QRegularExpression regExp("@\\d+@macro@");
      retval.replace(regExp, "@macro@");
   }

   return retval;
}

class DoxyVisitor : public clang::RecursiveASTVisitor<DoxyVisitor>
{
   public:
      explicit DoxyVisitor(clang::ASTContext *context)
         : m_context(context), m_policy(m_context->getLangOpts())  {

         m_policy.PolishForDeclaration = true;
      }

      static QString toQString(const llvm::StringRef &value) {
         return QString::fromUtf8(value.data(), value.size());
      }

      static QString toQString(const clang::DeclarationName &value) {
         return QString::fromUtf8(value.getAsString().c_str());
      }

      QString toQString(const clang::QualType &value) {
         return QString::fromUtf8(value.getAsString(m_policy).c_str());
      }

      virtual bool TraverseDecl(clang::Decl *node)  {
         bool retval = true;

         if (node == nullptr) {
            return retval;
         }

         clang::SourceManager & sourceManager = m_context->getSourceManager();

         if (sourceManager.isWrittenInMainFile(node->getLocation()) || std::string(node->getDeclKindName()) == "TranslationUnit") {
            retval = clang::RecursiveASTVisitor<DoxyVisitor>::TraverseDecl(node);
         }

         return retval;
      }

      virtual bool VisitClassTemplateDecl(clang::ClassTemplateDecl *node) {
         // class templates

         QSharedPointer<Entry> current;

         QString currentUSR = getUSR_Decl(node);

         current = s_entryMap.value(currentUSR);

         if (current == nullptr) {
            QString name = getName(node);

            current = QMakeShared<Entry>();
            s_entryMap.insert(currentUSR, current);

            current->section     = Entry::CLASS_SEC;
            current->m_entryName = name;
         }

         if (node->hasAssociatedConstraints()) {
            const clang::Expr *tExpr = node->getTemplateParameters()->getRequiresClause();

            if (tExpr != nullptr) {

               std::string tString;
               llvm::raw_string_ostream tStream(tString);
               tExpr->printPretty(tStream, 0, m_policy);

               current->setData(EntryKey::Requires_Clause, toQString(tStream.str()));
            }
         }

         return true;
      }

      virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *node) {
         // class, struct, union

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current;

         QString parentUSR  = getUSR_DeclContext(node->getParent());
         QString currentUSR = getUSR_Decl(node);

         current = s_entryMap.value(currentUSR);

         // bool newEntry      = false;
         bool isForwardDecl = false;

         if (current == nullptr) {
            current = QMakeShared<Entry>();
            s_entryMap.insert(currentUSR, current);

            // newEntry = true;
         }

         clang::FullSourceLoc location = m_context->getFullLoc(node->getBeginLoc());
         QString name = getName(node);

         if (node->isClass() ) {

            if (! node->isCompleteDefinition())  {
               isForwardDecl = true;
            }

            current->section     = Entry::CLASS_SEC;
            current->m_entryName = name;

            current->setData(EntryKey::Member_Type,  "class");
            current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

            current->m_srcLang     = SrcLangExt_Cpp;
            current->startLine     = location.getSpellingLineNumber();
            current->startColumn   = location.getSpellingColumnNumber();

            current->startBodyLine = current->startLine;
            current->endBodyLine   = m_context->getFullLoc(node->getEndLoc()).getSpellingLineNumber();

            if (true) {    //  (! isForwardDecl) {
               if (node->hasAttr<clang::FinalAttr>())  {
                  current->m_traits.setTrait(Entry::Virtue::Final);
               }

               clang::ClassTemplatePartialSpecializationDecl *specialNode =
                     llvm::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(node);

               if (specialNode != nullptr) {
                  // partial

                  std::string tString;
                  llvm::raw_string_ostream tStream(tString);

                  auto tmpArgs = specialNode->getTemplateArgsAsWritten();
                  clang::printTemplateArgumentList(tStream, tmpArgs->arguments(), m_policy);
                  current->m_entryName += toQString(tStream.str());

               } else {
                  clang::ClassTemplateSpecializationDecl *tmpSpecialNode =
                     llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(node);

                  if (tmpSpecialNode != nullptr) {
                     // full

                     std::string tString;
                     llvm::raw_string_ostream tStream(tString);

                     tmpSpecialNode->getNameForDiagnostic(tStream, m_policy, true);
                     current->m_entryName = toQString(tStream.str());
                  }
               }

               if (! isForwardDecl) {
                  // inheritance
                  for (auto &item : node->bases()) {
                     QString itemName = toQString(item.getType());

                     Protection protection = getAccessSpecifier(&item);
                     Specifier virtualType = Specifier::Normal;

                     if (item.isVirtual()) {
                        virtualType = Specifier::Virtual;
                     }

                     auto tmpBase = BaseInfo(itemName, protection, virtualType);
                     current->extends.append(tmpBase);
                  }
               }
            }

            if (parentUSR.isEmpty() || parentUSR == "TranslationUnit")  {

//             if (newEntry) {
                  s_current_root->addSubEntry(current);
//             }

            } else {
               // nested class
               parentEntry = s_entryMap.value(parentUSR);

               if (parentEntry != nullptr) {
                  current->m_entryName.prepend(parentEntry->m_entryName + "::");
                  current->protection = getAccessSpecifier(node);

                  parentEntry->addSubEntry(current);
               }
            }

         } else if (node->isStruct()) {

            current->section     = Entry::CLASS_SEC;
            current->m_entryName = name;

            current->setData(EntryKey::Member_Type,  "struct");
            current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

            current->m_srcLang     = SrcLangExt_Cpp;
            current->startLine     = location.getSpellingLineNumber();
            current->startColumn   = location.getSpellingColumnNumber();

            current->startBodyLine = current->startLine;
            current->endBodyLine   = m_context->getFullLoc(node->getEndLoc()).getSpellingLineNumber();

            current->m_traits.setTrait(Entry::Virtue::Struct);

            if (node->hasAttr<clang::FinalAttr>())  {
               current->m_traits.setTrait(Entry::Virtue::Final);
            }

            if (parentUSR.isEmpty() || parentUSR == "TranslationUnit")  {
               s_current_root->addSubEntry(current);

            } else {
               // nested struct
               parentEntry = s_entryMap.value(parentUSR);

               if (parentEntry != nullptr) {
                  current->m_entryName.prepend(parentEntry->m_entryName + "::");
                  current->protection = getAccessSpecifier(node);

                  parentEntry->addSubEntry(current);
               }
            }

         } else if (node->isUnion()) {

            current->section     = Entry::CLASS_SEC;
            current->m_entryName = name;

            current->setData(EntryKey::Member_Type,  " union");
            current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

            current->m_srcLang     = SrcLangExt_Cpp;
            current->startLine     = location.getSpellingLineNumber();
            current->startColumn   = location.getSpellingColumnNumber();

            current->startBodyLine = current->startLine;
            current->endBodyLine   = m_context->getFullLoc(node->getEndLoc()).getSpellingLineNumber();

            current->m_traits.setTrait(Entry::Virtue::Union);

            if (parentUSR.isEmpty() || parentUSR == "TranslationUnit")  {
               s_current_root->addSubEntry(current);

            } else {
               // nested union
               parentEntry = s_entryMap.value(parentUSR);

               if (parentEntry != nullptr) {
                  current->m_entryName.prepend(parentEntry->m_entryName + "::");
                  current->protection = getAccessSpecifier(node);

                  parentEntry->addSubEntry(current);
               }
            }
         }

         return true;
      }

      virtual bool VisitFunctionTemplateDecl(clang::FunctionTemplateDecl *node) {
         // function or method templates

         QSharedPointer<Entry> current;

         QString currentUSR = getUSR_Decl(node);

         current = s_entryMap.value(currentUSR);

         if (current == nullptr) {
            QString name = getName(node);

            current = QMakeShared<Entry>();
            s_entryMap.insert(currentUSR, current);

            current->section     = Entry::FUNCTION_SEC;
            current->m_entryName = name;

            // used to mark this entry is a template
            current->startBodyLine = -2;
         }

         if (node->hasAssociatedConstraints()) {
            const clang::Expr *tExpr = node->getTemplateParameters()->getRequiresClause();

            if (tExpr != nullptr) {

               std::string tString;
               llvm::raw_string_ostream tStream(tString);
               tExpr->printPretty(tStream, 0, m_policy);

               current->setData(EntryKey::Requires_Clause, toQString(tStream.str()));
            }
         }

         return true;
      }

      virtual bool VisitFunctionDecl(clang::FunctionDecl *node) {
         // method, function, constructor, destructor, conversion

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current;

         QString parentUSR  = getUSR_DeclContext(node->getParent());
         QString currentUSR = getUSR_Decl(node);

         current = s_entryMap.value(currentUSR);

         if (! parentUSR.isEmpty()) {
            parentEntry = s_entryMap.value(parentUSR);
         }

         if (current == nullptr) {
            current = QMakeShared<Entry>();
            s_entryMap.insert(currentUSR, current);

         } else if (current->startBodyLine != -2) {

            return true;
         }

         clang::FullSourceLoc location    = m_context->getFullLoc(node->getBeginLoc());
         clang::CXXMethodDecl *methodDecl = llvm::dyn_cast<clang::CXXMethodDecl>(node);

         QString name = toQString(node->getNameInfo().getName());
         QString returnType;

         clang::CXXConstructorDecl *declA = llvm::dyn_cast<clang::CXXConstructorDecl>(node);
         clang::CXXDestructorDecl  *declB = llvm::dyn_cast<clang::CXXDestructorDecl>(node);
         clang::CXXConversionDecl  *declC = llvm::dyn_cast<clang::CXXConversionDecl>(node);

         if (declA == nullptr && declB == nullptr && declC == nullptr) {
            // must be a method or function
            returnType = toQString(node->getReturnType());
         }

         // remove the scope from the return type
         QString scopeName = parentEntry->m_entryName + "::";
         int scopeLen = scopeName.length();

         if (returnType.startsWith(scopeName))  {
            returnType = returnType.midView(scopeLen);
         }

         QString args = "(";
         ArgumentList argList;

         bool addComma = false;

         for (auto item : node->parameters())  {

            if (addComma) {
               args += ", ";
            } else {
               addComma = true;
            }

            auto tQualType  = item->getOriginalType();

            QString tmpType = toQString(tQualType);
            QString tmpName = toQString(item->getName());
            QString tmpAfter;

            {
               // simulate printBefore printAfter
               std::string tString;
               llvm::raw_string_ostream tStream(tString);
               llvm::Twine twine("\n");

               clang::QualType::print(tQualType.split(), tStream, m_policy, twine);
               QString tmpStr1 = toQString(tStream.str());

               int pos = tmpStr1.indexOf("\n");

               if (pos != tmpStr1.length() - 1)  {
                  tmpType  = tmpStr1.left(pos);
                  tmpAfter = tmpStr1.mid(pos+1);

                  // handle these case by case
                  tmpAfter.replace("__attribute__((thiscall))", "");
               }
            }

            tmpType.remove("(anonymous namespace)::");

            if (tmpAfter.contains("...")) {
               // adjust location of parameter pack
               tmpType += "... ";
               tmpAfter.replace("...", "");
            }

            // remove the scope from a parameter data type
            if (tmpType.startsWith(scopeName))  {
               tmpType = tmpType.midView(scopeLen);
            }

            args += tmpType + " " + tmpName + tmpAfter;

            QString tmpDefValue;
            auto tExpr = item->getDefaultArg();

            if (tExpr != nullptr) {
               // get the real value
         	
               std::string tString;
               llvm::raw_string_ostream tStream(tString);
               tExpr->printPretty(tStream, 0, m_policy);

               tmpDefValue = toQString(tStream.str());
               args += "=" + tmpDefValue;
            }

            Argument tmp;
            tmp.type   = tmpType;
            tmp.name   = tmpName;
            tmp.defval = tmpDefValue; 	
            tmp.array  = tmpAfter;

            argList.append(tmp);
         }

         args += ")" ;

         // attributes
         if (node->hasAttrs())  {
            clang::AttrVec attributes = node->getAttrs();

            for (clang::Attr *item : attributes) {
               QByteArray itemName = item->getSpelling();

               if (itemName == "deprecated") {
                  current->m_traits.setTrait(Entry::Virtue::Deprecated);

               } else if (itemName == "nodiscard") {
                  current->m_traits.setTrait(Entry::Virtue::NoDiscard);

               } else if (itemName == "noreturn") {
                  current->m_traits.setTrait(Entry::Virtue::NoReturn);

               }
            }
         }

         if (node->isConstexpr())  {
            returnType.prepend("constexpr ");
            current->m_traits.setTrait(Entry::Virtue::ConstExpr);
         }

         if ( (declA != nullptr && declA->isExplicit()) || (declC != nullptr && declC->isExplicit()) )   {
            // constructor, conversion

            returnType.prepend("explicit ");
            current->m_traits.setTrait(Entry::Virtue::Explicit);
         }

         if (node->isInlined())  {
            current->m_traits.setTrait(Entry::Virtue::Inline);
         }

         if (node->isDeleted())  {
            args += " = delete ";
            argList.isDeleted = true;
            current->m_traits.setTrait(Entry::Virtue::Delete);
         }

         if (node->isDefaulted())  {
            args += " = default ";
            current->m_traits.setTrait(Entry::Virtue::Default);
         }

         if (node->hasAttr<clang::FinalAttr>())  {
            args += " final ";
            current->m_traits.setTrait(Entry::Virtue::Final);
         }

         // noexcept
         const clang::FunctionProtoType *protoType = node->getType()->getAs<clang::FunctionProtoType>();
         clang::ExceptionSpecificationType noExceptValue = protoType->getExceptionSpecType();

         switch (noExceptValue) {

           case clang::ExceptionSpecificationType::EST_NoThrow:
               args += " noexcept ";
               current->m_traits.setTrait(Entry::Virtue::NoExcept);
               break;

           case clang::ExceptionSpecificationType::EST_DependentNoexcept: {
               const clang::Expr *tExpr = protoType->getNoexceptExpr();

               std::string tString;
               llvm::raw_string_ostream tStream(tString);
               tExpr->printPretty(tStream, 0, m_policy);

               args += " noexcept(" + toQString(tStream.str()) + ")";
               current->m_traits.setTrait(Entry::Virtue::NoExcept);

               break;
           }

           case clang::ExceptionSpecificationType::EST_None:
           case clang::ExceptionSpecificationType::EST_DynamicNone:
           case clang::ExceptionSpecificationType::EST_Dynamic:
           case clang::ExceptionSpecificationType::EST_MSAny:
           case clang::ExceptionSpecificationType::EST_BasicNoexcept:
           case clang::ExceptionSpecificationType::EST_NoexceptFalse:
           case clang::ExceptionSpecificationType::EST_NoexceptTrue:
           case clang::ExceptionSpecificationType::EST_Unevaluated:
           case clang::ExceptionSpecificationType::EST_Uninstantiated:
           case clang::ExceptionSpecificationType::EST_Unparsed:
               break;
         }

         returnType.remove("(anonymous namespace)::");

         if (methodDecl == nullptr) {
            // function

            current->section     = Entry::FUNCTION_SEC;
            current->m_entryName = name;

            current->setData(EntryKey::Member_Type, returnType);
            current->setData(EntryKey::Member_Args, args);
            current->setData(EntryKey::File_Name,   toQString(location.getManager().getFilename(location)));

            current->argList       = argList;

            current->m_srcLang     = SrcLangExt_Cpp;
            current->startLine     = location.getSpellingLineNumber();
            current->startColumn   = location.getSpellingColumnNumber();

            current->startBodyLine = current->startLine;
            current->endBodyLine   = m_context->getFullLoc(node->getEndLoc()).getSpellingLineNumber();

            if (parentEntry) {
               parentEntry->addSubEntry(current);

            } else {
               s_current_root->addSubEntry(current);

            }

         } else {
            // method

            if (methodDecl->isStatic())  {
               returnType.prepend("static ");
               current->stat = true;
            }

            if (methodDecl->isPure())  {
               returnType.prepend(" virtual ");
               current->virt = Specifier::Pure;

               args += " = 0";
               argList.pureSpecifier = true;

            } else if (methodDecl->isVirtual())  {
               returnType.prepend(" virtual ");
               current->virt = Specifier::Virtual;

            }

            // value quaifiers
            if (methodDecl->getRefQualifier() == clang::RQ_RValue) {
               args  += " && ";
               argList.refSpecifier = RefType::RValueRef;

            } else if (methodDecl->getRefQualifier() == clang::RQ_LValue)  {
               args  += " & ";
               argList.refSpecifier = RefType::LValueRef;
            }

            // const qualifier
            if (methodDecl->isConst())  {
               args += " const ";
               argList.constSpecifier = true;
            }

            // override qualifier
            if (methodDecl->size_overridden_methods() != 0)  {
               args += " override ";
               current->m_traits.setTrait(Entry::Virtue::Override);
            }

            clang::Qualifiers qualifiers = methodDecl->getMethodQualifiers();

            if (qualifiers.hasRestrict())  {
               args += " restrict ";
               // not used - argList.restrictSpecifier = true;
            }

            if (qualifiers.hasVolatile())  {
               args += " volatile ";
               argList.volatileSpecifier = true;
            }

            current->section     = Entry::FUNCTION_SEC;
            current->m_entryName = name;

            current->setData(EntryKey::Member_Type,  returnType);
            current->setData(EntryKey::Member_Args,  args);
            current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

            current->argList       = argList;
            current->protection    = getAccessSpecifier(node);

            current->m_srcLang     = SrcLangExt_Cpp;
            current->startLine     = location.getSpellingLineNumber();
            current->startColumn   = location.getSpellingColumnNumber();

            current->startBodyLine = current->startLine;
            current->endBodyLine   = m_context->getFullLoc(node->getEndLoc()).getSpellingLineNumber();

            if (parentEntry) {
               parentEntry->addSubEntry(current);

            } else {
               // hold until we visit the parent
               s_orphanMap.insert(parentUSR, current);

            }
         }

         return true;
      }

      virtual bool VisitConceptDecl(clang::ConceptDecl *node) {
         // concept

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         QString parentUSR;
         QString currentUSR = getUSR_Decl(node);

         s_entryMap.insert(currentUSR, current);

         clang::FullSourceLoc location = m_context->getFullLoc(node->getBeginLoc());
         QString name = getName(node);

         //
         auto constraintExpr = node->getConstraintExpr();

         std::string tString;
         llvm::raw_string_ostream tStream(tString);
         constraintExpr->printPretty(tStream, 0, m_policy);

         QString constraintStr = toQString(tStream.str());

         //
         current->section     = Entry::CONCEPTDOC_SEC;
         current->m_entryName = name;

         current->setData(EntryKey::Member_Type,  "concept");
         current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

         current->m_srcLang     = SrcLangExt_Cpp;
         current->startLine     = location.getSpellingLineNumber();
         current->startColumn   = location.getSpellingColumnNumber();

         current->startBodyLine = current->startLine;
         current->endBodyLine   = m_context->getFullLoc(node->getEndLoc()).getSpellingLineNumber();

         current->setData(EntryKey::Constraint,  constraintStr);

         clang::TemplateParameterList *list = node->getTemplateParameters();

         for (clang::NamedDecl *item : *list) {
            QString usrDecl = getUSR_Decl(item);
            s_conceptMap.insert(usrDecl, current);
         }

         s_current_root->addSubEntry(current);

         return true;
      }

      virtual bool VisitFieldDecl(clang::FieldDecl *node) {
         // class member

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         QString parentUSR = getUSR_Decl(node->getParent());
         parentEntry = s_entryMap.value(parentUSR);

         clang::FullSourceLoc location = m_context->getFullLoc(node->getBeginLoc());

         auto tQualType = node->getType();
         QString name   = getName(node);

         // handle array
         QString args;
         const clang::ConstantArrayType *cat = m_context->getAsConstantArrayType(tQualType);

         while (cat != nullptr) {
            auto size = cat->getSize().getLimitedValue();
            args += "[" + QString::number(size) + "]";

            tQualType = cat->getElementType();
            cat       = m_context->getAsConstantArrayType(tQualType);
         }

         QString type = toQString(tQualType);

         // remove the scope from the data type
         QString scopeName = parentEntry->m_entryName + "::";
         int scopeLen = scopeName.length();

         if (type.startsWith(scopeName))  {
            type = type.midView(scopeLen);
         }

         current->section     = Entry::VARIABLE_SEC;
         current->m_entryName = name;

         current->setData(EntryKey::Member_Type,  type);
         current->setData(EntryKey::Member_Args,  args);
         current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

         current->protection    = getAccessSpecifier(node);

         current->m_srcLang     = SrcLangExt_Cpp;
         current->startLine     = location.getSpellingLineNumber();
         current->startColumn   = location.getSpellingColumnNumber();

         current->startBodyLine = current->startLine;
         current->endBodyLine   = m_context->getFullLoc(node->getEndLoc()).getSpellingLineNumber();

         if (node->isMutable()) {
            current->prependData(EntryKey::Member_Type, "mutable ");
            current->m_traits.setTrait(Entry::Virtue::Mutable);
         }

         QString currentUSR = getUSR_Decl(node);
         s_entryMap.insert(currentUSR, current);

         parentEntry->addSubEntry(current);

         return true;
      }

      virtual bool VisitEnumDecl(clang::EnumDecl *node) {
         // enum

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current;

         QString parentUSR  = getUSR_DeclContext(node->getParent());
         QString currentUSR = getUSR_Decl(node);

         current = s_entryMap.value(currentUSR);

         if (current == nullptr) {
            current = QMakeShared<Entry>();
            s_entryMap.insert(currentUSR, current);
         }

         clang::FullSourceLoc location = m_context->getFullLoc(node->getBeginLoc());
         QString name = toQString(node->getNameAsString());

         QString className;

         parentEntry = s_entryMap.value(parentUSR);

         if (parentEntry) {
            className = parentEntry->m_entryName;

            if (! name.isEmpty()) {
               // do not test for an anonymous enum

               for (auto entry : parentEntry->children() ) {
                  if (entry->m_entryName == className + name) {
                   return true;
                  }
               }
            }
         }

         current->section     = Entry::ENUM_SEC;
         current->m_entryName = className + "::" + name;

         current->setData(EntryKey::Member_Type,  "enum");
         current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

         current->protection    = getAccessSpecifier(node);
         current->m_srcLang     = SrcLangExt_Cpp;
         current->startLine     = location.getSpellingLineNumber();
         current->startColumn   = location.getSpellingColumnNumber();

         current->startBodyLine = current->startLine;
         current->endBodyLine   = m_context->getFullLoc(node->getEndLoc()).getSpellingLineNumber();

         if (node->isScoped()) {
            current->m_traits.setTrait(Entry::Virtue::Strong);
         }

         if (parentEntry) {
            parentEntry->addSubEntry(current);

         } else {
            // hold until we visit the parent
            s_orphanMap.insert(parentUSR, current);

         }

         return true;
      }

      virtual bool VisitEnumConstantDecl(clang::EnumConstantDecl *node) {
         // enum values

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current;

         QString parentUSR  = getUSR_DeclContext(node->getDeclContext());
         QString currentUSR = getUSR_Decl(node);

         if (current == nullptr) {
            current = QMakeShared<Entry>();
            s_entryMap.insert(currentUSR, current);
         }

         clang::FullSourceLoc location = m_context->getFullLoc(node->getBeginLoc());
         QString name = toQString(node->getNameAsString());

         parentEntry = s_entryMap.value(parentUSR);

         if (parentEntry == nullptr) {
            // may need to review
            return true;
         }

         current->section     = Entry::VARIABLE_SEC;
         current->m_entryName = name;

         current->setData(EntryKey::Member_Type,  "@");
         current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

         current->protection    = getAccessSpecifier(node);

         current->m_srcLang     = SrcLangExt_Cpp;
         current->startLine     = location.getSpellingLineNumber();
         current->startColumn   = location.getSpellingColumnNumber();

         current->startBodyLine = current->startLine;
         current->endBodyLine   = m_context->getFullLoc(node->getEndLoc()).getSpellingLineNumber();

         auto tExpr = node->getInitExpr();

         if (tExpr != nullptr) {
            // get the real value
      	
            std::string tString;
            llvm::raw_string_ostream tStream(tString);
            tExpr->printPretty(tStream, 0, m_policy);

            current->setData(EntryKey::Initial_Value, " = " + toQString(tStream.str()));
         }

         parentEntry->addSubEntry(current);

         //
         bool isStrong = parentEntry->m_traits.hasTrait(Entry::Virtue::Strong);

         if (! isStrong)  {
            auto tmpEntry = parentEntry->parent();

            if (tmpEntry) {
               tmpEntry->addSubEntry(current);
            }
         }

         return true;
      }

      virtual bool VisitVarDecl(clang::VarDecl *node) {
         // global variable

         if (! node->hasGlobalStorage() || node->isStaticLocal())  {
            return true;
         }

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         QString parentUSR  = getUSR_DeclContext(node->getDeclContext());
         QString currentUSR = getUSR_Decl(node);

         s_entryMap.insert(currentUSR, current);

         if (! parentUSR.isEmpty()) {
            parentEntry = s_entryMap.value(parentUSR);
         }

         clang::FullSourceLoc location = m_context->getFullLoc(node->getBeginLoc());

         QString name  = toQString(node->getNameAsString());
         QString type  = toQString(node->getType());

         if (! parentEntry->m_entryName.isEmpty()) {
            // remove a redundant scope from the return type

            QString scopeName = parentEntry->m_entryName + "::";
            type = type.remove(scopeName);
         }

         current->section       = Entry::VARIABLE_SEC;
         current->m_entryName   = name;

         current->setData(EntryKey::Member_Type,  type);
         current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

         current->protection    = getAccessSpecifier(node);

         current->m_srcLang     = SrcLangExt_Cpp;
         current->startLine     = location.getSpellingLineNumber();
         current->startColumn   = location.getSpellingColumnNumber();

         current->startBodyLine = current->startLine;
         current->endBodyLine   = m_context->getFullLoc(node->getEndLoc()).getSpellingLineNumber();

         if (node->getStorageClass() == clang::SC_Static) {
            current->prependData(EntryKey::Member_Type,  "static ");
            current->stat     = true;
         }

         if (node->isInline())  {
            current->prependData(EntryKey::Member_Type, "inline ");
            current->m_traits.setTrait(Entry::Virtue::Inline);
         }

         if (node->isConstexpr())  {
            current->prependData(EntryKey::Member_Type, "constexpr ");
            current->m_traits.setTrait(Entry::Virtue::ConstExpr);
         }

         if (parentEntry) {
            parentEntry->addSubEntry(current);

         } else {
            // part of a file
            s_current_root->addSubEntry(current);
         }

         return true;
      }

      virtual bool VisitFriendDecl(clang::FriendDecl *node) {
         // friend

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         QString parentUSR  = getUSR_DeclContext(node->getDeclContext());
         parentEntry = s_entryMap.value(parentUSR);

         clang::FullSourceLoc location = m_context->getFullLoc(node->getBeginLoc());

         QString name;
         QString returnType;

         QString args;
         ArgumentList argList;

         clang::NamedDecl *friendDecl = node->getFriendDecl();

         if (friendDecl == nullptr) {
            // class

            clang::TypeSourceInfo *friendType = node->getFriendType();

            if (friendType == nullptr) {
               return true;
            }

            QString text = toQString(friendType->getType());
            int index    = text.indexOf(" ");

            name         = text.mid(index + 1);
            returnType   = "friend " + text.left(index);

         } else {
            // method, function

            clang::FunctionDecl *functionDecl = llvm::dyn_cast<clang::FunctionDecl>(friendDecl);

            if (functionDecl == nullptr)  {
               return true;
            }

            name       = getName(functionDecl);
            returnType = "friend " + toQString(functionDecl->getReturnType());

            args = "(";

            bool addComma = false;

            for (auto item : functionDecl->parameters())  {

               if (addComma) {
                  args += ", ";
               } else {
                  addComma = true;
               }

               QString tmpType = toQString(item->getType());
               args += tmpType + " ";

               QString tmpName = toQString(item->getName());
               args += tmpName;

               QString tmpDefValue;
               auto tExpr = item->getDefaultArg();

               if (tExpr != nullptr) {
                  // get the real value (might be ignored by clang)
            	
                  std::string tString;
                  llvm::raw_string_ostream tStream(tString);
                  tExpr->printPretty(tStream, 0, m_policy);

                  tmpDefValue = toQString(tStream.str());
                  args += "=" + tmpDefValue;
               }

               Argument tmp;
               tmp.type   = tmpType;
               tmp.name   = tmpName;
               tmp.defval = tmpDefValue; 	

               argList.append(tmp);
            }

            args += ")";
         }

         current->section     = Entry::FUNCTION_SEC;
         current->m_entryName = name;

         current->setData(EntryKey::Member_Type,  returnType);
         current->setData(EntryKey::Member_Args,  args);
         current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

         current->argList       = argList;
         current->protection    = getAccessSpecifier(node);

         current->m_srcLang     = SrcLangExt_Cpp;
         current->startLine     = location.getSpellingLineNumber();
         current->startColumn   = location.getSpellingColumnNumber();

         current->startBodyLine = current->startLine;
         current->endBodyLine   = m_context->getFullLoc(node->getEndLoc()).getSpellingLineNumber();

         parentEntry->addSubEntry(current);

         return true;
      }

      virtual bool VisitNamespaceDecl(clang::NamespaceDecl *node) {
         // namespace
         static const bool extractAnonNS = Config::getBool("extract-anon-namespaces");

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         clang::FullSourceLoc location = m_context->getFullLoc(node->getBeginLoc());

         QString parentUSR    = getUSR_DeclContext(node->getParent());
         parentEntry          = s_entryMap.value(parentUSR);

         QString currentUSR   = getUSR_Decl(node);
         s_entryMap.insert(currentUSR, current);

         QString name         = getName(node);

         current->section     = Entry::NAMESPACE_SEC;
         current->m_entryName = name;

         current->setData(EntryKey::Member_Type,  "namespace");
         current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

         current->m_srcLang     = SrcLangExt_Cpp;
         current->startLine     = location.getSpellingLineNumber();
         current->startColumn   = location.getSpellingColumnNumber();

         current->startBodyLine = current->startLine;
         current->endBodyLine   = m_context->getFullLoc(node->getEndLoc()).getSpellingLineNumber();

         if (name.isEmpty() ) {
            // anonymous namespace

            if (extractAnonNS) {
               // use visible name
               current->m_entryName = "anonymous_namespace{" + stripPath(current->getData(EntryKey::File_Name)) + "}";

            } else {
               // use invisible name
               current->m_entryName = QString("@%1").formatArg(anonNSCount);
            }
         }

         if (parentEntry == nullptr)  {
            s_current_root->addSubEntry(current);
         } else {
            parentEntry->addSubEntry(current);
         }

         return true;
      }

      virtual bool VisitTemplateTypeParmDecl (clang::TemplateTypeParmDecl *node) {
         // template type

         QSharedPointer<Entry> parentEntry;

         QString parentUSR  = getUSR_DeclContext(node->getDeclContext());
         parentEntry = s_entryMap.value(parentUSR);

         QString currentUSR = getUSR_Decl(node);

         bool isConcept = false;

         if (parentEntry != nullptr && parentUSR == "TranslationUnit") {
            // concept will land here
            parentEntry = s_conceptMap.value(currentUSR);

            if (parentEntry != nullptr) {
               // parentEntry is the "current" entry this template node belongs to
               isConcept = true;
            }
         }

         if ((parentEntry != nullptr && parentUSR != "TranslationUnit") || isConcept) {
            auto parentNode = node->getParentFunctionOrMethod();

            if (s_parentNodeMap.contains(parentUSR) && (s_parentNodeMap.value(parentUSR) != parentNode) ) {
               // already have this template parameter
               return true;
            }

            QPair<QString, uint> key = {parentUSR, node->getIndex()};

            if (s_templateDeclMap.contains(key)) {
               // already have this template parameter, might be a forward declaration
               return true;
            }

            s_parentNodeMap.insert(parentUSR, parentNode);
            s_templateDeclMap.insert(key);

            QSharedPointer<Entry> current = QMakeShared<Entry>();

            s_entryMap.insert(currentUSR, current);

            if (parentEntry->m_templateArgLists.isEmpty()) {
               ArgumentList tmp;
               parentEntry->m_templateArgLists.append(tmp);
            }

            parentEntry->m_traits.setTrait(Entry::Virtue::Template);

            QString type = "class";
            QString name = getName(node);
            QString defValue;

            if (node->wasDeclaredWithTypename())  {
               type = "typename";

            } else if (node->hasTypeConstraint()) {
               auto tc   = node->getTypeConstraint();
               auto expr = tc->getImmediatelyDeclaredConstraint();

               std::string tString;
               llvm::raw_string_ostream tStream(tString);
               expr->printPretty(tStream, 0, m_policy);

               type = toQString(tStream.str());
            }

            if (node->isParameterPack())  {
               type += "...";
            }

            if (node->hasDefaultArgument()) {
               defValue = toQString(node->getDefaultArgument());
            }

            ArgumentList &argList = parentEntry->m_templateArgLists.last();

            Argument tmp;
            tmp.type   = type;
            tmp.name   = name;
            tmp.defval = defValue;

            argList.append(tmp);
         }

         return true;
      }

      virtual bool VisitTypedefNameDecl (clang::TypedefNameDecl *node) {
         // typedef, typeAlias, objective-c type parameter

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         QString parentUSR  = getUSR_DeclContext(node->getDeclContext());
         parentEntry = s_entryMap.value(parentUSR);

         QString currentUSR = getUSR_Decl(node);
         s_entryMap.insert(currentUSR, current);

         clang::FullSourceLoc location = m_context->getFullLoc(node->getBeginLoc());

         QString name = getName(node);

         current->section     = Entry::VARIABLE_SEC;
         current->m_entryName = name;

         current->setData(EntryKey::Member_Type,  "typedef " + toQString(node->getUnderlyingType()));
         current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

         current->protection    = getAccessSpecifier(node);
         current->m_srcLang     = SrcLangExt_Cpp;
         current->startLine     = location.getSpellingLineNumber();
         current->startColumn   = location.getSpellingColumnNumber();

         current->startBodyLine = current->startLine;
         current->endBodyLine   = m_context->getFullLoc(node->getEndLoc()).getSpellingLineNumber();

         if (llvm::dyn_cast<clang::TypeAliasDecl>(node)) {
           current->m_traits.setTrait(Entry::Virtue::Alias);
         }

         parentEntry->addSubEntry(current);
         return true;
      }

   private:
      clang::ASTContext *m_context;
      clang::PrintingPolicy m_policy;
};

class DoxyPPConsumer : public clang::PPCallbacks {

   public:
      explicit DoxyPPConsumer(clang::ASTContext *context)
            : m_context(context)
      {
      }

      void MacroDefined(const clang::Token &macroName, const clang::MacroDirective *node) override;

      static QString toQString(const llvm::StringRef &value) {
         return QString::fromUtf8(value.data(), value.size());
      }

   private:
      clang::ASTContext *m_context;
};

static void addDefine(QSharedPointer<Entry> current_define, ArgumentList argList,
      QString defineArgs, QString defineInit)
{
   QString defineName  = current_define->m_entryName;
   QString defineFName = current_define->getData(EntryKey::File_Name);

   QSharedPointer<MemberDef> md = QMakeShared<MemberDef>(
            defineFName, current_define->startLine, current_define->startColumn,
            "#define", defineName, defineArgs, "", Protection::Public, Specifier::Normal, false,
            Relationship::Member, MemberDefType::Define, ArgumentList(), ArgumentList());

   md->setArgumentList(argList);

   // align code
   static QRegularExpression regExp("^[^\\n]*?\\n(\\s*).*$", QPatternOption::DotMatchesEverythingOption);
   QRegularExpressionMatch match = regExp.match(defineInit);

   if (match.hasMatch()) {
      // align the items on the first line with the items on the second line
      defineInit = match.captured(1) + defineInit;
   }

   md->setInitializer(defineInit);

   bool ambig;
   QSharedPointer<FileDef> fd  = findFileDef(&Doxy_Globals::inputNameDict, defineFName, ambig);
   md->setFileDef(fd);

   md->setDefinition("#define " + defineName);

   QSharedPointer<MemberName> mn = Doxy_Globals::functionNameSDict.find(defineName);

   if (mn == nullptr) {
      mn = QMakeShared<MemberName>(defineName);
      Doxy_Globals::functionNameSDict.insert(defineName, mn);
   }

   mn->append(md);

   if (fd) {
      fd->insertMember(md);
   }
}

void DoxyPPConsumer::MacroDefined(const clang::Token &macroToken, const clang::MacroDirective *node)
{
   clang::SourceManager &srcManager  = m_context->getSourceManager();
   clang::SourceLocation srcLocation = node->getLocation();

   if (! srcManager.isWrittenInMainFile(srcLocation) ) {
      // not in the current file being processed
      return;
   }

   if (macroToken.isAnyIdentifier()) {
      // #define
      clang::StringRef srcName = macroToken.getIdentifierInfo()->getName();

      QSharedPointer<Entry> current = QMakeShared<Entry>();

      QString currentUSR = getUSR_PP(srcName, srcLocation, srcManager);
      s_entryMap.insert(currentUSR, current);

      clang::FullSourceLoc location = m_context->getFullLoc(srcLocation);

      const clang::MacroInfo *macroInfo = node->getMacroInfo();

      current->section       = Entry::DEFINE_SEC;
      current->m_entryName   = toQString(srcName);

      current->setData(EntryKey::File_Name,   toQString(srcManager.getFilename(location)));

      current->m_srcLang     = SrcLangExt_Cpp;
      current->startLine     = location.getSpellingLineNumber();
      current->startColumn   = location.getSpellingColumnNumber();

      current->startBodyLine = current->startLine;
      current->endBodyLine   = m_context->getFullLoc(macroInfo->getDefinitionEndLoc()).getSpellingLineNumber();

      // parameters
      ArgumentList argList;
      QString args;

      if (! macroInfo->param_empty()) {
         args = "(";

         bool addComma = false;

         for (auto item : macroInfo->params()) {

            if (addComma) {
               args += ", ";
            } else {
               addComma = true;
            }

            Argument tmp;
            tmp.name = toQString(item->getName());
            args += tmp.name;

            argList.append(std::move(tmp));
         }

         args += ")";
      }

      QString defineInit;

      if (! macroInfo->tokens_empty()) {

         auto first = macroInfo->tokens_begin();
         auto last  = macroInfo->tokens_end() - 1;

         const char *begin = srcManager.getCharacterData(first->getLocation());
         const char *end   = srcManager.getCharacterData(last->getEndLoc());

         defineInit = QString::fromUtf8(begin, end - begin);
      }

      s_current_root->addSubEntry(current);

      // create member definition for preprocessor macros
      addDefine(current, argList, args, defineInit);
   }
}

class DoxyASTConsumer : public clang::ASTConsumer {

   public:
      explicit DoxyASTConsumer(clang::ASTContext *context)
            : m_visitor(context)
      {
      }

      // get the TranslationUnitDecl, this is a single Decl which represents the entire source file

      virtual void HandleTranslationUnit(clang::ASTContext &context)  override {
         m_visitor.TraverseDecl(context.getTranslationUnitDecl());

         s_conceptMap.clear();

         // clean up orphan list
         auto iter = s_orphanMap.begin();

         while (iter != s_orphanMap.end()) {
            QSharedPointer<Entry> parentEntry = s_entryMap.value(iter.key());

            if (parentEntry) {
               // found a match
               parentEntry->addSubEntry(iter.value());

               // remove and reset the iter
               iter = s_orphanMap.erase(iter);

            } else {
               ++iter;

            }
         }

         // increment for each TU
         ++anonNSCount;
      }

   private:
      DoxyVisitor m_visitor;
};

std::unique_ptr<clang::ASTConsumer> DoxyFrontEnd::CreateASTConsumer(clang::CompilerInstance &compiler, llvm::StringRef file) {
   (void) file;

   // add callback for preprocessor macros
   std::unique_ptr<DoxyPPConsumer> callBack = std::make_unique<DoxyPPConsumer>(&compiler.getASTContext());

   clang::Preprocessor &pp = compiler.getPreprocessor();
   pp.addPPCallbacks(std::move(callBack));

   // add main call back for AST processing
   return std::unique_ptr<clang::ASTConsumer>(new DoxyASTConsumer(&compiler.getASTContext()));
}
