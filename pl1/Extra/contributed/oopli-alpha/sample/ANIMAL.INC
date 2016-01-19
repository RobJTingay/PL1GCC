/*
    OO PL/I Precompiler Sample Project
    Copyright (C) 1994-2006 Patrick Senti, patrick.senti@gmx.net

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

 /**********************************************************/
 /* Class declaration for class Animal                     */
 /*                                                        */
 /* Implementation in Module CANIMAL                       */
 /**********************************************************/
 DCL 1 Animal:CANIMAL OBJECT,
      2 setSound PROC(sound CHAR(10)),
      2 speak PROC,
      2 mySound CHAR(10) INIT('(silence)');
