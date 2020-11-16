#include "vm.hpp"

using namespace bf;

vm::vm(bool debug) : m_debug(debug)
{
    m_context.write(vtil::REG_SP, 0);
}

vtil::vm_exit_reason vm::execute(const vtil::instruction& instruction)
{
    if(m_debug)
    {
        auto sp_offset = m_context.read(vtil::REG_SP);
        auto sp_value = read_memory(sp_offset, 1)->get<uint8_t>().value();
        vtil::logger::log("%-50s | [SP+%d] => %d\n", instruction, sp_offset, sp_value);
    }

    if(*instruction.base == vtil::ins::vemit)
    {
        puts("vemit");
        auto bf_instruction = instruction.operands[0].imm().u64;
        switch(bf_instruction)
        {
            case '.': print(); break;
            case ',': read(); break;
            default: unreachable();
        }

        return vtil::vm_exit_reason::none;
    }

    // pin instructions and jumps are to be handled manually
    if(*instruction.base == vtil::ins::vpinr) return vtil::vm_exit_reason::none;
    if(*instruction.base == vtil::ins::vpinw) return vtil::vm_exit_reason::none;

    if(instruction.base->is_branching_virt()) return vtil::vm_exit_reason::unknown_instruction;
    if(instruction.base->is_branching_real()) return vtil::vm_exit_reason::none;

    // actually execute the instruction
    return vm_interface::execute(instruction);
}

void vm::execute(const vtil::routine* routine)
{
    m_stack_state.clear();
    m_context.reset();

    auto it = routine->entry_point->begin();

    while(true)
    {
        if(m_debug) vtil::logger::log("Executing block %d\n", it.block->entry_vip);

        auto [lim, reason] = run(it);
        if(lim.is_end()) break;

        if(*lim->base == vtil::ins::js)
        {
            // checks whether the condition is true/false and loads the respective basic_block
            auto reg = lim->operands[0].reg();
            auto true_condition = lim->operands[1].imm().u64;
            auto false_condition = lim->operands[2].imm().u64;
            it = read_register(reg)->get<bool>().value()
                 ? routine->explored_blocks.find(true_condition)->second->begin()
                 : routine->explored_blocks.find(false_condition)->second->begin();
        }

        // Brainfuck doesn't support absolute jumps but optimizations might introduce them
        // TODO: Handle accordingly once optimizations work reliably
    }
}

vtil::symbolic::expression::reference vm::read_register(const vtil::register_desc& desc) const
{
    vtil::logger::log("read %s\n", desc);
    return m_context.read(desc);
}

void vm::write_register(const vtil::register_desc& desc, vtil::symbolic::expression::reference value)
{
    vtil::logger::log("write %s\n", desc);
    m_context.write(desc, value);
}

vtil::symbolic::expression::reference vm::read_memory(const vtil::symbolic::expression::reference& pointer, size_t byte_count) const
{
    vtil::logger::log("pointer %s\n", pointer);
    auto ptr = pointer->get().value();

    if(m_stack_state.size() < ptr + byte_count)
        m_stack_state.resize(ptr + byte_count);

    uint64_t qword = 0;
    memcpy(&qword, &m_stack_state[ptr], byte_count);

    return { qword, vtil::math::narrow_cast<bitcnt_t>(byte_count * 8) };
}

bool vm::write_memory(const vtil::symbolic::expression::reference& pointer, vtil::deferred_value<vtil::symbolic::expression::reference> value, bitcnt_t size)
{
    auto ptr = pointer->value.get<uint64_t>().value();
    auto byte_count = size / 8;

    if(m_stack_state.size() < ptr + byte_count)
        m_stack_state.resize(ptr + byte_count);

    auto qword = value.get()->get<uint64_t>().value();
    memcpy(&m_stack_state[ptr], &qword, byte_count);
    return true;
}

static vtil::register_desc register_cast(x86_reg r)
{
    return vtil::register_cast<x86_reg>()(r);
}

vtil::symbolic::expression::reference vm::reference_io_port()
{
    return m_context.read(X86_REG_AL);
}

void vm::print()
{
    vtil::logger::log("%s", reference_io_port());
}

void vm::read()
{
    reference_io_port() = getc(stdin);
    std::cin.ignore(INT_MAX, '\n'); // TODO: fix this?
}