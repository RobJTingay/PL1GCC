    /* parseProcGroupList.h
     * ====================
     *
     * Defining the external interface to parseProcGroupList.c
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
     */
#ifndef PARSEPROCGROUPLIST_H_
#define PARSEPROCGROUPLIST_H_

#include "parseProcGroupListStructs.h"

extern struct ProcGroupList *newProcGroupList(void);
extern struct ProcGroupList *setProcGroupListParameterNames
         (struct ProcGroupList *
         ,struct ListOfNames *);
extern struct ProcGroupList *setProcGroupListParserStack(struct ProcGroupList *,struct ParserStack *);
extern void showStructProcGroupList(struct ScopeNode *current,struct ProcGroupList *pgl);
extern int setProcGroupListParameterNameList(struct ProcGroupList *pgl, struct ListOfNames *parameternames);
extern int setTristateAttribute(int *variable, int data);
extern int setProcGroupListOptionList(struct ProcGroupList *pgl, struct ProcOptionList *pol);
extern int setProcGroupListReturnsList(struct ProcGroupList *pgl, struct DclOptionList *dol);
#endif /*PARSEPROCGROUPLIST_H_*/
