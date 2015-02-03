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

#ifndef TRANSLATOR_PT_H
#define TRANSLATOR_PT_H


class TranslatorPortuguese : public Translator
{
 public:

   // --- Language control methods -------------------

   /*! Used for identification of the language. The identification
    * should not be translated. It should be replaced by the name
    * of the language in English using lower-case characters only
    * (e.g. "czech", "japanese", "russian", etc.). It should be equal to
    * the identification used in language.cpp.
    */
   QByteArray idLanguage() {
      return "portuguese";
   }

   /*! Used to get the LaTeX command(s) for the language support.
    *  This method should return string with commands that switch
    *  LaTeX to the desired language.  For example
    *  <pre>"\\usepackage[german]{babel}\n"
    *  </pre>
    *  or
    *  <pre>"\\usepackage{polski}\n"
    *  "\\usepackage[latin2]{inputenc}\n"
    *  "\\usepackage[T1]{fontenc}\n"
    *  </pre>
    *
    * The English LaTeX does not use such commands.  Because of this
    * the empty string is returned in this implementation.
    */
   virtual QByteArray latexLanguageSupportCommand() {
      return "Portuguese";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   QByteArray trRelatedFunctions() {
      return "Funções associadas";
   }

   /*! subscript for the related functions. */
   QByteArray trRelatedSubscript() {
      return "(Note que não são funções membro)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   QByteArray trDetailedDescription() {
      return "Descrição detalhada";
   }

   /*! header that is put before the list of typedefs. */
   QByteArray trMemberTypedefDocumentation() {
      return "Documentação das definições de tipo";
   }

   /*! header that is put before the list of enumerations. */
   QByteArray trMemberEnumerationDocumentation() {
      return "Documentação das enumerações";
   }

   /*! header that is put before the list of member functions. */
   QByteArray trMemberFunctionDocumentation() {
      return "Documentação dos métodos";
   }

   /*! header that is put before the list of member attributes. */
   QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Documentação dos campos e atributos";
      } else {
         return "Documentação dos dados membro";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   QByteArray trMore() {
      return "Mais...";
   }

   /*! put in the class documentation */
   QByteArray trListOfAllMembers() {
      return "Mostrar lista completa dos membros";
   }

   /*! used as the title of the "list of all members" page of a class */
   QByteArray trMemberList() {
      return "Lista dos membros";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   QByteArray trThisIsTheListOfAllMembers() {
      return "Lista completa de todos os membros de ";
   }

   /*! this is the remainder of the sentence after the class name */
   QByteArray trIncludingInheritedMembers() {
      return ", incluindo todos os membros herdados.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Gerado automaticamente por Doxygen";
      if (s) {
         result += (QByteArray)" para " + s;
      }
      result += " a partir do código fonte.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   QByteArray trEnumName() {
      return "enumeração";
   }

   /*! put after an enum value in the list of all members */
   QByteArray trEnumValue() {
      return "valor enumerado";
   }

   /*! put after an undocumented member in the list of all members */
   QByteArray trDefinedIn() {
      return "definido em";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   QByteArray trModules() {
      return "Módulos";
   }

   /*! This is put above each page as a link to the class hierarchy */
   QByteArray trClassHierarchy() {
      return "Hierarquia de classes";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Estruturas de dados";
      } else {
         return "Lista de componentes";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   QByteArray trFileList() {
      return "Lista de ficheiros";
   }

   /*! This is put above each page as a link to all members of compounds. */
   QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Campos de dados";
      } else {
         return "Componentes membro";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Globais";
      } else {
         return "Membros dos Ficheiros";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   QByteArray trRelatedPages() {
      return "Páginas relacionadas";
   }

   /*! This is put above each page as a link to all examples. */
   QByteArray trExamples() {
      return "Exemplos";
   }

   /*! This is put above each page as a link to the search engine. */
   QByteArray trSearch() {
      return "Localizar";
   }

   /*! This is an introduction to the class hierarchy. */
   QByteArray trClassHierarchyDescription() {
      return "Esta lista de heranças está organizada, dentro do possível, por ordem alfabética:";
   }

   /*! This is an introduction to the list with all files. */
   QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Lista de todos os ficheiros ";
      if (!extractAll) {
         result += "documentados ";
      }
      result += "com uma breve descrição:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   QByteArray trCompoundListDescription() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Lista das estruturas de dados com uma breve descrição:";
      } else {
         return "Lista de classes, estruturas, uniões e interfaces com uma breve descrição:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Lista de todas as";
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += " estruturas e campos de uniões";
      } else {
         result += " classes membro";
      }
      if (!extractAll) {
         result += " documentadas";
      }
      result += " com referência para ";
      if (!extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "a respectiva documentação:";
         } else {
            result += "a documentação de cada membro:";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "as estruturas/uniões a que pertencem:";
         } else {
            result += "as classes a que pertencem:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Lista de ";
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "todas as funções, variáveis, definições, enumerações e definições de tipo ";
         if (!extractAll) {
            result += "documentadas ";
         }
      } else {
         result += "todos os ficheiros membro ";
         if (!extractAll) {
            result += "documentados ";
         }
      }
      result += "com referência para ";
      if (extractAll) {
         result += "o ficheiro a que pertecem:";
      } else {
         result += "a respectiva documentação:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   QByteArray trExamplesDescription() {
      return "Lista de todos os exemplos:";
   }

   /*! This is an introduction to the page with the list of related pages */
   QByteArray trRelatedPagesDescription() {
      return "Lista de documentação relacionada:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   QByteArray trModulesDescription() {
      return "Lista de todos os módulos:";
   }

   // index titles (the project name is prepended for these)


   /*! This is used in HTML as the title of index.html. */
   QByteArray trDocumentation() {
      return "Documentação";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   QByteArray trModuleIndex() {
      return "Índice dos módulos";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   QByteArray trHierarchicalIndex() {
      return "Índice da hierarquia";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Índice das estruturas de dados";
      } else {
         return "Índice dos componentes";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   QByteArray trFileIndex() {
      return "Índice dos ficheiros";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   QByteArray trModuleDocumentation() {
      return "Documentação do módulo";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   QByteArray trClassDocumentation() {
      return "Documentação da classe";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   QByteArray trFileDocumentation() {
      return "Documentação do ficheiro";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   QByteArray trExampleDocumentation() {
      return "Documentação do exemplo";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   QByteArray trPageDocumentation() {
      return "Documentação da página";
   }

   /*! This is used in LaTeX as the title of the document */
   QByteArray trReferenceManual() {
      return "Manual de referência";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   QByteArray trDefines() {
      return "Macros";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   QByteArray trTypedefs() {
      return "Definições de tipos";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   QByteArray trEnumerations() {
      return "Enumerações";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   QByteArray trFunctions() {
      return "Funções";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   QByteArray trVariables() {
      return "Variáveis";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   QByteArray trEnumerationValues() {
      return "Valores de enumerações";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   QByteArray trDefineDocumentation() {
      return "Documentação das macros";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   QByteArray trTypedefDocumentation() {
      return "Documentação dos tipos";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   QByteArray trEnumerationTypeDocumentation() {
      return "Documentação dos valores da enumeração";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   QByteArray trFunctionDocumentation() {
      return "Documentação das funções";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   QByteArray trVariableDocumentation() {
      return "Documentação das variáveis";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Estruturas de Dados";
      } else {
         return "Componentes";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Gerado em " + date;
      if (projName) {
         result += (QByteArray)" para " + projName;
      }
      result += (QByteArray)" por";
      return result;
   }

   /*! this text is put before a class diagram */
   QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Diagrama de heranças da classe " + clName;
   }

   /*! this text is generated when the \\internal command is used. */
   QByteArray trForInternalUseOnly() {
      return "Apenas para uso interno.";
   }

   /*! this text is generated when the \\warning command is used. */
   QByteArray trWarning() {
      return "Aviso";
   }

   /*! this text is generated when the \\version command is used. */
   QByteArray trVersion() {
      return "Versão";
   }

   /*! this text is generated when the \\date command is used. */
   QByteArray trDate() {
      return "Data";
   }

   /*! this text is generated when the \\return command is used. */
   QByteArray trReturns() {
      return "Retorna";
   }

   /*! this text is generated when the \\sa command is used. */
   QByteArray trSeeAlso() {
      return "Veja também";
   }

   /*! this text is generated when the \\param command is used. */
   QByteArray trParameters() {
      return "Parâmetros";
   }

   /*! this text is generated when the \\exception command is used. */
   QByteArray trExceptions() {
      return "Excepções";
   }

   /*! this text is used in the title page of a LaTeX document. */
   QByteArray trGeneratedBy() {
      return "Gerado por";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Lista de namespaces";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Lista ";
      if (!extractAll) {
         result += "de toda a documentação ";
      }
      result += "dos namespaces com uma breve descrição:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Amigos";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Documentação das classes amigas e funções relacionadas";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result = "Referência ";
      switch (compType) {
         case ClassDef::Class:
            result += "à classe ";
            break;
         case ClassDef::Struct:
            result += "à estrutura ";
            break;
         case ClassDef::Union:
            result += "à união ";
            break;
         case ClassDef::Interface:
            result += "ao interface ";
            break;
         case ClassDef::Protocol:
            result += "ao protocolo ";
            break;
         case ClassDef::Category:
            result += "à categoria ";
            break;
         case ClassDef::Exception:
            result += "à excepção ";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += "Template ";
      }
      result += (QByteArray)clName;
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = "Referência ao ficheiro ";
      result += fileName;
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = "Referência ao namespace ";
      result += namespaceName;
      return result;
   }

   /* these are for the member sections of a class, struct or union */
   virtual QByteArray trPublicMembers() {
      return "Membros públicos";
   }
   virtual QByteArray trPublicSlots() {
      return "Slots públicos";
   }
   virtual QByteArray trSignals() {
      return "Sinais";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Membros públicos estáticos";
   }
   virtual QByteArray trProtectedMembers() {
      return "Membros protegidos";
   }
   virtual QByteArray trProtectedSlots() {
      return "Slots protegidos";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Membros protegidos estáticos";
   }
   virtual QByteArray trPrivateMembers() {
      return "Membros privados";
   }
   virtual QByteArray trPrivateSlots() {
      return "Slots privados";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Membros privados estáticos";
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
      return "Derivada de " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Herdado por " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Reimplementado de " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Reimplementado em " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Membros do namespace";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Lista ";
      if (extractAll) {
         result += "de todos os ";
      } else {
         result += "de toda a documentação dos ";
      }
      result += "membros do namespace com referência para ";
      if (extractAll) {
         result += "a documentação de cada membro:";
      } else {
         result += "o namespace correspondente:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
      *  index of all namespaces.
      */
   virtual QByteArray trNamespaceIndex() {
      return "Índice dos namespaces";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Documentação dos namespaces";
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
      QByteArray result = (QByteArray)"A documentação para ";
      switch (compType) {
         case ClassDef::Class:
            result += "esta classe";
            break;
         case ClassDef::Struct:
            result += "esta estrutura";
            break;
         case ClassDef::Union:
            result += "esta união";
            break;
         case ClassDef::Interface:
            result += "este interface";
            break;
         case ClassDef::Protocol:
            result += "este protocolo";
            break;
         case ClassDef::Category:
            result += "esta categoria";
            break;
         case ClassDef::Exception:
            result += "esta excepção";
            break;
         default:
            break;
      }
      result += " foi gerada a partir ";
      if (single) {
         result += " do seguinte ficheiro:";
      } else {
         result += "dos seguintes ficheiros:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Valores retornados";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Página principal";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "p. ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Definido na linha @0 do ficheiro @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Definido no ficheiro @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Desaprovado";
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
      return (QByteArray)"Diagrama de dependências de inclusão para " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Documentação dos Construtores & Destrutor";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Ir para o código fonte deste ficheiro.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Ir para a documentação deste ficheiro.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Precondição";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Poscondição";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Invariante";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Valor inicial:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "código";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Representação gráfica da hiearquia da classe";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Ir para a representação gráfica da hierarquia da classe";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Ir para a representação textual da hierarquia da classe";
   }
   virtual QByteArray trPageIndex() {
      return "Índice da página";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Nota";
   }
   virtual QByteArray trPublicTypes() {
      return "Tipos Públicos";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Campos de Dados";
      } else {
         return "Atributos Públicos";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Atributos Públicos Estáticos";
   }
   virtual QByteArray trProtectedTypes() {
      return "Tipos Protegidos";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Atributos Protegidos";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Atributos Protegidos Estáticos";
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
      return "Tarefa";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Lista de tarefas";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Referenciado por";
   }
   virtual QByteArray trRemarks() {
      return "Observações";
   }
   virtual QByteArray trAttention() {
      return "Atenção";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Este grafo mostra quais são os ficheiros que incluem directamente ou indirectamente este ficheiro:";
   }
   virtual QByteArray trSince() {
      return "Desde";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Legenda do grafo";
   }
   /*! page explaining how the dot graph's should be interpreted */
   virtual QByteArray trLegendDocs() {
      return
         "Esta página explica como interpretar os grafos gerados pelo doxygen.<p>\n"
         "Considere o seguinte exemplo:\n"
         "\\code\n"
         "/*! Esta classe vai estar escondida devido à truncação */\n"
         "class Invisible { };\n\n"
         "/*! Esta classe tem a relação de herança escondida */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Classe não documentada por comentários doxygen */\n"
         "class Undocumented { };\n\n"
         "/*! Classe derivada usando derivação pública */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! A template class */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Classe derivada usando derivação protegida */\n"
         "class ProtectedBase { };\n\n"
         "/*! Classe derivada usando derivação privada */\n"
         "class PrivateBase { };\n\n"
         "/*! Classe usada pela classe Inherited */\n"
         "class Used { };\n\n"
         "/*! Super classe que deriva de várias classes */\n"
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
         "<p><center><img src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
         "<p>\n"
         "As caixas no grafo anterior têm as seguintes interpretações:\n"
         "<ul>\n"
         "<li>Uma caixa inteiramente preta representa a estrutura ou a classe para "
         "a qual o grafo foi gerado.\n"
         "<li>Uma caixa com borda preta representa uma estrutura ou classe documentada.\n"
         "<li>Uma caixa com borda cinzenta representa uma estrutura ou classe não documentada.\n"
         "<li>Uma caixa com borda vermelha representa uma estrutura ou classe documentada onde "
         "nem todas as relações de herança/encapsulamento são exibidas. Um grafo é truncado "
         "quando não cabe na sua área predefinida.\n"
         "</ul>\n"
         "As setas têm a seguinte interpretação:\n"
         "<ul>\n"
         "<li>Uma seta azul escura representa uma relação de herança pública entre duas classes.\n"
         "<li>Uma seta verde escura representa uma relação de herança protegida.\n"
         "<li>Uma seta vermelha escura representa uma relação de herança privada.\n"
         "<li>Uma seta rocha em tracejado representa uma relação de encapsulamento ou uso por "
         "parte de outra classe. A legenda da seta contém o nome da variável ou variáveis da "
         "relação. A seta aponta da classe que estabelece a relação para a classe ou estrutura que "
         "é acessível.\n"
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
      return "Lista de teste";
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
      return "Documentação das propriedades";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "Estruturas de dados";
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
      return "Lista de pacotes";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Lista de pacotes com uma breve descrição (se disponível):";
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
      return "Bug";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Lista de Bugs";
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
      QByteArray result((first_capital ? "Ficheiro" : "ficheiro"));
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
      result += singular ? "l" : "ais";
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
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Implementado em " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Índice";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Lista de Deprecados";
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
      return "Documentação dos eventos";
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
      return "Tudo";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Grafo de chamadas desta função:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3 - Based on the Brazilian Portuguese Translation
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
         result += " seguinte ficheiro:";
      } else {
         result += "s seguintes ficheiros:";
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

  

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Ficheiro em " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Inclui ficheiro em " + name;
   }

   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Referências Bibliográficas";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      // Note: I will left it as is because "Direitos de autor" may not fit
      // in the text.
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Grafo de dependências do directório ") + name + ":";
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
         result += "do seguinte ficheiro:";
      } else {
         result += "dos seguintes ficheiros:";
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
         result += "do seguinte ficheiro:";
      } else {
         result += "dos: seguintes ficheiros:";
      }
      return result;
   }

   /** UNO IDL singleton page */
   virtual QByteArray trSingletonGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"A documentação para este singleton "
                          "foi gerada a partir ";
      if (single) {
         result += "do seguinte ficheiro:";
      } else {
         result += "dos: seguintes ficheiros:";
      }

      return result;
   }

   //////////////////////////////////////////////////////////////////////////
};

#endif
