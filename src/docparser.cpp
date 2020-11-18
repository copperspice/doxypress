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

#include <QFile>
#include <QFileInfo>
#include <QStack>
#include <QHash>
#include <QRegularExpression>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <docparser.h>

#include <arguments.h>
#include <cite.h>
#include <config.h>
#include <cmdmapper.h>
#include <doctokenizer.h>
#include <doxy_globals.h>
#include <emoji_entity.h>
#include <formula.h>
#include <htmlentity.h>
#include <language.h>
#include <message.h>
#include <parse_md.h>
#include <portable.h>
#include <printdocvisitor.h>
#include <util.h>

#define TK_COMMAND_CHAR(token) ((token)==TK_COMMAND_AT ? '@' : '\\')
// debug off
#define DBG(x) do {} while(0)

// debug to stdout
// #define DBG(x) printf x

// debug to stderr
// #define myprintf(x...) fprintf(stderr,x)
// #define DBG(x) myprintf x

#define INTERNAL_ASSERT(x) do {} while(0)
// #define INTERNAL_ASSERT(x) if (!(x)) DBG(("INTERNAL_ASSERT(%s) failed retval=0x%x: file=%s line=%d\n",#x,retval,__FILE__,__LINE__));

static bool defaultHandleToken(DocNode *parent, int tok, QList<DocNode *> &children, bool handleWord = true);

static const QString sectionLevelToName[] = {
   "page",
   "section",
   "subsection",
   "subsubsection",
   "paragraph",
   "subparagraph"
};

/** Parser's context to store all global variables.
 */
struct DocParserContext {
   QSharedPointer<Definition> scope;
   QString context;
   bool inSeeBlock;
   bool xmlComment;
   bool insideHtmlLink;

   QStack<DocNode *> nodeStack;
   QStack<DocStyleChange> styleStack;
   QStack<DocStyleChange> initialStyleStack;
   QList<QSharedPointer<Definition>> copyStack;

   QString fileName;
   QString relPath;

   int lineNum;

   bool hasParamCommand;
   bool hasReturnCommand;

   QSharedPointer<MemberDef> memberDef;
   QSet<QString> paramsFound;

   bool     isExample;
   QString  exampleName;
   SectionDict *sectionDict;
   QString  searchUrl;

   QString  includeFileText;
   uint     includeFileOffset;
   uint     includeFileLength;
   uint     includeFileLine;
   bool     includeFileUseLN;

   TokenInfo *token;
};

// Parser state variables during a call to validatingParseDoc
static QSharedPointer<Definition> s_scope;
static QString                 s_context;
static bool                    s_inSeeBlock;
static bool                    s_xmlComment;
static bool                    s_insideHtmlLink;
static QStack<DocNode *>       s_nodeStack;
static QStack<DocStyleChange>  s_styleStack;
static QStack<DocStyleChange>  s_initialStyleStack;

static QList<QSharedPointer<Definition>> s_copyStack;

static QString                 s_fileName;
static QString                 s_relPath;

static bool                    s_hasParamCommand;
static bool                    s_hasReturnCommand;
static QSet<QString>           s_paramsFound;

static QSharedPointer<MemberDef> s_memberDef;

static bool                    s_isExample;
static QString                 s_exampleName;
static SectionDict            *s_sectionDict;
static QString                 s_searchUrl;

static QString                 s_includeFileName;
static QString                 s_includeFileText;
static uint                    s_includeFileOffset;
static uint                    s_includeFileLength;
static uint                    s_includeFileLine;
static bool                    s_includeFileUseLN;

static QStack<DocParserContext> s_parserStack;

static void docParserPushContext(bool saveParamInfo = true)
{
   doctokenizerYYpushContext();

   DocParserContext ctx;

   ctx.scope              = s_scope;
   ctx.context            = s_context;
   ctx.inSeeBlock         = s_inSeeBlock;
   ctx.xmlComment         = s_xmlComment;
   ctx.insideHtmlLink     = s_insideHtmlLink;
   ctx.nodeStack          = s_nodeStack;
   ctx.styleStack         = s_styleStack;
   ctx.initialStyleStack  = s_initialStyleStack;
   ctx.copyStack          = s_copyStack;
   ctx.fileName           = s_fileName;
   ctx.lineNum            = getDoctokenLineNum();
   ctx.relPath            = s_relPath;

   if (saveParamInfo) {
      ctx.hasParamCommand   = s_hasParamCommand;
      ctx.hasReturnCommand  = s_hasReturnCommand;
      ctx.paramsFound       = s_paramsFound;
   }

   ctx.memberDef          = s_memberDef;
   ctx.isExample          = s_isExample;
   ctx.exampleName        = s_exampleName;
   ctx.sectionDict        = s_sectionDict;
   ctx.searchUrl          = s_searchUrl;

   ctx.includeFileText    = s_includeFileText;
   ctx.includeFileOffset  = s_includeFileOffset;
   ctx.includeFileLength  = s_includeFileLength;
   ctx.includeFileLine    = s_includeFileLine;
   ctx.includeFileUseLN   = s_includeFileUseLN;

   ctx.token = g_token;
   g_token   = new TokenInfo;

   s_parserStack.push(ctx);
}

static void docParserPopContext(bool keepParamInfo = false)
{
   DocParserContext ctx = s_parserStack.pop();

   s_scope               = ctx.scope;
   s_context             = ctx.context;
   s_inSeeBlock          = ctx.inSeeBlock;
   s_xmlComment          = ctx.xmlComment;
   s_insideHtmlLink      = ctx.insideHtmlLink;
   s_nodeStack           = ctx.nodeStack;
   s_styleStack          = ctx.styleStack;
   s_initialStyleStack   = ctx.initialStyleStack;
   s_copyStack           = ctx.copyStack;
   s_fileName            = ctx.fileName;
   s_relPath             = ctx.relPath;

   if (! keepParamInfo) {
      s_hasParamCommand     = ctx.hasParamCommand;
      s_hasReturnCommand    = ctx.hasReturnCommand;
      s_paramsFound         = ctx.paramsFound;
   }

   s_memberDef           = ctx.memberDef;
   s_isExample           = ctx.isExample;
   s_exampleName         = ctx.exampleName;
   s_sectionDict         = ctx.sectionDict;
   s_searchUrl           = ctx.searchUrl;

   s_includeFileText     = ctx.includeFileText;
   s_includeFileOffset   = ctx.includeFileOffset;
   s_includeFileLength   = ctx.includeFileLength;
   s_includeFileLine     = ctx.includeFileLine;
   s_includeFileUseLN    = ctx.includeFileUseLN;

   delete g_token;
   g_token               = ctx.token;

   setDoctokenLineNum(ctx.lineNum);
   doctokenizerYYpopContext();
}

// replaces { with < and } with > inside string s
static void unescapeCRef(QString &value)
{
   QString tmp;

   for (auto c :  value) {

      if (c == '{') {
         tmp += '<';

      } else if ( c== '}') {
         tmp += '>';

      } else {
         tmp += c;
      }
   }

   value = substitute(tmp,   "&lt;", "<");
   value = substitute(value, "&gt;", ">");
}

/*! search for an image in the imageNameDict and if found
 * copies the image to the output directory (which depends on the \a type
 * parameter).
 */
static QString findAndCopyImage(const QString &fileName, DocImage::Type type, bool dowarn = true)
{
   static const bool generateHtml    = Config::getBool("generate-html");
   static const bool generateLatex   = Config::getBool("generate-latex");
   static const bool generateDocbook = Config::getBool("generate-docbook");
   static const bool generateRtf     = Config::getBool("generate-rtf");
   static const bool latexPdf        = Config::getBool("latex-pdf");

   // static const bool generateXml   = Config::getBool("generate-xml");

   static const QString outputHtmlDir  = Config::getString("html-output");
   static const QString outputLatexDir = Config::getString("latex-output");

   QString result;
   bool ambig;

   QSharedPointer<FileDef> fd = findFileDef(&Doxy_Globals::imageNameDict, fileName, ambig);


   if (fd) {
      if (ambig & dowarn) {
         QString text = QString("Image file %1 is ambiguous.\n").formatArg(fileName);
         text += "Possible candidates:\n" + showFileDefMatches(Doxy_Globals::imageNameDict, fileName);

         warn_doc_error(s_fileName, doctokenizerYYlineno, text);
      }
      QString inputFile = fd->getFilePath();
      QFile inImage(inputFile);

      if (inImage.open(QIODevice::ReadOnly)) {
         result = fileName;

         int i;

         if ((i = result.lastIndexOf('/')) != -1 || (i = result.lastIndexOf('\\')) != -1) {
            result = result.right(result.length() - i - 1);
         }

         QString outputDir;

         switch (type) {
            case DocImage::Html:

               if (! generateHtml) {
                  return result;
               }
               outputDir = outputHtmlDir;
               break;

            case DocImage::Latex:
               if (! generateLatex) {
                  return result;
               }
               outputDir = outputLatexDir;
               break;

            case DocImage::DocBook:
               if (! generateDocbook) {
                  return result;
               }
               outputDir = Config::getString("docbook-output");
               break;

            case DocImage::Rtf:
               if (! generateRtf) {
                  return result;
               }
               outputDir = Config::getString("rtf-output");
               break;

            case DocImage::None:
               break;
         }

         QString outputFile = outputDir + "/" + result;

         if (outputFile != inputFile) {
            // prevent copying to ourself

            QFile::remove(outputFile);
            bool copyOk = inImage.copy(outputFile);

            if (copyOk) {
               // copy the image

               if (type == DocImage::Html) {
                  Doxy_Globals::indexList.addImageFile(result);
               }

            } else {
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unable to open image file %s for writing, OS Error #: %d\n",
                        csPrintable(outputFile), inImage.error());
            }

         } else {
            warn(s_fileName, doctokenizerYYlineno, "Source and destination directories are the same, unable to copy %s\n",
                        csPrintable(inputFile));
         }

      } else {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unable to open image file %s for reading, OS Error #: %d",
                        csPrintable(fileName), inImage.error() );
      }

      if (type == DocImage::Latex && latexPdf && fd->name().endsWith(".eps")) {
         // we have an .eps image in pdflatex mode => convert it to a pdf

         QString baseName  = fd->name().left(fd->name().length() - 4);

         QString epstopdfArgs;
         epstopdfArgs = QString("\"%1/%2.eps\" --outfile=\"%3/%4.pdf\"")
                  .formatArg(outputLatexDir).formatArg(baseName).formatArg(outputLatexDir).formatArg(baseName);

         portable_sysTimerStart();

         if (portable_system("epstopdf", epstopdfArgs) != 0) {
            err("Problem running epstopdf, verify TeX installation\n");
         }

         portable_sysTimerStop();

         return baseName;
      }



   } else {
      result = fileName;

      if (! result.startsWith("http:") && ! result.startsWith("https:") && dowarn)  {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Image file %s was not found in 'IMAGE PATH': "
                        "assuming it is an external image", csPrintable(fileName) );
      }
   }

   return result;
}

/*! Collects the parameters found with \@param or \@retval commands in a global
 *  list s_paramsFound. If \a isParam is set to true and the parameter is not
 *  an actual parameter of the current member s_memberDef, then a warning is
 *  raised (unless warnings are disabled altogether).
 */
static void checkArgumentName(const QString &name, bool isParam)
{
   if (! Config::getBool("warn-doc-error")) {
      return;
   }

   if (s_memberDef == nullptr) {
      return;
   }

   const ArgumentList *al;      // ok as a ptr

   if (s_memberDef->isDocsForDefinition())  {
      al = &s_memberDef->getArgumentList();

   } else {
      al = &s_memberDef->getDeclArgumentList();

   }

   if (al == nullptr) {
      return;
   }

   SrcLangExt lang  = s_memberDef->getLanguage();

   static QRegularExpression regExp("\\$?[a-zA-Z0-9_\\x80-\\xFF]+\\.*");
   QRegularExpressionMatch match = regExp.match(name);

   QString::const_iterator current_iter = name.constBegin();
   QString::const_iterator start_iter   = name.constBegin();

   while (match.hasMatch()) {
      // to handle param x,y

      start_iter = match.capturedStart();

      QString aName = QStringView(current_iter, match.capturedEnd());

      if (lang == SrcLangExt_Fortran) {
         aName = aName.toLower();
      }

      bool found = false;

      for (const auto &arg : *al) {
         QString argName = s_memberDef->isDefine() ? arg.type : arg.name;

         if (lang == SrcLangExt_Fortran) {
            argName = argName.toLower();
         }

         argName = argName.trimmed();

         if (argName.endsWith("...")) {
            argName = argName.left(argName.length() - 3);
         }

         if (aName == argName && isParam) {
            s_paramsFound.insert(aName);
            found = true;
            break;
         }
      }

      if (! found && isParam) {
         QString scope = s_memberDef->getScopeString();

         if (! scope.isEmpty()) {
            scope += "::";
         } else {
            scope = "";
         }

         QString inheritedFrom = "";

         QString docFile = s_memberDef->docFile();
         int docLine     = s_memberDef->docLine();

         QSharedPointer<MemberDef> inheritedMd = s_memberDef->inheritsDocsFrom();

         if (inheritedMd) {
            // documentation was inherited

            inheritedFrom = QString(" Inherited from member %1 at line %2 in file %3")
                  .formatArg(QString(inheritedMd->name()))
                  .formatArg(inheritedMd->docLine()).formatArg(QString(inheritedMd->docFile()));

            docFile = s_memberDef->getDefFileName();
            docLine = s_memberDef->getDefLine();
         }

         QString alStr = argListToString(*al);
         warn_doc_error(docFile, docLine, "Argument '%s' of command @param was not found in the argument list of %s%s%s%s",
                        csPrintable(aName), csPrintable(scope), csPrintable(s_memberDef->name()),
                        csPrintable(alStr), csPrintable(inheritedFrom));
      }

      current_iter = match.capturedEnd();
      match = regExp.match(name, current_iter);
   }
}

/*! Checks if the parameters that have been specified using param are all parameters.
 *  Must be called after checkArgumentName() has been called for each argument.
 */
static void checkUndocumentedParams()
{
   static const bool warnDocError = Config::getBool("warn-doc-error");

   if (s_memberDef && s_hasParamCommand && warnDocError) {
      const ArgumentList *al;         // ok as a ptr

      if (s_memberDef->isDocsForDefinition())  {
         al = &s_memberDef->getArgumentList();

      } else {
         al = &s_memberDef->getDeclArgumentList();

      }

      SrcLangExt lang = s_memberDef->getLanguage();

      if (al != nullptr) {
         bool found = false;

         for (const auto &arg : *al) {
            int count = 0;

            QString argName;
            QString tmpName;

            if (s_memberDef->isDefine()) {
               argName = arg.type;

            } else {
               argName = arg.name;

            }

            if (lang == SrcLangExt_Fortran) {
               argName = argName.toLower();
            }

            argName = argName.trimmed();
            tmpName = argName;

            if (argName.endsWith("...")) {
               argName = argName.left(argName.length() - 3);
            }

            if (lang == SrcLangExt_Python && (argName == "self" || argName == "cls")) {
               // allow undocumented self / cls parameter for Python

            } else if (! argName.isEmpty() && arg.docs.isEmpty() && ! s_paramsFound.contains(argName)) {
               found = true;

            } else {
               for (const auto &item : s_paramsFound) {
                  if (argName == item) {
                     count++;
                  }
               }
            }

            if (count > 1) {
               warn_doc_error(s_memberDef->getDefFileName(), s_memberDef->getDefLine(), "Argument '" + tmpName +
                  "' from the argument list of " + s_memberDef->qualifiedName() + " has multiple \\param documentation sections");
            }

         }

         if (found) {
            bool first = true;

            QString errMsg = "The following parameters of " + s_memberDef->qualifiedName() +
                  argListToString(*al) + " are not documented:\n";

            for (auto a : *al) {
               QString argName;

               if (s_memberDef->isDefine()) {
                  argName = a.type;
               } else {
                  argName = a.name;
               }

               if (lang == SrcLangExt_Fortran) {
                  argName = argName.toLower();
               }

               argName = argName.trimmed();

               if (lang == SrcLangExt_Python && (argName == "self" || argName == "cls")) {
                  // allow undocumented self / cls parameter for Python

               } else if (! argName.isEmpty() && ! s_paramsFound.contains(argName)) {

                  if (! first) {
                     errMsg += "\n";

                  } else {
                     first = false;

                  }

                  errMsg += "  parameter '" + argName + "'";
               }
            }

            warn_doc_error(s_memberDef->getDefFileName(), s_memberDef->getDefLine(), substitute(errMsg, "%", "%%"));

         }
      }
   }
}

static QString stripTrailingReturn(const QString trailingReturn)
{
  QString retval = trailingReturn;

  QStringView tmp = QStringView(retval).trimmed();

  if (tmp.startsWith("->")) {
    retval = tmp.mid(2).trimmed();
  }

  return retval;
}

/*! Check if a member has documentation for its parameter and or return
 *  type. If found this will be stored in the member, this
 *  is needed as a member can have brief and detailed documentation, while
 *  only one of these needs to document the parameters.
 */
static void detectUndocumentedParams()
{
   static const bool warnUndocParam = Config::getBool("warn-undoc-param");

   if (s_memberDef != nullptr &&  warnUndocParam) {
      const ArgumentList &argList     = s_memberDef->getArgumentList();
      const ArgumentList &declArgList = s_memberDef->getDeclArgumentList();

      QString returnType = s_memberDef->removeReturnKeywords();

      SrcLangExt lang = s_memberDef->getLanguage();
      bool isPython   = (lang == SrcLangExt_Python);
      bool isFortran  = (lang == SrcLangExt_Fortran);

      bool isFortranSubroutine = (isFortran && returnType.contains("subroutine"));
      bool isVoidReturn        = (returnType == "void");

      if (! isVoidReturn && returnType == "auto")   {
         const ArgumentList *defArgList;

         if (s_memberDef->isDocsForDefinition())  {
            defArgList = &argList;
         } else {
            defArgList = &declArgList;
         }

         if (! defArgList->trailingReturnType.isEmpty()) {
            QString strippedTrailingReturn = stripTrailingReturn(defArgList->trailingReturnType);
            isVoidReturn = (strippedTrailingReturn == "void") || (strippedTrailingReturn.endsWith(" void"));
         }
      }

      if (! s_memberDef->hasDocumentedParams() && s_hasParamCommand) {
         s_memberDef->setHasDocumentedParams(true);

      } else if (! s_memberDef->hasDocumentedParams()) {
         // no parameter => all parameters are documented
         bool allDoc = true;

         if (! argList.listEmpty() ) {
            // member has parameters but the member has a parameter list
            // with at least one parameter (that is not void)

            // see if all parameters have documentation
            for (const auto &arg : argList) {
               if (! allDoc) {
                  break;
               }

               if (! arg.name.isEmpty() && arg.type != "void" &&
                     ! (isPython && (arg.name == "self" || arg.name == "cls"))) {
                  allDoc = ! arg.docs.isEmpty();
               }
            }

            if (! allDoc && ! declArgList.listEmpty()) {
               // try declaration arguments as well
               allDoc = true;

               for (const auto &arg : declArgList) {

                  if (! allDoc) {
                     break;
                  }

                  if (! arg.name.isEmpty() && arg.type != "void" &&
                        ! (isPython && (arg.name == "self"|| arg.name == "cls"))) {
                     allDoc = ! arg.docs.isEmpty();
                  }
               }
            }
         }

         if (allDoc) {
            s_memberDef->setHasDocumentedParams(true);
         }
      }

      if (! s_memberDef->hasDocumentedReturnType() && s_hasReturnCommand) {
         // docs not yet found
         s_memberDef->setHasDocumentedReturnType(true);


      } else if ( s_hasReturnCommand &&
           (isVoidReturn || isFortranSubroutine ||
           s_memberDef->isConstructor() || s_memberDef->isDestructor()) )  {

         // return type is documented in a function without a return type
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Documented empty return type");


      } else if ( s_memberDef->hasDocumentedReturnType() ||
           isVoidReturn || isFortranSubroutine ||
           s_memberDef->isConstructor() || s_memberDef->isDestructor() ) {

         // return type should be documented
         s_memberDef->setHasDocumentedReturnType(true);

      }
   }
}

/*! Strips known html and tex extensions from \a text. */
static QString stripKnownExtensions(const QString &text)
{
   QString result  = text;
   QString htmlExt = Doxy_Globals::htmlFileExtension;

   if (result.endsWith(".tex")) {
      result = result.left(result.length() - 4);

   } else if (result.endsWith(htmlExt)) {
      result = result.left(result.length() - htmlExt.length());
   }

   return result;
}

/*! Returns true if node n is a child of a preformatted node */
static bool insidePRE(DocNode *n)
{
   while (n) {
      if (n->isPreformatted()) {
         return true;
      }
      n = n->parent();
   }

   return false;
}

/*! Returns true iff node n is a child of an html list item node */
static bool insideLI(DocNode *n)
{
   while (n) {
      if (n->kind() == DocNode::Kind_HtmlListItem) {
         return true;
      }
      n = n->parent();
   }
   return false;
}

/*! Returns true iff node n is a child of a unordered html list node */
static bool insideUL(DocNode *n)
{
   while (n) {
      if (n->kind() == DocNode::Kind_HtmlList && ((DocHtmlList *)n)->type() == DocHtmlList::Unordered) {
         return true;
      }
      n = n->parent();
   }
   return false;
}

/*! Returns true iff node n is a child of a ordered html list node */
static bool insideOL(DocNode *n)
{
   while (n) {
      if (n->kind() == DocNode::Kind_HtmlList && ((DocHtmlList *)n)->type() == DocHtmlList::Ordered) {
         return true;
      }
      n = n->parent();
   }
   return false;
}

static bool insideTable(DocNode *n)
{
   while (n) {
      if (n->kind() == DocNode::Kind_HtmlTable) {
         return true;
      }
      n = n->parent();
   }
   return false;
}

/*! Looks for a documentation block with name commandName in the current
 *  context (g_context). The resulting documentation string is put in doc and
 *  the definition in which the documentation was found is put in retval.
 */
static QSharedPointer<Definition> findDocsForMemberOrCompound(QString commandName, QString &doc, QString &brief)
{
   QSharedPointer<Definition> retval;

   if (commandName.isEmpty()) {
      return retval;
   }

   doc   = "";
   brief = "";

   QString cmdArg = commandName;

   QSharedPointer<FileDef>  fd;
   QSharedPointer<GroupDef> gd;
   QSharedPointer<PageDef>  pd;

   gd = Doxy_Globals::groupSDict.find(cmdArg);

   if (gd != nullptr)  {
      doc    = gd->documentation();
      brief  = gd->briefDescription();
      retval = gd;

      return retval;
   }

   pd = Doxy_Globals::pageSDict.find(cmdArg);

   if (pd != nullptr) {
      doc    = pd->documentation();
      brief  = pd->briefDescription();
      retval = pd;

      return retval;
   }

   bool ambig;
   fd = findFileDef(&Doxy_Globals::inputNameDict, cmdArg, ambig);

   if (fd != nullptr && ! ambig)  {
      doc    = fd->documentation();
      brief  = fd->briefDescription();
      retval = fd;

      return retval;
   }

   // normalize the separator, so A#B, or A\B, or A.B becomes A::B
   cmdArg = substitute(cmdArg, "#" , "::");
   cmdArg = substitute(cmdArg, "\\", "::");
   cmdArg = substitute(cmdArg, ".",  "::");

   int len = cmdArg.length();

   //
   int funcStart = cmdArg.indexOf('(');

   if (funcStart == -1) {
      funcStart = len;

   } else {
      // Check for the case of operator() and the like
      // beware of scenarios like operator()((foo)bar)
      int secondParen = cmdArg.indexOf('(', funcStart + 1);
      int leftParen   = cmdArg.indexOf(')', funcStart + 1);

      if (leftParen != -1 && secondParen != -1) {
         if (leftParen < secondParen) {
            funcStart = secondParen;
         }
      }
   }

   QString name = removeRedundantWhiteSpace(cmdArg.left(funcStart));
   QString args = cmdArg.right(len - funcStart);

   // test if the link is to a member
   QSharedPointer<MemberDef>    md;
   QSharedPointer<ClassDef>     cd;
   QSharedPointer<NamespaceDef> nd;

   // find('.') is a hack to detect files
   bool found = getDefs(s_context.indexOf('.') == -1 ? s_context : QString(), name, args.isEmpty() ? QString() : args,
                  md, cd, fd, nd, gd, false, QSharedPointer<FileDef>(), true);

   if (found && md) {
      doc    = md->documentation();
      brief  = md->briefDescription();
      retval = md;

      return retval;
   }

   int scopeOffset = s_context.length();

   do {
      // for each scope
      QString fullName = cmdArg;

      if (scopeOffset > 0) {
         fullName.prepend(s_context.left(scopeOffset) + "::");
      }

      // try class, namespace, group, page, file reference
      cd = Doxy_Globals::classSDict.find(fullName);

      if (cd) {
         doc     = cd->documentation();
         brief   = cd->briefDescription();
         retval  = cd;

         return retval;
      }

      nd = Doxy_Globals::namespaceSDict.find(fullName);

      if (nd) {
         doc    = nd->documentation();
         brief  = nd->briefDescription();
         retval = nd;

         return retval;
      }

      if (scopeOffset == 0) {
         scopeOffset = -1;

      } else {
         scopeOffset = s_context.lastIndexOf("::", scopeOffset - 1);

         if (scopeOffset == -1) {
            scopeOffset = 0;
         }
      }

   } while (scopeOffset >= 0);

   return retval;
}

static int handleStyleArgument(DocNode *parent, QList<DocNode *> &children, const QString &cmdName)
{
   DBG(("handleStyleArgument(%s)\n", csPrintable(cmdName)));

   int tok = doctokenizerYYlex();

   if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command", csPrintable(cmdName));
      return tok;
   }

   while ((tok = doctokenizerYYlex()) && tok != TK_WHITESPACE && tok != TK_NEWPARA &&
          tok != TK_LISTITEM && tok != TK_ENDLIST) {

      static QRegularExpression specialChar("[.,|()\\[\\]:;\\?]");

      if (tok == TK_WORD && g_token->name.length() == 1 && g_token->name.contains(specialChar)) {
         // special character that ends the markup command
         return tok;
      }

      if (! defaultHandleToken(parent, tok, children)) {
         switch (tok) {
            case TK_COMMAND_AT:
            case TK_COMMAND_BS:

               warn_doc_error(s_fileName, doctokenizerYYlineno, "Illegal command \\%s as the argument of a \\%s command",
                              csPrintable(g_token->name), csPrintable(cmdName));

               break;

            case TK_SYMBOL:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unsupported symbol %s found while handling command %s",
                              csPrintable(g_token->name), csPrintable(cmdName));
               break;

            case TK_HTMLTAG:
               if (insideLI(parent) && Mappers::htmlTagMapper->map(g_token->name) && g_token->endTag) {
                  // ignore </li> as the end of a style command
                  continue;
               }

               return tok;
               break;

            default:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s while handling command %s",
                  csPrintable(tokToString(tok)), csPrintable(cmdName));
               break;
         }
         break;
      }
   }

   DBG(("handleStyleArgument(%s) end tok=%x\n", csPrintable(cmdName), tok));

   return (tok == TK_NEWPARA || tok == TK_LISTITEM || tok == TK_ENDLIST) ? tok : RetVal_OK;
}

// Called when a style change starts. For instance a \<b\> command is encountered
static void handleStyleEnter(DocNode *parent, QList<DocNode *> &children, DocStyleChange::Style s,
            const HtmlAttribList *attribs)
{
   DBG(("HandleStyleEnter\n"));

   DocStyleChange *sc = new DocStyleChange(parent, s_nodeStack.count(), s, true, *attribs);
   children.append(sc);
   s_styleStack.push(*sc);
}

// Called when a style change ends. For instance a \</b\> command is encountered
static void handleStyleLeave(DocNode *parent, QList<DocNode *> &children, DocStyleChange::Style s, const QString &tagName)
{
   DBG(("HandleStyleLeave\n"));

   if (s_styleStack.isEmpty() || s_styleStack.top().style() != s || s_styleStack.top().position() != s_nodeStack.count()) {

      if (s_styleStack.isEmpty()) {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Found </%s> tag without matching <%s>",
                        csPrintable(tagName), csPrintable(tagName));

      } else if (s_styleStack.top().style() != s) {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Found </%s> tag while expecting </%s>",
                        csPrintable(tagName), csPrintable(s_styleStack.top().styleString()));

      } else {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Found </%s> at a different nesting level (%d) than expected (%d)",
                        csPrintable(tagName), s_nodeStack.count(), s_styleStack.top().position());
      }

   } else {
      // end the section
      DocStyleChange *sc = new DocStyleChange(parent, s_nodeStack.count(), s, false);

      children.append(sc);
      s_styleStack.pop();
   }
}

/*! Called at the end of a paragraph to close all open style changes
 *  (e.g. a <b> without a </b>). The closed styles are pushed onto a stack
 *  and entered again at the start of a new paragraph.
 */
static void handlePendingStyleCommands(DocNode *parent, QList<DocNode *> &children)
{
   if (! s_styleStack.isEmpty()) {
      DocStyleChange sc = s_styleStack.top();

      while (sc.position() >= s_nodeStack.count()) {

         // there are unclosed style modifiers in the paragraph
         children.append(new DocStyleChange(parent, s_nodeStack.count(), sc.style(), false));

         s_initialStyleStack.push(sc);

         //
         if (s_styleStack.isEmpty()) {
            break;
         }
         s_styleStack.pop();

         if (s_styleStack.isEmpty())  {
            // all done
            break;

         } else {
            sc = s_styleStack.top();

         }
      }
   }
}

static void handleInitialStyleCommands(DocPara *parent, QList<DocNode *> &children)
{
   while (! s_initialStyleStack.isEmpty()) {
      DocStyleChange sc = s_initialStyleStack.pop();
      handleStyleEnter(parent, children, sc.style(), &sc.attribs());
   }
}

static int handleAHref(DocNode *parent, QList<DocNode *> &children, const HtmlAttribList &tagHtmlAttribs)
{
   int index  = 0;
   int retval = RetVal_OK;

   for (auto opt : tagHtmlAttribs) {

      if (opt.name == "name" || opt.name == "id") {
         // <a name=label> tag

         if (! opt.value.isEmpty()) {
            DocAnchor *anc = new DocAnchor(parent, opt.value, true);
            children.append(anc);
            break; // stop looking for other tag attribs

         } else {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Found <a> tag with name option but without a value");
         }

      } else if (opt.name == "href") { // <a href=url>..</a> tag
         // copy attributes
         HtmlAttribList attrList = tagHtmlAttribs;

         // and remove the href attribute
         attrList.removeAt(index);

         DocHRef *href = new DocHRef(parent, attrList, opt.value, s_relPath);
         children.append(href);

         s_insideHtmlLink = true;
         retval = href->parse();
         s_insideHtmlLink = false;

         break;

      } else {
         // unsupported option for tag a

      }

      ++index;
   }

   return retval;
}

QString DocStyleChange::styleString() const
{
   QString retval = "<invalid>";

   switch (m_style) {
      case DocStyleChange::Bold:
         retval = "b";
         break;

      case DocStyleChange::Italic:
         retval = "em";
         break;

      case DocStyleChange::Code:
         retval = "code";
         break;

      case DocStyleChange::Center:
         retval = "center";
         break;

      case DocStyleChange::Small:
         retval = "small";
         break;

      case DocStyleChange::Subscript:
         retval = "subscript";
         break;

      case DocStyleChange::Superscript:
         retval = "superscript";
         break;

      case DocStyleChange::Preformatted:
         retval = "pre";
         break;

      case DocStyleChange::Div:
         retval = "div";
         break;

      case DocStyleChange::Span:
         retval = "span";
         break;

      case DocStyleChange::Strike:
         retval = "strike";
         break;

       case DocStyleChange::Underline:
         retval = "u";
         break;

       case DocStyleChange::Del:
         retval = "del";
         break;

       case DocStyleChange::Ins:
         retval = "ins";
         break;
   }

   return retval;
}

static void handleUnclosedStyleCommands()
{
   if (! s_initialStyleStack.isEmpty()) {
      DocStyleChange sc = s_initialStyleStack.top();
      s_initialStyleStack.pop();

      handleUnclosedStyleCommands();

      warn_doc_error(s_fileName, doctokenizerYYlineno, "End of comment block while expecting "
                     "command </%s>", csPrintable(sc.styleString()));
   }
}

static void handleLinkedWord(DocNode *parent, QList<DocNode *> &children, bool ignoreAutoLinkFlag = false)
{
   static const bool autolinkSupport = Config::getBool("auto-link");

   QString name = linkToText(SrcLangExt_Unknown, g_token->name, true);

   if (! autolinkSupport && ! ignoreAutoLinkFlag) {
      // no autolinking so add as normal word
      children.append(new DocWord(parent, name));
      return;
   }

   // try to turn the word 'name' into a link
   QSharedPointer<Definition> compound;
   QSharedPointer<MemberDef>  member;
   QSharedPointer<ClassDef>   cd;

   bool ambig;
   int len = g_token->name.length();

   QSharedPointer<FileDef> fd = findFileDef(&Doxy_Globals::inputNameDict, s_fileName, ambig);
   bool partA = false;

   if (! s_insideHtmlLink) {
      QString tName = g_token->name;

      // added 01/2016
      tName = renameNS_Aliases(tName, false);

      if (resolveRef(s_context, tName, s_inSeeBlock, &compound, &member, false, fd, true)) {
         partA = true;

      } else if ( ! s_context.isEmpty() )  {
         // tried once with s_context now try again with "" looking for a global scope

         if (resolveRef("", tName, s_inSeeBlock, &compound, &member, true, QSharedPointer<FileDef>(), true))  {
            partA = true;
         }
      }
   }

   if (partA) {

      if (member && member->isLinkable()) {

         if (member->isObjCMethod()) {
            bool localLink = s_memberDef ? member->getClassDef() == s_memberDef->getClassDef() : false;
            name = member->objCMethodName(localLink, s_inSeeBlock);
         }

         children.append(new DocLinkedWord(parent, name, member->getReference(), member->getOutputFileBase(),
                                           member->anchor(), member->briefDescriptionAsTooltip() ) );

      } else if (compound->isLinkable()) {
         // compound link
         QString anchor = compound->anchor();

         if (compound->definitionType() == Definition::TypeFile) {
            name = g_token->name;

         } else if (compound->definitionType() == Definition::TypeGroup) {
            name = compound.dynamicCast<GroupDef>()->groupTitle();

         }

         if (s_context == g_token->name) {
            // do not add a link to myself
            children.append(new DocWord(parent, name));

         } else {
            children.append(new DocLinkedWord(parent, name, compound->getReference(), compound->getOutputFileBase(),
                  anchor, compound->briefDescriptionAsTooltip() ) );
         }

      } else if (compound->definitionType() == Definition::TypeFile && compound.dynamicCast<FileDef>()->generateSourceFile() ) {

         // undocumented file that has source code we can link to
         children.append(new DocLinkedWord(parent, g_token->name, compound->getReference(), compound->getSourceFileBase(),
                  "", compound->briefDescriptionAsTooltip() ) );

      } else {
         // not linkable
         children.append(new DocWord(parent, name));
      }

   } else if (! s_insideHtmlLink && len > 1 && g_token->name.at(len - 1) == ':') {
      // special case where matching Foo: fails to be an Obj-C reference, but Foo itself might be linkable
      g_token->name = g_token->name.left(len - 1);

      handleLinkedWord(parent, children, ignoreAutoLinkFlag);
      children.append(new DocWord(parent, ":"));

   } else if (! s_insideHtmlLink && (cd = getClass(g_token->name + "-p"))) {
      // special case: the token name is not a class, but could be a Obj-C protocol

      children.append(new DocLinkedWord(parent, name, cd->getReference(), cd->getOutputFileBase(),
                  cd->anchor(), cd->briefDescriptionAsTooltip()));

   } else {
      // normal non-linkable word

      if (g_token->name.startsWith("#") || g_token->name.startsWith("::")) {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Explicit link request to '%s' could not be resolved",
                  csPrintable(name));

         children.append(new DocWord(parent, g_token->name));

      } else {
         children.append(new DocWord(parent, name));

      }
   }
}

static void handleParameterType(DocNode *parent, QList<DocNode *> &children, const QString &paramTypes)
{
   QString name = g_token->name;
   int p = 0;
   int i;

   while ((i = paramTypes.indexOf('|', p)) != -1) {
      g_token->name = paramTypes.mid(p, i - p);

      handleLinkedWord(parent, children);
      p = i + 1;
   }

   g_token->name = paramTypes.mid(p);
   handleLinkedWord(parent, children);

   g_token->name = name;
}

static DocInternalRef *handleInternalRef(DocNode *parent)
{
   int tok = doctokenizerYYlex();
   QString tokenName = g_token->name;

   if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command", csPrintable(tokenName));
      return 0;
   }

   doctokenizerYYsetStateInternalRef();
   tok = doctokenizerYYlex(); // get the reference id

   if (tok != TK_WORD && tok != TK_LNKWORD) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of %s",
                  csPrintable(tokToString(tok)), csPrintable(tokenName));
      return 0;
   }

   return new DocInternalRef(parent, g_token->name);
}

static DocAnchor *handleAnchor(DocNode *parent)
{
   int tok = doctokenizerYYlex();

   if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command",
                     csPrintable(g_token->name));
      return 0;
   }

   doctokenizerYYsetStateAnchor();
   tok = doctokenizerYYlex();   // get the anchor id

   if (tok == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment block while parsing the "
                     "argument of command %s", csPrintable(g_token->name));
      return 0;

   } else if (tok != TK_WORD && tok != TK_LNKWORD) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of %s",
                  csPrintable(tokToString(tok)), csPrintable(g_token->name));
      return 0;
   }

   DocAnchor *retval = new DocAnchor(parent, g_token->name, false);

   doctokenizerYYsetStatePara();

   return retval;
}

static void handleAnchorName(DocNode *parent)
{
   (void) parent;

   int tok = doctokenizerYYlex();

   if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command", csPrintable(g_token->name));
      return;
   }

   tok = doctokenizerYYlex();   // get the anchor id

   if (tok == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment block while parsing the "
                     "argument of command %s", csPrintable(g_token->name));
      return;

   } else if (tok != TK_WORD && tok != TK_LNKWORD) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of %s",
                  csPrintable(tokToString(tok)), csPrintable(g_token->name));
      return;
   }

   // save anchor id
   QString id = g_token->name;

   // skip whitespace
   tok = doctokenizerYYlex();

   if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\anchorname command");
      return;
   }

   // get the title
   tok = doctokenizerYYlex();

   QString title = g_token->name;
   title = title.mid(1, title.length()-2);

   QSharedPointer<SectionInfo> sec = Doxy_Globals::sectionDict.find(id);

   if (sec) {
      sec->title = title;

   } else {
     warn_doc_error(s_fileName, doctokenizerYYlineno, "Unable to find anchor %s for anchorname", csPrintable(id));

   }

   doctokenizerYYsetStatePara();
}


/* Helper function that deals with the title, width, and height arguments of various commands.
 * @param[in] cmd        Command id for which to extract caption and size info.
 * @param[in] parent     Parent node, owner of the children list passed as
 *                       the third argument.
 * @param[in] children   The list of child nodes to which the node representing
 *                       the token can be added.
 * @param[out] width     the extracted width specifier
 * @param[out] height    the extracted height specifier
 */
static void defaultHandleTitleAndSize(const int cmd, DocNode *parent, QList<DocNode *> &children,
                  QString &width, QString &height)
{
   s_nodeStack.push(parent);

   // parse title
   doctokenizerYYsetStateTitle();
   int tok;

   while ((tok = doctokenizerYYlex())) {

      if (tok == TK_WORD && (g_token->name == "width=" || g_token->name == "height=")) {
         // special case: no title, but we do have a size indicator
         break;
       }

      if (! defaultHandleToken(parent,tok,children))    {

         switch (tok) {
            case TK_COMMAND_AT:
            case TK_COMMAND_BS:  {
               warn_doc_error(s_fileName, doctokenizerYYlineno," Illegal command %s as part of a \\%s",
               csPrintable(g_token->name), csPrintable(Mappers::cmdMapper->map(cmd)) );
               break;
            }

            case TK_SYMBOL:
               warn_doc_error(s_fileName, doctokenizerYYlineno," Unsupported symbol %s found", csPrintable(g_token->name));
               break;

            default:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s", csPrintable(tokToString(tok)));
               break;
         }
      }
   }

   // parse size attributes
   if (tok == 0) {
     tok = doctokenizerYYlex();
   }

   while (tok == TK_WHITESPACE || tok == TK_WORD)  {
      // there are values following the title

      if (tok == TK_WORD) {

         if (g_token->name == "width=" || g_token->name == "height=") {
            doctokenizerYYsetStateTitleAttrValue();
            g_token->name = g_token->name.left(g_token->name.length() - 1);
         }

         if (g_token->name == "width")  {
            width = g_token->chars;

         } else if (g_token->name == "height")  {
            height = g_token->chars;

         } else {
            warn_doc_error(s_fileName,doctokenizerYYlineno,"Unknown option %s after \\%s command, expected 'width' or 'height'",
                     csPrintable(g_token->name), csPrintable(Mappers::cmdMapper->map(cmd)));

            break;
         }
      }

      tok = doctokenizerYYlex();
   }

   doctokenizerYYsetStatePara();

   handlePendingStyleCommands(parent,children);

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == parent);
}


/* Helper function that deals with the most common tokens allowed in
 * title like sections.
 * @param parent     Parent node, owner of the children list passed as
 *                   the third argument.
 * @param tok        The token to process.
 * @param children   The list of child nodes to which the node representing
 *                   the token can be added.
 * @param handleWord Indicates if word token should be processed
 * @retval true      The token was handled.
 * @retval false     The token was not handled.
 */
static bool defaultHandleToken(DocNode *parent, int tok, QList<DocNode *> &children, bool handleWord)
{
   DBG(("token %s at %d", csPrintable(tokToString(tok)), doctokenizerYYlineno));

   if (tok == TK_WORD || tok == TK_LNKWORD || tok == TK_SYMBOL || tok == TK_URL ||
         tok == TK_COMMAND_AT  || tok == TK_COMMAND_BS || tok == TK_HTMLTAG) {
      DBG((" name=%s", csPrintable(g_token->name)));
   }

   DBG(("\n"));

   while (true) {
      QString tokenName = g_token->name;

      switch (tok) {
         case TK_COMMAND_AT:
         case TK_COMMAND_BS:
            switch (Mappers::cmdMapper->map(tokenName)) {

               case CMD_BSLASH:
                  children.append(new DocSymbol(parent, DocSymbol::Sym_BSlash));
                  break;

               case CMD_AT:
                  children.append(new DocSymbol(parent, DocSymbol::Sym_At));
                  break;

               case CMD_LESS:
                  children.append(new DocSymbol(parent, DocSymbol::Sym_Less));
                  break;

               case CMD_GREATER:
                  children.append(new DocSymbol(parent, DocSymbol::Sym_Greater));
                  break;

               case CMD_AMP:
                  children.append(new DocSymbol(parent, DocSymbol::Sym_Amp));
                  break;

               case CMD_DOLLAR:
                  children.append(new DocSymbol(parent, DocSymbol::Sym_Dollar));
                  break;

               case CMD_HASH:
                  children.append(new DocSymbol(parent, DocSymbol::Sym_Hash));
                  break;

               case CMD_DCOLON:
                  children.append(new DocSymbol(parent, DocSymbol::Sym_DoubleColon));
                  break;

               case CMD_PERCENT:
                  children.append(new DocSymbol(parent, DocSymbol::Sym_Percent));
                  break;

               case CMD_NDASH:
                  children.append(new DocSymbol(parent, DocSymbol::Sym_Minus));
                  children.append(new DocSymbol(parent, DocSymbol::Sym_Minus));
                  break;

               case CMD_MDASH:
                  children.append(new DocSymbol(parent, DocSymbol::Sym_Minus));
                  children.append(new DocSymbol(parent, DocSymbol::Sym_Minus));
                  children.append(new DocSymbol(parent, DocSymbol::Sym_Minus));
                  break;

               case CMD_QUOTE:
                  children.append(new DocSymbol(parent, DocSymbol::Sym_Quot));
                  break;

               case CMD_PUNT:
                  children.append(new DocSymbol(parent, DocSymbol::Sym_Dot));
                  break;

               case CMD_PLUS:
                  children.append(new DocSymbol(parent, DocSymbol::Sym_Plus));
                  break;

               case CMD_MINUS:
                  children.append(new DocSymbol(parent, DocSymbol::Sym_Minus));
                  break;

               case CMD_EQUAL:
                  children.append(new DocSymbol(parent,DocSymbol::Sym_Equal));
                  break;

               case CMD_EMPHASIS: {
                  children.append(new DocStyleChange(parent, s_nodeStack.count(), DocStyleChange::Italic, true));
                  tok = handleStyleArgument(parent, children, tokenName);
                  children.append(new DocStyleChange(parent, s_nodeStack.count(), DocStyleChange::Italic, false));

                  if (tok != TK_WORD) {
                     children.append(new DocWhiteSpace(parent, " "));
                  }

                  if (tok == TK_NEWPARA) {
                     if (insidePRE(parent) || !children.isEmpty()) {
                        children.append(new DocWhiteSpace(parent, g_token->chars));
                     }
                     break;

                  } else if (tok == TK_WORD || tok == TK_HTMLTAG) {
                     DBG(("CMD_EMPHASIS: reparsing command %s\n", csPrintable(g_token->name)));
                     continue;
                  }
               }

               break;

               case CMD_BOLD: {
                  children.append(new DocStyleChange(parent, s_nodeStack.count(), DocStyleChange::Bold, true));
                  tok = handleStyleArgument(parent, children, tokenName);
                  children.append(new DocStyleChange(parent, s_nodeStack.count(), DocStyleChange::Bold, false));

                  if (tok != TK_WORD) {
                     children.append(new DocWhiteSpace(parent, " "));
                  }

                  if (tok == TK_NEWPARA) {
                   if (insidePRE(parent) || !children.isEmpty()) {
                        children.append(new DocWhiteSpace(parent, g_token->chars));
                     }
                     break;

                  } else if (tok == TK_WORD || tok == TK_HTMLTAG) {
                     DBG(("CMD_BOLD: reparsing command %s\n", csPrintable(g_token->name)));
                     continue;
                  }
               }
               break;

               case CMD_CODE: {
                  children.append(new DocStyleChange(parent, s_nodeStack.count(), DocStyleChange::Code, true));
                  tok = handleStyleArgument(parent, children, tokenName);
                  children.append(new DocStyleChange(parent, s_nodeStack.count(), DocStyleChange::Code, false));

                  if (tok != TK_WORD) {
                     children.append(new DocWhiteSpace(parent, " "));
                  }

                  if (tok == TK_NEWPARA) {
                     if (insidePRE(parent) || !children.isEmpty()) {
                        children.append(new DocWhiteSpace(parent, g_token->chars));
                     }
                     break;

                  } else if (tok == TK_WORD || tok == TK_HTMLTAG) {
                     DBG(("CMD_CODE: reparsing command %s\n", csPrintable(g_token->name)));
                     continue;
                  }
               }
               break;

               case CMD_HTMLONLY: {
                  doctokenizerYYsetStateHtmlOnly();
                  tok = doctokenizerYYlex();
                  children.append(new DocVerbatim(parent, s_context, g_token->verb, DocVerbatim::HtmlOnly,
                                  s_isExample, s_exampleName, g_token->name == "block"));

                  if (tok == 0) {
                     warn_doc_error(s_fileName, doctokenizerYYlineno, "Htmlonly section ended without an end marker");
                  }
                  doctokenizerYYsetStatePara();
               }
               break;

               case CMD_MANONLY: {
                  doctokenizerYYsetStateManOnly();
                  tok = doctokenizerYYlex();
                  children.append(new DocVerbatim(parent, s_context, g_token->verb, DocVerbatim::ManOnly, s_isExample, s_exampleName));

                  if (tok == 0) {
                     warn_doc_error(s_fileName, doctokenizerYYlineno, "Manonly section ended without an end marker");
                  }

                  doctokenizerYYsetStatePara();
               }
               break;

               case CMD_RTFONLY: {
                  doctokenizerYYsetStateRtfOnly();
                  tok = doctokenizerYYlex();
                  children.append(new DocVerbatim(parent, s_context, g_token->verb, DocVerbatim::RtfOnly, s_isExample, s_exampleName));

                  if (tok == 0) {
                     warn_doc_error(s_fileName, doctokenizerYYlineno, "Rtfonly section ended without an end marker");
                  }
                  doctokenizerYYsetStatePara();
               }
               break;

               case CMD_LATEXONLY: {
                  doctokenizerYYsetStateLatexOnly();
                  tok = doctokenizerYYlex();
                  children.append(new DocVerbatim(parent, s_context, g_token->verb, DocVerbatim::LatexOnly, s_isExample, s_exampleName));
                  if (tok == 0) {
                     warn_doc_error(s_fileName, doctokenizerYYlineno, "Latexonly section ended without an end marker", doctokenizerYYlineno);
                  }
                  doctokenizerYYsetStatePara();
               }
               break;

               case CMD_XMLONLY: {
                  doctokenizerYYsetStateXmlOnly();
                  tok = doctokenizerYYlex();
                  children.append(new DocVerbatim(parent, s_context, g_token->verb, DocVerbatim::XmlOnly, s_isExample, s_exampleName));
                  if (tok == 0) {
                     warn_doc_error(s_fileName, doctokenizerYYlineno, "Xmlonly section ended without an end marker", doctokenizerYYlineno);
                  }
                  doctokenizerYYsetStatePara();
               }
               break;

               case CMD_DBONLY: {
                  doctokenizerYYsetStateDbOnly();
                  tok = doctokenizerYYlex();
                  children.append(new DocVerbatim(parent, s_context, g_token->verb, DocVerbatim::DocbookOnly, s_isExample, s_exampleName));
                  if (tok == 0) {
                     warn_doc_error(s_fileName, doctokenizerYYlineno, "Docbookonly section ended without an end marker",
                           doctokenizerYYlineno);
                  }
                  doctokenizerYYsetStatePara();
               }
               break;

               case CMD_FORMULA: {
                  DocFormula *form = new DocFormula(parent, g_token->id);
                  children.append(form);
               }
               break;

               case CMD_ANCHOR: {
                  DocAnchor *anchor = handleAnchor(parent);
                  if (anchor) {
                     children.append(anchor);
                  }
               }
               break;

               case CMD_ANCHORNAME: {
                  handleAnchorName(parent);
               }
               break;

               case CMD_INTERNALREF: {
                  DocInternalRef *ref = handleInternalRef(parent);
                  if (ref) {
                     children.append(ref);
                     ref->parse();
                  }
                  doctokenizerYYsetStatePara();
               }
               break;

               case CMD_SETSCOPE: {
                  QString scope;

                  doctokenizerYYsetStateSetScope();
                  doctokenizerYYlex();

                  scope = g_token->name;
                  s_context = scope;

                  doctokenizerYYsetStatePara();
               }
               break;

               default:
                  return false;
            }
            break;

         case TK_HTMLTAG: {
            switch (Mappers::htmlTagMapper->map(tokenName)) {
               case HTML_DIV:
                  warn_doc_error(s_fileName, doctokenizerYYlineno, "Found <div> tag in heading\n");
                  break;

               case HTML_PRE:
                  warn_doc_error(s_fileName, doctokenizerYYlineno, "Found <pre> tag in heading\n");
                  break;

               case HTML_BOLD:
                  if (! g_token->endTag) {
                     handleStyleEnter(parent, children, DocStyleChange::Bold, &g_token->attribs);
                  } else {
                     handleStyleLeave(parent, children, DocStyleChange::Bold, tokenName);
                  }
                  break;

               case HTML_STRIKE:
                  if (! g_token->endTag) {
                     handleStyleEnter(parent,children, DocStyleChange::Strike, &g_token->attribs);
                  } else {
                     handleStyleLeave(parent,children,DocStyleChange::Strike, tokenName);
                  }

                  break;

                case HTML_DEL:
                  if (! g_token->endTag) {
                     handleStyleEnter(parent,children, DocStyleChange::Del, &g_token->attribs);
                  } else {
                     handleStyleLeave(parent,children, DocStyleChange::Del, tokenName);
                  }

                  break;

               case HTML_UNDERLINE:
                  if (! g_token->endTag) {
                     handleStyleEnter(parent,children, DocStyleChange::Underline, &g_token->attribs);
                  } else {
                     handleStyleLeave(parent,children, DocStyleChange::Underline, tokenName);
                  }

                  break;

               case HTML_INS:
                  if (! g_token->endTag) {
                     handleStyleEnter(parent,children, DocStyleChange::Ins, &g_token->attribs);
                  } else {
                     handleStyleLeave(parent,children, DocStyleChange::Ins,tokenName);
                  }
                  break;

               case HTML_CODE:
               case XML_C:
                  if (! g_token->endTag) {
                     handleStyleEnter(parent, children, DocStyleChange::Code, &g_token->attribs);
                  } else {
                     handleStyleLeave(parent, children, DocStyleChange::Code, tokenName);
                  }
                  break;

               case HTML_EMPHASIS:
                  if (!g_token->endTag) {
                     handleStyleEnter(parent, children, DocStyleChange::Italic, &g_token->attribs);
                  } else {
                     handleStyleLeave(parent, children, DocStyleChange::Italic, tokenName);
                  }
                  break;
               case HTML_SUB:
                  if (!g_token->endTag) {
                     handleStyleEnter(parent, children, DocStyleChange::Subscript, &g_token->attribs);
                  } else {
                     handleStyleLeave(parent, children, DocStyleChange::Subscript, tokenName);
                  }
                  break;

               case HTML_SUP:
                  if (!g_token->endTag) {
                     handleStyleEnter(parent, children, DocStyleChange::Superscript, &g_token->attribs);
                  } else {
                     handleStyleLeave(parent, children, DocStyleChange::Superscript, tokenName);
                  }
                  break;

               case HTML_CENTER:
                  if (!g_token->endTag) {
                     handleStyleEnter(parent, children, DocStyleChange::Center, &g_token->attribs);
                  } else {
                     handleStyleLeave(parent, children, DocStyleChange::Center, tokenName);
                  }
                  break;

               case HTML_SMALL:
                  if (!g_token->endTag) {
                     handleStyleEnter(parent, children, DocStyleChange::Small, &g_token->attribs);
                  } else {
                     handleStyleLeave(parent, children, DocStyleChange::Small, tokenName);
                  }
                  break;
               default:
                  return false;
                  break;
            }
         }
         break;

         case TK_SYMBOL: {
            DocSymbol::SymType s = DocSymbol::decodeSymbol(tokenName);
            if (s != DocSymbol::Sym_Unknown) {
               children.append(new DocSymbol(parent, s));
            } else {
               return false;
            }
         }
         break;

         case TK_WHITESPACE:
         case TK_NEWPARA:
            if (insidePRE(parent) || !children.isEmpty()) {
               children.append(new DocWhiteSpace(parent, g_token->chars));
            }
            break;

         case TK_LNKWORD:
            if (handleWord) {
               handleLinkedWord(parent, children);
            } else {
               return false;
            }
            break;

         case TK_WORD:
            if (handleWord) {
               children.append(new DocWord(parent, g_token->name));
            } else {
               return false;
            }
            break;

         case TK_URL:
            if (s_insideHtmlLink) {
               children.append(new DocWord(parent, g_token->name));
            } else {
               children.append(new DocURL(parent, g_token->name, g_token->isEMailAddr));
            }
            break;

         default:
            return false;
      }

      break;
   }

   return true;
}

static void handleImg(DocNode *parent, QList<DocNode *> &children, const HtmlAttribList &tagHtmlAttribs)
{
   bool found = false;
   int index = 0;

   for (auto opt : tagHtmlAttribs) {

      if (opt.name == "src" && ! opt.value.isEmpty()) {
         // copy attributes
         HtmlAttribList attrList = tagHtmlAttribs;

         // and remove the src attribute
         attrList.removeAt(index);

         DocImage *img = new DocImage(parent, attrList, findAndCopyImage(opt.value, DocImage::Html, false), DocImage::Html, opt.value);
         children.append(img);
         found = true;
      }

      ++index;
   }

   if (! found) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "IMG tag does not have a SRC attribute\n");
   }
}

DocSymbol::SymType DocSymbol::decodeSymbol(const QString &symName)
{
   DBG(("decodeSymbol(%s)\n", csPrintable(symName)));
   return HtmlEntityMapper::instance()->name2sym(symName);
}


DocEmoji::DocEmoji(DocNode *parent, const QString &symName)
   : m_symName(symName), m_index(-1)
{
   m_parent = parent;

   QString locSymName = symName;
   int len = locSymName.length();

   if (len > 0) {
      if (! locSymName.endsWith(":")) {
         locSymName.append(":");
      }

      if (! locSymName.startsWith(":")) {
         locSymName.prepend(":");
      }
   }

   m_symName = locSymName;
   m_index   = EmojiEntityMapper::instance()->symbol2index(m_symName);

   if (m_index == -1) {
      warn_doc_error(s_fileName, doctokenizerYYlineno," Found unsupported emoji symbol '%s'\n", csPrintable(m_symName));
   }
}
static int internalValidatingParseDoc(DocNode *parent, QList<DocNode *> &children, const QString &doc)
{
   int retval = RetVal_OK;

   if (doc.isEmpty()) {
      return retval;
   }

   doctokenizerYYinit(doc, s_fileName);

   // first parse any number of paragraphs
   bool isFirst = true;
   DocPara *lastPar = 0;

   if (! children.isEmpty() && children.last()->kind() == DocNode::Kind_Para) {
      // last child item was a paragraph
      lastPar = (DocPara *)children.last();
      isFirst = false;
   }

   do {
      DocPara *par = new DocPara(parent);

      if (isFirst) {
         par->markFirst();
         isFirst = false;
      }

      retval = par->parse();

      if (! par->isEmpty()) {
         children.append(par);

         if (lastPar) {
            lastPar->markLast(false);
         }

         lastPar = par;

      } else {
         delete par;
      }

   } while (retval == TK_NEWPARA);


   if (lastPar) {
      lastPar->markLast();
   }

   return retval;
}

static void readTextFileByName(const QString &file, QString &text)
{
   const QStringList examplePathList = Config::getList("example-source");
   const bool filterSourceFiles      = Config::getBool("filter-source-files");

   if (! QDir::isAbsolutePath(file)) {
      QFileInfo fi(file);

      if (fi.exists()) {
         text = fileToString(file, filterSourceFiles);
         return;
      }
   }

   for (auto s : examplePathList) {
      QString absFileName = s +  QDir::separator() + file;

      QFileInfo fi(absFileName);

      if (fi.exists()) {
         text = fileToString(absFileName, filterSourceFiles);
         return;
      }
   }

   // as a fallback we also look in the exampleNameDict
   bool ambig;
   QSharedPointer<FileDef> fd;

   if ((fd = findFileDef(&Doxy_Globals::exampleNameDict, file, ambig))) {
      text = fileToString(fd->getFilePath(), filterSourceFiles);

   } else if (ambig) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "included file name %s is ambiguous"
                  "Possible candidates:\n%s", csPrintable(file),
                  csPrintable(showFileDefMatches(Doxy_Globals::exampleNameDict, file)));

   } else {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Included file %s was not found. "
                     "Check the EXAMPLE PATH in your project file.", csPrintable(file));
   }
}

DocWord::DocWord(DocNode *parent, const QString &word)
   : m_word(word)
{
   m_parent = parent;

   if (Doxy_Globals::searchIndexBase != nullptr && ! s_searchUrl.isEmpty()) {
      Doxy_Globals::searchIndexBase->addWord(word, false);
   }
}

DocLinkedWord::DocLinkedWord(DocNode *parent, const QString &word, const QString &ref, const QString &file,
                             const QString &anchor, const QString &tooltip)
   : m_word(word), m_ref(ref), m_file(file), m_relPath(s_relPath), m_anchor(anchor), m_tooltip(tooltip)
{
   m_parent = parent;

   if (Doxy_Globals::searchIndexBase != nullptr && !s_searchUrl.isEmpty()) {
      Doxy_Globals::searchIndexBase->addWord(word, false);
   }
}

DocAnchor::DocAnchor(DocNode *parent, const QString &id, bool newAnchor)
{
   m_parent = parent;

   if (id.isEmpty()) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Empty anchor label");
   }

   if (newAnchor) {
      // found <a name="label">
      m_anchor = id;

   } else if (id.left(CiteConsts::anchorPrefix.length()) == CiteConsts::anchorPrefix) {
      QString citeValue = Doxy_Globals::citeDict.find(id.mid(CiteConsts::anchorPrefix.length()));

      if (citeValue.isEmpty()) {
         m_file   = convertNameToFile_X(CiteConsts::fileName, false, true);
         m_anchor = id;

      } else {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Invalid cite anchor id '%s'", csPrintable(id));
         m_file   = "invalid";
         m_anchor = "invalid";

      }

   } else {
      // found \anchor label
      QSharedPointer<SectionInfo> sec = Doxy_Globals::sectionDict.find(id);

      if (sec) {
         m_file   = sec->fileName;
         m_anchor = sec->label;

         if (s_sectionDict && s_sectionDict->find(id) == 0) {
            // insert in dictionary
            s_sectionDict->insert(id, sec);
         }

      } else {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Invalid anchor id '%s'", csPrintable(id));
         m_file   = "invalid";
         m_anchor = "invalid";

      }
   }
}

DocVerbatim::DocVerbatim(DocNode *parent, const QString &context, const QString &text, Type t, bool isExample,
                         const QString &exampleFile, bool isBlock, const QString &lang)
   : m_context(context), m_text(text), m_type(t), m_isExample(isExample), m_exampleFile(exampleFile),
     m_relPath(s_relPath), m_lang(lang), m_isBlock(isBlock)
{
   m_parent = parent;
}

void DocInclude::parse()
{
   DBG(("DocInclude::parse(file=%s,text=%s)\n", csPrintable(m_file), csPrintable(m_text)));

   switch (m_type) {
      case DontIncWithLines:
      case IncWithLines:
      case Include:
      case DontInclude:
         readTextFileByName(m_file, m_text);
         s_includeFileName   = m_file;
         s_includeFileText   = m_text;
         s_includeFileOffset = 0;
         s_includeFileLength = m_text.length();
         s_includeFileLine   = 0;
         s_includeFileUseLN  = (m_type == DontIncWithLines || m_type == IncWithLines);
         break;

      case VerbInclude:
      case HtmlInclude:
      case LatexInclude:
      case RtfInclude:
      case ManInclude:
      case XmlInclude:
      case DocbookInclude:
         readTextFileByName(m_file, m_text);
         break;

      case Snippet:
      case SnipWithLines:
         readTextFileByName(m_file, m_text);
         int count;

         // check for the existence of the blockId inside the file, so the warning is only generated once
         if (! m_blockId.isEmpty() && (count = m_text.count(m_blockId)) != 2) {

            warn_doc_error(s_fileName, doctokenizerYYlineno, "Block marked with %s for \\snippet should appear twice "
                "in file %s, found it %d times\n", csPrintable(m_blockId), csPrintable(m_file), count);
         }
         break;

      case DocInclude::IncludeDoc:
      case DocInclude::SnippetDoc:
         err("Unexpected command found for IncludeDoc or SnippetDoc in file: %s,"
               " contact the developers\n", csPrintable(m_file));

         break;
   }
}

void DocIncOperator::parse()
{
   if (s_includeFileName.isEmpty()) {
      QString cmd;

      switch(type()) {
         case Line:
            cmd = "\\line";
            break;

         case SkipLine:
            cmd = "\\skipLine";
            break;

         case Skip:
            cmd = "\\skip";
            break;

         case Until:
            cmd = "\\until";
            break;
      }

      warn_doc_error(s_fileName, doctokenizerYYlineno,
          "No previous '\\include' or \\dontinclude' command for '%s' present", csPrintable(cmd));
   }

   m_includeFileName = s_includeFileName;

   uint lineNo = s_includeFileLine;

   QString::const_iterator iter_offset = s_includeFileText.constBegin() + s_includeFileOffset;
   QString::const_iterator iter_end    = s_includeFileText.constEnd();

   QString::const_iterator iter_so     = iter_offset;
   QString::const_iterator iter_bo;

   bool nonEmpty = false;

   switch (type()) {

      case Line:  {
         while (iter_offset != iter_end) {
            QChar c = *iter_offset;

            if (c == '\n') {
               ++s_includeFileLine;

               if (nonEmpty) {
                  // pattern to match
                  break;
               }

               // no pattern, skip empty line
               iter_so = iter_offset + 1;

            } else if (! c.isSpace() ) {
               // no white space char
               nonEmpty = true;
            }

            ++iter_offset;
         }

         QStringView tmp = QStringView(iter_so, iter_offset);

         if (tmp.contains(m_pattern)) {
            m_line = lineNo;
            m_text = tmp;
         }

         // set pointer to start of new line
         s_includeFileOffset = iter_offset - s_includeFileText.constBegin();

         if (iter_offset != iter_end) {
            s_includeFileOffset += 1;
         }

         m_showLineNo = s_includeFileUseLN;

         break;
      }

      case SkipLine: {
         while (iter_offset != iter_end) {
            iter_so = iter_offset;

            while (iter_offset != iter_end) {
               QChar c = *iter_offset;

               if (c == '\n') {
                  ++s_includeFileLine;
                  if (nonEmpty) {
                     // we have a pattern to match
                     break;
                  }

                  // no pattern, skip empty line
                  iter_so = iter_offset + 1;

               } else if (! c.isSpace() ) {
                   // no white space char
                  nonEmpty = true;
               }

               ++iter_offset;
            }

            QStringView tmp = QStringView(iter_so, iter_offset);

            if (tmp.contains(m_pattern)) {
               m_line = lineNo;
               m_text = tmp;
               break;
            }

            // skip new line
            ++iter_offset;
         }

         // set pointer to start of new line
         s_includeFileOffset = iter_offset - s_includeFileText.constBegin();

         if (iter_offset != iter_end) {
            s_includeFileOffset += 1;
         }

         m_showLineNo = s_includeFileUseLN;

         break;
      }

      case Skip: {
         while (iter_offset != iter_end) {
            iter_so = iter_offset;

            while (iter_offset != iter_end) {
               QChar c = *iter_offset;

               if (c == '\n') {
                  ++s_includeFileLine;

                  if (nonEmpty) {
                     // we have a pattern to match
                     break;
                  }

                  // no pattern, skip empty line
                  iter_so = iter_offset + 1;

               } else if (! c.isSpace() ) {
                  // no white space char
                  nonEmpty = true;
               }

               ++iter_offset;
            }

            QStringView tmp = QStringView(iter_so, iter_offset);

            if (tmp.contains(m_pattern)) {
               break;
            }

            // skip new line
            ++iter_offset;
         }

         // set pointer to start of new line
         s_includeFileOffset = iter_so - s_includeFileText.constBegin();
         m_showLineNo = s_includeFileUseLN;

         break;
      }

      case Until: {
         iter_bo = iter_offset;

         while (iter_offset != iter_end) {
            iter_so = iter_offset;

            while (iter_offset != iter_end) {
               QChar c = *iter_offset;

               if (c == '\n') {
                  ++s_includeFileLine;
                  if (nonEmpty) {
                     // we have a pattern to match
                     break;
                  }

                  // no pattern, skip empty line
                  iter_so = iter_offset + 1;

               } else if (! c.isSpace() ) {
                  // no white space char
                  nonEmpty = true;
               }

               ++iter_offset;
            }

            QStringView tmp = QStringView(iter_so, iter_offset);

            if (tmp.contains(m_pattern)) {
               m_line = lineNo;
               m_text = QStringView(iter_bo, iter_offset);      // unusual to use iter_bo
               break;
            }

            // skip new line
            ++iter_offset;
         }

         // set pointer to start of new line
         s_includeFileOffset = iter_offset - s_includeFileText.constBegin();

         if (iter_offset != iter_end) {
            s_includeFileOffset += 1;
         }

         m_showLineNo = s_includeFileUseLN;

         break;
      }
   }
}

void DocCopy::parse(QList<DocNode *> &children)
{
   QString doc;
   QString brief;

   QSharedPointer<Definition> def = findDocsForMemberOrCompound(m_link, doc, brief);

   if (def) {

      if (s_copyStack.indexOf(def) == -1) {
         // definition not parsed earlier

         bool  hasParamCommand  = s_hasParamCommand;
         bool  hasReturnCommand = s_hasReturnCommand;

         QSet<QString> paramsFound  = s_paramsFound;

         docParserPushContext(false);
         s_scope = def;

         if (def->definitionType() == Definition::TypeMember && def->getOuterScope()) {
            if (def->getOuterScope() != Doxy_Globals::globalScope) {
               s_context = def->getOuterScope()->name();
            }

         } else if (def != Doxy_Globals::globalScope) {
            s_context = def->name();

         }

         s_styleStack.clear();
         s_nodeStack.clear();
         s_paramsFound.clear();
         s_copyStack.append(def);

         // make sure the descriptions end with a newline, so the parser will correctly
         // handle them in all cases

         if (m_copyBrief) {
            brief += '\n';
            internalValidatingParseDoc(m_parent, children, brief);

            hasParamCommand  = hasParamCommand  || s_hasParamCommand;
            hasReturnCommand = hasReturnCommand || s_hasReturnCommand;

            paramsFound.unite(s_paramsFound);
         }

         if (m_copyDetails) {
            doc += '\n';
            internalValidatingParseDoc(m_parent, children, doc);

            hasParamCommand  = hasParamCommand  || s_hasParamCommand;
            hasReturnCommand = hasReturnCommand || s_hasReturnCommand;

            paramsFound.unite(s_paramsFound);
         }

         s_copyStack.removeOne(def);
         assert(s_styleStack.isEmpty());
         assert(s_nodeStack.isEmpty());

         docParserPopContext(true);

         s_hasParamCommand  = hasParamCommand;
         s_hasReturnCommand = hasReturnCommand;
         s_paramsFound      = paramsFound;

      } else {
         // oops, recursion
         warn_doc_error(s_fileName, doctokenizerYYlineno, "recursive call chain of \\copydoc commands detected at %d\n",
                        doctokenizerYYlineno);
      }

   } else {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "target %s of \\copydoc command not found", csPrintable(m_link));
   }
}

DocXRefItem::DocXRefItem(DocNode *parent, int id, const QString &key)
    : m_id(id), m_key(key), m_relPath(s_relPath)
{
   m_parent = parent;
}

bool DocXRefItem::parse()
{
   if (Doxy_Globals::xrefLists.contains(m_key)) {

      static const bool todoList  = Config::getBool("generate-todo-list");
      static const bool testList  = Config::getBool("generate-test-list");
      static const bool bugList   = Config::getBool("generate-bug-list");
      static const bool depreList = Config::getBool("generate-deprecate-list");

      auto &refList = Doxy_Globals::xrefLists[m_key];

      if ( (m_key != "todo"       || todoList) &&
           (m_key != "test"       || testList) &&
           (m_key != "bug"        || bugList)  &&
           (m_key != "deprecated" || depreList) ) {

         // either not a built-in list or the list is enabled
         RefItem *item = refList.getRefItem(m_id);
         assert(item != nullptr);

         if (item) {
            if (s_memberDef && ! s_memberDef->name().isEmpty() && s_memberDef->name().at(0) == '@') {
               m_file   = "@";  // can not cross reference anonymous enum
               m_anchor = "@";

            } else {
               m_file   = refList.fileName();
               m_anchor = item->listAnchor;
            }

            m_title  = refList.sectionTitle();

            if (! item->text.isEmpty()) {
               docParserPushContext();
               internalValidatingParseDoc(this, m_children, item->text);
               docParserPopContext();
            }
         }

         return true;
      }
   }

   return false;
}

DocFormula::DocFormula(DocNode *parent, int id)
   : m_relPath(s_relPath)
{
   m_parent = parent;

   QString formCmd;
   formCmd = QString("\\form#%1").formatArg(id);

   auto formula = Doxy_Globals::formulaNameDict.find(formCmd);

   if (formula != Doxy_Globals::formulaNameDict.end()) {
      m_id   = formula->getId();
      m_name = QString("form_%1").formatArg(m_id);

      m_text = formula->getFormulaText();

   } else {
      // wrong \form#<n> command
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Incorrect formula id %d", id);
      m_id = -1;

   }
}

//int DocLanguage::parse()
//{
//  int retval;
//  DBG(("DocLanguage::parse() start\n"));
//  s_nodeStack.push(this);
//
//  // parse one or more paragraphs
//  bool isFirst=true;
//  DocPara *par=0;
//  do
//  {
//    par = new DocPara(this);
//    if (isFirst) { par->markFirst(); isFirst=false; }
//    m_children.append(par);
//    retval=par->parse();
//  }
//  while (retval==TK_NEWPARA);
//  if (par) par->markLast();
//
//  DBG(("DocLanguage::parse() end\n"));
//  DocNode *n = s_nodeStack.pop();
//  assert(n==this);
//  return retval;
//}

void DocSecRefItem::parse()
{
   DBG(("DocSecRefItem::parse() start\n"));
   s_nodeStack.push(this);

   doctokenizerYYsetStateTitle();
   int tok;

   while ((tok = doctokenizerYYlex())) {
      if (! defaultHandleToken(this, tok, m_children)) {
         switch (tok) {
            case TK_COMMAND_AT:
            case TK_COMMAND_BS:   {
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Illegal command %s as part of a \\refitem",
                              csPrintable(g_token->name));
               break;
            }

            case TK_SYMBOL:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unsupported symbol %s found",
                              csPrintable(g_token->name));
               break;

            default:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s", csPrintable(tokToString(tok)) );
               break;
         }
      }
   }

   doctokenizerYYsetStatePara();
   handlePendingStyleCommands(this, m_children);

   QSharedPointer<SectionInfo> sec;

   if (! m_target.isEmpty()) {
      sec = Doxy_Globals::sectionDict.find(m_target);

      if (sec) {
         m_file   = sec->fileName;
         m_anchor = sec->label;

         if (s_sectionDict && s_sectionDict->find(m_target) == 0) {
            s_sectionDict->insert(m_target, sec);
         }

      } else {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "reference to unknown section %s", csPrintable(m_target));
      }

   } else {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "reference to empty target");
   }

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocSecRefItem::parse() end\n"));
}

void DocSecRefList::parse()
{
   DBG(("DocSecRefList::parse() start\n"));
   s_nodeStack.push(this);

   int tok = doctokenizerYYlex();

   // skip white space
   while (tok == TK_WHITESPACE || tok == TK_NEWPARA) {
      tok = doctokenizerYYlex();
   }

   // handle items
   while (tok) {
      if (tok == TK_COMMAND_AT || tok == TK_COMMAND_BS) {
         switch (Mappers::cmdMapper->map(g_token->name)) {
            case CMD_SECREFITEM: {
               int tok = doctokenizerYYlex();
               if (tok != TK_WHITESPACE) {
                  warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\refitem command");
                  break;
               }
               tok = doctokenizerYYlex();
               if (tok != TK_WORD && tok != TK_LNKWORD) {
                  warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of \\refitem",
                                 csPrintable(tokToString(tok)));
                  break;
               }

               DocSecRefItem *item = new DocSecRefItem(this, g_token->name);
               m_children.append(item);
               item->parse();
            }

            break;
            case CMD_ENDSECREFLIST:
               goto endsecreflist;
            default:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Illegal command %s as part of a \\secreflist",
                              csPrintable(g_token->name));
               goto endsecreflist;
         }

      } else if (tok == TK_WHITESPACE) {
         // ignore whitespace

      } else {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s inside section reference list",
                        csPrintable(tokToString(tok)));

         goto endsecreflist;
      }

      tok = doctokenizerYYlex();
   }

endsecreflist:
   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocSecRefList::parse() end\n"));
}

DocInternalRef::DocInternalRef(DocNode *parent, const QString &ref)
   : m_relPath(s_relPath)
{
   m_parent = parent;
   int i = ref.indexOf('#');

   if (i != -1) {
      m_anchor = ref.right(ref.length() - i - 1);
      m_file   = ref.left(i);

   } else {
      m_file = ref;
   }
}

void DocInternalRef::parse()
{
   DBG(("DocInternalRef::parse() start\n"));
   s_nodeStack.push(this);

   int tok;

   while ((tok = doctokenizerYYlex())) {
      if (! defaultHandleToken(this, tok, m_children)) {
         switch (tok) {
            case TK_COMMAND_AT:
            case TK_COMMAND_BS: {
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Illegal command %s as part of a \\ref",
                              csPrintable(g_token->name));
               break;
            }

            case TK_SYMBOL:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unsupported symbol %s found",
                              csPrintable(g_token->name));
               break;
            default:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s", csPrintable(tokToString(tok)));
               break;
         }
      }
   }

   handlePendingStyleCommands(this, m_children);

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocInternalRef::parse() end\n"));
}

DocRef::DocRef(DocNode *parent, const QString &target, const QString &context)
   : m_refType(Unknown), m_isSubPage(false)
{
   m_parent = parent;
   QSharedPointer<Definition> compound;

   QString anchor;
   assert(! target.isEmpty());

   SrcLangExt lang = getLanguageFromFileName(target);

   m_relPath = s_relPath;

   QSharedPointer<SectionInfo> sec = Doxy_Globals::sectionDict.find(target);

   if (sec == nullptr && lang == SrcLangExt_Markdown) {
      // lookup as markdown file
      sec = Doxy_Globals::sectionDict.find(markdownFileNameToId(target));
   }

   if (sec) {
      // ref to section or anchor

      if (sec->dupAnchor_cnt > 0)  {
         warn(s_fileName, doctokenizerYYlineno, "Link to ambiguous anchor '%s', "
            "using first anchor declared in %s, line %d", csPrintable(target),
            csPrintable(sec->dupAnchor_fName), sec->lineNr);
      }

      QSharedPointer<PageDef> pd;

      if (sec->type == SectionInfo::Page) {
         pd = Doxy_Globals::pageSDict.find(target);
      }

      m_text = sec->title;

      if (m_text.isEmpty()) {
         m_text = sec->label;
      }

      m_ref  = sec->ref;
      m_file = stripKnownExtensions(sec->fileName);

      if (sec->type == SectionInfo::Anchor) {
         m_refType = Anchor;

      } else if (sec->type == SectionInfo::Table) {
         m_refType = Table;

      } else {
         m_refType = Section;
      }

      m_isSubPage = pd && pd->hasParentPage();

      if (sec->type != SectionInfo::Page || m_isSubPage) {
         m_anchor = sec->label;
      }

      return;

   } else if (resolveLink(context, target, true, &compound, anchor)) {

      bool isFile = compound ? (compound->definitionType() == Definition::TypeFile ||
                     compound->definitionType() == Definition::TypePage ? true : false) : false;

      m_text = linkToText(compound ? compound->getLanguage() : SrcLangExt_Unknown, target, isFile);
      m_anchor = anchor;

      if (compound && compound->isLinkable()) {
         if (anchor.isEmpty() && compound->definitionType() == Definition::TypeGroup &&
               ! compound.dynamicCast<GroupDef>()->groupTitle().isEmpty() ) {

            m_text = compound.dynamicCast<GroupDef>()->groupTitle(); // use group's title as link

         } else if (compound->definitionType() == Definition::TypeMember &&
                    compound.dynamicCast<MemberDef>()->isObjCMethod()) {

            // Objective C Method
            QSharedPointer<MemberDef> member = compound.dynamicCast<MemberDef>();
            bool localLink = s_memberDef ? member->getClassDef() == s_memberDef->getClassDef() : false;
            m_text = member->objCMethodName(localLink, s_inSeeBlock);
         }

         m_file = compound->getOutputFileBase();
         m_ref  = compound->getReference();

         return;

      } else if (compound && compound->definitionType() == Definition::TypeFile &&
                 compound.dynamicCast<FileDef>()->generateSourceFile() ) {

         // undocumented file has source code we can link to
         m_file = compound->getSourceFileBase();
         m_ref  = compound->getReference();
         return;
      }
   }

   m_text = target;
   warn_doc_error(s_fileName, doctokenizerYYlineno, "Unable to resolve reference to '%s' for \\ref command",
                  csPrintable(target));
}

static void flattenParagraphs(DocNode *root, QList<DocNode *> &children)
{
   QList<DocNode*> newChildren;

   for (auto dn : children) {
      if (dn->kind() == DocNode::Kind_Para) {
         DocPara *para = (DocPara *)dn;
         QList<DocNode *> &paraChildren = para->children();

         for (auto item : paraChildren) {
            newChildren.append(item); // add them to new node
         }
      }
   }

   children.clear();

   for (auto item : newChildren) {
      children.append(item);
      item->setParent(root);
   }
}

void DocRef::parse()
{
   DBG(("DocRef::parse() start\n"));
   s_nodeStack.push(this);

   int tok;

   while ((tok = doctokenizerYYlex())) {

      if (! defaultHandleToken(this, tok, m_children)) {

         switch (tok) {
            case TK_COMMAND_AT:
            case TK_COMMAND_BS: {
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Illegal command %s as part of a \\ref",
                              csPrintable(g_token->name));
               break;
            }

            case TK_SYMBOL:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unsupported symbol %s found",
                              csPrintable(g_token->name));
               break;

            case TK_HTMLTAG:
               break;

            default:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s",
                              csPrintable(tokToString(tok)));
               break;
         }
      }
   }

   if (m_children.isEmpty() && ! m_text.isEmpty()) {
      s_insideHtmlLink = true;

      docParserPushContext();
      internalValidatingParseDoc(this, m_children, m_text);
      docParserPopContext();

      s_insideHtmlLink = false;
      flattenParagraphs(this, m_children);
   }

   handlePendingStyleCommands(this, m_children);

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
}

DocCite::DocCite(DocNode *parent, const QString &target, const QString &)
{
   static const int numBibFiles = Config::getList("cite-bib-files").count();

   m_parent = parent;
   assert(! target.isEmpty());

   m_relPath = s_relPath;
   QString citeValue = Doxy_Globals::citeDict.find(target);

   if (! citeValue.isEmpty() && numBibFiles > 0) {
      // refer to the citation

      m_text   = citeValue;
      m_anchor = CiteConsts::anchorPrefix + target;
      m_file   = convertNameToFile_X(CiteConsts::fileName, false, true);

      return;
   }

   m_text = target;

   if (numBibFiles == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "No bib files for the \\cite command were specified in 'CITE BIB FILES'");

   } else if (citeValue.isEmpty()) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unable to resolve reference to '%s' for \\cite command",
                  csPrintable(target));

   } else {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "No value was specified for the \\cite command to '%s'",
                  csPrintable(target));
   }
}

DocLink::DocLink(DocNode *parent, const QString &target)
{
   m_parent = parent;

   QSharedPointer<Definition> compound;
   QString anchor;

   m_refText = target;
   m_relPath = s_relPath;

   if (! m_refText.isEmpty() && m_refText.at(0) == '#') {
      m_refText = m_refText.right(m_refText.length() - 1);
   }

   if (resolveLink(s_context, stripKnownExtensions(target), s_inSeeBlock, &compound, anchor)) {
      m_anchor = anchor;

      if (compound && compound->isLinkable()) {
         m_file = compound->getOutputFileBase();
         m_ref  = compound->getReference();

      } else if (compound && compound->definitionType() == Definition::TypeFile &&
               compound.dynamicCast<FileDef>()->generateSourceFile()) {

         // undocumented file that has source code we can link to
         m_file = compound->getSourceFileBase();
         m_ref  = compound->getReference();
      }

      return;
   }

   // bogus link target
   warn_doc_error(s_fileName, doctokenizerYYlineno, "Unable to resolve link to '%s' for \\link command", csPrintable(target));
}

QString DocLink::parse(bool isJavaLink, bool isXmlLink)
{
   DBG(("DocLink::parse() start\n"));
   s_nodeStack.push(this);

   QString result;
   int tok;

   while ((tok = doctokenizerYYlex())) {

      if (!defaultHandleToken(this, tok, m_children, false)) {
         switch (tok) {

            case TK_COMMAND_AT:
            case TK_COMMAND_BS:
               switch (Mappers::cmdMapper->map(g_token->name)) {
                  case CMD_ENDLINK:
                     if (isJavaLink) {
                        warn_doc_error(s_fileName, doctokenizerYYlineno, "{@link.. ended with @endlink command");
                     }
                     goto endlink;
                  default:
                     warn_doc_error(s_fileName, doctokenizerYYlineno, "Illegal command %s as part of a \\link",
                                    csPrintable(g_token->name));
                     break;
               }
               break;

            case TK_SYMBOL:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unsupported symbol %s found",
                              csPrintable(g_token->name));
               break;

            case TK_HTMLTAG:
               if (g_token->name != "see" || !isXmlLink) {
                  warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected xml/html command %s found",
                                 csPrintable(g_token->name));
               }
               goto endlink;

            case TK_LNKWORD:
            case TK_WORD:
               if (isJavaLink) { // special case to detect closing }
                  QString w = g_token->name;
                  int p;

                  if (w == "}") {
                     goto endlink;

                  } else if ((p = w.indexOf('}')) != -1) {
                     uint l = w.length();
                     m_children.append(new DocWord(this, w.left(p)));

                     if ((uint)p < l - 1) { // something left after the } (for instance a .)
                        result = w.right(l - p - 1);
                     }

                     goto endlink;
                  }
               }

               m_children.append(new DocWord(this, g_token->name));
               break;

            default:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s", csPrintable(tokToString(tok)));
               break;
         }
      }
   }

   if (tok == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment while inside link command\n");
   }

endlink:

   if (m_children.isEmpty()) {
      // no link text
      m_children.append(new DocWord(this, m_refText));
   }

   handlePendingStyleCommands(this, m_children);

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocLink::parse() end\n"));

   return result;
}

DocDotFile::DocDotFile(DocNode *parent, const QString &name, const QString &context) :
   m_name(name), m_relPath(s_relPath), m_context(context)
{
   m_parent = parent;
}

void DocDotFile::parse()
{
   defaultHandleTitleAndSize(CMD_DOTFILE, this, m_children, m_width, m_height);

   bool ambig;
   QSharedPointer<FileDef> fd = findFileDef(&Doxy_Globals::dotFileNameDict, m_name, ambig);

   if (fd == nullptr && ! m_name.endsWith(".dot")) {
      // try with .dot extension as well
      fd = findFileDef(&Doxy_Globals::dotFileNameDict, m_name + ".dot", ambig);
   }

   if (fd) {
      m_file = fd->getFilePath();

   } else if (ambig) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Included dot file name %s is ambiguous.\n"
                  "Possible candidates:\n%s", csPrintable(m_name),
                  csPrintable(showFileDefMatches(Doxy_Globals::exampleNameDict, m_name)));
   } else {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Included dot file %s was not found "
                  "in any of the paths specified via DOTFILE_DIRS", csPrintable(m_name));
   }
}

DocMscFile::DocMscFile(DocNode *parent, const QString &name, const QString &context) :
   m_name(name), m_relPath(s_relPath), m_context(context)
{
   m_parent = parent;
}

void DocMscFile::parse()
{
   defaultHandleTitleAndSize(CMD_MSCFILE, this, m_children, m_width, m_height);

   bool ambig;
   QSharedPointer<FileDef> fd = findFileDef(&Doxy_Globals::mscFileNameDict, m_name, ambig);

   if (fd == nullptr && ! m_name.endsWith(".msc")) {
      // try with .msc extension as well
      fd = findFileDef(&Doxy_Globals::mscFileNameDict, m_name + ".msc", ambig);
   }

   if (fd) {
      m_file = fd->getFilePath();

   } else if (ambig) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Included msc file name %s is ambiguous.\n"
                  "Possible candidates:\n%s", csPrintable(m_name),
                  csPrintable(showFileDefMatches(Doxy_Globals::exampleNameDict, m_name)));

   } else {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Included msc file %s was not found "
                  "in any of the paths specified via MSCFILE DIRS", csPrintable(m_name));
   }
}

DocDiaFile::DocDiaFile(DocNode *parent, const QString &name, const QString &context) :
   m_name(name), m_relPath(s_relPath), m_context(context)
{
   m_parent = parent;
}

void DocDiaFile::parse()
{
   defaultHandleTitleAndSize(CMD_DIAFILE, this, m_children, m_width, m_height);

   bool ambig;
   QSharedPointer<FileDef> fd = findFileDef(&Doxy_Globals::diaFileNameDict, m_name, ambig);

   if (fd == 0 && ! m_name.endsWith(".dia")) {
      // try with .dia extension as well
      fd = findFileDef(&Doxy_Globals::diaFileNameDict, m_name + ".dia", ambig);
   }

   if (fd) {
      m_file = fd->getFilePath();

   } else if (ambig) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Included dia file name %s is ambiguous.\n"
                  "Possible candidates:\n%s", csPrintable(m_name),
                  csPrintable(showFileDefMatches(Doxy_Globals::exampleNameDict, m_name)));

   } else {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Included dia file %s was not found "
                  "in any of the paths specified via DIAFILE DIRS", csPrintable(m_name));
   }
}

DocImage::DocImage(DocNode *parent, HtmlAttribList attribs, const QString &name, Type t,
   const QString &url, bool inlineImage)
   : m_attribs(std::move(attribs)), m_type(t), m_name(name), m_relPath(s_relPath), m_url(url), m_inlineImage(inlineImage)
{
   m_parent = parent;
}

void DocImage::parse()
{
   defaultHandleTitleAndSize(CMD_IMAGE, this, m_children, m_width, m_height);
}

int DocHtmlHeader::parse()
{
   DBG(("DocHtmlHeader::parse() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;
   int tok;

   while ((tok = doctokenizerYYlex())) {
      if (! defaultHandleToken(this, tok, m_children)) {
         switch (tok) {
            case TK_COMMAND_AT:
            case TK_COMMAND_BS:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Illegal command %s as part of a <h%d> tag",
                              csPrintable(g_token->name), m_level);
               break;

            case TK_HTMLTAG: {
               int tagId = Mappers::htmlTagMapper->map(g_token->name);
               if (tagId == HTML_H1 && g_token->endTag) {        // found </h1> tag
                  if (m_level != 1) {
                     warn_doc_error(s_fileName, doctokenizerYYlineno, "<h%d> ended with </h1>", m_level);
                  }
                  goto endheader;

               } else if (tagId == HTML_H2 && g_token->endTag) { // found </h2> tag
                  if (m_level != 2) {
                     warn_doc_error(s_fileName, doctokenizerYYlineno, "<h%d> ended with </h2>", m_level);
                  }
                  goto endheader;

               } else if (tagId == HTML_H3 && g_token->endTag) { // found </h3> tag
                  if (m_level != 3) {
                     warn_doc_error(s_fileName, doctokenizerYYlineno, "<h%d> ended with </h3>", m_level);
                  }
                  goto endheader;

               } else if (tagId == HTML_H4 && g_token->endTag) { // found </h4> tag
                  if (m_level != 4) {
                     warn_doc_error(s_fileName, doctokenizerYYlineno, "<h%d> ended with </h4>", m_level);
                  }
                  goto endheader;

               } else if (tagId == HTML_H5 && g_token->endTag) { // found </h5> tag
                  if (m_level != 5) {
                     warn_doc_error(s_fileName, doctokenizerYYlineno, "<h%d> ended with </h5>", m_level);
                  }
                  goto endheader;

               } else if (tagId == HTML_H6 && g_token->endTag) { // found </h6> tag
                  if (m_level != 6) {
                     warn_doc_error(s_fileName, doctokenizerYYlineno, "<h%d> ended with </h6>", m_level);
                  }
                  goto endheader;

               } else if (tagId == HTML_A) {
                  if (!g_token->endTag) {
                     handleAHref(this, m_children, g_token->attribs);
                  }
               } else if (tagId == HTML_BR) {
                  DocLineBreak *lb = new DocLineBreak(this);
                  m_children.append(lb);

               } else {
                  warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected html tag <%s%s> found within <h%d> context",
                                 g_token->endTag ? "/" : "", csPrintable(g_token->name), m_level);
               }

            }
            break;

            case TK_SYMBOL:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unsupported symbol %s found",
                              csPrintable(g_token->name));
               break;

            default:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s",
                              csPrintable(tokToString(tok)));
               break;
         }
      }
   }

   if (tok == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment while inside"
                     " <h%d> tag\n", m_level);
   }

endheader:
   handlePendingStyleCommands(this, m_children);

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlHeader::parse() end\n"));

   return retval;
}

int DocHRef::parse()
{
   DBG(("DocHRef::parse() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;
   int tok;

   while ((tok = doctokenizerYYlex())) {
      if (!defaultHandleToken(this, tok, m_children)) {
         switch (tok) {
            case TK_COMMAND_AT:
            case TK_COMMAND_BS:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Illegal command %s as part of a <a>..</a> block",
                              csPrintable(g_token->name));
               break;

            case TK_SYMBOL:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unsupported symbol %s found",
                              csPrintable(g_token->name));
               break;

            case TK_HTMLTAG:

            {
               int tagId = Mappers::htmlTagMapper->map(g_token->name);
               if (tagId == HTML_A && g_token->endTag) { // found </a> tag
                  goto endhref;
               } else {
                  warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected html tag <%s%s> found within <a href=...> context",
                                 g_token->endTag ? "/" : "", csPrintable(g_token->name), doctokenizerYYlineno);
               }
            }
            break;
            default:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s",
                              csPrintable(tokToString(tok)), doctokenizerYYlineno);
               break;
         }
      }
   }
   if (tok == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment while inside"
                     " <a href=...> tag", doctokenizerYYlineno);
   }

endhref:
   handlePendingStyleCommands(this, m_children);

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHRef::parse() end\n"));

   return retval;
}

int DocInternal::parse(int level)
{
   DBG(("DocInternal::parse() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;

   // first parse any number of paragraphs
   bool isFirst = true;
   DocPara *lastPar = 0;

   do {
      DocPara *par = new DocPara(this);

      if (isFirst) {
         par->markFirst();
      }

      retval = par->parse();

      if (! par->isEmpty()) {
         m_children.append(par);

         isFirst = false;
         lastPar = par;

      } else {
         delete par;

      }

      if (retval == TK_LISTITEM) {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Invalid list item found", doctokenizerYYlineno);
      }

   } while (retval != 0 &&
            retval != RetVal_Section &&
            retval != RetVal_Subsection &&
            retval != RetVal_Subsubsection &&
            retval != RetVal_Paragraph &&
            retval != RetVal_EndInternal
           );

   if (lastPar) {
      lastPar->markLast();
   }

   // then parse any number of level-n sections
   while ((level == 1 && retval == RetVal_Section) ||
          (level == 2 && retval == RetVal_Subsection) ||
          (level == 3 && retval == RetVal_Subsubsection) ||
          (level == 4 && retval == RetVal_Paragraph)) {

      DocSection *s = new DocSection(this, qMin(level + Doxy_Globals::subpageNestingLevel, 5), g_token->sectionId);
      m_children.append(s);

      retval = s->parse();
   }

   if (retval == RetVal_Internal) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "\\internal command found inside internal section");
   }

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocInternal::parse() end: retval=%x\n", retval));

   return retval;
}

int DocIndexEntry::parse()
{
   DBG(("DocIndexEntry::parse() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;
   int tok = doctokenizerYYlex();

   if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\addindex command");
      goto endindexentry;
   }

   doctokenizerYYsetStateTitle();
   m_entry = "";

   while ((tok = doctokenizerYYlex())) {
      switch (tok) {
         case TK_WHITESPACE:
            m_entry += " ";
            break;

         case TK_WORD:
         case TK_LNKWORD:
            m_entry += g_token->name;
            break;

         case TK_SYMBOL: {
            DocSymbol::SymType s = DocSymbol::decodeSymbol(g_token->name);
            switch (s) {
               case DocSymbol::Sym_BSlash:
                  m_entry += '\\';
                  break;
               case DocSymbol::Sym_At:
                  m_entry += '@';
                  break;
               case DocSymbol::Sym_Less:
                  m_entry += '<';
                  break;
               case DocSymbol::Sym_Greater:
                  m_entry += '>';
                  break;
               case DocSymbol::Sym_Amp:
                  m_entry += '&';
                  break;
               case DocSymbol::Sym_Dollar:
                  m_entry += '$';
                  break;
               case DocSymbol::Sym_Hash:
                  m_entry += '#';
                  break;
               case DocSymbol::Sym_Percent:
                  m_entry += '%';
                  break;
               case DocSymbol::Sym_apos:
                  m_entry += '\'';
                  break;
               case DocSymbol::Sym_Quot:
                  m_entry += '"';
                  break;
               case DocSymbol::Sym_lsquo:
                  m_entry += '`';
                  break;
               case DocSymbol::Sym_rsquo:
                  m_entry += '\'';
                  break;
               case DocSymbol::Sym_ldquo:
                  m_entry += "``";
                  break;
               case DocSymbol::Sym_rdquo:
                  m_entry += "''";
                  break;
               case DocSymbol::Sym_ndash:
                  m_entry += "--";
                  break;
               case DocSymbol::Sym_mdash:
                  m_entry += "---";
                  break;
               default:
                  warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected symbol found as argument of \\addindex");
                  break;
            }
         }
         break;

         case TK_COMMAND_AT:
         case TK_COMMAND_BS:
            switch (Mappers::cmdMapper->map(g_token->name)) {
               case CMD_BSLASH:
                  m_entry += '\\';
                  break;
               case CMD_AT:
                  m_entry += '@';
                  break;
               case CMD_LESS:
                  m_entry += '<';
                  break;
               case CMD_GREATER:
                  m_entry += '>';
                  break;
               case CMD_AMP:
                  m_entry += '&';
                  break;
               case CMD_DOLLAR:
                  m_entry += '$';
                  break;
               case CMD_HASH:
                  m_entry += '#';
                  break;
               case CMD_DCOLON:
                  m_entry += "::";
                  break;
               case CMD_PERCENT:
                  m_entry += '%';
                  break;
               case CMD_NDASH:
                  m_entry += "--";
                  break;
               case CMD_MDASH:
                  m_entry += "---";
                  break;

               case CMD_QUOTE:
                  m_entry += '"';
                  break;

               case CMD_PUNT:
                  m_entry += '.';
                  break;

               case CMD_PLUS:
                  m_entry += '+';
                  break;

               case CMD_MINUS:
                  m_entry += '-';
                  break;

               case CMD_EQUAL:
                  m_entry+='=';
                  break;

               default:
                  warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected command %s found as argument of \\addindex",
                        csPrintable(g_token->name));
                  break;
            }
            break;

         default:
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s", csPrintable(tokToString(tok)));
            break;
      }
   }
   doctokenizerYYsetStatePara();
   m_entry = m_entry.trimmed();

endindexentry:

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocIndexEntry::parse() end retval = %x\n", retval));

   return retval;
}

DocHtmlCaption::DocHtmlCaption(DocNode *parent, HtmlAttribList attribs)
{
   m_hasCaptionId = false;

   for (const auto &item : attribs)  {

      if (item.name == "id") {
         // interpret id attribute as an anchor

         QSharedPointer<SectionInfo> sec = Doxy_Globals::sectionDict.find(item.value);

         if (sec) {
            m_file   = sec->fileName;
            m_anchor = sec->label;
            m_hasCaptionId = true;

            if (s_sectionDict && s_sectionDict->find(item.value) == 0) {
               s_sectionDict->insert(item.value, sec);
            }

         } else {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Invalid caption id '%s'", csPrintable(item.value));

         }

      } else  {
         // copy attribute
         m_attribs.append(item);
      }
   }

   m_parent = parent;
}

int DocHtmlCaption::parse()
{
   DBG(("DocHtmlCaption::parse() start\n"));
   s_nodeStack.push(this);

   int retval = 0;
   int tok;

   while ((tok = doctokenizerYYlex())) {
      if (!defaultHandleToken(this, tok, m_children)) {
         switch (tok) {
            case TK_COMMAND_AT:
            case TK_COMMAND_BS:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Illegal command %s as part of a <caption> tag",
                              csPrintable(g_token->name));
               break;

            case TK_SYMBOL:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unsupported symbol %s found",
                              csPrintable(g_token->name));
               break;

            case TK_HTMLTAG: {
               int tagId = Mappers::htmlTagMapper->map(g_token->name);
               if (tagId == HTML_CAPTION && g_token->endTag) { // found </caption> tag
                  retval = RetVal_OK;
                  goto endcaption;
               } else {
                  warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected html tag <%s%s> found within <caption> context",
                                 g_token->endTag ? "/" : "", csPrintable(g_token->name));
               }
            }
            break;

            default:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s", csPrintable(tokToString(tok)));
               break;
         }
      }
   }
   if (tok == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment while inside"
                     " <caption> tag", doctokenizerYYlineno);
   }

endcaption:
   handlePendingStyleCommands(this, m_children);

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlCaption::parse() end\n"));

   return retval;
}

int DocHtmlCell::parse()
{
   DBG(("DocHtmlCell::parse() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;

   // parse one or more paragraphs
   bool isFirst = true;
   DocPara *par = 0;

   do {
      DocPara *tempPar = new DocPara(this);

      if (isFirst) {
         tempPar->markFirst();
      }

      retval = tempPar->parse();

      if (tempPar->isEmpty() ) {
         delete tempPar;

      } else {
         m_children.append(tempPar);

         isFirst = false;
         par = tempPar;
      }

      if (retval == TK_HTMLTAG) {
         int tagId = Mappers::htmlTagMapper->map(g_token->name);

         if (tagId == HTML_TD && g_token->endTag) {
            // found </dt> tag
            retval = TK_NEWPARA; // ignore the tag

         } else if (tagId == HTML_TH && g_token->endTag) {
            // found </th> tag
            retval = TK_NEWPARA; // ignore the tag
         }
      }

   } while (retval == TK_NEWPARA);

   if (par) {
      par->markLast();
   }

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlCell::parse() end\n"));

   return retval;
}

int DocHtmlCell::parseXml()
{
   DBG(("DocHtmlCell::parseXml() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;

   // parse one or more paragraphs
   bool isFirst = true;
   DocPara *par = 0;

   do {
      DocPara *tempPar = new DocPara(this);

      if (isFirst) {
         tempPar->markFirst();
      }

      retval = tempPar->parse();

      if (tempPar->isEmpty() ) {
         delete tempPar;

      } else {
         m_children.append(tempPar);

        isFirst = false;
        par = tempPar;
      }

      if (retval == TK_HTMLTAG) {
         int tagId = Mappers::htmlTagMapper->map(g_token->name);
         if (tagId == XML_ITEM && g_token->endTag) { // found </item> tag
            retval = TK_NEWPARA; // ignore the tag

         } else if (tagId == XML_DESCRIPTION && g_token->endTag) { // found </description> tag
            retval = TK_NEWPARA; // ignore the tag
         }
      }

   } while (retval == TK_NEWPARA);

   if (par) {
      par->markLast();
   }

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlCell::parseXml() end\n"));

   return retval;
}

int DocHtmlCell::rowSpan() const
{
   int retval = 0;
   HtmlAttribList attrs = attribs();

   for (uint i = 0; i < attrs.count(); ++i) {

      if (attrs.at(i).name.toLower() == "rowspan") {
         retval = attrs.at(i).value.toInteger<int>();
         break;
      }

   }

   return retval;
}

int DocHtmlCell::colSpan() const
{
   int retval = 1;
   HtmlAttribList attrs = attribs();

   for (uint i = 0; i < attrs.count(); ++i) {
      if (attrs.at(i).name.toLower() == "colspan") {
         retval = qMax(1, attrs.at(i).value.toInteger<int>());
         break;
      }
   }

   return retval;
}

DocHtmlCell::Alignment DocHtmlCell::alignment() const
{
   HtmlAttribList attrs = attribs();

   for (uint i = 0; i < attrs.count(); ++i) {

      if (attrs.at(i).name.toLower() == "align") {
         if (attrs.at(i).value.toLower() == "center") {
            return Center;

         } else if (attrs.at(i).value.toLower() == "right") {
            return Right;

         } else {
            return Left;
         }
      }
   }

   return Left;
}

int DocHtmlRow::parse()
{
   DBG(("DocHtmlRow::parse() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;

   bool isHeading = false;
   bool isFirst = true;
   DocHtmlCell *cell = 0;

   // get next token
   int tok = doctokenizerYYlex();

   // skip whitespace
   while (tok == TK_WHITESPACE || tok == TK_NEWPARA) {
      tok = doctokenizerYYlex();
   }

   // should find an html tag now
   if (tok == TK_HTMLTAG) {
      int tagId = Mappers::htmlTagMapper->map(g_token->name);

      if (tagId == HTML_TD && ! g_token->endTag) {
         // found <td> tag

      } else if (tagId == HTML_TH && !g_token->endTag) { // found <th> tag
         isHeading = true;

      } else {
         // found some other tag
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected <td> or <th> tag but "
                        "found <%s> instead", csPrintable(g_token->name));

         doctokenizerYYpushBackHtmlTag(g_token->name);
         goto endrow;
      }

   } else if (tok == 0) { // premature end of comment
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment while looking for an html description title");
      goto endrow;

   } else { // token other than html token
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected <td> or <th> tag, found %s token instead ", csPrintable(tokToString(tok)));
      goto endrow;

   }

   // parse one or more cells
   do {
      cell = new DocHtmlCell(this, g_token->attribs, isHeading);
      cell->markFirst(isFirst);

      isFirst = false;

      m_children.append(cell);
      retval = cell->parse();

      isHeading = (retval == RetVal_TableHCell);

   } while (retval == RetVal_TableCell || retval == RetVal_TableHCell);

   if (cell) {
      cell->markLast(true);
   }

endrow:
   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlRow::parse() end\n"));

   return retval;
}

int DocHtmlRow::parseXml(bool isHeading)
{
   DBG(("DocHtmlRow::parseXml() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;

   bool isFirst = true;
   DocHtmlCell *cell = 0;

   // get next token
   int tok = doctokenizerYYlex();

   // skip whitespace
   while (tok == TK_WHITESPACE || tok == TK_NEWPARA) {
      tok = doctokenizerYYlex();
   }

   // should find a html tag now
   if (tok == TK_HTMLTAG) {
      int tagId = Mappers::htmlTagMapper->map(g_token->name);

      if (tagId == XML_TERM && !g_token->endTag) {
         // found <term> tag

      } else if (tagId == XML_DESCRIPTION && !g_token->endTag) {
         // found <description> tag

      } else {
         // found some other tag
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected <term> or <description> tag, "
                        "found <%s> instead", csPrintable(g_token->name));

         doctokenizerYYpushBackHtmlTag(g_token->name);
         goto endrow;
      }

   } else if (tok == 0) { // premature end of comment
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment while looking"
                     " for an html description title");
      goto endrow;

   } else { // token other than html token
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected <td> or <th> tag, found %s token instead",
                     csPrintable(tokToString(tok)));
      goto endrow;
   }

   do {
      cell = new DocHtmlCell(this, g_token->attribs, isHeading);

      cell->markFirst(isFirst);
      isFirst = false;

      m_children.append(cell);
      retval = cell->parseXml();

   } while (retval == RetVal_TableCell || retval == RetVal_TableHCell);

   if (cell) {
      cell->markLast(true);
   }

endrow:
   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlRow::parseXml() end\n"));

   return retval;
}

int DocHtmlTable::parse()
{
   DBG(("DocHtmlTable::parse() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;

getrow:
   // get next token
   int tok = doctokenizerYYlex();

   // skip whitespace
   while (tok == TK_WHITESPACE || tok == TK_NEWPARA) {
      tok = doctokenizerYYlex();
   }

   // should find an html tag now
   if (tok == TK_HTMLTAG) {
      int tagId = Mappers::htmlTagMapper->map(g_token->name);

      if (tagId == HTML_TR && !g_token->endTag) {
         // found <tr> tag,  no caption, just rows
         retval = RetVal_TableRow;

      } else if (tagId == HTML_CAPTION && ! g_token->endTag) {
         // found <caption> tag

         if (m_caption) {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "table already has a caption, found another one");

         } else {
            m_caption = new DocHtmlCaption(this, g_token->attribs);
            retval    = m_caption->parse();

            if (retval == RetVal_OK) {
               // caption was parsed ok
               goto getrow;
            }
         }

      } else {
         // found wrong token
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected <tr> or <caption> tag but "
                        "found <%s%s> instead", g_token->endTag ? "/" : "", csPrintable(g_token->name));
      }

   } else if (tok == 0) {
      // premature end of comment
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment while looking for a <tr> or <caption> tag");

   } else {
      // token other than html token
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected <tr> tag, found %s token instead ", csPrintable(tokToString(tok)));
   }

   // parse one or more rows
   while (retval == RetVal_TableRow) {
      DocHtmlRow *tr = new DocHtmlRow(this, g_token->attribs);

      m_children.append(tr);
      retval = tr->parse();
   }

   computeTableGrid();

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlTable::parse() end\n"));

   return retval == RetVal_EndTable ? RetVal_OK : retval;
}

int DocHtmlTable::parseXml()
{
   DBG(("DocHtmlTable::parseXml() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;

   // get next token
   int tok = doctokenizerYYlex();

   // skip whitespace

   while (tok == TK_WHITESPACE || tok == TK_NEWPARA) {
      tok = doctokenizerYYlex();
   }

   // should find a html tag now
   int tagId = 0;
   bool isHeader = false;

   if (tok == TK_HTMLTAG) {
      tagId = Mappers::htmlTagMapper->map(g_token->name);
      if (tagId == XML_ITEM && !g_token->endTag) { // found <item> tag
         retval = RetVal_TableRow;
      }
      if (tagId == XML_LISTHEADER && !g_token->endTag) { // found <listheader> tag
         retval = RetVal_TableRow;
         isHeader = true;
      }
   }

   // parse one or more rows
   while (retval == RetVal_TableRow) {
      DocHtmlRow *tr = new DocHtmlRow(this, g_token->attribs);
      m_children.append(tr);

      retval = tr->parseXml(isHeader);
      isHeader = false;
   }

   computeTableGrid();

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlTable::parseXml() end\n"));

   tagId = Mappers::htmlTagMapper->map(g_token->name);

   if (tagId == XML_LIST && g_token->endTag)  {
      retval = RetVal_OK;
   }

   return retval;
}

/** Helper class to compute the grid for an HTML style table */
struct ActiveRowSpan {
   ActiveRowSpan(int rows, int col) : rowsLeft(rows), column(col) {}
   int rowsLeft;
   int column;
};

/** List of ActiveRowSpan classes. */
typedef QList<ActiveRowSpan> RowSpanList;

/** determines the location of all cells in a grid, resolving row and
    column spans. For each the total number of visible cells is computed,
    and the total number of visible columns over all rows is stored.
 */
void DocHtmlTable::computeTableGrid()
{
   RowSpanList rowSpans;

   int maxCols = 0;
   int rowIdx = 1;

   for (auto rowNode : children()) {
      int colIdx = 1;
      int cells = 0;

      if (rowNode->kind() == DocNode::Kind_HtmlRow) {
         uint i;
         DocHtmlRow *row = (DocHtmlRow *)rowNode;

         for (auto cellNode : row->children()) {

            if (cellNode->kind() == DocNode::Kind_HtmlCell) {
               DocHtmlCell *cell = (DocHtmlCell *)cellNode;
               int rs = cell->rowSpan();
               int cs = cell->colSpan();

               for (i = 0; i < rowSpans.count(); i++) {
                  if (rowSpans.at(i).rowsLeft > 0 && rowSpans.at(i).column == colIdx) {
                     colIdx = rowSpans.at(i).column + 1;
                     cells++;
                  }
               }

               if (rs > 0) {
                  rowSpans.append(ActiveRowSpan(rs, colIdx));
               }

               cell->setRowIndex(rowIdx);
               cell->setColumnIndex(colIdx);
               colIdx += cs;
               cells++;
            }
         }

         for (i = 0; i < rowSpans.count(); i++) {
            if (rowSpans[i].rowsLeft > 0) {
               rowSpans[i].rowsLeft--;
            }
         }

         row->setVisibleCells(cells);
         row->setRowIndex(rowIdx);
         rowIdx++;
      }

      if (colIdx - 1 > maxCols) {
         maxCols = colIdx - 1;
      }
   }

   m_numCols = maxCols;
}

void DocHtmlTable::accept(DocVisitor *v)
{
   v->visitPre(this);

   // put the caption first
   if (m_caption) {
      m_caption->accept(v);
   }

   for (auto n : m_children) {
      n->accept(v);
   }

   v->visitPost(this);
}

int DocHtmlDescTitle::parse()
{
   DBG(("DocHtmlDescTitle::parse() start\n"));
   s_nodeStack.push(this);

   int retval = 0;
   int tok;

   while ((tok = doctokenizerYYlex())) {

      if (! defaultHandleToken(this, tok, m_children)) {

         switch (tok) {

            case TK_COMMAND_AT:
            case TK_COMMAND_BS: {
               QString cmdName = g_token->name;
               bool isJavaLink = false;

               switch (Mappers::cmdMapper->map(cmdName)) {
                  case CMD_REF: {
                     int tok = doctokenizerYYlex();

                     if (tok != TK_WHITESPACE) {
                        warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command",
                              csPrintable(g_token->name));

                     } else {
                        doctokenizerYYsetStateRef();
                        tok = doctokenizerYYlex(); // get the reference id

                        if (tok != TK_WORD) {
                           warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of %s",
                                          csPrintable(tokToString(tok)), csPrintable(cmdName));

                        } else {
                           // add the name
                           DocRef *ref = new DocRef(this, g_token->name, s_context);
                           m_children.append(ref);
                           ref->parse();
                        }
                        doctokenizerYYsetStatePara();
                     }
                  }
                  break;

                  case CMD_JAVALINK:
                     isJavaLink = true;
                     [[fallthrough]];

                  case CMD_LINK: {
                     int tok = doctokenizerYYlex();
                     if (tok != TK_WHITESPACE) {
                        warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \%s command",
                                       csPrintable(cmdName));

                     } else {
                        doctokenizerYYsetStateLink();
                        tok = doctokenizerYYlex();

                        if (tok != TK_WORD) {
                           warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of %s",
                                          csPrintable(tokToString(tok)), csPrintable(cmdName));

                        } else {
                           doctokenizerYYsetStatePara();
                           DocLink *lnk = new DocLink(this, g_token->name);
                           m_children.append(lnk);

                           QString leftOver = lnk->parse(isJavaLink);
                           if (! leftOver.isEmpty()) {
                              m_children.append(new DocWord(this, leftOver));
                           }
                        }
                     }
                  }

                  break;
                  default:
                     warn_doc_error(s_fileName, doctokenizerYYlineno, "Illegal command %s as part of a <dt> tag",
                                    csPrintable(g_token->name));
               }
            }
            break;

            case TK_SYMBOL:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unsupported symbol %s found", csPrintable(g_token->name));
               break;

            case TK_HTMLTAG: {
               int tagId = Mappers::htmlTagMapper->map(g_token->name);

               if (tagId == HTML_DD && ! g_token->endTag) {
                  // found <dd> tag
                  retval = RetVal_DescData;
                  goto endtitle;

               } else if (tagId == HTML_DT && g_token->endTag) {
                  // ignore </dt> tag

               } else if (tagId == HTML_DT) {
                  // missing <dt> tag
                  retval = RetVal_DescTitle;
                  goto endtitle;

               } else if (tagId == HTML_DL && g_token->endTag) {
                  retval = RetVal_EndDesc;
                  goto endtitle;

               } else if (tagId == HTML_A) {
                  if (! g_token->endTag) {
                     handleAHref(this, m_children, g_token->attribs);
                  }

               } else {
                  warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected html tag <%s%s> found within <dt> context",
                                 g_token->endTag ? "/" : "", csPrintable(g_token->name));
               }
            }
            break;

            default:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s", csPrintable(tokToString(tok)));
               break;
         }
      }
   }

   if (tok == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment while inside <dt> tag");
   }

endtitle:
   handlePendingStyleCommands(this, m_children);

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlDescTitle::parse() end\n"));

   return retval;
}

int DocHtmlDescData::parse()
{
   DBG(("DocHtmlDescData::parse() start\n"));
   s_nodeStack.push(this);

   m_attribs = g_token->attribs;
   int retval = 0;

   bool isFirst = true;
   DocPara *par = 0;

   do {
      DocPara *tempPar = new DocPara(this);

      if (isFirst) {
         tempPar->markFirst();
      }

      retval = tempPar->parse();

      if (tempPar->isEmpty() ) {
         delete tempPar;

      } else {
         m_children.append(tempPar);

        isFirst = false;
        par = tempPar;
      }

   } while (retval == TK_NEWPARA);

   if (par) {
      par->markLast();
   }

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlDescData::parse() end\n"));

   return retval;
}

int DocHtmlDescList::parse()
{
   DBG(("DocHtmlDescList::parse() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;
   int tok = doctokenizerYYlex();

   // skip whitespace
   while (tok == TK_WHITESPACE || tok == TK_NEWPARA) {
      tok = doctokenizerYYlex();
   }

   // should find a html tag now
   if (tok == TK_HTMLTAG) {
      int tagId = Mappers::htmlTagMapper->map(g_token->name);

      if (tagId == HTML_DT && !g_token->endTag) {
         // found <dt> tag, continue

      } else {
         // found some other tag
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected <dt> tag, found <%s> instead", csPrintable(g_token->name));
         doctokenizerYYpushBackHtmlTag(g_token->name);
         goto enddesclist;
      }

   } else if (tok == 0) { // premature end of comment
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment while looking"
                     " for an html description title");
      goto enddesclist;

   } else { // token other than html token
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected <dt> tag, found %s token instead", csPrintable(tokToString(tok)));
      goto enddesclist;

   }

   {
      // first entry must be a <dt>
      DocHtmlDescTitle *dt = new DocHtmlDescTitle(this, g_token->attribs);
      m_children.append(dt);
      retval = dt->parse();

      while (retval == RetVal_DescTitle || retval == RetVal_DescData) {

         if (retval == RetVal_DescTitle) {

            DocHtmlDescTitle *dt = new DocHtmlDescTitle(this, g_token->attribs);
            m_children.append(dt);
            retval = dt->parse();

         } else if (retval == RetVal_DescData) {

            DocHtmlDescData *dd = new DocHtmlDescData(this);
            m_children.append(dd);
            retval = dd->parse();
         }
      }

      if (retval == 0) {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment while inside <dl> block");
      }
   }

enddesclist:

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlDescList::parse() end\n"));

   return retval == RetVal_EndDesc ? RetVal_OK : retval;
}

int DocHtmlListItem::parse()
{
   DBG(("DocHtmlListItem::parse() start\n"));
   s_nodeStack.push(this);

   int retval = 0;

   // parse one or more paragraphs
   bool isFirst = true;
   DocPara *par = 0;

   do {
      DocPara *tempPar = new DocPara(this);

      if (isFirst) {
         tempPar->markFirst();
      }

      retval = tempPar->parse();

      if (tempPar->isEmpty() ) {
         delete tempPar;

      } else {
         m_children.append(tempPar);

         isFirst = false;
         par = tempPar;
      }

   } while (retval == TK_NEWPARA);

   if (par) {
      par->markLast();
   }

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlListItem::parse() end retval=%x\n", retval));

   return retval;
}

int DocHtmlListItem::parseXml()
{
   DBG(("DocHtmlListItem::parseXml() start\n"));
   s_nodeStack.push(this);

   int retval = 0;

   // parse one or more paragraphs
   bool isFirst = true;
   DocPara *par = 0;

   do {
       DocPara *tempPar = new DocPara(this);

      if (isFirst) {
         tempPar->markFirst();
      }

      retval = tempPar->parse();

      if (tempPar->isEmpty() ) {
         delete tempPar;

      } else {
         m_children.append(tempPar);

        isFirst = false;
        par = tempPar;
      }


      if (retval == 0) {
         break;
      }

      if (retval == RetVal_ListItem) {
         break;
      }

   } while (retval != RetVal_CloseXml);

   if (par) {
      par->markLast();
   }

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlListItem::parseXml() end retval=%x\n", retval));

   return retval;
}

int DocHtmlList::parse()
{
   DBG(("DocHtmlList::parse() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;
   int num = 1;

   // get next token
   int tok = doctokenizerYYlex();

   // skip whitespace and paragraph breaks
   while (tok == TK_WHITESPACE || tok == TK_NEWPARA) {
      tok = doctokenizerYYlex();
   }

   // should find a html tag now
   if (tok == TK_HTMLTAG) {
      int tagId = Mappers::htmlTagMapper->map(g_token->name);

      if (tagId == HTML_LI && ! g_token->endTag) {
         // found <li> tag
         // ok, we can go on.

      } else if (((m_type == Unordered && tagId == HTML_UL) || (m_type == Ordered && tagId == HTML_OL)) && g_token->endTag) {
         // found empty list
         // add dummy item to obtain valid HTML

         m_children.append(new DocHtmlListItem(this, HtmlAttribList(), 1));
         warn_doc_error(s_fileName, doctokenizerYYlineno, "empty list");
         retval = RetVal_EndList;
         goto endlist;

      } else {
         // found some other tag
         // add dummy item to obtain valid HTML
         m_children.append(new DocHtmlListItem(this, HtmlAttribList(), 1));

         warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected <li> tag,  "
                        "found <%s%s> instead", g_token->endTag ? "/" : "", csPrintable(g_token->name));

         doctokenizerYYpushBackHtmlTag(g_token->name);
         goto endlist;
      }

   } else if (tok == 0) {
      // premature end of comment
      // add dummy item to obtain valid HTML

      m_children.append(new DocHtmlListItem(this, HtmlAttribList(), 1));
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment while looking"
                     " for an html list item");
      goto endlist;

   } else {
      // token other than html token
      // add dummy item to obtain valid HTML

      m_children.append(new DocHtmlListItem(this, HtmlAttribList(), 1));
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected <li> tag, found %s token instead",
                     csPrintable(tokToString(tok)));
      goto endlist;
   }

   do {
      DocHtmlListItem *li = new DocHtmlListItem(this, g_token->attribs, num++);
      m_children.append(li);
      retval = li->parse();

   } while (retval == RetVal_ListItem);

   if (retval == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment while inside <%cl> block",
                     m_type == Unordered ? 'u' : 'o');
   }

endlist:
   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlList::parse() end retval=%x\n", retval));

   return retval == RetVal_EndList ? RetVal_OK : retval;
}

int DocHtmlList::parseXml()
{
   DBG(("DocHtmlList::parseXml() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;
   int num = 1;

   // get next token
   int tok = doctokenizerYYlex();

   // skip whitespace and paragraph breaks
   while (tok == TK_WHITESPACE || tok == TK_NEWPARA) {
      tok = doctokenizerYYlex();
   }

   // should find a html tag now
   if (tok == TK_HTMLTAG) {
      int tagId = Mappers::htmlTagMapper->map(g_token->name);

      if (tagId == XML_ITEM && !g_token->endTag) { // found <item> tag
         // ok, we can go on.
      } else { // found some other tag
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected <item> tag, "
                        "found <%s> instead", csPrintable(g_token->name));

         doctokenizerYYpushBackHtmlTag(g_token->name);
         goto endlist;
      }

   } else if (tok == 0) { // premature end of comment
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment while looking"
                     " for an html list item");
      goto endlist;

   } else { // token other than html token
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected <item> tag, found %s token instead",
                     csPrintable(tokToString(tok)));
      goto endlist;
   }

   do {
      DocHtmlListItem *li = new DocHtmlListItem(this, g_token->attribs, num++);
      m_children.append(li);

      retval = li->parseXml();

      if (retval == 0) {
         break;
      }

   } while (retval == RetVal_ListItem);

   if (retval == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment while inside <list type=\"%s\"> block",
                     m_type == Unordered ? "bullet" : "number");
   }

endlist:
   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlList::parseXml() end retval=%x\n", retval));

   return retval == RetVal_EndList ||
         (retval == RetVal_CloseXml || g_token->name == "list") ? RetVal_OK : retval;
}

int DocHtmlBlockQuote::parse()
{
   DBG(("DocHtmlBlockQuote::parse() start\n"));
   s_nodeStack.push(this);

   int retval = 0;

   // parse one or more paragraphs
   bool isFirst = true;
   DocPara *par = 0;

   do {
      DocPara *tempPar = new DocPara(this);

      if (isFirst) {
         tempPar->markFirst();
      }

      retval = tempPar->parse();

      if (tempPar->isEmpty() ) {
         delete tempPar;

      } else {
         m_children.append(tempPar);

        isFirst = false;
        par = tempPar;
      }

   } while (retval == TK_NEWPARA);

   if (par) {
      par->markLast();
   }

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocHtmlBlockQuote::parse() end retval=%x\n", retval));

   return (retval == RetVal_EndBlockQuote) ? RetVal_OK : retval;
}

int DocParBlock::parse()
{
   DBG(("DocParBlock::parse() start\n"));
   s_nodeStack.push(this);

   int retval = 0;

   // parse one or more paragraphs
   bool isFirst = true;
   DocPara *par = 0;

   do {
      par = new DocPara(this);

      if (isFirst) {
         par->markFirst();
         isFirst = false;
      }

      retval = par->parse();

      if (! par->isEmpty() ) {
         m_children.append(par);

      } else {
         delete par;

      }

   } while (retval == TK_NEWPARA);

   if (par) {
      par->markLast();
   }

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocParBlock::parse() end retval=%x\n", retval));

   return (retval == RetVal_EndBlockQuote) ? RetVal_OK : retval;
}

int DocSimpleListItem::parse()
{
   s_nodeStack.push(this);

   int rv = m_paragraph->parse();
   m_paragraph->markFirst();
   m_paragraph->markLast();

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);

   return rv;
}

int DocSimpleList::parse()
{
   s_nodeStack.push(this);
   int rv;

   do {
      DocSimpleListItem *li = new DocSimpleListItem(this);
      m_children.append(li);
      rv = li->parse();

   } while (rv == RetVal_ListItem);

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);

   return (rv != TK_NEWPARA) ? rv : RetVal_OK;
}

DocAutoListItem::DocAutoListItem(DocNode *parent, int indent, int num)
   : m_indent(indent), m_itemNum(num)
{
   m_parent = parent;
}

int DocAutoListItem::parse()
{
   s_nodeStack.push(this);

   int retval = RetVal_OK;

   // first parse any number of paragraphs
   bool isFirst = true;
   DocPara *lastPar = 0;

   do {
      DocPara *par = new DocPara(this);

      if (isFirst) {
         par->markFirst();
         isFirst = false;
      }

      retval = par->parse();

      if (! par->isEmpty()) {
         m_children.append(par);

         if (lastPar) {
            lastPar->markLast(false);
         }

         lastPar = par;

      } else {
         delete par;
      }

      // next paragraph should be more indented than the - marker to belong
      // to this item
   } while (retval == TK_NEWPARA && g_token->indent > m_indent);
   if (lastPar) {
      lastPar->markLast();
   }

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);

   return retval;
}

DocAutoList::DocAutoList(DocNode *parent, int indent, bool isEnumList, int depth) :
   m_indent(indent), m_isEnumList(isEnumList), m_depth(depth)
{
   m_parent = parent;
}

int DocAutoList::parse()
{
   s_nodeStack.push(this);

   int retval = RetVal_OK;
   int num = 1;

   doctokenizerYYstartAutoList();

   // first item or sub list => create new list
   do {
      if (g_token->id != -1) {                           // explicitly numbered list
         num = g_token->id;                              // override num with real number given
      }

      DocAutoListItem *li = new DocAutoListItem(this, m_indent, num++);
      m_children.append(li);
      retval = li->parse();

   } while (retval == TK_LISTITEM &&                     // new list item
            m_indent == g_token->indent &&               // at same indent level
            m_isEnumList == g_token->isEnumList &&       // of the same kind
            (g_token->id == -1 || g_token->id >= num)    // increasing number (or no number)
           );

   doctokenizerYYendAutoList();

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);

   return retval;
}

void DocTitle::parse()
{
   DBG(("DocTitle::parse() start\n"));
   s_nodeStack.push(this);

   int tok;
   doctokenizerYYsetStateTitle();

   while ((tok = doctokenizerYYlex())) {

      if (! defaultHandleToken(this, tok, m_children)) {
         switch (tok) {
            case TK_COMMAND_AT:
            case TK_COMMAND_BS:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Illegal command %s as part of a title section",
                              csPrintable(g_token->name));
               break;

            case TK_SYMBOL:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unsupported symbol %s found",
                              csPrintable(g_token->name));
               break;

            default:
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s",
                              csPrintable(tokToString(tok)));
               break;
         }
      }
   }

   doctokenizerYYsetStatePara();
   handlePendingStyleCommands(this, m_children);

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);

   DBG(("DocTitle::parse() end\n"));
}

void DocTitle::parseFromString(const QString &text)
{
   m_children.append(new DocWord(this, text));
}

DocSimpleSect::DocSimpleSect(DocNode *parent, Type t)
   : m_type(t)
{
   m_parent = parent;
}

DocSimpleSect::~DocSimpleSect()
{
}

void DocSimpleSect::accept(DocVisitor *v)
{
   // generates the <dl> See Also: href, title>Text</a> </dl>

   v->visitPre(this);

   if (m_docTitle) {
      m_docTitle->accept(v);
   }

   for (auto n : m_children) {
      n->accept(v);
   }

   v->visitPost(this);
}

int DocSimpleSect::parse(bool userTitle, bool needsSeparator)
{
   DBG(("DocSimpleSect::parse() start\n"));
   s_nodeStack.push(this);

   // handle case for user defined title
   if (userTitle) {
      m_docTitle = QMakeShared<DocTitle>(this);
      m_docTitle->parse();
   }

   // add new paragraph as a child
   DocPara *par = new DocPara(this);

   if (m_children.isEmpty()) {
      par->markFirst();

   } else {
      assert(m_children.last()->kind() == DocNode::Kind_Para);
      ((DocPara *)m_children.last())->markLast(false);
   }

   par->markLast();

   if (needsSeparator) {
      m_children.append(new DocSimpleSectSep(this));
   }

   int retval = par->parse();

   if (! par->isEmpty() ) {
      m_children.append(par);

   } else {
      delete par;

   }

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocSimpleSect::parse() end retval=%d\n", retval));

   return retval;
}

int DocSimpleSect::parseRcs()
{
   DBG(("DocSimpleSect::parseRcs() start\n"));
   s_nodeStack.push(this);

   m_docTitle = QMakeShared<DocTitle>(this);
   m_docTitle->parseFromString(g_token->name);

   QString text = g_token->text;
   docParserPushContext(); // this will create a new g_token
   internalValidatingParseDoc(this, m_children, text);
   docParserPopContext(); // this will restore the old g_token

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocSimpleSect::parseRcs()\n"));

   return RetVal_OK;
}

int DocSimpleSect::parseXml()
{
   DBG(("DocSimpleSect::parse() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;

   for (;;) {
      // add new paragraph as child
      DocPara *par = new DocPara(this);

      if (m_children.isEmpty()) {
         par->markFirst();

      } else {
         assert(m_children.last()->kind() == DocNode::Kind_Para);
         ((DocPara *)m_children.last())->markLast(false);
      }

      par->markLast();

      retval = par->parse();

      if (! par->isEmpty() ) {
         m_children.append(par);

      } else {
         delete par;
      }

      if (retval == 0) {
         break;
      }

      if (retval == RetVal_CloseXml) {
         retval = RetVal_OK;
         break;
      }
   }

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocSimpleSect::parseXml() end retval=%d\n", retval));

   return retval;
}

void DocSimpleSect::appendLinkWord(const QString &word)
{
   DocPara *p;

   if (m_children.isEmpty() || m_children.last()->kind() != DocNode::Kind_Para) {
      p = new DocPara(this);
      m_children.append(p);

   } else {
      p = (DocPara *)m_children.last();

      // Comma separate <see also> links
      p->injectToken(TK_WORD, ",");
      p->injectToken(TK_WHITESPACE, " ");
   }

   s_inSeeBlock = true;
   p->injectToken(TK_LNKWORD, word);
   s_inSeeBlock = false;
}

QString DocSimpleSect::typeString() const
{
   QString retval = "unknown";

   switch (m_type) {
      case Unknown:
         break;

      case See:
         retval = "see";
         break;

      case Return:
         retval = "return";
         break;

      case Author:
      case Authors:
         retval = "author";
         break;

      case Version:
         retval = "version";
         break;

      case Since:
         retval = "since";
         break;

      case Date:
         retval = "date";
         break;

      case Note:
         retval = "note";
         break;

      case Warning:
         retval = "warning";
         break;

      case Pre:
         retval = "pre";
         break;

      case Post:
         retval = "post";
         break;

      case Copyright:
         retval = "copyright";
         break;

      case Invar:
         retval = "invariant";
         break;

      case Remark:
         retval = "remark";
         break;

      case Attention:
         retval = "attention";
         break;

      case User:
         retval = "user";
         break;

      case Rcs:
         retval = "rcs";
         break;
   }

   return retval;
}

int DocParamList::parse(const QString &cmdName)
{
   DBG(("DocParamList::parse() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;

   DocPara *par = 0;
   QString saveCmdName = cmdName;

   int tok = doctokenizerYYlex();

   if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command", csPrintable(cmdName));
      retval = 0;
      goto endparamlist;
   }

   doctokenizerYYsetStateParam();
   tok = doctokenizerYYlex();

   while (tok == TK_WORD) {
      // there is a parameter name

      if (m_type == DocParamSect::Param) {
         int typeSeparator = g_token->name.indexOf('#'); // explicit type position

         if (typeSeparator != -1) {
            handleParameterType(this, m_paramTypes, g_token->name.left(typeSeparator));
            g_token->name     = g_token->name.mid(typeSeparator + 1);
            s_hasParamCommand = true;

            checkArgumentName(g_token->name, true);
            ((DocParamSect *)parent())->m_hasTypeSpecifier = true;

         } else {
            s_hasParamCommand = true;
            checkArgumentName(g_token->name, true);
         }

      } else if (m_type == DocParamSect::RetVal) {
         s_hasReturnCommand = true;
         checkArgumentName(g_token->name, false);
      }

      handleLinkedWord(this, m_params);

      // get next word
      tok = doctokenizerYYlex();
   }

   doctokenizerYYsetStatePara();

   if (tok == 0) {
      // premature end of comment block
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment block while parsing the "
                     "argument of command %s", csPrintable(cmdName));
      retval = 0;
      goto endparamlist;
   }

   if (tok != TK_WHITESPACE) {
      // premature end of comment block
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token in comment block while parsing the "
                     "argument of command %s", csPrintable(saveCmdName));
      retval = 0;
      goto endparamlist;
   }

   par = new DocPara(this);
   m_paragraphs.append(par);
   retval = par->parse();

   par->markFirst();
   par->markLast();

endparamlist:
   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocParamList::parse() end retval = %d\n", retval));

   return retval;
}

int DocParamList::parseXml(const QString &paramName)
{
   DBG(("DocParamList::parseXml() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;
   g_token->name = paramName;

   if (m_type == DocParamSect::Param) {
      s_hasParamCommand = true;
      checkArgumentName(g_token->name, true);

   } else if (m_type == DocParamSect::RetVal) {
      s_hasReturnCommand = true;
      checkArgumentName(g_token->name, false);
   }

   handleLinkedWord(this, m_params);

   do {
      DocPara *par = new DocPara(this);
      retval = par->parse();

      if (par->isEmpty()) {
         // avoid adding an empty paragraph for the whitespace
         // after </para> and before </param>

         delete par;
         break;

      } else {
         // append the paragraph to the list

         if (m_paragraphs.isEmpty()) {
            par->markFirst();
         } else {
            m_paragraphs.last()->markLast(false);
         }

         par->markLast();
         m_paragraphs.append(par);
      }

      if (retval == 0) {
         break;
      }

   } while (retval == RetVal_CloseXml &&
            Mappers::htmlTagMapper->map(g_token->name) != XML_PARAM &&
            Mappers::htmlTagMapper->map(g_token->name) != XML_TYPEPARAM &&
            Mappers::htmlTagMapper->map(g_token->name) != XML_EXCEPTION);


   if (retval == 0) { /* premature end of comment block */
      warn_doc_error(s_fileName, doctokenizerYYlineno, "unterminated param or exception tag");
   } else {
      retval = RetVal_OK;
   }

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocParamList::parse() end retval=%d\n", retval));

   return retval;
}

int DocParamSect::parse(const QString &cmdName, bool xmlContext, Direction d)
{
   DBG(("DocParamSect::parse() start\n"));
   s_nodeStack.push(this);

   int retval = RetVal_OK;

   if (d != Unspecified) {
      m_hasInOutSpecifier = true;
   }

   DocParamList *pl = new DocParamList(this, m_type, d);

   if (m_children.isEmpty()) {
      pl->markFirst();
      pl->markLast();

   } else {
      assert(m_children.last()->kind() == DocNode::Kind_ParamList);
      ((DocParamList *)m_children.last())->markLast(false);
      pl->markLast();
   }

   m_children.append(pl);

   if (xmlContext) {
      retval = pl->parseXml(cmdName);
   } else {
      retval = pl->parse(cmdName);
   }

   if (retval == RetVal_EndParBlock) {
      retval = RetVal_OK;
   }

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocParamSect::parse() end retval = %d\n", retval));

   return retval;
}

int DocPara::handleSimpleSection(DocSimpleSect::Type t, bool xmlContext)
{
   DocSimpleSect *ss   = 0;
   bool needsSeparator = false;

   if (! m_children.isEmpty() &&                                // previous element
         m_children.last()->kind() == Kind_SimpleSect &&        // was a simple sect
         ((DocSimpleSect *)m_children.last())->type() == t &&   // of same type
         t != DocSimpleSect::User) {                            // but not user defined

      // append to previous section
      ss = (DocSimpleSect *)m_children.last();
      needsSeparator = true;

   } else {
      // start new section
      ss = new DocSimpleSect(this, t);
      m_children.append(ss);
   }

   int rv = RetVal_OK;

   if (xmlContext) {
      return ss->parseXml();

   } else {
      rv = ss->parse(t == DocSimpleSect::User, needsSeparator);
   }

   return (rv != TK_NEWPARA) ? rv : RetVal_OK;
}

int DocPara::handleParamSection(const QString &cmdName, DocParamSect::Type t, bool xmlContext = false,
                                int direction = DocParamSect::Unspecified)
{
   DocParamSect *ps = nullptr;

   if (! m_children.isEmpty() && m_children.last()->kind() == Kind_ParamSect &&
         ((DocParamSect *)m_children.last())->type() == t) {

      // previous element was a param sect of same type
      // append to previous section

      ps = (DocParamSect *)m_children.last();

   } else {
      // start new section

      ps = new DocParamSect(this, t);
      m_children.append(ps);
   }

   int rv = ps->parse(cmdName, xmlContext, (DocParamSect::Direction)direction);

   return (rv != TK_NEWPARA) ? rv : RetVal_OK;
}

void DocPara::handleCite()
{
   // get the argument of the cite command.
   int tok = doctokenizerYYlex();

   if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command", csPrintable("cite"));
      return;
   }

   doctokenizerYYsetStateCite();
   tok = doctokenizerYYlex();

   if (tok == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment block while parsing the "
                     "argument of command %s\n", csPrintable("cite"));
      return;

   } else if (tok != TK_WORD && tok != TK_LNKWORD) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of %s",
                     csPrintable(tokToString(tok)), csPrintable("cite"));
      return;
   }

   g_token->sectionId = g_token->name;
   DocCite *cite = new DocCite(this, g_token->name, s_context);
   m_children.append(cite);

   doctokenizerYYsetStatePara();
}

void DocPara::handleSortId()
{
   // get the argument
   int tok = doctokenizerYYlex();

   if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command", csPrintable("sortid"));
      return;
   }

   tok = doctokenizerYYlex();

   if (tok == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment block while parsing the "
                     "argument of command %s\n", csPrintable("sortid"));
      return;

   } else if (tok != TK_WORD && tok != TK_LNKWORD) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of %s",
                     csPrintable(tokToString(tok)), csPrintable("sortid"));
      return;
   }

   if (s_scope) {
      // save the sort id value
      s_scope->setSortId(g_token->name.toInteger<int>());
   }

   doctokenizerYYsetStatePara();
}

void DocPara::handleEmoji()
{
  // get the argument of the emoji command
  int tok = doctokenizerYYlex();

  if (tok != TK_WHITESPACE) {
    warn_doc_error(s_fileName,doctokenizerYYlineno,"Expected whitespace after \\%s command", csPrintable("emoji"));
    return;
  }

  doctokenizerYYsetStateEmoji();
  tok = doctokenizerYYlex();

  if (tok == 0) {
    warn_doc_error(s_fileName, doctokenizerYYlineno, "Unnexpected end of comment block while parsing the "
        "argument of command %s\n", csPrintable("emoji"));
    return;

  } else if (tok != TK_WORD) {
    warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of %s",
        csPrintable(tokToString(tok)), csPrintable("emoji"));
    return;
  }

  DocEmoji *emoji = new DocEmoji(this, g_token->name);
  m_children.append(emoji);
  doctokenizerYYsetStatePara();
}

int DocPara::handleXRefItem()
{
   int retval = doctokenizerYYlex();
   assert(retval == TK_WHITESPACE);
   doctokenizerYYsetStateXRefItem();

   retval = doctokenizerYYlex();

   if (retval == RetVal_OK) {
      DocXRefItem *ref = new DocXRefItem(this, g_token->id, g_token->name);

      if (ref->parse()) {
         m_children.append(ref);
      } else {
         delete ref;
      }
   }

   doctokenizerYYsetStatePara();
   return retval;
}

void DocPara::handleIncludeOperator(const QString &cmdName, DocIncOperator::Type t)
{
   DBG(("handleIncludeOperator(%s)\n", csPrintable(cmdName)));
   int tok = doctokenizerYYlex();

   if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command", csPrintable(cmdName));
      return;
   }

   doctokenizerYYsetStatePattern();
   tok = doctokenizerYYlex();
   doctokenizerYYsetStatePara();

   if (tok == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment block while parsing the "
                     "argument of command %s", csPrintable(cmdName));
      return;

   } else if (tok != TK_WORD) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of %s",
                     csPrintable(tokToString(tok)), csPrintable(cmdName));
      return;
   }

   DocIncOperator *op = new DocIncOperator(this, t, g_token->name, s_context, s_isExample, s_exampleName);

   DocNode *n1 = 0;
   DocNode *n2 = 0;

   if (! m_children.isEmpty()) {
      n1 = m_children[m_children.size() - 1];

      if (m_children.size() > 1) {
         n2 = m_children[m_children.size() - 2];
      }
   }

   // no last node, last node is not operator or whitespace, previous not is not operator
   bool isFirst = n1 == 0 ||  (n1->kind() != DocNode::Kind_IncOperator && n1->kind() != DocNode::Kind_WhiteSpace) ||
                  (n1->kind() == DocNode::Kind_WhiteSpace && n2 != 0 && n2->kind() != DocNode::Kind_IncOperator);

   op->markFirst(isFirst);
   op->markLast(true);

   if (n1 != 0 && n1->kind() == DocNode::Kind_IncOperator) {
      ((DocIncOperator *)n1)->markLast(false);

   } else if (n1 != 0 && n1->kind() == DocNode::Kind_WhiteSpace && n2 != 0 && n2->kind() == DocNode::Kind_IncOperator) {
      ((DocIncOperator *)n2)->markLast(false);
   }

   m_children.append(op);
   op->parse();
}

void DocPara::handleImage(const QString &cmdName)
{
   int tok = doctokenizerYYlex();

   if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command", csPrintable(cmdName));
      return;
   }

   tok = doctokenizerYYlex();

   if (tok != TK_WORD && tok != TK_LNKWORD) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of %s",
                     csPrintable(tokToString(tok)), csPrintable(cmdName));
      return;
   }
   tok = doctokenizerYYlex();
   if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command", csPrintable(cmdName));
      return;
   }

   DocImage::Type t;
   QString imgType = g_token->name.toLower();

   if (imgType == "html") {
      t = DocImage::Html;
   } else if (imgType == "latex") {
      t = DocImage::Latex;
   } else if (imgType == "docbook") {
      t = DocImage::DocBook;
   } else if (imgType == "rtf") {
      t = DocImage::Rtf;
   } else {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "image type %s specified as the first argument of "
                     "%s is not valid", csPrintable(imgType), csPrintable(cmdName));
      return;
   }

   doctokenizerYYsetStateFile();
   tok = doctokenizerYYlex();
   doctokenizerYYsetStatePara();

   if (tok != TK_WORD) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of %s",
                     csPrintable(tokToString(tok)), csPrintable(cmdName));
      return;
   }

   HtmlAttribList attrList;
   DocImage *img = new DocImage(this, attrList, findAndCopyImage(g_token->name, t), t);
   m_children.append(img);
   img->parse();
}

template<class T>
void DocPara::handleFile(const QString &cmdName)
{
   int tok = doctokenizerYYlex();

   if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command",
                     csPrintable(cmdName));
      return;
   }

   doctokenizerYYsetStateFile();
   tok = doctokenizerYYlex();
   doctokenizerYYsetStatePara();

   if (tok != TK_WORD) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of %s",
                     csPrintable(tokToString(tok)), csPrintable(cmdName));
      return;
   }

   QString name = g_token->name;

   T *df = new T(this, name, s_context);
   m_children.append(df);
   df->parse();
}

void DocPara::handleLink(const QString &cmdName, bool isJavaLink)
{
   int tok = doctokenizerYYlex();

  if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command", csPrintable(cmdName));
      return;
   }
   doctokenizerYYsetStateLink();
   tok = doctokenizerYYlex();

   if (tok != TK_WORD) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "%s as the argument of %s",
                    csPrintable(tokToString(tok)), csPrintable(cmdName));
      return;
   }
   doctokenizerYYsetStatePara();
   DocLink *lnk = new DocLink(this, g_token->name);
   m_children.append(lnk);

   QString leftOver = lnk->parse(isJavaLink);
   if (!leftOver.isEmpty()) {
      m_children.append(new DocWord(this, leftOver));
   }
}

void DocPara::handleRef(const QString &cmdName)
{
   DBG(("handleRef(%s)\n", csPrintable(cmdName)));
   int tok = doctokenizerYYlex();

   if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command", csPrintable(cmdName));
      return;
   }

   doctokenizerYYsetStateRef();
   tok = doctokenizerYYlex(); // get the reference id
   DocRef *ref = 0;

   if (tok != TK_WORD) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of %s",
                     csPrintable(tokToString(tok)), csPrintable(cmdName));
      goto endref;
   }

   ref = new DocRef(this, g_token->name, s_context);
   m_children.append(ref);
   ref->parse();

endref:
   doctokenizerYYsetStatePara();
}

void DocPara::handleInclude(const QString &cmdName, DocInclude::Type t)
{
   DBG(("handleInclude(%s)\n", csPrintable(cmdName)));
   int tok = doctokenizerYYlex();
   bool isBlock = false;

   if (tok == TK_WORD && g_token->name == "{") {
      doctokenizerYYsetStateOptions();
      tok = doctokenizerYYlex();
      doctokenizerYYsetStatePara();

      QStringList optList = g_token->name.split(",");

      if (t == DocInclude::Include && optList.contains("lineno")) {
         t = DocInclude::IncWithLines;

      } else if (t == DocInclude::Snippet && optList.contains("lineno")) {
         t = DocInclude::SnipWithLines;

      } else if (t == DocInclude::DontInclude && optList.contains("lineno")) {
         t = DocInclude::DontIncWithLines;

      } else if (t == DocInclude::Include && optList.contains("doc")) {
         t = DocInclude::IncludeDoc;

      } else if (t == DocInclude::Snippet && optList.contains("doc")) {
         t = DocInclude::SnippetDoc;
      }

      tok = doctokenizerYYlex();

   } else if (tok == TK_WORD && g_token->name == "[") {
      doctokenizerYYsetStateBlock();
      tok = doctokenizerYYlex();

      isBlock = (g_token->name.trimmed() == "block");

      doctokenizerYYsetStatePara();
      tok = doctokenizerYYlex();

   } else if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command", csPrintable(cmdName));
      return;
   }

   doctokenizerYYsetStateFile();
   tok = doctokenizerYYlex();
   doctokenizerYYsetStatePara();

   if (tok == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment block while parsing the "
                     "argument of command %s", csPrintable(cmdName));
      return;

   } else if (tok != TK_WORD) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of %s",
                     csPrintable(tokToString(tok)), csPrintable(cmdName));
      return;
   }

   QString fileName = g_token->name;
   QString blockId;

   if (t == DocInclude::Snippet || t == DocInclude::SnippetDoc || t == DocInclude::SnipWithLines) {

      if (fileName == "this") {
         fileName = s_fileName;
      }

      doctokenizerYYsetStateSnippet();
      tok = doctokenizerYYlex();
      doctokenizerYYsetStatePara();

      if (tok != TK_WORD) {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected block identifier, "
               "found token %s instead while parsing the %s command",
               csPrintable(tokToString(tok)), csPrintable(cmdName));

         return;
      }

      blockId = "[" + g_token->name + "]";
   }

  // only place to handle the \includedoc and \snippetdoc commands
  if (t == DocInclude::IncludeDoc || t == DocInclude::SnippetDoc) {
     int inc_line = 1;

     QString inc_text;
     readTextFileByName(fileName, inc_text);

     if (t == DocInclude::SnippetDoc) {
       inc_line = lineBlock(inc_text, blockId);
       inc_text = extractBlock(inc_text, blockId);
     }

     docParserPushContext();
     s_fileName = fileName;
     setDoctokenLineNum(inc_line);
     internalValidatingParseDoc(this,m_children,inc_text);
     docParserPopContext();

  } else {
     DocInclude *inc = new DocInclude(this, fileName, s_context, t, s_isExample, s_exampleName, blockId, isBlock);
     m_children.append(inc);
     inc->parse();
  }
}

void DocPara::handleSection(const QString &cmdName)
{
   // get the argument of the section command
   int tok = doctokenizerYYlex();

   if (tok != TK_WHITESPACE) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Expected whitespace after \\%s command", csPrintable(cmdName));
      return;
   }

   tok = doctokenizerYYlex();

   if (tok == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end of comment block while parsing the "
                     "argument of command %s\n", csPrintable(cmdName));
      return;

   } else if (tok != TK_WORD && tok != TK_LNKWORD) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s as the argument of %s",
                     csPrintable(tokToString(tok)), csPrintable(cmdName));
      return;
   }

   g_token->sectionId = g_token->name;

   doctokenizerYYsetStateSkipTitle();
   doctokenizerYYlex();
   doctokenizerYYsetStatePara();
}

int DocPara::handleHtmlHeader(const HtmlAttribList &tagHtmlAttribs, int level)
{
   DocHtmlHeader *header = new DocHtmlHeader(this, tagHtmlAttribs, level);
   m_children.append(header);

   int retval = header->parse();
   return (retval == RetVal_OK) ? TK_NEWPARA : retval;
}

// For XML tags whose content is stored in attributes rather than
// contained within the element, we need a way to inject the attribute
// text into the current paragraph.
bool DocPara::injectToken(int tok, const QString &tokText)
{
   g_token->name = tokText;
   return defaultHandleToken(this, tok, m_children);
}

int DocPara::handleStartCode()
{
   int retval = doctokenizerYYlex();
   QString lang = g_token->name;

   if (! lang.isEmpty() && lang.at(0) != '.') {
      lang = "." + lang;
   }

   if (s_xmlComment) {
      g_token->verb = substitute(substitute(g_token->verb,"&lt;","<"), "&gt;",">");
   }

   // search for the first non-whitespace line, index is stored in li
   int i  = 0;
   int li = 0;
   int l  = g_token->verb.length();

   while (i < l && (g_token->verb.at(i) == ' ' || g_token->verb.at(i) == '\n')) {
      if (g_token->verb.at(i) == '\n') {
         li = i + 1;
      }
      i++;
   }

   m_children.append(new DocVerbatim(this, s_context, stripIndentation(g_token->verb.mid(li)), DocVerbatim::Code, s_isExample,
                     s_exampleName, false, lang));

   if (retval == 0) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "Code section ended without end marker");
   }

   doctokenizerYYsetStatePara();

   return retval;
}

void DocPara::handleInheritDoc()
{
   if (s_memberDef) {
      // inheriting docs from a member
      QSharedPointer<MemberDef> reMd = s_memberDef->reimplements();

      if (reMd) {
         // member from which was inherited
         QSharedPointer<MemberDef> thisMd = s_memberDef;

         docParserPushContext();
         s_scope = reMd->getOuterScope();

         if (s_scope != Doxy_Globals::globalScope) {
            s_context = s_scope->name();
         }

         s_memberDef = reMd;
         s_styleStack.clear();
         s_nodeStack.clear();
         s_copyStack.append(reMd);

         internalValidatingParseDoc(this, m_children, reMd->briefDescription());
         internalValidatingParseDoc(this, m_children, reMd->documentation());

         s_copyStack.removeOne(reMd);

         docParserPopContext(true);
         s_memberDef = thisMd;
      }
   }
}

int DocPara::handleCommand(const QString &cmdName)
{
   DBG(("handleCommand(%s)\n", csPrintable(cmdName)));

   int retval = RetVal_OK;
   int cmdId  = Mappers::cmdMapper->map(cmdName);

   switch (cmdId) {
      case CMD_UNKNOWN:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Found unknown command '\\%s'", csPrintable(cmdName));
         break;

      case CMD_EMPHASIS:
         m_children.append(new DocStyleChange(this, s_nodeStack.count(), DocStyleChange::Italic, true));
         retval = handleStyleArgument(this, m_children, cmdName);
         m_children.append(new DocStyleChange(this, s_nodeStack.count(), DocStyleChange::Italic, false));

         if (retval != TK_WORD) {
            m_children.append(new DocWhiteSpace(this, " "));
         }
         break;

      case CMD_BOLD:
         m_children.append(new DocStyleChange(this, s_nodeStack.count(), DocStyleChange::Bold, true));
         retval = handleStyleArgument(this, m_children, cmdName);
         m_children.append(new DocStyleChange(this, s_nodeStack.count(), DocStyleChange::Bold, false));

         if (retval != TK_WORD) {
            m_children.append(new DocWhiteSpace(this, " "));
         }
         break;

      case CMD_CODE:
         m_children.append(new DocStyleChange(this, s_nodeStack.count(), DocStyleChange::Code, true));
         retval = handleStyleArgument(this, m_children, cmdName);
         m_children.append(new DocStyleChange(this, s_nodeStack.count(), DocStyleChange::Code, false));

         if (retval != TK_WORD) {
            m_children.append(new DocWhiteSpace(this, " "));
         }
         break;

      case CMD_BSLASH:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_BSlash));
         break;

      case CMD_AT:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_At));
         break;

      case CMD_LESS:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Less));
         break;

      case CMD_GREATER:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Greater));
         break;

      case CMD_AMP:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Amp));
         break;

      case CMD_DOLLAR:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Dollar));
         break;

      case CMD_HASH:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Hash));
         break;

      case CMD_PIPE:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Pipe));
         break;

      case CMD_DCOLON:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_DoubleColon));
         break;

      case CMD_PERCENT:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Percent));
         break;

      case CMD_NDASH:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Minus));
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Minus));
         break;

      case CMD_MDASH:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Minus));
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Minus));
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Minus));
         break;

      case CMD_QUOTE:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Quot));
         break;

      case CMD_PUNT:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Dot));
         break;

      case CMD_PLUS:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Plus));
         break;

      case CMD_MINUS:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Minus));
         break;

      case CMD_EQUAL:
         m_children.append(new DocSymbol(this, DocSymbol::Sym_Equal));
         break;

      case CMD_SA:
         s_inSeeBlock = true;
         retval = handleSimpleSection(DocSimpleSect::See);
         s_inSeeBlock = false;
         break;

      case CMD_RETURN:
         retval = handleSimpleSection(DocSimpleSect::Return);
         s_hasReturnCommand = true;
         break;

      case CMD_AUTHOR:
         retval = handleSimpleSection(DocSimpleSect::Author);
         break;

      case CMD_AUTHORS:
         retval = handleSimpleSection(DocSimpleSect::Authors);
         break;

      case CMD_VERSION:
         retval = handleSimpleSection(DocSimpleSect::Version);
         break;

      case CMD_SINCE:
         retval = handleSimpleSection(DocSimpleSect::Since);
         break;

      case CMD_DATE:
         retval = handleSimpleSection(DocSimpleSect::Date);
         break;

      case CMD_NOTE:
         retval = handleSimpleSection(DocSimpleSect::Note);
         break;

      case CMD_WARNING:
         retval = handleSimpleSection(DocSimpleSect::Warning);
         break;

      case CMD_PRE:
         retval = handleSimpleSection(DocSimpleSect::Pre);
         break;

      case CMD_POST:
         retval = handleSimpleSection(DocSimpleSect::Post);
         break;

      case CMD_COPYRIGHT:
         retval = handleSimpleSection(DocSimpleSect::Copyright);
         break;

      case CMD_INVARIANT:
         retval = handleSimpleSection(DocSimpleSect::Invar);
         break;

      case CMD_REMARK:
         retval = handleSimpleSection(DocSimpleSect::Remark);
         break;

      case CMD_ATTENTION:
         retval = handleSimpleSection(DocSimpleSect::Attention);
         break;

      case CMD_PAR:
         retval = handleSimpleSection(DocSimpleSect::User);
         break;

      case CMD_LI: {
         DocSimpleList *sl = new DocSimpleList(this);
         m_children.append(sl);
         retval = sl->parse();
      }
      break;

      case CMD_SECTION: {
         handleSection(cmdName);
         retval = RetVal_Section;
      }
      break;

      case CMD_SUBSECTION: {
         handleSection(cmdName);
         retval = RetVal_Subsection;
      }
      break;

      case CMD_SUBSUBSECTION: {
         handleSection(cmdName);
         retval = RetVal_Subsubsection;
      }
      break;

      case CMD_PARAGRAPH: {
         handleSection(cmdName);
         retval = RetVal_Paragraph;
      }
      break;

      case CMD_STARTCODE: {
         doctokenizerYYsetStateCode();
         retval = handleStartCode();
      }
      break;

      case CMD_HTMLONLY: {
         doctokenizerYYsetStateHtmlOnly();
         retval = doctokenizerYYlex();
         m_children.append(new DocVerbatim(this, s_context, g_token->verb, DocVerbatim::HtmlOnly,
                  s_isExample, s_exampleName, g_token->name == "block"));

         if (retval == 0) {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "htmlonly section ended without end marker");
         }
         doctokenizerYYsetStatePara();
      }
      break;

      case CMD_MANONLY: {
         doctokenizerYYsetStateManOnly();
         retval = doctokenizerYYlex();
         m_children.append(new DocVerbatim(this, s_context, g_token->verb, DocVerbatim::ManOnly, s_isExample, s_exampleName));

         if (retval == 0) {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "manonly section ended without end marker");
         }
         doctokenizerYYsetStatePara();
      }
      break;

      case CMD_RTFONLY: {
         doctokenizerYYsetStateRtfOnly();
         retval = doctokenizerYYlex();
         m_children.append(new DocVerbatim(this, s_context, g_token->verb, DocVerbatim::RtfOnly, s_isExample, s_exampleName));

         if (retval == 0) {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "rtfonly section ended without end marker");
         }
         doctokenizerYYsetStatePara();
      }
      break;

      case CMD_LATEXONLY: {
         doctokenizerYYsetStateLatexOnly();
         retval = doctokenizerYYlex();
         m_children.append(new DocVerbatim(this, s_context, g_token->verb, DocVerbatim::LatexOnly, s_isExample, s_exampleName));

         if (retval == 0) {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "latexonly section ended without end marker");
         }
         doctokenizerYYsetStatePara();
      }
      break;

      case CMD_XMLONLY: {
         doctokenizerYYsetStateXmlOnly();
         retval = doctokenizerYYlex();
         m_children.append(new DocVerbatim(this, s_context, g_token->verb, DocVerbatim::XmlOnly, s_isExample, s_exampleName));

         if (retval == 0) {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "xmlonly section ended without end marker");
         }
         doctokenizerYYsetStatePara();
      }
      break;

      case CMD_DBONLY: {
         doctokenizerYYsetStateDbOnly();
         retval = doctokenizerYYlex();
         m_children.append(new DocVerbatim(this, s_context, g_token->verb, DocVerbatim::DocbookOnly, s_isExample, s_exampleName));

         if (retval == 0) {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Docbookonly section ended without end marker", doctokenizerYYlineno);
         }
         doctokenizerYYsetStatePara();
      }
      break;

      case CMD_VERBATIM: {

         doctokenizerYYsetStateVerbatim();
         retval = doctokenizerYYlex();

         m_children.append(new DocVerbatim(this, s_context, g_token->verb, DocVerbatim::Verbatim, s_isExample, s_exampleName));

         if (retval == 0) {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Verbatim section ended without an end marker");
         }
         doctokenizerYYsetStatePara();
      }
      break;

      case CMD_DOT: {
         DocVerbatim *dv = new DocVerbatim(this, s_context, g_token->verb, DocVerbatim::Dot, s_isExample, s_exampleName);
         doctokenizerYYsetStatePara();

         QString width;
         QString height;

         defaultHandleTitleAndSize(CMD_DOT, dv, dv->children(),  width, height);
         doctokenizerYYsetStateDot();

         retval = doctokenizerYYlex();

         dv->setText(g_token->verb);
         dv->setWidth(width);
         dv->setHeight(height);

         m_children.append(dv);

         if (retval == 0) {
            warn_doc_error(s_fileName, doctokenizerYYlineno,"Dot section ended without an end marker");
         }

         doctokenizerYYsetStatePara();
      }
      break;

      case CMD_MSC: {
         DocVerbatim *dv = new DocVerbatim(this, s_context, g_token->verb, DocVerbatim::Msc, s_isExample, s_exampleName);
         doctokenizerYYsetStatePara();

         QString width;
         QString height;

         defaultHandleTitleAndSize(CMD_MSC, dv, dv->children(), width, height);
         doctokenizerYYsetStateMsc();

         retval = doctokenizerYYlex();

         dv->setText(g_token->verb);
         dv->setWidth(width);
         dv->setHeight(height);

         m_children.append(dv);

         if (retval == 0) {
            warn_doc_error(s_fileName, doctokenizerYYlineno,"Msc section ended without an end marker");
         }

         doctokenizerYYsetStatePara();
      }
      break;

      case CMD_STARTUML: {
         static const QString jarPath = Config::getString("plantuml-jar-path");

         doctokenizerYYsetStatePlantUMLOpt();
         retval = doctokenizerYYlex();

         QString plantFile(g_token->sectionId);

         DocVerbatim *dv = new DocVerbatim(this, s_context, g_token->verb, DocVerbatim::PlantUML, false, s_exampleName);
         doctokenizerYYsetStatePara();

         QString width;
         QString height;

         defaultHandleTitleAndSize(CMD_STARTUML, dv, dv->children(), width, height);

         doctokenizerYYsetStatePlantUML();
         retval = doctokenizerYYlex();

         int line = 0;

         dv->setText(trimEmptyLines(g_token->verb, line));
         dv->setWidth(width);
         dv->setHeight(height);

         if (jarPath.isEmpty()) {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Ignoring \\startuml command since 'PLANTUML JAR PATH' was not set");
            delete dv;

         } else {
            m_children.append(dv);
         }

         if (retval == 0) {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "startuml section ended without an end marker");
         }

         doctokenizerYYsetStatePara();
      }
      break;

      case CMD_ENDPARBLOCK:
         retval = RetVal_EndParBlock;
         break;

      case CMD_ENDCODE:
      case CMD_ENDHTMLONLY:
      case CMD_ENDMANONLY:
      case CMD_ENDRTFONLY:
      case CMD_ENDLATEXONLY:
      case CMD_ENDXMLONLY:
      case CMD_ENDDBONLY:
      case CMD_ENDLINK:
      case CMD_ENDVERBATIM:
      case CMD_ENDDOT:
      case CMD_ENDMSC:
      case CMD_ENDUML:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected command %s inside paragraph", csPrintable(g_token->name));
         break;

      case CMD_PARAM:
         retval = handleParamSection(cmdName, DocParamSect::Param, false, g_token->paramDir);
         break;

      case CMD_TPARAM:
         retval = handleParamSection(cmdName, DocParamSect::TemplateParam, false, g_token->paramDir);
         break;

      case CMD_RETVAL:
         retval = handleParamSection(cmdName, DocParamSect::RetVal);
         break;

      case CMD_EXCEPTION:
         retval = handleParamSection(cmdName, DocParamSect::Exception);
         break;

      case CMD_XREFITEM:
         retval = handleXRefItem();
         break;

      case CMD_LINEBREAK: {
         DocLineBreak *lb = new DocLineBreak(this);
         m_children.append(lb);
      }
      break;

      case CMD_ANCHOR: {
         DocAnchor *anchor = handleAnchor(this);

         if (anchor) {
            m_children.append(anchor);
         }
      }
      break;

      case CMD_ANCHORNAME: {
          handleAnchorName(this);
      }
      break;

      case CMD_ADDINDEX: {
         QSharedPointer<Definition> tmp;

         if (s_scope != Doxy_Globals::globalScope) {
            tmp = s_scope;
         } else {
            tmp = QSharedPointer<Definition>();
          }

         DocIndexEntry *ie = new DocIndexEntry(this, tmp, s_memberDef);
         m_children.append(ie);
         retval = ie->parse();
      }
      break;

      case CMD_INTERNAL:
         retval = RetVal_Internal;
         break;

      case CMD_ENDINTERNAL:
         retval = RetVal_EndInternal;
         break;

      case CMD_PARBLOCK: {
         DocParBlock *block = new DocParBlock(this);
         m_children.append(block);
         retval = block->parse();
      }
      break;

      case CMD_COPYDOC:
      case CMD_COPYBRIEF:
      case CMD_COPYDETAILS:
         // retval = RetVal_CopyDoc;
         // these commands should already be resolved by processCopyDoc()
         break;

      case CMD_INCLUDE:
         handleInclude(cmdName, DocInclude::Include);
         break;

      case CMD_INCWITHLINES:
         handleInclude(cmdName, DocInclude::IncWithLines);
         break;

      case CMD_DONTINCLUDE:
         handleInclude(cmdName, DocInclude::DontInclude);
         break;

      case CMD_HTMLINCLUDE:
         handleInclude(cmdName, DocInclude::HtmlInclude);
         break;

      case CMD_LATEXINCLUDE:
         handleInclude(cmdName, DocInclude::LatexInclude);
         break;

      case CMD_RTFINCLUDE:
         handleInclude(cmdName,DocInclude::RtfInclude);
         break;

      case CMD_MANINCLUDE:
         handleInclude(cmdName,DocInclude::ManInclude);
         break;

      case CMD_XMLINCLUDE:
         handleInclude(cmdName,DocInclude::XmlInclude);
         break;

      case CMD_DOCBOOKINCLUDE:
         handleInclude(cmdName,DocInclude::DocbookInclude);
         break;

      case CMD_VERBINCLUDE:
         handleInclude(cmdName, DocInclude::VerbInclude);
         break;

      case CMD_SNIPPET:
         handleInclude(cmdName, DocInclude::Snippet);
         break;

      case CMD_SNIPWITHLINES:
         handleInclude(cmdName, DocInclude::SnipWithLines);
         break;

      case CMD_INCLUDEDOC:
         handleInclude(cmdName, DocInclude::IncludeDoc);
         break;

      case CMD_SNIPPETDOC:
         handleInclude(cmdName, DocInclude::SnippetDoc);
         break;

      case CMD_SKIP:
         handleIncludeOperator(cmdName, DocIncOperator::Skip);
         break;

      case CMD_UNTIL:
         handleIncludeOperator(cmdName, DocIncOperator::Until);
         break;

      case CMD_SKIPLINE:
         handleIncludeOperator(cmdName, DocIncOperator::SkipLine);
         break;

      case CMD_LINE:
         handleIncludeOperator(cmdName, DocIncOperator::Line);
         break;

      case CMD_IMAGE:
         handleImage(cmdName);
         break;

      case CMD_DOTFILE:
         handleFile<DocDotFile>(cmdName);
         break;

      case CMD_MSCFILE:
         handleFile<DocMscFile>(cmdName);
         break;

      case CMD_DIAFILE:
         handleFile<DocDiaFile>(cmdName);
         break;

      case CMD_LINK:
         handleLink(cmdName, false);
         break;

      case CMD_JAVALINK:
         handleLink(cmdName, true);
         break;

      case CMD_CITE:
         handleCite();
         break;

      case CMD_EMOJI:
         handleEmoji();
         break;

      case CMD_REF:
      case CMD_SUBPAGE:
         handleRef(cmdName);
         break;

      case CMD_SECREFLIST: {
         DocSecRefList *list = new DocSecRefList(this);
         m_children.append(list);
         list->parse();
      }
      break;

      case CMD_SECREFITEM:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected command %s inside paragraph", csPrintable(g_token->name));
         break;

      case CMD_ENDSECREFLIST:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected command %s inside paragraph", csPrintable(g_token->name));
         break;

      case CMD_FORMULA: {
         DocFormula *form = new DocFormula(this, g_token->id);
         m_children.append(form);
      }
      break;

      //case CMD_LANGSWITCH:
      //  retval = handleLanguageSwitch();
      //  break;

      case CMD_INTERNALREF: {
         // warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected command %s", csPrintable(g_token->name));

         DocInternalRef *ref = handleInternalRef(this);
         if (ref) {
            m_children.append(ref);
            ref->parse();
         }

         doctokenizerYYsetStatePara();
      }
      break;

      case CMD_INHERITDOC:
         handleInheritDoc();
         break;

      case CMD_SORTID:
         handleSortId();
         break;

      default:
         // should not get here
         warn(s_fileName, doctokenizerYYlineno, "Invalid state, command = %d\n", cmdId);

         assert(0);
         break;
   }

   INTERNAL_ASSERT(retval == 0 || retval == RetVal_OK || retval == RetVal_SimpleSec ||
                   retval == TK_LISTITEM || retval == TK_ENDLIST || retval == TK_NEWPARA ||
                   retval == RetVal_Section || retval == RetVal_EndList ||
                   retval == RetVal_Internal || retval == RetVal_SwitchLang ||
                   retval == RetVal_EndInternal
                  );

   DBG(("handleCommand(%s) end retval=%x\n", csPrintable(cmdName), retval));

   return retval;
}

static bool findAttribute(const HtmlAttribList &tagHtmlAttribs, const QString &attrName, QString *result)
{
   for (auto opt : tagHtmlAttribs) {
      if (opt.name == attrName) {
         *result = opt.value;
         return true;
      }
   }

   return false;
}

int DocPara::handleHtmlStartTag(const QString &tagName, const HtmlAttribList &tagHtmlAttribs)
{
   DBG(("handleHtmlStartTag(%s,%d)\n", csPrintable(tagName), tagHtmlAttribs.count()));

   int retval = RetVal_OK;
   int tagId  = Mappers::htmlTagMapper->map(tagName);

   if (g_token->emptyTag && ! (tagId & XML_CmdMask) &&
         tagId != HTML_UNKNOWN && tagId != HTML_IMG && tagId != HTML_BR && tagId != HTML_HR && tagId != HTML_P) {
      warn_doc_error(s_fileName, doctokenizerYYlineno, "HTML tags may not use the 'empty tag' XHTML syntax");
   }

   switch (tagId) {

      case HTML_UL:
          if (! g_token->emptyTag) {
            DocHtmlList *list = new DocHtmlList(this, tagHtmlAttribs, DocHtmlList::Unordered);
            m_children.append(list);
            retval = list->parse();
         }
         break;

      case HTML_OL:
         if (! g_token->emptyTag) {
            DocHtmlList *list = new DocHtmlList(this, tagHtmlAttribs, DocHtmlList::Ordered);
            m_children.append(list);
            retval = list->parse();
         }
         break;

      case HTML_LI:
         if (g_token->emptyTag) {
            break;
         }

         if (! insideUL(this) && ! insideOL(this)) {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Lonely <li> tag found");
         } else {
            retval = RetVal_ListItem;
         }
         break;

      case HTML_BOLD:
         if (! g_token->emptyTag) {
            handleStyleEnter(this, m_children, DocStyleChange::Bold, &g_token->attribs);
         }
         break;

      case HTML_STRIKE:
         if (! g_token->emptyTag) {
            handleStyleEnter(this, m_children,DocStyleChange::Strike, &g_token->attribs);
         }
         break;

      case HTML_DEL:
         if (! g_token->emptyTag) {
            handleStyleEnter(this, m_children,DocStyleChange::Del, &g_token->attribs);
         }
         break;

      case HTML_UNDERLINE:
         if (! g_token->emptyTag) {
            handleStyleEnter(this, m_children,DocStyleChange::Underline, &g_token->attribs);
         }
         break;

      case HTML_INS:
         if (! g_token->emptyTag) {
            handleStyleEnter(this, m_children,DocStyleChange::Ins, &g_token->attribs);
         }
         break;

      case HTML_CODE:
         if (g_token->emptyTag) {
            break;
         }

         // getLanguageFromFileName(s_fileName) == SrcLangExt_CSharp ||
         if (s_xmlComment)  {
            // for C# source or inside a <summary> or <remark> section we
            // treat <code> as an XML tag (so similar to @code)

            doctokenizerYYsetStateXmlCode();
            retval = handleStartCode();

         } else {
            // normal HTML markup
            handleStyleEnter(this, m_children, DocStyleChange::Code, &g_token->attribs);
         }
         break;

      case HTML_EMPHASIS:
         if (! g_token->emptyTag) {
            handleStyleEnter(this, m_children, DocStyleChange::Italic, &g_token->attribs);
         }
         break;

      case HTML_DIV:
         if (! g_token->emptyTag) {
            handleStyleEnter(this, m_children, DocStyleChange::Div, &g_token->attribs);
         }
         break;

      case HTML_SPAN:
         if (! g_token->emptyTag) {
            handleStyleEnter(this, m_children, DocStyleChange::Span, &g_token->attribs);
         }
         break;

      case HTML_SUB:
         if (! g_token->emptyTag) {
            handleStyleEnter(this, m_children, DocStyleChange::Subscript, &g_token->attribs);
         }
         break;

      case HTML_SUP:
         if (! g_token->emptyTag) {
            handleStyleEnter(this, m_children, DocStyleChange::Superscript, &g_token->attribs);
         }
         break;

      case HTML_CENTER:
         if (! g_token->emptyTag) {
            handleStyleEnter(this, m_children, DocStyleChange::Center, &g_token->attribs);
         }
         break;

      case HTML_SMALL:
         if (! g_token->emptyTag) {
            handleStyleEnter(this, m_children, DocStyleChange::Small, &g_token->attribs);
         }
         break;

      case HTML_PRE:
         if (g_token->emptyTag) {
            break;
         }

         handleStyleEnter(this, m_children, DocStyleChange::Preformatted, &g_token->attribs);
         setInsidePreformatted(true);
         doctokenizerYYsetInsidePre(true);
         break;

      case HTML_P:
         retval = TK_NEWPARA;
         break;

      case HTML_DL:
         if (! g_token->emptyTag) {
            DocHtmlDescList *list = new DocHtmlDescList(this, tagHtmlAttribs);
            m_children.append(list);
            retval = list->parse();
         }
         break;

      case HTML_DT:
         retval = RetVal_DescTitle;
         break;

      case HTML_DD:
         retval = RetVal_DescData;
         break;

      case HTML_TABLE:
         if (! g_token->emptyTag) {
            DocHtmlTable *table = new DocHtmlTable(this, tagHtmlAttribs);
            m_children.append(table);
            retval = table->parse();
         }
         break;

      case HTML_TR:
         retval = RetVal_TableRow;
         break;

      case HTML_TD:
         retval = RetVal_TableCell;
         break;

      case HTML_TH:
         retval = RetVal_TableHCell;
         break;

      case HTML_CAPTION:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected tag <caption> found");
         break;

      case HTML_BR: {
         DocLineBreak *lb = new DocLineBreak(this);
         m_children.append(lb);
      }
      break;

      case HTML_HR: {
         DocHorRuler *hr = new DocHorRuler(this);
         m_children.append(hr);
      }
      break;

      case HTML_A:
         retval = handleAHref(this, m_children, tagHtmlAttribs);
         break;

      case HTML_H1:
         if (! g_token->emptyTag) {
            retval = handleHtmlHeader(tagHtmlAttribs, 1);
         }
         break;

      case HTML_H2:
         if (! g_token->emptyTag) {
            retval = handleHtmlHeader(tagHtmlAttribs, 2);
         }
         break;

      case HTML_H3:
         if (! g_token->emptyTag) {
            retval = handleHtmlHeader(tagHtmlAttribs, 3);
         }
         break;

      case HTML_H4:
         if (! g_token->emptyTag) {
            retval = handleHtmlHeader(tagHtmlAttribs, 4);
         }
         break;

      case HTML_H5:
         if (! g_token->emptyTag) {
            retval = handleHtmlHeader(tagHtmlAttribs, 5);
         }
         break;

      case HTML_H6:
         if (! g_token->emptyTag) {
            retval = handleHtmlHeader(tagHtmlAttribs, 6);
         }
         break;

      case HTML_IMG:
         handleImg(this, m_children, tagHtmlAttribs);
         break;

      case HTML_BLOCKQUOTE:
         if (! g_token->emptyTag) {
            DocHtmlBlockQuote *block = new DocHtmlBlockQuote(this, tagHtmlAttribs);
            m_children.append(block);
            retval = block->parse();
         }
         break;

      case XML_SUMMARY:
      case XML_REMARKS:
      case XML_EXAMPLE:
         s_xmlComment = true;
         [[fallthrough]];

      case XML_VALUE:
      case XML_PARA:
         if (! m_children.isEmpty()) {
            retval = TK_NEWPARA;
         }
         break;

      case XML_DESCRIPTION:
         if (insideTable(this)) {
            retval = RetVal_TableCell;
         }
         break;

      case XML_C:
         handleStyleEnter(this, m_children, DocStyleChange::Code, &g_token->attribs);
         break;

      case XML_PARAM:
      case XML_TYPEPARAM: {
         QString paramName;

         s_xmlComment = true;

         if (findAttribute(tagHtmlAttribs, "name", &paramName)) {

            if (paramName.isEmpty()) {
               if (Config::getBool("warn-undoc-param")) {
                  warn_doc_error(s_fileName, doctokenizerYYlineno, "Empty 'name' attribute for <param%s> tag.",
                        tagId == XML_PARAM ? "" : "type");

               }

            } else {
               retval = handleParamSection(paramName,
                        tagId == XML_PARAM ? DocParamSect::Param : DocParamSect::TemplateParam, true);
            }

         } else {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Missing 'name' attribute from <param%s> tag.",
                        tagId == XML_PARAM ? "" : "type");

         }
      }
      break;

      case XML_PARAMREF:
      case XML_TYPEPARAMREF: {
         QString paramName;

         if (findAttribute(tagHtmlAttribs, "name", &paramName)) {
            m_children.append(new DocStyleChange(this, s_nodeStack.count(), DocStyleChange::Italic, true));
            m_children.append(new DocWord(this, paramName));
            m_children.append(new DocStyleChange(this, s_nodeStack.count(), DocStyleChange::Italic, false));

            if (retval != TK_WORD) {
               m_children.append(new DocWhiteSpace(this, " "));
            }

         } else {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Missing 'name' attribute from <param%sref> tag.",
                  tagId == XML_PARAMREF ? "" : "type");
         }
      }
      break;

      case XML_EXCEPTION: {
         QString exceptName;

         s_xmlComment = true;

         if (findAttribute(tagHtmlAttribs, "cref", &exceptName)) {
            unescapeCRef(exceptName);
            retval = handleParamSection(exceptName, DocParamSect::Exception, true);

         } else {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Missing 'cref' attribute from <exception> tag.");
         }
      }

      break;

      case XML_ITEM:
      case XML_LISTHEADER:
         if (insideTable(this)) {
            retval = RetVal_TableRow;
         } else if (insideUL(this) || insideOL(this)) {
            retval = RetVal_ListItem;
         } else {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "lonely <item> tag found");
         }
         break;

      case XML_RETURNS:
         s_xmlComment = true;
         retval = handleSimpleSection(DocSimpleSect::Return, true);
         s_hasReturnCommand = true;
         break;

      case XML_TERM:
         //m_children.append(new DocStyleChange(this,s_nodeStack.count(),DocStyleChange::Bold,true));
         if (insideTable(this)) {
            retval = RetVal_TableCell;
         }
         break;

      case XML_SEE:
         // not sure if <see> is the same as <seealso> or if it should you link a member
         // without producing a section. The C# specification is extremely vague
      {
         QString cref;

         if (findAttribute(tagHtmlAttribs, "cref", &cref)) {
            unescapeCRef(cref);

            if (g_token->emptyTag) {
               // <see cref="..."/> style

               bool inSeeBlock = s_inSeeBlock;
               g_token->name   = cref;
               s_inSeeBlock    = true;
               handleLinkedWord(this, m_children, true);

               s_inSeeBlock = inSeeBlock;

            } else {
               // <see cref="...">...</see> style

               doctokenizerYYsetStatePara();
               DocLink *lnk = new DocLink(this, cref);
               m_children.append(lnk);

               QString leftOver = lnk->parse(false, true);
               if (! leftOver.isEmpty()) {
                  m_children.append(new DocWord(this, leftOver));
               }
            }

         } else if (findAttribute(tagHtmlAttribs, "langword", &cref)) {
            // <see langword="..."/> or <see langword="..."></see>

            bool inSeeBlock = s_inSeeBlock;
            g_token->name   = cref;
            s_inSeeBlock    = true;

            m_children.append(new DocStyleChange(this, s_nodeStack.count(), DocStyleChange::Code, true));
            handleLinkedWord(this, m_children,true);

            m_children.append(new DocStyleChange(this, s_nodeStack.count(), DocStyleChange::Code, false));
            s_inSeeBlock = inSeeBlock;

         } else {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Missing 'cref' or 'langword' attribute from <see> tag.");
         }
      }
      break;

      case XML_SEEALSO: {
         QString cref;

         s_xmlComment = true;

         if (findAttribute(tagHtmlAttribs, "cref", &cref)) {
            // Look for an existing "see" section
            unescapeCRef(cref);

            DocSimpleSect *ss = nullptr;

            for (auto n : m_children) {
               if (n->kind() == Kind_SimpleSect && ((DocSimpleSect *)n)->type() == DocSimpleSect::See) {
                  ss = (DocSimpleSect *)n;
               }
            }

            if (!ss) { // start new section
               ss = new DocSimpleSect(this, DocSimpleSect::See);
               m_children.append(ss);
            }

            ss->appendLinkWord(cref);
            retval = RetVal_OK;
         } else {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Missing 'cref' attribute from <seealso> tag.");
         }
      }
      break;

      case XML_LIST: {
         QString type;
         findAttribute(tagHtmlAttribs, "type", &type);
         DocHtmlList::Type listType = DocHtmlList::Unordered;
         HtmlAttribList emptyList;
         if (type == "number") {
            listType = DocHtmlList::Ordered;
         }

         if (type == "table") {
            DocHtmlTable *table = new DocHtmlTable(this, emptyList);
            m_children.append(table);
            retval = table->parseXml();

         } else {
            DocHtmlList *list = new DocHtmlList(this, emptyList, listType);
            m_children.append(list);
            retval = list->parseXml();
         }
      }
      break;

      case XML_INCLUDE:
      case XML_PERMISSION:
         // These tags are defined in .Net but are currently unsupported
         s_xmlComment = true;
         break;

      case HTML_UNKNOWN:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unsupported xml/html tag <%s> found", csPrintable(tagName));
         m_children.append(new DocWord(this, "<" + tagName + tagHtmlAttribs.toString() + ">"));
         break;

      case XML_INHERITDOC:
         handleInheritDoc();
         break;

      default:
         // we should not get here!
         assert(0);
         break;
   }

   return retval;
}

int DocPara::handleHtmlEndTag(const QString &tagName)
{
   DBG(("handleHtmlEndTag(%s)\n", csPrintable(tagName)));

   int tagId  = Mappers::htmlTagMapper->map(tagName);
   int retval = RetVal_OK;

   switch (tagId) {

      case HTML_UL:
         if (! insideUL(this)) {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "found </ul> tag without matching <ul>");
         } else {
            retval = RetVal_EndList;
         }
         break;

      case HTML_OL:
         if (! insideOL(this)) {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Found </ol> tag without matching <ol>");
         } else {
            retval = RetVal_EndList;
         }
         break;

      case HTML_LI:
         if (! insideLI(this)) {
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Found </li> tag without matching <li>");
         } else {
            // ignore </li> tags
         }
         break;

      case HTML_BLOCKQUOTE:
         retval = RetVal_EndBlockQuote;
         break;

      //case HTML_PRE:
      //  if (! insidePRE(this))
      //  {
      //    warn_doc_error(s_fileName,doctokenizerYYlineno,"Found </pre> tag without matching <pre>");
      //  }
      //  else
      //  {
      //    retval=RetVal_EndPre;
      //  }
      //  break;

      case HTML_BOLD:
         handleStyleLeave(this, m_children, DocStyleChange::Bold, "b");
         break;

      case HTML_STRIKE:
         handleStyleLeave(this, m_children,DocStyleChange::Strike, "strike");
         break;

      case HTML_DEL:
         handleStyleLeave(this, m_children, DocStyleChange::Del, "del");
         break;

      case HTML_UNDERLINE:
         handleStyleLeave(this, m_children,DocStyleChange::Underline, "u");
         break;

      case HTML_INS:
         handleStyleLeave(this, m_children,DocStyleChange::Ins, "ins");
         break;

      case HTML_CODE:
         handleStyleLeave(this, m_children, DocStyleChange::Code, "code");
         break;

      case HTML_EMPHASIS:
         handleStyleLeave(this, m_children, DocStyleChange::Italic, "em");
         break;

      case HTML_DIV:
         handleStyleLeave(this, m_children, DocStyleChange::Div, "div");
         break;

      case HTML_SPAN:
         handleStyleLeave(this, m_children, DocStyleChange::Span, "span");
         break;

      case HTML_SUB:
         handleStyleLeave(this, m_children, DocStyleChange::Subscript, "sub");
         break;

      case HTML_SUP:
         handleStyleLeave(this, m_children, DocStyleChange::Superscript, "sup");
         break;

      case HTML_CENTER:
         handleStyleLeave(this, m_children, DocStyleChange::Center, "center");
         break;

      case HTML_SMALL:
         handleStyleLeave(this, m_children, DocStyleChange::Small, "small");
         break;

      case HTML_PRE:
         handleStyleLeave(this, m_children, DocStyleChange::Preformatted, "pre");
         setInsidePreformatted(false);
         doctokenizerYYsetInsidePre(false);
         break;

      case HTML_P:
         retval = TK_NEWPARA;
         break;

      case HTML_DL:
         retval = RetVal_EndDesc;
         break;

      case HTML_DT:
         // ignore </dt> tag
         break;

      case HTML_DD:
         // ignore </dd> tag
         break;

      case HTML_TABLE:
         retval = RetVal_EndTable;
         break;

      case HTML_TR:
         // ignore </tr> tag
         break;

      case HTML_TD:
         // ignore </td> tag
         break;

      case HTML_TH:
         // ignore </th> tag
         break;

      case HTML_CAPTION:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected tag </caption> found");
         break;
      case HTML_BR:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Illegal </br> tag found\n");
         break;
      case HTML_H1:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected tag </h1> found");
         break;
      case HTML_H2:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected tag </h2> found");
         break;
      case HTML_H3:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected tag </h3> found");
         break;
      case HTML_H4:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected tag </h4> found");
         break;
      case HTML_H5:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected tag </h5> found");
         break;
      case HTML_H6:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected tag </h6> found");
         break;
      case HTML_IMG:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected tag </img> found");
         break;
      case HTML_HR:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected tag </hr> found");
         break;
      case HTML_A:
         //warn_doc_error(s_fileName,doctokenizerYYlineno,"Unexpected tag </a> found");
         // ignore </a> tag (can be part of <a name=...></a>
         break;

      case XML_TERM:
         //m_children.append(new DocStyleChange(this,s_nodeStack.count(),DocStyleChange::Bold,false));
         break;

      case XML_SUMMARY:
      case XML_REMARKS:
      case XML_PARA:
      case XML_VALUE:
      case XML_EXAMPLE:
      case XML_PARAM:
      case XML_LIST:
      case XML_TYPEPARAM:
      case XML_RETURNS:
      case XML_SEE:
      case XML_SEEALSO:
      case XML_EXCEPTION:
      case XML_INHERITDOC:
         retval = RetVal_CloseXml;
         break;

      case XML_C:
         handleStyleLeave(this, m_children, DocStyleChange::Code, "c");
         break;

      case XML_ITEM:
      case XML_LISTHEADER:
      case XML_INCLUDE:
      case XML_PERMISSION:
      case XML_DESCRIPTION:
      case XML_PARAMREF:
      case XML_TYPEPARAMREF:
         // These tags are defined in .Net but are currently unsupported
         break;

      case HTML_UNKNOWN:
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unsupported xml/html tag </%s> found", csPrintable(tagName));
         m_children.append(new DocWord(this, "</" + tagName + ">"));
         break;

      default:
         // we should not get here
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected end tag %s\n", csPrintable(tagName));
         assert(0);
         break;
   }

   return retval;
}

int DocPara::parse(bool skipParse, int token)
{
   DBG(("DocPara::parse() start\n"));
   s_nodeStack.push(this);

   // handle style commands "inherited" from the previous paragraph
   handleInitialStyleCommands(this, m_children);

   int tok;
   int retval = 0;

   while (true)  {

      if (skipParse) {
         // passed from DocRoot::parse()
         tok = token;
         skipParse = false;

      } else {
         // get the next token
         tok = doctokenizerYYlex();

      }

      if (tok == 0) {
         break;
      }

   reparsetoken:

      switch (tok) {

         case TK_WORD:
            m_children.append(new DocWord(this, g_token->name));
            break;

         case TK_LNKWORD:
            handleLinkedWord(this, m_children);
            break;

         case TK_URL:
            m_children.append(new DocURL(this, g_token->name, g_token->isEMailAddr));
            break;

         case TK_WHITESPACE: {
            // prevent leading whitespace and collapse multiple whitespace areas
            DocNode::Kind k;

            if (insidePRE(this) ||
               // all whitespace is relevant

                  (  // remove leading whitespace
                     ! m_children.isEmpty()  &&

                     // and whitespace after certain constructs
                     (k = m_children.last()->kind()) != DocNode::Kind_HtmlDescList &&
                     k != DocNode::Kind_HtmlTable &&
                     k != DocNode::Kind_HtmlList &&
                     k != DocNode::Kind_SimpleSect &&
                     k != DocNode::Kind_AutoList &&
                     k != DocNode::Kind_SimpleList &&
                     /*k!=DocNode::Kind_Verbatim &&*/
                     k != DocNode::Kind_HtmlHeader &&
                     k != DocNode::Kind_HtmlBlockQuote &&
                     k != DocNode::Kind_ParamSect &&
                     k != DocNode::Kind_XRefItem)) {
               m_children.append(new DocWhiteSpace(this, g_token->chars));
            }
         }
         break;

         case TK_LISTITEM: {
            DBG(("Found list item at %d parent = %d\n", g_token->indent, parent()->kind()));

            DocNode *n = parent();
            while (n && n->kind() != DocNode::Kind_AutoList) {
               n = n->parent();
            }

            if (n) { // we found an auto list up in the hierarchy
               DocAutoList *al = (DocAutoList *)n;
               DBG(("previous list item at %d\n", al->indent()));

               if (al->indent() >= g_token->indent) {
                  // new item at the same or lower indent level

                  retval = TK_LISTITEM;
                  goto endparagraph;
               }
            }

            // determine list depth
            int depth = 0;
            n = parent();

            while (n) {
               if (n->kind() == DocNode::Kind_AutoList && ((DocAutoList *)n)->isEnumList()) {
                  depth++;
               }

               n = n->parent();
            }

            // first item or sub list => create new list
            DocAutoList *al = 0;

            do {
               al = new DocAutoList(this, g_token->indent, g_token->isEnumList, depth);
               m_children.append(al);
               retval = al->parse();

            } while (retval == TK_LISTITEM &&           // new list
                     al->indent() == g_token->indent    // at same indent level
                    );

            // check the return value
            if (retval == RetVal_SimpleSec) { // auto list ended due to simple section command
               // Reparse the token that ended the section at this level,
               // so a new simple section will be started at this level.
               // This is the same as unputting the last read token and continuing.

               g_token->name = g_token->simpleSectName;

               if (g_token->name.startsWith("rcs:")) {
                  // RCS section
                  g_token->name = g_token->name.mid(4);
                  g_token->text = g_token->simpleSectText;
                  tok = TK_RCSTAG;

               } else {
                  // other section
                  tok = TK_COMMAND_BS;

               }

               DBG(("reparsing command %s\n", csPrintable(g_token->name)));

               goto reparsetoken;

            } else if (retval == TK_ENDLIST) {
               if (al->indent() > g_token->indent) {
                  // end list
                  goto endparagraph;

               } else {
                  // continue with current paragraph

               }

            } else { // paragraph ended due to TK_NEWPARA, TK_LISTITEM, or EOF
               goto endparagraph;
            }
         }
         break;

         case TK_ENDLIST:
            DBG(("Found end of list inside of paragraph at line %d\n", doctokenizerYYlineno));

            if (parent()->kind() == DocNode::Kind_AutoListItem) {
               assert(parent()->parent()->kind() == DocNode::Kind_AutoList);
               DocAutoList *al = (DocAutoList *)parent()->parent();

               if (al->indent() >= g_token->indent) {
                  // end of list marker ends this paragraph
                  retval = TK_ENDLIST;
                  goto endparagraph;

               } else {
                  warn_doc_error(s_fileName, doctokenizerYYlineno, "End of list marker found with an invalid indent level");
               }

            } else {
               warn_doc_error(s_fileName, doctokenizerYYlineno, "End of list marker found without any preceding list items");
            }
            break;

         case TK_COMMAND_AT:
         case TK_COMMAND_BS: {
            // see if we have to start a simple section
            int cmd = Mappers::cmdMapper->map(g_token->name);
            DocNode *n = parent();

            while (n && n->kind() != DocNode::Kind_SimpleSect && n->kind() != DocNode::Kind_ParamSect) {
               n = n->parent();
            }

            if (cmd & SIMPLESECT_BIT) {

               if (n) {
                  // already in a simple section
                  // simple section can not start in this paragraph, need to unwind the stack and remember the command

                  g_token->simpleSectName = g_token->name;
                  retval = RetVal_SimpleSec;
                  goto endparagraph;
               }
            }

            // see if we are in a simple list
            n = parent();

            while (n && n->kind() != DocNode::Kind_SimpleListItem) {
               n = n->parent();
            }

            if (n) {
               if (cmd == CMD_LI) {
                  retval = RetVal_ListItem;
                  goto endparagraph;
               }
            }

            // handle the command
            retval = handleCommand(g_token->name);

            DBG(("handleCommand returns %x\n", retval));

            // check the return value
            if (retval == RetVal_SimpleSec) {
               // Reparse the token which ended the section at this level
               // so a new simple section will be started at this level.
               // This is the same as unputting the last read token and continuing.

               g_token->name = g_token->simpleSectName;

               if (g_token->name.startsWith("rcs:")) {
                  // RCS section
                  g_token->name = g_token->name.mid(4);
                  g_token->text = g_token->simpleSectText;
                  tok = TK_RCSTAG;

               } else {
                  // other section
                  tok = TK_COMMAND_BS;
               }

               DBG(("reparsing command %s\n", csPrintable(g_token->name)));
               goto reparsetoken;

            } else if (retval == RetVal_OK) {
               // the command ended normally, keep scanning for new tokens
               retval = 0;

            } else if (retval > 0 && retval < RetVal_OK) {
               // the command ended with a new command, reparse this token
               tok = retval;
               goto reparsetoken;

            } else {
               // end of file, end of paragraph, start or end of section
               // or some auto list marker

               goto endparagraph;
            }
         }
         break;

         case TK_HTMLTAG: {

            if (! g_token->endTag) {
               // found a start tag
               retval = handleHtmlStartTag(g_token->name, g_token->attribs);

            } else {
               // found an end tag

               if (g_token->name == "div"  && s_styleStack.isEmpty() )  {
                  // let DocRoot add the closing <div>
                  retval = RetVal_EndDiv;

               } else {
                  retval = handleHtmlEndTag(g_token->name);

               }

            }

            if (retval == RetVal_OK) {
               // the command ended normally, keep scanner for new tokens
               retval = 0;

            } else {
               goto endparagraph;
            }
         }
         break;

         case TK_SYMBOL: {
            DocSymbol::SymType s = DocSymbol::decodeSymbol(g_token->name);

            if (s != DocSymbol::Sym_Unknown) {
               m_children.append(new DocSymbol(this, s));

            } else {
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unsupported symbol %s found", csPrintable(g_token->name));
            }
            break;
         }

         case TK_NEWPARA:
            retval = TK_NEWPARA;
            goto endparagraph;

         case TK_RCSTAG: {
            DocNode *n = parent();

            while (n && n->kind() != DocNode::Kind_SimpleSect && n->kind() != DocNode::Kind_ParamSect) {
               n = n->parent();
            }

            if (n) {
               // already in a simple section
               // simple section cannot start in this paragraph, need
               // to unwind the stack and remember the command

               g_token->simpleSectName = "rcs:" + g_token->name;
               g_token->simpleSectText = g_token->text;
               retval = RetVal_SimpleSec;
               goto endparagraph;
            }

            // see if we are in a simple list
            DocSimpleSect *ss = new DocSimpleSect(this, DocSimpleSect::Rcs);
            m_children.append(ss);
            ss->parseRcs();
         }
         break;

         default:
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Found unexpected token (id=%x)\n", tok);
            break;
      }
   }

   retval = 0;

endparagraph:
   handlePendingStyleCommands(this, m_children);

   if (! s_nodeStack.isEmpty() ) {
      DocNode *n = s_nodeStack.pop();
      (void) n;

      assert(n == this);
   }

   DBG(("DocPara::parse() end retval=%x\n", retval));

   INTERNAL_ASSERT(retval == 0 || retval == TK_NEWPARA || retval == TK_LISTITEM ||
                   retval == TK_ENDLIST || retval > RetVal_OK );

   return retval;
}

int DocSection::parse()
{
   DBG(("DocSection::parse() start %s level=%d\n", csPrintable(g_token->sectionId), m_level));
   s_nodeStack.push(this);

   int retval = RetVal_OK;
   QSharedPointer<SectionInfo> sec;

   if (! m_id.isEmpty()) {
      sec = Doxy_Globals::sectionDict.find(m_id);

      if (sec) {
         m_file   = sec->fileName;
         m_anchor = sec->label;
         m_title  = sec->title;

         if (m_title.isEmpty()) {
            m_title = sec->label;
         }

         if (s_sectionDict && s_sectionDict->find(m_id) == 0) {
            s_sectionDict->insert(m_id, sec);
         }
      }
   }

   // first parse any number of paragraphs
   bool isFirst = true;
   DocPara *lastPar = 0;

   do {
      DocPara *par = new DocPara(this);

      if (isFirst) {
         par->markFirst();
         isFirst = false;
      }

      retval = par->parse();

      if (! par->isEmpty()) {
         m_children.append(par);
         lastPar = par;

      } else {
         delete par;
      }

      if (retval == TK_LISTITEM) {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Invalid list item found");
      }

      if (retval == RetVal_Internal) {
         DocInternal *in = new DocInternal(this);
         m_children.append(in);

         retval = in->parse(m_level + 1);

         if (retval == RetVal_EndInternal) {
            retval = RetVal_OK;
         }
      }

   } while (retval != 0 &&
            retval != RetVal_Section       &&
            retval != RetVal_Subsection    &&
            retval != RetVal_Subsubsection &&
            retval != RetVal_Paragraph     &&
            retval != RetVal_EndInternal
           );

   if (lastPar) {
      lastPar->markLast();
   }

   if (retval == RetVal_Subsection && m_level == Doxy_Globals::subpageNestingLevel + 1) {
      // then parse any number of nested sections

      while (retval == RetVal_Subsection) { // more sections follow
         // SectionInfo *sec = Doxy_Globals::sectionDict[g_token->sectionId];

         DocSection *s = new DocSection(this, qMin(2 + Doxy_Globals::subpageNestingLevel, 5), g_token->sectionId);
         m_children.append(s);
         retval = s->parse();
      }

   } else if (retval == RetVal_Subsubsection && m_level == Doxy_Globals::subpageNestingLevel + 2) {
      // then parse any number of nested sections

      while (retval == RetVal_Subsubsection) { // more sections follow
         // SectionInfo *sec = Doxy_Globals::sectionDict[g_token->sectionId];

         DocSection *s = new DocSection(this, qMin(3 + Doxy_Globals::subpageNestingLevel, 5), g_token->sectionId);
         m_children.append(s);
         retval = s->parse();
      }

   } else if (retval == RetVal_Paragraph && m_level == qMin(5, Doxy_Globals::subpageNestingLevel + 3)) {
      // then parse any number of nested sections

      while (retval == RetVal_Paragraph) { // more sections follow
         // SectionInfo *sec = Doxy_Globals::sectionDict[g_token->sectionId];

         DocSection *s = new DocSection(this, qMin(4 + Doxy_Globals::subpageNestingLevel, 5), g_token->sectionId);
         m_children.append(s);
         retval = s->parse();
      }

   } else if ((m_level <= 1 + Doxy_Globals::subpageNestingLevel && retval == RetVal_Subsubsection) ||
              (m_level <= 2 + Doxy_Globals::subpageNestingLevel && retval == RetVal_Paragraph)
             ) {
      int level = (retval == RetVal_Subsubsection) ? 3 : 4;

      warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected %s command found inside %s",
                     csPrintable(sectionLevelToName[level]), csPrintable(sectionLevelToName[m_level]) );

      retval = 0; // stop parsing
   }

   INTERNAL_ASSERT(retval == 0 ||
                   retval == RetVal_Section ||
                   retval == RetVal_Subsection ||
                   retval == RetVal_Subsubsection ||
                   retval == RetVal_Paragraph ||
                   retval == RetVal_Internal ||
                   retval == RetVal_EndInternal
                  );

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocSection::parse() end: retval=%x\n", retval));

   return retval;
}

void DocText::parse()
{
   DBG(("DocText::parse() start\n"));

   s_nodeStack.push(this);
   doctokenizerYYsetStateText();

   int tok;

   while ((tok = doctokenizerYYlex())) {
      // get the next token

      switch (tok) {
         case TK_WORD:
            m_children.append(new DocWord(this, g_token->name));
            break;

         case TK_WHITESPACE:
            m_children.append(new DocWhiteSpace(this, g_token->chars));
            break;

         case TK_SYMBOL: {
            DocSymbol::SymType s = DocSymbol::decodeSymbol(g_token->name);

            if (s != DocSymbol::Sym_Unknown) {
               m_children.append(new DocSymbol(this, s));
            } else {
               warn_doc_error(s_fileName, doctokenizerYYlineno, "Unsupported symbol %s found", csPrintable(g_token->name));
            }
         }
         break;

         case TK_COMMAND_AT:
         case TK_COMMAND_BS:

            switch (Mappers::cmdMapper->map(g_token->name)) {
               case CMD_BSLASH:
                  m_children.append(new DocSymbol(this, DocSymbol::Sym_BSlash));
                  break;

               case CMD_AT:
                  m_children.append(new DocSymbol(this, DocSymbol::Sym_At));
                  break;

               case CMD_LESS:
                  m_children.append(new DocSymbol(this, DocSymbol::Sym_Less));
                  break;

               case CMD_GREATER:
                  m_children.append(new DocSymbol(this, DocSymbol::Sym_Greater));
                  break;

               case CMD_AMP:
                  m_children.append(new DocSymbol(this, DocSymbol::Sym_Amp));
                  break;

               case CMD_DOLLAR:
                  m_children.append(new DocSymbol(this, DocSymbol::Sym_Dollar));
                  break;

               case CMD_HASH:
                  m_children.append(new DocSymbol(this, DocSymbol::Sym_Hash));
                  break;

               case CMD_DCOLON:
                  m_children.append(new DocSymbol(this, DocSymbol::Sym_DoubleColon));
                  break;

               case CMD_PERCENT:
                  m_children.append(new DocSymbol(this, DocSymbol::Sym_Percent));
                  break;

               case CMD_NDASH:
                  m_children.append(new DocSymbol(this, DocSymbol::Sym_Minus));
                  m_children.append(new DocSymbol(this, DocSymbol::Sym_Minus));
                  break;

               case CMD_MDASH:
                  m_children.append(new DocSymbol(this, DocSymbol::Sym_Minus));
                  m_children.append(new DocSymbol(this, DocSymbol::Sym_Minus));
                  m_children.append(new DocSymbol(this, DocSymbol::Sym_Minus));
                  break;

               case CMD_QUOTE:
                  m_children.append(new DocSymbol(this, DocSymbol::Sym_Quot));
                  break;

               case CMD_PUNT:
                  m_children.append(new DocSymbol(this,DocSymbol::Sym_Dot));
                  break;

               case CMD_PLUS:
                  m_children.append(new DocSymbol(this,DocSymbol::Sym_Plus));
                  break;

               case CMD_MINUS:
                  m_children.append(new DocSymbol(this,DocSymbol::Sym_Minus));
                  break;

               case CMD_EQUAL:
                  m_children.append(new DocSymbol(this,DocSymbol::Sym_Equal));
                  break;

               default:
                  warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected command '%s' found", csPrintable(g_token->name));
                  break;
            }
            break;

         default:
            warn_doc_error(s_fileName, doctokenizerYYlineno, "Unexpected token %s", csPrintable(tokToString(tok)));
            break;
      }
   }

   handleUnclosedStyleCommands();

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocText::parse() end\n"));
}

void DocRoot::parse()
{
   DBG(("DocRoot::parse() start\n"));

   s_nodeStack.push(this);
   doctokenizerYYsetStatePara();

   int retval = 0;

   // first parse any number of paragraphs
   bool isFirst = true;
   DocPara *lastPar = 0;

   do {
      bool divFound = false;
      int tok = doctokenizerYYlex();

      if (tok == TK_HTMLTAG) {
         int tagId = Mappers::htmlTagMapper->map(g_token->name);

         if (tagId == HTML_DIV)   {
            retval   = -1;
            divFound = true;

            DocStyleChange *sc;

            if (! g_token->endTag) {
               sc = new DocStyleChange(this, s_nodeStack.count(), DocStyleChange::Div, true, g_token->attribs);
            } else {
               sc = new DocStyleChange(this, s_nodeStack.count(), DocStyleChange::Div, false);
            }

            m_children.append(sc);
         }
      }

      if (! divFound) {
         DocPara *par = new DocPara(this);

         if (isFirst) {
            par->markFirst();
            isFirst = false;
         }

         retval = par->parse(true, tok);

         // test for a <div>
         if (! par->isEmpty()) {
            m_children.append(par);
            lastPar = par;

            if (retval == RetVal_EndDiv) {
               // time to add the closing div

               DocStyleChange *sc = new DocStyleChange(this, s_nodeStack.count(), DocStyleChange::Div, false);
               m_children.append(sc);
            }

         } else {
            delete par;

         }
      }

      if (retval == TK_LISTITEM) {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Invalid list item found");

      } else if (retval == RetVal_Subsection) {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "found subsection command outside of section context");

      } else if (retval == RetVal_Subsubsection) {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "found subsubsection command outside of subsection context");

      } else if (retval == RetVal_Paragraph) {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "found paragraph command outside of subsubsection context");
      }

      if (retval == RetVal_Internal) {
         DocInternal *in = new DocInternal(this);
         m_children.append(in);
         retval = in->parse(1);
      }

   } while (retval != 0 && retval != RetVal_Section);

   if (lastPar) {
      lastPar->markLast();
   }

   // then parse any number of level1 sections
   while (retval == RetVal_Section) {
      QSharedPointer<SectionInfo> sec = Doxy_Globals::sectionDict.find(g_token->sectionId);

      if (sec) {
         DocSection *s = new DocSection(this, qMin(1 + Doxy_Globals::subpageNestingLevel, 5), g_token->sectionId);
         m_children.append(s);
         retval = s->parse();

      } else {
         warn_doc_error(s_fileName, doctokenizerYYlineno, "Invalid section id '%s', ignoring section",
                  csPrintable(g_token->sectionId));

         retval = 0;
      }
   }

   handleUnclosedStyleCommands();

   DocNode *n = s_nodeStack.pop();
   (void) n;

   assert(n == this);
   DBG(("DocRoot::parse() end\n"));
}

static QString extractCopyDocId(const QString &data, QString::const_iterator &iter_j, QString::const_iterator iter_end)
{
   QString::const_iterator iter_s = iter_j;
   QString::const_iterator iter_e = iter_j;

   int round = 0;

   bool insideDQuote = false;
   bool insideSQuote = false;
   bool found        = false;

   while (iter_j < iter_end && ! found) {
      QChar c = *iter_j;

      if (! insideSQuote && !insideDQuote) {

         switch (c.unicode()) {
            case '(':
               round++;
               break;

            case ')':
               round--;
               break;

            case '"':
               insideDQuote = true;
               break;

            case '\'':
               insideSQuote = true;
               break;

            case ' ':
            case '\t':
            case '\n':
               found = (round == 0);
               break;
         }

      } else if (insideSQuote) {
         // look for single quote end

         if (c == '\'' && (iter_j == data.constBegin() || iter_j[-1] != '\\')) {
            insideSQuote = false;
         }

      } else if (insideDQuote) {
         // look for double quote end

         if (c == '"' && (iter_j == data.constBegin() || iter_j[-1] != '\\')) {
            insideDQuote = false;
         }
      }

      if (! found) {
         ++iter_j;
      }
   }

   QStringView tmp = QStringView(iter_j, data.constEnd());

   if (tmp.startsWith(" const") ) {
      iter_j += 6;

   } else if (tmp.startsWith(" volatile") ) {
      iter_j += 9;
   }

   iter_e = iter_j;

   QString id;

   if (iter_e > iter_s) {
      id = QStringView(iter_s, iter_e);
   }

   return id;
}

static  QString::const_iterator isCopyBriefOrDetailsCmd(const QString &data, QString::const_iterator iter_i, bool &brief)
{
   QString::const_iterator iter_retval = data.constBegin();

   if (iter_i == data.constBegin() || (iter_i[-1] != '@' && iter_i[-1] != '\\')) {
      // not an escaped command
      QStringView tmp = QStringView(iter_i + 1, data.constEnd());

      if (tmp.startsWith("copybrief") ) {
         // @copybrief or \copybrief

         iter_retval = iter_i + 10;
         brief = true;

      } else if (tmp.startsWith("copydetails") ) {
         // @copydetails or \copydetails

         iter_retval = iter_i + 12;
         brief = false;
      }
   }

   return iter_retval;
}

static QString::const_iterator isVerbatimSection(const QString &data, QString::const_iterator iter_i, QString &endMarker)
{
   QString::const_iterator iter_retval = data.constEnd();

   if (iter_i == data.constBegin() || (iter_i[-1] != '@' && iter_i[-1] != '\\')) {
      // not an escaped command
      QStringView tmp = QStringView(iter_i + 1, data.constEnd());

      if (tmp.startsWith("dot") ) {
         iter_retval = iter_i + 4;
         endMarker   = "enddot";

      } else if (tmp.startsWith("code") ) {
         iter_retval = iter_i + 5;
         endMarker   = "endcode";

      } else if (tmp.startsWith("msc") ) {
         iter_retval = iter_i + 4;
         endMarker   = "endmsc";

      } else if (tmp.startsWith("startuml") ) {
         iter_retval = iter_i + 9;
         endMarker   = "enduml";

      } else if (tmp.startsWith("verbatim") ) {
         iter_retval = iter_i + 9;
         endMarker   = "endverbatim";

      } else if (tmp.startsWith("htmlonly") ) {
         iter_retval = iter_i + 9;
         endMarker   = "endhtmlonly";

      } else if (tmp.startsWith("latexonly") ) {
         iter_retval = iter_i + 10;
         endMarker   = "endlatexonly";

      } else if (tmp.startsWith("xmlonly") ) {
         iter_retval = iter_i + 8;
         endMarker   = "endxmlonly";

      } else if (tmp.startsWith("rtfonly") ) {
         iter_retval = iter_i + 8;
         endMarker   = "endrtlonly";

      } else if (tmp.startsWith("manonly") ) {
         iter_retval = iter_i + 8;
         endMarker   = "endmanonly";

      } else if (tmp.startsWith("docbookonly") ) {
         iter_retval = iter_i + 12;
         endMarker   = "enddocbookonly";

      }
   }

   return iter_retval;
}


static QString::const_iterator skipToEndMarker(QStringView data, QString::const_iterator iter_begin,
                  QString::const_iterator iter_end, const QString &endMarker)
{
   QString::const_iterator iter = iter_begin;
   QChar c_prev = '\0';

   while (iter != iter_end) {
      QChar c = *iter;

      if ((c == '@' || c == '\\') && (iter == data.begin() || (c_prev !='@' && c_prev != '\\')))  {

         if (QStringView(iter + 1, iter_end).startsWith(endMarker)) {
            return iter + endMarker.length() + 1;
         }
      }

      c_prev = c;
      ++iter;
   }

   // no endmarker found
   return iter != iter_end ? iter + 1 : iter_end;
}

static QString processCopyDoc(const QString &data, uint &len)
{
   QString retval;

   QString::const_iterator iter_i   = data.constBegin();
   QString::const_iterator iter_end = data.constBegin() + len;

   while (iter_i < iter_end) {
      QChar c = *iter_i;

      if (c == '@' || c == '\\') {
         // look for a command
         bool isBrief = true;

         QString::const_iterator iter_j = isCopyBriefOrDetailsCmd(data, iter_i, isBrief);

         if (iter_j > data.constBegin()) {
            // skip whitespace
            QChar ch = *iter_j;

            while (iter_j < iter_end && (ch == ' ' || ch == '\t')) {
               ++iter_j;

               ch = *iter_j;
            }

            // extract the argument
            QString id = extractCopyDocId(data, iter_j, iter_end);

            QString doc;
            QString brief;

            QSharedPointer<Definition> def = findDocsForMemberOrCompound(id, doc, brief);

            if (def) {
               if (s_copyStack.indexOf(def) == -1) {
                  // definition not parsed earlier
                  s_copyStack.append(def);

                  if (isBrief) {
                     uint briefLen = brief.length();
                     retval += processCopyDoc(brief, briefLen);

                  } else {
                     uint docLen = doc.length();
                     retval += processCopyDoc(doc, docLen);
                  }

                  s_copyStack.removeOne(def);

               } else {
                  warn_doc_error(s_fileName, doctokenizerYYlineno,
                                 "Found recursive @copy%s or @copydoc relation for argument '%s'.\n",
                                 isBrief ? "brief" : "details", csPrintable(id) );
               }

            } else {
               warn_doc_error(s_fileName, doctokenizerYYlineno,
                  "@copy%s or @copydoc target '%s' not found", isBrief ? "brief" : "details", csPrintable(id) );
            }

            // skip over command
            iter_i = iter_j;

         } else {
            QString endMarker;
            QString::const_iterator it_b = isVerbatimSection(data, iter_i, endMarker);

            if (it_b != iter_end) {
               auto iter_tmp = skipToEndMarker(data, it_b, iter_end, endMarker);
               retval += QStringView(iter_i, iter_tmp);
               iter_i = iter_tmp;

            } else {
               retval += c;
               ++iter_i;
            }
         }

      } else {
         // not a command, just copy
         retval += c;
         ++iter_i;
      }
   }

   len = retval.length();

   return retval;
}

Text::Direction textDirection(const QString &text)
{
   (void) text;
   // static const QString configDir = Config_getEnum("text_direction");

   // emerald, rtl
   static const QString configDir = "None";

   if (configDir == "None") {
      return Text::Direction::DirNeutral;
   }

   if (configDir == "Context") {
      // return text.basicDirection();
      return Text::Direction::DirNeutral;
   }

   if (configDir == "LTR") {
      // Text::Direction textDir = text.direction();
      Text::Direction textDir = Text::Direction::DirNeutral;

      if (textDir == Text::Direction::DirMixed) {
         return Text::Direction::DirLTR;
      }

      return textDir;
   }

   if (configDir == "RTL") {
      // Text::Direction textDir = text.direction();
      Text::Direction textDir = Text::Direction::DirNeutral;

      if (textDir == Text::Direction::DirMixed) {
         return Text::Direction::DirRTL;
      }

      return textDir;
   }

   return Text::Direction::DirNeutral;
}

Text::Direction textDirection(const DocNode *node)
{
   (void) node;
   // static const QString configDir = Config_getEnum("text_direction");

   // emerald, rtl
   static const QString configDir = "None";

   if (configDir == "None") {
      return Text::Direction::DirNeutral;
   }

   if (configDir == "Context") {
      // return node->getTextBasicDir();
      return Text::Direction::DirNeutral;
   }

   if (configDir == "LTR") {
      // Text::Direction::Direction textDir = node->getTextDir();
      Text::Direction textDir = Text::Direction::DirNeutral;

      if (textDir == Text::Direction::DirMixed) {
         return Text::Direction::DirLTR;
      }

      return textDir;
   }

   if (configDir == "RTL") {
      // Text::Direction::Direction textDir = node->getTextDir();
      Text::Direction textDir = Text::Direction::DirNeutral;

      if (textDir == Text::Direction::DirMixed) {
         return Text::Direction::DirRTL;
      }

      return textDir;
   }

   return Text::Direction::DirNeutral;
}

Text::Direction textDirection(const DocPara *para, int nodeIndex)
{
   (void) para;
   (void) nodeIndex;
   // static const QString configDir = Config_getEnum("text_direction");

   // emerald, rtl
   static const QString configDir = "None";

   if (configDir == "None") {
      return Text::Direction::DirNeutral;
   }

   if (configDir == "Context") {
      // return para->getTextBasicDir(nodeIndex);
      return Text::Direction::DirNeutral;
   }

   if (configDir == "LTR") {
      // Text::Direction textDir = para->getTextDir(nodeIndex);
      Text::Direction textDir = Text::Direction::DirNeutral;

      if (textDir == Text::Direction::DirMixed) {
         return Text::Direction::DirLTR;
      }

      return textDir;
   }

   if (configDir == "RTL") {
      // Text::Direction textDir = para->getTextDir(nodeIndex);
      Text::Direction textDir = Text::Direction::DirNeutral;

      if (textDir == Text::Direction::DirMixed) {
         return Text::Direction::DirRTL;
      }

      return textDir;
   }

   return Text::Direction::DirNeutral;
}

QString getDirHtmlClassOfNode(Text::Direction textDir, const QString &initValue)
{
   QString retval;

   if (textDir == Text::Direction::DirLTR) {
      retval = "DocNodeLTR";

   } else if (textDir == Text::Direction::DirRTL) {
      retval = "DocNodeRTL";

   }

   if (! initValue.isEmpty() && ! retval.isEmpty()) {
      retval = " class=\"" + initValue + " " + retval + "\"";

   } else  if (! initValue.isEmpty()) {
      retval = " class=\"" + initValue + "\"";

   } else if (! retval.isEmpty()) {
      retval = " class=\"" + retval + "\"";
   }

   return retval;
}

QString getDirHtmlClassOfPage(QString pageTitle)
{
   QString retval = " class=\"PageDoc";

   auto titleDir = textDirection(pageTitle);

   if (titleDir == Text::Direction::DirLTR) {
      retval += " PageDocLTR-title";

   } else if (titleDir == Text::Direction::DirRTL) {
      retval += " PageDocRTL-title";

   }

   retval += "\"";

   return retval;
}

QString getHtmlDirEmbeddingChar(Text::Direction textDir)
{
   QString retval;

   if (textDir == Text::DirLTR) {
      retval = "&#x202A;";

   } else if (textDir == Text::DirRTL) {
      retval = "&#x202B;";
   }

   return retval;
}

// main entry point
DocRoot *validatingParseDoc(const QString &fileName, int startLine, QSharedPointer<Definition> ctx,
                  QSharedPointer<MemberDef> md, const QString &input, bool indexWords, bool isExample,
                  const QString &exampleName, bool singleLine, bool linkFromIndex)
{
   // store parser state so we can re-enter this function if needed

   // bool fortranOpt = Config::getBool("optimize-fortran");
   docParserPushContext();

   if (ctx && ctx != Doxy_Globals::globalScope && (ctx->definitionType() == Definition::TypeClass ||
          ctx->definitionType() == Definition::TypeNamespace)) {

      s_context = ctx->name();

   } else if (ctx && ctx->definitionType() == Definition::TypePage) {
      QSharedPointer<Definition> scope = ctx.dynamicCast<PageDef>()->getPageScope();

      if (scope && scope != Doxy_Globals::globalScope) {
         s_context = scope->name();
      }

   } else if (ctx && ctx->definitionType() == Definition::TypeGroup) {
      QSharedPointer<Definition> scope = ctx.dynamicCast<GroupDef>()->getGroupScope();

      if (scope && scope != Doxy_Globals::globalScope) {
         s_context = scope->name();
      }

   } else {
      s_context = "";
   }

   s_scope = ctx;

   if (indexWords && Doxy_Globals::searchIndexBase != nullptr) {
      if (md) {
         s_searchUrl = md->getOutputFileBase();
         Doxy_Globals::searchIndexBase->setCurrentDoc(md, md->anchor(), false);

      } else if (ctx) {
         s_searchUrl = ctx->getOutputFileBase();
         Doxy_Globals::searchIndexBase->setCurrentDoc(ctx, ctx->anchor(), false);

      }

   } else {
      s_searchUrl = "";
   }

   s_fileName = fileName;
   s_relPath  = (! linkFromIndex && ctx) ? QString(relativePathToRoot(ctx->getOutputFileBase())) : "";

   s_memberDef = md;

   s_nodeStack.clear();
   s_styleStack.clear();
   s_initialStyleStack.clear();

   s_inSeeBlock        = false;
   s_xmlComment        = false;
   s_insideHtmlLink    = false;
   s_includeFileText   = "";
   s_includeFileOffset = 0;
   s_includeFileLength = 0;
   s_isExample         = isExample;
   s_exampleName       = exampleName;

   s_hasParamCommand   = false;
   s_hasReturnCommand  = false;

   s_paramsFound.clear();
   s_sectionDict = 0;

   setDoctokenLineNum(startLine);
   uint inputLen = input.length();

   QString tmpData = processCopyDoc(input, inputLen);
   if (tmpData.isEmpty() || ! tmpData.endsWith('\n')) {
      tmpData += '\n';
   }

   doctokenizerYYinit(tmpData, s_fileName);

   // build abstract syntax tree
   DocRoot *root = new DocRoot(md != nullptr, singleLine);
   root->parse();

   if (Debug::isFlagSet(Debug::PrintTree)) {
      // pretty print the result

      PrintDocVisitor *v = new PrintDocVisitor;
      root->accept(v);
      delete v;
   }

   checkUndocumentedParams();
   detectUndocumentedParams();

   // TODO: should be called at the end of the program
   // doctokenizerYYcleanup();
   // Mappers::cmdMapper->freeInstance();
   // Mappers::htmlTagMapper->freeInstance();

   // restore original parser state
   docParserPopContext();

   return root;
}

DocText *validatingParseText(const QString &input)
{
   // store parser state so we can re-enter this function if needed
   docParserPushContext();

   s_context  = "";
   s_fileName = "<parseText>";
   s_relPath  = "";

   s_memberDef = QSharedPointer<MemberDef>();

   s_nodeStack.clear();
   s_styleStack.clear();
   s_initialStyleStack.clear();

   s_inSeeBlock        = false;
   s_xmlComment        = false;
   s_insideHtmlLink    = false;
   s_includeFileText   = "";
   s_includeFileOffset = 0;
   s_includeFileLength = 0;
   s_isExample         = false;
   s_exampleName       = "";
   s_hasParamCommand   = false;
   s_hasReturnCommand  = false;

   s_paramsFound.clear();
   s_searchUrl = "";

   DocText *txt = new DocText;

   if (! input.isEmpty()) {
      setDoctokenLineNum(1);
      doctokenizerYYinit(input, s_fileName);

      // build abstract syntax tree
      txt->parse();

      if (Debug::isFlagSet(Debug::PrintTree)) {
         // pretty print the result
         PrintDocVisitor *v = new PrintDocVisitor;
         txt->accept(v);
         delete v;
      }
   }

   // restore original parser state
   docParserPopContext();

   return txt;
}

void docFindSections(const QString &input, QSharedPointer<Definition> def,
                  QSharedPointer<MemberGroup> mg, const QString &fileName)
{
   doctokenizerYYFindSections(input, def, mg, fileName);
}
