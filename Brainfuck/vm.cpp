#include "vm.hpp"

using namespace bf;

vm::vm(vtil::routine* routine, bool debug) : m_routine(routine), m_debug(debug)
{
    initialize_memory();
    initialize_hooks();
}

void vm::execute()
{
    auto it = m_routine->entry_point->begin();

    while(true)
    {
        if(m_debug) vtil::logger::log("Executing block %d\n", it.container->entry_vip);

        // a symbolic_vm can only execute per basic_block
        auto lim = m_vm.run(it, true);
        if(lim.is_end()) break;

        if(*lim->base == vtil::ins::js)
        {
            // checks whether the condition is true/false and loads the respective basic_block
            auto reg = lim->operands[0].reg();
            auto true_condition = lim->operands[1].imm().u64;
            auto false_condition = lim->operands[2].imm().u64;
            it = m_vm.read_register(reg).get<bool>().value()
                 ? m_routine->explored_blocks[true_condition]->begin()
                 : m_routine->explored_blocks[false_condition]->begin();
        }

        // Brainfuck doesn't support absolute jumps but optimizations might introduce them
        // TODO: Handle accordingly once optimizations work reliably
    }
}

void vm::initialize_hooks()
{
    m_vm.hooks.execute = [&](const vtil::instruction& instruction)
    {
        if(m_debug)
        {
            auto sp_offset = (m_vm.read_register(vtil::REG_SP) - vtil::symbolic::make_register_ex(vtil::REG_SP)).get<uint64_t>().value();
            auto sp_value = m_vm.read_memory(m_vm.read_register(vtil::REG_SP), 1).get<uint8_t>().value_or(0xff);
            vtil::logger::log("%-50s | [SP+%d] => %d\n", instruction, sp_offset, sp_value);
        }

        if (*instruction.base == vtil::ins::vemit)
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
        return m_vm.symbolic_vm::execute(instruction);
    };
}

void vm::initialize_memory()
{
    m_vm.memory_state =
         {
             [](auto& ptr, bitcnt_t size)
             {
                 return vtil::symbolic::expression{0, size};
             }
         };
}

void vm::print()
{
    auto reg = vtil::operand{x86_reg::X86_REG_AL}.reg();
    auto character = m_vm.read_register(reg).get<char>().value();
    vtil::logger::log("%c", character);
}

void vm::read()
{
    auto reg = vtil::operand{x86_reg::X86_REG_AL}.reg();
    auto character = getc(stdin);
    std::cin.ignore(INT_MAX, '\n'); // flush buffer to ignore \n character. NOTE: need to check if this interferes with a single \n as well
    m_vm.write_register(reg, vtil::symbolic::expression{character, 8});
}
