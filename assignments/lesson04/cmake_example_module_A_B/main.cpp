#include <iostream>

#ifdef HAVE_MODULE_A
#include "ModuleA.h"
#endif

#ifdef HAVE_MODULE_B
#include "ModuleB.h"
#endif

int main() {
    std::cout << "Hello main\n";

    #ifdef HAVE_MODULE_A
    ModuleAhello();
    #endif

    #ifdef HAVE_MODULE_B
    ModuleBhello();
    #endif

    return 0;
}
