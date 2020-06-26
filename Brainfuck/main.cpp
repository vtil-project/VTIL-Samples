#include <string>
#include <vtil/vtil>
#include "lifter.hpp"
#include "vm.hpp"

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

    bf::vm vm(routine);
    vm.execute();
}