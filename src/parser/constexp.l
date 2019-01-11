/*************************************************************************
 *
 * Copyright (C) 2014-2019 Barbara Geller & Ansel Sermersheim
 * Copyright (C) 1997-2014 by Dimitri van Heesch.

*************************************************************************/

%{

#include <constexp.h>
#include <cppvalue.h>
#include <ce_parse.h>         // generated header file
#include <message.h>

#define YY_NEVER_INTERACTIVE 1
#define YY_NO_INPUT 1

QString         g_strToken;
CPPValue        g_resultValue;
int             g_constExpLineNr;
QString         g_constExpFileName;

static QString  s_inputString;
static int      s_inputPosition;

#undef  YY_INPUT
#define YY_INPUT(buf,result,max_size) result = yyread(buf,max_size);

static int yyread(char *buf, int max_size)
{
   int len = max_size;

   const char *src = s_inputString.constData() + s_inputPosition;

   if (s_inputPosition + len >= s_inputString.size_storage()) {
      len = s_inputString.size_storage() - s_inputPosition;
   }

   memcpy(buf, src, len);
   s_inputPosition += len;

   return len;
}

%}

CONSTSUFFIX ([uU][lL]?[lL]?)|([lL][lL]?[uU]?)

%option nounput

%%

"?"       { return TOK_QUESTIONMARK;           }
":"       { return TOK_COLON;                  }
"||"      { return TOK_OR;                     }
"&&"      { return TOK_AND;                    }
"|"       { return TOK_BITWISEOR;              }
"^"       { return TOK_BITWISEXOR;             }
"&"       { return TOK_AMPERSAND;              }
"!="      { return TOK_NOTEQUAL;               }
"=="      { return TOK_EQUAL;                  }
"<"       { return TOK_LESSTHAN;               }
">"       { return TOK_GREATERTHAN;            }
"<="      { return TOK_LESSTHANOREQUALTO;      }
">="      { return TOK_GREATERTHANOREQUALTO;   }
"<<"      { return TOK_SHIFTLEFT;              }
">>"      { return TOK_SHIFTRIGHT;             }
"+"       { return TOK_PLUS;                   }
"-"       { return TOK_MINUS;                  }
"*"       { return TOK_STAR;                   }
"/"       { return TOK_DIVIDE;                 }
"%"       { return TOK_MOD;                    }
"~"       { return TOK_TILDE;                  }
"!"       { return TOK_NOT;                    }
"("       { return TOK_LPAREN;                 }
")"       { return TOK_RPAREN;                 }

"'"(([^\'\n\r\\]+)|(\\(([ntvbrfa\\?'\"])|([0-9]+)|([xX][0-9a-fA-F]+))))"'"   {
      g_strToken = QString::fromUtf8(yytext);
      return TOK_CHARACTER;
   }

0[0-7]*{CONSTSUFFIX}?              {
      g_strToken = QString::fromUtf8(yytext);
      return TOK_OCTALINT;
   }

[1-9][0-9]*{CONSTSUFFIX}?          {
      g_strToken = QString::fromUtf8(yytext);
      return TOK_DECIMALINT;
   }

(0x|0X)[0-9a-fA-F]+{CONSTSUFFIX}?  {
      QString text = QString::fromUtf8(yytext);
      g_strToken   = text.mid(2);
      return TOK_HEXADECIMALINT;
   }

(([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))([eE]([\-\+])?[0-9]+)?([fFlL])? {
      g_strToken = QString::fromUtf8(yytext);
      return TOK_FLOAT;
   }

([0-9]+[eE])([\-\+])?[0-9]+([fFlL])? {
      g_strToken = QString::fromUtf8(yytext);
      return TOK_FLOAT;
   }
.
\n

%%

bool parseconstexp(const QString &fileName, int lineNr, const QString &input)
{
   g_constExpFileName = fileName;
   g_constExpLineNr   = lineNr;

   s_inputString      = input;
   s_inputPosition    = 0;

   QString s = input.simplified();

   if (s == "0L"|| s == "! 1L") {
      return false;

   } else if (s == "1L" || s == "! 0L") {
      return true;

   }

   constexpYYrestart(constexpYYin);
   constexpYYparse();

   return ((long)g_resultValue != 0);
}

extern "C" {
   int constexpYYwrap()
   {
      return 1;
   }
}
