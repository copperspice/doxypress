/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License version 2
 * is hereby granted. No representations are made about the suitability of
 * this software for any purpose. It is provided "as is" without express or
 * implied warranty. See the GNU General Public License for more details.
 *
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#ifndef FILESTORAGE_H
#define FILESTORAGE_H

#include <QFile>

#include <assert.h>
#include <store.h>

/** @brief Store implementation based on a file.
    Writing is linear, after that the file is re-opened for reading.
    Reading is random (seek+read).
 */
class FileStorage : public StorageIntf
{
 public:
   FileStorage() : m_readOnly(false) {}

   FileStorage(const QString &name) : m_readOnly(false)
   {
      m_file.setFileName(name);
   }

   int read(char *buf, uint size) override {
      return m_file.read(buf, size);
   }

   int write(const char *buf, uint size) override {
      assert(m_readOnly == false);
      return m_file.write(buf, size);
   }

   void clear() {
      m_readOnly = false;

      m_file.close();
      m_file.setFileName("");
   }

   bool open( QIODevice::OpenModeFlag value)  {
      m_readOnly = (value == QIODevice::ReadOnly);

      return m_file.open(value);
   }

   bool seek(int64_t pos) {
      return m_file.seek(pos);
   }

   int64_t pos() const {
      return m_file.pos();
   }

   void close() {
      m_file.close();
   }

   void setName(QString name ) {
      m_file.setFileName(name);
   }

 private:
   bool  m_readOnly;
   QFile m_file;
};

#endif
