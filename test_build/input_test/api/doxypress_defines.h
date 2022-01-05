/************************************************************************
*
* Copyright (c) 2014-2022 Barbara Geller & Ansel Sermersheim
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

#define Q_C_CALLBACKS
#define Q_DECL_EXPORT
#define Q_DECL_IMPORT
#define Q_BYTE_ORDER
#define Q_BIG_ENDIAN
#define Q_LITTLE_ENDIAN
#define Q_CHECK_PTR ( pointer )

#define QT_LINUXBASE
#define QT_LARGEFILE_SUPPORT 64
#define QT_POINTER_SIZE
#define QT_VERSION
#define QT_VERSION_CHECK
#define QT_VERSION_STR

#define Q_OS_BSD4
#define Q_OS_DARWIN
#define Q_OS_FREEBSD
#define Q_OS_HPUX
#define Q_OS_LINUX
#define Q_OS_MAC
#define Q_OS_MSDOS
#define Q_OS_NACL
#define Q_OS_NETBSD
#define Q_OS_SOLARIS
#define Q_OS_UNIX
#define Q_OS_WIN
#define Q_OS_WIN32
#define Q_OS_WIN64

#defind Q_WS_QPA
#defind Q_WS_X11

#define Q_CC_CDS
#define Q_CC_CLANG
#define Q_CC_COMEAU
#define Q_CC_EDG
#define Q_CC_GNU
#define Q_CC_HPACC
#define Q_CC_INTEL
#define Q_CC_KAI
#define Q_CC_LINUX
#define Q_CC_MIPS
#define Q_CC_OC
#define Q_CC_OPENBSD
#define Q_CC_PGI
#define Q_CC_XLC

#define CS_VERSION
#define CS_VERSION_CHECK(major, minor, patch)
#define CS_VERSION_STR

#define Q_INT64_C( literal )
#define Q_UINT64_C( literal )

#define foreach(variable, container)
#define forever
#define qPrintable(str)

#define Q_ASSERT(cond)
#define Q_ASSERT_X(cond, where, what)
#define Q_CHECK_PTR(pointer)
#define Q_FOREACH(variable, container)
#define Q_FOREVER
#define Q_FUNC_INFO
#define Q_LIKELY(expr)
#define Q_UNLIKELY(expr)
#define Q_UNUSED(name)

#define QT_TRID_NOOP(id)
#define QT_TR_NOOP(sourceText)
#define QT_TR_NOOP_UTF8(sourceText)
#define QT_TRANSLATE_NOOP(context,  sourceText)
#define QT_TRANSLATE_NOOP_UTF8(context,  sourceText)
#define QT_TRANSLATE_NOOP3(context,  sourceText,  comment)
#define QT_TRANSLATE_NOOP3_UTF8(context,  sourceText,  comment)

#define QT_POINTER_SIZE

int qMacVersion();

#define Q_OS_OPENBSD
#define Q_OS_UNIXWARE
#define Q_WS_QPA
#define Q_WS_QWS

#define Q_DECLARE_METATYPE(TYPE)


class QObject {
   public:
      static QString tr ( const char * sourceText, const char * disambiguation = 0, int n = -1 );
      static QString trUtf8 ( const char * sourceText, const char * disambiguation = 0, int n = -1 );
};


