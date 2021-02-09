#include <iostream>
#include "HelloWorldManager.h"


int main()
{
    std::cout << "Hello, World!" << std::endl;
    HelloWorldManager helloWorldManager;
    helloWorldManager.wait();
    return 0;
}
