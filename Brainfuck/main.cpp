#include <string>
#include <vtil/vtil>

#pragma comment(linker, "/STACK:67108864")

const std::string program = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";

void handle_inc(vtil::basic_block*& block);
void handle_dec(vtil::basic_block*& block);
void handle_te(vtil::basic_block*& block, vtil::vip_t& pc);
void handle_print(vtil::basic_block*& block);
void handle_read(vtil::basic_block*& block);
vtil::basic_block* handle_tne(vtil::basic_block*& block);
vtil::basic_block* handle_instruction(vtil::basic_block*& block, vtil::vip_t& pc);
vtil::basic_block* process_block(vtil::basic_block*& block, vtil::vip_t& pc);

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

void handle_te(vtil::basic_block*& block, vtil::vip_t& pc)
{
    auto [tmp, cond] = block->tmp(8, 1);
    block->ldd(tmp, vtil::REG_SP, 0);
    block->te(cond, tmp, 0);
    block->js(cond, ++pc, vtil::invalid_vip);

    auto branch_block = block->fork(pc);
    process_block(branch_block, pc);

    auto after_branch_block = block->fork(++pc);
    auto tne_block = process_block(after_branch_block, pc);

    block->stream.back().operands[2].imm().u64 = after_branch_block->entry_vip;

    if(tne_block != nullptr) tne_block->stream.back().operands[2].imm().u64 = after_branch_block->entry_vip; // in case the program ends on one of the children
}

vtil::basic_block* handle_tne(vtil::basic_block*& block)
{
    auto [tmp, cond] = block->tmp(8, 1);
    block->ldd(tmp, vtil::REG_SP, 0);
    block->tne(cond, tmp, 0);
    block->js(cond, block->entry_vip, vtil::invalid_vip);

    return block;
}

void handle_print(vtil::basic_block*& block)
{
    block->ldd(x86_reg::X86_REG_AL, vtil::REG_SP, 0);
    block->vpinr(x86_reg::X86_REG_AL); // make sure this doesn't get optimized away
    block->vemits("write");
}

void handle_read(vtil::basic_block*& block)
{
    block->vemits("read");
    block->vpinw(x86_reg::X86_REG_AL); // make sure this doesn't get optimized away
    block->str(vtil::REG_SP, 0, x86_reg::X86_REG_AL);
}

vtil::basic_block* handle_instruction(vtil::basic_block*& block, vtil::vip_t& pc)
{
    switch (program[pc])
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
            handle_te(block, pc);
            break;
        case ']':
            return handle_tne(block);
        case '.':
            handle_print(block);
            break;
        case ',':
            handle_read(block);
            break;
        default:
            break;
    }

    return nullptr;
}

vtil::basic_block* process_block(vtil::basic_block*& block, vtil::vip_t& pc)
{
    for(; pc < program.size(); ++pc)
    {
        auto tne_block = handle_instruction(block, pc);
        if(tne_block != nullptr) return tne_block;
    }

    return nullptr;
}

int main()
{
    auto block = vtil::basic_block::begin(0x0);

    // allocate data memory
    block->shift_sp(-30); // TODO: is this even required?

    vtil::vip_t pc = 0;
    process_block(block, pc);

    auto end_block = block->fork(program.size());
    end_block->vpinr(vtil::REG_SP);
    end_block->vpinw(vtil::REG_SP);
    end_block->vexit(0ull);

    //block->owner->routine_convention = vtil::preserve_all_convention;
    //block->owner->routine_convention.purge_stack = false;

    //vtil::optimizer::apply_all(block->owner);

    vtil::debug::dump(block->owner);
}