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

#ifndef CONDPARSER_H
#define CONDPARSER_H

/**
 * C++ Expression parser for EXTABLED_SETIONS in Doxygen
 *
 * Features used:
 *     Operators:
 *         &&    AND operator
 *         ||    OR  operator
 *         !     NOT operator
 */

#include <QByteArray>

class CondParser
{
   // public functions
 public:
   CondParser() : m_e(0), m_tokenType(NOTHING) {}
   bool parse(const char *fileName, int lineNr, const char *expr);

   // enumerations
 private:
   enum TOKENTYPE {
      NOTHING = -1,
      DELIMITER,
      VARIABLE,
      UNKNOWN
   };
   enum OPERATOR_ID {
      UNKNOWN_OP = -1,
      AND = 1,
      OR,
      NOT
   };

   // data
 private:

   QByteArray m_err;                 //!< error state
   QByteArray m_expr;                //!< holds the expression
   const char *m_e;                //!< points to a character in expr

   QByteArray m_token;               //!< holds the token
   TOKENTYPE m_tokenType;          //!< type of the token

   // private functions
 private:
   void getToken();

   bool parseLevel1();
   bool parseLevel2();
   bool parseLevel3();
   bool parseVar();

   bool evalOperator(const int opId, bool lhs, bool rhs);
   bool evalVariable(const char *varName);
   int getOperatorId(const QByteArray &opName);
};

#endif

