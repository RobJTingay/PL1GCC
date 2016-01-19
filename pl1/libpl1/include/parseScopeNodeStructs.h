    /* parseScopeNodeStructs.h
     * =======================
     *
     * Defining the structures used by scope node.c
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
#ifndef PARSESCOPENODESTRUCTS_H_
#define PARSESCOPENODESTRUCTS_H_

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



/* Scope declares */
struct ScopeNode
{ struct ScopeNode *head,*tail ;
  struct ScopeNode *parent,*next ;
  struct ListOfNames *name ; /* this is really the label of the statement */
  struct DeclareElementList *dcl ;
  int stmttype; /* token number as defined in pl1.tab.h */
};

#endif /*PARSESCOPENODESTRUCTS_H_*/
