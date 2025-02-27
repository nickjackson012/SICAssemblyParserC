. Example assembly code from pg. 13
. Figure 1.2A in Chapter 1
. Modified to be a assembly program
DATMOV  START       7F00
        LDA         FIVE        LOAD CONSTANT 5 INTO REGISTER A
        STA         ALPHA       STORE IN ALPHA
        LDCH        CHARZ       LOAD CHARACTER 'Z' INTO REGISTER A
        STCH        C1          STORE IN CHARACTER VARIABLE C1
        XOS                     END PROGRAM AND EXIT TO THE OS
        .
        .
        .
ALPHA   RESW        1           ONE-WORD VARIABLE
FIVE    WORD        5           ONE-WORD CONSTANT
CHARZ   BYTE        C'Z'        ONE-BYTE CONSTANT
C1      RESB        1           ONE-BYTE VARIABLE
        END