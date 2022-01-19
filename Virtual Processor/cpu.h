#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "processorInstructions.h"

uint16_t ROM[1000];
uint16_t HDD[1000];

struct bus{
  uint16_t IP;  // instruction pointer
  uint16_t SP;  // stack pointer
  uint16_t *RAM;
  // 16 bit stack and register size
  uint16_t registers[15];
  uint16_t stack[15];
  uint8_t opcode;

};
typedef struct bus CPU1;
CPU1 *CPU = NULL;


//prototypes

void initializeCPU(int16_t instructionSet[], uint16_t CODESIZE);
void cpu();





