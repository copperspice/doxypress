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
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#ifndef TRANSLATOR_BR_H
#define TRANSLATOR_BR_H

class TranslatorBrazilian : public Translator
{
 public:

   // --- Language control methods -------------------

   /*! Used for identification of the language. May resemble
    * the string returned by latexBabelPackage(), but it is not used
    * for the same purpose. The identification should not be translated.
    * It should be replaced by the name of the language in English
    * (e.g. Czech, Japanese, Russian, etc.). It should be equal to
    * the identification in language.h.
    */
   virtual QByteArray idLanguage() {
      return "brazilian";
   }

   /*! Used to get the command(s) for the language support. This method
    *  was designed for languages which do not prefer babel package.
    *  If this methods returns empty string, then the latexBabelPackage()
    *  method is used to generate the command for using the babel package.
    */
   virtual QByteArray latexLanguageSupportCommand() {
      return "\\usepackage[brazil]{babel}";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Funções Relacionadas";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Note que estas não são funções membros.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Descrição Detalhada";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Definições de Tipos";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Enumerações";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Métodos";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Campos";
      } else {
         return "Atributos";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Mais...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Lista de todos os Membros";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Lista dos Membros";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Esta é a lista de todos os membros de ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", incluindo os membros herdados.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Gerado automaticamente por DoxyPress";
      if (s) {
         result += (QByteArray)" para " + s;
      }
      result += " a partir de seu código-fonte.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "[Enumeração]";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "[Valor enumerado]";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "definido(a) em";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Módulos";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Hierarquia de Classes";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config::getBool("optimize-c")) {
         return "Estruturas de Dados";
      } else {
         return "Lista de Componentes";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Lista de Arquivos";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config::getBool("optimize-c")) {
         return "Campos de Dados";
      } else {
         return "Componentes Membros";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config::getBool("optimize-c")) {
         return "Globais";
      } else {
         return "Membros dos Arquivos";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Páginas relacionadas";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Exemplos";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Busca";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Esta lista de hierarquias está parcialmente ordenada (ordem alfabética):";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Esta é a lista de todos os arquivos ";
      if (!extractAll) {
         result += "documentados ";
      }
      result += "e suas respectivas descrições:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config::getBool("optimize-c")) {
         return "Aqui estão as estruturas de dados, uniões e suas respectivas descrições:";
      } else {
         return "Aqui estão as classes, estruturas, uniões e interfaces e suas respectivas descrições:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Esta é a lista de ";

      if (Config::getBool("optimize-c")) {
         result += "todas as estruturas e campos de uniões ";
         if (!extractAll) {
            result += "documentas";
         }
      } else {
         result += "todos os membros de classes ";
         if (!extractAll) {
            result += "documentos";
         }
      }
      result += " com referências para ";
      if (!extractAll) {
         if (Config::getBool("optimize-c")) {
            result += "a documentação da estrutura/união de cada campo:";
         } else {
            result += "a documentação da classe de cada membro:";
         }
      } else {
         if (Config::getBool("optimize-c")) {
            result += "a estrutura/união a que pertencem:";
         } else {
            result += "a classe a que pertencem:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Esta é a lista de ";
      if (Config::getBool("optimize-c")) {
         result += "todas as funções, variáveis, definições, enumerações e definições de tipos ";
         if (!extractAll) {
            result += "documentadas ";
         }
      } else {
         result += "todos os membros dos arquivos ";
         if (!extractAll) {
            result += "documentados ";
         }
      }
      result += "com referências para ";
      if (extractAll) {
         result += "seus respectivos arquivos:";
      } else {
         result += "suas respectivas documentações:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Esta é a lista com todos os exemplos:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Esta é a lista com toda a documentação relacionadas:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Esta é a lista com todos os módulos:";
   }

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "\nDocumentação";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Índice dos Módulos";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Índice Hierárquico";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config::getBool("optimize-c")) {
         return "Índice das Estruturas de Dados";
      } else {
         return "Índice dos Componentes";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Índice dos Arquivos";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Módulos";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Estruturas";
      } else {
         return "Classes";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Arquivos";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Exemplos";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Documentação Relacionada";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Guia de Referência";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Definições e Macros";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Definições de Tipos";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Enumerações";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Funções";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Variáveis";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Valores de enumerações";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Definições e macros";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Definições dos tipos";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Enumerações";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Funções";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Variáveis";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config::getBool("optimize-c")) {
         return "Estruturas de Dados";
      } else {
         return "Componentes";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Gerado em " + date;
      if (projName) {
         result += (QByteArray)" para " + projName;
      }
      result += (QByteArray)" por";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Diagrama de Hierarquia para " + clName + ":";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Apenas para uso interno.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Aviso";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Versão";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Data";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Retorna";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Veja também";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Parâmetros";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Exceções";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Gerado por";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Lista de Namespaces";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Esta é a lista de todos os Namespaces ";
      if (!extractAll) {
         result += "documentados ";
      }
      result += "com suas respectivas descrições:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Amigas";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Amigas e Funções Relacionadas";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result = "Referência da";
      if (isTemplate) {
         result += " Template de";
      }
      switch (compType) {
         case ClassDef::Class:
            result += " Classe ";
            break;
         case ClassDef::Struct:
            result += " Estrutura ";
            break;
         case ClassDef::Union:
            result += " União ";
            break;
         case ClassDef::Interface:
            result += " Interface ";
            break;
         case ClassDef::Protocol:
            result += " Protocolo ";
            break;
         case ClassDef::Category:
            result += " Categoria ";
            break;
         case ClassDef::Exception:
            result += " Exceção ";
            break;
         default:
            break;
      }
      result += (QByteArray)clName;
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = "Referência do Arquivo ";
      result += fileName;
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = "Refência do Namespace ";
      result += namespaceName;
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Métodos Públicos";
   }
   virtual QByteArray trPublicSlots() {
      return "Slots Públicos";
   }
   virtual QByteArray trSignals() {
      return "Sinais";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Métodos Públicos Estáticos";
   }
   virtual QByteArray trProtectedMembers() {
      return "Métodos Protegidos";
   }
   virtual QByteArray trProtectedSlots() {
      return "Slots Protegidos";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Métodos Protegidos Estáticos";
   }
   virtual QByteArray trPrivateMembers() {
      return "Métodos Privados";
   }
   virtual QByteArray trPrivateSlots() {
      return "Slots Privados";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Métodos Privados Estáticos";
   }

   /*! this function is used to produce a comma-separated list of items.
    *  use generateMarker(i) to indicate where item i should be put.
    */
   virtual QByteArray trWriteList(int numEntries) {
      QByteArray result;
      int i;
      // the inherits list contain `numEntries' classes
      for (i = 0; i < numEntries; i++) {
         // use generateMarker to generate placeholders for the class links!
         result += generateMarker(i); // generate marker for entry i in the list
         // (order is left to right)

         if (i != numEntries - 1) { // not the last entry, so we need a separator
            if (i < numEntries - 2) { // not the fore last entry
               result += ", ";
            } else {            // the fore last entry
               result += " e ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Herdeiro de " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Superclasse de " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Reimplementação de " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Reimplementado por " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Membros do Namespace";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Esta é a lista com todos os membros do Namespace ";
      if (!extractAll) {
         result += "documentados ";
      }
      result += "com referências para ";
      if (extractAll) {
         result += "a documentação de seus respectivos Namespaces:";
      } else {
         result += "os seus respectivos namespaces:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Namespaces";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Namespaces";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Namespaces";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990728
   //////////////////////////////////////////////////////////////////////////

   /*! This is put at the bottom of a class documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFiles(ClassDef::CompoundType compType,
                                           bool single) {
      // here s is one of " Class", " Struct" or " Union"
      // single is true implies a single file
      QByteArray result = (QByteArray)"A documentação para esta ";
      switch (compType) {
         case ClassDef::Class:
            result += "classe ";
            break;
         case ClassDef::Struct:
            result += "estrutura ";
            break;
         case ClassDef::Union:
            result += "união ";
            break;
         case ClassDef::Interface:
            result += "interface ";
            break;
         case ClassDef::Protocol:
            result += "protocolo ";
            break;
         case ClassDef::Category:
            result += "categoria ";
            break;
         case ClassDef::Exception:
            result += "exceção ";
            break;
         default:
            break;
      }
      result += " foi gerada a partir ";
      if (single) {
         result += "do seguinte arquivo:";
      } else {
         result += "dos seguintes arquivos:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Valores Retornados";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Página Principal";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "pag.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Definição na linha @0 do arquivo @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Definição no arquivo @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      /*
      * This note is for brazilians only.
      * Esta é uma boa tradução para "deprecated"?
      */
      return "Descontinuado(a)";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Diagrama de colaboração para " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Gráfico de dependência de inclusões para " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Construtores & Destrutores";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Vá para o código-fonte deste arquivo.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Vá para a documentação deste arquivo.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Pré-Condição";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Pós-Condição";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Invariante";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Valor Inicial:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "código";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Gráfico de Hierarquia de Classes";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Vá para o Gráfico de Hierarquia de Classes";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Vá para a Hierarquia de Classes (texto)";
   }
   virtual QByteArray trPageIndex() {
      return "Índice da Página";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Observação";
   }
   virtual QByteArray trPublicTypes() {
      return "Tipos Públicos";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config::getBool("optimize-c")) {
         return "Campos de Dados";
      } else {
         return "Atributos Públicos";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Atributos Estáticos Públicos";
   }
   virtual QByteArray trProtectedTypes() {
      return "Tipos Protegidos";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Atributos Protegidos";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Atributos Estáticos Protegidos";
   }
   virtual QByteArray trPrivateTypes() {
      return "Tipos Privados";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Atributos Privados";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Atributos Privados Estáticos";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a todo item */
   virtual QByteArray trTodo() {
      return "Futuras Atividades";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Lista de Futuras Atividades";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Referenciado por";
   }
   virtual QByteArray trRemarks() {
      return "Anotações";
   }
   virtual QByteArray trAttention() {
      return "Atenção";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Este grafo mostra quais arquivos estão direta ou indiretamente relacionados com este arquivo:";
   }
   virtual QByteArray trSince() {
      return "Desde";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Legenda do Grafo";
   }
   /*! page explaining how the dot graph's should be interpreted */
   virtual QByteArray trLegendDocs() {
      return
         "Esta página explica como interpretar os grafos gerados pelo DoxyPress.<p>\n"
         "Considere o seguinte exemplo:\n"
         "\\code\n"
         "/*! Invisible class because of truncation */\n"
         "class Invisible { };\n\n"
         "/*! Truncated class, inheritance relation is hidden */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Class not documented with DoxyPress comments */\n"
         "class Undocumented { };\n\n"
         "/*! Class that is inherited using public inheritance */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! A template class */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Class that is inherited using protected inheritance */\n"
         "class ProtectedBase { };\n\n"
         "/*! Class that is inherited using private inheritance */\n"
         "class PrivateBase { };\n\n"
         "/*! Class that is used by the Inherited class */\n"
         "class Used { };\n\n"
         "/*! Super class that inherits a number of other classes */\n"
         "class Inherited : public PublicBase,\n"
         "                  protected ProtectedBase,\n"
         "                  private PrivateBase,\n"
         "                  public Undocumented,\n"
         "                  public Templ<int>\n"
         "{\n"
         "  private:\n"
         "    Used *m_usedClass;\n"
         "};\n"
         "\\endcode\n"
         "Isto irá gerar o seguinte gráfo:"
         "<p><center><img src=\"graph_legend." + Config::getEnum("dot-image-format").toUtf8() + "\"></center>\n"
         "<p>\n"
         "As caixas no grafo acima tem as seguintes interpretações:\n"
         "<ul>\n"
         "<li>Uma caixa inteiramente preta representa a estrutura ou a classe para qual "
         "o grafo foi gerado.\n"
         "<li>Uma caixa com bordas pretas denota uma estrutura ou classe documentada.\n"
         "<li>Uma caixa com bordas cinzas denota uma estrutura ou classe não documentada.\n"

         "<li>Uma caixa com bordas vermelhas denota uma estrutura ou classe documentada para\n"
         "a qual nem todas as heranças ou componentes são mostradas no grafo. Um grafo é "
         "truncado quando este é maior que o tamanho especificado."
         "</ul>\n"
         "As setas tem os seguintes significados:\n"
         "<ul>\n"
         "<li>Uma seta azul escura é utilizada para denotar uma relação de herança "
         "pública entre duas classes.\n"
         "<li>Uma seta verde escura é utilizada para denotar uma herança protegida.\n"
         "<li>Uma seta vermelho escura é utilizada para denotar uma herança privada.\n"
         "<li>Uma seta púrpura pontilhada é usada se uma classe está contida ou é utilizada"
         "por outra classe. A seta é marcada com a(s) variável(eis) "
         "através das quais a classe ou estrutura apontada está acessível. \n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "legenda";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "Teste";
   }
   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "Lista de Testes";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Propriedades";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Propriedades";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config::getBool("optimize-c")) {
         return "Estruturas de Dados";
      } else {
         return "Classes";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Pacote " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Lista de Pacotes";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Esta é a lista com os pacotes e suas respectivas descrições (se disponíveis):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Pacotes";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Valor:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Problema";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Lista de Problemas";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.6
   //////////////////////////////////////////////////////////////////////////

   /*! Used as ansicpg for RTF file
    *
    * The following table shows the correlation of Charset name, Charset Value and
    * <pre>
    * Codepage number:
    * Charset Name       Charset Value(hex)  Codepage number
    * ------------------------------------------------------
    * DEFAULT_CHARSET           1 (x01)
    * SYMBOL_CHARSET            2 (x02)
    * OEM_CHARSET             255 (xFF)
    * ANSI_CHARSET              0 (x00)            1252
    * RUSSIAN_CHARSET         204 (xCC)            1251
    * EE_CHARSET              238 (xEE)            1250
    * GREEK_CHARSET           161 (xA1)            1253
    * TURKISH_CHARSET         162 (xA2)            1254
    * BALTIC_CHARSET          186 (xBA)            1257
    * HEBREW_CHARSET          177 (xB1)            1255
    * ARABIC _CHARSET         178 (xB2)            1256
    * SHIFTJIS_CHARSET        128 (x80)             932
    * HANGEUL_CHARSET         129 (x81)             949
    * GB2313_CHARSET          134 (x86)             936
    * CHINESEBIG5_CHARSET     136 (x88)             950
    * </pre>
    *
    */
   virtual QByteArray trRTFansicp() {
      return "1252";
   }


   /*! Used as ansicpg for RTF fcharset
    *  \see trRTFansicp() for a table of possible values.
    */
   virtual QByteArray trRTFCharSet() {
      return "0";
   }

   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "Índice";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Classe" : "classe"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Arquivo" : "arquivo"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Namespace" : "namespace"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Grupo" : "grupo"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Página" : "página"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Membro" : "membro"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Globa" : "globa"));
      if (!singular) {
         result += "is";
      } else {
         result += "l";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Autor" : "autor"));
      if (!singular) {
         result += "es";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Referências";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Implementa " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implementation this member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Implementado por " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Conteúdo";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Lista de Descontinuados(as)";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Eventos";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Eventos";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Tipos do Pacote";
   }

   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Funções do Pacote";
   }

   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Funções Estáticas do Pacote";
   }

   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Atributos do Pacote";
   }

   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Atributos Estáticos do Pacote";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Todos";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Este é o diagrama das funções utilizadas por esta função:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Resultado da Busca";
   }
   /*! This string is put just before listing the search results. The
    *  text can be different depending on the number of documents found.
    *  Inside the text you can put the special marker $num to insert
    *  the number representing the actual number of search results.
    *  The @a numDocuments parameter can be either 0, 1 or 2, where the
    *  value 2 represents 2 or more matches. HTML markup is allowed inside
    *  the returned string.
    */
   virtual QByteArray trSearchResults(int numDocuments) {
      if (numDocuments == 0) {
         return "Nenhum documento foi encontrado.";
      } else if (numDocuments == 1) {
         return "Apenas <b>1</b> documento foi encontrado.";
      } else {
         return "<b>$num</b> documentos encontrados. "
                "Os melhores resultados vem primeiro.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Resultados:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return  "Código-Fonte de " + filename;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Hierarquia de Diretórios";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Documentação do Directório";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Diretórios";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Esta Hierarquia de Diretórios está parcialmente ordenada (ordem alfabética)";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = "Referência do diretório ";
      result += dirName;
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Diretório" : "diretório"));
      if (!singular) {
         result += "s";
      }
      return result;
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.1
   //////////////////////////////////////////////////////////////////////////

   /*! This text is added to the documentation when the \\overload command
    *  is used for a overloaded function.
    */
   virtual QByteArray trOverloadText() {
      return "Este é um método provido por conveniência. "
             "Ele difere do método acima apenas na lista de "
             "argumentos que devem ser utilizados.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Este é o diagrama das funções que utilizam esta função:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Documentação da enumeração";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Documentação de Função Membro/Subrotina";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Lista de Tipos de Dados";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Campos de Dados";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Aqui estão os tipos de dados com descrições resumidas:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Esta é a lista de todos os membros ";
      if (!extractAll) {
         result += "documentados ";
      }
      result += "dos tipos de dados com links para ";
      if (extractAll) {
         result += "a documentação dos tipos de dados para cada membro:";
      } else {
         result += "os tipos de dados a que pertencem:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Índice de Tipos de Dados";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Documentação dos Tipos de Dados";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Funções/Subrotinas";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Documentação da Função/Subrotina";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Tipos de Dados";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Lista de Módulos";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Esta é a lista de todos os módulos ";
      if (!extractAll) {
         result += "documentados ";
      }
      result += "com descrições resumidas:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = (QByteArray)"Referência ";

      if (isTemplate) {
         result += "da Template ";
      }

      switch (compType) {
         case ClassDef::Class:
            result += "do Modulo ";
            break;
         case ClassDef::Struct:
            result += "do Tipo ";
            break;
         case ClassDef::Union:
            result += "da União ";
            break;
         case ClassDef::Interface:
            result += "da Interface ";
            break;
         case ClassDef::Protocol:
            result += "do Protocolo ";
            break;
         case ClassDef::Category:
            result += "da Categoria ";
            break;
         case ClassDef::Exception:
            result += "da Exceção ";
            break;
         default:
            break;
      }
      result += clName;

      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = "Referência do Módulo ";
      result += namespaceName;
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Membros do Módulo";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Esta é a lista de todos os membros ";
      if (!extractAll) {
         result += "documentados ";
      }
      result += "dos módulos com links para ";
      if (extractAll) {
         result += "a documentação dos módulos para cada membro:";
      } else {
         result += "os módulos a que pertencem:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Índice dos Módulos";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Modulo" : "modulo"));
      if (!singular) {
         result += "s";
      }
      return result;
   }
   /*! This is put at the bottom of a module documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFilesFortran(ClassDef::CompoundType compType,
         bool single) {
      // here s is one of " Module", " Struct" or " Union"
      // single is true implies a single file
      QByteArray result = (QByteArray)"A documentação para ";
      switch (compType) {
         case ClassDef::Class:
            result += "este modulo ";
            break;
         case ClassDef::Struct:
            result += "este tipo ";
            break;
         case ClassDef::Union:
            result += "esta união ";
            break;
         case ClassDef::Interface:
            result += "esta interface ";
            break;
         case ClassDef::Protocol:
            result += "esto protocolo ";
            break;
         case ClassDef::Category:
            result += "esta categoria ";
            break;
         case ClassDef::Exception:
            result += "esta exceção ";
            break;
         default:
            break;
      }

      result += " foi gerada a partir do";
      if (single) {
         result += " seguinte arquivo:";
      } else {
         result += "s seguintes arquivos:";
      }

      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Tipo" : "tipo"));
      if (!singular) {
         result += "s";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Subprograma" : "subprograma"));
      if (!singular) {
         result += "s";
      }
      return result;
   }

   /*! C# Type Contraint list */
   virtual QByteArray trTypeConstraints() {
      return "Restrições do Tipo";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name
    */
   virtual QByteArray trDirRelation(const char *name) {
      return "Relação " + QByteArray(name);
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Carregando...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Namespace global";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Procurando...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Nenhuma entrada encontrada";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Arquivo em " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Inclui arquivo em " + name;
   }

   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Referências Bibliográficas";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      // Note: left it as is because "Direitos autorais" may not fit in the text.
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Grafo de dependências do diretório ") + name + ":";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.0
   //////////////////////////////////////////////////////////////////////////

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "nível de detalhes";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Parâmetros de template";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "e " + number + " mais...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "A documentação para esta enumeração foi gerada a partir";
      if (single) {
         result += "do seguinte arquivo:";
      } else {
         result += "dos seguintes arquivos:";
      }
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      QByteArray result = "Referência da enumeração ";
      return result + name;
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " herdados de " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Outros membros herdados";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      QByteArray opt = enable ? "ativar" : "desativar";
      return "clique para " + opt + " a sincronização do painel";
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "Provido pela categoria @1.";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "estende a classe @1.";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "Métodos de classe";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "Métodos de instância";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "Documentação do método";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      // I'm not sure how to accurately translate it
      return "Visão geral do design";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.4
   //////////////////////////////////////////////////////////////////////////

   /** old style UNO IDL services: implemented interfaces */
   virtual QByteArray trInterfaces() {
      return "Interfaces Exportadas";
   }

   /** old style UNO IDL services: inherited services */
   virtual QByteArray trServices() {
      return "Serviços Incluídos";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroups() {
      return "Grupos de Constantes";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroupReference(const char *namespaceName) {
      QByteArray result = "Referência do grupo de constantes ";
      result += namespaceName;
      return result;
   }

   /** UNO IDL service page title */
   virtual QByteArray trServiceReference(const char *sName) {
      QByteArray result = "Referência do serviço ";
      result += sName;
      return result;
   }

   /** UNO IDL singleton page title */
   virtual QByteArray trSingletonReference(const char *sName) {
      QByteArray result = "Referência do Singleton ";
      result += sName;
      return result;
   }

   /** UNO IDL service page */
   virtual QByteArray trServiceGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"A documentação para este serviço "
                          "foi gerada a partir ";
      if (single) {
         result += "do seguinte arquivo:";
      } else {
         result += "dos: seguintes arquivos:";
      }
      return result;
   }

   /** UNO IDL singleton page */
   virtual QByteArray trSingletonGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"A documentação para este singleton "
                          "foi gerada a partir ";
      if (single) {
         result += "do seguinte arquivo:";
      } else {
         result += "dos: seguintes arquivos:";
      }

      return result;
   }

   //////////////////////////////////////////////////////////////////////////
};
#endif

