    /* parseScopeNode.c
     * ================
     *
     * Implementing the datastructure for the scope Node.
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
/*
 * ScopeNode
 * ---------
 * 
 * Scope node is a bi-directional tree structure. Bi-directional in the sense
 * that scopes on the same level are handled a linked list (->next), and the
 * scopes within the current scope is maintained with a linked list (->head,->tail).
 * The containing scope node can be reached via ->parent.
 * 
 * When the parsing starts the scopeRoot variable is allocated and set to an 
 * empty ScopeNode. The scopeRoot contains all the scopes in the program.
 * This means that when '*process;' is met a new compilation unit starts, 
 * but it will share the same scopeRoot. 
 * 
 * Looking ahead....
 * Another usage of the scopeRoot, is to 'attach' all the built-in functions 
 * and defaults at this very first level. Then all other scopes will inherit 
 * the definitions from the root level. This would even make user defined
 * compiler defaults very easy to implement.
 *
 * 
 * For example:
 * 
 * a0: proc options(main);
 *   a0_1: entry;
 *   b1: proc;
 *      c2:proc;
 *      end c2;
 *      d2: proc;
 *      end d2;
 *   end b1;
 * end a0; 
 * 
 *               tail,
 *  -----------  head  --------------  next
 * ! scopeRoot ! ---> ! ScopeNode A0 ! ---> null
 *  -----------        --------------
 *                           ! 
 *                           ! head,tail
 *                           V
 *                     --------------
 *                    ! ScopeNode B1 !
 *                     --------------
 *                           ! 
 *                           ! head . . . . . . . . tail
 *                           V                      V
 *                     --------------  next  --------------  next
 *                    ! ScopeNode C2 ! ---> ! ScopeNode D2 ! ---> null
 *                     --------------        --------------
 *                           !                      !
 *                           ! head,tail            ! head,tail
 *                           V                      V
 *                         null                   null
 * 
 * Traversing the tree
 * -------------------
 * 
 * The recommended way to traverse the tree width first:
 * 	 traverseScopeTree(scopeRoot);
 * 
 * and use this as a skeleton for the traverse function 
 *    void traverseScopeTree(struct ScopeNode *current)
 *		{ struct ScopeNode *work;
 *		  /-* width first *-/
 *		  for(work=current;work;work=work->next)
 *		  { processNode(work);
 *		  }
 *		  /-* now recurse into the sub tree for the children for each node. *-/
 *		  for(work=current;work;work=work->next)
 *		  { traverseScopeTree(work->head);
 *		  }
 *		}
 * 
 * 
 * 
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "PPDebugLog.h"
#include "parseMisc.h"
#include "parseParserStackStructs.h"
#include "parseScopeNodeStructs.h"

#include "pl1.tab.h"
#include "yyFlex.h"

#include "parseDclStmt.h"

extern int error(const char *msgtext); // TODO: Fix error

/* prototypes */
struct ScopeNode *newScope(struct ScopeNode *c,struct ListOfNames *l,struct ParserStack *ps);
struct ScopeNode *newScopeRoot(void);
struct ScopeNode *endScope(struct ScopeNode *c,struct ParserStack *ps,struct ScopeNode *scopeRoot);
struct ScopeNode *initScopeNode(void);
void   showScopeTree(struct ScopeNode *sn);
void   showScopeTreeWhole(char *,struct ScopeNode *sn);
struct ScopeNode *lookUpMatchingNode(struct ScopeNode *s, char *n);

int checkNameInLabellist(struct ListOfNames *, char *);


extern char *pl1parserGetTokenText(int token); // defined in pl1-parser.y
/* prototypes from pl1-parser.y*/
extern struct ListOfNames *newListOfNames(char *);
extern char *allocFullLabelName(struct ListOfNames *l);
extern struct ScopeNode *lookUpMatchingNode(struct ScopeNode *s, char *n);
extern void yyerror (YYLTYPE *locp,
             yyscan_t scanner, 
             const char *msg);

/*
 * newScope
 * --------
 * 
 * Allocates a new ScopeNode, and add it to the tail of the scope pointed by 'current'.
 * 
 * Returns
 *   ScopeNode *: newly allocated scope node.
 * 
 * Parameters
 *   ScopeNode *current
 *   ListOfNames *scopeName
 *   int scopeType
 * 
 * 
 */
struct ScopeNode *newScope(struct ScopeNode *current,struct ListOfNames *scopeName,struct ParserStack *ps)
{ struct ScopeNode *work;
  int scopeType;

  work=initScopeNode();
  
  scopeType=ps->i;

  if (scopeName==NULL)
  { switch(scopeType)
    { case _ROOT_: work->name=newListOfNames((char*)"ROOT"); break;
      default: work->name=newListOfNames((char *)"Unnamed"); break;
    }
  }
  else
  work->name=scopeName;

  work->stmttype=scopeType;
  work->parent=current; /* store current as parent of the new ScopeNode*/

  debugParser("NewScope: %s, type (%i) %s\n",
	      allocFullLabelName(work->name),
	      scopeType,
	      pl1parserGetTokenText(scopeType));

  if (current!=NULL) /* not the root scope, so update parents child pointers */
  { if (current->head==NULL) /* this is the first child */
    { current->head=work;
      current->tail=work;
    }
    else /* not the first child */
    { current->tail->next=work;
      current->tail=work;
    }
  } /* not root scope: just return tn */
    return work;
} /* newScope */

struct ScopeNode *newScopeRoot(void)
{ struct ScopeNode *work;

  work=initScopeNode();
  work->name=newListOfNames((char*)"ROOT");
  work->stmttype=_ROOT_;

  debugParser("NewScopeRoot: %s, type (%i) %s\n",
	      allocFullLabelName(work->name),
	      _ROOT_,
	      pl1parserGetTokenText(_ROOT_));

    return work;
} /* newScopeRoot */

struct ScopeNode *initScopeNode(void)
{ struct ScopeNode *work;

  work=malloc(sizeof(struct ScopeNode));
  if (work==NULL) error("Internal compiler error: newScope out of memory.");
  
  work->head=NULL;
  work->tail=NULL;
  work->next=NULL;
  work->dcl=NULL;
  work->name=NULL;
  work->stmttype=-1;
  work->parent=NULL; /* store current as parent of the new ScopeNode*/

  debugParser("initScopeNode:\n");

    return work;
} /* initScopeRoot */

/* endScope
 * --------
 * 
 * Return the scope that is either the parent fof the current scope 
 * or if the END is named, the parent of the defining scope.
 * 
 * Returns
 *   ScopeNode *: parent scope of the enclosing end scope.
 * 
 * Parameters
 *    ScopeNode *current
 * 
 * 
 */
struct ScopeNode *endScope(struct ScopeNode *current,struct ParserStack *ps,struct ScopeNode *scopeRoot)
{ struct ScopeNode *fs;
  char *endlabel;
  if (current)
  { 
  	if (ps->l) /* this is a named END */
    { endlabel=ps->l->head->name;
      fs=lookUpMatchingNode(current,endlabel);
      if(fs==NULL)
      { 
	debugParser("pl1gcc: Named END %s, was not found.", endlabel);

        yyerror(NULL,NULL,"pl1gcc:Named END not found."); /* TODO: fixme use yyerror */
        /* TODO: ErrorMsg: named end not found */
        return scopeRoot;
      }
      else return fs->parent; /* return the parent scope of the defining block for the named END */
    }
    else
    { return current->parent;
    }
  }
  else
  { return current;
  }
} /* endScope */

/*
 * showScopeTree
 * -------------
 * 
 * Traverse the scope tree, via call to showScopeTreeWhole.
 * 
 * Returns
 *   nothing
 * 
 * Parameters
 *   ScopeNode *sn: 
 * 
 */
void showScopeTree(struct ScopeNode *sn)
{ /* show whole tree from the sn node and onwards */
  char *n1;

  n1=allocFullLabelName(sn->name);
  showScopeTreeWhole(n1,sn->head);

  free(n1); /* n1 is allocated by allocFullLabelName */
} /* showScopeTree */


/*
 * showScopeTreeWhole
 * ------------------
 * 
 * Recursively traverse the scope tree.
 * 
 * Parameters:
 * 
 *   char *name
 * 
 * 
 */

void showScopeTreeWhole(char *n, struct ScopeNode *sn) {
 
  /* show node, and all siblings */
  /* Then for all siblings,  recursively call showScopeTree for the head */

  struct ScopeNode *s;
  struct DeclareElement *work;
  char *n2,*n3;

  if(!isDebugLevel(DL_PARSER)) {
    return;
  }

  for (s=sn; s; s=s->next) {

    n2=allocFullLabelName(s->name);

    debugParser("\tname    :%s-%s_%s",n,n,n2);

    debugParser("\tstmttype:(%i) %s\n",
		s->stmttype,
		pl1parserGetTokenText(s->stmttype));
		
	/* show the list of declares */
	if(sn->dcl)
	{
  	  for(work=sn
  	  ->dcl
  	  ->head
  	  ;work
  	  ;work=work->next)
	  {
	    showDeclareElement(work);
	  }
	}

    free(n2); /* n2 is allocated by allocFullLabelName */
  }

  debugParser("===============");

  /* for all the siblings, recursively call showScopeTree */

  for (s=sn; s; s=s->next)
  { n2=allocFullLabelName(s->name);
    if (s->head)
    { n3=malloc(strlen(n)+strlen(n2)+2);
      if (n3==NULL) error("Internal compiler error: showScopeTreeWhole: Out of memory.");
      strcpy(n3,n);
      strcat(n3,"_");
      strcat(n3,n2);
      showScopeTreeWhole(n3,s->head);
      free(n3);
    }
    free(n2);
  }
} /* showScopeTreeWhole */


/*
 * looUpMatchingNode
 * -----------------
 * 
 * Looking through the list of parents for a scope node with the searched name.
 * 
 * Parameters
 *   ScopeNode *s: scope to start the search from
 *   char *n: name to look for
 * 
 */

struct ScopeNode *lookUpMatchingNode(struct ScopeNode *s, char *n) { 

  struct ScopeNode *sn;
  int cnt=0;

  for(sn=s;sn;sn=sn->parent) { 

    cnt++;

    if(checkNameInLabellist(sn->name,n)) {
      break;
    }
  }

  debugParser("*** Lookup returns count %i, node:%s\n",
	      cnt,
	      allocFullLabelName(sn->name));

  return sn;

} /* lookUpMatchingNode */

/*
 * checkNameInLabelList
 * --------------------
 * 
 * Look for a name in a label list.
 * 
 * Parameters
 *    ListOfNames l: label list to search.
 *    char *n: name to look for.
 * 
 * 
 */

int checkNameInLabellist(struct ListOfNames *l, char *n) {
 
  struct NameNode *ln;
  int found;
  found=0;

  debugParser("*** Looking for >%s<, in >%s<: ",
	      n,
	      allocFullLabelName(l));

  for(ln=l->head;ln;ln=ln->next) { 
    if (strcasecmp(n,ln->name)==0) { 
      found=1;
      break;
    }
  }

  if(found) {
    debugParser("found");
  } else {
    debugParser("not found");
  }

  return found;

} /* checkNameInLabellist */



