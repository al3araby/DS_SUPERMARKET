// main.cpp - thin entrypoint
#include "system.h"
#include <iostream>
using namespace std;

int main() {
    SupermarketSystem sys(3);
    cout << "Supermarket sys started.\n";
    sys.interactive_console();
    return 0;
}

