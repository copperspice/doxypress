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

#ifndef TRANSLATOR_ES_H
#define TRANSLATOR_ES_H

class TranslatorSpanish : public Translator
{
 public:

   // --- Language control methods -------------------

   /*! Used for identification of the language. The identification
    * should not be translated. It should be replaced by the name
    * of the language in English using lower-case characters only
    * (e.g. "czech", "japanese", "russian", etc.). It should be equal to
    * the identification used in language.cpp.
    */
   virtual QByteArray idLanguage() {
      return "spanish";
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
      return "\\usepackage[spanish]{babel}";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Funciones relacionadas";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Observar que estas no son funciones miembro.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Descripción detallada";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Documentación de los 'Typedef' miembros de la clase";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Documentación de las enumeraciones miembro de la clase";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Documentación de las funciones miembro";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Documentación de los campos";
      } else {
         return "Documentación de los datos miembro";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Más...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Lista de todos los miembros";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Lista de los miembros";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Lista completa de los miembros de ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", incluyendo todos los heredados:";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Generado automáticamente por Doxygen";
      if (s) {
         result += (QByteArray)" para " + s;
      }
      result += " del código fuente.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "nombre de la enumeración";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "valor enumerado";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "definido en";
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
      return "Jerarquía de la clase";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config::getBool("optimize-c")) {
         return "Estructura de datos";
      } else {
         return "Lista de clases";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Lista de archivos";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config::getBool("optimize-c")) {
         return "Campos de datos";
      } else {
         return "Miembros de las clases";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config::getBool("optimize-c")) {
         return "Globales";
      } else {
         return "Miembros de los ficheros";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Páginas relacionadas";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Ejemplos";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Buscar";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Esta lista de herencias esta ordenada "
             "aproximadamente por orden alfabético:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Lista de todos los archivos ";
      if (!extractAll) {
         result += "documentados y ";
      }
      result += "con descripciones breves:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {
      if (Config::getBool("optimize-c")) {
         return "Lista de estructuras con una breve descripción:";
      } else {
         return "Lista de las clases, estructuras, "
                "uniones e interfaces con una breve descripción:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Lista de todos los ";
      if (Config::getBool("optimize-c")) {
         result += "campos de estructuras y uniones";
      } else {
         result += "campos de clases";
      }
      if (!extractAll) {
         result += " documentados";
      }
      result += " con enlaces a ";
      if (!extractAll) {
         if (Config::getBool("optimize-c")) {
            result += "la documentación de la estructura/unión para cada campo:";
         } else {
            result += "la documentación de la clase para cada miembro:";
         }
      } else {
         if (Config::getBool("optimize-c")) {
            result += "las estructuras/uniones a que pertenecen:";
         } else {
            result += "las classes a que pertenecen:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Lista de ";
      if (Config::getBool("optimize-c")) {
         result += "todas las funciones, variables, 'defines', enumeraciones y 'typedefs'";
      } else {
         result += "todos los mienbros de los ficheros";
      }
      if (!extractAll) {
         result += " documentados";
      }
      result += " con enlaces ";
      if (extractAll) {
         result += "a los ficheros a los que corresponden:";
      } else {
         result += "a la documentación:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Lista de todos los ejemplos:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Lista de toda la documentación relacionada:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Lista de todos los módulos:";
   }

   // index titles (the project name is prepended for these)

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Documentación";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Indice de módulos";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Indice jerárquico";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config::getBool("optimize-c")) {
         return "Índice de estructura de datos";
      } else {
         return "Índice de clases";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Indice de archivos";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Documentación de módulos";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Documentación de las estructuras de datos";
      } else {
         return "Documentación de las clases";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Documentación de archivos";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Documentación de ejemplos";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Documentación de páginas";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Manual de referencia";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "'defines'";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "'typedefs'";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Enumeraciones";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Funciones";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Variables";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Valores de enumeraciones";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Documentación de los 'defines'";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Documentación de los 'typedefs'";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Documentación de las enumeraciones";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Documentación de las funciones";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Documentación de las variables";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config::getBool("optimize-c")) {
         return "Estructuras de datos";
      } else {
         return "Clases";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Generado el " + date;
      if (projName) {
         result += (QByteArray)" para " + projName;
      }
      result += (QByteArray)" por";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Diagrama de herencias de " + clName;
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Sólo para uso interno.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Atención";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Versión";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Fecha";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Devuelve";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Ver también";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Parámetros";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Excepciones";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Generado por";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Lista de 'namespaces'";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Lista de ";
      if (!extractAll) {
         result += "toda la documentación de ";
      }
      result += "los 'namespaces', con una breve descripción:";
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
      return "Documentación de las funciones relacionadas y clases amigas";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result = "Referencia de";
      if (isTemplate) {
         result += " la plantilla de";
      }
      switch (compType) {
         case ClassDef::Class:
            result += " la Clase ";
            break;
         case ClassDef::Struct:
            result += " la Estructura ";
            break;
         case ClassDef::Union:
            result += " la Unión ";
            break;
         case ClassDef::Interface:
            result += " la Interfaz ";
            break;
         case ClassDef::Protocol:
            result += "l Protocolo ";
            break;
         case ClassDef::Category:
            result += " la Categoria ";
            break;
         case ClassDef::Exception:
            result += " la Excepción ";
            break;
         default:
            break;
      }
      result += (QByteArray)clName;
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = "Referencia del Archivo ";
      result += fileName;
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = "Referencia del Namespace ";
      result += namespaceName;
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Métodos públicos";
   }

   virtual QByteArray trPublicSlots() {
      return "Slots públicos";
   }

   virtual QByteArray trSignals() {
      return "Señales";
   }

   virtual QByteArray trStaticPublicMembers() {
      return "Métodos públicos estáticos";
   }

   virtual QByteArray trProtectedMembers() {
      return "Métodos protegidos";
   }

   virtual QByteArray trProtectedSlots() {
      return "Slots protegidos";
   }

   virtual QByteArray trStaticProtectedMembers() {
      return "Métodos protegidos estáticos";
   }

   virtual QByteArray trPrivateMembers() {
      return "Métodos privados";
   }

   virtual QByteArray trPrivateSlots() {
      return "Slots privados";
   }

   virtual QByteArray trStaticPrivateMembers() {
      return "Métodos privados estáticos";
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
               result += " y ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Herencias " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Heredado por " + trWriteList(numEntries) + ".";
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
      return "Reimplementado en " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Miembros del Namespace ";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Lista de ";
      if (!extractAll) {
         result += "toda la documentación de ";
      }
      result += "los miembros del namespace con enlace a ";
      if (extractAll) {
         result += "los namespace de cada miembro:";
      } else {
         result += "la documentación de los namespaces pertenecientes a:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Indice de namespaces";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Documentación de namespaces";
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
      QByteArray result = (QByteArray)"La documentación para est";
      switch (compType) {
         case ClassDef::Class:
            result += "a clase";
            break;
         case ClassDef::Struct:
            result += "a estructura";
            break;
         case ClassDef::Union:
            result += "a unión";
            break;
         case ClassDef::Interface:
            result += "e interfaz";
            break;
         case ClassDef::Protocol:
            result += "e protocolo";
            break;
         case ClassDef::Category:
            result += "a categoría";
            break;
         case ClassDef::Exception:
            result += "a excepción";
            break;
         default:
            break;
      }
      result += " fue generada a partir de";
      if (single) {
         result += "l siguiente fichero:";
      } else {
         result += " los siguientes ficheros:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Valores devueltos";
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
      return "p.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Definición en la línea @0 del archivo @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Definición en el archivo @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Obsoleto";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Diagrama de colaboración para " + clName + ":";
   }

   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Dependencia gráfica adjunta para " + fName + ":";
   }

   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Documentación del constructor y destructor";
   }

   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Ir al código fuente de este archivo.";
   }

   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Ir a la documentación de este archivo.";
   }

   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Precondición";
   }

   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Postcondición";
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
      return "Representación gráfica de la clase";
   }

   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Ir a la representación gráfica de la jerarquía de la clase";
   }

   virtual QByteArray trGotoTextualHierarchy() {
      return "Ir a la jerarquía textual de la clase";
   }

   virtual QByteArray trPageIndex() {
      return "Página indice";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Nota";
   }

   virtual QByteArray trPublicTypes() {
      return "Tipos públicos";
   }

   virtual QByteArray trPublicAttribs() {
      if (Config::getBool("optimize-c")) {
         return "Campos de datos";
      } else {
         return "Atributos públicos";
      }
   }

   virtual QByteArray trStaticPublicAttribs() {
      return "Atributos públicos estáticos";
   }

   virtual QByteArray trProtectedTypes() {
      return "Tipos protegidos";
   }

   virtual QByteArray trProtectedAttribs() {
      return "Atributos protegidos";
   }

   virtual QByteArray trStaticProtectedAttribs() {
      return "Atributos protegidos estáticos";
   }

   virtual QByteArray trPrivateTypes() {
      return "Tipos privados";
   }

   virtual QByteArray trPrivateAttribs() {
      return "Atributos privados";
   }

   virtual QByteArray trStaticPrivateAttribs() {
      return "Atributos privados estáticos";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "Tareas pendientes";
   }

   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Lista de tareas pendientes";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Referenciado por";
   }

   virtual QByteArray trRemarks() {
      return "Comentarios";
   }

   virtual QByteArray trAttention() {
      return "Atención";
   }

   virtual QByteArray trInclByDepGraph() {
      return "Gráfico de los archivos que directa o "
             "indirectamente incluyen a este archivo:";
   }

   virtual QByteArray trSince() {
      return "Desde";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   QByteArray trLegendTitle() {
      return "Colores y flechas del Gráfico";
   }

   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Esta página explica como interpretar los gráficos que son generados "
         "por doxygen.<p>\n"
         "Considere el siguiente ejemplo:\n"
         "\\code\n"
         "/*! Clase invisible por truncamiento */\n"
         "class Invisible { };\n\n"
         "/*! Clase truncada, relación de herencia escondida */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Clase no documentada con comentarios de doxygen */\n"
         "class Undocumented { };\n\n"
         "/*! Clase que es heredera usando herencia publica */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! Clase plantilla */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Clase que es heredera usando herencia protegida  */\n"
         "class ProtectedBase { };\n\n"
         "/*! Clase que es heredera usando herencia privada  */\n"
         "class PrivateBase { };\n\n"
         "/*! Clase que es usada por la clase heredada */\n"
         "class Used { };\n\n"
         "/*! Super-Clase que hereda de varias otras clases */\n"
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
         "Dará como resultado el siguiente gráfico:"
         "<p><center><img alt=\"\" src=\"graph_legend." + Config::getEnum("dot-image-format").toUtf8() + "\"></center></p>\n"
         "<p>\n"
         "Las cajas en el gráfico arriba tienen el siguiente significado:\n"
         "</p>\n"
         "<ul>\n"
         "<li>Una caja llena gris representa la estructura o clase para la cuál"
         "se generó el gráfico.\n"
         "<li>Una caja con borde negro señala una estructura o clase documentada.\n"
         "<li>Una caja con borde griz señala una estructura o clase no documentada.\n"
         "<li>una caja con borde rojo señala una estructura o clase documentada"
         " de la cuál no toda las relaciones de jerarquía/contenido son "
         "mostradas. El gráfico sera truncado si este no calza dentro de los "
         "límites especificados."
         "</ul>\n"
         "<p>\n"
         "Las flechas tienen el siguiente significado:\n"
         "</p>\n"
         "<ul>\n"
         "<li>Una flecha azul oscuro es usada para visualizar una relación herencia publica entre dos clases.\n"
         "<li>Una flecha verde oscuro es usada para herencia protegida.\n"
         "<li>Una flecha rojo oscuro es usada para herencia privada.\n"
         "<li>Una flecha segmentada púrpura se usa si la clase es contenida o "
         "usada por otra clase. La flecha está etiquetada por la variable "
         "con que se accede a la clase o estructura apuntada. \n"
         "<li>Una flecha segmentada amarilla indica la relación entre una instancia template y la clase template de la que se ha instanciado."
         " La flecha se etiqueta con los parámetros con que se llama al template.\n"
         "</ul>\n";
   }

   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "significado de colores y flechas";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "Prueba";
   }

   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "Lista de pruebas";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Propiedades";
   }

   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Documentación de propiedades";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config::getBool("optimize-c")) {
         return "Estructuras de Datos";
      } else {
         return "Clases";
      }
   }

   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Paquetes " + name;
   }

   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Lista de Paquetes ";
   }

   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Aquí van los paquetes con una breve descripción (si etá disponible):";
   }

   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Paquetes";
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
      return "Lista de bugs";
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
      QByteArray result((first_capital ? "Clase" : "clase"));
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
      QByteArray result((first_capital ? "Archivo" : "archivo"));
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
      QByteArray result((first_capital ? "Miembro" : "miembro"));
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
      QByteArray result((first_capital ? "Global" : "global"));
      if (!singular) {
         result += "es";
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
      return "Hace referencia a";
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
      return "Implementado en " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Tabla de contenidos";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Lista de obsoletos";
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
      return "Documentación de los eventos";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Tipos del 'package'";
   }

   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Funciones del 'package'";
   }

   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Funciones estáticas del 'package'";
   }

   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Atributos del 'package'";
   }

   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Atributos Estáticos del 'package'";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Todo";
   }

   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Gráfico de llamadas para esta función:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Resultados de la Búsqueda";
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
         return "Disculpe, no se encontraron documentos que coincidan con su búsqueda.";
      } else if (numDocuments == 1) {
         return "Se encontró <b>1</b> documento que coincide con su búsqueda.";
      } else {
         return "Se encontraron <b>$num</b> documentos que coinciden con su búsqueda. "
                "Se muestran los mejores resultados primero.";
      }
   }

   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Coincidencias:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return "Fichero Fuente " + filename;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Jerarquía de directorios";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Documentación de directorios";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Directorios";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "La jeraquía de este directorio está ordenada"
             " alfabéticamente, de manera aproximada:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = "Referencia del directorio ";
      result += dirName;
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Directorio" : "directorio"));
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
      return "Esta es una función miembro sobrecargada que se "
             "suministra por conveniencia. Difiere de la anterior "
             "función solamente en los argumentos que acepta.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Gráfico de llamadas a esta función:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Documentación de los valores de la enumeración";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////
   // De parte de Bartomeu:
   //    No conozco el Fortran, salvo un par de ejercicios en la universidad
   // hace muchos años. Por lo tanto, las traducciones son del inglés
   // al español, no de un usuario de Fortran que puede que haya cosas que no
   // traduzca o traduzca de otra forma. Que los usuarios de Fortran disculpen
   // y espero se animen a mejorar mi traducción.

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Documetación de miembros Function/Subroutine";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Lista de tipos de datos";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Etiquetas de datos";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Aquí están los tipos de datos con una breve descripción:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Aquí está una lista de todos ";
      result += "los miembros de los tipos de datos ";
      if (!extractAll) {
         result += "documentados ";
      }
      result += "con enlaces a ";
      if (!extractAll) {
         result += "la documentación de la estructura de datos para cada miembro";
      } else {
         result += "los tipos de dato a que pertenece:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Índice de tipos de datos";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Documentación de tipos de datos";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Funciones/Subprogramas";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Documentación de Funciones/Subprogramas";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Tipos de datos";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Lista de módulos";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Lista de todos los módulos ";
      if (!extractAll) {
         result += "documentados ";
      }
      result += "con una breve descripción:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = "Referencia de";
      if (isTemplate) {
         result += " la plantilla de";
      }
      switch (compType) {
         case ClassDef::Class:
            result += "l módulo";
            break;
         case ClassDef::Struct:
            result += "l tipo";
            break;
         case ClassDef::Union:
            result += " la unión";
            break;
         case ClassDef::Interface:
            result += " la interfaz";
            break;
         case ClassDef::Protocol:
            result += "l protocolo";
            break;
         case ClassDef::Category:
            result += " la categoría";
            break;
         case ClassDef::Exception:
            result += " la excepción";
            break;
         default:
            break;
      }
      result += (QByteArray)clName;
      return result;
   }

   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = "Referencia módulo ";
      result += namespaceName;
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Miembros módulo";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Lista de todos los miembros del módulo ";
      if (!extractAll) {
         result += "documentados ";
      }
      result += "con enlaces ";
      if (extractAll) {
         result += "a la documentación del módulo para cada uno:";
      } else {
         result += "al módulo al que pertenecen:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Índice de módulos";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Módulo" : "módulo"));
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
      QByteArray result = (QByteArray)"La documentación para est";
      switch (compType) {
         case ClassDef::Class:
            result += "e módulo";
            break;
         case ClassDef::Struct:
            result += "e tipo";
            break;
         case ClassDef::Union:
            result += "a unión";
            break;
         case ClassDef::Interface:
            result += "e interfaz";
            break;
         case ClassDef::Protocol:
            result += "e protocolo";
            break;
         case ClassDef::Category:
            result += "a categoría";
            break;
         case ClassDef::Exception:
            result += "a excepción";
            break;
         default:
            break;
      }
      result += " fue generada de";
      if (single) {
         result += "l siguiente fichero:";
      } else {
         result += " los siguientes ficheros:";
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

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Restriciones de tipo";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return QByteArray(name) + " relación";
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Cargando...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Namespace global";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Buscando...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Nada coincide";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Fichero en " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Incluye ficheros en " + name;
   }

   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Referencias bibliográficas";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Gráfico de dependencias de directorios para ") + name + ":";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.0
   //////////////////////////////////////////////////////////////////////////

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "detalle nivel";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Parámetros del template";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "y " + number + " más...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "La documentación para este enum ha sido generada a partir de";
      if (single) {
         result += "l siguiente fichero:";
      } else {
         result += " los siguientes ficheros:";
      }
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return QByteArray("Referencia del enum ") + QByteArray(name);
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " heredados desde " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Otros miembros heredados";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      QByteArray opt = enable ? "habilitar" : "deshabilitar";
      return "click en " + opt + " sincronización";
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "Proporcionado por categoría @1.";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "Extiende la clase @1.";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "Métodos de la clase";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "Métodos de instancia";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "Método de documentación";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      return "Diseño información general";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.4
   //////////////////////////////////////////////////////////////////////////

   /** old style UNO IDL services: implemented interfaces */
   virtual QByteArray trInterfaces() {
      return "Interface exportada";
   }

   /** old style UNO IDL services: inherited services */
   virtual QByteArray trServices() {
      return "Servicios incluidos";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroups() {
      return "Grupos constantes";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroupReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " referencia de grupos constantes";
      return result;
   }
   /** UNO IDL service page title */
   virtual QByteArray trServiceReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " Referencia servicio";
      return result;
   }
   /** UNO IDL singleton page title */
   virtual QByteArray trSingletonReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " referencia Singleton";
      return result;
   }
   /** UNO IDL service page */
   virtual QByteArray trServiceGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"La documentacion para este servicio "
                          "se ha generado desde ";
      if (single) {
         result += "el siguiente fichero:";
      } else {
         result += "los siguientes ficheros:";
      }
      return result;
   }
   /** UNO IDL singleton page */
   virtual QByteArray trSingletonGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"La documentación para este singleton "
                          "se ha generado desde ";
      if (single) {
         result += "el siguiente fichero:";
      } else {
         result += "los siguientes ficheros:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////

};

#endif
