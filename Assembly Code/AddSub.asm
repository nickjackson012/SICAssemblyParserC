. Example assembly code from pg. 15
. Figure 1.3A in Chapter 1
. Modified to be a assembly program
ADDSUB  START       7F00
        LDA         ALPHA       LOAD ALPHA INTO REGISTER A
        ADD         INCR        ADD THE VALUE OF INCR
        SUB         ONE         SUBTRACT 1
        STA         BETA        STORE IN BETA
        LDA         GAMMA       LOAD GAMMA INTO REGISTER A
        ADD         INCR        ADD THE VALUE OF INCR
        SUB         ONE         SUBTRACT 1
        STA         DELTA       STORE IN DELTA
        XOS                     END PROGRAM AND EXIT TO THE OS
        .
        .
        .
ONE     WORD        1           ONE-WORD CONSTANT
ALPHA   WORD        -12         ONE-WORD CONSTANT
GAMMA   WORD        -2000       ONE-WORD CONSTANT
INCR    WORD        1           ONE-WORD CONSTANT
.
BETA    RESW        1           ONE-WORD VARIABLE
DELTA   RESW        1           ONE-WORD VARIABLE
        END