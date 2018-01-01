/*************************************************************************
 *
 * Copyright (C) 2014-2018 Barbara Geller & Ansel Sermersheim
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

#include <QString>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include <store.h>
#include <portable.h>

#include <doxy_globals.h>

#define BLOCK_SIZE         512 // should be >8 and a power of 2
#define BLOCK_POINTER_SIZE sizeof(portable_off_t)

#define ASSERTS_ENABLED

#ifdef ASSERTS_ENABLED
#define STORE_ASSERT(x) assert(x)
#else
#define STORE_ASSERT(x)
#endif

// Decide to use ftell or keep track of the current file pointer ourselves.
// Since valgrind shows that calling ftell has the unwanted side-effect of
// writing some uninitialized bytes (!) it might be better (and faster) to keep track
// of the current pointer ourselves.
#define USE_FTELL 0

Store::Store()
{
   m_fileHandle = nullptr;
   m_front      = 0;
   m_cur        = 0;
   m_head       = 0;
   m_state      = Init;
   m_reads      = 0;
   m_writes     = 0;
}

Store::~Store()
{
   if (m_fileHandle) {
      fclose(m_fileHandle);
   }

   // clean up free list
   while (m_head) {
      Node *node = m_head;
      m_head = node->next;
      delete node;
   }
}

int Store::open(QString name)
{
   STORE_ASSERT(m_state == Init);

   if (m_fileHandle) {
      // already open
      return 0;
   }

   m_fileHandle = fopen(name.toUtf8().constData(), "w+b");

   if (m_fileHandle == nullptr) {
      return -1;
   }

   // first block serves as header, so offset = 0 can be used as the end of the list
   for (int i = 0; i < BLOCK_SIZE / 8; i++) {
      fputc('T', m_fileHandle);
      fputc('E', m_fileHandle);
      fputc('M', m_fileHandle);
      fputc('P', m_fileHandle);
      fputc('_', m_fileHandle);
      fputc('X', m_fileHandle);
      fputc('X', m_fileHandle);
      fputc(0, m_fileHandle);
   }

   m_front  = BLOCK_SIZE;
   m_cur    = BLOCK_SIZE;
   m_head   = 0;
   m_state  = Reading;

   return 0;
}

void Store::close()
{
   if (m_fileHandle) {
      fclose(m_fileHandle);
   }

   m_fileHandle = 0;
   m_state      = Init;
}

portable_off_t Store::alloc()
{
   STORE_ASSERT(m_state == Reading);

   m_state = Writing;
   portable_off_t pos;

   if (m_head == 0) {
      // allocate new block

      if (portable_fseek(m_fileHandle, 0, SEEK_END) == -1) { // go to end of the file
         fprintf(stderr, "Store Alloc: Error seeking to end of file: %s\n", strerror(errno));
         Doxy_Work::stopDoxyPress();
      }

#if USE_FTELL
      pos = portable_ftell(m_fileHandle);
      STORE_ASSERT( (pos & (BLOCK_SIZE - 1)) == 0 );
      m_front = pos + BLOCK_SIZE; // move front to end of this block
#else
      m_cur = m_front;
      pos   = m_cur;
      STORE_ASSERT( (pos & (BLOCK_SIZE - 1)) == 0 );
      m_front = pos + BLOCK_SIZE;
#endif

   } else {
      // reuse freed block

      Node *node = m_head;
      pos = node->pos;

      // point head to next free item
      m_head = node->next;
      delete node;

      // move to start of the block
      if (portable_fseek(m_fileHandle, pos, SEEK_SET) == -1) {
         fprintf(stderr, "Store Alloc: Error seeking to position %d: %s\n", static_cast<int>(pos), strerror(errno));
         Doxy_Work::stopDoxyPress();
      }

      m_cur = pos;
      STORE_ASSERT( (pos & (BLOCK_SIZE - 1)) == 0 );
   }

   return pos;
}

int Store::write(const char *buf, uint size)
{
   STORE_ASSERT(m_state == Writing);

   do {

#if USE_FTELL
      portable_off_t curPos = portable_ftell(m_fileHandle);
#else
      portable_off_t curPos = m_cur;
#endif

      int bytesInBlock = (int)(BLOCK_SIZE - BLOCK_POINTER_SIZE - (curPos & (BLOCK_SIZE - 1)));
      int bytesLeft    = bytesInBlock < (int)size ? (int)size - bytesInBlock : 0;
      int numBytes     = size - bytesLeft;

      STORE_ASSERT(bytesInBlock >= 0);
      STORE_ASSERT(numBytes <= (int)(BLOCK_SIZE - BLOCK_POINTER_SIZE));

      if (numBytes > 0) {
         if ((int)fwrite(buf, 1, numBytes, m_fileHandle) != numBytes) {
            fprintf(stderr, "Error writing: %s\n", strerror(errno));
            Doxy_Work::stopDoxyPress();
         }

         m_cur += numBytes;
         m_writes++;
      }

      if (bytesLeft > 0) {
         // still more bytes to write

#if USE_FTELL
         STORE_ASSERT(((portable_ftell(m_fileHandle) + BLOCK_POINTER_SIZE) & (BLOCK_SIZE - 1)) == 0);
#else
         STORE_ASSERT(((m_cur + BLOCK_POINTER_SIZE) & (BLOCK_SIZE - 1)) == 0);
#endif

         // allocate new block
         if (m_head == 0) {
            // no free blocks to reuse
            // write pointer to next block

            if (fwrite(&m_front, BLOCK_POINTER_SIZE, 1, m_fileHandle) != 1) {
               fprintf(stderr, "Error writing to store: %s\n", strerror(errno));
               Doxy_Work::stopDoxyPress();
            }

            m_cur += BLOCK_POINTER_SIZE;
#if USE_FTELL
            STORE_ASSERT(portable_ftell(m_fileHandle) == (curPos & ~(BLOCK_SIZE - 1)) + BLOCK_SIZE);
#else
            STORE_ASSERT(m_cur == (curPos & ~(BLOCK_SIZE - 1)) + BLOCK_SIZE);
#endif

            // move to next block
            if (portable_fseek(m_fileHandle, 0, SEEK_END) == -1) { // go to end of the file
               fprintf(stderr, "Store Alloc: Error seeking to end of file: %s\n", strerror(errno));
               Doxy_Work::stopDoxyPress();
            }

            m_cur = m_front;

#if USE_FTELL
            STORE_ASSERT(portable_ftell(m_fileHandle) == m_front);
#else
            STORE_ASSERT(m_cur == m_front);
#endif
            // move front to the next of the block
            m_front += BLOCK_SIZE;

         } else {
            // reuse block from the free list
            // write pointer to next block

            if (fwrite(&m_head->pos, BLOCK_POINTER_SIZE, 1, m_fileHandle) != 1) {
               fprintf(stderr, "Error writing to store: %s\n", strerror(errno));
               Doxy_Work::stopDoxyPress();
            }

            Node *node = m_head;
            portable_off_t pos = node->pos;

            // point head to next free item
            m_head = node->next;
            delete node;

            // move to start of the block
            if (portable_fseek(m_fileHandle, pos, SEEK_SET) == -1) {
               fprintf(stderr, "Store Write: Error seeking to position %d: %s\n", static_cast<int>(pos), strerror(errno));
               Doxy_Work::stopDoxyPress();
            }

            m_cur = pos;

         }
      }

      size -= numBytes;
      buf += numBytes;

   } while (size > 0);

   return size;
}

void Store::end()
{
   STORE_ASSERT(m_state == Writing);

#if USE_FTELL
   portable_off_t curPos = portable_ftell(m_fileHandle);
#else
   portable_off_t curPos = m_cur;
#endif

   int bytesInBlock = (int)(BLOCK_SIZE - (curPos & (BLOCK_SIZE - 1)));

   // zero out rest of the block
   int i;
   for (i = 0; i < bytesInBlock; i++) {
      fputc(0, m_fileHandle);
   }
   m_state = Reading;
}

void Store::release(portable_off_t pos)
{
   STORE_ASSERT(m_state == Reading);
   STORE_ASSERT(pos > 0 && (pos & (BLOCK_SIZE - 1)) == 0);

   // goto end of the block
   portable_off_t cur = pos, next;

   while (1) {
      // add new node to the free list
      Node *node = new Node;
      node->next = m_head;
      node->pos  = cur;

      m_head = node;

      // goto the end of cur block
      if (portable_fseek(m_fileHandle, cur + BLOCK_SIZE - BLOCK_POINTER_SIZE, SEEK_SET) == -1) {
         fprintf(stderr, "Store Release: Error seeking to position %d: %s\n",
                 (int)(cur + BLOCK_SIZE - BLOCK_POINTER_SIZE), strerror(errno));
         Doxy_Work::stopDoxyPress();
      }

      // read pointer to next block
      if (fread(&next, BLOCK_POINTER_SIZE, 1, m_fileHandle) != 1) {
         fprintf(stderr, "Store::release: Error reading from store: %s\n", strerror(errno));
         Doxy_Work::stopDoxyPress();
      }

      m_cur = cur + BLOCK_SIZE;
      if (next == 0) {
         break;   // found end of list -> cur is last element
      }
      STORE_ASSERT((next & (BLOCK_SIZE - 1)) == 0);
      cur = next;

   }
}

void Store::seek(portable_off_t pos)
{
   STORE_ASSERT(m_state == Reading);

   if (portable_fseek(m_fileHandle, pos, SEEK_SET) == -1) {
      fprintf(stderr, "Store Seek: Error seeking to position %d: %s\n", pos, strerror(errno));
      Doxy_Work::stopDoxyPress();
   }

   m_cur = pos;
   STORE_ASSERT((pos & (BLOCK_SIZE - 1)) == 0);
}

int Store::read(char *buf, uint size)
{
   STORE_ASSERT(m_state == Reading);

   do {
#if USE_FTELL
      portable_off_t curPos = portable_ftell(m_fileHandle);
#else
      portable_off_t curPos = m_cur;
#endif

      int bytesInBlock = (int)(BLOCK_SIZE - BLOCK_POINTER_SIZE - (curPos & (BLOCK_SIZE - 1)));
      int bytesLeft    = bytesInBlock < (int)size ? (int)size - bytesInBlock : 0;
      int numBytes     = size - bytesLeft;
      //printf("  Store::read: pos=%x num=%d left=%d\n",(int)curPos,numBytes,bytesLeft);

      if (numBytes > 0) {

         if ((int)fread(buf, 1, numBytes, m_fileHandle) != numBytes) {
            fprintf(stderr, "Error reading from store: %s\n", strerror(errno));
            Doxy_Work::stopDoxyPress();
         }

         m_cur += numBytes;
         m_reads++;
      }

      if (bytesLeft > 0) {
         portable_off_t newPos;

         // read offset of the next block
#if USE_FTELL
         STORE_ASSERT(((portable_ftell(m_fileHandle) + BLOCK_POINTER_SIZE) & (BLOCK_SIZE - 1)) == 0);
#else
         STORE_ASSERT(((m_cur + BLOCK_POINTER_SIZE) & (BLOCK_SIZE - 1)) == 0);
#endif

         if (fread((char *)&newPos, BLOCK_POINTER_SIZE, 1, m_fileHandle) != 1) {
            fprintf(stderr, "Error reading from store: %s\n", strerror(errno));
            Doxy_Work::stopDoxyPress();
         }

         STORE_ASSERT(newPos != 0);
         STORE_ASSERT((newPos & (BLOCK_SIZE - 1)) == 0);
         curPos = newPos;

         // move to next block
         if (portable_fseek(m_fileHandle, curPos, SEEK_SET) == -1) {
            fprintf(stderr, "Store Read: Error seeking to position %d: %s\n", static_cast<int>(curPos), strerror(errno));
            Doxy_Work::stopDoxyPress();
         }

         m_cur = curPos;
      }

      size -= numBytes;
      buf += numBytes;
   } while (size > 0);

   return size;
}

void Store::printFreeList()
{
   printf("FreeList: ");

   while (m_head) {
      portable_off_t pos = m_head->pos;
      printf("%x ", static_cast<int>(pos));
      m_head = m_head->next;
   }

   printf("\n");
}

void Store::printStats()
{
   printf("ObjStore: block size %d bytes, total size %ld blocks, wrote %d blocks, read %d blocks\n",
          BLOCK_SIZE, (long)(m_front / BLOCK_SIZE), m_reads, m_writes);
}

void Store::dumpBlock(portable_off_t s, portable_off_t e)
{
   portable_fseek(m_fileHandle, s, SEEK_SET);
   int size = (int)(e - s);

   uchar *buf = new uchar[size];

   if (fread(buf, size, 1, m_fileHandle) == (size_t)size) {
      int i, j;

      for (i = 0; i < size; i += 16) {
         printf("%08x: ", (int)s + i);

         for (j = i; j < qMin(size, i + 16); j++) {
            printf("%02x ", buf[i + j]);
         }

         printf("  ");

         for (j = i; j < qMin(size, i + 16); j++) {
            printf("%c", (buf[i + j] >= 32 && buf[i + j] < 128) ? buf[i + j] : '.');
         }

         printf("\n");
      }
   }

   delete[] buf;
   portable_fseek(m_fileHandle, m_cur, SEEK_SET);
}

