    /* TokenLocation.h
     * ===============
     *
     * External definitions for TokenLocation.c
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

#ifndef TOKENLOCATION_H_
#define TOKENLOCATION_H_

#include "TokenLocationStructs.h"

extern void  SET_yylloc(YYLTYPE *y,
                   struct PLIParserIncludeFile *includeBuffer,
			       int *fl, /* first line */
			       int *fc, /* first column */
			       int *fo, /* first offset */
			       int ll, /* last line */
			       int lc, /* last column */
			       int lo, /* last offset */
			       int ln  /* length */
			       ) ;	  
extern char * sprintf_yylloc(YYLTYPE *y);
extern char * sprintf_yylloc_short(YYLTYPE *y);

#endif /*TOKENLOCATION_H_*/
