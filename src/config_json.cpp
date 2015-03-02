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

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <config.h>
#include <message.h>

QByteArray Config::json_ReadFile(const QString &fName)
{        
   QByteArray data;

   QFile file(fName);

   if (! file.open(QFile::ReadWrite | QFile::Text)) {      
      return data;
   }

   file.seek(0);
   data = file.readAll();
   file.close();

   return data;
}

bool Config::parseConfig(const QString &fName)
{ 
   printf("**  Read Project Configuration\n"); 

   load_Defaults();

   if (read_ProjectFile(fName)) {
      return false;
   }

   printf("**  Verify Project Configuration\n"); 

   if (preVerify()) {
      return false;
   }

   // not used at this time (broom)
   // Config::instance()->substituteEnvironmentVars();         

   initWarningFormat();   
   
   if (verify()) {
      return false;
   }

   return true;
}

void Config::load_Defaults()
{  
   // tab 1
   m_cfgString.insert("project-name",            struc_CfgString { "My Project",   DEFAULT } );                                           
   m_cfgString.insert("project-brief",           struc_CfgString { QString(),      DEFAULT } );
   m_cfgString.insert("project-version",         struc_CfgString { QString(),      DEFAULT } );  
   m_cfgString.insert("project-logo",            struc_CfgString { QString(),      DEFAULT } );
   m_cfgString.insert("output-dir",              struc_CfgString { QString(),      DEFAULT } );   

   m_cfgBool.insert("optimize-cplus",            struc_CfgBool   { true,           DEFAULT } );  
   m_cfgBool.insert("optimize-java",             struc_CfgBool   { false,          DEFAULT } );  
   m_cfgBool.insert("optimize-c",                struc_CfgBool   { false,          DEFAULT } );  
   m_cfgBool.insert("optimize-fortran",          struc_CfgBool   { false,          DEFAULT } );  
   
   // tab 2 - project
   m_cfgString.insert("project-encoding",        struc_CfgString { "UTF-8",        DEFAULT } );       
   m_cfgBool.insert("create-subdirs",            struc_CfgBool   { false,          DEFAULT } );  
   m_cfgBool.insert("allow-unicode-names",       struc_CfgBool   { false,          DEFAULT } );    
   m_cfgEnum.insert("output-language",           struc_CfgEnum   { "ENGLISH",      DEFAULT } );



   // *** (start) everything in this block is FALSE - get init values

   m_cfgBool.insert("brief-member-desc",         struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("repeat-brief",              struc_CfgBool   { false,          DEFAULT } );
   m_cfgList.insert("abbreviate-brief",          struc_CfgList   { QStringList(),  DEFAULT } );  
   m_cfgBool.insert("always-detailed-sec",       struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("inline-inherited-member",   struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("full-path-names",           struc_CfgBool   { false,          DEFAULT } );
   m_cfgList.insert("strip-from-path",           struc_CfgList   { QStringList(),  DEFAULT } );
   m_cfgList.insert("strip-from-inc-path",       struc_CfgList   { QStringList(),  DEFAULT } );

   m_cfgBool.insert("short-names",               struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("javadoc-auto-brief",        struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("qt-auto-brief",             struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("multiline-cpp-brief",       struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("inherit-docs",              struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("separate-member-pages",     struc_CfgBool   { false,          DEFAULT } );

   m_cfgInt.insert("tab-size",                   struc_CfgInt    { 0,              DEFAULT } );
   m_cfgString.insert("aliases",                 struc_CfgString { QString(),      DEFAULT } ); 
   m_cfgString.insert("tcl-subst",               struc_CfgString { QString(),      DEFAULT } );
   m_cfgString.insert("extension-mapping",       struc_CfgString { QString(),      DEFAULT } );
   m_cfgBool.insert("markdown",                  struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("auto-link",                 struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("built-in-stl-support",      struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("cpp-cli-support",           struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("sip-support",               struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("idl-support",               struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("dist-group-doc",            struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("allow-sub-grouping",        struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("inline-grouped-classes",    struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("inline-simple-struct",      struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("typedef-hides-struct",      struc_CfgBool   { false,          DEFAULT } );
   m_cfgInt.insert("lookup-cache-size",          struc_CfgInt    { 0,              DEFAULT } );

   // tab 2 - build
   m_cfgBool.insert("extract-all",               struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("extract-private",           struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("extract-package",           struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("extract-static",            struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("extract-local-classes",     struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("extract-local-methods",     struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("extract-anon-namespaces",   struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("hide-undoc-members",        struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("hide-undoc-classes",        struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("hide-friend-compounds",     struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("hide-in-body-docs",         struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("hide-scope-names",          struc_CfgBool   { false,          DEFAULT } );

   m_cfgBool.insert("internal-docs",             struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("case-sense-names",          struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("show-include-files",        struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("show-grouped-members-inc",  struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("force-local-includes",      struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("inline-info",               struc_CfgBool   { false,          DEFAULT } );

   m_cfgBool.insert("sort-member-docs",          struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("sort-brief-docs",           struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("sort-members-ctors-first",  struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("sort-group-names",          struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("sort-by-scope-name",        struc_CfgBool   { false,          DEFAULT } );

   m_cfgBool.insert("strict-proto-matching",     struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("generate-todo-list",        struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("generate-test-list",        struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("generate-bug-list",         struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("generate-deprecate-list",   struc_CfgBool   { false,          DEFAULT } );

   m_cfgList.insert("enabled-sections",          struc_CfgList   { QStringList(),  DEFAULT } ); 
   m_cfgInt.insert("max-init-lines",             struc_CfgInt    { 0,              DEFAULT } );
   m_cfgBool.insert("show-used-files",           struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("show-files",                struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("show-namespaces",           struc_CfgBool   { false,          DEFAULT } );
   m_cfgString.insert("file-version-filter",     struc_CfgString { QString(),      DEFAULT } );
   m_cfgString.insert("layout-file",             struc_CfgString { QString(),      DEFAULT } );
   m_cfgList.insert("cite-bib-files",            struc_CfgList   { QStringList(),  DEFAULT } ); 

   // tab 2 - messages
   m_cfgBool.insert("quiet",                     struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("warnings",                  struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("warn-undoc",                struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("warn-doc-error",            struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("warn-undoc-parm",           struc_CfgBool   { false,          DEFAULT } );
   m_cfgString.insert("warn-format",             struc_CfgString { QString(),      DEFAULT } );
   m_cfgString.insert("warn-logfile",            struc_CfgString { QString(),      DEFAULT } );

   // tab 2 -input
   m_cfgList.insert("input-source",              struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgString.insert("input-encoding",          struc_CfgString { QString(),       DEFAULT } );
   m_cfgList.insert("file-patterns",             struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgBool.insert("source-recursive",          struc_CfgBool   { false,           DEFAULT } );

   m_cfgList.insert("exclude-files",             struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgBool.insert("exclude-symlinks",          struc_CfgBool   { false,           DEFAULT } );
   m_cfgList.insert("exclude-patterns",          struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgList.insert("exclude-symbols",           struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgList.insert("example-source",            struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgList.insert("example-pattens",           struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgBool.insert("example-recursive",         struc_CfgBool   { false,           DEFAULT } );

   m_cfgList.insert("image-path",                struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgString.insert("input-filter",            struc_CfgString { QString(),       DEFAULT } );
   m_cfgList.insert("filter-patterns",           struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgBool.insert("filter-source-files",       struc_CfgBool   { false,           DEFAULT } );
   m_cfgList.insert("filter-source-patterns",    struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgString.insert("mdfile-mainpage",         struc_CfgString { QString(),       DEFAULT } );

   // tab 2 -browser
   m_cfgBool.insert("source-browser",            struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("inline-sources",            struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("strip-code-comments",       struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("ref-by-relation",           struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("ref-relation",              struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("ref-link-source",           struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("source-tooltips",           struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("use-htags",                 struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("verbatim-headers",          struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("clang-parsing",             struc_CfgBool   { false,          DEFAULT } );
   m_cfgList.insert("clang-options",             struc_CfgList   { QStringList(),  DEFAULT } );

   // tab 2 -index
   m_cfgBool.insert("alpha-index",               struc_CfgBool   { false,          DEFAULT } );
   m_cfgInt.insert("cols-in-index",              struc_CfgInt    { 0,              DEFAULT } );
   m_cfgList.insert("ignore-prefix",             struc_CfgList   { QStringList(),  DEFAULT } );

   // tab 2 - autogen
   m_cfgBool.insert("generate-autogen-def",      struc_CfgBool   { false,          DEFAULT } );

   // tab 2 - perlmod         
   m_cfgBool.insert("generate-perl",             struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("perl-latex",                struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("perl-pretty",               struc_CfgBool   { false,          DEFAULT } );
   m_cfgString.insert("perlmod-prefix",          struc_CfgString { QString(),      DEFAULT } );



/*

   // tab 2 - preprocess
   m_ui->enable_preprocessing_CB->setChecked(      object.value("enable-preprocessing",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->macro_expansion_CB->setChecked(           object.value("macro-expansion",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->expand_only_predefined_CB->setChecked(    object.value("expand-only-predefined",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->search_includes_CB->setChecked(           object.value("search-includes",      stru_CfgBool   { false,          DEFAULT } );

   m_ui->include_path->setPlainText(               getDataList(object,"include-path"));
   m_ui->include_file_patterns->setPlainText(      getDataList(object,"include-file-patterns"));
   m_ui->predefined_macros->setPlainText(          getDataList(object,"predefined-macros"));
   m_ui->expand_as_defined->setPlainText(          getDataList(object,"expand-as-defined"));
   m_ui->skip_function_macros_CB->setChecked(      object.value("skip-function-macros",      stru_CfgBool   { false,          DEFAULT } );

   // tab 2 - external
   m_ui->tag_files->setPlainText(                  getDataList(object,"tag-files"));
   m_ui->gen_tagfile->setText(                     object.value("generate-tagfile",             struc_CfgString { QString(),      DEFAULT } );
   m_ui->all_externals_CB->setChecked(             object.value("all-externals",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->external_groups_CB->setChecked(           object.value("external-groups",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->external_pages_CB->setChecked(            object.value("external-pages",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->perl_path->setText(                       object.value("perl-path").toString());

   // tab 2 - dot
   m_ui->class_diagrams_CB->setChecked(            object.value("class-diagrams",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->mscgen_path->setText(                     object.value("mscgen-path",             struc_CfgString { QString(),      DEFAULT } );
   m_ui->dia_path->setText(                        object.value("dia-path",             struc_CfgString { QString(),      DEFAULT } );
   m_ui->hide_undoc_relations_CB->setChecked(      object.value("hide-undoc-relations",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->have_dot_CB->setChecked(                  object.value("have-dot",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->dot_num_threads_SB->setValue(             object.value("dot-num-threads",          struc_CfgInt    { 0,              DEFAULT } );
   m_ui->dot_font_name->setText(                   object.value("dot-font-name",             struc_CfgString { QString(),      DEFAULT } );
   m_ui->dot_font_size_SB->setValue(               object.value("dot-font-size",          struc_CfgInt    { 0,              DEFAULT } );
   m_ui->dot_font_path->setText(                   object.value("dot-font-path",             struc_CfgString { QString(),      DEFAULT } );

   m_ui->group_graphs_CB->setChecked(              object.value("group-graphs",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->uml_look_CB->setChecked(                  object.value("uml-look",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->uml_limit_num_fields_SB->setValue(        object.value("uml-limit-num-fields",          struc_CfgInt    { 0,              DEFAULT } );
   m_ui->template_relations_CB->setChecked(        object.value("template-relations",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->directory_graph_CB->setChecked(           object.value("directory-graph",      struc_CfgBool   { false,          DEFAULT } );

   index = m_ui->dot_image_format_CM->findText(    object.value("dot-image-format",             struc_CfgString { QString(),      DEFAULT } );
   m_ui->dot_image_format_CM->setCurrentIndex(index);

   m_ui->interactive_svg_CB->setChecked(           object.value("interactive_svg",      struc_CfgBool   { false,          DEFAULT } );
   m_ui->dot_path->setText(                        object.value("dot-path").toString());
   m_ui->dot_file_dirs->setPlainText(              getDataList(object, "dot-file_dirs"));
   m_ui->msc_file_dirs->setPlainText(              getDataList(object, "msc-file_dirs"));
   m_ui->dia_file_dirs->setPlainText(              getDataList(object, "dia-file_dirs"));
   m_ui->plantuml_jar_path->setText(               object.value("plantuml-jar-path",             struc_CfgString { QString(),      DEFAULT } );

   m_ui->dot_graph_max_nodes_SB->setValue(         object.value("dot-graph_max-nodes",          struc_CfgInt    { 0,              DEFAULT } );
   m_ui->dot_graph_max_depth_SB->setValue(         object.value("dot-graph-max-depth",          struc_CfgInt    { 0,              DEFAULT } );
   m_ui->dot_transparent_CB->setChecked(           object.value("dot-transparent",      struc_CfgBool   { false,          DEFAULT } );
   m_ui->dot_multiple_targets_CB->setChecked(      object.value("dot-multiple-targets",      struc_CfgBool   { false,          DEFAULT } );
   m_ui->gen_legend_CB->setChecked(                object.value("generate-legend",      struc_CfgBool   { false,          DEFAULT } );
   m_ui->dot_cleanup_CB->setChecked(               object.value("dot-cleanup",      struc_CfgBool   { false,          DEFAULT } );
*/

   // ** end block - values ABOVE are not set 


 
   // tab 3 ( appear on tab 1 and tab 3 )
   m_cfgBool.insert("generate-html",         struc_CfgBool { true,              DEFAULT } );  
   m_cfgBool.insert("generate-latex",        struc_CfgBool { false,             DEFAULT } );  
   m_cfgBool.insert("latex-hyper-pdf",       struc_CfgBool { true,              DEFAULT } );  
   m_cfgBool.insert("latex-pdf",             struc_CfgBool { true,              DEFAULT } );  
   m_cfgBool.insert("latex-ps",              struc_CfgBool { false,             DEFAULT } );  
   m_cfgBool.insert("generate-rtf",          struc_CfgBool { false,             DEFAULT } );   
   m_cfgBool.insert("generate-man",          struc_CfgBool { false,             DEFAULT } );   
   m_cfgBool.insert("generate-xml",          struc_CfgBool { false,             DEFAULT } );   
   m_cfgBool.insert("generate-docbook",      struc_CfgBool { false,             DEFAULT } );  
  
   // tab 3 - html
  

/*

   // *** (start) everything in this block is FALSE - get init values


   m_ui->html_output->setText(                     object.value("html-output").toString());
   m_ui->html_file_extension->setText(             object.value("html-file-extension").toString());
   m_ui->html_header->setText(                     object.value("html-header").toString());
   m_ui->html_footer->setText(                     object.value("html-footer").toString());   
   m_ui->html_extra_stylesheets->setPlainText(     getDataList(object, "html-extra-stylesheets"));
   m_ui->html_extra_files->setPlainText(           getDataList(object, "html-extra-files"));

   m_ui->html_colorstyle_hue->setValue(            object.value("html-colorstyle-hue",          struc_CfgInt    { 0,              DEFAULT } );
   m_ui->html_colorstyle_sat->setValue(            object.value("html-colorstyle-sat",          struc_CfgInt    { 0,              DEFAULT } );
   m_ui->html_colorstyle_gamma->setValue(          object.value("html-colorstyle-gamma",          struc_CfgInt    { 0,              DEFAULT } );

   m_ui->html_timestamp_CB->setChecked(            object.value("html-timestamp",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->html_dynamic_sections_CB->setChecked(     object.value("html-dynamic-sections",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->html_index_num_entries_SB->setValue(      object.value("html-index-num-entries",          struc_CfgInt    { 0,              DEFAULT } );

   m_ui->disable_index_CB->setChecked(             object.value("disable-index",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->gen_treeview_CB->setChecked(              object.value("generate-treeview",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->enum_values_per_line_SB->setValue(        object.value("enum-values-per-line",          struc_CfgInt    { 0,              DEFAULT } );
   m_ui->treeview_width_SB->setValue(              object.value("treeview-width",          struc_CfgInt    { 0,              DEFAULT } );
   m_ui->external_links_in_window_CB->setChecked(  object.value("external-links-in-window",      stru_CfgBool   { false,          DEFAULT } );

   m_ui->server_based_search_CB->setChecked(       object.value("server-based-search",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->external_search_CB->setChecked(           object.value("external-search",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->search_engine_url->setText(               object.value("search-engine-url").toString());
   m_ui->search_data_file->setText(                object.value("search-data-file").toString());
   m_ui->search_external_id->setText(              object.value("search-external-id").toString());
   m_ui->search_mappings->setPlainText(            getDataList(object, "search-mappings"));

   m_ui->formula_fontsize_SB->setValue(            object.value("formula-fontsize",          struc_CfgInt    { 0,              DEFAULT } );
   m_ui->formula_transparent_CB->setChecked(       object.value("formula-transparent",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->use_mathjax_CB->setChecked(               object.value("use_mathjax",      stru_CfgBool   { false,          DEFAULT } );

   index = m_ui->mathjax_format_CM->findText(      object.value("mathjax-format").toString());
   m_ui->mathjax_format_CM->setCurrentIndex(index);

   m_ui->mathjax_relpath->setText(                 object.value("mathjax-relpath").toString());
   m_ui->mathjax_extensions->setPlainText(         getDataList(object, "mathjax-extensions"));
   m_ui->mathjax_codefile->setText(                object.value("mathjax-codefile").toString());

   // tab 3 - chm
   m_ui->gen_chm_CB->setChecked(                   object.value("generate-chm",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->chm_file->setText(                        object.value("chm-file").toString());
   m_ui->hhc_location->setText(                    object.value("hhc-location").toString());
   m_ui->gen_chi_CB->setChecked(                   object.value("generate-chi",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->chm_index_encoding->setText(              object.value("chm-index-encoding").toString());
   m_ui->binary_toc_CB->setChecked(                object.value("binary-toc",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->toc_expanded_CB->setChecked(              object.value("toc-expanded",      stru_CfgBool   { false,          DEFAULT } );

   // tab 3 - docset
   m_ui->gen_docset_CB->setChecked(                object.value("generate-docset",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->docset_feedname->setText(                 object.value("docset-feedname").toString());
   m_ui->docset_bundle_id->setText(                object.value("docset-bundle-id").toString());
   m_ui->docset_publisher_id->setText(             object.value("docset-publisher-id").toString());
   m_ui->docset_publisher_name->setText(           object.value("docset-publisher-name").toString());

   // tab 3 - eclipse
   m_ui->gen_eclipse_CB->setChecked(               object.value("generate-eclipse",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->eclipse_doc_id->setText(                  object.value("eclipse-doc-id").toString());

   // tab 3 - latex
   m_ui->latex_output->setText(                    object.value("latex-output").toString());
   m_ui->latex_cmd_name->setText(                  object.value("latex-cmd-name").toString());
   m_ui->make_index_cmd_name->setText(             object.value("make-index-cmd-name").toString());
   m_ui->latex_compact_CB->setChecked(             object.value("latex-compact",      stru_CfgBool   { false,          DEFAULT } );

   index = m_ui->latex_paper_type_CM->findText(    object.value("latex-paper-type").toString());
   m_ui->latex_paper_type_CM->setCurrentIndex(index);

   m_ui->latex_extra_packages->setPlainText(       getDataList(object, "latex-extra-packages"));
   m_ui->latex_header->setText(                    object.value("latex-header").toString());
   m_ui->latex_footer->setText(                    object.value("latex-footer").toString());
   m_ui->latex_extra_files->setPlainText(          getDataList(object, "latex-extra-files"));

   m_ui->latex_hyper_pdf_CB->setChecked(           object.value("latex-hyper-pdf",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->latex_pdf_CB->setChecked(                 object.value("latex-pdf",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->latex_batch_mode_CB->setChecked(          object.value("latex-batch-mode",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->latex_hide_indices_CB->setChecked(        object.value("latex-hide-indices",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->latex_source_code_CB->setChecked(         object.value("latex-source-code",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->latex_bib_style->setText(                 object.value("latex-bib-style").toString());

   // tab 3 - man
   m_ui->man_output->setText(                      object.value("man-output").toString());
   m_ui->man_extension->setText(                   object.value("man-extension").toString());
   m_ui->man_subdir->setText(                      object.value("man-subdir").toString());
   m_ui->man_links_CB->setChecked(                 object.value("man-links",      stru_CfgBool   { false,          DEFAULT } );

   // tab 3 - qt help
   m_ui->gen_qthelp_CB->setChecked(                object.value("generate-qthelp",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->qch_file->setText(                        object.value("qch-file").toString());
   m_ui->qhp_namespace->setText(                   object.value("qhp-namespace").toString());
   m_ui->qhp_virtual_folder->setText(              object.value("qhp-virtual-folder").toString());
   m_ui->qhp_cust_filter_name->setText(            object.value("qhp-cust-filter-name").toString());
   m_ui->qhp_cust_filter_attrib->setText(          object.value("qhp-cust-attrib").toString());
   m_ui->qhp_sect_filter_attrib->setText(          object.value("qhp-sect-attrib").toString());
   m_ui->qthelp_gen_path->setText(                 object.value("qthelp-gen-path").toString());

   // tab 3 - rtf
   m_ui->rtf_output->setText(                      object.value("rtf-output").toString());
   m_ui->rtf_compact_CB->setChecked(               object.value("rtf-compact",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->rtf_hyperlinks_CB->setChecked(            object.value("rtf-hyperlinks",      stru_CfgBool   { false,          DEFAULT } );
   m_ui->rtf_stylesheet->setText(                  object.value("rtf-stylesheet").toString());
   m_ui->rtf_extension->setText(                   object.value("rtf-extension").toString());

   // tab 3 - xml
   m_ui->xml_output->setText(                      object.value("xml-output").toString());
   m_ui->xml_program_listing_CB->setChecked(       object.value("xml-program-listing",      stru_CfgBool   { false,          DEFAULT } );

   // tab 3 - docbook     
   m_ui->docbook_output->setText(                  object.value("docbook-output").toString());
   m_ui->docbook_program_listing_CB->setChecked(   object.value("docbook-program-listing",      stru_CfgBool   { false,          DEFAULT } );

*/

}


bool Config::read_ProjectFile(const QString &fName)
{  
   // get existing json data
   QByteArray data = Config::json_ReadFile(fName);

   if (data.isEmpty()) {
      fprintf(stderr, "%s could not be read or contained no data\n", qPrintable(fName) );
      return false;
   }
   
   QJsonDocument doc  = QJsonDocument::fromJson(data);
   QJsonObject object = doc.object();

   int index;
   bool isError = false;
   
   for (auto iter = object.begin(); iter != object.end(); ++iter) {
      // walk the project file
      QString key = iter.key();
     
      if (key == "output-language" || key == "dot-image-format" || key == "mathjax-format" || key == "latex-paper-type")  { 

         auto hashIter = m_cfgEnum.find(key);

         if (hashIter != m_cfgEnum.end()) {
            hashIter.value() = { iter.value().toString(), PROJECT };            

         }  else {
            fprintf(stderr, "Error: %s was not found in the project enum table\n", qPrintable(key) );
            isError = true;

         } 
      }

      if (iter.value().isBool()) {
         auto hashIter = m_cfgBool.find(key);

         if (hashIter != m_cfgBool.end()) {
            hashIter.value() = { iter.value().toBool(), PROJECT };            

         }  else {
            fprintf(stderr, "Error: %s was not found in the project bool table\n", qPrintable(key) );
            isError = true;

         }
      } 

      if (iter.value().isDouble()) {
         auto hashIter = m_cfgInt.find(key);

         if (hashIter != m_cfgInt.end()) {
            hashIter.value() = { iter.value().toInt(), PROJECT };            

         }  else {
            fprintf(stderr, "Error: %s was not found in the project integer table\n", qPrintable(key) );
            isError = true;

         }
      }

      if (iter.value().isArray()) {
         auto hashIter = m_cfgList.find(key);

          if (hashIter != m_cfgList.end()) {

            QJsonArray temp = iter.value().toArray();
            QStringList listData;

            for (auto item : temp) {
               listData.append(item.toString()); 
            }   

            hashIter.value() = { listData, PROJECT };

         }  else {
            fprintf(stderr, "Error: %s was not found in the project array table\n", qPrintable(key) );
            isError = true;

         }
      } 

      if (iter.value().isString()) {
         auto hashIter = m_cfgString.find(key);

          if (hashIter != m_cfgString.end()) {
            hashIter.value() = { iter.value().toString(), PROJECT };            

         }  else {
            fprintf(stderr, "Error: %s was not found in the project string table\n", qPrintable(key) );
            isError = true;

         }
      } 
   }
 
   return isError;
}


/*


void Config::getLogo(const QString route)
{

   QString iconName;

   if (route == "load") {
      iconName = m_project_iconFN;

   } else {
      QString path = pathName(m_curFile);
      iconName = QFileDialog::getOpenFileName(this, tr("Select Project Icon"), path);
   }

   if (iconName.isEmpty()) {
      m_ui->project_icon->setText(tr("No Logo was selected"));

   } else {
      QFile fout(iconName);

      if (! fout.exists()) {
         m_ui->project_icon->setText(tr("Unable to find file: ") + iconName);

      } else {
         QPixmap pm(iconName);

         if (! pm.isNull()) {
            m_ui->project_icon->setPixmap(pm.scaledToHeight(55, Qt::SmoothTransformation));

         } else {
            m_ui->project_icon->setText(tr("No preview is available for: ") + iconName);
         }
      }

      m_project_iconFN = iconName;
   }

}

*/

