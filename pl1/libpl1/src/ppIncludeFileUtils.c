    /* IncludeFileUtils.c
     * ==================
     *
     * Utilities for locating PL/I include files
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
#include "ppVariableStructs.h"
#include "PLIParserStructs.h"

extern struct ppContextAnchor *scannerContext;  // set in pl1pp-parser.y, which gets it from pl1-scanner.l
//TODO scannerContext hould be passed as parameter

extern int error(const char *msgtext); //TODO: prober include for error function.


/* ===================================== */
/* prototypes */
int findFile(char *name);
int findEnvFile(char *env, char *name);
int findFileInDirList(char *dirList, char *name);
int findFileInDir(char *dir, char *name);
int fileExists(char *fullname);


/* looking for the include files.
 * The following compiler options are taken into consideration:
 * -I .. -I ..: list of directories to search for include files.
 * -isuffix : suffix to add to the file name on the INCLUDE stmt.
 * Runtime considerations:
 * Environment variable:
 * PL1SYSLIB: list of directories to be searched after the -I directories.
 * 
 */
 char * option_i;       /* include path */
 char * option_isuffix ; /* include suffix */

/** 
 * 
 * return 0 File not found
 *        1 File found
 * 
 * if the file is found the return area is set as follows:
 *    scannerContext->ppreturn->type=LIT_INCLUDE_FILE_NAME;
 *    scannerContext->ppreturn->string=<name of file>;;
 * 
 * 
 * 
 * 
 */
int findFile(char *name)
{
  char defaultSearchDirs[]= "./:./include";

  option_i=scannerContext->ppp->includePath;
  option_isuffix=scannerContext->ppp->includeSuffix;
  
  char *pathToUse;
  
  int isOK=0;
  
  if (!isOK&&option_i!=NULL) 
  { /* if option -I is specified, look there */
  	debugVerbose("include: looking in compiler option -I%s\n",option_i);
  	isOK=findFileInDirList(option_i,name);
  }

  if(!isOK)
  { /* look in default directories */
  	debugVerbose("include: looking in default search dirs: %s\n",defaultSearchDirs);
  	isOK=findFileInDirList(defaultSearchDirs,name);
  }

  if(!isOK)
  { /* look in directory list PL1SYSLIB */
  	pathToUse = getenv("PL1SYSLIB");
  	if(pathToUse) 
  	{
  	debugVerbose("include: looking in environment variable PL1SYSLIB: %s\n",pathToUse);
  	isOK=findFileInDirList(pathToUse,name);
  	}
  	else
  	isOK=0; 
  }

  return isOK;
}


int findEnvFile(char *env, char *name)
{ char *pathToUse;
  
  int isOK=0;
	
  if(!isOK)
  { /* look in directory list contained in environment variable */
  	pathToUse = getenv(env);
  	debugVerbose("include: looking in environment variable %s: %s\n",env,pathToUse);
  	isOK=findFileInDirList(pathToUse,name);
  }

  /* TODO: should the default search be performed if search in env fails ? */
  /* if(!isOK) 
   *   isOK=findFile(name); 
   */
	
  return isOK;
}

int findFileInDirList(char *dirList, char *name)
{
	char * buf=malloc(512);
	char * p1,*p2;
	int isOK;
	if (!buf) error("Internal compiler error: out of mem");
  debugParserpp("include findFileInDirList: dirlist %s, name %s\n",dirList,name);
	
	debugVerbose("include: Looking for file %s in directory list %s\n",name,dirList);
	/* the separator is either ':' or ';' */
	for(isOK=0,p1=buf,p2=dirList;*p2;p2++)
	{
		if(*p2==':' || *p2==';')
		{ // TODO also check last dir list ! *(p2+1)=='\0'
			*p1++='\0';
		    debugParserpp("include findFileInDirList: buf %s, name %s\n",buf,name);
			isOK=findFileInDir(buf,name);
			if(isOK) goto out_found;
			
			*buf='\0';
			p1=buf;
		}
		else
		{ /* TODO: check length of buf, and if >512 allocate larger */
			*p1++=*p2;
		}
	}
	
	goto out_not_found;
	
 out_found: ;
  debugVerbose("include: found the file %s in path %s\n",name,buf);
  free(buf);
  return 1;

out_not_found:;
  free(buf);
  return 0;
  
}


/* should return an index or handle to list of included files */
int findFileInDir(char *dir, char *name)
{
  char * buf, *p1, *p2, *peon;
  char *suffix=option_isuffix;
  unsigned int buflen;


  debugParserpp("include findFileInDir: dir %s, name %s, suffix %s\n",dir,name,suffix);
  
  buflen=strlen(dir)+strlen(name)+5;
  if(suffix!=NULL) buflen+=strlen(suffix);
  buf=malloc(buflen);
  
  for(p1=buf,p2=dir;*p2;p2++) *p1++=*p2;
  
  /* check if last copied char is '/', and if not then addit */
  

  if(*(p1-1)!='/') *p1++='/'; 
  *p1='\0';
  
  for(p2=name;*p2;p2++)  *p1++=*p2;
  *p1='\0';
  
  /* TODO: how should the search be done ? */  
  /* if suffix is specified, then always use the suffix */
  
  peon=p1; /* pointer to end of name without suffix */
  
  if(suffix!=NULL)
  {
  for(p2=suffix;*p2;p2++)  *p1++=*p2;
  *p1='\0';
  
  debugVerbose("include: looking for file with suffix %s\n",suffix);
  if(fileExists(buf)) goto out_found;
  }
  
  /* look for file without suffix */
  *peon='\0'; /* terminate string in buf */
  debugVerbose("include: looking for without suffix:%s\n",name);
  
  if(fileExists(buf)) goto out_found;
  
    
    /* file not found */
	free(buf);
	return 0;

out_found:;
   /* free(buf); dont free just pass buf */
   scannerContext->ppreturn->type=LIT_INCLUDE_FILE_NAME;
   scannerContext->ppreturn->string=buf;
  return 1;
}
		

int fileExists(char *fullname)
{
 debugVerbose("include: Checking existence of file %s\n",fullname);  

  FILE *input = fopen(fullname, "r");

  if(input==NULL) {
  	if(errno!=ENOENT)
  	{ /* only show detailed message if different from not found. */
    perror(fullname);
    debugParserpp("include: Could not open file %s, errno %i ...\n",fullname,errno);
  	}
    return 0;
  }
  
  fclose(input);

  return 1;
}

