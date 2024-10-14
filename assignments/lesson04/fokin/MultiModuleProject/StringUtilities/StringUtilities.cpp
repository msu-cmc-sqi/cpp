#include <string>
int countChars(const std::string& s){
    return s.length();
}
bool isPalindrome(const std::string& s){
    int l=s.length();
    for(int i=0; i<l/2; i++){
        if(s[i]!=s[l-1-i])
            return false;
    }
    return true;
}