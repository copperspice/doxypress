## DoxyPress

### Introduction

DoxyPress is a modern C++ application for generating documentation from annotated sources and API documentation.
DoxyPress and DoxyPressApp were initially derived from Doxygen and DoxyWizard.


###### Major enhancements

Parsing

* User selection for full parsing of C and C++ with clang in lieu of the legacy lex parser
* Clang parser allows complicated modern C++ to be parsed accurately, especially as C++ is updated
* Added a new parser for makefiles
* Documents the following computer languages: C, C++, Objective-C, C#, Fortran, Java, IDL, Python, PHP, and TCL

Classes and Containers

* DoxyPress links with the CopperSpice libraries
* Removed all Qt 2 classes and containers
* Refactored to use value based containers instead of pointer based containers
* For improved memory management variadic templates and shared pointers are used
* Source code reformatted for improved readability
* Modified source code to use modern C++ functionality, features, and syntax

Output

* Project file format enhanced to use a JSON format for improved functionality
* Command line option to convert Doxygen project files to the DoxyPress JSON format
* Enhanced layout file to include new sections
* Major improvements to the html output for correctness


### Building

DoxyPress prebuilt binaries are available for Linux, Mac OS X, and Windows.

https://download.copperspice.com/doxypress/binary/

To build DoxyPress from source a current copy of the CopperSpice libraries, a C++17 compiler, and a C++17 standard
library are required. DoxyPress links with CsCore and CsXml which are part of the CopperSpice libraries.

CMake build files are provided with the DoxyPress source distribution.


### Documentation

Full documentation for DoxyPress is available on the website or from our download page.


|URL      |Description|
|---------|-----------|
|https://www.copperspice.com/docs/doxypress/index.html| Online
|https://download.copperspice.com/doxypress/documentation| Download (tar and zip formats)|


For documentation on how to build the DoxyPress application refer to the following page:
www.copperspice.com/docs/doxypress/build-doxypress.html



### Presentations

Our YouTube channel videos are about modern C++, build systems, CopperSpice, DoxyPress, and other topics related to
our open source work.

https://www.youtube.com/copperspice


Links to technical presentations recorded at CppNow and CppCon:

www.copperspice.com/presentations.html


### Authors / Contributors

* **Ansel Sermersheim**
* **Barbara Geller**
* **Andrew Marlow**
* **Adam Mensel**
* **Tim van Deurzen**
* **Robert Jamnitzky**


### License

This program is released under the GPL 2 license. For more information refer to the LICENSE file provided with this
project.


### References

* Website: www.copperspice.com
* Email:   info@copperspice.com
