/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSERC_TAB_H_INCLUDED
# define YY_YY_PARSERC_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    IF = 258,                      /* IF  */
    ELSE = 259,                    /* ELSE  */
    INT = 260,                     /* INT  */
    ID = 261,                      /* ID  */
    FLOAT = 262,                   /* FLOAT  */
    CHAR = 263,                    /* CHAR  */
    FOR = 264,                     /* FOR  */
    NUM = 265,                     /* NUM  */
    EQUAL = 266,                   /* EQUAL  */
    ASPASSIMPLES = 267,            /* ASPASSIMPLES  */
    OPEN_PAREN = 268,              /* OPEN_PAREN  */
    CLOSE_PAREN = 269,             /* CLOSE_PAREN  */
    PONTO_VIRGULA = 270,           /* PONTO_VIRGULA  */
    PLUS = 271,                    /* PLUS  */
    MINUS = 272,                   /* MINUS  */
    MULT = 273,                    /* MULT  */
    DIV = 274,                     /* DIV  */
    INCREMENT = 275,               /* INCREMENT  */
    DECREMENT = 276,               /* DECREMENT  */
    ADD_EQUAL = 277,               /* ADD_EQUAL  */
    SUB_EQUAL = 278,               /* SUB_EQUAL  */
    MULT_EQUAL = 279,              /* MULT_EQUAL  */
    DIV_EQUAL = 280,               /* DIV_EQUAL  */
    MOD_EQUAL = 281,               /* MOD_EQUAL  */
    AND = 282,                     /* AND  */
    OR = 283,                      /* OR  */
    NOT = 284,                     /* NOT  */
    MOD = 285,                     /* MOD  */
    WHILE = 286,                   /* WHILE  */
    DO = 287,                      /* DO  */
    VOID = 288,                    /* VOID  */
    MAIN = 289,                    /* MAIN  */
    PRINTF = 290,                  /* PRINTF  */
    SCANF = 291,                   /* SCANF  */
    INCLUDE = 292,                 /* INCLUDE  */
    STDIO_H = 293,                 /* STDIO_H  */
    STDLIB_H = 294,                /* STDLIB_H  */
    MATH_H = 295,                  /* MATH_H  */
    SWITCH = 296,                  /* SWITCH  */
    CASE = 297,                    /* CASE  */
    DEFAULT = 298,                 /* DEFAULT  */
    RETURN = 299,                  /* RETURN  */
    BREAK = 300,                   /* BREAK  */
    CONTINUE = 301,                /* CONTINUE  */
    BOOL = 302                     /* BOOL  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_PARSERC_TAB_H_INCLUDED  */
