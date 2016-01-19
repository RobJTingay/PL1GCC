  /*

    GCC PL/I Compiler almost main (pl11)
    Called by GCC's toplev.c

    Copyright 2002, 2003-2006, 2011 Henrik Sorensen

    This file is part of GCC front-end for the PL/I programming language, GCC PL/I.

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

    ---------------------------------------------------------------------------

    Based on treelang/tree1.c Tim Josling 1999, 2000, 2001.

*/

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "flags.h"
#include "toplev.h"

#include "ggc.h"
#include "tree.h"
#include "diagnostic.h"
#include "version.h"

#include "pl1lang.h"
#include "pl1tree.h"

#include "opts.h"
#include "options.h"

#include "PLIParser.h"

struct PLIParserParms *pl1parserparms=NULL;


/* Linked list of symbols - all must be unique in pl1.  */

static struct prod_token_parm_item *symbol_table = NULL;

/* Language for usage for messages.  */

const char *const language_string = 
  "GNU PL/I - front end for GCC, based on TREELANG ";

/* Local prototypes.  */

void version (void);

/* Global variables.  */

extern struct cbl_tree_struct_parse_tree_top* parse_tree_top;

/*
   Options.
*/


/* Produce listing file in xml format */

unsigned int option_listing_xml = 0;

/* Warning levels.  */

/* Local variables.  */

/* This is 1 if we have output the version string.  */

static int version_done = 0;

/* Variable nesting level.  */

static unsigned int work_nesting_level = 0;

/*
   the FILE handle for the main input file, we may be 
   swapping input during the parse, so we can't just 
   close yyin when we want to.  If we error'ed out, then 
   yyin could be set to an inserted input buffer wich is
   closed off elsewhere. (double closes are bad!)
*/

static FILE *topLevel_yyin=NULL;

/* Prepare to handle switches.  */

unsigned int pl1_init_options (unsigned int argc ATTRIBUTE_UNUSED,
			       const char **argv ATTRIBUTE_UNUSED) {

  /* CL_PL1 defined by build process in options.h */

  return CL_PL1; 
}


int pl1_handle_option (size_t scode, 
		       const char *arg,
		       int value) {

  if (pl1parserparms==NULL) pl1parserparms=initPL1ParserParms();

  enum opt_code code = (enum opt_code) scode;

  switch (code)
    {
    default:
      abort();

    case OPT_v:
      if (!version_done)
	{
	  fputs (language_string, stdout);
	  fputs (version_string, stdout);
	  fputs ("\n", stdout);
	  version_done = 1;
	}
      break;

    case OPT_y:
      {
        pl1parserparms->debugParser=1;
        pl1parserparms->debugScanner=1;
        pl1parserparms->debugScannerPreprocessor=1;
        pl1parserparms->messageLevel=2; /* VERBOSE */
      }
      break;

    case OPT_fparser_trace:
      {
        pl1parserparms->debugParser=1;
        pl1parserparms->debugScannerPreprocessor=1;
      }
      break;

    case OPT_flexer_trace:
      {
        pl1parserparms->debugScanner=1;
        pl1parserparms->debugScannerPreprocessor=1;
      }
      break;

    case OPT_flisting_xml:
      option_listing_xml = value;
      break;

    case OPT_I:
      pl1parserparms->includePath=xstrdup(arg);
      break;

    case OPT_isuffix:
      pl1parserparms->includeSuffix=xstrdup(arg);
      break;

    case OPT_pp_only:
      /* we want to do just preprocessing */
      pl1parserparms->preprocessOnly=1;
      break;
    }

  return 1;
}


/* Language dependent parser setup.  */
bool pl1_init () {
  /* normal setup */

  if (pl1parserparms==NULL) pl1parserparms=initPL1ParserParms();
  

//XXX  input_line = 0; /* from input.h */

  /* Init decls etc.  */

  pl1_init_decl_processing ();

  /* This error will not happen from GCC as it will always create a
     fake input file.  */

  if(!main_input_filename || 
     main_input_filename[0] == ' ' || 
     !main_input_filename[0]) {

    if (!version_done) {
      fprintf (stderr, "No input file specified, try --help for help\n");
      exit (1);
    }

    return false;
  }


  topLevel_yyin = fopen(main_input_filename, "r");;
  if (!topLevel_yyin) {
    fprintf (stderr, "Unable to open input file %s\n", main_input_filename);
    exit (1);
  }
  pl1parserparms->inputFile = topLevel_yyin;
  pl1parserparms->fileName= xstrdup(main_input_filename);

  return true;
}

/* Language dependent wrapup.  */

void pl1_finish (void) { 

  /* TODO: free   pl1parserparms */


  if(topLevel_yyin!=NULL) {
    fclose(topLevel_yyin);
  }
 
}

/* Parse a file.  Debug flag doesn't seem to work. */

void pl1_debug() {
 /* all handled by pl1_parse function within pl1-parser.y */
};

void pl1_parse_file (int debug_flag ATTRIBUTE_UNUSED) { 

  int rc;
  
  if (pl1parserparms==NULL) pl1parserparms=initPL1ParserParms();
  
  /* turn on debugging if needed */

  pl1_debug ();

  /* invoke parser for language */
  rc=pl1_parser(pl1parserparms); /* defined in pl1-parser.y */
  
  if (rc!=0) 
      errorcount++; 
}

/* Allocate SIZE bytes and clear them.  Not to be used for strings
   which must go in stringpool.  */

void *my_malloc (size_t size) {

  void *mem;

  mem = ggc_alloc (size);

  if(!mem) {
    fprintf (stderr, "\nOut of memory\n");
    abort();
  }

  memset (mem, 0, size);

  return mem;
}

/* Look up a name in PROD->SYMBOL_TABLE_NAME in the symbol table;
   return the symbol table entry from the symbol table if found there,
   else 0.  */

struct prod_token_parm_item*
lookup_tree_name (struct prod_token_parm_item *prod)
{
  struct prod_token_parm_item *this;
  struct prod_token_parm_item *this_tok;
  struct prod_token_parm_item *tok;

  sanity_check (prod);

  tok = SYMBOL_TABLE_NAME (prod);
  sanity_check (tok);

  for (this = symbol_table; this; this = this->tp.pro.next) {

    sanity_check (this);
    this_tok = this->tp.pro.main_token;
    sanity_check (this_tok);

    if (tok->tp.tok.length != this_tok->tp.tok.length) {
      continue;
    }

    if (memcmp (tok->tp.tok.chars, this_tok->tp.tok.chars, this_tok->tp.tok.length)) {
      continue;
    }

/*
    debugParser("Found symbol %s (%i:%i) as %i \n",
		tok->tp.tok.chars,
		tok->tp.tok.location.line, tok->tp.tok.charno,
		NUMERIC_TYPE(this));
*/
    return this;
  }
/*
  debugParser("Not found symbol %s (%i:%i) as %i \n",
	      tok->tp.tok.chars,
	      tok->tp.tok.location.line,
	      tok->tp.tok.charno,stpcpy
	      tok->type);
*/
  return NULL;
}

/* Insert name PROD into the symbol table.  Return 1 if duplicate, 0 if OK.  */

int
insert_tree_name (struct prod_token_parm_item *prod)
{
  struct prod_token_parm_item *tok;
  tok = SYMBOL_TABLE_NAME (prod);
  sanity_check (prod);
  if (lookup_tree_name (prod))
    {
//XXX      fprintf (stderr, "%s:%i:%i duplicate name %s\n",
//XXX	       tok->tp.tok.location.file, tok->tp.tok.location.line,
//XXX               tok->tp.tok.charno, tok->tp.tok.chars);
      errorcount++;
      return 1;
    }
  prod->tp.pro.next = symbol_table;
  NESTING_LEVEL (prod) = work_nesting_level;
  symbol_table = prod;
  return 0;
}

/* Create a struct productions of type TYPE, main token MAIN_TOK.  */

struct prod_token_parm_item *
make_production (int type, struct prod_token_parm_item *main_tok)
{
  struct prod_token_parm_item *prod;
  prod = my_malloc (sizeof (struct prod_token_parm_item));
  prod->category = production_category;
  prod->type = type;
  prod->tp.pro.main_token = main_tok;
  return prod;
}

/* Abort if ITEM is not a valid structure, based on 'category'.  */

void
sanity_check (struct prod_token_parm_item *item)
{
  switch (item->category)
    {
    case   token_category:
    case production_category:
    case parameter_category:
      break;

    default:
      abort ();
    }
}

/* New garbage collection regime see gty.texi.  */
//#include "debug.h" /* for debug_hooks, needed by gt-treelang-treetree.h */
//#include "gt-pl1-pl11.h"
//#include "gt-pl1-pl11.h"
#include "gtype-pl1.h"
