MODULE HelloWorld;

CONST strlen = 12;
cr = 13;
lf = 10;

TYPE INTARRAY = ARRAY strlen OF INTEGER;

VAR str: INTARRAY;

PROCEDURE WriteStr();
BEGIN
    str[0] := 72;
    str[1] := 101;
    str[2] := 108;
    str[3] := 108;
    str[4] := 111;
    str[5] := 32;
    str[6] := 119;
    str[7] := 111;
    str[8] := 114;
    str[9] := 108;
    str[10] := 100;
    str[11] := 33;
END WriteStr;

PROCEDURE PrintStr();
VAR i: INTEGER;
BEGIN
    i := 0;
    WHILE i < strlen DO
        PutChar(str[i]);
        i := i + 1
    END;
    PutChar(cr);
    PutChar(lf)
END PrintStr;

BEGIN
    WriteStr();
    PrintStr();
END HelloWorld.
