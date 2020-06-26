#include <string>
#include <vtil/vtil>
#include "lifter.hpp"

#pragma comment(linker, "/STACK:67108864")

std::pair<std::string, std::optional<std::string>> handle_arguments(int argc, char* argv[])
{
    if(argc < 2)
    {
        vtil::logger::error(
                "%s\n%s\n",
                "Missing argument! Usage:",
                "Brainfuck.exe path_to_brainfuck_program.bf [path_to_output_vtil.vtil]");
    }
    else
    {
        std::ifstream stream(argv[1]);
        auto program = std::string(
                std::istreambuf_iterator<char>(stream),
                std::istreambuf_iterator<char>());

        std::optional<std::string> output = std::nullopt;
        if(argc == 3) output = std::make_optional(argv[2]);

        return {program, output};
    }
}

int main(int argc, char* argv[])
{
    auto [program, output] = handle_arguments(argc, argv);

    bf::lifter lifter(program);
    lifter.lift();
    //lifter.optimize();
    lifter.dump();

    if(output) lifter.save(output.value());

    auto routine = lifter.get_routine();

    vtil::lambda_vm<vtil::symbolic_vm> vm;
    vm.memory_state =
        {
            [](auto& ptr, bitcnt_t size)
            {
                return vtil::symbolic::expression{0, size};
            }
        };

    vm.hooks.execute = [&](const vtil::instruction& ins)
    {
        auto sp_offset = (vm.read_register(vtil::REG_SP) - vtil::symbolic::make_register_ex(vtil::REG_SP)).get<uint64_t>().value();
        auto sp_value = vm.read_memory(vm.read_register(vtil::REG_SP), 1).get<uint8_t>().value_or(0xff);
        vtil::logger::log("%-50s | [SP+%d] => %d\n", ins, sp_offset, sp_value);

        if (*ins.base == vtil::ins::vemit)
        {
            switch(ins.operands[0].imm().u64)
            {
                case '.': putc(*vm.read_register(vtil::operand{X86_REG_AL}.reg()).get<char>(), stdout); break;
                case ',': vm.write_register(vtil::operand{X86_REG_AL}.reg(), vtil::symbolic::expression{getc(stdin), 8}); break;
                default: unreachable();
            }

            return true;
        }

        if(*ins.base == vtil::ins::vpinr) return true;
        if(*ins.base == vtil::ins::vpinw) return true;

        if(ins.base->is_branching_virt()) return false;
        if(ins.base->is_branching_real()) return true;

        return vm.symbolic_vm::execute(ins);
    };

    auto it = routine->entry_point->begin();
    while(true)
    {
        vtil::logger::log("Executing block %d\n", it.container->entry_vip);

        auto lim = vm.run(it, true);
        if(lim.is_end()) break;

        if(*lim->base == vtil::ins::js)
        {
            it = *vm.read_register(lim->operands[0].reg()).get<bool>()
                 ? routine->explored_blocks[lim->operands[1].imm().u64]->begin()
                 : routine->explored_blocks[lim->operands[2].imm().u64]->begin();
            vm.write_register(vtil::REG_SP, vm.read_register(vtil::REG_SP) + lim.container->sp_offset);
            continue;
        }
        else if(*lim->base == vtil::ins::jmp)
        {
            it = routine->explored_blocks[lim->operands[0].imm().u64]->begin();
            vm.write_register(vtil::REG_SP, vm.read_register(vtil::REG_SP) + lim.container->sp_offset);
            continue;
        }

        unreachable();
    }
}