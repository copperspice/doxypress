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

#ifndef TRANSLATOR_RU_H
#define TRANSLATOR_RU_H

class TranslatorRussian : public Translator
{
 public:
   /*! Used for identification of the language. */
   virtual QByteArray idLanguage() {
      return "russian";
   }

   /* Used to get the command(s) for the language support. */
   virtual QByteArray latexLanguageSupportCommand() {
      return "\\usepackage[T2A]{fontenc}\n\\usepackage[russian]{babel}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Относящиеся к классу функции";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(не члены класса)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Подробное описание";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Определения типов";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Перечисления";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Методы";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Поля";
      } else {
         return "Данные класса";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Подробнее...";
   }

   /*! put in the class documentation */
   /* Dosn't use when optimization for C is on. */
   virtual QByteArray trListOfAllMembers() {
      return "Полный список членов класса";
   }

   /*! used as the title of the "list of all members" page of a class */
   /* Dosn't use when optimization for C is on. */
   virtual QByteArray trMemberList() {
      return "Cписок членов класса";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   /* Dosn't use when optimization for C is on. */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Полный список членов класса ";
   }

   /*! this is the remainder of the sentence after the class name */
   /* Dosn't use when optimization for C is on. */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", включая наследуемые из базового класса";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Автоматически создано DoxyPress";
      if (s) {
         result += QByteArray(" для ") + s;
      }
      result += " из исходного текста.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "перечисление";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "элементы перечисления";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "определено в";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Группы";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Иерархия классов";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config::getBool("optimize-c")) {
         return "Структуры данных";
      } else {
         return "Классы";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Файлы";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config::getBool("optimize-c")) {
         return "Поля структур";
      } else {
         return "Члены классов";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   /*??*/
   virtual QByteArray trFileMembers() {
      return "Список членов всех файлов";
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages()
   /* ?? Вариант перевода "См. также: " более удачный, но не в заголовке,
    как в данном случае. */
   {
      return "Описания";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Примеры";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Поиск";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Иерархия классов.";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Полный список ";
      if (!extractAll) {
         result += "документированных ";
      }
      result += "файлов.";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {
      if (Config::getBool("optimize-c")) {
         return "Структуры данных с их кратким описанием.";
      } else {
         return "Классы с их кратким описанием.";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Список всех ";
      if (!extractAll) {
         result += "документированных ";
      }
      if (Config::getBool("optimize-c")) {
         result += "членов структур данных со ссылками на ";
      } else {
         result += "членов классов со ссылками на ";
      }
      if (!extractAll) {
         if (Config::getBool("optimize-c")) {
            result += "документацию по структуре для каждого члена.";
         } else {
            result += "документацию по классу для каждого члена.";
         }
      } else {
         if (Config::getBool("optimize-c")) {
            result += "структуры";
         } else {
            result += "классы";
         }
         result += ", к которым они принадлежат.";
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Список всех ";
      if (!extractAll) {
         result += "документированных ";
      }

      if (Config::getBool("optimize-c")) {
         result += "функций, переменных, макроопределений, "
                   "перечислений и определений типов";
      } else {
         result += "членов файлов ";
      }
      result += " со ссылками на ";
      if (extractAll) {
         result += "файлы, к которым они принадлежат.";
      } else {
         result += "документацию.";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Полный список примеров.";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Полный список дополнительных описаний.";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Полный список групп.";
   }

   // index titles (the project name is prepended for these)


   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Документация";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Алфавитный указатель групп";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Иерархический список классов";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config::getBool("optimize-c")) {
         return "Алфавитный указатель структур данных";
      } else {
         return "Алфавитный указатель классов";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "Список файлов";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Группы";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Структуры данных";
      } else {
         return "Классы";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Файлы";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Примеры";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Тематические описания";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Оглавление";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Макросы";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Определения типов";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Перечисления";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Функции";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Переменные";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Элементы перечислений";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Макросы";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Типы";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Перечисления";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Функции";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Переменные";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config::getBool("optimize-c")) {
         return "Структуры данных";
      } else {
         return "Классы";
      }

   }

   /*! This is used in the documentation of a group before the list of
    *  links to documented files
    */
   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = "Документация ";
      if (projName) {
         result += QByteArray("по ") + projName;
      }
      result += QByteArray(". Последние изменения: ") + date;
      result += ". Создано системой";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return QByteArray("Граф наследования:") + clName + ":";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Только для внутреннего использования";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Предупреждения";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Версия";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Дата";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Возвращает";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "См. также";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Аргументы";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Исключения";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Создано системой";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Пространства имен";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Полный список ";
      if (!extractAll) {
         result += "документированных ";
      }
      result += "пространств имен.";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Друзья";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Документация по друзьям класса и функциям, относящимся"
             " к классу";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result;
      if (isTemplate) {
         result = "Шаблон ";
         switch (compType) {
            case ClassDef::Class:
               result += "класса";
               break;
            case ClassDef::Struct:
               result += "структуры";
               break;
            case ClassDef::Union:
               result += "объединения";
               break;
            case ClassDef::Interface:
               result += "интерфейса";
               break;
            case ClassDef::Protocol:
               result += "протокола";
               break;
            case ClassDef::Category:
               result += "категории";
               break;
            case ClassDef::Exception:
               result += "исключения";
               break;
            default:
               break;
         }
      } else {
         switch (compType) {
            case ClassDef::Class:
               result += "Класс";
               break;
            case ClassDef::Struct:
               result += "Структура";
               break;
            case ClassDef::Union:
               result += "Объединение";
               break;
            case ClassDef::Interface:
               result += "Интерфейс";
               break;
            case ClassDef::Protocol:
               result += "Протокол";
               break;
            case ClassDef::Category:
               result += "Категория";
               break;
            case ClassDef::Exception:
               result += "Исключение";
               break;
            default:
               break;
         }
      }
      result += " ";
      return result + clName;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      return QByteArray("Файл ") + fileName;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      return QByteArray("Пространство имен ") + namespaceName;
   }

   virtual QByteArray trPublicMembers() {
      return "Открытые члены";
   }
   virtual QByteArray trPublicSlots() {
      return "Открытые слоты";
   }
   virtual QByteArray trSignals() {
      return "Сигналы";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Открытые статические члены";
   }
   virtual QByteArray trProtectedMembers() {
      return "Защищенные члены";
   }
   virtual QByteArray trProtectedSlots() {
      return "Защищенные слоты";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Защищенные статические члены";
   }
   virtual QByteArray trPrivateMembers() {
      return "Закрытые члены";
   }
   virtual QByteArray trPrivateSlots() {
      return "Закрытые слоты";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Закрытые статические члены";
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
      return "Базовые классы:" + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Производные классы:" + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      QByteArray result = "Переопределяет метод";
      if (numEntries > 1) {
         result += "ы предков";
      } else {
         result += " предка";
      }
      return result + " " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Переопределяется в " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Члены пространств имен";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Полный список ";
      if (!extractAll) {
         result += "документированных ";
      }
      result += "членов простанств имен.";
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Алфавитный указатель пространств имен";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Пространства имен";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "Пространства имен";
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
      QByteArray result = QByteArray("Объявления и описания членов ");
      switch (compType) {
         case ClassDef::Class:
            result += "класс";
            if (single) {
               result += "а";
            } else {
               result += "ов";
            }
            break;
         case ClassDef::Struct:
            result += "структур";
            if (single) {
               result += "ы";
            }
            break;
         case ClassDef::Union:
            result += "объединени";
            if (single) {
               result += "я";
            } else {
               result += "й";
            }
            break;
         case ClassDef::Interface:
            result += "интерфейс";
            if (single) {
               result += "а";
            } else {
               result += "ов";
            }
            break;
         case ClassDef::Protocol:
            result += "протокол";
            if (single) {
               result += "а";
            } else {
               result += "ов";
            }
            break;
         case ClassDef::Category:
            result += "категори";
            if (single) {
               result += "и";
            } else {
               result += "й";
            }
            break;
         case ClassDef::Exception:
            result += "исключени";
            if (single) {
               result += "я";
            } else {
               result += "й";
            }
            break;
         default:
            break;
      }
      result += " находятся в файл";
      if (single) {
         result += "е:";
      } else {
         result += "ах:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Возвращаемые значения";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Титульная страница";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "стр.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991106
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "См. определение в файле @1 строка @0";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "См. определение в файле @0";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Уст.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Граф связей класса " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Граф включаемых заголовочных файлов для " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Конструктор(ы)";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "См. исходные тексты.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "См. документацию.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Предусловие";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Постусловие";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Инвариант";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Инициализатор";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "Исходные тексты";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Иерархия классов. Графический вид.";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "см. графический вид.";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "см. текстовый вид.";
   }
   virtual QByteArray trPageIndex() {
      return "Алфавитный указатель тематических описаний";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Заметки";
   }
   virtual QByteArray trPublicTypes() {
      return "Открытые типы";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config::getBool("optimize-c")) {
         return "Поля данных";
      } else {
         return "Открытые атрибуты";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Статические открытые данные";
   }
   virtual QByteArray trProtectedTypes() {
      return "Защищенные типы";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Защищенные данные";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Статические защищенные данные";
   }
   virtual QByteArray trPrivateTypes() {
      return "Закрытые типы";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Закрытые данные";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Закрытые статические данные";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a todo item */
   virtual QByteArray trTodo()
   /*??*/
   {
      return "Необходимо сделать";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList()
   /*??*/
   {
      return "Список задач";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Используется в";
   }
   virtual QByteArray trRemarks() {
      return "Прим.";
   }
   virtual QByteArray trAttention() {
      return "Внимание";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Граф файлов, в которые включается этот файл:";
   }
   virtual QByteArray trSince()
   /*??*/
   {
      return "Начиная с";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Легенда";
   }
   /*! page explaining how the dot graph's should be interpreted */
   virtual QByteArray trLegendDocs() {
      return
         "Обозначения, используемые в графах.<p>\n"
         "Рассмотрим следующий пример:\n"
         "\\code\n"
         "/*! Невидимый класс из-за усечения */\n"
         "class Invisible { };\n\n"
         "/*! Усеченный класс, отношение наследования скрыто */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Недокументированный класс */\n"
         "class Undocumented { };\n\n"
         "/*! Открытое наследование */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! Шаблон класса */\n"
         "template<class T> class Templ {};\n\n"
         "/*! Защищенное наследование */\n"
         "class ProtectedBase { };\n\n"
         "/*! Закрытое наследование */\n"
         "class PrivateBase { };\n\n"
         "/*! Класс, используемый классом Inherited */\n"
         "class Used { };\n\n"
         "/*! Класс, порожденный от других классов */\n"
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
         "Если \\c MAX_DOT_GRAPH_HEIGHT в конфигурационном файле "
         "установлен в 240, получится следующий граф:"
         "<p><center><img src=\"graph_legend." + Config::getEnum("dot-image-format").toUtf8() + "\"></center>\n"
         "<p>\n"
         "Прямоугольники в этом графе имеют следующее значение:\n"
         "<ul>\n"
         "<li>Заполненный черный прямоугольник представляет структуру или класс, "
         "для которого создан граф.\n"
         "<li>Прямоугольник с черной границей обозначает документированную структуру или класс.\n"
         "<li>Прямоугольник с серой границей обозначает недокументированную структуру или класс.\n"
         "<li>Прямоугольник с красной границей обозначает документированную структуру или класс, для которого\n"
         " не все отношения наследования/содержания показаны. Граф усечен, "
         "если он не поместился в указанных границах.\n"
         "</ul>\n"
         "Стрелки имеют следующее значение:\n"
         "<ul>\n"
         "<li>Темно-синяя стрелка используется для изображения отношения открытого наследования "
         "между двумя классами.\n"
         "<li>Темно-зеленая стрелка используется при защищенном наследовании.\n"
         "<li>Темно-красная стрелка используется при закрытом наследовании.\n"
         "<li>Фиолетовая стрелка используется, если класс содержится в"
         "другом класе или используется другим классом."
         "Со стрелкой указывается переменная, "
         "через которую доступен указываемый класс или структура. \n"
         "<li>Желтая стрелка используется для связи подстановки шаблона и "
         "шаблона, на основе которого эта подстановка выполнена. С шаблоном"
         "указывается параметр подстановки.\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "см. легенду";
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
      return "Список тестов";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Свойства";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Полный список свойств";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config::getBool("optimize-c")) {
         return "Структуры данных";
      } else {
         return "Классы";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return QByteArray("Пакет ") + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Полный список пакетов ";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Полный список документированных пакетов.";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Пакеты";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Макроопределение:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Ошибка";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Ошибки";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.6
   //////////////////////////////////////////////////////////////////////////
   /*! Used as ansicpg for RTF file */
   virtual QByteArray trRTFansicp() {
      return "1251";
   }
   /*! Used as ansicpg for RTF fcharset */
   virtual QByteArray trRTFCharSet() {
      return "204";
   }
   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "Алфавитный указатель";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      if (Config::getBool("optimize-c")) {
         QByteArray result((first_capital ? "Структуры данных" : "структуры данных"));
         return result;
      } else {
         QByteArray result((first_capital ? "Класс" : "класс"));
         if (!singular) {
            result += "ы";
         }
         return result;
      }
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Файл" : "файл"));
      if (!singular) {
         result += "ы";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Пространств" : "пространств"));
      result += (singular ? "о имен" : "а имен");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Групп" : "групп"));
      result += (singular ? "а" : "ы");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Страниц" : "страниц"));
      result += (singular ? "а" : "ы");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Член" : "член"));
      if (!singular) {
         result += "ы";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Глобальны" : "глобальны"));
      result += (singular ? "й" : "е");
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Автор" : "автор"));
      if (!singular) {
         result += "ы";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Перекрестные ссылки";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Замещает " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implementation this member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Замещается в " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Оглавление";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Список устаревших определений и описаний";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "События";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Cобытия";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Типы с областью видимости пакета";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Функции с областью видимости пакета";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Статические функции с областью видимости пакета";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Переменные с областью видимости пакета";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Статические переменные с областью видимости пакета";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Указатель";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Граф вызовов:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Результаты поиска";
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
         return "К сожалению, по Вашему запросу ничего не найдено.";
      } else if ( numDocuments == 1 ) {
         return "Найден 1 документ.";
      } else {
         return "Найден(о) <b>$num</b> документ(ов). "
                "Документы отсортированы по релевантности.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Найдено:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return "Исходный файл " + filename;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Дерево директорий";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Директории";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Алфавитный указатель директорий";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Дерево директорий";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = QByteArray("Содержание директории ") + dirName;
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Директори" : "директори"));
      if (singular) {
         result += "я";
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
      return "Эта функция перегружена и предоставляется исключительно "
             "для удобства использования. Она отличается от вышеупомянутой "
             "только фактическими аргументами.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Граф вызова функции:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Элементы перечислений";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////
   // Простите переводчика, уже лет 20 не писал на фортране...
   // Любые замечания приму с благодарностью.

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Функции/подпрограммы";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Типы данных";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Поля данных";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Аннотированный список типов данных:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Список всех ";
      if (!extractAll) {
         result += "документированных ";
      }
      result += "членов типа со ссылками ";
      if (!extractAll) {
         result += "на документацию для каждого члена:";
      } else {
         result += "на содержащую структуру:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Типы данных";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Оглавление типов данных";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Функции/подпрограммы";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Функции/подпрограммы";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Типы данных";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Указатель модулей";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Аннотированный список";
      if (!extractAll) {
         result += "документированных ";
      }
      result += "модулей:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      if (isTemplate) {
         switch (compType) {
            case ClassDef::Class:
               result += " Модуль";
               break;
            case ClassDef::Struct:
               result += " Тип";
               break;
            case ClassDef::Union:
               result += " Объединение";
               break;
            case ClassDef::Interface:
               result += " Интерфейс";
               break;
            case ClassDef::Protocol:
               result += " Протокол";
               break;
            case ClassDef::Category:
               result += " Категория";
               break;
            case ClassDef::Exception:
               result += " Исключение";
               break;
            default:
               break;
         }
      } else {
         result += " Шаблон ";
         switch (compType) {
            case ClassDef::Class:
               result += "модуля";
               break;
            case ClassDef::Struct:
               result += "типа";
               break;
            case ClassDef::Union:
               result += "объединения";
               break;
            case ClassDef::Interface:
               result += "интерфейса";
               break;
            case ClassDef::Protocol:
               result += "протокола";
               break;
            case ClassDef::Category:
               result += "категории";
               break;
            case ClassDef::Exception:
               result += "исключения";
               break;
            default:
               break;
         }
      }
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      return QByteArray("Модуль ") + namespaceName;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Члены модуля";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Список всех ";
      if (!extractAll) {
         result += "документированных ";
      }
      result += "модулей со ссылками ";
      if (extractAll) {
         result += "на документацию для каждого члена:";
      } else {
         result += "на модули, их содержащие:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Указатель модулей";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Модул" : "модул"));
      if (singular) {
         result += "ь";
      } else {
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
      QByteArray result = (QByteArray)"Документация по ";
      switch (compType) {
         case ClassDef::Class:
            result += "модулю";
            break;
         case ClassDef::Struct:
            result += "типу";
            break;
         case ClassDef::Union:
            result += "объединению";
            break;
         case ClassDef::Interface:
            result += "интерфейсу";
            break;
         case ClassDef::Protocol:
            result += "протоколу";
            break;
         case ClassDef::Category:
            result += "категории";
            break;
         case ClassDef::Exception:
            result += "исключению";
            break;
         default:
            break;
      }
      result += " сгенерирована на основе следующ";
      if (single) {
         result += "его файла:";
      } else {
         result += "их файлов:";
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
         result += "ы";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Подпрограмм" : "подпрограмм"));
      if (singular) {
         result += "а";
      } else {
         result += "ы";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Согласование типов";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return QByteArray(name) + " Связь";
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Загрузка...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Глобальное пространство имён";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Поиск...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Не найдено";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Файл в " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Включает файл в " + name;
   }

   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Библиографические ссылки";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Авторство";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Директория графа зависимостей ") + name + ":";
   }

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "уровень детализации";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Параметры шаблона";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "и " + number + " больше...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "Документация для этого перечисления сгенерерирована из файл";
      if (!single) {
         result += "ов";
      }
      result += "а:";
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return QByteArray(name) + " Ссылки на перечисление";
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " унаследованные от " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Дополнительные унаследованные члены";
   }

   ///////////////////////////////////////////////////////////////////////
   // new since 1.8.2
   ///////////////////////////////////////////////////////////////////////

   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      QByteArray opt = enable ? "включить" : "выключить";
      return "нажмите на " + opt + " для синхронизации панелей";
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "По группам @1.";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "Расширяет класс @1.";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "Методы класса";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "Методы экземпляра";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "Документация метода";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      return "Обзор дизайна";
   }

   ///////////////////////////////////////////////////////////////////////
   // new since 1.8.4
   ///////////////////////////////////////////////////////////////////////

   /** old style UNO IDL services: implemented interfaces */
   virtual QByteArray trInterfaces() {
      return "Экспортируемые интерфейсы";
   }

   /** old style UNO IDL services: inherited services */
   virtual QByteArray trServices() {
      return "Включённые сервисы";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroups() {
      return "Постоянные группы";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroupReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Ссылка на постоянную группу";
      return result;
   }
   /** UNO IDL service page title */
   virtual QByteArray trServiceReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " Ссылка на сервис";
      return result;
   }
   /** UNO IDL singleton page title */
   virtual QByteArray trSingletonReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " Ссылка на одиночку";
      return result;
   }
   /** UNO IDL service page */
   virtual QByteArray trServiceGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"Документация для этого сервиса "
                          "сгенерирована из следующего файл";
      if (single) {
         result += "а:";
      } else {
         result += "ов:";
      }
      return result;
   }
   /** UNO IDL singleton page */
   virtual QByteArray trSingletonGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"Документация по этому одиночке "
                          "сгенерирована из следующего файл";
      if (single) {
         result += "а:";
      } else {
         result += "ов:";
      }
      return result;
   }

   ///////////////////////////////////////////////////////////////////////
};

#endif
