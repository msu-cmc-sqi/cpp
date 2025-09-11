#include <iostream>

using namespace std;

struct dyn_arr {
    int* arr;      // данные
    int  mx_sz;    // capacity
    int  cur_sz;   // size

    dyn_arr() {
        arr    = (int*)malloc(4 * sizeof(int));
        mx_sz  = 4;
        cur_sz = 0;
    }

    ~dyn_arr() { free(arr); }

    void push_back(int x) {
        if (cur_sz < mx_sz) { arr[cur_sz++] = x; return; }
        // нет места — расширяем в 2 раза
        int* narr = (int*)malloc(2 * mx_sz * sizeof(int));
        for (int i = 0; i < mx_sz; ++i) narr[i] = arr[i];
        mx_sz *= 2;
        free(arr);
        arr = narr;
        arr[cur_sz++] = x;
    }

    void pop_back() { if (cur_sz > 0) --cur_sz; }

    int& operator[](int i) {
        if (i < 0 || i >= cur_sz) { printf("index out of range\n"); exit(1); }
        return arr[i];
    }

    void print() const {
        for(int i = 0; i < size(); i++) {
            cout << arr[i] << " ";
        }
        cout << endl;
    }

    int* begin() { return arr; }
    int* end()   { return arr + cur_sz; }

    const int* begin() const { return arr; }
    const int* end()   const { return arr + cur_sz; }

    void print_iter() const {
        for (auto x : *this) {   // тут *this = текущий объект
            cout << x << " ";
        }
        cout << endl;
    }

    int size() const { return cur_sz; }
    int capacity() const { return mx_sz; }
};

int main() {
    dyn_arr a;        // [ ]
    a.push_back(2);   // [2]
    a.push_back(5);   // [2, 5]

    a.print_iter();

    a.pop_back();     // [2]

    a.print();

    a[0] = 42;        // [42]



    return 0;
}