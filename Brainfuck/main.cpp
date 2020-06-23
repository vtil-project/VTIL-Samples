#include <string>
#include <vtil/vtil>

#pragma comment(linker, "/STACK:67108864")

const std::string program = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";

void handle_inc(vtil::basic_block*& block);
void handle_dec(vtil::basic_block*& block);
void handle_print(vtil::basic_block*& block);
void handle_read(vtil::basic_block*& block);
void handle_te(vtil::basic_block*& block, vtil::vip_t& pc, std::list<vtil::vip_t>& blocks);
void handle_tne(vtil::basic_block*& block, vtil::vip_t& pc, std::list<vtil::vip_t>& blocks);
void handle_instruction(vtil::basic_block*& block, vtil::vip_t& pc, std::list<vtil::vip_t>& blocks);
void update_branch(vtil::basic_block*& block, vtil::vip_t& pc, std::list<vtil::vip_t>& blocks);

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

void handle_te(vtil::basic_block*& block, vtil::vip_t& pc, std::list<vtil::vip_t>& blocks)
{
    auto [tmp, cond] = block->tmp(8, 1);
    block->ldd(tmp, vtil::REG_SP, 0);
    block->te(cond, tmp, 0);
    block->js(cond, ++pc, vtil::invalid_vip);

    blocks.push_back(block->entry_vip);
    block = block->fork(pc);
}

void handle_tne(vtil::basic_block*& block, vtil::vip_t& pc, std::list<vtil::vip_t>& blocks)
{
    auto [tmp, cond] = block->tmp(8, 1);
    block->ldd(tmp, vtil::REG_SP, 0);
    block->tne(cond, tmp, 0);
    block->js(cond, block->entry_vip, ++pc);

    update_branch(block, pc, blocks);
    block = block->fork(pc);
}

void handle_print(vtil::basic_block*& block)
{
    block->ldd(x86_reg::X86_REG_AL, vtil::REG_SP, 0);
    block->vpinr(x86_reg::X86_REG_AL); // make sure this doesn't get optimized away
    block->vemit('.');
}

void handle_read(vtil::basic_block*& block)
{
    block->vemit(',');
    block->vpinw(x86_reg::X86_REG_AL); // make sure this doesn't get optimized away
    block->str(vtil::REG_SP, 0, x86_reg::X86_REG_AL);
}

void update_branch(vtil::basic_block*& block, vtil::vip_t& pc, std::list<vtil::vip_t>& blocks)
{
    auto matching_vip = blocks.back(); blocks.pop_back();
    auto matching_block = block->owner->explored_blocks[matching_vip];
    matching_block->stream.back().operands[2].imm().u64 = pc; // this is prob wrong?
    block->fork(block->entry_vip); // link the previously undefined block
}

void handle_instruction(vtil::basic_block*& block, vtil::vip_t& pc, std::list<vtil::vip_t>& blocks)
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
            handle_te(block, pc, blocks);
            break;
        case ']':
            handle_tne(block, pc, blocks);
        case '.':
            handle_print(block);
            break;
        case ',':
            handle_read(block);
            break;
        default:
            break;
    }
}

int main()
{
    auto block = vtil::basic_block::begin(0x0);
    auto blocks = std::list<vtil::vip_t>();

    // allocate data memory
    //block->shift_sp(-30); // TODO: is this even required?

    for(vtil::vip_t pc = 0; pc < program.size(); ++pc)
    {
        handle_instruction(block, pc, blocks);
    }

    block->vpinr(vtil::REG_SP);
    block->vpinw(vtil::REG_SP);
    block->vexit(0ull);

    //block->owner->routine_convention = vtil::preserve_all_convention;
    //block->owner->routine_convention.purge_stack = false;

    //vtil::optimizer::apply_all(block->owner);

    vtil::debug::dump(block->owner);
}