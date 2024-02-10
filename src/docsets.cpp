/************************************************************************
*
* Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
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

#include <docsets.h>

#include <config.h>
#include <doxy_globals.h>
#include <message.h>
#include <util.h>

DocSets::DocSets()
{
   m_dc = 0;
   m_id = 0;
}

DocSets::~DocSets()
{
}

void DocSets::initialize()
{
   // get config options
   static const QString projectName    = Config::getString("project-name");
   static const QString bundleId       = Config::getString("docset-bundle-id");
   static const QString feedName       = Config::getString("docset-feedname");
   static const QString publisherId    = Config::getString("docset-publisher-id");
   static const QString publisherName  = Config::getString("docset-publisher-name");
   static const QString projectVersion = Config::getString("project-version");

   // write Makefile
   {
      static const QString mfName = Config::getString("html-output") + "/Makefile";

      QFile makefile(mfName);

      if (! makefile.open(QIODevice::WriteOnly)) {
         err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(mfName), makefile.error());
         Doxy_Work::stopDoxyPress();
      }

      QTextStream ts(&makefile);

      ts << "DOCSET_NAME=" << bundleId << ".docset\n"
         "DOCSET_CONTENTS=$(DOCSET_NAME)/Contents\n"
         "DOCSET_RESOURCES=$(DOCSET_CONTENTS)/Resources\n"
         "DOCSET_DOCUMENTS=$(DOCSET_RESOURCES)/Documents\n"
         "DESTDIR=~/Library/Developer/Shared/Documentation/DocSets\n"
         "XCODE_INSTALL=\"$(shell xcode-select -print-path)\"\n"
         "\n"
         "all: docset\n"
         "\n"
         "docset:\n"
         "\tmkdir -p $(DOCSET_DOCUMENTS)\n"
         "\tcp Nodes.xml $(DOCSET_RESOURCES)\n"
         "\tcp Tokens.xml $(DOCSET_RESOURCES)\n"
         "\tcp Info.plist $(DOCSET_CONTENTS)\n"
         "\ttar --exclude $(DOCSET_NAME) \\\n"
         "\t    --exclude Nodes.xml \\\n"
         "\t    --exclude Tokens.xml \\\n"
         "\t    --exclude Info.plist \\\n"
         "\t    --exclude Makefile -c -f - . \\\n"
         "\t    | (cd $(DOCSET_DOCUMENTS); tar xvf -)\n"
         "\t$(XCODE_INSTALL)/usr/bin/docsetutil index $(DOCSET_NAME)\n"
         "\trm -f $(DOCSET_DOCUMENTS)/Nodes.xml\n"
         "\trm -f $(DOCSET_DOCUMENTS)/Info.plist\n"
         "\trm -f $(DOCSET_DOCUMENTS)/Makefile\n"
         "\trm -f $(DOCSET_RESOURCES)/Nodes.xml\n"
         "\trm -f $(DOCSET_RESOURCES)/Tokens.xml\n"
         "\n"
         "clean:\n"
         "\trm -rf $(DOCSET_NAME)\n"
         "\n"
         "install: docset\n"
         "\tmkdir -p $(DESTDIR)\n"
         "\tcp -R $(DOCSET_NAME) $(DESTDIR)\n"
         "\n"
         "uninstall:\n"
         "\trm -rf $(DESTDIR)/$(DOCSET_NAME)\n"
         "\n"
         "always:\n";
   }

   // write Info.plist
   {
      static const QString plName = Config::getString("html-output") + "/Info.plist";
      QFile plist(plName);

      if (! plist.open(QIODevice::WriteOnly)) {
         err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(plName), plist.error());
         Doxy_Work::stopDoxyPress();
      }
      QTextStream ts(&plist);

      ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         "<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\"\n"
         "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
         "<plist version=\"1.0\">\n"
         "<dict>\n"
         "     <key>CFBundleName</key>\n"
         "     <string>" << projectName << "</string>\n"
         "     <key>CFBundleIdentifier</key>\n"
         "     <string>" << bundleId << "</string>\n"
         "     <key>CFBundleVersion</key>\n"
         "     <string>" << projectVersion << "</string>\n"
         "     <key>DocSetFeedName</key>\n"
         "     <string>" << feedName << "</string>\n"
         "     <key>DocSetPublisherIdentifier</key>\n"
         "     <string>" << publisherId << "</string>\n"
         "     <key>DocSetPublisherName</key>\n"
         "     <string>" << publisherName << "</string>\n"
         // markers for Dash
         "     <key>DashDocSetFamily</key>\n"
         "     <string>DoxyPress</string>\n"
         "     <key>DocSetPlatformFamily</key>\n"
         "     <string>DoxyPress</string>\n"
         "</dict>\n"
         "</plist>\n";
   }

   // start Nodes.xml
   static const QString notes = Config::getString("html-output") + "/Nodes.xml";
   m_nf.setFileName(notes);

   if (! m_nf.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(notes), m_nf.error());
      Doxy_Work::stopDoxyPress();
   }

   // QString indexName = Config::getBool("generate-treeview") ? "main" : "index";
   QString indexName = "index";

   m_nts.setDevice(&m_nf);
   m_nts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
   m_nts << "<DocSetNodes version=\"1.0\">" << endl;
   m_nts << "  <TOC>" << endl;
   m_nts << "    <Node>" << endl;
   m_nts << "      <Name>Root</Name>" << endl;
   m_nts << "      <Path>" << indexName << Doxy_Globals::htmlFileExtension << "</Path>" << endl;
   m_nts << "      <Subnodes>" << endl;
   m_dc = 1;

   m_firstNode.resize(m_dc);
   m_firstNode[0] = true;

   //
   static const QString tokens = Config::getString("html-output") + "/Tokens.xml";
   m_tf.setFileName(tokens);

   if (! m_tf.open(QIODevice::WriteOnly)) {
      err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(tokens), m_tf.error());
      Doxy_Work::stopDoxyPress();
   }

   m_tts.setDevice(&m_tf);
   m_tts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
   m_tts << "<Tokens version=\"1.0\">" << endl;
}

void DocSets::finalize()
{
   if (! m_firstNode.at(m_dc - 1)) {
      m_nts << indent() << " </Node>" << endl;
   }

   m_dc--;
   m_nts << "      </Subnodes>" << endl;
   m_nts << "    </Node>" << endl;
   m_nts << "  </TOC>" << endl;
   m_nts << "</DocSetNodes>" << endl;
   m_nf.close();

   m_tts << "</Tokens>" << endl;
   m_tf.close();
}

QString DocSets::indent()
{
   QString result;
   result.fill(' ', (m_dc + 2) * 2);
   return result;
}

void DocSets::incContentsDepth()
{
   //printf("DocSets::incContentsDepth() m_dc=%d\n",m_dc);
   ++m_dc;
   m_nts << indent() << "<Subnodes>" << endl;

   m_firstNode.resize(m_dc);

   if (m_dc > 0) {
      m_firstNode[m_dc - 1] = true;
   }
}

void DocSets::decContentsDepth()
{
   if (!m_firstNode.at(m_dc - 1)) {
      m_nts << indent() << " </Node>" << endl;
   }

   m_nts << indent() << "</Subnodes>" << endl;
   --m_dc;
   //printf("DocSets::decContentsDepth() m_dc=%d\n",m_dc);
}

void DocSets::addContentsItem(bool isDir, const QString &name, const QString &ref, const QString &file, const QString &anchor,
                              bool unused, QSharedPointer<Definition>, DirType category)
{
   (void) isDir;
   (void) unused;
   (void) category;

   if (! ref.isEmpty()) {

      if (! m_firstNode.at(m_dc - 1)) {
         m_nts << indent() << " </Node>" << endl;
      }

      m_firstNode[m_dc - 1] = false;

      m_nts << indent() << " <Node>" << endl;
      m_nts << indent() << "  <Name>" << convertToXML(name) << "</Name>" << endl;

      if (! file.isEmpty() && file[0] == '^') {
         // URL marker
         m_nts << indent() << "  <URL>" << convertToXML(file.mid(1) )
               << "</URL>" << endl;

      } else {
         // relative file
         m_nts << indent() << "  <Path>";

         if (! file.isEmpty() && file[0] == '!') {
            // user specified file
            m_nts << convertToXML(file.mid(1));

         } else if (! file.isEmpty()) {
            // generated file
            m_nts << file << Doxy_Globals::htmlFileExtension;
         }

         m_nts << "</Path>" << endl;
         if (! file.isEmpty() && ! anchor.isEmpty()) {
            m_nts << indent() << "  <Anchor>" << anchor << "</Anchor>" << endl;
         }
      }
   }
}

void DocSets::addIndexItem(QSharedPointer<Definition> context, QSharedPointer<MemberDef> md, const QString &, const QString &)
{
   if (context == nullptr && md == nullptr) {
      return;
   }

   QSharedPointer<FileDef> fd;
   QSharedPointer<ClassDef> cd;
   QSharedPointer<NamespaceDef> nd;

   if (md) {
      fd = md->getFileDef();
      cd = md->getClassDef();
      nd = md->getNamespaceDef();

      if (! md->isLinkable()) {
         return;   // internal symbol
      }
   }

   QString scope;
   QString type;
   QString decl;

   // determine language
   QString lang;
   SrcLangExt langExt = SrcLangExt_Cpp;

   if (md) {
      langExt = md->getLanguage();

   } else if (context) {
      langExt = context->getLanguage();

   }

   switch (langExt) {
      case SrcLangExt_Cpp:
      case SrcLangExt_ObjC: {
         if (md && (md->isObjCMethod() || md->isObjCProperty())) {
            lang = "occ";   // Objective C/C++

         } else if (fd && fd->name().endsWith(".c", Qt::CaseInsensitive)) {
            lang = "c";     // Plain C

         } else if (cd == nullptr && nd == nullptr) {
            lang = "c";     // Plain C symbol outside any class or namespace

         } else {
            lang = "cpp";   // C++
         }
      }
      break;

      case SrcLangExt_IDL:
         lang = "idl";
         break;

      case SrcLangExt_CSharp:
         lang = "csharp";
         break;

      case SrcLangExt_PHP:
         lang = "php";
         break;        // PHP 4/5

      case SrcLangExt_D:
         lang = "d";
         break;

      case SrcLangExt_Java:
         lang = "java";
         break;

      case SrcLangExt_JS:
         lang = "javascript";
         break;

      case SrcLangExt_Python:
         lang = "python";
         break;

      case SrcLangExt_Fortran:
         lang = "fortran";
         break;

      case SrcLangExt_XML:
         lang = "xml";
         break;        // DBUS XML

      case SrcLangExt_Tcl:
         lang = "tcl";
         break;

     case SrcLangExt_Markdown:
         lang = "markdown";
         break;

     case SrcLangExt_Make:
         lang = "make";
         break;

      case SrcLangExt_Docs:
         lang = "docs";
         break;

      case SrcLangExt_Unknown:
         lang = "unknown";
         break;
   }

   if (md) {
      if (context == nullptr) {
         if (md->getGroupDef()) {
            context = md->getGroupDef();

         } else if (md->getFileDef()) {
            context = md->getFileDef();
         }
      }

      if (context == nullptr) {
         return;   // should not happen
      }

      switch (md->memberType()) {
         case MemberDefType::Define:
            type = "macro";
            break;

         case MemberDefType::Function:
            if (cd && (cd->compoundType() == CompoundType::Interface ||
                       cd->compoundType() == CompoundType::Class)) {

               if (md->isStatic()) {
                  type = "clm";      // class member
               } else {
                  type = "instm";    // instance member
               }

            } else if (cd && cd->compoundType() == CompoundType::Protocol) {
               if (md->isStatic()) {
                  type = "intfcm";   // interface class member
               } else {
                  type = "intfm";    // interface member
               }

            } else {
               type = "func";
            }
            break;

         case MemberDefType::Variable:
            type = "data";
            break;

         case MemberDefType::Typedef:
            type = "tdef";
            break;

         case MemberDefType::Enumeration:
            type = "enum";
            break;

         case MemberDefType::EnumValue:
            type = "econst";
            break;

         // case MemberDef::Prototype:
         //   type = "prototype";
         //   break;

         case MemberDefType::Signal:
            type = "signal";
            break;

         case MemberDefType::Slot:
            type = "slot";
            break;

         case MemberDefType::DCOP:
            type = "dcop";
            break;

         case MemberDefType::Property:
            if (cd && cd->compoundType() == CompoundType::Protocol) {
               type = "intfp";   // interface property
            } else {
               type = "instp";   // instance property
            }
            break;

         case MemberDefType::Event:
            type = "event";
            break;

         case MemberDefType::Interface:
            type = "ifc";
            break;

         case MemberDefType::Service:
            type = "svc";
            break;
      }

      if (md->isFriend()) {
         type.prepend("ffunc ");
      }

      cd = md->getClassDef();
      nd = md->getNamespaceDef();

      if (cd) {
         scope = cd->qualifiedName();
      } else if (nd) {
         scope = nd->name();
      }

      QSharedPointer<MemberDef> declMd = md->memberDeclaration();

      if (declMd == nullptr) {
         declMd = md;
      }


      fd = md->getFileDef();
      if (fd) {
         decl = fd->name();
      }

      writeToken(m_tts, md, type, lang, scope, md->anchor(), decl);

   } else if (context && context->isLinkable()) {

      if (fd == nullptr && context->definitionType() == Definition::TypeFile) {
         fd = context.dynamicCast<FileDef>();
      }

      if (cd == nullptr && context->definitionType() == Definition::TypeClass) {
         cd = context.dynamicCast<ClassDef>();
      }

      if (nd == nullptr && context->definitionType() == Definition::TypeNamespace) {
         nd = context.dynamicCast<NamespaceDef>();
      }

      if (fd) {
         type = "file";

      } else if (cd) {
         scope = cd->qualifiedName();

         if (cd->isTemplate()) {
            type = "tmplt";

         } else if (cd->compoundType() == CompoundType::Protocol) {
            type = "intf";

            if (scope.endsWith("-p")) {
               scope.chop(2);
            }

         } else if (cd->compoundType() == CompoundType::Interface) {
            type = "cl";

         } else if (cd->compoundType() == CompoundType::Category) {
            type = "cat";

         } else {
            type = "cl";

         }

         const IncludeInfo &item = cd->getIncludeInfo();
         decl = item.includeName;

      } else if (nd) {
         scope = nd->name();
         type  = "ns";
      }

      if (! m_scopes.contains(context->getOutputFileBase())) {
         writeToken(m_tts, context, type, lang, scope, "", decl);
         m_scopes.insert(context->getOutputFileBase());

      }
   }
}

void DocSets::writeToken(QTextStream &t, QSharedPointer<Definition> d, const QString &type, const QString &lang,
                         const QString &scope, const QString &anchor, const QString &decl)
{
   t << "  <Token>" << endl;
   t << "    <TokenIdentifier>" << endl;

   QString name = d->name();

   if (name.endsWith("-p")) {
      name.chop(2);
   }

   t << "      <Name>" << convertToXML(name) << "</Name>" << endl;

   if (! lang.isEmpty()) {
      t << "      <APILanguage>" << lang << "</APILanguage>" << endl;
   }

   if (! type.isEmpty()) {
      t << "      <Type>" << type << "</Type>" << endl;
   }

   if (! scope.isEmpty()) {
      t << "      <Scope>" << convertToXML(scope) << "</Scope>" << endl;
   }

   t << "    </TokenIdentifier>" << endl;
   t << "    <Path>" << d->getOutputFileBase()
     << Doxy_Globals::htmlFileExtension << "</Path>" << endl;

   if (! anchor.isEmpty()) {
      t << "    <Anchor>" << anchor << "</Anchor>" << endl;
   }

   QString tooltip = d->briefDescriptionAsTooltip();

   if (! tooltip.isEmpty()) {
      t << "    <Abstract>" << convertToXML(tooltip) << "</Abstract>" << endl;
   }

   if (! decl.isEmpty() ) {
      t << "    <DeclaredIn>" << convertToXML(decl) << "</DeclaredIn>" << endl;
   }

   t << "  </Token>" << endl;
}
