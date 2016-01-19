    /* PP-VARIABLE.C
     * =============
     *
     * PL/I preprocessor variable handling
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
#include "PPDebugLog.h"
#include "TokenLocationStructs.h"
#include "PLIParserStructs.h"
#include "TokenLocation.h"
#include "SavedTokenStructs.h"
#include "ppVariableStructs.h"
#include "ppIncludeFileUtils.h"

#include "PLIMessage.h"
/* ======================== */
extern int asprintf(char **strp, const char *fmt, ...);
extern int error(const char *msgtext); //TODO: prober include for error function.

/* prototypes */
int ppSetIntValue(char *name, int value, YYLTYPE *locp);
int ppSetStringValue(char *name, char *value, YYLTYPE *locp);
int ppDefineLabel(char *name, int type, YYLTYPE *location);
int ppDefineVariable(char *name, int type, YYLTYPE *locp);
int ppDefineVariableList(struct namelist *, int type);
struct namelist *ppInitList(char *name, YYLTYPE *locp);
struct namelist *ppAddName(struct namelist*, char *name, YYLTYPE *locp);
int ppShowNameList(struct namelist* nl);
int ppShowNameListElement(struct namelistelement *w);

int ppGetIntValue(char *name, YYLTYPE *locp);
char * ppGetStringValue(char *name, YYLTYPE *locp);
struct ppDCLelement * ppFindDCLName(char *name);
struct ppDCLelement
    * ppFindDCLNameInList(struct ppDCLelement *head, char *name);
char * ppGetValueAsString(struct ppDCLelement *, YYLTYPE *locp);

int ppSetActivateStmt(char *name);
int ppSetDeactivateStmt(char *name);
int ppSetGotoStmt(char *label, YYLTYPE *llocp);
int ppSetIncludeStmt(char *fileName, char *environmentVariable, YYLTYPE *llocp);
int ppSetNoteStmt(char *level, char *msgtext);
int ppSetDoStmt(char *loopvarname, int fromexpr, int toexpr, YYLTYPE *llocp);
int ppSetEndStmt(YYLTYPE *llocp);

int disposeDCLElement(struct ppDCLelement *w);

struct ppDCLLabelStack * ppPushDCLLabelStack(struct ppDCLLabelStack *tail);
struct ppDCLLabelStack * ppPopDCLLabelStack(struct ppDCLLabelStack *tail);
int ppDisposeDCLLabelStackElement(struct ppDCLLabelStack *work);

int ppAnalyseStmt(char *labelName, struct anyTokenList *inTokensList,
    struct anyTokenList *outTokensList);

struct ppContextAnchor *newppContextAnchor(void);

/* =========================== */

extern struct ppContextAnchor *scannerContext; // set in pl1pp-parser.y, which gets it from pl1-scanner.l
//TODO scannerContext hould be passed as parameter

/* =========================== */
/**
 * Return 0: error when assigning value to variable
 *        1: value successfully assigned
 * 
 */
int ppSetIntValue(char *name, int value, YYLTYPE *locp)
  {
    struct ppDCLelement *w=ppFindDCLName(name);
    char* buf;

    if (w)
      {
        debugParserpp("$$$ ppSetIntValue Assigning %s (%s) to int value %i\n",
            name, w->name, value);
        if (w->type==LIT_DCL_TYPE_FIXED)
          w->value.number=value ;
        else
          {
            buf
                =sprintfString(
                    "Preprocessor error at (%s): wrong type for %s. Integer expected\n",
                    sprintf_yylloc(locp), name);
            addErrorMessage(scannerContext->ppp, buf);
            return 0;
          }
      }
    else
      {
        buf=sprintfString(
            "Preprocessor error at (%s): Variable is undefined: %s. \n",
            sprintf_yylloc(locp), name);
        addErrorMessage(scannerContext->ppp, buf);
        return 0;
      }
    return 1;
  }

/**
 * Return 0: error when assigning value to variable
 *        1: value successfully assigned
 * 
 */
int ppSetStringValue(char *name, char *value, YYLTYPE *locp)
  {
    struct ppDCLelement *w=ppFindDCLName(name);
    char * buf;

    if (w)
      {
        debugParserpp(
            "$$$ ppSetStringValue Assigning %s (%s) to string value %s\n",
            name, w->name, value);
        if (w->type==LIT_DCL_TYPE_CHAR)
          w->value.string=value ;
        else
          {
            buf
                =sprintfString(
                    "Preprocessor error at (%s): wrong type for %s. String expected\n",
                    sprintf_yylloc(locp), name);
            addErrorMessage(scannerContext->ppp, buf);
            return 0;
          }
      }
    else
      {
        buf=sprintfString(
            "Preprocessor Error at (%s): Variable is undefined: %s. \n",
            sprintf_yylloc(locp), name);
        addErrorMessage(scannerContext->ppp, buf);
        return 0;
      }
    return 1;
  }

/**
 * Return the value of a variable as an int.
 * If the variable is not defined as a fixed, return zero.
 * TODO: what if pp variable is inactive ?
 * 
 */
int ppGetIntValue(char * name, YYLTYPE *locp)
  {
    struct ppDCLelement *w=ppFindDCLName(name);
    char * buf;

    if (w)
      {
        debugParserpp("$$$ ppGetIntValue %s:%i\n", name, w->value.number);
        if (w->type==LIT_DCL_TYPE_FIXED)
          return w->value.number;
        else
          {
            buf
                =sprintfString(
                    "Preprocessor error at (%s): wrong type for %s. Integer expected\n",
                    sprintf_yylloc(locp), name);
            addErrorMessage(scannerContext->ppp, buf);
            return 0;
          }
      }
    else
      {
        buf=sprintfString(
            "Preprocessor Error at (%s): Variable is undefined: %s. \n",
            sprintf_yylloc(locp), name);
        addErrorMessage(scannerContext->ppp, buf);
        return 0;
      }
  }

/**
 * Return the value of a variable as a string.
 * 
 * If the variable is not defined as a string, return NULL.
 * 
 */
char * ppGetStringValue(char * name, YYLTYPE *locp)
  {
    struct ppDCLelement *w=ppFindDCLName(name);
    char * buf;
    if (w)
      {
        debugParserpp("$$$ ppGetStringValue %s:%s\n", name, w->value.string);
        if (w->type==LIT_DCL_TYPE_CHAR)
          return w->value.string;
        else
          {
            buf
                =sprintfString(
                    "Preprocessor Error at (%s): wrong type for %s. String expected\n",
                    sprintf_yylloc(locp), name);
            addErrorMessage(scannerContext->ppp, buf);
            return NULL;
          }
      }
    else
      {
        buf=sprintfString(
            "Preprocessor Error at (%s): Variable is undefined: %s. \n",
            sprintf_yylloc(locp), name);
        addErrorMessage(scannerContext->ppp, buf);
        return NULL;
      }

  }

/**
 * Returns the content of a variable converted to a string.
 * If the variable is not defined or an error occurs when 
 * converting the values to a string, NULL is returned.
 * 
 * 
 */
char * ppGetValueAsString(struct ppDCLelement *w, YYLTYPE *locp)
  {
    char *retv; /* gets allocated by asprintf */
    char *buf;
    int i;
    if (w)
      {
        if (w->type==LIT_DCL_TYPE_FIXED)
          {
            debugParserpp("$$$ ppGetValueAsString %s type %i:%i\n", w->name,
                w->type, w->value.number);

            i=asprintf(&retv, "%i", w->value.number); /* asprintf allocates a buffer */
            if (i<0)
              {
                buf
                    =sprintfString(
                        "Preprocessor Error at (%s): error converting %i to string.\n",
                        sprintf_yylloc(locp), w->value.number);
                addErrorMessage(scannerContext->ppp, buf);
                return NULL;
              }
            if (i==0)
              {
                debugParserpp("weird, converting %i to empty string?!?\n",
                    w->value.number);
                return NULL;
              }

            debugParserpp("$$$    returning %s\n", retv);
            return retv;
          }
        else if (w->type==LIT_DCL_TYPE_CHAR)
          {
            debugParserpp("$$$ ppGetValueAsString %s type %i:%s\n", w->name,
                w->type, w->value.string);
            return w->value.string;
          }
        else
          {
            buf
                =sprintfString(
                    "Preprocessor Error at (%s): wrong type for %s. String or number expected\n",
                    sprintf_yylloc(locp), w->name);
            addErrorMessage(scannerContext->ppp, buf);
            return NULL;
          }
      }
    else
      {
        buf
            =sprintfString(
                "Preprocessor Error at (%s): ppGetValueAsString: Calling sequence error. Variable is NULL\n",
                sprintf_yylloc(locp));
        addErrorMessage(scannerContext->ppp, buf);
        return NULL;
      }

  }

/**
 * Find name within scannerContext->ppDCLhead.
 * 
 * Note:
 *    This is used for normal preprocessor variables only. 
 *    Active labels are found using via scannerContext->ppDCLLabelTail->ppDCLLabelList 
 * 
 */
struct ppDCLelement * ppFindDCLName(char *name)
  {
    debugParserpp("$$$ ppFindDCLName %s\n", name);

    return ppFindDCLNameInList(scannerContext->ppDCLhead, name);
  }

/**
 * Lookup a name within a struct ppDCLelement list of names.
 * 
 * return NULL: name is not found
 *        struct ppDCLement * if name is found
 * 
 */
struct ppDCLelement * ppFindDCLNameInList(struct ppDCLelement *head, char *name)
  {
    struct ppDCLelement *w;
    int i;
    debugParserpp("$$$ ppFindDCLNameList %s\n", name);

    for (i=0, w=head; w; w=w->next, i++)
      {
        if (strcasecmp(name, w->name)==0)
          {
            debugParserpp(
                "$$$ i: %i, name: %s, w->name: %s, strcasecmp(name,w->name): %i\n",
                i, name, w->name, strcasecmp(name, w->name));
            return w;
          }
      }

    return NULL; /* not found */
  }

/**
 * Define a preprocessor variable or label.
 * 
 * If type is LIT_DCL_TYPE_LABEL then invoke ppDefineLabel to define the pp-label.
 * 
 * 
 */

int ppDefineVariable(char *name, int type, YYLTYPE *locp)
  {
    char * buf;
    struct ppDCLelement *w;
    debugParserpp("$$$ ppDefineVariable Defining variable %s as type %i\n",
        name, type);

    if (type==LIT_DCL_TYPE_LABEL)
      return ppDefineLabel(name, type, locp);

    w=ppFindDCLName(name);
    if (w)
      {
        debugParserpp("$$$ ppDefineVariable Defining variable %s as type %i\n",
            name, type);
        buf=sprintfString(
                "Preprocessor Error at (%s): \%DECLARE failed for variable named %s: already declared\n",
                sprintf_yylloc(locp), name);
        addErrorMessage(scannerContext->ppp, buf);
        return 0;
      }

    w=malloc(sizeof(struct ppDCLelement));
    if (!w)
      error("Internal compiler error. ppDefineVariable: Out of memory\n");

    w->name=name;
    w->type=type;
    w->location=malloc(sizeof(YYLTYPE));
    memcpy(w->location, locp, sizeof(YYLTYPE));
    w->active=LIT_DCL_ACTIVE;
    w->next=NULL;

    if (!scannerContext->ppDCLhead) /* first element */
      {
        scannerContext->ppDCLhead=w;
        scannerContext->ppDCLtail=w;
      }
    else
      {
        scannerContext->ppDCLtail->next=w;
        scannerContext->ppDCLtail=w;
      }

    return 1;
  }

/**
 * Define the LABEL in the correct structure for labels.
 * 
 * 
 * 
 */
int ppDefineLabel(char *name, int type, YYLTYPE *locp)
  {
    char * buf;
    struct ppDCLelement *w, *temp;

    debugParserpp(
        "$$$ ppDefineLabel Defining label %s as type %i, and location %s\n",
        name, type, sprintf_yylloc(locp));
    w=ppFindDCLName(name);

    if (w)
      {
        debugParserpp("$$$ ppDefineLabel Defining label %s as type %i\n", name,
            type);
        buf
            =sprintfString(
                "Preprocessor Error at (%s): \%DECLARE failed for label named %s: already declared\n",
                sprintf_yylloc(locp), name);
        addErrorMessage(scannerContext->ppp, buf);
        return 0;
      }

    if (!scannerContext)
      {
        error("ppDefineLabel:scannerContext is null\n");
        return 0;
      }
    if (!scannerContext->ppDCLLabelTail)
      {
        error("ppDefineLabel:scannerContext->ppDCLLabelTail is null\n");
        return 0;
      }

    w=ppFindDCLNameInList(scannerContext->ppDCLLabelTail->ppDCLLabelList, name);
    if (w)
      { /* LABEL is already defined, emit warning and update the location to the 
       * specified location
       */
        if (w->location->first_offset != locp->first_offset)
          {
            memcpy(w->location, locp, sizeof(YYLTYPE));
            addWarnMessage(scannerContext->ppp, sprintfString(
                "Location of label %s, has been overwritten.", name));
          }
      }
    else
      { /* Add label to list of labels */
        w=malloc(sizeof(struct ppDCLelement));
        if (!w)
          error("Internal compiler error. ppDefineLabel: Out of memory\n");

        w->name=name;
        w->type=type;
        w->location=malloc(sizeof(YYLTYPE));
        memcpy(w->location, locp, sizeof(YYLTYPE));
        w->active=LIT_DCL_ACTIVE;
        w->next=NULL;

        if (!scannerContext->ppDCLLabelTail->ppDCLLabelList) /* first element */
          {
            scannerContext->ppDCLLabelTail->ppDCLLabelList=w;
          }
        else
          { /* find last element in the list */
            for (temp=scannerContext->ppDCLLabelTail->ppDCLLabelList; temp->next; temp
                =temp->next)
              ;
            temp->next=w;
          }
      }
    return 1;
  }

/**
 * For all the variable names in namelist, define with the type.
 */
int ppDefineVariableList(struct namelist *nl, int type)
  {
    struct namelistelement *w;
    int isOK;
    debugParserpp(
        "$$$ ppDefineVariableList Defining list of %i variables as type %i\n",
        nl->listcount, type);

    ppShowNameList(nl);

    for (isOK=1, w=nl->head; isOK&&w; w=w->next)
      isOK=ppDefineVariable(w->name, type, w->location);

    return isOK;
  }

/**
 * define a list, and add first element to list
 */
struct namelist *ppInitList(char * name, YYLTYPE *location)
  {
    struct namelist *nl;
    debugParserpp("$$$ ppInitList with name %s\n", name);

    nl=malloc(sizeof(struct namelist));
    if (nl==NULL)
      error("Internal compiler error: ppInitList out of memory.");
    nl->head=NULL;
    nl->tail=NULL;
    nl->listcount=0;

    return ppAddName(nl, name, location);
  }

/**
 * add name to list 
 */
struct namelist *ppAddName(struct namelist* nl, char * name, YYLTYPE *location)
  {
    struct namelistelement *w;
    debugParserpp("$$$ ppAddName %s\n", name);
    if (nl==NULL)
      error("Internal compiler error: ppAddName called with NULL namelist.");

    w=malloc(sizeof(struct namelistelement));
    if (w==NULL)
      error("Internal compiler error: ppAddName out of memory.");
    w->next=NULL;
    w->name=name;
    w->location=location;

    if (nl->head==NULL)
      { // first element 
        nl->head=w;
        nl->tail=w;
      }
    else
      {
        nl->tail->next=w;
        nl->tail=w;
      }

    nl->listcount++;

    return nl;
  }

/* shows all names in the struct namelist */
int ppShowNameList(struct namelist* nl)
  {
    struct namelistelement *w;
    debugParserpp("$$$ ppShowNameList. Expecting %i names\n", nl->listcount);
    int i;
    for (i=0, w=nl->head; w; w=w->next, i++)
      {
        debugParserpp("ppShowNameList[%i]:", i);
        ppShowNameListElement(w);
        debugParserpp("\n");
      }
    return 1;
  }

/* shows the name of a struct namelistelement */
int ppShowNameListElement(struct namelistelement *w)
  {
    debugParserpp("%s", w->name);
    return 1;
  }

/**
 * 
 * Handle %GOTO label.
 * 
 * If the label has already been met, then return the location of the %label: statement,
 * else return the name of the label to search for. Normally the caller will start to
 * discard text until the matching preprocessor label statement is found.
 * 
 * Returns 0: Error finding the label
 *         1: The label was found, and scannerContext updated accordingly.
 * 
 * The ppreturn->string is set to the name of the label
 * 
 * The ppreturn->type is set to either
 *    LIT_PREPROCESSOR_GOTO_LABEL
 *    LIT_PREPROCESSOR_GOTO_LOCATION
 *    LIT_PREPROCESSOR_ERROR
 * 
 */
int ppSetGotoStmt(char *label, YYLTYPE *locp)
  {
    struct ppDCLelement * temp;

    scannerContext->ppreturn->string=strdup(label);

    temp=ppFindDCLNameInList(scannerContext->ppDCLLabelTail->ppDCLLabelList,
        label);
    if (temp==NULL) /* name not found, keep scanning until found */
      {
        scannerContext->ppreturn->type=LIT_PREPROCESSOR_GOTO_LABEL;
        /* TODO: check setting scannerContext->ppreturn->location=NULL; */
      }
    else if (temp->type==LIT_DCL_TYPE_LABEL)
      {
        scannerContext->ppreturn->type=LIT_PREPROCESSOR_GOTO_LOCATION;
        memcpy(scannerContext->ppreturn->location, temp->location,
            sizeof(YYLTYPE));
      }
    else
      {
        scannerContext->ppreturn->type=LIT_PREPROCESSOR_ERROR;
        debugParserpp("\%GOTO not possible. Target %s is not a LABEL\n", label);
        addErrorMessage(
            scannerContext->ppp,
            sprintfString(
                "Preprocessor Error at (%s): \%GOTO not possible. Target %s is not a LABEL\n",
                sprintf_yylloc(locp), label));
        return 0;
      }

    return 1;
  }

/**
 * Handle two different format of the %INCLUDE statement
 *   %INCLUDE filename
 *   %INCLUDE ddname ( filename )
 * 
 * Note, The notion of DDNAME is implemented as an environment variable. 
 *       This can then contain the list of places to search.
 * 
 * The search for the include file name is done in IncludeFileUtils.c.
 * 
 * Returns: 0: An error happend during search for the file.
 *          1: The request file was found.
 * 
 * Output structures (side effects)
 * The IncludeFileUtils.c uses the scannerContext->ppreturn->string,
 * it contains the fully qualified filename.
 * 
 */
int ppSetIncludeStmt(char *fileName, char *environmentVariable, YYLTYPE *locp)
  {
    int isOK;

    if (environmentVariable==NULL)
      isOK=findFile(fileName);
    else
      isOK=findEnvFile(environmentVariable, fileName);

    if (!isOK)
      { /* TODO: move all the setting of scannerContext and error message to IncludeFileUtils.c */
        scannerContext->ppreturn->string=strdup(fileName);
        scannerContext->ppreturn->type=LIT_PREPROCESSOR_ERROR;

        if (environmentVariable==NULL)
          {
            debugParserpp("\%INCLUDE failed for filename %s\n", fileName);
            addErrorMessage(
                scannerContext->ppp,
                sprintfString(
                    "Preprocessor Error at (%s): \%INCLUDE failed for filename %s\n",
                    sprintf_yylloc(locp), fileName));
          }
        else
          {
            debugParserpp(
                "\%INCLUDE failed for environment variable %s for filename %s\n",
                environmentVariable, fileName);
            addErrorMessage(
                scannerContext->ppp,
                sprintfString(
                    "Preprocessor Error at (%s): \%INCLUDE failed for environment variable %s for filename %s\n",
                    sprintf_yylloc(locp), environmentVariable, fileName));
          }

      }

    memcpy(scannerContext->ppreturn->location, locp, sizeof(YYLTYPE));

    return isOK;
  }

/**
 * 
 * Disposes the allocated memory for a struct ppDCLelement
 * 
 */
int disposeDCLElement(struct ppDCLelement *w)
  {
    if (w)
      {
        if (w->name)
          free(w->name);
        if (w->location)
          free(w->location);
        free(w);
      }

    return 0;
  }

/** 
 * Add new ppDCLLabelStack at the end of list 
 * 
 */
struct ppDCLLabelStack * ppPushDCLLabelStack(struct ppDCLLabelStack *tail)
  {
    struct ppDCLLabelStack *work;

    work=malloc(sizeof(struct ppDCLLabelStack));
    if (work==NULL)
      error("internal error allocating ppDCLLabelStack");

    work->ppDCLLabelList=NULL;
    work->prev=NULL;

    /* 
     * if tail is allocated, update the prev pointer of the allocated work
     * to the value of tail
     */
    if (tail)
      work->prev=tail;

    /*
     * update the tail point to the newly allocated element.
     */
    tail=work;

    return work;
  }

/**
 * Remove the last element on the list, and change tail pointer 
 * accordingly.
 * 
 */
struct ppDCLLabelStack *ppPopDCLLabelStack(struct ppDCLLabelStack *tail)
  {
    struct ppDCLLabelStack *work;
    work=tail;
    tail=work->prev;
    ppDisposeDCLLabelStackElement(work);
    free(work);

    return tail;
  }

/**
 * Free the associated storage for a ppDCLLabelStack element.
 * Note, before calling the dispose function, the pointer to
 * the previous record should be saved.
 * 
 */
int ppDisposeDCLLabelStackElement(struct ppDCLLabelStack *work)
  {
    if (work)
      { /* dispose the allocated DCLElements */
        disposeDCLElement(work->ppDCLLabelList);
        work->ppDCLLabelList=NULL;
        work->prev=NULL;
      }

    return 0;
  }

/**
 *  Handle the preprocessor %NOTE statement 
 * 
 *  Parameters:
 * 		level: I: info
 *             W: warn
 *             E,S: error
 *  	msgtext: text to be written
 * 
 *  Returns 0: preprocessing should stop with error
 *          1: preprocessing can continue
 * 
 */
int ppSetNoteStmt(char *level, char *msgtext)
  {
    int rc;
    switch (*level)
      {
      case 'I':
      case 'i':
        addInfoMessage(scannerContext->ppp, msgtext);
        rc=1;
        break;
      case 'W':
      case 'w':
        addWarnMessage(scannerContext->ppp, msgtext);
        rc=1;
        break;
      case 'E':
      case 'e':
      case 'S':
      case 's':
      default:
        addErrorMessage(scannerContext->ppp, msgtext);
        rc=0;
        break;
      }

    return rc;
  }

/**
 * 
 * Set the struct ppDCLelemet->active to LIT_DCL_ACTIVE
 * If the ->active is already active a warning message is written. 
 * 
 * Parameters: name of variable
 * 
 * Returns 0: Error activating the variable
 *         1: The variable has been activated.
 * 
 */
int ppSetActivateStmt(char *name)
  {
    struct ppDCLelement *w;
    int isOK;

    isOK=1;
    debugParserpp("$$$ ppSetActiveStmt for variable %s \n", name);
    w=ppFindDCLName(name);
    if (!w)
      {
        addErrorMessage(scannerContext->ppp, sprintfString(
            "\%Preprocessor variable not found %s\n", name));
        isOK=0;
      }
    else if (w->active==LIT_DCL_ACTIVE)
      {
        addWarnMessage(scannerContext->ppp, sprintfString(
            "\%Preprocessor variable is already active %s\n", name));

      }
    else
      w->active=LIT_DCL_ACTIVE;

    return isOK;
  }

/**
 * 
 * Set the struct ppDCLelemet->active to LIT_DCL_INACTIVE
 * If the ->active is already inactive a warning message is written. 
 *
 * Parameters: name of variable
 * 
 * Returns 0: Error deactivating the variable
 *         1: The variable has been deactivated.
 * 
 */
int ppSetDeactivateStmt(char *name)
  {
    struct ppDCLelement *w;
    int isOK;

    isOK=1;
    debugParserpp("$$$ ppSetDeactivateStmt for variable %s \n", name);
    w=ppFindDCLName(name);
    if (!w)
      {
        addErrorMessage(scannerContext->ppp, sprintfString(
            "\%Preprocessor variable not found %s\n", name));
        isOK=0;
      }
    else if (w->active==LIT_DCL_INACTIVE)
      {
        addWarnMessage(scannerContext->ppp, sprintfString(
            "\%Preprocessor variable is already deactivated %s\n", name));

      }
    else
      w->active=LIT_DCL_INACTIVE;

    return isOK;
  }

/**
 * ppAnalyseStmt is invoked from the scanner, and is used to detect which tokens to
 * pass the the parser.
 *  
 * returns 0: statement is not a preprocessor label statement
 *         1: statement is a preprocessor label statement and the label matches
 *         2: statement is a preprocessor label statement but the label doesnt match
 */
int ppAnalyseStmt(char *labelName, struct anyTokenList *inTokensList,
    struct anyTokenList *outTokensList)
  {
    /*
     * This function is called when the scanner is in a special mode: preprocess discarding.
     * The preprocess discarding is used when eg. a %GOTO is invoked and the label has not
     * been met. Until the label is met, all the tokens will be discarded.
     * 
     * accept the following type of preprocessor statements
     *   % name : name : ppstmt ;
     * more formally: the % can be followed by one or more 'name :', the labellist. 
     * After the labellist, a preprocessore statement and finally a semicolon at the end.
     * 
     * If the label name is met, return 1.
     * If the statement is a preprocessor statement with a label then transform the statement into
     *   % name : name : ;
     * and return 2. This will allow the preprocessor to be invoked, and hence the found labels will
     * be added to the list of labels in the preprocessor program. This is a way to ensure the 
     * ping-pong-pang example will work without errors:
     * 	%goto ping ;
     *	%pong: goto pang;
     *	%ping: goto pong;
     *	%pang:;
     * 
     * without any special processing, the text '%pong: goto pang;' would be discarded.
     * 
     */

    struct savedtokenlist *savedtemp, *savedtemp2;
    struct savedtokenlist *inPctToken;

    int xLabelFound, xColonFound, xPctFound, xSkipCount;

    /* find '%' */
    for (xPctFound=0, xSkipCount=0, savedtemp=inTokensList->head; savedtemp;
        savedtemp=savedtemp->next, xSkipCount++)
      {
        if (savedtemp->token=='%')
          {
            xPctFound=1;
            /* store the position of the '%', in the hope that this 
             * infact is the beginning of a preprocessor statement. 
             */
            savedtemp2=malloc(sizeof(struct savedtokenlist));
            memcpy(savedtemp2, savedtemp, sizeof(struct savedtokenlist));
            savedtemp2->next=NULL;
            inPctToken=savedtemp;
            outTokensList->head=savedtemp2;
            outTokensList->count=1;
            break;
          }
      }
    if (!xPctFound)
      goto out_zero;

    /* now match the following sequence of tokens
     *  
     * '%' any ( ',' any )* ':' ppStmt ';'
     *  
     */
    for (xLabelFound=0, xColonFound=0, savedtemp=savedtemp->next /* skip '%' */
    ; savedtemp; savedtemp=savedtemp->next)
      { /* create a copy of the token */
        savedtemp2->next=malloc(sizeof(struct savedtokenlist));
        memcpy(savedtemp2->next, savedtemp, sizeof(struct savedtokenlist));
        savedtemp2=savedtemp2->next;
        savedtemp2->next=NULL;
        outTokensList->count++;

        if (savedtemp->token==',') /* NOP */
          ;
        else if (savedtemp->token==':')
          {
            xColonFound=1;
            break;
          }
        else if (strcasecmp(savedtemp->string, labelName)==0)
          { /* label found, now continue looking at tokens to ensure the ':' is found as well */
            xLabelFound=1;
          }
      }

    if (!xColonFound)
      goto out_zero;
    if (xLabelFound)
      { /* TODO: discard copied tokens */
        /* set tokens to the original list of tokens. */

        outTokensList->head=inPctToken;
        outTokensList->count=inTokensList->count-xSkipCount;

        goto out_one;
      }

    /* 
     * The statement is a preprocessor label statement, but the label does not match,
     * so adjust the token list so the first token following the ':' is ';'
     * The reason is just to define the %label. The actual statement associated
     * with the %label will first be parsed when the %label is activated. 
     * 
     * Just to illustrate what is going on.
     *                     seq desc
     *    %goto x;          01 scan for %label x
     *    %y: dcl k fixed;  02 add %label y, insert ';' after ':', ignore %dcl k
     *                      05 ignore %label and define k
     *    %goto z;          06 scan for %label z
     *    %x: dcl j fixed;  03 %x found, dcl j and continue
     *    %goto y;          04 goto location of y
     *    %z:;              07 ok, found %z, continuing
     * 
     */

    savedtemp=savedtemp->next;
    savedtemp2->next=malloc(sizeof(struct savedtokenlist));
    memcpy(savedtemp2->next, savedtemp, sizeof(struct savedtokenlist));
    savedtemp2=savedtemp2->next;
    savedtemp2->next=NULL;
    outTokensList->count++;

    if (savedtemp->token!=';')
      {
        savedtemp2->token=';';
      }

    goto out_two;

    /* statement is not a preprocessor label statement */
    out_zero: return 0;
    /* statement is sought preprcessor label statement */
    out_one: return 1;
    /* statement is a preprocessor label statement */
    out_two: return 2;
  }

/**
 * Allocate preprocessor context block
 * Also allocate a new labelstack
 */
struct ppContextAnchor *newppContextAnchor(void)
  {
    struct ppContextAnchor *temp;
    /* allocate current context block */
    debugLexer("newppContextAnchor: Allocating preprocessor context block.\n");
    temp=malloc(sizeof(struct ppContextAnchor));
    if (!temp)
      {
        error("out of mem. newppContextAnchor");
        return NULL;
      }
    temp->ppDCLhead=NULL;
    temp->ppDCLtail=NULL;
    temp->ppreturn=NULL;
    temp->ppDCLLabelTail=NULL;

    /* allocate a new label stack */
    temp->ppDCLLabelTail=ppPushDCLLabelStack(temp->ppDCLLabelTail);

    /* allocate new return area */
    temp->ppreturn=malloc(sizeof(struct ppReturn));
    temp->ppreturn->location=malloc(sizeof(YYLTYPE));
    if (!temp->ppreturn)
      {
        error("out of mem. newppContextAnchor");
        return NULL;
      }

    temp->ppp=NULL;
    temp->ppGatherTokens=0;
    temp->ppScanForLabel=NULL;

    temp->dolevel = 0; /* %DO .. %END nesting level */
    temp->loopVarname=NULL;
    temp->loopStartValue=-1;
    temp->loopEndValue=-1;

    return temp;
  }

int ppSetDoStmt(char *loopvarname, int fromexpr, int toexpr, YYLTYPE *locp)
  {
    char *buf;
    /* */
    debugParserpp(
        "$$$ ppSetDoStmt level %i for loop variable %s, from %i, to %i\n",
        scannerContext->dolevel, loopvarname, fromexpr, toexpr);
    /* */
    scannerContext->ppreturn->type=LIT_PREPROCESSOR_LOOP_DO;
    memcpy(scannerContext->ppreturn->location, locp, sizeof(YYLTYPE));

    if (!loopvarname)
      {
        buf
            =sprintfString(
                "Preprocessor Error at (%s): ppVariable.c: ppSeDoStmt: Internal error, loopvarname %s is not allocated",
                sprintf_yylloc(locp), loopvarname);
        addErrorMessage(scannerContext->ppp, buf);
        goto out_error;
      }

    if (scannerContext->dolevel>0)
      {
        buf
            =sprintfString(
                "Preprocessor Error at (%s): ppVariable.c: ppSeDoStmt: Nested looping not yet supported.",
                sprintf_yylloc(locp));
        addErrorMessage(scannerContext->ppp, buf);
        goto out_error;
      }

    /* set loop variable to initial value */
    if (!ppSetIntValue(loopvarname, fromexpr, locp))
      goto out_error;

    scannerContext->dolevel++;
    scannerContext->loopVarname = loopvarname;
    scannerContext->loopStartValue = fromexpr;
    scannerContext->loopEndValue = toexpr;

    /*out_ok: ; */
    return 0;

    out_error: scannerContext->ppreturn->string=strdup(loopvarname);
    scannerContext->ppreturn->type=LIT_PREPROCESSOR_ERROR;

    return -1;

  }

int ppSetEndStmt(YYLTYPE *llocp)
  {
    debugParserpp("$$$ ppSetEndStmt. Level %i ", scannerContext->dolevel);
    scannerContext->ppreturn->type=LIT_PREPROCESSOR_LOOP_END;
    scannerContext->dolevel--;
    memcpy(scannerContext->ppreturn->location, llocp, sizeof(YYLTYPE));
    return 0;
  }
