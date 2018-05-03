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

#ifndef CONDPARSER_H
#define CONDPARSER_H

/**
 * C++ Expression parser for EXTABLED_SETIONS
 *
 * Features used:
 *     Operators:
 *         &&    AND operator
 *         ||    OR  operator
 *         !     NOT operator
 */

#include <QString>

class CondParser
{

 public:
   CondParser()
      : m_tokenType(NOTHING) {}

   bool parse(const QString &fileName, int lineNr, const QString &expr);

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

 private:
   QString m_err;                    // error state
   QString m_expr;                   // holds the expression

   QString::const_iterator m_iter;   // points to a character in expr

   QString m_token;                  // holds the token
   TOKENTYPE m_tokenType;            // type of the token

   void getToken();

   bool parseLevel1();
   bool parseLevel2();
   bool parseLevel3();
   bool parseVar();

   bool evalOperator(const int opId, bool lhs, bool rhs);
   bool evalVariable(const QString &varName);
   int getOperatorId(const QString &opName);
};

#endif

