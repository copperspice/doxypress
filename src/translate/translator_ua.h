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

#ifndef TRANSLATOR_UA_H
#define TRANSLATOR_UA_H

class TranslatorUkrainian : public TranslatorAdapter_1_8_4
{
 public:
   /*! Used for identification of the language. */
   virtual QByteArray idLanguage() {
      return "ukrainian";
   }

   /* Used to get the command(s) for the language support. */
   virtual QByteArray latexLanguageSupportCommand() {
      return "\\usepackage[T2A]{fontenc}\n\\usepackage[ukrainian]{babel}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "Споріднені фукції";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(не методи компонент)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Детальний опис";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Опис типів користувача";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Опис переліків користувача";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Опис методів компонент";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config::getBool("optimize-c")) {
         return  "Поля" ;
      } else {
         return  "Компонентні дані" ;
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Детальніше...";
   }

   /*! put in the class documentation */
   /* Dosn't use when optimization for C is on. */
   virtual QByteArray trListOfAllMembers() {
      return  "Список всіх елементів" ;
   }

   /*! used as the title of the "list of all members" page of a class */
   /* Dosn't use when optimization for C is on. */
   virtual QByteArray trMemberList() {
      return  "Cписок елементів" ;
   }

   /*! this is the first part of a sentence that is followed by a class name */
   /* Dosn't use when optimization for C is on. */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Повний список елементів";
   }

   /*! this is the remainder of the sentence after the class name */
   /* Dosn't use when optimization for C is on. */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", включаючи всі успадковані елементи";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Автоматично створено за допомогою DoxyPress";
      if (s) {
         result += QByteArray(" для ") + s;
      }
      result += " з тексту програми.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "Перелік";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "Елемент переліку";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "визначено в ";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Програмні модулі";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Ієрархія класів";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config::getBool("optimize-c")) {
         return  "Структури даних" ;
      } else {
         return  "Класи" ;
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Файли";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config::getBool("optimize-c")) {
         return  "Поля структур" ;
      } else {
         return  "Елементи класів" ;
      }
   }

   /*! This is put above each page as a link to all members of files. */
   /*??*/
   virtual QByteArray trFileMembers() {
      return  "Елементи файлу" ;
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages()
   /* ?? Вариант перевода "См. также: " более удачный, но не в заголовке,
    как в данном случае. */
   {
      return "Додаткова інформація";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Приклади";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Пошук";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Список успадкувань впорядковано наближено до алфавіту";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Повний список ";
      if (!extractAll) {
         result += "документованих ";
      }
      result += "файлів.";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {
      if (Config::getBool("optimize-c")) {
         return  "Структури даних з коротким описом." ;
      } else {
         return  "Класи, структури, об'єднання та інтерфейси  з коротким описом." ;
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Список всіх ";
      if (!extractAll) {
         result += "документоаних ";
      }
      if (Config::getBool("optimize-c")) {
         result += "елементів структур даних з посиланням на ";
      } else {
         result += "елементів класів даних з посиланням на ";
      }
      if (extractAll) {
         if (Config::getBool("optimize-c")) {
            result += "документацію по структурі/об'єднанню кожного елементу.";
         } else {
            result += "документацію по класу кожного елементу.";
         }
      } else {
         if (Config::getBool("optimize-c")) {
            result += "відповідні структури";
         } else {
            result += "відповідні класи";
         }
         result += ", до яких вони належать.";
      }
      return  result ;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Список всіх ";
      if (!extractAll) {
         result += "документованих ";
      }

      if (Config::getBool("optimize-c")) {
         result += "функцій, змінних, макровизначень, "
                   "переліків і визначень типів";
      } else {
         result += "елементів файлів ";
      }
      result += " з посиланям на ";
      if (extractAll) {
         result += "файли, до яких вони належать.";
      } else {
         result += "документацію.";
      }
      return  result ;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Повний список прикладів.";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Повний список додаткових описів.";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Повний список модулів.";
   }

   // index titles (the project name is prepended for these)

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Документація";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Алфавітний покажчик модулів";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Ієрархічний покажчик класів";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config::getBool("optimize-c")) {
         return  "Алфавітний покажчик структур даних" ;
      } else {
         return  "Алфавітний покажчик класів" ;
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return  "Покажчик файлв" ;
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Програмні Модулі";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config::getBool("optimize-c")) {
         return  "Структури даних" ;
      } else {
         return  "Класи" ;
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Файли";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Приклади";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Документація по темі";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Довідковий посібник";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Макровизначення";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Визначення типів";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Переліки";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Функції";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Змінні";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Елементи переліків";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Опис макровизначень";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Опис визначень типів";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Опис переліків";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Опис функцій";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Опис змінних";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config::getBool("optimize-c")) {
         return  "Структури даних" ;
      } else {
         return  "Класи" ;
      }

   }

   /*! This is used in the documentation of a group before the list of
    *  links to documented files
    */
   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = "Документація ";
      if (projName) {
         result += QByteArray("до ") + projName;
      }
      result += QByteArray(" створена ") + date;
      result += " системою";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return QByteArray("Схема успадкувань для ") + clName;
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Тільки для внутрішнього користування";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Застереження";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Версія";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Дата";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Повертає";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Див. також";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Аргументи";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Обробка виняткових ситуацій";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Створено системою";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Простір імен";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Повний список ";
      if (!extractAll) {
         result += "документованих ";
      }
      result += "просторів імен.";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Дружні класи";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Документация до дружніх класів та відповідних функцій";
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
               result += "класу";
               break;
            case ClassDef::Struct:
               result += "структури";
               break;
            case ClassDef::Union:
               result += "об'єднання";
               break;
            case ClassDef::Interface:
               result += "інтерфейсу";
               break;
            case ClassDef::Protocol:
               result += "протоколу";
               break;
            case ClassDef::Category:
               result += "категорії";
               break;
            case ClassDef::Exception:
               result += "Виняток";
               break;
            default:
               break;
         }
      } else {
         switch (compType) {
            case ClassDef::Class:
               result += "Клас";
               break;
            case ClassDef::Struct:
               result += "Структура";
               break;
            case ClassDef::Union:
               result += "Об'єднання";
               break;
            case ClassDef::Interface:
               result += "Інтерфейс";
               break;
            case ClassDef::Protocol:
               result += "Протокол";
               break;
            case ClassDef::Category:
               result += "Категорія";
               break;
            case ClassDef::Exception:
               result += "Виняток";
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
      return QByteArray("Простір імен ") + namespaceName;
   }

   virtual QByteArray trPublicMembers() {
      return "Загальнодоступні елементи";
   }
   virtual QByteArray trPublicSlots() {
      return "Загальнодоступні слоти";
   }
   virtual QByteArray trSignals() {
      return "Сигнали";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Загальнодоступні статичні елементи";
   }
   virtual QByteArray trProtectedMembers() {
      return "Захищені елементи";
   }
   virtual QByteArray trProtectedSlots() {
      return "Захищені слоти";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Захищені статичні елементи";
   }
   virtual QByteArray trPrivateMembers() {
      return "Приватні елементи";
   }
   virtual QByteArray trPrivateSlots() {
      return "Приватні слоти";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Приватні статичні елементи";
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
               result += " і " ;
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Успадковує клас " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Успадковано класами " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Переозначення з " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Переозначається в " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Елементи простору імен";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Повний список ";
      if (!extractAll) {
         result += "документованих ";
      }
      result += "елеметів простору імен з посиланням ";
      if (extractAll) {
         result += "на документацію для кожного елементу:";
      } else {
         result += "на простір імен до якого вони належать:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Алфавітний покажчик простору імен";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Опис простору імен";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return  "Простори імен" ;
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
      QByteArray result = (QByteArray)"Документація  ";
      switch (compType) {
         case ClassDef::Class:
            if (single) {
               result += "цього класу";
            } else {
               result += "цих класів";
            }
            break;
         case ClassDef::Struct:
            if (single) {
               result += "цієї структури";
            } else {
               result += "цих структур";
            }
            break;
         case ClassDef::Union:
            if (single) {
               result += "цього об'єднання";
            } else {
               result += "цих об'єднань";
            }
            break;
         case ClassDef::Interface:
            if (single) {
               result += "цього інтерфейсу";
            } else {
               result += "цих інтерфейсів";
            }
            break;
         case ClassDef::Protocol:
            if (single) {
               result += "цього протоколу";
            } else {
               result += "цих протоколів";
            }
            break;
         case ClassDef::Category:
            if (single) {
               result += "цієї категорії";
            } else {
               result += "цих категорій";
            }
            break;
         case ClassDef::Exception:
            if (single) {
               result += "цього винятку";
            } else {
               result += "цих винятків";
            }
            break;
         default:
            break;
      }
      result += " була створена з файл";
      if (single) {
         result += "у:";
      } else {
         result += "ів:";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Значення, що повертаються";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Титульна сторінка";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "стор.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991106
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Див. визначення в файлі @1, рядок @0";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Див. визначення в файлі @0";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Не рекомендовано";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Діаграма зв'язків класу " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return QByteArray("Діаграма включених заголовочних файлів для ") + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Конструктор(и)";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Див. вихідні тексти.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Див. документацію.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Передумови";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Післяумови";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Інваріант";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Початкові значення";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "Вихідні тексти";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Графічна ієрархія класів";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "див. графічну ієрархію";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "див. текстову ієрархію";
   }
   virtual QByteArray trPageIndex() {
      return "Алфавітний покажчик тематичних описів";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Нотатки";
   }
   virtual QByteArray trPublicTypes() {
      return "Загальнодоступні типи";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config::getBool("optimize-c")) {
         return  "Поля даних" ;
      } else {
         return  "Загальнодоступні атрибути" ;
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Статичні загальнодоступні данні";
   }
   virtual QByteArray trProtectedTypes() {
      return "Захищені типи";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Захищені дані";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Статичні захищені дані";
   }
   virtual QByteArray trPrivateTypes() {
      return "Приватні типи";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Приватні дані";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Приватні статичні дані";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a todo item */
   virtual QByteArray trTodo()
   /*??*/
   {
      return "Необхідно зробити";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList()
   /*??*/
   {
      return "Список завдань";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Вживається в";
   }
   virtual QByteArray trRemarks() {
      return "Прим.";
   }
   virtual QByteArray trAttention() {
      return "Увага";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Граф файлів, які включають цей файл:";
   }
   virtual QByteArray trSince()
   /*??*/
   {
      return "Починаючи з";
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
         "Позначення, що використовуються у графах.<p>\n"
         "Розглянемо наступний приклад:\n"
         "\\code\n"
         "/*! Невидимий клас через зрізання */\n"
         "class Invisible { };\n\n"
         "/*! Обмежений клас, відношення успадкування приховане */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Недокументований клас */\n"
         "class Undocumented { };\n\n"
         "/*! Загальнодоступне успадкування */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! A template class */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Захищене успадкування */\n"
         "class ProtectedBase { };\n\n"
         "/*! Приватне успадкування */\n"
         "class PrivateBase { };\n\n"
         "/*! Клас, що використовується класом Inherited */\n"
         "class Used { };\n\n"
         "/*! Клас, що успадковує інші класи */\n"
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
         "Таким чином, отримуємо наступний граф:"
         "<p><center><img src=\"graph_legend." + Config::getEnum("dot-image-format").toUtf8() + "\"></center>\n"
         "<p>\n"
         "Прямокутники в цьому графі мають наступний зміст:\n"
         "<ul>\n"
         "<li>Заповнений чорний прямокутник відображає структуру або клас, "
         "для якого створено граф.\n"
         "<li>Прямокутник з чорною межею відображає документовану структуру або клас.\n"
         "<li>Прямокутник з сірою межею відображає недокументовану структуру або клас.\n"
         "<li>Прямокутник з червоною межею відображає документовану структуру або клас, для яких\n"
         " не всі співвідношення успадкування/змісту показані. Граф зрізаниЙ, "
         "якщо він не вміщається у вказані межі."
         "</ul>\n"
         "Стрілки мають наступний зміст:\n"
         "<ul>\n"
         "<li>Темносиня стрілка відображає відношення загальнодоступного успадкування "
         "між двома класами.\n"
         "<li>Темнозелена стрілка використовується при захищеному успадкуванні.\n"
         "<li>Темночервона стрілка використовується при приватному успадкуванні.\n"
         "<li>Пурпурна стрілка використовується, якщо клас міститься в"
         "іншому класі або ним використовується."
         "Стрілка етикетується змінною, "
         "через яку відбувається доступ до вказаної структури або класу. \n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "див. легенду";
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
      return "Список тестів";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Властивості";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Повний список властивостей";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config::getBool("optimize-c")) {
         return  "Структури даних" ;
      } else {
         return  "Класи" ;
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return QByteArray("Пакет ") + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Повний список пакетів";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Повний список документованих пакетів.";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Пакети";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Макровизначення:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Дефект";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Список дефектів";
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
      return "Предметний покажчик";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      if (Config::getBool("optimize-c")) {
         QByteArray result((first_capital ? "Структури даних" : "структури даних"));
         return result;
      } else {
         QByteArray result((first_capital ? "Клас" : "клас"));
         if (!singular) {
            result += "и";
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
         result += "и";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Прост" : "прост"));
      result += (singular ? "ір імен" : "ори імен");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Груп" : "груп"));
      result += (singular ? "а" : "и");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Сторінк" : "сторінк"));
      result += (singular ? "а" : "и");
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Елемент" : "елемент"));
      if (!singular) {
         result += "и";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Глобальн" : "глобальн"));
      result += (singular ? "ий" : "і");
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
      return "Використовує";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Реалізує " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Реалізує в " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Зміст";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Застарілі елементи";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Події";
   }

   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Документація подій";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Типи пакетів";
   }

   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Функції пакетів";
   }

   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Статичні функцію пакетів";
   }

   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Атрибути пакетів";
   }

   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Статичні атрибути пакетів";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Всі";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Граф всіх викликів цієї функції:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Результати пошуку";
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
         return "Не знайдено документів відповідно до вашого запиту.";
      } else if (numDocuments == 1) {
         return "Було знайдено <b>1</b> документ відповідно до вашого запиту.";
      } else {
         return "Було знайдено <b>$num</b> документів відповідно до вашого запиту. "
                "Найкращі відповідності показано спочатку.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Відповідність:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return "Текст програми " + filename;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Дерево каталогів";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Документація каталогів";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of a HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Каталоги";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Дерево каталогів впорядковано наближено "
             "до алфавіту:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = "Довідник каталогу ";
      result += dirName;
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool, bool singular) {
      QByteArray result("Каталог");
      if (!singular) {
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
      return "Це перезавантажуваний метод, "
             "ввкдкний для зручності. Він відрізняється від "
             "описаної вище функції лише списком аргуметнів.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Граф викликів для цієї функції:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Документація переліку";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Документація функції/підпрограми";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Список типів даних";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Поля даних";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Типи даних з коротким описом:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Список усіх ";
      if (!extractAll) {
         result += "задокументованих ";
      }
      result += "елементів типів даних";
      result += " з посиланнями на ";
      if (!extractAll) {
         result += "документацію структури даних кожного елемента";
      } else {
         result += "типи даних, до яких вони належать:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Список типів даних";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Документація типів даних";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Функції/підпрограми";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Документація функцій/підпрограм";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Типи даних";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Список модулів";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Список усіх ";
      if (!extractAll) {
         result += "задокументованих ";
      }
      result += "модулів з коротким описом:";
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
               result += " Об'єднання";
               break;
            case ClassDef::Interface:
               result += " Інтерфейс";
               break;
            case ClassDef::Protocol:
               result += " Протокол";
               break;
            case ClassDef::Category:
               result += " Категорія";
               break;
            case ClassDef::Exception:
               result += " Виняток";
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
               result += "типу";
               break;
            case ClassDef::Union:
               result += "об'єднання";
               break;
            case ClassDef::Interface:
               result += "інтерфейсу";
               break;
            case ClassDef::Protocol:
               result += "протоколу";
               break;
            case ClassDef::Category:
               result += "категорії";
               break;
            case ClassDef::Exception:
               result += "винятка";
               break;
            default:
               break;
         }
      }
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Модуль";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Елементи модуля";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Список усіх ";
      if (!extractAll) {
         result += "задокументованих ";
      }
      result += "елементів модулів з посиланнями на ";
      if (extractAll) {
         result += "документацію модуля для кожного елемента:";
      } else {
         result += "модулі, до яких вони належать:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Алфавітний покажчик модулів";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Модул" : "модул"));
      result += (singular ? "ь" : "і");
      return result;
   }

   /*! This is put at the bottom of a module documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFilesFortran(ClassDef::CompoundType compType,
         bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"Документацію для ";
      switch (compType) {
         case ClassDef::Class:
            result += "цього модуля";
            break;
         case ClassDef::Struct:
            result += "цього типу";
            break;
         case ClassDef::Union:
            result += "цього об'єднання";
            break;
         case ClassDef::Interface:
            result += "цього інтерфейсу";
            break;
         case ClassDef::Protocol:
            result += "цього протоколу";
            break;
         case ClassDef::Category:
            result += "цієї категорії";
            break;
         case ClassDef::Exception:
            result += "цього винятку";
            break;
         default:
            break;
      }
      result += " було згенеровано з ";
      if (single) {
         result += "наступного файлу:";
      } else {
         result += "наступних файлів:";
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
         result += "и";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Підпрограм" : "підпрограм"));
      result += (singular ? "а" : "и");
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Обмеження типу";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return "Зв'язок з " + QByteArray(name);
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "Завантаження...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "Глобальний простір імен";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "Пошук...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "Нічого не знайдено";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"Файл у " + name;
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"Включає файли в " + name;
   }

   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "Бібліографічні посилання";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Копірайт";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray("Граф залежностей каталогу для ") + name + ":";
   }

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "рівень елемента";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "Параметри шаблона";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "і ще  " + number + "...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool single) {
      QByteArray result = "Документацію для цього переліку булозгенеровано з ";
      result += (single ? "наступного файла" : "наступних файлів");
      result += ":";
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return QByteArray(name) + " Перелік";
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + " успадковано з " + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "Додаткові успадковані елементи";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      QByteArray opt = enable ? "дозволити" : "заборонити";
      return "клацніть мишкою, щоб " + opt + " синхронізацію панелі";
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "Забезпечено категорією @1.";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "Розширює клас @1.";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "Методи класу";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "Методи об'єкта";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "Документація метода";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      return "Огляд дизайну проекту";
   }

   //////////////////////////////////////////////////////////////////////////

};

#endif

