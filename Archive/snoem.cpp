// snoem.cpp Emulator for the SNOW-1

#include <cstdint>


#define MEM_SIZE 0xFFFF


// Memory 
static uint8_t mem[MEM_SIZE];


// Registers
static uint16_t PC_reg;
static uint16_t ins_reg;
static uint16_t PCbuff_reg;
static uint16_t registers[16];


void init()
{
    for(uint16_t i = 0; i < MEM_SIZE; ++i)
    {
        mem[i] = 0;
    }

    PC_reg = 0;
    ins_reg = 0;
    PCbuff_reg = 0;
    
    registers[0] = 0;
    registers[1] = 0;
    registers[2] = 0;
    registers[3] = 0;
    registers[4] = 0;
    registers[5] = 0;
    registers[6] = 0;
    registers[7] = 0;
    registers[8] = 0;
    registers[9] = 0;
    registers[0] = 0;
    registers[1] = 0;
    registers[2] = 0;
    registers[3] = 0;
    registers[4] = 0;
    registers[5] = 0;
}


int main(int argc, char **argv)
{
    return 0;

    init();
}