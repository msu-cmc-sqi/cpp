#include <string>

int countChars(std::string s){
	return s.size();}

bool isPalindrome(std::string s){
	int k = s.size();
	for (int i = 0; i < (int)(k / 2); i++){
		if (s[i] != s[k - i - 1]){
			return false;
		}
	}
	return true;
}

