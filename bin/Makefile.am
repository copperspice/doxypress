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
	rsync -W -a $(top_srcdir)/resources/doxypress.png $(deploydir)
endif

if OSTYPE_DARWIN
deployapp=$(deploydir)/$(bin_PROGRAMS).app/Contents

deploy : all
	mkdir -p $(deployapp)/Frameworks
	mkdir -p $(deployapp)/MacOS
	mkdir -p $(deployapp)/Resources

	$(LIBTOOL) --mode=install cp $(bin_PROGRAMS) $(deployapp)/MacOS

	rm -f $(deployapp)/Info.plist
	sed -e "s,@ICON@,doxypres.icns,g" \
	    -e "s,@TYPEINFO@,????,g" \
	    -e "s,@EXECUTABLE@,$(bin_PROGRAMS),g" \
	    -e "s,com.yourcompany.,com.copperspice.,g" \
	$(CS_HOME)/mac/Info.plist.app > $(deployapp)/Info.plist

	rm -f $(deployapp)/PkgInfo
	echo "APPL????" > $(deployapp)/PkgInfo

	touch $(deployapp)/Resources/empty.lproj
	cp -r $(CS_HOME)/mac/qt_menu.nib $(deployapp)/Resources

	cp $(top_srcdir)/resources/doxypres.icns $(deployapp)/Resources
	
	rsync -W -a $(patsubst %,$(CS_HOME)/lib/libCs%1.0.dylib,$(cslibs)) $(deployapp)/Frameworks

	for libname in $(patsubst %,libCs%1.0.dylib,$(cslibs)) ; do \
	\
	install_name_tool -change $(CS_HOME)/lib/$$libname \
	@executable_path/../Frameworks/$$libname  $(deployapp)/MacOS/$(bin_PROGRAMS); \
	\
	for templib in $(patsubst %,libCs%1.0.dylib,$(cslibs)) ; do \
	install_name_tool -change $(CS_HOME)/lib/$$templib \
	@executable_path/../Frameworks/$$templib  $(deployapp)/Frameworks/$$libname \
	; done \
	\
	; done

   # generate dmg	
	hdiutil create -ov -srcfolder $(deploydir) -format UDBZ -volname $(bin_PROGRAMS) ../$(bin_PROGRAMS).dmg
	hdiutil internet-enable -yes ../$(bin_PROGRAMS).dmg
endif

if OSTYPE_WIN
deploy : all
	mkdir -p $(deploydir)

	$(LIBTOOL) --mode=install cp $(bin_PROGRAMS) $(deploydir)

	rsync -W -a --modify-window=2  $(top_srcdir)/resources/doxypress.ico $(deploydir)

	rsync -W -a --modify-window=2  $(patsubst %,$(gccpath)/%.dll,$(gcclibs))  $(deploydir)
	rsync -W -a --modify-window=2  $(patsubst %,$(CS_HOME)/bin/libCs%1-0.dll,$(cslibs))  $(deploydir)
endif

# search path for source
VPATH = ../

# linking
doxypress_LDFLAGS=$(AM_LDFLAGS) $(patsubst %,-lCs%1,$(cslibs))

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
   src/latexdocvisitor.cpp  \
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
   src/generated/fortranscanner.cpp \
   src/generated/pre.cpp \
   src/generated/pycode.cpp  \
   src/generated/pyscanner.cpp \
   src/generated/scanner.cpp \
   src/generated/tclscanner.cpp

# not currently using
# src/dbusxmlscanner.cpp

RESOURCES = doxypress.qrc

# use wilcard to obtain just the dependency list
qrc_doxypress.cpp: doxypress.qrc   $(wildcard ../resources/*)

# required to compile qrc
nodist_doxypress_SOURCES=qrc_doxypress.cpp

# install
if OSTYPE_LINUX
endif

if OSTYPE_DARWIN
endif

if OSTYPE_WIN
endif

# distribution
distdir: $(DISTFILES)

# distdir is a temp folder
dist-hook:

# create test_project zip file
dist-test-project:
	cd ..  ; \
\
	rm  -f   doxypress_test-project-$(VERSION).zip         ; \
	zip -r   doxypress-test-project-$(VERSION).zip test_build/{dox,images,*.css,*.html,*.json,*.xml}


