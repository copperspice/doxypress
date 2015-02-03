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

#ifndef TRANSLATOR_CN_H
#define TRANSLATOR_CN_H

/*!
   If you want insert a space whenever Chinese meets English characters, set
   CN_SPC to " ", else null.
*/
#define CN_SPC " "

class TranslatorChinese : public TranslatorAdapter_1_8_2
{
 public:
   /*! Used for identification of the language. The identification
    * should not be translated. It should be replaced by the name
    * of the language in English using lower-case characters only
    * (e.g. "czech", "japanese", "russian", etc.). It should be equal to
    * the identification used in the language.cpp.
    */

   virtual QByteArray idLanguage() {
      return "chinese";
   }

   /*! Used to get the LaTeX command(s) for the language support.
    * This method should return string with commands that switch
    * LaTeX to the desired language.  For example
    * <pre>"\\usepackage[german]{babel}\n"
    * </pre>
    * or
    * <pre>"\\usepackage{polski}\n"
    * "\\usepackage[latin2]{inputenc}\n"
    * "\\usepackage[T1]{fontenc}\n"
    * </pre>
    *
    * The English LaTeX does not use such commands.  Because of this
    * the empty string is returned in this implementation.
    */
   virtual QByteArray latexLanguageSupportCommand() {
      return "";
   }

   /*! used in the compound documentation before a list of related functions.
    */
   virtual QByteArray trRelatedFunctions() {
      return "相关函数";
   }

   /*! subscript for the related functions. */
   virtual QByteArray trRelatedSubscript() {
      return "(请注意: 这些不是成员函数.)";
   }

   /*! header that is put before the detailed description of files,
    * classes and namespaces.
    */
   virtual QByteArray trDetailedDescription() {
      return "详细描述";
   }

   /*! header that is put before the list of typedefs. */
   virtual QByteArray trMemberTypedefDocumentation() {
      return "成员类型定义说明";
   }

   /*! header that is put before the list of enumerations. */
   virtual QByteArray trMemberEnumerationDocumentation() {
      return "成员枚举类型说明";
   }

   /*! header that is put before the list of member function. */
   virtual QByteArray trMemberFunctionDocumentation() {
      return "成员函数说明";
   }

   /*! header that is put before the list of member attributes. */
   virtual QByteArray trMemberDataDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "结构体成员变量说明";
      } else {
         return "类成员变量说明";
      }
   }

   /*! this is the text of a link put after brief descriptions. */
   virtual QByteArray trMore() {
      return "更多...";
   }

   /*! put in the class documention */
   virtual QByteArray trListOfAllMembers() {
      return "所有成员列表";
   }

   /*! used as the title of the "list of all members" page of a class */
   virtual QByteArray trMemberList() {
      return "成员列表";
   }

   /*! this is the first part of a sentence that is followed by a class name */
   virtual QByteArray trThisIsTheListOfAllMembers() {
      return "成员的完整列表，这些成员属于"CN_SPC;
   }

   /*! this is the remainder of the sentence after the class name */
   virtual QByteArray trIncludingInheritedMembers() {
      return ",包括所有继承而来的类成员";
   }

   /*! this is put at the author sections at the bottom of man pages.
    *  parameter s is name of the project name.
    */
   virtual QByteArray trGeneratedAutomatically(const char *s) {
      QByteArray result;
      result = "由"CN_SPC"Doyxgen"CN_SPC"通过分析"CN_SPC;
      if (s) {
         result += ((QByteArray)s + CN_SPC"的"CN_SPC);
      }
      result += "源代码自动生成.";
      return result;
   }

   /*! put after an enum name in the list of all members */
   virtual QByteArray trEnumName() {
      return "枚举名称";
   }

   /*! put after an enum value in the list of all members */
   virtual QByteArray trEnumValue() {
      return "枚举值";
   }

   /*! put after an undocumented member in the list of all members */
   virtual QByteArray trDefinedIn() {
      return "定义于"CN_SPC;
   }

   // quick reference sections

   /*! This is put above each page as a link to the list of all groups of
    *  compounds or files (see the \\group command).
    */
   virtual QByteArray trModules() {
      return "模块";
   }

   /*! This is put above each page as a link to the class hierarchy */
   virtual QByteArray trClassHierarchy() {
      return "类继承关系";
   }

   /*! This is put above each page as a link to the list of annotated class */
   virtual QByteArray trCompoundList() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "结构体";
      } else {
         return "类列表";
      }
   }

   /*! This is put above each page as a link to the list of documented files */
   virtual QByteArray trFileList() {
      return "文件列表";
   }

   /*! This is put above each page as a link to all members of compounds. */
   virtual QByteArray trCompoundMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "成员变量";
      } else {
         return "类成员";
      }
   }

   /*! This is put above each page as a link to all member of files. */
   virtual QByteArray trFileMembers() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "全局定义";
      } else {
         return "文件成员";
      }
   }

   /*! This is put above each page as a link to all related pages. */
   virtual QByteArray trRelatedPages() {
      return "相关页面";
   }

   /*! This is put above each page as a link to all examples. */
   virtual QByteArray trExamples() {
      return "示例";
   }

   virtual QByteArray trSearch() {
      return "搜索";
   }

   virtual QByteArray trClassHierarchyDescription() {
      return "此继承关系列表按字典顺序粗略的排序:"CN_SPC;
   }

   virtual QByteArray trFileListDescription(bool extractAll) {
      QByteArray result = "这里列出了所有";
      if (!extractAll) {
         result += "文档化的";
      }
      result += "文件，并附带简要说明:";
      return result;
   }

   virtual QByteArray trCompoundListDescription() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "这里列出了所有结构体，并附带简要说明:";
      } else {
         return "这里列出了所有类、结构、联合以及接口定义等，并附带简要说明:";
      }
   }

   virtual QByteArray trCompoundMembersDescription(bool extractAll) {
      QByteArray result = "这里列出了所有";
      if (!extractAll) {
         result += "文档化的";
      }
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "结构体和联合体的成员变量，并附带";
      } else {
         result += "类成员，并附带";
      }
      //result+=" with links to ";
      if (extractAll) {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "结构体或联合的详细说明:";
         } else {
            result += "类的详细说明:";
         }
      } else {
         if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
            result += "结构或联合所属的文件:";
         } else {
            result += "类所属的文件:";
         }
      }
      return result;
   }

   virtual QByteArray trFileMembersDescription(bool extractAll) {
      QByteArray result = "这里列出了所有";
      if (!extractAll) {
         result += "文档化的";
      }
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         result += "函数,变量,宏,枚举和类型定义等,并附带";
      } else {
         result += "文件成员,并附带";
      }

      if (extractAll) {
         result += "其所属的文件:";
      } else {
         result += "其详细说明:";
      }
      return result;
   }

   virtual QByteArray trExamplesDescription() {
      return "这里列出了所有示例:";
   }

   virtual QByteArray trRelatedPagesDescription() {
      return "这里列出了所有相关页面:";
   }

   virtual QByteArray trModulesDescription() {
      return "这里列出了所有模块:";
   }

   virtual QByteArray trDocumentation() {
      return "文档";
   }

   virtual QByteArray trModuleIndex() {
      return "模块索引";
   }

   virtual QByteArray trHierarchicalIndex() {
      return "继承关系索引";
   }

   virtual QByteArray trCompoundIndex() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "结构体索引";
      } else {
         return "类索引";
      }
   }

   virtual QByteArray trFileIndex() {
      return "文件索引";
   }

   virtual QByteArray trModuleDocumentation() {
      return "模块说明";
   }

   virtual QByteArray trClassDocumentation() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "结构体说明";
      } else {
         return "类说明";
      }
   }

   virtual QByteArray trFileDocumentation() {
      return "文件说明";
   }

   virtual QByteArray trExampleDocumentation() {
      return "示例说明";
   }

   virtual QByteArray trPageDocumentation() {
      return "页面说明";
   }

   virtual QByteArray trReferenceManual() {
      return "参考手册";
   }

   virtual QByteArray trDefines() {
      return "宏定义";
   }

   virtual QByteArray trTypedefs() {
      return "类型定义";
   }

   virtual QByteArray trEnumerations() {
      return "枚举";
   }

   virtual QByteArray trFunctions() {
      return "函数";
   }

   virtual QByteArray trVariables() {
      return "变量";
   }

   virtual QByteArray trEnumerationValues() {
      return "枚举值";
   }


   virtual QByteArray trDefineDocumentation() {
      return "宏定义说明";
   }

   virtual QByteArray trTypedefDocumentation() {
      return "类型定义说明";
   }

   virtual QByteArray trEnumerationTypeDocumentation() {
      return "枚举类型说明";
   }

   virtual QByteArray trFunctionDocumentation() {
      return "函数说明";
   }

   virtual QByteArray trVariableDocumentation() {
      return "变量说明";
   }

   virtual QByteArray trCompounds() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "结构体";
      } else {
         return "类";
      }
   }

   virtual QByteArray trGeneratedAt(const char *date, const char *projName) {
      QByteArray result = (QByteArray)"生成于"CN_SPC + date;
      if (projName) {
         result += (QByteArray)CN_SPC", 为"CN_SPC + projName;
      }
      result += (QByteArray)"使用"CN_SPC;
      return result;
   }

   virtual QByteArray trClassDiagram(const char *clName) {
      return (QByteArray)"类"CN_SPC + clName + CN_SPC"继承关系图:";
   }

   virtual QByteArray trForInternalUseOnly() {
      return "仅限内部使用.";
   }

   virtual QByteArray trWarning() {
      return "警告";
   }

   virtual QByteArray trVersion() {
      return "版本";
   }

   virtual QByteArray trDate() {
      return "日期";
   }

   virtual QByteArray trReturns() {
      return "返回";
   }

   virtual QByteArray trSeeAlso() {
      return "参见";
   }

   virtual QByteArray trParameters() {
      return "参数";
   }

   virtual QByteArray trExceptions() {
      return "异常";
   }

   virtual QByteArray trGeneratedBy() {
      return "制作者";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990307
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNamespaceList() {
      return "命名空间列表";
   }

   virtual QByteArray trNamespaceListDescription(bool extractAll) {
      QByteArray result = "这里列出了所有";
      if (!extractAll) {
         result += "文档化的";
      }
      result += "命名空间定义,附带简要说明:";
      return result;
   }

   virtual QByteArray trFriends() {
      return "友元";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990405
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trRelatedFunctionDocumentation() {
      return "友元及相关函数文档";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990425
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trCompoundReference(const char *clName,
                                          ClassDef::CompoundType compType,
                                          bool isTemplate)
   // used as the title of the HTML page of a class/struct/union
   {
      QByteArray result = (QByteArray)clName;
      if (isTemplate) {
         result += CN_SPC"模板";
      }
      switch (compType) {
         case ClassDef::Class:
            result += "类";
            break;
         case ClassDef::Struct:
            result += "结构体";
            break;
         case ClassDef::Union:
            result += "联合体";
            break;
         case ClassDef::Interface:
            result += "接口";
            break;
         case ClassDef::Protocol:
            result += "协议";
            break;
         case ClassDef::Category:
            result += "分类";
            break;
         case ClassDef::Exception:
            result += "异常";
            break;
         default:
            break;
      }
      result += CN_SPC"参考";
      return result;
   }

   /*! used as the title of the HTML page of a file */
   virtual QByteArray trFileReference(const char *fileName) {
      QByteArray result = fileName;
      result += CN_SPC"文件参考";
      return result;
   }

   /*! used as the title of the HTML page of a namespace */
   virtual QByteArray trNamespaceReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += CN_SPC"命名空间参考";
      return result;
   }

   // these are for the member sections of a class, struct or union
   virtual QByteArray trPublicMembers() {
      return "Public 成员函数";
   }

   virtual QByteArray trPublicSlots() {
      return "Public 槽";
   }

   virtual QByteArray trSignals() {
      return "信号";
   }

   virtual QByteArray trStaticPublicMembers() {
      return "静态 Public 成员函数";
   }

   virtual QByteArray trProtectedMembers() {
      return "Protected 成员函数";
   }

   virtual QByteArray trProtectedSlots() {
      return "Protected 槽";
   }

   virtual QByteArray trStaticProtectedMembers() {
      return "静态 Protected 成员函数";
   }

   virtual QByteArray trPrivateMembers() {
      return "Private 成员函数";
   }

   virtual QByteArray trPrivateSlots() {
      return "Private 槽";
   }

   virtual QByteArray trStaticPrivateMembers() {
      return "静态 Private 成员函数";
   }

   // end of member sections
   virtual QByteArray trWriteList(int numEntries) {
      // this function is used to produce a comma-separated list of items.
      // use generateMarker(i) to indicate where item i should be put.
      QByteArray result;
      int i;
      // the inherits list contain `numEntries' classes
      for (i = 0; i < numEntries; i++) {
         // use generateMarker to generate placeholders for the class links!
         result += generateMarker(i); // generate marker for entry i in the list
         // (order is left to right)

         if (i != numEntries - 1) { // not the last entry, so we need a separator
            if (i < numEntries - 2) { // not the fore last entry
               result += ","CN_SPC;
            } else {            // the fore last entry
               result += CN_SPC", 以及"CN_SPC;
            }
         }
      }
      return result;
   }

   /*! used in class documentation to produce a list of base classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritsList(int numEntries) {
      return "继承自"CN_SPC + trWriteList(numEntries) + CN_SPC".";
   }

   /*! used in class documentation to produce a list of super classes,
    *  if class diagrams are disabled.
    */
   virtual QByteArray trInheritedByList(int numEntries) {
      return "被"CN_SPC + trWriteList(numEntries) + CN_SPC"继承.";
   }

   /*! used in member documentation blocks to produce a list of
    *  members that are hidden by this one.
    */
   virtual QByteArray trReimplementedFromList(int numEntries) {
      return "重载"CN_SPC + trWriteList(numEntries) + CN_SPC".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all member that overwrite the implementation of this member.
    */
   virtual QByteArray trReimplementedInList(int numEntries) {
      return "被"CN_SPC + trWriteList(numEntries) + CN_SPC"重载.";
   }

   /*! This is put above each page as a link to all members of namespaces. */
   virtual QByteArray trNamespaceMembers() {
      return "命名空间成员";
   }

   /*! This is an introduction to the page with all namespace members */
   virtual QByteArray trNamespaceMemberDescription(bool extractAll) {
      QByteArray result = "这里列出了所有";
      if (!extractAll) {
         result += "文档化的";
      }
      result += "命名空间成员，并附带";
      if (extractAll) {
         result += "其说明文档:";
      } else {
         result += "其所属的文件:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all namespaces.
    */
   virtual QByteArray trNamespaceIndex() {
      return "命名空间索引";
   }

   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all namespaces.
    */
   virtual QByteArray trNamespaceDocumentation() {
      return "命名空间文档";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990522
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in the documentation before the list of all
    *  namespaces in a file.
    */
   virtual QByteArray trNamespaces() {
      return "命名空间";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990728
   //////////////////////////////////////////////////////////////////////////

   /*! This is put at the bottom of a class documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFiles(ClassDef::CompoundType compType,
                                           bool) {
      // here s is one of " Class", " Struct" or " Union"
      // single is true implies a single file
      QByteArray result = (QByteArray)"该";
      switch (compType) {
         case ClassDef::Class:
            result += "类";
            break;
         case ClassDef::Struct:
            result += "结构体";
            break;
         case ClassDef::Union:
            result += "联合体";
            break;
         case ClassDef::Interface:
            result += "接口";
            break;
         case ClassDef::Protocol:
            result += "协议";
            break;
         case ClassDef::Category:
            result += "分类";
            break;
         case ClassDef::Exception:
            result += "异常";
            break;
         default:
            break;
      }
      result += "的文档由以下文件生成:";
      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-990901
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the heading text for the retval command. */
   virtual QByteArray trReturnValues() {
      return "返回值";
   }

   /*! This is in the (quick) index as a link to the main page (index.html)
    */
   virtual QByteArray trMainPage() {
      return "首页";
   }

   /*! This is used in references to page that are put in the LaTeX
    *  documentation. It should be an abbreviation of the word page.
    */
   virtual QByteArray trPageAbbreviation() {
      return "p.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991106
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDefinedAtLineInSourceFile() {
      return "在文件"CN_SPC"@1"CN_SPC"第"CN_SPC"@0"CN_SPC"行定义.";
   }

   virtual QByteArray trDefinedInSourceFile() {
      return "在文件"CN_SPC"@0"CN_SPC"中定义.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 0.49-991205
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trDeprecated() {
      return "弃用";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.0.0
   //////////////////////////////////////////////////////////////////////////

   /*! this text is put before a collaboration diagram */
   virtual QByteArray trCollaborationDiagram(const char *clName) {
      return (QByteArray)clName + CN_SPC"的协作图:";
   }

   /*! this text is put before an include dependency graph */
   virtual QByteArray trInclDepGraph(const char *fName) {
      return (QByteArray)fName + CN_SPC"的引用(Include)关系图:";
   }

   /*! header that is put before the list of constructor/destructors. */
   virtual QByteArray trConstructorDocumentation() {
      return "构造及析构函数说明";
   }

   /*! Used in the file documentation to point to the corresponding sources. */
   virtual QByteArray trGotoSourceCode() {
      return "浏览源代码.";
   }

   /*! Used in the file sources to point to the corresponding documentation. */
   virtual QByteArray trGotoDocumentation() {
      return "浏览该文件的文档.";
   }

   /*! Text for the \\pre command */
   virtual QByteArray trPrecondition() {
      return "前置条件";
   }

   /*! Text for the \\post command */
   virtual QByteArray trPostcondition() {
      return "后置条件";
   }

   /*! Text for the \\invariant command */
   virtual QByteArray trInvariant() {
      return "不变性";
   }

   /*! Text shown before a multi-line variable/enum initialization */
   virtual QByteArray trInitialValue() {
      return "初始值:";
   }

   /*! Text used the source code in the file index */
   virtual QByteArray trCode() {
      return "代码";
   }

   virtual QByteArray trGraphicalHierarchy() {
      return "类继承关系图";
   }

   virtual QByteArray trGotoGraphicalHierarchy() {
      return "浏览类继承关系图";
   }

   virtual QByteArray trGotoTextualHierarchy() {
      return "浏览类继承关系表";
   }

   virtual QByteArray trPageIndex() {
      return "页面索引";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.0
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trNote() {
      return "注解";
   }

   virtual QByteArray trPublicTypes() {
      return "Public 类型";
   }

   virtual QByteArray trPublicAttribs() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "成员变量";
      } else {
         return "Public 属性";
      }
   }

   virtual QByteArray trStaticPublicAttribs() {
      return "静态 Public 属性";
   }

   virtual QByteArray trProtectedTypes() {
      return "Protected 类型";
   }

   virtual QByteArray trProtectedAttribs() {
      return "Protected 属性";
   }

   virtual QByteArray trStaticProtectedAttribs() {
      return "静态 Protected 属性";
   }

   virtual QByteArray trPrivateTypes() {
      return "Private 类型";
   }

   virtual QByteArray trPrivateAttribs() {
      return "Private 属性";
   }

   virtual QByteArray trStaticPrivateAttribs() {
      return "静态 Private 属性";
   }


   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a todo item */
   virtual QByteArray trTodo() {
      return "待办事项";
   }

   /*! Used as the header of the todo list */
   virtual QByteArray trTodoList() {
      return "待办事项列表";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.4
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trReferencedBy() {
      return "参考自";
   }

   virtual QByteArray trRemarks() {
      return "备注";
   }

   virtual QByteArray trAttention() {
      return "注意";
   }

   virtual QByteArray trInclByDepGraph() {
      return "此图展示该文件直接或间接的被哪些文件引用了:";
   }

   virtual QByteArray trSince() {
      return "自从";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.1.5
   //////////////////////////////////////////////////////////////////////////

   /*! title of the graph legend page */
   virtual QByteArray trLegendTitle() {
      return "图例";
   }

   /*! page explaining how the dot graph's should be interpreted */
   virtual QByteArray trLegendDocs() {
      return "本页将向您解释如何理解由"CN_SPC"doxygen"CN_SPC"生成的图.<p>\n"
             "考虑如下例子:\n"
             "\\code\n"
             "/*! 由于截断而使 Invisible 不可见 */\n"
             "class Invisible { };\n\n"
             "/*! Truncated 的继承关系将被隐藏 */\n"
             "class Truncated : public Invisible { };\n\n"
             "/* 没有被doxygen文档化的类 */\n"
             "class Undocumented { };\n\n"
             "/*! public 继承关系的类 */\n"
             "class PublicBase : public Truncated { };\n\n"
             "/*! 一个模板类 */\n"
             "template<class T> class Templ { };\n\n"
             "/*! protected 继承关系的类 */\n"
             "class ProtectedBase { };\n\n"
             "/*! private 继承关系的类 */\n"
             "class PrivateBase { };\n\n"
             "/*! 被 Inherited 使用的类 */\n"
             "class Used { };\n\n"
             "/*! 继承自其它若干类的超级类 */\n"
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
             "结果将会生成以下图:"
             "<p><center><img alt=\"\" src=\"graph_legend." + Config_getEnum("DOT_IMAGE_FORMAT") + "\"></center></p>\n"
             "<p>\n"
             "上图中的矩形有如下意义:\n"
             "</p>\n"
             "<ul>\n"
             "<li>%灰色填充的矩形 表示上图是由该结构体或类生成.</li>\n"
             "<li>%黑色边框的矩形 表示已经被文档化的结构体或类.</li>\n"
             "<li>%灰色边框的矩形 表示未被文档化的结构体或类.</li>\n"
             "<li>%红色边框的矩形 表示该结构体或类的关系没有被完全显示."
             "%如果生成的图不能调整到制定的尺寸，有一些关系就会被截断而不显示出来.</li>\n"
             "</ul>\n"
             "<p>\n"
             "箭头有如下意义:\n"
             "</p>\n"
             "<ul>\n"
             "<li>%深蓝色的箭头被用于展示 public 的继承关系.</li>\n"
             "<li>%深绿色的箭头表示 protected 的继承关系.</li>\n"
             "<li>%深红色的箭头说明了是 privated 的继承关系.</li>\n"
             "<li>%紫色虚线箭头用来表示两个类之间的聚合关系. 被箭头指向的类的类型的变量,可以通过箭头旁标明的变量去访问.</li>\n"
             "<li>%黄色虚线箭头表示模板类实例和模板类之间的关系. 箭头旁边标明了模板类实例化的参数.</li>\n"
             "</ul>\n";
   }

   /*! text for the link to the legend page */
   virtual QByteArray trLegend() {
      return "图例";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.0
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a marker that is put before a test item */
   virtual QByteArray trTest() {
      return "测试";
   }

   /*! Used as the header of the test list */
   virtual QByteArray trTestList() {
      return "测试列表";
   }

   //////////////////////////////////////////////////////////////////////////
   //// new since 1.2.2
   ////////////////////////////////////////////////////////////////////////////

   /*! Used as a section header for IDL properties */
   virtual QByteArray trProperties() {
      return "属性";
   }

   /*! Used as a section header for IDL property documentation */
   virtual QByteArray trPropertyDocumentation() {
      return "属性说明";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.4
   //////////////////////////////////////////////////////////////////////////

   /*! Used for Java classes in the summary section of Java packages */
   virtual QByteArray trClasses() {
      if (Config_getBool("OPTIMIZE_OUTPUT_FOR_C")) {
         return "结构体";
      } else {
         return "类";
      }
   }

   /*! Used as the title of a Java package */
   virtual QByteArray trPackage(const char *name) {
      return (QByteArray)"包"CN_SPC + name;
   }

   /*! Title of the package index page */
   virtual QByteArray trPackageList() {
      return "包列表";
   }

   /*! The description of the package index page */
   virtual QByteArray trPackageListDescription() {
      return "这里列出所有的包，附带简要说明(如果有的话):";
   }

   /*! The link name in the Quick links header for each page */
   virtual QByteArray trPackages() {
      return "包";
   }

   /*! Text shown before a multi-line define */
   virtual QByteArray trDefineValue() {
      return "值:";
   }

   ////////////////////////////////////////////////////////////////////////////
   //// new since 1.2.6
   ////////////////////////////////////////////////////////////////////////////
   virtual QByteArray trBug () {
      return "Bug";
   }

   virtual QByteArray trBugList () {
      return "Bug"CN_SPC"列表";
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
      return "936";
   }

   /*! Used as ansicpg for RTF fcharset
    *  \see trRTFansicp() for a table of possible values.
    */
   virtual QByteArray trRTFCharSet() {
      return "134";
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
      /*
       QByteArray result((first_capital ? "Class" : "class"));
      if (!singular)  result+="es";
      return result;
      */
      return "类";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trFile(bool /*first_capital*/, bool /*singular*/) {
      /*
       QByteArray result((first_capital ? "File" : "file"));
      if (!singular)  result+="s";
      return result;
      */
      return "文件";

   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trNamespace(bool /*first_capital*/, bool /*singular*/) {
      /*
       QByteArray result((first_capital ? "Namespace" : "namespace"));
      if (!singular)  result+="s";
      return result;
      */
      return "命名空间";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGroup(bool /*first_capital*/, bool /*singular*/) {
      /*
       QByteArray result((first_capital ? "Group" : "group"));
      if (!singular)  result+="s";
      return result;
      */
      return "组";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trPage(bool /*first_capital*/, bool /*singular*/) {
      /*
       QByteArray result((first_capital ? "Page" : "page"));
      if (!singular)  result+="s";
      return result;
      */
      return "页";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trMember(bool /*first_capital*/, bool /*singular*/) {
      /*
       QByteArray result((first_capital ? "Member" : "member"));
      if (!singular)  result+="s";
      return result;
      */
      return "成员";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trGlobal(bool /*first_capital*/, bool /*singular*/) {
      /*
       QByteArray result((first_capital ? "Global" : "global"));
      if (!singular)  result+="s";
      return result;
      */
      return "全局";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.7
   //////////////////////////////////////////////////////////////////////////

   /*! This text is generated when the \\author command is used and
    *  for the author section in man pages. */
   virtual QByteArray trAuthor(bool /*first_capital*/, bool /*singular*/) {
      /*
       QByteArray result((first_capital ? "Author" : "author"));
      if (!singular)  result+="s";
      return result;
      */
      return "作者";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.11
   //////////////////////////////////////////////////////////////////////////

   /*! This text is put before the list of members referenced by a member
    */
   virtual QByteArray trReferences() {
      return "参考";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.13
   //////////////////////////////////////////////////////////////////////////

   /*! used in member documentation blocks to produce a list of
    *  members that are implemented by this one.
    */
   virtual QByteArray trImplementedFromList(int numEntries) {
      /* return "Implements "+trWriteList(numEntries)+"."; */
      return "实现了"CN_SPC + trWriteList(numEntries) + ".";
   }

   /*! used in member documentation blocks to produce a list of
    *  all members that implement this abstract member.
    */
   virtual QByteArray trImplementedInList(int numEntries) {
      /* return "Implemented in "+trWriteList(numEntries)+"."; */
      return "在"CN_SPC + trWriteList(numEntries) + CN_SPC"内被实现.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.16
   //////////////////////////////////////////////////////////////////////////

   /*! used in RTF documentation as a heading for the Table
    *  of Contents.
    */
   virtual QByteArray trRTFTableOfContents() {
      /* return "Table of Contents"; */
      return "目录";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.17
   //////////////////////////////////////////////////////////////////////////

   /*! Used as the header of the list of item that have been
    *  flagged deprecated
    */
   virtual QByteArray trDeprecatedList() {
      return "弃用列表";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.2.18
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a header for declaration section of the events found in
    * a C# program
    */
   virtual QByteArray trEvents() {
      return "事件";
   }

   /*! Header used for the documentation section of a class' events. */
   virtual QByteArray trEventDocumentation() {
      return "事件说明";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3
   //////////////////////////////////////////////////////////////////////////

   /*! Used as a heading for a list of Java class types with package scope.
    */
   virtual QByteArray trPackageTypes() {
      return "包类型";
   }

   /*! Used as a heading for a list of Java class functions with package
    * scope.
    */
   virtual QByteArray trPackageMembers() {
      return "包函数";
   }

   /*! Used as a heading for a list of static Java class functions with
    *  package scope.
    */
   virtual QByteArray trStaticPackageMembers() {
      return "静态包函数";
   }

   /*! Used as a heading for a list of Java class variables with package
    * scope.
    */
   virtual QByteArray trPackageAttribs() {
      return "包属性";
   }

   /*! Used as a heading for a list of static Java class variables with
    * package scope.
    */
   virtual QByteArray trStaticPackageAttribs() {
      return "静态包属性";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.1
   //////////////////////////////////////////////////////////////////////////

   /*! Used in the quick index of a class/file/namespace member list page
    *  to link to the unfiltered list of all members.
    */
   virtual QByteArray trAll() {
      return "全部";
   }

   /*! Put in front of the call graph for a function. */
   virtual QByteArray trCallGraph() {
      return "函数调用图:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.3
   //////////////////////////////////////////////////////////////////////////

   /*! This string is used as the title for the page listing the search
    *  results.
    */
   virtual QByteArray trSearchResultsTitle() {
      return "搜索结果";
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
         return "抱歉,未找到与您查询相符的文档.";
      } else if (numDocuments == 1) {
         return "找到<b>1</b>篇与您查询相符的文档.";
      } else {
         return "找到<b>$num</b>篇与您查询相符的文档."
                "优先显示最符合的文档.";
      }
   }
   /*! This string is put before the list of matched words, for each search
    *  result. What follows is the list of words that matched the query.
    */
   virtual QByteArray trSearchMatches() {
      return "符合的结果:";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.8
   //////////////////////////////////////////////////////////////////////////

   /*! This is used in HTML as the title of page with source code for file filename
    */
   virtual QByteArray trSourceFile(QByteArray &filename) {
      /* return filename + " Source File"; */
      return filename + CN_SPC"源文件";
   }
   //////////////////////////////////////////////////////////////////////////
   // new since 1.3.9
   //////////////////////////////////////////////////////////////////////////

   /*! This is used as the name of the chapter containing the directory
    *  hierarchy.
    */
   virtual QByteArray trDirIndex() {
      return "目录结构";
   }

   /*! This is used as the name of the chapter containing the documentation
    *  of the directories.
    */
   virtual QByteArray trDirDocumentation() {
      return "目录说明";
   }

   /*! This is used as the title of the directory index and also in the
   *  Quick links of an HTML page, to link to the directory hierarchy.
   */
   virtual QByteArray trDirectories() {
      return "目录";
   }

   /*! This returns a sentences that introduces the directory hierarchy.
    *  and the fact that it is sorted alphabetically per level
    */
   virtual QByteArray trDirDescription() {
      return "此继承关系列表按字典顺序粗略的排序:"CN_SPC;
   }

   /*! This returns the title of a directory page. The name of the
    *  directory is passed via \a dirName.
    */
   virtual QByteArray trDirReference(const char *dirName) {
      QByteArray result = dirName;
      result += CN_SPC"目录参考";
      return result;
   }

   /*! This returns the word directory with or without starting capital
    *  (\a first_capital) and in sigular or plural form (\a singular).
    */
   virtual QByteArray trDir(bool /*first_capital*/, bool /*singular*/) {
      return "目录";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.1
   //////////////////////////////////////////////////////////////////////////

   /*! This text is added to the documentation when the \\overload command
    *  is used for a overloaded function.
    */
   virtual QByteArray trOverloadText() {
      return "这是为便于使用而提供的一个重载成员函数."
             "与上面的函数相比，它接受不同类型的参数.";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.4.6
   //////////////////////////////////////////////////////////////////////////

   /*! This is used to introduce a caller (or called-by) graph */
   virtual QByteArray trCallerGraph() {
      return "这是这个函数的调用关系图:";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for enumeration values
    */
   virtual QByteArray trEnumerationValueDocumentation() {
      return "枚举变量说明";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.5.4 (mainly for Fortran)
   //////////////////////////////////////////////////////////////////////////

   /*! header that is put before the list of member subprograms (Fortran). */
   virtual QByteArray trMemberFunctionDocumentationFortran() {
      return "成员函数/子程序说明";
   }

   /*! This is put above each page as a link to the list of annotated data types (Fortran). */
   virtual QByteArray trCompoundListFortran() {
      return "数据类型列表";
   }

   /*! This is put above each page as a link to all members of compounds (Fortran). */
   virtual QByteArray trCompoundMembersFortran() {
      return "数据项";
   }

   /*! This is an introduction to the annotated compound list (Fortran). */
   virtual QByteArray trCompoundListDescriptionFortran() {
      return "带简要描述的数据类型列表:";
   }

   /*! This is an introduction to the page with all data types (Fortran). */
   virtual QByteArray trCompoundMembersDescriptionFortran(bool extractAll) {
      QByteArray result = "这里列出了所有";
      if (!extractAll) {
         result += "文档化的";
      }
      result += "数据类型成员,并附带";
      //result+=" with links to ";
      if (!extractAll) {
         result += "其说明文档:";
      } else {
         result += "其所属的文件:";
      }
      return result;
   }

   /*! This is used in LaTeX as the title of the chapter with the
    * annotated compound index (Fortran).
    */
   virtual QByteArray trCompoundIndexFortran() {
      return "数据类型索引";
   }


   /*! This is used in LaTeX as the title of the chapter containing
    *  the documentation of all data types (Fortran).
    */
   virtual QByteArray trTypeDocumentation() {
      return "数据类型文档";
   }

   /*! This is used in the documentation of a file as a header before the
    *  list of (global) subprograms (Fortran).
    */
   virtual QByteArray trSubprograms() {
      return "函数/子程序";
   }

   /*! This is used in the documentation of a file/namespace before the list
    *  of documentation blocks for subprograms (Fortran)
    */
   virtual QByteArray trSubprogramDocumentation() {
      return "函数/子程序说明";
   }


   /*! This is used in the documentation of a file/namespace/group before
    *  the list of links to documented compounds (Fortran)
    */
   virtual QByteArray trDataTypes() {
      return "数据类型";
   }

   /*! used as the title of page containing all the index of all modules (Fortran). */
   virtual QByteArray trModulesList() {
      return "模块列表";
   }

   /*! used as an introduction to the modules list (Fortran) */
   virtual QByteArray trModulesListDescription(bool extractAll) {
      QByteArray result = "这里列出了所有";
      if (!extractAll) {
         result += "文档化的";
      }
      result += "模块，并附带简要说明:";
      return result;
   }

   /*! used as the title of the HTML page of a module/type (Fortran) */
   virtual QByteArray trCompoundReferenceFortran(const char *clName,
         ClassDef::CompoundType compType,
         bool isTemplate) {
      QByteArray result = (QByteArray)clName;
      switch (compType) {
         case ClassDef::Class:
            result += CN_SPC"模块";
            break;
         case ClassDef::Struct:
            result += CN_SPC"类型";
            break;
         case ClassDef::Union:
            result += CN_SPC"联合体";
            break;
         case ClassDef::Interface:
            result += CN_SPC"接口";
            break;
         case ClassDef::Protocol:
            result += CN_SPC"协议";
            break;
         case ClassDef::Category:
            result += CN_SPC"目录";
            break;
         case ClassDef::Exception:
            result += CN_SPC"异常";
            break;
         default:
            break;
      }
      if (isTemplate) {
         result += "模板"CN_SPC;
      }
      result += "参考手册";
      return result;
   }

   /*! used as the title of the HTML page of a module (Fortran) */
   virtual QByteArray trModuleReference(const char *namespaceName) {
      QByteArray result = namespaceName;
      result += CN_SPC"模块参考手册";
      return result;
   }

   /*! This is put above each page as a link to all members of modules. (Fortran) */
   virtual QByteArray trModulesMembers() {
      return "模块成员";
   }

   /*! This is an introduction to the page with all modules members (Fortran) */
   virtual QByteArray trModulesMemberDescription(bool extractAll) {
      // QByteArray result="Here is a list of all ";
      // if (!extractAll) result+="documented ";
      // result+="module members with links to ";
      // if (extractAll)
      // {
      //   result+="the module documentation for each member:";
      // }
      // else
      // {
      //   result+="the modules they belong to:";
      // }
      // return result;
      if (!extractAll) {
         return "这里是有文档的模块成员列表，含有到每个成员所在模块的文档的链接:";
      } else {
         return "这里是模块成员列表，含有到成员所属的模块的链接:";
      }
   }

   /*! This is used in LaTeX as the title of the chapter with the
    *  index of all modules (Fortran).
    */
   virtual QByteArray trModulesIndex()
   // { return "Modules Index"; }
   {
      return "模块索引";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trModule(bool, bool) {
      // QByteArray result((first_capital ? "Module" : "module"));
      // if (!singular)  result+="s";
      // return result;
      return "模块";
   }
   /*! This is put at the bottom of a module documentation page and is
    *  followed by a list of files that were used to generate the page.
    */
   virtual QByteArray trGeneratedFromFilesFortran(ClassDef::CompoundType compType,
         bool) {
      QByteArray result = "该";
      switch (compType) {
         case ClassDef::Class:
            result += CN_SPC"模块";
            break;
         case ClassDef::Struct:
            result += CN_SPC"结构体";
            break;
         case ClassDef::Union:
            result += CN_SPC"联合体";
            break;
         case ClassDef::Interface:
            result += CN_SPC"接口";
            break;
         case ClassDef::Protocol:
            result += CN_SPC"协议";
            break;
         case ClassDef::Category:
            result += CN_SPC"目录";
            break;
         case ClassDef::Exception:
            result += CN_SPC"异常";
            break;
         default:
            break;
      }
      result += CN_SPC"的文档由以下文件生成:";
      return result;
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trType(bool, bool) {
      return "类型";
   }

   /*! This is used for translation of the word that will possibly
    *  be followed by a single name or by a list of names
    *  of the category.
    */
   virtual QByteArray trSubprogram(bool, bool) {
      return "子程序";
   }

   /*! C# Type Constraint list */
   virtual QByteArray trTypeConstraints() {
      return "类型限制";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.0
   //////////////////////////////////////////////////////////////////////////
   virtual QByteArray trDirRelation(const char *name) {
      // return QByteArray(name)+" Relation";
      // unsure
      return QByteArray(name) + CN_SPC"关系";
   }

   virtual QByteArray trLoading() {
      return "载入中...";
   }

   virtual QByteArray trGlobalNamespace() {
      return "全局命名空间";
   }

   virtual QByteArray trSearching() {
      return "搜索中...";
   }

   virtual QByteArray trNoMatches() {
      return "未找到";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.6.3
   //////////////////////////////////////////////////////////////////////////

   virtual QByteArray trFileIn(const char *name) {
      return (QByteArray)"文件在" + CN_SPC + name;
   }

   virtual QByteArray trIncludesFileIn(const char *name) {
      return (QByteArray)"在"CN_SPC + name + CN_SPC"中引用";
   }
 
   /*! Header for the page with bibliographic citations */
   virtual QByteArray trCiteReferences() {
      return "参考书目";
   }

   /*! Text for copyright paragraph */
   virtual QByteArray trCopyright() {
      return "版权所有";
   }

   /*! Header for the graph showing the directory dependencies */
   virtual QByteArray trDirDepGraph(const char *name) {
      return QByteArray(name) + CN_SPC"的目录依赖关系图";
   }

   //////////////////////////////////////////////////////////////////////////
   // new since 1.8.0
   //////////////////////////////////////////////////////////////////////////

   /*! Detail level selector shown for hierarchical indices */
   virtual QByteArray trDetailLevel() {
      return "详情级别";
   }

   /*! Section header for list of template parameters */
   virtual QByteArray trTemplateParameters() {
      return "模板参数";
   }

   /*! Used in dot graph when UML_LOOK is enabled and there are many fields */
   virtual QByteArray trAndMore(const QByteArray &number) {
      return "和"CN_SPC + number + CN_SPC"更多...";
   }

   /*! Used file list for a Java enum */
   virtual QByteArray trEnumGeneratedFromFiles(bool /*single*/) {
      return "枚举说明文档从下列文件生成:";
   }

   /*! Header of a Java enum page (Java enums are represented as classes). */
   virtual QByteArray trEnumReference(const char *name) {
      return QByteArray(name) + CN_SPC"枚举类型参考";
   }

   /*! Used for a section containing inherited members */
   virtual QByteArray trInheritedFrom(const char *members, const char *what) {
      return QByteArray(members) + CN_SPC"继承自"CN_SPC + what;
   }

   /*! Header of the sections with inherited members specific for the
    *  base class(es)
    */
   virtual QByteArray trAdditionalInheritedMembers() {
      return "额外继承的成员函数";
   }

   //////////////////////////////////////////////////////////////////////////
};

#endif

