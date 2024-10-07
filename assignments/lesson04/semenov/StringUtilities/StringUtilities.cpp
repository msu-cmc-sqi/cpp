#include "StringUtilities.h"

int countChars(const std::string &s){ return s.length(); }

bool isPalindrome(const std::string &s){
	if (s.empty()) return true;
	auto i = s.begin();
	auto j = s.end() - 1;
	while (i < j && *i == *j) {
		++i;
		--j;
	}

	return i >= j;
}