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

QByteArray Config::json_ReadFile(QString fName)
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

bool Config::parseConfig(QString fName)
{ 
    bool retval;

   load_Defaults();
   retval = read_ProjectFile(fName);

   return retval;
}

void Config::load_Defaults()
{ 

//      QHash<QString, struc_CfgBool>   m_cfgBool;
//      QHash<QString, struc_CfgInt>    m_cfgInt;  
//      QHash<QString, struc_CfgEnum>   m_cfgEnum;
//      QHash<QString, struc_CfgList>   m_cfgList; 
//      QHash<QString, struc_CfgString> m_cfgString;

  
   // tab 1
   m_cfgString.insert("project-name",        struc_CfgString { "My Project",   DEFAULT } );                                           
   m_cfgString.insert("project-brief",       struc_CfgString { "",             DEFAULT } );  
   m_cfgString.insert("project-version",     struc_CfgString { "",             DEFAULT } );  
   m_cfgString.insert("project-logo",        struc_CfgString { "",             DEFAULT } );  
   m_cfgString.insert("output-dir",          struc_CfgString { "",             DEFAULT } );     

   m_cfgBool.insert("optimize-cplus",        struc_CfgBool   { true,           DEFAULT } );  
   m_cfgBool.insert("optimize-java",         struc_CfgBool   { false,          DEFAULT } );  
   m_cfgBool.insert("optimize-c",            struc_CfgBool   { false,          DEFAULT } );  
   m_cfgBool.insert("optimize-fortran",      struc_CfgBool   { false,          DEFAULT } );  


/*

   //  ***
   // tab 2 - project
   m_ui->project_encoding->setText(                object.value("project-encoding").toString());
   m_ui->create_subdirs_CB->setChecked(            object.value("create-subdirs").toBool());
   m_ui->allow_unicode_names_CB->setChecked(       object.value("allow-unicode-names").toBool());

   index = m_ui->output_language_CM->findText(     object.value("output-language").toString());
   m_ui->output_language_CM->setCurrentIndex(index);

   m_ui->brief_member_desc_CB->setChecked(         object.value("brief-member-desc").toBool());
   m_ui->repeat_brief_CB->setChecked(              object.value("repeat-brief").toBool());
   m_ui->abbreviate_brief->setPlainText(           getDataList(object, "abbreviate-brief"));
   m_ui->always_detailed_sec_CB->setChecked(       object.value("always-detailed-sec").toBool());
   m_ui->inline_inherited_member_CB->setChecked(   object.value("inline-inherited-member").toBool());
   m_ui->full_path_names_CB->setChecked(           object.value("full-path-names").toBool());
   m_ui->strip_from_path->setPlainText(            getDataList(object, "strip-from-path"));
   m_ui->strip_from_inc_path->setPlainText(        getDataList(object, "strip-from-inc-path"));

   m_ui->short_names_CB->setChecked(               object.value("short-names").toBool());
   m_ui->javadoc_auto_brief_CB->setChecked(        object.value("javadoc-auto-brief").toBool());
   m_ui->qt_auto_brief_CB->setChecked(             object.value("qt-auto-brief").toBool());
   m_ui->multiline_cpp_brief_CB->setChecked(       object.value("multiline-cpp-brief").toBool());
   m_ui->inherit_docs_CB->setChecked(              object.value("inherit-docs").toBool());
   m_ui->separate_member_pages_CB->setChecked(     object.value("separate-member-pages").toBool());

   m_ui->tab_size_SB->setValue(                    object.value("tab-size").toInt());
   m_ui->aliases->setPlainText(                    getDataList(object, "aliases"));
   m_ui->tcl_subst->setPlainText(                  getDataList(object, "tcl-subst"));
   m_ui->extension_mapping->setPlainText(          getDataList(object, "extension-mapping"));

   m_ui->markdown_CB->setChecked(                  object.value("markdown").toBool());
   m_ui->auto_link_CB->setChecked(                 object.value("auto-link").toBool());
   m_ui->built_in_stl_support_CB->setChecked(      object.value("built-in-stl-support").toBool());
   m_ui->cpp_cli_support_CB->setChecked(           object.value("cpp-cli-support").toBool());
   m_ui->sip_support_CB->setChecked(               object.value("sip-support").toBool());
   m_ui->idl_support_CB->setChecked(               object.value("idl-support").toBool());
   m_ui->dist_group_doc_CB->setChecked(            object.value("dist-group-doc").toBool());
   m_ui->allow_sub_grouping_CB->setChecked(        object.value("allow-sub-grouping").toBool());
   m_ui->inline_grouped_classes_CB->setChecked(    object.value("inline-grouped-classes").toBool());
   m_ui->inline_simple_struct_CB->setChecked(      object.value("inline-simple-struct").toBool());
   m_ui->typedef_hides_struct_CB->setChecked(      object.value("typedef-hides-struct").toBool());
   m_ui->lookup_cache_size_SB->setValue(           object.value("lookup-cache-size").toInt());

   // tab 2 - build
   m_ui->extract_all_CB->setChecked(               object.value("extract-all").toBool());
   m_ui->extract_private_CB->setChecked(           object.value("extract-private").toBool());
   m_ui->extract_package_CB->setChecked(           object.value("extract-package").toBool());
   m_ui->extract_static_CB->setChecked(            object.value("extract-static").toBool());
   m_ui->extract_local_classes_CB->setChecked(     object.value("extract-local-classes").toBool());
   m_ui->extract_local_methods_CB->setChecked(     object.value("extract-local-methods").toBool());
   m_ui->extract_anon_namespaces_CB->setChecked(   object.value("extract-anon-namespaces").toBool());
   m_ui->hide_undoc_members_CB->setChecked(        object.value("hide-undoc-members").toBool());
   m_ui->hide_undoc_classes_CB->setChecked(        object.value("hide-undoc-classes").toBool());
   m_ui->hide_friend_compounds_CB->setChecked(     object.value("hide-friend-compounds").toBool());
   m_ui->hide_in_body_docs_CB->setChecked(         object.value("hide-in-body-docs").toBool());
   m_ui->hide_scope_names_CB->setChecked(          object.value("hide-scope-names").toBool());

   m_ui->internal_docs_CB->setChecked(             object.value("internal-docs").toBool());
   m_ui->case_sense_names_CB->setChecked(          object.value("case-sense-names").toBool());
   m_ui->show_include_files_CB->setChecked(        object.value("show-include-files").toBool());
   m_ui->show_grouped_members_inc_CB->setChecked(  object.value("show-grouped-members-inc").toBool());
   m_ui->force_local_includes_CB->setChecked(      object.value("force-local-includes").toBool());
   m_ui->inline_info_CB->setChecked(               object.value("inline-info").toBool());

   m_ui->sort_member_docs_CB->setChecked(          object.value("sort-member-docs").toBool());
   m_ui->sort_brief_docs_CB->setChecked(           object.value("sort-brief-docs").toBool());
   m_ui->sort_members_ctors_first_CB->setChecked(  object.value("sort-members-ctors-first").toBool());
   m_ui->sort_group_names_CB->setChecked(          object.value("sort-group-names").toBool());
   m_ui->sort_by_scope_name_CB->setChecked(        object.value("sort-by-scope-name").toBool());

   m_ui->strict_proto_matching_CB->setChecked(     object.value("strict-proto-matching").toBool());
   m_ui->gen_todo_list_CB->setChecked(             object.value("generate-todo-list").toBool());
   m_ui->gen_test_list_CB->setChecked(             object.value("generate-test-list").toBool());
   m_ui->gen_bug_list_CB->setChecked(              object.value("generate-bug-list").toBool());
   m_ui->gen_deprecate_list_CB->setChecked(        object.value("generate-deprecate-list").toBool());

   m_ui->enabled_sections->setPlainText(           getDataList(object, "enabled-sections"));
   m_ui->max_init_lines_SB->setValue(              object.value("max-init-lines").toInt());
   m_ui->show_used_files_CB->setChecked(           object.value("show-used-files").toBool());
   m_ui->show_files_CB->setChecked(                object.value("show-files").toBool());
   m_ui->show_namespaces_CB->setChecked(           object.value("show-namespaces").toBool());
   m_ui->file_version_filter->setText(             object.value("file-version-filter").toString());
   m_ui->layout_file->setText(                     object.value("layout-file").toString());
   m_ui->cite_bib_files->setPlainText(             getDataList(object, "cite-bib-files"));

   // tab 2 - messages
   m_ui->quiet_CB->setChecked(                     object.value("quiet").toBool());
   m_ui->warnings_CB->setChecked(                  object.value("warnings").toBool());
   m_ui->warn_undoc_CB->setChecked(                object.value("warn-undoc").toBool());
   m_ui->warn_doc_error_CB->setChecked(            object.value("warn-doc-error").toBool());
   m_ui->warn_undoc_parm_CB->setChecked(           object.value("warn-undoc-parm").toBool());
   m_ui->warn_format->setText(                     object.value("warn-format").toString());
   m_ui->warn_logfile->setText(                    object.value("warn-logfile").toString());

   // tab 2 -input
   m_ui->input_source->setPlainText(               getDataList(object, "input-source"));
   m_ui->input_encoding->setText(                  object.value("input-encoding").toString());
   m_ui->file_patterns->setPlainText(              getDataList(object, "file-patterns"));
   m_ui->source_recursive_CB->setChecked(          object.value("source-recursive").toBool());

   m_ui->exclude_files->setPlainText(              getDataList(object, "exclude-files"));
   m_ui->exclude_symlinks_CB->setChecked(          object.value("exclude-symlinks").toBool());
   m_ui->exclude_patterns->setPlainText(           getDataList(object, "exclude-patterns"));
   m_ui->exclude_symbols->setPlainText(            getDataList(object, "exclude-symbols"));
   m_ui->example_source->setPlainText(             getDataList(object, "example-source"));
   m_ui->example_patterns->setPlainText(           getDataList(object, "example-pattens"));
   m_ui->example_recursive_CB->setChecked(         object.value("example-recursive").toBool());

   m_ui->image_path->setPlainText(                 getDataList(object, "image-path"));
   m_ui->input_filter->setText(                    object.value("input-filter").toString());
   m_ui->filter_patterns->setPlainText(            getDataList(object, "filter-patterns"));
   m_ui->filter_source_files_CB->setChecked(       object.value("filter-source-files").toBool());
   m_ui->filter_source_patterns->setPlainText(     getDataList(object, "filter-source-patterns"));
   m_ui->mdfile_mainpage->setText(                 object.value("mdfile-mainpage").toString());

   // tab 2 -browser
   m_ui->source_browser_CB->setChecked(            object.value("source-browser").toBool());
   m_ui->inline_sources_CB->setChecked(            object.value("inline-sources").toBool());
   m_ui->strip_code_comments_CB->setChecked(       object.value("strip-code-comments").toBool());
   m_ui->ref_by_relation_CB->setChecked(           object.value("ref-by-relation").toBool());
   m_ui->ref_relation_CB->setChecked(              object.value("ref-relation").toBool());
   m_ui->ref_link_source_CB->setChecked(           object.value("ref-link-source").toBool());
   m_ui->source_tooltips_CB->setChecked(           object.value("source-tooltips").toBool());
   m_ui->use_htags_CB->setChecked(                 object.value("use-htags").toBool());
   m_ui->verbatim_headers_CB->setChecked(          object.value("verbatim-headers").toBool());
   m_ui->clang_parsing_CB->setChecked(             object.value("clang-parsing").toBool());
   m_ui->clang_options->setPlainText(              getDataList(object, "clang-options"));

   // tab 2 -index
   m_ui->alpha_index_CB->setChecked(               object.value("alpha-index").toBool());
   m_ui->cols_in_index_SB->setValue(               object.value("cols-in-index").toInt());
   m_ui->ignore_prefix->setPlainText(              getDataList(object, "ignore-prefix"));

   // tab 2 - autogen
   m_ui->gen_autogen_def_CB->setChecked(           object.value("generate-autogen-def").toBool());

   // tab 2 - perlmod         
   m_ui->gen_perl_CB->setChecked(                  object.value("generate-perl").toBool());
   m_ui->perl_latex_CB->setChecked(                object.value("perl-latex").toBool());
   m_ui->perl_pretty_CB->setChecked(               object.value("perl-pretty").toBool());
   m_ui->perlmod_prefix->setText(                  object.value("perlmod-prefix").toString());

   // tab 2 - preprocess
   m_ui->enable_preprocessing_CB->setChecked(      object.value("enable-preprocessing").toBool());
   m_ui->macro_expansion_CB->setChecked(           object.value("macro-expansion").toBool());
   m_ui->expand_only_predefined_CB->setChecked(    object.value("expand-only-predefined").toBool());
   m_ui->search_includes_CB->setChecked(           object.value("search-includes").toBool());

   m_ui->include_path->setPlainText(               getDataList(object,"include-path"));
   m_ui->include_file_patterns->setPlainText(      getDataList(object,"include-file-patterns"));
   m_ui->predefined_macros->setPlainText(          getDataList(object,"predefined-macros"));
   m_ui->expand_as_defined->setPlainText(          getDataList(object,"expand-as-defined"));
   m_ui->skip_function_macros_CB->setChecked(      object.value("skip-function-macros").toBool());

   // tab 2 - external
   m_ui->tag_files->setPlainText(                  getDataList(object,"tag-files"));
   m_ui->gen_tagfile->setText(                     object.value("generate-tagfile").toString());
   m_ui->all_externals_CB->setChecked(             object.value("all-externals").toBool());
   m_ui->external_groups_CB->setChecked(           object.value("external-groups").toBool());
   m_ui->external_pages_CB->setChecked(            object.value("external-pages").toBool());
   m_ui->perl_path->setText(                       object.value("perl-path").toString());

   // tab 2 - dot
   m_ui->class_diagrams_CB->setChecked(            object.value("class-diagrams").toBool());
   m_ui->mscgen_path->setText(                     object.value("mscgen-path").toString());
   m_ui->dia_path->setText(                        object.value("dia-path").toString());
   m_ui->hide_undoc_relations_CB->setChecked(      object.value("hide-undoc-relations").toBool());
   m_ui->have_dot_CB->setChecked(                  object.value("have-dot").toBool());
   m_ui->dot_num_threads_SB->setValue(             object.value("dot-num-threads").toInt());
   m_ui->dot_font_name->setText(                   object.value("dot-font-name").toString());
   m_ui->dot_font_size_SB->setValue(               object.value("dot-font-size").toInt());
   m_ui->dot_font_path->setText(                   object.value("dot-font-path").toString());

   m_ui->group_graphs_CB->setChecked(              object.value("group-graphs").toBool());
   m_ui->uml_look_CB->setChecked(                  object.value("uml-look").toBool());
   m_ui->uml_limit_num_fields_SB->setValue(        object.value("uml-limit-num-fields").toInt());      
   m_ui->template_relations_CB->setChecked(        object.value("template-relations").toBool());
   m_ui->directory_graph_CB->setChecked(           object.value("directory-graph").toBool());

   index = m_ui->dot_image_format_CM->findText(    object.value("dot-image-format").toString());
   m_ui->dot_image_format_CM->setCurrentIndex(index);

   m_ui->interactive_svg_CB->setChecked(           object.value("interactive_svg").toBool());
   m_ui->dot_path->setText(                        object.value("dot-path").toString());
   m_ui->dot_file_dirs->setPlainText(              getDataList(object, "dot-file_dirs"));
   m_ui->msc_file_dirs->setPlainText(              getDataList(object, "msc-file_dirs"));
   m_ui->dia_file_dirs->setPlainText(              getDataList(object, "dia-file_dirs"));
   m_ui->plantuml_jar_path->setText(               object.value("plantuml-jar-path").toString());

   m_ui->dot_graph_max_nodes_SB->setValue(         object.value("dot-graph_max-nodes").toInt());
   m_ui->dot_graph_max_depth_SB->setValue(         object.value("dot-graph-max-depth").toInt());
   m_ui->dot_transparent_CB->setChecked(           object.value("dot-transparent").toBool());
   m_ui->dot_multiple_targets_CB->setChecked(      object.value("dot-multiple-targets").toBool());
   m_ui->gen_legend_CB->setChecked(                object.value("generate-legend").toBool());
   m_ui->dot_cleanup_CB->setChecked(               object.value("dot-cleanup").toBool());
*/

 

/*
   m_cfgBool.insert("generate-latex"         struc_CfgBool { object.value("generate-latex").toBool(),     DEFAULT } ); 
   m_cfgBool.insert("latex-hyper-pdf"        struc_CfgBool { object.value("latex-hyper-pdf").toBool(),    DEFAULT } ); 
   m_cfgBool.insert("latex-pdf"              struc_CfgBool { object.value("latex-pdf").toBool(),          DEFAULT } ); 
   m_cfgBool.insert("latex-ps"               struc_CfgBool { object.value("latex-ps").toBool(),           DEFAULT } ); 
   m_cfgBool.insert("generate-rtf"           struc_CfgBool { object.value("generate-rtf").toBool(),       DEFAULT } ); 
   m_cfgBool.insert("generate-man"           struc_CfgBool { object.value("generate-man").toBool(),       DEFAULT } ); 
   m_cfgBool.insert("generate-xml"           struc_CfgBool { object.value("generate-xml").toBool(),       DEFAULT } ); 
   m_cfgBool.insert("generate-docbook"       struc_CfgBool { object.value("generate-docbook").toBool(),   DEFAULT } ); 
*/


   //  ***
   // tab 3 - html
   m_cfgBool.insert("generate-html",         struc_CfgBool { true,             DEFAULT } );  


/*
   m_ui->html_output->setText(                     object.value("html-output").toString());
   m_ui->html_file_extension->setText(             object.value("html-file-extension").toString());
   m_ui->html_header->setText(                     object.value("html-header").toString());
   m_ui->html_footer->setText(                     object.value("html-footer").toString());   
   m_ui->html_extra_stylesheets->setPlainText(     getDataList(object, "html-extra-stylesheets"));
   m_ui->html_extra_files->setPlainText(           getDataList(object, "html-extra-files"));

   m_ui->html_colorstyle_hue->setValue(            object.value("html-colorstyle-hue").toInt());
   m_ui->html_colorstyle_sat->setValue(            object.value("html-colorstyle-sat").toInt());
   m_ui->html_colorstyle_gamma->setValue(          object.value("html-colorstyle-gamma").toInt());

   m_ui->html_timestamp_CB->setChecked(            object.value("html-timestamp").toBool());
   m_ui->html_dynamic_sections_CB->setChecked(     object.value("html-dynamic-sections").toBool());
   m_ui->html_index_num_entries_SB->setValue(      object.value("html-index-num-entries").toInt());

   m_ui->disable_index_CB->setChecked(             object.value("disable-index").toBool());
   m_ui->gen_treeview_CB->setChecked(              object.value("generate-treeview").toBool());
   m_ui->enum_values_per_line_SB->setValue(        object.value("enum-values-per-line").toInt());
   m_ui->treeview_width_SB->setValue(              object.value("treeview-width").toInt());
   m_ui->external_links_in_window_CB->setChecked(  object.value("external-links-in-window").toBool());

   m_ui->server_based_search_CB->setChecked(       object.value("server-based-search").toBool());
   m_ui->external_search_CB->setChecked(           object.value("external-search").toBool());
   m_ui->search_engine_url->setText(               object.value("search-engine-url").toString());
   m_ui->search_data_file->setText(                object.value("search-data-file").toString());
   m_ui->search_external_id->setText(              object.value("search-external-id").toString());
   m_ui->search_mappings->setPlainText(            getDataList(object, "search-mappings"));

   m_ui->formula_fontsize_SB->setValue(            object.value("formula-fontsize").toInt());
   m_ui->formula_transparent_CB->setChecked(       object.value("formula-transparent").toBool());
   m_ui->use_mathjax_CB->setChecked(               object.value("use_mathjax").toBool());

   index = m_ui->mathjax_format_CM->findText(      object.value("mathjax-format").toString());
   m_ui->mathjax_format_CM->setCurrentIndex(index);

   m_ui->mathjax_relpath->setText(                 object.value("mathjax-relpath").toString());
   m_ui->mathjax_extensions->setPlainText(         getDataList(object, "mathjax-extensions"));
   m_ui->mathjax_codefile->setText(                object.value("mathjax-codefile").toString());

   // tab 3 - chm
   m_ui->gen_chm_CB->setChecked(                   object.value("generate-chm").toBool());
   m_ui->chm_file->setText(                        object.value("chm-file").toString());
   m_ui->hhc_location->setText(                    object.value("hhc-location").toString());
   m_ui->gen_chi_CB->setChecked(                   object.value("generate-chi").toBool());
   m_ui->chm_index_encoding->setText(              object.value("chm-index-encoding").toString());
   m_ui->binary_toc_CB->setChecked(                object.value("binary-toc").toBool());
   m_ui->toc_expanded_CB->setChecked(              object.value("toc-expanded").toBool());

   // tab 3 - docset
   m_ui->gen_docset_CB->setChecked(                object.value("generate-docset").toBool());
   m_ui->docset_feedname->setText(                 object.value("docset-feedname").toString());
   m_ui->docset_bundle_id->setText(                object.value("docset-bundle-id").toString());
   m_ui->docset_publisher_id->setText(             object.value("docset-publisher-id").toString());
   m_ui->docset_publisher_name->setText(           object.value("docset-publisher-name").toString());

   // tab 3 - eclipse
   m_ui->gen_eclipse_CB->setChecked(               object.value("generate-eclipse").toBool());
   m_ui->eclipse_doc_id->setText(                  object.value("eclipse-doc-id").toString());

   // tab 3 - latex
   m_ui->latex_output->setText(                    object.value("latex-output").toString());
   m_ui->latex_cmd_name->setText(                  object.value("latex-cmd-name").toString());
   m_ui->make_index_cmd_name->setText(             object.value("make-index-cmd-name").toString());
   m_ui->latex_compact_CB->setChecked(             object.value("latex-compact").toBool());

   index = m_ui->latex_paper_type_CM->findText(    object.value("latex-paper-type").toString());
   m_ui->latex_paper_type_CM->setCurrentIndex(index);

   m_ui->latex_extra_packages->setPlainText(       getDataList(object, "latex-extra-packages"));
   m_ui->latex_header->setText(                    object.value("latex-header").toString());
   m_ui->latex_footer->setText(                    object.value("latex-footer").toString());
   m_ui->latex_extra_files->setPlainText(          getDataList(object, "latex-extra-files"));

   m_ui->latex_hyper_pdf_CB->setChecked(           object.value("latex-hyper-pdf").toBool());
   m_ui->latex_pdf_CB->setChecked(                 object.value("latex-pdf").toBool());
   m_ui->latex_batch_mode_CB->setChecked(          object.value("latex-batch-mode").toBool());
   m_ui->latex_hide_indices_CB->setChecked(        object.value("latex-hide-indices").toBool());
   m_ui->latex_source_code_CB->setChecked(         object.value("latex-source-code").toBool());
   m_ui->latex_bib_style->setText(                 object.value("latex-bib-style").toString());

   // tab 3 - man
   m_ui->man_output->setText(                      object.value("man-output").toString());
   m_ui->man_extension->setText(                   object.value("man-extension").toString());
   m_ui->man_subdir->setText(                      object.value("man-subdir").toString());
   m_ui->man_links_CB->setChecked(                 object.value("man-links").toBool());

   // tab 3 - qt help
   m_ui->gen_qthelp_CB->setChecked(                object.value("generate-qthelp").toBool());
   m_ui->qch_file->setText(                        object.value("qch-file").toString());
   m_ui->qhp_namespace->setText(                   object.value("qhp-namespace").toString());
   m_ui->qhp_virtual_folder->setText(              object.value("qhp-virtual-folder").toString());
   m_ui->qhp_cust_filter_name->setText(            object.value("qhp-cust-filter-name").toString());
   m_ui->qhp_cust_filter_attrib->setText(          object.value("qhp-cust-attrib").toString());
   m_ui->qhp_sect_filter_attrib->setText(          object.value("qhp-sect-attrib").toString());
   m_ui->qthelp_gen_path->setText(                 object.value("qthelp-gen-path").toString());

   // tab 3 - rtf
   m_ui->rtf_output->setText(                      object.value("rtf-output").toString());
   m_ui->rtf_compact_CB->setChecked(               object.value("rtf-compact").toBool());
   m_ui->rtf_hyperlinks_CB->setChecked(            object.value("rtf-hyperlinks").toBool());
   m_ui->rtf_stylesheet->setText(                  object.value("rtf-stylesheet").toString());
   m_ui->rtf_extension->setText(                   object.value("rtf-extension").toString());

   // tab 3 - xml
   m_ui->xml_output->setText(                      object.value("xml-output").toString());
   m_ui->xml_program_listing_CB->setChecked(       object.value("xml-program-listing").toBool());

   // tab 3 - docbook     
   m_ui->docbook_output->setText(                  object.value("docbook-output").toString());
   m_ui->docbook_program_listing_CB->setChecked(   object.value("docbook-program-listing").toBool());

*/


   // final part
//BROOM       getIcon("load");

}


bool Config::read_ProjectFile(QString fName)
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

/*       auto hashIter = m_cfgEnum.find(key);

         if (hashIter != m_cfgEnum.end()) {
            hashIter.value() = { iter.value().toEnum(), PROJECT };            

         }  else {
            fprintf(stderr, "Error: %s was not found in the project enum table\n", qPrintable(key) );
            isError = true;

         } 
*/

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

/*
      if (iter.value().isInteger()) {
         auto hashIter = m_cfgInt.find(key);

         if (hashIter != m_cfgInt.end()) {
            hashIter.value() = { iter.value().toInt(), PROJECT };            

         }  else {
            fprintf(stderr, "Error: %s was not found in the project integer table\n", qPrintable(key) );
            isError = true;

         }
      }
*/



//    if (iter.value().isList()) {
//    }


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

   verify();

   return ! isError;
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

