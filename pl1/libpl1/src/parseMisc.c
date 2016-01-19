    /* parseMisc.c
     * ====================
     *
     * Implementing the datastructure for a procedure group list,
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
 #include "parseMiscStructs.h"


int setTristateAttribute(int *variable, int data);
int setIntegerAttribute(int *variable, int data);
int setBooleanAttribute(int *variable, int data);
int setNstateAttribute(int *variable, int data);
char *sprintTristateAttribute(const char *name,int value);
char *sprintIntegerAttribute(const char *name, int value);
char *sprintBooleanAttribute(const char *name, int data);
char *sprintNstateAttribute(const char *name, int data);


/* 
 * 
 * Possible return codes:
 *  0 : attribute set without errors.
 * 	  : attribute should be set but was already set
 *    : attribute should be set but was already unset
 * 	  : attribute should be unset but was already set
 *    : attribute should be unset but was already unset
 * 
 */ 
int setTristateAttribute(int *v, int d)
{   if(d<0) return 0;
	if(*v==-1) // TODO: ATTRIBUTE_UNDEFINED
	{
		*v=d;
		return 0;
	}
	else if (*v==d) return -1; // already set
	return -2;
}

/* 
 * 
 * Possible return codes:
 *  0 : attribute set without errors.
 * 	  : attribute should be set but was already set
 *    : attribute should be set but was already unset
 * 	  : attribute should be unset but was already set
 *    : attribute should be unset but was already unset
 * 
 */ 
int setBooleanAttribute(int *v, int d)
{   if(d<0) return 0;
	if(*v==-1) // TODO: ATTRIBUTE_UNDEFINED
	{
		*v=d;
		return 0;
	}
	else if (*v==d) return -1; // already set
	return -2;
}

int setIntegerAttribute(int *v, int d)
{ if(d<0) return 0; // TODO: have null indicator for integer attributes.
  *v=d;
  return 0;
}

int setNstateAttribute(int *v, int d)
{ return setIntegerAttribute(v,d);
}


char *sprintTristateAttribute(const char *name,int value)
{
	return sprintfString("%s: %s",name,(value<0?"UNDEFINED":(value==0?"UNSET":"SET")));
}
char *sprintBooleanAttribute(const char *name,int value)
{
	return sprintfString("%s: %s",name,(value<0?"UNDEFINED":(value==0?"UNSET":"SET")));
}
		
char *sprintIntegerAttribute(const char *name,int value)
{
	return sprintfString("%s: %i",name,value);
}

char *sprintNstateAttribute(const char *name,int value)
{
	return sprintIntegerAttribute(name,value);
}
