// snasm.cpp


#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <cerrno>
#include <cstring>
#include <charconv>
#include <string_view>
#include <optional>
#include <cmath>
#include <variant>


// Types -----------------------------------------------------------------------

using reg_T = uint8_t;

// Represents a sectoin inside the assembly file.
enum class SECTION
{
    TEXT,
    DATA,
    UNRESOLVED
};

struct AddInstr
{
    reg_T dst;
    reg_T src1;
    reg_T src2;
};

struct AddImInstr
{
    reg_T dst;
    int8_t imm;
};

struct MovLInstr
{
    reg_T dst;
    int8_t imm;
};

struct MovHInstr
{
    reg_T dst;
    int8_t imm;
};

struct SubInstr
{
    reg_T dst;
    reg_T src1;
    reg_T src2;
};

struct SubImInstr
{
    reg_T dst;
    int8_t imm;
};

struct LodBInstr
{
    reg_T dst;
    reg_T addr;
};

struct LodSBInstr
{
    reg_T dst;
    reg_T addr;
};

struct LodWInstr
{
    reg_T dst;
    reg_T addr;
};

struct StrBInstr
{
    reg_T dst;
    reg_T addr;
};

struct StrWInstr
{
    reg_T dst;
    reg_T addr;
};

struct MulInstr
{
    reg_T dst;
    reg_T src1;
    reg_T src2;
};

struct DivInstr
{
    reg_T dst;
    reg_T src1;
    reg_T src2;
};

enum class BCondType
{
    EQL,
    NEQ,
    GTU,
    LTU,
    GTS,
    LTS
};

struct BUncondInst
{
    std::string label;
};

struct BCondInst
{
    BCondType cond;
    std::string label;
};

struct BLinkInstr
{
    reg_T addr;
    reg_T link;
};

struct BRegInstr
{
    reg_T addr;
};

struct AndInstr
{
    reg_T dst;
    reg_T src;
};

struct NandInstr
{
    reg_T dst;
    reg_T src;
};

struct OrInstr
{
    reg_T dst;
    reg_T src;
};

struct NorInstr
{
    reg_T dst;
    reg_T src;
};

struct XorInstr
{
    reg_T dst;
    reg_T src;
};

struct XnorInstr
{
    reg_T dst;
    reg_T src;
};

struct NotInstr
{
    reg_T dst;
    reg_T src;
};

struct LslInstr
{
    reg_T dst;
    int8_t imm;
};

struct LsrInstr
{
    reg_T dst;
    int8_t imm;
};

struct NoopInstr {};

struct HaltInstr {};

struct CmpInstr
{
    reg_T src1;
    reg_T src2;
};

struct MovInstr
{
    reg_T dst;
    reg_T src;
};

struct CallInstr
{
    // int16_t offset; // Offset in instruction count, not bytes.
    std::string label;
};

struct RetInstr {};

struct PushInstr 
{
    reg_T reg;
};

struct PopInstr
{
    reg_T reg;
};

using Instruction = std::variant<
    AddInstr,
    AddImInstr,
    MovLInstr,
    MovHInstr,
    SubInstr,
    SubImInstr,
    LodBInstr,
    LodSBInstr,
    LodWInstr,
    StrBInstr,
    StrWInstr,
    MulInstr,
    DivInstr,
    BUncondInst,
    BCondInst,
    BLinkInstr,
    BRegInstr,
    AndInstr,
    NandInstr,
    OrInstr,
    NorInstr,
    XorInstr,
    XnorInstr,
    NotInstr,
    LslInstr,
    LsrInstr,
    NoopInstr,
    HaltInstr,
    CmpInstr,
    MovInstr,
    CallInstr,
    RetInstr,
    PushInstr,
    PopInstr
>;

struct DefinedLabel
{
    // Section the label is defined in.
    SECTION section;

    // Offset from the start of the defined section.
    size_t offset;
};


// Global Variables ------------------------------------------------------------

static std::ifstream in_file;
static std::ofstream out_file;

// Path to the file being parsed
static const char *parsed_file;

// Current parsed line inside the file being parsed.
static size_t current_line;

// Current offset from the start of the text section.
static size_t text_offset = 0;

// Current offset from the start of the data section.
static size_t data_offset = 0;

// Section the assembler is parsing.
static SECTION current_section = SECTION::UNRESOLVED;

static std::vector<Instruction> instructions;

// List of bytes in the data section. Each index corresponds to the same offset 
// from the data section.
static std::vector<uint8_t> data;

// Labels defined in the source file. 
static std::unordered_map<std::string, DefinedLabel> labels;


// Functions -------------------------------------------------------------------

/**
 * @brief Prints a location inside the source file.
 * 
 * @param col Column to print, default to 0.
 */
static void print_loc(size_t col = 0)
{
    std::cout << parsed_file << ':' << current_line << ':' << col + 1;
}


static void skip_whitespace(const std::string &line, size_t &idx)
{
    if(line.size() == 0 || idx >= line.size()) { return; }

    const std::string_view view {line.data() + idx, line.size() - idx};

    size_t end = view.find_first_not_of(" ");

    if(end == std::string_view::npos) 
    { 
        idx = line.size(); 
        return;
    }

    idx += end;
}

static std::string_view make_view(const std::string &line, const size_t idx)
{
    return std::string_view {line.data() + idx, line.size() - idx};
}

static reg_T parse_reg(const std::string &line, size_t &idx)
{
    skip_whitespace(line, idx);

    if(idx >= line.size() || line.size() - idx < 2)
    {
        print_loc(idx);
        std::cout << "-> Expected register.\n";
    }

    std::string_view view = make_view(line, idx);

    // Save the start position of the register in case we need to output it.
    size_t reg_start_pos = idx;

    size_t end = view.find_first_of(" ,");

    if(end == std::string_view::npos)
    {
        end = view.size();
        idx += view.size();
    }
    else
    {
        idx += end;
    }

    view = view.substr(0, end);

    if(view.size() < 2 || view.size() > 3 || view.at(0) != 'x')
    {
        print_loc(reg_start_pos);
        std::cout << " -> Expected register, got: \"" << view << "\"\n";
        exit(-1);
    }

    if(view.size() == 3)
    {
        char first_dig = view[1];
        char second_dig = view[2];

        if(!std::isdigit(first_dig) || !std::isdigit(second_dig))
        {
            print_loc(reg_start_pos);
            std::cout << " -> Expected register, got: \"" << view << "\"\n";
            exit(-1);
        }

        uint8_t first_val = static_cast<uint8_t>(first_dig - '0');
        uint8_t second_val = static_cast<uint8_t>(second_dig - '0');

        if(first_val != 1 || second_val > 5)
        {
            print_loc(reg_start_pos);
            std::cout << " -> Invalid register: \"" << view << "\"\n";
            exit(-1);
        }

        return 10 + second_val;
    }

    char dig = view[1];

    if(!std::isdigit(dig))
    {
        print_loc(reg_start_pos);
        std::cout << " -> Expected register, got: \"" << view << "\"\n";
        exit(-1);
    }

    return static_cast<uint8_t>(dig - '0');
}

std::optional<std::int64_t> parse_i64(std::string_view str)
{
    if (str.empty())
        return std::nullopt;

    bool negative = false;

    if (str.front() == '+' || str.front() == '-') {
        negative = str.front() == '-';
        str.remove_prefix(1);

        if (str.empty())
            return std::nullopt;
    }

    int base = 10;

    if (str.size() >= 2 && str[0] == '0') {
        if (str[1] == 'x' || str[1] == 'X') {
            base = 16;
            str.remove_prefix(2);
        } else if (str[1] == 'b' || str[1] == 'B') {
            base = 2;
            str.remove_prefix(2);
        }
    }

    if (str.empty())
        return std::nullopt;

    // Parse the magnitude as unsigned so INT64_MIN (-2^63)
    // can be represented without overflowing during parsing.
    std::uint64_t magnitude;

    auto [ptr, ec] = std::from_chars(
        str.data(),
        str.data() + str.size(),
        magnitude,
        base
    );

    if (ec != std::errc{} || ptr != str.data() + str.size())
        return std::nullopt;

    constexpr std::uint64_t max =
        static_cast<std::uint64_t>(INT64_MAX);

    if (negative) {
        // INT64_MIN has magnitude INT64_MAX + 1.
        if (magnitude > max + 1)
            return std::nullopt;

        if (magnitude == max + 1)
            return INT64_MIN;

        return -static_cast<std::int64_t>(magnitude);
    }

    if (magnitude > max)
        return std::nullopt;

    return static_cast<std::int64_t>(magnitude);
}

static int64_t parse_immediate(const std::string &line, size_t &idx,
    int64_t imm_min, int64_t imm_max)
{
    skip_whitespace(line, idx);

    if(idx >= line.size())
    {
        print_loc(idx);
        std::cout << " -> Expected immediate.\n";
        exit(-1);
    }

    const size_t view_start = idx;

    std::string_view view = make_view(line, idx);

    size_t end = view.find_first_of(" ");

    if(end == std::string_view::npos)
    {
        end = view.size();
    }

    view = view.substr(0, end);

    idx += end;

    std::optional<int64_t> value_opt = parse_i64(view);

    if(!value_opt.has_value())
    {
        print_loc(view_start);
        std::cout << " -> Invalid immediate.\n";
        exit(-1);
    }

    const int64_t value = *value_opt;

    if(value < imm_min || value > imm_max)
    {
        print_loc(view_start);
        std::cout << " -> Immediate must be in the range [" << imm_min <<
            ", " << imm_max << "]\n";
        exit(-1);
    }

    return value;
}

static void parse_comma(const std::string &line, size_t &idx)
{
    skip_whitespace(line, idx);
    if(idx >= line.size() || line[idx] != ',')
    {
        print_loc(idx);
        std::cout << " -> Expected ','\n";
        exit(-1);
    }

    ++idx;
}


static void parse_AddInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src1 = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src2 = parse_reg(line, idx);

    Instruction instr = AddInstr
    {
        .dst = dst,
        .src1 = src1,
        .src2 = src2
    };

    instructions.push_back(instr);
}

static void parse_AddImInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    const int8_t imm = static_cast<int8_t>(
        parse_immediate(line, idx, INT8_MIN, INT8_MAX));

    Instruction instr = AddImInstr
    {
        .dst = dst,
        .imm = imm
    };

    instructions.push_back(instr);
}

static void parse_MovLInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    const int8_t imm = static_cast<int8_t>(
        parse_immediate(line, idx, INT8_MIN, INT8_MAX));

    Instruction instr = MovLInstr
    {
        .dst = dst,
        .imm = imm
    };

    instructions.push_back(instr);
}

static void parse_MovHInstr(const std::string &line, size_t &idx)
{

    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    const int8_t imm = static_cast<int8_t>(
        parse_immediate(line, idx, INT8_MIN, INT8_MAX));

    Instruction instr = MovHInstr
    {
        .dst = dst,
        .imm = imm
    };

    instructions.push_back(instr);
}

static void parse_SubInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src1 = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src2 = parse_reg(line, idx);

    Instruction instr = SubInstr
    {
        .dst = dst,
        .src1 = src1,
        .src2 = src2
    };

    instructions.push_back(instr);
}

static void parse_SubImInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    const int8_t imm = static_cast<int8_t>(
        parse_immediate(line, idx, INT8_MIN, INT8_MAX));

    Instruction instr = SubImInstr
    {
        .dst = dst,
        .imm = imm
    };

    instructions.push_back(instr);
}

static void parse_LodBInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T addr = parse_reg(line, idx);

    Instruction instr = LodBInstr
    {
        .dst = dst,
        .addr = addr
    };

    instructions.push_back(instr);
}

static void parse_LodSBInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T addr = parse_reg(line, idx);

    Instruction instr = LodSBInstr
    {
        .dst = dst,
        .addr = addr
    };

    instructions.push_back(instr);
}

static void parse_LodWInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T addr = parse_reg(line, idx);

    Instruction instr = LodWInstr
    {
        .dst = dst,
        .addr = addr
    };

    instructions.push_back(instr);
}

static void parse_StrBInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T addr = parse_reg(line, idx);

    Instruction instr = StrBInstr
    {
        .dst = dst,
        .addr = addr
    };

    instructions.push_back(instr);
}

static void parse_StrWInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T addr = parse_reg(line, idx);

    Instruction instr = StrWInstr
    {
        .dst = dst,
        .addr = addr
    };

    instructions.push_back(instr);
}

static void parse_MulInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src1 = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src2 = parse_reg(line, idx);

    Instruction instr = MulInstr
    {
        .dst = dst,
        .src1 = src1,
        .src2 = src2
    };

    instructions.push_back(instr);
}

static void parse_DivInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src1 = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src2 = parse_reg(line, idx);

    Instruction instr = DivInstr
    {
        .dst = dst,
        .src1 = src1,
        .src2 = src2
    };
    
    instructions.push_back(instr);
}

static void parse_BUncondInst(const std::string &line, size_t &idx)
{
    skip_whitespace(line, idx);
    std::string_view view = make_view(line, idx);

    const size_t start = idx;
    size_t end = view.find_first_of(" ");

    if(end == std::string_view::npos)
    {
        end = view.size();
    }

    if(end == 0)
    {
        print_loc(start);
        std::cout << " -> Expected label\n";
    }

    idx += end;

    std::string label {view};

    Instruction instr = BUncondInst
    {
        .label = std::move(label)
    };

    instructions.push_back(instr);
}

static void parse_BCondInst(
    const std::string &line, 
    size_t &idx, 
    BCondType cond)
{
    skip_whitespace(line, idx);
    std::string_view view = make_view(line, idx);

    const size_t start = idx;
    size_t end = view.find_first_of(" ");

    if(end == std::string_view::npos)
    {
        end = view.size();
    }

    if(end == 0)
    {
        print_loc(start);
        std::cout << " -> Expected label\n";
    }


    idx += end;
    
    std::string label {view};

    Instruction instr = BCondInst
    {
        .cond = cond,
        .label = std::move(label),
    };

    instructions.push_back(instr);
}

static void parse_BLinkInstr(const std::string &line, size_t &idx)
{
    reg_T addr = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T link = parse_reg(line, idx);

    Instruction instr = BLinkInstr
    {
        .addr = addr,
        .link = link
    };

    instructions.push_back(instr);
}

static void parse_BRegInstr(const std::string &line, size_t &idx)
{
    reg_T addr = parse_reg(line, idx);

    Instruction instr = BRegInstr
    {
        .addr = addr
    };

    instructions.push_back(instr);
}

static void parse_AndInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src = parse_reg(line, idx);

    Instruction instr = AndInstr
    {
        .dst = dst,
        .src = src
    };

    instructions.push_back(instr);
}

static void parse_NandInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src = parse_reg(line, idx);

    Instruction instr = NandInstr
    {
        .dst = dst,
        .src = src
    };

    instructions.push_back(instr);
}

static void parse_OrInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src = parse_reg(line, idx);

    Instruction instr = OrInstr
    {
        .dst = dst,
        .src = src
    };

    instructions.push_back(instr);
}

static void parse_NorInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src = parse_reg(line, idx);

    Instruction instr = NorInstr
    {
        .dst = dst,
        .src = src
    };

    instructions.push_back(instr);
}

static void parse_XorInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src = parse_reg(line, idx);

    Instruction instr = XorInstr
    {
        .dst = dst,
        .src = src
    };

    instructions.push_back(instr);
}

static void parse_XnorInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src = parse_reg(line, idx);

    Instruction instr = XnorInstr
    {
        .dst = dst,
        .src = src
    };

    instructions.push_back(instr);
}

static void parse_NotInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src = parse_reg(line, idx);

    Instruction instr = NotInstr
    {
        .dst = dst,
        .src = src
    };

    instructions.push_back(instr);
}

static void parse_LslInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    int8_t imm = parse_immediate(line, idx, 0, 16);

    Instruction instr = LslInstr
    {
        .dst = dst,
        .imm = imm
    };
    
    instructions.push_back(instr);
}

static void parse_LsrInstr(const std::string &line, size_t &idx)
{
     reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    int8_t imm = parse_immediate(line, idx, 0, 16);

    Instruction instr = LslInstr
    {
        .dst = dst,
        .imm = imm
    };

    instructions.push_back(instr);
}

static void parse_NoopInstr()
{
    instructions.push_back(NoopInstr{});
}

static void parse_HaltInstr()
{
    instructions.push_back(HaltInstr{});
}

static void parse_CmpInstr(const std::string &line, size_t &idx)
{
    reg_T src1 = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src2 = parse_reg(line, idx);

    Instruction instr = CmpInstr 
    {
        .src1 = src1,
        .src2 = src2
    };

    instructions.push_back(instr);
}

static void parse_MovInstr(const std::string &line, size_t &idx)
{
    reg_T dst = parse_reg(line, idx);
    parse_comma(line, idx);

    reg_T src = parse_reg(line, idx);

    Instruction instr = MovInstr
    {
        .dst = dst,
        .src = src
    };

    instructions.push_back(instr);
}

static void parse_CallInstr(const std::string &line, size_t &idx)
{
    skip_whitespace(line, idx);
    std::string_view view = make_view(line, idx);

    const size_t start = idx;
    size_t end = view.find_first_of(" ");

    if(end == std::string_view::npos)
    {
        end = view.size();
    }

    if(end == 0)
    {
        print_loc(start);
        std::cout << " -> Expected label\n";
    }

    idx += end;

    std::string label {view};

    Instruction instr = CallInstr
    {
        .label = std::move(label)
    };

    instructions.push_back(instr);
}

static void parse_RetInstr()
{
    instructions.push_back(RetInstr{});
}

static void parse_PushInstr(const std::string &line, size_t &idx)
{
    reg_T reg = parse_reg(line, idx);

    instructions.push_back(PushInstr{reg});
}

static void parse_PopInstr(const std::string &line, size_t &idx)
{
    reg_T reg = parse_reg(line, idx);

    instructions.push_back(PopInstr{reg});
}


static void parse_instr(const std::string &line, size_t &idx)
{
    std::string_view view {line.data() + idx, line.size() - idx};

    const size_t start = idx;
    size_t end = view.find_first_of(" ");

    if(end == std::string_view::npos)
    {
        print_loc(idx);
        std::cout << " -> Invalid instruction: \"" << view << "\"\n";
        exit(-1);
    }

    view = view.substr(0, end);
    idx += end;

    switch(view.size())
    {
        case 1:

            if(view == "b") { parse_BUncondInst(line, idx); }
            else
            {
                print_loc(start);
                std::cout << " -> Unknown instruction: \"" << view << "\"\n";
                exit(-1);
            }
            break;

        case 2:

            if(view == "br") { parse_BRegInstr(line, idx); }
            else if(view == "bl") { parse_BLinkInstr(line, idx); }
            else if(view == "or") { parse_OrInstr(line, idx); }
            else
            {
                print_loc(start);
                std::cout << " -> Unknown instruction: \"" << view << "\"\n";
                exit(-1);
            }
            break;

        case 3:

            if(view == "add") { parse_AddInstr(line, idx); }
            else if(view == "sub") { parse_SubInstr(line, idx); }
            else if(view == "mul") { parse_MulInstr(line, idx); }
            else if(view == "div") { parse_DivInstr(line, idx); }
            else if(view == "and") { parse_AndInstr(line, idx); }
            else if(view == "nor") { parse_NorInstr(line, idx); }
            else if(view == "xor") { parse_XorInstr(line, idx); }
            else if(view == "not") { parse_NotInstr(line, idx); }
            else if(view == "lsl") { parse_LslInstr(line, idx); }
            else if(view == "lsr") { parse_LsrInstr(line, idx); }
            else if(view == "hlt") { parse_HaltInstr(); }
            else if(view == "cmp") { parse_CmpInstr(line, idx); }
            else if(view == "mov") { parse_MovInstr(line, idx); }
            else if(view == "ret") { parse_RetInstr(); }
            else if(view == "pop") { parse_PopInstr(line, idx); }
            else
            {
                print_loc(start);
                std::cout << " -> Unknown instruction: \"" << view << "\"\n";
                exit(-1);
            }
            break;

        case 4:

            if(view == "addi") { parse_AddImInstr(line, idx); }
            else if(view == "subi") { parse_SubImInstr(line, idx); }
            else if(view == "movl") { parse_MovLInstr(line, idx); }
            else if(view == "movh") { parse_MovHInstr(line, idx); }
            else if(view == "lodb") { parse_LodBInstr(line, idx); }
            else if(view == "lodw") { parse_LodWInstr(line, idx); }
            else if(view == "strb") { parse_StrBInstr(line, idx); }
            else if(view == "strw") { parse_StrWInstr(line, idx); }
            else if(view == "beql") 
                { parse_BCondInst(line, idx, BCondType::EQL); }
            else if(view == "bneq")
                { parse_BCondInst(line, idx, BCondType::NEQ); }
            else if(view == "bgtu")
                { parse_BCondInst(line, idx, BCondType::GTU); }
            else if(view == "bltu") 
                { parse_BCondInst(line, idx, BCondType::LTU); }
            else if(view == "bgts")
                { parse_BCondInst(line, idx, BCondType::GTS); }
            else if(view == "blts")
                { parse_BCondInst(line, idx, BCondType::LTS); }
            else if(view == "nand") { parse_NandInstr(line, idx); }
            else if(view == "xnor") { parse_XnorInstr(line, idx); }
            else if(view == "noop") { parse_NoopInstr(); }
            else if(view == "call") { parse_CallInstr(line, idx); }
            else if(view == "push") { parse_PushInstr(line, idx); }
            else
            {
                print_loc(start);
                std::cout << " -> Unknown instruction: \"" << view << "\"\n";
                exit(-1);
            }
            break;

        case 5:

            if(view == "lodsb") { parse_LodSBInstr(line, idx); }
            else
            {
                print_loc(start);
                std::cout << " -> Unknown instruction: \"" << view << "\"\n";
                exit(-1);
            }
            break;

        default:

            print_loc(start);
            std::cout << " -> Unknown instruction: \"" << view << "\"\n";
            exit(-1);
    }

    skip_whitespace(line, idx);
    ++text_offset;

    if(idx >= line.size()) { return; }

    if(line.at(idx) != '/')
    {
        print_loc(idx);
        std::cout << " -> Only comments can follow instructions.\n";
        exit(-1);
    }

    ++idx;

    if(idx >= line.size() || line.at(idx) != '/')
    {
        print_loc(idx - 1);
        std::cout << " -> Unexpected \"/\"\n";
        exit(-1);
    }

    // Comment has been encountered, ignore the rest of the line.
}

static void parse_dataop(const std::string &line, size_t &idx)
{
    skip_whitespace(line, idx);

    if(idx >= line.size()) { return; }

    std::string_view view {line.data() + idx, line.size() - idx};

    std::cout << "Parsing dataop: \"" << view << "\"\n";

    size_t end = view.find_first_of(" ");

    if(end == std::string_view::npos)
    {
        end = view.size();
    } 
}

static void parse_line(const std::string &line)
{   
    ++current_line;

    if(line.size() == 0) { return; }
    
    size_t idx = 0;

    if(line.at(0) == '.')
    {
        std::string_view view {line.data() + idx, line.size() + idx};

        size_t end = view.find_first_of(" \n");

        view = view.substr(0, end);

        if(view == ".text") { current_section = SECTION::TEXT; }

        else if(view == ".data") { current_section = SECTION::DATA; }

        else
        {
            print_loc();
            std::cout << " -> Invalid section: \"" << view << "\"\n";
            exit(-1);
        }

        std::cout << "Parsed section: " << view << '\n';

        if(end == std::string_view::npos)
        {
            idx = line.size();
            return;
        }

        idx += end;

        skip_whitespace(line, idx);

        if(idx != line.size())
        {
            print_loc();
            std::cout << " -> Section label must be alone on its line.\n";
            exit(-1);
        }
    }

    else if(line.at(0) == '/')
    {
        if(line.size() < 2 || line.at(1) != '/')
        {
            print_loc();
            std::cout << " -> Unexpected \"/\"\n";
            exit(-1);
        }

        // This line is a comment.
        return;
    }

    std::string_view view = make_view(line, idx);

    size_t colon_pos = view.find_first_of(":");

    // Label
    if(colon_pos != std::string_view::npos)
    {
        view = view.substr(0, colon_pos);

        const size_t start = idx;

        std::cout << "Got label: " << view << '\n';
        idx += colon_pos + 1;
    
        std::string label_ident {view};
    
        const auto it = labels.find(label_ident);

        if(it != labels.end())
        {
            print_loc(start);
            std::cout << " -> Label already defined: \"" << 
                label_ident << "\"\n";
            exit(-1);
        }

        DefinedLabel label
        {
            .section = current_section,
            .offset = 
                current_section == SECTION::TEXT ? text_offset : data_offset
        };

        labels.emplace(std::move(label_ident), label);
    }

    skip_whitespace(line, idx);

    if(idx == line.size()) { return; }
    
    if(current_section == SECTION::TEXT) { parse_instr(line, idx); }
    else { parse_dataop(line, idx); }
}


/**
 * @brief Makes a first pass over the source code, collecting instructions and 
 * data operations, as well as defining labels.
 */
static void first_pass()
{
    std::string line;

    while(std::getline(in_file, line)) { parse_line(line); }
}

static void second_pass()
{

}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        std::cout << "Usage: snasm <in_file> <out_file>\n";
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

    first_pass();

    return 0;
}
