    /* parseDclStmt.h
     * ==============
     *
     * External interface for the datatypes for declare statements.
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
#ifndef PARSEDCLSTMT_H_
#define PARSEDCLSTMT_H_

#include "parseDclStmtStructs.h"
#include "PLIParserStructs.h"

extern struct DeclareElement *newDeclareElement( void );
extern struct DeclareElement *setNewDeclareElement(char *varname,struct DclOptionList *dcloptionlist,int structureLevel,int unionType, YYLTYPE *location);
extern int mergeDeclareElements(struct DeclareElementList *list,struct DclOptionList *dcloptionlist,int structureLevel,int unionType,void * scanner);
extern struct DeclareElementList *addElement2List(struct DeclareElementList *list,struct DeclareElement *dclelement);
extern struct DeclareElementList *addElementList2List(struct DeclareElementList *list,struct DeclareElementList *list2);
extern struct DeclareElementList *newDeclareElementList(struct DeclareElement *dclelement);
extern int checkStructureLevels(struct DeclareElementList *list, struct PLIParserParms *ppp);
extern void showDeclareElement(struct DeclareElement *de);
extern char * sprintf_DeclareElement_short(struct DeclareElement *de);

#endif /*PARSEDCLSTMT_H_*/
