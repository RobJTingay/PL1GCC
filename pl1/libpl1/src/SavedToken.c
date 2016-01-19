    /* SavedToken.c
     * ============
     *
     * Functions to maintain lists of saved tokens.
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

/* SavedToken
 * ==========
 * 
 * Two structs are implemented.
 * The anyTokenList, which can be seen as a named list of
 * the structure savedtokenlist.
 * 
 * The scanner uses two disctint list of tokens, one for the
 * token based lookahead, and one for the preprocessor tokens.
 * 
 * Still TODO
 * - implement a flag indicating is a saved token can be disposed or not
 * - rename savedtokenlist into something relevant, eg. savedtokenelement.
 * 
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "PPDebugLog.h"
#include "TokenLocationStructs.h"
#include "TokenLocation.h"
#include "SavedTokenStructs.h"

extern int error(const char *msgtext); //TODO: proper include for error function.


/* ============================= */
/* savedTokenList
 * savetoken(struct anyTokenList list, int token, YYLTYPE *loctype, char *string)
 * showTokenList(struct anyTokenList *list,const char *text)
 * newSaveTokenList(char * name)
 * 
 */

/* prototypes */
int savetoken(struct anyTokenList *list, int token, YYLTYPE *loctype, char *string);
void showTokenList(struct anyTokenList *list);
struct anyTokenList *newSaveTokenList(const char * name);
int freeAnyTokenList(struct anyTokenList *list);
int emptyAnyTokenList(struct anyTokenList *list);
int appendAnyTokenList(struct anyTokenList *target, struct anyTokenList *source);
char *tokenListAsString(struct anyTokenList *inTokensList );
char *savedtokenListStmtAsString(struct savedtokenlist *inTokensList );

/* ========================== */
/* returns zero if error saving token in list */
int savetoken(struct anyTokenList *list, int token, YYLTYPE *loctype, char *string) {
 
  /*
    NOTE: we have to keep the preprocessor directive tokens
    in a separate list, since the look ahead tokens are the
    potentially the _result_ of a preprocessor directive. 
    This means we have to have a separate list so that the 
    the look ahead mechanism doesn't see anymore tokens until
    the preprocessor is done.
  */

    struct savedtokenlist *temp;

    if (list->head==NULL) /* first entry in list */
    { list->head=malloc(sizeof(struct savedtokenlist));
      if (list->head==NULL) {error("out of mem");return 0;}
      list->tail=list->head;
      list->count=0;
    }
    else
    { temp=malloc(sizeof(struct savedtokenlist));
      if (temp==NULL) {error("out of mem");return 0;}
      list->tail->next=temp;
      list->tail=temp;
    }

    /* ppcurrent now points the the entry to be filled */

    list->tail->locations=malloc(sizeof(YYLTYPE));
    if (list->tail->locations==NULL) {error("out of mem");return 0;}
    list->tail->string=malloc(strlen(string)+1);
    if (list->tail->string==NULL) {error("out of mem");return 0;}

    list->tail->next=NULL;
    list->tail->token=token;
    memcpy(list->tail->locations,loctype,sizeof(YYLTYPE));
    strcpy(list->tail->string,string);
    list->count++;

    debugVerbose("%s",sprintf_yylloc(list->tail->locations));
    debugLexer("\tSaving Token[%i]:%i\t>%s<\n",
               list->count,
               list->tail->token,
	           list->tail->string);

    return list->count;

} /* end savetoken */

void showTokenList(struct anyTokenList *list)
{   struct savedtokenlist *temp; 
	int i;
	
	if(isDebugLevel(DL_LEXER)) {
      debugLexer("%s: number of collected tokens %i\n",list->name,list->count);

      for(i=0,temp=list->head;
          i<list->count;
          i++,temp=temp->next) {	
        debugVerbose("%s",sprintf_yylloc(temp->locations));
        debugLexer("%s: token[%i]=%i, text=%s\n",
                   list->name,
                   i,
                   temp->token,
                   temp->string);
      }
    }
}

struct anyTokenList *newSaveTokenList(const char * name)
{   struct anyTokenList *temp=malloc(sizeof(struct anyTokenList));
	debugLexer("allocate tokenlist %s\n",name);
	if(!temp) {error("out of mem");return NULL;}
	temp->head=NULL;
	temp->tail=NULL;
	temp->count=0;
	temp->name=name;
	return temp;
}

int freeAnyTokenList(struct anyTokenList *list)
{
  debugLexer("freeing tokenlist named %s\n",list->name);

  emptyAnyTokenList(list);
  /* 
   * Note, since list->name is "const char *", the free does work 
   * 
   * if(list->name) free(list->name);
   */
  list->name=NULL;
  
  return 0;	
}

int emptyAnyTokenList(struct anyTokenList *list)
{
   struct savedtokenlist *temp, *work; 
   debugLexer("emptying tokenlist named %s, count %i\n",list->name,list->count);
   /* traverse the list, and free the individual tokens */
   /* note, find way to keep tokens that should not be freed, 
    * fx, name of preprocessor constants.
    */
   int i;
  for(i=0,work=list->head;work;i++)
  { debugVerbose("disposing token %i: %s\n",work->token,work->string);
        temp=work;
        work=work->next;
        // TODO: if(temp->disposable)
        // TODO if(temp->locations) free(temp->locations) ;
        // TODO if(temp->string) free(temp->string);
        free(temp);
  }
  list->head=NULL;
  list->tail=NULL;
  list->count=0;
  /* Keep name of the list */
  
   return 0;    
}


/*
 * Appends two token list to each other.
 * 
 * 
 */
int appendAnyTokenList(struct anyTokenList *target, struct anyTokenList *source)
{
   struct savedtokenlist *work; 
   debugLexer("appending tokenlist named %s to %s\n",source->name,target->name);


   /* traverse the list, and free the individual tokens */
   /* note, find way to keep tokens that should not be freed, 
    * fx, name of preprocessor constants.
    */
  for(work=source->head;work;work=work->next)
  { savetoken(target,work->token,work->locations,work->string);
  }
  
   return 0;    
}


/* 
 * Convert list of tokens into a string
 * 
struct savedtokenlist 
{ 
  int     token;
  YYLTYPE *locations;
  char    *string;
  struct  savedtokenlist *next;
};

struct anyTokenList
{ struct savedtokenlist *head;
  struct savedtokenlist *tail;
  int count;
  const char *name ;
};
 * 
 */
char *tokenListAsString(struct anyTokenList *inTokensList )
{ struct savedtokenlist *work;
  char *ret;
  char *workcat;
  int length; 

  char *stpcpy(char *, const char *);
  
  /* Estimate the max length of the string to allocate */
  length=1;
  for(work=inTokensList->head;work;work=work->next)
  { length+=strlen(work->string);
  }
  /* Add one for each token, for the separator space */
  length+=inTokensList->count;
  
  ret=malloc(length);
  if(ret==NULL) error("tokenListAsString:out of mem.");
  
  /* concatenate the tokens into a string */
  *ret='\0';
  for(workcat=ret,work=inTokensList->head;work;work=work->next)
  { workcat=stpcpy(workcat,work->string);
        workcat=stpcpy(workcat," ");
  }
  
  return ret;   
}


/* 
 * Convert list of tokens into a string
 * Stop converting when the first ';' is met.
 * Useful when a list of tokens should be used as a string for just the statement.
 * Note, the token pointer is updated to be the last token in the list
 * 
struct savedtokenlist 
{ 
  int     token;
  YYLTYPE *locations;
  char    *string;
  struct  savedtokenlist *next;
};

struct anyTokenList
{ struct savedtokenlist *head;
  struct savedtokenlist *tail;
  int count;
  const char *name ;
};
 * 
 */

#define STR_CONSTANT 0x0310  /* defined in pl1-parser.y */
char *savedtokenListStmtAsString(struct savedtokenlist *inTokensList )
{ struct savedtokenlist *work;
  char *ret;
  char *workcat;
  int length; 

  char *stpcpy(char *, const char *);
  
  /* Estimate the max length of the string to allocate */
  length=1;
  for(work=inTokensList;work;work=work->next,length++)
  { length+=strlen(work->string);
    if(work->token==STR_CONSTANT) length+=2;
    if (*work->string==';') break;
  }
  
  /* Add one for each token, for the separator space */
  
  
  ret=malloc(length);
  if(ret==NULL) error("tokenListAsString:out of mem.");
  // TODO: check if token is a string constant, then readd the quotes
  /* concatenate the tokens into a string */
  *ret='\0';
  for(workcat=ret,work=inTokensList;work;work=work->next)
  { if(work->token==STR_CONSTANT)
    workcat=stpcpy(workcat,q_requote_str(work->string));
    else
    workcat=stpcpy(workcat,work->string);
  
    if (*work->string==';') break;
    
    workcat=stpcpy(workcat," ");
  }
  
  return ret;   
}
