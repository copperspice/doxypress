include copperspice.am

# dist
distdir_custom=source_dist/dist/DoxyPress-$(VERSION)

# path to gcc retrieved from configure (windows)
gccpath=$(dir $(shell which $(word 1,$(CC))))

# list of gcc libraries (windows)  sjlj  or seh
gcclibs=libgcc_s_*-1  libstdc++-6  libwinpthread-1

# list of CopperSpice libraries
cslibs=Core Xml
csver=1.5

# location of includes
AM_CXXFLAGS+=-I../src

if OSTYPE_BSD
   # path to clang
   clang_path=`llvm-config --prefix`

   clang_so=$(clang_path)/lib/libclang.so.*
   clang_hdr=$(clang_path)/include

   clang_link=$(shell llvm-config --ldflags) -Wl,--start-group \
        -lLLVMSupport \
        -lclangAST \
        -lclangBasic \
        -lclangTooling \
        -lclangStaticAnalyzerFrontend \
        -lclangFrontend \
        -lLLVMProfileData \
        -lclangDriver \
        -lclangParse \
        -lclangIndex \
        -lLLVMMCParser \
        -lclangSerialization \
        -lclangSema \
        -lclangEdit  \
        -lLLVMBitReader \
        -lLLVMOption \
        -lclangStaticAnalyzerCheckers \
        -lclangStaticAnalyzerCore \
        -lclangAnalysis \
        -lclangAST \
        -lclangToolingCore \
        -lclangLex \
        -lclangBasic \
        -lLLVMCore \
        -lLLVMBinaryFormat \
        -lLLVMMC \
        -lLLVMSupport \
        -Wl,--end-group \
        -lclang \
        $(shell llvm-config --system-libs) \
        '-Wl,-rpath,$$ORIGIN'

   AM_CXXFLAGS+=`llvm-config --cxxflags`
endif

if OSTYPE_DARWIN
   # path to clang
   clang_path=`llvm-config --prefix`

   clang_so=$(clang_path)/lib/libclang.so.*
   clang_hdr=$(clang_path)/include

   clang_link=$(shell llvm-config --ldflags) \
        -lLLVMSupport \
        -lclangAST \
        -lclangBasic \
        -lclangTooling \
        -lclangStaticAnalyzerFrontend \
        -lclangFrontend \
        -lLLVMProfileData \
        -lclangDriver \
        -lclangParse \
        -lclangIndex \
        -lLLVMMCParser \
        -lclangSerialization \
        -lclangSema \
        -lclangEdit  \
        -lLLVMBitReader \
        -lLLVMOption \
        -lclangStaticAnalyzerCheckers \
        -lclangStaticAnalyzerCore \
        -lclangAnalysis \
        -lclangAST \
        -lclangToolingCore \
        -lclangLex \
        -lclangBasic \
        -lLLVMCore \
        -lLLVMBinaryFormat \
        -lLLVMMC \
        -lLLVMSupport \
        -lLLVMDemangle \
        -lclang \
        $(shell llvm-config --system-libs) \
        '-Wl,-rpath,$$ORIGIN'

   AM_CXXFLAGS+=`llvm-config --cxxflags`
endif

if OSTYPE_LINUX
   # path to clang
   clang_path=`llvm-config --prefix`

   clang_so=$(clang_path)/lib/libclang.so.*
   clang_hdr=$(clang_path)/include

   clang_link=$(shell llvm-config --ldflags) -Wl,--start-group \
        -lLLVMSupport \
        -lclangAST \
        -lclangBasic \
        -lclangTooling \
        -lclangStaticAnalyzerFrontend \
        -lclangFrontend \
        -lLLVMProfileData \
        -lclangDriver \
        -lclangParse \
        -lclangIndex \
        -lLLVMMCParser \
        -lclangSerialization \
        -lclangSema \
        -lclangEdit  \
        -lLLVMBitReader \
        -lLLVMOption \
        -lclangStaticAnalyzerCheckers \
        -lclangStaticAnalyzerCore \
        -lclangAnalysis \
        -lclangAST \
        -lclangToolingCore \
        -lclangLex \
        -lclangBasic \
        -lLLVMCore \
        -lLLVMBinaryFormat \
        -lLLVMMC \
        -lLLVMSupport \
        -Wl,--end-group \
        -lclang \
        $(shell llvm-config --system-libs) \
        '-Wl,-rpath,$$ORIGIN'

   AM_CXXFLAGS+=`llvm-config --cxxflags`
endif

if OSTYPE_WIN
   # path to clang
   clang_path=../clang_lib

   clang_dll=$(clang_path)/bin/libclang.dll
   clang_hdr=$(clang_path)/include

   clang_link=-L$(clang_path)/lib  -Wl,--start-group  \
        -lLLVMSupport \
        -lclangAST \
        -lclangBasic \
        -lclangTooling \
        -lclangStaticAnalyzerFrontend \
        -lclangFrontend \
        -lLLVMProfileData \
        -lclangDriver \
        -lclangParse \
        -lclangIndex \
        -lLLVMMCParser \
        -lclangSerialization \
        -lclangSema \
        -lclangEdit  \
        -lLLVMBitReader \
        -lLLVMOption \
        -lclangStaticAnalyzerCheckers \
        -lclangStaticAnalyzerCore \
        -lclangAnalysis \
        -lclangAST \
        -lclangToolingCore \
        -lclangLex \
        -lclangBasic \
        -lLLVMCore \
        -lLLVMBinaryFormat \
        -lLLVMMC \
        -lLLVMSupport \
        -Wl,--end-group \
        -llibclang

   AM_CXXFLAGS+=-I$(clang_hdr)

   # location for 'make install'
   installdir=$(abs_top_builddir)/install
endif

# target name
bin_PROGRAMS = doxypress

# search path for source
VPATH = ../

# linking
doxypress_LDFLAGS=$(AM_LDFLAGS) $(patsubst %,-lCs%$(csver),$(cslibs)) $(clang_link)

if OSTYPE_BSD
   doxypress_LDFLAGS+= -lz
endif

if OSTYPE_DARWIN
   doxypress_LDFLAGS+= -lz
endif

if OSTYPE_LINUX
   doxypress_LDFLAGS+= -lz
endif

if OSTYPE_WIN
   doxypress_LDFLAGS+= -lole32 -luuid -lversion -lz -ladvapi32
endif

doxypress_SOURCES = \
   src/doxy_work.cpp \
   src/doxy_globals.cpp \
   src/doxy_setup.cpp \
   src/a_define.cpp \
   src/arguments.cpp \
   src/cite.cpp \
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
   src/filenamelist.cpp \
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
   src/latexdocvisitor.cpp \
   src/latexgen.cpp  \
   src/layout.cpp \
   src/lodepng.cpp \
   src/logos.cpp \
   src/main.cpp \
   src/mandocvisitor.cpp \
   src/mangen.cpp \
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
   src/parse_clang.cpp \
   src/parse_lib_tooling.cpp \
   src/parse_file.cpp \
   src/parse_make.cpp \
   src/parse_md.cpp \
   src/pagedef.cpp \
   src/perlmodgen.cpp \
   src/plantuml.cpp \
   src/qhp.cpp \
   src/qhpxmlwriter.cpp \
   src/reflist.cpp \
   src/resourcemgr.cpp \
   src/sortedlist.cpp \
   src/searchindex.cpp \
   src/stringmap.cpp \
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
   src/ce_parse.cpp \
   src/generated/code_cstyle.cpp  \
   src/generated/code_fortran.cpp \
   src/generated/code_py.cpp  \
   src/generated/code_xml.cpp  \
   src/generated/commentcnv.cpp  \
   src/generated/commentscan.cpp  \
   src/generated/constexp.cpp \
   src/generated/default_args.cpp \
   src/generated/declinfo.cpp  \
   src/generated/doctokenizer.cpp \
   src/generated/parse_cstyle.cpp \
   src/generated/parse_fortran.cpp \
   src/generated/parse_py.cpp \
   src/generated/parse_tcl.cpp \
   src/generated/pre.cpp

noinst_HEADERS = \
   src/a_define.h \
   src/arguments.h \
   src/ce_parse.h \
   src/cite.h \
   src/classdef.h \
   src/classlist.h \
   src/cmdmapper.h \
   src/code_cstyle.h \
   src/code_cstyle_internal.h \
   src/code_xml.h \
   src/commentcnv.h \
   src/commentscan.h \
   src/condparser.h \
   src/config.h \
   src/constexp.h \
   src/cppvalue.h \
   src/declinfo.h \
   src/default_args.h \
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
   src/filenamelist.h \
   src/formula.h \
   src/ftvhelp.h \
   src/groupdef.h \
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
   src/parse_base.h \
   src/parse_clang.h \
   src/parse_lib_tooling.h \
   src/parse_cstyle.h \
   src/parse_file.h \
   src/parse_fortran.h \
   src/parse_make.h \
   src/parse_md.h \
   src/parse_py.h \
   src/parse_tcl.h \
   src/parse_xml.h \
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
   src/sortedlist.h \
   src/sortedlist_fwd.h \
   src/stringmap.h \
   src/tagreader.h \
   src/textdocvisitor.h \
   src/tooltip.h \
   src/translator.h \
   src/translator_cs.h \
   src/types.h \
   src/util.h \
   src/xmldocvisitor.h \
   src/xmlgen.h

TRANSLATIONS =  \
    resources/translation/doxy_en.ts \
    resources/translation/doxy_af.ts \
    resources/translation/doxy_ar.ts \
    resources/translation/doxy_am.ts \
    resources/translation/doxy_pt_br.ts \
    resources/translation/doxy_ca.ts \
    resources/translation/doxy_zh.ts \
    resources/translation/doxy_zh_tw.ts \
    resources/translation/doxy_hr.ts \
    resources/translation/doxy_cz.ts \
    resources/translation/doxy_da.ts \
    resources/translation/doxy_nl.ts \
    resources/translation/doxy_eo.ts \
    resources/translation/doxy_fa.ts \
    resources/translation/doxy_fi.ts \
    resources/translation/doxy_fr.ts \
    resources/translation/doxy_de.ts \
    resources/translation/doxy_el.ts \
    resources/translation/doxy_hu.ts \
    resources/translation/doxy_it.ts \
    resources/translation/doxy_id.ts \
    resources/translation/doxy_ja.ts \
    resources/translation/doxy_ja_en.ts \
    resources/translation/doxy_ko.ts \
    resources/translation/doxy_ko_en.ts \
    resources/translation/doxy_lv.ts \
    resources/translation/doxy_lt.ts \
    resources/translation/doxy_mk.ts \
    resources/translation/doxy_nb.ts \
    resources/translation/doxy_pl.ts \
    resources/translation/doxy_pt.ts \
    resources/translation/doxy_ro.ts \
    resources/translation/doxy_ru.ts \
    resources/translation/doxy_sr.ts \
    resources/translation/doxy_sr_cyrillic.ts \
    resources/translation/doxy_sk.ts \
    resources/translation/doxy_sl.ts \
    resources/translation/doxy_es.ts \
    resources/translation/doxy_sv.ts \
    resources/translation/doxy_tr.ts \
    resources/translation/doxy_uk.ts \
    resources/translation/doxy_vi.ts

RESOURCES = doxypress.qrc

# use wilcard to obtain the dependency list
qrc_doxypress.cpp: doxypress.qrc   $(shell find ../resources -type f)

# required to compile qrc
nodist_doxypress_SOURCES=qrc_doxypress.cpp


# deployment rules
.PHONY: deploy

if OSTYPE_BSD
deploy : all
	mkdir -p $(deploydir)
	mkdir -p $(deploydir)/include/

	$(LIBTOOL) --mode=install cp $(bin_PROGRAMS) $(deploydir)

	rsync -W -a $(patsubst %,$(CS_HOME)/lib/libCs%$(csver).so*,$(cslibs)) $(deploydir)

	rsync -W -a --modify-window=2  $(clang_so)         $(deploydir)
	rsync -W -a --modify-window=2  $(clang_hdr)/6.0.1  $(deploydir)/include/
endif

if OSTYPE_DARWIN
	mkdir -p $(deploydir)
	mkdir -p $(deploydir)/include/

	$(LIBTOOL) --mode=install cp $(bin_PROGRAMS) $(deploydir)

	rsync -W -a $(patsubst %,$(CS_HOME)/lib/libCs%$(csver).so*,$(cslibs)) $(deploydir)

	rsync -W -a --modify-window=2  $(clang_so)         $(deploydir)
	rsync -W -a --modify-window=2  $(clang_hdr)/6.0.1  $(deploydir)/include/
endif

if OSTYPE_LINUX
deploy : all
	mkdir -p $(deploydir)
	mkdir -p $(deploydir)/include/

	$(LIBTOOL) --mode=install cp $(bin_PROGRAMS) $(deploydir)

	rsync -W -a $(patsubst %,$(CS_HOME)/lib/libCs%$(csver).so*,$(cslibs)) $(deploydir)

	rsync -W -a --modify-window=2  $(clang_so)         $(deploydir)
	rsync -W -a --modify-window=2  $(clang_hdr)/6.0.1  $(deploydir)/include/
endif

if OSTYPE_WIN
deploy : all
	mkdir -p $(deploydir)
	mkdir -p $(deploydir)/include/

	$(LIBTOOL) --mode=install cp $(bin_PROGRAMS) $(deploydir)

	rsync -W -a --modify-window=2  $(patsubst %,$(gccpath)%.dll,$(gcclibs))  $(deploydir)
	rsync -W -a --modify-window=2  $(patsubst %,$(CS_HOME)/bin/libCs%$(csver).dll,$(cslibs))  $(deploydir)

	rsync -W -a --modify-window=2  $(clang_dll)        $(deploydir)
	rsync -W -a --modify-window=2  $(clang_hdr)/6.0.1  $(deploydir)/include/
endif


# soruce distribution rules
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
	rm  -f   ../DoxyPress-$(VERSION).zip          ; \
	zip -r   ../DoxyPress-$(VERSION).zip .        ; \
	rm  -f   ../DoxyPress-$(VERSION).tar.bz2      ; \
	tar -cjf ../DoxyPress-$(VERSION).tar.bz2 .


# create test_project zip file
dist-test-project:
	cd ..  ; \
\
	rm  -f   DoxyPress-Test-Project-$(VERSION).zip         ; \
	zip -r   DoxyPress-Test-Project-$(VERSION).zip test_build/{input_test,input_clang,images,*.css,*.html,*.json,*.xml}


