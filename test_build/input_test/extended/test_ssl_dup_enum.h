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
