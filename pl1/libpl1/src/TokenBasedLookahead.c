    /* TokenBasedLookahead.c
     * =====================
     *
     * Functions to maintain the token based look ahead.
     *
     * Copyright 2006 Henrik Sorensen
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

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "PPDebugLog.h"
#include "TokenLocationStructs.h"
#include "SavedTokenStructs.h"
#include "pl1.tab.h"


/* =================================== */
/* Lookahead ...
 * checkstmt(struct savedtokenlist *head)
 * enable_lookahead( int prev_token, int token )
 * fixKeywordTokens(struct savedtokenlist *head, int prev_token)
 */

/* prototypes */
int checkstmt(struct savedtokenlist *head, int prev_token);
int enable_lookahead( int prev_token, int token );
void fixKeywordTokens(struct savedtokenlist *head, int prev_token);
/*
   checkstmt - Check if statement is assignment or keyword statement.
   Returns 0: Not statement
	   1: Is statement
   Remember the whole statement until the terminating ';' has been
   read by flex, and unless there is a syntax error, the statement
   should be complete, and furthermore, the first token will be one
   of the trouble keywords: DCL, FORMAT, DEFAULT.
*/
int checkstmt(struct savedtokenlist *head, int prev_token)
{ /* Generally solve
	kw:kw:kw-stmt;
	one problem, at the first kw, the lookahead is enabled, and
	at the second kw,
  */
  struct savedtokenlist *temp;
  int plvl,t;

  debugLexer("Lookahead checking Head->token:%i...",head->token);

  if(
      head->token == DECLARE
   || head->token == DEFAULT
   || head->token == END
   || head->token == ENTRY
   || head->token == ENTRY_STMT
   || head->token == FORMAT_STMT
   || head->token == PROCEDURE
   )
   { /*
       {precond} - Must be true
	  must be preceeded by ';' or ':'
       {rule 0} - '='
	  if the token following the keyword and an optional matching set
	  of parentisises, is '=', then the keyword a VARNAME, otherwise
	  just assume the keyword is a statement.
     */

    plvl=0;
    for(temp=head->next;temp;temp=temp->next)
    { switch(temp->token)
      { case '(': plvl++; break;
        case ')': plvl--; break;
        default: break;
      } /* switch token */
      if((temp->token=='='||temp->token==':')&&plvl==0) break;
    } /* all tokens */
    if(temp)
    {
    if((temp->token=='='||temp->token==':')&&plvl==0)
    { /* kw is an identifier */
      goto out_id;
    }
    }

   } /* end if declare,default,end,entry,entry_stmt,format_stmt,procedure */
   else 
   if
   ( head->token== OTHERWISE
   ||head->token== SNAP
   )
   { 
   	if (prev_token==CALL) goto out_id;
    
    plvl=0; t=0;
    for(temp=head->next;temp;temp=temp->next)
    { t++;
      switch(temp->token)
      { case '(': plvl++; break;
        case ')': plvl--; break;
        default: if (plvl==0) break;
      } /* switch token */
      if(plvl==0) break;
    } /* all tokens */
    if(temp)
    {
    if((plvl==0&&t>1)  /* if more than one token follows until plvl is zero */
     ||(plvl==0&&t==1&&temp->token=='=') /* if first token is '=' */
     )
    { /* kw is an identifier */
	goto out_id;
    }
    }
   } /* end if otherwise,snap */
   else
   { printf("... checkstmt: token %i not matched ... should not happend!\n",head->token);
   }
  
  
goto out_stmt; 
  
  out_id:

    debugLexer("is an identifier\n");

    return 0;
   
  out_stmt:

    debugLexer("is a keyword statement\n");

    return 1;
    
} /* checkstmt */

int enable_lookahead( int prev_token, int token )
{	
	if
	(	token == DECLARE
	||	token == DEFAULT
	)
	{	if( prev_token == ';'
		 || prev_token == ':'
		 || prev_token == -1 
		  )  goto out_true;
		else goto out_false;
	}
	else 
	if
	(	token == ENTRY
	||	token == FORMAT
	||	token == PROCEDURE
	)
	{	if(	prev_token == ':'
		||	prev_token == -1 
		   ) goto out_true;
		else goto out_false;
	}
	else
	if
	(token == OTHERWISE)
	{	    if (prev_token!=';' 
		     || prev_token=='.' 
		     || prev_token==-1
		       ) goto out_false;
		    else goto out_true;
	}
	else
	if
	(token == SNAP)
	{		if (prev_token=='.' 
		     || prev_token==-1
		       ) goto out_false;
		    else goto out_true;
	}
	else 
	{
		debugLexer("Scanner: enable_lookahead fall through for token %d\n",token);
		goto out_false;
	}

	out_false:
	return 0;
	
	out_true:
	return 1;
}

void fixKeywordTokens(struct savedtokenlist *head, int prev_token)
{ /* 
    check if the first token is kw or identifier
    and set the token accordingly.
   */
			    
   if (checkstmt(head,prev_token)) 
   { 
       /* is the statement */
        if(head->token==ENTRY)
            head->token=ENTRY_STMT; 
        else if(head->token==FORMAT)
            head->token=FORMAT_STMT; 
   } 
   else 
   { 
		/* check if token should be changed to VARNAME */
         if(head->token==END
         || head->token==ENTRY
         || head->token==FORMAT
         ) /* nothing */ ;
         else if(head->token==ENTRY_STMT)
	             head->token= ENTRY; 
	     else if(head->token==FORMAT_STMT)
	             head->token= FORMAT; 
	     else    head->token= VARNAME;
    }
}
