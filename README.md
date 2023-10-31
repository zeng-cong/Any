# Any
the class is a container that can wrap any type in c++

# example
```
    std::string testStr="hello ,world";
    Any any(testStr);
    std::string &refStr=any;
    std::string valueStr=any;
    any=10;
    int &refInt=any;
```