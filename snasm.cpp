// snasm.cpp


#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>


// Types -----------------------------------------------------------------------

using reg_T = uint8_t;

// Represents a sectoin inside the assembly file.
enum class SECTION
{
    TEXT,
    DATA,
    UNRESOLVED
};

// Types instructions can represent
enum class INSTR_TYPE
{
    ADD,
    ADDI,
    MOVL,
    MOVH,
    SUB,
    LODB,
    LODW,
    STRB,
    STRW,
    MUL,
    DIV,
    BCOND,
    BLINK,
    AND,
    NAND,
    OR,
    NOR,
    XOR,
    NXOR,
    NOT,
    SEXTB,
    LSH,
    RSH,
    NOOP,
    HALT,
    MOV,
    CMP,
    SUBI,
    CALL,
    RET
};

struct AddInstr
{
    reg_T dest;
    reg_T sr1;
    reg_T sr2;
};

struct AddImInstr
{
    reg_T dest;
    int8_t immediate;
};

struct MovLInstr
{
    reg_T dest;
    int8_t immediate;
};

struct MovHInstr
{
    reg_T dest;
    int8_t immediate;
};

struct SubInstr
{
    reg_T dest;
    reg_T sr1;
    reg_T sr2;
};

struct LodBInstr
{
    reg_T dest;
    reg_T addr;
    int8_t offset;
};

struct LodWInstr
{
    reg_T dest;
    reg_T addr;
    int8_t offset;
};

struct StrBInstr
{
    reg_T dest;
    reg_T addr;
    int8_t offset;
};

struct StrWInstr
{
    reg_T dest;
    reg_T addr;
    int8_t offset;
};

struct MulInstr
{
    reg_T dest;
    reg_T src1;
    reg_T src2;
};

struct DivInstr
{
    reg_T dest;
    reg_T src1;
    reg_T src2;
};

enum class BCondType
{
    EQ,
    NE,
    GT,
    LT
};

struct BCondInst
{
    BCondType cond;
    int16_t offset; // Only 10 bits are available.
};

struct BUncondInst
{
    int16_t offset; // Only 12 bits are available.
};

struct AndInstr
{
    reg_T dest;
    reg_T src;
};

struct NandInstr
{
    reg_T dest;
    reg_T src;
};

struct OrInstr
{
    reg_T dest;
    reg_T src;
};

struct NorInstr
{
    reg_T dest;
    reg_T src;
};

struct XorInstr
{
    reg_T dest;
    reg_T src;
};

struct XnorInstr
{
    reg_T dest;
    reg_T src;
};

struct NotInstr
{
    reg_T dest;
    reg_T src;
};

struct SextBInstr
{
    reg_T dest;
    reg_T src;
};

struct LshInstr
{
    reg_T dest;
    int8_t immediate;
};

struct RshInstr
{
    reg_T dest;
    int8_t immediate;
};

struct NoopInstr {};

struct HaltInstr {};

struct BLinkInstr
{
    reg_T link;
    reg_T addr;
};

struct CmpInstr
{
    reg_T src1;
    reg_T src2;
};

struct MovInstr
{
    reg_T dest;
    reg_T src;
};

struct SubImInstr
{
    reg_T dest;
    int8_t immediate;
};

struct CallInstr
{
    int16_t offset; // Offset in instruction count, not bytes.
};

struct RetInstr {};


struct Instruction
{
    // Type this instruction represents.
    INSTR_TYPE type;    

    union 
    {
        
    } as;
    
};




// Global Variables ------------------------------------------------------------

static std::ifstream in_file;

// Path to the file being parsed
static const char *parsed_file;

// Current parsed line inside the file being parsed.
static size_t current_line;





int main(int argc, char **argv)
{
    if(argc != 3)
    {
        std::cout << "Usage: snasm <in_file> <out_file>\n";
        return -1;
    }



    std::cout << "Hello World!\n";

    return 0;
}