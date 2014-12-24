/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_QUESTIONMARK = 258,
     TOK_COLON = 259,
     TOK_OR = 260,
     TOK_AND = 261,
     TOK_BITWISEOR = 262,
     TOK_BITWISEXOR = 263,
     TOK_AMPERSAND = 264,
     TOK_NOTEQUAL = 265,
     TOK_EQUAL = 266,
     TOK_LESSTHAN = 267,
     TOK_GREATERTHAN = 268,
     TOK_LESSTHANOREQUALTO = 269,
     TOK_GREATERTHANOREQUALTO = 270,
     TOK_SHIFTLEFT = 271,
     TOK_SHIFTRIGHT = 272,
     TOK_PLUS = 273,
     TOK_MINUS = 274,
     TOK_STAR = 275,
     TOK_DIVIDE = 276,
     TOK_MOD = 277,
     TOK_TILDE = 278,
     TOK_NOT = 279,
     TOK_LPAREN = 280,
     TOK_RPAREN = 281,
     TOK_OCTALINT = 282,
     TOK_DECIMALINT = 283,
     TOK_HEXADECIMALINT = 284,
     TOK_CHARACTER = 285,
     TOK_FLOAT = 286
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE ce_parsexpYYlval;


