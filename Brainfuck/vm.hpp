#include <vtil/vtil>

namespace bf
{
    class vm : vtil::vm_interface
    {
    public:
        explicit vm(bool debug = false);
        vtil::vm_exit_reason execute(const vtil::instruction& instruction) override;
        void execute(const vtil::routine* routine);

        vm(vm&&) = default;
        vm(const vm&) = default;
        vm& operator=(vm&&) = default;
        vm& operator=(const vm&) = default;
    private:
        bool m_debug;
        mutable std::vector<uint8_t> m_stack_state;
        mutable vtil::symbolic::context m_context;

        vtil::symbolic::expression::reference read_register(const vtil::register_desc& desc) const override;
        void write_register(const vtil::register_desc& desc, vtil::symbolic::expression::reference value) override;

        vtil::symbolic::expression::reference read_memory(const vtil::symbolic::expression::reference& pointer, size_t byte_count) const override;
        bool write_memory(const vtil::symbolic::expression::reference& pointer, vtil::deferred_value<vtil::symbolic::expression::reference> value, bitcnt_t size) override;

        vtil::symbolic::expression::reference reference_io_port();

        void print();
        void read();
    };
}