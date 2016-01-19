    /* TokenLocationStructs.h
     * ======================
     *
     * Structs needed by Tokenlocation.h/c
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

#ifndef TOKENLOCATIONSTRUCTS_H_
#define TOKENLOCATIONSTRUCTS_H_

/* 
   include the preprocessor first so that we don't get
   hammered by token #define's in the open namespace.
*/

typedef struct pl1ltype {
  struct PLIParserIncludeFile *first_file;
  int first_line;
  int first_column;
  int first_offset;
  struct PLIParserIncludeFile *last_file;  
  int last_line;
  int last_column;
  int last_offset;

} pl1ltype;

/* note YYLTYPE has to be defined before include of pl1.tab.h */
#define YYLTYPE pl1ltype

/* ============================= */
/* Location tracking ....        */
/* YYLLOC_DEFAULT -- Compute the default location
  (before the actions are run).  */

#define YYLLOC_DEFAULT(Current, Rhs, N)                    \
	(Current).first_file   = (Rhs)[1].first_file;      \
	(Current).first_line   = (Rhs)[1].first_line;      \
	(Current).first_column = (Rhs)[1].first_column;    \
	(Current).first_offset = (Rhs)[1].first_offset;    \
	(Current).last_file    = (Rhs)[(N)].last_file;     \
	(Current).last_line    = (Rhs)[(N)].last_line;     \
	(Current).last_column  = (Rhs)[(N)].last_column;   \
	(Current).last_offset  = (Rhs)[(N)].last_offset;




#endif /*TOKENLOCATIONSTRUCTS_H_*/
