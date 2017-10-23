# Cxxcurl
just c++ wrapper for curl

useage:<br />
```cpp
	auto func = [](char* data, size_t len) {
                printf("%d\n",len);
        };
        CXXCURL::Get("www.example.com", func);
	CXXCURL::Post("www.example.com", "123", func);
```
