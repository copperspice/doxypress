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

#ifndef TRANSLATOR_JP_H
#define TRANSLATOR_JP_H

class TranslatorJapanese : public Translator
{
 public:
   virtual QByteArray idLanguage() {
      return "japanese";
   }

   virtual QByteArray latexLanguageSupportCommand() {
      return "";
   }

   /*! used in the compound documentation before a list of related functions. */
   virtual QByteArray trRelatedFunctions() {
      return "関連関数";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "（これらはメソッドではありません）";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "詳解";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "型定義メンバ詳解";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "列挙型メンバ詳解";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      if ( Config_getBool("OPTIMIZE_OUTPUT_JAVA")) {
         return "メソッド詳解";
      } else {
         return "関数詳解";
      }
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if ( Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "フィールド詳解";
      } else {
         return "メンバ詳解";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "[詳解]";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "全メンバ一覧";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "メンバ一覧";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "継承メンバを含む ";
   }
   /* trIncludingInheritedMembers に続くように定義すること */

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return " の全メンバ一覧です。";
   }
   /* trThisIsTheListOfAllMembers から続くように定義すること */

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Doxygen により";
      if (s) {
         result = (QByteArray)" " + s + "の";
      }
      result += "ソースコードから抽出しました。";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "列挙名";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "列挙値";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "定義場所: ";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "モジュール";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "クラス階層";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "データ構造";
      } else {
         return "クラス一覧";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "ファイル一覧";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "データフィールド";
      } else {
         return "クラスメンバ";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "大域各種";
      } else {
         return "ファイルメンバ";
      }
   }
   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "諸情報";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "各種例";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "検索";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "クラス階層一覧です。大雑把に文字符号順で並べられています。";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      /* 概要がついているのは見ればわかるので省略 */
      /* extractAll こと EXTRACT_ALL はすべての詳解が存在することを
         実際の有無を度外視してユーザーが保証する設定なので
         詳解がなければこの関数が返す文字列は当然に矛盾を起こす。
      */
      if (extractAll) {
         return "ファイル一覧です。";
      }
      return "詳解が付けられているファイルの一覧です。";
   }


   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {
      /* 概要がついているのは見ればわかるので省略 */
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "データ構造一覧です。";
      } else {
         return "クラス・構造体・共用体・インターフェースの一覧です。";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      const bool forC = Config_getBool("OPTIMIZE_OUTPUT_FOR_C");
      QByteArray result;
      if (forC) {
         result = "構造体・共用体の";
      }
      if (extractAll) {
         result += "全";
      } else {
         result += "詳解あり";
      }
      if (forC) {
         result += "フィールド";
      } else {
         result += "クラスメンバ";
      }
      if (!extractAll && !forC) {
         result += "の";
      }
      result += "一覧です。";
      if (!extractAll) {
         if (forC) {
            result += "各フィールド詳解";
         } else {
            result += "各クラスメンバ詳解";
         }
      } else {
         if (forC) {
            result += "各フィールドが属する構造体・共用体";
         } else {
            result += "各メンバが属するクラス";
         }
      }
      result += "へのリンクがあります。";
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool /*extractAll*/) {
      QByteArray result;
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "関数・変数・マクロ・列挙・型定義";
      } else {
         result += "ファイル直下のメンバ";
      }
      result += "一覧です。各々詳解があればそこへリンクしています。";
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "各種例の一覧です。";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "諸情報の一覧です。";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "全モジュールの一覧です。";
   }

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "詳解";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "モジュール索引";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "階層索引";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "データ構造索引";
      } else {
         return "クラス索引";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "ファイル索引";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "モジュール詳解";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "データ構造詳解";
      } else {
         return "クラス詳解";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "ファイル詳解";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "各例詳解";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "ページ詳解";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "リファレンスマニュアル";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "マクロ定義";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "型定義";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "列挙型";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "関数";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "変数";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "列挙値";
   }
   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "マクロ定義詳解";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "型定義詳解";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "列挙型詳解";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "関数詳解";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "変数詳解";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "データ構造";
      } else {
         return "クラス";
      }
   }
   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)date + "作成";
      if (projName) {
         result += (QByteArray)" - " + projName;
      }
      result += " / 構成: ";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)clName + " の継承関係図";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "内部処理用です。";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "警告";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "バージョン";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "日付";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "戻り値";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "参照";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "引数";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "例外";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "構築:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "名前空間一覧";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      if (extractAll) {
         return "全名前空間の一覧です。";
      }
      return "詳解が付いた名前空間の一覧です。";
   }

   /*! used in the class documentation as a header before the list of all
    *  friends of a class
    */
   virtual QByteArray trFriends() {
      return "フレンド";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   /*! used in the class documentation as a header before the list of all
    * related classes
    */
   virtual QByteArray trRelatedFunctionDocumentation() {
      return "フレンドと関連関数の詳解";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of the HTML page of a class/struct/union */
   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate) {
      QByteArray result = (QByteArray)clName + " ";
      switch (compType) {
         case ClassDef::Class:
            result += "クラス";
            break;
         case ClassDef::Struct:
            result += "構造体";
            break;
         case ClassDef::Union:
            result += "共用体";
            break;
         case ClassDef::Interface:
            result += "インタフェース";
            break;
         case ClassDef::Protocol:
            result += "プロトコル";
            break;
         case ClassDef::Category:
            result += "カテゴリ";
            break;
         case ClassDef::Exception:
            result += "例外";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += "テンプレート";
      }
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = (QByteArray)fileName + " ファイル";
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = (QByteArray)namespaceName + " 名前空間";
      return result;
   }

   /* these are for the member sections of a class, struct or union */
   virtual QByteArray trPublicMembers() {
      return "公開メンバ関数";
   }
   virtual QByteArray trPublicSlots() {
      return "公開スロット";
   }
   virtual QByteArray trSignals() {
      return "シグナル";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "静的公開メンバ関数";
   }
   virtual QByteArray trProtectedMembers() {
      return "限定公開メンバ関数";
   }
   virtual QByteArray trProtectedSlots() {
      return "限定公開スロット";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "静的限定公開メンバ関数";
   }
   virtual QByteArray trPrivateMembers() {
      return "非公開メンバ関数";
   }
   virtual QByteArray trPrivateSlots() {
      return "非公開スロット";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "静的非公開メンバ関数";
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
            result += ", ";
         }
      }
      if ( result.length() > 60 ) {
         QByteArray countStr;
         countStr.sprintf(" (計%d項目)", numEntries);
         result += countStr;
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return trWriteList(numEntries) + "を継承しています。";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return trWriteList(numEntries) + "に継承されています。";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return trWriteList(numEntries) + "を再実装しています。";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return trWriteList(numEntries) + "で再実装されています。";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "名前空間メンバ";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "これは";
      result += "名前空間の一覧です。それぞれ";
      if (extractAll) {
         result += "の名前空間";
      } else {
         result += "が属している名前空間";
      }
      result += "へリンクしています。";
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "名前空間索引";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "名前空間詳解";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "名前空間";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990728
   //////////////////////////////////////////////////////////////////////////

   /*! This is put at the bottom of a class documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFiles(ClassDef::CompoundType compType,
                                           bool /*single*/) {
      // here s is one of " Class", " Struct" or " Union"
      // single is true implies a single file
      QByteArray result = (QByteArray)"この";
      switch (compType) {
         case ClassDef::Class:
            result += "クラス";
            break;
         case ClassDef::Struct:
            result += "構造体";
            break;
         case ClassDef::Union:
            result += "共用体";
            break;
         case ClassDef::Interface:
            result += "インタフェース";
            break;
         case ClassDef::Protocol:
            result += "プロトコル";
            break;
         case ClassDef::Category:
            result += "カテゴリ";
            break;
         case ClassDef::Exception:
            result += "例外";
            break;
         default:
            break;
      }
      result += "詳解は次のファイルから抽出されました:";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "戻り値";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "総合概要";
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
      return " @1 の @0 行目に定義があります。";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return " @0 に定義があります。";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "非推奨";
   }

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)clName + " 連携図";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)fName + " の依存先関係図:";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "構築子と解体子";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "[ソースコード]";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "[詳解]";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "事前条件";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "事後条件";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "不変";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "初期値:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "コード";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "クラス階層図";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "[クラス階層図]";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "[クラス階層表]";
   }
   virtual QByteArray trPageIndex() {
      return "ページ索引";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "覚え書き";
   }
   virtual QByteArray trPublicTypes() {
      return "公開型";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "フィールド";
      } else {
         return "公開変数類";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "静的公開変数類";
   }
   virtual QByteArray trProtectedTypes() {
      return "限定公開型";
   }
   virtual QByteArray trProtectedAttribs() {
      return "限定公開変数類";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "静的限定公開変数類";
   }
   virtual QByteArray trPrivateTypes() {
      return "非公開型";
   }
   virtual QByteArray trPrivateAttribs() {
      return "非公開変数類";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "静的非公開変数類";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a todo item */
   virtual QByteArray trTodo() {
      return "todo";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "todo一覧";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "参照元";
   }
   virtual QByteArray trRemarks() {
      return "注釈";
   }
   virtual QByteArray trAttention() {
      return "注意";
   }
   virtual QByteArray trInclByDepGraph() {
      return "被依存関係図:";
   }
   virtual QByteArray trSince() {
      return "から";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "グラフの凡例";
   }
   /*! page explaining how the dot graph's should be interpreted */
   virtual QByteArray trLegendDocs() {
      return
         "Doxygen が生成したグラフを読み方について。<p>\n"
         "次のコード例をご覧ください。\n"
         "\\code\n"
         "/*! 全体の大きさの関係で見えなくなるクラスです。 */\n"
         "class Invisible { };\n\n"
         "/*! 表示を切り捨てられたクラス(Invisibleクラスの分が見えません) */\n"
         "class Truncated : public Invisible { };\n\n"
         "/* Doxygen 用のコメントコードがないクラス */\n"
         "class Undocumented { };\n\n"
         "/*! 公開継承されているクラス */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*! A template class */\n"
         "template<class T> class Templ { };\n\n"
         "/*! 限定公開で継承されているクラス */\n"
         "class ProtectedBase { };\n\n"
         "/*! 非公開継承されているクラス */\n"
         "class PrivateBase { };\n\n"
         "/*! Inherited クラス内で使われているクラス */\n"
         "class Used { };\n\n"
         "/*! 複数のクラスを継承している上位クラス */\n"
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
         "\\c MAX_DOT_GRAPH_" /* わざわざちょん切っているのは doc/translator.py の検出回避のため */
         "HEIGHT タグに 200 を与えた設定ファイル"
         "を使うと、次のようなグラフとなります。"
         "<p><center><img src=\"graph_legend." +Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center>\n"
         "<p>\n"
         "グラフ内の矩形は構造体やクラスを表しています。色の意味は次の通りです。\n"
         "<ul>\n"
         "<li>中を黒く塗られた四角は、図が注目している起点です。</li>\n"
         "<li>黒枠は詳解があることを示しています。</li>\n"
         "<li>灰色枠で示されたクラス等には詳解がありません。</li>\n"
         "<li>赤枠で示されたものは詳解を持つクラスですが、"
         "指定された大きさに収まらないことから一部の継承・包含関係が"
         "省略されていることを表します。</li>\n"
         "</ul>\n"
         "<p>矢印の意味は次の通りです。</p>\n"
         "<ul>\n"
         "<li>青い矢印は二つのクラス間の公開継承関係を示します。</li>\n"
         "<li>緑の矢印は限定公開の継承関係を示します。</li>\n"
         "<li>赤の矢印は非公開の継承関係を示します。</li>\n"
         "<li>紫の破線矢印は、そのクラスが他のクラスに含まれているか、"
         "利用されていることを示します。また、矢印のラベルは矢の先にあるクラス等を"
         "アクセスしている矢の根本のメンバを表しています。</li>\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "凡例";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "テスト";
   }
   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "テスト一覧";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "プロパティ";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "プロパティ詳解";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "データ構造";
      } else {
         return "クラス";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)name + " パッケージ";
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "パッケージ一覧";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "パッケージ一覧です。";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "パッケージ";
   }

   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "値:";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "バグ";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "バグ一覧";
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
      return "932";
   }


   /*! Used as ansicpg for RTF fcharset
    *  \see trRTFansicp() for a table of possible values.
    */
   virtual QByteArray trRTFCharSet() {
      return "128";
   }

   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "索引";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool /*first_capital*/, bool /*singular*/) {
      return "クラス";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool /*first_capital*/, bool /*singular*/) {
      return "ファイル";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool /*first_capital*/, bool /*singular*/) {
      return "名前空間";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool /*first_capital*/, bool /*singular*/) {
      return "グループ";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool /*first_capital*/, bool /*singular*/) {
      return "ページ";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool /*first_capital*/, bool /*singular*/) {
      return "メンバ";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool /*first_capital*/, bool /*singular*/) {
      return "大域各種";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool /*first_capital*/, bool /*singular*/) {
      return "著者";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "参照先";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return trWriteList(numEntries) + "を実装しています。";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return trWriteList(numEntries) + "で実装されています。";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "目次";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "非推奨一覧";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "イベント";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "イベント詳解";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "パッケージ内の型定義";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "関数";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "静的関数";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "変数";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "静的変数";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "全て";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "呼び出し関係図:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "検索結果";
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
         return "入力条件を満たす文書がありません。";
      } else if (numDocuments == 1) {
         return "入力条件を満たす文書が <b>1</b> 件ありました.";
      } else {
         return "入力条件を満たす文書が <b>$num</b> 件ありました. "
                "一致度の高いものから表示されます.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "照合語:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return filename + " ソースファイル";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "ディレクトリ索引";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "ディレクトリ詳解";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "ディレクトリ";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "このディレクトリ一覧はおおまかにはソートされていますが、"
             "完全にアルファベット順でソートされてはいません。";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " ディレクトリリファレンス";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool, bool) {
      return "ディレクトリ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.1
   //////////////////////////////////////////////////////////////////////////

   /*! This text is added to the documentation when the \\overload command
    *  is used for a overloaded function.
    */
   virtual QByteArray trOverloadText() {
      return "これはオーバーロードされたメンバ関数です。"
             "利便性のために用意されています。"
             "元の関数との違いは引き数のみです。";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "被呼び出し関係図:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "列挙型詳解";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "メンバ関数/サブルーチン詳解";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "データ型一覧";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "データフィールド";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "これはデータ型の一覧です:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "これは";
      result += "フィールドの一覧です。それぞれ";
      if (extractAll) {
         result += "が属しているデータ型";
      }
      result += "の詳解へリンクしています。";
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "データ型索引";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "データ型詳解";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "関数/サブルーチン";
   }


   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "関数/サブルーチン詳解";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "データ型";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "モジュール一覧";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result;
      if (!extractAll) {
         result += "詳解が記されている";
      } else {
         result += "全";
      }
      result += "モジュールの一覧です";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = "";
      switch (compType) {
         case ClassDef::Class:
            result += "モジュール ";
            break;
         case ClassDef::Struct:
            result += "TYPE ";
            break;
         case ClassDef::Union:
            result += "共用体 ";
            break;
         case ClassDef::Interface:
            result += "インターフェース ";
            break;
         case ClassDef::Protocol:
            result += "プロトコル ";
            break;
         case ClassDef::Category:
            result += "カテゴリ ";
            break;
         case ClassDef::Exception:
            result += "例外 ";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += "テンプレート ";
      }
      result += (QByteArray)clName;
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += "モジュール";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "モジュールメンバ";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "これはモジュールメンバ一覧です。それぞれ ";
      if (extractAll) {
         result += "属しているモジュール";
      }
      result += "の詳解へリンクしています。";
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "モジュール索引";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool /*first_capital*/, bool /*singular*/) {
      return "モジュール";
   }
   /*! This is put at the bottom of a module documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFilesFortran(ClassDef::CompoundType compType,
         bool /*single*/) {
      // here s is one of " Module", " Struct" or " Union"
      // single is true implies a single file
      QByteArray result = "次のファイルから";
      switch (compType) {
         case ClassDef::Class:
            result += "モジュール";
            break;
         case ClassDef::Struct:
            result += "型";
            break;
         case ClassDef::Union:
            result += "共用体";
            break;
         case ClassDef::Interface:
            result += "インターフェース";
            break;
         case ClassDef::Protocol:
            result += "プロトコル";
            break;
         case ClassDef::Category:
            result += "カテゴリ";
            break;
         case ClassDef::Exception:
            result += "例外";
            break;
         default:
            break;
      }
      result += "の詳解が抽出されました:";
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool /*first_capital*/, bool /*singular*/) {
      QByteArray result = "型";
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool /*first_capital*/, bool /*singular*/) {
      QByteArray result = "サブプログラム";
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "型制約";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0 (mainly for the new search engine)
   //////////////////////////////////////////////////////////////////////////

   /*! directory relation for \a name */
   virtual QByteArray trDirRelation(const char *name) {
      return QByteArray(name) + " 関係";
   }

   /*! Loading message shown when loading search results */
   virtual QByteArray trLoading() {
      return "読み取り中…";
   }

   /*! Label used for search results in the global namespace */
   virtual QByteArray trGlobalNamespace() {
      return "大域名前空間";
   }

   /*! Message shown while searching */
   virtual QByteArray trSearching() {
      return "検索中…";
   }

   /*! Text shown when no search results are found */
   virtual QByteArray trNoMatches() {
      return "一致する文字列を見つけられません";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3 (missing items for the directory pages)
   //////////////////////////////////////////////////////////////////////////

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the first column mentions the
    *  source file that has a relation to another file.
    */
   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)name + "にあるファイル";
   }

   /*! when clicking a directory dependency label, a page with a
    *  table is shown. The heading for the second column mentions the
    *  destination file that is included.
    */
   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)name + "にあるファイルを include している";
   }

   /** Compiles a date string.
    *  @param year Year in 4 digits
    *  @param month Month of the year: 1=January
    *  @param day Day of the Month: 1..31
    *  @param dayOfWeek Day of the week: 1=Monday..7=Sunday
    *  @param hour Hour of the day: 0..23
    *  @param minutes Minutes in the hour: 0..59
    *  @param seconds Seconds within the minute: 0..59
    *  @param includeTime Include time in the result string?
    */
   virtual QByteArray trDateTime(int year, int month, int day, int dayOfWeek,
                                 int hour, int minutes, int seconds,
                                 bool includeTime) {
      static const char *days[]   = { "月", "火", "水", "木", "金", "土", "日" };
      QByteArray sdate;
      sdate.sprintf("%.4d年%.2d月%.2d日(%s)", year, month, day, days[dayOfWeek - 1]);
      if (includeTime) {
         QByteArray stime;
         stime.sprintf(" %.2d時%.2d分%.2d秒", hour, minutes, seconds);
         sdate += stime;
      }
      return sdate;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.7.5
   //////////////////////////////////////////////////////////////////////////

   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "書誌参照";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "著作権所有";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray(name) + " のディレクトリ依存関係図";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.0
   //////////////////////////////////////////////////////////////////////////

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "表示階層";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "テンプレート引数";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "ほか " + number + " 件…";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool) {
      return "次のファイルからこの列挙についての詳解を抽出しました:";
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return QByteArray("列挙 ") + name + " 詳解";
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray("基底クラス ") + what + " に属する継承" + members;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "その他の継承メンバ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a tooltip for the toggle button that appears in the
    *  navigation tree in the HTML output when GENERATE_TREEVIEW is
    *  enabled. This tooltip explains the meaning of the button.
    */
   virtual QByteArray trPanelSynchronisationTooltip(bool enable) {

      QByteArray opt = enable ? "有効" : "無効";
      return "クリックで同期表示が" + opt + "になります";
   }

   /*! Used in a method of an Objective-C class that is declared in a
    *  a category. Note that the @1 marker is required and is replaced
    *  by a link.
    */
   virtual QByteArray trProvidedByCategory() {
      return "@1 カテゴリーから提供されています。";
   }

   /*! Used in a method of an Objective-C category that extends a class.
    *  Note that the @1 marker is required and is replaced by a link to
    *  the class method.
    */
   virtual QByteArray trExtendsClass() {
      return "@1 を拡張しています。";
   }

   /*! Used as the header of a list of class methods in Objective-C.
    *  These are similar to static public member functions in C++.
    */
   virtual QByteArray trClassMethods() {
      return "クラスメソッド";
   }

   /*! Used as the header of a list of instance methods in Objective-C.
    *  These are similar to public member functions in C++.
    */
   virtual QByteArray trInstanceMethods() {
      return "実体メソッド";
   }

   /*! Used as the header of the member functions of an Objective-C class.
    */
   virtual QByteArray trMethodDocumentation() {
      return "メソッド詳解";
   }

   /*! Used as the title of the design overview picture created for the
    *  VHDL output.
    */
   virtual QByteArray trDesignOverview() {
      return "デザイン概観";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.4
   //////////////////////////////////////////////////////////////////////////

   /** old style UNO IDL services: implemented interfaces */
   virtual QByteArray trInterfaces() {
      return "実装されたインターフォース";
   }

   /** old style UNO IDL services: inherited services */
   virtual QByteArray trServices() {
      return "継承されたサービス";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroups() {
      return "定数グループ";
   }

   /** UNO IDL constant groups */
   virtual QByteArray trConstantGroupReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " 定数グループ詳解";
      return result;
   }
   /** UNO IDL service page title */
   virtual QByteArray trServiceReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " サービス詳解";
      return result;
   }
   /** UNO IDL singleton page title */
   virtual QByteArray trSingletonReference(const char *sName) {
      QByteArray result = (QByteArray)sName;
      result += " Singleton 詳解";
      return result;
   }
   /** UNO IDL service page */
   virtual QByteArray trServiceGeneratedFromFiles(bool /*single*/) {
      // single is true implies a single file
      return "次のファイルからこのサービスについて"
             "の詳解を抽出しました:";
   }
   /** UNO IDL singleton page */
   virtual QByteArray trSingletonGeneratedFromFiles(bool /*single*/) {
      // single is true implies a single file
      return "次のファイルからこの Singleton について"
             "の詳解を抽出しました:";
   }

   //////////////////////////////////////////////////////////////////////////

};

#endif
