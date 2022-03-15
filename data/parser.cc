/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 2 "parser.yy"


/***************************************************************************
                          parser.yy  -  description
                             -------------------
    begin                : Mit Jul 12 22:54:51 MEST 2000
    copyright            : (C) 2000 by Immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2002,2003,2005,2006,2008,2010,2011 by the cuyo developers

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cstdlib>

#include "cuyointl.h"
#include "code.h"
#include "fehler.h"
#include "knoten.h"
#include "variable.h"
#include "ort.h"
#include "version.h"
#include "global.h"
#include "leveldaten.h"
#include "sound.h"

#define YYMALLOC malloc
#define YYFREE free


/***********************************************************************/
/* Globale Parse-Variablen */

/** Für Fehlerausgabe: Aktueller Dateiname */
Str gDateiName;
/** Für Fehlerausgabe: Aktuelle Zeilen-Nummer */
int gZeilenNr;

/** True, wenn es während des Parsens (mindestens) einen Fehler gab. */
bool gGabFehler;


/** Wenn der Parser aufgerufen wird, muss in DefKnoten schon ein DefKnoten
    stehen, an den alles geparste angefügt wird. Normalerweise erzeugt man
    einen neuen Defknoten. Beim Includen will man da aber schon was drin
    haben.
    Siehe auch %type <defknoten> alles */
static DefKnoten * gAktDefKnoten;

//#define MAX_INCLUDE_TIEFE 16
/** YY_BUFFER_STATE ist ein flex-Datentyp für eine Datei, an der grade
    geparst wird. */
//static YY_BUFFER_STATE gIncludeStack[MAX_INCLUDE_TIEFE];
/** Aktuelle Include-Tiefe. (0 bei Hauptdatei) */
//static int gIncludeTiefe;

//static DefKnoten * gIncludeMerk;


/* Beim Erzeugen eines Codes müssen einige Variablen jedes Mal übergeben
   werden:
   - der zugehörige DefKnoten, damit ggf. noch Variablen reserviert werden
     können (genauer: die Busy-Variable).
   - Dateiname und Zeilennummer, damit der Code schönere Fehlermeldungen
     ausgeben kann.
   Damit ich das aber nicht jedes Mal eintippen muss, hier ein paar Macros:
 */
#define newCode0(ART) new Code(\
  gAktDefKnoten, gDateiName, gZeilenNr, ART)
#define newCode1(ART, X1) new Code(\
  gAktDefKnoten, gDateiName, gZeilenNr, ART, X1)
#define newCode2(ART, X1, X2) new Code(\
  gAktDefKnoten, gDateiName, gZeilenNr, ART, X1, X2)
#define newCode3(ART, X1, X2, X3) new Code(\
  gAktDefKnoten, gDateiName, gZeilenNr, ART, X1, X2, X3)
#define newCode4(ART, X1, X2, X3, X4) new Code(\
  gAktDefKnoten, gDateiName, gZeilenNr, ART, X1, X2, X3, X4)



/***********************************************************************/


#define VIEL 32767


/* PBEGIN_TRY und PEND_TRY() fangen Fehler ab und geben sie aus, damit
   weitergeparst werden kann und mehrere Fehler gleichzeitig ausgegeben
   werden können. Sie werden in fehler.h definiert. */


#define YYDEBUG 1


/* Bug in Bison? Er scheint den Stack nicht automatisch vergrößern zu wollen,
   wenn er voll ist. Es sieht so aus, als hätte er angst, dass da c++-Variablen
   vorkommen, wo er (wegen constructor und so) nicht einfach so rumalloziieren
   kann. (Ich weiß nicht, was der LTYPE ist, von dem er nicht festgestellt hat,
   das er trivial ist.) */
#define YYLTYPE_IS_TRIVIAL 1



/* Line 268 of yacc.c  */
#line 184 "parser.cc"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IF_PREC = 258,
     ELSE_TOK = 259,
     OR_TOK = 260,
     AND_TOK = 261,
     LE_TOK = 262,
     GE_TOK = 263,
     NE_TOK = 264,
     EQ_TOK = 265,
     BIS_TOK = 266,
     BITUNSET_ATOK = 267,
     BITSET_ATOK = 268,
     NEG_PREC = 269,
     INCLUDE_TOK = 270,
     BEGIN_CODE_TOK = 271,
     END_CODE_TOK = 272,
     SWITCH_TOK = 273,
     IF_TOK = 274,
     VAR_TOK = 275,
     BUSY_TOK = 276,
     ADD_TOK = 277,
     SUB_TOK = 278,
     MUL_TOK = 279,
     DIV_TOK = 280,
     MOD_TOK = 281,
     BITSET_TOK = 282,
     BITUNSET_TOK = 283,
     RND_TOK = 284,
     GGT_TOK = 285,
     BONUS_TOK = 286,
     MESSAGE_TOK = 287,
     SOUND_TOK = 288,
     EXPLODE_TOK = 289,
     VERLIER_TOK = 290,
     DEFAULT_TOK = 291,
     DA_KIND_TOK = 292,
     FREMD_TOK = 293,
     REINWORT_TOK = 294,
     WORT_TOK = 295,
     NACHBAR8_TOK = 296,
     NACHBAR6_TOK = 297,
     NULLEINS_TOK = 298,
     ZAHL_TOK = 299,
     HALBZAHL_TOK = 300,
     BUCHSTABE_TOK = 301,
     PFEIL_TOK = 302
   };
#endif
/* Tokens.  */
#define IF_PREC 258
#define ELSE_TOK 259
#define OR_TOK 260
#define AND_TOK 261
#define LE_TOK 262
#define GE_TOK 263
#define NE_TOK 264
#define EQ_TOK 265
#define BIS_TOK 266
#define BITUNSET_ATOK 267
#define BITSET_ATOK 268
#define NEG_PREC 269
#define INCLUDE_TOK 270
#define BEGIN_CODE_TOK 271
#define END_CODE_TOK 272
#define SWITCH_TOK 273
#define IF_TOK 274
#define VAR_TOK 275
#define BUSY_TOK 276
#define ADD_TOK 277
#define SUB_TOK 278
#define MUL_TOK 279
#define DIV_TOK 280
#define MOD_TOK 281
#define BITSET_TOK 282
#define BITUNSET_TOK 283
#define RND_TOK 284
#define GGT_TOK 285
#define BONUS_TOK 286
#define MESSAGE_TOK 287
#define SOUND_TOK 288
#define EXPLODE_TOK 289
#define VERLIER_TOK 290
#define DEFAULT_TOK 291
#define DA_KIND_TOK 292
#define FREMD_TOK 293
#define REINWORT_TOK 294
#define WORT_TOK 295
#define NACHBAR8_TOK 296
#define NACHBAR6_TOK 297
#define NULLEINS_TOK 298
#define ZAHL_TOK 299
#define HALBZAHL_TOK 300
#define BUCHSTABE_TOK 301
#define PFEIL_TOK 302




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 116 "parser.yy"

  Code * code;
  Code * codepaar[2];
  Str * str;
  int zahl;
  int zahlpaar[2];
  Knoten * knoten;
  DefKnoten * defknoten;
  ListenKnoten * listenknoten;
  WortKnoten * wortknoten;
  Variable * variable;
  Ort * ort;
  Version * version;
  CodeArt codeart;
  OrtHaelfte haelfte;



/* Line 293 of yacc.c  */
#line 333 "parser.cc"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */

/* Line 343 of yacc.c  */
#line 211 "parser.yy"

int yyerror(const char * s);
int yylex(YYSTYPE * lvalPtr);

/** Wechselt die Lex-Datei temporär. Liefert was zurück, was an popLex
    übergeben werden muss, um wieder zurückzuschalten. Throwt evtl.
    setzdefault wird an den Pfaditerator übergeben.
    Wird in lex.ll definiert, weil da die nötigen Lex-Dinge definiert sind. */
void * pushLex(const char * na, bool setzdefault = false);
void popLex(void * merkBuf);


/* Line 343 of yacc.c  */
#line 358 "parser.cc"

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
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

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

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
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
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   826

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  70
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  46
/* YYNRULES -- Number of rules.  */
#define YYNRULES  160
/* YYNRULES -- Number of states.  */
#define YYNSTATES  280

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   302

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    17,     2,     2,     2,    23,    24,     2,
      67,    68,    21,    18,     7,    19,    29,    22,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    20,     3,
      10,    63,    11,     2,    69,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    64,     2,     4,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    65,    25,    66,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     5,     6,
       8,     9,    12,    13,    14,    15,    16,    26,    27,    28,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,    10,    15,    19,    21,    23,    25,
      29,    30,    34,    36,    40,    41,    46,    48,    50,    54,
      56,    58,    62,    64,    66,    70,    73,    77,    81,    85,
      89,    93,    94,    97,   103,   107,   111,   113,   115,   117,
     121,   123,   127,   128,   131,   135,   137,   139,   141,   145,
     150,   152,   156,   161,   166,   173,   181,   185,   189,   191,
     193,   196,   198,   201,   202,   204,   211,   213,   218,   223,
     228,   230,   232,   236,   238,   240,   242,   244,   246,   248,
     250,   252,   254,   257,   260,   262,   265,   267,   272,   280,
     286,   288,   292,   296,   298,   300,   302,   304,   308,   310,
     312,   316,   320,   323,   327,   331,   335,   339,   343,   347,
     351,   355,   359,   363,   367,   371,   375,   379,   383,   386,
     390,   394,   398,   403,   410,   413,   416,   420,   422,   424,
     426,   429,   431,   433,   435,   437,   439,   441,   442,   444,
     445,   447,   451,   453,   457,   463,   464,   466,   467,   469,
     473,   475,   479,   485,   488,   491,   493,   495,   497,   500,
     502
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      71,     0,    -1,    -1,    71,    98,    74,    63,    76,    -1,
      71,    31,    81,    32,    -1,    71,    30,    98,    -1,    99,
      -1,   113,    -1,    72,    -1,    72,     7,    73,    -1,    -1,
      64,    73,     4,    -1,   115,    -1,    10,    80,    11,    -1,
      -1,    65,    77,    71,    66,    -1,    78,    -1,    79,    -1,
      78,     7,    79,    -1,    98,    -1,    75,    -1,    98,    21,
      75,    -1,   113,    -1,    99,    -1,    67,    80,    68,    -1,
      19,    80,    -1,    80,    18,    80,    -1,    80,    19,    80,
      -1,    80,    21,    80,    -1,    80,    22,    80,    -1,    80,
      23,    80,    -1,    -1,    81,    82,    -1,    83,    74,    63,
      92,     3,    -1,    35,    84,     3,    -1,    51,    89,     3,
      -1,    98,    -1,    61,    -1,    87,    -1,    84,     7,    87,
      -1,    80,    -1,    80,    20,    52,    -1,    -1,    63,    85,
      -1,    88,    74,    86,    -1,    99,    -1,    61,    -1,    90,
      -1,    89,     7,    90,    -1,    88,    74,    63,    85,    -1,
      92,    -1,    92,     3,    91,    -1,    33,    65,    97,    66,
      -1,    34,   100,    62,    92,    -1,    34,   100,    62,    92,
       6,    92,    -1,    34,   100,    62,    92,     6,    62,    92,
      -1,    65,    91,    66,    -1,    92,     7,    92,    -1,   113,
      -1,    96,    -1,   113,    96,    -1,    98,    -1,    24,    98,
      -1,    -1,    93,    -1,    64,   102,    63,   100,     4,    92,
      -1,    36,    -1,    46,    67,   100,    68,    -1,    47,    67,
      98,    68,    -1,    48,    67,    98,    68,    -1,    50,    -1,
      49,    -1,   103,    94,   100,    -1,    63,    -1,    37,    -1,
      38,    -1,    39,    -1,    40,    -1,    41,    -1,    42,    -1,
      43,    -1,    21,    -1,    21,   112,    -1,   112,    21,    -1,
      61,    -1,    61,    95,    -1,    95,    -1,   100,    62,    92,
       3,    -1,   100,    62,    92,     3,    62,    92,     3,    -1,
     100,    62,    92,     3,    97,    -1,    99,    -1,    98,    29,
      99,    -1,    98,    29,    61,    -1,    55,    -1,    54,    -1,
     103,    -1,   113,    -1,    67,   100,    68,    -1,    56,    -1,
      57,    -1,   100,    20,   100,    -1,   100,    18,   100,    -1,
      19,   100,    -1,   100,    19,   100,    -1,   100,    21,   100,
      -1,   100,    22,   100,    -1,   100,    23,   100,    -1,   100,
      27,   100,    -1,   100,    26,   100,    -1,   100,    29,   100,
      -1,   100,    15,   100,    -1,   100,    14,   100,    -1,   100,
      13,   100,    -1,   100,    12,   100,    -1,   100,    11,   100,
      -1,   100,    10,   100,    -1,   100,    24,   100,    -1,   100,
      25,   100,    -1,    17,   100,    -1,   100,     9,   100,    -1,
     100,     8,   100,    -1,   100,    15,   101,    -1,    44,    67,
     100,    68,    -1,    45,    67,   100,     7,   100,    68,    -1,
     100,    16,    -1,    16,   100,    -1,   100,    16,   100,    -1,
      99,    -1,    61,    -1,   102,    -1,    99,   112,    -1,   100,
      -1,   114,    -1,    63,    -1,    17,    -1,    10,    -1,    11,
      -1,    -1,    58,    -1,    -1,   100,    -1,   104,     7,   104,
      -1,   106,    -1,    67,   107,    68,    -1,    67,   107,     3,
     105,    68,    -1,    -1,    58,    -1,    -1,   100,    -1,   104,
       7,   104,    -1,   109,    -1,    67,   110,    68,    -1,    67,
     110,     3,   105,    68,    -1,    69,   111,    -1,    53,   108,
      -1,    59,    -1,    58,    -1,    60,    -1,    19,    60,    -1,
     113,    -1,    19,   113,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   232,   232,   233,   241,   252,   295,   296,   302,   307,
     315,   316,   320,   321,   324,   324,   340,   343,   347,   353,
     356,   359,   365,   366,   381,   382,   383,   384,   385,   386,
     387,   396,   397,   400,   405,   406,   416,   417,   427,   428,
     432,   433,   437,   438,   442,   452,   453,   464,   465,   469,
     488,   489,   493,   494,   499,   508,   516,   517,   518,   519,
     520,   523,   532,   542,   543,   544,   553,   554,   557,   561,
     565,   568,   574,   584,   585,   586,   587,   588,   589,   590,
     591,   596,   597,   598,   602,   603,   607,   612,   617,   622,
     633,   634,   638,   644,   645,   652,   661,   662,   663,   666,
     673,   674,   675,   676,   677,   678,   679,   680,   681,   682,
     683,   684,   685,   686,   687,   688,   689,   690,   691,   692,
     693,   694,   697,   700,   707,   708,   709,   713,   724,   734,
     735,   748,   749,   753,   754,   755,   756,   760,   761,   765,
     766,   767,   771,   772,   773,   780,   781,   785,   786,   787,
     791,   792,   793,   800,   801,   808,   809,   815,   819,   822,
     823
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "';'", "']'", "IF_PREC", "ELSE_TOK",
  "','", "OR_TOK", "AND_TOK", "'<'", "'>'", "LE_TOK", "GE_TOK", "NE_TOK",
  "EQ_TOK", "BIS_TOK", "'!'", "'+'", "'-'", "':'", "'*'", "'/'", "'%'",
  "'&'", "'|'", "BITUNSET_ATOK", "BITSET_ATOK", "NEG_PREC", "'.'",
  "INCLUDE_TOK", "BEGIN_CODE_TOK", "END_CODE_TOK", "SWITCH_TOK", "IF_TOK",
  "VAR_TOK", "BUSY_TOK", "ADD_TOK", "SUB_TOK", "MUL_TOK", "DIV_TOK",
  "MOD_TOK", "BITSET_TOK", "BITUNSET_TOK", "RND_TOK", "GGT_TOK",
  "BONUS_TOK", "MESSAGE_TOK", "SOUND_TOK", "EXPLODE_TOK", "VERLIER_TOK",
  "DEFAULT_TOK", "DA_KIND_TOK", "FREMD_TOK", "REINWORT_TOK", "WORT_TOK",
  "NACHBAR8_TOK", "NACHBAR6_TOK", "NULLEINS_TOK", "ZAHL_TOK",
  "HALBZAHL_TOK", "BUCHSTABE_TOK", "PFEIL_TOK", "'='", "'['", "'{'", "'}'",
  "'('", "')'", "'@'", "$accept", "alles", "versionsmerkmal",
  "versionierung", "version", "ld_konstante", "rechts_von_def", "@1",
  "def_liste", "def_liste_eintrag", "konstante", "code_modus",
  "code_zeile", "proc_def_wort", "var_liste", "echter_default",
  "unechter_default", "var_def", "var_def_wort", "default_liste",
  "default_def", "code", "code_1", "set_zeile", "zuweisungs_operator",
  "stern_at", "buch_stern", "auswahl_liste", "punktwort", "wort",
  "ausdruck", "intervall", "lokale_variable", "variable", "halbort",
  "haelften_spez", "absort_klammerfrei", "absort_geklammert", "absort",
  "relort_klammerfrei", "relort_geklammert", "relort", "ort", "zahl",
  "halbzahl", "vorzeichen_zahl", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,    59,    93,   258,   259,    44,   260,   261,
      60,    62,   262,   263,   264,   265,   266,    33,    43,    45,
      58,    42,    47,    37,    38,   124,   267,   268,   269,    46,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,    61,    91,   123,   125,    40,    41,    64
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    70,    71,    71,    71,    71,    72,    72,    73,    73,
      74,    74,    75,    75,    77,    76,    76,    78,    78,    79,
      79,    79,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    81,    81,    82,    82,    82,    83,    83,    84,    84,
      85,    85,    86,    86,    87,    88,    88,    89,    89,    90,
      91,    91,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    93,    94,    94,    94,    94,    94,    94,    94,
      94,    95,    95,    95,    96,    96,    96,    97,    97,    97,
      98,    98,    98,    99,    99,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   101,   101,   101,   102,   102,   103,
     103,   104,   104,   105,   105,   105,   105,   106,   106,   107,
     107,   107,   108,   108,   108,   109,   109,   110,   110,   110,
     111,   111,   111,   112,   112,   113,   113,   114,   114,   115,
     115
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     5,     4,     3,     1,     1,     1,     3,
       0,     3,     1,     3,     0,     4,     1,     1,     3,     1,
       1,     3,     1,     1,     3,     2,     3,     3,     3,     3,
       3,     0,     2,     5,     3,     3,     1,     1,     1,     3,
       1,     3,     0,     2,     3,     1,     1,     1,     3,     4,
       1,     3,     4,     4,     6,     7,     3,     3,     1,     1,
       2,     1,     2,     0,     1,     6,     1,     4,     4,     4,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     1,     2,     1,     4,     7,     5,
       1,     3,     3,     1,     1,     1,     1,     3,     1,     1,
       3,     3,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     3,
       3,     3,     4,     6,     2,     2,     3,     1,     1,     1,
       2,     1,     1,     1,     1,     1,     1,     0,     1,     0,
       1,     3,     1,     3,     5,     0,     1,     0,     1,     3,
       1,     3,     5,     2,     2,     1,     1,     1,     2,     1,
       2
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     0,    31,    94,    93,    10,    90,     5,
       0,     0,     0,     0,     4,     0,     0,    37,    32,    10,
      36,    92,    91,   156,   155,     8,     0,     6,     7,     0,
      46,     0,    38,    10,    45,    10,     0,    47,     0,     0,
      11,     0,     0,    14,    20,     3,    16,    17,    19,   159,
      12,    34,     0,    42,     0,    35,     0,    63,     9,     0,
       0,     0,    23,    22,   160,     2,     0,     0,    39,     0,
      44,     0,    48,    81,     0,     0,     0,    66,     0,     0,
       0,    71,    70,   137,   128,     0,    63,   145,     0,    64,
      86,    59,    61,   127,   129,     0,     0,    58,    25,     0,
      13,     0,     0,     0,     0,     0,     0,    18,    21,    40,
      43,    49,    82,    62,     0,     0,     0,     0,     0,    98,
      99,   128,     0,   127,     0,    95,    96,     0,     0,     0,
     138,   139,   142,   154,    85,   127,     0,     0,    50,   146,
     147,   150,   153,    33,    63,   130,    74,    75,    76,    77,
      78,    79,    80,    73,     0,    83,    84,    60,    24,    26,
      27,    28,    29,    30,    15,     0,     0,     0,   118,   102,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    63,     0,     0,     0,     0,   157,   140,     0,
       0,   132,     0,    56,    63,   148,     0,     0,    57,    72,
      41,    52,    63,     0,     0,    97,   120,   119,   115,   114,
     113,   112,   111,     0,   110,   121,   101,   103,   100,   104,
     105,   106,   116,   117,   108,   107,   109,    53,    67,    68,
      69,   158,     0,     0,   143,     0,    51,     0,     0,   151,
       0,   122,     0,   125,   124,    63,   131,   141,   135,   136,
     134,   133,     0,    63,   149,     0,    87,     0,   126,    63,
      54,   144,    65,   152,    63,    89,   123,    55,     0,    88
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    25,    26,    13,    44,    45,    65,    46,    47,
     109,    10,    18,    19,    31,   110,    70,    32,    33,    36,
      37,   137,   138,    89,   154,    90,    91,   166,    92,   123,
     167,   225,    94,   125,   199,   262,   132,   200,   133,   141,
     207,   142,    96,   126,   201,    50
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -134
static const yytype_int16 yypact[] =
{
    -134,   132,  -134,   112,  -134,  -134,  -134,   -16,  -134,     2,
     573,   153,   220,   -21,  -134,   204,   204,  -134,  -134,   -25,
       2,  -134,  -134,  -134,  -134,    47,    45,  -134,  -134,   123,
    -134,    29,  -134,   -25,  -134,   -25,    54,  -134,    11,   220,
    -134,   227,    87,  -134,  -134,  -134,    57,  -134,     0,  -134,
    -134,  -134,   204,    33,    55,  -134,   204,   548,  -134,   227,
     227,   568,  -134,  -134,  -134,  -134,   212,    76,  -134,   227,
    -134,   227,  -134,   -45,   112,   104,   493,  -134,   118,   122,
     125,  -134,  -134,    52,    13,   237,   548,    82,   108,  -134,
    -134,  -134,     2,    91,  -134,   598,   169,    68,  -134,     4,
    -134,   227,   227,   227,   227,   227,   198,  -134,  -134,   339,
    -134,  -134,  -134,     2,   493,   493,   493,   142,   148,  -134,
    -134,  -134,   493,   -45,   449,  -134,  -134,   493,   112,   112,
    -134,   182,  -134,  -134,  -134,  -134,   154,    10,   110,  -134,
     182,  -134,  -134,  -134,   548,  -134,  -134,  -134,  -134,  -134,
    -134,  -134,  -134,  -134,   493,  -134,    85,  -134,  -134,   266,
     266,  -134,  -134,  -134,  -134,   167,   158,   478,   777,   191,
     493,   493,   324,   493,   493,   493,   493,   493,   493,   493,
     139,   493,   493,   493,   493,   493,   493,   493,   493,   493,
     493,   493,   548,   376,   -24,     1,   464,  -134,   667,   223,
       9,  -134,   493,  -134,   548,   667,   225,    15,  -134,   712,
    -134,  -134,   548,   398,   690,  -134,   733,   753,   777,   777,
     777,   777,   777,   493,   765,  -134,   787,   787,   797,   297,
     297,   297,   191,   191,   191,   191,   205,   173,  -134,  -134,
    -134,  -134,   182,    36,  -134,   644,  -134,   182,    36,  -134,
     144,  -134,   493,   777,   493,   509,   712,  -134,  -134,  -134,
    -134,  -134,   179,   548,  -134,   180,   311,   427,   777,   548,
     228,  -134,   228,  -134,   548,  -134,  -134,   228,   161,  -134
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -134,   185,  -134,   216,   117,   190,  -134,  -134,  -134,   195,
      71,  -134,  -134,  -134,  -134,   192,  -134,   241,   -15,  -134,
     221,    92,   -53,  -134,  -134,   -81,   175,    14,    42,    -1,
     129,  -134,   242,   -51,  -133,    49,  -134,  -134,  -134,  -134,
    -134,  -134,   -56,    21,  -134,  -134
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -132
static const yytype_int16 yytable[] =
{
       8,    35,     8,   134,    88,    11,    95,   206,    83,     8,
      22,    27,   243,    11,    34,    34,   -84,   112,   248,   -84,
     -84,    67,   101,   102,    87,   103,   104,   105,     8,    11,
      11,    11,    51,    28,    73,    95,    52,   145,    27,    12,
      62,    35,    29,     7,   239,     9,   258,   259,    12,    40,
      49,    34,    20,   260,    39,    34,    93,    55,    62,    62,
      28,    56,    63,    64,    66,     8,    83,   145,    62,   240,
      62,    48,   158,     8,    57,   134,   203,   244,    97,   -84,
      63,    63,    87,   249,   135,    93,    41,    49,    49,    73,
      63,   208,    63,    95,   -90,    42,    69,   -90,   -90,   261,
      62,    62,    62,    62,    62,     8,    73,    97,    48,   257,
     130,   143,    61,   204,   264,   144,   113,   144,    71,   131,
     -90,    83,    63,    63,    63,    63,    63,     8,     8,   156,
      98,    99,     2,    41,    23,    24,    38,    87,    83,   237,
     139,    95,    42,    93,    83,    23,    24,   266,     7,   140,
      53,   144,    54,    95,    87,   223,   115,   -90,   116,   250,
      87,    95,     3,     4,   279,    97,     5,     6,   144,   114,
     194,   195,   159,   160,   161,   162,   163,     5,     6,   255,
     144,    23,    24,   117,   118,   127,     5,     6,    43,   128,
     155,    93,   129,     5,     6,   119,   120,    23,    24,   115,
     121,   196,   270,    93,    95,   124,   122,     5,     6,   170,
     272,    93,    95,    97,    21,   171,   277,   202,    95,   210,
     191,   278,    41,    95,   211,    97,   117,   118,     3,     4,
     242,    42,   247,    97,  -132,   144,     5,     6,   119,   120,
      23,    24,   197,   121,   168,   169,    59,   271,   273,   122,
     106,   172,     5,     6,    93,    58,   193,   108,     5,     6,
     198,   107,    93,   111,   164,    30,     5,     6,    93,   205,
      23,    24,   157,    93,     5,     6,    97,    72,    23,    24,
     275,     5,     6,   209,    97,    23,    24,   103,   104,   105,
      97,     5,     6,    68,    60,    97,   246,   265,   121,   213,
     214,     0,   216,   217,   218,   219,   220,   221,   222,   224,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   187,   188,   189,   190,   169,   191,   136,   115,     0,
     116,   245,   173,   174,   175,   176,   177,   178,   179,   180,
       0,     0,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   253,   191,     0,   117,   118,   101,   102,   165,
     103,   104,   105,     0,     0,     5,     6,   119,   120,    23,
      24,   256,   121,   274,     0,     0,   256,     0,   122,     0,
       0,   267,     0,   268,   173,   174,   175,   176,   177,   178,
     179,   180,   215,     0,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,     0,   191,   173,   174,   175,   176,
     177,   178,   179,   180,     0,     0,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,     0,   191,     0,     0,
       0,     0,     0,     0,     0,   173,   174,   175,   176,   177,
     178,   179,   180,     0,   238,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,     0,   191,   173,   174,   175,
     176,   177,   178,   179,   180,     0,   251,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,     0,   191,     0,
       0,   115,     0,   116,     0,     0,   173,   174,   175,   176,
     177,   178,   179,   180,     0,   276,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,     0,   191,   117,   118,
     115,   192,   116,     0,     0,     0,     0,     0,     5,     6,
     119,   120,    23,    24,   241,   121,     0,     0,     0,     0,
      73,   122,     0,    74,     0,     0,     0,   117,   118,     0,
     212,     0,    75,    76,     0,    77,     0,     5,     6,   119,
     120,    23,    24,     0,   121,    78,    79,    80,    81,    82,
     122,     0,    83,     5,     6,     0,     0,    23,    24,    73,
      84,   269,    74,    85,    86,     0,     0,     0,    87,   100,
       0,    75,    76,     0,    77,     0,   101,   102,     0,   103,
     104,   105,     0,     0,    78,    79,    80,    81,    82,     0,
       0,    83,     5,     6,     0,    14,    23,    24,    15,    84,
       0,     0,    85,    86,     0,     0,     0,    87,     0,     0,
       0,     0,     0,     0,    16,     0,     0,     5,     6,     0,
       0,     0,     0,     0,    17,   146,   147,   148,   149,   150,
     151,   152,     0,     0,     0,     0,     0,     0,   263,     0,
       0,     0,   173,   174,   175,   176,   177,   178,   179,   180,
       0,   153,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,     0,   191,  -131,   173,   174,   175,   176,   177,
     178,   179,   180,     0,     0,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,     0,   191,   252,   173,   174,
     175,   176,   177,   178,   179,   180,     0,     0,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,     0,   191,
     173,   174,   175,   176,   177,   178,   179,   180,     0,     0,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
       0,   191,   174,   175,   176,   177,   178,   179,   180,     0,
       0,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,     0,   191,   175,   176,   177,   178,   179,   180,     0,
       0,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   254,   191,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,     0,   191,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,     0,   191,   183,   184,   185,
     186,   187,   188,   189,   190,     0,   191,  -132,   184,   185,
     186,   187,   188,   189,   190,     0,   191
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-134))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-132))

static const yytype_int16 yycheck[] =
{
       1,    16,     3,    84,    57,    29,    57,   140,    53,    10,
      11,    12,     3,    29,    15,    16,     3,    73,     3,     6,
       7,    21,    18,    19,    69,    21,    22,    23,    29,    29,
      29,    29,     3,    12,    21,    86,     7,    93,    39,    64,
      41,    56,    63,     1,    68,     3,    10,    11,    64,     4,
      29,    52,    10,    17,     7,    56,    57,     3,    59,    60,
      39,     7,    41,    42,     7,    66,    53,   123,    69,    68,
      71,    29,    68,    74,    63,   156,    66,    68,    57,    66,
      59,    60,    69,    68,    85,    86,    10,    66,    67,    21,
      69,   144,    71,   144,     3,    19,    63,     6,     7,    63,
     101,   102,   103,   104,   105,   106,    21,    86,    66,   242,
      58,     3,    41,     3,   247,     7,    74,     7,    63,    67,
      29,    53,   101,   102,   103,   104,   105,   128,   129,    61,
      59,    60,     0,    10,    58,    59,    19,    69,    53,   192,
      58,   192,    19,   144,    53,    58,    59,     3,   106,    67,
      33,     7,    35,   204,    69,    16,    17,    66,    19,   212,
      69,   212,    30,    31,     3,   144,    54,    55,     7,    65,
     128,   129,   101,   102,   103,   104,   105,    54,    55,     6,
       7,    58,    59,    44,    45,    67,    54,    55,    65,    67,
      21,   192,    67,    54,    55,    56,    57,    58,    59,    17,
      61,    19,   255,   204,   255,    76,    67,    54,    55,    67,
     263,   212,   263,   192,    61,    67,   269,    63,   269,    52,
      29,   274,    10,   274,    66,   204,    44,    45,    30,    31,
       7,    19,     7,   212,    29,     7,    54,    55,    56,    57,
      58,    59,    60,    61,   115,   116,    19,    68,    68,    67,
      65,   122,    54,    55,   255,    39,   127,    67,    54,    55,
     131,    66,   263,    71,    66,    61,    54,    55,   269,   140,
      58,    59,    97,   274,    54,    55,   255,    56,    58,    59,
     266,    54,    55,   154,   263,    58,    59,    21,    22,    23,
     269,    54,    55,    52,    67,   274,   204,   248,    61,   170,
     171,    -1,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    24,    25,    26,    27,   196,    29,    85,    17,    -1,
      19,   202,     8,     9,    10,    11,    12,    13,    14,    15,
      -1,    -1,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,   223,    29,    -1,    44,    45,    18,    19,    20,
      21,    22,    23,    -1,    -1,    54,    55,    56,    57,    58,
      59,   242,    61,    62,    -1,    -1,   247,    -1,    67,    -1,
      -1,   252,    -1,   254,     8,     9,    10,    11,    12,    13,
      14,    15,    68,    -1,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    -1,    29,     8,     9,    10,    11,
      12,    13,    14,    15,    -1,    -1,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    -1,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     8,     9,    10,    11,    12,
      13,    14,    15,    -1,    68,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    -1,    29,     8,     9,    10,
      11,    12,    13,    14,    15,    -1,    68,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    -1,    29,    -1,
      -1,    17,    -1,    19,    -1,    -1,     8,     9,    10,    11,
      12,    13,    14,    15,    -1,    68,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    -1,    29,    44,    45,
      17,    62,    19,    -1,    -1,    -1,    -1,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    -1,    -1,    -1,    -1,
      21,    67,    -1,    24,    -1,    -1,    -1,    44,    45,    -1,
      62,    -1,    33,    34,    -1,    36,    -1,    54,    55,    56,
      57,    58,    59,    -1,    61,    46,    47,    48,    49,    50,
      67,    -1,    53,    54,    55,    -1,    -1,    58,    59,    21,
      61,    62,    24,    64,    65,    -1,    -1,    -1,    69,    11,
      -1,    33,    34,    -1,    36,    -1,    18,    19,    -1,    21,
      22,    23,    -1,    -1,    46,    47,    48,    49,    50,    -1,
      -1,    53,    54,    55,    -1,    32,    58,    59,    35,    61,
      -1,    -1,    64,    65,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    54,    55,    -1,
      -1,    -1,    -1,    -1,    61,    37,    38,    39,    40,    41,
      42,    43,    -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,
      -1,    -1,     8,     9,    10,    11,    12,    13,    14,    15,
      -1,    63,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    -1,    29,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    -1,    -1,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    -1,    29,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    -1,    -1,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    -1,    29,
       8,     9,    10,    11,    12,    13,    14,    15,    -1,    -1,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      -1,    29,     9,    10,    11,    12,    13,    14,    15,    -1,
      -1,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    -1,    29,    10,    11,    12,    13,    14,    15,    -1,
      -1,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    16,    29,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    -1,    29,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    -1,    29,    20,    21,    22,
      23,    24,    25,    26,    27,    -1,    29,    20,    21,    22,
      23,    24,    25,    26,    27,    -1,    29
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    71,     0,    30,    31,    54,    55,    98,    99,    98,
      81,    29,    64,    74,    32,    35,    51,    61,    82,    83,
      98,    61,    99,    58,    59,    72,    73,    99,   113,    63,
      61,    84,    87,    88,    99,    88,    89,    90,    74,     7,
       4,    10,    19,    65,    75,    76,    78,    79,    98,   113,
     115,     3,     7,    74,    74,     3,     7,    63,    73,    19,
      67,    80,    99,   113,   113,    77,     7,    21,    87,    63,
      86,    63,    90,    21,    24,    33,    34,    36,    46,    47,
      48,    49,    50,    53,    61,    64,    65,    69,    92,    93,
      95,    96,    98,    99,   102,   103,   112,   113,    80,    80,
      11,    18,    19,    21,    22,    23,    71,    79,    75,    80,
      85,    85,   112,    98,    65,    17,    19,    44,    45,    56,
      57,    61,    67,    99,   100,   103,   113,    67,    67,    67,
      58,    67,   106,   108,    95,    99,   102,    91,    92,    58,
      67,   109,   111,     3,     7,   112,    37,    38,    39,    40,
      41,    42,    43,    63,    94,    21,    61,    96,    68,    80,
      80,    80,    80,    80,    66,    20,    97,   100,   100,   100,
      67,    67,   100,     8,     9,    10,    11,    12,    13,    14,
      15,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    29,    62,   100,    98,    98,    19,    60,   100,   104,
     107,   114,    63,    66,     3,   100,   104,   110,    92,   100,
      52,    66,    62,   100,   100,    68,   100,   100,   100,   100,
     100,   100,   100,    16,   100,   101,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,    92,    68,    68,
      68,    60,     7,     3,    68,   100,    91,     7,     3,    68,
      92,    68,     7,   100,    16,     6,   100,   104,    10,    11,
      17,    63,   105,     4,   104,   105,     3,   100,   100,    62,
      92,    68,    92,    68,    62,    97,    68,    92,    92,     3
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


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
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
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


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
  for (; yybottom <= yytop; yybottom++)
    {
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

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
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
   if the built-in stack extension method is used).

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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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

  while ((*yyd++ = *yys++) != '\0')
    continue;

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
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

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


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

    /* Number of syntax errors so far.  */
    int yynerrs;

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

  if (yyss + yystacksize - 1 <= yyssp)
    {
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
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

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
  if (yyn == 0)
    goto yyerrlab;
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
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1806 of yacc.c  */
#line 232 "parser.yy"
    { /* Nix zu tun. */ }
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 233 "parser.yy"
    {
	
      PBEGIN_TRY
        gAktDefKnoten->fuegeEin(*(yyvsp[(2) - (5)].str), *(yyvsp[(3) - (5)].version), (yyvsp[(5) - (5)].knoten));
        delete (yyvsp[(2) - (5)].str);
        delete (yyvsp[(3) - (5)].version);
      PEND_TRY(;);
    }
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 241 "parser.yy"
    {
			   
      /* Nix zu tun; die Codes speichern sich von alleine nach
         gAktDefKnoten */
    }
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 252 "parser.yy"
    {
      PBEGIN_TRY
      
        /* Lex auf neue Datei umschalten */
	void * merkBuf = pushLex((yyvsp[(3) - (3)].str)->data());


        /***** Bison-Aufruf für include-Datei *****/
        /* Hier muss man aufpassen, dass mit allen globalen Variablen
	   das richtige passiert. Nichts zu tun ist bei:
	   - gGabFehler (Fehler in include-Datei ist halt auch Fehler)
	   - gAktDefKnoten (Die Include-Datei speichert ihre Ergebnisse
	     auch einfach da mit rein.)
	   */

	
	/* Datei und Zeilennummer zwischenspeichern. */
	Str merkDat = gDateiName;
	gDateiName = *(yyvsp[(3) - (3)].str);
	int merkZNr = gZeilenNr;
	gZeilenNr = 1;
	
	/* Der rekursive Aufruf! Hier! Live! (Die Ergebnisse werden in
	   gAktDefKnoten eingefügt.) */
	if ((yyparse()) && !gGabFehler) {
	  print_to_stderr(_("Unknown error during file inclusion!\n"));
	  gGabFehler = true;
	}
	
	gDateiName = merkDat;
	gZeilenNr = merkZNr;
		
	/* Lex auf alte Datei zurückschalten */
	popLex(merkBuf);
	
      PEND_TRY(;);
    }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 295 "parser.yy"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 296 "parser.yy"
    { (yyval.str) = new Str(_sprintf("%d",(yyvsp[(1) - (1)].zahl))); }
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 302 "parser.yy"
    {
      (yyval.version) = new Version();
      (yyval.version)->nochEinMerkmal(*(yyvsp[(1) - (1)].str));
      delete (yyvsp[(1) - (1)].str);
    }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 307 "parser.yy"
    {
      (yyval.version) = (yyvsp[(3) - (3)].version);
      (yyval.version)->nochEinMerkmal(*(yyvsp[(1) - (3)].str));
      delete (yyvsp[(1) - (3)].str);
    }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 315 "parser.yy"
    { (yyval.version) = new Version(); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 316 "parser.yy"
    { (yyval.version) = (yyvsp[(2) - (3)].version); }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 320 "parser.yy"
    { (yyval.zahl) = (yyvsp[(1) - (1)].zahl); }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 321 "parser.yy"
    { (yyval.zahl) = (yyvsp[(2) - (3)].zahl); }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 324 "parser.yy"
    {
      /* OK, hier wird ein neuer Defknoten eröffnet. Der alte wird
         auf dem Bison-Stack zwischengespeichert... */
      DefKnoten * merk = gAktDefKnoten;
      /* Neuen Defknoten erzeugen, mit dem alten als Vater */
      gAktDefKnoten = new DefKnoten(gDateiName, gZeilenNr, merk);
      (yyval.defknoten) = merk;

                    }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 332 "parser.yy"
    {
		    
      /* Jetzt wurde gAktDefKnoten mit Inhalt gefüllt, den wir
         zurückliefern */
      (yyval.knoten) = gAktDefKnoten;
      /* POP DefKnoten */
      gAktDefKnoten = (yyvsp[(2) - (4)].defknoten);
    }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 340 "parser.yy"
    { (yyval.knoten) = (yyvsp[(1) - (1)].listenknoten); }
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 343 "parser.yy"
    {
      (yyval.listenknoten) = new ListenKnoten(gDateiName, gZeilenNr);
      (yyval.listenknoten)->fuegeEin((yyvsp[(1) - (1)].knoten));
    }
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 347 "parser.yy"
    {
      (yyval.listenknoten) = (yyvsp[(1) - (3)].listenknoten);
      (yyval.listenknoten)->fuegeEin((yyvsp[(3) - (3)].knoten));
    }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 353 "parser.yy"
    {
      (yyval.knoten) = new WortKnoten(gDateiName, gZeilenNr, *(yyvsp[(1) - (1)].str)); delete (yyvsp[(1) - (1)].str);
    }
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 356 "parser.yy"
    {
      (yyval.knoten) = new ZahlKnoten(gDateiName, gZeilenNr, (yyvsp[(1) - (1)].zahl));
    }
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 359 "parser.yy"
    {
      (yyval.knoten) = new VielfachheitKnoten(gDateiName, gZeilenNr, *(yyvsp[(1) - (3)].str), (yyvsp[(3) - (3)].zahl)); delete (yyvsp[(1) - (3)].str);
    }
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 365 "parser.yy"
    { (yyval.zahl) = (yyvsp[(1) - (1)].zahl); }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 366 "parser.yy"
    {
      Knoten * def = gAktDefKnoten->getVerwandten(*(yyvsp[(1) - (1)].str), ld->mVersion, false);
      const DatenKnoten * datum = 0;
      switch (def->type()) {
        case type_DatenKnoten:
          datum=(const DatenKnoten *) def;
          break;
        case type_ListenKnoten:
          datum=((ListenKnoten*) def)->getEinzigesDatum();
          break;
        default: throw Fehler(_("%s not a number"),(yyvsp[(1) - (1)].str)->data());
      }
      (yyval.zahl) = datum->assert_datatype(type_ZahlDatum)->getZahl();
      delete (yyvsp[(1) - (1)].str);
    }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 381 "parser.yy"
    { (yyval.zahl) = (yyvsp[(2) - (3)].zahl); }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 382 "parser.yy"
    { (yyval.zahl) = -(yyvsp[(2) - (2)].zahl); }
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 383 "parser.yy"
    { (yyval.zahl) = (yyvsp[(1) - (3)].zahl) + (yyvsp[(3) - (3)].zahl); }
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 384 "parser.yy"
    { (yyval.zahl) = (yyvsp[(1) - (3)].zahl) - (yyvsp[(3) - (3)].zahl); }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 385 "parser.yy"
    { (yyval.zahl) = (yyvsp[(1) - (3)].zahl) * (yyvsp[(3) - (3)].zahl); }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 386 "parser.yy"
    { (yyval.zahl) = divv((yyvsp[(1) - (3)].zahl),(yyvsp[(3) - (3)].zahl)); }
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 387 "parser.yy"
    { (yyval.zahl) = modd((yyvsp[(1) - (3)].zahl),(yyvsp[(3) - (3)].zahl)); }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 400 "parser.yy"
    {
      gAktDefKnoten->speicherDefinition(namespace_prozedur, *(yyvsp[(1) - (5)].str),
                                        *(yyvsp[(2) - (5)].version), (yyvsp[(4) - (5)].code));
      delete (yyvsp[(1) - (5)].str); delete (yyvsp[(2) - (5)].version);
    }
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 416 "parser.yy"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 417 "parser.yy"
    {
      /* Wie gibt man einen Fehler möglichst umständlich aus?
         (Aber so, dass er genauso aussieht wie die anderen Fehler. */
      PBEGIN_TRY
        throw Fehler("%s",_("Procedure names can't be single letters."));
      PEND_TRY((yyval.str) = new Str());
    }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 432 "parser.yy"
    { (yyval.zahlpaar)[0]=(yyvsp[(1) - (1)].zahl); (yyval.zahlpaar)[1]=da_init; }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 433 "parser.yy"
    { (yyval.zahlpaar)[0]=(yyvsp[(1) - (3)].zahl); (yyval.zahlpaar)[1]=da_kind; }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 437 "parser.yy"
    { (yyval.zahlpaar)[0]=0;  (yyval.zahlpaar)[1]=da_init; }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 438 "parser.yy"
    { (yyval.zahlpaar)[0]=(yyvsp[(2) - (2)].zahlpaar)[0];  (yyval.zahlpaar)[1]=(yyvsp[(2) - (2)].zahlpaar)[1];}
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 442 "parser.yy"
    {
      PBEGIN_TRY
        gAktDefKnoten->neueVarDefinition(*(yyvsp[(1) - (3)].str), *(yyvsp[(2) - (3)].version), (yyvsp[(3) - (3)].zahlpaar)[0], (yyvsp[(3) - (3)].zahlpaar)[1]);
        delete (yyvsp[(1) - (3)].str); delete (yyvsp[(2) - (3)].version);
      PEND_TRY(;)
    }
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 452 "parser.yy"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 453 "parser.yy"
    {
      /* Wie gibt man einen Fehler möglichst umständlich aus?
         (Aber so, dass er genauso aussieht wie die anderen Fehler. */
      PBEGIN_TRY
        throw Fehler("%s",_("Variable names can't be single letters."));
      PEND_TRY((yyval.str) = new Str());
    }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 469 "parser.yy"
    {
      PBEGIN_TRY
        gAktDefKnoten->neuerDefault(
          ((VarDefinition*)
              (gAktDefKnoten->getDefinition(namespace_variable,*(yyvsp[(1) - (4)].str),*(yyvsp[(2) - (4)].version),false)))
            -> mNummer,
          (yyvsp[(4) - (4)].zahlpaar)[0], (yyvsp[(4) - (4)].zahlpaar)[1]);
        delete (yyvsp[(1) - (4)].str); delete (yyvsp[(2) - (4)].version);
      PEND_TRY(;)
    }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 488 "parser.yy"
    { (yyval.code) = (yyvsp[(1) - (1)].code); }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 489 "parser.yy"
    { (yyval.code) = newCode2(stapel_code, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 493 "parser.yy"
    { (yyval.code) = (yyvsp[(3) - (4)].code); }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 494 "parser.yy"
    {
      (yyval.code) = newCode4(bedingung_code, (yyvsp[(2) - (4)].code), (yyvsp[(4) - (4)].code),
                     newCode0(nop_code),
                     (yyvsp[(3) - (4)].zahl) + 2 * ohne_merk_pfeil);
    }
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 499 "parser.yy"
    {
      if ((yyvsp[(3) - (6)].zahl)==ohne_merk_pfeil)
        (yyval.code) = newCode4(bedingung_code, (yyvsp[(2) - (6)].code), (yyvsp[(4) - (6)].code),
                      (yyvsp[(6) - (6)].code),
                      3*ohne_merk_pfeil);
      else
	/* TRANSLATORS: The text in the literal strings should not be translated. */
        throw Fehler("%s",_("Please specify \"else ->\" or \"else =>\""));
    }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 508 "parser.yy"
    {
      /* Nach else kann, muss aber kein Pfeil stehen.
         (Kein Pfeil will man vermutlich, wenn dann gleich das
	 nächste if kommt.) */
      (yyval.code) = newCode4(bedingung_code, (yyvsp[(2) - (7)].code), (yyvsp[(4) - (7)].code),
                     (yyvsp[(7) - (7)].code),
                     (yyvsp[(3) - (7)].zahl) + 2 * (yyvsp[(6) - (7)].zahl));
    }
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 516 "parser.yy"
    { (yyval.code) = (yyvsp[(2) - (3)].code); }
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 517 "parser.yy"
    { (yyval.code) = newCode2(folge_code, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 518 "parser.yy"
    { (yyval.code) = newCode1(zahl_code, (yyvsp[(1) - (1)].zahl)); }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 519 "parser.yy"
    { (yyval.code) = (yyvsp[(1) - (1)].code); }
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 520 "parser.yy"
    {
      (yyval.code) = newCode2(stapel_code, newCode1(zahl_code, (yyvsp[(1) - (2)].zahl)), (yyvsp[(2) - (2)].code));
    }
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 523 "parser.yy"
    {
      PBEGIN_TRY
        /* Kopie erzeugen...) */
        (yyval.code) = new Code(gAktDefKnoten, * (Code*)
               gAktDefKnoten->getDefinition(namespace_prozedur, *(yyvsp[(1) - (1)].str),
                                            ld->mVersion, false), true);
        delete (yyvsp[(1) - (1)].str);
      PEND_TRY((yyval.code) = newCode0(undefiniert_code))
    }
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 532 "parser.yy"
    {
      PBEGIN_TRY
        /* Kopie erzeugen...) */
        (yyval.code) = newCode1(weiterleit_code,
	      new Code(gAktDefKnoten, * (Code*)
                gAktDefKnoten->getDefinition(namespace_prozedur, *(yyvsp[(2) - (2)].str),
                                             ld->mVersion, false), false));
        delete (yyvsp[(2) - (2)].str);
      PEND_TRY((yyval.code) = newCode0(undefiniert_code))
    }
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 542 "parser.yy"
    { (yyval.code) = newCode0(nop_code); }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 543 "parser.yy"
    { (yyval.code) = (yyvsp[(1) - (1)].code); }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 544 "parser.yy"
    {
      PBEGIN_TRY
        if ((yyvsp[(2) - (6)].variable)->istKonstante())
          throw Fehler(_sprintf(_("%s is a constant. (Variable expected.)"),
                     (yyvsp[(2) - (6)].variable)->getName().data()));
        (yyval.code) = newCode3(push_code, (yyvsp[(4) - (6)].code), (yyvsp[(6) - (6)].code), (yyvsp[(2) - (6)].variable));
        
      PEND_TRY((yyval.code) = newCode0(undefiniert_code))
    }
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 553 "parser.yy"
    { (yyval.code) = newCode0(busy_code); }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 554 "parser.yy"
    {
      (yyval.code) = newCode1(bonus_code, (yyvsp[(3) - (4)].code));
    }
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 557 "parser.yy"
    {
      (yyval.code) = newCode1(message_code, _((yyvsp[(3) - (4)].str)->data()));
      delete (yyvsp[(3) - (4)].str);
    }
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 561 "parser.yy"
    {
      (yyval.code) = newCode1(sound_code, Sound::ladSample(*(yyvsp[(3) - (4)].str)));
      delete (yyvsp[(3) - (4)].str);
    }
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 565 "parser.yy"
    {
      (yyval.code) = newCode0(verlier_code);
    }
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 568 "parser.yy"
    {
      (yyval.code) = newCode0(explode_code);
    }
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 574 "parser.yy"
    {
      PBEGIN_TRY
        if ((yyvsp[(1) - (3)].variable)->istKonstante())
          throw Fehler(_sprintf(_("%s is a constant. (Variable expected.)"),
                     (yyvsp[(1) - (3)].variable)->getName().data()));
        (yyval.code) = newCode2((yyvsp[(2) - (3)].codeart), (yyvsp[(3) - (3)].code), (yyvsp[(1) - (3)].variable));
      PEND_TRY((yyval.code) = newCode0(undefiniert_code))
    }
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 584 "parser.yy"
    { (yyval.codeart) = set_code; }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 585 "parser.yy"
    { (yyval.codeart) = add_code; }
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 586 "parser.yy"
    { (yyval.codeart) = sub_code; }
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 587 "parser.yy"
    { (yyval.codeart) = mul_code; }
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 588 "parser.yy"
    { (yyval.codeart) = div_code; }
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 589 "parser.yy"
    { (yyval.codeart) = mod_code; }
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 590 "parser.yy"
    { (yyval.codeart) = bitset_code; }
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 591 "parser.yy"
    { (yyval.codeart) = bitunset_code; }
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 596 "parser.yy"
    { (yyval.code) = newCode0(mal_code); }
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 597 "parser.yy"
    { (yyval.code) = newCode2(mal_code_fremd, (yyvsp[(2) - (2)].ort), 1); }
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 598 "parser.yy"
    { (yyval.code) = newCode2(mal_code_fremd, (yyvsp[(1) - (2)].ort), -1); }
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 602 "parser.yy"
    { (yyval.code) = newCode1(buchstabe_code, (yyvsp[(1) - (1)].zahl)); }
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 603 "parser.yy"
    {
      (yyval.code) = newCode2(stapel_code, newCode1(buchstabe_code, (yyvsp[(1) - (2)].zahl)),
                     (yyvsp[(2) - (2)].code));
    }
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 607 "parser.yy"
    { (yyval.code) = (yyvsp[(1) - (1)].code); }
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 612 "parser.yy"
    {
      (yyval.code) = newCode4(bedingung_code, (yyvsp[(1) - (4)].code), (yyvsp[(3) - (4)].code),
                     newCode0(nop_code),
                     (yyvsp[(2) - (4)].zahl) + 2 * ohne_merk_pfeil);
    }
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 617 "parser.yy"
    {
      (yyval.code) = newCode4(bedingung_code, (yyvsp[(1) - (7)].code), (yyvsp[(3) - (7)].code),
                     (yyvsp[(6) - (7)].code),
                     (yyvsp[(2) - (7)].zahl) + 2 * (yyvsp[(5) - (7)].zahl));
    }
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 622 "parser.yy"
    {
      (yyval.code) = newCode4(bedingung_code, (yyvsp[(1) - (5)].code), (yyvsp[(3) - (5)].code),
                     (yyvsp[(5) - (5)].code),
                     (yyvsp[(2) - (5)].zahl) + 2 * mit_merk_pfeil);
    }
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 633 "parser.yy"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 634 "parser.yy"
    {
      *(yyvsp[(1) - (3)].str) += '.';  *(yyvsp[(1) - (3)].str) += *(yyvsp[(3) - (3)].str);  (yyval.str) = (yyvsp[(1) - (3)].str);
      delete (yyvsp[(3) - (3)].str);
    }
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 638 "parser.yy"
    {
      *(yyvsp[(1) - (3)].str) += '.';  *(yyvsp[(1) - (3)].str) += ((yyvsp[(3) - (3)].zahl)>=26 ? 'a'+(yyvsp[(3) - (3)].zahl)-26 : 'A'+(yyvsp[(3) - (3)].zahl));  (yyval.str) = (yyvsp[(1) - (3)].str);
    }
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 644 "parser.yy"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 645 "parser.yy"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 652 "parser.yy"
    {
      if ((yyvsp[(1) - (1)].variable)->istKonstante()) {
        /* Wenn die Variable in Wirklichkeit eine Konstante ist,
           dann gleich die Konstante einsetzen. */
        (yyval.code) = newCode1(zahl_acode, (yyvsp[(1) - (1)].variable)->getDefaultWert());
        delete (yyvsp[(1) - (1)].variable);
      } else
        (yyval.code) = newCode1(variable_acode, (yyvsp[(1) - (1)].variable));
    }
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 661 "parser.yy"
    { (yyval.code) = newCode1(zahl_acode, (yyvsp[(1) - (1)].zahl)); }
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 662 "parser.yy"
    { (yyval.code) = (yyvsp[(2) - (3)].code); }
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 663 "parser.yy"
    {
      (yyval.code) = newNachbarCode(gAktDefKnoten, gDateiName, gZeilenNr, (yyvsp[(1) - (1)].str));
    }
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 666 "parser.yy"
    {
      (yyval.code) = newNachbarCode(gAktDefKnoten, gDateiName, gZeilenNr, (yyvsp[(1) - (1)].str));
    }
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 673 "parser.yy"
    { (yyval.code) = newCode2(manchmal_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 674 "parser.yy"
    { (yyval.code) = newCode2(add_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 675 "parser.yy"
    { (yyval.code) = newCode1(neg_acode, (yyvsp[(2) - (2)].code));}
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 676 "parser.yy"
    { (yyval.code) = newCode2(sub_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 677 "parser.yy"
    { (yyval.code) = newCode2(mul_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 678 "parser.yy"
    { (yyval.code) = newCode2(div_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 679 "parser.yy"
    { (yyval.code) = newCode2(mod_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 680 "parser.yy"
    { (yyval.code) = newCode2(bitset_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 681 "parser.yy"
    { (yyval.code) = newCode2(bitunset_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 682 "parser.yy"
    { (yyval.code) = newCode2(bittest_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 683 "parser.yy"
    { (yyval.code) = newCode2(eq_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 684 "parser.yy"
    { (yyval.code) = newCode2(ne_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 685 "parser.yy"
    { (yyval.code) = newCode2(ge_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 686 "parser.yy"
    { (yyval.code) = newCode2(le_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 687 "parser.yy"
    { (yyval.code) = newCode2(gt_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 688 "parser.yy"
    { (yyval.code) = newCode2(lt_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 689 "parser.yy"
    { (yyval.code) = newCode2(bitand_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 690 "parser.yy"
    { (yyval.code) = newCode2(bitor_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 691 "parser.yy"
    { (yyval.code) = newCode1(not_acode, (yyvsp[(2) - (2)].code));}
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 692 "parser.yy"
    { (yyval.code) = newCode2(und_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 693 "parser.yy"
    { (yyval.code) = newCode2(oder_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code));}
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 694 "parser.yy"
    {
      (yyval.code) = newCode3(intervall_acode, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].codepaar)[0], (yyvsp[(3) - (3)].codepaar)[1]);
    }
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 697 "parser.yy"
    {
      (yyval.code) = newCode1(rnd_acode, (yyvsp[(3) - (4)].code));
    }
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 700 "parser.yy"
    {
      (yyval.code) = newCode2(ggt_acode, (yyvsp[(3) - (6)].code), (yyvsp[(5) - (6)].code));
    }
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 707 "parser.yy"
    { (yyval.codepaar)[0]=(yyvsp[(1) - (2)].code); (yyval.codepaar)[1]=newCode1(zahl_acode, VIEL); }
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 708 "parser.yy"
    { (yyval.codepaar)[0]=newCode1(zahl_acode, -VIEL); (yyval.codepaar)[1]=(yyvsp[(2) - (2)].code); }
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 709 "parser.yy"
    { (yyval.codepaar)[0]=(yyvsp[(1) - (3)].code); (yyval.codepaar)[1]=(yyvsp[(3) - (3)].code); }
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 713 "parser.yy"
    {
      PBEGIN_TRY
        (yyval.variable) = new Variable(//gDateiName, gZeilenNr,
               (VarDefinition*) gAktDefKnoten->
                     getDefinition(namespace_variable, *(yyvsp[(1) - (1)].str),
                                   ld->mVersion, false),
               0
             );
      PEND_TRY((yyval.variable) = new Variable())
      delete (yyvsp[(1) - (1)].str);
    }
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 724 "parser.yy"
    {
      /* Wie gibt man einen Fehler möglichst umständlich aus?
         (Aber so, dass er genauso aussieht wie die anderen Fehler. */
      PBEGIN_TRY
        throw Fehler("%s",_("Variable names can't be single letters."));
      PEND_TRY((yyval.variable) = new Variable());
    }
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 734 "parser.yy"
    { (yyval.variable) = (yyvsp[(1) - (1)].variable); }
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 735 "parser.yy"
    {
      PBEGIN_TRY
        (yyval.variable) = new Variable(//gDateiName, gZeilenNr,
               (VarDefinition*) gAktDefKnoten->
                     getDefinition(namespace_variable, *(yyvsp[(1) - (2)].str),
                                   ld->mVersion, false),
               (yyvsp[(2) - (2)].ort));
      PEND_TRY((yyval.variable) = new Variable())
      delete (yyvsp[(1) - (2)].str);
    }
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 748 "parser.yy"
    { (yyval.code) = (yyvsp[(1) - (1)].code); }
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 749 "parser.yy"
    { (yyval.code) = newCode1(zahl_acode, (yyvsp[(1) - (1)].zahl)); }
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 753 "parser.yy"
    { (yyval.haelfte) = haelfte_hier; }
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 754 "parser.yy"
    { (yyval.haelfte) = haelfte_drueben; }
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 755 "parser.yy"
    { (yyval.haelfte) = haelfte_links; }
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 756 "parser.yy"
    { (yyval.haelfte) = haelfte_rechts; }
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 760 "parser.yy"
    { (yyval.ort) = new Ort(absort_semiglobal); }
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 761 "parser.yy"
    { (yyval.ort) = new Ort(absort_fall, newCode1(zahl_acode, (yyvsp[(1) - (1)].zahl))); }
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 765 "parser.yy"
    { (yyval.ort) = new Ort(absort_semiglobal); }
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 766 "parser.yy"
    { (yyval.ort) = new Ort(absort_fall, (yyvsp[(1) - (1)].code)); }
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 767 "parser.yy"
    { (yyval.ort) = new Ort(absort_feld, (yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code)); }
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 771 "parser.yy"
    { (yyval.ort) = (yyvsp[(1) - (1)].ort); }
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 772 "parser.yy"
    { (yyval.ort) = (yyvsp[(2) - (3)].ort); }
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 773 "parser.yy"
    {
      (yyvsp[(2) - (5)].ort)->setzeHaelfte((yyvsp[(4) - (5)].haelfte));
      (yyval.ort) = (yyvsp[(2) - (5)].ort);
    }
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 780 "parser.yy"
    { (yyval.ort) = new Ort(absort_global); }
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 781 "parser.yy"
    { (yyval.ort) = new Ort(newCode1(zahl_acode, (yyvsp[(1) - (1)].zahl))); }
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 785 "parser.yy"
    { (yyval.ort) = new Ort(absort_global); }
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 786 "parser.yy"
    { (yyval.ort) = new Ort((yyvsp[(1) - (1)].code)); }
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 787 "parser.yy"
    { (yyval.ort) = new Ort((yyvsp[(1) - (3)].code), (yyvsp[(3) - (3)].code)); }
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 791 "parser.yy"
    { (yyval.ort) = (yyvsp[(1) - (1)].ort); }
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 792 "parser.yy"
    { (yyval.ort) = (yyvsp[(2) - (3)].ort); }
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 793 "parser.yy"
    {
      (yyvsp[(2) - (5)].ort)->setzeHaelfte((yyvsp[(4) - (5)].haelfte));
      (yyval.ort) = (yyvsp[(2) - (5)].ort);
    }
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 800 "parser.yy"
    { (yyval.ort) = (yyvsp[(2) - (2)].ort); }
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 801 "parser.yy"
    { (yyval.ort) = (yyvsp[(2) - (2)].ort); }
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 808 "parser.yy"
    { (yyval.zahl) = (yyvsp[(1) - (1)].zahl); }
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 809 "parser.yy"
    { (yyval.zahl) = (yyvsp[(1) - (1)].zahl); }
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 815 "parser.yy"
    {
       /* Halbzahlen sollen intern aufgerundet gespeichert werden... */
       (yyval.zahl) = (yyvsp[(1) - (1)].zahl) + 1;
     }
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 819 "parser.yy"
    { (yyval.zahl) = -(yyvsp[(2) - (2)].zahl); }
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 822 "parser.yy"
    { (yyval.zahl) = (yyvsp[(1) - (1)].zahl); }
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 823 "parser.yy"
    { (yyval.zahl) = -(yyvsp[(2) - (2)].zahl); }
    break;



/* Line 1806 of yacc.c  */
#line 3284 "parser.cc"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
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
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
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
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

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

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
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
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 827 "parser.yy"

/*****************************************************************************/






extern FILE * yyin;
int yyparse();
//void initLex();



int yyerror (const char * s)  /* Called by yyparse on error */
{
  PBEGIN_TRY
   throw Fehler(Str(s));
  PEND_TRY(;)
  return 0;
}


/* Öffnet die Datei mit dem angegebenen Namen und parst sie. Das
   Ergebnis wird in den Defknoten erg geschrieben. */
/** Komplettbeschreibung vom Parse-Vorgang siehe leveldaten.h */
void parse(const Str & name, DefKnoten * erg) {

  /* Datei öffnen, Lex initialisieren. Eigentlich bräuchte man
     kein pushLex und popLex ganz außen; aber es ist irgendwie
     sauberer. Vor allem ist dann sicher, dass ein Fehler in einem
     früheren Parsen keine Auswirkungen auf ein späteres Parsen
     hat.
     true = Default-Pfad merken für die Includes. (wird an den
            Pfaditerator weitergegeben.) */
  void * merkBuf = pushLex(name.data(), true);

  gDateiName = name;
  gZeilenNr = 1;
  gGabFehler = false;  /* Wenn es denn mal ein bison-Fehler-recovery gibt,
                         sollte gGabFehler dort auf true gesetzt werden */

  /* Das Parse-Ergebnis soll in den Knoten erg geschrieben werden. */
  gAktDefKnoten = erg;
  
  /* Hier findet das Parsen statt. Man beachte: Um Flex und Bison nicht
     zu verwirren, kann yyparse() nicht mit throw verlassen werden.
     Deshalb brauchen wir nix zu catchen, um alles wieder aufräumen zu
     können. */
  int perg = yyparse();
  
  /* Datei schließen, lex zurücksetzen. */
  popLex(merkBuf);
  
  
  /* Hier werden vermutlich mehr Bedingungen getestet als nötig. */
  if (perg || gGabFehler)
    throw Fehler("%s",_("There have been errors parsing the level description files."));
  
}


