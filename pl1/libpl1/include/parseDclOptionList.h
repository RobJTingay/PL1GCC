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
#ifndef PARSEDCLOPTIONLIST_H_
#define PARSEDCLOPTIONLIST_H_

#include "parseDclOptionListStructs.h"

extern struct DclOptionList *newDclOptionList(void);
extern struct DclOptionNumeric *newDclOptionNumeric(void);
extern struct DclOptionPgm *newDclOptionPgm(void);
extern struct DclOptionChar *newDclOptionChar(void);

extern int initDclOptionNumeric(struct DclOptionList *dol);
extern int initDclOptionPgm(struct DclOptionList *dol);
extern int initDclOptionChar(struct DclOptionList *dol);

extern void showStructDclOptionList(struct DclOptionList *dol);
extern void showStructDclOptionNumeric(struct DclOptionNumeric *don);
extern void showStructDclOptionPgm(struct DclOptionPgm *dop);
extern void showStructDclOptionChar(struct DclOptionChar *doc);
extern char *sprintf_DclOptionList_short(struct DclOptionList *dol);

extern int setDclElementAttr(struct DclOptionList *old, struct DclOptionList *new);
#endif /*PARSEDCLOPTIONLIST_H_*/
