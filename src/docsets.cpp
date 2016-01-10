/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim 
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

#include <docsets.h>

#include <config.h>
#include <doxy_globals.h>
#include <message.h>
#include <util.h>

DocSets::DocSets()
{
   m_nf = 0;
   m_tf = 0;

   m_dc = 0;
   m_id = 0;   
}

DocSets::~DocSets()
{
   delete m_nf;
   delete m_tf;
}

void DocSets::initialize()
{
   // get config options
   QString projectName    = Config::getString("project-name");
   QString bundleId       = Config::getString("docset-bundle-id");
   QString feedName       = Config::getString("docset-feedname");
   QString publisherId    = Config::getString("docset-publisher-id");
   QString publisherName  = Config::getString("docset-publisher-name");
   QString projectVersion = Config::getString("project-version");
  
   // write Makefile
   {
      QString mfName = Config::getString("html-output") + "/Makefile";

      QFile makefile(mfName);

      if (! makefile.open(QIODevice::WriteOnly)) {
         err("Unable to open file for writing %s, error: %d\n", qPrintable(mfName), makefile.error());
         exit(1);
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
      QString plName = Config::getString("html-output") + "/Info.plist";
      QFile plist(plName);

      if (! plist.open(QIODevice::WriteOnly)) {
         err("Unable to open file for writing %s, error: %d\n", qPrintable(plName), plist.error());
         exit(1);
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
   QString notes = Config::getString("html-output") + "/Nodes.xml";
   m_nf = new QFile(notes);

   if (! m_nf->open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", qPrintable(notes), m_nf->error());
      exit(1);
   }

   // QString indexName = Config::getBool("generate-treeview") ? "main" : "index";
   QString indexName = "index";

   m_nts.setDevice(m_nf);
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

   QString tokens = Config::getString("html-output") + "/Tokens.xml";
   m_tf = new QFile(tokens);
  
   if (! m_tf->open(QIODevice::WriteOnly)) {
      err("Unable to open file for writing %s, error: %d\n", qPrintable(tokens), m_tf->error());
      exit(1);
   }

   m_tts.setDevice(m_tf);
   m_tts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
   m_tts << "<Tokens version=\"1.0\">" << endl;
}

void DocSets::finalize()
{
   if (!m_firstNode.at(m_dc - 1)) {
      m_nts << indent() << " </Node>" << endl;
   }

   m_dc--;
   m_nts << "      </Subnodes>" << endl;
   m_nts << "    </Node>" << endl;
   m_nts << "  </TOC>" << endl;
   m_nts << "</DocSetNodes>" << endl;
   m_nf->close();

   delete m_nf;
   m_nf = 0;

   m_tts << "</Tokens>" << endl;
   m_tf->close();

   delete m_tf;
   m_tf = 0;
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
                              bool, bool, QSharedPointer<Definition>)
{    
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
   if (md == 0 && context == 0) {
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

         } else if (fd && fd->name().right(2).toLower() == ".c") {
            lang = "c";   // Plain C

         } else if (cd == 0 && nd == 0) {
            lang = "c";   // Plain C symbol outside any class or namespace

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
         break;        // PHP4/5

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

      case SrcLangExt_Unknown:
         lang = "unknown";
         break;    
   }

   if (md) {
      if (context == 0) {
         if (md->getGroupDef()) {
            context = md->getGroupDef();
         } else if (md->getFileDef()) {
            context = md->getFileDef();
         }
      }
      if (context == 0) {
         return;   // should not happen
      }

      switch (md->memberType()) {
         case MemberType_Define:
            type = "macro";
            break;
         case MemberType_Function:
            if (cd && (cd->compoundType() == ClassDef::Interface ||
                       cd->compoundType() == ClassDef::Class)) {
               if (md->isStatic()) {
                  type = "clm";   // class member
               } else {
                  type = "instm";   // instance member
               }
            } else if (cd && cd->compoundType() == ClassDef::Protocol) {
               if (md->isStatic()) {
                  type = "intfcm";   // interface class member
               } else {
                  type = "intfm";   // interface member
               }
            } else {
               type = "func";
            }
            break;
         case MemberType_Variable:
            type = "data";
            break;
         case MemberType_Typedef:
            type = "tdef";
            break;
         case MemberType_Enumeration:
            type = "enum";
            break;
         case MemberType_EnumValue:
            type = "econst";
            break;
         //case MemberDef::Prototype:
         //  type="prototype"; break;
         case MemberType_Signal:
            type = "signal";
            break;
         case MemberType_Slot:
            type = "slot";
            break;
         case MemberType_Friend:
            type = "ffunc";
            break;
         case MemberType_DCOP:
            type = "dcop";
            break;
         case MemberType_Property:
            if (cd && cd->compoundType() == ClassDef::Protocol) {
               type = "intfp";   // interface property
            } else {
               type = "instp";   // instance property
            }
            break;
         case MemberType_Event:
            type = "event";
            break;
         case MemberType_Interface:
            type = "ifc";
            break;
         case MemberType_Service:
            type = "svc";
            break;
      }
      cd = md->getClassDef();
      nd = md->getNamespaceDef();

      if (cd) {
         scope = cd->qualifiedName();
      } else if (nd) {
         scope = nd->name();
      }

      QSharedPointer<MemberDef> declMd = md->memberDeclaration();

      if (declMd == 0) {
         declMd = md;
      }

      {
         fd = md->getFileDef();
         if (fd) {
            decl = fd->name();
         }
      }

      writeToken(m_tts, md, type, lang, scope, md->anchor(), decl);

   } else if (context && context->isLinkable()) {

      if (fd == 0 && context->definitionType() == Definition::TypeFile) {
         fd = context.dynamicCast<FileDef>();
      }

      if (cd == 0 && context->definitionType() == Definition::TypeClass) {
         cd = context.dynamicCast<ClassDef>();
      }

      if (nd == 0 && context->definitionType() == Definition::TypeNamespace) {
         nd = context.dynamicCast<NamespaceDef>();
      }

      if (fd) {
         type = "file";

      } else if (cd) {
         scope = cd->qualifiedName();

         if (cd->isTemplate()) {
            type = "tmplt";

         } else if (cd->compoundType() == ClassDef::Protocol) {
            type = "intf";
            if (scope.right(2) == "-p") {
               scope = scope.left(scope.length() - 2);
            }

         } else if (cd->compoundType() == ClassDef::Interface) {
            type = "cl";

         } else if (cd->compoundType() == ClassDef::Category) {
            type = "cat";

         } else {
            type = "cl";

         }

         IncludeInfo *ii = cd->includeInfo();
         if (ii) {
            decl = ii->includeName;           
         }

      } else if (nd) {
         scope = nd->name();
         type = "ns";
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

   if (name.right(2) == "-p") {
      name = name.left(name.length() - 2);
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

   if (! decl.isNull() ) {
      t << "    <DeclaredIn>" << convertToXML(decl) << "</DeclaredIn>" << endl;
   }

   t << "  </Token>" << endl;
}
