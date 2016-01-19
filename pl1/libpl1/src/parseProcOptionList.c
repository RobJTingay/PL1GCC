    /* parseProcOptionList.c
     * =====================
     *
     * Implementing the datastructure for the parser rule procoptionlist
     *
     * Copyright 2006, Henrik Sorensen
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
#include "parseProcOptionListStructs.h"

extern int error(const char *msgtext); //TODO: fix error definition

struct ProcOptionList *newProcOptionList(void);
void showProcOptionList(struct ScopeNode *current,struct ProcOptionList *pol);


struct ProcOptionList *newProcOptionList(void)
{ struct ProcOptionList *work;
  work=malloc(sizeof(struct ProcOptionList));
  if(!work) error("procoptionlist: out of memory.");
  
  work->main = -1 ;
  work->task = -1 ;
  
  return work;
}

/**
 * Dumps the ProcOptionList structure, using debugParser.
 */
void showProcOptionList(struct ScopeNode *current,struct ProcOptionList *pol)
{	if(pol==NULL) return ;
	if(current==NULL) return ;
	
	debugParser("showStructProcOptionList invoked\n");
	debugParser("  Attribute %s\n",sprintTristateAttribute("MAIN",pol->main));
	debugParser("  Attribute %s\n",sprintTristateAttribute("TASK",pol->task));
}
