#include <string>
#include <vtil/vtil>

#pragma comment(linker, "/STACK:67108864")

//const std::string hello_world = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";
const std::string hello_world = "[>>>>>---]+";

void handle_inc(vtil::basic_block*& block)
{
    auto current_value = block->tmp(8);
    block->ldd(current_value, vtil::REG_SP, vtil::make_imm(0ull));
    block->add(current_value, 1);
    block->str(vtil::REG_SP, vtil::make_imm(0ull), current_value);
}

void handle_dec(vtil::basic_block*& block)
{
    auto current_value = block->tmp(8);
    block->ldd(current_value, vtil::REG_SP, vtil::make_imm(0ull));
    block->sub(current_value, 1);
    block->str(vtil::REG_SP, vtil::make_imm(0ull), current_value);
}

void handle_js(vtil::basic_block*& block)
{
    auto [tmp, cond] = block->tmp(8, 1);
    block->ldd(tmp, vtil::REG_SP, 0);
    block->te(cond, tmp, 0);
    block->js(cond, vtil::invalid_vip, vtil::invalid_vip);
}

void handle_instruction(char instruction, vtil::basic_block*& block)
{
    switch (instruction)
    {
        case '>':
            block->add(vtil::REG_SP, 1);
            break;
        case '<':
            block->sub(vtil::REG_SP, 1);
            break;
        case '+':
            handle_inc(block);
            break;
        case '-':
            handle_dec(block);
            break;
        case '[':
            handle_js(block);
            break;
        case ']':
            break;
        default:
            break;
    }
}

int main()
{
    auto block = vtil::basic_block::begin(0x0);

    // allocate data memory
    block->shift_sp(-30); // TODO: is this even required?

    for(auto instruction : hello_world)
    {
        handle_instruction(instruction, block);
    }

    block->vpinr(vtil::REG_SP);
    block->vpinw(vtil::REG_SP);
    block->vexit(0ull);

    //vtil::optimizer::apply_all(block->owner);

    vtil::debug::dump(block->owner);
}