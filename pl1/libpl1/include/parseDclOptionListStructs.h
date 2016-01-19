    /* parseDclOptionListStructs.h
     * ===========================
     *
     * Definition of datastructures for the declare option list.
     *
     * Copyright 2006, 2011 Henrik Sorensen
     *
     * This file is part of GCC front-end for the PL/I programming language, GCC PL/I.
     *
     * GCC PL/I is free software; you can redistribute it and/or modify it under
     * the terms of the GNU General Public License as published by the Free
     * Software Foundation; version 3.

     * GCC PL/I is distributed in the hope that it will be useful, but WITHOUT ANY
     * WARRANTY; without even the implied warranty of MERCHANTABILITY or
     * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
     * for more details.

     * You should have received a copy of the GNU General Public License
     * along with GCC PL/I; see the file COPYING3.  If not see
     * <http://www.gnu.org/licenses/>.
     *
     */
#ifndef PARSEDCLOPTIONLISTSTRUCTS_H_
#define PARSEDCLOPTIONLISTSTRUCTS_H_

/* Keep changes in sync with showStructDclOptionList */
struct DclOptionList
{
	union
	{
	  struct DclOptionNumeric *dclnum;
	  struct DclOptionPgm     *dclpgm;
	  struct DclOptionChar    *dclchar;
	};
	int uniontype ; /* unknown,dclnum,dclpgm  */
#define DCL_UNIONTYPE_UNKNOWN -1
#define DCL_UNIONTYPE_DCLNUM   0
#define DCL_UNIONTYPE_DCLPGM   1
#define DCL_UNIONTYPE_DCLCHAR  2
	int storageclass; /* n-state: AUTOMATIC/BASED/CONTROLLED/DEFINED/STATIC/PARAMETER */
	int internal  ; /* tristate: INTERNAL/EXTERNAL */
} ;


struct DclOptionNumeric
{
	int domain ; /* tristate: REAL, COMPLEX   */
	int base   ; /* tristate: FLOAT, FIXED    */
	int type   ; /* tristate: DECIMAL, BINARY */
	
	int precision,scale; /* integer */
} ;

struct DclOptionPgm
{ 
  struct ProcOptionList *optionlist;
  struct DclOptionList *returnsList;
  int reducible ;   /* tristate: REDUCIBLE/IRREDUCIBLE     */
  int pgmtype   ;   /* tristate: PROCEDURE/ENTRY*/
};

struct DclOptionChar
{
  int chartype ; /* n-state AREA/BIT/CHARACTER/GRAPHIC/PICTURE/WIDECHAR/DATE */
  struct Expression *expr ; /* can be NULL */
  struct DeclareElement *refer ; /* can be NULL */
  int star ;     /* boolean: '*' */
  
  /*
   * There are three different kind of expressions:
   * 
   *	'(' '*' ')'
   *	'(' expr ')'
   *	'(' expr REFER '(' varnameref ')' ')'
   * 
   */
  
} ;
#endif /*PARSEDCLOPTIONLISTSTRUCTS_H_*/
