/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
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

#include <parse_lib_tooling.h>

#include <config.h>
#include <util.h>

static QMap<QString, clang::DeclContext *>  s_parentNodeMap;
static QMultiMap<QString, QSharedPointer<Entry>> s_orphanMap;
static int anonNSCount  = 0;

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

   if (node->getLocation().isInvalid() && (std::string(node->getDeclKindName()) == "TranslationUnit") ) {
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

static QString getUSR_PP(const clang::MacroDefinitionRecord *node)
{
   (void) node;

   QString retval;
   llvm::SmallVector<char, 100> buffer;

   bool ignore = false;      // broom - clang::index::generateUSRForMacro(node, buffer);

   if (! ignore) {
      retval = QString::fromUtf8(buffer.data(), buffer.size());
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

      virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *node) {
         // class, struct, union

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         QString parentUSR  = getUSR_DeclContext(node->getParent());

         QString currentUSR = getUSR_Decl(node);
         s_entryMap.insert(currentUSR, current);

         clang::FullSourceLoc location = m_context->getFullLoc(node->getLocation());

         QString name = getName(node);

         if (node->isClass() ) {

            current->section     = Entry::CLASS_SEC;
            current->m_entryName = name;

            current->setData(EntryKey::Member_Type,  "class");
            current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

            current->m_srcLang        = SrcLangExt_Cpp;
            current->startLine   = location.getSpellingLineNumber();
            current->startColumn = location.getSpellingColumnNumber();
            current->bodyLine    = current->startLine;

            if (node->hasAttr<clang::FinalAttr>())  {
               current->m_traits.setTrait(Entry::Virtue::Final);
            }

            if (! node->hasDefinition()) {
               // required, check to ensure bases() container has entries
               return true;
            }

            //
            clang::ClassTemplatePartialSpecializationDecl *specialNode =
                  llvm::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(node);

            if (specialNode != nullptr) {
               // partial

               std::string tString;
               llvm::raw_string_ostream tStream(tString);

               //  3.8.1 working code
               // auto tmpArgs = specialNode->getTemplateArgsAsWritten();    //  ASTTemplateArgumentListInfo *
               // clang::TemplateSpecializationType::PrintTemplateArgumentList(tStream, tmpArgs->getTemplateArgs(),
               //         tmpArgs->NumTemplateArgs, m_policy);

               auto tmpArgs = specialNode->getTemplateArgsAsWritten();
               clang::printTemplateArgumentList(tStream, tmpArgs->arguments(), m_policy);
               current->m_entryName += toQString(tStream.str());

            } else {
               clang::ClassTemplateSpecializationDecl *specialNode =
                  llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(node);

               if (specialNode != nullptr) {
                  // full

                  std::string tString;
                  llvm::raw_string_ostream tStream(tString);

                  specialNode->getNameForDiagnostic(tStream, m_policy, true);
                  current->m_entryName = toQString(tStream.str());
               }
            }


            // inheritance
            for (auto &item : node->bases()) {
               QString name = toQString(item.getType());

               Protection protection = getAccessSpecifier(&item);
               Specifier virtualType = Specifier::Normal;

               if (item.isVirtual()) {
                  virtualType = Specifier::Virtual;
               }

               auto tmpBase = BaseInfo(name, protection, virtualType);
               current->extends.append(tmpBase);
            }

            if (parentUSR.isEmpty() || parentUSR == "TranslationUnit")  {
               s_current_root->addSubEntry(current, s_current_root);

            } else {
               // nested class
               parentEntry = s_entryMap.value(parentUSR);

               if (parentEntry != nullptr) {
                  current->m_entryName.prepend(parentEntry->m_entryName + "::");
                  current->protection = getAccessSpecifier(node);

                  parentEntry->addSubEntry(current, parentEntry);
               }
            }

         } else if (node->isStruct()) {

            current->section     = Entry::CLASS_SEC;
            current->m_entryName = name;

            current->setData(EntryKey::Member_Type,  "struct");
            current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

            current->m_srcLang        = SrcLangExt_Cpp;
            current->startLine   = location.getSpellingLineNumber();
            current->startColumn = location.getSpellingColumnNumber();
            current->bodyLine    = current->startLine;

            current->m_traits.setTrait(Entry::Virtue::Struct);

            if (node->hasAttr<clang::FinalAttr>())  {
               current->m_traits.setTrait(Entry::Virtue::Final);
            }

            if (parentUSR.isEmpty() || parentUSR == "TranslationUnit")  {
               s_current_root->addSubEntry(current, s_current_root);

            } else {
               // nested struct
               parentEntry = s_entryMap.value(parentUSR);

               if (parentEntry != nullptr) {
                  current->m_entryName.prepend(parentEntry->m_entryName + "::");
                  current->protection = getAccessSpecifier(node);

                  parentEntry->addSubEntry(current, parentEntry);
               }
            }

         } else if (node->isUnion()) {

            current->section     = Entry::CLASS_SEC;
            current->m_entryName = name;

            current->setData(EntryKey::Member_Type,  " union");
            current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

            current->m_srcLang   = SrcLangExt_Cpp;
            current->startLine   = location.getSpellingLineNumber();
            current->startColumn = location.getSpellingColumnNumber();
            current->bodyLine    = current->startLine;

            current->m_traits.setTrait(Entry::Virtue::Union);

            if (parentUSR.isEmpty() || parentUSR == "TranslationUnit")  {
               s_current_root->addSubEntry(current, s_current_root);

            } else {
               // nested union
               parentEntry = s_entryMap.value(parentUSR);

               if (parentEntry != nullptr) {
                  current->m_entryName.prepend(parentEntry->m_entryName + "::");
                  current->protection = getAccessSpecifier(node);

                  parentEntry->addSubEntry(current, parentEntry);
               }
            }
         }

         return true;
      }

      virtual bool VisitFunctionDecl(clang::FunctionDecl *node) {
         // method, function, constructor, destructor, conversion

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         QString parentUSR = getUSR_DeclContext(node->getParent());

         if (! parentUSR.isEmpty()) {
            parentEntry = s_entryMap.value(parentUSR);
         }

         QString currentUSR = getUSR_Decl(node);

         if (s_entryMap.contains(currentUSR)) {
            // can occur when a method is declared in the class and then defined later in the same file
            return true;
         }

         s_entryMap.insert(currentUSR, current);

         clang::FullSourceLoc location    = m_context->getFullLoc(node->getLocation());
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

           case clang::ExceptionSpecificationType::EST_DependentNoexcept:
               {
                 const clang::Expr *tExpr = protoType->getNoexceptExpr();
                 std::string tString;
                 llvm::raw_string_ostream tStream(tString);
                 tExpr->printPretty(tStream, 0, m_policy);

                 args += " noexcept(" + toQString(tStream.str()) + ")";
                 current->m_traits.setTrait(Entry::Virtue::NoExcept);
               }
               break;
           case clang::EST_None:
           case clang::EST_DynamicNone:
           case clang::EST_Dynamic:
           case clang::EST_MSAny:
           case clang::EST_BasicNoexcept:
           case clang::EST_NoexceptFalse:
           case clang::EST_NoexceptTrue:
           case clang::EST_Unevaluated:
           case clang::EST_Uninstantiated:
           case clang::EST_Unparsed:
             break;
           }

         returnType.remove("(anonymous namespace)::");

         if (methodDecl == nullptr) {
            // function

            current->section     = Entry::FUNCTION_SEC;
            current->m_entryName = name;

            current->setData(EntryKey::Member_Type,  returnType);
            current->setData(EntryKey::Member_Args,  args);
            current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

            current->argList     = argList;

            current->m_srcLang        = SrcLangExt_Cpp;
            current->startLine   = location.getSpellingLineNumber();
            current->startColumn = location.getSpellingColumnNumber();
            current->bodyLine    = current->startLine;

            if (parentEntry) {
               parentEntry->addSubEntry(current, parentEntry);

            } else {
               s_current_root->addSubEntry(current, s_current_root);

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

            // const quaifier
            if (methodDecl->isConst())  {
               args += " const ";
               argList.constSpecifier = true;
            }

            // override quaifier
            if (methodDecl->size_overridden_methods() != 0)  {
               args += " override ";
               current->m_traits.setTrait(Entry::Virtue::Override);
            }

            clang::Qualifiers qualifiers = methodDecl->getMethodQualifiers();

            if (qualifiers.hasRestrict()) {
               args += " restrict ";
               // not used - argList.restrictSpecifier = true;
            }

            if (qualifiers.hasVolatile()) {
               args += " volatile ";
               argList.volatileSpecifier = true;
            }

            current->section     = Entry::FUNCTION_SEC;
            current->m_entryName = name;

            current->setData(EntryKey::Member_Type,  returnType);
            current->setData(EntryKey::Member_Args,  args);
            current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

            current->argList     = argList;
            current->protection  = getAccessSpecifier(node);

            current->m_srcLang   = SrcLangExt_Cpp;
            current->startLine   = location.getSpellingLineNumber();
            current->startColumn = location.getSpellingColumnNumber();
            current->bodyLine    = current->startLine;

            if (parentEntry) {
               parentEntry->addSubEntry(current, parentEntry);

            } else {
               // hold until we visit the parent
               s_orphanMap.insert(parentUSR, current);

            }
         }

         return true;
      }

      virtual bool VisitFieldDecl(clang::FieldDecl *node) {
         // class member

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         QString parentUSR = getUSR_Decl(node->getParent());
         parentEntry = s_entryMap.value(parentUSR);

         clang::FullSourceLoc location = m_context->getFullLoc(node->getLocation());

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

         current->protection  = getAccessSpecifier(node);

         current->m_srcLang   = SrcLangExt_Cpp;
         current->startLine   = location.getSpellingLineNumber();
         current->startColumn = location.getSpellingColumnNumber();
         current->bodyLine    = current->startLine;

         if (node->isMutable()) {
            current->prependData(EntryKey::Member_Type, "mutable ");
            current->m_traits.setTrait(Entry::Virtue::Mutable);
         }

         QString currentUSR = getUSR_Decl(node);
         s_entryMap.insert(currentUSR, current);

         parentEntry->addSubEntry(current, parentEntry);

         return true;
      }

      virtual bool VisitEnumDecl(clang::EnumDecl *node) {
         // enum

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         QString parentUSR  = getUSR_DeclContext(node->getParent());
         parentEntry = s_entryMap.value(parentUSR);

         clang::FullSourceLoc location = m_context->getFullLoc(node->getLocation());

         QString name = toQString(node->getNameAsString());
         QString className;

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

         } else {
            // printf("\n broom enum parentEntry NULLPTR   name = %s  USR = %s", csPrintable(name), csPrintable(parentUSR) );
         }

         QString currentUSR = getUSR_Decl(node);
         s_entryMap.insert(currentUSR, current);

         current->section     = Entry::ENUM_SEC;
         current->m_entryName = className + "::" + name;

         current->setData(EntryKey::Member_Type,  "enum");
         current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

         current->protection  = getAccessSpecifier(node);
         current->m_srcLang        = SrcLangExt_Cpp;
         current->startLine   = location.getSpellingLineNumber();
         current->startColumn = location.getSpellingColumnNumber();
         current->bodyLine    = current->startLine;

         if (node->isScoped()) {
            current->m_traits.setTrait(Entry::Virtue::Strong);
         }

         if (parentEntry) {
            parentEntry->addSubEntry(current, parentEntry);

         } else {
            // hold until we visit the parent
            s_orphanMap.insert(parentUSR, current);
         }

         return true;
      }

      virtual bool VisitEnumConstantDecl(clang::EnumConstantDecl *node) {
         // enum values

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         QString parentUSR  = getUSR_DeclContext(node->getDeclContext());
         parentEntry = s_entryMap.value(parentUSR);

         clang::FullSourceLoc location = m_context->getFullLoc(node->getLocation());

         QString name = toQString(node->getNameAsString());
         QString className;

         if (parentEntry == nullptr) {
           // might need to reslove

         } else {
            className = parentEntry->m_entryName;

         }

         if (! name.isEmpty()) {
            // not sure there is an anonymous enum constant

            for (auto entry : parentEntry->children() ) {
               if (entry->m_entryName == className + name) {
                   return true;
               }
            }
         }

         QString currentUSR = getUSR_Decl(node);
         s_entryMap.insert(currentUSR, current);

         current->section     = Entry::VARIABLE_SEC;
         current->m_entryName = name;

         current->setData(EntryKey::Member_Type,  "@");
         current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

         current->protection  = getAccessSpecifier(node);

         current->m_srcLang   = SrcLangExt_Cpp;
         current->startLine   = location.getSpellingLineNumber();
         current->startColumn = location.getSpellingColumnNumber();
         current->bodyLine    = current->startLine;

         auto tExpr = node->getInitExpr();

         if (tExpr != nullptr) {
            // get the real value
      	
            std::string tString;
            llvm::raw_string_ostream tStream(tString);
            tExpr->printPretty(tStream, 0, m_policy);

            current->setData(EntryKey::Initial_Value, " = " + toQString(tStream.str()));
         }

         parentEntry->addSubEntry(current, parentEntry);

         //
         bool isStrong = parentEntry->m_traits.hasTrait(Entry::Virtue::Strong);

         if (! isStrong)  {
            auto tmpEntry = parentEntry->parent();

            if (tmpEntry) {
               tmpEntry->addSubEntry(current, tmpEntry);
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
         parentEntry = s_entryMap.value(parentUSR);

         clang::FullSourceLoc location = m_context->getFullLoc(node->getLocation());

         QString name         = toQString(node->getNameAsString());
         QString type         = toQString(node->getType());

         // remove the scope from the return type
         QString scopeName = parentEntry->m_entryName + "::";
         type = type.remove(scopeName);

         current->section     = Entry::VARIABLE_SEC;
         current->m_entryName = name;

         current->setData(EntryKey::Member_Type,  type);
         current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

         current->protection  = getAccessSpecifier(node);

         current->m_srcLang   = SrcLangExt_Cpp;
         current->startLine   = location.getSpellingLineNumber();
         current->startColumn = location.getSpellingColumnNumber();
         current->bodyLine    = current->startLine;

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
            parentEntry->addSubEntry(current, parentEntry);

         } else {
            // part of a file
            s_current_root->addSubEntry(current, s_current_root);

         }

         return true;
      }

      virtual bool VisitFriendDecl(clang::FriendDecl *node) {
         // friend

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         QString parentUSR  = getUSR_DeclContext(node->getDeclContext());
         parentEntry = s_entryMap.value(parentUSR);

         clang::FullSourceLoc location = m_context->getFullLoc(node->getLocation());

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

         current->argList     = argList;
         current->protection  = getAccessSpecifier(node);

         current->m_srcLang   = SrcLangExt_Cpp;
         current->startLine   = location.getSpellingLineNumber();
         current->startColumn = location.getSpellingColumnNumber();
         current->bodyLine    = current->startLine;

         parentEntry->addSubEntry(current, parentEntry);

         return true;
      }

      virtual bool VisitMacroDefinitionRecord(clang::MacroDefinitionRecord *node) {
         // #define

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         QString currentUSR = getUSR_PP(node);
         s_entryMap.insert(currentUSR, current);

         clang::SourceRange smRange     = node->getSourceRange();
         clang::SourceLocation location = smRange.getBegin();

         QString name         = toQString(node->getName());

         current->section     = Entry::DEFINE_SEC;
         current->m_entryName = name;

         current->m_srcLang   = SrcLangExt_Cpp;
//       current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));
//       current->startLine   = location.getSpellingLineNumber();
//       current->startColumn = location.getSpellingColumnNumber();
         current->bodyLine    = current->startLine;

         // printf("\n  broom - Macro Definition  name: %s   line: %d  col: %d \n",
         //         csPrintable(name), current->startLine, current->startColumn );

         s_current_root->addSubEntry(current, s_current_root);

         return true;
      }

      virtual bool VisitMacroExpansion(clang::MacroExpansion *node) {

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         QString name = toQString(node->getName());

         // printf("\n  broom - Macro Expansion  %s\n", csPrintable(name));

         if (name == "CS_OBJECT" || name == "Q_OBJECT") {
            // do nothing at this time
         }

         return true;
      }

      virtual bool VisitNamespaceDecl(clang::NamespaceDecl *node) {
         // namespace
         static const bool extractAnonNS = Config::getBool("extract-anon-namespaces");

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         clang::FullSourceLoc location = m_context->getFullLoc(node->getLocation());

         QString parentUSR    = getUSR_DeclContext(node->getParent());
         parentEntry          = s_entryMap.value(parentUSR);

         QString currentUSR   = getUSR_Decl(node);
         s_entryMap.insert(currentUSR, current);

         QString name         = getName(node);

         current->section     = Entry::NAMESPACE_SEC;
         current->m_entryName = name;

         current->setData(EntryKey::Member_Type,  "namespace");
         current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

         current->m_srcLang        = SrcLangExt_Cpp;
         current->startLine   = location.getSpellingLineNumber();
         current->startColumn = location.getSpellingColumnNumber();
         current->bodyLine    = current->startLine;

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
            s_current_root->addSubEntry(current, s_current_root);
         } else {
            parentEntry->addSubEntry(current, parentEntry);
         }

         return true;
      }

      virtual bool VisitTemplateTypeParmDecl (clang::TemplateTypeParmDecl *node) {
         // template type

         QSharedPointer<Entry> parentEntry;
         QSharedPointer<Entry> current = QMakeShared<Entry>();

         clang::FullSourceLoc location = m_context->getFullLoc(node->getLocation());

         QString parentUSR  = getUSR_DeclContext(node->getDeclContext());
         parentEntry = s_entryMap.value(parentUSR);

         if (parentEntry != nullptr && parentUSR != "TranslationUnit") {

            auto parentNode = node->getParentFunctionOrMethod();

            if (s_parentNodeMap.contains(parentUSR) && (s_parentNodeMap.value(parentUSR) != parentNode) ) {
               // already have this template parameter
               return true;
            }

            s_parentNodeMap.insert(parentUSR, parentNode);

            QString currentUSR = getUSR_Decl(node);
            s_entryMap.insert(currentUSR, current);

            if (parentEntry->m_templateArgLists.isEmpty()) {
               ArgumentList temp;
               parentEntry->m_templateArgLists.append(temp);
            }

            parentEntry->m_traits.setTrait(Entry::Virtue::Template);

            QString type = "class";
            QString name = getName(node);
            QString defValue;

            if (node->wasDeclaredWithTypename())  {
               type = "typename";
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

         clang::FullSourceLoc location = m_context->getFullLoc(node->getLocation());

         QString name = getName(node);

         current->section     = Entry::VARIABLE_SEC;
         current->m_entryName = name;

         current->setData(EntryKey::Member_Type,  "typedef " + toQString(node->getUnderlyingType()));
         current->setData(EntryKey::File_Name,    toQString(location.getManager().getFilename(location)));

         current->protection  = getAccessSpecifier(node);
         current->m_srcLang  = SrcLangExt_Cpp;
         current->startLine   = location.getSpellingLineNumber();
         current->startColumn = location.getSpellingColumnNumber();
         current->bodyLine    = current->startLine;

         if (llvm::dyn_cast<clang::TypeAliasDecl>(node)) {
           current->m_traits.setTrait(Entry::Virtue::Alias);
         }

         parentEntry->addSubEntry(current, parentEntry);
         return true;
      }

   private:
      clang::ASTContext *m_context;
      clang::PrintingPolicy m_policy;
};

class DoxyASTConsumer : public clang::ASTConsumer {

   public:
      explicit DoxyASTConsumer(clang::ASTContext *context)
            : m_visitor(context) {}

      // get the TranslationUnitDecl, this is a single Decl which represents the entire source file

      virtual void HandleTranslationUnit(clang::ASTContext &context)  override {
         m_visitor.TraverseDecl(context.getTranslationUnitDecl());

         // clean up orphan list
         auto iter = s_orphanMap.begin();

         while (iter != s_orphanMap.end()) {
            QSharedPointer<Entry> parentEntry = s_entryMap.value(iter.key());

            if (parentEntry) {
               // found a match
               parentEntry->addSubEntry(iter.value(), parentEntry);

               // remove and reset the iter
               iter = s_orphanMap.erase(iter);

            } else {
               ++iter;

            }
         }

         // increment for each TU
         anonNSCount++;
      }

   private:
       DoxyVisitor m_visitor;
};

std::unique_ptr<clang::ASTConsumer> DoxyFrontEnd::CreateASTConsumer(clang::CompilerInstance &compiler, llvm::StringRef file) {
   (void) file;

   return std::unique_ptr<clang::ASTConsumer>(new DoxyASTConsumer(&compiler.getASTContext()));
}
