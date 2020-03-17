/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
*
* DoxyPress is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License version 2
* as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
*************************************************************************/

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <config.h>
#include <message.h>

QHash<QString, Config::struc_CfgBool>   Config::m_cfgBool;
QHash<QString, Config::struc_CfgInt>    Config::m_cfgInt;
QHash<QString, Config::struc_CfgEnum>   Config::m_cfgEnum;
QHash<QString, Config::struc_CfgList>   Config::m_cfgList;
QHash<QString, Config::struc_CfgString> Config::m_cfgString;

QDir Config::m_configDir;

QDir Config::getConfigDir()
{
   return m_configDir;
}

QString Config::getFullName(const QString &fName)
{
   QString retval;

   if (! fName.isEmpty()) {
      QFileInfo fi(m_configDir, fName);

      if (fi.exists() ) {
         retval = fi.absoluteFilePath();
      }
   }

   return retval;
}

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

   // set currentDir
   QFileInfo temp(fName);

   QString workingDir = temp.absolutePath();
   QDir::setCurrent(workingDir);

   // save currentDir
   m_configDir = QDir{workingDir};

   return data;
}

bool Config::parseConfig(const QString &fName)
{
   printf("**  Read Project Configuration\n");

   if (! read_ProjectFile(fName)) {
      return false;
   }

   printf("**  Verify Project Configuration\n\n");
   msgVerify();

   if (! preVerify()) {
      return false;
   }

   if (! verify()) {
      return false;
   }

   // not used at this time
   // Config::instance()->substituteEnvironmentVars();

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
   m_cfgBool.insert("optimize-python",           struc_CfgBool   { false,          DEFAULT } );

   // tab 2 - project configuration
   m_cfgEnum.insert("output-language",           struc_CfgEnum   { "ENGLISH",      DEFAULT } );
   m_cfgList.insert("aliases",                   struc_CfgList   { QStringList(),  DEFAULT } );

   const QStringList tempList1 = Config::getAbbreviateBrief();
   m_cfgList.insert("abbreviate-brief",          struc_CfgList   { tempList1,      DEFAULT } );

   m_cfgList.insert("strip-from-path",           struc_CfgList   { QStringList(),  DEFAULT } );
   m_cfgList.insert("strip-from-inc-path",       struc_CfgList   { QStringList(),  DEFAULT } );

   m_cfgBool.insert("full-path-names",           struc_CfgBool   { true,           DEFAULT } );
   m_cfgBool.insert("brief-member-desc",         struc_CfgBool   { true,           DEFAULT } );
   m_cfgBool.insert("repeat-brief",              struc_CfgBool   { true,           DEFAULT } );
   m_cfgBool.insert("always-detailed-sec",       struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("javadoc-auto-brief",        struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("qt-auto-brief",             struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("multiline-cpp-brief",       struc_CfgBool   { false,          DEFAULT } );

   m_cfgInt.insert("tab-size",                   struc_CfgInt    { 4,              DEFAULT } );
   m_cfgInt.insert("lookup-cache-size",          struc_CfgInt    { 0,              DEFAULT } );

   // tab 2 - build configuration
   m_cfgBool.insert("extract-all",               struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("extract-private",           struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("extract-package",           struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("extract-static",            struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("extract-local-classes",     struc_CfgBool   { true,           DEFAULT } );
   m_cfgBool.insert("extract-local-methods",     struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("extract-anon-namespaces",   struc_CfgBool   { false,          DEFAULT } );

   m_cfgBool.insert("hide-undoc-members",        struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("hide-undoc-classes",        struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("hide-friend-compounds",     struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("hide-in-body-docs",         struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("hide-scope-names",          struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("hide-compound-ref",         struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("hide-navtree-members",      struc_CfgBool   { false,          DEFAULT } );

   m_cfgBool.insert("show-include-files",        struc_CfgBool   { true,           DEFAULT } );
   m_cfgBool.insert("show-grouped-members-inc",  struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("show-used-files",           struc_CfgBool   { true,           DEFAULT } );
   m_cfgBool.insert("show-file-page",            struc_CfgBool   { true,           DEFAULT } );
   m_cfgBool.insert("show-namespace-page",       struc_CfgBool   { true,           DEFAULT } );

   m_cfgBool.insert("inline-inherited-member",   struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("inline-info",               struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("inline-grouped-classes",    struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("inline-simple-struct",      struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("use-typedef-name",          struc_CfgBool   { false,          DEFAULT } );

   m_cfgBool.insert("sort-member-docs",          struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("sort-brief-docs",           struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("sort-constructors-first",   struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("sort-group-names",          struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("sort-by-scope-name",        struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("sort-class-case-sensitive", struc_CfgBool   { false,          DEFAULT } );

   m_cfgBool.insert("generate-todo-list",        struc_CfgBool   { true,           DEFAULT } );
   m_cfgBool.insert("generate-test-list",        struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("generate-bug-list",         struc_CfgBool   { true,           DEFAULT } );
   m_cfgBool.insert("generate-deprecate-list",   struc_CfgBool   { true,           DEFAULT } );

   m_cfgBool.insert("short-names",               struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("allow-unicode-names",       struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("create-subdirs",            struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("case-sensitive-fname",      struc_CfgBool   { true,           DEFAULT } );

   m_cfgBool.insert("markdown",                  struc_CfgBool   { true,           DEFAULT } );
   m_cfgBool.insert("auto-link",                 struc_CfgBool   { true,           DEFAULT } );
   m_cfgBool.insert("strict-sig-matching",       struc_CfgBool   { false,          DEFAULT } );

   m_cfgBool.insert("internal-docs",             struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("force-local-includes",      struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("inherit-docs",              struc_CfgBool   { true,           DEFAULT } );

   m_cfgBool.insert("separate-member-pages",     struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("allow-sub-grouping",        struc_CfgBool   { true,           DEFAULT } );
   m_cfgBool.insert("duplicate-docs",            struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("group-nested-compounds",    struc_CfgBool   { false,          DEFAULT } );

   // tab 2 -build options
   m_cfgList.insert("enabled-sections",          struc_CfgList   { QStringList(),  DEFAULT } );
   m_cfgString.insert("file-version-filter",     struc_CfgString { QString(),      DEFAULT } );
   m_cfgString.insert("main-page-name",          struc_CfgString { "",             DEFAULT } );
   m_cfgBool.insert("main-page-omit",            struc_CfgBool   { false,          DEFAULT } );
   m_cfgString.insert("layout-file",             struc_CfgString { QString(),      DEFAULT } );
   m_cfgList.insert("ns-alias",                  struc_CfgList   { QStringList(),  DEFAULT } );
   m_cfgInt.insert("max-init-lines",             struc_CfgInt    { 30,             DEFAULT } );
   m_cfgInt.insert("toc-include-headers",        struc_CfgInt    { 0,              DEFAULT } );
   m_cfgBool.insert("bb-style",                  struc_CfgBool   { false,          DEFAULT } );

   // tab 2 -programming languages
   m_cfgList.insert("tcl-subst",                 struc_CfgList   { QStringList(),  DEFAULT } );
   m_cfgList.insert("language-mapping",          struc_CfgList   { QStringList(),  DEFAULT } );
   m_cfgBool.insert("built-in-stl-support",      struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("cpp-cli-support",           struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("sip-support",               struc_CfgBool   { false,          DEFAULT } );
   m_cfgBool.insert("idl-support",               struc_CfgBool   { true,           DEFAULT } );

   // tab 2 - messages
   m_cfgBool.insert("quiet",                     struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("warnings",                  struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("warn-undoc",                struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("warn-doc-error",            struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("warn-undoc-param",          struc_CfgBool   { false,           DEFAULT } );
   m_cfgString.insert("warn-format",             struc_CfgString { "$file:$line: $text", DEFAULT } );
   m_cfgString.insert("warn-logfile",            struc_CfgString { QString(),       DEFAULT } );

   // tab 2 -input source files
   m_cfgList.insert("input-source",              struc_CfgList   { QStringList(),   DEFAULT } );

   QStringList tempList2 = Config::getFilePatterns();
   m_cfgList.insert("input-patterns",            struc_CfgList   { tempList2,       DEFAULT } );

   m_cfgString.insert("input-encoding",          struc_CfgString { "UTF-8",         DEFAULT } );
   m_cfgBool.insert("input-recursive",           struc_CfgBool   { false,           DEFAULT } );

   m_cfgList.insert("exclude-files",             struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgBool.insert("exclude-symlinks",          struc_CfgBool   { false,           DEFAULT } );
   m_cfgList.insert("exclude-patterns",          struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgList.insert("exclude-symbols",           struc_CfgList   { QStringList(),   DEFAULT } );

   // tab 2 - input other files
   m_cfgList.insert("example-source",            struc_CfgList   { QStringList(),   DEFAULT } );

   QStringList temp;
   temp.append("*");
   m_cfgList.insert("example-patterns",          struc_CfgList   { temp,            DEFAULT } );

   m_cfgBool.insert("example-recursive",         struc_CfgBool   { false,           DEFAULT } );

   m_cfgList.insert("image-path",                struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgString.insert("mdfile-mainpage",         struc_CfgString { QString(),       DEFAULT } );

   // tab 2 -index filters
   m_cfgString.insert("filter-program",          struc_CfgString { QString(),       DEFAULT } );
   m_cfgList.insert("filter-patterns",           struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgBool.insert("filter-source-files",       struc_CfgBool   { false,           DEFAULT } );
   m_cfgList.insert("filter-source-patterns",    struc_CfgList   { QStringList(),   DEFAULT } );

   // tab 2 -index page
   m_cfgBool.insert("alpha-index",               struc_CfgBool   { true,            DEFAULT } );
   m_cfgInt.insert("cols-in-index",              struc_CfgInt    { 5,               DEFAULT } );
   m_cfgList.insert("ignore-prefix",             struc_CfgList   { QStringList(),   DEFAULT } );

   // tab 2 - preprocessor
   m_cfgBool.insert("enable-preprocessing",      struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("search-includes",           struc_CfgBool   { true,            DEFAULT } );

   m_cfgList.insert("include-path",              struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgList.insert("include-patterns",          struc_CfgList   { QStringList(),   DEFAULT } );

   m_cfgBool.insert("macro-expansion",           struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("expand-only-predefined",    struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("skip-function-macros",      struc_CfgBool   { true,            DEFAULT } );

   m_cfgList.insert("predefined-macros",         struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgList.insert("expand-as-defined",         struc_CfgList   { QStringList(),   DEFAULT } );

   // tab 2 - clang
   m_cfgBool.insert("clang-parsing",             struc_CfgBool   { false,           DEFAULT } );
   m_cfgString.insert("clang-compilation-path",  struc_CfgString { QString(),       DEFAULT } );
   m_cfgString.insert("clang-dialect",           struc_CfgString { "--std=c++14",   DEFAULT } );
   m_cfgBool.insert("clang-use-headers",         struc_CfgBool   { true,            DEFAULT } );
   m_cfgList.insert("clang-flags",               struc_CfgList   { QStringList(),   DEFAULT } );

   // tab 2 - source listing
   m_cfgBool.insert("source-code",               struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("inline-source",             struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("verbatim-headers",          struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("strip-code-comments",       struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("ref-by-relation",           struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("ref-relation",              struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("ref-link-source",           struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("source-tooltips",           struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("use-htags",                 struc_CfgBool   { false,           DEFAULT } );

   QStringList tempList3 = Config::getSuffixSource();
   m_cfgList.insert("suffix-source-navtree",     struc_CfgList   { tempList3,       DEFAULT } );

   QStringList tempList4 = Config::getSuffixHeader();
   m_cfgList.insert("suffix-header-navtree",     struc_CfgList   { tempList4,       DEFAULT } );

   QStringList tempList5 = Config::getSuffixExclude();
   m_cfgList.insert("suffix-exclude-navtree",    struc_CfgList   { tempList5,       DEFAULT } );

   // tab 2 - external
   m_cfgList.insert("tag-files",                 struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgString.insert("generate-tagfile",        struc_CfgString { QString(),       DEFAULT } );
   m_cfgBool.insert("all-externals",             struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("external-groups",           struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("external-pages",            struc_CfgBool   { true,            DEFAULT } );
   m_cfgString.insert("perl-path",               struc_CfgString { "/usr/bin/perl", DEFAULT } );

   // tab 2 - dot
   m_cfgBool.insert("class-diagrams",            struc_CfgBool   { true,            DEFAULT } );
   m_cfgString.insert("mscgen-path",             struc_CfgString { QString(),       DEFAULT } );
   m_cfgString.insert("dia-path",                struc_CfgString { QString(),       DEFAULT } );
   m_cfgBool.insert("hide-undoc-relations",      struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("have-dot",                  struc_CfgBool   { false,           DEFAULT } );
   m_cfgInt.insert("dot-num-threads",            struc_CfgInt    { 0,               DEFAULT } );
   m_cfgString.insert("dot-font-name",           struc_CfgString { "Helvetica",     DEFAULT } );
   m_cfgInt.insert("dot-font-size",              struc_CfgInt    { 10,              DEFAULT } );
   m_cfgString.insert("dot-font-path",           struc_CfgString { QString(),       DEFAULT } );

   m_cfgBool.insert("group-graphs",              struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("uml-look",                  struc_CfgBool   { false,           DEFAULT } );
   m_cfgInt.insert("uml-limit-num-fields",       struc_CfgInt    { 10,              DEFAULT } );
   m_cfgBool.insert("template-relations",        struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("directory-graph",           struc_CfgBool   { true,            DEFAULT } );

   m_cfgEnum.insert("dot-image-format",          struc_CfgEnum   { "png",           DEFAULT } );
   m_cfgBool.insert("interactive-svg",           struc_CfgBool   { false,           DEFAULT } );
   m_cfgString.insert("dot-path",                struc_CfgString { QString(),       DEFAULT } );
   m_cfgList.insert("dot-file-dirs",             struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgList.insert("msc-file-dirs",             struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgList.insert("dia-file-dirs",             struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgString.insert("plantuml-jar-path",       struc_CfgString { QString(),       DEFAULT } );
   m_cfgString.insert("plantuml-cfg-file",       struc_CfgString { QString(),       DEFAULT } );
   m_cfgList.insert("plantuml-inc-path",         struc_CfgList   { QStringList(),   DEFAULT } );

   m_cfgInt.insert("dot-graph-max-nodes",        struc_CfgInt    { 50,              DEFAULT } );
   m_cfgInt.insert("dot-graph-max-depth",        struc_CfgInt    { 0,               DEFAULT } );
   m_cfgBool.insert("dot-transparent",           struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("dot-multiple-targets",      struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("generate-legend",           struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("dot-cleanup",               struc_CfgBool   { true,            DEFAULT } );

   // tab 3 ( appear on tab 1 and tab 3 )
   m_cfgBool.insert("generate-html",             struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("generate-latex",            struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("generate-rtf",              struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("generate-man",              struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("generate-xml",              struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("generate-docbook",          struc_CfgBool   { false,           DEFAULT } );

   m_cfgBool.insert("dot-class-graph",           struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("dot-collaboration",         struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("dot-hierarchy",             struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("dot-include",               struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("dot-included-by",           struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("dot-call",                  struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("dot-called-by",             struc_CfgBool   { false,           DEFAULT } );

   // tab 3 - html
   m_cfgString.insert("html-output",             struc_CfgString { "html",          DEFAULT } );
   m_cfgString.insert("html-file-extension",     struc_CfgString { ".html",         DEFAULT } );
   m_cfgString.insert("html-header",             struc_CfgString { QString(),       DEFAULT } );
   m_cfgString.insert("html-footer",             struc_CfgString { QString(),       DEFAULT } );
   m_cfgList.insert("html-stylesheets",          struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgList.insert("html-extra-files",          struc_CfgList   { QStringList(),   DEFAULT } );

   m_cfgInt.insert("html-colorstyle-hue",        struc_CfgInt    { 220,             DEFAULT } );
   m_cfgInt.insert("html-colorstyle-sat",        struc_CfgInt    { 100,             DEFAULT } );
   m_cfgInt.insert("html-colorstyle-gamma",      struc_CfgInt    { 80,              DEFAULT } );
   m_cfgBool.insert("html-timestamp",            struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("html-dynamic-menus",        struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("html-dynamic-sections",     struc_CfgBool   { false,           DEFAULT } );
   m_cfgInt.insert("html-index-num-entries",     struc_CfgInt    { 100,             DEFAULT } );

   m_cfgBool.insert("disable-index",             struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("generate-treeview",         struc_CfgBool   { false,           DEFAULT } );
   m_cfgInt.insert("enum-values-per-line",       struc_CfgInt    { 4,               DEFAULT } );
   m_cfgInt.insert("treeview-width",             struc_CfgInt    { 250,             DEFAULT } );
   m_cfgBool.insert("external-links-in-window",  struc_CfgBool   { false,           DEFAULT } );

   m_cfgBool.insert("html-search",               struc_CfgBool   { true,             DEFAULT } );
   m_cfgBool.insert("search-server-based",       struc_CfgBool   { false,            DEFAULT } );
   m_cfgBool.insert("search-external",           struc_CfgBool   { false,            DEFAULT } );
   m_cfgString.insert("search-external-url",     struc_CfgString { QString(),        DEFAULT } );
   m_cfgString.insert("search-data-file",        struc_CfgString { "searchdata.xml", DEFAULT } );
   m_cfgString.insert("search-external-id",      struc_CfgString { QString(),        DEFAULT } );
   m_cfgList.insert("search-mappings",           struc_CfgList   { QStringList(),    DEFAULT } );

   m_cfgInt.insert("formula-fontsize",           struc_CfgInt    { 10,              DEFAULT } );
   m_cfgBool.insert("formula-transparent",       struc_CfgBool   { true,            DEFAULT } );
   m_cfgString.insert("ghostscript",             struc_CfgString { QString(),       DEFAULT } );
   m_cfgBool.insert("use-mathjax",               struc_CfgBool   { false,           DEFAULT } );
   m_cfgEnum.insert("mathjax-format",            struc_CfgEnum   { "HTML-CSS",      DEFAULT } );
   m_cfgString.insert("mathjax-relpath",         struc_CfgString { "https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.2/", DEFAULT } );
   m_cfgList.insert("mathjax-extensions",        struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgString.insert("mathjax-codefile",        struc_CfgString { QString(),       DEFAULT } );

   // tab 3 - chm
   m_cfgBool.insert("generate-chm",              struc_CfgBool   { false,           DEFAULT } );
   m_cfgString.insert("chm-file",                struc_CfgString { QString(),       DEFAULT } );
   m_cfgString.insert("hhc-location",            struc_CfgString { QString(),       DEFAULT } );
   m_cfgBool.insert("generate-chi",              struc_CfgBool   { false,           DEFAULT } );
   m_cfgString.insert("chm-index-encoding",      struc_CfgString { QString(),       DEFAULT } );
   m_cfgBool.insert("binary-toc",                struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("toc-expanded",              struc_CfgBool   { false,           DEFAULT } );

   // tab 3 - docbook
   m_cfgString.insert("docbook-output",          struc_CfgString { "docbook",        DEFAULT } );
   m_cfgBool.insert("docbook-program-listing",   struc_CfgBool   { false,           DEFAULT } );

   // tab 3 - docset
   m_cfgBool.insert("generate-docset",           struc_CfgBool   { false,           DEFAULT } );
   m_cfgString.insert("docset-feedname",         struc_CfgString { "DoxyPress generated docs", DEFAULT } );
   m_cfgString.insert("docset-bundle-id",        struc_CfgString { "org.doxypress.Project",    DEFAULT } );
   m_cfgString.insert("docset-publisher-id",     struc_CfgString { "org.doxypress.Publisher",  DEFAULT } );
   m_cfgString.insert("docset-publisher-name",   struc_CfgString { "Publisher",     DEFAULT } );

   // tab 3 - eclipse
   m_cfgBool.insert("generate-eclipse",          struc_CfgBool   { false,           DEFAULT } );
   m_cfgString.insert("eclipse-doc-id",          struc_CfgString { "org.doxypress.Project",    DEFAULT } );

   // tab 3 - latex
   m_cfgString.insert("latex-output",            struc_CfgString { "latex",         DEFAULT } );
   m_cfgString.insert("latex-cmd-name",          struc_CfgString { "latex",         DEFAULT } );
   m_cfgString.insert("make-index-cmd-name",     struc_CfgString { "makeindex",     DEFAULT } );
   m_cfgBool.insert("latex-compact",             struc_CfgBool   { false,           DEFAULT } );
   m_cfgEnum.insert("latex-paper-type",          struc_CfgEnum   { "a4",            DEFAULT } );
   m_cfgList.insert("latex-extra-packages",      struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgString.insert("latex-header",            struc_CfgString { QString(),       DEFAULT } );
   m_cfgString.insert("latex-footer",            struc_CfgString { QString(),       DEFAULT } );
   m_cfgList.insert("latex-stylesheets",         struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgList.insert("latex-extra-files",         struc_CfgList   { QStringList(),   DEFAULT } );

   m_cfgBool.insert("latex-timestamp",           struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("latex-hyper-pdf",           struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("latex-pdf",                 struc_CfgBool   { true,            DEFAULT } );
   m_cfgBool.insert("latex-ps",                  struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("latex-batch-mode",          struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("latex-hide-indices",        struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("latex-source-code",         struc_CfgBool   { false,           DEFAULT } );

   // tab 3 - laxtex bib
   m_cfgString.insert("latex-bib-style",         struc_CfgString { "plain",         DEFAULT } );
   m_cfgList.insert("cite-bib-files",            struc_CfgList   { QStringList(),   DEFAULT } );

   // tab 3 - man
   m_cfgString.insert("man-output",              struc_CfgString { "man",           DEFAULT } );
   m_cfgString.insert("man-extension",           struc_CfgString { ".3",            DEFAULT } );
   m_cfgString.insert("man-subdir",              struc_CfgString { QString(),       DEFAULT } );
   m_cfgBool.insert("man-links",                 struc_CfgBool   { false,           DEFAULT } );

   // tab 2 - perlmod
   m_cfgBool.insert("generate-perl",             struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("perl-latex",                struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("perl-pretty",               struc_CfgBool   { true,            DEFAULT } );
   m_cfgString.insert("perl-prefix",             struc_CfgString { QString(),       DEFAULT } );

   // tab 3 - qt help
   m_cfgBool.insert("generate-qthelp",           struc_CfgBool   { false,           DEFAULT } );
   m_cfgString.insert("qch-file",                struc_CfgString { QString(),       DEFAULT } );
   m_cfgString.insert("qhp-namespace",           struc_CfgString { "org.doxypress.Project",  DEFAULT } );
   m_cfgString.insert("qhp-virtual-folder",      struc_CfgString { "doc",           DEFAULT } );
   m_cfgString.insert("qhp-cust-filter-name",    struc_CfgString { QString(),       DEFAULT } );
   m_cfgList.insert("qhp-cust-attrib",           struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgList.insert("qhp-sect-attrib",           struc_CfgList   { QStringList(),   DEFAULT } );
   m_cfgString.insert("qthelp-gen-path",         struc_CfgString { QString(),       DEFAULT } );

   // tab 3 - rtf
   m_cfgString.insert("rtf-output",              struc_CfgString { "rtf",           DEFAULT } );
   m_cfgBool.insert("rtf-compact",               struc_CfgBool   { false,           DEFAULT } );
   m_cfgBool.insert("rtf-hyperlinks",            struc_CfgBool   { false,           DEFAULT } );
   m_cfgString.insert("rtf-stylesheet",          struc_CfgString { QString(),       DEFAULT } );
   m_cfgString.insert("rtf-extension",           struc_CfgString { QString(),       DEFAULT } );
   m_cfgBool.insert("rtf-source-code",           struc_CfgBool   { false,           DEFAULT } );
   m_cfgEnum.insert("rtf-paper-type",            struc_CfgEnum   { "a4",            DEFAULT } );

   // tab 3 - xml
   m_cfgString.insert("xml-output",              struc_CfgString { "xml",           DEFAULT } );
   m_cfgBool.insert("xml-program-listing",       struc_CfgBool   { true,            DEFAULT } );
}

bool Config::read_ProjectFile(const QString &fName)
{
   // get existing json data
   QByteArray data = Config::json_ReadFile(fName);

   if (data.isEmpty()) {
      printf("Project file '%s' could not be read or contained no data\n", csPrintable(fName) );
      return false;
   }

   QJsonDocument doc  = QJsonDocument::fromJson(data);
   QJsonObject object = doc.object();

   // test json format
   int format;
   QJsonValue temp = object.value("doxypress-format");

   if (temp.isNull())  {
      format = 0;

   } else {
      format = temp.toInt();

   }

   if (format == 0)  {
      printf("DoxyPress project file is not a JSON format or is using an older project file format.\n"
         "Open your project file in DoxyPressApp, saving will automatically update the format.\n");

      return false;
   }

   QJsonObject topObj;

   for (auto iter = object.begin(); iter != object.end(); ++iter) {
      // walk the project file

      if (iter.value().isObject()) {
         topObj = iter.value().toObject();

      } else {
         // ignore entry since it is not a json object
         continue;
      }

      //
      for (auto iter2 = topObj.begin(); iter2 != topObj.end(); ++iter2) {

         QString key        = iter2.key();
         QJsonValue tempObj = iter2.value();

         if (key == "output-language" || key == "dot-image-format" || key == "mathjax-format" || key == "latex-paper-type" ||
                  key == "rtf-paper-type")  {

            auto hashIter = m_cfgEnum.find(key);

            if (hashIter != m_cfgEnum.end()) {
               hashIter.value() = { tempObj.toString(), PROJECT };

            }  else {
               fprintf(stderr, "Error: %s is an unknown enum configuration parameter and not "
                     "recognized by DoxyPress\n", csPrintable(key) );

            }

            continue;
         }

         if (tempObj.isBool()) {
            auto hashIter = m_cfgBool.find(key);

            if (hashIter != m_cfgBool.end()) {
               hashIter.value() = { tempObj.toBool(), PROJECT };

            }  else {
               fprintf(stderr, "Error: %s is an unknown boolean configuration parameter and not "
                     "recognized by DoxyPress\n", csPrintable(key) );

            }
         }

         if (tempObj.isDouble()) {
            auto hashIter = m_cfgInt.find(key);

            if (hashIter != m_cfgInt.end()) {
               hashIter.value() = { tempObj.toInt(), PROJECT };

            }  else {
               fprintf(stderr, "Error: %s is an unknown integer configuration parameter and not "
                     "recognized by DoxyPress\n", csPrintable(key) );

            }
         }

         if (tempObj.isArray()) {
            auto hashIter = m_cfgList.find(key);

            if (hashIter != m_cfgList.end()) {

               QJsonArray jsonArray = tempObj.toArray();
               QStringList listData;

               for (auto item : jsonArray) {
                  QString temp = item.toString();

                  if (! temp.isEmpty()) {
                     listData.append(temp.trimmed());
                  }
               }

               hashIter.value() = { listData, PROJECT };

            }  else {
               fprintf(stderr, "Error: %s is an unknown array configuration parameter and not "
                  "recognized by DoxyPress\n", csPrintable(key) );

            }
         }

         if (tempObj.isString()) {
            auto hashIter = m_cfgString.find(key);

            if (hashIter != m_cfgString.end()) {
               hashIter.value() = { tempObj.toString(), PROJECT };

            }  else {
               fprintf(stderr, "Error: %s is an unknown string configuration parameter and not "
                  "recognized by DoxyPress\n", csPrintable(key) );

            }
         }
      }
   }

   return true;
}
