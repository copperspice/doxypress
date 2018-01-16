/*************************************************************************
*
* Copyright (C) 2012-2018 Barbara Geller & Ansel Sermersheim
* All rights reserved.
*
* GNU Free Documentation License
*
*************************************************************************/

#ifndef  QSSL_DUP_ENUM_H
#define QSSL_DUP_ENUM_H

QT_BEGIN_NAMESPACE

namespace QSsl_DupEnum {

   enum KeyType {
      PrivateKey,
      PublicKey
   };

   enum KeyAlgorithm_NotADup {
      Rsa,
      Dsa
   };

};

QT_END_NAMESPACE

#endif

