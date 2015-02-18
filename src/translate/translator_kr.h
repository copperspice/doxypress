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

#ifndef TRANSLATOR_KR_H
#define TRANSLATOR_KR_H

class TranslatorKorean : public Translator
{
 protected:
   friend class TranslatorAdapterBase;
   virtual ~TranslatorKorean() {}

 public:

   // --- Language control methods -------------------

   /*! Used for identification of the language. The identification
    * should not be translated. It should be replaced by the name
    * of the language in English using lower-case characters only
    * (e.g. "czech", "japanese", "russian", etc.). It should be equal to
    * the identification used in language.cpp.
    */
   virtual QByteArray idLanguage() {
      return "korean";
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
      // I'm not sure what this should be.
      // When I figure it out, I'll update this.
      // see http://www.ktug.or.kr/jsboard/read.php?table=operate&no=4422&page=1
      return "\\usepackage{hfont}\n";
   }

   // --- Language translation methods -------------------

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "관련된 함수들";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(다음은 멤버 함수들이 아닙니다. 주의하십시오.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "상세한 설명";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "멤버 타입정의 문서화";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "멤버 열거형 문서화";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "멤버 함수 문서화";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "필드 문서화";
      } else {
         return "멤버 데이타 문서화";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "더 자세히 ...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "모든 멤버 목록";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "멤버 목록";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "다음에 대한 모든 멤버의 목록입니다 : ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return " (모든 상속된 멤버들도 포함합니다.)";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "소스 코드로부터 ";
      if (s) {
         result += s + (QByteArray)"를 위해 ";
      }
      result += "Doxygen에 의해 자동으로 생성됨.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "열거형 이름";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "열거형 값";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "다음에서 정의됨 :";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "모듈";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "클래스 계통도";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "데이타 구조";
      } else {
         return "클래스 목록";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "파일 목록";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "데이타 필드";
      } else {
         return "클래스 멤버";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "전역";
      } else {
         return "파일 멤버";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "관련된 페이지";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "예제";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "검색";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "이 상속 목록은 완전하진 않지만 알파벳순으로 대략적으로 정렬되어있습니다.:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "다음은 ";
      if (!extractAll) {
         result += "문서화된 ";
      }
      result += "모든 파일에 대한 목록입니다. (간략한 설명만을 보여줍니다) :";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "다음은 데이타 구조들입니다. (간략한 설명만을 보여줍니다) :";
      } else {
         return "다음은 클래스, 구조체, 공용체 그리고 인터페이스들입니다. "
                "(간략한 설명만을 보여줍니다) :";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "다음은 ";
      if (!extractAll) {
         result += "문서화된 ";
      }
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "모든 구조체와 공용체의 필드들";
      } else {
         result += "모든 클래스 멤버들";
      }
      result += "의 목록입니다. ";

      if (!extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "각 필드들은 해당 필드에 대한 구조체와 공용체의 "
                      "문서화 페이지의 링크를 가지고 있습니다. :";
         } else {
            result += "각 멤버들은 해당 멤버에 대한 클래스의 문서화 페이지의 "
                      "링크를 가지고 있습니다. :";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "각 필드들은 해당 필드가 속해 있는 구조체와 공용체에 "
                      "대한 링크를 가지고 있습니다. :";
         } else {
            result += "각 멤버들은 해당 멤버가 속해 있는 클래스에 대한 "
                      "링크를 가지고 있습니다. :";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "다음은 ";
      if (!extractAll) {
         result += "문서화된  ";
      }

      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "모든 함수, 변수, 매크로, 열거형, 타입정의들";
      } else {
         result += "파일 멤버들";
      }
      result += "의 목록입니다. ";

      result += "각 항목은 ";
      if (extractAll) {
         result += "그들이 속한 파일 페이지의 링크를 가지고 있습니다. :";
      } else {
         result += "그들에 대한 문서화 페이지의 링크를 가지고 있습니다. :";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "다음은 모든 예제들의 목록입니다.:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "다음은 관련된 모든 문서화 페이지들의 목록입니다.:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "다음은 모든 모듈들의 목록입니다.:";
   }

   // index titles (the project name is prepended for these)

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "문서화";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "모듈 색인";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "계통도 색인";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "데이타 구조 색인";
      } else {
         return "클래스 색인";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "파일 색인";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "모듈 문서화";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "데이타 구조 문서화";
      } else {
         return "클래스 문서화";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "파일 문서화";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "예제 문서화";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "페이지 문서화";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "참조 매뉴얼";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "매크로";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "타입정의";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "열거형 타입";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "함수";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "변수";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "열거형 멤버";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "매크로 문서화";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "타입정의 문서화";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "열거형 타입 문서화";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "함수 문서화";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "변수 문서화";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "데이타 구조";
      } else {
         return "클래스";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"생성시간 : " + date;
      if (projName) {
         result += (QByteArray)", 프로젝트명 : " + projName;
      }
      result += (QByteArray)", 생성자 : ";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)clName + "에 대한 상속 다이어그램 : ";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "내부적적으로만 사용하기 위해.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "경고";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "버전";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "날짜";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "반환값";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "참고";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "매개변수";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "예외";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "다음에 의해 생성됨 : ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "네임스페이스 목록";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "다음은 ";
      if (!extractAll) {
         result += "문서화된 ";
      }
      result += "모든 네임스페이스에 대한 목록입니다. (간략한 설명만을 보여줍니다) :";
      return result;
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "Friends";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "Friend, 그리고 관련된 함수 문서화";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      switch (compType) {
         case ClassDef::Class:
            result += " 클래스";
            break;
         case ClassDef::Struct:
            result += " 구조체";
            break;
         case ClassDef::Union:
            result += " 공용체";
            break;
         case ClassDef::Interface:
            result += " 인터페이스";
            break;
         case ClassDef::Protocol:
            result += " 프로토콜";
            break;
         case ClassDef::Category:
            result += " 카테고리";
            break;
         case ClassDef::Exception:
            result += " 예외";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += " 템플릿";
      }
      result += " 참조";
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = fileName;
      result += " 파일 참조";
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " 네임스페이스 참조";
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Public 멤버 함수";
   }
   virtual QByteArray trPublicSlots() {
      return "Public Slots";
   }
   virtual QByteArray trSignals() {
      return "Signals";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "정적 Public 멤버 함수";
   }
   virtual QByteArray trProtectedMembers() {
      return "Protected 멤버 함수";
   }
   virtual QByteArray trProtectedSlots() {
      return "Protected Slots";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "정적 Protected 멤버 함수";
   }
   virtual QByteArray trPrivateMembers() {
      return "Private 멤버 함수";
   }
   virtual QByteArray trPrivateSlots() {
      return "Private Slots";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "정적 Private 멤버 함수";
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
            //if (i<numEntries-2) // not the fore last entry
            result += ", ";
            //else                // the fore last entry
            //  result+=", ";     // TODO: does the 'and' need to be translated here?
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return trWriteList(numEntries) + "를(을) 상속했습니다.";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return trWriteList(numEntries) + "에 의해 상속되었습니다.";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return trWriteList(numEntries) + "(으)로부터 재구현되었습니다.";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return trWriteList(numEntries) + "에서 재구현되었습니다.";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "네임스페이스 멤버";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "다음은 ";
      if (!extractAll) {
         result += "문서화된 ";
      }
      result += "모든 네임스페이스 멤버들의 목록입니다. ";
      if (extractAll) {
         result += "각 멤버들은 해당 멤버의 네임스페이스 문서화 페이지의 링크를 가지고 있습니다. :";
      } else {
         result += "각 멤버들은 해당 멤버가 속한 네임스페이스 페이지의 링크를 가지고 있습니다. :";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "네임스페이스 색인";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "네임스페이스 문서화";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "네임스페이스";
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
      QByteArray result = (QByteArray)"이 ";
      switch (compType) {
         case ClassDef::Class:
            result += "클래스";
            break;
         case ClassDef::Struct:
            result += "구조체";
            break;
         case ClassDef::Union:
            result += "공용체";
            break;
         case ClassDef::Interface:
            result += "인터페이스";
            break;
         case ClassDef::Protocol:
            result += "프로토콜";
            break;
         case ClassDef::Category:
            result += "카테고리";
            break;
         case ClassDef::Exception:
            result += "예외";
            break;
         default:
            break;
      }
      result += "에 대한 문서화 페이지는 다음의 파일";
      if (!single) {
         result += "들";
      }
      result += "로부터 생성되었습니다.:";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "반환값";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "메인 페이지";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "페이지";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "@1 파일의 @0 번째 라인에서 정의되었습니다.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "@0 파일에서 정의되었습니다.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "잘못된 코드";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)clName + "에 대한 협력 다이어그램:";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)fName + "에 대한 include 의존 그래프";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "생성자 & 소멸자 문서화";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "이 파일의 소스 코드 페이지로 가기";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "이 파일의 문서화 페이지로 가기";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "전제조건";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "후미조건";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "변하지 않는";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "초기값:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "코드";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "그래픽컬한 클래스 계통도";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "그래픽컬한 클래스 계통도 페이지로 가기";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "텍스트 형식의 클래스 계통도 페이지로 가기";
   }
   virtual QByteArray trPageIndex() {
      return "페이지 색인";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "주의";
   }
   virtual QByteArray trPublicTypes() {
      return "Public 타입";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "데이타 필드";
      } else {
         return "Public 속성";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "정적 Public 속성";
   }
   virtual QByteArray trProtectedTypes() {
      return "Protected 타입";
   }
   virtual QByteArray trProtectedAttribs() {
      return "Protected 속성";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "정적 Protected 속성";
   }
   virtual QByteArray trPrivateTypes() {
      return "Private 타입";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Private 속성";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "정적 Private 속성";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "할일";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "할일 목록";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "다음에 의해서 참조됨 : ";
   }
   virtual QByteArray trRemarks() {
      return "Remarks";
   }
   virtual QByteArray trAttention() {
      return "주의";
   }
   virtual QByteArray trInclByDepGraph() {
      return "이 그래프는 이 파일을 직/간접적으로 include 하는 파일들을 보여줍니다.:";
   }
   virtual QByteArray trSince() {
      return "Since";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "그래프 범례";
   }
   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "이 페이지는 doxygen에 의해 생성된 그래프들을 이해하는 방법을 설명합니다.<p>\n"
         "다음의 예제를 참고하십시오.:\n"
         "\\code\n"
         "/*! 생략되었기 때문에 보이지 않는 클래스 */\n"
         "class Invisible { };\n\n"
         "/*! Truncated 클래스, 상속관계가 숨겨짐 */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* doxygen 주석에 의해서 문서화되지 않는 클래스 */\n"
         "class Undocumented { };\n\n"
         "/*! public 상속을 통해서 상속된 클래스 */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! 템플릿 클래스 */\n"
         "template<class T> class Templ { };\n\n"
         "/*! protected 상속을 통해서 상속된 클래스 */\n"
         "class ProtectedBase { };\n\n"
         "/*! private 상속을 통해서 상속된 클래스 */\n"
         "class PrivateBase { };\n\n"
         "/*! 상속되어진 클래스에 의해 (멤버로) 사용되어지는 클래스 */\n"
         "class Used { };\n\n"
         "/*! 다른 클래스들을 상속하는 슈퍼 클래스 */\n"
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
         "다음과 같은 그래프가 출력될 것입니다. :"
         "<p><center><img alt=\"\" src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
         "<p>\n"
         "위 그래프의 박스들은 다음과 같은 의미를 가집니다. :\n"
         "<ul>\n"
         "<li>%A 회색으로 채워진 박스는 이 그래프를 생성해 낸 구조체나 클래스를 의미합니다.\n"
         "<li>%A 검은색 테두리의 박스는 문서화된 구조체나 클래스를 의미합니다.\n"
         "<li>%A 회색 테두리의 박스는 문서화되지 않은 구조체나 클래스를 의미합니다.\n"
         "<li>%A 빨간색 테두리의 박스는 모든 상속이나 포함관계가 보여지지 않는 "
         "구조체나 클래스를 의미합니다."
         "%A 만약 그래프가 지정된 경계내에 맞지 않으면, 그래프가 잘려집니다.\n"
         "</ul>\n"
         "화살표들은 다음과 같은 의미를 가집니다. :\n"
         "<ul>\n"
         "<li>%A 어두운 파랑색 화살표는 두 클래스들 간에 public 상속이 있음을 의미합니다.\n"
         "<li>%A 어두운 연두색 화살표는 protected 상속이 있음을 의미합니다.\n"
         "<li>%A 어두운 빨간색 화살표는 private 상속이 있음을 의미합니다.\n"
         "<li>%A 보라색 점선 화살표는 다른 클래스에 의해 포함되거나 사용되어짐을 의미합니다. "
         "화살표의 라벨은 화살표가 가리키는 클래스나 구조체로 접근하는 변수명(들)으로 붙습니다.\n"
         "<li>%A 노란색 점선 화살표는 템플릿 인스턴스와 템프릿 클래스에 대한 관계를 의미합니다. "
         "화살표의 라벨은 인스턴스의 템플릿 파라메터로 붙습니다.\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "범례";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "테스트";
   }
   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "테스트 목록";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "속성";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "속성 문서화";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "데이타 구조";
      } else {
         return "클래스";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return name + (QByteArray)" 패키지";
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "패키지 목록";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "다음은 패키지들입니다. (가능한한 간략한 설명만을 보여줍니다) :";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "패키지";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "값:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "버그";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "버그 목록";
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
      return "949";
   }


   /*! Used as ansicpg for RTF fcharset
    *  \see trRTFansicp() for a table of possible values.
    */
   virtual QByteArray trRTFCharSet() {
      return "129";
   }

   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "색인";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool, bool singular) {
      QByteArray result("클래스");
      if (!singular) {
         result += "들";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool, bool singular) {
      QByteArray result("파일");
      if (!singular) {
         result += "들";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool, bool singular) {
      QByteArray result("네임스페이스");
      if (!singular) {
         result += "들";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool, bool singular) {
      QByteArray result("그룹");
      if (!singular) {
         result += "들";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool, bool singular) {
      QByteArray result("페이지");
      if (!singular) {
         result += "들";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool, bool singular) {
      QByteArray result("멤버");
      if (!singular) {
         result += "들";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool, bool singular) {
      QByteArray result("전역");
      if (!singular) {
         result += "";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool, bool singular) {
      QByteArray result("작성자");
      if (!singular) {
         result += "들";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "다음을 참조함 : ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return trWriteList(numEntries) + "를 구현.";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return trWriteList(numEntries) + "에서 구현되었습니다.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "목차";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "잘못된 코드 목록";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "이벤트";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "이벤트 문서화";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "패키지 타입";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "패키지 함수";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "정적 패키지 함수";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "패키지 속성";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "정적 패키지 속성";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "모두";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "이 함수 내부에서 호출하는 함수들에 대한 그래프입니다.:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "검색 결과";
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
         return "죄송합니다. 질의에 일치하는 문서가 없습니다.";
      } else if (numDocuments == 1) {
         return "질의에 일치하는 <b>1</b> 개의 문서를 찾았습니다.";
      } else {
         return "질의에 일치하는 <b>$num</b> 개의 문서를 찾았습니다. "
                "가장 많이 일치하는 문서를 가장 먼저 보여줍니다.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "결과:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return filename + " 소스 파일";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "디렉토리 계통도";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "디렉토리 문서화";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "디렉토리";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "이 디렉토리 목록은 완전하진 않지만, (대략적으로) 알파벳순으로 정렬되어있습니다.:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " 디렉토리 참조";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool, bool singular) {
      QByteArray result("디렉토리");
      if (singular) {
         result += "";
      } else {
         result += "들";
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
      return "이 함수는 편의를 제공하기 위해 오버로드된 멤버 함수입니다. "
             "위의 함수와 틀린 점은 단지 받아들이는 아규먼트(argument)가 다르다는 것입니다.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "이 함수를 호출하는 함수들에 대한 그래프입니다.:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "열거형 문서화";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "멤버 함수/서브루틴 문서화";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "데이타 타입 목록";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "데이타 필드";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "대략적인 설명과 함께 데이타 타입들의 목록입니다.:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "다음은 ";
      if (!extractAll) {
         result += "문서화된 ";
      }
      result += "모든 데이타 타입 멤버들의 목록입니다. ";

      result += "각 항목은 ";
      if (!extractAll) {
         result += "각 멤버에 대한 데이타 구조 문서화 페이지의 링크를 가지고 있습니다.";
      } else {
         result += "그들이 속한 데이타 타입의 링크를 가지고 있습니다. :";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "데이타 타입 색인";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "데이타 타입 문서화";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "함수/서브루틴";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "함수/서브루틴 문서화";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "데이타 타입들";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "모듈 목록";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "다음은 ";
      if (!extractAll) {
         result += "문서화된 ";
      }
      result += "모든 모듈에 대한 목록입니다. (간략한 설명만을 보여줍니다) :";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      switch (compType) {
         case ClassDef::Class:
            result += " 모듈";
            break;
         case ClassDef::Struct:
            result += " 타입";
            break;
         case ClassDef::Union:
            result += " 공용체";
            break;
         case ClassDef::Interface:
            result += " 인터페이스";
            break;
         case ClassDef::Protocol:
            result += " 프로토콜";
            break;
         case ClassDef::Category:
            result += " 카테고리";
            break;
         case ClassDef::Exception:
            result += " 예외";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += " 템플릿";
      }
      result += " 참조";
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " 모듈 참조";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "모듈 멤버들";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "다음은 ";
      if (!extractAll) {
         result += "문서화된 ";
      }
      result += "모든 모듈 멤버의 목록입니다. ";
      if (extractAll) {
         result += "각 항목은 각 멤버의 모듈 문서화 페이지의 링크를 가지고 있습니다. :";
      } else {
         result += "각 항목은 그들이 속한 모듈의 링크를 가지고 있습니다. :";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "모듈 색인";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool, bool singular) {
      QByteArray result("모듈");
      if (!singular) {
         result += "들";
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
      QByteArray result = (QByteArray)"다음 파일";
      if (single) {
         result += "";
      } else {
         result += "들";
      }
      result += "로부터 생성된 ";
      result += "이 ";
      switch (compType) {
         case ClassDef::Class:
            result += "모듈";
            break;
         case ClassDef::Struct:
            result += "타입";
            break;
         case ClassDef::Union:
            result += "공용체";
            break;
         case ClassDef::Interface:
            result += "인터페이스";
            break;
         case ClassDef::Protocol:
            result += "프로토콜";
            break;
         case ClassDef::Category:
            result += "카테고리";
            break;
         case ClassDef::Exception:
            result += "예외";
            break;
         default:
            break;
      }
      result += "의 문서화 페이지:";
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool, bool singular) {
      QByteArray result("타입");
      if (!singular) {
         result += "들";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool, bool singular) {
      QByteArray result("서브프로그램");
      if (!singular) {
         result += "들";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "타입 한정자들";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return QByteArray(name) + " 관계";
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "로딩중...";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "전역 이름공간";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "검색중...";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "일치하는것 없음";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return QByteArray(name) + "의 파일";
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return QByteArray(name) + "의 파일 포함";
   }
  
   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "참고 문헌";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "Copyright";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray(name) + QByteArray("에 대한 디렉토리 의존성 그래프:");
   }

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "상세 단계";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "템플릿 파라메터";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return QByteArray("그리고 ") + number + "개 더...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool /*single*/) {
      QByteArray result = "이 열거형에 대한 문서가 다음 파일(들)로부터 생성되었습니다.:";
      return result;
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return QByteArray(name) + " Enum Reference";
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(what) + QByteArray("(으)로부터 상속된 ") + QByteArray(members);
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "추가로 상속된 멤버들";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {
      QByteArray opt = enable ? "활성화" : "비활성화";
      return "패널 동기화를 " + opt + "하기 위해 클릭하십시오";
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "카테고리 @1에 의해 제공됨.";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "클래스 @1 확장.";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "클래스 메소드들";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "인스턴스 메소드들";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "메소드 문서화";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      return "디자인 개요";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.4
   //////////////////////////////////////////////////////////////////////////

   /** old style UNO IDL services: implemented interfaces */
   virtual QByteArray trInterfaces() {
      return "익스포트된 인터페이스들";
   }

   /** old style UNO IDL services: inherited services */
   virtual QByteArray trServices() {
      return "포함된 서비스들";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroups() {
      return "상수 그룹들";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroupReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " 상수 그룹 레퍼런스";
      return result;
   }
   /** UNO IDL service page title */
   virtual QByteArray trServiceReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " 서비스 레퍼런스";
      return result;
   }
   /** UNO IDL singleton page title */
   virtual QByteArray trSingletonReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " 싱글톤 레퍼런스";
      return result;
   }
   /** UNO IDL service page */
   virtual QByteArray trServiceGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"이 서비스에 대한 문서화는 다음의 파일";
      if (!single) {
         result += "들";
      }
      result += "로부터 생성되었습니다.:";
      return result;
   }
   /** UNO IDL singleton page */
   virtual QByteArray trSingletonGeneratedFromFiles(bool single) {
      // single is true implies a single file
      QByteArray result = (QByteArray)"이 싱글톤에 대한 문서화는 다음의 파일";
      if (!single) {
         result += "들";
      }
      result += "로부터 생성되었습니다.:";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////

};

#endif
