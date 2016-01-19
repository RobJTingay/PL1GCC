    /* TokenLocation.c
     * ===============
     *
     * Functions to keep track token locations.
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
     *
     */


#include <stdio.h>
#include <string.h>
#include <errno.h>


#include "PPDebugLog.h"
#include "TokenLocationStructs.h"
#include "PLIParserStructs.h"

/* =========================== */
/* location ...                */
/* SET_yylloc
 * sprintf_yylloc(YYLTYPE *y)
 */


/* Prototypes */
void  SET_yylloc(YYLTYPE *y,
                   struct PLIParserIncludeFile *includeBuffer,
			       int *fl, /* first line */
			       int *fc, /* first column */
			       int *fo, /* first offset */
			       int ll, /* last line */
			       int lc, /* last column */
			       int lo, /* last offset */
			       int ln  /* length */
			       ) ;

char * sprintf_yylloc(YYLTYPE *y);
char * sprintf_yylloc_short(YYLTYPE *y);

 
/* SET_yylloc
 * extern YYLTYPE yylloc; defined in pl1pp-parser.h
 * assign values to pl1ltype structure.
 * f: pointer to current struct fileListfile number,
 *    assumed to be same for first and last
 * fl,fc,fo: first line and column and offset
 * ll,lc,lo: last line and column and offset
 * ln: length of token
 * if fl<0 is assumed to be ll, and fc=lc-ln
 *
 * Side effects
 * ------------
 * sets fl and fc to -1, if fl>=0.
 */

void  SET_yylloc(YYLTYPE *y,
                   struct PLIParserIncludeFile *includeBuffer,
			       int *fl, /* first line */
			       int *fc, /* first column */
			       int *fo, /* first offset */
			       int ll, /* last line */
			       int lc, /* last column */
			       int lo, /* last offset */
			       int ln  /* length */
			       ) 
{
  if(y==NULL) return;
  
  y->first_file=includeBuffer; 
  y->last_file=includeBuffer; 

  y->last_line=ll; 
  y->last_column=lc; 
  y->last_offset=lo;

  if(*fl>=0) { 

    /* set initial values for next file */

    y->first_line=*fl; 
    y->first_column=*fc; 
    y->first_offset=*fo;

    *fl=-1;*fc=-1;*fo=-1;

  } else {
    y->first_line=ll; 
    y->first_column=lc-ln; 
    y->first_offset=lo-ln;
  }
}

/** returns the location formatted in string */
char * sprintf_yylloc(YYLTYPE *y) {
  char *buffer=malloc(512);
  int f1,f2;

  if (y==NULL) return NULL;
  
  if (y->first_file) f1=y->first_file->refno; else f1=0;
  if (y->last_file)  f2=y->last_file->refno ; else f2=0;
  
  /* TODO: add filename to output */
  if (y->first_line<y->last_line) {
    snprintf(buffer,512,"(from file,line,column)-(to file,line,column)([%i],%i,%i)-([%i],%i,%i) offset[%i-%i[\t",
       f1,
	   y->first_line,
	   y->first_column,
	   f2,
	   y->last_line,
	   y->last_column,
	   y->first_offset,
	   y->last_offset);
  } else {
    /* start and end location is on same line */
    snprintf(buffer,512,"(File,line,column from,to)([%i],%i,%i-%i) offset from-to[%i-%i[\t",
       f1,
	   y->first_line,
	   y->first_column,
	   y->last_column,
	   y->first_offset,
	   y->last_offset);
  }
  
  return buffer;
}

/** returns the location formatted in string */
char * sprintf_yylloc_short(YYLTYPE *y) {
  char *buffer=malloc(100);
  if (y==NULL) return NULL;
  if(y->first_file)
    snprintf(buffer,100,"[%i] (%i,%i)\t"
      ,  y->first_file->refno
	  ,  y->first_line
	  ,  y->first_column
             );  
  else
    snprintf(buffer,100,"(%i,%i)\t"
	  ,  y->first_line
	  ,  y->first_column
             );
  
  return buffer;
}

