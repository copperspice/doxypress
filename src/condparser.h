/************************************************************************
*
* Copyright (c) 2014-2022 Barbara Geller & Ansel Sermersheim
* Copyright (c) 1997-2014 Dimitri van Heesch
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

