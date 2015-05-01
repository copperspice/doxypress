include copperspice.am

# dist
distdir_custom=source_dist/dist/DoxyPress-$(VERSION)

# path to gcc (windows)
gccpath=/c/MingW/bin

# list of gcc libraries (windows)
gcclibs=libgcc_s_sjlj-1  libstdc++-6  libwinpthread-1

# list of CopperSpice libraries
cslibs=Core Xml

# location of includes
AM_CXXFLAGS+=-I../src

if OSTYPE_LINUX
endif

if OSTYPE_DARWIN
endif

if OSTYPE_WIN
   # path to clang
   clang_path=../clang_lib

   clang_dll=$(clang_path)/bin/libclang.dll
   clang_hdr=$(clang_path)/include
   clang_lib=$(clang_path)/lib/libclang.dll.a

   clang_link="-L $(clang_path)/lib -lclang"

   AM_CXXFLAGS+=-I$(clang_hdr)

   # location for 'make install'
   installdir=$(abs_top_builddir)/install
endif

# target app name
bin_PROGRAMS = doxypress

# copy required CopperSpice and gcc files for deployment
.PHONY: deploy

if OSTYPE_LINUX
deploy : all
	mkdir -p $(deploydir)

	$(LIBTOOL) --mode=install cp $(bin_PROGRAMS) $(deploydir)

	rsync -W -a $(patsubst %,$(CS_HOME)/lib/libCs%1.so*,$(cslibs)) $(deploydir)	
endif

if OSTYPE_DARWIN
endif

if OSTYPE_WIN
deploy : all
	mkdir -p $(deploydir)

	$(LIBTOOL) --mode=install cp $(bin_PROGRAMS) $(deploydir)

	rsync -W -a --modify-window=2  $(patsubst %,$(gccpath)/%.dll,$(gcclibs))  $(deploydir)
	rsync -W -a --modify-window=2  $(patsubst %,$(CS_HOME)/bin/libCs%1-0.dll,$(cslibs))  $(deploydir)
	rsync -W -a --modify-window=2  $(clang_dll)  $(deploydir)
endif

# search path for source
VPATH = ../

# linking
doxypress_LDFLAGS=$(AM_LDFLAGS) $(patsubst %,-lCs%1,$(cslibs)) $(clang_link)

if OSTYPE_WIN
   doxypress_LDFLAGS+= -lole32
endif

doxypress_SOURCES = \
   src/doxy_work.cpp \
   src/doxy_globals.cpp \
   src/doxy_setup.cpp \
   src/a_define.cpp \
   src/arguments.cpp \
   src/cite.cpp \
   src/clangparser.cpp \
   src/classdef.cpp \
   src/classlist.cpp \
   src/config.cpp    \
   src/config_json.cpp \
   src/cmdmapper.cpp \
   src/condparser.cpp \
   src/cppvalue.cpp \
   src/definition.cpp \
   src/dia.cpp \
   src/diagram.cpp \
   src/dirdef.cpp \
   src/docparser.cpp \
   src/docbookgen.cpp  \
   src/docbookvisitor.cpp \
   src/docsets.cpp \
   src/dot.cpp \
   src/eclipsehelp.cpp \
   src/entry.cpp \
   src/filedef.cpp \
   src/filename.cpp \
   src/formula.cpp \
   src/ftvhelp.cpp \
   src/groupdef.cpp \
   src/htags.cpp \
   src/htmldocvisitor.cpp \
   src/htmlentity.cpp \
   src/htmlgen.cpp \
   src/htmlhelp.cpp \
   src/image.cpp \
   src/index.cpp \
   src/language.cpp \
   src/latexdocvisitor.cpp \
   src/latexgen.cpp  \
   src/layout.cpp \
   src/lodepng.cpp \
   src/logos.cpp \
   src/main.cpp \
   src/mandocvisitor.cpp \
   src/mangen.cpp \
   src/markdown.cpp \
   src/marshal.cpp \
   src/memberdef.cpp \
   src/membergroup.cpp \
   src/memberlist.cpp \
   src/membername.cpp \
   src/message.cpp \
   src/msc.cpp \
   src/namespacedef.cpp \
   src/objcache.cpp \
   src/outputgen.cpp \
   src/outputlist.cpp \
   src/parser_file.cpp \
   src/parser_make.cpp \
   src/pagedef.cpp \
   src/perlmodgen.cpp \
   src/plantuml.cpp \
   src/qhp.cpp \
   src/qhpxmlwriter.cpp \
   src/reflist.cpp \
   src/resourcemgr.cpp \
   src/sortedlist.cpp \
   src/searchindex.cpp \
   src/sqlite3gen.cpp \
   src/store.cpp \
   src/tagreader.cpp \
   src/textdocvisitor.cpp \
   src/tooltip.cpp \
   src/util.cpp \
   src/rtfdocvisitor.cpp \
   src/rtfgen.cpp \
   src/rtfstyle.cpp  \
   src/xmldocvisitor.cpp  \
   src/xmlgen.cpp  \
   src/portable.cpp  \
   src/ce_parse.cpp  \
   src/generated/code.cpp  \
   src/generated/commentcnv.cpp  \
   src/generated/commentscan.cpp  \
   src/generated/constexp.cpp \
   src/generated/defargs.cpp \
   src/generated/declinfo.cpp  \
   src/generated/doctokenizer.cpp \
   src/generated/fortrancode.cpp \
   src/generated/parser_cpp.cpp \
   src/generated/parser_fortran.cpp \
   src/generated/parser_py.cpp \
   src/generated/parser_tcl.cpp \
   src/generated/pre.cpp \
   src/generated/pycode.cpp

noinst_HEADERS = \
   src/a_define.h \
   src/arguments.h \
   src/bufstr.h \
   src/ce_parse.h \
   src/cite.h \
   src/clangparser.h \
   src/classdef.h \
   src/classlist.h \
   src/cmdmapper.h \
   src/code.h \
   src/commentcnv.h \
   src/commentscan.h \
   src/condparser.h \
   src/config.h \
   src/constexp.h \
   src/cppvalue.h \
   src/dbusxmlscanner.h \
   src/declinfo.h \
   src/defargs.h \
   src/definition.h \
   src/dia.h \
   src/diagram.h \
   src/dirdef.h \
   src/docbookgen.h \
   src/docbookvisitor.h \
   src/docparser.h \
   src/docsets.h \
   src/doctokenizer.h \
   src/docvisitor.h \
   src/dot.h \
   src/doxy_build_info.h \
   src/doxy_globals.h \
   src/doxy_setup.h \
   src/doxy_shared.h \
   src/eclipsehelp.h \
   src/entry.h \
   src/example.h \
   src/filedef.h \
   src/filelist.h \
   src/filename.h \
   src/filestorage.h \
   src/formula.h \
   src/fortrancode.h \
   src/ftvhelp.h \
   src/groupdef.h \
   src/growbuf.h \
   src/htags.h \
   src/htmlattrib.h \
   src/htmldocvisitor.h \
   src/htmlentity.h \
   src/htmlgen.h \
   src/htmlhelp.h \
   src/image.h \
   src/index.h \
   src/language.h \
   src/latexdocvisitor.h \
   src/latexgen.h \
   src/layout.h \
   src/lodepng.h \
   src/logos.h \
   src/mandocvisitor.h \
   src/mangen.h \
   src/markdown.h \
   src/marshal.h \
   src/memberdef.h \
   src/membergroup.h \
   src/memberlist.h \
   src/membername.h \
   src/message.h \
   src/msc.h \
   src/namespacedef.h \
   src/objcache.h \
   src/outputgen.h \
   src/outputlist.h \
   src/pagedef.h \
   src/parser_cpp.h \
   src/parser_file.h \
   src/parser_fortran.h \
   src/parser_make.h \
   src/parser_py.h \
   src/parser_tcl.h \
   src/parserintf.h \
   src/perlmodgen.h \
   src/plantuml.h \
   src/portable.h \
   src/pre.h \
   src/printdocvisitor.h \
   src/qhp.h \
   src/qhpxmlwriter.h \
   src/reflist.h \
   src/resourcemgr.h \
   src/rtfdocvisitor.h \
   src/rtfgen.h \
   src/rtfstyle.h \
   src/searchindex.h \
   src/section.h \
   src/settings.h \
   src/sortedlist.h \
   src/sqlite3gen.h \
   src/store.h \
   src/stringmap.h \
   src/tagreader.h \
   src/textdocvisitor.h \
   src/tooltip.h \
   src/translator.h \
   src/translator_en.h \
   src/types.h \
   src/util.h \
   src/xmldocvisitor.h \
   src/xmlgen.h

noinst_HEADERS += \
   src/translate/translator_adapter.h \
   src/translate/translator_am.h \
   src/translate/translator_ar.h \
   src/translate/translator_br.h \
   src/translate/translator_ca.h \
   src/translate/translator_cn.h \
   src/translate/translator_cz.h \
   src/translate/translator_de.h \
   src/translate/translator_dk.h \
   src/translate/translator_eo.h \
   src/translate/translator_es.h \
   src/translate/translator_fa.h \
   src/translate/translator_fi.h \
   src/translate/translator_fr.h \
   src/translate/translator_gr.h \
   src/translate/translator_hr.h \
   src/translate/translator_hu.h \
   src/translate/translator_id.h \
   src/translate/translator_it.h \
   src/translate/translator_je.h \
   src/translate/translator_jp.h \
   src/translate/translator_ke.h \
   src/translate/translator_kr.h \
   src/translate/translator_lt.h \
   src/translate/translator_lv.h \
   src/translate/translator_mk.h \
   src/translate/translator_nl.h \
   src/translate/translator_no.h \
   src/translate/translator_pl.h \
   src/translate/translator_pt.h \
   src/translate/translator_ro.h \
   src/translate/translator_ru.h \
   src/translate/translator_sc.h \
   src/translate/translator_si.h \
   src/translate/translator_sk.h \
   src/translate/translator_sr.h \
   src/translate/translator_sv.h \
   src/translate/translator_tr.h \
   src/translate/translator_tw.h \
   src/translate/translator_ua.h \
   src/translate/translator_vi.h \
   src/translate/translator_za.h

# not currently using
# src/dbusxmlscanner.cpp

RESOURCES = doxypress.qrc

# use wilcard to obtain just the dependency list
qrc_doxypress.cpp: doxypress.qrc   $(wildcard ../resources/*)

# required to compile qrc
nodist_doxypress_SOURCES=qrc_doxypress.cpp

# distribution
distdir: $(DISTFILES)
	cd .. ; \
	test -d "$(distdir_custom)" || mkdir -p "$(distdir_custom)" ; \
	mkdir -p $(distdir_custom)/bin                              ; \
	mkdir -p $(distdir_custom)/resources                        ; \
\
	for f in $(DIST_SOURCES)  ; do install -D $$f $(distdir_custom)/$$f ; done   ;\
	for f in $(HEADERS)       ; do install -D $$f $(distdir_custom)/$$f ; done   ;\
	for f in $(FORMS)         ; do install -D $$f $(distdir_custom)/$$f ; done   ;\
\
	cp doxypress.qrc                 $(distdir_custom)/                          ; \
	cp bin/*.am                      $(distdir_custom)/bin/                      ; \
	cp bin/Makefile.in               $(distdir_custom)/bin/                      ; \
	cp -r resources/*                $(distdir_custom)/resources/                ;

	$(MAKE) $(AM_MAKEFLAGS) dist-hook

# distdir is a temp folder
dist-hook:
	cd ..  ; \
\
	cp -r bin/$(distdir)/../*   $(distdir_custom) ; \
	cd $(distdir_custom)/..                       ; \
\
	rm  -f   ../DoxyPress-$(VERSION).zip         ; \
	zip -r   ../DoxyPress-$(VERSION).zip .       ; \
	rm  -f   ../DoxyPress-$(VERSION).tar.bz2     ; \
	tar -cjf ../DoxyPress-$(VERSION).tar.bz2 .


# create test_project zip file
dist-test-project:
	cd ..  ; \
\
	rm  -f   doxypress_test-project-$(VERSION).zip         ; \
	zip -r   doxypress-test-project-$(VERSION).zip test_build/{dox,images,*.css,*.html,*.json,*.xml}


