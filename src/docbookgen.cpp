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

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QHash>

#include <stdlib.h>

#include <docbookgen.h>

#include <arguments.h>
#include <config.h>
#include <default_args.h>
#include <docbookvisitor.h>
#include <docparser.h>
#include <dot.h>
#include <doxy_globals.h>
#include <doxy_build_info.h>
#include <groupdef.h>
#include <language.h>
#include <message.h>
#include <membergroup.h>
#include <outputgen.h>
#include <parse_base.h>
#include <util.h>

class DocbookSectionMapper : public QHash<long, QString>
{
 public:
   DocbookSectionMapper() : QHash<long, QString>() {
      insert(MemberListType_pubTypes,    "public-type");
      insert(MemberListType_pubMethods,  "public-func");
      insert(MemberListType_pubAttribs,  "public-attrib");
      insert(MemberListType_pubSlots,    "public-slot");
      insert(MemberListType_pubSignals,  "public-signal");
      insert(MemberListType_dcopMethods, "dcop-func");
      insert(MemberListType_properties,  "property");
      insert(MemberListType_events,      "event");
      insert(MemberListType_pubStaticMethods, "public-static-func");
      insert(MemberListType_pubStaticAttribs, "public-static-attrib");
      insert(MemberListType_proTypes, "protected-type");
      insert(MemberListType_proMethods, "protected-func");
      insert(MemberListType_proAttribs, "protected-attrib");
      insert(MemberListType_proSlots, "protected-slot");
      insert(MemberListType_proSignals,  "protected-signal");
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
      insert(MemberListType_priSignals,  "private-signal");
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


inline void writeDocbookString(QTextStream &t, const QString &text)
{
   t << convertToXML(text);
}

inline void writeDocbookCodeString(QTextStream &t, const QString &s, int &col)
{
   for (auto c : s) {

      switch (c.unicode()) {
         case '\t': {
            static int tabSize = Config::getInt("tab-size");
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

static void writeDocbookHeaderMainpage(QTextStream &t)
{
   t << "<?xml version='1.0' encoding='UTF-8' standalone='no'?>" << endl;;
   t << "<chapter xmlns=\"http://docbook.org/ns/docbook\" version=\"5.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">" << endl;
}

static void writeDocbookHeader_ID(QTextStream &t, QString id)
{
   t << "<?xml version='1.0' encoding='UTF-8' standalone='no'?>" << endl;;
   t << "<section xmlns=\"http://docbook.org/ns/docbook\" version=\"5.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xml:id=\"" << id << "\">" << endl;
}

void writeDocbookLink(QTextStream &t, const QString &compoundId, const QString &anchorId, const QString &text )
{
   t << "<link linkend=\"" << compoundId;

   if (! anchorId.isEmpty()) {
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
   TextGeneratorDocbookImpl(QTextStream &t): m_t(t) {}

   void writeString(const QString &text, bool) const override {
      writeDocbookString(m_t, text);
   }

   void writeBreak(int) const override {}

   void writeLink(const QString &extRef, const QString &file, const QString &anchor, const QString &text) const override  {
      (void) extRef;

      writeDocbookLink(m_t, file, anchor, text);
   }

 private:
   QTextStream &m_t;
};

class DocbookCodeGenerator : public CodeOutputInterface
{
 public:
   DocbookCodeGenerator(QTextStream &t) : m_t(t), m_lineNumber(-1), m_col(0),
      m_insideCodeLine(false), m_insideSpecialHL(false) {}

   virtual ~DocbookCodeGenerator() {}

   void codify(const QString &text)  override {
      writeDocbookCodeString(m_t, text, m_col);
   }

   void writeCodeLink(const QString &ref, const QString &file, const QString &anchor,
                  const QString &name, const QString &tooltip) override {

      (void) ref;
      (void) tooltip;

      writeDocbookLink(m_t, file, anchor, name);
      m_col += name.length();
   }

   void writeTooltip(const QString &, const DocLinkInfo &, const QString &,
                     const QString &, const SourceLinkInfo &, const SourceLinkInfo &) override {
   }

   void startCodeLine(bool) override {

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

   void endCodeLine() override {
      m_t << endl;
      m_lineNumber = -1;
      m_refId.resize(0);
      m_external.resize(0);
      m_insideCodeLine = false;
   }

   void startFontClass(const QString &colorClass)  override {
      m_t << "<emphasis class=\"" << colorClass << "\">";
      m_insideSpecialHL = true;
   }

   void endFontClass() override {
      m_t << "</emphasis>"; // non DocBook
      m_insideSpecialHL = false;
   }

   void writeCodeAnchor(const QString &) override {
   }

   void writeLineNumber(const QString &extRef, const QString &compId, const QString &anchorId, int l) override {
      // we remember the information provided here to use it
      // at the <codeline> start tag.

      m_lineNumber = l;

      if (! compId.isEmpty()) {
         m_refId = compId;

         if (! anchorId.isEmpty()) {
            m_refId += "_1" + anchorId;
         }

         if (! extRef.isEmpty()) {
            m_external = extRef;
         }
      }
   }

   void setCurrentDoc(QSharedPointer<Definition>, const QString &, bool) override {
   }

   void addWord(const QString &, bool) override {
   }

   void finish() {
      if (m_insideCodeLine) {
         endCodeLine();
      }
   }

 private:
   QTextStream &m_t;
   QString m_refId;
   QString m_external;

   int m_lineNumber;
   int m_col;
   bool m_insideCodeLine;
   bool m_insideSpecialHL;
};

static void writeTemplateArgumentList(const ArgumentList &argList, QTextStream &t, QSharedPointer<Definition> scope,
                  QSharedPointer<FileDef> fileScope, int indent)
{
   QString indentStr;
   indentStr.fill(' ', indent);

   t << indentStr << "<templateparamlist>" << endl;

   for (auto &item : argList) {
      t << indentStr << "  <param>" << endl;

      if (! item.type.isEmpty()) {
         t << indentStr <<  "    <type>";
         linkifyText(TextGeneratorDocbookImpl(t), scope, fileScope, QSharedPointer<Definition>(), item.type);
         t << "</type>" << endl;
      }

      if (! item.name.isEmpty()) {
         t << indentStr <<  "    <declname>" << item.name << "</declname>" << endl;
         t << indentStr <<  "    <defname>" << item.name << "</defname>" << endl;
      }

      if (! item.defval.isEmpty()) {
         t << indentStr << "    <defval>";
         linkifyText(TextGeneratorDocbookImpl(t), scope, fileScope, QSharedPointer<Definition>(), item.defval);
         t << "</defval>" << endl;
      }

      t << indentStr << "  </param>" << endl;
   }

   t << indentStr << "</templateparamlist>" << endl;

}

static void writeTemplateList(QSharedPointer<ClassDef> cd, QTextStream &t)
{
   writeTemplateArgumentList(cd->getTemplateArgumentList(), t, cd, QSharedPointer<FileDef>(), 4);
}

static void writeDocbookDocBlock(QTextStream &t, const QString &fileName, int lineNr, QSharedPointer<Definition> scope,
                  QSharedPointer<MemberDef> md, const QString &text)
{
   QString stext = text.trimmed();
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

void writeDocbookCodeBlock(QTextStream &t, QSharedPointer<FileDef> fd)
{
   static const bool filterSourceFiles = Config::getBool("filter-source-files");

   ParserInterface *pIntf = Doxy_Globals::parserManager.getParser(fd->getDefFileExtension());
   SrcLangExt langExt = getLanguageFromFileName(fd->getDefFileExtension());
   pIntf->resetCodeParserState();

   DocbookCodeGenerator *docbookGen = new DocbookCodeGenerator(t);

   pIntf->parseCode(*docbookGen, 0, fileToString(fd->getFilePath(), filterSourceFiles),
                    langExt, false, 0, fd, -1, -1, false, QSharedPointer<MemberDef>(), true);

   docbookGen->finish();
   delete docbookGen;
}

static QString classOutputFileBase(QSharedPointer<ClassDef> cd)
{
   return cd->getOutputFileBase();
}

static QString memberOutputFileBase(QSharedPointer<MemberDef> md)
{
   return md->getOutputFileBase();
}


static void generateDocbookForMember(QSharedPointer<MemberDef> md, QTextStream &t, QSharedPointer<Definition> def, bool detailed = 0)
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

   static const bool repeatBrief = Config::getBool("repeat-brief");

   // enum values are written as part of the enum
   if (md->memberType() == MemberDefType::EnumValue) {
      return;
   }

   if (md->isHidden()) {
      return;
   }

   // if (md->name().at(0)=='@') return; // anonymous member
   // group members are only visible in their group
   // if (def->definitionType()!=Definition::TypeGroup && md->getGroupDef()) return;

   QString memType;

   switch (md->memberType()) {
      case MemberDefType::Define:
         memType = "define";
         break;

      case MemberDefType::Function:
         memType = "function";
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
         break;

      case MemberDefType::Slot:
         memType = "slot";
         break;

      case MemberDefType::DCOP:
         memType = "dcop";
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

   QString scopeName;

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

      if (md->memberType() == MemberDefType::Enumeration) {
         QSharedPointer<MemberList> enumFields = md->enumFieldList();

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

            for (auto emd : *enumFields) {
               if (cnt != 0) {
                  t << "," << endl;
               }

               t << "<link linkend=\"" << memberOutputFileBase(emd) << "_1" << emd->anchor() << "\">";
               writeDocbookString(t, emd->name());

               t << "</link>";
               if (! emd->initializer().isEmpty()) {
                  writeDocbookString(t, emd->initializer());
               }

               cnt++;
            }

            t << endl << "}";
         }

         t << "</literallayout>" << endl;

         if (! md->briefDescription().isEmpty()) {
            t << "<para><emphasis>";
            writeDocbookString(t, md->briefDescription());
            t << "</emphasis></para>" << endl;
         }

      } else if (md->memberType() == MemberDefType::Define) {
         t << "                            <para>" << "#" << memType << " <link linkend=\"";

         if (md->getGroupDef() && def->definitionType() == Definition::TypeGroup) {
            t << md->getGroupDef()->getOutputFileBase();
         } else {
            t << memberOutputFileBase(md);
         }

         t << "_1" << md->anchor() << "\">" << convertToXML(md->name()) << "</link>";

         if (! md->initializer().isEmpty() && md->initializer().length() < 2000) {
            t << " ";
            linkifyText(TextGeneratorDocbookImpl(t), def, md->getBodyDef(), md, md->initializer());
         }

         if (! md->briefDescription().isEmpty()) {
            t << "<para><emphasis>";
            writeDocbookString(t, md->briefDescription());
            t << "</emphasis></para>" << endl;
         }

      } else if (md->memberType() == MemberDefType::Variable) {
         if (md->getClassDef()) {
            t << "                        <para>" << convertToXML(md->declaration());

            if (! md->briefDescription().isEmpty()) {
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

            if (! md->briefDescription().isEmpty()) {
               t << "<para><emphasis>";
               writeDocbookString(t,md->briefDescription());
               t << "</emphasis></para>" << endl;
            }
         }

      } else if (md->memberType() == MemberDefType::Typedef) {
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

         if (! md->briefDescription().isEmpty()) {
            t << "<para><emphasis>";
            writeDocbookString(t,md->briefDescription());
            t << "</emphasis></para>" << endl;
         }

      } else if (md->memberType() == MemberDefType::Function) {
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

         const ArgumentList &declAl = md->getDeclArgumentList();
         int cnt = 0;

         for (auto &arg : declAl) {
            if (cnt != 0) {
               t << ", ";
            }

            if (! arg.type.isEmpty()) {
               linkifyText(TextGeneratorDocbookImpl(t), def, md->getBodyDef(), md, arg.type);
            }

            t << " ";

            if (! arg.name.isEmpty()) {
               writeDocbookString(t, arg.name);
            }

            cnt++;
         }

         t << ")";

         if (! md->briefDescription().isEmpty()) {
           t << "<para><emphasis>";
           writeDocbookString(t,md->briefDescription());
           t << "</emphasis></para>" << endl;
          }

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

      if (md->memberType() == MemberDefType::Enumeration) {
         QSharedPointer<MemberList> enumFields = md->enumFieldList();

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

            t << "               <formalpara>" << endl;
            t << "                    <title>" << theTranslator->trEnumerationValues() << ":</title>" << endl;
            t << "                    <variablelist>" << endl;

            for (auto emd : *enumFields) {

               t << "                        <varlistentry xml:id=\"";
               t << memberOutputFileBase(emd) << "_1" << emd->anchor() << "\">" << endl;
               t << "                            <term>";

               writeDocbookString(t, emd->name());
               t << "</term>" << endl;
               t << "                            <listitem>" << endl;

               if (repeatBrief) {
                  t << "                                <para>";
                  writeDocbookString(t,emd->briefDescription());
                  t << "</para>" << endl;
               }

               t << "                            </listitem>" << endl;
               t << "                        </varlistentry>" << endl;
            }

            t << "                     </variablelist>" << endl;
            t << "                </formalpara>" << endl;
            t << "                <para>";
            t << "Definition at line " << md->getDefLine() << " of file " << stripPath(md->getDefFileName()) << endl;
            t << "                    <computeroutput><literallayout>" << endl;
            t << "{" << endl;

            for (auto emd : *enumFields) {
               writeDocbookString(t, emd->name());

               if (! emd->initializer().isEmpty()) {
                  writeDocbookString(t, emd->initializer());
               }
               t << ", " << endl;

            }

            t << "}" << convertToXML(md->name()) << ";" << endl;
            t << "                    </literallayout></computeroutput>" << endl;
            t << "                </para>" << endl;
            t << "            </section>" << endl;
         }

      } else if (md->memberType() == MemberDefType::Typedef) {
         t << "            <section xml:id=\"";

         if (md->getGroupDef() && def->definitionType() == Definition::TypeGroup) {
            t << md->getGroupDef()->getOutputFileBase();
         } else {
            t << memberOutputFileBase(md);
         }

         t << "_1" << md->anchor() << "\">" << endl;
         t << "                <title>" << convertToXML(md->definition()) << "</title>";

         if (repeatBrief) {
            t << " <emphasis>";
            writeDocbookString(t,md->briefDescription());
            t << "</emphasis>" << endl;
         }

         t << "                ";
         writeDocbookDocBlock(t, md->docFile(), md->docLine(), md->getOuterScope(), md, md->documentation());

         t << endl;
         t << "            </section>" << endl;

      } else if (md->memberType() == MemberDefType::Function) {
         t << "            <section xml:id=\"";

         if (md->getGroupDef() && def->definitionType() == Definition::TypeGroup) {
            t << md->getGroupDef()->getOutputFileBase();
         } else {
            t << memberOutputFileBase(md);
         }

         t << "_1" << md->anchor() << "\">" << endl;
         t << "                <title>" << convertToXML(md->definition()) << " " << convertToXML(md->argsString()) << "</title>";

         if (repeatBrief)   {
            t << " <emphasis>";
            writeDocbookString(t,md->briefDescription());
            t << "</emphasis>" << endl;
         }

         t << "                ";
         writeDocbookDocBlock(t, md->docFile(), md->docLine(), md->getOuterScope(), md, md->documentation());

         t << endl;
         t << "            </section>" << endl;

      } else if (md->memberType() == MemberDefType::Define) {
         if (! md->documentation().isEmpty()) {
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

            t << "                <para><itemizedlist><listitem><para>" << stripPath(md->getDefFileName())
              << "</para></listitem></itemizedlist></para>" << endl;

            t << "            </section>" << endl;
         }

      } else if (md->memberType() == MemberDefType::Variable) {

         if (md->getClassDef()) {
            if (! md->documentation().isEmpty()) {
               t << "            <simplesect>" << endl;
               t << "                <title>" << convertToXML(md->definition()) << "</title>";
               t << "                ";
               writeDocbookDocBlock(t, md->docFile(), md->docLine(), md->getOuterScope(), md, md->documentation());

               t << endl;

               t << "                <para>Definition at line " << md->getDefLine() << " of file "
                 << stripPath(md->getDefFileName()) << "</para>" << endl;

               t << "                <para>The Documentation for this struct was generated from the following file: </para>" << endl;

               t << "                <para><itemizedlist><listitem><para>" << stripPath(md->getDefFileName())
                 << "</para></listitem></itemizedlist></para>" << endl;

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

            if (repeatBrief) {
               t << " <emphasis>";
               writeDocbookString(t,md->briefDescription());
               t << "</emphasis>" << endl;
            }

            t << "                ";
            writeDocbookDocBlock(t, md->docFile(), md->docLine(), md->getOuterScope(), md, md->documentation());
            t << endl;
            t << "            </section>" << endl;
         }
      }
   }
}

static void generateDocbookSection(QSharedPointer<Definition> d, QTextStream &t, QSharedPointer<MemberList> ml, const QString &kind,
            bool detailed = false, const QString &header = QString(), const QString &documentation = QString())
{
   (void) kind;

   if (ml == 0) {
      return;
   }

   static const bool repeatBrief = Config::getBool("repeat-brief");

   int count     = 0;
   int doc_count = 0;

   QString title;
   QString desctitle;

   for (auto md : *ml) {
      // namespace members are also inserted in the file scope
      // to prevent this duplication in the Docbook output, we filter those here

      if (d->definitionType() != Definition::TypeFile || md->getNamespaceDef() == 0) {
         count++;
      }
   }

   if (count == 0) {
      // empty list
      return;
   }

   switch (ml->listType()) {

      case MemberListType_decDefineMembers:
         title     = theTranslator->trDefines();
         desctitle = theTranslator->trDefineDocumentation();
         break;

      case MemberListType_decTypedefMembers:
         title     = theTranslator->trTypedefs();
         desctitle = theTranslator->trTypedefDocumentation();
         break;

      case MemberListType_decEnumMembers:
         title     = theTranslator->trEnumerations();
         desctitle = theTranslator->trEnumerationTypeDocumentation();
         break;

      case MemberListType_decFuncMembers:
         title     = theTranslator->trFunctions();
         desctitle = theTranslator->trFunctionDocumentation();
         break;

      case MemberListType_decVarMembers:
         title     = theTranslator->trVariables();
         desctitle = theTranslator->trVariableDocumentation();
         break;

      case MemberListType_pubAttribs:
         title     = theTranslator->trPublicAttribs();
         desctitle = "Member Documentation";
         break;

      case MemberListType_priAttribs:
         title     = theTranslator->trPrivateAttribs();
         desctitle = theTranslator->trMemberDataDocumentation();
         break;

      case MemberListType_proAttribs:
         title     = theTranslator->trProtectedAttribs();
         desctitle = theTranslator->trMemberDataDocumentation();
         break;

      default:
         title     = "";
         desctitle = "";
         break;
   }

   if (detailed) {

      for (auto md : *ml) {

         if (md->documentation().isEmpty() && ! repeatBrief) {
            continue;
         }

         doc_count = 1;
         break;
      }

      if (doc_count == 0) {
         return;
      }

      if (! desctitle.isEmpty()) {
         t << "        <section>" << endl;
         t << "            <title>" << desctitle << "</title>" << endl;
      }

   } else {

      t << "        <section>" << endl;

      if (! header.isEmpty()) {
         t << "            <title>" << convertToXML(header) << "</title>" << endl;
      } else {
         t << "            <title>" << title << "</title>" << endl;
      }
   }

   if (! documentation.isEmpty()) {
      t << "      <description>";
      writeDocbookDocBlock(t, d->docFile(), d->docLine(), d, QSharedPointer<MemberDef>(), documentation);
      t << "</description>" << endl;
   }

   for (auto md : *ml) {
      // namespace members are also inserted in the file scope,
      // to prevent this duplication in the Docbook output, we filter those here

      if (d->definitionType() != Definition::TypeFile || md->getNamespaceDef() == 0) {

         if (detailed && md->documentation().isEmpty() && ! repeatBrief) {
            continue;
         }

         generateDocbookForMember(md, t, d, detailed);
      }
   }

   if (detailed) {

      if (! desctitle.isEmpty()) {
         t << "        </section>" << endl;
      }

   } else {
      t << "        </section>" << endl;
   }
}

static void writeInnerClasses(const ClassSDict &cl, QTextStream &t)
{
   QString title = theTranslator->trClasses();

   if (! cl.isEmpty()) {
         t << "        <section>" << endl;
         t << "            <title> " << title << " </title>" << endl;
   }

   for (auto cd : cl) {
      if (! cd->isHidden() && cd->name().indexOf('@') == -1) {
         t << "            <para>" << endl;
         t << "                <itemizedlist>" << endl;
         t << "                    <listitem>" << endl;

         t << "                        <para>" << "struct <link linkend=\""
           << classOutputFileBase(cd) << "\">" << convertToXML(cd->name()) << "</link>";

         t << "</para>" << endl;

         if (! cd->briefDescription().isEmpty()) {
            t << "<para><emphasis>";
            writeDocbookString(t, cd->briefDescription());
            t << "</emphasis></para>" << endl;
         }

         t << "                    </listitem>" << endl;
         t << "                </itemizedlist>" << endl;
         t << "            </para>" << endl;
      }
   }

   if (! cl.isEmpty()) {
      t << "        </section>" << endl;
   }
}

static void writeInnerNamespaces(const NamespaceSDict &nl, QTextStream &t)
{
   if (nl.isEmpty()) {
      return;
   }

   static const QString title = theTranslator->trNamespaces();

   t << "        <simplesect>" << endl;
   t << "            <title> " << title << " </title>" << endl;

   for (auto &nd : nl) {
      if (! nd->isHidden() && nd->name().indexOf('@') == -1) {
         // skip anonymous scopes

         t << "            <para>" << endl;
         t << "                <itemizedlist>" << endl;
         t << "                    <listitem>" << endl;

         t << "                        <para>" << "struct <link linkend=\"" << nd->getOutputFileBase()
           << "\">" << convertToXML(nd->name()) << "</link>";

         t << "</para>" << endl;
         t << "                    </listitem>" << endl;
         t << "                </itemizedlist>" << endl;
         t << "            </para>" << endl;
      }
   }

   t << "        </simplesect>" << endl;
}

static void writeInnerFiles(const FileList &fl, QTextStream &t)
{
   QString title = theTranslator->trFile(true, true);

   if (! fl.isEmpty()) {
      t << "        <simplesect>" << endl;
      t << "            <title> " << title << " </title>" << endl;
   }

   for (auto fd : fl) {
      t << "            <para>" << endl;
      t << "                <itemizedlist>" << endl;
      t << "                    <listitem>" << endl;
      t << "                        <para>" << "file <link linkend=\"" << fd->getOutputFileBase() << "\">" << convertToXML(fd->name()) << "</link>";
      t << "</para>" << endl;
      t << "                    </listitem>" << endl;
      t << "                </itemizedlist>" << endl;
      t << "            </para>" << endl;
   }

   if (! fl.isEmpty()) {
      t << "        </simplesect>" << endl;
   }
}

static void writeInnerPages(const PageSDict *pl, QTextStream &t)
{
   if (pl) {
      for (auto pd : *pl) {
         t << "<xi:include href=\"" << pd->getOutputFileBase() << ".xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>"  << endl;
      }
   }
}

static void writeInnerGroups(const SortedList<QSharedPointer<GroupDef>> *gl, QTextStream &t)
{
   if (gl) {
      // Docbook header tags for inner groups

      if (! gl->isEmpty()) {
         t << "    <simplesect>" << endl;
         t << "        <title>" << theTranslator->trModules() << "</title>" << endl;
         t << "    </simplesect>" << endl;
         t << "    <para>" << endl;
         t << "        <itemizedlist>" << endl;
      }

      for (auto sgd : *gl) {
         t << "            <listitem><para><link linkend=\"" << sgd->getOutputFileBase() << "\">" << convertToXML(
              sgd->groupTitle()) << "</link></para></listitem>" << endl;
      }

      //Docbook footer tags for inner groups
      if (! gl->isEmpty()) {
         t << "        </itemizedlist>" << endl;
         t << "    </para>" << endl;
      }

   }
}

static void writeInnerDirs(const SortedList<QSharedPointer<DirDef>> dl, QTextStream &t)
{
   QString title = theTranslator->trDirectories();

   if (! dl.isEmpty()) {
      t << "        <simplesect>" << endl;
      t << "            <title> " << title << " </title>" << endl;
   }

   for (auto subdir : dl) {
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

   if (! dl.isEmpty()) {
      t << "        </simplesect>" << endl;
   }

}

static void writeInnerGroupFiles(const SortedList<QSharedPointer<GroupDef>> *gl, QTextStream &t)
{
   if (gl) {
      for (auto sgd : *gl) {
         t << "<xi:include href=\"" << sgd->getOutputFileBase()
           << ".xml\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>"  << endl;
      }
   }
}

static void generateDocbookForClass(QSharedPointer<ClassDef> cd, QTextStream &ti)
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

   static const QString docbookOutDir = Config::getString("docbook-output");
   static const bool haveDot          = Config::getBool("have-dot");
   static const bool repeatBrief      = Config::getBool("repeat-brief");

   msg("Generating Docbook output for class %s\n", csPrintable(cd->name()));

   // Add the file Documentation info to index file
   QString fileDocbook = cd->getOutputFileBase() + ".xml";
   ti << "        <xi:include href=\"" << fileDocbook << "\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;

   //
   QString fileName  = docbookOutDir + "/" + classOutputFileBase(cd) + ".xml";
   QString relPath   = relativePathToRoot(fileName);

   QFile fi(fileName);

   if (! fi.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", csPrintable(fileName), fi.error());
      return;
   }

   QTextStream t(&fi);

   writeDocbookHeader_ID(t, classOutputFileBase(cd));
   t << "<title>";

   writeDocbookString(t, cd->name());
   t << " " << cd->compoundTypeString() << " Reference";
   t << "</title>" << endl;

   const IncludeInfo &item = cd->includeInfo();

   QString nm = item.includeName;

   if (nm.isEmpty() && item.fileDef) {
      nm = item.fileDef->docName();
   }

   if (! nm.isEmpty()) {
      t << "<para>" << endl;
      t << "    <programlisting>#include ";

      if (item.fileDef && !item.fileDef->isReference()) {
         // TODO: support external references
         t << "<link linkend=\"" << item.fileDef->getOutputFileBase() << "\">";
      }

      if (item.local) {
         t << "&quot;";
      } else {
         t << "&lt;";
      }

      t << convertToXML(nm);

      if (item.local) {
         t << "&quot;";
      } else {
         t << "&gt;";
      }

      if (item.fileDef && ! item.fileDef->isReference()) {
         t << "</link>";
      }

      t << "</programlisting>" << endl;
      t << "</para>" << endl;
   }


   if (haveDot && (Config::getBool("class-diagrams") || Config::getBool("dot-class-graph"))) {
      t << "<para>Inheritance diagram for " << convertToXML(cd->name()) << "</para>" << endl;
      DotClassGraph inheritanceGraph(cd, DotNode::Inheritance);
      inheritanceGraph.writeGraph(t, GOF_BITMAP, EOF_DocBook, docbookOutDir, fileName, relPath, true, false);
   }

   if (haveDot && Config::getBool("dot-collaboration")) {
      t << "<para>Collaboration diagram for " << convertToXML(cd->name()) << "</para>" << endl;
      DotClassGraph collaborationGraph(cd, DotNode::Collaboration);
      collaborationGraph.writeGraph(t, GOF_BITMAP, EOF_DocBook, docbookOutDir, fileName, relPath, true, false);
   }

   writeInnerClasses(cd->getClassSDict(), t);
   writeTemplateList(cd, t);

   for (auto mg : cd->getMemberGroupSDict()) {
      generateDocbookSection(cd, t, mg->members(), "user-defined", false, mg->header(), mg->documentation());
   }


   for (auto ml : cd->getMemberLists()) {
      if ((ml->listType() & MemberListType_detailedLists) == 0) {
         generateDocbookSection(cd, t, ml, g_docbookSectionMapper.value(ml->listType()));
      }
   }

   if (repeatBrief) {

      if (! cd->briefDescription().isEmpty()) {
         t << "    <simplesect>" << endl;

         writeDocbookDocBlock(t, cd->briefFile(), cd->briefLine(), cd, QSharedPointer<MemberDef>(), cd->briefDescription());
         t << "    </simplesect>" << endl;
      }
   }

   if (! cd->documentation().isEmpty()) {
      t << "        <simplesect>" << endl;
      t << "            <title>" << theTranslator->trDetailedDescription() << "</title>" << endl;

      writeDocbookDocBlock(t, cd->docFile(), cd->docLine(), cd, QSharedPointer<MemberDef>(), cd->documentation());

      t << "                <para>Definition at line " << cd->getDefLine() << " of file "
        << stripPath(cd->getDefFileName()) << "</para>" << endl;

      t << "                <para>The Documentation for this struct was generated from the following file: </para>" << endl;

      t << "                <para><itemizedlist><listitem><para>" << stripPath(cd->getDefFileName())
        << "</para></listitem></itemizedlist></para>" << endl;

      t << "        </simplesect>" << endl;
   }

   for (auto ml : cd->getMemberLists()) {
      if ((ml->listType()&MemberListType_detailedLists) == 0) {
         generateDocbookSection(cd, t, ml, g_docbookSectionMapper.value(ml->listType()), true);
      }
   }

   t << "</section>" << endl;
}

static void generateDocbookForNamespace(QSharedPointer<NamespaceDef> nd, QTextStream &ti)
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

   QString outputDirectory       = Config::getString("docbook-output");
   static const bool repeatBrief = Config::getBool("repeat-brief");

   QString fileDocbook = nd->getOutputFileBase() + ".xml";
   //Add the file Documentation info to index file
   ti << "        <xi:include href=\"" << fileDocbook << "\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;

  QString fileName = outputDirectory + "/" + nd->getOutputFileBase() + ".xml";

   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", csPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);
   writeDocbookHeader_ID(t, nd->getOutputFileBase());

   t << "<title>";
   writeDocbookString(t, nd->name());

   t << "</title>" << endl;
   writeInnerClasses(nd->getClassSDict(), t);
   writeInnerNamespaces(nd->getNamespaceSDict(), t);

   for (auto mg : nd->getMemberGroupSDict() ) {
      generateDocbookSection(nd, t, mg->members(), "user-defined", false, mg->header(), mg->documentation());
   }

   for (auto ml : nd->getMemberLists()) {
      if ((ml->listType() & MemberListType_declarationLists) != 0) {
         generateDocbookSection(nd, t, ml, g_docbookSectionMapper.value(ml->listType()));
      }
   }

   if (repeatBrief) {

      if (! nd->briefDescription().isEmpty()) {
         t << "    <simplesect>" << endl;

         writeDocbookDocBlock(t, nd->briefFile(), nd->briefLine(), nd, QSharedPointer<MemberDef>(),
                  nd->briefDescription());

         t << "    </simplesect>" << endl;
      }
   }

   if (! nd->documentation().isEmpty()) {
      t << "        <simplesect>" << endl;
      t << "            <title>" << theTranslator->trDetailedDescription() << "</title>" << endl;

      writeDocbookDocBlock(t, nd->docFile(), nd->docLine(), nd, QSharedPointer<MemberDef>(), nd->documentation());

      t << "                <para>Definition at line " << nd->getDefLine() << " of file ";
      t << stripPath(nd->getDefFileName()) << "</para>" << endl;
      t << "                <para>The Documentation for this struct was generated from the following file: </para>";
      t << endl;

      t << "                <para><itemizedlist><listitem><para>" << stripPath(nd->getDefFileName())
        << "</para></listitem></itemizedlist></para>" << endl;

      t << "        </simplesect>" << endl;
   }
   t << "</section>" << endl;
}

static void generateDocbookForFile(QSharedPointer<FileDef> fd, QTextStream &ti)
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

   static const bool haveDot = Config::getBool("have-dot");

   if (fd->isReference()) {
      return;   // skip external references
   }

   QString fileDocbook = fd->getOutputFileBase() + ".xml";

   //Add the file Documentation info to index file
   ti << "        <xi:include href=\"" << fileDocbook << "\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;

   QString outputDirectory = Config::getString("docbook-output");
   QString fileName = outputDirectory + "/" + fd->getOutputFileBase() + ".xml";
   QString relPath = relativePathToRoot(fileName);

   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", csPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);
   writeDocbookHeader_ID(t, fd->getOutputFileBase());

   t << "    <title>";
   writeDocbookString(t, fd->name());
   t << " File Reference";
   t << "</title>" << endl;

   if (fd->includeFileList()) {
      for (auto &inc : *fd->includeFileList()) {
         t << "    <programlisting>#include ";

         if (inc.local) {
            t << "&quot;";
         } else {
            t << "&lt;";
         }

         t << convertToXML(inc.includeName);

         if (inc.local) {
            t << "&quot;";
         } else {
            t << "&gt;";
         }

         t << "</programlisting>" << endl;
      }
   }

   if (haveDot) {

      if (Config::getBool("dot-include")) {
         t << "<para>Include dependency diagram for " << convertToXML(fd->name()) << "</para>" << endl;
         DotInclDepGraph idepGraph(fd, false);
         idepGraph.writeGraph(t, GOF_BITMAP, EOF_DocBook, Config::getString("docbook-output"), fileName, relPath, false);
      }

      if (Config::getBool("dot-included-by")) {
         t << "<para>Included by dependency diagram for " << convertToXML(fd->name()) << "</para>" << endl;

         DotInclDepGraph ibdepGraph(fd, true);
         ibdepGraph.writeGraph(t, GOF_BITMAP, EOF_DocBook, Config::getString("docbook-output"), fileName, relPath, false);
      }
   }

   if (fd->getClassSDict().count() > 0) {
      writeInnerClasses(fd->getClassSDict(), t);
   }

   if (! fd->getNamespaceSDict().isEmpty()) {
      writeInnerNamespaces(fd->getNamespaceSDict(), t);
   }

   for (auto mg : fd->getMemberGroupSDict()) {
      generateDocbookSection(fd, t, mg->members(), "user-defined", false, mg->header(), mg->documentation());
   }

   for (auto ml : fd->getMemberLists()) {
      if ((ml->listType()&MemberListType_declarationLists) != 0) {
         generateDocbookSection(fd, t, ml, g_docbookSectionMapper.value(ml->listType()));
      }
   }

   t << "    <simplesect>" << endl;
   t << "        <title>" << theTranslator->trDetailedDescription() << "</title>" << endl;

   writeDocbookDocBlock(t, fd->briefFile(), fd->briefLine(), fd, QSharedPointer<MemberDef>(), fd->briefDescription());
   writeDocbookDocBlock(t, fd->docFile(), fd->docLine(), fd,  QSharedPointer<MemberDef>(), fd->documentation());

   if (Config::getBool("full-path-names")) {
      t << "    <para>Definition in file " << fd->getDefFileName() << "</para>" << endl;
   } else {
      t << "    <para>Definition in file " << stripPath(fd->getDefFileName()) << "</para>" << endl;
   }
   t << "    </simplesect>" << endl;

   if (Config::getBool("docbook-program-listing")) {
      t << "    <literallayout><computeroutput>" << endl;
      writeDocbookCodeBlock(t, fd);
      t << "    </computeroutput></literallayout>" << endl;
   }

   t << "</section>" << endl;
}

static void generateDocbookForGroup(QSharedPointer<GroupDef> gd, QTextStream &ti)
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
      QString fileDocbook = gd->getOutputFileBase() + ".xml";

      //Add the file Documentation info to index file
      ti << "        <xi:include href=\"" << fileDocbook << "\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;
   }

   QString outputDirectory = Config::getString("docbook-output");
   QString fileName = outputDirectory + "/" + gd->getOutputFileBase() + ".xml";
   QString relPath = relativePathToRoot(fileName);

   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", csPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);
   //t.setEncoding(QTextStream::UnicodeUTF8);
   writeDocbookHeader_ID(t, gd->getOutputFileBase());

   t << "    <title>" << convertToXML(gd->groupTitle()) << "</title>" << endl;

   if (Config::getBool("group-graphs") && Config::getBool("have-dot")) {
      t << "<para>Collaboration diagram for " << convertToXML(gd->groupTitle()) << "</para>" << endl;
      DotGroupCollaboration collaborationGraph(gd);
      collaborationGraph.writeGraph(t, GOF_BITMAP, EOF_DocBook, Config::getString("docbook-output"), fileName, relPath, false);
   }

   if (! gd->briefDescription().isEmpty()) {
      writeDocbookDocBlock(t, gd->briefFile(), gd->briefLine(), gd, QSharedPointer<MemberDef>(), gd->briefDescription());
   }

   if (! gd->documentation().isEmpty()) {
      t << "        <section>" << endl;
      t << "            <title>" << theTranslator->trDetailedDescription() << "</title>" << endl;
      writeDocbookDocBlock(t, gd->docFile(), gd->docLine(), gd, QSharedPointer<MemberDef>(), gd->documentation());
      t << "        </section>" << endl;
   }

   writeInnerFiles(gd->getFiles(), t);
   writeInnerClasses(gd->getClasses(), t);
   writeInnerNamespaces(gd->getNamespaces(), t);
   writeInnerPages(gd->getPages(), t);
   writeInnerGroups(gd->getSubGroups(), t);

   for (auto mg : gd->getMemberGroupSDict()) {
      generateDocbookSection(gd, t, mg->members(), "user-defined", false, mg->header(), mg->documentation());
   }

   for (auto ml : gd->getMemberLists()) {
      if ((ml->listType()&MemberListType_declarationLists) != 0) {
         generateDocbookSection(gd, t, ml, g_docbookSectionMapper.value(ml->listType()));
      }
   }

   for (auto ml : gd->getMemberLists()) {
      if ((ml->listType()&MemberListType_declarationLists) != 0) {
         generateDocbookSection(gd, t, ml, g_docbookSectionMapper.value(ml->listType()), true);
      }
   }

   writeInnerGroupFiles(gd->getSubGroups(), t);

   t << "</section>" << endl;
}

static void generateDocbookForDir(QSharedPointer<DirDef> dd, QTextStream &ti)
{
   if (dd->isReference()) {
      return;   // skip external references
   }

   static const bool directoryGraph = Config::getBool("directory-graph");
   static const bool haveDot        = Config::getBool("have-dot");

   QString fileDocbook = dd->getOutputFileBase() + ".xml";

   //Add the file Documentation info to index file
   ti << "        <xi:include href=\"" << fileDocbook << "\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;

   QString outputDirectory = Config::getString("docbook-output");
   QString fileName = outputDirectory + "/" + dd->getOutputFileBase() + ".xml";

   QFile f(fileName);
   QString relPath = relativePathToRoot(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", csPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);
   writeDocbookHeader_ID(t, dd->getOutputFileBase());

   t << "    <title>";
   t << theTranslator->trDirReference(dd->displayName());
   t << "</title>" << endl;

   if (directoryGraph && haveDot) {
      t << "<para>Directory dependency diagram for " << convertToXML(dd->displayName()) << "</para>" << endl;
      DotDirDeps dirdepGraph(dd);
      dirdepGraph.writeGraph(t, GOF_BITMAP, EOF_DocBook, Config::getString("docbook-output"), fileName, relPath, false);
   }

   writeInnerDirs(dd->subDirs(), t);
   writeInnerFiles(dd->getFiles(), t);

   t << "    <simplesect>" << endl;
   t << "        <title>" << theTranslator->trDetailedDescription() << "</title>" << endl;

   writeDocbookDocBlock(t, dd->briefFile(), dd->briefLine(), dd, QSharedPointer<MemberDef>(), dd->briefDescription());
   writeDocbookDocBlock(t, dd->docFile(), dd->docLine(), dd, QSharedPointer<MemberDef>(), dd->documentation());

   t << "    <para>Directory location is " << dd->name() << "</para>" << endl;
   t << "    </simplesect>" << endl;

   t << "</section>" << endl;
}

static void generateDocbookForPage(QSharedPointer<PageDef> pd, QTextStream &ti, bool isExample)
{
   // + name
   // + title
   // + documentation

   static const QString outputDocbook = Config::getString("docbook-output");

   if (pd->isReference()) {
      return;
   }

   QString pageName = pd->getOutputFileBase();

   if (pd->getGroupDef()) {
      pageName += "_" + pd->name();
   }

   if (pageName == "index") {
      pageName = "mainpage"; // to prevent overwriting the generated index page
   }

   QString fileName = outputDocbook + "/" + pageName + ".xml";

   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", csPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);

   QString pName = pageName;

   if (isExample) {
      QString fileDocbook = pName + ".xml";
      ti << "        <xi:include href=\"" << fileDocbook << "\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;
   }

   if (! pd->hasParentPage() && !isExample) {
      QString fileDocbook = pName + ".xml";

      //Add the file Documentation info to index file
      ti << "        <xi:include href=\"" << fileDocbook << "\" xmlns:xi=\"http://www.w3.org/2001/XInclude\"/>" << endl;
      writeDocbookHeaderMainpage(t);

   } else {
      QString pid;

      if(isExample)     {
         pid = pageName;
      } else {
         pid = pageName+"_1" + pageName;
      }

      writeDocbookHeader_ID(t, pid);
   }

   QSharedPointer<SectionInfo> si = Doxy_Globals::sectionDict.find(pd->name());

   if (si) {
      t << "    <title>" << convertToXML(si->title) << "</title>" << endl;
   } else {
      t << "    <title>" << convertToXML(pd->name()) << "</title>" << endl;
   }

   if (isExample) {
      writeDocbookDocBlock(t, pd->docFile(), pd->docLine(), pd, QSharedPointer<MemberDef>(), pd->documentation() + "\n\\include " + pd->name());

   } else {
      writeDocbookDocBlock(t, pd->docFile(), pd->docLine(), pd, QSharedPointer<MemberDef>(), pd->documentation());
   }
   writeInnerPages(pd->getSubPages(), t);

   if (! pd->hasParentPage() && !isExample) {
      t << endl << "</chapter>" << endl;

   } else {
      t << endl << "</section>" << endl;
   }
}

void generateDocbook_output()
{
   // + classes
   // + namespaces
   // + files
   // + groups
   // + related pages
   // - examples

   static QString outputDirectory   = Config::getString("docbook-output");
   static QString dbk_projectName   = Config::getString("project-name");

   static const bool showFiles      = Config::getBool("show-file-page");
   static const bool directoryGraph = Config::getBool("directory-graph");
   static const bool haveDot        = Config::getBool("have-dot");

   if (outputDirectory.isEmpty()) {
      outputDirectory = QDir::currentPath();

   } else {
      QDir dir(outputDirectory);

      if (!dir.exists()) {
         dir.setPath(QDir::currentPath());

         if (! dir.mkdir(outputDirectory)) {
            err("DocBook Output directory `%s' does not exist and can not be created\n", csPrintable(outputDirectory));
            Doxy_Work::stopDoxyPress();

         } else {
            msg("DOCBOOK Output directory `%s' created\n", csPrintable(outputDirectory));
         }

         dir.cd(outputDirectory);
      }

      outputDirectory = dir.absolutePath();
   }

   QDir dir(outputDirectory);

   if (! dir.exists()) {
      dir.setPath(QDir::currentPath());

      if (! dir.mkdir(outputDirectory)) {
         err("Unable to create directory %s\n", csPrintable(outputDirectory));
         return;
      }
   }

   QDir docbookDir(outputDirectory);
   createSubDirs(docbookDir);

   QString fileName = outputDirectory + "/index.xml";

   QFile f(fileName);
   f.setFileName(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", csPrintable(fileName), f.error());
      return;
   }

   QTextStream t(&f);
   //t.setEncoding(QTextStream::UnicodeUTF8);

   // write index header for Docbook which calls the structure file
   t << "<?xml version='1.0' encoding='UTF-8' standalone='no'?>" << endl;;
   t << "<book xmlns=\"http://docbook.org/ns/docbook\" version=\"5.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">" << endl;
   t << "    <info>" << endl;
   t << "    <title>" << dbk_projectName << "</title>" << endl;
   t << "    </info>" << endl;

   // NAMESPACE DOCUMENTATION
   // Namespace Documentation index header

   if (! Doxy_Globals::namespaceSDict.isEmpty()) {
      t << "    <chapter>" << endl;
      t << "        <title>Namespace Documentation</title>" << endl;
   }

   for (auto &nd : Doxy_Globals::namespaceSDict)  {
      msg("Generating Docbook output for namespace %s\n", csPrintable(nd->name()));
      generateDocbookForNamespace(nd, t);
   }

   // Namespace Documentation index footer
   if (! Doxy_Globals::namespaceSDict.isEmpty()) {
      t << "    </chapter>" << endl;
   }

   // mainpage documentation
   if (Doxy_Globals::mainPage) {
      msg("Generating Docbook output for the main page\n");
      generateDocbookForPage(Doxy_Globals::mainPage, t, false);
   }

   // page documentation
   for (auto &pd : Doxy_Globals::pageSDict) {
      msg("Generating Docbook output for page %s\n", csPrintable(pd->name()));
      generateDocbookForPage(pd, t, false);
   }

   // ** module group documentation

   // Module group Documentation index header
   if (! Doxy_Globals::groupSDict.isEmpty()) {
      t << "    <chapter>" << endl;
      t << "        <title>" << theTranslator->trModuleDocumentation() << "</title>" << endl;
   }

   for (auto gd : Doxy_Globals::groupSDict) {
      msg("Generating Docbook output for group %s\n", csPrintable(gd->name()));
      generateDocbookForGroup(gd, t);
   }

   // Module group Documentation index footer
   if (! Doxy_Globals::groupSDict.isEmpty()) {
      t << "    </chapter>" << endl;
   }

   // ** CLASS DOCUMENTATION

   {
      // Class Documentation index header
      if (! Doxy_Globals::classSDict.isEmpty()) {
         t << "    <chapter>" << endl;
         t << "        <title>" << theTranslator->trClassDocumentation() << "</title>" << endl;
      }

      for (auto cd : Doxy_Globals::classSDict) {
         generateDocbookForClass(cd, t);
      }

      // Class Documentation index footer
      if (! Doxy_Globals::classSDict.isEmpty()) {
         t << "    </chapter>" << endl;
      }
   }

   // file documentation
   if (showFiles) {

      // File Documentation index header
      if (! Doxy_Globals::inputNameList.isEmpty()) {
         t << "    <chapter>" << endl;
         t << "        <title>" << theTranslator->trFileDocumentation() << "</title>" << endl;
      }

      for (auto &fn : Doxy_Globals::inputNameList) {
         for (auto fd : *fn) {
            msg("Generating Docbook output for file %s\n", csPrintable(fd->name()));
            generateDocbookForFile(fd, t);
         }
      }

      // File Documentation index footer
      if (! Doxy_Globals::inputNameList.isEmpty()) {
         t << "    </chapter>" << endl;
      }
   }

   // DIRECTORY DOCUMENTATION
   if (directoryGraph && haveDot) {

      // Directory Documentation index header
      if (! Doxy_Globals::directories.isEmpty()) {
         t << "    <chapter>" << endl;
         t << "        <title>" << theTranslator->trDirDocumentation() << "</title>" << endl;
      }

      for (auto dir : Doxy_Globals::directories) {
         msg("Generate Docbook output for dir %s\n", csPrintable(dir->name()));
         generateDocbookForDir(dir, t);
      }

      //Module group Documentation index footer
      if (! Doxy_Globals::directories.isEmpty()) {
         t << "    </chapter>" << endl;
      }
   }

   // EXAMPLE PAGE DOCUMENTATION

   {
      //Example Page Documentation index header
      if (! Doxy_Globals::exampleSDict.isEmpty()) {
         t << "    <chapter>" << endl;
         t << "        <title>" << theTranslator->trExampleDocumentation() << "</title>" << endl;
      }

      for (auto &pd : Doxy_Globals::exampleSDict) {
         msg("Generating Docbook output for example %s\n", csPrintable(pd->name()));
         generateDocbookForPage(pd, t, true);
      }

      // Example Page Documentation index footer
      if (! Doxy_Globals::exampleSDict.isEmpty()) {
         t << "    </chapter>" << endl;
      }
   }

   t << "</book>" << endl;
}
