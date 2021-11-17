/************************************************************************
*
* Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
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

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QHash>

#include <stdlib.h>

#include <xmlgen.h>

#include <arguments.h>
#include <config.h>
#include <docparser.h>
#include <doxy_build_info.h>
#include <doxy_globals.h>
#include <dot.h>
#include <default_args.h>
#include <htmlentity.h>
#include <language.h>
#include <message.h>
#include <membergroup.h>
#include <outputgen.h>
#include <parse_base.h>
#include <resourcemgr.h>
#include <util.h>
#include <xmldocvisitor.h>

// no debug info
#define XML_DB(x) do {} while(0)

// debug to stdout
//#define XML_DB(x) printf x

// debug inside output
//#define XML_DB(x) QByteArray __t;__t.sprintf x;m_t << __t

/** Helper class mapping MemberList::ListType to a string representing */
class XmlSectionMapper : public QHash<long, QString>
{
 public:
   XmlSectionMapper() : QHash<long, QString>() {
      insert(MemberListType_pubTypes,          "public-type");
      insert(MemberListType_pubMethods,        "public-func");
      insert(MemberListType_pubAttribs,        "public-attrib");
      insert(MemberListType_pubSlots,          "public-slot");
      insert(MemberListType_pubSignals,        "public-signal");
      insert(MemberListType_dcopMethods,       "dcop-func");
      insert(MemberListType_properties,        "property");
      insert(MemberListType_events,            "event");
      insert(MemberListType_interfaces,        "interfaces");
      insert(MemberListType_services,          "services");
      insert(MemberListType_pubStaticMethods,  "public-static-func");
      insert(MemberListType_pubStaticAttribs,  "public-static-attrib");
      insert(MemberListType_proTypes,          "protected-type");
      insert(MemberListType_proMethods,        "protected-func");
      insert(MemberListType_proAttribs,        "protected-attrib");
      insert(MemberListType_proSlots,          "protected-slot");
      insert(MemberListType_proSignals,        "protected-signal");
      insert(MemberListType_proStaticMethods,  "protected-static-func");
      insert(MemberListType_proStaticAttribs,  "protected-static-attrib");
      insert(MemberListType_pacTypes,          "package-type");
      insert(MemberListType_pacMethods,        "package-func");
      insert(MemberListType_pacAttribs,        "package-attrib");
      insert(MemberListType_pacStaticMethods,  "package-static-func");
      insert(MemberListType_pacStaticAttribs,  "package-static-attrib");
      insert(MemberListType_priTypes,          "private-type");
      insert(MemberListType_priMethods,        "private-func");
      insert(MemberListType_priAttribs,        "private-attrib");
      insert(MemberListType_priSlots,          "private-slot");
      insert(MemberListType_priSignals,        "private-signal");
      insert(MemberListType_priStaticMethods,  "private-static-func");
      insert(MemberListType_priStaticAttribs,  "private-static-attrib");
      insert(MemberListType_friends,           "friend");
      insert(MemberListType_related,           "related");
      insert(MemberListType_decDefineMembers,  "define");
      insert(MemberListType_decProtoMembers,   "prototype");
      insert(MemberListType_decTypedefMembers, "typedef");
      insert(MemberListType_decEnumMembers,    "enum");
      insert(MemberListType_decFuncMembers,    "func");
      insert(MemberListType_decVarMembers,     "var");
   }
};
static XmlSectionMapper g_xmlSectionMapper;

inline void writeXMLString(QTextStream &t, const QString &text)
{
   t << convertToXML(text);
}

inline void writeXMLCodeString(QTextStream &t, const QString &text, int &col)
{
   static const int tabSize = Config::getInt("tab-size");

   for (auto c : text) {

      switch (c.unicode()) {

         case '\t': {
            int spacesToNextTabStop = tabSize - (col % tabSize);
            col += spacesToNextTabStop;

            while (spacesToNextTabStop--) {
               t << "<sp/>";
            }
            break;
         }

         case ' ':
            t << "<sp/>";
            col++;
            break;

         case '<':
            t << "&lt;";
            col++;
            break;

         case '>':
            t << "&gt;";
            col++;
            break;

         case '&':
            t << "&amp;";
            col++;
            break;

         case '\'':
            t << "&apos;";
            col++;
            break;

         case '"':
            t << "&quot;";
            col++;
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
            // encode invalid XML characters (see http://www.w3.org/TR/2000/REC-xml-20001006#NT-Char)

            t << "<sp value=\"" << int(c.unicode()) << "\"/>";
            break;

         default:
            t << c;
            col++;

            break;
      }
   }
}

static void writeXMLHeader(QTextStream &t)
{
   t << "<?xml version='1.0' encoding='UTF-8' standalone='no'?>" << endl;;
   t << "<doxypress xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ";
   t << "xsi:noNamespaceSchemaLocation=\"compound.xsd\" ";
   t << "version=\""  << versionString << "\" ";
   t << "xml:lang=\"" << theTranslator->trLanguageId() << "\"";
   t << ">" << endl;
}

static void writeCombineScript()
{
   QString outputDirectory = Config::getString("xml-output");
   QString fileName = outputDirectory + "/combine.xslt";

   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);


   t <<
     "<!-- XSLT script to combine the generated output into a single file. \n"
     "     If you have xsltproc you could use:\n"
     "     xsltproc combine.xslt index.xml >all.xml\n"
     "-->\n"
     "<xsl:stylesheet xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\" version=\"1.0\">\n"
     "  <xsl:output method=\"xml\" version=\"1.0\" indent=\"no\" standalone=\"yes\" />\n"
     "  <xsl:template match=\"/\">\n"
     "    <doxypress version=\"{doxypressindex/@version}\" xml:lang=\"{doxypressindex/@xml:lang}\">\n"
     "      <!-- Load all doxypress generated xml files -->\n"
     "      <xsl:for-each select=\"doxypressindex/compound\">\n"
     "        <xsl:copy-of select=\"document( concat( @refid, '.xml' ) )/doxypress/*\" />\n"
     "      </xsl:for-each>\n"
     "    </doxypress>\n"
     "  </xsl:template>\n"
     "</xsl:stylesheet>\n";
}


void writeXMLLink(QTextStream &t, const QString &extRef, const QString &compoundId,
                  const QString &anchorId, const QString &text, const QString &tooltip)
{
   t << "<ref refid=\"" << compoundId;
   if (! anchorId.isEmpty()) {
      t << "_1" << anchorId;
   }

   t << "\" kindref=\"";
   if (! anchorId.isEmpty()) {
      t << "member";
   } else {
      t << "compound";
   }

   t << "\"";
   if (! extRef.isEmpty()) {
      t << " external=\"" << extRef << "\"";
   }

   if (! tooltip.isEmpty()) {
      t << " tooltip=\"" << convertToXML(tooltip) << "\"";
   }

   t << ">";
   writeXMLString(t, text);
   t << "</ref>";
}

/** Implements TextGenerator for an XML stream. */
class TextFragmentXml : public TextFragmentBase
{
 public:
   TextFragmentXml(QTextStream &t)
      : m_t(t)
   {
   }

   void writeString(const QString &text, bool) const override {
      writeXMLString(m_t, text);
   }

   void writeBreak(int) const override {
   }

   void writeLink(const QString &extRef, const QString &file, const QString &anchor, const QString &text) const override {
      writeXMLLink(m_t, extRef, file, anchor, text, QString());
   }

 private:
   QTextStream &m_t;
};

void XMLCodeGenerator::codify(const QString &text) {
   XML_DB(("(codify \"%s\")\n", text));

   if (m_insideCodeLine && ! m_insideSpecialHL && m_normalHLNeedStartTag) {
      m_t << "<highlight class=\"normal\">";
      m_normalHLNeedStartTag = false;
   }

   writeXMLCodeString(m_t, text, m_col);
}

void XMLCodeGenerator::writeCodeLink(const QString &ref, const QString &file, const QString &anchor,
                   const QString &name, const QString &tooltip) {

   XML_DB(("(writeCodeLink)\n"));

   if (m_insideCodeLine && ! m_insideSpecialHL && m_normalHLNeedStartTag) {
      m_t << "<highlight class=\"normal\">";
      m_normalHLNeedStartTag = false;
   }

   writeXMLLink(m_t, ref, file, anchor, name, tooltip);
   m_col += name.length();
}

void XMLCodeGenerator::writeTooltip(const QString &, const DocLinkInfo &, const QString &,
                  const QString &, const SourceLinkInfo &, const SourceLinkInfo &) {
   XML_DB(("(writeToolTip)\n"));
}

void XMLCodeGenerator::startCodeLine(bool) {
   XML_DB(("(startCodeLine)\n"));

   m_t << "<codeline";

   if (m_lineNumber != -1) {
      m_t << " lineno=\"" << m_lineNumber << "\"";

      if (! m_refId.isEmpty()) {
         m_t << " refid=\"" << m_refId << "\"";

         if (m_isMemberRef) {
            m_t << " refkind=\"member\"";
         } else {
            m_t << " refkind=\"compound\"";
         }
      }

      if (! m_external.isEmpty()) {
         m_t << " external=\"" << m_external << "\"";
      }
   }

   m_t << ">";
   m_insideCodeLine = true;
   m_col = 0;
}

void XMLCodeGenerator::endCodeLine() {
   XML_DB(("(endCodeLine)\n"));

   if (! m_insideSpecialHL && ! m_normalHLNeedStartTag) {
      m_t << "</highlight>";
      m_normalHLNeedStartTag = true;
   }

   m_t << "</codeline>" << endl;    // non DocBook

   m_lineNumber = -1;
   m_refId.resize(0);
   m_external.resize(0);
   m_insideCodeLine = false;
}

void XMLCodeGenerator::startFontClass(const QString &colorClass) {
   XML_DB(("(startFontClass)\n"));

   if (m_insideCodeLine && !m_insideSpecialHL && !m_normalHLNeedStartTag) {
      m_t << "</highlight>";
      m_normalHLNeedStartTag = true;
   }

   m_t << "<highlight class=\"" << colorClass << "\">"; // non DocBook
   m_insideSpecialHL = true;
}

void XMLCodeGenerator::endFontClass() {
   XML_DB(("(endFontClass)\n"));
   m_t << "</highlight>"; // non DocBook
   m_insideSpecialHL = false;
}

void XMLCodeGenerator::writeCodeAnchor(const QString &) {
   XML_DB(("(writeCodeAnchor)\n"));
}

void XMLCodeGenerator::writeLineNumber(const QString &extRef, const QString &compId, const QString &anchorId, int l) {
   XML_DB(("(writeLineNumber)\n"));

   // remember the information provided here to use it at the <codeline> start tag
   m_lineNumber = l;

   if (! compId.isEmpty()) {
      m_refId = compId;

      m_isMemberRef = (! anchorId.isEmpty());

      if (m_isMemberRef) {
         m_refId += "_1" + anchorId;
      }

      if (! extRef.isEmpty()) {
         m_external = extRef;
      }
   }
}

void XMLCodeGenerator::startCodeFragment(const QString &)  {
   m_t << "    <programlisting>" << endl;
}

void XMLCodeGenerator::endCodeFragment(const QString &)  {
   m_t << "    </programlisting>" << endl;
}

void XMLCodeGenerator::finish() {
      if (m_insideCodeLine) {
         endCodeLine();
      }
}

static void writeTemplateArgumentList(const ArgumentList &al, QTextStream &t, QSharedPointer<Definition> scope,
                  QSharedPointer<FileDef> fileScope, int indent)
{
   QString indentStr;
   indentStr.fill(' ', indent);

   if (! al.listEmpty()) {
      t << indentStr << "<templateparamlist>" << endl;

      for (const auto &arg : al) {
         t << indentStr << "  <param>" << endl;

         if (! arg.type.isEmpty()) {
            t << indentStr <<  "    <type>";
            linkifyText(TextFragmentXml(t), scope, fileScope, QSharedPointer<Definition>(), arg.type);
            t << "</type>" << endl;
         }

         if (! arg.name.isEmpty()) {
            t << indentStr <<  "    <declname>" << convertToXML(arg.name) << "</declname>" << endl;
            t << indentStr <<  "    <defname>"  << convertToXML(arg.name) << "</defname>" << endl;
         }

         if (! arg.defval.isEmpty()) {
            t << indentStr << "    <defval>";
            linkifyText(TextFragmentXml(t), scope, fileScope, QSharedPointer<Definition>(), arg.defval);
            t << "</defval>" << endl;
         }

         if (! arg.typeConstraint.isEmpty()) {
           t << indentStr << "    <typeconstraint>";
           linkifyText(TextFragmentXml(t), scope, fileScope, QSharedPointer<Definition>(), arg.typeConstraint);
           t << "</typeconstraint>" << endl;
         }

         t << indentStr << "  </param>" << endl;
      }

      t << indentStr << "</templateparamlist>" << endl;
   }
}

static void writeMemberTemplateLists(QSharedPointer<MemberDef> md, QTextStream &t)
{
   const ArgumentList &argList = md->getTemplateArgumentList();


   writeTemplateArgumentList(argList, t, md->getClassDef(), md->getFileDef(), 8);

}

static void writeTemplateList(QSharedPointer<ClassDef> cd, QTextStream &t)
{
   writeTemplateArgumentList(cd->getTemplateArgumentList(), t, cd, QSharedPointer<FileDef>(), 4);
}

static void writeXMLDocBlock(QTextStream &t, const QString &fileName, int lineNr,
                             QSharedPointer<Definition> scope, QSharedPointer<MemberDef> md, const QString &text)
{
   QString stext = text.trimmed();

   if (stext.isEmpty()) {
      return;
   }

   // convert the documentation string into an abstract syntax tree
   DocNode *root = validatingParseDoc(fileName, lineNr, scope, md, text, false, false);

   // create a code generator
   XMLCodeGenerator *xmlCodeGen = new XMLCodeGenerator(t);

   // create a parse tree visitor for XML
   XmlDocVisitor *visitor = new XmlDocVisitor(t, *xmlCodeGen);

   // visit all nodes
   root->accept(visitor);

   // clean up
   delete visitor;
   delete xmlCodeGen;
   delete root;
}

void writeXMLCodeBlock(QTextStream &t, QSharedPointer<FileDef> fd)
{
   ParserInterface *pIntf = Doxy_Globals::parserManager.getParser(fd->getDefFileExtension());
   SrcLangExt langExt = getLanguageFromFileName(fd->getDefFileExtension());
   pIntf->resetCodeParserState();

   QSharedPointer<XMLCodeGenerator> xmlGen = QMakeShared<XMLCodeGenerator>(t);
   xmlGen->startCodeFragment("DoxyCode");

   pIntf->parseCode(*xmlGen, QString(), fileToString(fd->getFilePath(), Config::getBool("filter-source-files")),
                    langExt, false, QString(), fd, -1, -1, false, QSharedPointer<MemberDef>(), true );

   xmlGen->endCodeFragment("DoxyCode");
   xmlGen->finish();
}

static void writeMemberReference(QTextStream &t, QSharedPointer<Definition> def,
            QSharedPointer<MemberDef> rmd, const QString &tagName)
{
   QString scope = rmd->getScopeString();
   QString name  = rmd->name();

   if (! scope.isEmpty() && scope != def->name()) {
      name.prepend(scope + getLanguageSpecificSeparator(rmd->getLanguage()));
   }

   t << "        <" << tagName << " refid=\"";
   t << rmd->getOutputFileBase() << "_1" << rmd->anchor() << "\"";

   if (rmd->getStartBodyLine() != -1 && rmd->getBodyDef()) {
      t << " compoundref=\"" << rmd->getBodyDef()->getOutputFileBase() << "\"";
      t << " startline=\"" << rmd->getStartBodyLine() << "\"";
      if (rmd->getEndBodyLine() != -1) {
         t << " endline=\"" << rmd->getEndBodyLine() << "\"";
      }
   }

   t << ">" << convertToXML(name) << "</" << tagName << ">" << endl;
}

static void stripQualifiers(QString &typeStr)
{
   bool done = false;

   while (! done) {

      if (typeStr.startsWith("static ")) {
         typeStr = stripPrefix(typeStr, "static ");

      } else if (typeStr.startsWith("virtual ")) {
         typeStr = stripPrefix(typeStr, "virtual ");

      } else if (typeStr.startsWith("volatile ")) {
         typeStr = stripPrefix(typeStr, "volatile ");

      } else if (typeStr == "virtual") {
         typeStr = "";

      } else {
         done = true;

      }
   }
}

static QString classOutputFileBase(QSharedPointer<ClassDef> cd)
{
   return cd->getOutputFileBase();
}

static QString memberOutputFileBase(QSharedPointer<MemberDef> md)
{
   return md->getOutputFileBase();
}

static void generateXMLForMember(QSharedPointer<MemberDef> md, QTextStream &ti, QTextStream &t, QSharedPointer<Definition> def)
{
   // + declaration/definition arg lists
   // + reimplements
   // + reimplementedBy
   // + exceptions
   // + const/volatile specifiers
   // - examples
   // + source definition
   // + source references
   // + source referenced by
   // - body code
   // + template arguments
   //     (templateArguments(), definitionTemplateParameterLists())
   // - call graph

   // enum values are written as part of the enum
   if (md->memberType() == MemberDefType::EnumValue) {
      return;
   }

   if (md->isHidden()) {
      return;
   }

   // group members are only visible in their group
   // if (def->definitionType()!=Definition::TypeGroup && md->getGroupDef()) return;

   QString memType;
   bool isFunc = false;

   switch (md->memberType()) {
      case MemberDefType::Define:
         memType = "define";
         break;

      case MemberDefType::Function:
         memType = "function";
         isFunc = true;
         break;

      case MemberDefType::Variable:
         memType = "variable";
         break;

      case MemberDefType::Typedef:
         memType = "typedef";
         break;

      case MemberDefType::Enumeration:
         memType = "enum";
         break;

      case MemberDefType::EnumValue:
         assert(0);
         break;

      case MemberDefType::Signal:
         memType = "signal";
         isFunc = true;
         break;

      case MemberDefType::Slot:
         memType = "slot";
         isFunc = true;
         break;

      case MemberDefType::DCOP:
         memType = "dcop";
         isFunc = true;
         break;

      case MemberDefType::Property:
         memType = "property";
         break;

      case MemberDefType::Event:
         memType = "event";
         break;

      case MemberDefType::Interface:
         memType = "interface";
         break;

      case MemberDefType::Service:
         memType = "service";
         break;
   }

   if (md->isFriend()) {
      memType.prepend("friend ");
   }

   ti << "    <member refid=\"" << memberOutputFileBase(md)
      << "_1" << md->anchor() << "\" kind=\"" << memType << "\"><name>"
      << convertToXML(md->name()) << "</name></member>" << endl;

   QString scopeName;
   if (md->getClassDef()) {
      scopeName = md->getClassDef()->name();

   } else if (md->getNamespaceDef()) {
      scopeName = md->getNamespaceDef()->name();
   }

   t << "      <memberdef kind=\"";
   //enum { define_t,variable_t,typedef_t,enum_t,function_t } xmlType = function_t;
   t << memType << "\" id=\"";

   if (md->getGroupDef() && def->definitionType() == Definition::TypeGroup) {
      t << md->getGroupDef()->getOutputFileBase();
   } else {
      t << memberOutputFileBase(md);
   }

   t << "_1"      // encoded `:' character (see util.cpp:convertNameToFile)
     << md->anchor();
   t << "\" prot=\"";

   switch (md->protection()) {
      case Public:
         t << "public";
         break;

      case Protected:
         t << "protected";
         break;

      case Private:
         t << "private";
         break;

      case Package:
         t << "package";
         break;
   }
   t << "\"";

   t << " static=\"";
   if (md->isStatic()) {
      t << "yes";
   } else {
      t << "no";
   }
   t << "\"";

   if (md->isConstExpr()) {
      t << " constexpr=\"yes\"";
   }

   if (isFunc) {
      const ArgumentList &al = md->getArgumentList();

      // **
      t << " const=\"";

      if (al.constSpecifier) {
         t << "yes";

      } else {
         t << "no";

      }
      t << "\"";

      // **
      t << " explicit=\"";

      if (md->isExplicit()) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";

      // **
      t << " inline=\"";

      if (md->isInline()) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";

      if (al.refSpecifier == RefType::LValueRef) {
         t << " refSpecifier=\"lvalue\"";

      } else if (al.refSpecifier == RefType::RValueRef) {
         t << " refSpecifier=\"rvalue\"";

      }

      // **
      if (md->isFinal()) {
         t << " final=\"yes\"";
      }

      if (md->isSealed()) {
         t << " sealed=\"yes\"";
      }

      if (md->isNew()) {
         t << " new=\"yes\"";
      }

      if (md->isOptional()) {
         t << " optional=\"yes\"";
      }

      if (md->isRequired()) {
         t << " required=\"yes\"";
      }

      if (md->isNoExcept()) {
         t << " noexcept=\"yes\"";
      }

      if (al.volatileSpecifier) {
         t << " volatile=\"yes\"";
      }


      t << " virt=\"";
      switch (md->virtualness()) {
         case Normal:
            t << "non-virtual";
            break;

         case Virtual:
            t << "virtual";
            break;

         case Pure:
            t << "pure-virtual";
            break;

         default:
            assert(0);
      }
      t << "\"";
   }

   if (md->memberType() == MemberDefType::Enumeration) {
      t << " strong=\"";

      if (md->isStrong()) {
         t << "yes";

      } else {
          t << "no";
      }

      t << "\"";
   }

   if (md->memberType() == MemberDefType::Variable) {

      t << " mutable=\"";
      if (md->isMutable()) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";

      if (md->isInitonly()) {
         t << " initonly=\"yes\"";
      }
      if (md->isAttribute()) {
         t << " attribute=\"yes\"";
      }
      if (md->isUNOProperty()) {
         t << " property=\"yes\"";
      }
      if (md->isReadonly()) {
         t << " readonly=\"yes\"";
      }
      if (md->isBound()) {
         t << " bound=\"yes\"";
      }
      if (md->isRemovable()) {
         t << " removable=\"yes\"";
      }
      if (md->isConstrained()) {
         t << " constrained=\"yes\"";
      }
      if (md->isTransient()) {
         t << " transient=\"yes\"";
      }
      if (md->isMaybeVoid()) {
         t << " maybevoid=\"yes\"";
      }
      if (md->isMaybeDefault()) {
         t << " maybedefault=\"yes\"";
      }
      if (md->isMaybeAmbiguous()) {
         t << " maybeambiguous=\"yes\"";
      }

   } else if (md->memberType() == MemberDefType::Property) {
      t << " readable=\"";
      if (md->isReadable()) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";

      t << " writable=\"";
      if (md->isWritable()) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";

      t << " gettable=\"";
      if (md->isGettable()) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";

      t << " privategettable=\"";
      if (md->isPrivateGettable()) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";

      t << " protectedgettable=\"";
      if (md->isProtectedGettable()) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";

      t << " settable=\"";
      if (md->isSettable()) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";

      t << " privatesettable=\"";
      if (md->isPrivateSettable()) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";

      t << " protectedsettable=\"";
      if (md->isProtectedSettable()) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";

      if (md->isAssign() || md->isCopy() || md->isRetain() || md->isStrong() || md->isWeak()) {
         t << " accessor=\"";
         if (md->isAssign()) {
            t << "assign";

         } else if (md->isCopy()) {
            t << "copy";

         } else if (md->isRetain()) {
            t << "retain";

         } else if (md->isStrong()) {
            t << "strong";

         } else if (md->isWeak()) {
            t << "weak";
         }

         t << "\"";
      }

   } else if (md->memberType() == MemberDefType::Event) {
      t << " add=\"";

      if (md->isAddable()) {
         t << "yes";
      } else {
         t << "no";
      }

      t << "\"";
      t << " remove=\"";

      if (md->isRemovable()) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";

      t << " raise=\"";
      if (md->isRaisable()) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";
   }

   t << ">" << endl;

   if (md->memberType() != MemberDefType::Define && md->memberType() != MemberDefType::Enumeration) {
      writeMemberTemplateLists(md, t);

      QString typeStr = md->typeString();    // replaceAnonymousScopes(md->typeString());
      stripQualifiers(typeStr);

      t << "        <type>";
      linkifyText(TextFragmentXml(t), def, md->getBodyDef(), md, typeStr);

      t << "</type>" << endl;
      t << "        <definition>" << convertToXML(md->definition()) << "</definition>" << endl;
      t << "        <argsstring>" << convertToXML(md->argsString()) << "</argsstring>" << endl;
   }

   if (md->memberType() == MemberDefType::Enumeration)  {
      t << "        <type>";
      linkifyText(TextFragmentXml(t), def, md->getBodyDef(), md, md->enumBaseType());
      t << "</type>" << endl;
   }

   t << "        <name>" << convertToXML(md->name()) << "</name>" << endl;

   if (md->memberType() == MemberDefType::Property) {
      if (md->isReadable()) {
         t << "        <read>" << convertToXML(md->getPropertyRead()) << "</read>" << endl;
      }

      if (md->isWritable()) {
         t << "        <write>" << convertToXML(md->getPropertyWrite()) << "</write>" << endl;
      }
   }

   if (md->memberType() == MemberDefType::Variable && ! md->bitfieldString().isEmpty() ) {
      QString bitfield = md->bitfieldString();

      if (bitfield.size() > 0 &&  bitfield.at(0) == ':') {
         bitfield = bitfield.mid(1);
      }

      t << "        <bitfield>" << convertToXML(bitfield) << "</bitfield>" << endl;
   }

   QSharedPointer<MemberDef> rmd = md->reimplements();
   if (rmd) {
      t << "        <reimplements refid=\""
        << memberOutputFileBase(rmd) << "_1" << rmd->anchor() << "\">"
        << convertToXML(rmd->name()) << "</reimplements>" << endl;
   }

   QSharedPointer<MemberList> rbml = md->reimplementedBy();

   if (rbml) {
      for (auto rmd : *rbml) {
         t << "        <reimplementedby refid=\""
           << memberOutputFileBase(rmd) << "_1" << rmd->anchor() << "\">"
           << convertToXML(rmd->name()) << "</reimplementedby>" << endl;
      }
   }

   auto mdType = md->memberType();

   if (isFunc) {
      // function
      const ArgumentList &declAl = md->getDeclArgumentList();
      const ArgumentList &defAl  = md->getArgumentList();

      auto iter = defAl.begin();

      for (const auto &arg : declAl) {

         t << "        <param>" << endl;
         if (! arg.attrib.isEmpty()) {
            t << "          <attributes>";
            writeXMLString(t, arg.attrib);
            t << "</attributes>" << endl;
         }

         if (! arg.type.isEmpty()) {
            t << "          <type>";
            linkifyText(TextFragmentXml(t), def, md->getBodyDef(), md, arg.type);
            t << "</type>" << endl;
         }

         if (! arg.name.isEmpty()) {
            t << "          <declname>";
            writeXMLString(t, arg.name);
            t << "</declname>" << endl;
         }

         if (iter != defAl.end() && ! iter->name.isEmpty() && iter->name != arg.name) {
            t << "          <defname>";
            writeXMLString(t, iter->name);
            t << "</defname>" << endl;
         }

         if (! arg.array.isEmpty()) {
            t << "          <array>";
            writeXMLString(t, arg.array);
            t << "</array>" << endl;
         }

         if (! arg.defval.isEmpty()) {
            t << "          <defval>";
            linkifyText(TextFragmentXml(t), def, md->getBodyDef(), md, arg.defval);
            t << "</defval>" << endl;
         }

         if (iter != defAl.end() && iter->hasDocumentation()) {
            t << "          <briefdescription>";

            writeXMLDocBlock(t, md->getDefFileName(), md->getDefLine(),
                             md->getOuterScope(), md, iter->docs);

            t << "</briefdescription>" << endl;
         }

         t << "        </param>" << endl;

         if (iter != defAl.end()) {
            ++iter;
         }
      }

   } else if (mdType == MemberDefType::Define && ! md->argsString().isEmpty() ) {
      // define
      const ArgumentList &argList = md->getArgumentList();

      if (argList.listEmpty())  {
         // special case for "foo()" to distinguish it from "foo".
         t << "        <param></param>" << endl;

      } else {

         for (const auto &arg : argList) {
            t << "        <param><defname>" << arg.type << "</defname></param>" << endl;
         }
      }
   }

   if (md->hasOneLineInitializer() || md->hasMultiLineInitializer()) {
      t << "        <initializer>";
      linkifyText(TextFragmentXml(t), def, md->getBodyDef(), md, md->initializer());
      t << "</initializer>" << endl;
   }

   if (! md->excpString().isEmpty() ) {
      t << "        <exceptions>";
      linkifyText(TextFragmentXml(t), def, md->getBodyDef(), md, md->excpString());
      t << "</exceptions>" << endl;
   }

   if (mdType == MemberDefType::Enumeration) {
      // enum
      QSharedPointer<MemberList> enumFields = md->enumFieldList();

      if (enumFields) {

         for (auto emd : *enumFields) {

            ti << "    <member refid=\"" << memberOutputFileBase(md)
               << "_1" << emd->anchor() << "\" kind=\"enumvalue\"><name>"
               << convertToXML(emd->name()) << "</name></member>" << endl;

            t << "        <enumvalue id=\"" << memberOutputFileBase(md) << "_1"
              << emd->anchor() << "\" prot=\"";

            switch (emd->protection()) {
               case Public:
                  t << "public";
                  break;

               case Protected:
                  t << "protected";
                  break;

               case Private:
                  t << "private";
                  break;

               case Package:
                  t << "package";
                  break;
            }

            t << "\">" << endl;
            t << "          <name>";
            writeXMLString(t, emd->name());

            t << "</name>" << endl;
            if (! emd->initializer().isEmpty()) {
               t << "          <initializer>";
               writeXMLString(t, emd->initializer());
               t << "</initializer>" << endl;
            }

            t << "          <briefdescription>" << endl;
            writeXMLDocBlock(t, emd->briefFile(), emd->briefLine(), emd->getOuterScope(), emd, emd->briefDescription());
            t << "          </briefdescription>" << endl;
            t << "          <detaileddescription>" << endl;
            writeXMLDocBlock(t, emd->docFile(), emd->docLine(), emd->getOuterScope(), emd, emd->documentation());
            t << "          </detaileddescription>" << endl;
            t << "        </enumvalue>" << endl;
         }
      }
   }

   t << "        <briefdescription>" << endl;
   writeXMLDocBlock(t, md->briefFile(), md->briefLine(), md->getOuterScope(), md, md->briefDescription());

   t << "        </briefdescription>" << endl;
   t << "        <detaileddescription>" << endl;
   writeXMLDocBlock(t, md->docFile(), md->docLine(), md->getOuterScope(), md, md->documentation());

   t << "        </detaileddescription>" << endl;
   t << "        <inbodydescription>" << endl;
   writeXMLDocBlock(t, md->docFile(), md->inbodyLine(), md->getOuterScope(), md, md->inbodyDocumentation());

   t << "        </inbodydescription>" << endl;

   if (md->getDefLine() != -1) {
      t << "        <location file=\""
        << convertToXML(stripFromPath(md->getDefFileName())) << "\" line=\""
        << md->getDefLine() << "\" column=\""
        << md->getDefColumn() << "\"" ;

      if (md->getStartBodyLine() != -1) {
         QSharedPointer<FileDef> bodyDef = md->getBodyDef();

         if (bodyDef) {
            t << " bodyfile=\"" << convertToXML(stripFromPath(bodyDef->getFilePath())) << "\"";
         }
         t << " bodystart=\"" << md->getStartBodyLine() << "\" bodyend=\""
           << md->getEndBodyLine() << "\"";
      }

      if (md->getDeclLine() != -1 ) {
         t << " declfile=\"" << convertToXML(stripFromPath(md->getDeclFileName())) << "\" declline=\""
           << md->getDeclLine() << "\" declcolumn=\""
           << md->getDeclColumn() << "\"";
      }

      t << "/>" << endl;
   }

   const MemberSDict &mdict_1 = md->getReferencesMembers();

   for (auto &item : mdict_1) {
      writeMemberReference(t, def, item, "references");
   }

   const MemberSDict &mdict_2 = md->getReferencedByMembers();

   for (auto &item : mdict_2) {
      writeMemberReference(t, def, item, "referencedby");
   }

   t << "      </memberdef>" << endl;
}

// namespace members are also inserted in the file scope, but
// used to prevent duplication in the XML output
static bool memberVisible(QSharedPointer<Definition> def, QSharedPointer<MemberDef> md)
{
   static const bool xmlIncludeNsMembers = Config::getBool("xml-include-ns-members");

   return xmlIncludeNsMembers || def->definitionType() != Definition::TypeFile || md->getNamespaceDef() == nullptr;
}

static void generateXMLSection(QSharedPointer<Definition> d, QTextStream &ti, QTextStream &t, QSharedPointer<MemberList> ml,
                               const QString &kind, const QString &header = QString(), const QString &documentation = QString() )
{
   if (ml == nullptr) {
      return;
   }

   int count = 0;

   for (auto md : *ml) {
      if (memberVisible(d, md) && (md->memberType() != MemberDefType::EnumValue) && ! md->isHidden()) {
         count++;
      }
   }

   if (count == 0) {
      return;   // empty list
   }

   t << "      <sectiondef kind=\"" << kind << "\">" << endl;

   if (! header.isEmpty()) {
      t << "      <header>" << convertToXML(header) << "</header>" << endl;
   }

   if (! documentation.isEmpty() ) {
      t << "      <description>";
      writeXMLDocBlock(t, d->docFile(), d->docLine(), d, QSharedPointer<MemberDef>(), documentation);
      t << "</description>" << endl;
   }

   for (auto md : *ml) {
      if (memberVisible(d, md)) {
         generateXMLForMember(md, ti, t, d);
      }

   }
   t << "      </sectiondef>" << endl;
}

static void writeListOfAllMembers(QSharedPointer<ClassDef> cd, QTextStream &t)
{
   t << "    <listofallmembers>" << endl;

   for (auto mni : cd->memberNameInfoSDict()) {

     for (auto &mi : *mni) {
         QSharedPointer<MemberDef> md = mi.memberDef;

         if (! md->name().isEmpty() && ! md->name().startsWith('@')) {
            // skip anonymous members
            Protection prot = mi.prot;
            Specifier virt = md->virtualness();

            t << "      <member refid=\"" << memberOutputFileBase(md) << "_1" <<
              md->anchor() << "\" prot=\"";

            switch (prot) {
               case Public:
                  t << "public";
                  break;

               case Protected:
                  t << "protected";
                  break;

               case Private:
                  t << "private";
                  break;

               case Package:
                  t << "package";
                  break;
            }

            t << "\" virt=\"";
            switch (virt) {
               case Normal:
                  t << "non-virtual";
                  break;

               case Virtual:
                  t << "virtual";
                  break;

               case Pure:
                  t << "pure-virtual";
                  break;
            }
            t << "\"";

            if (! mi.ambiguityResolutionScope.isEmpty()) {
               t << " ambiguityscope=\"" << convertToXML(mi.ambiguityResolutionScope) << "\"";
            }

            t << "><scope>" << convertToXML(cd->name()) << "</scope><name>" <<
              convertToXML(md->name()) << "</name></member>" << endl;
         }
      }
   }

   t << "    </listofallmembers>" << endl;
}

static void writeInnerClasses(const ClassSDict &cl, QTextStream &t)
{
   for (auto cd : cl) {
      if (! cd->isHidden() && ! cd->isAnonymous()) {
         // skip anonymous scopes
         t << "    <innerclass refid=\"" << classOutputFileBase(cd) << "\" prot=\"";

         switch (cd->protection()) {
            case Public:
               t << "public";
               break;

            case Protected:
               t << "protected";
               break;

            case Private:
               t << "private";
               break;

            case Package:
               t << "package";
               break;
         }
         t << "\">" << convertToXML(cd->name()) << "</innerclass>" << endl;
      }
   }

}

static void writeInnerNamespaces(const NamespaceSDict &nl, QTextStream &t)
{
   for (const auto &nd : nl) {
      if (! nd->isHidden() && nd->name().indexOf('@') == -1) {
         // skip anonymous scopes
         t << "    <innernamespace refid=\"" << nd->getOutputFileBase()
           << "\"" << (nd->isInlineNS() ? " inline=\"yes\"" : "")
           << ">" << convertToXML(nd->name()) << "</innernamespace>" << endl;
      }
   }
}

static void writeInnerFiles(const FileList &fl, QTextStream &t)
{
   for (const auto &fd : fl) {
      t << "    <innerfile refid=\"" << fd->getOutputFileBase()
        << "\">" << convertToXML(fd->name()) << "</innerfile>" << endl;
   }
}

static void writeInnerPages(const PageSDict *pl, QTextStream &t)
{
   if (pl != nullptr) {
     for (const auto pd : *pl) {
         t << "    <innerpage refid=\"" << pd->getOutputFileBase();
         if (pd->getGroupDef()) {
            t << "_" << pd->name();
         }
         t << "\">" << convertToXML(pd->title()) << "</innerpage>" << endl;
      }
   }
}

static void writeInnerGroups(const SortedList<QSharedPointer<GroupDef>> *gl, QTextStream &t)
{
   if (gl != nullptr) {
      for (const auto &sgd : *gl) {
         t << "    <innergroup refid=\"" << sgd->getOutputFileBase()
           << "\">" << convertToXML(sgd->groupTitle())
           << "</innergroup>" << endl;
      }
   }
}

static void writeInnerDirs(const SortedList<QSharedPointer<DirDef>> dl, QTextStream &t)
{
   for (const auto &subdir : dl) {
      t << "    <innerdir refid=\"" << subdir->getOutputFileBase()
        << "\">" << convertToXML(subdir->displayName()) << "</innerdir>" << endl;
   }
}

static void generateXMLForClass(QSharedPointer<ClassDef> cd, QTextStream &ti)
{
   // + brief description
   // + detailed description
   // + template argument list(s)
   // - include file
   // + member groups
   // + inheritance diagram
   // + list of direct super classes
   // + list of direct sub classes
   // + list of inner classes
   // + collaboration diagram
   // + list of all members
   // + user defined member sections
   // + standard member sections
   // + detailed member documentation
   // - examples using the class

   if (cd->isReference()) {
      return;   // skip external references
   }

   if (cd->isHidden()) {
      return;   // skip hidden classes
   }

   if (cd->isAnonymous()) {
      return;   // skip anonymous compounds
   }

   if (cd->templateMaster() != 0) {
      return;   // skip generated template instances
   }

   if (cd->isArtificial()) {
      return;   // skip artificially created classes
   }

   msg("Generating XML output for class %s\n", csPrintable(cd->name()));

   ti << "  <compound refid=\"" << classOutputFileBase(cd)
      << "\" kind=\"" << cd->compoundTypeString()
      << "\"><name>" << convertToXML(cd->name()) << "</name>" << endl;

   static const QString xmlOutDir = Config::getString("xml-output");
   QString fileName  = xmlOutDir + "/" + classOutputFileBase(cd) + ".xml";

   QFile fi(fileName);

   if (! fi.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), fi.error());
      return;
   }

   QTextStream t(&fi);

   writeXMLHeader(t);
   t << "  <compounddef id=\""
     << classOutputFileBase(cd) << "\" kind=\""
     << cd->compoundTypeString() << "\" language=\""
     << langToString(cd->getLanguage()) << "\" prot=\"";

   switch (cd->protection()) {
      case Public:
         t << "public";
         break;

      case Protected:
         t << "protected";
         break;

      case Private:
         t << "private";
         break;

      case Package:
         t << "package";
         break;
   }

   if (cd->isFinal()) {
      t << "\" final=\"yes";
   }

   if (cd->isSealed()) {
      t << "\" sealed=\"yes";
   }

   if (cd->isAbstract()) {
      t << "\" abstract=\"yes";
   }

   t << "\">" << endl;
   t << "    <compoundname>";
   writeXMLString(t, cd->name());
   t << "</compoundname>" << endl;

   if (cd->baseClasses()) {

      for (auto &bcd : *cd->baseClasses()) {
         t << "    <basecompoundref ";
         if (bcd->classDef->isLinkable()) {
            t << "refid=\"" << classOutputFileBase(bcd->classDef) << "\" ";
         }
         t << "prot=\"";

         switch (bcd->prot) {
            case Public:
               t << "public";
               break;

            case Protected:
               t << "protected";
               break;

            case Private:
               t << "private";
               break;

            case Package:
               assert(0);
               break;
         }
         t << "\" virt=\"";

         switch (bcd->virt) {
            case Normal:
               t << "non-virtual";
               break;

            case Virtual:
               t << "virtual";
               break;

            case Pure:
               t << "pure-virtual";
               break;
         }
         t << "\">";

         if (! bcd->templSpecifiers.isEmpty()) {
            t << convertToXML(insertTemplateSpecifierInScope(bcd->classDef->name(), bcd->templSpecifiers));
         } else {
            t << convertToXML(bcd->classDef->displayName());
         }
         t  << "</basecompoundref>" << endl;
      }
   }


   if (cd->subClasses()) {

      for (auto &bcd : *cd->subClasses()) {

         t << "    <derivedcompoundref refid=\""
           << classOutputFileBase(bcd->classDef)
           << "\" prot=\"";

         switch (bcd->prot) {
            case Public:
               t << "public";
               break;

            case Protected:
               t << "protected";
               break;

            case Private:
               t << "private";
               break;

            case Package:
               assert(0);
               break;
         }
         t << "\" virt=\"";

         switch (bcd->virt) {
            case Normal:
               t << "non-virtual";
               break;

            case Virtual:
               t << "virtual";
               break;

            case Pure:
               t << "pure-virtual";
               break;
         }

         t << "\">" << convertToXML(bcd->classDef->displayName())
           << "</derivedcompoundref>" << endl;
      }
   }

   const IncludeInfo &item = cd->includeInfo();

   QString nm = item.includeName;

   if (nm.isEmpty() && item.fileDef) {
      nm = item.fileDef->docName();
   }

   if (! nm.isEmpty()) {
      t << "    <includes";

      if (item.fileDef && ! item.fileDef->isReference()) {
         // verify external references are supported
         t << " refid=\"" << item.fileDef->getOutputFileBase() << "\"";
      }

      t << " local=\"" << (item.local ? "yes" : "no") << "\">";
      t << nm;
      t << "</includes>" << endl;
   }

   writeInnerClasses(cd->getClassSDict(), t);
   writeTemplateList(cd, t);

   for (auto &mg : cd->getMemberGroupSDict() ) {
      generateXMLSection(cd, ti, t, mg->members(), "user-defined", mg->header(), mg->documentation());
   }

   for (auto &ml : cd->getMemberLists() ) {
      if ((ml->listType() & MemberListType_detailedLists) == 0) {
         generateXMLSection(cd, ti, t, ml, g_xmlSectionMapper.value(ml->listType()));
      }
   }

   t << "    <briefdescription>" << endl;
   writeXMLDocBlock(t, cd->briefFile(), cd->briefLine(), cd, QSharedPointer<MemberDef>(), cd->briefDescription());

   t << "    </briefdescription>" << endl;
   t << "    <detaileddescription>" << endl;
   writeXMLDocBlock(t, cd->docFile(), cd->docLine(), cd, QSharedPointer<MemberDef>(), cd->documentation());

   t << "    </detaileddescription>" << endl;
   DotClassGraph inheritanceGraph(cd, DotNode::Inheritance);

   if (!inheritanceGraph.isTrivial()) {
      t << "    <inheritancegraph>" << endl;
      inheritanceGraph.writeXML(t);
      t << "    </inheritancegraph>" << endl;
   }

   DotClassGraph collaborationGraph(cd, DotNode::Collaboration);
   if (!collaborationGraph.isTrivial()) {
      t << "    <collaborationgraph>" << endl;
      collaborationGraph.writeXML(t);
      t << "    </collaborationgraph>" << endl;
   }

   t << "    <location file=\""
     << convertToXML(stripFromPath(cd->getDefFileName())) << "\" line=\""
     << cd->getDefLine() << "\"" << " column=\""
     << cd->getDefColumn() << "\"" ;

   if (cd->getStartBodyLine() != -1) {
      QSharedPointer<FileDef> bodyDef = cd->getBodyDef();

      if (bodyDef) {
         t << " bodyfile=\"" << convertToXML(stripFromPath(bodyDef->getFilePath())) << "\"";
      }
      t << " bodystart=\"" << cd->getStartBodyLine() << "\" bodyend=\""
        << cd->getEndBodyLine() << "\"";
   }

   t << "/>" << endl;
   writeListOfAllMembers(cd, t);
   t << "  </compounddef>" << endl;
   t << "</doxypress>" << endl;

   ti << "  </compound>" << endl;
}

static void generateXMLForNamespace(QSharedPointer<NamespaceDef> nd, QTextStream &ti)
{
   // + contained class definitions
   // + contained namespace definitions
   // + member groups
   // + normal members
   // + brief desc
   // + detailed desc
   // + location
   // - files containing (parts of) the namespace definition

   if (nd->isReference() || nd->isHidden()) {
      return;   // skip external references
   }

   ti << "  <compound refid=\"" << nd->getOutputFileBase()
      << "\" kind=\"namespace\"" << "><name>"
      << convertToXML(nd->name()) << "</name>" << endl;

   QString outputDirectory = Config::getString("xml-output");
   QString fileName = outputDirectory + "/" + nd->getOutputFileBase() + ".xml";

   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);

   writeXMLHeader(t);
   t << "  <compounddef id=\"" << nd->getOutputFileBase()
     << "\" kind=\"namespace\" "
     << (nd->isInlineNS() ? "inline=\"yes\" ":"")
     << "language=\""
     << langToString(nd->getLanguage()) << "\">" << endl;
   t << "    <compoundname>";

   writeXMLString(t, nd->name());
   t << "</compoundname>" << endl;

   writeInnerClasses(nd->getClassSDict(), t);
   writeInnerNamespaces(nd->getNamespaceSDict(), t);

   for (auto &mg : nd->getMemberGroupSDict()) {
      generateXMLSection(nd, ti, t, mg->members(), "user-defined", mg->header(), mg->documentation());
   }

   for (auto &ml : nd->getMemberLists()) {
      if ((ml->listType()&MemberListType_declarationLists) != 0) {
         generateXMLSection(nd, ti, t, ml, g_xmlSectionMapper.value(ml->listType()));
      }
   }

   t << "    <briefdescription>" << endl;

 writeXMLDocBlock(t, nd->briefFile(), nd->briefLine(), nd, QSharedPointer<MemberDef>(), nd->briefDescription());
   t << "    </briefdescription>" << endl;
   t << "    <detaileddescription>" << endl;

  writeXMLDocBlock(t, nd->docFile(), nd->docLine(), nd, QSharedPointer<MemberDef>(), nd->documentation());
   t << "    </detaileddescription>" << endl;
   t << "    <location file=\""
     << convertToXML(stripFromPath(nd->getDefFileName())) << "\" line=\""
     << nd->getDefLine() << "\"" << " column=\""
     << nd->getDefColumn() << "\"/>" << endl ;
   t << "  </compounddef>" << endl;
   t << "</doxypress>" << endl;

   ti << "  </compound>" << endl;
}

static void generateXMLForFile(QSharedPointer<FileDef> fd, QTextStream &ti)
{
   static const QString outputDirectory = Config::getString("xml-output");
   static const bool xmlProgramListing  = Config::getBool("xml-program-listing");

   if (fd->isReference()) {
      return;   // skip external references
   }

   ti << "  <compound refid=\"" << fd->getOutputFileBase()
      << "\" kind=\"file\"><name>" << convertToXML(fd->name())
      << "</name>" << endl;

   QString fileName = outputDirectory + "/" + fd->getOutputFileBase() + ".xml";

   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);
   // t.setEncoding(QTextStream::UnicodeUTF8);

   writeXMLHeader(t);
   t << "  <compounddef id=\"" << fd->getOutputFileBase()
     << "\" kind=\"file\" language=\""
     << langToString(fd->getLanguage()) << "\">" << endl;
   t << "    <compoundname>";

   writeXMLString(t, fd->name());
   t << "</compoundname>" << endl;

   if (fd->includeFileList()) {

      for (auto &inc : *fd->includeFileList()) {
         t << "    <includes";

         if (inc.fileDef && ! inc.fileDef->isReference()) {
            // TODO: support external references
            t << " refid=\"" << inc.fileDef->getOutputFileBase() << "\"";
         }

         t << " local=\"" << (inc.local ? "yes" : "no") << "\">";
         t << inc.includeName;
         t << "</includes>" << endl;
      }
   }

   if (fd->includedByFileList()) {

      for (auto &inc : *fd->includedByFileList())   {
         t << "    <includedby";
         if (inc.fileDef && ! inc.fileDef->isReference()) {
            // TODO: support external references
            t << " refid=\"" << inc.fileDef->getOutputFileBase() << "\"";
         }

         t << " local=\"" << (inc.local ? "yes" : "no") << "\">";
         t << inc.includeName;
         t << "</includedby>" << endl;
      }
   }

   DotInclDepGraph incDepGraph(fd, false);
   if (! incDepGraph.isTrivial()) {
      t << "    <incdepgraph>" << endl;
      incDepGraph.writeXML(t);
      t << "    </incdepgraph>" << endl;
   }

   DotInclDepGraph invIncDepGraph(fd, true);
   if (! invIncDepGraph.isTrivial()) {
      t << "    <invincdepgraph>" << endl;
      invIncDepGraph.writeXML(t);
      t << "    </invincdepgraph>" << endl;
   }

   if (! fd->getClassSDict().isEmpty()) {
      writeInnerClasses(fd->getClassSDict(), t);
   }

   if (! fd->getNamespaceSDict().isEmpty() ) {
      writeInnerNamespaces(fd->getNamespaceSDict(), t);
   }

   for (auto &mg : fd->getMemberGroupSDict() ) {
      generateXMLSection(fd, ti, t, mg->members(), "user-defined", mg->header(), mg->documentation());
   }

   for (auto &ml : fd->getMemberLists()) {
      if ((ml->listType()&MemberListType_declarationLists) != 0) {
         generateXMLSection(fd, ti, t, ml, g_xmlSectionMapper.value(ml->listType()));
      }
   }

   t << "    <briefdescription>" << endl;

   writeXMLDocBlock(t, fd->briefFile(), fd->briefLine(), fd, QSharedPointer<MemberDef>(), fd->briefDescription());
   t << "    </briefdescription>" << endl;
   t << "    <detaileddescription>" << endl;

   writeXMLDocBlock(t, fd->docFile(), fd->docLine(), fd, QSharedPointer<MemberDef>(), fd->documentation());
   t << "    </detaileddescription>" << endl;

   if (xmlProgramListing) {
      writeXMLCodeBlock(t, fd);
   }

   t << "    <location file=\"" << stripFromPath(fd->getDefFileName()) << "\"/>" << endl;
   t << "  </compounddef>" << endl;
   t << "</doxypress>" << endl;

   ti << "  </compound>" << endl;
}

static void generateXMLForGroup(QSharedPointer<GroupDef> gd, QTextStream &ti)
{
   // + members
   // + member groups
   // + files
   // + classes
   // + namespaces
   // - packages
   // + pages
   // + child groups
   // - examples
   // + brief description
   // + detailed description

   static const QString outputDirectory = Config::getString("xml-output");

   if (gd->isReference()) {
      return;   // skip external references
   }

   ti << "  <compound refid=\"" << gd->getOutputFileBase()
      << "\" kind=\"group\"><name>" << convertToXML(gd->name()) << "</name>" << endl;

   QString fileName = outputDirectory + "/" + gd->getOutputFileBase() + ".xml";

   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
       err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);
   //t.setEncoding(QTextStream::UnicodeUTF8);

   writeXMLHeader(t);
   t << "  <compounddef id=\""
     << gd->getOutputFileBase() << "\" kind=\"group\">" << endl;
   t << "    <compoundname>" << convertToXML(gd->name()) << "</compoundname>" << endl;
   t << "    <title>" << convertToXML(gd->groupTitle()) << "</title>" << endl;

   writeInnerFiles(gd->getFiles(), t);
   writeInnerClasses(gd->getClasses(), t);
   writeInnerNamespaces(gd->getNamespaces(), t);
   writeInnerPages(gd->getPages(), t);
   writeInnerGroups(gd->getSubGroups(), t);

   for (auto &mg : gd->getMemberGroupSDict()) {
      generateXMLSection(gd, ti, t, mg->members(), "user-defined", mg->header(), mg->documentation());
   }

   for (auto &ml : gd->getMemberLists()) {
      if ((ml->listType() & MemberListType_declarationLists) != 0) {
         generateXMLSection(gd, ti, t, ml, g_xmlSectionMapper.value(ml->listType()));
      }
   }

   t << "    <briefdescription>" << endl;

   writeXMLDocBlock(t, gd->briefFile(), gd->briefLine(), gd, QSharedPointer<MemberDef>(), gd->briefDescription());
   t << "    </briefdescription>" << endl;
   t << "    <detaileddescription>" << endl;

   writeXMLDocBlock(t, gd->docFile(), gd->docLine(), gd, QSharedPointer<MemberDef>(), gd->documentation());
   t << "    </detaileddescription>" << endl;
   t << "  </compounddef>" << endl;
   t << "</doxypress>" << endl;

   ti << "  </compound>" << endl;
}

static void generateXMLForDir(QSharedPointer<DirDef> dd, QTextStream &ti)
{
   static const QString outputDirectory = Config::getString("xml-output");

   if (dd->isReference()) {
      return;   // skip external references
   }

   ti << "  <compound refid=\"" << dd->getOutputFileBase()
      << "\" kind=\"dir\"><name>" << convertToXML(dd->displayName())
      << "</name>" << endl;

   QString fileName = outputDirectory + "/" + dd->getOutputFileBase() + ".xml";

   QFile f(fileName);

   if (!f.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);
   //t.setEncoding(QTextStream::UnicodeUTF8);
   writeXMLHeader(t);
   t << "  <compounddef id=\""
     << dd->getOutputFileBase() << "\" kind=\"dir\">" << endl;
   t << "    <compoundname>" << convertToXML(dd->displayName()) << "</compoundname>" << endl;

   writeInnerDirs(dd->subDirs(), t);
   writeInnerFiles(dd->getFiles(), t);

   t << "    <briefdescription>" << endl;
   writeXMLDocBlock(t, dd->briefFile(), dd->briefLine(), dd, QSharedPointer<MemberDef>(), dd->briefDescription());
   t << "    </briefdescription>" << endl;
   t << "    <detaileddescription>" << endl;
   writeXMLDocBlock(t, dd->docFile(), dd->docLine(), dd, QSharedPointer<MemberDef>(), dd->documentation());
   t << "    </detaileddescription>" << endl;
   t << "    <location file=\"" << convertToXML(stripFromPath(dd->name())) << "\"/>" << endl;
   t << "  </compounddef>" << endl;
   t << "</doxypress>" << endl;

   ti << "  </compound>" << endl;
}

static void generateXMLForPage(QSharedPointer<PageDef> pd, QTextStream &ti, bool isExample)
{
   static const QString outputDirectory = Config::getString("xml-output");
   static const QString projectName     = Config::getString("project-name");

   const char *kindName = isExample ? "example" : "page";

   if (pd->isReference()) {
      return;
   }

   QString pageName = pd->getOutputFileBase();
   if (pd->getGroupDef()) {
      pageName += "_" + pd->name();
   }

   if (pageName == "index") {
      pageName = "indexpage";   // to prevent overwriting the generated index page
   }

   ti << "  <compound refid=\"" << pageName
      << "\" kind=\"" << kindName << "\"><name>" << convertToXML(pd->name())
      << "</name>" << endl;

   QString fileName = outputDirectory + "/" + pageName + ".xml";

   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);
   // t.setEncoding(QTextStream::UnicodeUTF8);

   writeXMLHeader(t);
   t << "  <compounddef id=\"" << pageName;
   t << "\" kind=\"" << kindName << "\">" << endl;
   t << "    <compoundname>" << convertToXML(pd->name())
     << "</compoundname>" << endl;

   if (pd == Doxy_Globals::mainPage) {
      // main page is special
      QString title;

      if (! pd->title().isEmpty() && pd->title().toLower() != "notitle") {
         title = filterTitle(convertCharEntities(Doxy_Globals::mainPage->title()));

      } else {
         title = projectName;
      }

      t << "    <title>" << convertToXML(convertCharEntities(title))
        << "</title>" << endl;

   } else {
      QSharedPointer<SectionInfo> si = Doxy_Globals::sectionDict.find(pd->name());

      if (si != nullptr) {
         t << "    <title>" << convertToXML(convertCharEntities(filterTitle(si->title)))
           << "</title>" << endl;
      }
   }

   writeInnerPages(pd->getSubPages(), t);
   SectionDict &sectionDict = pd->getSectionDict();

   if (pd->localToc().isXmlEnabled() && ! sectionDict.isEmpty()) {
      t << "    <tableofcontents>" << endl;

      int level = 1;

      bool activeLevel[5] = { false, false, false, false, false };
      int maxLevel = pd->localToc().xmlLevel();

      for (auto si : sectionDict) {
         if (si->type == SectionInfo::Section || si->type == SectionInfo::Subsection ||
                  si->type == SectionInfo::Subsubsection || si->type == SectionInfo::Paragraph) {

            int nextLevel = (int)si->type;

            if (nextLevel > level) {

               for (int index = level; index < nextLevel; ++index) {
                  if (index < maxLevel) {
                     t << "    <tableofcontents>" << endl;
                  }
               }

            } else if (nextLevel < level) {

               for (int index = level; index > nextLevel; --index) {
                  if (index <= maxLevel && activeLevel[index]) {
                     t << "    </tocsect>" << endl;
                  }

                  activeLevel[index] = false;

                  if (index <= maxLevel) {
                     t << "    </tableofcontents>" << endl;
                  }
               }
            }

            if (nextLevel <= maxLevel) {

               if (activeLevel[nextLevel]) {
                  t << "    </tocsect>" << endl;
               }

               QString titleDoc = convertToXML(si->title);

               t << "      <tocsect>" << endl;
               t << "        <name>" << (si->title.isEmpty() ? si->label : titleDoc) << "</name>" << endl;
               t << "        <reference>"  <<  convertToXML(pageName) << "_1" << convertToXML(si->label) << "</reference>" << endl;

               activeLevel[nextLevel] = true;
               level = nextLevel;
            }
         }
      }

      while (level > 1 && level <= maxLevel) {
         if (activeLevel[level]) {
            t << "    </tocsect>" << endl;
         }

         activeLevel[level] = false;
         t << "    </tableofcontents>" << endl;
         --level;
      }

      if (level <= maxLevel && activeLevel[level]) {
         t << "    </tocsect>" << endl;
      }

      activeLevel[level] = false;
      t << "    </tableofcontents>" << endl;
   }

   t << "    <briefdescription>" << endl;
   writeXMLDocBlock(t, pd->briefFile(), pd->briefLine(), pd, QSharedPointer<MemberDef>(), pd->briefDescription());
   t << "    </briefdescription>" << endl;

   t << "    <detaileddescription>" << endl;

   if (isExample) {
      writeXMLDocBlock(t, pd->docFile(), pd->docLine(), pd, QSharedPointer<MemberDef>(),
                  pd->documentation() + "\n\\include " + pd->name());

   } else {
      writeXMLDocBlock(t, pd->docFile(), pd->docLine(), pd, QSharedPointer<MemberDef>(), pd->documentation());
   }

   t << "    </detaileddescription>" << endl;

   t << "    <location file=\"" << convertToXML(stripFromPath(pd->getDefFileName())) << "\"/>\n";
   t << "  </compounddef>" << endl;
   t << "</doxypress>" << endl;

   ti << "  </compound>" << endl;
}

void generateXML_output()
{
   // + classes
   // + namespaces
   // + files
   // + groups
   // + related pages
   // - examples

   static const QString outputDirectory = Config::getString("xml-output");
   QDir xmlDir(outputDirectory);

   createSubDirs(xmlDir);

   ResourceMgr::instance().copyResourceAs("xml/xml.xsd",   outputDirectory, "xml.xsd");
   ResourceMgr::instance().copyResourceAs("xml/index.xsd", outputDirectory, "index.xsd");

   QString fileName = outputDirectory + "/compound.xsd";
   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), f.error());
      return;
   }

   // write compound.xsd, but replace special marker with the entities
   QByteArray tmp = ResourceMgr::instance().getAsString("xml/compound.xsd");

   QString xmlData = QString::fromUtf8(tmp);
   QString::const_iterator iter = xmlData.constBegin();

   while (iter != xmlData.end()) {
      // find end of the line
      QString::const_iterator iter_endA = iter + 1;

      while (iter_endA != xmlData.constEnd()) {
         QChar c = iter_endA[-1];

         if (c == '\n') {
            break;
         } else {
            ++iter_endA;                        // skip to end of the line including \n
         }
      }

      if (iter != iter_endA) {
         QString tmp(iter, iter_endA);

         QTextStream t(&f);

         if (tmp.indexOf("<!-- Automatically insert here the HTML entities -->") != -1) {
            HtmlEntityMapper::instance()->writeXMLSchema(t);

         } else {
            t << tmp;
         }
      }

      iter = iter_endA;
   }
   f.close();

   //
   fileName = outputDirectory + "/index.xml";
   f.setFileName(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);

   // write index header
   t << "<?xml version='1.0' encoding='UTF-8' standalone='no'?>" << endl;;
   t << "<doxypressindex xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ";
   t << "xsi:noNamespaceSchemaLocation=\"index.xsd\" ";
   t << "version=\""  << versionString << "\" ";
   t << "xml:lang=\"" << theTranslator->trLanguageId() << "\"";
   t << ">" << endl;

   for (auto &cd : Doxy_Globals::classSDict) {
       generateXMLForClass(cd, t);
   }

   for (auto &nd : Doxy_Globals::namespaceSDict) {
      msg("Generating XML output for namespace %s\n", csPrintable(nd->name()));
      generateXMLForNamespace(nd, t);
   }

   for (auto &fn : Doxy_Globals::inputNameList) {
      for (auto fd : *fn) {
         msg("Generating XML output for file %s\n", csPrintable(fd->name()));
         generateXMLForFile(fd, t);
      }
   }

   for (auto &gd : Doxy_Globals::groupSDict) {
      msg("Generating XML output for group %s\n", csPrintable(gd->name()));
      generateXMLForGroup(gd, t);
   }

   for (auto &pd : Doxy_Globals::pageSDict) {
      msg("Generating XML output for page %s\n", csPrintable(pd->name()));
      generateXMLForPage(pd, t, false);
   }

   for (auto dir : Doxy_Globals::directories) {
      msg("Generating XML output for dir %s\n", csPrintable(dir->name()));
      generateXMLForDir(dir, t);
   }

   for (auto &pd : Doxy_Globals::exampleSDict) {
      msg("Generating XML output for example %s\n", csPrintable(pd->name()));
      generateXMLForPage(pd, t, true);
   }

   if (Doxy_Globals::mainPage) {
      msg("Generating XML output for the main page\n");
      generateXMLForPage(Doxy_Globals::mainPage, t, false);
   }

   //t << "  </compoundlist>" << endl;
   t << "</doxypressindex>" << endl;

   writeCombineScript();
}
