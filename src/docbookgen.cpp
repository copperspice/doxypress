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
#include <docbookgen.h>
#include <doxygen.h>
#include <message.h>
#include <config.h>
#include <classlist.h>
#include <util.h>
#include <defargs.h>
#include <outputgen.h>
#include <dot.h>
#include <pagedef.h>
#include <filename.h>
#include <dox_build_info.h>
#include <docbookvisitor.h>
#include <docparser.h>
#include <language.h>
#include <parserintf.h>
#include <arguments.h>
#include <memberlist.h>
#include <groupdef.h>
#include <memberdef.h>
#include <namespacedef.h>
#include <membername.h>
#include <membergroup.h>
#include <dirdef.h>
#include <section.h>

// no debug info
#define Docbook_DB(x) do {} while(0)

// debug to stdout
// #define Docbook_DB(x) printf x

// debug inside output
// #define Docbook_DB(x) QByteArray __t;__t.sprintf x;m_t << __t


class DocbookSectionMapper : public QHash<long, QByteArray>
{
 public:
   DocbookSectionMapper() : QHash<long, QByteArray>() {
      insert(MemberListType_pubTypes, "public-type");
      insert(MemberListType_pubMethods, "public-func");
      insert(MemberListType_pubAttribs, "public-attrib");
      insert(MemberListType_pubSlots, "public-slot");
      insert(MemberListType_signals, "signal");
      insert(MemberListType_dcopMethods, "dcop-func");
      insert(MemberListType_properties, "property");
      insert(MemberListType_events, "event");
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

static DocbookSectionMapper g_docbookSectionMapper;


inline void writeDocbookString(FTextStream &t, const char *s)
{
   t << convertToXML(s);
}

inline void writeDocbookCodeString(FTextStream &t, const char *s, int &col)
{
   char c;
   while ((c = *s++)) {
      switch (c) {
         case '\t': {
            static int tabSize = Config_getInt("TAB_SIZE");
            int spacesToNextTabStop = tabSize - (col % tabSize);
            col += spacesToNextTabStop;
            while (spacesToNextTabStop--) {
               t << "&#32;";
            }
            break;
         }
         case ' ':
            t << "&#32;";
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
         default:
            t << c;
            col++;
            break;
      }
   }
}

static void writeDocbookHeaderMainpage(FTextStream &t)
{
   t << "<?xml version='1.0' encoding='UTF-8' standalone='no'?>" << endl;;
   t << "<chapter xmlns=\"http://docbook.org/ns/docbook\" version=\"5.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">" << endl;
}

static void writeDocbookHeader_ID(FTextStream &t, QByteArray id)
{
   t << "<?xml version='1.0' encoding='UTF-8' standalone='no'?>" << endl;;
   t << "<section xmlns=\"http://docbook.org/ns/docbook\" version=\"5.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xml:id=\"" << id << "\">" << endl;
}

void writeDocbookLink(FTextStream &t, const char * /*extRef*/, const char *compoundId,
                      const char *anchorId, const char *text, const char * /*tooltip*/)
{
   t << "<link linkend=\"" << compoundId;
   if (anchorId) {
      t << "_1" << anchorId;
   }
   t << "\"";
   t << ">";
   writeDocbookString(t, text);
   t << "</link>";
}

class TextGeneratorDocbookImpl : public TextGeneratorIntf
{
 public:
   TextGeneratorDocbookImpl(FTextStream &t): m_t(t) {}
   void writeString(const char *s, bool /*keepSpaces*/) const {
      writeDocbookString(m_t, s);
   }
   void writeBreak(int) const {}
   void writeLink(const char *extRef, const char *file,
                  const char *anchor, const char *text
                 ) const {
      writeDocbookLink(m_t, extRef, file, anchor, text, 0);
   }
 private:
   FTextStream &m_t;
};

class DocbookCodeGenerator : public CodeOutputInterface
{
 public:
   DocbookCodeGenerator(FTextStream &t) : m_t(t), m_lineNumber(-1), m_col(0),
      m_insideCodeLine(false), m_insideSpecialHL(false) {}
   virtual ~DocbookCodeGenerator() {}

   void codify(const char *text) {
      Docbook_DB(("(codify \"%s\")\n", text));
      writeDocbookCodeString(m_t, text, m_col);
   }
   void writeCodeLink(const char *ref, const char *file,
                      const char *anchor, const char *name,
                      const char *tooltip) {
      Docbook_DB(("(writeCodeLink)\n"));
      writeDocbookLink(m_t, ref, file, anchor, name, tooltip);
      m_col += strlen(name);
   }
   void writeTooltip(const char *, const DocLinkInfo &, const char *,
                     const char *, const SourceLinkInfo &, const SourceLinkInfo &
                    ) {
      Docbook_DB(("(writeToolTip)\n"));
   }
   void startCodeLine(bool) {
      Docbook_DB(("(startCodeLine)\n"));
      if (m_lineNumber != -1) {
         if (!m_refId.isEmpty()) {
            m_t << "<link linkend=\"" << m_refId << "\">";
         }
         m_t << m_lineNumber << " ";
         if (!m_refId.isEmpty()) {
            m_t << "</link>";
         }
      }
      m_insideCodeLine = true;
      m_col = 0;
   }
   void endCodeLine() {
      m_t << endl;
      Docbook_DB(("(endCodeLine)\n"));
      m_lineNumber = -1;
      m_refId.resize(0);
      m_external.resize(0);
      m_insideCodeLine = false;
   }
   void startFontClass(const char *colorClass) {
      Docbook_DB(("(startFontClass)\n"));
      m_t << "<emphasis class=\"" << colorClass << "\">";
      m_insideSpecialHL = true;
   }
   void endFontClass() {
      Docbook_DB(("(endFontClass)\n"));
      m_t << "</emphasis>"; // non DocBook
      m_insideSpecialHL = false;
   }
   void writeCodeAnchor(const char *) {
      Docbook_DB(("(writeCodeAnchor)\n"));
   }
   void writeLineNumber(const char *extRef, const char *compId,
                        const char *anchorId, int l) {
      Docbook_DB(("(writeLineNumber)\n"));
      // we remember the information provided here to use it
      // at the <codeline> start tag.
      m_lineNumber = l;
      if (compId) {
         m_refId = compId;
         if (anchorId) {
            m_refId += (QByteArray)"_1" + anchorId;
         }
         if (extRef) {
            m_external = extRef;
         }
      }
   }
   void setCurrentDoc(Definition *, const char *, bool) {
   }
   void addWord(const char *, bool) {
   }
   void finish() {
      if (m_insideCodeLine) {
         endCodeLine();
      }
   }

 private:
   FTextStream &m_t;
   QByteArray m_refId;
   QByteArray m_external;
   int m_lineNumber;
   int m_col;
   bool m_insideCodeLine;
   bool m_insideSpecialHL;
};

static void writeTemplateArgumentList(ArgumentList *al, FTextStream &t, Definition *scope, FileDef *fileScope, int indent)
{
   QByteArray indentStr;
   indentStr.fill(' ', indent);

   if (al) {
      t << indentStr << "<templateparamlist>" << endl;

      for (auto item : *al) {
         t << indentStr << "  <param>" << endl;

         if (! item.type.isEmpty()) {
            t << indentStr <<  "    <type>";
            linkifyText(TextGeneratorDocbookImpl(t), scope, fileScope, 0, item.type);
            t << "</type>" << endl;
         }

         if (! item.name.isEmpty()) {
            t << indentStr <<  "    <declname>" << item.name << "</declname>" << endl;
            t << indentStr <<  "    <defname>" << item.name << "</defname>" << endl;
         }

         if (! item.defval.isEmpty()) {
            t << indentStr << "    <defval>";
            linkifyText(TextGeneratorDocbookImpl(t), scope, fileScope, 0, item.defval);
            t << "</defval>" << endl;
         }

         t << indentStr << "  </param>" << endl;
      }

      t << indentStr << "</templateparamlist>" << endl;
   }
}

static void writeTemplateList(ClassDef *cd, FTextStream &t)
{
   writeTemplateArgumentList(cd->templateArguments(), t, cd, 0, 4);
}

static void writeDocbookDocBlock(FTextStream &t, const QByteArray &fileName, int lineNr, Definition *scope, MemberDef *md, const QByteArray &text)
{
   QByteArray stext = text.trimmed();
   if (stext.isEmpty()) {
      return;
   }

   // convert the documentation string into an abstract syntax tree
   DocNode *root = validatingParseDoc(fileName, lineNr, scope, md, text, false, false);

   // create a code generator
   DocbookCodeGenerator *docbookCodeGen = new DocbookCodeGenerator(t);

   // create a parse tree visitor for Docbook
   DocbookDocVisitor *visitor = new DocbookDocVisitor(t, *docbookCodeGen);

   // visit all nodes
   root->accept(visitor);

   // clean up
   delete visitor;
   delete docbookCodeGen;
   delete root;
}

void writeDocbookCodeBlock(FTextStream &t, FileDef *fd)
{
   ParserInterface *pIntf = Doxygen::parserManager->getParser(fd->getDefFileExtension());
   SrcLangExt langExt = getLanguageFromFileName(fd->getDefFileExtension());
   pIntf->resetCodeParserState();

   DocbookCodeGenerator *docbookGen = new DocbookCodeGenerator(t);

   pIntf->parseCode(*docbookGen,  // codeOutIntf
                    0,           // scopeName
                    fileToString(fd->absFilePath(), Config_getBool("FILTER_SOURCE_FILES")),
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
   docbookGen->finish();
   delete docbookGen;
}

static QByteArray classOutputFileBase(ClassDef *cd)
{
   //static bool inlineGroupedClasses = Config_getBool("INLINE_GROUPED_CLASSES");
   //if (inlineGroupedClasses && cd->partOfGroups()!=0)

   return cd->getOutputFileBase();

   //else
   //  return cd->getOutputFileBase();
}

static QByteArray memberOutputFileBase(MemberDef *md)
{
   //static bool inlineGroupedClasses = Config_getBool("INLINE_GROUPED_CLASSES");
   //if (inlineGroupedClasses && md->getClassDef() && md->getClassDef()->partOfGroups()!=0)
   //  return md->getClassDef()->getDocbookOutputFileBase();
   //else
   //  return md->getOutputFileBase();

   return md->getOutputFileBase();
}


static void generateDocbookForMember(MemberDef *md, FTextStream &t, Definition *def, bool detailed = 0)
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
   //if (md->name().at(0)=='@') return; // anonymous member

   // group members are only visible in their group
   //if (def->definitionType()!=Definition::TypeGroup && md->getGroupDef()) return;

   QByteArray memType;

   switch (md->memberType()) {
      case MemberType_Define:
         memType = "define";
         break;
      case MemberType_Function:
         memType = "function";
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
         break;
      case MemberType_Slot:
         memType = "slot";
         break;
      case MemberType_Friend:
         memType = "friend";
         break;
      case MemberType_DCOP:
         memType = "dcop";
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
   QByteArray scopeName;
   if (md->getClassDef()) {
      scopeName = md->getClassDef()->name();
   } else if (md->getNamespaceDef()) {
      scopeName = md->getNamespaceDef()->name();
   }
   if (detailed == 0) {
      t << "            <para>" << endl;
      t << "                <itemizedlist>" << endl;
      t << "                    <listitem>" << endl;

      //enum
      bool closePara = true;
      if (md->memberType() == MemberType_Enumeration) {
         MemberList *enumFields = md->enumFieldList();

         t << "                            <para><literallayout>" << memType << " <link linkend=\"";

         if (md->getGroupDef() && def->definitionType() == Definition::TypeGroup) {
            t << md->getGroupDef()->getOutputFileBase();
         } else {
            t << memberOutputFileBase(md);
         }

         t << "_1" << md->anchor() << "\">" << convertToXML(md->name()) << "</link>";

         if (enumFields != 0) {
           
            t << " {" << endl;
            int cnt = 0;

            for (auto item: *enumFields) {
               if (cnt != 0) {
                  t << "," << endl;
               }

               t << "<link linkend=\"" << memberOutputFileBase(item) << "_1" << item.anchor() << "\">";
               writeDocbookString(t, item.name());

               t << "</link>";
               if (! item.initializer().isEmpty()) {
                  writeDocbookString(t, item->initializer());
               }

               cnt++;
            }
            t << endl << "}";
            t << "</literallayout>" << endl;
         }

      } else if (md->memberType() == MemberType_Define) {
         t << "                            <para>" << "#" << memType << " <link linkend=\"";

         if (md->getGroupDef() && def->definitionType() == Definition::TypeGroup) {
            t << md->getGroupDef()->getOutputFileBase();
         } else {
            t << memberOutputFileBase(md);
         }

         t << "_1" << md->anchor() << "\">" << convertToXML(md->name()) << "</link>";

         if (!md->initializer().isEmpty() && md->initializer().length() < 2000) {
            t << " ";
            linkifyText(TextGeneratorDocbookImpl(t), def, md->getBodyDef(), md, md->initializer());
         }

         if (md->briefDescription()) {
            t << "<para><emphasis>";
            writeDocbookString(t, md->briefDescription());
            t << "</emphasis></para>" << endl;
         }

      } else if (md->memberType() == MemberType_Variable) {
         if (md->getClassDef()) {
            t << "                        <para>" << convertToXML(md->declaration());
            if (md->briefDescription()) {
               t << "<para><emphasis>";
               writeDocbookString(t, md->briefDescription());
               t << "</emphasis></para>";
            }
         } else {
            t << "                        <para>";
            linkifyText(TextGeneratorDocbookImpl(t), def, md->getBodyDef(), md, md->typeString());
            t << " <link linkend=\"";
            if (md->getGroupDef() && def->definitionType() == Definition::TypeGroup) {
               t << md->getGroupDef()->getOutputFileBase();
            } else {
               t << memberOutputFileBase(md);
            }
            t << "_1" << md->anchor() << "\">" << convertToXML(md->name()) << "</link>";
         }

      } else if (md->memberType() == MemberType_Typedef) {
         t << "                            <para>" << memType;
         t << " ";
         linkifyText(TextGeneratorDocbookImpl(t), def, md->getBodyDef(), md, md->typeString());
         t << " ";
         t << " <link linkend=\"";
         if (md->getGroupDef() && def->definitionType() == Definition::TypeGroup) {
            t << md->getGroupDef()->getOutputFileBase();
         } else {
            t << memberOutputFileBase(md);
         }
         t << "_1" << md->anchor() << "\">" << convertToXML(md->name()) << "</link>";

      } else if (md->memberType() == MemberType_Function) {
         t << "                        <para>";
         linkifyText(TextGeneratorDocbookImpl(t), def, md->getBodyDef(), md, md->typeString());
         t << " <link linkend=\"";
         if (md->getGroupDef() && def->definitionType() == Definition::TypeGroup) {
            t << md->getGroupDef()->getOutputFileBase();
         } else {
            t << memberOutputFileBase(md);
         }
         t << "_1" << md->anchor() << "\">" << convertToXML(md->name()) << "</link>";
         t << " (" << endl;

         ArgumentList *declAl = md->declArgumentList();
         ArgumentList *defAl = md->argumentList();

         if (declAl && declAl->count() > 0) {

            int cnt = 0;
            ArgumentListIterator defAli(*defAl);

/*          BROOM
            ArgumentListIterator declAli(*declAl);            
            Argument *a;           
            for (declAli.toFirst(); (a = declAli.current()); ++declAli) {
*/

            for (auto a : *ali) {
               if (cnt != 0) {
                  t << ", ";
               }

               if (!a->type.isEmpty()) {
                  linkifyText(TextGeneratorDocbookImpl(t), def, md->getBodyDef(), md, a->type);
               }

               t << " ";

               if (!a->name.isEmpty()) {
                  writeDocbookString(t, a->name);
               }
               cnt++;
            }
         }

         t << ")";

      } else {
         closePara = false;
      }
      if (closePara) {
         t << "</para>" << endl;
      }
      t << "                    </listitem>" << endl;
      t << "                </itemizedlist>" << endl;
      t << "            </para>" << endl;
   } else {
      if (md->memberType() == MemberType_Enumeration) {
         MemberList *enumFields = md->enumFieldList();
         t << "            <section xml:id=\"";
         if (md->getGroupDef() && def->definitionType() == Definition::TypeGroup) {
            t << md->getGroupDef()->getOutputFileBase();
         } else {
            t << memberOutputFileBase(md);
         }
         t << "_1" << md->anchor() << "\">" << endl;
         t << "               <title>" << memType << " " << convertToXML(md->name()) << " " << "</title>" << endl;
         t << "               ";
         writeDocbookDocBlock(t, md->docFile(), md->docLine(), md->getOuterScope(), md, md->documentation());
         t << endl;
         if (enumFields != 0) {
            QListIterator<MemberDef> emli(*enumFields);
            MemberDef *emd;
            t << "               <formalpara>" << endl;
            t << "                    <title>Enumerator:</title>" << endl;
            t << "                    <variablelist>" << endl;
            for (emli.toFirst(); (emd = emli.current()); ++emli) {
               t << "                        <varlistentry xml:id=\"";
               t << memberOutputFileBase(emd) << "_1" << emd->anchor() << "\">" << endl;
               t << "                            <term>";
               writeDocbookString(t, emd->name());
               t << "</term>" << endl;
               t << "                            <listitem>" << endl;
               t << "                                <para>";
               writeDocbookString(t, emd->briefDescription());
               t << "</para>" << endl;
               t << "                            </listitem>" << endl;
               t << "                        </varlistentry>" << endl;
            }
            t << "                     </variablelist>" << endl;
            t << "                </formalpara>" << endl;
            t << "                <para>";
            t << "Definition at line " << md->getDefLine() << " of file " << stripPath(md->getDefFileName()) << endl;
            t << "                    <computeroutput><literallayout>" << endl;
            t << "{" << endl;
            for (emli.toFirst(); (emd = emli.current()); ++emli) {
               writeDocbookString(t, emd->name());
               if (!emd->initializer().isEmpty()) {
                  writeDocbookString(t, emd->initializer());
               }
               t << ", " << endl;
            }
            t << "}" << convertToXML(md->name()) << ";" << endl;
            t << "                    </literallayout></computeroutput>" << endl;
            t << "                </para>" << endl;
            t << "            </section>" << endl;
         }
      } else if (md->memberType() == MemberType_Typedef) {
         t << "            <section xml:id=\"";
         if (md->getGroupDef() && def->definitionType() == Definition::TypeGroup) {
            t << md->getGroupDef()->getOutputFileBase();
         } else {
            t << memberOutputFileBase(md);
         }
         t << "_1" << md->anchor() << "\">" << endl;
         t << "                <title>" << convertToXML(md->definition()) << "</title>";
         t << " <emphasis>";
         writeDocbookString(t, md->briefDescription());
         t << "</emphasis>" << endl;
         t << "                ";
         writeDocbookDocBlock(t, md->docFile(), md->docLine(), md->getOuterScope(), md, md->documentation());
         t << endl;
         t << "            </section>" << endl;
      } else if (md->memberType() == MemberType_Function) {
         t << "            <section xml:id=\"";
         if (md->getGroupDef() && def->definitionType() == Definition::TypeGroup) {
            t << md->getGroupDef()->getOutputFileBase();
         } else {
            t << memberOutputFileBase(md);
         }
         t << "_1" << md->anchor() << "\">" << endl;
         t << "                <title>" << convertToXML(md->definition()) << " " << convertToXML(md->argsString()) << "</title>";
         t << " <emphasis>";
         writeDocbookString(t, md->briefDescription());
         t << "</emphasis>" << endl;
         t << "                ";
         writeDocbookDocBlock(t, md->docFile(), md->docLine(), md->getOuterScope(), md, md->documentation());
         t << endl;
         t << "            </section>" << endl;
      } else if (md->memberType() == MemberType_Define) {
         if (md->documentation()) {
            t << "            <section xml:id=\"";
            if (md->getGroupDef() && def->definitionType() == Definition::TypeGroup) {
               t << md->getGroupDef()->getOutputFileBase();
            } else {
               t << memberOutputFileBase(md);
            }
            t << "_1" << md->anchor() << "\">" << endl;
            t << "                <title>" << convertToXML(md->definition()) << "</title>";
            t << "                ";
            writeDocbookDocBlock(t, md->docFile(), md->docLine(), md->getOuterScope(), md, md->documentation());
            t << endl;
            t << "                <para>Definition at line " << md->getDefLine() << " of file " << stripPath(md->getDefFileName()) << "</para>" << endl;
            t << "                <para>The Documentation for this define was generated from the following file: </para>" << endl;
            t << "                <para><itemizedlist><listitem><para>" << stripPath(md->getDefFileName()) << "</para></listitem></itemizedlist></para>" << endl;
            t << "            </section>" << endl;
         }
      } else if (md->memberType() == MemberType_Variable) {
         if (md->getClassDef()) {
            if (md->documentation()) {
               t << "            <simplesect>" << endl;
               t << "                <title>" << convertToXML(md->definition()) << "</title>";
               t << "                ";
               writeDocbookDocBlock(t, md->docFile(), md->docLine(), md->getOuterScope(), md, md->documentation());
               t << endl;
               t << "                <para>Definition at line " << md->getDefLine() << " of file " << stripPath(md->getDefFileName()) << "</para>" << endl;
               t << "                <para>The Documentation for this struct was generated from the following file: </para>" << endl;
               t << "                <para><itemizedlist><listitem><para>" << stripPath(md->getDefFileName()) << "</para></listitem></itemizedlist></para>" << endl;
               t << "            </simplesect>" << endl;
            }
         } else {
            t << "            <section xml:id=\"";
            if (md->getGroupDef() && def->definitionType() == Definition::TypeGroup) {
               t << md->getGroupDef()->getOutputFileBase();
            } else {
               t << memberOutputFileBase(md);
            }
            t << "_1" << md->anchor() << "\">" << endl;
            t << "                <title>" << convertToXML(md->definition()) << "</title>";
            t << " <emphasis>";
            writeDocbookString(t, md->briefDescription());
            t << "</emphasis>" << endl;
            t << "                ";
            writeDocbookDocBlock(t, md->docFile(), md->docLine(), md->getOuterScope(), md, md->documentation());
            t << endl;
            t << "            </section>" << endl;
         }
      }
   }
}

static void generateDocbookSection(Definition *d, FTextStream &t, MemberList *ml, const char *kind,
                                   bool detailed = 0, const char *header = 0, const char *documentation = 0)
{
   if (ml == 0) {
      return;
   }
   QListIterator<MemberDef> mli(*ml);
   MemberDef *md;
   int count = 0;
   int doc_count = 0;
   QByteArray compkind = kind;
   QByteArray title, desctitle;

   for (mli.toFirst(); (md = mli.current()); ++mli) {
      // namespace members are also inserted in the file scope, but
      // to prevent this duplication in the Docbook output, we filter those here.
      if (d->definitionType() != Definition::TypeFile || md->getNamespaceDef() == 0) {
         count++;
      }
   }

   switch (ml->listType()) {
      case MemberListType_decDefineMembers:
         title = "Defines";
         desctitle = "Define Documentation";
         break;
      case MemberListType_decTypedefMembers:
         title = "Typedefs";
         desctitle = "Typedef Documentation";
         break;
      case MemberListType_decEnumMembers:
         title = "Enumerations";
         desctitle = "Enumeration Type documentation";
         break;
      case MemberListType_decFuncMembers:
         title = "Functions";
         desctitle = "Function Documentation";
         break;
      case MemberListType_decVarMembers:
         title = "Variables";
         desctitle = "Variable Documentation";
         break;
      case MemberListType_pubAttribs:
         title = "Public Attributes";
         desctitle = "Member Documentation";
         break;
      case MemberListType_priAttribs:
         title = "Private Attributes";
         desctitle = "Member Documentation";
         break;
      case MemberListType_proAttribs:
         title = "Protected Attributes";
         desctitle = "Member Documentation";
         break;
      default:
         title = "";
         desctitle = "";
         break;
   }

   if (count == 0) {
      return;   // empty list
   }

   for (mli.toFirst(); (md = mli.current()); ++mli) {
      if (md->documentation()) {
         doc_count++;
      }
   }

   if (detailed) {
      if (desctitle) {
         if (desctitle == "Member Documentation") {
            if (doc_count > 0) {
               t << "        <simplesect>" << endl;
               t << "            <title>" << desctitle << "</title>" << endl;
            }
         } else if (desctitle == "Define Documentation") {
            if (doc_count > 0) {
               t << "        <section>" << endl;
               t << "            <title>" << desctitle << "</title>" << endl;
            }
         } else {
            t << "        <section>" << endl;
            t << "            <title>" << desctitle << "</title>" << endl;
         }
      }
   } else {
      t << "        <simplesect>" << endl;
      if (header) {
         t << "            <title>" << convertToXML(header) << "</title>" << endl;
      } else {
         t << "            <title>" << title << "</title>" << endl;
      }
   }

   if (documentation) {
      t << "      <description>";
      writeDocbookDocBlock(t, d->docFile(), d->docLine(), d, 0, documentation);
      t << "</description>" << endl;
   }
   for (mli.toFirst(); (md = mli.current()); ++mli) {
      // namespace members are also inserted in the file scope, but
      // to prevent this duplication in the Docbook output, we filter those here.
      if (d->definitionType() != Definition::TypeFile || md->getNamespaceDef() == 0) {
         generateDocbookForMember(md, t, d, detailed);
      }
   }
   if (detailed) {
      if (desctitle) {
         if (desctitle == "Member Documentation") {
            if (doc_count > 0) {
               t << "        </simplesect>" << endl;
            }
         } else if (desctitle == "Define Documentation") {
            if (doc_count > 0) {
               t << "        </section>" << endl;
            }
         } else {
            t << "        </section>" << endl;
         }
      }
   } else {
      t << "        </simplesect>" << endl;
   }
}

static void writeInnerClasses(const ClassSDict *cl, FTextStream &t)
{
   if (cl) {
      ClassSDict::Iterator cli(*cl);
      ClassDef *cd;
      QByteArray title = "Classes";

      if (cli.toFirst()) {
         t << "        <simplesect>" << endl;
         t << "            <title> " << title << " </title>" << endl;
      }
      for (cli.toFirst(); (cd = cli.current()); ++cli) {
         if (!cd->isHidden() && cd->name().find('@') == -1) {
            t << "            <para>" << endl;
            t << "                <itemizedlist>" << endl;
            t << "                    <listitem>" << endl;
            t << "                        <para>" << "struct <link linkend=\"" << classOutputFileBase(cd) << "\">" << convertToXML(cd->name()) << "</link>";
            t << "</para>" << endl;
            t << "                    </listitem>" << endl;
            t << "                </itemizedlist>" << endl;
            t << "            </para>" << endl;
         }
      }
      if (cli.toFirst()) {
         t << "        </simplesect>" << endl;
      }
   }
}

static void writeInnerNamespaces(const NamespaceSDict *nl, FTextStream &t)
{
   if (nl) {
      NamespaceSDict::Iterator nli(*nl);
      NamespaceDef *nd;
      QByteArray title = "Namespaces";

      if (nli.toFirst()) {
         t << "        <simplesect>" << endl;
         t << "            <title> " << title << " </title>" << endl;
      }
      for (nli.toFirst(); (nd = nli.current()); ++nli) {
         if (!nd->isHidden() && nd->name().find('@') == -1) { // skip anonymouse scopes
            t << "            <para>" << endl;
            t << "                <itemizedlist>" << endl;
            t << "                    <listitem>" << endl;
            t << "                        <para>" << "struct <link linkend=\"" << nd->getOutputFileBase() << "\">" << convertToXML(nd->name()) << "</link>";
            t << "</para>" << endl;
            t << "                    </listitem>" << endl;
            t << "                </itemizedlist>" << endl;
            t << "            </para>" << endl;
         }
      }
      if (nli.toFirst()) {
         t << "        </simplesect>" << endl;
      }
   }
}

static void writeInnerFiles(const FileList *fl, FTextStream &t)
{
   if (fl) {
      QListIterator<FileDef> fli(*fl);
      FileDef *fd;
      QByteArray title = "Files";

      if (fli.toFirst()) {
         t << "        <simplesect>" << endl;
         t << "            <title> " << title << " </title>" << endl;
      }
      for (fli.toFirst(); (fd = fli.current()); ++fli) {
         t << "            <para>" << endl;
         t << "                <itemizedlist>" << endl;
         t << "                    <listitem>" << endl;
         t << "                        <para>" << "file <link linkend=\"" << fd->getOutputFileBase() << "\">" << convertToXML(fd->name()) << "</link>";
         t << "</para>" << endl;
         t << "                    </listitem>" << endl;
         t << "                </itemizedlist>" << endl;
         t << "            </para>" << endl;
      }
      if (fli.toFirst()) {
         t << "        </simplesect>" << endl;
      }
   }
}

static void writeInnerPages(const PageSDict *pl, FTextStream &t)
{
   if (pl) {
      PageSDict::Iterator pli(*pl);
      PageDef *pd;

      for (pli.toFirst(); (pd = pli.current()); ++pli) {
         t << "<xi:include href=\"" << pd->getOutputFileBase() << ".xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>"  << endl;
      }
   }
}

static void writeInnerGroups(const GroupList *gl, FTextStream &t)
{
   if (gl) {
      GroupListIterator gli(*gl);
      GroupDef *sgd;

      //Docbook header tags for inner groups
      if (gli.toFirst()) {
         t << "    <simplesect>" << endl;
         t << "        <title>Modules</title>" << endl;
         t << "    </simplesect>" << endl;
         t << "    <para>" << endl;
         t << "        <itemizedlist>" << endl;
      }

      for (gli.toFirst(); (sgd = gli.current()); ++gli) {
         t << "            <listitem><para><link linkend=\"" << sgd->getOutputFileBase() << "\">" << convertToXML(
              sgd->groupTitle()) << "</link></para></listitem>" << endl;
      }

      //Docbook footer tags for inner groups
      if (gli.toFirst()) {
         t << "        </itemizedlist>" << endl;
         t << "    </para>" << endl;
      }

   }
}

static void writeInnerDirs(const DirList *dl, FTextStream &t)
{
   if (dl) {
      QListIterator<DirDef> subdirs(*dl);
      DirDef *subdir;
      QByteArray title = "Directories";
      if (subdirs.toFirst()) {
         t << "        <simplesect>" << endl;
         t << "            <title> " << title << " </title>" << endl;
      }
      for (subdirs.toFirst(); (subdir = subdirs.current()); ++subdirs) {
         t << "            <para>" << endl;
         t << "                <itemizedlist>" << endl;
         t << "                    <listitem>" << endl;
         t << "                        <para>" << "dir <link linkend=\"" << subdir->getOutputFileBase() << "\">" << convertToXML(
              subdir->displayName()) << "</link>";
         t << "</para>" << endl;
         t << "                    </listitem>" << endl;
         t << "                </itemizedlist>" << endl;
         t << "            </para>" << endl;
      }
      if (subdirs.toFirst()) {
         t << "        </simplesect>" << endl;
      }
   }
}

static void writeInnerGroupFiles(const GroupList *gl, FTextStream &t)
{
   if (gl) {
      GroupListIterator gli(*gl);
      GroupDef *sgd;

      for (gli.toFirst(); (sgd = gli.current()); ++gli) {
         t << "<xi:include href=\"" << sgd->getOutputFileBase() << ".xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>"  << endl;
      }
   }
}

static void generateDocbookForClass(ClassDef *cd, FTextStream &ti)
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
   if (cd->name().find('@') != -1) {
      return;   // skip anonymous compounds.
   }
   if (cd->templateMaster() != 0) {
      return;   // skip generated template instances.
   }

   msg("Generating Docbook output for class %s\n", cd->name().data());

   QByteArray fileDocbook = cd->getOutputFileBase() + ".xml";
   //Add the file Documentation info to index file
   ti << "        <xi:include href=\"" << fileDocbook << "\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;

   QByteArray outputDirectory = Config_getString("DOCBOOK_OUTPUT");
   QByteArray fileName = outputDirectory + "/" + classOutputFileBase(cd) + ".xml";
   QByteArray relPath = relativePathToRoot(fileName);
   QFile f(fileName);
   if (!f.open(QIODevice::WriteOnly)) {
      err("Cannot open file %s for writing!\n", fileName.data());
      return;
   }
   FTextStream t(&f);
   //t.setEncoding(FTextStream::UnicodeUTF8);

   writeDocbookHeader_ID(t, classOutputFileBase(cd));
   t << "<title>";
   writeDocbookString(t, cd->name());
   t << " " << cd->compoundTypeString() << " Reference";
   t << "</title>" << endl;

   IncludeInfo *ii = cd->includeInfo();
   if (ii) {
      QByteArray nm = ii->includeName;
      if (nm.isEmpty() && ii->fileDef) {
         nm = ii->fileDef->docName();
      }
      if (!nm.isEmpty()) {
         t << "<para>" << endl;
         t << "    <programlisting>#include ";
         if (ii->fileDef && !ii->fileDef->isReference()) { // TODO: support external references
            t << "<link linkend=\"" << ii->fileDef->getOutputFileBase() << "\">";
         }
         if (ii->local) {
            t << "&quot;";
         } else {
            t << "&lt;";
         }
         t << convertToXML(nm);
         if (ii->local) {
            t << "&quot;";
         } else {
            t << "&gt;";
         }
         if (ii->fileDef && !ii->fileDef->isReference()) {
            t << "</link>";
         }
         t << "</programlisting>" << endl;
         t << "</para>" << endl;
      }
   }

   if (Config_getBool("HAVE_DOT") && (Config_getBool("CLASS_DIAGRAMS") || Config_getBool("CLASS_GRAPH"))) {
      t << "<para>Inheritance diagram for " << convertToXML(cd->name()) << "</para>" << endl;
      DotClassGraph inheritanceGraph(cd, DotNode::Inheritance);
      inheritanceGraph.writeGraph(t, GOF_BITMAP, EOF_DocBook, Config_getString("DOCBOOK_OUTPUT"), fileName, relPath, true, false);
   }

   if (Config_getBool("HAVE_DOT") && Config_getBool("COLLABORATION_GRAPH")) {
      t << "<para>Collaboration diagram for " << convertToXML(cd->name()) << "</para>" << endl;
      DotClassGraph collaborationGraph(cd, DotNode::Collaboration);
      collaborationGraph.writeGraph(t, GOF_BITMAP, EOF_DocBook, Config_getString("DOCBOOK_OUTPUT"), fileName, relPath, true, false);
   }

   writeInnerClasses(cd->getClassSDict(), t);

   writeTemplateList(cd, t);
   if (cd->getMemberGroupSDict()) {
      MemberGroupSDict::Iterator mgli(*cd->getMemberGroupSDict());
      MemberGroup *mg;
      for (; (mg = mgli.current()); ++mgli) {
         generateDocbookSection(cd, t, mg->members(), "user-defined", 0, mg->header(), mg->documentation());
      }
   }


   QListIterator<MemberList> mli(cd->getMemberLists());
   MemberList *ml;
   for (mli.toFirst(); (ml = mli.current()); ++mli) {
      if ((ml->listType()&MemberListType_detailedLists) == 0) {
         generateDocbookSection(cd, t, ml, g_docbookSectionMapper.find(ml->listType()));
      }
   }

   if (cd->briefDescription()) {
      t << "    <simplesect>" << endl;
      t << "        <title>Brief Description</title>" << endl;
      writeDocbookDocBlock(t, cd->briefFile(), cd->briefLine(), cd, 0, cd->briefDescription());
      t << "    </simplesect>" << endl;
   }

   if (cd->documentation()) {
      t << "        <simplesect>" << endl;
      t << "            <title>Detailed Description</title>" << endl;
      writeDocbookDocBlock(t, cd->docFile(), cd->docLine(), cd, 0, cd->documentation());
      t << "                <para>Definition at line " << cd->getDefLine() << " of file " << stripPath(cd->getDefFileName()) << "</para>" << endl;
      t << "                <para>The Documentation for this struct was generated from the following file: </para>" << endl;
      t << "                <para><itemizedlist><listitem><para>" << stripPath(cd->getDefFileName()) << "</para></listitem></itemizedlist></para>" << endl;
      t << "        </simplesect>" << endl;
   }
   for (mli.toFirst(); (ml = mli.current()); ++mli) {
      if ((ml->listType()&MemberListType_detailedLists) == 0) {
         generateDocbookSection(cd, t, ml, g_docbookSectionMapper.find(ml->listType()), 1);
      }
   }

   /*// TODO: Handling of Inheritance and Colloboration graph for Docbook to be implemented
     DotClassGraph inheritanceGraph(cd,DotNode::Inheritance);
     if (!inheritanceGraph.isTrivial())
     {
     t << "    <inheritancegraph>" << endl;
     inheritanceGraph.writeDocbook(t);
     t << "    </inheritancegraph>" << endl;
     }
     DotClassGraph collaborationGraph(cd,DotNode::Collaboration);
     if (!collaborationGraph.isTrivial())
     {
     t << "    <collaborationgraph>" << endl;
     collaborationGraph.writeDocbook(t);
     t << "    </collaborationgraph>" << endl;
     }
     t << "    <location file=\""
     << cd->getDefFileName() << "\" line=\""
     << cd->getDefLine() << "\"";
     if (cd->getStartBodyLine()!=-1)
     {
     FileDef *bodyDef = cd->getBodyDef();
     if (bodyDef)
     {
     t << " bodyfile=\"" << bodyDef->absFilePath() << "\"";
     }
     t << " bodystart=\"" << cd->getStartBodyLine() << "\" bodyend=\""
     << cd->getEndBodyLine() << "\"";
     }
     t << "/>" << endl;
     writeListOfAllMembers(cd,t);
    */

   t << "</section>" << endl;

}

static void generateDocbookForNamespace(NamespaceDef *nd, FTextStream &ti)
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

   QByteArray fileDocbook = nd->getOutputFileBase() + ".xml";
   //Add the file Documentation info to index file
   ti << "        <xi:include href=\"" << fileDocbook << "\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;

   QByteArray outputDirectory = Config_getString("DOCBOOK_OUTPUT");
   QByteArray fileName = outputDirectory + "/" + nd->getOutputFileBase() + ".xml";
   QFile f(fileName);
   if (!f.open(QIODevice::WriteOnly)) {
      err("Cannot open file %s for writing!\n", fileName.data());
      return;
   }
   FTextStream t(&f);
   //t.setEncoding(FTextStream::UnicodeUTF8);

   writeDocbookHeader_ID(t, nd->getOutputFileBase());
   t << "<title>";
   writeDocbookString(t, nd->name());
   t << "</title>" << endl;

   writeInnerClasses(nd->getClassSDict(), t);
   writeInnerNamespaces(nd->getNamespaceSDict(), t);

   if (nd->getMemberGroupSDict()) {
      MemberGroupSDict::Iterator mgli(*nd->getMemberGroupSDict());
      MemberGroup *mg;
      for (; (mg = mgli.current()); ++mgli) {
         generateDocbookSection(nd, t, mg->members(), "user-defined", 0, mg->header(),
                                mg->documentation());
      }
   }

   QListIterator<MemberList> mli(nd->getMemberLists());
   MemberList *ml;
   for (mli.toFirst(); (ml = mli.current()); ++mli) {
      if ((ml->listType()&MemberListType_declarationLists) != 0) {
         generateDocbookSection(nd, t, ml, g_docbookSectionMapper.find(ml->listType()));
      }
   }

   if (nd->briefDescription()) {
      t << "    <simplesect>" << endl;
      t << "        <title>Brief Description</title>" << endl;
      writeDocbookDocBlock(t, nd->briefFile(), nd->briefLine(), nd, 0, nd->briefDescription());
      t << "    </simplesect>" << endl;
   }

   if (nd->documentation()) {
      t << "        <simplesect>" << endl;
      t << "            <title>Detailed Description</title>" << endl;
      writeDocbookDocBlock(t, nd->docFile(), nd->docLine(), nd, 0, nd->documentation());
      t << "                <para>Definition at line " << nd->getDefLine() << " of file " << stripPath(nd->getDefFileName()) << "</para>" << endl;
      t << "                <para>The Documentation for this struct was generated from the following file: </para>" << endl;
      t << "                <para><itemizedlist><listitem><para>" << stripPath(nd->getDefFileName()) << "</para></listitem></itemizedlist></para>" << endl;
      t << "        </simplesect>" << endl;
   }
   t << "</section>" << endl;
}

static void generateDocbookForFile(FileDef *fd, FTextStream &ti)
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

   QByteArray fileDocbook = fd->getOutputFileBase() + ".xml";
   //Add the file Documentation info to index file
   ti << "        <xi:include href=\"" << fileDocbook << "\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;

   QByteArray outputDirectory = Config_getString("DOCBOOK_OUTPUT");
   QByteArray fileName = outputDirectory + "/" + fd->getOutputFileBase() + ".xml";
   QByteArray relPath = relativePathToRoot(fileName);

   QFile f(fileName);
   if (!f.open(QIODevice::WriteOnly)) {
      err("Cannot open file %s for writing!\n", fileName.data());
      return;
   }
   FTextStream t(&f);
   //t.setEncoding(FTextStream::UnicodeUTF8);
   writeDocbookHeader_ID(t, fd->getOutputFileBase());

   t << "    <title>";
   writeDocbookString(t, fd->name());
   t << " File Reference";
   t << "</title>" << endl;

   IncludeInfo *inc;

   if (fd->includeFileList()) {
      QListIterator<IncludeInfo> ili1(*fd->includeFileList());
      for (ili1.toFirst(); (inc = ili1.current()); ++ili1) {
         t << "    <programlisting>#include ";
         if (inc->local) {
            t << "&quot;";
         } else {
            t << "&lt;";
         }
         t << convertToXML(inc->includeName);
         if (inc->local) {
            t << "&quot;";
         } else {
            t << "&gt;";
         }
         t << "</programlisting>" << endl;
      }
   }
   if (Config_getBool("HAVE_DOT")) {
      if (Config_getBool("INCLUDE_GRAPH")) {
         t << "<para>Include dependency diagram for " << convertToXML(fd->name()) << "</para>" << endl;
         DotInclDepGraph idepGraph(fd, false);
         idepGraph.writeGraph(t, GOF_BITMAP, EOF_DocBook, Config_getString("DOCBOOK_OUTPUT"), fileName, relPath, false);
      }
      if (Config_getBool("INCLUDED_BY_GRAPH")) {
         t << "<para>Included by dependency diagram for " << convertToXML(fd->name()) << "</para>" << endl;
         DotInclDepGraph ibdepGraph(fd, true);
         ibdepGraph.writeGraph(t, GOF_BITMAP, EOF_DocBook, Config_getString("DOCBOOK_OUTPUT"), fileName, relPath, false);
      }
   }

   if (fd->getClassSDict()) {
      writeInnerClasses(fd->getClassSDict(), t);
   }
   if (fd->getNamespaceSDict()) {
      writeInnerNamespaces(fd->getNamespaceSDict(), t);
   }

   if (fd->getMemberGroupSDict()) {
      MemberGroupSDict::Iterator mgli(*fd->getMemberGroupSDict());
      MemberGroup *mg;
      for (; (mg = mgli.current()); ++mgli) {
         generateDocbookSection(fd, t, mg->members(), "user-defined", 0, mg->header(),
                                mg->documentation());
      }
   }

   QListIterator<MemberList> mli(fd->getMemberLists());
   MemberList *ml;
   for (mli.toFirst(); (ml = mli.current()); ++mli) {
      if ((ml->listType()&MemberListType_declarationLists) != 0) {
         generateDocbookSection(fd, t, ml, g_docbookSectionMapper.find(ml->listType()));
      }
   }

   t << "    <simplesect>" << endl;
   t << "        <title>Detailed Description</title>" << endl;
   writeDocbookDocBlock(t, fd->briefFile(), fd->briefLine(), fd, 0, fd->briefDescription());
   writeDocbookDocBlock(t, fd->docFile(), fd->docLine(), fd, 0, fd->documentation());
   if (Config_getBool("FULL_PATH_NAMES")) {
      t << "    <para>Definition in file " << fd->getDefFileName() << "</para>" << endl;
   } else {
      t << "    <para>Definition in file " << stripPath(fd->getDefFileName()) << "</para>" << endl;
   }
   t << "    </simplesect>" << endl;

   if (Config_getBool("DOCBOOK_PROGRAMLISTING")) {
      t << "    <literallayout><computeroutput>" << endl;
      writeDocbookCodeBlock(t, fd);
      t << "    </computeroutput></literallayout>" << endl;
   }

   t << "</section>" << endl;
}

static void generateDocbookForGroup(GroupDef *gd, FTextStream &ti)
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

   if (!gd->isASubGroup()) {
      QByteArray fileDocbook = gd->getOutputFileBase() + ".xml";
      //Add the file Documentation info to index file
      ti << "        <xi:include href=\"" << fileDocbook << "\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;
   }

   QByteArray outputDirectory = Config_getString("DOCBOOK_OUTPUT");
   QByteArray fileName = outputDirectory + "/" + gd->getOutputFileBase() + ".xml";
   QByteArray relPath = relativePathToRoot(fileName);

   QFile f(fileName);
   if (!f.open(QIODevice::WriteOnly)) {
      err("Cannot open file %s for writing!\n", fileName.data());
      return;
   }

   FTextStream t(&f);
   //t.setEncoding(FTextStream::UnicodeUTF8);
   writeDocbookHeader_ID(t, gd->getOutputFileBase());

   t << "    <title>" << convertToXML(gd->groupTitle()) << "</title>" << endl;

   if (Config_getBool("GROUP_GRAPHS") && Config_getBool("HAVE_DOT")) {
      t << "<para>Collaboration diagram for " << convertToXML(gd->groupTitle()) << "</para>" << endl;
      DotGroupCollaboration collaborationGraph(gd);
      collaborationGraph.writeGraph(t, GOF_BITMAP, EOF_DocBook, Config_getString("DOCBOOK_OUTPUT"), fileName, relPath, false);
   }

   if (gd->briefDescription()) {
      t << "    <simplesect>" << endl;
      t << "        <title>Brief Description</title>" << endl;
      writeDocbookDocBlock(t, gd->briefFile(), gd->briefLine(), gd, 0, gd->briefDescription());
      t << "    </simplesect>" << endl;
   }

   if (gd->documentation()) {
      t << "        <simplesect>" << endl;
      t << "            <title>Detailed Description</title>" << endl;
      writeDocbookDocBlock(t, gd->docFile(), gd->docLine(), gd, 0, gd->documentation());
      t << "        </simplesect>" << endl;
   }

   writeInnerFiles(gd->getFiles(), t);
   writeInnerClasses(gd->getClasses(), t);
   writeInnerNamespaces(gd->getNamespaces(), t);
   writeInnerPages(gd->getPages(), t);
   writeInnerGroups(gd->getSubGroups(), t);

   if (gd->getMemberGroupSDict()) {
      MemberGroupSDict::Iterator mgli(*gd->getMemberGroupSDict());
      MemberGroup *mg;
      for (; (mg = mgli.current()); ++mgli) {
         generateDocbookSection(gd, t, mg->members(), "user-defined", 0, mg->header(),
                                mg->documentation());
      }
   }

   QListIterator<MemberList> mli(gd->getMemberLists());
   MemberList *ml;
   for (mli.toFirst(); (ml = mli.current()); ++mli) {
      if ((ml->listType()&MemberListType_declarationLists) != 0) {
         generateDocbookSection(gd, t, ml, g_docbookSectionMapper.find(ml->listType()));
      }
   }
   for (mli.toFirst(); (ml = mli.current()); ++mli) {
      if ((ml->listType()&MemberListType_declarationLists) != 0) {
         generateDocbookSection(gd, t, ml, g_docbookSectionMapper.find(ml->listType()), 1);
      }
   }

   writeInnerGroupFiles(gd->getSubGroups(), t);

   t << "</section>" << endl;

}

static void generateDocbookForDir(DirDef *dd, FTextStream &ti)
{
   if (dd->isReference()) {
      return;   // skip external references
   }

   QByteArray fileDocbook = dd->getOutputFileBase() + ".xml";
   //Add the file Documentation info to index file
   ti << "        <xi:include href=\"" << fileDocbook << "\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;

   QByteArray outputDirectory = Config_getString("DOCBOOK_OUTPUT");
   QByteArray fileName = outputDirectory + "/" + dd->getOutputFileBase() + ".xml";
   QFile f(fileName);
   QByteArray relPath = relativePathToRoot(fileName);

   if (!f.open(QIODevice::WriteOnly)) {
      err("Cannot open file %s for writing!\n", fileName.data());
      return;
   }

   FTextStream t(&f);
   //t.setEncoding(FTextStream::UnicodeUTF8);
   writeDocbookHeader_ID(t, dd->getOutputFileBase());

   t << "    <title>";
   writeDocbookString(t, dd->displayName());
   t << " Directory Reference";
   t << "</title>" << endl;
   if (Config_getBool("DIRECTORY_GRAPH") && Config_getBool("HAVE_DOT")) {
      t << "<para>Directory dependency diagram for " << convertToXML(dd->displayName()) << "</para>" << endl;
      DotDirDeps dirdepGraph(dd);
      dirdepGraph.writeGraph(t, GOF_BITMAP, EOF_DocBook, Config_getString("DOCBOOK_OUTPUT"), fileName, relPath, false);
   }

   writeInnerDirs(&dd->subDirs(), t);
   writeInnerFiles(dd->getFiles(), t);

   t << "    <simplesect>" << endl;
   t << "        <title>Detailed Description</title>" << endl;
   writeDocbookDocBlock(t, dd->briefFile(), dd->briefLine(), dd, 0, dd->briefDescription());
   writeDocbookDocBlock(t, dd->docFile(), dd->docLine(), dd, 0, dd->documentation());
   t << "    <para>Directory location is " << dd->name() << "</para>" << endl;
   t << "    </simplesect>" << endl;

   t << "</section>" << endl;
}

static void generateDocbookForPage(PageDef *pd, FTextStream &ti, bool isExample)
{
   // + name
   // + title
   // + documentation

   if (pd->isReference()) {
      return;
   }

   QByteArray pageName = pd->getOutputFileBase();
   if (pd->getGroupDef()) {
      pageName += (QByteArray)"_" + pd->name();
   }
   if (pageName == "index") {
      pageName = "mainpage"; // to prevent overwriting the generated index page.
   }

   QByteArray outputDirectory = Config_getString("DOCBOOK_OUTPUT");
   QByteArray fileName = outputDirectory + "/" + pageName + ".xml";
   QFile f(fileName);
   if (!f.open(QIODevice::WriteOnly)) {
      err("Cannot open file %s for writing!\n", fileName.data());
      return;
   }

   FTextStream t(&f);
   //t.setEncoding(FTextStream::UnicodeUTF8);

   if (isExample) {
      QByteArray fileDocbook = pageName + ".xml";
      ti << "        <xi:include href=\"" << fileDocbook << "\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;
   }

   if (!pd->hasParentPage() && !isExample) {
      QByteArray fileDocbook = pageName + ".xml";
      //Add the file Documentation info to index file
      ti << "        <xi:include href=\"" << fileDocbook << "\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;
      writeDocbookHeaderMainpage(t);
   } else {
      QByteArray pid = pageName + "_1" + pageName;
      writeDocbookHeader_ID(t, pid);
   }

   SectionInfo *si = Doxygen::sectionDict->find(pd->name());
   if (si) {
      t << "    <title>" << convertToXML(si->title) << "</title>" << endl;
   } else {
      t << "    <title>" << convertToXML(pd->name()) << "</title>" << endl;
   }

   if (isExample) {
      writeDocbookDocBlock(t, pd->docFile(), pd->docLine(), pd, 0,
                           pd->documentation() + "\n\\include " + pd->name());
   } else {
      writeDocbookDocBlock(t, pd->docFile(), pd->docLine(), pd, 0,
                           pd->documentation());
   }
   writeInnerPages(pd->getSubPages(), t);

   if (!pd->hasParentPage() && !isExample) {
      t << endl << "</chapter>" << endl;
   } else {
      t << endl << "</section>" << endl;
   }
}

void generateDocbook()
{

   // + classes
   // + namespaces
   // + files
   // + groups
   // + related pages
   // - examples

   QByteArray outputDirectory = Config_getString("DOCBOOK_OUTPUT");
   if (outputDirectory.isEmpty()) {
      outputDirectory = QDir::currentPath().toUtf8();
   } else {
      QDir dir(outputDirectory);
      if (!dir.exists()) {
         dir.setPath(QDir::currentPath());
         if (!dir.mkdir(outputDirectory)) {
            err("tag DOCBOOK_OUTPUT: Output directory `%s' does not "
                "exist and cannot be created\n", outputDirectory.data());
            exit(1);
         } else {
            msg("Notice: Output directory `%s' does not exist. "
                "I have created it for you.\n", outputDirectory.data());
         }
         dir.cd(outputDirectory);
      }
      outputDirectory = dir.absolutePath().toUtf8();
   }

   QDir dir(outputDirectory);
   if (!dir.exists()) {
      dir.setPath(QDir::currentPath());
      if (!dir.mkdir(outputDirectory)) {
         err("Cannot create directory %s\n", outputDirectory.data());
         return;
      }
   }
   QDir docbookDir(outputDirectory);
   createSubDirs(docbookDir);

   QByteArray fileName = outputDirectory + "/index.xml";
   QByteArray dbk_projectName = Config_getString("PROJECT_NAME");

   QFile f(fileName);
   f.setFileName(fileName);

   if (!f.open(QIODevice::WriteOnly)) {
      err("Cannot open file %s for writing!\n", fileName.data());
      return;
   }

   FTextStream t(&f);
   //t.setEncoding(FTextStream::UnicodeUTF8);

   // write index header for Docbook which calls the structure file
   t << "<?xml version='1.0' encoding='UTF-8' standalone='no'?>" << endl;;
   t << "<book xmlns=\"http://docbook.org/ns/docbook\" version=\"5.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">" << endl;
   t << "    <info>" << endl;
   t << "    <title>" << dbk_projectName << "</title>" << endl;
   t << "    </info>" << endl;

   // NAMESPACE DOCUMENTATION
   NamespaceSDict::Iterator nli(*Doxygen::namespaceSDict);
   NamespaceDef *nd;

   //Namespace Documentation index header
   if (nli.toFirst()) {
      t << "    <chapter>" << endl;
      t << "        <title>Namespace Documentation</title>" << endl;
   }

   for (nli.toFirst(); (nd = nli.current()); ++nli) {
      msg("Generating Docbook output for namespace %s\n", nd->name().data());
      generateDocbookForNamespace(nd, t);
   }

   //Namespace Documentation index footer
   if (nli.toFirst()) {
      t << "    </chapter>" << endl;
   }

   /** MAINPAGE DOCUMENTATION **/

   if (Doxygen::mainPage) {
      msg("Generating Docbook output for the main page\n");
      generateDocbookForPage(Doxygen::mainPage, t, false);
   }

   // PAGE DOCUMENTATION
   {
      PageSDict::Iterator pdi(*Doxygen::pageSDict);
      PageDef *pd = 0;

      for (pdi.toFirst(); (pd = pdi.current()); ++pdi) {
         msg("Generating Docbook output for page %s\n", pd->name().data());
         generateDocbookForPage(pd, t, false);
      }
   }

   /** MODULE GROUP DOCUMENTATION **/

   GroupSDict::Iterator gli(*Doxygen::groupSDict);
   GroupDef *gd;

   //Module group Documentation index header
   if (gli.toFirst()) {
      t << "    <chapter>" << endl;
      t << "        <title>Module Documentation</title>" << endl;
   }

   for (; (gd = gli.current()); ++gli) {
      msg("Generating Docbook output for group %s\n", gd->name().data());
      generateDocbookForGroup(gd, t);
   }

   //Module group Documentation index footer
   if (gli.toFirst()) {
      t << "    </chapter>" << endl;
   }

   //CLASS DOCUMENTATION

   {
      ClassSDict::Iterator cli(*Doxygen::classSDict);
      ClassDef *cd;

      //Class Documentation index header
      if (cli.toFirst()) {
         t << "    <chapter>" << endl;
         t << "        <title>Class Documentation</title>" << endl;
      }

      for (cli.toFirst(); (cd = cli.current()); ++cli) {
         generateDocbookForClass(cd, t);
      }

      //Class Documentation index footer
      if (cli.toFirst()) {
         t << "    </chapter>" << endl;
      }
   }

   // FILE DOCUMENTATION

   static bool showFiles = Config_getBool("SHOW_FILES");
   if (showFiles) {
      FileNameListIterator fnli(*Doxygen::inputNameList);
      FileName *fn;

      //File Documentation index header
      if (fnli.toFirst()) {
         t << "    <chapter>" << endl;
         t << "        <title>File Documentation</title>" << endl;
      }

      for (; (fn = fnli.current()); ++fnli) {
         FileNameIterator fni(*fn);
         FileDef *fd;
         for (; (fd = fni.current()); ++fni) {
            msg("Generating Docbook output for file %s\n", fd->name().data());
            generateDocbookForFile(fd, t);
         }
      }

      //File Documentation index footer
      if (fnli.toFirst()) {
         t << "    </chapter>" << endl;
      }
   }

   // DIRECTORY DOCUMENTATION
   if (Config_getBool("DIRECTORY_GRAPH") && Config_getBool("HAVE_DOT")) {
      DirDef *dir;
      DirSDict::Iterator sdi(*Doxygen::directories);

      //Directory Documentation index header
      if (sdi.toFirst()) {
         t << "    <chapter>" << endl;
         t << "        <title>Directory Documentation</title>" << endl;
      }

      for (sdi.toFirst(); (dir = sdi.current()); ++sdi) {
         msg("Generate Docbook output for dir %s\n", dir->name().data());
         generateDocbookForDir(dir, t);
      }

      //Module group Documentation index footer
      if (sdi.toFirst()) {
         t << "    </chapter>" << endl;
      }
   }

   // EXAMPLE PAGE DOCUMENTATION

   {
      PageSDict::Iterator pdi(*Doxygen::exampleSDict);
      PageDef *pd = 0;

      //Example Page Documentation index header
      if (pdi.toFirst()) {
         t << "    <chapter>" << endl;
         t << "        <title>Example Documentation</title>" << endl;
      }

      for (pdi.toFirst(); (pd = pdi.current()); ++pdi) {
         msg("Generating Docbook output for example %s\n", pd->name().data());
         generateDocbookForPage(pd, t, true);
      }

      //Example Page Documentation index footer
      if (pdi.toFirst()) {
         t << "    </chapter>" << endl;
      }
   }

   t << "</book>" << endl;

}


