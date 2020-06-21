#include <vtil/io>

int main()
{
	std::vector<std::string> test = { "Hello", "VTIL!" };
	vtil::logger::log("=> %s\n", test);

	return 0;
}
