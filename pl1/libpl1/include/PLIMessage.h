    /* PLIMessage.h
     * ============
     *
     * Contains the external entry points for the PLIMessage functions.
     *
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
     *
     */
#ifndef PLIMESSAGE_H_
#define PLIMESSAGE_H_

#include "PLIMessageStructs.h"
#include "PLIParserStructs.h"


extern struct PLIMessage * initPLIMessage(void);

extern int addMessage(struct PLIMessage *, char *msg);

extern void showMessage(const char *header,struct PLIMessage *);


extern int addInfoMessage(struct PLIParserParms *ppp, char *msg);
extern int addErrorMessage(struct PLIParserParms *ppp, char *msg);
extern int addWarnMessage(struct PLIParserParms *ppp, char *msg);

#endif /*PLIMESSAGE_H_*/
