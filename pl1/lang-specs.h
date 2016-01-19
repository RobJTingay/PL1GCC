/* Definitions for specs for GCC PL/I

   The format of the specs file is documented in gcc.c

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
    <http://www.gnu.org/licenses/>.  */

/*
   This is the contribution to the `default_compilers' array in GCC.c for
   gcc pl1.

   This file must compile with 'traditional', so no ANSI string concatenations

*/

{".pl1", "@gccpl1", NULL, 0, 0},
{".PL1", "@gccpl1", NULL, 0, 0},
{".pli", "@gccpl1", NULL, 0, 0},
{".PLI", "@gccpl1", NULL, 0, 0},
{"@gccpl1",
    "pl11\
       %{!Q:-quiet}\
       %{d*}\
       %{m*}\
       %{a}\
       %{g*}\
       %{O*}\
       %{W*}\
       %{w}\
       %{ansi}\
       %{v}\
       %{--help:--help}\
       %{pg:-p}\
       %{p}\
       %{f*}\
       %{pg|p:%{fomit-frame-pointer:%e-pg or -p and -fomit-frame-pointer are incompatible}}\
       %{S:%W{o*}%{!o*:-o %b.s}}\
       %{!S:-o %g.s}\
       %i\n\
       %{!S:as %a\
       %Y\
       %{c:%W{o*}%{!o*:-o %w%b%O}}\
       %{!c:-o %d%w%u%O}\
       %g.s\
       %A\n}\
       ", NULL, 0, 0
},
