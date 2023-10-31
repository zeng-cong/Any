#include "Any.h"
#include <string>
#include <vector>
int main()
{
    std::string testStr = "hello ,world";
    Any any(testStr);
    std::string &refStr = any;
    std::string valueStr = any;
    any = 10;
    int &refInt = any;
    int valueInt = any;
    std::vector<Any> arr{std::string("hello world"), 10, 23123.141, 'q', "hello world"};
    return 0;
}