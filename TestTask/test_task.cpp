#include <iostream>
#include "my_string.h"
#include <vector>
#include <algorithm>

int main()
{
	std::vector<my::string> strings;

	std::cout << "Type some text..." << std::endl;
	std::cout << "Double press ENTER to stop" << std::endl;

	{
		for(;;)
		{
			my::string str;
			std::cin >> str;
			if (str.size() <= 0) {
				break;
			}
			strings.push_back(str);
		}
	}

	// Про сортировку ничего не было сказано
	// взял из std :)
	std::sort(strings.rbegin(), strings.rend());

	std::cout << std::endl
		<< "There are sorted by descending strings:" << std::endl;

	for (auto str : strings)
	{
		std::cout << str << std::endl;
	}

	return 0;
}
