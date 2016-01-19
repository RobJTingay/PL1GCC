    /* parseParserStack.c
     * ==================
     *
     * Implementing the datastructure for the parser stack.
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "PPDebugLog.h"
#include "parseMisc.h"
#include "parseParserStackStructs.h"
#include "parseScopeNode.h"
#include "parseProcGroupList.h"

extern int error(const char *msgtext); //TODO: fix error...
 
/*prototypes*/
struct ParserStack *newParserStack(int type);
struct StatementElement *newStatementElement(int stmttype);

struct ParserStack *initParserStack(void);
struct ParserStack *newStackScopeNode(int stmtype);

struct ParserStack *newParserStack(int type)
{ struct ParserStack *work;

	work=initParserStack();
	work->i=type; //statementtype
	return work;
}

struct ParserStack *initParserStack(void)
{ struct ParserStack *work;

	work=malloc(sizeof(struct ParserStack));
	if(work==NULL) error("parseParserStack.c(initParserStack): out of memory.");
	
	work->i=-1; //statementtype
	work->type=-1; // stacktype
	work->anyPointer=NULL;
	
	return work;
}

struct StatementElement *newStatementElement(int stmttype)
{ struct StatementElement *work;
  work=malloc(sizeof(struct StatementElement));
  if(work==NULL) error("parseParserStack.c(newStatementElement): out of memory.");
  work->type=stmttype;
  work->next=NULL;
     
  return work;
}


struct ParserStack * newStackScopeNode(int stmttype)
{ struct ParserStack *work;
  work=initParserStack(); 
  work->type=_PARSERSTACK_SCOPENODE_;
  work->i=stmttype; 
  work->scopenode=initScopeNode();
  return work;
}
