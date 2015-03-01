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

#ifndef TRANSLATOR_SC_H
#define TRANSLATOR_SC_H

class TranslatorSerbianCyrillic : public TranslatorAdapter_1_6_0
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
      return "serbian-cyrillic";
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
      return "";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Повезане функције";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Напомињемо да ово нису функције чланице.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Опширније";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Документација дефиниције типа";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Документација члана набрајања";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Документација функције чланице";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Документација поља";
      } else {
         return "Документација атрибута";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Још...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Списак свих чланова";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Списак чланова";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Ово је списак свих чланова од ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", укључујући све наслеђене чланове.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Аутоматски направљено помоћу Doxygen-а";
      if (s) {
         result += (QByteArray)" за " + s;
      }
      result += " из изворног кода.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "назив набрајања";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "вредност набрајања";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "дефинисано у";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Модули";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Хијерархија класа";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config::getBool("optimize-c")) {
         return "Структуре";
      } else {
         return "Списак класа";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Списак датотека";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config::getBool("optimize-c")) {
         return "Поља";
      } else {
         return "Чланови класе";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config::getBool("optimize-c")) {
         return "Глобално";
      } else {
         return "Чланови датотеке";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Повезане странице";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Примери";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Тражи";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Овај списак наслеђивања је уређен "
             "скоро по абецеди:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Овде је списак свих ";
      if (!extractAll) {
         result += "документованих ";
      }
      result += "датотека са кратким описима:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config::getBool("optimize-c")) {
         return "Овде су структуре са кратким описима:";
      } else {
         return "Овде су класе, структуре, "
                "уније и интерфејси са кратким описима:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Овде је списак свих ";
      if (!extractAll) {
         result += "документованих ";
      }
      if (Config::getBool("optimize-c")) {
         result += "поља структура и унија";
      } else {
         result += "чланова класа";
      }
      result += " са везама ка ";
      if (!extractAll) {
         if (Config::getBool("optimize-c")) {
            result += "документацији структуре/уније за свако поље:";
         } else {
            result += "документацији класе за сваки члан:";
         }
      } else {
         if (Config::getBool("optimize-c")) {
            result += "структури/унији којој припадају:";
         } else {
            result += "класи којој припадају:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Овде је списак свих ";
      if (!extractAll) {
         result += "документованих ";
      }

      if (Config::getBool("optimize-c")) {
         result += "фрункција, променљивих, макро замена, набрајања, и дефиниција типова";
      } else {
         result += "чланова датотеке";
      }
      result += " са везама ка ";
      if (extractAll) {
         result += "датотекама којима припадају:";
      } else {
         result += "документацији:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Овде је списак свих примера:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Овде је списак свих повезаних страница документације:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Овде је списак свих модула:";
   }

   // index titles (the project name is prepended for these)

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Документација";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Индекс модула";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Хијерархијски индекс";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config::getBool("optimize-c")) {
         return "Индекс структура";
      } else {
         return "Индекс класа";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Индекс датотека";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Документација модула";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Документација структуре";
      } else {
         return "Документација класе";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Документација датотеке";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Документација примера";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Документација странице";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Приручник";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Дефиниције";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Дефиниције типова";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Набрајања";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Функције";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Променљиве";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Вредности набрајања";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Документација дефиниције";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Документација дефиниције типа";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Документација набрајања";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Документација функције";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Документација променљиве";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config::getBool("optimize-c")) {
         return "Структуре";
      } else {
         return "Класе";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Направљено " + date;
      if (projName) {
         result += (QByteArray)" за " + projName;
      }
      result += (QByteArray)" помоћу";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Дијаграм наслеђивања за " + clName + ":";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Само за унутрашњу употребу.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Упозорење";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Верзија";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Датум";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Враћа";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Види";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Параметри";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Изизеци";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Направљено помоћу";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Списак простора имена";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Овде је списак свих ";
      if (!extractAll) {
         result += "документованих ";
      }
      result += "простора имена са кратким описима:";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Пријатељи";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Документација за пријатеље и повезане функције";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      if (isTemplate) {
         result += " Шаблон";
         switch (compType) {
            case ClassDef::Class:
               result += "ска класа";
               break;
            case ClassDef::Struct:
               result += "ска структура";
               break;
            case ClassDef::Union:
               result += "ска унија";
               break;
            case ClassDef::Interface:
               result += "ски интерфејс";
               break;
            case ClassDef::Protocol:
               result += "ски протокол";
               break;
            case ClassDef::Category:
               result += "ска категорија";
               break;
            case ClassDef::Exception:
               result += "ски изузетак";
               break;
            default:
               break;
         }
      } else {
         result += " Референца";
         switch (compType) {
            case ClassDef::Class:
               result += " класе";
               break;
            case ClassDef::Struct:
               result += " структуре";
               break;
            case ClassDef::Union:
               result += " уније";
               break;
            case ClassDef::Interface:
               result += " интерфејса";
               break;
            case ClassDef::Protocol:
               result += " протокола";
               break;
            case ClassDef::Category:
               result += " категорије";
               break;
            case ClassDef::Exception:
               result += " изузетка";
               break;
            default:
               break;
         }
      }
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = fileName;
      result += " Референца датотеке";
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Референца простора имена";
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Јавне функције чланице";
   }
   virtual QByteArray trPublicSlots() {
      return "Јавни слотови";
   }
   virtual QByteArray trSignals() {
      return "Сигнали";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Статичке јавне функције чланице";
   }
   virtual QByteArray trProtectedMembers() {
      return "Заштићене функције чланице";
   }
   virtual QByteArray trProtectedSlots() {
      return "Заштићени слотови";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Статичке заштићене функције чланице";
   }
   virtual QByteArray trPrivateMembers() {
      return "Приватне функције чланице";
   }
   virtual QByteArray trPrivateSlots() {
      return "Приватни слотови";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Статичке приватне функције чланице";
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
               result += " и ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Наслеђује " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Наслеђују " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Поново имплементирано од " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Поново имплементирано у " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Чланови простора имена";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Овде је списак свих ";
      if (!extractAll) {
         result += "документованих ";
      }
      result += "чланова простора имена са везама ка ";
      if (extractAll) {
         result += "документацији простора имена за сваки члан:";
      } else {
         result += "просторима имена којима припадају:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Индекс простора имена";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Документација простора имена";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Простори имена";
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
      QByteArray result = (QByteArray)"Документација за ";
      switch (compType) {
         case ClassDef::Class:
            result += "ову класу";
            break;
         case ClassDef::Struct:
            result += "ову структуру";
            break;
         case ClassDef::Union:
            result += "ову унију";
            break;
         case ClassDef::Interface:
            result += "овај интерфејс";
            break;
         case ClassDef::Protocol:
            result += "овај протокол";
            break;
         case ClassDef::Category:
            result += "ову категорију";
            break;
         case ClassDef::Exception:
            result += "овај изузетак";
            break;
         default:
            break;
      }
      result += " је произведена из";
      if (single) {
         result += "следеће датотеке:";
      } else {
         result += "следећих датотека:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Враћене вредности";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Главна страница";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "стр.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Дефиниција у линији @0 датотеке @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Дефиниција у датотеци @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Застарело";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Дијаграм сарадње за " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Дијаграм зависности укључивања за " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Документација конструктора и деструктора";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Иди на изворни код овог фајла.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Иди на документацију овог фајла.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Услов пре";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Услов после";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Инваријанта";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Почетна вредност:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "код";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Графичка хијерархија класа";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Иди на графичку хијерархију класа";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Иди на текстуалну хијерархију класа";
   }
   virtual QByteArray trPageIndex() {
      return "Индекс страна";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Напомена";
   }
   virtual QByteArray trPublicTypes() {
      return "Јавни типови";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config::getBool("optimize-c")) {
         return "Поља";
      } else {
         return "Јавни атрибути";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Статички јавни атрибути";
   }
   virtual QByteArray trProtectedTypes() {
      return "Заштићени типови";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Заштићени атрибути";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Статички заштићени атрибути";
   }
   virtual QByteArray trPrivateTypes() {
      return "Приватни типови";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Приватни атрибути";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Статички приватни атрибути";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "Урадити";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Подсетник шта још урадити";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Референцирано од";
   }
   virtual QByteArray trRemarks() {
      return "Напомене";
   }
   virtual QByteArray trAttention() {
      return "Пажња";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Овај граф показује које датотеке директно или "
             "или индиректно укључују овај фајл:";
   }
   virtual QByteArray trSince() {
      return "Од";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Легенда графова";
   }
   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Ова страница објашњава како тумачити графове који су направљени "
         "doxygen-ом.<p>\n"
         "Размотримо следећи пример:\n"
         "\\code\n"
         "/*! Невидљива класа због одсецања */\n"
         "class Invisible { };\n\n"
         "/*! Одсечена класа, веза наслеђивања је скривена */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Класа која није документована doxygen коментарима */\n"
         "class Undocumented { };\n\n"
         "/*! Класа која је наслеђена јавним наслеђивањем */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! Шаблонска класа */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Класа која је наслеђена заштићеним наслеђивањем */\n"
         "class ProtectedBase { };\n\n"
         "/*! Класа која је наслеђена јавним наслеђивањем */\n"
         "class PrivateBase { };\n\n"
         "/*! Класа коју користи наслеђена класа */\n"
         "class Used { };\n\n"
         "/*! Надкласа која наслеђује неки број других класа */\n"
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
         "Ако је \\c MAX_DOT_GRAPH_HEIGHT таг у конфигурационој датотеци "
         "подешен на 240, то ће резултовати на следећи начин:"
         "<p><center><img alt=\"\" src=\"graph_legend." + Config::getEnum("dot-image-format").toUtf8() + "\"></center>\n"
         "<p>\n"
         "Правоугаоници имају следеће значење:\n"
         "<ul>\n"
         "<li>%Пуни сиви правоугаоник представља структуру или класу за коју је "
         "граф направљен.\n"
         "<li>%Правоугаоник са црним оквиром означава документовану структуру или класу.\n"
         "<li>%Правоугаоник са сивим оквиром означава недокументовану структуру или класу.\n"
         "<li>%Правоугаоник са црвеним оквиром означава документовану структуру или класу за"
         "за коју нису све релације наслеђивања/садржавања приказане. %Граф је "
         "одсечен ако излази из специфицираних оквира.\n"
         "</ul>\n"
         "Стрелице имају следећа значења:\n"
         "<ul>\n"
         "<li>%Тамноплава стрелица се користи да прикаже релацију јавног извођења "
         "између двеју класа.\n"
         "<li>%Тамнозелена стрелица се користи за заштићено наслеђивање.\n"
         "<li>%Тамноцрвена стрелица се користи за приватно наслеђивање.\n"
         "<li>%Љубичаста испрекидана стрелица се користи ако класа садржи или користи "
         "друга класа. Стрелица је означена променљивом/променљивама "
         "кроз које је показивана класа или структура доступна.\n"
         "<li>%Жута испрекидана стрелица означава везу између примерка шаблона и "
         "и шаблонске класе из које је инстанцирана. Стрелица је означена "
         "параметрима примерка шаблона.\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "легенда";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "Тест";
   }
   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "Списак тестова";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Своства";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Документација свосјтва";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config::getBool("optimize-c")) {
         return "Структуре";
      } else {
         return "Класе";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Пакет " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Списак пакета";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Овде су пакети са кратким описима (ако су доступни):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Пакети";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Вредност:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Грешка";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Списак грешака";
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
      return "1251";
   }


   /*! Used as ansicpg for RTF fcharset
    *  \see trRTFansicp() for a table of possible values.
    */
   virtual QByteArray trRTFCharSet() {
      return "204";
   }

   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "Индекс";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Клас" : "клас"));
      if (!singular) {
         result += "e";
      } else {
         result += "a";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Датотек" : "датотек"));
      if (!singular) {
         result += "e";
      } else {
         result += "a";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Простор" : "простор"));
      if (!singular) {
         result += "и имена";
      } else {
         result += " имена";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Груп" : "груп"));
      if (!singular) {
         result += "е";
      } else {
         result += "a";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Страниц" : "страниц"));
      if (!singular) {
         result += "е";
      } else {
         result += "a";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Члан" : "члан"));
      if (!singular) {
         result += "ови";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Глобалн" : "глобалн"));
      if (!singular) {
         result += "а";
      } else {
         result += "о";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Аутор" : "аутор"));
      if (!singular) {
         result += "и";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Референце";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Имплементира " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Имплементирано у " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Садржај";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Списак застарелог";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Догађаји";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Документација догажаја";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Типови пакета";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Функције пакета";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Статичке функције пакета";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Атрибути пакета";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Статички атрибути пакета";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Све";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Овде је граф позивања за ову функцију:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Резултати претраге";
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
         return "Жао ми је, али нема докумената који одговарају упиту.";
      } else if (numDocuments == 1) {
         return "Пронађен <b>1</b> документ који одговара упиту.";
      } else if (numDocuments == 2) {
         return "Пронађена <b>а</b> документа која одговарају упиту.";
      } else if (numDocuments == 3) {
         return "Пронађена <b>3</b> документа која одговарају упиту.";
      } else if (numDocuments == 4) {
         return "Пронађена <b>4</b> документа која одговарају упиту.";
      } else {
         return "Пронађено <b>$num</b> докумената који одговарају упиту. "
                "Приказују се прво најбољи поготци.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Поготци:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return filename + " Изворна датотека";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Хијерархија директоријума";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Документација директоријума";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Директоријуми";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Ова хијерархија директоријума је уређена "
             "приближно по абецеди:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " Референца директоријума";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Директоријум" : "директоријум"));
      if (singular) {
         result += "";
      } else {
         result += "и";
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
      return "Ово је преоптерећена функција чланица. "
             "Разликује се од наведене само по врсти аргумената кое прихвата";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Ово је граф функција које позивају ову функцију:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Документација вредности набрајања";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Документацијаr функције чланице, односно потпрограма члана";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Списак типова података";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Поља";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Овде су типови података са кратким описима:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Овде је списак свих ";
      if (!extractAll) {
         result += "документованих ";
      }
      result += "чланова типова података";
      result += " са везама ка ";
      if (!extractAll) {
         result += "документацији структуре података за сваки члан";
      } else {
         result += "типовима података којима припадају:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Индекс типова података";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Документација типова података";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Функције/потпрограми";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Документација функције/потпрограма";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Типови података";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Списак модула";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Овде је списак свих ";
      if (!extractAll) {
         result += "документованих ";
      }
      result += "модула са кратким описима:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      if (isTemplate) {
         result += " Шаблон";
      }
      result += " Референца";
      switch (compType) {
         case ClassDef::Class:
            result += " модула";
            break;
         case ClassDef::Struct:
            result += " типа";
            break;
         case ClassDef::Union:
            result += " уније";
            break;
         case ClassDef::Interface:
            result += " интерфејса";
            break;
         case ClassDef::Protocol:
            result += " протокола";
            break;
         case ClassDef::Category:
            result += " категорије";
            break;
         case ClassDef::Exception:
            result += " изузетка";
            break;
         default:
            break;
      }
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Референца модула";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Чланови модула";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Овде је списак свих ";
      if (!extractAll) {
         result += "документованих ";
      }
      result += "чланова модула са везама ка ";
      if (extractAll) {
         result += "документацији модула за сваки члан:";
      } else {
         result += "модулима којима припадају:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Индекс модула";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Модул" : "модул"));
      if (!singular) {
         result += "и";
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
      QByteArray result = (QByteArray)"Документација за ";
      switch (compType) {
         case ClassDef::Class:
            result += "овај модул";
            break;
         case ClassDef::Struct:
            result += "овај тип";
            break;
         case ClassDef::Union:
            result += "ову унију";
            break;
         case ClassDef::Interface:
            result += "овај интерфејс";
            break;
         case ClassDef::Protocol:
            result += "овај протокол";
            break;
         case ClassDef::Category:
            result += "ову категорију";
            break;
         case ClassDef::Exception:
            result += "овај изузетак";
            break;
         default:
            break;
      }
      result += " је направљен из следећ";
      if (single) {
         result += "е датотеке:";
      } else {
         result += "их датотека:";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Тип" : "тип"));
      if (!singular) {
         result += "ови";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Потпрограм" : "потпрограм"));
      if (!singular) {
         result += "и";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Ограничења типова";
   }

};

#endif
