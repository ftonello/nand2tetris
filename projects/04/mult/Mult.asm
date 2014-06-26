// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[3], respectively.)

// Put your code here.
  @R2
  M=0 // set output to zero

(LOOP)
  @R1
  D=M
  @END
  D;JEQ // if R1 == 0 then goto END

  @R1
  MD=D-1 // subtract R1 - 1

  @R0
  D=M
  @R2
  M=D+M // do R2 += R0
  @LOOP
  0;JMP

(END)
  @END
  0;JMP // end of a hack program
