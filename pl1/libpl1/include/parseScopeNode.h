    /* parseScopeNode.h
     * ================
     *
     * Defining the external interface to parseScopeNode.h
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
#ifndef PARSESCOPENODE_H_
#define PARSESCOPENODE_H_

#include "parseScopeNodeStructs.h"

extern struct ScopeNode *newScope(struct ScopeNode *c,struct ListOfNames *l,struct ParserStack *ps);
extern struct ScopeNode *newScopeRoot(void);
extern struct ScopeNode *initScopeNode(void);
extern struct ScopeNode *endScope(struct ScopeNode *c,struct ParserStack *ps,struct ScopeNode *scopeRoot);
extern void   showScopeTree(struct ScopeNode *sn);
extern void   showScopeTreeWhole(char *,struct ScopeNode *sn);
extern struct ScopeNode *lookUpMatchingNode(struct ScopeNode *s, char *n);
extern int checkNameInLabellist(struct ListOfNames *, char *);



#endif /*PARSESCOPENODE_H_*/
