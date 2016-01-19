    /* PLIMESSAGE.C
     * =============
     *
     * PL/I Compiler message handling
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
#include <errno.h>
#include <stdlib.h>

#include "TokenLocationStructs.h"
#include "PLIParserStructs.h"
#include "PLIMessageStructs.h"

extern int error(const char *msgtext); //TODO: prober include for error function.
/* prototypes */
int addMessage(struct PLIMessage *msg, char *text);
struct PLIMessage * initPLIMessage(void);
void showMessage(const char * header, struct PLIMessage* msg);
int addInfoMessage(struct PLIParserParms *ppp, char *msg);
int addErrorMessage(struct PLIParserParms *ppp, char *msg);
int addWarnMessage(struct PLIParserParms *ppp, char *msg);
/* ----------------- */
int addMessage(struct PLIMessage *msg, char *text)
  {
    struct PLIMessageElement *work;
    if (msg==NULL)
      error("get real ... PLIMessage has to be allocated ");

    work=malloc(sizeof(struct PLIMessageElement));
    if (work==NULL)
      error("out of memory allocating PLIMessageElement");
    work->next=NULL;
    work->text=text; //strdup?
    if (msg->head==NULL) /* first element */
      {
        msg->head=work;
        msg->tail=msg->head;
      }
    else
      {
        msg->tail->next=work;
        msg->tail=work;
      }

    msg->count++;

    return 0;
  }

struct PLIMessage * initPLIMessage(void)
  {
    struct PLIMessage* work;

    work=malloc(sizeof(struct PLIMessage));
    if (work==NULL)
      error("could not allocate PLIMessage");

    work->head=NULL;
    work->tail=NULL;
    work->count=0;

    return work;
  }

void showMessage(const char * header, struct PLIMessage* msg)
  {
    struct PLIMessageElement *work;
    int i;

    if (msg==NULL)
      return;
    if (msg->count==0)
      return;

    printf("*** %s ***\n", header);

    for (work=msg->head; work; work=work->next)
      {
        i=strlen(work->text);;
        if (*(work->text+i)=='\n')
          printf("%s", work->text);
        else
          printf("%s\n", work->text);
      }
  }

/** 
 * 
 * Write a messages to the compiler output structure
 * 
 */
int addInfoMessage(struct PLIParserParms *ppp, char *msg)
  {
    if (ppp==NULL)
      error("get real, parser parameters are null");

    return addMessage(ppp->msgInfos, msg);
  }

int addErrorMessage(struct PLIParserParms *ppp, char *msg)
  {
    if (ppp==NULL)
      error("get real, parser parameters are null");

    return addMessage(ppp->msgErrors, msg);
  }

int addWarnMessage(struct PLIParserParms *ppp, char *msg)
  {
    if (ppp==NULL)
      error("get real, parser parameters are null");

    return addMessage(ppp->msgWarns, msg);
  }

