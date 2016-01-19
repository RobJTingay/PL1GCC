    /* parseParserStackStructs.h
     * =========================
     *
     * Defining structures used by parseParserStackStructs.c
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

#ifndef PARSEPARSERSTACKSTRUCTS_H_
#define PARSEPARSERSTACKSTRUCTS_H_

struct ParserStack
{ int i;
  int type;
#define _PARSERSTACK_SCOPENODE_ 1
  union {
  void *anyPointer;
  struct ListOfNames *l;
  struct ProcGroupList *pgl;
  struct StatementElement *stmtelem;
  struct ScopeNode *scopenode;
  };
};

struct StatementElement
{ int type;
  struct StatementElement *next; 
};

#endif /*PARSEPARSERSTACKSTRUCTS_H_*/
