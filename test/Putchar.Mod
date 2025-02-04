MODULE Putchar;

VAR a: INTEGER;
c: INTEGER;

(* Convert integers from 0-9 to their ascii code
Sets out to null char if a is out of range*)
PROCEDURE IntToCharCode(a: INTEGER; VAR out: INTEGER);
BEGIN
IF (a < 0) OR (a > 9) THEN out := 0;
ELSE out := a + 48 END
END IntToCharCode;

BEGIN
    a := 10;
    IntToCharCode(a, c);
    PutChar(c);
    PutChar(10);
    PutChar(13);
END Putchar.