#include <string>
#include <utility>
#include <vtil/vtil>

namespace bf
{
    class lifter
    {
    public:
        explicit lifter(std::string program);
        void lift();
        void optimize();
        void dump();
        void save(std::string path);
        vtil::routine* get_routine();
    private:
        std::string m_program;
        vtil::basic_block* m_block;
        vtil::vip_t m_vip;
        std::list<vtil::vip_t> m_branches;
        vtil::routine* m_routine;

        void handle_instruction(char instruction);
        void handle_inc();
        void handle_dec();
        void handle_print();
        void handle_read();
        void handle_te();
        void handle_tne();
        void update_branch();
    };
}