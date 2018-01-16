/*************************************************************************
*
* Copyright (C) 2012-2018 Barbara Geller & Ansel Sermersheim
* All rights reserved.
*
* GNU Free Documentation License
*
*************************************************************************/

#ifndef QSSL_H
#define QSSL_H

QT_BEGIN_NAMESPACE

namespace QSsl {

   enum KeyType {
      PrivateKey,
      PublicKey
   };

   enum KeyAlgorithm {
      Rsa,
      Dsa
   };

   void someFunction(QString data);
};

QT_END_NAMESPACE

#endif
