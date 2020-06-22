#include <string>
#include <vtil/vtil>

#pragma comment(linker, "/STACK:67108864")

//const std::string hello_world = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";
const std::string hello_world = "+>++>+++<--<-";

void handle_inc(vtil::basic_block*& block)
{
    auto current_value = block->tmp(64);
    block->ldd(current_value, vtil::REG_SP, vtil::make_imm(0ull));
    block->add(current_value, 1);
    block->str(vtil::REG_SP, vtil::make_imm(0ull), current_value);
}

void handle_dec(vtil::basic_block*& block)
{
    auto current_value = block->tmp(64);
    block->ldd(current_value, vtil::REG_SP, vtil::make_imm(0ull));
    block->sub(current_value, 1);
    block->str(vtil::REG_SP, vtil::make_imm(0ull), current_value);
}

int main()
{
    auto block = vtil::basic_block::begin(0x40000000);

    // allocate data memory
    block->shift_sp(-30);

    for(auto instruction : hello_world)
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
            default:
                break;
        }
    }

    block->vpinr(vtil::REG_SP);
    block->vpinw(vtil::REG_SP);
    block->vexit(0ull);

    vtil::optimizer::apply_all(block->owner);

    vtil::debug::dump(block->owner);
}