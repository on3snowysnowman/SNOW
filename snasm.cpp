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


// Global Variables ------------------------------------------------------------
static const char *parsed_file = nullptr;

static std::ifstream in_file;
static std::ofstream out_file;

static std::unordered_map<std::string_view, uint16_t> local_labels;

// Section currently being parsed.
static SECTION current_section = SECTION::UNRESOLVED;

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


static void print_err_location(size_t line)
{
    std::cout << parsed_file << ':' << line;
}




static void parse_labels()
{
    size_t current_line = 0;
    std::string line;

    while(std::getline(in_file, line))
    {
        ++current_line;
        if(line.size() == 0) { continue; }

        // Section
        if(line.at(0) == '.')
        {
            std::string_view view{line};

            size_t end = view.find_first_of(" \n");

            view = view.substr(0, end);

            if(view == "text")
            {
                current_section == SECTION::TEXT;
            }
            else if(view == "data")
            {
                current_section == SECTION::DATA;
            }
            else
            {
                print_err_location(current_line);
                std::cout << " -> Invalid section identifier.\n";
                exit(-1);
            }
        }

        // Label
        if(line.at(0) != ' ')
        {
            std::string_view view{line};

            size_t end = view.find_first_of(":");

            if(end == std::string_view::npos ||    
                view.find_first_of(" ") > end)
            {
                print_err_location(current_line);

                std::cout << " -> Invalid label.\n";
                exit(-1);
            }


        }

    }
}

static void assemble()
{


}


int main(int argc, char **argv)
{
    if(argc != 3)
    {
        std::cout << "Usage: snasm <input_file> <output_file\n";
        return -1;
    }

    const char *in_file_path = argv[1];
    const char *out_file_path = argv[2];

    in_file.open(in_file_path);

    if(!in_file.is_open()) 
    {
        std::cerr << "Failed to open file: "
                  << std::strerror(errno) << '\n';
        return 1;
    }

    out_file.open(out_file_path);

    if(!out_file.is_open())
    {
        std::cerr << "Failed to open file: "
                  << std::strerror(errno) << '\n';
        return -1;
    }



    return 0;
}