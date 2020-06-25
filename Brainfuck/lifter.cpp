#include "lifter.hpp"

using namespace bf;

lifter::lifter(std::string program) : m_program(std::move(program)), m_block(nullptr), m_vip(0) {}

void lifter::lift()
{
    m_block = vtil::basic_block::begin(m_vip);
    m_branches = std::list<vtil::vip_t>();

    for(auto instruction : m_program)
    {
        handle_instruction(instruction);
    }

    m_block->vexit(0ull);

    vtil::logger::log("Lifted! Running optimizations...\n\n");
}

void lifter::optimize()
{
    vtil::optimizer::apply_each<
            vtil::optimizer::profile_pass,
            vtil::optimizer::collective_pass
    >{}(m_block->owner);

    vtil::logger::log("\nOptimizations applied! Here's the VTIL:\n\n");
}

void lifter::dump()
{
    vtil::debug::dump(m_block->owner);
}

void lifter::save(std::string path)
{
    vtil::logger::log("\nSaving VTIL to %s\n\n", path);
    vtil::save_routine(m_block->owner, path);
}

vtil::routine* lifter::get_routine()
{
    return m_block->owner;
}

void lifter::handle_instruction(char instruction)
{
    switch (instruction)
    {
        case '>':
            m_block->add(vtil::REG_SP, 1);
            break;
        case '<':
            m_block->sub(vtil::REG_SP, 1);
            break;
        case '+':
            handle_inc();
            break;
        case '-':
            handle_dec();
            break;
        case '[':
            handle_te();
            break;
        case ']':
            handle_tne();
            break;
        case '.':
            handle_print();
            break;
        case ',':
            handle_read();
            break;
        default:
            break;
    }
}

void lifter::handle_inc()
{
    auto current_value = m_block->tmp(8);
    m_block->ldd(current_value, vtil::REG_SP, vtil::make_imm(0ull));
    m_block->add(current_value, 1);
    m_block->str(vtil::REG_SP, vtil::make_imm(0ull), current_value);
}

void lifter::handle_dec()
{
    auto current_value = m_block->tmp(8);
    m_block->ldd(current_value, vtil::REG_SP, vtil::make_imm(0ull));
    m_block->sub(current_value, 1);
    m_block->str(vtil::REG_SP, vtil::make_imm(0ull), current_value);
}

void lifter::handle_te()
{
    auto [tmp, cond] = m_block->tmp(8, 1);
    m_block->ldd(tmp, vtil::REG_SP, 0);
    m_block->te(cond, tmp, 0);
    m_block->js(cond, vtil::invalid_vip, ++m_vip);

    m_branches.push_back(m_block->entry_vip);
    m_block = m_block->fork(m_vip);
}

void lifter::handle_tne()
{
    auto [tmp, cond] = m_block->tmp(8, 1);
    m_block->ldd(tmp, vtil::REG_SP, 0);
    m_block->tne(cond, tmp, 0);
    m_block->js(cond, vtil::invalid_vip, ++m_vip);

    update_branch();
    m_block = m_block->fork(m_vip);
}

void lifter::handle_print()
{
    m_block->ldd(x86_reg::X86_REG_AL, vtil::REG_SP, 0);
    m_block->vpinr(x86_reg::X86_REG_AL); // make sure this doesn't get optimized away
    m_block->vemit('.');
}

void lifter::handle_read()
{
    m_block->vemit(',');
    m_block->vpinw(x86_reg::X86_REG_AL); // make sure this doesn't get optimized away
    m_block->str(vtil::REG_SP, 0, x86_reg::X86_REG_AL);
}

void lifter::update_branch()
{
    auto matching_vip = m_branches.back(); m_branches.pop_back();
    auto matching_block = m_block->owner->explored_blocks[matching_vip];
    matching_block->stream.back().operands[1].imm().u64 = m_vip;
    //matching_block->fork(m_vip); // link the previously undefined block
    m_block->stream.back().operands[1].imm().u64 = matching_vip + 1;
    //m_block->fork(matching_vip + 1); // link the previously undefined block
    m_block->fork(m_block->entry_vip); // link the previously undefined block
}