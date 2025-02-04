(* Oberon-0 implementations of various sort algorithms. *)
MODULE Sort0;

(* Length of the array to be sorted. *)
CONST Dim = 10;
lf = 10;
cr = 13;

(* Array type *)
TYPE INTARRAY = ARRAY Dim OF INTEGER;

(* Array to be sorted. *)
VAR a: INTARRAY;
c: INTEGER;

(* Initializes the array. *)
PROCEDURE Init();
VAR i: INTEGER;
BEGIN
    i := 0;
    WHILE i < Dim DO
        a[i] := Dim-i;
        i := i + 1
    END
END Init;

(* Convert integers from 0-9 to their ascii code
Sets out to null char if a is out of range*)
PROCEDURE IntToCharCode(a: INTEGER; VAR out: INTEGER);
BEGIN
    IF (a < 0) OR (a > 9) THEN out := 0;
    ELSE out := a + 48 END
END IntToCharCode;

(* Prints the array *)
PROCEDURE PrintArray();
VAR i: INTEGER;
BEGIN
    i := 0;
    WHILE i < Dim DO
        IntToCharCode(a[i], c);
        PutChar(c);
        i := i + 1
    END;
    PutChar(cr);
    PutChar(lf)
END PrintArray;

(* Swaps the two values passed as var-parameters. *)
PROCEDURE Swap(VAR a, b: INTEGER);
VAR t: INTEGER;
BEGIN
    t := a;
    a := b; 
    b := t
END Swap;

(* Applies the insertion-sort algorithm the the global array. *)
PROCEDURE InsertionSort;
VAR i, j: INTEGER;
BEGIN
    i := 0;
    WHILE i < Dim DO
        j := i;
        WHILE (j > 0) & (a[j - 1] > a[j]) DO
            Swap(a[j], a[j - 1]);
            j := j - 1
        END;
        i := i + 1
    END
END InsertionSort;

(* Applies the selection-sort algorithm the the global array. *)
PROCEDURE SelectionSort;
VAR i, j: INTEGER;
    min: INTEGER;
BEGIN
    i := 0;
    WHILE i < Dim - 1 DO
        min := i;
        j := i + 1;
        WHILE  j < Dim DO
            IF a[j] < a[min] THEN min := j END;
            j := j + 1
        END;
        IF min # i THEN Swap(a[i], a[min]) END;
        i := i + 1
    END
END SelectionSort;
    
(* Applies the bubble-sort algorithm to the global array. *)
PROCEDURE BubbleSort;
VAR i, j: INTEGER;
BEGIN
    i := 0;
    WHILE i < Dim DO
        j := Dim - 1;
        WHILE j > i DO
            IF a[j - 1] > a[j] THEN Swap(a[j - 1], a[j]) END;
            j := j - 1
        END;
        i := i + 1
    END
END BubbleSort;

(* Applies the quick-sort algorithm to the global array. *)
PROCEDURE QuickSort;

    PROCEDURE QSort(l, r: INTEGER);
    VAR i, j, x: INTEGER;
    BEGIN
        i := l;
        j := r;
        x := a[(r + l) DIV 2];
        WHILE i <= j DO
            WHILE a[i] < x DO i := i + 1 END;
            WHILE x < a[j] DO j := j - 1 END;
            IF i <= j THEN
                Swap(a[i], a[j]);
                i := i + 1;
                j := j - 1
            END
        END;
        IF l < j THEN QSort(l, j) END;
        IF i < r THEN QSort(i, r) END
    END QSort;

BEGIN
    QSort(0, Dim - 1)
END QuickSort;

(* Main program. *)
BEGIN
    Init;
    PrintArray();
    QuickSort;
    PrintArray();
    Init;
    PrintArray();
    BubbleSort;
    PrintArray();
    Init;
    PrintArray();
    InsertionSort;
    PrintArray();
END Sort0.
