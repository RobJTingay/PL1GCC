    /*
     * Copyright 2007, 2011 Henrik Sorensen
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
#ifndef PL1PARSER_H_
#define PL1PARSER_H_

#include "TokenLocationStructs.h"
#include "PLIParserStructs.h"

extern struct PLIParserParms *initPL1ParserParms(void); 
extern int pl1_parser (struct PLIParserParms *pl1parserparms); // NOTE: defined in pl1-parser.y

extern struct PLIParserIncludeFileList *newPLIParserIncludeFileList(void);
extern struct PLIParserIncludeFile * addIncludeFile(struct PLIParserIncludeFileList *list,int type ,char *name,struct PLIParserIncludeFile *current, YYLTYPE *location);

extern void produceXrefList(struct ScopeNode *current);
extern void produceIncludeList(struct PLIParserIncludeFileList *list);

extern int showWelcome(struct PLIParserParms *ppp);

#endif /*PL1PARSER_H_*/
