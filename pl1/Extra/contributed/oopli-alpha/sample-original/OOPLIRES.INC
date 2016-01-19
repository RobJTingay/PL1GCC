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

 DCL 1 entryStruct BASED,
      2 callEntry ENTRY,
      2 funcEntry ENTRY RETURNS(PTR);

 DCL 1 varStruct BASED,
      2 className VAR CHAR(35),
      2 superClassName VAR CHAR(35),
      2 varMem PTR,
      2 pNext  PTR;

 DCL pEntryStruct PTR STATIC INIT(NULL());
 IF pEntryStruct = NULL() THEN
  ALLOCATE entryStruct SET(pEntryStruct);

 /*
   allg. in OOPLIResolve 

   search in oopliMethodTable for className/memberName and return pointer to
   entry if found. If none is found this does not mean that the class does
   not exist and the member could be in a super class so look the class up
   in the oopliClassTable and see if we have a superclass. Then search
   oopliMethodTable again

 */


 OOPLIResolve: PROC(inClassName, inMemberName) RETURNS(PTR);
  DCL inClassName  VAR CHAR(35);
  DCL inMemberName VAR CHAR(35);
  
  DCL i BIN FIXED(31);
  DCL j BIN FIXED(31);
  DCL superClass VAR CHAR(35) INIT('');
  DCL fError BIT(1) INIT('0'B);
  DCL fFound BIT(1) INIT('0'B);
  DCL fClassFound BIT(1) INIT('0'B);
  DCL className VAR CHAR(35);
  DCL memberName VAR CHAR(35);
 
  ON ERROR SNAP
    BEGIN;
     ON ERROR SYSTEM;
     dcl i bin fixed(31);

     PUT SKIP LIST('could not find method >' || memberName
                   || '< in class >' || className || '<');
     put skip list(oopliMethodCount);
     do i = 1 to oopliMethodCount;
      put skip list(i,'>' ||oopliMethodTable(i).className
                 || '< >'||oopliMethodTable(i).methodName
                 || '<');
     END;
     SIGNAL ERROR;
    END;

  i = 1;
  className = inClassName;
  memberName = inMemberName;

  DO WHILE(^fFound & ^fError);
   IF className = oopliMethodTable(i).className THEN
   DO;
    superClass = oopliMethodTable(i).superClassName;
    fClassFound = '1'B;

    IF memberName = oopliMethodTable(i).methodName THEN
    DO;
     fFound = '1'b;
     IF oopliMethodTable(i).functionCall THEN
      pentryStruct->entryStruct.funcEntry =
                        oopliMethodTable(i).methodFuncEntry;
     ELSE
      pentryStruct->entryStruct.CallEntry =
                        oopliMethodTable(i).methodCallEntry;
    END;
   END;

   i = i + 1;
   IF i > oopliMethodCount THEN
   DO;
    IF ^fClassFound THEN          /* class not found? */
    DO;
     DO j = 1 TO oopliClassCount;
      IF className = oopliClassTable(j).className THEN
      DO;
       superClass = oopliClassTable(j).superClassName;
       IF oopliClassTable(j).fExternal THEN
       DO;
        pentryStruct = oopliClassTable(j).classEntry(className,
                                                  memberName);
        fFound = '1'B;
       END;
      END;
     END;
    END;

    IF superClass ^= '' THEN
    DO;
     className = superClass;
     fClassFound = '0'B;
     i = 1;
    END;
    ELSE
     fError = '1'b;
   END;
  END; /* while */

  IF fFound THEN
   RETURN(pentryStruct);
  ELSE
   SIGNAL ERROR;
 END; /* OOPLIResolve */
