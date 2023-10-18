#include <util/circularList.h>

#include <iostream>

int main(int argc, char const *argv[])
{
	CircularList<int> list(10);
	for(int i = 0; i < 5; ++i) {
		list.insert(i);
	}
	for(auto & elem : list) {
		std::cout << elem << std::endl;
	}

	for(int i = 0; i < 15; ++i) {
		list.insert(i);
	}

	for(CircularList<int>::Iterator it = list.begin(); it != list.end(); ++it) {
		std::cout << *it << std::endl;
	}
	return 0;
}