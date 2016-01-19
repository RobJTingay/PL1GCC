#ifndef PPDebugLog_H
#define PPDebugLog_H 1

/*

  This particular code is part of the PL/I preprocessor, intended to be
  used in the PL/I front end for GCC.  

  Copyright 2006  Michael J. Garvin
  Copyright 2006, 2011  Henrik Sorensen

  Original author:

    Michael J. Garvin
    http://www.magma.ca/~mgarvin/
    mgarvin@magma.ca
    mgarvin@nortel.com
 
*/

/*

  This file is part of GCC PL/I.

  GCC PL/I is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; version 3.

  GCC PL/I is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with GCC PL/I; see the file COPYING3.  If not see
  <http://www.gnu.org/licenses/>.

*/

/* system includes */

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

/* PPDebugLog - utility stuff for developer debugging and logging. */

/* 
   This is the main switch that will turn on debugging mode
   regardless of  weather or not debugging structures/data 
   are turned on in Flex/Bison.  You should not modify this
   variable it is controled by command line options.

   The values of pDebugLogEnabled can be any bitwise or of
   the values in PPLogLevel.
*/

extern int ppDebugLogEnabled;


/* the kinds of debug messages */

typedef enum {

  DL_NO_TRACE  =0,       /* no trace, aka quiet */
  DL_INFO      =1<<0,    /* info msg only */
  DL_PARSER    =1<<1,    /* parser trace */
  DL_LEXER     =1<<2,    /* scanner trace */
  DL_VERBOSE   =1<<3,    /* very detailed tracing */
  DL_PARSER_PP =1<<4,    /* very detailed tracing */

  __last_dl__  =1<<5, 

} PPLogLevel;

/* 
   The main debug/logging method.  Essentially a wrapper for 
   printf(), but will filter the requests by the current 
   debug level that is set in ppDebugLogEnabled, which is
   ONLY initialized by the command line.

   The value of ppDebugLogEnabled is a bit mask that will
   be bitwise and'ed with the given debug level.  If the
   bit wise and result is non-zero then the printf request
   will succeed, otherwise it will be filtered out.

   An example invocation:

      debugPrint(DL_LEXER, "this is a token %s\n", token);

*/

static inline void debugPrint(PPLogLevel level, const char *fmt, ...) {

  if(!(ppDebugLogEnabled & level)) {

    /* this level is not enabled */

    return ;
  }

  va_list ap;
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

}

/* convenience methods */

static inline void debugLexer(const char *fmt, ...) {

  if(!(ppDebugLogEnabled & DL_LEXER)) {

    /* this level is not enabled */

    return ;
  }

  va_list ap;
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

}

static inline void debugParser(const char *fmt, ...) {

  if(!(ppDebugLogEnabled & DL_PARSER)) {

    /* this level is not enabled */

    return ;
  }

  va_list ap;
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

}

static inline void debugParserpp(const char *fmt, ...) {
  if(!(ppDebugLogEnabled & DL_PARSER_PP)) {

    /* this level is not enabled */

    return ;
  }

  va_list ap;
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

}


static inline void debugVerbose(const char *fmt, ...) {

  if(!(ppDebugLogEnabled & DL_VERBOSE)) {

    /* this level is not enabled */

    return ;
  }

  va_list ap;
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

}



static inline void debugInfo(const char *fmt, ...) {

  if(!(ppDebugLogEnabled & DL_INFO)) {

    /* this level is not enabled */

    return ;
  }

  va_list ap;
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

}

/* set the debug level, should only be called by the command line */

static inline void setDebugLevel(PPLogLevel level) {
  ppDebugLogEnabled=level;
}

/* test a debug level */

static inline int isDebugLevel(PPLogLevel level) {
  return ((ppDebugLogEnabled&level)!=0) ? 1 : 0;
}


static inline char *sprintfString( const char *fmt, ...) {

  char buffer[500];
  char *ret;
  int c,c1;

  va_list ap;
  va_start (ap, fmt);
  c=500;
  c1=vsnprintf (buffer, c, fmt, ap)+1;
  // if c1>500 then allocate new buffer an retry
  va_end (ap);
  
  ret=malloc(c1>c?c:c1);
  strncpy(ret,buffer,c1>c?c:c1);

  return ret;
}


#endif
