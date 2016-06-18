
if (WIN32)
   # (todo: enhance) hard coded for testing
   set(CS_INCLUDE_DIR "z:/cs5_x_lib/include" "z:/cs5_x_lib/include/QtCore" "z:/cs5_x_lib/include/QtXml")

   set(CLANG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/clang_lib/include)
   set(CLANG_LIB_DIR     ${CMAKE_SOURCE_DIR}/clang_lib/lib)
endif()

include_directories( ${CMAKE_SOURCE_DIR}/src
                     ${CS_INCLUDE_DIR}
                     ${CLANG_INCLUDE_DIR}
)

check_include_file(windows.h HAVE_WINDOWS_H)

if (HAVE_WINDOWS_H)
   add_definitions(-DHAVE_WINDOWS_H)
endif()

set(DOXYPRESS_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/a_define.h
   ${CMAKE_CURRENT_SOURCE_DIR}/arguments.h
   ${CMAKE_CURRENT_SOURCE_DIR}/ce_parse.h
   ${CMAKE_CURRENT_SOURCE_DIR}/cite.h
   ${CMAKE_CURRENT_SOURCE_DIR}/classdef.h
   ${CMAKE_CURRENT_SOURCE_DIR}/classlist.h
   ${CMAKE_CURRENT_SOURCE_DIR}/cmdmapper.h
   ${CMAKE_CURRENT_SOURCE_DIR}/code_cstyle..h
   ${CMAKE_CURRENT_SOURCE_DIR}/commentcnv.h
   ${CMAKE_CURRENT_SOURCE_DIR}/commentscan.h
   ${CMAKE_CURRENT_SOURCE_DIR}/condparser.h
   ${CMAKE_CURRENT_SOURCE_DIR}/config.h
   ${CMAKE_CURRENT_SOURCE_DIR}/constexp.h
   ${CMAKE_CURRENT_SOURCE_DIR}/cppvalue.h
   ${CMAKE_CURRENT_SOURCE_DIR}/declinfo.h
   ${CMAKE_CURRENT_SOURCE_DIR}/defargs.h
   ${CMAKE_CURRENT_SOURCE_DIR}/definition.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dia.h
   ${CMAKE_CURRENT_SOURCE_DIR}/diagram.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dirdef.h
   ${CMAKE_CURRENT_SOURCE_DIR}/docbookgen.h
   ${CMAKE_CURRENT_SOURCE_DIR}/docbookvisitor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/docparser.h
   ${CMAKE_CURRENT_SOURCE_DIR}/docsets.h
   ${CMAKE_CURRENT_SOURCE_DIR}/doctokenizer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/docvisitor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dot.h
   ${CMAKE_CURRENT_SOURCE_DIR}/doxy_build_info.h
   ${CMAKE_CURRENT_SOURCE_DIR}/doxy_globals.h
   ${CMAKE_CURRENT_SOURCE_DIR}/doxy_setup.h
   ${CMAKE_CURRENT_SOURCE_DIR}/doxy_shared.h
   ${CMAKE_CURRENT_SOURCE_DIR}/eclipsehelp.h
   ${CMAKE_CURRENT_SOURCE_DIR}/entry.h
   ${CMAKE_CURRENT_SOURCE_DIR}/example.h
   ${CMAKE_CURRENT_SOURCE_DIR}/filedef.h
   ${CMAKE_CURRENT_SOURCE_DIR}/filenamelist.h
   ${CMAKE_CURRENT_SOURCE_DIR}/filestorage.h
   ${CMAKE_CURRENT_SOURCE_DIR}/formula.h
   ${CMAKE_CURRENT_SOURCE_DIR}/ftvhelp.h
   ${CMAKE_CURRENT_SOURCE_DIR}/groupdef.h
   ${CMAKE_CURRENT_SOURCE_DIR}/htags.h
   ${CMAKE_CURRENT_SOURCE_DIR}/htmlattrib.h
   ${CMAKE_CURRENT_SOURCE_DIR}/htmldocvisitor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/htmlentity.h
   ${CMAKE_CURRENT_SOURCE_DIR}/htmlgen.h
   ${CMAKE_CURRENT_SOURCE_DIR}/htmlhelp.h
   ${CMAKE_CURRENT_SOURCE_DIR}/image.h
   ${CMAKE_CURRENT_SOURCE_DIR}/index.h
   ${CMAKE_CURRENT_SOURCE_DIR}/language.h
   ${CMAKE_CURRENT_SOURCE_DIR}/latexdocvisitor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/latexgen.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lodepng.h
   ${CMAKE_CURRENT_SOURCE_DIR}/logos.h
   ${CMAKE_CURRENT_SOURCE_DIR}/mandocvisitor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/mangen.h
   ${CMAKE_CURRENT_SOURCE_DIR}/marshal.h
   ${CMAKE_CURRENT_SOURCE_DIR}/memberdef.h
   ${CMAKE_CURRENT_SOURCE_DIR}/membergroup.h
   ${CMAKE_CURRENT_SOURCE_DIR}/memberlist.h
   ${CMAKE_CURRENT_SOURCE_DIR}/membername.h
   ${CMAKE_CURRENT_SOURCE_DIR}/message.h
   ${CMAKE_CURRENT_SOURCE_DIR}/msc.h
   ${CMAKE_CURRENT_SOURCE_DIR}/namespacedef.h
   ${CMAKE_CURRENT_SOURCE_DIR}/objcache.h
   ${CMAKE_CURRENT_SOURCE_DIR}/outputgen.h
   ${CMAKE_CURRENT_SOURCE_DIR}/outputlist.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pagedef.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parse_base.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parse_clang.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parse_lib_tooling.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parse_cstyle.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parse_file.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parse_fortran.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parse_make.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parse_md.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parse_py.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parse_tcl.h
   ${CMAKE_CURRENT_SOURCE_DIR}/perlmodgen.h
   ${CMAKE_CURRENT_SOURCE_DIR}/plantuml.h
   ${CMAKE_CURRENT_SOURCE_DIR}/portable.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pre.h
   ${CMAKE_CURRENT_SOURCE_DIR}/printdocvisitor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qhp.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qhpxmlwriter.h
   ${CMAKE_CURRENT_SOURCE_DIR}/reflist.h
   ${CMAKE_CURRENT_SOURCE_DIR}/resourcemgr.h
   ${CMAKE_CURRENT_SOURCE_DIR}/rtfdocvisitor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/rtfgen.h
   ${CMAKE_CURRENT_SOURCE_DIR}/rtfstyle.h
   ${CMAKE_CURRENT_SOURCE_DIR}/searchindex.h
   ${CMAKE_CURRENT_SOURCE_DIR}/section.h
   ${CMAKE_CURRENT_SOURCE_DIR}/sortedlist.h
   ${CMAKE_CURRENT_SOURCE_DIR}/sortedlist_fwd.h
   ${CMAKE_CURRENT_SOURCE_DIR}/store.h
   ${CMAKE_CURRENT_SOURCE_DIR}/stringmap.h
   ${CMAKE_CURRENT_SOURCE_DIR}/tagreader.h
   ${CMAKE_CURRENT_SOURCE_DIR}/textdocvisitor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/tooltip.h
   ${CMAKE_CURRENT_SOURCE_DIR}/translator.h
   ${CMAKE_CURRENT_SOURCE_DIR}/translator_cs.h
   ${CMAKE_CURRENT_SOURCE_DIR}/types.h
   ${CMAKE_CURRENT_SOURCE_DIR}/util.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xmldocvisitor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xmlgen.h
)

set(DOXYPRESS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/doxy_work.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/doxy_globals.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/doxy_setup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/a_define.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/arguments.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cite.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/classdef.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/classlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/config_json.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cmdmapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/condparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cppvalue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/definition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dia.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/diagram.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dirdef.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/docparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/docbookgen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/docbookvisitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/docsets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eclipsehelp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/entry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filedef.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filenamelist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/formula.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ftvhelp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/groupdef.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/htags.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/htmldocvisitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/htmlentity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/htmlgen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/htmlhelp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/index.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/latexdocvisitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/latexgen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/layout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lodepng.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mandocvisitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mangen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/marshal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/memberdef.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/membergroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/memberlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/membername.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/message.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/msc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/namespacedef.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/objcache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/outputgen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/outputlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parse_clang.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parse_lib_tooling.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parse_file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parse_make.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parse_md.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pagedef.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/perlmodgen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/plantuml.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qhp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qhpxmlwriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/reflist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resourcemgr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sortedlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/searchindex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/store.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stringmap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tagreader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/textdocvisitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tooltip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtfdocvisitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtfgen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtfstyle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xmldocvisitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xmlgen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/portable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ce_parse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/code_cstyle..cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/code_fortran.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/code_py.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/commentcnv.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/commentscan.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/constexp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/defargs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/declinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/doctokenizer.cpp    
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/parse_cstyle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/parse_fortran.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/parse_py.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/parse_tcl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/pre.cpp  
)

# (todo: enhance) run rcc to generate qrc output
set(DOXYPRESS_SOURCES  ${DOXYPRESS_SOURCES}
                       ${CMAKE_CURRENT_SOURCE_DIR}/../doxypress.qrc
)

add_executable(doxypress
       ${DOXYPRESS_SOURCES}
       ${doxypress_RESOURCES_RCC}
)

# (todo: enhance)
# target_link_libraries(doxypress CopperSpice::CsCore CopperSpice::CsXml ${CLANG_LIB_DIR} )

target_link_libraries(doxypress

   # (todo: enhance)
   "z:/cs5_x_lib/lib/libCsCore1.dll.a"
   "z:/cs5_x_lib/lib/libCsXml1.dll.a"

   -L${CLANG_LIB_DIR}
   -Wl,--start-group
   -lLLVMSupport
   -lclangAST
   -lclangBasic
   -lclangTooling
   -lclangStaticAnalyzerFrontend
   -lclangFrontend
   -lclangDriver
   -lclangParse
   -lclangIndex
   -lLLVMMCParser
   -lclangSerialization
   -lclangSema
   -lclangEdit
   -lLLVMBitReader
   -lLLVMOption
   -lclangStaticAnalyzerCheckers
   -lclangStaticAnalyzerCore
   -lclangAnalysis
   -lclangAST
   -lclangToolingCore
   -lclangLex
   -lclangBasic
   -lLLVMCore
   -lLLVMMC
   -lLLVMSupport
   -Wl,--end-group
   -llibclang
)

install(TARGETS doxypress DESTINATION .)
install(FILES ${CS_LIBS} DESTINATION .)

# (todo: enhance) need to copy the CS libraries and MingW files 
