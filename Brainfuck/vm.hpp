#include <vtil/vtil>

namespace bf
{
    class vm : vtil::vm_interface
    {
    public:
        explicit vm(bool debug = false);
        bool execute(const vtil::instruction& instruction) override;
        void execute(const vtil::routine* routine);

        vm(vm&&) = default;
        vm(const vm&) = default;
        vm& operator=(vm&&) = default;
        vm& operator=(const vm&) = default;
    private:
        bool m_debug;
        std::vector<uint8_t> m_stack_state;
        std::map<vtil::register_desc, uint64_t> m_register_state;

        vtil::symbolic::expression::reference read_register(const vtil::register_desc& desc) override;
        void write_register(const vtil::register_desc& desc, vtil::symbolic::expression::reference value) override;

        vtil::symbolic::expression::reference read_memory(const vtil::symbolic::expression::reference& pointer, size_t byte_count) override;
        void write_memory(const vtil::symbolic::expression::reference& pointer, vtil::symbolic::expression::reference value) override;

        uint8_t& reference_io_port();

        void print();
        void read();
    };
}