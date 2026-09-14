// snasm.cpp

#include <iostream>
#include <cstdint>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <unordered_map>


enum class SECTION
{
    TEXT,
    DATA,
    UNRESOLVED
};


enum class OPERATION
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
    MOV,
    CMP,
    SUBI,
    CALL,
    RET
};

// Global Variables ------------------------------------------------------------
static const char *parsed_file = nullptr;

static std::ifstream in_file;
static std::ofstream out_file;

static std::unordered_map<std::string_view, uint16_t> local_labels;

// Section currently being parsed.
static SECTION current_section = SECTION::UNRESOLVED;

// Current parsed line number in the source file.
static size_t current_line = 0;

// Current byte offset within the .text section.
static size_t current_text_offset = 0;

// Current byte offset within the .data section.
static size_t current_data_offset = 0;




// Containers ------------------------------------------------------------------

// Represents a label that has been defined by the program and is visible within
// this unit.
struct DefinedLabel
{
    // Location where the symbol is defined. This will not be unresolved, as it
    // is a local label.
    SECTION section;

    // Offset from the section where this symbol is defined. 
    size_t offset;
};



// Represents an entry in the relocation table.
// struct RelocTableEntry
// {

// }


// Represents a label that 
// struct GlobalLabel
// {
//     std::string name;

//     // Location where the symbol is defined.
//     SECTION section; 
    
//     // Offset from section where this symbol is defined. 0 if the section is
//     // UNRESOLVED.
//     size_t offset;
// };


static void print_err_location(size_t col = 0)
{
    std::cout << parsed_file << ':' << current_line << ':' << col + 1;
}

static void skip_whitespace(const std::string &str, size_t &idx)
{
    while(idx < str.size() && str[idx] == ' ') { ++idx; }
}

static void print_unexp_eol(size_t col = 0)
{
    print_err_location(col);
    std::cout << " -> Unexpected end of line.\n";
}

static void consume_reg(const std::string &str, size_t &idx, bool expect_comma)
{
    skip_whitespace(str, idx);

    std::string_view view(str.data() + idx, str.size() - idx);

    size_t end_reg_ident = view.find_first_of(" ,\t\n");

    view = view.substr(0, end_reg_ident);

    if(view.size() != 2 || 
        view.at(0) != 'x' ||
        !std::isdigit(view.at(1)))
    {
        print_err_location(idx);
        std::cout << " -> Expected register, got: \"" << view << "\"\n";
        exit(-1);
    }

    // Consume register
    idx += 2;

    if(expect_comma)
    {
        if(idx >= str.size())
        {
            print_unexp_eol(str.size());
            exit(-1);
        }

        if(str[idx] != ',')
        {
            print_err_location(idx);
            std::cout << " -> Expected comma.\n";
            exit(-1);
        }
        
        // Consume comma
        ++idx;
    }
}

static void enforce_ins_section()
{
    if(current_section == SECTION::DATA)
    {
        print_err_location();
        std::cout << " -> Instructions cannot be defined in the data "
            "section.\n";
        exit(-1);
    }
}

/**
 * @brief Given a position inside a string, attempts to parse an 
 * operation, crashing if invalid. Index is passed in by reference and is 
 * modified directly. 
 * 
 * @param str String to parse.
 * @param idx Index to begin parsing.
 * 
 * @return OPERATION Parsed operation. 
 */
static OPERATION get_op(const std::string &str, size_t &idx)
{
    // Skip any whitespace before the operation
    skip_whitespace(str, idx);

    std::string_view view(str.data() + idx, str.size() - idx);

    size_t end_op_ident = view.find_first_of(" \t\n");

    view = view.substr(0, end_op_ident);

    // Consume operator ident.
    idx += end_op_ident;

    if(view == "add")
    {
        enforce_ins_section();

        consume_reg(str, idx, true);
        consume_reg(str, idx, true);
        consume_reg(str, idx, false);
    }
    else if(view == "addi")
    {
        enforce_ins_section();

        

    }
    else if(view == "movl")
    {

    }
    else if(view == "movh")
    {

    }
    else if(view == "sub")
    {

    }
    else if(view == "lodb")
    {

    }
    else if(view == "lodw")
    {

    }
    else if(view == "strb")
    {

    }
    else if(view == "strw")
    {

    }
    else if(view == "mul")
    {

    }
    else if(view == "div")
    {

    }
    else if(view == "beq" || view == "bne" || view == "blt" || view == "bgt")
    {

    }
    else if(view == "bl")
    {

    }
    else if(view == "db")
    {

    }
    else if(view == "dw")
    {

    }
    else
    {
        print_err_location();
        std::cout << " -> Unkown operation: \"" << view << "\"\n";
        exit(-1);
    }


    return OPERATION::ADD;
}



/**
 * @brief Given an index inside a string, parses till the end of the string 
 * validating operations and checking if the given operation 
 * updates the offset in the given section.
 * 
 * @param str String to parse.
 * @param idx Index to start parsing from.
 */
static void check_op_update_offset(const std::string &str, size_t idx)
{
    if(idx >= str.size()) { return; }

    get_op(str, idx);

    // while(idx < str.size())
    // {





    // }
}


static void parse_labels()
{
    std::string line;
    current_line = 0;
    
    while(std::getline(in_file, line))
    {
        size_t line_idx = 0;
        ++current_line;
        if(line.size() == 0) { continue; }

        // Section
        if(line.at(0) == '.')
        {
            std::string_view view{line};

            size_t end = view.find_first_of(" \n");

            view = view.substr(0, end);

            if(view == ".text")
            {
                print_err_location();
                std::cout << " -> Parsed section: TEXT\n";
                current_section = SECTION::TEXT;
            }
            else if(view == ".data")
            {
                print_err_location();
                std::cout << " -> Parsed section: DATA\n";
                current_section = SECTION::DATA;
            }
            else
            {
                print_err_location();
                std::cout << " -> Invalid section identifier.\n";
                exit(-1);
            }

            line_idx += end;
        }

        // Label
        else if(line.at(0) != ' ')
        {
            std::string_view view{line};

            size_t end = view.find_first_of(":");

            if(end == std::string_view::npos ||    
                view.find_first_of(" ") <= end)
            {
                print_err_location();
                std::cout << " -> Invalid label.\n";
                exit(-1);
            }

            view = view.substr(0, end);

            print_err_location();
            std::cout << " -> Parsed label: " << view << '\n';
        
            // +1 to consume the ':'
            line_idx += end + 1;
        }

        // No more text in the line
        if(line_idx >= line.size()) { continue; }

        check_op_update_offset(line, line_idx);
    }
}

static void assemble()
{
    parse_labels();
}


int main(int argc, char **argv)
{
    if(argc != 3)
    {
        std::cout << "Usage: snasm <input_file> <output_file>\n";
        return -1;
    }

    const char *in_file_path = argv[1];
    const char *out_file_path = argv[2];

    parsed_file = in_file_path;

    in_file.open(in_file_path);

    if(!in_file.is_open()) 
    {
        std::cerr << "Failed to open file: "
                  << std::strerror(errno) << '\n';
        return -1;
    }

    out_file.open(out_file_path);

    if(!out_file.is_open())
    {
        std::cerr << "Failed to open file: "
                  << std::strerror(errno) << '\n';
        return -1;
    }

    assemble();

    return 0;
}