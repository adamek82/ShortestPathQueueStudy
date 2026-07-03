#include "app.hpp"

#include <iostream>
#include <string>

namespace app {

std::string greeting() {
    return "Hello from CppCMakeVSCodeTemplate";
}

} // namespace app

int main() {
    std::cout << app::greeting() << '\n';
    return 0;
}
