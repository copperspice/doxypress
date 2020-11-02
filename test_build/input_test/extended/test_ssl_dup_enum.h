/*************************************************************************
*
* Copyright (C) 2012-2020 Barbara Geller & Ansel Sermersheim
*
* GNU Free Documentation License
*
*************************************************************************/

#ifndef  QSSL_DUP_ENUM_H
#define QSSL_DUP_ENUM_H

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

#endif
