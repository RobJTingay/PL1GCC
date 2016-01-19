/* Definitions for switches for GCC PL/I.
   Copyright 2002, 2003-2004, 2011 Henrik Sorensen

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
  */

DEFINE_LANG_NAME ("pl1")
     
/* This is the contribution to the `lang_options' array in gcc.c for
  pl1gcc.  */
 {"-fparser-trace", N_("(debug) trace parsing process")},
 {"-flexer-trace", N_("(debug) trace lexical analysis")},
 {"-isuffix", N_("suffix for included files, eg -isuffix .incl.pl1")},
 {"-pp-only", N_("Do preprocessing only.")},

 


