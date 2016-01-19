    /* parseParserStack.h
     * ==================
     *
     * External interface for parseParserStack.c
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

#ifndef PARSEPARSERSTACK_H_
#define PARSEPARSERSTACK_H_

#include "parseParserStackStructs.h"

extern struct ParserStack *newParserStack(int type);
extern struct StatementElement *newStatementElement(int stmttype);
extern struct ParserStack * newStackScopeNode(int stmtype);
extern struct ParserStack * initStackScopeNode(void);
#endif /*PARSEPARSERSTACK_H_*/
