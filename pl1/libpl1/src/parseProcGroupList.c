    /* parseProcGroupList.c
     * ====================
     *
     * Implementing the datastructure for a procedure group list,
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
 #include "parseParserStack.h"
 #include "parseScopeNodeStructs.h"
 #include "parseProcGroupListStructs.h"
 #include "parseProcOptionList.h"
 #include "parseDclOptionList.h"
 #include "pl1.tab.h" 


extern int error(const char *msgtext); //TODO: fix error


/* prototypes */

struct ProcGroupList *newProcGroupList(void);
struct ProcGroupList *setProcGroupListParameterNames
         (struct ProcGroupList *
         ,struct ListOfNames *);
struct ProcGroupList *setProcGroupListParserStack(struct ProcGroupList *,struct ParserStack *);
void showStructProcGroupList(struct ScopeNode *current,struct ProcGroupList *pgl);
int setProcGroupListParameterNameList(struct ProcGroupList *pgl, struct ListOfNames *parameternames);
int setProcGroupListOptionList(struct ProcGroupList *pgl, struct ProcOptionList *pol);
int setProcGroupListReturnsList(struct ProcGroupList *pgl, struct DclOptionList *dol);
	 /* ProcGroupList           */

/**
 * Allocates a new ProcGroupList structure
 * 
 */	
struct ProcGroupList *newProcGroupList(void)
{ struct ProcGroupList *work;
	debugParser("newProcGroupList invoked\n");
  work = malloc(sizeof(struct ProcGroupList));
  if(!work) error("out of memory: alloc of ProcGroupList");
  work->parameters=NULL;
  work->optionlist=NULL;
  work->returnsList=NULL;
  work->reducible=-1; //TODO: define constant ATTRIBUTE_UNDEFINED
  work->order=-1;
  work->recursive=-1;
  work->chargraphic=-1;
  return work;
}

struct ProcGroupList *setProcGroupListParameterNames
(
  struct ProcGroupList *pgl
 ,struct ListOfNames *l
 )
{   debugParser("setProcGroupListParameterNames invoked\n");
	pgl->parameters=l;
	
	return pgl;
}


/**
 * Update the ProcGroupList structure with the ParserStack element.
 *
struct ProcGroupList *setProcGroupListParserStack
(
    struct ProcGroupList * pgl
   ,struct ParserStack *ps
)
{ 	debugParser("setProcGroupListParser invoked\n");
	
	switch(ps->i) /-* statement type *-/
  { case _varnamecommalist_ : pgl->parameters=ps->l;
  	                     break;
  	default: break;
  }
  
  return pgl;	
}
*/
/**
 * Dumps the ProcGroupList structure, using debugParser.
 */
void showStructProcGroupList(struct ScopeNode *current,struct ProcGroupList *pgl)
{ int i;
	if(pgl==NULL) return;
	struct NameNode *n;
	debugParser("showStructProcGroupList invoked\n");
	debugParser("Name of procedure %s \n",current->name->head->name);
	if(pgl->parameters==NULL)
	{  debugParser("  has no parameters\n");
	}
	else
	for(i=1,n=pgl->parameters->head;n;n=n->next,i++)
	{
		debugParser("  parameter [%i] name %s \n",i,n->name);
	}	
	debugParser("  Attribute %s\n",sprintTristateAttribute("REDUCIBLE",pgl->reducible));
	debugParser("  Attribute %s\n",sprintTristateAttribute("ORDER",pgl->order));
	debugParser("  Attribute %s\n",sprintTristateAttribute("RECURSIVE",pgl->recursive));
	debugParser("  Attribute %s\n",sprintTristateAttribute("CHARGRAPHIC",pgl->chargraphic));
	showProcOptionList(current,pgl->optionlist);
	showStructDclOptionList(pgl->returnsList);
}


int setProcGroupListParameterNameList(struct ProcGroupList *pgl, struct ListOfNames *parameternames)
{
	pgl->parameters=parameternames;
	return 0;
}


int setProcGroupListOptionList(struct ProcGroupList *pgl, struct ProcOptionList *pol)
{
	pgl->optionlist=pol;
	return 0;
}


int setProcGroupListReturnsList(struct ProcGroupList *pgl, struct DclOptionList *dol)
{
	pgl->returnsList=dol;
	return 0;
}
