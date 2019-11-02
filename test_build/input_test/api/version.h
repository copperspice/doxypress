/*************************************************************************
*
* Copyright (C) 2012-2019 Barbara Geller & Ansel Sermersheim
*
* GNU Free Documentation License
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