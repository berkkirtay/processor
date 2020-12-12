#include "cpu.h"
void initializeCPU(int16_t instructionSet[], uint16_t CODESIZE){
  printf("New instruction set inserted.. Size of set= %d\n", CODESIZE);
  CPU = (CPU1 *)malloc(sizeof(CPU1));
  CPU->IP = 0;
  CPU->SP = 0;
  CPU->RAM = &instructionSet[0];
}
void cpu(){
   while(CPU->IP != -1){
    //  if(debug) printf("Instruction turn= %d | Instruction type= %s\n", CPU->IP, instructionToString(instructionSet[IP]));
     CPU->opcode = 0xFF & (CPU->RAM[CPU->IP] >> 8);  //fetching from the given instructionSet
     switch(CPU->opcode){
       case ADD:
         CPU->registers[CPU->RAM[CPU->IP] & 0x000F - 1] += CPU->registers[CPU->RAM[CPU->IP] & 0x000F];
         CPU->IP++;
         break;
       case SUB:
         CPU->registers[CPU->RAM[CPU->IP] & 0x000F - 1] -= CPU->registers[CPU->RAM[CPU->IP] & 0x000F];
         CPU->IP++;
         break;
       case MUL:
         CPU->registers[CPU->RAM[CPU->IP] & 0x000F - 1] *= CPU->registers[CPU->RAM[CPU->IP] & 0x000F];
         CPU->IP++;
         break;
       case DIV:
         CPU->registers[CPU->RAM[CPU->IP] & 0x000F - 1] /= CPU->registers[CPU->RAM[CPU->IP] & 0x000F];
         CPU->IP++;
         break;
       case AND:
         CPU->registers[CPU->RAM[CPU->IP] & 0x000F - 1] &= CPU->registers[CPU->RAM[CPU->IP] & 0x000F];
         CPU->IP++;
         break;
       case NOT:
         CPU->registers[CPU->RAM[CPU->IP] & 0x000F - 1] = ~CPU->registers[CPU->RAM[CPU->IP] & 0x000F - 1];
         CPU->IP++;
         break;
       case OR:
         CPU->registers[CPU->RAM[CPU->IP] & 0x000F - 1] |= CPU->registers[CPU->RAM[CPU->IP] & 0x000F];
         CPU->IP++;
         break;
       case XOR:
         CPU->registers[CPU->RAM[CPU->IP] & 0x000F - 1] ^= CPU->registers[CPU->RAM[CPU->IP] & 0x000F];
         CPU->IP++;
         break;
       case LOAD:
         CPU->registers[CPU->RAM[CPU->IP] & 0x000F] = CPU->RAM[CPU->IP + 1];
         CPU->IP += 2;
         break;
       case STOR:
         printf("STORED VALUE TO THE RAM : %d\n", CPU->registers[CPU->RAM[CPU->IP] & 0x000F]); 
         CPU->RAM[CPU->RAM[CPU->IP] & 0x000F] = CPU->registers[CPU->RAM[CPU->IP] & 0x000F];
         CPU->IP++;
         break;
       case PUSH:
         CPU->registers[CPU->RAM[CPU->IP] & 0x000F] = CPU->RAM[CPU->IP + 1];
         CPU->IP += 2;
         break;
       case POP:
         CPU->registers[CPU->RAM[CPU->IP] & 0x000F] = 0;
         break;
       case CALL:  // simple CALL instruction which pushes current location into stack and jumps..
         CPU->stack[CPU->SP] = CPU->IP + 2;  
         CPU->SP++;
         CPU->IP = CPU->RAM[CPU->IP + 1];   
         break;
       case DEB: // for debugging
         printf("RAM : \n");
         int i = 0;
         while(CPU->RAM[i] != 0 && CPU->RAM[i+1] != 0 && CPU->RAM[i+2] != 0){
           printf("%d at %d.\n", CPU->RAM[i], i);
           i++;
         }
         printf("Subroutine registers: \n");
         i= 0;
         while(CPU->registers[i] != 0 && CPU->registers[i+1] != 0 && CPU->registers[i+2] != 0){
           printf("%d at %d.\n", CPU->registers[i], i);
           i++;
         }
         break;
       case RET: //Returns from stack
         CPU->SP--;
         CPU->IP = CPU->stack[CPU->SP];
         break;
       case JMP:
         CPU->IP = CPU->RAM[CPU->IP + 1];
         break;
       case PRINT:  // printing compiled data to a file (i.e. HDD)
         printf("Current written value to the file is %d at %p\n", CPU->registers[CPU->RAM[CPU->IP] & 0x000F], &HDD[CPU->RAM[CPU->IP]]);
         HDD[CPU->RAM[CPU->IP] & 0x000F] = CPU->registers[CPU->RAM[CPU->IP] & 0x000F];
         CPU->IP++;
         break;
       case HALT:
         CPU->IP = -1;
         printf("CPU HALTED\n");
         return;
         break;
       default:  // it does nothing when cpu encounters an invalid instruction.
         break;
     }
     int j;
     for(j=0 ; j < 100000; j++){ // here I tried to emulate processor clock cycles.
         //for sleeping -_-
     }
   }
} 


