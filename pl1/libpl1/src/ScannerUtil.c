    /* ScannerUtil.c
     * =============
     *
     * Helper functions used by the scanner.
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


#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "PPDebugLog.h"

/* Scanner utilities */
/*
 * q_strndup()
 */

/* prototypes */
char * q_strndup(char *s, int l);
char * q_requote_str(char *s);

/** (mike's text)
   utility method for use only here in Flex.  This method
   will walk through a string, and turn all the 
   escaped quotes (a pair of either "" or '') into 
   just one quote.  Quotes are escaped by doubling them
   up, but the final string should have just one in each
   such case.
*/
char * q_strndup(char *s, int l)
{ char q;
  char *r=malloc(l);
  char *w;
  int skip_c;
  
  debugVerbose("$$$ removing quotes from >%s<\n",s);
  
  q=*s; /* first char is the quote to remove. Can be ' or " */
  if(q!='\'' && q!='"') q='\0'; /* first char not quote. */
  else s++; /* skip the first quote */
  
  skip_c=0;
  for(w=r;*s;s++)
  { if(*s==q&&skip_c==0) skip_c=1;
  	else skip_c=0;
  	
  	if(!skip_c) *w++=*s; 
  }
  *w='\0';
  debugVerbose("$$$ result w/o quotes from >%s<\n",r);
  return r;
}

char * q_requote_str(char *s)
{ int l;
  char *r;
  char *w;
  
  debugVerbose("$$$ adding quotes to >%s<\n",s);
 
  l=strlen(s)+3;
  r=malloc(l);
  
  w=r;
  *w++='\'';
  
  for(;*s;s++)
  { *w++=*s; 
  }
  
  *w++='\'';
  *w='\0';
  
  debugVerbose("$$$ result w quotes >%s<\n",r);
  return r;
}

