/*
    OO PL/I Precompiler  
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

/*****************************************************************************/
/* OOPLI - PL/I precompiler for object oriented code            Alpha, r000.1*/
/*                                                                           */
/* Syntax: OOPLI infile outfile                                              */
/*                                                                           */
/* Software ...... PL/I, REXX/2, OS/2 or MVS                                 */
/* Author ........ Patrick Senti, PSEN@CHVM1                                 */
/* Date .......... Dec 94                                                    */
/*                                                                           */
/* (c) 1994 IBM Corp, written by Patrick Senti, IBM Switzerland, Appl.Devlpmt*/
/*****************************************************************************/

PARSE ARG inFile outFile

ADDRESS CMD "@IF EXIST" outFile "@ERASE" outFile

SAY 'Object Oriented PL/I (c) 1994 IBM Corporation'
SAY '(c) 1994-2006 Patrick Senti'
SAY 'licensed under GNU GPL v2, see LICENSE file'
SAY 'written by Patrick Senti, patrick.senti@gmx.net'
SAY 'Alpha Release 0.001'
SAY ''

MAIN:
 CALL Init
 CALL ReadFile inFile
 CALL ParseInput

 DO i = 1 TO x.msg.0
  SAY x.msg.i.location x.msg.i.severity x.msg.i.text
 END

 IF \x.severity THEN
  DO i = 1 to x.output.0
   CALL LineOut outFile, " /*"x.output.i.inLineNo"*/ "x.output.i
  END
 ELSE
  DO
   SAY 'OOPLI: output not written, aborted due to one ore more errors.'
   EXIT 8
  END
EXIT


Init:
 PROCEDURE EXPOSE x.

 x.        = ''
 x.0       = 0
 x.class.  = ''
 x.class.0 = 0
 x.class.0.methods.0 = 0
 x.output. = ''
 x.output.0 = 0
 x.var.    = ''
 x.var.0   = 0
 x.methodTableCalled = 0
 x.procNestedCount = 0
 x.callActive = 0
 x.methodActive      = 0
 x.module  = ''
 x.classesInModule = 0
 x.msg.0   = 0
 x.severity = 0

/* strcuture

 x.msg.0                     # of messages
 x.msg.#.severity            severity of message (W/E)
 x.msg.#.location            line#:word#
 x.msg.#.text                message text

 x.class.0                   # of classes
 x.class.#.fConWritten       1 = written, 0 = not written (for methods only)
 x.class.#.fDesWritten       - dito -
 x.class.0.methods.0         # of members (total for all classes)
 x.class.#.name              name of #th class
 x.class.#.module            name of module or blank for current
 x.class.#.member.0          # of members for #th class
 x.class.#.member.#.name     name of member for #th member of #th class
 x.class.#.member.#.attr     attributes for the member
 x.class.#.member.#.fWritten 1 = method written, 0 = not written (for
                             internal classes only!)

 x.var.0                     declared object variables
 x.var.#.name
 x.var.#.class
 x.var.thisVar.class         class name of <this> object variable

 x.methodTableCalled         0 = no, 1 = yes
*/
RETURN


/* this will read the input file; will read %INCLUDEd files as well */
ReadFile:
 PROCEDURE EXPOSE x.
 PARSE ARG inFile
 inFile = Strip(inFile)

 lineCount = 0

 DO WHILE Lines(inFile) > 0
  curLine = MakeWords(LineIn(inFile))
  lineCount = lineCount + 1

  IF Pos('%INCLUDE', curLine) > 0 THEN
  DO
   PARSE VALUE curLine WITH '%INCLUDE' incFile ';'
   IF Pos('(', incFile) > 0 THEN
    PARSE VAR incFile '('incFile')'
   CALL ReadFile incFile
  END
  ELSE
   DO
    i = x.0 + 1
    x.0 = i
    x.i = curLine
    x.i.file = inFile
    x.i.line = lineCount
   END
 END
RETURN

/* this is the input file parser. It is 'word' based and therefore has some */
/* limitations; a token based parser/interperter would probably be more     */
/* effective                                                                */
ParseInput:
 PROCEDURE EXPOSE x.
 j = 1
 i = 1

 DO WHILE i <= x.0
   curLine = x.i
   curWord = Translate(Strip(SubWord(curLine,j,1)))

   SELECT
    WHEN curWord = 'DCL' THEN
      DO
       PARSE VALUE BuildDeclare(i,j+1) WITH i j fDone
      END
    WHEN (DataType(curWord) = 'CHAR' & NextWord(i,j) = ':') THEN
    DO
     x.procNestedCount = x.procNestedCount + 1
     IF x.procNestedCount = 1 THEN
      PARSE VALUE BuildMainProcedure(i,j) WITH i j x.fmainModule
     ELSE
      PARSE VALUE BuildProcedure(i,j) WITH i j x.methodActive
    END
    WHEN (curword = 'END' & NextWord(i,j) = ';') THEN
     DO
      IF x.procNestedCount > 1 THEN
      DO
       x.procNestedCount = x.procNestedCount - 1
       x.var.thisVar.class = ''
       x.methodActive = 0
       CALL AddOutput i, j, 'END;', 'T'                  /* write END; anyway! */
       PARSE VALUE SetLineWordCount(i,j,0,+2) WITH i j
      END
      ELSE
       DO
        CALL WriteUndefConDestructors
        CALL CheckMethods
        CALL WriteMethodTable
        IF \x.methodTableCalled THEN
        DO
         CALL AddOutput 0, 0, "%INCLUDE OOPLIRES;", 'T'
         CALL AddOutput 0, 0, "CALL SetupMethodTable;", 'T'
         x.methodTableCalled = 1
         IF \x.fmainModule THEN
          CALL AddOutput 0, 0, "RETURN(OOPLIResolve(className, memberName));", 'T'
        END
        CALL AddOutput i, j, 'END;', 'T'                 /* write END; anyway! */
        LEAVE
       END
     END
    WHEN curword = 'CALL' THEN
       DO
        x.callActive = 1
        CALL AddOutput i, j, 'CALL', 'T'
        PARSE VALUE SetLineWordCount(i,j,0,+1) WITH i j
       END
    WHEN IsObjectVar(curWord) THEN
        DO
         PARSE VALUE curWord WITH varName'.'memberName

         varName   = Translate(Strip(varName))
         className = GetVarClass(varName)

         IF className <> '' THEN
         DO
          IF Strip(memberName) = '' THEN
          DO
           CALL AddOutput i,j,, 'I'
           PARSE VALUE SetLineWordCount(i,j,0,+1) WITH i j
          END
          ELSE
           IF IsMemberMethod(className, memberName) THEN
           DO
            IF NextWord(i,j) = '(' THEN
            DO
             PARSE VALUE SetLineWordCount(i,j,0,+2) WITH i j
             PARSE VALUE WordsUpTo(')', i, j) WITH i j callArgs
             callArgs = Strip(callArgs)
            END
            ELSE
             DO
              PARSE VALUE SetLineWordCount(i,j,0,+1) WITH i j
              callArgs = ''
             END

            IF x.callActive THEN
            DO
             CALL WriteOOPLIResolveCall className, memberName, varName, callArgs
             x.callActive = 0
            END
            ELSE
             CALL WriteOOPLIResolveCall className, memberName, varName, callArgs, 1
           END
           ELSE
            DO
             PARSE VALUE SetLineWordCount(i,j,0,+1) WITH i j
             IF Translate(varName) = 'THIS' THEN
              varName = 'varThis'
             CALL AddOutput  0, 0, varName"->varStruct.varMem->"||,
                               className"."memberName, 'T'
            END
           END
           ELSE
            DO
             CALL AddOutput i,j,, 'I'
             PARSE VALUE SetLineWordCount(i,j,0,+1) WITH i j
             IF varName = 'THIS',
              | varName = 'PARENT' THEN
              CALL AddMessage "E", i,j, "class for" varName "not found."
            END
        END
    WHEN curWord = "'" THEN                   /* make sure we overread */
      DO                                      /* strings in '...'      */
       PARSE VALUE WordsUpTo("'", i,j) WITH i j string
       CALL AddOutput 0, 0, "'"string"'", 'T'
      END
    WHEN curWord = '"' THEN                   /* make sure we overread */
      DO                                      /* strings in "..."      */
       PARSE VALUE WordsUpTo('"', i,j) WITH i j string
       CALL AddOutput 0, 0, '"'string'"', 'T'
      END
    WHEN curWord = '/' & NextWord(i,j) = '*' THEN /* make sure we overread */
     DO                                           /* comments              */
      DO UNTIL (NextWord(i,j-1) = '/')            /* not perfect...        */
       PARSE VALUE WordsUpTo('*',i,j) WITH i j .
      END
      PARSE VALUE SetLineWordCount(i,j,0,+1) WITH i j
     END
    OTHERWISE                                     /* none of all, so just  */
     CALL AddOutput i, j,, 'I'                    /* get allong and output */
     x.callActive = 0                             /* the word "as is"      */
     PARSE VALUE SetLineWordCount(i,j,0,+1) WITH i j
   END
  END
RETURN


/*****************************************************************************/
/* lineNo wordNo = BuildDeclare(lineNo, wordNo)                              */
/*                                                                           */
/* PARAMETERS        lineNo, i             # of current line                 */
/*                   wordNo, i             # of current word                 */
/*                                                                           */
/* tries to find OBJECT definition and, if found, processes this def. Calls  */
/* AddNewClass and AddClassMember in order to build object definition        */
/*****************************************************************************/
BuildDeclare:
 PROCEDURE EXPOSE x.
 PARSE ARG lineNo, wordNo

 IF IsObjectDeclare(lineNo, wordNo) THEN      /* is it a new OBJECT decl? */
 DO
  className      = ''
  superClassName = ''
  moduleName     = ''

  DO WHILE delim <> ';'
   PARSE VALUE GetDeclareLevel(lineNo, wordNo) WITH lineNo wordNo delim thisLevel

   PARSE VALUE thisLevel WITH levelNo levelDef

   IF DataType(levelNo,N) &,                   /* level numbering found? */
      Strip(levelDef) \= '' THEN
   DO
    IF Pos('OBJECT', levelDef) >0 THEN   /* get class and superclass */
    DO
     PARSE VALUE levelDef WITH className 'OBJECT' '(' superClassName ')'
     className = Strip(className)

     IF Pos(':', className) > 0 THEN
      PARSE VALUE className WITH className ':' moduleName

     IF Pos(':', superClassName) > 0 THEN
     DO
      PARSE VAR superClassName superClassName ':' superClassModuleName
      i = GetClassOrdinal(superClassName)
      IF i > 0 THEN
       x.class.i.module = superClassModuleName
     END

     CALL AddNewClass className, superClassName, moduleName, lineNo, wordNo
    END
    ELSE
     IF className <> '' THEN
     DO
      PARSE VALUE levelDef WITH levelName levelAttribute
      CALL AddClassMember className, levelName, levelAttribute
     END
   END
   ELSE
    DO
     CALL AddMessage 'W', lineNo, wordNo,,
            "level # expected in OBJECT declaration"
     IF delim <> ';' THEN
      PARSE VALUE WordsUpTo(';', lineNo, wordNo) WITH lineNo wordNo .
     RETURN lineNo wordNo
    END
  END
  IF delim = ';' THEN                           /* declare end delim? */
  DO
   CALL WriteClassDeclare className
   RETURN lineNo wordNo                         /* get back!          */
  END
 END

 IF IsObjectVarDeclare(lineNo, wordNo) THEN     /* is it an object var def? */
 DO
  PARSE VALUE GetObjectVar(lineNo, wordNo) WITH,
                                     lineNo wordNo varName className super


  CALL AddObjectVar varName, className

  IF \x.methodTableCalled THEN                /* method table called already? */
  DO
   CALL AddOutput 0, 0, "%INCLUDE OOPLIRES;", 'T';
   CALL AddOutput 0, 0, "CALL SetupMethodTable;", 'T';
   x.methodTableCalled = 1
  END
                                              /* generate allocations for var */
  CALL AddOutput lineNo, 0, 'DCL' varName 'PTR;', 'T'
  CALL AddOutput lineNo, 0, 'ALLOCATE varStruct SET('varName');', 'T'
  CALL AddOutput lineNo, 0, varName"->varStruct.varMem = NULL();", 'T'
  CALL AddOutput lineNo, 0, varName"->varStruct.className = '"className"';", 'T'
  CALL AddOutput lineNo, 0, varName"->varStruct.superClassName = '"super"';", 'T'
  CALL AddOutput lineNo, 0, varName"->varStruct.pNext = NULL();", 'T'

  CALL AddOutput 0, 0, 'CALL', 'T'
  CALL WriteOOPLIResolveCall className, 'C'className, varName
  CALL AddOutput 0, 0, ';', 'T'
 END
 ELSE                       /* not an OBJECT declaration nor object variable */
  DO                        /* so just add the whole thing to the output     */
   CALL AddOutput lineNo, wordNo-1,, 'I'
   CALL AddOutput lineNo, wordNo,, 'I'
   PARSE VALUE SetLineWordCount(lineNo, wordNo, 0, +1) WITH lineNo wordNo
  END
RETURN lineNo wordNo


/*****************************************************************************/
/* lineNo wordNo = BuildProcedure(lineNo, wordNo)                            */
/*                                                                           */
/* PARAMETERS        lineNo, i             # of current line                 */
/*                   wordNo, i             # of current word                 */
/*                                                                           */
/* see if we have a procedure for an object and if so, get the procedure     */
/* name and attributes / set up a member in x.class.#.member structure       */
/* changes and adds procedure header to contain the <this> parameter and the */
/* <varThis> variable as the class variable mem resolution                   */
/*****************************************************************************/
BuildProcedure:
 PROCEDURE EXPOSE x.
 PARSE ARG lineNo, wordNo

 curWord = SubWord(x.lineNo,wordNo,1)
 argList = ''
 rest    = ''

 IF Pos('.', curWord) > 0 THEN               /* great, we have found an entry! */
 DO
  PARSE VALUE curWord WITH className'.'memberName ':'

  className = Translate(Strip(className))
  memberName = Translate(Strip(memberName))

  IF IsClassDefined(className) &,
     IsMemberOf(memberName, className) THEN
  DO
   PARSE VALUE SetLineWordCount(lineNo, wordNo, 0, +2) WITH lineNo wordNo
   curWord = SubWord(x.lineNo,wordNo,1)

   IF Pos('PROC', curWord) > 0 THEN        /* parse PROC attribute */
   DO
    IF NextWord(lineNo, wordNo) = '(' THEN
    DO
     PARSE VALUE SetLineWordCount(lineNo, wordNo, 0, +2) WITH lineNo wordNo
     PARSE VALUE WordsUpTo(')', lineNo, wordNo) WITH lineNo wordNo argList
     PARSE VALUE WordsUpTo(';', lineNo, wordNo) WITH lineNo wordNo rest
    END
    ELSE
     DO
      PARSE VALUE SetLineWordCount(lineNo, wordNo, 0, +1) WITH lineNo wordNo
      PARSE VALUE WordsUpTo(';', lineNo, wordNo) WITH lineNo wordNo rest
     END
   END
   ELSE
    PARSE VALUE WordsUpTo(';', lineNo, wordNo) WITH lineNo wordNo rest

   x.methodActive.class = className
   x.methodActive.member = memberName

   i = GetClassOrdinal(className)             /* set method-written flag */
   j = GetMemberOrdinal(className, memberName)
   x.class.i.member.j.fWritten = 1

   IF argList <> '' THEN                       /* generate new procedure */
    procHead = "PROC(this,"argList")"          /* definition             */
   ELSE
    procHead = "PROC(this)"

   procHead = procHead rest';'

   CALL AddOutput 0, 0, className||MemberName':' procHead, 'T'

   CALL AddOutput 0, 0, "DCL this PTR;", 'T'
   CALL AddOutput 0, 0, "DCL varThis PTR INIT(this);", 'T'

   CALL AddOutput 0, 0, "DO WHILE(varThis->varStruct.className ^= '"className"');", 'T'
   CALL AddOutput 0, 0, "  varThis = varThis->varStruct.pNext;", 'T'
   CALL AddOutput 0, 0, "END;", 'T'

   x.var.thisVar.class = Translate(className)

   RETURN lineNo WordNo 1
  END /* not defined */
  ELSE
   DO
    IF IsClassDefined(className) THEN
     CALL AddMessage "W", lineNo, wordNo,,
             memberName "is not declared in class" className"."
    ELSE
     CALL AddMessage "E", lineNo, wordNo,,
             "Class" className "is not declared."
    CALL AddOutput lineNo, wordNo,, 'I'
   END
 END
 ELSE /* probably no method member */
  CALL AddOutput lineNo, wordNo,, 'I'
RETURN lineNo WordNo+1 1


/*****************************************************************************/
/* lineNo wordNo = BuildMainProcedure(lineNo, wordNo)                        */
/*                                                                           */
/* PARAMETERS        lineNo, i             # of current line                 */
/*                   wordNo, i             # of current word                 */
/*                                                                           */
/* called when the first procedure in header in the module is found. Checks  */
/* if OPTIONS(MAIN) is set; if not generates parameters necessary for        */
/* external object resolution. Attributes other than PROC and OPTIONS are    */
/* skipped and will not be added to the output. This is short of an error... */
/*****************************************************************************/
BuildMainProcedure:
 PROCEDURE EXPOSE x.
 PARSE ARG lineNo, wordNo

 PARSE VALUE WordsUpTo(':', lineNo, wordNo) WITH lineNo wordNo procName
 x.module = Translate(Strip(procName))

 fmainModule = 0
 argList = ''
 optionList = ''

 DO WHILE Strip(SubWord(x.lineNo, wordNo, 1)) <> ';'       /* read up to ; */
  curWord = Translate(Strip(SubWord(x.lineNo,wordNo,1)))

  IF Pos('PROC', curWord) > 0,                  /* procedure found */
   & NextWord(lineNo, wordNo) = '(' THEN
  DO                                            /* get arg. list   */
   PARSE VALUE SetLineWordCount(lineNo,wordNo,0,+2) WITH lineNo wordNo
   PARSE VALUE WordsUpTo(')', lineNo, wordNo) WITH lineNo wordNo argList
  END
  ELSE
   IF Pos('OPTIONS', curWord) > 0,            /* get options list */
    & NextWord(lineNo, wordNo) = '(' THEN
   DO
    PARSE VALUE SetLineWordCount(lineNo,wordNo,0,+2) WITH lineNo wordNo
    PARSE VALUE WordsUpTo(')', lineNo, wordNo) WITH lineNo wordNo optionList

    IF Pos('MAIN', optionList) > 0 THEN       /* is it a MAIN pgm? */
     fmainModule = 1
   END
   ELSE                                       /* none at all, overread... */
    PARSE VALUE SetLineWordCount(lineNo,wordNo,0,+1) WITH lineNo wordNo
 END

 IF fmainModule THEN                          /* build procedure header   */
 DO
  IF argList = '' THEN
   CALL AddOutput 0, 0, procName': PROC', 'T'
  ELSE
   CALL AddOutput 0, 0, procName': PROC('argList')', 'T'
  IF optionList <> '' THEN
   CALL AddOutput 0, 0, 'OPTIONS('optionList')', 'T'
  CALL AddOutput 0, 0, ';', 'T'
 END
 ELSE
  DO
   CALL AddOutput 0, 0, procName': PROC(className, memberName) RETURNS(PTR);', 'T'
   CALL AddOutput 0, 0, 'DCL (className, memberName) VAR CHAR(35);', 'T'
  END
RETURN SetLineWordCount(lineNo,wordNo,0,+1) fmainModule


/*****************************************************************************/
/* fTrue = IsObjectDeclare(lineNo, wordNo)                                   */
/*                                                                           */
/* PARAMETERS        lineNo, i             # of current line                 */
/*                   wordNo, i             # of current word                 */
/*                                                                           */
/* returns TRUE if current DCL is for an OBJECT; FALSE if not or if end      */
/* delim (;) is found before OBJECT keyword                                  */
/*****************************************************************************/
IsObjectDeclare:
 PROCEDURE EXPOSE x.
 PARSE ARG lineNo, wordNo

 PARSE VALUE WordsUpTo(';', lineNo, wordNo) WITH lineNo wordNo testWord

 IF Pos('OBJECT', testWord) > 0 THEN                 /* OBJECT keyword found */
  RETURN 1
 ELSE
  RETURN 0
RETURN 0


/*****************************************************************************/
/* fTrue = IsObjectVarDeclare(lineNo, wordNo)                                */
/*                                                                           */
/* PARAMETERS        lineNo, i             # of current line                 */
/*                   wordNo, i             # of current word                 */
/*                                                                           */
/* returns TRUE if current DCL has an attribute that is an object class      */
/* does not (yet) test for ; or , in declare statement. Does not (yet)       */
/* recognize structure declares                                              */
/*****************************************************************************/
IsObjectVarDeclare:
 PROCEDURE EXPOSE x.
 PARSE ARG lineNo, wordNo

 PARSE VALUE WordsUpTo(';', lineNo, wordNo) WITH lineNo wordNo varName className .

 className = Strip(Translate(className, '', ';'))
RETURN IsClassDefined(className)



/*****************************************************************************/
/* fTrue = IsClassDefined(className)                                         */
/*                                                                           */
/* PARAMETERS        className, i          name of class to look up          */
/*                                                                           */
/* returns TRUE if className can be found in class name structure            */
/*****************************************************************************/
IsClassDefined:
 PROCEDURE EXPOSE x.
 PARSE ARG className

 IF Translate(className) = 'THIS' THEN
  className = x.var.thisVar.class

 DO i = 1 TO x.class.0
  IF x.class.i.name = Translate(className) THEN
   RETURN 1
 END
RETURN 0



/*****************************************************************************/
/* fTrue = IsObjectVarDeclare(varName)                                       */
/*                                                                           */
/* PARAMETERS        varName, i            name of variable                  */
/*                                                                           */
/* returns TRUE if variable <varName> has *previously* been defined as an    */
/* object variable (other than PLI forward references are not allowed!)      */
/* if <varName> is THIS allways returns TRUE; varName can be a composite     */
/* name (in the form var.member) or a simple variable ref                    */
/*****************************************************************************/
IsObjectVar:
 PROCEDURE EXPOSE x.
 PARSE ARG varName

 IF Pos('.', varName) > 0 THEN
 DO
  PARSE VALUE varName WITH varName'.'
 END

 IF Translate(varName) = 'THIS',
  | Translate(varName) = 'PARENT' THEN
  RETURN 1

 DO i = 1 TO x.var.0
  IF Translate(x.var.i.name) = Translate(varName) THEN
   RETURN 1
 END
RETURN 0



/*****************************************************************************/
/* fTrue = IsMemberOf(memberName, className)                                 */
/*                                                                           */
/* PARAMETERS        memberName, i         name of member                    */
/*                   className, i          name of class                     */
/*                                                                           */
/* returns TRUE if <memberName> is a member of class <className>, FALSE      */
/* if not.                                                                   */
/*****************************************************************************/
IsMemberOf:
 PROCEDURE EXPOSE x.
 PARSE ARG memberName, className

 i = GetClassOrdinal(className)

 DO j = 1 TO x.class.i.member.0
  IF x.class.i.member.j.name = Translate(Strip(memberName)) THEN
   RETURN 1
 END
RETURN 0


/*****************************************************************************/
/* fTrue = IsMemberMethod(memberName, className)                             */
/*                                                                           */
/* PARAMETERS        memberName, i         name of member                    */
/*                   className, i          name of class                     */
/*                                                                           */
/* returns TRUE if <memberName> is a method member of class <className>      */
/* A method member contains PROC or PROCEDURE in its attribute list          */
/*****************************************************************************/
IsMemberMethod:
 PROCEDURE EXPOSE x.
 PARSE ARG className, memberName


 i = GetClassOrdinal(Strip(className))

 IF i = 0 THEN
  RETURN 0

 DO j = 1 TO x.class.i.member.0
  IF x.class.i.member.j.name = Translate(Strip(memberName)) THEN
   IF Pos('PROC', x.class.i.member.j.attr) THEN
    RETURN 1
 END
RETURN 0


/*****************************************************************************/
/* lineNo wordNo delim thisLevel = GetDeclareLevel(lineNo, wordNo)           */
/*                                                                           */
/* PARAMETERS        lineNo, i             # of current line                 */
/*                   wordNo, i             # of current word                 */
/*                                                                           */
/* returns current declare level up to next delimiter (, or ;) lineNo and    */
/* wordNo are returned as well as the found delim; the level text is returned*/
/* in <thisLevel>                                                            */
/*****************************************************************************/
GetDeclareLevel:
 PROCEDURE EXPOSE x.
 PARSE ARG lineNo, wordNo

 delim = ';'
 thisLevel = ''

 DO WHILE(Verify(SubWord(x.lineNo,wordNo,1), ',;') > 0)
  thisLevel = thisLevel Strip(SubWord(x.lineNo,wordNo,1))
  PARSE VALUE SetLineWordCount(lineNo, wordNo,0,+1) WITH lineNo wordNo
 END

 IF Strip(SubWord(x.lineNo,wordNo,1)) = ';' THEN
  delim = ';'
 ELSE
  delim = ','

 PARSE VALUE SetLineWordCount(lineNo, wordNo,0,+1) WITH lineNo wordNo
RETURN lineNo wordNo delim thisLevel


/*****************************************************************************/
/* lineNo wordNo varName className =  GetObjectVar(lineNo, wordNo)           */
/*                                                                           */
/* PARAMETERS        lineNo, i             # of current line                 */
/*                   wordNo, i             # of current word                 */
/*                                                                           */
/* returns varName and className of current DCL stmt. does not check for     */
/* validity. Expects wordNo to be varName, wordNo+1 className!               */
/* -> to be refined                                                          */
/*****************************************************************************/
GetObjectVar:
 PROCEDURE EXPOSE x.
 PARSE ARG lineNo, wordNo

 PARSE VALUE WordsUpTo(';', lineNo, wordNo) WITH lineNo wordNo varName className .

 className = Strip(Translate(className, '', ';'))
 className = Translate(className)
 varName = Translate(varName)

 i = GetClassOrdinal(className)
 superClass = x.class.i.super
RETURN lineNo wordNo varName className superClass


/*****************************************************************************/
/* className = GetVarClass(varName)                                          */
/*                                                                           */
/* PARAMETERS        varName, i            name of variable                  */
/*                                                                           */
/* returns the name of the class that variable <varName> is declared to      */
/* belong to. Returns blank if <varName> has not been declared               */
/*****************************************************************************/
GetVarClass:
 PROCEDURE EXPOSE x.
 PARSE ARG varName

 IF Translate(varName) = 'THIS' THEN
   RETURN x.var.thisVar.class

 IF Translate(varName) = 'PARENT' THEN
  RETURN GetSuperClass(x.var.thisVar.class)

 DO i = 1 TO x.var.0
  IF x.var.i.name = Translate(Strip(varName)) THEN
   RETURN x.var.i.class
 END
RETURN ''



/*****************************************************************************/
/* ord = GetClassOrdinal(className)                                          */
/*                                                                           */
/* PARAMETERS        className, i          name of class                     */
/*                                                                           */
/* returns the ordinal value of class <className>. The ordinal is the index  */
/* of the class in the x.class.# struct                                      */
/*****************************************************************************/
GetClassOrdinal:
 PROCEDURE EXPOSE x.
 PARSE ARG className


 IF Translate(className) = 'THIS' THEN
  className = x.var.thisVar.class

 DO i = 1 TO x.class.0
  IF x.class.i.name = Translate(className) THEN
   RETURN i
 END
RETURN 0


/*****************************************************************************/
/* ord = GetMemberOrdinal(className, memberName)                             */
/*                                                                           */
/* PARAMETERS        className, i          name of class                     */
/*                                                                           */
/* returns the ordinal value of member <memberName> in class <className>     */
/* the ordinal value of a member is the index in the x.class.#.member.# str. */
/*****************************************************************************/
GetMemberOrdinal:
 PROCEDURE EXPOSE x.
 PARSE ARG className, memberName

 i = GetClassOrdinal(className)
 IF i = 0 THEN
  RETURN 0

 DO j = 1 TO x.class.i.member.0
  IF x.class.i.member.j.name = Translate(memberName) THEN
   RETURN j
 END
RETURN 0

/*****************************************************************************/
/* superClassName = GetSuperClass(className9                                 */
/*                                                                           */
/* PARAMETERS        className, i          name of class                     */
/*                                                                           */
/* returns the name of the super class or blank if class not found or        */
/* no super class exists for <className>                                     */
/*****************************************************************************/
GetSuperClass:
 PROCEDURE EXPOSE x.
 PARSE ARG className

 i = GetClassOrdinal(Translate(Strip(className)))

 IF i > 0 THEN
  RETURN x.class.i.super
RETURN ''


/*****************************************************************************/
/* CALL AddNewClass className, superClassName, lineNo, wordNo                */
/*                                                                           */
/* PARAMETERS        className, i          name of class to add              */
/*                   superClassName, i     name of superclass of this class  */
/*                   lineNo                # of current line                 */
/*                   wordNo                # of current word                 */
/*                                                                           */
/* adds a new class definition to the class table (struct x.)                */
/*****************************************************************************/
AddNewClass:
 PROCEDURE EXPOSE x.
 PARSE ARG className, superClassName, moduleName, lineNo, wordNo

 i = x.class.0 + 1
 x.class.0 = i

 x.class.i.name     = Translate(Strip(className))
 x.class.i.super    = Translate(Strip(superClassName))
 x.class.i.pos      = lineNo wordNo
 x.class.i.member.  = ''
 x.class.i.member.0 = 0
 IF moduleName <> '' THEN
  x.class.i.module  = Translate(Strip(moduleName))
 ELSE
  x.class.i.module  = x.module
 x.class.i.fConWritten = 0
 x.class.i.fDesWritten = 0

 IF x.class.i.module = x.module THEN
  x.classesInModule = x.classesInModule + 1
RETURN


/*****************************************************************************/
/* CALL AddClassMember className, memberName, memberAttribute                */
/*                                                                           */
/* PARAMETERS        className, i          name of class to add member for   */
/*                   memberName, i         name of member                    */
/*                                                                           */
/* adds memberName to class className (structure x.class.#.member.#)        */
/*****************************************************************************/
AddClassMember:
 PROCEDURE EXPOSE x.
 PARSE ARG className, memberName, memberAttribute

 i = GetClassOrdinal(className)
 j = x.class.i.member.0 + 1
 x.class.i.member.0 = j

 x.class.i.member.j.name = Translate(memberName)
 x.class.i.member.j.attr = Translate(memberAttribute)
 x.class.i.member.j.fWritten = 0

 IF Pos('PROC', Translate(x.class.i.member.j.attr)) > 0 THEN
  x.class.0.methods.0 = x.class.0.methods.0 + 1
RETURN


/*****************************************************************************/
/* CALL AddObjectVar varName, className                                      */
/*                                                                           */
/* PARAMETERS        varName, i            name of object variable           */
/*                   className, i          name of classr                    */
/*                                                                           */
/* adds variable varName of class className to variable name space           */
/*****************************************************************************/
AddObjectVar:
 PROCEDURE EXPOSE x.
 PARSE ARG varName, className

 i = x.var.0 + 1
 x.var.0 = i
 x.var.i.name = Translate(varName)
 x.var.i.class = Translate(className);
RETURN


AddMessage:
 PROCEDURE EXPOSE x.
 PARSE ARG type, lineNo, wordNo, msgText

 IF lineNo = 0 THEN
  lineNo = 1

 i = x.msg.0 + 1
 x.msg.0 = i
 x.msg.i.severity = type
 x.msg.i.location = x.lineNo.file'('x.lineNo.line':'wordNo')'
 x.msg.i.text = msgText
 IF type = 'E' THEN
  x.severity = 1
RETURN


/*****************************************************************************/
/* CALL AddOutput(lineNo, wordNo, text, addType)                             */
/*                                                                           */
/* PARAMETERS        lineNo, i             input lineNo                      */
/*                   wordNo, i             input wordNo in lineNo            */
/*                   text  , i             text to be added to output        */
/*                   addType, i (I / T)    type of addition                  */
/*                                                                           */
/* for addType I, the word at lineNo/wordNo is added to the output and is    */
/* appended to the most recently added line if lineNo matches. For addType   */
/* T a new line is generated and <text> is written into this line            */
/*****************************************************************************/
AddOutput:
 PROCEDURE EXPOSE x.
 PARSE ARG lineNo, wordNo, text, addType

 IF addType = 'I' THEN
 DO
  i = x.output.0
  IF x.output.i.inLineNo < lineNo,
   | i = 0 THEN
  DO
   i = x.output.0 + 1
   x.output.0 = i
   x.output.i.inLineNo = lineNo
   x.output.i = ''
  END

  IF x.output.i = '' THEN
   x.output.i = SubWord(x.lineNo, wordNo, 1)
  ELSE
   x.output.i = x.output.i SubWord(x.lineNo, wordNo, 1)
 END
 ELSE
  IF addType = 'T' THEN
  DO
   i = x.output.0 + 1
   x.output.0 = i
   x.output.i.inLineNo = lineNo
   x.output.i = text
  END
RETURN


CheckMethods:
 PROCEDURE EXPOSE x.
 DO i = 1 TO x.class.0
  DO j = 1 TO x.class.i.member.0
   IF x.class.i.module = x.module,
    & IsMemberMethod(x.class.i.name, x.class.i.member.j.name),
    & \x.class.i.member.j.fWritten THEN
   DO
    CALL AddMessage "E", 0,0, "Method" x.class.i.member.j.name,
                              "for class" x.class.i.name "not found."
   END
  END
 END
RETURN


/*****************************************************************************/
/* CALL WriteClassDeclare(className)                                         */
/*                                                                           */
/* PARAMETERS        className, i          name of class                     */
/*                                                                           */
/* the necessary *data* declarations (without method members!) is written    */
/* to the output (using AddOutput) for class <className>. If the class is    */
/* declared as external an external entry is also declared.                  */
/*****************************************************************************/
WriteClassDeclare:
 PROCEDURE EXPOSE x.
 PARSE ARG className

 i = GetClassOrdinal(className)

 IF i > 0 THEN
 DO
  CALL AddOutput 0, 0, 'DCL 1' className 'BASED', 'T'

  DO j = 1 TO x.class.i.member.0
   IF Pos('PROC', Translate(x.class.i.member.j.attr)) = 0 THEN
    CALL AddOutput 0, 0, ',   2' x.class.i.member.j.name,
                                 x.class.i.member.j.attr, 'T'
  END
  CALL AddOutput 0, 0, ';', 'T'

  IF x.class.i.module <> '',
   & x.module <> x.class.i.module THEN
   CALL AddOutput 0, 0, 'DCL' x.class.i.module,
                   'EXT ENTRY(VAR CHAR(35), VAR CHAR(35)) RETURNS(PTR);', 'T'
 END
RETURN


/*****************************************************************************/
/* CALL WriteMethodTable                                                     */
/*                                                                           */
/* PARAMETERS                                                                */
/*                                                                           */
/* the oopliMethodTable and oopliClassTable array declaration and the        */
/* SetupMethodTable procedure are written to the output as required.         */
/*****************************************************************************/
WriteMethodTable:
 PROCEDURE EXPOSE x.

 CALL AddOutput 0, 0, 'DCL 1 oopliMethodTable('x.class.0.methods.0') STATIC,', 'T'
 CALL AddOutput 0, 0, '   2 className      VAR CHAR(35),', 'T'
 CALL AddOutput 0, 0, '   2 superClassName VAR CHAR(35),', 'T'
 CALL AddOutput 0, 0, '   2 methodName     VAR CHAR(35),', 'T'
 CALL AddOutput 0, 0, '   2 functionCall BIT(1),', 'T'
 CALL AddOutput 0, 0, '   2 methodCallEntry  ENTRY,', 'T'
 CALL AddOutput 0, 0, '   2 methodFuncEntry  ENTRY RETURNS(PTR);', 'T'
 CALL AddOutput 0, 0, 'DCL oopliMethodCount BIN FIXED(31) INIT('x.class.0.methods.0') STATIC;', 'T'

 CALL AddOutput 0, 0, 'DCL 1 oopliClassTable('x.class.0') STATIC,', 'T'
 CALL AddOutput 0, 0, '     2 className    VAR CHAR(35),', 'T'
 CALL AddOutput 0, 0, '     2 superClassName VAR CHAR(35),', 'T'
 CALL AddOutput 0, 0, "     2 fExternal BIT(1),", 'T'
 CALL AddOutput 0, 0, '     2 classEntry ENTRY(VAR CHAR(35), VAR CHAR(35)) RETURNS(PTR);', 'T'
 CALL AddOutput 0, 0, 'DCL oopliClassCount BIN FIXED(31) INIT('x.class.0') STATIC;', 'T'

 cur = 0

 CALL AddOutput 0, 0, "SetupMethodTable: PROC;", 'T'

 DO i = 1 TO x.class.0
  DO j = 1 TO x.class.i.member.0
   IF x.class.i.module = x.module THEN
    IF Pos('PROC', Translate(x.class.i.member.j.attr)) > 0 THEN
    DO
     cur = cur + 1
     CALL AddOutput 0, 0, "oopliMethodTable("cur").className='"||,
                                                x.class.i.name"';", 'T'
     CALL AddOutput 0, 0, "oopliMethodTable("cur").superClassName='"||,
                                               x.class.i.super"';", 'T'
     CALL AddOutput 0, 0, "oopliMethodTable("cur").methodName='"||,
                                               x.class.i.member.j.name"';", 'T'
     IF Pos('RETURNS', Translate(x.class.i.member.j.attr)) > 0 THEN
     DO
      CALL AddOutput 0, 0, "oopliMethodTable("cur").methodFuncEntry="||,
                                 x.class.i.name||x.class.i.member.j.name";", 'T'
      CALL AddOutput 0, 0, "oopliMethodTable("cur").functionCall= '1'B;", 'T'
     END
     ELSE
      DO
       CALL AddOutput 0, 0, "oopliMethodTable("cur").methodCallEntry=",
                                  x.class.i.name||x.class.i.member.j.name";", 'T'
       CALL AddOutput 0, 0, "oopliMethodTable("cur").functionCall= '0'B;", 'T'
      END
    END
  END
 END

 DO i = 1 TO x.class.0
  CALL AddOutput 0, 0, "oopliClassTable("i").className='"x.class.i.name"';", 'T'
  CALL AddOutput 0, 0, "oopliClassTable("i").superClassName='"x.class.i.super"';", 'T'
  IF x.class.i.module <> x.module THEN
  DO
   CALL AddOutput 0, 0, "oopliClassTable("i").fExternal = '1'B;", 'T'
   CALL AddOutput 0, 0, "oopliClassTable("i").classEntry =" x.class.i.module";", 'T'
  END
  ELSE
   CALL AddOutput 0, 0, "oopliClassTable("i").fExternal = '0'B;", 'T'
 END;

 CALL AddOutput 0, 0, "END;", 'T'
RETURN


/*****************************************************************************/
/* CALL WriteOOPLIResolveCall(className, memberName, varName, callArgs,      */
/*                            functionCall)                                  */
/*                                                                           */
/* PARAMETERS       className, i           name of class                     */
/*                  memberName, i          name of member                    */
/*                  varName, i             name of object variable           */
/*                  callArgs, i            arguments to be added             */
/*                  functionCall, i        1 = is a function call, 0 = is not*/
/*                                                                           */
/* writes the OOPLIResolve call/function call to the output for class        */
/* <className> and member <memberName>. <varName> is the name of the         */
/* object variable as declared by the OOPLI programmer; it is passed as      */
/* a pointer. If <functionCall> is TRUE <memberName> is assumed to be        */
/* a function that returns a PTR, otherwise a call is assumed. This is very  */
/* essential because if functionCall is true the entryStruct.funcEntry will  */
/* be used and not entryStruct.callEntry. If <varName> is PARENT, the        */
/* resolve call will be explicitly for <className> and the object variable   */
/* passed will be THIS. Otherwise the resolve call will default to           */
/* the class of the THIS object variable                                     */
/*****************************************************************************/
WriteOOPLIResolveCall:
 PROCEDURE EXPOSE x.
 PARSE ARG className, memberName, varName, callArgs, functionCall

 className  = Translate(className)
 varName    = Translate(varName)
 memberName = Translate(memberName)
 IF functionCall = '' THEN
  functionCall = 0

 i = GetClassOrdinal(className)
 j = GetMemberOrdinal(className, memberName)

 IF x.class.i.module <> '',
  & x.module <> x.class.i.module THEN
  funcName = x.class.i.module
 ELSE
  funcName = 'OOPLIResolve'

 IF varName = 'PARENT' THEN
  CALL AddOutput 0, 0, funcName"('"className"',", 'T'
 ELSE
  CALL AddOutput 0, 0, "OOPLIResolve("varName"->varStruct.className,", 'T'

 IF \functionCall THEN
  CALL AddOutput 0, 0, "'"memberName"')->entryStruct.callEntry(", 'T'
 ELSE
  CALL AddOutput 0, 0, "'"memberName"')->entryStruct.funcEntry(", 'T'

 IF varName = 'PARENT' THEN
  CALL AddOutput 0, 0, "(THIS)", 'T'
 ELSE
  CALL AddOutput 0, 0, "("varName")", 'T'

 IF callArgs <> '' THEN
  CALL AddOutput 0, 0, ','callArgs, 'T'

 CALL AddOutput 0, 0, ')', 'T'
RETURN


/*****************************************************************************/
/* CALL WriteUndefConDestructors                                             */
/*                                                                           */
/*                                                                           */
/* for each class check if x.class.#.fConWritten is TRUE; if not the         */
/* constructor for the class has not been written. It will only be written   */
/* when the class is internal to the current module. The constructor         */
/* has not been written when it has not been used by the OOPLI programmer    */
/* and therefore a new member for the class will be created. This procedure  */
/* must be called *before* WriteMethodTable!                                 */
/*****************************************************************************/
WriteUndefConDestructors:
 PROCEDURE EXPOSE x.

 DO i = 1 TO x.class.0
  IF x.class.i.module = x.module,                /* internal and         */
   & \x.class.i.fConWritten THEN                 /* constructor written? */
  DO
   className = x.class.i.name
   memberName = 'C'className
   superClass = x.class.i.super

   CALL AddOutput 0, 0, className||memberName": PROC(this);", 'T'
   CALL AddOutput 0, 0, "DCL this PTR;", 'T'
   CALL AddOutput 0, 0, "DCL varThis PTR;", 'T'
   CALL AddOutput 0, 0, "varThis = this;", 'T'

   CALL AddOutput 0, 0, 'DO WHILE(varThis->varStruct.pNext ^= NULL());', 'T'
   CALL AddOutput 0, 0, ' varThis = varThis->varStruct.pNext;', 'T'
   CALL AddOutput 0, 0, 'END;', 'T'

   CALL AddOutput 0, 0, "IF varThis->varStruct.className ^= '"className"' THEN", 'T'
   CALL AddOutput 0, 0, "DO;", 'T'
   CALL AddOutput 0, 0, " ALLOCATE varStruct SET (varThis->varStruct.pNext);", 'T'
   CALL AddOutput 0, 0, " varThis = varThis->varStruct.pNext;", 'T'
   CALL AddOutput 0, 0, "END;", 'T'

   CALL AddOutput 0, 0, 'ALLOCATE' className 'SET (',
                            'varThis->varStruct.varMem);', 'T'
   CALL AddOutput 0, 0, 'varThis->varStruct.pNext = NULL();', 'T'
   CALL AddOutput 0, 0, "varThis->varStruct.className = '"className"';", 'T'
   CALL AddOutput 0, 0, "varThis->varStruct.superClassName = '"superClass"';", 'T'
   CALL AddOutput 0, 0, "varThis->varStruct.pNext = NULL();", 'T'

   IF superClass <> '' THEN
   DO
    CALL AddOutput 0, 0, 'CALL', 'T'
    CALL WriteOOPLIResolveCall superClass, 'C'superClass, 'PARENT'
    CALL AddOutput 0, 0, ';', 'T'
   END

   CALL AddOutput 0, 0, 'END;', 'T'

   CALL AddClassMember className, memberName, 'PROC(this)'
   j = GetMemberOrdinal(className, memberName)
   x.class.i.member.j.fWritten = 1
   x.class.i.fConWritten = 1
  END /* IF */
 END /* i */
RETURN


RemoveWhiteSpace:
 PROCEDURE
 PARSE ARG in

 out = ''

 DO i = 1 TO Words(in)
  out = out||in
 END
RETURN out


MakeWords:
 PROCEDURE
 PARSE ARG in

 out = ''
 standAlone = '(),;-+*/:'
 noTouchIn  = '"' || "'"
 wordDelim = standAlone || noTouchIn
 noTouchOpen = 0

 DO i = 1 TO Length(in)
  ch = SubStr(in, i, 1)
  IF Verify(ch, wordDelim) = 0 THEN
  DO
   IF \noTouchOpen THEN
    out = out ch
   ELSE
    out = out||ch

   IF Verify(ch, noTouchIn) = 0 THEN
   DO
    noTouchOpen = \noTouchOpen
   END
   ELSE
    IF \noTouchOpen THEN
     out = out || ' '
  END
  ELSE
   out = out || ch
 END
RETURN Strip(out)


NextWord:
 PROCEDURE EXPOSE x.
 PARSE ARG lineNo, wordNo

 IF wordNo < Words(x.lineNo) THEN
  RETURN Strip(SubWord(x.lineNo, wordNo+1,1))
 ELSE
  DO
   lineNo = lineNo + 1
   wordNo = 1
   RETURN Strip(SubWord(x.lineNo, wordNo,1))
  END
RETURN


SetLineWordCount:
 PROCEDURE EXPOSE x.
 PARSE ARG lineNo, wordNo, incLine, incWord

 IF incLine > 0 THEN
 DO
  IF lineNo + incLine <= x.0 THEN
   RETURN lineNo+incLine 1
  ELSE
   CALL Abort 'End of program found before logical end!'
 END
 ELSE
  IF wordNo + incWord <= Words(x.lineNo) THEN
   RETURN lineNo wordNo+incWord
  ELSE
   IF lineNo+1 <= x.0 THEN
    RETURN lineNo+1 1
   ELSE
    CALL Abort 'End of program found before logical end!'
RETURN


WordsUpTo:
 PROCEDURE EXPOSE x.
 PARSE ARG delimWord, lineNo, wordNo

 out = ''

 DO WHILE Strip(SubWord(x.lineNo,wordNo,1)) <> Strip(delimWord)
  out = out SubWord(x.lineNo, wordNo,1)
  PARSE VALUE SetLineWordCount(lineNo, wordNo, 0, +1) WITH lineNo wordNo
 END

 PARSE VALUE SetLineWordCount(lineNo, wordNo, 0, +1) WITH lineNo wordNo
RETURN lineNo wordNo Strip(out)


Abort:
 PROCEDURE
 PARSE ARG msg

 SAY msg
 rc = ShowCallStack(thisIsNotAnError)
EXIT 12
