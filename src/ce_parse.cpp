/************************************************************************
*
* Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
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

#include <stdio.h>
#include <stdlib.h>

#include <cppvalue.h>
#include <constexp.h>
#include <message.h>

/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers  */
#define YYPURE 0

/* Push parsers  */
#define YYPUSH 0

/* Pull parsers  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         constexpYYparse
#define yylex           constexpYYlex
#define yyerror         constexpYYerror
#define yylval          constexpYYlval
#define yychar          constexpYYchar
#define yydebug         constexpYYdebug
#define yynerrs         constexpYYnerrs

#define YYSTYPE CPPValue

int constexpYYerror(const char *s)
{
   warn(g_constExpFileName, g_constExpLineNr, "Preprocessing issue while doing constant expression evaluation: %s", s);
   return 0;
}

int constexpYYlex();

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE

/* Put the tokens into the symbol table, so GDB and other debuggers know about them.  */
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

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ || defined __cplusplus || defined Q_OS_WIN)
   typedef signed char yytype_int8;
#else
   typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
   typedef YYTYPE_UINT16 yytype_uint16;
#else
   typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
   typedef YYTYPE_INT16 yytype_int16;
#else
   typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ || defined __cplusplus || defined Q_OS_WIN)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ || defined __cplusplus || defined Q_OS_WIN)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
int yyi;
#endif
{
   return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc, define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined Q_OS_WIN
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined Q_OS_WIN)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
/* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
/* The OS might guarantee only one guard page at the bottom of the stack,
   and a page size can be as small as 4096 bytes.  So we cannot safely
   invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
   to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined Q_OS_WIN)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined Q_OS_WIN)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc {
   yytype_int16 yyss_alloc;
   YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  31
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   63
/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  32
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  16
/* YYNRULES -- Number of rules.  */
#define YYNRULES  44
/* YYNRULES -- Number of states.  */
#define YYNSTATES  73
/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   286

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] = {
   0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
   5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
   15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
   25,    26,    27,    28,    29,    30,    31
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] = {
   0,     0,     3,     5,     7,    13,    15,    19,    21,    25,
   27,    31,    33,    37,    39,    43,    45,    49,    53,    55,
   59,    63,    67,    71,    73,    77,    81,    83,    87,    91,
   93,    97,   101,   105,   107,   110,   113,   116,   119,   121,
   125,   127,   129,   131,   133
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] = {
   33,     0,    -1,    34,    -1,    35,    -1,    35,     3,    35,
   4,    35,    -1,    36,    -1,    35,     5,    36,    -1,    37,
   -1,    36,     6,    37,    -1,    38,    -1,    37,     7,    38,
   -1,    39,    -1,    38,     8,    39,    -1,    40,    -1,    39,
   9,    40,    -1,    41,    -1,    40,    11,    41,    -1,    40,
   10,    41,    -1,    42,    -1,    41,    12,    42,    -1,    41,
   13,    42,    -1,    41,    14,    42,    -1,    41,    15,    42,
   -1,    43,    -1,    42,    16,    43,    -1,    42,    17,    43,
   -1,    44,    -1,    43,    18,    44,    -1,    43,    19,    44,
   -1,    45,    -1,    44,    20,    45,    -1,    44,    21,    45,
   -1,    44,    22,    45,    -1,    46,    -1,    18,    45,    -1,
   19,    45,    -1,    23,    45,    -1,    24,    45,    -1,    47,
   -1,    25,    34,    26,    -1,    27,    -1,    28,    -1,    29,
   -1,    30,    -1,    31,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] = {
   0,    77,    77,    81,    83,    92,    94,   100,   102,   108,
   110,   117,   119,   125,   127,   133,   135,   139,   145,   147,
   151,   155,   160,   167,   169,   173,   179,   181,   192,   205,
   207,   218,   231,   239,   241,   243,   250,   254,   260,   262,
   266,   268,   270,   272,   274
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */

static const char *const yytname[] = {
   "$end", "error", "$undefined", "TOK_QUESTIONMARK", "TOK_COLON",
   "TOK_OR", "TOK_AND", "TOK_BITWISEOR", "TOK_BITWISEXOR", "TOK_AMPERSAND",
   "TOK_NOTEQUAL", "TOK_EQUAL", "TOK_LESSTHAN", "TOK_GREATERTHAN",
   "TOK_LESSTHANOREQUALTO", "TOK_GREATERTHANOREQUALTO", "TOK_SHIFTLEFT",
   "TOK_SHIFTRIGHT", "TOK_PLUS", "TOK_MINUS", "TOK_STAR", "TOK_DIVIDE",
   "TOK_MOD", "TOK_TILDE", "TOK_NOT", "TOK_LPAREN", "TOK_RPAREN",
   "TOK_OCTALINT", "TOK_DECIMALINT", "TOK_HEXADECIMALINT", "TOK_CHARACTER",
   "TOK_FLOAT", "$accept", "start", "constant_expression",
   "logical_or_expression", "logical_and_expression",
   "inclusive_or_expression", "exclusive_or_expression", "and_expression",
   "equality_expression", "relational_expression", "shift_expression",
   "additive_expression", "multiplicative_expression", "unary_expression",
   "primary_expression", "constant", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] = {
   0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
   265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
   275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
   285,   286
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] = {
   0,    32,    33,    34,    34,    35,    35,    36,    36,    37,
   37,    38,    38,    39,    39,    40,    40,    40,    41,    41,
   41,    41,    41,    42,    42,    42,    43,    43,    43,    44,
   44,    44,    44,    45,    45,    45,    45,    45,    46,    46,
   47,    47,    47,    47,    47
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] = {
   0,     2,     1,     1,     5,     1,     3,     1,     3,     1,
   3,     1,     3,     1,     3,     1,     3,     3,     1,     3,
   3,     3,     3,     1,     3,     3,     1,     3,     3,     1,
   3,     3,     3,     1,     2,     2,     2,     2,     1,     3,
   1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] = {
   0,     0,     0,     0,     0,     0,    40,    41,    42,    43,
   44,     0,     2,     3,     5,     7,     9,    11,    13,    15,
   18,    23,    26,    29,    33,    38,    34,    35,    36,    37,
   0,     1,     0,     0,     0,     0,     0,     0,     0,     0,
   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
   0,    39,     0,     6,     8,    10,    12,    14,    17,    16,
   19,    20,    21,    22,    24,    25,    27,    28,    30,    31,
   32,     0,     4
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] = {
   -1,    11,    12,    13,    14,    15,    16,    17,    18,    19,
   20,    21,    22,    23,    24,    25
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -29
static const yytype_int8 yypact[] = {
   -13,   -13,   -13,   -13,   -13,   -13,   -29,   -29,   -29,   -29,
   -29,    13,   -29,     4,    29,     1,    19,    33,    21,     7,
   17,    18,     8,   -29,   -29,   -29,   -29,   -29,   -29,   -29,
   20,   -29,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
   -13,   -29,    34,    29,     1,    19,    33,    21,     7,     7,
   17,    17,    17,    17,    18,    18,     8,     8,   -29,   -29,
   -29,   -13,    45
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] = {
   -29,   -29,    48,   -28,    22,    23,    24,    25,    26,     2,
   -17,     0,     5,    -1,   -29,   -29
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] = {
   26,    27,    28,    29,    52,     1,     2,    32,    35,    33,
   3,     4,     5,    31,     6,     7,     8,     9,    10,    40,
   41,    42,    43,    60,    61,    62,    63,    36,    48,    49,
   50,    38,    39,    44,    45,    34,    46,    47,    71,    33,
   58,    59,    37,    72,    64,    65,    51,    68,    69,    70,
   33,    66,    67,    30,     0,    53,     0,    54,     0,    55,
   0,    56,     0,    57
};

static const yytype_int8 yycheck[] = {
   1,     2,     3,     4,    32,    18,    19,     3,     7,     5,
   23,    24,    25,     0,    27,    28,    29,    30,    31,    12,
   13,    14,    15,    40,    41,    42,    43,     8,    20,    21,
   22,    10,    11,    16,    17,     6,    18,    19,     4,     5,
   38,    39,     9,    71,    44,    45,    26,    48,    49,    50,
   5,    46,    47,     5,    -1,    33,    -1,    34,    -1,    35,
   -1,    36,    -1,    37
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] = {
   0,    18,    19,    23,    24,    25,    27,    28,    29,    30,
   31,    33,    34,    35,    36,    37,    38,    39,    40,    41,
   42,    43,    44,    45,    46,    47,    45,    45,    45,    45,
   34,     0,     3,     5,     6,     7,     8,     9,    10,    11,
   12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
   22,    26,    35,    36,    37,    38,    39,    40,    41,    41,
   42,    42,    42,    42,    43,    43,    44,    44,    45,    45,
   45,     4,    35
};

#define yyerrok	(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY	(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT	goto yyabortlab
#define YYERROR	goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
/* This is here to suppress warnings from the GCC cpp's
   -Wunused-macros.  Normally we don't worry about that warning, but
   some users do, and we want to make it easy for users to remove
   YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("Error: Syntax error, unable to back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ || defined __cplusplus || defined Q_OS_WIN)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const *const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
FILE *yyoutput;
int yytype;
YYSTYPE const *const yyvaluep;
#endif
{
   if (! yyvaluep) {
      return;
   }

# ifdef YYPRINT
   if (yytype < YYNTOKENS) {
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
   }

# else
   YYUSE (yyoutput);

# endif
   switch (yytype) {
      default:
         break;
   }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ || defined __cplusplus || defined Q_OS_WIN)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const *const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
FILE *yyoutput;
int yytype;
YYSTYPE const *const yyvaluep;
#endif
{
   if (yytype < YYNTOKENS) {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
   } else {
      YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);
   }

   yy_symbol_value_print (yyoutput, yytype, yyvaluep);
   YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ || defined __cplusplus || defined Q_OS_WIN)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
yytype_int16 *yybottom;
yytype_int16 *yytop;
#endif
{
   YYFPRINTF (stderr, "Stack now");
   for (; yybottom <= yytop; yybottom++) {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
   }
   YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ || defined __cplusplus || defined Q_OS_WIN)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
YYSTYPE *yyvsp;
int yyrule;
#endif
{
   int yynrhs = yyr2[yyrule];
   int yyi;
   unsigned long int yylno = yyrline[yyrule];
   YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
              yyrule - 1, yylno);
   /* The symbols being reduced.  */
   for (yyi = 0; yyi < yynrhs; yyi++) {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                      );
      YYFPRINTF (stderr, "\n");
   }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ || defined __cplusplus || defined Q_OS_WIN)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
const char *yystr;
#endif
{
   YYSIZE_T yylen;
   for (yylen = 0; yystr[yylen]; yylen++) {
      continue;
   }
   return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ || defined __cplusplus || defined Q_OS_WIN)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
char *yydest;
const char *yysrc;
#endif
{
   char *yyd = yydest;
   const char *yys = yysrc;

   while ((*yyd++ = *yys++) != '\0') {
      continue;
   }

   return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
   if (*yystr == '"') {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
         switch (*++yyp) {
            case '\'':
            case ',':
               goto do_not_strip_quotes;

            case '\\':
               if (*++yyp != '\\') {
                  goto do_not_strip_quotes;
               }
               [[fallthrough]];

            default:
               if (yyres) {
                  yyres[yyn] = *yyp;
               }
               yyn++;
               break;

            case '"':
               if (yyres) {
                  yyres[yyn] = '\0';
               }
               return yyn;
         }
   do_not_strip_quotes:
      ;
   }

   if (! yyres) {
      return yystrlen (yystr);
   }

   return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
   int yyn = yypact[yystate];

   if (! (YYPACT_NINF < yyn && yyn <= YYLAST)) {
      return 0;
   } else {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
                    + sizeof yyexpecting - 1
                    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
                       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
      YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
         if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR) {
            if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM) {
               yycount = 1;
               yysize = yysize0;
               yyformat[sizeof yyunexpected - 1] = '\0';
               break;
            }
            yyarg[yycount++] = yytname[yyx];
            yysize1 = yysize + yytnamerr (0, yytname[yyx]);
            yysize_overflow |= (yysize1 < yysize);
            yysize = yysize1;
            yyfmt = yystpcpy (yyfmt, yyprefix);
            yyprefix = yyor;
         }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow) {
         return YYSIZE_MAXIMUM;
      }

      if (yyresult) {
         /* Avoid sprintf, as that infringes on the user's name space.
            Don't have undefined behavior even if the translation
            produced a string with the wrong number of "%s"s.  */
         char *yyp = yyresult;
         int yyi = 0;
         while ((*yyp = *yyf) != '\0') {
            if (*yyp == '%' && yyf[1] == 's' && yyi < yycount) {
               yyp += yytnamerr (yyp, yyarg[yyi++]);
               yyf += 2;
            } else {
               yyp++;
               yyf++;
            }
         }
      }
      return yysize;
   }
}
#endif /* YYERROR_VERBOSE */

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ || defined __cplusplus || defined Q_OS_WIN)

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
const char *yymsg;
int yytype;
YYSTYPE *yyvaluep;
#endif
{
   YYUSE (yyvaluep);

   if (!yymsg) {
      yymsg = "Deleting";
   }
   YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

   switch (yytype) {

      default:
         break;
   }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ || defined __cplusplus || defined Q_OS_WIN)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ || defined __cplusplus || defined Q_OS_WIN)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{

   int yystate;
   /* Number of tokens to shift before error messages enabled.  */
   int yyerrstatus;

   /* The stacks and their tools:
      `yyss': related to states.
      `yyvs': related to semantic values.

      Refer to the stacks thru separate pointers, to allow yyoverflow
      to reallocate them elsewhere.  */

   /* The state stack.  */
   yytype_int16 yyssa[YYINITDEPTH];
   yytype_int16 *yyss;
   yytype_int16 *yyssp;

   /* The semantic value stack.  */
   YYSTYPE yyvsa[YYINITDEPTH];
   YYSTYPE *yyvs;
   YYSTYPE *yyvsp;

   YYSIZE_T yystacksize;

   int yyn;
   int yyresult;
   /* Lookahead token as an internal (translated) token number.  */
   int yytoken;
   /* The variables used to return semantic value and location from the
      action routines.  */
   YYSTYPE yyval;

#if YYERROR_VERBOSE
   /* Buffer for error messages, and its allocated size.  */
   char yymsgbuf[128];
   char *yymsg = yymsgbuf;
   YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

   /* The number of symbols on the RHS of the reduced rule.
      Keep to zero when no symbol should be popped.  */
   int yylen = 0;

   yytoken = 0;
   yyss = yyssa;
   yyvs = yyvsa;
   yystacksize = YYINITDEPTH;

   YYDPRINTF ((stderr, "Starting parse\n"));

   yystate = 0;
   yyerrstatus = 0;
   yynerrs = 0;
   yychar = YYEMPTY; /* Cause a token to be read.  */

   /* Initialize stack pointers.
      Waste one element of value and location stack
      so that they stay on the same level as the state stack.
      The wasted elements are never initialized.  */
   yyssp = yyss;
   yyvsp = yyvs;

   goto yysetstate;

   /*------------------------------------------------------------.
   | yynewstate -- Push a new state, which is found in yystate.  |
   `------------------------------------------------------------*/
yynewstate:
   /* In all cases, when you get here, the value and location stacks
      have just been pushed.  So pushing a state here evens the stacks.  */
   yyssp++;

yysetstate:
   *yyssp = yystate;

   if (yyss + yystacksize - 1 <= yyssp) {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
         /* Give user a chance to reallocate the stack.  Use copies of
            these so that the &'s don't force the real ones into
            memory.  */
         YYSTYPE *yyvs1 = yyvs;
         yytype_int16 *yyss1 = yyss;

         /* Each stack pointer address is followed by the size of the
            data in use in that stack, in bytes.  This used to be a
            conditional around just the two extra args, but that might
            be undefined if yyoverflow is a macro.  */
         yyoverflow (YY_("memory exhausted"),
                     &yyss1, yysize * sizeof (*yyssp),
                     &yyvs1, yysize * sizeof (*yyvsp),
                     &yystacksize);

         yyss = yyss1;
         yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize) {
         goto yyexhaustedlab;
      }
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize) {
         yystacksize = YYMAXDEPTH;
      }

      {
         yytype_int16 *yyss1 = yyss;
         union yyalloc *yyptr =
                  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
         if (! yyptr) {
            goto yyexhaustedlab;
         }
         YYSTACK_RELOCATE (yyss_alloc, yyss);
         YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
         if (yyss1 != yyssa) {
            YYSTACK_FREE (yyss1);
         }
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n", (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp) {
         YYABORT;
      }
   }

   YYDPRINTF ((stderr, "Entering state %d\n", yystate));

   if (yystate == YYFINAL) {
      YYACCEPT;
   }

   goto yybackup;

   /*-----------.
   | yybackup.  |
   `-----------*/
yybackup:

   /* Do appropriate processing given the current state.  Read a
      lookahead token if we need one and don't already have one.  */

   /* First try to decide what to do without reference to lookahead token.  */
   yyn = yypact[yystate];
   if (yyn == YYPACT_NINF) {
      goto yydefault;
   }

   /* Not known => get a lookahead token if don't already have one.  */

   /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
   if (yychar == YYEMPTY) {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
   }

   if (yychar <= YYEOF) {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
   } else {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
   }

   /* If the proper action on seeing token YYTOKEN is to reduce or to
      detect an error, take that action.  */
   yyn += yytoken;
   if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken) {
      goto yydefault;
   }
   yyn = yytable[yyn];
   if (yyn <= 0) {
      if (yyn == 0 || yyn == YYTABLE_NINF) {
         goto yyerrlab;
      }
      yyn = -yyn;
      goto yyreduce;
   }

   /* Count tokens shifted since error; after three, turn off error
      status.  */
   if (yyerrstatus) {
      yyerrstatus--;
   }

   /* Shift the lookahead token.  */
   YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

   /* Discard the shifted token.  */
   yychar = YYEMPTY;

   yystate = yyn;
   *++yyvsp = yylval;

   goto yynewstate;


   /*-----------------------------------------------------------.
   | yydefault -- do the default action for the current state.  |
   `-----------------------------------------------------------*/
yydefault:
   yyn = yydefact[yystate];
   if (yyn == 0) {
      goto yyerrlab;
   }
   goto yyreduce;


   /*-----------------------------.
   | yyreduce -- Do a reduction.  |
   `-----------------------------*/
yyreduce:
   /* yyn is the number of a rule to reduce with.  */
   yylen = yyr2[yyn];

   /* If YYLEN is nonzero, implement the default value of the action:
      `$$ = $1'.

      Otherwise, the following line sets YYVAL to garbage.
      This behavior is undocumented and Bison
      users should not rely upon it.  Assigning to YYVAL
      unconditionally makes the parser a bit smaller, and it avoids a
      GCC warning that YYVAL may be used uninitialized.  */
   yyval = yyvsp[1 - yylen];

   YY_REDUCE_PRINT (yyn);
   switch (yyn) {
      case 2: {
            g_resultValue = (yyvsp[(1) - (1)]);
            return 0;
         }

         break;

      case 3: {
            (yyval) = (yyvsp[(1) - (1)]);
         }

         break;

      case 4: {
            bool c = ((yyvsp[(1) - (5)]).isInt() ? ((long)(yyvsp[(1) - (5)]) != 0) : ((double)(yyvsp[(1) - (5)]) != 0.0));
            (yyval) = c ? (yyvsp[(3) - (5)]) : (yyvsp[(5) - (5)]);
         }
         break;

      case 5: {
            (yyval) = (yyvsp[(1) - (1)]);
         }
         break;

      case 6:

         {
            (yyval) = CPPValue( (long)((long)(yyvsp[(1) - (3)]) || (long)(yyvsp[(3) - (3)])) );
         }
         break;

      case 7:

         {
            (yyval) = (yyvsp[(1) - (1)]);
         }
         break;

      case 8:

         {
            (yyval) = CPPValue( (long)((long)(yyvsp[(1) - (3)]) && (long)(yyvsp[(3) - (3)])) );
         }
         break;

      case 9:

         {
            (yyval) = (yyvsp[(1) - (1)]); ;
         }
         break;

      case 10:

         {
            (yyval) = CPPValue( (long)(yyvsp[(1) - (3)]) | (long)(yyvsp[(3) - (3)]) );
         }
         break;

      case 11:

         {
            (yyval) = (yyvsp[(1) - (1)]); ;
         }
         break;

      case 12:

         {
            (yyval) = CPPValue( (long)(yyvsp[(1) - (3)]) ^ (long)(yyvsp[(3) - (3)]) );
         }
         break;

      case 13:

         {
            (yyval) = (yyvsp[(1) - (1)]); ;
         }
         break;

      case 14:

         {
            (yyval) = CPPValue( (long)(yyvsp[(1) - (3)]) & (long)(yyvsp[(3) - (3)]) );
            ;
         }
         break;

      case 15:

         {
            (yyval) = (yyvsp[(1) - (1)]);
         }
         break;

      case 16:

         {
            (yyval) = CPPValue( (long)((double)(yyvsp[(1) - (3)]) == (double)(yyvsp[(3) - (3)])) );
         }
         break;

      case 17:

         {
            (yyval) = CPPValue( (long)((double)(yyvsp[(1) - (3)]) != (double)(yyvsp[(3) - (3)])) );
         }
         break;

      case 18:

         {
            (yyval) = (yyvsp[(1) - (1)]); ;
         }
         break;

      case 19:

         {
            (yyval) = CPPValue( (long)((double)(yyvsp[(1) - (3)]) < (double)(yyvsp[(3) - (3)])) );
         }
         break;

      case 20:

         {
            (yyval) = CPPValue( (long)((double)(yyvsp[(1) - (3)]) > (double)(yyvsp[(3) - (3)])) );
         }
         break;

      case 21:

         {
            (yyval) = CPPValue( (long)((double)(yyvsp[(1) - (3)]) <= (double)(yyvsp[(3) - (3)])) );
         }
         break;

      case 22:

         {
            (yyval) = CPPValue( (long)((double)(yyvsp[(1) - (3)]) >= (double)(yyvsp[(3) - (3)])) );
         }
         break;

      case 23:

         {
            (yyval) = (yyvsp[(1) - (1)]); ;
         }
         break;

      case 24:

         {
            (yyval) = CPPValue( (long)(yyvsp[(1) - (3)]) << (long)(yyvsp[(3) - (3)]) );
            ;
         }
         break;

      case 25:

         {
            (yyval) = CPPValue( (long)(yyvsp[(1) - (3)]) >> (long)(yyvsp[(3) - (3)]) );
         }
         break;

      case 26:

         {
            (yyval) = (yyvsp[(1) - (1)]);
         }
         break;

      case 27:

         {
            if (!(yyvsp[(1) - (3)]).isInt() || !(yyvsp[(3) - (3)]).isInt()) {
               (yyval) = CPPValue( (double)(yyvsp[(1) - (3)]) + (double)(yyvsp[(3) - (3)]) );
            } else {
               (yyval) = CPPValue( (long)(yyvsp[(1) - (3)]) + (long)(yyvsp[(3) - (3)]) );
            }
            ;
         }
         break;

      case 28:

         {
            if (!(yyvsp[(1) - (3)]).isInt() || !(yyvsp[(3) - (3)]).isInt()) {
               (yyval) = CPPValue( (double)(yyvsp[(1) - (3)]) - (double)(yyvsp[(3) - (3)]) );
            } else {
               (yyval) = CPPValue( (long)(yyvsp[(1) - (3)]) - (long)(yyvsp[(3) - (3)]) );
            }
            ;
         }
         break;

      case 29:

         {
            (yyval) = (yyvsp[(1) - (1)]);
         }
         break;

      case 30:

         {
            if (!(yyvsp[(1) - (3)]).isInt() || !(yyvsp[(3) - (3)]).isInt()) {
               (yyval) = CPPValue( (double)(yyvsp[(1) - (3)]) * (double)(yyvsp[(3) - (3)]) );
            } else {
               (yyval) = CPPValue( (long)(yyvsp[(1) - (3)]) * (long)(yyvsp[(3) - (3)]) );
            }
         }
         break;

      case 31:

         {
            if (!(yyvsp[(1) - (3)]).isInt() || !(yyvsp[(3) - (3)]).isInt()) {
               (yyval) = CPPValue( (double)(yyvsp[(1) - (3)]) / (double)(yyvsp[(3) - (3)]) );
            } else {
               long value = (yyvsp[(3) - (3)]);
               if (value == 0) {
                  value = 1;
               }
               (yyval) = CPPValue( (long)(yyvsp[(1) - (3)]) / value );
            }
         }
         break;

      case 32:

         {
            long value = (yyvsp[(3) - (3)]);
            if (value == 0) {
               value = 1;
            }
            (yyval) = CPPValue( (long)(yyvsp[(1) - (3)]) % value );
         }
         break;

      case 33:

         {
            (yyval) = (yyvsp[(1) - (1)]); ;
         }
         break;

      case 34:

         {
            (yyval) = (yyvsp[(1) - (2)]); ;
         }
         break;

      case 35:

         {
            if ((yyvsp[(2) - (2)]).isInt()) {
               (yyval) = CPPValue(-(long)(yyvsp[(2) - (2)]));
            } else {
               (yyval) = CPPValue(-(double)(yyvsp[(2) - (2)]));
            }
         }
         break;

      case 36:

         {
            (yyval) = CPPValue(~(long)(yyvsp[(2) - (2)]));
         }
         break;

      case 37:

         {
            (yyval) = CPPValue((long)!(long)(yyvsp[(2) - (2)]));
         }
         break;

      case 38:

         {
            (yyval) = (yyvsp[(1) - (1)]); ;
         }
         break;

      case 39:

         {
            (yyval) = (yyvsp[(2) - (3)]); ;
         }
         break;

      case 40:

         {
            (yyval) = parseOctal(); ;
         }
         break;

      case 41:

         {
            (yyval) = parseDecimal(); ;
         }
         break;

      case 42:

         {
            (yyval) = parseHexadecimal(); ;
         }
         break;

      case 43:

         {
            (yyval) = parseCharacter(); ;
         }
         break;

      case 44:

         {
            (yyval) = parseFloat(); ;
         }
         break;


      default:
         break;
   }
   YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

   YYPOPSTACK (yylen);
   yylen = 0;
   YY_STACK_PRINT (yyss, yyssp);

   *++yyvsp = yyval;

   /* Now `shift' the result of the reduction.  Determine what state
      that goes to, based on the state we popped back to and the rule
      number reduced by.  */

   yyn = yyr1[yyn];

   yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
   if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp) {
      yystate = yytable[yystate];
   } else {
      yystate = yydefgoto[yyn - YYNTOKENS];
   }

   goto yynewstate;


   /*------------------------------------.
   | yyerrlab -- here on detecting error |
   `------------------------------------*/
yyerrlab:
   /* If not already recovering from an error, report this error.  */
   if (!yyerrstatus) {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
         YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
         if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM) {
            YYSIZE_T yyalloc = 2 * yysize;
            if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM)) {
               yyalloc = YYSTACK_ALLOC_MAXIMUM;
            }
            if (yymsg != yymsgbuf) {
               YYSTACK_FREE (yymsg);
            }
            yymsg = (char *) YYSTACK_ALLOC (yyalloc);
            if (yymsg) {
               yymsg_alloc = yyalloc;
            } else {
               yymsg = yymsgbuf;
               yymsg_alloc = sizeof yymsgbuf;
            }
         }

         if (0 < yysize && yysize <= yymsg_alloc) {
            (void) yysyntax_error (yymsg, yystate, yychar);
            yyerror (yymsg);
         } else {
            yyerror (YY_("syntax error"));
            if (yysize != 0) {
               goto yyexhaustedlab;
            }
         }
      }
#endif
   }



   if (yyerrstatus == 3) {
      /* If just tried and failed to reuse lookahead token after an
      error, discard it.  */

      if (yychar <= YYEOF) {
         /* Return failure if at end of input.  */
         if (yychar == YYEOF) {
            YYABORT;
         }
      } else {
         yydestruct ("Error: discarding",
                     yytoken, &yylval);
         yychar = YYEMPTY;
      }
   }

   /* Else will try to reuse lookahead token after shifting the error
      token.  */
   goto yyerrlab1;


   /*---------------------------------------------------.
   | yyerrorlab -- error raised explicitly by YYERROR.  |
   `---------------------------------------------------*/
yyerrorlab:

   /* Pacify compilers like GCC when the user code never invokes
      YYERROR and the label yyerrorlab therefore never appears in user
      code.  */
   if (/*CONSTCOND*/ 0) {
      goto yyerrorlab;
   }

   /* Do not reclaim the symbols of the rule which action triggered
      this YYERROR.  */
   YYPOPSTACK (yylen);
   yylen = 0;
   YY_STACK_PRINT (yyss, yyssp);
   yystate = *yyssp;
   goto yyerrlab1;


   /*-------------------------------------------------------------.
   | yyerrlab1 -- common code for both syntax error and YYERROR.  |
   `-------------------------------------------------------------*/
yyerrlab1:
   yyerrstatus = 3;	/* Each real token shifted decrements this.  */

   for (;;) {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF) {
         yyn += YYTERROR;
         if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR) {
            yyn = yytable[yyn];
            if (0 < yyn) {
               break;
            }
         }
      }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss) {
         YYABORT;
      }


      yydestruct ("Error: popping", yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
   }

   *++yyvsp = yylval;


   /* Shift the error token.  */
   YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

   yystate = yyn;
   goto yynewstate;


   /*-------------------------------------.
   | yyacceptlab -- YYACCEPT comes here.  |
   `-------------------------------------*/
yyacceptlab:
   yyresult = 0;
   goto yyreturn;

   /*-----------------------------------.
   | yyabortlab -- YYABORT comes here.  |
   `-----------------------------------*/
yyabortlab:
   yyresult = 1;
   goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
   /*-------------------------------------------------.
   | yyexhaustedlab -- memory exhaustion comes here.  |
   `-------------------------------------------------*/
yyexhaustedlab:
   yyerror (YY_("memory exhausted"));
   yyresult = 2;
   /* Fall through.  */
#endif

yyreturn:
   if (yychar != YYEMPTY)
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
   /* Do not reclaim the symbols of the rule which action triggered
      this YYABORT or YYACCEPT.  */
   YYPOPSTACK (yylen);
   YY_STACK_PRINT (yyss, yyssp);
   while (yyssp != yyss) {
      yydestruct ("Cleanup: popping", yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
   }
#ifndef yyoverflow
   if (yyss != yyssa) {
      YYSTACK_FREE (yyss);
   }
#endif
#if YYERROR_VERBOSE
   if (yymsg != yymsgbuf) {
      YYSTACK_FREE (yymsg);
   }
#endif
   /* Make sure YYID is used.  */
   return YYID (yyresult);
}

