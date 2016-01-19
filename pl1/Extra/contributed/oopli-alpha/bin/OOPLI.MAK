
#   OO PL/I Precompiler Sample Project
#    Copyright (C) 1994-2006 Patrick Senti, patrick.senti@gmx.net
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#===================================================================
#
#   Sample application makefile, common definitions for the IBM C
#   compiler environment
#===================================================================
.SUFFIXES: .opl .pli .obj
#===================================================================

OOPLI   = OOPLI.CMD
PLI     = PLI

LFLAGS  = /NOE /DOSSEG /STACK:500000 /CO
CFLAGS  = (SOURCE STMT GOSTMT INSOURCE OPTIONS SNAP FLAG(E)
LINK    = LINK386  $(LFLAGS)
PLILIBS = IBMLINK + CEELINK

.opl.pli:
   -4 $(OOPLI) $*.opl $*.pli

.pli.obj:
   -4 $(PLI) $*.pli $(CFLAGS)
   @DEL *.PLI
