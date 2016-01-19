    /* SavedTokenStructs.h
     * ===================
     *
     * Structure definitions.
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


#ifndef SAVEDTOKENSTRUCTS_H_
#define SAVEDTOKENSTRUCTS_H_

struct savedtokenlist 
{ /*TODO: int     disposable; /-* 0: cannot be freed. 1:token can be freed. *-/ */
  int     token;
  YYLTYPE *locations;
  char    *string;
  struct  savedtokenlist *next;
};

struct anyTokenList
{ struct savedtokenlist *head;
  struct savedtokenlist *tail;
  int count;
  const char *name ;
};



#endif /*SAVEDTOKENSTRUCTS_H_*/
