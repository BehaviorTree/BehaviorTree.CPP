## Single file header only sqlite wrapper for C++

## Example
```cpp
#include "sqlite.hpp"
#include <iostream>

int main()
{
    sqlite::Connection connection("example.db");

    sqlite::Statement(connection, "CREATE TABLE IF NOT EXISTS exampleTable ("
                                  "textData TEXT, "
                                  "intData INTEGER, "
                                  "floatData REAL)");

    sqlite::Statement(connection,
                      "INSERT INTO exampleTable VALUES (?, ?, ?)",
                      "Hello world",
                      1234,
                      5.6789);

    sqlite::Result res = sqlite::Query(connection, "SELECT * FROM exampleTable");

    while(res.Next())
    {
        std::string textData = res.Get(0);
        int intData = res.Get(1);
        float floatData = res.Get(2);

        std::cout << textData << " " << intData << " " << floatData << std::endl;
    }
    
    return 0;
}

```
