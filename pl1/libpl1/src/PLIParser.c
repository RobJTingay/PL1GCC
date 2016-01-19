    /* PLIParser.c
     * ====================
     *
     * Implementing the functions parser parameters and the list of included files.
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

#include "PPDebugLog.h"
#include "TokenLocation.h"
#include "PLIParserStructs.h"
#include "PLIMessage.h"
#include "parseParserStackStructs.h"
#include "parseScopeNode.h"
#include "parseDclStmt.h"
#include "parseDclOptionList.h"
#include "parseMiscStructs.h"
#include "pl1.tab.h"
#include "yyFlex.h"


extern int error(const char *msgtext); //TODO: fix error

/* prototypes */
struct PLIParserParms*  initPL1ParserParms(void);
 
struct PLIParserIncludeFileList *newPLIParserIncludeFileList(void);
struct PLIParserIncludeFile * addIncludeFile(struct PLIParserIncludeFileList *list,int type ,char *name,struct PLIParserIncludeFile *current, YYLTYPE *location);
void produceXrefList(struct ScopeNode *current);
void showXrefList(struct ScopeNode *node);
void produceIncludeList(struct PLIParserIncludeFileList *list);
int showWelcome(struct PLIParserParms *ppp);
int pl1_parser(struct PLIParserParms *ppp);

/* functions */
struct PLIParserParms* initPL1ParserParms(void)
{ struct PLIParserParms *work;
  work=malloc(sizeof(struct PLIParserParms));
  if (work==NULL)
     error("PLIParser.c:could not allocate pl1parserparms.\n");
     
  /* initialize struct */
  work->debugParser=0;
  work->debugScanner=0;
  work->debugScannerPreprocessor=0;
  work->includeSuffix=NULL;
  work->includePath=NULL;
  work->preprocessOnly=0;
  work->messageLevel=1; /* INFO */
  work->inputFile = NULL;
  work->fileName= NULL;
	
 /* initialise message structures */
 work->msgErrors=initPLIMessage();
 work->msgWarns=initPLIMessage();
 work->msgInfos=initPLIMessage();

 /* initialise structure for list of included files */
 work->includefilelist=newPLIParserIncludeFileList();
 
 /* initialise structure for scope root, the variable that contain everything */
 work->scopeRoot=newScopeRoot();
 
 return work;
}


struct PLIParserIncludeFileList *newPLIParserIncludeFileList()
{
	struct PLIParserIncludeFileList *work;
	
	work=malloc(sizeof(struct PLIParserIncludeFileList));
	if(!work)
	{ error("ouf of mem. newPLIParserIncludeFileList"); 
	  return NULL;
	}
	
	work->head=NULL;
	work->tail=NULL;
	work->count[INCTYPE_FILE]=0;
	work->count[INCTYPE_PPVAR]=0;
	
	return work;
}

struct PLIParserIncludeFile * addIncludeFile
( struct PLIParserIncludeFileList *list
 ,int type 
 ,char *name
 ,struct PLIParserIncludeFile *current
 , YYLTYPE *location
 )
{ struct PLIParserIncludeFile *temp;
  debugLexer("PLIParserAddIncludeFile type %i, name %s\n",type,name);
  if(list==NULL) error("get real, include file list pointer cannot be NULL");
  
  temp=malloc(sizeof(struct PLIParserIncludeFile));
  if(!temp)
  { error("ouf of mem. addIncludeBuffer"); 
  	return NULL;
  }

  temp->name=strdup(name);
  temp->type=type;
  
  if(location)
  {
    temp->location=malloc(sizeof(YYLTYPE));
    if(temp->location==NULL) error("PLIParser.c: PLIParserAddIncludeFile: out of mem.");
    memcpy(temp->location,location,sizeof(YYLTYPE));
  }
  else
    temp->location=NULL;
  
  if(current)
  {
    temp->includedFrom=malloc(sizeof(struct PLIParserIncludeFile));
    if(temp->includedFrom==NULL) error("PLIParser.c: PLIParserAddIncludeFile: out of mem.");
    temp->includedFrom=memcpy(temp->includedFrom,current,sizeof(struct PLIParserIncludeFile));
  }
  else
    temp->includedFrom=NULL;
  
  temp->next=NULL;

  if(list->head==NULL)
  { /* first element */
  	list->head=temp;
    list->tail=temp;
    list->count[type]=1;
  }
  else
  { /* add element to end of list, and update tail pointer */
  	list->tail->next=temp;
  	list->tail=temp;
    list->count[type]++;
  }
  
  temp->refno=list->count[type];
  
  return temp;
}


/* traverse the scope nodes, width first, and for eachnode invokd
 * the function showXrefList
 */
void produceXrefList(struct ScopeNode *current)
{ struct ScopeNode *work;
  debugParser("produceXrefList\n");
  /* width first */
  for(work=current;work;work=work->next)
  { showXrefList(work);
  }
  /* now recurse into the tree for the first child for each node. */
  for(work=current;work;work=work->next)
  { produceXrefList(work->head);
  }
  
}

/* for the scopeNode show the content of the structure DeclareElementList */ 
void showXrefList(struct ScopeNode *node)
{   struct DeclareElement *work;
	/*
	 * 	  layout of ref.list
	 * 	  fileno  lineno   name   type   scope
	 * 
	 */
	if (node == NULL) return;
	if (node->dcl==NULL) return;
	if (node->dcl->head==NULL) return;
	
	debugParser("showXrefList\n");
	
	if(node->parent==NULL)
	{ /* show header */
	   printf("xref \t%s\t%s\t%s\t%s\n"
	                ,"line"
	                ,"name"
	                ,"type"
	                ,"scope"
	                );
	}
	
	/* go through the list of declared elements */
	for(work=node->dcl->head;work;work=work->next)
	{
	   printf("xref \t%s\t%s\t%s\t%s\t%s\n"
	                ,sprintf_yylloc_short(work->dclLocation)
	                ,work->dclName
	                ,sprintf_DclOptionList_short(work->dclAttr)
	                ,sprintf_DeclareElement_short(work)
	                ,node->name->head->name
	         );
	}
	
}


void produceIncludeList(struct PLIParserIncludeFileList *list)
{ /*
   * Format of listing of included files
   *  layout:
   *	  included-at fileno lineno    file-name  include-from name 
   */
   
  if(list==NULL) return;
  if(list->count[INCTYPE_FILE]==0) return;
  debugParser("produceIncludeList\n");
  /*
	struct PLIParserIncludeFile
	{ char * name;
	  int type;
	#define INCTYPE_FILE 0
	#define INCTYPE_PPVAR 1
	  YYLTYPE *location;
	  struct PLIParserIncludeFile *includedFrom;
	  struct PLIParserIncludeFile *next;
	  int refno;
	};
  */
  struct PLIParserIncludeFile *work;
  
  printf("\n Include file summary of %i files\n",list->count[INCTYPE_FILE]);
  printf("\t\tref\tline,col\tfilename\n");
  for(work=list->head; work; work=work->next)
  { if(work->type==INCTYPE_FILE)
  	{
  	  printf("include\t[%i]\t%s\t%s"
  	  		  , work->refno
              , sprintf_yylloc_short(work->location)
  	          , work->name
  	        );
      if(work->includedFrom)
         printf(" included from\t[%i]\t%s"
  	              , work->includedFrom->refno
  	              , work->includedFrom->name
  	           );
  	  printf("\n");
  	} 
  }
}

/* show compiler settings and versions */
int showWelcome(struct PLIParserParms *ppp)
{	char * buf;
	printf("*****************************************\n");
	printf("*  PL/I COMPILER FOR GCC VERSION 0.0.15 *\n");
	printf("*  built  %s %s          *\n",__DATE__,__TIME__);
//	printf("*  built  01234567891 01234567          *\n",__DATE__,__TIME__);
	printf("*        pl1gcc.sourceforge.net         *\n");
	printf("*****************************************\n");
	printf("\n");
	printf("*****************************************\n");
	printf("* \tCompiling file \t%s\n",ppp->fileName);
	printf("* Compiler options used:\n");
	printf("* \tdebug parser\t%s\n",ppp->debugParser?"ON":"OFF");
	printf("* \tdebug scanner\t%s\n",ppp->debugScanner?"ON":"OFF");
	printf("* \tdebug preprocessor\t%s\n",ppp->debugScannerPreprocessor?"ON":"OFF");
	printf("* \tpreprocessor only\t%s\n",ppp->preprocessOnly?"ON":"OFF");
	printf("* \tinclude path\t%s\n",ppp->includePath?ppp->includePath:"not specified");
  	buf = getenv("PL1SYSLIB");
	printf("* \tPL1SYSLIB path\t%s\n",buf?buf:"variable not found");
	printf("* \tinclude suffix\t%s\n",ppp->includeSuffix?ppp->includeSuffix:"not specified");
	printf("*****************************************\n");
	return 0;
}

/* ================================  */
/* main entry to the PL/I parser. 
 * 
 * Called from pl11.c
 *
 * - initializes reentrant scanner
 * - verifies the parameter structure
 * - set logging level 
 * - calls yyparse
 * - finallys:
 * -  prints messages
 * -  variable xref
 * -  include file summary
 * 
 * 
 *  
 * 
 */
int pl1_parser(struct PLIParserParms *ppp)
{ 
  int rc;
  extern int yydebug;
  extern int yyparse( yyscan_t scanner );

  /* TODO: Add default parameter structure */
  if(ppp==NULL) printf("Parameter structure is not allocated");

  /* set debug flags as needed */

  /* initialize the reentrant scanner */
  yyscan_t scanner;
  rc=yylex_init( &scanner ); 
  ppp->flexScanner=&scanner ;
  yyset_in(ppp->inputFile,scanner); 
  yyset_debug(ppp->debugScanner,scanner); 
  yyset_extra((void *)ppp, scanner); 

  yydebug=ppp->debugParser;
  //pl1pp_yydebug=ppp->debugScannerPreprocessor;
  //printf("pl1gcc: starting parse. yydebug:%i, yy_flex_debug:%i\n",yydebug,yyget_debug(scanner));

  int option_debug_level=0;

  showWelcome(ppp);

  if (ppp->debugParser) option_debug_level |= DL_PARSER ;
  if (ppp->debugScanner) option_debug_level |= DL_LEXER ;
  if (ppp->debugScannerPreprocessor) option_debug_level |= DL_PARSER_PP ;
  if (ppp->messageLevel==1) option_debug_level |= DL_INFO ;
  if (ppp->messageLevel==2) option_debug_level |= DL_VERBOSE ;

  setDebugLevel(option_debug_level);
   
  if(ppp->preprocessOnly) 
  { /* 
       in this mode we are only doing preprocessing so that 
       the user can see if the parser is getting the post
       processed text they think it is.

       To do this just invoke the lexer so that it generates tokens,
       we don't do anything with them though.

    */
    YYSTYPE *lvalp=malloc(sizeof(YYSTYPE));
    YYLTYPE *llocp=malloc(sizeof(YYLTYPE));
    rc=0;
    while(yylex(lvalp,llocp,scanner)) 
    {
      /* keep processing tokens until end of file or error. */
    }
  } 
  else 
  {
    /* do the parse */
    rc=yyparse (scanner);
    yylex_destroy( scanner );
  }
  
  showMessage("errors",ppp->msgErrors);
  showMessage("warnings",ppp->msgWarns);
  showMessage("information",ppp->msgInfos);
  
  produceXrefList(ppp->scopeRoot);
  produceIncludeList(ppp->includefilelist);
  
  /* 
   * ensure the pl1_parser returns 1 if at least 
   * one error is found 
   */
  if(ppp->msgErrors->count>0) rc=1;
  
  return rc;
}
