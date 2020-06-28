#include "vm.hpp"

using namespace bf;

vm::vm(bool debug) : m_debug(debug)
{
    m_register_state[vtil::REG_SP] = 0x0;
}

bool vm::execute(const vtil::instruction& instruction)
{
    if(m_debug)
    {
        auto sp_offset = m_register_state[vtil::REG_SP];
        auto sp_value = read_memory(sp_offset, 1).get<uint8_t>().value();
        vtil::logger::log("%-50s | [SP+%d] => %d\n", instruction, sp_offset, sp_value);
    }

    if(*instruction.base == vtil::ins::vemit)
    {
        auto bf_instruction = instruction.operands[0].imm().u64;
        switch(bf_instruction)
        {
            case '.': print(); break;
            case ',': read(); break;
            default: unreachable();
        }

        return true;
    }

    // pin instructions and jumps are to be handled manually
    if(*instruction.base == vtil::ins::vpinr) return true;
    if(*instruction.base == vtil::ins::vpinw) return true;

    if(instruction.base->is_branching_virt()) return false;
    if(instruction.base->is_branching_real()) return true;

    // actually execute the instruction
    return vm_interface::execute(instruction);
}

void vm::execute(const vtil::routine* routine)
{
    m_stack_state.clear();
    m_register_state.clear();

    auto it = routine->entry_point->begin();

    while(true)
    {
        if(m_debug) vtil::logger::log("Executing block %d\n", it.container->entry_vip);

        auto lim = run(it, true);
        if(lim.is_end()) break;

        if(*lim->base == vtil::ins::js)
        {
            // checks whether the condition is true/false and loads the respective basic_block
            auto reg = lim->operands[0].reg();
            auto true_condition = lim->operands[1].imm().u64;
            auto false_condition = lim->operands[2].imm().u64;
            it = read_register(reg).get<bool>().value()
                 ? routine->explored_blocks.find(true_condition)->second->begin()
                 : routine->explored_blocks.find(false_condition)->second->begin();
        }

        // Brainfuck doesn't support absolute jumps but optimizations might introduce them
        // TODO: Handle accordingly once optimizations work reliably
    }
}

vtil::symbolic::expression vm::read_register(const vtil::register_desc& desc)
{
    vtil::register_desc full = { desc.flags, desc.local_id, size_register(desc), 0, desc.architecture };

    auto value = m_register_state[full];
    return { (value >> desc.bit_offset) & vtil::math::fill(desc.bit_count), desc.bit_count };
}

void vm::write_register(const vtil::register_desc& desc, vtil::symbolic::expression value)
{
    vtil::register_desc full = { desc.flags, desc.local_id, size_register(desc), 0, desc.architecture };

    auto& rvalue = m_register_state[full];
    rvalue &= ~desc.get_mask();
    rvalue |= ((value.get().value() & vtil::math::fill(desc.bit_count)) << desc.bit_offset);
}

vtil::symbolic::expression vm::read_memory(const vtil::symbolic::expression& pointer, size_t byte_count)
{
    auto ptr = pointer.value.get().value();

    if(m_stack_state.size() < ptr + byte_count)
        m_stack_state.resize(ptr + byte_count);

    uint64_t qword = 0;
    memcpy(&qword, &m_stack_state[ptr], byte_count);

    return { qword, vtil::math::narrow_cast<bitcnt_t>(byte_count * 8) };
}

void vm::write_memory(const vtil::symbolic::expression& pointer, vtil::symbolic::expression value)
{
    auto ptr = pointer.value.get<uint64_t>().value();
    auto byte_count = value.size() / 8;

    if(m_stack_state.size() < ptr + byte_count)
        m_stack_state.resize(ptr + byte_count);

    auto qword = value.get<uint64_t>().value();
    memcpy(&m_stack_state[ptr], &qword, byte_count);
}

uint8_t& vm::reference_io_port()
{
    return reinterpret_cast<uint8_t&>(m_register_state[vtil::operand{ X86_REG_RAX }.reg()]);
}

void vm::print()
{
    vtil::logger::log("%c", reference_io_port());
}

void vm::read()
{
    reference_io_port() = getc(stdin);
    std::cin.ignore(INT_MAX, '\n'); // TODO: fix this?
}