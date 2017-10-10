# Cxxcurl
just c++ wrapper for curl

useage:
	auto func = [](char* data, size_t len) {
                printf("%d\n",len);
        };
        CXXCURL::Get("www.example.com", func);
