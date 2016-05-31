cls
@echo off

set build=d:\clang\build
set buildTools=d:\clang\build\tools\clang

set llvm=d:\clang\llvm
set llvmTools=d:\clang\llvm\tools\clang

set toDir=d:\DoxyPress\clang_lib

md clang_lib\include\3.7.1\include
md clang_lib\include\clang\AST
md clang_lib\include\clang\Basic
md clang_lib\include\clang\Driver
md clang_lib\include\clang\Frontend
md clang_lib\include\clang\Index
md clang_lib\include\clang\Lex
md clang_lib\include\clang\Sema
md clang_lib\include\clang\Serialization
md clang_lib\include\clang\StaticAnalyzer\Core
md clang_lib\include\clang\Tooling
md clang_lib\include\clang-c
md clang_lib\include\llvm\ADT
md clang_lib\include\llvm\Bitcode
md clang_lib\include\llvm\Config
md clang_lib\include\llvm\Option
md clang_lib\include\llvm\Support
md clang_lib\include\llvm-c
md clang_lib\bin
md clang_lib\lib

echo.
echo Copy include files
xcopy /q /y %build%\lib\clang\3.7.1\include\*.*                              %toDir%\include\3.7.1\include  > nul
xcopy /q /y %llvmTools%\include\clang\AST\APValue.h                          %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\ASTConsumer.h                      %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\ASTContext.h                       %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\ASTFwd.h                           %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\ASTTypeTraits.h                    %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\ASTUnresolvedSet.h                 %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\ASTVector.h                        %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\Attr.h                             %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\AttrIterator.h                     %toDir%\include\clang\AST      > nul
xcopy /q /y %buildTools%\include\clang\AST\Attrs.inc                         %toDir%\include\clang\AST      > nul
xcopy /q /y %buildTools%\include\clang\AST\AttrVisitor.inc                   %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\BuiltinTypes.def                   %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\CanonicalType.h                    %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\CharUnits.h                        %toDir%\include\clang\AST      > nul
xcopy /q /y %buildTools%\include\clang\AST\CommentCommandList.inc            %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\CommentCommandTraits.h             %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\Decl.h                             %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\DeclAccessPair.h                   %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\DeclarationName.h                  %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\DeclBase.h                         %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\DeclCXX.h                          %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\DeclFriend.h                       %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\DeclGroup.h                        %toDir%\include\clang\AST      > nul
xcopy /q /y %buildTools%\include\clang\AST\DeclNodes.inc                     %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\DeclObjC.h                         %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\DeclOpenMP.h                       %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\DeclTemplate.h                     %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\Expr.h                             %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\ExprCXX.h                          %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\ExprObjC.h                         %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\ExternalASTSource.h                %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\LambdaCapture.h                    %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\NestedNameSpecifier.h              %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\OpenMPClause.h                     %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\OperationKinds.h                   %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\PrettyPrinter.h                    %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\RawCommentList.h                   %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\RecursiveASTVisitor.h              %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\Redeclarable.h                     %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\SelectorLocationsKind.h            %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\Stmt.h                             %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\StmtCXX.h                          %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\StmtIterator.h                     %toDir%\include\clang\AST      > nul
xcopy /q /y %buildTools%\include\clang\AST\StmtNodes.inc                     %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\StmtObjC.h                         %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\StmtOpenMP.h                       %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\TemplateBase.h                     %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\TemplateName.h                     %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\Type.h                             %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\TypeLoc.h                          %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\TypeLocNodes.def                   %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\TypeNodes.def                      %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\AST\UnresolvedSet.h                    %toDir%\include\clang\AST      > nul
xcopy /q /y %llvmTools%\include\clang\Basic\AddressSpaces.h                  %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\AttrKinds.h                      %toDir%\include\clang\Basic    > nul
xcopy /q /y %buildTools%\include\clang\Basic\AttrList.inc                    %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\Builtins.def                     %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\CapturedStmt.h                   %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\CharInfo.h                       %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\CommentOptions.h                 %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\Diagnostic.h                     %toDir%\include\clang\Basic    > nul
xcopy /q /y %buildTools%\include\clang\Basic\DiagnosticCommonKinds.inc       %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\DiagnosticIDs.h                  %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\DiagnosticOptions.def            %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\DiagnosticOptions.h              %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\ExceptionSpecificationType.h     %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\ExpressionTraits.h               %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\FileManager.h                    %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\FileSystemOptions.h              %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\IdentifierTable.h                %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\Lambda.h                         %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\LangOptions.def                  %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\LangOptions.h                    %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\Linkage.h                        %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\LLVM.h                           %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\Module.h                         %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\ObjCRuntime.h                    %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\OpenCLExtensions.def             %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\OpenMPKinds.def                  %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\OpenMPKinds.h                    %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\OperatorKinds.def                %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\OperatorKinds.h                  %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\PartialDiagnostic.h              %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\SanitizerBlacklist.h             %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\Sanitizers.def                   %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\Sanitizers.h                     %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\SourceLocation.h                 %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\SourceManager.h                  %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\Specifiers.h                     %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\TargetOptions.h                  %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\TokenKinds.def                   %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\TokenKinds.h                     %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\TypeTraits.h                     %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\VersionTuple.h                   %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\VirtualFileSystem.h              %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Basic\Visibility.h                     %toDir%\include\clang\Basic    > nul
xcopy /q /y %llvmTools%\include\clang\Driver\Util.h                          %toDir%\include\clang\Driver   > nul              
xcopy /q /y %llvmTools%\include\clang\Frontend\ASTUnit.h                     %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Frontend\CodeGenOptions.def            %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Frontend\CodeGenOptions.h              %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Frontend\CommandLineSourceLoc.h        %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Frontend\CompilerInstance.h            %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Frontend\CompilerInvocation.h          %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Frontend\DependencyOutputOptions.h     %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Frontend\FrontendAction.h              %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Frontend\FrontendActions.h             %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Frontend\FrontendOptions.h             %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Frontend\LangStandard.h                %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Frontend\LangStandards.def             %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Frontend\MigratorOptions.h             %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Frontend\PCHContainerOperations.h      %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Frontend\PreprocessorOutputOptions.h   %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Frontend\Utils.h                       %toDir%\include\clang\Frontend > nul
xcopy /q /y %llvmTools%\include\clang\Index\*.*                              %toDir%\include\clang\Index    > nul
xcopy /q /y %llvmTools%\include\clang\Lex\DirectoryLookup.h                  %toDir%\include\clang\Lex      > nul
xcopy /q /y %llvmTools%\include\clang\Lex\HeaderSearchOptions.h              %toDir%\include\clang\Lex      > nul
xcopy /q /y %llvmTools%\include\clang\Lex\ModuleLoader.h                     %toDir%\include\clang\Lex      > nul
xcopy /q /y %llvmTools%\include\clang\Lex\ModuleMap.h                        %toDir%\include\clang\Lex      > nul
xcopy /q /y %llvmTools%\include\clang\Lex\PPCallbacks.h                      %toDir%\include\clang\Lex      > nul
xcopy /q /y %llvmTools%\include\clang\Lex\Pragma.h                           %toDir%\include\clang\Lex      > nul
xcopy /q /y %llvmTools%\include\clang\Lex\PreprocessingRecord.h              %toDir%\include\clang\Lex      > nul
xcopy /q /y %llvmTools%\include\clang\Lex\PreprocessorOptions.h              %toDir%\include\clang\Lex      > nul
xcopy /q /y %llvmTools%\include\clang\Sema\CodeComplete*.h                   %toDir%\include\clang\Sema     > nul
xcopy /q /y %llvmTools%\include\clang\Serialization\ASTBitCodes.h            %toDir%\include\clang\Serialization        > nul 
xcopy /q /y %llvmTools%\include\clang\StaticAnalyzer\Core\Analyses.def       %toDir%\include\clang\StaticAnalyzer\Core  > nul
xcopy /q /y %llvmTools%\include\clang\StaticAnalyzer\Core\AnalyzerOptions.h  %toDir%\include\clang\StaticAnalyzer\Core  > nul
xcopy /q /y %llvmTools%\include\clang\Tooling\ArgumentsAdjusters.h           %toDir%\include\clang\Tooling  > nul
xcopy /q /y %llvmTools%\include\clang\Tooling\CommonOptionsParser.h          %toDir%\include\clang\Tooling  > nul
xcopy /q /y %llvmTools%\include\clang\Tooling\CompilationDatabase.h          %toDir%\include\clang\Tooling  > nul
xcopy /q /y %llvmTools%\include\clang\Tooling\Tooling.h                      %toDir%\include\clang\Tooling  > nul
xcopy /q /y %llvm%\include\llvm\ADT\APFloat.h                                %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\APInt.h                                  %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\APSInt.h                                 %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\ArrayRef.h                               %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\DenseMap.h                               %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\DenseMapInfo.h                           %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\DenseSet.h                               %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\EpochTracker.h                           %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\FoldingSet.h                             %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\Hashing.h                                %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\IntrusiveRefCntPtr.h                     %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\iterator.h                               %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\iterator_range.h                         %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\None.h                                   %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\Optional.h                               %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\PointerIntPair.h                         %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\PointerUnion.h                           %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\SetVector.h                              %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\SmallPtrSet.h                            %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\SmallSet.h                               %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\SmallString.h                            %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\SmallVector.h                            %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\STLExtras.h                              %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\StringMap.h                              %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\StringRef.h                              %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\StringSet.h                              %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\StringSwitch.h                           %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\TinyPtrVector.h                          %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\Triple.h                                 %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\ADT\Twine.h                                  %toDir%\include\llvm\ADT  > nul
xcopy /q /y %llvm%\include\llvm\Bitcode\BitCodes.h                           %toDir%\include\llvm\Bitcode  > nul
xcopy /q /y %build%\include\llvm\Config\llvm-config.h                        %toDir%\include\llvm\Config   > nul
xcopy /q /y %llvm%\include\llvm\Option\Opt*.h                                %toDir%\include\llvm\Option   > nul
xcopy /q /y %llvm%\include\llvm\Support\AlignOf.h                            %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\Allocator.h                          %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\Casting.h                            %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\CBindingWrapping.h                   %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\CommandLine.h                        %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\Compiler.h                           %toDir%\include\llvm\Support  > nul
xcopy /q /y %build%\include\llvm\Support\DataTypes.h                         %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\ErrorHandling.h                      %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\ErrorOr.h                            %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\FileSystem.h                         %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\Host.h                               %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\MathExtras.h                         %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\MD5.h                                %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\Memory.h                             %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\MemoryBuffer.h                       %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\Path.h                               %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\PointerLikeTypeTraits.h              %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\PrettyStackTrace.h                   %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\raw_ostream.h                        %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\Regex.h                              %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\SMLoc.h                              %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\SourceMgr.h                          %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\SpecialCaseList.h                    %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\SwapByteOrder.h                      %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\TimeValue.h                          %toDir%\include\llvm\Support  > nul
xcopy /q /y %llvm%\include\llvm\Support\type_traits.h                        %toDir%\include\llvm\Support  > nul

xcopy /q /y %llvm%\include\llvm-c\Support.h                                  %toDir%\include\llvm-c\Support.h  > nul

echo.
echo Copy lib files
xcopy /q /y %build%\lib\libclangAnalysis.a                                   %toDir%\lib  > nul
xcopy /q /y %build%\lib\libclangAST.a                                        %toDir%\lib  > nul
xcopy /q /y %build%\lib\libclangBasic.a                                      %toDir%\lib  > nul
xcopy /q /y %build%\lib\libclangDriver.a                                     %toDir%\lib  > nul
xcopy /q /y %build%\lib\libclangEdit.a                                       %toDir%\lib  > nul
xcopy /q /y %build%\lib\libclangFrontend.a                                   %toDir%\lib  > nul
xcopy /q /y %build%\lib\libclangIndex.a                                      %toDir%\lib  > nul
xcopy /q /y %build%\lib\libclangLex.a                                        %toDir%\lib  > nul
xcopy /q /y %build%\lib\libclangParse.a                                      %toDir%\lib  > nul
xcopy /q /y %build%\lib\libclangSema.a                                       %toDir%\lib  > nul
xcopy /q /y %build%\lib\libclangSerialization.a                              %toDir%\lib  > nul
xcopy /q /y %build%\lib\libclangStaticAnalyzerCheckers.a                     %toDir%\lib  > nul
xcopy /q /y %build%\lib\libclangStaticAnalyzerCore.a                         %toDir%\lib  > nul
xcopy /q /y %build%\lib\libclangStaticAnalyzerFrontend.a                     %toDir%\lib  > nul
xcopy /q /y %build%\lib\libclangTooling.a                                    %toDir%\lib  > nul
xcopy /q /y %build%\lib\libclangToolingCore.a                                %toDir%\lib  > nul
xcopy /q /y %build%\lib\liblibclang.dll.a                                    %toDir%\lib  > nul
xcopy /q /y %build%\lib\libLLVMBitReader.a                                   %toDir%\lib  > nul
xcopy /q /y %build%\lib\libLLVMCore.a                                        %toDir%\lib  > nul
xcopy /q /y %build%\lib\libLLVMMC.a                                          %toDir%\lib  > nul
xcopy /q /y %build%\lib\libLLVMMCParser.a                                    %toDir%\lib  > nul
xcopy /q /y %build%\lib\libLLVMOption.a                                      %toDir%\lib  > nul
xcopy /q /y %build%\lib\libLLVMSupport.a                                     %toDir%\lib  > nul

echo.
echo Copy clang files
xcopy /q /y %build%\bin\libclang.dll                                         %toDir%\bin  > nul
xcopy /q /y %llvmTools%\include\clang-c\*.h                                  %toDir%\include\clang-c > nul
xcopy /q /y %build%\lib\liblibclang.dll.a                                    %toDir%\lib  > nul

echo.
echo Completed