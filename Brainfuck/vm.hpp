#include <vtil/vtil>

namespace bf
{
    class vm
    {
    public:
        explicit vm(vtil::routine* routine, bool debug = false);
        void execute();
    private:
        vtil::routine* m_routine;
        vtil::lambda_vm<vtil::symbolic_vm> m_vm;
        bool m_debug;

        void initialize_hooks();
        void initialize_memory();
        void print();
        void read();
    };
}