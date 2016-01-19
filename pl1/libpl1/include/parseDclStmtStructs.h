    /* parseDclStmtStructs.h
     * =====================
     *
     * Structure definitions for the datatypes to the DECLARE statement.
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
#ifndef PARSEDCLSTMTSTRUCTS_H_
#define PARSEDCLSTMTSTRUCTS_H_

/*
 * Structures for the declare symbollist
 * 
 */

struct DeclareElementList
{
	struct DeclareElement *head,*tail;
};

struct DeclareElement
{ /* remember to keep showDeclareElement in sync with changes */
	struct DeclareElement *next;	
	char * dclName;
	struct DclOptionList * dclAttr;
	int implied  ; /* boolean: IMPLIED  */
	/* Structure definitions */
	int structureLevel;
	struct DeclareElement *structureParent;     /* level - 1  */
	struct DeclareElement *structureSibling;    /* same level */
	struct DeclareElement *structureFirstChild; /* level + 1  */
	
	/* location tracking */
	YYLTYPE *dclLocation ; /* type is defined in TokenLocationStructs.h */
	// TODO have references as list of locations as well
};


#endif /*PARSEDCLSTMTSTRUCTS_H_*/
