    /*
     * Copyright 2007, 2011 Henrik Sorensen
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
#ifndef PPVARIABLESTRUCTS_H_
#define PPVARIABLESTRUCTS_H_

/* create a list of names */
/* used for example by the rule
 * varnamecommalist: VARNAME	{ $$=ppInitList($1); }
 *	|	varnamecommalist ',' VARNAME	{ $$=ppAddName($1,$3); }
 *	;
 */
struct namelistelement
{ char * name;
  YYLTYPE *location;
  struct namelistelement *next;
};
	
struct namelist
{	struct namelistelement *head;
	struct namelistelement *tail;
	unsigned int listcount;
};


/* ========================== */
/* keep track of declared pp variables */

struct ppDCLelement
{ char * name;
  int type;
#define LIT_DCL_TYPE_FIXED 0
#define LIT_DCL_TYPE_CHAR  1
#define LIT_DCL_TYPE_LABEL 2

  union{ int number;
  	char *string;
  } value;
  YYLTYPE *location;
  int active ;
#define LIT_DCL_ACTIVE 1
#define LIT_DCL_INACTIVE 2
  struct ppDCLelement *next;
};


/**
 * ppReturn is used by scanner to figure out how to 
 * handle the preprocessor statement 
 */
struct ppReturn
{ char * string;
  int type;
#define LIT_PREPROCESSOR_ERROR 0
#define LIT_INCLUDE_FILE_NAME 1
#define LIT_PREPROCESSOR_GOTO_LABEL 2
#define LIT_PREPROCESSOR_GOTO_LOCATION 3
#define LIT_PREPROCESSOR_LOOP_DO 4
#define LIT_PREPROCESSOR_LOOP_END 5

  YYLTYPE *location;
};

/**
 * ppDCLLabelStack is used to keep track of the defined 
 * preprocessor labels within one include file, well within
 * the current active include file
 * 
 */
struct ppDCLLabelStack
{
	struct ppDCLelement *ppDCLLabelList;
	struct ppDCLLabelStack *prev;
};

/**
 * ppContextAnchor struct is allocated by the pl1-scanner, and 
 * passed as parameter to the pl1pp-parser.
 */
struct ppContextAnchor
{	struct ppDCLelement *ppDCLhead, *ppDCLtail;
	struct ppDCLLabelStack *ppDCLLabelTail;
	struct ppReturn *ppreturn;
	struct PLIParserParms *ppp;
	int ppGatherTokens;
	char *ppScanForLabel;
	/* %DO handling */
	int   dolevel;   /* %DO .. %END nesting level */
	struct savedtokenlist *loopCurrentToken; /* loop counter. Used when scanner has to repeat the tokenlist  */
	char *loopVarname;
	int   loopStartValue;
	int   loopEndValue;
};


#endif /*PPVARIABLESTRUCTS_H_*/
