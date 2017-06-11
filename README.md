## DoxyPress

### Introduction

DoxyPress is a modern C++ application for generating documentation from annotated sources
and API documentation.

DoxyPress and DoxyPressApp were initially derived from Doxygen and DoxyWizard.

###### Major enhancements

Parsing

* User selection for full parsing of C and C++ with clang in lieu of the legacy lex parser
* Clang parser allows complicated modern C++ to be parsed accurately, especially as C++ is updated
* Added a new parser for makefiles

Classes and Containers

* DoxyPress links with the CopperSpice libraries
* Removed all Qt 2 classes and containers
* Refactored to use value based containers instead of pointer based containers
* For improved memory management variadic templates and shared pointers are used
* Source code reformatted for improved readability
* Modified source code to use C++11 functionality, features, and syntax

Output

* Project file format enhanced to use a JSON format for improved functionality
* Command line option to convert Doxygen project files to the DoxyPress format
* Enhanced layout file to include new sections
* Improved html output for correctness

### System Requirements

To build DoxyPress you will need the CopperSpice libraries (CsCore, CsXml), a C++11 compiler,
and a C++11 standard library.

Uses CMake or Autotools for building binary files.


### Documentation

Full documentation for DoxyPress is available on the CopperSpice website:

www.copperspice.com/docs/doxypress/index.html


### Building

For documentation on how to build the DoxyPress application refer to the following page:

www.copperspice.com/docs/doxypress/build-doxypress.html


### Presentations

For additional information refer to our DoxyPress presentations at CppNow and CppCon:

www.copperspice.com/presentations.html


### Authors

* **Ansel Sermersheim**
* **Barbara Geller**


### License

This library is released under the GPL V2 license. For more information refer to the
LICENSE file provided with this project.


### References

* Website: www.copperspice.com
* Email:   info@copperspice.com
