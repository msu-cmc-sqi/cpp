#include <iostream>
using namespace std;

class Test {
    int x;
public:
    Test(int val) { x = val; }
    // Отсутствует деструктор, копирующий конструктор, оператор присваивания
    
    void print() {
        cout << "Value: " << x << endl;
    }
};

int main() {
    int x = 10  // Отсутствует точка с запятой
    cout << "Value: " << x << endl;
    
    int* ptr = new int(5);
    // Потенциальная утечка памяти - нет delete
    
    char buffer[10];
    cin >> buffer;  // Возможное переполнение буфера
    
    int arr[5];
    arr[5] = 10;  // Выход за границы массива
    
    return 0;
}