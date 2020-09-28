#include <vtil/vtil>

int main()
{
	// https://0xnobody.github.io/devirtualization-intro/

	constexpr auto block_vip = 0x1234;
	constexpr auto reg_size = 64;
	constexpr auto reg_offs = 0;

	// block_vip is just something to identify the block.
	// you can use the relative or absolute VIP of the block's first instruction.
	//
	auto block = vtil::basic_block::begin(block_vip);

	// the following instruction defines a register. In VTIL, we can define as many registers as we want.
	//
	// - vtil::register_virtual means that the register is virtual i.e. it is only existent in the VM context
	// - we get the register id via reg_offs / 8, as in our example VM all registers are 8-byte aligned. This
	// won't always be the case.
	// - 64 specifies the register's size. For our example, the register is always 64 bits.
	// - next, we get the bit offst by getting the modulus of our register offset
	//
	vtil::register_desc reg(vtil::register_virtual, 64, reg_size, (reg_offs % 8) * 8);

	// quite self explanatory :^)
	// note that in VTIL, we can chain these calls for that super clean look!
	//
	block->push(reg);

	// And finally let's use dump our precious little routine:
	vtil::debug::dump(block->owner);
}
