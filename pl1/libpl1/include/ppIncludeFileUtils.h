    /* ppIncludeFileUtils.h
     * ====================
     *
     * External definitions for handling preprocessor %INCLUDE.
     *
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
#ifndef INCLUDEFILEUTILS_H_
#define INCLUDEFILEUTILS_H_

/* ============================= */
/* Include files ...             */
extern int findFile(char *name);
extern int findEnvFile(char *env, char *name);
extern int findFileInDirList(char *dirList, char *name);
extern int findFileInDir(char *dir, char *name);
extern int fileExists(char *fullname);


#endif /*INCLUDEFILEUTILS_H_*/
