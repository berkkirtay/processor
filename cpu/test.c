#include "cpu.c"

int main(){
  int16_t instructionSet[]={
  0x0800,  // LOAD 
  0x0010,  // 16 to register[0]
  0x0801,  // LOAD
  0x0003,  // 3 to register[1]
  0x0201,  // MUL -> register[1] *= register[2]
  0x0900,  // STOR register[0] 
  0x0C00,  // CALL and also pushes 0x901 into stack
  0x000A,  // Memory location which CALL isntruction uses for jumping
  0x0901,  // STOR register[1] 
  0x1100,  // HALT
  0x1000,  // -> A'th location on the memory.
  0x0E00   // RET : return from the stack subroutine.
  };
  initializeCPU(instructionSet, sizeof(instructionSet)/sizeof(instructionSet[0]));
  cpu();

  return 0;    
}