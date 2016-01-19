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
#    Foundation, Inc., 51 Franklin St,include oopli.mak

all: testoo.exe

testoo.obj: testoo.pli animal.inc
canimal.obj: canimal.pli animal.inc
cdog.obj: cdog.pli animal.inc

testoo.exe: testoo.obj canimal.obj cdog.obj
 $(LINK) testoo.obj canimal.obj cdog.obj,,,$(PLILIBS);

canimal.inc:
