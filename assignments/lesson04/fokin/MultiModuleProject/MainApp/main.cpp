#include <iostream>
#include <MathFunctions.h>
#include <StringUtilities.h>
using namespace std;
int main(){
    int a=5;
    double b=0.2;
    string s1="aaa", s2="aab";
    cout << square(a) << " " << cube(a) << endl;
    cout << square(b) << " " << cube(b) << endl;
    cout << countChars(s1) << " " << isPalindrome(s1) << endl;
    cout << countChars(s2) << " " << isPalindrome(s2) << endl;
    return 0;
}
