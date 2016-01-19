    /*
     * Copyright 2007, 2011 Henrik Sorensen
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
#ifndef PLIPARSERSTRUCTS_H_
#define PLIPARSERSTRUCTS_H_


struct PLIParserParms
{ char * fileName ;
  FILE * inputFile;
  int debugParser;
  int debugScanner;
  int debugScannerPreprocessor;
  int preprocessOnly;
  int messageLevel;
  char * includePath; 
  char * includeSuffix;
  struct PLIMessage *msgErrors;
  struct PLIMessage *msgWarns;
  struct PLIMessage *msgInfos;
  struct ScopeNode *scopeRoot;
  void *flexScanner;
  struct PLIParserIncludeFileList *includefilelist;
};

struct PLIParserIncludeFile
{ char * name;
  int type;
#define INCTYPE_FILE 0
#define INCTYPE_PPVAR 1
  YYLTYPE *location;
  struct PLIParserIncludeFile *includedFrom;
  struct PLIParserIncludeFile *next;
  int refno; // include file enumeration 
};

struct PLIParserIncludeFileList
{
	struct PLIParserIncludeFile *head;
	struct PLIParserIncludeFile *tail;
    int count[2] ; // one count per type
};


#endif /*PLIPARSERSTRUCTS_H_*/
