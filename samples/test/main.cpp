#include "spl/slot_map.h"
#include <iostream>
#include <string_view>

int main()
{
	spl::slot_map<std::string_view> sm;
	spl::slot_wrap<std::string_view> s1 = sm.as_wrap(sm.emplace_back("Hello!\n"));
	spl::slot_handle s2 = sm.emplace_back("Buh-bye!\n");

	for (std::string_view& s : sm) {
		std::cout << s;
	}

	sm.erase(s1.get_key());

	std::cout << sm[s2];

	try
	{
		/* code */
		*s1;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
	
	return 0;
}