    /* parseDclStmt.c
     * ====================
     *
     * Implementing the datastructure and functions for the declare statement.
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
     */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

extern int error(const char *msgtext); //TODO: fix error

#include "PPDebugLog.h"
#include "parseMisc.h"
#include "TokenLocation.h"
#include "parseDclOptionList.h"
#include "parseDclStmtStructs.h"
#include "PLIParserStructs.h"
#include "PLIMessage.h"

/* Prototypes */
struct DeclareElement *newDeclareElement( void );
struct DeclareElement *setNewDeclareElement(char *varname,struct DclOptionList *dcloptionlist,int structureLevel,int unionType, YYLTYPE *location);
int mergeDeclareElements(struct DeclareElementList *list,struct DclOptionList *dcloptionlist,int structureLevel,int unionType,void * scanner);
struct DeclareElementList *addElement2List(struct DeclareElementList *list,struct DeclareElement *dclelement);
struct DeclareElementList *addElementList2List(struct DeclareElementList *list,struct DeclareElementList *list2);
struct DeclareElementList *newDeclareElementList(struct DeclareElement *dclelement);
int checkStructureLevels(struct DeclareElementList *list, struct PLIParserParms *ppp);
struct DeclareElement *findStructureParent(struct DeclareElement *previous,struct DeclareElement *current);
void showDeclareElement(struct DeclareElement *de);
char * sprintf_DeclareElement_short(struct DeclareElement *de);

/* code ... */
struct DeclareElement *newDeclareElement ( void  )
{ struct DeclareElement *work;
  work=malloc(sizeof(struct DeclareElement));
  if(work==NULL) error("parseDclStmt.c: error allocating DeclareElement.");
  work->next=NULL;	
  work->dclName=NULL;
  work->dclAttr=NULL;
  work->implied=-1; /* explicit defined */
  work->structureLevel=-1;
  work->structureParent=NULL;     /* level - 1  */
  work->structureSibling=NULL;    /* same level */
  work->structureFirstChild=NULL; /* level + 1  */
  work->dclLocation=NULL;
  return work;
}

struct DeclareElement *setNewDeclareElement
  ( char *varname
  , struct DclOptionList *dcloptionlist
  , int structureLevel
  , int unionType // CELL/UNION
  , YYLTYPE *location
  )
{ struct DeclareElement *work;
  work=newDeclareElement();
  debugParser("newDeclareElement...name %s, structure level %i, declare attributes is %s allocated, CELL/UNION %i\n",varname,structureLevel,(dcloptionlist?"":"not"),unionType);

  work->dclName=varname;
  if(dcloptionlist && dcloptionlist->uniontype==DCL_UNIONTYPE_UNKNOWN)
  { 
  	/* indicates the parsing of the declare statement is not yet complete,
  	 * and the parser has tried to allocate a structure, but has not been able
  	 * assign any information to the structure.
  	 * Better drop it now, before problems arises. 
  	 */
   	debugParser("newDeclareElement...name %s, dcloptionlist uniontype is UNKNOWN, discarding attributes\n",varname);
  	free(dcloptionlist);
  	work->dclAttr=NULL;
  }
  else
  work->dclAttr=dcloptionlist;
  work->implied=0; /* explicit defined */
  work->structureLevel=structureLevel;
  
  if(location==NULL)
    work->dclLocation=NULL;
  else
  { work->dclLocation=malloc(sizeof(YYLTYPE));
    memcpy(work->dclLocation,location,sizeof(YYLTYPE));
  }
  
  
  return work;
}


/**
 * assign the dclelement to all elements within the element list.
 * 
 * returns
 *   0: ok.
 *  <0: error.
 *  -1: conflicting uniontype
 *  
 */
int mergeDeclareElements(struct DeclareElementList *list
,struct DclOptionList *dcloptionlist
,int structureLevel
,int unionType // CELL/UNION
, void * scanner
)
{ struct DeclareElement *work;
  struct DclOptionList  *prevAttr;
  int ok;
  debugParser("mergeDeclareElements...structureLevel %i, dcloptionlist is %sallocated, CELL/UNION %i\n",structureLevel,(dcloptionlist?"":"not "),unionType);
	
	prevAttr=NULL;
	for(ok=1,work=list->head; (ok>=0)&&work; work=work->next)
	{   debugParser("mergeDeclareElements...merging with work->name %s, work->structureLevel %i, work->dclAttr is %sallocated\n"
		            ,work->dclName,work->structureLevel,(work->dclAttr?"":"not "));
		
		if(work->structureLevel==-1) work->structureLevel=structureLevel; /* only use structure level if not already set */
		
		if(dcloptionlist)
		{
			if(work->dclAttr==NULL) 
			{   debugParser("mergeDeclareElements...merging with work->name %s, work->dclAttr was null, and is set to dcloptionlist \n",work->dclName);
				work->dclAttr=dcloptionlist; // dcl (a,b) fixed
			} 
			else
			if(work->dclAttr->uniontype==DCL_UNIONTYPE_UNKNOWN 
			&& dcloptionlist->uniontype!=DCL_UNIONTYPE_UNKNOWN) // dcl (a ctl,n) fixed
			{    debugParser("mergeDeclareElements...merging with work->name %s, has unknown uniontype, is set to %i \n"
				            ,work->dclName,dcloptionlist->uniontype);
				 work->dclAttr->uniontype=dcloptionlist->uniontype;
				 work->dclAttr->dclpgm=dcloptionlist->dclpgm; // Update any of the union pointers
			}
			else
			if(work->dclAttr->uniontype!=DCL_UNIONTYPE_UNKNOWN 
			&& dcloptionlist->uniontype==DCL_UNIONTYPE_UNKNOWN) // dcl (a float,n fixed) external/internal/parameter
			{ debugParser("mergeDeclareElements...merging with work->name %s, has uniontype %i, and dcloptionlist has uniontype unknown "
				          ,work->dclName,work->dclAttr->uniontype);
			   /* check that work->dclAttr is different from the previous 
			   * work->dclAttr, because it could be the two elements share 
			   * the same dclAttr.
			   */
			 if (work->dclAttr!=prevAttr)
			 {debugParser(", setting work->dclAttr to dcloptionlist\n");
			  ok=setDclElementAttr(work->dclAttr, dcloptionlist);
			 }
			 else
			 {debugParser(", work->dclAttr sharing same address as previous and is thus reused.\n");
			 }
			}
			else
			if(work->dclAttr->uniontype!=dcloptionlist->uniontype) 
			{debugParser("ERROR:mergeDeclareElements...merging with work->name %s, work->dclAttr->uniontype %i is not equal to dcloptionlist %i\n"
				         ,work->dclName, work->dclAttr->uniontype, dcloptionlist->uniontype);
				yyerror(work->dclLocation,scanner,sprintfString("Setting UNION/CELL for %s",work->dclName));
				ok=-1; // dcl (a entry,b) binary
			}
			else
			{ 
				debugParser("mergeDeclareElements...merging with work->name %s, has same uniontype %i as dcloptionlist "
				           ,work->dclName,work->dclAttr->uniontype);
			  /* check that work->dclAttr is different from the previous 
			   * work->dclAttr, because it could be the two elements share 
			   * the same dclAttr.
			   */
			 if (work->dclAttr!=prevAttr)
			 {debugParser(", setting work->dclAttr to dcloptionlist\n");
			   ok=setDclElementAttr(work->dclAttr, dcloptionlist);
			   if(ok<0)
                 yyerror(work->dclLocation,scanner,sprintfString("Setting attribute for %s",work->dclName));
			 }
			 else
			 {debugParser(", work->dclAttr sharing same address as previous and is thus reused.\n");
			 }
			}
		}
	  prevAttr=work->dclAttr;
	}
	return ok; 
}

struct DeclareElementList *addElement2List(struct DeclareElementList *list,struct DeclareElement *dclelement)
{   debugParser("addElement2List...name %s\n",dclelement->dclName);
	
	if(dclelement==NULL) return list; // dclelement is not allocated
	
	if(list==NULL) error("parseDclStmt.c: sequencing error, list is NULL");

	if(list->head==NULL)
	  list->head=dclelement;
    else /* add dclelement to list */
	  list->tail->next=dclelement;
	  
	list->tail=dclelement;

	return list;	 
} 

struct DeclareElementList *addElementList2List(struct DeclareElementList *list,struct DeclareElementList *list2)
{   debugParser("addElementList2List ...\n");
	
	if(list2==NULL) return list; // list2 is not allocated
	if(list2->head==NULL) return list; // list2 is empty
	
	if(list==NULL) error("parseDclStmt.c: sequencing error, list is NULL");
	
	if(list->head==NULL)
	  list->head=list2->head;
	else /* add list2 to list */
	  list->tail->next=list2->head;
	  
	list->tail=list2->tail;
	// TODO: check if dispose list2?
	return list;	 
} 

struct DeclareElementList *newDeclareElementList(struct DeclareElement *dclelement)
{ struct DeclareElementList *work;
  debugParser("newDeclareElementList...name %s\n",(dclelement?dclelement->dclName:"(null)"));
  
  work=malloc(sizeof(struct DeclareElementList));
  if(work==NULL) error("parseDclStmt.c: error allocating struct DeclareElementList");
  
  work->head=dclelement;
  work->tail=work->head;
  
  return work;
}


/*
 * 
 * assign parent, child, sibling relations for sstructures.
 * 
 * 
 * dcl (a,b,c)  fixed
 *  , 1 d 
 *  , 3 e
 *  , 5 (f,g) fixed 
 *  , 5 h     
 *  , 7 (h1,h2) float
 *  , 3 i     fixed
 *  ;
 * 
 *
 * On entry the struct DeclareElementList list contains:
 * 
 *   /-----------------\
 *   !  1) ! 2)  ! 3)  !-->
 *   !  4) ! 5)  ! 6)  !  
 *   \-----------------/
 * 
 * 1) name
 * 2) structure level
 * 3) next
 * 4) structureParent
 * 5) structureSibling
 * 6) structureFirstChild
 * 
 *   /-----------------\    /-----------------\    /-----------------\
 *   !  a  ! -1  !  b  !    !  b  ! -1  !  c  !    !  c  ! -1  !  d  !
 *   !  @  !  @  !  @  !    !  @  !  @  !  @  !    !  @  !  @  !  @  !
 *   \-----------------/    \-----------------/    \-----------------/     
 *        
 * 
 *   /-----------------\    /-----------------\    /-----------------\
 *   !  d  !  1  !  e  !    !  e  !  3  !  f  !    !  f  !  5  !  g  !
 *   !  @  !  @  !  @  !    !  @  !  @  !  @  !    !  @  !  @  !  @  !
 *   \-----------------/    \-----------------/    \-----------------/     
 *        
 * 
 *   /-----------------\    /-----------------\    /-----------------\
 *   !  g  !  5  !  h  !    !  h  !  5  ! h1  !    ! h1  !  7  ! h2  !
 *   !  @  !  @  !  @  !    !  @  !  @  !  @  !    !  @  !  @  !  @  !
 *   \-----------------/    \-----------------/    \-----------------/     
 *        
 * 
 *   /-----------------\    /-----------------\ 
 *   ! h2  !  7  !  i  !    !  i  !  3  !  @  ! 
 *   !  @  !  @  !  @  !    !  @  !  @  !  @  ! 
 *   \-----------------/    \-----------------/      
 *        
 *
 * 
 * when all is good, 1) - 3) is unchanged, and 4) - 6) contains the correct references 
 *  
 *   /-----------------\    /-----------------\    /-----------------\
 *   !  a  ! -1  !  b  !    !  b  ! -1  !  c  !    !  c  ! -1  !  d  !
 *   !  @  !  @  !  @  !    !  @  !  @  !  @  !    !  @  !  @  !  @  !
 *   \-----------------/    \-----------------/    \-----------------/     
 *   /-----------------\    /-----------------\    /-----------------\
 *   !  d  !  1  !  e  !    !  e  !  3  !  f  !    !  f  !  5  !  g  !
 *   !  @  !  @  !  e  !    !  d  !  i* !  f  !    !  e  !  g* !  @  !
 *   \-----------------/    \-----------------/    \-----------------/     
 *   /-----------------\    /-----------------\    /-----------------\
 *   !  g  !  5  !  h  !    !  h  !  5  ! h1  !    ! h1  !  7  ! h2  !
 *   !  e* !  h* !  @  !    !  e* !  @  ! h1  !    ! h   !  h2*!  @  !
 *   \-----------------/    \-----------------/    \-----------------/     
 *   /-----------------\    /-----------------\ 
 *   ! h2  !  7  !  i  !    !  i  !  3  !  @  ! 
 *   ! h*  !  @  !  @  !    !  d* !  @  !  @  ! 
 *   \-----------------/    \-----------------/
 * 
 * references with * is changed via findStructureParent, eg      
 * @*: work->parent=findStructureParent(h2,i)
 *                 =findStructureParent(h,i)
 *                 =findStructureParent(e,i)
 *                 =d
 *        
 * 
 * 1) name
 * 2) structure level
 * 3) next
 * 4) structureParent
 * 5) structureSibling
 * 6) structureFirstChild
 * 
 *  
 *  
 */


int checkStructureLevels(
  struct DeclareElementList *list
, struct PLIParserParms *ppp
)

{  struct DeclareElement *work, *prev;
	
  prev=NULL;
  for(work=list->head;work;work=work->next)
  { if(work->structureLevel==1)
  	{ prev=work;
  	  if(( work->next 
  	   && work->next->structureLevel<=1 )
  	   || work->next==NULL  /* work is the only element in the declare statement. */
  	    )
  	  { 
  	  	addWarnMessage(ppp,	sprintfString("structure %s only contains level 1 element.", work->dclName));
  	  }
  	}
  	else
  	if(work->structureLevel>1)
  	{ if(prev==NULL)
  	  { addErrorMessage(ppp,sprintfString("structure %s must start at level 1, and not at %i.", work->dclName,work->structureLevel));
  	  	return -1;
  	  }
      if (prev->structureLevel<work->structureLevel)
      {  prev->structureFirstChild=work;
         work->structureParent=prev;
      }
      else
         work->structureParent=findStructureParent(prev,work);
      prev=work;
  	}
  	else
  	  prev=NULL;
  }
  return 0;	
}

struct DeclareElement *findStructureParent(struct DeclareElement *p,struct DeclareElement *w)
{  if(p==NULL) return NULL;
   if(p->structureLevel<=w->structureLevel)
   { if (p->structureSibling==NULL) p->structureSibling=w;
     return p->structureParent;
   }
   
   return findStructureParent(p->structureParent,w);  
}

/**
 * 
 * show the content of a DeclareElement using debugParser
 * 
 * 
 */

void showDeclareElement(struct DeclareElement *de)
{ /*
struct DeclareElement 
	struct DeclareElement *next;	
	char * dclName;
	struct DclOptionList * dclAttr;
	int structureLevel;
	int implied  ; /-* boolean: IMPLIED  *-/
	struct DeclareElement *structureParent;     /-* level - 1  *-/
	struct DeclareElement *structureSibling;    /-* same level *-/
	struct DeclareElement *structureFirstChild; /-* level + 1  *-/
};
   */
	
	if(de==NULL) 
    { error("get real ... DeclareElement cannot be NULL."); 
      return;
    }
	//debugParser(" name %s: type: %i, attr: %i\n",de->dclName,de->dclType,de->dclAttr);
	debugParser(" name %s, structure level %i\n",de->dclName, de->structureLevel);
	debugParser("  Attribute implied %s\n",sprintBooleanAttribute("IMPLIED",de->implied));
	debugParser("  Location %s\n",sprintf_yylloc(de->dclLocation));
	showStructDclOptionList(de->dclAttr);
}

char * sprintf_DeclareElement_short(struct DeclareElement *de)
{   char * work;
	/*
struct DeclareElement 
	struct DeclareElement *next;	
	char * dclName;
	struct DclOptionList * dclAttr;
	int structureLevel;
	int implied  ; /-* boolean: IMPLIED  *-/
	struct DeclareElement *structureParent;     /-* level - 1  *-/
	struct DeclareElement *structureSibling;    /-* same level *-/
	struct DeclareElement *structureFirstChild; /-* level + 1  *-/
};
   */
	
	if(de==NULL)   return sprintfString(".. no element ..");
	debugParser("sprintf_DeclareElement_short name %s structure level %i\n",de->dclName,de->structureLevel);	
	
	if(de->structureLevel>1)
	{ if(de->structureParent==NULL) printf("unexpected error: structureParent is null\n");
	  work=sprintfString("%sStructure level %i of %s",(de->implied==1?"IMPLIED ":"")
	                    , de->structureLevel
	                    , (de->structureParent->dclName?de->structureParent->dclName:"?? no parent name")
	                    );
	}
	else
	if(de->structureLevel==1)
	  work=sprintfString("%sStructure level %i",(de->implied==1?"IMPLIED ":"")
	                    , de->structureLevel
	                    );
	else
	  work=sprintfString("%s",(de->implied==1?"IMPLIED ":" ")
	                    );
	
	return work;
	
}

