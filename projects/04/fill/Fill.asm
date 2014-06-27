// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, the
// program clears the screen, i.e. writes "white" in every pixel.

  @screen_status
  M=0              // screen status begins white

(MAIN_LOOP)
  @KBD
  D=M
  @PAINT         // argument is 0 (white)
  D;JEQ          // keyboard is 0 it means no key pressed

  D=-1           // if key was pressed then set
                 // argument to 0xffff (black)

//--------
// PAINT:
// @argument: register D should have the value of the new
//            status. 0 for white and -1 for black.
//--------
(PAINT)
  @screen_status
  D=M-D
  @MAIN_LOOP     // if old status == new status then
                 // do nothing
  D;JEQ

  @screen_status // invert screen status
  M=!M

  @SCREEN        // current_paint is a pointer to the
  D=A            // current address to paint
  @current_paint
  M=D

(PAINT_LOOP)
  @screen_status
  D=M
  @current_paint
  A=M
  M=D            // paint with current_status
  @current_paint
  MD=M+1         // increase current_paint
  @24576         // SCREEN + (512 / 16) * 256 = 24576
  D=A-D          // 24576 - current_paint
  @PAINT_LOOP
  D;JGT          // loop if not done painting

  @MAIN_LOOP
  0;JMP          // infinite loop
