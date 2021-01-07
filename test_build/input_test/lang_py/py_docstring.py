/************************************************************************
*
* Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
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

"""@package py_docString
Documentation for package using docString comment syntax.

More details (docString-1).
"""

def func():
    """Documentation for a function.

    More details (docString-2).
    """
    pass

class PyClass:
    """Documentation for a class.

    More details (docString-3).
    """

    def __init__(self):
        """The constructor documentation."""
        self._memVar = 0;

    def PyMethod(self):
        """Documentation for a method."""
        pass

