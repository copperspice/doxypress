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

/** @brief Simplified and optimized version of QTextStream */
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


/*

{
 public:
   FTextStream();
   FTextStream( QIODevice *);
   FTextStream( QByteArray *);
   FTextStream( FILE *);

   virtual ~FTextStream();

   QIODevice	*device() const;
   void	 setDevice( QIODevice *);
   void	 unsetDevice();

   FTextStream &operator<<( char );
   FTextStream &operator<<( const char *);
   FTextStream &operator<<( const QString &);
   FTextStream &operator<<( const QByteArray &);
   FTextStream &operator<<( signed short );
   FTextStream &operator<<( unsigned short );
   FTextStream &operator<<( signed int );
   FTextStream &operator<<( unsigned int );
   FTextStream &operator<<( signed long );
   FTextStream &operator<<( unsigned long );
   FTextStream &operator<<( float );
   FTextStream &operator<<( double );

 private:
   QIODevice *m_dev;
   bool m_owndev;
   FTextStream &output_int( ulong n, bool neg );

 private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
   FTextStream( const FTextStream &);
   FTextStream &operator=( const FTextStream &);
#endif
};

inline FTextStream &FTextStream::operator<<( char c)
{
   if (m_dev) {
      m_dev->putChar(c);
   }
   return *this;
}

inline FTextStream &FTextStream::operator<<( const char *s)
{
   uint len = qstrlen( s );
   if (m_dev) {
      m_dev->write( s, len );
   }
   return *this;
}

inline FTextStream &FTextStream::operator<<( const QString &s)
{
   return operator<<(s.toUtf8().constData());
}

inline FTextStream &FTextStream::operator<<( const QByteArray &s)
{
   return operator<<(s.constData());
}

typedef FTextStream &(*FTSFUNC)(FTextStream &); // manipulator function

inline FTextStream &operator<<( FTextStream &s, FTSFUNC f )
{
   return (*f)( s );
}

inline FTextStream &endl( FTextStream &s)
{
   return s << '\n';
}

*/

#endif // FTEXTSTREAM_H
