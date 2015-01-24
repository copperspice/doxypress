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

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QHash>

#include <stdlib.h>

#include <arguments.h>
#include <config.h>
#include <classlist.h>
#include <dirdef.h>
#include <docparser.h>
#include <doxygen.h>
#include <doxy_build_info.h>
#include <dot.h>
#include <defargs.h>
#include <groupdef.h>
#include <htmlentity.h>
#include <filename.h>
#include <language.h>
#include <message.h>
#include <memberdef.h>
#include <memberlist.h>
#include <membername.h>
#include <membergroup.h>
#include <namespacedef.h>
#include <outputgen.h>
#include <pagedef.h>
#include <parserintf.h>
#include <resourcemgr.h>
#include <section.h>
#include <util.h>
#include <xmlgen.h>
#include <xmldocvisitor.h>

#include <doxy_globals.h>

// no debug info
#define XML_DB(x) do {} while(0)

// debug to stdout
//#define XML_DB(x) printf x
// debug inside output
//#define XML_DB(x) QByteArray __t;__t.sprintf x;m_t << __t

/** Helper class mapping MemberList::ListType to a string representing */
class XmlSectionMapper : public QHash<long, const char *>
{
 public:
   XmlSectionMapper() : QHash<long, const char *>() {
      insert(MemberListType_pubTypes, "public-type");
      insert(MemberListType_pubMethods, "public-func");
      insert(MemberListType_pubAttribs, "public-attrib");
      insert(MemberListType_pubSlots, "public-slot");
      insert(MemberListType_signals, "signal");
      insert(MemberListType_dcopMethods, "dcop-func");
      insert(MemberListType_properties, "property");
      insert(MemberListType_events, "event");
      insert(MemberListType_interfaces, "interfaces");
      insert(MemberListType_services, "services");
      insert(MemberListType_pubStaticMethods, "public-static-func");
      insert(MemberListType_pubStaticAttribs, "public-static-attrib");
      insert(MemberListType_proTypes, "protected-type");
      insert(MemberListType_proMethods, "protected-func");
      insert(MemberListType_proAttribs, "protected-attrib");
      insert(MemberListType_proSlots, "protected-slot");
      insert(MemberListType_proStaticMethods, "protected-static-func");
      insert(MemberListType_proStaticAttribs, "protected-static-attrib");
      insert(MemberListType_pacTypes, "package-type");
      insert(MemberListType_pacMethods, "package-func");
      insert(MemberListType_pacAttribs, "package-attrib");
      insert(MemberListType_pacStaticMethods, "package-static-func");
      insert(MemberListType_pacStaticAttribs, "package-static-attrib");
      insert(MemberListType_priTypes, "private-type");
      insert(MemberListType_priMethods, "private-func");
      insert(MemberListType_priAttribs, "private-attrib");
      insert(MemberListType_priSlots, "private-slot");
      insert(MemberListType_priStaticMethods, "private-static-func");
      insert(MemberListType_priStaticAttribs, "private-static-attrib");
      insert(MemberListType_friends, "friend");
      insert(MemberListType_related, "related");
      insert(MemberListType_decDefineMembers, "define");
      insert(MemberListType_decProtoMembers, "prototype");
      insert(MemberListType_decTypedefMembers, "typedef");
      insert(MemberListType_decEnumMembers, "enum");
      insert(MemberListType_decFuncMembers, "func");
      insert(MemberListType_decVarMembers, "var");
   }
};

static XmlSectionMapper g_xmlSectionMapper;

inline void writeXMLString(QTextStream &t, const char *s)
{
   t << convertToXML(s);
}

inline void writeXMLCodeString(QTextStream &t, const char *s, int &col)
{
   char c;
   while ((c = *s++)) {

      switch (c) {
         case '\t': {
            static int tabSize = Config_getInt("TAB_SIZE");
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
            break; // skip invalid XML characters (see http://www.w3.org/TR/2000/REC-xml-20001006#NT-Char)
         default:
            s = writeUtf8Char(t, s - 1);
            col++;
            break;
      }
   }
}

static void writeXMLHeader(QTextStream &t)
{
   t << "<?xml version='1.0' encoding='UTF-8' standalone='no'?>" << endl;;
   t << "<doxygen xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ";
   t << "xsi:noNamespaceSchemaLocation=\"compound.xsd\" ";
   t << "version=\"" << versionString << "\">" << endl;
}

static void writeCombineScript()
{
   QByteArray outputDirectory = Config_getString("XML_OUTPUT");
   QByteArray fileName = outputDirectory + "/combine.xslt";
   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", qPrintable(fileName), f.error());
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
     "    <doxygen version=\"{doxygenindex/@version}\">\n"
     "      <!-- Load all doxgen generated xml files -->\n"
     "      <xsl:for-each select=\"doxygenindex/compound\">\n"
     "        <xsl:copy-of select=\"document( concat( @refid, '.xml' ) )/doxygen/*\" />\n"
     "      </xsl:for-each>\n"
     "    </doxygen>\n"
     "  </xsl:template>\n"
     "</xsl:stylesheet>\n";
}


void writeXMLLink(QTextStream &t, const char *extRef, const char *compoundId,
                  const char *anchorId, const char *text, const char *tooltip)
{
   t << "<ref refid=\"" << compoundId;
   if (anchorId) {
      t << "_1" << anchorId;
   }
   t << "\" kindref=\"";
   if (anchorId) {
      t << "member";
   } else {
      t << "compound";
   }
   t << "\"";
   if (extRef) {
      t << " external=\"" << extRef << "\"";
   }
   if (tooltip) {
      t << " tooltip=\"" << convertToXML(tooltip) << "\"";
   }
   t << ">";
   writeXMLString(t, text);
   t << "</ref>";
}

/** Implements TextGeneratorIntf for an XML stream. */
class TextGeneratorXMLImpl : public TextGeneratorIntf
{
 public:
   TextGeneratorXMLImpl(QTextStream &t): m_t(t) {}
   void writeString(const char *s, bool /*keepSpaces*/) const {
      writeXMLString(m_t, s);
   }

   void writeBreak(int) const {}

   void writeLink(const char *extRef, const char *file, const char *anchor, const char *text) const {
      writeXMLLink(m_t, extRef, file, anchor, text, 0);
   }

 private:
   QTextStream &m_t;
};


/** Generator for producing XML formatted source code. */
class XMLCodeGenerator : public CodeOutputInterface
{
 public:

   XMLCodeGenerator(QTextStream &t) : m_t(t), m_lineNumber(-1), m_isMemberRef(false), m_col(0),
      m_insideCodeLine(false), m_normalHLNeedStartTag(true), m_insideSpecialHL(false) 
   {}

   virtual ~XMLCodeGenerator() 
   { }

   void codify(const QByteArray &text) override {
      XML_DB(("(codify \"%s\")\n", text));

      if (m_insideCodeLine && !m_insideSpecialHL && m_normalHLNeedStartTag) {
         m_t << "<highlight class=\"normal\">";
         m_normalHLNeedStartTag = false;
      }

      writeXMLCodeString(m_t, text, m_col);
   }

   void writeCodeLink(const QByteArray &ref, const QByteArray &file, const QByteArray &anchor, 
                      const QByteArray &name, const QByteArray &tooltip) override {

      XML_DB(("(writeCodeLink)\n"));

      if (m_insideCodeLine && !m_insideSpecialHL && m_normalHLNeedStartTag) {
         m_t << "<highlight class=\"normal\">";
         m_normalHLNeedStartTag = false;
      }

      writeXMLLink(m_t, ref, file, anchor, name, tooltip);
      m_col += qstrlen(name);
   }

   void writeTooltip(const char *, const DocLinkInfo &, const QByteArray &,
                     const QByteArray &, const SourceLinkInfo &, const SourceLinkInfo &) override {

      XML_DB(("(writeToolTip)\n"));
   }

   void startCodeLine(bool) {
      XML_DB(("(startCodeLine)\n"));

      m_t << "<codeline";

      if (m_lineNumber != -1) {
         m_t << " lineno=\"" << m_lineNumber << "\"";
         if (!m_refId.isEmpty()) {
            m_t << " refid=\"" << m_refId << "\"";
            if (m_isMemberRef) {
               m_t << " refkind=\"member\"";
            } else {
               m_t << " refkind=\"compound\"";
            }
         }
         if (!m_external.isEmpty()) {
            m_t << " external=\"" << m_external << "\"";
         }
      }
      m_t << ">";
      m_insideCodeLine = true;
      m_col = 0;
   }

   void endCodeLine() {
      XML_DB(("(endCodeLine)\n"));
      if (!m_insideSpecialHL && !m_normalHLNeedStartTag) {
         m_t << "</highlight>";
         m_normalHLNeedStartTag = true;
      }
      m_t << "</codeline>" << endl; // non DocBook
      m_lineNumber = -1;
      m_refId.resize(0);
      m_external.resize(0);
      m_insideCodeLine = false;
   }

   void startFontClass(const char *colorClass) {
      XML_DB(("(startFontClass)\n"));
      if (m_insideCodeLine && !m_insideSpecialHL && !m_normalHLNeedStartTag) {
         m_t << "</highlight>";
         m_normalHLNeedStartTag = true;
      }
      m_t << "<highlight class=\"" << colorClass << "\">"; // non DocBook
      m_insideSpecialHL = true;
   }

   void endFontClass() {
      XML_DB(("(endFontClass)\n"));
      m_t << "</highlight>"; // non DocBook
      m_insideSpecialHL = false;
   }

   void writeCodeAnchor(const char *) {
      XML_DB(("(writeCodeAnchor)\n"));
   }
   void writeLineNumber(const char *extRef, const QByteArray &compId, const char *anchorId, int l) override {
      XML_DB(("(writeLineNumber)\n"));

      // we remember the information provided here to use it
      // at the <codeline> start tag.

      m_lineNumber = l;

      if (! compId.isEmpty()) {
         m_refId = compId;

         if (anchorId) {
            m_refId += (QByteArray)"_1" + anchorId;
         }

         m_isMemberRef = anchorId != 0;

         if (extRef) {
            m_external = extRef;
         }
      }
   }

   void setCurrentDoc(Definition *, const char *, bool) {}
   void addWord(const QString &, bool) override {}

   void finish() {
      if (m_insideCodeLine) {
         endCodeLine();
      }
   }

 private:
   QTextStream &m_t;
   QByteArray m_refId;
   QByteArray m_external;
   int m_lineNumber;
   bool m_isMemberRef;
   int m_col;

   bool m_insideCodeLine;
   bool m_normalHLNeedStartTag;
   bool m_insideSpecialHL;
};

static void writeTemplateArgumentList(ArgumentList *al, QTextStream &t, Definition *scope, 
                                      FileDef *fileScope, int indent)
{
   QByteArray indentStr;
   indentStr.fill(' ', indent);

   if (al) {
      t << indentStr << "<templateparamlist>" << endl;
     
      for (auto a : *al) {
         t << indentStr << "  <param>" << endl;

         if (! a.type.isEmpty()) {
            t << indentStr <<  "    <type>";
            linkifyText(TextGeneratorXMLImpl(t), scope, fileScope, 0, a.type);
            t << "</type>" << endl;
         }
         if (!a.name.isEmpty()) {
            t << indentStr <<  "    <declname>" << a.name << "</declname>" << endl;
            t << indentStr <<  "    <defname>" << a.name << "</defname>" << endl;
         }
         if (!a.defval.isEmpty()) {
            t << indentStr << "    <defval>";
            linkifyText(TextGeneratorXMLImpl(t), scope, fileScope, 0, a.defval);
            t << "</defval>" << endl;
         }
         t << indentStr << "  </param>" << endl;
      }
      t << indentStr << "</templateparamlist>" << endl;
   }
}

static void writeMemberTemplateLists(MemberDef *md, QTextStream &t)
{
   ArgumentList *templMd = md->templateArguments();

   if (templMd) { // function template prefix
      writeTemplateArgumentList(templMd, t, md->getClassDef(), md->getFileDef(), 8);
   }
}

static void writeTemplateList(ClassDef *cd, QTextStream &t)
{
   writeTemplateArgumentList(cd->templateArguments(), t, cd, 0, 4);
}

static void writeXMLDocBlock(QTextStream &t, const QByteArray &fileName, int lineNr,
                             Definition *scope, MemberDef *md, const QByteArray &text)
{
   QByteArray stext = text.trimmed();

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

void writeXMLCodeBlock(QTextStream &t, FileDef *fd)
{
   ParserInterface *pIntf = Doxygen::parserManager->getParser(fd->getDefFileExtension());
   SrcLangExt langExt = getLanguageFromFileName(fd->getDefFileExtension());
   pIntf->resetCodeParserState();

   XMLCodeGenerator *xmlGen = new XMLCodeGenerator(t);

   pIntf->parseCode(*xmlGen,  // codeOutIntf
                    0,           // scopeName
                    fileToString(fd->getFilePath(), Config_getBool("FILTER_SOURCE_FILES")),
                    langExt,     // lang
                    false,       // isExampleBlock
                    0,           // exampleName
                    fd,          // fileDef
                    -1,          // startLine
                    -1,          // endLine
                    false,       // inlineFragement
                    0,           // memberDef
                    true         // showLineNumbers
                   );
   xmlGen->finish();
   delete xmlGen;
}

static void writeMemberReference(QTextStream &t, Definition *def, MemberDef *rmd, const char *tagName)
{
   QByteArray scope = rmd->getScopeString();
   QByteArray name  = rmd->name();

   if (!scope.isEmpty() && scope != def->name()) {
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

static void stripQualifiers(QByteArray &typeStr)
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

static QByteArray classOutputFileBase(ClassDef *cd)
{ 
   return cd->getOutputFileBase();   
}

static QByteArray memberOutputFileBase(MemberDef *md)
{
   return md->getOutputFileBase();
}

static void generateXMLForMember(MemberDef *md, QTextStream &ti, QTextStream &t, Definition *def)
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
   if (md->memberType() == MemberType_EnumValue) {
      return;
   }
   if (md->isHidden()) {
      return;
   }
  

   // group members are only visible in their group
   //if (def->definitionType()!=Definition::TypeGroup && md->getGroupDef()) return;

   QByteArray memType;
   bool isFunc = false;
   switch (md->memberType()) {
      case MemberType_Define:
         memType = "define";
         break;
      case MemberType_Function:
         memType = "function";
         isFunc = true;
         break;
      case MemberType_Variable:
         memType = "variable";
         break;
      case MemberType_Typedef:
         memType = "typedef";
         break;
      case MemberType_Enumeration:
         memType = "enum";
         break;
      case MemberType_EnumValue:
         assert(0);
         break;
      case MemberType_Signal:
         memType = "signal";
         isFunc = true;
         break;
      case MemberType_Slot:
         memType = "slot";
         isFunc = true;
         break;
      case MemberType_Friend:
         memType = "friend";
         isFunc = true;
         break;
      case MemberType_DCOP:
         memType = "dcop";
         isFunc = true;
         break;
      case MemberType_Property:
         memType = "property";
         break;
      case MemberType_Event:
         memType = "event";
         break;
      case MemberType_Interface:
         memType = "interface";
         break;
      case MemberType_Service:
         memType = "service";
         break;
   }

   ti << "    <member refid=\"" << memberOutputFileBase(md)
      << "_1" << md->anchor() << "\" kind=\"" << memType << "\"><name>"
      << convertToXML(md->name()) << "</name></member>" << endl;

   QByteArray scopeName;
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

   if (isFunc) {
      ArgumentList *al = md->argumentList();
      t << " const=\"";
      if (al != 0 && al->constSpecifier) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";

      t << " explicit=\"";
      if (md->isExplicit()) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";

      t << " inline=\"";
      if (md->isInline()) {
         t << "yes";
      } else {
         t << "no";
      }
      t << "\"";

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

      if (al && al->volatileSpecifier) {
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

   if (md->memberType() == MemberType_Variable) {
      //ArgumentList *al = md->argumentList();
      //t << " volatile=\"";
      //if (al && al->volatileSpecifier) t << "yes"; else t << "no";

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
   } else if (md->memberType() == MemberType_Property) {
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
   } else if (md->memberType() == MemberType_Event) {
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

   if (md->memberType() != MemberType_Define &&
         md->memberType() != MemberType_Enumeration
      ) {
      if (md->memberType() != MemberType_Typedef) {
         writeMemberTemplateLists(md, t);
      }
      QByteArray typeStr = md->typeString(); //replaceAnonymousScopes(md->typeString());
      stripQualifiers(typeStr);
      t << "        <type>";
      linkifyText(TextGeneratorXMLImpl(t), def, md->getBodyDef(), md, typeStr);
      t << "</type>" << endl;
      t << "        <definition>" << convertToXML(md->definition()) << "</definition>" << endl;
      t << "        <argsstring>" << convertToXML(md->argsString()) << "</argsstring>" << endl;
   }

   t << "        <name>" << convertToXML(md->name()) << "</name>" << endl;

   if (md->memberType() == MemberType_Property) {
      if (md->isReadable()) {
         t << "        <read>" << convertToXML(md->getReadAccessor()) << "</read>" << endl;
      }
      if (md->isWritable()) {
         t << "        <write>" << convertToXML(md->getWriteAccessor()) << "</write>" << endl;
      }
   }
   if (md->memberType() == MemberType_Variable && md->bitfieldString()) {
      QByteArray bitfield = md->bitfieldString();

      if (bitfield.at(0) == ':') {
         bitfield = bitfield.mid(1);
      }
      t << "        <bitfield>" << bitfield << "</bitfield>" << endl;
   }

   MemberDef *rmd = md->reimplements();
   if (rmd) {
      t << "        <reimplements refid=\""
        << memberOutputFileBase(rmd) << "_1" << rmd->anchor() << "\">"
        << convertToXML(rmd->name()) << "</reimplements>" << endl;
   }

   MemberList *rbml = md->reimplementedBy();

   if (rbml) {     
      for (auto rmd : *rbml) {
         t << "        <reimplementedby refid=\""
           << memberOutputFileBase(rmd) << "_1" << rmd->anchor() << "\">"
           << convertToXML(rmd->name()) << "</reimplementedby>" << endl;
      }
   }

   if (isFunc) { 
      //function
      ArgumentList *declAl = md->declArgumentList();
      ArgumentList *defAl  = md->argumentList();

      if (declAl && declAl->count() > 0) {
        
         auto defAli = defAl->begin();

         for (auto a : *declAl) {
                   
            t << "        <param>" << endl;
            if (!a.attrib.isEmpty()) {
               t << "          <attributes>";
               writeXMLString(t, a.attrib);
               t << "</attributes>" << endl;
            }
            if (!a.type.isEmpty()) {
               t << "          <type>";
               linkifyText(TextGeneratorXMLImpl(t), def, md->getBodyDef(), md, a.type);
               t << "</type>" << endl;
            }
            if (!a.name.isEmpty()) {
               t << "          <declname>";
               writeXMLString(t, a.name);
               t << "</declname>" << endl;
            }
            if (defAli != defAl->end() && ! defAli->name.isEmpty() && defAli->name != a.name) {
               t << "          <defname>";
               writeXMLString(t, defAli->name);
               t << "</defname>" << endl;
            }
            if (!a.array.isEmpty()) {
               t << "          <array>";
               writeXMLString(t, a.array);
               t << "</array>" << endl;
            }
            if (!a.defval.isEmpty()) {
               t << "          <defval>";
               linkifyText(TextGeneratorXMLImpl(t), def, md->getBodyDef(), md, a.defval);
               t << "</defval>" << endl;
            }
            if (defAli != defAl->end() && defAli->hasDocumentation()) {
               t << "          <briefdescription>";

               writeXMLDocBlock(t, md->getDefFileName(), md->getDefLine(),
                                md->getOuterScope(), md, defAli->docs);

               t << "</briefdescription>" << endl;
            }
            t << "        </param>" << endl;

            if (defAli != defAl->end()) {
               ++defAli;
            }
         }
      }

   } else if (md->memberType() == MemberType_Define && md->argsString()) { // define

      if (md->argumentList()->count() == 0) // special case for "foo()" to
         // disguish it from "foo".
      {
         t << "        <param></param>" << endl;

      } else {

         for (auto a : *md->argumentList()) {
            t << "        <param><defname>" << a.type << "</defname></param>" << endl;
         }
      }
   }

   if (md->hasOneLineInitializer() || md->hasMultiLineInitializer()) {
      t << "        <initializer>";
      linkifyText(TextGeneratorXMLImpl(t), def, md->getBodyDef(), md, md->initializer());
      t << "</initializer>" << endl;
   }

   if (md->excpString()) {
      t << "        <exceptions>";
      linkifyText(TextGeneratorXMLImpl(t), def, md->getBodyDef(), md, md->excpString());
      t << "</exceptions>" << endl;
   }

   if (md->memberType() == MemberType_Enumeration) { // enum
      MemberList *enumFields = md->enumFieldList();

      if (enumFields) {
       
         for (auto emd : *enumFields) {
            ti << "    <member refid=\"" << memberOutputFileBase(emd)
               << "_1" << emd->anchor() << "\" kind=\"enumvalue\"><name>"
               << convertToXML(emd->name()) << "</name></member>" << endl;

            t << "        <enumvalue id=\"" << memberOutputFileBase(emd) << "_1"
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
            if (!emd->initializer().isEmpty()) {
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
        << md->getDefFileName() << "\" line=\""
        << md->getDefLine() << "\"" << " column=\""
        << md->getDefColumn() << "\"" ;

      if (md->getStartBodyLine() != -1) {
         QSharedPointer<FileDef> bodyDef = md->getBodyDef();

         if (bodyDef) {
            t << " bodyfile=\"" << bodyDef->getFilePath() << "\"";
         }
         t << " bodystart=\"" << md->getStartBodyLine() << "\" bodyend=\""
           << md->getEndBodyLine() << "\"";
      }
      t << "/>" << endl;
   }

   //printf("md->getReferencesMembers()=%p\n",md->getReferencesMembers());
   MemberSDict *mdict = md->getReferencesMembers();

   if (mdict) {     
      for (auto item : *mdict) {
         writeMemberReference(t, def, item.data(), "references");
      }
   }

   mdict = md->getReferencedByMembers();
   if (mdict) {      
      for (auto item : *mdict) {
         writeMemberReference(t, def, item.data(), "referencedby");
      }
   }

   t << "      </memberdef>" << endl;
}

static void generateXMLSection(Definition *d, QTextStream &ti, QTextStream &t, MemberList *ml, 
                               const char *kind, const char *header = 0, const char *documentation = 0)
{
   if (ml == 0) {
      return;
   }

   int count = 0;

   for (auto md : *ml) {
      // namespace members are also inserted in the file scope, but
      // to prevent this duplication in the XML output, we filter those here.

      if (d->definitionType() != Definition::TypeFile || md->getNamespaceDef() == 0) {
         count++;
      }
   }

   if (count == 0) {
      return;   // empty list
   }

   t << "      <sectiondef kind=\"" << kind << "\">" << endl;

   if (header) {
      t << "      <header>" << convertToXML(header) << "</header>" << endl;
   }

   if (documentation) {
      t << "      <description>";
      writeXMLDocBlock(t, d->docFile(), d->docLine(), d, 0, documentation);
      t << "</description>" << endl;
   }

 
   for (auto md : *ml) {
      // namespace members are also inserted in the file scope, but
      // to prevent this duplication in the XML output, we filter those here.
      if (d->definitionType() != Definition::TypeFile || md->getNamespaceDef() == 0) {
         generateXMLForMember(md, ti, t, d);
      }
   }
   t << "      </sectiondef>" << endl;
}

static void writeListOfAllMembers(ClassDef *cd, QTextStream &t)
{
   t << "    <listofallmembers>" << endl;

   if (cd->memberNameInfoSDict()) {
     
      for (auto mni : *cd->memberNameInfoSDict()) {

        for (auto mi : *mni) {

            MemberDef *md = mi.memberDef;

            if (md->name().at(0) != '@') { 
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
   }
   t << "    </listofallmembers>" << endl;
}

static void writeInnerClasses(const ClassSDict *cl, QTextStream &t)
{
   if (cl) {

      for (auto cd : *cl) {
         if (! cd->isHidden() && cd->name().indexOf('@') == -1) { 
            // skip anonymous scopes
            t << "    <innerclass refid=\"" << classOutputFileBase(cd.data())
              << "\" prot=\"";

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
}

static void writeInnerNamespaces(const NamespaceSDict *nl, QTextStream &t)
{
   if (nl) {  
      for (auto nd : *nl) {
         if (!nd->isHidden() && nd->name().indexOf('@') == -1) { // skip anonymouse scopes
            t << "    <innernamespace refid=\"" << nd->getOutputFileBase()
              << "\">" << convertToXML(nd->name()) << "</innernamespace>" << endl;
         }
      }
   }
}

static void writeInnerFiles(const FileList *fl, QTextStream &t)
{
   if (fl) {     
      for (auto fd : *fl) {
         t << "    <innerfile refid=\"" << fd->getOutputFileBase()
           << "\">" << convertToXML(fd->name()) << "</innerfile>" << endl;
      }
   }
}

static void writeInnerPages(const PageSDict *pl, QTextStream &t)
{
   if (pl) {
     for (auto pd : *pl) {
         t << "    <innerpage refid=\"" << pd->getOutputFileBase();
         if (pd->getGroupDef()) {
            t << "_" << pd->name();
         }
         t << "\">" << convertToXML(pd->title()) << "</innerpage>" << endl;
      }
   }
}

static void writeInnerGroups(const SortedList<GroupDef *> *gl, QTextStream &t)
{
   if (gl) {
      for (auto sgd : *gl) {
         t << "    <innergroup refid=\"" << sgd->getOutputFileBase()
           << "\">" << convertToXML(sgd->groupTitle())
           << "</innergroup>" << endl;
      }
   }
}

static void writeInnerDirs(const SortedList<DirDef *> *dl, QTextStream &t)
{
   if (dl) {     
      for (auto subdir : *dl) {
         t << "    <innerdir refid=\"" << subdir->getOutputFileBase()
           << "\">" << convertToXML(subdir->displayName()) << "</innerdir>" << endl;
      }
   }
}

static void generateXMLForClass(ClassDef *cd, QTextStream &ti)
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
      return;   // skip external references.
   }

   if (cd->isHidden()) {
      return;   // skip hidden classes.
   }

   if (cd->name().indexOf('@') != -1) {
      return;   // skip anonymous compounds.
   }
   if (cd->templateMaster() != 0) {
      return;   // skip generated template instances.
   }
   if (cd->isArtificial()) {
      return;   // skip artificially created classes
   }

   msg("Generating XML output for class %s\n", cd->name().data());

   ti << "  <compound refid=\"" << classOutputFileBase(cd)
      << "\" kind=\"" << cd->compoundTypeString()
      << "\"><name>" << convertToXML(cd->name()) << "</name>" << endl;

   QByteArray outputDirectory = Config_getString("XML_OUTPUT");
   QByteArray fileName = outputDirectory + "/" + classOutputFileBase(cd) + ".xml";
   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", qPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);
 
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
    
      for (auto bcd : *cd->baseClasses()) {
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
         if (!bcd->templSpecifiers.isEmpty()) {
            t << convertToXML(
                 insertTemplateSpecifierInScope(
                    bcd->classDef->name(), bcd->templSpecifiers)
              );
         } else {
            t << convertToXML(bcd->classDef->displayName());
         }
         t  << "</basecompoundref>" << endl;
      }
   }

   if (cd->subClasses()) {
   
      for (auto bcd : *cd->baseClasses()) {
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

   IncludeInfo *ii = cd->includeInfo();

   if (ii) {
      QByteArray nm = ii->includeName;

      if (nm.isEmpty() && ii->fileDef) {
         nm = ii->fileDef->docName();
      }

      if (!nm.isEmpty()) {
         t << "    <includes";
         if (ii->fileDef && !ii->fileDef->isReference()) { // TODO: support external references
            t << " refid=\"" << ii->fileDef->getOutputFileBase() << "\"";
         }
         t << " local=\"" << (ii->local ? "yes" : "no") << "\">";
         t << nm;
         t << "</includes>" << endl;
      }
   }

   writeInnerClasses(cd->getClassSDict(), t);

   writeTemplateList(cd, t);

   if (cd->getMemberGroupSDict()) {     
      for (auto mg : *cd->getMemberGroupSDict() ) {
         generateXMLSection(cd, ti, t, mg->members(), "user-defined", mg->header(), mg->documentation());
      }
   }

   for (auto ml : cd->getMemberLists() ) {
      if ((ml->listType()&MemberListType_detailedLists) == 0) {
         generateXMLSection(cd, ti, t, ml.data(), g_xmlSectionMapper.value(ml->listType()));
      }
   }

   t << "    <briefdescription>" << endl;
   writeXMLDocBlock(t, cd->briefFile(), cd->briefLine(), cd, 0, cd->briefDescription());
   t << "    </briefdescription>" << endl;
   t << "    <detaileddescription>" << endl;
   writeXMLDocBlock(t, cd->docFile(), cd->docLine(), cd, 0, cd->documentation());
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
     << cd->getDefFileName() << "\" line=\""
     << cd->getDefLine() << "\"" << " column=\""
     << cd->getDefColumn() << "\"" ;

   if (cd->getStartBodyLine() != -1) {
      QSharedPointer<FileDef> bodyDef = cd->getBodyDef();

      if (bodyDef) {
         t << " bodyfile=\"" << bodyDef->getFilePath() << "\"";
      }
      t << " bodystart=\"" << cd->getStartBodyLine() << "\" bodyend=\""
        << cd->getEndBodyLine() << "\"";
   }
   t << "/>" << endl;
   writeListOfAllMembers(cd, t);
   t << "  </compounddef>" << endl;
   t << "</doxygen>" << endl;

   ti << "  </compound>" << endl;
}

static void generateXMLForNamespace(NamespaceDef *nd, QTextStream &ti)
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

   QByteArray outputDirectory = Config_getString("XML_OUTPUT");

   QByteArray fileName = outputDirectory + "/" + nd->getOutputFileBase() + ".xml";
   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", qPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);
  
   writeXMLHeader(t);
   t << "  <compounddef id=\"" << nd->getOutputFileBase()
     << "\" kind=\"namespace\" language=\""
     << langToString(nd->getLanguage()) << "\">" << endl;
   t << "    <compoundname>";

   writeXMLString(t, nd->name());
   t << "</compoundname>" << endl;

   writeInnerClasses(nd->getClassSDict(), t);
   writeInnerNamespaces(nd->getNamespaceSDict(), t);

   if (nd->getMemberGroupSDict()) {    
       for (auto mg : *nd->getMemberGroupSDict()) {
         generateXMLSection(nd, ti, t, mg->members(), "user-defined", mg->header(), mg->documentation());
      }
   }
  
   for (auto ml : nd->getMemberLists()) {
      if ((ml->listType()&MemberListType_declarationLists) != 0) {
         generateXMLSection(nd, ti, t, ml.data(), g_xmlSectionMapper.value(ml->listType()));
      }
   }

   t << "    <briefdescription>" << endl;
  
 writeXMLDocBlock(t, nd->briefFile(), nd->briefLine(), nd, 0, nd->briefDescription());
   t << "    </briefdescription>" << endl;
   t << "    <detaileddescription>" << endl;
 
  writeXMLDocBlock(t, nd->docFile(), nd->docLine(), nd, 0, nd->documentation());
   t << "    </detaileddescription>" << endl;
   t << "    <location file=\""
     << nd->getDefFileName() << "\" line=\""
     << nd->getDefLine() << "\"" << " column=\""
     << nd->getDefColumn() << "\"/>" << endl ;
   t << "  </compounddef>" << endl;
   t << "</doxygen>" << endl;

   ti << "  </compound>" << endl;
}

static void generateXMLForFile(FileDef *fd, QTextStream &ti)
{
   // + includes files
   // + includedby files
   // + include graph
   // + included by graph
   // + contained class definitions
   // + contained namespace definitions
   // + member groups
   // + normal members
   // + brief desc
   // + detailed desc
   // + source code
   // + location
   // - number of lines

   if (fd->isReference()) {
      return;   // skip external references
   }

   ti << "  <compound refid=\"" << fd->getOutputFileBase()
      << "\" kind=\"file\"><name>" << convertToXML(fd->name())
      << "</name>" << endl;

   QByteArray outputDirectory = Config_getString("XML_OUTPUT");
   QByteArray fileName = outputDirectory + "/" + fd->getOutputFileBase() + ".xml";
   QFile f(fileName);

   if (!f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", qPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);
   //t.setEncoding(QTextStream::UnicodeUTF8);

   writeXMLHeader(t);
   t << "  <compounddef id=\"" << fd->getOutputFileBase()
     << "\" kind=\"file\" language=\""
     << langToString(fd->getLanguage()) << "\">" << endl;
   t << "    <compoundname>";
   writeXMLString(t, fd->name());
   t << "</compoundname>" << endl;

   IncludeInfo *inc;

   if (fd->includeFileList()) {

      for (auto inc : *fd->includeFileList()) {
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

      for (auto inc : *fd->includedByFileList())   {
         t << "    <includedby";
         if (inc.fileDef && ! inc.fileDef->isReference()) { // TODO: support external references
            t << " refid=\"" << inc.fileDef->getOutputFileBase() << "\"";
         }

         t << " local=\"" << (inc.local ? "yes" : "no") << "\">";
         t << inc.includeName;
         t << "</includedby>" << endl;
      }
   }

   DotInclDepGraph incDepGraph(fd, false);
   if (!incDepGraph.isTrivial()) {
      t << "    <incdepgraph>" << endl;
      incDepGraph.writeXML(t);
      t << "    </incdepgraph>" << endl;
   }

   DotInclDepGraph invIncDepGraph(fd, true);
   if (!invIncDepGraph.isTrivial()) {
      t << "    <invincdepgraph>" << endl;
      invIncDepGraph.writeXML(t);
      t << "    </invincdepgraph>" << endl;
   }

   if (fd->getClassSDict()) {
      writeInnerClasses(fd->getClassSDict(), t);
   }
   if (fd->getNamespaceSDict()) {
      writeInnerNamespaces(fd->getNamespaceSDict(), t);
   }

   if (fd->getMemberGroupSDict()) {     
      for (auto mg : *fd->getMemberGroupSDict() ) {
         generateXMLSection(fd, ti, t, mg->members(), "user-defined", mg->header(), mg->documentation());
      }
   }
 
   for (auto ml : fd->getMemberLists()) {
      if ((ml->listType()&MemberListType_declarationLists) != 0) {
         generateXMLSection(fd, ti, t, ml.data(), g_xmlSectionMapper.value(ml->listType()));
      }
   }

   t << "    <briefdescription>" << endl;

   writeXMLDocBlock(t, fd->briefFile(), fd->briefLine(), fd, 0, fd->briefDescription());
   t << "    </briefdescription>" << endl;
   t << "    <detaileddescription>" << endl;

   writeXMLDocBlock(t, fd->docFile(), fd->docLine(), fd, 0, fd->documentation());
   t << "    </detaileddescription>" << endl;

   if (Config_getBool("XML_PROGRAMLISTING")) {
      t << "    <programlisting>" << endl;
      writeXMLCodeBlock(t, fd);
      t << "    </programlisting>" << endl;
   }
   t << "    <location file=\"" << fd->getDefFileName() << "\"/>" << endl;
   t << "  </compounddef>" << endl;
   t << "</doxygen>" << endl;

   ti << "  </compound>" << endl;
}

static void generateXMLForGroup(GroupDef *gd, QTextStream &ti)
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

   if (gd->isReference()) {
      return;   // skip external references
   }

   ti << "  <compound refid=\"" << gd->getOutputFileBase()
      << "\" kind=\"group\"><name>" << convertToXML(gd->name()) << "</name>" << endl;

   QByteArray outputDirectory = Config_getString("XML_OUTPUT");
   QByteArray fileName = outputDirectory + "/" + gd->getOutputFileBase() + ".xml";

   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
       err("Unable to open file for writing %s, error: %d\n", qPrintable(fileName), f.error());
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

   if (gd->getMemberGroupSDict()) {
     
      for (auto mg : *gd->getMemberGroupSDict()) {
         generateXMLSection(gd, ti, t, mg->members(), "user-defined", mg->header(), mg->documentation());
      }
   }
   
   for (auto ml : gd->getMemberLists()) {
      if ((ml.listType()&MemberListType_declarationLists) != 0) {
         generateXMLSection(gd, ti, t, &ml, g_xmlSectionMapper.value(ml.listType()));
      }
   }

   t << "    <briefdescription>" << endl;

   writeXMLDocBlock(t, gd->briefFile(), gd->briefLine(), gd, 0, gd->briefDescription());
   t << "    </briefdescription>" << endl;
   t << "    <detaileddescription>" << endl;

   writeXMLDocBlock(t, gd->docFile(), gd->docLine(), gd, 0, gd->documentation());
   t << "    </detaileddescription>" << endl;
   t << "  </compounddef>" << endl;
   t << "</doxygen>" << endl;

   ti << "  </compound>" << endl;
}

static void generateXMLForDir(DirDef *dd, QTextStream &ti)
{
   if (dd->isReference()) {
      return;   // skip external references
   }
   ti << "  <compound refid=\"" << dd->getOutputFileBase()
      << "\" kind=\"dir\"><name>" << convertToXML(dd->displayName())
      << "</name>" << endl;

   QByteArray outputDirectory = Config_getString("XML_OUTPUT");
   QByteArray fileName = outputDirectory + "/" + dd->getOutputFileBase() + ".xml";
   QFile f(fileName);

   if (!f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", qPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);
   //t.setEncoding(QTextStream::UnicodeUTF8);
   writeXMLHeader(t);
   t << "  <compounddef id=\""
     << dd->getOutputFileBase() << "\" kind=\"dir\">" << endl;
   t << "    <compoundname>" << convertToXML(dd->displayName()) << "</compoundname>" << endl;

   writeInnerDirs(&dd->subDirs(), t);
   writeInnerFiles(dd->getFiles(), t);

   t << "    <briefdescription>" << endl;
   writeXMLDocBlock(t, dd->briefFile(), dd->briefLine(), dd, 0, dd->briefDescription());
   t << "    </briefdescription>" << endl;
   t << "    <detaileddescription>" << endl;
   writeXMLDocBlock(t, dd->docFile(), dd->docLine(), dd, 0, dd->documentation());
   t << "    </detaileddescription>" << endl;
   t << "    <location file=\"" << dd->name() << "\"/>" << endl;
   t << "  </compounddef>" << endl;
   t << "</doxygen>" << endl;

   ti << "  </compound>" << endl;
}

static void generateXMLForPage(PageDef *pd, QTextStream &ti, bool isExample)
{
   // + name
   // + title
   // + documentation

   const char *kindName = isExample ? "example" : "page";

   if (pd->isReference()) {
      return;
   }

   QByteArray pageName = pd->getOutputFileBase();
   if (pd->getGroupDef()) {
      pageName += (QByteArray)"_" + pd->name();
   }
   if (pageName == "index") {
      pageName = "indexpage";   // to prevent overwriting the generated index page.
   }

   ti << "  <compound refid=\"" << pageName
      << "\" kind=\"" << kindName << "\"><name>" << convertToXML(pd->name())
      << "</name>" << endl;

   QByteArray outputDirectory = Config_getString("XML_OUTPUT");
   QByteArray fileName = outputDirectory + "/" + pageName + ".xml";
   QFile f(fileName);

   if (!f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", qPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);
   //t.setEncoding(QTextStream::UnicodeUTF8);
   writeXMLHeader(t);
   t << "  <compounddef id=\"" << pageName;
   t << "\" kind=\"" << kindName << "\">" << endl;
   t << "    <compoundname>" << convertToXML(pd->name())
     << "</compoundname>" << endl;

   if (pd == Doxygen::mainPage) { // main page is special
      QByteArray title;
      if (!pd->title().isEmpty() && pd->title().toLower() != "notitle") {
         title = filterTitle(convertCharEntitiesToUTF8(Doxygen::mainPage->title()));
      } else {
         title = Config_getString("PROJECT_NAME");
      }
      t << "    <title>" << convertToXML(convertCharEntitiesToUTF8(title))
        << "</title>" << endl;

   } else {
      QSharedPointer<SectionInfo> si = Doxygen::sectionDict->find(pd->name());

      if (si) {
         t << "    <title>" << convertToXML(convertCharEntitiesToUTF8(filterTitle(si->title)))
           << "</title>" << endl;
      }
   }

   writeInnerPages(pd->getSubPages(), t);
   t << "    <detaileddescription>" << endl;
   if (isExample) {
      writeXMLDocBlock(t, pd->docFile(), pd->docLine(), pd, 0, pd->documentation() + "\n\\include " + pd->name());

   } else {
      writeXMLDocBlock(t, pd->docFile(), pd->docLine(), pd, 0,pd->documentation());
   }
   t << "    </detaileddescription>" << endl;

   t << "  </compounddef>" << endl;
   t << "</doxygen>" << endl;

   ti << "  </compound>" << endl;
}

void generateXML()
{
   // + classes
   // + namespaces
   // + files
   // + groups
   // + related pages
   // - examples

   QByteArray outputDirectory = Config_getString("XML_OUTPUT");
   QDir xmlDir(outputDirectory);
   createSubDirs(xmlDir);

   ResourceMgr::instance().copyResourceAs("xml/index.xsd", outputDirectory, "index.xsd");

   QByteArray fileName = outputDirectory + "/compound.xsd";
   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", qPrintable(fileName), f.error());     
      return;
   }

   // write compound.xsd, but replace special marker with the entities
   QByteArray compound_xsd = ResourceMgr::instance().getAsString("xml/compound.xsd");
   const char *startLine = compound_xsd.data();

   while (*startLine) {
      // find end of the line
      const char *endLine = startLine + 1;

      while (*endLine && *(endLine - 1) != '\n') {
         endLine++;   // skip to end of the line including \n
      }

      int len = endLine - startLine;

      if (len > 0) {
         QByteArray s;
         s.resize(len + 1);

         qstrncpy(s.data(), startLine, len);
         s[len] = '\0';

         if (s.indexOf("<!-- Automatically insert here the HTML entities -->") != -1) {
            QTextStream t(&f);
            HtmlEntityMapper::instance()->writeXMLSchema(t);

         } else {
            f.write(startLine, len);
         }
      }
      startLine = endLine;
   }
   f.close();

   //
   fileName = outputDirectory + "/index.xml";
   f.setFileName(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", qPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);  

   // write index header
   t << "<?xml version='1.0' encoding='UTF-8' standalone='no'?>" << endl;;
   t << "<doxygenindex xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ";
   t << "xsi:noNamespaceSchemaLocation=\"index.xsd\" ";
   t << "version=\"" << versionString << "\">" << endl;

       
   for (auto cd : *Doxygen::classSDict) {
       generateXMLForClass(cd.data(), t);
   }
   
   for (auto nd : *Doxygen::namespaceSDict) {
      msg("Generating XML output for namespace %s\n", nd->name().data());
      generateXMLForNamespace(nd.data(), t);
   }
 
   for (auto fn : *Doxygen::inputNameList) {     
      for (auto fd : *fn) {
         msg("Generating XML output for file %s\n", fd->name().data());
         generateXMLForFile(fd, t);
      }
   }
 
   for (auto gd : *Doxygen::groupSDict) {
      msg("Generating XML output for group %s\n", gd->name().data());
      generateXMLForGroup(gd.data(), t);
   } 

   for (auto pd : *Doxygen::pageSDict) {
      msg("Generating XML output for page %s\n", pd->name().data());
      generateXMLForPage(pd.data(), t, false);
   }
    
   for (auto dir : Doxygen::directories) {
      msg("Generate XML output for dir %s\n", dir->name().data());
      generateXMLForDir(dir.data(), t);
   }
    
   for (auto pd : *Doxygen::exampleSDict) {
      msg("Generating XML output for example %s\n", pd->name().data());
      generateXMLForPage(pd.data(), t, true);
   }
   
   if (Doxygen::mainPage) {
      msg("Generating XML output for the main page\n");
      generateXMLForPage(Doxygen::mainPage, t, false);
   }

   //t << "  </compoundlist>" << endl;
   t << "</doxygenindex>" << endl;

   writeCombineScript();
}
