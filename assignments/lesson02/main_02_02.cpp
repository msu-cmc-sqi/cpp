#include <iostream>
#include <stdexcept>
#include <algorithm>

class DynArr {
    int* data_  = nullptr;
    int  sz_    = 0;   // size
    int  cap_   = 0;   // capacity

    void grow() {
        int new_cap = std::max(1, cap_ * 2);
        int* nd = new int[new_cap];
        for (int i = 0; i < sz_; ++i) nd[i] = data_[i];
        delete[] data_;
        data_ = nd;
        cap_  = new_cap;
    }

public:
    DynArr() = default;

    ~DynArr() { delete[] data_; }

    // Копирующий конструктор
    DynArr(const DynArr& o) : data_(nullptr), sz_(o.sz_), cap_(o.cap_) {
        if (cap_) data_ = new int[cap_];
        for (int i = 0; i < sz_; ++i) data_[i] = o.data_[i];
    }

    // Копирующее присваивание
    DynArr& operator=(const DynArr& o) {
        if (this == &o) return *this;
        int* nd = (o.cap_ ? new int[o.cap_] : nullptr);
        for (int i = 0; i < o.sz_; ++i) nd[i] = o.data_[i];
        delete[] data_;
        data_ = nd; sz_ = o.sz_; cap_ = o.cap_;
        return *this;
    }

    // Перемещение (ускоряет возврат временных значений)
    DynArr(DynArr&& o) noexcept : data_(o.data_), sz_(o.sz_), cap_(o.cap_) {
        o.data_ = nullptr; o.sz_ = o.cap_ = 0;
    }
    DynArr& operator=(DynArr&& o) noexcept {
        if (this != &o) {
            delete[] data_;
            data_ = o.data_; sz_ = o.sz_; cap_ = o.cap_;
            o.data_ = nullptr; o.sz_ = o.cap_ = 0;
        }
        return *this;
    }

    void push_back(int x) {
        if (sz_ == cap_) grow();
        data_[sz_++] = x;
    }

    void pop_back() { if (sz_ > 0) --sz_; }

    int& operator[](int i) {
        if (i < 0 || i >= sz_) throw std::out_of_range("index");
        return data_[i];
    }
    const int& operator[](int i) const {
        if (i < 0 || i >= sz_) throw std::out_of_range("index");
        return data_[i];
    }

    void print() const {
        for (int i = 0; i < size(); ++i) {
            std::cout << data_[i] << " ";
        }
        std::cout << std::endl;
    }

    int  size() const { return sz_; }
    int  capacity() const { return cap_; }
    bool empty() const { return sz_ == 0; }
    void clear() { sz_ = 0; }
};

int sum(const DynArr& arr) {
    int s = 0;
    for (int i = 0; i < arr.size(); ++i) {
        s += arr[i];   // тут вызовется const-версия operator[]
    }
    return s;
}

int main() {
    DynArr a;

    // добавление
    a.push_back(10);
    a.push_back(20);
    a.push_back(30);

    // доступ по индексу
    std::cout << "a[1] = " << a[1] << "\n";  // 20
    a[1] = 99;

    // проход по элементам (через size() и operator[])
    std::cout << "a: ";
    for (int i = 0; i < a.size(); ++i) {
        std::cout << a[i] << ' ';
    }
    std::cout << "\n";

    // pop_back
    a.pop_back(); // удалили 30

    // копирование
    DynArr b = a;        // копирующий конструктор
    b.push_back(777);
    b.print();

    // перемещение
    DynArr c = std::move(b); // b опустошён (нулевые указатели/размер)
    std::cout << "c.size() = " << c.size() << "\n";

    // работа через константную ссылку (пример функции ниже)
    const DynArr& cref = c;
    c.print();
    std::cout << "sum(c) = " << sum(cref) << "\n";

    return 0;
}