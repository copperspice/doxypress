/************************************************************************
*
* Copyright (c) 2014-2025 Barbara Geller & Ansel Sermersheim
*
* DoxyPress is free software. You can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
* https://www.gnu.org/licenses/
*
*************************************************************************/

#ifndef VERSION_H
#define VERSION_H

namespace my_library
{
   namespace v1
   {
      /// Documentation for function my_library::v1::seed. If no docs are desired for
      /// the older version, just leave off this comment.
      void seed(int value) {
         // does nothing
      }
   }

   inline namespace v2
   {
      /// Documentation for function my_library::v2::seed. This method is located inside an inline namespace.
      void seed(int value) {
         // does nothing
      }
   }
}