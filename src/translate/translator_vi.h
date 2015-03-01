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

#ifndef TRANSLATOR_VI_H
#define TRANSLATOR_VI_H

class TranslatorVietnamese : public TranslatorAdapter_1_6_0
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
      return "vietnamese";
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
      return "Những hàm liên quan";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(Chú ý những hàm này không phải là hàm thành viên.)";
   }

   /*! header that is put before the detailed description of files, classes and namespaces. */
   virtual QByteArray trDetailedDescription() {
      return "Mô tả chi tiết";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "Thông tin về Member Typedef";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "Thông tin về Member Enumeration";
   }

   /*! header that is put before the list of member functions. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "Thông tin về hàm thành viên";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Thông tin về trường";
      } else {
         return "Thông tin về Member Data";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "Tiếp...";
   }

   /*! put in the class documentation */
   virtual QByteArray trListOfAllMembers() {
      return "Liệt kê tất cả các thành viên";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "Danh sách thành viên";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "Danh sách các thành viên đầy đủ cho ";
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ", cùng với tất cả các thành viên kế thừa.";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result = "Được tạo ra bởi Doxygen";
      if (s) {
         result += (QByteArray)" cho " + s;
      }
      result += " từ mã nguồn.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "tên enum";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "giá trị enum";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "được định nghĩa trong";
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "Các Modules";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "Kiến trúc Class";
   }

   /*! This is put above each page as a link to the list of annotated classes */
   virtual QByteArray trCompoundList() {
      if (Config::getBool("optimize-c")) {
         return "Cấu trúc cơ sở dữ liệu (Data Structures)";
      } else {
         return "Danh mục các Class";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "Danh mục File";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config::getBool("optimize-c")) {
         return "Các trường dữ liệu";
      } else {
         return "Các thành viên của Class";
      }
   }

   /*! This is put above each page as a link to all members of files. */
   virtual QByteArray trFileMembers() {
      if (Config::getBool("optimize-c")) {
         return "Toàn cục";
      } else {
         return "File thành viên";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "Các trang liên quan";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "Các ví dụ";
   }

   /*! This is put above each page as a link to the search engine. */
   virtual QByteArray trSearch() {
      return "Tìm kiếm";
   }

   /*! This is an introduction to the class hierarchy. */
   virtual QByteArray trClassHierarchyDescription() {
      return "Danh sách kế thừa đã được sắp xếp theo ABC, "
             "nhưng chưa đầy đủ:";
   }

   /*! This is an introduction to the list with all files. */
   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "Danh mục đầy đủ tất cả các ";
      if (!extractAll) {
         result += "(đã được biên soạn) ";
      }
      result += "files cùng với các mô tả tóm tắt:";
      return result;
   }

   /*! This is an introduction to the annotated compound list. */
   virtual QByteArray trCompoundListDescription() {

      if (Config::getBool("optimize-c")) {
         return "Đây là cấu trúc cơ sở dữ liệu với mô tả tóm tắt:";
      } else {
         return "Đây là các classes, structs, "
                "unions và interfaces với các mô tả tóm tắt:";
      }
   }

   /*! This is an introduction to the page with all class members. */
   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "Danh mục tất cả các ";
      if (!extractAll) {
         result += "(đã được mô tả) ";
      }
      if (Config::getBool("optimize-c")) {
         result += "struct và union fields";
      } else {
         result += "class members";
      }
      result += " cùng với các các liên kết đến ";
      if (!extractAll) {
         if (Config::getBool("optimize-c")) {
            result += "Thông tin về struct/union cho từng trường:";
         } else {
            result += "Thông tin về class cho từng thành viên:";
         }
      } else {
         if (Config::getBool("optimize-c")) {
            result += "các structures/unions thuộc:";
         } else {
            result += "các lớp thuộc:";
         }
      }
      return result;
   }

   /*! This is an introduction to the page with all file members. */
   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "Danh sách tất cả các ";
      if (!extractAll) {
         result += "(đã được mô tat) ";
      }

      if (Config::getBool("optimize-c")) {
         result += "functions, variables, defines, enums, và typedefs";
      } else {
         result += "các file thành viên";
      }
      result += " cùng với links tới ";
      if (extractAll) {
         result += "các files thuộc:";
      } else {
         result += "tài liệu:";
      }
      return result;
   }

   /*! This is an introduction to the page with the list of all examples */
   virtual QByteArray trExamplesDescription() {
      return "Danh sách tất cả các ví dụ:";
   }

   /*! This is an introduction to the page with the list of related pages */
   virtual QByteArray trRelatedPagesDescription() {
      return "Danh sách tất cả các trang Thông tin có liên quan:";
   }

   /*! This is an introduction to the page with the list of class/file groups */
   virtual QByteArray trModulesDescription() {
      return "Danh sách tất cả các thành viên:";
   }

   // index titles (the project name is prepended for these)

   /*! This is used in HTML as the title of index.html. */
   virtual QByteArray trDocumentation() {
      return "Thông tin";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * index of all groups.
    */
   virtual QByteArray trModuleIndex() {
      return "Chỉ mục (Index) Module";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * class hierarchy.
    */
   virtual QByteArray trHierarchicalIndex() {
      return "Hierarchical Index";
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index.
    */
   virtual QByteArray trCompoundIndex() {
      if (Config::getBool("optimize-c")) {
         return "Index cấu trúc cơ sở dữ liệu";
      } else {
         return "Class Index";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * list of all files.
    */
   virtual QByteArray trFileIndex() {
      return "File Index";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all groups.
    */
   virtual QByteArray trModuleDocumentation() {
      return "Thông tin về các Module";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all classes, structs and unions.
    */
   virtual QByteArray trClassDocumentation() {
      if (Config::getBool("optimize-c")) {
         return "Thông tin về cấu trúc cơ sở dữ liệu";
      } else {
         return "Thông tin về Class";
      }
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all files.
    */
   virtual QByteArray trFileDocumentation() {
      return "Thông tin về File";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all examples.
    */
   virtual QByteArray trExampleDocumentation() {
      return "Thông tin về các ví dụ";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all related pages.
    */
   virtual QByteArray trPageDocumentation() {
      return "Trang Thông tin";
   }

   /*! This is used in LaTeX as the title of the document */
   virtual QByteArray trReferenceManual() {
      return "Thông tin tham chiếu";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of defines
    */
   virtual QByteArray trDefines() {
      return "Định nghĩa";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of typedefs
    */
   virtual QByteArray trTypedefs() {
      return "Typedefs";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of enumerations
    */
   virtual QByteArray trEnumerations() {
      return "Enumerations";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) functions
    */
   virtual QByteArray trFunctions() {
      return "Các hàm";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trVariables() {
      return "Các biến";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) variables
    */
   virtual QByteArray trEnumerationValues() {
      return "Enumerator";
   }

   /*! This is used in the documentation of a file before the list of
    *  documentation blocks for defines
    */
   virtual QByteArray trDefineDocumentation() {
      return "Thông tin về định nghĩa";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for typedefs
    */
   virtual QByteArray trTypedefDocumentation() {
      return "Thông tin về Typedef";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration types
    */
   virtual QByteArray trEnumerationTypeDocumentation() {
      return "Thông tin về Enumeration Type";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for functions
    */
   virtual QByteArray trFunctionDocumentation() {
      return "Thông tin về hàm";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for variables
    */
   virtual QByteArray trVariableDocumentation() {
      return "Thông tin về các biến";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds
    */
   virtual QByteArray trCompounds() {
      if (Config::getBool("optimize-c")) {
         return "Cấu trúc cơ sở dữ liệu";
      } else {
         return "Classes";
      }
   }

   /*! This is used in the standard footer of each page and indicates when
    *  the page was generated
    */
   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"Được biên soạn vào " + date;
      if (projName) {
         result += (QByteArray)" cho mã nguồn dự án " + projName;
      }
      result += (QByteArray)" bởi";
      return result;
   }

   /*! this text is put before a class diagram */
   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"Sơ đồ kế thừa cho " + clName + ":";
   }

   /*! this text is generated when the \\internal command is used. */
   virtual QByteArray trForInternalUseOnly() {
      return "Chỉ cho sử dụng nội bộ.";
   }

   /*! this text is generated when the \\warning command is used. */
   virtual QByteArray trWarning() {
      return "Lưu ý";
   }

   /*! this text is generated when the \\version command is used. */
   virtual QByteArray trVersion() {
      return "Phiên bản";
   }

   /*! this text is generated when the \\date command is used. */
   virtual QByteArray trDate() {
      return "Ngày";
   }

   /*! this text is generated when the \\return command is used. */
   virtual QByteArray trReturns() {
      return "Giá trị trả về";
   }

   /*! this text is generated when the \\sa command is used. */
   virtual QByteArray trSeeAlso() {
      return "Xem thêm";
   }

   /*! this text is generated when the \\param command is used. */
   virtual QByteArray trParameters() {
      return "Các tham số";
   }

   /*! this text is generated when the \\exception command is used. */
   virtual QByteArray trExceptions() {
      return "Exceptions";
   }

   /*! this text is used in the title page of a LaTeX document. */
   virtual QByteArray trGeneratedBy() {
      return "Được biên soạn bởi";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   /*! used as the title of page containing all the index of all namespaces. */
   virtual QByteArray trNamespaceList() {
      return "Danh sách Namespace";
   }

   /*! used as an introduction to the namespace list */
   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "Danh sách tất cả các ";
      if (!extractAll) {
         result += "(đã được biên tập) ";
      }
      result += "namespaces với mô tả tóm tắt:";
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
      return "Thông tin về Friends và các hàm liên quan";
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
            result += " Class";
            break;
         case ClassDef::Struct:
            result += " Struct";
            break;
         case ClassDef::Union:
            result += " Union";
            break;
         case ClassDef::Interface:
            result += " Interface";
            break;
         case ClassDef::Protocol:
            result += " Protocol";
            break;
         case ClassDef::Category:
            result += " Category";
            break;
         case ClassDef::Exception:
            result += " Exception";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += " Template";
      }
      result += " Tham chiếu";
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = fileName;
      result += " File Tham chiếu";
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Namespace Tham chiếu";
      return result;
   }

   virtual QByteArray trPublicMembers() {
      return "Các hàm thành viên Public";
   }
   virtual QByteArray trPublicSlots() {
      return "Public Slots";
   }
   virtual QByteArray trSignals() {
      return "Signals";
   }
   virtual QByteArray trStaticPublicMembers() {
      return "Các hàm Static Public";
   }
   virtual QByteArray trProtectedMembers() {
      return "Các hàm thành viên Protected";
   }
   virtual QByteArray trProtectedSlots() {
      return "Protected Slots";
   }
   virtual QByteArray trStaticProtectedMembers() {
      return "Các hàm thành viên Static Protected";
   }
   virtual QByteArray trPrivateMembers() {
      return "Các hàm thành viên Private";
   }
   virtual QByteArray trPrivateSlots() {
      return "Private Slots";
   }
   virtual QByteArray trStaticPrivateMembers() {
      return "Các hàm thành viên Static Private";
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
               result += ", và ";
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "Kế thừa " + trWriteList(numEntries) + ".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "Được kế thừa bởi " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "Được thực thi lại từ " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "Được thực thi lại trong " + trWriteList(numEntries) + ".";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "Thành viên Namespace";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "Danh sách tất cả các ";
      if (!extractAll) {
         result += "(đã được biên soạn) ";
      }
      result += "các thành viên namespace cùng với link tới ";
      if (extractAll) {
         result += "Thông tin namespace cho từng thành viên:";
      } else {
         result += " namespaces mà phụ thuộc bởi:";
      }
      return result;
   }
   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "Namespace Index";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "Thông tin về Namespace";
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
   virtual QByteArray trGeneratedFromFiles(ClassDef::CompoundType compType, bool) {
      // here s is one of " Class", " Struct" or " Union"
      // single is true implies a single file
      QByteArray result = (QByteArray)"Thông tin cho ";
      switch (compType) {
         case ClassDef::Class:
            result += "class";
            break;
         case ClassDef::Struct:
            result += "struct";
            break;
         case ClassDef::Union:
            result += "union";
            break;
         case ClassDef::Interface:
            result += "interface";
            break;
         case ClassDef::Protocol:
            result += "protocol";
            break;
         case ClassDef::Category:
            result += "category";
            break;
         case ClassDef::Exception:
            result += "exception";
            break;
         default:
            break;
      }
      result += " được biên soạn từ các file sau đây";
      result += ":";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "Các giá trị trả về";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "Tranh chính";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "tr.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991003
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "Định nghĩa tại dòng @0 trong file @1.";
   }
   virtual QByteArray trDefinedInSourceFile() {
      return "Định nghĩa trong file @0.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "Không tán thành";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)"Sơ đồ liên kết cho " + clName + ":";
   }
   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)"Kèm theo graph phụ thuộc cho " + fName + ":";
   }
   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "Thông tin về Constructor và Destructor";
   }
   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "Tới mã nguồn của file này.";
   }
   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "Tới Thông tin của file này.";
   }
   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "Điều kiện trước";
   }
   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "Điều kiện sau";
   }
   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "Bất biến";
   }
   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "Giá trị khởi tạo:";
   }
   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "mã nguồn";
   }
   virtual QByteArray trGraphicalHierarchy() {
      return "Kiến trúc đồ họa của Class";
   }
   virtual QByteArray trGotoGraphicalHierarchy() {
      return "Tới kiến trúc đồ họa của Class";
   }
   virtual QByteArray trGotoTextualHierarchy() {
      return "Tới kiến trúc text của Class";
   }
   virtual QByteArray trPageIndex() {
      return "Chỉ mục trang";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "Ghi chú";
   }
   virtual QByteArray trPublicTypes() {
      return "Public Types";
   }
   virtual QByteArray trPublicAttribs() {
      if (Config::getBool("optimize-c")) {
         return "các trường đữ liệu";
      } else {
         return "Các thuộc tính Public";
      }
   }
   virtual QByteArray trStaticPublicAttribs() {
      return "Các thuộc tính Static Public";
   }
   virtual QByteArray trProtectedTypes() {
      return "Các kiểu Protected";
   }
   virtual QByteArray trProtectedAttribs() {
      return "các thuộc tính Protected";
   }
   virtual QByteArray trStaticProtectedAttribs() {
      return "Các thuộc tính Static Protected";
   }
   virtual QByteArray trPrivateTypes() {
      return "Các kiểu Private";
   }
   virtual QByteArray trPrivateAttribs() {
      return "Các thuộc tính Private";
   }
   virtual QByteArray trStaticPrivateAttribs() {
      return "Các thuộc tính Static Private";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\todo item */
   virtual QByteArray trTodo() {
      return "Mục tiêu";
   }
   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "Danh sách hàng mục cần thực hiện";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "Tham chiếu bởi";
   }
   virtual QByteArray trRemarks() {
      return "Ghi chú";
   }
   virtual QByteArray trAttention() {
      return "Chú ý";
   }
   virtual QByteArray trInclByDepGraph() {
      return "Đồ thị này biểu thị những file nào trực tiếp hoặc"
             "không trực tiếp bao gồm file này:";
   }
   virtual QByteArray trSince() {
      return "Từ";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "Chú giải Graph";
   }
   /*! page explaining how the dot graph's should be interpreted
    *  The %A in the text below are to prevent link to classes called "A".
    */
   virtual QByteArray trLegendDocs() {
      return
         "Trang này giải nghĩa các biểu đồ được biên soạn bởi "
         " doxygen.<p>\n"
         "Hãy xem xét các ví dụ sau:\n"
         "\\code\n"
         "/*! Các lướp không thấy được bởi sự cắt ngắn */\n"
         "Lớp không nhìn thấy class { };\n\n"
         "/*! class bị cắt, quan hệ kế thừa bị ẩn */\n"
         "class bị cắt : bị ẩn toàn cục { };\n\n"
         "/* Class không được mô tả với các chú giải doxygen */\n"
         "class không được mô tả { };\n\n"
         "/*! Class được kế thừa sử dụng các kế thừa public */\n"
         "class PublicBase : public Truncated { };\n\n"
         "/*!  template class */\n"
         "template<class T> class Templ { };\n\n"
         "/*! Class được kế thừa sử dụng kế thừa protected */\n"
         "class ProtectedBase { };\n\n"
         "/*! Class được kế thừa sử dụng kế thừa protected private */\n"
         "class PrivateBase { };\n\n"
         "/*! Class được sử dụng bởi các class kế thừa */\n"
         "class được sử dụng { };\n\n"
         "/*! Super class kế thừa một số các class khác */\n"
         "class được kế thừa : public PublicBase,\n"
         "                  protected ProtectedBase,\n"
         "                  private PrivateBase,\n"
         "                  public Undocumented,\n"
         "                  public Templ<int>\n"
         "{\n"
         "  private:\n"
         "    Used *m_usedClass;\n"
         "};\n"
         "\\endcode\n"
         "Kết quả trong biểu đồ sau đây:"
         "<p><center><img alt=\"\" src=\"graph_legend." + Config::getEnum("dot-image-format").toUtf8() + "\"></center>\n"
         "<p>\n"
         "Các hộp trong biểu đồ trên có ý nghĩa như sau:\n"
         "<ul>\n"
         "<li>%Hộp màu xám biểu thị cấu trúc clas cho "
         "biểu đồ được thể hiện.\n"
         "<li>%Hộp có khung mầu đen biểu thị struct hoặc class được mô tả.\n"
         "<li>%Hộp có khung mầu xám biểu thị struct hoặc class chưa được mô tả.\n"
         "<li>%Hộp có khung mầu đỏ biểu thị struct hoặc class được mổ tả "
         "khi mà không phải tất cả các quan hệ kế thừa/containment được biển diễn.. %Biểu đồ bị "
         "cắt nếu nó không nằm trọn trong các biên được cho trước..\n"
         "</ul>\n"
         "Các mũi tên có ý nghĩa như sau::\n"
         "<ul>\n"
         "<li>%Mũi tên mầu xanh đậm biểu thị các quan hệ kế thừa công khai "
         "giữa 2 class.\n"
         "<li>%Mũi tên màu xanh lá cây đậm biểu thị kế thừa được bảo về (protected).\n"
         "<li>%Mũi tên đỏ đậm biểu thị kế thừa dạng private.\n"
         "<li>%Mũi tên màu hồng đứt quảng biểu thị class được sử dụng "
         "bởi class khác. Mũi tên được gán nhãn với các giá trị "
         "mà các calsss hoặc struct được truy cập tới.\n"
         "<li>%Mũi tên vàng đắt quãng được thị quan hệ giữa template instance và "
         "template class được dẫn xuất từ đó. Mũi tên được gán nhãn với "
         "tham số của template.\n"
         "</ul>\n";
   }
   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "Chú giải";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "Test";
   }
   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "Danh sách Test";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.2
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "Thuộc tính";
   }
   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "Thông tin thuộc tính (Property)";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config::getBool("optimize-c")) {
         return "Cấu trúc dữ liệu";
      } else {
         return "Classes";
      }
   }
   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"Gói " + name;
   }
   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "Danh sách gói";
   }
   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "Danh sách các gói cùng với mô tả tóm tắt (khi có thể có):";
   }
   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "Các gói";
   }
   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "Giá trị:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.5
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a \\bug item */
   virtual QByteArray trBug() {
      return "Lỗi";
   }
   /*! Used as the header of the bug list */
   virtual QByteArray trBugList() {
      return "Danh sách lỗi";
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
      return "1258";
   }


   /*! Used as ansicpg for RTF fcharset
    *  \see trRTFansicp() for a table of possible values.
    */
   virtual QByteArray trRTFCharSet() {
      return "163";
   }

   /*! Used as header RTF general index */
   virtual QByteArray trRTFGeneralIndex() {
      return "Chỉ số";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trClass(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Class" : "class"));
      if (!singular) {
         result += "es";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "File" : "file"));
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
      QByteArray result((first_capital ? "Group" : "group"));
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
      QByteArray result((first_capital ? "Trang" : "trang"));
      if (!singular) {
         result += "";
      }
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Member" : "member"));
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
         result += "s";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Tác giả" : "tác giả"));
      if (!singular) {
         result += "";
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "Tham chiếu";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      return "Thực hiện " + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      return "Được thực hiện trong " + trWriteList(numEntries) + ".";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      return "Mục lục";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "Danh sách Deprecated";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "Sự kiện";
   }
   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "Thông tin về sự kiện";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "Kiểu gói";
   }
   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "Các hàm Package";
   }
   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "Các hàm Static Package";
   }
   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "Các thuộc tính Package";
   }
   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "Các thuộc tính Static Package";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "Tất cả";
   }
   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "Biểu đồ các lời gọi cho hàm này:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "Kết quả tìm kiếm";
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
         return "Không có tài liệu nào thỏa mãn các truy vấn của bạn.";
      } else if (numDocuments == 1) {
         return "Tìm thấy <b>1</b> tài liệu thỏa mã truy vấn của bạn.";
      } else {
         return "Tìm thấy tất cả <b>$num</b> tài liệu thỏa mã truy vấn của bạn. "
                "Hiển thị những thỏa mãn tốt nhất trước.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "Các kết quả thỏa mãn đk:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      return filename + " File nguồn";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "Cấu trúc thư mục";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "Thông tin về thư mục";
   }

   /*! This is used as the title of the directory index and also in the
    *  Quick links of an HTML page, to link to the directory hierarchy.
    */
   virtual QByteArray trDirectories() {
      return "Các thư mục";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "Thư mục đã được sắp xếp theo al-pha-bê, "
             "nhưng chưa đầy đủ:";
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += " Tham chiếu thư mục";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool first_capital, bool) {
      QByteArray result((first_capital ? "Thư mục" : "thư mục"));
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.1
   //////////////////////////////////////////////////////////////////////////

   /*! This text is added to the documentation when the \\overload command
    *  is used for a overloaded function.
    */
   virtual QByteArray trOverloadText() {
      return "Hàm thành viên dạng overloaded, "
             "được chỉ ra cho việc tra cứu dễ dàng. Nó khác với hàm ở trên"
             "chỉ ở chỗ những tham số nào nó chấp nhận.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "Biểu đồ các lời gọi cho hàm này:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "Thông tin Enumerator";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "Thông tin về các hàm và các thủ tục thành viên";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "Danh sách kiêu dữ liệu";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "Trường dữ liệu";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "Kiểu dữ liệu với các mô tả tóm tắt:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "Danh sách tất cả ";
      if (!extractAll) {
         result += "(đã được mô tả) ";
      }
      result += "các kiểu dữ liệu thành viên";
      result += " cùng với liên kết với ";
      if (!extractAll) {
         result += "Thông tin cấu trúc dữ liệu cho từng thành viên";
      } else {
         result += " các kiểu dữ liệu thuộc:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "Chỉ mục kiểu dữ liệu";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "Thông tin về kiểu dữ liệu";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "Functions/Subroutines";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "Thông tin về Function/Subroutine";
   }

   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "Kiểu dữ liệu";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "Danh sách Modules";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "Danh sách tất cả ";
      if (!extractAll) {
         result += "(đã được mô tả) ";
      }
      result += "các module với mô tả tóm tắt:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      switch (compType) {
         case ClassDef::Class:
            result += " Module";
            break;
         case ClassDef::Struct:
            result += " Type";
            break;
         case ClassDef::Union:
            result += " Union";
            break;
         case ClassDef::Interface:
            result += " Interface";
            break;
         case ClassDef::Protocol:
            result += " Protocol";
            break;
         case ClassDef::Category:
            result += " Category";
            break;
         case ClassDef::Exception:
            result += " Exception";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += " Template";
      }
      result += " Tham chiếu";
      return result;
   }
   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += " Tham chiếu Module";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "Thành viên Module";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      QByteArray result = "Danh sách tất cả ";
      if (!extractAll) {
         result += "(đã được mô tả) ";
      }
      result += "các module thành viên cùng với liên kết tới ";
      if (extractAll) {
         result += "Thông tin module cho từng thành viên:";
      } else {
         result += "các module thuộc:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex() {
      return "Chỉ mục các Module";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Module" : "module"));
      if (!singular) {
         result += "";
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
      QByteArray result = (QByteArray)"Thông tin cho ";
      switch (compType) {
         case ClassDef::Class:
            result += "module";
            break;
         case ClassDef::Struct:
            result += "type";
            break;
         case ClassDef::Union:
            result += "union";
            break;
         case ClassDef::Interface:
            result += "interface";
            break;
         case ClassDef::Protocol:
            result += "protocol";
            break;
         case ClassDef::Category:
            result += "category";
            break;
         case ClassDef::Exception:
            result += "exception";
            break;
         default:
            break;
      }
      result += " được biên soạn từ các file sau đây";
      if (single) {
         result += ":";
      } else {
         result += "s:";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Kiểu" : "kiểu"));
      if (!singular) {
         result += "";
      }
      return result;
   }
   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool first_capital, bool singular) {
      QByteArray result((first_capital ? "Chương trình con" : "chương trình con"));
      if (!singular) {
         result += "";
      }
      return result;
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "Ràng buộc của kiểu (Type)";
   }

};

#endif
