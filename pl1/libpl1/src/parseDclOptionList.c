    /* parseDclOptionList.c
     * ====================
     *
     * Implementing the datastructure for the declare option list.
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
#include "parseDclOptionListStructs.h"



struct DclOptionList *newDclOptionList(void);
struct DclOptionNumeric *newDclOptionNumeric(void);
struct DclOptionPgm *newDclOptionPgm(void);
struct DclOptionChar *newDclOptionChar(void);

int initDclOptionNumeric(struct DclOptionList *dol);
int initDclOptionPgm(struct DclOptionList *dol);
int initDclOptionChar(struct DclOptionList *dol);

void showStructDclOptionList(struct DclOptionList *dol);
void showStructDclOptionNumeric(struct DclOptionNumeric *don);
void showStructDclOptionPgm(struct DclOptionPgm *dop);
void showStructDclOptionChar(struct DclOptionChar *doc);
char * sprintf_DclOptionList_short(struct DclOptionList *dol);

int setDclElementAttr(struct DclOptionList *old, struct DclOptionList *new);

struct DclOptionList *newDclOptionList(void)
{ struct DclOptionList *work;
  work=malloc(sizeof(struct DclOptionList));
  if(!work) error("DclOptionList: out of memory.");
 
  work->uniontype=DCL_UNIONTYPE_UNKNOWN;
  work->storageclass=-1; /* n-state: AUTOMATIC/BASED/CONTROLLED/DEFINED/PARAMETER/STATIC */
  work->internal =-1; /* tristate: INTERNAL/EXTERNAL */
  
  return work;
}

struct DclOptionNumeric *newDclOptionNumeric(void)
{ struct DclOptionNumeric *work;
  work=malloc(sizeof(struct DclOptionNumeric));
  if(!work) error("DclOptionNumeric: out of memory.");
  
  work->domain=-1; /* REAL, COMPLEX */
  work->base=-1;   /* FIXED, FLOAT  */
  work->type=-1;   /* BINARY, DECIMAL */
  
  work->precision=-1;
  work->scale=-1;
  
  return work;
}

struct DclOptionPgm *newDclOptionPgm(void)
{ struct DclOptionPgm *work;
  work=malloc(sizeof(struct DclOptionPgm));
  if(!work) error("DclOptionPgm: out of memory.");
  
  work->optionlist=NULL;
  work->returnsList=NULL;
  
  work->reducible=-1;   /* tristate: REDUCIBLE/IRREDUCIBLE     */
  work->pgmtype=-1  ;   /* tristate: PROCEDURE/ENTRY*/
  
  return work;
}

struct DclOptionChar *newDclOptionChar(void)
{ struct DclOptionChar *work;
  work=malloc(sizeof(struct DclOptionChar));
  if(!work) error("DclOptionChar: out of memory.");
  
  work->chartype=-1;/* n-state AREA/BIT/CHARACTER/GRAPHIC/PICTURE/WIDECHAR/DATE */
  work->expr=NULL;
  work->refer=NULL;
  
  work->star=-1;/* boolean: '*' */
  
  return work;
}


int initDclOptionNumeric(struct DclOptionList *dol)
{
	if(dol->uniontype==DCL_UNIONTYPE_UNKNOWN) 
    { dol->dclnum=newDclOptionNumeric(); 
      dol->uniontype=DCL_UNIONTYPE_DCLNUM;
    }
    else
    if(dol->uniontype!=DCL_UNIONTYPE_DCLNUM) return -1;
	
	return 0;
}

int initDclOptionPgm(struct DclOptionList *dol)
{
	if(dol->uniontype==DCL_UNIONTYPE_UNKNOWN) 
    { dol->dclpgm=newDclOptionPgm(); 
      dol->uniontype=DCL_UNIONTYPE_DCLPGM;
    }
    else
    if(dol->uniontype!=DCL_UNIONTYPE_DCLPGM) return -1;
	
	return 0;
}

int initDclOptionChar(struct DclOptionList *dol)
{
	if(dol->uniontype==DCL_UNIONTYPE_UNKNOWN) 
    { dol->dclchar=newDclOptionChar(); 
      dol->uniontype=DCL_UNIONTYPE_DCLCHAR;
    }
    else
    if(dol->uniontype!=DCL_UNIONTYPE_DCLCHAR) return -1;
	
	return 0;
}

char * sprintf_DclOptionList_short(struct DclOptionList *dol)
{   char * work;
	debugParser("sprintf_DclOptionList_short\n");	
	
	if(dol==NULL) return sprintfString("-- no attributes declared --");
	
	struct DclOptionNumeric *don;
	struct DclOptionPgm *dop;
	struct DclOptionChar *doch;
	
	// const char *uniontype_text[]={"UNKNOWN ","NUMERIC ","PROGRAM ","CHARACTER "};
	const char *storageclass_text[]={"","AUTOMATIC ","BASED ","CONTROLLED ","DEFINED ","STATIC ","PARAMETER "};
	const char *internal_text[]={"","INTERNAL ","EXTERNAL "};
	
	/* struct DclOptionNumeric */
	const char *domain_text[]={"","REAL ","COMPLEX "};
	const char *base_text[]={"","FLOAT ","FIXED "};
	const char *type_text[]={"","DECIMAL ","BINARY "};
	// const char *precision_text[]={"PRECISION"}; /* integer */
	// const char *scale_text[]={"SCALE"}; /* integer */
	
	/* struct DclOptionPgm */
	const char *reducible_text[]={"","REDUCIBLE ","IRREDUCIBLE "};
	const char *pgmtype_text[]={"","PROCEDURE ","ENTRY "};
	
	/* struct DclOptionChar */
	const char *chartype_text[]={"","AREA ","BIT ","CHARACTER ","GRAPHIC ","PICTURE ","WIDECHAR ","DATE "};


	if(dol->uniontype==DCL_UNIONTYPE_DCLNUM)
	{ don=dol->dclnum;
	  work=sprintfString("%s%s%s%s%s"
	                ,internal_text[dol->internal+1]
	                ,storageclass_text[dol->storageclass+1]
	                ,domain_text[don->domain+1]
	                ,base_text[don->base+1]
	                ,type_text[don->type+1]
	                ); //TODO: precision scale
	}
	else
	if(dol->uniontype==DCL_UNIONTYPE_DCLPGM)
	{ dop=dol->dclpgm;
	  work=sprintfString("%s%s%s%s%s"
	                ,internal_text[dol->internal+1]
	                ,storageclass_text[dol->storageclass+1]
	                ,reducible_text[dop->reducible+1]
	                ,pgmtype_text[dop->pgmtype+1]
	                ,(dop->returnsList?"with RETURNS ":" ")
	                );
	}
	else
	if(dol->uniontype==DCL_UNIONTYPE_DCLCHAR)
	{ doch=dol->dclchar;
	  work=sprintfString("%s%s%s"
	                ,internal_text[dol->internal+1]
	                ,storageclass_text[dol->storageclass+1]
	                ,chartype_text[doch->chartype+1]
	                );
	}
	else
	if(dol->uniontype==DCL_UNIONTYPE_UNKNOWN)
	{ doch=dol->dclchar;
	  work=sprintfString("%s%s"
	                ,internal_text[dol->internal+1]
	                ,storageclass_text[dol->storageclass+1]
	                );
	}
	else
	{ work=sprintfString("%s%s uniontype %i, not in formatting function"
	                ,internal_text[dol->internal+1]
	                ,storageclass_text[dol->storageclass+1]
                    ,dol->uniontype	                
	                );
	}


	return work;
}

void showStructDclOptionList(struct DclOptionList *dol)
{	if(dol==NULL) return;
	debugParser("showStructDclOptionList\n");
	debugParser("  Attribute union type %s\n",sprintNstateAttribute("NUMERIC/PGM/CHAR",dol->uniontype));
	debugParser("  Attribute storageclass %s\n",sprintNstateAttribute("AUTOMATIC/BASED/CONTROLLED/DEFINED/STATIC/PARAMETER",dol->storageclass));
	debugParser("  Attribute visibility %s\n",sprintTristateAttribute("INTERNAL/EXTERNAL",dol->internal));

	switch(dol->uniontype)
	{ case DCL_UNIONTYPE_DCLNUM: showStructDclOptionNumeric(dol->dclnum);
	                             break;
	  case DCL_UNIONTYPE_DCLPGM: showStructDclOptionPgm(dol->dclpgm);
	                             break;
	  case DCL_UNIONTYPE_DCLCHAR: showStructDclOptionChar(dol->dclchar);
	                             break;
	  case DCL_UNIONTYPE_UNKNOWN: debugParser("uniontype is UNKNOWN\n");
	                             break;
	  default:
	  debugParser("uniontype is not yet implementet %i\n",dol->uniontype);
	                             break;
	}
	  
}

void showStructDclOptionNumeric(struct DclOptionNumeric *don)
{	if(don==NULL) return;
	/*
	 * int domain ; /-* REAL, COMPLEX *-/
	 * int base   ; /-* FIXED, FLOAT  *-/
	 * int type   ; /-* BINARY, DECIMAL *-/
	 * int precision,scale;
	 * 
	 */
	debugParser("showStructDclOptionNumeric invoked\n");
	debugParser("  Attribute %s\n",sprintTristateAttribute("DOMAIN REAL/COMPLEX",don->domain));
	debugParser("  Attribute %s\n",sprintTristateAttribute("BASE FLOAT/FIXED",don->base));
	debugParser("  Attribute %s\n",sprintTristateAttribute("TYPE DECIMAL/BINARY",don->type));
	debugParser("  Attribute %s\n",sprintIntegerAttribute("PRECISION",don->precision));
	debugParser("  Attribute %s\n",sprintIntegerAttribute("SCALE",don->scale));
}


void showStructDclOptionPgm(struct DclOptionPgm *dop)
{	if(dop==NULL) return;
	/*
	 * int reducible ; /-* REDUCIBLE, IRREDUCIBLE *-/
	 */
	debugParser("showStructDclOptionPgm invoked\n");
	debugParser("  Attribute %s\n",sprintTristateAttribute("REDUCIBLE/IRREDUCIBLE",dop->reducible));
	debugParser("  Attribute %s\n",sprintTristateAttribute("PROCEDURE/ENTRY",dop->pgmtype));
}

void showStructDclOptionChar(struct DclOptionChar *doc)
{	if(doc==NULL) return;
	/*	
	 * int chartype ;                 /-* n-state AREA/BIT/CHARACTER/GRAPHIC/PICTURE/WIDECHAR/DATE *-/
     * struct Expression *expr ;      /-* can be NULL *-/
     * struct DeclareElement *refer ; /-* can be NULL *-/
     * int star ;                     /-* boolean '*' *-/
	 */
	debugParser("showStructDclOptionChar invoked\n");
	debugParser("  Type %s\n",sprintNstateAttribute("AREA/BIT/CHARACTER/GRAPHIC/PICTURE/WIDECHAR/DATE",doc->chartype));
}

int setDclElementAttr(struct DclOptionList *old, struct DclOptionList *new)
{   int err;

    err=setNstateAttribute(&(old)->storageclass,new->storageclass); /* AUTOMATIC/BASED/CONTROLLED/DEFINED/STATIC/PARAMETER */
    if(err<0) goto out_error;
	err=setTristateAttribute(&(old)->internal,new->internal);  /* INTERNAL/EXTERNAL */
    if(err<0) goto out_error;
	
	if (new->uniontype==DCL_UNIONTYPE_UNKNOWN) goto out_ok; // done with attributes that are independent of uniontype
	
	
	err=-1;
	// to continute, must either, the old uniontype be unknown, and the new not-unknown, 
	// or the old uniontype is not unknown.
	if (old->uniontype!=new->uniontype 
	 && old->uniontype!=DCL_UNIONTYPE_UNKNOWN
	   ) goto out_error;
    
	if(old->uniontype==DCL_UNIONTYPE_DCLNUM)
	{ 
	  err=setTristateAttribute(&(old)->dclnum->domain,new->dclnum->domain);      /* REAL/COMPLEX */
      if(err<0) goto out_error;
	  err=setTristateAttribute(&(old)->dclnum->type,new->dclnum->type);          /* BINARY/DECIMAL */
      if(err<0) goto out_error;
	  err=setTristateAttribute(&(old)->dclnum->base,new->dclnum->base);          /* FIXED/FLOAT */
      if(err<0) goto out_error;
	  err=setIntegerAttribute(&(old)->dclnum->precision,new->dclnum->precision); /* precision */
      if(err<0) goto out_error;
	  err=setIntegerAttribute(&(old)->dclnum->scale,new->dclnum->scale);         /* scale */
      if(err<0) goto out_error;
	}
	else
	if(old->uniontype==DCL_UNIONTYPE_DCLPGM)
	{ err=setTristateAttribute(&(old)->dclpgm->reducible,new->dclpgm->reducible); /* REDUCIBLE/IRREDUCIBLE */
      if(err<0) goto out_error;
      err=setTristateAttribute(&(old)->dclpgm->pgmtype,new->dclpgm->pgmtype); /* PROCEDURE/ENTRY */
      if(err<0) goto out_error;
	}
	if(old->uniontype==DCL_UNIONTYPE_DCLCHAR)
	{ err=setNstateAttribute(&(old)->dclchar->chartype,new->dclchar->chartype); /* AREA/BIT/CHARACTER/GRAPHIC/PICTURE/WIDECHAR/DATE */
      if(err<0) goto out_error;
      err=setBooleanAttribute(&(old)->dclchar->star,new->dclchar->star); /* '*' */
      if(err<0) goto out_error;
      
      //TODO: check how to do pointer assignment. Should give error is already set.
      if(new->dclchar->expr!=NULL)  old->dclchar->expr=new->dclchar->expr;
      if(new->dclchar->refer!=NULL) old->dclchar->refer=new->dclchar->refer;
	}
	else
	if( old->uniontype==DCL_UNIONTYPE_UNKNOWN
	 && new->uniontype!=DCL_UNIONTYPE_UNKNOWN)
	{ old->uniontype=new->uniontype;
	  //TODO: need to copy structure instead of pointer assignment ?
      if(old->uniontype==DCL_UNIONTYPE_DCLNUM) old->dclnum=new->dclnum;
      else
      if(old->uniontype==DCL_UNIONTYPE_DCLPGM) old->dclpgm=new->dclpgm;
      else 
      if(old->uniontype==DCL_UNIONTYPE_DCLCHAR) old->dclchar=new->dclchar;
      else 
      { err=-1;
      	goto out_error; 
      }
	}
	
	out_ok:
		return 0;
	
	out_error:
		return err;
}



