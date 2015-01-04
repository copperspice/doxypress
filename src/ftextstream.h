/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
 * All rights reserved.    
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License version 2
 * is hereby granted. No representations are made about the suitability of
 * this software for any purpose. It is provided "as is" without express or
 * implied warranty. See the GNU General Public License for more details.
 *
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#ifndef FTEXTSTREAM_H
#define FTEXTSTREAM_H

#include <QTextStream>

class FTextStream : public QTextStream
{
   public:   

      FTextStream()
      {} 
    
      FTextStream(QIODevice *device) : QTextStream(device) 
      {
      }

   	FTextStream(FILE *fileHandle, QIODevice::OpenMode openMode = QIODevice::ReadWrite) 
         : QTextStream(fileHandle, openMode)
      {
      }

   	FTextStream(QString *string, QIODevice::OpenMode openMode = QIODevice::ReadWrite)
         : QTextStream(string, openMode)
      {
      }

   	FTextStream(QByteArray *array, QIODevice::OpenMode openMode = QIODevice::ReadWrite)
         : QTextStream(array, openMode)
      {
      }

   	FTextStream(const QByteArray &array, QIODevice::OpenMode openMode = QIODevice::ReadOnly)
         : QTextStream(array, openMode)
      {
      }

};

#endif
